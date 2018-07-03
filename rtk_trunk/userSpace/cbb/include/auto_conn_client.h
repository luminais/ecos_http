#ifndef __AUTO_CONN_CLINET_H__
#define __AUTO_CONN_CLINET_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef struct auto_conn_client_info_struct
{
	PIU8 enable;
	PI8 status;
	PI8 led_status;
	PI8 get_ip_status;
	PIU8 restarting_wifi;/*用于标记自动桥接最后一步是否正在重启无线，在APCLIENT_DHCPC中使用*/
} AUTO_CONN_CLIENT_INFO_STRUCT, *P_AUTO_CONN_CLIENT_INFO_STRUCT;

/*API*/

/*GPI*/
#if 0
extern PI8 gpi_apclient_dhcpc_addr(PI8 *ip, PI8 *mask);
extern PI8 gpi_apclient_dhcpc_enable();
extern PI8 gpi_apclient_dhcpc_enable_by_mib();
#endif
/*TPI*/
extern RET_INFO tpi_auto_conn_client_update_info();
extern RET_INFO tpi_auto_conn_client_struct_init();
extern RET_INFO tpi_auto_conn_client_first_init();
extern RET_INFO tpi_auto_conn_client_action(RC_MODULES_COMMON_STRUCT *var);

extern PI8 tpi_auto_conn_client_get_enable();

#endif/*__APCLIENT_DHCPC_H__*/
