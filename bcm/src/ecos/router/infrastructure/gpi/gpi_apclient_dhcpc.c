#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi_common.h"

extern PI_INFO tpi_get_apclient_dhcpc_info(char *ip,char *mask);
extern int tpi_get_apclient_dhcpc_enable();
extern char *get_wl0_mode(char *value);

int gpi_get_apclient_dhcpc_addr(char *ip,char *mask)
{
	if(NULL == ip || NULL == mask)
		return 0;
		
	return tpi_get_apclient_dhcpc_info(ip,mask);	
}

int gpi_get_apclient_dhcpc_enable()
{		
	return tpi_get_apclient_dhcpc_enable();	
}

int gpi_get_apclient_dhcpc_enable_by_mib()
{	
	char wl_mode[16] = {0};

	get_wl0_mode(wl_mode);
	if(0 == strcmp(wl_mode,"wet"))
	{
		return 1;
	}
	return 0;	
}

