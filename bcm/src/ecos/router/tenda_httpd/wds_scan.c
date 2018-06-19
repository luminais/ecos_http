#if 0
#include <bcmutils.h>
#include <wlioctl.h>
#include <shutils.h>
#include <proto/802.11.h>
#include <proto/ethernet.h>
#endif
#include "wds_scan.h"
#include "route_cfg.h"
#include "string.h"
#include "cJSON.h"
#ifdef __CONFIG_SUPPORT_GB2312__
#include "wds_encode.h"
WIFI_STA_LIST_SSID_ENCODE_ARRAY encode_gb_list[WDS_SAN_LIST_LENGTH];	//hong add for full-width character
#endif

extern int tenda_dump_bss_info(wl_bss_info_t *bi);
extern int tenda_dump_bss_crypto(wl_bss_info_t *bi);

static wds_ap_list_info_t ap_wds_list[WDS_ENR_MAX_AP_SCAN_LIST_LEN];
static char wds_scan_result[WDS_ENR_DUMP_BUF_LEN]; 
static int /*wds_scanned = 0,*/ wds_ap_num = 0;

extern void udelay(int delay);
#define SLEEP(n)	cyg_thread_delay(n * 100)
#define USLEEP(X)	udelay(X)
#define IFVALUE(value)	 if(value==NULL)	\
							value=""	 
#define _GET_VALUE(N,V)	V=nvram_safe_get(N)
#define nvram_safe_get(name) (nvram_get(name) ? : "")
#define _SET_VALUE(N,V)	do{\
		nvram_set(N,V); \
}while(0)

#define SSID_LENGTH_MAX (48 - 1)	//hong add for full-width character

extern int get_ifname_unit(const char* ifname, int *unit, int *subunit);

/* This utility routine builds the wl prefixes from wl_unit.
 * Input is expected to be a null terminated string
 *
 * Inputs -prefix: Pointer to prefix buffer
 *	  -prefix_size: Size of buffer
 *        -Mode flag: If set generates unit.subunit output
 *                    if not set generates unit only
 *	  -ifname: Optional interface name string
 *
 *
 * Returns - pointer to prefix, NULL if error.
 *
 *
*/
static char * make_wds_prefix(char *prefix,int prefix_size, int mode, char *ifname)
{
	int unit=-1,subunit=-1;
	char *wl_unit=NULL;

	//assert(prefix);
	//assert(prefix_size);

	if (ifname){
		//assert(*ifname);
		wl_unit=ifname;
	}else{
		wl_unit=nvram_get("wl_unit");

		if (!wl_unit)
			return NULL;
	}

	if (get_ifname_unit(wl_unit,&unit,&subunit) < 0 )
		return NULL;

	if (unit < 0) return NULL;

	if  ((mode) && (subunit > 0 ))
		snprintf(prefix, prefix_size, "wl%d.%d_", unit,subunit);
	else
		snprintf(prefix, prefix_size, "wl%d_", unit);

	return prefix;
}


static int wds_enr_display_aplist(wds_ap_list_info_t *ap)
{
	char eastr[ETHER_ADDR_STR_LEN];
	int i=0;
	
	if(!ap)
		return 0;
#if 0
	diag_printf("-------------------------------------\n");
	while(ap->used == TRUE ) {
		diag_printf(" %d :  ", i);
		diag_printf("SSID:%s  ", ap->ssid);
		diag_printf("BSSID:%s  ", ether_etoa(ap->BSSID, eastr));	
		diag_printf("Channel:%d  ", ap->channel);
		if(ap->wep)
			diag_printf("WEP");
		diag_printf("RSSI:%d ", ap->sig);
		diag_printf("\n");
		ap++; 
		i++;
	}
	
	diag_printf("-------------------------------------\n");
#endif	
	return 1;
}
					
static bool
wds_enr_wl_is_wds_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len)
{
	uint8 *ie = *wpaie;

	/* If the contents match the WPA_OUI and type=1 */
	if ((ie[1] >= 6) && !memcmp(&ie[2], WPA_OUI "\x04", 4)) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[1] + 2;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)(ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return FALSE;
}
					
static uint8 *
wds_enr_parse_tlvs(uint8 *tlv_buf, int buflen, uint key)
{
	uint8 *cp;
	int totlen;

	cp = tlv_buf;
	totlen = buflen;

	/* find tagged parameter */
	while (totlen >= 2) {
		uint tag;
		int len;

		tag = *cp;
		len = *(cp +1);

		/* validate remaining totlen */
		if ((tag == key) && (totlen >= (len + 2)))
			return (cp);

		cp += (len + 2);
		totlen -= (len + 2);
	}

	return NULL;
}


static bool
wds_enr_is_wds_ies(uint8* cp, uint len)
{
	uint8 *parse = cp;
	uint parse_len = len;
	uint8 *wpaie;

	while ((wpaie = wds_enr_parse_tlvs(parse, parse_len, DOT11_MNG_WPA_ID)))
		if (wds_enr_wl_is_wds_ie(&wpaie, &parse, &parse_len))
			break;
	if (wpaie)
		return TRUE;
	else
		return FALSE;
}

