#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <bcmnvram.h>
#include "flash_cgi.h"
#include "../include/alinkgw_api.h"
#include "cJSON.h"

#define SCH_ON_TIME				0
#define SCH_OFF_TIME			1
#define SCH_TIME_NUM			2

#define ALILINK_WLAN_SCHE_MAX	10

#define SCH_NULL_VAR	"{\"enabled\":\"0\", \"offTime\":[],\"onTime\":[]}"
#define CJSON_STA_FMT	"\"enabled\":\"%d\""
#define CJSON_ONTIME_FMT	"\"onTime\":[%s]"
#define CJSON_OFFTIME_FMT	"\"offTime\":[%s]"

int g_isWlanSwSchdChange = 0;
extern int g_cur_wl_radio_status;
#define WL_RADIO_ON		1
#define WL_RADIO_OFF		0
extern int gWifiStatusConfig;
extern int gWifiSchedDebug;

extern int wl_restart_check_main_loop();

int get_wlan_switch_scheduler(char *buff, unsigned int buff_len)
{
        
	char mib_name[64] = {0};
	char *mib_val;
	char time[SCH_TIME_NUM][512];
	char *p1,*p2;
	
	char buff_tmp[1024]={0};
	int i,num;
	int n1 = 0, n2=0;

	if(NULL == buff)
		return ALINKGW_ERR;	
		
	_GET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, mib_val);
	if( atoi(mib_val) > ALILINK_WLAN_SCHE_MAX || atoi(mib_val) == 0)
	{
		strcpy(buff_tmp, SCH_NULL_VAR);
		if(buff_len < strlen(buff_tmp) + 1)
		{
	      		 return ALINKGW_BUFFER_INSUFFICENT;
		}
		strncpy(buff,SCH_NULL_VAR,buff_len);
		return ALINKGW_OK;	
	}
	
	num = atoi(mib_val);	
	p1 = time[SCH_ON_TIME];
	p2 = time[SCH_OFF_TIME];
	memset(time,0, SCH_TIME_NUM*512);
	for(i = 1; i <= num; i++)
	{
	        
		sprintf(mib_name,ALILINK_WLAN_SCHE_ON_LIST,i);
		_GET_VALUE(mib_name, mib_val);
				
		n1 += sprintf(p1+n1, "\"%s\",", mib_val);
		sprintf(mib_name,ALILINK_WLAN_SCHE_OFF_LIST,i);
		_GET_VALUE(mib_name, mib_val);
			
		n2 += sprintf(p2+n2, "\"%s\",", mib_val);
		
	}
	
	p1[n1-1] = '\0';
	p2[n2-1] = '\0';
	_GET_VALUE(WLN0_CTL_ENABLE, mib_val);
	if(atoi(mib_val) == 0)
	{
		printf("schedule disable !\n");
		strcpy(buff_tmp, SCH_NULL_VAR);
		if(buff_len < strlen(buff_tmp) + 1)
		{
	      		 return ALINKGW_BUFFER_INSUFFICENT;
		}
		strncpy(buff,SCH_NULL_VAR,buff_len);
		return ALINKGW_OK;	
	}

	sprintf(buff_tmp,"{" CJSON_STA_FMT "," CJSON_ONTIME_FMT "," CJSON_OFFTIME_FMT "}",
			atoi(mib_val)==1?1:0,time[SCH_ON_TIME],time[SCH_OFF_TIME]);
	if(buff_len < strlen(buff_tmp) + 1)
	{
      		 return ALINKGW_BUFFER_INSUFFICENT;
	}
	strncpy(buff,buff_tmp,buff_len);
	if(gWifiSchedDebug)
		printf("[debug] func=%s buff:%s\n",__func__,buff);

    return ALINKGW_OK;
}

#define TIME_SEC_MIN		1 //秒、分 60进制
#define TIME_HOUR			2//时，24进制
int checkPerTime(char *time_str, int time_type)
{
	if(time_str == NULL)
		return ALINKGW_ERR;
	
	int i = 0;
	int time = 0,max_time = 0, min_time = 0,max_len = 0, min_len = 0;
	int len = strlen(time_str);
	
	if(TIME_SEC_MIN == time_type)
	{
		max_len = 2;
		min_len = 1;
		max_time = 60 -1;
		min_time = 0;
	}
	else if(TIME_HOUR == time_type)
	{
		max_len = 2;
		min_len = 1;
		max_time = 24 -1;
		min_time = 0;
	}
	
	if(len <min_len || len >max_len)//检验字符串格式长度
		return ALINKGW_ERR;
	for(i=0; i<len; i++)
	{
		if(time_str[i] < '0' || time_str[i] > '9')//检验是否整形
			return ALINKGW_ERR;
	}
	
	time = atoi(time_str);//取整后再比较范围
	if(time < min_time || time >max_time)
		return ALINKGW_ERR;

	return ALINKGW_OK;
	
}

