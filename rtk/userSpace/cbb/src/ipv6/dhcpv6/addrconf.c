/*	$KAME: addrconf.c,v 1.8 2005/09/16 11:30:13 suz Exp $	*/

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
#include <netinet6/nd6.h>
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

TAILQ_HEAD(statefuladdr_list, statefuladdr);
struct iactl_na {
	struct iactl common;
	struct statefuladdr_list statefuladdr_head;
};
#define iacna_ia common.iactl_ia
#define iacna_callback common.callback
#define iacna_isvalid common.isvalid
#define iacna_duration common.duration
#define iacna_renew_data common.renew_data
#define iacna_rebind_data common.rebind_data
#define iacna_reestablish_data common.reestablish_data
#define iacna_release_data common.release_data
#define iacna_cleanup common.cleanup

struct statefuladdr {
	TAILQ_ENTRY (statefuladdr) link;

	struct dhcp6_statefuladdr addr;
	time_t updatetime;
	struct dhcp6_timer *timer;
	struct iactl_na *ctl;
	struct dhcp6_if *dhcpif;
};

static struct statefuladdr *find_addr __P((struct statefuladdr_list *,
    struct dhcp6_statefuladdr *));
static int remove_addr __P((struct statefuladdr *));
static int isvalid_addr __P((struct iactl *));
static u_int32_t duration_addr __P((struct iactl *));
static void cleanup_addr __P((struct iactl *));
static int renew_addr __P((struct iactl *, struct dhcp6_ia *,
    struct dhcp6_eventdata **, struct dhcp6_eventdata *));
static void na_renew_data_free __P((struct dhcp6_eventdata *));

static struct dhcp6_timer *addr_timo __P((void *));

static int na_ifaddrconf __P((ifaddrconf_cmd_t, struct statefuladdr *));

extern struct dhcp6_timer *client6_timo6s __P((void *));


int
update_address6(ia, addr, dhcpifp, ctlp, callback)
	struct ia *ia;
	struct dhcp6_statefuladdr *addr;
	struct dhcp6_if *dhcpifp;
	struct iactl **ctlp;
	void (*callback)__P((struct ia *));
{
	struct iactl_na *iac_na = (struct iactl_na *)*ctlp;
	struct statefuladdr *sa;
	int sacreate = 0;
	struct timeval timo;

	/*
	 * A client discards any addresses for which the preferred
         * lifetime is greater than the valid lifetime.
	 * [RFC3315 22.6] 
	 */
	if (addr->vltime != DHCP6_DURATION_INFINITE &&
	    (addr->pltime == DHCP6_DURATION_INFINITE ||
	    addr->pltime > addr->vltime)) {
		dprintfs(LOG_INFO, FNAME, "invalid address %s: "
		    "pltime (%lu) is larger than vltime (%lu)",
		    in6addr2strs(&addr->addr, 0),
		    addr->pltime, addr->vltime);
		return (-1);
	}

	if (iac_na == NULL) {
		if ((iac_na = malloc(sizeof(*iac_na))) == NULL) {
			dprintfs(LOG_NOTICE, FNAME, "memory allocation failed");
			return (-1);
		}
		memset(iac_na, 0, sizeof(*iac_na));
		iac_na->iacna_ia = ia;
		iac_na->iacna_callback = callback;
		iac_na->iacna_isvalid = isvalid_addr;
		iac_na->iacna_duration = duration_addr;
		iac_na->iacna_cleanup = cleanup_addr;
		iac_na->iacna_renew_data =
		    iac_na->iacna_rebind_data =
		    iac_na->iacna_release_data =
		    iac_na->iacna_reestablish_data = renew_addr;

		TAILQ_INIT(&iac_na->statefuladdr_head);
		*ctlp = (struct iactl *)iac_na;
	}

	/* search for the given address, and make a new one if it fails */
	if ((sa = find_addr(&iac_na->statefuladdr_head, addr)) == NULL) {
		if ((sa = malloc(sizeof(*sa))) == NULL) {
			dprintfs(LOG_NOTICE, FNAME, "memory allocation failed");
			return (-1);
		}
		memset(sa, 0, sizeof(*sa));
		sa->addr.addr = addr->addr;
		sa->ctl = iac_na;
		TAILQ_INSERT_TAIL(&iac_na->statefuladdr_head, sa, link);
		sacreate = 1;
	}

	/* update the timestamp of update */
	sa->updatetime = time(NULL);

	/* update the prefix according to addr */
	sa->addr.pltime = addr->pltime;
	sa->addr.vltime = addr->vltime;
	sa->dhcpif = dhcpifp;
	printf("%s an address %s pltime=%lu, vltime=%lu",
	    sacreate ? "create" : "update",
	    in6addr2strs(&addr->addr, 0), addr->pltime, addr->vltime);

	if (sa->addr.vltime != 0)
	{
		printf("will na_ifaddrconf \n");
		if (na_ifaddrconf(IFADDRCONF_ADD, sa) < 0)
		{
			return (-1);
		}
	}
	/*
	 * If the new vltime is 0, this address immediately expires.
	 * Otherwise, set up or update the associated timer.
	 */
	switch (sa->addr.vltime) {
	case 0:
		if (remove_addr(sa) < 0)
			return (-1);
		break;
	case DHCP6_DURATION_INFINITE:
		if (sa->timer)
			dhcp6_remove_timer6s(&sa->timer);
		break;
	default:
		if (sa->timer == NULL) {
			sa->timer = dhcp6_add_timer6s(addr_timo, sa);
			if (sa->timer == NULL) {
				dprintfs(LOG_NOTICE, FNAME,
				    "failed to add stateful addr timer");
				remove_addr(sa); /* XXX */
				return (-1);
			}
		}
		/* update the timer */
		timo.tv_sec = sa->addr.vltime;
		timo.tv_usec = 0;

		dhcp6_set_timers(&timo, sa->timer);
		break;
	}

	return (0);
}

