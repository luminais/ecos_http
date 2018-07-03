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
#include "led.h"


#include "sys_module.h"



#define LED_TIME_CHECK_SIZE	4*1024
static cyg_handle_t led_time_check_daemon_handle;
static PI8 led_time_check_daemon_stack[LED_TIME_CHECK_SIZE];
static cyg_thread led_time_check_daemon_thread;

static LED_TIMER_CHECK_INFO_STRUCT led_timer_check_info;



static RET_INFO tpi_led_main();
static RET_INFO tpi_led_start();
static RET_INFO tpi_led_stop();
static RET_INFO tpi_led_restart();

static LED_STATUS tpi_led_get_status();			//获取LED的当前状态,LED_STA_OFF:关闭,LED_STA_ON:开启
static int tpi_led_set(int action);				//开启或关闭LED,0:关闭 1:开启
//计算休眠时间,以及返回当前时间LED所应处在的状态
static LED_STATUS tpi_led_check_time_area(struct tm *pt, P_LED_TIMER_CHECK_INFO_STRUCT led_info);
//判断当前时间是否在LED定时时间内,返回LED下一个变更状态
static LED_STATUS tpi_led_check_time_in_range(P_LED_TIMER_CHECK_INFO_STRUCT led_info);


extern int tenda_set_all_led_off(void);
extern int tenda_set_all_led_on(void);



static int check_led_timer_module_exit_flag = 0;
inline void set_led_timer_exit_flag(int flag)
{
	check_led_timer_module_exit_flag = flag;
}

inline int get_led_timer_exit_flag()
{
	return check_led_timer_module_exit_flag;
}


/***************************************************************************
描述:tpi_set_all_led_off
参数:
	
	
返回值:
	
作者:
日期:
***************************************************************************/
int tpi_set_all_led_off()
{
	char buf[64] = {0};
	//所有LED灯
	tenda_set_all_led_off();

	//关闭射频灯
	sprintf(buf,"wlan0 led 0");
    run_clicmd(buf);
	memset(buf,0x0,sizeof(buf));
	sprintf(buf,"wlan1 led 0");
    run_clicmd(buf);
	
	return 0;
}


/***************************************************************************
描述:tpi_set_all_led_auto
参数:
	
	
返回值:
	
作者:
日期:
***************************************************************************/
int tpi_set_all_led_auto()
{
	char buf[64] = {0};
	//所有LED灯
	tenda_set_all_led_on();

	//开启射频灯
	sprintf(buf,"wlan0 led 3");
	run_clicmd(buf);
	
	memset(buf,0x0,sizeof(buf));
	sprintf(buf,"wlan1 led 3");
	run_clicmd(buf);

	if(INTERFACE_UP == get_interface_state(TENDA_WLAN24_AP_IFNAME))
	{
		memset(buf,0x0,sizeof(buf));
		sprintf(buf,"wlan1 led 2");
		run_clicmd(buf);
	}

	if(INTERFACE_UP == get_interface_state(TENDA_WLAN5_AP_IFNAME))
	{
		memset(buf,0x0,sizeof(buf));
		sprintf(buf,"wlan0 led 2");
		run_clicmd(buf);
	}

	
	return 0;
}



static int tpi_led_set(int action)				//开启或关闭LED,0:关闭 1:开启
{
	LED_STATUS led_cur_sta = LED_STA_ON;

	if(LED_STA_OFF == action)
	{
		tpi_set_all_led_off();
	}
	else
	{
		tpi_set_all_led_auto();
	}
	
	return 0;
}



