#ifndef __SWITCH_LED_H_
#define __SWITCH_LED_H_

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif


/*API*/

/*GPI*/

/*TPI*/
//extern RET_INFO tpi_led_update_info();
extern RET_INFO tpi_switch_led_struct_init();
extern RET_INFO tpi_switch_led_first_init();
extern RET_INFO tpi_switch_led_action(RC_MODULES_COMMON_STRUCT *var);

//extern RET_INFO tpi_wifi_switch_sched_web_info(PI8 *enable,PI8 *times,PI8 *weeks);

//extern void tpi_wifi_switch_sched_debug_level(PI32 level);
#endif/*__WIFI_SWITCH_SCHED_CHECK_H_*/
