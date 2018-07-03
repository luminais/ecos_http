//==========================================================================
//
//      src/sys/net/if.c
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
 * Copyright (c) 1980, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if.c	8.3 (Berkeley) 1/4/94
 * $FreeBSD: src/sys/net/if.c,v 1.85.2.9 2001/07/24 19:10:17 brooks Exp $
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/sockio.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/radix.h>
#include <net/route.h>

#if defined(INET) || defined(INET6)
/*XXX*/
#include <netinet/in.h>
#include <netinet/in_var.h>
#ifdef INET6
#include <netinet6/in6_var.h>
#include <netinet6/in6_ifattach.h>
#endif
#endif
#if NBRIDGE > 0
#include <net/ethernet.h>
#include <net/if_bridge.h>
#endif

/*
 * System initialization
 */
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WRAPPER
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif

static int ifconf __P((u_long, caddr_t));
static void ifinit __P((void *));
/*modify by lq*/
/*static*/ void if_qflush __P((struct ifqueue *));
/*end modify*/
static void if_slowtimo __P((void *));
static void link_rtrequest __P((int, struct rtentry *, struct sockaddr *));
static int  if_rtdel __P((struct radix_node *, void *));

SYSINIT(interfaces, SI_SUB_PROTO_IF, SI_ORDER_FIRST, ifinit, NULL)

int	ifqmaxlen = IFQ_MAXLEN;
struct	ifnethead ifnet;	/* depend on static init XXX */

#ifdef INET6
/*
 * XXX: declare here to avoid to include many inet6 related files..
 * should be more generalized?
 */
extern void	nd6_setmtu __P((struct ifnet *));
#endif

struct if_clone *if_clone_lookup __P((const char *, int *));
int if_clone_list __P((struct if_clonereq *));

LIST_HEAD(, if_clone) if_cloners = LIST_HEAD_INITIALIZER(if_cloners);
int if_cloners_count;

/*
 * Network interface utility routines.
 *
 * Routines with ifa_ifwith* names take sockaddr *'s as
 * parameters.
 */
/* ARGSUSED*/
void
ifinit(dummy)
	void *dummy;
{
#ifdef DEBUG_IFINIT
	struct ifnet *ifp;
	int s;

	s = splimp();
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
            log(LOG_INIT, "IFP: %p, next: %p\n", ifp, ifp->if_link.tqe_next);
        }
	splx(s);
#endif
	if_slowtimo(0);
}

int if_index = 0;
struct ifaddr **ifnet_addrs;
struct ifnet **ifindex2ifnet = NULL;

char *
_sa(struct ifaddr *ifa)
{
    struct sockaddr *sa = ifa->ifa_addr;
    static char _unknown[128];

    switch (sa->sa_family) {
    case AF_INET:
        return inet_ntoa((struct in_addr)((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr);
#ifdef INET6
    case AF_INET6:
        diag_sprintf(_unknown, "%s/%d", ip6_sprintf(IFA_IN6(ifa)), 
                     in6_mask2len(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr, NULL));
        return _unknown;
#endif
    case AF_LINK:
        diag_sprintf(_unknown, "<<%p>>", sa);
        return _unknown;
    default:
        diag_sprintf(_unknown, "[%d]", sa->sa_family);
        return _unknown;
    }
}

void
_show_ifp(struct ifnet *ifp)
{
    log_(LOG_ADDR) {
        struct ifaddr *ifa;
        diag_printf("IFP: %p (%s%d)\n", ifp, ifp->if_name, ifp->if_unit);
        TAILQ_FOREACH(ifa, &ifp->if_addrlist, ifa_list) {
            diag_printf("IFA: %p - %s\n", ifa, _sa(ifa));
        }
    }
}

/*
 * Attach an interface to the
 * list of "active" interfaces.
 */
void
if_attach(ifp)
	struct ifnet *ifp;
{
	unsigned socksize, ifasize;
	int namelen, masklen;
	char workbuf[64];
	register struct sockaddr_dl *sdl;
	register struct ifaddr *ifa;
	static int if_indexlim = 8;
	static int inited;

	if (!inited) {
		TAILQ_INIT(&ifnet);
		inited = 1;
	}

        if (ifp->if_snd.ifq_maxlen == 0) {
            ifp->if_snd.ifq_maxlen = ifqmaxlen;
        }

	TAILQ_INSERT_TAIL(&ifnet, ifp, if_link);
	ifp->if_index = ++if_index;
	/*
	 * XXX -
	 * The old code would work if the interface passed a pre-existing
	 * chain of ifaddrs to this code.  We don't trust our callers to
	 * properly initialize the tailq, however, so we no longer allow
	 * this unlikely case.
	 */
	TAILQ_INIT(&ifp->if_addrhead);        
        log_(LOG_ADDR) {
            diag_printf("%s.%d - After initialize list %p\n",   
                        __FUNCTION__, __LINE__,                         
                        &ifp->if_addrlist);                             
            _show_ifp(ifp);                                     
        }
	TAILQ_INIT(&ifp->if_prefixhead);
	LIST_INIT(&ifp->if_multiaddrs);
	getmicrotime(&ifp->if_lastchange);
	if (ifnet_addrs == 0 || if_index >= if_indexlim) {
		unsigned n = (if_indexlim <<= 1) * sizeof(ifa);
		caddr_t q = malloc(n, M_IFADDR, M_WAITOK);
		if (q == NULL) {
			diag_printf("%s: out of memory\n", __FUNCTION__);
			return;
		}
		bzero(q, n);
		if (ifnet_addrs) {
			bcopy((caddr_t)ifnet_addrs, (caddr_t)q, n/2);
			free((caddr_t)ifnet_addrs, M_IFADDR);
		}
		ifnet_addrs = (struct ifaddr **)q;

		/* grow ifindex2ifnet */
		n = if_indexlim * sizeof(struct ifnet *);
		q = malloc(n, M_IFADDR, M_WAITOK);
		if (q == NULL) {
			diag_printf("%s: out of memory\n", __FUNCTION__);
			free((caddr_t)ifnet_addrs, M_IFADDR);
			return;
		}
		bzero(q, n);
		if (ifindex2ifnet) {
			bcopy((caddr_t)ifindex2ifnet, q, n/2);
			free((caddr_t)ifindex2ifnet, M_IFADDR);
		}
		ifindex2ifnet = (struct ifnet **)q;
	}

	ifindex2ifnet[if_index] = ifp;

	/*
	 * create a Link Level name for this device
	 */
	namelen = snprintf(workbuf, sizeof(workbuf),
	    "%s%d", ifp->if_name, ifp->if_unit);
#define _offsetof(t, m) ((int)((caddr_t)&((t *)0)->m))
	masklen = _offsetof(struct sockaddr_dl, sdl_data[0]) + namelen;
	socksize = masklen + ifp->if_addrlen;
#define ROUNDUP(a) (1 + (((a) - 1) | (sizeof(long) - 1)))
	if (socksize < sizeof(*sdl))
		socksize = sizeof(*sdl);
	socksize = ROUNDUP(socksize);
	ifasize = sizeof(*ifa) + 2 * socksize;
	ifa = (struct ifaddr *)malloc(ifasize, M_IFADDR, M_WAITOK);
	if (ifa) {
		bzero((caddr_t)ifa, ifasize);
		sdl = (struct sockaddr_dl *)(ifa + 1);
		sdl->sdl_len = socksize;
		sdl->sdl_family = AF_LINK;
		bcopy(workbuf, sdl->sdl_data, namelen);
		sdl->sdl_nlen = namelen;
		sdl->sdl_index = ifp->if_index;
		sdl->sdl_type = ifp->if_type;
		ifnet_addrs[if_index - 1] = ifa;
		ifa->ifa_ifp = ifp;
		ifa->ifa_rtrequest = link_rtrequest;
		ifa->ifa_addr = (struct sockaddr *)sdl;
		sdl = (struct sockaddr_dl *)(socksize + (caddr_t)sdl);
		ifa->ifa_netmask = (struct sockaddr *)sdl;
		sdl->sdl_len = masklen;
		while (namelen != 0)
			sdl->sdl_data[--namelen] = 0xff;
		TAILQ_INSERT_HEAD(&ifp->if_addrhead, ifa, ifa_link);
                log_(LOG_ADDR) {
                    diag_printf("%s.%d - After inserting %p into list %p\n", 
                                __FUNCTION__, __LINE__,
                                ifa, &ifp->if_addrlist);
                    _show_ifp(ifp);
                }
	}
#ifdef ALTQ
	ifp->if_snd.altq_type = 0;
	ifp->if_snd.altq_disc = NULL;
	ifp->if_snd.altq_flags &= ALTQF_CANTCHANGE;
	ifp->if_snd.altq_tbr  = NULL;
	ifp->if_snd.altq_ifp  = ifp;
#endif
}

/*
 * Detach an interface, removing it from the
 * list of "active" interfaces.
 */
void
if_detach(ifp)
	struct ifnet *ifp;
{
	struct ifaddr *ifa;
	struct radix_node_head	*rnh;
	int s;
	int i;

	/*
	 * Remove routes and flush queues.
	 */
	s = splnet();
#if NBRIDGE > 0
	/* Remove the interface from any bridge it is part of.  */
	if (ifp->if_bridge)
		bridge_ifdetach(ifp);
#endif
	if_down(ifp);
#ifdef ALTQ
	if (ALTQ_IS_ENABLED(&ifp->if_snd))
		altq_disable(&ifp->if_snd);
	if (ALTQ_IS_ATTACHED(&ifp->if_snd))
		altq_detach(&ifp->if_snd);
#endif

	/*
	 * Remove address from ifnet_addrs[] and maybe decrement if_index.
	 * Clean up all addresses.
	 */
	ifnet_addrs[ifp->if_index - 1] = 0;
	while (if_index > 0 && ifnet_addrs[if_index - 1] == 0)
		if_index--;

	for (ifa = TAILQ_FIRST(&ifp->if_addrhead); ifa;
	     ifa = TAILQ_FIRST(&ifp->if_addrhead)) {
#ifdef INET
		/* XXX: Ugly!! ad hoc just for INET */
		if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
			struct ifaliasreq ifr;

			bzero(&ifr, sizeof(ifr));
			ifr.ifra_addr = *ifa->ifa_addr;
			if (ifa->ifa_dstaddr)
				ifr.ifra_broadaddr = *ifa->ifa_dstaddr;
			if (in_control(NULL, SIOCDIFADDR, (caddr_t)&ifr, ifp,
			    NULL) == 0)
				continue;
		}
#endif /* INET */
#ifdef INET6
		if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6) {
			in6_purgeaddr(ifa);
			/* ifp_addrhead is already updated */
			continue;
		}
#endif /* INET6 */
		TAILQ_REMOVE(&ifp->if_addrhead, ifa, ifa_link);
		IFAFREE(ifa);
	}

#ifdef INET6
	/*
	 * Remove all IPv6 kernel structs related to ifp.  This should be done
	 * before removing routing entries below, since IPv6 interface direct
	 * routes are expected to be removed by the IPv6-specific kernel API.
	 * Otherwise, the kernel will detect some inconsistency and bark it.
	 */
	in6_ifdetach(ifp);
#endif

	/*
	 * Delete all remaining routes using this interface
	 * Unfortuneatly the only way to do this is to slog through
	 * the entire routing table looking for routes which point
	 * to this interface...oh well...
	 */
	for (i = 1; i <= AF_MAX; i++) {
		if ((rnh = rt_tables[i]) == NULL)
			continue;
		(void) rnh->rnh_walktree(rnh, if_rtdel, ifp);
	}

	TAILQ_REMOVE(&ifnet, ifp, if_link);
	splx(s);
}

