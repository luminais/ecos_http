#ifndef __WIFI_SWITCH_SCHED_CHECK_H_
#define __WIFI_SWITCH_SCHED_CHECK_H_

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#define WLAN_SCHEDULE_LIST_MAX	10

/* 记录wifi定时当前时间的上一个时间*/
struct wifi_timer{
	PI32 wday;
	PI32 time;
};

struct timer_period
{
	PI32 is_time_zone_east;
	PI32 time_zone;
	PIU32 timer;     //起始时间（从一天的0时计算，单位为s）  
	PI8 select_week[7]; 
};

typedef struct wifi_switch_sched_info_struct
{
	PIU8 enable;	
	PI8 sel_week[7];                  //选择的星期，since sunday ,if workday selected, set {0,1,1,1,1,1,0}
	PI32 sche_count;
	struct timer_period timer_period_offtime[WLAN_SCHEDULE_LIST_MAX];           
	struct timer_period timer_period_ontime[WLAN_SCHEDULE_LIST_MAX]; 

}WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT,*P_WIFI_SWITCH_SCHED_CHECK_INFO_STRUCT;

/*API*/

/*GPI*/

/*TPI*/
extern RET_INFO tpi_wifi_switch_sched_update_info();
extern RET_INFO tpi_wifi_switch_sched_struct_init();
extern RET_INFO tpi_wifi_switch_sched_first_init();
extern RET_INFO tpi_wifi_switch_sched_action(RC_MODULES_COMMON_STRUCT *var);

extern RET_INFO tpi_wifi_switch_sched_web_info(PI8 *enable,PI8 *times,PI8 *weeks);

extern void tpi_wifi_switch_sched_debug_level(PI32 level);
#endif/*__WIFI_SWITCH_SCHED_CHECK_H_*/
