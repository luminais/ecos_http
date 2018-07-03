/*
 * Compatible to BSD getifaddrs.c
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ifaddrs.c,v 1.1.1.1 2010-04-09 10:37:02 Exp $
 *
 */
/*
 * Copyright (c) 1995, 1999
 *	Berkeley Software Design, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY Berkeley Software Design, Inc. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Berkeley Software Design, Inc. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	BSDI getifaddrs.c,v 2.12 2000/02/23 14:51:59 dab Exp
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#undef _KERNEL
#undef __INSIDE_NET
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/sysctl.h>
#include <net/route.h>
#include <ifaddrs.h>

#define ROUNDUP(a) \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define ADVANCE(x, n) (x += ROUNDUP((n)->sa_len))

int
getifaddrs(struct ifaddrs **pif)
{
	int mib[6];
	char *buf = 0;
	int needed;

	char *next, *lim;
	struct if_msghdr *ifm;
	struct ifa_msghdr *ifam;

	struct ifaddrs *ifaddrs = 0;
	struct ifaddrs *ifa;
	char *data;
	char *name;
	int flags;
	int ifcount = 0;
	int len;

	/* Dump routing entries to buffer */
	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;			/* protocol */
	mib[3] = AF_INET;
	mib[4] = NET_RT_IFLIST;
	mib[5] = 0;

	if (sysctl(mib, 6, NULL, (unsigned int *)&needed, NULL, 0) < 0)
		return -1;

	if ((buf = (char *)malloc(needed)) == NULL)
		return -1;

	if (sysctl(mib, 6, buf, (unsigned int *)&needed, NULL, 0) < 0) {
		free(buf);
		return -1;
	}

	/* Count interfaces */
	lim = buf + needed;
	next = buf;
	while (next < lim) {
		/* Get if_msghdr */
		ifm = (struct if_msghdr *)next;
		if (ifm->ifm_type != RTM_IFINFO)
			goto bad;

		ifcount++;
		next += ifm->ifm_msglen;

		/* Get ifa_msghdr */
		while (next < lim) {
			ifam = (struct ifa_msghdr *)next;
			if (ifam->ifam_type != RTM_NEWADDR)
				break;
			ifcount++;
			next += ifam->ifam_msglen;
		}
	}

	if (ifcount == 0)
		goto bad;

	/* Allocate buffer for ifaddrs */
	len = needed + ifcount * (sizeof(struct ifaddrs) + IFNAMSIZ);
	ifaddrs = (struct ifaddrs *)malloc(len);
	if (ifaddrs == 0)
		goto bad;

	memset(ifaddrs, 0, len);
	data = (char *)(ifaddrs + ifcount);
	name = data + needed;

	/* Copy buf to data, make the pif include the interface data */
	memcpy(data, buf, needed);

	/* Setup each entry */
	lim = data + needed;
	next = data;

	ifa = ifaddrs;
	while (next < lim) {
		struct sockaddr_dl *sdl;

		/* Get if_msghdr */
		ifm = (struct if_msghdr *)next;
		if (ifm->ifm_type != RTM_IFINFO)
			goto bad;

		/* name and flags */
		sdl = (struct sockaddr_dl *)(ifm + 1);
		strncpy(name, sdl->sdl_data, sdl->sdl_nlen);
		name[sdl->sdl_nlen] = '\0';

		flags = ifm->ifm_flags;

		/* Copy data */
		ifa->ifa_next = ifa+1;
		ifa->ifa_name = name;
		ifa->ifa_flags = flags;
		ifa->ifa_addr = (struct sockaddr *)sdl;
		ifa->ifa_data = &ifm->ifm_data;

		ifa++;
		next += ifm->ifm_msglen;

		/* Get ifa_msghdr */
		while (next < lim) {
			char *cp, *cplim;
			struct sockaddr *sa;
			int i;

			ifam = (struct ifa_msghdr *)next;
			if (ifam->ifam_type != RTM_NEWADDR)
				break;

			/* Set common part */
			ifa->ifa_next = ifa+1;
			ifa->ifa_name = name;
			ifa->ifa_flags = flags;

			/* Set struct sockaddr part */
			cp = (char *)(ifam + 1);
			cplim = (char *)ifam + ifam->ifam_msglen;
			for (i = 0; (i < RTAX_MAX) && (cp < cplim); i++) {
				if ((ifam->ifam_addrs & (1 << i)) == 0)
					continue;

				sa = (struct sockaddr *)cp;
				if ((cp + sa->sa_len) > cplim)
					goto bad;

				switch (i)
				{
				case RTAX_IFA:
					ifa->ifa_addr = sa;
					break;
				case RTAX_NETMASK:
					ifa->ifa_netmask = sa;
					break;
				case RTAX_BRD:
					ifa->ifa_dstaddr = sa;
					break;
				default:
					break;
				}

				ADVANCE(cp, sa);
			}

			ifa++;
			next += ifam->ifam_msglen;
		}

		/* Advanced name */
		name += IFNAMSIZ;
	}

	(ifa-1)->ifa_next = 0;

	free(buf);
	*pif = ifaddrs;
	return 0;

