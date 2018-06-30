#include "cgi_lib.h"

extern CGI_LIB_MODULE_INFO cgi_lib_get_modules[];
extern CGI_LIB_MODULE_INFO cgi_lib_set_modules[];
/*****************************************************************************
 函 数 名  : cgi_lib_get
 功能描述  : 调用lib库函数的接口函数，传入需要调用的模块名即可
 输入参数  : CGI_LIB_INFO get_info  
             void *info             
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月15日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get(CGI_LIB_INFO get_info, void *info)
{
	PIU8 index = 0;
	PIU8 module_serch = 0;
	PIU8 module_index = 0;
	while(index < get_info.module_num)
	{
		module_index = get_info.modules[index];
		module_serch = 0;
		while(module_serch < MODULE_GET_END)
		{
			if(cgi_lib_get_modules[module_serch].module_index == module_index)
			{
				CGI_LIB_GET_FUN(cgi_lib_get_modules[module_serch])(get_info.wp,get_info.root, info);
				break;
			}
			module_serch++;
		}
		index++;
	}
	return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_set
 功能描述  : 配置lib函数的接口函数，在进行设置时调用的，页面使用wp，app使用-
             root
 输入参数  : CGI_LIB_INFO set_info  
             CGI_MSG_MODULE *msg    
             char *err_code         
             void *info             
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月15日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set(CGI_LIB_INFO set_info,CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	PIU8 index = 0;
	PIU8 module_serch = 0;
	PIU8 module_index = 0;
	while(index < set_info.module_num)
	{
		module_index = set_info.modules[index];
		module_serch = 0;
		while(module_serch < MODULE_SET_END)
		{
			if(cgi_lib_set_modules[module_serch].module_index == module_index)
			{
				CGI_LIB_SET_FUN(cgi_lib_set_modules[module_serch])(set_info.wp,set_info.root,msg,err_code, info);
			}
			module_serch++;
		}
		index++;
	}
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_var
 功能描述  : 代替原有的websGetvar，用来兼容app
 输入参数  : webs_t wp                
             cJSON*root               
             char_t *var              
             char_t *defaultGetValue  
 输出参数  : 无
 返 回 值  : char_t
 
 修改历史      :
  1.日    期   : 2016年12月15日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
char_t *cgi_lib_get_var(webs_t wp,cJSON*root, char_t *var, char_t *defaultGetValue)
{
	char* value = NULL;
	cJSON* Item = NULL;
	if(wp != NULL)
	{
		value =  websGetVar(wp, var, defaultGetValue);
	}
	else if(root != NULL)
	{
		Item = cJSON_GetObjectItem(root,var);
		if(Item == NULL)
		{
			cJSON_AddStringToObject(root,var,defaultGetValue);
			Item = cJSON_GetObjectItem(root,var);	
		}
		value = Item->valuestring;
	}
	else
		value = NULL;
	return value;
}


