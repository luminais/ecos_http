/*************************************************************************
	> Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
	> File Name: cgi_wan.c
	> Description: 
	> Author: yepeng
	> Mail:yepeng@tenda.com
	> Version: 1.0
	> Created Time: Thu 21 July 2016 14:24:42 AM CST
	> Function List: 

	> History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/



#include "cgi_lib.h"


RET_INFO cgi_sysmanage_remoteWeb_get(webs_t wp, cJSON *root, void *info)
{
	
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_REMOTE_INFO,	
	};
	cJSON_AddItemToObject(root, "remoteWeb", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);

	return RET_SUC;
}

RET_INFO cgi_sysmanage_remoteWeb_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info)
{
	
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	
	PIU8 modules[] = 
	{
		MODULE_SET_REMOTE_INFO,	
	};
		
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);



	return(RET_SUC);
	
}


