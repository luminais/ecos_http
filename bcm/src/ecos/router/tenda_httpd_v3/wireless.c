
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kautoconf.h>//in bsp/include

#include "route_cfg.h"
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"

static int need_prompt_pppoe=1;
static int need_prompt_wlpwd=1;
#define IFVALUE(value)	 if(value==NULL)	\
							value=""	
#define WL_RADIO_ON		1
#define WL_RADIO_OFF		0  
							
#ifdef __CONFIG_WL_LED__
extern void wl_led_test_on(void);
extern void wl_led_test_off(void);
#endif
#ifdef __CONFIG_WPS_LED__
extern void wps_led_test_off();
extern void wps_led_test_on();
#endif
void wireless_asp_define(void);
void wireless_form_define(void);
static void clear_security_parameter(void);

static int getwireless11bchannels(int eid, webs_t wp, int argc, char_t **argv);
static int getwireless11gchannels(int eid, webs_t wp, int argc, char_t **argv);
static int getwirelessmac(int eid, webs_t wp, int argc, char_t **argv);
static int get_wireless_basic(int eid, webs_t wp, int argc, char_t **argv);
static int get_wireless_adv(int eid, webs_t wp, int argc, char_t **argv);
static int get_wps_configured(int eid, webs_t wp, int argc, char_t **argv);
static int get_wireless_wds(int eid, webs_t wp, int argc, char_t **argv);
static int get_wireless_filter(int eid, webs_t wp, int argc, char_t **argv);
static int get_wireless_wps(int eid, webs_t wp, int argc, char_t **argv);

static int 	get_wireless_password(int eid, webs_t wp, int argc, char_t **argv);
static void init_wireless_security(webs_t wp, char_t *path, char_t *query);
static void get_wireless_security(webs_t wp, char_t *path, char_t *query);

static void init_wireless_basic(webs_t wp, char_t *path, char_t *query);	
static void set_wireless_basic(webs_t wp, char_t *path, char_t *query);
static void set_wireless_adv(webs_t wp, char_t *path, char_t *query);
static void set_wireless_security(webs_t wp, char_t *path, char_t *query);
static void set_wireless_filter(webs_t wp, char_t *path, char_t *query);
static void set_wireless_oob(webs_t wp, char_t *path, char_t *query);

#ifdef CONFIG_WL_USE_APSTA
static void set_wireless_wire_mode(webs_t wp, char_t *path, char_t *query);
static void get_wireless_sta_info(webs_t wp, char_t *path, char_t *query);
static void set_wireless_unit(webs_t wp, char_t *path, char_t *query);
static void set_universal_repeater(webs_t wp, char_t *path, char_t *query);
static void init_universal_repeater(webs_t wp, char_t *path, char_t *query);
#endif

extern void wireless_wds_scan(webs_t wp, char_t *path, char_t *query);
extern void  set_wireless_wps(webs_t wp, char_t *path, char_t *query);
extern void  wps_oob(const char *flag);
extern int get_wireless_station(int eid, webs_t wp, int argc, char_t **argv);
extern int wps_is_enable(void);
extern void tenda_stop_wps_timer(void);

extern void copy_wl_index_to_unindex(char *unit_str);
extern void validate_vif_ssid(char *vif_ssid);
extern void wl_unit(char *unit,int from_secu);
extern char *get_vif_ssid();
extern int is_wl0_vifs(char *unit_str);
extern char *get_wl0_ssid();


static void set_wireless_extra(webs_t wp, char_t *path, char_t *query);


void wireless_asp_define(void){
	websAspDefine(T("get_wireless_basic"), get_wireless_basic);
	////websAspDefine(T("getwireless11bchannels"), getwireless11bchannels);
	////websAspDefine(T("getwireless11gchannels"), getwireless11gchannels);
	websAspDefine(T("getwirelessmac"), getwirelessmac);
	
	websAspDefine(T("get_wireless_adv"), get_wireless_adv);
	websAspDefine(T("isWPSConfiguredASP"), get_wps_configured);
	websAspDefine(T("get_wireless_wds"), get_wireless_wds);
	websAspDefine(T("get_wireless_filter"), get_wireless_filter);
	websAspDefine(T("get_wireless_wps"), get_wireless_wps);
	websAspDefine(T("get_wireless_station"), get_wireless_station);
	websAspDefine(T("get_wireless_password"), get_wireless_password);
	return ;
}

void wireless_form_define(void){
	
	websFormDefine(T("wirelessInitBasic"), init_wireless_basic); 	
	websFormDefine(T("wirelessBasic"), set_wireless_basic); 
	
	websFormDefine(T("wirelessAdvanced"), set_wireless_adv); 
	websFormDefine(T("wirelessInitSecurity"), init_wireless_security);
	websFormDefine(T("wirelessGetSecurity"), get_wireless_security);
	websFormDefine(T("wirelessSetSecurity"), set_wireless_security);
#if 0	
	websFormDefine(T("wirelessWDS"), set_wireless_wds);
#endif
	websFormDefine(T("WDSScan"), wireless_wds_scan);
	websFormDefine(T("WlanMacFilter"), set_wireless_filter);
	websFormDefine(T("WPS"), set_wireless_wps);
	websFormDefine(T("OOB"), set_wireless_oob);
#ifdef CONFIG_WL_USE_APSTA

	#if 0  //add by z10312 整合 无线 wan (wds, wisp, ap-client)
	websFormDefine(T("wirelessMode"), set_wireless_wire_mode);
	#else
	websFormDefine(T("wirelessMode"), set_wireless_extra);
	#endif
	
	websFormDefine(T("wirelessInitMode"), get_wireless_sta_info);
	websFormDefine(T("setUniversalRepeater"), set_universal_repeater);	
	websFormDefine(T("initUniversalRepeater"), init_universal_repeater);
	websFormDefine(T("onSSIDChange"), set_wireless_unit);
#endif
	return ;
}

 //add by liangia SSID全字符支持
 char_t* encodeSSID(char_t *SSID,char_t *buf)
 {
 	//buf[>256]避免全特殊字符时数组越界
	char_t *ch = SSID;

	while (*ch != '\0'){
		
		//将SSID 特殊字符进行实体编码
		switch(*ch)
		{
			case '"':
				strcat(buf,"&#34;");
				break;
			case '%':
				strcat(buf,"&#37;");
				break;
			case '&':
				strcat(buf,"&#38;");
				break;
			case '\'':
				strcat(buf,"&#39;");
				break;
			case '<':
				strcat(buf,"&#60;");
				break;
			case '>':
				strcat(buf,"&#62;");
				break;
			case '\\':
				strcat(buf,"&#92;");
				break;
			case ' '://使HTML标签中空格不合并；但不能用&#160;保存会乱码
				strcat(buf,"&#160;");
				break;
			
			default:
				sprintf(buf,"%s%c",buf,*ch);
			
		}
		
		ch++;	
	}
	//diag_printf("buf=%s\n", buf);
	return buf;
}

int get_wireless_basic(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name=NULL;
	char_t *value=NULL;
	char_t *tempwide = NULL ;
	char_t temSSID[256] = {0};
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}
	
	/*
	 * WirelessMode----->  0:bg mode; 1:b mode; 4:g mode; 9: bgn mode.
	 * WLN0_G_MODE----> 1:bg mode; 0:b mode; 2:g mode.
	 * WLN0_N_MODE----> -1: Auto 0: off
	 */
	if(0 == strncmp(name, "WirelessMode", strlen("WirelessMode"))){
	
		if(0 == strcmp(_GET_VALUE(WLN0_N_MODE,value), "-1")){//11bgn
			websWrite(wp, T("%s"), "9");
		}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "1")){//11b
			websWrite(wp, T("%s"), "0");
		}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "0")){//11bg
			websWrite(wp, T("%s"), "1");
		}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "2")){//11g
			websWrite(wp, T("%s"), "4");
		}
		return 0;
		
	}else if(0 ==  strcmp(name, "SSID")){
		_GET_VALUE("wl0_ssid", value);
		IFVALUE(value);
		value = encodeSSID(value,temSSID);
		//diag_printf("get_wireless_basic_SSID=%s\n", value);
	}else if(0 ==  strcmp(name, "SSID1")){
		
		if((value = get_vif_ssid()) != NULL){
			value = encodeSSID(value,temSSID);
			//diag_printf("get_wireless_basic_SSID1=%s\n", value);
			websWrite(wp, T("%s"), value);
		}else{
			websWrite(wp, T("%s"), "");
		}
		return 0;
	}
	else if(0 == strncmp(name, "HideSSID", strlen("HideSSID"))){
		_GET_VALUE(WLN0_HIDE_SSID0, value);
		
	}else if(0 == strncmp(name, "Channel", strlen("Channel"))){
		_GET_VALUE(WLN0_CHANNEL, value);
		
	}else if(0 == strncmp(name, "CountryCode", strlen("CountryCode"))){
		_GET_VALUE(WLN0_COUNTRY, value);
		
	}else if(0 == strncmp(name, "HT_OpMode", strlen("HT_OpMode"))){
		_GET_VALUE(WLN0_HT_MODE, value); //???
		
	}else if(0 == strncmp(name, "HT_BW", strlen("HT_BW"))){
		/*
		  * HT_BW---------> 0: 20MHz; 1:20/40MHz
		  * WLN0_HT_BW---->0: 20MHz; 1: 40MHz ; 2: 20/40MHz
		  */
		 
		_GET_VALUE(WLN0_HT_BW, value);
		_GET_VALUE(WLN0_HT_BW_FAKE_AUTO, tempwide);
		if(strcmp("1" , tempwide) == 0 )
		{
			strcpy(value , "2") ;
		}
	}else if(0 == strncmp(name, "HT_GI", strlen("HT_GI"))){


	}else if(0 == strncmp(name, "HT_MCS", strlen("HT_MCS"))){
		_GET_VALUE(WLN0_HT_MCS, value);

		
	}else if(0 == strncmp(name, "HT_RDG", strlen("HT_RDG"))){
		
		
	}else if(0 == strncmp(name, "HT_EXTCHA", strlen("HT_EXTCHA"))){
		/*
		  * WLN0_HT_EXTCHA-----> "none", "lower", "upper"
		  * HT_EXTCHA-----------> 1: lower; 0: upper
		  */
		_GET_VALUE(WLN0_HT_EXTCHA, value);		
	}else if(0 == strncmp(name, "HT_AMSDU",strlen("HT_AMSDU"))){
		_GET_VALUE(WLN0_HT_AMSDU, value);
	}else if(0 == strncmp(name, "WirelessEnable", strlen("WirelessEnable"))){
		/*
		 * 0:disable; 1: enable 
		 */
		_GET_VALUE(WLN0_WIRELESS_ENABLE, value);	
	}else if(0 == strncmp(name, "Mode", strlen("Mode"))){
		_GET_VALUE(WLN0_MODE, value);

	}else if(0 == strncmp(name, "WirelessT", strlen("WirelessT"))){
		_GET_VALUE(WLN0_APORWDS, value);	//0: ap;   1:wds
	}
#ifdef CONFIG_WL_USE_APSTA	
	else if(0 == strncmp(name,"SsidList",strlen("SsidList"))){
		char result_ssid[512];
		memset(result_ssid,0,sizeof(result_ssid));
		
		value = get_wl0_ssid();
		IFVALUE(value);
		encodeSSID(value,result_ssid);
		//diag_printf("primary_SSID=%s\n",result_ssid);
		//strncat(result_ssid, value, 512);

		if((value = get_vif_ssid()) != NULL){
			strncat(result_ssid, "\t", 512);
			encodeSSID(value,result_ssid);
			//diag_printf("secondary_SSID=%s\n",result_ssid);
			//strncat(result_ssid, value, 256);
		}
		value = result_ssid;
	}else if(0 == strncmp(name,"Cur_wl_unit",strlen("Cur_wl_unit"))){
		_GET_VALUE(WLN0_UNIT, value);
		IFVALUE(value);
		if(is_wl0_vifs(value))
			value =  "1";
		else
			value =  "0";
	}else if(0 == strncmp(name,"Cur_wl_mode",strlen("Cur_wl_mode"))){
		_GET_VALUE(WLN0_MODE, value);
		IFVALUE(value);
	}
#endif
	//diag_printf("%s=%s\n", name, value);
	
	websWrite(wp, T("%s"), value);
	return 0;
}

