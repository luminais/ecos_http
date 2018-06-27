#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include <net/if.h>
#include "wlioctl.h"
#include <wlutils.h>

#include "http.h"
#include "wps.h"
#include "sys_module.h"

#define WPS_SIZE		24*1024
static cyg_handle_t wps_daemon_handle;
static PI8 wps_daemon_stack[WPS_SIZE];
static cyg_thread wps_daemon_thread;

static WPS_INFO_STRUCT wps_info;

static RET_INFO tpi_wps_handle(MODULE_COMMON_OP_TYPE action);
static RET_INFO tpi_wps_main();
static RET_INFO tpi_wps_start();
static RET_INFO tpi_wps_stop();
static RET_INFO tpi_wps_restart();
static RET_INFO tpi_wps_iw_start();
static RET_INFO tpi_create_wps_thread();


extern cyg_flag_t wsc_flag;
extern cyg_mbox wscd_mbox;
extern cyg_handle_t wscd0_mbox_hdl;
extern void iw_init();
#ifdef __CONFIG_AUTO_CONN_CLIENT__
enum AUTO_CONN_EXTEND_STATUS
{
	AUTO_CONN_VIF_EXTEND_UNDO,
	AUTO_CONN_VIF_EXTEND_SETDEF,
	AUTO_CONN_VIF_EXTEND_DOING,
	AUTO_CONN_VIF_EXTEND_DONE
};
extern int extend_get_status();
#endif
/*以下函数用于api调用*/
RET_INFO tpi_wps_update_info()
{
	PI8 mib_value[16] = {0};
	strcpy(mib_value, nvram_safe_get(WPS_MODE_ENABLE));

	if(0 == strcmp(mib_value, "enabled"))
		wps_info.enable = 1;
	else
		wps_info.enable = 0;
	
	if(1 == wps_info.enable)
	{
		if(nvram_match(SYSCONFIG_WORKMODE,"route") && nvram_match(WLAN24G_ENABLE,"0") && nvram_match(WLAN5G_ENABLE,"0"))
		{
			printf("wps enabled but wl not enable!\n");
			wps_info.enable = 0;
		}
		if((!is_interface_up(TENDA_WLAN5_AP_IFNAME)) && (!is_interface_up(TENDA_WLAN24_AP_IFNAME)))
		{
			printf("wps enabled but wlan0/wlan1 not up!\n");
			wps_info.enable = 0;
		}
	#ifndef __CONFIG_A9__
		if(!nvram_match(SYSCONFIG_WORKMODE,"route"))
		{
			printf("wps enabled but wl is in repeater mode!\n");
			wps_info.enable = 0;
		}
	#endif
	}
	
	return RET_SUC;
}

RET_INFO tpi_wps_struct_init()
{
	memset(&wps_info,0x0,sizeof(wps_info));

	/*Create the message box*/
	cyg_mbox_create(&wscd0_mbox_hdl,&wscd_mbox);
	
	/*struct init*/
	iw_init();
	
	return tpi_wps_update_info();
}

extern void start_wps();
RET_INFO tpi_wps_first_init()
{
	if(wps_info.enable)
	{
		tpi_create_wps_thread();
		tpi_wps_iw_start();
	}
		//start_wps();
	
	return RET_SUC;
}

static RET_INFO tpi_wps_start()
{
	cyg_thread_info tinfo;
	tpi_wps_update_info();
	if(wps_info.enable)
	{
		if(get_thread_info_by_name("wps", &tinfo))
		{
			wsc_reinit();
		}
		else
		{
			tpi_create_wps_thread();
			tpi_wps_iw_start();
		}
		PI_PRINTF(TPI,"start success!\n");
	}
	return RET_SUC;
}
static RET_INFO tpi_wps_stop()
{
	if((get_interface_state(TENDA_WLAN24_AP_IFNAME) 
		&& get_interface_state(TENDA_WLAN5_AP_IFNAME)) || nvram_match("wps_mode", "disabled"))
	{
		wsc_stop_service();
	}
	PI_PRINTF(TPI,"stop success!\n");
	return RET_SUC;
}
static RET_INFO tpi_wps_restart()
{
	RET_INFO ret = RET_SUC;
	tpi_wps_update_info();
	if(wps_info.enable)
	{
		if(RET_ERR == tpi_wps_start())
		{
			ret = RET_ERR;
		}
	}
	else
	{
		if(RET_ERR == tpi_wps_stop())
		{
			ret = RET_ERR;
		}
	}
	
	
	return ret;
}




