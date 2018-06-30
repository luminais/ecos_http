/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_lib_wifi.c
  版 本 号   : 初稿
  作    者   : fh
  生成日期   : 2016年12月13日
  最近修改   :
  功能描述   :

  功能描述   : wifi最小功能单元的get和set库

  修改历史   :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "cgi_lib.h"
#include <wifi.h>
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern int gWifiStatusConfig;//用于无线定时开关

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern void changeWeek(char *in_week, char *out_week);
extern void send_wifi_msg_handle(char *ifname,int enable,CGI_MSG_MODULE *msg);
/*****************************************************************************
 函 数 名  : cgi_lib_get_isWifiClient
 功能描述  : 获取是否是无线客户端
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
RET_INFO cgi_lib_get_isWifiClient(webs_t wp, cJSON *root, void *info)
{	
	char *temp = NULL;
	if(wp != NULL)
	{
		temp = tenda_arp_ip_to_flag(inet_addr(wp->ipaddr))?"true":"false";
	}
	else
	{
		temp = "true";
	}
	//是否是无线客户端
	cJSON_AddStringToObject(root,LIB_ISWIFICLIENT,temp);
	
	return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_enable
 功能描述  : 获取无线开关
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  : RET_INFO

 修改历史      :
  1.日    期   : 2017年10月23日
    作    者   : lrl
    修改内容   : 添加适配5G

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_enable(webs_t wp, cJSON *root, void *info)
{
	PI8 *wl_enable = "true";
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;

	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);

	wl_enable = wifi_info->wl_radio ? "true" : "false";
	cJSON_AddStringToObject(root, LIB_WIFI_EN, wl_enable);

	wl_enable = wifi_info_5g->wl_radio ? "true" : "false";
	cJSON_AddStringToObject(root, LIB_WIFI_EN_5G, wl_enable);
	
	return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_basic
 功能描述  : 获取无线基本参数
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史  :
 1.日    期: 2017年10月23日
   作    者: lrl
   修改内容: 添加适配5G

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_basic(webs_t wp, cJSON *root, void *info)
{
	PI8 *wl_enable = "true";
	PI8 *hide_ssid_enable = "false";
	PI8 security_mode[PI_BUFLEN_32] = {0};
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	
	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);

	wl_enable = wifi_info->wl_radio ? "true" : "false";
	cJSON_AddStringToObject(root, LIB_WIFI_EN, wl_enable);

	wl_enable = wifi_info_5g->wl_radio ? "true" : "false";
	cJSON_AddStringToObject(root, LIB_WIFI_EN_5G, wl_enable);
	
	//2.4GwifiHideSSID
	hide_ssid_enable = wifi_info->ap_ssid_cfg.ssid_hide ? "true" : "false";
	cJSON_AddStringToObject(root, LIB_WIFI_HIDE_SSID, hide_ssid_enable);
	
	//5GwifiHideSSID
    hide_ssid_enable = wifi_info_5g->ap_ssid_cfg.ssid_hide ? "true" : "false";
	cJSON_AddStringToObject(root, LIB_WIFI_HIDE_SSID_5G, hide_ssid_enable);
	
	//2.4GwifiSSID
	cJSON_AddStringToObject(root, LIB_WIFI_AP_SSID,  wifi_info->ap_ssid_cfg.ssid );

	//5GwifiSSID
	cJSON_AddStringToObject(root, LIB_WIFI_AP_SSID_5G,  wifi_info_5g->ap_ssid_cfg.ssid );

	//2.4G wifiSecurityMode
	//加密模式页面下发的格式需要与桥接页面下发的加密模式保持一致,如:WPA/AES
	get_wlan_security_mode(&(wifi_info->ap_ssid_cfg),security_mode);
	cJSON_AddStringToObject(root, LIB_WIFI_AP_SEC_MODE, security_mode);
	
	//5G wifiSecurityMode
	memset(security_mode,0x0,sizeof(security_mode));
	get_wlan_security_mode(&(wifi_info_5g->ap_ssid_cfg),security_mode);
	cJSON_AddStringToObject(root, LIB_WIFI_AP_SEC_MODE_5G, security_mode);
	
	//2.4GwifiPwd
	cJSON_AddStringToObject(root, LIB_WIFI_AP_PWD,  wifi_info->ap_ssid_cfg.wpa_psk_key );
	cJSON_AddStringToObject(root, LIB_WIFI_AP_NO_PWD, strcmp(wifi_info->ap_ssid_cfg.security, "none") ? "false" : "true");

	//5GGwifiPwd
	cJSON_AddStringToObject(root, LIB_WIFI_AP_PWD_5G,  wifi_info_5g->ap_ssid_cfg.wpa_psk_key );
	cJSON_AddStringToObject(root, LIB_WIFI_AP_NO_PWD_5G, strcmp(wifi_info_5g->ap_ssid_cfg.security, "none") ? "false" : "true");

#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
	//双频合一开关
	PI8 *doubleband_enable = "false";
	PI8 *wifi_total_enable = "true";
	doubleband_enable = strcmp(nvram_safe_get(WLAN_PUBLIC_DOUBLEBANDUN_ENBALE),"1") ? "false" : "true";
	cJSON_AddStringToObject(root,LIB_WIFI_HAS_DOUBLEBAND_UNITY,"true");
	cJSON_AddStringToObject(root,LIB_WIFI_DOUBLEBAND_UNITY_ENABLE,doubleband_enable);

	//无线总开关
	wifi_total_enable = (wifi_info->wl_radio | wifi_info_5g->wl_radio) ? "true" : "false";
	cJSON_AddStringToObject(root,LIB_WIFI_TOTAL_ENABLE,wifi_total_enable);
#else
	cJSON_AddStringToObject(root,LIB_WIFI_HAS_DOUBLEBAND_UNITY,"false");
#endif

	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_power
 功能描述  : 获取无线功率信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年10月23日
    作    者   : lrl
    修改内容   : 添加适配5G

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_power(webs_t wp, cJSON *root, void *info)
{
	int power_num = 0;
	char power[3][8] = {"100", "100", "100"};
	char value[PI_BUFLEN_128] = {0};
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	
	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);

	//2.4GwifiPower
	cJSON_AddStringToObject(root, LIB_WIFI_POWER, wifi_info->power);
	//5GwifiPower
	cJSON_AddStringToObject(root, LIB_WIFI_POWER_5G, wifi_info_5g->power);
	
	//2.4Gwifi support power (78,92,100)
	power_num = sscanf(wifi_info->power_percent, "%[^,],%[^,],%s", power[0], power[1], power[2]);

	if (power_num == 1)
	{
		sprintf(value, "%s", SURPOT_LOW_POWER);
	}
	else if (power_num == 2)
	{
		sprintf(value, "%s", SURPOT_NORMAL_POWER);
	}
	else
	{
		sprintf(value, "%s", SURPOT_HIGH_POWER);
	}
	cJSON_AddStringToObject(root, LIB_WIFI_POWER_GEAR, value);
	
	//5Cwifi support power (78,92,100)
	memset(value , 0 , sizeof(value));
	power_num = sscanf(wifi_info_5g->power_percent, "%[^,],%[^,],%s", power[0], power[1], power[2]);

	if (power_num == 1)
	{
		sprintf(value, "%s", SURPOT_LOW_POWER);
	}
	else if (power_num == 2)
	{
		sprintf(value, "%s", SURPOT_NORMAL_POWER);
	}
	else
	{
		sprintf(value, "%s", SURPOT_HIGH_POWER);
	}
	cJSON_AddStringToObject(root, LIB_WIFI_POWER_GEAR_5G, value);
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_sched
 功能描述  : 获取无线定时开关信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_sched(webs_t wp, cJSON *root, void *info)
{
	PI8 wifi_sched_enable[PI_BUFLEN_16] = {0};
	PI8 wifi_sched_times[PI_BUFLEN_16] = {0};
	PI8 wifi_sched_weeks[PI_BUFLEN_16] = {0};

	gpi_wifi_switch_sched_web_info(wifi_sched_enable, wifi_sched_times, wifi_sched_weeks);

	//wifiTimeEn
	if (strcmp(wifi_sched_enable, "0") == 0)
	{
		cJSON_AddStringToObject(root, LIB_WIFI_TIME_EN, "false");
	}
	else
	{
		cJSON_AddStringToObject(root, LIB_WIFI_TIME_EN, "true");
	}

	//wifiTimeClose
	cJSON_AddStringToObject(root, LIB_WIFI_TIME_CLOSE, wifi_sched_times);
	//wifiTimeDate
	cJSON_AddStringToObject(root, LIB_WIFI_TIME_DATE, wifi_sched_weeks);
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_relay_type
 功能描述  : 获取无线中继类型
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年10月25日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_relay_type(webs_t wp, cJSON *root, void *info)
{
	char value[PI_BUFLEN_128] = {0};
	
	switch(gpi_wifi_get_mode())
	{
		case WL_ROUTE_MODE: strcpy(value, "disabled");
			break;
		case WL_WISP_MODE: strcpy(value, "wisp");
			break;
		case WL_APCLIENT_MODE: strcpy(value, "client+ap");
			break;
		case WL_BRIDGEAP_MODE: strcpy(value, "ap");
			break;
		default:
			strcpy(value, "disabled");
			break;
	}

	cJSON_AddStringToObject(root, LIB_WIFI_RELAY_TYPE, value);

	return RET_SUC;
}

//wifi抗干扰开关
#define WIFI_ANTINAM_EN_OFF "44"
#define WIFI_ANTINAM_EN_ON "0"


/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_adv_cfg
 功能描述  : 获取无线高级参数
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年10月23日
    作    者   : lrl
    修改内容   : 添加适配5G

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_adv_cfg(webs_t wp, cJSON *root, void *info)
{
	int i = 0;
	cJSON *array = NULL;
	char value[PI_BUFLEN_128] = {0};
	PIU16 channel_list[PI_BUFLEN_256] = {0};
	PI32 channel_count = 0;
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	WIFI_CURRCET_INFO_STRUCT wifi_curr_info;
	WIFI_CURRCET_INFO_STRUCT wifi_curr_info_5g;
	
	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);
	
	memset(&wifi_curr_info, 0x0, sizeof(WIFI_CURRCET_INFO_STRUCT));
	gpi_wifi_get_curret_info(WLAN_RATE_24G,&wifi_curr_info);
	
	memset(&wifi_curr_info_5g, 0x0, sizeof(WIFI_CURRCET_INFO_STRUCT));
	gpi_wifi_get_curret_info(WLAN_RATE_5G,&wifi_curr_info_5g);
   
	cJSON_AddStringToObject(root, LIB_WIFI_BGN_MODE, wifi_info->nettype);/*2.4G 网络模式*/
	
	/*根绝国家代码和频宽返回具体的信道列表*/
	memset(channel_list,0x0,sizeof(channel_list));
    channel_count = gpi_wifi_get_channels(WLAN_RATE_24G,20,wifi_info->coutry_code,channel_list,sizeof(channel_list));
	if(channel_count > 0)
	{
		cJSON_AddItemToObject(root,LIB_WIFI_CHANNEL_LIST,array = cJSON_CreateArray());
		assemble_channelList_cJsonArray(array,channel_list,channel_count);
	}
	
	cJSON_AddStringToObject(root, LIB_WIFI_CH, wifi_info->channel ? inttostr(value,wifi_info->channel,sizeof(value)) : "auto");/*2.4G 设置的信道*/
	cJSON_AddStringToObject(root, LIB_WIFI_C_CH, inttostr(value,wifi_curr_info.channel,sizeof(value)));/*2.4G 当前信道*/
	cJSON_AddStringToObject(root, LIB_WIFI_BW, !strcmp(wifi_info->bandwidth, "auto") ? "auto" : wifi_info->bandwidth); 
	cJSON_AddStringToObject(root, LIB_WIFI_C_BW, inttostr(value,wifi_curr_info.bandwidth,sizeof(value)));
	
	cJSON_AddStringToObject(root, LIB_WIFI_BGN_MODE_5G, wifi_info_5g->nettype); /*5G 网络模式*/
	//5G 信道列表
	cJSON *obj = NULL;
	cJSON_AddItemToObject(root,LIB_WIFI_CHANNEL_LIST_5G,obj=cJSON_CreateObject());
	//20MHZ
	memset(channel_list,0x0,sizeof(channel_list));
    channel_count = gpi_wifi_get_channels(WLAN_RATE_5G,20,wifi_info->coutry_code,channel_list,sizeof(channel_list));
	cJSON_AddItemToObject(obj,"20",array = cJSON_CreateArray());
	assemble_channelList_cJsonArray(array,channel_list,channel_count);
	//40MHZ
	memset(channel_list,0x0,sizeof(channel_list));
    channel_count = gpi_wifi_get_channels(WLAN_RATE_5G,40,wifi_info->coutry_code,channel_list,sizeof(channel_list));
	cJSON_AddItemToObject(obj,"40",array = cJSON_CreateArray());
	assemble_channelList_cJsonArray(array,channel_list,channel_count);
	//80MHZ
	memset(channel_list,0x0,sizeof(channel_list));
    channel_count = gpi_wifi_get_channels(WLAN_RATE_5G,80,wifi_info->coutry_code,channel_list,sizeof(channel_list));
    cJSON_AddItemToObject(obj,"80",array = cJSON_CreateArray());
    assemble_channelList_cJsonArray(array,channel_list,channel_count);
	
	cJSON_AddStringToObject(root, LIB_WIFI_CH_5G, wifi_info_5g->channel ? inttostr(value,wifi_info_5g->channel,sizeof(value)) : "auto");/*5G 设置的信道*/
	cJSON_AddStringToObject(root, LIB_WIFI_C_CH_5G, inttostr(value,wifi_curr_info_5g.channel,sizeof(value)));/*5G当前信道*/
	cJSON_AddStringToObject(root, LIB_WIFI_BW_5G, !strcmp(wifi_info_5g->bandwidth, "auto") ? "auto" : wifi_info_5g->bandwidth);
	cJSON_AddStringToObject(root, LIB_WIFI_C_BW_5G, inttostr(value,wifi_curr_info_5g.bandwidth,sizeof(value)));
	
