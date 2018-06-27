#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>

#include "wan.h"

RET_INFO gpi_wan_get_connect()
{
	return tpi_wan_get_connect();
}
PIU32 gpi_wan_get_err_result_info()
{
	return tpi_wan_get_err_result_info();
}

P_WAN_ERR_INFO_STRUCT gpi_wan_get_err_info_other()
{
	return tpi_wan_get_err_info_other();
}

P_WAN_INFO_STRUCT gpi_wan_get_info()
{	
	return tpi_wan_get_info();
}

P_WAN_HWADDR_INFO_STRUCT gpi_wan_get_hwaddr_info()
{	
	return tpi_wan_get_hwaddr_info();
}

P_WAN_CURRCET_INFO_STRUCT gpi_wan_get_currcet_info(P_WAN_CURRCET_INFO_STRUCT p)
{
	if(NULL != p)
	{
		p = tpi_wan_get_currcet_info(p);
	}
	
	return p;
}
