/*
 * WIDE Project DHCP Implementation
 * Copyright (c) 1995, 1996 Akihiro Tominaga
 * Copyright (c) 1995, 1996 WIDE Project
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided the following conditions
 * are satisfied,
 *
 * 1. Both the copyright notice and this permission notice appear in
 *    all copies of the software, derivative works or modified versions,
 *    and any portions thereof, and that both notices appear in
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by WIDE Project and
 *      its contributors.
 * 3. Neither the name of WIDE Project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE DEVELOPER ``AS IS'' AND WIDE
 * PROJECT DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES
 * WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. ALSO, THERE
 * IS NO WARRANTY IMPLIED OR OTHERWISE, NOR IS SUPPORT PROVIDED.
 *
 * Feedback of the results generated from any improvements or
 * extensions made to this software would be much appreciated.
 * Any such feedback should be sent to:
 * 
 *  Akihiro Tominaga
 *  WIDE Project
 *  Keio University, Endo 5322, Kanagawa, Japan
 *  (E-mail: dhcp-dist@wide.ad.jp)
 *
 * WIDE project has the rights to redistribute these changes.
 */

#ifndef BINDING_DB
#define BINDING_DB              "/etc/dhcpdb.bind"
#endif

#ifndef PATH_PID
#define PATH_PID		"/var/run/dhcps.pid"
#endif

#ifndef ADDRPOOL_DB
#define ADDRPOOL_DB             "/etc/dhcpdb.pool"
#endif

#ifndef RELAY_DB
#define RELAY_DB                "/etc/dhcpdb.relay"
#endif

#ifdef SUPPORT_MAC_FILTER
#ifndef MAC_DB
#define MAC_DB                  "/etc/dhcpdb.MAC"
#endif
#endif

#define GC_INTERVAL    3600      /* interval between garbage collections */

#define DHCPD_THREAD_PRIORITY 16
//#define DHCPD_THREAD_STACK_SIZE 0x00004000
#define DHCPD_THREAD_STACK_SIZE 0x00002000
/*
 *  function prototype definition
 */
static void usage();
static void version();
static void alloc_sbuf();
static void garbage_collect();
static void become_daemon();
static void set_sighand();
static void timeout();
static void get_cid();
static void clean_sbuf();
static void init_db();
static void construct_msg();
static void turnoff_bind();
static char *nvttostr();
static int get_maxoptlen();
static int get_subnet();
static int available_res();
static int choose_lease();
#ifndef sun
static int check_pkt1();
#endif
static int check_pkt2();
static int update_db();
static int cidcmp();
static int icmp_check();
static int insert_it();
static int free_bind();
static int send_dhcp();
static int insert_opt();
#define PASSIVE 0
#define ACTIVE  1
static u_Long get_reqlease();
static struct dhcp_resource *select_wciaddr();
static struct dhcp_resource *select_wcid();
static struct dhcp_resource *select_wreqip();
static struct dhcp_resource *select_newone();
static struct dhcp_resource *choose_res();

extern struct hash_member  *reslist;
extern struct hash_member  *bindlist;
extern struct hash_tbl     cidhashtable;
extern struct hash_tbl     iphashtable;
extern struct hash_tbl     relayhashtable;

extern int bindcidcmp();
extern int resipcmp();
extern int relayipcmp();
extern int open_if();

static int      discover();
static int      request();
static int      decline();
static int      release();
static int      inform();

#ifndef NOBOOTP
static void construct_bootp();
static int available_forbootp();
static int send_bootp();
static struct dhcp_resource *choose_forbootp();
static int      bootp();
#endif /* NOBOOTP */


static int (*process_msg[])() = {
#ifdef NOBOOTP
  NULL,
#else
  bootp,
#endif
  discover,
  NULL,
  request,
  decline,
  NULL,
  NULL,
  release,
  inform
};

static int  ins_ip();
static int  ins_ips();
static int  ins_ippairs();
static int  ins_long();
static int  ins_short();
static int  ins_octet();
static int  ins_str();

#ifndef CONFIG_RTL_819X
static int  ins_mtpt();
#endif

static int  ins_dht();

void delarp();

