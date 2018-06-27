#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "bcmdnsmasq.h"

RET_INFO api_dnsmasq_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_dnsmasq_init();

RET_INFO api_dnsmasq_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_dnsmasq_action(var);
}

RET_INFO api_lan_dnsmasq_handle(RC_MODULES_COMMON_STRUCT *var)
{	
    return api_dnsmasq_handle(var);
}

static struct rc_msg_ops rc_dnsmasq_ops[] =
{
    {
        .intent_module_id = RC_DNSMASQ_MODULE,
		.type = INTENT_NONE,
        .ops = api_dnsmasq_handle,
    },
};

static RET_INFO api_dnsmasq_init()
{
	tpi_dnsmasq_struct_init();
	tpi_dnsmasq_first_init();
    rc_register_module_msg_opses(rc_dnsmasq_ops,RC_DNSMASQ_MODULE,ARRAY_SIZE(rc_dnsmasq_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_DNSMASQ_MODULE,"rc_dnsmasq",api_dnsmasq_init,NULL);
