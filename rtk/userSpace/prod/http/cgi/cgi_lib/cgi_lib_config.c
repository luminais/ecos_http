#include "cgi_lib_config.h"
/*在新添加模块的时候需要保证添加的模块的枚举添加的位置与此处一致*/
CGI_LIB_MODULE_INFO cgi_lib_get_modules[]=
{	
	{MODULE_GET_WAN_TYPE,				cgi_lib_get_wan_type},	
	{MODULE_GET_ADSL_INFO,				cgi_lib_get_adsl_info},	
	{MODULE_GET_NET_INFO,				cgi_lib_get_net_info},
	{MODULE_GET_WAN_DETECTION,			cgi_lib_get_wan_detection},
	{MODULE_GET_WAN_SERVER,				cgi_lib_get_wan_server},
	{MODULE_GET_WAN_MTU,				cgi_lib_get_wan_mtu},
	{MODULE_GET_WAN_SPEED,				cgi_lib_get_wan_speed},
	{MODULE_GET_MAC_CLONE,				cgi_lib_get_mac_clone},
	{MODULE_GET_WAN_MAC,				cgi_lib_get_wan_mac},
	{MODULE_GET_SYSTEM_STATUS_INFO,		cgi_lib_get_system_status_info},
	{MODULE_GET_STATUS_WANMAC,      	cgi_lib_get_system_status_wanMac},
	{MODULE_GET_SOFTWARE_VERSION,   	cgi_lib_get_software_version},
	{MODULE_GET_WAN_CONNECT_TIME,   	cgi_lib_get_wan_connectTime},
#ifdef __CONFIG_TENDA_APP__		
	{MODULE_GET_UCLOUD_VERSION,     	cgi_lib_get_ucloud_version},
	{MODULE_GET_UCLOUD_UPGRADE,     	cgi_lib_get_ucloud_upgrade},
#endif	
	{MODULE_GET_WIFISCANRESAULT,    	cgi_lib_get_wifiScanresault},
	{MODULE_GET_NATIVE_HOST_MAC,		cgi_lib_get_native_host_mac},
	{MODULE_GET_LAN_INFO,				cgi_lib_get_lan_info},
	{MODULE_GET_DHCP_SERVER_INFO,		cgi_lib_get_dhcp_server_info},
	{MODULE_GET_TC_ONLINE_LIST,			cgi_lib_get_tc_online_list},
#ifdef __CONFIG_GUEST__
	{MODULE_GET_GUEST_ONLINE_LIST,		cgi_lib_get_guest_online_list},
#endif
	{MODULE_GET_LOCALHOST,				cgi_lib_get_localhost},
	{MODULE_GET_ONLINE_NUM,				cgi_lib_get_online_num},
	{MODULE_GET_BLACK_NUM,				cgi_lib_get_black_num},
	{MODULE_GET_STREAM_STATISTIC,		cgi_lib_get_stream_statistic},
	{MODULE_GET_WIFI_RATE,				cgi_lib_get_wifi_rate},
	{MODULE_GET_WIFI_NAME,				cgi_lib_get_wifi_name},
	{MODULE_GET_WIFI_EN,				cgi_lib_get_wifi_enable},
	{MODULE_GET_WIFI_BASIC,				cgi_lib_get_wifi_basic},
	{MODULE_GET_WIFI_POWER,				cgi_lib_get_wifi_power},
	{MODULE_GET_WIFI_SCHED,				cgi_lib_get_wifi_sched},
	{MODULE_GET_WIFI_RELAY_TYPE,		cgi_lib_get_wifi_relay_type},
	{MODULE_GET_WIFI_ADV_CFG,			cgi_lib_get_wifi_adv_cfg},
#ifdef __CONFIG_WPS_RTK__
	{MODULE_GET_WIFI_WPS,				cgi_lib_get_wifi_wps},
#endif
	{MODULE_GET_WIFIRELAY_INFO,              cgi_lib_get_wifiRelay_info},
	{MODULE_GET_WIFIRELAY_CONNECT_DUTATION,  cgi_lib_get_connect_duration},
	{MODULE_GET_ISWIFICLIENT,       		 cgi_lib_get_isWifiClient},


	{MODULE_GET_MACFILTER_MODE,			cgi_lib_get_macfilter_mode},
	{MODULE_GET_MACFILTER_LIST,			cgi_lib_get_macfilter_list},
	{MODULE_GET_PARENT_ACCESS_CTRL,		cgi_lib_get_parentAccessCtrl},	
	{MODULE_GET_PARENT_ONLINE_LIST,		cgi_lib_get_parent_online_list},
	{MODULE_GET_REMOTE_INFO,			cgi_lib_get_remote_info},
	{MODULE_GET_FIREWARE,				cgi_lib_get_fireware},
	{MODULE_GET_SYSTIME,				cgi_lib_get_systime},
	{MODULE_GET_LOGIN_PWD,				cgi_lib_get_login_pwd},
	{MODULE_GET_INTERNET_STATUS,		cgi_lib_get_internet_status},
	{MODULE_GET_NAT_STATICIP_LIST,		cgi_lib_get_nat_staticip},
    {MODULE_GET_NAT_PORT_FORWARD,		cgi_lib_get_nat_portForward},
    {MODULE_GET_NAT_DDNS,				cgi_lib_get_nat_ddns},
    {MODULE_GET_NAT_DMZ,				cgi_lib_get_nat_dmz},
    {MODULE_GET_NAT_UPNP,				cgi_lib_get_nat_upnp},
	{MODULE_GET_WLFI_CHANNEL_GRADE,     cgi_lib_get_wifi_channel_grade},
    {MODULE_GET_EXPIRED_CLIENT_NUM,     cgi_lib_get_expired_client_num},
    {MODULE_GET_EXPIRED_INFO,           cgi_lib_get_all_expired_info},
#ifdef __CONFIG_AUTO_CONN_CLIENT__
    {MODULE_GET_AUTO_SYNC_INFO,         cgi_lib_auto_sync_info_get},
#endif
#ifdef __CONFIG_LED__
	{MODULE_GET_LED_INFO,           cgi_lib_get_led},	//获取LED
	{MODULE_GET_LED_INFO_APP,           cgi_lib_get_led_app},	//获取LED
#endif
#ifdef __CONFIG_IPTV__
	{MODULE_GET_NAT_IPTV,			cgi_lib_get_nat_iptv},	//获取IPTV
#endif
#ifdef __CONFIG_GUEST__
	{MODULE_GET_WIFI_GUEST_INFO,        cgi_lib_get_wifi_guest_info},
#endif
#ifdef __CONFIG_WL_BEAMFORMING_EN__
	{MODULE_GET_BEAFORMING_ENABLE,		cgi_lib_get_beaforming_enable},
#endif
    {MODULE_GET_NAT_ELINK,               cgi_lib_get_nat_elink},
	{MODULE_GET_END,					NULL},
};

