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
 * $Id: kdhcpc.c,v 1.11 2010-07-06 02:12:14 Exp $
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
#include <netinet/if_ether.h>
#include <sys/socketvar.h>

#include <cyg/fileio/fileio.h>
#include <kdhcpc.h>
#include <ip_dev.h>
#include <kdev.h>

#undef	malloc
#undef  free
extern  void *malloc(int);
extern  void free(void *);

#define SERVER_PORT			67
#define CLIENT_PORT			68

static struct ipdev kdhcpc_ipdev;
static struct kdhcpc *kdhcpc_head;

/* Hook function to the ether_input */
static int
kdhcpc_check(struct ifnet *ifp, char *head, struct mbuf *m)
{
	struct kdhcpc *kdp;
	struct udphdr *udp;
	struct ip *ip;
	int len;

	for (kdp = kdhcpc_head; kdp; kdp = kdp->next) {
		if (kdp->status && kdp->ifp == ifp)
			break;
	}
	if (!kdp)
		return 0;

	ip =  mtod(m, struct ip *);
	if (ip->ip_p != IPPROTO_UDP)
		return 0;

	udp =  (struct udphdr *) ((char *)ip + (ip->ip_hl << 2));
	if (ntohs(udp->uh_dport) != CLIENT_PORT)
		return 0;


	/* Adjust to dhcp header */
	len = (ip->ip_hl << 2) + sizeof(struct udphdr);
	m_adj(m, len);

	/* Send up to kdev */
	kdev_input(kdp->devtab, m);
	return 1;
}

static int
kdhcpc_write(struct kdhcpc *kdp, char *buf, int len)
{
	struct mbuf *m;
	struct sockaddr dst;
	struct ether_header *eh;
	int s;

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
	m->m_data += 64;
	m->m_len = len;

	memcpy(m->m_data, buf, len);
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
	eh->ether_type = ntohs(ETHERTYPE_IP);

	memcpy(eh->ether_dhost, "\xff\xff\xff\xff\xff\xff", sizeof(eh->ether_dhost));

	/* Raw send */
	s = splimp();
	ether_output(kdp->ifp, m, &dst, 0);
	splx(s);

	return 0;
}

/* Add a receive interface to the dhcpc filter */
struct kdhcpc *
kdhcpc_add(void *devtab, char *ifname)
{
	struct kdhcpc *kdp;
	struct ifnet *ifp;

	/* Search in the list */
	for (kdp = kdhcpc_head; kdp; kdp = kdp->next) {
		if (strcmp(kdp->ifname, ifname) == 0)
			break;
	}
	if (kdp)
		return kdp;

	ifp = ifunit(ifname);
	if (ifp == 0)
		return NULL;

	/* Allocate a new one if not found */
	kdp = (struct kdhcpc *)malloc(sizeof(*kdp));
	if (kdp == 0)
		return NULL;

	memset(kdp, 0, sizeof(*kdp));

	strcpy(kdp->ifname, ifname);
	kdp->ifp = ifp;
	kdp->status = 0;
	kdp->devtab = devtab;

	/* Do prepend */
	kdp->next = kdhcpc_head;
	kdhcpc_head = kdp;

	/* Install to ipdev filter, if this is the first */
	if (kdp->next == NULL) {
		memset(&kdhcpc_ipdev, 0, sizeof(kdhcpc_ipdev));
		kdhcpc_ipdev.func = kdhcpc_check;

		ipdev_add(&kdhcpc_ipdev);
	}

	return kdp;
}

/* Delete the interface from the receive filter */
static int
kdhcpc_remove(struct kdhcpc *kdp)
{
	struct kdhcpc *curr, *prev;

	/* Free this interface */
	for (prev = 0, curr = kdhcpc_head; curr; prev = curr, curr = curr->next) {
		if (curr == kdp) {
			/* Dequeue */
			if (prev == 0)
				kdhcpc_head = curr->next;
			else
				prev->next = curr->next;

			/* Do clean up */
			free(curr);

			/* Uninstall filter */
			if (kdhcpc_head == NULL)
				ipdev_remove(&kdhcpc_ipdev);
			break;
		}
	}

	return 0;
}

static int
kdhcpc_ioctl(struct kdhcpc *kdp, unsigned long cmd, char *data)
{
	switch (cmd) {
	case DHCPCIOCADDIFFR:
		kdp->status = 1;
		return 0;

	case DHCPCIOCDELIFFR:
		kdp->status = 0;
		return 0;

	default:
		break;
	}

	return 0;
}

/* Declare kdev */
KDEV_NODE(dhcpc,
	kdhcpc_add,
	kdhcpc_write,
	kdhcpc_ioctl,
	kdhcpc_remove);
