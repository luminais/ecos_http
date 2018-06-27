
/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : sys_config.h
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2017年10月23日
  最近修改   :
  功能描述   : 

  功能描述   : 所有的nvram参数都需要添加到该文件中，保证后续再修改参数的时候
               可以统一修改

  修改历史   :
  1.日    期   : 2017年10月23日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#ifndef TENDA_SYS_CONFIG_H
#define TENDA_SYS_CONFIG_H

/*************2.4G基本参数*****************/
#define WLAN24G_SSID                        		"wl0_ssid"
#define WLAN24G_MAC                        			"wl0_hwaddr"
#define WLAN24G_ENABLE                      		"wl0_radio"
#define WLAN24G_ENCODE		               	        "wl0_encode"
#define WLAN24G_HIDE_SSID                   		"wl0_closed"
#define WLAN24G_AKM                         		"wl0_akm"
#define WLAN24G_WEP                         		"wl0_wep"
#define WLAN24G_PASSWD                      		"wl0_wpa_psk"
#define WLAN24G_CRYPTO                      		"wl0_crypto"
#define WLAN24G_NET_TYPE                    		"wl0_nettype"
#define WLAN24G_CHANNEL                     		"wl0_channel"
#define WLAN24G_BANDSIDE                    		"wl0_bandside"
#define WLAN24G_BANDWIDTH                   		"wl0_bandwidth"
#define WLAN24G_CURRENT_POWER           	        "wl0_ctv_power"
#define WLAN24G_SUPPOR_POWER_LEVER 		        	"wl0_country_pwr_power"
#define WLAN24G_OFFSET_POWER             			"wl0_offset_power"
#define WLAN24G_WORK_MODE                  			"wl0_mode"               //sta,ap
#define WLAN24G_IFNAME                      		"wl0_ifname"

/*************2.4G其他设置参数*******************/
    
/*************2.4G次接口基本参数*****************/
#define WLAN24G_REPEATER_SSID               	"wl0.1_ssid"
#define WLAN24G_REPEATER_MAC               		"wl0.1_hwaddr"
#define WLAN24G_REPEATER_ENCODE               	"wl0.1_encode"
#define WLAN24G_REPEATER_ENABLE           		"wl0.1_radio"
#define WLAN24G_REPEATER_HIDE_SSID      		"wl0.1_closed"
#define WLAN24G_REPEATER_AKM                	"wl0.1_akm"
#define WLAN24G_REPEATER_WEP                	"wl0.1_wep"
#define WLAN24G_REPEATER_PASSWD        	        "wl0.1_wpa_psk"
#define WLAN24G_REPEATER_CRYPTO             	"wl0.1_crypto"
#define WLAN24G_REPEATER_WORK_MODE     	        "wl0.1_mode"
#define WLAN24G_REPEATER_IFNAME             	"wl0.1_ifname"  
/*************2.4G 访客网络基本参数*****************/
#define WLAN24G_GUEST_SSID                  	"wl0.2_ssid"
#define WLAN24G_GUEST_MAC						"wl0.2_hwaddr"
#define WLAN24G_GUEST_NET_TYPE 					"wl0.2_nettype"
#define WLAN24G_GUEST_ENABLE                	"wl0.2_radio"
#define WLAN24G_GUEST_HIDE_SSID             	"wl0.2_closed"
#define WLAN24G_GUEST_AKM                   	"wl0.2_akm"
#define WLAN24G_GUEST_WEP                   	"wl0.2_wep"
#define WLAN24G_GUEST_PASSWD                	"wl0.2_wpa_psk"
#define WLAN24G_GUEST_CRYPTO                	"wl0.2_crypto"
#define WLAN24G_GUEST_IFNAME                	"wl0.2_ifname"

