/*	$KAME: mld6.c,v 1.25 2003/09/02 09:57:04 itojun Exp $	*/

/*
 * Copyright (C) 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1998-2001
 * The University of Southern California/Information Sciences Institute.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Part of this program has been derived from mrouted.
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted".
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 */

#include "defs.h"
#include <sys/uio.h>

/*
 * Exported variables.
 */

char *pim6dd_mld6_recv_buf=NULL;		/* input packet buffer */
char *pim6dd_mld6_send_buf=NULL;		/* output packet buffer */
int pim6dd_mld6_socket;		/* socket for all network I/O */
struct sockaddr_in6 pim6dd_allrouters_group = {sizeof(struct sockaddr_in6), AF_INET6};
struct sockaddr_in6 pim6dd_allnodes_group = {sizeof(struct sockaddr_in6), AF_INET6};

/* Extenals */

extern const struct in6_addr in6addr_linklocal_allnodes;

/* local variables. */
static struct sockaddr_in6 	pim6dd_dst = {sizeof(pim6dd_dst), AF_INET6};
static struct msghdr 		pim6dd_sndmh, pim6dd_rcvmh;
static struct iovec 		pim6dd_sndiov[2];
static struct iovec 		pim6dd_rcviov[2];
static struct sockaddr_in6 	pim6dd_from;
static u_char   			*pim6dd_rcvcmsgbuf = NULL;
static int					pim6dd_rcvcmsglen;

#ifndef USE_RFC2292BIS
u_int8_t pim6dd_raopt[IP6OPT_RTALERT_LEN];
#endif 
static char *pim6dd_sndcmsgbuf = NULL;
static int pim6dd_ctlbuflen = 0;
static u_int16_t pim6dd_rtalert_code;

/* local functions */

static void pim6dd_mld6_read __P((int i, fd_set * fds));
static void pim6dd_accept_mld6 __P((int len));
static void pim6dd_make_mld6_msg __P((int, int, struct sockaddr_in6 *,
	struct sockaddr_in6 *, struct in6_addr *, int, int, int, int));

#ifndef IP6OPT_ROUTER_ALERT	/* XXX to be compatible older systems */
#define IP6OPT_ROUTER_ALERT IP6OPT_RTALERT
#endif

/*
 * Open and initialize the MLD socket.
 */
