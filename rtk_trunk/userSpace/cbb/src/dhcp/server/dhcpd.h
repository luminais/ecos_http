/*
 * DHCP server control structure defintions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd.h,v 1.9 2010-07-04 03:36:40 Exp $
 */

#ifndef __DHCPD_H__
#define __DHCPD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dhcp_var.h>
#include <dhcp_option.h>

#include <dhcpd_osl.h>
#include <sys/queue.h>
#include "tenda_arp.h"
/*
 * Forward definition
 */
typedef struct dhcpd_ifc	DHCPD_IFC;
typedef	struct dhcpd_lease	DHCPD_LEASE;
typedef	struct lease_container	LEASE_CONTAINER;

/* DHCP server configuration */
#define DEFAULT_START		"192.168.0.100"
#define DEFAULT_END		"192.168.0.200"
#define DEFAULT_LEASE_TIME	3600
#define DEFAULT_LEASE_MIN_TIME	60
#define DEFAULT_RESERVE_TIME	3600

#define DYNAMICL		0x0
#define STATICL			0x1
//roy +++2010/09/03
#define DISABLED				0x2
#define DHCPD_STATIC_LEASE_NU 19
//roy+++
#define RESERVED		0x4

#define MAX_NO_BRIDGE (1)  // added for dhcp server, by zhuhuan on 2016.01.08

/* For flat dump */
struct lease_t {
	unsigned char	last;
	unsigned char	flag;
	unsigned char	mac[6];
	struct in_addr	ipaddr;
	unsigned int	expiry;
	unsigned char	hostname[64];
};

struct dhcpd_lease {
	CIRCLEQ_ENTRY(dhcpd_lease) lease_queue;
	unsigned char	flag;
	unsigned char	mac[6];
	struct in_addr	ipaddr;
	unsigned int	expiry;
	unsigned char	*hostname;
};

CIRCLEQ_HEAD(lease_head, dhcpd_lease);

struct lease_pool {
	struct in_addr start;
	struct in_addr end;
};

#define	DHCPD_POOL_NUM		1
struct lease_container {
	unsigned long lease_time;
	unsigned long lease_min;
	unsigned long reserve_time;
	unsigned long offer_time;

	int pool_num;
	struct lease_pool pool[DHCPD_POOL_NUM];

	struct lease_head inuse;
	struct lease_head free;
	DHCPD_LEASE *lease_array;
	DHCPD_LEASE *lineup[256];	/* Only works for C class ip address */
};

#define	DHCPD_IPC_PORT		6767
#define DHCPD_DEVNAME		"/dev/net/dhcpd"

struct dhcpd_ifc {
	DHCPD_IFC *next;

	char ifname[20]; 	/* interface name */
	int unit; 		/* interface instance */
	int fd;			/* dhcpd device */
	struct in_addr ipaddr;
	struct in_addr netmask;

	/* TX RX buffer */
	struct dhcphdr sndpkt;

	/* buffer to process client options */
	unsigned char	coptions[512];

	/* Pointer to hold client options */
	struct in_addr	*cserver;
	struct in_addr	*creqip;
	unsigned char	*chostname;

	unsigned long	clease_time;
	unsigned char	cstate;
	unsigned long	cxid;
	unsigned short	cflags;
	struct in_addr	ciaddr;
	struct in_addr	cyiaddr;
	struct in_addr	cgiaddr;
	unsigned char	chaddr[16];

	/* Lease container */
	LEASE_CONTAINER	container;
};

extern DHCPD_IFC *dhcpd_iflist;

struct ip_hn {
	unsigned char	hostname[64];
	struct in_addr	ipaddr;
	unsigned char 	mac[6];
	int iptype;		//0-->dhcp 1-->static
};
struct machost_t{
		in_addr_t ip;
        unsigned char mac[6];
		char hostname[64];
		struct machost_t *next
};

void cut_filterhostname_head(char *in,char *out);
void cut_filterhostname_tail(char *in,char *out);
//int count_ip_entries();
void isinMacfilter();
void printf_ip_entries();
void clearHostnameFilter();
extern	int check_oneip(char *str1);
extern	char *tenda_arp_inet_ntoa(in_addr_t ina);
extern void tenda_arp_get_all(struct detec *buf);
extern void tenda_arp_set_hostname(struct machost_t *buf);


/* lease functions */
void dhcpd_lease_expiry_renew(DHCPD_IFC *ifp, DHCPD_LEASE *lease, unsigned int new_expiry);
void dhcpd_lease_free(DHCPD_IFC *ifp, DHCPD_LEASE *lease);

DHCPD_LEASE *dhcpd_lease_match(DHCPD_IFC *ifp, unsigned char *mac, struct in_addr *ipaddr);
DHCPD_LEASE *dhcpd_lease_add(DHCPD_IFC *ifp, unsigned char *chaddr, struct in_addr yiaddr,
	unsigned long lease, unsigned char *hostname, int flag);
struct in_addr *dhcpd_addr_alloc(DHCPD_IFC *ifp, struct in_addr *addr);
int dhcpd_probe(DHCPD_IFC *ifp, DHCPD_LEASE *lease, struct in_addr ipaddr, unsigned char *chaddr);

#ifdef __CONFIG_AUTO_CONN__
int dhcpd_probe_extend(DHCPD_IFC *ifp, DHCPD_LEASE *lease, struct in_addr ipaddr, unsigned char *chaddr);

#endif

void dhcpd_lease_init(DHCPD_IFC *ifp);
void dhcpd_lease_deinit(DHCPD_IFC *ifp);
struct lease_t *dhcpd_lease_copy(char *ifname);

/*
 * Functions called by OSL main
 */
void dhcpd_mainloop(void);

/*
 * External OSL functions declaration
 */
extern	void dhcpd_osl_arpdel(struct in_addr ipaddr);
extern	void dhcpd_osl_arpget(struct in_addr ipaddr, unsigned char *testchaddr);

extern	int dhcpd_osl_gateway_list(DHCPD_IFC *ifp, char *buf);
extern	int dhcpd_osl_get_domain(DHCPD_IFC *ifp, char *buf);
extern  int dhcpd_osl_dns_list(DHCPD_IFC *ifp, char *buf);
//roy +++2010/09/03
extern	int dhcpd_osl_static_lease(DHCPD_IFC *ifp, char *buf,int index);
extern 	int dhcpd_osl_lan_ip(DHCPD_IFC *ifp, char *buf);
//roy +++

extern	int dhcpd_osl_init_config(DHCPD_IFC *ifp);
extern	int dhcpd_osl_ifname_list(char *ifname_list);

extern int open_udp_socket(char *ipaddr, unsigned short port);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __DHCPD_H__ */
