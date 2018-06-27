#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp_server.h"

P_DHCP_SERVER_INFO_STRUCT gpi_dhcp_server_info()
{		
	return tpi_dhcp_server_get_info();	
}

P_STATIC_IP_LIST gpi_dhcp_server_static_ip_get_info()
{
    return tpi_dhcp_server_static_ip_get_info();
}

