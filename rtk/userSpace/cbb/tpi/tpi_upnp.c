
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "bcmupnp.h"

static RET_INFO tpi_upnp_start();
static RET_INFO tpi_upnp_stop();
static RET_INFO tpi_upnp_restart();

static UPNP_INFO_STRUCT upnp_info;

/*以下函数用于api调用*/
RET_INFO tpi_upnp_update_info()
{
	upnp_info.enable = atoi(nvram_safe_get("upnp_enable"));
	return RET_SUC;
}

RET_INFO tpi_upnp_struct_init()
{
	memset(&upnp_info,0x0,sizeof(upnp_info));
	return tpi_upnp_update_info();
}

RET_INFO tpi_upnp_first_init()
{
	if(upnp_info.enable)
		tpi_upnp_start();
	
	return RET_SUC;
}

RET_INFO tpi_wan_upnp_action(RC_MODULES_COMMON_STRUCT *var)
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
			tpi_upnp_restart();
			break;
		case OP_STOP:
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n");
			ret = RET_ERR;
			break;			
	}	
	return ret;
}

RET_INFO tpi_upnp_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	if(NULL == var)
	{
		PI_ERROR(TPI,"upnp var is null!\n");
		return RET_ERR;
	}

	if(RET_ERR == tpi_upnp_update_info())
	{
		PI_PRINTF(TPI,"update fail!\n");
		return RET_ERR;
	}

	switch(var->op)
	{
		case OP_START:
			tpi_upnp_start();
			break;
		case OP_STOP:
			tpi_upnp_stop();
			break;
		case OP_RESTART:
			tpi_upnp_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n");
			ret = RET_ERR;
			break;			
	}	
	return ret;
}

/*called by gpi*/
P_UPNP_INFO_STRUCT tpi_upnp_get_info()
{
	tpi_upnp_update_info();
	return &upnp_info;
}

/*only called by this file */
static RET_INFO tpi_upnp_start()
{
	igd_start();
	PI_PRINTF(TPI,"start success!\n");
	return RET_SUC;
}

static RET_INFO tpi_upnp_stop()
{
	igd_stop();
	PI_PRINTF(TPI,"stop success!\n");
	return RET_SUC;
}

static RET_INFO tpi_upnp_restart()
{
	if(RET_ERR == tpi_upnp_stop() || RET_ERR == tpi_upnp_start())
	{
		PI_ERROR(TPI,"restart fail!\n");
	}
	else 
	{
		PI_PRINTF(TPI,"restart success!\n");
	}

	return RET_SUC;
}
