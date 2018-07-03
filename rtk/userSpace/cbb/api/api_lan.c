#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#ifndef __CONFIG_NAT__
#include "firewall.h"
#endif
#ifndef __CONFIG_DNSMASQ__
#include "bcmdnsmasq.h"
#endif

static RET_INFO api_lan_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_lan_init();

static RET_INFO api_lan_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_lan_action(var);
}

static struct rc_msg_ops rc_lan_ops[] =
{
    {
        .intent_module_id = RC_LAN_MODULE,
		.type = INTENT_NONE,
        .ops = api_lan_handle,
    },
#ifndef __CONFIG_NAT__
    {
        .intent_module_id = RC_FIREWALL_MODULE,
		.type = INTENT_NEXT,
        .ops = api_lan_firewall_handle,
    },
#endif
#ifndef __CONFIG_DNSMASQ__
    {
        .intent_module_id = RC_DNSMASQ_MODULE,
		.type = INTENT_NEXT,
        .ops = api_lan_dnsmasq_handle,
    },
#endif
};

static RET_INFO api_lan_init()
{
	tpi_lan_struct_init();
	tpi_lan_first_init();
    rc_register_module_msg_opses(rc_lan_ops,RC_LAN_MODULE,ARRAY_SIZE(rc_lan_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_LAN_MODULE,"rc_lan",api_lan_init,NULL);
