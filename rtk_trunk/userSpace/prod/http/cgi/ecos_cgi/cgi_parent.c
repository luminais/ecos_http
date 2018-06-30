#include "cgi_lib.h"
#if 0
PIU8 get_modules_parentCtrlList[] = 
{
	MODULE_GET_PARENT_ONLINE_LIST,
	MODULE_GET_END,
};

PIU8 get_modules_parentAccessCtrl[] = 
{
	MODULE_GET_PARENT_ACCESS_CTRL,
	MODULE_GET_END,
};

PIU8 set_modules_parentCtrlList[] = 
{
	MODULE_SET_PARENT_ONLINELIST,	
	MODULE_SET_END,
};

PIU8 set_modules_parentAccessCtrl[] = 
{
	MODULE_SET_PARENT_ACCESS_CTRL,
	MODULE_SET_END,
};
#endif
RET_INFO cgi_parent_get_online_list(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_PARENT_ONLINE_LIST,
	};
	
	cJSON_AddItemToObject(root, "parentCtrlList", obj = cJSON_CreateArray());
	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;

}

RET_INFO cgi_parent_get_parentAccessCtrl(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_PARENT_ACCESS_CTRL,
	};
	
	cJSON_AddItemToObject(root, "parentAccessCtrl", obj = cJSON_CreateObject());
	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;	
}

RET_INFO cgi_parent_set_onlineList(webs_t wp, CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_PARENT_ONLINELIST,	
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return RET_SUC;
	
}

RET_INFO cgi_parent_set_parentAccessCtrl(webs_t wp, CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_PARENT_ACCESS_CTRL,	
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return RET_SUC;
}

