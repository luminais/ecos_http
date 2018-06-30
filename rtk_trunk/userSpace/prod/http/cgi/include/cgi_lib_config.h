#ifndef __CGI_LIB_CONFIG_H__
#define __CGI_LIB_CONFIG_H__
#include "webs.h"
#include "cJSON.h"
#include "pi_common.h"
#include "cgi_common.h"
/*下面的index必须保持顺序，与*/
/********************************module index(get)***********************************/
enum get_module_index{
		MODULE_GET_WAN_TYPE = 0,				     	//获取wan口接入类型
		MODULE_GET_ADSL_INFO,  			        	//获取pppoe的账号和密码
		MODULE_GET_NET_INFO,						//获取wan口ip/ipmask/gateway/dns1/dns2
		MODULE_GET_WAN_DETECTION,				//获取wan的接入类型dhcp/pppoe/static/detecting
		MODULE_GET_WAN_SERVER,			        	//获取wan的pppoe service和server name
		MODULE_GET_WAN_MTU,						//获取wan口的MTU
		MODULE_GET_WAN_SPEED,			      		//获取wan口速率类型和当前速率
		MODULE_GET_MAC_CLONE,					//获取mac克隆类型以及路由器原本的MAC地址
		MODULE_GET_WAN_MAC,						//获取wan口当前的mac地址
		MODULE_GET_SYSTEM_STATUS_INFO,  
		MODULE_GET_STATUS_WANMAC,              
		MODULE_GET_SOFTWARE_VERSION,    
		MODULE_GET_WAN_CONNECT_TIME,  
#ifdef __CONFIG_TENDA_APP__
		MODULE_GET_UCLOUD_VERSION,             //页面在线升级 获取新版本 
		MODULE_GET_UCLOUD_UPGRADE,             //立即升级
#endif
		MODULE_GET_WIFISCANRESAULT,       
		MODULE_GET_NATIVE_HOST_MAC,				//获取当前进入管理界面的主机mac
		MODULE_GET_LAN_INFO,				      		//获取lan口ip/ipmask
		MODULE_GET_DHCP_SERVER_INFO,		       //获取dhcp server的配置信息
		MODULE_GET_TC_ONLINE_LIST,				//获取上网控制页面的在线列表信息
#ifdef __CONFIG_GUEST__
		MODULE_GET_GUEST_ONLINE_LIST,				//获取访客网络在线列表信息
#endif
		MODULE_GET_LOCALHOST,					//获取本机IP
		MODULE_GET_ONLINE_NUM,					//获取在线客户端的个数
		MODULE_GET_BLACK_NUM,					//获取无线黑名单的个数
		MODULE_GET_STREAM_STATISTIC,				//获取上下行的总流量
		MODULE_GET_WIFI_RATE,						//获取当前wifi的速率
		MODULE_GET_WIFI_NAME,					//获取主ssid和次ssid
		MODULE_GET_WIFI_EN,						//获取wifi开关
		MODULE_GET_WIFI_BASIC,					//获取wifi基本参数
		MODULE_GET_WIFI_POWER,					//获取无线功率
		MODULE_GET_WIFI_SCHED,					//获取无线定时开关
		MODULE_GET_WIFI_RELAY_TYPE,				//获取无线中继类型
		MODULE_GET_WIFI_ADV_CFG,					//获取无线高级参数
		MODULE_GET_WIFI_WPS,						//获取WPS信息
		MODULE_GET_WIFIRELAY_INFO,              		//获取wifiRelay 信息		
		MODULE_GET_WIFIRELAY_CONNECT_DUTATION, //无线中继下连接时间
		MODULE_GET_ISWIFICLIENT,                   		//是否是无线客户端
		MODULE_GET_MACFILTER_MODE,		
		MODULE_GET_MACFILTER_LIST,			
		MODULE_GET_PARENT_ACCESS_CTRL,	
		MODULE_GET_PARENT_ONLINE_LIST,
		MODULE_GET_REMOTE_INFO,					//获取远程管理的IP和端口号
		MODULE_GET_FIREWARE,						//获取软件版本信息
		MODULE_GET_SYSTIME,						//获取系统时间
		MODULE_GET_LOGIN_PWD,					//获取web登录密码
		MODULE_GET_INTERNET_STATUS,				//获取系统联网状态
		MODULE_GET_NAT_STATICIP_LIST,               	//获取静态ip列表
		MODULE_GET_NAT_PORT_FORWARD,               	//获取端口转发
		MODULE_GET_NAT_DDNS,                        		//获取ddns
		MODULE_GET_NAT_DMZ,                         			//获取DMZ信息
		MODULE_GET_NAT_UPNP,                        		//获取upnp信息
		MODULE_GET_WLFI_CHANNEL_GRADE,					//获取当前无线优劣等级
		MODULE_GET_EXPIRED_CLIENT_NUM,            //离线客户端数量
		MODULE_GET_EXPIRED_INFO,                //获取离线设备信息 
#ifdef __CONFIG_AUTO_CONN_CLIENT__
		MODULE_GET_AUTO_SYNC_INFO,                //获取自动同步配置是否结束
#endif
#ifdef __CONFIG_GUEST__
		MODULE_GET_WIFI_GUEST_INFO,              //获取访客网络基本信息
#endif 
#ifdef __CONFIG_WL_BEAMFORMING_EN__
		MODULE_GET_BEAFORMING_ENABLE,            //获取beaforming开关
#endif 
#ifdef __CONFIG_IPTV__
		MODULE_GET_NAT_IPTV,							//获取IPTV信息
#endif
#ifdef __CONFIG_LED__
		MODULE_GET_LED_INFO,							//获取LED
		MODULE_GET_LED_INFO_APP,						//用于APP获取LED状态
#endif

