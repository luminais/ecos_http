/*
 * PPPOE kernel mode code
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ifpppoe_usrreq.c,v 1.8 2010-07-19 08:16:40 Exp $
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
#include <arpa/inet.h>
#include <if_pppoe.h>
#include <kdev.h>
#include "nvram.h"

#ifndef	FALSE
#define	FALSE	0
#endif
#ifndef TRUE
#define	TRUE	1
#endif

extern int atoi(const char *nptr);
extern int (*dialup_checkp) __P((void));

#define	senderr(x) do {error = x; goto quit;} while (0)

/* PPPoE data */
#define NPPPOE CONFIG_PPP_NUM_SESSIONS
static struct ifpppoe ifpppoectl[NPPPOE];
static struct ifpppoe *ifpppoe = 0;

int ifpppoe_input(struct ifnet *, struct ether_header *, struct mbuf *);
int (*pppoe_input_hook)(struct ifnet *, struct ether_header *, struct mbuf *) = ifpppoe_input;

/* Dialup check function */
static int
ifpppoe_dialup_check(void)
{
	struct ifpppoe *ifp;
	struct mbuf *m;

	for (ifp = ifpppoe; ifp; ifp = ifp->next) {
		if (ifp->state != PPPOE_SDWAIT)
			continue;

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
	}

	return 0;
}

/*
 * Send packet to ethernet
 */
static int
ifpppoe_send(struct ifnet *eth_ifp, struct mbuf *m, char *dhost)
{
	struct sockaddr dst;
	struct ether_header *eh;
	int len;

	int s;

	if (m == 0)
	return 0;

	memset(&dst, 0, sizeof(dst));
	dst.sa_len = 16;
	dst.sa_family = AF_UNSPEC;

	eh = (struct ether_header *)dst.sa_data;
	eh->ether_type = ntohs(ETHERTYPE_PPPOE_SESS);
	memcpy(eh->ether_dhost, dhost, sizeof(eh->ether_dhost));

	/* Back to ph heder offset */
	len = sizeof(*eh);
	m->m_len -= len;
	m->m_data += len;
	m->m_pkthdr.len -= len;

	s = splimp();
	ether_output(eth_ifp, m, &dst, 0);
	splx(s);

	return 0;
}

/*
 * interface output function called by ppp protocol stack
 */
static int
ifpppoe_output(struct ifnet *ppp_ifp, struct mbuf *m)
{
	struct ifpppoe *ifp = (struct ifpppoe *)ppp_ifp;
	struct mbuf *n;
	short len;
	struct pppoe_hdr *ph;
	static const u_char addrctrl[] = { 0xff, 0x03 };

	/* Only available in connected state */
	if (ifp->state != PPPOE_CONNECTED) {
		m_freem(m);
		return ENETUNREACH;
	}

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
	* Remove PPP address and control fields, if any.
	* For example, ng_ppp(4) always sends LCP packets
	* with address and control fields as required by
	* generic PPP. PPPoE is an exception to the rule.
	*/
	if (m->m_len < 2) {
		if ((m = m_pullup(m, 2)) == 0)
			return ENOBUFS;
	}

	if (memcmp(mtod(m, u_char *), addrctrl, 2) == 0)
		m_adj(m, 2);

	/* Reload len because of m_adj */
	len = m->m_pkthdr.len;

	/* Reserved room for pppoe session */
	M_PREPEND(m, sizeof(struct pppoe_full_hdr), M_DONTWAIT);
	if (m == 0)
		return ENOBUFS;

	/* Build pppoe header */
	ph = (struct pppoe_hdr *)(m->m_data + sizeof(struct ether_header));
	ph->ver = 0x1;
	ph->type = 0x1;
	ph->code = 0;
	if(ifp->if_ppp.sc_unit == 1)//gong
	{
		ph->sid = 0x1234;
	}
	else
	{
	ph->sid = htons(ifp->sid);
	}
	ph->length = htons(len);

	/* Send out */
	ifpppoe_send(ifp->eth_ifp, m, (char *)ifp->dhost);
	return 0;
}

/*
 * This routine is called directly by the
 * ethernet interface (ifp), so we have to
 * build the PADT without using pppoe data.
 */