/*************2.4G 虚拟网络基本参数*****************/
#define WLAN24G_VIRTUAL_SSID                  	"wl0.3_ssid"
#define WLAN24G_VIRTUAL_MAC						"wl0.3_hwaddr"
#define WLAN24G_VIRTUAL_NET_TYPE 				"wl0.3_nettype"
#define WLAN24G_VIRTUAL_ENABLE                	"wl0.3_radio"
#define WLAN24G_VIRTUAL_HIDE_SSID             	"wl0.3_closed"
#define WLAN24G_VIRTUAL_AKM                   	"wl0.3_akm"
#define WLAN24G_VIRTUAL_WEP                   	"wl0.3_wep"
#define WLAN24G_VIRTUAL_PASSWD                	"wl0.3_wpa_psk"
#define WLAN24G_VIRTUAL_CRYPTO                	"wl0.3_crypto"
#define WLAN24G_VIRTUAL_IFNAME                	"wl0.3_ifname"
    
/*****************5G基本参数*****************/
#define WLAN5G_SSID                         		"wl1_ssid"
#define WLAN5G_MAC                        			"wl1_hwaddr"
#define WLAN5G_ENCODE		               			"wl1_encode"
#define WLAN5G_ENABLE                       		"wl1_radio"
#define WLAN5G_HIDE_SSID                    		"wl1_closed"
#define WLAN5G_AKM                          		"wl1_akm"
#define WLAN5G_WEP                          		"wl1_wep"
#define WLAN5G_PASSWD                       		"wl1_wpa_psk"
#define WLAN5G_CRYPTO                       		"wl1_crypto"
#define WLAN5G_NET_TYPE                     		"wl1_nettype"
#define WLAN5G_CHANNEL                      		"wl1_channel"
#define WLAN5G_BANDSIDE                     		"wl1_bandside"
#define WLAN5G_BANDWIDTH                    		"wl1_bandwidth"
#define WLAN5G_CURRENT_POWER                	    "wl1_ctv_power"
#define WLAN5G_SUPPOR_POWER_LEVER   		        "wl1_country_pwr_power"
#define WLAN5G_OFFSET_POWER                 	    "wl1_offset_power"
#define WLAN5G_WORK_MODE                    		"wl1_mode"                         //sta,ap
#define WLAN5G_IFNAME                       		"wl1_ifname"

/*************5G次接口基本参数*****************/
#define WLAN5G_REPEATER_SSID                	"wl1.1_ssid"
#define WLAN5G_REPEATER_MAC               		"wl1.1_hwaddr"
#define WLAN5G_REPEATER_ENCODE		      		"wl1.1_encode"
#define WLAN5G_REPEATER_ENABLE              	"wl1.1_radio"
#define WLAN5G_REPEATER_HIDE_SSID           	"wl1.1_closed"
#define WLAN5G_REPEATER_AKM                 	"wl1.1_akm"
#define WLAN5G_REPEATER_WEP                 	"wl1.1_wep"
#define WLAN5G_REPEATER_PASSWD              	"wl1.1_wpa_psk"
#define WLAN5G_REPEATER_CRYPTO              	"wl1.1_crypto"
#define WLAN5G_REPEATER_WORK_MODE       		"wl1.1_mode"
#define WLAN5G_REPEATER_IFNAME              	"wl1.1_ifname"  
/*************5G 访客网络基本参数*****************/
#define WLAN5G_GUEST_SSID                   	"wl1.2_ssid"
#define WLAN5G_GUEST_MAC						"wl1.2_hwaddr"
#define WLAN5G_GUEST_NET_TYPE 					"wl1.2_nettype"
#define WLAN5G_GUEST_ENABLE                 	"wl1.2_radio"
#define WLAN5G_GUEST_HIDE_SSID              	"wl1.2_closed"
#define WLAN5G_GUEST_AKM                    	"wl1.2_akm"
#define WLAN5G_GUEST_WEP                    	"wl1.2_wep"
#define WLAN5G_GUEST_PASSWD                 	"wl1.2_wpa_psk"
#define WLAN5G_GUEST_CRYPTO                 	"wl1.2_crypto"
#define WLAN5G_GUEST_IFNAME                 	"wl1.2_ifname"

