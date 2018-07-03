/*************************************************************************
  > Copyright (C) 1998-2016, Tenda Tech. Co., All Rights Reserved.
  > File Name: userSpace/cbb/src/button/src/reset_button.c
  > Description: 
  > Author: ZhuHuan
  > Mail: zhuhuan_IT@outlook.com
  > Version: 1.0
  > Created Time: Monday 2016-07-04 11:14 CST
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

static void reset_button_handle(void);

extern RET_INFO msg_send(PIU8 center,PIU8 id,PI8 *msg);

BUTTON reset_button_desc = {
    .name = "reset",
    .gpio = RESET_BUTTON_GPIO,
    .min_time = RESET_BUTTON_MIN_TIME,
    .max_time = RESET_BUTTON_MAX_TIME,
    .count = 0,
    .trigger_type = LOCKED_EFFECT, 
    .is_handled = 0,
    .handle = reset_button_handle,
};

static void reset_button_handle(void)
{
    PI_PRINTF(MAIN,"reset button checked!\n");

    msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,"string_info=restore");

    return ;
}
