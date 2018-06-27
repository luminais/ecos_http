#ifndef __WPS_H__
#define __WPS_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef struct wps_info_struct
{
	PIU8 enable;
	PI8 status;
	PI8 led_status;
	PI8 get_ip_status;
} WPS_INFO_STRUCT,*P_WPS_INFO_STRUCT;

/*API*/

/*GPI*/
#if 0
extern PI8 gpi_apclient_dhcpc_addr(PI8 *ip,PI8 *mask);
extern PI8 gpi_apclient_dhcpc_enable();
extern PI8 gpi_apclient_dhcpc_enable_by_mib();
#endif
/*TPI*/
extern RET_INFO tpi_wps_update_info();
extern RET_INFO tpi_wps_struct_init();
extern RET_INFO tpi_wps_first_init();
extern RET_INFO tpi_wps_action(RC_MODULES_COMMON_STRUCT *var);

extern PI8 tpi_wps_get_enable();

#endif/*__APCLIENT_DHCPC_H__*/
