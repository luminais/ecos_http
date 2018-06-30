#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wifi_switch_sched.h"

RET_INFO gpi_wifi_switch_sched_web_info(PI8 *enable,PI8 *times,PI8 *weeks)
{
	if(NULL == enable || NULL == times || NULL == weeks)
		return RET_ERR;
		
	return tpi_wifi_switch_sched_web_info(enable,times,weeks);	
}