#ifndef __FIREWALL_H__
#define __FIREWALL_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef enum
{
	WIRE_WIRED = 0,
	WIRE_WIRELESS,
}WIRE_TYPE;


/*API*/

/*GPI*/

/*TPI*/
extern RET_INFO tpi_tc_update_info();
extern RET_INFO tpi_tc_struct_init();
extern RET_INFO tpi_tc_first_init();
extern RET_INFO tpi_tc_action(RC_MODULES_COMMON_STRUCT *var);
#endif/*__FIREWALL_H__*/