		MODULE_GET_END     
};
/*****************************************************************************/


/********************************module index(set)***********************************/
enum set_module_index{
		MODULE_SET_WAN_ACCESS = 0,			 	//配置wan口接入
		MODULE_SET_WAN_MTU,						//配置wan口MTU
		MODULE_SET_WAN_SPEED	,					//配置wan口速率
		MODULE_SET_MAC_CLONE	,					//配置wan口MAC 克隆
		MODULE_SET_WAN_SERVER,					//配置wan口的pppoe service和server name
		MODULE_SET_LAN_INFO,						//配置lan口ip dhcp地址池等
		MODULE_SET_WIFI_EN,						//设置wifi开关
		MODULE_SET_WIFI_BASIC,					//设置wifi基本参数
		MODULE_SET_WIFI_POWER,					//设置无线功率
		MODULE_SET_WIFI_SCHED,					//设置无线定时开关
		MODULE_SET_WIFI_ADV_CFG,					//设置取无线高级参数
#ifdef __CONFIG_WPS_RTK__
		MODULE_SET_WIFI_WPS,						//设置WPS信息
#endif
		MODULE_SET_WIFIRELAY,                 			 //wifiRelay设置
		MODULE_SET_WIZARD_SYSTIME,            		 //时区设置
		MODULE_SET_QOS_TC,						//配置限速，可单条也可以多条操作
		MODULE_SET_MACFILTER	,	
		MODULE_SET_PARENT_ONLINELIST,		
		MODULE_SET_PARENT_ACCESS_CTRL,
		MODULE_SET_REMOTE_INFO,					//配置远程web管理
		MODULE_SET_FIREWARE,						//配置软件版本信息
		MODULE_SET_OPERATE,						//设置reboot，reset
		MODULE_SET_SYSTIME,						//设置系统时间
		MODULE_SET_LOGIN_PWD,					//设置web登录密码
		MODULE_SET_NAT_STATICIP,                 //配置静态ip
		MODULE_SET_NAT_PORT_FORWARD,             //配置端口转发
		MODULE_SET_NAT_DDNS,                          //配置ddns
		MODULE_SET_NAT_DMZ,                           //配置DMZ主机
		MODULE_SET_NAT_UPNP,                         //配置UPNP
#ifdef __CONFIG_TENDA_APP__
		MODULE_SET_NO_NOW_UPGRADE,
#endif
		MODULE_SET_WLFI_CHANNEL_GRADE,   			//设置自动信道
		MODULE_SET_RUB_NET_ADD_BLACKLIST,           //添加黑名单 (单条生效)
		MODULE_SET_RUB_NET_DELETE_BLACKLIST,
		MODULE_SET_RUB_NET_ADD_TRUSTLIST,           //关闭在线提醒
		MODULE_SET_RUB_NET_DELETE_TRUSTLIST,        //开启在线提醒
		MODULE_SET_WIZARD_SUCCEED,					//快速设置完成
		MODULE_SET_MACFILTER_MODE,					//设置黑白名单模式
		MODELE_SET_FLUSH_BLACKLIST,					//清空所有的黑名单
#ifdef __CONFIG_GUEST__
		MODULE_SET_WIFI_GUEST_INFO,              //获取访客网络基本信息
#endif
#ifdef __CONFIG_WL_BEAMFORMING_EN__
		MODULE_SET_BEAFORMING_ENABLE,
#endif
#ifdef __CONFIG_IPTV__
		MODULE_SET_NAT_IPTV,							//设置IPTV
#endif
#ifdef __CONFIG_LED__
		MODULE_SET_LED_INFO,							//获取LED
#endif

