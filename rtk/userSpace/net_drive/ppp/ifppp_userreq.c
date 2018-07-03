/*
 * PPP user request functions
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: ifppp_userreq.c,v 1.8 2010-08-10 10:44:55 Exp $
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/errno.h>
#include <sys/sysctl.h>
#include <net/netisr.h>
#include <net/if_types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <arpa/inet.h>

#include <net/ppp_defs.h>
#include <net/if_ppp.h>
#include <kdev.h>

#ifndef	EINVAL
#define EINVAL -1
#endif

int ppp_ccp_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data);
void ppp_ccp_closed(struct ifppp *ifppp);
struct mbuf *ppp_ccp_pktin(struct ifppp *ifppp, struct mbuf *m);
struct mbuf * ppp_dequeue(struct ifppp *ifppp, struct mbuf *m);

/* IP device filter hook */
int (*ipdev_check_hook)(struct ifnet *ifp, char *head, struct mbuf **m0) __attribute__((weak));


int ppp_num_sessions = CONFIG_PPP_NUM_SESSIONS;

/* Attachs PPP interface to ifnet list */
int
pppattach(struct ifppp *ifppp, int unit)
{
    struct ifnet *ifp = &ifppp->sc_if;
    int s;

    /* Check if this ppp exists */
    if (ifunit(ifppp->pppname) != 0)
	return EALREADY;

    /* Disable PPP_IP */
    ifppp->npmode = NPMODE_DROP;
    ifppp->devtab = 0;

    /* Attach the ppp interface to ifnet */
    sprintf(ifp->if_xname, "ppp%d", unit);
    ifp->if_name = "ppp";
    ifp->if_unit = unit;
    ifp->if_dname = "ppp";
    ifp->if_dunit = unit;

    ifp->if_mtu = PPP_MTU;
    ifp->if_flags = IFF_POINTOPOINT | IFF_MULTICAST;
    ifp->if_ioctl = ifpppioctl;
    ifp->if_output = pppoutput;
    ifp->if_type = IFT_PPP;
    ifp->if_hdrlen = PPP_HDRLEN;

    s = splimp();
    if_attach(ifp);
    splx(s);
	
    return 0;
}

/* Process an ioctl request to interface. */
int
ifpppioctl(ifp, cmd, data)
    struct ifnet *ifp;
    u_long cmd;
    caddr_t data;
{
    struct ifaddr *ifa = (struct ifaddr *)data;
    struct ifreq *ifr = (struct ifreq *)data;
    int error = 0;
    int s = splimp();
    
    switch (cmd) {
    case SIOCSIFFLAGS:
	if (ifp->if_flags & IFF_UP) {
	    ifp->if_flags |= IFF_RUNNING;
        }
	else {
	    ifp->if_flags &= ~IFF_RUNNING;
        }
	break;
		
    case SIOCSIFADDR:
#ifdef INET6
	if ((ifa->ifa_dstaddr->sa_family != AF_INET) &&
		(ifa->ifa_dstaddr->sa_family != AF_INET6))
#else
	if (ifa->ifa_addr->sa_family != AF_INET)
	    error = EAFNOSUPPORT;
#endif
	break;
	
    case SIOCSIFDSTADDR:
#ifdef INET6
	if ((ifa->ifa_dstaddr->sa_family != AF_INET) &&
		(ifa->ifa_dstaddr->sa_family != AF_INET6))
	    error = EAFNOSUPPORT;
#else
	if (ifa->ifa_dstaddr->sa_family != AF_INET)
	    error = EAFNOSUPPORT;
#endif
	break;

    case SIOCADDMULTI:
    case SIOCDELMULTI:
	if (ifr == 0) {
	    error = EAFNOSUPPORT;
	    break;
	}
	switch(ifr->ifr_addr.sa_family) {
#ifdef INET
	case AF_INET:
	    break;
#endif
#ifdef INET6
	case AF_INET6:
	    break;
#endif
	default:
	    error = EAFNOSUPPORT;
	    break;
	}
	break;

    default:
	break;
    }

    splx(s);
    return (error);
}

