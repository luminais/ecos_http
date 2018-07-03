#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apclient_dhcpc.h"
#include <bcmnvram.h>

static RET_INFO api_apclient_dhcpc_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_apclient_dhcpc_init();
RET_INFO api_apclient_wifi_dhcpc_action(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	if(nvram_match(SYSCONFIG_WORKMODE,"wisp") 
		||nvram_match(SYSCONFIG_WORKMODE,"route"))
	{
		PI_ERROR(TPI,"mode is ap,not need restart_dhcpc_only!\n");
		return RET_SUC;
	}
	if((0 == strcmp(var->wlan_ifname,TENDA_WLAN24_REPEATER_IFNAME) 
		||0 == strcmp(var->wlan_ifname,TENDA_WLAN5_REPEATER_IFNAME))
		&& var->op == OP_RESTART)
	{
		RC_MODULES_COMMON_STRUCT dhcpc_var;
		memset(&dhcpc_var,0,sizeof(dhcpc_var));
		strcpy((&dhcpc_var)->string_info,"restart_dhcpc_only");
   		return tpi_apclient_dhcpc_action(&dhcpc_var);
	}
}

static RET_INFO api_apclient_dhcpc_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_apclient_dhcpc_action(var);
}

static struct rc_msg_ops rc_apclient_dhcpc_ops[] =
{
    {
        .intent_module_id = RC_APCLIENT_DHCPC_MODULE,
		.type = INTENT_NONE,
        .ops = api_apclient_dhcpc_handle,
    },
};

static RET_INFO api_apclient_dhcpc_init()
{
	tpi_apclient_dhcpc_struct_init();
	tpi_apclient_dhcpc_first_init();
    rc_register_module_msg_opses(rc_apclient_dhcpc_ops,RC_APCLIENT_DHCPC_MODULE,ARRAY_SIZE(rc_apclient_dhcpc_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_APCLIENT_DHCPC_MODULE,"rc_apclient_dhcpc",api_apclient_dhcpc_init,NULL);
