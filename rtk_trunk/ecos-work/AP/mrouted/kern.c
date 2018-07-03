/*
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE".  Use of the mrouted program represents acceptance of
 * the terms and conditions listed in that file.
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 *
 * kern.c,v 3.8.4.10 1998/01/06 02:00:51 fenner Exp
 */

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include "defs.h"

int curttl = 0;

void k_set_rcvbuf(bufsize, minsize)
    int bufsize;
    int minsize;
{
    int delta = bufsize / 2;
    int iter = 0;

    /*
     * Set the socket buffer.  If we can't set it as large as we
     * want, search around to try to find the highest acceptable
     * value.  The highest acceptable value being smaller than
     * minsize is a fatal error.
     */
    if (setsockopt(igmp_socket, SOL_SOCKET, SO_RCVBUF,
		(char *)&bufsize, sizeof(bufsize)) < 0) {
	bufsize -= delta;
	while (1) {
	    iter++;
	    if (delta > 1)
		delta /= 2;

	    if (setsockopt(igmp_socket, SOL_SOCKET, SO_RCVBUF,
			(char *)&bufsize, sizeof(bufsize)) < 0) {
		    bufsize -= delta;
	    } else {
		    if (delta < 1024)
			break;
		    bufsize += delta;
	    }
	}
	if (bufsize < minsize) {
	    log(LOG_ERR, 0, "OS-allowed buffer size %u < app min %u",
		bufsize, minsize);
	    /*NOTREACHED*/
	}
    }
    IF_DEBUG(DEBUG_KERN)
    log(LOG_DEBUG, 0, "Got %d byte buffer size in %d iterations",
	    bufsize, iter);
}


void k_hdr_include(int bool)
   // int bool;
{
#ifdef IP_HDRINCL
    if (setsockopt(igmp_socket, IPPROTO_IP, IP_HDRINCL,
		   (char *)&bool, sizeof(bool)) < 0)
	log(LOG_ERR, errno, "setsockopt IP_HDRINCL %u", bool);
#endif
}


void k_set_ttl(t)
    int t;
{
#ifndef RAW_OUTPUT_IS_RAW
    u_char ttl;

    ttl = t;
    if (setsockopt(igmp_socket, IPPROTO_IP, IP_MULTICAST_TTL,
		   (char *)&ttl, sizeof(ttl)) < 0)
	log(LOG_ERR, errno, "setsockopt IP_MULTICAST_TTL %u", ttl);
#endif
    curttl = t;
}


void k_set_loop(l)
    int l;
{
    u_char loop;

    loop = l;
    if (setsockopt(igmp_socket, IPPROTO_IP, IP_MULTICAST_LOOP,
		   (char *)&loop, sizeof(loop)) < 0)
	log(LOG_ERR, errno, "setsockopt IP_MULTICAST_LOOP %u", loop);
}


void k_set_if(ifa)
    u_int32 ifa;
{
    struct in_addr adr;

    adr.s_addr = ifa;
    if (setsockopt(igmp_socket, IPPROTO_IP, IP_MULTICAST_IF,
		   (char *)&adr, sizeof(adr)) < 0)
	log(LOG_ERR, errno, "setsockopt IP_MULTICAST_IF %s",
	    		    inet_fmt(ifa, s1));
}


void k_join(grp, ifaddr)
    u_int32 grp;
    u_int32 ifaddr;
{
    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = grp;
    mreq.imr_interface.s_addr = ifaddr;
	DBG_PRINT("IP_ADD_MEMBERSHIP.grp:%x.ifaddr:%x.[%s]:[%d].\n",grp,ifaddr,__FUNCTION__,__LINE__);
    if (setsockopt(igmp_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		   (char *)&mreq, sizeof(mreq)) < 0){
		diag_printf("can't join group %s on interface %s",
				inet_fmt(grp, s1), inet_fmt(ifaddr, s2)); 
	log(LOG_WARNING, errno, "can't join group %s on interface %s",
				inet_fmt(grp, s1), inet_fmt(ifaddr, s2));
	}
}