/*************5G 虚拟网络基本参数*****************/
#define WLAN5G_VIRTUAL_SSID                  	"wl1.3_ssid"
#define WLAN5G_VIRTUAL_MAC						"wl1.3_hwaddr"
#define WLAN5G_VIRTUAL_NET_TYPE 				"wl1.3_nettype"
#define WLAN5G_VIRTUAL_ENABLE                	"wl1.3_radio"
#define WLAN5G_VIRTUAL_HIDE_SSID             	"wl1.3_closed"
#define WLAN5G_VIRTUAL_AKM                   	"wl1.3_akm"
#define WLAN5G_VIRTUAL_WEP                   	"wl1.3_wep"
#define WLAN5G_VIRTUAL_PASSWD                	"wl1.3_wpa_psk"
#define WLAN5G_VIRTUAL_CRYPTO                	"wl1.3_crypto"
#define WLAN5G_VIRTUAL_IFNAME                	"wl1.3_ifname"   

/********************WLAN 公共参数******************/
/*------------------------------------------------------*/
#define WLAN_PUBLIC_GUEST_EFFECTIVE_TIME        "wl_guest_effective_time"
#define WLAN_PUBLIC_GUEST_DOWNSPEED_LIMIT       "wl_guest_down_speed_limit"	
#define WLAN_PUBLIC_BEAMFORMING_EN               "rltk_wlan_tx_beamforming"
#define WLAN_PUBLIC_AUTOCONN_EXT_RSSI   		"auto_conn_extend_rssi"	
#define WLAN_PUBLIC_DIG_SEL                 	"td_dig_sel"	
#define WLAN_PUBLIC_AUTO_ANTI               	"td_auto_antiInterferance"	
#define WLAN_PUBLIC_ADAPTIVITY              	"adaptivity_enable"
#define WLAN_PUBLIC_ADAPTIVITY_MODE     		"td_adaptivity_mode"
#define WLAN_PUBLIC_WISP_ID                 	"rltk_wisp_wlan_id"
#define WLAN_PUBLIC_ACK_TIMEOUT             	"rltk_wlan_ack_timeout"
#define WLAN_PUBLIC_AGGREGATION             	"rltk_wlan_aggregation"
#define WLAN_PUBLIC_BLOCK_RELAY             	"rltk_wlan_block_relay"
#define WLAN_PUBLIC_CONTROL_SIDEBAND    		"rltk_wlan_control_sideband"
#define WLAN_PUBLIC_IAPP_DISABLE            	"rltk_wlan_iapp_disable"
#define WLAN_PUBLIC_LDPC_ENABLE             	"rltk_wlan_ldpc_enable"
#define WLAN_PUBLIC_MACCLONE_ENABLE    			"rltk_wlan_macclone_enable"
#define WLAN_PUBLIC_NETWORK_TYPE            	"rltk_wlan_network_type"
#define WLAN_PUBLIC_PREAMBLE                	"rltk_wlan_preamble"
#define WLAN_PUBLIC_PROTECTION_DISABLE 			"rltk_wlan_protection_disable"
#define WLAN_PUBLIC_SHORT_GI                	"rltk_wlan_short_gi"
#define WLAN_PUBLIC_STA_NUM                 	"rltk_wlan_sta_num"
#define WLAN_PUBLIC_STBC_ENABLE             	"rltk_wlan_stbc_enable"
#define WLAN_PUBLIC_TX_BEAMFORMING       		"rltk_wlan_tx_beamforming"
#define WLAN_PUBLIC_WMM_ENABLE              	"rltk_wlan_wmm_enable"
#define WLAN_PUBLIC_REAUTH_TIME             	"rltk_wlan_wpa_group_rekey_time"
#define WLAN_PUBLIC_PWR_OFFSET_PERCENT          "country_offset_power"
/*-------------------wifi定时相关参数-----------------*/
//elink type wifi timer
#define ELINK_WLAN_PUBLIC_SCHEDULE_ENABLE(i)   racat("elink_wifictl_enable_", i)
#define ELINK_WLAN_PUBLIC_SCHEDULE_DAY(i)   	    racat("elink_wifictl_day_", i)
#define ELINK_WLAN_PUBLIC_SCHEDULE_TIME(i)   racat("elink_wifictl_time_", i)
#define ELINK_WLAN_PUBLIC_SCHEDULE_NUM       "elink_wifictl_num"

