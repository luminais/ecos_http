/*	$KAME: prefixconf.c,v 1.33 2005/09/16 11:30:15 suz Exp $	*/

/*
 * Copyright (C) 2002 WIDE Project.
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
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/ioctl.h>

#include <net/if.h>
#ifdef __FreeBSD__
#include <net/if_var.h>
#endif

#include <netinet/in.h>

#ifdef __KAME__
#include <netinet6/in6_var.h>
#endif

#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dhcp6.h"
#ifdef __ECOS
#include "config6.h"
#else
#include "config.h"
#endif
#include "common.h"
#include "timer.h"
#include "dhcp6c_ia.h"
#include "prefixconf.h"
#include "network6.h"

char	tmp_prefixs[40]={0};

TAILQ_HEAD(siteprefix_list, siteprefix);
struct iactl_pd {
	struct iactl common;
	struct pifc_list *pifc_head;
	struct siteprefix_list siteprefix_head;
};
#define iacpd_ia common.iactl_ia
#define iacpd_callback common.callback
#define iacpd_isvalid common.isvalid
#define iacpd_duration common.duration
#define iacpd_renew_data common.renew_data
#define iacpd_rebind_data common.rebind_data
#define iacpd_reestablish_data common.reestablish_data
#define iacpd_release_data common.release_data
#define iacpd_cleanup common.cleanup
extern DHCP6C_STATUS dhcp6c_status;

struct siteprefix {
	TAILQ_ENTRY (siteprefix) link;

	struct dhcp6_prefix prefix;
	time_t updatetime;
	struct dhcp6_timer *timer;
	struct iactl_pd *ctl;
	TAILQ_HEAD(, dhcp6_ifprefix) ifprefix_list; /* interface prefixes */
};

struct dhcp6_ifprefix {
	TAILQ_ENTRY(dhcp6_ifprefix) plink;

	/* interface configuration */
	struct prefix_ifconf *ifconf;

	/* interface prefix parameters */
	struct sockaddr_in6 paddr;
	int plen;

	/* address assigned on the interface based on the prefix */
	struct sockaddr_in6 ifaddr;
};

static struct siteprefix *find_siteprefix __P((struct siteprefix_list *,
    struct dhcp6_prefix *, int));
static void remove_siteprefix __P((struct siteprefix *));
static int isvalid __P((struct iactl *));
static u_int32_t duration __P((struct iactl *));
static void cleanup __P((struct iactl *));
static int renew_prefix __P((struct iactl *, struct dhcp6_ia *,
    struct dhcp6_eventdata **, struct dhcp6_eventdata *));
static void renew_data_free __P((struct dhcp6_eventdata *));

static struct dhcp6_timer *siteprefix_timo __P((void *));

static int add_ifprefix __P((struct siteprefix *,
    struct dhcp6_prefix *, struct prefix_ifconf *));

extern struct dhcp6_timer *client6_timo6s __P((void *));
static int pd_ifaddrconf __P((ifaddrconf_cmd_t, struct dhcp6_ifprefix *ifpfx));

extern P_RTADVD_CFG rtadvd_cfg;
extern P_DHCP6S_CFG dhcp6s_cfg;
extern LAN6_STATUS lan_ipv6_status;
extern DHCP6C_STATUS dhcp6c_status;;

