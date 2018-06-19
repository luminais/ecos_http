/*
 * Route cfg macro 
 *
 * Copyright (C) 2010, Tenda Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Tenda Corporation.
 *	
 * $Id: routercfg.h,v 1.0 2010/08/27   Exp $
 * $author: stanley 
 * $description: 
 *			  
 */

 struct upgrade_mem{
	struct upgrade_mem *next;
	int totlen;
	int inuse;
	char *data;
};
 
/*About Router version parameters*/
#define _SYS_NAME			"os_name"
#define _SYS_VERS			"os_version"
#define _SYS_DATE			"os_date"
#define _WLN_VERS			"wl_version"
#define _NVRAM_VERS		"nvram_version"
#define _SYS_LANGUAGE		"os_language"
//#define _SYS_SUBFLAG		"os_subflag"


/* Miscellaneous parameters */
#define _TIMER_INTERVAL		"timer_interval"
#define _NTP_SERVER0		"ntp_server"
#define _NTP_SERVER1		
#define _NTP_SERVER2		
#define _LOG_LEVEL			"log_level"
#define _UPNP_ENABLE		"upnp_enable"
	/*bei++*/
#define _SYS_TZONE 			"time_zone"
#define _SYS_NTPTYPE		"time_mode"/*0:关闭手动输入 1：启用手动输入*/

/*About lan0 interface parameters*/
#define _LAN0_IFNAME		"lan_ifname"
#define _LAN0_IFNAMES		"lan_ifnames"
#define _LAN0_HWNAMES		"lan_hwnames"
#define	_LAN0_HWADDR		"lan_hwaddr"
#define _LAN0_DHCP			"lan_dhcp"
#define _LAN0_IP				"lan_ipaddr"
#define _LAN0_NETMASK	 	"lan_netmask"
#define _LAN0_GATEWAY		"lan_gateway"
#define _LAN0_PROTO			"lan_proto"
#define _LAN0_WINS			"lan_wins"
#define _LAN0_DOMAIN		"lan_domain"
#define _LAN0_LEASE			"lan_lease"
#define _LAN0_STP			"lan_stp"
#define _LAN0_ROUTE			"lan_route"
#define	_LAN0_HWADDR1		"lan_hwaddr1"
#define	_LAN0_IP1		"lan_ipaddr1"



//roy +++2010/09/8
/*common*/
#define _WAN0_PRIMARY 		"wan0_primary"
#define _WAN0_IFNAME 		"wan0_ifname"
#define _WAN0_DOMAIN 		"wan0_domain"
//#define _WAN0_ROUTE 		"wan0_route"
#define _WAN0_WINS 		"wan0_wins"
#define _WAN0_DESC 			"wan0_desc"
#define _WAN0_HWADDR 		"wan0_hwaddr"
#define _WAN0_UNIT 			"wan0_unit"
#define _WAN0_PROTO 		"wan0_proto"
#define _WAN0_PROTO_INDEX 	"wan0_proto_index"
#define _WAN0_CONNECT 		"wan0_connect"
#define _WAN0_CHECK			"wan0_check"

/*static*/
#define _WAN0_IPADDR 		"wan0_ipaddr"
#define _WAN0_NETMASK 		"wan0_netmask"
#define _WAN0_GATEWAY 	"wan0_gateway"
#define _WAN0_DNS 			"wan0_dns"
#define _WAN0_DNS_FIX		"wan0_dns_fix"

/*dhcp*/
#define _WAN0_HAOTNAME 	"wan0_hostname"
#define _WAN0_MTU 			"wan0_mtu"
#define _DHCP_WAN0_MTU 			"dhcp_wan0_mtu"
#define _STATIC_WAN0_MTU 			"static_wan0_mtu"
#define _PPPOE_WAN0_MTU 			"pppoe_wan0_mtu"


