#include <stdio.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include "flash_cgi.h"

#include "cgi_handle_module.h"
#include "cgi_lib_config.h"

/*****************************************************************************
 函 数 名  : app_get_safe_grade
 功能描述  : app获取安全等级
 			 注意:DNS劫持和ddos攻击无法判断
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
void app_get_safe_grade(cJSON *recv_root,cJSON *send_root, void *info)
{
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_WIFI_BASIC,
	};

	get_info.wp = NULL;
	get_info.root = recv_root;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	

	cJSON *obj = cJSON_GetObjectItem(get_info.root, LIB_WIFI_AP_NO_PWD);
	//如果nvram值"wifiNoPwd"是false说明有密码，如果是ture则表示没有密码
	if(!strcmp(obj->valuestring,"false"))
	{
		obj = cJSON_GetObjectItem(get_info.root, LIB_WIFI_AP_PWD);
		cJSON_AddStringToObject(send_root, "wifi_password",obj->valuestring);
	}
	else
		cJSON_AddStringToObject(send_root, "wifi_password","");

	//获取明文的登录密码
	char login_depw[64] = {0},*login_enpw =NULL;
	_GET_VALUE("http_passwd",login_enpw);
	base64_decode(login_enpw,login_depw,64);
	cJSON_AddStringToObject(send_root, "decode_login_password",login_depw);

}






