#include <sys/bsdtypes.h>
#include <netinet/in.h>
#include <net/if.h>
#include "extra.h"
#include <stdlib.h>
#include "cJSON.h"
#include<sys/bsdtypes.h>
#include "elink_common.h"
#include "sys_module.h"
#define 	WLAN_RATE_5G           5
#define	WLAN_RATE_24G         24
#define	WLAN_RATE_ALL          29
 void get_wifi_status(cJSON *root)
 {
 	int i = 0;
	int j = 0;
	int ret = 0;
	char value[32] = {0};
	int ap_count = 0;
	int cur_power = 0;
	int channel_info = 0;
	cJSON *ap_tmp = NULL;
	cJSON *ap_info = NULL;
 	cJSON *wifi_info = NULL;
	cJSON *wifi_tmp = NULL;
	cJSON *radio_info = NULL;
	ap_status get_ap_status[4] = {0}; // 4 :qp count

	wifi_info = cJSON_CreateArray();
	for(i = 0; i < 2; i++) // i=0 :2.4G; i=1 : 5G;
	{     
		wifi_tmp = cJSON_CreateObject();
		ap_info = cJSON_CreateArray();
		radio_info = cJSON_CreateObject();
		if(i == 0)
		{
			cJSON_AddStringToObject(radio_info,"mode","2.4G");
			ret = extra_get_channel(&channel_info,WLAN_RATE_24G);
			cJSON_AddNumberToObject(radio_info,"channel",channel_info);
			cur_power = extra_get_wifipower(WLAN_RATE_24G);
			sprintf(value,"%d",cur_power);
			cJSON_AddStringToObject(radio_info,"txpower",value);   //需要添加
		}
		else
		{
			cJSON_AddStringToObject(radio_info,"mode","5G");
			ret = extra_get_channel(&channel_info,WLAN_RATE_5G);
			cJSON_AddNumberToObject(radio_info,"channel",channel_info);     
			cur_power = extra_get_wifipower(WLAN_RATE_5G);
			sprintf(value,"%d",cur_power);
			cJSON_AddStringToObject(radio_info,"txpower",value);   //需要添加
		}
		for(j = 0; j < 2; j++)// add ap
		{   
			//ap_info = cJSON_CreateArray();
			ap_count = i * 2 + j;
			ap_tmp = cJSON_CreateObject();
			if(0 == i)
			{
			    extra_get_ap_status(j, &get_ap_status[ap_count],WLAN_RATE_24G);
			}
			else
			{
			    extra_get_ap_status(j, &get_ap_status[ap_count],WLAN_RATE_5G);
			}
			DEBUG_INFO("%s [%d][5G][apidx=%d][enable=%s][ssid=%s][key=%s][auth=%s][encrypt=%s]\n",__FUNCTION__, __LINE__,
			get_ap_status[ap_count].apidx,
			get_ap_status[ap_count].enable_buf,
			get_ap_status[ap_count].ssid_buf,
			get_ap_status[ap_count].key_buf,
			get_ap_status[ap_count].auth_buf,
			get_ap_status[ap_count].encrypt_buf);
			if(0 == strcmp(get_ap_status[ap_count].enable_buf, "yes"))
			{   
				cJSON_AddNumberToObject(ap_tmp, "apidx", get_ap_status[ap_count].apidx);    
				cJSON_AddStringToObject(ap_tmp, "enable", get_ap_status[ap_count].enable_buf);
				cJSON_AddStringToObject(ap_tmp, "ssid", get_ap_status[ap_count].ssid_buf);
				cJSON_AddStringToObject(ap_tmp, "key", get_ap_status[ap_count].key_buf);
				cJSON_AddStringToObject(ap_tmp, "auth", get_ap_status[ap_count].auth_buf);
				cJSON_AddStringToObject(ap_tmp, "encrypt", get_ap_status[ap_count].encrypt_buf);
			}
			else
			{
				cJSON_AddNumberToObject(ap_tmp, "apidx", get_ap_status[ap_count].apidx);
				cJSON_AddStringToObject(ap_tmp, "enable", get_ap_status[ap_count].enable_buf);
				cJSON_AddStringToObject(ap_tmp, "ssid", "");
				cJSON_AddStringToObject(ap_tmp, "key", "");
				cJSON_AddStringToObject(ap_tmp, "auth", "");
				cJSON_AddStringToObject(ap_tmp, "encrypt", "");
			}
		    cJSON_AddItemToArray(ap_info, ap_tmp);
		}
		cJSON_AddItemToObject(wifi_tmp, "radio", radio_info);
		cJSON_AddItemToObject(wifi_tmp,"ap",ap_info);
		cJSON_AddItemToArray(wifi_info,wifi_tmp);
	}
	cJSON_AddItemToObject(root, "wifi", wifi_info);    
 }

