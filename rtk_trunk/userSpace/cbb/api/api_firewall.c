#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "firewall.h"

static RET_INFO api_firewall_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_firewall_init();

static RET_INFO api_firewall_handle(RC_MODULES_COMMON_STRUCT *var)
{
    if(NULL == var)
    {
        PI_ERROR(API,"var is null!\n");
        return RET_ERR;
    }

    return tpi_firewall_action(var);
}

#ifdef __CONFIG_TC__
RET_INFO api_tc_firewall_handle(RC_MODULES_COMMON_STRUCT *var)
{
    if(NULL == var)
    {
        PI_ERROR(API,"var is null!\n");
        return RET_ERR;
    }

    return tpi_tc_firewall_action(var);
}
#endif

RET_INFO api_lan_firewall_handle(RC_MODULES_COMMON_STRUCT *var)
{
    return api_firewall_handle(var);
}

static struct rc_msg_ops rc_firewall_ops[] =
{
    {
        .intent_module_id = RC_FIREWALL_MODULE,
        .type = INTENT_NONE,
        .ops = api_firewall_handle,
    },
};

static RET_INFO api_firewall_init()
{
    tpi_firewall_struct_init();
    tpi_firewall_first_init();
    rc_register_module_msg_opses(rc_firewall_ops,RC_FIREWALL_MODULE,ARRAY_SIZE(rc_firewall_ops));
    return RET_SUC;
}

RC_MODULE_REGISTER(RC_FIREWALL_MODULE,"rc_firewall",api_firewall_init,NULL);
