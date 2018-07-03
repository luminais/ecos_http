#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcmsntp.h"

static RET_INFO api_sntp_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_sntp_init();

static RET_INFO api_sntp_handle(RC_MODULES_COMMON_STRUCT *var)
{
    if(NULL == var)
    {
        PI_ERROR(TPI,"var is null!\n");
        return RET_ERR;
    }

    return tpi_sntp_action(var);
}

static struct rc_msg_ops rc_sntp_ops[] =
{
    {
        .intent_module_id = RC_SNTP_MODULE,
        .type = INTENT_NONE,
        .ops = api_sntp_handle,
    },
};

static RET_INFO api_sntp_init()
{
    tpi_sntp_struct_init();
    tpi_sntp_first_init();
    rc_register_module_msg_opses(rc_sntp_ops,RC_SNTP_MODULE,ARRAY_SIZE(rc_sntp_ops));
    return RET_SUC;
}

RC_MODULE_REGISTER(RC_SNTP_MODULE,"rc_sntp",api_sntp_init,NULL);
