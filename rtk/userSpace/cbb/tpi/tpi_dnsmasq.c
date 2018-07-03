#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "debug.h"
#include "bcmdnsmasq.h"

static RET_INFO tpi_dnsmasq_start();
static RET_INFO tpi_dnsmasq_stop();
static RET_INFO tpi_dnsmasq_restart();

/*以下函数用于api调用*/
RET_INFO tpi_dnsmasq_update_info()
{
	return RET_SUC;
}

RET_INFO tpi_dnsmasq_struct_init()
{
	

	extern int dns_redirect_disable;
	//add by z10312 快速设置前 开启dns 重定向
	if (!strcmp (nvram_safe_get("restore_quick_set"), "1") )
	{
		dns_redirect_disable = 0;
	}
	
	
	return RET_SUC;
}

RET_INFO tpi_dnsmasq_first_init()
{
	return tpi_dnsmasq_start();
}

RET_INFO tpi_dnsmasq_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_dnsmasq_start();
			break;
		case OP_STOP:
			tpi_dnsmasq_stop();
			break;
		case OP_RESTART:
			tpi_dnsmasq_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/

/*以下为该模块具体执行实现函数*/

/*在dnsmasq等其他线程可能会用到*/

/*只有本文件里面用*/
static RET_INFO tpi_dnsmasq_start()
{
	tpi_dnsmasq_update_info();


	dnsmasq_start();
	
	PI_PRINTF(TPI,"start success!\n");
	
	return RET_SUC;
}

static RET_INFO tpi_dnsmasq_stop()
{
	tpi_dnsmasq_update_info();
	
	dnsmasq_stop();
	
	PI_PRINTF(TPI,"stop success!\n");	
	return RET_SUC;
}

static RET_INFO tpi_dnsmasq_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_dnsmasq_stop() || RET_ERR == tpi_dnsmasq_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}
