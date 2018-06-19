/*
 * eCos network interface code
 *
 * Copyright (C) 2010, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: interface.c,v 1.11 2011-01-06 03:08:20 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <proto/ethernet.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmparams.h>
#include <rc.h>
#include <netdb.h>
#include <arpa/inet.h>


struct m_rtmsg {
	struct	rt_msghdr m_rtm;
	char	m_space[512];
};

int
ifscrub(char *name)
{
	int s;
	struct ifreq ifr;

	/* In case name is not given */
	if (name == NULL)
		return -1;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	/* Set interface name */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);

	while (ioctl(s, SIOCGIFADDR, &ifr) == 0) {
		if (ioctl(s, SIOCDIFADDR, &ifr) != 0)
			break;
	}

	close(s);
	return 0;
}

int
ifconfig(char *name, int flags, char *addr, char *netmask)
{
	int s;
	struct ifreq ifr;
	int old_flags;
	struct in_addr in_addr, in_netmask, in_broadaddr;
	struct sockaddr_in *sin;

	/* In case name is not given */
	if (name == NULL)
		return -1;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		//goto err;
		return -1;//tenda modify

	/* Set interface name */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);

	/* Get old interface flags */
	if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0)
		goto err;

	old_flags = ifr.ifr_flags;

	/* Set interface flags */
	ifr.ifr_flags = flags;
	if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
		goto err;
	
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	memset(sin, 0, sizeof(*sin));
	sin->sin_len = sizeof(*sin);
	sin->sin_family = AF_INET;

	/* Set IP netmask and broadcast */
	if (addr && netmask) {
		inet_aton(netmask, &in_netmask);
		sin->sin_addr = in_netmask;
		if (ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
			goto err;

		inet_aton(addr, &in_addr);
		/* Set IP address */
		sin->sin_addr = in_addr;
		if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
			goto err;

		if ((old_flags & IFF_POINTOPOINT) == 0) {
			in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr)
				| ~in_netmask.s_addr;
			sin = (struct sockaddr_in *)&ifr.ifr_broadaddr;
			memset(sin, 0, sizeof(*sin));
			sin->sin_addr = in_broadaddr;
			sin->sin_len = sizeof(*sin);
			sin->sin_family = AF_INET;
			if (ioctl(s, SIOCSIFBRDADDR, &ifr) < 0)
				goto err;
		}
	}
	else if (addr) {
		inet_aton(addr, &in_addr);
		/* Set IP address */
		sin->sin_addr = in_addr;
		if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
			goto err;
	}


	/* Remove all the ip addressses of this interface */
	if ((flags == 0 && addr == NULL && netmask == NULL) ||
	    (addr && strcmp(addr, "0.0.0.0") == 0)) {
		if (memcmp(name, "br", 2) != 0) {
			while (ioctl(s, SIOCGIFADDR, &ifr) == 0) {
				if (ioctl(s, SIOCDIFADDR, &ifr) != 0)
					break;
			}
		}
	}
	close(s);

	return 0;

err:
	close(s);
	return errno;
}

#define SA_SIZE(sa)						\
	((!(sa) || ((struct sockaddr *)(sa))->sa_len == 0) ?	\
	   sizeof(long)	:				\
	   1 + ((((struct sockaddr *)(sa))->sa_len - 1) | (sizeof(long) - 1)))

#define	SIN(a)	((struct sockaddr_in *)a)

/*
 * Write rtm message to route socket, and read the response if the
 * message type is RTM_GET
 */
static int
rtmsg(struct rt_msghdr *rtm)
{
	static int seq;
	int cmd;

	int len;
	int rc = -1;

	/* Create a routing socket */
	int s;

	s = socket(PF_ROUTE, SOCK_RAW, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}

	/* Write to route socket */
	cmd = rtm->rtm_type;
	rtm->rtm_seq = ++seq;

	if ((len = write(s, (char *)rtm, rtm->rtm_msglen)) < 0)
		goto bad;

	/* Read back routing messaeg */
	if (cmd == RTM_GET) {
		do {
			len = read(s, (char *)rtm, sizeof(struct m_rtmsg));
		}
		while (len > 0 && (rtm->rtm_seq != seq));

		if (len < sizeof(*rtm) ||
		    rtm->rtm_msglen > len ||
		    rtm->rtm_version != RTM_VERSION ||
		    rtm->rtm_errno != 0) {
			goto bad;
		}
	}

	rc = 0;
bad:
	close(s);
	return rc;
}

static int
copysa(struct rt_msghdr *rtm, char *cp,
	int w, struct sockaddr *sa)
{
	int l = SA_SIZE(sa);
	rtm->rtm_addrs |= w;
	if (sa)
		memcpy(cp, sa, l);

	return l;
}

