#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcmddns.h"

P_DDNS_INFO_STRUCT gpi_ddns_info()
{
    return tpi_ddns_get_info();
}