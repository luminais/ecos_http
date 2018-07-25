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
 * $Id: ip_filfast.c,v 1.4 2010-08-07 07:32:04 Exp $
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
#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include "netinet/ip_nat.h"
#include "netinet/ip_frag.h"
#include "ip_dev.h"
#include <sys/ioctl.h>
#include "ip_filfast.h"
#include "macf.h"
#include "urlf.h"
#include "wan2lanf.h"
#include "url_rule_match.h"

#ifdef __CONFIG_AL_SECURITY__
#include "al_security.h"
#endif




extern int ip_fastpath_init(void);
extern int ip_fastpath_remove(void);

int fr_fastcheck(struct ifnet *ifp, char *eh, struct mbuf *m);

int (*macfilter_checkp)(struct ifnet *ifp, char *eh, struct mbuf *m);
int (*urlfilter_checkp)(struct ifnet *ifp, char *eh, struct mbuf *m);
int (*ip_fastpath)(struct ifnet *ifp, struct mbuf **mpp);
int (*wan2lanfilter_checkp)(struct ifnet *ifp, char *eh, struct mbuf *m);
int (*url_match_rule)(struct ifnet *ifp, char *eh, struct mbuf *m) = NULL;

int url_match_rule_enabled(void)
{
	return (url_match_rule==NULL?0:1);
}

#define FASTCHECK_PRIORITY 1	/* highest priority */
struct ipdev fastcheck_ipdev;

#ifdef __CONFIG_TENDA_MULTI__

int (*macfilter_checkp_quebb)(struct ifnet *ifp, char *eh, struct mbuf *m);


/* Matching the MAC filter table entries */
int
macfilter_match_quebb(struct ifnet *ifp, char *head, struct mbuf *m)
{

	if(head == NULL)
		return 0;//MACF_PASS;

	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) != 0)//from wan side
		return 0;
	
	/* huang add */
	if(ifp->if_index == 6)
	{
		//只能通过的MAC地址:0023f8、988b5d、002104
		if((((head[6] &0xFF) == 0x00) && ((head[7]&0xFF) == 0x23) && ((head[8]&0xFF) == 0xf8)) ||
			(((head[6]&0xFF) == 0x98) && ((head[7]&0xFF) == 0x8b) && ((head[8]&0xFF) == 0x5d)) ||
			(((head[6]&0xFF) == 0x00) && ((head[7]&0xFF) == 0x21) && ((head[8]&0xFF) == 0x04))/* ||
			(((head[6]&0xFF) == 0x00) && ((head[7]&0xFF) == 0xb0) && ((head[8]&0xFF) == 0x0c))*/)
		{
			//MAC前缀匹配，数据通过
			return 0;//MACF_PASS;
		}
		else{
			//MAC前缀不匹配，数据丢弃
			return 1;//MACF_BLOCK;
		}
	}
	/* add end */	

	return 0;//MACF_PASS;

}
#endif

/*
 * fr_fast_check.
 * fast path hook functions include urlfilter, mac filter and ipnat_fastpath
 */
int
fr_fastcheck(struct ifnet *ifp, char *eh, struct mbuf *m)
{
	int rc = 0;

#ifdef __CONFIG_AL_SECURITY__
	if(al_security_checkp){
		rc = (*al_security_checkp)(ifp, eh, m);
		if (rc != 0) {
				//m_freem(m);
				return 1;
		}
	}

#endif

#ifdef __CONFIG_TENDA_MULTI__
	if(macfilter_checkp_quebb){
		rc =(*macfilter_checkp_quebb)(ifp, eh, m);
		if (rc != 0) {
				m_freem(m);
				return 1;
		}
	}
#endif	

/*block package like this:
  *SRC_MAC:any SRC:any|DST_MAC:wan mac DST:lan side ip
  */
	if(wan2lanfilter_checkp){
		rc =(*wan2lanfilter_checkp)(ifp, eh, m);
		if (rc != 0) {
				m_freem(m);
				return 1;
		}
	}	

	/* Do MAC filter */
	if (macfilter_checkp) {
		rc = (*macfilter_checkp)(ifp, eh, m);
		if (rc != 0) {
			m_freem(m);
			return 1;
		}
	}

	/* Do URL filter */
	if (urlfilter_checkp) {
		rc = (*urlfilter_checkp)(ifp, eh, m);
		if (rc != 0) {
			m_freem(m);
			return 1;
		}
	}

	if(url_match_rule) {
		rc = (*url_match_rule)(ifp, eh, m);
		if (rc != 0) {
				//m_freem(m);
				return 1;
		}
	}

	/* Do fast nat */
	if (ip_fastpath) {
		rc = (*ip_fastpath)(ifp, &m);
		if (rc != 0) {
			return 1;	/* ip_fastpath eats this packet */
		}
	}

	return 0;
}

/*
 * fastcheck_enable, fastcheck_disable.
 * enable or disable the fast check hook function.
 * fast path hook functions include urlfilter, mac filter and ipnat_fastpath
 */
