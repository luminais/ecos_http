//==========================================================================
//
//      sys/net/if_bridge.c
//
//     
//
//==========================================================================
// ####BSDALTCOPYRIGHTBEGIN####                                             
// -------------------------------------------                              
// Portions of this software may have been derived from OpenBSD             
// or other sources, and if so are covered by the appropriate copyright     
// and license included herein.                                             
// -------------------------------------------                              
// ####BSDALTCOPYRIGHTEND####                                               
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Jason L. Wright (jason@thought.net)  
// Contributors: andrew.lunn@ascom.ch (Andrew Lunn), hmt, manu.sharma@ascom.com
// Date:         2000-07-18
// Purpose:      Ethernet bridge
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/*	$OpenBSD: if_bridge.c,v 1.33 2000/06/20 05:50:16 jason Exp $	*/

/*
 * Copyright (c) 1999, 2000 Jason L. Wright (jason@thought.net)
 * All rights reserved.
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
 *	This product includes software developed by Jason L. Wright
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __ECOS
#define __ECOS
#endif
#ifdef __ECOS
#include <pkgconf/system.h>
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WRAPPER
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#endif
#include <pkgconf/net.h>
#else
#include "bridge.h"
#include "bpfilter.h"
#include "enc.h"
#endif

#include <sys/param.h>
#ifndef __ECOS
#include <sys/proc.h>
#include <sys/systm.h>
#endif
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <time.h>
#ifndef __ECOS
#include <sys/device.h>
#endif
#include <sys/kernel.h>
//#include <machine/cpu.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_llc.h>
#include <net/route.h>
#include <net/netisr.h>
#include "../../../../../../devs/eth/rltk/819x/switch/v3_0/src/rtl_types.h"
#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
//#include <netinet/ip_ipsp.h>
#include <netinet/ip6.h>
#ifndef __ECOS
#include <net/if_enc.h>
#endif
#ifdef IPFILTER
#include <netinet/ip_fil_compat.h>
#include <netinet/ip_fil.h>
#endif
#endif

#if 0//NBPFILTER > 0
#include <net/bpf.h>
#endif

#include <net/if_bridge.h>

#ifdef __ECOS
#include <stdio.h> /* for sprintf */
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>

#endif 
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <netinet/igmp.h>
#include <rtl/rtl865x_igmpsnooping.h>

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)

#include <rtl/rtl865x_multicast.h>
uint32 br0SwFwdPortMask;

#endif
#endif

#include <switch/rtl865x_fdb_api.h>

#ifdef __CONFIG_APCLIENT_DHCPC__
#include <netinet/udp.h>
#include <arpa/nameser.h>

typedef HEADER DNSHEADER;
extern int gpi_apclient_dhcpc_enable();
extern int dnsmasq_parse_request(DNSHEADER *dnsheader, unsigned int qlen, char *name);
//extern void dns_redirect_pkt(struct mbuf *m);
#endif

#ifndef	BRIDGE_RTABLE_SIZE
#define	BRIDGE_RTABLE_SIZE	256
#endif
#define	BRIDGE_RTABLE_MASK	(BRIDGE_RTABLE_SIZE - 1)

/*
 * Maximum number of addresses to cache
 */
#ifndef BRIDGE_RTABLE_MAX
#define BRIDGE_RTABLE_MAX	100
#endif

/*
 * Timeout (in seconds) for entries learned dynamically
 */
#ifndef BRIDGE_RTABLE_TIMEOUT
#define BRIDGE_RTABLE_TIMEOUT	300
#endif

/* Spanning tree defaults */
#define BSTP_DEFAULT_MAX_AGE            (20 * 256)
#define BSTP_DEFAULT_HELLO_TIME         (2 * 256)
#define BSTP_DEFAULT_FORWARD_DELAY      (15 * 256)
#define BSTP_DEFAULT_HOLD_TIME          (1 * 256)
#define BSTP_DEFAULT_BRIDGE_PRIORITY    0x8000
#define BSTP_DEFAULT_PORT_PRIORITY      0x80
#define BSTP_DEFAULT_PATH_COST          55

extern int ifqmaxlen;
#if defined (CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
extern int rtl_vlan_support_enable;
#endif

#if defined(CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT)
extern int get_multirepeaterFlag(int wlanidx);
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#if defined(CONFIG_RTL_MLD_SNOOPING)	
extern int re865x_getIpv6TransportProtocol(struct ip6_hdr* ipv6h);
extern int rtl_isHopbyHop(struct ip6_hdr* ipv6h);
#endif

void bridge_multicast_forward(struct bridge_softc *sc,struct ifnet *ifp,struct ether_header *eh, u_int32_t fwdPortMask, struct mbuf * m, int clone);
#if defined (CONFIG_RTL_BRIDGE_QUERY_SUPPORT)
#define MCAST_QUERY_INTERVAL 30
struct callout mCastQuerytimer;

void bridge_mCastQueryTimerExpired(void * para);
#endif
#if defined(CONFIG_RTL_819X_SWCORE)
extern unsigned int brIgmpModuleIndex;
extern unsigned int nicIgmpModuleIndex;
extern int igmpsnoopenabled;
#if defined (CONFIG_RTL_MLD_SNOOPING)
extern int mldSnoopEnabled;
#endif
#endif
//#define DBG_IGMP_SNOOPING 1
//#define DBG 1
#define MCAST_TO_UNICAST 1
#ifdef MCAST_TO_UNICAST
#define IPV6_MCAST_TO_UNICAST
#define SIOCGIMCAST_ADD			0x8B80
#define SIOCGIMCAST_DEL			0x8B81

#endif
//#define DBG_ICMPv6

#define IPV4_MULTICAST_MAC(mac) ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
#define IPV6_MULTICAST_MAC(mac) ((mac[0]==0x33)&&(mac[1]==0x33) && mac[2]!=0xff)
#if defined DBG
#define DEBUG_PRINT diag_printf
#else
#define DEBUG_PRINT
#endif
#endif
#define IP_VERSION4 4
#define IP_VERSION6 6

#define RTL_PS_BR0_DEV_NAME "bridge0"
#define RTL_PS_LAN_P0_DEV_NAME "eth0"
#define RTL_PS_WLAN_NAME	"wlan"
#define RTL_PS_ETH_NAME	RTL_PS_LAN_P0_DEV_NAME
#define SUCCESS 0
#define FAILED -1

/* SNAP LLC header */
struct snap {
	u_int8_t dsap;
	u_int8_t ssap;
	u_int8_t control;
	u_int8_t org[3];
	u_int16_t type;
};
#ifndef CYGNUM_NET_BRIDGES
#define CYGNUM_NET_BRIDGES 1
#endif
//lq 添加访客网络再加一个桥。
//在配置里面修改宏没有生效，所以在此处直接修改
#define CYGNUM_NET_BRIDGES 2
struct bridge_softc bridgectl[CYGNUM_NET_BRIDGES];
struct callout br_timer;

void	bridgeattach __P((int));
int	bridge_ioctl __P((struct ifnet *, u_long, caddr_t));
void	bridge_start __P((struct ifnet *));
void	bridgeintr(void);

void	bridgeintr_frame __P((struct bridge_softc *, struct mbuf *));
void	bridge_broadcast __P((struct bridge_softc *, struct ifnet *,
    struct ether_header *, struct mbuf *)) __attribute ((weak));
void	bridge_stop __P((struct bridge_softc *));
void	bridge_init __P((struct bridge_softc *));
int	bridge_bifconf __P((struct bridge_softc *, struct ifbifconf *));

int	bridge_rtfind __P((struct bridge_softc *, struct ifbaconf *));
void	bridge_rtage __P((void *));
void	bridge_rttrim __P((struct bridge_softc *));
int	bridge_rtdaddr __P((struct bridge_softc *, struct ether_addr *));
int	bridge_rtflush __P((struct bridge_softc *, int));
struct ifnet *	bridge_rtupdate __P((struct bridge_softc *,
    struct ether_addr *, struct ifnet *ifp, int, u_int8_t));
struct ifnet *	bridge_rtlookup __P((struct bridge_softc *,
    struct ether_addr *));
struct bridge_rtnode *bridge_rtget __P((struct bridge_softc *,
    struct ether_addr *));

//struct bridge_rtnode *bridge_rtinsert __P((struct bridge_softc *,
//   struct ether_addr *, struct ifnet *ifp, int, u_int8_t));
	

u_int32_t	bridge_hash __P((struct ether_addr *));
int bridge_blocknonip __P((struct ether_header *, struct mbuf *));
int		bridge_addrule __P((struct bridge_iflist *,
    struct ifbrlreq *, int out));
int		bridge_flushrule __P((struct bridge_iflist *));
int	bridge_brlconf __P((struct bridge_softc *, struct ifbrlconf *));
u_int8_t bridge_filterrule __P((struct brl_node *, struct ether_header *));
int     bridge_ifenqueue __P((struct bridge_softc *, struct ifnet *, struct mbuf *));

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
void bridge_span (struct bridge_softc *, struct ether_header *, struct mbuf *);
#endif
#define IS_MCAST_ETHERADDR(dest) ((dest)->octet[0]&&0x01 == 1)

#define	ETHERADDR_IS_IP_MCAST(a) \
	/* struct etheraddr *a;	*/				\
	((a)->octet[0] == 0x01 &&			\
	 (a)->octet[1] == 0x00 &&			\
	 (a)->octet[2] == 0x5e)
	 
#define ETHERADDR_IS_IPv6_MCAST(a) \
	/* struct etheraddr *a;	*/				\
	((a)->octet[0] == 0x33 &&			\
	 (a)->octet[1] == 0x33 &&			\
	 (a)->octet[2] != 0xff)

#if defined(INET) && (defined(IPFILTER) || defined(IPFILTER_LKM))
/*
 * Filter hooks
 */
struct mbuf *bridge_filter __P((struct bridge_softc *, struct ifnet *,
    struct ether_header *, struct mbuf *m));
#endif

void
bridgeattach(unused)
	int unused;
{
	int i;
	struct ifnet *ifp;
	for (i = 0; i < CYGNUM_NET_BRIDGES; i++) {
		bridgectl[i].sc_brtmax = BRIDGE_RTABLE_MAX;
		bridgectl[i].sc_brttimeout = (BRIDGE_RTABLE_TIMEOUT * hz)/2;
        bridgectl[i].sc_bridge_max_age = BSTP_DEFAULT_MAX_AGE;
        bridgectl[i].sc_bridge_hello_time = BSTP_DEFAULT_HELLO_TIME;
        bridgectl[i].sc_bridge_forward_delay= BSTP_DEFAULT_FORWARD_DELAY;
        bridgectl[i].sc_bridge_priority = BSTP_DEFAULT_BRIDGE_PRIORITY;
        bridgectl[i].sc_hold_time = BSTP_DEFAULT_HOLD_TIME;
		LIST_INIT(&bridgectl[i].sc_iflist);
		LIST_INIT(&bridgectl[i].sc_spanlist);
		ifp = &bridgectl[i].sc_if;
		sprintf(ifp->if_xname, "bridge%d", i);
		ifp->if_name="bridge";		
		ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST ;
		ifp->if_softc = &bridgectl[i];
		ifp->if_unit = i;
		ifp->if_mtu = ETHERMTU;
		ifp->if_ioctl = bridge_ioctl;
		ifp->if_output = bridge_output;
		ifp->if_start = bridge_start;
		ifp->if_type = IFT_PROPVIRTUAL;
		IFQ_SET_MAXLEN(&ifp->if_snd, ifqmaxlen);
		IFQ_SET_READY(&ifp->if_snd);
		ifp->if_hdrlen = sizeof(struct ether_header);		
		ether_ifattach(ifp, 0);
		bridge_init(ifp->if_softc);
#if 0//NBPFILTER > 0
		bpfattach(&bridgectl[i].sc_if.if_bpf, ifp,
		    DLT_EN10MB, sizeof(struct ether_header));
#endif
	}
}

int
bridge_ioctl(ifp, cmd, data)
	struct ifnet *ifp;
	u_long cmd;
	caddr_t	data;
{
#ifndef __ECOS
	struct proc *prc = curproc;		/* XXX */
#endif
	struct ifnet *ifs;
	struct bridge_softc *sc = (struct bridge_softc *)ifp->if_softc;
	struct ifbreq *req = (struct ifbreq *)data;
	struct ifbaconf *baconf = (struct ifbaconf *)data;
	struct ifbareq *bareq = (struct ifbareq *)data;
	struct ifbcachereq *bcachereq = (struct ifbcachereq *)data;
	struct ifbifconf *bifconf = (struct ifbifconf *)data;
	struct ifbcachetoreq *bcacheto = (struct ifbcachetoreq *)data;
	struct ifbrlreq *brlreq = (struct ifbrlreq *)data;
	struct ifbrlconf *brlconf = (struct ifbrlconf *)data;
	struct ifreq ifreq;
	int error = 0, s;
	struct bridge_iflist *p;
	char ifname[IFNAMSIZ];
	s = splimp();
	switch (cmd) {
	case SIOCBRDGADD:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		//printf("ifp name:%s unit:%d ifp->if_softc:%x\n",
				//ifp->if_name,ifp->if_unit,ifp->if_softc);
		//printf("SIOCBRDGADD if_name:%s\n",req->ifbr_ifsname);
		ifs = ifunit(req->ifbr_ifsname);
		//printf("if_name:%s ifx_name:%s unit:%d\n",ifs->if_name,ifs->if_xname,ifs->if_unit);
		if (ifs == NULL) {			/* no such interface */
			error = ENOENT;
			break;
		}
		if (ifs->if_bridge == (caddr_t)sc) {
			error = EEXIST;
			break;
		}
		if (ifs->if_bridge != NULL) {
			error = EBUSY;
			break;
		}

        #ifdef CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT
        if( !strcmp(req->ifbr_ifsname,"wlan0-va0") || 
            !strcmp(req->ifbr_ifsname,"wlan0-va2") ||
            !strcmp(req->ifbr_ifsname,"wlan1-va0") ||
            !strcmp(req->ifbr_ifsname,"wlan1-va2"))
        {
              if(1==get_multirepeaterFlag(atoi(&req->ifbr_ifsname[4])))
                ifs->zone_type=1;
              else
                ifs->zone_type=0;
        }
        #endif

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
                /* If it's in the span list, it can't be a member. */
                LIST_FOREACH(p, &sc->sc_spanlist, next) {
                        if (p->ifp == ifs)
                                break;
                }
                if (p != LIST_END(&sc->sc_spanlist)) {
                        error = EBUSY;
                        break;
                }
#endif


		if (ifs->if_type == IFT_ETHER) {
			if ((ifs->if_flags & IFF_UP) == 0) {
				/*
				 * Bring interface up long enough to set
				 * promiscuous flag, then shut it down again.
				 */
				strncpy(ifreq.ifr_name, req->ifbr_ifsname,
				    sizeof(ifreq.ifr_name) - 1);
				ifreq.ifr_name[sizeof(ifreq.ifr_name) - 1] = '\0';
				ifs->if_flags |= IFF_UP;
				ifreq.ifr_flags = ifs->if_flags;
				error = (*ifs->if_ioctl)(ifs, SIOCSIFFLAGS,
				    (caddr_t)&ifreq);
				if (error != 0)
					break;

				error = ifpromisc(ifs, 1);
				if (error != 0)
					break;

				strncpy(ifreq.ifr_name, req->ifbr_ifsname,
				    sizeof(ifreq.ifr_name) - 1);
				ifreq.ifr_name[sizeof(ifreq.ifr_name) - 1] = '\0';
				ifs->if_flags &= ~IFF_UP;
				ifreq.ifr_flags = ifs->if_flags;
				error = (*ifs->if_ioctl)(ifs, SIOCSIFFLAGS,
				    (caddr_t)&ifreq);
				if (error != 0) {
					ifpromisc(ifs, 0);
					break;
				}
			} else {
				error = ifpromisc(ifs, 1);
				if (error != 0)
					break;
			}
		}
#ifndef __ECOS
#if NENC > 0
		else if (ifs->if_type == IFT_ENC) {
			/* Can't bind enc0 to a bridge */
			if (ifs->if_softc == &encif[0]) {
				error = EINVAL;
				break;
			}
		}
#endif /* NENC */
#endif
		else {
			error = EINVAL;
			break;
		}

		p = (struct bridge_iflist *) malloc(
		    sizeof(struct bridge_iflist), M_DEVBUF, M_NOWAIT);
		if (p == NULL && ifs->if_type == IFT_ETHER) {
			error = ENOMEM;
			ifpromisc(ifs, 0);
			break;
		}

		p->ifp = ifs;
		p->bif_flags = IFBIF_LEARNING | IFBIF_DISCOVER;
                p->bif_priority = BSTP_DEFAULT_PORT_PRIORITY;
                p->bif_path_cost = BSTP_DEFAULT_PATH_COST;
		SIMPLEQ_INIT(&p->bif_brlin);
		SIMPLEQ_INIT(&p->bif_brlout);
		LIST_INSERT_HEAD(&sc->sc_iflist, p, next);
		ifs->if_bridge = (caddr_t)sc;
		
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(strcmp(sc->sc_if.if_xname,RTL_PS_BR0_DEV_NAME)==0)
		{
			rtl_multicastDeviceInfo_t brDevInfo;
			rtl865x_generateBridgeDeviceInfo(sc, &brDevInfo);
			#if defined(CONFIG_RTL_819X_SWCORE)
			if(brIgmpModuleIndex!=0xFFFFFFFF)
			{
				rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex,&brDevInfo);
			}
			#endif
			br0SwFwdPortMask=brDevInfo.swPortMask;
			//diag_printf("br0SwFwdPortMask:%x,[%s]:[%d].\n",br0SwFwdPortMask,__FUNCTION__,__LINE__);
		}
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
		if(strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)==0)
		{
			rtl_multicastDeviceInfo_t brDevInfo;
			rtl865x_generateBridgeDeviceInfo(br, &brDevInfo);
			#if defined(CONFIG_RTL_819X_SWCORE)
			if(brIgmpModuleIndex!=0xFFFFFFFF)
			{
				rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex_2,&brDevInfo);
			}
			#endif
			br1SwFwdPortMask=brDevInfo.swPortMask;
		}
#endif
#endif
		break;
	case SIOCBRDGDEL:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		p = LIST_FIRST(&sc->sc_iflist);
		while (p != NULL) {
			sprintf(ifname,"%s%d",p->ifp->if_name,p->ifp->if_unit);
			if (strncmp(ifname, req->ifbr_ifsname,
			    sizeof(req->ifbr_ifsname)) == 0) {
				p->ifp->if_bridge = NULL;

				error = ifpromisc(p->ifp, 0);

				LIST_REMOVE(p, next);
				bridge_rtdelete(sc, p->ifp, 0);
				bridge_flushrule(p);
				free(p, M_DEVBUF);
				break;
			}
			p = LIST_NEXT(p, next);
		}
		if (p == NULL) {
			error = ENOENT;
			break;
		}
		break;
	case SIOCBRDGIFS:
		error = bridge_bifconf(sc, bifconf);
		break;
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        case SIOCBRDGADDS:
#ifndef __ECOS
                if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
                        break;
#endif

                ifs = ifunit(req->ifbr_ifsname);
                if (ifs == NULL) {                      /* no such interface */
                        error = ENOENT;
                        break;
                }
                if (ifs->if_bridge == (caddr_t)sc) {
                        error = EEXIST;
                        break;
                }
                if (ifs->if_bridge != NULL) {
                        error = EBUSY;
                        break;
                }
                LIST_FOREACH(p, &sc->sc_spanlist, next) {
                        if (p->ifp == ifs)
                        break;
                }
                if (p != LIST_END(&sc->sc_spanlist)) {
                        error = EBUSY;
                        break;
                }
                p = (struct bridge_iflist *)malloc(
                    sizeof(struct bridge_iflist), M_DEVBUF, M_NOWAIT);
                if (p == NULL) {
                       error = ENOMEM;
                       break;
                }
                bzero(p, sizeof(struct bridge_iflist));
                p->ifp = ifs;
                SIMPLEQ_INIT(&p->bif_brlin);
                SIMPLEQ_INIT(&p->bif_brlout);
                LIST_INSERT_HEAD(&sc->sc_spanlist, p, next);
                break;
                                                                                                                                                                                                           case SIOCBRDGDELS:
#ifndef __ECOS
                if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
                       break;
#endif

                LIST_FOREACH(p, &sc->sc_spanlist, next) {
                       sprintf(ifname,"%s%d",p->ifp->if_name,p->ifp->if_unit);
                       if (strncmp(ifname, req->ifbr_ifsname,
                          sizeof(req->ifbr_ifsname)) == 0) {
                              LIST_REMOVE(p, next);
                              free(p, M_DEVBUF);
                              break;
                       }
                }
                if (p == LIST_END(&sc->sc_spanlist)) {
                       error = ENOENT;
                       break;
                }
                break;
#endif

	case SIOCBRDGGIFFLGS:
		ifs = ifunit(req->ifbr_ifsname);
		if (ifs == NULL) {
			error = ENOENT;
			break;
		}
		if ((caddr_t)sc != ifs->if_bridge) {
			error = ESRCH;
			break;
		}
		p = LIST_FIRST(&sc->sc_iflist);
		while (p != NULL && p->ifp != ifs) {
			p = LIST_NEXT(p, next);
		}
		if (p == NULL) {
			error = ESRCH;
			break;
		}
		req->ifbr_ifsflags = p->bif_flags;
                req->ifbr_state = p->bif_state;
                req->ifbr_priority = p->bif_priority;
                req->ifbr_path_cost = p->bif_path_cost;
                req->ifbr_portno = p->ifp->if_index & 0xff;
		break;
	case SIOCBRDGSIFFLGS:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		ifs = ifunit(req->ifbr_ifsname);
		if (ifs == NULL) {
			error = ENOENT;
			break;
		}
		if ((caddr_t)sc != ifs->if_bridge) {
			error = ESRCH;
			break;
		}
		p = LIST_FIRST(&sc->sc_iflist);
		while (p != NULL && p->ifp != ifs) {
			p = LIST_NEXT(p, next);
		}
		if (p == NULL) {
			error = ESRCH;
			break;
		}
		p->bif_flags = req->ifbr_ifsflags;
		break;
        case SIOCBRDGSIFPRIO:
        case SIOCBRDGSIFCOST:
#ifndef __ECOS
                if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
                        break;
#endif

                ifs = ifunit(req->ifbr_ifsname);
                if (ifs == NULL) {
                        error = ENOENT;
                        break;
                }
                if ((caddr_t)sc != ifs->if_bridge) {
                        error = ESRCH;
                        break;
                }
                LIST_FOREACH(p, &sc->sc_iflist, next) {
                        if (p->ifp == ifs)
                        break;
                }
                if (p == LIST_END(&sc->sc_iflist)) {
                        error = ESRCH;
                        break;
                }
                if (cmd == SIOCBRDGSIFPRIO)
                        p->bif_priority = req->ifbr_priority;
                else {
                        if (req->ifbr_path_cost < 1)
                                error = EINVAL;
                        else
                                p->bif_path_cost = req->ifbr_path_cost;
                }
                break;
	case SIOCBRDGRTS:
		error = bridge_rtfind(sc, baconf);
		break;
	case SIOCBRDGFLUSH:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		error = bridge_rtflush(sc, req->ifbr_ifsflags);
		break;
	case SIOCBRDGSADDR:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		ifs = ifunit(bareq->ifba_ifsname);
		if (ifs == NULL) {			/* no such interface */
			error = ENOENT;
			break;
		}

		if (ifs->if_bridge == NULL ||
		    ifs->if_bridge != (caddr_t)sc) {
			error = ESRCH;
			break;
		}

		ifs = bridge_rtupdate(sc, &bareq->ifba_dst, ifs, 1,
		    bareq->ifba_flags);
		if (ifs == NULL)
			error = ENOMEM;
		break;
	case SIOCBRDGDADDR:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		error = bridge_rtdaddr(sc, &bareq->ifba_dst);
		break;
	case SIOCBRDGGCACHE:
		bcachereq->ifbc_size = sc->sc_brtmax;
		break;
	case SIOCBRDGSCACHE:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		sc->sc_brtmax = bcachereq->ifbc_size;
		bridge_rttrim(sc);
		break;
	case SIOCBRDGSTO:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		sc->sc_brttimeout = (bcacheto->ifbct_time * hz) / 2;
		untimeout(bridge_rtage, sc);
		if (bcacheto->ifbct_time != 0)
			timeout(bridge_rtage, sc, sc->sc_brttimeout);
		break;
	case SIOCBRDGGTO:
		bcacheto->ifbct_time = (2 * sc->sc_brttimeout) / hz;
		break;
	case SIOCSIFFLAGS:
		if ((ifp->if_flags & IFF_UP) == IFF_UP)
			bridge_init(sc);

		if ((ifp->if_flags & IFF_UP) == 0)
			bridge_stop(sc);

		break;
	case SIOCBRDGARL:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		ifs = ifunit(brlreq->ifbr_ifsname);
		if (ifs == NULL) {
			error = ENOENT;
			break;
		}
		if (ifs->if_bridge == NULL ||
		    ifs->if_bridge != (caddr_t)sc) {
			error = ESRCH;
			break;
		}
		p = LIST_FIRST(&sc->sc_iflist);
		while (p != NULL && p->ifp != ifs) {
			p = LIST_NEXT(p, next);
		}
		if (p == NULL) {
			error = ESRCH;
			break;
		}
		if ((brlreq->ifbr_action != BRL_ACTION_BLOCK &&
		    brlreq->ifbr_action != BRL_ACTION_PASS) ||
		    (brlreq->ifbr_flags & (BRL_FLAG_IN|BRL_FLAG_OUT)) == 0) {
			error = EINVAL;
			break;
		}
		if (brlreq->ifbr_flags & BRL_FLAG_IN) {
			error = bridge_addrule(p, brlreq, 0);
			if (error)
				break;
		}
		if (brlreq->ifbr_flags & BRL_FLAG_OUT) {
			error = bridge_addrule(p, brlreq, 1);
			if (error)
				break;
		}
		break;
	case SIOCBRDGFRL:
