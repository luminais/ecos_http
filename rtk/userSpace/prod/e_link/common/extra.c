#include<stdlib.h>
#include <string.h>
#include "elink_log_debug.h"
#include "extra.h"
#include "led.h"
#include "proto.h"
#include "wan.h"
#include "flash_cgi.h"
#include "sys_module.h"
#include "wifi.h"
#include "version.h"
#include "cgi_tc.h"
#include "wlioctl.h"
#include "arp_clients.h"
#include "dhcp_server.h"
#include "wl_utility_rltk.h"
#include "cJSON.h"
struct roaming_ctx {
	int roaming_enable;      //enable
	int threshold_rssi;        //rssi 阀值
	int report_interval;       //时间间隔
	int start_time;               //开始检测时间
	int start_rssi;                //强于该值
};

typedef struct WIFI_BUFFER
{
	char wifi_buf[MAX_ROW_LINE];
	char mib_buf[MAX_ROW_LINE];
	int max_length;
}WIFI_BUF;

typedef enum {
	DEVICE_TYPE_ANDROID = 0,
	DEVICE_TYPE_IPAD,
	DEVICE_TYPE_IPHONE,
	DEVICE_TYPE_WPHOHE,
	DEVICE_TYPE_UNKNOWN,
	DEVICE_TYPE_PC,
	DEVICE_TYPE_ROUTER,
	DEVICE_TYPE_MAX,
}device_host_type;

#pragma pack(1)
struct client_mac_info {
	char access_type;
	char band_type;
	char forbid;
	char client_mac[18];
	char ssid[32];
	char client_name[256];
	unsigned int client_ip;
	unsigned int second;
	device_host_type	host_type;
};
#pragma pack()

/*****************************************************************************
 函 数 名  : extra_get_channel
 功能描述  : 获取信道
 输入参数  : int *channel_count      
             WLAN_RATE_TYPE wl_rate  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年5月16日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int extra_get_channel(int *channel_count,WLAN_RATE_TYPE wl_rate)
{	
    WIFI_CURRCET_INFO_STRUCT wifi_info;

    if(NULL == channel_count)
    {
        return -1;
    }

    gpi_wifi_get_curret_info(wl_rate,&wifi_info);

    *channel_count = wifi_info.channel;

    return 0;
}

int extra_get_current_channel(int *channel_count,WLAN_RATE_TYPE wl_rate)
{	
	WIFI_CURRCET_INFO_STRUCT wifi_curr_info;
	
	if(NULL == channel_count)
	{
	    return -1;
	}

	memset(&wifi_curr_info, 0x0, sizeof(WIFI_CURRCET_INFO_STRUCT));
	gpi_wifi_get_curret_info(wl_rate,&wifi_curr_info);

	*channel_count = wifi_curr_info.channel;

    return 0;
}
/*****************************************************************************
 函 数 名  : extra_set_channel
 功能描述  : 设置无线信道
 输入参数  : int channel_count       
             WLAN_RATE_TYPE wl_rate  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2018年5月16日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int extra_set_channel(int channel_count,WLAN_RATE_TYPE wl_rate)
{
	log_trace(LOG_TRACE_STRING);
	char value[16] = {0};
	char str_channel_count[16] = {0};
	P_WIFI_INFO_STRUCT wifi_info = NULL;

	wifi_info = gpi_wifi_get_info(wl_rate);
	
	sprintf(str_channel_count,"%d",channel_count);

	memset(value, 0x0, sizeof(value));
	sprintf(value, "%d", wifi_info->channel);


	if (strcmp(value, str_channel_count) != 0)
	{
		if(wl_rate == WLAN_RATE_24G)
		{
			_SET_VALUE(WLAN24G_CHANNEL, str_channel_count);

			/*bandside*/
			if (channel_count <= 5 && channel_count >= 1) //Realtek中扩展信道改为5及其以下向上扩展
			{
				_SET_VALUE(WLAN24G_BANDSIDE, "lower");
			}
			else if (channel_count <= 13 && channel_count >= 8)
			{
				_SET_VALUE(WLAN24G_BANDSIDE, "upper");
			}
		}else if(wl_rate == WLAN_RATE_5G)
		{
			_SET_VALUE(WLAN5G_CHANNEL, str_channel_count);
		}
	}
	
	return 0;

}

int extra_get_ap_status(int apidx,ap_status *status_buf,WLAN_RATE_TYPE wl_rate)
{
	char value[64] = {0};
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_SSID_CFG   ssid_cfg = NULL;
	wifi_info = gpi_wifi_get_info(wl_rate);
	

	status_buf->apidx = apidx;
	
	if(0 == apidx)
	{ 
		ssid_cfg = &(wifi_info->ap_ssid_cfg);
	}
	else 
	{
		ssid_cfg = &(wifi_info->guest_ssid_cfg);
	}

	if(ssid_cfg->bss_enable == 1)
	{
		strcpy(status_buf->enable_buf,"yes");  //enable
		strcpy(status_buf->ssid_buf,ssid_cfg->ssid);
		if(strcmp(ssid_cfg->security, "none") == 0) //不加密
		{
			strcpy(status_buf->auth_buf,"open");	//加密方式
			strcpy(status_buf->key_buf,"none");		//密码
		}
		else
		{
			if (strcmp(ssid_cfg->wpa_psk_type, "psk") == 0)
			{
				snprintf(value , sizeof(value) ,  "%s" , "wpapsk" );
			}
			else if (strcmp(ssid_cfg->wpa_psk_type, "psk2") == 0)
			{
				snprintf(value , sizeof(value) ,  "%s" , "wpa2psk" );
			}
			else if (strcmp(ssid_cfg->wpa_psk_type, "psk psk2") == 0)
			{
				
				snprintf(value , sizeof(value) ,  "%s" , "wpapskwpa2psk" );
			}

			if (strcmp(ssid_cfg->wap_psk_crypto, "aes") == 0)
			{
				strcpy(status_buf->encrypt_buf,"aes");	
			}
			else if (strcmp(ssid_cfg->wap_psk_crypto, "tkip") == 0)
			{
				strcpy(status_buf->encrypt_buf,"tkip");	//数据认证方式
			}
			else if (strcmp(ssid_cfg->wap_psk_crypto, "tkip+aes") == 0)
			{
				strcpy(status_buf->encrypt_buf,"aestkip");	//数据认证方式
			}
			else
				strcpy(status_buf->encrypt_buf,"aes");	//数据认证方式

			strcpy(status_buf->auth_buf,value);
			strcpy(status_buf->key_buf,ssid_cfg->wpa_psk_key);
		}

	}
	else
	{
		strcpy(status_buf->enable_buf,"no");
		strcpy(status_buf->ssid_buf,"\0");
		strcpy(status_buf->key_buf,"\0");
		strcpy(status_buf->auth_buf,"\0");
		strcpy(status_buf->encrypt_buf,"\0");
	}	
	return 0;	

}


