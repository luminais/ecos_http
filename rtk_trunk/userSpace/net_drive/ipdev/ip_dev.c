/*
 * ip layer packet handler
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ip_dev.c,v 1.5 2010-08-09 11:04:29 Exp $
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_var.h>
#include <netinet/ip_var.h>
#include "ip_dev.h"

int (*ipdev_check_hook)(struct ifnet *ifp, char *head, struct mbuf **m0);
static struct ipdev *ipdev_head = NULL;

int
ipdev_check(struct ifnet *ifp, char *eh, struct mbuf **m0)
{
	struct ip *ip;
	struct ipdev *ipdev;
	struct mbuf *m = *m0;

	/*
	 * Big enough for an IP/UDP/TCP packet?
	 */
	ip = mtod(m, struct ip *);

	if (m->m_len < 40 ||
	    ip->ip_v != IPVERSION ||
	    ip->ip_hl != (sizeof(struct ip) >> 2) ||
	    (ntohs(ip->ip_off) & 0x3fff) != 0 ||
	    ntohs(ip->ip_len) > m->m_pkthdr.len) {
		return 0;
	}

	/*
	 * Find a filter.
	 */
	for (ipdev = ipdev_head; ipdev; ipdev = ipdev->next) {
		if (ipdev->func && (*ipdev->func)(ifp, eh, m)) {
			/* Caught by the ipdev */
			*m0 = 0;
			return 1;
		}
	}

	return 0;
}

/* Add function */
int
ipdev_add(struct ipdev *ipdev)
{
	int s;
	struct ipdev *temp, *prev;

	if (ipdev == NULL || ipdev->func == NULL)
		return -1;

	if (!ipdev->priority)
		ipdev->priority = 0xff;

	s = splimp();

	/* Inert head */
	if (ipdev_head == NULL) {
		ipdev->next = 0;
		ipdev_head = ipdev;
		goto done;
	}

	/* Check duplication */
	for (temp = ipdev_head; temp; temp = temp->next) {
		if (temp->func == ipdev->func)
			goto done;
	}

	/* Do insertion */
	prev = NULL;
	for (temp = ipdev_head; temp; prev = temp, temp = temp->next) {
		/* Append at end of the same priority ones */
		if (ipdev->priority < temp->priority) {
			if (prev == NULL)
				ipdev_head = ipdev;
			else
				prev->next = ipdev;

			ipdev->next = temp;
			goto done;
		}

		/* Check last */
		if (temp->next == NULL) {
			ipdev->next = NULL;
			temp->next = ipdev;
			goto done;
		}
	}
done:
	/* Add hook */
	if (ipdev_head && !ipdev_check_hook)
		ipdev_check_hook = ipdev_check;

	splx(s);
	return 0;
}

/* Delete function */
void
ipdev_remove(struct ipdev *ipdev)
{
	int s;
	struct ipdev *temp, *prev;

	if (ipdev == NULL || ipdev->func == NULL)
		return;

	s = splimp();

	prev = NULL;
	for (temp = ipdev_head; temp; prev = temp, temp = temp->next) {
		if (ipdev->func == temp->func) {
			if (prev == NULL)
				ipdev_head = temp->next;
			else
				prev->next = temp->next;
			break;
		}
	}

	/* Remove hook */
	if (!ipdev_head)
		ipdev_check_hook = NULL;

	splx(s);
	return;
}
