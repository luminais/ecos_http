#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auto_conn_server.h"

static RET_INFO api_auto_conn_server_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_auto_conn_server_init();
extern RET_INFO api_auto_connn_wifi_handle(RC_MODULES_COMMON_STRUCT *var);

static RET_INFO api_auto_conn_server_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if (NULL == var)
	{
		PI_ERROR(TPI, "var is null!\n");
		return RET_ERR;
	}

	return tpi_auto_conn_server_action(var);
}

static struct rc_msg_ops rc_auto_conn_server_ops[] =
{
	{
		.intent_module_id = RC_AUTO_CONN_SERVER_MODULE,
		.type = INTENT_NONE,
		.ops = api_auto_conn_server_handle,
	}
};

static RET_INFO api_auto_conn_server_init()
{
	tpi_auto_conn_server_struct_init();
	tpi_auto_conn_server_first_init();
	rc_register_module_msg_opses(rc_auto_conn_server_ops, RC_AUTO_CONN_SERVER_MODULE, ARRAY_SIZE(rc_auto_conn_server_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_AUTO_CONN_SERVER_MODULE, "rc_auto_conn_server", api_auto_conn_server_init, NULL);
