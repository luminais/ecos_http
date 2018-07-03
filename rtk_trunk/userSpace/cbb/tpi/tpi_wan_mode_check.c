/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "debug.h"
#include "wan_mode_check.h"

#define WAN_MODE_CHECK_SIZE	12*1024
static cyg_handle_t wan_mode_daemon_handle;
static char wan_mode_daemon_stack[WAN_MODE_CHECK_SIZE];
static cyg_thread wan_mode_daemon_thread;

extern int tenda_wan_link_status();

static WAN_MODE_CHECK_INFO_STRUCT wan_mode_check_info;

static RET_INFO tpi_wan_mode_check_pppoe_dhcp();
static RET_INFO tpi_wan_mode_check_main();
static RET_INFO tpi_wan_mode_check_start();
static RET_INFO tpi_wan_mode_check_stop();
static RET_INFO tpi_wan_mode_check_restart();

/*以下函数用于api调用*/
RET_INFO tpi_wan_mode_check_update_info()
{
	PI8 wan_mode[PI_BUFLEN_16] = {0};

	if((strcmp(nvram_safe_get("mode_need_switch"),"yes") == 0 || strcmp(nvram_safe_get("restore_quick_set"),"1") == 0) && (0 == strcmp(nvram_safe_get("wl0_mode"),"ap")))
	{
		wan_mode_check_info.enable = 1;
		wan_mode_check_info.type = CHECK_PPPOE_DHCP;
		wan_mode_check_info.check_finish_tag = CHECK_ING;

		strcpy__(wan_mode,nvram_safe_get("wan0_proto"));

		if(strcmp(wan_mode,"pppoe") == 0)
		{
			wan_mode_check_info.wan_mode = WAN_PPPOE_MODE;
		}
		else if(strcmp(wan_mode,"dhcp") == 0)
		{
			wan_mode_check_info.wan_mode = WAN_DHCP_MODE;
		}
		else if(strcmp(wan_mode,"static") == 0)
		{
			wan_mode_check_info.wan_mode = WAN_STATIC_MODE;
		}
		else
		{
			wan_mode_check_info.wan_mode = WAN_MAX_MODE;
		}

		wan_mode_check_info.check_result = WAN_MAX_MODE;
		
		wan_mode_check_info.discover_send_num = 0;
		wan_mode_check_info.offer_rcv_tag = 0;
		
		wan_mode_check_info.padi_send_num = 0;
		wan_mode_check_info.pado_rcv_tag = 0;
		
	}
	else
	{
		wan_mode_check_info.enable = 0;		
		wan_mode_check_info.check_finish_tag = CHECK_END;
		wan_mode_check_info.check_result = WAN_NONE_MODE;
		PI_PRINTF(TPI,"mode_need_switch is not yes!\n");
	}
	return RET_SUC;
}

RET_INFO tpi_wan_mode_check_struct_init()
{
	memset(&wan_mode_check_info,0x0,sizeof(wan_mode_check_info));
	return tpi_wan_mode_check_update_info();
}

RET_INFO tpi_wan_mode_check_first_init()
{
	if(wan_mode_check_info.enable)
		tpi_wan_mode_check_start();
	
	return RET_SUC;
}
/*添加线程退出接口函数lq*/
static int check_wan_mode_exit_flag = 0;
inline void set_wan_mode_exit_flag(int flag)
{
	check_wan_mode_exit_flag = flag;
}

inline int get_wan_mode_exit_flag()
{
	return check_wan_mode_exit_flag;
}

RET_INFO tpi_wan_mode_check_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_wan_mode_check_start();
			break;
		case OP_STOP:
			tpi_wan_mode_check_stop();
			break;
		case OP_RESTART:
			tpi_wan_mode_check_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/
WANMODE tpi_wan_mode_check_get_check_result()
{
	return wan_mode_check_info.check_result;
}