#define WLAN_PUBLIC_SCHEDULE_ENABLE        		"wl_wifictl_enable"
#define WLAN_PUBLIC_SCHEDULE_DAY            	"wl_wifictl_day_round"
#define WLAN_PUBLIC_SCHEDULE_TIME           	"wl_wifictl_time_interval"
#define WLAN_PUBLIC_SCHEDULE_WEEK           	"wl_time_week"
#define WLAN_PUBLIC_SCHEDULE_LIST_NUM			"wl_wifictl_schedule_listnum"
#define WLAN_PUBLIC_SCHEDULE_ONTIME_LIST		"wl_wifictl_schedule_ontime_list%d"
#define WLAN_PUBLIC_SCHEDULE_OFFTIME_LIST		"wl_wifictl_schedule_offtime_list%d"

#define WLAN_PUBLIC_COUNTRY_CODE            	"country_code"

/*-------------------无线双频合一---------------------*/
#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
#define WLAN_PUBLIC_DOUBLEBANDUN_ENBALE     "wl_doubleBandUn_enable"
#endif

/**************wps相关的参数*****************/
#define WPS_APLOCKDOWN_CAP                  		"wps_aplockdown_cap"
#define WPS_CONFIG_METHOD                   		"wps_config_method"
#define WPS_DEVICE_NAME                     		"wps_device_name"
#define WPS_DEVIDE_PIN                      		"wps_device_pin"
#define WPS_MFSTRING                        		"wps_mfstring"
#define WPS_MODE_ENABLE                            	"wps_mode"
#define WPS_MODELNAME                       		"wps_modelname"
#define WPS_MODELNUM                        		"wps_modelnum"
#define WPS_PROC_STATUS                     		"wps_proc_status"
#define WPS_RANDOM_SSID_PRE                 		"wps_random_ssid_prefix"
#define WPS_RESTART                         		"wps_restart"
#define WPS_STA_PIN                         		"wps_sta_pin"
#define WPS_TIMEOUT_ENABLE                  		"wps_timeout_enable"
#define WPS_VERSION2                        		"wps_version2"
#define WPS_WER_MODE                        		"wps_wer_mode"
#define WPS_XTALFREQ                       		 	"xtalfreq"
#define WPS_OS_NAME                         		"os_name"
#define WPS_OOB										"lan_wps_oob"
#define WPS_AP_PIN									"wl_wps_pin"
#define WPS_METHOD									"wl_wps_method"
#define WPS_CONF_STAT               "wps_configure_state"
/******************app相关的参数*****************/
#define UCLOUD_MANAGE_EN                    		"uc_manage_en"
#define UCLOUD_NEED_CLEAR_ACC               		"ucloud_need_clear_acc"

/*****************其他配置参数*******************/
#define SYSCONFIG_WORKMODE							"sys_workmode"        //route,client+ap,wisp,bridge
#define SYSCONFIG_VLAN2PORTS                		"vlan2ports"
#define SYSCONFIG_RESTORE                   		"restore_defaults"
#define SYSCONFIG_QUICK_SET_ENABLE          		"restore_quick_set"
#define SYSCONFIG_SDRAM_CONFIG              		"sdram_config"
#define SYSCONFIG_SDRAM_INIT                		"sdram_init"
#define SYSCONFIG_SDRAM_REFRESH             		"sdram_refresh"
#define SYSCONFIG_STATS_SERVER              		"stats_server"
#define SYSCONFIG_WATCHDOG                  		"watchdog"
/*------------------WAN口参数配置---------------*/
#define WAN_PPPOE_MTU                       		"pppoe_wan0_mtu"
#define WAN_DHCP_MTU                        		"dhcp_wan0_mtu"
#define WAN_STATIC_MTU                      		"static_wan0_mtu"
#define WAN_NETWORK_STATUS                  		"err_check"
#define WAN_DOMAIN                          		"wan_domain"
#define WAN_WHOLE_IFNAME                    		"wan_ifnames"
#define WAN_SPEED                           		"wan_speed"
#define WAN0_MAC_ADDR								"wan0_hwaddr"
#define WAN0_CHECK                          		"wan0_check"
#define WAN0_CONNECT                        		"wan0_connect"
#define WAN0_DESC                           		"wan0_desc"
#define WAN0_DNS                            		"wan0_dns"
#define WAN0_DNS_FIX                        		"wan0_dns_fix"
#define WAN0_DOMAIN                         		"wan0_domain"
#define WAN0_GATEWAY                        		"wan0_gateway"
#define WAN0_HOSTNAMW                       		"wan0_hostname"
#define WAN0_IFNAME                         		"wan0_ifname"
#define WAN0_IFNAMES                        		"wan0_ifnames"
#define WAN0_IPADDR                         		"wan0_ipaddr"
#define WAN0_MACCLONE_MODE                  		"wan0_macclone_mode"
#define WAN0_MTU                            		"wan0_mtu"
#define WAN0_NETMASK                        		"wan0_netmask"