static void
ifpppoe_send_padt(struct ifnet *eth_ifp, char *dhost, unsigned short sid)
{
	struct mbuf *m;
	struct ether_header *eh;
	struct pppoe_hdr *ph;
	struct pppoe_tag *tag;
	unsigned short msglen = strlen(PADT_SIGNOFF);
	struct sockaddr dst;
	int s;

	/* generate a packet of that type */
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
		return;

	MCLGET(m, M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) {
		m_freem(m);
		return;
	}

	/* Reserved room for ether header */
	m->m_data += sizeof(*eh) + 2;

	/* Build PADT pppoe header */
	ph = mtod(m, struct pppoe_hdr *);
	memset(ph, 0, sizeof(*ph));

	ph->ver = 0x1;
	ph->type = 0x1;
	ph->code = PADT_CODE;
	ph->sid = htons(sid);
	ph->length = htons(sizeof(*tag) + msglen);

	/*
	* Add a General error message and adjust
	* sizes
	*/
	tag = ph->tag;
	tag->tag_type = htons(PTT_GEN_ERR);
	tag->tag_len = htons(msglen);
	strncpy((char *)tag->tag_data, PADT_SIGNOFF, msglen);

	m->m_len = sizeof(*ph) + sizeof(*tag) + msglen;
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
	eh->ether_type = ntohs(ETHERTYPE_PPPOE_DISC);
	memcpy(eh->ether_dhost, dhost, sizeof(eh->ether_dhost));

	/* Raw send */
	s = splimp();
	ether_output(eth_ifp, m, &dst, 0);
	splx(s);

	return;
}

/*
 * Routine to find a particular session,
 * which matches an incoming packet.
 */
