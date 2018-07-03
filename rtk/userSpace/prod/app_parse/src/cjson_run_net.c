#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedefs.h>
#include <time.h>

#include "arp_clients.h"
#include "cgi_lib.h"
#include "wifi.h"



#define TR_RULE_NUMBER_MAX    20     /*定义一个信任列表的最大值*/     
#define _FW_TRUST_MAC(i)           racat("trust_list", i)
#define	TIME_LEN			16
#define	ASSO_SN_LEN			20


extern int tenda_arp_is_wireless_client(unsigned char *mac);
extern char *cjson_get_value(cJSON *root, char *name, char *defaultGetValue);

 /*****************************************************************************
 函 数 名  : app_set_rub_net_add_blacklist
 功能描述  : 添加黑名单 单条生效
 输入参数  : cJSON *recv_root	
			 CGI_MSG_MODULE *msg 
			 int *result_code
			 void *info

 返 回 值  : 
 
 修改历史	   :
  1.日	  期   : 2016年12月13日
	作	  者   : luorilin
	修改内容   : 新生成函数
*****************************************************************************/
void app_set_rub_net_add_blacklist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	/*调用cgi lib库实现设置*/
	PI8 err_code[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_RUB_NET_ADD_BLACKLIST,	
	};
	if(NULL == recv_root)
	{
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
	}
	set_info.wp = NULL;
	set_info.root = recv_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);

	if(0 == strcmp(err_code,"80"))
	{
		*result_code = 80;
	}
	else if(0 == strcmp(err_code,"0"))
	{
		*result_code = 0;
	}
	else
	{
		*result_code = 1;
	}
	
	return;	
}

/*****************************************************************************
函 数 名	: app_set_rub_net_delete_blacklist
功能描述	: 删除黑名单 单条生效
输入参数	: cJSON *recv_root	 
			  CGI_MSG_MODULE *msg 
			  int *result_code
			  void *info

返 回 值	: 

修改历史		:
1.日    期	: 2016年12月13日
 作    者	: luorilin
 修改内容	: 新生成函数
*****************************************************************************/
void app_set_rub_net_delete_blacklist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	PI8 err_code[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_RUB_NET_DELETE_BLACKLIST,	
	};
	if(NULL == recv_root)
	{
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
	}
	set_info.wp = NULL;
	set_info.root = recv_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	
	if(0 != strcmp(err_code,"0"))
	{
		*result_code = 1;
	}
	return;
}
/*****************************************************************************
函 数 名  : app_set_rub_net_add_to_trustlist
功能描述  : 关闭在线提醒
输入参数  : cJSON *recv_root	
			 CGI_MSG_MODULE *msg 
			 int *result_code
			 void *info

返 回 值  : 

修改历史	   :
1.日	  期   : 2016年12月13日
作	  者   : luorilin
修改内容   : 新生成函数
*****************************************************************************/
void app_set_rub_net_add_to_trustlist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{	
	PI8 err_code[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_RUB_NET_ADD_TRUSTLIST,	
	};
	if(NULL == recv_root)
	{
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
	}
	set_info.wp = NULL;
	set_info.root = recv_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);

	if(0 != strcmp(err_code,"0"))
	{
		*result_code = 1;
	}
	return ;
}


