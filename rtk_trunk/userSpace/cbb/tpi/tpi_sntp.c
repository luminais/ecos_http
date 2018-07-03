#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "debug.h"
#include "bcmsntp.h"

static SNTP_INFO_STRUCT sntp_info;

static RET_INFO tpi_sntp_start();
static RET_INFO tpi_sntp_stop();
static RET_INFO tpi_sntp_restart();

/*以下函数用于api调用*/
RET_INFO tpi_sntp_update_info()
{
	// sntp is not supported by A9, modified by zhuhuan on 2016.05.03
#ifdef __CONFIG_A9__
	if((0 != strcmp(nvram_safe_get("wl0_mode"), "wet")) && (0 == strcmp("0",nvram_safe_get("time_mode"))))
#else
    if(0 == strcmp("0",nvram_safe_get("time_mode")))                     //关闭手动输入
#endif
        sntp_info.enable = 1;
    else
        sntp_info.enable = 0;

    sntp_info.time_zone = nvram_safe_get("time_zone");

    return RET_SUC;
}

RET_INFO tpi_sntp_struct_init()
{
    memset(&sntp_info,0x0,sizeof(sntp_info));
    return tpi_sntp_update_info();
}

RET_INFO tpi_sntp_first_init()
{
    return tpi_sntp_start();
}

RET_INFO tpi_sntp_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_sntp_start();
            break;
        case OP_STOP:
            tpi_sntp_stop();
            break;
        case OP_RESTART:
            tpi_sntp_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}

/*以下用于gpi获取信息函数*/
P_SNTP_INFO_STRUCT tpi_sntp_get_info()
{
	tpi_sntp_update_info();
    return &sntp_info;
}

/*本文件使用*/
static RET_INFO tpi_sntp_start()
{
    tpi_sntp_update_info();
    if(1 == sntp_info.enable)
    {
        start_ntpc();
        PI_PRINTF(TPI,"start success!\n");
    }
    else
    {
        PI_ERROR(TPI,"the mib is off, connot start!\n");
    }

    return RET_SUC;
}

static RET_INFO tpi_sntp_stop()
{
    tpi_sntp_update_info();
    stop_ntpc();
    PI_PRINTF(TPI,"stop success!\n");
    return RET_SUC;
}

static RET_INFO tpi_sntp_restart()
{
    RET_INFO ret = RET_SUC;

    if(RET_ERR == tpi_sntp_stop() || RET_ERR == tpi_sntp_start())
    {
        PI_ERROR(TPI,"restart error!\n");
    }

    return ret;
}
