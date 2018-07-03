/*
 * NAT fast routing for ip_filter
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: ip_fastnat.c,v 1.2 2010-05-11 10:21:15 Exp $
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/udp.h>
#include <ip_compat.h>
#include <ip_fil.h>
#include <ip_nat.h>
#include <ip_frag.h>

#include <router_net.h>
extern u_long fr_tcpidletimeout;
extern int (*ip_fastpath)(struct ifnet *ifp, struct mbuf **mp);

#ifdef __CONFIG_TC__
extern int stream_control(int nat_dir, int nflags, ip_t *ip/*, fr_info_t *fin*/);
extern  int g_user_rate_control;
#ifdef __CONFIG_TC_WINDOWS__
extern int up_win_stream_control(int nat_dir, ip_t *ip);
extern int down_win_stream_control(int nat_dir, ip_t *ip);
#endif	/*__CONFIG_TC_WINDOWS__*/

#ifdef __CONFIG_STREAM_STATISTIC__
extern int stream_statistic(ip_t * ip, int direction);
extern int g_user_stream_statistic;
#endif/*__CONFIG_STREAM_STATISTIC__*/
#endif/*__CONFIG_TC__*/

#ifdef __CONFIG_STAR_WARS_ENABLE__

struct star_wars_game_list{
	unsigned long  private_net_ip;
	unsigned short game_port;
};

#define MAX_STAR_WARS_MAP 128
static struct star_wars_game_list star_wars_map[MAX_STAR_WARS_MAP];

static unsigned short chsum_fixup_long_tenda(unsigned short sum,unsigned long old_data,
						   unsigned long new_data)
{
	unsigned long sum_new;

	old_data = (old_data >> 16) + (old_data & 0x0000FFFF);
	new_data = (new_data >> 16) + (new_data & 0x0000FFFF);

	sum_new = sum + old_data  - new_data;
	sum_new = (sum_new >> 16) + (sum_new & 0x0000FFFF);

	return (unsigned short)sum_new;
}

#endif


#ifdef TENDA_WLJB
extern char g_wan_ip[20];
extern char g_private_ip[20];

extern int qq_packet_change(struct ip *pIpHdr,char *private_ip,char *wan_ip);
extern int xunlei_packet_change(struct ip *pIpHdr,char *private_ip,char *wan_ip);
extern int tenda_change_packet2(struct ip *pIpHdr);

#endif


/*
 * Set this NAT entry as a fast cache.
 * Only a non-ALG entry with tcp/udp
 * type
 */
int
nat_set_fcache(fr_info_t *fin, nat_t *nat)
{
	struct in_ifaddr *ia;
	struct sockaddr_in sin = {sizeof(sin), AF_INET};

	/* through ALG can not set into cache */
	if ((nat->nat_aps != NULL) ||
		(nat->nat_flags & IPN_FASTCACHE))
		return -1;
	if ((nat->nat_p != IPPROTO_TCP) && (nat->nat_p != IPPROTO_UDP))
		return -1;
	if (((struct ifnet *)nat->nat_ifp)->if_fltflags & IFFLT_NAT)
		return -1;
#if defined(__OpenBSD__)
	for (ia = in_ifaddr.tqh_first; ia; ia = ia->ia_list.tqe_next) {
		if (nat->nat_inip.s_addr == ia->ia_addr.sin_addr.s_addr)
			return -1;
	}
#else
	TAILQ_FOREACH(ia, &in_ifaddrhead, ia_link)
		if (IA_SIN(ia)->sin_addr.s_addr == nat->nat_inip.s_addr)
			return -1;
#endif
	/* obtain destination route entry */
	sin.sin_addr = nat->nat_oip;
#if defined(__OpenBSD__)
	nat->nat_rt = (struct rtentry *)rtalloc1((struct sockaddr *)&sin, 1);
#else
	nat->nat_rt = (struct rtentry *)rtalloc1((struct sockaddr *)&sin, 1, RTF_PRCLONING);
#endif
	if (nat->nat_rt == 0)
		return -1;
	/* obtain source route entry and interface */
	sin.sin_addr = nat->nat_inip;
#if defined(__OpenBSD__)
	nat->nat_ort = (struct rtentry *)rtalloc1((struct sockaddr *)&sin, 1);
#else
	nat->nat_ort = (struct rtentry *)rtalloc1((struct sockaddr *)&sin, 1, RTF_PRCLONING);
#endif
	if (nat->nat_ort == 0) {
		rtfree(nat->nat_rt);
		nat->nat_rt = 0;
		return -1;
	}
	return 0;
}

