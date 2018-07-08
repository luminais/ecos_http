#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include "sys_timer.h"
#include "debug.h"
#include "login_keep.h"

void login_keep_start();
void login_keep_stop();
static RET_INFO tpi_login_keep_start();
static RET_INFO tpi_login_keep_stop();
static RET_INFO tpi_login_keep_restart();

RET_INFO tpi_login_keep_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_login_keep_start();
            break;
        case OP_STOP:
            tpi_login_keep_stop();
            break;
        case OP_RESTART:
            tpi_login_keep_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}

static RET_INFO tpi_login_keep_start()
{
	login_keep_start();
    return RET_SUC;
}

static RET_INFO tpi_login_keep_stop()
{
	login_keep_stop();
    PI_PRINTF(TPI,"stop success!\n");
    return RET_SUC;
}

static RET_INFO tpi_login_keep_restart()
{
    RET_INFO ret = RET_SUC;

    if(RET_ERR == tpi_login_keep_stop() || RET_ERR == tpi_login_keep_start())
    {
        PI_ERROR(TPI,"restart error!\n");
    }

    return ret;
}

RET_INFO tpi_login_keep_first_init()
{
	return tpi_login_keep_start();
}