/***************************************************************************
描述:计算当前时间是否在LED定时关闭时间段内
参数:
	sleep_time:当前时间点到led下个状态切换时间点的时间
	pt:
	led_info:描述LED结构体相关的信息
返回值:
	0:当前时间在定时关闭时间段外 1:当前时间在定时关闭时间段内
作者:
日期:
***************************************************************************/
static LED_STATUS tpi_led_check_time_area(struct tm *pt, P_LED_TIMER_CHECK_INFO_STRUCT led_info)
{
	LED_STATUS led_cur_status = LED_STA_ON;
	PIU32 curtime = 0;
	PI32 offtimer = 0,ontimer = 0;
	PI32 on_time_pre = -1 , off_time_pre = -1;
	PI32 on_time_next = -1 , off_time_next = -1;
	int ret = 0;
	
	curtime = (pt->tm_hour * 60 * 60) + (pt->tm_min * 60) + pt->tm_sec;
	if(curtime < 0 || curtime > ONE_DAY_IN_SEC)
	{
		return ret;
	}

	//启动时间
	ontimer = led_info->led_timer_strtime.timer;
	//当前时间大于启动时间
	if(curtime >= ontimer)
	{
		//获取所有规则中离当前时间最近的前一个开启时间
		on_time_pre = ontimer;
			
	}
	//当前时间小于启动时间
	else
	{
		//获取所有的规则中离当前时间最近的下一个开启时间
		on_time_next = ontimer;
		
	}

	//关闭时间
	offtimer = led_info->led_timer_endtime.timer;
	
	if(curtime >= offtimer)
	{
		//获取所有规则中离当前时间最近的前一个关闭时间
		off_time_pre = offtimer;		
	}
	else
	{
		//获取所有规则中离当前时间最近的下一个关闭时间
		off_time_next = offtimer;
	}

	if(0 == led_info->time_flag)
	{
		if((curtime >= led_info->led_timer_strtime.timer) && (curtime <= led_info->led_timer_endtime.timer))
		{
			ret = 1;
		}
		else
		{
			ret = 0;
		}
	}
	else
	{
		if((curtime >= led_info->led_timer_strtime.timer) || (curtime <= led_info->led_timer_endtime.timer))
		{
			ret = 1;
		}
		else
		{
			ret = 0;
		}
	}

	return ret;
}


/***************************************************************************
描述:获取当前时间点LED所应处在的状态
参数:
	sleep_time:当前时间点到led下个状态切换时间点的时间
	led_info:描述LED结构体相关的信息
返回值:
	当前时间点上led所应处在的状态
作者:
日期:
***************************************************************************/

static LED_STATUS tpi_led_check_time_in_range(P_LED_TIMER_CHECK_INFO_STRUCT led_info)
{
	LED_STATUS led_status = LED_STA_ON;
	time_t now = 0;
	struct tm TM;
	int ret = 0;

	now = time(0);
	gmtime_r(&now,&TM);

	ret = tpi_led_check_time_area(&TM,led_info);
	
	return ret;
}





RET_INFO tpi_led_update_info()
{
	LED_CTL_TYPE led_ctl_type = LED_CTL_TYPE_ON;
	PI8 *mib_val = NULL;
	char value[32] = {0};
	char led_time[64] = {0};
	PIU32 str_hour = 0,end_hour = 0;
	PIU32 str_min = 0,end_min = 0;

	memset(&led_timer_check_info,0x0,sizeof(led_timer_check_info));

	//获取LED的控制类型,常开,常关,定时关闭
	strcpy(value,nvram_safe_get(SAVE_POWER_LED_TYPE));
	if(0 == strcmp("2",value))
	{
		led_timer_check_info.ctl_type = LED_CTL_TYPE_DURATION;
	}
	else if(0 == strcmp("1",value))
	{
		led_timer_check_info.ctl_type = LED_CTL_TYPE_OFF;
	}
	else
	{
		led_timer_check_info.ctl_type = LED_CTL_TYPE_ON;
	}

	//更新led的定时关闭时间段
	memset(led_time,0x0,sizeof(led_time));
	strcpy(led_time,nvram_safe_get(SAVE_POWER_LED_TIME));
	//sscanf(led_time, "%[^:]:%[^-]-%[^:]:%s", str_time[0], str_time[1], end_time[0], end_time[1]);
	sscanf(led_time,"%d:%d-%d:%d",&str_hour,&str_min,&end_hour,&end_min);
	led_timer_check_info.led_timer_strtime.timer = (str_hour * 60 * 60) + (str_min * 60);
	led_timer_check_info.led_timer_endtime.timer = (end_hour * 60 * 60) + (end_min * 60);
	if(led_timer_check_info.led_timer_strtime.timer <= led_timer_check_info.led_timer_endtime.timer)
	{
		led_timer_check_info.time_flag = 0;
	}
	else
	{
		led_timer_check_info.time_flag = 1;
 	}

	return RET_SUC;
}
/*获取当前led的状态*/
LED_STATUS current_status = LED_STA_MAX;
LED_STATUS tpi_led_current_state()
{
	return current_status;
}