#ifdef __CONFIG_DISTURB_EN__
	cJSON_AddStringToObject(root, LIB_HAS_WIFI_ANTIJAM, "true");/*页面根据此参数是否显示抗干扰功能*/ 
	cJSON_AddStringToObject(root,LIB_WIFI_ANTIJAM_EN,!strcmp(wifi_info->antijam_en,WIFI_ANTINAM_EN_ON)?"true":"false");
#else
	cJSON_AddStringToObject(root, LIB_HAS_WIFI_ANTIJAM, "false");
#endif 

	return RET_SUC;
}

extern void wl_channel_scan_app();
extern unsigned int chnl_score_app[MAX_CHANNEL_NUM][2];
/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_channel_grade
 功能描述  : 获取当前信道优劣等级，20代表最高，10代表最低
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年1月25日
    作    者   : lrl
    修改内容   : 修改逻辑

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_channel_grade(webs_t wp, cJSON *root, void *info)
{
	unsigned int current_channel = 0;
	int channel_grade = 0;
	WIFI_CURRCET_INFO_STRUCT wifi_curr_info;
	
	memset(&wifi_curr_info, 0x0, sizeof(WIFI_CURRCET_INFO_STRUCT));
	gpi_wifi_get_curret_info(WLAN_RATE_24G,&wifi_curr_info);

	current_channel = wifi_curr_info.channel;
	memset(chnl_score_app[0],0x0,sizeof(chnl_score_app));
	wl_channel_scan_app();//开始扫描，使chnl_score_app的值生效

  	int i=0;
	//目前需求只评估2.4G的
	for(i = 0; i < 13; i++)
	{
		printf("channel:%d,score:%d\n",chnl_score_app[i][0],chnl_score_app[i][1]);
		if(chnl_score_app[i][0] == current_channel)
			channel_grade = chnl_score_app[i][1];
	}
	
	/*获取当前信道评分等级，20分最高，10分最低*/
	//channel_grade = chnl_score_app[current_channel-1];
	if(-1 == channel_grade)
		channel_grade = 20;
	else
		channel_grade = channel_grade + 10;
	printf("current_channel:%d,score:%d\n",current_channel,channel_grade);

	cJSON_AddNumberToObject(root, LIB_CHANNEL_2G_GRADE, channel_grade);

	return RET_SUC;
}