static int
wds_enr_get_aplist(wds_ap_list_info_t *list_in, wds_ap_list_info_t *list_out )
{
	wds_ap_list_info_t *ap_in = &list_in[0];
	wds_ap_list_info_t *ap_out = &list_out[0];
	int i=0, wds_apcount = 0;

	/* ignore hidden SSID AP */
	while(ap_in->used == TRUE && ap_in->ssidLen && i < WDS_ENR_MAX_AP_SCAN_LIST_LEN) {
		if(TRUE == wds_enr_is_wds_ies(ap_in->ie_buf, ap_in->ie_buflen)) {
			memcpy(ap_out, ap_in, sizeof(wds_ap_list_info_t));
			wds_apcount++;
			ap_out = &list_out[wds_apcount];	
		}
		i++;
		ap_in = &list_in[i];
	}
	/* in case of on-the-spot filtering, make sure we stop the list  */
	if(wds_apcount < WDS_ENR_MAX_AP_SCAN_LIST_LEN)
		ap_out->used = 0;

	return wds_apcount;
}
 
 
 
static char *wds_enr_get_scan_results(char *ifname)
{
	int ret, retry_times = 0;
	wl_scan_params_t *params;
	wl_wds_scan_results_t *list = (wl_wds_scan_results_t*)wds_scan_result;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE +WDS_NUMCHANS * sizeof(uint16);
 	int org_scan_time = 20, scan_time = 80;
	
	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL) {
		return NULL;
	}

	memset(params, 0, params_size);
	
	params->bss_type = DOT11_BSSTYPE_ANY;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = -1;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

		/* extend scan channel time to get more AP probe resp */
	wl_ioctl(ifname, WLC_GET_SCAN_CHANNEL_TIME, &org_scan_time, sizeof(org_scan_time));
	if (org_scan_time < scan_time)
		wl_ioctl(ifname, WLC_SET_SCAN_CHANNEL_TIME, &scan_time,	sizeof(scan_time));
	
retry:
	ret = wl_ioctl(ifname, WLC_SCAN, params, params_size);
	if (ret < 0) {
		if (retry_times++ < WDS_ENR_SCAN_RETRY_TIMES) {
			diag_printf("set scan command failed, retry %d\n", retry_times);
			SLEEP(1);	
			goto retry;
		}
	}
	SLEEP(3);

	list->buflen = WDS_ENR_DUMP_BUF_LEN;
	ret = wl_ioctl(ifname, WLC_SCAN_RESULTS, wds_scan_result, WDS_ENR_DUMP_BUF_LEN);
	if (ret < 0 && retry_times++ < WDS_ENR_SCAN_RETRY_TIMES) {
		diag_printf("get scan result failed, retry %d\n", retry_times);
		SLEEP(1);
		goto retry;
	}

	free(params);
	if (ret < 0)
		return NULL;

	return wds_scan_result;
}