CGI_LIB_MODULE_INFO cgi_lib_set_modules[]=
{	
	{MODULE_SET_WAN_ACCESS,				cgi_lib_set_wan_access},
	{MODULE_SET_WAN_MTU,				cgi_lib_set_wan_mtu},
	{MODULE_SET_WAN_SPEED,				cgi_lib_set_wan_speed},
	{MODULE_SET_MAC_CLONE,				cgi_lib_set_wan_macClone},
	{MODULE_SET_WAN_SERVER,				cgi_lib_set_wan_server},
	{MODULE_SET_LAN_INFO,				cgi_lib_set_lan_info},
	{MODULE_SET_WIFI_BASIC,				cgi_lib_set_wifi_basic},
	{MODULE_SET_WIFI_POWER,				cgi_lib_set_wifi_power},
	{MODULE_SET_WIFI_SCHED,				cgi_lib_set_wifi_sched},
	{MODULE_SET_WIFI_ADV_CFG,			cgi_lib_set_wifi_adv_cfg},
#ifdef __CONFIG_WPS_RTK__
	{MODULE_SET_WIFI_WPS,				cgi_lib_set_wifi_wps},
#endif
	{MODULE_SET_WIFIRELAY,      		cgi_lib_set_wifiRelay},

	{MODULE_SET_WIZARD_SYSTIME, 		cgi_lib_set_wziard_systime},

	{MODULE_SET_QOS_TC,					cgi_lib_set_tc_qoslist},
	{MODULE_SET_MACFILTER,				cgi_lib_set_disposable_macfilter},
	{MODULE_SET_PARENT_ONLINELIST, 		cgi_lib_set_parent_onlineList},
	{MODULE_SET_PARENT_ACCESS_CTRL,  	cgi_lib_set_parent_access_ctrl},
	{MODULE_SET_REMOTE_INFO,			cgi_lib_set_remote_info},
	{MODULE_SET_FIREWARE,				cgi_lib_set_fireware},
	{MODULE_SET_OPERATE,				cgi_lib_set_operate},
	{MODULE_SET_SYSTIME,				cgi_lib_set_systime},
	{MODULE_SET_LOGIN_PWD,				cgi_lib_set_login_pwd},
 	{MODULE_SET_NAT_STATICIP,			cgi_lib_set_nat_staticip},
    {MODULE_SET_NAT_PORT_FORWARD,		cgi_lib_set_nat_portForward},
    {MODULE_SET_NAT_DDNS,				cgi_lib_set_nat_ddns},
    {MODULE_SET_NAT_DMZ,				cgi_lib_set_nat_dmz},
    {MODULE_SET_NAT_UPNP,				cgi_lib_set_nat_upnp},		
#ifdef __CONFIG_TENDA_APP__
    {MODULE_SET_NO_NOW_UPGRADE, 		cgi_lib_notNow_upgrade_set},
#endif 
    {MODULE_SET_WLFI_CHANNEL_GRADE,     	cgi_lib_set_wifi_channel_grade},
	{MODULE_SET_RUB_NET_ADD_BLACKLIST,  	cgi_lib_set_rub_net_add_blacklist},
	{MODULE_SET_RUB_NET_DELETE_BLACKLIST,	cgi_lib_set_rub_net_delete_blacklist},
	{MODULE_SET_RUB_NET_ADD_TRUSTLIST,  	cgi_lib_set_rub_net_add_to_trustlist},
	{MODULE_SET_RUB_NET_DELETE_TRUSTLIST,	cgi_lib_set_rub_net_delete_from_trustlist},
	{MODULE_SET_WIZARD_SUCCEED,				cgi_lib_set_wizard_succeed}, 
	{MODULE_SET_MACFILTER_MODE,				cgi_lib_set_macfilter_mode},
	{MODELE_SET_FLUSH_BLACKLIST,			cgi_lib_set_flush_blacklist},
#ifdef __CONFIG_GUEST__
	{MODULE_SET_WIFI_GUEST_INFO,        	cgi_lib_set_wifi_guest_info},
#endif
#ifdef __CONFIG_WL_BEAMFORMING_EN__
	{MODULE_SET_BEAFORMING_ENABLE,			cgi_lib_set_beaforming_enable},
#endif
#ifdef __CONFIG_LED__
	{MODULE_SET_LED_INFO,cgi_lib_set_led},		//获取LED
#endif
#ifdef __CONFIG_IPTV__
	{MODULE_SET_NAT_IPTV,cgi_lib_set_nat_iptv},		//设置IPTV
#endif
    {MODULE_SET_NAT_ELINK,               cgi_lib_set_nat_elink},     
	{MODULE_SET_END,					NULL},
};

