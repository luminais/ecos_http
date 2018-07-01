//==========================================================================
//
//      src/sys/netinet/if_ether.c
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
 * Copyright (c) 1982, 1986, 1988, 1993
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
 *	@(#)if_ether.c	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/if_ether.c,v 1.64.2.11 2001/07/25 17:27:56 jlemon Exp $
 */

/*
 * Ethernet address resolution protocol.
 * TODO:
 *	add "inuse/lock" bit (or ref. count) along with valid bit
 */

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/sysctl.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <net/netisr.h>
#include <net/if_llc.h>
#if NBRIDGE > 0
#include <net/ethernet.h>
#include <net/if_bridge.h>
#endif

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>

#include <net/iso88025.h>

#ifdef CONFIG_RTL_VLAN_SUPPORT
#include <netinet/rtl_vlan.h>
static struct vlan_tag arp_tag;
#endif

//#include <netinet/fastpath/rtl_types.h>
#include <switch/rtl865x_arp_api.h>

#ifndef ETH_ALEN
#define ETH_ALEN		6		/* Octets in one ethernet addr   */
#endif

#define SIN(s) ((struct sockaddr_in *)s)
#define SDL(s) ((struct sockaddr_dl *)s)

SYSCTL_DECL(_net_link_ether);
SYSCTL_NODE(_net_link_ether, PF_INET, inet, CTLFLAG_RW, 0, "");

#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
/* change by liuchengchi 2015-07-21 : timer values */
static int arpt_prune = (2 * 60); /* walk list every 2 minutes */
#define NUD_REACHABLE		0x1
#define NUD_DELAY			0x2
#define NUD_FAILED			0x4
/* end change */
#else
/* timer values */
/*lqz add for arp update for 5 minutes*/
static int arpt_prune = (3 * 60); /* walk list every 3 minutes */
#endif
static int arpt_keep = (2 * 60); /* once resolved, good for 2 more minutes */
static int arpt_down = 20;	/* once declared down, don't send for 20 sec */

SYSCTL_INT(_net_link_ether_inet, OID_AUTO, prune_intvl, CTLFLAG_RW,
	   &arpt_prune, 0, "");
SYSCTL_INT(_net_link_ether_inet, OID_AUTO, max_age, CTLFLAG_RW, 
	   &arpt_keep, 0, "");
SYSCTL_INT(_net_link_ether_inet, OID_AUTO, host_down_time, CTLFLAG_RW,
	   &arpt_down, 0, "");

#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
#define RTL_HOLD_LIST_NUM_MAX 8
#endif

#define	rt_expire rt_rmx.rmx_expire

struct llinfo_arp {
	LIST_ENTRY(llinfo_arp) la_le;
	struct	rtentry *la_rt;
	struct	mbuf *la_hold;		/* last packet until resolved/timeout */
	long	la_asked;		/* last time we QUERIED for this addr */
#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
	int hold_num;
#endif
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__	
	int		flag;
#endif
#define la_timer la_rt->rt_rmx.rmx_expire /* deletion time in seconds */
};

static	LIST_HEAD(, llinfo_arp) llinfo_arp;

struct	ifqueue arpintrq = {0, 0, 0, 16};
static int	arp_inuse, arp_allocated;

// Description: The following code is used to cli command of tenda arp, Ported by zhuhuan on 2015.12.29
//roy+++++,for lan port arp count
static int	lan_arp_inuse;
// +++++

static int	arp_maxtries = 5;
static int	useloopback = 1; /* use loopback interface for local traffic */
static int	arp_proxyall = 0;

#ifdef CONFIG_RTL_819X
//static struct in_addr tip_dhcp_arp;
//static char smac_dhcp_arp[6];
//static int res_dhcp_arp=0;
//static int flag_dhcp_arp=0;


typedef enum  
{
	DHCPC_SEND_ARP=0, 
	DHCPD_SEND_ARP=1
}RTL_ARP_TYPE;



#define MAX_RTL_ARP_TYPE 5

struct rtl_arp_arg
{
	struct in_addr arp_sip;	
	u_char arp_smac[6];	
	struct in_addr arp_tip;	

	int arp_res;
	int arp_flag;	
};

static struct rtl_arp_arg rtl_arp_array[MAX_RTL_ARP_TYPE];
#endif
SYSCTL_INT(_net_link_ether_inet, OID_AUTO, maxtries, CTLFLAG_RW,
	   &arp_maxtries, 0, "");
SYSCTL_INT(_net_link_ether_inet, OID_AUTO, useloopback, CTLFLAG_RW,
	   &useloopback, 0, "");
SYSCTL_INT(_net_link_ether_inet, OID_AUTO, proxyall, CTLFLAG_RW,
	   &arp_proxyall, 0, "");

static void	arp_rtrequest __P((int, struct rtentry *, struct sockaddr *));
static void	arprequest __P((struct arpcom *,
			struct in_addr *, struct in_addr *, u_char *));
static void	arpintr __P((void));
static void	arptfree __P((struct llinfo_arp *));
static void	arptimer __P((void *));
struct llinfo_arp
		*arplookup __P((u_long, int, int));
#ifdef INET
static void	in_arpinput __P((struct mbuf *));
#endif

extern int tenda_arp_update_node(
unsigned char *mac
, in_addr_t ip
, int peration 
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
, u_long rmx_expire
#endif
);
extern void tenda_arp_delete_expired_list(void);

typedef void pr_fun(char *fmt, ...);

#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
int rtl_wanip_same_with_gwip_support = 0;
unsigned int is_dhcp_wantype = 0;
void rtl_set_is_dhcp_wantype(unsigned int value)
{
		is_dhcp_wantype = value;
}
unsigned int get_interface_ip_for_same(char *interface_name)
{
		unsigned int ipaddr = 0;
		#include <sys/sockio.h>
		int fd;
		struct ifreq ifr;
		fd = socket(AF_INET, SOCK_DGRAM, 0);
		ifr.ifr_addr.sa_family = AF_INET;
		strncpy(ifr.ifr_name, interface_name, IFNAMSIZ-1);
		ioctl(fd, SIOCGIFADDR, &ifr);
		close(fd);
		ipaddr = *(unsigned int *)(&(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
		return ipaddr;
}
int check_wan_gw_same_ip(void)
{
    if(is_dhcp_wantype == 1)
	{
		register struct rtentry *rt;
		unsigned int ipaddr_gw = 0;
		unsigned int ipaddr_wan = 0;
		struct  sockaddr *rt_gateway;   
		struct sockaddr_in sin = {sizeof(sin), AF_INET };
		rt = rtalloc1((struct sockaddr *)&sin, 0UL, 0UL); 
		if (rt&&rt->rt_flags & RTF_GATEWAY){
		    rt_gateway = rt->rt_gateway; //get default gateway
			if(rt_gateway){
				ipaddr_gw = *(unsigned int *)(&(((struct sockaddr_in *)rt_gateway)->sin_addr));
				ipaddr_wan = get_interface_ip_for_same("eth1");
				if(ipaddr_gw == ipaddr_wan){
				    rtl_wanip_same_with_gwip_support  = 1;
					return 1;
				}else{
				    rtl_wanip_same_with_gwip_support  = 0;
					return 0;
				}
			}
		}
	}
	return 0;
}
#endif

void _show_arp(pr_fun *pr)
{
	register struct llinfo_arp *la = llinfo_arp.lh_first;
	struct sockaddr_dl *sdl = NULL;
	unsigned char mac[ETH_ALEN] = {0};
	struct llinfo_arp *ola;
	while ((ola = la) != 0) {
		register struct rtentry *rt = la->la_rt;
		la = la->la_le.le_next;
		if (rt->rt_gateway ){
			sdl = SDL(rt->rt_gateway);
			if(sdl->sdl_family == AF_LINK && sdl->sdl_alen == ETH_ALEN) {
				(void)memcpy(mac, LLADDR(sdl), ETH_ALEN);
				(*pr)("%s	%02x:%02x:%02x:%02x:%02x:%02x\n",inet_ntoa(SIN(rt_key(rt))->sin_addr),
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			}	
		}
	}
	return 0;
}

/*
 * Timeout routine.  Age arp_tab entries periodically.
 */
/* ARGSUSED */
static void
arptimer(ignored_arg)
	void *ignored_arg;
{
	int s = splnet();
	register struct llinfo_arp *la = llinfo_arp.lh_first;
	struct llinfo_arp *ola;

	timeout(arptimer, (caddr_t)0, arpt_prune * hz);
	while ((ola = la) != 0) {
		register struct rtentry *rt = la->la_rt;
		la = la->la_le.le_next;
		
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		if (rt->rt_expire && rt->rt_expire < time_second && ola->flag == NUD_REACHABLE ){
			ola->flag = NUD_DELAY;
			arprequest((struct arpcom *)rt->rt_ifp,
			     &SIN(rt->rt_ifa->ifa_addr)->sin_addr,
			    &SIN(rt_key(rt))->sin_addr,
			    ((struct arpcom *)rt->rt_ifp)->ac_enaddr);
			
		}else if(ola->flag == NUD_DELAY || ola->flag == NUD_FAILED){
#else
		if (rt->rt_expire && rt->rt_expire <= time_second){
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)&&defined(CONFIG_RTL_HARDWARE_NAT)
			if(rtl865x_arpSync(ntohl(SIN(rt_key(rt))->sin_addr.s_addr), 0)>0)
				continue;
#endif
			arptfree(ola); /* timer has expired, clear */
		}

		#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)&&defined(CONFIG_RTL_HARDWARE_NAT)
		else if (rt->rt_expire) {
			rtl865x_updateFdbByArp(ntohl(SIN(rt_key(rt))->sin_addr.s_addr));
		}
		#endif

	}
	splx(s);
	
	/* 
	 * Description: delete the expired_list created by the function of tenda_arp_delete_node
	 * Author: zhuhuan
	 * Date: 2016.03.19
	 */
	 //不移除expired_list的结点
	// tenda_arp_delete_expired_list();
}

/*
 * Parallel to llc_rtrequest.
 */
static void
arp_rtrequest(req, rt, sa)
	int req;
	register struct rtentry *rt;
	struct sockaddr *sa;
{
	register struct sockaddr *gate = rt->rt_gateway;
	register struct llinfo_arp *la = (struct llinfo_arp *)rt->rt_llinfo;
	static struct sockaddr_dl null_sdl = {sizeof(null_sdl), AF_LINK};
	static int arpinit_done;
	#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
	struct mbuf *m_tmp;
	#endif

	if (!arpinit_done) {
		arpinit_done = 1;
		LIST_INIT(&llinfo_arp);
		timeout(arptimer, (caddr_t)0, hz);
		register_netisr(NETISR_ARP, arpintr);
	}
#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
	if ((rt->rt_flags & RTF_GATEWAY) && req != RTM_RESOLVE)
	//if ((rt->rt_flags & RTF_GATEWAY) &&req != RTM_RESOLVE && !check_wan_gw_same_ip())
			return;
#else
	if (rt->rt_flags & RTF_GATEWAY)
		return;
#endif
	switch (req) {

	case RTM_ADD:
		/*
		 * XXX: If this is a manually added route to interface
		 * such as older version of routed or gated might provide,
		 * restore cloning bit.
		 */
		if ((rt->rt_flags & RTF_HOST) == 0 &&
		    SIN(rt_mask(rt))->sin_addr.s_addr != 0xffffffff)
			rt->rt_flags |= RTF_CLONING;
		if (rt->rt_flags & RTF_CLONING) {
			/*
			 * Case 1: This route should come from a route to iface.
			 */
			rt_setgate(rt, rt_key(rt),
					(struct sockaddr *)&null_sdl);
			gate = rt->rt_gateway;
			SDL(gate)->sdl_type = rt->rt_ifp->if_type;
			SDL(gate)->sdl_index = rt->rt_ifp->if_index;
			rt->rt_expire = time_second;
			break;
		}
		/* Announce a new entry if requested. */
		if (rt->rt_flags & RTF_ANNOUNCE)
			arprequest((struct arpcom *)rt->rt_ifp,
			    &SIN(rt_key(rt))->sin_addr,
			    &SIN(rt_key(rt))->sin_addr,
			    (u_char *)LLADDR(SDL(gate)));
		/*FALLTHROUGH*/
	case RTM_RESOLVE:
#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
		//if ((rt->rt_flags & RTF_GATEWAY) &&check_wan_gw_same_ip()) 
		//{
		//		gate = rt->rt_gwroute->rt_gateway;
		//}
		if ((rt->rt_flags & RTF_GATEWAY)) {
				if(check_wan_gw_same_ip())
						gate = rt->rt_gwroute->rt_gateway;
				else{
						return;
				}
		}
#endif
		if (gate->sa_family != AF_LINK ||
		    gate->sa_len < sizeof(null_sdl)) {
			log(LOG_DEBUG, "arp_rtrequest: bad gateway value\n");
			break;
		}
		SDL(gate)->sdl_type = rt->rt_ifp->if_type;
		SDL(gate)->sdl_index = rt->rt_ifp->if_index;
		if (la != 0)
			break; /* This happens on a route change */
		/*
		 * Case 2:  This route may come from cloning, or a manual route
		 * add with a LL address.
		 */
		R_Malloc(la, struct llinfo_arp *, sizeof(*la));
		
		if (la == 0) {
			log(LOG_DEBUG, "arp_rtrequest: malloc failed\n");
			break;
		}
		#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
		la->hold_num = 0;
		#endif

		rt->rt_llinfo = (caddr_t)la;

		// Description: The following code is used to cli command of tenda arp, Ported by zhuhuan on 2015.12.29
		//roy+++
		if(rt->rt_ifp->if_index == 2)
			lan_arp_inuse++;
		//+++
		
		arp_inuse++, arp_allocated++;
		Bzero(la, sizeof(*la));
		la->la_rt = rt;
		rt->rt_flags |= RTF_LLINFO;
		LIST_INSERT_HEAD(&llinfo_arp, la, la_le);
#ifdef INET
		/*
		 * This keeps the multicast addresses from showing up
		 * in `arp -a' listings as unresolved.  It's not actually
		 * functional.  Then the same for broadcast.
		 */
		if (IN_MULTICAST(ntohl(SIN(rt_key(rt))->sin_addr.s_addr))) {
			ETHER_MAP_IP_MULTICAST(&SIN(rt_key(rt))->sin_addr,
					       LLADDR(SDL(gate)));
			SDL(gate)->sdl_alen = 6;
			rt->rt_expire = 0;
		}
		if (in_broadcast(SIN(rt_key(rt))->sin_addr, rt->rt_ifp)) {
			memcpy(LLADDR(SDL(gate)), etherbroadcastaddr, 6);
			SDL(gate)->sdl_alen = 6;
			rt->rt_expire = 0;
		}
#endif

		if (SIN(rt_key(rt))->sin_addr.s_addr ==
		    (IA_SIN(rt->rt_ifa))->sin_addr.s_addr) {
		    /*
		     * This test used to be
		     *	if (loif.if_flags & IFF_UP)
		     * It allowed local traffic to be forced
		     * through the hardware by configuring the loopback down.
		     * However, it causes problems during network configuration
		     * for boards that can't receive packets they send.
		     * It is now necessary to clear "useloopback" and remove
		     * the route to force traffic out to the hardware.
		     */
			rt->rt_expire = 0;
			Bcopy(((struct arpcom *)rt->rt_ifp)->ac_enaddr,
				LLADDR(SDL(gate)), SDL(gate)->sdl_alen = 6);
			if (useloopback)
				rt->rt_ifp = loif;

		}
		break;

	case RTM_DELETE:
		if (la == 0)
			break;
		arp_inuse--;

		// Description: The following code is used to cli command of tenda arp, Ported by zhuhuan on 2015.12.29
		//roy+++
		if(rt->rt_ifp->if_index == 2)
			lan_arp_inuse--;
		//+++
		
#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)&&defined(CONFIG_RTL_HARDWARE_NAT)
		if((IN_MULTICAST(ntohl(SIN(rt_key(rt))->sin_addr.s_addr))==0)&&
			(in_broadcast(SIN(rt_key(rt))->sin_addr, rt->rt_ifp)==0)){
				rtl865x_delArp(ntohl(SIN(rt_key(rt))->sin_addr.s_addr));
		}
#endif
		LIST_REMOVE(la, la_le);
		rt->rt_llinfo = 0;
		rt->rt_flags &= ~RTF_LLINFO;
		#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
		while (la->la_hold) {
			m_tmp = la->la_hold->m_nextpkt;
			m_freem(la->la_hold);
			la->la_hold = m_tmp;
			la->hold_num--;
		}
		#else
		if (la->la_hold)
			m_freem(la->la_hold);
		#endif

		R_Free((caddr_t)la);
	}
}

/*
 * Broadcast an ARP request. Caller specifies:
 *	- arp header source ip address
 *	- arp header target ip address
 *	- arp header source ethernet address
 */
static void
arprequest(ac, sip, tip, enaddr)
	register struct arpcom *ac;
	register struct in_addr *sip, *tip;
	register u_char *enaddr;
{
	register struct mbuf *m;
	register struct ether_header *eh;
	register struct ether_arp *ea;
	struct sockaddr sa;
	static u_char	llcx[] = { 0x82, 0x40, LLC_SNAP_LSAP, LLC_SNAP_LSAP,
				   LLC_UI, 0x00, 0x00, 0x00, 0x08, 0x06 };

	if ((m = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL)
		return;
	m->m_pkthdr.rcvif = (struct ifnet *)0;
	#ifdef CONFIG_RTL_VLAN_SUPPORT
	m->m_pkthdr.tag.v = arp_tag.v;
	#endif
	switch (ac->ac_if.if_type) {
	case IFT_ISO88025:
		m->m_len = sizeof(*ea) + sizeof(llcx);
		m->m_pkthdr.len = sizeof(*ea) + sizeof(llcx);
		MH_ALIGN(m, sizeof(*ea) + sizeof(llcx));
		(void)memcpy(mtod(m, caddr_t), llcx, sizeof(llcx));
		(void)memcpy(sa.sa_data, etherbroadcastaddr, 6);
		(void)memcpy(sa.sa_data + 6, enaddr, 6);
		sa.sa_data[6] |= TR_RII;
		sa.sa_data[12] = TR_AC;
		sa.sa_data[13] = TR_LLC_FRAME;
		ea = (struct ether_arp *)(mtod(m, char *) + sizeof(llcx));
		bzero((caddr_t)ea, sizeof (*ea));
		ea->arp_hrd = htons(ARPHRD_IEEE802);
		break;
	case IFT_FDDI:
	case IFT_ETHER:
		/*
		 * This may not be correct for types not explicitly
		 * listed, but this is our best guess
		 */
	default:
	#if 0 //def CONFIG_RTL_819X
		if(flag_dhcp_arp)
		{
			tip_dhcp_arp.s_addr=tip->s_addr;
			memcpy(smac_dhcp_arp, enaddr, 6);
			res_dhcp_arp=0;
			flag_dhcp_arp=0;
		}
	#endif
		m->m_len = sizeof(*ea);
		m->m_pkthdr.len = sizeof(*ea);
		MH_ALIGN(m, sizeof(*ea));
		ea = mtod(m, struct ether_arp *);
		eh = (struct ether_header *)sa.sa_data;
		bzero((caddr_t)ea, sizeof (*ea));
		/* if_output will not swap */
		eh->ether_type = htons(ETHERTYPE_ARP);
		(void)memcpy(eh->ether_dhost, etherbroadcastaddr,
		    sizeof(eh->ether_dhost));
		ea->arp_hrd = htons(ARPHRD_ETHER);
		break;
	}
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);	/* hardware address length */
	ea->arp_pln = sizeof(ea->arp_spa);	/* protocol address length */
	ea->arp_op = htons(ARPOP_REQUEST);
	(void)memcpy(ea->arp_sha, enaddr, sizeof(ea->arp_sha));
	(void)memcpy(ea->arp_spa, sip, sizeof(ea->arp_spa));
	(void)memcpy(ea->arp_tpa, tip, sizeof(ea->arp_tpa));
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
}
#ifdef CONFIG_RTL_819X
void rtl_arpRequest(ac, type, sip, tip, enaddr)
	register struct arpcom *ac;
	register int type;
	register struct in_addr *sip;
	register struct in_addr *tip;
	register u_char *enaddr;
{
	if(type<0 || type>=MAX_RTL_ARP_TYPE)
		return;
	
	rtl_arp_array[type].arp_flag=1;
	rtl_arp_array[type].arp_sip.s_addr=sip->s_addr;
	rtl_arp_array[type].arp_tip.s_addr=tip->s_addr;
	rtl_arp_array[type].arp_res=0;

	memcpy(rtl_arp_array[type].arp_smac, enaddr, 6);
	
	arprequest(ac, sip, tip, enaddr);
}	
void rtl_dhcpArpRequest(struct arpcom *ac, struct in_addr *sip, struct in_addr *tip, u_char *enaddr)
{
	rtl_arpRequest(ac, DHCPC_SEND_ARP, sip, tip, enaddr);
}
int rtl_checkArpReply(type)
	register int type;
{	
	if(type<0 || type>=MAX_RTL_ARP_TYPE)
		return 0;
	
	rtl_arp_array[type].arp_flag=0;
	return rtl_arp_array[type].arp_res;
}
int rtl_checkArpReplyForDhcp()
{
	return rtl_checkArpReply(DHCPC_SEND_ARP);
}
#endif

#ifdef CONFIG_RTL_FREEBSD_FAST_PATH_SKB
int arpget(ac, rt, dst, desten, rt0)
		register struct arpcom *ac;
	register struct rtentry *rt;
	register struct sockaddr *dst;
	register u_char *desten;
	struct rtentry *rt0;
{
	struct llinfo_arp *la = 0;
	struct sockaddr_dl *sdl;

#if 0
	if (m->m_flags & M_BCAST) {	/* broadcast */
		(void)memcpy(desten, etherbroadcastaddr, sizeof(etherbroadcastaddr));
		return (1);
	}
	if (m->m_flags & M_MCAST) {	/* multicast */
		ETHER_MAP_IP_MULTICAST(&SIN(dst)->sin_addr, desten);
		return(1);
	}
#endif	
	if (rt)
		la = (struct llinfo_arp *)rt->rt_llinfo;
	if (la == 0) {
		la = arplookup(SIN(dst)->sin_addr.s_addr, 1, 0);
		if (la)
			rt = la->la_rt;
	}
	if (la == 0 || rt == 0) {
		log(LOG_DEBUG, "arpresolve: can't allocate llinfo for %s%s%s\n",
			inet_ntoa(SIN(dst)->sin_addr), la ? "la" : "",
				rt ? "rt" : "");
		//m_freem(m);
		return (0);
	}
	sdl = SDL(rt->rt_gateway);
	/*
	 * Check the address family and length is valid, the address
	 * is resolved; otherwise, try to resolve.
	 */
	if ((rt->rt_expire == 0 || rt->rt_expire > time_second) &&
	    sdl->sdl_family == AF_LINK && sdl->sdl_alen != 0) {
		bcopy(LLADDR(sdl), desten, sdl->sdl_alen);
		return 1;
	}
	/*
	 * If ARP is disabled on this interface, stop.
	 * XXX
	 * Probably should not allocate empty llinfo struct if we are
	 * not going to be sending out an arp request.
	 */
	if (ac->ac_if.if_flags & IFF_NOARP)
		return (0);
	
}
#endif

/*
 * Resolve an IP address into an ethernet address.  If success,
 * desten is filled in.  If there is no entry in arptab,
 * set one up and broadcast a request for the IP address.
 * Hold onto this mbuf and resend it once the address
 * is finally resolved.  A return value of 1 indicates
 * that desten has been filled in and the packet should be sent
 * normally; a 0 return indicates that the packet has been
 * taken over here, either now or for later transmission.
 */
int
arpresolve(ac, rt, m, dst, desten, rt0)
	register struct arpcom *ac;
	register struct rtentry *rt;
	struct mbuf *m;
	register struct sockaddr *dst;
	register u_char *desten;
	struct rtentry *rt0;
{
	struct llinfo_arp *la = 0;
	struct sockaddr_dl *sdl;
	#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
	struct mbuf *m_tmp;
	#endif
    #if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
	int same_flags = 0;
    #endif

	if (m->m_flags & M_BCAST) {	/* broadcast */
		(void)memcpy(desten, etherbroadcastaddr, sizeof(etherbroadcastaddr));
		return (1);
	}
	if (m->m_flags & M_MCAST) {	/* multicast */
		ETHER_MAP_IP_MULTICAST(&SIN(dst)->sin_addr, desten);
		return(1);
	}
	if (rt)
		la = (struct llinfo_arp *)rt->rt_llinfo;
	if (la == 0) {
		la = arplookup(SIN(dst)->sin_addr.s_addr, 1, 0);
		if (la)
			rt = la->la_rt;
	}
	if (la == 0 || rt == 0) {
		log(LOG_DEBUG, "arpresolve: can't allocate llinfo for %s%s%s\n",
			inet_ntoa(SIN(dst)->sin_addr), la ? "la" : "",
				rt ? "rt" : "");
		m_freem(m);
		return (0);
	}
	sdl = SDL(rt->rt_gateway);
	/*
	 * Check the address family and length is valid, the address
	 * is resolved; otherwise, try to resolve.
	 */
//#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
//	if ((rt->rt_expire == 0 || rt->rt_expire > time_second || (rtl_wanip_same_with_gwip_support )) && 
//					sdl->sdl_family == AF_LINK && sdl->sdl_alen != 0) 
//#else
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	if ((rt->rt_expire == 0 || rt->rt_expire > time_second) &&
	    sdl->sdl_family == AF_LINK && sdl->sdl_alen != 0 && (la->flag & NUD_REACHABLE)) {
#else
	if ((rt->rt_expire == 0 || rt->rt_expire > time_second) &&
		sdl->sdl_family == AF_LINK && sdl->sdl_alen != 0) 
//#endif
	{
#endif
#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
		if(is_dhcp_wantype == 1)
		{
			if (bcmp(LLADDR(sdl),ac->ac_enaddr, ETHER_ADDR_LEN) == 0) {                                                                       
					same_flags = 1;
			}else{
					bcopy(LLADDR(sdl), desten, sdl->sdl_alen);
					return 1;
			}
		}else
#endif
		{
		    bcopy(LLADDR(sdl), desten, sdl->sdl_alen);
			return 1;
		}
	}
	/*
	 * If ARP is disabled on this interface, stop.
	 * XXX
	 * Probably should not allocate empty llinfo struct if we are
	 * not going to be sending out an arp request.
	 */
	if (ac->ac_if.if_flags & IFF_NOARP){
		m_freem(m);
		return (0);
	}
	
	#ifdef CONFIG_RTL_VLAN_SUPPORT
	arp_tag.v = m->m_pkthdr.tag.v;
	#endif
	/*
	 * There is an arptab entry, but no ethernet address
	 * response yet.  Replace the held mbuf with this
	 * latest one.
	 */
	#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
	m->m_nextpkt = 0;
	if (la->hold_num <= RTL_HOLD_LIST_NUM_MAX) {
		if (la->la_hold) {
			m_tmp = la->la_hold;
			while (m_tmp->m_nextpkt) {
				m_tmp = m_tmp->m_nextpkt;
			}
			/*here m_tmp is the last one*/
			m_tmp->m_nextpkt = m;
		} else {
			la->la_hold = m;
		}
		la->hold_num++;
	} else {
		if (la->la_hold) {
			//diag_printf("la_hold[%d] is bigger than max!\n", la->hold_num);
			m_tmp = la->la_hold->m_nextpkt;
			m_freem(la->la_hold);
			la->hold_num--;
			
			la->la_hold = m_tmp;
			while (m_tmp->m_nextpkt) {
				m_tmp = m_tmp->m_nextpkt;
			}
			/*here m_tmp is the last one*/
			m_tmp->m_nextpkt = m;
			la->hold_num++;
		}
	}
	#else
	if (la->la_hold)
		m_freem(la->la_hold);
	la->la_hold = m;
	#endif
    #if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
	if (rt->rt_expire||same_flags) 
    #else
	if (rt->rt_expire) 
    #endif
	{
		rt->rt_flags &= ~RTF_REJECT;
		if (la->la_asked == 0 || rt->rt_expire != time_second) {
#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
		    if(rt->rt_expire)
#endif
			rt->rt_expire = time_second;
			if (la->la_asked++ < arp_maxtries)
			    arprequest(ac,
			        &SIN(rt->rt_ifa->ifa_addr)->sin_addr,
				&SIN(dst)->sin_addr, ac->ac_enaddr);
			else {
				rt->rt_flags |= RTF_REJECT;
				rt->rt_expire += arpt_down;
				la->la_asked = 0;
				
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
				la->flag = NUD_FAILED;
#endif				
			}

		}
	}
	#ifdef CONFIG_RTL_VLAN_SUPPORT
	arp_tag.v = 0;
	#endif
	return (0);
}

/*
 * Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
static void
arpintr()
{
	register struct mbuf *m;
	register struct arphdr *ar;
	int s;

	while (arpintrq.ifq_head) {
		s = splimp();
		IF_DEQUEUE(&arpintrq, m);
		splx(s);
		if (m == 0 || (m->m_flags & M_PKTHDR) == 0)
			panic("arpintr");
	
                if (m->m_len < sizeof(struct arphdr) &&
                    ((m = m_pullup(m, sizeof(struct arphdr))) == NULL)) {
			log(LOG_ERR, "arp: runt packet -- m_pullup failed\n");
			continue;
		}
		ar = mtod(m, struct arphdr *);

		if (ntohs(ar->ar_hrd) != ARPHRD_ETHER
		    && ntohs(ar->ar_hrd) != ARPHRD_IEEE802) {
			log(LOG_ERR,
			    "arp: unknown hardware address format (0x%d)\n",
			    ntohs(ar->ar_hrd));
            #if 0
            int i = 0;
            unsigned char *p = m->m_data - 14;
            if (p != NULL){
    			diag_printf("dmac:0x%02x:%02x:%02x:%02x:%02x:%02x smac:0x%02x:%02x:%02x:%02x:%02x:%02x type:0x%02x:%02x\n",
    			    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13]);
            }
            p = m->m_data;
            if (p != NULL){
                diag_printf("m->m_pkthdr.len:%d data: ", m->m_pkthdr.len);
                for (i = 0; i < 46; i++)
                {
                    diag_printf("%02x ",p[i]);
                    if((i+1)%8==0)
                        diag_printf("\n");
                }
                diag_printf("\n");
            }
            #endif
			m_freem(m);
			continue;
		}

		if (m->m_pkthdr.len < sizeof(struct arphdr) + 2 * ar->ar_hln
		    + 2 * ar->ar_pln) {
			log(LOG_ERR, "arp: runt packet\n");
			m_freem(m);
			continue;
		}

		switch (ntohs(ar->ar_pro)) {
#ifdef INET
			case ETHERTYPE_IP:
				in_arpinput(m);
				continue;
#endif
		}
		m_freem(m);
	}
}

#ifdef INET
/*
 * ARP for Internet protocols on 10 Mb/s Ethernet.
 * Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 * We no longer handle negotiations for use of trailer protocol:
 * Formerly, ARP replied for protocol type ETHERTYPE_TRAIL sent
 * along with IP replies if we wanted trailers sent to us,
 * and also sent them in response to IP replies.
 * This allowed either end to announce the desire to receive
 * trailer packets.
 * We no longer reply to requests for ETHERTYPE_TRAIL protocol either,
 * but formerly didn't normally send requests.
 */
static int log_arp_wrong_iface = 1;

SYSCTL_INT(_net_link_ether_inet, OID_AUTO, log_arp_wrong_iface, CTLFLAG_RW,
	&log_arp_wrong_iface, 0,
	"log arp packets arriving on the wrong interface");

static void
in_arpinput(m)
	struct mbuf *m;
{
	register struct ether_arp *ea;
	register struct arpcom *ac = (struct arpcom *)m->m_pkthdr.rcvif;
	struct ether_header *eh;
	struct iso88025_header *th = (struct iso88025_header *)0;
	register struct llinfo_arp *la = 0;
	register struct rtentry *rt;
	struct in_ifaddr *ia, *maybe_ia = 0;
	struct sockaddr_dl *sdl;
	struct sockaddr sa;
	struct in_addr isaddr, itaddr, myaddr;
	int op, rif_len;
	#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)&&defined(CONFIG_RTL_HARDWARE_NAT)
	u_char mac_addr[ETHER_ADDR_LEN];
	#endif	
	#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
	struct mbuf* m_tmp;
	#endif

	int idx;

	if (m->m_len < sizeof(struct ether_arp) &&
	    (m = m_pullup(m, sizeof(struct ether_arp))) == NULL) {
		log(LOG_ERR, "in_arp: runt packet -- m_pullup failed\n");
		return;
	}

	ea = mtod(m, struct ether_arp *);
	op = ntohs(ea->arp_op);
	(void)memcpy(&isaddr, ea->arp_spa, sizeof (isaddr));
	(void)memcpy(&itaddr, ea->arp_tpa, sizeof (itaddr));
	for (ia = in_ifaddrhead.tqh_first; ia; ia = ia->ia_link.tqe_next) {
		/*
		 * For a bridge, we want to check the address irrespective
		 * of the receive interface. (This will change slightly
		 * when we have clusters of interfaces).
		 */
#if NBRIDGE > 0
#define BRIDGE_TEST (1)
#else
#define BRIDGE_TEST (0) /* cc will optimise the test away */
#endif
#if 1
		if ((ia->ia_ifp == &ac->ac_if) ||
		(ia->ia_ifp->if_bridge && ia->ia_ifp->if_bridge == ac->ac_if.if_bridge)) {
#else
		if ((BRIDGE_TEST) || (ia->ia_ifp == &ac->ac_if)) {
#endif /* 1 */
			maybe_ia = ia;
			if ((itaddr.s_addr == ia->ia_addr.sin_addr.s_addr) ||
			     (isaddr.s_addr == ia->ia_addr.sin_addr.s_addr)) {
			   	break;
			}
		}
	}
	if (maybe_ia == 0) {
		m_freem(m);
		return;
	}
	myaddr = ia ? ia->ia_addr.sin_addr : maybe_ia->ia_addr.sin_addr;
#ifdef CONFIG_RTL_819X
	if(ia)
		ac=(struct arpcom *)ia->ia_ifp;
	else if(maybe_ia)
		ac=(struct arpcom *)maybe_ia->ia_ifp;
	//if((op==ARPOP_REPLY) && (isaddr.s_addr==tip_dhcp_arp.s_addr) && (memcmp(ea->arp_tha, smac_dhcp_arp, 6)==0))
	//	res_dhcp_arp=1;	

	if(op==ARPOP_REPLY)
	for(idx=0; idx<MAX_RTL_ARP_TYPE; idx++)
	{
		if((rtl_arp_array[idx].arp_flag>0) && (isaddr.s_addr==rtl_arp_array[idx].arp_tip.s_addr) && (memcmp(ea->arp_tha, rtl_arp_array[idx].arp_smac, 6)==0))
		{
			rtl_arp_array[idx].arp_res=1;
			rtl_arp_array[idx].arp_flag=0;
			break;
		}
	}	
#endif
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)ac->ac_enaddr,
	    sizeof (ea->arp_sha))) {
		m_freem(m);	/* it's from me, ignore it. */
		return;
	}
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)etherbroadcastaddr,
	    sizeof (ea->arp_sha))) {
		log(LOG_ERR,
		    "arp: ether address is broadcast for IP address %s!\n",
		    inet_ntoa(isaddr));
		m_freem(m);
		return;
	}
	if (isaddr.s_addr == myaddr.s_addr) {
		log(LOG_ERR,
		   "arp: %6p is using my IP address %s!\n",
		   ea->arp_sha, inet_ntoa(isaddr));
		itaddr = myaddr;
		goto reply;
	}
	la = arplookup(isaddr.s_addr, itaddr.s_addr == myaddr.s_addr, 0);
	if (la && (rt = la->la_rt) && (sdl = SDL(rt->rt_gateway))) {
#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
			if (rt->rt_flags & RTF_GATEWAY)                                                                                                   
			{
					sdl = SDL(rt->rt_gwroute->rt_gateway);
					la = ((struct llinfo_arp *)rt->rt_gwroute->rt_llinfo);
					if(rt->rt_gwroute->rt_expire) /*must not be in,in case*/
							rt->rt_gwroute->rt_expire = time_second + arpt_keep;
			}
#endif
			/* the following is not an error when doing bridging */
		if (!BRIDGE_TEST && rt->rt_ifp != &ac->ac_if) {
		    if (log_arp_wrong_iface)
			log(LOG_ERR, "arp: %s is on %s%d but got reply from %6p on %s%d\n",
			    inet_ntoa(isaddr),
			    rt->rt_ifp->if_name, rt->rt_ifp->if_unit,
			    ea->arp_sha, 
			    ac->ac_if.if_name, ac->ac_if.if_unit);
		    goto reply;
		}
#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
		if (rt->rt_flags & RTF_GATEWAY)                                                                                           
				rt->rt_expire = time_second;
#endif
		if (sdl->sdl_alen &&
		    bcmp((caddr_t)ea->arp_sha, LLADDR(sdl), sdl->sdl_alen)) {
			if (rt->rt_expire)
			{
				// added for solving one ip marked by two or more mac address in tenda_arp, by zhuhuan on 2016.02.26
				#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
				tenda_arp_update_node((u_char *)LLADDR(sdl), isaddr.s_addr, 2, 0);
				#else
				tenda_arp_update_node((u_char *)LLADDR(sdl), isaddr.s_addr, 2);
				#endif
			    log(LOG_INFO, "arp: %s moved from %s ",
				inet_ntoa(isaddr), inet_mactoa((u_char *)LLADDR(sdl)),
				ea->arp_sha,
				ac->ac_if.if_name, ac->ac_if.if_unit);
				log(LOG_INFO, "to %s on %s%d\n",
				inet_mactoa(ea->arp_sha),
				ac->ac_if.if_name, ac->ac_if.if_unit);
			}
			else {
			    log(LOG_ERR,
				"arp: %6p attempts to modify permanent entry for %s on %s%d\n",
				ea->arp_sha, inet_ntoa(isaddr),
				ac->ac_if.if_name, ac->ac_if.if_unit);
			    goto reply;
			}
		}
		(void)memcpy(LLADDR(sdl), ea->arp_sha, sizeof(ea->arp_sha));
		sdl->sdl_alen = sizeof(ea->arp_sha);
        sdl->sdl_rcf = (u_short)0;
	    #if defined(CONFIG_RTL_LAYERED_DRIVER_L3)&&defined(CONFIG_RTL_HARDWARE_NAT)
		if((IN_MULTICAST(ntohl(SIN(rt_key(rt))->sin_addr.s_addr))==0)&&
			(in_broadcast(SIN(rt_key(rt))->sin_addr, rt->rt_ifp)==0)){
				memcpy(mac_addr, LLADDR(sdl), ETHER_ADDR_LEN);
				//diag_printf("%s[%d], mac is %2x-%2x-%2x-%2x-%2x-%2x, ip is 0x%x\n",
				//		__FUNCTION__, __LINE__, mac_addr[0], mac_addr[1], mac_addr[2],
				//		mac_addr[3], mac_addr[4], mac_addr[5], ntohl(SIN(rt_key(rt))->sin_addr.s_addr));
				rtl865x_addArp(ntohl(SIN(rt_key(rt))->sin_addr.s_addr), mac_addr);
		}
		#endif

		/*
		 * If we receive an arp from a token-ring station over
		 * a token-ring nic then try to save the source
		 * routing info.
		 */
		if (ac->ac_if.if_type == IFT_ISO88025) {
			th = (struct iso88025_header *)m->m_pkthdr.header;
			rif_len = TR_RCF_RIFLEN(th->rcf);
			if ((th->iso88025_shost[0] & TR_RII) &&
			    (rif_len > 2)) {
				sdl->sdl_rcf = th->rcf;
				sdl->sdl_rcf ^= htons(TR_RCF_DIR);
				memcpy(sdl->sdl_route, th->rd, rif_len - 2);
				sdl->sdl_rcf &= ~htons(TR_RCF_BCST_MASK);
				/*
				 * Set up source routing information for
				 * reply packet (XXX)
				 */
				m->m_data -= rif_len;
				m->m_len  += rif_len;
				m->m_pkthdr.len += rif_len;
			} else {
				th->iso88025_shost[0] &= ~TR_RII;
			}
			m->m_data -= 8;
			m->m_len  += 8;
			m->m_pkthdr.len += 8;
			th->rcf = sdl->sdl_rcf;
		} else {
			sdl->sdl_rcf = (u_short)0;
		}
		if (rt->rt_expire) {
			if(tenda_arp_isnot_lanhost(isaddr))
				rt->rt_expire = time_second + 10 * arpt_keep;
			else
				rt->rt_expire = time_second + arpt_keep;
		}
		rt->rt_flags &= ~RTF_REJECT;
		la->la_asked = 0;
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		la->flag = NUD_REACHABLE;
		#endif
		#if defined(CONFIG_RTL_HOLD_LIST_SUPPORT)
		while (la->la_hold) {
			m_tmp = la->la_hold->m_nextpkt;
			la->la_hold->m_nextpkt = 0;
			(*ac->ac_if.if_output)(&ac->ac_if, la->la_hold, rt_key(rt), rt);
			la->la_hold = m_tmp;
			la->hold_num--;
		}
		if (la->hold_num < 0)
			la->hold_num = 0;
		if (la->la_hold)
			la->la_hold = 0;
		#else
		if (la->la_hold) {
			(*ac->ac_if.if_output)(&ac->ac_if, la->la_hold,
				rt_key(rt), rt);
			la->la_hold = 0;
		}
		#endif
	}
reply:
	if (op != ARPOP_REQUEST) {
		m_freem(m);
		return;
	}
	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		(void)memcpy(ea->arp_tha, ea->arp_sha, sizeof(ea->arp_sha));		
		(void)memcpy(ea->arp_sha, ac->ac_enaddr, sizeof(ea->arp_sha));		
	} else {
		la = arplookup(itaddr.s_addr, 0, SIN_PROXY);
		if (la == NULL) {
			struct sockaddr_in sin;

			if (!arp_proxyall) {
				m_freem(m);
				return;
			}

			bzero(&sin, sizeof sin);
			sin.sin_family = AF_INET;
			sin.sin_len = sizeof sin;
			sin.sin_addr = itaddr;

			rt = rtalloc1((struct sockaddr *)&sin, 0, 0UL);
			if (!rt) {
				m_freem(m);
				return;
			}
			/*
			 * Don't send proxies for nodes on the same interface
			 * as this one came out of, or we'll get into a fight
			 * over who claims what Ether address.
			 */
			if (rt->rt_ifp == &ac->ac_if) {
				rtfree(rt);
				m_freem(m);
				return;
			}
			(void)memcpy(ea->arp_tha, ea->arp_sha, sizeof(ea->arp_sha));
			(void)memcpy(ea->arp_sha, ac->ac_enaddr, sizeof(ea->arp_sha));
			rtfree(rt);
#ifdef DEBUG_PROXY
			printf("arp: proxying for %s\n",
			       inet_ntoa(itaddr));
#endif
		} else {
			rt = la->la_rt;
			(void)memcpy(ea->arp_tha, ea->arp_sha, sizeof(ea->arp_sha));
			sdl = SDL(rt->rt_gateway);
			(void)memcpy(ea->arp_sha, LLADDR(sdl), sizeof(ea->arp_sha));
		}
	}

	(void)memcpy(ea->arp_tpa, ea->arp_spa, sizeof(ea->arp_spa));
	(void)memcpy(ea->arp_spa, &itaddr, sizeof(ea->arp_spa));
	ea->arp_op = htons(ARPOP_REPLY);
	ea->arp_pro = htons(ETHERTYPE_IP); /* let's be sure! */
	switch (ac->ac_if.if_type) {
	case IFT_ISO88025:
		/* Re-arrange the source/dest address */
		memcpy(th->iso88025_dhost, th->iso88025_shost,
		    sizeof(th->iso88025_dhost));
		memcpy(th->iso88025_shost, ac->ac_enaddr,
		    sizeof(th->iso88025_shost));
		/* Set the source routing bit if neccesary */
		if (th->iso88025_dhost[0] & TR_RII) {
			th->iso88025_dhost[0] &= ~TR_RII;
			if (TR_RCF_RIFLEN(th->rcf) > 2)
				th->iso88025_shost[0] |= TR_RII;
		}
		/* Copy the addresses, ac and fc into sa_data */
		memcpy(sa.sa_data, th->iso88025_dhost,
		    sizeof(th->iso88025_dhost) * 2);
		sa.sa_data[(sizeof(th->iso88025_dhost) * 2)] = TR_AC;
		sa.sa_data[(sizeof(th->iso88025_dhost) * 2) + 1] = TR_LLC_FRAME;
		break;
	case IFT_ETHER:
	case IFT_FDDI:
	/*
	 * May not be correct for types not explictly
	 * listed, but it is our best guess.
	 */
	default:
		eh = (struct ether_header *)sa.sa_data;
		(void)memcpy(eh->ether_dhost, ea->arp_tha,
		    sizeof(eh->ether_dhost));
		eh->ether_type = htons(ETHERTYPE_ARP);
		break;
	}
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
	return;
}
#endif

