
#include "cgi_lib.h"
#if 0
PIU8 set_modules_sysOperate[] = 
{
	MODULE_SET_OPERATE,	
	MODULE_SET_END,
};

PIU8 set_modules_softWare[] = 
{
	MODULE_SET_FIREWARE,
	MODULE_SET_END,
};

PIU8 get_modules_softWare[] = 
{
	MODULE_GET_FIREWARE,
	MODULE_GET_END,
};
#endif
RET_INFO cgi_sysmanage_operate_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_OPERATE,					
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return(RET_SUC);
}


RET_INFO cgi_sysmanage_fireware_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_FIREWARE,	
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);


	return(RET_SUC);
}
RET_INFO cgi_sysmanage_fireware_get(webs_t wp, cJSON *root, void * info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_FIREWARE,	
	};
	cJSON_AddItemToObject(root, "softWare", obj = cJSON_CreateObject());
	
	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	
	return RET_SUC;
}

