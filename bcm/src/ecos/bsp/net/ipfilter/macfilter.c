/*
 * MAC filter.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: macfilter.c,v 1.2 2010-05-11 10:26:09 Exp $
 *
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <sys/malloc.h>
#include "netinet/ip_compat.h"
#include "macf.h"
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
#define HASHTB_NUM		20
struct macfilter macfilhash[HASHTB_NUM];
#else
#define HASHTB_NUM		10
struct macfilter macfilhash[HASHTB_NUM];
#endif
int macfil_default_action;
int macfilter_num = 0;

extern int (*macfilter_checkp)(struct ifnet *ifp, char *eh, struct mbuf *m);

#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
extern int mac_filter_mode;
#endif
/* Matching the MAC filter table entries */
int
macfilter_match(struct ifnet *ifp, char *head, struct mbuf *m)
{
	struct macfilter *entry;
	struct in_ifaddr *ia;
	struct ip *ip;
	int idx;
	//char vlan2[5];

//roy add,20101111
	if(head == NULL)
		return MACF_PASS;

	//if (macfilter_num == 0)
	//	return macfil_default_action;
	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) == 0)
		return MACF_PASS;


	if (m->m_pkthdr.len < 20)
		return macfil_default_action;
	ip = mtod(m, struct ip *);
	if (ip->ip_dst.s_addr == INADDR_BROADCAST)
		return MACF_PASS;
	
/* Check destination IP, if it is to the interface IP, pass it */
#if defined(__OpenBSD__)
	for (ia = in_ifaddr.tqh_first; ia; ia = ia->ia_list.tqe_next) {
		if (ip->ip_dst.s_addr == ia->ia_addr.sin_addr.s_addr)
			return MACF_PASS;
	}
#else
	TAILQ_FOREACH(ia, &in_ifaddrhead, ia_link)
		if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr)
			return MACF_PASS;
#endif

	//macfil_default_action 表示对不在列表中的做何处理
	if (macfilter_num == 0)
		return macfil_default_action;
#if 0
	idx = MAC2INDEX(&head[6]);
	entry = macfilhash[idx];
	while (entry) {	
#endif
	for(idx = 0;idx<HASHTB_NUM;idx++){
		entry = (struct macfilter *)&macfilhash[idx];
		//diag_printf("entry->mac = %s\n",entry->mac);
		//diag_printf("macfilter_num = %d\n",macfilter_num);
		
		if (memcmp(&head[6], entry->mac, ETHER_ADDR_LEN) == 0) {
			//在列表中
#ifdef CONFIG_MACFLT_LOG
			if ((MACF_BLOCK^macfil_default_action) == MACF_BLOCK)
				syslog(LOG_KERN|LOG_NOTICE, "**Drop Packet** [%s] match", ether_sprintf(entry->mac));
#endif
			return macfil_default_action^MACF_BLOCK;
		}
		
#if 0
		entry = entry->next;
#endif
	}
	return macfil_default_action;
}

int macfilter_set_mode(int mode)
{
	int error = 0;

	switch (mode) {
	case MACF_MODE_DISABLED:
		macfilter_flush();
		macfilter_checkp = NULL;
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		mac_filter_mode = MACF_MODE_DISABLED;
		#endif
		break;
	case MACF_MODE_DENY:
		macfilter_flush();
		macfilter_checkp = macfilter_match;
		//memset(macfilhash, 0, sizeof(macfilhash));
		//如果是仅禁止,那么不在列表中的就是PASS
		macfil_default_action = MACF_PASS;
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		mac_filter_mode = MACF_MODE_DENY;
		#endif
		break;
	case MACF_MODE_PASS:
		macfilter_flush();
		macfilter_checkp = macfilter_match;
		//memset(macfilhash, 0, sizeof(macfilhash));
		//如果是仅允许,那么不在列表中的就是BLOCK
		macfil_default_action = MACF_BLOCK;
		#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
		mac_filter_mode = MACF_MODE_PASS;
		#endif
		break;
	default:
		error = EINVAL;
		break;
	}
	return error;
}

/* Delete a mac filter entry */
int
macfilter_del(struct macfilter *macinfo)
{
	int idx;// = MAC2INDEX(macinfo->mac);
	struct macfilter *entry;//, *prev = NULL;
#if 0	
	entry = macfilhash[idx];
	while (entry) {
		if (memcmp(macinfo->mac, entry->mac, ETHER_ADDR_LEN) == 0) {
			if (prev)
				prev->next = entry->next;
			else
				macfilhash[idx] = entry->next;
			macfilter_num--;
			free(entry, M_PFIL);
			return 0;
		}
		prev = entry;
		entry = entry->next;
	}
	//return -1;
#else
	for(idx = 0;idx <HASHTB_NUM;idx++)
	{
		entry = (struct macfilter *)&macfilhash[idx];
		if (memcmp(macinfo->mac, entry->mac, ETHER_ADDR_LEN) == 0) {
			memset(&macfilhash[idx],'\0',sizeof(struct macfilter));
			macfilter_num--;
			break;
		}
	}
#endif
	return 0;
}

/* Add a mac filter entry */
int
macfilter_add(struct macfilter *macinfo)
{
	struct macfilter *entry;
	int idx;// = MAC2INDEX(macinfo->mac);
#if 0
	/* prevent dupication */
	macfilter_del(macinfo);
	entry = (struct macfilter *)malloc(sizeof(struct macfilter), M_PFIL, M_NOWAIT);
	if (entry == 0)
		return -1;
	memcpy(entry, macinfo, sizeof(struct macfilter));
	entry->next = macfilhash[idx];
	macfilhash[idx] = entry;
	macfilter_num++;
#else
	for(idx =0; idx<HASHTB_NUM;idx++){
		entry = (struct macfilter *)&macfilhash[idx];
		if(memcmp(entry->mac,"\0\0\0\0\0\0",ETHER_ADDR_LEN) == 0){
			memcpy(&macfilhash[idx],macinfo, sizeof(struct macfilter));
			macfilter_num++;
			break;
		}
	}

#endif
	return 0;
}

/* Flush all the mac filter entry out */
void
macfilter_flush(void)
{
#if 0
	struct macfilter *entry, *next_entry;
	int i;

	/* Clear all the hash entry */
	for (i = 0; i < HASHTB_NUM; i++) {
		/* Get the start of each hash entry */ 
		entry = macfilhash[i];
		while (entry) {
			next_entry = entry->next;
			free(entry, M_PFIL);
			entry = next_entry;
		}
	}
#endif

	macfilter_num = 0;
	memset(macfilhash, '\0', HASHTB_NUM*sizeof(struct macfilter));// *));
}
