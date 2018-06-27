#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wan.h"
#include "bcmddns.h"
static RET_INFO api_wan_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_wan_init();

static MODULE_WORK_STATUS api_wan_callback(MODULE_CALLBACK_TYPE type)
{
	MODULE_WORK_STATUS ret = MODULE_BEGIN;
	
	switch(type)
	{
		case MODULE_CALLBACK_GET:
			ret = tpi_wan_get_action_type();
			break;
		case MODULE_CALLBACK_REINIT:
			tpi_wan_action_reinit();
			break;
		default:
			PI_PRINTF(API,"callbak type[%d] is not unknow!\n",type);
			ret = MODULE_COMPLETE;
			break;
	}
	
	return ret;
}

static RET_INFO api_wan_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

    return tpi_wan_action(var);
}

/*A handle contact to the systools*/
RET_INFO api_wan_systools_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

    return tpi_wan_systools_action(var);	
}

/*wan module register infomation*/
extern RET_INFO api_wan_upnp_handle(RC_MODULES_COMMON_STRUCT *var);

struct rc_msg_ops rc_wan_ops[] =
{
    {
        .intent_module_id = RC_WAN_MODULE,
		.type = INTENT_NONE,
        .ops = api_wan_handle,
    },
    {
		/*这里将UPNP与WAN口关联，因为UPNP启动的时候需要读取WAN口接口名称，如果不关联的话，UPNP功能不正常*/
        .intent_module_id = RC_UPNP_MODULE,
		.type = INTENT_NEXT,
        .ops = api_wan_upnp_handle,
    },
};

static RET_INFO api_wan_init()
{
	tpi_wan_struct_init();
	tpi_wan_first_init();
    rc_register_module_msg_opses(rc_wan_ops,RC_WAN_MODULE,ARRAY_SIZE(rc_wan_ops));
	return RET_SUC;
}

PI32 api_wan_rc(int argc, char **argv)
{	
	char *base = argv[0];
		

#ifdef	__CONFIG_DHCPC__
	if (strstr(base, "wandhcpc")) {
		return wandhcpc(argc, argv);
	}
#ifdef __CONFIG_8021X__
	if (strstr(base, "8021xdhcpc")) {
		return wandhcpc(argc, argv);
	}
#endif
	if (strstr(base, "landhcpc")) {
		return landhcpc(argc, argv);
	}
#ifdef	__CONFIG_NAT__
#ifdef	__CONFIG_PPTP__
	if (strstr(base, "pptpdhcpc")) {
		return pptpdhcpc(argc, argv);
	}
#endif
#ifdef	__CONFIG_L2TP__
	if (strstr(base, "l2tpdhcpc")) {
		return l2tpdhcpc(argc, argv);
	}
#endif
#ifdef  __CONFIG_PPPOE2__
	if (strstr(base, "pppoe2dhcpc")) {
		return pppoe2dhcpc(argc, argv);
	}
#endif
#endif	/* __CONFIG_NAT__ */
#endif	/* __CONFIG_DHCPC__ */




#ifdef	__CONFIG_NAT__
#ifdef	__CONFIG_PPP__
	else if (strstr(base, "ip-up")) {
		return ipup_main(argc, argv);
	}
	else if (strstr(base, "ip-down")) {
		return ipdown_main(argc, argv);
	}
#endif
#endif	/* __CONFIG_NAT__ */




	return 0;
}

RC_MODULE_REGISTER(RC_WAN_MODULE,"rc_wan",api_wan_init,api_wan_callback);