int
pim6dd_init_mld6()
{
    struct icmp6_filter filt;
    int             on;

    pim6dd_rtalert_code = htons(IP6OPT_RTALERT_MLD);
    if (!pim6dd_mld6_recv_buf && (pim6dd_mld6_recv_buf = malloc(RECV_BUF_SIZE)) == NULL) {
	    pim6dd_log_msg(LOG_ERR, 0, "pim6dd_mld6_recv_buf malloc failed");
		return -1;
		}
#ifdef ECOS_DBG_STAT
    dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif

    if (!pim6dd_mld6_send_buf && (pim6dd_mld6_send_buf = malloc(RECV_BUF_SIZE)) == NULL) {
	    pim6dd_log_msg(LOG_ERR, 0, "pim6dd_mld6_send_buf malloc failed");
		free(pim6dd_mld6_recv_buf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		return -1;
		}
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif

    pim6dd_rcvcmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo)) +
	    CMSG_SPACE(sizeof(int));
    if (pim6dd_rcvcmsgbuf == NULL && (pim6dd_rcvcmsgbuf = malloc(pim6dd_rcvcmsglen)) == NULL) {
	    pim6dd_log_msg(LOG_ERR, 0,"pim6dd_rcvcmsgbuf malloc failed");
		free(pim6dd_mld6_recv_buf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		free(pim6dd_mld6_send_buf);

#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		return -1;
		}
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif
    
    IF_DEBUG(DEBUG_KERN)
        pim6dd_log_msg(LOG_DEBUG,0,"%d octets allocated for the emit/recept buffer mld6",RECV_BUF_SIZE);

    if ((pim6dd_mld6_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0) {
		pim6dd_log_msg(LOG_ERR, errno, "MLD6 socket");
		free(pim6dd_mld6_recv_buf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		free(pim6dd_mld6_send_buf);

#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		free(pim6dd_rcvcmsgbuf);

#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		return -1;
		}

    pim6dd_k_set_rcvbuf(pim6dd_mld6_socket, SO_RECV_BUF_SIZE_MAX,
		 SO_RECV_BUF_SIZE_MIN);	/* lots of input buffering */
    pim6dd_k_set_hlim(pim6dd_mld6_socket, MINHLIM);	/* restrict multicasts to one hop */
#if 0
    /*
     * Since we don't have to handle DMVRP messages via the MLD6 socket,
     * we can just let outgoing multicast packets be loop-backed.
     */
    pim6dd_k_set_loop(pim6dd_mld6_socket, FALSE);	/* disable multicast loopback     */
#endif

    /* address initialization */
    pim6dd_allnodes_group.sin6_addr = in6addr_linklocal_allnodes;
    if (inet_pton(AF_INET6, "ff02::2",
		  (void *) &pim6dd_allrouters_group.sin6_addr) != 1)
	pim6dd_log_msg(LOG_ERR, 0, "inet_pton failed for ff02::2");

    /* filter all non-MLD ICMP messages */
    ICMP6_FILTER_SETBLOCKALL(&filt);
    ICMP6_FILTER_SETPASS(ICMP6_MEMBERSHIP_QUERY, &filt);
    ICMP6_FILTER_SETPASS(ICMP6_MEMBERSHIP_REPORT, &filt);
    ICMP6_FILTER_SETPASS(ICMP6_MEMBERSHIP_REDUCTION, &filt);
    ICMP6_FILTER_SETPASS(MLD_MTRACE_RESP, &filt);
    ICMP6_FILTER_SETPASS(MLD_MTRACE, &filt);
    if (setsockopt(pim6dd_mld6_socket, IPPROTO_ICMPV6, ICMP6_FILTER, &filt,
		   sizeof(filt)) < 0)
	pim6dd_log_msg(LOG_ERR, errno, "setsockopt(ICMP6_FILTER)");

    /* specify to tell receiving interface */
    on = 1;
#ifdef IPV6_RECVPKTINFO
    if (setsockopt(pim6dd_mld6_socket, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on,
		   sizeof(on)) < 0)
	pim6dd_log_msg(LOG_ERR, errno, "setsockopt(IPV6_RECVPKTINFO)");
#else  /* old adv. API */
    if (setsockopt(pim6dd_mld6_socket, IPPROTO_IPV6, IPV6_PKTINFO, &on,
		   sizeof(on)) < 0)
	pim6dd_log_msg(LOG_ERR, errno, "setsockopt(IPV6_PKTINFO)");
#endif 
    on = 1;
    /* specify to tell value of hoplimit field of received IP6 hdr */
#ifdef IPV6_RECVHOPLIMIT
    if (setsockopt(pim6dd_mld6_socket, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on,
		   sizeof(on)) < 0)
	pim6dd_log_msg(LOG_ERR, errno, "setsockopt(IPV6_RECVHOPLIMIT)");
#else  /* old adv. API */
    if (setsockopt(pim6dd_mld6_socket, IPPROTO_IPV6, IPV6_HOPLIMIT, &on,
		   sizeof(on)) < 0)
	pim6dd_log_msg(LOG_ERR, errno, "setsockopt(IPV6_HOPLIMIT)");
#endif 
    /* initialize msghdr for receiving packets */
    pim6dd_rcviov[0].iov_base = (caddr_t) pim6dd_mld6_recv_buf;
    pim6dd_rcviov[0].iov_len = RECV_BUF_SIZE;
    pim6dd_rcvmh.msg_name = (caddr_t) & pim6dd_from;
    pim6dd_rcvmh.msg_namelen = sizeof(pim6dd_from);
    pim6dd_rcvmh.msg_iov = pim6dd_rcviov;
    pim6dd_rcvmh.msg_iovlen = 1;
    pim6dd_rcvmh.msg_control = (caddr_t) pim6dd_rcvcmsgbuf;
    pim6dd_rcvmh.msg_controllen = pim6dd_rcvcmsglen;

    /* initialize msghdr for sending packets */
    pim6dd_sndiov[0].iov_base = (caddr_t)pim6dd_mld6_send_buf;
    pim6dd_sndmh.msg_namelen = sizeof(struct sockaddr_in6);
    pim6dd_sndmh.msg_iov = pim6dd_sndiov;
    pim6dd_sndmh.msg_iovlen = 1;
    /* specifiy to insert router alert option in a hop-by-hop opt hdr. */
#ifndef USE_RFC2292BIS
    pim6dd_raopt[0] = IP6OPT_ROUTER_ALERT;
    pim6dd_raopt[1] = IP6OPT_RTALERT_LEN - 2;
    memcpy(&pim6dd_raopt[2], (caddr_t) & pim6dd_rtalert_code, sizeof(u_int16_t));
#endif 

    /* register MLD message handler */
    if (pim6dd_register_input_handler(pim6dd_mld6_socket, pim6dd_mld6_read) < 0)
	pim6dd_log_msg(LOG_ERR, 0,
	    "Couldn't register pim6dd_mld6_read as an input handler");

	return 0;
}

/* Read an MLD message */
static void
pim6dd_mld6_read(i, rfd)
    int             i;
    fd_set         *rfd;
{
    register int    mld6_recvlen;

    mld6_recvlen = recvmsg(pim6dd_mld6_socket, &pim6dd_rcvmh, 0); 		
#ifdef __ECOS
	pim6dd_rcvmh.msg_iov[0].iov_base = (caddr_t) pim6dd_mld6_recv_buf;
#endif

    if (mld6_recvlen < 0)
    {
	if (errno != EINTR)
	    pim6dd_log_msg(LOG_ERR, errno, "MLD6 recvmsg");
	return;
    }

    /* TODO: make it as a thread in the future releases */
    pim6dd_accept_mld6(mld6_recvlen);
}

/*
 * Process a newly received MLD6 packet that is sitting in the input packet
 * buffer.
 */
static void
pim6dd_accept_mld6(recvlen)
int recvlen;
{
	struct in6_addr *group, *dst = NULL;
	struct mld_hdr *mldh;
	struct cmsghdr *cm;
	struct in6_pktinfo *pi = NULL;
	int *hlimp = NULL;
	int ifindex = 0;
	struct sockaddr_in6 *src = (struct sockaddr_in6 *) pim6dd_rcvmh.msg_name;

	if (recvlen < sizeof(struct mld_hdr))
	{
		pim6dd_log_msg(LOG_WARNING, 0,
		    "received packet too short (%u bytes) for MLD header",
		    recvlen);
		return;
	}
	mldh = (struct mld_hdr *) pim6dd_rcvmh.msg_iov[0].iov_base;

	/*
	 * Packets sent up from kernel to daemon have ICMPv6 type = 0.
	 * Note that we set filters on the pim6dd_mld6_socket, so we should never
	 * see a "normal" ICMPv6 packet with type 0 of ICMPv6 type.
	 */
	if (mldh->mld_type == 0) {
		/* XXX: msg_controllen must be reset in this case. */
		pim6dd_rcvmh.msg_controllen = pim6dd_rcvcmsglen;

		pim6dd_process_kernel_call();
		return;
	}

	group = &mldh->mld_addr;

	/* extract optional information via Advanced API */
	for (cm = (struct cmsghdr *) CMSG_FIRSTHDR(&pim6dd_rcvmh);
	     cm;
	     cm = (struct cmsghdr *) CMSG_NXTHDR(&pim6dd_rcvmh, cm))
	{
		if (cm->cmsg_level == IPPROTO_IPV6 &&
		    cm->cmsg_type == IPV6_PKTINFO &&
		    cm->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo)))
		{
			pi = (struct in6_pktinfo *) (CMSG_DATA(cm));
			ifindex = pi->ipi6_ifindex;
			dst = &pi->ipi6_addr;
		}
		if (cm->cmsg_level == IPPROTO_IPV6 &&
		    cm->cmsg_type == IPV6_HOPLIMIT &&
		    cm->cmsg_len == CMSG_LEN(sizeof(int)))
			hlimp = (int *) CMSG_DATA(cm);
	}
	if (hlimp == NULL)
	{
		pim6dd_log_msg(LOG_WARNING, 0,
		    "failed to get receiving hop limit");
		return;
	}

	/* TODO: too noisy. Remove it? */
#undef NOSUCHDEF
#ifdef NOSUCHDEF
	IF_DEBUG(DEBUG_PKT | pim6dd_debug_kind(IPPROTO_ICMPV6, mldh->mld_type,
					mldh->mld_code))
		pim6dd_log_msg(LOG_DEBUG, 0, "RECV %s from %s to %s",
		    pim6dd_packet_kind(IPPROTO_ICMPV6,
				mldh->mld_type, mldh->mld_code),
		    pim6dd_inet6_fmt(&src->sin6_addr), pim6dd_inet6_fmt(dst));
#endif				/* NOSUCHDEF */

	/* for an mtrace message, we don't need strict checks */
	if (mldh->mld_type == MLD_MTRACE) {
		pim6dd_accept_mtrace(src, dst, group, ifindex, (char *)(mldh + 1),
			      mldh->mld_code, recvlen - sizeof(struct mld_hdr));
		return;
	}

	/* hop limit check */
	if (*hlimp != 1)
	{
		pim6dd_log_msg(LOG_WARNING, 0,
		    "received an MLD6 message with illegal hop limit(%d) from %s",
		    *hlimp, pim6dd_inet6_fmt(&src->sin6_addr));
		/* but accept the packet */
	}
	if (ifindex == 0)
	{
		pim6dd_log_msg(LOG_WARNING, 0, "failed to get receiving interface");
		return;
	}

	/* scope check */
	if (IN6_IS_ADDR_MC_NODELOCAL(&mldh->mld_addr))
	{
		pim6dd_log_msg(LOG_INFO, 0,
		    "RECV %s with an invalid scope: %s from %s",
		    pim6dd_packet_kind(IPPROTO_ICMPV6, mldh->mld_type,
				mldh->mld_code),
		    pim6dd_inet6_fmt(&mldh->mld_addr),
		    pim6dd_inet6_fmt(&src->sin6_addr));
		return;			/* discard */
	}

	/* source address check */
	if (!IN6_IS_ADDR_LINKLOCAL(&src->sin6_addr))
	{
		pim6dd_log_msg(LOG_INFO, 0,
		    "RECV %s from a non link local address: %s",
		    pim6dd_packet_kind(IPPROTO_ICMPV6, mldh->mld_type,
				mldh->mld_code),
		    pim6dd_inet6_fmt(&src->sin6_addr));
		return;
	}

	switch (mldh->mld_type)
	{
	case MLD_LISTENER_QUERY:
		pim6dd_accept_listener_query(src, dst, group,
				      ntohs(mldh->mld_maxdelay));
		return;

	case MLD_LISTENER_REPORT:
		pim6dd_accept_listener_report(src, dst, group);
		return;

	case MLD_LISTENER_DONE:
		pim6dd_accept_listener_done(src, dst, group);
		return;

	default:
		/* This must be impossible since we set a type filter */
		pim6dd_log_msg(LOG_INFO, 0,
		    "ignoring unknown ICMPV6 message type %x from %s to %s",
		    mldh->mld_type, pim6dd_inet6_fmt(&src->sin6_addr),
		    pim6dd_inet6_fmt(dst));
		return;
	}
}