/*pppoe*/
#define _WAN0_PPPOE_IFNAME 		"wan0_pppoe_ifname"
#define _WAN0_PPPOE_USERNAME 	"wan0_pppoe_username"
#define _WAN0_PPPOE_PASSWD 		"wan0_pppoe_passwd"
#define _WAN0_PPPOE_AC 			"wan0_pppoe_ac"
#define _WAN0_PPPOE_SERVICE 		"wan0_pppoe_service"
#define _WAN0_PPPOE_MTU 			"wan0_pppoe_mtu"
#define _WAN0_PPPOE_MRU 			"wan0_pppoe_mru"
#define _WAN0_PPPOE_DEMAND 		"wan0_pppoe_demand"
#define _WAN0_PPPOE_IDLETIME 		"wan0_pppoe_idletime"
#define _WAN0_PPPOE_KEEPLIVE 		"wan0_pppoe_keepalive"
#define _WAN0_PPPOE_ST				"wan0_pppoe_st"
#define _WAN0_PPPOE_ET				"wan0_pppoe_et"
#define _WAN0_PPPOE_XKJX_TIME		"wan0_pppoe_xkjx_time"
#define _WAN0_PPPOE_SHANXUN		"wan0_pppoe_shanxun"
#define _WAN0_PPPOE_XKJX_TIME		"wan0_pppoe_xkjx_time"

/*pppoe2*/
#define _WAN0_PPPOE2_USERNAME 		"wan0_pppoe_username"
#define _WAN0_PPPOE2_PASSWD 			"wan0_pppoe_passwd"
#define _WAN0_PPPOE2_AC 				"wan0_pppoe_ac"
#define _WAN0_PPPOE2_SERVICE 			"wan0_pppoe_service"
#define _WAN0_PPPOE2_MTU 				"wan0_pppoe_mtu"
#define _WAN0_PPPOE2_MRU 				"wan0_pppoe_mru"
#define _WAN0_PPPOE2_STATIC 			"wan0_pppoe_static"
#define _WAN0_PPPOE2_IPADDR 			"wan0_pppoe_ipaddr"
#define _WAN0_PPPOE2_NETMASK 			"wan0_pppoe_netmask"
#define _WAN0_PPPOE2_GATEWAY 		"wan0_pppoe_gateway"
#define _WAN0_PPPOE2_HAOTNAME 		"wan0_pppoe_hostname"
#define _WAN0_PPPOE2_MTU2 			"wan0_mtu"
/*pptp*/
#define _WAN0_PPTP_IFNAME 		"wan0_pptp_ifname"
#define _WAN0_PPTP_USERNAME 		"wan0_pptp_username"
#define _WAN0_PPTP_PASSWD 		"wan0_pptp_passwd"
#define _WAN0_PPTP_SERVER_NAME 	"wan0_pptp_server_name"
#define _WAN0_PPTP_STATIC 			"wan0_pptp_static"
#define _WAN0_PPTP_IPADDR 			"wan0_pptp_ipaddr"
#define _WAN0_PPTP_NETMASK 		"wan0_pptp_netmask"
#define _WAN0_PPTP_GATEWAY 		"wan0_pptp_gateway"
#define _WAN0_PPTP_DNS 			"wan0_pptp_dns"
#define _WAN0_PPTP_MTU 			"wan0_pptp_mtu"
#define _WAN0_PPTP_MRU 			"wan0_pptp_mru"
#define _WAN0_PPTP_DEMAND 		"wan0_pptp_demand"
#define _WAN0_PPTP_IDLETIME 		"wan0_pptp_idletime"
#define _WAN0_PPTP_KEEPLIVE 		"wan0_pptp_keepalive"
#define _WAN0_PPTP_MPPE_EN		"wan0_pptp_mppe_en"

/*l2tp*/
#define _WAN0_l2TP_IFNAME 		 	"wan0_l2tp_ifname"
#define _WAN0_l2TP_USERNAME 	 	"wan0_l2tp_username"
#define _WAN0_l2TP_PASSWD 		"wan0_l2tp_passwd"
#define _WAN0_l2TP_SERVER_NAM  	"wan0_l2tp_server_name"
#define _WAN0_l2TP_STATIC 		 	"wan0_l2tp_static"
#define _WAN0_l2TP_IPADDR 		 	"wan0_l2tp_ipaddr"
#define _WAN0_l2TP_NETMASK 		"wan0_l2tp_netmask"
#define _WAN0_l2TP_GATEWAY 		"wan0_l2tp_gateway"
#define _WAN0_l2TP_DNS 			"wan0_l2tp_dns"
#define _WAN0_l2TP_MTU 			"wan0_l2tp_mtu"
#define _WAN0_l2TP_MRU 			"wan0_l2tp_mru"
#define _WAN0_l2TP_DEMAND 		"wan0_l2tp_demand"
#define _WAN0_l2TP_IDLETIME 	 	"wan0_l2tp_idletime"
#define _WAN0_l2TP_KEEPLIVE 	 	"wan0_l2tp_keepalive"

//static route
#define _WAN0_ROUTE				"wan0_route"

