//==========================================================================
//
//      src/sys/netinet/ip_input.c
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
 *	@(#)ip_input.c	8.2 (Berkeley) 1/4/94
 * $FreeBSD: src/sys/netinet/ip_input.c,v 1.130.2.25 2001/08/29 21:41:37 jesper Exp $
 */

#define	_IP_VHL

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <net/netisr.h>
#include <net/intrq.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>

#include <sys/socketvar.h>

#include <netinet/ip_fw.h>

#ifdef IPSEC
#include <netinet6/ipsec.h>
#include <netkey/key.h>
#endif

#ifdef DUMMYNET
#include <netinet/ip_dummynet.h>
#endif

#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#endif
#if defined(CONFIG_RTL_819X)
	#include <netinet/udp.h>
#endif

/*lqz add for APCLIENT_DHCPC*/
#include <arpa/nameser.h>
/*end*/

#ifdef __CONFIG_TCP_AUTO_MTU__
/*lqz add for PPPOE auto mtu*/
#include <netinet/tcp.h> /* MSS */
#endif

int rsvp_on = 0;
static int ip_rsvp_on;
struct socket *ip_rsvpd;

//int	ipforwarding = 0;
int	ipforwarding = 1;
SYSCTL_INT(_net_inet_ip, IPCTL_FORWARDING, forwarding, CTLFLAG_RW,
    &ipforwarding, 0, "Enable IP forwarding between interfaces");

static int	ipsendredirects = 1; /* XXX */
SYSCTL_INT(_net_inet_ip, IPCTL_SENDREDIRECTS, redirect, CTLFLAG_RW,
    &ipsendredirects, 0, "Enable sending IP redirects");

int	ip_defttl = IPDEFTTL;
SYSCTL_INT(_net_inet_ip, IPCTL_DEFTTL, ttl, CTLFLAG_RW,
    &ip_defttl, 0, "Maximum TTL on IP packets");

static int	ip_dosourceroute = 0;
SYSCTL_INT(_net_inet_ip, IPCTL_SOURCEROUTE, sourceroute, CTLFLAG_RW,
    &ip_dosourceroute, 0, "Enable forwarding source routed IP packets");

static int	ip_acceptsourceroute = 0;
SYSCTL_INT(_net_inet_ip, IPCTL_ACCEPTSOURCEROUTE, accept_sourceroute, 
    CTLFLAG_RW, &ip_acceptsourceroute, 0, 
    "Enable accepting source routed IP packets");

#if defined(NFAITH) && 0 < NFAITH || defined(CYGPKG_NET_FREEBSD_SYSCTL)
static int	ip_keepfaith = 0;
#endif

SYSCTL_INT(_net_inet_ip, IPCTL_KEEPFAITH, keepfaith, CTLFLAG_RW,
	&ip_keepfaith,	0,
	"Enable packet capture for FAITH IPv4->IPv6 translater daemon");

#ifdef CYGPKG_NET_FREEBSD_SYSCTL
static int	ip_maxfragpackets;	/* initialized in ip_init() */
#endif
SYSCTL_INT(_net_inet_ip, OID_AUTO, maxfragpackets, CTLFLAG_RW,
	&ip_maxfragpackets, 0,
	"Maximum number of IPv4 fragment reassembly queue entries");

static int    nipq = 0;         /* total # of reass queues */
static int    maxnipq;

/*
 * XXX - Setting ip_checkinterface mostly implements the receive side of
 * the Strong ES model described in RFC 1122, but since the routing table
 * and transmit implementation do not implement the Strong ES model,
 * setting this to 1 results in an odd hybrid.
 *
 * XXX - ip_checkinterface currently must be disabled if you use ipnat
 * to translate the destination address to another local interface.
 *
 * XXX - ip_checkinterface must be disabled if you add IP aliases
 * to the loopback interface instead of the interface where the
 * packets for those addresses are received.
 */
static int	ip_checkinterface = 0;
SYSCTL_INT(_net_inet_ip, OID_AUTO, check_interface, CTLFLAG_RW,
    &ip_checkinterface, 0, "Verify packet arrives on correct interface");

#ifdef DIAGNOSTIC
static int	ipprintfs = 0;
#endif

extern	struct domain inetdomain;
extern	struct protosw inetsw[];
u_char	ip_protox[IPPROTO_MAX];
static int	ipqmaxlen = IFQ_MAXLEN;
struct	in_ifaddrhead in_ifaddrhead; /* first inet address */
SYSCTL_INT(_net_inet_ip, IPCTL_INTRQMAXLEN, intr_queue_maxlen, CTLFLAG_RW,
    &ipintrq.ifq_maxlen, 0, "Maximum size of the IP input queue");
SYSCTL_INT(_net_inet_ip, IPCTL_INTRQDROPS, intr_queue_drops, CTLFLAG_RD,
    &ipintrq.ifq_drops, 0, "Number of packets dropped from the IP input queue");

struct ipstat ipstat;
SYSCTL_STRUCT(_net_inet_ip, IPCTL_STATS, stats, CTLFLAG_RD,
    &ipstat, ipstat, "IP statistics (struct ipstat, netinet/ip_var.h)");

/* Packet reassembly stuff */
#define IPREASS_NHASH_LOG2      6
#define IPREASS_NHASH           (1 << IPREASS_NHASH_LOG2)
#define IPREASS_HMASK           (IPREASS_NHASH - 1)
#define IPREASS_HASH(x,y) \
	(((((x) & 0xF) | ((((x) >> 8) & 0xF) << 4)) ^ (y)) & IPREASS_HMASK)

static struct ipq ipq[IPREASS_NHASH];
const  int    ipintrq_present = 1;

#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
#define  RTL_IP_FRAG_TIMEOUT	5
#define  RTL_IP_FRAG_HECK		(hz)
#define RTL_IP_FRAG_NHASH		(IPREASS_NHASH>>3)
static int rtl_ipq_num = 0;
static int rtl_ipq_max = RTL_IP_FRAG_NHASH*4;
struct callout rtl_ip_frag_timer;
struct rtl_ipq rtl_ipq[RTL_IP_FRAG_NHASH];
static void rtl_initIpFrag(void);
static void rtl_ipFragTimeout(void);
#endif

#ifdef IPCTL_DEFMTU
SYSCTL_INT(_net_inet_ip, IPCTL_DEFMTU, mtu, CTLFLAG_RW,
    &ip_mtu, 0, "Default MTU");
#endif

#ifdef IPSTEALTH
static int	ipstealth = 0;
SYSCTL_INT(_net_inet_ip, OID_AUTO, stealth, CTLFLAG_RW,
    &ipstealth, 0, "");
#endif


/* Firewall hooks */
ip_fw_chk_t *ip_fw_chk_ptr;
ip_fw_ctl_t *ip_fw_ctl_ptr;
int fw_enable = 1 ;

#ifdef DUMMYNET
ip_dn_ctl_t *ip_dn_ctl_ptr;
#endif

int (*fr_checkp) __P((struct ip *, int, struct ifnet *, int, struct mbuf **)) = NULL;


/*
 * We need to save the IP options in case a protocol wants to respond
 * to an incoming packet over the same route if the packet got here
 * using IP source routing.  This allows connection establishment and
 * maintenance when the remote end is on a network that is not known
 * to us.
 */
static int	ip_nhops = 0;
static	struct ip_srcrt {
	struct	in_addr dst;			/* final destination */
	char	nop;				/* one NOP to align */
	char	srcopt[IPOPT_OFFSET + 1];	/* OPTVAL, OLEN and OFFSET */
	struct	in_addr route[MAX_IPOPTLEN/sizeof(struct in_addr)];
} ip_srcrt;

struct sockaddr_in *ip_fw_fwd_addr;

static void	save_rte __P((u_char *, struct in_addr));
static int	ip_dooptions __P((struct mbuf *));
#ifdef NATPT
       void	ip_forward __P((struct mbuf *, int));
#else
static void	ip_forward __P((struct mbuf *, int));
#endif
static void	ip_freef __P((struct ipq *));
#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
static struct	mbuf *ip_reass __P((struct mbuf *,
			struct ipq *, struct ipq *, u_int32_t *, u_int16_t *));
#else
static struct	mbuf *ip_reass __P((struct mbuf *, struct ipq *, struct ipq *));
#endif
static struct	in_ifaddr *ip_rtaddr __P((struct in_addr));
static void	ipintr __P((void));

#ifdef NATPT
extern	int			ip6_protocol_tr;
int	natpt_in4		__P((struct mbuf *, struct mbuf **));
extern	void ip6_forward	__P((struct mbuf *, int));
#endif	/* NATPT */

/* CONFIG_PPP + */
/*add by lq*/
extern int (*dialup_checkp) __P((void));
/* CONFIG_PPP -*/
/*
 * IP initialization: fill in IP protocol switch table.
 * All protocols not implemented in kernel go to raw IP protocol handler.
 */
#if defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
extern int rtl_enterNaptProcess(struct mbuf *m, int incoming, int port);
#endif

#if defined(IPFIREWALL_FORWARD)	
extern void ip_fw_init() ;
#endif
void
ip_init()
{
	register struct protosw *pr;
	register int i;

	TAILQ_INIT(&in_ifaddrhead);
	pr = pffindproto(PF_INET, IPPROTO_RAW, SOCK_RAW);
	if (pr == 0)
		panic("ip_init");
	for (i = 0; i < IPPROTO_MAX; i++)
		ip_protox[i] = pr - inetsw;
	for (pr = inetdomain.dom_protosw;
	    pr < inetdomain.dom_protoswNPROTOSW; pr++)
		if (pr->pr_domain->dom_family == PF_INET &&
		    pr->pr_protocol && pr->pr_protocol != IPPROTO_RAW)
			ip_protox[pr->pr_protocol] = pr - inetsw;

	for (i = 0; i < IPREASS_NHASH; i++)
	    ipq[i].next = ipq[i].prev = &ipq[i];
	
#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
	rtl_initIpFrag();
#endif

	maxnipq = nmbclusters / 4;

#ifndef RANDOM_IP_ID
	ip_id = time_second & 0xffff;
#endif
	ipintrq.ifq_maxlen = ipqmaxlen;

#if defined(CONFIG_RTL_819X) //jwj:20120704
#if defined(IPFIREWALL_FORWARD)	
	extern void ip_fw_init(void);
	ip_fw_init();
#endif
#ifdef DUMMYNET
	ip_dn_init();
#endif
	#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
	extern int fastpath_init(void);
	fastpath_init();
	#endif
    
   	#if defined(CONFIG_RTL_NETSNIPER_SUPPORT)
	extern int netsniper_init(void);
	netsniper_init();
	#endif
    
    #if defined(CONFIG_RTL_SPI_FIREWALL_SUPPORT)
    int  init_spi_firewall(void);
	init_spi_firewall();
	#endif
    
#endif

	register_netisr(NETISR_IP, ipintr);
}

static struct	sockaddr_in ipaddr = { sizeof(ipaddr), AF_INET };
static struct	route ipforward_rt;

#ifdef __CONFIG_TCP_AUTO_MTU__
extern void ip_input_update_mss(unsigned int *mss);
static unsigned int tcp_mss = 1460;
static unsigned int tcp_mss_enable = 0;
#define MIN_MSS 1300

void ip_input_set_mss(unsigned int mss)
{
	if(mss < MIN_MSS)
		return;
		
	tcp_mss = mss;
	return;
}

unsigned int ip_input_get_mss()
{
	return tcp_mss;
}

void ip_input_set_mss_enable(unsigned int enable)
{
	if(1 == enable)
		tcp_mss_enable = 1;
	else
		tcp_mss_enable = 0;
	return;
}

unsigned int ip_input_get_mss_enable()
{
	return tcp_mss_enable;
}
#endif

/*
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassemble.  Process options.  Pass to next level.
 */
void
ip_input(struct mbuf *m)
{
	struct ip *ip;
	struct ipq *fp;
	struct in_ifaddr *ia = NULL;
	int    i, hlen, mff, checkif;
	u_short sum;
	u_int16_t divert_cookie;		/* firewall cookie */
	struct in_addr pkt_dst;
#if defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
	int iresult;
#endif
#ifdef IPDIVERT
	u_int32_t divert_info = 0;		/* packet divert/tee info */
#endif
	struct ip_fw_chain *rule = NULL;

#if defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
input_again:		//For ipfw qos and filter
#endif
#if defined(IPDIVERT)
	/* Get and reset firewall cookie */
	divert_cookie = ip_divert_cookie;
	ip_divert_cookie = 0;
#if defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
	rule = NULL;
	divert_info = 0;
#endif
#endif

#if defined(IPFIREWALL) && defined(DUMMYNET)
        /*
         * dummynet packet are prepended a vestigial mbuf with
         * m_type = MT_DUMMYNET and m_data pointing to the matching
         * rule.
         */
        if (m->m_type == MT_DUMMYNET) {
            rule = (struct ip_fw_chain *)(m->m_data) ;
            m = m->m_next ;
            ip = mtod(m, struct ip *);
            hlen = IP_VHL_HL(ip->ip_vhl) << 2;
            goto iphack ;
        } else
            rule = NULL ;
#endif

#ifdef	DIAGNOSTIC
	if (m == NULL || (m->m_flags & M_PKTHDR) == 0)
		panic("ip_input no HDR");
#endif
	ipstat.ips_total++;

	if (m->m_pkthdr.len < sizeof(struct ip))
		goto tooshort;

	if (m->m_len < sizeof (struct ip) &&
	    (m = m_pullup(m, sizeof (struct ip))) == 0) {
		ipstat.ips_toosmall++;
		return;
	}
	ip = mtod(m, struct ip *);

	if (IP_VHL_V(ip->ip_vhl) != IPVERSION) {
		ipstat.ips_badvers++;
		goto bad;
	}

	hlen = IP_VHL_HL(ip->ip_vhl) << 2;
	if (hlen < sizeof(struct ip)) {	/* minimum header length */
		ipstat.ips_badhlen++;
		goto bad;
	}
	if (hlen > m->m_len) {
		if ((m = m_pullup(m, hlen)) == 0) {
			ipstat.ips_badhlen++;
			return;
		}
		ip = mtod(m, struct ip *);
	}

	/* 127/8 must not appear on wire - RFC1122 */
	if ((ntohl(ip->ip_dst.s_addr) >> IN_CLASSA_NSHIFT) == IN_LOOPBACKNET ||
	    (ntohl(ip->ip_src.s_addr) >> IN_CLASSA_NSHIFT) == IN_LOOPBACKNET) {
		if ((m->m_pkthdr.rcvif->if_flags & IFF_LOOPBACK) == 0) {
			ipstat.ips_badaddr++;
			goto bad;
		}
	}

	if (m->m_pkthdr.csum_flags & CSUM_IP_CHECKED) {
		sum = !(m->m_pkthdr.csum_flags & CSUM_IP_VALID);
	} else {
		if (hlen == sizeof(struct ip)) {
			sum = in_cksum_hdr(ip);
		} else {
			sum = in_cksum(m, hlen);
		}
	}
	if (sum) {
		ipstat.ips_badsum++;
		goto bad;
	}

#ifdef ALTQ
	if (altq_input != NULL && (*altq_input)(m, AF_INET) == 0)
		/* packet is dropped by traffic conditioner */
		return;
#endif
	/*
	 * Convert fields to host representation.
	 */
	NTOHS(ip->ip_len);
	if (ip->ip_len < hlen) {
		ipstat.ips_badlen++;
		goto bad;
	}
	NTOHS(ip->ip_off);

	/*
	 * Check that the amount of data in the buffers
	 * is as at least much as the IP header would have us expect.
	 * Trim mbufs if longer than we expect.
	 * Drop packet if shorter than we expect.
	 */
	if (m->m_pkthdr.len < ip->ip_len) {
tooshort:
		ipstat.ips_tooshort++;
		goto bad;
	}
	if (m->m_pkthdr.len > ip->ip_len) {
		if (m->m_len == m->m_pkthdr.len) {
			m->m_len = ip->ip_len;
			m->m_pkthdr.len = ip->ip_len;
		} else
			m_adj(m, ip->ip_len - m->m_pkthdr.len);
	}

#ifdef IPSEC
	if (ipsec_getnhist(m))
		goto pass;
#endif

	/*
	 * IpHack's section.
	 * Right now when no processing on packet has done
	 * and it is still fresh out of network we do our black
	 * deals with it.
	 * - Firewall: deny/allow/divert
	 * - Xlate: translate packet's addr/port (NAT).
	 * - Pipe: pass pkt through dummynet.
	 * - Wrap: fake packet's addr/port <unimpl.>
	 * - Encapsulate: put it in another IP and send out. <unimp.>
 	 */

#if defined(IPFIREWALL) && defined(DUMMYNET)
iphack:
#endif
	/*
	 * Check if we want to allow this packet to be processed.
	 * Consider it to be bad if not.
	 */
	// diag_printf("1: sIp is 0x%x, dIp is 0x%x\n", ip->ip_src.s_addr, ip->ip_dst.s_addr);

	if (fr_checkp) {
		struct	mbuf	*m1 = m;

		if ((*fr_checkp)(ip, hlen, m->m_pkthdr.rcvif, 0, &m1) || !m1)
			return;
		ip = mtod(m = m1, struct ip *);
	}
	if (fw_enable && ip_fw_chk_ptr) {
#ifdef IPFIREWALL_FORWARD
		/*
		 * If we've been forwarded from the output side, then
		 * skip the firewall a second time
		 */
		if (ip_fw_fwd_addr){
			goto ours;
		}
#endif	/* IPFIREWALL_FORWARD */
		/*
		 * See the comment in ip_output for the return values
		 * produced by the firewall.
		 */
		
		i = (*ip_fw_chk_ptr)(&ip,
		    hlen, NULL, &divert_cookie, &m, &rule, &ip_fw_fwd_addr);
		if ( (i & IP_FW_PORT_DENY_FLAG) || m == NULL) { /* drop */
                        if (m){
                                m_freem(m);
                        }
			return ;
                }
		ip = mtod(m, struct ip *); /* just in case m changed */
		if (i == 0 && ip_fw_fwd_addr == NULL)	/* common case */
			goto pass;
		
#ifdef IPDIVERT
		if (i != 0 && (i & IP_FW_PORT_DYNT_FLAG) == 0) {
			/* Divert or tee packet */
			divert_info = i;
			#if defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
            #ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS
            rtl_setInputDeviceName(m->m_pkthdr.rcvif->if_name, m->m_pkthdr.rcvif->if_unit);
            #endif
			iresult = rtl_enterNaptProcess(m, 1, divert_info&0xffff);
			/*#define PKT_ALIAS_OK 1
			   #define PKT_ALIAS_FOUND_HEADER_FRAGMENT 4
			   #define PKT_ALIAS_FOUND_FRAGMENT_OUT_OF_ORDER 5*/
			if((iresult==1) ||(iresult==4)){
				ip_divert_cookie = divert_cookie;
#if BYTE_ORDER == LITTLE_ENDIAN
				ip = mtod(m, struct ip *);
				HTONS(ip->ip_len);
				HTONS(ip->ip_off);
#endif
				goto input_again;
			}
            else if (!iresult)
            {
                /* iresult == 0 means alg off drop this packet */
                if (m)
                {
                    m_freem(m);
                }  
                return;
            }
			#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
			else if(iresult == 5){
				goto ours;
			}
			#endif
			#else
			goto ours;
			#endif
		}
#endif

#ifdef DUMMYNET
                if ((i & IP_FW_PORT_DYNT_FLAG) != 0) {
	#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)	
					addQosCache(i&0xffff,rule->rule);
	#endif
                    /* Send packet to the appropriate pipe */
	#if defined (CONFIG_RTL_QOS)
					dummynet_io(i&0xffff,DN_TO_IP_IN,m,
                    	NULL,NULL,0, rule,0,NULL);
	#else
					dummynet_io(i&0xffff,DN_TO_IP_IN,m,
                    	NULL,NULL,0, rule,0);
	#endif
					return;
				}
#endif
#ifdef IPFIREWALL_FORWARD
		if (i == 0 && ip_fw_fwd_addr != NULL)
			goto pass;
#endif
		/*
		 * if we get here, the packet must be dropped
		 */
		m_freem(m);
		return;
	}
pass:

    #if defined(CONFIG_RTL_SPI_FIREWALL_SUPPORT)
    /* handle packets from lan to dut, wan to dut, lan to wan, wan to lan, lan to lan */
    rtl_add_spi_firewall_new_connect(ip);//learning syn packets            
    if (!tcp_in_window(ip))
    {
        if (m)
        {
            m_freem(m);
        }  
        return;
    }
    #endif

	/*
	 * Process options and, if not destined for us,
	 * ship it on.  ip_dooptions returns 1 when an
	 * error was detected (causing an icmp message
	 * to be sent and the original packet to be freed).
	 */
	ip_nhops = 0;		/* for source routed packets */
	if (hlen > sizeof (struct ip) && ip_dooptions(m)) {
#ifdef IPFIREWALL_FORWARD
		ip_fw_fwd_addr = NULL;
#endif
		return;
	}

        /* greedy RSVP, snatches any PATH packet of the RSVP protocol and no
         * matter if it is destined to another node, or whether it is 
         * a multicast one, RSVP wants it! and prevents it from being forwarded
         * anywhere else. Also checks if the rsvp daemon is running before
	 * grabbing the packet.
         */
	if (rsvp_on && ip->ip_p==IPPROTO_RSVP) 
		goto ours;

#ifdef NATPT
	/*
	 * NATPT (Network Address Translation - Protocol Translation)
	 */
	if (ip6_protocol_tr) {
		struct mbuf	*m1 = NULL;

		switch (natpt_in4(m, &m1)) {
		case IPPROTO_IP:	/* this packet is not changed	*/
			goto checkaddresses;

		case IPPROTO_IPV4:
			ip_forward(m1, 0);
			break;

		case IPPROTO_IPV6:
			ip6_forward(m1, 1);
			break;

		case IPPROTO_DONE:	/* discard without free	*/
			return;

		case IPPROTO_MAX:	/* discard this packet	*/
		default:
			break;
		}

		if (m != m1)
			m_freem(m);

		return;
	}
checkaddresses:;
#endif

	/*
	 * Check our list of addresses, to see if the packet is for us.
	 * If we don't have any addresses, assume any unicast packet
	 * we receive might be for us (and let the upper layers deal
	 * with it).
	 */
	 if((m->m_flags & (M_MCAST|M_BCAST)) == 0) 
	{ 
		if((m->m_flags & (M_MACERROR)))
		{ 
			ipstat.ips_tooshort++; 
			goto bad; 
		} 
	}
	if (TAILQ_EMPTY(&in_ifaddrhead) &&
	    (m->m_flags & (M_MCAST|M_BCAST)) == 0)
		goto ours;

	/*
	 * Cache the destination address of the packet; this may be
	 * changed by use of 'ipfw fwd'.
	 */
	pkt_dst = ip_fw_fwd_addr == NULL ?
	    ip->ip_dst : ip_fw_fwd_addr->sin_addr;

	/*
	 * Enable a consistency check between the destination address
	 * and the arrival interface for a unicast packet (the RFC 1122
	 * strong ES model) if IP forwarding is disabled and the packet
	 * is not locally generated and the packet is not subject to
	 * 'ipfw fwd'.
	 *
	 * XXX - Checking also should be disabled if the destination
	 * address is ipnat'ed to a different interface.
	 *
	 * XXX - Checking is incompatible with IP aliases added
	 * to the loopback interface instead of the interface where
	 * the packets are received.
	 */
	checkif = ip_checkinterface && (ipforwarding == 0) && 
	    ((m->m_pkthdr.rcvif->if_flags & IFF_LOOPBACK) == 0) &&
	    (ip_fw_fwd_addr == NULL);

	TAILQ_FOREACH(ia, &in_ifaddrhead, ia_link) {
#define	satosin(sa)	((struct sockaddr_in *)(sa))

#ifdef BOOTP_COMPAT
		if (IA_SIN(ia)->sin_addr.s_addr == INADDR_ANY)
			goto ours;
#endif
		/*
		 * If the address matches, verify that the packet
		 * arrived via the correct interface if checking is
		 * enabled.
		 */
		if (IA_SIN(ia)->sin_addr.s_addr == pkt_dst.s_addr && 
		    (!checkif || ia->ia_ifp == m->m_pkthdr.rcvif))
			goto ours;
		/*
		 * Only accept broadcast packets that arrive via the
		 * matching interface.  Reception of forwarded directed
		 * broadcasts would be handled via ip_forward() and
		 * ether_output() with the loopback into the stack for
		 * SIMPLEX interfaces handled by ether_output().
		 */
		if (ia->ia_ifp == m->m_pkthdr.rcvif &&
		    ia->ia_ifp && ia->ia_ifp->if_flags & IFF_BROADCAST) {
			if (satosin(&ia->ia_broadaddr)->sin_addr.s_addr ==
			    pkt_dst.s_addr)
				goto ours;
			if (ia->ia_netbroadcast.s_addr == pkt_dst.s_addr)
				goto ours;
		}
	}
#ifdef	NETBIOS_SUPPORT
#if defined(CONFIG_RTL_819X) 
	/*special patch for netbios,note:to refine it*/
	struct udphdr *udp_hdr = (struct udphdr *)((unsigned char *)ip +(IP_VHL_HL(ip->ip_vhl) << 2));
	if((udp_hdr) && (ip->ip_p == IPPROTO_UDP) && (udp_hdr->uh_dport ==htons(137)) ) {
				TAILQ_FOREACH(ia, &in_ifaddrhead, ia_link) {	
				 if(ia->ia_ifp && ia->ia_ifp->if_flags &IFF_BROADCAST) 
				 {
					if(satosin(&ia->ia_broadaddr)->sin_addr.s_addr ==pkt_dst.s_addr)
					{
						goto ours;
					}
				 }				
		}
	}
#endif
#endif

	if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr))) {
		//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
		struct in_multi *inm;
	#ifndef CONFIG_RTL_IGMP_PROXY_KERNEL_MODE
		if (ip_mrouter) 
	#endif	
			{
			/*
			 * If we are acting as a multicast router, all
			 * incoming multicast packets are passed to the
			 * kernel-level multicast forwarding function.
			 * The packet is returned (relatively) intact; if
			 * ip_mforward() returns a non-zero value, the packet
			 * must be discarded, else it may be accepted below.
			 */
			if (ip_mforward(ip, m->m_pkthdr.rcvif, m, 0) != 0) {
				ipstat.ips_cantforward++;
				m_freem(m);
				return;
			}

			/*
			 * The process-level routing demon needs to receive
			 * all multicast IGMP packets, whether or not this
			 * host belongs to their destination groups.
			 */
			if (ip->ip_p == IPPROTO_IGMP)
				goto ours;
			ipstat.ips_forward++;
		}
#if defined(CONFIG_RTL_819X)
		//#include <netinet/udp.h>
		struct udphdr *udp_hdr = (struct udphdr *)((unsigned char *)ip + (IP_VHL_HL(ip->ip_vhl) << 2));
		/*for socket to receive upnp m-search from wlan interface*/
		if((udp_hdr) && (ip->ip_p == IPPROTO_UDP) && (udp_hdr->uh_dport == htons(1900)) && (ip->ip_dst.s_addr == htonl(0xeffffffa)) ){
			if(m->m_pkthdr.__unused == 0xe5)
				m->m_pkthdr.__unused = 0; 
			goto ours;
		}
#endif
		/*
		 * See if we belong to the destination multicast group on the
		 * arrival interface.
		 */
		IN_LOOKUP_MULTI(ip->ip_dst, m->m_pkthdr.rcvif, inm);
		if (inm == NULL) {
			ipstat.ips_notmember++;
			m_freem(m);
			return;
		}
		goto ours;
	}

