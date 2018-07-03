/*
*
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"

static RET_INFO api_debug_msg_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_debug_init();

static RET_INFO api_debug_msg_handle(RC_MODULES_COMMON_STRUCT *var)
{
    RET_INFO ret = RET_SUC;

	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}

    if(RC_DBG_MODULE == var->debug_id)
    {
        ret = tpi_debug_show_info();
    }
    else
    {
        ret = tpi_debug_action(var->debug_id);
    }
    return ret;
}

static struct rc_msg_ops rc_debug_ops[] =
{
    {
        .intent_module_id = RC_DBG_MODULE,
		.type = INTENT_NONE,
        .ops = api_debug_msg_handle,
    },
};

static RET_INFO api_debug_init()
{
    memset(&_RC_DEBUG_TAG_,0x0,sizeof(_RC_DEBUG_TAG_));
    rc_register_module_msg_opses(rc_debug_ops,RC_DBG_MODULE,ARRAY_SIZE(rc_debug_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_DBG_MODULE,"rc_debug",api_debug_init,NULL);
