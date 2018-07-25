#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "login_keep.h"

static RET_INFO api_login_keep_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_login_keep_init();

static RET_INFO api_login_keep_handle(RC_MODULES_COMMON_STRUCT *var)
{
    if(NULL == var)
    {
        PI_ERROR(API,"var is null!\n");
        return RET_ERR;
    }

    return tpi_login_keep_action(var);
}

static struct rc_msg_ops rc_login_keep_ops[] =
{
    {
        .intent_module_id = RC_LOGIN_KEEP_MODULE,
        .type = INTENT_NONE,
        .ops = api_login_keep_handle,
    },
};

static RET_INFO api_login_keep_init()
{
    tpi_login_keep_first_init();
    rc_register_module_msg_opses(rc_login_keep_ops,RC_LOGIN_KEEP_MODULE,ARRAY_SIZE(rc_login_keep_ops));
    return RET_SUC;
}

RC_MODULE_REGISTER(RC_LOGIN_KEEP_MODULE,"login_keep",api_login_keep_init,NULL);