int app_msg_op_code_2g = COMMON_MSG_MAX; //app设置发消息用
int app_msg_op_code_5g = COMMON_MSG_MAX; //app设置发消息用

/*****************************************************************************
 函 数 名  : cgi_lib_set_wifi_basic
 功能描述  : 设置无线基本参数
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :
 修改历史  :
1.日    期 : 2017年10月23日
  作    者 : lrl
  修改内容 : 添加适配5G
*****************************************************************************/
RET_INFO cgi_lib_set_wifi_basic(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	char value[PI_BUFLEN_256] = {0};
	int wl5g_restart =0;
	int wl2g_restart =0;
	PI8 *wirelessenable_2g = NULL;
	PI8 *wirelessenable_5g = NULL;
	PI8 *ssid_2g = NULL;
	PI8 *ssid_5g = NULL;
	PI8 *hide_ssid_2g = NULL;
	PI8 *hide_ssid_5g = NULL;
	PI8 *security_2g = NULL;
	PI8 *security_5g = NULL;
	PI8 *password_2g = NULL;
	PI8 *password_5g = NULL;
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	
	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);

	/*获取web下发的参数*/
	wirelessenable_2g = strcmp(cgi_lib_get_var(wp, root, T(LIB_WIFI_EN), T("true")), "true") ? "0" : "1";
	hide_ssid_2g = strcmp(cgi_lib_get_var(wp, root, T(LIB_WIFI_HIDE_SSID), T("false")),"true") ? "0" : "1";
	ssid_2g = cgi_lib_get_var(wp, root, T(LIB_WIFI_AP_SSID), T(""));
	security_2g = cgi_lib_get_var(wp, root, T(LIB_WIFI_AP_SEC_MODE), T("NONE"));
	password_2g = cgi_lib_get_var(wp, root, T(LIB_WIFI_AP_PWD), T(""));	

	wirelessenable_5g = strcmp(cgi_lib_get_var(wp, root, T(LIB_WIFI_EN_5G), T("true")), "true") ? "0" : "1";
	hide_ssid_5g = strcmp(cgi_lib_get_var(wp, root, T(LIB_WIFI_HIDE_SSID_5G), T("false")),"true") ? "0" : "1";
	ssid_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_AP_SSID_5G), T(""));
	security_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_AP_SEC_MODE_5G), T("NONE"));
	password_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_AP_PWD_5G), T(""));
		
	/*在快速设置时，规格规定需要将ssid修改为tenda_mac(eth0mac 后6位)*/
	if(nvram_match(SYSCONFIG_QUICK_SET_ENABLE,"1")) 
	{
		if(0 != strcmp(ssid_2g,""))
		{
			_SET_VALUE(WLAN24G_SSID, ssid_2g);

			memset(value,0x0,sizeof(value));
			sprintf(value,"%s_5G",ssid_2g);
			
#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
			//规格：开启双频优选功能，需要将2.4G和5G ssid设置为一样
			if(1 ==  atoi(nvram_safe_get(WLAN_PUBLIC_DOUBLEBANDUN_ENBALE)))
			{
				memset(value,0x0,sizeof(value));
				sprintf(value,"%s",ssid_2g);
			}
#endif	
			_SET_VALUE(WLAN5G_SSID, value);	  
		}

		if(0 != strcmp(password_2g,""))
		{	
			config_wlan_security(TENDA_WLAN24_AP_IFNAME,"WPAWPA2/AES",password_2g);
			config_wlan_security(TENDA_WLAN5_AP_IFNAME,"WPAWPA2/AES",password_2g);
		}
		
		send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,OP_RESTART,msg);
		send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,OP_RESTART,msg);	

		sprintf(err_code, "%s", "0");
		
		return RET_SUC;
	}
	
#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
	//双频合一
	int wl_doubleBandUn_en_change = 0;
	PI8 *wl_doubleBandUn_en = NULL;
	PI8 *value_en = NULL;
	
	wl_doubleBandUn_en = nvram_safe_get(WLAN_PUBLIC_DOUBLEBANDUN_ENBALE);
	value_en = cgi_lib_get_var(wp, root, T(LIB_WIFI_DOUBLEBAND_UNITY_ENABLE), T(""));
	if((NULL == value_en) ||( 0 == strcmp(value_en,"")))
		value_en = wl_doubleBandUn_en;
	else
		value_en = strcmp(value_en,"true") ? "0" : "1";
			
	if((1 == atoi(value_en)) 
		&& (0 == strcmp(ssid_2g,ssid_5g)) 
		&& (0 == strcmp(hide_ssid_2g,hide_ssid_5g))
		&& (0 == strcmp(security_2g,security_5g))
		&& (0 == strcmp(password_2g,password_5g)))
	{
		value_en = "1";
	}
	else
	{
		value_en = "0";
	}
	
	_SET_VALUE(WLAN_PUBLIC_DOUBLEBANDUN_ENBALE, value_en);