/*
 * Delete Routes for a Network Interface
 * 
 * Called for each routing entry via the rnh->rnh_walktree() call above
 * to delete all route entries referencing a detaching network interface.
 *
 * Arguments:
 *	rn	pointer to node in the routing table
 *	arg	argument passed to rnh->rnh_walktree() - detaching interface
 *
 * Returns:
 *	0	successful
 *	errno	failed - reason indicated
 *
 */
static int
if_rtdel(rn, arg)
	struct radix_node	*rn;
	void			*arg;
{
	struct rtentry	*rt = (struct rtentry *)rn;
	struct ifnet	*ifp = arg;
	int		err;

	if (rt->rt_ifp == ifp) {

		/*
		 * Protect (sorta) against walktree recursion problems
		 * with cloned routes
		 */
		if ((rt->rt_flags & RTF_UP) == 0)
			return (0);

		err = rtrequest(RTM_DELETE, rt_key(rt), rt->rt_gateway,
				rt_mask(rt), rt->rt_flags,
				(struct rtentry **) NULL);
		if (err) {
			log(LOG_WARNING, "if_rtdel: error %d\n", err);
		}
	}

	return (0);
}

/*
 * Create a clone network interface.
 */
int
if_clone_create(name, len)
	char *name;
	int len;
{
	struct if_clone *ifc;
	char *dp;
	int wildcard;
	int unit;
	int err;

	ifc = if_clone_lookup(name, &unit);
	if (ifc == NULL)
		return (EINVAL);

	if (ifunit(name) != NULL)
		return (EEXIST);

	wildcard = (unit < 0);

	err = (*ifc->ifc_create)(ifc, &unit);
	if (err != 0)
		return (err);

	/* In the wildcard case, we need to update the name. */
	if (wildcard) {
		for (dp = name; *dp != '\0'; dp++);
		if (snprintf(dp, len - (dp-name), "%d", unit) >
		    len - (dp-name) - 1) {
			/*
			 * This can only be a programmer error and
			 * there's no straightforward way to recover if
			 * it happens.
			 */
			panic("if_clone_create(): interface name too long");
		}
			
	}

	return (0);
}

/*
 * Destroy a clone network interface.
 */
int
if_clone_destroy(name)
	const char *name;
{
	struct if_clone *ifc;
	struct ifnet *ifp;

	ifc = if_clone_lookup(name, NULL);
	if (ifc == NULL)
		return (EINVAL);

	ifp = ifunit(name);
	if (ifp == NULL)
		return (ENXIO);

	if (ifc->ifc_destroy == NULL)
		return (EOPNOTSUPP);

	(*ifc->ifc_destroy)(ifp);
	return (0);
}

/*
 * Look up a network interface cloner.
 */
struct if_clone *
if_clone_lookup(name, unitp)
	const char *name;
	int *unitp;
{
	struct if_clone *ifc;
	const char *cp;
	int i;

	for (ifc = LIST_FIRST(&if_cloners); ifc != NULL;) {
		for (cp = name, i = 0; i < ifc->ifc_namelen; i++, cp++) {
			if (ifc->ifc_name[i] != *cp)
				goto next_ifc;
		}
		goto found_name;
 next_ifc:
		ifc = LIST_NEXT(ifc, ifc_list);
	}

	/* No match. */
	return ((struct if_clone *)NULL);

 found_name:
	if (*cp == '\0') {
		i = -1;
	} else {
		for (i = 0; *cp != '\0'; cp++) {
			if (*cp < '0' || *cp > '9') {
				/* Bogus unit number. */
				return (NULL);
			}
			i = (i * 10) + (*cp - '0');
		}
	}

	if (unitp != NULL)
		*unitp = i;
	return (ifc);
}

/*
 * Register a network interface cloner.
 */
void
if_clone_attach(ifc)
	struct if_clone *ifc;
{

	LIST_INSERT_HEAD(&if_cloners, ifc, ifc_list);
	if_cloners_count++;
}

/*
 * Unregister a network interface cloner.
 */
void
if_clone_detach(ifc)
	struct if_clone *ifc;
{

	LIST_REMOVE(ifc, ifc_list);
	if_cloners_count--;
}

/*
 * Provide list of interface cloners to userspace.
 */
int
if_clone_list(ifcr)
	struct if_clonereq *ifcr;
{
	char outbuf[IFNAMSIZ], *dst;
	struct if_clone *ifc;
	int count, error = 0;

	ifcr->ifcr_total = if_cloners_count;
	if ((dst = ifcr->ifcr_buffer) == NULL) {
		/* Just asking how many there are. */
		return (0);
	}