#ifdef __CONFIG_TCP_AUTO_MTU__
	struct tcphdr *tp;
	char   *tp_options = NULL;
	unsigned int tcp_len = 0,tcp_i = 0,tcp_option_len = 0,tcp_mss_now = 1460;
	
	if(1 == ip_input_get_mss_enable())
	{
		if(ip->ip_p == IPPROTO_TCP && (m->m_flags & M_PKTHDR) != 0 )
		{
			if(m->m_len > hlen + sizeof(struct tcphdr))
			{
				tp = (struct tcphdr *)((char *)(m->m_data) + (unsigned int)(IP_VHL_HL(ip->ip_vhl) << 2));
				
				if(((tp->th_flags) == TH_SYN || (tp->th_flags) == (TH_SYN|TH_ACK)) &&
					m->m_len >= hlen + (unsigned int)(((tp->th_off)&0x0F)<<2) && 
					(((tp->th_off)&0x0F)<<2) > sizeof(struct tcphdr))
				{					
					tcp_len = (unsigned int)(((tp->th_off)&0x0F)<<2) - sizeof(struct tcphdr);
					tcp_i = 0;
										
					tp_options = (char *)tp + sizeof(struct tcphdr);
										
					while(tcp_i < tcp_len)
					{
						if(tp_options[tcp_i] == 0x01 || tp_options[tcp_i] == 0x00)
						{
							tcp_i++;
							continue;
						}
						
						tcp_option_len = tp_options[tcp_i+1];					
						
						if(tcp_option_len <= 0 || tcp_option_len > 10)
						{
							break;
						}
						
						if(tp_options[tcp_i] == 0x02)
						{
							if(tcp_option_len > 0x4)
							{
								printf("tcp error\n");
							}
							else
							{
								tcp_mss_now = (tp_options[tcp_i+2]&0xff)*16*16+ (tp_options[tcp_i+3]&0xff);
								
								if(tcp_mss_now < MIN_MSS)
									break;
								
								if(tcp_mss_now < ip_input_get_mss())
								{
									ip_input_set_mss(tcp_mss_now);
									ip_input_update_mss(tcp_mss_now);
								}
							}
							break;
						}
						else
						{
							tcp_i += tcp_option_len;
						}
					}
				}
			}
		}
	}