static void
pim6dd_make_mld6_msg(type, code, src, dst, group, ifindex, delay, datalen, alert)
    int type, code, ifindex, delay, datalen, alert;
    struct sockaddr_in6 *src, *dst;
    struct in6_addr *group;
{
    struct sockaddr_in6 dst_sa;
    struct mld_hdr *mhp = (struct mld_hdr *)pim6dd_mld6_send_buf;
    int ctllen, hbhlen = 0;

    memset(&dst_sa, 0, sizeof(dst_sa));
    dst_sa.sin6_family = AF_INET6;
    dst_sa.sin6_len = sizeof(dst_sa);

    switch(type) {
    case MLD_MTRACE:
    case MLD_MTRACE_RESP:
	pim6dd_sndmh.msg_name = (caddr_t)dst;
	break;
    default:
	if (IN6_IS_ADDR_UNSPECIFIED(group))
	    dst_sa.sin6_addr = pim6dd_allnodes_group.sin6_addr;
	else
	    dst_sa.sin6_addr = *group;
	pim6dd_sndmh.msg_name = (caddr_t)&dst_sa;
	datalen = sizeof(struct mld_hdr);
	break;
    }
   
    bzero(mhp, sizeof(*mhp));
    mhp->mld_type = type;
    mhp->mld_code = code;
    mhp->mld_maxdelay = htons(delay);
    mhp->mld_addr = *group;

#ifdef __ECOS
    pim6dd_sndiov[0].iov_base = mhp;
#endif
    pim6dd_sndiov[0].iov_len = datalen;

