#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcmucloud.h"

P_UCLOUD_INFO_STRUCT gpi_ucloud_info()
{
    return tpi_ucloud_get_info();
}