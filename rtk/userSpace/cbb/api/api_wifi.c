#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "wifi.h"

extern RET_INFO api_apclient_wifi_dhcpc_action(RC_MODULES_COMMON_STRUCT *var);
#ifdef __CONFIG_WPS_RTK__
extern RET_INFO api_wifi_wps_action(RC_MODULES_COMMON_STRUCT *var);
#endif


static RET_INFO api_wifi_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_wifi_init();

#ifdef __CONFIG_A9__
static MODULE_WORK_STATUS api_wifi_callback(MODULE_CALLBACK_TYPE type)
{
	MODULE_WORK_STATUS ret = MODULE_BEGIN;

	switch(type)
	{
		case MODULE_CALLBACK_GET:
			ret = tpi_wifi_get_action_type();
			break;
		case MODULE_CALLBACK_REINIT:
			tpi_wifi_action_reinit();
			break;
		default:
			PI_PRINTF(API,"callbak type[%d] is not unknow!\n",type);
			ret = MODULE_COMPLETE;
			break;
	}

	return ret;
}
#endif

static RET_INFO api_wifi_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_wifi_action(var);
}

#ifdef __CONFIG_AUTO_CONN_CLIENT__
RET_INFO api_auto_connn_wifi_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}

    return tpi_auto_connn_wifi_action(var);
}
#endif

static struct rc_msg_ops rc_wifi_ops[] =
{
    {
        .intent_module_id = RC_WIFI_MODULE,
		.type = INTENT_NONE,
        .ops = api_wifi_handle,
    },
    {
		/*重启次接口时关联停止dhcpc*/
        .intent_module_id = RC_APCLIENT_DHCPC_MODULE,
		.type = INTENT_PREV,
        .ops = api_apclient_wifi_dhcpc_action,
   },
   #ifdef __CONFIG_WPS_RTK__
   {
		/*如果WPS可用时，重启无线之后需要重启WPS*/
        .intent_module_id = RC_WPS_MODULE,
		.type = INTENT_NEXT,
        .ops = api_wifi_wps_action,
   },
   #endif
};

static RET_INFO api_wifi_init()
{
	tpi_wifi_struct_init();
	tpi_wifi_first_init();
    rc_register_module_msg_opses(rc_wifi_ops,RC_WIFI_MODULE,ARRAY_SIZE(rc_wifi_ops));
	return RET_SUC;
}

#ifdef __CONFIG_A9__
RC_MODULE_REGISTER(RC_WIFI_MODULE,"rc_wifi",api_wifi_init,api_wifi_callback);
#else
RC_MODULE_REGISTER(RC_WIFI_MODULE,"rc_wifi",api_wifi_init,NULL);
#endif