/*****************************************************************************
函 数 名  : app_set_rub_net_delete_from_trustlist
功能描述  : 开启在线提醒
输入参数  : cJSON *recv_root	
			CGI_MSG_MODULE *msg 
			int *result_code
			void *info

返 回 值  : 

修改历史	   :
1.日	  期   : 2016年12月13日
作	  者   : luorilin
修改内容   : 新生成函数
*****************************************************************************/
void app_set_rub_net_delete_from_trustlist(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	PI8 err_code[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_RUB_NET_DELETE_TRUSTLIST,	
	};
	if(NULL == recv_root)
	{
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
	}
	set_info.wp = NULL;
	set_info.root = recv_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	
	if(0 != strcmp(err_code,"0"))
	{
		*result_code = 1;
	}
	
	return;
}
/*****************************************************************************
函 数 名  : app_get_rub_net_black_list
功能描述  : 获取路由模式下的黑名单
输入参数  : cJSON *recv_root	
			 cJSON *send_root 
			 void *info
返 回 值  : 
修改历史	   :
1.日	  期   : 2016年12月13日
作	  者   : luorilin
修改内容   : 新生成函数
*****************************************************************************/
void app_get_rub_net_black_list(cJSON *recv_root,cJSON *send_root,void *info)
{
	int count = 0,i = 0;
	char dev_name[TPI_BUFLEN_64] = {0};   
	cJSON *obj = NULL;
    cJSON *root = NULL;

	char *filtermode = NULL;  //路由器模式
	
	cJSON *macFilterList =NULL; 
	cJSON *macFilterList_obj = NULL;
	cJSON *macFilterList_item = NULL;
	
	macFilterList  = cJSON_CreateObject();  /*创建对象,保存获取的数据*/
	
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_MACFILTER_LIST,
	};
	
	/*调用cgi,获取所有的黑名单列表*/
	get_info.wp = NULL;
	get_info.root = macFilterList;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);

    /*获取mac地址过滤列表*/
    macFilterList_obj = cJSON_GetObjectItem(macFilterList,LIB_MAC_FILTER_LIST);
	if(NULL == macFilterList_obj)
	{
		cJSON_Delete(macFilterList);
		printf("[%s][%d]macFilterList_obj is null!\n",__func__,__LINE__);
		return;
	}
	/*黑明单总数(cgi_lib库中获取的包括黑名单模式和白名单模式下的)*/
	count = cJSON_GetArraySize(macFilterList_obj);

    /*将数据解析重新封装 添加到send_root对象*/
	/*这里封装的数据都是安照app接口对应函数的参数*/
	cJSON_AddItemToObject(send_root,"black_list_info", root = cJSON_CreateArray());

	for(i = 0; i < count; i++)
	{  
		/*从mac过滤表中取出一个子对象*/
		macFilterList_item = cJSON_GetArrayItem(macFilterList_obj,i);

		/*在这里只关心黑名单模式下的*/
		filtermode = cjson_get_value(macFilterList_item,LIB_MAC_FILTER_MODE,"deny");

		if(0 == strcmp(filtermode,"deny"))
		{
			cJSON_AddItemToArray(root,obj = cJSON_CreateObject());

			/*重新组装数据*/
			cJSON_AddStringToObject(obj,"mac",cjson_get_value(macFilterList_item,LIB_FILTER_MAC,""));

			strcpy(dev_name,cjson_get_value(macFilterList_item,LIB_REMARK,""));
			if(0 == strcmp(dev_name,""))  /*没有备注名则返回主机名*/
			{
				strcpy(dev_name,cjson_get_value(macFilterList_item,LIB_HOST_NAME,""));
			}
		    cJSON_AddStringToObject(obj,"dev_name",dev_name);
		}	
	}
	
	cJSON_Delete(macFilterList);
	return;
}

/*****************************************************************************
函 数 名  : is_mac_trust
功能描述  : 判断是否在信任列表中
输入参数  : char *m_mac	

返 回 值  : 0:不在信任列表 1:在信任列表

修改历史	   :
1.日	  期   : 2016年12月13日
作	  者   : luorilin
修改内容   : 新生成函数
*****************************************************************************/
int is_mac_trust(const char *m_mac)
{
    int i = 0,j = 0;
	char *value = NULL;

    if(NULL == m_mac)
    {
        return 0;
    }
    for (i = 0; i < TR_RULE_NUMBER_MAX; ++i)
	{
        _GET_VALUE(_FW_TRUST_MAC(i), value);
        if(0 == strcmp(value,""))
        {
            break;
        }
        if((NULL != value) && (strncasecmp(value,m_mac, 17) == 0))
        {
            return 1;
        }
    }
    return 0;
}

