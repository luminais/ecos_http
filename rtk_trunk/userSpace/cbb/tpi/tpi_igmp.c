#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "debug.h"
#include "wifi.h"
extern void create_mrouted(char * wan_if, char * lan_if);
extern void clean_mrouted( );

RET_INFO tpi_igmp_start()
{
    char wan_if[32] = {0};
    char lan_if[32] = {0};

	if(!nvram_match(SYSCONFIG_WORKMODE,"route")) //ap || wisp || client+ap
	{
		diag_printf("[%s][%d][luminais] not support such wireless mode.\n", __FUNCTION__, __LINE__);
		return RET_SUC;
	}

	if(nvram_match(WAN0_PROTO,"pppoe"))
	{
		diag_printf("[%s][%d][luminais] not support pppoe.\n", __FUNCTION__, __LINE__);
		return RET_SUC;
	}
	
	if(!nvram_match(ADVANCE_IPTV_ENABLE, "1"))
	{
		diag_printf("[%s][%d][luminais] igmp_enable != 1\n", __FUNCTION__, __LINE__);
		return RET_SUC;
	}
    
    	strcpy(wan_if, nvram_safe_get(WAN0_IFNAME));
    	strcpy(lan_if, nvram_safe_get(LAN_IFNAME));
		
	if (iflib_getifaddr(wan_if, 0, 0) != 0)
		return RET_SUC;
	
	create_mrouted(wan_if, lan_if);
	PI_PRINTF(TPI,"igmp start success!\n");
	return RET_SUC;
}

RET_INFO tpi_igmp_stop()
{
	int pid;

	pid = oslib_getpidbyname("mrouted thread");
	if(0 != pid)
	{
		clean_mrouted();
		/* Wait until thread exit */
		while (oslib_waitpid(pid, NULL) != 0)
			cyg_thread_delay(1);
	}
	PI_PRINTF(TPI,"igmp_stop success!\n");	
	return RET_SUC;
}

RET_INFO tpi_igmp_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_igmp_stop() || RET_ERR == tpi_igmp_start())
	{
		PI_ERROR(TPI,"restart error!\n");
		ret = RET_ERR;
	}
	
	return ret;
}

RET_INFO tpi_igmp_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_igmp_start();
			break;
		case OP_STOP:
			tpi_igmp_stop();
			break;
		case OP_RESTART:
			tpi_igmp_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

RET_INFO tpi_wan_igmp_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	if(NULL == var)
	{
		PI_ERROR(TPI,"upnp var is null!\n");
		return RET_ERR;
	}

	switch(var->op)
	{
		case OP_START:
		case OP_RESTART:
			tpi_igmp_restart();
			break;
		case OP_STOP:
			tpi_igmp_stop();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n");
			ret = RET_ERR;
			break;			
	}	
	return ret;
}


RET_INFO tpi_igmp_init()
{
	diag_printf("[%s][%d][luminais] nothing to do.\n", __FUNCTION__, __LINE__);
	return RET_SUC;
}

