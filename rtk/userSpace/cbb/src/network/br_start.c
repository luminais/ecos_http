/*	$OpenBSD: brconfig.c,v 1.6 2000/02/04 06:32:04 deraadt Exp $	*/

/*
 * Copyright (c) 1999, 2000 Jason L. Wright (jason@thought.net)
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Jason L. Wright
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <netinet/if_ether.h>
#include <net/if_bridge.h>
#include <sys/errno.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pi_common.h"
#include <shutils.h>

#include <bcmnvram.h>

void tenda_bridge_usage __P((void));
#ifndef	__ECOS
int main __P((int, char **));
#endif
int tenda_bridge_setflag __P((int, char *, short));
int tenda_bridge_clrflag __P((int, char *, short));
int tenda_bridge_ifsetflag __P((int, char *, char *, u_int32_t));
int tenda_bridge_ifclrflag __P((int, char *, char *, u_int32_t));
int tenda_bridge_list __P((int, char *, char *));
int tenda_bridge_addrs __P((int, char *, char *));
int tenda_bridge_addaddr __P((int, char *, char *, char *));
int tenda_bridge_deladdr __P((int, char *, char *));
int tenda_bridge_maxaddr __P((int, char *, char *));
int tenda_bridge_timeout __P((int, char *, char *));
int tenda_bridge_flush __P((int, char *));
int tenda_bridge_flushall __P((int, char *));
int tenda_bridge_add __P((int, char *, char *));
int tenda_bridge_delete __P((int, char *, char *));
int tenda_bridge_status __P((int, char *));
int tenda_is_bridge __P((int, char *));
int tenda_bridge_show_all __P((int));
#if 1 //add by z10312 临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
void printb __P((char *, unsigned short, char *));
#endif
int tenda_bridge_rule __P((int, char *, int, char **, int));
int tenda_bridge_rules __P((int, char *, char *, char *));
int tenda_bridge_flush_rule __P((int, char *, char *));
#if 0 //add by z10312 临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
void bridge_badrule __P((int, char **, int));
#endif
void tenda_bridge_showrule __P((struct ifbrlreq *, char *));
int bridge_rulefile __P((int, char *, char *));

/* if_flags bits: borrowed from ifconfig.c */
#define	IFFBITS \
"\020\1UP\2BROADCAST\3DEBUG\4LOOPBACK\5POINTOPOINT\6NOTRAILERS\7RUNNING\10NOARP\
\11PROMISC\12ALLMULTI\13OACTIVE\14SIMPLEX\15LINK0\16LINK1\17LINK2\20MULTICAST"

#define	IFBAFBITS	"\020\1STATIC"
#define	IFBIFBITS	"\020\1LEARNING\2DISCOVER\3BLOCKNONIP\4BLOCKARP"

#define warnx(fmt, arg...)		printf(fmt, ##arg)
#define err(x, y) assert(0);


void
tenda_bridge_usage()
{
	printf("usage: brconfig -a\n");
	printf("usage: brconfig interface [up] [down]"
		" [add if] [del if] ...\n");
}

int
tenda_bridge_ifsetflag(s, brdg, ifsname, flag)
	int s;
	char *brdg, *ifsname;
	u_int32_t flag;
{
	struct ifbreq req;

	strncpy(req.ifbr_name, brdg, sizeof(req.ifbr_name));
	strncpy(req.ifbr_ifsname, ifsname, sizeof(req.ifbr_ifsname));
	if (ioctl(s, SIOCBRDGGIFFLGS, (caddr_t)&req) < 0) {
		return (1);
	}

	req.ifbr_ifsflags |= flag;

	if (ioctl(s, SIOCBRDGSIFFLGS, (caddr_t)&req) < 0) {
		warnx("%s: set if flags %s error %d\n", brdg, ifsname, errno);
		return (1);
	}
	return (0);
}