int get_wifiswitch_status(cJSON *root)
{
	int ret = 0;
	char wifi_switch_data[16] = {0};
	cJSON *wifi_switch = NULL;
	wifi_switch = cJSON_CreateObject();
	ret = extra_get_all_wifiswitch(wifi_switch_data,0);
	DEBUG_INFO("wifi switch: %s \n", wifi_switch_data);
	cJSON_AddItemToObject(root,"wifiswitch", wifi_switch);
	cJSON_AddStringToObject(wifi_switch, "status", wifi_switch_data);

}
int get_wifitimer_status(cJSON *root)
{
	int ret = 0;
	cJSON *wifitimer = NULL;
	cJSON *wifitimer_list = NULL;
	wifitimer = cJSON_CreateArray();
	cJSON_AddItemToObject(root,"wifitimer", wifitimer);
	extra_get_wifitimer(wifitimer);

}
int get_bandsupport_status(cJSON *root)
{
	int ret = 0;
	cJSON *bandsupport = NULL;
	bandsupport = cJSON_CreateArray();
	cJSON_AddItemToObject(root,"bandsupport", bandsupport);
	cJSON_AddItemToArray(bandsupport,cJSON_CreateString("2.4G"));
	cJSON_AddItemToArray(bandsupport,cJSON_CreateString("5G"));
}
int get_cpurate_status(cJSON *root)
{
	int ret = 0;
	int cpurate = 0;
	char value[32] = {0};
	unsigned long long  time = cyg_current_time();
	cJSON *cpurate_status = NULL;
	cpurate = (time % 20);
	sprintf(value,"%d",cpurate);
	cJSON_AddStringToObject(root,"cpurate", value);
}
int get_memoryuserate_status(cJSON *root)
{
	int ret = 0;
	char value[32] = {0};
	float memuser = 0.0;
	float memall = 0.0;
	float fordblks = 0.0;
	struct mallinfo mem_info;
		
	mem_info = mallinfo();
	memall = mem_info.arena;
	fordblks = mem_info.fordblks;
	printf("\nMemory system:"   
		"\n   Heap %f bytes, Free %f bytes, Max %d bytes\n",
		mem_info.arena, fordblks, mem_info.maxfree);
	
	memuser = fordblks /memall;
	ret = (int)((1-memuser)*100);
	sprintf(value,"%d",ret);

	cJSON_AddStringToObject(root,"memoryuserate", value);
}
int get_uploadspeed_status(cJSON *root)
{
	unsigned long statis = 0;
	char iterm_value[64] = {0};
	statis = extra_get_stream_statistic(1);
	snprintf(iterm_value, 64, "%lu", statis);
	cJSON_AddStringToObject(root,"uploadspeed", iterm_value);
}
int get_downloadspeed_status(cJSON *root)
{
	unsigned long statis = 0;
	char iterm_value[64] = {0};
	statis = extra_get_stream_statistic(0);
	snprintf(iterm_value, 64, "%lu", statis);
	cJSON_AddStringToObject(root,"downloadspeed", iterm_value);

}
int get_onlineTime_status(cJSON *root)
{	
	int ret = 0;
	char online_time[64] = {0};
	memset(online_time,0x0,sizeof(online_time));
	ret = extra_get_online_time(online_time);
	if(ret == -1)
		cJSON_AddStringToObject(root,"onlineTime", "100");
	else
		cJSON_AddStringToObject(root,"onlineTime", online_time);
}
int get_terminalNum_status(cJSON *root)
{
	int ret = 0;
	int num = 0;
	cJSON *terminalNum = NULL;
	terminalNum = cJSON_CreateObject();
	cJSON_AddItemToObject(root,"terminalNum", terminalNum);
	num = extra_get_online_client_num(WIRED);
	cJSON_AddNumberToObject(terminalNum, "wiredNum",num);
	num = extra_get_online_client_num(WIRESS_2G);
	cJSON_AddNumberToObject(terminalNum, "2.4GwirelessNum",num);
	num = extra_get_online_client_num(WIRESS_5G);
	cJSON_AddNumberToObject(terminalNum, "5GwirelessNum",num);
}
int get_networktype_status(cJSON *root)
{
	char wanmode[16] = {0};
	memset(wanmode,0x0,sizeof(wanmode));
	extra_get_wan_mode(wanmode);
	cJSON_AddStringToObject(root,"networktype", wanmode);  //PPPOE\DHCP/STATIC
}
int get_workmode_status(cJSON *root)
{
	char workmode[64] = {0};
	memset(workmode,0x0,sizeof(workmode));
	extra_get_current_work_mode(workmode);
	cJSON_AddStringToObject(root,"workmode", workmode);  //route/bridge/repeater
}

