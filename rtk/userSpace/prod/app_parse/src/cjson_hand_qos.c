#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi_lib.h"
#include "cgi_tc.h"

/*添加、删除、修改操作*/
typedef enum {
	HAND_QOS_OPT_SET_ADD = 0,
	HAND_QOS_OPT_SET_UPDATE,
	HAND_QOS_OPT_SET_DELETE,
}hand_qos_set_param_opt;

extern char *cjson_get_value(cJSON *root, char *name, char *defaultGetValue);
extern int cjson_get_number(cJSON *root, char *name, int defaultGetValue);

/*****************************************************************************
函 数 名  : app_get_hand_qos_info
功能描述  : 获取设备带宽信息
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
void app_get_hand_qos_info(cJSON *recv_root,cJSON *send_root,void *info)
{
	char dev_name[PI_DEVNAME_STRING_LEN] = {0}; 
	float u_rate = 0;		
	float d_rate = 0;		
    cJSON *root = NULL;
	cJSON *obj = NULL;	
	char *opt = NULL;
	int count = 0,i = 0;     //count 设备总数
	
	cJSON *online_expired_list =NULL;  /*用于保存获取的所有在线离线设备信息*/
	cJSON *online_expired_child_obj = NULL;

	if(NULL == recv_root)
	{
		printf("[%s][%d] recv_root is null!\n",__func__,__LINE__);
		return;
	}
	online_expired_list = cJSON_CreateArray();

	/*调用cgi_lib库获取所有在线离线设备的信息*/
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_TC_ONLINE_LIST,
	    MODULE_GET_EXPIRED_INFO,  /*离线设备*/
	};

	/*调用cgi,获取所有的在线列表*/
	get_info.wp = NULL;
	get_info.root = online_expired_list;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	g_speedtest_web_update_tc_timeout = 30;
	/*获取opt,判断获取固定mac地址设备，还是所有设备信息*/	
	cJSON *extern_data = cJSON_GetObjectItem(recv_root,"extern_data");	
	if(NULL == extern_data)
	{
		cJSON_Delete(online_expired_list);
		printf("[%s][%d] extern_data is null!\n",__func__,__LINE__);
		return;
	}
	opt = cjson_get_value(extern_data,"opt","");  /*/获取一条还是所有的限速规则*/

	cJSON_AddItemToObject(send_root,"hand_qosInfo", root = cJSON_CreateArray());

	/*解析数据，从新封装*/
	if(0 == strcmp(opt,"one"))
	{
		char *mac = NULL;
		mac = cjson_get_value(extern_data,"mac","");  /*获取mac地址*/
		count  = cJSON_GetArraySize (online_expired_list);    //设备总数
		
	    for(i = 0; i < count; i++)
	    {
	        online_expired_child_obj = cJSON_GetArrayItem(online_expired_list,i);
            if(0 == strncasecmp(mac,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_MAC,""),17))
            {
				cJSON_AddItemToArray(root,obj = cJSON_CreateObject());
				
				cJSON_AddStringToObject(obj,"mac",cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_MAC,""));
                /*app端 -1代表不限速*/
				d_rate = atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_DOWN_LIMIT,""));
				u_rate = atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_UP_LIMIT,""));
				if((UP_RATE_LIMIT * 128) <= u_rate)
				{
					u_rate = -1;
				}
				if((DOWN_RATE_LIMIT * 128) <= d_rate)
				{
					d_rate = -1;
				}
				cJSON_AddNumberToObject(obj,"u_rate",u_rate);
				cJSON_AddNumberToObject(obj,"d_rate",d_rate);

				strcpy(dev_name,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_REMARK,""));
				if(0 == strcmp("",dev_name))
				{
					strcpy(dev_name,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_HOSTNAME,""));
				}					
                cJSON_AddStringToObject(obj,"dev_name",dev_name);
				break;
            }
		}
	}
	if(0 == strcmp(opt,"all"))
	{
		count  = cJSON_GetArraySize (online_expired_list);
		
	    for(i = 0; i < count; i++)
	    {
	        online_expired_child_obj = cJSON_GetArrayItem(online_expired_list,i);

			cJSON_AddItemToArray(root,obj = cJSON_CreateObject());
   
			cJSON_AddStringToObject(obj,"mac",cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_MAC,""));

			/*app端 -1代表不限速*/
			d_rate = atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_DOWN_LIMIT,""));
			u_rate = atof(cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_UP_LIMIT,""));
			if((UP_RATE_LIMIT * 128) <= u_rate)
			{
				u_rate = -1;
			}
			if((DOWN_RATE_LIMIT * 128) <= d_rate)
			{
				d_rate = -1;
			}
			cJSON_AddNumberToObject(obj,"u_rate",u_rate);
			cJSON_AddNumberToObject(obj,"d_rate",d_rate);

			strcpy(dev_name,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_REMARK,""));
			if(0 == strcmp("",dev_name))
			{
				strcpy(dev_name,cjson_get_value(online_expired_child_obj,LIB_QOS_LIST_HOSTNAME,""));
			}	
            cJSON_AddStringToObject(obj,"dev_name",dev_name);
		}
	}
	cJSON_Delete(online_expired_list);
	return;
}