#endif

	//开启并设置无线
	if(1 == atoi(wirelessenable_2g))
	{
		//2.4G enable
		if(wifi_info->wl_radio != atoi(wirelessenable_2g))
		{
			wl2g_restart = 1;
			_SET_VALUE(WLAN24G_ENABLE, wirelessenable_2g);
		}

		//2.4G hide ssid		
		if (wifi_info->ap_ssid_cfg.ssid_hide != atoi(hide_ssid_2g))
		{
			wl2g_restart = 1;
			_SET_VALUE(WLAN24G_HIDE_SSID, hide_ssid_2g);
		}
		//2.4G ssid		
		if((0 != strcmp(wifi_info->ap_ssid_cfg.ssid,ssid_2g)) && (0 != strcmp(ssid_2g,"")))
		{	
			wl2g_restart = 1;
			_SET_VALUE(WLAN24G_SSID, ssid_2g);
		}
		//2.4G security web下发的格式 如:WAP/AES
		memset(value, 0x0, sizeof(value));
		get_wlan_security_mode(&(wifi_info->ap_ssid_cfg),value);
		if (0 == strcasecmp(value, security_2g))
		{
			if (0 != strcasecmp(security_2g, "NONE"))
			{
				/*加密方式相同且不为NONE,密码不相同*/
				if (0 != strcmp(wifi_info->ap_ssid_cfg.wpa_psk_key, password_2g))
				{
					wl2g_restart = 1;
					config_wlan_security(TENDA_WLAN24_AP_IFNAME,security_2g,password_2g);
				}
			}
		}
		else
		{
			wl2g_restart = 1;
			config_wlan_security(TENDA_WLAN24_AP_IFNAME,security_2g,password_2g);
		}

		//无线配置更改,需要重启无线
		if(1 == wl2g_restart)
		{
			app_msg_op_code_2g = OP_RESTART;
			send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,OP_RESTART,msg);	
		}
	}
	else
	{
		if(wifi_info->wl_radio != atoi(wirelessenable_2g))
		{
			_SET_VALUE(WLAN24G_ENABLE, wirelessenable_2g);
			app_msg_op_code_2g = OP_STOP;
			send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,OP_STOP,msg);	
		}
	}
		
	//开启并设置5G无线
	if(1 == atoi(wirelessenable_5g))
	{
		//5G enable
		if(wifi_info_5g->wl_radio != atoi(wirelessenable_5g))
		{
			wl5g_restart = 1;
			_SET_VALUE(WLAN5G_ENABLE, wirelessenable_5g);
		}
		//5G hide ssid		
		if (wifi_info_5g->ap_ssid_cfg.ssid_hide != atoi(hide_ssid_5g))
		{
			wl5g_restart = 1;
			_SET_VALUE(WLAN5G_HIDE_SSID, hide_ssid_5g);
		}
		//5G ssid		
		if((0 != strcmp(wifi_info_5g->ap_ssid_cfg.ssid,ssid_5g)) && (0 != strcmp(ssid_5g,"")))
		{
			wl5g_restart = 1;
			_SET_VALUE(WLAN5G_SSID, ssid_5g);  
		}
		//5G security security web下发的格式 如:WAP/AES
		memset(value, 0x0, sizeof(value));
		get_wlan_security_mode(&(wifi_info_5g->ap_ssid_cfg),value);
		if (0 == strcasecmp(value, security_5g))
		{
			if (0 != strcasecmp(security_5g, "NONE"))
			{
				if (0 != strcmp(wifi_info_5g->ap_ssid_cfg.wpa_psk_key, password_5g))
				{
					wl5g_restart = 1;
					config_wlan_security(TENDA_WLAN5_AP_IFNAME,security_5g,password_5g);
				}
			}
		}
		else
		{
			wl5g_restart = 1;
			config_wlan_security(TENDA_WLAN5_AP_IFNAME,security_5g,password_5g);
		}
		//无线配置更改，需要重启无线
#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
		/*双频优选开启生效需要重启无线*/
#else
		if(1 == wl5g_restart)
#endif
		{
			app_msg_op_code_5g = OP_RESTART;
			send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,OP_RESTART,msg);	
		}
	}
	else
	{
		if(wifi_info_5g->wl_radio != atoi(wirelessenable_5g))
		{
			_SET_VALUE(WLAN5G_ENABLE, wirelessenable_5g);
			//发消息关闭无线
			app_msg_op_code_5g = OP_STOP;
			send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,OP_STOP,msg);	
		}
	}
#ifdef __CONFIG_GUEST__
	if(0 == atoi(wirelessenable_2g) && 0 == atoi(wirelessenable_5g))
	{
		/*无线关闭，访客网络有效时间为：一直有效；需要同步关闭访客网络*/
		if(atoi(nvram_safe_get("wl_guest_effective_time")))
		{
			nvram_set(WLAN24G_GUEST_ENABLE,"0");
			nvram_set(WLAN5G_GUEST_ENABLE,"0");	
		}
	}
#endif
	
	sprintf(err_code, "%s", "0");

	return RET_SUC;

}

/*****************************************************************************
 函 数 名  : cgi_lib_set_wifi_power
 功能描述  : 无线功率设置
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史    :
1.日    期   : 2017年10月23日
  作    者   : lrl
  修改内容   : 添加适配5G
*****************************************************************************/
RET_INFO cgi_lib_set_wifi_power(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	PI8 *power = NULL;
	PI8 *power_5g = NULL;
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	PI8 power_default[PI_BUFLEN_16] = {0};
	PI8 country_default[PI_BUFLEN_16] = {0};
	
	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);
	
	
	power = cgi_lib_get_var(wp, root, T(LIB_WIFI_POWER), T(""));
	power_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_POWER_5G), T(""));

	/*2.4G功率设置*/
	if(1 == wifi_info->wl_radio)
	{
		if(0 != strcmp(power,"") && 0 != strcmp(wifi_info->power, power))
		{
			_SET_VALUE(WLAN24G_CURRENT_POWER, power);
			#ifdef __CONFIG_CE_POWER__
			if(0 == is_ce_power_limit())
			{
			app_msg_op_code_2g = OP_RESTART;
			send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,OP_RESTART,msg);	
			}
			#else
			app_msg_op_code_2g = OP_RESTART;
			send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,OP_RESTART,msg);
			#endif
		}
	}
	/*5G 功率设置*/
	if(1 == wifi_info_5g->wl_radio)
	{
		if (0 != strcmp(power_5g,"") && 0 != strcmp(wifi_info_5g->power, power_5g))
		{
			_SET_VALUE(WLAN5G_CURRENT_POWER, power_5g);
			#ifdef __CONFIG_CE_POWER__
			if(0 == is_ce_power_limit())
			{
				app_msg_op_code_5g = OP_RESTART;
				send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,OP_RESTART,msg);	
			}
			#else
			app_msg_op_code_5g = OP_RESTART;
			send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,OP_RESTART,msg);	
			#endif
		}
	}
	sprintf(err_code, "%s", "0");
	
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_wifi_sched
 功能描述  : 无线定时开关设置
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年12月14日
    作    者   : lrl
    修改内容   : 修改逻辑

