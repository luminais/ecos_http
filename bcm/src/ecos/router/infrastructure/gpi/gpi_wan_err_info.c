#include <stdlib.h>
#include <string.h>

#include <bcmnvram.h>
#include "pi_wan_err_info.h"

extern WANERRINFO tpi_get_wan_err_info();

WANERRINFO gpi_get_wan_err_info()
{
	return tpi_get_wan_err_info();
}
