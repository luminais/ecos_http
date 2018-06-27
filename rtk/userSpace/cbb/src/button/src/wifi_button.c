/*************************************************************************
  > Copyright (C) 1998-2016, Tenda Tech. Co., All Rights Reserved.
  > File Name: userSpace/cbb/src/button/src/wifi_button.c
  > Description: 
  > Author: ZhuHuan
  > Mail: zhuhuan_IT@outlook.com
  > Version: 1.0
  > Created Time: Monday 2016-07-04 15:56 CST
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

static void wifi_button_handle(void);

extern RET_INFO msg_send(PIU8 center,PIU8 id,PI8 *msg);

BUTTON wifi_button_desc = {
    .name = "wifi",
    .gpio = WIFI_BUTTON_GPIO,
    .min_time = WIFI_BUTTON_MIN_TIME,
    .max_time = WIFI_BUTTON_MAX_TIME,
    .count = 0,
    .trigger_type = UNLOCKED_EFFECT, 
    .is_handled = 0,
    .handle = wifi_button_handle,
};

static void wifi_button_handle(void)
{
	char buf[64] = {0};
    PI_PRINTF(MAIN,"wifi button checked!\n");
	if(nvram_match(SYSCONFIG_WORKMODE,"route") 
		|| nvram_match(SYSCONFIG_WORKMODE,"bridge"))
	{
		sprintf(buf,"wlan0 led 3");
    		run_clicmd(buf);
		memset(buf,0x0,sizeof(buf));
		sprintf(buf,"wlan1 led 3");
   		run_clicmd(buf);
    		msg_send(MODULE_RC, RC_WIFI_MODULE, "string_info=button");
	}

    return ;
}