static int time_turn( time_t *t,char date[TIME_LEN],char time[TIME_LEN])
{
     
     struct tm *p = NULL;
     int year,month,day,hour,min,sec;
     
     p=gmtime(t);

     sprintf(date,"%d-%d-%d",1900 + p->tm_year,1 + p->tm_mon,p->tm_mday);
     sprintf(time,"%d:%d",p->tm_hour,p->tm_min);
    
    return 0;
}
/*****************************************************************************
函 数 名  : app_get_strange_host_info
功能描述  : 获取刚上线的陌生主机信息
输入参数  : cJSON *recv_root	
			 cJSON *send_root 
			 void *info

返 回 值  : 

修改历史  :
1.日    期   : 2016年12月13日
  作	者   : luorilin
  修改内容   : 新生成函数
*****************************************************************************/
void app_get_strange_host_info(cJSON *recv_root,cJSON *send_root,void *info)
{
	char date_temp[TIME_LEN] = {0},time_temp[TIME_LEN] = {0};
    int i = 0, client_num = 0,strange_num = 0;
	char uc_serial_num[ASSO_SN_LEN] = {0};
	
    P_WIFI_INFO_STRUCT wifi_info = NULL;
	unsigned char cur_hostname[TPI_BUFLEN_64] = {0}; 

	struct client_info  *clients_list = NULL;
	//注意: 
	//调用该函数一次只获取一个陌生上线的设备信息。
	//如果同时有A和B两个上线的设备，则第一次调用该函数只获取A设备的信息，下次调用该函数时则获取B设备的信息
    clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list == NULL )
	{
		return;
	}
	
	/*这里不区分陌生主机上线是哪个频段接入，无论是2.4G/5G/2.4G_guest/5G_guest,都是显示2.4G ssid*/
    wifi_info = gpi_wifi_get_info(WLAN_RATE_24G);  
	
	memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
	
    client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);   //连接客户端数
	
	cJSON *obj = NULL;
    cJSON *root = NULL;

	cJSON_AddItemToObject(send_root,"strang_host_info", root = cJSON_CreateArray());
	
    for(i = 0; i < client_num; i++)
    {
        if( is_mac_trust(clients_list[i].mac) == 1)     //判断是否在信任列表中
            continue;
        if(clients_list[i].time == 0)	
			continue;
       	//根据app规格要求，有线设备不算陌生设备，是有线设备忽略 2017/1/9 修改
        #ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		/*客户端类型 0：有线 1：2.4G 2：5G 3：2.4G_guest 4：5G_uest*/
		if(0 != tenda_arp_ip_to_flag(inet_addr(clients_list[i].ip)))
		{
			/*客户端离线了，可能还在arp表里*/
			if(0 == tenda_arp_is_wireless_client(ether_aton(clients_list[i].mac)))
				continue;
		}
        else
        {
			continue;
        }
		#endif
		cJSON_AddItemToArray(root,obj = cJSON_CreateObject());

		/*将数据封装好，返回*/
        if(0 != strcmp(clients_list[i].hostname,""))
		{
			/*没有进行编码格式的判断，导致设备名称出现乱码*/
				memset(cur_hostname,0x0,TPI_BUFLEN_64);
				
			#ifdef __CONFIG_SUPPORT_GB2312__
				if (1 == is_cn_encode(clients_list[i].hostname))
				{
					set_cn_ssid_encode("utf-8", clients_list[i].hostname, cur_hostname);
					strcpy(clients_list[i].hostname, cur_hostname);
				}
			#else
				if(1 == is_gb2312_code(clients_list[i].hostname))
				{
					strcpy(cur_hostname, "Unknown") ;
				}
			#endif
				else
				{
					strcpy(cur_hostname,clients_list[i].hostname);
				}
		}
        else
        {
        	strcpy(cur_hostname,"Unknown");
        }
		
        if(!strcmp(clients_list[i].mark,""))
        {
        	cJSON_AddStringToObject(obj,"strange_name", cur_hostname);  
        }
        else
        {
        	cJSON_AddStringToObject(obj,"strange_name", clients_list[i].mark);
        }	
		cJSON_AddStringToObject(obj,"strange_mac",clients_list[i].mac);
      
        time_turn(&clients_list[i].time,date_temp,time_temp);

		cJSON_AddStringToObject(obj,"strange_date",date_temp);
		cJSON_AddStringToObject(obj,"strange_time",time_temp);
		cJSON_AddStringToObject(obj,"strange_ssid",wifi_info->ap_ssid_cfg.ssid);   
		cJSON_AddNumberToObject(obj,"strange_type",0);   
 		strcpy(uc_serial_num ,nvram_safe_get("uc_serialnum"));
		cJSON_AddStringToObject(obj,"uc_serialnum",uc_serial_num);  /*sn序列号*/
		cJSON_AddNumberToObject(obj,"strange_dype",0);  /*设备类型*/
    }
	
    free(clients_list);
	return;
}