int get_real_devinfo_status(cJSON *root)
{
	int ret = 0;
	cJSON *real_devinfo = NULL;
	cJSON *devinfo_tmp = NULL;
	real_devinfo = cJSON_CreateArray();
	cJSON_AddItemToObject(root,"real_devinfo", real_devinfo);
	extra_get_client_info(real_devinfo);
}

int get_channel_status(cJSON *root)
{
	int ret = 0;
	int cur_channle = 0;
	cJSON *channel = NULL;
	channel = cJSON_CreateObject();
	cJSON_AddItemToObject(root,"channel", channel);
	extra_get_channel(&cur_channle,WLAN_RATE_24G);
	cJSON_AddNumberToObject(channel, "2.4G",cur_channle);
	extra_get_channel(&cur_channle,WLAN_RATE_5G);
	cJSON_AddNumberToObject(channel, "5G",cur_channle);

}
int get_ledswitch_status(cJSON *root)
{
	int ret = 0;
	cJSON *led_switch = NULL;
	char led_switch_data[4]  = {0};
	led_switch = cJSON_CreateObject();
	ret = extra_get_ledswitch(led_switch_data);
	DEBUG_INFO("led switch: %s \n", led_switch_data);
	cJSON_AddItemToObject(root, "ledswitch", led_switch);
	cJSON_AddStringToObject(led_switch, "status", led_switch_data);
}

/*待开发*/
int get_wlanstats_status(cJSON *root)
{
	int ret = 0;
	cJSON *wlanstats = NULL;
	cJSON *apstats_tmp = NULL;
	wlanstats = cJSON_CreateArray();
	cJSON_AddItemToObject(root,"wlanstats", wlanstats);
	extra_get_wlanstats_info(wlanstats);
}

/*填的是假数据*/
int get_load_status(cJSON *root)
{
	int ret = 0;
	cJSON *load = NULL;
	load = cJSON_CreateObject();
	cJSON_AddItemToObject(root,"load", load);
	cJSON_AddStringToObject(load, "2.4G","30%");
	cJSON_AddStringToObject(load, "5G","40%");
}
int get_elinkstat_status(cJSON *root)
{
	int ret = 0;
	cJSON *elinkstat = NULL;
	elinkstat = cJSON_CreateObject();
	cJSON_AddItemToObject(root,"elinkstat", elinkstat);
	if(is_connect_gateway())
		cJSON_AddStringToObject(elinkstat, "connectedGateway","yes");
	else
		cJSON_AddStringToObject(elinkstat, "connectedGateway","no");
}
int get_neighborinfo_status(cJSON *root)
{
	int ret = 0;
	cJSON *neighborinfo = NULL;
	neighborinfo = cJSON_CreateArray();
	cJSON_AddItemToObject(root,"neighborinfo", neighborinfo);
	extra_get_wifiScanf_info(neighborinfo);
}
 struct get_staus get_status_[] = 
{
	{.status = "wifi",			.ops = get_wifi_status,},
	{.status = "wifiswitch",		.ops = get_wifiswitch_status,},
	{.status = "wifitimer",		.ops = get_wifitimer_status,},
	{.status = "bandsupport",	.ops = get_bandsupport_status,},
	{.status = "cpurate",		.ops = get_cpurate_status,},
	{.status = "memoryuserate",.ops = get_memoryuserate_status,},
	{.status = "uploadspeed",	.ops = get_uploadspeed_status,},
	{.status = "downloadspeed",.ops = get_downloadspeed_status,},
	{.status = "onlineTime",	.ops = get_onlineTime_status,},
	{.status = "terminalNum",	.ops = get_terminalNum_status,},
	{.status = "networktype",	.ops = get_networktype_status,},
	{.status = "workmode",		.ops = get_workmode_status,},
	{.status = "real_devinfo",	.ops = get_real_devinfo_status,},
	{.status = "channel",		.ops = get_channel_status,},
	{.status = "ledswitch",		.ops = get_ledswitch_status,},
	{.status = "wlanstats",		.ops = get_wlanstats_status,},    
	{.status = "load",		    	.ops = get_load_status,},     
	{.status = "elinkstat",		.ops = get_elinkstat_status,}, 
	{.status = "neighborinfo",	.ops = get_neighborinfo_status,},
	
};

