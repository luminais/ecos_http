/*
 * PPTP GRE protocol switch.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ifpptp.c,v 1.5 2010-07-19 08:19:31 Exp $
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <net/pptp_gre.h>
#include <kdev.h>

struct ifpptp *ifpptp = NULL;
#define NPPTP 1

#undef	malloc
#undef  free
extern  void *malloc(int);
extern  void free(void *);
int atoi(const char *nptr);

extern int (*dialup_checkp) __P((void));
struct mbuf *(*ifpptp_inputp) __P((struct mbuf *m, int hlen));

/* Dialup check function */
static int
ifpptp_dialup_check(void)
{
	struct ifpptp *ifp = ifpptp;
	struct mbuf *m;

	if (ifp == NULL)
		return -1;

	/* Allocate mbuf */
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
		return 0;

	MCLGET(m, M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) {
		m_freem(m);
		return 0;
	}

	/* Copy event to mbuf */
	m->m_len = sizeof("DIALUP");
	memcpy(m->m_data, "DIALUP", m->m_len);

	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.rcvif = 0;

	/* Send up to kdev */
	kdev_input(ifp->devtab, m);
	return 0;
}

/* output function call by pppoutput */
int
ifpptp_output(struct ifnet *ppp_ifp, struct mbuf *m)
{
	struct ifpptp *ifp = (struct ifpptp *)ppp_ifp;
	struct ip *ip;
	struct pptp_gre_hdr *gre;
	struct mbuf *n;
	int len, hlen;
	struct route ro;
	struct sockaddr_in *sin;

	/*
	 * Bang in a pre-made header, and set the length up
	 * to be correct. Then send it to the ethernet driver.
	 * But first correct the length.
	 */
	len = 0;
	for (n = m; n; n = n->m_next)
		len += n->m_len;

	m->m_pkthdr.len = len;

	/*
	 * Fill the gre header
	 */
	hlen = sizeof(*ip) + sizeof(*gre);
	if (ifp->ack_sent == ifp->seq_recv)
		hlen -= sizeof(gre->ack_num);

	/* Reserved room for pptp session */
	M_PREPEND(m, (hlen + max_linkhdr), M_DONTWAIT);
	if (m == 0)
		return ENOBUFS;

	/* Back to gre heder offset */
	m->m_len -= max_linkhdr;
	m->m_data += max_linkhdr;
	m->m_pkthdr.len -= max_linkhdr;

	ip = mtod(m, struct ip *);
	gre = (struct pptp_gre_hdr *)(ip+1);

	/* Build gre header */
	gre->flags = PPTP_GRE_FLAGS_K | PPTP_GRE_FLAGS_S;
	gre->version = PPTP_GRE_VER;
	gre->protocol = htons(PPTP_GRE_PROTO);
	gre->payload_len = htons(len);
	gre->call_id = htons(ifp->call_id);
	gre->seq_num = htonl(ifp->seq_sent);
	if (ifp->ack_sent != ifp->seq_recv) {
		gre->version |= PPTP_GRE_VER_ACK;
		gre->ack_num = htonl(ifp->seq_recv);
		ifp->ack_sent = ifp->seq_recv;
	}

	ifp->seq_sent++;

	/* Build ip header */
	memset(ip, 0, sizeof(*ip));
	ip->ip_tos = 0;
	ip->ip_len = hlen + len;
	ip->ip_off = 0;
	ip->ip_p = IPPROTO_GRE;
	ip->ip_src = ifp->tunnel_ip;
	ip->ip_dst = ifp->server_ip;
	ip->ip_ttl= 128;
	memset(&ro, 0, sizeof(struct route));
	sin = (struct sockaddr_in *)&ro.ro_dst;
	sin->sin_addr = ifp->server_ip;
	/* Directly output to ip */
	ip_output(m, 0, &ro, 0, 0);
	return 0;
}

/*
 * PPTP protocol entry for gre_input
 */