*****************************************************************************/
RET_INFO cgi_lib_set_wifi_sched(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	PI8 *wlctl_enable = NULL; 
	PI8	*time_round = NULL; 
	PI8	*time_week = NULL; 
	PI8	*time_interval = NULL; 
	PI8	*sche_cnt = NULL;
	PI8 time[4][16] = {0};
	PI8 mib_name[PI_BUFLEN_64] = {0};
	int off_time, off_sec, off_min, off_hour;
	int on_time, on_sec, on_min, on_hour;
	PI8 week_new[16] = {0};
	PI8 time_str[64] = {0};
	PI8 wifi_sched_enable[PI_BUFLEN_16] = {0};
	PI8 wifi_sched_times[PI_BUFLEN_16] = {0};
	PI8	wifi_sched_weeks[PI_BUFLEN_16] = {0};
	
	CGI_MSG_MODULE msg_tmp;
	
	wlctl_enable = cgi_lib_get_var(wp, root, T(LIB_WIFI_TIME_EN), T("false"));
	time_round = cgi_lib_get_var(wp, root, T("time-round"), T("1111111")); //everyday defaultly
	time_interval = cgi_lib_get_var(wp, root, T(LIB_WIFI_TIME_CLOSE), T("00:00-00:00"));
	time_week = cgi_lib_get_var(wp, root, T(LIB_WIFI_TIME_DATE), T("10101010"));

	gpi_wifi_switch_sched_web_info(wifi_sched_enable, wifi_sched_times, wifi_sched_weeks);

	wlctl_enable = strcmp(wlctl_enable, "true") ? "0" : "1";

	if(0 == atoi(wlctl_enable) && 0 != strcmp(wifi_sched_enable, wlctl_enable))
	{
		//关闭无线定时
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_ENABLE, "0");
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_LIST_NUM, "0");
		
		memset(&msg_tmp,0x0,sizeof(CGI_MSG_MODULE));
		msg_tmp.id = RC_WIFI_SWITCH_SCHED_MODULE;
		sprintf(msg_tmp.msg, "op=%d", OP_RESTART);
		add_msg_to_list(msg, &msg_tmp);	
	}
			
	else if(1 == atoi(wlctl_enable) && 
			((0 != strcmp(wifi_sched_enable, wlctl_enable))
			|| (0 != strcmp(wifi_sched_times, time_interval))
			|| (0 != strcmp(wifi_sched_weeks, time_week))))
	{
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_WEEK, time_week);
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_TIME, time_interval);

		char week[16] = {0};
		char temp[16] = {0};
		int flag = 0;
		int i = 1;

		if (time_week[0] == '1')
		{
			strcpy(week, "1,2,3,4,5,6,7");		//~{Q!VPAK~}everyday
		}
		else
		{
			if (time_week[7] == '1')
			{
				memset(temp,0x0,sizeof(temp));
				sprintf(temp, "%s%d", week, 1);
				strcpy(week,temp);
				flag = 1;
			}

			for (i = 1; i <= 6; i++)
			{
				if (time_week[i] == '1')
				{
					if (flag == 0)
					{
						memset(temp,0x0,sizeof(temp));
						sprintf(temp, "%s%d", week, i + 1);
						strcpy(week,temp);
						flag = 1;
					}
					else
					{
						memset(temp,0x0,sizeof(temp));
						sprintf(temp, "%s,%d", week, i + 1);
						strcpy(week,temp);
					}
				}
			}
		}

		time_week = week ;
		sscanf(time_interval, "%[^:]:%[^-]-%[^:]:%s", time[0], time[1], time[2], time[3]);
		
		_GET_VALUE(WLAN_PUBLIC_SCHEDULE_LIST_NUM, sche_cnt);  /*支持设定几个WiFi定时时间段*/ 

		if (atoi(sche_cnt) == 0)
		{
			_SET_VALUE(WLAN_PUBLIC_SCHEDULE_LIST_NUM, "1");
		}

		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_ENABLE, "1");
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_DAY, time_round);
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_TIME, time_interval);

		if ((atoi(time[0])) * 60 + (atoi(time[1])) < (atoi(time[2])) * 60 + (atoi(time[3])))
		{
			off_hour = atoi(time[0]);
			off_min = atoi(time[1]);
			off_sec = 0;
			on_hour = atoi(time[2]);
			on_min = atoi(time[3]);
			on_sec = 0;
			
			//00:00-01:00
			//UTC+08:00 0 0 0 ? * 1,2,3,4,5,6,7 
			memset(time_str , 0 , sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour, time_week);
			snprintf(mib_name,sizeof(mib_name),WLAN_PUBLIC_SCHEDULE_OFFTIME_LIST,1);
			_SET_VALUE(mib_name, time_str);  
			
			//UTC+08:00 0 0 1 ? * 1,2,3,4,5,6,7
			memset(time_str , 0 , sizeof(time_str));
			memset(mib_name , 0 , sizeof(mib_name));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour, time_week);
			snprintf(mib_name,sizeof(mib_name),WLAN_PUBLIC_SCHEDULE_ONTIME_LIST,1);
			_SET_VALUE(mib_name, time_str); 
		}
		else		//7200,86400;0,3600
		{
			//sscanf(time_interval, "%[^,],%[^;];%[^,],%s", time[0], time[1], time[2], time[3]);
			off_hour = atoi(time[0]);
			off_min = atoi(time[1]);
			off_sec = 0;
			on_hour = atoi(time[2]);
			on_min =  atoi(time[3]);
			on_sec = 0;
			memset(time_str , 0 , sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour, time_week);
			snprintf(mib_name,sizeof(mib_name),WLAN_PUBLIC_SCHEDULE_OFFTIME_LIST,1);
			_SET_VALUE(mib_name, time_str);
			strcpy(week_new, time_week);
			changeWeek(time_week, week_new);
			memset(time_str , 0 , sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour, week_new);
			snprintf(mib_name,sizeof(mib_name),WLAN_PUBLIC_SCHEDULE_ONTIME_LIST,1);
			_SET_VALUE(mib_name, time_str);
		}

		gWifiStatusConfig = 0;
		memset(&msg_tmp,0x0,sizeof(CGI_MSG_MODULE));
		msg_tmp.id = RC_WIFI_SWITCH_SCHED_MODULE;
		sprintf(msg_tmp.msg, "op=%d", OP_RESTART);
		add_msg_to_list(msg, &msg_tmp);
	}
	
	sprintf(err_code, "%s", "0");
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_wifi_adv_cfg
 功能描述  : 无线高级参数设置
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

修改历史	:
1.日    期	: 2017年10月23日
  作    者	: lrl
  修改内容	: 添加适配5G