int
tenda_bridge_ifclrflag(s, brdg, ifsname, flag)
	int s;
	char *brdg, *ifsname;
	u_int32_t flag;
{
	struct ifbreq req;

	strncpy(req.ifbr_name, brdg, sizeof(req.ifbr_name));
	strncpy(req.ifbr_ifsname, ifsname, sizeof(req.ifbr_ifsname));

	if (ioctl(s, SIOCBRDGGIFFLGS, (caddr_t)&req) < 0) {
		warnx("%s: get if flags %s error %d\n", brdg, ifsname, errno);
		return (1);
	}

	req.ifbr_ifsflags &= ~flag;

	if (ioctl(s, SIOCBRDGSIFFLGS, (caddr_t)&req) < 0) {
		warnx("%s: set if flags %s error %d\n", brdg, ifsname, errno);
		return (1);
	}
	return (0);
}

int
tenda_bridge_show_all(s)
	int s;
{
	char *inbuf = NULL;
	struct ifconf ifc;
	struct ifreq *ifrp, ifreq;
	int len = 8192, i;

	while (1) {
		ifc.ifc_len = len;
		ifc.ifc_buf = inbuf = realloc(inbuf, len);
		if (inbuf == NULL)
			err(1, "malloc");
		if (ioctl(s, SIOCGIFCONF, &ifc) < 0)
			err(1, "ioctl(SIOCGIFCONF)");
		if (ifc.ifc_len + sizeof(struct ifreq) < len)
			break;
		len *= 2;
	}
	ifrp = ifc.ifc_req;
	ifreq.ifr_name[0] = '\0';
	for (i = 0; i < ifc.ifc_len;) {
		ifrp = (struct ifreq *)((caddr_t)ifc.ifc_req + i);
		i += sizeof(ifrp->ifr_name) +
		    (ifrp->ifr_addr.sa_len > sizeof(struct sockaddr) ?
		    ifrp->ifr_addr.sa_len : sizeof(struct sockaddr));
		if (ifrp->ifr_addr.sa_family != AF_LINK)
			continue;
		if (!tenda_is_bridge(s, ifrp->ifr_name))
			continue;
		tenda_bridge_status(s, ifrp->ifr_name);
	}
	free(ifc.ifc_buf);
	return (0);
}

int
tenda_bridge_setflag(s, brdg, f)
	int s;
	char *brdg;
	short f;
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, brdg, sizeof(ifr.ifr_name));

	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
		warnx("%s: get flags error %d\n", brdg, errno);
		return (1);
	}

	ifr.ifr_flags |= f;

	if (ioctl(s, SIOCSIFFLAGS, (caddr_t)&ifr) < 0) {
		warnx("%s: set flags error %d\n", brdg, errno);
		return (1);
	}

	return (0);
}

int
tenda_bridge_clrflag(s, brdg, f)
	int s;
	char *brdg;
	short f;
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, brdg, sizeof(ifr.ifr_name));

	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
		warnx("%s: get flags error %d\n", brdg, errno);
		return (1);
	}

	ifr.ifr_flags &= ~(f);

	if (ioctl(s, SIOCSIFFLAGS, (caddr_t)&ifr) < 0) {
		warnx("%s: set flags error %d\n", brdg, errno);
		if (errno == EPERM)
			return (1);
		return (1);
	}

	return (0);
}

int
tenda_bridge_flushall(s, brdg)
	int s;
	char *brdg;
{
	struct ifbreq req;

	strncpy(req.ifbr_name, brdg, sizeof(req.ifbr_name));
	req.ifbr_ifsflags = IFBF_FLUSHALL;
	if (ioctl(s, SIOCBRDGFLUSH, &req) < 0) {
		warnx("%s: flush all addr cache error %d\n", brdg, errno);
		return (1);
	}
	return (0);
}

int
tenda_bridge_flush(s, brdg)
	int s;
	char *brdg;
{
	struct ifbreq req;

	strncpy(req.ifbr_name, brdg, sizeof(req.ifbr_name));
	req.ifbr_ifsflags = IFBF_FLUSHDYN;
	if (ioctl(s, SIOCBRDGFLUSH, &req) < 0) {
		warnx("%s: flush addr cache error %d\n", brdg, errno);
		return (1);
	}
	return (0);
}

