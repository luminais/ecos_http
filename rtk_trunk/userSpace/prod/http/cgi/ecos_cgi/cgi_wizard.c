/***********************************************************
 * Copyright (C), 1998-2016, Tenda Tech. Co., Ltd.
 *
 * FileName : cgi_wizard.c
 * Description : 快速设置页面部分特有功能
 * Author : fh
 * Version : V1.0
 * Date :2016-07-21
 *
 * Function List:
 * 1.
 * 2.
 * Others :
 *
 *
 * History :
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +
 +
 *******************************************************************/



#include "webs.h"
#include "wan.h"
#include <dhcp_server.h>
#include <tenda_arp.h>
#include "cgi_common.h"

//快速设置页面时区设置
#define TIME_ZONES_NUMBER 75
#include "flash_cgi.h"
#include <tz_info.h>
#ifdef __CONFIG_INDICATE_LED__
typedef enum{
	INDICATE_LED_OFF = 0,
	INDICATE_LED_EXIT
}LED_OFF_TYPE;
extern int  set_indicate_led_off(int type);
#endif
extern int dns_redirect_disable;//定于与 ./userSpace/cbb/src/dnsmasq/src/dnsmasq_proto.c

RET_INFO cgi_wizardCheck_get (webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	cJSON_AddItemToObject(root, "wizardCheck", obj = cJSON_CreateObject());

	char result[2048] = {0};
	char wan_detection[32] = {0};     //detecting|disabled|pppoe|dhcp|static
	char value[128] = {0};
	P_WAN_ERR_INFO_STRUCT wan_err_info = NULL;
	P_DHCP_SERVER_INFO_STRUCT dhcp_lan_info = NULL;
	P_WAN_INFO_STRUCT wan_common_info = NULL;

	wan_err_info = gpi_wan_get_err_info_other();
	wan_common_info = gpi_wan_get_info();
	dhcp_lan_info = gpi_dhcp_server_info();

	//以下函数可能无用，如果与UI联调没有问题可删除

	//Wan口检测
	if (COMMON_NO_WIRE == wan_err_info->code)
	{
		sprintf(wan_detection, "disabled" );
	}
	else
	{
		if (WAN_STATIC_MODE == wan_err_info->network_check)
		{
			sprintf(wan_detection, "static");
		}
		else if (WAN_DHCP_MODE == wan_err_info->network_check)
		{
			sprintf(wan_detection, "dhcp");
		}
		else if (WAN_PPPOE_MODE == wan_err_info->network_check)
		{
			sprintf(wan_detection, "pppoe");
		}
		else if (WAN_MAX_MODE == wan_err_info->network_check)
		{
			sprintf(wan_detection, "detecting");
		}
	}

	cJSON_AddStringToObject(obj, "wizardWANDetection", wan_detection);

	//当前wan lan部分参数
	cJSON_AddStringToObject(obj, "wanType", wan_common_info->wan_proto);
	cJSON_AddStringToObject(obj, "lanIP", dhcp_lan_info->lan_ip);
	cJSON_AddStringToObject(obj, "lanMask", dhcp_lan_info->lan_mask);

	//联网检测
	int internetStat = get_wan_onln_connstatus();
	memset(value, 0, sizeof(value));
	strncat(result, ",", 1);

	if (2 == internetStat)
	{
		sprintf(value, "%s", "true");
	}
	else
	{
		sprintf(value, "%s", "false");
	}

	cJSON_AddStringToObject(obj, "connectInternet", value);

	//是否无线客户端
	if(wp != NULL)
	{
		cJSON_AddStringToObject(obj, "iswirelessclient", tenda_arp_ip_to_flag(inet_addr(wp->ipaddr)) ? "true" : "false");
	}
	else
	{
		cJSON_AddStringToObject(obj, "iswirelessclient",  "true");
	}
	return RET_SUC;
}



RET_INFO cgi_timeZone_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	int	i_timezone;
	char *time_zone = websGetVar(wp, T("timeZone"), T(""));
	char value_timezone[15] = {0};
	int i = 0 ;

	if (time_zone[0] != 0)
	{
		float ftime_zone;
		ftime_zone = atof(time_zone);
		i_timezone = ftime_zone * 3600;

		for (i = 0; i < TIME_ZONES_NUMBER; i++ )
		{
			if (time_zones[i].tz_offset == i_timezone )
			{
				sprintf(value_timezone, "%d", time_zones[i].index );
				_SET_VALUE(FUNCTION_TIME_ZONE, value_timezone);
				break;
			}
		}
	}

	return (RET_SUC);
}

RET_INFO cgi_wizard_set_succeed(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
	unsigned int type = 0;
#ifdef __CONFIG_CHINA_NET_CLIENT__ 
	if (1 == nvram_match(SYSCONFIG_QUICK_SET_ENABLE, "1"))
	{
		/*lq 添加自动克隆MAC，在引导界面进行配置备份*/
		unsigned char *p_host_mac = tenda_arp_ip_to_mac(inet_addr(wp->ipaddr));
		unsigned char host_mac[18] = {0};

		if (p_host_mac)
		{
			snprintf(host_mac, 18, "%s", inet_mactoa(p_host_mac));
		}
		else
		{
			snprintf(host_mac, 18, "%s", "00:00:00:00:00:00");
		}

		_SET_VALUE("wan0_hwaddr_bak", host_mac);

		type = tenda_arp_is_wireless_client(p_host_mac);
		if (type == 1 || type == 2)
		{
			_SET_VALUE("wan0_macclone_mode_bak", "manual");
		}
		else
		{
			_SET_VALUE("wan0_macclone_mode_bak", "clone");
		}

		_SET_VALUE("restore_pppoe_first", "1");
		/*lq   end*/
	}
#endif
	//add by z10312 dns重定向只在快速设置前生效
	dns_redirect_disable = 1;
	_SET_VALUE("mode_need_switch", "no");//标志着联网类型检测结束或提前结束
	_SET_VALUE(SYSCONFIG_QUICK_SET_ENABLE, "0"); //标志着快速设置结束
#ifdef __CONFIG_PPPOE_SERVER__
	set_synchro_type(MANUAL_SYNCHRO);
#endif
#ifdef __CONFIG_INDICATE_LED__
	set_indicate_led_off(INDICATE_LED_EXIT);
#endif

	return RET_SUC;		//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
}


