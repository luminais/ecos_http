#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcmsntp.h"

P_SNTP_INFO_STRUCT gpi_sntp_info()
{
    return tpi_sntp_get_info();
}