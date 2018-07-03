/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : sys_led.c
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2016年11月16日
  最近修改   :
  功能描述   : 

  功能描述   : LED的初始化以及状态处理

  修改历史   :
  1.日    期   : 2016年11月16日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <sys/param.h>
#include <osl.h>
#include <siutils.h>
#include <gpio_api.h>
#include <sys/syslog.h>
#include <cyg/io/watchdog.h>

#include "sys_module.h"
#include "sys_init_services.h"
#include "sys_timer.h"

#ifdef __CONFIG_EXTEND_LED__
#include "sys_extend_timer.h"
#include "wifi.h"
#endif

#ifdef __CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__
#include "sys_lan_wan_led.h"
static void init_lan_wan_led_timer();
#endif

/*init_led_services*/
static void sys_led_timer();
static void init_sys_led_timer();
#ifdef __CONFIG_WPS_RTK__
static void init_wps_led_timer();
#endif

#ifdef __CONFIG_EXTEND_LED__
static void extend_led_start();
#endif
#ifdef __CONFIG_WL_LED__
extern void wl_led_test_init(void);
extern void wl_led_test_on(void);
extern void wl_led_test_off(void);
#endif/*__CONFIG_WL_LED__*/
#ifdef __CONFIG_WPS__
extern int wps_gpio_btn_init();
#endif/*__CONFIG_WPS__*/
#ifdef __CONFIG_WPS_LED__
extern void wps_led_test_off();
extern void wps_led_test_on();
extern void wps_led_test_init(void);
static void wps_led_test(void);
#endif/*__CONFIG_WPS_LED__*/
#ifdef __CONFIG_LED__
int sys_led_turn = 1;  //用于控制系统灯是否翻转
int set_sys_led_turn(int turn_flag)
{
	if(1 == turn_flag)
	{
		sys_led_turn = 1;
	}
	else
	{
		sys_led_turn = 0;
	}
	return 0;
}
#endif
/*init_led_services*/
void sys_led_timer()
{
    static PI32 sys_led_on = 1;

	sys_led_turn_on_off(sys_led_on);
#ifdef __CONFIG_LED__
	if(sys_led_turn == 0)
	{
		sys_led_on = 1;
	}
	else	//系统灯闪烁
#endif
	{
    sys_led_on = (sys_led_on?0:1);
	}

    return;
}

