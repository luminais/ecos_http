#ifndef __SYS_TIMER_H__
#define	__SYS_TIMER_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#define DO_TIMER_MIN_TIME (100)	/*最小定时周期*/

#define DO_TIMER_MAX_NAME (PI_BUFLEN_32)/*定时器名称最大长度*/
#ifdef __TENDA_MEM_H__
#define  MEMCHECK_TIMER       "memcheck"
#endif
#define STACK_GUARD               "stack_guard"
#define WATCHDOG_TIMER 		"watchdog"
#define SYSLED_TIMER 		"sysled"
#define BUTTON_TIMER 		"button"
#define HTTP_LOGIN_TIMER 	"http login"
#define LAN_LINK_TIMER 		"lan_link"
#define TENDA_ARP_TIMER 	"tenda_arp"
#define TC_STREAM_TIMER 	"tc stream"
#define TC_BRIDGE_STREAM_TIMER 	"bridge stream"
#ifdef __CONFIG_GUEST__
#define GUEST_NETWORK_TIMER 	"guest_network"
#endif
#define WAN_LINK_TIMER 		"wan_link"
#define PARENT_CTL_TIMER      "parent_ctl"
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
#define APCLIENT_TIMER 		"apclient"
#endif
#ifdef __CONFIG_WPS_RTK__
#define WPSLED_TIMER 		"wps_led"
#endif
#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
#define LAN_WANLED		"lan_wan_led"
//#define LAN_WAN_LED_TIMER 100
#endif


typedef enum
{
	DO_TIMER_OFF = 0,
	DO_TIMER_ON = 1
} DOTIMERINFO;

typedef struct do_timer_fun
{
	PI8 name[DO_TIMER_MAX_NAME];
	DOTIMERINFO enable;
	PIU32 sleep_time;
	PIU32 before_run_time;
	PIU32 next_run_time;
	void (*fun)();
	struct do_timer_fun *next;
}DO_TIMER_FUN,*P_DO_TIMER_FUN;

void sys_do_timer();
void sys_do_timer_add(P_DO_TIMER_FUN ptr);
void sys_do_timer_del(PI8 *name);
void sys_do_timer_action(DOTIMERINFO action,PI8 *name);
void sys_do_timer_show(void);

#ifdef __CONFIG_EXTEND_LED__
#define EXTENDLED_TIMER		"extendled"

void sys_extend_do_timer();
void sys_extend_do_timer_add(P_DO_TIMER_FUN ptr);
void sys_extend_do_timer_del(PI8 *name);
void sys_extend_do_timer_action(DOTIMERINFO action,PI8 *name);
void sys_extend_do_timer_show(void);
#endif
#endif/*__SYS_TIMER_H__*/