static int getwirelessmac(int eid, webs_t wp, int argc, char_t **argv)
{
	char if_hw[20] = {0};
	struct ifreq ifr;
	unsigned char *ptr=NULL;
	int skfd;

	memset(if_hw, 0, 18);

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		diag_printf("====%s\n", __FUNCTION__);
		perror("socket");
		return 0;
	}

	/* hardware address */
	strncpy(ifr.ifr_name, "eth0", IF_NAMESIZE);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) 
	{
		perror("SIOCGIFHWADDR");
		return 0;
	}

	ptr = (char *)&ifr.ifr_addr.sa_data;
	sprintf(if_hw, "%02X:%02X:%02X:%02X:%02X:%02X",
			(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
			(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

	(void) close(skfd);
	
	websWrite(wp, T("%s"), if_hw);
	return 0;
}

static int   initwireless11bchannels(void){
	int idx = 0, channel;
	char_t *value=NULL, *channel_s=NULL;
	
	_GET_VALUE(WLN0_COUNTRY, value);
	_GET_VALUE(WLN0_CHANNEL, channel_s);

	channel = (channel_s == NULL)? 0 : atoi(channel_s);
	if ((value == NULL) || (strcmp(value, "") == 0) ||
		(strcmp(value, "US") == 0) ) {
			idx = 11;
	}

	if ((value == NULL) || (strcmp(value, "") == 0) ||
		(strcmp(value, "TW") == 0) ||
		(strcmp(value, "FR") == 0) ||
		(strcmp(value, "IE") == 0) || (strcmp(value, "CN") == 0) ||
		(strcmp(value, "HK") == 0)  || (strcmp(value, "ALL") == 0)) {
			idx=13;				  
	}

	if ((value == NULL) || (strcmp(value, "") == 0) ||
		(strcmp(value, "JP") == 0)) {
			idx=14;
	}

	return idx;
}
/*
 * description: write 802.11g channels in <select> tag
 */
static int initwireless11gchannels(void )
{
	int idx = 0, channel;
	char *value=NULL, *channel_s=NULL;

	_GET_VALUE(WLN0_COUNTRY, value);
	_GET_VALUE(WLN0_CHANNEL, channel_s);

	channel = (channel_s == NULL)? 0 : atoi(channel_s);
	if ((value == NULL) || (strcmp(value, "") == 0) ||
		(strcmp(value, "US") == 0) ) {
			idx=11;
	}
	if ((value == NULL) || (strcmp(value, "") == 0) ||
		(strcmp(value, "JP") == 0) || (strcmp(value, "TW") == 0) ||
		(strcmp(value, "FR") == 0) || (strcmp(value, "IE") == 0) ||
		(strcmp(value, "CN") == 0) || (strcmp(value, "HK") == 0) || (strcmp(value, "ALL") == 0)) {
			idx=13;
	}
	
	if ((value == NULL) || (strcmp(value, "") == 0)) {
		idx=14;
	}
	
	return idx;
}
static void init_wireless_basic(webs_t wp, char_t *path, char_t *query){
	/*		WirelessWorkMode = str[0];
			wdsList = str[1];
			PhyMode = str[2];
			broadcastssidEnable = str[3];
			channel_index = str[4];
			countrycode = str[5];
			ht_bw = str[6];
			ht_extcha = str[7];
			Enablewireless = str[8];
			mode = str[9];
			wmmCapable = str[10];
			APSDCapable = str[11];*/
#if 1			
	char_t result[2048];
	char_t *value=NULL;
	char_t channel[4];
	char_t unit[]="0";
	char_t temSSID[256]={0};
	memset(result,0,sizeof(result));

	copy_wl_index_to_unindex(unit);
	
	_GET_VALUE(WLN0_APORWDS, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_WDS_LIST, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	if(0 == strcmp(_GET_VALUE(WLN0_N_MODE,value), "-1")){//11bgn
			strncat(result, "9", 2048);
	}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "1")){//11b
			strncat(result, "0", 2048);
	}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "0")){//11bg
			strncat(result, "1", 2048);
	}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "2")){//11g
			strncat(result, "4", 2048);
	}
	strncat(result, "\r", 2048);

	
	_GET_VALUE(WLN0_HIDE_SSID0, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_CHANNEL, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_COUNTRY, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	char_t *tempwide = NULL ;

	_GET_VALUE(WLN0_HT_BW, value);
	_GET_VALUE(WLN0_HT_BW_FAKE_AUTO, tempwide);
	if(strcmp("1" , tempwide) == 0 )
	{
		strcpy(value , "2") ;
	}
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_HT_EXTCHA, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_WIRELESS_ENABLE, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_MODE, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_WMM_CAPABLE, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_APSD_CAPABLE, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_SSID0, value);
	IFVALUE(value);
	value = encodeSSID(value,temSSID);
	//diag_printf("init_wireless_basic_SSID=%s\n",value);

	
#ifdef CONFIG_WL_USE_APSTA
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	memset(temSSID,0,sizeof(temSSID));
	value = get_vif_ssid();
	IFVALUE(value);
	value = encodeSSID(value,temSSID);
	//diag_printf("init_wireless_basic_SSID1=%s\n",value);
	
#endif

	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	channel[0] ="\0";
	sprintf(channel, "%d", initwireless11bchannels());
	strncat(result, channel, 2048);
	strncat(result, "\r", 2048);

	channel[0] ="\0";
	sprintf(channel, "%d", initwireless11gchannels());
	strncat(result, channel, 2048);
	strncat(result, "\r", 2048);
	
#ifdef CONFIG_WL_USE_APSTA
	_GET_VALUE(WLN0_AP_ISOLATE, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
//是AP还是STA?
	_GET_VALUE(WLN0_MODE, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
#endif	

	strncat(result, "\n", 2048);
#else
	char_t result[2048],*p;
	char_t *value=NULL;
	char_t channel[4];
	char_t unit[]="0";
	int n;
	memset(result,0,sizeof(result));

	copy_wl_index_to_unindex(unit);

	p = result;
	//n = sprintf(p,"new Array(");
	//p = p+n;
	
	_GET_VALUE(WLN0_APORWDS, value);
	IFVALUE(value);
	n = sprintf(p,"\"%s\"",value);
	p = p+n;
	
	_GET_VALUE(WLN0_WDS_LIST, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	if(0 == strcmp(_GET_VALUE(WLN0_N_MODE,value), "-1")){//11bgn
			n = sprintf(p,",\"9\"");
			p = p+n;

	}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "1")){//11b
			n = sprintf(p,",\"0\"");
			p = p+n;
	}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "0")){//11bg
			n = sprintf(p,",\"1\"");
			p = p+n;
	}else if(0 == strcmp(_GET_VALUE(WLN0_G_MODE,value), "2")){//11g
			n = sprintf(p,",\"4\"");
			p = p+n;
	}
	
	_GET_VALUE(WLN0_HIDE_SSID0, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;
	
	_GET_VALUE(WLN0_CHANNEL, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_COUNTRY, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_HT_BW, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_HT_EXTCHA, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_WIRELESS_ENABLE, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_MODE, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_WMM_CAPABLE, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_APSD_CAPABLE, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	_GET_VALUE(WLN0_SSID0, value);
	IFVALUE(value);
	value = encodeSSID(value,temSSID);
	
	
#ifdef CONFIG_WL_USE_APSTA
	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	value = get_vif_ssid();
	memset(temSSID,0,sizeof(temSSID));
	IFVALUE(value);
	value = encodeSSID(value,temSSID);
#endif

	n = sprintf(p,",\"%s\"",value);
	p = p+n;

	channel[0] ="\0";
	sprintf(channel, "%d", initwireless11bchannels());
	n = sprintf(p,",\"%s\"",channel);
	p = p+n;

	channel[0] ="\0";
	sprintf(channel, "%d", initwireless11gchannels());
	n = sprintf(p,",\"%s\"",channel);
	p = p+n;
	
#ifdef CONFIG_WL_USE_APSTA
	_GET_VALUE(WLN0_AP_ISOLATE, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;
//是AP还是STA?
	_GET_VALUE(WLN0_MODE, value);
	IFVALUE(value);
	n = sprintf(p,",\"%s\"",value);
	p = p+n;
#endif	

	//n = sprintf(p,");");
	//p = p+n;
#endif
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	
	return ;
}

static void set_wireless_basic(webs_t wp, char_t *path, char_t *query)
{
	char_t *wirelessmode, *ssid, *hidessid,*wirelessenable, *sz11bChannel,*sz11gChannel;
	char_t *htopmode, *htbw, *htgi, *htmcs, *htrdg, *htextcha, *htamsdu;
/*add by stanley 2010/10/13*/
	char_t *aporwds;
	char_t  *wds_mode;
	char_t *wds_list, *wmmcapable, *apsdcapable;
	char_t *wl_mode;
	char_t *wps_enable;
	
#ifdef CONFIG_WL_USE_APSTA
	char_t *ssid_1;
	char_t *ap_isolate;
	char_t *ure_disable;
	int disble_ure;
	int wds_en,old_wds_en;

	char *wl0_ssid;					
	char *wan_ifnames = "wan_ifnames";	
	char *wan0_ifnames = "wan0_ifnames";		
	char *wan_ifname = "wan_ifname";						
	char_t *old_wds;
	char *wlunit="0";
	char *wlunit_vif="0.1";
#else
	char_t *wlunit;
#endif	
/*end*/	
	char *def_ssid;
	char *go = websGetVar(wp, T("GO"), T("wireless_basic.asp")); 

	/*0:bg mode; 1:b mode; 4:g mode; 9: bgn mode*/
	wirelessmode = websGetVar(wp, T("wirelessmode"), T("9")); 
	ssid = websGetVar(wp, T("ssid"), T(""));
	hidessid =  websGetVar(wp, T("broadcastssid"), T("0"));
	wirelessenable = websGetVar(wp, T("enablewirelessEx"), T("1"));	
	sz11bChannel = websGetVar(wp, T("sz11bChannel"), T("0"));
	sz11gChannel = websGetVar(wp, T("sz11gChannel"), T("0")); 
	htopmode = websGetVar(wp, T("n_mode"), T("0"));
	htbw = websGetVar(wp, T("n_bandwidth"), T("0"));
	htgi = websGetVar(wp, T("n_gi"), T("0"));
	htmcs = websGetVar(wp, T("n_mcs"), T("-1"));
	htrdg = websGetVar(wp, T("n_rdg"), T("0"));
	htextcha = websGetVar(wp, T("n_extcha"), T("0"));
	htamsdu = websGetVar(wp, T("n_amsdu"), T("auto"));
	
/*add by stanley 2010/10/13*/
	aporwds = websGetVar(wp, T("WirelessT"), T("0"));
	wds_mode = websGetVar(wp, T("wds_mode"), T("0"));
	wmmcapable = websGetVar(wp, T("wmm_capable"), T("off"));
	apsdcapable = websGetVar(wp, T("apsd_capable"), T("off"));
	wds_list = websGetVar(wp, T("wds_list"), T("")); 

#ifdef CONFIG_WL_USE_APSTA
	ssid_1 = websGetVar(wp, T("mssid_1"), T("")); 
	ap_isolate = websGetVar(wp, T("ap_isolate"), T("0")); 
/*end*/
	_GET_VALUE("ure_disable", ure_disable);
	disble_ure = atoi(ure_disable);// if disble_ure == 1, wl_ure is disable
	/*
	if(ssid_1 == ""){
		ssid_1 = NULL;
	}
	*/
#endif

	/* pxy ++, when user frequently click on save button in wireless_basic.asp,
	 * that will cause "wl_mode" to be NULL, do it like this, it can solve the problem.
	 */
	copy_wl_index_to_unindex("0");

	if (strcmp(wirelessenable, "0") == 0){
		_SET_VALUE(WLN0_WIRELESS_ENABLE, wirelessenable);
		
#ifdef __CONFIG_TENDA_HTTPD_V3__
		wl_change_states(WL_RADIO_OFF);
#endif
#ifdef __CONFIG_WL_LED__
		wl_led_test_off();		//add by stanley 2010/11/3
#endif
	}
/*else*/
	if(strcmp(wirelessenable, "1") == 0 ){
#ifdef __CONFIG_WL_LED__
		wl_led_test_on();						//add by stanley 2010/11/3
#endif
#ifdef __CONFIG_TENDA_HTTPD_V3__
		wl_change_states(WL_RADIO_ON);
#endif

		_SET_VALUE(WLN0_WIRELESS_ENABLE, wirelessenable);	
		
		wl_mode = nvram_get("wl_mode");
		
		if(strcmp(wl_mode, "sta") != 0)
		{
			if ( 0==strcmp(sz11bChannel, "0") && 0==strcmp(sz11gChannel, "0") ){	
				_SET_VALUE(WLN0_CHANNEL, "0");	
			}	
			if (0 != strcmp(sz11bChannel, "0") && strlen(sz11bChannel) != 0){
				_SET_VALUE(WLN0_CHANNEL, sz11bChannel);
			}else if (0 != strcmp(sz11gChannel, "0") && strlen(sz11gChannel) != 0){
				_SET_VALUE(WLN0_CHANNEL, sz11gChannel);	
			}	
		}

		
	
	/*
	 * WirelessMode----->  0:bg mode; 1:b mode; 4:g mode; 9: bgn mode.
	 * WLN0_G_MODE----> 1:bg mode; 0:b mode; 2:g mode.
	 * WLN0_N_MODE----> -1: Auto 0: off
	 */
	 	_SET_VALUE(WLN0_BASICRATE, "12");	//add by stanley for only G mode 2010/10/28	
		if(0 == strcmp(wirelessmode, "0")){//11bg
			_SET_VALUE(WLN0_G_MODE, "1");	
			_SET_VALUE(WLN0_N_MODE, "0");	
			_SET_VALUE(WLN0_PLCPHDR, "long"); //add by stanley for 150M /135M
		}else if(0 == strcmp(wirelessmode, "1")){//11b only
			_SET_VALUE(WLN0_G_MODE, "0");	
			_SET_VALUE(WLN0_N_MODE, "0");	
			_SET_VALUE(WLN0_PLCPHDR, "long"); //add by stanley for 150M /135M
		}else if(0 == strcmp(wirelessmode, "4")){//11g only
			_SET_VALUE(WLN0_G_MODE, "2");
			_SET_VALUE(WLN0_N_MODE, "0");
			_SET_VALUE(WLN0_BASICRATE, "default"); //add by stanley for only G mode 2010/10/28	
			//_SET_VALUE(WLN0_PLCPHDR, "long"); //add by stanley for 150M /135M	
			_SET_VALUE(WLN0_PLCPHDR, "short");//reference from wlioctl.h,roy modify
		}else if(0 == strcmp(wirelessmode, "9")){//11bgn	
			_SET_VALUE(WLN0_N_MODE, "-1");
			_SET_VALUE(WLN0_G_MODE, "1");	
			//_SET_VALUE(WLN0_PLCPHDR, "short"); //add by stanley for 150M /135M	
			_SET_VALUE(WLN0_PLCPHDR, "long");//reference from wlioctl.h,roy modify
		}

		_SET_VALUE(WLN0_HIDE_SSID0, hidessid);
//when bssid disable,we should set 	WLN0_PLCPHDR to long	
//		if(strcmp(hidessid,"1") == 0){
//			_SET_VALUE(WLN0_PLCPHDR, "long");
//		}


#ifdef CONFIG_WL_USE_APSTA
		if(wds_en == 1)
		{
			_SET_VALUE(WLN0_AP_ISOLATE,"0");
		}
		else
		{
			_SET_VALUE(WLN0_AP_ISOLATE,ap_isolate);
		}
			
#endif

		if(strlen(ssid) > 0) _SET_VALUE(WLN0_SSID0, ssid);	
		//add by ll
		if(atoi(htbw) == 0 )
		{
			_SET_VALUE(WLN0_HT_BW, "0");
			_SET_VALUE(WLN0_OBSS, "0");
			_SET_VALUE(WLN0_OBSS1, "0");
			_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"0");
		}
		else if(atoi(htbw) == 1)
		{
			_SET_VALUE(WLN0_HT_BW, "1");
			_SET_VALUE(WLN0_OBSS, "0");
			_SET_VALUE(WLN0_OBSS1, "0");
			_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"0");
		}
		else
		{
			_SET_VALUE(WLN0_HT_BW, "1");
			_SET_VALUE(WLN0_OBSS, "1");
			_SET_VALUE(WLN0_OBSS1, "1");
			_SET_VALUE(WLN0_HT_BW_FAKE_AUTO,"1");
		}
		
		//end by ll

		if(strcmp(htbw, "0") == 0 || nvram_match(WLN0_CHANNEL, "0")){						//add by stanley 2010/11/01
			_SET_VALUE(WLN0_HT_EXTCHA, "none");
		}else if(strcmp(htbw, "1") == 0 || strcmp(htbw, "2") == 0){			
			if(strcmp(htextcha, "0") == 0)
				_SET_VALUE(WLN0_HT_EXTCHA, "upper");
			else 
				_SET_VALUE(WLN0_HT_EXTCHA, "lower");	
		}
#if 0
		//add by ll
		else if(strcmp(htbw, "2") == 0)
		{
			if(atoi(nvram_safe_get(WLN0_CHANNEL)) >= 7 )
			{
				_SET_VALUE(WLN0_HT_EXTCHA, "upper");
				
				_SET_VALUE(WLN0_HT_EXTCHA1, "upper");
			}
			else
			{
				_SET_VALUE(WLN0_HT_EXTCHA, "lower");
				
				_SET_VALUE(WLN0_HT_EXTCHA1, "lower");
			}
		}
		//end by ll
#endif
		_SET_VALUE(WLN0_HT_AMSDU, htamsdu);

	
	/*add by stanley 2010/10/13*/
		_SET_VALUE(WLN0_WMM_CAPABLE, wmmcapable);
		if(strcmp(wmmcapable, "off") == 0)
			_SET_VALUE(WLN0_APSD_CAPABLE, "off");
		else
			_SET_VALUE(WLN0_APSD_CAPABLE, apsdcapable);

		_SET_VALUE(WLN0_APORWDS, aporwds);
		if(strcmp(aporwds,"1") == 0 ) //wds enable	
		{
			if (strlen(wds_list) > 0){
				wds_list[strlen(wds_list) - 1] = '\0';
				strtoupper(wds_list);
			}
			//repeater
			_SET_VALUE(WLN0_WDS_TIMEOUT, "1");
			//_SET_VALUE(WLN0_WDS_LAZY, "1");	//modify by stanley for more WDS point 2010/11/2
			//_SET_VALUE(WLN0_MODE, "ap");
			_SET_VALUE(WLN0_WDS_LIST, wds_list);	
		} else{						//close wds
			//if(nvram_match(WLN0_WDS_LAZY,"1")){
				_SET_VALUE(WLN0_WDS_TIMEOUT, "0");//add by stanley for close WDS	
				//_SET_VALUE(WLN0_WDS_LAZY, "0");
				//_SET_VALUE(WLN0_MODE, "ap");
				_SET_VALUE(WLN0_WDS_LIST, "");		//add by stanley for close WDS, this is bad way.
			//}	
		}
	}	
	_GET_VALUE(WLN0_UNIT,wlunit);
	wl_unit(wlunit,0);
	def_ssid = nvram_safe_get("default_ssid");
	if(strcmp(def_ssid,ssid) != 0)
	{
		nvram_set(WLN0_WPS_OOB, "disabled");
	}

#if 0
	//add by ll
	if(atoi(htbw) == 0 || atoi(htbw) == 1)
	{
		_SET_VALUE(WLN0_OBSS1, "0");
	}
	else
	{
		_SET_VALUE(WLN0_OBSS1, "1");
	}

	//end by ll
#endif
	
#ifdef CONFIG_WL_USE_APSTA	
	if(!strcmp(nvram_safe_get("wl0.1_ssid"), "") && strcmp("", ssid_1) != 0 
	   || strcmp(nvram_safe_get("wl0.1_ssid"), "") && strcmp(nvram_safe_get("wl0.1_ssid"), ssid_1))
	{
		nvram_set("lan1_wps_oob", "disabled");
	}
	if(strcmp(ssid_1,"") == 0)
		validate_vif_ssid(NULL);
	else
		validate_vif_ssid(ssid_1);
#endif

	_COMMIT();
	_RESTART();
	websRedirect(wp, go);
	
}

