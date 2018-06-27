#ifndef __MANUFACTURER_H__
#define __MANUFACTURER_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef struct elink_info_struct
{
	PIU8 enable;
    int status; // 0 is not connect , 1 is connected to smart GW.
	
}ELINK_INFO_STRUCT,*P_ELINK_INFO_STRUCT;

/*TPI*/
extern RET_INFO tpi_elink_first_init();
extern RET_INFO tpi_elink_action(RC_MODULES_COMMON_STRUCT *var);
#endif/*__MANUFACTURER_H__*/