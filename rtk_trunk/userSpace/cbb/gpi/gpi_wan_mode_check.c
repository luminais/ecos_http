/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wan_mode_check.h"

WANMODE gpi_wan_mode_check_get_check_result()
{
	WANMODE ret = WAN_MAX_MODE;
	CHECK_RESULT check_finish_tag = tpi_wan_mode_check_get_check_finish_tag();
	WANMODE check_result = tpi_wan_mode_check_get_check_result();

	if(CHECK_END == check_finish_tag)
	{
		ret = check_result;
	}

	return ret;
}