void start_lan_service6s(char *new_prefix)
{
	if(lan_ipv6_status.lan_prefix_mode==1)
		return ;

	if(dhcp6s_cfg == NULL)
		dhcp6s_cfg = (P_DHCP6S_CFG)malloc(sizeof(DHCP6S_CFG));
	if(rtadvd_cfg == NULL)
		rtadvd_cfg = (P_RTADVD_CFG)malloc(sizeof(RTADVD_CFG));
	
	memset(dhcp6s_cfg,0,sizeof(DHCP6S_CFG));
	memset(rtadvd_cfg,0,sizeof(RTADVD_CFG));

	dhcp6s_cfg->d6ns_mode=lan_ipv6_status.lan_dns_type;
	if(dhcp6s_cfg->d6ns_mode==1)
	{
		strcpy(dhcp6s_cfg->pri_dns,lan_ipv6_status.lan_pri_dns);
		strcpy(dhcp6s_cfg->sec_dns,lan_ipv6_status.lan_sec_dns);
	}
	else
	{
		strcpy(dhcp6s_cfg->pri_dns,dhcp6c_status.pri_dns);
		strcpy(dhcp6s_cfg->sec_dns,dhcp6c_status.sec_dns);
	}
	
	dhcp6s_cfg->dhcp6s_mode=lan_ipv6_status.lan_dhcp6_type;
	if(dhcp6s_cfg->dhcp6s_mode==1)  //auto:0   manu:1
	{
		lan6id2addr(lan_ipv6_status.lan_start_id, lan_ipv6_status.lan_end_id, new_prefix);	
		rtadvd_cfg->rtadvd_mFlag=1;
		rtadvd_cfg->rtadvd_oFlag=1;
		rtadvd_cfg->rapfopt[0].rtadvd_aFlag=0;
	
	}
	else
	{
		rtadvd_cfg->rtadvd_mFlag=0;
		rtadvd_cfg->rtadvd_oFlag=1;
		rtadvd_cfg->rapfopt[0].rtadvd_aFlag=1;
	}
	rtadvd_cfg->ra_maxinterval=10;
	rtadvd_cfg->ra_mininterval=0;
	strcpy(rtadvd_cfg->ra_interface,"br0");
	sprintf(rtadvd_cfg->rapfopt[0].prefix_addr,"%s::",new_prefix);
	rtadvd_cfg->rapfopt[0].pref_lifetime=1000;
	rtadvd_cfg->rapfopt[0].valid_lifetime=1300;

	if(lan_ipv6_status.lan_ipv6_enable==1)
	{
		dhcp6s_start();
		start_rtadvd();
	}
	else
	{
		lan6_dhcp_stop();
		lan6_rtadvd_stop();
	}

}

void  update_radvd_cfgs(struct in6_addr *prefix, int prefix_len)
{
	struct in6_addr new_addr;
	unsigned short sla=64;
	char new_prefix[40];
	if(prefix_len > 64 || prefix_len <= 0)
	{
		printf("error.\n");
		return ;
	}

	memcpy(&new_addr, prefix, sizeof(struct in6_addr));
	strcpy(new_prefix,in6addr2strs(&new_addr, 0));

	new_addr.s6_addr[7] = sla;
	sprintf(new_prefix,"%02x%02x:%02x%02x:%02x%02x:%02x%02x",new_addr.s6_addr[0],new_addr.s6_addr[1],
	new_addr.s6_addr[2],new_addr.s6_addr[3],new_addr.s6_addr[4],new_addr.s6_addr[5],
	new_addr.s6_addr[6],new_addr.s6_addr[7]);
	strcpy(tmp_prefixs,new_prefix);
}