int checkweek(char *week_str)
{
	char *ptr;
	int errCount = 0;
	if(NULL == week_str)
		return ALINKGW_ERR;
	//星期格式中带- 的话，只支持X-X 格式
	if(strchr(week_str,'-'))
	{
		if(strlen(week_str) != 3)
		{
			printf("week whith -,but len err, %s ,len is %d\n",week_str,strlen(week_str));
			return ALINKGW_ERR;
		}
		if(week_str[0] < '1' || week_str[0] > '7' ||week_str[2] < '1' || week_str[2] > '7' ||week_str[1] != '-' )
		{
			printf("week whith -,value err, week is %s\n",week_str);
			return ALINKGW_ERR;
		}
		if(week_str[0] >= week_str[2])
		{
			printf("week whith -,value err, week is %s\n",week_str);
			return ALINKGW_ERR;
		}
		return ALINKGW_OK;		
	}
	
	ptr = week_str; 
	while( *ptr!= '\0')
	{
		if(errCount)
			break;
		
		switch(*ptr)
		{	
			case ',':
				break;
			case '-':
				break;
			case '8':
			case '9':
			case '0':
				printf("illegal character %c\n",*ptr);
				errCount++;
				break;
			default:
				if(*ptr >= '1' && *ptr <= '7')
				{
					if(atoi(ptr) != (*ptr-0x30))
					{
						printf("atoi err. %d\n",atoi(ptr));
						errCount++;
					}
				}
				else
				{
					printf("illegal character %c\n",*ptr);
					errCount++;
				}
				
				break;
		}
		ptr++;
	}
	if(errCount)
		return ALINKGW_ERR;

	return ALINKGW_OK;	
}
int checkInputTime(char *time)
{
	char sec_str[64] = {0};
	char min_str[64] = {0};
	char hour_str[64] = {0};
	char week_str[64] = {0};
	int ret = 0;
	if(time == NULL)
		return ALINKGW_ERR;
	/*UTC+08:00 0 0 22 ? * 1,2,3,4,5,6,7*/
	if(strstr(time, "UTC+08:00"))
		ret = sscanf(time, "UTC+08:00 %[^ ] %[^ ] %[^ ] %*[^ ] %*[^ ] %[^\s]",sec_str, min_str, hour_str, week_str);
	else
	{
		printf("time err.\n");
		return ALINKGW_ERR;
	}
	
	if(ret != 4)
	{
		printf("sscanf error.\n");
		return ALINKGW_ERR;
	}

	if( checkPerTime(sec_str, TIME_SEC_MIN) != ALINKGW_OK)
	{
		printf("sec err .\n");
		return ALINKGW_ERR;
	}
	if( checkPerTime(min_str, TIME_SEC_MIN) != ALINKGW_OK)
	{
		printf("min err .\n");
		return ALINKGW_ERR;
	}
	if( checkPerTime(hour_str, TIME_HOUR) != ALINKGW_OK)
	{
		printf("hour err .\n");
		return ALINKGW_ERR;
	}

	if( checkweek(week_str) != ALINKGW_OK)
	{
		printf("week err .\n");
		return ALINKGW_ERR;
	}
	
	return ALINKGW_OK;	
}