/*
 * Set this NAT entry as noncached.
 * Usually do for tcp finish state.
 */
void
nat_unset_fcache(nat_t *nat)
{
	nat->nat_flags &= ~IPN_FASTCACHE;
}

/*
 * For a tcp entry, check the tcp sync
 * completed, or tcp finish started.
 * For a udp entry, set to cache always.
 */ 
void
nat_check_fcache(nat_t *nat, tcphdr_t *tcp)
{
	frentry_t *fr;

	fr = nat->nat_fstate.rule;
	/* Only FW pass seesion can fast-out */
	if ((fr = nat->nat_fstate.rule)) {
		if (!(fr->fr_flags & FR_PASS))
			return;
	}
	/*
	 * Do nothing if the fast route for
	 * not both input and output being
	 * allocated.
	 */ 
	if (!nat->nat_rt || !nat->nat_ort)
		return;
	switch (nat->nat_p) {
	case IPPROTO_TCP:
		if (nat->nat_tcpstate[nat->nat_dir] >= TCPS_CLOSE_WAIT ||
		    nat->nat_tcpstate[1 - nat->nat_dir] >= TCPS_CLOSE_WAIT) {
			/*
			 * Let slow path handle the nat fade-out
			 * elagently.
			 */
			nat->nat_flags &= ~IPN_FASTCACHE;
		} else if (nat->nat_tcpstate[nat->nat_dir] > TCPS_SYN_RECEIVED &&
		           nat->nat_tcpstate[1 - nat->nat_dir] > TCPS_SYN_RECEIVED) {
			/* tcp sync complete, do fast nat */
			nat->nat_flags |= IPN_FASTCACHE;
		}
		break;
	case IPPROTO_UDP:
		nat->nat_flags |= IPN_FASTCACHE;
		break;
	default:
		break;
	}
}

/* Set all the NAT entries to noncached */
void
flush_fcache(void)
{
	register struct nat *nat;

	for (nat = nat_instances; nat; nat = nat->nat_next) {
		nat->nat_fstate.rule = 0;
		if (nat->nat_flags & IPN_FASTCACHE)
			nat->nat_flags &= ~IPN_FASTCACHE;
	}
}

static void
nat_fix_outcksum(int dlen, u_short *sp, u_32_t n)
{
	register u_short sumshort;
	register u_32_t sum1;

	if (!n)
		return;
	else if (n & NAT_HW_CKSUM) {
		n &= 0xffff;
		n += dlen;
		n = (n & 0xffff) + (n >> 16);
		*sp = n & 0xffff;
		return;
	}
	sum1 = (~ntohs(*sp)) & 0xffff;
	sum1 += (n);
	sum1 = (sum1 >> 16) + (sum1 & 0xffff);
	/* Again */
	sum1 = (sum1 >> 16) + (sum1 & 0xffff);
	sumshort = ~(u_short)sum1;
	*(sp) = htons(sumshort);
}

static void
nat_fix_incksum(int dlen, u_short *sp, u_32_t n)
{
	register u_short sumshort;
	register u_32_t sum1;

	if (!n)
		return;
	else if (n & NAT_HW_CKSUM) {
		n &= 0xffff;
		n += dlen;
		n = (n & 0xffff) + (n >> 16);
		*sp = n & 0xffff;
		return;
	}
	sum1 = (~ntohs(*sp)) & 0xffff;
	sum1 += ~(n) & 0xffff;
	sum1 = (sum1 >> 16) + (sum1 & 0xffff);
	/* Again */
	sum1 = (sum1 >> 16) + (sum1 & 0xffff);
	sumshort = ~(u_short)sum1;
	*(sp) = htons(sumshort);
}

//tenda add for pppoe on demand