static int get_wireless_adv(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name=NULL;
	char_t *value=NULL;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}
	
	if(0 == strncmp(name, "BasicRate", strlen("BasicRate"))){
		_GET_VALUE(WLN0_BASICRATE, value);
		
	}else if(0 == strncmp(name, "BGProtection", strlen("BGProtection"))){
		_GET_VALUE(WLN0_G_PROTECT, value);
									
	}else if(0 == strncmp(name, "BeaconPeriod", strlen("BeaconPeriod"))){
		_GET_VALUE(WLN0_BEACON_PERIOD, value);
		
	}else if(0 == strncmp(name, "FragThreshold", strlen("FragThreshold"))){
		_GET_VALUE(WLN0_FRAG_THRESHOLD, value);

	}else if(0 == strncmp(name, "RTSThreshold", strlen("RTSThreshold"))){
		_GET_VALUE(WLN0_RTS_THRESHOLD, value);

	}else if(0 == strncmp(name, "WmmCapable", strlen("WmmCapable"))){
		_GET_VALUE(WLN0_WMM_CAPABLE, value);

	}else if(0 == strncmp(name, "APSDCapable", strlen("APSDCapable"))){
		_GET_VALUE(WLN0_APSD_CAPABLE, value);

	}/*else if(0 == strncmp(name, "TxPower", strlen("TxPower"))){
		_GET_VALUE(WLN0_RTS_THRESHOLD, value);

	}*/

	websWrite(wp, T("%s"), value);
	
	
	return 0;
}

static void set_wireless_adv(webs_t wp, char_t *path, char_t *query)
{
	char_t *basicrate, *bgprotection, *fragmentthreshold, *rtsthreshold;
	char_t *apsdcapable, *txpower, *wmmcapable, *beaconinterval;

	basicrate = websGetVar(wp, T("basic_rate"), T("default"));
	bgprotection = websGetVar(wp, T("bg_protection"), T("auto"));
	beaconinterval = websGetVar(wp, T("beacon"), T("100"));
	fragmentthreshold = websGetVar(wp, T("fragment"), T("2346"));
	rtsthreshold = websGetVar(wp, T("rts"), T("2347"));
	apsdcapable = websGetVar(wp, T("apsd_capable"), T("off"));
	txpower = websGetVar(wp, T("tx_power"), T("100"));
	wmmcapable = websGetVar(wp, T("wmm_capable"), T("off"));

	_SET_VALUE(WLN0_BASICRATE, basicrate);
	_SET_VALUE(WLN0_G_PROTECT, bgprotection);
	_SET_VALUE(WLN0_BEACON_PERIOD, beaconinterval);
	_SET_VALUE(WLN0_FRAG_THRESHOLD, fragmentthreshold);
	_SET_VALUE(WLN0_RTS_THRESHOLD, rtsthreshold);
	_SET_VALUE(WLN0_APSD_CAPABLE, apsdcapable);
	_SET_VALUE(WLN0_WMM_CAPABLE, wmmcapable);
	//_SET_VALUE(WLN0_Tx_POWER, txpower);

	_COMMIT();
	
	_RESTART();
	websRedirect(wp, T("wireless_Adv.asp"));
	return;
}

static void init_wireless_security(webs_t wp, char_t *path, char_t *query)
{
	//var mode = "<%get_wireless_wps("wpsmode");%>";
	//var pin = "<%get_wireless_wps("wpspin");%>";
	//var wpsmethod = "<%get_wireless_wps("wpsmethod");%>";
	//var enablewireless='<% get_wireless_basic("WirelessEnable"); %>'; 
	char_t result[1024];
	char_t *value=NULL,*value_1=NULL;
	memset(result,0,sizeof(result));
	
	_GET_VALUE(WLN0_WPS_ENABLE, value);
	IFVALUE(value);
	
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	_GET_VALUE(WLN0_WPS_PIN, value);
	IFVALUE(value);
	
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	_GET_VALUE(WLN0_WPS_METHOD, value);
	IFVALUE(value);

	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	_GET_VALUE(WLN0_WIRELESS_ENABLE, value);
	IFVALUE(value);
	
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

/*liuke 2011/11/24 modify begin*/
/*根据wl*_wps_mode的值决定是否显示提示框*/
	//_GET_VALUE(WLN0_WPS_OOB,value);
	_GET_VALUE(WLN0_UNIT, value_1);
	IFVALUE(value_1);
	if(0 == strcmp(value_1,"0"))
	{
		_GET_VALUE(WLN0_WPS_ENABLE, value);
		IFVALUE(value);	
	}
	else if(0 == strcmp(value_1,"0.1"))
	{
		_GET_VALUE("wl0.1_wps_mode", value);
		IFVALUE(value);
	}
		
	if(0 == strcmp(value, "enabled"))
		strncat(result, "1", 1024);
	else 
		strncat(result, "0", 1024);
	strncat(result, "\r", 1024);
	if(0 == strcmp(value_1,"0"))
	{
		_GET_VALUE(WLN0_WPS_OOB, value);
	}
	else if(0 == strcmp(value_1,"0.1"))
	{
		_GET_VALUE("lan1_wps_oob", value);
	}
	IFVALUE(value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	strncat(result, "\n", 1024);

/*liuke 2011/11/24 modify end*/

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	
	return ;
}
static void get_wireless_security(webs_t wp, char_t *path, char_t *query)
{	char_t result[2048];
	char_t *value=NULL;
	char_t temSSID[256] = {0};
	  /*  SSID[i] = fields_str[0];
		PreAuth[i] = fields_str[1];
		AuthMode[i] = fields_str[2];
		EncrypType[i] = fields_str[3];
		DefaultKeyID[i] = fields_str[4];
		Key1Type[i] = fields_str[5];
		Key1Str[i] = fields_str[6];
		Key2Type[i] = fields_str[7];
		Key2Str[i] = fields_str[8];
		Key3Type[i] = fields_str[9];
		Key3Str[i] = fields_str[10];
		Key4Type[i] = fields_str[11];
		Key4Str[i] = fields_str[12];
		WPAPSK[i] = fields_str[13];
		RekeyMethod[i] = fields_str[14];
		RekeyInterval[i] = fields_str[15];*/

	memset(result,0,sizeof(result));
	value = get_wl0_ssid();
	IFVALUE(value);
	value = encodeSSID(value,temSSID);
	//diag_printf("get_wireless_security_SSID=%s\n", value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	

	_GET_VALUE(WLN0_SECURITY_TYPE, value);
	IFVALUE(value);	
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	
	_GET_VALUE(WLN0_AUTH_MODE, value);
	IFVALUE(value);	
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	
	_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	IFVALUE(value);	
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	
	_GET_VALUE(WLN0_KEY_ID, value);
	IFVALUE(value);	
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	
	/*0:Hexadecimal; 1:ASCII*/
	_GET_VALUE(WLN0_KEY1_STR1, value);
	IFVALUE(value);	
	if(5 == strlen(value) || 13 == strlen(value)){			//ASCII
		strncat(result, "1", 2048);

	}else if(10 == strlen(value) || 26 == strlen(value)){		//Hexadecimal
		strncat(result, "0", 2048);
	}
	strncat(result, "\r", 2048);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_KEY2_STR1, value);
	IFVALUE(value);	
	if(5 == strlen(value) || 13 == strlen(value)){			//ASCII
		strncat(result, "1", 2048);
	}else if(10 == strlen(value) || 26 == strlen(value)){		//Hexadecimal
		strncat(result, "0", 2048);
	}
	strncat(result, "\r", 2048);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);

	_GET_VALUE(WLN0_KEY3_STR1, value);
	IFVALUE(value);	
	if(5 == strlen(value) || 13 == strlen(value)){			//ASCII
		strncat(result, "1", 2048);
	}else if(10 == strlen(value) || 26 == strlen(value)){		//Hexadecimal
		strncat(result, "0", 2048);
	}	
	strncat(result, "\r", 2048);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	
	_GET_VALUE(WLN0_KEY4_STR1, value);
	IFVALUE(value);	
	if(5 == strlen(value) || 13 == strlen(value)){			//ASCII
		strncat(result, "1", 2048);
		//diag_printf("WLN0_KEY1_TYPE=1 (ASCII)\n");
	}else if(10 == strlen(value) || 26 == strlen(value)){		//Hexadecimal
		strncat(result, "0", 2048);
		//diag_printf("WLN0_KEY1_TYPE=0 (Hexadecimal)\n");
	}	
	strncat(result, "\r", 2048);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	//diag_printf("%s=%s\n", WLN0_KEY4_STR1, value);

	
	_GET_VALUE(WLN0_WPA_PSK1, value);
	IFVALUE(value);	

	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	//diag_printf("%s=%s\n", WLN0_WPA_PSK1, value);
	
	_GET_VALUE(WLN0_WEP, value);
	IFVALUE(value);	
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	//diag_printf("%s=%s\n", WLN0_WEP, value);
	
	_GET_VALUE(WLN0_REKEY_INTERVAL, value);
	IFVALUE(value);	
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
	_GET_VALUE(WLN0_WPS_DEVICE_PIN, value);
	IFVALUE(value);
	strncat(result, value, 2048);
	strncat(result, "\r", 2048);
#ifdef CONFIG_WL_USE_APSTA
	
	_GET_VALUE(WLN0_UNIT, value);
	IFVALUE(value);
	if(is_wl0_vifs(value))
		strncat(result, "1", 2048);
	else
		strncat(result, "0", 2048);
	strncat(result, "\r", 2048);
	
//返回wl0是AP 还是sta模式
	_GET_VALUE(WLN0_MODE, value);
	IFVALUE(value);
	strncat(result, value, 2048);

	if((value = get_vif_ssid()) != NULL){
		strncat(result, "\n", 2048);
		memset(temSSID,0,sizeof(temSSID));
		value = encodeSSID(value,temSSID);
		strncat(result, value, 2048);
		strncat(result, "\r", 2048);
	}
#else
	strncat(result, "\n", 2048);
#endif		
	//websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\n\n"));
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	return ;
}