/*
 * Transform rtm address info to struct rt_addrinfo for further processing
 */
#define RTROUNDUP(a) \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define ADVANCE(x, n) (x += RTROUNDUP((n)->sa_len))

static int
xaddrs(cp, cplim, rtinfo)
	caddr_t cp, cplim;
	struct rt_addrinfo *rtinfo;
{
	register struct sockaddr *sa;
	register int i;

	bzero(rtinfo->rti_info, sizeof(rtinfo->rti_info));
	for (i = 0; (i < RTAX_MAX) && (cp < cplim); i++) {
		if ((rtinfo->rti_addrs & (1 << i)) == 0)
			continue;

		sa = (struct sockaddr *)cp;
		if ((cp + sa->sa_len) > cplim) {
			return -1;
		}
		rtinfo->rti_info[i] = sa;
		ADVANCE(cp, sa);
	}
	return 0;
}

#define COPYSA(w, sa)	(cp += copysa(rtm, cp, w, sa))

static int
rtrequest(int req,
	struct sockaddr *dst, struct sockaddr *gateway, struct sockaddr *netmask,
	struct sockaddr *ifp, int *flags, int *metric)
{
	struct m_rtmsg m_rtmsg;
	struct rt_msghdr *rtm = &m_rtmsg.m_rtm;
	char *cp = (char *)(rtm + 1);
	int rc;

	memset(&m_rtmsg, 0, sizeof(m_rtmsg));

	/* Do header initialization */
	rtm->rtm_type = req;
	rtm->rtm_version = RTM_VERSION;
	rtm->rtm_flags = RTF_UP;
	if (req == RTM_ADD) {
		if (flags)
			rtm->rtm_flags |= *flags;

		if (metric) {
			if (netmask) {
				rtm->rtm_inits = RTV_HOPCOUNT;
				rtm->rtm_rmx.rmx_hopcount = *metric;
			}
			else {
				rtm->rtm_inits = RTV_EXPIRE;
				rtm->rtm_rmx.rmx_expire = *metric;
			}
		}
	}

	/*
	 * Setup rtm_addrs, the sequence must be
	 *    RTA_DST,
	 *    RTA_GATEWAY,
	 *    RTA_NETMASK,
	 *    RTA_IFP
	 */
	COPYSA(RTA_DST, dst);

	if (req == RTM_GET)
		COPYSA(RTA_GATEWAY, 0);
	else {
		if (gateway)
			COPYSA(RTA_GATEWAY, gateway);
	}

	if (netmask)
		COPYSA(RTA_NETMASK, netmask);

	if (req == RTM_GET && ifp)
		COPYSA(RTA_IFP, 0);

	/* Set the message length */
	rtm->rtm_msglen = cp - (char *)rtm;

	/* Do route message send */
	rc = rtmsg(rtm);
	if (rc == 0 && req == RTM_GET) {
		struct rt_addrinfo info;

		bzero(&info, sizeof(info));
		info.rti_addrs = rtm->rtm_addrs;

		cp = (char *)(rtm + 1);
		if (xaddrs(cp, cp + rtm->rtm_msglen, &info) != 0)
			return -1;

		if (gateway)
			memcpy(gateway, info.rti_info[RTAX_GATEWAY],
				(info.rti_info[RTAX_GATEWAY])->sa_len);

		if (ifp)
			memcpy(ifp, info.rti_info[RTAX_IFP], (info.rti_info[RTAX_IFP])->sa_len);

		if (flags)
			*flags = rtm->rtm_flags;

		if (metric) {
			if (netmask)
				*metric = rtm->rtm_rmx.rmx_hopcount;
			else
				*metric = rtm->rtm_rmx.rmx_expire;
		}
	}

	return rc;
}

/* Get all routing entries */
static char *
sysctl_rtable(int op, int sflag, int *needed)
{
	int mib[6];
	char *buf = 0;
	char *opstr = (op == NET_RT_IFLIST? "iflist" : "route");

	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;			/* protocol */
	mib[3] = AF_INET;
	mib[4] = op;

	if (op == NET_RT_FLAGS)
		mib[5] = sflag;
	else
		mib[5] = 0;

	/* Dump routing entries to buffer */
	if (sysctl(mib, 6, NULL, (size_t *)needed, NULL, 0) < 0) {
		printf("%s-sysctl-estimate failed\n", opstr);
		return 0;
	}

	if ((buf = (char *)malloc(*needed)) == NULL) {
		printf("%s-sysctl-malloc failed\n", opstr);
		return 0;
	}

	if (sysctl(mib, 6, buf, (size_t *)needed, NULL, 0) < 0) {
		printf("%s-sysctl-retrieval failed\n", opstr);

		free(buf);
		return 0;
	}

	return buf;
}

