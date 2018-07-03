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

#ifdef __alpha
#define    Long    int
#define  u_Long  u_int
#else
#define    Long   long
#define  u_Long u_long
#endif

#define MAX_HLEN      16    /* maximum length of haddr field */
#define MAX_SNAME     64
#define MAX_FILE     128
#define DFLTOPTLEN   312
#define BOOTPOPTLEN   64
#define MAXOPT      0xff
#define INFINITY    0xffffffff

/*
 * DHCP message format
 */
struct dhcp {
  unsigned char	  op;	                /* packet type */
  unsigned char	  htype;	        /* hardware address type */
  unsigned char	  hlen;	                /* hardware address length */
  unsigned char	  hops;	                /* gateway hops */
  unsigned Long	  xid;	                /* transaction ID */
  unsigned short  secs;	                /* seconds since boot began */
  unsigned short  flags;
#define ISBRDCST(X)   ((X & htons(0x8000)) != 0)
#define SETBRDCST(X)  ((X) |= htons(0x8000))
  struct in_addr  ciaddr;               /* client IP address */
  struct in_addr  yiaddr;               /* 'your' IP address */
  struct in_addr  siaddr;               /* server IP address */
  struct in_addr  giaddr;               /* relay agent IP address */
  char            chaddr[MAX_HLEN];     /* client hardware address */
  char            sname[MAX_SNAME];     /* server host name */
  char            file[MAX_FILE];       /* boot file name */
  char            options[DFLTOPTLEN];  /* optional parameters field */
};

#define DFLTDHCPLEN   sizeof(struct dhcp)        /* default DHCP msg size */
#define MINDHCPLEN    (DFLTDHCPLEN - DFLTOPTLEN + MAGIC_LEN)
                                                 /* min DHCP msg size */
#define DFLTBOOTPLEN  (DFLTDHCPLEN - DFLTOPTLEN + BOOTPOPTLEN)

/*
 * UDP port numbers, server and client.
 */
#define	DHCPS_PORT		67
#define	DHCPC_PORT		68

#define BOOTREQUEST		1
#define BOOTREPLY		2


/*
 * magic cookie for RFC1048
 */
#define RFC1048_MAGIC { 99, 130, 83, 99 }
#define MAGIC_LEN  4

/*
 * tag values used to specify what information is being supplied in
 * the options field of the packet.
 */
