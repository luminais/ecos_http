#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi_lib.h"
#include "cgi_tc.h"

/*定义hostType两个宏，与app端hostType枚举类型匹配*/
#define DEVICE_TYPE_ANDROID 0
#define DEVICE_TYPE_UNKNOWN 4

/*定义接入类型的宏,有线,2G,5G,2G访客,5G访客,与APP的协议约定*/
#define WIRES	0
#define WIFI_2G	10
#define WIFI_5G	11
#define WIFI_2G_GUEST	12
#define WIFI_5G_GUEST	13
extern char *cjson_get_value(cJSON *root, char *name, char *defaultGetValue);
extern int cjson_get_number(cJSON *root, char *name, int defaultGetValue);

extern int is_mac_trust(const char *m_mac);

/*****************************************************************************
 函 数 名  : app_get_ol_host_info
 功能描述  : 获取所有在线、离线设备信息
 输入参数  : webs_t wp,
 			 cJSON *root
 			 void *info
 输出参数  : 无
 返 回 值  : 

 修改历史      :
  1.日    期   : 2016年12月20日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
extern int g_speedtest_web_update_tc_timeout;
void app_get_ol_host_info(cJSON *recv_root,cJSON *send_root,void *info)
{
	int count = 0, i = 0;
	int host_type = DEVICE_TYPE_ANDROID;
	float up_limit = 0;	
	float down_limit = 0;
	int bw_limited = 1;
	char mac[PI_MAC_STRING_LEN] = {0};
	char  manufactory[PI_BUFLEN_32] = {0};
	cJSON *online_expired_list =NULL;
	char value[32] = {0};
	
	online_expired_list  = cJSON_CreateArray(); /*用于保存所有的离线在线设备信息*/
	
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_TC_ONLINE_LIST,
		MODULE_GET_EXPIRED_INFO,
		#ifdef __CONFIG_GUEST__
		MODULE_GET_GUEST_ONLINE_LIST,
		#endif
	};
	
	/*调用cgi,获取所有的在线列表*/
	get_info.wp = NULL;
	get_info.root = online_expired_list;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	g_speedtest_web_update_tc_timeout = 30;	
	count = cJSON_GetArraySize(online_expired_list);

    cJSON *online_expired_child_obj = NULL;
	cJSON *root = NULL;
	cJSON *obj = NULL;

	cJSON_AddItemToObject(send_root,"hostInfo", root = cJSON_CreateArray());
	for(i = 0; i < count; i++)
	{
		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		
		online_expired_child_obj = cJSON_GetArrayItem(online_expired_list,i);	
		/*hostname*/
		cJSON_AddStringToObject(obj,"hostname", cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_HOSTNAME,""));	
		/*remark*/
		cJSON_AddStringToObject(obj,"remark", cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_REMARK,""));
		/*ip*/
		cJSON_AddStringToObject(obj,"ip", cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_IP,""));
	
		strcpy(mac,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_MAC,""));
        cJSON_AddStringToObject(obj,"mac", mac);/*mac*/
		
		strcpy(value,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_CONN_TYPE,"wifi"));
		if(0 == strcmp("wires",value))
		{
			cJSON_AddNumberToObject(obj,"ConnectType",WIRES);/*有线*/
		}
		else
		{
			memset(value,0x0,sizeof(value));
			strcpy(value,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_ACCESS_TYPE,""));
			if(0 == strcmp(value,"wifi_2G"))	//2G
			{
				cJSON_AddNumberToObject(obj,"ConnectType",WIFI_2G);
			}
			else if(0 == strcmp(value,"wifi_5G"))	//5G
			{
				cJSON_AddNumberToObject(obj,"ConnectType",WIFI_5G);
			}
			#ifdef __CONFIG_GUEST__
			else if(0 == strcmp(value,"wifi_guest_2G"))	//2G访客 
			{
				cJSON_AddNumberToObject(obj,"ConnectType",WIFI_2G_GUEST);
			}
			else if(0 == strcmp(value,"wifi_guest_5G"))
			{
				cJSON_AddNumberToObject(obj,"ConnectType",WIFI_5G_GUEST);
			}
			#endif
		}

		/*是否在信任列表*/
		if(1 == is_mac_trust(mac))
		{
			cJSON_AddNumberToObject(obj,"is_mac_trust",1);
		}
		else
		{
			cJSON_AddNumberToObject(obj,"is_mac_trust",0);
		}
		/*connectTime*/
		cJSON_AddNumberToObject(obj,"ConnetTime",cjson_get_number(online_expired_child_obj,LIB_QOS_LIST_CONNECT_TIME,0));
		
		/*判断是否限速	38528 * 128KB/s表示不限速*/
        down_limit = atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_DOWN_LIMIT,"38528"));
		up_limit = atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_UP_LIMIT,"38528"));
	    
		/*app端-1表示不限速*/
		if( (UP_RATE_LIMIT * 128 == up_limit) && (DOWN_RATE_LIMIT * 128 == down_limit))
		{
			bw_limited = 0;  /*不限速*/
			
			down_limit = -1;
			up_limit = -1;	
		}
		else
		{
			bw_limited = 1;  /*限速*/
			
			if(UP_RATE_LIMIT * 128 == up_limit)
				up_limit = -1;
			if(DOWN_RATE_LIMIT * 128 == down_limit)
				down_limit = -1;
		}
		cJSON_AddNumberToObject(obj,"down_limit",down_limit);
		cJSON_AddNumberToObject(obj,"up_limit",up_limit);
		cJSON_AddNumberToObject(obj,"bw_limited",bw_limited);
		
		cJSON_AddNumberToObject(obj,"curr_down_rate",atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_DOWN_SPEED,"")));
		cJSON_AddNumberToObject(obj,"curr_up_rate",atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_UP_SPEED,"")));

		/*设备厂商*/
		strcpy(manufactory,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_MANUFACTURER,""));
		if(0 == strcmp(manufactory,"other"))
		{
			strcpy(manufactory,"unknown");
			host_type = DEVICE_TYPE_UNKNOWN;  //未知主机类型
		}
		cJSON_AddStringToObject(obj,"manufactory",manufactory); 
		
		cJSON_AddNumberToObject(obj,"hostTpye",host_type);   //设备类型 
		/**/
		if(0 == cjson_get_number(online_expired_child_obj,LIB_EXPIRED_IS,1))
		{		
			cJSON_AddNumberToObject(obj,"online",0);    //设备离线
			/*白名单模式*/
			if(-1 != is_mac_filter(mac,"deny"))
			{
				cJSON_AddNumberToObject(obj,"mac_blocked",1);
			}
			else
			{
				/*不管设备是不是在白名单模式*/
				cJSON_AddNumberToObject(obj,"mac_blocked",0);
			} 
		}
		else
		{		
			cJSON_AddNumberToObject(obj,"online",1);    //设备在线
			/*这里注意黑白名单模式*/
	        if(!strcmp("deny",nvram_safe_get("filter_mac_mode"))) //黑名单模式
	        {
				cJSON_AddNumberToObject(obj,"mac_blocked",0); 
	        }
			/*要确定 app 用mac_blocked于做什么????*/
	        else  /*白名单模式*/
	        {
				/*判断是否在mac地址过滤中*/
	            if(-1 == is_mac_filter(mac,"deny"))
	            {
					/*该设备不在黑名单模式下*/
	            	cJSON_AddNumberToObject(obj,"mac_blocked",0);
	            }
	            else
	            {
	                cJSON_AddNumberToObject(obj,"mac_blocked",1);
	            }
	        }
		}
	}
		
	cJSON_Delete(online_expired_list);  
    return ;
}

