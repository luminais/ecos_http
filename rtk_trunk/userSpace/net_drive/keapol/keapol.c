/*
 * DHCPC kernel mode code
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: keapol.c,v 1.6 2010-07-06 02:12:15 Exp $
 */


#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <sys/socketvar.h>

#include <cyg/fileio/fileio.h>
#include "keapol.h"
#include <kdev.h>

#undef	malloc
#undef  free
extern  void *malloc(int);
extern  void free(void *);
extern	int atoi(char *);

static struct keapol *keapol_head;

/* Hook to eapol input */
int keapol_check(struct ifnet *ifp, struct ether_header *eh, struct mbuf *m);
int (*eapol_input_hook)(struct ifnet *ifp, struct ether_header *eh, struct mbuf *m) = keapol_check;


/* Hook function to the ether_input */
int
keapol_check(struct ifnet *ifp, struct ether_header *eh, struct mbuf *m)
{
	struct keapol *kdp;

	/* Search in the filter list */
	for (kdp = keapol_head; kdp; kdp = kdp->next) {
		if ((kdp->ifp == ifp && eh->ether_type == kdp->type) || (eh->ether_type == htons(ETHERTYPE_PAE)&& eh->ether_type == kdp->type))
		//if (kdp->ifp == ifp && eh->ether_type == kdp->type)
			break;
	}
	if (!kdp) {
		m_freem(m);
		return 0;
	}

	/*
	 * Move back to ether header
	 */
	m->m_data -= sizeof(*eh);
	m->m_len += sizeof(*eh);
	m->m_pkthdr.len += sizeof(*eh);

	/* Send up to kdev */
	kdev_input(kdp->devtab, m);
	return 1;
}

static int
keapol_write(struct keapol *kdp, char *buf, int len)
{
	struct mbuf *m;
	struct sockaddr dst;
	struct ether_header *eh;
	char *payload;
	int payload_len;
	int s;

	if (len < sizeof(*eh)) {
		printf("%s: Warning: len < sizeof(*eh)\n", __func__);
		return EINVAL;
	}

	/* generate a packet of that type */
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
		return ENOBUFS;

	MCLGET(m, M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) {
		m_freem(m);
		return ENOBUFS;
	}

	/* Reserved room for ether header, count WLAN header */
	payload = buf + sizeof(*eh);
	payload_len = len - sizeof(*eh);

	m->m_data += 64;
	m->m_len = payload_len;

	memcpy(m->m_data, payload, payload_len);
	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.rcvif = 0;

	/*
	* Set destination to send and tell
	* the ether_output() to do raw send
	* without routing for us.
	*/
	memset(&dst, 0, sizeof(dst));
	dst.sa_len = 16;
	dst.sa_family = AF_UNSPEC;

	eh = (struct ether_header *)dst.sa_data;
	memcpy(eh, buf, sizeof(*eh));

	/* Raw send */
	s = splimp();
	ether_output(kdp->ifp, m, &dst, 0);
	splx(s);

	return 0;
}

/* Add a receive interface to the keapol filter */
static struct keapol *
keapol_add(void *devtab, char *pathname)
{
	struct keapol *kdp;
	struct ifnet *ifp;
	char ifname[32] = {0};
	char *p;
	int type = 0;

	/* Search in the list */
	for (kdp = keapol_head; kdp; kdp = kdp->next) {
		if (strcmp(kdp->pathname, pathname) == 0)
			return kdp;
	}

	/* Make ifname and type */
	if (strlen(pathname) > sizeof(ifname)-1)
		return NULL;

	strcpy(ifname, pathname);

	p = strchr(ifname, '/');
	if (p == NULL)
		return NULL;
	*p++ = 0;

	/* Break ifname and type */
	type = atoi(p);
	if (type <= 0 || type >= 65536)
		return NULL;

	/* Check ifunit */
	ifp = ifunit(ifname);
	if (ifp == 0)
		return NULL;

	/* Allocate a new one if not found */
	kdp = (struct keapol *)malloc(sizeof(*kdp));
	if (kdp == 0)
		return NULL;

	memset(kdp, 0, sizeof(*kdp));

	strcpy(kdp->pathname, pathname);
	kdp->type = htons(type);
	kdp->ifp = ifp;
	kdp->devtab = devtab;

	/* Do prepend */
	kdp->next = keapol_head;
	keapol_head = kdp;

	return kdp;
}

/* Delete the interface from the receive filter */
static int
keapol_remove(struct keapol *kdp)
{
	struct keapol *curr, *prev;

	/* Free this interface */
	prev = 0;
	for (curr = keapol_head; curr; prev = curr, curr = curr->next) {
		if (curr == kdp) {
			/* Dequeue */
			if (prev == 0)
				keapol_head = curr->next;
			else
				prev->next = curr->next;

			/* Do clean up */
			free(curr);
			break;
		}
	}

	return 0;
}

/* Declare kdev */
KDEV_NODE(eapol,
	keapol_add,
	keapol_write,
	NULL,
	keapol_remove);
