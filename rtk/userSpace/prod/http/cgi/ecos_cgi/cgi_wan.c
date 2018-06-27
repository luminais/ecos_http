/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_wan.c
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2016年12月12日
  最近修改   :
  功能描述   : 

  功能描述   : wan口接入设置和参数获取

  修改历史   :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/
#include "cgi_lib.h"
#if 0

PIU8 set_modules_wanBasicCfg[] = 
{
	MODULE_SET_WAN_ACCESS,	
	MODULE_SET_END,
};

PIU8 set_modules_manage_wan[] = 
{
	MODULE_SET_WAN_MTU,
	MODULE_SET_WAN_SPEED,
	MODULE_SET_MAC_CLONE,	
	MODULE_SET_WAN_SERVER,
	MODULE_SET_END,
};

PIU8 get_modules_wanDetection[] = 
{
	MODULE_GET_WAN_DETECTION,
	MODULE_GET_END,
};

PIU8 get_modules_wanBasicCfg[] = 
{
	MODULE_GET_WAN_TYPE,
	MODULE_GET_ADSL_INFO,
	MODULE_GET_NET_INFO,
	MODULE_GET_END,
};

PIU8 get_modules_wanAdvCfg[] = 
{
	MODULE_GET_WAN_TYPE,
	MODULE_GET_WAN_SERVER,	
	MODULE_GET_WAN_MTU,		
	MODULE_GET_WAN_SPEED,		
	MODULE_GET_MAC_CLONE,		
	MODULE_GET_WAN_MAC,		
	MODULE_GET_NATIVE_HOST_MAC,
	MODULE_GET_END,
};
#endif
RET_INFO cgi_wanDetection_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_WAN_DETECTION,
	};

	cJSON_AddItemToObject(root, "wanDetection", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;

}

RET_INFO cgi_accessParam_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_WAN_TYPE,
		MODULE_GET_ADSL_INFO,
		MODULE_GET_NET_INFO,	
	};
	cJSON_AddItemToObject(root, "wanBasicCfg", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;

}

RET_INFO cgi_sysmanage_wan_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_WAN_TYPE,
		MODULE_GET_WAN_SERVER,	
		MODULE_GET_WAN_MTU,		
		MODULE_GET_WAN_SPEED,		
		MODULE_GET_MAC_CLONE,		
		MODULE_GET_WAN_MAC,		
		MODULE_GET_NATIVE_HOST_MAC,	
	};
	cJSON_AddItemToObject(root, "wanAdvCfg", obj = cJSON_CreateObject());
	
	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;
}

RET_INFO cgi_accessParam_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_WAN_ACCESS,	
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return(RET_SUC);
}

RET_INFO cgi_sysmanage_wan_set(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_WAN_MTU,
		MODULE_SET_WAN_SPEED,
		MODULE_SET_MAC_CLONE,	
		MODULE_SET_WAN_SERVER,		
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return(RET_SUC);
}

RET_INFO cgi_set_wan(webs_t wp,CGI_MSG_MODULE * msg,char * err_code, void *info)
{
	int i = 0;
	int wan_flag = 0;
	char *private_event = (char*)info;
	CGI_MSG_MODULE remov_msg;
	for(i = 0; i < MAX_MSG_NUM; ++i)
	{
		if(msg[i].id == RC_WAN_MODULE)
		{
			msg_send(MODULE_RC, msg[i].id, msg[i].msg);
			wan_flag = 1;
		}
	}
	
	if(wan_flag == 1)
	{
		msg_waitback(MODULE_RC,RC_WAN_MODULE,100,0);
		memset(remov_msg.msg,0x0,MAX_MODULE_MSG_MAX_LEN);
		remov_msg.id = RC_WAN_MODULE;
		remove_msg_to_list(msg,remov_msg);
	}
	return RET_SUC;
}
