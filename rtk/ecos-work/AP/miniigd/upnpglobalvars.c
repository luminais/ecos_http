/* $Id: upnpglobalvars.c,v 1.3 2009/07/02 01:26:29 bert Exp $ */
/* project: MiniUPnP
 * Website: http://miniupnp.free.fr/
 * Author : Thomas Bernard
 * This software is subject to the conditions
 * detailed in the LICENCE file provided in this
 * distribution. */
#include <sys/types.h>
#include <netinet/in.h>
#include "upnpglobalvars.h"

/* network interface for internet */
char ext_if_name[MAX_LEN_IF_NAME];
const char * lan_if_name = 0;

/* ip address of this interface */
char ext_ip_addr[INET_ADDRSTRLEN];

/* LAN address */
char lan_ip_addr[INET_ADDRSTRLEN];
char listen_addr[INET_ADDRSTRLEN];

unsigned long downstream_bitrate = 0;
unsigned long upstream_bitrate = 0;

#ifdef ENABLE_NATPMP
/* UPnP permission rules : */
struct upnpperm * upnppermlist = 0;
unsigned int num_upnpperm = 0;

/* NAT-PMP */
unsigned int nextnatpmptoclean_timestamp = 0;
unsigned short nextnatpmptoclean_eport = 0;
unsigned short nextnatpmptoclean_proto = 0;
#endif
char wan_if_name[16];
/* startup time */
time_t startup_time = 0;
/*wan iface uptime*/
time_t wan_uptime = 0;
int wan_states=-1;

/* use system uptime */
int sysuptime = 0;

/* log packets flag */
int logpackets = 0;

const char * pidfilename = "/var/run/miniupnpd.pid";

//char uuidvalue[] = "uuid:00000000-0000-0000-0000-000000000000";
//char uuidvalue[] = "uuid:02350000-4000-0000-0000-000000000001";

#ifdef CONFIG_RTL8186_KB
char uuidvalue[] = "uuid:82350000-4000-0000-0000-000000000001";
#else
#ifdef CONFIG_RTL8186_GR
char uuidvalue[] = "uuid:02350000-4000-0000-0000-000000000001";
#else
char uuidvalue[] = "uuid:12342409-1234-1234-5678-ee1234cc5678";
#endif
#endif

char location_url[72];
char server_id[72];
char ssdp_ext[72];
unsigned int ssdp_mx=0;
int isCfgFromFile=0;

/* presentation url :
 * http://nnn.nnn.nnn.nnn:ppppp/  => max 30 bytes including terminating 0 */
char presentationurl[PRESENTATIONURL_LEN];