/*****************************************************************************
函 数 名  : app_set_hand_qos_info
功能描述  : 设置设备带宽信息
输入参数  : cJSON *recv_root,
			CGI_MSG_MODULE *msg
			char *err_code
			void *info
输出参数  : 无
返 回 值  : 

修改历史      :
1.日    期   : 2016年12月20日
作    者   : lrl
修改内容   : 新生成函数
*****************************************************************************/
void app_set_hand_qos_info (cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	char mac_addr[PI_MAC_STRING_LEN] = {0};
	
	char hostname[PI_DEVNAME_STRING_LEN] = {0};    
	char *mac_hostname = NULL; 
	char remark[PI_DEVNAME_STRING_LEN] = {0};
	char *mac_remark = NULL;
	
	char temp[PI_BUFLEN_256] = {0};  
	char *data = NULL;
	float u_rate = 0;	
	float d_rate = 0;				   
	int hand_qosInfo_opt = 0;
    int i = 0;
	int count = 0;
	cJSON *hand_qosInfo = NULL;
	cJSON *obj = NULL;
	
    if(NULL == recv_root)
	{
		printf("[%s][%d] recv_root is null!\n",__func__,__LINE__);
		return;
	}
	hand_qosInfo = cJSON_GetObjectItem(recv_root,"set_hand_qosInfo");
	if(NULL == hand_qosInfo)
	{
		printf("[%s][%d] hand_qosInfo is null!\n",__func__,__LINE__);
		return;
	}

    count = cJSON_GetArraySize(hand_qosInfo);

	data = (char *)malloc(sizeof(temp) * count);  /*用于存放进行网速控制设置的数据*/
	memset(data,0x0,sizeof(temp) * count);
	
	for(i = 0; i < count; i++)
	{
		obj = cJSON_GetArrayItem(hand_qosInfo,i);
		if(NULL == obj)
		{
			break;
		}
		hand_qosInfo_opt = cjson_get_number(obj,"hand_qosInfo_opt",0);
		
		//strcpy(dev_name,cjson_get_value(obj,"dev_name",""));
		strcpy(mac_addr,cjson_get_value(obj,"mac",""));

		/*获取主机名*/
		mac_hostname = tenda_arp_mac_to_hostname(mac_addr);

		if(mac_hostname != NULL)
		{
			memset(hostname, 0x0, PI_DEVNAME_STRING_LEN);
#ifdef __CONFIG_SUPPORT_GB2312__
			if (1 == is_cn_encode(mac_hostname))
			{
				char host_name_utf_8[PI_DEVNAME_STRING_LEN] = {0};
				set_cn_ssid_encode("utf-8", mac_hostname, host_name_utf_8);
				strcpy(hostname, host_name_utf_8);
			}
#else
			if(1 == is_gb2312_code(mac_hostname))
			{
				strcpy(hostname, "Unknown");
			}
#endif
			else
			{
				strcpy(hostname, mac_hostname);
			}
		}
		else
		{
			strcpy(hostname, "Unknown");
		}
		/*获取备注名*/
		mac_remark = get_remark(mac_addr);
		if(NULL != mac_remark)
		{
			strcpy(remark,mac_remark);
		}
		else
		{
			strcpy(remark,"");
		}
		u_rate = cjson_get_number(obj,"up_rate",-1);
		d_rate = cjson_get_number(obj,"down_rate",-1);

		//删除操作， app -1 = u_rate -1 = d_rate 表示无限制
		if(HAND_QOS_OPT_SET_DELETE == hand_qosInfo_opt)
	    {
			u_rate = (UP_RATE_LIMIT * 128);    /*无限制 38528KB/s = 301Mbps * 128KB/s */
			d_rate = (DOWN_RATE_LIMIT * 128);  /*无限制 38528KB/s = 301Mbps * 128KB/s */
	    }
		else 
		{
			if(-1 == u_rate)
			{
				u_rate = (UP_RATE_LIMIT * 128);   
			}
			if(-1 == d_rate)
			{
				d_rate = (DOWN_RATE_LIMIT * 128);  
			}
		}
		/*后台处理的格式为: host_name\tdev_remark\tmac\tu_rade\td_rate\ttrue\n*/
		sprintf(temp,"%s\t%s\t%s\t%0.2f\t%0.2f\ttrue\n",hostname,remark,mac_addr,u_rate,d_rate);	
	    strcat(data,temp);
	}
	
	/*调用lib库函数实现配置*/
	PI8 err_code[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO set_info;
	cJSON *set_root = NULL;
	set_root = cJSON_CreateObject();
	
	cJSON_AddStringToObject(set_root,LIB_QOS_ONLINE_LIST,data);
	
	PIU8 modules[] = 
	{
		MODULE_SET_QOS_TC,	
	};
	
	set_info.wp = NULL;
	set_info.root = set_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	
    if(0 != strcmp(err_code,"0"))
	{
		*result_code = 1;
	}
	
	free(data);
	cJSON_Delete(set_root);
    return;
}

/*****************************************************************************
函 数 名  : app_get_hand_qos_max_uplimit
功能描述  : 获取设置带宽限速的最大限制
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
void app_get_hand_qos_max_uplimit(cJSON *recv_root,cJSON *send_root,void *info)
{
	/*限速设置输入框最大有效值*/
	cJSON_AddNumberToObject(send_root,"up_val",UP_RATE_LIMIT_MAX); /* 上行限速的最大值, Mbps = 128KB/s */
	cJSON_AddNumberToObject(send_root,"down_val",DOWN_RATE_LIMIT_MAX); /* 下行限速的最大值, Mbps = 128KB/s */
	
	return;
} 

