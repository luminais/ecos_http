//==========================================================================
//
//      src/sys/netinet/igmp.c
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
 * Copyright (c) 1988 Stephen Deering.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Stephen Deering of Stanford University.
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
 *	@(#)igmp.c	8.1 (Berkeley) 7/19/93
 * $FreeBSD: src/sys/netinet/igmp.c,v 1.29 1999/12/22 19:13:17 shin Exp $
 */

/*
 * Internet Group Management Protocol (IGMP) routines.
 *
 * Written by Steve Deering, Stanford, May 1988.
 * Modified by Rosen Sharma, Stanford, Aug 1994.
 * Modified by Bill Fenner, Xerox PARC, Feb 1995.
 * Modified to fully comply to IGMPv2 by Bill Fenner, Oct 1995.
 *
 * MULTICAST Revision: 3.5.1.4
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/igmp.h>
#include <netinet/igmp_var.h>

#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)

static int igmp_report_process( struct igmp *igmp,int igmp_type,struct ifnet *ifp,in_addr_t igmp_group);

#endif
static struct router_info *
		find_rti __P((struct ifnet *ifp));

static struct igmpstat igmpstat;

SYSCTL_STRUCT(_net_inet_igmp, IGMPCTL_STATS, stats, CTLFLAG_RD,
	&igmpstat, igmpstat, "");

static int igmp_timers_are_running;
static u_long igmp_all_hosts_group;
static u_long igmp_all_rtrs_group;
static struct mbuf *router_alert;
static struct router_info *Head;

static void igmp_sendpkt __P((struct in_multi *, int, unsigned long));

void
igmp_init()
{
	struct ipoption *ra;

	/*
	 * To avoid byte-swapping the same value over and over again.
	 */
	igmp_all_hosts_group = htonl(INADDR_ALLHOSTS_GROUP);
	igmp_all_rtrs_group = htonl(INADDR_ALLRTRS_GROUP);

	igmp_timers_are_running = 0;

	/*
	 * Construct a Router Alert option to use in outgoing packets
	 */
	MGET(router_alert, M_DONTWAIT, MT_DATA);
	ra = mtod(router_alert, struct ipoption *);
	ra->ipopt_dst.s_addr = 0;
	ra->ipopt_list[0] = IPOPT_RA;	/* Router Alert Option */
	ra->ipopt_list[1] = 0x04;	/* 4 bytes long */
	ra->ipopt_list[2] = 0x00;
	ra->ipopt_list[3] = 0x00;
	router_alert->m_len = sizeof(ra->ipopt_dst) + ra->ipopt_list[1];

	Head = (struct router_info *) 0;
}

static struct router_info *
find_rti(ifp)
	struct ifnet *ifp;
{
        register struct router_info *rti = Head;

#ifdef IGMP_DEBUG
	printf("[igmp.c, _find_rti] --> entering \n");
#endif
        while (rti) {
                if (rti->rti_ifp == ifp) {
#ifdef IGMP_DEBUG
			printf("[igmp.c, _find_rti] --> found old entry \n");
#endif
                        return rti;
                }
                rti = rti->rti_next;
        }
	MALLOC(rti, struct router_info *, sizeof *rti, M_IGMP, M_NOWAIT);
	if (rti == NULL) {
#ifdef IGMP_DEBUG
		printf("[igmp.c, _find_rti] --> out of memory \n");
#endif
		return NULL;
	}
        rti->rti_ifp = ifp;
        rti->rti_type = IGMP_V2_ROUTER;
        rti->rti_time = 0;
        rti->rti_next = Head;
        Head = rti;
#ifdef IGMP_DEBUG
	printf("[igmp.c, _find_rti] --> created an entry \n");
#endif
        return rti;
}