int
tenda_bridge_list(s, brdg, delim)
	int s;
	char *brdg, *delim;
{
	struct ifbreq *reqp;
	struct ifbifconf bifc;
	int i, len = 8192;
	char buf[sizeof(reqp->ifbr_ifsname) + 1] = {0}, *inbuf = NULL;

	while (1) {
		strncpy(bifc.ifbic_name, brdg, sizeof(bifc.ifbic_name));
		bifc.ifbic_len = len;
		bifc.ifbic_buf = inbuf = realloc(inbuf, len);
		if (inbuf == NULL)
			err(1, "malloc");
		if (ioctl(s, SIOCBRDGIFS, &bifc) < 0)
			err(1, brdg);
		if (bifc.ifbic_len + sizeof(*reqp) < len)
			break;
		len *= 2;
	}
	for (i = 0; i < bifc.ifbic_len / sizeof(*reqp); i++) {
		reqp = bifc.ifbic_req + i;
		strncpy(buf, reqp->ifbr_ifsname, sizeof(buf));
		printf("%s%s ", delim, buf);
		#if 0 //add by z10312临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
		printb("flags", reqp->ifbr_ifsflags, IFBIFBITS);
		#endif
		printf("\n");
		tenda_bridge_rules(s, brdg, buf, delim);
	}
	free(bifc.ifbic_buf);
	return (0);             /* NOTREACHED */
}

int
tenda_bridge_add(s, brdg, ifn)
	int s;
	char *brdg, *ifn;
{
	char errno_string[8];
	struct ifbreq req;

	strncpy(req.ifbr_name, brdg, sizeof(req.ifbr_name));
	strncpy(req.ifbr_ifsname, ifn, sizeof(req.ifbr_ifsname));
	if (ioctl(s, SIOCBRDGADD, &req) < 0) {
		snprintf(errno_string, sizeof(errno_string), "%d", errno);
		warnx("%s: add if %s error (%s)\n", brdg, ifn,
			(errno == EEXIST) ? "exist" : errno_string);
		return (1);
	}
	return (0);
}

int
tenda_bridge_delete(s, brdg, ifn)
	int s;
	char *brdg, *ifn;
{
	char errno_string[8];
	struct ifbreq req;

	strncpy(req.ifbr_name, brdg, sizeof(req.ifbr_name));
	strncpy(req.ifbr_ifsname, ifn, sizeof(req.ifbr_ifsname));
	if (ioctl(s, SIOCBRDGDEL, &req) < 0) {
		snprintf(errno_string, sizeof(errno_string), "%d", errno);
		warnx("%s: del if %s error (%s)\n", brdg, ifn,
			(errno == ENOENT) ? " not exist" : errno_string);
		return (1);
	}
	return (0);
}

int
tenda_bridge_timeout(s, brdg, arg)
	int s;
	char *brdg, *arg;
{
	struct ifbcachetoreq ifbct;
	u_int32_t newtime;
	char *endptr;

	newtime = strtoul(arg, &endptr, 0);
	if (arg[0] == '\0' || endptr[0] != '\0') {
		printf("invalid arg for timeout: %s\n", arg);
		return (1);
	}

	strncpy(ifbct.ifbct_name, brdg, sizeof(ifbct.ifbct_name));
	ifbct.ifbct_time = newtime;
	if (ioctl(s, SIOCBRDGSTO, (caddr_t)&ifbct) < 0) {
		warnx("%s: set cache timeout error %d\n", brdg, errno);
		return (1);
	}
	return (0);
}

int
tenda_bridge_maxaddr(s, brdg, arg)
	int s;
	char *brdg, *arg;
{
	struct ifbcachereq ifbc;
	u_int32_t newsize;
	char *endptr;

	newsize = strtoul(arg, &endptr, 0);
	if (arg[0] == '\0' || endptr[0] != '\0') {
		printf("invalid arg for maxaddr: %s\n", arg);
		return (1);
	}

	strncpy(ifbc.ifbc_name, brdg, sizeof(ifbc.ifbc_name));
	ifbc.ifbc_size = newsize;
	if (ioctl(s, SIOCBRDGSCACHE, (caddr_t)&ifbc) < 0) {
		warnx("%s: set cache size error %d\n", brdg, errno);
		return (1);
	}
	return (0);
}