bad:
	if (buf)
		free(buf);

	if (ifaddrs)
		free(ifaddrs);

	return -1;
}

void
freeifaddrs(struct ifaddrs *ifp)
{
	free(ifp);
}


/*
 * From RFC 2553:
 *
 * 4.1 Name-to-Index
 *
 *
 *    The first function maps an interface name into its corresponding
 *    index.
 *
 *       #include <net/if.h>
 *
 *       unsigned int  if_nametoindex(const char *ifname);
 *
 *    If the specified interface name does not exist, the return value is
 *    0, and errno is set to ENXIO.  If there was a system error (such as
 *    running out of memory), the return value is 0 and errno is set to the
 *    proper value (e.g., ENOMEM).
 */

unsigned int
if_nametoindex(const char *ifname)
{
	struct ifaddrs *ifaddrs, *ifa;
	unsigned int ni;

	if (getifaddrs(&ifaddrs) < 0)
		return (0);

	ni = 0;

	for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr &&
		    ifa->ifa_addr->sa_family == AF_LINK &&
		    strcmp(ifa->ifa_name, ifname) == 0) {
			ni = ((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index;
			break;
		}
	}

	freeifaddrs(ifaddrs);
	if (!ni)
		errno = ENXIO;
	return (ni);
}

/*
 * From RFC 2533:
 *
 * The second function maps an interface index into its corresponding
 * name.
 *
 *    #include <net/if.h>
 *
 *    char  *if_indextoname(unsigned int ifindex, char *ifname);
 *
 * The ifname argument must point to a buffer of at least IF_NAMESIZE
 * bytes into which the interface name corresponding to the specified
 * index is returned.  (IF_NAMESIZE is also defined in <net/if.h> and
 * its value includes a terminating null byte at the end of the
 * interface name.) This pointer is also the return value of the
 * function.  If there is no interface corresponding to the specified
 * index, NULL is returned, and errno is set to ENXIO, if there was a
 * system error (such as running out of memory), if_indextoname returns
 * NULL and errno would be set to the proper value (e.g., ENOMEM).
 */

char *
if_indextoname(unsigned int ifindex, char *ifname)
{
	struct ifaddrs *ifaddrs, *ifa;
	int error = 0;

	if (getifaddrs(&ifaddrs) < 0)
		return(NULL);	/* getifaddrs properly set errno */

	for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr &&
		    ifa->ifa_addr->sa_family == AF_LINK &&
		    ifindex == ((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index)
		break;
	}

	if (ifa == NULL) {
		error = ENXIO;
		ifname = NULL;
	}
	else
		strncpy(ifname, ifa->ifa_name, IFNAMSIZ);

	freeifaddrs(ifaddrs);

	errno = error;
	return (ifname);
}