int extra_get_dev_reg_vendor(char *vendor)
{
	strcpy(vendor, "Tenda");
	return 0;

}


char extra_get_dev_reg_model(char *model)
{
	strcpy(model,"AC6");
	return 0;

}


char extra_get_dev_reg_url(char *url)
{
	strcpy(url,"www.tenda.cn.com");
	return 0;
}



char extra_get_soft_version(char *swversion)
{
	char code_name[32] = {0};
		//获取版本信息
	memset(code_name,0x0,sizeof(code_name));
#if defined(__CONFIG_WEB_VERSION__)
	sprintf(code_name, "%s_%s", W311R_ECOS_SV,NORMAL_WEB_VERSION);
#else
	sprintf(code_name, "%s", W311R_ECOS_SV);
#endif
	strcpy(swversion,code_name);

	return 0;
}


char extra_get_hd_version(char *hdversion)
{
	
	strcpy(hdversion,nvram_safe_get("BOARD_NAME"));

	return 0;
}


char extra_get_dev_reg_wireless(char *wireless)
{
	strcpy(wireless,"no");
	return 0;

}

extern unsigned long get_stream_statistic(int dir);
extern void get_bridge_stream_speed(u_long * tx,u_long * rx);
unsigned long extra_get_stream_statistic(int dir)
{
	unsigned long statis = 0;
	unsigned long tx = 0;
	unsigned long rx = 0;
	 if(nvram_match(SYSCONFIG_WORKMODE, "client+ap")
	|| nvram_match(SYSCONFIG_WORKMODE, "bridge"))
	{
		get_bridge_stream_speed(&tx,&rx);
		if(dir)
			statis = tx / 1024;
		else
			statis = rx /1024;
	 }else
	 {
	 	statis = get_stream_statistic(dir);
	 }
	return statis;
}
extern int get_olnline_time();
int extra_get_online_time(char *online_time)
{
	if(online_time == NULL)
		return -1;
	P_WAN_INFO_STRUCT wan_common_info = NULL;
	wan_common_info = gpi_wan_get_info();
	/*run time*/
	if(nvram_match(SYSCONFIG_WORKMODE, "route") || nvram_match(SYSCONFIG_WORKMODE, "wisp"))
		sprintf(online_time, "%llu", wan_common_info->connect_time);
	else
		sprintf(online_time, "%d",get_olnline_time());
	return 0;
}
//route/bridge/repeater
//route,client+ap,wisp,bridge
int extra_get_current_work_mode(char *workmode)
{
	if(workmode == NULL)
		return -1;
	if (nvram_match(SYSCONFIG_WORKMODE, "route"))
	{
		strcpy(workmode,"route");
	}else if(nvram_match(SYSCONFIG_WORKMODE, "client+ap")
	|| nvram_match(SYSCONFIG_WORKMODE, "wisp"))
	{
		strcpy(workmode,"repeater");
	}else if(nvram_match(SYSCONFIG_WORKMODE, "bridge"))
	{
		strcpy(workmode,"bridge");
	}
	
}


