/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "reboot_check.h"

RET_INFO gpi_reboot_check_time(PIU32 *start_time,PIU32 *end_time)
{
	if(NULL == start_time || NULL == end_time)
	{
		PI_ERROR(GPI,"start_time or end_time is NULL!\n");
		return RET_ERR;
	}
	
	return tpi_reboot_check_time(start_time,end_time);
}

PIU8 gpi_reboot_check_enable()
{
	return tpi_reboot_check_enable();
}