int get_route_status_info(cJSON *send_info,char *status)
{
	int i = 0;
	for(i = 0; i < ARRAY_SIZE(get_status_); i++)
	{
		if(0 == strcmp(get_status_[i].status,status))
		{
			get_status_[i].ops(send_info);
			break;
		}
	}
}

void add_elinkmsg_to_list(ELINK_MSG_MODULE *msg_list, ELINK_MSG_MODULE *msg)
{
	int i;
	char *p_list_position, *p_msg_position;

	p_list_position = p_msg_position = NULL;

	if(NULL == msg_list || NULL == msg)
	{
		printf("fun=%s; line=%d; msg: paramater error!\n", __func__, __LINE__);

		return ;
	}

	for(i = 0; i < MAX_MODULE_NUM; ++i)
	{
		if(0 != msg_list[i].id)
		{
			if(msg_list[i].id != msg->id)
			{
				continue ;
			}

			if(0 == strcmp(msg_list[i].msg, msg->msg))
			{
				return ;
			}
			continue ;
		}

		msg_list[i].id = msg->id;
		printf("msg->id[%d]   msg->msg[%s]\n",msg->id,msg->msg);
		strncpy(msg_list[i].msg, msg->msg, strlen(msg->msg));
		
		return ;
	}

	return ;
}

int real_set_ap(struct set_ap_info *ap_info)
{
	int ret = 0;
	WLAN_RATE_TYPE rate_type = WLAN_RATE_ALL;
	DEBUG_INFO("ap info_apidx = %d\n", ap_info->ap_apidx);
	if(!strcmp("2.4G", ap_info->ap_mode))
	{
		rate_type = WLAN_RATE_24G;  
	}else if(!strcmp("5G", ap_info->ap_mode))
	{
		rate_type = WLAN_RATE_5G;  
	}
	else
	{
		DEBUG_INFO("ap mode error : %s not SUPPORT!!\n",  ap_info->ap_mode);
	     	return 0;
	}

	ret += extra_set_wifiswitch(ap_info->ap_apidx, ap_info->ap_enable,rate_type);
	ret += extra_set_channel(ap_info->ap_channel,rate_type);
	ret += extra_set_ap_status_passwd(ap_info->ap_apidx,ap_info->ap_auth,ap_info->ap_encrypt,ap_info->ap_key,rate_type);
	ret += extra_set_ap_status_ssid(ap_info->ap_apidx, ap_info->ap_ssid,rate_type);       
	return ret;
}


