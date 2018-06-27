#ifndef __BCMUPNP_H__
#define __BCMUPNP_H__

#ifndef  __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef  __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef struct upnp_info_struct
{
	PIU8 enable;
	
}UPNP_INFO_STRUCT,*P_UPNP_INFO_STRUCT;

/*API*/

/*GPI*/
extern P_UPNP_INFO_STRUCT gpi_upnp_get_info();
/*TPI*/
extern RET_INFO tpi_upnp_update_info();
extern RET_INFO tpi_upnp_struct_init();
extern RET_INFO tpi_upnp_first_init();
RET_INFO tpi_wan_upnp_action(RC_MODULES_COMMON_STRUCT *var);
extern RET_INFO tpi_upnp_action(RC_MODULES_COMMON_STRUCT *var);
extern P_UPNP_INFO_STRUCT tpi_upnp_get_info();
#endif /*__BCMUPNP_H__*/
