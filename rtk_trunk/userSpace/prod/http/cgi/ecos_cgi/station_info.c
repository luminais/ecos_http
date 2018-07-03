#include <stdlib.h>
#include <stdio.h>
 #include <unistd.h>

#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmparams.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <netconf.h>
#include <nvparse.h>
#include <wlutils.h>
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"
#include "../tc/tc.h"

#define MAX_STA_COUNT	256
#define NVRAM_BUFSIZE	100