void confWEP(webs_t wp)
{
	char_t *Key1Str1, *Key2Str1, *Key3Str1, *Key4Str1;
	char_t *DefaultKeyID;
	
	DefaultKeyID = websGetVar(wp, T("wep_default_key"), T("1"));
	Key1Str1 = websGetVar(wp, T("wep_key_1"), T(""));
	Key2Str1 = websGetVar(wp, T("wep_key_2"), T(""));
	Key3Str1 = websGetVar(wp, T("wep_key_3"), T(""));
	Key4Str1 = websGetVar(wp, T("wep_key_4"), T(""));
	
	_SET_VALUE(WLN0_KEY1_STR1, Key1Str1);
	_SET_VALUE(WLN0_KEY2_STR1, Key2Str1);
	_SET_VALUE(WLN0_KEY3_STR1, Key3Str1);
	_SET_VALUE(WLN0_KEY4_STR1, Key4Str1);
	_SET_VALUE(WLN0_KEY_ID, DefaultKeyID);
//选择WEP加密模式，强制转换为"b/g模式"
	_SET_VALUE(WLN0_G_MODE, "1");	
	_SET_VALUE(WLN0_N_MODE, "0");	
	_SET_VALUE(WLN0_PLCPHDR, "long"); //add by stanley for 150M /135M
		
	return;
}

void confWPAGeneral( webs_t wp)
{
	char_t *cipher_str;
	char_t *key_renewal_interval;

	cipher_str = websGetVar(wp, T("cipher"), T("aes"));
	key_renewal_interval = websGetVar(wp, T("keyRenewalInterval"), T("3600"));

	if( 0 == strcmp(cipher_str,"tkip"))
	{
		_SET_VALUE(WLN0_ENCRYP_TYPE,"tkip");
	//huangxiaoli add 当为"tkip"加密规则时强制转换为bg模式
		_SET_VALUE(WLN0_G_MODE, "1");	
		_SET_VALUE(WLN0_N_MODE, "0");	
		_SET_VALUE(WLN0_PLCPHDR, "long"); //add by stanley for 150M /135M
	//add end	
	}
	else if(0 == strcmp(cipher_str,"aes"))
	{
		_SET_VALUE(WLN0_ENCRYP_TYPE,"aes");
		//set bgn mode
		_SET_VALUE(WLN0_N_MODE, "-1");
		_SET_VALUE(WLN0_G_MODE, "1");	
		_SET_VALUE(WLN0_PLCPHDR, "long");//reference from wlioctl.h,roy modify
	}
	else if(0 == strcmp(cipher_str,"tkip+aes"))
	{
		_SET_VALUE(WLN0_ENCRYP_TYPE,"tkip+aes");
		//set bgn mode
		_SET_VALUE(WLN0_N_MODE, "-1");
		_SET_VALUE(WLN0_G_MODE, "1");	
		_SET_VALUE(WLN0_PLCPHDR, "long");//reference from wlioctl.h,roy modify
	}
	_SET_VALUE(WLN0_REKEY_INTERVAL, key_renewal_interval);
	return ;
	
}
extern void disabled_wps(void);

void set_wireless_secur_from_indexasp(char *pass_phrase_str)
{
	char *ure,*wlunit, *v;
	char *unit="0",*v_unit="0.1";
	if(!pass_phrase_str)
		return;
	
	_GET_VALUE("ure_disable", ure);

	if(strncmp(ure,"0",1) == 0){
		//apclient 开启,把次SSID拷到当前配置
		copy_wl_index_to_unindex(v_unit);
	}else{
		//关闭，把主SSID拷到当前配置
		copy_wl_index_to_unindex(unit);
	}

	_SET_VALUE(WLN0_WEP, DISABLE);
	_SET_VALUE(WLN0_AUTH_MODE, OPEN);
	
#ifdef __CONFIG_WEB_VERSION__
	if(strcmp(__CONFIG_WEB_VERSION__,"ko") == 0)
	{
		_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2");
	}
	else
	{
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
	}
#else
	_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
#endif	
	_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
	_SET_VALUE(WLN0_ENCRYP_TYPE,"aes");
	_SET_VALUE(WLN0_WPS_ENABLE, "disabled");	//add by stanley for  index.asp and wireless_seucrity,asp
	_SET_VALUE(WLN0_REKEY_INTERVAL, "3600");

	_GET_VALUE(WLN0_UNIT,wlunit);
		
	wl_unit(wlunit,1);
	return;
}

extern void stop_wps_when_failed(void);
	
static void set_wireless_security(webs_t wp, char_t *path, char_t *query)
{	  
	char_t *security_mode,  *wpsenable,*old_wps,old_wps_str[16];
	char_t *wps_oob_status ;
	char_t *wlunit;
//	int disabled_flag=0;
	
	security_mode = websGetVar(wp, T("security_mode"), T(""));
	/*add by stanley 2010/10/14*/
	wpsenable = websGetVar(wp, T("wifiEn"), T("disabled"));
	/*end*/

	char *go = websGetVar(wp, T("GO"), T("wireless_security.asp"));
	_GET_VALUE(WLN0_UNIT,wlunit);

	_GET_VALUE(WLN0_WPS_ENABLE,old_wps);
	//must do this
	strcpy(old_wps_str,old_wps);
	/*add by stanley 2010/10/14*/
	if( 0== strcmp(wpsenable, "enabled") ){	
		
			if(strcmp(wlunit, "0") == 0)
				_SET_VALUE(WLN0_WPS_OOB, "disabled");	//MM,liuke
			else if(strcmp(wlunit, "0.1") == 0)
				_SET_VALUE("lan1_wps_oob", "disabled");
			//first,let's stop wps enabled former
			stop_wps_when_failed();
			
			if(strcmp(old_wps_str,"enabled") == 0){
				set_wireless_wps( wp,  path,  query);
			}else{
				nvram_set(WLN0_WPS_ENABLE, "enabled");
				nvram_set(WLN0_WPS_METHOD, "pbc");
#ifdef __CONFIG_WPS_LED__
				wps_led_test_on();
#endif				
			}

			wl_unit(wlunit,1);
			_COMMIT();
		
			if(strcmp(old_wps_str,"enabled") != 0){
				//now enabled, so need restart
				_RESTART();
				cyg_thread_delay(600);
			}
			else			
				cyg_thread_delay(100);
			
			websRedirect(wp, go);
			return;
	}
	 /*end*/	
	 if(0 == strcmp(wpsenable, "disabled")){	
	 	if(strcmp(old_wps_str,"enabled") == 0){
			/*add by stanley 2010/10/25*/
			//_GET_VALUE(WLN0_WPS_OOB, wps_oob_status);
			//if(strcmp(wps_oob_status, "disabled") == 0)
			//{
			set_wireless_wps( wp,  path,  query);
			/* ldf rm */
			//_SET_VALUE(WLN0_WPS_OOB, "enabled");		//MM,设置为enabled则WIN7下不会弹出WPS图标，Modify by liuke 2011/11/23
			//}else{
			//	tenda_stop_wps_timer();		
			//}
			_SET_VALUE(WLN0_WPS_ENABLE, "disabled");	
			_SET_VALUE(WLN0_WPS_METHOD, "");
#ifdef __CONFIG_WPS_LED__
			wps_led_test_off();		//Can I move it ???
#endif	
	 	}
		if (0 == strcmp(security_mode, "Disable") ) {	// !--- Disable Mode ---
			//0:open; 1:shared
			diag_printf("\n security mode: Disable\n");
			
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_SECURITY_TYPE,"");//modify 2010/11/04
			/*add by stanley 2010/10/14*/
			//_SET_VALUE(WLN0_WPA_PSK1,"");//modify 2010/11/04
			_SET_VALUE(WLN0_ENCRYP_TYPE, "");
			/*end*/
		}else if(0 == strcmp(security_mode, "0")){		// !--- Open Mode ---
			diag_printf("\n security mode: OPEN\n");	
			
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_WEP, ENABLE);
			/*add by stanley 2010/10/14*/
			_SET_VALUE(WLN0_SECURITY_TYPE, "0");
			//_SET_VALUE(WLN0_WPA_PSK1, "");
			_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
			/*end*/
			confWEP( wp);
		}else if(0 == strcmp(security_mode, "1")){	// !--- Shared Mode ---
			char_t *security_shared_mode;
			security_shared_mode = websGetVar(wp, T("security_shared_mode"), T(""));		
			diag_printf("\n security mode: Shared\n");		
	
			_SET_VALUE(WLN0_AUTH_MODE, SHARED);
			_SET_VALUE(WLN0_WEP, ENABLE);
			/*add by stanley 2010/10/14*/
			_SET_VALUE(WLN0_SECURITY_TYPE, "1");
			//_SET_VALUE(WLN0_WPA_PSK1, "");
			_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
			/*end*/
			confWEP( wp);
			
		}else if(0 == strcmp(security_mode, "psk2") ){// !---  WPA2 Personal Mode ----
			char_t *pass_phrase_str;
			pass_phrase_str = websGetVar(wp, T("passphrase"), T(""));
			diag_printf("\n security mode: psk2\n");	
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk2");
			_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(security_mode, "psk psk2") )	{ //! ----   WPA PSK WPA2 PSK mixed
			char_t *pass_phrase_str;
			pass_phrase_str = websGetVar(wp, T("passphrase"), T(""));
			diag_printf("\n security mode: PSK PSK2\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2");
			_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(security_mode, "psk")){	// !---  WPA Personal Mode ---
			char_t *pass_phrase_str;
			pass_phrase_str = websGetVar(wp, T("passphrase"), T(""));	
			diag_printf("\n security mode: PSK\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
			_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
			confWPAGeneral( wp);	
		}
		if(strcmp(security_mode, "Disable") != 0)
		{
			if(strcmp(wlunit, "0") == 0)
			{
				nvram_set(WLN0_WPS_OOB, "disabled");
			}
			else if(strcmp(wlunit, "0.1") == 0)
			{
				nvram_set("lan1_wps_oob", "disabled");
			}
		}
	}
		
	wl_unit(wlunit,1);
#if 1
	_COMMIT();
	_RESTART();
	cyg_thread_delay(100);
	websRedirect(wp, go);
#else 
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\n\n"));
	websDone(wp, 200);
#endif
	return ;
}
static int get_wps_configured(int eid, webs_t wp, int argc, char_t **argv)
{
	char *wpsenable;
	_GET_VALUE(WLN0_WPS_OOB,wpsenable);
	if(0 == strcmp(wpsenable, "disabled")){
		return websWrite(wp, T("1"));//wps enable
	} else {
		return websWrite(wp, T("0")); //wps disable
	}
}

static int get_wireless_wds(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name=NULL;
	char_t *value=NULL;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}

	if(0 == strncmp(name, "wdsmode", strlen("wdsmode"))){
		char_t *list, *time, *mode, *lazy;
		_GET_VALUE(WLN0_WDS_LAZY, lazy);
		_GET_VALUE(WLN0_WDS_LIST, list);
		_GET_VALUE(WLN0_WDS_TIMEOUT, time);
		_GET_VALUE(WLN0_MODE, mode);
		if(0 == strcmp(time, "0")){//disable wds
		
			 websWrite(wp, T("0"));
			 return 0;
		}else if(0 == strcmp(time, "1")  && 0 == strcmp(lazy, "1") &&
			0 == strlen(list) && 0 == strcmp(mode, "ap")){	//lazy mode
			
			websWrite(wp, T("4"));
			 return 0;
		}else if(0==strcmp(time, "1") && 0==strcmp(lazy, "0") &&
			0!=strlen(list) && 0 == strcmp(mode, "ap")){//repeater mode
			
			websWrite(wp, T("3"));
			 return 0;
		}else if(0 == strcmp(time, "1") && 0==strcmp(lazy, "0")&&
			0!= strlen(list) && 0 == strcmp(mode, "wds")){//bridge mode
			
			websWrite(wp, T("2"));
			return 0;
		}
	}

	if(0 == strncmp(name, "wdslist",	strlen("wdslist"))){
		_GET_VALUE(WLN0_WDS_LIST, value);
		if(value == NULL)
			value='\0';
		
		websWrite(wp, T("%s"), value);
		 return 0;
	}

	//diag_printf("%s=%s\n", name, value);
	
	websWrite(wp, T("%s"), value);
	return 0;
}

static int get_wireless_filter(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name=NULL;
	char_t *value=NULL;

	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}

	if(0 == strncmp(name, "macmode", strlen("macmode"))){
			_GET_VALUE(WLN0_MAC_MODE, value);
	}else if(0 == strncmp(name, "maclist", strlen("maclist"))){
			_GET_VALUE(WLN0_MAC_LIST, value);
	}

	//diag_printf("%s=%s\n", name, value);
	
	websWrite(wp, T("%s"), value);
	return 0;
}

static void set_wireless_filter(webs_t wp, char_t *path, char_t *query)
{
	char_t *mode, *list;
	char_t *wlunit;
	
	mode = websGetVar(wp,T("FilterMode"),T("disabled"));
	list = websGetVar(wp,T("maclist"),T(""));

	char *go = websGetVar(wp,T("GO"),T("wireless_filter.asp"));


	_SET_VALUE(WLN0_MAC_MODE, mode);
	_SET_VALUE(WLN0_MAC_LIST, list);

	/*if(0 == strcmp(mode, "disabled"))
	{
		_DEL_VALUE(WLN0_MAC_LIST);
	}*/
	_GET_VALUE(WLN0_UNIT,wlunit);

	wl_unit(wlunit,1);
	
	_COMMIT();
	
	_RESTART();
	websRedirect(wp, go);
	return ;
}


static int get_wireless_wps(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name=NULL;
	char_t *value=NULL;

	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}

	if(0 == strncmp(name, "wpsmode", strlen("wpsmode"))){
			_GET_VALUE(WLN0_WPS_ENABLE, value);
	}else if(0 == strncmp(name, "wpspin", strlen("wpspin"))){
			_GET_VALUE(WLN0_WPS_PIN, value);
			if(value == NULL)
				value ="";
	}else if(0 == strncmp(name, "wpsmethod", strlen("wpsmethod"))){
			_GET_VALUE(WLN0_WPS_METHOD, value);
		/*
			if(strcmp(value,"pbc")!=0 || strcmp(value,"pin")!=0){
				strcpy(value, "pbc");
				diag_printf("this=====\n");
			}*/
	}else if(0 == strncmp(name, "dyn_wpspin", strlen("dyn_wpspin"))){
			//_GET_VALUE("wps_device_pin", value);
			value=nvram_get("wps_device_pin");
			if(value == NULL)
				value ="";
	}
	websWrite(wp, T("%s"), value);
	return 0;
}