		MODULE_SET_END
};
/*****************************************************************************/

typedef struct cgi_lib_info{
	webs_t	 		wp;
	cJSON *			root;
	unsigned char* 	modules;
	int 				module_num;
}CGI_LIB_INFO;

typedef struct cgi_lib_module_info{
	PIU8 module_index;
	union
	{
		RET_INFO (*get_fun)(webs_t wp,cJSON *root, void *info);
		RET_INFO (*set_fun)(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code, void *info);
	} cgi_action;
	
}CGI_LIB_MODULE_INFO;

#define CGI_LIB_GET_FUN(node) (((node).cgi_action).get_fun)
#define CGI_LIB_SET_FUN(node) (((node).cgi_action).set_fun)

/****************************param define****************************************/
#define LIB_WAN_TYPE   					"wanType"
#define LIB_PPPOE_USER 					"wanPPPoEUser"
#define LIB_PPPOE_PWD					"wanPPPoEPwd"
#define LIB_WAN_IP           				"wanIP"
#define LIB_WAN_MASK					"wanMask"
#define LIB_WAN_GATEWAY 				"wanGateway"
#define LIB_WAN_DNS1					"wanDns1"
#define LIB_WAN_DNS2					"wanDns2"
#define LIB_WAN_DETECTION				"wanDetection"
#define LIB_WAN_SERVERNAME	 	    	"wanServerName"
#define LIB_WAN_SERVICENAME		    	"wanServiceName"
#define LIB_WAN_MTU						"wanMTU"
#define LIB_WAN_CURR_MTU				"wanMTUCurrent"
#define LIB_WAN_SPEED					"wanSpeed"
#define LIB_WAN_CURR_SPEED				"wanSpeedCurrent"
#define LIB_WAN_MAC_CLONE				"macClone"
#define LIB_WAN_MAC_ROUTER		    	"macRouter"
#define LIB_WAN_MAC_EIFI_DEV 			"macWifiDevice"
#define LIB_WAN_MAC_CUR_WAN		    	"macCurrentWan"
#define LIB_WAN_MAC_HOST				"macHost"
#define LIB_WAN_MAC						"wanMAC"
#define LIB_LAN_IP						"lanIP"
#define LIB_LAN_MASK					"lanMask"
#define LIB_LAN_DNS1						"lanDns1"
#define LIB_LAN_DNS2						"lanDns2"
#define LIB_LAN_DHCP_ENABLE		    	"dhcpEn"
#define LIB_LAN_DHCP_START_IP			"lanDhcpStartIP"
#define LIB_LAN_DHCP_END_IP				"lanDhcpEndIP"
#define LIB_STATUS_ONLINE_NUM			"statusOnlineNumber"
#define LIB_STATUS_BLACK_NUM			"statusBlackNum"
#define LIB_STATUS_UP_SPEED				"statusUpSpeed"
#define LIB_STATUS_DOWN_SPEED			"statusDownSpeed"
#define LIB_WIFI_RATE			    		"wifiRate"
#define LIB_ROUTER_NAME			   	"routerName"
#define LIB_EXTEND_NAME			   		"extendName"
#define LIB_LOCAL_HOST			    		"localhost"
#define LIB_LOCAL_HOST_MAC			    		"localhostMAC"
#define LIB_QOS_LIST_HOSTNAME			"qosListHostname"
#define LIB_QOS_LIST_REMARK			"qosListRemark"
#define LIB_QOS_LIST_IP					"qosListIP"
#define LIB_QOS_LIST_CONN_TYPE			"qosListConnectType"
#define LIB_QOS_LIST_ACCESS_TYPE		"qosListAccessType"		//接入类型2G,5G,2G访客,5G访客,APP用
#define LIB_QOS_LIST_MAC				"qosListMac"
#define LIB_QOS_LIST_MANUFACTURER		"qosListManufacturer"
#define LIB_QOS_LIST_DOWN_SPEED		"qosListDownSpeed"
#define LIB_QOS_LIST_UP_SPEED			"qosListUpSpeed"
#define LIB_QOS_LIST_DOWN_LIMIT		"qosListDownLimit"
#define LIB_QOS_LIST_UP_LIMIT			"qosListUpLimit"
#define LIB_QOS_LIST_ACCESS				"qosListAccess"
#define LIB_QOS_LIST_CONNECT_TIME       "qoslistConnetTime"
#define LIB_QOS_ONLINE_LIST         		"onlineList"
#define LIB_MAC_FILTER_MODE             		"filterMode"
#define LIB_MAC_FILTER_CUR_MODE             "curFilterMode"
#define LIB_MAC_FILTER_LIST             		"macFilterList"
#define LIB_WL1_MAC_MODE             		"wl0.1_macmode"
#define LIB_WL0_MAC_MODE             		"wl0_macmode"
#define LIB_HOST_NAME             			"hostname"
#define LIB_REMARK                  			"remark"
#define LIB_FILTER_MAC                  		"mac"
#define LIB_REMOTE_ENABLE				"remoteWebEn"
#define LIB_REMOTE_TYPE					"remoteWebType"
#define LIB_REMOTE_IP					"remoteWebIP"
#define LIB_REMOTE_PORT					"remoteWebPort"
#define LIB_SOFT_VER 					"softVersion"
#define LIB_AUTO_MAINT_ENABLE			"autoMaintenanceEn"
#define LIB_SYS_TIME_ZONE				"sysTimeZone"
#define LIB_SYS_CURRENT_TIME			"sysTimecurrentTime"
#define LIB_SYS_SNTP_TYPE				"sysTimeSntpType"
#define LIB_SYS_INTER_STATE				"internetState"
#define LIB_HAS_HTTP_PWD				"hasLoginPwd"
#define LIB_HTTP_USERNAME				"username"
#define LIB_WAN_CONNECT_STATUS 		"wanConnectStatus"
#define LIB_NEW_LAN_IP					"newLanIP"
#define LIB_LAN_WAN_IP_CONFILICT    		"lanWanIPConflict"
#define LIB_WIFIRELAY_SSID           		"wifiRelaySSID"
#define LIB_UPPERWIFISSID          	  		"upperWifiSsid"
#define LIB_EXTENDERSSID            	 		"extenderSsid"
#define LIB_WIFIRELAYMAC             		"wifiRelayMAC"
#define LIB_WIFIRELAY_SECURITY_TYPE  	"wifiRelaySecurityMode"
#define LIB_WIFIRELAY_SECURITY_PASS  	"wifiRelayPwd"
#define LIB_EXTENDERPWD              			"extenderPwd"
#define LIB_WIFIRELAY_CHANNEL        		"wifiRelayChannel"
#define LIB_WIFIRELAY_CONNECT_STATUS   "wifiRelayConnectStatus"
#define LIB_WIFIRELAY_CHK_HZ			"wifiRelayChkHz"
#define LIB_WIFI_EN						"wifiEn"
#define LIB_WIFI_EN_5G					"wifiEn_5G"
#define LIB_WIFI_HIDE_SSID				"wifiHideSSID"
#define LIB_WIFI_HIDE_SSID_5G			"wifiHideSSID_5G"
#define LIB_WIFI_AP_SSID				"wifiSSID"
#define LIB_WIFI_AP_SSID_5G				"wifiSSID_5G"
#define LIB_WIFI_AP_SEC_MODE			"wifiSecurityMode"
#define LIB_WIFI_AP_SEC_MODE_5G			"wifiSecurityMode_5G"
#define LIB_WIFI_AP_PWD					"wifiPwd"
#define LIB_WIFI_AP_PWD_5G				"wifiPwd_5G"
#define LIB_WIFI_AP_NO_PWD				"wifiNoPwd"
#define LIB_WIFI_AP_NO_PWD_5G			"wifiNoPwd_5G"
#define LIB_WIFI_POWER					"wifiPower"
#define LIB_WIFI_POWER_5G				"wifiPower_5G"
#define LIB_WIFI_POWER_GEAR			    "wifiPowerGear"
#define LIB_WIFI_POWER_GEAR_5G			"wifiPowerGear_5G"
#define LIB_WIFI_TIME_EN				"wifiTimeEn"
#define LIB_WIFI_TIME_CLOSE				"wifiTimeClose"
#define LIB_WIFI_TIME_DATE				"wifiTimeDate"
#define LIB_WIFI_RELAY_TYPE				"wifiRelayType"
#define LIB_WIFI_RELAY_TYPE_5G			"wifiRelayType_5G"
#define LIB_WIFI_BGN_MODE				"wifiMode"
#define LIB_WIFI_BGN_MODE_5G			"wifiMode_5G"
#define LIB_WIFI_M_CH					"wifiMaxChannel"
#define LIB_WIFI_M_CH_5G				"wifiMaxChannel_5G"
#define LIB_WIFI_CHANNEL_LIST			"wifiChannelList"
#define LIB_WIFI_CHANNEL_LIST_5G		"wifiChannelList_5G"
#define LIB_WIFI_CH						"wifiChannel"
#define LIB_WIFI_CH_5G					"wifiChannel_5G"
#define LIB_WIFI_C_CH					"wifiChannelCurrent"
#define LIB_WIFI_C_CH_5G				"wifiChannelCurrent_5G"
#define LIB_WIFI_BW						"wifiBandwidth"
#define LIB_WIFI_BW_5G					"wifiBandwidth_5G"
#define LIB_WIFI_C_BW					"wifiBandwidthCurrent"
#define LIB_WIFI_C_BW_5G				"wifiBandwidthCurrent_5G"
#define LIB_WIFI_ANTIJAM_EN				"wifiAntijamEn"
#define LIB_HAS_WIFI_ANTIJAM			"hasWifiAntijam"
#define LIB_WIFI_BEAFORMING_ENABLE      "wifiBeaformingEn"
#define LIB_HAS_WIFI_BEAFORMING         "hasWifiBeaforming"
#define LIB_TD_DIG_SEL					"td_dig_sel"
#define LIB_TD_DIG_AUTO				"td_auto_antiInterferance"
#define LIB_TD_DIG_CUR_FA				"cur_fa"
#define LIB_TD_DIG_CUR_IGI				"cur_igi"