#endif

	if (ip->ip_dst.s_addr == (u_long)INADDR_BROADCAST)
		goto ours;
	if (ip->ip_dst.s_addr == INADDR_ANY)
		goto ours;

#if defined(NFAITH) && 0 < NFAITH
	/*
	 * FAITH(Firewall Aided Internet Translator)
	 */
	if (m->m_pkthdr.rcvif && m->m_pkthdr.rcvif->if_type == IFT_FAITH) {
		if (ip_keepfaith) {
			if (ip->ip_p == IPPROTO_TCP || ip->ip_p == IPPROTO_ICMP) 
				goto ours;
		}
		m_freem(m);
		return;
	}
#endif
	/*
	 * Not for us; forward if possible and desirable.
	 */
	if (ipforwarding == 0) {
		ipstat.ips_cantforward++;
		m_freem(m);
	} else
		ip_forward(m, 0);
#ifdef IPFIREWALL_FORWARD
	ip_fw_fwd_addr = NULL;
#endif
	return;

ours:
	/* Count the packet in the ip address stats */
	if (ia != NULL) {
		ia->ia_ifa.if_ipackets++;
		ia->ia_ifa.if_ibytes += m->m_pkthdr.len;
	}

#if defined(CONFIG_RTL_819X)
	if(m->m_pkthdr.__unused == 0xe5)
	{
		//#include <netinet/udp.h>
		struct udphdr *hdr = (struct udphdr *)((unsigned char *)ip + (IP_VHL_HL(ip->ip_vhl) << 2));
		if (ip->ip_p== IPPROTO_UDP && (hdr->uh_dport== htons(53) || hdr->uh_dport== htons(67))) // DNS Domain or dhcp
			m->m_pkthdr.__unused = 0;
		else if(ip->ip_p== IPPROTO_TCP && hdr->uh_dport== htons(52869) )
			m->m_pkthdr.__unused = 0;
		
		if(m->m_pkthdr.__unused == 0xe5)
		{
			m_freem(m);
			return;
		}
	}
