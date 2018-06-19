#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h>

#include <bcmnvram.h>
#include "flash_cgi.h"
#include "route_cfg.h"
#include "../include/alinkgw_api.h"

#define MIN_ETHADDR_LEN				17
#define MAX_PROBE_INFO_NUM		500
#define TIME_1S		 100

struct wlan_security_info{
	char mac[32];
	int is_reported;
};
struct wlan_security_info wlan_probe_info[500];
int next_send_index = 0;
unsigned int wlan_probe_num = 0;
int gProbeInfoDebug = 0;
int gProbed_switch = 1;//此属性为无线路由器扫描监测功能的开关，默认开启

void show_probe_info()
{
	int i;
	printf("#####show probe info#####\n");
	printf("reported count is %u\n",wlan_probe_num);
	for( i = 0 ; i < wlan_probe_num; i++)
	{
		printf("%d\t%s\n",i+1, wlan_probe_info[i].mac);
	}
	printf("\n");
}

void probe_info_debug_level(int level)
{
    if (level > 0)
        gProbeInfoDebug = 1;
    else
        gProbeInfoDebug = 0;

    printf("probe_info debug level = %d\n", gProbeInfoDebug);
}

int set_probe_info(const char *jsonin)
{
	return ALINKGW_OK;
}

int get_probe_info(char *buf, unsigned int buff_len)
{
	int len = 0;
	int i = 0, index = 0;
	char str[128] = {0};
	int found = 0;
	
	if(gProbed_switch == 0)
		return ALINKGW_OK;
	
	if(NULL == buf)
		return ALINKGW_ERR;
	
	for( i = 0 ; i < MAX_PROBE_INFO_NUM; i++)
	{		
		if(strlen(wlan_probe_info[i].mac) < MIN_ETHADDR_LEN)
		{
			break;
		}
		if(wlan_probe_info[i].is_reported == 0 &&strlen(wlan_probe_info[i].mac) >= MIN_ETHADDR_LEN)
		{
			index = i;
			found = 1;
			break;	
		}
	}

	if(0 == found)
	{
		if(gProbeInfoDebug)
			printf("[debug] not find .\n");
		return ALINKGW_OK;
		
	}
	len = sprintf(str,"%s",wlan_probe_info[index].mac);
	if(gProbeInfoDebug)
		printf("[debug] find sta, index %d mac is [%s]\n",index,str);
	if( len >= buff_len)
	{
		return ALINKGW_BUFFER_INSUFFICENT;
	}
	memcpy(buf, str, len+1);
	wlan_probe_info[i].is_reported = 1;
	wlan_probe_num ++;
	return ALINKGW_OK;
	
}

int set_probe_num(const char *jsonin)
{
	return ALINKGW_OK;
}

int get_probe_num(char *buf, unsigned int buff_len)
{
	int len = 0;
	char str[32] = {0};
	
//	if(gProbed_switch == 0)
//		return ALINKGW_OK;
	
	if(NULL == buf)
		return ALINKGW_ERR;

	len = sprintf(str,"%u",wlan_probe_num);

	if(len >= buff_len)
		return ALINKGW_BUFFER_INSUFFICENT;
	memcpy(buf, str, len+1);
	if(gProbeInfoDebug)
		printf("[debug] wlan_probe_num:%u\n",wlan_probe_num);
	return ALINKGW_OK;
}

void check_alilink_probe_info() 
{
	int i = 0, index = 0;
	const char *attr[] = { ALINKGW_ATTR_PROBE_INFO, ALINKGW_ATTR_PROBE_NUMBER, NULL};
	int found = 0;
	int ret;

	if(gProbed_switch == 0)
		return ;
	
	for( i = 0 ; i < MAX_PROBE_INFO_NUM; i++)
	{		
		if(strlen(wlan_probe_info[i].mac) < MIN_ETHADDR_LEN)
		{
			break;
		}
		if(wlan_probe_info[i].is_reported == 0 &&strlen(wlan_probe_info[i].mac) >= MIN_ETHADDR_LEN)
		{
			index = i;
			found = 1;
			break;	
		}
	}
        
	if(0 == found)
	{
		if(gProbeInfoDebug)
			printf("[debug] not find no reported sta.return.\n");
		
	}
	else if(1 == found)
	{
		ret = ALINKGW_report_attrs(attr);	
		if(ret != ALINKGW_OK)
		{
			if(gProbeInfoDebug)
				printf("[sample] [%s], report attibutes(%s) failed\n", __func__,ALINKGW_ATTR_PROBE_INFO);
		 	
		}
		
	}
}
#define ALILINK_PROBED_SWITCH			"alilink_probed_switch"
int set_probed_switch_state(const char *json)
{
	printf("%s JSON:%s\n",__func__, json);
	if(json == NULL)
		return ALINKGW_ERR;
	
	if(strlen(json) != 1)
		return ALINKGW_ERR;
	
	if(strcmp(json, "1") == 0)
		gProbed_switch = 1;
	else if(strcmp(json, "0") == 0)
		gProbed_switch = 0;
	else
		return ALINKGW_ERR;
	
	_SET_VALUE( ALILINK_PROBED_SWITCH,  json);
	_COMMIT();
	return ALINKGW_OK;
}

int get_probed_switch_state(char *buf, unsigned int buff_len)
{
	printf("%s %d buf:%s buff_len:%u\n",__func__,__LINE__,buf, buff_len);
	if(buf == NULL)
		return ALINKGW_ERR;
	
	if(buff_len < 2) //bool类型至少需2个空间
		return ALINKGW_BUFFER_INSUFFICENT;
	
	sprintf(buf  , "%d" , gProbed_switch);

	printf("return buf:%s len:%d \n",buf, strlen(buf));
	return ALINKGW_OK;
}