int
update_prefix2s(ia, pinfo, pifc, dhcpifp, ctlp, callback)

	struct ia *ia;
	struct dhcp6_prefix *pinfo;
	struct pifc_list *pifc;
	struct dhcp6_if *dhcpifp;
	struct iactl **ctlp;
	void (*callback)__P((struct ia *));
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)*ctlp;
	struct siteprefix *sp;
	struct prefix_ifconf *pif;
	int spcreate = 0;
	struct timeval timo;
	
	if (pinfo->vltime != DHCP6_DURATION_INFINITE &&
	    (pinfo->pltime == DHCP6_DURATION_INFINITE ||
	    pinfo->pltime > pinfo->vltime)) {
		printf( "invalid prefix %s/%d: pltime (%lu) is larger than vltime (%lu)\n",
		    in6addr2strs(&pinfo->addr, 0), pinfo->plen,
		    pinfo->pltime, pinfo->vltime);
		return (-1);
	}

	if (iac_pd == NULL) {
		if ((iac_pd = malloc(sizeof(*iac_pd))) == NULL) {
			printf("memory allocation failed\n");
			return (-1);
		}
		memset(iac_pd, 0, sizeof(*iac_pd));
		iac_pd->iacpd_ia = ia;
		iac_pd->iacpd_callback = callback;
		iac_pd->iacpd_isvalid = isvalid;
		iac_pd->iacpd_duration = duration;
		iac_pd->iacpd_cleanup = cleanup;
		iac_pd->iacpd_renew_data =
		iac_pd->iacpd_rebind_data =
	    iac_pd->iacpd_release_data =
	    iac_pd->iacpd_reestablish_data = renew_prefix;

		iac_pd->pifc_head = pifc;
		TAILQ_INIT(&iac_pd->siteprefix_head);
		*ctlp = (struct iactl *)iac_pd;
	}

	/* search for the given prefix, and make a new one if it fails */
	if ((sp = find_siteprefix(&iac_pd->siteprefix_head, pinfo, 1)) == NULL) {
		if ((sp = malloc(sizeof(*sp))) == NULL) 
		{
			printf("123 memory allocation failed\n");
			return (-1);
		}
		memset(sp, 0, sizeof(*sp));
		sp->prefix.addr = pinfo->addr;
		sp->prefix.plen = pinfo->plen;
		sp->ctl = iac_pd;
		TAILQ_INIT(&sp->ifprefix_list);

		TAILQ_INSERT_TAIL(&iac_pd->siteprefix_head, sp, link);

		spcreate = 1;
	}

	/* update the timestamp of update */
	sp->updatetime = time(NULL);

	/* update the prefix according to pinfo */
	sp->prefix.pltime = pinfo->pltime;
	sp->prefix.vltime = pinfo->vltime;
	dprintfs(LOG_DEBUG, FNAME, "%s a prefix %s/%d pltime=%lu, vltime=%lu",
	    spcreate ? "create" : "update",
	    in6addr2strs(&pinfo->addr, 0), pinfo->plen,
	    pinfo->pltime, pinfo->vltime);

	/* update prefix interfaces if necessary */
	if (sp->prefix.vltime != 0 && spcreate) {
		for (pif = TAILQ_FIRST(iac_pd->pifc_head); pif;
		    pif = TAILQ_NEXT(pif, link)) {
			/*
			 * The requesting router MUST NOT assign any delegated
			 * prefixes or subnets from the delegated prefix(es) to
			 * the link through which it received the DHCP message
			 * from the delegating router.
			 * [RFC3633 Section 12.1]
			 */
			if (strcmp(pif->ifname, dhcpifp->ifname) == 0) {
				dprintfs(LOG_INFO, FNAME,
				    "skip %s as a prefix interface",
				    dhcpifp->ifname);
				continue;
			}

			add_ifprefix(sp, pinfo, pif);
		}
	}




	/*
	 * If the new vltime is 0, this prefix immediately expires.
	 * Otherwise, set up or update the associated timer.
	 */
	switch (sp->prefix.vltime) {
	case 0:
		remove_siteprefix(sp);
		break;
	case DHCP6_DURATION_INFINITE:
		if (sp->timer)
			dhcp6_remove_timer6s(&sp->timer);
		break;
	default:
		if (sp->timer == NULL) {
			sp->timer = dhcp6_add_timer6s(siteprefix_timo, sp);
			if (sp->timer == NULL) {
				dprintfs(LOG_NOTICE, FNAME,
				    "failed to add prefix timer");
				remove_siteprefix(sp); /* XXX */
				return (-1);
			}
		}
		/* update the timer */
		timo.tv_sec = sp->prefix.vltime;
		timo.tv_usec = 0;
		
		update_radvd_cfgs(&pinfo->addr,pinfo->plen);
		dhcp6_set_timers(&timo, sp->timer);
		break;
	}





	return (0);
}

static struct siteprefix *
find_siteprefix(head, prefix, match_plen)
	struct siteprefix_list *head;
	struct dhcp6_prefix *prefix;
	int match_plen;
{
	struct siteprefix *sp;

	for (sp = TAILQ_FIRST(head); sp; sp = TAILQ_NEXT(sp, link)) {
		if (!IN6_ARE_ADDR_EQUAL(&sp->prefix.addr, &prefix->addr))
			continue;
		if (match_plen == 0 || sp->prefix.plen == prefix->plen)
			return (sp);
	}

	return (NULL);
}

static void
remove_siteprefix(sp)
	struct siteprefix *sp;
{
	struct dhcp6_ifprefix *ip;