void
igmp_input(m, off)
	register struct mbuf *m;
	int off;
{
	register int iphlen = off;
	register struct igmp *igmp;
	register struct ip *ip;
	register int igmplen;
	register struct ifnet *ifp = m->m_pkthdr.rcvif;
	register int minlen;
	register struct in_multi *inm;
	register struct in_ifaddr *ia;
	struct in_multistep step;
	struct router_info *rti;
	
	int timer; /** timer value in the igmp query header **/

	++igmpstat.igps_rcv_total;

	ip = mtod(m, struct ip *);
	igmplen = ip->ip_len;

	/*
	 * Validate lengths
	 */
	if (igmplen < IGMP_MINLEN) {
		++igmpstat.igps_rcv_tooshort;
		m_freem(m);
		return;
	}
	minlen = iphlen + IGMP_MINLEN;
	if ((m->m_flags & M_EXT || m->m_len < minlen) &&
	    (m = m_pullup(m, minlen)) == 0) {
		++igmpstat.igps_rcv_tooshort;
		return;
	}

	/*
	 * Validate checksum
	 */
	m->m_data += iphlen;
	m->m_len -= iphlen;
	igmp = mtod(m, struct igmp *);
	if (in_cksum(m, igmplen)) {
		++igmpstat.igps_rcv_badsum;
		m_freem(m);
		return;
	}
	m->m_data -= iphlen;
	m->m_len += iphlen;

	ip = mtod(m, struct ip *);
	timer = igmp->igmp_code * PR_FASTHZ / IGMP_TIMER_SCALE;
	if (timer == 0)
		timer = 1;
	rti = find_rti(ifp);

	/*
	 * In the IGMPv2 specification, there are 3 states and a flag.
	 *
	 * In Non-Member state, we simply don't have a membership record.
	 * In Delaying Member state, our timer is running (inm->inm_timer)
	 * In Idle Member state, our timer is not running (inm->inm_timer==0)
	 *
	 * The flag is inm->inm_state, it is set to IGMP_OTHERMEMBER if
	 * we have heard a report from another member, or IGMP_IREPORTEDLAST
	 * if I sent the last report.
	 */
	switch (igmp->igmp_type) {

	case IGMP_MEMBERSHIP_QUERY:
		++igmpstat.igps_rcv_queries;

		if (ifp->if_flags & IFF_LOOPBACK)
			break;

		if (igmp->igmp_code == 0) {
			/*
			 * Old router.  Remember that the querier on this
			 * interface is old, and set the timer to the
			 * value in RFC 1112.
			 */

			rti->rti_type = IGMP_V1_ROUTER;
			rti->rti_time = 0;

			timer = IGMP_MAX_HOST_REPORT_DELAY * PR_FASTHZ;

			if (ip->ip_dst.s_addr != igmp_all_hosts_group ||
			    igmp->igmp_group.s_addr != 0) {
				++igmpstat.igps_rcv_badqueries;
				m_freem(m);
				return;
			}
		} else {
			/*
			 * New router.  Simply do the new validity check.
			 */
			
			if (igmp->igmp_group.s_addr != 0 &&
			    !IN_MULTICAST(ntohl(igmp->igmp_group.s_addr))) {
				++igmpstat.igps_rcv_badqueries;
				m_freem(m);
				return;
			}
		}

		/*
		 * - Start the timers in all of our membership records
		 *   that the query applies to for the interface on
		 *   which the query arrived excl. those that belong
		 *   to the "all-hosts" group (224.0.0.1).
		 * - Restart any timer that is already running but has
		 *   a value longer than the requested timeout.
		 * - Use the value specified in the query message as
		 *   the maximum timeout.
		 */
		IN_FIRST_MULTI(step, inm);
		while (inm != NULL) {
			if (inm->inm_ifp == ifp &&
			    inm->inm_addr.s_addr != igmp_all_hosts_group &&
			    (igmp->igmp_group.s_addr == 0 ||
			     igmp->igmp_group.s_addr == inm->inm_addr.s_addr)) {
				if (inm->inm_timer == 0 ||
				    inm->inm_timer > timer) {
					igmp_joingroup(inm);
					inm->inm_timer =
						IGMP_RANDOM_DELAY(timer);
					igmp_timers_are_running = 1;
				}
			}
			IN_NEXT_MULTI(step, inm);
		}

		break;

	case IGMP_V1_MEMBERSHIP_REPORT:
	case IGMP_V2_MEMBERSHIP_REPORT:
		//diag_printf("join group:%x,[%s]:[%d].\n",igmp->igmp_group.s_addr,__FUNCTION__,__LINE__);
		/*
		 * For fast leave to work, we have to know that we are the
		 * last person to send a report for this group.  Reports
		 * can potentially get looped back if we are a multicast
		 * router, so discard reports sourced by me.
		 */
		 
		IFP_TO_IA(ifp, ia);
		if (ia && ip->ip_src.s_addr == IA_SIN(ia)->sin_addr.s_addr){
			break;
		}	

		++igmpstat.igps_rcv_reports;

		if (ifp->if_flags & IFF_LOOPBACK){
			break;
		}
		if (!IN_MULTICAST(ntohl(igmp->igmp_group.s_addr))) {
			++igmpstat.igps_rcv_badreports;
			m_freem(m);
			return;
		}

		/*
		 * KLUDGE: if the IP source address of the report has an
		 * unspecified (i.e., zero) subnet number, as is allowed for
		 * a booting host, replace it with the correct subnet number
		 * so that a process-level multicast routing demon can
		 * determine which subnet it arrived from.  This is necessary
		 * to compensate for the lack of any way for a process to
		 * determine the arrival interface of an incoming packet.
		 */
		if ((ntohl(ip->ip_src.s_addr) & IN_CLASSA_NET) == 0)
			if (ia) ip->ip_src.s_addr = htonl(ia->ia_subnet);

	#if 0
		/*
		 * If we belong to the group being reported, stop
		 * our timer for that group.
		 */
		IN_LOOKUP_MULTI(igmp->igmp_group, ifp, inm);

		if (inm != NULL) {
			inm->inm_timer = 0;
			++igmpstat.igps_rcv_ourreports;

			inm->inm_state = IGMP_OTHERMEMBER;
		}
	#endif

		break;
#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)	
		case IGMP_V2_LEAVE_GROUP:
			//diag_printf("leave group:%x,[%s]:[%d].\n",igmp->igmp_group.s_addr,__FUNCTION__,__LINE__);

			igmp_report_process( igmp,igmp->igmp_type,ifp,igmp->igmp_group.s_addr);

			break;
		case IGMP_V3_REPORT:
			//diag_printf("v3 report:%x,[%s]:[%d].\n",igmp->igmp_group.s_addr,__FUNCTION__,__LINE__);
			igmp_report_process( igmp,igmp->igmp_type,ifp,igmp->igmp_group.s_addr);
			
			break;
#endif			
	}

	/*
	 * Pass all valid IGMP packets up to any process(es) listening
	 * on a raw IGMP socket.
	 */
	rip_input(m, off);
}

