#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wifi_switch_sched.h"

static RET_INFO api_wifi_switch_sched_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_wifi_switch_sched_init();

static RET_INFO api_wifi_switch_sched_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_wifi_switch_sched_action(var);
}

static struct rc_msg_ops rc_wifi_switch_sched_ops[] =
{
    {
        .intent_module_id = RC_WIFI_SWITCH_SCHED_MODULE,
		.type = INTENT_NONE,
        .ops = api_wifi_switch_sched_handle,
    },
};

static RET_INFO api_wifi_switch_sched_init()
{
	tpi_wifi_switch_sched_struct_init();
	tpi_wifi_switch_sched_first_init();
    rc_register_module_msg_opses(rc_wifi_switch_sched_ops,RC_WIFI_SWITCH_SCHED_MODULE,ARRAY_SIZE(rc_wifi_switch_sched_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_WIFI_SWITCH_SCHED_MODULE,"rc_wifi_switch_sched",api_wifi_switch_sched_init,NULL);
