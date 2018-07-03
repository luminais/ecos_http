#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "debug.h"
#include "bcmddns.h"

static DDNS_INFO_STRUCT ddns_info;

static RET_INFO tpi_ddns_start();
static RET_INFO tpi_ddns_stop();
static RET_INFO tpi_ddns_restart();

RET_INFO tpi_ddns_update_info()
{

    if(strcmp("0",nvram_safe_get("ddns_enable")))                     
        ddns_info.enable = 1;
    else
        ddns_info.enable = 0;

    ddns_info.isp= atoi(nvram_safe_get("ddns_isp"));
    ddns_info.ddns_set = nvram_safe_get("ddns_set1");
    ddns_info.status = nvram_safe_get("ddns_status");
    return RET_SUC;
}

RET_INFO tpi_ddns_struct_init()
{
    memset(&ddns_info,0x0,sizeof(ddns_info));
    return tpi_ddns_update_info();
}

RET_INFO tpi_ddns_first_init()
{
    return tpi_ddns_start();
}

RET_INFO tpi_wan_ddns_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	if(NULL == var)
	{
		PI_ERROR(TPI,"ddns var is null!\n");
		return RET_ERR;
	}

	switch(var->op)
	{
		case OP_START:
		case OP_RESTART:
			tpi_ddns_restart();
			break;
		case OP_STOP:
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n");
			ret = RET_ERR;
			break;			
	}	
	return ret;
}

RET_INFO tpi_ddns_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_ddns_start();
            break;
        case OP_STOP:
            tpi_ddns_stop();
            break;
        case OP_RESTART:
            tpi_ddns_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}

/*以下用于gpi获取信息函数*/
P_DDNS_INFO_STRUCT tpi_ddns_get_info()
{
	tpi_ddns_update_info();
    return &ddns_info;
}


static RET_INFO tpi_ddns_start()
{
    tpi_ddns_update_info();
    if(1 == ddns_info.enable)
    {
        ddns_start();
        PI_PRINTF(TPI,"start success!\n");
    }
    else
    {
        PI_ERROR(TPI,"the mib is off, connot start!\n");
    }

    return RET_SUC;
}

static RET_INFO tpi_ddns_stop()
{
    tpi_ddns_update_info();
    ddns_stop();
    PI_PRINTF(TPI,"stop success!\n");
    return RET_SUC;
}

static RET_INFO tpi_ddns_restart()
{
    RET_INFO ret = RET_SUC;

    if(RET_ERR == tpi_ddns_stop() || RET_ERR == tpi_ddns_start())
    {
        PI_ERROR(TPI,"restart error!\n");
    }

    return ret;
}