int extra_get_online_client_num(terminal_type type)
{
	int wired_num = 0;
	int wl2_num = 0;
	int wl5_num = 0;
	int wl2_guest_num = 0;
	int wl5_guest_num = 0;
	int ret_num = 0;
	switch(type)
	{
		case WIRED:
			wired_num = tenda_arp_get_wired_online_client_num();
			ret_num = wired_num;
			break;
		case WIRESS_2G:
			getWlStaNum( TENDA_WLAN24_AP_IFNAME, &wl2_num);
#ifdef __CONFIG_GUEST__
			getWlStaNum( TENDA_WLAN24_GUEST_IFNAME, &wl2_guest_num);
#endif
			ret_num = wl2_num + wl2_guest_num;
			break;
		case WIRESS_5G:
			getWlStaNum( TENDA_WLAN5_AP_IFNAME, &wl2_num);
#ifdef __CONFIG_GUEST__
			getWlStaNum( TENDA_WLAN5_GUEST_IFNAME, &wl2_guest_num);
#endif
			ret_num = wl2_num + wl2_guest_num;
			break;
	}

    	return ret_num;
}
//PPPOE\DHCP/STATIC
int extra_get_wan_mode(char *wanmode)
{
	if(wanmode == NULL)
		return -1;
	P_WAN_INFO_STRUCT wan_common_info = NULL;
	wan_common_info = gpi_wan_get_info();
	if(0 == strcmp(wan_common_info->wan_proto,"static"))
	{
		strcpy(wanmode,"STATIC");
	}else if(0 == strcmp(wan_common_info->wan_proto,"dhcp"))
	{
		strcpy(wanmode,"DHCP");
	}else if(0 == strcmp(wan_common_info->wan_proto,"pppoe"))
	{
		strcpy(wanmode,"PPPOE");
	}else
	{
		strcpy(wanmode,"DHCP");
	}
    	return RET_SUC;
}
int extra_get_wifiswitch(int apidx,char *wifiswitch,WLAN_RATE_TYPE wl_rate)
{	

	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_SSID_CFG   ssid_cfg = NULL;

	wifi_info = gpi_wifi_get_info(wl_rate);

	if(0 == apidx)
	{ 
		ssid_cfg = &(wifi_info->ap_ssid_cfg);
	}
	else 
	{
		ssid_cfg = &(wifi_info->guest_ssid_cfg);
	}
	
	if(ssid_cfg->bss_enable == 0)
	{
		strcpy(wifiswitch,"OFF");
	}
	else if(ssid_cfg->bss_enable == 1)
	{
		strcpy(wifiswitch,"ON");
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int extra_get_wifipower(WLAN_RATE_TYPE wl_rate)
{
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	
	wifi_info = gpi_wifi_get_info(wl_rate);
	if(0 == strcmp(wifi_info->power,"high"))
		return 0;
	else if(0 == strcmp(wifi_info->power,"normal"))
		return 1;
	else if(0 == strcmp(wifi_info->power,"low"))
		return 2;
}

int extra_set_wifiswitch(int apidx,char *wifiswitch,WLAN_RATE_TYPE wl_rate)
{
	log_trace(LOG_TRACE_STRING);
	char wifi_switch[32] = {0};
	char value[16] = {0};
	int need_restart = 0;
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_SSID_CFG   ssid_cfg = NULL;

	wifi_info = gpi_wifi_get_info(wl_rate);

	if(0 == apidx)
	{ 
		ssid_cfg = &(wifi_info->ap_ssid_cfg);
	}
	else 
	{
		ssid_cfg = &(wifi_info->guest_ssid_cfg);
	}	

	if(0 == strcmp(wifiswitch,"ON") 
		|| 0 == strcmp(wifiswitch,"yes"))
		strcpy(wifi_switch,"1");
	else if(0 == strcmp(wifiswitch,"OFF") 
			|| 0 == strcmp(wifiswitch,"no"))
		strcpy(wifi_switch,"0");
	else
		return -1;

	sprintf(value, "%d", ssid_cfg->bss_enable);
	
	if (0 != strcmp(value, wifi_switch))
	{
		need_restart = 1;
		switch(wl_rate)
		{
			case WLAN_RATE_24G:
				if(apidx == 0)
					_SET_VALUE(WLAN24G_ENABLE, wifi_switch);
				else
					_SET_VALUE(WLAN24G_GUEST_ENABLE, wifi_switch);
				break;
			case WLAN_RATE_5G:
				if(apidx == 0)
					_SET_VALUE(WLAN5G_ENABLE, wifi_switch);
				else
					_SET_VALUE(WLAN5G_GUEST_ENABLE, wifi_switch);
				break;
		}
	}

	return need_restart;	
}

/*type == 1 表示返回结果是根据全开全关判断的*/
int extra_get_all_wifiswitch(char *all_wifiswitch,int type)
{
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	int wlan_enable = 0;
	if(NULL == all_wifiswitch)
	{
		return -1;
	}
	
	wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);

	if (wifi_info->ap_ssid_cfg.bss_enable == 1)
	{
		wlan_enable = 1;
	}

	wifi_info = gpi_wifi_get_info(WLAN_RATE_5G);

	if (wifi_info->ap_ssid_cfg.bss_enable == 1)
	{
		wlan_enable += 1;
	}
	
	if ((type == 0 && wlan_enable) || (type && wlan_enable == 2))
	{
		strcpy(all_wifiswitch,"ON");
	}
	else
	{
		strcpy(all_wifiswitch,"OFF");
	}
	return 0;

}


int extra_set_all_wifiswitch(char *all_wifiswitch)
{
	char wifi_all_switch[32] = {0};
	char action[64] = {0};
	char value[256] = {0};
	int need_restart_wifi = 0;

	need_restart_wifi += extra_set_wifiswitch(0,all_wifiswitch,WLAN_RATE_24G);
	need_restart_wifi += extra_set_wifiswitch(1,all_wifiswitch,WLAN_RATE_24G);
	need_restart_wifi += extra_set_wifiswitch(0,all_wifiswitch,WLAN_RATE_5G);
	need_restart_wifi += extra_set_wifiswitch(1,all_wifiswitch,WLAN_RATE_5G);
	
	return need_restart_wifi;

}
extern LED_STATUS gpi_led_current_state();
int extra_get_ledswitch(char *ledswitch)
{

	if(ledswitch == NULL)
		return -1;
	
	if(LED_STA_OFF == gpi_led_current_state())
		strcpy(ledswitch,"OFF");
	else
		strcpy(ledswitch,"ON");
    
	return 0;
}

int extra_set_ledswitch(char *ledswitch)
{
	if(ledswitch == NULL)
		return -1;
	
	if(strcmp(ledswitch,"ON"))
		nvram_set("led_ctl_type","1");
	else
		nvram_set("led_ctl_type","0");
    
	return 0;
}
#if 0
int extra_get_wifitimer(char *sched_wifi_enable,char *sched_start_time,
	char *sched_end_time,char *wifi_enable,char *sched_repeats)
{
	if((!sched_wifi_enable) || (!sched_start_time) || (!sched_end_time) || (!wifi_enable) || (!sched_repeats))
	{
		return -1;
	}

	char sched_enable[PI_BUFLEN_16] = {0}, wifi_sched_times[PI_BUFLEN_16] = {0}, weeks[PI_BUFLEN_16] = {0};

	gpi_wifi_switch_sched_web_info(sched_enable, wifi_sched_times, weeks);

	strcpy(sched_wifi_enable,sched_enable);
	if (strcmp(sched_wifi_enable, "0") == 0)
	{
		return 0;
	}

	char *p = NULL;
	p = strchr(wifi_sched_times,'-');
	*p = '\0';
	p++;
	strcpy(sched_start_time,wifi_sched_times);
	strcpy(sched_end_time,p);

	sprintf(sched_repeats,"%c%c%c%c%c%c%c",weeks[1],weeks[2],weeks[3],weeks[4],weeks[5],weeks[6],weeks[7]);

	return 0;

}


int extra_set_wifitimer(char *sched_wifi_enable,char *sched_start_time,char *sched_end_time,char *sched_repeats)
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
	
	gpi_wifi_switch_sched_web_info(wifi_sched_enable, wifi_sched_times, wifi_sched_weeks);
	
	sprintf(time_interval,"%s-%s",sched_start_time,sched_end_time);
	
	sched_wifi_enable = strcmp(sched_wifi_enable, "enable") ? "0" : "1";

	if(0 == atoi(sched_wifi_enable) && 0 != strcmp(wifi_sched_enable, sched_wifi_enable))
	{
		//关闭无线定时
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_ENABLE, "0");
		_SET_VALUE(WLAN_PUBLIC_SCHEDULE_LIST_NUM, "0");
		msg_send(MODULE_RC, RC_WIFI_SWITCH_SCHED_MODULE, "op=3");
	}
			
	else if(1 == atoi(sched_wifi_enable) && 
			((0 != strcmp(wifi_sched_enable, sched_wifi_enable))
			|| (0 != strcmp(wifi_sched_times, time_interval))
			|| (0 != strcmp(wifi_sched_weeks, time_week)))) //数据格式不是很清楚
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
			msg_send(MODULE_RC, RC_WIFI_SWITCH_SCHED_MODULE, "op=3");
		}
	}

	return 0;
}
#else
int extra_get_wifitimer(cJSON *timearray)
{
	int i = 0;
	int num = 0;
	char *tmp = NULL;
	cJSON *timearray_item = NULL;
	if(timearray == NULL)
	{
		return -1;
	}

	tmp = nvram_get(ELINK_WLAN_PUBLIC_SCHEDULE_NUM);

	if(tmp == NULL)
		num = 1;
	else
		num = atoi(tmp);

	for(i = 0;i < num;i++)
	{
		timearray_item = cJSON_CreateObject();
		cJSON_AddItemToArray(timearray,timearray_item);
		tmp = nvram_get(ELINK_WLAN_PUBLIC_SCHEDULE_DAY(i));
		if(tmp == NULL)
			cJSON_AddStringToObject(timearray_item, "weekday", "2");
		else
			cJSON_AddStringToObject(timearray_item, "weekday", tmp);
		
		tmp = nvram_get(ELINK_WLAN_PUBLIC_SCHEDULE_TIME(i));
		if(tmp == NULL)
			cJSON_AddStringToObject(timearray_item, "time", "17:30");
		else
			cJSON_AddStringToObject(timearray_item, "time", tmp);
		
		tmp = nvram_get(ELINK_WLAN_PUBLIC_SCHEDULE_ENABLE(i));
		if(tmp == NULL)
			cJSON_AddStringToObject(timearray_item, "enable", "1");
		else
			cJSON_AddStringToObject(timearray_item, "enable", tmp);
	}
	
	return 0;

}

