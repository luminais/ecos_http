#include <stdlib.h>
#include <string.h>

#include <bcmnvram.h>
#include "pi_wifi.h"

extern PI_INFO tpi_get_wifi_status(WANSTATUS * wan_status,WIFISTASTATUS * wifi_status);
extern WLMODE tpi_get_wl_mode();

WLMODE gpi_get_wifi_mode()
{
	return tpi_get_wl_mode();
}

PI_INFO gpi_get_wifi_status_info(WANSTATUS *wan_status,WIFISTASTATUS *wifi_status)
{
	if(NULL == wan_status || NULL == wifi_status)
	{
		return PI_ERR;
	}
	return tpi_get_wifi_status(wan_status,wifi_status);
}