/*
 * @ Source Code, from BCM project, Ported by zhuhuan @
 * Function name: del_tenda_arp
 * Description: The following function is used to update(delete) arp tables of tenda_arp
 * Date: 2015.12.18
 */
// +++++
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__

static struct ecos_arpentry *arpfree_head = NULL;

static void del_tenda_arp( void )
{
	struct ecos_arpentry *arpfree_node;
	struct ecos_arpentry *arpfree_node_old;

	arpfree_node = arpfree_head;
	arpfree_head = NULL;
	while( arpfree_node!= NULL ){
			
		tenda_arp_update_node(arpfree_node->enaddr, arpfree_node->addr.s_addr, 2 , 0); 
		arpfree_node_old = arpfree_node;
		arpfree_node = arpfree_node->next;
		R_Free((caddr_t)arpfree_node_old);
	}
}

// +++++ modified by zhuhuan on 2016.02.01, for clearing the action of delete arp node of arp table.
static void tenda_app_add_free_list(struct rtentry *rt)
{
	struct ecos_arpentry *arpfree_node;
	struct ecos_arpentry *arpfree_node_old;	

	struct sockaddr *dst;
	struct in_addr ip_addr;
	unsigned char *mac_addr;

	if(rt == NULL)
		return;
	
	if(SDL(rt->rt_gateway)->sdl_alen != ETH_ALEN)
	{
		return ;
	}
	
	dst = rt_key(rt);
	ip_addr = ((struct sockaddr_in *)dst)->sin_addr;
	mac_addr = (u_char *)LLADDR(SDL(rt->rt_gateway));

	arpfree_node = arpfree_head;

	if( arpfree_head == NULL ){
		R_Malloc(arpfree_head,struct ecos_arpentry *,sizeof(struct ecos_arpentry));
		if(arpfree_head == NULL ){
			return ;
		}
		arpfree_node = arpfree_head;
	}
	else{
		while( arpfree_node != NULL  ){
			if(memcmp(arpfree_node->enaddr,mac_addr,ETHER_ADDR_LEN) == 0 && arpfree_node->addr.s_addr == ip_addr.s_addr){
				break;
			}
			arpfree_node_old = arpfree_node;
			arpfree_node = arpfree_node->next;
		}
		if(arpfree_node != NULL ){
			return ;
		}
		else{
			R_Malloc(arpfree_node,struct ecos_arpentry *,sizeof(struct ecos_arpentry));
			if(arpfree_node == NULL ){
				return ;
			}
			arpfree_node_old->next = arpfree_node;
		}
	}

	arpfree_node->next =  NULL ;
	memcpy(arpfree_node->enaddr, mac_addr, ETHER_ADDR_LEN);
	arpfree_node->addr.s_addr = ip_addr.s_addr;
	
	return ;
}
#else
// +++++ modified by zhuhuan on 2016.02.01, for clearing the action of delete arp node of arp table.
static void del_tenda_arp(struct rtentry *rt)
{

	struct sockaddr *dst;
	struct in_addr ip_addr;
	unsigned char *mac_addr;
	struct ecos_arpentry arp_free_node;

	if(rt == NULL)
		return;

	if(SDL(rt->rt_gateway)->sdl_alen != ETH_ALEN)
	{
		return ;
	}

	bzero((char *)&arp_free_node, sizeof(arp_free_node));
	dst = rt_key(rt);
	ip_addr = ((struct sockaddr_in *)dst)->sin_addr;
	mac_addr = (u_char *)LLADDR(SDL(rt->rt_gateway));
	arp_free_node.addr.s_addr = ip_addr.s_addr;
	memcpy(arp_free_node.enaddr, mac_addr, ETHER_ADDR_LEN);
	
    tenda_arp_update_node(arp_free_node.enaddr, arp_free_node.addr.s_addr, 2);

	return ;
}
// +++++
#endif