int
tenda_bridge_deladdr(s, brdg, addr)
	int s;
	char *brdg, *addr;
{
	struct ifbareq ifba;
	struct ether_addr *ea;

	strncpy(ifba.ifba_name, brdg, sizeof(ifba.ifba_name));
	ea = ether_aton(addr);
	if (ea == NULL) {
		warnx("%s: Invalid address: %s\n", brdg, addr);
		return (1);
	}
	bcopy(ea, &ifba.ifba_dst, sizeof(struct ether_addr));

	if (ioctl(s, SIOCBRDGDADDR, &ifba) < 0) {
		warnx("%s: del address %s error %d\n", brdg, addr, errno);
		return (1);
	}

	return (0);
}

int
tenda_bridge_addaddr(s, brdg, ifname, addr)
	int s;
	char *brdg, *ifname, *addr;
{
	struct ifbareq ifba;
	struct ether_addr *ea;

	strncpy(ifba.ifba_name, brdg, sizeof(ifba.ifba_name));
	strncpy(ifba.ifba_ifsname, ifname, sizeof(ifba.ifba_ifsname));

	ea = ether_aton(addr);
	if (ea == NULL) {
		warnx("%s: Invalid address: %s\n", brdg, addr);
		return (1);
	}
	bcopy(ea, &ifba.ifba_dst, sizeof(struct ether_addr));
	ifba.ifba_flags = IFBAF_STATIC;

	if (ioctl(s, SIOCBRDGSADDR, &ifba) < 0) {
		warnx("%s: set address flags %s error %d\n", brdg, addr, errno);
		return (1);
	}

	return (0);
}

int
tenda_bridge_addrs(s, brdg, delim)
	int s;
	char *brdg, *delim;
{
	struct ifbaconf ifbac;
	struct ifbareq *ifba;
	char *inbuf = NULL, buf[sizeof(ifba->ifba_ifsname) + 1];
	int i, len = 8192;

	while (1) {
		strncpy(ifbac.ifbac_name, brdg, sizeof(ifbac.ifbac_name));
		ifbac.ifbac_len = len;
		ifbac.ifbac_buf = inbuf = realloc(inbuf, len);
		//strncpy(ifbac.ifbac_name, brdg, sizeof(ifbac.ifbac_name));
		if (inbuf == NULL)
			err(1, "malloc");
		if (ioctl(s, SIOCBRDGRTS, &ifbac) < 0) {
			if (errno == ENETDOWN)
				return (0);
			err(1, brdg);
		}
		if (ifbac.ifbac_len + sizeof(*ifba) < len)
			break;
		len *= 2;
	}

	for (i = 0; i < ifbac.ifbac_len / sizeof(*ifba); i++) {
		ifba = ifbac.ifbac_req + i;
		strncpy(buf, ifba->ifba_ifsname, sizeof(buf));
		printf("%s%s %s %d ", delim, ether_ntoa(&ifba->ifba_dst),
		    buf, ifba->ifba_age);
		#if 0 //add by z10312 临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
		printb("flags", ifba->ifba_flags, IFBAFBITS);
		#endif 
		
		printf("\n");
	}
	free(ifbac.ifbac_buf);
	return (0);
}

/*
 * Check to make sure 'brdg' is really a bridge interface.
 */
int
tenda_is_bridge(s, brdg)
	int s;
	char *brdg;
{
	struct ifreq ifr;
	struct ifbaconf ifbac;

	strncpy(ifr.ifr_name, brdg, sizeof(ifr.ifr_name));

	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0)
		return (0);

	ifbac.ifbac_len = 0;
	strncpy(ifbac.ifbac_name, brdg, sizeof(ifbac.ifbac_name));
	if (ioctl(s, SIOCBRDGRTS, (caddr_t)&ifbac) < 0) {
		if (errno == ENETDOWN)
			return (1);
		return (0);
	}
	return (1);
}