	printf("remove a site prefix %s/%d",in6addr2strs(&sp->prefix.addr, 0), sp->prefix.plen);

	if (sp->timer)
		dhcp6_remove_timer6s(&sp->timer);

	while ((ip = TAILQ_FIRST(&sp->ifprefix_list)) != NULL) 
	{
		TAILQ_REMOVE(&sp->ifprefix_list, ip, plink);
		pd_ifaddrconf(IFADDRCONF_REMOVE, ip);
		free(ip);
	}

	TAILQ_REMOVE(&sp->ctl->siteprefix_head, sp, link);
	free(sp);
}

static int
isvalid(iac)
	struct iactl *iac;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;

	if (TAILQ_EMPTY(&iac_pd->siteprefix_head))
		return (0);	/* this IA is invalid */
	return (1);
}

static u_int32_t
duration(iac)
	struct iactl *iac;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;
	struct siteprefix *sp;
	u_int32_t base = DHCP6_DURATION_INFINITE, pltime, passed;
	time_t now;

	/* Determine the smallest period until pltime expires. */
	now = time(NULL);
	for (sp = TAILQ_FIRST(&iac_pd->siteprefix_head); sp;
	    sp = TAILQ_NEXT(sp, link)) {
		passed = now > sp->updatetime ?
		    (u_int32_t)(now - sp->updatetime) : 0;
		pltime = sp->prefix.pltime > passed ?
		    sp->prefix.pltime - passed : 0;

		if (base == DHCP6_DURATION_INFINITE || pltime < base)
			base = pltime;
	}

	return (base);
}

static void
cleanup(iac)
	struct iactl *iac;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;
	struct siteprefix *sp;

	while ((sp = TAILQ_FIRST(&iac_pd->siteprefix_head)) != NULL) {
		TAILQ_REMOVE(&iac_pd->siteprefix_head, sp, link);
		remove_siteprefix(sp);
	}

	free(iac);
}

static int
renew_prefix(iac, iaparam, evdp, evd)
	struct iactl *iac;
	struct dhcp6_ia *iaparam;
	struct dhcp6_eventdata **evdp, *evd;
{
	struct iactl_pd *iac_pd = (struct iactl_pd *)iac;
	struct siteprefix *sp;
	struct dhcp6_list *ial = NULL, pl;

	TAILQ_INIT(&pl);
	for (sp = TAILQ_FIRST(&iac_pd->siteprefix_head); sp;
	    sp = TAILQ_NEXT(sp, link)) {
		if (dhcp6_add_listval6s(&pl, DHCP6_LISTVAL_PREFIX6,
		    &sp->prefix, NULL) == NULL)
			goto fail;
	}

	if ((ial = malloc(sizeof(*ial))) == NULL)
		goto fail;
	TAILQ_INIT(ial);
	if (dhcp6_add_listval6s(ial, DHCP6_LISTVAL_IAPD, iaparam, &pl) == NULL)
		goto fail;
	dhcp6_clear_lists(&pl);

	evd->type = DHCP6_EVDATA_IAPD;
	evd->data = (void *)ial;
	evd->privdata = (void *)evdp;
	evd->destructor = renew_data_free;

	return (0);

  fail:
	dhcp6_clear_lists(&pl);
	if (ial)
		free(ial);
	return (-1);
}

static void
renew_data_free(evd)
	struct dhcp6_eventdata *evd;
{
	struct dhcp6_list *ial;

	if (evd->type != DHCP6_EVDATA_IAPD) {
		dprintfs(LOG_ERR, FNAME, "assumption failure");
		exit(1);
	}

	if (evd->privdata)
		*(struct dhcp6_eventdata **)evd->privdata = NULL;
	ial = (struct dhcp6_list *)evd->data;
	dhcp6_clear_lists(ial);
	free(ial);
}

static struct dhcp6_timer *
siteprefix_timo(arg)
	void *arg;
{
	struct siteprefix *sp = (struct siteprefix *)arg;
	struct ia *ia;
	void (*callback)__P((struct ia *));

	dprintfs(LOG_DEBUG, FNAME, "prefix timeout for %s/%d",
	    in6addr2strs(&sp->prefix.addr, 0), sp->prefix.plen);

	ia = sp->ctl->iacpd_ia;
	callback = sp->ctl->iacpd_callback;

	if (sp->timer)
		dhcp6_remove_timer6s(&sp->timer);

	remove_siteprefix(sp);

	(*callback)(ia);

	return (NULL);
}

