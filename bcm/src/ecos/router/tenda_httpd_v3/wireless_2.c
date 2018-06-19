
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shutils.h>

#include "route_cfg.h"
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"

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
	/* ALL wl_XXXX variables are per-interface  */
	/* This group is per ssid */
	{ "wl_bss_enabled", "BSS Enable", NVRAM_MI },
	{ "wl_ssid", "Network Name (ESSID)", NVRAM_MI},
	{ "wl_bridge", "Bridge Details", NVRAM_MI},
	{ "wl_closed", "Network Type", NVRAM_MI},
	{ "wl_ap_isolate", "AP Isolate", NVRAM_MI },
	{ "wl_wmf_bss_enable", "WMF Enable", NVRAM_MI },
	{ "wl_macmode", "MAC Restrict Mode", NVRAM_MI | NVRAM_SECURITY },
	{ "wl_maclist", "Allowed MAC Address", NVRAM_MI | NVRAM_SECURITY  },
	{ "wl_mode", "Mode", NVRAM_MI },
	{ "wl_infra", "Network", NVRAM_MI },
	{ "wl_bss_maxassoc", "Per BSS Max Association Limit", NVRAM_MI },
	{ "wl_wme_bss_disable", "Per-BSS WME Disable", NVRAM_MI },


	/* This group is per radio */
	{ "wl_ure", "URE Mode", NVRAM_MI | VIF_IGNORE },
	{ "wl_vifs", "WL Virtual Interfaces", NVRAM_MI | WEB_IGNORE },
	{ "wl_country_code", "Country Code", NVRAM_MI | VIF_IGNORE},
	{ "wl_lazywds", "Bridge Restrict", NVRAM_MI | VIF_IGNORE },
	{ "wl_wds", "Bridges", NVRAM_MI | VIF_IGNORE },
	{ "wl_wds_timeout", "Link Timeout Interval", NVRAM_MI | VIF_IGNORE},
	{ "wl_radio", "Radio Enable", NVRAM_MI },
	{ "wl_phytype", "Radio Band", NVRAM_MI | VIF_IGNORE },
	{ "wl_antdiv", "Antenna Diversity", NVRAM_MI | VIF_IGNORE },
	/* Channel and rate are fixed in wlconf() if incorrect */
	{ "wl_channel", "Channel", NVRAM_MI | VIF_IGNORE},
	{ "wl_reg_mode", "Regulatory Mode", NVRAM_MI | VIF_IGNORE},
	{ "wl_dfs_preism", "Pre-Network Radar Check", NVRAM_MI | VIF_IGNORE},
	{ "wl_dfs_postism", "In Network Radar Check", NVRAM_MI | VIF_IGNORE},
	{ "wl_tpc_db", "TPC Mitigation (db)",  NVRAM_MI | VIF_IGNORE},
	{ "wl_rate", "Rate", NVRAM_MI | VIF_IGNORE },
	{ "wl_rateset", "Supported Rates", NVRAM_MI | VIF_IGNORE},
	{ "wl_mrate", "Multicast Rate", NVRAM_MI | VIF_IGNORE },
	{ "wl_frag", "Fragmentation Threshold", NVRAM_MI | VIF_IGNORE },
	{ "wl_rts", "RTS Threshold", NVRAM_MI | VIF_IGNORE },
	{ "wl_dtim", "DTIM Period", NVRAM_MI | VIF_IGNORE },
	{ "wl_bcn", "Beacon Interval", NVRAM_MI | VIF_IGNORE},
	{ "wl_bcn_rotate", "Beacon Rotation", NVRAM_MI | VIF_IGNORE },
	{ "wl_plcphdr", "Preamble Type", NVRAM_MI | VIF_IGNORE },
	{ "wl_maxassoc", "Max Association Limit", NVRAM_MI | VIF_IGNORE },
	{ "wl_gmode", "54g Mode", NVRAM_MI | VIF_IGNORE },
	{ "wl_gmode_protection", "54g Protection", NVRAM_MI | VIF_IGNORE },
	{ "wl_frameburst", "XPress Technology", NVRAM_MI | VIF_IGNORE},
	{ "wl_afterburner", "AfterBurner Technology", NVRAM_MI | VIF_IGNORE},
	{ "wl_nband", "Radio Band for EWC", NVRAM_MI | VIF_IGNORE },
	{ "wl_nbw_cap", "Channel Bandwidth", NVRAM_MI | VIF_IGNORE},
	{ "wl_nctrlsb", "Control Sideband", NVRAM_MI | VIF_IGNORE},
	{ "wl_nmcsidx", "MCS Index", NVRAM_MI | VIF_IGNORE},
	{ "wl_nmode", "802.11 N mode", NVRAM_MI | VIF_IGNORE},
	{ "wl_txchain", "Number of TxChains", NVRAM_MI | VIF_IGNORE },
	{ "wl_rxchain", "Number of RxChains", NVRAM_MI | VIF_IGNORE },
	{ "wl_nreqd", "require 802.11n support", NVRAM_MI | VIF_IGNORE },
	{ "wl_vlan_prio_mode", "VLAN Priority Support", NVRAM_MI | VIF_IGNORE},
	{ "wl_nmode_protection", "802.11n Protection", NVRAM_MI | VIF_IGNORE },
	{ "wl_mimo_preamble", "802.11n Preamble", NVRAM_MI | VIF_IGNORE },
	{ "wl_rifs", "Enable/Disable RIFS Transmissions", NVRAM_MI | VIF_IGNORE},
	{ "wl_rifs_advert", "RIFS Mode Advertisement", NVRAM_MI | VIF_IGNORE},
	{ "wl_stbc_tx", "Enable/Disable STBC Transmissions", NVRAM_MI | VIF_IGNORE},
	{ "wl_amsdu", "MSDU aggregation Technology", NVRAM_MI | VIF_IGNORE },
	{ "wl_ampdu", "MPDU aggregation Technology", NVRAM_MI | VIF_IGNORE },
	{ "wl_wme", "WME Support", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_no_ack", "No-Acknowledgement", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_apsd", "U-APSD Support", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_ap_be", "WME AP BE", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_ap_bk", "WME AP BK", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_ap_vi", "WME AP VI", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_ap_vo", "WME AP VO", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_sta_be", "WME STA BE", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_sta_bk", "WME STA BK", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_sta_vi", "WME STA VI", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_sta_vo", "WME STA VO", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_txp_be", "WME TXP BE", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_txp_bk", "WME TXP BK", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_txp_vi", "WME TXP VI", NVRAM_MI | VIF_IGNORE},
	{ "wl_wme_txp_vo", "WME TXP VO", NVRAM_MI | VIF_IGNORE},
	{ "wl_obss_coex", "Overlapping BSS Coexistence", NVRAM_MI | VIF_IGNORE },
	/* security parameters */
	{ "wl_key", "Network Key Index", NVRAM_MI|NVRAM_SECURITY },
	{ "wl_key1", "Network Key 1", NVRAM_MI|NVRAM_SECURITY},
	{ "wl_key2", "Network Key 2", NVRAM_MI|NVRAM_SECURITY},
	{ "wl_key3", "Network Key 3", NVRAM_MI|NVRAM_SECURITY},
	{ "wl_key4", "Network Key 4", NVRAM_MI|NVRAM_SECURITY},
	{ "wl_auth", "802.11 Authentication", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_auth_mode", "Network Authentication", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_akm", "Authenticated Key Management", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_wep", "WEP Encryption", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_crypto", "WPA Encryption", NVRAM_MI |NVRAM_SECURITY },

	{ "wl_radius_ipaddr", "RADIUS Server", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_radius_port", "RADIUS Port", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_radius_key", "RADIUS Shared Secret", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_wpa_psk", "WPA Pre-Shared Key", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_wpa_gtk_rekey", "Network Key Rotation Interval", NVRAM_MI |NVRAM_SECURITY},
	/* Multi SSID Guest interface flag */
	{ "wl_guest", "Guest SSID Interface", WEB_IGNORE|NVRAM_MI},
	{ "wl_sta_retry_time", "STA Retry Time", NVRAM_MI},
	/* WPS Setting */
	{ "wl_wps_mode", "WPS Mode", NVRAM_MI|NVRAM_SECURITY},
	{ "wl_wps_pin", "WPS PIN", NVRAM_MI |NVRAM_SECURITY},
	{ "wl_wps_method", "WPS Method", NVRAM_MI |NVRAM_SECURITY},
	//{ "wl_old_ssid", "WPS Old SSID", NVRAM_MI |NVRAM_SECURITY},
	/* MUST leave this entry here after all wl_XXXX variables */
	{ "wl_unit", "802.11 Instance", NVRAM_IGNORE|NVRAM_MI},
	{ NULL, NULL, 0}
};

