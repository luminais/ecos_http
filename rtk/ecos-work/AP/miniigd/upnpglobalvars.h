/* $Id: upnpglobalvars.h,v 1.4 2009/10/23 07:32:59 bert Exp $ */
/* project : MiniUPnP
 * website : http://miniupnp.free.fr
 * Author : Thomas Bernard
 * This software is subject to the conditions detailed in
 * the LICENCE file provided within this distribution */
#ifndef __UPNPGLOBALVARS_H__
#define __UPNPGLOBALVARS_H__

#include <time.h>

#define MAX_LEN_IF_NAME 20
#define UUID_LEN					36
#define	MAX_RULE_LEN	110
#define MAX_SERVER_LEN     72
#define BACKUP_RULES_FILENAME	("/tmp/miniigd_rules.bak")
#define DEFAULT_CONFIG_FILENAME		("/etc/miniigd.conf")

/* name of the network interface used to acces internet */
extern char ext_if_name[];
extern char wan_if_name[16];

/* IP address of this interface */
extern char ext_ip_addr[];

/* LAN address */
extern char listen_addr[];

/* parameters to return to upnp client when asked */

extern unsigned long downstream_bitrate;
extern unsigned long upstream_bitrate;

/* log packets flag */
extern int logpackets;

/* statup time */
extern time_t startup_time;
/* use system uptime */
extern int sysuptime;

extern const char * pidfilename;

extern char uuidvalue[];

#ifdef ENABLE_NATPMP
/* UPnP permission rules : */
extern struct upnpperm * upnppermlist;
extern unsigned int num_upnpperm;

/* NAT-PMP */
extern unsigned int nextnatpmptoclean_timestamp;
extern unsigned short nextnatpmptoclean_eport;
extern unsigned short nextnatpmptoclean_proto;
#endif


extern time_t wan_uptime;
extern int wan_states;
extern char location_url[];
extern char server_id[];
extern char ssdp_ext[];
extern unsigned int ssdp_mx;
extern int isCfgFromFile;

#define PRESENTATIONURL_LEN (32)
extern char presentationurl[];

#endif