static int
add_ifprefix(siteprefix, prefix, pconf)
	struct siteprefix *siteprefix;
	struct dhcp6_prefix *prefix;
	struct prefix_ifconf *pconf;
{
	struct dhcp6_ifprefix *ifpfx = NULL;
	struct in6_addr *a;
	u_long sla_id;
	char *sp;
	int b, i;

	if ((ifpfx = malloc(sizeof(*ifpfx))) == NULL) 
	{
		dprintfs(LOG_NOTICE, FNAME,
		    "failed to allocate memory for ifprefix");
		return (-1);
	}
	memset(ifpfx, 0, sizeof(*ifpfx));

	ifpfx->ifconf = pconf;

	ifpfx->paddr.sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
	ifpfx->paddr.sin6_len = sizeof(struct sockaddr_in6);
#endif
	ifpfx->paddr.sin6_addr = prefix->addr;

	/* brcm: FIXME: Find out how ISP will deploy site local prefix configuration */
	pconf->sla_len = pconf->ifid_len - prefix->plen;

	ifpfx->plen = prefix->plen + pconf->sla_len;
	/*
	 * XXX: our current implementation assumes ifid len is a multiple of 8
	 */
	if ((pconf->ifid_len % 8) != 0) {
		dprintfs(LOG_ERR, FNAME,
		    "assumption failure on the length of interface ID");
		goto bad;
	}
	if (ifpfx->plen + pconf->ifid_len < 0 ||
	    ifpfx->plen + pconf->ifid_len > 128) {
		dprintfs(LOG_INFO, FNAME,
			"invalid prefix length %d + %d + %d",
			prefix->plen, pconf->sla_len, pconf->ifid_len);
		goto bad;
	}

	/* copy prefix and SLA ID */
	a = &ifpfx->paddr.sin6_addr;
	b = prefix->plen;
	for (i = 0, b = prefix->plen; b > 0; b -= 8, i++)
		a->s6_addr[i] = prefix->addr.s6_addr[i];
	sla_id = htonl(pconf->sla_id);
	
	sp = ((char *)&sla_id + 3);
	i = (128 - pconf->ifid_len) / 8;
	for (b = pconf->sla_len; b > 7; b -= 8, sp--)
		a->s6_addr[--i] = *sp;
	if (b)
		a->s6_addr[--i] |= *sp;

	/* configure the corresponding address */
	ifpfx->ifaddr = ifpfx->paddr;
	for (i = 15; i >= pconf->ifid_len / 8; i--)
		ifpfx->ifaddr.sin6_addr.s6_addr[i] = pconf->ifid[i];

	
	if (pd_ifaddrconf(IFADDRCONF_ADD, ifpfx))
		goto bad;

	/* TODO: send a control message for other processes */

	TAILQ_INSERT_TAIL(&siteprefix->ifprefix_list, ifpfx, plink);

	return (0);

  bad:
	if (ifpfx)
		free(ifpfx);
	return (-1);
}

#ifndef ND6_INFINITE_LIFETIME
#define ND6_INFINITE_LIFETIME 0xffffffff
#endif

static int
pd_ifaddrconf(cmd, ifpfx)
	ifaddrconf_cmd_t cmd;
	struct dhcp6_ifprefix *ifpfx;
{   
	struct prefix_ifconf *pconf;
	pconf = ifpfx->ifconf;
	strcpy(dhcp6c_status.lan6_addr,in6addr2strs(&ifpfx->ifaddr.sin6_addr,0));
printf("++++++++in6addr2strs(&ifpfx->ifaddr.sin6_addr,0)=%s\n",in6addr2strs(&ifpfx->ifaddr.sin6_addr,0));
	return (ifaddrconf6s(cmd, pconf->ifname, in6addr2strs(&ifpfx->ifaddr.sin6_addr,0), ifpfx->plen, 
	    ND6_INFINITE_LIFETIME, ND6_INFINITE_LIFETIME));
}