static struct statefuladdr *
find_addr(head, addr)
	struct statefuladdr_list *head;
	struct dhcp6_statefuladdr *addr;
{
	struct statefuladdr *sa;

	for (sa = TAILQ_FIRST(head); sa; sa = TAILQ_NEXT(sa, link)) {
		if (!IN6_ARE_ADDR_EQUAL(&sa->addr.addr, &addr->addr))
			continue;
		return (sa);
	}

	return (NULL);
}

static int
remove_addr(sa)
	struct statefuladdr *sa;
{
	int ret;

	dprintfs(LOG_DEBUG, FNAME, "remove an address %s",
	    in6addr2strs(&sa->addr.addr, 0));

	if (sa->timer)
		dhcp6_remove_timer6s(&sa->timer);

	TAILQ_REMOVE(&sa->ctl->statefuladdr_head, sa, link);
	ret = na_ifaddrconf(IFADDRCONF_REMOVE, sa);
	free(sa);

	return (ret);
}

static int
isvalid_addr(iac)
	struct iactl *iac;
{
	struct iactl_na *iac_na = (struct iactl_na *)iac;

	if (TAILQ_EMPTY(&iac_na->statefuladdr_head))
		return (0);	/* this IA is invalid */
	return (1);
}

static u_int32_t
duration_addr(iac)
	struct iactl *iac;
{
	struct iactl_na *iac_na = (struct iactl_na *)iac;
	struct statefuladdr *sa;
	u_int32_t base = DHCP6_DURATION_INFINITE, pltime, passed;
	time_t now;

	/* Determine the smallest period until pltime expires. */
	now = time(NULL);
	for (sa = TAILQ_FIRST(&iac_na->statefuladdr_head); sa;
	    sa = TAILQ_NEXT(sa, link)) {
		passed = now > sa->updatetime ?
		    (u_int32_t)(now - sa->updatetime) : 0;
		pltime = sa->addr.pltime > passed ?
		    sa->addr.pltime - passed : 0;

		if (base == DHCP6_DURATION_INFINITE || pltime < base)
			base = pltime;
	}

	return (base);
}

static void
cleanup_addr(iac)
	struct iactl *iac;
{
	struct iactl_na *iac_na = (struct iactl_na *)iac;
	struct statefuladdr *sa;

	while ((sa = TAILQ_FIRST(&iac_na->statefuladdr_head)) != NULL) {
		TAILQ_REMOVE(&iac_na->statefuladdr_head, sa, link);
		remove_addr(sa);
	}

	free(iac);
}

static int
renew_addr(iac, iaparam, evdp, evd)
	struct iactl *iac;
	struct dhcp6_ia *iaparam;
	struct dhcp6_eventdata **evdp, *evd;
{
	struct iactl_na *iac_na = (struct iactl_na *)iac;
	struct statefuladdr *sa;
	struct dhcp6_list *ial = NULL, pl;

	TAILQ_INIT(&pl);
	for (sa = TAILQ_FIRST(&iac_na->statefuladdr_head); sa;
	    sa = TAILQ_NEXT(sa, link)) {
		if (dhcp6_add_listval6s(&pl, DHCP6_LISTVAL_STATEFULADDR6,
		    &sa->addr, NULL) == NULL)
			goto fail;
	}

	if ((ial = malloc(sizeof(*ial))) == NULL)
		goto fail;
	TAILQ_INIT(ial);
	if (dhcp6_add_listval6s(ial, DHCP6_LISTVAL_IANA, iaparam, &pl) == NULL)
		goto fail;
	dhcp6_clear_lists(&pl);