#ifndef __ECOS
		if ((error = suser(prc->p_ucred, &prc->p_acflag)) != 0)
			break;
#endif
		ifs = ifunit(brlreq->ifbr_ifsname);
		if (ifs == NULL) {
			error = ENOENT;
			break;
		}
		if (ifs->if_bridge == NULL ||
		    ifs->if_bridge != (caddr_t)sc) {
			error = ESRCH;
			break;
		}
		p = LIST_FIRST(&sc->sc_iflist);
		while (p != NULL && p->ifp != ifs) {
			p = LIST_NEXT(p, next);
		}
		if (p == NULL) {
			error = ESRCH;
			break;
		}
		error = bridge_flushrule(p);
		break;
	case SIOCBRDGGRL:
		error = bridge_brlconf(sc, brlconf);
		break;
        case SIOCBRDGGPRI:
        case SIOCBRDGGMA:
        case SIOCBRDGGHT:
        case SIOCBRDGGFD:
                break;
        case SIOCBRDGSPRI:
        case SIOCBRDGSFD:
        case SIOCBRDGSMA:
        case SIOCBRDGSHT:
#ifndef __ECOS
                error = suser(prc->p_ucred, &prc->p_acflag);
#endif
                break;
	default:
		error = EINVAL;
	}

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        if (!error)
                error = bstp_ioctl(ifp, cmd, data);
#endif
	splx(s);
	return (error);
}

/* Detach an interface from a bridge.  */
void
bridge_ifdetach(ifp)
	struct ifnet *ifp;
{
	struct bridge_softc *bsc = (struct bridge_softc *)ifp->if_bridge;
	struct bridge_iflist *bif;

	for (bif = LIST_FIRST(&bsc->sc_iflist); bif;
	    bif = LIST_NEXT(bif, next))
		if (bif->ifp == ifp) {
			LIST_REMOVE(bif, next);
			bridge_rtdelete(bsc, ifp, 0);
			bridge_flushrule(bif);
			free(bif, M_DEVBUF);
			ifp->if_bridge = NULL;
			break;
		}
}

int
bridge_bifconf(
	struct bridge_softc *sc,
	struct ifbifconf *bifc
)
{
	struct bridge_iflist *p;
	u_int32_t total = 0, i;
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        u_int32_t j;
#endif
	int error = 0;
	struct ifbreq breq;
	char ifname[IFNAMSIZ];
	p = LIST_FIRST(&sc->sc_iflist);
	while (p != NULL) {
		total++;
		p = LIST_NEXT(p, next);
	}

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        p = LIST_FIRST(&sc->sc_spanlist);
        while (p != NULL) {
                total++;
                p = LIST_NEXT(p, next);
        }
#endif

	if (bifc->ifbic_len == 0) {
		i = total;
		goto done;
	}

	p = LIST_FIRST(&sc->sc_iflist);
	i = 0;
	while (p != NULL && bifc->ifbic_len > sizeof(breq)) {		
		//strncpy(breq.ifbr_name, sc->sc_if.if_xname,sizeof(breq.ifbr_name)-1);
		sprintf(ifname,"%s%d",sc->sc_if.if_name,sc->sc_if.if_unit);
		strncpy(breq.ifbr_name, ifname,sizeof(breq.ifbr_name)-1);
		breq.ifbr_name[sizeof(breq.ifbr_name) - 1] = '\0';
		if(p->ifp!=NULL){
			sprintf(ifname,"%s%d",p->ifp->if_name,p->ifp->if_unit);
			strncpy(breq.ifbr_ifsname, ifname,
		    sizeof(breq.ifbr_ifsname)-1);
		}
		else{
			error=-1;
			goto done;
		}
		breq.ifbr_ifsname[sizeof(breq.ifbr_ifsname) - 1] = '\0';
		//printf("breq.ifbr_ifsname:%s\n",breq.ifbr_ifsname);
		breq.ifbr_ifsflags = p->bif_flags;		
        breq.ifbr_state = p->bif_state;		
        breq.ifbr_priority = p->bif_priority;		
        breq.ifbr_path_cost = p->bif_path_cost;		
        breq.ifbr_portno = p->ifp->if_index & 0xff;
		error = copyout((caddr_t)&breq,
		    (caddr_t)(bifc->ifbic_req + i), sizeof(breq));
		if (error)
			goto done;
		p = LIST_NEXT(p, next);
		i++;
		bifc->ifbic_len -= sizeof(breq);
	}

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        p = LIST_FIRST(&sc->sc_spanlist);
        j = 0;
        while (p != NULL && bifc->ifbic_len > j * sizeof(breq)) {
                //strncpy(breq.ifbr_name, sc->sc_if.if_xname,sizeof(breq.ifbr_name)-1);
                sprintf(ifname,"%s%d",sc->sc_if.if_name,sc->sc_if.if_unit);
				strncpy(breq.ifbr_name, ifname,sizeof(breq.ifbr_name)-1);
                breq.ifbr_name[sizeof(breq.ifbr_name) - 1] = '\0';
                //strncpy(breq.ifbr_ifsname, p->ifp->if_xname,sizeof(breq.ifbr_ifsname)-1);
                sprintf(ifname,"%s%d",p->ifp->if_name,p->ifp->if_unit);
				strncpy(breq.ifbr_ifsname, ifname,sizeof(breq.ifbr_ifsname)-1);
                breq.ifbr_ifsname[sizeof(breq.ifbr_ifsname) - 1] = '\0';
                breq.ifbr_ifsflags = p->bif_flags | IFBIF_SPAN;
                breq.ifbr_state = p->bif_state;
                breq.ifbr_priority = p->bif_priority;
                breq.ifbr_path_cost = p->bif_path_cost;
                breq.ifbr_portno = p->ifp->if_index & 0xff;
                error = copyout((caddr_t)&breq,
                    (caddr_t)(bifc->ifbic_req + j), sizeof(breq));
                if (error)
                        goto done;
                p = LIST_NEXT(p, next);
                j++;
                bifc->ifbic_len -= sizeof(breq);
        }
#endif
done:
	bifc->ifbic_len = i * sizeof(breq);
	return (error);
}

int
bridge_brlconf(sc, bc)
	struct bridge_softc *sc;
	struct ifbrlconf *bc;
{
	struct ifnet *ifp;
	struct bridge_iflist *ifl;
	struct brl_node *n;
	struct ifbrlreq req;
	int error = 0;
	u_int32_t i, total=0;
	char ifname[IFNAMSIZ];

	ifp = ifunit(bc->ifbrl_ifsname);
	if (ifp == NULL)
		return (ENOENT);
	if (ifp->if_bridge == NULL || ifp->if_bridge != (caddr_t)sc)
		return (ESRCH);
	ifl = LIST_FIRST(&sc->sc_iflist);
	while (ifl != NULL && ifl->ifp != ifp)
		ifl = LIST_NEXT(ifl, next);
	if (ifl == NULL)
		return (ESRCH);

	n = SIMPLEQ_FIRST(&ifl->bif_brlin);
	while (n != NULL) {
		total++;
		n = SIMPLEQ_NEXT(n, brl_next);
	}
	n = SIMPLEQ_FIRST(&ifl->bif_brlout);
	while (n != NULL) {
		total++;
		n = SIMPLEQ_NEXT(n, brl_next);
	}

	if (bc->ifbrl_len == 0) {
		i = total;
		goto done;
	}

	i = 0;
	n = SIMPLEQ_FIRST(&ifl->bif_brlin);
	while (n != NULL && bc->ifbrl_len > i * sizeof(req)) {
		//strncpy(req.ifbr_name, sc->sc_if.if_xname,sizeof(req.ifbr_name) - 1);
		sprintf(ifname,"%s%d",sc->sc_if.if_name,sc->sc_if.if_unit);
		strncpy(req.ifbr_name, ifname,sizeof(req.ifbr_name) - 1);
		req.ifbr_name[sizeof(req.ifbr_name) - 1] = '\0';
		//strncpy(req.ifbr_ifsname, ifl->ifp->if_xname,sizeof(req.ifbr_ifsname) - 1);
		sprintf(ifname,"%s%d",ifl->ifp->if_name,ifl->ifp->if_unit);
		strncpy(req.ifbr_ifsname, ifname,sizeof(req.ifbr_ifsname) - 1);
		req.ifbr_ifsname[sizeof(req.ifbr_ifsname) - 1] = '\0';
		req.ifbr_action = n->brl_action;
		req.ifbr_flags = n->brl_flags;
		req.ifbr_src = n->brl_src;
		req.ifbr_dst = n->brl_dst;
		error = copyout((caddr_t)&req,
		    (caddr_t)(bc->ifbrl_buf + (i * sizeof(req))), sizeof(req));
		if (error)
			goto done;
		n = SIMPLEQ_NEXT(n, brl_next);
		i++;
		bc->ifbrl_len -= sizeof(req);
	}

	n = SIMPLEQ_FIRST(&ifl->bif_brlout);
	while (n != NULL && bc->ifbrl_len > i * sizeof(req)) {
		//strncpy(req.ifbr_name, sc->sc_if.if_xname,sizeof(req.ifbr_name) - 1);
		sprintf(ifname,"%s%d",sc->sc_if.if_name,sc->sc_if.if_unit);
		strncpy(req.ifbr_name, ifname,sizeof(req.ifbr_name) - 1);
		req.ifbr_name[sizeof(req.ifbr_name) - 1] = '\0';
		//strncpy(req.ifbr_ifsname, ifl->ifp->if_xname,sizeof(req.ifbr_ifsname) - 1);
		sprintf(ifname,"%s%d",ifl->ifp->if_name,ifl->ifp->if_unit);
		strncpy(req.ifbr_ifsname, ifname,sizeof(req.ifbr_ifsname) - 1);
		req.ifbr_ifsname[sizeof(req.ifbr_ifsname) - 1] = '\0';
		req.ifbr_action = n->brl_action;
		req.ifbr_flags = n->brl_flags;
		req.ifbr_src = n->brl_src;
		req.ifbr_dst = n->brl_dst;
		error = copyout((caddr_t)&req,
		    (caddr_t)(bc->ifbrl_buf + (i * sizeof(req))), sizeof(req));
		if (error)
			goto done;
		n = SIMPLEQ_NEXT(n, brl_next);
		i++;
		bc->ifbrl_len -= sizeof(req);
	}

done:
	bc->ifbrl_len = i * sizeof(req);
	return (error);
}

void
bridge_init(sc)
	struct bridge_softc *sc;
{
	struct ifnet *ifp = &sc->sc_if;
	int i, s;
#if defined (CONFIG_RTL_IGMP_SNOOPING)&& defined (CONFIG_RTL_BRIDGE_QUERY_SUPPORT)

#endif

	if ((ifp->if_flags & IFF_RUNNING) == IFF_RUNNING)
		return;
	register_netisr(NETISR_BRIDGE, bridgeintr);
	s = splhigh();
	if (sc->sc_rts == NULL) {
		sc->sc_rts = (struct bridge_rthead *)malloc(
		    BRIDGE_RTABLE_SIZE * (sizeof(struct bridge_rthead)),
		    M_DEVBUF, M_NOWAIT);
		if (sc->sc_rts == NULL) {
			splx(s);
			return;
		}
		for (i = 0; i < BRIDGE_RTABLE_SIZE; i++) {
			LIST_INIT(&sc->sc_rts[i]);
		}
	}
	ifp->if_flags |= IFF_RUNNING;
	splx(s);
	
	if (sc->sc_brttimeout != 0) {
		callout_init(&br_timer);
		callout_reset(&br_timer,sc->sc_brttimeout ,bridge_rtage, sc);
	}
	//lq igmp初始化的只需要初始化一次
	if(ifp->if_unit != 0 )
		return;
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	int retVal;
	#if defined(CONFIG_RTL_819X_SWCORE)
	retVal=rtl_registerIgmpSnoopingModule(&brIgmpModuleIndex);	
	#endif
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(retVal==SUCCESS)
	{
		rtl_multicastDeviceInfo_t devInfo;
		memset(&devInfo, 0 , sizeof(rtl_multicastDeviceInfo_t));
		strcpy(devInfo.devName, RTL_PS_BR0_DEV_NAME);
		#if defined(CONFIG_RTL_819X_SWCORE)
		rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex, &devInfo);
		#endif
	}
	#endif
#if defined (CONFIG_RTL_BRIDGE_QUERY_SUPPORT)
	
	if(sc!=NULL){
		callout_init(&mCastQuerytimer);
		callout_reset(&mCastQuerytimer,(MCAST_QUERY_INTERVAL* hz) ,bridge_mCastQueryTimerExpired, sc);
	}	
#endif
#if defined(CONFIG_RTL_819X_SWCORE)
	rtl_setIpv4UnknownMCastFloodMap(brIgmpModuleIndex, 0xFFFFFFFF);
#ifdef CONFIG_RTL_MLD_SNOOPING		
	rtl_setIpv6UnknownMCastFloodMap(brIgmpModuleIndex, 0xFFFFFFFF);
#endif
#endif
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(name,RTL_PS_BR1_DEV_NAME)==0)
	{
		rtl_multicastDeviceInfo_t devInfo2;
		memset(&devInfo2, 0, sizeof(rtl_multicastDeviceInfo_t ));
		strcpy(devInfo2.devName, RTL_PS_BR1_DEV_NAME);
		
		ret=rtl_registerIgmpSnoopingModule(&brIgmpModuleIndex_2);
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(ret==0)
		{
			#if defined(CONFIG_RTL_819X_SWCORE)
			 rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex_2,&devInfo2);
			#endif
		}
	#endif
		bridge1=netdev_priv(dev);
		if(bridge1!=NULL)
		{
			init_timer(&bridge1->mCastQuerytimer);
			bridge1->mCastQuerytimer.data=bridge1;
			bridge1->mCastQuerytimer.expires=jiffies+MCAST_QUERY_INTERVAL*HZ;
			bridge1->mCastQuerytimer.function=(void*)br_mCastQueryTimerExpired;
			add_timer(&bridge1->mCastQuerytimer);
		}
	}
#endif
#endif
}

/*
 * Stop the bridge and deallocate the routing table.
 */
void
bridge_stop(sc)
	struct bridge_softc *sc;
{
	struct ifnet *ifp = &sc->sc_if;

	/*
	 * If we're not running, there's nothing to do.
	 */
	if ((ifp->if_flags & IFF_RUNNING) == 0)
		return;

	//untimeout(bridge_rtage, sc);
	callout_stop(&br_timer);
	bridge_rtflush(sc, IFBF_FLUSHDYN);

	ifp->if_flags &= ~IFF_RUNNING;
}

/*
 * Send output from the bridge.  The mbuf has the ethernet header
 * already attached.  We must enqueue or free the mbuf before exiting.
 */
int
bridge_output(ifp, m, sa, rt)
	struct ifnet *ifp;
	struct mbuf *m;
	struct sockaddr *sa;
	struct rtentry *rt;
{
	struct ether_header *eh;
	struct ifnet *dst_if;
	struct ether_addr *src, *dst;
	struct bridge_softc *sc;
	int s;
#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
	extern unsigned int _br0_ip;
#endif
	
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	int srcPort=0xFFFF;
	int srcVlanId =0 ;
	unsigned int pvlanId=0;
	struct bridge_softc *br_sc ;
	br_sc = &bridgectl[0];
	//srcPort= m->m_pkthdr.srcPort;
	//srcVlanId = m->m_pkthdr.srcVlanId;
	#endif
	
	if (m->m_len < sizeof(*eh)) {
		m = m_pullup(m, sizeof(*eh));
		if (m == NULL)
			return (0);
	}
	eh = mtod(m, struct ether_header *);
	dst = (struct ether_addr *)&eh->ether_dhost[0];
	src = (struct ether_addr *)&eh->ether_shost[0];
	sc = (struct bridge_softc *)ifp->if_bridge;
	
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		srcPort= m->m_pkthdr.srcPort;
		srcVlanId = m->m_pkthdr.srcVlanId;
	#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
		if(m&&(m->m_pkthdr.tag.f.tpid == htons(ETH_P_8021Q))){
			pvlanId = ntohs(m->m_pkthdr.tag.f.pci&0xfff);
			//diag_printf("vlanId:%d,[%s]:[%d].\n",vlanId,__FUNCTION__,__LINE__);
		}	
			
	#endif
		
	#endif
	
	s = splimp();

	/*
	 * If bridge is down, but original output interface is up,
	 * go ahead and send out that interface.  Otherwise the packet
	 * is dropped below.
	 */
	if ((sc->sc_if.if_flags & IFF_RUNNING) == 0) {
		dst_if = ifp;
		goto sendunicast;
	}
	/*
	 * If the packet is a broadcast or we don't know a better way to
	 * get there, send to all interfaces.
	 */
	dst_if = bridge_rtlookup(sc, dst);
	if (dst_if == NULL || eh->ether_dhost[0] & 1) {
		struct bridge_iflist *p;
		struct mbuf *mc;
		int used = 0;
		
		//diag_printf("dst_if:%x,%s,[%s]:[%d].\n",dst_if->if_index,dst_if->if_xname,__FUNCTION__,__LINE__);
#if defined (CONFIG_RTL_IGMP_SNOOPING)	
		u_int32_t mcastFwdPortMask=0xFFFFFFFF;
		#if defined(CONFIG_RTL_819X_SWCORE)
		if(igmpsnoopenabled) 
		#endif
		{	
			struct ip *iph=NULL;
			unsigned char proto=0;
			unsigned char ret=0;
			unsigned char reserved=0;
			
			struct rtl_multicastDataInfo multicastDataInfo;
			struct rtl_multicastFwdInfo multicastFwdInfo;
			if(ETHERADDR_IS_IP_MCAST(dst) 
			&& ( ntohs(eh->ether_type) == ETH_P_IP))
			{
				//diag_printf("m->m_len:%d,[%s]:[%d]. \n",m->m_len,__FUNCTION__,__LINE__);
				if(m->m_len < (sizeof(struct ether_header)+sizeof (struct ip))){
					m = m_pullup(m, (sizeof(struct ether_header)+sizeof (struct ip))) ;
					if (m == NULL){
						return (0);
					}	
				}	
				iph =(struct ip*)( m->m_data+sizeof(struct ether_header));
				proto =  iph->ip_p;
				//diag_printf("bridge_output.proto:%d,dst:%x,[%s]:[%d].\n",proto,iph->ip_dst.s_addr,__FUNCTION__,__LINE__);
		#if 0
				if(( iph->daddr&0xFFFFFF00)==0xE0000000)
				{
						reserved=1;
				}
		#endif
				/*0xE00001B2 == 224.0.1.178 for IAPP mutilcast */
#if defined(CONFIG_USB_UWIFI_HOST)
				if(ntohl(iph->ip_dst.s_addr) == 0xEFFFFFFA || ntohl(iph->ip_dst.s_addr) == 0xE1010101 ||(ntohl(iph->ip_dst.s_addr) == 0xE00001B2))
#else
				if(ntohl(iph->ip_dst.s_addr) == 0xEFFFFFFA ||(ntohl(iph->ip_dst.s_addr) == 0xE00001B2))
#endif
				{
					/*for microsoft upnp*/
					reserved=1;
				}
				
				if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))	&& (reserved ==0))
				{
					multicastDataInfo.ipVersion=4;
					multicastDataInfo.sourceIp[0]=	(u_int32_t)(iph->ip_src.s_addr);
					multicastDataInfo.groupAddr[0]=  (u_int32_t)(iph->ip_dst.s_addr);
					multicastDataInfo.sourceIp[0]=	ntohl(multicastDataInfo.sourceIp[0]);
					multicastDataInfo.groupAddr[0]=  ntohl(multicastDataInfo.groupAddr[0]);
					
					#if defined(CONFIG_RTL_819X_SWCORE)
					ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
					#endif
					mcastFwdPortMask = multicastFwdInfo.fwdPortMask;
					#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
					//to be check _br0_ip little endian issue
					if((ret!=SUCCESS)&&(_br0_ip!=ntohl(iph->ip_src.s_addr)))
						mcastFwdPortMask=0;
					#endif
					//diag_printf("grp:%x,mcastFwdPortMask:%x,%d,%d,[%s]:[%d].\n",multicastDataInfo.groupAddr[0],mcastFwdPortMask,srcPort,srcVlanId,__FUNCTION__,__LINE__);
					if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
					{
						
				#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
						if((srcVlanId!=0) && (srcPort!=0xFFFF))
						{
					#if defined(CONFIG_RTL_819X_SWCORE)	
					#if defined(CONFIG_RTL_VLAN_SUPPORT)
					#ifndef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
							if(rtl_vlan_support_enable == 0)
					#endif		
							{
								rtl865x_ipMulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0],pvlanId);
							}
					#else
							rtl865x_ipMulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0],pvlanId);
					#endif
					#endif
						}
				#endif		
					}
					
				
				}
				else{
					//diag_printf("bridge_output.proto:%d[%s]:[%d].\n",proto,__FUNCTION__,__LINE__);
					#if 0
					diag_printf("bridge_output.mdata:%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x.\n",
						*(m->m_data),*(m->m_data+1),*(m->m_data+2),*(m->m_data+3),*(m->m_data+4),*(m->m_data+5),
						*(m->m_data+6),*(m->m_data+7),*(m->m_data+8),*(m->m_data+9),*(m->m_data+10),*(m->m_data+11)
						);
					#endif
				}
				
			}
#if defined(CONFIG_RTL_MLD_SNOOPING)	
			else if(mldSnoopEnabled && (ETHERADDR_IS_IPv6_MCAST(dst)
				&& ( ntohs(eh->ether_type) == ETH_P_IPV6)))
			{
				struct ip6_hdr *ipv6h=NULL;
				#if 1
				if(m->m_len < (sizeof(struct ether_header)+sizeof (struct ip6_hdr))){
					m = m_pullup(m, (sizeof(struct ether_header)+sizeof (struct ip6_hdr))) ;
					//diag_printf("m->m_len:%d,[%s]:[%d]. \n",m->m_len,__FUNCTION__,__LINE__);
					if (m == NULL){
						//diag_printf("m->m_len:%d,[%s]:[%d]. \n",m->m_len,__FUNCTION__,__LINE__);
						return (0);
					}	
				}	
				#endif
				ipv6h =(struct ip6_hdr*)( m->m_data+sizeof(struct ether_header));
				proto=re865x_getIpv6TransportProtocol(ipv6h);
				if ((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
				{
					multicastDataInfo.ipVersion=6;
					memcpy(&multicastDataInfo.sourceIp, &ipv6h->ip6_src, sizeof(struct in6_addr));
					memcpy(&multicastDataInfo.groupAddr,&ipv6h->ip6_dst, sizeof(struct in6_addr));

					multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
					multicastDataInfo.sourceIp[1] = ntohl(multicastDataInfo.sourceIp[1]);
					multicastDataInfo.sourceIp[2] = ntohl(multicastDataInfo.sourceIp[2]);
					multicastDataInfo.sourceIp[3] = ntohl(multicastDataInfo.sourceIp[3]);
					multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
					multicastDataInfo.groupAddr[1] = ntohl(multicastDataInfo.groupAddr[1]);
					multicastDataInfo.groupAddr[2] = ntohl(multicastDataInfo.groupAddr[2]);
					multicastDataInfo.groupAddr[3] = ntohl(multicastDataInfo.groupAddr[3]);
					
					#if defined(CONFIG_RTL_819X_SWCORE)
					ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
					#endif
					//br_multicast_deliver(br, multicastFwdInfo.fwdPortMask, skb, 0);
					mcastFwdPortMask = multicastFwdInfo.fwdPortMask;
					
#if defined (CONFIG_RTL_HARDWARE_MULTICAST) && defined(CONFIG_RTL_8197F)
					if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
					{

						if((srcVlanId!=0) && (srcPort!=0xFFFF))
						{
	#if defined(CONFIG_RTL_819X_SWCORE) 
	#if defined(CONFIG_RTL_VLAN_SUPPORT)
	#ifndef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
							if(rtl_vlan_support_enable == 0)
	#endif		
							{
								rtl819x_ipv6MulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp, multicastDataInfo.groupAddr,pvlanId);
							}
	#else
							rtl819x_ipv6MulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp, multicastDataInfo.groupAddr,pvlanId);
	#endif
	#endif
						}
					}
#endif		
				}
				
			}