    /* estimate total ancillary data length */
    ctllen = 0;
    if (ifindex != -1 || src)
	    ctllen += CMSG_SPACE(sizeof(struct in6_pktinfo));
    if (alert) {
#ifdef USE_RFC2292BIS
	if ((hbhlen = inet6_opt_init(NULL, 0)) == -1)
		pim6dd_log_msg(LOG_ERR, 0, "inet6_opt_init(0) failed");
	if ((hbhlen = inet6_opt_append(NULL, 0, hbhlen, IP6OPT_ROUTER_ALERT, 2,
				       2, NULL)) == -1)
		pim6dd_log_msg(LOG_ERR, 0, "inet6_opt_append(0) failed");
	if ((hbhlen = inet6_opt_finish(NULL, 0, hbhlen)) == -1)
		pim6dd_log_msg(LOG_ERR, 0, "inet6_opt_finish(0) failed");
	ctllen += CMSG_SPACE(hbhlen);
#else  /* old advanced API */
	hbhlen = inet6_option_space(sizeof(pim6dd_raopt));
	ctllen += hbhlen;
#endif
	
    }
    /* extend ancillary data space (if necessary) */
    if (pim6dd_ctlbuflen < ctllen) {
	    if (pim6dd_sndcmsgbuf)
        {   
		    free(pim6dd_sndcmsgbuf);
#ifdef ECOS_DBG_STAT
            dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
            pim6dd_sndcmsgbuf = NULL;
        }
	    if ((pim6dd_sndcmsgbuf = malloc(ctllen)) == NULL)
        {   
		    pim6dd_log_msg(LOG_ERR, 0, "pim6dd_make_mld6_msg: malloc failed"); /* assert */
            return; 
        }
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif
	    pim6dd_ctlbuflen = ctllen;
    }
    /* store ancillary data */
    if ((pim6dd_sndmh.msg_controllen = ctllen) > 0) {
	    struct cmsghdr *cmsgp;

	    pim6dd_sndmh.msg_control = pim6dd_sndcmsgbuf;
	    cmsgp = CMSG_FIRSTHDR(&pim6dd_sndmh);

	    if (ifindex != -1 || src) {
		    struct in6_pktinfo *pktinfo;

		    cmsgp->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
		    cmsgp->cmsg_level = IPPROTO_IPV6;
		    cmsgp->cmsg_type = IPV6_PKTINFO;
		    pktinfo = (struct in6_pktinfo *)CMSG_DATA(cmsgp);
		    memset((caddr_t)pktinfo, 0, sizeof(*pktinfo));
		    if (ifindex != -1)
			    pktinfo->ipi6_ifindex = ifindex;
		    if (src)
			    pktinfo->ipi6_addr = src->sin6_addr;
		    cmsgp = CMSG_NXTHDR(&pim6dd_sndmh, cmsgp);
	    }
	    if (alert) {
#ifdef USE_RFC2292BIS
		    int currentlen;
		    void *hbhbuf, *optp = NULL;

		    cmsgp->cmsg_len = CMSG_LEN(hbhlen);
		    cmsgp->cmsg_level = IPPROTO_IPV6;
		    cmsgp->cmsg_type = IPV6_HOPOPTS;
		    hbhbuf = CMSG_DATA(cmsgp);

		    if ((currentlen = inet6_opt_init(hbhbuf, hbhlen)) == -1)
			    pim6dd_log_msg(LOG_ERR, 0, "inet6_opt_init(len = %d) failed",
				hbhlen);
		    if ((currentlen = inet6_opt_append(hbhbuf, hbhlen,
						       currentlen,
						       IP6OPT_ROUTER_ALERT, 2,
						       2, &optp)) == -1)
			    pim6dd_log_msg(LOG_ERR, 0,
				"inet6_opt_append(len = %d/%d) failed",
				currentlen, hbhlen);
		    (void)inet6_opt_set_val(optp, 0, &pim6dd_rtalert_code,
					    sizeof(pim6dd_rtalert_code));
		    if (inet6_opt_finish(hbhbuf, hbhlen, currentlen) == -1)
			    pim6dd_log_msg(LOG_ERR, 0, "inet6_opt_finish(buf) failed");
#else  /* old advanced API */
		    if (inet6_option_init((void *)cmsgp, &cmsgp, IPV6_HOPOPTS))
			    pim6dd_log_msg(LOG_ERR, 0, /* assert */
				"pim6dd_make_mld6_msg: inet6_option_init failed");
		    if (inet6_option_append(cmsgp, pim6dd_raopt, 4, 0))
			    pim6dd_log_msg(LOG_ERR, 0, /* assert */
				"pim6dd_make_mld6_msg: inet6_option_append failed");
#endif 
		    cmsgp = CMSG_NXTHDR(&pim6dd_sndmh, cmsgp);
	    }
    }
    else
	    pim6dd_sndmh.msg_control = NULL; /* clear for safety */
}

