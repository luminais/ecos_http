/*
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE".  Use of the mrouted program represents acceptance of
 * the terms and conditions listed in that file.
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 *
 * igmp.c,v 3.8.4.19 1998/01/06 01:57:43 fenner Exp
 */

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include "defs.h"

/*
 * Exported variables.
 */
char		*recv_buf; 		     /* input packet buffer         */
char		*send_buf; 		     /* output packet buffer        */
int		igmp_socket;		     /* socket for all network I/O  */
u_int32		allhosts_group;		     /* All hosts addr in net order */
u_int32		allrtrs_group;		     /* All-Routers "  in net order */
u_int32		dvmrp_group;		     /* DVMRP grp addr in net order */
u_int32		dvmrp_genid;		     /* IGMP generation id          */

/*
 * Local function definitions.
 */
/* u_char promoted to u_int */
static int	igmp_log_level __P((u_int type, u_int code));

/*
 * Open and initialize the igmp socket, and fill in the non-changing
 * IP header fields in the output packet buffer.
 */
int 
init_igmp()
{
    struct ip *ip;
	//diag_printf("init process.[%s]:[%d].\n",__FUNCTION__,__LINE__);
	
	recv_buf = (char *)malloc(RECV_BUF_SIZE);
	if (recv_buf == 0){
	    log(LOG_ERR, 0, "Out of memory allocating recv_buf!");
		return 0;
	}
    send_buf = (char *)malloc(RECV_BUF_SIZE);
    if (send_buf == 0){
	    log(LOG_ERR, 0, "Out of memory allocating send_buf!");
		return 0;	
	}
    
    if ((igmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP)) < 0) 
	log(LOG_ERR, errno, "IGMP socket");
#ifdef 	ECOS_DBG_STAT
    dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
    k_hdr_include(TRUE);	/* include IP header when sending */
    k_set_rcvbuf(256*1024,48*1024);	/* lots of input buffering        */
    k_set_ttl(1);		/* restrict multicasts to one hop */
    k_set_loop(FALSE);		/* disable multicast loopback     */

    ip         = (struct ip *)send_buf;
    bzero(ip, sizeof(struct ip));
    /*
     * Fields zeroed that aren't filled in later:
     * - IP ID (let the kernel fill it in)
     * - Offset (we don't send fragments)
     * - Checksum (let the kernel fill it in)
     */
    ip->ip_v   = IPVERSION;
    ip->ip_hl  = sizeof(struct ip) >> 2;
    ip->ip_tos = 0xc0;		/* Internet Control */
    ip->ip_ttl = MAXTTL;	/* applies to unicasts only */
    ip->ip_p   = IPPROTO_IGMP;

    allhosts_group = htonl(INADDR_ALLHOSTS_GROUP);
    dvmrp_group    = htonl(INADDR_DVMRP_GROUP);
    allrtrs_group  = htonl(INADDR_ALLRTRS_GROUP);
	return 1;
}
void
exit_igmp()
{
   
	 //diag_printf("\nexit process.[%s]:[%d].\n",__FUNCTION__,__LINE__);
	if(recv_buf)
	{
		//diag_printf("\nrecv_buf exit process.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		free(recv_buf);
		recv_buf=NULL;
	}

	if(send_buf){
		//diag_printf("\nsend_buf exit process.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		free(send_buf);
		send_buf=NULL;
	}
	
}

#define PIM_QUERY        0
#define PIM_REGISTER     1
#define PIM_REGISTER_STOP 	2
#define PIM_JOIN_PRUNE   3
#define PIM_RP_REACHABLE 4
#define PIM_ASSERT       5
#define PIM_GRAFT        6
#define PIM_GRAFT_ACK    7

