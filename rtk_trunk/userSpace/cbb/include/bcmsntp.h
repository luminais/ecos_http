#ifndef __BCMSNTP_H__
#define __BCMSNTP_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

/* sntp 配置参数 */
typedef struct sntp_info_struct
{
    PIU8            enable;                     /*开关*/
    PI8_P           time_zone;                  /*时区*/
} SNTP_INFO_STRUCT,*P_SNTP_INFO_STRUCT;

/*API*/

/*GPI*/

/*TPI*/
extern RET_INFO tpi_sntp_update_info();
extern RET_INFO tpi_sntp_struct_init();
extern RET_INFO tpi_sntp_first_init();
extern RET_INFO tpi_sntp_action(RC_MODULES_COMMON_STRUCT *var);

extern P_SNTP_INFO_STRUCT tpi_sntp_get_info();
#endif/*__BCMSNTP_H__*/