/*
* Pass the packet to the appropriate stack LCP or IP.
* This routine determines packet type and queues the 
* mbuf to the appropriate queue. This routine returns 
* a value of 1 if the packet type is other than PPP_IP.
*/
int
ppppktin(struct ifppp *ifppp, struct mbuf *m)
{
    struct ifnet *ifp = &ifppp->sc_if;
    int s, ilen, proto;
    u_char *cp, adrs, ctrl;
    struct mbuf *mp;
    struct ifqueue *inq;
	
    ifp->if_ipackets++;
    ifp->if_lastchange = ktime;
	
#ifdef PPP_COMPRESS
    m = ppp_ccp_pktin(ifppp, m);
    if (m == NULL)
	return 0;
#endif

    /* Pull up again */
    if (m->m_len < PPP_HDRLEN && (m = m_pullup(m, PPP_HDRLEN)) == 0)
	return 0;

    for (mp = m, ilen = 0; mp != NULL; mp = mp->m_next)
	ilen += mp->m_len;

    m->m_pkthdr.len = ilen;
    m->m_pkthdr.rcvif = ifp;

    /* Check ALL station and UI (0xff 0x03) */
    cp = mtod(m, u_char *);
    adrs = cp[0];
    ctrl = cp[1];
    if (cp[0] != PPP_ALLSTATIONS || cp[1] != PPP_UI) {
	m_freem(m);
	return 0;
    }

    /*
     * If the packet will fit in a header mbuf, don't waste a
     * whole cluster on it.
     */
    proto = (cp[2] << 8) + cp[3];
    switch (proto) {
    case PPP_IP:
	/* IP packet - take off the ppp header and pass it up to IP. */
	if (ifppp->npmode != NPMODE_PASS ||(ifp->if_flags & IFF_UP) == 0) {
	    /* interface is down - drop the packet. */
	    m_freem(m);
	    return 0;
	}
		
	/* Trim the PPP header from head */
	m_adj(m, PPP_HDRLEN);
//roy add,20101211	
    if (ipdev_check_hook) {
			/* Intercept by ipdev */
			(*ipdev_check_hook)(ifp, (char *)NULL, &m);
			if (m == NULL)
				return 0;
    }
//end	
	
	#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
	#pragma message("defined CONFIG_RTL_FREEBSD_FAST_PATH")
	extern int FastPath_Enter(struct ifnet *ifp, struct mbuf  **pm, struct ether_header *eh);
	if(1 == FastPath_Enter(ifp, &m, NULL)) {
		return 0;
	}
		
	#endif
	
	inq = &ipintrq;
	break;
#ifdef INET6
    case PPP_IPV6:
	if (ifppp->npmode != NPMODE_PASS ||(ifp->if_flags & IFF_UP) == 0) {
	    m_freem(m);
				return 0;
    }
	m_adj(m, PPP_HDRLEN);
	inq = &ip6intrq;
	break;
#endif	
    default:
	if ((ifp->if_flags & IFF_UP) == 0) {
	    /* interface is not ready, drop packet */
	    m_freem(m);
	    return 0;
	}

	/* Send up to kdev */
	kdev_input(ifppp->devtab, m);
	return 0;
    }
 
    /* Insert to queue */
    s = splimp();
    if (IF_QFULL(inq)) {   
	IF_DROP(inq);
	m_freem(m);

	ifp->if_ierrors++;
	ifp->if_iqdrops++;
    }
    else {
	IF_ENQUEUE(inq, m);
    }
    splx(s);

    switch (proto) {
    case PPP_IP:
    schednetisr(NETISR_IP);
    break;
#ifdef INET6
    case PPP_IPV6:
    schednetisr(NETISR_IPV6);
    break;
#endif
    }
    return 0;
}

/*
 * Queue a packet.  Start transmission if not active.
 * Packet is placed in Information field of PPP frame.
 */
int
pppoutput(ifp, m, dst, rt)
    struct ifnet *ifp;
    struct mbuf *m;
    struct sockaddr *dst;
    struct rtentry *rt;
{
    struct ifppp *ifppp = (struct ifppp *)ifp;
    u_char *cp;
    int error = 0;
    int s = splimp();
	
    /* ppp must be up */
    if (((ifp->if_flags & IFF_RUNNING) == 0 ||
	(ifp->if_flags & IFF_UP) == 0) &&
	dst->sa_family != AF_UNSPEC) {
	error = ENETDOWN;
	goto bad;
    }
	
    /*
     * Compute PPP header.
     */
    switch (dst->sa_family) {
    case AF_INET:
	if (ifppp->npmode != NPMODE_PASS)
	    goto bad; 
    
	/*
         * Update the last idle time
         * everytime routing traffic found
         */
	if (m->m_pkthdr.rcvif)
	    ifppp->last_idle_time = ktime.tv_sec;
		
	M_PREPEND(m, PPP_HDRLEN, M_DONTWAIT);
	if (m == NULL) {
	    errno = ENOBUFS;
	    goto bad;
	}
		
	/* Append PPP header */
	cp = mtod(m, u_char *);
	*cp++ = PPP_ALLSTATIONS;
	*cp++ = PPP_UI;
	*cp++ = PPP_IP >> 8;
	*cp++ = PPP_IP & 0xff;
	break;
#ifdef INET6
    case AF_INET6:
	if (m->m_pkthdr.rcvif)
	    ifppp->last_idle_time = ktime.tv_sec;
		
	M_PREPEND(m, PPP_HDRLEN, M_DONTWAIT);
	if (m == NULL) {
	    errno = ENOBUFS;
	    goto bad;
	}
	cp = mtod(m, u_char *);
	*cp++ = PPP_ALLSTATIONS;
	*cp++ = PPP_UI;
	*cp++ = PPP_IPV6 >> 8;
	*cp++ = PPP_IPV6 & 0xff;
	break;
#endif		
    case AF_UNSPEC:
	break;
	
    default:
	error = EAFNOSUPPORT;
	goto bad;
    }
	
#ifdef PPP_COMPRESS
    m = ppp_dequeue(ifppp, m);
    if (m == 0) {
	error = ENOBUFS;
	goto bad;
    }
#endif