#endif		
			
		}
		
#endif

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
                bridge_span(sc, NULL, m);
#endif

		for (p = LIST_FIRST(&sc->sc_iflist); p != NULL;
		     p = LIST_NEXT(p, next)) {
			if(!p->ifp) 	
				continue;
			if ((p->ifp->if_flags & IFF_RUNNING) == 0)
					continue;
			
			#if defined (CONFIG_RTL_IGMP_SNOOPING)	
			if (eh->ether_dhost[0] & 1){
				u_int port_bitmask = (1 << p->ifp->if_index);
				//diag_printf("bridge_output: mcastFwdPortMask:%x,[%s]:[%d].\n",mcastFwdPortMask,__FUNCTION__,__LINE__);
				if(!(port_bitmask & mcastFwdPortMask)){
					//diag_printf("not forwarding to ifp:%x,%s,[%s]:[%d].\n",p->ifp->if_index,p->ifp->if_xname,__FUNCTION__,__LINE__);
					continue;
				}
				//diag_printf("forwarding to ifp:%x,%s,[%s]:[%d].\n",p->ifp->if_index,p->ifp->if_xname,__FUNCTION__,__LINE__);
			}
			#endif
            #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
			 if (should_deliver(p->ifp, m) == 0){
                //diag_printf("not forwarding to ifp:%s%x,[%s]:[%d].\n",p->ifp->if_xname,p->ifp->if_unit,__FUNCTION__,__LINE__);   
                continue;
            } 
            #endif
			if (IF_QFULL(&p->ifp->if_snd)) {
				sc->sc_if.if_oerrors++;
				continue;
			}

			if (LIST_NEXT(p, next) == NULL) {
				used = 1;
				mc = m;
			} else {
				mc = m_copym(m, 0, M_COPYALL, M_NOWAIT);
				if (mc == NULL) {
					sc->sc_if.if_oerrors++;
					continue;
				}
			}
          //diag_printf("ifp:%s,%d.[%s]:[%d].\n",p->ifp->if_name,p->ifp->if_unit,__FUNCTION__,__LINE__);
			sc->sc_if.if_opackets++;
			sc->sc_if.if_obytes += m->m_pkthdr.len;
                        // Also count the bytes in the outgoing interface; normally
                        // done in if_ethersubr.c but here we bypass that route.
                        p->ifp->if_obytes += m->m_pkthdr.len;
			IF_ENQUEUE(&p->ifp->if_snd, mc);
			if ((p->ifp->if_flags & IFF_OACTIVE) == 0)
				(*p->ifp->if_start)(p->ifp);
		}
		if (!used)
			m_freem(m);
		splx(s);
		return (0);
	}
sendunicast:
	if ((dst_if->if_flags & IFF_RUNNING) == 0) {
		m_freem(m);
		splx(s);
		return (0);
	}
	if (IF_QFULL(&dst_if->if_snd)) {
		sc->sc_if.if_oerrors++;
		m_freem(m);
		splx(s);
		return (0);
	}
	sc->sc_if.if_opackets++;
	sc->sc_if.if_obytes += m->m_pkthdr.len;
        // Also count the bytes in the outgoing interface; normally
        // done in if_ethersubr.c but here we bypass that route.
        dst_if->if_obytes += m->m_pkthdr.len;
	IF_ENQUEUE(&dst_if->if_snd, m);
	if ((dst_if->if_flags & IFF_OACTIVE) == 0)
		(*dst_if->if_start)(dst_if);
	splx(s);
	return (0);
}

/*
 * Start output on the bridge.  This function should never be called.
 */
void
bridge_start(ifp)
	struct ifnet *ifp;
{
}

void
bridgeintr(void)
{
	struct bridge_softc *sc;
	struct mbuf *m;
	int i, s;
	
	for (i = 0; i < CYGNUM_NET_BRIDGES; i++) {
		sc = &bridgectl[i];		
		for (;;) {
			s = splimp();
			IF_DEQUEUE(&sc->sc_if.if_snd, m);
			splx(s);
			if (m == NULL)
				break;
			bridgeintr_frame(sc, m);
		}
	}
}

/*
 * Loop through each bridge interface and process their input queues.
 */

#if defined (CONFIG_RTL_IGMP_SNOOPING)

struct bridge_softc *rtl_getBridge(int brIndex)
{
	struct bridge_softc *sc = NULL ;

	if (brIndex < CYGNUM_NET_BRIDGES)
	{
		sc =  &bridgectl[brIndex];
	}
	
	return sc;
}

#if defined (IPV6_MCAST_TO_UNICAST)
/*Convert  MultiCatst IPV6_Addr to MAC_Addr*/
static void CIPV6toMac
	(unsigned char* icmpv6_McastAddr, unsigned char *gmac )
{
	/*ICMPv6 valid addr 2^32 -1*/
	gmac[0] = 0x33;
	gmac[1] = 0x33;
	gmac[2] = icmpv6_McastAddr[12];
	gmac[3] = icmpv6_McastAddr[13];
	gmac[4] = icmpv6_McastAddr[14];
	gmac[5] = icmpv6_McastAddr[15];			
}



//static char ICMPv6_check(struct mbuf *m , unsigned char *gmac)
static char ICMPv6_check(unsigned char *data , unsigned char *gmac)
{
	
	struct ip6_hdr *ipv6h;
	char* protoType;	
	
	/* check IPv6 header information */
	
	//ipv6h=(struct ip6_hdr *)( m->m_data+sizeof(struct ether_header));
	ipv6h=(struct ip6_hdr *)( data+sizeof(struct ether_header));
	if(ipv6h->ip6_vfc != IPV6_VERSION){	
		#ifdef	DBG_ICMPv6	
		diag_printf("not IPV6_VERSION\n");
		#endif
		return -1;
	}


	/*Next header: IPv6 hop-by-hop option (0x00)*/
	if(ipv6h->ip6_nxt == 0)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ip6_hdr) );	
	}else{
		//printk("ipv6h->nexthdr != 0\n");
		return -1;
	}

	if(protoType[0] == 0x3a){
		
		//printk("recv icmpv6 packet\n");
		struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		unsigned char* icmpv6_McastAddr ;
	
		if(icmpv6h->icmp6_type == 0x83){
			
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8);
			#ifdef	DBG_ICMPv6					
			diag_printf("Type: 0x%x (Multicast listener report) \n",icmpv6h->icmp6_type);
			#endif

		}else if(icmpv6h->icmp6_type == 0x8f){		
		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 + 4);
			#ifdef	DBG_ICMPv6					
			diag_printf("Type: 0x%x (Multicast listener report v2) \n",icmpv6h->icmp6_type);
			#endif			
		}else if(icmpv6h->icmp6_type == 0x84){
		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 );			
			#ifdef	DBG_ICMPv6					
			diag_printf("Type: 0x%x (Multicast listener done ) \n",icmpv6h->icmp6_type);
			#endif			
		}
		else{
			#ifdef	DBG_ICMPv6
			diag_printf("Type: 0x%x (unknow type)\n",icmpv6h->icmp6_type);
			#endif			
			return -1;
		}				

		#ifdef	DBG_ICMPv6			
		diag_printf("MCAST_IPV6Addr:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			icmpv6_McastAddr[0],icmpv6_McastAddr[1],icmpv6_McastAddr[2],icmpv6_McastAddr[3],
			icmpv6_McastAddr[4],icmpv6_McastAddr[5],icmpv6_McastAddr[6],icmpv6_McastAddr[7],
			icmpv6_McastAddr[8],icmpv6_McastAddr[9],icmpv6_McastAddr[10],icmpv6_McastAddr[11],
			icmpv6_McastAddr[12],icmpv6_McastAddr[13],icmpv6_McastAddr[14],icmpv6_McastAddr[15]);
		#endif

		CIPV6toMac(icmpv6_McastAddr, gmac);
		
		#ifdef	DBG_ICMPv6					
		diag_printf("group_mac [%02x:%02x:%02x:%02x:%02x:%02x] \n",
			gmac[0],gmac[1],gmac[2],
			gmac[3],gmac[4],gmac[5]);
		#endif
			


		if(icmpv6h->icmp6_type == 0x83){

			return 1;//icmpv6 listener report (add)
		}
		else if(icmpv6h->icmp6_type == 0x8f){
			return 1;//icmpv6 listener report v2 (add) 
		}
		else if(icmpv6h->icmp6_type == 0x84){
			return 2;//icmpv6 Multicast listener done (del)
		}
	}		
	else{
		//printk("protoType[0] != 0x3a\n");		
		return -1;//not icmpv6 type
	}
		
	return -1;
}

#endif	//end of IPV6_MCAST_TO_UNICAST

/*2008-01-15,for porting igmp snooping to linux kernel 2.6*/
static void ConvertMulticatIPtoMacAddr(__u32 group, unsigned char *gmac)
{
	__u32 u32tmp, tmp;
	int i;

	u32tmp = group & 0x007FFFFF;
	gmac[0]=0x01; gmac[1]=0x00; gmac[2]=0x5e;
	for (i=5; i>=3; i--) {
		tmp=u32tmp&0xFF;
		gmac[i]=tmp;
		u32tmp >>= 8;
	}
}

//static char igmp_type_check(struct mbuf *m, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag)
static char igmp_type_check(unsigned char *data, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag)
{
    struct ip *iph;
	__u8 hdrlen;
	u8 version;
	struct igmp *igmph;
	int i;
	unsigned int groupAddr=0;// add  for fit igmp v3
	u_short sum;
	*moreFlag=0;
	/* check IP header information */
	iph=(struct ip*)( data+sizeof(struct ether_header));
	hdrlen = iph->ip_hl << 2;
	version=iph->ip_v ;
	//diag_printf("hdrlen:%d,version:%d,[%s]:[%d].\n",hdrlen,version,__FUNCTION__,__LINE__);
	if ((version != 4) &&  (hdrlen < 20))
		return -1;
	#if 0
	if (m->m_pkthdr.csum_flags & CSUM_IP_CHECKED) {
		sum = !(m->m_pkthdr.csum_flags & CSUM_IP_VALID);
	} else {
		
		sum = in_cksum(m, hdrlen);
		
	}
	if (sum) {
		diag_printf("checksum error![s]:[%d].\n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	
	{ /* check the length */
		__u32 len = ntohs(iph->ip_len);
		if (m->m_pkthdr.len < len || len < hdrlen)
			return -1; 
	}
	#endif 
	/* parsing the igmp packet */
	igmph = (struct igmp *)((u8*)iph+hdrlen);

	
	
	if ((igmph->igmp_type==IGMP_V1_MEMBERSHIP_REPORT) ||
	    (igmph->igmp_type==IGMP_V2_MEMBERSHIP_REPORT)) 
	{
		groupAddr = ntohl(igmph->igmp_group.s_addr);
		if(!IN_MULTICAST(groupAddr))
		{			
				return -1;
		}
		
		ConvertMulticatIPtoMacAddr(groupAddr, gmac);
		//diag_printf("report and add it.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		return 1; /* report and add it */
	}
	else if (igmph->igmp_type==IGMPV3_HOST_MEMBERSHIP_REPORT)	{ 
		
	
		/*for support igmp v3 ; plusWang add 2009-0311*/   	
		struct igmpv3_report *igmpv3report=(struct igmpv3_report * )igmph;
		struct igmpv3_grec	*igmpv3grec=NULL; 
		//diag_printf("%s:%d,*gIndex is %d,igmpv3report->ngrec is %d\n",__FUNCTION__,__LINE__,*gIndex,igmpv3report->ngrec);
		if(*gIndex>=ntohs(igmpv3report->ngrec))
		{
			*moreFlag=0;
			return -1;
		}
	
		for(i=0;i<ntohs(igmpv3report->ngrec);i++)
		{

			if(i==0)
			{
				igmpv3grec = (struct igmpv3_grec *)(&(igmpv3report->grec)); /*first igmp group record*/
			}
			else
			{
				igmpv3grec=(struct igmpv3_grec *)((unsigned char*)igmpv3grec+8+ntohs(igmpv3grec->grec_nsrcs)*4+(igmpv3grec->grec_auxwords)*4);
				
				
			}
			
			if(i!=*gIndex)
			{	
				
				continue;
			}
			
			if(i==(ntohs(igmpv3report->ngrec)-1))
			{
				/*last group record*/
				*moreFlag=0;
			}
			else
			{
				*moreFlag=1;
			}
			
			/*gIndex move to next group*/
			*gIndex=*gIndex+1;	
			
			groupAddr=ntohl(igmpv3grec->grec_mca.s_addr);
			//diag_printf("%s:%d,groupAddr is %d.%d.%d.%d\n",__FUNCTION__,__LINE__,NIPQUAD(groupAddr));
			if(!IN_MULTICAST(groupAddr))
			{			
				return -1;
			}
			
			ConvertMulticatIPtoMacAddr(groupAddr, gmac);
			if(((igmpv3grec->grec_type == IGMPV3_CHANGE_TO_INCLUDE) || (igmpv3grec->grec_type == IGMPV3_MODE_IS_INCLUDE))&& (ntohs(igmpv3grec->grec_nsrcs)==0))
			{	
				//diag_printf(" leave and delete it.[%s]:[%d].\n",__FUNCTION__,__LINE__);
				return 2; /* leave and delete it */	
			}
			else if((igmpv3grec->grec_type == IGMPV3_CHANGE_TO_EXCLUDE) ||
				(igmpv3grec->grec_type == IGMPV3_MODE_IS_EXCLUDE) ||
				(igmpv3grec->grec_type == IGMPV3_ALLOW_NEW_SOURCES))
			{
				//diag_printf("report and add it.[%s]:[%d].\n",__FUNCTION__,__LINE__);
				return 1;
			}
			else
			{
				/*ignore it*/
			}
			
			return -1;
		}
		
		/*avoid dead loop in case of initial gIndex is too big*/
		if(i>=(ntohs(igmpv3report->ngrec)-1))
		{
			/*last group record*/
			*moreFlag=0;
			return -1;
		}
		
	
	}
	else if (igmph->igmp_type==IGMP_HOST_LEAVE_MESSAGE){

		groupAddr = ntohl(igmph->igmp_group.s_addr);
		if(!IN_MULTICAST(groupAddr))
		{			
				return -1;
		}
		
		ConvertMulticatIPtoMacAddr(groupAddr, gmac);
		//diag_printf(" leave and delete it.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		return 2; /* leave and delete it */
	}	
	
	
	return -1;
}

int chk_igmp_ext_entry(struct bridge_rtnode *rt ,unsigned char *srcMac);
void add_igmp_ext_entry(struct bridge_rtnode *rt , unsigned char *srcMac , unsigned char portComeIn);
void update_igmp_ext_entry(	struct bridge_rtnode *rt,unsigned char *srcMac , unsigned char portComeIn);
void del_igmp_ext_entry(	struct bridge_rtnode *rt,unsigned char *srcMac , unsigned char portComeIn );
void br_igmp_ext_entry_expired(struct bridge_rtnode *rt);
int get_igmp_ext_entryCnt( struct bridge_rtnode *rt);


//static void br_update_igmp_snoop_rt_entry(unsigned char op, struct bridge_softc *sc, struct ifnet *src_if, unsigned char *dest ,struct mbuf *m)
static void br_update_igmp_snoop_rt_entry(unsigned char op, struct bridge_softc *sc, struct ifnet *src_if, unsigned char *dest , unsigned char *data)
{
	struct bridge_rtnode *rt;
	unsigned char *src;
	unsigned short del_group_src=0;
	unsigned char port_comein;
	int tt1;
	int portNum;
	if(src_if==NULL)
		return;
	portNum=src_if->if_index;
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	struct ether_header *eh;
#else
	struct ether_header eh;
#endif
#if defined (MCAST_TO_UNICAST)
	struct net_device *dev; 
	struct eth_drv_sc *drv_sc;
	Rltk819x_t *rltk819x_info;
	drv_sc= (struct eth_drv_sc *)src_if->if_softc;
	rltk819x_info = (Rltk819x_t *)(drv_sc->driver_private);

	dev = (struct net_device *)(rltk819x_info->dev);
	if(!dev){
		//diag_printf("not get dev RETRUN. [%s]:[%d].\n",__FUNCTION__,__LINE__);
		return;
	}	

#if 0	
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	eh = mtod(m, struct ether_header *);
	src = (struct ether_addr *)&eh->ether_shost[0];
#else	
	m_copydata(m, 0, sizeof(struct ether_header), (caddr_t)&eh);
	src = (struct ether_addr *)&eh.ether_shost[0];
#endif
#else
	eh = (struct ether_header *)data;
	src = (struct ether_addr *)&eh->ether_shost[0];
#endif

	if(!dest){
		//diag_printf("!dest,RETRUN.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		return;
	}	
	if( !IPV4_MULTICAST_MAC(dest)
#if defined (IPV6_MCAST_TO_UNICAST)
		&& !IPV6_MULTICAST_MAC(dest)
#endif	
	   )
	   { 
	   //diag_printf("NOT MULTICAST.[%s]:[%d].\n",__FUNCTION__,__LINE__);
	   	return; 
	   }
#endif

	//if(dest == 0xEFFFFFFA)
	//	return;

#if defined( CONFIG_RTL_HARDWARE_MULTICAST) || defined(CONFIG_RTL865X_LANPORT_RESTRICTION)

	if(src_if->if_index!=0xFFFF)
	{
		port_comein = 1<<src_if->if_index;
	}
	else
	{
		port_comein=0x80;
	}
	
#else
	if(src_if && dev && dev->name && !memcmp(dev->name, RTL_PS_LAN_P0_DEV_NAME, 4))
	{
		port_comein = 0x01;
	}
	
	if(src_if && dev && dev->name && !memcmp(dev->name, RTL_PS_WLAN_NAME, 4))
	{
		port_comein=0x80;
	}
	
#endif
#if defined (MCAST_TO_UNICAST)

	//src=(unsigned char*)(skb_mac_header(skb)+ETH_ALEN);
	/* check whether entry exist */
	rt = bridge_rtget(sc, dest);

	if (op == 1) /* add */
	{	
		//diag_printf("dev:%s add.[%s]:[%d].\n",dev->name,__FUNCTION__,__LINE__);	
#if 1
		if (rt) 
		{
			DEBUG_PRINT("rt exist. \n");
			rt->group_src = rt->group_src | (1 << portNum);
			rt->brt_age= 1;
			tt1 = chk_igmp_ext_entry(rt , src); 
			if(tt1 == 0)
			{
				add_igmp_ext_entry(rt , src , port_comein); 								
			}
			else
			{
				update_igmp_ext_entry(rt , src , port_comein);
			}	
		}
		else
		{
			/* update rt */
			DEBUG_PRINT("update rt\n");
			bridge_rtupdate(sc, dest, src_if, 0, IFBAF_DYNAMIC);
			rt = bridge_rtget(sc, dest);
			if(rt !=NULL)
			{
				rt->igmpFlag=1;
				rt->brt_age= 1;
				rt->portlist = port_comein; 
				rt->group_src = rt->group_src | (1 << portNum);
				DEBUG_PRINT("rt exist. \n");
				
				tt1 = chk_igmp_ext_entry(rt , src); 
				if(tt1 == 0)
				{
					add_igmp_ext_entry(rt , src , port_comein); 								
				}
				else
				{
					update_igmp_ext_entry(rt , src , port_comein);
				}	
			}
		}
#endif 
		
#if defined (MCAST_TO_UNICAST)
		/*process wlan client join --- start*/
		//if (dst && p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4)) 
		if (rt&&src_if && dev && dev->name && !memcmp(dev->name, RTL_PS_WLAN_NAME, 4)) 
		{ 
			rt->portlist |= 0x80;
			port_comein = 0x80;
			
			if (dev) 
			{		
				unsigned char StaMacAndGroup[20];
				memcpy(StaMacAndGroup, dest, 6);
				memcpy(StaMacAndGroup+6, src, 6);	
			#if 1//defined(CONFIG_COMPAT_NET_DEV_OPS)
				if (dev->do_ioctl != NULL) 
				{
					dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, SIOCGIMCAST_ADD);
					DEBUG_PRINT("... add to wlan mcast table:  DA:%02x:%02x:%02x:%02x:%02x:%02x ; SA:%02x:%02x:%02x:%02x:%02x:%02x\n", 
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);					
				}
			#else
				if (dev->netdev_ops->ndo_do_ioctl != NULL) 
				{
					dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B80);
					DEBUG_PRINT("... add to wlan mcast table:  DA:%02x:%02x:%02x:%02x:%02x:%02x ; SA:%02x:%02x:%02x:%02x:%02x:%02x\n", 
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);	
				}
			#endif
				
														
			}
		}
	/*process wlan client join --- end*/
#endif

		
	}
	
	else if (op == 2 && rt) /* delete */
	{
		//diag_printf("dev:%s delete.[%s]:[%d].\n",dev->name,__FUNCTION__,__LINE__);
		DEBUG_PRINT("dst->group_src = %x change to ",rt->group_src);		
			del_group_src = ~(1 << src_if->if_index);
			rt->group_src = rt->group_src & del_group_src;
		DEBUG_PRINT(" %x ; port_no=%x \n",rt->group_src , src_if->if_index);
			
		/*process wlan client leave --- start*/
		//if (p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4)) 
		if (src_if && dev && dev->name && !memcmp(dev->name, RTL_PS_WLAN_NAME, 4))
		{ 			
			#ifdef	MCAST_TO_UNICAST
			//struct net_device *dev = __dev_get_by_name(&init_net,RTL_PS_WLAN0_DEV_NAME);
			//struct net_device *dev=p->dev;
			if (dev) 
			{			
				unsigned char StaMacAndGroup[12];
				memcpy(StaMacAndGroup, dest , 6);
				memcpy(StaMacAndGroup+6, src, 6);
			#if 1//defined(CONFIG_COMPAT_NET_DEV_OPS)
				if (dev->do_ioctl != NULL) 
				{
					dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, SIOCGIMCAST_DEL);									
					DEBUG_PRINT("(del) wlan0 ioctl (del) M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
				}
			#else
				if (dev->netdev_ops->ndo_do_ioctl != NULL) 
				{
					dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);				
					DEBUG_PRINT("(del) wlan0 ioctl (del) M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
				}
			#endif	
			
			}
			#endif	
			rt->portlist &= ~0x80;	// move to del_igmp_ext_entry
			port_comein	= 0x80;
		}
		/*process wlan client leave --- end*/
		#if 1
		/*process entry del , portlist update*/
		del_igmp_ext_entry(rt , src ,port_comein);
		
		if (rt->portlist == 0)  // all joined sta are gone
		{
			DEBUG_PRINT("----all joined sta are gone,make it expired----\n");
			rt->igmpFlag=0;
			rt->brt_age=0;		
		}
		#endif

	}
	
#endif	
}

#endif // CONFIG_RTL_IGMP_SNOOPING

void
bridgeintr_frame(
	struct bridge_softc *sc,
	struct mbuf *m)
{
#if defined(CONFIG_RTL_DNS_TRAP)
	int recapped = 0;
#endif
	int s;
	struct ifnet *src_if, *dst_if;
	struct bridge_iflist *ifl;
	struct ether_addr *dst, *src;
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	//struct ether_header *eh;	 
	/* pointer eh not initialized may cause problem in line 2039(m_copydata) when open macro 
	    CYGPKG_NET_BRIDGE_STP_CODE */
    struct ether_header *eh = mtod(m, struct ether_header *);
#else
	struct ether_header eh;
#endif
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	unsigned char igmpMldData[1500]={0};//recode igmp packet for parse
	int igmpMldLen;
#endif
#if defined  (CONFIG_RTL_HARDWARE_MULTICAST)
	struct bridge_softc *br_sc ;
	br_sc = &bridgectl[0];
	int srcPort=0xFFFF;	
	int srcVlanId =0;
	unsigned int pvlanId=0;

	srcPort= m->m_pkthdr.srcPort;
	srcVlanId= m->m_pkthdr.srcVlanId;
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
	if(m&&(m->m_pkthdr.tag.f.tpid == htons(ETH_P_8021Q))){
		pvlanId = ntohs(m->m_pkthdr.tag.f.pci&0xfff);
		//diag_printf("vlanId:%d,[%s]:[%d].\n",vlanId,__FUNCTION__,__LINE__);
	}	
	
#endif
	
#endif

	
	if ((sc->sc_if.if_flags & IFF_RUNNING) == 0) {
		m_freem(m);
		return;
	}

	src_if = m->m_pkthdr.rcvif;

        /* 
         * Pick out 802.1D packets.
         *                   */
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
#ifdef __ECOS
        if (m->m_flags & (M_BCAST | M_MCAST)) {
                if (bcmp (mtod(m,struct ether_header *), bstp_etheraddr, ETHER_ADDR_LEN) == 0) {
                        m_copydata(m, 0, sizeof(struct ether_header), (caddr_t)&eh);
                        m_adj (m, sizeof(struct ether_header));
                        m = bstp_input(sc, src_if, &eh, m);
                        if (m == NULL)
                                return;
                 }
        }
#endif   // __ECOS
#endif

#if	0//def NBPFILTER //> 0
	if (sc->sc_if.if_bpf)
		bpf_mtap(sc->sc_if.if_bpf, m);
#endif

