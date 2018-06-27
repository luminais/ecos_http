#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "firewall.h"

P_FIREWALL_INFO_STRUCT gpi_firewall_info()
{		
	return tpi_firewall_get_info();	
}