/*DHCP server parameters*/
#define _LAN0_DHCPD_EN				"lan_proto"
#define _LAN0_DHCPD_START			"dhcp_start"
#define _LAN0_DHCPD_END			"dhcp_end"
#define _LAN0_DHCPD_LEASET		"lan_lease"

#define LAN0_DHCP_SATIC(i)			racat("dhcp_static_lease",i)
#define TC_LIST(i)     racat("tc_",i)
//+++roy

/*About wan1 interface parameter*/


/*About firewall interface parameter*/
//#define _FW_FILTER_MACLIST			"filter_maclist"
//#define _FW_FILTER_MACMODE		"filter_macmode"


//SafeNetAttack
#define _FW_HACKER_ATT			"hacker_att"

//SafeWanWebMan
#define _SYS_RM_EN				"rm_web_en"
#define _SYS_RM_PORT			"rm_web_port"
#define _SYS_RM_IP				"rm_web_ip"

#define _FW_PING_DIS_WAN		"ping_dis_wan"

/*****************************************/

/*******ADD BY Zliang ***/

//SafeIpFilter
#define _FW_FLT_CLN_EN			"filter_client_mode"
#define _FW_FLT_CLN_CUR_NU		"filter_client_cur_nu"
#define _FW_FILTER_CLIENT(i)	racat("filter_client",i)

//SafeUrlFilter
#define _FW_FLT_URL_EN			"filter_url_mode"
#define _FW_FLT_URL_CUR_NU		"filter_url_cur_nu"
#define _FW_FILTER_URL(i)		racat("filter_url",i)

//SafeMaclFilter
#define _FW_FLT_MAC_EN			"filter_mac_mode"
#define _FW_FLT_MAC_CUR_NU		"filter_mac_cur_nu"
#define _FW_FILTER_MAC(i)		racat("filter_mac", i)


//pxy w316r_vsl01 2013.1.18
#define _FW_FLT_HOSTNAME_EN				"filter_hostname_mode"
#define _FW_FLT_HOSTNAME_CUR_NU		"filter_hostname_cur_nu"
#define _FW_FILTER_HOSTNAME(i)			racat("filter_hostname", i)
//end add

//_FW_AUTOFW_PORT0-9
#define _FW_FORWARD_PORT(i)		racat("forward_port", i)
#define _FW_DMZ_IPADDR_EN		"dmz_ipaddr_en"	
#define _FW_DMZ_IPADDR			"dmz_ipaddr"
/********* ADD BY Zliang *****/
#define _FW_UPNP_EN             "upnp_enable"

/*Qos parmemters*/
#define _QOS_ORATES				"qos_orates"
#define _QOS_IRATES				"qos_irates"
#define _QOS_ENABLE				"qos_enable"
#define _QOS_METHOD				"qos_method"
#define _QOS_STICKY				"qos_sticky"
#define _QOS_ACK				"qos_ack"
#define _QOS_ICMP				"qos_icmp"
#define _QOS_RESET				"qos_reset"
#define _QOS_OBW				"qos_obw"
#define _QOS_IBW				"qos_ibw"
#define _QOS_ORULES				"qos_orules"
#define _QOS_BURST0				"qos_burst0"
#define _QOS_BURST1				"qos_burst1"
#define _QOS_DEFAULT			"qos_default"

/*restore nvram flag*/
#define _RESTORE_DEFAULTS		"restore_defaults"

//#ifdef __CONFIG_QUICK_SET__			
#define _RESTORE_QUICK_SET		"restore_quick_set" //add liuchengchi 2014-11-11
//#endif

/*Web server parameters*/
#define 	HTTP_USERNAME		"http_username"
#define		HTTP_PASSWD			"http_passwd"
#define     HTTP_DEFAULTPWD     "http_defaultpwd"
#define     HTTP_DEFAULTPWD1     "http_defaultpwd1"
#define		HTTP_PASSWD_TIP		"http_passwd_tip"
#define		HTTP_PASSWD_TIP1		"http_passwd_tip1"

#define 	HTTP_LANPORT

#define 	HTTP_WAN_ENABLE
#define 	HHTP_WANIP			
#define 	HTTP_WANPORT

//ddns
#define _DDNS_ENABLE		"ddns_enable"
#define _DDNS_ISP			"ddns_isp"
#define _DDNS_SET1			"ddns_set1"
#define _DDNS_HOST_NAME     "ddns_hostname1"
#define _DDNS_STATUS       	"ddns_status"

