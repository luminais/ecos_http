#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "debug.h"
#include "systools.h"

extern void do_reset(int );
extern void nvram_default(void);

/*本文件里面使用*/
static RET_INFO tpi_reboot_action()
{
	cyg_thread_delay(2*RC_MODULE_1S_TIME);
	do_reset(1);
	return RET_SUC;
}

static RET_INFO tpi_restore_action()
{
	cyg_thread_delay(2*RC_MODULE_1S_TIME);
	nvram_default();
	do_reset(1);
	return RET_SUC;
}

RET_INFO tpi_systools_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;

	//根据coverity分析结果修改，原来为无效的判断:NULL == var->string_info  2017/1/11 F9项目修改
	if(NULL == var || 0 == strcmp("",var->string_info))
	{
		return RET_ERR;
	}
	
	if(0 == memcmp("reboot",var->string_info,strlen("reboot")))
	{
		ret = tpi_reboot_action();
	}
	else if(0 == memcmp("restore",var->string_info,strlen("restore")))
	{
		ret = tpi_restore_action();
	}
	else if(0 == memcmp("commit",var->string_info,strlen("commit")))
	{
	/*添加commit操作，对于一些需要及时返回的操作，可以先发commit消息,
	然后立即返回，前提条件，发送消息的线程优先级需要大于等于main线程的优先级*/
		nvram_commit();
	}

	return ret;
}