void
igmp_joingroup(inm)
	struct in_multi *inm;
{
	int s = splnet();

	if (inm->inm_addr.s_addr == igmp_all_hosts_group
	    || inm->inm_ifp->if_flags & IFF_LOOPBACK) {
		inm->inm_timer = 0;
		inm->inm_state = IGMP_OTHERMEMBER;
	} else {
		inm->inm_rti = find_rti(inm->inm_ifp);
		igmp_sendpkt(inm, inm->inm_rti->rti_type, 0);
		inm->inm_timer = IGMP_RANDOM_DELAY(
					IGMP_MAX_HOST_REPORT_DELAY*PR_FASTHZ);
		inm->inm_state = IGMP_IREPORTEDLAST;
		igmp_timers_are_running = 1;
	}
	splx(s);
}

void
igmp_leavegroup(inm)
	struct in_multi *inm;
{
	if (inm->inm_state == IGMP_IREPORTEDLAST &&
	    inm->inm_addr.s_addr != igmp_all_hosts_group &&
	    !(inm->inm_ifp->if_flags & IFF_LOOPBACK) &&
	    inm->inm_rti->rti_type != IGMP_V1_ROUTER){
		igmp_sendpkt(inm, IGMP_V2_LEAVE_GROUP, igmp_all_rtrs_group);
	}	
	
}

void
igmp_fasttimo()
{
	register struct in_multi *inm;
	struct in_multistep step;
	int s;

	/*
	 * Quick check to see if any work needs to be done, in order
	 * to minimize the overhead of fasttimo processing.
	 */

	if (!igmp_timers_are_running)
		return;

	s = splnet();
	igmp_timers_are_running = 0;
	IN_FIRST_MULTI(step, inm);
	while (inm != NULL) {
		if (inm->inm_timer == 0) {
			/* do nothing */
		} else if (--inm->inm_timer == 0) {
			igmp_sendpkt(inm, inm->inm_rti->rti_type, 0);
			inm->inm_state = IGMP_IREPORTEDLAST;
		} else {
			igmp_timers_are_running = 1;
		}
		IN_NEXT_MULTI(step, inm);
	}
	splx(s);
}

void
igmp_slowtimo()
{
	int s = splnet();
	register struct router_info *rti =  Head;

#ifdef IGMP_DEBUG
	printf("[igmp.c,_slowtimo] -- > entering \n");
#endif
	while (rti) {
	    if (rti->rti_type == IGMP_V1_ROUTER) {
		rti->rti_time++;
		if (rti->rti_time >= IGMP_AGE_THRESHOLD) {
			rti->rti_type = IGMP_V2_ROUTER;
		}
	    }
	    rti = rti->rti_next;
	}
#ifdef IGMP_DEBUG	
	printf("[igmp.c,_slowtimo] -- > exiting \n");
#endif
	splx(s);
}