#if 0
/*Wireless paramters*/
			/*basic html*/
/*Whether only allow station to assoticate with 11N capabilities*/
#define WLN0_ONLY_N_MODE				"wl0_nregd"	
/*-1: Auto 0: off*/
#define WLN0_N_MODE					"wl0_nmode"
/*0: legacy B mode, 1: B/G mix mode, 2: G only mode.*/
#define WLN0_G_MODE					"wl0_gmode"			
#define WLN0_SSID0						"wl0_ssid"					
#define WLN0_COUNTRY					"wl_country_code"
#define WLN0_CHANNEL					"wl0_channel"
#define WLN0_HIDE_SSID0				"wl0_closed"
/*"long", "short"*/
#define WLN0_PLCPHDR					"wl0_plcphdr"		//add for 150M /135M
#define WLN0_HT_MODE					"wl0_mimo_preamble"	
/*0: 20 MHz in Both Bands; 1: 40 MHz in Both Bands; 2: 20MHz in 2.4G Band and 40MHz in 5G Band*/
#define WLN0_HT_BW						"wl0_nbw_cap"		
#define WLN0_OBSS						"wl0_obss_coex"
#define WLN0_HT_GI									
#define WLN0_HT_MCS					"wl0_nmcsidx"
#define WLN0_HT_RDG	
/*"none", "lower", "upper"*/
#define WLN0_HT_EXTCHA					"wl0_nctrlsb"
#define WLN0_HT_AMSDU					"wl0_amsdu"
#define WLN0_WIRELESS_ENABLE			"wl0_radio"
#define WLN0_MODE						"wl0_mode"
#define WLN0_APORWDS					"wl0_aporwds"

/*advanced html*/
#define WLN0_BASICRATE					"wl0_rateset"
#define WLN0_G_PROTECT					"wl0_gmode_protection"
#define WLN0_N_PROTEC					"wl0_nmode_protection"
#define WLN0_BEACON_PERIOD			"wl0_bcn"
#define WLN0_FRAG_THRESHOLD			"wl0_frag"
#define WLN0_RTS_THRESHOLD			"wl0_rts"
#define WLN0_WMM_CAPABLE				"wl0_wme"
#define WLN0_APSD_CAPABLE				"wl0_wme_apsd"
#define WLN0_Tx_POWER						""

/*security html*/
#define OPEN			"0"
#define SHARED 		"1"
#define DISABLE		"disabled"
#define ENABLE		"enabled"

#define WLN0_WPS_OOB					"lan_wps_oob"
#define WLN0_PRE_AUTH					""
#define WLN0_AUTH_MODE				"wl0_auth"		/*0:open; 1:shared*/
/*WPA Encryption:"tkip","aes","tkip+aes"*/
#define WLN0_ENCRYP_TYPE				"wl0_crypto"
/*"wpa" "psk" "wpa2" "psk2" "brcm_psk"*/
#define WLN0_SECURITY_TYPE				"wl0_akm"
#define WLN0_WEP						"wl0_wep"
#define WLN0_KEY_ID						"wl0_key"
#define WLN0_KEY1_TYPE					""
#define WLN0_KEY2_TYPE					""
#define WLN0_KEY3_TYPE					""
#define WLN0_KEY4_TYPE					""

#define WLN0_KEY1_STR1					"wl0_key1"
#define WLN0_KEY2_STR1					"wl0_key2"
#define WLN0_KEY3_STR1					"wl0_key3"
#define WLN0_KEY4_STR1					"wl0_key4"

#define WLN0_WPA_PSK1					"wl0_wpa_psk"
#define WLN0_REKEY_METHOD				"wl0_wpa_gtk_rekey"
//#define WLN0_REKEY_INTERVAL			"wl0_net_reauth"
#define WLN0_REKEY_INTERVAL			"timer_interval"

#define WLN0_WDS_LIST					"wl0_wds"
#define WLN0_WDS_LAZY					"wl0_lazywds"
#define WLN0_WDS_TIMEOUT				"wl0_wds_timeout"
#define WLN0_WDS_KEYID
#define WLN0_WDS_KEY1STR
#define WLN0_WDS_KEY2STR
#define WLN0_WDS_KEY3STR
#define WLN0_WDS_KEY4STR
#define WLN0_WDS_WPAPSK
#define WLN0_WDS_AUTH
#define WLN0_WDS_ENCRYP
#define WLN0_WDS_SECURITY

