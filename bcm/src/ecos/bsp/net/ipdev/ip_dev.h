/*
 * Include file for ip layer device handler
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ip_dev.h,v 1.2 2010-08-09 11:04:29 Exp $
 */

#ifndef _NETINET_IP_DEV_H
#define _NETINET_IP_DEV_H

struct ipdev {
	struct ipdev *next;
	unsigned char priority;
	int (*func)(struct ifnet *, char *, struct mbuf *);
};

int ipdev_check(struct ifnet *ifp, char *eh, struct mbuf **m0);
int ipdev_add(struct ipdev *ipdev);
void ipdev_remove(struct ipdev *ipdev);

#endif	/* _NETINET_IP_DEV_H */