#define WAN0_PPPOE_XKJX_TIME				"wan0_pppoe_xkjx_time"
#define WAN0_PPPOE_SHANXUN				"wan0_pppoe_shanxun"
#define WAN0_PPPOE_XKJX_TIME				"wan0_pppoe_xkjx_time"
#define WAN0_PPPOE_AC                       		"wan0_pppoe_ac"
#define WAN0_PPPOE_DEMAND                   		"wan0_pppoe_demand"
#define WAN0_PPPOE_ET                       		"wan0_pppoe_et"
#define WAN0_PPPOE_IDLETIME                 		"wan0_pppoe_idletime"
#define WAN0_PPPOE_IFNAME                   		"wan0_pppoe_ifname"
#define WAN0_PPPOE_KEEPALIVE                		"wan0_pppoe_keepalive"
#define WAN0_PPPOE_MRU                      		"wan0_pppoe_mru"
#define WAN0_PPPOE_MTU                      		"wan0_pppoe_mtu"
#define WAN0_PPPOE_PASSWD                   		"wan0_pppoe_passwd"
#define WAN0_PPPOE_PRIO_XKLS                		"wan0_pppoe_prio_xkjs"
#define WAN0_PPPOE_SERVICE                  		"wan0_pppoe_service"
#define WAN0_PPPOE_ST                       		"wan0_pppoe_st"
#define WAN0_PPPOE_USERNAME                 		"wan0_pppoe_username"
#define WAN0_PPPOE_TMP_PASSWD               		"wan0_tmp_pppoe_passwd"
#define WAN0_PPPOE_TMP_USERNAME             		"wan0_tmp_pppoe_username"
#define WAN0_L2TP_DEMAND                    		"wan0_l2tp_demand"
#define WAN0_L2TP_DNS                       		"wan0_l2tp_dns"
#define WAN0_L2TP_GATEWAY                   		"wan0_l2tp_gateway"
#define WAN0_L2TP_IDLETIME                  		"wan0_l2tp_idletime"
#define WAN0_L2TP_IFNAME                    		"wan0_l2tp_ifname"
#define WAN0_L2TP_IPADDR                    		"wan0_l2tp_ipaddr"
#define WAN0_L2TP_KEEPLIVE                  		"wan0_l2tp_keepalive"
#define WAN0_L2TP_MRU                       		"wan0_l2tp_mru"
#define WAN0_L2TP_MTU                       		"wan0_l2tp_mtu"
#define WAN0_L2TP_NETMASK                   		"wan0_l2tp_netmask"
#define WAN0_L2TP_PASSWD                    		"wan0_l2tp_passwd"
#define WAN0_L2TP_SERVER_NAME               		"wan0_l2tp_server_name"
#define WAN0_L2TP_STATIC                    		"wan0_l2tp_static"
#define WAN0_L2TP_USERNAME                  		"wan0_l2tp_username"
#define WAN0_PPTP_DEMAND                    		"wan0_pptp_demand"
#define WAN0_PPTP_DNS                       		"wan0_pptp_dns"
#define WAN0_PPTP_GATEWAY                   		"wan0_pptp_gateway"
#define WAN0_PPTP_IDLETIME                  		"wan0_pptp_idletime"
#define WAN0_PPTP_IFNAME                    		"wan0_pptp_ifname"
#define WAN0_PPTP_IPADDR                    		"wan0_pptp_ipaddr"
#define WAN0_PPTP_KEEPLIVE                  		"wan0_pptp_keepalive"
#define WAN0_PPTP_MPPE_EN                   		"wan0_pptp_mppe_en"
#define WAN0_PPTP_MRU                       		"wan0_pptp_mru"
#define WAN0_PPTP_MTU                       		"wan0_pptp_mtu"
#define WAN0_PPTP_NETMASK                   		"wan0_pptp_netmask"
#define WAN0_PPTP_PASSWD                    		"wan0_pptp_passwd"
#define WAN0_PPTP_SERVER_NAME               		"wan0_pptp_server_name"
#define WAN0_PPTP_STATIC                    		"wan0_pptp_static"
#define WAN0_PPTP_USERNAME                  		"wan0_pptp_username"
#define WAN0_PRIMARY                        		"wan0_primary"
#define WAN0_PROTO                          			"wan0_proto"
#define WAN0_INDEX                          			"wan0_proto_index"
#define WAN0_ROUTE                          			"wan0_route"
#define WAN0_UNIT                           			"wan0_unit"
#define WAN0_WINS                           			"wan0_wins"
#define WAN1_PPPOE_IFNAME                   		"wan1_pppoe_ifname"
/*------------------LAN口参数-------------------*/
#define LAN_DHCP_DOMAIN                     		"dhcp_domain"
#define LAN_DHCP_POOL_END                   		"dhcp_end"
#define LAN_DHCP_POOL_START                 		"dhcp_start"
#define LAN_HTTP_FAC_PW                     		"http_defaultpwd"
#define LAN_HTTP_HAS_LOGIN_PASSWD           		"http_defaultpwd1"
#define LAN_HTTP_LANPORT                    		"http_lanport"
#define LAN_HTTP_LOGIN_PASSWD               		"http_passwd"
#define LAN_HTTP_LOGIN_USERNAME             		"http_username"
#define LAN_HTTP_WANPORT                    		"http_wanport"
#define LAN_BR                              		"lan_br"
#define LAN_DHCP                            		"lan_dhcp"
#define LAN_DNS                             		"lan_dns"
#define LAN_DOMAIN                          		"lan_domain"
#define LAN_GATEWAY                         		"lan_gateway"
#define LAN_HWNAMES                         		"lan_hwnames"
#define LAN_IFNAME                          		"lan_ifname"
#define LAN_IFNAMES                        		"lan_ifnames"
#define LAN_IPADDR                          		"lan_ipaddr"
#define LAN_LEASE                           		"lan_lease"
#define LAN_NETMASK                         		"lan_netmask"
#define LAN_PROTO                           		"lan_proto"
#define LAN_ROUTE                           		"lan_route"
#define LAN0_PPPOE_IFNAME                   		"lan0_pppoe_ifname"
#define LAN1_DHCP                          	 	"lan1_dhcp"
#define LAN1_DOMAIN                         		"lan1_domain"
#define LAN1_GATEWAY                        		"lan1_gateway"
#define LAN1_HWNAMES                        		"lan1_hwnames"
#define LAN1_IFNAME                         		"lan1_ifname"
#define LAN1_IPADDR                         		"lan1_ipaddr"
#define LAN1_LEASE                          		"lan1_lease"
#define LAN1_NETMASK                        		"lan1_netmask"
#define LAN1_PROTO                          		"lan1_proto"
#define LAN1_ROUTE                          		"lan1_route"
#define LAN1_DNS						"lan1_dns"
#define LAN_DEVS                            		"landevs"
#define LAN_LOG_IPADDR                      		"log_ipaddr"
#define LAN_LOG_LEVEL                       		"log_level"
/*------------------高级功能参数----------------*/

