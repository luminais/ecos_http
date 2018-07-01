//==========================================================================
//
//      src/sys/netinet/ip_mroute.c
//
//==========================================================================
// ####BSDCOPYRIGHTBEGIN####                                    
// -------------------------------------------                  
// This file is part of eCos, the Embedded Configurable Operating System.
//
// Portions of this software may have been derived from FreeBSD 
// or other sources, and if so are covered by the appropriate copyright
// and license included herein.                                 
//
// Portions created by the Free Software Foundation are         
// Copyright (C) 2002 Free Software Foundation, Inc.            
// -------------------------------------------                  
// ####BSDCOPYRIGHTEND####                                      
//==========================================================================

/*
 * IP multicast forwarding procedures
 *
 * Written by David Waitzman, BBN Labs, August 1988.
 * Modified by Steve Deering, Stanford, February 1989.
 * Modified by Mark J. Steiglitz, Stanford, May, 1991
 * Modified by Van Jacobson, LBL, January 1993
 * Modified by Ajit Thyagarajan, PARC, August 1993
 * Modified by Bill Fenner, PARC, April 1995
 *
 * MROUTING Revision: 3.5
 * $FreeBSD: src/sys/netinet/ip_mroute.c,v 1.56.2.2 2001/07/19 06:37:26 kris Exp $
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in_var.h>
#include <netinet/igmp.h>
#include <netinet/ip_mroute.h>
#include <netinet/udp.h>
#include <stdio.h>


#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
#include <rtl/rtl865x_eventMgr.h>

typedef struct rtl_multicastEventContext_s
{
	char devName[16];
	unsigned int  moduleIndex;
	unsigned int ipVersion;
	unsigned int groupAddr[4];
	unsigned int sourceAddr[4];
	unsigned int portMask;
}rtl_multicastEventContext_t;
#endif

#ifdef CONFIG_RTL_8196D
int rtl865x_checkMfcCache(unsigned long origin,unsigned long mcastgrp)
{
	return 0;
}
#endif

#ifndef NTOHL
#if BYTE_ORDER != BIG_ENDIAN
#define NTOHL(d) ((d) = ntohl((d)))
#define NTOHS(d) ((d) = ntohs((u_short)(d)))
#define HTONL(d) ((d) = htonl((d)))
#define HTONS(d) ((d) = htons((u_short)(d)))
#else
#define NTOHL(d)
#define NTOHS(d)
#define HTONL(d)
#define HTONS(d)
#endif
#endif


#ifndef MROUTING
extern u_long	_ip_mcast_src __P((int vifi));
extern int	_ip_mforward __P((struct ip *ip, struct ifnet *ifp,
				  struct mbuf *m, struct ip_moptions *imo));
extern int	_ip_mrouter_done __P((void));
extern int	_ip_mrouter_get __P((struct socket *so, struct sockopt *sopt));
extern int	_ip_mrouter_set __P((struct socket *so, struct sockopt *sopt));
extern int	_mrt_ioctl __P((int req, caddr_t data, struct proc *p));

/*
 * Dummy routines and globals used when multicast routing is not compiled in.
 */

struct socket  *ip_mrouter  = NULL;
u_int		rsvpdebug = 0;

int
_ip_mrouter_set(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
{
	return(EOPNOTSUPP);
}

int (*ip_mrouter_set)(struct socket *, struct sockopt *) = _ip_mrouter_set;


int
_ip_mrouter_get(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
{
	return(EOPNOTSUPP);
}

int (*ip_mrouter_get)(struct socket *, struct sockopt *) = _ip_mrouter_get;

int
_ip_mrouter_done()
{
	return(0);
}

int (*ip_mrouter_done)(void) = _ip_mrouter_done;

int
_ip_mforward(ip, ifp, m, imo)
	struct ip *ip;
	struct ifnet *ifp;
	struct mbuf *m;
	struct ip_moptions *imo;
{
	return(0);
}

int (*ip_mforward)(struct ip *, struct ifnet *, struct mbuf *,
		   struct ip_moptions *) = _ip_mforward;

int
_mrt_ioctl(int req, caddr_t data, struct proc *p)
{
	return EOPNOTSUPP;
}

int (*mrt_ioctl)(int, caddr_t, struct proc *) = _mrt_ioctl;

void
rsvp_input(m, off)		/* XXX must fixup manually */
	struct mbuf *m;
	int off;
{
    /* Can still get packets with rsvp_on = 0 if there is a local member
     * of the group to which the RSVP packet is addressed.  But in this
     * case we want to throw the packet away.
     */
    if (!rsvp_on) {
	m_freem(m);
	return;
    }
 
    if (ip_rsvpd != NULL) {
	if (rsvpdebug)
	    printf("rsvp_input: Sending packet up old-style socket\n");
	rip_input(m, off);
	return;
    }
    /* Drop the packet */
    m_freem(m);
}

void ipip_input(struct mbuf *m, int off) { /* XXX must fixup manually */
	rip_input(m, off);
}

int (*legal_vif_num)(int) = 0;

/*
 * This should never be called, since IP_MULTICAST_VIF should fail, but
 * just in case it does get called, the code a little lower in ip_output
 * will assign the packet a local address.
 */
u_long
_ip_mcast_src(int vifi) { return INADDR_ANY; }
u_long (*ip_mcast_src)(int) = _ip_mcast_src;

int
ip_rsvp_vif_init(so, sopt)
    struct socket *so;
    struct sockopt *sopt;
{
    return(EINVAL);
}

int
ip_rsvp_vif_done(so, sopt)
    struct socket *so;
    struct sockopt *sopt;
{
    return(EINVAL);
}

void
ip_rsvp_force_done(so)
    struct socket *so;
{
    return;
}

#else /* MROUTING */

#define M_HASCL(m)	((m)->m_flags & M_EXT)

#define INSIZ		sizeof(struct in_addr)
#define	same(a1, a2) \
	(bcmp((caddr_t)(a1), (caddr_t)(a2), INSIZ) == 0)

/*
 * Globals.  All but ip_mrouter and ip_mrtproto could be static,
 * except for netstat or debugging purposes.
 */
#ifndef MROUTE_LKM
struct socket  *ip_mrouter  = NULL;
static struct mrtstat	mrtstat;
#else /* MROUTE_LKM */
extern void	X_ipip_input __P((struct mbuf *m, int iphlen));
extern struct mrtstat mrtstat;
static int ip_mrtproto;
#endif

#define NO_RTE_FOUND 	0x1
#define RTE_FOUND	0x2

//static struct mfc	*mfctable[MFCTBLSIZ];
struct mfc	*mfctable[MFCTBLSIZ];		//jwj
static u_char		nexpire[MFCTBLSIZ];
static struct vif	viftable[MAXVIFS];
static u_int	mrtdebug = 0;	  /* debug level 	*/
#define		DEBUG_MFC	0x02
#define		DEBUG_FORWARD	0x04
#define		DEBUG_EXPIRE	0x08
#define		DEBUG_XMIT	0x10
static u_int  	tbfdebug = 0;     /* tbf debug level 	*/
static u_int	rsvpdebug = 0;	  /* rsvp debug level   */

//static struct callout_handle expire_upcalls_ch;
#ifdef __NetBSD__
static struct callout expire_upcalls_ch = CALLOUT_INITIALIZER;
#elif (defined(__FreeBSD__) && __FreeBSD__ >= 3)
static struct callout expire_upcalls_ch;
#elif defined(__OpenBSD__)
static struct timeout expire_upcalls_ch;
#endif


#define		EXPIRE_TIMEOUT	(hz / 4)	/* 4x / second		*/
#define		UPCALL_EXPIRE	6		/* number of timeouts	*/

/*
 * Define the token bucket filter structures
 * tbftable -> each vif has one of these for storing info 
 */

static struct tbf tbftable[MAXVIFS];
#define		TBF_REPROCESS	(hz / 100)	/* 100x / second */

/*
 * 'Interfaces' associated with decapsulator (so we can tell
 * packets that went through it from ones that get reflected
 * by a broken gateway).  These interfaces are never linked into
 * the system ifnet list & no routes point to them.  I.e., packets
 * can't be sent this way.  They only exist as a placeholder for
 * multicast source verification.
 */
static struct ifnet multicast_decap_if[MAXVIFS];

#define ENCAP_TTL 64
#define ENCAP_PROTO IPPROTO_IPIP	/* 4 */

/* prototype IP hdr for encapsulated packets */
static struct ip multicast_encap_iphdr = {
#if BYTE_ORDER == LITTLE_ENDIAN
	sizeof(struct ip) >> 2, IPVERSION,
#else
	IPVERSION, sizeof(struct ip) >> 2,
#endif
	0,				/* tos */
	sizeof(struct ip),		/* total length */
	0,				/* id */
	0,				/* frag offset */
	ENCAP_TTL, ENCAP_PROTO,	
	0,				/* checksum */
};

/*
 * Private variables.
 */
static vifi_t	   numvifs = 0;
static int have_encap_tunnel = 0;

/*
 * one-back cache used by ipip_input to locate a tunnel's vif
 * given a datagram's src ip address.
 */
static u_long last_encap_src;
static struct vif *last_encap_vif;

static u_long	X_ip_mcast_src __P((int vifi));
static int	X_ip_mforward __P((struct ip *ip, struct ifnet *ifp, struct mbuf *m, struct ip_moptions *imo));
static int	X_ip_mrouter_done __P((void));
static int	X_ip_mrouter_get __P((struct socket *so, struct sockopt *m));
static int	X_ip_mrouter_set __P((struct socket *so, struct sockopt *m));
static int	X_legal_vif_num __P((int vif));
static int	X_mrt_ioctl __P((int cmd, caddr_t data));

static int get_sg_cnt(struct sioc_sg_req *);
static int get_vif_cnt(struct sioc_vif_req *);
static int ip_mrouter_init(struct socket *, int);
static int add_vif(struct vifctl *);
static int del_vif(vifi_t);
static int add_mfc(struct mfcctl *);
static int del_mfc(struct mfcctl *);
static int socket_send(struct socket *, struct mbuf *, struct sockaddr_in *);
static int set_assert(int);
static void expire_upcalls(void *);
static int ip_mdq(struct mbuf *, struct ifnet *, struct mfc *,
		  vifi_t);
static void phyint_send(struct ip *, struct vif *, struct mbuf *);
static void encap_send(struct ip *, struct vif *, struct mbuf *);
static void tbf_control(struct vif *, struct mbuf *, struct ip *, u_long);
static void tbf_queue(struct vif *, struct mbuf *);
static void tbf_process_q(struct vif *);
static void tbf_reprocess_q(void *);
static int tbf_dq_sel(struct vif *, struct ip *);
static void tbf_send_packet(struct vif *, struct mbuf *);
static void tbf_update_tokens(struct vif *);
static int priority(struct vif *, struct ip *);
void multiencap_decap(struct mbuf *);
#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
static int ip_mroute_init_vif(char * wan_if,char *lan_if );
extern void igmp_packet_send(int type,struct ifnet *ifp,in_addr_t igmp_group);
#endif

/*
 * whether or not special PIM assert processing is enabled.
 */
static int pim_assert;
/*
 * Rate limit for assert notification messages, in usec
 */
#define ASSERT_MSG_TIME		3000000

/*
 * Hash function for a source, group entry
 */
#define MFCHASH(a, g) MFCHASHMOD(((a) >> 20) ^ ((a) >> 10) ^ (a) ^ \
			((g) >> 20) ^ ((g) >> 10) ^ (g))

/*
 * Find a route for a given origin IP address and Multicast group address
 * Type of service parameter to be added in the future!!!
 */

#define MFCFIND(o, g, rt) { \
	register struct mfc *_rt = mfctable[MFCHASH(o,g)]; \
	rt = NULL; \
	++mrtstat.mrts_mfc_lookups; \
	while (_rt) { \
		if ((_rt->mfc_origin.s_addr == o) && \
		    (_rt->mfc_mcastgrp.s_addr == g) && \
		    (_rt->mfc_stall == NULL)) { \
			rt = _rt; \
			break; \
		} \
		_rt = _rt->mfc_next; \
	} \
	if (rt == NULL) { \
		++mrtstat.mrts_mfc_misses; \
	} \
}


