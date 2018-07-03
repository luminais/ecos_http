#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "http.h"

static RET_INFO api_http_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_http_init();
	
static RET_INFO api_http_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_http_action(var);
}

static struct rc_msg_ops rc_http_ops[] =
{
    {
        .intent_module_id = RC_HTTP_MODULE,
		.type = INTENT_NONE,
        .ops = api_http_handle,
    },
};

static RET_INFO api_http_init()
{
	tpi_http_struct_init();
	tpi_http_first_init();
    rc_register_module_msg_opses(rc_http_ops,RC_HTTP_MODULE,ARRAY_SIZE(rc_http_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_HTTP_MODULE,"rc_http",api_http_init,NULL);