	sc->sc_if.if_lastchange = time;
	sc->sc_if.if_ipackets++;
	sc->sc_if.if_ibytes += m->m_pkthdr.len;

#if defined(CONFIG_RTL_DNS_TRAP)
	recapped = br_dns_filter_enter(m);
#endif

	ifl = LIST_FIRST(&sc->sc_iflist);
	while (ifl != NULL && ifl->ifp != src_if) {
		ifl = LIST_NEXT(ifl, next);
	}
	if (ifl == NULL) {		
		m_freem(m);
		return;
	}
	
	
		
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        if ((ifl->bif_flags & IFBIF_STP) &&
            (ifl->bif_state == BSTP_IFSTATE_BLOCKING ||
            ifl->bif_state == BSTP_IFSTATE_LISTENING ||
            ifl->bif_state == BSTP_IFSTATE_DISABLED)) {
                m_freem(m);
                return;
        }
#endif

	if (m->m_pkthdr.len < sizeof(eh)) {
		m_freem(m);
		return;
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	eh = mtod(m, struct ether_header *);
	dst = (struct ether_addr *)&eh->ether_dhost[0];
	src = (struct ether_addr *)&eh->ether_shost[0];
#else	
	m_copydata(m, 0, sizeof(struct ether_header), (caddr_t)&eh);
	dst = (struct ether_addr *)&eh.ether_dhost[0];
	src = (struct ether_addr *)&eh.ether_shost[0];
#endif

	/*
	 * If interface is learning, and if source address
	 * is not broadcast or multicast, record it's address.
	 */
	if ((ifl->bif_flags & IFBIF_LEARNING) &&
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	    (eh->ether_shost[0] & 1) == 0 &&
	    !(eh->ether_shost[0] == 0 &&
	      eh->ether_shost[1] == 0 &&
	      eh->ether_shost[2] == 0 &&
	      eh->ether_shost[3] == 0 &&
	      eh->ether_shost[4] == 0 &&
	      eh->ether_shost[5] == 0))
#else
	    (eh.ether_shost[0] & 1) == 0 &&
	    !(eh.ether_shost[0] == 0 &&
	      eh.ether_shost[1] == 0 &&
	      eh.ether_shost[2] == 0 &&
	      eh.ether_shost[3] == 0 &&
	      eh.ether_shost[4] == 0 &&
	      eh.ether_shost[5] == 0))
#endif
		bridge_rtupdate(sc, src, src_if, 0, IFBAF_DYNAMIC);

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        if ((ifl->bif_flags & IFBIF_STP) &&
            (ifl->bif_state == BSTP_IFSTATE_LEARNING)) {
                m_freem(m);
                return;
        }
#endif
        /*
         * At this point, the port either does not participate in stp or
         * it is in forwarding state.
         */

	/*
	 * If packet is unicast, destined for someone on "this"
	 * side of the bridge, drop it.
	 */

	if ((m->m_flags & (M_BCAST | M_MCAST)) == 0) {

		dst_if = bridge_rtlookup(sc, dst);
        
		if (dst_if == src_if) {
		#if defined(CONFIG_RTL_DNS_TRAP)
			if (recapped == 0)
		#endif
			{
			m_freem(m);
			return;
			}
		}
	} else
		dst_if = NULL;

	/*
	 * Multicast packets get handled a little differently:
	 * If interface is:
	 *	-link0,-link1	(default) Forward all multicast
	 *			as broadcast.
	 *	-link0,link1	Drop non-IP multicast, forward
	 *			as broadcast IP multicast.
	 *	link0,-link1	Drop IP multicast, forward as
	 *			broadcast non-IP multicast.
	 *	link0,link1	Drop all multicast.
	 */
	if (m->m_flags & M_MCAST) {
		if ((sc->sc_if.if_flags &
		    (IFF_LINK0 | IFF_LINK1)) ==
		    (IFF_LINK0 | IFF_LINK1)) {
			m_freem(m);
			return;
		}
		if (sc->sc_if.if_flags & IFF_LINK0 &&
		    ETHERADDR_IS_IP_MCAST(dst)) {
			m_freem(m);
			return;
		}
		if (sc->sc_if.if_flags & IFF_LINK1 &&
		    !ETHERADDR_IS_IP_MCAST(dst)) {
			m_freem(m);
			return;
		}
	}

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	if (ifl->bif_flags & IFBIF_BLOCKNONIP && bridge_blocknonip(eh, m)) {
#else
	if (ifl->bif_flags & IFBIF_BLOCKNONIP && bridge_blocknonip(&eh, m)) {
#endif
		m_freem(m);
		return;
	}

	if (SIMPLEQ_FIRST(&ifl->bif_brlin) &&
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	    bridge_filterrule(SIMPLEQ_FIRST(&ifl->bif_brlin), eh) ==
#else
	    bridge_filterrule(SIMPLEQ_FIRST(&ifl->bif_brlin), &eh) ==
#endif
	    BRL_ACTION_BLOCK) {
		m_freem(m);
		return;
	}

#if defined(INET) && (defined(IPFILTER) || defined(IPFILTER_LKM))
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	m = bridge_filter(sc, src_if, eh, m);
#else
	m = bridge_filter(sc, src_if, &eh, m);
#endif
	if (m == NULL)
		return;
#endif

	/*
	 * If the packet is a multicast or broadcast OR if we don't
	 * know any better, forward it to all interfaces.
	 */
	if ((m->m_flags & (M_BCAST | M_MCAST)) || dst_if == NULL) {
		sc->sc_if.if_imcasts++;
		s = splimp();
#if defined (CONFIG_RTL_IGMP_SNOOPING)
		#ifdef DBG_IGMP_SNOOPING
		//diag_printf("dst=%x%x%x%x%x%x",dst->octet[0],dst->octet[1],dst->octet[2],dst->octet[3],dst->octet[4],dst->octet[5]);
		#endif
		if (IS_MCAST_ETHERADDR(dst) 
			#if defined(CONFIG_RTL_819X_SWCORE)
			&& igmpsnoopenabled
			#endif
			) {
			#ifdef DBG_IGMP_SNOOPING
			diag_printf("multicast,if:%s,portnum:%d[%s]:[%d].\n",src_if->if_name,src_if->if_index,__FUNCTION__,__LINE__);
			#endif
			struct ip *iph=NULL;
			u_int32_t fwdPortMask=0;

	
			unsigned char proto=0;
			unsigned char reserved=0;
			int ret=FAILED;
			
			unsigned char macAddr[6];
			unsigned char operation;
			char tmpOp;
			unsigned int gIndex=0;
			unsigned int moreFlag=1;
			unsigned int portNum=src_if->if_index;
			struct rtl_multicastDataInfo multicastDataInfo;
			struct rtl_multicastFwdInfo multicastFwdInfo;
			
			if (ETHERADDR_IS_IP_MCAST(dst) 
			&& ( ntohs(eh->ether_type) == ETH_P_IP))
			{
				
				if (m->m_pkthdr.len < (sizeof(struct ip)+sizeof(struct ether_header))){
					diag_printf("%s:%d pkt too short!\n", __FUNCTION__,__LINE__);
					goto no_snooping;
				}
				if (m->m_len < (sizeof (struct ip) +sizeof(struct ether_header))&&
				    (m = m_pullup(m, (sizeof (struct ip)+sizeof(struct ether_header)))) == 0) {
				    diag_printf("%s:%d pullup fail!\n", __FUNCTION__,__LINE__);
					return;
				}
				
#if 0
				
				int ii;
					
				diag_printf("m->m_len:%d,%d,[%s]:[%d]. \n",m->m_len,(sizeof(struct ether_header)+sizeof (struct ip)),__FUNCTION__,__LINE__);
				for(ii=0;ii<(m->m_len);ii++)
				{
					if(ii%16==0)
						diag_printf("\n");
					
					diag_printf("%02x  ", m->m_data[ii]);
				}
				diag_printf("\n");
#endif	
				iph =(struct ip*)( m->m_data+sizeof(struct ether_header));
				
#if defined(CONFIG_USB_UWIFI_HOST)
					if(ntohl(iph->ip_dst.s_addr) == 0xEFFFFFFA || ntohl(iph->ip_dst.s_addr) == 0xE1010101)
#else
					if(ntohl(iph->ip_dst.s_addr) == 0xEFFFFFFA)
#endif
				
				{
					/*for microsoft upnp*/
					reserved=1;
				}
#if 0
				if((iph->ip_dst.s_addr&0xFFFFFF00)==0xE0000000)
					reserved=1;
#endif
				proto =  iph->ip_p; 
				//diag_printf("proto:%d,[%s]:[%d].\n",proto,__FUNCTION__,__LINE__);
				if ((proto == IPPROTO_IGMP)&&(reserved ==0)) 
				{
					#ifdef DBG_IGMP_SNOOPING
					diag_printf("igmp[%s]:[%d].\n",__FUNCTION__,__LINE__);
					#endif
					struct mbuf *m_tmp=NULL;
					int k=0;
					m_tmp=m;
					igmpMldLen = 0;
					memset(igmpMldData, 0, 1500*sizeof(unsigned char));
					while(m_tmp && (igmpMldLen+m_tmp->m_len < 1500))
					{
						for(k=0; k<m_tmp->m_len; k++)
						{
							igmpMldData[igmpMldLen+k] = (unsigned char)m_tmp->m_data[k];
						}
						igmpMldLen += m_tmp->m_len;
						m_tmp = m_tmp->m_next;
					}
			#ifdef MCAST_TO_UNICAST
					while(moreFlag)
					{
						//tmpOp=igmp_type_check(m, macAddr, &gIndex, &moreFlag);
						tmpOp = igmp_type_check(igmpMldData, macAddr, &gIndex, &moreFlag);
						
						if(tmpOp>0)
						{
							//diag_printf("%s:%d,macAddr is 0x%x:%x:%x:%x:%x:%x,op:%d.\n",__FUNCTION__,__LINE__,macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5],tmpOp);
							operation=(unsigned char)tmpOp;
							//br_update_igmp_snoop_rt_entry(operation,sc, src_if, macAddr,m);
							br_update_igmp_snoop_rt_entry(operation,sc, src_if, macAddr,igmpMldData);
						}
					}
			#endif
					#if defined(CONFIG_RTL_819X_SWCORE)
					if(brIgmpModuleIndex!=0xFFFFFFFF)
					{
						#ifdef DBG_IGMP_SNOOPING
						diag_printf("br module igmp process[%s]:[%d].\n",__FUNCTION__,__LINE__);
						#endif
                        if(!((memcmp(src_if->if_name,"eth",3)==0)&&(src_if->if_unit==7)))
						    //rtl_igmpMldProcess(brIgmpModuleIndex,( u_int8_t *)((m)->m_data), portNum, &fwdPortMask);
						    rtl_igmpMldProcess(brIgmpModuleIndex,igmpMldData, portNum, &fwdPortMask);
						
					}
					#endif
					#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
					bridge_multicast_forward(sc, src_if, eh, fwdPortMask, m, 0);
					#else
					bridge_multicast_forward(sc, src_if, &eh, fwdPortMask, m, 0);
					#endif
					
				}
				else if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP)) && (reserved ==0))
				{
					
					multicastDataInfo.ipVersion=4;
					multicastDataInfo.sourceIp[0]=	(u_int32_t)(iph->ip_src.s_addr);
					multicastDataInfo.groupAddr[0]=  (u_int32_t)(iph->ip_dst.s_addr);

					multicastDataInfo.sourceIp[0]=	ntohl(multicastDataInfo.sourceIp[0]);
					multicastDataInfo.groupAddr[0]=  ntohl(multicastDataInfo.groupAddr[0]);

					
					#if defined(CONFIG_RTL_819X_SWCORE)
					if(brIgmpModuleIndex!=0xFFFFFFFF){
						ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
					}
					#endif
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)&& defined (CONFIG_RTL_IGMP_SNOOPING)
					rtl_refinePortMask(&multicastFwdInfo.fwdPortMask, sc, src_if,m);
#endif
					#ifdef DBG_IGMP_SNOOPING
					diag_printf("v4 udp fwdPortMask:%x.[%s]:[%d].\n",multicastFwdInfo.fwdPortMask,__FUNCTION__,__LINE__);
					#endif
					#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
					bridge_multicast_forward(sc, src_if, eh, multicastFwdInfo.fwdPortMask, m, 0);
					#else
					bridge_multicast_forward(sc, src_if, &eh, multicastFwdInfo.fwdPortMask, m, 0);
					#endif
					if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
					{
						#if defined  (CONFIG_RTL_HARDWARE_MULTICAST)
						if((srcVlanId!=0) && (srcPort!=0xFFFF))
						{
							#if defined(CONFIG_RTL_819X_SWCORE)	
							#if defined(CONFIG_RTL_VLAN_SUPPORT)
							#ifndef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
								if(rtl_vlan_support_enable == 0)
							#endif		
							
							{
								rtl865x_ipMulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0],pvlanId);
							}
							#else
							rtl865x_ipMulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0],pvlanId);
							#endif
							#endif
						}	
						#endif
					}
					
	
				}
				else
				{
					#ifdef DBG_IGMP_SNOOPING
					diag_printf("other protocol:%d.[%s]:[%d].\n",proto,__FUNCTION__,__LINE__);
					#endif
					#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
					bridge_broadcast(sc, src_if, eh, m);
					#else
					bridge_broadcast(sc, src_if, &eh, m);
					#endif
				}
			}
			else if(ETHERADDR_IS_IPv6_MCAST(dst)
				&& ( ntohs(eh->ether_type) == ETH_P_IPV6))
			{
				struct ip6_hdr *ipv6h=NULL;

				/* Copy minimal header, and drop invalids */
				if (m->m_pkthdr.len < (sizeof(struct ip6_hdr)+sizeof(struct ether_header))){
					diag_printf("%s:%d pkt too short!\n", __FUNCTION__,__LINE__);
					goto no_snooping;
				}
				if (m->m_len < (sizeof(struct ip6_hdr)+sizeof(struct ether_header)) &&
				    (m = m_pullup(m, (sizeof(struct ip6_hdr)+sizeof(struct ether_header)))) == NULL)
					return (NULL);
				
				ipv6h =(struct ip6_hdr*)( m->m_data+sizeof(struct ether_header));
	
#if defined (CONFIG_RTL_MLD_SNOOPING)
				if(rtl_isHopbyHop(ipv6h) &&
				    m->m_len < (sizeof(struct ip6_hdr)+sizeof(struct ether_header)+sizeof(struct ip6_hbh)) &&
				   (m = m_pullup(m, (sizeof(struct ip6_hdr)+sizeof(struct ether_header)+sizeof(struct ip6_hbh)))) == NULL)
				   return NULL;

				if(mldSnoopEnabled)
				{
									
					proto =  re865x_getIpv6TransportProtocol(ipv6h);
					/*icmp protocol*/
					
					if (proto == IPPROTO_ICMPV6) 
					{	
						struct mbuf *m_tmp=NULL;
						int k=0;
						m_tmp=m;
						igmpMldLen = 0;
						memset(igmpMldData, 0, 1500*sizeof(unsigned char));
						while(m_tmp && (igmpMldLen+m_tmp->m_len < 1500))
						{
							for(k=0; k<m_tmp->m_len; k++)
							{
								igmpMldData[igmpMldLen+k] = (unsigned char)m_tmp->m_data[k];
							}
							igmpMldLen += m_tmp->m_len;
							m_tmp = m_tmp->m_next;
						}
				
#if defined (IPV6_MCAST_TO_UNICAST)
						//tmpOp=ICMPv6_check(m , macAddr);
						tmpOp=ICMPv6_check(igmpMldData , macAddr);
						if(tmpOp > 0){
							operation=(unsigned char)tmpOp;
#ifdef	DBG_ICMPv6
						if( operation == 1)
							diag_printf("icmpv6 add from frame finish\n");
						else if(operation == 2)
							diag_printf("icmpv6 del from frame finish\n");	
#endif
							//br_update_igmp_snoop_rt_entry(operation,sc, src_if, macAddr,m);
							br_update_igmp_snoop_rt_entry(operation,sc, src_if, macAddr,igmpMldData);
						}
#endif
						#ifdef DBG_IGMP_SNOOPING
						diag_printf("br module ICMPV6 process[%s]:[%d].\n",__FUNCTION__,__LINE__);
						#endif
						#if defined(CONFIG_RTL_819X_SWCORE)
						//rtl_igmpMldProcess(brIgmpModuleIndex,( u_int8_t *)((m)->m_data), portNum, &fwdPortMask);
						rtl_igmpMldProcess(brIgmpModuleIndex,igmpMldData, portNum, &fwdPortMask);
						#endif
						#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
						bridge_multicast_forward(sc, src_if, eh, fwdPortMask, m, 0);
						#else
						bridge_multicast_forward(sc, src_if, &eh, fwdPortMask, m, 0);
						#endif
					}
					else if ((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
					{
						multicastDataInfo.ipVersion=6;
						memcpy(&multicastDataInfo.sourceIp, &ipv6h->ip6_src, sizeof(struct in6_addr));
						memcpy(&multicastDataInfo.groupAddr,&ipv6h->ip6_dst, sizeof(struct in6_addr));

						multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
						multicastDataInfo.sourceIp[1] = ntohl(multicastDataInfo.sourceIp[1]);
						multicastDataInfo.sourceIp[2] = ntohl(multicastDataInfo.sourceIp[2]);
						multicastDataInfo.sourceIp[3] = ntohl(multicastDataInfo.sourceIp[3]);
						multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
						multicastDataInfo.groupAddr[1] = ntohl(multicastDataInfo.groupAddr[1]);
						multicastDataInfo.groupAddr[2] = ntohl(multicastDataInfo.groupAddr[2]);
						multicastDataInfo.groupAddr[3] = ntohl(multicastDataInfo.groupAddr[3]);
						
						#if defined(CONFIG_RTL_819X_SWCORE)
						ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
						#endif
						#ifdef DBG_IGMP_SNOOPING
						
						diag_printf("v6 UDP ipv6h->ip6_dst:%x-%x-%x-%x,fwdPortMask:%x[%s]:[%d].\n",
						ipv6h->ip6_dst.s6_addr32[0],ipv6h->ip6_dst.s6_addr32[1],ipv6h->ip6_dst.s6_addr32[2],ipv6h->ip6_dst.s6_addr32[3],multicastFwdInfo.fwdPortMask,__FUNCTION__,__LINE__);
						#endif
						#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
						bridge_multicast_forward(sc, src_if, eh, multicastFwdInfo.fwdPortMask, m, 0);
						#else
						bridge_multicast_forward(sc, src_if, &eh, multicastFwdInfo.fwdPortMask, m, 0);
						#endif
						#if defined (CONFIG_RTL_HARDWARE_MULTICAST) && defined(CONFIG_RTL_8197F)
						if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
						{
							if((srcVlanId!=0) && (srcPort!=0xFFFF))
							{
							#if defined(CONFIG_RTL_819X_SWCORE) 
							#if defined(CONFIG_RTL_VLAN_SUPPORT)
							#ifndef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
								if(rtl_vlan_support_enable == 0)
							#endif		
								{
									rtl819x_ipv6MulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp, multicastDataInfo.groupAddr,pvlanId);
								}
							#else
								rtl819x_ipv6MulticastHardwareAccelerate(br_sc, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp, multicastDataInfo.groupAddr,pvlanId);
							#endif
							#endif
							}
						}
#endif		

					}
					else
					{
						#ifdef DBG_IGMP_SNOOPING
						diag_printf("other proto:%d.[%s]:[%d].\n",proto,__FUNCTION__,__LINE__);
						#endif
						#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
						bridge_broadcast(sc, src_if, eh, m);
						#else
						bridge_broadcast(sc, src_if, &eh, m);
						#endif
					}	
				}
				else
#endif				
				{
					#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
					bridge_broadcast(sc, src_if, eh, m);
					#else
					bridge_broadcast(sc, src_if, &eh, m);
					#endif
				}
	
			}
			else
			{

no_snooping:			
				#ifdef DBG_IGMP_SNOOPING
				diag_printf("broad cast if:%s,portnum:%d[%s]:[%d].\n",src_if->if_name,src_if->if_index,__FUNCTION__,__LINE__);
				#endif
				
				#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
				bridge_broadcast(sc, src_if, eh, m);
				#else
				bridge_broadcast(sc, src_if, &eh, m);
				#endif
			}
	
		
		}
		else
		{
			
			#ifdef DBG_IGMP_SNOOPING
			diag_printf("broad cast if:%s,portnum:%d[%s]:[%d].\n",src_if->if_name,src_if->if_index,__FUNCTION__,__LINE__);
			#endif
			
			#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
			bridge_broadcast(sc, src_if, eh, m);
			#else
			bridge_broadcast(sc, src_if, &eh, m);
			#endif
		}	
#else
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
		bridge_broadcast(sc, src_if, eh, m);
#else
		bridge_broadcast(sc, src_if, &eh, m);
#endif

#endif	// CONFIG_RTL_IGMP_SNOOPING
		splx(s);
		return;
	}

	/*
	 * At this point, we're dealing with a unicast frame going to a
	 * different interface
	 */
	if ((dst_if->if_flags & IFF_RUNNING) == 0) {
		m_freem(m);
		return;
	}
	ifl = LIST_FIRST(&sc->sc_iflist);
	while (ifl != NULL && ifl->ifp != dst_if)
		ifl = LIST_NEXT(ifl, next);
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        if ((ifl->bif_flags & IFBIF_STP) &&
            (ifl->bif_state == BSTP_IFSTATE_DISABLED ||
            ifl->bif_state == BSTP_IFSTATE_BLOCKING)) {
                m_freem(m);
                return;
        }
#endif
	if(NULL == ifl)	return;
	if (SIMPLEQ_FIRST(&ifl->bif_brlout) &&
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	    bridge_filterrule(SIMPLEQ_FIRST(&ifl->bif_brlout), eh) ==
#else
	    bridge_filterrule(SIMPLEQ_FIRST(&ifl->bif_brlout), &eh) ==
#endif
	    BRL_ACTION_BLOCK) {
		m_freem(m);
		return;
	}

    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    if (should_deliver(dst_if, m) == 0)
        return;
    #endif

	s = splimp();
	if (IF_QFULL(&dst_if->if_snd)) {
		sc->sc_if.if_oerrors++;
		m_freem(m);
		splx(s);
		//diag_printf("IF_QFULL2\n");
		return;
	}
	sc->sc_if.if_opackets++;
	sc->sc_if.if_obytes += m->m_pkthdr.len;
    // Also count the bytes in the outgoing interface; normally
    // done in if_ethersubr.c but here we bypass that route.
    dst_if->if_obytes += m->m_pkthdr.len;
	
	IF_ENQUEUE(&dst_if->if_snd, m);
	if ((dst_if->if_flags & IFF_OACTIVE) == 0)
		(*dst_if->if_start)(dst_if);
	splx(s);
}

#ifdef __CONFIG_APCLIENT_DHCPC__
/*lqz add for apclient www.tendawifi.com dns redirct*/
inline static int apclient_dns_redirct(struct mbuf *m)
{
	/*此时m已经去掉头部了*/
	struct mbuf *mm = m;
	struct ip *ip;
	struct udphdr *udp;
	int iplen = 0;
	DNSHEADER *dnsheader;
	unsigned int qlen = 0;

	if(NULL == mm)
	{
		return 0;
	}

	if(1 == gpi_apclient_dhcpc_enable())
	{
		ip = mtod(mm, struct ip *);
		iplen = sizeof(struct ip);
		
		if(ip->ip_p == IPPROTO_UDP && mm->m_len > iplen + sizeof(struct udphdr)){
			udp = (struct udphdr *)((char *)ip + iplen);
			if (udp->uh_dport == ntohs(53))
			{
				dnsheader = (DNSHEADER *)((char *)udp + sizeof(struct udphdr));			
				if(udp->uh_ulen > sizeof(struct udphdr))
				{
					char dname[256]={0};
					unsigned int request = 0;
					qlen = udp->uh_ulen - sizeof(struct udphdr);	

					request = dnsmasq_parse_request(dnsheader,qlen, dname);

				#ifdef __CONFIG_A9__
					if(strcmp(dname,"re.tenda.cn") == 0)
				#else
					if(strcmp(dname,"tendawifi.com") == 0 || strcmp(dname,"www.tendawifi.com") == 0)
				#endif
					{
						if(request == 4)/*如果是IPV6报文的话，且是tendawifi.com的话，就返回，不往上级传*/
						{
							m_freem(m);
							return 1;
						}
						else if(request != 0)
						{
							printf("name=[%s]\n",dname);
#if defined(__CONFIG_A9__)&&defined(__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__)
							dns_redirect_pkt(mm,DNS_REQUEST_TTL);
#else
							dns_redirect_pkt(mm);
#endif
							return 1;
						}
					}
				}
			}
		}
	}
	return 0;
}
/*end*/
#endif	

