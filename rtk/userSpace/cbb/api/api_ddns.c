#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bcmddns.h"

static RET_INFO api_ddns_handle(RC_MODULES_COMMON_STRUCT * var);
static RET_INFO api_ddns_init();

RET_INFO api_wan_ddns_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_wan_ddns_action(var);
}

static RET_INFO api_ddns_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_ddns_action(var);
}

static struct rc_msg_ops rc_ddns_ops[] =
{
    {
        .intent_module_id = RC_DDNS_MODULE,
		.type = INTENT_NONE,
        .ops = api_ddns_handle,
    },
};

static RET_INFO api_ddns_init()
{
	tpi_ddns_struct_init();
	tpi_ddns_first_init();
    rc_register_module_msg_opses(rc_ddns_ops,RC_DDNS_MODULE,ARRAY_SIZE(rc_ddns_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_DDNS_MODULE,"rc_ddns",api_ddns_init,NULL);


