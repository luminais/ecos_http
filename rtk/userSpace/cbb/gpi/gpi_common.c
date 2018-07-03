/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "elink.h"
#include "led.h"
/*获取elink状态信息*/
P_ELINK_INFO_STRUCT gpi_elink_get_info()
{
	return tpi_elink_get_info();
}
/*获取当前灯的状态*/
LED_STATUS gpi_led_current_state()
{
	return tpi_led_current_state();
}

TIME_CHECK_RESULT gpi_common_time_check_result(PIU32 start_time_sec,PIU32 end_time_sec)
{
	return tpi_common_time_check_result(start_time_sec,end_time_sec);
}

TIME_UPDATE_RESULT gpi_common_time_update_result()
{
	return tpi_common_time_update_result();
}
