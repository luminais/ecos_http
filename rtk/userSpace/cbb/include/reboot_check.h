#ifndef __REBOOT_CHECK_H_
#define __REBOOT_CHECK_H_

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef enum
{
	REBOOT_CHECK_EACH_DAY = 0,
	REBOOT_CHECK_EACH_DAY_FLOW,
	REBOOT_CHECK_ONE_DAY,
	REBOOT_CHECK_ONE_DAY_FLOW,
}REBOOT_CHECK_TYPE;

#define REBOOT_TIME_CHECK RC_MODULE_1S_TIME

#define REBOOT_CHECK_MAX_TIME (30 * 60 * RC_MODULE_1S_TIME)//连续检测30分钟

#define REBOOT_CHECK_MAX_NUM (REBOOT_CHECK_MAX_TIME / REBOOT_TIME_CHECK)

#define REBOOT_CHECK_TIME_INTERVAL (RC_MODULE_1S_TIME * 5)

#define REBOOT_CHECK_STATR_TIME_RANGE (RC_MODULE_1S_TIME * 150 * 60)//开始检测时间随机范围

#define REBOOT_CHECK_STATR_TIME_INTERVAL_NUM (REBOOT_CHECK_STATR_TIME_RANGE / REBOOT_CHECK_TIME_INTERVAL)//开始检测时间随机个数

#define REBOOT_CHECK_DEFAULT_RX_SPEED (3000 * RC_MODULE_1S_TIME / RC_MODULE_1S_TIME)

#define REBOOT_CHECK_DEFAULT_TX_SPEED (3000 * RC_MODULE_1S_TIME / RC_MODULE_1S_TIME)

typedef struct reboot_check_info_struct
{
	PIU8 enable;
	REBOOT_CHECK_TYPE type;
	
	PIU32 start_time_sec;
	PIU32 end_time_sec;

	PIU32 max_tx_bytes;
	PIU32 max_rx_bytes;
}REBOOT_CHECK_INFO_STRUCT,*P_REBOOT_CHECK_INFO_STRUCT;

/*API*/

/*GPI*/
extern RET_INFO gpi_reboot_check_time(PIU32 *start_time,PIU32 *end_time);
extern PIU8 gpi_reboot_check_enable();

/*TPI*/
extern RET_INFO tpi_reboot_check_update_info();
extern RET_INFO tpi_reboot_check_struct_init();
extern RET_INFO tpi_reboot_check_first_init();
extern RET_INFO tpi_reboot_check_action(RC_MODULES_COMMON_STRUCT *var);

extern PIU8 tpi_reboot_check_enable();
extern RET_INFO tpi_reboot_check_time(PIU32 *start_time,PIU32 *end_time);
#endif/*__REBOOT_CHECK_H_*/