static struct route igmprt;

static void
igmp_sendpkt(inm, type, addr)
	struct in_multi *inm;
	int type;
	unsigned long addr;
{
        struct mbuf *m;
        struct igmp *igmp;
        struct ip *ip;
        struct ip_moptions imo;

        MGETHDR(m, M_DONTWAIT, MT_HEADER);
        if (m == NULL)
                return;

	m->m_pkthdr.rcvif = loif;
	m->m_pkthdr.len = sizeof(struct ip) + IGMP_MINLEN;
	MH_ALIGN(m, IGMP_MINLEN + sizeof(struct ip));
	m->m_data += sizeof(struct ip);
        m->m_len = IGMP_MINLEN;
        igmp = mtod(m, struct igmp *);
	#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
		memset(igmp,0,sizeof(struct igmp));
	#endif
        igmp->igmp_type   = type;
	#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	
		if(type == IGMP_MEMBERSHIP_QUERY){
			igmp->igmp_code   = 0x64; //max response time
			if((inm->inm_addr).s_addr==0)	
				(igmp->igmp_group).s_addr = 0;
			else
				igmp->igmp_group= inm->inm_addr;
				
		}	
		else	
		{
       		igmp->igmp_code   = 0;
			igmp->igmp_group= inm->inm_addr;
			
		}
	#else
	
		igmp->igmp_code   = 0;
		igmp->igmp_group= inm->inm_addr;
	#endif
	

        igmp->igmp_cksum  = 0;
        igmp->igmp_cksum  = in_cksum(m, IGMP_MINLEN);

        m->m_data -= sizeof(struct ip);
        m->m_len += sizeof(struct ip);
        ip = mtod(m, struct ip *);
        ip->ip_tos        = 0;
        ip->ip_len        = sizeof(struct ip) + IGMP_MINLEN;
        ip->ip_off        = 0;
        ip->ip_p          = IPPROTO_IGMP;
        ip->ip_src.s_addr = INADDR_ANY;
        ip->ip_dst.s_addr = addr ? addr : igmp->igmp_group.s_addr;

        imo.imo_multicast_ifp  = inm->inm_ifp;
        imo.imo_multicast_ttl  = 1;
	
	#if 0//defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
		if(imo.imo_multicast_ifp){
			struct in_ifaddr *ia;
			//struct sockaddr_in *in_addr;
			IFP_TO_IA(imo.imo_multicast_ifp, ia);
			if (ia)
				imo.imo_multicast_addr.s_addr=IA_SIN(ia)->sin_addr.s_addr ;
		}
	#endif
	
		
		imo.imo_multicast_vif  = -1;
        /*
         * Request loopback of the report if we are acting as a multicast
         * router, so that the process-level routing demon can hear it.
         */
	#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
		 imo.imo_multicast_loop = 0;
	#else
         imo.imo_multicast_loop = (ip_mrouter != NULL);
	#endif

	/*
	 * XXX
	 * Do we have to worry about reentrancy here?  Don't think so.
	 */
	
	#if 0//defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	 	//diag_printf("type:%x,  igmp->igmp_group:%x,saddr:%x,dst:%x[%s]:[%d].\n",type,  igmp->igmp_group.s_addr,imo.imo_multicast_addr.s_addr,ip->ip_dst.s_addr,__FUNCTION__,__LINE__);
        ip_output(m, router_alert, &igmprt, IP_ROUTETOIF, &imo);
	#else
		ip_output(m, router_alert, &igmprt, 0, &imo);
	#endif
	 		
        ++igmpstat.igps_snd_reports;

}
	
#if defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
extern int mfc_process(int mfctype,struct ifnet *ifp,in_addr_t igmp_group);	
extern struct ifnet * get_igmpproxyif(int flag);
static struct callout squery_timer;