int
tenda_bridge_status(s, brdg)
	int s;
	char *brdg;
{
	struct ifreq ifr;
	struct ifbcachereq ifbc;
	struct ifbcachetoreq ifbct;
	int err;

	strncpy(ifr.ifr_name, brdg, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
		warnx("%s: get ifnet flags error %d\n", brdg, errno);
		return (1);
	}

	printf("%s: ", brdg);
	#if 0 //add by z10312 临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
	printb("flags", ifr.ifr_flags, IFFBITS);
	#endif 
	printf("\n");

	printf("Interfaces:\n");
	err = tenda_bridge_list(s, brdg, "  ");
	if (err)
		return (err);

	strncpy(ifbc.ifbc_name, brdg, sizeof(ifbc.ifbc_name));
	if (ioctl(s, SIOCBRDGGCACHE, (caddr_t)&ifbc) < 0) {
		warnx("%s: get cache size error %d\n", brdg, errno);
		return (1);
	}

	strncpy(ifbct.ifbct_name, brdg, sizeof(ifbct.ifbct_name));
	if (ioctl(s, SIOCBRDGGTO, (caddr_t)&ifbct) < 0) {
		warnx("%s: get cache timeout error %d\n", brdg, errno);
		return (1);
	}

	printf("Addresses (max cache: %d, timeout: %d):\n",
	    ifbc.ifbc_size, ifbct.ifbct_time);

	err = tenda_bridge_addrs(s, brdg, "   ");
	return (err);
}

int
tenda_bridge_flush_rule(s, brdg, ifname)
	int s;
	char *brdg, *ifname;
{
	struct ifbrlreq req;

	strncpy(req.ifbr_name, brdg, sizeof(req.ifbr_name));
	strncpy(req.ifbr_ifsname, ifname, sizeof(req.ifbr_ifsname));
	if (ioctl(s, SIOCBRDGFRL, &req) < 0) {
		warnx("%s: flush rules %s error %d\n", brdg, ifname, errno);
		return (1);
	}
	return (0);
}

int
tenda_bridge_rules(s, brdg, ifname, delim)
	int s;
	char *brdg, *ifname;
	char *delim;
{
	char *inbuf = NULL;
	struct ifbrlconf ifc;
	struct ifbrlreq *ifrp, ifreq;
	int len = 8192, i;

	while (1) {
		strncpy(ifc.ifbrl_name, brdg, sizeof(ifc.ifbrl_name));
		strncpy(ifc.ifbrl_ifsname, ifname, sizeof(ifc.ifbrl_ifsname));
		ifc.ifbrl_len = len;
		ifc.ifbrl_buf = inbuf = realloc(inbuf, len);
		//strncpy(ifc.ifbrl_name, brdg, sizeof(ifc.ifbrl_name));
		//strncpy(ifc.ifbrl_ifsname, ifname, sizeof(ifc.ifbrl_ifsname));
		if (inbuf == NULL)
			err(1, "malloc");
		if (ioctl(s, SIOCBRDGGRL, &ifc) < 0)
			err(1, "ioctl(SIOCBRDGGRL)");
		if (ifc.ifbrl_len + sizeof(ifreq) < len)
			break;
		len *= 2;
	}
	ifrp = ifc.ifbrl_req;
	for (i = 0; i < ifc.ifbrl_len; i += sizeof(ifreq)) {
		ifrp = (struct ifbrlreq *)((caddr_t)ifc.ifbrl_req + i);
		tenda_bridge_showrule(ifrp, delim);
	}
	free(ifc.ifbrl_buf);
	return (0);
}

void
tenda_bridge_showrule(r, delim)
	struct ifbrlreq *r;
	char *delim;
{
	if (delim)
		printf("%s    ", delim);
	else
		printf("%s: ", r->ifbr_name);

	if (r->ifbr_action == BRL_ACTION_BLOCK)
		printf("block ");
	else if (r->ifbr_action == BRL_ACTION_PASS)
		printf("pass ");
	else
		printf("[neither block nor pass?]\n");

	if ((r->ifbr_flags & (BRL_FLAG_IN | BRL_FLAG_OUT)) ==
	    (BRL_FLAG_IN | BRL_FLAG_OUT))
		printf("in/out ");
	else if (r->ifbr_flags & BRL_FLAG_IN)
		printf("in ");
	else if (r->ifbr_flags & BRL_FLAG_OUT)
		printf("out ");
	else
		printf("[neither in nor out?]\n");

	printf("on %s", r->ifbr_ifsname);

	if (r->ifbr_flags & BRL_FLAG_SRCVALID)
		printf(" src %s", ether_ntoa(&r->ifbr_src));
	if (r->ifbr_flags & BRL_FLAG_DSTVALID)
		printf(" dst %s", ether_ntoa(&r->ifbr_dst));

	printf("\n");
}

