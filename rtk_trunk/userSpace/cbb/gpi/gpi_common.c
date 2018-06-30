/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

TIME_CHECK_RESULT gpi_common_time_check_result(PIU32 start_time_sec,PIU32 end_time_sec)
{
	return tpi_common_time_check_result(start_time_sec,end_time_sec);
}

TIME_UPDATE_RESULT gpi_common_time_update_result()
{
	return tpi_common_time_update_result();
}
