
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shutils.h>

#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#define NVRAM_BASIC	0x40000000 /* from basic settings */
#define NVRAM_SECURITY		0x80000000 /* from security settings */
#define NVRAM__IGNORE		0x01000000 /* Don't save or restore to NVRAM */
#define NVRAM_VLAN_MULTI	0x02000000 /* Multi instance VLAN variable vlanXXname */
#define NVRAM_GENERIC_MULTI	0x04000000 /* Port forward multi instance variables nameXX */
#define NVRAM_IGNORE	0x08000000 /* Skip NVRAM processing ie no save to file or validate */
#define NVRAM_MI		0x00100000 /* Multi instance NVRAM variable */
#define WEB_IGNORE		0x00200000 /* Ignore during web validation */
#define VIF_IGNORE		0x00400000 /* Ignore from wlx.y ->wl and from wl->wlx.y */

struct variable {
	char *name;
	char *longname;
	int ezc_flags;
};

struct variable wl_variables[] = {	
	/***************2.4G基本参数（中间参数）****************/
	{ "wl_ssid", "Network Name (ESSID)", NVRAM_MI},
	{ "wl_radio", "Radio Enable", NVRAM_MI },
	{ "wl_closed", "hide ssid", NVRAM_MI},
	{ "wl_auth", "802.11 Authentication", NVRAM_MI},
	{ "wl_akm", "Authenticated Key Management", NVRAM_MI},
	{ "wl_wep", "WEP Encryption", NVRAM_MI},
	{ "wl_crypto", "WPA Encryption", NVRAM_MI},
	{ "wl_wpa_psk", "WPA Pre-Shared Key", NVRAM_MI},
	{ "wl_channel", "Channel", NVRAM_MI},
    { "wl_bandwidth", "Channel Bandwidth", NVRAM_MI},
	{ "wl_bandside", "Control Sideband", NVRAM_MI},
	{ "wl_nettype", "Network Type", NVRAM_MI},
	{ "wl_ctv_power", "Current power", NVRAM_MI},
	{ "wl_mode", "work Mode", NVRAM_MI},
	{ "wl_macmode", "MAC Restrict Mode", NVRAM_MI},
	{ "wl_maclist", "Allowed MAC Address", NVRAM_MI},
	{ "wl_plcphdr", "150M 135M", NVRAM_MI},
	{ "wl_ht_bw_fake_auto", "auto bandwith", NVRAM_MI},

	{ "wl_unit", "802.11 Instance", NVRAM_MI},
	/***************5G基本参数（中间参数）****************/
	{ "wl5g_ssid", "Network Name (ESSID)", NVRAM_MI},
	{ "wl5g_radio", "Radio Enable", NVRAM_MI },
	{ "wl5g_closed", "Network Type", NVRAM_MI},
	{ "wl5g_auth", "802.11 Authentication", NVRAM_MI},
	{ "wl5g_wep", "WEP Encryption", NVRAM_MI},
	{ "wl5g_akm", "Authenticated Key Management", NVRAM_MI},
	{ "wl5g_crypto", "WPA Encryption", NVRAM_MI},
	{ "wl5g_wpa_psk", "WPA Pre-Shared Key", NVRAM_MI},
	{ "wl5g_channel", "Channel", NVRAM_MI},
	{ "wl5g_bandside", "Control Sideband", NVRAM_MI},
	{ "wl5g_bandwidth", "Channel Bandwidth", NVRAM_MI},
	{ "wl5g_nettype", "54g Mode", NVRAM_MI},
	{ "wl5g_ctv_power", "Current power", NVRAM_MI},
	{ "wl5g_mode", "work Mode", NVRAM_MI},
	{ "wl_macmode", "MAC Restrict Mode", NVRAM_MI},
	{ "wl_maclist", "Allowed MAC Address", NVRAM_MI},
	{ "wl5g_plcphdr", "150M 135M", NVRAM_MI},
	{ "wl5g_ht_bw_fake_auto", "auto bandwith", NVRAM_MI},
	
	{ "wl5g_unit", "802.11 Instance",NVRAM_MI},
	{ NULL, NULL, 0}
};

/* Copy all wl%d_XXXXX to wl_XXXXX */
void
copy_wl_index_to_unindex(char *wl_index_str,char *wl_str)
{
	struct variable *v=NULL;
	char tmp[PI_BUFLEN_128];
	char wl_index[PI_BUFLEN_32] = {0};
	char wl[PI_BUFLEN_32] = {0};
	char *val = NULL;
	int i = 0;
	
	if(NULL == wl_index_str || NULL == wl_str)
	{
		return;
	}
	
	snprintf(wl_index,sizeof(wl_index),"%s_",wl_index_str);
	snprintf(wl,sizeof(wl),"%s_",wl_str);

	for(i = 0; i < sizeof(wl_variables)/sizeof(wl_variables[0]);i++)
	{
		v = &wl_variables[i];
		if(strncmp(v->name, wl, strlen(wl)))
			continue;

		nvram_set(v->name, nvram_safe_get(strcat_r(wl_index, &v->name[strlen(wl)], tmp)));
		
		memset(tmp,0x0,sizeof(tmp));
		if (!strncmp(v->name, strcat_r(wl,"unit",tmp), 7)) 
		{
			break;
		}
	}
	return;
}

