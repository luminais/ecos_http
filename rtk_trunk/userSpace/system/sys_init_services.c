/*add by lqz 2015-12-17.
*
*
*/
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
#include "button.h"

#ifdef __CONFIG_EXTEND_LED__
#include "sys_extend_timer.h"
#include "wifi.h"
#endif

/*init_sys_services*/
static void init_cpu();
extern void cli_start();
static void init_sys_services();

/*init_do_timer_services*/
static void init_watchdog_timer();
static void init_do_timer_services();

extern void init_led_services();

/*init_net_services*/
extern void tapf_vlan_config();
extern RET_INFO br_init();
static void init_net_services();

/*init_button_services*/
extern void init_button_services(void);
#ifdef __CONFIG_PPPOE_SERVER__
extern void init_lan_pppoe_service(void);
#endif
/*模块初始化之前开启的服务*/
static struct services_funs before_module_init_services_funs_list[] =
{
    { init_sys_services },//初始化系统服务
    { init_do_timer_services },//初始化定时器服务
    { init_led_services },//初始化LED灯、GPIO
    { init_net_services },//初始化网络服务
};

/*模块初始化之后开启的服务*/
static struct services_funs after_module_init_services_funs_list[] =
{
    { init_button_services },//初始化按键
#ifdef __CONFIG_PPPOE_SERVER__
    { init_lan_pppoe_service },//初始lan pppoe server
#endif
};

void init_before_module_init_services()
{
    PIU8 i = 0;
    for(i = 0; i < ARRAY_SIZE(before_module_init_services_funs_list); i++)
    {
        before_module_init_services_funs_list[i].fun();
    }
    PI_PRINTF(MAIN,"init_before_module_init_services ok!\n");
    return;
}

void init_after_module_init_services()
{
    PIU8 i = 0;
    for(i = 0; i < ARRAY_SIZE(after_module_init_services_funs_list); i++)
    {
        after_module_init_services_funs_list[i].fun();
    }
    PI_PRINTF(MAIN,"init_after_module_init_services ok!\n");
    return;
}

/*init_sys_services*/
static void init_cpu()
{
#if 0
/*不知道去哪里了*/
    if (nvram_match("wait", "1"))
        cpu_idle_sleep(1);
#endif
    return;
}

void RTLWIFINIC_GPIO_write(unsigned int gpio_num, unsigned int value);
static void init_sys_services()
{
    init_cpu();

    syslog_init();

    syslog(LOG_INFO|LOG_DEBUG,"System start");

    cli_start();
	
#ifdef RTL819X
	/*init GPIO if needed*/
	gpio_init();
#endif
			
    return;	
}		

		
#if RTL819X
PI32 watchdog_enable = 1;  
#endif

void tapf_watchdog_disable(void)
{
	
	#if RTL819X
	watchdog_enable = 0;
	watchdog_reset();
	REG32(BSP_WDTCNR) |= (0xA5000000);		
	#endif
	
	return;
}


/*add by z10312 规避方法解决概率性mbuf内存耗尽网络不通问题
   sys_issue_check线程代码先临时放在这里,待后续整理监听线程再独立出来
*/
int g_eth_recv_out_mbuf = 0;

#define SYS_ISSUE_CHECK_STACK_SIZE	5*1024
static cyg_handle_t sys_issue_check_handle;
static char sys_issue_check_stack[SYS_ISSUE_CHECK_STACK_SIZE];
static cyg_thread sys_issue_check_thread;
static int sys_issue_check_thread_already_start = 0;
static int enable_out_mbuf_ping = 0;

extern int get_kern_mbuf_free_mem();
extern int whether_times_network_support_run();


int  tpi_sys_issue_check_main(void)
{  

	diag_printf ("%s %d \n", __FUNCTION__, __LINE__);
	int icmp_ping_error = 0;
	
	while(1)
	{
		
		if (enable_out_mbuf_ping)
		{
		
			unsigned int dst = inet_addr("127.0.0.1");
			dst = ntohl(dst);
			diag_printf ("%s %d \n", __FUNCTION__, __LINE__);
			if(-1 == icmp_ping(dst, 0, 3000))
			{
				diag_printf ("%s %d \n", __FUNCTION__, __LINE__);
				icmp_ping_error++;
				if (icmp_ping_error >= 2)  //出现了2次ping回环失败，则重启系统
				{
					diag_printf (" reboot test ok !!!! %s %d \n", __FUNCTION__, __LINE__);
					diag_printf (" reboot test ok !!!! %s %d \n", __FUNCTION__, __LINE__);
					do_reset(1);
					break;
				}
			}
			else
			{
				diag_printf ("%s %d \n", __FUNCTION__, __LINE__);
				enable_out_mbuf_ping = 0;	
				g_eth_recv_out_mbuf = 0;
				icmp_ping_error = 0;
			}
		}
		else
		{
			icmp_ping_error = 0;
		}
		
		cyg_thread_delay(200);	
	}
	
	return 0;
}



void tpi_sys_issue_check_start(void)
{
	
	cyg_thread_create(
		3,
		(cyg_thread_entry_t *) tpi_sys_issue_check_main,
		0, 
		"sys_issue_check",
		sys_issue_check_stack, 
		sizeof(sys_issue_check_stack), 
		&sys_issue_check_handle, 
		&sys_issue_check_thread);
		
	cyg_thread_resume(sys_issue_check_handle);     
	
	return ;
}

/* 规避方法解决run_sh0 线程sleep 问题*/
static int g_sh_main_sleep = 0;
extern int g_sh_main_exit;