#endif

	/*
	 * If offset or IP_MF are set, must reassemble.
	 * Otherwise, nothing need be done.
	 * (We could look in the reassembly queue to see
	 * if the packet was previously fragmented,
	 * but it's not worth the time; just let them time out.)
	 */
	if (ip->ip_off & (IP_MF | IP_OFFMASK | IP_RF)) {

#if 0	/*
	 * Reassembly should be able to treat a mbuf cluster, for later
	 * operation of contiguous protocol headers on the cluster. (KAME)
	 */
		if (m->m_flags & M_EXT) {		/* XXX */
			if ((m = m_pullup(m, hlen)) == 0) {
				ipstat.ips_toosmall++;
#ifdef IPFIREWALL_FORWARD
				ip_fw_fwd_addr = NULL;
#endif
				return;
			}
			ip = mtod(m, struct ip *);
		}
#endif

		/* If maxnipq is 0, never accept fragments. */
		if (maxnipq == 0) {
                	ipstat.ips_fragments++;
			ipstat.ips_fragdropped++;
			goto bad;
		}

		sum = IPREASS_HASH(ip->ip_src.s_addr, ip->ip_id);
		/*
		 * Look for queue of fragments
		 * of this datagram.
		 */
		for (fp = ipq[sum].next; fp != &ipq[sum]; fp = fp->next)
			if (ip->ip_id == fp->ipq_id &&
			    ip->ip_src.s_addr == fp->ipq_src.s_addr &&
			    ip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
			    ip->ip_p == fp->ipq_p)
				goto found;

		fp = 0;

		/*
		 * Enforce upper bound on number of fragmented packets
		 * for which we attempt reassembly;
		 * If maxnipq is -1, accept all fragments without limitation.
		 */
		if ((nipq > maxnipq) && (maxnipq > 0)) {
		    /*
		     * drop something from the tail of the current queue
		     * before proceeding further
		     */
		    if (ipq[sum].prev == &ipq[sum]) {   /* gak */
			for (i = 0; i < IPREASS_NHASH; i++) {
			    if (ipq[i].prev != &ipq[i]) {
				ip_freef(ipq[i].prev);
				ipstat.ips_fragtimeout++;
				break;
			    }
			}
		    } else {
			ip_freef(ipq[sum].prev);
			ipstat.ips_fragtimeout++;
		    }
		}
found:
		/*
		 * Adjust ip_len to not reflect header,
		 * set ip_mff if more fragments are expected,
		 * convert offset of this to bytes.
		 */
		ip->ip_len -= hlen;
		mff = (ip->ip_off & IP_MF) != 0;
		if (mff) {
		        /*
		         * Make sure that fragments have a data length
			 * that's a non-zero multiple of 8 bytes.
		         */
			if (ip->ip_len == 0 || (ip->ip_len & 0x7) != 0) {
				ipstat.ips_toosmall++; /* XXX */
				goto bad;
			}
			m->m_flags |= M_FRAG;
		}
		ip->ip_off <<= 3;

		/*
		 * If datagram marked as having more fragments
		 * or if this is not the first fragment,
		 * attempt reassembly; if it succeeds, proceed.
		 */
		if (mff || ip->ip_off) {
			ipstat.ips_fragments++;
			m->m_pkthdr.header = ip;
#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
			m = ip_reass(m,
			    fp, &ipq[sum], &divert_info, &divert_cookie);
#else
			m = ip_reass(m, fp, &ipq[sum]);
#endif
			if (m == 0) {
#ifdef IPFIREWALL_FORWARD
				ip_fw_fwd_addr = NULL;
#endif
				return;
			}
			ipstat.ips_reassembled++;
			ip = mtod(m, struct ip *);
			/* Get the header length of the reassembled packet */
			hlen = IP_VHL_HL(ip->ip_vhl) << 2;
#if defined(IPDIVERT)
			/* Restore original checksum before diverting packet */
			if (divert_info != 0) {
				ip->ip_len += hlen;
				HTONS(ip->ip_len);
				HTONS(ip->ip_off);
				ip->ip_sum = 0;
				if (hlen == sizeof(struct ip))
					ip->ip_sum = in_cksum_hdr(ip);
				else
					ip->ip_sum = in_cksum(m, hlen);
				NTOHS(ip->ip_off);
				NTOHS(ip->ip_len);
				#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
				ip_divert_cookie = 0;	/*The reass go back to ip_fw_chk to check all the ipfw rule, because it does not do napt*/
				goto input_again;
				#else
				ip->ip_len -= hlen;
				#endif
			}
#endif
		} else
			if (fp)
				ip_freef(fp);
	} else
		ip->ip_len -= hlen;

#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
	/*
	 * Divert or tee packet to the divert protocol if required.
	 *
	 * If divert_info is zero then cookie should be too, so we shouldn't
	 * need to clear them here.  Assume divert_packet() does so also.
	 */
	if (divert_info != 0) {
		struct mbuf *clone = NULL;

		/* Clone packet if we're doing a 'tee' */
		if ((divert_info & IP_FW_PORT_TEE_FLAG) != 0)
			clone = m_dup(m, M_DONTWAIT);

		/* Restore packet header fields to original values */
		ip->ip_len += hlen;
		HTONS(ip->ip_len);
		HTONS(ip->ip_off);

		/* Deliver packet to divert input routine */
		ip_divert_cookie = divert_cookie;
		divert_packet(m, 1, divert_info & 0xffff);
		ipstat.ips_delivered++;

		/* If 'tee', continue with original packet */
		if (clone == NULL)
			return;
		m = clone;
		ip = mtod(m, struct ip *);
	}
#endif

#ifdef IPSEC
	/*
	 * enforce IPsec policy checking if we are seeing last header.
	 * note that we do not visit this with protocols with pcb layer
	 * code - like udp/tcp/raw ip.
	 */
	if ((inetsw[ip_protox[ip->ip_p]].pr_flags & PR_LASTHDR) != 0 &&
	    ipsec4_in_reject(m, NULL)) {
		ipsecstat.in_polvio++;
		goto bad;
	}
#endif

	/*
	 * Switch out to protocol's input routine.
	 */
	ipstat.ips_delivered++;
    {
	int off = hlen;

	(*inetsw[ip_protox[ip->ip_p]].pr_input)(m, off);
#ifdef	IPFIREWALL_FORWARD
	ip_fw_fwd_addr = NULL;	/* tcp needed it */
#endif
	return;
    }
bad:
#ifdef	IPFIREWALL_FORWARD
	ip_fw_fwd_addr = NULL;
#endif
	m_freem(m);
}

/*
 * IP software interrupt routine - to go away sometime soon
 */
static void
ipintr(void)
{
	int s;
	struct mbuf *m;

	while(1) {
		s = splimp();
		IF_DEQUEUE(&ipintrq, m);
		splx(s);
		if (m == 0)
			return;
		ip_input(m);
	}
}

/*
 * Take incoming datagram fragment and try to reassemble it into
 * whole datagram.  If a chain for reassembly of this datagram already
 * exists, then it is given as fp; otherwise have to make a chain.
 *
 * When IPDIVERT enabled, keep additional state with each packet that
 * tells us if we need to divert or tee the packet we're building.
 */

static struct mbuf *
#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
ip_reass(m, fp, where, divinfo, divcookie)
#else
ip_reass(m, fp, where)
#endif
	register struct mbuf *m;
	register struct ipq *fp;
	struct   ipq    *where;
#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
	u_int32_t *divinfo;
	u_int16_t *divcookie;
#endif
{
	struct ip *ip = mtod(m, struct ip *);
	register struct mbuf *p = 0, *q, *nq;
	struct mbuf *t;
	int hlen = IP_VHL_HL(ip->ip_vhl) << 2;
	int i, next;

	/*
	 * Presence of header sizes in mbufs
	 * would confuse code below.
	 */
	m->m_data += hlen;
	m->m_len -= hlen;

	/*
	 * If first fragment to arrive, create a reassembly queue.
	 */
	if (fp == 0) {
		if ((t = m_get(M_DONTWAIT, MT_FTABLE)) == NULL)
			goto dropfrag;
		fp = mtod(t, struct ipq *);
		insque(fp, where);
		nipq++;
		fp->ipq_ttl = IPFRAGTTL;
		fp->ipq_p = ip->ip_p;
		fp->ipq_id = ip->ip_id;
		fp->ipq_src = ip->ip_src;
		fp->ipq_dst = ip->ip_dst;
		fp->ipq_frags = m;
		m->m_nextpkt = NULL;
#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
		fp->ipq_div_info = 0;
		fp->ipq_div_cookie = 0;
#endif
		goto inserted;
	}

#define GETIP(m)	((struct ip*)((m)->m_pkthdr.header))

	/*
	 * Find a segment which begins after this one does.
	 */
	for (p = NULL, q = fp->ipq_frags; q; p = q, q = q->m_nextpkt)
		if (GETIP(q)->ip_off > ip->ip_off)
			break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us, otherwise
	 * stick new segment in the proper place.
	 *
	 * If some of the data is dropped from the the preceding
	 * segment, then it's checksum is invalidated.
	 */
	if (p) {
		i = GETIP(p)->ip_off + GETIP(p)->ip_len - ip->ip_off;
		if (i > 0) {
			if (i >= ip->ip_len)
				goto dropfrag;
			m_adj(m, i);
			m->m_pkthdr.csum_flags = 0;
			ip->ip_off += i;
			ip->ip_len -= i;
		}
		m->m_nextpkt = p->m_nextpkt;
		p->m_nextpkt = m;
	} else {
		m->m_nextpkt = fp->ipq_frags;
		fp->ipq_frags = m;
	}

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	for (; q != NULL && ip->ip_off + ip->ip_len > GETIP(q)->ip_off;
	     q = nq) {
		i = (ip->ip_off + ip->ip_len) -
		    GETIP(q)->ip_off;
		if (i < GETIP(q)->ip_len) {
			GETIP(q)->ip_len -= i;
			GETIP(q)->ip_off += i;
			m_adj(q, i);
			q->m_pkthdr.csum_flags = 0;
			break;
		}
		nq = q->m_nextpkt;
		m->m_nextpkt = nq;
		m_freem(q);
	}

inserted:

#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
	/*
	 * Transfer firewall instructions to the fragment structure.
	 * Any fragment diverting causes the whole packet to divert.
	 */
	fp->ipq_div_info = *divinfo;
	fp->ipq_div_cookie = *divcookie;
	*divinfo = 0;
	*divcookie = 0;
#endif

	/*
	 * Check for complete reassembly.
	 */
	next = 0;
	for (p = NULL, q = fp->ipq_frags; q; p = q, q = q->m_nextpkt) {
		if (GETIP(q)->ip_off != next)
			return (0);
		next += GETIP(q)->ip_len;
	}
	/* Make sure the last packet didn't have the IP_MF flag */
	if (p->m_flags & M_FRAG)
		return (0);

	/*
	 * Reassembly is complete.  Make sure the packet is a sane size.
	 */
	q = fp->ipq_frags;
	ip = GETIP(q);
	if (next + (IP_VHL_HL(ip->ip_vhl) << 2) > IP_MAXPACKET) {
		ipstat.ips_toolong++;
		ip_freef(fp);
		return (0);
	}

	/*
	 * Concatenate fragments.
	 */
	m = q;
	t = m->m_next;
	m->m_next = 0;
	m_cat(m, t);
	nq = q->m_nextpkt;
	q->m_nextpkt = 0;
	for (q = nq; q != NULL; q = nq) {
		nq = q->m_nextpkt;
		q->m_nextpkt = NULL;
		m->m_pkthdr.csum_flags &= q->m_pkthdr.csum_flags;
		m->m_pkthdr.csum_data += q->m_pkthdr.csum_data;
		m_cat(m, q);
	}

#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
	/*
	 * Extract firewall instructions from the fragment structure.
	 */
	*divinfo = fp->ipq_div_info;
	*divcookie = fp->ipq_div_cookie;
#endif

	/*
	 * Create header for new ip packet by
	 * modifying header of first packet;
	 * dequeue and discard fragment reassembly header.
	 * Make header visible.
	 */
	ip->ip_len = next;
	ip->ip_src = fp->ipq_src;
	ip->ip_dst = fp->ipq_dst;
	remque(fp);
	#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
	rtl_delIpFragEntryByIpq(fp);
	#endif
	nipq--;
	(void) m_free(dtom(fp));
	m->m_len += (IP_VHL_HL(ip->ip_vhl) << 2);
	m->m_data -= (IP_VHL_HL(ip->ip_vhl) << 2);
	/* some debugging cruft by sklower, below, will go away soon */
	if (m->m_flags & M_PKTHDR) { /* XXX this should be done elsewhere */
		register int plen = 0;
		for (t = m; t; t = t->m_next)
			plen += t->m_len;
		m->m_pkthdr.len = plen;
	}
	return (m);

dropfrag:
#if !defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)&&defined(IPDIVERT)
	*divinfo = 0;
	*divcookie = 0;
#endif

	ipstat.ips_fragdropped++;
	m_freem(m);
	return (0);

#undef GETIP
}

