#ifndef __BCMUCLOUD_H__
#define __BCMUCLOUD_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

/* ucloud 配置参数 */
typedef struct ucloud_info_struct
{
    PIU8            enable;                    	 	/*开关*/	
} UCLOUD_INFO_STRUCT,*P_UCLOUD_INFO_STRUCT;


/*TPI*/
extern RET_INFO tpi_ucloud_update_info();
extern RET_INFO tpi_ucloud_struct_init();
extern RET_INFO tpi_ucloud_first_init();
extern RET_INFO tpi_ucloud_action(RC_MODULES_COMMON_STRUCT *var);

extern P_UCLOUD_INFO_STRUCT tpi_ucloud_get_info();
extern void  biz_m_ucloud_proc_start(void);
#endif/*__BCMUCLOUD_H__*/