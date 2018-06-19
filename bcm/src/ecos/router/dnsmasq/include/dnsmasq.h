/*
 * DNSMASQ server control structure defintions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dnsmasq.h,v 1.4 2010-07-26 01:44:41 Exp $
 */

#ifndef	__DNSMASQ_H__
#define	__DNSMASQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(linux)
#include <dnsmasq_linux_osl.h>
#elif defined(__ECOS)
#include <dnsmasq_ecos_osl.h>
#else
#error "Unsupported OSL requested"
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/nameser.h>

#define MAXHOSTSENTRY	__CONFIG_DNSMASQ_HOSTS_NUM__
#define MAXCOUNT		150			/* max number of outstanding requests */
#define QUERY_TIMEOUT	40			/* drop queries after TIMEOUT seconds */
#define QUICK_TIMEOUT	5

#define PACKETSZ		512			/* maximum packet size */
#define RRFIXEDSZ		10			/* #/bytes of fixed data in r record */

#define FLAG_NOERR		0x0
#define FLAG_NEG		0x1
#define FLAG_IPV4		0x2
#define FLAG_QUERY		0x4

#define UNKNOW_QUERY		0x0
#define GOOD_QUERY		0x1
#define UNKNOW_TYPE		0x2

typedef HEADER DNSHEADER;

typedef struct dnsmasq_interface	DNSMASQ_INTERFACE;
typedef struct dnsmasq_server		DNSMASQ_SERVER;
typedef struct dnsmasq_host		DNSMASQ_HOST;
typedef struct dnsmasq_entry		DNSMASQ_ENTRY;
typedef struct dnsmasq_container	DNSMASQ_CONTAINER;
typedef struct dnsmasq_context		DNSMASQ_CONTEXT;

struct dnsmasq_host {
	DNSMASQ_HOST *next;
	struct in_addr	addr;
	char name[MAXDNAME];
};

struct dnsmasq_server {
	DNSMASQ_SERVER *next;
	struct in_addr	addr;
	int fd;
};

struct dnsmasq_interface {
	DNSMASQ_INTERFACE *next;
	char ifname[IFNAMSIZ];		/* interface name */
	int unit; 			/* interface instance */
	int pkt_sock;			/* packet socket */
	struct in_addr ipaddr;
};

struct dnsmasq_entry {
	DNSMASQ_ENTRY *next;
	DNSMASQ_SERVER *forwardto;
	struct sockaddr_in queryaddr;
	time_t expire;
	int fd;
	int flag;
	char dnsname[MAXDNAME];
	unsigned short oid;
	unsigned short nid;
	struct in_addr answer_addr;
};

/* export functions */
void dnsmasq_process_reply(DNSHEADER *dnsheader, int plen, int fd);
void dnsmasq_process_query(DNSHEADER *dnsheader, int plen, int fd,
	struct in_addr ipaddr, struct sockaddr_in *queryaddr);

DNSMASQ_ENTRY *dnsmasq_cache_find(char *dnsname);
void dnsmasq_cache_add(DNSMASQ_ENTRY *entry);

DNSMASQ_ENTRY *dnsmasq_forward_find_by_addr(struct in_addr addr, char *dnsname);
DNSMASQ_ENTRY *dnsmasq_forward_find_by_nid(unsigned short nid);
void dnsmasq_forward_add(DNSMASQ_ENTRY *entry);
void dnsmasq_forward_del(DNSMASQ_ENTRY *entry);

DNSMASQ_ENTRY *dnsmasq_new(void);
void dnsmasq_free(DNSMASQ_ENTRY *entry);

char *dnsmasq_get_localhost(void);
DNSMASQ_HOST *dnsmasq_host_find(char *dnsname);

DNSMASQ_SERVER *dnsmasq_server_next(DNSMASQ_SERVER *current);
DNSMASQ_SERVER *dnsmasq_get_last_reply_server(void);
void dnsmasq_set_last_reply_server(DNSMASQ_SERVER *server);

/*
 * Functions called by OSL main
 */
void dnsmasq_mainloop(void);
void dnsmasq_terminate(void);
void dnsmasq_restart(void);

/*
 * External OSL functions declaration
 */
extern int  dnsmasq_osl_ifname_list(char *ifname_list);
extern int  dnsmasq_osl_dns_list(char *buf);
extern int  dnsmasq_osl_host_list(char *hostlist);
extern void dnsmasq_osl_localhost_init(char *localhost);
extern int  dnsmasq_osl_ifaddr(char *ifname, struct in_addr *ipaddr);

extern int open_udp_socket(char *ipaddr, unsigned short port);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __DNSMASQ_H__ */