static int wds_list_entry_number=0;
static wds_ap_list_info_t *wds_enr_create_aplist()
{
	char *name = NULL;
	char tmp[WDS_NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	memset(wds_scan_result, 0, sizeof(wds_scan_result));
	wl_wds_scan_results_t *list = (wl_wds_scan_results_t*)wds_scan_result;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi;
	uint i, wds_ap_count = 0;

	if (!make_wds_prefix(prefix,sizeof(prefix),0,NULL))
		return NULL;
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (wds_enr_get_scan_results(name) == NULL)
		return NULL;

	memset(ap_wds_list, 0, sizeof(ap_wds_list));
	if (list->count == 0)
		return NULL;
	else if (list->version != WL_BSS_INFO_VERSION &&
			list->version != LEGACY_WL_BSS_INFO_VERSION) {
			diag_printf( "Sorry, your driver has bss_info_version %d "
		    	"but this program supports only version %d.\n",
		   	 list->version, WL_BSS_INFO_VERSION); 
		return NULL;
	}

	bi = list->bss_info;
	//diag_printf("wds_ap_count=%d\n",list->count);
	for (i = 0; i < list->count; i++) {
        /* Convert version 107 to 108 */
		if (bi->version == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
			bi->ie_length = old_bi->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}
		if (bi->ie_length) {
			if(wds_ap_count < WDS_ENR_MAX_AP_SCAN_LIST_LEN){
				ap_wds_list[wds_ap_count].used = TRUE;
				memcpy(ap_wds_list[wds_ap_count].BSSID, (uint8 *)&bi->BSSID, 6);
				strncpy((char *)ap_wds_list[wds_ap_count].ssid, (char *)bi->SSID, bi->SSID_len);
				ap_wds_list[wds_ap_count].ssid[bi->SSID_len] = '\0';
				ap_wds_list[wds_ap_count].ssidLen= bi->SSID_len;
				ap_wds_list[wds_ap_count].ie_buf = (uint8 *)(((uint8 *)bi) + bi->ie_offset);
				ap_wds_list[wds_ap_count].ie_buflen = bi->ie_length;

				if( (uint8)bi->ctl_ch == 0)
					ap_wds_list[wds_ap_count].channel = (uint8)(bi->chanspec & WL_CHANSPEC_CHAN_MASK);
				else
					ap_wds_list[wds_ap_count].channel =  (uint8)bi->ctl_ch;
				
				if( bi->capability & DOT11_CAP_PRIVACY){
					ap_wds_list[wds_ap_count].wep = tenda_dump_bss_info(bi);
					ap_wds_list[wds_ap_count].cipher = tenda_dump_bss_crypto(bi);					
				}	
				else		
					ap_wds_list[wds_ap_count].wep = 0;
				
				ap_wds_list[wds_ap_count].sig = bi->RSSI;
				wds_ap_count++;
				wds_list_entry_number=wds_ap_count;
				
			}
		}
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}

	/*wds_scanned = 1;*/
	return ap_wds_list;
}

#ifdef __CONFIG_SUPPORT_GB2312__
/*****************hong add for full-width character*******************/
static void encode_aplist_ssid_2_utf8(wds_ap_list_info_t *aplists)
{
	wds_ap_list_info_t *aplist = aplists;
	char tmp_ssid[50] = {0};
	int encode_num = 0;

	int ni;
	for(ni = 0; ni < WDS_SAN_LIST_LENGTH + 1; ni++)
	{
		strcpy(encode_gb_list[ni].ssid_gb, "\0");
	}
	if (!aplist)
	{
		printf("aplist is null!\n");
		return ;
	}

	while (TRUE == aplist->used)
	{
		/*judge encode*/
		int ret = is_cn_encode(aplist->ssid);
		
		/*when found gb2312 chinese or characters,encode to utf-8*/
		if (1 == ret)
		{
			memset(tmp_ssid, '\0', sizeof(tmp_ssid));
			
			set_cn_ssid_encode("utf-8", aplist->ssid, tmp_ssid);
			
			if ( strlen(tmp_ssid) > SSID_LENGTH_MAX + 1)
			{
				printf("SSID Encode Length out of range!\n");
			}
			else
			{
				strncpy(aplist->ssid, tmp_ssid, SSID_LENGTH_MAX + 1);
				
				/*record ssid encode to array,restore ssid origanal encode*/
				strcpy(encode_gb_list[encode_num].ssid_gb, aplist->ssid);
				encode_gb_list[encode_num].ssid_gb[strlen(aplist->ssid) + 1] = '\0';
				strcpy(encode_gb_list[encode_num].code_type, "gb2312");
				encode_num++;
			}
		}
		aplist++;
	}
	
	//printf("gb2312 encode num:%d\n", encode_num);
	
	return;
}

/******************************end add*******************************/
#endif

#define callwdsscan 1
#define uncallwdsscan 0
static int static_wds_scan = callwdsscan;

/*static int ej_wds_enr_scan_result(int eid, webs_t wp, int argc, char_t **argv)*/

void wireless_wds_scan(webs_t wp, char_t *path, char_t *query)
{
	int i = 0, iflag = 0, retry=0;
	unsigned char macstr[18];
	//char ssid[36],channe[4], wep[32], sig[8],cipher[16],sec[64];
	char ssid[52],channe[4], wep[32], sig[8],cipher[16],sec[64];	//hong modify ssid
	wds_ap_list_info_t *wdsaplist, *ap;
	char_t temSSID[256]={0};

	if(static_wds_scan == uncallwdsscan)
		return;
	
	static_wds_scan = uncallwdsscan;

recreate:		
	if( NULL == wds_enr_create_aplist()  &&
		retry++ < WDS_ENR_SCAN_RETRY_TIMES && wds_list_entry_number <=0){
		diag_printf("wds retry=%d\n",retry);
		goto recreate;
	}

	wdsaplist = ap_wds_list;
	
	if(wds_list_entry_number==0){	//add by stanley 2010/12/08
		websWrite(wp, T("stanley\n"));
		websDone(wp,200);
		goto out;
	}
	
	if(0 == wds_enr_display_aplist(wdsaplist)){
		websWrite(wp, T("stanley\n"));
		websDone(wp,200);
		goto out;

	}

	/**********hong add for full-width character************/
#ifdef __CONFIG_SUPPORT_GB2312__
	encode_aplist_ssid_2_utf8(wdsaplist);
#endif

	ap = wdsaplist;
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	while(ap->used == TRUE ) {
		sprintf((char *)macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
			ap->BSSID[0], ap->BSSID[1], ap->BSSID[2],
			ap->BSSID[3], ap->BSSID[4], ap->BSSID[5]);
		sprintf((char *)ssid, "%s", ap->ssid);
		sprintf((char *)channe, "%d", ap->channel);
		switch(ap->wep){
			case 0:
					sprintf((char *)wep, "%s", "NONE");
					break;
			case 1:
					sprintf((char *)wep, "%s", "WPA");
					break;
			case 2:
					sprintf((char *)wep, "%s", "WPA2");
					break;
			case 3:
					sprintf((char *)wep, "%s", "WPAWPA2");
					break;
			case 4:
					sprintf((char *)wep, "%s", "WEP");
					break;				
			default:
					sprintf((char *)wep, "%s", "UNKNOW");
					break;		
		}
		if(ap->wep != 0 && ap->wep != 4){
			switch(ap->cipher){
				case 1:
						sprintf((char *)cipher, "%s", "AES");
						break;
				case 2:
						sprintf((char *)cipher, "%s", "TKIP");
						break;
				case 3:
						sprintf((char *)cipher, "%s", "AESTKIP");
						break;			
				default:
						sprintf((char *)cipher, "%s", "UNKNOW");
						break;		
			}			
			sprintf((char *)sec,"%s/%s",wep,cipher);		
		}
		else			
			sprintf((char *)sec,"%s",wep);		

		if(ap->sig < 0)
			sprintf((char *)sig, "%d\%", -(ap->sig));
		else
			sprintf((char *)sig, "%d\%", (ap->sig));
		
		if (! iflag) {
			memset(temSSID,0,sizeof(temSSID));
			websWrite(wp, T("%s\t%s\t%s\t%s\t%s"), encodeSSID(ssid,temSSID), macstr, channe, sec/*wep*/, sig);
			iflag = 1;
		} else {
			memset(temSSID,0,sizeof(temSSID));
			websWrite(wp, T("\r%s\t%s\t%s\t%s\t%s"), encodeSSID(ssid,temSSID), macstr, channe, sec/*wep*/, sig);

		}
		ap++;
		i++;
	}


	wds_ap_num = i;
	websWrite(wp, T("\n"));
	websDone(wp,200);
out:
	static_wds_scan = callwdsscan;
	return ;
}

/*************************hqw add for F307***********************************/
/************************************************************
Function:	 formWirelessRepeatInit               
Description:  无线中继的前台接口

Input:                                          

Output: 

Return:         返回给前台扫描到的无线信号的数据

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/

void formWirelessRepeatInit(webs_t wp, char_t *path, char_t *query)
{
	int i = 0, iflag = 0, retry=0;
	unsigned char macstr[18];
	char ssid[256],channe[4], wep[32], sig[8],cipher[16],sec[64];
	wds_ap_list_info_t *wdsaplist, *ap;
	char_t temSSID[256]={0};
	char_t result[1024];
	char_t *value=NULL;
	char_t value1[128]={0};
	char SSID1_name[128] = {0}, SSID1_passwd[128] = {0};

	if(static_wds_scan == uncallwdsscan)
		return;
	
	static_wds_scan = uncallwdsscan;

recreate:		
	if( NULL == wds_enr_create_aplist()  &&
		retry++ < WDS_ENR_SCAN_RETRY_TIMES && wds_list_entry_number <=0){
		diag_printf("wds retry=%d\n",retry);
		goto recreate;
	}

	wdsaplist = ap_wds_list;
#ifdef __CONFIG_SUPPORT_GB2312__
	//hong add
	encode_aplist_ssid_2_utf8(wdsaplist);
	//end
#endif
	if(wds_list_entry_number==0){
		formWirelessRepeatApInit(wp, path, query);
		goto out;
	}
	
	if(0 == wds_enr_display_aplist(wdsaplist)){
		formWirelessRepeatApInit(wp, path, query);
		goto out;

	}

	ap = wdsaplist;
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	
	char_t unit[]="0";
	
	copy_wl_index_to_unindex(unit);

	memset(result,0,sizeof(result));

	
	_GET_VALUE(WLN0_MODE, value);//模式
	IFVALUE(value);
	char mid_value[25] = {0};
	strncat(result, "{", 1);	
	if(!strcmp(value,"sta"))
		strcpy(mid_value,"1");
	else if(!strcmp(value,"wet"))
		strcpy(mid_value,"2");
	else
		strcpy(mid_value,"0");
	string_cat(result,"repeat-mode",mid_value);
	strncat(result, ",", 1);	

	string_cat(result,"error-info",nvram_safe_get("err_check"));
	strncat(result, ",", 1);	

	char temp_ssid[256]={0};
	//hqw add for gbl2312 to utf-8
	_GET_VALUE("wl0_ssid", value);
#ifdef __CONFIG_SUPPORT_GB2312__
	if (1 == is_cn_encode(value))
	{
		char_t mib_value[49] = {0};
		char_t tmp_ssid[49] = {0};
		char_t *tmp_value = tmp_ssid;
		strcpy(mib_value, value);
		set_cn_ssid_encode("utf-8", mib_value, tmp_ssid);
		//printf("Decoded(to %s) ssid:%s\n","utf-8", tmp_ssid);
		value = tmp_value;
	}
	//end
#endif
	string_cat(result,"super-ssid",encodeSSID(value,temp_ssid));
	strncat(result, ",", 1);

	string_cat(result,"super-channel",nvram_safe_get(WLN0_CHANNEL));	
	strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_SECURITY_TYPE, value);//加密方式
	if(!strcmp(value,"0"))
		strcpy(mid_value,"OPEN/");
	else if(!strcmp(value,"1"))
		strcpy(mid_value,"SHARED/");
	else if(!strcmp(value,"psk"))
		strcpy(mid_value,"WPA/");
	else if(!strcmp(value,"psk2"))
		strcpy(mid_value,"WPA2/");
	else if(!strcmp(value,"psk psk2"))
		strcpy(mid_value,"WPAWPA2/");
	else
		strcpy(mid_value,"NONE");
	
	_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	if(!strcmp(value,"wep"))
		strncat(mid_value, "WEP", strlen("WEP"));
	else if(!strcmp(value,"aes"))
		strncat(mid_value, "AES", strlen("AES"));
	else if(!strcmp(value,"tkip"))
		strncat(mid_value, "TKIP", strlen("TKIP"));
	else if(!strcmp(value,"tkip+aes"))
		strncat(mid_value, "AESTKIP", strlen("AESTKIP"));
	string_cat(result,"super-encrypt",mid_value);
	strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_WPA_PSK1, value);//密码
	_GET_VALUE(WLN0_MODE, value);//模式
	if(strcmp(value,"ap") != 0)
		get_wl0_passwd(SSID1_passwd);
	else
		sprintf(SSID1_passwd,"%s",nvram_safe_get(WLN0_WPA_PSK1));
	char temp_pwd[256]={0};	
	string_cat(result,"super-pwd",encodeSSID(SSID1_passwd,temp_pwd));
	strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_WDS_MAC_LIST, value);//mac
	string_cat(result,"super-mac",nvram_safe_get(WLN0_WDS_MAC_LIST));
	strncat(result, ",", 1);

	strncat(result, "\"networks\"", strlen("\"networks\""));
	strncat(result, ":", 1);
	strncat(result, "[", 1);

	websWrite(wp, result);
	
	while(ap->used == TRUE ) {
		sprintf((char *)macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
			ap->BSSID[0], ap->BSSID[1], ap->BSSID[2],
			ap->BSSID[3], ap->BSSID[4], ap->BSSID[5]);
		sprintf((char *)ssid, "%s", ap->ssid);
		sprintf((char *)channe, "%d", ap->channel);
		switch(ap->wep){
			case 0:
					sprintf((char *)wep, "%s", "NONE");
					break;
			case 1:
					sprintf((char *)wep, "%s", "WPA");
					break;
			case 2:
					sprintf((char *)wep, "%s", "WPA2");
					break;
			case 3:
					sprintf((char *)wep, "%s", "WPAWPA2");
					break;
			case 4:
					sprintf((char *)wep, "%s", "WEP");
					break;				
			default:
					sprintf((char *)wep, "%s", "UNKNOW");
					break;		
		}
		if(ap->wep != 0 && ap->wep != 4){
			switch(ap->cipher){
				case 1:
						sprintf((char *)cipher, "%s", "AES");
						break;
				case 2:
						sprintf((char *)cipher, "%s", "TKIP");
						break;
				case 3:
						sprintf((char *)cipher, "%s", "AESTKIP");
						break;			
				default:
						sprintf((char *)cipher, "%s", "UNKNOW");
						break;		
			}			
			sprintf((char *)sec,"%s/%s",wep,cipher);		
		}
		else			
			sprintf((char *)sec,"%s",wep);		

		if(ap->sig > 0)
			sprintf((char *)sig, "%d\%", -(ap->sig));
		else
			sprintf((char *)sig, "%d\%", (ap->sig));

		if (! iflag) {
			memset(temSSID,0,sizeof(temSSID));
			websWrite(wp, T("{\"s-ssid\":\"%s\",\"s-mac\":\"%s\",\"s-channel\":\"%s\",\"s-encrypt\":\"%s\",\"s-signal\":\"%s\"}"), encodeSSID(ssid,temSSID), macstr, channe, sec/*wep*/, sig);
			iflag = 1;
		} else {
			memset(temSSID,0,sizeof(temSSID));
			websWrite(wp, T(",{\"s-ssid\":\"%s\",\"s-mac\":\"%s\",\"s-channel\":\"%s\",\"s-encrypt\":\"%s\",\"s-signal\":\"%s\"}"), encodeSSID(ssid,temSSID), macstr, channe, sec/*wep*/, sig);

		}
		ap++;
		i++;
	}
	copy_wl_index_to_unindex("0.1");


	wds_ap_num = i;
	websWrite(wp, T("]}\n"));
	websDone(wp,200);
