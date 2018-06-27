#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#define	ONE_DAY_IN_SEC	86400	//24*60*60

typedef enum time_check_result
{ 
	TIME_OUT_RANGE = 0,
	TIME_IN_RANGE,
}TIME_CHECK_RESULT;

typedef enum time_update_result
{ 
	TIME_UPDATE_FAIL = 0,
	TIME_UPDATE_SUCC,
}TIME_UPDATE_RESULT;

/*GPI*/
extern TIME_CHECK_RESULT gpi_common_time_check_result(PIU32 start_time_sec,PIU32 end_time_sec);
extern TIME_UPDATE_RESULT gpi_common_time_update_result();

/*TPI*/
extern TIME_CHECK_RESULT tpi_common_time_check_result(PIU32 start_time_sec,PIU32 end_time_sec);
extern TIME_UPDATE_RESULT tpi_common_time_update_result();

#endif/*__COMMON_H__*/