/*
 * Free an arp entry.
 */
static void
arptfree(la)
	register struct llinfo_arp *la;
{
	register struct rtentry *rt = la->la_rt;
	register struct sockaddr_dl *sdl;

	if (rt == 0)
		panic("arptfree");
	
	if (rt->rt_refcnt > 0 && (sdl = SDL(rt->rt_gateway)) &&
	    sdl->sdl_family == AF_LINK) {
	    /*
		 * @ Source Code, from BCM project, Ported by zhuhuan @
		 * Function name: NULL
		 * Description: The following code is used to update(delete) arp tables of tenda_arp
		 * Date: 2015.12.18
		 */
		// +++++ add by z10312  解决概率性 有线设备离线时,5分钟内页面网速控制还存在该信息。2015-04-28
	#ifdef __CONFIG_TENDA_HTTPD_NORMAL__		
		tenda_app_add_free_list(rt);
		del_tenda_arp();
	#else
		del_tenda_arp(rt);
	#endif
		// +++++
		
		sdl->sdl_alen = 0;
		la->la_asked = 0;
		rt->rt_flags &= ~RTF_REJECT;
		
		return;
	}
	
		/*
		 * @ Source Code, from BCM project, Ported by zhuhuan @
		 * Function name: NULL
		 * Description: The following code is used to update(delete) arp tables of tenda_arp
		 * Date: 2015.12.18
		 */
		// +++++ 
	#ifdef __CONFIG_TENDA_HTTPD_NORMAL__	
		tenda_app_add_free_list(rt);
		del_tenda_arp();
	#else
		del_tenda_arp(rt);
	#endif	
		// +++++
	rtrequest(RTM_DELETE, rt_key(rt), (struct sockaddr *)0, rt_mask(rt),
			0, (struct rtentry **)0);
}

