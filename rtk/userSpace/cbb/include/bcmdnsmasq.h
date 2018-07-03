#ifndef __BCMDNSMASQ_H__
#define __BCMDNSMASQ_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

/*API*/
extern RET_INFO api_lan_dnsmasq_handle(RC_MODULES_COMMON_STRUCT *var);

/*GPI*/

/*TPI*/
extern RET_INFO tpi_dnsmasq_action(RC_MODULES_COMMON_STRUCT *var);
extern RET_INFO tpi_dnsmasq_struct_init();
extern RET_INFO tpi_dnsmasq_first_init();
#endif/*__BCMDNSMASQ_H__*/
