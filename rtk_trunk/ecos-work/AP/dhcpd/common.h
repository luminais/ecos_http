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

#include <sys/param.h>

#ifdef ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif

#define MAXPATHLEN 128

#define IF_BUFSIZE 1024

#define NBBY 8

#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	       (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)


#define S_TABLE_CONT          0
#define S_SNAME               1
#define S_FILE                2
#define S_SIADDR              3
#define S_ALLOW_BOOTP         4
#define S_IP_ADDR             5
#define S_MAX_LEASE           6
#define S_DEFAULT_LEASE       7
#define S_CLIENT_ID           8
#define S_SUBNET_MASK         9
#define S_TIME_OFFSET        10
#define S_ROUTER             11
#define S_TIME_SERVER        12
#define S_NAME_SERVER        13
#define S_DNS_SERVER         14
#define S_LOG_SERVER         15
#define S_COOKIE_SERVER      16
#define S_LPR_SERVER         17
#define S_IMPRESS_SERVER     18
#define S_RLS_SERVER         19
#define S_HOSTNAME           20
#define S_BOOTSIZE           21
#define S_MERIT_DUMP         22
#define S_DNS_DOMAIN         23
#define S_SWAP_SERVER        24
#define S_ROOT_PATH          25
#define S_EXTENSIONS_PATH    26
#define S_IP_FORWARD         27
#define S_NONLOCAL_SRCROUTE  28
#define S_POLICY_FILTER      29
#define S_MAX_DGRAM_SIZE     30
#define S_DEFAULT_IP_TTL     31
#define S_MTU_AGING_TIMEOUT  32
#define S_MTU_PLATEAU_TABLE  33
#define S_IF_MTU             34
#define S_ALL_SUBNET_LOCAL   35
#define S_BRDCAST_ADDR       36
#define S_MASK_DISCOVER      37
#define S_MASK_SUPPLIER      38
#define S_ROUTER_DISCOVER    39
#define S_ROUTER_SOLICIT     40
#define S_STATIC_ROUTE       41
#define S_TRAILER            42
#define S_ARP_CACHE_TIMEOUT  43
#define S_ETHER_ENCAP        44
#define S_DEFAULT_TCP_TTL    45
#define S_KEEPALIVE_INTER    46
#define S_KEEPALIVE_GARBA    47
#define S_NIS_DOMAIN         48
#define S_NIS_SERVER         49
#define S_NTP_SERVER         50
#define S_NBN_SERVER         51
#define S_NBDD_SERVER        52
#define S_NB_NODETYPE        53
#define S_NB_SCOPE           54
#define S_XFONT_SERVER       55
#define S_XDISPLAY_MANAGER   56
#define S_DHCP_T1            57
#define S_DHCP_T2            58
#define S_NISP_DOMAIN        59
#define S_NISP_SERVER        60
#define S_MOBILEIP_HA        61
#define S_SMTP_SERVER        62
#define S_POP3_SERVER        63
#define S_NNTP_SERVER        64
#define S_DFLT_WWW_SERVER    65
#define S_DFLT_FINGER_SERVER 66
#define S_DFLT_IRC_SERVER    67
#define S_STREETTALK_SERVER  68
#define S_STDA_SERVER        69

#define S_LAST_OPTION        S_STDA_SERVER
#define VALIDSIZE   ((S_LAST_OPTION / 8) + 1)

struct msg
{
  struct ether_header *ether;
  struct ip *ip;
  struct udphdr *udp;
  struct dhcp *dhcp;
};

struct if_info {
  int fd;
  char name[sizeof "enxx"];
  unsigned int htype;
  unsigned char hlen;
  char haddr[6];
  struct in_addr *ipaddr;
  struct in_addr *subnetmask;
  int buf_size;
  char *buf;
  struct if_info *next;				/* Null terminate */
};

struct relay_acl {
  struct in_addr relay,
                 subnet_mask;
};

struct client_id {
  unsigned char   idtype,
                  idlen;
   char            *id;
  struct in_addr  subnet;
};

struct u_shorts {
  int num;
  unsigned short *shorts;
};

struct in_addrs {
  int num;
  struct in_addr *addr;
};

struct ip_pairs {
  int num;
  struct in_addr *addr1,
                 *addr2;
};

struct chaddr {
  unsigned char  htype,
                 hlen;
  char           *haddr;
};

struct dhcp_binding {

  /* external info */
  struct client_id           cid;           /* hashing key. client identifier */
  struct chaddr              haddr;
  time_t                     expire_epoch;  /* lease expiration epoch */
  char                       *res_name;     /* name of corresponding resource */

  /* internal info */
  char                       flag;
#define COMPLETE_ENTRY       0x1         /* complete entry */
#define STATIC_ENTRY         0x2         /* static entry */
#define BOOTP_ENTRY          0x4         /* bootp entry */
  time_t                     temp_epoch;        /* memo expiration epoch */
  struct dhcp_resource       *res;              /* pointer to resource */
};


struct dhcp_resource {
  /* external info */
  char                      allow_bootp,         /* boolean */
                            ip_forward,          /* boolean */
                            
#ifndef CONFIG_RTL_819X 
                            nonlocal_srcroute,   /* boolean */
                            all_subnet_local,    /* boolean */
                            mask_discover,       /* boolean */
                            mask_supplier,       /* boolean */
                            router_discover,     /* boolean */
                            trailer,             /* boolean */
                            ether_encap,         /* boolean */
                            keepalive_garba,     /* boolean */
#endif