char *
igmp_packet_kind(type, code)
     u_int type, code;
{
    static char unknown[20];

    switch (type) {
	case IGMP_MEMBERSHIP_QUERY:		return "membership query  ";
	case IGMP_V1_MEMBERSHIP_REPORT:		return "V1 member report  ";
	case IGMP_V2_MEMBERSHIP_REPORT:		return "V2 member report  ";
	case IGMP_V2_LEAVE_GROUP:		return "leave message     ";
	case IGMP_DVMRP:
	  switch (code) {
	    case DVMRP_PROBE:			return "neighbor probe    ";
	    case DVMRP_REPORT:			return "route report      ";
	    case DVMRP_ASK_NEIGHBORS:		return "neighbor request  ";
	    case DVMRP_NEIGHBORS:		return "neighbor list     ";
	    case DVMRP_ASK_NEIGHBORS2:		return "neighbor request 2";
	    case DVMRP_NEIGHBORS2:		return "neighbor list 2   ";
	    case DVMRP_PRUNE:			return "prune message     ";
	    case DVMRP_GRAFT:			return "graft message     ";
	    case DVMRP_GRAFT_ACK:		return "graft message ack ";
	    case DVMRP_INFO_REQUEST:		return "info request      ";
	    case DVMRP_INFO_REPLY:		return "info reply        ";
	    default:
		    sprintf(unknown, "unknown DVMRP %3d ", code);
		    return unknown;
	  }
 	case IGMP_PIM:
 	  switch (code) {
 	    case PIM_QUERY:			return "PIM Router-Query  ";
 	    case PIM_REGISTER:			return "PIM Register      ";
 	    case PIM_REGISTER_STOP:		return "PIM Register-Stop ";
 	    case PIM_JOIN_PRUNE:		return "PIM Join/Prune    ";
 	    case PIM_RP_REACHABLE:		return "PIM RP-Reachable  ";
 	    case PIM_ASSERT:			return "PIM Assert        ";
 	    case PIM_GRAFT:			return "PIM Graft         ";
 	    case PIM_GRAFT_ACK:			return "PIM Graft-Ack     ";
 	    default:
 		    sprintf(unknown, "unknown PIM msg%3d", code);
		    return unknown;
 	  }
	case IGMP_MTRACE:			return "IGMP trace query  ";
	case IGMP_MTRACE_RESP:			return "IGMP trace reply  ";
	default:
		sprintf(unknown, "unk: 0x%02x/0x%02x    ", type, code);
		return unknown;
    }
}

int
igmp_debug_kind(type, code)
     u_int type, code;
{
    switch (type) {
	case IGMP_MEMBERSHIP_QUERY:		return DEBUG_IGMP;
	case IGMP_V1_MEMBERSHIP_REPORT:		return DEBUG_IGMP;
	case IGMP_V2_MEMBERSHIP_REPORT:		return DEBUG_IGMP;
	case IGMP_V2_LEAVE_GROUP:		return DEBUG_IGMP;
	case IGMP_DVMRP:
	  switch (code) {
	    case DVMRP_PROBE:			return DEBUG_PEER;
	    case DVMRP_REPORT:			return DEBUG_ROUTE;
	    case DVMRP_ASK_NEIGHBORS:		return 0;
	    case DVMRP_NEIGHBORS:		return 0;
	    case DVMRP_ASK_NEIGHBORS2:		return 0;
	    case DVMRP_NEIGHBORS2:		return 0;
	    case DVMRP_PRUNE:			return DEBUG_PRUNE;
	    case DVMRP_GRAFT:			return DEBUG_PRUNE;
	    case DVMRP_GRAFT_ACK:		return DEBUG_PRUNE;
	    case DVMRP_INFO_REQUEST:		return 0;
	    case DVMRP_INFO_REPLY:		return 0;
	    default:				return 0;
	  }
	case IGMP_PIM:
	  switch (code) {
	    case PIM_QUERY:			return 0;
	    case PIM_REGISTER:			return 0;
	    case PIM_REGISTER_STOP:		return 0;
	    case PIM_JOIN_PRUNE:		return 0;
	    case PIM_RP_REACHABLE:		return 0;
	    case PIM_ASSERT:			return 0;
	    case PIM_GRAFT:			return 0;
	    case PIM_GRAFT_ACK:			return 0;
	    default:				return 0;
	  }
	case IGMP_MTRACE:			return DEBUG_TRACE;
	case IGMP_MTRACE_RESP:			return DEBUG_TRACE;
	default:				return DEBUG_IGMP;
    }
}
#if defined (RTL_IGMP_NOT_PROCESS_RESERVEDADDR)