RET_INFO tpi_led_struct_init()
{
	return RET_SUC;
}


RET_INFO tpi_led_first_init()
{
	//if(wifi_switch_sched_info.enable)
	//	tpi_wifi_switch_sched_start();
	tpi_led_start();
	
	return RET_SUC;
}


RET_INFO tpi_led_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_led_start();
			break;
		case OP_STOP:
			tpi_led_stop();
			break;
		case OP_RESTART:
			tpi_led_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

static RET_INFO tpi_led_main()
{
	TIME_UPDATE_RESULT time_update_status = TIME_UPDATE_FAIL;
	PIU32 times = 0;
	int time_in_area = 0;
	LED_CTL_TYPE led_ctl_type = LED_CTL_TYPE_ON;
	LED_STATUS led_sta = LED_STA_ON;
	LED_STATUS led_status_to_change = LED_STA_ON;
	LED_STATUS last_status = LED_STA_MAX;

	//从nvram中更新费配置参数
	tpi_led_update_info();

	//获取灯的当前状态以及控制方式,如果控制方式为常开或者常关,则执行相应的动作之后直接退出线程
	led_ctl_type = led_timer_check_info.ctl_type;

	if(LED_CTL_TYPE_ON == led_ctl_type)	//常开
	{
		tpi_led_set(LED_STA_ON);
		current_status = LED_STA_ON;
		goto finish;
	}
	else if(LED_CTL_TYPE_OFF == led_ctl_type)	//常关
	{
		cyg_thread_delay(50);
		tpi_led_set(LED_STA_OFF);
		current_status = LED_STA_OFF;
		goto finish;
	}

	while(1)
	{
		if(1 == get_led_timer_exit_flag())
			break;
	
		//从nvram中更新配置参数
		tpi_led_update_info();

		time_update_status = gpi_common_time_update_result();
		//检查网络更新
		if(TIME_UPDATE_FAIL == time_update_status)
		{
			goto next;
		}
		time_in_area = tpi_led_check_time_in_range(&led_timer_check_info);
		if(1 == time_in_area)
		{
			if(last_status != LED_STA_OFF)
			{
				tpi_led_set(LED_STA_OFF);
				last_status = LED_STA_OFF;
				current_status = LED_STA_OFF;
			}
		}
		else
		{
			if(last_status != LED_STA_ON)
			{
				tpi_led_set(LED_STA_ON);
				last_status = LED_STA_ON;
				current_status = LED_STA_ON;
			}
		}

next:
		//线程休眠
		for(times = 0; times < 5; times++)
		{
			if(1 == get_led_timer_exit_flag())
				break;
			cyg_thread_delay(RC_MODULE_1S_TIME);
		}
	}
finish:
	
	return RET_SUC;
}




RET_INFO tpi_led_start()
{
	RET_INFO ret = RET_SUC;

	if(led_time_check_daemon_handle == 0)
	{
		tpi_led_update_info();

		if(1)
		{
			cyg_thread_create(
				8, 
				(cyg_thread_entry_t *)tpi_led_main,
				0, 
				"led_time_check",
				led_time_check_daemon_stack, 
				sizeof(led_time_check_daemon_stack), 
				&led_time_check_daemon_handle, 
				&led_time_check_daemon_thread);
			
			cyg_thread_resume(led_time_check_daemon_handle);     
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


static RET_INFO tpi_led_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if(led_time_check_daemon_handle != 0)
	{
		//设置退出标志为退出lq
		set_led_timer_exit_flag(1);

		/* Wait until thread exit */
		pid = oslib_getpidbyname("led_time_check");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
		
		cyg_thread_delete(led_time_check_daemon_handle);
		
		PI_PRINTF(TPI,"stop success!\n");
		set_led_timer_exit_flag(0);
		led_time_check_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");		
	}
	
	return ret;
}


static RET_INFO tpi_led_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_led_stop() || RET_ERR == tpi_led_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;	
}