*****************************************************************************/
RET_INFO cgi_lib_set_wifi_adv_cfg(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	char value[PI_BUFLEN_256] = {0};
	static int wifi_band_upper[] = {40,48,136,153,161};
	static int wifi_band_lower[] = {36,44,132,140,149,157};
	static int wifi_band_none[] = {0,165};
	int wl5g_restart =0;
	int wl2g_restart =0;
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	
	PI8 *wifiMode = NULL, *wifiChannel = NULL, *wifiBandwidth = NULL;
	PI8 *wifiMode_5g = NULL, *wifiChannel_5g = NULL, *wifiBandwidth_5g = NULL;
	PI8 buf_temp[PI_BUFLEN_16] = {0};

	wifiMode = cgi_lib_get_var(wp, root, T(LIB_WIFI_BGN_MODE), "bgn");
	wifiChannel = cgi_lib_get_var(wp, root, T(LIB_WIFI_CH), T("0"));
	wifiBandwidth = cgi_lib_get_var(wp, root, T(LIB_WIFI_BW), "auto");
	wifiMode_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_BGN_MODE_5G), "ac");
	wifiChannel_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_CH_5G), T("0"));
	wifiBandwidth_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_BW_5G), "auto");

	if(!wifiChannel || !wifiBandwidth || !wifiMode_5g || !wifiChannel_5g || !wifiBandwidth_5g)
	{
		sprintf(err_code, "%s", "1");
		return;
	}

	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);

	//无线开启下 才能设置信道频宽模式
	if(1 == wifi_info->wl_radio)
	{
		wifiChannel = strcmp(wifiChannel,"auto") ? wifiChannel : "0";
		/*wifi channel*/
		sprintf(value, "%d", wifi_info->channel);	    
		if (0 != strcmp(value, wifiChannel))
		{
			wl2g_restart = 1;
			/*add wzs ,channel 10~13 sideband should be "upper".*/
			_SET_VALUE(WLAN24G_CHANNEL, wifiChannel);

			/*bandside*/
			if (atoi(wifiChannel) <= 5 && atoi(wifiChannel) >= 1) //Realtek中扩展信道改为5及其以下向上扩展
			{
				_SET_VALUE(WLAN24G_BANDSIDE, "lower");
			}
			else if (atoi(wifiChannel) <= 13 && atoi(wifiChannel) >= 8)
			{
				_SET_VALUE(WLAN24G_BANDSIDE, "upper");
			}
		}

		/*wifi bandwidth*/
		if(0 != strcmp(wifiBandwidth,wifi_info->bandwidth))
		{
			wl2g_restart = 1;
       		_SET_VALUE(WLAN24G_BANDWIDTH, wifiBandwidth);
		}
		/* wifi mode */
		if (0 != strcmp(wifiMode, wifi_info->nettype))
		{
			wl2g_restart = 1;
			_SET_VALUE(WLAN24G_NET_TYPE, wifiMode);
		}
		//无线配置修改才重启无线
		if(1 == wl2g_restart)
		{
			send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,OP_RESTART,msg);
		}
	}
	if(1 == wifi_info_5g->wl_radio)
	{
		wifiChannel_5g = strcmp(wifiChannel_5g,"auto") ? wifiChannel_5g : "0";
		/*wifi channel*/
		memset(value,0x0,sizeof(value));
		sprintf(value, "%d", wifi_info_5g->channel);
		if (0 != strcmp(value, wifiChannel_5g))
		{
			wl5g_restart = 1;
			/*add wzs ,channel 10~13 sideband should be "upper".*/
			_SET_VALUE(WLAN5G_CHANNEL, wifiChannel_5g);	
		}

		/*wifi bandwidth*/
		if(0 != strcmp(wifiBandwidth_5g,wifi_info_5g->bandwidth))
		{
			wl5g_restart = 1;
			//wifi bandsid
			int i = 0;
			if (0 == strcmp(wifiBandwidth_5g,"20") || 0 == strcmp(wifiBandwidth_5g,"80") 
				|| 0 == strcmp(wifiChannel_5g,"0"))
			{
				_SET_VALUE(WLAN5G_BANDSIDE, "none");
			}
			else 
			{
				PI8 bandside_5g[16] = "none";
				for(i = 0;i < sizeof(wifi_band_upper)/sizeof(int);i++)
				{
					if(wifi_band_upper[i] == atoi(wifiChannel_5g))
					{
						strcpy(bandside_5g,"upper");
						break;
					}
				}
				for(i = 0;i < sizeof(wifi_band_lower)/sizeof(int);i++)
				{
					if(wifi_band_lower[i] == atoi(wifiChannel_5g))
					{
						strcpy(bandside_5g,"lower");
						break;
					}
				}
				for(i = 0;i < sizeof(wifi_band_none)/sizeof(int);i++)
				{
					if(wifi_band_none[i] == atoi(wifiChannel_5g))
					{
						strcpy(bandside_5g,"none");
						break;
					}
				}
				_SET_VALUE(WLAN5G_BANDSIDE, bandside_5g);
			}
			
       		 _SET_VALUE(WLAN5G_BANDWIDTH, wifiBandwidth_5g);
		}
		/* wifi mode */
		if (0 != strcmp(wifiMode_5g, wifi_info_5g->nettype))
		{
			wl5g_restart = 1;
			_SET_VALUE(WLAN5G_NET_TYPE, wifiMode_5g);
		}
		//无线配置修改 才重启无线
		if(1 == wl5g_restart)
		{
			send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,OP_RESTART,msg);
		}
	}

	//2017/3/14 F9添加无线抗干扰功能
#ifdef __CONFIG_DISTURB_EN__
	PI8 *wifiAntijamEn = NULL;
   	//无线抗干扰只对2.4G有效
	wifiAntijamEn = cgi_lib_get_var(wp, root,T(LIB_WIFI_ANTIJAM_EN), "false");
	if(!strcmp(wifiAntijamEn,"false"))	//关闭抗干扰
	{
		strcpy(buf_temp,WIFI_ANTINAM_EN_OFF);
	}
	else
	{
		strcpy(buf_temp,WIFI_ANTINAM_EN_ON);
	}

	if(strcmp(wifi_info->antijam_en,buf_temp))
	{
		_SET_VALUE(WLAN_PUBLIC_DIG_SEL, buf_temp);
		send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,OP_RESTART,msg);	
	}
#endif
	sprintf(err_code, "%s", "0");
	
	return RET_SUC;
}

extern void wl_channel_optimize_app();
/*****************************************************************************
 函 数 名  : cgi_lib_set_wifi_channel_grade
 功能描述  : 设置自动信道，app专用
 输入参数  : webs_t wp            
             cJSON *root          
             CGI_MSG_MODULE *msg  
             char *err_code       
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月22日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wifi_channel_grade(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	_SET_VALUE(WLAN24G_CHANNEL,"0");	
	/*进行信道优化，就是设置为自动信道,驱动会根据当前选择最优信道*/
	wl_channel_optimize_app();
	return RET_SUC;
}

#ifdef __CONFIG_WPS_RTK__
/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_wps
 功能描述  : 获取WPS信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月20日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_wps(webs_t wp, cJSON *root, void *info)
{
	PI8 *wps_en = NULL;
	P_WIFI_INFO_STRUCT wifi_info = NULL;

	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);
	
	//wpsEn
	wps_en = strncmp(wifi_info->wps_enable,"enable",strlen("enable")) ? "false" : "true";
	cJSON_AddStringToObject(root, LIB_WPS_EN, wps_en);	
	//wpsPIN
	cJSON_AddStringToObject(root, LIB_WPS_PIN, nvram_safe_get(WPS_DEVIDE_PIN));

	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_wifi_wps
 功能描述  : 设置WPS参数
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月20日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wifi_wps(webs_t wp, cJSON *root, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	int msg_op_code = COMMON_MSG_MAX;
	PI8 *wps_enable = NULL;
	
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	
	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);


	char_t *old_wps, old_wps_str[16];
	char_t *wps_oob_status ;

	wps_enable = cgi_lib_get_var(wp, root, T("wpsEn"), T("false"));
	wps_enable = strncmp(wps_enable,"true",strlen("true")) ? "disabled" : "enabled";

	//开启wps 只在路由模式下支持 wps
	if(NULL != wps_enable && 0 == strncmp(wps_enable,"enabled",strlen("enabled")))
	{
		if(0 != strcmp(wps_enable,wifi_info->wps_enable))
		{
			msg_op_code = OP_START;		
			_SET_VALUE(WPS_MODE_ENABLE,"enabled");
			_SET_VALUE(WPS_METHOD,"pbc");
		}
	}
	else
	{
		if(0 != strcmp(wps_enable,wifi_info->wps_enable))
		{
			msg_op_code = OP_STOP;			
			_SET_VALUE(WPS_MODE_ENABLE, "disabled");
			_SET_VALUE(WPS_METHOD, "");
		}
	}

	if (COMMON_MSG_MAX != msg_op_code)
	{
		CGI_MSG_MODULE msg_tmp;
		msg_tmp.id = RC_WPS_MODULE;
		sprintf(msg_tmp.msg, "op=%d", msg_op_code);
		add_msg_to_list(msg, &msg_tmp);
	}

	sprintf(err_code, "%s", "0");

	return RET_SUC;
}
#endif