#define WLN0_MAC_MODE					"wl0_macmode"
#define WLN0_MAC_LIST					"wl0_maclist"

#define WLN0_WPS_ENABLE				"wl0_wps_mode"
#define WLN0_WPS_NAME					"wps_device_name"
#define WLN0_WPS_PIN					"wl0_wps_pin"
#define WLN0_WPS_METHOD				"wl0_wps_method"
#define WLN0_WPS_SSID					"wps_randomssid"
#define WLN0_OLD_SSID0					"wl0_old_ssid"

#else
/*Wireless paramters*/
			/*basic html*/
#define WLN0_UNIT						"wl_unit"
#define WLN0_AP_ISOLATE					"wl_ap_isolate"
/*Whether only allow station to assoticate with 11N capabilities*/
#define WLN0_ONLY_N_MODE				"wl_nregd"	
/*-1: Auto 0: off*/
#define WLN0_N_MODE						"wl_nmode"
/*0: legacy B mode, 1: B/G mix mode, 2: G only mode.*/
#define WLN0_G_MODE						"wl_gmode"			
#define WLN0_SSID0						"wl_ssid"					
#define WLN0_COUNTRY					"wl_country_code"
#define WLN0_CHANNEL					"wl_channel"
#define WLN0_CHANNEL1					"wl0_channel"
#define WLN0_HIDE_SSID0				"wl_closed"
#define WLN0_HIDE_SSID					"wl0_closed"
#define WLN0_WDS_MAC_LIST				"wl_prev_mac"//存放WDS上级路由的MAC

/*"long", "short"*/
#define WLN0_PLCPHDR					"wl_plcphdr"		//add for 150M /135M
#define WLN0_HT_MODE					"wl_mimo_preamble"	
/*0: 20 MHz in Both Bands; 1: 40 MHz in Both Bands; 2: 20MHz in 2.4G Band and 40MHz in 5G Band*/
#define WLN0_HT_BW						"wl_nbw_cap"		
#define WLN0_HT_BW1						"wl0_nbw_cap"	
#define WLN0_OBSS						"wl_obss_coex"
#define WLN0_OBSS1						"wl0_obss_coex"
#define WLN0_HT_BW_FAKE_AUTO			"ht_bw_fake_auto"
#define WLN0_HT_GI									
#define WLN0_HT_MCS						"wl_nmcsidx"
#define WLN0_HT_RDG	
/*"none", "lower", "upper"*/
#define WLN0_HT_EXTCHA					"wl_nctrlsb"
#define WLN0_HT_EXTCHA1					"wl0_nctrlsb"
#define WLN0_HT_AMSDU					"wl_amsdu"
#define WLN0_WIRELESS_ENABLE			"wl_radio"
#define WLN0_MODE						"wl_mode"
#define WLN0_APORWDS					"wl_aporwds"

/*advanced html*/
#define WLN0_BASICRATE					"wl_rateset"
#define WLN0_G_PROTECT					"wl_gmode_protection"
#define WLN0_N_PROTEC					"wl_nmode_protection"
#define WLN0_BEACON_PERIOD				"wl_bcn"
#define WLN0_FRAG_THRESHOLD				"wl_frag"
#define WLN0_RTS_THRESHOLD				"wl_rts"
#define WLN0_WMM_CAPABLE				"wl_wme"
#define WLN0_APSD_CAPABLE				"wl_wme_apsd"
#define WLN0_Tx_POWER						""

#define WLN0_POWER						"wl_ctv_power"
#define WLN0_PWR_PERCENT				"wl_ctv_pwr_percent"

#define WLN0_CTL_ENABLE					"wl_wifictl_enable"
#define WLN0_CTL_DAY_ROUND				"wl_wifictl_day_round"			
#define WLN0_CTL_TIME_INTERVAL			"wl_wifictl_time_interval"

#define WLN0_CTL_ALILINK_SWITCH_SCHEDULER	"wl_alilink_switch_scheduler"
#define ALILINK_WLAN_SCHE_STATE			"alilink_wlan_state"
#define ALILINK_WLAN_SCHE_LIST_NUM		"alilink_wlan_sche_listnum"
#define ALILINK_WLAN_SCHE_ON_LIST		"alilink_wlan_ontime_list%d"
#define ALILINK_WLAN_SCHE_OFF_LIST		"alilink_wlan_offtime_list%d"