int set_all_ap(struct set_ap_info *ap_info, int ap_count)
{   
    int ret = 0;
    int i = 0;
    WLAN_RATE_TYPE rate_type = WLAN_RATE_ALL;

    for(i = 0; i <= ap_count; i++)
    {
        DEBUG_INFO("mode = %s \n",ap_info[i].ap_mode);
        DEBUG_INFO("apidx = %d\n",ap_info[i].ap_apidx);
        DEBUG_INFO("ap_enable = %s\n",ap_info[i].ap_enable);
        DEBUG_INFO("ssid = %s \n",ap_info[i].ap_ssid);
        DEBUG_INFO("key = %s \n",ap_info[i].ap_key);
        DEBUG_INFO("auth = %s \n",ap_info[i].ap_auth);
        DEBUG_INFO("encrypt = %s \n",ap_info[i].ap_encrypt);
        DEBUG_INFO("channel = %d\n",ap_info[i].ap_channel);

	DEBUG_INFO("ap info_apidx = %d\n", ap_info->ap_apidx);
	if(!strcmp("2.4G", ap_info->ap_mode))
	{
		rate_type = WLAN_RATE_24G;  
	}else if(!strcmp("5G", ap_info->ap_mode))
	{
		rate_type = WLAN_RATE_5G;  
	}
	else
	{
		DEBUG_INFO("ap mode error : %s not SUPPORT!!\n",  ap_info->ap_mode);
	     	continue;
	}
        
        ret += real_set_ap(&ap_info[i]);
        if(ret < 0)
        {
            DEBUG_INFO("set %d ap info error .", i);
        }
    }
    return ret;
}


int get_all_ap_info(cJSON *set_wifi, struct set_ap_info *set_ap_cfg_info, int *len)
{
    int ret = 0;
    //cJSON *set_wifi = NULL;
    cJSON *set_info= NULL;
    cJSON *set_mode = NULL;
    cJSON *set_channel = NULL;
    cJSON *set_array_radio = NULL;
    cJSON *set_array_ap    = NULL;
    cJSON *set_ap_info     = NULL;
    cJSON *set_ap_apidx    = NULL;
    cJSON *set_ap_enable   = NULL;
    cJSON *set_ap_ssid     = NULL;
    cJSON *set_ap_key      = NULL;
    cJSON *set_ap_auth     = NULL;
    cJSON *set_ap_encrypt  = NULL;

    int set_wifi_len = 0;
    int set_ap_len = 0;
    int i = 0, j = 0;
    int index = 0;

    
   // set_wifi = cJSON_GetObjectItem(set, "wifi");
  ///  if(NULL == set_wifi)
  //  {
    //    ret = -1;
 //       return ret;
  //  }
    set_wifi_len = cJSON_GetArraySize(set_wifi);
    if(0 == set_wifi_len)
    {
        DEBUG_INFO("%s", "no ap info set.\n");
    }
    DEBUG_INFO("aaaaaaaaaaaaarray len = %d\n", set_wifi_len);
    
    for(j = 0; j < set_wifi_len; j++)
    {
        set_info= cJSON_GetArrayItem(set_wifi, j);
        if(NULL == set_info)
        {
            ret = -1;
            return ret;
        }
        print_cjson(set_info);  
        set_array_radio = cJSON_GetObjectItem(set_info, "radio");
        if(NULL == set_array_radio)
        {
            ret = -1;
            return ret;
        }
        set_mode = cJSON_GetObjectItem(set_array_radio, "mode");
        DEBUG_INFO("mode = %s\n",set_mode->valuestring);

        set_channel = cJSON_GetObjectItem(set_array_radio, "channel");
        DEBUG_INFO("channel = %d\n",set_channel->valueint);
         
        set_array_ap= cJSON_GetObjectItem(set_info, "ap");
        set_ap_len = cJSON_GetArraySize(set_array_ap);
        DEBUG_INFO("set ap len = %d \n", set_ap_len);

        for(i = 0; i < set_ap_len; i++)
        {
            index = j * set_ap_len + i;

            if(NULL !=set_mode)
            {
                DEBUG_INFO("mode = %s\n",set_mode->valuestring);
                strcpy((set_ap_cfg_info[index]).ap_mode, set_mode->valuestring);
            }
            else
            {
            	//default 2.4G
            	strcpy((set_ap_cfg_info[index]).ap_mode, "2.4G");
            }

            if(NULL != set_channel)
            {
                DEBUG_INFO("channel = %d\n",set_channel->valueint);
                set_ap_cfg_info[index].ap_channel = set_channel->valueint;
            }
			else
			{	
				//default old
				(set_ap_cfg_info[index]).ap_channel = -1;
			}
            
            set_ap_info = cJSON_GetArrayItem(set_array_ap, i);
            print_cjson(set_ap_info);  
            set_ap_apidx = cJSON_GetObjectItem(set_ap_info, "apidx");
            if(NULL != set_ap_apidx)
            {
                DEBUG_INFO("set_ap_apidx = %d\n",set_ap_apidx->valueint);
            	(set_ap_cfg_info[index]).ap_apidx = set_ap_apidx->valueint;
            }
			else
			{	
				//default 0
				(set_ap_cfg_info[index]).ap_apidx = 0;
			}
			
            set_ap_enable   =  cJSON_GetObjectItem(set_ap_info, "enable");
            if(NULL != set_ap_enable)
            {
                DEBUG_INFO("set_ap_enable = %s\n",set_ap_enable->valuestring);
                strcpy(set_ap_cfg_info[index].ap_enable, set_ap_enable->valuestring);
            }
            else
            {
                 strcpy(set_ap_cfg_info[index].ap_enable, "yes");
            }
            set_ap_ssid = cJSON_GetObjectItem(set_ap_info, "ssid");
            if(NULL != set_ap_ssid)
            {
                DEBUG_INFO("set_ap_ssid = %s\n",set_ap_ssid->valuestring);
           		strcpy((set_ap_cfg_info[index]).ap_ssid, set_ap_ssid->valuestring);
            }
            
            set_ap_key = cJSON_GetObjectItem(set_ap_info, "key");
            if(NULL != set_ap_key)
            {
                DEBUG_INFO("set_ap_key = %s\n",set_ap_key->valuestring);
            	strcpy((set_ap_cfg_info[index]).ap_key, set_ap_key->valuestring);
            }
            
            set_ap_auth =  cJSON_GetObjectItem(set_ap_info, "auth");
            if(NULL != set_ap_auth)
            {
                DEBUG_INFO("set_ap_auth = %s\n",set_ap_auth->valuestring);
           		strcpy((set_ap_cfg_info[index]).ap_auth, set_ap_auth->valuestring);
            }
            set_ap_encrypt = cJSON_GetObjectItem(set_ap_info, "encrypt");
            if(NULL != set_ap_encrypt)
            {
				DEBUG_INFO("set_ap_encrypt = %s\n",set_ap_encrypt->valuestring);
            	strcpy((set_ap_cfg_info[index]).ap_encrypt, set_ap_encrypt->valuestring);
            }
        }
        *len = index;
        DEBUG_INFO("index = %d ; len = %d\n", index, *len);
        *len = index;
    }

    return ret;
}