out:
	static_wds_scan = callwdsscan;
	return ;
}

/************************************************************
Function:	 formWirelessRepeatApInit               
Description:  无线中继的前台接口

Input:                                          

Output: 

Return:         返回给前台AP模式下的数据

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/

void formWirelessRepeatApInit(webs_t wp, char_t *path, char_t *query)
{
	char_t result[1024];
	char_t *value=NULL;
	char_t value1[128]={0};
	char_t SSID1_passwd[128] = {0};

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	
	char_t unit[]="0";
	
	copy_wl_index_to_unindex(unit);

	memset(result,0,sizeof(result));

	
	_GET_VALUE(WLN0_MODE, value);//模式
	IFVALUE(value);
	char mid_value[25] = {0};
	strncat(result, "{", 1);
	
	//add by ll
	string_cat(result,"wifi-power", get_product_pwr_info());
	strncat(result, ",", 1);
	//end by ll

	if(!strcmp(value,"sta"))
		strcpy(mid_value,"1");
	else if(!strcmp(value,"wet"))
		strcpy(mid_value,"2");
	else
		strcpy(mid_value,"0");
	string_cat(result,"repeat-mode",mid_value);
	strncat(result, ",", 1);	

	string_cat(result,"error-info",nvram_safe_get("err_check"));
	strncat(result, ",", 1);	

	char temp_ssid[256]={0};
	//hqw add for gbl2312 to utf-8
	_GET_VALUE("wl0_ssid", value);
#ifdef __CONFIG_SUPPORT_GB2312__
	if (1 == is_cn_encode(value))
	{
		char_t mib_value[49] = {0};
		char_t tmp_ssid[49] = {0};
		char_t *tmp_value = tmp_ssid;
		strcpy(mib_value, value);
		set_cn_ssid_encode("utf-8", mib_value, tmp_ssid);
		//printf("Decoded(to %s) ssid:%s\n","utf-8", tmp_ssid);
		value = tmp_value;
	}
	//end
#endif
	string_cat(result,"super-ssid",encodeSSID(value,temp_ssid));
	strncat(result, ",", 1);

	string_cat(result,"super-channel",nvram_safe_get(WLN0_CHANNEL))	;
	strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_SECURITY_TYPE, value);//加密方式
	if(!strcmp(value,"0"))
		strcpy(mid_value,"OPEN/");
	else if(!strcmp(value,"1"))
		strcpy(mid_value,"SHARED/");
	else if(!strcmp(value,"psk"))
		strcpy(mid_value,"WPA/");
	else if(!strcmp(value,"psk2"))
		strcpy(mid_value,"WPA2/");
	else if(!strcmp(value,"psk psk2"))
		strcpy(mid_value,"WPAWPA2/");
	else
		strcpy(mid_value,"NONE");
	
	_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	if(!strcmp(value,"wep"))
		strncat(mid_value, "WEP", strlen("WEP"));
	else if(!strcmp(value,"aes"))
		strncat(mid_value, "AES", strlen("AES"));
	else if(!strcmp(value,"tkip"))
		strncat(mid_value, "TKIP", strlen("TKIP"));
	else if(!strcmp(value,"tkip+aes"))
		strncat(mid_value, "AESTKIP", strlen("AESTKIP"));
	string_cat(result,"super-encrypt",mid_value);
	strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_WPA_PSK1, value);//密码
	_GET_VALUE(WLN0_MODE, value);//模式
	if(strcmp(value,"ap") != 0)
		get_wl0_passwd(SSID1_passwd);
	else
		sprintf(SSID1_passwd,"%s",nvram_safe_get(WLN0_WPA_PSK1));
	char temp_pwd[256]={0};	
	string_cat(result,"super-pwd",encodeSSID(SSID1_passwd,temp_pwd));
	strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_WDS_MAC_LIST, value);//mac
	string_cat(result,"super-mac",nvram_safe_get(WLN0_WDS_MAC_LIST));
	strncat(result, ",", 1);


	strncat(result, "\"networks\"", strlen("\"networks\""));
	strncat(result, ":", 1);
	strncat(result, "[", 1);

	websWrite(wp, result);
	websWrite(wp, T("]}\n"));
	websDone(wp,200);

	return ;
}
/************************************************************/