void
pim6dd_send_mld6(type, code, src, dst, group, index, delay, datalen, alert)
    int type;
    int code;		/* for trace packets only */
    struct sockaddr_in6 *src;
    struct sockaddr_in6 *dst; /* may be NULL */
    struct in6_addr *group;
    int index, delay, alert;
    int datalen;		/* for trace packets only */
{
    struct sockaddr_in6 *dstp;
	
    pim6dd_make_mld6_msg(type, code, src, dst, group, index, delay, datalen, alert);
    dstp = (struct sockaddr_in6 *)pim6dd_sndmh.msg_name;

#ifdef __KAME__
    if (IN6_IS_ADDR_LINKLOCAL(&dstp->sin6_addr) || 
	IN6_IS_ADDR_MC_LINKLOCAL(&dstp->sin6_addr))
	dstp->sin6_scope_id = index;
#endif

    if (sendmsg(pim6dd_mld6_socket, &pim6dd_sndmh, 0) < 0) {
	if (errno == ENETDOWN)
	    pim6dd_check_vif_state();
	else
	    pim6dd_log_msg(pim6dd_log_level(IPPROTO_ICMPV6, type, 0), errno,
		"sendmsg to %s with src %s on %s",
		pim6dd_inet6_fmt(&dstp->sin6_addr),
		src ? pim6dd_inet6_fmt(&src->sin6_addr) : "(unspec)",
		pim6dd_ifindex2str(index));

	return;
    }
    
    IF_DEBUG(DEBUG_PKT|pim6dd_debug_kind(IPPROTO_IGMP, type, 0))
	pim6dd_log_msg(LOG_DEBUG, 0, "SENT %s from %-15s to %s",
	    pim6dd_packet_kind(IPPROTO_ICMPV6, type, 0),
	    src ? pim6dd_inet6_fmt(&src->sin6_addr) : "unspec",
	    pim6dd_inet6_fmt(&dstp->sin6_addr));
}

void pim6dd_clean_mld()
{

	if (pim6dd_mld6_recv_buf) {
		free(pim6dd_mld6_recv_buf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		pim6dd_mld6_recv_buf = NULL;		/* input packet buffer */
	}

	if (pim6dd_rcvcmsgbuf) {
		free(pim6dd_rcvcmsgbuf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		pim6dd_rcvcmsgbuf = NULL;
	}

#if 1
    if (pim6dd_sndcmsgbuf) {
		free(pim6dd_sndcmsgbuf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		pim6dd_sndcmsgbuf = NULL;
	}
    #endif

    pim6dd_ctlbuflen = 0;
}