#define LIB_WIFI_HAS_DOUBLEBAND_UNITY    "HasDoubleBandUnity"
#define LIB_WIFI_TOTAL_ENABLE            "wifiTotalEn"
#define LIB_WIFI_DOUBLEBAND_UNITY_ENABLE "doubleBandUnityEnable"

/*wifi guest param*/
#define LIB_WIFI_GUEST_EN				"guestEn"
#define LIB_WIFI_GUEST_SSID             "guestSSID"
#define LIB_WIFI_GUEST_HIDE_SSID        "guestHideSSID"
#define LIB_WIFI_GUEST_PWD              "guestPwd"
#define LIB_WIFI_GUEST_SEC_MODE         "guestSecurityMode"
#define LIB_WIFI_GUEST_EN_5G			"guestEn_5G"
#define LIB_WIFI_GUEST_SSID_5G          "guestSSID_5G"
#define LIB_WIFI_GUEST_HIDE_SSID_5G     "guestHideSSID_5G"
#define LIB_WIFI_GUEST_PWD_5G           "guestPwd_5G"
#define LIB_WIFI_GUEST_SEC_MODE_5G      "guestSecurityMode_5G"
#define LIB_WIFI_GUEST_EFFECT_TIME      "guestValidTime"
#define LIB_WIFI_GUEST_SHARE_SPEED      "guestShareSpeed"