static int
session_match(struct ifpppoe *ifp,
    const struct ether_header *eh, unsigned short sid)
{
	/*
	* find matching peer/session combination.
	*/
	if (ifp->state == PPPOE_CONNECTED &&
	    ifp->sid == sid )
	{
	    if ( memcmp(ifp->dhost, eh->ether_shost, 6) == 0) 
		{
		return TRUE;
	}
		else
		{
			if(ifp->if_ppp.sc_unit == 1)//gong			
			{
				memcpy(ifp->dhost,eh->ether_shost , 6);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	return FALSE;
}

/*
 * Check whether the incoming packet is ours
 * or not.  If yes, send to upper layer, or
 * return the mbuf to the caller, pppoe_input.
 */
static int
ifpppoe_sess_recv(struct ifnet *eth_ifp,
	struct ether_header *eh, struct mbuf *m)
{
	struct ifpppoe *ifp;
	struct pppoe_hdr *ph = mtod(m, struct pppoe_hdr *);
	unsigned short sid;
	unsigned short len;
	char *p;

	/*
	* find matching peer/session combination.
	*/
	for (ifp = ifpppoe; ifp; ifp = ifp->next) 
	{
		if (ifp->eth_ifp == eth_ifp)
		{
			if(ifp->if_ppp.sc_unit == 1)
			{	
				sid = 0x1234;
			}
			else
			{
				sid = ntohs(ph->sid);
			}
		  	if(session_match(ifp, eh, sid) == TRUE) 
			{
			break;
			}
		}
	}
	if (ifp == 0)
		return FALSE;

	/*
	* PPPoE does not include 0xff 0x03,
	* so make it as standard PPP input packet
	*/
	len = ntohs(ph->length) + 2;
	m_adj(m, sizeof(*ph)-2);
	if (m->m_pkthdr.len < len)
		goto quit;

	p = mtod(m, char *);
	*p++ = PPP_ALLSTATIONS;
	*p++ = PPP_UI;

	/* Also need to trim excess at the end */
	if (m->m_pkthdr.len > len)
		m_adj(m, -((int)(m->m_pkthdr.len - len)));

	/* Indicate to ppp */
	ppppktin(&ifp->if_ppp, m);
	return TRUE;

quit:
	if (m)
		m_freem(m);
	return TRUE;
}

/*
 * Return the location where the next
 * tag can be put.
 */
static __inline struct pppoe_tag *
next_tag(struct pppoe_hdr *ph)
{
	return (struct pppoe_tag *)(((char *)&ph->tag[0]) + ntohs(ph->length));
}

/*
 * Look for a tag of a specific type
 * Don't trust any length the other
 * end says.  but assume we already
 * sanity checked ph->length.
 */
static struct pppoe_tag *
get_tag(struct pppoe_hdr *ph, unsigned short idx)
{
	char *end = (char *)next_tag(ph);
	char *ptn;
	struct pppoe_tag *pt = &ph->tag[0];

	/*
	* Keep processing tags while a tag header will still fit.
	*/
	while ((char*)(pt + 1) <= end) {
		/*
		* If the tag data would go past the end of the packet, abort.
		*/
		ptn = (((char *)(pt + 1)) + ntohs(pt->tag_len));
		if (ptn > end)
			return NULL;

		if (ntohs(pt->tag_type) == idx)
			return pt;

		pt = (struct pppoe_tag *)ptn;
	}

	return NULL;
}

static int
host_match(struct ifpppoe *ifp, struct pppoe_full_hdr *fh)
{
	struct pppoe_hdr *ph = &fh->ph;
	struct pppoe_tag *utag = 0;
	unsigned short sid;

	/* Do session matching */
	switch (ifp->state) {
	case PPPOE_SINIT:
	case PPPOE_SREQ:
		utag = get_tag(ph, PTT_HOST_UNIQ);
		if (utag && ntohs(utag->tag_len) == sizeof(ifp->unique) &&
		    memcmp(utag->tag_data, ifp->unique, sizeof(ifp->unique)) == 0) {
			return TRUE;
		}
		break;

	case PPPOE_CONNECTED:
		sid = ntohs(ph->sid);
		if (ifp->sid == sid)
			return TRUE;

	default:
		break;
	}

	return FALSE;
}

static int
ifpppoe_disc_recv(struct ifnet *eth_ifp, struct mbuf *m)
{
	struct ifpppoe *ifp;
	struct pppoe_full_hdr *fh;

	/*
	 * Move back to ether header
	 */
	m->m_data -= sizeof(fh->eh);
	m->m_len += sizeof(fh->eh);
	m->m_pkthdr.len += sizeof(fh->eh);


	/* Do pppoe distiguishing */
	fh = mtod(m, struct pppoe_full_hdr *);

	for (ifp = ifpppoe; ifp; ifp = ifp->next) {
		if (ifp->eth_ifp == eth_ifp)
		{
		   	if( host_match(ifp, fh) == TRUE) 
				break;
			/*lq,如果先起ppp2接口，再起ppp1接口，
			则会导致数据包全部跑到ppp1接口去*/
			if(ifp->if_ppp.sc_unit == 1)
				break;
		}
	}
	if (ifp == 0)
		return FALSE;

	/* Send up to kdev */
	kdev_input(ifp->devtab, m);
	return TRUE;
}

/*
 * PPPOE input routine, called from
 * ether_input().
 */
int
ifpppoe_input(struct ifnet *eth_ifp,
	struct ether_header *eh, struct mbuf *m)
{
	struct pppoe_hdr *ph;
	struct ifpppoe *ifp;

	/*
	 * Note: The caller has to make sure there is
	 *       a ethernet header before the m_data
	 *       and the at least contains a pppoe
	 *       header of data.
	 */
	if (m->m_len < sizeof(*ph))
		goto drop_it;

	/* Check ETHERTYPE_PPPOE_DISC type packet */
	if (ntohs(eh->ether_type) == ETHERTYPE_PPPOE_DISC) {
		if (ifpppoe_disc_recv(eth_ifp, m) == FALSE)
			goto drop_it;
		return TRUE;
	}

	/* This must be a ETHERTYPE_PPPOE_SESS type packet */
	if (ifpppoe_sess_recv(eth_ifp, eh, m) == TRUE)
		return TRUE;

	/*
	* Send PADT ETHERTYPE_PPPOE_SESS packet received
	* with no interface wants it.  This is mostly like
	* because of rebooting.
	*/
	if (memcmp(eh->ether_dhost, ((struct arpcom *)eth_ifp)->ac_enaddr, 6) == 0) {
		const struct pppoe_hdr *ph = mtod(m, struct pppoe_hdr *);

		for (ifp = ifpppoe; ifp; ifp = ifp->next) {
			/*
			* Some buggy PPPOE servers send LCP before
			* before a PADR packet. If we send PADT in
			* this case, the PPPOE connect will never
			* be connected. So we choose to drop it in
			* the PADR state.
			*/
			if (memcmp(eh->ether_shost, ifp->dhost, 6) == 0) {
				if (ifp->state == PPPOE_SREQ) {
					/* Don't send padt, drop it silently */
					goto drop_it;
				}
				break;
			}
		}
		/*lq 网络环境导致，服务器具有不同的MAC，DUT识别非建立连接的MAC时会发送PADT，导致断网，修改，判断查找不到相应的接口就直接FREE掉该报文*/
		if(ifp == NULL)
			goto drop_it;
		/* 
		 * Now we are sure this session is not belonged to us,
		 * and we are all in PPPOE_CONNECTED stat, send PADT.
		 */
		ifpppoe_send_padt(eth_ifp, (char *)eh->ether_shost, ntohs(ph->sid));
	}

drop_it:
	/* Free the packet anyway */
	m_freem(m);
	return TRUE;
}

static int
ifpppoe_set_state(struct ifpppoe *ifp, struct ifpppoereq *req)
{
	int state;

	state = req->state;

	/* Do state */
	switch (state) {
	case PPPOE_SNONE:
	case PPPOE_SDWAIT:
		ifp->state = state;
		break;

	case PPPOE_SINIT:
		/* Setup pppoe structure */
		ifp->eth_ifp = ifunit(req->ethname);
		if (ifp->eth_ifp == 0)
			return EINVAL;

		memcpy(ifp->shost, req->shost, 6);
		memcpy(ifp->unique, req->unique, sizeof(req->unique));
		ifp->state = state;
		break;

	case PPPOE_SREQ:
		ifp->state = state;
		memcpy(ifp->dhost, req->dhost, 6);
		break;

	case PPPOE_CONNECTED:
		if (ifp->state != state) {
			/* Save sid for session state */
			ifp->state = state;
			ifp->sid = req->sid;
		}
		break;

	default:
		break;
	}

	/* Check dialup */
	for (ifp = ifpppoe; ifp; ifp = ifp->next) {
		if (ifp->state == PPPOE_SDWAIT) {
			dialup_checkp = ifpppoe_dialup_check;
			break;
		}
	}
	if (ifp == NULL)
		dialup_checkp = 0;

	return 0;
}

/* Do ifpppoe delete */
static int
ifpppoe_del(struct ifpppoe *ifp)
{
	struct ifpppoe *curr, *prev;

	/* Do remove */
	for (prev = 0, curr = ifpppoe; curr; prev = curr, curr = curr->next) {
		/*
		* Dequeue pppoe ifp and close PPP if it is
		* still alive.
		*/
		if (curr == ifp) {
			/* Do del */
			if (prev == 0)
				ifpppoe = curr->next;
			else
				prev->next = curr->next;
			break;
		}
	}

	/* Do clean up */
	if (curr)
		if_detach(&curr->if_ppp.sc_if);
	return 0;
}

/* Add interface pppoe to list */
static struct ifpppoe *
ifpppoe_add(void *devtab, char *pppname)
{
	struct ifpppoe *ifp;
	struct ifppp *ifppp;
	int unit;

	/* Check if this one is used */
	for (ifp = ifpppoe; ifp; ifp = ifp->next) {
		if (strcmp(ifp->if_ppp.pppname, pppname) == 0)
			break;
	}
	if (ifp)
		return ifp;

	if (ifunit(pppname) != 0)
		return NULL;

	/* Allocate ifpppoe control block */
	unit = atoi(pppname+3);
	if (memcmp(pppname, "ppp", 3) != 0 ||
	    unit < 0 || unit > NPPPOE-1) {
		return NULL;
	}

	ifp = &ifpppoectl[unit];
	memset(ifp, 0, sizeof(*ifp));
	ifp->devtab = devtab;

	/* Do prepend */
	ifp->next = ifpppoe;
	ifpppoe = ifp;

	/* Setup ppp structure */
	ifppp = &ifp->if_ppp;
	strcpy(ifppp->pppname, pppname);
	ifppp->sc_unit = unit;
	ifppp->pOutput = ifpppoe_output;

	pppattach(ifppp, unit);
	return ifpppoe;
}

/*
 * The following functions are to support the
 * pppoe_device called with file, pppoe_dev_fd.
 */
static int
pppoewrite(struct ifpppoe *ifp, char *buf, int len)
{
	struct mbuf *m;
	struct ether_header *eh;
	struct sockaddr dst;
	int s;

	/* Set sockaddr to send */
	memset(&dst, 0, sizeof(dst));
	dst.sa_len = 16;
	dst.sa_family = AF_UNSPEC;

	eh = (struct ether_header *)dst.sa_data;
	memcpy(eh, buf, sizeof(*eh));

	buf += sizeof(*eh);
	len -= sizeof(*eh);

	/* generate a packet of that type */
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0)
		return ENOBUFS;

	MCLGET(m, M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) {
		m_freem(m);
		return ENOBUFS;
	}

	/*
	* Reserve room for ether header,
	* we reserve 2 more bytes for
	* alignment, this is good for
	* filters.
	*/
	m->m_data += sizeof(*eh) + 2;
	m->m_len = len;

	memcpy(m->m_data, buf, len);
	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.rcvif = 0;

	/* Raw send */
	s = splimp();
	ether_output(ifp->eth_ifp, m, &dst, 0);
	splx(s);
	return 0;
}

static int
pppoeioctl(struct ifpppoe *ifp, u_long cmd, caddr_t data)
{
	struct ifpppoereq *req = (struct ifpppoereq *)data;
	int error = 0;
	int s = splimp();

	switch (cmd) {
	case PPPIOCSPPPOESTATE:
		ifpppoe_set_state(ifp, req);
		break;

	default:
		senderr(EPFNOSUPPORT);
		break;
	}

quit:
	splx(s);
	return error;
}

/* Declare kdev */
KDEV_NODE(pppoe,
	ifpppoe_add,
	pppoewrite,
	pppoeioctl,
	ifpppoe_del);