/*
 * Lookup or enter a new address in arptab.
 */
struct llinfo_arp *
arplookup(addr, create, proxy)
	u_long addr;
	int create, proxy;
{
	register struct rtentry *rt;
	static struct sockaddr_inarp sin = {sizeof(sin), AF_INET };
	const char *why = 0;

	sin.sin_addr.s_addr = addr;
	sin.sin_other = proxy ? SIN_PROXY : 0;
	rt = rtalloc1((struct sockaddr *)&sin, create, 0UL);
	if (rt == 0)
		return (0);
	rt->rt_refcnt--;

#if defined(CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT)
	if (rt->rt_flags & RTF_GATEWAY){
			if(!check_wan_gw_same_ip())
					why = "host is not on local network";
	}
#else
	if (rt->rt_flags & RTF_GATEWAY)
		why = "host is not on local network";
#endif
	else if ((rt->rt_flags & RTF_LLINFO) == 0)
		why = "could not allocate llinfo";
	else if (rt->rt_gateway->sa_family != AF_LINK)
		why = "gateway route is not ours";

	if (why && create) {
		log(LOG_DEBUG, "arplookup %s failed: %s\n",
		    inet_ntoa(sin.sin_addr), why);
		return 0;
	} else if (why) {
		return 0;
	}
	return ((struct llinfo_arp *)rt->rt_llinfo);
}