/*****************************************************************************
函 数 名  : app_get_mac_filter_mode
功能描述  : 获取路由器当前是工作在黑名单模式还是白名单模式
输入参数  : cJSON *recv_root	
			 cJSON *send_root 
			 void *info

返 回 值  : 

修改历史	   :
1.日	  期   : 2017年04月17日
作	  者   : 
修改内容   : 新生成函数
*****************************************************************************/

void app_get_mac_filter_mode(cJSON *recv_root,cJSON *send_root, void *info)
{
	cJSON *obj = NULL;
	char * curFilterMode = NULL;
	int mac_mode = 1;
	int enable = 0;
	CGI_LIB_INFO get_info;
	
	if((NULL == recv_root) || (NULL == send_root))
	{
		printf("line[%d] recv_roor or send_root is null\n",__LINE__);
		return ;
	}

	//调用cgi_lib库获取app需要的信息
    PIU8 modules[] =
    {
        MODULE_GET_MACFILTER_MODE,
    };

	obj = cJSON_CreateObject();
	if(NULL == obj)
	{
		printf("line[%d] create obj fail\n",__LINE__);
		return ;
	}

	get_info.wp = NULL;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);

    cgi_lib_get(get_info, NULL);        //调用lib库公共函数

	curFilterMode = cjson_get_value(obj,"curFilterMode","deny");
	if(NULL == curFilterMode)
	{
		printf("line[%d] return\n",__LINE__);
		cJSON_Delete(obj);
		return ;
	}

	if(0 == strcmp(curFilterMode,"deny"))		//黑名单
	{
		mac_mode = 1;
		enable = 1;
	}
	else if(0 == strcmp(curFilterMode,"pass"))	//白名单
	{	
		mac_mode = 2;
		enable = 1;
	}
	else
	{
		mac_mode = 0;
		enable = 0;
	}
	cJSON_AddNumberToObject(send_root,"mac_mode",mac_mode);
	cJSON_AddNumberToObject(send_root,"enable",enable);
	cJSON_AddNumberToObject(send_root,"supt_mac_mode",3);	//给app返回路由器支持的模式，1-仅支持黑名单，2-仅支持白名单，3-黑白名单都支持
	
	cJSON_Delete(obj);
	return ;
}


/*****************************************************************************
函 数 名  : app_set_macfilter_mode
功能描述  : 设置路由器的黑白名单模式
输入参数  : cJSON *recv_root	
			 cJSON *send_root 
			 void *info

返 回 值  : 

修改历史	   :
1.日	  期   : 2017年04月24日
作	  者   : 
修改内容   : 新生成函数
*****************************************************************************/
void app_set_macfilter_mode(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	PI8 err_code[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO set_info;
	int mac_mode = 1;
	cJSON *obj = NULL;
	int ret = 0;
	
	PIU8 modules[] = 
	{
		MODULE_SET_MACFILTER_MODE,	
	};
	if(NULL == send_root)
	{
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
	}
	obj = cJSON_CreateObject();
	if(NULL == obj)
	{
		printf("create obj fail\n");
		return ;
	}
	

	//对app传来的模式字段进行转换1-黑名单，2-白名单
	mac_mode = cjson_get_number(send_root,"mac_mode",1);
	if(2 == mac_mode)
	{
		cJSON_AddStringToObject(obj,LIB_MAC_FILTER_MODE,"pass");
	}
	else
	{
		cJSON_AddStringToObject(obj,LIB_MAC_FILTER_MODE,"deny");
	}
	set_info.wp = NULL;
	set_info.root = obj;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	ret = cgi_lib_set(set_info,msg,err_code,NULL);


	cJSON_Delete(obj);
	
	return;
}

/*****************************************************************************
函 数 名  : app_set_rub_net_flush_blacklist
功能描述  : 清空所有的黑名单
输入参数  : cJSON *recv_root	
			 cJSON *send_root 
			 void *info

返 回 值  : 

修改历史	   :
1.日	  期   : 2017年04月24日
作	  者   : 
修改内容   : 新生成函数
*****************************************************************************/
void app_set_rub_net_flush_blacklist(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	PI8 err_code[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO set_info;
	int ret = 0;

	PIU8 modules[]=
	{
		MODELE_SET_FLUSH_BLACKLIST,	
	};

	set_info.wp = NULL;
	set_info.root = NULL;		//这里传NULL进去，因为清空黑名单的lib库函数不用从set_info取任何值
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	ret = cgi_lib_set(set_info,msg,err_code,NULL);

	
	return ;
}