CHECK_RESULT tpi_wan_mode_check_get_check_finish_tag()
{
	return wan_mode_check_info.check_finish_tag;
}

/*以下为该模块具体执行实现函数*/
PIU8 tpi_wan_mode_check_get_exit_tag()
{	
	return wan_mode_check_info.exit_tag;
}

RET_INFO tpi_wan_mode_check_set_exit_tag(PIU8 enable)
{
	if(1 == enable)
	{
		wan_mode_check_info.exit_tag = 1;
	}
	else
	{
		wan_mode_check_info.exit_tag = 0;		
	}
	return RET_SUC;
}

RET_INFO tpi_wan_mode_check_wait_exit()
{
	while(1)
	{	
		if(0 == tpi_wan_mode_check_get_exit_tag())
			break;
		cyg_thread_delay(RC_MODULE_1S_TIME/10);		
	}
	return RET_SUC;
}

/*在wan_mode_check等其他线程可能会用到*/
void tpi_wan_mode_check_set_dhcp_discover_add()
{
	if(wan_mode_daemon_handle != 0)
		wan_mode_check_info.discover_send_num++;
	return;
}

void tpi_wan_mode_check_set_dhcp_offer_rcv()
{
	if(wan_mode_daemon_handle != 0)
		wan_mode_check_info.offer_rcv_tag = 1;
	return;
}

void tpi_wan_mode_check_set_pppoe_padi_add()
{
	if(wan_mode_daemon_handle != 0)
		wan_mode_check_info.padi_send_num++;
	return;
}

void tpi_wan_mode_check_set_pppoe_pado_rcv()
{
	if(wan_mode_daemon_handle != 0)
		wan_mode_check_info.pado_rcv_tag= 1;
	return;
}

/*只有本文件里面用*/
static RET_INFO tpi_wan_mode_check_pppoe_dhcp()
{
	PIU8 ppp2_start_tag = 0,ppp2_stop_tag = 1;
	PIU8 dhcp_check_ok = 0;

	while(1)
	{
		/*检测步骤如下:
		*拉起PPP2发PADI报文，检测是否在3个报文内有回应，此时DHCP依旧照常发包
		* 如果在3个报文内有收到回包，则等待PPPOE检测结果
    	* 如果PPPOE有回应，则认为是DHCP
    	* 如果PPPOE没有回应，则认为是PPPOE
		* 如果DHCP和PPPOE都没有回应，则认为是STATIC
		*/
		//判断线程推出标志是否为请求退出lq
		if(1 == get_wan_mode_exit_flag())
		{
			break;
		}
		
		cyg_thread_delay(RC_MODULE_1S_TIME);

		if(1 == wan_mode_check_info.exit_tag)
		{
			goto finish;
		}

		if(0 == ppp2_start_tag)
		{
			if(1 == tenda_wan_link_status())
			{
#ifdef __CONFIG_PPPOE__  			//added by yp ,for A9,2016-3-17
				wan_pppoe_start2();
#endif
				ppp2_start_tag = 1;
			}
		}
		
		if(strcmp(nvram_safe_get("wan0_connect"),"Connected") == 0 ||
			(wan_mode_check_info.offer_rcv_tag == 1 && wan_mode_check_info.discover_send_num <= DHCP_CHECK_MAX_NUM))
		{
			//在检测PPPOE三个包的过程中已经收到offer报文的时候，如果没有PPPOE的情况下则就是DHCP
			dhcp_check_ok = 1;
		}

		if(wan_mode_check_info.padi_send_num <= PPPOE_CHECK_MAX_NUM)
		{
			if(1 == wan_mode_check_info.pado_rcv_tag)		
			{
				PI_PRINTF(TPI,"check_result : PPPOE!\n");
				wan_mode_check_info.check_result = WAN_PPPOE_MODE;
				break;
			}
			else
			{
				continue;
			}
		}

		/*这里要提前关闭ppp2，否则的话可能会造成在DHCP发包过程中多发一个PADI报文*/
		if(1 == ppp2_stop_tag && ppp2_start_tag == 1)
		{
#ifdef  __CONFIG_PPPOE__  					//added by yp, 2016-3-17
			wan_pppoe_down2("ppp2");
#endif
			ppp2_stop_tag = 0;
		}

		//如果不是PPPOE接入且接收到offer报文并且发送dicover报文个数少于3个，则是dhcp接入
		if(1 == dhcp_check_ok)
		{
			PI_PRINTF(TPI,"check_result : DHCP!\n");
			wan_mode_check_info.check_result = WAN_DHCP_MODE;
			break;
		}

		if(wan_mode_check_info.offer_rcv_tag != 1 && wan_mode_check_info.discover_send_num <= DHCP_CHECK_MAX_NUM)
		{
			continue;
		}
		else
		{
			PI_PRINTF(TPI,"check_result : STATIC IP!\n");
			wan_mode_check_info.check_result = WAN_STATIC_MODE;
			break;
		}
	}
		
finish:
	nvram_set("mode_need_switch","no");
	nvram_commit();
	
	if(1 == wan_mode_check_info.exit_tag)
	{
		wan_mode_check_info.exit_tag = 0;
	}

	if(1 == ppp2_stop_tag && ppp2_start_tag == 1)
	{
#ifdef  __CONFIG_PPPOE__  					//added by yp,for A9, 2016-3-17
		wan_pppoe_down2("ppp2");
#endif
	}
	
	wan_mode_check_info.check_finish_tag = CHECK_END;
	
	return RET_SUC;
}

