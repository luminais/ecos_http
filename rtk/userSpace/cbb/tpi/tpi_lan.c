#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <net/if.h>
#include <sys/sockio.h>
#include <sys/syslog.h>

#include "debug.h"
#include "sys_timer.h"

extern void start_lan(void);
extern void stop_lan(void);
#ifdef __CONFIG_PPPOE_SERVER__
extern int lan_pppoe_start(int unit);
#endif

void tpi_lan_start_handle();
void tpi_lan_stop_handle();

static void tpi_lan_link_timer();
static void tpi_lan_link_timer_init();
extern int iflib_ioctl(char *ifname, int cmd, void *data);
static RET_INFO tpi_lan_start();
static RET_INFO tpi_lan_stop();
static RET_INFO tpi_lan_restart();

/*以下函数用于api调用*/
RET_INFO tpi_lan_update_info()
{	
	return RET_SUC;
}

RET_INFO tpi_lan_struct_init()
{
	return RET_SUC;
}

RET_INFO tpi_lan_first_init()
{
	tpi_lan_link_timer_init();		
	return tpi_lan_start();
}

RET_INFO tpi_lan_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_lan_start();
			break;
		case OP_STOP:
			tpi_lan_stop();
			break;
		case OP_RESTART:
			tpi_lan_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/

/*以下为该模块具体执行实现函数*/
void tpi_lan_start_handle(void)
{
	start_lan();
	return;
}

void tpi_lan_stop_handle(void)
{
	stop_lan();
	return;
}

/*在lan等其他线程可能会用到*/

/*只有本文件里面用*/
static void tpi_lan_link_timer()
{
	PIU8 i=0;
	static PIU8 lan[4]={0};  //根据coverity分析结果修改，原来为:static PIU8 lan[3]={0}	2017/1/10 F9项目
	PI8 info[15]={0};
	PIU8 max_lan_num = 3;//lan口数量
	
	//AP模式下所有port口都是lan口，并且插上网线时需要记录系统日志
	if(nvram_match(SYSCONFIG_WORKMODE, "bridge"))
	{
		max_lan_num = 4;
	}
	
	PI32 lan_link_status = 0;
	extern unsigned char tenda_show_phy_stats(int port);
	#if defined(__CONFIG_LAN1_PORT__) && defined(__CONFIG_LAN2_PORT__) && defined(__CONFIG_LAN3_PORT__) && defined(__CONFIG_WAN_PORT__)
	int port_gpio[4] = {__CONFIG_LAN1_PORT__,__CONFIG_LAN2_PORT__,__CONFIG_LAN3_PORT__,__CONFIG_WAN_PORT__};
	#else
	int port_gpio[4] = {3,2,1,0};
	#endif

	for(i = 1;i <= max_lan_num;i++)
	{
		lan_link_status = tenda_show_phy_stats(port_gpio[i-1]);

		if(lan_link_status != lan[i-1])
		{
			if(lan_link_status == 0)
			{
				sprintf(info,"lan%d down",i);
				syslog(LOG_INFO,info);
			}
			else
			{
				sprintf(info,"lan%d up",i);
				syslog(LOG_INFO,info);
			}
			lan[i-1] = lan_link_status;
		}	
	}
	
	return;
}

static void tpi_lan_link_timer_init()
{
	DO_TIMER_FUN timer;
	
	memset(&timer,0x0,sizeof(DO_TIMER_FUN));	
	strcpy(timer.name,LAN_LINK_TIMER);
	timer.enable = DO_TIMER_ON;			
	timer.sleep_time = DO_TIMER_MIN_TIME;
	timer.fun = tpi_lan_link_timer;
	sys_do_timer_add(&timer); 

	return;
}

static RET_INFO tpi_lan_start()
{
	tpi_lan_start_handle();
	PI_PRINTF(TPI,"start success!\n");	
	return RET_SUC;
}

static RET_INFO tpi_lan_stop()
{
	tpi_lan_stop_handle();
	PI_PRINTF(TPI,"stop success!\n");	
	return RET_SUC;
}

static RET_INFO tpi_lan_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_lan_stop() || RET_ERR == tpi_lan_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}