	evd->type = DHCP6_EVDATA_IANA;
	evd->data = (void *)ial;
	evd->privdata = (void *)evdp;
	evd->destructor = na_renew_data_free;

	return (0);

  fail:
	dhcp6_clear_lists(&pl);
	if (ial)
		free(ial);
	return (-1);
}

static void
na_renew_data_free(evd)
	struct dhcp6_eventdata *evd;
{
	struct dhcp6_list *ial;

	if (evd->type != DHCP6_EVDATA_IANA) {
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
addr_timo(arg)
	void *arg;
{
	struct statefuladdr *sa = (struct statefuladdr *)arg;
	struct ia *ia;
	void (*callback)__P((struct ia *));

	dprintfs(LOG_DEBUG, FNAME, "address timeout for %s",
	    in6addr2strs(&sa->addr.addr, 0));

	ia = sa->ctl->iacna_ia;
	callback = sa->ctl->iacna_callback;

	if (sa->timer)
		dhcp6_remove_timer6s(&sa->timer);

	remove_addr(sa);

	(*callback)(ia);

	return (NULL);
}

static void v6len2mask(maskp, len)
	struct in6_addr *maskp;
	int len;
{
	u_char maskarray[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	int bytelen, bitlen, i;

	bzero(maskp, sizeof(*maskp));
	bytelen = len / 8;
	bitlen = len % 8;
	for (i = 0; i < bytelen; i++)
		maskp->s6_addr[i] = 0xff;
	if (bitlen)
		maskp->s6_addr[bytelen] = maskarray[bitlen - 1];
}

int ifaddrconf6s(int mode, char *ifname, char * address, int plen,int pltime,int vltime) 
{
    int s =-1 ;
    struct in6_aliasreq in6_addr;
    int retcode = false;
    struct in6_addr mask;
    char *ptr=NULL;

    s = socket(AF_INET6, SOCK_DGRAM, 0);
    if (s < 0) 
	{
        perror("socket IPv6");
        goto out;
    }

    if (ptr=strchr(address, '/')) 
	{
		*ptr = NULL;
		plen = atoi(++ptr);
    }

    bzero(&in6_addr, sizeof(in6_addr));
    in6_addr.ifra_addr.sin6_len = sizeof(struct sockaddr_in6);
    in6_addr.ifra_addr.sin6_family = AF_INET6;
    if (!inet_pton(AF_INET6, address, (char *)&in6_addr.ifra_addr.sin6_addr)) 
	{
        printf("Can't set IPv6 address: %s\n", address);
        goto out;
    }
    in6_addr.ifra_prefixmask.sin6_len = sizeof(struct sockaddr_in6);
    in6_addr.ifra_prefixmask.sin6_family = AF_INET6;
    if (plen) {
	v6len2mask(&mask, plen);
        bcopy(&mask, &in6_addr.ifra_prefixmask.sin6_addr, sizeof(in6_addr.
ifra_prefixmask.sin6_addr));
    }
    strcpy(in6_addr.ifra_name, ifname);
    
    in6_addr.ifra_lifetime.ia6t_vltime = vltime;
    in6_addr.ifra_lifetime.ia6t_pltime = pltime;
    if (mode == IFADDRCONF_ADD) {
	if (ioctl(s, SIOCAIFADDR_IN6, &in6_addr)) 
	{
    	    perror("SIOCAIFADDR_IN6");
    	    goto out;
	}
    }
    else if (mode == IFADDRCONF_REMOVE) {
	if (ioctl(s, SIOCDIFADDR_IN6, &in6_addr)) {
    	    perror("SIOCDIFADDR_IN6");
    	    goto out;
	}
    }
    else
    	    perror("ifv6config() err");
    retcode = true;
 out:
    if (s != -1) 
      close(s);
    return retcode;
}

static int
na_ifaddrconf(cmd, sa)
	ifaddrconf_cmd_t cmd;
	struct statefuladdr *sa;
{
	struct dhcp6_statefuladdr *addr;
	struct sockaddr_in6 sin6;

	addr = &sa->addr;
	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
	sin6.sin6_len = sizeof(sin6);
#endif
	sin6.sin6_addr = addr->addr;

#if 0 /* brcm: TODO: should be done in CMS */
	return (ifaddrconf(cmd, sa->dhcpif->ifname, &sin6, 64,
	    addr->pltime, addr->vltime));
#else
	printf("sa->dhcpif->ifname = %s  **********addr2strs((struct sockaddr *)(&sin6))=%s \n",sa->dhcpif->ifname,addr2str((struct sockaddr *)(&sin6)));
	return( ifaddrconf6s(cmd,sa->dhcpif->ifname,addr2str((struct sockaddr *)(&sin6)),64,addr->pltime, addr->vltime));
#endif
}