//llm, 修改为extern，dns模块已调试通过，不需要临时定义
extern int dns_query_on_idel_time;
//int dns_query_on_idel_time;
/* Fast path of NAT process */
int
ipnat_fastpath(struct ifnet *ifp, struct mbuf **mp)
{
	register struct mbuf *m = *mp;
	register struct ip *ip;
	register tcphdr_t *tcp;
	register nat_t *nat;
	struct ifnet *difp = 0;
	struct rtentry *drt = 0;
	struct sockaddr_in sin = {sizeof(sin), AF_INET};
	struct sockaddr_in *dst;
	unsigned short *csump = NULL;
	unsigned int hlen;
	unsigned int hv;

	if (m->m_pkthdr.len < 40)
		 goto out;
	ip = mtod(m, struct ip *);
	/*
	 * Fast path checking criteria,
	 * 1. Must inclue ip header, tcp/udp ports.
	 * 2. Only handle the IPv4.
	 * 3. Only handle ip header with no options.
	 * 4. Don't deal with fragment.
	 * 5. ip_ttl is not empty.
	 */
	 //验证IP头部的有效性
	if (m->m_len < 24 ||
	    ip->ip_v != 4 ||
	    ip->ip_hl != sizeof(struct ip)/4 ||
	    m->m_pkthdr.len < ntohs(ip->ip_len) ||
	    (ip->ip_off & htons(0x3fff)) != 0 ||
	    //tenda modify for xi'an
	    //ip->ip_ttl <= IPTTLDEC)
	    ip->ip_ttl < 0)
		goto out;
	hlen = ip->ip_hl << 2;
	/* And also check the ip checksum */
	if (in_cksum(m, hlen) != 0)
		goto out;
	//后面是tcp
	tcp = (tcphdr_t *)((char *)ip + hlen);
	/* inbound */
	hv = NAT_HASH_FN(ip->ip_dst.s_addr, tcp->th_dport, 0xffffffff);
	hv = NAT_HASH_FN(ip->ip_src.s_addr, hv + tcp->th_sport, ipf_nattable_sz);
	nat = nat_table[1][hv];
	for (; nat; nat = nat->nat_hnext[1]) {
		/* Do NAT entry inbound lookup */
		if ((nat->nat_flags & IPN_FASTCACHE) == 0 ||
		    ifp != nat->nat_ifp ||
		    nat->nat_oip.s_addr != ip->ip_src.s_addr ||
		    nat->nat_outip.s_addr != ip->ip_dst.s_addr ||
		    ip->ip_p != nat->nat_p ||
		    tcp->th_sport != nat->nat_oport ||
		    tcp->th_dport != nat->nat_outport)
			continue;
		if ((drt = nat->nat_ort) == NULL)
			goto out;
		/*
		 * Do not deal packet larger than interface mtu,
		 * because the we have to do fragment.
		 */
		difp = drt->rt_ifp;
		if (difp->if_mtu < ntohs(ip->ip_len))
			goto out;
		switch (ip->ip_p) {
		case IPPROTO_TCP:
			if (tcp->th_flags & (TH_RST | TH_FIN | TH_SYN))
				goto out;
//星际争霸服务器和客户端口默认时都是6112
//查找星际争霸 登录战网时的数据包(包含特征开头两个字节为 ff50)，
//并做alg把数据包中的私网Ip,修改成路由器的公网ip
#ifdef __CONFIG_STAR_WARS_ENABLE__
			{
				unsigned char *p = NULL,*pdata = NULL;
				int data_len, private_ip;
				//int lan_ip, lanip, wan_ip, wanip;
				int lan_ip, wan_ip;
				unsigned char net_ip_buf[20], wan_ip_buf[20];
				if(ntohs(tcp->th_dport) == 0x6112)
				{
					 pdata = (unsigned char *)tcp + (tcp->th_off * 4) ;
					 if ( (*pdata == 0xff) && (*(pdata + 1) == 0x50))
					 {
						lan_ip=ntohl(SYS_lan_ip);
						sprintf((char *)net_ip_buf,"%c%c%c",
												(lan_ip>>24)&0xff,(lan_ip>>16)&0xff,(lan_ip>>8)&0xff);	
						
					 	//data_len = ntohs(ip->ip_len) - ( ( unsigned char *) tcp + ( tcp->th_off << 2) - ( unsigned char *) ip);
					 	data_len = ntohs(ip->ip_len) - ( /*ntohs*/(ip->ip_hl << 2) + /*ntohs*/( tcp->th_off << 2));
						
						p = memstr(net_ip_buf,pdata,sizeof(net_ip_buf),data_len);
						
						if (p)
						{	
							wan_ip = ntohl(SYS_wan_ip);
							sprintf((char *)wan_ip_buf,"%c%c%c%c",
										(wan_ip>>24)&0xff,(wan_ip>>16)&0xff,(wan_ip>>8)&0xff, wan_ip&0xff);	
							
							private_ip =  (unsigned int) *((unsigned int *)p);
							tcp->th_sum = chsum_fixup_long_tenda(tcp->th_sum, private_ip, htonl(wan_ip));
							memcpy(p,wan_ip_buf,sizeof(wan_ip_buf));
						}
		
					 }
				}
			}	
			{
					if(ntohs(tcp->th_dport) == 0x6112 &&
						ntohs(tcp->th_sport) ==0x6112)
					{
						int i, register_flag=0;
						for(i = 0; i < MAX_STAR_WARS_MAP && (star_wars_map[i].private_net_ip) !=0; ++i)
						{
							if( (star_wars_map[i].private_net_ip) == ntohl(ip->ip_src.s_addr))
							{
								star_wars_map[i].game_port = ntohs( nat->nat_outport);
								register_flag=1;
								break;
							}
						}
						if(register_flag == 0 && i < MAX_STAR_WARS_MAP)
						{	
							
							star_wars_map[i].private_net_ip = ntohl(ip->ip_src.s_addr);
							star_wars_map[i].game_port = ntohs( nat->nat_outport);
						}	
						
					}
			}

#endif
			csump = &(tcp->th_sum);
			tcp->th_dport = nat->nat_inport;
			nat->nat_age = fr_tcpidletimeout;
			break;
		case IPPROTO_UDP:
			if (((udphdr_t *)tcp)->uh_sum)
				csump = &(((udphdr_t *)tcp)->uh_sum);
			tcp->th_dport = nat->nat_inport;
			nat->nat_age = fr_defnatage;
			break;
		default:
			goto out;
		}
		if (csump) {
			if (nat->nat_dir == NAT_OUTBOUND)
				nat_fix_incksum((ip->ip_len-hlen), csump, nat->nat_sumd[0]);
			else
				nat_fix_outcksum((ip->ip_len-hlen), csump, nat->nat_sumd[0]);
		}
		ip->ip_dst = nat->nat_inip;
		
#ifdef __CONFIG_TC__
		/*	flow stream control of wan to lan,	add by stanley .
			stream_control(int nat_dir, int nflags, ip_t *ip, fr_info_t *fin)
				$nat_dir:	nat direction, up or download.
				$nflags: 	protocol type, TCP or UDP or ICMP.
				$ip:		
				$fin:		
		*/

		if(g_user_rate_control == 1){
#ifdef __CONFIG_TC_WINDOWS__
			if(down_win_stream_control(NAT_INBOUND, ip) == -1)	 //down
			{
				//diag_printf("drop!\n");
				m_freem(m);
				return 1;			
				//goto out;
			}
#endif
		}

#ifdef __CONFIG_STREAM_STATISTIC__
		stream_statistic(ip, NAT_INBOUND);
#endif/*__CONFIG_STREAM_STATISTIC__*/

#endif/*__CONFIG_TC__*/

#ifdef __CONFIG_STAR_WARS_ENABLE__
{
		if(	ntohs(tcp->th_sport) == 0x6112)
		{
			int i, register_flag=0;
			for(i = 0; i < MAX_STAR_WARS_MAP && (star_wars_map[i].private_net_ip) !=0; ++i)
			{
				if( (star_wars_map[i].private_net_ip == ntohl(ip->ip_dst.s_addr) )&&
					(star_wars_map[i].game_port == ntohs(tcp->th_dport)) )
				{
					
					tcp->th_sum = chsum_fixup_long_tenda(tcp->th_sum, tcp->th_dport, htons(0x6112));
					tcp->th_dport= htons(0x6112);
					register_flag=1;
					break;
				}
			}
			if(register_flag == 0)
			{					
				diag_printf("STAR_WARS_ENABLE 0x6112\n");		
			}	
					
		}
}
#endif
		break;
	}
	if (nat)
		goto fastout;
	/* outbound */
	nat = nat_table[0][hv];
	for (; nat; nat = nat->nat_hnext[0]) {
		/* Do NAT entry outbound lookup */
		if ((nat->nat_flags & IPN_FASTCACHE) == 0 ||
		    ifp != nat->nat_ort->rt_ifp ||
		    nat->nat_inip.s_addr != ip->ip_src.s_addr ||
		    nat->nat_oip.s_addr != ip->ip_dst.s_addr ||
		    ip->ip_p != nat->nat_p ||
		    tcp->th_sport != nat->nat_inport ||
		    tcp->th_dport != nat->nat_oport)
			continue;

#ifdef __CONFIG_TC__
		/*	flow stream control of lan to wan,	add by stanley .
			stream_control(int nat_dir, int nflags, ip_t *ip, fr_info_t *fin)
				$nat_dir:	nat direction, up or download.
				$nflags: 	protocol type, TCP or UDP or ICMP.
				$ip:		
				$fin:		
		*/
		if(g_user_rate_control == 1){
#ifdef __CONFIG_TC_WINDOWS__
			if(up_win_stream_control(NAT_OUTBOUND, ip) == -1)
			{//up
				//diag_printf("drop!\n");
				m_freem(m);
				return 1;
			}
#endif
		}

#ifdef __CONFIG_STREAM_STATISTIC__
		stream_statistic(ip, NAT_OUTBOUND);
#endif/*__CONFIG_STREAM_STATISTIC__*/

#endif/*__CONFIG_TC__*/


		if ((drt = nat->nat_rt) == NULL)
			goto out;
		/*
		 * Do not deal packet larger than interface mtu,
		 * because the we have to do fragment.
		 */
		difp = drt->rt_ifp;
		if (difp->if_mtu < ntohs(ip->ip_len))
			goto out;
		switch (ip->ip_p) {
		case IPPROTO_TCP:
			if (tcp->th_flags & (TH_RST | TH_FIN | TH_SYN))
				goto out;
			csump = &(tcp->th_sum);
			tcp->th_sport = nat->nat_outport;
			nat->nat_age = fr_tcpidletimeout;
			break;
		case IPPROTO_UDP:
			if (((udphdr_t *)tcp)->uh_sum)
				csump = &(((udphdr_t *)tcp)->uh_sum);
			tcp->th_sport = nat->nat_outport;
			nat->nat_age = fr_defnatage;
			break;
		default:
			goto out;
		}
		if (csump) {
			if (nat->nat_dir == NAT_OUTBOUND)
				nat_fix_outcksum((ip->ip_len-hlen), csump, nat->nat_sumd[1]);
			else
				nat_fix_incksum((ip->ip_len-hlen), csump, nat->nat_sumd[1]);
		}
		ip->ip_src = nat->nat_outip;
		break;
	}
	if (nat == NULL)
		goto out;
fastout:
#ifdef TENDA_WLJB
	tenda_change_packet2(ip);
	qq_packet_change(ip,g_private_ip,g_wan_ip);
    	xunlei_packet_change(ip,g_private_ip,g_wan_ip);
#endif

	if (drt->rt_flags & RTF_GATEWAY)
		dst = (struct sockaddr_in *)(drt->rt_gateway);
	else {
		sin.sin_addr = ip->ip_dst;
		dst = &sin;
	}
//#ifndef TENDA_WLJB
//	ip->ip_ttl -= IPTTLDEC;
//#endif
	ip->ip_sum = 0;
	ip->ip_sum = in_cksum(m, hlen);
	(*difp->if_output)(difp, m, (struct sockaddr *)dst, drt);
//tenda add for pppoe on demand
	dns_query_on_idel_time++;
	return 1;
out:
	return 0;
}

/* Hook the nat fast path */
int
ip_fastpath_init(void)
{

#ifdef CONFIG_RTL_FREEBSD_FAST_PATH
	ip_fastpath = NULL;
#else
	ip_fastpath = ipnat_fastpath;
#endif
	return 0;
}

/* Unhook the nat fast path */
int
ip_fastpath_remove(void)
{
	ip_fastpath = NULL;
	return 0;
}