    if (ifppp->pOutput) {
	ifp->if_opackets++;
	ifppp->pOutput((struct ifnet *)ifppp, m);

	splx(s);
	return 0;
    }

bad:
    m_freem(m);
	
    errno = error;
    splx(s);
    return (error);
}

/*
 * The following functions are to support the
 * ppp_device called with file descriptor,
 * ppp_dev_fd.
 */
int
pppwrite(struct ifnet *ifp, char *buf, int len)
{
    struct mbuf *m;
    struct sockaddr dst;

    /* Check MTU */	
    if (len > ifp->if_mtu + PPP_HDRLEN ||
	len < PPP_HDRLEN) {
	errno = EMSGSIZE;
	return (-1);
    }
	
    /*
     * Get mbuf to send data out
     */
    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == NULL)
	return 0;
	
    MCLGET(m, M_DONTWAIT);
    if ((m->m_flags & M_EXT) == 0) {
	/*
	 * we couldn't get a cluster - if memory's this
	 * low, it's time to start dropping packets.
	 */
	m_free(m);
	return 0;
    }
	
    /*
     * Zero padding to fit some buggy
     * ppp server.
     */
#ifndef	ETHERMIN
#define	ETHERMIN	60
#endif
    memset(mtod(m, caddr_t), 0, ETHERMIN);
	
    bcopy(buf, mtod(m, caddr_t), len);
    m->m_len = len;
    m->m_pkthdr.len = len;
    m->m_pkthdr.rcvif = ifp;

    /* This is ppp specific protocol */
    dst.sa_family = AF_UNSPEC;
    return pppoutput(ifp, m, &dst, (struct rtentry *)ifp);
}

int
pppioctl(ifp, cmd, data)
    struct ifnet *ifp;
    u_long cmd;
    caddr_t data;
{
    struct ifppp *ifppp = (struct ifppp *)ifp;
    struct npioctl *npi = (struct npioctl *)data;
    struct ppp_idle *idle = (struct ppp_idle *)data;
    int flags;
    int error = 0;

    switch (cmd) {
    case PPPIOCGFLAGS:
	*(int *)data = ifppp->sc_flags;
	break;
    
    case PPPIOCSFLAGS:
	flags = *(int *)data;
#ifdef PPP_COMPRESS
	if ((ifppp->sc_flags & (SC_CCP_OPEN | SC_CCP_UP)) != 0 &&
	    (flags & (SC_CCP_OPEN | SC_CCP_UP)) == 0) {
	    ppp_ccp_closed(ifppp);
	}
#endif	
	ifppp->sc_flags &= ~SC_MASK;
	ifppp->sc_flags |= (flags & SC_MASK);
	break;
    
    case PPPIOCSNPMODE:
	if (npi->protocol == PPP_IP||npi->protocol == PPP_IPV6)
	    ifppp->npmode = npi->mode;
	break;

    case PPPIOCGIDLE:
        idle->xmit_idle = ktime.tv_sec - ifppp->last_idle_time;
        idle->recv_idle = ktime.tv_sec - ifppp->last_idle_time;
	break;

    case PPPIOCGASYNCMAP:
    case PPPIOCSASYNCMAP:
    case PPPIOCGUNIT:
    case PPPIOCGRASYNCMAP:
    case PPPIOCSRASYNCMAP:
    case PPPIOCGMRU:
    case PPPIOCSMAXCID:
	break;

    case PPPIOCSMRU:
	ifp->if_mtu = *(int *)data;
	break;

#ifdef PPP_COMPRESS
    case PPPIOCSCOMPRESS:
	error = ppp_ccp_ioctl(ifp, cmd, data);
	break;
#endif

    default:
	error = EPFNOSUPPORT;
	break;
    }

    return error;
}

/* Return for ppp kdev open function */
static struct ifnet *
ppp_open(void *devtab, char *pathname)
{
	struct ifppp *ifp;

	ifp = (struct ifppp *)ifunit(pathname);
	if (ifp == NULL)
		return NULL;

	/* It should be fine.  The out layer used count
	 * can handle multiple opens.
	 */
	if (ifp->devtab == 0)
		ifp->devtab = devtab;

	return (struct ifnet *)ifp;
}

/* ppp kdev close */
static int
ppp_close(struct ifppp *ifp)
{
	ifp->devtab = 0;

	return 0;
}

/* Declare kdev */
KDEV_NODE(ppp,
	ppp_open,
	pppwrite,
	pppioctl,
	ppp_close);
