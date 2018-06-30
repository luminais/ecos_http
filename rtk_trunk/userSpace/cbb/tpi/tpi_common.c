/*
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"

TIME_CHECK_RESULT tpi_common_time_check_result(PIU32 start_time_sec,PIU32 end_time_sec)
{
	TIME_CHECK_RESULT ret = TIME_OUT_RANGE;
	struct tm *p;
	time_t now_time;
	PIU32 now_time_sec;
	time(&now_time);
	p = localtime(&now_time);

	now_time_sec = p->tm_hour * 60 * 60 + p->tm_min * 60 + p->tm_sec;

	if(start_time_sec <= end_time_sec)
	{
		if(start_time_sec <= now_time_sec && now_time_sec <= end_time_sec)
			ret = TIME_IN_RANGE;
		else
			ret = TIME_OUT_RANGE;
	}
	else
	{
		if(start_time_sec <= now_time_sec || now_time_sec <= end_time_sec)
			ret = TIME_OUT_RANGE;
		else
			ret = TIME_IN_RANGE;		
	}

	return ret;	
}

TIME_UPDATE_RESULT tpi_common_time_update_result()
{
	time_t now;
	struct tm TM;
	now = time(0);
	gmtime_r(&now,&TM);
	
	if(TM.tm_year != 70)
	{
		return TIME_UPDATE_SUCC;
	}
	
	return TIME_UPDATE_FAIL;
}