	if (ifcr->ifcr_count < 0)
		return (EINVAL);

	count = (if_cloners_count < ifcr->ifcr_count) ?
	    if_cloners_count : ifcr->ifcr_count;

	for (ifc = LIST_FIRST(&if_cloners); ifc != NULL && count != 0;
	     ifc = LIST_NEXT(ifc, ifc_list), count--, dst += IFNAMSIZ) {
		strncpy(outbuf, ifc->ifc_name, IFNAMSIZ);
		outbuf[IFNAMSIZ - 1] = '\0';	/* sanity */
		error = copyout(outbuf, dst, IFNAMSIZ);
		if (error)
			break;
	}

	return (error);
}

struct in_ifaddr *find_binddevice(u_short dev_index)
{
	struct ifnet *ifp;
	struct ifaddr *ifa;
	
//	diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
	
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	{
		if(ifp->if_flags & IFF_UP)	
		{
			if(ifp->if_index==dev_index)
			{				
				for(ifa = ifp->if_addrhead.tqh_first; ifa; ifa = ifa->ifa_link.tqe_next)
				{
					if (!(ifa->ifa_addr) || (ifa->ifa_addr->sa_family != AF_INET) )
						continue;
					return (struct in_ifaddr *)ifa;
				}						
			}				
		}
	}
//	diag_printf("%s:%d--can't find the device %d\n", __FUNCTION__,__LINE__, dev_index);
	return NULL;	
}

/*
 * Locate an interface based on a complete address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	
#define	equal(a1, a2) \
  (bcmp((caddr_t)(a1), (caddr_t)(a2), ((struct sockaddr *)(a1))->sa_len) == 0)
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	    for (ifa = ifp->if_addrhead.tqh_first; ifa;
		 ifa = ifa->ifa_link.tqe_next) {
		if (ifa->ifa_addr->sa_family != addr->sa_family)
			continue;
		if (equal(addr, ifa->ifa_addr))
		{
			//diag_printf("\n---------------\nfind1!!![%s]:[%d].\n",__FUNCTION__,__LINE__);
			return (ifa);
		}	
		if ((ifp->if_flags & IFF_BROADCAST) && ifa->ifa_broadaddr &&
		    /* IP6 doesn't have broadcast */
		    ifa->ifa_broadaddr->sa_len != 0 &&
		    equal(ifa->ifa_broadaddr, addr))
			return (ifa);
	}
	return ((struct ifaddr *)0);
}

/*
 *	Find the first device with a given source address.
 */

struct ifaddr *
ifa_ifwithsrcaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	struct sockaddr_in *ifadr_in;
	struct sockaddr_in *addr_in=(struct sockaddr_in *)addr;
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	    for (ifa = ifp->if_addrhead.tqh_first; ifa;ifa = ifa->ifa_link.tqe_next) 
		{
		
			if (ifa->ifa_addr->sa_family != addr->sa_family)
				continue;
			
			ifadr_in=(struct sockaddr_in *)ifa->ifa_addr;
			//diag_printf("ifadr:%x,%x,%x.[%s]:[%d].\n",ifadr_in->sin_family,ifadr_in->sin_len,ifadr_in->sin_addr.s_addr,__FUNCTION__,__LINE__);
			//diag_printf("addr_in:%x,%x,%x.[%s]:[%d].\n",addr_in->sin_family,addr_in->sin_len,addr_in->sin_addr.s_addr,__FUNCTION__,__LINE__);
			if((ifadr_in->sin_addr.s_addr==addr_in->sin_addr.s_addr)&&(ifadr_in->sin_family==addr_in->sin_family)){
				//diag_printf("\n---------------\nfind!!![%s]:[%d].\n",__FUNCTION__,__LINE__);
				return (ifa);
			}	
			
		}
	
	return ((struct ifaddr *)0);
}


/*
 * Locate the point to point interface with a given destination address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithdstaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	    if (ifp->if_flags & IFF_POINTOPOINT)
		for (ifa = ifp->if_addrhead.tqh_first; ifa;
		     ifa = ifa->ifa_link.tqe_next) {
			if (ifa->ifa_addr->sa_family != addr->sa_family)
				continue;
			if (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr))
				return (ifa);
	}
	return ((struct ifaddr *)0);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is most specific found.
 */
struct ifaddr *
ifa_ifwithnet(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
#ifdef __CONFIG_IPV6__
	register struct ifaddr *ifas;
#endif
	struct ifaddr *ifa_maybe = (struct ifaddr *) 0;
	u_int af = addr->sa_family;
	char *addr_data = addr->sa_data, *cplim;

	/*
	 * AF_LINK addresses can be looked up directly by their index number,
	 * so do that if we can.
	 */
	if (af == AF_LINK) {
	    register struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
	    if (sdl->sdl_index && sdl->sdl_index <= if_index)
		return (ifnet_addrs[sdl->sdl_index - 1]);
	}

	/*
	 * Scan though each interface, looking for ones that have
	 * addresses in this address family.
	 */
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
#ifdef __CONFIG_IPV6__
		/*A.wzs 2015-3-27, for ipv6 can't set a unknown default route.*/
		//printf("==> ifp->if_name: %s, ifp->if_unit: %d, ifp->if_index: %d, ifp->if_flags: %x\n", 
		//	ifp->if_name, ifp->if_unit, ifp->if_index, ifp->if_flags);
		if(!memcmp(ifp->if_name, "vlan", strlen("vlan")) && ifp->if_unit == 2)
			ifas = ifp->if_addrhead.tqh_first;
#endif		
		for (ifa = ifp->if_addrhead.tqh_first; ifa;
		     ifa = ifa->ifa_link.tqe_next) {
			register char *cp, *cp2, *cp3;

			if (ifa->ifa_addr->sa_family != af)
next:				continue;
			if (
#ifdef INET6 /* XXX: for maching gif tunnel dst as routing entry gateway */
			    addr->sa_family != AF_INET6 &&
#endif
			    ifp->if_flags & IFF_POINTOPOINT) {
				/*
				 * This is a bit broken as it doesn't
				 * take into account that the remote end may
				 * be a single node in the network we are
				 * looking for.
				 * The trouble is that we don't know the
				 * netmask for the remote end.
				 */
				if (ifa->ifa_dstaddr != 0
				    && equal(addr, ifa->ifa_dstaddr))
 					return (ifa);
			} else {
				/*
				 * if we have a special address handler,
				 * then use it instead of the generic one.
				 */
	          		if (ifa->ifa_claim_addr) {
					if ((*ifa->ifa_claim_addr)(ifa, addr)) {
						return (ifa);
					} else {
						continue;
					}
				}

				/*
				 * Scan all the bits in the ifa's address.
				 * If a bit dissagrees with what we are
				 * looking for, mask it with the netmask
				 * to see if it really matters.
				 * (A byte at a time)
				 */
				if (ifa->ifa_netmask == 0)
					continue;
				cp = addr_data;
				cp2 = ifa->ifa_addr->sa_data;
				cp3 = ifa->ifa_netmask->sa_data;
				cplim = ifa->ifa_netmask->sa_len
					+ (char *)ifa->ifa_netmask;
				while (cp3 < cplim)
					if ((*cp++ ^ *cp2++) & *cp3++)
						goto next; /* next address! */
				/*
				 * If the netmask of what we just found
				 * is more specific than what we had before
				 * (if we had one) then remember the new one
				 * before continuing to search
				 * for an even better one.
				 */
				if (ifa_maybe == 0 ||
				    rn_refines((caddr_t)ifa->ifa_netmask,
				    (caddr_t)ifa_maybe->ifa_netmask))
					ifa_maybe = ifa;
			}
		}
	}
#ifdef __CONFIG_IPV6__
	/*A.wzs 2015-3-27, for ipv6 can't set an unknown default route.*/
	if(ifa_maybe == 0)
		ifa_maybe = ifas;
#endif	
	return (ifa_maybe);
}

/*
 * Find an interface address specific to an interface best matching
 * a given address.
 */
