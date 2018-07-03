/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_lib_sysmanage.c
  版 本 号   : 初稿
  作    者   : 段靖铖
  生成日期   : 2016年12月13日
  最近修改   :
  功能描述   :

  功能描述   : sysmanage的最小功能单元的get和set库

  修改历史   :
  1.日    期   : 2016年12月13日
    作    者   : 段靖铖
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <router_net.h>
#include "flash_cgi.h"
#include "webs.h"
#include "cJSON.h"
#include "cgi_common.h"
#include "sys_module.h"
#include "http.h"
#include <autoconf.h>
#include "cgi_lib.h"


extern int sys_led_turn;

/*****************************************************************************
 函 数 名  : cgi_lib_set_led
 功能描述  : led定时关闭
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月6日
    作    者   : 刘松明
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_led(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	char_t *ledType,*led_timer;
	char led_ctl_type[16] = {0};
	char timer[64] = {0};
	int restart = 0;
	

    ledType = cgi_lib_get_var(wp,root, T(LIB_LED_STATUS),"");
    led_timer = cgi_lib_get_var(wp,root, T(LIB_LED_CLOSE_TIME),"");

	if(strcmp(nvram_safe_get(SAVE_POWER_LED_TYPE),ledType))
	{
		nvram_set(SAVE_POWER_LED_TYPE,ledType);
		restart = 1;
	}
	//LED类型为定时关闭
	if(0 == strcmp("2",ledType))
	{
		if(strcmp(nvram_safe_get(SAVE_POWER_LED_TIME),led_timer))
		{
			nvram_set(SAVE_POWER_LED_TIME,led_timer);
			restart = 1;
		}
	}
	
	if(restart)
    {
        CGI_MSG_MODULE msg_tmp;
        msg_tmp.id = RC_LED_MODULE;
        sprintf(msg_tmp.msg, "op=%d", OP_RESTART);
        add_msg_to_list(msg, &msg_tmp);
    }

	
	 if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_get_led
 功能描述  : 获取LED状态信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月6日
    作    者   : 刘松明
    修改内容   : 新生成函数

********	*********************************************************************/
RET_INFO cgi_lib_get_led(webs_t wp, cJSON *root, void *info)
{
	char iterm_value[PI_BUFLEN_64] = {0};
	//LED控制类型 0:常开 1:常关 2:定时关闭
	strcpy(iterm_value,nvram_safe_get(SAVE_POWER_LED_TYPE));
	cJSON_AddStringToObject(root,LIB_LED_STATUS,iterm_value);

	//LED控制关闭时间段
	memset(iterm_value,0x0,sizeof(iterm_value));
	strcpy(iterm_value,nvram_safe_get(SAVE_POWER_LED_TIME));
	cJSON_AddStringToObject(root,LIB_LED_CLOSE_TIME,iterm_value);

	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_get_led
 功能描述  : 用于APP获取LED状态信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年11月6日
    作    者   : 刘松明
    修改内容   : 新生成函数

********	*********************************************************************/
RET_INFO cgi_lib_get_led_app(webs_t wp, cJSON *root, void *info)
{
	char iterm_value[PI_BUFLEN_64] = {0};
	//LED控制类型 0:常开 1:常关 2:定时关闭
	strcpy(iterm_value,nvram_safe_get(SAVE_POWER_LED_TYPE));
	//当LED被设置为定时关闭时,系统灯不闪则认为是LED全关,否则为全开
	if(0 == strcmp("2",iterm_value))
	{
		if(0 == sys_led_turn)
		{
			cJSON_AddStringToObject(root,LIB_LED_STATUS,"1");
		}
		else
		{
			cJSON_AddStringToObject(root,LIB_LED_STATUS,"0");
		}
	}
	else
	{
		cJSON_AddStringToObject(root,LIB_LED_STATUS,iterm_value);
	}

	return RET_SUC;
}