/*
 * Free a fragment reassembly header and all
 * associated datagrams.
 */
static void
ip_freef(fp)
	struct ipq *fp;
{
	register struct mbuf *q;

	while (fp->ipq_frags) {
		q = fp->ipq_frags;
		fp->ipq_frags = q->m_nextpkt;
		m_freem(q);
	}
	remque(fp);
	#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
	rtl_delIpFragEntryByIpq(fp);
	#endif
	(void) m_free(dtom(fp));
	nipq--;
}

/*
 * IP timer processing;
 * if a timer expires on a reassembly
 * queue, discard it.
 */
void
ip_slowtimo()
{
	register struct ipq *fp;
	int s = splnet();
	int i;

	for (i = 0; i < IPREASS_NHASH; i++) {
		fp = ipq[i].next;
		if (fp == 0)
			continue;
		while (fp != &ipq[i]) {
			--fp->ipq_ttl;
			fp = fp->next;
			if (fp->prev->ipq_ttl == 0) {
				ipstat.ips_fragtimeout++;
				ip_freef(fp->prev);
			}
		}
	}
	/*
	 * If we are over the maximum number of fragments
	 * (due to the limit being lowered), drain off
	 * enough to get down to the new limit.
	 */
	if (maxnipq >= 0 && nipq > maxnipq) {
		for (i = 0; i < IPREASS_NHASH; i++) {
			while ((nipq > maxnipq) &&
				(ipq[i].next != &ipq[i])) {
				ipstat.ips_fragdropped++;
				ip_freef(ipq[i].next);
			}
		}
	}
	ipflow_slowtimo();
	splx(s);
}

/*
 * Drain off all datagram fragments.
 */
void
ip_drain()
{
	int     i;

	for (i = 0; i < IPREASS_NHASH; i++) {
		while (ipq[i].next != &ipq[i]) {
			ipstat.ips_fragdropped++;
			ip_freef(ipq[i].next);
		}
	}
	in_rtqdrain();
}

/*
 * Do option processing on a datagram,
 * possibly discarding it if bad options are encountered,
 * or forwarding it if source-routed.
 * Returns 1 if packet has been forwarded/freed,
 * 0 if the packet should be processed further.
 */
static int
ip_dooptions(m)
	struct mbuf *m;
{
	register struct ip *ip = mtod(m, struct ip *);
	register u_char *cp;
	register struct ip_timestamp *ipt;
	register struct in_ifaddr *ia;
	int opt, optlen, cnt, off, code, type = ICMP_PARAMPROB, forward = 0;
	struct in_addr *sin, dst;
	n_time ntime;

	dst = ip->ip_dst;
	cp = (u_char *)(ip + 1);
	cnt = (IP_VHL_HL(ip->ip_vhl) << 2) - sizeof (struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[IPOPT_OPTVAL];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else {
			if (cnt < IPOPT_OLEN + sizeof(*cp)) {
				code = &cp[IPOPT_OLEN] - (u_char *)ip;
				goto bad;
			}
			optlen = cp[IPOPT_OLEN];
			if (optlen < IPOPT_OLEN + sizeof(*cp) || optlen > cnt) {
				code = &cp[IPOPT_OLEN] - (u_char *)ip;
				goto bad;
			}
		}
		switch (opt) {

		default:
			break;

		/*
		 * Source routing with record.
		 * Find interface with current destination address.
		 * If none on this machine then drop if strictly routed,
		 * or do nothing if loosely routed.
		 * Record interface address and bring up next address
		 * component.  If strictly routed make sure next
		 * address is on directly accessible net.
		 */
		case IPOPT_LSRR:
		case IPOPT_SSRR:
			if (optlen < IPOPT_OFFSET + sizeof(*cp)) {
				code = &cp[IPOPT_OLEN] - (u_char *)ip;
				goto bad;
			}
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *)ip;
				goto bad;
			}
			ipaddr.sin_addr = ip->ip_dst;
			ia = (struct in_ifaddr *)
				ifa_ifwithaddr((struct sockaddr *)&ipaddr);
			if (ia == 0) {
				if (opt == IPOPT_SSRR) {
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_SRCFAIL;
					goto bad;
				}
				if (!ip_dosourceroute)
					goto nosourcerouting;
				/*
				 * Loose routing, and not at next destination
				 * yet; nothing to do except forward.
				 */
				break;
			}
			off--;			/* 0 origin */
			if (off > optlen - (int)sizeof(struct in_addr)) {
				/*
				 * End of source route.  Should be for us.
				 */
				if (!ip_acceptsourceroute)
					goto nosourcerouting;
				save_rte(cp, ip->ip_src);
				break;
			}

			if (!ip_dosourceroute) {
				if (ipforwarding) {
					char buf[16]; /* aaa.bbb.ccc.ddd\0 */
					/*
					 * Acting as a router, so generate ICMP
					 */
nosourcerouting:
					strcpy(buf, inet_ntoa(ip->ip_dst));
					log(LOG_WARNING, 
					    "attempted source route from %s to %s\n",
					    inet_ntoa(ip->ip_src), buf);
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_SRCFAIL;
					goto bad;
				} else {
					/*
					 * Not acting as a router, so silently drop.
					 */
					ipstat.ips_cantforward++;
					m_freem(m);
					return (1);
				}
			}

			/*
			 * locate outgoing interface
			 */
			(void)memcpy(&ipaddr.sin_addr, cp + off,
			    sizeof(ipaddr.sin_addr));

			if (opt == IPOPT_SSRR) {
#define	INA	struct in_ifaddr *
#define	SA	struct sockaddr *
			    if ((ia = (INA)ifa_ifwithdstaddr((SA)&ipaddr)) == 0)
				ia = (INA)ifa_ifwithnet((SA)&ipaddr);
			} else
				ia = ip_rtaddr(ipaddr.sin_addr);
			if (ia == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_SRCFAIL;
				goto bad;
			}
			ip->ip_dst = ipaddr.sin_addr;
			(void)memcpy(cp + off, &(IA_SIN(ia)->sin_addr),
			    sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			/*
			 * Let ip_intr's mcast routing check handle mcast pkts
			 */
			forward = !IN_MULTICAST(ntohl(ip->ip_dst.s_addr));
			break;

		case IPOPT_RR:
			if (optlen < IPOPT_OFFSET + sizeof(*cp)) {
				code = &cp[IPOPT_OFFSET] - (u_char *)ip;
				goto bad;
			}
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *)ip;
				goto bad;
			}
			/*
			 * If no space remains, ignore.
			 */
			off--;			/* 0 origin */
			if (off > optlen - (int)sizeof(struct in_addr))
				break;
			(void)memcpy(&ipaddr.sin_addr, &ip->ip_dst,
			    sizeof(ipaddr.sin_addr));
			/*
			 * locate outgoing interface; if we're the destination,
			 * use the incoming interface (should be same).
			 */
			if ((ia = (INA)ifa_ifwithaddr((SA)&ipaddr)) == 0 &&
			    (ia = ip_rtaddr(ipaddr.sin_addr)) == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_HOST;
				goto bad;
			}
			(void)memcpy(cp + off, &(IA_SIN(ia)->sin_addr),
			    sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			break;

		case IPOPT_TS:
			code = cp - (u_char *)ip;
			ipt = (struct ip_timestamp *)cp;
			if (ipt->ipt_len < 4 || ipt->ipt_len > 40) {
				code = (u_char *)&ipt->ipt_len - (u_char *)ip;
				goto bad;
			}
			if (ipt->ipt_ptr < 5) {
				code = (u_char *)&ipt->ipt_ptr - (u_char *)ip;
				goto bad;
			}
			if (ipt->ipt_ptr >
			    ipt->ipt_len - (int)sizeof(int32_t)) {
				if (++ipt->ipt_oflw == 0) {
					code = (u_char *)&ipt->ipt_ptr -
					    (u_char *)ip;
					goto bad;
				}
				break;
			}
			sin = (struct in_addr *)(cp + ipt->ipt_ptr - 1);
			switch (ipt->ipt_flg) {

			case IPOPT_TS_TSONLY:
				break;

			case IPOPT_TS_TSANDADDR:
				if (ipt->ipt_ptr - 1 + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len) {
					code = (u_char *)&ipt->ipt_ptr -
					    (u_char *)ip;
					goto bad;
				}
				ipaddr.sin_addr = dst;
				ia = (INA)ifaof_ifpforaddr((SA)&ipaddr,
							    m->m_pkthdr.rcvif);
				if (ia == 0)
					continue;
				(void)memcpy(sin, &IA_SIN(ia)->sin_addr,
				    sizeof(struct in_addr));
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			case IPOPT_TS_PRESPEC:
				if (ipt->ipt_ptr - 1 + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len) {
					code = (u_char *)&ipt->ipt_ptr -
					    (u_char *)ip;
					goto bad;
				}
				(void)memcpy(&ipaddr.sin_addr, sin,
				    sizeof(struct in_addr));
				if (ifa_ifwithaddr((SA)&ipaddr) == 0)
					continue;
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			default:
				/* XXX can't take &ipt->ipt_flg */
				code = (u_char *)&ipt->ipt_ptr -
				    (u_char *)ip + 1;
				goto bad;
			}
			ntime = iptime();
			(void)memcpy(cp + ipt->ipt_ptr - 1, &ntime,
			    sizeof(n_time));
			ipt->ipt_ptr += sizeof(n_time);
		}
	}
	if (forward && ipforwarding) {
		ip_forward(m, 1);
		return (1);
	}
	return (0);