void formGetWifiRelayAP(webs_t wp, char_t *path, char_t *query)
{
	char *out = NULL;
	char *value = NULL;
	cJSON *root = NULL;
	char_t result[1024];
	char_t value1[128]={0};
	char_t SSID1_passwd[128] = {0};

	//websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	char_t unit[]="0";
	
	copy_wl_index_to_unindex(unit);

	root = cJSON_CreateObject();

	memset(result,0,sizeof(result));
	
	_GET_VALUE(WLN0_MODE, value);//模式
	IFVALUE(value);
	char mid_value[25] = {0};
	//strncat(result, "{", 1);
	
	//add by ll
	cJSON_AddStringToObject(root, "wifiRelayPower", get_product_pwr_info());
	//string_cat(result,"wifi-power", get_product_pwr_info());
	//strncat(result, ",", 1);
	//end by ll

	if(!strcmp(value,"sta"))
		strcpy(mid_value,"1");
	else if(!strcmp(value,"wet"))
		strcpy(mid_value,"2");
	else
		strcpy(mid_value,"0");
	cJSON_AddStringToObject(root, "wifiRelayType", mid_value);
	//string_cat(result,"repeat-mode",mid_value);
	//strncat(result, ",", 1);	

	cJSON_AddStringToObject(root, "wifiRelayErrorInfo", nvram_safe_get("err_check"));
	//string_cat(result,"error-info",nvram_safe_get("err_check"));
	//strncat(result, ",", 1);	

	char temp_ssid[256]={0};
	//hqw add for gbl2312 to utf-8
	_GET_VALUE("wl0_ssid", value);
#ifdef __CONFIG_SUPPORT_GB2312__
	if (1 == is_cn_encode(value))
	{
		char_t mib_value[49] = {0};
		char_t tmp_ssid[49] = {0};
		char_t *tmp_value = tmp_ssid;
		strcpy(mib_value, value);
		set_cn_ssid_encode("utf-8", mib_value, tmp_ssid);
		//printf("Decoded(to %s) ssid:%s\n","utf-8", tmp_ssid);
		value = tmp_value;
	}
	//end
#endif
	cJSON_AddStringToObject(root, "wifiRelaySSID", encodeSSID(value,temp_ssid));
	//string_cat(result,"super-ssid",encodeSSID(value,temp_ssid));
	//strncat(result, ",", 1);

	cJSON_AddStringToObject(root, "wifiRelaySSID", nvram_safe_get(WLN0_CHANNEL));
	//string_cat(result,"super-channel",nvram_safe_get(WLN0_CHANNEL)) ;
	//strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_SECURITY_TYPE, value);//加密方式
	if(!strcmp(value,"0"))
		strcpy(mid_value,"OPEN/");
	else if(!strcmp(value,"1"))
		strcpy(mid_value,"SHARED/");
	else if(!strcmp(value,"psk"))
		strcpy(mid_value,"WPA/");
	else if(!strcmp(value,"psk2"))
		strcpy(mid_value,"WPA2/");
	else if(!strcmp(value,"psk psk2"))
		strcpy(mid_value,"WPAWPA2/");
	else
		strcpy(mid_value,"NONE");
	
	_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	if(!strcmp(value,"wep"))
		strncat(mid_value, "WEP", strlen("WEP"));
	else if(!strcmp(value,"aes"))
		strncat(mid_value, "AES", strlen("AES"));
	else if(!strcmp(value,"tkip"))
		strncat(mid_value, "TKIP", strlen("TKIP"));
	else if(!strcmp(value,"tkip+aes"))
		strncat(mid_value, "AESTKIP", strlen("AESTKIP"));
	cJSON_AddStringToObject(root, "wifiRelaySecurityMode", mid_value);
	//string_cat(result,"super-encrypt",mid_value);
	//strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_WPA_PSK1, value);//密码
	_GET_VALUE(WLN0_MODE, value);//模式
	if(strcmp(value,"ap") != 0)
		get_wl0_passwd(SSID1_passwd);
	else
		sprintf(SSID1_passwd,"%s",nvram_safe_get(WLN0_WPA_PSK1));
	char temp_pwd[256]={0}; 
	cJSON_AddStringToObject(root, "wifiRelaySecurityMode", encodeSSID(SSID1_passwd,temp_pwd));
	//string_cat(result,"super-pwd",encodeSSID(SSID1_passwd,temp_pwd));
	//strncat(result, ",", 1);
	
	_GET_VALUE(WLN0_WDS_MAC_LIST, value);//mac
	cJSON_AddStringToObject(root, "wifiRelaySecurityMode", value);
	//string_cat(result,"super-mac",nvram_safe_get(WLN0_WDS_MAC_LIST));
	//strncat(result, ",", 1);

	out = cJSON_Print(root);
	cJSON_Delete(root);
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp,T("%s"),out);
	free(out);
	websDone(wp, 200);

	return ;

}