/*
 * Parse a rule definition and send it upwards.
 *
 * Syntax:
 *	{block|pass} {in|out|in/out} on {ifs} [src {mac}] [dst {mac}]
 */
int
tenda_bridge_rule(int s, char *brdg, int targc, char **targv, int ln)
{
	char **argv = targv;
	int argc = targc;
	struct ifbrlreq rule;
	struct ether_addr *ea, *dea;

	if (argc == 0) {
		printf("invalid rule\n");
		return (1);
	}
	rule.ifbr_flags = 0;
	rule.ifbr_action = 0;
	strncpy(rule.ifbr_name, brdg, sizeof(rule.ifbr_name));

	if (strcmp(argv[0], "block") == 0)
		rule.ifbr_action = BRL_ACTION_BLOCK;
	else if (strcmp(argv[0], "pass") == 0)
		rule.ifbr_action = BRL_ACTION_PASS;
	else
		goto bad_rule;
	argc--;	argv++;

	if (argc == 0) {
		#if 0 //add by z10312 临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
		bridge_badrule(targc, targv, ln);
		#endif
		return (1);
	}
	if (strcmp(argv[0], "in") == 0)
		rule.ifbr_flags |= BRL_FLAG_IN;
	else if (strcmp(argv[0], "out") == 0)
		rule.ifbr_flags |= BRL_FLAG_OUT;
	else if (strcmp(argv[0], "in/out") == 0)
		rule.ifbr_flags |= BRL_FLAG_IN | BRL_FLAG_OUT;
	else
		goto bad_rule;
	argc--; argv++;

	if (argc == 0 || strcmp(argv[0], "on"))
		goto bad_rule;
	argc--; argv++;

	if (argc == 0)
		goto bad_rule;
	strncpy(rule.ifbr_ifsname, argv[0], sizeof(rule.ifbr_ifsname));
	argc--; argv++;

	while (argc) {
		if (strcmp(argv[0], "dst") == 0) {
			if (rule.ifbr_flags & BRL_FLAG_DSTVALID)
				goto bad_rule;
			rule.ifbr_flags |= BRL_FLAG_DSTVALID;
			dea = &rule.ifbr_dst;
		}
		else if (strcmp(argv[0], "src") == 0) {
			if (rule.ifbr_flags & BRL_FLAG_SRCVALID)
				goto bad_rule;
			rule.ifbr_flags |= BRL_FLAG_SRCVALID;
			dea = &rule.ifbr_src;
		}
		else
			goto bad_rule;

		argc--; argv++;

		if (argc == 0)
			goto bad_rule;
		ea = ether_aton(argv[0]);
		if (ea == NULL) {
			warnx("%s: Invalid address: %s", brdg, argv[0]);
			return (1);
		}
		bcopy(ea, dea, sizeof(*dea));
		argc--; argv++;
	}

	if (ioctl(s, SIOCBRDGARL, &rule) < 0) {
		warnx("%s: add rule error %d\n", brdg, errno);
		return (1);
	}
	return (0);

bad_rule:
	#if 0 //add by z10312 临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
	bridge_badrule(targc, targv, ln);
	#endif
	return (1);
}

#define MAXRULEWORDS 8
#ifdef ZAP
int
bridge_rulefile(s, brdg, fname)
	int s;
	char *brdg, *fname;
{
	FILE *f;
	char *str, *argv[MAXRULEWORDS], buf[1024], xbuf[1024];
	int ln = 1, argc = 0, err = 0, xerr;

	f = fopen(fname, "r");
	if (f == NULL) {
		warnx("%s: fopen error\n", fname);
		return (1);
	}

	while (1) {
		fgets(buf, sizeof(buf), f);
		if (feof(f))
			break;
		ln++;
		if (buf[0] == '#')
			continue;

		argc = 0;
		str = strtok(buf, "\n\r ");
		strncpy(xbuf, buf, sizeof(xbuf));
		while (str != NULL) {
			argv[argc++] = str;
			if (argc > MAXRULEWORDS) {
				printf("invalid rule: %d: %s\n", ln, xbuf);
				break;
			}
			str = strtok(NULL, "\n\r ");
		}

		if (argc > MAXRULEWORDS)
			continue;

		xerr = tenda_bridge_rule(s, brdg, argc, argv, ln);
		if (xerr)
			err = xerr;
	}
	fclose(f);
	return (err);
}
#endif	/* ZAP */

