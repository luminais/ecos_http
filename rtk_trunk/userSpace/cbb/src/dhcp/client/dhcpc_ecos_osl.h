/*
 * DHCP client include file for eCos platform.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpc_ecos_osl.h,v 1.5 2010-07-24 12:48:31 Exp $
 */

#ifndef __DHCPC_ECOS_OSL_H__
#define __DHCPC_ECOS_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <unistd.h>

#include <sys/syslog.h>

#define DHCPC_DBG(a, b...) 	syslog(LOG_USER|LOG_INFO, a,##b)
#define DHCPC_DIAG(a, b...)

void dhcpc_start(char *ifname, char *script, char *hostname);
void dhcpc_stop(char *ifname);
void dhcpc_renew(char *ifname);
void dhcpc_release(char *ifname);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __DHCPC_ECOS_OSL_H__ */