extern void set_wps_oobssid(void);

void OOB(void)
{
	char_t *ssid=NULL;
	char_t *wlunit = NULL;
		
	set_wps_oobssid();
	clear_security_parameter();
	wps_oob("unconfigured");/*reset OOB*/

	_SET_VALUE(WLN0_WPS_METHOD, "pbc");
	_SET_VALUE(WLN0_WPS_PIN, "");	

	_GET_VALUE(WLN0_UNIT,wlunit);
	wl_unit(wlunit,1);
	_COMMIT();
	
	_RESTART();
	return;
}
 static void set_wireless_oob(webs_t wp, char_t *path, char_t *query)
  {
	OOB();
	tenda_stop_wps_timer();
	cyg_thread_delay(300);
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websDone(wp, 200);
	return ;
  }
 
#ifdef CONFIG_WL_USE_APSTA

 static void set_wireless_wire_mode(webs_t wp, char_t *path, char_t *query)
 {
	char_t *wlMode,*sta_ssid,*sta_mac,*sta_channel,
			*sta_security_mode,*passphrase;

	char_t *wps_oob_status;
	//char_t *wl0_ure_p;
	char_t *htbw;

	int /*old_wl_ure,*/wl_ure;
	int reboot_cmd = 0;
	char *old_wl0_mode;
	char_t *wl0_mode;
												
	char *wan_ifnames = "wan_ifnames";
	char *wan0_ifnames = "wan0_ifnames";
	char *wan_ifname = "wan_ifname";
	char *lan1_ifnames = "lan1_ifnames";
	char *lan1_ifname = "lan1_ifname";
	char lan0_ifnames[] = "vlan1 XXX XXXXXXXXXXX";
	char *wlunit="0";
	char *wlunit_vif="0.1";
	char *wl0_ssid;

	wlMode = websGetVar(wp,T("wlMode"),T("0"));//default is wire
	sta_ssid = websGetVar(wp,T("sta_ssid"),T(""));
	sta_mac = websGetVar(wp,T("sta_mac"),T(""));
	sta_channel = websGetVar(wp,T("sta_channel"),T("0"));
	sta_security_mode = websGetVar(wp,T("sta_security_mode"),T(""));
	passphrase = websGetVar(wp,T("passphrase"),T(""));
	wl0_mode = websGetVar(wp,T("wl_mode"),T(""));
	
	//_GET_VALUE("wl_ure", wl0_ure_p);
	_GET_VALUE("wl_mode", old_wl0_mode);
	
	//if(wl0_ure_p && strlen(wl0_ure_p) > 0)
	//	old_wl_ure = atoi(wl0_ure_p);
	//else
	//	old_wl_ure = 0;
	
	wl_ure = atoi(wlMode);// if wlMode == 1, wl_ure is enable

	if(strcmp(old_wl0_mode,"wet") != 0)
	{
		if((strcmp(old_wl0_mode,wl0_mode) != 0))
		{
			reboot_cmd = 1;
		}
	}
	else
	{
		if(wl_ure == 1)
		{
			reboot_cmd = 1;
		}
		_SET_VALUE(_LAN0_DHCPD_EN, "dhcp");
	}

	diag_printf("wl_ure=%d,reboot_cmd=%d\n",wl_ure,reboot_cmd);
	diag_printf("old_wl0_mode=%s,wl0_mode=%s\n",old_wl0_mode,wl0_mode);
	if(strcmp(wl0_mode,"sta") == 0){
		//sta
		_SET_VALUE("ure_disable", "0");
		//A5无线wan的时候RJ45 就为lan口
	
#ifdef A5
		_SET_VALUE("vlan1ports", "0 1 2 3 4 5*");
		_SET_VALUE("vlan2ports", "5");
#endif 			
		
		//判断有没有设置双SSID,如果没有，把之前
		//的主SSID所有参数移到次SSID
		if(get_vif_ssid() == NULL){
			//先移到(wl0->wl)
			copy_wl_index_to_unindex(wlunit);
			//再从wl移动wl0.1
			wl_unit(wlunit_vif,1);// 用1表示连安全设置也移动
			
			_GET_VALUE(WLN0_SSID0,wl0_ssid);
			validate_vif_ssid(wl0_ssid);
		}
		
		//set mode
		_SET_VALUE("wl_ure", "1");
		_SET_VALUE("wl_mode", "sta");

		//set sta info
		_SET_VALUE(WLN0_SSID0, sta_ssid);	
		_SET_VALUE(WLN0_CHANNEL, sta_channel);

		_GET_VALUE(WLN0_HT_BW, htbw);

		if(strncmp(sta_channel,"0",1) == 0 || strncmp(htbw,"0",1) == 0){
			_SET_VALUE(WLN0_HT_EXTCHA, "none");
		}else if(atoi(sta_channel) < 5 && strncmp(htbw,"0",1) != 0)
		{
			_SET_VALUE(WLN0_HT_EXTCHA, "lower");
		}else if(atoi(sta_channel) > 7 && strncmp(htbw,"0",1) != 0)
		{
			_SET_VALUE(WLN0_HT_EXTCHA, "upper");
		}
		
		if(reboot_cmd){//和上次模式不同
			//close wps
			_SET_VALUE(WLN0_WPS_OOB, "disabled");		//MM,liuke modify,为了保证次SSID的WPS生效
			_SET_VALUE(WLN0_WPS_ENABLE, "disabled");	
			_SET_VALUE(WLN0_WPS_METHOD, "pbc");

			 //close wds
			_SET_VALUE(WLN0_WDS_TIMEOUT, "0");//add by stanley for close WDS	
			_SET_VALUE(WLN0_WDS_LAZY, "0");
			_SET_VALUE(WLN0_WDS_LIST, "");		
		}
		
		//set security
		if (0 == strcmp(sta_security_mode, "Disable") ) {	// !--- Disable Mode ---
			//0:open; 1:shared
			diag_printf("\n security mode: Disable\n");
			
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_SECURITY_TYPE,"");//modify 2010/11/04
			/*add by stanley 2010/10/14*/
			//_SET_VALUE(WLN0_WPA_PSK1,"");//modify 2010/11/04
			_SET_VALUE(WLN0_ENCRYP_TYPE, "");
			/*end*/
		}else if(0 == strcmp(sta_security_mode, "0")){		// !--- Open Mode ---
			diag_printf("\n security mode: OPEN\n");	
			
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_WEP, ENABLE);
			/*add by stanley 2010/10/14*/
			_SET_VALUE(WLN0_SECURITY_TYPE, "0");
			//_SET_VALUE(WLN0_WPA_PSK1, "");
			_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
			/*end*/
			confWEP( wp);
		}else if(0 == strcmp(sta_security_mode, "1")){	// !--- Shared Mode ---
			char_t *security_shared_mode;
			security_shared_mode = websGetVar(wp, T("security_shared_mode"), T(""));		
			diag_printf("\n security mode: Shared\n");		
	
			_SET_VALUE(WLN0_AUTH_MODE, SHARED);
			_SET_VALUE(WLN0_WEP, ENABLE);
			/*add by stanley 2010/10/14*/
			_SET_VALUE(WLN0_SECURITY_TYPE, "1");
			//_SET_VALUE(WLN0_WPA_PSK1, "");
			_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
			/*end*/
			confWEP( wp);
		}else if(0 == strcmp(sta_security_mode, "psk2") ){// !---  WPA2 Personal Mode ----

			diag_printf("\n security mode: psk2\n");	
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk2");
			_SET_VALUE(WLN0_WPA_PSK1, passphrase);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(sta_security_mode, "psk psk2") )	{ //! ----   WPA PSK WPA2 PSK mixed
			diag_printf("\n security mode: PSK PSK2\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2");
			_SET_VALUE(WLN0_WPA_PSK1, passphrase);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(sta_security_mode, "psk")){	// !---  WPA Personal Mode ---
			diag_printf("\n security mode: PSK\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
			_SET_VALUE(WLN0_WPA_PSK1, passphrase);
			confWPAGeneral( wp);	
		}

		//remove wireless mac filter
		_SET_VALUE(WLN0_MAC_MODE, "disabled");
		_SET_VALUE(WLN0_MAC_LIST, "");
	
		//set wan lan ifname
		_SET_VALUE(_LAN0_IFNAMES, "vlan1 wl0.1");
		_SET_VALUE(_WAN0_IFNAME, "eth1");
		_SET_VALUE(wan_ifnames, "eth1");
		_SET_VALUE(wan0_ifnames, "eth1");
		_SET_VALUE(wan_ifname, "eth1");
		if(reboot_cmd){
			int i;
			char nv_interface[32];
			/* Make lan1_ifname lan1_ifnames empty sothat br1 is not created in URE mode. */
			/* Disable all VIFS wlX.2 onwards */
		        _SET_VALUE(lan1_ifname, "" );
		        _SET_VALUE(lan1_ifnames, "" );
			_SET_VALUE("wl1.1_bss_enabled", "0");

			for (i=2; i<16; i++) {
				sprintf( nv_interface, "wl0.%d_bss_enabled", i );
				_SET_VALUE(nv_interface, "0");
				sprintf( nv_interface, "wl1.%d_bss_enabled", i );
				_SET_VALUE(nv_interface, "0");
			}
		}
//when ure enable,we should set WLN0_PLCPHDR to short
//		_SET_VALUE(WLN0_HIDE_SSID0, "0");
//		_SET_VALUE(WLN0_PLCPHDR, "short");
		nvram_set(WLN0_WPS_OOB, "disabled");
	}
	else if(strcmp(wl0_mode,"ap") == 0){
		//wired
		_SET_VALUE("ure_disable", "1");
		
		//A5 有线wan的时候RJ45 就为wan口
#ifdef A5
		_SET_VALUE("vlan1ports", "1 2 3 4 5*");
		_SET_VALUE("vlan2ports", "0 5");
#endif
		if(!reboot_cmd){
			//和上次一样,什么都不做
			cyg_thread_delay(100);
			websRedirect(wp, T("/wireless_mode.asp"));
			return ;
		}
		//判断有没有设置双SSID,如果有，把之前
		//的次SSID所有参数移到主SSID
		if(get_vif_ssid() != NULL){
			//先移到(wl0.1->wl)
			copy_wl_index_to_unindex(wlunit_vif);
			//再从wl移动wl0
			wl_unit(wlunit,1);// 用1表示连安全设置也移动
		}

		_SET_VALUE("wl_ure", "0");

		//enable wps
		//_SET_VALUE(WLN0_WPS_ENABLE, "enabled");	

		//set mode
		_SET_VALUE("wl_mode", "ap");

		//set wan lan ifname
		memset(lan0_ifnames,0,sizeof(lan0_ifnames));
		snprintf(lan0_ifnames,sizeof(lan0_ifnames),"%s %s %s","vlan1","eth1",
				get_vif_ssid()?"wl0.1":"");
		
		_SET_VALUE(_LAN0_IFNAMES, lan0_ifnames);
		
		_SET_VALUE(_WAN0_IFNAME, "vlan2");
		_SET_VALUE(wan_ifnames, "vlan2");
		_SET_VALUE(wan0_ifnames, "vlan2");
		_SET_VALUE(wan_ifname, "vlan2");
		//清除vif的MAC，这样，会重新生成vif MAC,不会和main ssid的MAC一样
		_SET_VALUE("wl0.1_hwaddr","");
	}
	else if(strcmp(wl0_mode,"wet") == 0)
	{
		cyg_thread_delay(100);
		websRedirect(wp, T("/wireless_mode.asp"));
		return ;
	}
	
	/* We're unsetting the WAN hardware address so that we get the correct
		 address for the new WAN interface the next time we boot. */
	_SET_VALUE(_WAN0_HWADDR, "");

	wl_unit(wlunit,0);

	if(reboot_cmd && strcmp(wl0_mode,"ap") == 0){
		//比无线WAN切到有线WAN时
		validate_vif_ssid(NULL);//去掉次SSID,次SSID 变主SSID
	}
	
	_COMMIT();

	if(reboot_cmd){
		websRedirect(wp, T("/direct_reboot.asp"));
		sys_reboot();
	}else{
		_RESTART();
		cyg_thread_delay(200);
		websRedirect(wp, T("/wireless_mode.asp"));
	}
 }