/*
 * Receive input from an interface.  Queue the packet for bridging if its
 * not for us, and schedule an interrupt.
 */
struct mbuf *
bridge_input(
	struct ifnet *ifp,
	struct ether_header *eh,
	struct mbuf *m
)
{
	struct bridge_softc *sc;
	int s;
	struct bridge_iflist *ifl;
	struct arpcom *ac;
	struct mbuf *mc;
	//printf("bridge_input ifname:%s%d\n",ifp->if_name,ifp->if_unit);
	/*
	 * Make sure this interface is a bridge member.
	 */
	if (ifp == NULL || ifp->if_bridge == NULL || m == NULL)
		return (m);

	if ((m->m_flags & M_PKTHDR) == 0)
		panic("bridge_input(): no HDR");

#ifdef __CONFIG_APCLIENT_DHCPC__
	if(eh && ntohs(eh->ether_type) == 0x0800)
	{
		if(1 == apclient_dns_redirct(m))
		{
			return NULL;
		}
	}
#endif	
	
	sc = (struct bridge_softc *)ifp->if_bridge;
	if ((sc->sc_if.if_flags & IFF_RUNNING) == 0)
		return (m);

        LIST_FOREACH (ifl, &sc->sc_iflist, next) {
                if (ifl->ifp == ifp)
                        break;
        }
        if (ifl == LIST_END (&sc->sc_iflist))
                return (m);

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        bridge_span(sc, eh, m);
        /*
         * Tap off 802.1D packets, they do not get forwarded 
         */
        if (m->m_flags & (M_BCAST | M_MCAST)) {
                if (bcmp(eh->ether_dhost, bstp_etheraddr, ETHER_ADDR_LEN) == 0) {
#ifdef __ECOS
                        M_PREPEND(m, sizeof (struct ether_header), M_DONTWAIT);
                        if (m == NULL)
                                return (NULL);
                        bcopy(eh, mtod(m, caddr_t), sizeof(struct ether_header));

                        s = splimp ();
                        if (IF_QFULL(&sc->sc_if.if_snd)) {
                                m_freem (m);
                                splx (s);
                                return (NULL);
                        }
                        IF_ENQUEUE(&sc->sc_if.if_snd, m);
                        splx (s);
                        schednetisr (NETISR_BRIDGE);
                        return (NULL);
#else
                        m = bstp_input(sc, ifp, eh, m);
                        if (m == NULL)
                                return (NULL);
#endif
                }
        }
#endif
	//printf("m->m_flags:%x\n",m->m_flags);
	if (m->m_flags & (M_BCAST | M_MCAST)) {
		/*
		 * make a copy of 'm' with 'eh' tacked on to the
		 * beginning.  Return 'm' for local processing
		 * and enqueue the copy.  Schedule netisr.
		 */
		 
#if 0//def RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
		mc = m_copym3(m, 0, M_COPYALL, M_NOWAIT);
#else		
		mc = m_copym2(m, 0, M_COPYALL, M_NOWAIT);
#endif
		//mc = m_copym(m, 0, M_COPYALL, M_NOWAIT);
		if (mc == NULL)
			return (m);
		
#if 0      //def RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
		(mc)->m_data -= sizeof(struct ether_header);
		(mc)->m_len += sizeof(struct ether_header);
		(mc)->m_pkthdr.len += sizeof(struct ether_header);
		/*if using m_copym3, need to copy the eth header*/
		bcopy(eh, mtod(mc, caddr_t), sizeof(struct ether_header));
#else
		M_PREPEND(mc, sizeof(struct ether_header), M_DONTWAIT);
		if (mc == NULL)
			return (m);
		bcopy(eh, mtod(mc, caddr_t), sizeof(struct ether_header));
#endif
		s = splimp();
		if (IF_QFULL(&sc->sc_if.if_snd)) {
			m_freem(mc);
			splx(s);
			return (m);
		}
		IF_ENQUEUE(&sc->sc_if.if_snd, mc);
		splx(s);
		schednetisr(NETISR_BRIDGE);
		return (m);
	}
	
        /*
         * No need to queue frames for ifs in blocking, disabled or listening state
         */
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
        if ((ifl->bif_flags & IFBIF_STP) &&
            ((ifl->bif_state == BSTP_IFSTATE_BLOCKING) ||
             (ifl->bif_state == BSTP_IFSTATE_LISTENING) ||
             (ifl->bif_state == BSTP_IFSTATE_DISABLED)))
                 return (m);
#endif
	
	/*
	 * Unicast, make sure it's not for us.
	 */
	for (ifl = LIST_FIRST(&sc->sc_iflist);ifl; ifl = LIST_NEXT(ifl,next)) {
		if(!ifl->ifp)
			continue;
		//printf("ifname:%s%d ifindex:%d\n",ifl->ifp->if_name,ifl->ifp->if_unit,ifl->ifp->if_index);
		if (ifl->ifp->if_type != IFT_ETHER)
			continue;
		
		ac = (struct arpcom *)ifl->ifp;
		if (bcmp(ac->ac_enaddr, eh->ether_dhost, ETHER_ADDR_LEN) == 0) {			
#ifdef CONFIG_SAME_LAN_MAC
#if 0 
	//lq 目前使用的mac复用		/*skip wlan0 for wlan0 and eth0 has same mac*/
			if(bcmp(ifl->ifp->if_name,"wlan",4)==0
					&& ifl->ifp->if_unit==0)
				continue;
#endif	
#if defined(CONFIG_RTL_VLAN_SUPPORT)&& defined (CONFIG_RTL_BRIDGE_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
			if(rtl_vlan_support_enable)
			{
				if((bcmp(ifl->ifp->if_name,"eth",3)==0&& ifl->ifp->if_unit==2)
					||(bcmp(ifl->ifp->if_name,"eth",3)==0&& ifl->ifp->if_unit==3)
					||(bcmp(ifl->ifp->if_name,"eth",3)==0&& ifl->ifp->if_unit==4)
					||(bcmp(ifl->ifp->if_name,"eth",3)==0&& ifl->ifp->if_unit==7))			
					continue;
			}
#endif
#endif
			if (ifl->bif_flags & IFBIF_LEARNING)
				bridge_rtupdate(sc,
				    (struct ether_addr *)&eh->ether_shost,
				    ifp, 0, IFBAF_DYNAMIC);
			m->m_pkthdr.rcvif = ifl->ifp;
			return (m);
		}
		if (bcmp(ac->ac_enaddr, eh->ether_shost, ETHER_ADDR_LEN) == 0) {
			m_freem(m);
			return (NULL);
		}
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	(m)->m_data -= sizeof(struct ether_header);
	(m)->m_len += sizeof(struct ether_header);
	(m)->m_pkthdr.len += sizeof(struct ether_header);
#else
	M_PREPEND(m, sizeof(struct ether_header), M_DONTWAIT);
	if (m == NULL)
		return (NULL);
	bcopy(eh, mtod(m, caddr_t), sizeof(struct ether_header));
#endif
	
	s = splimp();
	if (IF_QFULL(&sc->sc_if.if_snd)) {
		m_freem(m);
		splx(s);
		//diag_printf("IF_QFULL\n");
		return (NULL);
	}
	
	IF_ENQUEUE(&sc->sc_if.if_snd, m);
	splx(s);
	schednetisr(NETISR_BRIDGE);
	return (NULL);
}
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT) && defined (CONFIG_RTL_IGMP_SNOOPING)
extern int32 rtl_getIpv4UnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap);
void rtl_refinePortMask(unsigned int *FwdPortMask, struct bridge_softc *sc, struct ifnet *ifp, struct mbuf * m)
{
	struct bridge_iflist *p;
	
	unsigned int forwarding_rule_dst = 0, forwarding_rule_org = 0;
	unsigned char ethname[5] = {0}; 
	unsigned int vlanFwdPortMask = 0;
	unsigned int oriFwdPortMask = 0;
	unsigned int unknowFwdPortMask = 0;

	if(FwdPortMask==NULL || sc==NULL || m==NULL || ifp==NULL)
		return;
#if 1
	sprintf(ethname,"%s%d",ifp->if_name, ifp->if_unit);
	if(strcmp(ethname, "eth7")
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
		&&strcmp(ethname, "peth0")
#endif
	)
		return;
#endif
	
	oriFwdPortMask = *FwdPortMask;
	rtl_getIpv4UnknownMCastFloodMap(brIgmpModuleIndex,&unknowFwdPortMask);
	
	if(oriFwdPortMask==0 || oriFwdPortMask==unknowFwdPortMask)
		return;

	forwarding_rule_org = m->m_pkthdr.forward_rule;
	if(forwarding_rule_org==0)
		return;
		
	for (p = LIST_FIRST(&sc->sc_iflist); p; p = LIST_NEXT(p, next)) {

		if (p->ifp->if_index == ifp->if_index)
			continue;

		sprintf(ethname,"%s%d",p->ifp->if_name, p->ifp->if_unit);
		unsigned short dstvid = rtl_getVlanPortPvidByDevName(ethname);
		rtl_getRtlVlanForwardRule(dstvid ,&forwarding_rule_dst);

		if(forwarding_rule_dst == forwarding_rule_org)
		 	vlanFwdPortMask |= (1 << p->ifp->if_index);
	}

	//printf("vlanFwdPortMask = %x, oriFwdPortMask = %x, unkown = %x\n", vlanFwdPortMask, oriFwdPortMask, unknowFwdPortMask);
	if((vlanFwdPortMask & oriFwdPortMask)==0)
		*FwdPortMask = unknowFwdPortMask;
}
#endif
void bridge_multicast_forward(struct bridge_softc *sc,struct ifnet *ifp,struct ether_header *eh, u_int32_t fwdPortMask, struct mbuf * m, int clone)
{
	struct bridge_iflist *p;
	struct mbuf *mc;
	int used = 0;
	unsigned int port_bitmask=0;
	#if 0
	if (clone) 
	{
		struct mbuf *mc;

		mc = m_copym(m, 0, M_COPYALL, M_NOWAIT);
		if (mc == NULL)
			return (m);

	}
	#endif
	//diag_printf("ifp:%s,%d.[%s]:[%d].\n",ifp->if_name,ifp->if_index,__FUNCTION__,__LINE__);
	for (p = LIST_FIRST(&sc->sc_iflist); p; p = LIST_NEXT(p, next)) {
		/*
		 * Don't retransmit out of the same interface where
		 * the packet was received from.
		 */
		if (p->ifp->if_index == ifp->if_index)
			continue;
        
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        if (should_deliver(p->ifp, m) == 0)
            continue;
        #endif
		
		port_bitmask = (1 << p->ifp->if_index);
		if(!(port_bitmask&fwdPortMask))
			continue;
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
				if ((p->bif_flags & IFBIF_STP) &&
					(p->bif_state != BSTP_IFSTATE_FORWARDING))
						continue;
#endif

		if ((p->bif_flags & IFBIF_DISCOVER) == 0 &&
			(m->m_flags & (M_BCAST | M_MCAST)) == 0)
			continue;

		if ((p->ifp->if_flags & IFF_RUNNING) == 0)
			continue;

		if (IF_QFULL(&p->ifp->if_snd)) {
			sc->sc_if.if_oerrors++;
			continue;
		}

		if (SIMPLEQ_FIRST(&p->bif_brlout) &&
			bridge_filterrule(SIMPLEQ_FIRST(&p->bif_brlout), eh) ==
			BRL_ACTION_BLOCK)
			continue;

		/* If last one, reuse the passed-in mbuf */
		if (LIST_NEXT(p, next) == NULL) {
			
			mc = m;
			used = 1;
		} else {
			
			mc = m_copym(m, 0, M_COPYALL, M_DONTWAIT);
			if (mc == NULL) {
				sc->sc_if.if_oerrors++;
				continue;
			}
		}

		if (p->bif_flags & IFBIF_BLOCKNONIP &&
			bridge_blocknonip(eh, mc)) {
			m_freem(mc);
			continue;
		}
		
        //diag_printf("ifp:%s,%d.[%s]:[%d].\n",p->ifp->if_name,p->ifp->if_unit,__FUNCTION__,__LINE__);
		sc->sc_if.if_opackets++;
		sc->sc_if.if_obytes += mc->m_pkthdr.len;
		if (ifp && ((eh->ether_shost[0] & 1) == 0) )
			ifp->if_omcasts++;
				// Also count the bytes in the outgoing interface; normally
				// done in if_ethersubr.c but here we bypass that route.
				p->ifp->if_obytes += m->m_pkthdr.len;
		IF_ENQUEUE(&p->ifp->if_snd, mc);
		if ((p->ifp->if_flags & IFF_OACTIVE) == 0)
			(*p->ifp->if_start)(p->ifp);
		
	}

	if (!used)
		m_freem(m);

}



/*
 * Send a frame to all interfaces that are members of the bridge
 * (except the one it came in on).  This code assumes that it is
 * running at splnet or higher.
 */
void
bridge_broadcast(sc, ifp, eh, m)
	struct bridge_softc *sc;
	struct ifnet *ifp;
	struct ether_header *eh;
	struct mbuf *m;
{
	struct bridge_iflist *p;
	struct mbuf *mc;
	int used = 0;

	for (p = LIST_FIRST(&sc->sc_iflist); p; p = LIST_NEXT(p, next)) {

		/*
		 * Don't retransmit out of the same interface where
		 * the packet was received from.
		 */
		if (p->ifp->if_index == ifp->if_index)
			continue;

        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        if (should_deliver(p->ifp, m) == 0)
        {
            //diag_printf("%s %d should_deliver 0\n", __FUNCTION__, __LINE__);
            continue;
        }
        #endif

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
                if ((p->bif_flags & IFBIF_STP) &&
                    (p->bif_state != BSTP_IFSTATE_FORWARDING))
                        continue;
#endif

		if ((p->bif_flags & IFBIF_DISCOVER) == 0 &&
		    (m->m_flags & (M_BCAST | M_MCAST)) == 0)
			continue;

		if ((p->ifp->if_flags & IFF_RUNNING) == 0)
			continue;

		if (IF_QFULL(&p->ifp->if_snd)) {
			sc->sc_if.if_oerrors++;
			continue;
		}

		if (SIMPLEQ_FIRST(&p->bif_brlout) &&
		    bridge_filterrule(SIMPLEQ_FIRST(&p->bif_brlout), eh) ==
		    BRL_ACTION_BLOCK)
			continue;

		/* If last one, reuse the passed-in mbuf */
		if (LIST_NEXT(p, next) == NULL) {
			mc = m;
			used = 1;
		} else {
			mc = m_copym(m, 0, M_COPYALL, M_DONTWAIT);
			if (mc == NULL) {
				sc->sc_if.if_oerrors++;
				continue;
			}
		}

		if (p->bif_flags & IFBIF_BLOCKNONIP &&
		    bridge_blocknonip(eh, mc)) {
			m_freem(mc);
			continue;
		}

		sc->sc_if.if_opackets++;
		sc->sc_if.if_obytes += mc->m_pkthdr.len;
		if (ifp && ((eh->ether_shost[0] & 1) == 0) )
			ifp->if_omcasts++;
                // Also count the bytes in the outgoing interface; normally
                // done in if_ethersubr.c but here we bypass that route.
                p->ifp->if_obytes += m->m_pkthdr.len;
		IF_ENQUEUE(&p->ifp->if_snd, mc);
		if ((p->ifp->if_flags & IFF_OACTIVE) == 0)
			(*p->ifp->if_start)(p->ifp);
	}

	if (!used)
		m_freem(m);
}

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
void
bridge_span(sc, eh, morig)
        struct bridge_softc *sc;
        struct ether_header *eh;
        struct mbuf *morig;
{
        struct bridge_iflist *p;
        struct ifnet *ifp;
        struct mbuf *mc, *m;
        int error;

        if (LIST_EMPTY(&sc->sc_spanlist))
                return;

        m = m_copym2(morig, 0, M_COPYALL, M_NOWAIT);
        if (m == NULL)
                return;
        if (eh != NULL) {
                M_PREPEND(m, sizeof(struct ether_header), M_DONTWAIT);
                if (m == NULL)
                        return;
                bcopy(eh, mtod(m, caddr_t), sizeof(struct ether_header));
        }

        LIST_FOREACH(p, &sc->sc_spanlist, next) {
                ifp = p->ifp;

                if ((ifp->if_flags & IFF_RUNNING) == 0)
                        continue;

#ifdef ALTQ
                if (ALTQ_IS_ENABLED(&ifp->if_snd) == 0)
#endif
                        if (IF_QFULL(&ifp->if_snd)) {
                                IF_DROP(&ifp->if_snd);
                                sc->sc_if.if_oerrors++;
                                continue;
                        }

                mc = m_copym(m, 0, M_COPYALL, M_DONTWAIT);
                if (mc == NULL) {
                        sc->sc_if.if_oerrors++;
                        continue;
                }

                error = bridge_ifenqueue(sc, ifp, m);
                if (error)
                        continue;
        }
        m_freem(m);
}
#endif

struct ifnet *
bridge_rtupdate(sc, ea, ifp, setflags, flags)
	struct bridge_softc *sc;
	struct ether_addr *ea;
	struct ifnet *ifp;
	int setflags;
	u_int8_t flags;
{
	struct bridge_rtnode *p, *q;
	u_int32_t h;
	int s, dir;

	s = splhigh();
	if (sc->sc_rts == NULL) {
		if (setflags && flags == IFBAF_STATIC) {
			sc->sc_rts = (struct bridge_rthead *)malloc(
			    BRIDGE_RTABLE_SIZE *
			    (sizeof(struct bridge_rthead)),M_DEVBUF,M_NOWAIT);

			if (sc->sc_rts == NULL)
				goto done;

			for (h = 0; h < BRIDGE_RTABLE_SIZE; h++)
				LIST_INIT(&sc->sc_rts[h]);
		} else
			goto done;
	}

	h = bridge_hash(ea);
	p = LIST_FIRST(&sc->sc_rts[h]);
	if (p == NULL) {
		if (sc->sc_brtcnt >= sc->sc_brtmax)
			goto done;
		p = (struct bridge_rtnode *)malloc(
		    sizeof(struct bridge_rtnode), M_DEVBUF, M_NOWAIT);
		if (p == NULL)
			goto done;
		
		memset(p, 0,  sizeof(struct bridge_rtnode));
		bcopy(ea, &p->brt_addr, sizeof(p->brt_addr));
		p->brt_if = ifp;
		p->brt_age = 1;
#if defined (CONFIG_RTL_IGMP_SNOOPING)
		int i;
		p->group_src = 0;
		p->igmpFlag=0;
		for(i=0 ; i<IGMP_EXT_NUM ;i++)
		{
			p->igmp_ext_arr[i].valid = 0;
			p->portUsedNum[i] = 0;		
		}
#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
 		rtl865x_addFDBEntry(ea->octet);
#endif
		if (setflags)
			p->brt_flags = flags;
		else
			p->brt_flags = IFBAF_DYNAMIC;

		LIST_INSERT_HEAD(&sc->sc_rts[h], p, brt_next);
		sc->sc_brtcnt++;
		goto want;
	}

	do {
		q = p;
		p = LIST_NEXT(p, brt_next);

		dir = memcmp(ea, &q->brt_addr, sizeof(q->brt_addr));
		if (dir == 0) {
			#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
			 if((q->brt_if != ifp)&&ifp->if_name)
			 	update_hw_l2table(ifp->if_name, ea->octet);
			#endif

			/*fix:wlan sta can not access DUT when change from root to virtual ap*/
			#if defined (CONFIG_RTL_819X)
			if(((q->brt_if !=ifp) && (q->brt_flags==0)) ||(setflags))
			#else
			if (setflags) 
			#endif
			{
				q->brt_if = ifp;
				q->brt_flags = flags;
			}

			if (q->brt_if == ifp)
				q->brt_age = 1;
			ifp = q->brt_if;
			goto want;
		}

		if (dir > 0) {
			if (sc->sc_brtcnt >= sc->sc_brtmax)
				goto done;
			p = (struct bridge_rtnode *)malloc(
			    sizeof(struct bridge_rtnode), M_DEVBUF, M_NOWAIT);
			if (p == NULL)
				goto done;
			
			memset(p, 0,  sizeof(struct bridge_rtnode));
			bcopy(ea, &p->brt_addr, sizeof(p->brt_addr));
			p->brt_if = ifp;
			p->brt_age = 1;
			
#if defined (CONFIG_RTL_IGMP_SNOOPING)
			int i;
			p->group_src = 0;
			p->igmpFlag=0;
			for(i=0 ; i<IGMP_EXT_NUM ;i++)
			{
				p->igmp_ext_arr[i].valid = 0;
				p->portUsedNum[i] = 0;		
			}
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
 			rtl865x_addFDBEntry(ea->octet);
#endif
			if (setflags)
				p->brt_flags = flags;
			else
				p->brt_flags = IFBAF_DYNAMIC;

			LIST_INSERT_BEFORE(q, p, brt_next);
			sc->sc_brtcnt++;
			goto want;
		}

		if (p == NULL) {
			if (sc->sc_brtcnt >= sc->sc_brtmax)
				goto done;
			p = (struct bridge_rtnode *)malloc(
			    sizeof(struct bridge_rtnode), M_DEVBUF, M_NOWAIT);
			if (p == NULL)
				goto done;

			memset(p, 0,  sizeof(struct bridge_rtnode));
			bcopy(ea, &p->brt_addr, sizeof(p->brt_addr));
			p->brt_if = ifp;
			p->brt_age = 1;
#if defined (CONFIG_RTL_IGMP_SNOOPING)
			int i;
			p->group_src = 0;
			p->igmpFlag=0;
			for(i=0 ; i<IGMP_EXT_NUM ;i++)
			{
				p->igmp_ext_arr[i].valid = 0;
				p->portUsedNum[i] = 0;		
			}
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
 			rtl865x_addFDBEntry(ea->octet);
#endif
			if (setflags)
				p->brt_flags = flags;
			else
				p->brt_flags = IFBAF_DYNAMIC;
			LIST_INSERT_AFTER(q, p, brt_next);
			sc->sc_brtcnt++;
			goto want;
		}
	} while (p != NULL);

done:
	ifp = NULL;
want:
	splx(s);
	return (ifp);
}

struct ifnet *
bridge_rtlookup(sc, ea)
	struct bridge_softc *sc;
	struct ether_addr *ea;
{
	struct bridge_rtnode *p;
	u_int32_t h;
	int s, dir;

	/*
	 * Lock out everything else
	 */
	s = splhigh();

	if (sc->sc_rts == NULL)
		goto fail;

	h = bridge_hash(ea);
	p = LIST_FIRST(&sc->sc_rts[h]);
	while (p != NULL) {
		dir = memcmp(ea, &p->brt_addr, sizeof(p->brt_addr));
		if (dir == 0) {
			splx(s);
			return (p->brt_if);
		}
		if (dir > 0)
			goto fail;
		p = LIST_NEXT(p, brt_next);
	}
fail:
	splx(s);
	return (NULL);
}
	
struct bridge_rtnode *
bridge_rtget(sc, ea)
	struct bridge_softc *sc;
	struct ether_addr *ea;
{
	struct bridge_rtnode *p;
	u_int32_t h;
	int s, dir;

	/*
	 * Lock out everything else
	 */
	s = splhigh();

	if (sc->sc_rts == NULL)
		goto fail;

	h = bridge_hash(ea);
	p = LIST_FIRST(&sc->sc_rts[h]);
	while (p != NULL) {
		dir = memcmp(ea, &p->brt_addr, sizeof(p->brt_addr));
		if (dir == 0) {
			splx(s);
			return p;
		}
		if (dir > 0)
			goto fail;
		p = LIST_NEXT(p, brt_next);
	}
fail:
	splx(s);
	return (NULL);
}

