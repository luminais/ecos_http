/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_lib_wifiRelay.c
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2017年10月25日
  最近修改   :
  功能描述   : 

  功能描述   : 配置模式切换以及桥接

  修改历史   :
  1.日    期   : 2017年10月25日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/

#include "cgi_lib.h"
#include <wifi.h>
#include <wan.h>
#include <router_net.h>
#include <netinet/if_ether.h>
/*****************************************************************************
 函 数 名  : cgi_lib_get_wifiRelay_info
 功能描述  : 获取桥接接口配置信息
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年10月25日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wifiRelay_info(webs_t wp, cJSON *root, void *info)
{
	char ssid_value[64] = {0}, ssid_buf[64] = {0};
	char security_type[64] = {0}, security_pass[64] = {0}, relay_status[64] = {0};
	char wireless_channel[16] = {0};
	int wep_key_index = 0;
	int repeater_2g_enable = 0;
	int repeater_5g_enable = 0;
	WANSTATUS wan_status = WAN_DISCONNECTED;
	WIFISTASTATUS wifi_status = WIFI_OK;
	
	strcpy(relay_status, "disconnect");
	
	P_WAN_ERR_INFO_STRUCT wan_error_info = NULL;
	WIFI_INFO_STRUCT wireless_info;

	wan_error_info = gpi_wan_get_err_info_other();

	memset(&wireless_info, 0x0, sizeof(WIFI_INFO_STRUCT));
	if(nvram_match(WLAN24G_WORK_MODE,"sta"))
	{
		repeater_2g_enable = 1;
		memcpy(&wireless_info, gpi_wifi_get_info(WLAN_RATE_24G), sizeof(WIFI_INFO_STRUCT));
	}
	else
	{
		repeater_5g_enable = 1;
		memcpy(&wireless_info, gpi_wifi_get_info(WLAN_RATE_5G), sizeof(WIFI_INFO_STRUCT));
	}
	
	if (WL_WISP_MODE == gpi_wifi_get_mode() || WL_APCLIENT_MODE == gpi_wifi_get_mode())
	{
		/*SSID*/
		strcpy(ssid_value, wireless_info.sta_ssid_cfg.ssid);
		cJSON_AddStringToObject(root,LIB_WIFIRELAY_SSID, ssid_value);
		cJSON_AddStringToObject(root,LIB_UPPERWIFISSID, ssid_value);//A9页面使用该关键字
		cJSON_AddStringToObject(root,LIB_EXTENDERSSID, wireless_info.ap_ssid_cfg.ssid);//A9页面使用该关键字

		if (!strcmp(wireless_info.ap_ssid_cfg.security, "none"))
		{
			
			cJSON_AddStringToObject(root,LIB_EXTENDERPWD, ""); //A9页面使用该关键字
		}
		else
		{
			cJSON_AddStringToObject(root,LIB_EXTENDERPWD, wireless_info.ap_ssid_cfg.wpa_psk_key);//A9页面使用该关键字
		}

		/*channel*/
		sprintf(wireless_channel, "%d", wireless_info.channel);
		cJSON_AddStringToObject(root, LIB_WIFIRELAY_CHANNEL, nvram_safe_get(WLAN24G_CHANNEL));

		/*security*/
		if (0 == strcmp(wireless_info.sta_ssid_cfg.security, "none"))
		{
			strcpy(security_type, "none");
			strcpy(security_pass, "");
		}
		else if (0 == strcmp(wireless_info.sta_ssid_cfg.security, "wep"))
		{
			if (0 == strcmp(wireless_info.sta_ssid_cfg.wep_type, "open"))
			{
				strcpy(security_type, "OPEN/WEP");
			}
			else if (0 == strcmp(wireless_info.sta_ssid_cfg.wep_type, "share"))
			{
				strcpy(security_type, "SHARED/WEP");
			}

			wep_key_index = wireless_info.sta_ssid_cfg.wep_key;
			strcpy(security_pass, wireless_info.sta_ssid_cfg.wep_key_arry[wep_key_index]);
		}
		else if (0 == strcmp(wireless_info.sta_ssid_cfg.security, "wpapsk"))
		{
			if (0 == strcmp(wireless_info.sta_ssid_cfg.wpa_psk_type, "psk"))
			{
				strcpy(security_type, "wpa/");
			}
			else if (0 == strcmp(wireless_info.sta_ssid_cfg.wpa_psk_type, "psk2"))
			{
				strcpy(security_type, "wpa2/");
			}
			else if (0 == strcmp(wireless_info.sta_ssid_cfg.wpa_psk_type, "psk psk2"))
			{
				strcpy(security_type, "wpawpa2/");
			}

			if (!strcmp(wireless_info.sta_ssid_cfg.wap_psk_crypto, "aes"))
			{
				strncat(security_type, "AES", strlen("AES"));
			}
			else if (!strcmp(wireless_info.sta_ssid_cfg.wap_psk_crypto, "tkip"))
			{
				strncat(security_type, "TKIP", strlen("TKIP"));
			}
			else if (!strcmp(wireless_info.sta_ssid_cfg.wap_psk_crypto, "tkip+aes"))
			{
				strncat(security_type, "AESTKIP", strlen("AESTKIP"));
			}

			strcpy(security_pass, wireless_info.sta_ssid_cfg.wpa_psk_key);
		}
		else
		{
			strcpy(security_type, "none");
			strcpy(security_pass, "");
		}

		cJSON_AddStringToObject(root, LIB_WIFIRELAY_SECURITY_TYPE, security_type);
		cJSON_AddStringToObject(root, LIB_WIFIRELAY_SECURITY_PASS, security_pass);
	

		gpi_wifi_get_status_info(&wan_status, &wifi_status);

		if (wan_error_info->code == STATIC_WL_CHECKED_PASSWORD_FAIL || wan_error_info->code == DHCP_WL_CHECKED_PASSWORD_FAIL ||
		    wan_error_info->code == PPPOE_WL_CHECKED_PASSWORD_FAIL)
		{
			strcpy(relay_status, "pwdError");
		}
		else if (wan_status == WAN_CONNECTED) //获取到ip才表示桥接成功
		{
			strcpy(relay_status, "bridgeSuccess");
		}
		else
		{
			strcpy(relay_status, "disconnect");
		}

		cJSON_AddStringToObject(root, LIB_WIFIRELAY_CONNECT_STATUS, relay_status);
		cJSON_AddStringToObject(root, LIB_CONNECT_STATE, relay_status);//A9
		if(repeater_2g_enable == 1)
		{	
			cJSON_AddStringToObject(root, LIB_WIFIRELAY_CHK_HZ, "24G");
		}else
		{
			cJSON_AddStringToObject(root, LIB_WIFIRELAY_CHK_HZ, "5G");
		}
	}
	else
	{
		cJSON_AddStringToObject(root, LIB_WIFIRELAY_SSID, "");
		cJSON_AddStringToObject(root, LIB_WIFIRELAY_CHANNEL, "0");
		cJSON_AddStringToObject(root, LIB_WIFIRELAY_SECURITY_TYPE, "none");
		cJSON_AddStringToObject(root, LIB_WIFIRELAY_SECURITY_PASS, "");
		cJSON_AddStringToObject(root, LIB_WIFIRELAYMAC, "");
		cJSON_AddStringToObject(root, LIB_WIFIRELAY_CHK_HZ, "");
		cJSON_AddNumberToObject(root, LIB_WIFIRELAY_CONNECT_STATUS, WAN_DISCONNECTED);
	}
	
    return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_get_connect_duration
 功能描述  : 连接时间，A9页面新增的
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : luorilin
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_connect_duration(webs_t wp, cJSON *root, void *info)
{
	char temp[64] = {0};
	char *relay_status = NULL;
	
	//连接时间，A9页面新增
	WIFI_BRIDGE_INFO_STRUCT bridge_info;
	gpi_wifi_get_bridge_rssi(WLAN_RATE_24G,&bridge_info);

	relay_status = cgi_lib_get_var(wp,root, T(LIB_WIFIRELAY_CONNECT_STATUS), T(""));
	
	memset(&temp, 0x0, sizeof(temp));
	snprintf(temp , sizeof(temp) , "%lld" , (RELAY_SUC == relay_status ? bridge_info.link_time : 0));
	
	cJSON_AddStringToObject(root, LIB_CONNECT_DURATION, temp);//A9 页面新增 连接函数

	return RET_SUC;
}