void formGetWifiRelay(webs_t wp, char_t *path, char_t *query)
{
	int i = 0;
	char *out = NULL;
	char *value = NULL;
	cJSON *root = NULL;
	unsigned char macstr[18];
	char ssid[256],channe[4], wep[32], sig[8],cipher[16],sec[64];
	char_t temSSID[256]={0};
	char_t value1[128]={0};
	char SSID1_name[128] = {0}, SSID1_passwd[128] = {0};


	root = cJSON_CreateObject();
	
	copy_wl_index_to_unindex("0");

	_GET_VALUE(WLN0_MODE, value);//模式
	IFVALUE(value);
	char mid_value[25] = {0};
	if(!strcmp(value,"sta")){
		strcpy(mid_value,"wisp");
	}
	else if(!strcmp(value,"wet")){
		strcpy(mid_value,"client+ap");
	}
	else{
		strcpy(mid_value,"ap");
	}
	cJSON_AddStringToObject(root, "wifiRelayType", mid_value);

	if(!strcmp(mid_value,"ap")){
		if(strcmp(nvram_safe_get("wl0_radio"),"0") == 0){
			snprintf(mid_value , sizeof(mid_value) ,  "%s" , "false" );
		}
		else{
			snprintf(mid_value , sizeof(mid_value) ,  "%s" , "true" );
		}

	}else {
		if(strcmp(nvram_safe_get("wl0.1_radio"),"0") == 0){
			snprintf(mid_value , sizeof(mid_value) ,  "%s" , "false" );
		}
		else{
			snprintf(mid_value , sizeof(mid_value) ,  "%s" , "true" );
		}
	}

	
	cJSON_AddStringToObject(root, "wifiEn", mid_value);
	

	char temp_ssid[256]={0};
	_GET_VALUE("wl0_ssid", value);
#ifdef __CONFIG_SUPPORT_GB2312__
	if (1 == is_cn_encode(value))
	{
		char_t mib_value[49] = {0};
		char_t tmp_ssid[49] = {0};
		char_t *tmp_value = tmp_ssid;
		strcpy(mib_value, value);
		set_cn_ssid_encode("utf-8", mib_value, tmp_ssid);
		//printf("Decoded(to %s) ssid:%s\n","utf-8", tmp_ssid);
		value = tmp_value;
	}
#endif
	cJSON_AddStringToObject(root, "wifiRelaySSID", encodeSSID(value,temp_ssid));
	cJSON_AddStringToObject(root, "wifiRelayChannel", nvram_safe_get(WLN0_CHANNEL));

	
	_GET_VALUE(WLN0_SECURITY_TYPE, value);//加密方式
	if(!strcmp(value,"0"))
		strcpy(mid_value,"OPEN-");
	else if(!strcmp(value,"1"))
		strcpy(mid_value,"SHARED-");
	else if(!strcmp(value,"psk"))
		strcpy(mid_value,"WPA-");
	else if(!strcmp(value,"psk2"))
		strcpy(mid_value,"WPA2-");
	else if(!strcmp(value,"psk psk2"))
		strcpy(mid_value,"WPAWPA2-");
	else
		strcpy(mid_value,"NONE");
	
	_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	if(!strcmp(value,"wep"))
		strncat(mid_value, "WEP", strlen("WEP"));
	else if(!strcmp(value,"aes"))
		strncat(mid_value, "AES", strlen("AES"));
	else if(!strcmp(value,"tkip"))
		strncat(mid_value, "TKIP", strlen("TKIP"));
	else if(!strcmp(value,"tkip+aes"))
		strncat(mid_value, "AESTKIP", strlen("AESTKIP"));
	cJSON_AddStringToObject(root, "wifiRelaySecurityMode", mid_value);

	
	_GET_VALUE(WLN0_WPA_PSK1, value);//密码
	_GET_VALUE(WLN0_MODE, value);//模式
	if(strcmp(value,"ap") != 0)
		get_wl0_passwd(SSID1_passwd);
	else
		sprintf(SSID1_passwd,"%s",nvram_safe_get(WLN0_WPA_PSK1));
	char temp_pwd[256]={0}; 
	cJSON_AddStringToObject(root, "wifiRelayPwd", encodeSSID(SSID1_passwd,temp_pwd));

	
	_GET_VALUE(WLN0_WDS_MAC_LIST, value);//mac
	cJSON_AddStringToObject(root, "wifiRelayMAC", value);

	out = cJSON_Print(root);
	cJSON_Delete(root);
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp,T("%s"),out);
	free(out);
	websDone(wp, 200);
