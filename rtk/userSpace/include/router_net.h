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
    cfg_net.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
*/
#ifndef CFG_IF_H
#define CFG_IF_H

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
//#include <network.h>
#include <sys/types.h>

#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>

//==============================================================================
//                                    MACROS
//==============================================================================
#define NET_ERR(level, msg)	\
{	diag_printf(msg); \
	return false; \
}

// mode state
#define NODEFMODE		0
#define STATICMODE		1
#define DHCPMODE		2
#define PPPOEMODE		3
#define PPTPMODE		4
#define L2TPMODE		5
#define PPPOEMODE2		9
//roy +++
#define C8021XMODE		6
//roy ---
//#define DHCPPXYMODE		6
#define DUALWAN_PPPOEMODE	7
#define DUALWAN_PPTP		8
#define PPTPMODE2		10

enum
{
    PPP_NO_ETH = 0,
    PPP_WAIT_ETH,
    PPP_CHK_SRV,
    PPP_GET_ETH,
    PPP_IN_BEGIN,
    PPP_IN_ACTIVE
};
//roy +++
enum{
	PPPOE_AUTO = 0,
	PPPOE_TRAFFIC,
	PPPOE_BY_HAND,
	PPPOE_BY_TIME
};//pppoe connect mode
//roy ---
#define DISCONNECTED	0
#define CONNECTED		1
#define CONNECTING		2

#define ROUTE_ADD	0
#define ROUTE_DEL	1

//#define MAX_DNS_NUM		3

#define DOMAINNAME_LEN	256
//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================
struct ip_set{
	char	ifname[IFNAMSIZ];			//	which interface to bind
	unsigned int mtu;			// interface MTU
	unsigned int ip;			//	IP address
	unsigned int netmask;		//	netmask
    unsigned int gw_ip;			//	gateway address
    unsigned int server_ip;		//	server address
    unsigned int broad_ip;		//	broadcast address
    unsigned char mode; 		//	which mode to get IP, ex static, dhcp, pppoe...
    unsigned char up;			//	connected/disconnectd
    unsigned short flags;
    unsigned int tunnel_gw;		//	tunnel gateway address
    unsigned int tunnel_mode;
    cyg_tick_count_t 	conn_time;    //connection time
    unsigned char domain[DOMAINNAME_LEN]; //domain name
    int               time_correct;		//     system time have been correct
    void *data;					//	point to other related data
};

#define IPSET_FG_IFCLOSED	0x0001
#define IPSET_FG_DEFROUTE	0x0020
#define IPSET_FG_NATIF		0x0040
#define IPSET_FG_FWIF		0x0080

#define IPSET_FG_IFCONFIG	0x0100
#define IPSET_FG_NATCONFIG	0x0400
#define IPSET_FG_FWCONFIG	0x0800

extern char lan_name[IFNAMSIZ];
extern struct ip_set primary_lan_ip_set;
#define LAN_NAME			lan_name

#define SYS_lan_if			(primary_lan_ip_set.ifname)
#define SYS_lan_ip			(primary_lan_ip_set.ip)
#define SYS_lan_mask		(primary_lan_ip_set.netmask)
#define SYS_lan_gw			(primary_lan_ip_set.gw_ip)
#define SYS_lan_brstip		(primary_lan_ip_set.broad_ip)
#define SYS_lan_mode		(primary_lan_ip_set.mode)
#define SYS_lan_up			(primary_lan_ip_set.up)

// WAN status
extern char wan_name[IFNAMSIZ];
extern struct ip_set primary_wan_ip_set;
#define WAN_NAME			wan_name

extern char wan_dns1[20];
extern char wan_dns2[20];

#define SYS_dns_1			wan_dns1
#define SYS_dns_2			wan_dns2
		
#define SYS_wan_if			(primary_wan_ip_set.ifname)		
#define SYS_wan_ip			(primary_wan_ip_set.ip)
#define SYS_wan_mask		(primary_wan_ip_set.netmask)
#define SYS_wan_gw			(primary_wan_ip_set.gw_ip)
#define SYS_wan_brstip		(primary_wan_ip_set.broad_ip)
#define SYS_wan_mode		(primary_wan_ip_set.mode)
#define SYS_wan_up			(primary_wan_ip_set.up)
#define SYS_wan_data		(primary_wan_ip_set.data)
#define SYS_wan_domain		(primary_wan_ip_set.domain)
#define SYS_wan_conntime    (primary_wan_ip_set.conn_time)
#define SYS_wan_mtu			(primary_wan_ip_set.mtu)
#define SYS_wan_serverip	(primary_wan_ip_set.server_ip)

#define NSTR(x)			inet_ntoa(*(struct in_addr *)&x)

#endif // CFG_IF_H