#define LIB_WPS_HAS_MODE                "hasWPSModule"
#define LIB_WPS_EN						"wpsEn"
#define LIB_WPS_PIN						"wpsPIN"

#define LIB_CONNECT_STATE               		"connectState"      //A9? ?П??l???
#define LIB_CONNECT_DURATION           		"connectDuration"   //A9??т?? l???

#define LIB_ISWIFICLIENT             			"isWifiClients"    
#define LIB_SYSTIMEZONE              			"sysTimeZone"
#define LIB_STATUS_WAN_MASK          		"statusWanMask"
#define LIB_STATUS_WAN_IP           		"statusWanIP"
#define LIB_STATUS_WAN_GATWAY           	"statusWanGaterway"
#define LIB_WAN_CONNECT_TIME         		"wanConnectTime"
#define LIB_STATUS_DNS1              			"statusWanDns1"
#define LIB_STATUS_DNS2              			"statusWanDns2"                 
#define LIB_SOFTWARE_VERSION         		"softVersion"
#define LIB_STATUS_WAN_MAC           		"statusWanMAC"
#define LIB_HANSNEWSOFTVERSION      		"hasNewSoftVersion"
#define LIB_UPDATEVERSION           			"updateVersion"
#define LIB_NEWVERISONOPTIMIZE      		"newVersionOptimize"
#define LIB_UPDATEDATE              			"updateDate"	
#define LIB_UPGRADEREADY            			"upgradeReady"
#define LIB_WRITING_TO_FLASH                    "writing"
#define LIB_TOTALSIZE               			"totalSize"
#define LIB_DOWNLOADSIZE            			"downloadSize"
#define LIB_RESTTIME                			"restTime"
#define LIB_UPGRADEERORCODE         		"upgradeErrorCode"
#define LIB_NOPROMPT                			"noPrompt"
#define LIB_STATIC_IPLIST_IP    			"staticIP"
#define LIB_STATIC_IPLIST_MAC    			"staticMac"
#define LIB_STATIC_IPLIST_MARK   		"staticRemark"
#define LIB_NAT_DDNS_EN             			"ddnsEn"
#define LIB_NAT_DDNS_SERVICE_NAME 		"ddnsServiceName"
#define LIB_NAT_DDNS_USER           		"ddnsUser"
#define LIB_NAT_DDNS_PWD            		"ddnsPwd"
#define LIB_NAT_DDNS_SERVER         		"ddnsServer"
#define LIB_NAT_DDNS_STATUS        		"ddnsStatus"
#define LIB_NAT_DMZ_EN              			"dmzEn"
#define LIB_NAT_DMZ_HOST_IP         		"dmzHostIP"
#define LIB_NAT_UPNP_EN             			"upnpEn"
#define LIB_CHANNEL_2G_GRADE					"channelGrade_2g"
#define LIB_EXPIRED_IS                  "IsExpired"
#define LIB_EXPRIED_NUM                 "ExpriedNum"
#ifdef __CONFIG_AUTO_CONN_CLIENT__
#define LIB_AUTO_SYNC_STATUS             "synchroStatus"
#endif
#ifdef __CONFIG_IPTV__
#define LIB_NAT_IPTV_EN					"IPTVEn"
#define LIB_NAT_IPTV_ZONE				"VLANArea"
#define LIB_NAT_IPTV_VLANID				"VLANID"
#define LIB_NAT_IPTV_SELECT				"VLANSelect"
#endif
#ifdef __CONFIG_LED__
#define LIB_LED_STATUS					"LEDStatus"
#define LIB_LED_CLOSE_TIME				"LEDCloseTime"
#endif


