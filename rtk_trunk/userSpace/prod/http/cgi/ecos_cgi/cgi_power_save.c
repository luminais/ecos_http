/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_power_save.c
  版 本 号   : 初稿
  作    者   : 
  生成日期   : 2017年12月15日
  最近修改   :
  功能描述   :

  功能描述   : 无线参数的获取与设置

  修改历史   :
  1.日    期   : 2017年11月15日
    作    者   : 
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#include "cgi_lib.h"
#ifdef __CONFIG_LED__
RET_INFO cgi_power_save_led_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void * info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	{
		MODULE_SET_LED_INFO,	
	};	

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	
	return RET_SUC;	
}

RET_INFO cgi_power_save_led_get(webs_t wp, cJSON *root, void * info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_LED_INFO,	
	};

	cJSON_AddItemToObject(root, "LEDControl", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);

	return RET_SUC;
}
#endif

