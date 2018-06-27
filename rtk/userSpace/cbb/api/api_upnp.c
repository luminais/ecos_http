#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bcmupnp.h"

static RET_INFO api_upnp_handle(RC_MODULES_COMMON_STRUCT * var);
static RET_INFO api_upnp_init();

RET_INFO api_wan_upnp_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_wan_upnp_action(var);
}

static RET_INFO api_upnp_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_upnp_action(var);
}

static struct rc_msg_ops rc_upnp_ops[] =
{
    {
        .intent_module_id = RC_UPNP_MODULE,
		.type = INTENT_NONE,
        .ops = api_upnp_handle,
    },
};

static RET_INFO api_upnp_init()
{
	tpi_upnp_struct_init();
	tpi_upnp_first_init();
    rc_register_module_msg_opses(rc_upnp_ops,RC_UPNP_MODULE,ARRAY_SIZE(rc_upnp_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_UPNP_MODULE,"rc_upnp",api_upnp_init,NULL);