struct mbuf *
ifpptp_input(struct mbuf *m, int iphlen)
{
	struct ifpptp *ifp;
	int grelen, minlen;
	struct ip *ip = mtod(m, struct ip *);
	struct pptp_gre_hdr *gre;
	unsigned int payload_len;
	u_int32_t ack_num;
	u_int32_t seq_num;

	/*
	* ip_input substracts the iphlen,
	* Validate lengths
	*/
	grelen = ip->ip_len;
	if (grelen < GREMINLEN)
		goto quit;

	minlen = iphlen + GREMINLEN;
	if (m->m_len < minlen && (m = m_pullup(m, minlen)) == 0)
		goto quit;

	/* Now ip/gre is okay to check */
	ip = mtod(m, struct ip *);
	gre = (struct pptp_gre_hdr *)((char *)ip + iphlen);

	if ((gre->version & 0x7f) != PPTP_GRE_VER ||
	    ntohs(gre->protocol) != PPTP_GRE_PROTO ||
	    (gre->flags & PPTP_GRE_FLAGS_C) ||
	    (gre->flags & PPTP_GRE_FLAGS_R) ||
	    !(gre->flags & PPTP_GRE_FLAGS_K) ||
	    (gre->flags & 0x0f)) {
		/*
		 * Something wrong,
		 * free this one.
		 */
		goto quit;
	}

	/* Check the pptp sessions */
	ifp = ifpptp;
	if (ifp == 0 || !(ip->ip_src.s_addr == ifp->server_ip.s_addr &&
		ip->ip_dst.s_addr == ifp->tunnel_ip.s_addr &&
		ntohs(gre->call_id) == ifp->peer_call_id)) {
		/*
		 * Not match, let rip_input
		 * continue the job.
		 */
		return m;
	}

	/* Check the ack status */
	if (gre->version & PPTP_GRE_VER_ACK) {
		/*
		 * They might be ack only packet,
		 * which located at the same position
		 * as sequence.
		 */
		if (gre->flags & PPTP_GRE_FLAGS_S)
			ack_num = ntohl(gre->ack_num);
		else
			ack_num = ntohl(gre->seq_num);

		/* Larger ack or wrap around, update it */
		if (ack_num > ifp->ack_recv ||
		    ((ack_num >> 31) == 0 &&
		     (ifp->ack_recv >> 31) == 1)) {
			ifp->ack_recv = ack_num;
		}
	}

	/*
	 * Should have a sequence number,
	 * or it is a pure ack, free it.
	 */
	if ((gre->flags & PPTP_GRE_FLAGS_S) == 0)
		goto quit;

	/*
	 * Check payload
	 */
	grelen = sizeof(*gre);
	if ((gre->version & PPTP_GRE_VER_ACK) == 0)
		grelen -= sizeof(gre->ack_num);

	payload_len = ntohs(gre->payload_len);
	seq_num = ntohl(gre->seq_num);

	/* Check out of order sequence */
	if (seq_num > ifp->seq_recv ||
	    (seq_num == 0 && ifp->seq_recv == 0) ||
	    ((seq_num >> 31) == 0 && (ifp->seq_recv >> 31) == 1)) {
		/* Update sequence received */
		ifp->seq_recv = seq_num;

		/* Decapsulate the gre header */
		m_adj(m, iphlen + grelen);

		/*
		 * Patch for the ip protocol stack
		 * M_PREPEND issue.
		 */
		if (m->m_len == 0 && !(m->m_flags & M_EXT)) {
			/* Do mbuf cleanup */
			struct mbuf *n = m->m_next;
			if (n && (n->m_flags & M_EXT)) {
				m->m_flags |= M_EXT;
				m->m_ext = n->m_ext;
				m->m_next = n->m_next;
				m->m_len  = n->m_len;
				m->m_data = n->m_data;

				/* Merge cluster of n to m */
				n->m_flags &= ~M_EXT;
				m_free(n);
			}
		}

		/* Send to ppp */
		ppppktin(&ifp->if_ppp, m);
		return 0;
	}

quit:
	if (m)
		m_freem(m);

	return 0;
}

/* Do ifpptp delete */
static int
ifpptp_del(struct ifpptp *ifp)
{
	if (!ifpptp)
		return ENODEV;

	ifpptp = NULL;
	if_detach(&ifp->if_ppp.sc_if);
	free(ifp);

	ifpptp_inputp = NULL;
	dialup_checkp = NULL;
	return 0;
}

/* Add interface pptp to list */
static struct ifpptp *
ifpptp_add(void *devtab, char *pppname)
{
	struct ifpptp *ifp;
	struct ifppp *ifppp;
	int unit;

	/* Check if PPTP is used */
	if (ifpptp)
		return ifpptp;

	if (ifunit(pppname) != 0)
		return NULL;

	unit = atoi(pppname+3);
	if (memcmp(pppname, "ppp", 3) != 0 || unit < 0)
		return NULL;

	/* Take this resouce */
	ifp = (struct ifpptp *)malloc(sizeof(*ifp));
	if (ifp == NULL)
		return NULL;
	
	memset(ifp, 0, sizeof(*ifp));
	ifp->devtab = devtab;
	ifpptp = ifp;

	/* Setup ppp structure */
	ifppp = &ifp->if_ppp;
	strcpy(ifppp->pppname, pppname);
	ifppp->sc_unit = unit;
	ifppp->pOutput = ifpptp_output;
	
	pppattach(ifppp, unit);
	return ifp;
}

static int
ifpptp_connect(struct ifpptp *ifp, struct ifpptpreq *req)
{
	/* Set pptp gre param */
	ifp->tunnel_ip = req->tunnel_ip;
	ifp->server_ip = req->server_ip;
	ifp->call_id = req->call_id;
	ifp->peer_call_id = req->peer_call_id;

	ifp->ack_sent = 0;
	ifp->ack_recv = 0;
	ifp->seq_sent = 0;
	ifp->seq_recv = 0;

	/* Add hook */
	if (req->tunnel_ip.s_addr != 0)
		ifpptp_inputp = ifpptp_input;
	else
		ifpptp_inputp = NULL;

	return 0;
}

/* 
 * Do pptp gre ioctl
 */
static int
ifpptp_ioctl(struct ifpptp *ifp, u_long cmd, caddr_t data)
{
	struct ifpptpreq *req = (struct ifpptpreq *)data;
	int error = 0;
	int s = splimp();

	switch (cmd) {
	case PPPIOCPPTPCONN:
		error = ifpptp_connect(ifp, req);
		break;

	case PPPIOCPPTPWAIT:
		if (req->wait)
			dialup_checkp = ifpptp_dialup_check;
		else
			dialup_checkp = 0;
		break;

	default:
		error = EPFNOSUPPORT;
		break;
	}

	splx(s);
	return error;
}

/* Declare kdev */
KDEV_NODE(pptp,
	ifpptp_add,
	NULL,
	ifpptp_ioctl,
	ifpptp_del);