/*
 * Macros to compute elapsed time efficiently
 * Borrowed from Van Jacobson's scheduling code
 */
#define TV_DELTA(a, b, delta) { \
	    register int xxs; \
		\
	    delta = (a).tv_usec - (b).tv_usec; \
	    if ((xxs = (a).tv_sec - (b).tv_sec)) { \
	       switch (xxs) { \
		      case 2: \
			  delta += 1000000; \
			      /* fall through */ \
		      case 1: \
			  delta += 1000000; \
			  break; \
		      default: \
			  delta += (1000000 * xxs); \
	       } \
	    } \
}

#define TV_LT(a, b) (((a).tv_usec < (b).tv_usec && \
	      (a).tv_sec <= (b).tv_sec) || (a).tv_sec < (b).tv_sec)

#ifdef UPCALL_TIMING
u_long upcall_data[51];
static void collate(struct timeval *);
#endif /* UPCALL_TIMING */


/*
 * Handle MRT setsockopt commands to modify the multicast routing tables.
 */
static int
X_ip_mrouter_set(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
{
	int	error, optval;
	vifi_t	vifi;
	struct	vifctl vifc;
	struct	mfcctl mfc;

	if (so != ip_mrouter && sopt->sopt_name != MRT_INIT)
		return (EPERM);

	error = 0;
	switch (sopt->sopt_name) {
	case MRT_INIT:
		error = sooptcopyin(sopt, &optval, sizeof optval, 
				    sizeof optval);
		if (error)
			break;
		error = ip_mrouter_init(so, optval);
		break;

	case MRT_DONE:
		error = ip_mrouter_done();
		break;

	case MRT_ADD_VIF:
		error = sooptcopyin(sopt, &vifc, sizeof vifc, sizeof vifc);
		if (error)
			break;
		
		//diag_printf("MRT_ADD_VIF.vif:%d,%x,%x.[%s]:[%d].\n",vifc.vifc_vifi,vifc.vifc_flags,vifc.vifc_lcl_addr.s_addr,__FUNCTION__,__LINE__);
		error = add_vif(&vifc);
		break;

	case MRT_DEL_VIF:
		error = sooptcopyin(sopt, &vifi, sizeof vifi, sizeof vifi);
		if (error)
			break;
		//diag_printf("MRT_DEL_VIF.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		error = del_vif(vifi);
		break;

	case MRT_ADD_MFC:
	case MRT_DEL_MFC:
		error = sooptcopyin(sopt, &mfc, sizeof mfc, sizeof mfc);
		if (error)
			break;
		//diag_printf("MRT_ADD/DEL_MFC.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		if (sopt->sopt_name == MRT_ADD_MFC)
			error = add_mfc(&mfc);
		else
			error = del_mfc(&mfc);
		break;

	case MRT_ASSERT:
		error = sooptcopyin(sopt, &optval, sizeof optval, 
				    sizeof optval);
		if (error)
			break;
		set_assert(optval);
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}

#ifndef MROUTE_LKM
int (*ip_mrouter_set)(struct socket *, struct sockopt *) = X_ip_mrouter_set;
#endif

/*
 * Handle MRT getsockopt commands
 */
static int
X_ip_mrouter_get(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
{
	int error;
	static int version = 0x0305; /* !!! why is this here? XXX */

	switch (sopt->sopt_name) {
	case MRT_VERSION:
		error = sooptcopyout(sopt, &version, sizeof version);
		break;

	case MRT_ASSERT:
		error = sooptcopyout(sopt, &pim_assert, sizeof pim_assert);
		break;
	default:
		error = EOPNOTSUPP;
		break;
	}
	return (error);
}

#ifndef MROUTE_LKM
int (*ip_mrouter_get)(struct socket *, struct sockopt *) = X_ip_mrouter_get;
#endif

/*
 * Handle ioctl commands to obtain information from the cache
 */
static int
X_mrt_ioctl(cmd, data)
    int cmd;
    caddr_t data;
{
    int error = 0;

    switch (cmd) {
	case (SIOCGETVIFCNT):
	    return (get_vif_cnt((struct sioc_vif_req *)data));
	    break;
	case (SIOCGETSGCNT):
	    return (get_sg_cnt((struct sioc_sg_req *)data));
	    break;
	default:
	    return (EINVAL);
	    break;
    }
    return error;
}

#ifndef MROUTE_LKM
int (*mrt_ioctl)(int, caddr_t) = X_mrt_ioctl;
#endif

/*
 * returns the packet, byte, rpf-failure count for the source group provided
 */
static int
get_sg_cnt(req)
    register struct sioc_sg_req *req;
{
    register struct mfc *rt;
    int s;

    s = splnet();
    MFCFIND(req->src.s_addr, req->grp.s_addr, rt);
    splx(s);
    if (rt != NULL) {
	req->pktcnt = rt->mfc_pkt_cnt;
	req->bytecnt = rt->mfc_byte_cnt;
	req->wrong_if = rt->mfc_wrong_if;
    } else
	req->pktcnt = req->bytecnt = req->wrong_if = 0xffffffff;

    return 0;
}

/*
 * returns the input and output packet and byte counts on the vif provided
 */
static int
get_vif_cnt(req)
    register struct sioc_vif_req *req;
{
    register vifi_t vifi = req->vifi;

    if (vifi >= numvifs) return EINVAL;

    req->icount = viftable[vifi].v_pkt_in;
    req->ocount = viftable[vifi].v_pkt_out;
    req->ibytes = viftable[vifi].v_bytes_in;
    req->obytes = viftable[vifi].v_bytes_out;

    return 0;
}

/*
 * Enable multicast routing
 */
static int
ip_mrouter_init(so, version)
	struct socket *so;
	int version;
{
    if (mrtdebug)
	log(LOG_DEBUG,"ip_mrouter_init: so_type = %d, pr_protocol = %d\n",
		so->so_type, so->so_proto->pr_protocol);

    if (so->so_type != SOCK_RAW ||
	so->so_proto->pr_protocol != IPPROTO_IGMP) return EOPNOTSUPP;

    if (version != 1)
	return ENOPROTOOPT;

    if (ip_mrouter != NULL) return EADDRINUSE;

    ip_mrouter = so;

    bzero((caddr_t)mfctable, sizeof(mfctable));
    bzero((caddr_t)nexpire, sizeof(nexpire));

    pim_assert = 0;

    //expire_upcalls_ch = timeout(expire_upcalls, (caddr_t)NULL, EXPIRE_TIMEOUT);
	
#if defined(__NetBSD__) || (defined(__FreeBSD__) && __FreeBSD__ >= 3)
		callout_reset(&expire_upcalls_ch, EXPIRE_TIMEOUT,
			expire_upcalls, NULL);
#elif defined(__OpenBSD__)
		timeout_set(&expire_upcalls_ch, expire_upcalls, NULL);
		timeout_add(&expire_upcalls_ch, EXPIRE_TIMEOUT);
#else
		timeout(expire_upcalls, (caddr_t)NULL, EXPIRE_TIMEOUT);
#endif

    if (mrtdebug)
	log(LOG_DEBUG, "ip_mrouter_init\n");

    return 0;
}

/*
 * Disable multicast routing
 */
static int
X_ip_mrouter_done()
{
    vifi_t vifi;
    int i;
    struct ifnet *ifp;
    struct ifreq ifr;
    struct mfc *rt;
    struct rtdetq *rte;
    int s;

    s = splnet();

    /*
     * For each phyint in use, disable promiscuous reception of all IP
     * multicasts.
     */
    for (vifi = 0; vifi < numvifs; vifi++) {
	if (viftable[vifi].v_lcl_addr.s_addr != 0 &&
	    !(viftable[vifi].v_flags & VIFF_TUNNEL)) {
	    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
	    ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr
								= INADDR_ANY;
	    ifp = viftable[vifi].v_ifp;
	    if_allmulti(ifp, 0);
	}
    }
    bzero((caddr_t)tbftable, sizeof(tbftable));
    bzero((caddr_t)viftable, sizeof(viftable));
    numvifs = 0;
    pim_assert = 0;

  // untimeout(expire_upcalls, (caddr_t)NULL, expire_upcalls_ch);
   
#if defined(__NetBSD__) || (defined(__FreeBSD__) && __FreeBSD__ >= 3)
	   callout_stop(&expire_upcalls_ch);
#elif defined(__OpenBSD__)
	   timeout_del(&expire_upcalls_ch);
#else
	   untimeout(expire_upcalls, (caddr_t)NULL);
#endif
   

    /*
     * Free all multicast forwarding cache entries.
     */
    for (i = 0; i < MFCTBLSIZ; i++) {
	for (rt = mfctable[i]; rt != NULL; ) {
	    struct mfc *nr = rt->mfc_next;

	    for (rte = rt->mfc_stall; rte != NULL; ) {
		struct rtdetq *n = rte->next;

		m_freem(rte->m);
		free(rte, M_MRTABLE);
		rte = n;
	    }
	    free(rt, M_MRTABLE);
	    rt = nr;
	}
    }

    bzero((caddr_t)mfctable, sizeof(mfctable));

    /*
     * Reset de-encapsulation cache
     */
    last_encap_src = 0;
    last_encap_vif = NULL;
    have_encap_tunnel = 0;
 
    ip_mrouter = NULL;

    splx(s);

    if (mrtdebug)
	log(LOG_DEBUG, "ip_mrouter_done\n");

    return 0;
}

#ifndef MROUTE_LKM
int (*ip_mrouter_done)(void) = X_ip_mrouter_done;
#endif

/*
 * Set PIM assert processing global
 */
static int
set_assert(i)
	int i;
{
    if ((i != 1) && (i != 0))
	return EINVAL;

    pim_assert = i;

    return 0;
}

/*
 * Add a vif to the vif table
 */
static int
add_vif(vifcp)
    register struct vifctl *vifcp;
{
    register struct vif *vifp = viftable + vifcp->vifc_vifi;
    static struct sockaddr_in sin = {sizeof sin, AF_INET};
    struct ifaddr *ifa;
    struct ifnet *ifp;
    int error, s;
    struct tbf *v_tbf = tbftable + vifcp->vifc_vifi;

    if (vifcp->vifc_vifi >= MAXVIFS)  return EINVAL;
    if (vifp->v_lcl_addr.s_addr != 0) return EADDRINUSE;

    /* Find the interface with an address in AF_INET family */
    sin.sin_addr = vifcp->vifc_lcl_addr;
    ifa = ifa_ifwithaddr((struct sockaddr *)&sin);
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
   	if (ifa == 0)
	{
		ifa = ifa_ifwithsrcaddr((struct sockaddr *)&sin);
	}	
#endif	
    if (ifa == 0) return EADDRNOTAVAIL;
    ifp = ifa->ifa_ifp;

    if (vifcp->vifc_flags & VIFF_TUNNEL) {
	if ((vifcp->vifc_flags & VIFF_SRCRT) == 0) {
		/*
		 * An encapsulating tunnel is wanted.  Tell ipip_input() to
		 * start paying attention to encapsulated packets.
		 */
		if (have_encap_tunnel == 0) {
			have_encap_tunnel = 1;
			for (s = 0; s < MAXVIFS; ++s) {
				multicast_decap_if[s].if_name = "mdecap";
				multicast_decap_if[s].if_unit = s;
			}
		}
		/*
		 * Set interface to fake encapsulator interface
		 */
		ifp = &multicast_decap_if[vifcp->vifc_vifi];
		/*
		 * Prepare cached route entry
		 */
		bzero(&vifp->v_route, sizeof(vifp->v_route));
	} else {
	    log(LOG_ERR, "source routed tunnels not supported\n");
	    return EOPNOTSUPP;
	}
    } else {
	/* Make sure the interface supports multicast */
	if ((ifp->if_flags & IFF_MULTICAST) == 0)
	    return EOPNOTSUPP;

	/* Enable promiscuous reception of all IP multicasts from the if */
	s = splnet();
	error = if_allmulti(ifp, 1);
	splx(s);
	if (error)
	    return error;
    }

    s = splnet();
    /* define parameters for the tbf structure */
    vifp->v_tbf = v_tbf;
    GET_TIME(vifp->v_tbf->tbf_last_pkt_t);
    vifp->v_tbf->tbf_n_tok = 0;
    vifp->v_tbf->tbf_q_len = 0;
    vifp->v_tbf->tbf_max_q_len = MAXQSIZE;
    vifp->v_tbf->tbf_q = vifp->v_tbf->tbf_t = NULL;

    vifp->v_flags     = vifcp->vifc_flags;
    vifp->v_threshold = vifcp->vifc_threshold;
    vifp->v_lcl_addr  = vifcp->vifc_lcl_addr;
    vifp->v_rmt_addr  = vifcp->vifc_rmt_addr;
    vifp->v_ifp       = ifp;
    /* scaling up here allows division by 1024 in critical code */
    vifp->v_rate_limit= vifcp->vifc_rate_limit * 1024 / 1000;
    vifp->v_rsvp_on   = 0;
    vifp->v_rsvpd     = NULL;
    /* initialize per vif pkt counters */
    vifp->v_pkt_in    = 0;
    vifp->v_pkt_out   = 0;
    vifp->v_bytes_in  = 0;
    vifp->v_bytes_out = 0;
    splx(s);

    /* Adjust numvifs up if the vifi is higher than numvifs */
    if (numvifs <= vifcp->vifc_vifi) numvifs = vifcp->vifc_vifi + 1;

    if (mrtdebug)
	log(LOG_DEBUG, "add_vif #%d, lcladdr %lx, %s %lx, thresh %x, rate %d\n",
	    vifcp->vifc_vifi, 
	    (u_long)ntohl(vifcp->vifc_lcl_addr.s_addr),
	    (vifcp->vifc_flags & VIFF_TUNNEL) ? "rmtaddr" : "mask",
	    (u_long)ntohl(vifcp->vifc_rmt_addr.s_addr),
	    vifcp->vifc_threshold,
	    vifcp->vifc_rate_limit);    

    return 0;
}

/*
 * Delete a vif from the vif table
 */
static int
del_vif(vifi)
	vifi_t vifi;
{
    register struct vif *vifp = &viftable[vifi];
    register struct mbuf *m;
    struct ifnet *ifp;
    struct ifreq ifr;
    int s;

    if (vifi >= numvifs) return EINVAL;
    if (vifp->v_lcl_addr.s_addr == 0) return EADDRNOTAVAIL;

    s = splnet();

    if (!(vifp->v_flags & VIFF_TUNNEL)) {
	((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
	((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr = INADDR_ANY;
	ifp = vifp->v_ifp;
	if_allmulti(ifp, 0);
    }

    if (vifp == last_encap_vif) {
	last_encap_vif = 0;
	last_encap_src = 0;
    }

    /*
     * Free packets queued at the interface
     */
    while (vifp->v_tbf->tbf_q) {
	m = vifp->v_tbf->tbf_q;
	vifp->v_tbf->tbf_q = m->m_act;
	m_freem(m);
    }

    bzero((caddr_t)vifp->v_tbf, sizeof(*(vifp->v_tbf)));
    bzero((caddr_t)vifp, sizeof (*vifp));

    if (mrtdebug)
      log(LOG_DEBUG, "del_vif %d, numvifs %d\n", vifi, numvifs);

    /* Adjust numvifs down */
    for (vifi = numvifs; vifi > 0; vifi--)
	if (viftable[vifi-1].v_lcl_addr.s_addr != 0) break;
    numvifs = vifi;

    splx(s);

    return 0;
}
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
#define	VIFF_LOCAL		0x400000	/* vif local flag*/

void show_ip_mr_vif(void)
{
	int vifindex;
	int localFlag=0;
	int remoteFlag=0;
	register struct vif *vifp = viftable ;

	diag_printf("Interface\t\t\t\tBytesIn\tPktsIn\tByteOut\tPktsOut\tFlags\tLocal\tRemote\n");
	for(vifindex=0;vifindex<numvifs;vifindex++)
	{
		vifp = viftable+vifindex;
		if(vifp){
			localFlag =((vifp->v_flags)&VIFF_LOCAL)? 1:0;
			remoteFlag =((vifp->v_flags)&VIFF_LOCAL)? 0:1;
			
			if(vifp->v_ifp)
				diag_printf("%x\t%s%d\t%8x\t%d\t%d\t%d\t%d\t%x\t%d\t%d\n",vifindex,vifp->v_ifp->if_name,vifp->v_ifp->if_unit,vifp->v_lcl_addr.s_addr,
				vifp->v_bytes_in,vifp->v_pkt_in,vifp->v_bytes_out,vifp->v_pkt_out,vifp->v_flags,localFlag,remoteFlag);
			else
			{
				diag_printf("%x\t%s%d\t%8x\t%d\t%d\t%d\t%d\t%x\t%d\t%d\n",vifindex," ",-1,vifp->v_lcl_addr.s_addr,
				vifp->v_bytes_in,vifp->v_pkt_in,vifp->v_bytes_out,vifp->v_pkt_out,vifp->v_flags,localFlag,remoteFlag);
			
			}
		
		}
	}
}
void show_ip_mr_cache(void)
{
	struct in_addr 	origin;
    struct in_addr 	mcastgrp;
	struct mfc	 	**nptr;
	
	struct mfc 		*rt;
	int i;
	
	diag_printf("Group\t\tOrigin\tIif\tPkts\tBytes\tWrong\n");
	for(i=0;i<MFCTBLSIZ;i++){
		nptr = &mfctable[i];
	 	while ((rt = *nptr) != NULL) {
			origin=rt->mfc_origin;
			mcastgrp=rt->mfc_mcastgrp;
			diag_printf("%8x\t%x\t%x\t%d\t%d\t%d\n",mcastgrp.s_addr,origin.s_addr,rt->mfc_parent,rt->mfc_pkt_cnt,rt->mfc_byte_cnt,rt->mfc_wrong_if);
			nptr = &rt->mfc_next;
	    }
	}
}
#if 0
void dump_ip_mr_cache(void)
{

	struct in_addr 	origin;
    struct in_addr 	mcastgrp;
	struct mfc	 	**nptr;
	
	struct mfc 		*rt;
	FILE* fp;
	
	int i;
	fp = fopen("/tmp/ip_mr_cache","w");
	if(fp == NULL){
		return;
	}
	
	
	fprintf(fp,"Group    Origin   Iif     Pkts    Bytes    Wrong  Oifs\n");
	for(i=0;i<MFCTBLSIZ;i++){
		nptr = &mfctable[i];
	 	while ((rt = *nptr) != NULL) {
			origin=rt->mfc_origin;
			mcastgrp=rt->mfc_mcastgrp;
			fprintf(fp,"%x %x %x %d %d %d\n",mcastgrp.s_addr,origin.s_addr,rt->mfc_parent,
				rt->mfc_pkt_cnt,rt->mfc_byte_cnt,rt->mfc_wrong_if);
			nptr = &rt->mfc_next;
	    }
	}
	fclose(fp);
}
#endif
#endif
/*
 * Add an mfc entry
 */
static int
add_mfc(mfccp)
    struct mfcctl *mfccp;
{
    struct mfc *rt;
    u_long hash;
    struct rtdetq *rte;
    register u_short nstl;
    int s;
    int i;
	//diag_printf("origin:%x,group:%x.[%s]:[%d].\n",
	//mfccp->mfcc_origin.s_addr,mfccp->mfcc_mcastgrp.s_addr,__FUNCTION__,__LINE__);
	
	MFCFIND(mfccp->mfcc_origin.s_addr, mfccp->mfcc_mcastgrp.s_addr, rt);

    /* If an entry already exists, just update the fields */
    if (rt) {
		//diag_printf("find mfc.[%s]:[%d].\n",__FUNCTION__,__LINE__);
	if (mrtdebug & DEBUG_MFC)
	    log(LOG_DEBUG,"add_mfc update o %lx g %lx p %x\n",
		(u_long)ntohl(mfccp->mfcc_origin.s_addr),
		(u_long)ntohl(mfccp->mfcc_mcastgrp.s_addr),
		mfccp->mfcc_parent);

	s = splnet();
	rt->mfc_parent = mfccp->mfcc_parent;
	for (i = 0; i < numvifs; i++)
	    rt->mfc_ttls[i] = mfccp->mfcc_ttls[i];
	splx(s);
	return 0;
    }

    /* 
     * Find the entry for which the upcall was made and update
     */
    s = splnet();
    hash = MFCHASH(mfccp->mfcc_origin.s_addr, mfccp->mfcc_mcastgrp.s_addr);
    for (rt = mfctable[hash], nstl = 0; rt; rt = rt->mfc_next) {

	if ((rt->mfc_origin.s_addr == mfccp->mfcc_origin.s_addr) &&
	    (rt->mfc_mcastgrp.s_addr == mfccp->mfcc_mcastgrp.s_addr) &&
	    (rt->mfc_stall != NULL)) {
  
	    if (nstl++)
		log(LOG_ERR, "add_mfc %s o %lx g %lx p %x dbx %p\n",
		    "multiple kernel entries",
		    (u_long)ntohl(mfccp->mfcc_origin.s_addr),
		    (u_long)ntohl(mfccp->mfcc_mcastgrp.s_addr),
		    mfccp->mfcc_parent, (void *)rt->mfc_stall);

	    if (mrtdebug & DEBUG_MFC)
		log(LOG_DEBUG,"add_mfc o %lx g %lx p %x dbg %p\n",
		    (u_long)ntohl(mfccp->mfcc_origin.s_addr),
		    (u_long)ntohl(mfccp->mfcc_mcastgrp.s_addr),
		    mfccp->mfcc_parent, (void *)rt->mfc_stall);

	    rt->mfc_origin     = mfccp->mfcc_origin;
	    rt->mfc_mcastgrp   = mfccp->mfcc_mcastgrp;
	    rt->mfc_parent     = mfccp->mfcc_parent;
	    for (i = 0; i < numvifs; i++)
		rt->mfc_ttls[i] = mfccp->mfcc_ttls[i];
	    /* initialize pkt counters per src-grp */
	    rt->mfc_pkt_cnt    = 0;
	    rt->mfc_byte_cnt   = 0;
	    rt->mfc_wrong_if   = 0;
	    rt->mfc_last_assert.tv_sec = rt->mfc_last_assert.tv_usec = 0;

	    rt->mfc_expire = 0;	/* Don't clean this guy up */
	    nexpire[hash]--;

	    /* free packets Qed at the end of this entry */
	    for (rte = rt->mfc_stall; rte != NULL; ) {
		struct rtdetq *n = rte->next;

		ip_mdq(rte->m, rte->ifp, rt, -1);
		m_freem(rte->m);
#ifdef UPCALL_TIMING
		collate(&(rte->t));
#endif /* UPCALL_TIMING */
		free(rte, M_MRTABLE);
		rte = n;
	    }
	    rt->mfc_stall = NULL;
	}
    }

    /*
     * It is possible that an entry is being inserted without an upcall
     */
    if (nstl == 0) {
	if (mrtdebug & DEBUG_MFC)
	    log(LOG_DEBUG,"add_mfc no upcall h %lu o %lx g %lx p %x\n",
		hash, (u_long)ntohl(mfccp->mfcc_origin.s_addr),
		(u_long)ntohl(mfccp->mfcc_mcastgrp.s_addr),
		mfccp->mfcc_parent);
	
	for (rt = mfctable[hash]; rt != NULL; rt = rt->mfc_next) {
	    
	    if ((rt->mfc_origin.s_addr == mfccp->mfcc_origin.s_addr) &&
		(rt->mfc_mcastgrp.s_addr == mfccp->mfcc_mcastgrp.s_addr)) {

		rt->mfc_origin     = mfccp->mfcc_origin;
		rt->mfc_mcastgrp   = mfccp->mfcc_mcastgrp;
		rt->mfc_parent     = mfccp->mfcc_parent;
		for (i = 0; i < numvifs; i++)
		    rt->mfc_ttls[i] = mfccp->mfcc_ttls[i];
		/* initialize pkt counters per src-grp */
		rt->mfc_pkt_cnt    = 0;
		rt->mfc_byte_cnt   = 0;
		rt->mfc_wrong_if   = 0;
		rt->mfc_last_assert.tv_sec = rt->mfc_last_assert.tv_usec = 0;
		if (rt->mfc_expire)
		    nexpire[hash]--;
		rt->mfc_expire	   = 0;
	    }
	}
	if (rt == NULL) {
	    /* no upcall, so make a new entry */
	    rt = (struct mfc *)malloc(sizeof(*rt), M_MRTABLE, M_NOWAIT);
	    if (rt == NULL) {
		splx(s);
		return ENOBUFS;
	    }
	    
	    /* insert new entry at head of hash chain */
	    rt->mfc_origin     = mfccp->mfcc_origin;
	    rt->mfc_mcastgrp   = mfccp->mfcc_mcastgrp;
	    rt->mfc_parent     = mfccp->mfcc_parent;
	    for (i = 0; i < numvifs; i++)
		    rt->mfc_ttls[i] = mfccp->mfcc_ttls[i];
	    /* initialize pkt counters per src-grp */
	    rt->mfc_pkt_cnt    = 0;
	    rt->mfc_byte_cnt   = 0;
	    rt->mfc_wrong_if   = 0;
	    rt->mfc_last_assert.tv_sec = rt->mfc_last_assert.tv_usec = 0;
	    rt->mfc_expire     = 0;
	    rt->mfc_stall      = NULL;
	    
	    /* link into table */
	    rt->mfc_next = mfctable[hash];
	    mfctable[hash] = rt;
	}
    }
    splx(s);
    return 0;
}
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)

int rtl865x_checkMfcCache(unsigned long origin,unsigned long mcastgrp)
{
	struct mfc *mfc=NULL;
	struct mfc **nptr;
	unsigned long origin_temp=0;
	int hash_index=0;
	
	MFCFIND(origin,mcastgrp,mfc);
	if(mfc==NULL)
	{
		MFCFIND(origin_temp,mcastgrp,mfc);
	}	
	if(mfc!=NULL)
	{
		return 0;
	}
	else
	{
		for ( hash_index = 0; hash_index<MFCTBLSIZ; hash_index++)
		{
			nptr = &mfctable[hash_index];
		 	while ((mfc = *nptr) != NULL) {
			
				if(mcastgrp==mfc->mfc_mcastgrp.s_addr)
					return 0;
				
				nptr = &mfc->mfc_next;
		    }
		}
	}
	return -1;
}
#endif

#ifdef UPCALL_TIMING
/*
 * collect delay statistics on the upcalls 
 */
static void collate(t)
register struct timeval *t;
{
    register u_long d;
    register struct timeval tp;
    register u_long delta;
    
    GET_TIME(tp);
    
    if (TV_LT(*t, tp))
    {
	TV_DELTA(tp, *t, delta);
	
	d = delta >> 10;
	if (d > 50)
	    d = 50;
	
	++upcall_data[d];
    }
}
#endif /* UPCALL_TIMING */

/*
 * Delete an mfc entry
 */
static int
del_mfc(mfccp)
    struct mfcctl *mfccp;
{
    struct in_addr 	origin;
    struct in_addr 	mcastgrp;
    struct mfc 		*rt;
    struct mfc	 	**nptr;
    u_long 		hash;
    int s;

    origin = mfccp->mfcc_origin;
    mcastgrp = mfccp->mfcc_mcastgrp;
    hash = MFCHASH(origin.s_addr, mcastgrp.s_addr);
	//diag_printf("origin:%x,mcastgrp:%x[%s]:[%d].\n",origin,mcastgrp,__FUNCTION__,__LINE__);
    if (mrtdebug & DEBUG_MFC)
	log(LOG_DEBUG,"del_mfc orig %lx mcastgrp %lx\n",
	    (u_long)ntohl(origin.s_addr), (u_long)ntohl(mcastgrp.s_addr));

    s = splnet();

    nptr = &mfctable[hash];
    while ((rt = *nptr) != NULL) {
	if (origin.s_addr == rt->mfc_origin.s_addr &&
	    mcastgrp.s_addr == rt->mfc_mcastgrp.s_addr &&
	    rt->mfc_stall == NULL)
	    break;

	nptr = &rt->mfc_next;
    }
    if (rt == NULL) {
	splx(s);
	return EADDRNOTAVAIL;
    }

    *nptr = rt->mfc_next;
    free(rt, M_MRTABLE);

    splx(s);

    return 0;
}

#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)

static struct ifnet *upifp;
static struct ifnet *downifp;
static unsigned short upvif_index;
static unsigned short downvif_index;

struct ifnet * get_igmpproxyif(int flag)
{
	if (flag == IGMP_PROXY_UP_STREAM_FLAG ){
		return upifp;
	}	
	else if (flag == IGMP_PROXY_DOWN_STREAM_FLAG)
		return downifp;
	else 
		return NULL;
}


int mfc_process(int mfctype,struct ifnet *ifp,in_addr_t igmp_group)
{
	struct	mfcctl mfc;
	int err;
	
	memset(&mfc,0,sizeof(mfc));
	mfc.mfcc_origin.s_addr=0;
	mfc.mfcc_mcastgrp.s_addr=igmp_group;
	if(upifp && (upvif_index!=0xFFFF))
		mfc.mfcc_parent = upvif_index;
	memset(mfc.mfcc_ttls,255,sizeof(mfc.mfcc_ttls));
	if (downifp && (downvif_index!=0xFFFF))
		mfc.mfcc_ttls[downvif_index]=1;	
	
	if(mfctype==ADD_MFC_FLAG)
		err = add_mfc(&mfc);
	else if (mfctype==DEL_MFC_FLAG)
		err = del_mfc(&mfc);
	
	return err;
}

int rtl865x_igmpProxyJoinCallbackFn(void *param)
{
	int mfctype=ADD_MFC_FLAG;
	rtl_multicastEventContext_t mcastEventContext;
	in_addr_t igmp_group;
	int err;
	struct	in_addr imr_multiaddr;
	
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));
	igmp_group = mcastEventContext.groupAddr[0];
	mfc_process(mfctype,downifp, igmp_group);
	imr_multiaddr.s_addr=igmp_group;
//	diag_printf("--------join group:%x,[%s]:[%d].\n",igmp_group,__FUNCTION__,__LINE__);
	if (upifp){
		#if 1
		in_addmulti(&imr_multiaddr, upifp);
		#else
		igmp_packet_send(IGMP_V2_MEMBERSHIP_REPORT,upifp,igmp_group);	
		#endif
	}
	return err;
	
}

int rtl865x_igmpProxyLeaveCallbackFn(void *param)
{
	int mfctype=DEL_MFC_FLAG;
	rtl_multicastEventContext_t mcastEventContext;
	in_addr_t igmp_group;
	int err;
	struct in_multi *inm=NULL;
//	diag_printf("-------leave callback[%s]:[%d].\n",__FUNCTION__,__LINE__);
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));
	igmp_group = mcastEventContext.groupAddr[0];
	mfc_process(mfctype,downifp, igmp_group);
//	diag_printf("---------leave group:%x[%s]:[%d].\n",igmp_group,__FUNCTION__,__LINE__);
	
	/*send report to upstream*/
	if (upifp){
		//diag_printf("--------[%s]:[%d].\n",__FUNCTION__,__LINE__);
		#if 1
		if((inm=find_multi(upifp,igmp_group))!=NULL){
			//diag_printf("FIND MULTI,DEL [%s]:[%d]\n",__FUNCTION__,__LINE__);
			in_delmulti(inm);
		}	
		#else
		igmp_packet_send(IGMP_V2_LEAVE_GROUP,upifp,igmp_group);	
		#endif
		
	}	
	
	return err;
}

int _rtl865x_igmpProxyJoinUnRegisterEvent(void)
{
	int ret=FAILED;
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_GROUP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_igmpProxyJoinCallbackFn;
	ret = rtl865x_unRegisterEvent(&eventParam);

	return ret;
}

int _rtl865x_igmpProxyJoinRegisterEvent(void)
{
	int ret=FAILED;	
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_GROUP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_igmpProxyJoinCallbackFn;
	ret=rtl865x_registerEvent(&eventParam);

	return ret;
}

int _rtl865x_igmpProxyLeaveUnRegisterEvent(void)
{
	int ret=FAILED;
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_GROUP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_igmpProxyLeaveCallbackFn;
	ret=rtl865x_unRegisterEvent(&eventParam);

	return ret;
}

int _rtl865x_igmpProxyLeaveRegisterEvent(void)
{
	int ret=FAILED;
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_GROUP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_igmpProxyLeaveCallbackFn;
	ret=rtl865x_registerEvent(&eventParam);

	return ret;
}

int rtl865x_initIgmpProxy(void)
{
	_rtl865x_igmpProxyJoinUnRegisterEvent();
	_rtl865x_igmpProxyJoinRegisterEvent();
	
	_rtl865x_igmpProxyLeaveUnRegisterEvent();
	_rtl865x_igmpProxyLeaveRegisterEvent();
	return SUCCESS;
}	

int rtl865x_exitIgmpProxy(void)
{
	_rtl865x_igmpProxyJoinUnRegisterEvent();
	_rtl865x_igmpProxyLeaveUnRegisterEvent();
	return SUCCESS;
}	

static int ip_mroute_init_vif(char * wan_if,char *lan_if )
{
	struct ifnet *ifp;
	struct ifnet *add_ifp=NULL;
	
	struct ifaddr *ifa;
	int find=0;
	struct uvif *v;
	struct vifctl vifc;
	int vifi = -1;
	int err;
	
	bzero((caddr_t)viftable, sizeof(viftable));
	numvifs = 0;
	downifp=NULL;
	upifp=NULL;
	upvif_index=0xFFFF;
	downvif_index=0xFFFF;
	
	//diag_printf("\n --------------wan:%s,lan:%s[%s]:[%d].\n",wan_if,lan_if,__FUNCTION__,__LINE__);
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	{
		char workbuf[64];
		int ifnlen=0;
		char if_name[IFNAMSIZ];
		find = 0;
		add_ifp = NULL;
		
		//diag_printf("--------------if:%d,%s,%s,%d.[%s]:[%d].\n",ifp->if_index,ifp->if_name,ifp->if_xname,ifp->if_unit,__FUNCTION__,__LINE__);
		
		ifnlen = snprintf(workbuf, sizeof(workbuf),
	    	"%s%d", ifp->if_name, ifp->if_unit);
		if(ifnlen + 1 > sizeof (if_name)) {
			err = ENAMETOOLONG;
			continue;
		} else {
			strcpy(if_name, workbuf);
		}
		
		
		if(ifp->if_flags & IFF_UP==0)	
			continue;
		
		if ((ifp->if_flags & (IFF_LOOPBACK|IFF_MULTICAST)) != IFF_MULTICAST){
			//diag_printf("IF:%s,flags:%x,next.[%s]:[%d].\n",ifrq.ifr_name,ifrq.ifr_flags,__FUNCTION__,__LINE__);
			continue;
		}	
			 				
		for(ifa = ifp->if_addrhead.tqh_first; ifa; ifa = ifa->ifa_link.tqe_next)
		{
			if ((ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_INET) ){
				find = 1;
				add_ifp = ifp;
				vifi++;
				break;
			}
		}
		
		if(find ==1 && add_ifp!=NULL)
		{
			
			//diag_printf("--------------if:%d,%s,%s,%d.[%s]:[%d].\n",add_ifp->if_index,add_ifp->if_name,add_ifp->if_xname,add_ifp->if_unit,__FUNCTION__,__LINE__);
			memset(&vifc,0,sizeof(vifc));
			#if 1
			vifc.vifc_vifi            = vifi;
			vifc.vifc_flags           = IFF_MULTICAST;//120816hx
			vifc.vifc_threshold = 1;    /* Packet TTL must be at least 1 to pass them */
			vifc.vifc_rate_limit = 0;   /* hopefully no limit */
			vifc.vifc_lcl_addr.s_addr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
			vifc.vifc_rmt_addr.s_addr = INADDR_ANY;
			#endif 
			if(vifc.vifc_lcl_addr.s_addr)
			{
				add_vif(&vifc);		
				if(strcmp(if_name,lan_if)==0) 
				{
					downifp = add_ifp;
					downvif_index = vifi;
					/*
					if (downifp)
					diag_printf("--------------down:%d,%d,%s.[%s]:[%d].\n",downvif_index,downifp->if_index,downifp->if_name,__FUNCTION__,__LINE__);
					*/
				}
				else if(strcmp(if_name,wan_if)==0) 
				{
					upifp = add_ifp;
					upvif_index = vifi;
					/*
					if (upifp)
					diag_printf("--------------up:%d,%d,%s.[%s]:[%d].\n",upvif_index,upifp->if_index,upifp->if_name,__FUNCTION__,__LINE__);
					*/
				}
			}
		}
		
	}	
	if(upifp&&downifp)
		return 1;
	else
		return 0;
}


static struct callout gquery_timer;
static void igmp_general_query_expired(void * para)
{
	struct query_timer * query_para =(struct query_timer *)para;
	
	//diag_printf("----igmp_general_query_expired----\n");
	if(query_para==NULL)
		return;
	
	
//	diag_printf("-----------igmp general query expired.%p,type:%d,retry:%d,group:%x\n",query_para,query_para->type,query_para->retry_left_time,query_para->igmp_group);
	if(query_para->type== GENERAL_PERIODICAL_TIMER_TPYE)	
	{
		if(downifp)
			igmp_packet_send(IGMP_MEMBERSHIP_QUERY,downifp,query_para->igmp_group);
		callout_reset(&gquery_timer,(IGMP_QUERY_INTERVAL*hz) ,igmp_general_query_expired, query_para);
	}
	else
	{
		
		if(query_para->retry_left_time)
		{
			if(downifp)
			{
				igmp_packet_send(IGMP_MEMBERSHIP_QUERY,downifp,query_para->igmp_group);
			
				query_para->retry_left_time--;
					
				if(query_para->retry_left_time)	
					callout_reset(&gquery_timer,(IGMP_STARTUP_QUERY_INTERVAL*hz) ,igmp_general_query_expired, query_para);
				else
				{
					query_para->type = GENERAL_PERIODICAL_TIMER_TPYE;
					callout_reset(&gquery_timer,(IGMP_QUERY_INTERVAL*hz) ,igmp_general_query_expired, query_para);
				}	
			}
			else
			{
				free(query_para, M_TEMP);
			}
		}
		
	}
	return;
	
}

void rtl865x_send_general_query()
{
	struct query_timer *query_para;
	
	/*send general query when start up */
	//		diag_printf("----igmp proxy set general query timer----\n");
	query_para=(struct query_timer *)malloc(
			sizeof(struct query_timer), M_TEMP, M_NOWAIT);
	if(query_para==NULL){
		diag_printf("no memory![%s]:[%d].\n",__FUNCTION__,__LINE__);
		return;
	}
	
	memset(query_para,0, sizeof(struct query_timer));
	query_para->retry_left_time = IGMP_STARTUP_QUERY_COUNT;
	query_para->igmp_group = 0;
	query_para->type= GENERAL_START_TIMER_TPYE;

		
	callout_init(&gquery_timer);
	callout_reset(&gquery_timer,(5*hz) ,igmp_general_query_expired, query_para);
	return ;

}
struct initpara_timer
{
	uint32 retry_time;
	char wan_if[IFNAMSIZ];
	char lan_if[IFNAMSIZ];
};

static struct callout initvif_timer;
#define IGMPPROXY_INIT_RETRY_TIMES 100

static void ip_mroute_reinit_vif_expire(void * para)
{
	struct initpara_timer * init_para =(struct initpara_timer *)para;
	int ret=0;
	
	if(init_para==NULL)
		return;
	diag_printf("igmp proxy (%s %s) reinit %d......\n",init_para->wan_if,init_para->lan_if,(init_para->retry_time+1));
	ret=ip_mroute_init_vif( init_para->wan_if,init_para->lan_if);
	if(ret==0)
	{
		init_para->retry_time++;
		callout_reset(&initvif_timer,((2<<init_para->retry_time)*hz) ,ip_mroute_reinit_vif_expire, init_para);
		if(init_para->retry_time>=IGMPPROXY_INIT_RETRY_TIMES)
		{
			free(init_para, M_TEMP);
			diag_printf("----igmp proxy (%s %s) kernel mode init fail----\n",init_para->wan_if,init_para->lan_if);
			ip_mroute_kernel_mode_exit();
		}
	}
	else
	{
		
		if(downifp && upifp)
		{
			rtl865x_initIgmpProxy();
			rtl865x_send_general_query();
					
		}	
		free(init_para, M_TEMP);
	}
	return;
	
}

int ip_mroute_kernel_mode_init(char * wan_if,char *lan_if )
{
	
	struct initpara_timer *init_para;
	int ret=0;
	diag_printf("igmp proxy (%s %s) kernel mode init...\n",wan_if,lan_if);
	ret=ip_mroute_init_vif( wan_if,lan_if);
	if(ret==0)/*ip_mroute_init_vif fail */
	{
		init_para=(struct initpara_timer *)malloc(
							sizeof(struct initpara_timer), M_TEMP, M_NOWAIT);
		if(init_para==NULL){
			diag_printf("no memory![%s]:[%d].\n",__FUNCTION__,__LINE__);
			return 0;
		}
		
		memset(init_para,0, sizeof(struct initpara_timer));
		init_para->retry_time = 0;
		strcpy(init_para->wan_if,wan_if);
		strcpy(init_para->lan_if, lan_if);
		callout_init(&initvif_timer);
		callout_reset(&initvif_timer,((2<<init_para->retry_time)*hz) ,ip_mroute_reinit_vif_expire, init_para);
	}
	
	
	if(downifp && upifp)
	{
		rtl865x_initIgmpProxy();
		rtl865x_send_general_query();
				
	}	
	
	return 0;
}
int ip_mroute_kernel_mode_exit(void)
{
	int i;
	struct mfc *rt;
	struct rtdetq *rte;
	
	diag_printf("igmp proxy kernel mode exit!!!");
	bzero((caddr_t)viftable, sizeof(viftable));
	numvifs = 0;
	/*
     * Free all multicast forwarding cache entries.
     */
    for (i = 0; i < MFCTBLSIZ; i++) {
	for (rt = mfctable[i]; rt != NULL; ) {
	    struct mfc *nr = rt->mfc_next;

	    for (rte = rt->mfc_stall; rte != NULL; ) {
			struct rtdetq *n = rte->next;

			m_freem(rte->m);
			free(rte, M_MRTABLE);
			rte = n;
	    }
	    free(rt, M_MRTABLE);
	    rt = nr;
	}
    }

    bzero((caddr_t)mfctable, sizeof(mfctable));
	
	rtl865x_exitIgmpProxy();
	
	return 0;
}

#endif


/*
 * Send a message to mrouted on the multicast routing socket
 */
static int
socket_send(s, mm, src)
	struct socket *s;
	struct mbuf *mm;
	struct sockaddr_in *src;
{
	if (s) {
		if (sbappendaddr(&s->so_rcv,
				 (struct sockaddr *)src,
				 mm, (struct mbuf *)0) != 0) {
			sorwakeup(s);
			return 0;
		}
	}
	m_freem(mm);
	return -1;
}

/*
 * IP multicast forwarding function. This function assumes that the packet
 * pointed to by "ip" has arrived on (or is about to be sent to) the interface
 * pointed to by "ifp", and the packet is to be relayed to other networks
 * that have members of the packet's destination IP multicast group.
 *
 * The packet is returned unscathed to the caller, unless it is
 * erroneous, in which case a non-zero return value tells the caller to
 * discard it.
 */

#define IP_HDR_LEN  20	/* # bytes of fixed IP header (excluding options) */
#define TUNNEL_LEN  12  /* # bytes of IP option for tunnel encapsulation  */

static int
X_ip_mforward(ip, ifp, m, imo)
    register struct ip *ip;
    struct ifnet *ifp;
    struct mbuf *m;
    struct ip_moptions *imo;
{
    register struct mfc *rt;
    register u_char *ipoptions;
    static struct sockaddr_in 	k_igmpsrc	= { sizeof k_igmpsrc, AF_INET };
    static int srctun = 0;
    register struct mbuf *mm;
    int s;
    vifi_t vifi;
    struct vif *vifp;
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	unsigned long origin=0;
#endif
	
    if (mrtdebug & DEBUG_FORWARD)
	log(LOG_DEBUG, "ip_mforward: src %lx, dst %lx, ifp %p\n",
	    (u_long)ntohl(ip->ip_src.s_addr), (u_long)ntohl(ip->ip_dst.s_addr),
	    (void *)ifp);
	
    if (ip->ip_hl < (IP_HDR_LEN + TUNNEL_LEN) >> 2 ||
	(ipoptions = (u_char *)(ip + 1))[1] != IPOPT_LSRR ) {
	/*
	 * Packet arrived via a physical interface or
	 * an encapsulated tunnel.
	 */
    } else {
	/*
	 * Packet arrived through a source-route tunnel.
	 * Source-route tunnels are no longer supported.
	 */
	if ((srctun++ % 1000) == 0)
	    log(LOG_ERR,
		"ip_mforward: received source-routed packet from %lx\n",
		(u_long)ntohl(ip->ip_src.s_addr));

	return 1;
    }

    if ((imo) && ((vifi = imo->imo_multicast_vif) < numvifs)) {
	if (ip->ip_ttl < 255)
		ip->ip_ttl++;	/* compensate for -1 in *_send routines */
	if (rsvpdebug && ip->ip_p == IPPROTO_RSVP) {
	    vifp = viftable + vifi;
	    printf("Sending IPPROTO_RSVP from %lx to %lx on vif %d (%s%s%d)\n",
		ntohl(ip->ip_src.s_addr), ntohl(ip->ip_dst.s_addr), vifi,
		(vifp->v_flags & VIFF_TUNNEL) ? "tunnel on " : "",
		vifp->v_ifp->if_name, vifp->v_ifp->if_unit);
	}
	return (ip_mdq(m, ifp, NULL, vifi));
    }
    if (rsvpdebug && ip->ip_p == IPPROTO_RSVP) {
	printf("Warning: IPPROTO_RSVP from %lx to %lx without vif option\n",
	    ntohl(ip->ip_src.s_addr), ntohl(ip->ip_dst.s_addr));
	if(!imo)
		printf("In fact, no options were specified at all\n");
    }

    /*
     * Don't forward a packet with time-to-live of zero or one,
     * or a packet destined to a local-only group.
     */
    if (ip->ip_ttl <= 1 ||
	ntohl(ip->ip_dst.s_addr) <= INADDR_MAX_LOCAL_GROUP)
	return 0;

    /*
     * Determine forwarding vifs from the forwarding cache table
     */
    s = splnet();
	
	MFCFIND(ip->ip_src.s_addr, ip->ip_dst.s_addr, rt);
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	if(rt==NULL)
		MFCFIND(origin, ip->ip_dst.s_addr, rt);
#endif
	if(rt!=NULL){
	
		//diag_printf("Entry exists.src:%x,dst:%x,origin:%x.[%s]:[%d].\n",ip->ip_src.s_addr,ip->ip_dst.s_addr,origin,__FUNCTION__,__LINE__);
	splx(s);
	return (ip_mdq(m, ifp, rt, -1));
    } else {
    	//diag_printf("Entry not exists.[%s]:[%d].\n",__FUNCTION__,__LINE__);
	/*
	 * If we don't have a route for packet's origin,
	 * Make a copy of the packet &
	 * send message to routing daemon
	 */

	register struct mbuf *mb0;
	register struct rtdetq *rte;
	register u_long hash;
	int hlen = ip->ip_hl << 2;
#ifdef UPCALL_TIMING
	struct timeval tp;

	GET_TIME(tp);
#endif

	mrtstat.mrts_no_route++;
	if (mrtdebug & (DEBUG_FORWARD | DEBUG_MFC))
	    log(LOG_DEBUG, "ip_mforward: no rte s %lx g %lx\n",
		(u_long)ntohl(ip->ip_src.s_addr),
		(u_long)ntohl(ip->ip_dst.s_addr));

	/*
	 * Allocate mbufs early so that we don't do extra work if we are
	 * just going to fail anyway.  Make sure to pullup the header so
	 * that other people can't step on it.
	 */
	rte = (struct rtdetq *)malloc((sizeof *rte), M_MRTABLE, M_NOWAIT);
	if (rte == NULL) {
	    splx(s);
	    return ENOBUFS;
	}
	mb0 = m_copy(m, 0, M_COPYALL);
	if (mb0 && (M_HASCL(mb0) || mb0->m_len < hlen))
	    mb0 = m_pullup(mb0, hlen);
	if (mb0 == NULL) {
	    free(rte, M_MRTABLE);
	    splx(s);
	    return ENOBUFS;
	}

	/* is there an upcall waiting for this packet? */
	hash = MFCHASH(ip->ip_src.s_addr, ip->ip_dst.s_addr);
	for (rt = mfctable[hash]; rt; rt = rt->mfc_next) {
	    if ((ip->ip_src.s_addr == rt->mfc_origin.s_addr) &&
		(ip->ip_dst.s_addr == rt->mfc_mcastgrp.s_addr) &&
		(rt->mfc_stall != NULL))
		break;
	}

	if (rt == NULL) {
	    int i;
	    struct igmpmsg *im;

	    /* no upcall, so make a new entry */
	    rt = (struct mfc *)malloc(sizeof(*rt), M_MRTABLE, M_NOWAIT);
	    if (rt == NULL) {
		free(rte, M_MRTABLE);
		m_freem(mb0);
		splx(s);
		return ENOBUFS;
	    }
	    /* Make a copy of the header to send to the user level process */
	    mm = m_copy(mb0, 0, hlen);
	    if (mm == NULL) {
		free(rte, M_MRTABLE);
		m_freem(mb0);
		free(rt, M_MRTABLE);
		splx(s);
		return ENOBUFS;
	    }

	    /* 
	     * Send message to routing daemon to install 
	     * a route into the kernel table
	     */
	    k_igmpsrc.sin_addr = ip->ip_src;
	    
	    im = mtod(mm, struct igmpmsg *);
	    im->im_msgtype	= IGMPMSG_NOCACHE;
	    im->im_mbz		= 0;

	    mrtstat.mrts_upcalls++;

	

	    if (socket_send(ip_mrouter, mm, &k_igmpsrc) < 0) 
		{
		#ifndef CONFIG_RTL_IGMP_PROXY_KERNEL_MODE
		//log(LOG_WARNING, "ip_mforward: ip_mrouter socket queue full\n");
		#endif
		++mrtstat.mrts_upq_sockfull;
		free(rte, M_MRTABLE);
		m_freem(mb0);
		free(rt, M_MRTABLE);
		splx(s);
		return ENOBUFS;
	    }

	    /* insert new entry at head of hash chain */
	    rt->mfc_origin.s_addr     = ip->ip_src.s_addr;
	    rt->mfc_mcastgrp.s_addr   = ip->ip_dst.s_addr;
	    rt->mfc_expire	      = UPCALL_EXPIRE;
	    nexpire[hash]++;
	    for (i = 0; i < numvifs; i++)
		rt->mfc_ttls[i] = 0;
	    rt->mfc_parent = -1;

	    /* link into table */
	    rt->mfc_next   = mfctable[hash];
	    mfctable[hash] = rt;
	    rt->mfc_stall = rte;

	} else {
	    /* determine if q has overflowed */
	    int npkts = 0;
	    struct rtdetq **p;

	    for (p = &rt->mfc_stall; *p != NULL; p = &(*p)->next)
		npkts++;

	    if (npkts > MAX_UPQ) {
		mrtstat.mrts_upq_ovflw++;
		free(rte, M_MRTABLE);
		m_freem(mb0);
		splx(s);
		return 0;
	    }

	    /* Add this entry to the end of the queue */
	    *p = rte;
	}

	rte->m 			= mb0;
	rte->ifp 		= ifp;
#ifdef UPCALL_TIMING
	rte->t			= tp;
#endif
	rte->next		= NULL;

	splx(s);

	return 0;
    }		
}

#ifndef MROUTE_LKM
int (*ip_mforward)(struct ip *, struct ifnet *, struct mbuf *,
		   struct ip_moptions *) = X_ip_mforward;
#endif

/*
 * Clean up the cache entry if upcall is not serviced
 */
static void
expire_upcalls(void *unused)
{
    struct rtdetq *rte;
    struct mfc *mfc, **nptr;
    int i;
    int s;

    s = splnet();
    for (i = 0; i < MFCTBLSIZ; i++) {
	if (nexpire[i] == 0)
	    continue;
	nptr = &mfctable[i];
	for (mfc = *nptr; mfc != NULL; mfc = *nptr) {
	    /*
	     * Skip real cache entries
	     * Make sure it wasn't marked to not expire (shouldn't happen)
	     * If it expires now
	     */
	    if (mfc->mfc_stall != NULL &&
	        mfc->mfc_expire != 0 &&
		--mfc->mfc_expire == 0) {
		if (mrtdebug & DEBUG_EXPIRE)
		    log(LOG_DEBUG, "expire_upcalls: expiring (%lx %lx)\n",
			(u_long)ntohl(mfc->mfc_origin.s_addr),
			(u_long)ntohl(mfc->mfc_mcastgrp.s_addr));
		/*
		 * drop all the packets
		 * free the mbuf with the pkt, if, timing info
		 */
		for (rte = mfc->mfc_stall; rte; ) {
		    struct rtdetq *n = rte->next;

		    m_freem(rte->m);
		    free(rte, M_MRTABLE);
		    rte = n;
		}
		++mrtstat.mrts_cache_cleanups;
		nexpire[i]--;

		*nptr = mfc->mfc_next;
		free(mfc, M_MRTABLE);
	    } else {
		nptr = &mfc->mfc_next;
	    }
	}
    }
    splx(s);
    //expire_upcalls_ch = timeout(expire_upcalls, (caddr_t)NULL, EXPIRE_TIMEOUT);
    
#if defined(__NetBSD__) || (defined(__FreeBSD__) && __FreeBSD__ >= 3)
		callout_reset(&expire_upcalls_ch, EXPIRE_TIMEOUT,
			expire_upcalls, NULL);
#elif defined(__OpenBSD__)
		timeout_set(&expire_upcalls_ch, expire_upcalls, NULL);
		timeout_add(&expire_upcalls_ch, EXPIRE_TIMEOUT);
#else
		timeout(expire_upcalls, (caddr_t)NULL, EXPIRE_TIMEOUT);
#endif
}

/*
 * Packet forwarding routine once entry in the cache is made
 */
static int
ip_mdq(m, ifp, rt, xmt_vif)
    register struct mbuf *m;
    register struct ifnet *ifp;
    register struct mfc *rt;
    register vifi_t xmt_vif;
{
    register struct ip  *ip = mtod(m, struct ip *);
    register vifi_t vifi;
    register struct vif *vifp;
    register int plen = ip->ip_len;
	
	//diag_printf("ip_mdq.ifp:%p,%d,xmt_vif:%d,numvifs:%d.[%s]:[%d].\n",(void *)ifp,ifp->if_index,xmt_vif,numvifs,__FUNCTION__,__LINE__);
/*
 * Macro to send packet on vif.  Since RSVP packets don't get counted on
 * input, they shouldn't get counted on output, so statistics keeping is
 * seperate.
 */
#define MC_SEND(ip,vifp,m) {                             \
                if ((vifp)->v_flags & VIFF_TUNNEL)  	 \
                    encap_send((ip), (vifp), (m));       \
                else                                     \
                    phyint_send((ip), (vifp), (m));      \
}

    /*
     * If xmt_vif is not -1, send on only the requested vif.
     *
     * (since vifi_t is u_short, -1 becomes MAXUSHORT, which > numvifs.)
     */
    

	if (xmt_vif < numvifs) {
		//diag_printf("ip_mdq.xmt_vif < numvifs[%s]:[%d].\n",__FUNCTION__,__LINE__);
	MC_SEND(ip, viftable + xmt_vif, m);
	return 1;
    }

    /*
     * Don't forward if it didn't arrive from the parent vif for its origin.
     */
    vifi = rt->mfc_parent;
	//diag_printf("ip_mdq,vifi:%d,ifp:%s,%d,%s,%d.[%s]:[%d].\n",vifi,viftable[vifi].v_ifp->if_name,viftable[vifi].v_ifp->if_index,ifp->if_name,ifp->if_index,__FUNCTION__,__LINE__);
    if ((vifi >= numvifs) || (viftable[vifi].v_ifp != ifp)) {
	/* came in the wrong interface */
	if (mrtdebug & DEBUG_FORWARD)
	    log(LOG_DEBUG, "wrong if: ifp %p vifi %d vififp %p\n",
		(void *)ifp, vifi, (void *)viftable[vifi].v_ifp); 
	++mrtstat.mrts_wrong_if;
	++rt->mfc_wrong_if;
	/*
	 * If we are doing PIM assert processing, and we are forwarding
	 * packets on this interface, and it is a broadcast medium
	 * interface (and not a tunnel), send a message to the routing daemon.
	 */
	if (pim_assert && rt->mfc_ttls[vifi] &&
		(ifp->if_flags & IFF_BROADCAST) &&
		!(viftable[vifi].v_flags & VIFF_TUNNEL)) {
	    struct sockaddr_in k_igmpsrc;
	    struct mbuf *mm;
	    struct igmpmsg *im;
	    int hlen = ip->ip_hl << 2;
	    struct timeval now;
	    register u_long delta;

	    GET_TIME(now);

	    TV_DELTA(rt->mfc_last_assert, now, delta);

	    if (delta > ASSERT_MSG_TIME) {
		mm = m_copy(m, 0, hlen);
		if (mm && (M_HASCL(mm) || mm->m_len < hlen))
		    mm = m_pullup(mm, hlen);
		if (mm == NULL) {
		    return ENOBUFS;
		}

		rt->mfc_last_assert = now;

		im = mtod(mm, struct igmpmsg *);
		im->im_msgtype	= IGMPMSG_WRONGVIF;
		im->im_mbz		= 0;
		im->im_vif		= vifi;

		k_igmpsrc.sin_addr = im->im_src;

		socket_send(ip_mrouter, mm, &k_igmpsrc);
	    }
	}
	//diag_printf("ip_mdq.[%s]:[%d].\n",__FUNCTION__,__LINE__);
	return 0;
    }

    /* If I sourced this packet, it counts as output, else it was input. */
    if (ip->ip_src.s_addr == viftable[vifi].v_lcl_addr.s_addr) {
	viftable[vifi].v_pkt_out++;
	viftable[vifi].v_bytes_out += plen;
    } else {
	viftable[vifi].v_pkt_in++;
	viftable[vifi].v_bytes_in += plen;
    }
    rt->mfc_pkt_cnt++;
    rt->mfc_byte_cnt += plen;

    /*
     * For each vif, decide if a copy of the packet should be forwarded.
     * Forward if:
     *		- the ttl exceeds the vif's threshold
     *		- there are group members downstream on interface
     */
 //	diag_printf("ip_mdq.ip->ip_ttl:%d,[%s]:[%d].\n",ip->ip_ttl,__FUNCTION__,__LINE__);
    for (vifp = viftable, vifi = 0; vifi < numvifs; vifp++, vifi++){
		//diag_printf("vifi:%d.ttl:%d.[%s]:[%d].\n",vifi,rt->mfc_ttls[vifi],__FUNCTION__,__LINE__);
	if ((rt->mfc_ttls[vifi] > 0) &&
	    (ip->ip_ttl > rt->mfc_ttls[vifi])) {
	   	//diag_printf("ip_mdq.vifi:%d,[%s]:[%d].\n",vifi,__FUNCTION__,__LINE__);
	    vifp->v_pkt_out++;
	    vifp->v_bytes_out += plen;
	    MC_SEND(ip, vifp, m);
	}
	}
    return 0;
}

/*
 * check if a vif number is legal/ok. This is used by ip_output, to export
 * numvifs there, 
 */
static int
X_legal_vif_num(vif)
    int vif;
{
    if (vif >= 0 && vif < numvifs)
       return(1);
    else
       return(0);
}

#ifndef MROUTE_LKM
int (*legal_vif_num)(int) = X_legal_vif_num;
#endif

/*
 * Return the local address used by this vif
 */
static u_long
X_ip_mcast_src(vifi)
    int vifi;
{
    if (vifi >= 0 && vifi < numvifs)
	return viftable[vifi].v_lcl_addr.s_addr;
    else
	return INADDR_ANY;
}

#ifndef MROUTE_LKM
u_long (*ip_mcast_src)(int) = X_ip_mcast_src;
#endif

static void
phyint_send(ip, vifp, m)
    struct ip *ip;
    struct vif *vifp;
    struct mbuf *m;
{
    register struct mbuf *mb_copy;
    register int hlen = ip->ip_hl << 2;

    /*
     * Make a new reference to the packet; make sure that
     * the IP header is actually copied, not just referenced,
     * so that ip_output() only scribbles on the copy.
     */
    mb_copy = m_copy(m, 0, M_COPYALL);
    if (mb_copy && (M_HASCL(mb_copy) || mb_copy->m_len < hlen))
	mb_copy = m_pullup(mb_copy, hlen);
    if (mb_copy == NULL)
	return;

    if (vifp->v_rate_limit == 0)
	tbf_send_packet(vifp, mb_copy);
    else
	tbf_control(vifp, mb_copy, mtod(mb_copy, struct ip *), ip->ip_len);
}

static void
encap_send(ip, vifp, m)
    register struct ip *ip;
    register struct vif *vifp;
    register struct mbuf *m;
{
    register struct mbuf *mb_copy;
    register struct ip *ip_copy;
    register int i, len = ip->ip_len;

    /*
     * copy the old packet & pullup its IP header into the
     * new mbuf so we can modify it.  Try to fill the new
     * mbuf since if we don't the ethernet driver will.
     */
    MGETHDR(mb_copy, M_DONTWAIT, MT_HEADER);
    if (mb_copy == NULL)
	return;
    mb_copy->m_data += max_linkhdr;
    mb_copy->m_len = sizeof(multicast_encap_iphdr);

    if ((mb_copy->m_next = m_copy(m, 0, M_COPYALL)) == NULL) {
	m_freem(mb_copy);
	return;
    }
    i = MHLEN - M_LEADINGSPACE(mb_copy);
    if (i > len)
	i = len;
    mb_copy = m_pullup(mb_copy, i);
    if (mb_copy == NULL)
	return;
    mb_copy->m_pkthdr.len = len + sizeof(multicast_encap_iphdr);

    /*
     * fill in the encapsulating IP header.
     */
    ip_copy = mtod(mb_copy, struct ip *);
    *ip_copy = multicast_encap_iphdr;
#ifdef RANDOM_IP_ID
    ip_copy->ip_id = ip_randomid();
#else
    ip_copy->ip_id = htons(ip_id++);
#endif
    ip_copy->ip_len += len;
    ip_copy->ip_src = vifp->v_lcl_addr;
    ip_copy->ip_dst = vifp->v_rmt_addr;

    /*
     * turn the encapsulated IP header back into a valid one.
     */
    ip = (struct ip *)((caddr_t)ip_copy + sizeof(multicast_encap_iphdr));
    --ip->ip_ttl;
    HTONS(ip->ip_len);
    HTONS(ip->ip_off);
    ip->ip_sum = 0;
    mb_copy->m_data += sizeof(multicast_encap_iphdr);
    ip->ip_sum = in_cksum(mb_copy, ip->ip_hl << 2);
    mb_copy->m_data -= sizeof(multicast_encap_iphdr);

    if (vifp->v_rate_limit == 0)
	tbf_send_packet(vifp, mb_copy);
    else
	tbf_control(vifp, mb_copy, ip, ip_copy->ip_len);
}

/*
 * De-encapsulate a packet and feed it back through ip input (this
 * routine is called whenever IP gets a packet with proto type
 * ENCAP_PROTO and a local destination address).
 */
void
#ifdef MROUTE_LKM
X_ipip_input(m, off)
#else
ipip_input(m, off)
#endif
	register struct mbuf *m;
	int off;
{
    struct ifnet *ifp = m->m_pkthdr.rcvif;
    register struct ip *ip = mtod(m, struct ip *);
    register int hlen = ip->ip_hl << 2;
    register int s;
    register struct ifqueue *ifq;
    register struct vif *vifp;

    if (!have_encap_tunnel) {
	    rip_input(m, off);
	    return;
    }
    /*
     * dump the packet if it's not to a multicast destination or if
     * we don't have an encapsulating tunnel with the source.
     * Note:  This code assumes that the remote site IP address
     * uniquely identifies the tunnel (i.e., that this site has
     * at most one tunnel with the remote site).
     */
    if (! IN_MULTICAST(ntohl(((struct ip *)((char *)ip + hlen))->ip_dst.s_addr))) {
	++mrtstat.mrts_bad_tunnel;
	m_freem(m);
	return;
    }
    if (ip->ip_src.s_addr != last_encap_src) {
	register struct vif *vife;
	
	vifp = viftable;
	vife = vifp + numvifs;
	last_encap_src = ip->ip_src.s_addr;
	last_encap_vif = 0;
	for ( ; vifp < vife; ++vifp)
	    if (vifp->v_rmt_addr.s_addr == ip->ip_src.s_addr) {
		if ((vifp->v_flags & (VIFF_TUNNEL|VIFF_SRCRT))
		    == VIFF_TUNNEL)
		    last_encap_vif = vifp;
		break;
	    }
    }
    if ((vifp = last_encap_vif) == 0) {
	last_encap_src = 0;
	mrtstat.mrts_cant_tunnel++; /*XXX*/
	m_freem(m);
	if (mrtdebug)
	  log(LOG_DEBUG, "ip_mforward: no tunnel with %lx\n",
		(u_long)ntohl(ip->ip_src.s_addr));
	return;
    }
    ifp = vifp->v_ifp;

    if (hlen > IP_HDR_LEN)
      ip_stripoptions(m, (struct mbuf *) 0);
    m->m_data += IP_HDR_LEN;
    m->m_len -= IP_HDR_LEN;
    m->m_pkthdr.len -= IP_HDR_LEN;
    m->m_pkthdr.rcvif = ifp;

    ifq = &ipintrq;
    s = splimp();
    if (IF_QFULL(ifq)) {
	IF_DROP(ifq);
	m_freem(m);
    } else {
	IF_ENQUEUE(ifq, m);
	/*
	 * normally we would need a "schednetisr(NETISR_IP)"
	 * here but we were called by ip_input and it is going
	 * to loop back & try to dequeue the packet we just
	 * queued as soon as we return so we avoid the
	 * unnecessary software interrrupt.
	 */
    }
    splx(s);
}

/*
 * Token bucket filter module
 */

static void
tbf_control(vifp, m, ip, p_len)
	register struct vif *vifp;
	register struct mbuf *m;
	register struct ip *ip;
	register u_long p_len;
{
    register struct tbf *t = vifp->v_tbf;

    if (p_len > MAX_BKT_SIZE) {
	/* drop if packet is too large */
	mrtstat.mrts_pkt2large++;
	m_freem(m);
	return;
    }

    tbf_update_tokens(vifp);

    /* if there are enough tokens, 
     * and the queue is empty,
     * send this packet out
     */

    if (t->tbf_q_len == 0) {
	/* queue empty, send packet if enough tokens */
	if (p_len <= t->tbf_n_tok) {
	    t->tbf_n_tok -= p_len;
	    tbf_send_packet(vifp, m);
	} else {
	    /* queue packet and timeout till later */
	    tbf_queue(vifp, m);
	    timeout(tbf_reprocess_q, (caddr_t)vifp, TBF_REPROCESS);
	}
    } else if (t->tbf_q_len < t->tbf_max_q_len) {
	/* finite queue length, so queue pkts and process queue */
	tbf_queue(vifp, m);
	tbf_process_q(vifp);
    } else {
	/* queue length too much, try to dq and queue and process */
	if (!tbf_dq_sel(vifp, ip)) {
	    mrtstat.mrts_q_overflow++;
	    m_freem(m);
	    return;
	} else {
	    tbf_queue(vifp, m);
	    tbf_process_q(vifp);
	}
    }
    return;
}

/* 
 * adds a packet to the queue at the interface
 */
static void
tbf_queue(vifp, m) 
	register struct vif *vifp;
	register struct mbuf *m;
{
    register int s = splnet();
    register struct tbf *t = vifp->v_tbf;

    if (t->tbf_t == NULL) {
	/* Queue was empty */
	t->tbf_q = m;
    } else {
	/* Insert at tail */
	t->tbf_t->m_act = m;
    }

    /* Set new tail pointer */
    t->tbf_t = m;

#ifdef DIAGNOSTIC
    /* Make sure we didn't get fed a bogus mbuf */
    if (m->m_act)
	panic("tbf_queue: m_act");
#endif
    m->m_act = NULL;

    t->tbf_q_len++;

    splx(s);
}


/* 
 * processes the queue at the interface
 */
static void
tbf_process_q(vifp)
    register struct vif *vifp;
{
    register struct mbuf *m;
    register int len;
    register int s = splnet();
    register struct tbf *t = vifp->v_tbf;

    /* loop through the queue at the interface and send as many packets
     * as possible
     */
    while (t->tbf_q_len > 0) {
	m = t->tbf_q;

	len = mtod(m, struct ip *)->ip_len;

	/* determine if the packet can be sent */
	if (len <= t->tbf_n_tok) {
	    /* if so,
	     * reduce no of tokens, dequeue the packet,
	     * send the packet.
	     */
	    t->tbf_n_tok -= len;

	    t->tbf_q = m->m_act;
	    if (--t->tbf_q_len == 0)
		t->tbf_t = NULL;

	    m->m_act = NULL;
	    tbf_send_packet(vifp, m);

	} else break;
    }
    splx(s);
}

static void
tbf_reprocess_q(xvifp)
	void *xvifp;
{
    register struct vif *vifp = xvifp;
    if (ip_mrouter == NULL) 
	return;

    tbf_update_tokens(vifp);

    tbf_process_q(vifp);

    if (vifp->v_tbf->tbf_q_len)
	timeout(tbf_reprocess_q, (caddr_t)vifp, TBF_REPROCESS);
}

/* function that will selectively discard a member of the queue
 * based on the precedence value and the priority
 */
static int
tbf_dq_sel(vifp, ip)
    register struct vif *vifp;
    register struct ip *ip;
{
    register int s = splnet();
    register u_int p;
    register struct mbuf *m, *last;
    register struct mbuf **np;
    register struct tbf *t = vifp->v_tbf;

    p = priority(vifp, ip);

    np = &t->tbf_q;
    last = NULL;
    while ((m = *np) != NULL) {
	if (p > priority(vifp, mtod(m, struct ip *))) {
	    *np = m->m_act;
	    /* If we're removing the last packet, fix the tail pointer */
	    if (m == t->tbf_t)
		t->tbf_t = last;
	    m_freem(m);
	    /* it's impossible for the queue to be empty, but
	     * we check anyway. */
	    if (--t->tbf_q_len == 0)
		t->tbf_t = NULL;
	    splx(s);
	    mrtstat.mrts_drop_sel++;
	    return(1);
	}
	np = &m->m_act;
	last = m;
    }
    splx(s);
    return(0);
}

static void
tbf_send_packet(vifp, m)
    register struct vif *vifp;
    register struct mbuf *m;
{
    struct ip_moptions imo;
    int error;
    static struct route ro;
#if 0//defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	u_long vif_idx=vifp-viftable;
#endif
    int s = splnet();
	//diag_printf("vif_indx:%x[%s]:[%d].\n",vif_idx,__FUNCTION__,__LINE__);

    if (vifp->v_flags & VIFF_TUNNEL) {
	/* If tunnel options */
	ip_output(m, (struct mbuf *)0, &vifp->v_route,
		  IP_FORWARDING, (struct ip_moptions *)0);
    } else {
	imo.imo_multicast_ifp  = vifp->v_ifp;
	imo.imo_multicast_ttl  = mtod(m, struct ip *)->ip_ttl - 1;
	imo.imo_multicast_loop = 1;
	#if 0//def HAVE_IGMPPROXY
	imo.imo_multicast_addr = vifp->v_lcl_addr;
	imo.imo_multicast_vif  =vif_idx;
	#else
	imo.imo_multicast_vif  = -1;
	#endif
	/*
	 * Re-entrancy should not be a problem here, because
	 * the packets that we send out and are looped back at us
	 * should get rejected because they appear to come from
	 * the loopback interface, thus preventing looping.
	 */
	error = ip_output(m, (struct mbuf *)0, &ro,
			  IP_FORWARDING, &imo);

	if (mrtdebug & DEBUG_XMIT)
	    log(LOG_DEBUG, "phyint_send on vif %d err %d\n", 
		vifp - viftable, error);
    }
    splx(s);
}

/* determine the current time and then
 * the elapsed time (between the last time and time now)
 * in milliseconds & update the no. of tokens in the bucket
 */
static void
tbf_update_tokens(vifp)
    register struct vif *vifp;
{
    struct timeval tp;
    register u_long tm;
    register int s = splnet();
    register struct tbf *t = vifp->v_tbf;

    GET_TIME(tp);

    TV_DELTA(tp, t->tbf_last_pkt_t, tm);

    /*
     * This formula is actually
     * "time in seconds" * "bytes/second".
     *
     * (tm / 1000000) * (v_rate_limit * 1000 * (1000/1024) / 8)
     *
     * The (1000/1024) was introduced in add_vif to optimize
     * this divide into a shift.
     */
    t->tbf_n_tok += tm * vifp->v_rate_limit / 1024 / 8;
    t->tbf_last_pkt_t = tp;

    if (t->tbf_n_tok > MAX_BKT_SIZE)
	t->tbf_n_tok = MAX_BKT_SIZE;

    splx(s);
}

static int
priority(vifp, ip)
    register struct vif *vifp;
    register struct ip *ip;
{
    register int prio;

    /* temporary hack; may add general packet classifier some day */

    /*
     * The UDP port space is divided up into four priority ranges:
     * [0, 16384)     : unclassified - lowest priority
     * [16384, 32768) : audio - highest priority
     * [32768, 49152) : whiteboard - medium priority
     * [49152, 65536) : video - low priority
     */
    if (ip->ip_p == IPPROTO_UDP) {
	struct udphdr *udp = (struct udphdr *)(((char *)ip) + (ip->ip_hl << 2));
	switch (ntohs(udp->uh_dport) & 0xc000) {
	    case 0x4000:
		prio = 70;
		break;
	    case 0x8000:
		prio = 60;
		break;
	    case 0xc000:
		prio = 55;
		break;
	    default:
		prio = 50;
		break;
	}
	if (tbfdebug > 1)
		log(LOG_DEBUG, "port %x prio%d\n", ntohs(udp->uh_dport), prio);
    } else {
	    prio = 50;
    }
    return prio;
}

/*
 * End of token bucket filter modifications 
 */

int
ip_rsvp_vif_init(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
{
    int error, i, s;

    if (rsvpdebug)
	printf("ip_rsvp_vif_init: so_type = %d, pr_protocol = %d\n",
	       so->so_type, so->so_proto->pr_protocol);

    if (so->so_type != SOCK_RAW || so->so_proto->pr_protocol != IPPROTO_RSVP)
	return EOPNOTSUPP;

    /* Check mbuf. */
    error = sooptcopyin(sopt, &i, sizeof i, sizeof i);
    if (error)
	    return (error);
 
    if (rsvpdebug)
	printf("ip_rsvp_vif_init: vif = %d rsvp_on = %d\n", i, rsvp_on);
 
    s = splnet();

    /* Check vif. */
    if (!legal_vif_num(i)) {
	splx(s);
	return EADDRNOTAVAIL;
    }

    /* Check if socket is available. */
    if (viftable[i].v_rsvpd != NULL) {
	splx(s);
	return EADDRINUSE;
    }

    viftable[i].v_rsvpd = so;
    /* This may seem silly, but we need to be sure we don't over-increment
     * the RSVP counter, in case something slips up.
     */
    if (!viftable[i].v_rsvp_on) {
	viftable[i].v_rsvp_on = 1;
	rsvp_on++;
    }

    splx(s);
    return 0;
}

int
ip_rsvp_vif_done(so, sopt)
	struct socket *so;
	struct sockopt *sopt;
{
	int error, i, s;
 
	if (rsvpdebug)
		printf("ip_rsvp_vif_done: so_type = %d, pr_protocol = %d\n",
		       so->so_type, so->so_proto->pr_protocol);
 
	if (so->so_type != SOCK_RAW || 
	    so->so_proto->pr_protocol != IPPROTO_RSVP)
		return EOPNOTSUPP;
 
	error = sooptcopyin(sopt, &i, sizeof i, sizeof i);
	if (error)
		return (error);
 
	s = splnet();
 
	/* Check vif. */
	if (!legal_vif_num(i)) {
		splx(s);
		return EADDRNOTAVAIL;
	}

	if (rsvpdebug)
		printf("ip_rsvp_vif_done: v_rsvpd = %p so = %p\n",
		       viftable[i].v_rsvpd, so);

	viftable[i].v_rsvpd = NULL;
	/*
	 * This may seem silly, but we need to be sure we don't over-decrement
	 * the RSVP counter, in case something slips up.
	 */
	if (viftable[i].v_rsvp_on) {
		viftable[i].v_rsvp_on = 0;
		rsvp_on--;
	}

	splx(s);
	return 0;
}

void
ip_rsvp_force_done(so)
    struct socket *so;
{
    int vifi;
    register int s;

    /* Don't bother if it is not the right type of socket. */
    if (so->so_type != SOCK_RAW || so->so_proto->pr_protocol != IPPROTO_RSVP)
	return;

    s = splnet();

    /* The socket may be attached to more than one vif...this
     * is perfectly legal.
     */
    for (vifi = 0; vifi < numvifs; vifi++) {
	if (viftable[vifi].v_rsvpd == so) {
	    viftable[vifi].v_rsvpd = NULL;
	    /* This may seem silly, but we need to be sure we don't
	     * over-decrement the RSVP counter, in case something slips up.
	     */
	    if (viftable[vifi].v_rsvp_on) {
		viftable[vifi].v_rsvp_on = 0;
		rsvp_on--;
	    }
	}
    }

    splx(s);
    return;
}

void
rsvp_input(m, off)
	struct mbuf *m;
	int off;
{
    int vifi;
    register struct ip *ip = mtod(m, struct ip *);
    int proto = ip->ip_p;
    static struct sockaddr_in rsvp_src = { sizeof rsvp_src, AF_INET };
    register int s;
    struct ifnet *ifp;
#ifdef ALTQ
    /* support IP_RECVIF used by rsvpd rel4.2a1 */
    struct inpcb *inp;
    struct socket *so;
    struct mbuf *opts;
#endif

    if (rsvpdebug)
	printf("rsvp_input: rsvp_on %d\n",rsvp_on);

    /* Can still get packets with rsvp_on = 0 if there is a local member
     * of the group to which the RSVP packet is addressed.  But in this
     * case we want to throw the packet away.
     */
    if (!rsvp_on) {
	m_freem(m);
	return;
    }

    s = splnet();

    if (rsvpdebug)
	printf("rsvp_input: check vifs\n");

#ifdef DIAGNOSTIC
    if (!(m->m_flags & M_PKTHDR))
	    panic("rsvp_input no hdr");
#endif

    ifp = m->m_pkthdr.rcvif;
    /* Find which vif the packet arrived on. */
    for (vifi = 0; vifi < numvifs; vifi++)
	if (viftable[vifi].v_ifp == ifp)
	    break;

#ifdef ALTQ
    if (vifi == numvifs || (so = viftable[vifi].v_rsvpd) == NULL) {
#else
    if (vifi == numvifs || viftable[vifi].v_rsvpd == NULL) {
#endif
	/*
	 * If the old-style non-vif-associated socket is set,
	 * then use it.  Otherwise, drop packet since there
	 * is no specific socket for this vif.
	 */
	if (ip_rsvpd != NULL) {
	    if (rsvpdebug)
		printf("rsvp_input: Sending packet up old-style socket\n");
	    rip_input(m, off);  /* xxx */
	} else {
	    if (rsvpdebug && vifi == numvifs)
		printf("rsvp_input: Can't find vif for packet.\n");
	    else if (rsvpdebug && viftable[vifi].v_rsvpd == NULL)
		printf("rsvp_input: No socket defined for vif %d\n",vifi);
	    m_freem(m);
	}
	splx(s);
	return;
    }
    rsvp_src.sin_addr = ip->ip_src;

    if (rsvpdebug && m)
	printf("rsvp_input: m->m_len = %d, sbspace() = %ld\n",
	       m->m_len,sbspace(&(viftable[vifi].v_rsvpd->so_rcv)));

#ifdef ALTQ
    opts = NULL;
    inp = (struct inpcb *)so->so_pcb;
    if (inp->inp_flags & INP_CONTROLOPTS ||
	inp->inp_socket->so_options & SO_TIMESTAMP)
	ip_savecontrol(inp, &opts, ip, m);
    if (sbappendaddr(&so->so_rcv,
		     (struct sockaddr *)&rsvp_src,m, opts) == 0) {
	m_freem(m);
	if (opts)
	    m_freem(opts);
	if (rsvpdebug)
	    printf("rsvp_input: Failed to append to socket\n");
    }
    else {
	sorwakeup(so);
	if (rsvpdebug)
	    printf("rsvp_input: send packet up\n");
    }
#else /* !ALTQ */
    if (socket_send(viftable[vifi].v_rsvpd, m, &rsvp_src) < 0) {
	if (rsvpdebug)
	    printf("rsvp_input: Failed to append to socket\n");
    } else {
	if (rsvpdebug)
	    printf("rsvp_input: send packet up\n");
    }
#endif /* !ALTQ */

    splx(s);
}

#ifdef MROUTE_LKM
#include <sys/conf.h>
#include <sys/exec.h>
#include <sys/sysent.h>
#include <sys/lkm.h>

MOD_MISC("ip_mroute_mod")

static int
ip_mroute_mod_handle(struct lkm_table *lkmtp, int cmd)
{
	int i;
	struct lkm_misc	*args = lkmtp->private.lkm_misc;
	int err = 0;

	switch(cmd) {
		static int (*old_ip_mrouter_cmd)();
		static int (*old_ip_mrouter_done)();
		static int (*old_ip_mforward)();
		static int (*old_mrt_ioctl)();
		static void (*old_proto4_input)();
		static int (*old_legal_vif_num)();
		extern struct protosw inetsw[];

	case LKM_E_LOAD:
		if(lkmexists(lkmtp) || ip_mrtproto)
		  return(EEXIST);
		old_ip_mrouter_cmd = ip_mrouter_cmd;
		ip_mrouter_cmd = X_ip_mrouter_cmd;
		old_ip_mrouter_done = ip_mrouter_done;
		ip_mrouter_done = X_ip_mrouter_done;
		old_ip_mforward = ip_mforward;
		ip_mforward = X_ip_mforward;
		old_mrt_ioctl = mrt_ioctl;
		mrt_ioctl = X_mrt_ioctl;
              old_proto4_input = inetsw[ip_protox[ENCAP_PROTO]].pr_input;
              inetsw[ip_protox[ENCAP_PROTO]].pr_input = X_ipip_input;
		old_legal_vif_num = legal_vif_num;
		legal_vif_num = X_legal_vif_num;
		ip_mrtproto = IGMP_DVMRP;

		printf("\nIP multicast routing loaded\n");
		break;

	case LKM_E_UNLOAD:
		if (ip_mrouter)
		  return EINVAL;

		ip_mrouter_cmd = old_ip_mrouter_cmd;
		ip_mrouter_done = old_ip_mrouter_done;
		ip_mforward = old_ip_mforward;
		mrt_ioctl = old_mrt_ioctl;
              inetsw[ip_protox[ENCAP_PROTO]].pr_input = old_proto4_input;
		legal_vif_num = old_legal_vif_num;
		ip_mrtproto = 0;
		break;

	default:
		err = EINVAL;
		break;
	}

	return(err);
}

int
ip_mroute_mod(struct lkm_table *lkmtp, int cmd, int ver) {
	DISPATCH(lkmtp, cmd, ver, ip_mroute_mod_handle, ip_mroute_mod_handle,
		 nosys);
}

#endif /* MROUTE_LKM */
#endif /* MROUTING */