struct ifaddr *
ifaof_ifpforaddr(addr, ifp)
	struct sockaddr *addr;
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;
	register char *cp, *cp2, *cp3;
	register char *cplim;
	struct ifaddr *ifa_maybe = 0;
	u_int af = addr->sa_family;

	if (af >= AF_MAX)
		return (0);
	for (ifa = ifp->if_addrhead.tqh_first; ifa;
	     ifa = ifa->ifa_link.tqe_next) {
		if (ifa->ifa_addr->sa_family != af)
			continue;
		if (ifa_maybe == 0)
			ifa_maybe = ifa;
		if (ifa->ifa_netmask == 0) {
			if (equal(addr, ifa->ifa_addr) ||
			    (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
				return (ifa);
			continue;
		}
		if (ifp->if_flags & IFF_POINTOPOINT) {
			if (equal(addr, ifa->ifa_dstaddr))
				return (ifa);
		} else {
			cp = addr->sa_data;
			cp2 = ifa->ifa_addr->sa_data;
			cp3 = ifa->ifa_netmask->sa_data;
			cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
			for (; cp3 < cplim; cp3++)
				if ((*cp++ ^ *cp2++) & *cp3)
					break;
			if (cp3 == cplim)
				return (ifa);
		}
	}
	return (ifa_maybe);
}

/*
 * Default action when installing a route with a Link Level gateway.
 * Lookup an appropriate real ifa to point to.
 * This should be moved to /sys/net/link.c eventually.
 */
static void
link_rtrequest(cmd, rt, sa)
	int cmd;
	register struct rtentry *rt;
	struct sockaddr *sa;
{
	register struct ifaddr *ifa;
	struct sockaddr *dst;
	struct ifnet *ifp;

	if (cmd != RTM_ADD || ((ifa = rt->rt_ifa) == 0) ||
	    ((ifp = ifa->ifa_ifp) == 0) || ((dst = rt_key(rt)) == 0))
		return;
	ifa = ifaof_ifpforaddr(dst, ifp);
	if (ifa) {
		IFAFREE(rt->rt_ifa);
		rt->rt_ifa = ifa;
		ifa->ifa_refcnt++;
		if (ifa->ifa_rtrequest && ifa->ifa_rtrequest != link_rtrequest)
			ifa->ifa_rtrequest(cmd, rt, sa);
	}
}

/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_unroute(ifp, flag, fam)
	register struct ifnet *ifp;
	int flag, fam;
{
	register struct ifaddr *ifa;

	ifp->if_flags &= ~flag;
	getmicrotime(&ifp->if_lastchange);
	TAILQ_FOREACH(ifa, &ifp->if_addrhead, ifa_link)
		if (fam == PF_UNSPEC || (fam == ifa->ifa_addr->sa_family))
			pfctlinput(PRC_IFDOWN, ifa->ifa_addr);
	if_qflush(&ifp->if_snd);
	rt_ifmsg(ifp);
}

/*
 * Mark an interface up and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_route(ifp, flag, fam)
	register struct ifnet *ifp;
	int flag, fam;
{
	register struct ifaddr *ifa;

	ifp->if_flags |= flag;
	getmicrotime(&ifp->if_lastchange);
	TAILQ_FOREACH(ifa, &ifp->if_addrhead, ifa_link)
		if (fam == PF_UNSPEC || (fam == ifa->ifa_addr->sa_family))
			pfctlinput(PRC_IFUP, ifa->ifa_addr);
	rt_ifmsg(ifp);
#ifdef INET6
	in6_if_up(ifp);
#endif
}

/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_down(ifp)
	register struct ifnet *ifp;
{

	if_unroute(ifp, IFF_UP, AF_UNSPEC);
}

/*
 * Mark an interface up and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_up(ifp)
	register struct ifnet *ifp;
{

	if_route(ifp, IFF_UP, AF_UNSPEC);
}

/*
 * Flush an interface queue.
 */
 /*modify by lq*/
/*static*/ void
if_qflush(ifq)
#ifdef ALTQ
	struct ifaltq *ifq;
#else
	register struct ifqueue *ifq;
#endif
{
	register struct mbuf *m, *n;

#ifdef ALTQ
	if (ALTQ_IS_ENABLED(ifq))
		ALTQ_PURGE(ifq);
#endif
	n = ifq->ifq_head;
	while ((m = n) != 0) {
		n = m->m_act;
		m_freem(m);
	}
	ifq->ifq_head = 0;
	ifq->ifq_tail = 0;
	ifq->ifq_len = 0;
}

/*
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 */
static void
if_slowtimo(arg)
	void *arg;
{
	register struct ifnet *ifp;
	int s = splimp();

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		if (ifp->if_timer == 0 || --ifp->if_timer)
			continue;
		if (ifp->if_watchdog)
			(*ifp->if_watchdog)(ifp);
	}
	splx(s);
	timeout(if_slowtimo, (void *)0, hz / IFNET_SLOWHZ);
}

/*
 * Map interface name to
 * interface structure pointer.
 */
struct ifnet *
ifunit(const char *name)
{
	char namebuf[IFNAMSIZ + 1];
	const char *cp;
	struct ifnet *ifp;
	int unit;
	unsigned len, m;
	char c;

	len = strlen(name);
	if (len < 2 || len > IFNAMSIZ)
		return NULL;
	cp = name + len - 1;
	c = *cp;
	if (c < '0' || c > '9')
		return NULL;		/* trailing garbage */
	unit = 0;
	m = 1;
	do {
		if (cp == name)
			return NULL;	/* no interface name */
		unit += (c - '0') * m;
		if (unit > 1000000)
			return NULL;	/* number is unreasonable */
		m *= 10;
		c = *--cp;
	} while (c >= '0' && c <= '9');
	len = cp - name + 1;
	bcopy(name, namebuf, len);
	namebuf[len] = '\0';
	/*
	 * Now search all the interfaces for this name/number
	 */
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		if(ifp->if_name){
			if (strcmp(ifp->if_name, namebuf))
				continue;
		}
		else
			continue;

		if (unit == ifp->if_unit)
			break;
	}
	return (ifp);
}

#if 1
//#ifdef BCM47XX

//roy+++,2010/09/27
/*
 * Map interface name to
 * interface structure pointer.
 */
struct ifnet *ifunit2(const char *name)
{
	return ifunit(name);
}
//+++I don't now why?we cann't reference ifunit in filcmd.c directly
/*
 * Map interface name to
 * interface structure pointer.
 */
static int
ifname(struct ifreq *ifr)
{
	struct ifnet *ifp;

	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
		if (ifp->if_index == ifr->ifr_ifindex) {
			strcpy(ifr->ifr_name, ifp->if_xname);
			return 0;
		}
	}
	return ENXIO;
}
#endif

/*
 * Map interface name in a sockaddr_dl to
 * interface structure pointer.
 */
struct ifnet *
if_withname(sa)
	struct sockaddr *sa;
{
	char ifname[IFNAMSIZ+1];
	struct sockaddr_dl *sdl = (struct sockaddr_dl *)sa;

	if ( (sa->sa_family != AF_LINK) || (sdl->sdl_nlen == 0) ||
	     (sdl->sdl_nlen > IFNAMSIZ) )
		return NULL;

	/*
	 * ifunit wants a null-terminated name.  It may not be null-terminated
	 * in the sockaddr.  We don't want to change the caller's sockaddr,
	 * and there might not be room to put the trailing null anyway, so we
	 * make a local copy that we know we can null terminate safely.
	 */

	bcopy(sdl->sdl_data, ifname, sdl->sdl_nlen);
	ifname[sdl->sdl_nlen] = '\0';
	return ifunit(ifname);
}


// TEMP?
#define _NAME(s) \
  case s: return #s; break; 
