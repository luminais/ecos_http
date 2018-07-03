/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wan_mode_check.h"

static RET_INFO api_wan_mode_check_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_wan_mode_check_action(var);
}

static struct rc_msg_ops rc_wan_mode_check_ops[] =
{
    {
        .intent_module_id = RC_WAN_MODE_CHECK_MODULE,
		.type = INTENT_NONE,
        .ops = api_wan_mode_check_handle,
    },
};

static RET_INFO api_wan_mode_check_init()
{
	tpi_wan_mode_check_struct_init();
	tpi_wan_mode_check_first_init();
    rc_register_module_msg_opses(rc_wan_mode_check_ops,RC_WAN_MODE_CHECK_MODULE,ARRAY_SIZE(rc_wan_mode_check_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_WAN_MODE_CHECK_MODULE,"rc_wan_mode_check",api_wan_mode_check_init,NULL);