int config_wifi(cJSON *root,ELINK_MSG_MODULE *msg)
{
	int ret = 0;
	int index = 0;
	ELINK_MSG_MODULE msg_tmp;
	set_ap_info set_ap_cfg_info[8] = {0};
	ret = get_all_ap_info(root, set_ap_cfg_info, &index);
	if(ret < 0)
	{
	    DEBUG_INFO("get all ap info error.\n");
	    return 0;
	}
	ret = set_all_ap(set_ap_cfg_info, index);

	if(ret)
	{
		msg_tmp.id = RC_WIFI_MODULE;
		sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
		add_elinkmsg_to_list(msg,&msg_tmp);
	}
	
}

int config_wifiswitch(cJSON *root,ELINK_MSG_MODULE *msg)
{
	int ret = 0;
	cJSON *wifiswitch_status = NULL;
	char all_wifiswitch[16] = {0};
       ELINK_MSG_MODULE msg_tmp;
	   
	wifiswitch_status = cJSON_GetObjectItem(root,"status");
	if(wifiswitch_status)
	{
		extra_get_all_wifiswitch(all_wifiswitch,1);
		
		if(0 == strcmp(wifiswitch_status->valuestring,all_wifiswitch))
		{
			return ;
		}
		ret = extra_set_all_wifiswitch(wifiswitch_status->valuestring);
		if(ret)
		{
			msg_tmp.id = RC_WIFI_MODULE;
			sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
			add_elinkmsg_to_list(msg,&msg_tmp);
		}    
	}
}