bad:
	icmp_error(m, type, code, 0, 0);
	ipstat.ips_badoptions++;
	return (1);
}

/*
 * Given address of next destination (final or next hop),
 * return internet address info of interface to be used to get there.
 */
static struct in_ifaddr *
ip_rtaddr(dst)
	 struct in_addr dst;
{
	register struct sockaddr_in *sin;

	sin = (struct sockaddr_in *) &ipforward_rt.ro_dst;

	if (ipforward_rt.ro_rt == 0 || dst.s_addr != sin->sin_addr.s_addr) {
		if (ipforward_rt.ro_rt) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = dst;

		rtalloc_ign(&ipforward_rt, RTF_PRCLONING);
	}
	if (ipforward_rt.ro_rt == 0)
		return ((struct in_ifaddr *)0);
	return ((struct in_ifaddr *) ipforward_rt.ro_rt->rt_ifa);
}

/*
 * Save incoming source route for use in replies,
 * to be picked up later by ip_srcroute if the receiver is interested.
 */
void
save_rte(option, dst)
	u_char *option;
	struct in_addr dst;
{
	unsigned olen;

	olen = option[IPOPT_OLEN];
#ifdef DIAGNOSTIC
	if (ipprintfs)
		printf("save_rte: olen %d\n", olen);
#endif
	if (olen > sizeof(ip_srcrt) - (1 + sizeof(dst)))
		return;
	bcopy(option, ip_srcrt.srcopt, olen);
	ip_nhops = (olen - IPOPT_OFFSET - 1) / sizeof(struct in_addr);
	ip_srcrt.dst = dst;
}

/*
 * Retrieve incoming source route for use in replies,
 * in the same form used by setsockopt.
 * The first hop is placed before the options, will be removed later.
 */
struct mbuf *
ip_srcroute()
{
	register struct in_addr *p, *q;
	register struct mbuf *m;

	if (ip_nhops == 0)
		return ((struct mbuf *)0);
	m = m_get(M_DONTWAIT, MT_HEADER);
	if (m == 0)
		return ((struct mbuf *)0);

#define OPTSIZ	(sizeof(ip_srcrt.nop) + sizeof(ip_srcrt.srcopt))

	/* length is (nhops+1)*sizeof(addr) + sizeof(nop + srcrt header) */
	m->m_len = ip_nhops * sizeof(struct in_addr) + sizeof(struct in_addr) +
	    OPTSIZ;
#ifdef DIAGNOSTIC
	if (ipprintfs)
		printf("ip_srcroute: nhops %d mlen %d", ip_nhops, m->m_len);
#endif

	/*
	 * First save first hop for return route
	 */
	p = &ip_srcrt.route[ip_nhops - 1];
	*(mtod(m, struct in_addr *)) = *p--;
#ifdef DIAGNOSTIC
	if (ipprintfs)
		printf(" hops %lx", (u_long)ntohl(mtod(m, struct in_addr *)->s_addr));
#endif

	/*
	 * Copy option fields and padding (nop) to mbuf.
	 */
	ip_srcrt.nop = IPOPT_NOP;
	ip_srcrt.srcopt[IPOPT_OFFSET] = IPOPT_MINOFF;
	(void)memcpy(mtod(m, caddr_t) + sizeof(struct in_addr),
	    &ip_srcrt.nop, OPTSIZ);
	q = (struct in_addr *)(mtod(m, caddr_t) +
	    sizeof(struct in_addr) + OPTSIZ);
#undef OPTSIZ
	/*
	 * Record return path as an IP source route,
	 * reversing the path (pointers are now aligned).
	 */
	while (p >= ip_srcrt.route) {
#ifdef DIAGNOSTIC
		if (ipprintfs)
			printf(" %lx", (u_long)ntohl(q->s_addr));
#endif
		*q++ = *p--;
	}
	/*
	 * Last hop goes to final destination.
	 */
	*q = ip_srcrt.dst;
#ifdef DIAGNOSTIC
	if (ipprintfs)
		printf(" %lx\n", (u_long)ntohl(q->s_addr));
#endif
	return (m);
}

/*
 * Strip out IP options, at higher
 * level protocol in the kernel.
 * Second argument is buffer to which options
 * will be moved, and return value is their length.
 * XXX should be deleted; last arg currently ignored.
 */
void
ip_stripoptions(m, mopt)
	register struct mbuf *m;
	struct mbuf *mopt;
{
	register int i;
	struct ip *ip = mtod(m, struct ip *);
	register caddr_t opts;
	int olen;

	olen = (IP_VHL_HL(ip->ip_vhl) << 2) - sizeof (struct ip);
	opts = (caddr_t)(ip + 1);
	i = m->m_len - (sizeof (struct ip) + olen);
	bcopy(opts + olen, opts, (unsigned)i);
	m->m_len -= olen;
	if (m->m_flags & M_PKTHDR)
		m->m_pkthdr.len -= olen;
	ip->ip_vhl = IP_MAKE_VHL(IPVERSION, sizeof(struct ip) >> 2);
}

int inetctlerrmap[PRC_NCMDS] = {
	0,		0,		0,		0,
	0,		EMSGSIZE,	EHOSTDOWN,	EHOSTUNREACH,
	EHOSTUNREACH,	EHOSTUNREACH,	ECONNREFUSED,	ECONNREFUSED,
	EMSGSIZE,	EHOSTUNREACH,	0,		0,
	0,		0,		0,		0,
	ENOPROTOOPT,	ECONNREFUSED
};
/*add by lq about nat loopback */
extern unsigned int	br0_ip_addr;
extern unsigned int	br0_ip_mask;

int rtl_srcip_dstip_in_lan_subnet(unsigned int srcip, unsigned int dstip)
{
	if (((srcip & br0_ip_mask) == (br0_ip_addr & br0_ip_mask))
		&& ((dstip & br0_ip_mask) == (br0_ip_addr & br0_ip_mask))){
		return 1;	
	}
	return 0;
}
/*lqz add for APCLIENT_DHCPC*/
typedef HEADER DNSHEADER;
#if defined(__CONFIG_TENDA_HTTPD_NORMAL__)||defined(__CONFIG_DNS_REDIRECT__)
extern int dnsmasq_parse_request(DNSHEADER *dnsheader, unsigned int qlen, char *name);
#endif
#if defined(__CONFIG_A9__)&&defined(__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__)
void dns_redirect_pkt(struct mbuf *m,unsigned int dns_ttl)
#else
void dns_redirect_pkt(struct mbuf *m)
#endif
{
    DNSHEADER *pdns;
	register struct udphdr *uh;
    register struct ip *ip = mtod(m, struct ip *);
	struct route ro;   

	int iphlen = IP_VHL_HL(ip->ip_vhl) << 2;
	int off = 0,inc = 0,nlen = 0;
	unsigned long src_addr,dest_addr;
	unsigned short dest_port;
	char newbuf[128];
	struct in_ifaddr *ia;
	
	uh = (struct udphdr *)((caddr_t)ip + iphlen);

	pdns = (DNSHEADER *)((caddr_t)uh + sizeof(struct udphdr));
	off = iphlen + ntohs(uh->uh_ulen);	
	
	unsigned short temp = sizeof(DNSHEADER) | 0xc000;
	unsigned long ttl = 0,addr = 0;
	char *panswer;

//get br0 ip address,learn from if_ether.c->in_arpinput
	for (ia = in_ifaddrhead.tqh_first; ia; ia = ia->ia_link.tqe_next) {
		if (ia->ia_ifp->if_xname[0] == 'e' && ia->ia_ifp->if_xname[1] == 't' && ia->ia_ifp->if_xname[2] == 'h'
			/*&& (ia->ia_ifp->if_xname[3] == '0' || ia->ia_ifp->if_xname[3] == '1')*/) {
			addr = ia->ia_addr.sin_addr.s_addr;
			break;
		}
	}
	struct in_addr ina;
	
	ina.s_addr = addr;

	printf("lan_ip:%s(%x)\n",inet_ntoa(ina),addr);
	
	if(addr == 0) {m_freem(m);return;}
		
//make dns pkt
	pdns->qr = 1;		/* response */
	pdns->aa = 0;		/* authoritive */
	pdns->ra = 1;		/* recursion if available */
	pdns->tc = 0;			/* not truncated */
	pdns->nscount = htons(0);
	pdns->arcount = htons(0);
	pdns->ancount = htons(0);	/* no answers unless changed below */

	/* we know the address */
	pdns->rcode = NOERROR;
	pdns->ancount = htons(1);
	pdns->aa = 1;

	bzero(newbuf,sizeof(newbuf));
	panswer = newbuf;
#if defined(__CONFIG_A9__)&&defined(__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__)
	ttl = dns_ttl;
#endif
	PUTSHORT(temp, panswer);
	PUTSHORT(T_A, panswer);
	PUTSHORT(C_IN, panswer);
	PUTLONG(ttl, panswer);
	PUTSHORT(sizeof(addr), panswer);
	memcpy(panswer, &addr, sizeof(addr));
	panswer += sizeof(addr);
	inc = panswer - newbuf;
	nlen = inc;

//learn form ip_ftp_pxy.c->ippr_ftp_port()
	m_copyback(m, off, nlen, newbuf);
	
#  ifdef	M_PKTHDR
	if (!(m->m_flags & M_PKTHDR))
		m->m_pkthdr.len += inc;
#  endif
	
	src_addr = ip->ip_dst.s_addr;
	dest_addr = ip->ip_src.s_addr;
	printf("src_addr:%s(%x),",inet_ntoa(ip->ip_src),src_addr);
	printf("dest_addr:%s(%x)\n",inet_ntoa(ip->ip_dst),dest_addr);
	
	bzero(ip,iphlen);

//make presudo uip header,learn form l2tp_usrreq.c->ifl2tp_output()
	ip->ip_p = IPPROTO_UDP;
	ip->ip_len = htons(ntohs(uh->uh_ulen) + inc);
	ip->ip_src.s_addr = src_addr ;
	ip->ip_dst.s_addr = dest_addr;
	
	dest_port = uh->uh_sport;
	uh->uh_sport = uh->uh_dport;
	uh->uh_dport = dest_port;
	uh->uh_ulen = ip->ip_len;
	
	uh->uh_sum = 0;
	
	if ((uh->uh_sum = in_cksum(m, iphlen+ntohs(uh->uh_ulen))) == 0)
		uh->uh_sum = 0xffff;
	
	/* Directly output to ip */
	ip->ip_len = iphlen + ntohs(uh->uh_ulen);
	ip->ip_ttl= 128;
	
	bzero(&ro, sizeof ro);

    ip_output(m, 0, &ro, 0, 0);
}
//add end
/*end*/

