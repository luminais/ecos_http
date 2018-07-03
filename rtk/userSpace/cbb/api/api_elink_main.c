#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "elink.h"

static RET_INFO api_elink_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_elink_action(var);
}

 
static struct rc_msg_ops rc_elink_ops[] =
{
    {
        .intent_module_id = RC_ELINK_MODULE,
		.type = INTENT_NONE,
        .ops = api_elink_handle,
    },
};


static RET_INFO api_elink_init()
{
	tpi_elink_first_init();
    
    rc_register_module_msg_opses(rc_elink_ops,RC_ELINK_MODULE,ARRAY_SIZE(rc_elink_ops));
    
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_ELINK_MODULE,"rc_elink",api_elink_init,NULL);