void
arp_ifinit(ac, ifa)
	struct arpcom *ac;
	struct ifaddr *ifa;
{
	if (ntohl(IA_SIN(ifa)->sin_addr.s_addr) != INADDR_ANY)
		arprequest(ac, &IA_SIN(ifa)->sin_addr,
			       &IA_SIN(ifa)->sin_addr, ac->ac_enaddr);
	ifa->ifa_rtrequest = arp_rtrequest;
	ifa->ifa_flags |= RTF_CLONING;
	#ifdef CONFIG_RTL_VLAN_SUPPORT
	arp_tag.v = 0;
	#endif
}

/*
 * @ Source Code, from BCM project, Ported by zhuhuan @
 * Function name: arpioctl
 * Description: The following function is used for ARP ioctl 
 * Date: 2015.12.18
 */
// +++++
//#ifdef BCM47XX
#if 1	//llm modify
#include <sys/sockio.h>
/* ARP ioctl */
int
arpioctl(req, data, p)
	int req;
	caddr_t data;
	struct proc *p;
{
	struct ecos_arpentry *arpreq = (struct ecos_arpentry *)data;
	register struct rtentry *rt = 0;
	struct llinfo_arp *la = 0;
	struct sockaddr_dl *sdl;

	switch (req) {
	case SIOCGARPRT:
		/* ARP table look up */
		la = arplookup(arpreq->addr.s_addr, 0, 0);
		if (la)
			rt = la->la_rt;
		if (!la || !rt)
			break;

		/* Copy back */
		sdl = SDL(rt->rt_gateway);
		if ((rt->rt_expire == 0 || rt->rt_expire > time_second) &&
			sdl->sdl_family == AF_LINK && sdl->sdl_alen != 0) {
			bcopy(LLADDR(sdl), arpreq->enaddr, sdl->sdl_alen);
		}
		break;
//roy+++,2010/10/08
	case SIOSGARPRT:
		 /* call arplookup with create flag */
	// diag_printf("[%s]::inet_ntoa(arpreq->addr)=%s,arpreq->enaddr=%s\n", __FUNCTION__,inet_ntoa(arpreq->addr), ether_ntoa(arpreq->enaddr));
		la = arplookup(arpreq->addr.s_addr, 1, 0);
		if (la)
			rt = la->la_rt;
		if (!la || !rt)
			break;
		
		sdl = SDL(rt->rt_gateway);
		
		bcopy((caddr_t)arpreq->enaddr, LLADDR(sdl),
			  sdl->sdl_alen = sizeof(struct ether_addr)); 

		rt->rt_expire = 0; 

		rt->rt_flags &= ~RTF_REJECT;
		rt->rt_flags |= RTF_STATIC; /* manually added route. */
		la->la_asked = 0;
		rt->rt_flags |= RTF_ANNOUNCE;
		
		break;
	case SIOCGARPNU:
		arpreq->addr.s_addr =  lan_arp_inuse;
		break;
//+++
	case SIOCDELARPRT://删除ARP表项
		/* ARP table look up */
		la = arplookup(arpreq->addr.s_addr, 0, 0);
		if (la)
			rt = la->la_rt;
		if (!la || !rt)
			break;

		/* Free this one */
		arptfree(la);
		break;

	default:
		return EOPNOTSUPP;
	}

	return 0;
}
#endif	/* BCM47XX */
// +++++