static int
fastcheck_enable(void)
{
	int rc, s;

	s = splnet();
	memset(&fastcheck_ipdev, 0, sizeof(fastcheck_ipdev));
	fastcheck_ipdev.func = fr_fastcheck;
	fastcheck_ipdev.priority = FASTCHECK_PRIORITY;
	rc = ipdev_add(&fastcheck_ipdev);
	
#ifdef __CONFIG_AL_SECURITY__
	get_lan_ip_addr();
#endif
	
	splx(s);
	return rc;
}

static int
fastcheck_disable(void)
{
	int s;

	s = splnet();
	ipdev_remove(&fastcheck_ipdev);
	splx(s);
	return 0;
}

int filfast_ioctl(int cmd, caddr_t data)
{
	int activate, mode;
	int error = EINVAL;

	switch (cmd) {
	case SIOCSFILFAST:
		if (data) {
			activate = *(int *)data;
			if (activate) {
				/* printf("Enabling fast filter\n"); */
				error = fastcheck_enable();
			} else {
				/* printf("Disabling fast filter\n"); */
				error = fastcheck_disable();
			}
		}
		break;
/*lq 去掉拨通的加速*/
#if 0
	case SIOCSFASTNAT:
		if (data) {
			activate = *(int *)data;
			if (activate) {
				/* printf("Enabling fast NAT\n"); */
				error = ip_fastpath_init();
			} else {
				/* printf("Disabling fast NAT\n"); */
				error = ip_fastpath_remove();
			}
		}
		break;
#endif
	case SIOCSMACFIL:
		if (data) {
			mode = *(int *)data;
			/* printf("Setting mac filter mode %d\n", mode); */
			error = macfilter_set_mode(mode);
		}
		break;
	case SIOCADMACFR:
		if (data) {
			/* printf("Adding mac filter entry\n"); */
			error = macfilter_add((struct macfilter *)data);
		}
		break;
	case SIOCRMMACFR:
		if (data) {
			/* printf("Deleting mac filter entry\n"); */
			error = macfilter_del((struct macfilter *)data);
		}
		break;
	case SIOCFLMACFR:
		/* printf("Flushing filter entries\n"); */
		macfilter_flush();
		error = 0;
		break;
#if 0//roy modify
	case SIOCSURLFIL:
		if (data) {
			activate = *(int *)data;
			if (activate) {
				/* printf("Enabling url filter\n"); */
				urlfilter_act();
				error = 0;
			} else {
				/* printf("Disabling url filter\n"); */
				urlfilter_inact();
				error = 0;
			}
		}
		break;
#else
//roy+++,2010/09/20
	case SIOCSURLFIL:
		if (data) {
			mode = *(int *)data;
			/* printf("Setting url filter mode %d\n", mode); */
			error = urlfilter_set_mode(mode);
		}
		break;
	case SIOCRMURLFR:
		if (data) {
			/* printf("del url filter entry\n"); */
			error = urlfilter_del((struct urlfilter *)data);
		}
		break;
//2011/11/04
	case SIOCSWAN2LANFIL:
		if (data) {
			mode = *(int *)data;
			//printf("Setting url filter mode %d\n", mode);
			error = wan2lanfilter_set_mode(mode);
		}
		break;
	case SIOCADWAN2LAN_IP:
		if (data) {
			mode = *(int *)data;
			error = wan2lanf_set_ip(mode);
		}
		break;
	case SIOCADWAN2LAN_MASK:
		if (data) {
			mode = *(int *)data;
			error = wan2lanf_set_mask(mode);
		}
		break;
//+++
#endif
	case SIOCADURLFR:
		if (data) {
			/* printf("Adding url filter entry\n"); */
			error = urlfilter_add((struct urlfilter *)data);
		}
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
			error = 0;
		#endif
		break;
	case SIOCFLURLFR:
		/* printf("Flushing url filter entries\n"); */
		urlfilter_flush();
		error = 0;
		break;
#ifdef __CONFIG_TENDA_MULTI__
	case SIOCSFIL_QUEBB:
		if (data) {
				activate = *(int *)data;
				if (activate) {
					/* printf("Enabling fast NAT\n"); */
					macfilter_checkp_quebb = macfilter_match_quebb;
				} else {
					/* printf("Disabling fast NAT\n"); */
					macfilter_checkp_quebb = NULL;
				}
		}
		error = 0;
		break;
#endif

#ifdef __CONFIG_AL_SECURITY__//add by ll
	case SIOCALSECURITY:
		if(data){
			activate =  *(int *)data ;
			if(activate)
			{
				al_security_checkp = al_security_handle_hook;
			}else{
				al_security_checkp = NULL ;
			}

		}
		error = 0;
		break ;
#endif//end by ll
// add by luminais
	case SIOCURLMATCHRULE:
		if(data){
			activate =  *(int *)data ;
			if(activate)
			{
				url_record_post_init();
				nis_init_lanip();
				url_match_rule = url_match_rule_handle;
			}else{
				url_match_rule = NULL ;
			}

		}
		error = 0;
		break;
// luminais end
	default:
		error = EINVAL;
		break;
	}
	return error;
}