RET_INFO tpi_wps_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI8	msg_param[PI_BUFLEN_256] = {0};

	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_wps_start();
			break;
		case OP_STOP:
			tpi_wps_stop();
			break;
		case OP_RESTART:
			tpi_wps_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	PI_PRINTF(TPI,"var->string_info=%s\n",var->string_info);
	
	if(NULL == var->string_info || 0 == strcmp("",var->string_info))
	{
		return RET_ERR;
	}
	
	if(0 == memcmp("startpbc",var->string_info,strlen("startpbc")))
	{
	#ifdef __CONFIG_EXTEND_LED__
		if(extend_led_blink())//该状态表示WPS或自动桥接正在进行中
		{
	#ifdef __CONFIG_AUTO_CONN_CLIENT__
			if(AUTO_CONN_VIF_EXTEND_DOING == extend_get_status() || AUTO_CONN_VIF_EXTEND_SETDEF == extend_get_status())
			{
				printf("%s:the extender is in auto extending status!\n",__func__);
			}
			else
	#endif
			{
				printf("%s:the extender is in wps extending status!\n",__func__);
			}
		}
		else
	#endif
		{
			cyg_thread_info tinfo;
			if(!get_thread_info_by_name("wps", &tinfo))
			{
				printf("the wps thread not created we need create first!\n");
				tpi_wps_start();
				cyg_thread_delay(200);
			}
			cyg_flag_setbits(&wsc_flag, 8);
		}
	}
	else if(0 == memcmp("btn_startpbc",var->string_info,strlen("btn_startpbc")))
	{
		//未开启WPS功能的时候按wps按钮需要先开启wps
		tpi_wps_start();
		cyg_thread_delay(200);
		cyg_flag_setbits(&wsc_flag, 8);
	}
	else if(0 == memcmp("starpin",var->string_info,strlen("starpin")))
	{
		;//need fix
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/
PI8 tpi_wps_get_enable()
{
	return wps_info.enable;
}


/*以下为该模块具体执行实现函数*/



/*只有本文件里面用*/
//extern void auto_conn_extend_main();
static RET_INFO tpi_wps_main()
{
	auto_conn_extend_main();
	
	return RET_SUC;
}

extern void wscd_main(cyg_addrword_t data);
static RET_INFO tpi_create_wps_thread()
{
	RET_INFO ret = RET_SUC;

	if(wps_daemon_handle == 0)
	{
		tpi_wps_update_info();
		if(wps_info.enable)
		{
			cyg_thread_create(
				8, 
				(cyg_thread_entry_t *)wscd_main,
				0, 
				"wps",
				wps_daemon_stack, 
				sizeof(wps_daemon_stack), 
				&wps_daemon_handle, 
				&wps_daemon_thread);
			
			cyg_thread_resume(wps_daemon_handle);     
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


/*add for iw*/
#define WPS_IW_SIZE 8*1024
static cyg_handle_t wps_iw_daemon_handle;
static PI8 wps_iw_daemon_stack[WPS_SIZE];
static cyg_thread wps_iw_daemon_thread;
extern int iw_main(cyg_addrword_t data);

static RET_INFO tpi_wps_iw_start()
{
	RET_INFO ret = RET_SUC;

	if(wps_iw_daemon_handle == 0)
	{
		tpi_wps_update_info();
		if(wps_info.enable)
		{
			cyg_thread_create(
				16, 
				(cyg_thread_entry_t *)iw_main,
				0, 
				"iw",
				wps_iw_daemon_stack, 
				sizeof(wps_iw_daemon_stack), 
				&wps_iw_daemon_handle, 
				&wps_iw_daemon_thread);
			
			cyg_thread_resume(wps_iw_daemon_handle);     
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

