#include "cgi_lib.h"

RET_INFO cgi_wifiRelay_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_WIFIRELAY_INFO,
		MODULE_GET_WIFI_RELAY_TYPE,     
		MODULE_GET_WIFIRELAY_CONNECT_DUTATION, 
	};
	cJSON_AddItemToObject(root, "wifiRelay", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;
}

RET_INFO cgi_wifiRelay_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_WIFIRELAY,	
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return(RET_SUC);
}


RET_INFO cgi_wifiScanresault(webs_t wp, cJSON *root, void *info)
{
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_WIFISCANRESAULT
	};
	
	get_info.wp = wp;
	get_info.root = root;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	
	return RET_SUC;
}