#define ADVANCE_FW_DISABLE                  		"fw_disable"
/*+++++++++++++++++++upnp++++++++++++++++++++++*/
#define ADVANCE_UPNP_ENABLE                 		"upnp_enable"


/*+++++++++++++++++++远程web访问++++++++++++++++*/
#define ADVANCE_RM_WEB_EN                   		"rm_web_en"
#define ADVANCE_RM_WEB_IP                   		"rm_web_ip"
#define ADVANCE_RM_WEB_PORT                 	        "rm_web_port"

/*+++++++++++++++++++++DDNS+++++++++++++++++++++*/
#define ADVANCE_DDNS_ENABLE                 		"ddns_enable"
#define ADVANCE_DDNS_SET1                   		"ddns_set1"
#define ADVANCE_DDNS_ISP				"ddns_isp"
#define ADVANCE_DDNS_HOSTNAME    			"ddns_hostname1"
#define ADVANCE_DDNS_STATUS				"ddns_status"

/*+++++++++++++++++DMZ&&protMapping+++++++++++++*/
#define ADVANCE_DMZ_IPADDR                  		"dmz_ipaddr"
#define ADVANCE_DMZ_IPADDR_EN               		"dmz_ipaddr_en"
#define ADVANCE_PORT_FORWART_LIST          		"forward_port_list"
#define ADVANCE_PORT_FORWART_RULE(i)        		racat("forward_port", i)