static void get_wireless_sta_info(webs_t wp, char_t *path, char_t *query)
{
	char_t result[1024];
	char_t *value=NULL;
	char_t temSSID[256] = {0};
	char_t channel[4];
	 
	  /* mode = str[0] ;
	 	ssid =str[1];
		resmac = str[2];
		channel = str[3];
		sec = str[4];
		wpaAlg = str[5];
		wpakeyVa = str[6];
		RekeyInterval = str[7];
		DefaultKeyID = str[8] ;
		Key1Type = str[9] ;
		Key1Str = str[10];
		Key2Type = str[11];
		Key2Str = str[12];
		Key3Type = str[13];
		Key3Str = str[14];
		Key4Type = str[15];
		Key4Str = str[16];*/
					
	char_t unit[]="0";				
								
	copy_wl_index_to_unindex(unit);
		
	memset(result,0,sizeof(result));
	_GET_VALUE(WLN0_MODE, value);
	IFVALUE(value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);	
	
	_GET_VALUE(WLN0_SSID0, value);
	IFVALUE(value);
	value = encodeSSID(value,temSSID);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);//sta mac doesn,t need in 5357
	
	_GET_VALUE(WLN0_CHANNEL, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	_GET_VALUE(WLN0_SECURITY_TYPE, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_SECURITY_TYPE is %s \n", value);
	if(value && strlen(value) <1)
		strncat(result, "Disable", 1024);
	else	
		strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	_GET_VALUE(WLN0_WPA_PSK1, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	_GET_VALUE(WLN0_REKEY_INTERVAL, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	_GET_VALUE(WLN0_KEY_ID, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY_ID is %s \n", value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	/*0:Hexadecimal; 1:ASCII*/
	_GET_VALUE(WLN0_KEY1_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY1_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	}
	
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	_GET_VALUE(WLN0_KEY2_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY2_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	}
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	_GET_VALUE(WLN0_KEY3_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY3_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	} 
	
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	_GET_VALUE(WLN0_KEY4_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY4_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	//diag_printf("WLN0_KEY1_TYPE=1 (ASCII)\n");
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	//diag_printf("WLN0_KEY1_TYPE=0 (Hexadecimal)\n");
	} 
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	
	//huangxiaoli add
	_GET_VALUE(WLN0_WIRELESS_ENABLE, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);	

	_GET_VALUE(WLN0_WDS_LIST, value);
	IFVALUE(value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	
	channel[0] ="\0"; 
	sprintf(channel, "%d", initwireless11gchannels());
	strncat(result, channel, 2048);
	strncat(result, "\r", 2048);

	strncat(result, "\n", 1024);
		
		
	//websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\n\n"));
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
		
		
	websWrite(wp, result);
	websDone(wp, 200);	
	return ;
}

static void set_wireless_unit(webs_t wp, char_t *path, char_t *query)
{
	char_t *unit,*go;
//	char_t unit_str[8];
	char subunit[]="0.1";
	
	unit = websGetVar(wp, T("ssid_index"), T("0"));
	go = websGetVar(wp, T("GO"), T("wireless_security.asp"));

	if(atoi(unit) != 0){
		unit = subunit;
	}

	copy_wl_index_to_unindex(unit);
	websRedirect(wp, go);
}

static void set_universal_repeater(webs_t wp, char_t *path, char_t *query)
{
	char_t *wet_enable,*sta_ssid,*sta_mac,*sta_channel,
			*sta_security_mode,*passphrase;

	char_t *wps_oob_status;
	//char_t *wl0_ure_p;
	char_t *htbw;

	int /*old_wl_ure,*/wl_ure;
	int reboot_cmd = 0;
	char *old_wl0_mode;
	char_t *wl0_mode;

	char *wan_ifnames = "wan_ifnames";
	char *wan0_ifnames = "wan0_ifnames";
	char *wan_ifname = "wan_ifname";
	char *lan1_ifnames = "lan1_ifnames";
	char *lan1_ifname = "lan1_ifname";
	char lan0_ifnames[] = "vlan1 XXX XXXXXXXXXXX";
	char *wlunit="0";
	char *wlunit_vif="0.1";
	char *wl0_ssid;

	wet_enable = websGetVar(wp,T("wlMode"),T("0"));//default is wire
	sta_ssid = websGetVar(wp,T("sta_ssid"),T(""));
	sta_mac = websGetVar(wp,T("sta_mac"),T(""));
	sta_channel = websGetVar(wp,T("sta_channel"),T("0"));
	sta_security_mode = websGetVar(wp,T("sta_security_mode"),T(""));
	passphrase = websGetVar(wp,T("passphrase"),T(""));
	wl0_mode = websGetVar(wp,T("wl_mode"),T(""));

	//_GET_VALUE("wl_ure", wl0_ure_p);
	_GET_VALUE("wl_mode", old_wl0_mode);
	
	//if(wl0_ure_p && strlen(wl0_ure_p) > 0)
	//	old_wl_ure = atoi(wl0_ure_p);
	//else
	//	old_wl_ure = 0;

	wl_ure = atoi(wet_enable);// if wlMode == 1, wl_ure is enable

	if((strcmp(old_wl0_mode,"wet") == 0 && wl_ure == 0) ||
		(strcmp(old_wl0_mode,"ap") == 0 && wl_ure == 1))
	{
		reboot_cmd = 1;
	}
	else if(strcmp(old_wl0_mode,"sta") == 0 && wl_ure == 0)
	{
		websRedirect(wp, T("/wireless_apclient.asp"));
	}

	diag_printf("wl_ure=%d,reboot_cmd=%d\n",wl_ure,reboot_cmd);
	diag_printf("old_wl0_mode=%s,wl0_mode=%s\n",old_wl0_mode,wl0_mode);
	if(strcmp(wl0_mode,"wet") == 0){
		//wet
		_SET_VALUE("ure_disable", "0");
		//A5无线wan的时候RJ45 就为lan口
	
#ifdef A5
		_SET_VALUE("vlan1ports", "0 1 2 3 4 5*");
		_SET_VALUE("vlan2ports", "5");
#endif 			
		
		//判断有没有设置双SSID,如果没有，把之前
		//的主SSID所有参数移到次SSID
		if(get_vif_ssid() == NULL){
			//先移到(wl0->wl)
			copy_wl_index_to_unindex(wlunit);
			//再从wl移动wl0.1
			wl_unit(wlunit_vif,1);// 用1表示连安全设置也移动
			
			_GET_VALUE(WLN0_SSID0,wl0_ssid);
			validate_vif_ssid(wl0_ssid);
		}
		
		//set mode
		_SET_VALUE("wl_ure", "1");
		_SET_VALUE("wl_mode", "wet");

		//set sta info
		_SET_VALUE(WLN0_SSID0, sta_ssid);	
		_SET_VALUE(WLN0_CHANNEL, sta_channel);

		_GET_VALUE(WLN0_HT_BW, htbw);

		if(strncmp(sta_channel,"0",1) == 0 || strncmp(htbw,"0",1) == 0){
			_SET_VALUE(WLN0_HT_EXTCHA, "none");
		}else if(atoi(sta_channel) < 5 && strncmp(htbw,"0",1) != 0)
		{
			_SET_VALUE(WLN0_HT_EXTCHA, "lower");
		}else if(atoi(sta_channel) > 7 && strncmp(htbw,"0",1) != 0)
		{
			_SET_VALUE(WLN0_HT_EXTCHA, "upper");
		}
		
		if(reboot_cmd){//和上次模式不同
			//close wps
			_SET_VALUE(WLN0_WPS_OOB, "disabled");		//MM,liuke modify,为了保证次SSID的WPS生效
			_SET_VALUE(WLN0_WPS_ENABLE, "disabled");	
			_SET_VALUE(WLN0_WPS_METHOD, "pbc");

			 //close wds
			_SET_VALUE(WLN0_WDS_TIMEOUT, "0");//add by stanley for close WDS	
			_SET_VALUE(WLN0_WDS_LAZY, "0");
			_SET_VALUE(WLN0_WDS_LIST, "");		
		}
		
		//set security
		if (0 == strcmp(sta_security_mode, "Disable") ) {	// !--- Disable Mode ---
			//0:open; 1:shared
			diag_printf("\n security mode: Disable\n");
			
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_SECURITY_TYPE,"");//modify 2010/11/04
			/*add by stanley 2010/10/14*/
			//_SET_VALUE(WLN0_WPA_PSK1,"");//modify 2010/11/04
			_SET_VALUE(WLN0_ENCRYP_TYPE, "");
			/*end*/
		}else if(0 == strcmp(sta_security_mode, "0")){		// !--- Open Mode ---
			diag_printf("\n security mode: OPEN\n");	
			
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_WEP, ENABLE);
			/*add by stanley 2010/10/14*/
			_SET_VALUE(WLN0_SECURITY_TYPE, "0");
			//_SET_VALUE(WLN0_WPA_PSK1, "");
			_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
			/*end*/
			confWEP( wp);
		}else if(0 == strcmp(sta_security_mode, "1")){	// !--- Shared Mode ---
			char_t *security_shared_mode;
			security_shared_mode = websGetVar(wp, T("security_shared_mode"), T(""));		
			diag_printf("\n security mode: Shared\n");		
	
			_SET_VALUE(WLN0_AUTH_MODE, SHARED);
			_SET_VALUE(WLN0_WEP, ENABLE);
			/*add by stanley 2010/10/14*/
			_SET_VALUE(WLN0_SECURITY_TYPE, "1");
			//_SET_VALUE(WLN0_WPA_PSK1, "");
			_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
			/*end*/
			confWEP( wp);
		}else if(0 == strcmp(sta_security_mode, "psk2") ){// !---  WPA2 Personal Mode ----

			diag_printf("\n security mode: psk2\n");	
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk2");
			_SET_VALUE(WLN0_WPA_PSK1, passphrase);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(sta_security_mode, "psk psk2") )	{ //! ----	 WPA PSK WPA2 PSK mixed
			diag_printf("\n security mode: PSK PSK2\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2");
			_SET_VALUE(WLN0_WPA_PSK1, passphrase);
			confWPAGeneral( wp);
			
		}else if(0 == strcmp(sta_security_mode, "psk")){	// !---  WPA Personal Mode ---
			diag_printf("\n security mode: PSK\n");
			
			_SET_VALUE(WLN0_WEP, DISABLE);
			_SET_VALUE(WLN0_AUTH_MODE, OPEN);
			_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
			_SET_VALUE(WLN0_WPA_PSK1, passphrase);
			confWPAGeneral( wp);	
		}

		//remove wireless mac filter
		_SET_VALUE(WLN0_MAC_MODE, "disabled");
		_SET_VALUE(WLN0_MAC_LIST, "");
	
		//set wan lan ifname
		_SET_VALUE(_LAN0_IFNAMES, "vlan1 eth1 wl0.1");
		_SET_VALUE(_WAN0_IFNAME, "vlan2");
		_SET_VALUE(wan_ifnames, "vlan2");
		_SET_VALUE(wan0_ifnames, "vlan2");
		_SET_VALUE(wan_ifname, "vlan2");

		_SET_VALUE(_LAN0_DHCPD_EN, "static");
		if(reboot_cmd){
			int i;
			char nv_interface[32];
			/* Make lan1_ifname lan1_ifnames empty sothat br1 is not created in URE mode. */
			/* Disable all VIFS wlX.2 onwards */
				_SET_VALUE(lan1_ifname, "" );
				_SET_VALUE(lan1_ifnames, "" );
			_SET_VALUE("wl1.1_bss_enabled", "0");

			for (i=2; i<16; i++) {
				sprintf( nv_interface, "wl0.%d_bss_enabled", i );
				_SET_VALUE(nv_interface, "0");
				sprintf( nv_interface, "wl1.%d_bss_enabled", i );
				_SET_VALUE(nv_interface, "0");
			}
		}
//when ure enable,we should set WLN0_PLCPHDR to short
//		_SET_VALUE(WLN0_HIDE_SSID0, "0");
//		_SET_VALUE(WLN0_PLCPHDR, "short");
		nvram_set(WLN0_WPS_OOB, "disabled");
	}
	else if(strcmp(wl0_mode,"ap") == 0){
		//wired
		_SET_VALUE("ure_disable", "1");
		
		//A5 有线wan的时候RJ45 就为wan口
#ifdef A5
		_SET_VALUE("vlan1ports", "1 2 3 4 5*");
		_SET_VALUE("vlan2ports", "0 5");
#endif
		if(!reboot_cmd){
			//和上次一样,什么都不做
			cyg_thread_delay(100);
			websRedirect(wp, T("/wireless_apclient.asp"));
			return ;
		}
		//判断有没有设置双SSID,如果有，把之前
		//的次SSID所有参数移到主SSID
		if(get_vif_ssid() != NULL){
			//先移到(wl0.1->wl)
			copy_wl_index_to_unindex(wlunit_vif);
			//再从wl移动wl0
			wl_unit(wlunit,1);// 用1表示连安全设置也移动
		}

		_SET_VALUE("wl_ure", "0");

		//enable wps
		//_SET_VALUE(WLN0_WPS_ENABLE, "enabled");	

		//set mode
		_SET_VALUE("wl_mode", "ap");

		//set wan lan ifname
		memset(lan0_ifnames,0,sizeof(lan0_ifnames));
		snprintf(lan0_ifnames,sizeof(lan0_ifnames),"%s %s %s","vlan1","eth1",
				get_vif_ssid()?"wl0.1":"");
		
		_SET_VALUE(_LAN0_IFNAMES, lan0_ifnames);
		
		_SET_VALUE(_WAN0_IFNAME, "vlan2");
		_SET_VALUE(wan_ifnames, "vlan2");
		_SET_VALUE(wan0_ifnames, "vlan2");
		_SET_VALUE(wan_ifname, "vlan2");

		_SET_VALUE(_LAN0_DHCPD_EN, "dhcp");
		//清除vif的MAC，这样，会重新生成vif MAC,不会和main ssid的MAC一样
		_SET_VALUE("wl0.1_hwaddr","");
	}
	else 
	{
		cyg_thread_delay(100);
		websRedirect(wp, T("/wireless_apclient.asp"));
		return ;
	}
	
	/* We're unsetting the WAN hardware address so that we get the correct
		 address for the new WAN interface the next time we boot. */
	_SET_VALUE(_WAN0_HWADDR, "");

	wl_unit(wlunit,0);

	if(reboot_cmd && strcmp(wl0_mode,"ap") == 0){
		//比无线WAN切到有线WAN时
		validate_vif_ssid(NULL);//去掉次SSID,次SSID 变主SSID
	}
	
	_COMMIT();

	if(reboot_cmd){
		websRedirect(wp, T("/direct_reboot.asp"));
		sys_reboot();
	}else{
		_RESTART();
		cyg_thread_delay(200);
		websRedirect(wp, T("/wireless_apclient.asp"));
	}
 }


static void init_universal_repeater(webs_t wp, char_t *path, char_t *query)
{
	char_t result[1024];
	char_t *value=NULL;
	char_t temSSID[256] = {0};

	  /* mode = str[0] ;
		ssid =str[1];
		resmac = str[2];
		channel = str[3];
		sec = str[4];
		wpaAlg = str[5];
		wpakeyVa = str[6];
		RekeyInterval = str[7];
		DefaultKeyID = str[8] ;
			Key1Type = str[9] ;
			Key1Str = str[10];
			Key2Type = str[11];
			Key2Str = str[12];
			Key3Type = str[13];
			Key3Str = str[14];
			Key4Type = str[15];
			Key4Str = str[16];*/
			
	char_t unit[]="0";
	
	copy_wl_index_to_unindex(unit);

	memset(result,0,sizeof(result));
	_GET_VALUE("wl0_mode", value);
	if(value && strcmp(value, "wet") == 0){
	//sta enable	
		strncat(result, "1", 1024);
		strncat(result, "\r", 1024);
	}	
	else{
	//sta disable
		strncat(result, "0", 1024);
		strncat(result, "\r", 1024);
	}

	_GET_VALUE(WLN0_SSID0, value);
	IFVALUE(value);
	value = encodeSSID(value,temSSID);
	//diag_printf("wirelessInitMode_SSID=%s\n", value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	strncat(result, "\r", 1024);//sta mac doesn,t need in 5357
	
	_GET_VALUE(WLN0_CHANNEL, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	_GET_VALUE(WLN0_SECURITY_TYPE, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_SECURITY_TYPE is %s \n", value);
	if(value && strlen(value) <1)
		strncat(result, "Disable", 1024);
	else	
		strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	_GET_VALUE(WLN0_ENCRYP_TYPE, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	_GET_VALUE(WLN0_WPA_PSK1, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	
	_GET_VALUE(WLN0_REKEY_INTERVAL, value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	_GET_VALUE(WLN0_KEY_ID, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY_ID is %s \n", value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	/*0:Hexadecimal; 1:ASCII*/
	_GET_VALUE(WLN0_KEY1_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY1_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	}
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	_GET_VALUE(WLN0_KEY2_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY2_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	}
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	_GET_VALUE(WLN0_KEY3_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY3_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	} 
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	_GET_VALUE(WLN0_KEY4_STR1, value);
	//diag_printf("[wireless.c][get_wireless_sta_info]: WLN0_KEY4_STR1 is %s \n", value);
	IFVALUE(value); 
	if(5 == strlen(value) || 13 == strlen(value)){ //ASCII
	strncat(result, "1", 1024);
	//diag_printf("WLN0_KEY1_TYPE=1 (ASCII)\n");
	}else if(10 == strlen(value) || 26 == strlen(value)){ //Hexadecimal
	strncat(result, "0", 1024);
	//diag_printf("WLN0_KEY1_TYPE=0 (Hexadecimal)\n");
	} 
	strncat(result, "\r", 1024);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);
	//huangxiaoli add
	_GET_VALUE("wl_mode", value);
	strncat(result, value, 1024);
	strncat(result, "\r", 1024);

	strncat(result, "\n", 1024);
	
	//websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\n\n"));
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, result);
	websDone(wp, 200);	
	return ;
}
#endif


 static void clear_security_parameter(void){
 	_SET_VALUE(WLN0_SECURITY_TYPE, "");
	_SET_VALUE(WLN0_AUTH_MODE, "0");
	_SET_VALUE(WLN0_ENCRYP_TYPE, "tkip+aes");
	_SET_VALUE(WLN0_WPA_PSK1, "12345678");
	return;
}

 static int get_wireless_password(int eid, webs_t wp, int argc, char_t **argv)
 {
 	char_t	*item, *type;
//	int retv=0;
	char *v = NULL;
			
 	if (ejArgs(argc, argv, T("%s %s"), &type, &item) < 2) 
	{
	}
	
	if(0 == strcmp(type,"wireless"))
	{
		if(0 == strcmp(item,"wirelesspassword"))
		{
			 _SAFE_GET_VALUE(WLN0_WPA_PSK1,v);
		}
		else if(0 == strcmp(item,"wpsen"))
		{
			 _SAFE_GET_VALUE(WLN0_WPS_ENABLE,v);
		}
		else if(0 ==  strncmp(item, "SSID", strlen("SSID")))
		{
			_SAFE_GET_VALUE("wl0_ssid", v);	
		}
		return websWrite(wp, T("%s"),v);
	}

	return 0;
 }
int getRedirectWlsecStatus()
{
	char_t *wlSec=NULL;
	char_t *never_prompt_wlpwd=NULL;
	
	_GET_VALUE("wl0_akm", wlSec);
	_GET_VALUE("never_prompt_wlpwd", never_prompt_wlpwd);
	
	if (!strcmp(wlSec,"")&& need_prompt_wlpwd ==1 
		&& atoi(never_prompt_wlpwd)!=1){
		return 1;
	}else{
		return 0;
	}
}
void setRedirectWlsecStatus(int status)
{
	need_prompt_wlpwd = status;
}
	
	
void formWifiPowerInit(webs_t wp, char_t *path, char_t *query)
{	
	char *tmp_value;
	char sigle_mode[8] = {0};
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, "{");
	
	_GET_VALUE(WLN0_POWER, tmp_value);
	
	if (strlen(tmp_value)>2)
	{
		strcpy(sigle_mode,tmp_value);
	}
	else
	{	
		strcpy(sigle_mode,"middle");
	}
		
	websWrite(wp, "\"signal-mode\":\"%s\"", sigle_mode);		
		
	websWrite(wp, "}");	
			
	websDone(wp, 200);	
	
	return;
}

#define _SET_WL_PWR_PENCENT() reset_wl_pwr_percent()


void formWifiPowerSet(webs_t wp, char_t *path, char_t *query)
{
	char *tmp_value;
	char sigle_mode[8] = {0};
	
	tmp_value = websGetVar(wp, T("signal-mode"), T("high"));

	strcpy(sigle_mode,tmp_value);

	_SET_VALUE(WLN0_POWER, sigle_mode);

	/*for NH325: high-19db,middle-17.5db*/
	if(0 == strcmp(sigle_mode, "high"))
	{
		_SET_VALUE(WLN0_PWR_PERCENT, "100");
	}
	else
	{
		_SET_VALUE(WLN0_PWR_PERCENT, "92");
	}
	
	_COMMIT();


	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, "1");
	websDone(wp, 200);
	
	_SET_WL_PWR_PENCENT();

}

void wirelessPwrAspDefine()
{
	
}

void wirelessPwrGoformDefine()
{
	websFormDefine(T("GetPowerControl"),formWifiPowerInit);	
	websFormDefine(T("SetPowerControl"), formWifiPowerSet);
}

void wirelessPwrAspGoformDefine()
{
	wirelessPwrAspDefine();
	
	wirelessPwrGoformDefine();
}



extern int wl_restart_check_main_loop();


void changeWeekForm(char *in_week, char *out_week)
{
	char *ptr;
	if(in_week == NULL ||out_week == NULL )
		return;
	ptr = in_week; 
	while( *ptr!= '\0')
	{
		switch(*ptr)
		{	
			case ',':
				break;
			case '-':
				break;
			case '8':
			case '9':
			case '0':
				printf("illegal character %c\n",*ptr);
				return 0;	
			default:
				if(*ptr >= '1' && *ptr <= '7')//星期天到星期六
				{
					out_week[atoi(ptr) - 1] = '1';
				}
				break;
		}
		ptr++;
	}
	return;
}


extern char* get_product_pwr_info();


void formWifiControlInit(webs_t wp, char_t *path, char_t *query)
{
	char *tmp_value;
	int zone_hh,zone_mm;
	int sec,min,hour;
	char week[16]={0};
	int on_time = 0, off_time = 0;
	char week_tmp[8] ={"0000000"}; 

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
													
	websWrite(wp, "{");
	websWrite(wp, "\"wifi-power\":\"%s\",", get_product_pwr_info());
	websWrite(wp, "\"enable\":\"%s\",", nvram_safe_get(WLN0_CTL_ENABLE));
	
	_GET_VALUE("alilink_wlan_ontime_list1", tmp_value);
	if(tmp_value)
	{
		if(strchr(tmp_value,'+'))
		{
			sscanf(tmp_value,"UTC+%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
			on_time = hour*60*60 + min*60 + sec;
		}
		
	}

	_GET_VALUE("alilink_wlan_offtime_list1", tmp_value);
	if(tmp_value)
	{
		if(strchr(tmp_value,'+'))
		{
			sscanf(tmp_value,"UTC+%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
			off_time = hour*60*60 + min*60 + sec;
		}
		
	}
	if(on_time >= off_time)
	{
		websWrite(wp, "\"time-interval\":\"%d,%d\",",off_time,on_time);
	}
	else
	{
		websWrite(wp, "\"time-interval\":\"%d,86400;0,%d\",",off_time,on_time);
	}
	changeWeekForm(week , week_tmp);
	websWrite(wp, "\"time-round\":\"%s\",",week_tmp);
	websWrite(wp, "\"wanMode\":\"%s\"", nvram_safe_get("wl0_mode"));
	websWrite(wp, "}");
	
	websDone(wp, 200);

	return;
}


void changeWeekForm2(char *in_week, char *out_week)
{
	char *ptr;
	if(in_week == NULL ||out_week == NULL )
		return;
	int n = 0;
	ptr = in_week; 
	while( *ptr!= '\0')
	{
		switch(*ptr)
		{	
			case ',':
				n ++;
				break;
			case '-':
				break;
			case '8':
			case '9':
			case '0':
				printf("illegal character %c\n",*ptr);
				return 0;
			case '7':
				out_week[n] = '1';
				n ++;
			default:
				if(*ptr >= '1' && *ptr <= '6')//星期天到星期六
				{
				
					out_week[n] = *ptr + 1;
					n ++;
				}
				
				break;
		}
		ptr++;
	}	

	int i,j;
	char temp; 

	for(i=0; i<strlen(out_week) ;  i+=2) 
	{
		for(j=i+2; j<strlen(out_week);  j+=2)
		{
			if(out_week[i]>out_week[j]) 
			{ 
				temp=out_week[i]; 
				out_week[i]=out_week[j]; 
				out_week[j]=temp; 
			} 
		}
	}
	return;
}

extern int gWifiStatusConfig;
extern int g_cur_wl_radio_status;
#define WL_RADIO_ON		1
#define WL_RADIO_OFF		0

void formWifiControlSet(webs_t wp, char_t *path, char_t *query)
{
	char *wlctl_enable, *time_round, *time_week, *time_interval,*sche_cnt;
	char time[4][16] = {0};
	int off_time,off_sec,off_min,off_hour;
	int on_time,on_sec,on_min,on_hour;
	char week_new[16] = {0};;
	char time_str[64] = {0};
	int flag = 0;
	wlctl_enable = websGetVar(wp, T("enable"), T("0"));
	time_round = websGetVar(wp, T("time-round"), T("1111111"));//everyday defaultly
	time_interval = websGetVar(wp, T("time-interval"), T(""));
	time_week = websGetVar(wp, T("time-str"), T("1,2,3,4,5,6,7"));
	
	_SET_VALUE(WLN0_CTL_ENABLE, wlctl_enable);
	
	_GET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, sche_cnt);
	if (atoi(sche_cnt) == 0)
	{
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "1");
	}
	
	if (atoi(wlctl_enable))
	{
		
		_SET_VALUE(WLN0_CTL_DAY_ROUND, time_round);
		_SET_VALUE(WLN0_CTL_TIME_INTERVAL, time_interval);
		if(!strstr(time_interval, ";"))
		{
			sscanf(time_interval, "%[^,],%s", time[0], time[1]);
			off_time = atoi( time[0]);
			off_hour = off_time /3600;
			off_min = (off_time - 3600*off_hour)/60;
			off_sec = 0;
			
			on_time = atoi( time[1]);
			on_hour = on_time /3600;
			on_min = (on_time - 3600*on_hour)/60;
			on_sec = 0;
			
			
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour,time_week);
			_SET_VALUE("alilink_wlan_offtime_list1", time_str);
			
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour,time_week);
			_SET_VALUE("alilink_wlan_ontime_list1", time_str);
			
		}
		else
		{
			sscanf(time_interval, "%[^,],%[^;];%[^,],%s", time[0], time[1], time[2], time[3]);
			off_time = atoi( time[0]);
			off_hour = off_time /3600;
			off_min = (off_time - 3600*off_hour)/60;
			off_sec = 0;

			on_time = atoi( time[3]);
			on_hour = on_time /3600;
			on_min = (on_time - 3600*on_hour)/60;
			on_sec = 0;
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour,time_week);
			_SET_VALUE("alilink_wlan_offtime_list1", time_str);

			strcpy(week_new, time_week);
			changeWeekForm2(time_week, week_new);
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour,week_new);
			_SET_VALUE("alilink_wlan_ontime_list1", time_str);
		}
		
	}
	else
	{
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "0");
	}
	
	_COMMIT();	
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=gb2312\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, "1");
	websDone(wp, 200);
				
	gWifiStatusConfig = 0;
	wl_restart_check_main_loop();
		
}
	
	
void wirelessCtlAspDefine()
{

}