#if 0
struct bridge_rtnode *
bridge_rtinsert(sc, ea, ifp, setflags, flags)
	struct bridge_softc *sc;
	struct ether_addr *ea;
	struct ifnet *ifp;
	int setflags;
	u_int8_t flags;
{
	struct bridge_rtnode *p, *q;
	u_int32_t h;
	int s, dir;

	s = splhigh();
	if (sc->sc_rts == NULL) {
		if (setflags && flags == IFBAF_STATIC) {
			sc->sc_rts = (struct bridge_rthead *)malloc(
			    BRIDGE_RTABLE_SIZE *
			    (sizeof(struct bridge_rthead)),M_DEVBUF,M_NOWAIT);

			if (sc->sc_rts == NULL)
				goto done;

			for (h = 0; h < BRIDGE_RTABLE_SIZE; h++)
				LIST_INIT(&sc->sc_rts[h]);
		} else
			goto done;
	}

	h = bridge_hash(ea);
	p = LIST_FIRST(&sc->sc_rts[h]);
	if (p == NULL) {
		if (sc->sc_brtcnt >= sc->sc_brtmax)
			goto done;
		p = (struct bridge_rtnode *)malloc(
		    sizeof(struct bridge_rtnode), M_DEVBUF, M_NOWAIT);
		if (p == NULL)
			goto done;

		bcopy(ea, &p->brt_addr, sizeof(p->brt_addr));
		p->brt_if = ifp;
		p->brt_age = 1;

		if (setflags)
			p->brt_flags = flags;
		else
			p->brt_flags = IFBAF_DYNAMIC;
		
#if defined (CONFIG_RTL_IGMP_SNOOPING)
		int i;
		p->group_src = 0;
		p->igmpFlag=0;
		for(i=0 ; i<IGMP_EXT_NUM ;i++)
		{
			p->igmp_ext_arr[i].valid = 0;
			p->portUsedNum[i] = 0;		
		}
#endif
		LIST_INSERT_HEAD(&sc->sc_rts[h], p, brt_next);
		sc->sc_brtcnt++;
		goto want;
	}

	do {
		q = p;
		p = LIST_NEXT(p, brt_next);

		dir = memcmp(ea, &q->brt_addr, sizeof(q->brt_addr));
		if (dir == 0) {
			if (setflags) {
				q->brt_if = ifp;
				q->brt_flags = flags;
			}

			if (q->brt_if == ifp)
				q->brt_age = 1;
			ifp = q->brt_if;
			goto want;
		}

		if (dir > 0) {
			if (sc->sc_brtcnt >= sc->sc_brtmax)
				goto done;
			p = (struct bridge_rtnode *)malloc(
			    sizeof(struct bridge_rtnode), M_DEVBUF, M_NOWAIT);
			if (p == NULL)
				goto done;

			bcopy(ea, &p->brt_addr, sizeof(p->brt_addr));
			p->brt_if = ifp;
			p->brt_age = 1;
			
#if defined (CONFIG_RTL_IGMP_SNOOPING)
			int i;
			p->group_src = 0;
			p->igmpFlag=0;
			for(i=0 ; i<IGMP_EXT_NUM ;i++)
			{
				p->igmp_ext_arr[i].valid = 0;
				p->portUsedNum[i] = 0;		
			}
#endif
			if (setflags)
				p->brt_flags = flags;
			else
				p->brt_flags = IFBAF_DYNAMIC;

			LIST_INSERT_BEFORE(q, p, brt_next);
			sc->sc_brtcnt++;
			goto want;
		}

		if (p == NULL) {
			if (sc->sc_brtcnt >= sc->sc_brtmax)
				goto done;
			p = (struct bridge_rtnode *)malloc(
			    sizeof(struct bridge_rtnode), M_DEVBUF, M_NOWAIT);
			if (p == NULL)
				goto done;

			bcopy(ea, &p->brt_addr, sizeof(p->brt_addr));
			p->brt_if = ifp;
			p->brt_age = 1;
			
#if defined (CONFIG_RTL_IGMP_SNOOPING)
			int i;
			p->group_src = 0;
			p->igmpFlag=0;
			for(i=0 ; i<IGMP_EXT_NUM ;i++)
			{
				p->igmp_ext_arr[i].valid = 0;
				p->portUsedNum[i] = 0;		
			}
#endif
			if (setflags)
				p->brt_flags = flags;
			else
				p->brt_flags = IFBAF_DYNAMIC;
			LIST_INSERT_AFTER(q, p, brt_next);
			sc->sc_brtcnt++;
			goto want;
		}
	} while (p != NULL);

done:
	p = NULL;
want:
	splx(s);
	return (p);
}
#endif
/*
 * The following hash function is adapted from 'Hash Functions' by Bob Jenkins
 * ("Algorithm Alley", Dr. Dobbs Journal, September 1997).
 * "You may use this code any way you wish, private, educational, or
 *  commercial.  It's free."
 */
#define	mix(a,b,c) \
	do {						\
		a -= b; a -= c; a ^= (c >> 13);		\
		b -= c; b -= a; b ^= (a << 8);		\
		c -= a; c -= b; c ^= (b >> 13);		\
		a -= b; a -= c; a ^= (c >> 12);		\
		b -= c; b -= a; b ^= (a << 16);		\
		c -= a; c -= b; c ^= (b >> 5);		\
		a -= b; a -= c; a ^= (c >> 3);		\
		b -= c; b -= a; b ^= (a << 10);		\
		c -= a; c -= b; c ^= (b >> 15);		\
	} while(0)

u_int32_t
bridge_hash(addr)
	struct ether_addr *addr;
{
	u_int32_t a = 0x9e3779b9, b = 0x9e3779b9, c = 0xdeadbeef;

	b += addr->octet[5] << 8;
	b += addr->octet[4];
	a += addr->octet[3] << 24;
	a += addr->octet[2] << 16;
	a += addr->octet[1] << 8;
	a += addr->octet[0];

	mix(a, b, c);
	return (c & BRIDGE_RTABLE_MASK);
}

/*
 * Trim the routing table so that we've got a number of routes
 * less than or equal to the maximum.
 */
void
bridge_rttrim(sc)
	struct bridge_softc *sc;
{
	struct bridge_rtnode *n, *p;
	int s, i;

	s = splhigh();
	if (sc->sc_rts == NULL)
		goto done;

	/*
	 * Make sure we have to trim the address table
	 */
	if (sc->sc_brtcnt <= sc->sc_brtmax)
		goto done;

	/*
	 * Force an aging cycle, this might trim enough addresses.
	 */
	splx(s);
	bridge_rtage(sc);
	s = splhigh();

	if (sc->sc_brtcnt <= sc->sc_brtmax)
		goto done;

	for (i = 0; i < BRIDGE_RTABLE_SIZE; i++) {
		n = LIST_FIRST(&sc->sc_rts[i]);
		while (n != NULL) {
			p = LIST_NEXT(n, brt_next);
			if ((n->brt_flags & IFBAF_TYPEMASK) == IFBAF_DYNAMIC) {
				LIST_REMOVE(n, brt_next);
				sc->sc_brtcnt--;
				#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
				rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, n->brt_addr.octet);
				#endif
				free(n, M_DEVBUF);
				n = p;
				if (sc->sc_brtcnt <= sc->sc_brtmax)
					goto done;
			}
		}
	}

done:
	if (sc->sc_rts != NULL && sc->sc_brtcnt == 0 &&
	    (sc->sc_if.if_flags & IFF_UP) == 0) {
		free(sc->sc_rts, M_DEVBUF);
		sc->sc_rts = NULL;
	}

	splx(s);
}

/*
 * Perform an aging cycle
 */
void
bridge_rtage(vsc)
	void *vsc;
{
	struct bridge_softc *sc = (struct bridge_softc *)vsc;
	struct bridge_rtnode *n, *p;
	int s, i;
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	int igmp_cnt = 0;
	#endif
	//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
	if(sc==NULL)	
		return;
	
	s = splhigh();
	if (sc->sc_rts == NULL) {
		splx(s);
		return;
	}

	for (i = 0; i < BRIDGE_RTABLE_SIZE; i++) {
		n = LIST_FIRST(&sc->sc_rts[i]);
		while (n != NULL) {
			if ((n->brt_flags & IFBAF_TYPEMASK) == IFBAF_STATIC) {
				n->brt_age = !n->brt_age;
				if (n->brt_age)
					n->brt_age = 0;
				n = LIST_NEXT(n, brt_next);
			} else if (n->brt_age) {
				n->brt_age = 0;
				n = LIST_NEXT(n, brt_next);
			} else {
				p = LIST_NEXT(n, brt_next);
				#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
				if(rtl_bridge_rtnode_cleanup_hooks(n)==SUCCESS){
					n=p;
					continue;
				}
				#endif
				#if defined (CONFIG_RTL_IGMP_SNOOPING)
				if(n->igmpFlag){
					
					br_igmp_ext_entry_expired(n);
					igmp_cnt = get_igmp_ext_entryCnt(n);
					
					DEBUG_PRINT("[%s]:[%d],igmp_cnt:%d,n->igmpFlag:%d,da:%x-%x-%x-%x-%x-%x,.\n",__FUNCTION__,__LINE__,igmp_cnt,n->igmpFlag,
					n->brt_addr.octet[0],n->brt_addr.octet[1],n->brt_addr.octet[2],n->brt_addr.octet[3],n->brt_addr.octet[4],n->brt_addr.octet[5]);
					
											
				}
				if((n->igmpFlag &&igmp_cnt==0)||(n->igmpFlag==0) )
				{
					
					DEBUG_PRINT("DEL rt![%s]:[%d].n->igmpFlag:%d.da:%x-%x-%x-%x-%x-%x.\n",__FUNCTION__,__LINE__,n->igmpFlag,
					n->brt_addr.octet[0],n->brt_addr.octet[1],n->brt_addr.octet[2],n->brt_addr.octet[3],n->brt_addr.octet[4],n->brt_addr.octet[5]);

					#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
					rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, n->brt_addr.octet);
					#endif

					LIST_REMOVE(n, brt_next);
					sc->sc_brtcnt--;
					free(n, M_DEVBUF);
				}
				n = p;
				#else
				#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
				rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, n->brt_addr.octet);
				#endif

				LIST_REMOVE(n, brt_next);
				sc->sc_brtcnt--;
				free(n, M_DEVBUF);
				n = p;
				#endif
			}
		}
	}
	splx(s);

	if (sc->sc_brttimeout != 0){
		callout_reset(&br_timer,sc->sc_brttimeout ,bridge_rtage, sc);
	}
}

/*
 * Remove all dynamic addresses from the cache
 */
int
bridge_rtflush(sc, full)
	struct bridge_softc *sc;
	int full;
{
	int s, i;
	struct bridge_rtnode *p, *n;

	s = splhigh();
	if (sc->sc_rts == NULL)
		goto done;

	for (i = 0; i < BRIDGE_RTABLE_SIZE; i++) {
		n = LIST_FIRST(&sc->sc_rts[i]);
		while (n != NULL) {
			if (full ||
			    (n->brt_flags & IFBAF_TYPEMASK) == IFBAF_DYNAMIC) {
				p = LIST_NEXT(n, brt_next);
				LIST_REMOVE(n, brt_next);
				sc->sc_brtcnt--;
				#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
				rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, n->brt_addr.octet);
				#endif
				free(n, M_DEVBUF);
				n = p;
			} else
				n = LIST_NEXT(n, brt_next);
		}
	}

	if (sc->sc_brtcnt == 0 && (sc->sc_if.if_flags & IFF_UP) == 0) {
		free(sc->sc_rts, M_DEVBUF);
		sc->sc_rts = NULL;
	}

done:
	splx(s);
	return (0);
}

/*
 * Remove an address from the cache
 */
int
bridge_rtdaddr(sc, ea)
	struct bridge_softc *sc;
	struct ether_addr *ea;
{
	int h, s;
	struct bridge_rtnode *p;

	s = splhigh();
	if (sc->sc_rts == NULL)
		goto done;

	h = bridge_hash(ea);
	p = LIST_FIRST(&sc->sc_rts[h]);
	while (p != NULL) {
		if (bcmp(ea, &p->brt_addr, sizeof(p->brt_addr)) == 0) {
			LIST_REMOVE(p, brt_next);
			sc->sc_brtcnt--;
			#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
			rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, p->brt_addr.octet);
			#endif
			free(p, M_DEVBUF);
			if (sc->sc_brtcnt == 0 &&
			    (sc->sc_if.if_flags & IFF_UP) == 0) {
				free(sc->sc_rts, M_DEVBUF);
				sc->sc_rts = NULL;
			}
			splx(s);
			return (0);
		}
		p = LIST_NEXT(p, brt_next);
	}

done:
	splx(s);
	return (ENOENT);
}
/*
 * Delete routes to a specific interface member.
 */
void
bridge_rtdelete(sc, ifp, dynonly)
	struct bridge_softc *sc;
	struct ifnet *ifp;
{
	int i, s;
	struct bridge_rtnode *n, *p;

	s = splhigh();
	if (sc->sc_rts == NULL)
		goto done;

	/*
	 * Loop through all of the hash buckets and traverse each
	 * chain looking for routes to this interface.
	 */
	for (i = 0; i < BRIDGE_RTABLE_SIZE; i++) {
		n = LIST_FIRST(&sc->sc_rts[i]);
		while (n != NULL) {
			if (n->brt_if == ifp) {		/* found one */
				p = LIST_NEXT(n, brt_next);
				LIST_REMOVE(n, brt_next);
				sc->sc_brtcnt--;
				#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
				diag_printf("[%s]:[%d],da:%x-%x-%x-%x-%x-%x,.\n",__FUNCTION__,__LINE__,
				n->brt_addr.octet[0],n->brt_addr.octet[1],n->brt_addr.octet[2],n->brt_addr.octet[3],n->brt_addr.octet[4],n->brt_addr.octet[5]);

				rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, n->brt_addr.octet);
				#endif
				free(n, M_DEVBUF);
				n = p;
			} else
				n = LIST_NEXT(n, brt_next);
		}
	}
	if (sc->sc_brtcnt == 0 && (sc->sc_if.if_flags & IFF_UP) == 0) {
		free(sc->sc_rts, M_DEVBUF);
		sc->sc_rts = NULL;
	}

done:
	splx(s);
}

/*
 * Gather all of the routes for this interface.
 */
int
bridge_rtfind(sc, baconf)
	struct bridge_softc *sc;
	struct ifbaconf *baconf;
{
	int i, s, error = 0;
	u_int32_t cnt = 0;
	struct bridge_rtnode *n;
	struct ifbareq bareq;
	char ifname[IFNAMSIZ];

	s = splhigh();

	if (sc->sc_rts == NULL || baconf->ifbac_len == 0)
		goto done;

	for (i = 0, cnt = 0; i < BRIDGE_RTABLE_SIZE; i++) {
		n = LIST_FIRST(&sc->sc_rts[i]);
		while (n != NULL) {
			if (baconf->ifbac_len <
			    (cnt + 1) * sizeof(struct ifbareq))
				goto done;
			sprintf(ifname,"%s%d",sc->sc_if.if_name,sc->sc_if.if_unit);
			bcopy(ifname, bareq.ifba_name,
			    sizeof(bareq.ifba_name));

			sprintf(ifname,"%s%d",n->brt_if->if_name,n->brt_if->if_unit);
			bcopy(ifname, bareq.ifba_ifsname,
			    sizeof(bareq.ifba_ifsname));

			bcopy(&n->brt_addr, &bareq.ifba_dst,
			    sizeof(bareq.ifba_dst));
			bareq.ifba_age = n->brt_age;
			bareq.ifba_flags = n->brt_flags;
			error = copyout((caddr_t)&bareq,
		    	    (caddr_t)(baconf->ifbac_req + cnt), sizeof(bareq));
			if (error)
				goto done;
			n = LIST_NEXT(n, brt_next);
			cnt++;
		}
	}
done:
	baconf->ifbac_len = cnt * sizeof(struct ifbareq);
	splx(s);
	return (error);
}

/*
 * Block non-ip frames:
 * Returns 0 if frame is ip, and 1 if it should be dropped.
 */
int
bridge_blocknonip(eh, m)
	struct ether_header *eh;
	struct mbuf *m;
{
	struct snap snap;
	u_int16_t etype;

	if (m->m_pkthdr.len < sizeof(struct ether_header))
		return (1);

	etype = ntohs(eh->ether_type);
	switch (etype) {
	case ETHERTYPE_ARP:
	case ETHERTYPE_REVARP:
	case ETHERTYPE_IP:
	case ETHERTYPE_IPV6:
		return (0);
	}

	if (etype > ETHERMTU)
		return (1);

	if (m->m_pkthdr.len <
	    (sizeof(struct ether_header) + sizeof(struct snap)))
		return (1);

	m_copydata(m, sizeof(struct ether_header), sizeof(struct snap),
	    (caddr_t)&snap);

	etype = ntohs(snap.type);
	if (snap.dsap == LLC_SNAP_LSAP && snap.ssap == LLC_SNAP_LSAP &&
	    snap.control == LLC_UI &&
	    snap.org[0] == 0 && snap.org[1] == 0 && snap.org[2] == 0 &&
	    (etype == ETHERTYPE_ARP ||
	     etype == ETHERTYPE_REVARP ||
	     etype == ETHERTYPE_IP ||
	     etype == ETHERTYPE_IPV6)) {
		return (0);
	}

	return (1);
}

u_int8_t
bridge_filterrule(n, eh)
	struct brl_node *n;
	struct ether_header *eh;
{
	u_int8_t flags;

	for (; n != NULL; n = SIMPLEQ_NEXT(n, brl_next)) {
		flags = n->brl_flags & (BRL_FLAG_SRCVALID|BRL_FLAG_DSTVALID);
		if (flags == 0)
			return (n->brl_action);
		if (flags == (BRL_FLAG_SRCVALID|BRL_FLAG_DSTVALID)) {
			if (bcmp(eh->ether_shost, &n->brl_src, ETHER_ADDR_LEN))
				continue;
			if (bcmp(eh->ether_dhost, &n->brl_src, ETHER_ADDR_LEN))
				continue;
			return (n->brl_action);
		}
		if (flags == BRL_FLAG_SRCVALID) {
			if (bcmp(eh->ether_shost, &n->brl_src, ETHER_ADDR_LEN))
				continue;
			return (n->brl_action);
		}
		if (flags == BRL_FLAG_DSTVALID) {
			if (bcmp(eh->ether_dhost, &n->brl_dst, ETHER_ADDR_LEN))
				continue;
			return (n->brl_action);
		}
	}
	return (BRL_ACTION_PASS);
}

int
bridge_addrule(bif, req, out)
	struct bridge_iflist *bif;
	struct ifbrlreq *req;
	int out;
{
	struct brl_node *n;

	n = (struct brl_node *)malloc(sizeof(struct brl_node), M_DEVBUF, M_NOWAIT);
	if (n == NULL)
		return (ENOMEM);
	bcopy(&req->ifbr_src, &n->brl_src, sizeof(struct ether_addr));
	bcopy(&req->ifbr_dst, &n->brl_dst, sizeof(struct ether_addr));
	n->brl_action = req->ifbr_action;
	n->brl_flags = req->ifbr_flags;
	if (out) {
		n->brl_flags &= ~BRL_FLAG_IN;
		n->brl_flags |= BRL_FLAG_OUT;
		SIMPLEQ_INSERT_TAIL(&bif->bif_brlout, n, brl_next);
	} else {
		n->brl_flags &= ~BRL_FLAG_OUT;
		n->brl_flags |= BRL_FLAG_IN;
		SIMPLEQ_INSERT_TAIL(&bif->bif_brlin, n, brl_next);
	}
	return (0);
}

int
bridge_flushrule(bif)
	struct bridge_iflist *bif;
{
	struct brl_node *p, *q;

	p = SIMPLEQ_FIRST(&bif->bif_brlin);
	while (p != NULL) {
		q = SIMPLEQ_NEXT(p, brl_next);
		SIMPLEQ_REMOVE_HEAD(&bif->bif_brlin, p, brl_next);
		free(p, M_DEVBUF);
		p = q;
	}
	p = SIMPLEQ_FIRST(&bif->bif_brlout);
	while (p != NULL) {
		q = SIMPLEQ_NEXT(p, brl_next);
		SIMPLEQ_REMOVE_HEAD(&bif->bif_brlout, p, brl_next);
		free(p, M_DEVBUF);
		p = q;
	}
	return (0);
}

#if defined(INET) && (defined(IPFILTER) || defined(IPFILTER_LKM))

/*
 * Maximum sized IP header
 */
union maxip {
	struct ip ip;
	u_int32_t _padding[16];
};

/*
 * Filter IP packets by peeking into the ethernet frame.  This violates
 * the ISO model, but allows us to act as a IP filter at the data link
 * layer.  As a result, most of this code will look familiar to those
 * who've read net/if_ethersubr.c and netinet/ip_input.c
 */
struct mbuf *
bridge_filter(sc, ifp, eh, m)
	struct bridge_softc *sc;
	struct ifnet *ifp;
	struct ether_header *eh;
	struct mbuf *m;
{
	struct snap snap;
	int hassnap = 0;
	struct ip *ip;
	int hlen;

	if (fr_checkp == NULL)
		return (m);

	if (eh->ether_type != htons(ETHERTYPE_IP)) {
		if (eh->ether_type > ETHERMTU ||
		    m->m_pkthdr.len < (sizeof(struct snap) +
		    sizeof(struct ether_header)))
			return (m);

		m_copydata(m, sizeof(struct ether_header),
		    sizeof(struct snap), (caddr_t)&snap);

		if (snap.dsap != LLC_SNAP_LSAP || snap.ssap != LLC_SNAP_LSAP ||
		    snap.control != LLC_UI ||
		    snap.org[0] != 0 || snap.org[1] != 0 || snap.org[2] ||
		    snap.type != htons(ETHERTYPE_IP))
			return (m);
		hassnap = 1;
	}

	m_adj(m, sizeof(struct ether_header));
	if (hassnap)
		m_adj(m, sizeof(struct snap));

	if (m->m_pkthdr.len < sizeof(struct ip))
		goto dropit;

	/* Copy minimal header, and drop invalids */
	if (m->m_len < sizeof(struct ip) &&
	    (m = m_pullup(m, sizeof(struct ip))) == NULL)
		return (NULL);
	ip = mtod(m, struct ip *);

	if (ip->ip_v != IPVERSION)
		goto dropit;

	hlen = ip->ip_hl << 2;	/* get whole header length */
	if (hlen < sizeof(struct ip))
		goto dropit;
	if (hlen > m->m_len) {
		if ((m = m_pullup(m, sizeof(struct ip))) == NULL)
			return (NULL);
		ip = mtod(m, struct ip *);
	}

	if ((ip->ip_sum = in_cksum(m, hlen)) != 0)
		goto dropit;

	NTOHS(ip->ip_len);
	if (ip->ip_len < hlen)
		goto dropit;
	NTOHS(ip->ip_id);
	NTOHS(ip->ip_off);

	if (m->m_pkthdr.len < ip->ip_len)
		goto dropit;
	if (m->m_pkthdr.len > ip->ip_len) {
		if (m->m_len == m->m_pkthdr.len) {
			m->m_len = ip->ip_len;
			m->m_pkthdr.len = ip->ip_len;
		} else
			m_adj(m, ip->ip_len - m->m_pkthdr.len);
	}

	/* Finally, we get to filter the packet! */
	if (fr_checkp && (*fr_checkp)(ip, hlen, ifp, 0, &m))
		return (NULL);

	/* Rebuild the IP header */
	if (m->m_len < hlen && ((m = m_pullup(m, hlen)) == NULL))
		return (NULL);
	if (m->m_len < sizeof(struct ip))
		goto dropit;
	ip = mtod(m, struct ip *);
	HTONS(ip->ip_len);
	HTONS(ip->ip_id);
	HTONS(ip->ip_off);
	ip->ip_sum = in_cksum(m, hlen);

	/* Reattach SNAP header */
	if (hassnap) {
		M_PREPEND(m, sizeof(snap), M_DONTWAIT);
		if (m == NULL)
			goto dropit;
		bcopy(&snap, mtod(m, caddr_t), sizeof(snap));
	}

	/* Reattach ethernet header */
	M_PREPEND(m, sizeof(*eh), M_DONTWAIT);
	if (m == NULL)
		goto dropit;
	bcopy(eh, mtod(m, caddr_t), sizeof(*eh));

	return (m);

dropit:
	if (m != NULL)
		m_freem(m);
	return (NULL);
}
#endif
#if 0
int
ifpromisc(ifp, pswitch)
	struct ifnet *ifp;
	int pswitch;
{
	struct ifreq ifr;

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
	} else {
		if (--ifp->if_pcount > 0)
			return (0);
		ifp->if_flags &= ~IFF_PROMISC;
		/*
		 * If the device is not configured up, we should not need to
		 * turn off promiscuous mode (device should have turned it
		 * off when interface went down; and will look at IFF_PROMISC
		 * again next time interface comes up).
		 */
		if ((ifp->if_flags & IFF_UP) == 0)
			return (0);
	}
	ifr.ifr_flags = ifp->if_flags;
	
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WRAPPER
  if (!memcmp(ifp->if_xname, "eth", 3) || 
        !memcmp(ifp->if_xname, "wlan", 4))
      return 0;  
#endif  
          
	return ((*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, (caddr_t)&ifr));
}
#endif
int
bridge_ifenqueue(sc, ifp, m)
        struct bridge_softc *sc;
        struct ifnet *ifp;
        struct mbuf *m;
{
        int error, len;
        short mflags;

        len = m->m_pkthdr.len;
        mflags = m->m_flags;
        //IFQ_ENQUEUE(&ifp->if_snd, m, NULL, error);
		do {                                                                    
        	if (IF_QFULL((&ifp->if_snd))) {                                          
                m_freem((m));                                           
                (error) = 1;                                        
        	} else {                                                        
                IF_ENQUEUE((&ifp->if_snd), (m));                                 
                (error) = 0;                                              
       	 	}                                                               
       		 if ((error))                                                      
                (&ifp->if_snd)->ifq_drops++;                                     
		} while (0);
		
        if (error) {
                sc->sc_if.if_oerrors++;
                return (error);
        }
        sc->sc_if.if_opackets++;
        sc->sc_if.if_obytes += len;
        ifp->if_obytes += len;
        if (mflags & M_MCAST)
                ifp->if_omcasts++;
        if ((ifp->if_flags & IFF_OACTIVE) == 0)
                (*ifp->if_start)(ifp);
                                                                                                           return (0);
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)