static void init_sys_led_timer()
{
    DO_TIMER_FUN timer;

    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,SYSLED_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = sys_led_timer;
    sys_do_timer_add(&timer);

    return;
}
#ifdef __CONFIG_WPS_RTK__
/*****************************************************************************
 函 数 名  : init_wps_led_timer
 功能描述  : 初始化wps指示灯定时器
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2016年10月9日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
static void init_wps_led_timer()
{
    DO_TIMER_FUN timer;

    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,WPSLED_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = wps_led_timer;
    sys_do_timer_add(&timer);

    return;
}
#endif

#ifdef __CONFIG_EXTEND_LED__
int extend_led_stop_status = 0;
static int led_thread_type = 0;
void extend_led_stop()
{
	int pid;
	char tname[64];

	sprintf(tname, "LED");
	pid = oslib_getpidbyname(tname);
	if (pid == 0)
		return;
	extend_led_stop_status = 1;
	while (oslib_waitpid(pid, NULL) != 0)
		cyg_thread_delay(1);
	led_thread_type = 0;
	return;
}
int wps_button_action = 0;
static void extend_led_main_loop()
{
	int wait_time  = 25;
	static PI32 sys_led_on = 1;
	int wifi_status = 0, wan_status = 0;
	WIFI_BRIDGE_INFO_STRUCT bridge_info;
	static int only_wps_start = 1;
	int once_coming = 1;
	/*lq add 添加 get_wan_status、check_times使调用gpi_wifi_get_status_info
	函数的次数由之前的每秒4~6次减小为1次，从而减小多个线程高频率调用包含static变量的函数造成死机的概率*/
	int get_wan_status = 0;
	int check_times = 4;
	
	while(1)
	{	
		if(extend_led_stop_status == 1)
		{
			extend_led_stop_status = 0;
			break;
		}
		else if(extend_led_blink())
		{
			if(wps_button_action == 1)
			{
				if(only_wps_start == 1)
				{
					wait_time = 300;
					only_wps_start = 0;
					sys_led_on = 0;
					sys_led_turn_on_off(sys_led_on);
				}
				else
				{
					sys_led_turn_on_off(sys_led_on);
					sys_led_on = (sys_led_on?0:1);
					wait_time = 16;
				}
			}
			else
			{
				sys_led_turn_on_off(sys_led_on);
				sys_led_on = (sys_led_on?0:1);
				wait_time = 25;
			}
			
		}
		else
		{
			if(get_wan_status >= check_times)
			{
				gpi_wifi_get_status_info(&wan_status, &wifi_status);
				
				if(WAN_CONNECTED == wan_status)
				{
					gpi_wifi_get_bridge_rssi(&bridge_info);
				}
				
				get_wan_status = 0;
			}
			
			only_wps_start = 1;
			wait_time  = 25;
			
			check_times = 4;
			get_wan_status++;
			
			wps_button_action = 0;
			if(WAN_CONNECTED == wan_status)
			{
				if(bridge_info.rssi >= 40)
				{
					extend_and_sys_led_on_off(LED_ON,LED_OFF);//绿色常亮
				}
				else if(bridge_info.rssi <= 25)
				{
					extend_and_sys_led_on_off(sys_led_on,sys_led_on);//橙色闪烁
					sys_led_on = (sys_led_on?0:1);
					wait_time = 16;
					check_times = 6;
				}
				else
				{
					extend_and_sys_led_on_off(LED_ON,LED_ON);//橙色常亮
				}
			}
			else
			{
				//关掉扩展灯:绿色   闪烁系统灯:红色
				extend_and_sys_led_on_off(LED_OFF,sys_led_on);
				sys_led_on = (sys_led_on?0:1);
			}
		}
		cyg_thread_delay(wait_time);
			
	}
}
static char led_daemon_stack[4096];
static cyg_handle_t led_daemon_handle;
static cyg_thread led_daemon_thread;

/*lq 添加扩展灯线程，原因：需要1秒内闪四次，定时器最小单位为500ms无法搬到，所以使用线程代替*/
void extend_led_start()
{
	if(led_thread_type == 0)
	{
		 cyg_thread_create( 8,
                       (cyg_thread_entry_t *)extend_led_main_loop,
                       0,
                       "LED",
                       &led_daemon_stack,
                       sizeof(led_daemon_stack),
                       &led_daemon_handle,
                       &led_daemon_thread);
		cyg_thread_resume(led_daemon_handle);
	}
}
#endif

/*************************************************************/

#ifdef __CONFIG_INDICATE_LED__
static int indicate_led_thread_type = 0;
static char indicate_led_daemon_stack[2048];
static cyg_handle_t indicate_led_daemon_handle;
static cyg_thread indicate_led_daemon_thread;
typedef enum {
	GPIO = 0,
	INDICATE_EXIT,
	INDICATE_LED_ON,
	INDICATE_LED_BLINK
}INDICATE_TYPE;

