/*
 * $ Copyright Open Broadcom Corporation $
 * 
 * $Id: net_lltd.c,v 1.1 2010-03-29 10:40:36 musterc Exp $
 */
#include <typedefs.h>
#include <sys/param.h>
#include <sys/mbuf.h>

#include <proto/ethernet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

int	topoinput
(
	struct ifnet *ifp,
	struct ether_header *eh,
	struct mbuf *n
)
{
	int len;
	struct mbuf *m;
	int iphlen = sizeof (struct ip);
	int udphlen = sizeof (struct udphdr); 
	int ehlen = sizeof (struct ether_header);
	int hdrlen = iphlen + udphlen;
	struct ip *ip;
	struct udphdr *uh;
	struct in_ifaddr *ia;
	struct sockaddr_in *sia;

	IFP_TO_IA((ifp), ia);
	if (ia == NULL)
	{
		// find from mbuf again.
		IFP_TO_IA((n->m_pkthdr.rcvif), ia);
		if (ia == NULL)
		{
			m_freem(n);
			diag_printf("%s: no interface address\n", __func__);
			return -1; //Error caller need to free mbuf
		}
	}
	
	sia = (struct sockaddr_in *)(ia->ia_ifa.ifa_addr);
	
	// Move back to ether header
	n->m_data -= ehlen;
	n->m_len += ehlen;
	n->m_pkthdr.len += ehlen;
	len = n->m_pkthdr.len;
	
	// Allocate a new m for header
	MGET(m, M_DONTWAIT, n->m_type);
	if (m == NULL) {
		m_freem(n);
		return 0;
	}
	if (n->m_flags & M_PKTHDR)
		M_MOVE_PKTHDR(m, n);
	
	m->m_next = n;
	
	// Do IP copy
	ip = mtod(m, struct ip *);
	
	/* Fill in ip header data */
	ip->ip_v = IPVERSION;
	ip->ip_tos = 0;
	ip->ip_len = hdrlen + len;
	ip->ip_off &= IP_DF;
#ifdef RANDOM_IP_ID
	ip->ip_id = ip_randomid();
#else
	ip->ip_id = htons(ip_id++);
#endif
	ip->ip_hl = iphlen >> 2;
	ip->ip_ttl = 1;
	ip->ip_p = IPPROTO_UDP;
	ip->ip_sum = 0;
	ip->ip_src = sia->sin_addr;
	ip->ip_dst.s_addr = 0xFFFFFFFF;
	
	/* Fill in udp header data */
	uh = (struct udphdr *)(ip+1);
	
	uh->uh_sport = eh->ether_type;
	uh->uh_dport = eh->ether_type;
	uh->uh_ulen = htons(ip->ip_len - iphlen);
	uh->uh_sum = 0;
	
	// Set packet header length again
	m->m_len = hdrlen;
	m->m_pkthdr.len = ip->ip_len;
	
	/* send to udp handler */
	udp_input(m, iphlen, NULL, 0);
	
	return 0; //ok, callee handle this mbuf
}
