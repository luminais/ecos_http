/*
*禁止从WAN发报文到LAN侧
*roy add,2011/11/04
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/malloc.h>
#include <ip_compat.h>
#include <wan2lanf.h>

static unsigned int wan2lan_lan_ip = 0;
static unsigned int wan2lan_lan_mask = 0;

extern int (*wan2lanfilter_checkp)(struct ifnet *ifp, char *eh, struct mbuf *m);

int
wan2lanfilter_match(struct ifnet *ifp, char *head, struct mbuf *m)
{
	struct in_ifaddr *ia;
	struct ip *ip;

	if(head == NULL)
		return 0;

	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) != 0)//from wan side
		return 0;

	if (m->m_pkthdr.len < 40)
		return 0;

	/* Check destination IP, if it is to the interface IP, pass it */
	ip = mtod(m, struct ip *);

#if defined(__OpenBSD__)
	for (ia = in_ifaddr.tqh_first; ia; ia = ia->ia_list.tqe_next) {
		if (ip->ip_dst.s_addr == ia->ia_addr.sin_addr.s_addr)
			return 0;
	}
#else
	TAILQ_FOREACH(ia, &in_ifaddrhead, ia_link)
		if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr)
			return 0;
#endif
	//如果从WAN进来的报文目的地址是LAN侧的，就丢掉
	if((ip->ip_dst.s_addr&wan2lan_lan_mask) == (wan2lan_lan_ip & wan2lan_lan_mask)){
		return 1;
	}else{
		return 0;	
	}	
}


void
wan2lanfilter_act(void)
{
	wan2lanfilter_checkp = wan2lanfilter_match;
}


void
wan2lanfilter_inact(void)
{
	wan2lanfilter_checkp = 0;
}

int wan2lanf_set_ip(int ip)
{
	wan2lan_lan_ip = (unsigned int)ip;
	return 0;
}

int wan2lanf_set_mask(int mask)
{
	wan2lan_lan_mask = (unsigned int)mask;
	return 0;
}

int wan2lanfilter_set_mode(int mode)
{
	int error = 0;

	switch (mode) {
	case WAN2LANF_MODE_DISABLED:
		wan2lanfilter_inact();
		break;
	case WAN2LANF_MODE_ENABLED:
		wan2lanfilter_act();
		break;
	default:
		error = EINVAL;
		break;
	}
	return error;
}

