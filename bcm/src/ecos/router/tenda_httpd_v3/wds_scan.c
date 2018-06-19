#if 0
#include <bcmutils.h>
#include <wlioctl.h>
#include <shutils.h>
#include <proto/802.11.h>
#include <proto/ethernet.h>
#endif
#include "wds_scan.h"


extern int tenda_dump_bss_info(wl_bss_info_t *bi);
extern int tenda_dump_bss_crypto(wl_bss_info_t *bi);

static wds_ap_list_info_t ap_wds_list[WDS_ENR_MAX_AP_SCAN_LIST_LEN];
static char wds_scan_result[WDS_ENR_DUMP_BUF_LEN]; 
static int /*wds_scanned = 0,*/ wds_ap_num = 0;

extern void udelay(int delay);
#define SLEEP(n)	cyg_thread_delay(n * 100)
#define USLEEP(X)	udelay(X)


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
	params->active_time = 80;
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

	SLEEP(2);
	
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
#if 1
	else if (list->version != WL_BSS_INFO_VERSION &&
			list->version != LEGACY_WL_BSS_INFO_VERSION) {
			diag_printf( "Sorry, your driver has bss_info_version %d "
		    	"but this program supports only version %d.\n",
		   	 list->version, WL_BSS_INFO_VERSION); 
		return NULL;
	}
#endif

	bi = list->bss_info;
	diag_printf("wds_ap_count=%d\n",list->count);
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
				//diag_printf("[%d]ssid:%s;	                 channel:%d\n", wds_ap_count,  ap_wds_list[wds_ap_count].ssid, bi->ctl_ch);
				//diag_printf("%d\n",  (uint8)(bi->chanspec & WL_CHANSPEC_CHAN_MASK));
				//diag_printf("bi->RSSI=%d\n",bi->RSSI );
				wds_ap_count++;
				wds_list_entry_number=wds_ap_count;
				
			}
		}
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}

	/*wds_scanned = 1;*/
	return ap_wds_list;
}

#define callwdsscan 1
#define uncallwdsscan 0
static int static_wds_scan = callwdsscan;

/*static int ej_wds_enr_scan_result(int eid, webs_t wp, int argc, char_t **argv)*/
void wireless_wds_scan(webs_t wp, char_t *path, char_t *query)
{
	int i = 0, iflag = 0, retry=0;
	unsigned char macstr[18];
	char ssid[52],channe[4], wep[32], sig[8],cipher[16],sec[64];
	wds_ap_list_info_t *wdsaplist, *ap;
	char_t temSSID[256]={0};

	if(static_wds_scan == uncallwdsscan)
		return;
	
	static_wds_scan = uncallwdsscan;

//	if (/*!wds_scanned &&*/ nvram_match( "wl0_wds_timeout", "0" )) {
		//diag_printf("wl0_wds_timeout=0, The AP disable wds!\n");
//		nvram_set("wl0_wds_timeout", "1");	/*add by stanley 2010/10/14*/
//		wds_ap_num = 0;
		//return ;
//	}
//	else /*if (!wds_scanned)*/{
recreate:		
		if( NULL == wds_enr_create_aplist()  &&
			retry++ < WDS_ENR_SCAN_RETRY_TIMES && wds_list_entry_number <=0){
			diag_printf("wds retry=%d\n",retry);
			goto recreate;
		}
//	}

	wdsaplist = ap_wds_list;
	
	if(wds_list_entry_number==0){	//add by stanley 2010/12/08
		websWrite(wp, T("stanley\n"));
		websDone(wp,200);
		goto out;
	}
	
//	wds_enr_get_aplist(wdsaplist, wdsaplist);
	if(0 == wds_enr_display_aplist(wdsaplist)){
		websWrite(wp, T("stanley\n"));
		websDone(wp,200);
		goto out;

	}

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
		
		sprintf((char *)sig, "%d\%", (ap->sig));

		//diag_printf("%s,%s,%s,%s,%s\n", ssid, macstr, channe, wep, sig);
		
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