int extra_set_wifitimer(cJSON *timearray)
{
	int i = 0;
	char value[16] = {0};
	int array_num = 0;
	cJSON *item = NULL;
	cJSON *timearray_item = NULL;
	if(timearray == NULL)
	{
		return -1;
	}
	
	array_num = cJSON_GetArraySize(timearray);

	sprintf(value,"%d",array_num);
	nvram_set(ELINK_WLAN_PUBLIC_SCHEDULE_NUM,value);
	for(i = 0;i < array_num;i++)
	{
		timearray_item = cJSON_GetArrayItem(timearray,i);

		item = cJSON_GetObjectItem(timearray_item,"weekday");

		nvram_set(ELINK_WLAN_PUBLIC_SCHEDULE_DAY(i),item->valuestring);

		item = cJSON_GetObjectItem(timearray_item,"time");

		nvram_set(ELINK_WLAN_PUBLIC_SCHEDULE_TIME(i),item->valuestring);

		item = cJSON_GetObjectItem(timearray_item,"enable");

		nvram_set(ELINK_WLAN_PUBLIC_SCHEDULE_ENABLE(i),item->valuestring);
	}
	return 0;

}

#endif


int  extra_set_ap_status_ssid(int apidx,char *ap_status_ssid,WLAN_RATE_TYPE wl_rate)
{

	log_trace(LOG_TRACE_STRING);
	int need_restart_wifi = 0;
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_SSID_CFG   ssid_cfg = NULL;
	
	wifi_info = gpi_wifi_get_info(wl_rate);

	if((NULL == ap_status_ssid) || 0 == strlen(ap_status_ssid))
	{
		return -1;
	}
	
	if(0 == apidx)
	{ 
		ssid_cfg = &(wifi_info->ap_ssid_cfg);
	}
	else 
	{
		ssid_cfg = &(wifi_info->guest_ssid_cfg);
	}
	    
	if (0 != strcmp(ssid_cfg->ssid, ap_status_ssid))
	{
		if (strlen(ap_status_ssid) > 0)
		{
			need_restart_wifi = 1;
			if(wl_rate == WLAN_RATE_24G)
			{
				if(apidx == 0)
				{
					_SET_VALUE(WLAN24G_SSID, ap_status_ssid);
				}else
				{
					_SET_VALUE(WLAN24G_GUEST_SSID, ap_status_ssid);
				}
			}else if(wl_rate == WLAN_RATE_5G)
			{
				if(apidx == 0)
				{
					_SET_VALUE(WLAN5G_SSID, ap_status_ssid);
				}else
				{
					_SET_VALUE(WLAN5G_GUEST_SSID, ap_status_ssid);
				}
			}
		}

	}    
	return 0;	
}




int  extra_get_ap_status_ssid(int apidx,char *ap_status_ssid,WLAN_RATE_TYPE wl_rate)
{
	P_WIFI_INFO_STRUCT wifi_info = NULL;
	P_WIFI_SSID_CFG   ssid_cfg = NULL;
	
	wifi_info = gpi_wifi_get_info(wl_rate);

	if((NULL == ap_status_ssid))
	{
		return -1;
	}
	
	if(0 == apidx)
	{ 
		ssid_cfg = &(wifi_info->ap_ssid_cfg);
	}
	else 
	{
		ssid_cfg = &(wifi_info->guest_ssid_cfg);
	}

	strcpy(ap_status_ssid,ssid_cfg->ssid);
	return 0;	
}


int  extra_set_ap_status_passwd(int apidx,char *auth,char*encrypt,char*key,WLAN_RATE_TYPE wl_rate)
{
	char config_wep[32] = {0};
	char config_akm[32] = {0};
	char config_passwd[32] = {0};
	char config_crypto[32] = {0};
	char wl_config[16] = {0};
	int config_encrypt = 0;
	log_trace(LOG_TRACE_STRING);
	
	if((NULL == auth) || 0 == strlen(auth) 
		|| (NULL == encrypt) || 0 == strlen(encrypt)
		|| (NULL == key) || 0 == strlen(key))
	{
		log_debug("[%s]input none value.\n", __FUNCTION__);
		return -1;
	}

	if(wl_rate == WLAN_RATE_24G)
	{
		if(apidx == 0)
		{	
			sprintf(wl_config,"%s",WL_24G);
				
		}else 
		{
			sprintf(wl_config,"%s",WL_24G_GUEST);
		}
	}else if(wl_rate == WLAN_RATE_5G)
	{
		if(apidx == 0)
		{	
			sprintf(wl_config,"%s",WL_5G);
				
		}else 
		{
			sprintf(wl_config,"%s",WL_5G_GUEST);
		}
	}

	sprintf(config_wep,"%s_wep",wl_config);
	sprintf(config_akm,"%s_akm",wl_config);
	sprintf(config_passwd,"%s_wpa_psk",wl_config);
	sprintf(config_crypto,"%s_crypto",wl_config);
	
	
	if(0 == strcmp(auth, "wpa") 
		|| 0 == strcmp(auth, "wpapsk"))
	{
		log_debug("\n security mode: PSK PSK2\n");
		nvram_set(config_wep, "disabled");
		nvram_set(config_akm, "psk");
		nvram_set(config_passwd, key);
		config_encrypt = 1;


		log_trace(LOG_TRACE_STRING);
	}
	else if(0 == strcmp(auth, "wpa2") 
		|| 0 == strcmp(auth, "wpa2psk"))
	{
		log_debug("\n security mode: PSK PSK2\n");
		nvram_set(config_wep, "disabled");
		nvram_set(config_akm, "psk2");
		nvram_set(config_passwd, key);
		config_encrypt = 1;
		log_trace(LOG_TRACE_STRING);
	}
	else if(0 == strcmp(auth, "wpapskwpa2psk"))
	{
		log_debug("\n security mode: PSK PSK2\n");
		nvram_set(config_wep, "disabled");
		nvram_set(config_akm, "psk psk2");
		nvram_set(config_passwd, key);
		config_encrypt = 1;
		log_trace(LOG_TRACE_STRING);
	}
	else
	{
		log_debug("\n security mode: NONE\n");     
		nvram_set(config_wep, "disabled");
		nvram_set(config_akm, "");
		nvram_set(config_passwd, "");
		nvram_set(config_crypto, "");
	}

	if(config_encrypt == 1)
	{
		if(0 == strcmp(encrypt,"tkip"))
			nvram_set(config_crypto, "tkip");
		else if(0 == strcmp(encrypt,"aes"))
			nvram_set(config_crypto, "aes");
		else if(0 == strcmp(encrypt,"aestkip"))
			nvram_set(config_crypto, "tkip+aes");
		else
			nvram_set(config_crypto, "aes");
	}
	return 1;
}