void set_lanwan_ifname(SYS_WORK_MODE mode,char* ifname)
{
	switch (mode)
	{
		case WL_ROUTE_MODE:
			printf("%s ap_router mode!\n", __func__);
	
			nvram_set(LAN_IFNAMES, "eth0 wlan0 wlan1");
			nvram_set(WAN0_IFNAME, ifname);
			nvram_set(WAN_WHOLE_IFNAME, ifname);
			nvram_set(WAN0_IFNAMES, ifname);
			nvram_set("wan_ifname", ifname);
			break;

		case WL_WISP_MODE:
			printf("%s wisp mode!\n", __func__);
			//set lan ifname
			nvram_set(LAN_IFNAMES, "eth0 wlan0 wlan1");
			//set wan ifname
			nvram_set(WAN0_IFNAME, ifname);
			nvram_set(WAN_WHOLE_IFNAME, ifname);
			nvram_set(WAN0_IFNAMES, ifname);
			nvram_set("wan_ifname", ifname);
			break;

		case WL_APCLIENT_MODE:
			printf("%s ap_client mode!\n", __func__);
			//set lan ifname
			nvram_set(LAN_IFNAMES, "eth0 wlan0 wlan1 wlan0-vxd0 wlan1-vxd0");
			//set wan ifname
			nvram_set(WAN0_IFNAME, ifname);
			nvram_set(WAN_WHOLE_IFNAME, ifname);
			nvram_set(WAN0_IFNAMES, ifname);
			nvram_set("wan_ifname", ifname);
			break;
		case WL_BRIDGEAP_MODE:
			printf("%s ap_bridge mode!\n", __func__);
			nvram_set(LAN_IFNAMES, "eth1 eth0 wlan0 wlan1");
			nvram_set(WAN0_IFNAME, ifname);
			nvram_set(WAN_WHOLE_IFNAME, ifname);
			nvram_set(WAN0_IFNAMES, ifname);
			nvram_set("wan_ifname", ifname);
			break;
		case WL_WDS_MODE:
			printf("%s wds mode!\n", __func__);
			break;

		default:
			printf("%s unknown mode!\n", __func__);
	}
}
/*****************************************************************************
 函 数 名  : config_repeater_interface
 功能描述  : 配置桥接接口，仅仅使用本文件
 输入参数  : webs_t wp  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年10月25日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static void config_repeater_interface(webs_t wp,char *chkhz)
{
	char *ssid = NULL;
	char *encode = NULL;
	char *channel = NULL;
	char *security = NULL;
	char *pass_phrase_str = NULL;
	ssid = websGetVar(wp, T("wifiRelaySSID"), T(""));
	encode = websGetVar(wp, T("wifiRelayEncode"), T("UTF-8"));
	channel = websGetVar(wp, T("wifiRelayChannel"), T("0"));
	security = websGetVar(wp, T("wifiRelaySecurityMode"), T("NONE"));
	pass_phrase_str = websGetVar(wp, T("wifiRelayPwd"), T(""));	

	if(0 == strcmp(chkhz,"24G"))
	{
		nvram_set(WLAN24G_REPEATER_SSID,ssid);
		nvram_set(WLAN24G_REPEATER_ENCODE,encode);
		nvram_set(WLAN24G_REPEATER_ENABLE,"1");
		nvram_set(WLAN24G_WORK_MODE, "sta");
		config_wlan_security(TENDA_WLAN24_REPEATER_IFNAME,security,pass_phrase_str);
	}
	else
	{
		nvram_set(WLAN5G_REPEATER_SSID,ssid);
		nvram_set(WLAN5G_REPEATER_ENCODE,encode);
		nvram_set(WLAN5G_REPEATER_ENABLE,"1");
		nvram_set(WLAN5G_WORK_MODE, "sta");	
		config_wlan_security(TENDA_WLAN5_REPEATER_IFNAME,security,pass_phrase_str);
	}
}
/*****************************************************************************
 函 数 名  : bridge_to_route
 wisp_or_apclient_to_route
 other_to_wisp
 other_to_apclient
 route_to_bridge
 other_to_bridge
 功能描述  : 配置桥接切换的一系列函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2017年10月24日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static inline void bridge_to_route()
{
	nvram_set(SYSCONFIG_WORKMODE,"route");
	set_lanwan_ifname(WL_ROUTE_MODE,"eth1");
	nvram_set(LAN_PROTO, "dhcp");
}

static inline void wisp_or_apclient_to_route()
{
	nvram_set(WAN0_CHECK, "0");		
	nvram_set(SYSCONFIG_WORKMODE,"route");
	set_lanwan_ifname(WL_ROUTE_MODE,"eth1");
	nvram_set(LAN_PROTO, "dhcp");
}

static inline void other_to_wisp(webs_t wp,char*chkhz)
{
	nvram_set(SYSCONFIG_WORKMODE,"wisp");
	nvram_set(WAN0_PROTO, "dhcp");
	nvram_set(WAN0_CHECK, "0");
	nvram_set(WLAN_PUBLIC_SCHEDULE_ENABLE, "0");//无线扩展模式，强制关闭无线定时开关功能
	nvram_set(WPS_MODE_ENABLE, "disabled");
	if(!strcmp(chkhz,"24G"))
	{
		set_lanwan_ifname(WL_WISP_MODE,TENDA_WLAN24_REPEATER_IFNAME);
	}
	else
	{
		set_lanwan_ifname(WL_WISP_MODE,TENDA_WLAN5_REPEATER_IFNAME);
	}
	nvram_set(LAN_PROTO, "dhcp");
	//代码修改的时候，需要参考
	config_repeater_interface(wp,chkhz);
}

static inline void other_to_apclient(webs_t wp,char*chkhz)
{
	nvram_set(SYSCONFIG_WORKMODE,"client+ap");
	nvram_set(WAN0_PROTO, "dhcp");
	nvram_set(WAN0_CHECK, "0");
	nvram_set(WLAN_PUBLIC_SCHEDULE_ENABLE, "0");//无线扩展模式，强制关闭无线定时开关功能
	nvram_set(WPS_MODE_ENABLE, "disabled");
	set_lanwan_ifname(WL_APCLIENT_MODE,"eth1");
	nvram_set(LAN_PROTO, "dhcp");
	//代码修改的时候，需要参考
	config_repeater_interface(wp,chkhz);
}

static inline void route_to_bridge()
{
	nvram_set(SYSCONFIG_WORKMODE,"bridge");
	set_lanwan_ifname(WL_BRIDGEAP_MODE,"eth0");
	nvram_set(LAN_PROTO, "dhcp");
}

static inline void other_to_bridge()
{
	nvram_set(WAN0_CHECK, "0");
	nvram_set(SYSCONFIG_WORKMODE,"bridge");
	nvram_set(WLAN24G_WORK_MODE, "ap");
	nvram_set(WLAN5G_WORK_MODE, "ap");
	set_lanwan_ifname(WL_BRIDGEAP_MODE,"eth0");
	nvram_set(LAN_PROTO, "dhcp");
}
/*****************************************************************************
 函 数 名  : cgi_lib_set_wan_access
 功能描述  : 设置wan口的接入方式
 输入参数  : webs_t wp            
             cJSON *root          
             CGI_MSG_MODULE *msg  
             char *err_code       
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wifiRelay(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	char *chkhz = NULL;
	char *extra_mode = NULL;
	char *super_mac = NULL;
	char	*old_mode = NULL;
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	
	
	extra_mode = cgi_lib_get_var(wp,root, T(LIB_WIFI_RELAY_TYPE), T("")); //1wisp 模式还是2apclient模式	

	old_mode = nvram_safe_get(SYSCONFIG_WORKMODE);

	chkhz = websGetVar(wp, T("wifiRelayChkHz"), T("24G"));

	printf("====extra_mode:%s======old_mode:%s=====%s [%d]\n",extra_mode,old_mode, __FUNCTION__, __LINE__);

	//如果之前是路由模式，当前需要切换的还是路由模式
	if ( strcmp("route", old_mode) == 0
		&& strcmp("disabled", extra_mode) == 0)
	{
		if (!err_code[0])
		{
			strcpy(err_code, "0");
		}
	}
	//如果之前是桥模式，当前还是需要切换到桥模式
	else if (strcmp("bridge", old_mode) == 0
		&& strcmp("ap", extra_mode) == 0)
	{
		if (!err_code[0])
		{
			strcpy(err_code, "0");
		}
	}
	else
	{	
		//桥接接口先设成关闭,将接口都设置成ap
		//，只有在切换到apclient和wisp的时候才需要修改桥接接口的状态
		nvram_set(WLAN24G_REPEATER_ENABLE,"0");
		nvram_set(WLAN5G_REPEATER_ENABLE,"0");
		nvram_set(WLAN24G_WORK_MODE,"ap");
		nvram_set(WLAN5G_WORK_MODE,"ap");
		//切换到路由模式
		if (!strcmp("disabled", extra_mode))
		{
			//之前是桥模式

			if (WL_BRIDGEAP_MODE == old_mode)
			{
				bridge_to_route();
			}
			//之前是wisp或者ap+client模式
			else
			{
				wisp_or_apclient_to_route();
			}
		}
		//切换到wisp模式
		else if (!strcmp("wisp", extra_mode))
		{
			other_to_wisp(wp,chkhz);
		}
		//切换到client+ap模式
		else if (!strcmp("client+ap", extra_mode))
		{
			other_to_apclient(wp,chkhz);
		}
		//切换到ap模式
		else if (!strcmp("ap", extra_mode))
		{
			//之前是路由模式
			if (WL_ROUTE_MODE == old_mode)
			{
				route_to_bridge();
			}
			//之前是wisp或者ap+client模式
			else
			{
				other_to_bridge();
			}
		}

		/* 将消息加入消息列表 */
		CGI_MSG_MODULE msg_tmp;
		msg_tmp.id = RC_SYSTOOLS_MODULE;
		sprintf(msg_tmp.msg, "%s", "string_info=reboot");
		add_msg_to_list(msg, &msg_tmp);

		if (!err_code[0])
		{
			strcpy(err_code, "100");
		}
	}

	return RET_SUC;
}

RET_INFO cgi_lib_get_wifiScanresault(webs_t wp, cJSON *root, void *info)
{
	cgi_wifi_scanresult_get(root);
	return RET_SUC;
}