/*+++++++++++++++++ELINK+++++++++++++*/
#define ADVANCE_ELINK_ENABLE             		"elink_enable"

/*+++++++++++++++++staticIPMapping++++++++++++++*/
#define ADVANCE_STATIC_IP_MAPPING_LIST      "dhcp_static_list"
#define ADVANCE_STATIC_IP_MAPPING_RULE(i)   racat("dhcp_static_lease", i)

/*+++++++++++++++++FilterWrap++++++++++++++++*/

#define ADVANCE_MACFILTER_CUR_NU            	"filter_mac_cur_nu"
#define ADVANCE_MACFILTER_LOC              	"filter_mac_loc"
#define ADVANCE_MACFILTER_MODE              	"filter_mac_mode"
#define ADVANCE_MACFILTER_NUM               	"filter_mac_num"
#define ADVANCE_MACFILTER_DENY_RULE(i)      	racat("filter_mac", i)
#define ADVANCE_MACFILTER_PASS_RULE(i)      	racat("white_list", i)
#define ADVICE_MAC_LIST						"wlan_maclist"
#define MACFILTER_ITEM_LEN 		17
#define MAX_MACFILTER_LIST_MUM  	20

#define ADVANCE_HOSTFILTER_MODE			"filter_hostname_mode"
#define ADVANCE_HOSTFILTER_MODE_CUR_NU		"filter_hostname_cur_nu"
#define ADVANCE_HOSTFILTER_DENY_RULE(i)		racat("filter_hostname", i)

#define ADVANCE_URLFILTER_CUR_NU            	"filter_url_cur_nu"
#define ADVANCE_URLFILTER_MODE              	"filter_url_mode"
#define ADVANCE_URLFILTER_RULE(i)           	racat("filter_url", i)
/*+++++++++++++++++++++++++++++++++++++++*/
#define ADVANCE_PARENT_CONTROL_CLIENTNUM		       "parentCtl_client_num"
#define ADVANCE_PARENT_CONTROL_RULE(i)	   	        	racat("parentCtl_client_list",i)		
#define ADVANCE_PARENT_CONTROL_TIME				"parentCtl_time"
#define ADVANCE_PARENT_CONTROL_DATE				"parentCtl_date"
#define ADVANCE_PARENT_CONTROL_URLFILTER_MODE	"parentCtl_urlFilter_mode"
#define ADVANCE_PARENT_CONTROL_URLFILTER_LIST	"parentCtl_urlFilter_list"
/*++++++++++++++++++++++remark++++++++++++++++++*/

#define ADVANCE_DEVICE_REMARK_NUM           		"device_remark_num"
#define ADVANCE_DEVICE_REMARK(i)            		racat("device_remark_list", i)

/*++++++++++++++++++++++流控++++++++++++++++++++*/
#define ADVANCE_TC_RULE(i)                  			racat("tc_", i)
#define ADVANCE_TC_ENABLE                   			"tc_enable"
#define ADVANCE_TC_ISP_DOWNRATE             		        "tc_isp_downrate"
#define ADVANCE_TC_ISP_UPRATE               			"tc_isp_uprate"
#define ADVANCE_TC_LOC                      			"tc_loc"
#define ADVANCE_TC_NUM                      			"tc_num"