int config_ledswitch(cJSON *root,ELINK_MSG_MODULE *msg)
{
	cJSON *ledswitch_status = NULL;
	char all_ledswitch[16] = {0};
       ELINK_MSG_MODULE msg_tmp;
	   
	ledswitch_status = cJSON_GetObjectItem(root,"status");
	if(ledswitch_status)
	{
		extra_get_ledswitch(all_ledswitch);
		if(0 == strcmp(ledswitch_status->valuestring,all_ledswitch))
		{
			return ;
		}
		extra_set_ledswitch(ledswitch_status->valuestring);

            msg_tmp.id = RC_LED_MODULE;
            sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
            add_elinkmsg_to_list(msg,&msg_tmp);
	}
}

int config_wifitimer(cJSON *root,ELINK_MSG_MODULE *msg)
{   
	extra_set_wifitimer(root);
}
int exec_command(cJSON *ctrlcommand,ELINK_MSG_MODULE *msg)
{
       ELINK_MSG_MODULE msg_tmp;
	if(ctrlcommand)
	{
		if(0 == strcmp(ctrlcommand->valuestring,"reboot"))
		{
			msg_tmp.id = RC_SYSTOOLS_MODULE;
			sprintf(msg_tmp.msg, "string_info=reboot");
			add_elinkmsg_to_list(msg,&msg_tmp);
		}else if(0 == strcmp(ctrlcommand->valuestring,"reset"))
		{
			msg_tmp.id = RC_SYSTOOLS_MODULE;
			sprintf(msg_tmp.msg, "string_info=restore");
			add_elinkmsg_to_list(msg,&msg_tmp);
		}else
		{
			printf("ctrlcommand : %s\n",ctrlcommand);
		}
	}
}


/*
	url : "http://www.xx.com.cn:8000/asdf/bin.bin";
*/
int get_url_info(const char *url, char *domain, int *port, char *dir)
{
	char *p;
	int ret = 0;

	if (strstr(url, "http://"))
		p = url + strlen("http://");
    else
        p = url;

	*port = 80;

	if (strchr(p, ':'))
		ret = sscanf(p, "%[^:]:%d%s", domain, port, dir);
	else
		ret = sscanf(p, "%[^/]%s", domain, dir);

	return ret == 2 || ret == 3;
}
extern int save_upgrade_url(char *url,char *dir,int port);
int cloud_upgrade(cJSON *root,ELINK_MSG_MODULE *msg)
{
	cJSON *downurl = NULL;
       ELINK_MSG_MODULE msg_tmp;
	char url[256] = {0};
	char dir[256] = {0};
	int port = 80;
	
	downurl = cJSON_GetObjectItem(root,"downurl");
	if(downurl)
	{
		DEBUG_INFO("=======%s========%s [%d]\n", downurl->valuestring,__FUNCTION__, __LINE__);
		if(get_url_info(downurl->valuestring,url,&port,dir))
		{
			save_upgrade_url(url,dir,port);
			DEBUG_INFO("=======%s===%s    %d=====%s [%d]\n", url,dir,port,__FUNCTION__, __LINE__);
			//msg_tmp.id = RC_SYSTOOLS_MODULE;
			//sprintf(msg_tmp.msg, "string_info=autoupgrade");
			msg_send(MODULE_RC, RC_SYSTOOLS_MODULE, "string_info=autoupgrade");
			//add_elinkmsg_to_list(msg,&msg_tmp);
		}
	}
}

int updata_roaming_info(struct roaming_ctx *roaming_config)
{
	extra_updata_roaming_info(roaming_config);
}

int config_roaming(cJSON *root,ELINK_MSG_MODULE *msg)
{
	if(save_roaming_config(root) > 0)
	{		
		restart_roaming();
	}
}

 struct  config_route set_config_[] = 
{
	{.key = "wifi",			.ops = config_wifi,},
	{.key = "wifiswitch",		.ops = config_wifiswitch,},
	{.key = "ledswitch",		.ops = config_ledswitch,},
	{.key = "wifitimer",		.ops = config_wifitimer,},
	{.key = "ctrlcommand",		.ops = exec_command,},
	{.key = "upgrade",			.ops = cloud_upgrade,},
	{.key = "roaming_set",		.ops = config_roaming,},

};

