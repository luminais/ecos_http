#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "webs.h"
#include "cgi_common.h"
#include "cgi_lib.h"

RET_INFO cgi_system_get_device_statistics(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_ONLINE_NUM,		
		MODULE_GET_BLACK_NUM,		
		MODULE_GET_STREAM_STATISTIC,
		MODULE_GET_WIFI_RATE,		
		MODULE_GET_WIFI_NAME,		
	};
	cJSON_AddItemToObject(root, "deviceStastics", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
    
    return RET_SUC;
}


RET_INFO cgi_get_localhost(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_LOCALHOST,
	};

	cJSON_AddItemToObject(root, "localhost", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);

	return RET_SUC;
}


RET_INFO cgi_tc_get_online_list(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_TC_ONLINE_LIST,
	};

	cJSON_AddItemToObject(root, T("onlineList"), obj = cJSON_CreateArray());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;
}

#ifdef __CONFIG_GUEST__	
RET_INFO cgi_guest_get_online_list(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_GUEST_ONLINE_LIST,
	};

	cJSON_AddItemToObject(root, T("guestList"), obj = cJSON_CreateArray());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;
}
#endif
RET_INFO  cgi_tc_set_qoslist(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_QOS_TC,	
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return RET_SUC;
}