#ifdef __CONFIG_AUTO_CONN_CLIENT__
/*****************************************************************************
 函 数 名  : cgi_auto_sync_info_get
 功能描述  : 获取本次启动后自动同步配置是否完成，只有恢复出厂设置时调用才有意义
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年3月24日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
extern int auto_sync_end;//全局变量定义在auto_conn_client_main.c文件中
RET_INFO cgi_lib_auto_sync_info_get(webs_t wp, cJSON *root, void *info)
{
	char value[16] = {0};

	if (auto_sync_end)
	{
		snprintf(value , sizeof(value) ,  "%s" , "true" );
	}
	else
	{
		snprintf(value , sizeof(value) ,  "%s" , "false" );
	}

	cJSON_AddStringToObject(root, LIB_AUTO_SYNC_STATUS, value);
	return RET_SUC;
}
#endif

#ifdef __CONFIG_GUEST__
/*****************************************************************************
 函 数 名  : cgi_lib_get_wifi_guest_info
 功能描述  : 获取访客网络基本信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wifi_guest_info(webs_t wp, cJSON *root, void *info)
{
	char *wifi_guest_en = "false";
	PI8 security_mode[PI_BUFLEN_32] = {0};
	SYS_WORK_MODE sys_work_mode = WL_ROUTE_MODE;
	char *mib_value = NULL;
	char effect_time_string[PI_BUFLEN_32] = {0};
	char down_speed_limit_str[PI_BUFLEN_32] = {0};
	P_WIFI_INFO_STRUCT wifi_info_2g = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	WIFI_SSID_CFG wifi_guest_info;

	wifi_info_2g = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);
	
	//2.4G wifi guest
	memset(&wifi_guest_info,0x0,sizeof(wifi_guest_info));
	gpi_wifi_guest_get_info(&wifi_guest_info,TENDA_WLAN24_GUEST_IFNAME);

	wifi_guest_en = wifi_guest_info.bss_enable ? "true" : "false";
	if(WL_ROUTE_MODE != gpi_wifi_get_mode())
	{
		wifi_guest_en = "false";  /*访客网络仅在路由模式下并且无线开启条件下支持*/
	}
	cJSON_AddStringToObject(root, LIB_WIFI_GUEST_EN,wifi_guest_en);
	
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_SSID,wifi_guest_info.ssid);
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_HIDE_SSID,wifi_guest_info.ssid_hide ? "true":"false");
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_PWD,wifi_guest_info.wpa_psk_key);

	/*security mode*/
	get_wlan_security_mode(&wifi_guest_info,security_mode);
	cJSON_AddStringToObject(root, LIB_WIFI_GUEST_SEC_MODE, security_mode);

	//5G wifi guest
	memset(&wifi_guest_info,0x0,sizeof(wifi_guest_info));
	gpi_wifi_guest_get_info(&wifi_guest_info,TENDA_WLAN5_GUEST_IFNAME);

    wifi_guest_en = wifi_guest_info.bss_enable ? "true" : "false";
	if(WL_ROUTE_MODE != gpi_wifi_get_mode())
	{
		wifi_guest_en = "false";  
	}
	cJSON_AddStringToObject(root, LIB_WIFI_GUEST_EN_5G,wifi_guest_en);
	
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_SSID_5G,wifi_guest_info.ssid);
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_HIDE_SSID_5G,wifi_guest_info.ssid_hide ? "true":"false");
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_PWD_5G,wifi_guest_info.wpa_psk_key);
	/*security mode*/
	memset(security_mode,0x0,sizeof(security_mode));
	get_wlan_security_mode(&wifi_guest_info,security_mode);
	cJSON_AddStringToObject(root, LIB_WIFI_GUEST_SEC_MODE_5G, security_mode);
	/*wifi guest effect_time */
	mib_value = nvram_safe_get(WLAN_PUBLIC_GUEST_EFFECTIVE_TIME);
	snprintf(effect_time_string,sizeof(effect_time_string),"%d",atoi(mib_value)/3600);
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_EFFECT_TIME,strcmp(effect_time_string,"0") ? effect_time_string : "always");
	/*wifi guest down speed limit /Mbps*/
	mib_value = nvram_safe_get(WLAN_PUBLIC_GUEST_DOWNSPEED_LIMIT);
	snprintf(down_speed_limit_str,sizeof(down_speed_limit_str),"%d",atoi(mib_value));
	cJSON_AddStringToObject(root,LIB_WIFI_GUEST_SHARE_SPEED,down_speed_limit_str);

	return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_set_wifi_guest_info
 功能描述  : 设置访客网络基本信息
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wifi_guest_info(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	char *guest_en = NULL;
	char *guest_en_5g = NULL;
	char *guest_ssid = NULL;
	char *guest_pwd = NULL;
	char *guest_ssid_5g = NULL;
	char *guest_pwd_5g = NULL;
	char *effect_time_p = NULL;
	char *down_speed_limit_p =NULL;
	char effect_time_str[PI_BUFLEN_32] = {0};
	char down_speed_limit_str[PI_BUFLEN_32] = {0};
    int guest_conf_change_2g = 0;
	int guest_conf_change_5g = 0;
	P_WIFI_INFO_STRUCT wifi_info_2g = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	
	WIFI_SSID_CFG wifi_guest_info_2g;
	WIFI_SSID_CFG wifi_guest_info_5g;
	
	wifi_info_2g = gpi_wifi_get_info(WLAN_RATE_24G);
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);

	memset(&wifi_guest_info_2g,0x0,sizeof(wifi_guest_info_2g));
	memset(&wifi_guest_info_5g,0x0,sizeof(wifi_guest_info_5g));
	gpi_wifi_guest_get_info(&wifi_guest_info_2g,TENDA_WLAN24_GUEST_IFNAME);
	gpi_wifi_guest_get_info(&wifi_guest_info_5g,TENDA_WLAN5_GUEST_IFNAME);

	if((WL_ROUTE_MODE != gpi_wifi_get_mode())  
		|| (0 == wifi_info_2g->wl_radio && 0 == wifi_info_5g->wl_radio))
	{
		sprintf(err_code, "%s", "0");
		printf("[%s][%d] current mode is not route or not open wifi,do not set guest!\n",__func__,__LINE__);
		return;
	}
	guest_en = strcmp(cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_EN), T("false")),"true") ? "0" : "1";
	guest_ssid = cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_SSID), T("Tenda_VIP"));
	guest_pwd = cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_PWD), T(""));
	guest_en_5g = strcmp(cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_EN_5G), T("false")),"true") ? "0" : "1";
	guest_ssid_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_SSID_5G), T("Tenda_VIP_5G"));
	guest_pwd_5g = cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_PWD_5G), T(""));

    //2.4G 开启并配置访客网络
	if(1 == atoi(guest_en))
	{
		//guest enable
		if(atoi(guest_en) != wifi_guest_info_2g.bss_enable)
		{
			guest_conf_change_2g = 1;
			_SET_VALUE(WLAN24G_GUEST_ENABLE,guest_en);
		}
		//guest ssid
		if(0 != strcmp(guest_ssid,"") && 0 != strcmp(guest_ssid,wifi_guest_info_2g.ssid))
		{
			guest_conf_change_2g = 1;
			_SET_VALUE(WLAN24G_GUEST_SSID,guest_ssid);
		}
		//guest pwd
		if(0 != strcmp(guest_pwd,wifi_guest_info_2g.wpa_psk_key))
		{
			guest_conf_change_2g = 1;
			if(0 == strcmp(guest_pwd,""))
			{
				config_wlan_security(TENDA_WLAN24_GUEST_IFNAME,"NONE",guest_pwd);	//密码为空 表示不加密
			}
			else
			{
				config_wlan_security(TENDA_WLAN24_GUEST_IFNAME,"WPAWPA2/AES",guest_pwd);//默认 混合加密
			}
		}

		if(1 == guest_conf_change_2g)
		{
			app_msg_op_code_2g = OP_RESTART;
			send_wifi_msg_handle(TENDA_WLAN24_GUEST_IFNAME,OP_RESTART,msg);	
		}
	}
	else
	{
		//关闭访客网络
		if(atoi(guest_en) != wifi_guest_info_2g.bss_enable)
		{
			_SET_VALUE(WLAN24G_GUEST_ENABLE,guest_en);
			app_msg_op_code_2g = OP_STOP;
			send_wifi_msg_handle(TENDA_WLAN24_GUEST_IFNAME,OP_STOP,msg);	
		}	
	}

	//5G 开启并配置访客网络
	if(1 == atoi(guest_en_5g))
	{
		//guest enable
		if(atoi(guest_en_5g) != wifi_guest_info_5g.bss_enable)
		{
			guest_conf_change_5g = 1;
			_SET_VALUE(WLAN5G_GUEST_ENABLE,guest_en_5g);
		}
		//guest ssid
		if(0 != strcmp(guest_ssid_5g,"") && 0 != strcmp(guest_ssid_5g,wifi_guest_info_5g.ssid))
		{
			guest_conf_change_5g = 1;
			_SET_VALUE(WLAN5G_GUEST_SSID,guest_ssid_5g);
		}
		//guest pwd
		if(0 != strcmp(guest_pwd_5g,wifi_guest_info_5g.wpa_psk_key))
		{
			guest_conf_change_5g = 1;
			if(0 == strcmp(guest_pwd_5g,""))
			{
				config_wlan_security(TENDA_WLAN5_GUEST_IFNAME,"NONE",guest_pwd_5g);	//密码为空 表示不加密
			}
			else
			{
				config_wlan_security(TENDA_WLAN5_GUEST_IFNAME,"WPAWPA2/AES",guest_pwd_5g);//默认 混合加密
			}
		}

		if(1 == guest_conf_change_5g)
		{
			app_msg_op_code_5g = OP_RESTART;
			send_wifi_msg_handle(TENDA_WLAN5_GUEST_IFNAME,OP_RESTART,msg);	
		}
	}
	else
	{
		//关闭访客网络
		if(atoi(guest_en_5g) != wifi_guest_info_5g.bss_enable)
		{
			_SET_VALUE(WLAN5G_GUEST_ENABLE,guest_en_5g);
			app_msg_op_code_5g = OP_STOP;
			send_wifi_msg_handle(TENDA_WLAN5_GUEST_IFNAME,OP_STOP,msg);	
		}	
	}

	//访客网络开启 才去设置有效时间和限速
	if((1 == atoi(guest_en) && 1 == wifi_info_2g->wl_radio)
		|| (1 == atoi(guest_en_5g) && 1 == wifi_info_5g->wl_radio))
	{
		effect_time_p = cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_EFFECT_TIME), T("8"));
		snprintf(effect_time_str,sizeof(effect_time_str),"%d",atoi(effect_time_p)*3600);   
		down_speed_limit_p = cgi_lib_get_var(wp, root, T(LIB_WIFI_GUEST_SHARE_SPEED), T("0"));
		snprintf(down_speed_limit_str,sizeof(down_speed_limit_str),"%d",atoi(down_speed_limit_p));   //页面为 KB

		effect_time_p = nvram_safe_get(WLAN_PUBLIC_GUEST_EFFECTIVE_TIME);
		down_speed_limit_p = nvram_safe_get(WLAN_PUBLIC_GUEST_DOWNSPEED_LIMIT);
		
		//effective time
		effect_time_p = strcmp(effect_time_p,"always") ? effect_time_p : "0";
		if(0 != strcmp(effect_time_str,effect_time_p) )
		{
			app_msg_op_code_2g = OP_RESTART;
			app_msg_op_code_5g = OP_RESTART;
			nvram_set(WLAN_PUBLIC_GUEST_EFFECTIVE_TIME, effect_time_str);
			send_wifi_msg_handle(TENDA_WLAN24_GUEST_IFNAME,OP_RESTART,msg);
			send_wifi_msg_handle(TENDA_WLAN5_GUEST_IFNAME,OP_RESTART,msg);
		}
		//shared down speed
		if(0 != strcmp(down_speed_limit_str,down_speed_limit_p))
		{
			nvram_set(WLAN_PUBLIC_GUEST_DOWNSPEED_LIMIT, down_speed_limit_str);

			CGI_MSG_MODULE msg_tmp;
			msg_tmp.id = RC_TC_MODULE;
			sprintf(msg_tmp.msg, "op=%d", OP_RESTART);
			add_msg_to_list(msg, &msg_tmp);
		}
	}

	sprintf(err_code, "%s", "0");
	
	return RET_SUC;
}
#endif 

