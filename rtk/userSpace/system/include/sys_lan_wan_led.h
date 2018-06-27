#ifndef __SYS_LAN_WAN_LED_TIMER_H__
#define	__SYS_LAN_WAN_LED_TIMER_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif


#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
void lan_wan_led_timer();
#endif

#endif/*__SYS_INIT_SERVICES_H__*/