static int (*ins_opt[])() = {
  NULL,         /* PAD == 0 */
  ins_ip,       /* SUBNET_MASK */
  ins_long,     /* TIME_OFFSET */
  ins_ips,      /* ROUTER */
  ins_ips,      /* TIME_SERVER */
  ins_ips,      /* NAME_SERVER */
  ins_ips,      /* DNS_SERVER */
  ins_ips,      /* LOG_SERVER */
  ins_ips,      /* COOKIE_SERVER */
  ins_ips,      /* LPR_SERVER */
  ins_ips,      /* IMPRESS_SERVER */
  ins_ips,      /* RLS_SERVER */
  ins_str,      /* HOSTNAME */
  ins_short,    /* BOOTSIZE */
  ins_str,      /* MERIT_DUMP */
  ins_str,      /* DNS_DOMAIN */
  ins_ip,       /* SWAP_SERVER */
  ins_str,      /* ROOT_PATH */
  ins_str,      /* EXTENSIONS_PATH */
  ins_octet,    /* IP_FORWARD */
  ins_octet,    /* NONLOCAL_SRCROUTE */
  ins_ippairs,  /* POLICY_FILTER */
  ins_short,    /* MAX_DGRAM_SIZE */
  ins_octet,    /* DEFAULT_IP_TTL */
  ins_long,     /* MTU_AGING_TIMEOUT */
  
#ifndef CONFIG_RTL_819X
  ins_mtpt,     /* MTU_PLATEAU_TABLE */
#else
  NULL,
#endif

  ins_short,    /* IF_MTU */
  ins_octet,    /* ALL_SUBNET_LOCAL */
  ins_ip,       /* BRDCAST_ADDR */
  ins_octet,    /* MASK_DISCOVER */
  ins_octet,    /* MASK_SUPPLIER */
  ins_octet,    /* ROUTER_DISCOVER */
  ins_ip,       /* ROUTER_SOLICIT */
  ins_ippairs,  /* STATIC_ROUTE */
  ins_octet,    /* TRAILER */
  ins_long,     /* ARP_CACHE_TIMEOUT */
  ins_octet,    /* ETHER_ENCAP */
  ins_octet,    /* DEFAULT_TCP_TTL */
  ins_long,     /* KEEPALIVE_INTER */
  ins_octet,    /* KEEPALIVE_GARBA */
  ins_str,      /* NIS_DOMAIN */
  ins_ips,      /* NIS_SERVER */
  ins_ips,      /* NTP_SERVER */
  NULL,         /* VENDOR_SPEC */
  ins_ips,      /* NBN_SERVER */
  ins_ips,      /* NBDD_SERVER */
  ins_octet,    /* NB_NODETYPE */
  ins_str,      /* NB_SCOPE */
  ins_ips,      /* XFONT_SERVER */
  ins_ips,      /* XDISPLAY_MANAGER */
  NULL,         /* REQUEST_IPADDR */
  NULL,         /* LEASE_TIME */
  NULL,         /* OPT_OVERLOAD */
  NULL,         /* DHCP_MSGTYPE */
  NULL,         /* SERVER_ID */
  NULL,         /* REQ_LIST */
  NULL,         /* DHCP_ERRMSG */
  NULL,         /* DHCP_MAXMSGSIZE */
  ins_dht,      /* DHCP_T1 */
  ins_dht,      /* DHCP_T2 */
  NULL,         /* CLASS_ID */
  NULL,         /* CLIENT_ID */
  NULL,
  NULL,
  ins_str,      /* NISP_DOMAIN */
  ins_ips,      /* NISP_SERVER */
  NULL,         /* TFTP_SERVERNAME */
  NULL,         /* BOOTFILE */
  ins_ips,      /* MOBILEIP_HA */
  ins_ips,      /* SMTP_SERVER */
  ins_ips,      /* POP3_SERVER */
  ins_ips,      /* NNTP_SERVER */
  ins_ips,      /* DFLT_WWW_SERVER */
  ins_ips,      /* DFLT_FINGER_SERVER */
  ins_ips,      /* DFLT_IRC_SERVER */
  ins_ips,      /* STREETTALK_SERVER */
  ins_ips       /* STDA_SERVER */
};
