/*************************************************************************
  > Copyright (C) 1998-2016, Tenda Tech. Co., All Rights Reserved.
  > File Name: button.h
  > Description: 
    本头文件主要是定义了按键触发类型、按键结构实体、公共函数原型等。
  > Author: ZhuHuan
  > Mail: zhuhuan_IT@outlook.com
  > Version: 1.0
  > Created Time: Friday 2016-07-01 08:41 CST
  > Function List: 

  > History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "pi_common.h"
#include "gpio_api.h"
#include <bcmnvram.h>

#define RESET_BUTTON_GPIO RESET_BTN_PIN
/* 与测试达成一致，ecos产品的复位时间固定为>=8s，2016.07.13 */
#define RESET_BUTTON_MIN_TIME  8
#define RESET_BUTTON_MAX_TIME  0xFF

#ifdef __CONFIG_WPS_RESET_MULTIPLEXED__       // wps、reset按键复用
#define WPS_BUTTON_GPIO RESET_BUTTON_GPIO 
#else
#define WPS_BUTTON_GPIO WPS_BTN_PIN
#endif
#define WPS_BUTTON_MIN_TIME  1
#define WPS_BUTTON_MAX_TIME  3

#ifdef __CONFIG_WIFI_RESET_MULTIPLEXED__       // wifi、reset按键复用
#define WIFI_BUTTON_GPIO RESET_BUTTON_GPIO 
#else
#define WIFI_BUTTON_GPIO WIFI_SWITCH_BTN_PIN
#endif
#define WIFI_SWITCH_BUTTON_GPIO WIFI_BUTTON_GPIO 
#define WIFI_BUTTON_MIN_TIME  1
#define WIFI_BUTTON_MAX_TIME  3

#define MAX_BUTTON_NAME_LEN 16

typedef enum
{
	UNLOCKED_EFFECT = 0,    // 松开生效
    LOCKED_EFFECT = 1       // 按下生效
}TRIGGER_TYPE;

typedef struct button
{
    PI8 name[MAX_BUTTON_NAME_LEN];// 按键名
	PIU8 gpio;                    // 标识按键的gpio口
	PIU8 min_time;                // 按键按下的最短时间计数(单位为1s)
	PIU8 max_time;                // 按键按下的最长时间计数(单位为1s)
	PIU8 count;                   // 按下按键的时间计数(单位为1s)
	TRIGGER_TYPE trigger_type;    // 按键生效类型，按下生效还是松开生效
    PIU8 is_handled;              // 标识该按键按下的动作是否已经处理
	void (*handle)();             // 按键处理函数
}BUTTON,*P_BUTTON;

/* Function declaration */
extern void init_button_services(void);
extern void button_polling_timer(void);
extern void sys_button_change_ate_mode(PIU8 status);
extern PIU8 sys_button_get_ate_result(PIU8 gpio);
extern PIU8 sys_button_get_button_result(void);

#endif
