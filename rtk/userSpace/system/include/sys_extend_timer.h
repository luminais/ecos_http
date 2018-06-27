#ifndef __SYS_EXTEND_TIMER_H__
#define	__SYS_EXTEND_TIMER_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifdef __CONFIG_EXTEND_LED__
typedef enum{
	STOP_BLINK = 0,
	START_BLINK
}EXTEND_LED_STATUS;

extern EXTEND_LED_STATUS extend_led_blink_status;

extern void extend_led_start_blink();
extern void extend_led_stop_blink();
extern EXTEND_LED_STATUS extend_led_blink();
#endif

#endif/*__SYS_EXTEND_TIMER_H__*/