char *
_ioctl_name(u_long cmd)
{
    static char unknown[32];

    switch (cmd) {
        _NAME(SIOCSHIWAT);
        _NAME(SIOCGHIWAT);
        _NAME(SIOCSLOWAT);
        _NAME(SIOCGLOWAT);
        _NAME(SIOCATMARK);
        _NAME(SIOCSPGRP);
        _NAME(SIOCGPGRP);
        _NAME(SIOCADDRT);
        _NAME(SIOCDELRT);
//_NAME(SIOCGETVIFCNT);
//_NAME(SIOCGETSGCNT);
        _NAME(SIOCSIFADDR);
        _NAME(OSIOCGIFADDR);
        _NAME(SIOCGIFADDR);
        _NAME(SIOCSIFDSTADDR);
        _NAME(OSIOCGIFDSTADDR);
        _NAME(SIOCGIFDSTADDR);
        _NAME(SIOCSIFFLAGS);
        _NAME(SIOCGIFFLAGS);
        _NAME(OSIOCGIFBRDADDR);
        _NAME(SIOCGIFBRDADDR);
        _NAME(SIOCSIFBRDADDR);
        _NAME(OSIOCGIFCONF);
        _NAME(SIOCGIFCONF);
        _NAME(OSIOCGIFNETMASK);
        _NAME(SIOCGIFNETMASK);
        _NAME(SIOCSIFNETMASK);
        _NAME(SIOCGIFMETRIC);
        _NAME(SIOCSIFMETRIC);
        _NAME(SIOCDIFADDR);
        _NAME(SIOCAIFADDR);
        _NAME(SIOCALIFADDR);
        _NAME(SIOCGLIFADDR);
        _NAME(SIOCDLIFADDR);
        _NAME(SIOCADDMULTI);
        _NAME(SIOCDELMULTI);
        _NAME(SIOCGIFMTU);
        _NAME(SIOCSIFMTU);
        _NAME(SIOCGIFPHYS);
        _NAME(SIOCSIFPHYS);
        _NAME(SIOCSIFMEDIA);
        _NAME(SIOCGIFMEDIA);
        _NAME(SIOCSIFPHYADDR  );
        _NAME(SIOCGIFPSRCADDR);
        _NAME(SIOCGIFPDSTADDR);
        _NAME(SIOCDIFPHYADDR);
        _NAME(SIOCSLIFPHYADDR);
        _NAME(SIOCGLIFPHYADDR);
        _NAME(SIOCSIFGENERIC);
        _NAME(SIOCGIFGENERIC);
        _NAME(SIOCGIFSTATUS);
        _NAME(SIOCSIFLLADDR);
        _NAME(SIOCIFCREATE);
        _NAME(SIOCIFDESTROY);
        _NAME(SIOCIFGCLONERS);
        _NAME(FIONBIO);
        _NAME(FIOASYNC);
        _NAME(FIONREAD);
        _NAME(SIOCGIFHWADDR);
        _NAME(SIOCSIFHWADDR);
        _NAME(SIOCGIFSTATSUD);
        _NAME(SIOCGIFSTATS);
    default:
        diag_sprintf(unknown, "0x%08lx", cmd);
        return unknown;
    }
}
// TEMP?

/*
 * Interface ioctls.
 */
int
ifioctl(so, cmd, data, p)
	struct socket *so;
	u_long cmd;
	caddr_t data;
	struct proc *p;
{
	register struct ifnet *ifp;
	register struct ifreq *ifr;
	struct ifstat *ifs;
	int error;
	short oif_flags;

        log(LOG_IOCTL, "%s: cmd: %s, data:\n", __FUNCTION__, _ioctl_name(cmd));
	switch (cmd) {

	case SIOCGIFCONF:
	case OSIOCGIFCONF:
		return (ifconf(cmd, data));
	}
	ifr = (struct ifreq *)data;
	
	
	switch (cmd) {
	case SIOCIFCREATE:
	case SIOCIFDESTROY:
		return ((cmd == SIOCIFCREATE) ?
			if_clone_create(ifr->ifr_name, sizeof(ifr->ifr_name)) :
			if_clone_destroy(ifr->ifr_name));
	
	case SIOCIFGCLONERS:
		return (if_clone_list((struct if_clonereq *)data));
	}
#if 1
//#ifdef BCM47XX
	if (cmd == SIOCGIFNAME)
		return ifname(ifr);
#endif

	ifp = ifunit(ifr->ifr_name);
	if (ifp == 0)
		return (ENXIO);

	switch (cmd) {
#if 1
//#ifdef BCM47XX
	case SIOCGIFFLTFLAGS:
		ifr->ifr_flags = ifp->if_fltflags;
		break;
	case SIOCSIFFLTFLAGS:
		ifp->if_fltflags = ifr->ifr_flags;
		break;	
#ifdef __CONFIG_IPV6__
	case SIOCGIFINDEX:  //add by hzz
		ifr->ifr_ifindex = ifp->if_index;
		break;
#endif
#endif
	case SIOCGIFFLAGS:
		ifr->ifr_flags = ifp->if_flags;
		break;

	case SIOCGIFMETRIC:
		ifr->ifr_metric = ifp->if_metric;
		break;

	case SIOCGIFMTU:
		ifr->ifr_mtu = ifp->if_mtu;
		break;

	case SIOCGIFPHYS:
		ifr->ifr_phys = ifp->if_physical;
		break;

	case SIOCSIFFLAGS:
		ifr->ifr_prevflags = ifp->if_flags;
		if (ifp->if_flags & IFF_SMART) {
			/* Smart drivers twiddle their own routes */
		} else if (ifp->if_flags & IFF_UP &&
		    (ifr->ifr_flags & IFF_UP) == 0) {
			int s = splimp();
			if_down(ifp);
			splx(s);
		} else if (ifr->ifr_flags & IFF_UP &&
		    (ifp->if_flags & IFF_UP) == 0) {
			int s = splimp();
			if_up(ifp);
			splx(s);
		}
		ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
			(ifr->ifr_flags &~ IFF_CANTCHANGE);
		if (ifp->if_ioctl)
			(void) (*ifp->if_ioctl)(ifp, cmd, data);
		getmicrotime(&ifp->if_lastchange);
		break;
#ifdef __CONFIG_IPV6__
//ethctl usage
    case SIOCGMIIPHY:
    case SIOCGMIIREG:
    case SIOCSMIIREG:
		if (!ifp->if_ioctl)
		    return EOPNOTSUPP;
		error = (*ifp->if_ioctl)(ifp, cmd, data);
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		//diag_printf("error = %d!\n", error);	
		return(error);
		
//ethctl usage   add by hzz
#endif
	case SIOCSIFMETRIC:
		ifp->if_metric = ifr->ifr_metric;
		getmicrotime(&ifp->if_lastchange);
		break;

	case SIOCSIFPHYS:
		if (!ifp->if_ioctl)
		        return EOPNOTSUPP;
		error = (*ifp->if_ioctl)(ifp, cmd, data);
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		return(error);

	case SIOCSIFMTU:
	{
		u_long oldmtu = ifp->if_mtu;

		if (ifp->if_ioctl == NULL)
			return (EOPNOTSUPP);
		if (ifr->ifr_mtu < IF_MINMTU || ifr->ifr_mtu > IF_MAXMTU)
			return (EINVAL);
		error = (*ifp->if_ioctl)(ifp, cmd, data);
		if (error == 0) {
			getmicrotime(&ifp->if_lastchange);
			rt_ifmsg(ifp);
		}
		/*
		 * If the link MTU changed, do network layer specific procedure.
		 */
		if (ifp->if_mtu != oldmtu) {
#ifdef INET6
			nd6_setmtu(ifp);
#endif
		}
		return (error);
	}

	case SIOCADDMULTI:
	case SIOCDELMULTI:

		/* Don't allow group membership on non-multicast interfaces. */
		if ((ifp->if_flags & IFF_MULTICAST) == 0)
			return EOPNOTSUPP;

		/* Don't let users screw up protocols' entries. */
		if (ifr->ifr_addr.sa_family != AF_LINK)
			return EINVAL;

                log(LOG_IOCTL, "%s: %s Multi\n", __FUNCTION__, 
                    (cmd == SIOCADDMULTI) ? "Add" : "Del");
                log_dump(LOG_IOCTL, &ifr->ifr_addr, 32);
		if (cmd == SIOCADDMULTI) {
			struct ifmultiaddr *ifma;
			error = if_addmulti(ifp, &ifr->ifr_addr, &ifma);
		} else {
			error = if_delmulti(ifp, &ifr->ifr_addr);
		}
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		return error;

	case SIOCSIFPHYADDR:
	case SIOCDIFPHYADDR:
#ifdef INET6
	case SIOCSIFPHYADDR_IN6:
#endif
	case SIOCSLIFPHYADDR:
        case SIOCSIFMEDIA:
	case SIOCSIFGENERIC:
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		error = (*ifp->if_ioctl)(ifp, cmd, data);
		if (error == 0)
			getmicrotime(&ifp->if_lastchange);
		return error;

	case SIOCGIFSTATUS:	
#ifndef CONFIG_RTL_819X
		ifs = (struct ifstat *)data;
		ifs->ascii[0] = '\0';
#endif
	case SIOCGIFPSRCADDR:
	case SIOCGIFPDSTADDR:
	case SIOCGLIFPHYADDR:
	case SIOCGIFMEDIA:
	case SIOCGIFGENERIC:
    case SIOCGIFSTATS:
    case SIOCGIFSTATSUD:
		if (ifp->if_ioctl == 0)
			return (EOPNOTSUPP);		
		return ((*ifp->if_ioctl)(ifp, cmd, data));

	case SIOCSIFLLADDR:
		return if_setlladdr(ifp,
		    (u_char *)ifr->ifr_addr.sa_data, ifr->ifr_addr.sa_len);

	default:
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WRAPPER
		if ((cmd >= SIOCDEVPRIVATE && cmd <= SIOCDEVPRIVATE + 15) ||
		    (cmd >= SIOCIWFIRST && cmd <= SIOCIWLAST)) {
			if (ifp->if_ioctl == 0)
				return (EOPNOTSUPP);
			error = (*ifp->if_ioctl)(ifp, cmd, data);
			return(error);
		}
#endif
		oif_flags = ifp->if_flags;
		if (so->so_proto == 0)
			return (EOPNOTSUPP);
#ifndef COMPAT_43
		error = ((*so->so_proto->pr_usrreqs->pru_control)(so, cmd,
								 data,
								 ifp, p));
#else
	    {
		int ocmd = cmd;

		switch (cmd) {

		case SIOCSIFDSTADDR:
		case SIOCSIFADDR:
		case SIOCSIFBRDADDR:
		case SIOCSIFNETMASK:
#if BYTE_ORDER != BIG_ENDIAN
			if (ifr->ifr_addr.sa_family == 0 &&
			    ifr->ifr_addr.sa_len < 16) {
				ifr->ifr_addr.sa_family = ifr->ifr_addr.sa_len;
				ifr->ifr_addr.sa_len = 16;
			}
#else
			if (ifr->ifr_addr.sa_len == 0)
				ifr->ifr_addr.sa_len = 16;
#endif
			break;

		case OSIOCGIFADDR:
			cmd = SIOCGIFADDR;
			break;

		case OSIOCGIFDSTADDR:
			cmd = SIOCGIFDSTADDR;
			break;

		case OSIOCGIFBRDADDR:
			cmd = SIOCGIFBRDADDR;
			break;

		case OSIOCGIFNETMASK:
			cmd = SIOCGIFNETMASK;
		}
		error =  ((*so->so_proto->pr_usrreqs->pru_control)(so,
								   cmd,
								   data,
								   ifp, p));
		switch (ocmd) {

		case OSIOCGIFADDR:
		case OSIOCGIFDSTADDR:
		case OSIOCGIFBRDADDR:
		case OSIOCGIFNETMASK:
			*(u_short *)&ifr->ifr_addr = ifr->ifr_addr.sa_family;

		}
	    }
#endif /* COMPAT_43 */

		if ((oif_flags ^ ifp->if_flags) & IFF_UP) {
#ifdef INET6
#define DELAY cyg_thread_delay
			DELAY(100);/* XXX: temporary workaround for fxp issue*/
			if (ifp->if_flags & IFF_UP) {
				int s = splimp();
				in6_if_up(ifp);
				splx(s);
			}
#endif
		}
		return (error);

	}
	return (0);
}