static int
walkrtable(int (*action)(struct rt_msghdr *rtm, struct rt_addrinfo *info, void *arg), void *w)
{
	char *buf;
	int needed;

	char *next, *lim;
	struct rt_msghdr *rtm;
	struct rt_addrinfo info;

	buf = sysctl_rtable(NET_RT_DUMP, 0, &needed);
	if (buf == 0)
		return -1;

	lim = buf + needed;
	for (next = buf; next < lim; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)next;

		bzero(&info, sizeof(info));
		info.rti_addrs = rtm->rtm_addrs;
		if (xaddrs((caddr_t)(rtm+1),
			(caddr_t)(next + rtm->rtm_msglen), &info) != 0) {
			free(buf);
			return -1;
		}

		(*action)(rtm, &info, w);
	}

	free(buf);
	return 0;
}

/* Add route */
static int
rtadd(struct in_addr *dst, struct in_addr *mask, struct in_addr *gw, int metric)
{
	struct sockaddr rtkey = {sizeof(struct sockaddr_in), AF_INET};
	struct sockaddr netmask = {sizeof(struct sockaddr_in), AF_INET};
	struct sockaddr gateway = {sizeof(struct sockaddr_in), AF_INET};
	int flags = (RTF_STATIC | RTF_GATEWAY);
	int rc;

	/* Make sure the dstination is netmasked */
	SIN(&rtkey)->sin_addr.s_addr = (dst->s_addr & mask->s_addr);
	SIN(&netmask)->sin_addr.s_addr = mask->s_addr;
	SIN(&gateway)->sin_addr.s_addr = gw->s_addr;

	rc = rtrequest(RTM_ADD, &rtkey, &gateway, &netmask, 0, &flags, &metric);
	return (rc == 0);
}

/* Delete route */
static int
rtdel(struct in_addr *dst, struct in_addr *mask, struct in_addr *gw, int metric)
{
	struct sockaddr rtkey = {sizeof(struct sockaddr_in), AF_INET};
	struct sockaddr netmask = {sizeof(struct sockaddr_in), AF_INET};
	struct sockaddr gateway = {sizeof(struct sockaddr_in), AF_INET};
	int rc;

	SIN(&rtkey)->sin_addr.s_addr = (dst->s_addr & mask->s_addr);
	SIN(&netmask)->sin_addr.s_addr = mask->s_addr;
	SIN(&gateway)->sin_addr.s_addr = gw->s_addr;

	rc = rtrequest(RTM_DELETE, &rtkey, &gateway, &netmask, 0, 0, &metric);
	return (rc == 0);
}

int
route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	struct in_addr in_ipaddr = {0};
	struct in_addr in_netmask = {0};
	struct in_addr in_gateway = {0};

	inet_aton(dst, &in_ipaddr);
	inet_aton(genmask, &in_netmask);
	inet_aton(gateway, &in_gateway);
	return rtadd(&in_ipaddr, &in_netmask, &in_gateway, metric+1);
}

int
route_del(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	struct in_addr in_ipaddr = {0};
	struct in_addr in_netmask = {0};
	struct in_addr in_gateway = {0};

	inet_aton(dst, &in_ipaddr);
	inet_aton(genmask, &in_netmask);
	inet_aton(gateway, &in_gateway);
	return rtdel(&in_ipaddr, &in_netmask, &in_gateway, metric+1);
}

/*
 * flush route with ifname in routing table
 */
static int
rm_rt(struct rt_msghdr *rtm, struct rt_addrinfo *info, void *arg)
{
	char out[32];
	struct sockaddr_dl *dl;

	if ((info->rti_addrs & RTA_DST) == 0 ||
	    (info->rti_info[RTAX_DST])->sa_family != AF_INET ||
	    (rtm->rtm_flags & RTF_LLINFO) != 0) {
		return 0;
	}

	/* Print if_name */
	if (info->rti_addrs & RTA_IFP) {
		dl = (struct sockaddr_dl *)info->rti_info[RTAX_IFP];
		memcpy(out, (char *)dl->sdl_data, dl->sdl_nlen);
		out[dl->sdl_nlen] = 0;

		/* Delete this entry */
		if (strcmp(out, (char *)arg) == 0) {
			rtm->rtm_type = RTM_DELETE;
			rtmsg(rtm);
		}
	}

	return 0;
}

void
route_flush(char *ifname)
{
	walkrtable(rm_rt, ifname);
}
//roy add
int ifconfig_mtu(char *ifname,int mtu)
{
	int s;
	struct ifreq ifr;

	/* In case name is not given */
	if (ifname == NULL)
		return -1;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	/* Set interface name */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	ifr.ifr_mtu = mtu;
	ioctl(s, SIOCSIFMTU, &ifr);

	close(s);
	
	return 0;
}