/*************************************************
Function:     extra_get_wan_mac
Description:  获得路由器的mac地址

Input:
	  unsigned char *mac				  mac地址
	  len                				  sizeof(mac)
Output:
    	无
Return:
	 0 成功
	-1 失败
*************************************************/
int extra_get_wan_mac(unsigned char *mac,int len)
{
	P_WAN_HWADDR_INFO_STRUCT wan_hwaddr_info = NULL;

	if(NULL == mac)
	{
		return -1;
	}
    wan_hwaddr_info = gpi_wan_get_hwaddr_info();
	if(strlen(wan_hwaddr_info->wan_hwaddr) < 17)
	{
		log_error("get wan mac error\n");
		return -1;
	}
	strcpy(mac,wan_hwaddr_info->wan_hwaddr);

	return 0;
}

int extra_get_elink_sn(unsigned char *sn)
{
	char *tmp = NULL;
	if(NULL == sn)
	{
		return -1;
	}

	_GET_VALUE("elink_sn",tmp);
	strcpy(sn,tmp);
}

int save_roaming_config(cJSON *root)
{
	cJSON *enable = NULL;
	cJSON *threshold_rssi = NULL;
	cJSON *report_interval = NULL;
	cJSON *start_time = NULL;
	cJSON *start_rssi = NULL;
	char value[32] ;
	int restart_roaming_timer = 0;

	enable = cJSON_GetObjectItem(root,"enable");
	threshold_rssi = cJSON_GetObjectItem(root,"threshold_rssi");
	report_interval = cJSON_GetObjectItem(root,"report_interval");
	start_time = cJSON_GetObjectItem(root,"start_time");
	start_rssi = cJSON_GetObjectItem(root,"start_rssi");

	if(!nvram_match("roaming_enable",enable->valuestring))
	{
		nvram_set("roaming_enable",enable->valuestring);
		restart_roaming_timer +=1;
	}

	memset(value,0x0,32);
	sprintf(value,"%d",threshold_rssi->valueint);
	if(!nvram_match("threshold_rssi",value))
	{
		nvram_set("threshold_rssi",value);
		restart_roaming_timer +=1;
	}

	memset(value,0x0,32);
	sprintf(value,"%d",report_interval->valueint);
	if(!nvram_match("report_interval",value))
	{
		nvram_set("report_interval",value);
		restart_roaming_timer +=1;
	}

	memset(value,0x0,32);
	sprintf(value,"%d",start_time->valueint);
	if(!nvram_match("start_time",value))
	{
		nvram_set("start_time",value);
		restart_roaming_timer +=1;
	}

	memset(value,0x0,32);
	sprintf(value,"%d",start_rssi->valueint);
	if(!nvram_match("start_rssi",value))
	{
		nvram_set("start_rssi",value);
		restart_roaming_timer +=1;
	}

	return 1;
	
}

int extra_updata_roaming_info(struct roaming_ctx *roaming_config)
{

	char *value = NULL;
	memset(roaming_config,0x0,sizeof(struct roaming_ctx));
	if(nvram_match("roaming_enable","yes"))
	{
		roaming_config->roaming_enable = 1;
	}else
	{
		roaming_config->roaming_enable = 0;
		return 0;
	}

	value = nvram_safe_get("threshold_rssi");

	roaming_config->threshold_rssi = atoi(value);

	value = nvram_safe_get("report_interval");

	roaming_config->report_interval = atoi(value);

	value = nvram_safe_get("start_time");

	roaming_config->start_time = atoi(value);

	value = nvram_safe_get("start_rssi");

	roaming_config->start_rssi = atoi(value);
	return 0;
	
}



int get_client_rssi_and_band(char*src_mac,int *rssi,char*band)
{
	unsigned int i;
	int j = 0;
	char dev_mac[18] = {0};
	WLAN_STA_INFO_T pInfo[MAX_STA_NUM + 1];
	unsigned char mac[18] = {0};
	char *ifname[] ={TENDA_WLAN24_AP_IFNAME,
					TENDA_WLAN5_AP_IFNAME,
#ifdef __CONFIG_GUEST__
					TENDA_WLAN24_GUEST_IFNAME,
					TENDA_WLAN5_GUEST_IFNAME,
#endif
					""};

	*rssi = 0;
	for(j = 0;strcmp(ifname[j],"");j++)
	{
		memset(pInfo,0x0,sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM + 1));
		/* 获取接入路由器的无线客户端 */
		getWlStaInfo( ifname[j], pInfo);

		for (i=1; i<=MAX_STA_NUM; i++)
		{

		    if (pInfo[i].aid && (pInfo[i].flag & STA_INFO_FLAG_ASOC))
		    {
		        memset(mac,0x0,18);
		        sprintf(mac,"%02X%02X%02X%02X%02X%02X",
		                pInfo[i].addr[0],pInfo[i].addr[1],pInfo[i].addr[2],
		                pInfo[i].addr[3],pInfo[i].addr[4],pInfo[i].addr[5]);
			printf("%s",mac);
			printf("%s",src_mac);
			if( 0 == memcmp(src_mac,mac, 12))
			{
				if(strstr(ifname[j],"wlan0"))
				{
					if(band)
						strcpy(band,"5G");
				}else
				{
					if(band)
						strcpy(band,"2.4G");
				}
				*rssi = pInfo[i].rssi - 100;
				return 0;
			}
		    }
		}
	}

    return 0;
}

