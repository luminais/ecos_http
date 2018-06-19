/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ddns.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
//#include <cfg_net.h>
//#include <sys_status.h>
//#include <eventlog.h>

#include <sys/syslog.h>
//roy+++
#include <router_net.h>
//

//==============================================================================
//                                    MACROS
//==============================================================================
#define DDNS_DBG(c...)	//syslog(LOG_DEBUG,##c)
#define DDNS_LOG(c...)	syslog(LOG_INFO|LOG_USER, ##c)

#define CONFIG_ORAY_SERVER "phservice2.oray.net"

#define EZIP_NAME "ez-ip"
#define EZIP_DEFAULT_SERVER "www.EZ-IP.Net"
#define EZIP_DEFAULT_PORT 80
#define EZIP_REQUEST "/members/update/"

#define DYNDNS_NAME "dyndns"
#define DYNDNS_DEFAULT_SERVER "members.dyndns.org"
#define DYNDNS_DEFAULT_PORT 80
#define DYNDNS_REQUEST "/nic/update"
#define DYNDNS_STAT_REQUEST "/nic/update"
#define DYNDNSDYNDNS_MAX_INTERVAL (25*24*3600)

#define DYNDNS_ST_NAME "dyndns_static"
#define DYNDNS_ST_DEFAULT_SERVER "members.dyndns.org"
#define DYNDNS_ST_DEFAULT_PORT 80
#define DYNDNS_ST_REQUEST "/nic/update"
#define DYNDNS_ST_STAT_REQUEST "/nic/update"
#define DYNDNSDYNDNS_ST_MAX_INTERVAL (25*24*3600)

#define DYNDNS_CU_NAME "dyndns_cust"
#define DYNDNS_CU_DEFAULT_SERVER "members.dyndns.org"
#define DYNDNS_CU_DEFAULT_PORT 80
#define DYNDNS_CU_REQUEST "/nic/update"
#define DYNDNS_CU_STAT_REQUEST "/nic/update"
#define DYNDNSDYNDNS_CU_MAX_INTERVAL (25*24*3600)

#define ALLDDNS_NAME "allddns"
#define ALLDDNS_DEFAULT_SERVER "www.allddns.com"
#define ALLDDNS_DEFAULT_PORT 80
#define ALLDDNS_REQUEST "/nic/update"
#define ALLDDNS_STAT_REQUEST "/nic/update"
#define ALLDDNS_MAX_INTERVAL (25*24*3600)

#define QDNS_NAME "qdns"
#define QDNS_DEFAULT_SERVER "members.3322.org"
#define QDNS_DEFAULT_PORT 80
#define QDNS_REQUEST "/dyndns/update"
#define QDNS_STAT_REQUEST "/dyndns/update"
#define QDNS_MAX_INTERVAL (25*24*3600)

#define ODS_NAME "ods"
#define ODS_DEFAULT_SERVER "update.ods.org"
#define ODS_DEFAULT_PORT 7070
#define ODS_REQUEST "update"

#define TZO_NAME "tzo"
#define TZO_DEFAULT_SERVER "cgi.tzo.com"
#define TZO_DEFAULT_PORT 80
#define TZO_REQUEST "/webclient/signedon.html"

#define JUSTL_NAME "minidns"
#define JUSTL_DEFAULT_SERVER "www.minidns.net"
#define JUSTL_DEFAULT_PORT 80
#define JUSTL_REQUEST "/areg.php"

#define DHK_NAME "domainhk"
#define DHK_DEFAULT_SERVER "www.3domain.hk"
#define DHK_DEFAULT_PORT 80
#define DHK_REQUEST "/areg.php"


#define DYNS_NAME "dyns"
#define DYNS_DEFAULT_SERVER "www.dyns.cx"
#define DYNS_DEFAULT_PORT 80
#define DYNS_REQUEST "/postscript.php"

#define HN_NAME "hn"
#define HN_DEFAULT_SERVER "dup.hn.org"
#define HN_DEFAULT_PORT 80
#define HN_REQUEST "/vanity/update"

#define ZOE_NAME "zoneedit"
#define ZOE_DEFAULT_SERVER "www.zoneedit.com"
#define ZOE_DEFAULT_PORT 80
#define ZOE_REQUEST "/auth/dynamic.html"