/*
 * Forward a packet.  If some error occurs return the sender
 * an icmp packet.  Note we can't always generate a meaningful
 * icmp message because icmp doesn't have a large enough repertoire
 * of codes and types.
 *
 * If not forwarding, just drop the packet.  This could be confusing
 * if ipforwarding was zero but some routing protocol was advancing
 * us as a gateway to somewhere.  However, we must let the routing
 * protocol deal with that.
 *
 * The srcrt parameter indicates whether the packet is being forwarded
 * via a source route.
 */
#ifdef NATPT
void
#else
static void
#endif
ip_forward(m, srcrt)
	struct mbuf *m;
	int srcrt;
{
	register struct ip *ip = mtod(m, struct ip *);
	register struct sockaddr_in *sin;
	register struct rtentry *rt;
	int error, type = 0, code = 0;
	struct mbuf *mcopy;
	n_long dest;
	struct ifnet *destifp;
#ifdef IPSEC
	struct ifnet dummyifp;
#endif

	dest = 0;
#ifdef DIAGNOSTIC
	if (ipprintfs)
		printf("forward: src %lx dst %lx ttl %x\n",
		    (u_long)ip->ip_src.s_addr, (u_long)ip->ip_dst.s_addr,
		    ip->ip_ttl);
#endif


	if (m->m_flags & (M_BCAST|M_MCAST) || in_canforward(ip->ip_dst) == 0) {
		ipstat.ips_cantforward++;
		m_freem(m);
		return;
	}
	
	
	int iplen = IP_VHL_HL(ip->ip_vhl) << 2;
	DNSHEADER *dnsheader;
	unsigned int qlen = 0;
	int good_dns_query = 0;
	register struct udphdr *udp;
	
	if(ip->ip_p == IPPROTO_UDP && m->m_len > iplen + sizeof(struct udphdr)){
		udp = (struct udphdr *)((caddr_t)ip + iplen);
		if (udp->uh_dport == ntohs(53))
		{
			dnsheader = (DNSHEADER *)((caddr_t)udp + sizeof(struct udphdr));
			if(udp->uh_ulen > sizeof(struct udphdr))
			{
				char dname[256] = {0};
				qlen = udp->uh_ulen - sizeof(struct udphdr);
				
				if (dnsmasq_parse_request(dnsheader,qlen, dname) != 0)
				{
					good_dns_query = 1;
				if(
				#ifdef __CONFIG_A9__
					(strcmp(dname,"re.tenda.cn") == 0)
				#else
					(strcmp(dname,"tendawifi.com") == 0 || strcmp(dname,"www.tendawifi.com") == 0 )
				#endif
				)
					{
						
#if defined(__CONFIG_A9__)&&defined(__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__)
						dns_redirect_pkt(m,DNS_REQUEST_TTL);
#else
						dns_redirect_pkt(m);
#endif
						return;
					}
				}
				
			}
		}
	}

/* Disable those codes, since we will also able to contorl the traffic even when packets 
do not go through the fastpath, added by zhuhuan on 2016.03.17 */
#if 0  //def __CONFIG_TC__
	extern int tc_control_forward(struct ip *ip, unsigned long lan_addr ,unsigned long lan_mask);

	static unsigned long lan_addr = 0,lan_mask = 0;
	struct in_ifaddr *ia;
	
	if(lan_addr == 0){
	//get br0 ip address,learn from if_ether.c->in_arpinput
		for (ia = in_ifaddrhead.tqh_first; ia; ia = ia->ia_link.tqe_next) {
			if (ia->ia_ifp->if_xname[0] == 'e' && ia->ia_ifp->if_xname[1] == 't' && ia->ia_ifp->if_xname[2] == 'h'
				/*&& (ia->ia_ifp->if_xname[3] == '0' || ia->ia_ifp->if_xname[3] == '1')*/) {
				lan_addr = ntohl(ia->ia_addr.sin_addr.s_addr);
				lan_mask = ia->ia_netmask;
				break;
			}
		}
	}

	if(lan_addr != 0){
		if(tc_control_forward(ip,lan_addr,lan_mask) == -1){
			//drop this pkt
			m_freem(m);
			return;
		}
	}
#endif

#ifdef IPSTEALTH
	if (!ipstealth) {
#endif
		if (ip->ip_ttl <= IPTTLDEC) {
			icmp_error(m, ICMP_TIMXCEED, ICMP_TIMXCEED_INTRANS,
			    dest, 0);
			return;
		}
#ifdef IPSTEALTH
	}
#endif

	sin = (struct sockaddr_in *)&ipforward_rt.ro_dst;
	if ((rt = ipforward_rt.ro_rt) == 0 ||
	    ip->ip_dst.s_addr != sin->sin_addr.s_addr) {
		if (ipforward_rt.ro_rt) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = ip->ip_dst;

		rtalloc_ign(&ipforward_rt, RTF_PRCLONING);
		if (ipforward_rt.ro_rt == 0) {
/*modify by lq*/
			if ((dialup_checkp) && !(*dialup_checkp)())
				m_freem(m);
			else{


				//add by z10312新增  dns重定向只在快速设置前生效需求
				#ifdef __CONFIG_DNS_REDIRECT__
				extern int dns_redirect_disable;
				int iphlen = IP_VHL_HL(ip->ip_vhl) << 2;
				register struct udphdr *uh;
				if( good_dns_query  && (dns_redirect_disable == 0) && 
					(ip->ip_p == IPPROTO_UDP) && ( m->m_len > iphlen + sizeof(struct udphdr)) ){
					uh = (struct udphdr *)((caddr_t)ip + iphlen);
					if (uh->uh_dport == ntohs(53))
					{
#if defined(__CONFIG_A9__)&&defined(__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__)
						dns_redirect_pkt(m,DNS_REQUEST_TTL);
#else
						dns_redirect_pkt(m);
#endif
						return;
					}
				}
#endif
				icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_HOST, dest, 0);
				}
/*modify end*/
			return;
		}
		rt = ipforward_rt.ro_rt;
	}

	/*
	 * Save the IP header and at most 8 bytes of the payload,
	 * in case we need to generate an ICMP message to the src.
	 *
	 * We don't use m_copy() because it might return a reference
	 * to a shared cluster. Both this function and ip_output()
	 * assume exclusive access to the IP header in `m', so any
	 * data in a cluster may change before we reach icmp_error().
	 */
	#ifdef CONFIG_RTL_819X
    mcopy= m_copym2(m, 0, imin((IP_VHL_HL(ip->ip_vhl) << 2) + 8,(int)ip->ip_len),M_DONTWAIT);
    if((mcopy == NULL))
    {
        diag_printf("%s %d m_copym2 error !!\n", __FUNCTION__, __LINE__);
    }
    #else    
	MGET(mcopy, M_DONTWAIT, m->m_type);
	if (mcopy != NULL) {
		M_COPY_PKTHDR(mcopy, m);
		mcopy->m_len = imin((IP_VHL_HL(ip->ip_vhl) << 2) + 8,
		    (int)ip->ip_len);
		m_copydata(m, 0, mcopy->m_len, mtod(mcopy, caddr_t));
	}
    #endif
#ifdef IPSTEALTH
	if (!ipstealth) {
#endif
		ip->ip_ttl -= IPTTLDEC;
#ifdef IPSTEALTH
	}
#endif

	/*
	 * If forwarding packet using same interface that it came in on,
	 * perhaps should send a redirect to sender to shortcut a hop.
	 * Only send redirect if source is sending directly to us,
	 * and if packet was not source routed (or has any options).
	 * Also, don't send redirect if forwarding using a default route
	 * or a route modified by a redirect.
	 */
#define	satosin(sa)	((struct sockaddr_in *)(sa))
	if (rt->rt_ifp == m->m_pkthdr.rcvif &&
	    (rt->rt_flags & (RTF_DYNAMIC|RTF_MODIFIED)) == 0 &&
	    satosin(rt_key(rt))->sin_addr.s_addr != 0 &&
	    ipsendredirects && !srcrt		
		&& !rtl_srcip_dstip_in_lan_subnet(ip->ip_src.s_addr, ip->ip_dst.s_addr)   /*add by lq about nat loopback */
		) {
#define	RTA(rt)	((struct in_ifaddr *)(rt->rt_ifa))
		u_long src = ntohl(ip->ip_src.s_addr);

		if (RTA(rt) &&
		    (src & RTA(rt)->ia_subnetmask) == RTA(rt)->ia_subnet) {
		    if (rt->rt_flags & RTF_GATEWAY)
			dest = satosin(rt->rt_gateway)->sin_addr.s_addr;
		    else
			dest = ip->ip_dst.s_addr;
		    /* Router requirements says to only send host redirects */
		    type = ICMP_REDIRECT;
		    code = ICMP_REDIRECT_HOST;
#ifdef DIAGNOSTIC
		    if (ipprintfs)
		        printf("redirect (%d) to %lx\n", code, (u_long)dest);
#endif
		}
	}

	error = ip_output(m, (struct mbuf *)0, &ipforward_rt, 
			  IP_FORWARDING, 0);
	if (error)
		ipstat.ips_cantforward++;
	else {
		ipstat.ips_forward++;
		if (type)
			ipstat.ips_redirectsent++;
		else {
			if (mcopy) {
				ipflow_create(&ipforward_rt, mcopy);
				m_freem(mcopy);
			}
			return;
		}
	}
	if (mcopy == NULL)
		return;
	destifp = NULL;

	switch (error) {

	case 0:				/* forwarded, but need redirect */
		/* type, code set above */
		break;

	case ENETUNREACH:		/* shouldn't happen, checked above */
	case EHOSTUNREACH:
	case ENETDOWN:
	case EHOSTDOWN:
	default:
		type = ICMP_UNREACH;
		code = ICMP_UNREACH_HOST;
		break;

	case EMSGSIZE:
		type = ICMP_UNREACH;
		code = ICMP_UNREACH_NEEDFRAG;
#ifndef IPSEC
		if (ipforward_rt.ro_rt)
			destifp = ipforward_rt.ro_rt->rt_ifp;
#else
		/*
		 * If the packet is routed over IPsec tunnel, tell the
		 * originator the tunnel MTU.
		 *	tunnel MTU = if MTU - sizeof(IP) - ESP/AH hdrsiz
		 * XXX quickhack!!!
		 */
		if (ipforward_rt.ro_rt) {
			struct secpolicy *sp = NULL;
			int ipsecerror;
			int ipsechdr;
			struct route *ro;

			sp = ipsec4_getpolicybyaddr(mcopy,
						    IPSEC_DIR_OUTBOUND,
			                            IP_FORWARDING,
			                            &ipsecerror);

			if (sp == NULL)
				destifp = ipforward_rt.ro_rt->rt_ifp;
			else {
				/* count IPsec header size */
				ipsechdr = ipsec4_hdrsiz(mcopy,
							 IPSEC_DIR_OUTBOUND,
							 NULL);

				/*
				 * find the correct route for outer IPv4
				 * header, compute tunnel MTU.
				 *
				 * XXX BUG ALERT
				 * The "dummyifp" code relies upon the fact
				 * that icmp_error() touches only ifp->if_mtu.
				 */
				/*XXX*/
				destifp = NULL;
				if (sp->req != NULL
				 && sp->req->sav != NULL
				 && sp->req->sav->sah != NULL) {
					ro = &sp->req->sav->sah->sa_route;
					if (ro->ro_rt && ro->ro_rt->rt_ifp) {
						dummyifp.if_mtu =
						    ro->ro_rt->rt_ifp->if_mtu;
						dummyifp.if_mtu -= ipsechdr;
						destifp = &dummyifp;
					}
				}

				key_freesp(sp);
			}
		}
#endif /*IPSEC*/
		ipstat.ips_cantfrag++;
		break;

	case ENOBUFS:
#ifdef ALTQ
		/*
		 * don't generate ICMP_SOURCEQUENCH
		 * (RFC1812 Requirements for IP Version 4 Routers)
		 */
		if (mcopy)
			m_freem(mcopy);
		return;
#else
		type = ICMP_SOURCEQUENCH;
		code = 0;
		break;
#endif

	case EACCES:			/* ipfw denied packet */
		m_freem(mcopy);
		return;
	}
	icmp_error(mcopy, type, code, dest, destifp);
}