int elink_router_mode_get_online_list(cJSON *root)
{
	int client_num = 0;
	unsigned int tmp[32] = {0}; 
	char tmp_mac[18] = {0};
	char value[32] = {0};
	int client_rssi = 0;
	unsigned int index = 0;
	struct ether_addr *hw_addr = NULL;
	struct client_info  *clients_list = NULL;
	time_t now_time = time(0);
	unsigned int  up_byte_pers = 0;
	unsigned int  down_byte_pers = 0;
	int online_type = 0;
	cJSON *obj = NULL;
	clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list != NULL )
	{
		memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
		client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);
	}
	else
	{
		perror("memery exhausted!");
		client_num = 0;
	}

	for(index = 0; index < client_num; ++index)
	{
		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		hw_addr = ether_aton(clients_list[index].mac);
		if(hw_addr == NULL)
			continue;
		sscanf(clients_list[index].mac, "%02x:%02x:%02x:%02x:%02x:%02x", &tmp[0],&tmp[1],&tmp[2],&tmp[3],&tmp[4],&tmp[5]);
		sprintf(tmp_mac, "%02X%02X%02X%02X%02X%02X", tmp[0], tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);

		cJSON_AddStringToObject(obj, "mac",tmp_mac);
		if (clients_list[index].hostname[0] == '\0')
		{
			cJSON_AddStringToObject(obj, "hostname","Unknown");
		}else
		{
			cJSON_AddStringToObject(obj, "hostname",clients_list[index].hostname);
		}

		sprintf(value,"%d",now_time -clients_list[index].time);
		cJSON_AddStringToObject(obj, "onlineTime",value);
		         /* 1:    2.4G
 			   2:    5G
 			   3:    2.4G_GUEST
 			   4:	   5G_GUEST*/
		online_type = tenda_arp_mac_to_flag(clients_list[index].mac);
		if(online_type == 0)
			cJSON_AddNumberToObject(obj, "connecttype",0);
		else
			cJSON_AddNumberToObject(obj, "connecttype",1);
		if(online_type == 3 || online_type == 4)
			get_cur_rate(1,inet_addr(clients_list[index].ip), &up_byte_pers, &down_byte_pers);
		else
			get_cur_rate(0,inet_addr(clients_list[index].ip), &up_byte_pers, &down_byte_pers);
		sprintf(value, "%.02f", ((float)up_byte_pers));
		cJSON_AddStringToObject(obj, "uploadspeed",value);
		sprintf(value, "%.02f", ((float)down_byte_pers));
		cJSON_AddStringToObject(obj, "downloadspeed",value);
		if(online_type == 1 || online_type == 3)
			cJSON_AddStringToObject(obj, "band","2.4G");
		else if(online_type == 2 || online_type == 4)
			cJSON_AddStringToObject(obj, "band","5G");
		if(online_type != 0)
		{
			get_client_rssi_and_band(tmp_mac,&client_rssi,NULL);
			sprintf(value, "%d", client_rssi);
			cJSON_AddStringToObject(obj, "rssi",value);
		}
			
	}
	if(clients_list)
		free(clients_list);

	return RET_SUC;	
}

int ap_mode_online_list(cJSON *root)
{
	unsigned int i;
	int j = 0;
	char tmp[32] = {0};
	cJSON *obj = NULL;
	char dev_mac[18] = {0};
	WLAN_STA_INFO_T pInfo[MAX_STA_NUM + 1];
	unsigned char mac[18] = {0};
	char *ifname[] ={TENDA_WLAN24_AP_IFNAME,
					TENDA_WLAN5_AP_IFNAME,
#ifdef __CONFIG_GUEST__
					TENDA_WLAN24_GUEST_IFNAME,
					TENDA_WLAN5_GUEST_IFNAME,
#endif
					""};
	for(j = 0;strcmp(ifname[j],"");j++)
	{
		memset(pInfo,0x0,sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM + 1));
		/* 获取接入路由器的无线客户端 */
		getWlStaInfo( ifname[j], pInfo);

		for (i=1; i<=MAX_STA_NUM; i++)
		{

		    if (pInfo[i].aid && (pInfo[i].flag & STA_INFO_FLAG_ASOC))
		    {
		    	 cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		        memset(mac,0x0,18);
		        sprintf(mac,"%02X%02X%02X%02X%02X%02X",
		                pInfo[i].addr[0],pInfo[i].addr[1],pInfo[i].addr[2],
		                pInfo[i].addr[3],pInfo[i].addr[4],pInfo[i].addr[5]);
			cJSON_AddStringToObject(obj, "mac",mac);
			sprintf(tmp,"%d",pInfo[i].link_time);

			cJSON_AddStringToObject(obj, "onlineTime",tmp);

			cJSON_AddNumberToObject(obj, "connecttype",1);
			
			cJSON_AddStringToObject(obj, "uploadspeed","12");
			cJSON_AddStringToObject(obj, "downloadspeed","232");

			if(strstr(ifname[j],"wlan0"))
			{
				cJSON_AddStringToObject(obj, "band","5G");
			}else
			{
				cJSON_AddStringToObject(obj, "band","2.4G");
			}

			sprintf(tmp, "%d", pInfo[i].rssi - 100);
			cJSON_AddStringToObject(obj, "rssi",tmp);
	
		    }
		}
	}
	return RET_SUC;		
}