#define ORAY_NAME "oray"
#define ORAY_DEFAULT_SERVER CONFIG_ORAY_SERVER
#define ORAY_DEFAULT_PORT 6060
#define ORAY_REQUEST "/auth/dynamic.html"

#define DTDNS_NAME "dtdns"
#define DTDNS_DEFAULT_SERVER "www.dtdns.com"
#define DTDNS_DEFAULT_PORT 80
#define DTDNS_REQUEST "/api/autodns.cfm?"
#define DTDNS_STAT_REQUEST ""
#define DTDNS_MAX_INTERVAL (25*24*3600)

#define NOIP_NAME "noip"
#define NOIP_DEFAULT_SERVER "dynupdate.no-ip.com"
#define NOIP_DEFAULT_PORT 80
#define NOIP_REQUEST ""
#define NOIP_STAT_REQUEST ""
#define NOIP_MAX_INTERVAL (25*24*3600)
#define NOIP_ALREADYSET               0
#define NOIP_SUCCESS                  1
#define NOIP_BADHOST                  2
#define NOIP_BADPASSWD                3
#define NOIP_BADUSER                  4
#define NOIP_TOSVIOLATE               6
#define NOIP_PRIVATEIP                7
#define NOIP_HOSTDISABLED             8
#define NOIP_HOSTISREDIRECT           9
#define NOIP_BADGRP                  10
#define NOIP_SUCCESSGRP              11
#define NOIP_ALREADYSETGRP           12
#define NOIP_RELEASEDISABLED         99

#define CHANGEIP_NAME "changeip"
#define CHANGEIP_DEFAULT_SERVER "www.changeip.com"
#define CHANGEIP_DEFAULT_PORT 80
#define CHANGEIP_REQUEST "https://changeip.com/nic/update"

#define OVH_NAME "ovh"
#define OVH_DEFAULT_SERVER "ovh.com"
#define OVH_DEFAULT_PORT 80
#define OVH_REQUEST "/nic/update"

#define EURODYNDNS_NAME "eurodyndns"
#define EURODYNDNS_DEFAULT_SERVER "eurodyndns.org"
#define EURODYNDNS_DEFAULT_PORT 80
#define EURODYNDNS_REQUEST "/update/"

#define REGFISH_NAME "regfish"
#define REGFISH_DEFAULT_SERVER "www.regfish.com"
#define REGFISH_DEFAULT_PORT 80
#define REGFISH_REQUEST "/dyndns/2/"

#define M88IP_NAME "88ip"
#define M88IP_DEFAULT_SERVER "user.dipns.com"
#define M88IP_DEFAULT_PORT 80
#define M88IP_REQUEST "/api/"

#define MY_IP			SYS_wan_ip
#define MY_IP_STR		NSTR(SYS_wan_ip)

#define ROUTER_NAME		"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)"
#define FW_VERSION		""
#define FW_OS			"ECOS"
#define AUTHOR			"by RogerC"

//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================
struct user_info;

struct service_t
{
  char *name;
  int (*update_entry)(struct user_info *);
  char *default_server;
  unsigned short default_port;
  char *default_request;
  int (*init_entry)(struct user_info *);
  int manual_timeout;
};

enum {
	UPDATERES_OK = 0,
	UPDATERES_ERROR,
	UPDATERES_SHUTDOWN
};

struct user_info {
	struct service_t *service;
	char usrname[60];
	char usrpwd[40];
	char host[30];
	char mx[30];
	char url[30];
	char wildcard;
	int trytime;
	int ip;
	unsigned long updated_time;
	int ticks;
	int status;
	struct user_info *next;
};

struct ORAY_DOMAIN_INFO
{
	unsigned char DName[50];
	int  Statuscode;
};


void DDNS_init(void);
void DDNS_down(void);
void DDNS_mainloop(void);
void DDNS_add_account(struct user_info *account);

//==============================================================================
//                               LOCAL VARIABLES
//==============================================================================

//==============================================================================
//                          LOCAL FUNCTION PROTOTYPES
//==============================================================================

//==============================================================================
//                              EXTERNAL FUNCTIONS
//==============================================================================