void wirelessCtlGoformDefine()
{
	websFormDefine(T("GetWifiControl"),formWifiControlInit);	
	websFormDefine(T("SetWifiControl"), formWifiControlSet);
}

void wirelessCtlAspGoformDefine()
{
	wirelessCtlAspDefine();
	
	wirelessCtlGoformDefine();
}

void wips_to_wds_set_wireless_ap(void)
{
	#if 0
	if(get_vif_ssid())		
	{
		diag_printf("Change wl0.1 to wl0.\n");
		/*step1:AP 接口由wl0.1转移到wl0接口*/
		copy_wl_index_to_unindex("0.1");	//先移到(wl0.1->wl)
		wl_unit("0",1);		//再从wl移动wl0, 用1表示连安全设置也移动		
		//diag_printf("function=%s,line=%d,wl_mode=%s\n", __FUNCTION__,__LINE__,nvram_safe_get("wl_mode"));
	}
	#endif
	/* Step3: 将wl0设置为ap模式*/
	_SET_VALUE("wl_unit", "0");
	_SET_VALUE("wl_mode", "ap");
	_SET_VALUE("wl_radio", "1");
	//set mode
	_SET_VALUE("ure_disable", "1");
	_SET_VALUE("wl_ure", "0");
	//set lan ifname	
	_SET_VALUE("lan_ifnames", "vlan1 eth1");
	//set wan ifname
	_SET_VALUE("wan0_ifname", "vlan2");
	_SET_VALUE("wan_ifnames", "vlan2");
	_SET_VALUE("wan0_ifnames", "vlan2");	
	_SET_VALUE("wan_ifname", "vlan2");
	//set DHCP dhcp
	_SET_VALUE(_LAN0_DHCPD_EN, "dhcp");
	//cdy add
	wl_unit("0",1);			//再从wl移动wl0, 用1表示连安全设置也移动
	//end
	/* Step2: 关闭wl0.1接口*/
	//validate_vif_ssid(nvram_get("wl0.1_ssid"));		//配置wl0.1接口生效
	validate_vif_ssid(NULL);		//配置wl0接口生效
	
}


