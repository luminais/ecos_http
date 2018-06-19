/*
 * DHCP server ecos include files.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd_ecos_osl.h,v 1.4 2010-08-10 06:07:37 Exp $
 */

#ifndef __DHCPD_ECOS_OSL_H__
#define __DHCPD_ECOS_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ecos_oslib.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <fcntl.h>
#include <errno.h>
#include <netinet/ip_var.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <bcmnvram.h>

#include <sys/syslog.h>

#if 0
#define DHCPD_DEBUG_OFF		0
#define DHCPD_DEBUG_ERROR		1
extern int DHCPDebugLevel;
#define DHCPD_DBG(args...)   \
{                                           \
    if (DHCPDebugLevel == DHCPD_DEBUG_ERROR)                      \
    {                                       \
		syslog(LOG_DEBUG,##args);        \
    }                                       \
}

#define DHCPD_LOG(args...)   \
{                                           \
    if (DHCPDebugLevel == DHCPD_DEBUG_ERROR)                      \
    {                                       \
		syslog(LOG_DEBUG,##args);        \
    }                                       \
}
#else

#define DHCPD_DEBUG_OFF		0
#define DHCPD_DEBUG_ERROR		1
extern int DHCPDebugLevel;
#define DHCPD_DBG(args...) 
#define DHCPD_LOG(args...)

#endif

extern char *ether_ntoa_r(const struct ether_addr *addr, char *buf);
extern struct ether_addr *ether_aton_r(const char *asc, struct ether_addr *addr);

int  dhcpd_start(void);
void dhcpd_stop(void);

struct lease_t;
struct lease_t *dhcpd_lease_dump(char *ifname);

#define	DHCPD_LOCK()	cyg_scheduler_lock()
#define DHCPD_UNLOCK()	cyg_scheduler_unlock()

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __DHCPD_ECOS_OSL_H__ */
