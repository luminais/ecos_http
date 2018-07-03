#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_timer.h"
#include "bcmtc.h"

#if 0
static P_TC_CLIENT_INFO_STRUCT head;
#endif

/* init of tenda_arp */
extern void tenda_arp_init(void);
extern void del_stream_control();
extern void stream_tmr_func(void);
extern void bridge_stream_tmr_func(void);
void tpi_tc_start_handle();
void tpi_tc_stop_handle();

static void tpi_tc_timer();
static void tpi_tc_timer_init();
static RET_INFO tpi_tc_start();
static RET_INFO tpi_tc_stop();
static RET_INFO tpi_tc_restart();
static PI32 tpi_do_stream_timer();
static PI32 tpi_do_bridge_stream_timer();
RET_INFO tpi_tc_struct_init()
{
	return RET_SUC;
}

RET_INFO tpi_tc_first_init()
{
#if 0
	head = NULL;
#endif
	tenda_arp_init();
	init_stream_ip();
	enable_tc();
	tpi_do_stream_timer();
	tpi_do_bridge_stream_timer();
	tpi_tc_timer_init();
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	load_remark_config();
#endif
	return RET_SUC;
}

RET_INFO tpi_tc_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_tc_start();
			break;
		case OP_STOP:
			tpi_tc_stop();
			break;
		case OP_RESTART:
			tpi_tc_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/

/*以下为该模块具体执行实现函数*/
void tpi_tc_start_handle(void)
{
	new_init_stream_control();
	return;
}

void tpi_tc_stop_handle(void)
{
	del_stream_control();
	return;
}

/*在其他线程可能会用到*/

/*只有本文件里面用*/
static void tpi_tc_timer()
{
	tenda_arp_update_all();
	return;
}
static void tpi_tc_timer_init()
{
    DO_TIMER_FUN timer;
	
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,TENDA_ARP_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = tpi_tc_timer;
    sys_do_timer_add(&timer);
	
	return;
}


static PI32 tpi_do_stream_timer()
{
	DO_TIMER_FUN timer;
	
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,TC_STREAM_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = stream_tmr_func;
    sys_do_timer_add(&timer);
	return 0;
}

static PI32 tpi_do_bridge_stream_timer()
{
	DO_TIMER_FUN timer;
	
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,TC_BRIDGE_STREAM_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = bridge_stream_tmr_func;
    sys_do_timer_add(&timer);
	return 0;
}
static RET_INFO tpi_tc_start()
{
	tpi_tc_start_handle();
	PI_PRINTF(TPI,"start success!\n");	
	return RET_SUC;
}

static RET_INFO tpi_tc_stop()
{
	tpi_tc_stop_handle();
	PI_PRINTF(TPI,"stop success!\n");	
	return RET_SUC;
}

static RET_INFO tpi_tc_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_tc_stop() || RET_ERR == tpi_tc_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}
