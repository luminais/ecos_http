#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
//#include "cbb_igmp.h"

static RET_INFO api_igmp_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_igmp_init();

static RET_INFO api_igmp_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_igmp_action(var);
}

RET_INFO api_wan_igmp_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}

	return tpi_wan_igmp_action(var);
}

static struct rc_msg_ops rc_igmp_ops[] =
{
	{
		.intent_module_id = RC_IGMP_MODULE,
		.type = INTENT_NONE,
		.ops = api_igmp_handle,
	},
};

static RET_INFO api_igmp_init()
{
	tpi_igmp_init();
	rc_register_module_msg_opses(rc_igmp_ops,RC_IGMP_MODULE,ARRAY_SIZE(rc_igmp_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_IGMP_MODULE,"rc_igmp",api_igmp_init,NULL);