#define ALILINK_WLAN_TPSK_LIST_NUM		"alilink_wlan_tpsklist_num"
#define ALILINK_WLAN_TPSK_LIST				"alilink_wlan_tpsklist"
#define ALILINK_PROBED_SWITCH				"alilink_probed_switch"
#define ALILINK_ATTACK_SWITCH				"alilink_attack_switch"
#define ALILINK_SPEEDTEST_LIST(i)			racat("alilink_speedtest_list", i)
#define ALILINKGW_ENABLE					"alilinkgw_enable"
#define ALILINKGW_SEC_ENABLE				"alilinkgw_sec_enable"

/*security html*/
#define OPEN		"0"
#define SHARED 		"1"
#define DISABLE		"disabled"
#define ENABLE		"enabled"

#define WLN0_WPS_OOB					"lan_wps_oob"
#define WLN0_PRE_AUTH					""
#define WLN0_AUTH_MODE					"wl_auth"		/*0:open; 1:shared*/
/*WPA Encryption:"tkip","aes","tkip+aes"*/
#define WLN0_ENCRYP_TYPE				"wl_crypto"
/*"wpa" "psk" "wpa2" "psk2" "brcm_psk"*/
#define WLN0_SECURITY_TYPE				"wl_akm"
#define WLN0_WEP						"wl_wep"
#define WLN0_KEY_ID						"wl_key"
#define WLN0_KEY1_TYPE					""
#define WLN0_KEY2_TYPE					""
#define WLN0_KEY3_TYPE					""
#define WLN0_KEY4_TYPE					""

#define WLN0_KEY1_STR1					"wl_key1"
#define WLN0_KEY2_STR1					"wl_key2"
#define WLN0_KEY3_STR1					"wl_key3"
#define WLN0_KEY4_STR1					"wl_key4"

#define WLN0_WPA_PSK1					"wl_wpa_psk"
#define WLN0_REKEY_METHOD				"wl_wpa_gtk_rekey"
//#define WLN0_REKEY_INTERVAL			"wl0_net_reauth"
#define WLN0_REKEY_INTERVAL				"timer_interval"

#define WLN0_WDS_LIST					"wl_wds"
#define WLN0_WDS_LAZY					"wl_lazywds"
#define WLN0_WDS_TIMEOUT				"wl_wds_timeout"
#define WLN0_WDS_KEYID
#define WLN0_WDS_KEY1STR
#define WLN0_WDS_KEY2STR
#define WLN0_WDS_KEY3STR
#define WLN0_WDS_KEY4STR
#define WLN0_WDS_WPAPSK
#define WLN0_WDS_AUTH
#define WLN0_WDS_ENCRYP
#define WLN0_WDS_SECURITY

#define WLN0_MAC_MODE					"wl_macmode"
#define WLN0_MAC_LIST					"wl_maclist"

#define WLN0_WPS_ENABLE					"wl_wps_mode"
#define WLN0_WPS_NAME					"wps_device_name"
#define WLN0_WPS_PIN					"wl_wps_pin"
#define WLN0_WPS_METHOD					"wl_wps_method"
#define WPS_DEVICE_PIN					"wps_device_pin"
#endif

#define TC_ENABLE						"tc_enable"
#define TC_WAN_UPRATE					"wan_up_rate"
#define TC_WAN_DOWNRATE					"wan_down_rate"
#define TC_ISP_UPRATE					"tc_isp_uprate"
#define TC_ISP_DOWNRATE					"tc_isp_downrate"
#define TC_STREAM_STAT_EN				"tc_stream_stat_en"
#define TC_RULE_(i)						racat("tc_", i)
/*function define*/
char *racat(char *s, int i);
int 	get_wan0_dns(int index,char *dnsBuf);
int 	get_pptp_dns(int index, char *dnsBuf);
int 	get_l2tp_dns(int index, char *dnsBuf);
int 	get_wan_type(void);
int 	set_wan_str(int type);
int 	get_wan_connstatus(void);
int   get_wan_onln_connstatus(void);//gong add 标记能否上网
int 	get_wan_linkstatus(void);
int 	get_dhcpd_en();
int 	get_filter_mode(char *);
void set_dhcpd_en(int enable);
int 	CGI_same_net_with_lan(unsigned int wan_ip,unsigned int wan_mask);
int 	parse_webstr2portforward(char *oldstr, char *newstr);
int 	parse_portforward2webstr(char *oldstr,char *newstr);
void set_wan_mac(const char *ipaddr);
int 	CGI_do_wan_connect_tenda(int action);
void strtoupper(char *buf);

