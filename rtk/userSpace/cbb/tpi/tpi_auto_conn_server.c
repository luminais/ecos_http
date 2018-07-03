#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include <net/if.h>
#include "wlioctl.h"
#include <wlutils.h>

#include "sys_module.h"
#include "http.h"
#include "auto_conn_server.h"

#define AUTO_CONN_SERVER_SIZE		8*1024
static cyg_handle_t auto_conn_server_daemon_handle;
static PI8 auto_conn_server_daemon_stack[AUTO_CONN_SERVER_SIZE];
static cyg_thread auto_conn_server_daemon_thread;

static AUTO_CONN_SERVER_INFO_STRUCT auto_conn_server_info;

static RET_INFO tpi_auto_conn_server_handle(MODULE_COMMON_OP_TYPE action);
static RET_INFO tpi_auto_conn_server_main();
static RET_INFO tpi_auto_conn_server_start();
static RET_INFO tpi_auto_conn_server_stop();
static RET_INFO tpi_auto_conn_server_restart();

/*以下函数用于api调用*/
extern void auto_conn_extend_status_init();
RET_INFO tpi_auto_conn_server_update_info()
{
	if (nvram_match("restore_quick_set", "0"))
	{
		auto_conn_server_info.enable = 1;
	}
	else
	{
		auto_conn_server_info.enable = 0;
	}

	return RET_SUC;
}

RET_INFO tpi_auto_conn_server_struct_init()
{
	memset(&auto_conn_server_info, 0x0, sizeof(auto_conn_server_info));
	return tpi_auto_conn_server_update_info();
}

RET_INFO tpi_auto_conn_server_first_init()
{
	if (auto_conn_server_info.enable)
	{
		tpi_auto_conn_server_start();
	}

	return RET_SUC;
}

RET_INFO tpi_auto_conn_server_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI, "op=%d\n", var->op);

	switch (var->op)
	{
		case OP_START:
			tpi_auto_conn_server_start();

			break;

		case OP_STOP:
			tpi_auto_conn_server_stop();
			break;

		case OP_RESTART:
			tpi_auto_conn_server_restart();
			break;

		default:
			PI_ERROR(TPI, "op[%d] donnot have handle!\n", var->op);
			break;
	}

	return RET_SUC;
}

/*以下用于gpi获取信息函数*/
PI8 tpi_auto_conn_server_get_enable()
{
	return auto_conn_server_info.enable;
}


/*以下为该模块具体执行实现函数*/


/*只有本文件里面用*/
extern void auto_conn_route_main();
static RET_INFO tpi_auto_conn_server_main()
{
	auto_conn_route_main();

	return RET_SUC;
}
static RET_INFO tpi_auto_conn_server_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if (auto_conn_server_daemon_handle != 0)
	{

		auto_conn_route_stop();

		/* Wait until thread exit */
		pid = oslib_getpidbyname("auto_conn_server");

		if (pid)
		{
			while (oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}

		cyg_thread_delete(auto_conn_server_daemon_handle);

		PI_PRINTF(TPI, "stop success!\n");

		auto_conn_server_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI, "is already stop!\n");
	}

	return ret;
}
static RET_INFO tpi_auto_conn_server_restart()
{
	RET_INFO ret = RET_SUC;

	if (RET_ERR == tpi_auto_conn_server_stop() || RET_ERR == tpi_auto_conn_server_start())
	{
		PI_ERROR(TPI, "restart error!\n");
	}

	return ret;
}


static RET_INFO tpi_auto_conn_server_start()
{
	RET_INFO ret = RET_SUC;

	if (auto_conn_server_daemon_handle == 0)
	{
		tpi_auto_conn_server_update_info();

		if (auto_conn_server_info.enable)
		{
			cyg_thread_create(
			    8,
			    (cyg_thread_entry_t *)tpi_auto_conn_server_main,
			    0,
			    "auto_conn_server",
			    auto_conn_server_daemon_stack,
			    sizeof(auto_conn_server_daemon_stack),
			    &auto_conn_server_daemon_handle,
			    &auto_conn_server_daemon_thread);

			cyg_thread_resume(auto_conn_server_daemon_handle);
			cyg_thread_delay(1);
			PI_PRINTF(TPI, "start success!\n");
		}
		else
		{
			PI_ERROR(TPI, "the mib is off, connot start!\n");
		}
	}
	else
	{
		PI_PRINTF(TPI, "is already start!\n");
	}

	return ret;
}

