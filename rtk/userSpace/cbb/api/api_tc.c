#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bcmtc.h"

#ifdef __CONFIG_DHCPD__
extern RET_INFO api_tc_dhcp_server_handle(RC_MODULES_COMMON_STRUCT *var);
#endif

#ifdef __CONFIG_NAT__
extern RET_INFO api_tc_firewall_handle(RC_MODULES_COMMON_STRUCT *var);
#endif


static RET_INFO api_tc_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_tc_init();

static RET_INFO api_tc_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

    return tpi_tc_action(var);
}

/*tc module register infomation*/
struct rc_msg_ops rc_tc_ops[] =
{
    {
        .intent_module_id = RC_TC_MODULE,
		.type = INTENT_NONE,
        .ops = api_tc_handle,
    },
#if 0 
#ifdef __CONFIG_DHCPD__
    {
        .intent_module_id = RC_DHCP_SERVER_MODULE,
		.type = INTENT_NEXT,
        .ops = api_tc_dhcp_server_handle,
    },
#endif
#ifdef __CONFIG_NAT__
    {
        .intent_module_id = RC_FIREWALL_MODULE,
		.type = INTENT_NEXT,
        .ops = api_tc_firewall_handle,
    },
#endif
#endif
};

static RET_INFO api_tc_init()
{
	tpi_tc_struct_init();
	tpi_tc_first_init();
    rc_register_module_msg_opses(rc_tc_ops,RC_TC_MODULE,ARRAY_SIZE(rc_tc_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_TC_MODULE,"rc_tc",api_tc_init,NULL);