static int indicate_type = 0; 
/*****************************************************************************
 函 数 名  : indicate_led_main_loop
 功能描述  : 控制所有LAN口灯的闪烁
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2016年11月16日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static void indicate_led_main_loop()
{
	int wait_time  = 16;
	int wait_times  = 0;
       int state = 0;
	while(1)
	{	
		switch(indicate_type)
		{
			case INDICATE_LED_ON:
				lan_port_led_on_off(1);
				wait_time = 50;
				wait_times++;
				break;
			case INDICATE_LED_BLINK:	
				lan_port_led_on_off(state);
				state = (~state)&1;
				wait_time = 16;
				break;
			case INDICATE_EXIT:
				set_lan_port_as_led();
				return;
			default:
				wait_time = 16;
				break;
		}
		
		cyg_thread_delay(wait_time);
		
		if(indicate_type == INDICATE_LED_ON && wait_times == 7)
		{
			state = 0;
			wait_times = 0;
			set_lan_port_as_led();
			indicate_type = GPIO;
		}
	}
}
/*****************************************************************************
 函 数 名  : set_indicate_led_off
 功能描述  : 关闭gpio的控制
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月16日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
typedef enum{
	INDICATE_LED_OFF = 0,
	INDICATE_LED_EXIT
}LED_OFF_TYPE;
int set_indicate_led_off(int type)
{
	if(type == INDICATE_LED_EXIT)
	{
		indicate_type = INDICATE_EXIT;
	}
	else
	{
		set_lan_port_as_led();
		indicate_type = GPIO;
	}
	return indicate_type;
}

/*****************************************************************************
 函 数 名  : set_indicate_led_on
 功能描述  : 设置led全开
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月16日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int set_indicate_led_on()
{
	if(indicate_type != INDICATE_EXIT)
	{
		indicate_type = INDICATE_LED_ON;
	}
	return indicate_type;
}

/*****************************************************************************
 函 数 名  : set_indicate_led_blink
 功能描述  : 设置lan口闪烁，如果返回-1,则标识正在执行一次任务，否则设置
 输入参数  : int wait_time  设置等待时间，也就是闪烁的时间，设置为0，表示一直闪烁
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月16日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int set_indicate_led_blink(int wait_time)
{
	if(indicate_type == INDICATE_EXIT)
		return -1;
	
	if(indicate_type == INDICATE_LED_BLINK || indicate_type == INDICATE_LED_ON)
		return -1;
	
	set_lan_port_as_gpio();
	indicate_type = INDICATE_LED_BLINK;

	if(wait_time > 0)
	{
		cyg_thread_delay(wait_time * 100);
		indicate_type = INDICATE_LED_ON;
	}
	return 1;
}

void indicate_led_start()
{
	if(indicate_led_thread_type == 0)
	{
		 cyg_thread_create( 6,
                       (cyg_thread_entry_t *)indicate_led_main_loop,
                       0,
                       "indicate_led",
                       &indicate_led_daemon_stack,
                       sizeof(indicate_led_daemon_stack),
                       &indicate_led_daemon_handle,
                       &indicate_led_daemon_thread);
		cyg_thread_resume(indicate_led_daemon_handle);
		indicate_led_thread_type = 1;
	}
}

#endif
/*************************************************************/

#ifdef __CONFIG_WL_LED__
PI32 wl_led_init_g = 1;  //route system init wl led status
static void wl_led_test()
{
    PI8 wl_enable[4];
    strcpy(wl_enable, nvram_safe_get("wl0_radio"));
    wl_led_init_g = 1;
    if(strcmp(wl_enable, "1") == 0)
    {
        PI_PRINTF(MAIN,"wireless led on\n");
        wl_led_test_on();
    }
    else
    {
        PI_PRINTF(MAIN,"wireless led off\n");
        wl_led_test_off();
    }
    wl_led_init_g = 0;
    return;
}
#endif /*__CONFIG_WL_LED__*/

#ifdef __CONFIG_WPS_LED__
PI32 wps_led_init_g  = 1;   //route system init wps led status
static void wps_led_test()
{
    PI8 wl_enable[4];
    PI8 wps_enable[16];

    strcpy(wl_enable,  nvram_safe_get("wl0_radio"));
    strcpy(wps_enable, nvram_safe_get("wps_mode"));
    wps_led_init_g = 1;
    if(strcmp(wl_enable, "1") == 0 && strcmp(wps_enable, "enabled") == 0)
    {
        wps_led_test_on();
    }
    else
    {
        wps_led_test_off();
    }
    wps_led_init_g = 0;
    return;
}
#endif/*__CONFIG_WPS_LED__*/

void init_led_services()
{
#if defined(__CONFIG_SYS_LED__) || defined(__CONFIG_TENDA_HTTPD_UCD__)
    init_sys_led_timer();
#endif

#ifdef __CONFIG_EXTEND_LED__
	extend_led_start();
#endif

#ifdef __CONFIG_INDICATE_LED__
	if(nvram_match("restore_quick_set","1"))
		indicate_led_start();
#endif

#ifdef __CONFIG_WL_LED__
    wl_led_test_init();
    wl_led_test();  //wireless led on or off
#endif

#ifdef __CONFIG_WPS__
    wps_gpio_btn_init();
#endif

#ifdef __CONFIG_WPS_LED__
    wps_led_test_init();
    wps_led_test(); //wps led on or off
#endif
#ifdef __CONFIG_WPS_RTK__
	init_wps_led_timer();
#endif



    return;
}