/* Copy all wl%d_XXXXX to wl_XXXXX */
void
copy_wl_index_to_unindex(char *unit_str)
{
	struct variable *v=NULL;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";

	if (unit_str) {
		snprintf(prefix, sizeof(prefix), "wl%s_", unit_str);
	
		for (v = &wl_variables[0]; v->name && !strncmp(v->name, "wl_", 3); v++){
			char *val = NULL;
			if ((v->ezc_flags & WEB_IGNORE) || ((prefix[3] == '.') && (v->ezc_flags & VIF_IGNORE))) {
				continue;
			}
			if (!strncmp(v->name, "wl_", 3)) {
				(void)strcat_r(prefix, &v->name[3], tmp);
				/* 
				 * tmp holds fully qualified name (wl0.1_....)
				 * First try nvram; if NULL, try default; ? : is gcc-specific
				 */
				val = nvram_get(tmp) ? : nvram_default_get(tmp);
				if (val == NULL) {
					val = "";
				}
				nvram_set(v->name, val);
			}
			if (!strncmp(v->name, "wl_unit", 7)) {
				break;
			}
		}
	}

	/* Set currently selected unit */
	nvram_set("wl_unit", unit_str);
}

/* Hook to write wl_* default set through to wl%d_* variable set */
void
wl_unit(char *unit,int from_secu)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";

	if (!unit) return;

	struct variable *v = &wl_variables[0];
	
	/* The unit numbers are built dynamically so what is
	   present is assumed to be running */

	snprintf(prefix,sizeof(prefix),"wl%s_",unit);

	/* Write through to selected variable set 
	 * If the VIF_IGNORE flag is set, we still need to write if the interface is the 
	 * physical device. 
	 */

	for (; v->name && !strncmp(v->name, "wl_", 3); v++){
		if (( v->ezc_flags & WEB_IGNORE) || ((v->ezc_flags & VIF_IGNORE) && (prefix[3] == '.')) ||
			(!from_secu && (v->ezc_flags & NVRAM_SECURITY) && (prefix[3] == '.'))) 
			continue;
		nvram_set(strcat_r(prefix, &v->name[3], tmp), nvram_safe_get(v->name));
	}
}

