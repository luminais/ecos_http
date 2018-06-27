/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "reboot_check.h"

static RET_INFO api_restart_check_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_reboot_check_init();

static RET_INFO api_restart_check_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_reboot_check_action(var);
}

static struct rc_msg_ops rc_reboot_check_ops[] =
{
    {
        .intent_module_id = RC_REBOOT_CHECK_MODULE,
		.type = INTENT_NONE,
        .ops = api_restart_check_handle,
    },
};

static RET_INFO api_reboot_check_init()
{
	tpi_reboot_check_struct_init();
	tpi_reboot_check_first_init();
    rc_register_module_msg_opses(rc_reboot_check_ops,RC_REBOOT_CHECK_MODULE,ARRAY_SIZE(rc_reboot_check_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_REBOOT_CHECK_MODULE,"rc_restart_check",api_reboot_check_init,NULL);
