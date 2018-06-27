/*************************************************************************
  > Copyright (C) 1998-2016, Tenda Tech. Co., All Rights Reserved.
  > File Name: userSpace/cbb/src/button/src/wps_button.c
  > Description: 
  > Author: ZhuHuan
  > Mail: zhuhuan_IT@outlook.com
  > Version: 1.0
  > Created Time: Monday 2016-07-04 16:01 CST
  > Function List: 

  > History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include <stdio.h>

#include "button.h"
#include "sys_module.h"

#ifndef ZH_DEBUG
#define ZH_DEBUG(format, string...) printf("func=%s;line=%d; " format, __func__, __LINE__,  ##string)
#endif

static void wps_button_handle(void);

extern RET_INFO msg_send(PIU8 center,PIU8 id,PI8 *msg);
#ifdef __CONFIG_EXTEND_LED__
extern int wps_button_action;
#endif

BUTTON wps_button_desc = {
    .name = "wps",
    .gpio = WPS_BUTTON_GPIO,
    .min_time = WPS_BUTTON_MIN_TIME,
    .max_time = WPS_BUTTON_MAX_TIME,
    .count = 0,
#ifdef __CONFIG_WPS_RESET_MULTIPLEXED__
	.trigger_type = UNLOCKED_EFFECT, 
#else
    .trigger_type = LOCKED_EFFECT, 
#endif
    .is_handled = 0,
    .handle = wps_button_handle,
};

static void wps_button_handle(void)
{
    PI_PRINTF(MAIN,"wps button checked!\n");
	
	/*规格要求非无线中继模式下页面wps为关闭状态时，如果加密方式支持则开启WPS功能*/
	if(nvram_match("wps_mode","disabled") 
		&& (nvram_match(SYSCONFIG_WORKMODE,"route") 
		|| nvram_match(SYSCONFIG_WORKMODE,"bridge")))
	{
		if(nvram_match("wl0_akm","psk"))
		{
			printf("AUTH_TYPE error,start wps fail!\n");
			return ;
		}
		nvram_set("wps_mode","enabled");
		
		msg_send(MODULE_RC,RC_WPS_MODULE,"string_info=btn_startpbc");
	}
	else
	{
    	msg_send(MODULE_RC,RC_WPS_MODULE,"string_info=startpbc");
	}
	#ifdef __CONFIG_EXTEND_LED__
	wps_button_action = 1;
	#endif

    return ;
}