/* Hook to write wl_* default set through to wl%d_* variable set */
void
copy_wl_to_wl_index(char *wl_str,char *wl_index_str)
{
	struct variable *v = NULL;
	char tmp[PI_BUFLEN_128];
	char wl_index[PI_BUFLEN_32] = {0};
	char wl[PI_BUFLEN_32] = {0};
	int i = 0;
	
	if(NULL == wl_str || NULL == wl_index_str)
	{
		return;
	}

	snprintf(wl_index,sizeof(wl_index),"%s_",wl_index_str);
	snprintf(wl,sizeof(wl),"%s_",wl_str);
	
	for(i =0; i < sizeof(wl_variables)/sizeof(wl_variables[0]); i++)
	{
	 	v = &wl_variables[i];
		
		if(strncmp(v->name,wl,strlen(wl)))
			continue;
		
		nvram_set(strcat_r(wl_index, &v->name[strlen(wl)], tmp), nvram_safe_get(v->name));

		memset(tmp,0x0,sizeof(tmp));
		if (!strncmp(v->name, strcat_r(wl,"unit",tmp), 7)) 
		{
			break;
		}
	}

	return;
}

static void
vif_lan_bridge_op(int add, char *wl_unit)
{
	char vif[]= "XXXXXX_";
	char vif_name[] = "xxxxxxxxxxxxx_" ;
	char del_br[] = "xxxxxxxxxxxxx_" ;
	char add_br[] = "xxxxxxxxxxxxx_" ;
	char name[IFNAMSIZ] = {0};
	char *next = NULL;
	int need_to_add = 1;
	char buf[255];

	snprintf(vif_name, sizeof(vif_name), "wl%s_ifname",wl_unit) ;

	snprintf(vif,sizeof(vif),"%s",nvram_safe_get(vif_name));

	snprintf(del_br, sizeof(del_br), "lan_ifnames") ;
	snprintf(add_br, sizeof(add_br), "lan_ifnames") ;

	memset(buf,0,sizeof(buf));
	/* Copy all the interfaces except the the one we want to remove*/
	foreach(name, nvram_safe_get(del_br),next) {
		if (strcmp(name,vif)){
			int len;
			len = strlen(buf);
			if (*buf)
				strncat(buf," ",1);
			strncat(buf,name,strlen(name));
		}
	}

	if(!add)
		nvram_set(del_br, buf);
	
	memset(buf,0,sizeof(buf));
	strcpy(buf, nvram_safe_get(add_br));
	foreach(name, buf, next) {
		if (!strcmp(name,vif)){
			need_to_add = 0;
			break;
		}
	}

	if (need_to_add && add) {
		/* the first entry ? */
		if (*buf)
			strncat(buf," ",1);
		strncat(buf, vif, strlen(vif));
	}

	nvram_set(add_br, buf);
	
}