void tapf_watchdog_enable(void)
{	
	
	
	//add by z10312 规避方法解决概率性mbuf内存耗尽网络不通问题
	if (g_eth_recv_out_mbuf)
	{
		
		if ((get_kern_mbuf_free_mem() <= 0 ) && whether_times_network_support_run())
		{
			
			enable_out_mbuf_ping = 1;
			if (!sys_issue_check_thread_already_start)
			{
				diag_printf ("%s %d \n", __FUNCTION__, __LINE__);
				sys_issue_check_thread_already_start = 1;
				tpi_sys_issue_check_start();
			}
		}
		else 
		{
			enable_out_mbuf_ping = 0;
			g_eth_recv_out_mbuf = 0;
		}
	}					
			
	#if RTL819X
	if (watchdog_enable)
	{
		watchdog_reset();
		watchdog_start();		
	}
	#endif	

	

	/* 规避方法解决run_sh0 线程sleep 问题*/
	#define MAX_SH_MAIN_TIMEOUT 35
	
	if ( g_sh_main_exit ) 
	{	
		g_sh_main_sleep++;
		if (g_sh_main_sleep > MAX_SH_MAIN_TIMEOUT)
		{
			printf ("do_reset  run_sh! \n");
			do_reset(1);
            return ;
		}
	}
	else
	{
		g_sh_main_sleep = 0;
	}
	
	return;
}	

static void init_watchdog_timer()
{
    DO_TIMER_FUN timer;
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));

	strcpy(timer.name,WATCHDOG_TIMER);
    timer.enable = DO_TIMER_ON;	
    timer.sleep_time = DO_TIMER_MIN_TIME;	
    timer.fun = tapf_watchdog_enable;				
    sys_do_timer_add(&timer);
	
    return;
}
/*lq  添加线程栈守卫*/
CYG_HAL_TABLE_BEGIN(__stack_check_tab_start__, stack_check_tab);/*将stack_check_tab_start指向stack_check_tab数据段首地址*/
CYG_HAL_TABLE_END(__stack_check_tab_end__, stack_check_tab);/*将stack_check_tab_end指向stack_check_tab数据段尾地址*/

void stack_guard_check(void)
{	
	struct stack_guard* temp = NULL;
	
	for(temp = __stack_check_tab_start__;temp != &__stack_check_tab_end__; temp++)
	{
		if(temp->front_guard[0]  != 0x4c || temp->front_guard[1] != 0x51)
		{
			printf("*********%s stack overflow!!!!!\n",temp->name);		
		}
		else if(temp->back_guard[0] != 0x4c || temp->back_guard[1] 	!= 0x51 )
		{
			printf("*********%s stack bottom soiled!!!!!\n",temp->name);
		}
	}
	return;
}

static void init_stack_guard_timer()
{
	struct stack_guard* temp = NULL;
	printf("this thread can be checked ! if thread stack overflow or soiled\n");
	for(temp = __stack_check_tab_start__;temp != &__stack_check_tab_end__; temp++)
	{
		temp->back_guard[0] 	= 0x4c;
		temp->back_guard[1] 	= 0x51;
		temp->front_guard[0] 	= 0x4c;
		temp->front_guard[1] 	= 0x51;
		printf("------------%s\n",temp->name);
	}
	
	DO_TIMER_FUN timer;
	memset(&timer,0x0,sizeof(DO_TIMER_FUN));

	strcpy(timer.name,STACK_GUARD);
	timer.enable = DO_TIMER_ON;	
	timer.sleep_time = DO_TIMER_MIN_TIME;	
	timer.fun = stack_guard_check;				
	sys_do_timer_add(&timer);
	
   	 return;
}

#ifdef __TENDA_MEM_H__
extern void memcheck_do_time();
static void init_memcheck_timer()
{
    DO_TIMER_FUN timer;
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
    strcpy(timer.name,MEMCHECK_TIMER);
    timer.enable = DO_TIMER_ON;	
    timer.sleep_time = DO_TIMER_MIN_TIME*300;	
    timer.fun = memcheck_do_time;				
    sys_do_timer_add(&timer);
	
    return;
}
#endif
static void init_do_timer_services()
{
    sys_do_timer();

#ifdef __CONFIG_EXTEND_LED__
	sys_extend_do_timer();
#endif
	
    init_watchdog_timer();
#ifdef __TENDA_MEM_H__
    init_memcheck_timer();
#endif
    init_stack_guard_timer();
    return;
}
#ifdef __CONFIG_IPTV__
extern void config_tenda_vlan(void);
#endif
/*init_net_services*/
static void init_net_services()
{
    lo_set_ip();
#ifdef __CONFIG_IPTV__
    config_tenda_vlan();
#endif
#if 0
/*CONFIG_VLAN宏不知道去哪里了*/
#ifdef  CONFIG_VLAN
	tapf_vlan_config();
#endif
#else
	tapf_vlan_config();
#endif
	
	RunSystemCmd(0, "ipfw", "add", "65534", "allow", "ip", "from", "any", "to", "any", "");

	br_init();

    return;
}

/*以下为系统级函数实现*/
extern void do_reset(int type);
void tapf_board_reboot(void)
{	
#if REALTEK	
	printf (" %s %d \n", __FUNCTION__, __LINE__);
	do_reset(1);	
#endif
	return;
}

void sys_reboot()
{
	PI8 ptr_str[PI_BUFLEN_32] = {0};
	sprintf(ptr_str,"string_info=reboot");
	msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,ptr_str);

	cyg_thread_delay(10*RC_MODULE_1S_TIME);
	do_reset(1);
	
	return;
}

void sys_before_upgrade(void)
{
//升级前调用此函数,释放一些系统资源
	PI8 ptr_str[PI_BUFLEN_32] = {0};
	sprintf(ptr_str,"op=2");
	msg_send(MODULE_RC,RC_WAN_MODULE,ptr_str);
	
	cyg_thread_delay(200);
	return;
}

void sys_restart2(void)
{
	return;
}