/******************************************************************************/

/***********************************cgi lib get************************************/
RET_INFO cgi_lib_get_wan_type(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wan_type(webs_t wp, cJSON *root, void *info);	
RET_INFO cgi_lib_get_adsl_info(webs_t wp, cJSON *root, void *info);	
RET_INFO cgi_lib_get_net_info(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wan_detection(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wan_server(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wan_mtu(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wan_speed(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_mac_clone(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wan_mac(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_native_host_mac(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_lan_info(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_dhcp_server_info(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_tc_online_list(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_guest_online_list(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_localhost(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_online_num(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_black_num(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_stream_statistic(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_rate(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_name(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_enable(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_basic(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_power(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_sched(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_relay_type(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifi_adv_cfg(webs_t wp, cJSON *root, void *info);
#ifdef __CONFIG_WPS_RTK__
RET_INFO cgi_lib_get_wifi_wps(webs_t wp, cJSON *root, void *info);
#endif 
RET_INFO cgi_lib_get_wifiRelay_info(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_connect_duration(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_isWifiClient(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_macfilter_list(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_macfilter_mode(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_parentAccessCtrl(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_parent_online_list(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_remote_info(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_fireware(webs_t wp, cJSON *root, void * info);
RET_INFO cgi_lib_get_systime(webs_t wp, cJSON *root, void * info);
RET_INFO cgi_lib_get_login_pwd(webs_t wp, cJSON *root, void * info);
RET_INFO cgi_lib_get_internet_status(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_system_status_info(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_software_version(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wan_connectTime(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_system_status_wanMac(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_wifiScanresault(webs_t wp, cJSON *root, void *info);

#ifdef __CONFIG_TENDA_APP__
RET_INFO cgi_lib_get_ucloud_upgrade(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_ucloud_version(webs_t wp, cJSON *root, void *info);
#endif 

RET_INFO cgi_lib_get_nat_staticip(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_nat_portForward(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_nat_ddns(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_nat_dmz(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_nat_upnp(webs_t wp, cJSON *root, void *info);

RET_INFO cgi_lib_get_wifi_channel_grade(webs_t wp, cJSON *root, void *info);

RET_INFO cgi_lib_get_expired_client_num(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_all_expired_info (webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_router_mode_black_list(webs_t wp, cJSON *root, void *info);
#ifdef __CONFIG_AUTO_CONN_CLIENT__
RET_INFO cgi_lib_auto_sync_info_get(webs_t wp, cJSON *root, void *info);
#endif
#ifdef __CONFIG_IPTV__
RET_INFO cgi_lib_get_nat_iptv(webs_t wp, cJSON *root, void *info);
#endif
#ifdef __CONFIG_LED__
RET_INFO cgi_lib_get_led(webs_t wp, cJSON *root, void *info);
RET_INFO cgi_lib_get_led_app(webs_t wp, cJSON *root, void *info);
#endif

#ifdef __CONFIG_GUEST__
RET_INFO cgi_lib_get_wifi_guest_info(webs_t wp, cJSON *root, void *info);
#endif

#ifdef __CONFIG_WL_BEAMFORMING_EN__
RET_INFO cgi_lib_get_beaforming_enable(webs_t wp, cJSON *root, void *info);
#endif
/******************************************************************************/

/************************************************cgi lib set*********************************************/
RET_INFO cgi_lib_set_wan_access(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_wan_mtu(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_wan_speed(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_wan_macClone(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_wan_server(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_lan_info(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_wifi_basic(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info);
RET_INFO cgi_lib_set_wifi_power(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info);
RET_INFO cgi_lib_set_wifi_sched(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info);
RET_INFO cgi_lib_set_wifi_adv_cfg(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info);
RET_INFO cgi_lib_set_wifi_wps(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info);
RET_INFO cgi_lib_set_wifiRelay(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_tc_qoslist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_disposable_macfilter(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_parent_onlineList(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_parent_access_ctrl(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_remote_info(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_fireware(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_operate(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_systime(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_login_pwd(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_wziard_systime(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_nat_staticip(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_nat_portForward(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_nat_ddns(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_nat_dmz(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_nat_upnp(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
RET_INFO cgi_lib_set_wifi_channel_grade(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info);

#ifdef __CONFIG_TENDA_APP__
RET_INFO cgi_lib_notNow_upgrade_set(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
#endif 
RET_INFO cgi_lib_set_rub_net_add_blacklist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info);
RET_INFO cgi_lib_set_rub_net_delete_blacklist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info);
RET_INFO cgi_lib_set_rub_net_add_to_trustlist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info);
RET_INFO cgi_lib_set_rub_net_delete_from_trustlist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info);
RET_INFO cgi_lib_set_wizard_succeed(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info);
RET_INFO cgi_lib_set_macfilter_mode(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info);
RET_INFO cgi_lib_set_flush_blacklist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info);

#ifdef __CONFIG_GUEST__
RET_INFO cgi_lib_set_wifi_guest_info(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
#endif

#ifdef	__CONFIG_WL_BEAMFORMING_EN__
RET_INFO cgi_lib_set_beaforming_enable(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
#endif
#ifdef __CONFIG_IPTV__
RET_INFO cgi_lib_set_nat_iptv(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
#endif
#ifdef __CONFIG_LED__
RET_INFO cgi_lib_set_led(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info);
#endif

/****************************************************************************************************/

#endif __CGI_LIB_CONFIG_H__*/