                            *entryname,          /* tag name of this entry */
                            *sname,              /* server host name */
                            *file,               /* boot file name */
                            *hostname,
                            
#ifndef CONFIG_RTL_819X
                            *merit_dump,
#endif
                            *dns_domain;
                            
#ifndef CONFIG_RTL_819X
                            *root_path,
                            *extensions_path,
                            *nis_domain,
                            *nb_scope,           /* NetBIOS scope */
                            *nisp_domain;        /* NIS+ domain name */
#endif

  unsigned char             default_ip_ttl,      /* time to live (1 - 255) */
                            default_tcp_ttl;

 #ifndef CONFIG_RTL_819X
                            nb_nodetype;         /* NetBIOS node type */
 #endif

#define BNODE   0x1
#define PNODE   0x2
#define MNODE   0x4
#define HNODE   0x8
  unsigned short            bootsize,            /* bootfile size in 16 bits */
                            max_dgram_size,      /* max dgram reassembly */
                            intf_mtu,            /* interface mtu */
                            dhcp_t1,             /* host byte order */
                            dhcp_t2;             /* host byte order */
  Long                      time_offset;         /* offset from UTC */
  unsigned Long             max_lease,           /* maximum lease duration
						    (host byte order) */
                            default_lease;       /* default lease duration
						    (host byte order) */
#ifndef CONFIG_RTL_819X
                            mtu_aging_timeout,   /* unsigned 32 bits integer */
                            arp_cache_timeout,
                            keepalive_inter;

  struct u_shorts           *mtu_plateau_table;  /* array of 16 bits int */
#endif

  struct in_addr            *siaddr,             /* (tftp) server */
                            *ip_addr,            /* IP address */
                            *subnet_mask,
                            
#ifndef CONFIG_RTL_819X
                            *swap_server,
#endif
                            *brdcast_addr;

#ifndef CONFIG_RTL_819X
                            *router_solicit;
#endif

  struct in_addrs           *router,
                            *time_server,
                            *name_server,
                            *dns_server;       /* DNS server */
                            
#ifndef CONFIG_RTL_819X
                            *log_server,
                            *cookie_server,
                            *lpr_server,
                            *impress_server,
                            *rls_server,        /* Resource Location Server */
                            *nis_server,
                            *ntp_server,
                            *nbn_server,        /* NetBIOS name server */
                            *nbdd_server,       /* NetBIOS dgram distrib server */
                            *xfont_server,      /* X font erver */
                            *xdisplay_manager,  /* X display manager */
                            *nisp_server,       /* NIS+ server */
                            *mobileip_ha,       /* Mobile IP Home Agent */
                            *smtp_server,       /* SMTP server */
                            *pop3_server,       /* POP3 server */
                            *nntp_server,       /* NNTP server */
                            *dflt_www_server,   /* Default WWW server */
                            *dflt_finger_server,/* Default Finger server */
                            *dflt_irc_server,   /* Default IRC server */
                            *streettalk_server, /* StreetTalk server */
                            *stda_server;       /* STDA server */
#endif

  struct ip_pairs           *policy_filter,   
                            *static_route;

  /* internal info */
  char                      valid[VALIDSIZE],  /* indicate which are available */
                            active[VALIDSIZE]; /* indicate which differ from
						  Host Requirements RFC */
  struct dhcp_binding       *binding;            /* pointer to dhcp_binding */
};

#ifndef TRUE
#define TRUE   1
#endif
#ifndef FALSE
#define FALSE  0
#endif
#ifndef GOOD
#define GOOD   1
#endif
#ifndef BAD
#define BAD    0
#endif
#ifndef NBPFILTER
#define NBPFILTER 10
#endif
#ifndef ADDRPOOL_DUMP
#define ADDRPOOL_DUMP           "/tmp/addrpool_dump"
#endif
#define MAX_NBIND       50000   /* maximum number of dhcp_binding
				 ( this is used in garbage_collect() ) */

#ifdef SUPPORT_MAC_FILTER
#define MAX_CLIENTS     500     /* maximum number of clients which registered
				   in dhcpdb.MAC database */
#endif

#define	QWOFF    2                 /* to make dhcp long to 4w alignment */
#define ETHERHL  sizeof(struct ether_header)  /* ethernet header length */
#define IPHL     sizeof(struct ip)            /* IP header length */
#define UDPHL    sizeof(struct udphdr)        /* UDP header length */
#define WORD     4                            /* word alignment in bits */

extern FILE *addrpool_dbfp;
extern FILE *binding_dbfp;
extern FILE *relay_dbfp;

#ifdef SUPPORT_MAC_FILTER
extern FILE *mac_dbfp;
#endif

extern FILE *dump_fp;
extern char binding_db[MAXPATHLEN];
extern char addrpool_db[MAXPATHLEN];
extern char relay_db[MAXPATHLEN];

#ifdef SUPPORT_MAC_FILTER
extern char mac_db[MAXPATHLEN];
#endif
extern int nbind;
extern u_short dhcps_port;     /* DHCP server port (network byte order) */
extern u_short dhcpc_port;     /* DHCP server port (network byte order) */
extern char **argv_orig;

#ifndef sun
extern struct bpf_hdr *rbpf;        /* pointer to bpf header */
#endif
extern struct msg rcv;

int     add_bind();
void    read_bind_db();
void    read_addrpool_db();
void    read_relay_db();

#ifdef SUPPORT_MAC_FILTER
void    read_mac_db();
#endif

void    dump_bind_db();
void    dump_addrpool_db();
void    finish();
void    restart();
char    *haddrtos();
char    *cidtos();
char    *_ctime();