static RET_INFO tpi_wan_mode_check_main()
{
	if(WAN_DHCP_MODE == wan_mode_check_info.wan_mode)
	{
		switch(wan_mode_check_info.type)
		{
			case CHECK_DHCP_PING_PPPOE:
				PI_ERROR(TPI,"CHECK_DHCP_PING_PPPOE not used here!\n");
				break;
			case CHECK_DHCP_PPPOE:
				PI_ERROR(TPI,"CHECK_DHCP_PPPOE not used here!\n");
				break;
			case CHECK_PPPOE_DHCP:
				tpi_wan_mode_check_pppoe_dhcp();
				break;
			default:
				PI_ERROR(TPI,"unknow type[%d]!\n",wan_mode_check_info.type);
				break;
		}
	}
	else
	{
		PI_ERROR(TPI,"wan type is not dhcp,exit!\n");
	}


	return RET_SUC;
}

static RET_INFO tpi_wan_mode_check_start()
{
	RET_INFO ret = RET_SUC;

	if(wan_mode_daemon_handle == 0)
	{
		tpi_wan_mode_check_update_info();
		if(wan_mode_check_info.enable)
		{
			cyg_thread_create(
				8, 
				(cyg_thread_entry_t *)tpi_wan_mode_check_main,
				0, 
				"wan_mode_check",
				wan_mode_daemon_stack, 
				sizeof(wan_mode_daemon_stack), 
				&wan_mode_daemon_handle, 
				&wan_mode_daemon_thread);
			
			cyg_thread_resume(wan_mode_daemon_handle);     
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

static RET_INFO tpi_wan_mode_check_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if(wan_mode_daemon_handle != 0)
	{
		tpi_wan_mode_check_set_exit_tag(1);
		tpi_wan_mode_check_wait_exit();
		
		//设置退出标志为退出lq
		set_wan_mode_exit_flag(1);
		/* Wait until thread exit */
		pid = oslib_getpidbyname("wan_mode_check");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
		
		cyg_thread_delete(wan_mode_daemon_handle);
		
		PI_PRINTF(TPI,"stop success!\n");
		set_wan_mode_exit_flag(0);
		wan_mode_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");		
	}
	
	return ret;
}

static RET_INFO tpi_wan_mode_check_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_wan_mode_check_stop() || RET_ERR == tpi_wan_mode_check_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}
