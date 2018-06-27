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
 * $Id: kdhcpd.c,v 1.7 2010-07-06 02:12:14 Exp $
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
#include "kdhcpd.h"
#include <ip_dev.h>
#include <kdev.h>

#undef	malloc
#undef  free
extern  void *malloc(int);
extern  void free(void *);

#define SERVER_PORT			67
#define CLIENT_PORT			68

static struct ipdev kdhcpd_ipdev;
static struct kdhcpd *kdhcpd_head;


/* Hook function to the ether_input */
static int
kdhcpd_check(struct ifnet *ifp, char *head, struct mbuf *m)
{
	struct kdhcpd *kdp;
	struct udphdr *udp;
	struct ip *ip;
	int len;

	/* Search in the filter list */
	for (kdp = kdhcpd_head; kdp; kdp = kdp->next) {

		if(kdp->br_index & (1 << ifp->if_index))
		{
			//printf("kdp->ifp->if_name:%s===ifp->if_name:%s%d index:%d,%s [%d]\n",
			//	kdp->ifp->if_name,ifp->if_name,ifp->if_unit,ifp->if_index,__FUNCTION__, __LINE__);
			break;
		}
	}
	if (!kdp)
		return 0;
	ip =  mtod(m, struct ip *);
	if (ip->ip_p != IPPROTO_UDP)
		return 0;
	
	udp =  (struct udphdr *) ((char *)ip + (ip->ip_hl << 2));
	if (ntohs(udp->uh_dport) != SERVER_PORT)
		return 0;


	/* Adjust to dhcp header */
	len = (ip->ip_hl << 2) + sizeof(struct udphdr);
	m_adj(m, len);

	/* Send up to kdev */
	kdev_input(kdp->devtab, m);
	return 1;
}
struct ifnet* get_ifp_output(struct kdhcpd *kdp)
{
	unsigned int br_index = 0;
	unsigned int mask = 1;
	char ifname[16] = {0};
	struct ifnet *ifp = NULL;

	if(kdp == NULL)
		return NULL;

	br_index = kdp->br_index;
	//接口目前不会超过32个
	for(;mask <32;mask++)
	{
		if(br_index & (1<<mask))
		{
			if_indextoname(mask,ifname);
			if(INTERFACE_UP == get_interface_state(ifname))
			{
				ifp = ifunit(ifname);
				if(ifp != 0)
				{
					 return ifp;
				}
			}
		}
	}
	return NULL;
}
static int
kdhcpd_write(struct kdhcpd *kdp, char *buf, int len)
{
	struct ifnet *ifp;
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
	ifp = get_ifp_output(kdp);

	if(NULL == ifp)
	{	
		m_freem(m);
		return 0;
	}

	memset(&dst, 0, sizeof(dst));
	dst.sa_len = 16;
	dst.sa_family = AF_UNSPEC;

	eh = (struct ether_header *)dst.sa_data;
	eh->ether_type = ntohs(ETHERTYPE_IP);

	memcpy(eh->ether_dhost, "\xff\xff\xff\xff\xff\xff", sizeof(eh->ether_dhost));
	/* Raw send */
	s = splimp();
	ether_output(ifp, m, &dst, 0);
	splx(s);

	return 0;
}
extern unsigned int get_br_ifnames_index(char *ifname);
/* Add kdhcpd receive filter to ipdev */
static struct kdhcpd *
kdhcpd_add(void *devtab, char *ifname)
{
	struct kdhcpd *kdp;
	struct ifnet *ifp;
	/* Search in the list */
	for (kdp = kdhcpd_head; kdp; kdp = kdp->next) {
		if (strcmp(kdp->ifname, ifname) == 0)
			break;
	}
	if (kdp)
		return kdp;

	ifp = ifunit(ifname);
	if (ifp == 0)
		return NULL;

	/* Allocate a new one if not found */
	kdp = (struct kdhcpd *)malloc(sizeof(*kdp));
	if (kdp == 0)
	    return NULL;
	memset(kdp, 0, sizeof(*kdp));
	
	strcpy(kdp->ifname, ifname);
	kdp->ifp = ifp;
	kdp->br_index = get_br_ifnames_index(kdp->ifname);
	kdp->devtab = devtab;

	/* Do prepend */
	kdp->next = kdhcpd_head;
	kdhcpd_head = kdp;

	/* Install ipdev filter, if this is the first */
	if (kdp->next == NULL) {
		memset(&kdhcpd_ipdev, 0, sizeof(kdhcpd_ipdev));
		kdhcpd_ipdev.func = kdhcpd_check;

		ipdev_add(&kdhcpd_ipdev);
	}

	return kdp;
}

/* Delete the interface from the receive filter */
static int
kdhcpd_remove(struct kdhcpd *kdp)
{
	struct kdhcpd *curr, *prev;

	/* Free this interface */
	for (prev = 0, curr = kdhcpd_head; curr; prev = curr, curr = curr->next) {
		if (curr == kdp) {
			/* Dequeue */
			if (prev == 0)
				kdhcpd_head = curr->next;
			else
				prev->next = curr->next;

			/* Do clean up */
			free(curr);

			/* Uninstall filter */
			if (kdhcpd_head == NULL)
				ipdev_remove(&kdhcpd_ipdev);
			break;
		}
	}

	return 0;
}


/* Declare kdev */
KDEV_NODE(dhcpd,
	kdhcpd_add,
	kdhcpd_write,
	NULL,
	kdhcpd_remove);