static void
vif_lan_bridge_op(int add, char *wl_unit)
{
	char vif[]= "XXXXXX_";
	char vif_name[] = "xxxxxxxxxxxxx_" ;
	char del_br[] = "xxxxxxxxxxxxx_" ;
	char add_br[] = "xxxxxxxxxxxxx_" ;
	char name[IFNAMSIZ], *next = NULL;
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
	

	
#if 0
	/* Regenerate virtual interface list */
	if (!wl_vif_value && *buf){
		//原来没有启用,现在启用,第一次开启次SSID
		nvram_set(wl_vif,buf);
		snprintf(wl_vif,sizeof(wl_vif),"%s_ifname",buf);
		nvram_set(wl_vif,buf);//set wl0.1_ifname
	}
#endif
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
		//cdy add 
		nvram_unset("wl0.1_hwaddr");
		nvram_unset("wl0.1_ifname");
		//end
	}

	//wl_* default set through to wl%d_* 
	if(nvram_match("wl_unit", "0")){
		//for set wl0.1_unit = 0.1
		nvram_set("wl_unit",vif);
		wl_unit(vif,0);
		nvram_set("wl_unit","0");
	}else{
		wl_unit(vif,0);
	}

	/* Regenerate virtual interface list */
	if (!wl_vif_value && *buf){
		//原来没有启用,现在启用,第一次开启次SSID
		nvram_set(wl_vif,buf);
		snprintf(wl_vif,sizeof(wl_vif),"%s_ifname",buf);
		nvram_set(wl_vif,buf);//set wl0.1_ifname
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
			nvram_set(wl_vif,"disable");//set wl0.1_wps_mode
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

char *get_wl0_ssid()
{
	char vif[]="wlXXXXXXXXXXXXXXXXXXXXX";
	char unit[]="0";

	snprintf(vif,sizeof(vif),"wl%s_ssid",unit);
	return nvram_safe_get(vif);//get wl0_ssid
}

int is_wl0_vifs(char *unit_str)
{
	char vif[]="wlXXXXXXXXXXXXXXXXXXXXX";
	char vif_name[]="wlXXXXXXXXXXXXXXXXXXXXX";
	char buf[100];
	char name[IFNAMSIZ], *next = NULL;

	int is_wl0_vif = 0;
	
//we only use wl0.1	
	char unit[]="0";
	snprintf(vif,sizeof(vif),"wl%s_vifs",unit);
	memset(buf,0,sizeof(buf));

	/* This logic here decides if updates to virtual interface list on the
	   parent is required */

	snprintf(buf,sizeof(buf),"%s",nvram_safe_get(vif));

	snprintf(vif_name,sizeof(vif_name),"wl%s",unit_str);

	foreach(name, buf, next) {
		if (!strcmp(name,vif_name)){
			is_wl0_vif = 1;
			break;
		}
	}
	return is_wl0_vif;
}
