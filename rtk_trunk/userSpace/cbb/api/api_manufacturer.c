#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "manufacturer.h"

static RET_INFO api_manufacturer_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_manufacturer_action(var);
}

static struct rc_msg_ops rc_manufacturer_ops[] =
{
    {
        .intent_module_id = RC_MANUFACTURER_MODULE,
		.type = INTENT_NONE,
        .ops = api_manufacturer_handle,
    },
};
static RET_INFO api_manufacturer_init()
{
	tpi_manufacturer_first_init();
    rc_register_module_msg_opses(rc_manufacturer_ops,RC_MANUFACTURER_MODULE,ARRAY_SIZE(rc_manufacturer_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_MANUFACTURER_MODULE,"rc_manufacturer",api_manufacturer_init,NULL);