#ifdef __CONFIG_WL_BEAMFORMING_EN__
/*****************************************************************************
 函 数 名  : cgi_lib_get_beaforming_enable
 功能描述  : 获取无线beaforming 功能
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月13日
    作    者   : lrl
    修改内容   : 添加适配5G

*****************************************************************************/
RET_INFO cgi_lib_get_beaforming_enable(webs_t wp, cJSON *root, void *info)
{
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);
	
	cJSON_AddStringToObject(root, LIB_HAS_WIFI_BEAFORMING, "true");/*页面根据此参数是否显示beaforming*/ 
    cJSON_AddStringToObject(root, LIB_WIFI_BEAFORMING_ENABLE, wifi_info_5g->wl_beamforming_en ? "true" : "false");

	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_beaforming_enable
 功能描述  : 开启/关闭 beaforming功能
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月13日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_beaforming_enable(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{ 
	PI8 *wl_beamforming_en = NULL;
	P_WIFI_INFO_STRUCT wifi_info_5g = NULL;
	
	wifi_info_5g = gpi_wifi_get_info(WLAN_RATE_5G);
	
	wl_beamforming_en = cgi_lib_get_var(wp,root,T(LIB_WIFI_BEAFORMING_ENABLE),"true");
	wl_beamforming_en = strncmp(wl_beamforming_en,"true",strlen("true")) ? "0" : "1";

	if(atoi(wl_beamforming_en) != wifi_info_5g->wl_beamforming_en)
	{
		_SET_VALUE(WLAN_PUBLIC_BEAMFORMING_EN, wl_beamforming_en);
		send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,OP_RESTART,msg);
	}
	
	sprintf(err_code, "%s", "0");

	return RET_SUC;
}
#endif