void
ip_savecontrol(inp, mp, ip, m)
	register struct inpcb *inp;
	register struct mbuf **mp;
	register struct ip *ip;
	register struct mbuf *m;
{
	if (inp->inp_socket->so_options & SO_TIMESTAMP) {
		struct timeval tv;

		microtime(&tv);
		*mp = sbcreatecontrol((caddr_t) &tv, sizeof(tv),
			SCM_TIMESTAMP, SOL_SOCKET);
		if (*mp)
			mp = &(*mp)->m_next;
	}
	if (inp->inp_flags & INP_RECVDSTADDR) {
		*mp = sbcreatecontrol((caddr_t) &ip->ip_dst,
		    sizeof(struct in_addr), IP_RECVDSTADDR, IPPROTO_IP);
		if (*mp)
			mp = &(*mp)->m_next;
	}
#ifdef notyet
	/* XXX
	 * Moving these out of udp_input() made them even more broken
	 * than they already were.
	 */
	/* options were tossed already */
	if (inp->inp_flags & INP_RECVOPTS) {
		*mp = sbcreatecontrol((caddr_t) opts_deleted_above,
		    sizeof(struct in_addr), IP_RECVOPTS, IPPROTO_IP);
		if (*mp)
			mp = &(*mp)->m_next;
	}
	/* ip_srcroute doesn't do what we want here, need to fix */
	if (inp->inp_flags & INP_RECVRETOPTS) {
		*mp = sbcreatecontrol((caddr_t) ip_srcroute(),
		    sizeof(struct in_addr), IP_RECVRETOPTS, IPPROTO_IP);
		if (*mp)
			mp = &(*mp)->m_next;
	}
#endif
	if (inp->inp_flags & INP_RECVIF) {
		struct ifnet *ifp;
		struct sdlbuf {
			struct sockaddr_dl sdl;
			u_char	pad[32];
		} sdlbuf;
		struct sockaddr_dl *sdp;
		struct sockaddr_dl *sdl2 = &sdlbuf.sdl;

		if (((ifp = m->m_pkthdr.rcvif)) 
		&& ( ifp->if_index && (ifp->if_index <= if_index))) {
			sdp = (struct sockaddr_dl *)(ifnet_addrs
					[ifp->if_index - 1]->ifa_addr);
			/*
			 * Change our mind and don't try copy.
			 */
			if ((sdp->sdl_family != AF_LINK)
			|| (sdp->sdl_len > sizeof(sdlbuf))) {
				goto makedummy;
			}
			bcopy(sdp, sdl2, sdp->sdl_len);
		} else {
makedummy:	
			sdl2->sdl_len
				= offsetof(struct sockaddr_dl, sdl_data[0]);
			sdl2->sdl_family = AF_LINK;
			sdl2->sdl_index = 0;
			sdl2->sdl_nlen = sdl2->sdl_alen = sdl2->sdl_slen = 0;
		}
		*mp = sbcreatecontrol((caddr_t) sdl2, sdl2->sdl_len,
			IP_RECVIF, IPPROTO_IP);
		if (*mp)
			mp = &(*mp)->m_next;
	}
}

int
ip_rsvp_init(struct socket *so)
{
	if (so->so_type != SOCK_RAW ||
	    so->so_proto->pr_protocol != IPPROTO_RSVP)
	  return EOPNOTSUPP;

	if (ip_rsvpd != NULL)
	  return EADDRINUSE;

	ip_rsvpd = so;
	/*
	 * This may seem silly, but we need to be sure we don't over-increment
	 * the RSVP counter, in case something slips up.
	 */
	if (!ip_rsvp_on) {
		ip_rsvp_on = 1;
		rsvp_on++;
	}

	return 0;
}

int
ip_rsvp_done(void)
{
	ip_rsvpd = NULL;
	/*
	 * This may seem silly, but we need to be sure we don't over-decrement
	 * the RSVP counter, in case something slips up.
	 */
	if (ip_rsvp_on) {
		ip_rsvp_on = 0;
		rsvp_on--;
	}
	return 0;
}

#if defined(CONFIG_RTL_IP_FRAG_OUT_OF_ORDER_SUPPORT)
#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED (-1)
#endif
static void rtl_initIpFrag(void)
{
	int i;
	
	for (i=0; i<RTL_IP_FRAG_NHASH; i++)
	    rtl_ipq[i].next = rtl_ipq[i].prev = &rtl_ipq[i];
	
	callout_init(&rtl_ip_frag_timer);
	callout_reset(&rtl_ip_frag_timer, RTL_IP_FRAG_HECK , rtl_ipFragTimeout, 0);
}

static void rtl_ipFragTimeout(void)
{
	int i, s;
	unsigned long now;
	struct rtl_ipq *fp;

	//s = splnet();
	now = (unsigned long)jiffies;
	for(i=0; i<RTL_IP_FRAG_NHASH; i++)
	{
		for (fp = rtl_ipq[i].next; fp != &rtl_ipq[i]; fp = fp->next)
		{
			if(NULL == fp)
				break;
			
			if (fp&&((fp->last_used+RTL_IP_FRAG_TIMEOUT*HZ) < now)){
				remque(fp);
				free(fp, MT_FTABLE);
				rtl_ipq_num--;
			}
		}
	}
	//splx(s);
	callout_reset(&rtl_ip_frag_timer, RTL_IP_FRAG_HECK, rtl_ipFragTimeout, 0);
}

u_short rtl_ipFragHash(unsigned long sIp, u_short id)
{
	u_short hash = 0;

	hash = ((((sIp & 0xF) | (((sIp >> 8) & 0xF) << 4)) ^ id) & (RTL_IP_FRAG_NHASH-1));
	return hash;
}

struct rtl_ipq* rtl_lookupIpFragEntry(struct ip* pip)
{
	int s;
	u_short hash;
	struct rtl_ipq *fp;

	hash = rtl_ipFragHash(pip->ip_src.s_addr, pip->ip_id);
	//s = splnet();
	for (fp = rtl_ipq[hash].next; fp != &rtl_ipq[hash]; fp = fp->next)
	{
		if (NULL == fp)
			break;
		
		if (fp&&(pip->ip_id == fp->ipq_id &&
		    pip->ip_src.s_addr == fp->ipq_src.s_addr &&
		    pip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
		    pip->ip_p == fp->ipq_p)){
		    	fp->last_used = (unsigned long)jiffies;
			//diag_printf("lookup: sip is 0x%x, dip is 0x%x, id is %d, proto is %d\n", fp->ipq_src.s_addr, fp->ipq_dst.s_addr,
			//		        fp->ipq_id, fp->ipq_p);
		    	//splx(s);
			return fp;
		}
	}

	//splx(s);
	return NULL;
}

int rtl_addIpFragEntry(struct ip* pip)
{
	int s;
	u_short hash;
	struct rtl_ipq *fp;

	hash = rtl_ipFragHash(pip->ip_src.s_addr, pip->ip_id);
	if(rtl_lookupIpFragEntry(pip))
		return -2;	//frag entry already exist.

	if(rtl_ipq_num > rtl_ipq_max)
		return -3;	//rtl ip frag table is full

	fp = malloc(sizeof(struct rtl_ipq), MT_FTABLE, M_NOWAIT);
	if(fp==NULL)
		return FAILED;	//can not malloc struct rtl_ipq
	memset(fp, 0, sizeof(struct rtl_ipq));

	//s = splnet();
	insque(fp, &rtl_ipq[hash]);
	//splx(s);
	rtl_ipq_num++;
	fp->ipq_p = pip->ip_p;
	fp->ipq_id = pip->ip_id;
	fp->ipq_src = pip->ip_src;
	fp->ipq_dst = pip->ip_dst;
	fp->last_used = (unsigned long)jiffies;
	//diag_printf("add: sip is 0x%x, dip is 0x%x, id is %d, proto is %d\n", fp->ipq_src.s_addr, fp->ipq_dst.s_addr,
	//			fp->ipq_id, fp->ipq_p);

	return SUCCESS;
}

int rtl_delIpFragEntry(struct ip* pip)
{
	int s;
	u_short hash;
	struct rtl_ipq *fp;

	hash = rtl_ipFragHash(pip->ip_src.s_addr, pip->ip_id);
	//s = splnet();
	for (fp = rtl_ipq[hash].next; fp != &rtl_ipq[hash]; fp = fp->next)
	{
		if (NULL == fp)
			break;
		
		if (fp&&(pip->ip_id == fp->ipq_id &&
		    pip->ip_src.s_addr == fp->ipq_src.s_addr &&
		    pip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
		    pip->ip_p == fp->ipq_p)){
		    	remque(fp);
			free(fp, MT_FTABLE);
			rtl_ipq_num--;
			//splx(s);
			return SUCCESS;
		}
	}	
	//splx(s);
	
	return FAILED;
}

int rtl_delIpFragEntryByIpq(struct ipq* fp_tmp)
{
	int s;
	u_short hash;
	struct rtl_ipq *fp;

	hash = rtl_ipFragHash(fp_tmp->ipq_src.s_addr, fp_tmp->ipq_id);
	//s = splnet();
	for (fp = rtl_ipq[hash].next; fp != &rtl_ipq[hash]; fp = fp->next)
	{
		if (NULL == fp)
			break;
		
		if (fp&&(fp_tmp->ipq_id == fp->ipq_id &&
		    fp_tmp->ipq_src.s_addr == fp->ipq_src.s_addr &&
		    fp_tmp->ipq_dst.s_addr == fp->ipq_dst.s_addr &&
		    fp_tmp->ipq_p == fp->ipq_p)){
		    	remque(fp);
			free(fp, MT_FTABLE);
			rtl_ipq_num--;
			//splx(s);
			return SUCCESS;
		}
	}	
	//splx(s);
	
	return FAILED;
}
#endif
