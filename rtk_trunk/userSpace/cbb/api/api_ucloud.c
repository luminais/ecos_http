#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bcmucloud.h"
static RET_INFO api_ucloud_handle(RC_MODULES_COMMON_STRUCT * var);
static RET_INFO api_ucloud_init();

static RET_INFO api_ucloud_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_ucloud_action(var);
}

static struct rc_msg_ops rc_ucloud_ops[] =
{
    {
        .intent_module_id = RC_UCLOUD_MODULE,
		.type = INTENT_NONE,
        .ops = api_ucloud_handle,
    },
};
static RET_INFO api_ucloud_init()
{
	tpi_ucloud_struct_init();
	tpi_ucloud_first_init();
    rc_register_module_msg_opses(rc_ucloud_ops,RC_UCLOUD_MODULE,ARRAY_SIZE(rc_ucloud_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_UCLOUD_MODULE,"rc_ucloud",api_ucloud_init,NULL);