/*
 * Set/clear promiscuous mode on interface ifp based on the truth value
 * of pswitch.  The calls are reference counted so that only the first
 * "on" request actually has an effect, as does the final "off" request.
 * Results are undefined if the "off" and "on" requests are not matched.
 */
int
ifpromisc(ifp, pswitch)
	struct ifnet *ifp;
	int pswitch;
{
	struct ifreq ifr;
	int error;
	int oldflags;

	oldflags = ifp->if_flags;
	if (pswitch) {
		/*
		 * If the device is not configured up, we cannot put it in
		 * promiscuous mode.
		 */
		if ((ifp->if_flags & IFF_UP) == 0)
			return (ENETDOWN);
		if (ifp->if_pcount++ != 0)
			return (0);
		ifp->if_flags |= IFF_PROMISC;
		log(LOG_INFO, "%s%d: promiscuous mode enabled\n",
		    ifp->if_name, ifp->if_unit);
	} else {
		if (--ifp->if_pcount > 0)
			return (0);
		ifp->if_flags &= ~IFF_PROMISC;
		log(LOG_INFO, "%s%d: promiscuous mode disabled\n",
		    ifp->if_name, ifp->if_unit);
	}
	ifr.ifr_flags = ifp->if_flags;
	error = (*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, (caddr_t)&ifr);
	if (error == 0)
		rt_ifmsg(ifp);
	else
		ifp->if_flags = oldflags;
	return error;
}

/*
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 */
/*ARGSUSED*/
static int
ifconf(cmd, data)
	u_long cmd;
	caddr_t data;
{
	register struct ifconf *ifc = (struct ifconf *)data;
	register struct ifnet *ifp = ifnet.tqh_first;
	register struct ifaddr *ifa;
	struct ifreq ifr, *ifrp;
	int space = ifc->ifc_len, error = 0;

	ifrp = ifc->ifc_req;
	for (; space > sizeof (ifr) && ifp; ifp = ifp->if_link.tqe_next) {
		char workbuf[64];
		int ifnlen, addrs;

		ifnlen = snprintf(workbuf, sizeof(workbuf),
		    "%s%d", ifp->if_name, ifp->if_unit);
		if(ifnlen + 1 > sizeof ifr.ifr_name) {
			error = ENAMETOOLONG;
			break;
		} else {
			strcpy(ifr.ifr_name, workbuf);
		}

		addrs = 0;
		ifa = ifp->if_addrhead.tqh_first;
		for ( ; space > sizeof (ifr) && ifa;
		    ifa = ifa->ifa_link.tqe_next) {
			register struct sockaddr *sa = ifa->ifa_addr;
			addrs++;
#ifdef COMPAT_43
			if (cmd == OSIOCGIFCONF) {
				struct osockaddr *osa =
					 (struct osockaddr *)&ifr.ifr_addr;
				ifr.ifr_addr = *sa;
				osa->sa_family = sa->sa_family;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else
#endif
			if (sa->sa_len <= sizeof(*sa)) {
				ifr.ifr_addr = *sa;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else {
				if (space < sizeof (ifr) + sa->sa_len -
					    sizeof(*sa))
					break;
				space -= sa->sa_len - sizeof(*sa);
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr.ifr_name));
				if (error == 0)
				    error = copyout((caddr_t)sa,
				      (caddr_t)&ifrp->ifr_addr, sa->sa_len);
				ifrp = (struct ifreq *)
					(sa->sa_len + (caddr_t)&ifrp->ifr_addr);
			}
			if (error)
				break;
			space -= sizeof (ifr);
		}
		if (error)
			break;
		if (!addrs) {
			bzero((caddr_t)&ifr.ifr_addr, sizeof(ifr.ifr_addr));
			error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
			    sizeof (ifr));
			if (error)
				break;
			space -= sizeof (ifr);
			ifrp++;
		}
	}
	ifc->ifc_len -= space;
	return (error);
}

/*
 * Just like if_promisc(), but for all-multicast-reception mode.
 */