/*++++++++++++++++++++++LED++++++++++++++++++++*/
#define SAVE_POWER_LED_TYPE						"led_ctl_type"   //on,off,duration
#define SAVE_POWER_LED_TIME						"led_time"						
#define ADVANCE_IPTV_ENABLE				"iptv_enable"
#define ADVANCE_IPTV_PORT				"iptv_portName"
#define ADVANCE_IPTV_VLAN_ZONE			"vlan_zone"
#define ADVANCE_IPTV_VLAN_ID			"vlan_id"		//vlan_id=1,2,3,4,5,6
#define ADVANCE_IPTV_VLAN_SEL			"vlan_select"
#define ADVANCE_IPTV_VLAN_ID_CUR		"vlan_id_cur"	//当前选中的vlan id,vlan_id中的某个值
/*++++++++++++++++++++++WAN口防ping++++++++++++++++++++*/
#define _PING_WAN_EN   "pingwan_enable"
/***********************功能模块区分************************/
/*---------------------wan_mode_check----------------------*/
#define FUNCTION_WAN_MODE_CHECK_ENABLE      		"mode_need_switch"
/*-------------------------firewall------------------------*/
#define FUNCTION_FIREWALL_NAT_TYPE          		"nat_type"
/*-------------------------sntp----------------------------*/
#define FUNCTION_SNTP_SERVER                		"ntp_server"
/*---------------------reboot_check------------------------*/
#define FUNCTION_AUTO_MAINTENANCE_ENABLE    		"reboot_enable"
#define FUNCTION_AUTO_MAINTENANCE_MAX_RX    		"reboot_max_rx_bytes"
#define FUNCTION_AUTO_MAINTENANCE_MAX_TX    		"reboot_max_tx_bytes"
#define FUNCTION_AUTO_MAINTENANCE_TIME      		"reboot_time"
#define FUNCTION_AUTO_MAINTENANCE_TYPE      		"reboot_type"
/*----------------------时区设置---------------------------*/
#define FUNCTION_TIME_MODE                  		"time_mode"
#define FUNCTION_TIME_ZONE                  		"time_zone"


/*************************common****struct******************************/
#ifdef __CONFIG_GUEST__
#define MAX_BRIDGE_NUM					2
#else
#define MAX_BRIDGE_NUM					1
#endif
#define TENDA_WLAN24_AP_IFNAME 			"wlan1"
#define TENDA_WLAN24_REPEATER_IFNAME 	"wlan1-vxd0"
#define TENDA_WLAN24_GUEST_IFNAME 		"wlan1-va0"
#define TENDA_WLAN24_VIRTUAL_IFNAME		"wlan1-va1"
#define TENDA_WLAN5_AP_IFNAME 			"wlan0"
#define TENDA_WLAN5_REPEATER_IFNAME 		"wlan0-vxd0"
#define TENDA_WLAN5_GUEST_IFNAME 		"wlan0-va0"
#define TENDA_WLAN5_VIRTUAL_IFNAME		"wlan0-va1"
#define TENDA_WLAN_SUPPORT_HZ                   2

 struct upgrade_mem{
	struct upgrade_mem *next;
	int totlen;
	int inuse;
	char *data;
};

typedef enum network_interface_state{
	INTERFACE_UP = 0,		
	INTERFACE_DOWN,
	INTERFACE_STAT_UNKNOW,
}NETWORK_INTERFACE_STATE;


#ifndef nvram_safe_get
#define nvram_safe_get(name) (nvram_get(name) ? : "")
#endif

 char *  racat(char *s, int i);
int 	get_wan_type(void);
int 	get_wan_connstatus(void);
int   get_wan_onln_connstatus(void);//gong add 标记能否上网
int 	get_wan_linkstatus(void);
void set_dhcpd_en(int enable);
int 	CGI_same_net_with_lan(unsigned int wan_ip,unsigned int wan_mask);
int 	CGI_do_wan_connect_tenda(int action);
/*****************************************************************************
 函 数 名  : get_interface_state
 功能描述  : 获取当前接口的状态，
 输入参数  : char* net_name  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年10月24日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
NETWORK_INTERFACE_STATE get_interface_state(char* net_name);
#endif
