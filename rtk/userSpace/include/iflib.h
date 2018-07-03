/*
 * eCos interface ioctl function prototypes
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: iflib.h,v 1.1.1.1 2010-04-09 10:37:02 Exp $
 */

#ifndef __IFLIB_H__
#define __IFLIB_H__

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <unistd.h>
#include <ifaddrs.h>
//roy +++
#include <router_net.h>//for struct ip_set
//+++
struct ifaddrs *readifaddrs(char *ifname, struct ifaddrs *ifp);
int read_interface(char *ifname, unsigned long *addr, unsigned long *mask, unsigned char *arp);

int iflib_ioctl(char *ifname, int cmd, void *data);
int iflib_flushifip(char *ifname);
int iflib_getifhwaddr(char *ifname, unsigned char ifmac[6]);
int iflib_setifhwaddr(char *ifname, unsigned char ifmac[6]);
int iflib_getifaddr(char *ifname, struct in_addr *ipaddr, struct in_addr *netmask);
int iflib_setifaddr(char *ifname, struct in_addr ipaddr, struct in_addr netmask);
int iflib_setaliasaddr(char *ifname, struct in_addr ipaddr, struct in_addr netmask);
int iflib_ifup(char *ifname);
int iflib_ifdown(char *ifname);
/*
 *roy+++
 */

void buid_ip_set(struct ip_set *setp,
                 const char *if_name,
                 unsigned int addrs_ip,
                 unsigned int addrs_netmask,
                 unsigned int addrs_broadcast,
                 unsigned int addrs_gateway,
                 unsigned int addrs_server,
                 unsigned int mtu,
                 unsigned char	mode,
                  const char *domain,
                 unsigned char	*data	);

//+++
#endif	/* __IFLIB_H__ */