int
if_allmulti(ifp, onswitch)
	struct ifnet *ifp;
	int onswitch;
{
	int error = 0;
#ifdef __CONFIG_IPV6__
	struct ifreq ifr; //add by hzz
#endif
	int s = splimp();

    if(NULL == ifp)
    {
        printf("%s [%d]Error happen.\n", __FUNCTION__, __LINE__);
        return error;
    }
	if (onswitch) {
		if (ifp->if_amcount++ == 0) {
			ifp->if_flags |= IFF_ALLMULTI;
#ifndef __CONFIG_IPV6__
			error = ifp->if_ioctl(ifp, SIOCSIFFLAGS, 0);
#else
			ifr.ifr_flags = 0;//add by hzz
			error = ifp->if_ioctl(ifp, SIOCSIFFLAGS, (caddr_t)&ifr); //add by hzz
#endif			
		}
	} else {
		if (ifp->if_amcount > 1) {
			ifp->if_amcount--;
		} else {
			ifp->if_amcount = 0;
			ifp->if_flags &= ~IFF_ALLMULTI;
			#ifndef CONFIG_RTL_819X
			error = ifp->if_ioctl(ifp, SIOCSIFFLAGS, 0);
			#endif
		}
	}
	splx(s);

	if (error == 0)
		rt_ifmsg(ifp);
	return error;
}

/*
 * Add a multicast listenership to the interface in question.
 * The link layer provides a routine which converts
 */
int
if_addmulti(ifp, sa, retifma)
	struct ifnet *ifp;	/* interface to manipulate */
	struct sockaddr *sa;	/* address to add */
	struct ifmultiaddr **retifma;
{
	struct sockaddr *llsa, *dupsa;
	int error, s;
	struct ifmultiaddr *ifma;

	/*
	 * If the matching multicast address already exists
	 * then don't add a new one, just add a reference
	 */
	for (ifma = ifp->if_multiaddrs.lh_first; ifma;
	     ifma = ifma->ifma_link.le_next) {
		if (equal(sa, ifma->ifma_addr)) {
			ifma->ifma_refcount++;
			if (retifma)
				*retifma = ifma;
			return 0;
		}
	}

	/*
	 * Give the link layer a chance to accept/reject it, and also
	 * find out which AF_LINK address this maps to, if it isn't one
	 * already.
	 */
	if (ifp->if_resolvemulti) {
		error = ifp->if_resolvemulti(ifp, &llsa, sa);
		if (error) return error;
	} else {
		llsa = 0;
	}

	MALLOC(ifma, struct ifmultiaddr *, sizeof *ifma, M_IFMADDR, M_WAITOK);
	if (ifma == NULL) {
		return ENOBUFS;
	}
	MALLOC(dupsa, struct sockaddr *, sa->sa_len, M_IFMADDR, M_WAITOK);
	if (dupsa == NULL) {
		FREE(ifma, M_IFMADDR);
		return ENOBUFS;
	}
	bcopy(sa, dupsa, sa->sa_len);

	ifma->ifma_addr = dupsa;
	ifma->ifma_lladdr = llsa;
	ifma->ifma_ifp = ifp;
	ifma->ifma_refcount = 1;
	ifma->ifma_protospec = 0;
	rt_newmaddrmsg(RTM_NEWMADDR, ifma);

	/*
	 * Some network interfaces can scan the address list at
	 * interrupt time; lock them out.
	 */
	s = splimp();
	LIST_INSERT_HEAD(&ifp->if_multiaddrs, ifma, ifma_link);
	splx(s);
	*retifma = ifma;

	if (llsa != 0) {
		for (ifma = ifp->if_multiaddrs.lh_first; ifma;
		     ifma = ifma->ifma_link.le_next) {
			if (equal(ifma->ifma_addr, llsa))
				break;
		}
		if (ifma) {
			ifma->ifma_refcount++;
		} else {
			MALLOC(ifma, struct ifmultiaddr *, sizeof *ifma,
			       M_IFMADDR, M_WAITOK);
			if (ifma == NULL)
				return ENOBUFS;
			MALLOC(dupsa, struct sockaddr *, llsa->sa_len,
			       M_IFMADDR, M_WAITOK);
			if (dupsa == NULL) {
				FREE(ifma, M_IFMADDR);
				return ENOBUFS;
			}
			bcopy(llsa, dupsa, llsa->sa_len);
			ifma->ifma_addr = dupsa;
			ifma->ifma_ifp = ifp;
			ifma->ifma_refcount = 1;
			s = splimp();
			LIST_INSERT_HEAD(&ifp->if_multiaddrs, ifma, ifma_link);
			splx(s);
		}
	}
	/*
	 * We are certain we have added something, so call down to the
	 * interface to let them know about it.
	 */
	s = splimp();
	ifp->if_ioctl(ifp, SIOCADDMULTI, 0);
	splx(s);

	return 0;
}

/*
 * Remove a reference to a multicast address on this interface.  Yell
 * if the request does not match an existing membership.
 */
int
if_delmulti(ifp, sa)
	struct ifnet *ifp;
	struct sockaddr *sa;
{
	struct ifmultiaddr *ifma;
	int s;

	if (sa == 0) {
		diag_printf("[%s][%d][luminais] sa == 0\n", __FUNCTION__, __LINE__);
		return 0;
	}
	for (ifma = ifp->if_multiaddrs.lh_first; ifma;
	     ifma = ifma->ifma_link.le_next) {
		if(!ifma->ifma_addr)
			diag_printf("[%s][%d][luminais] ifma->ifma_addr == NULL\n", __FUNCTION__, __LINE__);
		if (ifma->ifma_addr && equal(sa, ifma->ifma_addr))
			break;
	}
	if (ifma == 0)
		return ENOENT;

	if (ifma->ifma_refcount > 1) {
		ifma->ifma_refcount--;
		return 0;
	}

	rt_newmaddrmsg(RTM_DELMADDR, ifma);
	sa = ifma->ifma_lladdr;
	s = splimp();
	LIST_REMOVE(ifma, ifma_link);
	/*
	 * Make sure the interface driver is notified
	 * in the case of a link layer mcast group being left.
	 */
	if (ifma->ifma_addr->sa_family == AF_LINK && sa == 0)
		ifp->if_ioctl(ifp, SIOCDELMULTI, 0);
	splx(s);
	free(ifma->ifma_addr, M_IFMADDR);
	free(ifma, M_IFMADDR);
	if (sa == 0)
		return 0;

	/*
	 * Now look for the link-layer address which corresponds to
	 * this network address.  It had been squirreled away in
	 * ifma->ifma_lladdr for this purpose (so we don't have
	 * to call ifp->if_resolvemulti() again), and we saved that
	 * value in sa above.  If some nasty deleted the
	 * link-layer address out from underneath us, we can deal because
	 * the address we stored was is not the same as the one which was
	 * in the record for the link-layer address.  (So we don't complain
	 * in that case.)
	 */
	for (ifma = ifp->if_multiaddrs.lh_first; ifma;
	     ifma = ifma->ifma_link.le_next)
		if (equal(sa, ifma->ifma_addr))
			break;
	if (ifma == 0)
		return 0;

	if (ifma->ifma_refcount > 1) {
		ifma->ifma_refcount--;
		return 0;
	}

	s = splimp();
	LIST_REMOVE(ifma, ifma_link);
	ifp->if_ioctl(ifp, SIOCDELMULTI, 0);
	splx(s);
	free(ifma->ifma_addr, M_IFMADDR);
	free(sa, M_IFMADDR);
	free(ifma, M_IFMADDR);

	return 0;
}

/*
 * Set the link layer address on an interface.
 *
 * At this time we only support certain types of interfaces,
 * and we don't allow the length of the address to change.
 */
int
if_setlladdr(struct ifnet *ifp, const u_char *lladdr, int len)
{
	struct sockaddr_dl *sdl;
	struct ifaddr *ifa;

	ifa = ifnet_addrs[ifp->if_index - 1];
	if (ifa == NULL)
		return (EINVAL);
	sdl = (struct sockaddr_dl *)ifa->ifa_addr;
	if (sdl == NULL)
		return (EINVAL);
	if (len != sdl->sdl_alen)	/* don't allow length to change */
		return (EINVAL);
	switch (ifp->if_type) {
	case IFT_ETHER:			/* these types use struct arpcom */
	case IFT_FDDI:
	case IFT_XETHER:
	case IFT_ISO88025:
	case IFT_L2VLAN:
		bcopy(lladdr, ((struct arpcom *)ifp->if_softc)->ac_enaddr, len);
		bcopy(lladdr, LLADDR(sdl), len);
		break;
	default:
		return (ENODEV);
	}
	/*
	 * If the interface is already up, we need
	 * to re-init it in order to reprogram its
	 * address filter.
	 */
	if ((ifp->if_flags & IFF_UP) != 0) {
		ifp->if_flags &= ~IFF_UP;
		(*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, NULL);
		ifp->if_flags |= IFF_UP;
		(*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, NULL);
	}
	return (0);
}

