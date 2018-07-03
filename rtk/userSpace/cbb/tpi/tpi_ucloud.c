#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "debug.h"
#include "bcmucloud.h"

static UCLOUD_INFO_STRUCT ucloud_info;

static RET_INFO tpi_ucloud_start();
static RET_INFO tpi_ucloud_stop();
static RET_INFO tpi_ucloud_restart();
/*
tpi_ucloud_update_info:
获取ucloud线程所需要的nvram 参数，如果还有
其他参数请给出，并添加到UCLOUD_INFO_STRUCT 结构中
*/
RET_INFO tpi_ucloud_update_info()
{

    if(strcmp("0",nvram_safe_get("ucloud_enable")))                     
        ucloud_info.enable = 1;
    else
        ucloud_info.enable = 0;
    return RET_SUC;
}

RET_INFO tpi_ucloud_struct_init()
{
    memset(&ucloud_info,0x0,sizeof(ucloud_info));
    return tpi_ucloud_update_info();
}

RET_INFO tpi_ucloud_first_init()
{
    return tpi_ucloud_start();
}

RET_INFO tpi_wan_ucloud_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	
	if(NULL == var)
	{
		PI_ERROR(TPI,"ucloud var is null!\n");
		return RET_ERR;
	}

	switch(var->op)
	{
		case OP_START:
		case OP_RESTART:
			tpi_ucloud_restart();
			break;
		case OP_STOP:
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n");
			ret = RET_ERR;
			break;			
	}	
	return ret;
}

RET_INFO tpi_ucloud_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_ucloud_start();
            break;
        case OP_STOP:
            tpi_ucloud_stop();
            break;
        case OP_RESTART:
            tpi_ucloud_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}

/*以下用于gpi获取信息函数*/
P_UCLOUD_INFO_STRUCT tpi_ucloud_get_info()
{
	tpi_ucloud_update_info();
    return &ucloud_info;
}


static RET_INFO tpi_ucloud_start()
{
    tpi_ucloud_update_info();
    if(1 == ucloud_info.enable)
    {
        biz_m_ucloud_proc_start();
        PI_PRINTF(TPI,"start success!\n");
    }
    else
    {
        PI_ERROR(TPI,"the mib is off, connot start!\n");
    }

    return RET_SUC;
}

static RET_INFO tpi_ucloud_stop()
{
    tpi_ucloud_update_info();
/***************请给出*******************/
 //   ucloud_stop();
    PI_PRINTF(TPI,"stop success!\n");
    return RET_SUC;
}

static RET_INFO tpi_ucloud_restart()
{
    RET_INFO ret = RET_SUC;
/***************请给出*******************/
#if 0
    if(RET_ERR == tpi_ucloud_stop() || RET_ERR == tpi_ucloud_start())
    {
        PI_ERROR(TPI,"restart error!\n");
    }
#endif
    if(RET_ERR == tpi_ucloud_start())
    {
        PI_ERROR(TPI,"restart error!\n");
    }
    return ret;
}

