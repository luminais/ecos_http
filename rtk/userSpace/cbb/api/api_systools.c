#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "systools.h"
#ifdef __CONFIG_NAT__
#include "wan.h"
#endif

static RET_INFO api_systools_msg_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_systools_init();

static RET_INFO api_systools_msg_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}
	
	if(RET_SUC != tpi_systools_action(var))
	{
		PI_ERROR(API,"systools action fail!\n");
		return RET_ERR;		
	}
	
	PI_PRINTF(API,"action end!\n");
    return RET_SUC;
}

/*wan module register infomation*/
struct rc_msg_ops rc_sysytools_ops[] =
{
    {
        .intent_module_id = RC_SYSTOOLS_MODULE,
		.type = INTENT_NONE,
        .ops = api_systools_msg_handle,
    },
#ifdef __CONFIG_PPPOE__
	{
        .intent_module_id = RC_WAN_MODULE,
		.type = INTENT_PREV,
        .ops = api_wan_systools_handle,
    },
#endif
};

static RET_INFO api_systools_init()
{
    rc_register_module_msg_opses(rc_sysytools_ops,RC_SYSTOOLS_MODULE,ARRAY_SIZE(rc_sysytools_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_SYSTOOLS_MODULE,"rc_systools",api_systools_init,NULL);
