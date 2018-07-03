#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp_server.h"

static RET_INFO api_dhcp_server_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_dhcp_server_init();
extern RET_INFO api_dnsmasq_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_dhcp_server_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_dhcp_server_action(var);
}

#ifdef __CONFIG_TC__
RET_INFO api_tc_dhcp_server_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_tc_dhcp_server_action(var);
}
#endif

static struct rc_msg_ops rc_dhcp_server_ops[] =
{
    {
        .intent_module_id = RC_DHCP_SERVER_MODULE,
		.type = INTENT_NONE,
        .ops = api_dhcp_server_handle,
    },
    {
        .intent_module_id = RC_DNSMASQ_MODULE,
		.type = INTENT_NONE,
        .ops = api_dnsmasq_handle,
    },
};

static RET_INFO api_dhcp_server_init()
{
	tpi_dhcp_server_struct_init();
	tpi_dhcp_server_first_init();
    rc_register_module_msg_opses(rc_dhcp_server_ops,RC_DHCP_SERVER_MODULE,ARRAY_SIZE(rc_dhcp_server_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_DHCP_SERVER_MODULE,"rc_dhcp_server",api_dhcp_server_init,NULL);