#define PAD		     ((char)   0)
#define SUBNET_MASK	     ((char)   1)
#define TIME_OFFSET	     ((char)   2)
#define ROUTER		     ((char)   3)
#define TIME_SERVER	     ((char)   4)
#define NAME_SERVER	     ((char)   5)
#define DNS_SERVER           ((char)   6)
#define LOG_SERVER	     ((char)   7)
#define COOKIE_SERVER	     ((char)   8)
#define LPR_SERVER	     ((char)   9)
#define IMPRESS_SERVER	     ((char)  10)
#define RLS_SERVER 	     ((char)  11)
#define HOSTNAME	     ((char)  12)
#define BOOTSIZE	     ((char)  13)
#define MERIT_DUMP           ((char)  14)
#define DNS_DOMAIN           ((char)  15)
#define SWAP_SERVER          ((char)  16)
#define ROOT_PATH            ((char)  17)
#define EXTENSIONS_PATH      ((char)  18)
#define IP_FORWARD           ((char)  19)
#define NONLOCAL_SRCROUTE    ((char)  20)
#define POLICY_FILTER        ((char)  21)
#define MAX_DGRAM_SIZE       ((char)  22)
#define DEFAULT_IP_TTL       ((char)  23)
#define MTU_AGING_TIMEOUT    ((char)  24)
#define MTU_PLATEAU_TABLE    ((char)  25)
#define IF_MTU               ((char)  26)
#define ALL_SUBNET_LOCAL     ((char)  27)
#define BRDCAST_ADDR         ((char)  28)
#define MASK_DISCOVER        ((char)  29)
#define MASK_SUPPLIER        ((char)  30)
#define ROUTER_DISCOVER      ((char)  31)
#define ROUTER_SOLICIT       ((char)  32)
#define STATIC_ROUTE         ((char)  33)
#define TRAILER              ((char)  34)
#define ARP_CACHE_TIMEOUT    ((char)  35)
#define ETHER_ENCAP          ((char)  36)
#define DEFAULT_TCP_TTL      ((char)  37)
#define KEEPALIVE_INTER      ((char)  38)
#define KEEPALIVE_GARBA      ((char)  39)
#define NIS_DOMAIN           ((char)  40)
#define NIS_SERVER           ((char)  41)
#define NTP_SERVER           ((char)  42)
#define VENDOR_SPEC          ((char)  43)
#define NBN_SERVER           ((char)  44)
#define NBDD_SERVER          ((char)  45)
#define NB_NODETYPE          ((char)  46)
#define NB_SCOPE             ((char)  47)
#define XFONT_SERVER         ((char)  48)
#define XDISPLAY_MANAGER     ((char)  49)
#define REQUEST_IPADDR       ((char)  50)
#define LEASE_TIME           ((char)  51)
#define OPT_OVERLOAD         ((char)  52)
#define DHCP_MSGTYPE         ((char)  53)
#define SERVER_ID            ((char)  54)
#define REQ_LIST             ((char)  55)
#define DHCP_ERRMSG          ((char)  56)
#define DHCP_MAXMSGSIZE      ((char)  57)
#define DHCP_T1              ((char)  58) 
#define DHCP_T2              ((char)  59)
#define CLASS_ID             ((char)  60)
#define CLIENT_ID            ((char)  61)
#define NISP_DOMAIN          ((char)  64)
#define NISP_SERVER          ((char)  65)
#define TFTP_SERVERNAME      ((char)  66)
#define BOOTFILE             ((char)  67)
#define MOBILEIP_HA          ((char)  68)
#define SMTP_SERVER          ((char)  69)
#define POP3_SERVER          ((char)  70)
#define NNTP_SERVER          ((char)  71)
#define DFLT_WWW_SERVER      ((char)  72)
#define DFLT_FINGER_SERVER   ((char)  73)
#define DFLT_IRC_SERVER      ((char)  74)
#define STREETTALK_SERVER    ((char)  75)
#define STDA_SERVER          ((char)  76)
#define END		     ((char) 255)

#define LAST_OPTION          STDA_SERVER

/* DHCP Message Type */
#define DHCPDISCOVER           1
#define DHCPOFFER              2
#define DHCPREQUEST            3
#define DHCPDECLINE            4
#define DHCPACK                5
#define DHCPNAK                6
#define DHCPRELEASE            7
#define DHCPINFORM          8
#define BOOTP                  0

#define FILE_ISOPT             1
#define SNAME_ISOPT            2
#define BOTH_AREOPT            FILE_ISOPT + SNAME_ISOPT

/* Hardware type from arp section of the 'assigned number' */

#define ETHER		1	/* Ethernet (10Mb) */
#define EXPETHER	2	/* Experimental Ethernet (3Mb) */
#define AX25		3	/* Amateur Radio AX.25 */
#define PRONET		4	/* Proteon ProNET Token Ring */
#define CHAOS		5	/* Chaos */
#define IEEE802		6	/* IEEE 802 Networks */
#define ARCNET		7	/* ARCNET */
#define HYPERCH		8	/* Hyperchannel */
#define LANSTAR		9	/* Lanstar */
#define AUTONET		10	/* Autonet Short Address */
#define LOCALTALK	11	/* LocalTalk */
#define LOCALNET        12      /* LocalNet */
#define DNS_NAME        128     /* DNS name */

#define DHCP_THREAD_PRIORITY 16
#define DHCP_THREAD_STACK_SIZE 0x00004000
