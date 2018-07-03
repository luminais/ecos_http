#include <stdio.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include "flash_cgi.h"

#include "cgi_handle_module.h"
#include "cgi_lib_config.h"

/*****************************************************************************
 函 数 名  : app_get_delogin_pwd
 功能描述  : app获取登录密码（明文）
 输入参数  : cJSON *recv_root  
             cJSON *send_root  
             void *info        
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月28日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
void app_get_delogin_pwd(cJSON *recv_root,cJSON *send_root, void *info)
{
	char login_depw[64] = {0},*login_enpw =NULL;
	_GET_VALUE("http_passwd",login_enpw);
	base64_decode(login_enpw,login_depw,64);

	cJSON_AddStringToObject(send_root, "decode_login_password",login_depw);
}


/*****************************************************************************
 函 数 名  : app_set_login_pwd
 功能描述  : app设置管理密码
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月28日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
void app_set_login_pwd(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	CGI_LIB_INFO set_info;
    PI8 err_code[PI_BUFLEN_32] = {0};
	
	PIU8 modules[] = 
	{
		MODULE_SET_LOGIN_PWD,	
	};
	
	set_info.wp = NULL;
	set_info.root = send_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);

	//err_code进行判断
	if(strcmp(err_code,"2") == 0)
		*result_code = 1;
	else
		*result_code = 0;  //成功
}

