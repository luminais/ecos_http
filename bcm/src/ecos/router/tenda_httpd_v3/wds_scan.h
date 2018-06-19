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
#include <bcmconfig.h>
#include <opencrypto.h>
#include <time.h>
#include <epivers.h>
#include "router_version.h"
#include <proto/802.11.h>
#include <proto/802.1d.h>
#include <security_ipc.h>

#include "webs.h"
#include "uemf.h"


#define WDS_ENR_MAX_AP_SCAN_LIST_LEN  64
#define WDS_NUMCHANS 64
#define WDS_ENR_DUMP_BUF_LEN (64 * 512)
#define WDS_ENR_SCAN_RETRY_TIMES	5
#define WDS_NVRAM_BUFSIZE	100
#define ETHER_ADDR_STR_LEN	18	

typedef struct wds_ap_list_info
{
	bool        used;
	uint8       ssid[33];
	uint8       ssidLen; 
	uint8       BSSID[6];
	uint8       *ie_buf;
	uint32      ie_buflen;
	uint8       channel;
	uint8       wep;
	uint8		cipher;
	int16 	sig;
} wds_ap_list_info_t;