EXIT:
	static_wds_scan = callwdsscan;
	return ;

}
void formGetWifiScan(webs_t wp, char_t *path, char_t *query)
{
	char *out = NULL;
	char *value = NULL;
	cJSON *root = NULL;
	cJSON *array = NULL;
	cJSON *obj = NULL;
	
	int i = 0, iflag = 0, retry=0;
	unsigned char macstr[18];
	char ssid[52],channe[4], wep[32], sig[8],cipher[16],sec[64];	//hong modify ssid
	wds_ap_list_info_t *wdsaplist, *ap;
	char_t temSSID[256]={0};

	if(static_wds_scan == uncallwdsscan)
		return;
	
	static_wds_scan = uncallwdsscan;

recreate:		
	if( NULL == wds_enr_create_aplist()  &&
		retry++ < WDS_ENR_SCAN_RETRY_TIMES && wds_list_entry_number <=0){
		diag_printf("wds retry=%d\n",retry);
		goto recreate;
	}

	wdsaplist = ap_wds_list;
	
	if(wds_list_entry_number==0){	//add by stanley 2010/12/08
		websWrite(wp, T("stanley\n"));
		websDone(wp,200);
		goto EXIT;
	}
	
	if(0 == wds_enr_display_aplist(wdsaplist)){
		websWrite(wp, T("stanley\n"));
		websDone(wp,200);
		goto EXIT;

	}

	/**********hong add for full-width character************/
#ifdef __CONFIG_SUPPORT_GB2312__
	encode_aplist_ssid_2_utf8(wdsaplist);
#endif

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "wifiScan", array = cJSON_CreateArray());
	
	ap = wdsaplist;
	while(ap->used == TRUE ) 
	{
		sprintf((char *)macstr, "%02X:%02X:%02X:%02X:%02X:%02X",
			ap->BSSID[0], ap->BSSID[1], ap->BSSID[2],
			ap->BSSID[3], ap->BSSID[4], ap->BSSID[5]);
		sprintf((char *)ssid, "%s", ap->ssid);
		sprintf((char *)channe, "%d", ap->channel);
		switch(ap->wep){
			case 0:
					sprintf((char *)wep, "%s", "NONE");
					break;
			case 1:
					sprintf((char *)wep, "%s", "WPA");
					break;
			case 2:
					sprintf((char *)wep, "%s", "WPA2");
					break;
			case 3:
					sprintf((char *)wep, "%s", "WPAWPA2");
					break;
			case 4:
					sprintf((char *)wep, "%s", "WEP");
					break;				
			default:
					sprintf((char *)wep, "%s", "UNKNOW");
					break;		
		}
		if(ap->wep != 0 && ap->wep != 4){
			switch(ap->cipher){
				case 1:
						sprintf((char *)cipher, "%s", "AES");
						break;
				case 2:
						sprintf((char *)cipher, "%s", "TKIP");
						break;
				case 3:
						sprintf((char *)cipher, "%s", "AESTKIP");
						break;			
				default:
						sprintf((char *)cipher, "%s", "UNKNOW");
						break;		
			}			
			sprintf((char *)sec,"%s/%s",wep,cipher);		
		}
		else			
			sprintf((char *)sec,"%s",wep);		

		if(ap->sig < 0)
			sprintf((char *)sig, "%d\%", -(ap->sig));
		else
			sprintf((char *)sig, "%d\%", (ap->sig));
		
		if (! iflag) {
			memset(temSSID,0,sizeof(temSSID));
			//websWrite(wp, T("%s\t%s\t%s\t%s\t%s"), encodeSSID(ssid,temSSID), macstr, channe, sec/*wep*/, sig);
			iflag = 1;
			cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
			cJSON_AddStringToObject(obj, "wifiScanSSID", encodeSSID(ssid,temSSID));
			cJSON_AddStringToObject(obj, "wifiScanMAC", macstr);
			cJSON_AddStringToObject(obj, "wifiScanChannel", channe);
			cJSON_AddStringToObject(obj, "wifiScanSecurityMode", sec);
			cJSON_AddStringToObject(obj, "wifiScanSignalStrength", sig);
		} else {
			memset(temSSID,0,sizeof(temSSID));
			//websWrite(wp, T("\r%s\t%s\t%s\t%s\t%s"), encodeSSID(ssid,temSSID), macstr, channe, sec/*wep*/, sig);
			cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
			cJSON_AddStringToObject(obj, "wifiScanSSID", encodeSSID(ssid,temSSID));
			cJSON_AddStringToObject(obj, "wifiScanMAC", macstr);
			cJSON_AddStringToObject(obj, "wifiScanChannel", channe);
			cJSON_AddStringToObject(obj, "wifiScanSecurityMode", sec);
			cJSON_AddStringToObject(obj, "wifiScanSignalStrength", sig);
		}
		ap++;
		i++;
	}
	wds_ap_num = i;
	
	out = cJSON_Print(root);
	//printf("[ httpd ]------out:%s\n", out);
	cJSON_Delete(root);
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	
	websWriteLongString(wp, out);
	
	free(out);
	websDone(wp, 200);
	
EXIT:
	static_wds_scan = callwdsscan;
	return ;

}