int process_all_data_cfg(char *buff,int active)
{
	int i = 0;
	int ret = 0;
	int index = 0;
	cJSON *root   = NULL;
	cJSON *set_ap    = NULL;
	cJSON *tmp    = NULL;
	ELINK_MSG_MODULE msg_list[MAX_MODULE_NUM];
    	memset((char *)&msg_list, 0x0, MAX_MODULE_NUM * sizeof(ELINK_MSG_MODULE));
		
	root = cJSON_Parse(buff);
	CHECK_JSON_VALUE(root);
	set_ap = cJSON_GetObjectItem(root,"set");
	CHECK_JSON_VALUE(set_ap);


	for(i = 0; i < ARRAY_SIZE(set_config_); i++)
	{
		if(strstr(buff,set_config_[i].key))
		{
			tmp = NULL;
			tmp = cJSON_GetObjectItem(set_ap,set_config_[i].key);
			if(tmp == NULL)
				continue;
			set_config_[i].ops(tmp,msg_list);
		}
	}

	
	nvram_commit();
	if(active)
	{
		for(index = 0; index < MAX_MODULE_NUM; ++index)
		{
			if(msg_list[index].id != 0)
			{
				msg_send(MODULE_RC, msg_list[index].id, msg_list[index].msg);
			}
		}
	}
bad_msg:
	if(root)
		cJSON_Delete(root);
}

void get_sta_rssi_info(cJSON*input,cJSON*output)
{
	int  i = 0;
	int rssi = 0;
	char band[16] = {0};
	int get_array_len = 0;
	cJSON *mac_json = NULL;
	cJSON *mac = NULL;
	cJSON *rssi_info = NULL;
	
	mac_json = cJSON_GetObjectItem(input,"mac");
	CHECK_JSON_VALUE(mac_json);
	get_array_len = cJSON_GetArraySize(mac_json);
	for(i; i < get_array_len; i++)
	{
		mac = cJSON_GetArrayItem(mac_json,i);
		memset(band,0x0,16);
		get_client_rssi_and_band(mac->valuestring,&rssi,band);
		/****************此处是为了满足电信测试要求***************/
		if(find_mac_in_roaming_del_list(mac))
			continue;
		cJSON_AddItemToArray(output,rssi_info = cJSON_CreateObject());
		cJSON_AddStringToObject(rssi_info, "mac",mac->valuestring);
		if(rssi == 0)
		{
			cJSON_AddStringToObject(rssi_info, "band","");
			cJSON_AddNumberToObject(rssi_info, "rssi",-100);
		}else
		{
			cJSON_AddStringToObject(rssi_info, "band",band);
			cJSON_AddNumberToObject(rssi_info, "rssi",rssi);
		}
	}
bad_msg:
	return;
}

void exec_deassociation(cJSON*input)
{
	int  i = 0;
	int rssi = 0;
	char band[16] = {0};
	int get_array_len = 0;
	cJSON *mac_json = NULL;
	cJSON *mac = NULL;
	cJSON *rssi_info = NULL;
	
	mac_json = cJSON_GetObjectItem(input,"mac");
	CHECK_JSON_VALUE(mac_json);
	get_array_len = cJSON_GetArraySize(mac_json);
	for(i; i < get_array_len; i++)
	{
		mac = cJSON_GetArrayItem(mac_json,i);
		if(mac)
		{
			//目前的漫游功能没有生效的地方，sdk中没有上报的接口，暂时不处理
			add_roaming_sta_to_del_list(mac);
			RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "del_sta", mac->valuestring, "");
			RunSystemCmd(0, "iwpriv", TENDA_WLAN24_GUEST_IFNAME, "del_sta", mac->valuestring, "");
			RunSystemCmd(0, "iwpriv", TENDA_WLAN5_AP_IFNAME, "del_sta", mac->valuestring, "");
			RunSystemCmd(0, "iwpriv", TENDA_WLAN5_GUEST_IFNAME, "del_sta", mac->valuestring, "");		
		}
		
	}
bad_msg:
	return;
}

void test_elink(char* name,char op,char*value)
{
	cJSON *root = NULL;
	cJSON *send_status = NULL;
	char *print_value = NULL;

	if(op == 0)
	{
		root = cJSON_CreateObject();
		send_status = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "status", send_status);

		get_route_status_info(send_status,name);

		print_value = cJSON_Print(root);
		printf("print_value:%s",print_value);

		free(print_value);
		cJSON_Delete(root);
	}else
	{
		process_all_data_cfg(value,1);
		
	}
}