int set_wlan_switch_scheduler(const char *json_in)
{    
	/*
	{\"enabled\":\"1\",
	\"offTime\":[\"UTC+08:00 0 0 22 ? * 1-5\",\"UTC+08:00 0 0 23 ? * 6-7\"],
	\"onTime\":[\"UTC+08:00 0 0 8 ? * 1-5\",\"UTC+08:00 0 0 10 ? * 6-7\"]}
	*/

	char status[8] = {0};
	cJSON* pArrayItem = NULL;
	int i = 0;
	int err_flag = 0;
	char buf[64] = {0};
	
	if(json_in == NULL)
		return ALINKGW_ERR;
	
	if(gWifiSchedDebug)
      		printf("[sample] [%s], json_in = %s  len:%d\n", __func__, json_in,strlen(json_in));
	cJSON * pJson = cJSON_Parse(json_in);
	if(NULL == pJson)
	{
		printf("cJSON_Parse error.\n");
		return ALINKGW_ERR;
	}
//get enabled
	cJSON * pSub = cJSON_GetObjectItem(pJson, "enabled");
	if(NULL == pSub)
	{
		printf("Without enable \n");
		goto ERR;
	}
//	printf("enable %p %s %d\n",pSub->valuestring,pSub->valuestring,strlen(pSub->valuestring));
	
	if(pSub->valuestring ==NULL)
	{
		printf("enable err.\n");
		goto ERR;
	}
	if( strlen(pSub->valuestring) != 1)
	{
		printf("enable len err,len=%d \n",strlen(pSub->valuestring));
		goto ERR;
	}
	strncpy(status, pSub->valuestring,strlen(pSub->valuestring));

//get offTime and onTime
	
	cJSON *pArray_off = cJSON_GetObjectItem ( pJson, "offTime" );
	cJSON *pArray_on = cJSON_GetObjectItem ( pJson, "onTime" );
	if( NULL == pArray_off || NULL == pArray_on)
	{
		printf("Without offTime or onTime.\n");
		goto ERR;
	}
	int nCount_off = cJSON_GetArraySize ( pArray_off ); //获取pArray数组的大小
	int nCount_on = cJSON_GetArraySize ( pArray_on );
	if(nCount_off < 0 || nCount_on< 0 || nCount_off != nCount_on)//个数必须成对出现
	{
		printf("count err,%d %d\n",nCount_off, nCount_on);
		goto ERR;
	}
	if(nCount_off > ALILINK_WLAN_SCHE_MAX)
	{
		printf("count err,max count is 10,now count %d\n",nCount_off);
		goto ERR;
	}

	if(strcmp(status, "1") == 0)
	{
		_SET_VALUE(WLN0_CTL_ENABLE, "1");
		
		for( i = 0; i < nCount_off; i++)
		{
			pArrayItem = cJSON_GetArrayItem(pArray_off, i);
			if(pArrayItem && pArrayItem->valuestring)
			{
				if(gWifiSchedDebug)
					printf( "offtime%d: %s len:%d \n",i+1, pArrayItem->valuestring, strlen(pArrayItem->valuestring));
				if(checkInputTime(pArrayItem->valuestring) !=ALINKGW_OK)
				{
					err_flag = 1; 
					break;
				}
				memset(buf, 0, sizeof(buf));
				sprintf(buf, ALILINK_WLAN_SCHE_OFF_LIST, i+1);
				_SET_VALUE(buf, pArrayItem->valuestring);
			}
			else
			{
				err_flag = 1; //防错处理，不会进来
				break;
			}
			
			pArrayItem = cJSON_GetArrayItem(pArray_on, i);
			if(pArrayItem && pArrayItem->valuestring)
			{
				if(gWifiSchedDebug)
					printf( "ontime%d: %s len:%d \n",i+1, pArrayItem->valuestring, strlen(pArrayItem->valuestring));
				if(checkInputTime(pArrayItem->valuestring) !=ALINKGW_OK)
				{
					err_flag = 1; 
					break;
				}				
				memset(buf, 0, sizeof(buf));
				sprintf(buf, ALILINK_WLAN_SCHE_ON_LIST, i+1);
				_SET_VALUE(buf, pArrayItem->valuestring);
			}
			else
			{
				err_flag = 1; //防错处理，不会进来
				break;
			}
				
		 }
		
		if(err_flag)
		{
			printf("check err. return err.\n");
			_SET_VALUE(WLN0_CTL_ENABLE, "0");
			_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "0");
			_COMMIT();
			gWifiStatusConfig = 0;
			wl_restart_check_main_loop();
			goto ERR;
		}
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", nCount_off);
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, buf);
		
		if( 0 == nCount_off )
			_SET_VALUE(WLN0_CTL_ENABLE, "0");
	}
	else if(strcmp(status, "0") == 0)
	{
		_SET_VALUE(WLN0_CTL_ENABLE, "0");
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "0");
	}
	else
	{
		printf("enable err, %s\n",status);
		goto ERR;
	}
	
	_COMMIT();
	gWifiStatusConfig = 0;
	g_isWlanSwSchdChange = 1;
	
	wl_restart_check_main_loop();
	if(pJson)
		cJSON_Delete(pJson);
	return ALINKGW_OK;
	
ERR:
	printf("err, return.\n");
	if(pJson)
		cJSON_Delete(pJson);
	return ALINKGW_ERR;
	
}

int report_wlanswitchscheduler_state()
{
	
	int ret = ALINKGW_report_attr(ALINKGW_ATTR_WLAN_SWITCH_SCHEDULER);
   	if(ret != ALINKGW_OK)
   	{
   	   if(gWifiSchedDebug)
	  	 printf("[sample] [%s], report attibute(%s) failed\n", __func__, ALINKGW_ATTR_WLAN_SWITCH_SCHEDULER);
	   return ret;
   	}
	if(gWifiSchedDebug)
   		printf("[sample] [%s], report attibute(%s) success\n", __func__, ALINKGW_ATTR_WLAN_SWITCH_SCHEDULER);
  	return ALINKGW_OK;
}

int check_wlanswitchscheduler_state()
{
	if(g_isWlanSwSchdChange)
	{
		g_isWlanSwSchdChange = 0;
		return 1;
	}
	return 0;
}