typedef enum{
MODE_NONE=0,
MODE_WDS,
MODE_APCLIENT,
MODE_WISP
} EXTRA_MODE;

EXTRA_MODE check_extra_mode(void)
{
	char * wds_list = NULL;
	char *wl_mode;
	EXTRA_MODE extra_mode;

	wds_list = nvram_get("wl0_wds");
	wl_mode = nvram_get("wl0_mode");

	if(!strcmp("sta",wl_mode))
		extra_mode = MODE_WISP;
	else if(!strcmp("wet",wl_mode))
		extra_mode = MODE_APCLIENT;
	else if(!strcmp("ap",wl_mode))
	{
		if(wds_list && wds_list[0] != 0)
			extra_mode = MODE_WDS;
		else
			extra_mode = MODE_NONE;
	}
	else
		extra_mode = MODE_NONE;
	return extra_mode;
}

static void set_extra_channel(int channel)
{
	char *htbw;//,*channel	
	_GET_VALUE(WLN0_HT_BW, htbw);
	if(channel == 0 || strncmp(htbw,"0",1) == 0){
		_SET_VALUE(WLN0_HT_EXTCHA, "none");
	}else if(channel < 5 && strncmp(htbw,"0",1) != 0){
		_SET_VALUE(WLN0_HT_EXTCHA, "lower");
	}else if(channel >=5 && strncmp(htbw,"0",1) != 0){
		_SET_VALUE(WLN0_HT_EXTCHA, "upper");
	}	
}

static void set_wireless_security2(webs_t wp)
{
	char *security;
	security = websGetVar(wp, T("security"), T("Disable"));
	//close wps settings
	_SET_VALUE(WLN0_WPS_ENABLE, "disabled");
	if (0 == strcmp(security, "Disable") ) {	// !--- Disable Mode ---
		//0:open; 1:shared
		diag_printf("\n security mode: Disable\n");
		
		_SET_VALUE(WLN0_AUTH_MODE, OPEN);
		_SET_VALUE(WLN0_WEP, DISABLE);
		_SET_VALUE(WLN0_SECURITY_TYPE,"");
		_SET_VALUE(WLN0_WPA_PSK1,"");
		_SET_VALUE(WLN0_ENCRYP_TYPE, "");
	}
	else if(0 == strcmp(security, "0")){		// !--- Open Mode ---
		diag_printf("\n security mode: OPEN\n");	
		
		_SET_VALUE(WLN0_AUTH_MODE, OPEN);
		_SET_VALUE(WLN0_WEP, ENABLE);
		_SET_VALUE(WLN0_SECURITY_TYPE, "0");
		_SET_VALUE(WLN0_WPA_PSK1, "");
		_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
		confWEP( wp);
	}else if(0 == strcmp(security, "1")){	// !--- Shared Mode ---		
		diag_printf("\n security mode: Shared\n");		

		_SET_VALUE(WLN0_AUTH_MODE, SHARED);
		_SET_VALUE(WLN0_WEP, ENABLE);
		_SET_VALUE(WLN0_SECURITY_TYPE, "1");
		_SET_VALUE(WLN0_WPA_PSK1, "");
		_SET_VALUE(WLN0_ENCRYP_TYPE, "wep");
		confWEP( wp);
	}else if(0 == strcmp(security, "psk2") ){// !---  WPA2 Personal Mode ----
		char_t *pass_phrase_str;
		pass_phrase_str = websGetVar(wp, T("passphrase"), T(""));
		diag_printf("\n security mode: psk2\n");	
		
		_SET_VALUE(WLN0_WEP, DISABLE);
		_SET_VALUE(WLN0_AUTH_MODE, OPEN);
		_SET_VALUE(WLN0_SECURITY_TYPE, "psk2");
		_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
		confWPAGeneral( wp);
		
	}else if(0 == strcmp(security, "psk psk2") )	{ //! ----   WPA PSK WPA2 PSK mixed
		char_t *pass_phrase_str;
		pass_phrase_str = websGetVar(wp, T("passphrase"), T(""));
		diag_printf("\n security mode: PSK PSK2\n");
		
		_SET_VALUE(WLN0_WEP, DISABLE);
		_SET_VALUE(WLN0_AUTH_MODE, OPEN);
		_SET_VALUE(WLN0_SECURITY_TYPE, "psk psk2");
		_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
		confWPAGeneral( wp);
	
	}else if(0 == strcmp(security, "psk")){	// !---  WPA Personal Mode ---
		char_t *pass_phrase_str;
		pass_phrase_str = websGetVar(wp, T("passphrase"), T(""));	
		diag_printf("\n security mode: PSK\n");
		
		_SET_VALUE(WLN0_WEP, DISABLE);
		_SET_VALUE(WLN0_AUTH_MODE, OPEN);
		_SET_VALUE(WLN0_SECURITY_TYPE, "psk");
		_SET_VALUE(WLN0_WPA_PSK1, pass_phrase_str);
		confWPAGeneral( wp);
	}
	else
		diag_printf("Unsupport security.\n");
}

static void set_extra_ssid(webs_t wp, char *wlname)
{
	char * ssid, * channel;

	ssid = websGetVar(wp, T("ssid"), T(""));
	channel = websGetVar(wp, T("channel"), T("0")); 
	_SET_VALUE(WLN0_SSID0, ssid);
	//only main ssid can set channel(use wl0)
	if(!strcmp("wl0",wlname))
	{	
		_SET_VALUE(WLN0_CHANNEL, channel);
		set_extra_channel(atoi(channel));
	}
	
	set_wireless_security2(wp);
	if(!strcmp("wl0",wlname)){			//copy wl_XXX to wl0_XXX  wds时为什么还要移动到wl0
		wl_unit("0",1);
	}
	else if(!strcmp("wl0.1",wlname)){	//copy wl_XXX to wl0.1_XXX
		wl_unit("0.1",1);
	}
	else
		diag_printf("unsupport interface %s!\n",wlname);
			
}


 static void set_wireless_wds(webs_t wp)
{
	char *wds_list,*old_ssid;	
	wds_list = websGetVar(wp, T("wds_list"), T(""));   //先以单个MAC测试,如00:11:22:33:44:55
	diag_printf("WDS mode.wds_list=%s\n",wds_list);
	if(strlen(wds_list)>1)
	{	
		/* Step1: 配置AP接口(wl0.1)
		if(strcmp(nvram_get("wl0.1_ssid"),"")==0){
			//将原来的主SSID配置移动到次SSID
			copy_wl_index_to_unindex("0");	//先移到(wl0->wl)
			wl_unit("0.1",1);			//再从wl移动wl0.1, 用1表示连安全设置也移动
			
			_GET_VALUE("wl0_ssid",old_ssid);
			validate_vif_ssid(old_ssid);		//配置wl0.1接口生效
		}*/
		strtoupper(wds_list);
		_SET_VALUE(WLN0_WDS_TIMEOUT, "1");
		_SET_VALUE(WLN0_WDS_LIST, wds_list);
	}
	else
		diag_printf("wds list is null!\n");
	set_extra_ssid(wp,"wl0");
} 

static void clear_wds_settings(void)
{
	char *wds_list;
	
	 _GET_VALUE("wl0_wds",wds_list);
	if(strcmp("",wds_list) != 0)
	{
		_SET_VALUE("wds_wds_timeout","0");
		_SET_VALUE("wl0_wds","");
	}
}


 static void set_wireless_wisp(webs_t wp)
 {
 	char *old_ssid;
	//clear_wds_settings();
	/* Step1: 配置AP接口(wl0.1)*/
	if(strcmp(nvram_get("wl0.1_ssid"),"")==0){
	//将原来的主SSID配置移动到次SSID
		copy_wl_index_to_unindex("0");	//先移到(wl0->wl)
		wl_unit("0.1",1);			//再从wl移动wl0.1, 用1表示连安全设置也移动
		_GET_VALUE("wl0_ssid",old_ssid);
		validate_vif_ssid(old_ssid);		//配置wl0.1接口生效
	}
	/* Step2: 配置WISP接口(wl0)*/
	//set mode
	_SET_VALUE("ure_disable", "0");
	_SET_VALUE("wl_ure", "1");
	_SET_VALUE("wl_mode", "sta");
	_SET_VALUE("wl_wds", "");

	//remove wireless mac filter
	_SET_VALUE(WLN0_MAC_MODE, "disabled");
	_SET_VALUE(WLN0_MAC_LIST, "");
	
	//set lan ifname
	_SET_VALUE(_LAN0_IFNAMES, "vlan1 wl0.1");
	//set wan ifname
	_SET_VALUE(_WAN0_IFNAME, "eth1");
	_SET_VALUE("wan_ifnames", "eth1");
	_SET_VALUE("wan0_ifnames", "eth1");
	_SET_VALUE("wan_ifname", "eth1");

	//set DHCP dhcp
	_SET_VALUE(_LAN0_DHCPD_EN, "dhcp");
	set_extra_ssid(wp,"wl0");		//set ssid,channel and security to wl0
	return;
	
 }


 static void set_wireless_apclient(webs_t wp)
{
	char *old_ssid, *wds_list;

	clear_wds_settings();
	/* Step1: 配置AP接口(wl0.1)*/
	if(strcmp(nvram_get("wl0.1_ssid"),"")==0){
	//将原来的主SSID配置移动到次SSID
		copy_wl_index_to_unindex("0");	//先移到(wl0->wl)
		wl_unit("0.1",1);			//再从wl移动wl0.1, 用1表示连安全设置也移动
		_GET_VALUE("wl0_ssid",old_ssid);	
		validate_vif_ssid(old_ssid);		//配置wl0.1接口生效
	}
/* Step2: 配置client 接口(wl0)*/
	//set mode
	_SET_VALUE("ure_disable", "0");		
	_SET_VALUE("wl_ure", "1");
	_SET_VALUE("wl_mode", "wet");		//wet模式才能转发AP接口数据

	//set lan ifname	
	_SET_VALUE(_LAN0_IFNAMES, "vlan1 eth1 wl0.1");	
	
	//set wan ifname
	_SET_VALUE(_WAN0_IFNAME, "vlan2");
	_SET_VALUE("wan_ifnames", "vlan2");
	_SET_VALUE("wan0_ifnames", "vlan2");
	_SET_VALUE("wan_ifname", "vlan2");

	//set dhcp static
	_SET_VALUE(_LAN0_DHCPD_EN, "static");
	set_extra_ssid(wp,"wl0");		//set ssid,channel and security to wl0
	return;
}

 void set_wireless_ap(void)
{
	if(get_vif_ssid())		
	{
		diag_printf("Change wl0.1 to wl0.\n");
		/*step1:AP 接口由wl0.1转移到wl0接口*/
		copy_wl_index_to_unindex("0.1");	//先移到(wl0.1->wl)
		wl_unit("0",1);		//再从wl移动wl0, 用1表示连安全设置也移动		
		//diag_printf("function=%s,line=%d,wl_mode=%s\n", __FUNCTION__,__LINE__,nvram_safe_get("wl_mode"));
	}
	/* Step3: 将wl0设置为ap模式*/
	_SET_VALUE("wl_unit", "0");
	_SET_VALUE("wl_mode", "ap");
	_SET_VALUE("wl_radio", "1");
	//set mode
	_SET_VALUE("ure_disable", "1");
	_SET_VALUE("wl_ure", "0");
	//set lan ifname	
	_SET_VALUE("lan_ifnames", "vlan1 eth1");
	//set wan ifname
	_SET_VALUE("wan0_ifname", "vlan2");
	_SET_VALUE("wan_ifnames", "vlan2");
	_SET_VALUE("wan0_ifnames", "vlan2");	
	_SET_VALUE("wan_ifname", "vlan2");
	//set DHCP dhcp
	_SET_VALUE(_LAN0_DHCPD_EN, "dhcp");
	//cdy add
	wl_unit("0",1);			//再从wl移动wl0, 用1表示连安全设置也移动
	//end
	/* Step2: 关闭wl0.1接口*/
	validate_vif_ssid(NULL);		//配置wl0接口生效
	
}

 
static void set_wireless_extra(webs_t wp, char_t *path, char_t *query)
{
	char * extra_mode;
	EXTRA_MODE old_mode;
	bool reboot = true;//false;
			
	extra_mode = websGetVar(wp, T("extra_mode"), T("none")); 
	old_mode = check_extra_mode();
	
	if(!strcmp("wds", extra_mode))
	{
		set_wireless_wds(wp);
		
		if(old_mode == MODE_APCLIENT || old_mode == MODE_WISP)
			//set_wireless_ap();
			wips_to_wds_set_wireless_ap();
	}
	else if(!strcmp("wisp", extra_mode))
	{
		clear_wds_settings();
		set_wireless_wisp(wp);
	}
	else if(!strcmp("apclient", extra_mode))
	{
		clear_wds_settings();
		set_wireless_apclient(wp);
	}
	else if(!strcmp("none", extra_mode))
	{
		_SET_VALUE("wl0.1_hwaddr", "");
		if(old_mode == MODE_WDS)
		{
			diag_printf("\n clear_wds_settings \n");
			clear_wds_settings();
		}
		else if(old_mode == MODE_APCLIENT || old_mode == MODE_WISP)
			set_wireless_ap();
	}
	if(strcmp("none", extra_mode))
	{
		/* ldf add, follow Tenda wps standard edited by wangwanshun */
		nvram_set(WLN0_WPS_OOB, "disabled");
	}
	_COMMIT();
	cyg_thread_delay(100);

	/*if((old_mode == MODE_WDS || old_mode == MODE_NONE) &&
		strcmp("none",extra_mode) && strcmp("wds",extra_mode))
		reboot = true;
	else if(old_mode == MODE_APCLIENT && strcmp("apclient",extra_mode))
		reboot = true;
	else if(old_mode == MODE_WISP && strcmp("wisp",extra_mode))
		reboot = true;*/

	if(reboot)
	{
		sys_reboot();
		websRedirect(wp, T("/direct_reboot.asp"));
	}
	else
	{
		_RESTART();
		websRedirect(wp,T( "/wireless_extra.asp"));
	}

}