#define IGMP_EXPIRE_TIME (260*HZ)



int chk_igmp_ext_entry(
	struct bridge_rtnode *rt,
	unsigned char *srcMac)
{

	int i2;
	unsigned char *add;
	add = rt->brt_addr.octet;

	for(i2=0 ; i2 < IGMP_EXT_NUM ; i2++){
		if(rt->igmp_ext_arr[i2].valid == 1){
			if(!memcmp(rt->igmp_ext_arr[i2].SrcMac , srcMac ,6)){
				return 1;
			}
		}
	}
	return 0;
}

int bitmask_to_id(unsigned char val)
{
	int i;
	for (i=0; i<8; i++) {
		if (val & (1 <<i))
			break;
	}

	if(i>=8)
	{
		i=7;
	}
	return (i);
}

void add_igmp_ext_entry(struct bridge_rtnode *rt ,
	unsigned char *srcMac , unsigned char portComeIn)
{

//	if(fdb->igmp_ext_array == NULL)
//		return 0;

	int i2;
	unsigned char *add;
	add =rt->brt_addr.octet;	

	DEBUG_PRINT("add_igmp.DA=%02x:%02x:%02x:%02x:%02x:%02x  SA=%02x:%02x:%02x:%02x:%02x:%02x\n",
		add[0],add[1],add[2],add[3],add[4],add[5],
		srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);
	
	for(i2=0 ; i2 < IGMP_EXT_NUM ; i2++){
		if(rt->igmp_ext_arr[i2].valid == 0){
			rt->igmp_ext_arr[i2].valid = 1	;
			rt->igmp_ext_arr[i2].ageing_time = (unsigned long)(jiffies) ;			
			memcpy(rt->igmp_ext_arr[i2].SrcMac , srcMac ,6);
			rt->igmp_ext_arr[i2].port = portComeIn ;
			
			rt->portlist |= portComeIn;
			rt->portUsedNum[bitmask_to_id(portComeIn)]++;
			DEBUG_PRINT("portUsedNum[%d]=%d\n\n",bitmask_to_id(portComeIn) , rt->portUsedNum[bitmask_to_id(portComeIn)]);
			return ;
		}
	}
	DEBUG_PRINT("%s:entry Rdy existed!!!\n", __FUNCTION__);	
}

void update_igmp_ext_entry(struct bridge_rtnode *rt ,
	unsigned char *srcMac , unsigned char portComeIn)
{
	int i2;
	unsigned char *add;
	add =rt->brt_addr.octet;

		DEBUG_PRINT("update_igmp,DA=%02x:%02x:%02x:%02x:%02x:%02x ; SA=%02x:%02x:%02x:%02x:%02x:%02x\n",
		add[0],add[1],add[2],add[3],add[4],add[5],
		srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);

	for(i2=0 ; i2 < IGMP_EXT_NUM ; i2++){
		if(rt->igmp_ext_arr[i2].valid == 1){
			if(!memcmp(rt->igmp_ext_arr[i2].SrcMac , srcMac ,6)){

				rt->igmp_ext_arr[i2].ageing_time = (unsigned long)(jiffies) ;
				
				//DEBUG_PRINT("update jiffies ok!\n");
				if(rt->igmp_ext_arr[i2].port != portComeIn){

					unsigned char port_orig = rt->igmp_ext_arr[i2].port ;					
					int index = bitmask_to_id(port_orig);

					rt->portUsedNum[index]-- ;
					DEBUG_PRINT("(--) portUsedNum[%d]=%d\n",index,rt->portUsedNum[index]);					
					if(rt->portUsedNum[index] <= 0){
						rt->portlist &= ~(port_orig);
						if(rt->portUsedNum[index]< 0){
							DEBUG_PRINT("!! portNum[%d] < 0 at (update_igmp_ext_entry)\n",index);
							rt->portUsedNum[index] = 0 ;
						}
					}					

					
					rt->portUsedNum[bitmask_to_id(portComeIn)]++;
					DEBUG_PRINT("(++) portUsedNum[%d]=%d\n",bitmask_to_id(portComeIn) , rt->portUsedNum[bitmask_to_id(portComeIn)] );										
					rt->portlist |= portComeIn;						

					
					rt->igmp_ext_arr[i2].port = portComeIn ;					
					DEBUG_PRINT("	!!! portlist be updated:%x !!!!\n",rt->portlist);
					
				}
				return ;
			}			
		}		
	}

	DEBUG_PRINT("%s: ...fail!!\n", __FUNCTION__);
}


void del_igmp_ext_entry(struct bridge_rtnode *rt ,unsigned char *srcMac , unsigned char portComeIn )
{
	int i2;
	unsigned char *add;
	add =rt->brt_addr.octet;	

	DEBUG_PRINT("del_igmp,DA=%02x:%02x:%02x:%02x:%02x:%02x ; SA=%02x:%02x:%02x:%02x:%02x:%02x\n",
		add[0],add[1],add[2],add[3],add[4],add[5],
		srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);
				
	for(i2=0 ; i2 < IGMP_EXT_NUM ; i2++){
		if(rt->igmp_ext_arr[i2].valid == 1){
			if(!memcmp(rt->igmp_ext_arr[i2].SrcMac , srcMac ,6))
			{
				rt->igmp_ext_arr[i2].ageing_time -=  300*HZ; // make it expired	
				rt->igmp_ext_arr[i2].valid = 0;
				DEBUG_PRINT("\ndel_igmp_ext_entry,DA=%02x:%02x:%02x:%02x:%02x:%02x ; SA=%02x:%02x:%02x:%02x:%02x:%02x success!!!\n",
				add[0],add[1],add[2],add[3],add[4],add[5],
				srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);
				
				//DEBUG_PRINT("%s:success!!\n", __FUNCTION__);
				
				if(portComeIn != 0){
					int index;
					index = bitmask_to_id(portComeIn);
					rt->portUsedNum[index]--;
					if(rt->portUsedNum[index] <= 0){
						DEBUG_PRINT("portUsedNum[%d] == 0 ,update portlist from (%x)" ,index ,rt->portlist);
						rt->portlist &= ~ portComeIn;
						DEBUG_PRINT("to (%x) \n" ,rt->portlist);
						
						if(rt->portUsedNum[index] < 0){
						DEBUG_PRINT("!! portUsedNum[%d]=%d < 0 at (del_igmp_ext_entry)\n",index ,rt->portUsedNum[index]);
						rt->portUsedNum[index] = 0;
						}
					}else{
						DEBUG_PRINT("(del) portUsedNum[%d] = %d \n",index, rt->portUsedNum[index]);
					}
				
				}	
				DEBUG_PRINT("\n");
				return ;
			}			
		}
	}

	DEBUG_PRINT("%s:entry not existed!!\n\n", __FUNCTION__);	
}

int get_igmp_ext_entryCnt (struct bridge_rtnode * rt)
{
	int i2;
	int ext_cnt=0;
	unsigned long igmp_walktimeout;	
	
	igmp_walktimeout = (unsigned long)(jiffies - IGMP_EXPIRE_TIME);
	if (rt==NULL)
		return -1;
	
	for(i2=0 ; i2 < IGMP_EXT_NUM ; i2++){
		if(rt->igmp_ext_arr[i2].valid == 1){

			// not timeout entry
			if(time_after_eq(rt->igmp_ext_arr[i2].ageing_time, igmp_walktimeout))
			{
				ext_cnt += 1;
				
			}
		}
	}	
	return ext_cnt;
	
}


void br_igmp_ext_entry_expired(struct bridge_rtnode *rt)
{
	
	int i2;
	unsigned long igmp_walktimeout;	
	unsigned char *DA;
	unsigned char *SA;
	#if defined	(MCAST_TO_UNICAST)
	struct net_device *dev=NULL;
	#endif
	struct ifnet *ifp;
	if (rt==NULL)
		return;
	igmp_walktimeout = 	(unsigned long)(jiffies - IGMP_EXPIRE_TIME);	
	DEBUG_PRINT("---------------------\ncheck br_igmp_rt_expired.\n---------------------\n");  
	//IGMP_EXPIRE_TIME
	for(i2=0 ; i2 < IGMP_EXT_NUM ; i2++)
	{
		if(rt->igmp_ext_arr[i2].valid == 1){

			// when timeout expire
			if(time_before_eq(rt->igmp_ext_arr[i2].ageing_time, igmp_walktimeout))
			{
				DEBUG_PRINT("%s:%d\n",__FUNCTION__,__LINE__);	
				SA = rt->igmp_ext_arr[i2].SrcMac;					
				DEBUG_PRINT("expired src mac:%02x,%02x,%02x,%02x,%02x,%02x\n",
					SA[0],SA[1],SA[2],SA[3],SA[4],SA[5]);								

				DA = rt->brt_addr.octet;					
				DEBUG_PRINT("dmac:%02x:%02x:%02x-%02x:%02x:%02x\n",
					DA[0],DA[1],DA[2],DA[3],DA[4],DA[5]);				



				/*---for process wlan client expired start---*/								
				
				//dev = __dev_get_by_name(&init_net, RTL_PS_WLAN0_DEV_NAME);	
				struct net_device *dev; 
				struct eth_drv_sc *drv_sc;
				Rltk819x_t *rltk819x_info;
				ifp=rt->brt_if;
				drv_sc= (struct eth_drv_sc *)ifp->if_softc;
				rltk819x_info = (Rltk819x_t *)(drv_sc->driver_private);

				dev = (struct net_device *)(rltk819x_info->dev);
				//if(dev)
				//	diag_printf("dev:%s,[%s]:[%d].\n",dev->name,__FUNCTION__,__LINE__);
				
				
				if (dev) {		
					unsigned char StaMacAndGroup[20];
					memcpy(StaMacAndGroup, DA , 6);
					memcpy(StaMacAndGroup+6, SA, 6);
				#if defined	(MCAST_TO_UNICAST)
					if(!memcmp(dev->name, RTL_PS_WLAN_NAME, 4)){
						#if 1//defined(CONFIG_COMPAT_NET_DEV_OPS)
							if (dev->do_ioctl != NULL) {
								dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);
						#else
							if (dev->netdev_ops->ndo_do_ioctl != NULL) {
								dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);
						#endif
								DEBUG_PRINT("(rt expire) wlan0 ioctl to DEL! M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
									StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
									StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
							}
						}
					}	
				#endif			
				/*---for process wlan client expired end---*/

				
				del_igmp_ext_entry(rt , SA , rt->igmp_ext_arr[i2].port);
				
				
				if ( (rt->portlist & 0x7f)==0){
					rt->group_src &=  ~(1 << 1); // eth0 all leave
				}
			
				if ( (rt->portlist & 0x80)==0){
					rt->group_src &=  ~(1 << 2); // wlan0 all leave
				}
			
			
			}			
			
		}		
		
	}		
	
}
#if defined (CONFIG_RTL_BRIDGE_QUERY_SUPPORT)

int igmpVersion=2;
int mldVersion=1;

static int igmpQueryEnabled=1;
static int mldQueryEnabled=0;
static unsigned int mCastQueryTimerCnt=0;

#define IFP2AC(IFP) ((struct arpcom *)IFP)

/*igmpv3 general query*/
static unsigned char igmpV3QueryBuf[64]={	0x01,0x00,0x5e,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*offset:6*/
									0x08,0x00,						/*offset:12*/
									0x46,0x00,0x00,0x24,				/*offset:14*/
									0x00,0x00,0x40,0x00,				/*offset:18*/
									0x01,0x02,0x00,0x00,				/*offset:22*/
									0x00,0x00,0x00,0x00,				/*offset:26,source ip*/
									0xe0,0x00,0x00,0x01,				/*offset:30,destination ip*/
									0x94,0x04,0x00,0x00,				/*offset:34,router alert option*/
									0x11,0x01,0x00,0x00,				/*offset:38*/
									0x00,0x00,0x00,0x00,				/*offset:42,queried multicast ip address*/
									0x0a,0x3c,0x00,0x00,				/*offset:46*/
									0x00,0x00,0x00,0x00,				/*offset:50*/
									0x00,0x00,0x00,0x00,				/*offset:54*/
									0x00,0x00,0x00,0x00,				/*offset:58*/
									0x00,0x00							/*offset:62*/
									
								};			



/*igmpv2 general query*/
static unsigned char igmpV2QueryBuf[64]={	0x01,0x00,0x5e,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,
/*0x00,0x00,0x00,0x46,0x79,0x78,*/		/*offset:6*/
									0x08,0x00,						/*offset:12*/
									0x45,0x00,0x00,0x1c,				/*offset:14*/
									0x00,0x00,0x40,0x00,				/*offset:18*/
									0x01,0x02,0x00,0x00,				/*offset:22*/
									0xc0,0xa8,0x01,0xfe,				/*offset:26*/
									0xe0,0x00,0x00,0x01,				/*offset:30*/
									0x11,0x01,0x0c,0xfa,				/*offset:34*/
									0x00,0x00,0x00,0x00,				/*offset:38*/
									0x00,0x00,0x00,0x00,				/*offset:42*/
									0x00,0x00,0x00,0x00,				/*offset:46*/
									0x00,0x00,0x00,0x00,				/*offset:50*/
									0x00,0x00,0x00,0x00,				/*offset:54*/
									0x00,0x00,0x00,0x00,				/*offset:58*/
									0x00,0x00							/*offset:62*/
									
								};			


static unsigned char mldQueryBuf[90]={	0x33,0x33,0x00,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*source mac*/	/*offset:6*/
									0x86,0xdd,						/*ether type*/	/*offset:12*/
									0x60,0x00,0x00,0x00,				/*version(1 byte)-traffic cliass(1 byte)- flow label(2 bytes)*/	/*offset:14*/
									0x00,0x20,0x00,0x01,				/*payload length(2 bytes)-next header(1 byte)-hop limit(value:1 1byte)*//*offset:18*/
									0xfe,0x80,0x00,0x00,				/*source address*/	/*offset:22*/
									0x00,0x00,0x00,0x00,				/*be zero*/	/*offset:26*/
									0x00,0x00,0x00,					/*upper 3 bytes mac address |0x02*/ /*offset:30*/
									0xff,0xfe,						/*fixed*/
									0x00,0x00,0x00,					/*lowert 3 bytes mac address*/	 /*offset:35*/
									0xff,0x02,0x00,0x00,				/*destination address is fixed as FF02::1*/	/*offset:38*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x01,			
									0x3a,0x00,						/*icmp type(1 byte)-length(1 byte)*/	 /*offset:54*/
									0x05,0x02,0x00,0x00,				/*router alert option*/
									0x01,0x00,						/*padN*/
									0x82,0x00,						/*type(query:0x82)-code(0)*/	/*offset:62*/
									0x00,0x00,						/*checksum*/	/*offset:64*/
									0x00,0x0a,						/*maximum reponse code*/
									0x00,0x00,						/*reserved*/
									0x00,0x00,0x00,0x00,				/*multicast address,fixed as 0*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,
									0x0a,0x3c,0x00,0x00
								};		

static unsigned char ipv6PseudoHdrBuf[40]=	{	
									0xfe,0x80,0x00,0x00,				/*source address*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0xff,			
									0xfe,0x00,0x00,0x00,			 	
									0xff,0x02,0x00,0x00,				/*destination address*/
									0x00,0x00,0x00,0x00,		
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x01,				
									0x00,0x00,0x00,0x18,				/*upper layer packet length*/
									0x00,0x00,0x00,0x3a					/*zero padding(3 bytes)-next header(1 byte)*/
									};	

static unsigned short  br_ipv4Checksum(unsigned char *pktBuf, unsigned int pktLen)
{
	/*note: the first bytes of	packetBuf should be two bytes aligned*/
	unsigned int  checksum=0;
	unsigned int  count=pktLen;
	unsigned short	 *ptr= (unsigned short *)pktBuf;	
	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((unsigned short) ~ checksum);

}


static unsigned short br_ipv6Checksum(unsigned char *pktBuf, unsigned int pktLen, unsigned char  *ipv6PseudoHdrBuf)
{
	unsigned int  checksum=0;
	unsigned int count=pktLen;
	unsigned short   *ptr;

	/*compute ipv6 pseudo-header checksum*/
	ptr= (unsigned short  *) (ipv6PseudoHdrBuf);	
	for(count=0; count<20; count++) /*the pseudo header is 40 bytes long*/
	{
		  checksum+= ntohs(*ptr);
		  ptr++;
	}
	
	/*compute the checksum of mld buffer*/
	 count=pktLen;
	 ptr=(unsigned short  *) (pktBuf);	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);
}	


static unsigned char* br_generateIgmpQuery(int type,struct bridge_softc *sc )
{

	unsigned short checkSum=0;
	struct ifaddr *ifa;
	struct ifnet * ifp =NULL;
	struct bridge_iflist *p;
	struct sockaddr_in * addr; 
	struct arpcom *ac ;
	
	if(sc == NULL)
	{
		return NULL;
	}

	for (p = LIST_FIRST(&sc->sc_iflist); p != NULL;p = LIST_NEXT(p, next)) {
		if(!p->ifp) 	
			continue;
		if((memcmp(p->ifp->if_name,"eth",3)==0)&&(p->ifp->if_unit==0)){
			ifp= p->ifp;
			break;
		}	
	}
	if(ifp==NULL)
		return NULL;
	ac= IFP2AC(ifp);
	//diag_printf("set smac:%x-%x-%x-%x-%x-%x,[%s]:[%d].\n",ac->ac_enaddr[0],ac->ac_enaddr[1],ac->ac_enaddr[2],ac->ac_enaddr[3],ac->ac_enaddr[4],ac->ac_enaddr[5],__FUNCTION__,__LINE__);
	if (ac)
	{
		if(igmpVersion == 3)
		{
			memcpy(&igmpV3QueryBuf[6],ac->ac_enaddr,6);			/*set source mac address*/
		}
		else
		{
			memcpy(&igmpV2QueryBuf[6],ac->ac_enaddr,6);			/*set source mac address*/
		}
	}
	/*set source ip address*/
	for (ifa = ifp->if_addrhead.tqh_first; ifa;ifa = ifa->ifa_link.tqe_next) {
		if (ifa->ifa_addr->sa_family != AF_INET)
			continue;
		if(ifa)
		{
			addr =(struct sockaddr_in *)ifa->ifa_addr;
			//diag_printf("addr:%x[%s]:[%d].\n",addr->sin_addr.s_addr,__FUNCTION__,__LINE__);
		}
	}

	if(addr){
		if(igmpVersion==3)
		{
			memcpy(&igmpV3QueryBuf[26],&addr->sin_addr.s_addr,4);
		}
		else
		{
			memcpy(&igmpV2QueryBuf[26],&addr->sin_addr.s_addr,4);
		}
	}	

	if(igmpVersion==3)
	{
		igmpV3QueryBuf[24]=0;
		igmpV3QueryBuf[25]=0;
	}
	else
	{
		igmpV2QueryBuf[24]=0;
		igmpV2QueryBuf[25]=0;
	}

	
	if(igmpVersion==3)
	{
		checkSum=br_ipv4Checksum(&igmpV3QueryBuf[14],24);
	}
	else
	{
		checkSum=br_ipv4Checksum(&igmpV2QueryBuf[14],20);
	}

	if(igmpVersion==3)
	{
		igmpV3QueryBuf[24]=(checkSum&0xff00)>>8;
		igmpV3QueryBuf[25]=(checkSum&0x00ff);

	}
	else
	{
		igmpV2QueryBuf[24]=(checkSum&0xff00)>>8;
		igmpV2QueryBuf[25]=(checkSum&0x00ff);

	}
	

	if(igmpVersion==3)
	{
		igmpV3QueryBuf[40]=0;
		igmpV3QueryBuf[41]=0;
		checkSum=br_ipv4Checksum(&igmpV3QueryBuf[38],12);
		igmpV3QueryBuf[40]=(checkSum&0xff00)>>8;
		igmpV3QueryBuf[41]=(checkSum&0x00ff);
	}
	else
	{
		igmpV2QueryBuf[36]=0;
		igmpV2QueryBuf[37]=0;
		checkSum=br_ipv4Checksum(&igmpV2QueryBuf[34],8);
		igmpV2QueryBuf[36]=(checkSum&0xff00)>>8;
		igmpV2QueryBuf[37]=(checkSum&0x00ff);
	}

	if(igmpVersion==3)
	{
		return igmpV3QueryBuf;
	}
	else
	{
		return igmpV2QueryBuf;
	}
	
	return NULL;
}

static unsigned char* br_generateMldQuery(int type,struct bridge_softc *sc )
{

	unsigned short checkSum=0;
	struct ifaddr *ifa;
	struct ifnet * ifp = NULL;
	struct bridge_iflist *p;
	struct sockaddr_in * addr; 
	struct arpcom *ac ;
	
	if(sc == NULL)
	{
		return NULL;
	}

	
	for (p = LIST_FIRST(&sc->sc_iflist); p != NULL;p = LIST_NEXT(p, next)) {
		if(!p->ifp) 	
			continue;
		if((memcmp(p->ifp->if_name,"eth",3)==0)&&(p->ifp->if_unit==0)){
			ifp= p->ifp;
			break;
		}	
	}
	if(ifp==NULL)
		return NULL;
	ac= IFP2AC(ifp);
	//diag_printf("set smac:%x-%x-%x-%x-%x-%x,[%s]:[%d].\n",ac->ac_enaddr[0],ac->ac_enaddr[1],ac->ac_enaddr[2],ac->ac_enaddr[3],ac->ac_enaddr[4],ac->ac_enaddr[5],__FUNCTION__,__LINE__);
	if (ac)
	{
		memcpy(&mldQueryBuf[6],ac->ac_enaddr,6);			/*set source mac address*/
	
		memcpy(&mldQueryBuf[30],ac->ac_enaddr,3);		/*set  mld query packet source ip address*/
		memcpy(&mldQueryBuf[35],&ac->ac_enaddr[3],3);	
	}
	mldQueryBuf[30]=mldQueryBuf[30]|0x02;		
		

	memcpy(ipv6PseudoHdrBuf,&mldQueryBuf[22],16);			/*set pseudo-header source ip*/
	if(mldVersion==2)
	{
		mldQueryBuf[19]=	0x24;
	}
	else
	{
		mldQueryBuf[19]=	0x20;
	}

	mldQueryBuf[64]=0;/*reset checksum*/
	mldQueryBuf[65]=0;
	if(mldVersion==2)
	{
		ipv6PseudoHdrBuf[35]=28;
		checkSum=br_ipv6Checksum(&mldQueryBuf[62],28,ipv6PseudoHdrBuf);
	}
	else
	{
		ipv6PseudoHdrBuf[35]=24;
		checkSum=br_ipv6Checksum(&mldQueryBuf[62],24,ipv6PseudoHdrBuf);
	}
	
	
	mldQueryBuf[64]=(checkSum&0xff00)>>8;
	mldQueryBuf[65]=(checkSum&0x00ff);

	return mldQueryBuf;

}

int rtl_get_igmpQueryEnabled(void)
{
	int enabled=0;
	#if defined(CONFIG_RTL_819X_SWCORE)
	if(igmpsnoopenabled)
	#endif
		enabled=igmpQueryEnabled;
	
	return enabled;
}

int  rtl_set_igmpQueryEnabled(int enabled)
{

	int ret=FAILED;

#if defined(CONFIG_RTL_819X_SWCORE)
	if(igmpsnoopenabled)
#endif
	{
		igmpQueryEnabled=enabled;
		ret = SUCCESS;
	}	
	
	return ret;
	
}
#if defined (CONFIG_RTL_MLD_SNOOPING)

int rtl_get_mldQueryEnabled(void)
{
	int enabled=0;
	
	if(mldSnoopEnabled)
		enabled=mldQueryEnabled;
	
	return enabled;
}

int  rtl_set_mldQueryEnabled(int enabled)
{

	int ret=FAILED;

	if(mldSnoopEnabled)
	{
		mldQueryEnabled=enabled;
		ret = SUCCESS;
	}	
	
	return ret;
	
}
#endif
extern int32 rtl_getGroupNum(uint32 ipVersion);