void k_leave(grp, src)
    u_int32 grp;
    u_int32 src;
{
    struct ip_mreq mreq;
	
	struct uvif *v;
	vifi_t vifi=igmp_up_if_idx;
	u_int32 ifaddr;
	
	v   = &uvifs[vifi];
    ifaddr = v->uv_lcl_addr;
	mreq.imr_multiaddr.s_addr = grp;
    mreq.imr_interface.s_addr = ifaddr;
	DBG_PRINT("IP_DROP_MEMBERSHIP.grp:%x.ifaddr:%x.[%s]:[%d].\n",grp,ifaddr,__FUNCTION__,__LINE__);
    if (setsockopt(igmp_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		   (char *)&mreq, sizeof(mreq)) < 0){
		diag_printf("can't leave group %s on interface %s",
				inet_fmt(grp, s1), inet_fmt(ifaddr, s2)); 
		log(LOG_WARNING, errno, "can't leave group %s on interface %s",
				inet_fmt(grp, s1), inet_fmt(ifaddr, s2));
	}
}


void k_init_dvmrp()
{

    int v=1;
	
	if (igmp_socket == 0) 
	{
		 
	  	if( (igmp_socket = socket( AF_INET, SOCK_RAW, IPPROTO_IGMP )) < 0 ){
    		log( LOG_ERR, errno, "IGMP socket open" ); 
			return ;
  		}	
		
#ifdef 	ECOS_DBG_STAT
		dbg_stat_add(dbg_igmpproxy_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
	}	
	
    if (setsockopt(igmp_socket, IPPROTO_IP, MRT_INIT,
	   		(char *)&v, sizeof(int)) < 0)

	{
		DBG_PRINT("can't enable Multicast routing in kernel.[%s]:[%d].\n",__FUNCTION__,__LINE__);
		log(LOG_ERR, errno, "can't enable Multicast routing in kernel");
	}
	else
		DBG_PRINT("enable Multicast routing in kernel.[%s]:[%d].\n",__FUNCTION__,__LINE__);

	return;
}


void k_stop_dvmrp()
{

	if(igmp_socket)
	{
		if ((setsockopt(igmp_socket, IPPROTO_IP, MRT_DONE,
		   		(char *)NULL, 0) < 0) ){
		   	
			DBG_PRINT("can't enable Multicast routing in kernel.[%s]:[%d].\n",__FUNCTION__,__LINE__);
			log(LOG_ERR, errno, "can't disable Multicast routing in kernel");
		}
		else
			DBG_PRINT("disable Multicast routing in kernel.\n");	
	}
}

void k_reinit_dvmrp()
{
	int Va = 1;
	
	if(igmp_socket)
	{
		if (setsockopt(igmp_socket, IPPROTO_IP, MRT_DONE,
			   (char *)NULL, 0) < 0)
			log(LOG_WARNING, errno, "can't disable Multicast routing in kernel");
		else
			DBG_PRINT("disable Multicast routing in kernel.\n");	
		
		if (setsockopt(igmp_socket, IPPROTO_IP, MRT_INIT,
			   (char *)&Va, sizeof(Va)) < 0)
		{
			diag_printf("can't enable Multicast routing in kernel.[%s]:[%d].\n",__FUNCTION__,__LINE__);
			log(LOG_ERR, errno, "can't enable Multicast routing in kernel");
		}
	}
}

void k_add_vif(vifi, v)
    vifi_t vifi;
    struct uvif *v;
{
    struct vifctl vc;
	
    vc.vifc_vifi            = vifi;
    //vc.vifc_flags           = v->uv_flags | (VIFF_TUNNEL|VIFF_SRCRT|IFF_MULTICAST);

	vc.vifc_flags           = IFF_MULTICAST;//120816hx
    vc.vifc_threshold       = v->uv_threshold;
    vc.vifc_rate_limit	    = v->uv_rate_limit;
    vc.vifc_lcl_addr.s_addr = v->uv_lcl_addr;
    //vc.vifc_rmt_addr.s_addr = v->uv_rmt_addr;
    vc.vifc_rmt_addr.s_addr = INADDR_ANY;//0816hx
	DBG_PRINT("vc.vifi:%d.flag:%x.lcladdr:%x.rmt_addr:%x.[%s]:[%d].\n",vc.vifc_vifi,vc.vifc_flags,vc.vifc_lcl_addr.s_addr,vc.vifc_rmt_addr.s_addr,__FUNCTION__,__LINE__);
    if (setsockopt(igmp_socket, IPPROTO_IP, MRT_ADD_VIF,
		   (char *)&vc, sizeof(vc)) < 0){
	log(LOG_ERR, errno, "setsockopt MRT_ADD_VIF on vif %d", vifi);
	}
}


void k_del_vif(vifi)
    vifi_t vifi;
{
    if (setsockopt(igmp_socket, IPPROTO_IP, MRT_DEL_VIF,
		   (char *)&vifi, sizeof(vifi)) < 0)
	log(LOG_ERR, errno, "setsockopt MRT_DEL_VIF on vif %d", vifi);
}


/*
 * Adds a (source, mcastgrp) entry to the kernel
 */
void k_add_rg(origin, g)
    u_int32 origin;
    struct gtable *g;
{
    struct mfcctl mc;
    vifi_t i;
	
	//diag_printf("origin:%x,g->gt_mcastgrp:%x.[%s]:[%d].\n",origin,g->gt_mcastgrp,__FUNCTION__,__LINE__);
#ifdef DEBUG_MFC
    md_log(MD_ADD, origin, g->gt_mcastgrp);
#endif
    /* copy table values so that setsockopt can process it */
    //mc.mfcc_origin.s_addr = origin;
    mc.mfcc_origin.s_addr =0;
#ifdef OLD_KERNEL
    mc.mfcc_originmask.s_addr = 0xffffffff;
#endif
    mc.mfcc_mcastgrp.s_addr = g->gt_mcastgrp;
	/*
    mc.mfcc_parent = g->gt_route ? g->gt_route->rt_parent : NO_VIF;
	
	
	for (i = 0; i < numvifs; i++)
	{
		if((1<<i)&g->gt_grpmems)
		{
			mc.mfcc_parent =i;
			break;
		}	
	}	
	*/
	//mc.mfcc_parent =1;
	mc.mfcc_parent = igmp_up_if_idx;
	//diag_printf("mc.mfcc_parent:%d,g->gt_route:%d,[%s]:[%d].\n",mc.mfcc_parent,g->gt_route,__FUNCTION__,__LINE__);
	memset(mc.mfcc_ttls,255,sizeof(mc.mfcc_ttls));
	
	mc.mfcc_ttls[igmp_down_if_idx]=1;
	/*
	for (i = 0; i < numvifs; i++){
		mc.mfcc_ttls[i] = g->gt_ttls[i];
	}
	*/
    /* write to kernel space */
	DBG_PRINT("MRT_ADD_MFC[%s]:[%d].\n",__FUNCTION__,__LINE__);
    if (setsockopt(igmp_socket, IPPROTO_IP, MRT_ADD_MFC,
		   (char *)&mc, sizeof(mc)) < 0) {
	   diag_printf("setsockopt MRT_ADD_MFC<0[%s]:[%d].\n",__FUNCTION__,__LINE__);
#ifdef DEBUG_MFC
	md_log(MD_ADD_FAIL, origin, g->gt_mcastgrp);
#endif
	log(LOG_WARNING, errno, "setsockopt MRT_ADD_MFC",
		inet_fmt(origin, s1), inet_fmt(g->gt_mcastgrp, s2));
    }
}


/*
 * Deletes a (source, mcastgrp) entry from the kernel
 */
int k_del_rg(origin, g)
    u_int32 origin;
    struct gtable *g;
{
    struct mfcctl mc;
    int retval;
	DBG_PRINT("MRT_DEL_MFC.[%s]:[%d].\n",__FUNCTION__,__LINE__);
#ifdef DEBUG_MFC
    md_log(MD_DEL, origin, g->gt_mcastgrp);
#endif
	origin=0;
    /* copy table values so that setsockopt can process it */
    mc.mfcc_origin.s_addr = origin;
#ifdef OLD_KERNEL
    mc.mfcc_originmask.s_addr = 0xffffffff;
#endif
    mc.mfcc_mcastgrp.s_addr = g->gt_mcastgrp;

    /* write to kernel space */
    if ((retval = setsockopt(igmp_socket, IPPROTO_IP, MRT_DEL_MFC,
		   (char *)&mc, sizeof(mc))) < 0) {
#ifdef DEBUG_MFC
	md_log(MD_DEL_FAIL, origin, g->gt_mcastgrp);
#endif
	log(LOG_WARNING, errno, "setsockopt MRT_DEL_MFC of (%s %s)",
		inet_fmt(origin, s1), inet_fmt(g->gt_mcastgrp, s2));
    }

    return retval;
}	

/*
 * Get the kernel's idea of what version of mrouted needs to run with it.
 */
int k_get_version()
{
#ifdef OLD_KERNEL
    return -1;
#else
    int vers;
    int len = sizeof(vers);

    if (getsockopt(igmp_socket, IPPROTO_IP, MRT_VERSION,
			(char *)&vers, &len) < 0)
	log(LOG_ERR, errno,
		"getsockopt MRT_VERSION: perhaps your kernel is too old");
	
	DBG_PRINT("getsockopt MRT_VERSION:%d.\n",vers);
    return vers;
#endif
}

#if 0
/*
 * Get packet counters
 */
int
k_get_vif_count(vifi, icount, ocount, ibytes, obytes)
    vifi_t vifi;
    int *icount, *ocount, *ibytes, *obytes;
{
    struct sioc_vif_req vreq;
    int retval = 0;

    vreq.vifi = vifi;
    if (ioctl(udp_socket, SIOCGETVIFCNT, (char *)&vreq) < 0) {
	log(LOG_WARNING, errno, "SIOCGETVIFCNT on vif %d", vifi);
	vreq.icount = vreq.ocount = vreq.ibytes =
		vreq.obytes = 0xffffffff;
	retval = 1;
    }
    if (icount)
	*icount = vreq.icount;
    if (ocount)
	*ocount = vreq.ocount;
    if (ibytes)
	*ibytes = vreq.ibytes;
    if (obytes)
	*obytes = vreq.obytes;
    return retval;
}

/*
 * Get counters for a desired source and group.
 */
int
k_get_sg_count(src, grp, pktcnt, bytecnt, wrong_if)
    u_int32 src;
    u_int32 grp;
    struct sg_count *retval;
{
    struct sioc_sg_req sgreq;
    int retval = 0;

    sgreq.src.s_addr = src;
    sgreq.grp.s_addr = grp;
    if (ioctl(udp_socket, SIOCGETSGCNT, (char *)&sgreq) < 0) {
	log(LOG_WARNING, errno, "SIOCGETSGCNT on (%s %s)",
	    inet_fmt(src, s1), inet_fmt(grp, s2));
	sgreq.pktcnt = sgreq.bytecnt = sgreq.wrong_if = 0xffffffff;
	return 1;
    }
    if (pktcnt)
    	*pktcnt = sgreq.pktcnt;
    if (bytecnt)
    	*bytecnt = sgreq.bytecnt;
    if (wrong_if)
    	*wrong_if = sgreq.wrong_if;
    return retval;
}
#endif