/*
 * Print a value ala the %b format of the kernel's printf
 * (borrowed from ifconfig.c)
 */

#ifdef	__ECOS
/*****************************************************************************
 函 数 名  : get_br_ifname
 输入参数  : int unit      
             char *ifname  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static char first_init = 1;//只有初始化br的时候才会从nvram里面获取桥接口的ifname
char* get_br_ifname(int unit)
{
	PI8 *lan_ifname = NULL;
	PI8 tmp[PI_BUFLEN_128] = {0};
	static char br_ifname [MAX_BRIDGE_NUM][PI_BUFLEN_16];
	//只有初始化br的时候才会从nvram里面获取桥接口的ifname
	if(first_init)
	{
		if(unit == 0)
		{
			lan_ifname = nvram_safe_get(BR_NAME);
		}
		else
		{
			snprintf(tmp, sizeof(tmp), BRS_NAME,unit);
			lan_ifname = nvram_safe_get(tmp);
		}

		if(NULL == lan_ifname || 0 == strcmp(lan_ifname,""))
		{
			printf("++++get br ifname error++++   %s [%d]\n", __FUNCTION__, __LINE__);
			return NULL;
		}

		strcpy(br_ifname[unit],lan_ifname);
	}
	
	return br_ifname[unit];
}
/*****************************************************************************
 函 数 名  : br_init
 功能描述  : 初始化桥接口，只是up起来
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO br_init()
{
	PIU32 i = 0;
	char *lan_ifname = NULL;
	first_init = 1;
	for (i = 0; i < MAX_BRIDGE_NUM; i++)
	{
		lan_ifname = get_br_ifname(i);
		if(NULL == lan_ifname)
			continue;

		/*拉起桥接口，此时不需要设置桥MAC地址*/
		if(0 == tenda_ifconfig(lan_ifname, IFUP,NULL, NULL))
		{
			PI_PRINTF(MAIN,"br(%s) init ok!\n",lan_ifname);
		}
		else
		{
			PI_ERROR(MAIN,"br(%s) init fail!\n",lan_ifname);			
		}
	}
	first_init = 0;
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : get_br_ifnames
 功能描述  : 获取添加在桥上的接口
 输入参数  : int unit       
             char *ifnames  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO get_br_ifnames(int unit,char *ifnames)
{
	PI8 *lan_ifname = NULL;
	PI8 tmp[PI_BUFLEN_128] = {0};
	if(unit == 0)
	{
		lan_ifname = nvram_safe_get(BR_NAMES);
	}
	else
	{
		snprintf(tmp, sizeof(tmp), BRS_NAMES, unit);
		lan_ifname = nvram_safe_get(tmp);
	}

	if(NULL == lan_ifname || 0 == strcmp(lan_ifname,""))
		return RET_ERR;

	strcpy(ifnames,lan_ifname);
	return RET_SUC;
}
int get_br_index_by_interface(char* ifname)
{
	PIU32 i = 0;
	char idge = 0;
	char name[80] = {0};
	char *next = NULL;
	char ifnames[128] = {0};

	
	if(strstr(ifname,"br"))
	{
		for(i = 0;i < strlen(ifname);i++)
		{
			idge = *(ifname + i);
			if(idge >= '0' && idge <= '9')
				return idge - '0';
		}
		return 0;
	}
	
	for (i = 0; i < MAX_BRIDGE_NUM; i++)
	{
		get_br_ifnames(i,ifnames);
		foreach(name, ifnames, next)
		{
			if(0 == strncmp(name,ifname,strlen(ifname)))
			{
				return i;
			}
		
		}
	}
	
	return MAX_BRIDGE_NUM + 1;
}
#ifdef __CONFIG_IPTV__
extern int rtl_portNameConvertNetifName(unsigned char *portName, unsigned char *netifName);
#endif
unsigned int get_br_ifnames_index(char *ifname)
{
	unsigned int ret = 0;
	unsigned int mask = 0;
	char index = 0;
	char name[80] = {0};
	char *next = NULL;
	char ifnames[128] = {0};
#ifdef __CONFIG_IPTV__
	unsigned char netifName[16] = {0};
	char *iptv_portname = NULL;

	iptv_portname = nvram_get(ADVANCE_IPTV_PORT);
	if(!iptv_portname)
	{
		rtl_portNameConvertNetifName("lan1",netifName);
	}else
	{
		rtl_portNameConvertNetifName(iptv_portname,netifName);
	}
	
#endif
	
	index = get_br_index_by_interface(ifname);

	if(index > MAX_BRIDGE_NUM)
	{
		printf("=find=index=fail=%s [%d]\n", __FUNCTION__, __LINE__);
		return 0;
	}
	
	get_br_ifnames(index,ifnames);
	
	foreach(name, ifnames, next)
	{
#ifdef __CONFIG_IPTV__
		if(nvram_match(ADVANCE_IPTV_ENABLE,"1") 
			&& nvram_match(SYSCONFIG_WORKMODE,"route"))
	    	{
	           	if(0 == strcmp(netifName,name))
				continue;
	     	}
#endif
		mask = if_nametoindex(name);
		ret |=(1<<mask);
	}
	return ret;

}