struct ifmultiaddr *
ifmaof_ifpforaddr(sa, ifp)
	struct sockaddr *sa;
	struct ifnet *ifp;
{
	struct ifmultiaddr *ifma;
	
	for (ifma = ifp->if_multiaddrs.lh_first; ifma;
	     ifma = ifma->ifma_link.le_next)
		if (equal(ifma->ifma_addr, sa))
			break;

	return ifma;
}

#if 1//ined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
struct ifnet* rtl_getIfpByName(char *name)
{
	int unit = -1;
	register struct ifnet *ifp;

	if(('0'<=name[strlen(name)-1])&&('9'>=name[strlen(name)-1]))
		unit = name[strlen(name)-1] - '0';
	
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	{
		if((strncmp(ifp->if_name, name, strlen(name)-1)==0) && (strlen(ifp->if_name)==strlen(name)-1) && (unit == ifp->if_unit))
		{
			return ifp;
		}
	}
	return NULL;
}
#endif

#ifdef INET6   //add by hzz

static void
_mask6(struct sockaddr *sa, char *buf, int _len)
{
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
  int addrword = 0;
  int bits = 0;
  int index;

  while (addrword < 4) {
    if (sin6->sin6_addr.s6_addr32[addrword] == 0) {
      break;
    }
    HAL_LSBIT_INDEX(index, sin6->sin6_addr.s6_addr32[addrword++]);
    bits += (32-index);
    if (index != 0) {
      break;
    }
  }
  diag_sprintf(buf, "%d", bits);
}


static int
_dumpv6entry(struct radix_node *rn, void *vifp)
{
    struct rtentry *rt = (struct rtentry *)rn;
    struct sockaddr *dst, *gate, *netmask, *genmask;
    char addr[64], *cp;

    dst = rt_key(rt);
    gate = rt->rt_gateway;
    netmask = rt_mask(rt);
    genmask = rt->rt_genmask;
    if ((rt->rt_flags & (RTF_UP | RTF_WASCLONED)) == RTF_UP) {
        _inet_ntop(dst, addr, sizeof(addr));
        printf("%-15s ", addr);
        if (gate != NULL) {
            _inet_ntop(gate, addr, sizeof(addr));
            printf("%-15s ", addr);
        } else {
            printf("%-15s ", " ");
        }
        if (netmask != NULL) {
	    if (dst->sa_family == AF_INET6) {
	      _mask6(netmask, addr, sizeof(addr));
	      printf("/%-14s ", addr);
	    }
        } else {
            printf("%-15s ", " ");
        }
        cp = addr;
        if ((rt->rt_flags & RTF_UP)) *cp++ = 'U';
        if ((rt->rt_flags & RTF_GATEWAY)) *cp++ = 'G';
	if ((rt->rt_flags & RTF_HOST)) *cp++ = 'H';
	if ((rt->rt_flags & RTF_REJECT)) *cp++ = '!';
        if ((rt->rt_flags & RTF_STATIC)) *cp++ = 'S';
        if ((rt->rt_flags & RTF_DYNAMIC)) *cp++ = 'D';
	if ((rt->rt_flags & RTF_MODIFIED)) *cp++ = 'M';
        *cp = '\0';
        printf("%-8s ", addr);  // Flags
        if_indextoname(rt->rt_ifp->if_index, addr, 64);
        printf("%-8s ", addr);
        printf("\n");
    }
    return 0;
}

void route_v6_dump(void)
{
    struct radix_node_head *rnh;

    printf("IPv6 routing tables\n");
    printf("Destination     Gateway         Mask            Flags    Interface\n");
    if ((rnh = rt_tables[AF_INET6]) != NULL) 
	{
        rnh->rnh_walktree(rnh, _dumpv6entry, NULL);
    }
}
void ifaddrv6(char * ifname)
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	{
	    if (strcmp(ifp->if_xname, ifname))
		continue;
	    for (ifa = ifp->if_addrhead.tqh_first; ifa;ifa = ifa->ifa_link.tqe_next) 
		{
		 if (ifa->ifa_addr->sa_family == AF_INET6)
		    printf("\tinet6 addr: %s/%d\n", ip6_sprintf(IFA_IN6(ifa)), in6_mask2len(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr, NULL));
		}
	}
}

void get_wan_addrv6(char *ifname,char *ip_addr)
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	{
		if (strcmp(ifp->if_xname, ifname))
		continue;
		for (ifa = ifp->if_addrhead.tqh_first; ifa;ifa = ifa->ifa_link.tqe_next) 
		{
			 if (ifa->ifa_addr->sa_family == AF_INET6)
			 {
			 	 if(strncmp(ip6_sprintf(IFA_IN6(ifa)),"fe80",4)==0)
				 	continue;
				 printf("\tinet6 addr: %s/%d\n", ip6_sprintf(IFA_IN6(ifa)), in6_mask2len(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr, NULL));
				 strcpy(ip_addr,ip6_sprintf(IFA_IN6(ifa)));
				 break;
			 }
		}
	}
}




void get_ppp_addrv6(char *ifname,char *ip_addr)
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
	{
		if (strcmp(ifp->if_xname, ifname))
		continue;
		for (ifa = ifp->if_addrhead.tqh_first; ifa;ifa = ifa->ifa_link.tqe_next) 
		{
			 if (ifa->ifa_addr->sa_family == AF_INET6)
			 {
				 printf("\tinet6 addr: %s/%d\n", ip6_sprintf(IFA_IN6(ifa)), in6_mask2len(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr, NULL));
				 strcpy(ip_addr,ip6_sprintf(IFA_IN6(ifa)));
				 break;
			 }
		}
	}
}


int getifaddrv6(const char *ifname, int addrIdx,
      char *ipAddr, int *ifIdx, int *prefixLen, int *scope, int *ifaFlags)
{
   register struct ifnet *ifp;
   register struct ifaddr *ifa;
   int count = 0;

   // each interface
   for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
   {
      count = 0;
      if (strcmp(ifp->if_xname, ifname))
         continue;

      // each address
      for (ifa = ifp->if_addrhead.tqh_first; ifa ; ifa = ifa->ifa_link.tqe_next)
      {
         if (count++ == addrIdx)
         {
            if (ifa->ifa_addr->sa_family != AF_INET6)
               return 0;
            sprintf(ipAddr, "%s", ip6_sprintf(IFA_IN6(ifa)));
            *prefixLen = in6_mask2len(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr, NULL);
			*ifIdx = ifp->if_index;
            *scope = in6_addrscope(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr);
            return 1;
         }
      }
   }
   // cannot find an address with addrIdx for ifname
   return 0;
}

int getlanaddrv6(const char *ifname, char *ipAddr)
{
   register struct ifnet *ifp;
   register struct ifaddr *ifa;
   int found=0;

   // each interface
   for (ifp = ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next)
   {
      if (strcmp(ifp->if_xname, ifname))
         continue;

      // each address
      for (ifa = ifp->if_addrhead.tqh_first; ifa ; ifa = ifa->ifa_link.tqe_next)
      {
         if (ifa->ifa_addr->sa_family != AF_INET6)
            continue;
         
         if (in6_addrscope(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr) == IPV6_ADDR_SCOPE_GLOBAL)
         {
            sprintf(ipAddr, "%s/%d", ip6_sprintf(IFA_IN6(ifa)),in6_mask2len(&((struct in6_ifaddr *)ifa)->ia_prefixmask.sin6_addr, NULL));
            found = 1;
         }
      }
   }
   return found;
}
#endif
SYSCTL_NODE(_net, PF_LINK, link, CTLFLAG_RW, 0, "Link layers");
SYSCTL_NODE(_net_link, 0, generic, CTLFLAG_RW, 0, "Generic link-management");