void
validate_vif_ssid(char *vif_ssid)
{
/* Validation of the guest ssids does 3 things
 * 1)adds the wlX.Y_ssid field
 * 2)updates the wlX_vif list
 * 3)removes entry from wlX_vif if the interface is empty
*/
	
	

	char wl_vif[]="wlXXXXXXXXX_vifs",*wl_vif_value=NULL;
	char wl_ssid[]="wlXXXXXXXXX_ssid";
	char wl_radio[]="wlXXXXXXXXX_radio";
	char wl_mode[]="wlXXXXXXXXX_mode";
	char vif[]="wlXXXXXXXXX";
	char buf[100];

	
//we only use wl0.1	
	char subunit[]="1",unit[]="0";

	snprintf(wl_ssid,sizeof(wl_ssid),"wl_ssid");
	snprintf(wl_vif,sizeof(wl_vif),"wl%s_vifs",unit);

	memset(buf,0,sizeof(buf));

	/* This logic here decides if updates to virtual interface list on the
	   parent is required */

	
	wl_vif_value = nvram_get(wl_vif);

	
	if( (wl_vif_value != NULL))
	{
		 if(strlen(wl_vif_value) < 2)
		 {
		 	 wl_vif_value = NULL;
		 }
		 
	}
	

//我们只做两个SSID,wl_vif_value为真表示第二个已经启用
	if (wl_vif_value){
		//already exist vif
		//printf("%s:%d-----wl_vif_value = %s-----vif_ssid=%s-----\n", __FUNCTION__, __LINE__, wl_vif_value, vif_ssid);	
			snprintf(vif,sizeof(vif),"wl%s.%s",unit,subunit);

			if (vif_ssid){
				//设置了ssid,表示第二个ssid启用
				snprintf(buf,sizeof(buf),"%s",wl_vif_value);
			}else{
				//没有设置，清掉wl_vif里的内容
				nvram_unset(wl_vif);
	
			}
	}else{
		//原来没有启用,现在启用

		if (vif_ssid) snprintf(buf,sizeof(buf),"wl%s.%s",unit,subunit);
		
	}	
	
	/* Update/clean up wlX.Y_guest flag and wlX.Y_ssid */

	snprintf(wl_radio,sizeof(wl_radio),"wl_radio");
	snprintf(wl_mode,sizeof(wl_mode),"wl_mode");

	snprintf(vif,sizeof(vif),"%s.%s",unit,subunit);

	if (vif_ssid){
		nvram_set(wl_mode,"ap");
		nvram_set(wl_ssid,vif_ssid);
	}else{
		nvram_unset(wl_ssid);
		nvram_unset(wl_radio);
		nvram_unset(wl_mode);
		nvram_unset("wl_bss_enabled");
	}

	//wl_* default set through to wl%d_* 
	if(nvram_match("wl_unit", "0")){
		//for set wl0.1_unit = 0.1
		nvram_set("wl_unit",vif);
		//wl_unit(vif,0);   //接口有改动??????????????
		nvram_set("wl_unit","0");
	}else{
		//wl_unit(vif,0); //接口有改动??????????????
	}

	/* Regenerate virtual interface list */
	if (!wl_vif_value && *buf){
		//原来没有启用,现在启用,第一次开启次SSID
		nvram_set(wl_vif,buf);
		snprintf(wl_vif,sizeof(wl_vif),"%s_ifname",buf);
		nvram_set(wl_vif,buf);//set wl0.1_ifname
		//snprintf(wl_vif,sizeof(wl_vif),"%s_wps_mode",buf);
		//nvram_set(wl_vif,"disable");
		if(! nvram_match("ure_disable","0")){
			//不是ure时
			snprintf(wl_vif,sizeof(wl_vif),"%s_wps_mode",buf);
#ifdef __CONFIG_WPS_LED__
			//次SSID开启WPS
			//nvram_set(wl_vif,"enabled");//set wl0.1_wps_mode
			snprintf(wl_vif,sizeof(wl_vif),"%s_wps_method",buf);
			nvram_set(wl_vif,"pbc");//set wl0.1_wps_method
#else
			//次SSID关闭WPS
			nvram_set(wl_vif,"disabled");//set wl0.1_wps_mode
#endif
		}
	}
	vif_lan_bridge_op(vif_ssid?1:0,vif);
}

char *get_vif_ssid()
{
	char vif[]="wlXXXXXXXXXXXXXXXXXXXXX";
	char buf[100];
	
//we only use wl0.1	
	char unit[]="0";

	snprintf(vif,sizeof(vif),"wl%s_vifs",unit);
	memset(buf,0,sizeof(buf));

	/* This logic here decides if updates to virtual interface list on the
	   parent is required */

	snprintf(buf,sizeof(buf),"%s",nvram_safe_get(vif));
	if(strlen(buf) >strlen("wl0")){
		snprintf(vif,sizeof(vif),"%s_ssid",buf);
		return nvram_safe_get(vif);//get wl0.1_ssid
	}
	else{
		return NULL;
	}
}

char *get_wl0_passwd(char *value)
{
	char *sec;
	char *keyID, *wep_passwd, *wpa_passwd ;
	char tmp[16] = "wl0_key";

	_GET_VALUE("wl0_akm",sec);
	_GET_VALUE("wl0_key",keyID);
	_GET_VALUE(strcat(tmp,keyID),wep_passwd);
	_GET_VALUE("wl0_wpa_psk",wpa_passwd);
	
	if(!strcmp(sec,"")){
		/*disabled*/
		strcpy(value,"");
	}
	else if(!strcmp(sec,"0")||!strcmp(sec,"1")){
		/*wep*/
		strcpy(value,wep_passwd);
	}
	else if(!strcmp(sec,"psk")||!strcmp(sec,"psk2")||!strcmp(sec,"psk psk2")){
		/*wap*/
		strcpy(value,wpa_passwd);
	}
	else{
		/*unknown security type*/
		strcpy(value,"");
	}
	
	return value;
}

char *get_wl_mode(WLAN_RATE_TYPE wl_rate,char *value)
{
	char *mode;

	if(NULL == value)
	{
		return;
	}
	if(WLAN_RATE_24G == wl_rate)
	{
		_GET_VALUE(WLAN24G_WORK_MODE,mode);
	}	
	if(WLAN_RATE_5G == wl_rate)
	{
		_GET_VALUE(WLAN5G_WORK_MODE,mode);
	}
	
	strcpy(value,mode);
	
	return value;
}