void extra_get_client_info(cJSON*root)
{
	if(nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
	{
		ap_mode_online_list(root);

	}
	else
	{
		elink_router_mode_get_online_list(root);
	}	
	return ;
	
}


int  get_wl_dev_probe_list( char *ifname,  int len,  WLAN_DEV_PROBE_INFO_ELINK_Tp  dev_probe_cfg)
{
	char mac[TPI_BUFLEN_32] = {0};	
	int assoc_count = 0;
	int i = 0,dev_num = 0,min_num = len;	
	
	WLAN_DEV_PROBE_INFO_Tp pInfo;
	char *buff = calloc(1, sizeof(WLAN_DEV_PROBE_INFO_T) * (128+1));
	
	if(!getWlDevProbeNum(ifname,&dev_num))
	{
		if(dev_num <= 0){
			return 0;
		}
	}
	else{		
		return 0;
	}
	
	if(min_num > dev_num)
	{
	    min_num = dev_num;
	}
	
	if (getWlDevProbeInfo(ifname, (WLAN_DEV_PROBE_INFO_Tp)buff ) < 0)
	{
		printf("Read wlan dev probe info failed!\n");
		free(buff);
		return 0;
	}

	for (i = 0; i < min_num; i++) 
	{
		pInfo = (WLAN_DEV_PROBE_INFO_Tp)&buff[i*sizeof(WLAN_DEV_PROBE_INFO_T)];			
		sprintf(mac,"%02X%02X%02X%02X%02X%02X",pInfo->mac[0],pInfo->mac[1],pInfo->mac[2],pInfo->mac[3],pInfo->mac[4],pInfo->mac[5]);
		strcpy(dev_probe_cfg[i].mac, mac);		
		dev_probe_cfg[i].isAP = pInfo->isAP;		
		dev_probe_cfg[i].rssi = pInfo->rssi;		
		dev_probe_cfg[i].network = pInfo->network;
		sprintf(dev_probe_cfg[i].ssid,"%s",pInfo->ssid);
	}
	
	free(buff);
	return dev_num;
}

void extra_get_wifi_mode(int mode,char *wifimode)
{
	char value[32] = {0};
	int len = 0;
	if(wifimode == NULL)
		return ;
	
	sprintf(value,"11");
	if(mode & WIRELESS_11B)
		strcat(value,"b/");
	if(mode & WIRELESS_11G)
		strcat(value,"g/");
	if(mode & WIRELESS_11A)
		strcat(value,"a/");
	if(mode & WIRELESS_11N)
		strcat(value,"n/");
	if(mode & WIRELESS_11AC)
		strcat(value,"ac/");

	len = strlen(value) - 1;
	memcpy(wifimode,value,len);
	
}

void extra_get_wifiScanf_info(cJSON *root)
{
	int i =0;
	int j = 0;
	cJSON *obj = NULL;
	char value[64] = {0};
	int dev_num = 0;
	int channel_count = 0;
	WLAN_DEV_PROBE_INFO_ELINK_Tp probe_info = NULL;
	char *ifname[] ={TENDA_WLAN24_AP_IFNAME,
				TENDA_WLAN5_AP_IFNAME,
				""};

	probe_info = calloc(1, sizeof(WLAN_DEV_PROBE_INFO_ELINK_T) * (128+1));
	for(j = 0;strcmp(ifname[j],"");j++)
	{
		memset(probe_info,0x0,sizeof(WLAN_DEV_PROBE_INFO_ELINK_T) * (128+1));
		dev_num = get_wl_dev_probe_list(ifname[j],128,probe_info);
		if(strcmp(TENDA_WLAN24_AP_IFNAME,ifname[j]) == 0)
			extra_get_current_channel(&channel_count,WLAN_RATE_24G);
		else
			extra_get_current_channel(&channel_count,WLAN_RATE_5G);
		for(i = 0;i < (dev_num > 16 ? 16:dev_num);i++)
		{
			if(1 == is_gb2312_code(probe_info[i].ssid))
			{
				continue;
			}
			 cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
			 if(strcmp(TENDA_WLAN24_AP_IFNAME,ifname[j]) == 0)
			 	cJSON_AddStringToObject(obj, "rfband","2.4G");
			 else
			 	cJSON_AddStringToObject(obj, "rfband","5G");

			cJSON_AddStringToObject(obj, "ssidname",probe_info[i].ssid);
			cJSON_AddStringToObject(obj, "bssid",probe_info[i].mac);
			if(probe_info[i].isAP)
				cJSON_AddStringToObject(obj, "networktype","AP");
			else
				cJSON_AddStringToObject(obj, "networktype","Ad-Hoc");
			cJSON_AddNumberToObject(obj, "channel",channel_count);
			sprintf(value,"%d",probe_info[i].rssi);
			cJSON_AddStringToObject(obj, "rssi",value);
			memset(value,0x0,sizeof(value));
			extra_get_wifi_mode(probe_info[i].network,value);
			if(probe_info[i].isAP)
				cJSON_AddStringToObject(obj, "standard",value);
			else
				cJSON_AddStringToObject(obj, "standard","");
		}
		
	}
	if(probe_info)
		free(probe_info);
}

int get_apindex(char* ifname)
{
	if(ifname == NULL)
		return 0;

	if(0 == strcmp(ifname,TENDA_WLAN24_AP_IFNAME)
		|| 0 == strcmp(ifname,TENDA_WLAN5_AP_IFNAME))
		return 0;
	else 
		return 1;
	
}
extern int rtl8192cd_elink_wlan_stats(char*name,struct net_device_elink_stats *stats)	;
void extra_get_wlanstats_info(cJSON *root)
{
	int j = 0;
	cJSON *obj = NULL;
	char value[64] = {0};
	char ap_ssid[65] = {0};
	struct net_device_elink_stats stats;
	char *ifname[] ={TENDA_WLAN24_AP_IFNAME,
				TENDA_WLAN24_GUEST_IFNAME,
				TENDA_WLAN5_AP_IFNAME,
				TENDA_WLAN5_GUEST_IFNAME
				""};

	for(j = 0;strcmp(ifname[j],"");j++)
	{	
		
		rtl8192cd_elink_wlan_stats(ifname[j],&stats);
		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		
		cJSON_AddNumberToObject(obj, "apidx",get_apindex(ifname[j]));

		memset(ap_ssid,0x0,sizeof(ap_ssid));
		if(strstr(ifname[j],TENDA_WLAN24_AP_IFNAME))
		{
			extra_get_ap_status_ssid(get_apindex(ifname[j]),ap_ssid,WLAN_RATE_24G);
			cJSON_AddStringToObject(obj, "ssid",ap_ssid);
			cJSON_AddStringToObject(obj, "band","2.4G");
		}
		else
		{
			extra_get_ap_status_ssid(get_apindex(ifname[j]),ap_ssid,WLAN_RATE_5G);
			cJSON_AddStringToObject(obj, "ssid",ap_ssid);
			cJSON_AddStringToObject(obj, "band","5G");
		}
		cJSON_AddNumberToObject(obj, "totalBytesSent",stats.tx_bytes);
		cJSON_AddNumberToObject(obj, "totalBytesReceived",stats.rx_bytes);
		cJSON_AddNumberToObject(obj, "totalPacketsSent",stats.tx_packets);
		cJSON_AddNumberToObject(obj, "totalPacketsReceived",stats.rx_packets);
		cJSON_AddNumberToObject(obj, "errorsSent",stats.tx_errors);
		cJSON_AddNumberToObject(obj, "errorsReceived",stats.rx_errors);
		cJSON_AddNumberToObject(obj, "discardPacketsSent",stats.tx_drops);
		cJSON_AddNumberToObject(obj, "discardPacketsReceived",stats.rx_data_drops);
	}
	
}
extern int find_mac_in_roaming_del_list(char *mac);
void extra_get_client_mac(struct elink_client *info, int *pnum)
{
	int i = 0;
	int client_num = 0;
	int index = 0;
	struct maclist *mac_list = NULL;
	struct client_info  *clients_list = NULL;
	int mac_list_size;
	unsigned int tmp[32] = {0}; 
	char mac[32] = {0};
	int j = 0;
	char *ifname[] ={TENDA_WLAN24_AP_IFNAME,
					TENDA_WLAN5_AP_IFNAME,
#ifdef __CONFIG_GUEST__
					TENDA_WLAN24_GUEST_IFNAME,
					TENDA_WLAN5_GUEST_IFNAME,
#endif
					""};
#if 0
	if(nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		|| nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
	{
#endif
		mac_list_size = sizeof(uint) + MAX_STA_NUM * sizeof(struct ether_addr);
		mac_list = (struct maclist *)malloc(mac_list_size);
		if(NULL == mac_list)
		{
			log_debug("*** fun=%s; line=%d; no buffers! ***\n", __func__, __LINE__);

			return -1;
		}
		for(j = 0;strcmp(ifname[j],"");j++)
		{
			memset(mac_list,0x0,mac_list_size);
			if (getwlmaclist(ifname[j], mac_list))
			{
				free(mac_list);

				return 0;
			}

			for (i = 0; i < mac_list->count; ++i)
			{
				sprintf(mac,"%02x%02x%02x%02x%02x%02x",mac_list->ea[i].octet[0],mac_list->ea[i].octet[1],\
				mac_list->ea[i].octet[2],mac_list->ea[i].octet[3],mac_list->ea[i].octet[4],mac_list->ea[i].octet[5]);
				/****************此处是为了满足电信测试要求***************/
				if(find_mac_in_roaming_del_list(mac))
					continue;
				//mac 和vmac一样
				strcpy(info[index].mac,mac);
				strcpy(info[index].vmac,mac);
				//所有客户端的接入类型都默认为无线
				info[index].connType = 1;
				index++;
			}

			(*pnum) += (unsigned int)(mac_list->count);
		}
#if 0
	}

	else
	{
		clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
		if(clients_list != NULL )
		{
			memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
			client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);
		}
		else
		{
			perror("memery exhausted!");
			client_num = 0;
		}
		for(index = 0; index < client_num; ++index)
		{
			sscanf(clients_list[index].mac, "%02x:%02x:%02x:%02x:%02x:%02x", &tmp[0],&tmp[1],&tmp[2]
																				,&tmp[3],&tmp[4],&tmp[5]);
			sprintf(info[index].mac, "%02X%02X%02X%02X%02X%02X", tmp[0], tmp[1],tmp[2],
														tmp[3],tmp[4],tmp[5]);
			sprintf(info[index].vmac, "%02X%02X%02X%02X%02X%02X", tmp[0], tmp[1],tmp[2],
														tmp[3],tmp[4],tmp[5]);
			info[index].connType = 1;
		}

		*pnum = client_num;
	}
#endif
	if(mac_list)
		free(mac_list);
	if(clients_list)
		free(clients_list);
	
	return ;
	
}


int extra_get_gateway_ip(char *ip)
{
	SYS_WORK_MODE old_mode = 0;
	P_WAN_INFO_STRUCT wan_common_info = NULL;
	P_DHCP_SERVER_INFO_STRUCT dhcp_lan_info = NULL;

	if(NULL == ip)
	{
		return -1;
	}

    wan_common_info = gpi_wan_get_info();


	old_mode = gpi_wifi_get_mode();

	/*
	WL_AP_MODE              = 0,
    WL_WISP_MODE            = 1,
    WL_APCLIENT_MODE        = 2,
    WL_WDS_MODE             = 3,
    WL_BRIDGEAP_MODE        = 4,

	*/
	log_debug("current mode:%d\n",old_mode);
	
	if(WL_BRIDGEAP_MODE == old_mode)
	{
		log_debug("current mode is ap mode\n");
		dhcp_lan_info = gpi_dhcp_server_info();
		/*lq ap 和 ap+client模式下获取lan口ip*/
		if(nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		|| nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
		{
			log_debug("func:%s line:%d ip:%s\n",__func__,__LINE__,dhcp_lan_info->lan_ip);
			char apclient_dhcpc_ip[17] = {0},apclient_dhcpc_mask[17] = {0};
			gpi_apclient_dhcpc_addr(apclient_dhcpc_ip,apclient_dhcpc_mask);
			if(0 == strcmp(apclient_dhcpc_ip,"") || 0 == strcmp(apclient_dhcpc_mask,""))
			{
				strcpy(apclient_dhcpc_ip,dhcp_lan_info->lan_ip);
				strcpy(apclient_dhcpc_mask,dhcp_lan_info->lan_mask);
			}
			//strcpy(ip,apclient_dhcpc_ip);
			strcpy(ip,nvram_safe_get("wan0_gateway"));
			return 0;
		}
		return -1;
	}
	
	log_debug("old_mode:%d\n",old_mode);


	strcpy(ip,wan_common_info->wan_cur_gw);
	log_debug("func:%s line:%d ip:%s\n",__func__,__LINE__,ip);
	
	return 0;
}

int extra_get_gateway_mac(char *mac)
{
	char ip[20] = {0};
	char *gate_mac = NULL;
	char gateway_mac[18] = {0};
	struct in_addr ip_addr;
	P_WAN_INFO_STRUCT wan_common_info = NULL;
	extra_get_gateway_ip(ip);
	inet_aton(ip,&ip_addr);
	wan_common_info = gpi_wan_get_info();
	if(nvram_match(SYSCONFIG_WORKMODE, "route") && 0 == strcmp(wan_common_info->wan_proto,"pppoe"))
	{
		gate_mac = nvram_safe_get("pppoe_last_dst");
	}else
	{
		get_gateway_mac(ip_addr,gateway_mac);
		gate_mac = gateway_mac;
	}
	strcpy(mac,gate_mac);
}

void extra_skip_quick()
{
	nvram_set(SYSCONFIG_QUICK_SET_ENABLE,"0");
}
//将路由器的工作模式切换为AP模式
extern void route_to_bridge();
extern void other_to_bridge();
void extra_enter_conf_state()
{	
	SYS_WORK_MODE old_mode = 0;

    	tpi_elink_set_connect_st(1);
    
	old_mode = gpi_wifi_get_mode();

	log_debug("current mode:%d\n",old_mode);
	
	if(WL_BRIDGEAP_MODE == old_mode)
	{
		log_debug("current mode is ap mode\n");
		return ;
	}
	
	nvram_set(WLAN24G_REPEATER_ENABLE,"0");
	nvram_set(WLAN5G_REPEATER_ENABLE,"0");
	nvram_set(WLAN24G_WORK_MODE,"ap");
	nvram_set(WLAN5G_WORK_MODE,"ap");
	nvram_set(SYSCONFIG_QUICK_SET_ENABLE,"0");
	log_debug("old_mode:%d\n",old_mode);
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
	
	msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,"string_info=commit");
	msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,"string_info=reboot");
	
}


void extra_enter_reset_state()
{

	/*1. wirte sate file*/
	//doSystemCmd("echo 0 > %s", ELINK_STATUS_INFO_FILE);
	tpi_elink_set_connect_st(0);
	/*2. Tell netctrl mode change. */
    //sprintf(param_str,"op=%d",OP_STOP);
    //send_msg_to_netctrl(ELINK_SWITCH_MODE_MODULE,param_str);
    //log_msg( LOG_LEVEL_DEBUG, "send stop dhcp info.\n");
}


/*************************************************
Function:     extra_wan_connect_status
Description:  获得路由器的wan口状态

Input:
		无
Output:

Return:
	-1 失败
	ELINK_WAN_CONNECTED   1
    ELINK_WAN_DISCONNECT  0
*************************************************/
int extra_wan_connect_status()
{	
	P_WAN_ERR_INFO_STRUCT wan_err_info = NULL;

	wan_err_info = gpi_wan_get_err_info_other();

	if(COMMON_NO_WIRE == wan_err_info->code)
    {
        return ELINK_WAN_DISCONNECT;
    }
	else
	{
		return ELINK_WAN_CONNECTED;
	}

}