int rtl_is_reserve_mcastAddr(u_int32  groupAddress)
{
	int ret=0;
	int enable=rtl_get_mrouteReserveEnable();
	//printf("reservedEnabled:%d,groupAddress:%x,[%s]:[%d].\n",enable,groupAddress,__FUNCTION__,__LINE__);
	if(enable)
	{
		/*239.x.x.x private multicast address*/
		if((groupAddress&0xFF000000)==0xEF000000){
			ret=1;
		}	
	}
	
	//printf("ret:%d,[%s]:[%d].\n",ret,__FUNCTION__,__LINE__);
	return ret;
}
#endif
/*
 * Process a newly received IGMP packet that is sitting in the input
 * packet buffer.
 */
void
accept_igmp(recvlen)
    int recvlen;
{
    register u_int32 src, dst, group;
    struct ip *ip;
    struct igmp *igmp;
    int ipdatalen, iphdrlen, igmpdatalen;
	//diag_printf("accept packet.[%s]:[%d]\n",__FUNCTION__,__LINE__);
    if (recvlen < sizeof(struct ip)) {
	log(LOG_WARNING, 0,
	    "received packet too short (%u bytes) for IP header", recvlen);
	return;
    }

    ip        = (struct ip *)recv_buf;
    src       = ip->ip_src.s_addr;
    dst       = ip->ip_dst.s_addr;

    /* 
     * this is most likely a message from the kernel indicating that
     * a new src grp pair message has arrived and so, it would be 
     * necessary to install a route into the kernel for this.
     */
    
    if(ip->ip_p != 0x2){
		//diag_printf("not igmp pkt.\n");
		return;
	}
	if(dst==0xEFFFFFFA)
		return;
    iphdrlen  = ip->ip_hl << 2;

    ipdatalen = ip->ip_len;

    if (iphdrlen + ipdatalen != recvlen) {
	log(LOG_WARNING, 0,
	    "received packet from %s shorter (%u bytes) than hdr+data length (%u+%u)",
	    inet_fmt(src, s1), recvlen, iphdrlen, ipdatalen);
	return;
    }

    igmp        = (struct igmp *)(recv_buf + iphdrlen);
    group       = igmp->igmp_group.s_addr;
    igmpdatalen = ipdatalen - IGMP_MINLEN;
    if (igmpdatalen < 0) {
	log(LOG_WARNING, 0,
	    "received IP data field too short (%u bytes) for IGMP, from %s",
	    ipdatalen, inet_fmt(src, s1));
	return;
    }
	
#if defined (RTL_IGMP_NOT_PROCESS_RESERVEDADDR)
	if(rtl_is_reserve_mcastAddr(group))
		return;
#endif

    IF_DEBUG(DEBUG_PKT|igmp_debug_kind(igmp->igmp_type, igmp->igmp_code))
    log(LOG_DEBUG, 0, "RECV %s from %-15s to %s",
	igmp_packet_kind(igmp->igmp_type, igmp->igmp_code),
	inet_fmt(src, s1), inet_fmt(dst, s2));
	DBG_PRINT("accept IGMP packet type:%x,src:%x,dst:%x,group:%x.\n",igmp->igmp_type,src,dst,group);
    switch (igmp->igmp_type) {

	case IGMP_MEMBERSHIP_QUERY:
		DBG_PRINT("accept IGMP query packet[%s]:[%d].\n",__FUNCTION__,__LINE__);
	    accept_membership_query(src, dst, group, igmp->igmp_code);
	    return;

	case IGMP_V1_MEMBERSHIP_REPORT:
	case IGMP_V2_MEMBERSHIP_REPORT:
		DBG_PRINT("accept IGMP report packet. type:%x,src:%x,dst:%x,group:%x.\n",igmp->igmp_type,src,dst,group);
	    accept_group_report(src, dst, group, igmp->igmp_type);
	    return;
	    
	case IGMP_V2_LEAVE_GROUP:
		DBG_PRINT("accept IGMP leave packet. type:%x,src:%x,dst:%x,group:%x.\n",igmp->igmp_type,src,dst,group);
	    accept_leave_message(src, dst, group);
	    return;
	#if 1
	case IGMPV3_HOST_MEMBERSHIP_REPORT:
	{
			struct igmpv3_report *igmpv3;
			struct igmpv3_grec *igmpv3grec;
			unsigned short rec_id;
			
			igmpv3 = (struct igmpv3_report *)igmp;
			DBG_PRINT( "recv IGMP_HOST_V3_MEMBERSHIP_REPORT,igmpv3->type:0x%x,igmpv3->ngrec:0x%x.\n",igmpv3->type,ntohs(igmpv3->ngrec) );
					
			rec_id=0;
			igmpv3grec =  &igmpv3->grec[0];
			while( rec_id < ntohs(igmpv3->ngrec) )
			{
				
				//diag_printf( "igmpv3grec[%d]->grec_type:0x%x\n", rec_id, igmpv3grec->grec_type );
				//diag_printf( "igmpv3grec[%d]->grec_auxwords:0x%x\n", rec_id, igmpv3grec->grec_auxwords );
				//diag_printf( "igmpv3grec[%d]->grec_nsrcs:0x%x\n", rec_id, ntohs(igmpv3grec->grec_nsrcs) );
				//diag_printf( "igmpv3grec[%d]->grec_mca:%s\n", rec_id, inet_ntoa(igmpv3grec->grec_mca) );
			
				group = igmpv3grec->grec_mca.s_addr;
				
			#if 0 //realtek对类型处理有问题
				switch( igmpv3grec->grec_type )
				{
					case IGMPV3_MODE_IS_INCLUDE:
					case IGMPV3_MODE_IS_EXCLUDE:
						
						//diag_printf( "IS_IN or IN_EX\n" );
						accept_group_report(src, dst, group, igmp->igmp_type);
						
						break;
					case IGMPV3_CHANGE_TO_INCLUDE: 
						//diag_printf( "TO_IN\n" );
						if( igmpv3grec->grec_nsrcs )
							accept_group_report(src, dst, group, igmp->igmp_type);
						else //empty
							accept_leave_message(src, dst, group);
						break;
					case IGMPV3_CHANGE_TO_EXCLUDE: 
						//diag_printf( "TO_EX\n" );
						accept_group_report(src, dst, group, igmp->igmp_type);
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
			#else
				switch( igmpv3grec->grec_type )
				{
					case IGMPV3_MODE_IS_INCLUDE:
					case IGMPV3_CHANGE_TO_INCLUDE:
						accept_leave_message(src, dst, group);
						break;
					case IGMPV3_MODE_IS_EXCLUDE:
					case IGMPV3_CHANGE_TO_EXCLUDE:
						accept_group_report(src, dst, group, igmp->igmp_type);
						break;
					default:
						diag_printf("[%s][%d][luminais] not accept record type : %02x\n", __FUNCTION__, __LINE__, igmpv3grec->grec_type);
				}
			#endif
			
				rec_id++;
				//diag_printf( "count next: 0x%x %d %d %d %d\n", igmpv3grec, sizeof( struct igmpv3_grec ), igmpv3grec->grec_auxwords, ntohs(igmpv3grec->grec_nsrcs), sizeof( __u32 ) );
				igmpv3grec = (struct igmpv3_grec *)( (char*)igmpv3grec + sizeof( struct igmpv3_grec ) + (igmpv3grec->grec_auxwords+ntohs(igmpv3grec->grec_nsrcs))*sizeof(u_int32 ) );
				//diag_printf( "count result: 0x%x\n", igmpv3grec );
			}		
			break;
}	
		
	
	#endif
	default:
	    DBG_PRINT("ignoring unknown IGMP message type %x from %s to %s",
		igmp->igmp_type, inet_fmt(src, s1),
		inet_fmt(dst, s2));
	    return;
    }
}

/*
 * Some IGMP messages are more important than others.  This routine
 * determines the logging level at which to log a send error (often
 * "No route to host").  This is important when there is asymmetric
 * reachability and someone is trying to, i.e., mrinfo me periodically.
 */
static int
igmp_log_level(type, code)
    u_int type, code;
{
    switch (type) {
	case IGMP_MTRACE_RESP:
	    return LOG_INFO;

	case IGMP_DVMRP:
	  switch (code) {
	    case DVMRP_NEIGHBORS:
	    case DVMRP_NEIGHBORS2:
		return LOG_INFO;
	  }
    }
    return LOG_WARNING;
}

/*
 * Construct an IGMP message in the output packet buffer.  The caller may
 * have already placed data in that buffer, of length 'datalen'.
 */
void
build_igmp(src, dst, type, code, group, datalen)
    u_int32 src, dst;
    int type, code;
    u_int32 group;
    int datalen;
{
    struct ip *ip;
    struct igmp *igmp;
    extern int curttl;

    ip                      = (struct ip *)send_buf;
    ip->ip_src.s_addr       = src;
    ip->ip_dst.s_addr       = dst;
    ip->ip_len              = MIN_IP_HEADER_LEN + IGMP_MINLEN + datalen;
#ifdef RAW_OUTPUT_IS_RAW
    ip->ip_len		    = htons(ip->ip_len);
#endif
    if (IN_MULTICAST(ntohl(dst))) {
	ip->ip_ttl = curttl;
    } else {
	ip->ip_ttl = MAXTTL;
    }

    igmp                    = (struct igmp *)(send_buf + MIN_IP_HEADER_LEN);
    igmp->igmp_type         = type;
    igmp->igmp_code         = code;
    igmp->igmp_group.s_addr = group;
    igmp->igmp_cksum        = 0;
    igmp->igmp_cksum        = inet_cksum((u_short *)igmp,
					 IGMP_MINLEN + datalen);
}

/* 
 * Call build_igmp() to build an IGMP message in the output packet buffer.
 * Then send the message from the interface with IP address 'src' to
 * destination 'dst'.
 */
void
send_igmp(src, dst, type, code, group, datalen)
    u_int32 src, dst;
    int type, code;
    u_int32 group;
    int datalen;
{
    struct sockaddr_in sdst;
    int setloop = 0;
	
	DBG_PRINT("src:%x.dst:%x.group:%x.type:%x.code:%x.[%s]:[%d].\n",src,dst,group,type,code,__FUNCTION__,__LINE__);
    build_igmp(src, dst, type, code, group, datalen);

    if (IN_MULTICAST(ntohl(dst))) {
		k_set_if(src);
		if (type != IGMP_DVMRP || dst == allhosts_group) {
		    setloop = 1;
		    k_set_loop(TRUE);
		}
    }

    bzero(&sdst, sizeof(sdst));
    sdst.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
    sdst.sin_len = sizeof(sdst);
#endif
    sdst.sin_addr.s_addr = dst;
	//diag_printf("----------------------\nsend igmp!\n--------------\n[%s]:[%d].\n",__FUNCTION__,__LINE__);
	#if 1
    if (sendto(igmp_socket, send_buf,
			MIN_IP_HEADER_LEN + IGMP_MINLEN + datalen, 0,
			(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
			
	if (errno == ENETDOWN)
	    check_vif_state();
	else
	    log(igmp_log_level(type, code), errno,
		"sendto to %s on %s",
		inet_fmt(dst, s1), inet_fmt(src, s2));
    }
	#endif
    if (setloop)
	    k_set_loop(FALSE);

    IF_DEBUG(DEBUG_PKT|igmp_debug_kind(type, code))
    log(LOG_DEBUG, 0, "SENT %s from %-15s to %s",
	igmp_packet_kind(type, code), src == INADDR_ANY ? "INADDR_ANY" :
				 inet_fmt(src, s1), inet_fmt(dst, s2));
}
