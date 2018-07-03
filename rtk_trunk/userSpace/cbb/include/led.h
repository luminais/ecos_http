#ifndef __WIFI_SWITCH_SCHED_CHECK_H_
#define __WIFI_SWITCH_SCHED_CHECK_H_

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif


//LED的控制类型,常开,常关,定时关闭
typedef enum
{
	LED_CTL_TYPE_ON = 0,		//常开
	LED_CTL_TYPE_OFF,	//常关
	LED_CTL_TYPE_DURATION,	//定时关闭
}LED_CTL_TYPE;

//LED的状态
typedef enum
{
	LED_STA_OFF = 0,
	LED_STA_ON,
	LED_STA_MAX,
}LED_STATUS;

struct timer_period
{
	PI32 is_time_zone_east;
	PI32 time_zone;
	PIU32 timer;     //起始时间（从一天的0时计算，单位为s）  
};

typedef struct led_timer_info_struct
{
	PIU8 enable;	
	PI8 sel_week[7];                  //选择的星期，since sunday ,if workday selected, set {0,1,1,1,1,1,0}
	PIU8 ctl_type;			//LED的控制类型,0:常关,1:常关，2:定时关闭
	PIU8 time_flag;			//用于标记起始时间大于结束时间,0:str_time < end_tim,1:str_time > end_time
	struct timer_period led_timer_strtime;           
	struct timer_period led_timer_endtime; 

}LED_TIMER_CHECK_INFO_STRUCT,*P_LED_TIMER_CHECK_INFO_STRUCT;

/*API*/

/*GPI*/

/*TPI*/
extern RET_INFO tpi_led_update_info();
extern RET_INFO tpi_led_struct_init();
extern RET_INFO tpi_led_first_init();
extern RET_INFO tpi_led_action(RC_MODULES_COMMON_STRUCT *var);

extern RET_INFO tpi_wifi_switch_sched_web_info(PI8 *enable,PI8 *times,PI8 *weeks);

extern void tpi_wifi_switch_sched_debug_level(PI32 level);
#endif/*__WIFI_SWITCH_SCHED_CHECK_H_*/