void igmp_packet_send(int type,struct ifnet *ifp,unsigned long igmp_group)
{
	struct in_multi  inm;
	
	int s;
	
	if(ifp==NULL)
		return;
	
	s = splimp();
	
	memset(&inm,0,sizeof(struct in_multi));
	inm.inm_addr.s_addr= igmp_group;
	inm.inm_ifp = ifp;
	
	if(type==IGMP_V2_LEAVE_GROUP){
		/*leave report*/
		igmp_sendpkt(&inm, type, igmp_all_rtrs_group);
	}
	else if((type==IGMP_MEMBERSHIP_QUERY)&&(igmp_group==0)){
		/*general query*/

		//diag_printf("---------inm:%p,type:%x,ifp:%s%d,igmp_group:%x,[%s]:[%d].\n",inm,type,ifp->if_name,ifp->if_unit,igmp_group,__FUNCTION__,__LINE__);
		igmp_sendpkt(&inm, type, igmp_all_hosts_group);
	}	
	else
	{
		/*special query || join report*/
		//diag_printf("---------inm:%p,type:%x,ifp:%s%d,igmp_group:%x,[%s]:[%d].\n",inm,type,ifp->if_name,ifp->if_unit,igmp_group,__FUNCTION__,__LINE__);
		igmp_sendpkt(&inm, type, igmp_group);
	}
	
	splx(s);
	return;
	
}
static void igmp_special_query_expired(void * para)
{
	struct query_timer * query_para =(struct query_timer *)para;
	struct ifnet * downif=NULL;

	if(query_para==NULL)
		return;
	
	downif=get_igmpproxyif(IGMP_PROXY_DOWN_STREAM_FLAG);
	//diag_printf("igmp special query expired.%p,retry:%d,group:%x\n",query_para,query_para->retry_left_time,query_para->igmp_group);
	if(query_para->retry_left_time)
	{
		if(downif)
		{
			igmp_packet_send(IGMP_MEMBERSHIP_QUERY,downif,query_para->igmp_group);
		
			query_para->retry_left_time--;
				
			if(query_para->retry_left_time)	
				callout_reset(&squery_timer,(IGMP_LAST_MEMBER_QUERY_INTERVAL*hz) ,igmp_special_query_expired, query_para);
		
			else
			{
				//diag_printf("FREE:%p[%s]:[%d].\n",query_para,__FUNCTION__,__LINE__);
				free(query_para, M_TEMP);
				
			}
		}
		else
		{
			free(query_para, M_TEMP);
		}
	}
	return;
	
}
static int igmp_report_process( struct igmp *igmp,int igmp_type,struct ifnet *ifp,in_addr_t igmp_group)
{
	int err;
	struct ifnet * upvif=NULL;
	struct ifnet * downvif=NULL;
	struct query_timer *query_para=NULL;
	struct igmpv3_report *igmpv3;
	struct igmpv3_grec *igmpv3grec;
	unsigned int rec_id;
	unsigned int  group;
	
	upvif=get_igmpproxyif(IGMP_PROXY_UP_STREAM_FLAG);
	downvif=get_igmpproxyif(IGMP_PROXY_DOWN_STREAM_FLAG);
	//diag_printf("[%s]:[%d],igmp_type:%x,ifp:%x,%s.\n",__FUNCTION__,__LINE__,igmp_type,ifp->if_index,ifp->if_name);
	if((upvif==NULL)||(downvif==NULL)||(ifp==NULL)){
		//diag_printf("------not init yet!\n");		
		return 0;
	}
	if (ifp->if_index==upvif->if_index){
		//diag_printf("ignore igmp packet from upstream.ifp:%d,upifp:%d.\n",ifp->if_index,upvif->if_index);
		return 0;
	}	
	if (!IN_MULTICAST(ntohl(igmp_group)))
		return 0;
	
	switch (igmp_type) {
		
	case IGMP_V1_MEMBERSHIP_REPORT:
	case IGMP_V2_MEMBERSHIP_REPORT:	
		//diag_printf("igmp join report.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		break;
	
	case IGMP_V2_LEAVE_GROUP:

		//diag_printf("IGMP_V2_LEAVE_GROUP:%x,%s%d.[%s]:[%d].\n",igmp_group,ifp->if_name,ifp->if_unit,__FUNCTION__,__LINE__);
		/*set timer set query count*/
		
		query_para=(struct query_timer *)malloc(
			    sizeof(struct query_timer), M_TEMP, M_NOWAIT);
		if(query_para==NULL)
			return;
		memset(query_para,0,sizeof(struct query_timer));
		query_para->retry_left_time = IGMP_LAST_MEMBER_QUERY_COUNT;
		query_para->igmp_group = igmp_group;
		query_para->type = SPECIAL_TIMER_TPYE;
		if(downvif){
			//diag_printf("send special query![%s]:[%d].\n",__FUNCTION__,__LINE__);
			igmp_packet_send(IGMP_MEMBERSHIP_QUERY,downvif,query_para->igmp_group);
			query_para->retry_left_time--; 
		}	
		callout_init(&squery_timer);
		callout_reset(&squery_timer,(IGMP_LAST_MEMBER_QUERY_INTERVAL*hz) ,igmp_special_query_expired, query_para);
				
		break;
		
	case IGMP_V3_REPORT:
		
		igmpv3 = (struct igmpv3_report *)igmp;
		//diag_printf( "recv IGMP_HOST_V3_MEMBERSHIP_REPORT,igmpv3->type:0x%x,igmpv3->ngrec:0x%x.\n",igmpv3->type,ntohs(igmpv3->ngrec) );
				
		rec_id=0;
		igmpv3grec =  &igmpv3->grec[0];
		while( rec_id < ntohs(igmpv3->ngrec) )
		{
			
			//diag_printf( "igmpv3grec[%d]->grec_type:0x%x\n", rec_id, igmpv3grec->grec_type );
			//diag_printf( "igmpv3grec[%d]->grec_auxwords:0x%x\n", rec_id, igmpv3grec->grec_auxwords );
			//diag_printf( "igmpv3grec[%d]->grec_nsrcs:0x%x\n", rec_id, ntohs(igmpv3grec->grec_nsrcs) );
			//diag_printf( "igmpv3grec[%d]->grec_mca:%s\n", rec_id, inet_ntoa(igmpv3grec->grec_mca) );
		
			group = igmpv3grec->grec_mca.s_addr;
			
			switch( igmpv3grec->grec_type )
			{
				case IGMPV3_MODE_IS_INCLUDE:
				case IGMPV3_MODE_IS_EXCLUDE:
					
					//diag_printf( "IS_IN or IN_EX\n" );
					//accept_group_report(src, dst, group, igmp->igmp_type);
					
					break;
				case IGMPV3_CHANGE_TO_INCLUDE: 
					//diag_printf( "TO_IN\n" );
					if( igmpv3grec->grec_nsrcs==0 )//empty
					{
						query_para=(struct query_timer *)malloc(
							    sizeof(struct query_timer), M_TEMP, M_NOWAIT);
						if(query_para==NULL)
							return;
						
						
						memset(query_para,0,sizeof(struct query_timer));
						query_para->retry_left_time = IGMP_LAST_MEMBER_QUERY_COUNT;
						query_para->igmp_group = igmp_group;
						query_para->type = SPECIAL_TIMER_TPYE;
						if(downvif){
							//diag_printf("send special query![%s]:[%d].\n",__FUNCTION__,__LINE__);
							igmp_packet_send(IGMP_MEMBERSHIP_QUERY,downvif,query_para->igmp_group);
							query_para->retry_left_time--; 
						}	
						
						//diag_printf("init para:%p,time%d,group:%x\n",query_para,query_para->retry_left_time,query_para->igmp_group);
						callout_init(&squery_timer);
						callout_reset(&squery_timer,(IGMP_LAST_MEMBER_QUERY_INTERVAL*hz) ,igmp_special_query_expired, query_para);
							
					}
					break;
				case IGMPV3_CHANGE_TO_EXCLUDE: 
					//diag_printf( "TO_EX\n" );
					//accept_group_report(src, dst, group, igmp->igmp_type);
					break;
				case IGMPV3_ALLOW_NEW_SOURCES:
					//diag_printf( "ALLOW\n" );
					break;
				case IGMPV3_BLOCK_OLD_SOURCES:
					//diag_printf( "BLOCK\n" );
					break;
				default:
					//diag_printf( "!!! can't handle the group record types: %d\n", igmpv3grec->grec_type );
					break;
			}
		
			rec_id++;
			//diag_printf( "count next: 0x%x %d %d %d %d\n", igmpv3grec, sizeof( struct igmpv3_grec ), igmpv3grec->grec_auxwords, ntohs(igmpv3grec->grec_nsrcs), sizeof( __u32 ) );
			igmpv3grec = (struct igmpv3_grec *)( (char*)igmpv3grec + sizeof( struct igmpv3_grec ) + (igmpv3grec->grec_auxwords+ntohs(igmpv3grec->grec_nsrcs))*sizeof(unsigned int) );
			//diag_printf( "count result: 0x%x\n", igmpv3grec );
		}		
		break;
	}	


	return 0;
	
}

#endif

#if 1
#include <netinet/ip_mroute.h>
#include <stdio.h>
extern struct mfc	*mfctable[MFCTBLSIZ];
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


