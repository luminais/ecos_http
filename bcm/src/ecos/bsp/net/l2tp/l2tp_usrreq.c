/*
 * L2 to PPP functions
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp_usrreq.c,v 1.5 2010-07-19 08:21:08 Exp $
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <net/l2tp_proto.h>
#include <net/ifl2tp.h>
#include <kdev.h>


#define	L2TP_MINLEN		6

#define NL2TP			1
struct ifl2tp *ifl2tp = NULL;

#undef	malloc
#undef  free
extern  void *malloc(int);
extern  void free(void *);
int	atoi(const char *nptr);

extern int (*dialup_checkp) __P((void));
struct mbuf *(*l2tpinputp) __P((struct mbuf *m, int hlen));

/* Dialup check function */
static int
ifl2tp_dialup_check(void)
{
	struct ifl2tp *ifp = ifl2tp;
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

#if 0        //modify by zzh 20140328
/* Call by pppouput() */
int
ifl2tp_output(struct ifnet *ppp_ifp, struct mbuf *m0)
{
	struct ifl2tp *ifp = (struct ifl2tp *)ppp_ifp;

	struct mbuf *m;
	struct ip *ip;
	struct udphdr *uh;
	unsigned char *l2h;
	int len;
	int iphlen;
	int uhlen;
	struct route ro;
	struct sockaddr_in *sin;
	/*
	 * Check if this interface has
	 * been established.
	 */
	if (ifp != ifl2tp) {
		m_freem(m0);
		return 0;
	}

	/* Prepare l2tp header */
	M_PREPEND(m0, L2TP_MINLEN+2, M_DONTWAIT);
	if (m0 == 0)
		return 0;

	len = m0->m_pkthdr.len;

	/* Fill l2 header */
	l2h = (unsigned char *)mtod(m0, char *);
	l2h[0] = LENGTH_BIT;
	l2h[1] = 2;
	l2h[2] = len >> 8;
	l2h[3] = len & 0xff;
	l2h[4] = ifp->assigned_tid >> 8;
	l2h[5] = ifp->assigned_tid & 0xff;
	l2h[6] = ifp->assigned_sid >> 8;
	l2h[7] = ifp->assigned_sid & 0xff;

	/*
	 * Allocate a new m for header
	 */
	MGET(m, M_DONTWAIT, m0->m_type);
	if (m == NULL) {
		m_freem(m0);
		return 0;
	}

	if (m0->m_flags & M_PKTHDR)
		M_COPY_PKTHDR(m, m0);

	m->m_next = m0;

	/* Setup udp/ip header */
	m->m_data += 16;
	ip = mtod(m, struct ip *);
	uh = (struct udphdr *)(ip+1);

	iphlen = sizeof(*ip);
	uhlen = sizeof(*uh);

	m->m_len = iphlen + uhlen;
	m->m_pkthdr.len = m->m_len + len;

	/* Setup ui and checksum */
	memset(ip, 0, iphlen + uhlen);

	ip->ip_p = IPPROTO_UDP;
	ip->ip_len = htons(uhlen + len);
	ip->ip_src = ifp->tunnel_addr;
	ip->ip_dst = ifp->server_addr;
	uh->uh_sport = htons(L2TP_PORT);
	uh->uh_dport = ifp->server_port;
	uh->uh_ulen = ip->ip_len;

	/*
	 * Stuff checksum and output datagram.
	 */
	uh->uh_sum = 0;
	if ((uh->uh_sum = in_cksum(m, sizeof(struct ip)+sizeof(struct udphdr) + len)) == 0)
		uh->uh_sum = 0xffff;

	/* Directly output to ip */
	ip->ip_len = iphlen + uhlen + len;
	ip->ip_ttl= 128;
	memset(&ro, 0, sizeof(struct route));
	sin = (struct sockaddr_in *)&ro.ro_dst;
	sin->sin_addr = ifp->server_addr;
	/* Directly output to ip */
	ip_output(m, 0, &ro, 0, 0);
	return 0;
}
#endif
int
ifl2tp_output(struct ifnet *ppp_ifp, struct mbuf *m0)
{
	struct ifl2tp *ifp = (struct ifl2tp *)ppp_ifp;
	struct mbuf *m;
	struct ip *ip;
	struct udphdr *uh;
	unsigned char *l2h;
	int len,hlen;
	int iphlen;
	int uhlen;
	struct route ro;
	struct sockaddr_in *sin;
	if (ifp != ifl2tp) {
		m_freem(m0);
		return 0;
	}
    len = 0;
	for (m = m0; m; m = m->m_next)
		len += m->m_len;
	m0->m_pkthdr.len = len;  
	iphlen = sizeof(*ip);
	uhlen = sizeof(*uh);
	hlen = iphlen+ uhlen + L2TP_MINLEN+2;
	M_PREPEND(m0, hlen , M_DONTWAIT);
	if (m0 == 0)
		return 0;
	ip = mtod(m0, struct ip *);
	uh = (struct udphdr *)(ip+1);
	l2h=(unsigned char *)(uh+1);
	l2h[0] = LENGTH_BIT;
	l2h[1] = 2;
	l2h[2] = (m0->m_pkthdr.len-28) >> 8;
	l2h[3] = (m0->m_pkthdr.len-28) & 0xff;
	l2h[4] = ifp->assigned_tid >> 8;
	l2h[5] = ifp->assigned_tid & 0xff;
	l2h[6] = ifp->assigned_sid >> 8;
	l2h[7] = ifp->assigned_sid & 0xff;
	memset(ip, 0, iphlen + uhlen);
	ip->ip_p = IPPROTO_UDP;
	ip->ip_len = htons(uhlen + len+8);
	ip->ip_src = ifp->tunnel_addr;
	ip->ip_dst = ifp->server_addr;
	uh->uh_sport = htons(L2TP_PORT);
	uh->uh_dport = ifp->server_port;
	uh->uh_ulen = ip->ip_len;
	uh->uh_sum = 0;
	if ((uh->uh_sum = in_cksum(m0, m0->m_pkthdr.len)) == 0)
		uh->uh_sum = 0xffff;
	ip->ip_len = m0->m_pkthdr.len;
	ip->ip_ttl= 128;
	memset(&ro, 0, sizeof(struct route));
	sin = (struct sockaddr_in *)&ro.ro_dst;
	sin->sin_addr = ifp->server_addr;
	ip_output(m0, 0, &ro, 0, 0);
	return 0;
}

/* Call by udp_input() */
struct mbuf *
ifl2tp_input(struct mbuf *m, int hlen)
{
	struct ifl2tp *ifp;
	unsigned char *buf;
	unsigned char *tidptr;
	unsigned short tid;
	unsigned short sid;
	int off = L2TP_MINLEN;
	int payload_len = m->m_pkthdr.len - hlen;
	int framelen = payload_len;

	/* Make sure l2 header exist */
	if (m->m_len < hlen+off &&
	    (m = m_pullup(m, hlen+off)) == 0) {
		return 0;
	}

	buf = mtod(m, unsigned char *) + hlen;
	tidptr = buf + 2;

	/* Check version, and data frame type */
	if ((buf[1] & VERSION_MASK) != 2 ||
	    (buf[0] & TYPE_BIT)) {
		return m;
	}

	/* Check length bit */
	if (buf[0] & LENGTH_BIT) {
		off += 2;
		tidptr += 2;

		/*
		 * Pull two more bytes
		 * if m_len is not enough
		 */
		if (m->m_len < hlen+off) {
			if ((m = m_pullup(m, hlen+off)) == 0)
				return 0;

			buf = mtod(m, unsigned char *) + hlen;
		}

		framelen = (((unsigned short)buf[2]) << 8) + buf[3];
	}

	tid = tidptr[0]*256 + tidptr[1];
	sid = tidptr[2]*256 + tidptr[3];

	/* Check tid and sid */
	ifp = ifl2tp;
	if (ifp == 0 || !(ifp->tid == tid &&
		ifp->sid == sid))
		return m;

	/*
	 * The l2tp session was found
	 */
	if (buf[0] & SEQUENCE_BIT)
		off += 4;

	/* Check offset bit for the last step */
	if (buf[0] & OFFSET_BIT) {
		off += 2;

		/* Pull two more bytes for offset */
		if (m->m_len < hlen+off) {
			if ((m = m_pullup(m, hlen+off)) == 0)
				return 0;

			buf = mtod(m, unsigned char *) + hlen;
		}

		/* Recaculate the offset */
		off += buf[off-2]*256 + buf[off-1];
	}

	/* Check packet length */
	if (framelen > payload_len || framelen < off) {
		m_freem(m);
		return 0;
	}

	/* Trim the l2tp header */
	m_adj(m, hlen+off);

	/* Set received interface */
	m->m_pkthdr.rcvif = &ifp->if_ppp.sc_if;

	ppppktin(&ifp->if_ppp, m);
	return 0;
}

/* Do pptp gre detach */
static int
ifl2tp_del(struct ifl2tp *ifp)
{
	if (!ifl2tp)
		return ENODEV;

	ifl2tp = NULL;
	if_detach(&ifp->if_ppp.sc_if);
	free(ifp);

	l2tpinputp = NULL;
	dialup_checkp = NULL;
	return 0;
}

static struct ifl2tp *
ifl2tp_add(void *devtab, char *pppname)
{
	struct ifl2tp *ifp;
	struct ifppp *ifppp;
	int unit;

	/* Check if this one is used */
	if (ifl2tp)
		return ifl2tp;

	if (ifunit(pppname) != 0)
		return NULL;

	unit = atoi(pppname+3);
	if (memcmp(pppname, "ppp", 3) != 0 ||
	    unit < 0 || unit > NL2TP) {
		return NULL;
	}

	/* Take this resouce */
	ifp = (struct ifl2tp *)malloc(sizeof(*ifp));
	if (ifp == NULL)
		return NULL;

	memset(ifp, 0, sizeof(*ifp));
	ifp->devtab = devtab;
	ifl2tp = ifp;

	/* Setup ppp structure */
	ifppp = &ifp->if_ppp;
	strcpy(ifppp->pppname, pppname);
	ifppp->sc_unit = unit;
	ifppp->pOutput = ifl2tp_output;

	pppattach(ifppp, unit);
	return ifp;
}

static int
ifl2tp_connect(struct ifl2tp *ifp, struct ifl2tpreq *req)
{
	/* Set l2tp param */
	ifp->tunnel_addr = req->tunnel_addr;
	ifp->server_addr = req->server_addr;
	ifp->server_port = req->server_port;
	ifp->assigned_tid = req->assigned_tid;
	ifp->assigned_sid = req->assigned_sid;
	ifp->tid = req->tid;
	ifp->sid = req->sid;

	/* Add hook */
	if (ifp->tunnel_addr.s_addr != 0)
		l2tpinputp = ifl2tp_input;
	else
		l2tpinputp = NULL;
	return 0;
}

/* 
 * Do ifl2tp ioctl
 * Note, should connect to socket,
 * but will complete it after
 * reconstruting ppp module.
 */
static int
ifl2tp_ioctl(struct ifl2tp *ifp, u_long cmd, caddr_t data)
{
	struct ifl2tpreq *req = (struct ifl2tpreq *)data;
	int error = 0;
	int s = splimp();

	switch (cmd) {
	case PPPIOCIFL2TPCONN:
		error = ifl2tp_connect(ifp, req);
		break;

	case PPPIOCIFL2TPWAIT:
		if(req->wait)
			dialup_checkp = ifl2tp_dialup_check;
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
KDEV_NODE(l2tp,
	ifl2tp_add,
	NULL,
	ifl2tp_ioctl,
	ifl2tp_del);