void br_igmpQueryTimerExpired(void * para)
{
	struct mbuf *m=NULL;
	struct bridge_softc *sc=(struct bridge_softc *)para;
    unsigned char *igmpBuf=NULL;
	struct mbuf *m0=NULL;
	struct bridge_iflist *p;
	struct mbuf *mc;
	int used = 0;
	int s;
	struct igmp *igmp;
    struct ip *ip;
	struct ether_header * ether;
	int len;

	if(igmpQueryEnabled==0)
	{
		return;
	}
	
	if(rtl_getGroupNum(IP_VERSION4)==0)
		return;
	
	if(sc == NULL){
		return;
	}	
	
	if(igmpVersion==3)
		len = 50;	
	else
		len = 42;
	
		
#if defined (CONFIG_RTL_QUERIER_SELECTION)
	if(br_querierSelection(br,4)==0)
	{
		return;
	}
#endif	
	MGETHDR(m, M_DONTWAIT, MT_HEADER);
	
	if (m == NULL)
	   return;
	
	igmpBuf=br_generateIgmpQuery(0,sc);
	
	if(igmpBuf==NULL)
	{
		return;
	}
	
	if (len <= MHLEN){
		MH_ALIGN(m, len);
		m->m_len=len;
		memcpy(m->m_data,igmpBuf,len);
	}	
	else{
		MGET(m0, M_DONTWAIT, MT_DATA);
		if (m0 == NULL) {
			m_freem(m);
			return;
		}	
		memcpy(m0->m_dat,igmpBuf,len);
		m0->m_flags &= (~M_PKTHDR);
		m0->m_len=len;
		m->m_next = m0;
		m->m_len=0;
	}
	
	m->m_pkthdr.len = len;
	
	
#if 0
	diag_printf("[%s]:[%d].M:%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x\n",__FUNCTION__,__LINE__,
	*(m->m_data),*(m->m_data+1),*(m->m_data+2),*(m->m_data+3),*(m->m_data+4),*(m->m_data+5),
	*(m->m_data+6),*(m->m_data+7),*(m->m_data+8),*(m->m_data+9),*(m->m_data+10),*(m->m_data+11),
	*(m->m_data+12),*(m->m_data+13),*(m->m_data+14),*(m->m_data+15),*(m->m_data+16),*(m->m_data+17));
	diag_printf("m->m_pkthdr.len :%d,m->m_len:%d,[%s]:[%d].\n",m->m_pkthdr.len,m->m_len,__FUNCTION__,__LINE__);
#endif
	
	s = splimp();
	for (p = LIST_FIRST(&sc->sc_iflist); p != NULL;p = LIST_NEXT(p, next)) 
	{
		if(!p->ifp) 	
			continue;
		
		if((strncmp(p->ifp->if_name, "peth",4)==0)|| (strncmp(p->ifp->if_name, "pwlan",5)==0))
			continue;
		
		if ((p->ifp->if_flags & IFF_RUNNING) == 0)
				continue;
		
		
		if (IF_QFULL(&p->ifp->if_snd)) {
			sc->sc_if.if_oerrors++;
			continue;
		}

		if (LIST_NEXT(p, next) == NULL) {
			used = 1;
			mc = m;
		} else {
			mc = m_copym(m, 0, M_COPYALL, M_NOWAIT);
			if (mc == NULL) {
				sc->sc_if.if_oerrors++;
				continue;
			}
		}
		sc->sc_if.if_opackets++;
		sc->sc_if.if_obytes += m->m_pkthdr.len;
					// Also count the bytes in the outgoing interface; normally
					// done in if_ethersubr.c but here we bypass that route.
					p->ifp->if_obytes += m->m_pkthdr.len;
		//diag_printf("p->ifp:%s%d,%x[%s]:[%d].\n",p->ifp->if_name,p->ifp->if_unit,p->ifp->if_index,__FUNCTION__,__LINE__); 		
		IF_ENQUEUE(&p->ifp->if_snd, mc);
		if ((p->ifp->if_flags & IFF_OACTIVE) == 0)
			(*p->ifp->if_start)(p->ifp);
	}
	if (!used)
		m_freem(m);
	
	splx(s);
	return ;
}
#if defined (CONFIG_RTL_MLD_SNOOPING)

void br_mldQueryTimerExpired(void * para)
{
	struct mbuf *m=NULL;
	struct mbuf *m0=NULL;
	struct bridge_softc *sc=(struct bridge_softc *)para;
    unsigned char *mldBuf=NULL;
	
	struct bridge_iflist *p;
	struct mbuf *mc;
	int used = 0;
	int s;
	struct igmp *igmp;
    struct ip *ip;
	struct ether_header * ether;
	int len;
	unsigned char *data=NULL;

	if(mldQueryEnabled==0)
	{
		return;
	}
	
	if(rtl_getGroupNum(IP_VERSION6)==0)
		return;
	
	if(sc == NULL)
		return;
	if(mldVersion==2)
		len = 90;
	else
		len = 86;
		
#if defined (CONFIG_RTL_QUERIER_SELECTION)
	if(br_querierSelection(br,4)==0)
	{
		return;
	}
#endif	
	MGETHDR(m, M_DONTWAIT, MT_HEADER);

	if (m == NULL)
	   return;

	m->m_len = 0 ;
	mldBuf=br_generateMldQuery(0,sc);
	
	if(mldBuf==NULL)
	{
		return;	
	}

	if (len <= MHLEN){
		MH_ALIGN(m, len);
		m->m_len=len;
		memcpy(m->m_data,mldBuf,len);
	}	
	else {
		MGET(m0, M_DONTWAIT, MT_DATA);
		if (m0 == NULL) {
			m_freem(m);
			return;
		}	
		memcpy(m0->m_dat,mldBuf,len);
		m0->m_flags &= (~M_PKTHDR);
		m0->m_len=len;
		m->m_next = m0;
		m->m_len=0;
	}
	m->m_pkthdr.len = len;
	
#if 0
	diag_printf("[%s]:[%d].M:%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x\n",__FUNCTION__,__LINE__,
	*(m->m_data),*(m->m_data+1),*(m->m_data+2),*(m->m_data+3),*(m->m_data+4),*(m->m_data+5),
	*(m->m_data+6),*(m->m_data+7),*(m->m_data+8),*(m->m_data+9),*(m->m_data+10),*(m->m_data+11),
	*(m->m_data+12),*(m->m_data+13),*(m->m_data+14),*(m->m_data+15),*(m->m_data+16),*(m->m_data+17));
	diag_printf("m->m_pkthdr.len :%d,m->m_len:%d,[%s]:[%d].\n",m->m_pkthdr.len,m->m_len,__FUNCTION__,__LINE__);
#endif
	
	s = splimp();
	for (p = LIST_FIRST(&sc->sc_iflist); p != NULL;p = LIST_NEXT(p, next)) 
	{
		
		if(!p->ifp)	
			continue;
		
		if ((strncmp(p->ifp->if_xname, "peth",4)==0) || (strncmp(p->ifp->if_xname, "pwlan",5)==0))
			continue;
		
		if ((p->ifp->if_flags & IFF_RUNNING) == 0)
				continue;
		
		
		if (IF_QFULL(&p->ifp->if_snd)) {
			sc->sc_if.if_oerrors++;
			continue;
		}

		if (LIST_NEXT(p, next) == NULL) {
			used = 1;
			mc = m;
		} else {
			mc = m_copym(m, 0, M_COPYALL, M_NOWAIT);
			if (mc == NULL) {
				sc->sc_if.if_oerrors++;
				continue;
			}
		}
		sc->sc_if.if_opackets++;
		sc->sc_if.if_obytes += m->m_pkthdr.len;
					// Also count the bytes in the outgoing interface; normally
					// done in if_ethersubr.c but here we bypass that route.
					p->ifp->if_obytes += m->m_pkthdr.len;
		//diag_printf("p->ifp:%s%d[%s]:[%d].\n",p->ifp->if_name,p->ifp->if_unit,__FUNCTION__,__LINE__); 		
		IF_ENQUEUE(&p->ifp->if_snd, mc);
		if ((p->ifp->if_flags & IFF_OACTIVE) == 0)
			(*p->ifp->if_start)(p->ifp);
	}
	
	if (!used)
		m_freem(m);
	
	splx(s);

	return;
}	
#endif
void bridge_mCastQueryTimerExpired(void * para)
{

	if((igmpQueryEnabled==0)&&(mldQueryEnabled==0))
	{
		return;
	}
	
	if(mCastQueryTimerCnt%2==0)
		br_igmpQueryTimerExpired(para);
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	else	
		br_mldQueryTimerExpired(para);
	#endif
	
	mCastQueryTimerCnt++;	
	
	callout_reset(&mCastQuerytimer,(MCAST_QUERY_INTERVAL* hz) ,bridge_mCastQueryTimerExpired, para);
	
	return;
}
#endif

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
//uint32 br0SwFwdPortMask;
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
extern unsigned int rtl_getPortMaskByDevName(unsigned char *name);
extern int rtl_getVlanPortPvidByDevName(unsigned char *name);

int rtl_getNicFwdPortMask(struct bridge_softc *br_sc, unsigned int brFwdPortMask, unsigned int *NicFwdPortMask, uint32 vlanId)
{
	int ret = FAILED;
	struct bridge_iflist  *p, *n;
	unsigned short port_bitmask=0;
	struct net_device *dev=NULL; 
	struct eth_drv_sc *drv_sc=NULL;
	Rltk819x_t *rltk819x_info=NULL;
	unsigned int portMask;
	
	for (p = LIST_FIRST(&br_sc->sc_iflist); p != NULL;
		     p = LIST_NEXT(p, next))
	
	{
		port_bitmask = (1 << p->ifp->if_index);
        if (port_bitmask & brFwdPortMask) {
			if(strncmp(p->ifp->if_name,"wlan",4)==0)
				continue;
			drv_sc= (struct eth_drv_sc *)((p->ifp)->if_softc);
			rltk819x_info = (Rltk819x_t *)(drv_sc->driver_private);
			dev = (struct net_device *)(rltk819x_info->dev);
			if(dev==NULL)
				continue;
			
			if((vlanId!=0)&&(vlanId == rtl_getVlanPortPvidByDevName(dev->name)))
			{
				portMask=rtl_getPortMaskByDevName(dev->name);
				*NicFwdPortMask |= portMask;
				ret = SUCCESS;
			}
       }
	}

	return ret;
}
unsigned int rtl_getNicFwdPortMaskofBr0(unsigned int brFwdPortMask)
{
	struct bridge_softc *br_sc=&bridgectl[0];
	unsigned int NicFwdPortMask=0;
	uint32 vlanId=0;
	int ret;
	ret = rtl_getNicFwdPortMask(br_sc, brFwdPortMask, &NicFwdPortMask,  vlanId);
	return NicFwdPortMask;	
}

#endif
int rtl865x_ipMulticastHardwareAccelerate(struct bridge_softc *br_sc, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr,unsigned int vlanId)
{
	int ret;
	//int fwdDescCnt;
	//unsigned short port_bitmask=0;

	unsigned int tagged_portmask=0;


	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo  multicastFwdInfo;
	
	rtl865x_tblDrv_mCast_t * existMulticastEntry;
	rtl865x_mcast_fwd_descriptor_t  fwdDescriptor;
	//uint32 vlanId=0;
	unsigned int srcNicPort;
	#if 0//defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
	if(m&&(m->m_pkthdr.tag.f.tpid == htons(ETH_P_8021Q))){
		vlanId = ntohs(m->m_pkthdr.tag.f.pci&0xfff);
		//diag_printf("vlanId:%d,[%s]:[%d].\n",vlanId,__FUNCTION__,__LINE__);
	}	
	
	#endif
	#if 0
	diag_printf("%s:%d,srcPort is %d,srcVlanId is %d,srcIpAddr is 0x%x,destIpAddr is 0x%x,vlanId:%d,\n",__FUNCTION__,__LINE__,srcPort,srcVlanId,srcIpAddr,destIpAddr,vlanId);
	#endif
	
	if(strcmp(br_sc->sc_if.if_xname,RTL_PS_BR0_DEV_NAME)!=0)
	{
		return -1;
	}

	if(brFwdPortMask & br0SwFwdPortMask)
	{
		return -1;
	}
	existMulticastEntry=rtl865x_findMCastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort);
	if(existMulticastEntry!=NULL)
	{
		/*it's already in cache */
		return 0;

	}

	if(brFwdPortMask==0)
	{
		rtl865x_blockMulticastFlow(srcVlanId, srcPort, srcIpAddr, destIpAddr);
		return 0;
	}
	
	multicastDataInfo.ipVersion=4;
	multicastDataInfo.sourceIp[0]=  srcIpAddr;
	multicastDataInfo.groupAddr[0]=  destIpAddr;

	/*add hardware multicast entry*/
#if defined(CONFIG_RTL_VLAN_SUPPORT)
#if defined(CONFIG_RTL_819X_SWCORE)	
	if(rtl_vlan_support_enable == 0)
	{
		memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
		strcpy(fwdDescriptor.netifName,"eth*");
		fwdDescriptor.fwdPortMask=0xFFFFFFFF;
		ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
		if(ret!=0)
		{
			return -1;
		}
		else
		{
			if(multicastFwdInfo.cpuFlag)
			{
				fwdDescriptor.toCpu=1;
			}
			fwdDescriptor.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<srcPort));
		}
	}
	else
	{
		#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
		memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
		strcpy(fwdDescriptor.netifName,"bridge0");
		fwdDescriptor.fwdPortMask = 0;
	
		ret = rtl_getNicFwdPortMask(br_sc, brFwdPortMask, &fwdDescriptor.fwdPortMask,vlanId);
		if(ret == SUCCESS){
			fwdDescriptor.fwdPortMask = fwdDescriptor.fwdPortMask & (~(1<<srcPort));
		}
		else
		{
			return -1;
		}
		#if 0
		ret=FAILED;
		ret = rtl_getNicFwdPortMask(br_sc, srcPort, &srcNicPort,vlanId);
		if(ret == SUCCESS)
		{
			srcPort=srcNicPort;
		}
		#endif	
		#endif
	}
#endif
#else
	memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
	strcpy(fwdDescriptor.netifName,"eth*");
	fwdDescriptor.fwdPortMask=0xFFFFFFFF;
	#if defined(CONFIG_RTL_819X_SWCORE)	
	ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
	#endif
	if(ret!=0)
	{
		return -1;
	}
	else
	{
		if(multicastFwdInfo.cpuFlag)
		{
			fwdDescriptor.toCpu=1;
		}
		fwdDescriptor.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<srcPort));
	}
#endif
#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if(rtl_hw_vlan_ignore_tagged_mc == 1)
		tagged_portmask = rtl_hw_vlan_get_tagged_portmask();
#endif
	if((fwdDescriptor.fwdPortMask & tagged_portmask) == 0)
	{

		//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
		ret=rtl865x_addMulticastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort,
							&fwdDescriptor, 1, 0, 0, 0);
	}

	return 0;
}

int rtl865x_generateBridgeDeviceInfo( struct bridge_softc *sc, rtl_multicastDeviceInfo_t *devInfo)
{
	struct bridge_iflist *p; 
	
	if((sc==NULL) || (devInfo==NULL))
	{
		return -1;
	}
	
	memset(devInfo, 0, sizeof(rtl_multicastDeviceInfo_t));

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)!=0 && strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)!=0 )
#else
	if(strcmp(sc->sc_if.if_xname,RTL_PS_BR0_DEV_NAME)!=0)
#endif
	{
		return -1;
	}

	strcpy(devInfo->devName,sc->sc_if.if_xname);
	
	for (p = LIST_FIRST(&sc->sc_iflist); p != NULL;
		     p = LIST_NEXT(p, next))	
	{
		//diag_printf("[%s]:[%d].bif_port_id:%x,bif_designated_port:%x,flag:%x,ifindex:%d,name:%s%d,\n",__FUNCTION__,__LINE__,
		//	p->bif_port_id,p->bif_designated_port,p->bif_flags,(p->ifp->if_index),p->ifp->if_xname,p->ifp->if_unit);
		if(memcmp(p->ifp->if_xname, RTL_PS_ETH_NAME,3)!=0)
		{
			devInfo->swPortMask|=(1<<p->ifp->if_index);
		}
		devInfo->portMask|=(1<<p->ifp->if_index);
		
	}
	
	return 0;
}

#endif

#if defined(CONFIG_RTL_MLD_SNOOPING) && defined(CONFIG_RTL_HARDWARE_MULTICAST) && defined(CONFIG_RTL_8197F)
int rtl819x_ipv6MulticastHardwareAccelerate(struct bridge_softc *br_sc, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int *srcIpAddr, unsigned int *destIpAddr,unsigned int vlanId)
{
	int ret;
	//int fwdDescCnt;
	//unsigned short port_bitmask=0;

	unsigned int tagged_portmask=0;


	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo  multicastFwdInfo;
	
	rtl819x_tblDrv_mCastv6_t * existMulticastEntry;
	rtl819x_mcast_fwd_descriptor6_t  fwdDescriptor;
	//uint32 vlanId=0;
	unsigned int srcNicPort;
	#if 0//defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
	if(m&&(m->m_pkthdr.tag.f.tpid == htons(ETH_P_8021Q))){
		vlanId = ntohs(m->m_pkthdr.tag.f.pci&0xfff);
		//diag_printf("vlanId:%d,[%s]:[%d].\n",vlanId,__FUNCTION__,__LINE__);
	}	
	
	#endif

#if 0
	diag_printf("%s:%d,srcPort is %d,srcVlanId is %d,srcIpAddr is 0x%x-%x-%x-%x,destIpAddr is 0x%x-%x-%x-%x,vlanId:%d,	brFwdPortMask:%x\n",__FUNCTION__,__LINE__,srcPort,srcVlanId,srcIpAddr[0],srcIpAddr[1],srcIpAddr[2],srcIpAddr[3],destIpAddr[0],destIpAddr[1],destIpAddr[2],destIpAddr[3],vlanId, brFwdPortMask);
#endif

	
	if(strcmp(br_sc->sc_if.if_xname,RTL_PS_BR0_DEV_NAME)!=0)
	{
		return -1;
	}

	if(brFwdPortMask & br0SwFwdPortMask)
	{
		return -1;
	}
	existMulticastEntry=rtl819x_findMCastv6Entry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort);
	if(existMulticastEntry!=NULL)
	{
		/*it's already in cache */
		return 0;

	}

	if(brFwdPortMask==0)
	{
		rtl819x_blockMulticastv6Flow(srcVlanId, srcPort, srcIpAddr, destIpAddr);
		return 0;
	}
	
	multicastDataInfo.ipVersion=6;
	memcpy(multicastDataInfo.sourceIp, srcIpAddr, 4*sizeof(unsigned int));
	memcpy(multicastDataInfo.groupAddr, destIpAddr, 4*sizeof(unsigned int));

	/*add hardware multicast entry*/
#if defined(CONFIG_RTL_VLAN_SUPPORT)
#if defined(CONFIG_RTL_819X_SWCORE)	
	if(rtl_vlan_support_enable == 0)
	{
		memset(&fwdDescriptor, 0, sizeof(rtl819x_mcast_fwd_descriptor6_t ));
		strcpy(fwdDescriptor.netifName,"eth*");
		fwdDescriptor.fwdPortMask=0xFFFFFFFF;
		ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
		if(ret!=0)
		{
			return -1;
		}
		else
		{
			if(multicastFwdInfo.cpuFlag)
			{
				fwdDescriptor.toCpu=1;
			}
			fwdDescriptor.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<srcPort));
		}
	}
	else
	{
		#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
		memset(&fwdDescriptor, 0, sizeof(rtl819x_mcast_fwd_descriptor6_t ));
		strcpy(fwdDescriptor.netifName,"bridge0");
		fwdDescriptor.fwdPortMask = 0;
	
		ret = rtl_getNicFwdPortMask(br_sc, brFwdPortMask, &fwdDescriptor.fwdPortMask,vlanId);
		if(ret == SUCCESS){
			fwdDescriptor.fwdPortMask = fwdDescriptor.fwdPortMask & (~(1<<srcPort));
		}
		else
		{
			return -1;
		}
		#if 0
		ret=FAILED;
		ret = rtl_getNicFwdPortMask(br_sc, srcPort, &srcNicPort,vlanId);
		if(ret == SUCCESS)
		{
			srcPort=srcNicPort;
		}
		#endif	
		#endif
	}
#endif
#else
	memset(&fwdDescriptor, 0, sizeof(rtl819x_mcast_fwd_descriptor6_t ));
	strcpy(fwdDescriptor.netifName,"eth*");
	fwdDescriptor.fwdPortMask=0xFFFFFFFF;
	#if defined(CONFIG_RTL_819X_SWCORE)	
	ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
	#endif

	#if 0
	diag_printf("ret:%d, cpu:%d, fwdPortmask:%x, [%s:%d]\n", ret, multicastFwdInfo.cpuFlag, multicastFwdInfo.fwdPortMask, __FUNCTION__, __LINE__);
	#endif
	
	if(ret!=0)
	{
		return -1;
	}
	else
	{
		if(multicastFwdInfo.cpuFlag)
		{
			fwdDescriptor.toCpu=1;
		}
		fwdDescriptor.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<srcPort));
	}
#endif
#if 0//defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if(rtl_hw_vlan_ignore_tagged_mc == 1)
		tagged_portmask = rtl_hw_vlan_get_tagged_portmask();
#endif
	if((fwdDescriptor.fwdPortMask & tagged_portmask) == 0)
	{
		ret=rtl819x_addMulticastv6Entry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort,
							&fwdDescriptor, 1, 0, 0, 0);
	}

	return 0;
}


#endif
#endif//CONFIG_RTL_IGMP_SNOOPING


#if defined(CONFIG_RTL_LAYERED_DRIVER_L2)&&defined(CONFIG_RTL_HARDWARE_NAT)
int rtl_bridge_rtnode_cleanup_hooks(struct bridge_rtnode *n)
{
	int ret = FAILED;
	int hw_aging;
	int port_num = -100;
	
	hw_aging = rtl865x_arrangeFdbEntry(n->brt_addr.octet, &port_num);
	switch (hw_aging) {
		case RTL865X_FDBENTRY_450SEC:
		case RTL865X_FDBENTRY_300SEC:
		case RTL865X_FDBENTRY_150SEC:	
			//n->brt_age = 1;
			ret = SUCCESS;
			break;
		case RTL865X_FDBENTRY_TIMEOUT:
		case FAILED:
		default:
			break;					
	}

	return ret;
}
				
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
#include "../../../../../../devs/eth/rltk/819x/switch/v3_0/src/rtl_types.h"
extern int rtl_getPassthruMask(void);

int32 rtl_checkisPassthruFrame(const struct mbuf * m)
{
	int	ret;	
	int passthruMask=rtl_getPassthruMask();
	ret = FAILED;
    struct ether_header *eh = mtod(m, struct ether_header *);
	
	if (passthruMask)
	{
		if (passthruMask&IP6_PASSTHRU_MASK)
		{
			
			if (eh->ether_type ==  ETH_P_IPV6)		
			{
				ret = SUCCESS;
			}
		}
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if (passthruMask&PPPOE_PASSTHRU_MASK)
		{
			if ((eh->ether_type == __constant_htons(ETH_P_PPP_SES))||(eh->ether_type ==__constant_htons(ETH_P_PPP_DISC))) 
				
			{
				ret = SUCCESS;
			}
		}
		#endif
	}

	return ret;
}
#endif

#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
inline int should_deliver(const struct ifnet *ifp, const struct mbuf * m)
{
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)	
	struct ether_header *eh = mtod(m, struct ether_header *);
	struct bridge_softc *sc = (struct bridge_softc *)ifp->if_bridge;
#endif
     unsigned int forwarding_rule_dst = 0, forwarding_rule_org = 0;
     unsigned char ethname[5] = {0}; 
     forwarding_rule_org = m->m_pkthdr.forward_rule;
     sprintf(ethname,"%s%d",ifp->if_name, ifp->if_unit);
     if (!strcmp(ethname, "eth7"))
       forwarding_rule_dst = 2;
	 #if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	 else if (!strcmp(ethname, "peth0"))
		 forwarding_rule_dst = 1;
	 #endif
     else
    {
        unsigned short dstvid = rtl_getVlanPortPvidByDevName(ethname);
        rtl_getRtlVlanForwardRule(dstvid ,&forwarding_rule_dst);
    }
     
   //diag_printf("%s %d forwarding_rule_org %d forwarding_rule_dst %d m->m_pkthdr.index %d flag_src %d ethname %s \n", __FUNCTION__, __LINE__, forwarding_rule_org, forwarding_rule_dst, m->m_pkthdr.index, m->m_pkthdr.flag_src,ethname);
     if(m->m_pkthdr.flag_src)
     {
     
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
		   /*ipv6 known unicast from lan not check forwarding rule*/
		if((rtl_checkisPassthruFrame(m)==SUCCESS)&&((eh->ether_dhost[0]&0x1)==0)
			&&(bridge_rtlookup(sc, eh->ether_dhost)!=NULL)) 
		 {
			 
			 //diag_printf("%s %d \n", __FUNCTION__, __LINE__);
			 return 1;
		 }
		#endif	
		{
	         /* index == 1, it means skb is cloned skb in rx_vlan_process */
	         if (m->m_pkthdr.index) {
	             if (forwarding_rule_dst != 2)
	                 return 0;
	         }
	         
	         /* vlan_br can't send packet to vlan_nat */
	         if (forwarding_rule_org == 2) {
	             if (forwarding_rule_dst == 1)
	                 return 0;
	         }
	         
	         /* vlan_nat can't send packet to vlan_br */
	         if (forwarding_rule_org == 1) {
	             if (forwarding_rule_dst == 2)
	                 return 0;
	        } 
		}
    }
    #ifdef CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT
    if(m->m_pkthdr.rcvif && ifp->zone_type!=m->m_pkthdr.rcvif->zone_type)
        return 0;
    #endif
    
    return 1; 
}
#endif