/*****************************************************************************
 函 数 名  : br_interfaces
 功能描述  : 添加接口到桥上
 输入参数  : int unit          
             BR_TYPE type      
             BR_ACTION action  
             PI8 *ifname       
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static RET_INFO br_interfaces(int unit,BR_TYPE type,BR_ACTION action,PI8 *ifname)
{
	char name[80] = {0};
	char *next = NULL;
	unsigned char mac_addr[6];
	PI8 lan_ifnames[128] = {0};

	if(type == BR_WIRED)
	{
		ether_atoe(nvram_safe_get("et0macaddr"), mac_addr);	
	}

	if(action != BR_DOWN && action != BR_UP)
	{
		return RET_ERR;
	}
	
	
	if(RET_ERR == get_br_ifnames(unit,lan_ifnames))
		return RET_ERR;
	
	foreach(name, lan_ifnames, next)
	{
		if(0 == strncmp(name,ifname,strlen(ifname)))
		{
			if(BR_DOWN == action)
			{
				if(type != BR_WIRLESS)//无线接口先从桥里面移除，再关闭无线接口
					tenda_ifconfig(name, 0, NULL, NULL);
				cyg_thread_delay(10);

				br_del_if(unit, name);
			}
			else if(BR_UP == action)
			{
				 if(type == BR_WIRED)
				{	
					tenda_ifconfig(name, IFUP, NULL, NULL);
					cyg_thread_delay(10);
					tenda_set_mac_address(name, mac_addr);
				}					
				
				if (br_add_if(unit, name))
				{
					PI_PRINTF(MAIN,"interface(%s) add br(%d) fail!\n",name,unit);	
				}
			}
			return RET_SUC;
		
		}
	}
	
	return RET_ERR;
}
/*****************************************************************************
 函 数 名  : br_interfaces_action
 功能描述  : 对外提供的添加接口到桥上的接口
 输入参数  : BR_TYPE type      
             BR_ACTION action  
             PI8 *ifname       
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO br_interfaces_action(BR_TYPE type,BR_ACTION action,PI8 *ifname)
{
	PIU32 i = 0;
	
	for (i = 0; i < MAX_BRIDGE_NUM; i++)
	{
		if(RET_SUC == br_interfaces(i,type,action,ifname))
		{
			break;
		}
	}
	
	return RET_SUC;
}
/*lq add end*/

int
br_add_if(int index, char *ifname)
{
	int sock, ret;
	char ifn[IFNAMSIZ];

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return -1;
	sprintf(ifn, "bridge%d", index);
	ret = tenda_bridge_add(sock, ifn, ifname);
	close(sock);
	return ret;
}

int
br_del_if(int index, char *ifname)
{
	int sock, ret;
	char ifn[IFNAMSIZ];

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return -1;
	sprintf(ifn, "bridge%d", index);
	ret = tenda_bridge_delete(sock, ifn, ifname);
	close(sock);
	return ret;
}

#endif	/* __ECOS */



