/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <time.h>

#include "debug.h"
#include "common.h"
#include "switch_led.h"


#include "sys_module.h"

#define SWITCH_LED_SIZE	4*1024
static cyg_handle_t switch_led_daemon_handle;
static PI8 switch_led_daemon_stack[SWITCH_LED_SIZE];
static cyg_thread switch_led_daemon_thread;


extern void lan_wan_led_timer();
static RET_INFO tpi_switch_led_start();
static RET_INFO tpi_switch_led_stop();
static RET_INFO tpi_switch_led_restart();
static RET_INFO tpi_led_restart();


static int switch_led_module_exit_flag = 0;
inline void set_switch_led_exit_flag(int flag)
{
	switch_led_module_exit_flag = flag;
}

inline int get_switch_led_exit_flag()
{
	return switch_led_module_exit_flag;
}


RET_INFO tpi_switch_led_struct_init()
{
	return RET_SUC;
}

RET_INFO tpi_switch_led_first_init()
{
	tpi_switch_led_start();
	
	return RET_SUC;
}


RET_INFO tpi_switch_led_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_switch_led_start();
			break;
		case OP_STOP:
			tpi_switch_led_stop();
			break;
		case OP_RESTART:
			tpi_switch_led_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}


static RET_INFO tpi_switch_led_main()
{
	while(1)
	{
		if(1 == get_switch_led_exit_flag())
			break;
		
		lan_wan_led_timer();

		cyg_thread_delay(RC_MODULE_1S_TIME);
	}

	return RET_SUC;
}



RET_INFO tpi_switch_led_start()
{
	RET_INFO ret = RET_SUC;

	if(switch_led_daemon_handle == 0)
	{
		if(1)
		{
			cyg_thread_create(
				8, 
				(cyg_thread_entry_t *)tpi_switch_led_main,
				0, 
				"switch_led",
				switch_led_daemon_stack, 
				sizeof(switch_led_daemon_stack), 
				&switch_led_daemon_handle, 
				&switch_led_daemon_thread);
			
			cyg_thread_resume(switch_led_daemon_handle);     
			cyg_thread_delay(1);
			PI_PRINTF(TPI,"start success!\n");
		}
		else
		{
			PI_ERROR(TPI,"the mib is off, connot start!\n");
		}
	}
	else
	{
		PI_PRINTF(TPI,"is already start!\n");			
	}
	
	return ret;
}

#if 1
static RET_INFO tpi_switch_led_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if(switch_led_daemon_handle != 0)
	{
		//设置退出标志为退出lq
		set_switch_led_exit_flag(1);

		/* Wait until thread exit */
		pid = oslib_getpidbyname("switch_led");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
		
		cyg_thread_delete(switch_led_daemon_handle);
		
		PI_PRINTF(TPI,"stop success!\n");
		set_switch_led_exit_flag(0);
		switch_led_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");		
	}
	
	return ret;
}


static RET_INFO tpi_switch_led_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_switch_led_stop() || RET_ERR == tpi_switch_led_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;	
}

#endif



