/*	$OpenBSD: ifconfig.c,v 1.220 2009/06/19 14:05:32 henning Exp $	*/
/*	$NetBSD: ifconfig.c,v 1.40 1997/10/01 02:19:43 enami Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1997, 1998, 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <network.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#ifdef CYGPKG_NET_FREEBSD_STACK
#include <net/if_var.h>
#endif
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <arpa/inet.h>
//#include <netinet/ip_ipsp.h>
#include <netinet/if_ether.h>

#include <netdb.h>

#include <ctype.h>
//#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>

#define CONFIG_RTL_MTU_IOCTL_SUPPORT 1

static struct	ifreq		ifr, ridreq;
static struct	in_aliasreq	in_addreq;
static int	flags, setaddr, doalias;
static u_long	metric;

static int	clearaddr;
static int	newaddr = 0;

extern int inet_net_pton(int af, const char *src, void *dst, size_t size);
void	notealias(const char *, int, char *);
void	setifaddr(const char *, int, char *);
void	setifflags(const char *, int, char *);
void	setifbroadaddr(const char *, int, char *);
void	setifmetric(const char *, int, char *);
#if defined(CONFIG_RTL_MTU_IOCTL_SUPPORT)
void setifmtu(const char *val, int ignored, char *ifname);
#endif
void	setifnetmask(const char *, int, char *);

#define A_SILENT	0x8000000	/* doing operation, do not print */

#define	NEXTARG0	0xffffff
#define NEXTARG		0xfffffe
#define	NEXTARG2	0xfffffd

static const struct	cmd {
	char	*c_name;
	int	c_parameter;		/* NEXTARG means next argv */
	int	c_action;		/* defered action */
	void	(*c_func)(const char *, int, char *);
	void	(*c_func2)(const char *, const char *, char *);
} cmds[] = {
	{ "up",		IFF_UP,		0,		setifflags } ,
	{ "down",	-IFF_UP,	0,		setifflags },
	{ "arp",	-IFF_NOARP,	0,		setifflags },
	{ "-arp",	IFF_NOARP,	0,		setifflags },
	{ "debug",	IFF_DEBUG,	0,		setifflags },
	{ "-debug",	-IFF_DEBUG,	0,		setifflags },
	{ "alias",	IFF_UP,		0,		notealias },
	{ "-alias",	-IFF_UP,	0,		notealias },
	{ "delete",	-IFF_UP,	0,		notealias },

	{ "netmask",	NEXTARG,	0,		setifnetmask },
	{ "metric",	NEXTARG,	0,		setifmetric },

	{ "broadcast",	NEXTARG,	0,		setifbroadaddr },
	
	#if defined(CONFIG_RTL_MTU_IOCTL_SUPPORT)
	{ "mtu",	NEXTARG,	0,		setifmtu },
	#endif

	{ NULL, /*src*/	0,		0,		setifaddr },
	{ NULL, /*illegal*/0,		0,		NULL },
};

int	getinfo(struct ifreq *, int);

void	printif(char *, int);
void	printb(char *, unsigned short, char *);
void	status(int, struct sockaddr_dl *, char *);

void	in_status(int, char *);
void	in_getaddr(const char *, int);
void	in_getprefix(const char *, int);

/* Known address families */
static const struct afswtch {
	char *af_name;
	short af_af;
	void (*af_status)(int,  char *);
	void (*af_getaddr)(const char *, int);
	void (*af_getprefix)(const char *, int);
	u_long af_difaddr;
	u_long af_aifaddr;
	caddr_t af_ridreq;
	caddr_t af_addreq;
} afs[] = {
#define C(x) ((caddr_t) &x)
	{ "inet", AF_INET, in_status, in_getaddr, in_getprefix,
	    SIOCDIFADDR, SIOCAIFADDR, C(ridreq), C(in_addreq) },
	{ 0,	0,	    0,		0 }
};

static const struct afswtch *afp;	/*the address family being set or asked about*/

static int ifaliases = 0;

#if defined(CYGPKG_NET_OPENBSD_STACK)
char *ether_ntoa (struct ether_addr *e)
{
	static char a[] = "xx:xx:xx:xx:xx:xx";

	/*
	if (e->ether_addr_octet[0] > 0xFF || e->ether_addr_octet[1] > 0xFF ||
	    e->ether_addr_octet[2] > 0xFF || e->ether_addr_octet[3] > 0xFF ||
	    e->ether_addr_octet[4] > 0xFF || e->ether_addr_octet[5] > 0xFF) {
		errno = EINVAL;
		return (NULL);
	}
	*/
	(void)sprintf(a, "%02x:%02x:%02x:%02x:%02x:%02x",
	    e->ether_addr_octet[0], e->ether_addr_octet[1],
	    e->ether_addr_octet[2], e->ether_addr_octet[3],
	    e->ether_addr_octet[4], e->ether_addr_octet[5]);

	return (a);
}
#elif defined(CYGPKG_NET_FREEBSD_STACK)
char *ether_ntoa(const struct ether_addr *e)
{
	static char a[] = "xx:xx:xx:xx:xx:xx";

	/*
	if (e->ether_addr_octet[0] > 0xFF || e->ether_addr_octet[1] > 0xFF ||
	    e->ether_addr_octet[2] > 0xFF || e->ether_addr_octet[3] > 0xFF ||
	    e->ether_addr_octet[4] > 0xFF || e->ether_addr_octet[5] > 0xFF) {
		errno = EINVAL;
		return (NULL);
	}
	*/
	(void)sprintf(a, "%02x:%02x:%02x:%02x:%02x:%02x",
	    e->octet[0], e->octet[1],
	    e->octet[2], e->octet[3],
	    e->octet[4], e->octet[5]);
	return (a);
}
#endif

int
ifconfig_main(unsigned int argc, unsigned char *argv[])
{
	const struct afswtch *rafp = NULL;
	int actions = 0;			/* Actions performed */
	int i;
	char name[IFNAMSIZ];

	clearaddr = 0;
	newaddr = 0;
	setaddr = 0;
	doalias = 0;
	metric = 0;
	flags = 0;
	afp = 0;
	ifaliases = 0;
	memset(&ifr, 0, sizeof(ifr));
	memset(&ridreq, 0, sizeof(ridreq));
	memset(&in_addreq, 0, sizeof(in_addreq));
	
	/* If no args at all, print all interfaces.  */
	if (argc < 1) {
		printif(NULL, ifaliases);
		return 0;
	}

	strncpy(name, (char *)(*argv), sizeof(name));
	name[IFNAMSIZ-1] = '0';
	if (strlen((char *)(*argv)) >= IFNAMSIZ) {
		printf("interface name '%s' too long", *argv);
		return 0;
	}
	
	argc--, argv++;
	if (argc > 0) {
		for (afp = rafp = afs; rafp->af_name; rafp++)
			if (strcmp(rafp->af_name, (char *)(*argv)) == 0) {
				afp = rafp;
				argc--;
				argv++;
				break;
			}
		rafp = afp;
		ifr.ifr_addr.sa_family = rafp->af_af;
	}

	(void) strcpy(ifr.ifr_name, name);

	while (argc > 0) {
		const struct cmd *p;

		for (p = cmds; p->c_name; p++)
			if (strcmp((char *)(*argv), p->c_name) == 0)
				break;
		if (p->c_name == 0 && setaddr)
			for (i = setaddr; i > 0; i--) {
				p++;
				if (p->c_func == NULL)
					printf("%s: bad value", *argv);
			}
		if (p->c_func || p->c_func2) {
			if (p->c_parameter == NEXTARG0) {
				const struct cmd *p0;
				int noarg = 1;

				if (argv[1]) {
					for (p0 = cmds; p0->c_name; p0++)
						if (strcmp((char *)argv[1], p0->c_name) == 0) {
							noarg = 0;
							break;
						}
				} else
					noarg = 0;

				if (noarg == 0)
					(*p->c_func)(NULL, 0, name);
				else
					goto nextarg;
			} else if (p->c_parameter == NEXTARG) {
nextarg:
				if (argv[1] == NULL)
					printf("'%s' requires argument",
					    p->c_name);
				(*p->c_func)((char *)argv[1], 0, name);
				argc--, argv++;
				actions = actions | A_SILENT | p->c_action;
			} else if (p->c_parameter == NEXTARG2) {
				if ((argv[1] == NULL) ||
				    (argv[2] == NULL))
					printf("'%s' requires 2 arguments",
					    p->c_name);
				(*p->c_func2)((char *)argv[1], (char *)argv[2], name);
				argc -= 2;
				argv += 2;
				actions = actions | A_SILENT | p->c_action;
			} else {
				(*p->c_func)((char *)(*argv), p->c_parameter, name);
				actions = actions | A_SILENT | p->c_action;
			}
		}
		argc--, argv++;
	}

	if (argc == 0 && actions == 0) {
		printif(ifr.ifr_name, 1);
		return 0;
	}

	if (clearaddr) {
		int s;
		
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s < 0) {
			perror("socket");
			return -1;
		}
		(void) strncpy(rafp->af_ridreq, name, sizeof(ifr.ifr_name));
		if (ioctl(s, rafp->af_difaddr, rafp->af_ridreq) < 0) {
			if (errno == EADDRNOTAVAIL && (doalias >= 0)) {
				/* means no previous address for interface */
			} else
				perror("SIOCDIFADDR");
		}
		close(s);
	}

	if (newaddr) {
		int s;
		
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s < 0) {
			perror("socket");
			return -1;
		}
		(void) strncpy(rafp->af_addreq, name, sizeof(ifr.ifr_name));
		if (ioctl(s, rafp->af_aifaddr, rafp->af_addreq) < 0)
			perror("SIOCAIFADDR");
		close(s);
	}

	return 0;
}

int getinfo(struct ifreq *ifr, int create)
{
	int s;
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}
	/*
	// do not check if interface name ends with digit
	if (!isdigit(ifr->ifr_name[strlen(ifr->ifr_name) - 1])) {
		close(s);
		return (-1);	// ignore groups here
	}
	*/
	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)ifr) < 0) {
		if (ioctl(s, SIOCGIFFLAGS, (caddr_t)ifr) < 0) {
			close(s);
			return (-1);
		}
	}
	flags = ifr->ifr_flags;
	if (ioctl(s, SIOCGIFMETRIC, (caddr_t)ifr) < 0)
		metric = 0;
	else
		metric = ifr->ifr_metric;
	close(s);
	return (0);
}


void
printif(char *ifname, int ifaliases)
{
	struct ifaddrs *ifap, *ifa;
	const char *namep;
	char *oname = NULL;
	struct ifreq *ifrp;
	int count = 0, noinet = 1;
	size_t nlen = 0;
	char name[IFNAMSIZ];

	if (ifname) {
		if ((oname = strdup(ifname)) == NULL)
			perror("strdup");
		nlen = strlen(oname);
	}

	if (getifaddrs(&ifap) != 0)
		perror("getifaddrs");

	namep = NULL;
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
#if 0
		if (oname) {
			if (nlen && isdigit(oname[nlen - 1])) {
				/* must have exact match */
				if (strcmp(oname, ifa->ifa_name) != 0)
					continue;
			} else {
				/* partial match OK if it ends w/ digit */
				if (strncmp(oname, ifa->ifa_name, nlen) != 0 ||
				    !isdigit(ifa->ifa_name[nlen]))
					continue;
			}
		}
#else
		// do not check if interface name ends with digit
		if (oname) {
			if (nlen) {
				/* must have exact match */
				if (strcmp(oname, ifa->ifa_name) != 0)
					continue;
			} else {
				/* partial match OK if it ends w/ digit */
				if (strncmp(oname, ifa->ifa_name, nlen) != 0 )
					continue;
			}
		}
#endif
		memset(&ifr, 0, sizeof(ifr));
		//memcpy(&ifr.ifr_addr, ifa->ifa_addr,
		//    MIN(sizeof(ifr.ifr_addr), ifa->ifa_addr->sa_len));
		if (sizeof(ifr.ifr_addr) <= ifa->ifa_addr->sa_len)
			memcpy(&ifr.ifr_addr, ifa->ifa_addr, sizeof(ifr.ifr_addr));
		else
			memcpy(&ifr.ifr_addr, ifa->ifa_addr, ifa->ifa_addr->sa_len);
		ifrp = &ifr;

		strncpy(name, ifa->ifa_name, sizeof(name));
		name[IFNAMSIZ-1] = '0';
		strcpy(ifrp->ifr_name, name);

		if (ifa->ifa_addr->sa_family == AF_LINK) {
			namep = ifa->ifa_name;
			if (getinfo(ifrp, 0) < 0)
				continue;
			status(1, (struct sockaddr_dl *)ifa->ifa_addr, name);
			count++;
			noinet = 1;
			continue;
		}

		if (!namep || !strcmp(namep, ifa->ifa_name)) {
			const struct afswtch *p;

			if (ifa->ifa_addr->sa_family == AF_INET &&
			    ifaliases == 0 && noinet == 0)
				continue;
			if ((p = afp) != NULL) {
				if (ifa->ifa_addr->sa_family == p->af_af)
					(*p->af_status)(1, name);
			} else {
				for (p = afs; p->af_name; p++) {
					if (ifa->ifa_addr->sa_family ==
					    p->af_af)
						(*p->af_status)(0, name);
				}
			}
			count++;
			if (ifa->ifa_addr->sa_family == AF_INET)
				noinet = 0;
			continue;
		}
	}
	freeifaddrs(ifap);
	if (oname != NULL)
		free(oname);
	if (count == 0)
		fprintf(stderr, "%s: no such interface\n", ifname);
}


#define RIDADDR 0
#define ADDR	1
#define MASK	2
#define DSTADDR	3

/*ARGSUSED*/
void
setifaddr(const char *addr, int param, char *ifname)
{
	/*
	 * Delay the ioctl to set the interface addr until flags are all set.
	 * The address interpretation may depend on the flags,
	 * and the flags may change when the address is set.
	 */
	setaddr++;
	if (doalias >= 0)
		newaddr = 1;
	if (doalias == 0)
		clearaddr = 1;
	//(*afp->af_getaddr)(addr, (doalias >= 0 ? ADDR : RIDADDR));
	(*afp->af_getaddr)(addr, ADDR);
}

/* ARGSUSED */
void
setifnetmask(const char *addr, int ignored, char *ifname)
{
	(*afp->af_getaddr)(addr, MASK);
}

/* ARGSUSED */
void
setifbroadaddr(const char *addr, int ignored, char *ifname)
{
	(*afp->af_getaddr)(addr, DSTADDR);
}

#define rqtosa(x) (&(((struct ifreq *)(afp->x))->ifr_addr))
/*ARGSUSED*/
void
notealias(const char *addr, int param, char *ifname)
{
	if (setaddr && doalias == 0 && param < 0)
		memcpy(rqtosa(af_ridreq), rqtosa(af_addreq),
		    rqtosa(af_addreq)->sa_len);
	doalias = param;
	if (param < 0) {
		clearaddr = 1;
		newaddr = 0;
	} else
		clearaddr = 0;
}
/*
 * Note: doing an SIOCGIFFLAGS scribbles on the union portion
 * of the ifreq structure, which may confuse other parts of ifconfig.
 * Make a private copy so we can avoid that.
 */
/* ARGSUSED */
void
setifflags(const char *vname, int value, char *ifname)
{
	struct ifreq my_ifr;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return;
	}
	
	bcopy((char *)&ifr, (char *)&my_ifr, sizeof(struct ifreq));
	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&my_ifr) < 0)
		perror("SIOCGIFFLAGS");
	(void) strcpy(my_ifr.ifr_name, ifname);
	flags = my_ifr.ifr_flags;

	if (value < 0) {
		value = -value;
		flags &= ~value;
	} else
		flags |= value;
	my_ifr.ifr_flags = flags;
	if (ioctl(s, SIOCSIFFLAGS, (caddr_t)&my_ifr) < 0)
		perror("SIOCSIFFLAGS");
	close(s);
}


/* ARGSUSED */
void
setifmetric(const char *val, int ignored, char *ifname)
{
	const char *errmsg = NULL;
	int s;
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return;
	}
	
	(void) strcpy(ifr.ifr_name, ifname);

	//ifr.ifr_metric = strtonum(val, 0, INT_MAX, &errmsg);
	ifr.ifr_metric = atoi(val);
	if (errmsg)
		printf("metric %s: %s", val, errmsg);
	if (ioctl(s, SIOCSIFMETRIC, (caddr_t)&ifr) < 0)
		printf("SIOCSIFMETRIC");
	close(s);
}

#if defined(CONFIG_RTL_MTU_IOCTL_SUPPORT)
void
setifmtu(const char *val, int ignored, char *ifname)
{
	struct ifreq my_ifr;
	const char *errmsg = NULL;
	int s;

	bcopy((char *)&ifr, (char *)&my_ifr, sizeof(struct ifreq));
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return;
	}
	
	(void) strcpy(my_ifr.ifr_name, ifname);

	//ifr.ifr_metric = strtonum(val, 0, INT_MAX, &errmsg);
	my_ifr.ifr_mtu = atoi(val);
	if (errmsg)
		printf("mtu %s: %s", val, errmsg);
	if (ioctl(s, SIOCSIFMTU, (caddr_t)&my_ifr) < 0)
		printf("SIOCSIFMTU");
	close(s);
}
#endif

#define	IFFBITS \
"\020\1UP\2BROADCAST\3DEBUG\4LOOPBACK\5POINTOPOINT\6NOTRAILERS\7RUNNING\10NOARP\
\11PROMISC\12ALLMULTI\13OACTIVE\14SIMPLEX\15LINK0\16LINK1\17LINK2\20MULTICAST"


/*
 * Print the status of the interface.  If an address family was
 * specified, show it and it only; otherwise, show them all.
 */
void
status(int link, struct sockaddr_dl *sdl, char *name)
{
	const struct afswtch *p = afp;

	printf("%s: ", name);
	printb("flags", flags, IFFBITS);

	if (metric)
		printf(" metric %lu", metric);

	putchar('\n');
	if (sdl != NULL && sdl->sdl_alen &&
	    (sdl->sdl_type == IFT_ETHER))
		(void)printf("\tlladdr %s\n", ether_ntoa(
		    (struct ether_addr *)LLADDR(sdl)));

//proto_status:
	if (link == 0) {
		if ((p = afp) != NULL) {
			(*p->af_status)(1, name);
		} else for (p = afs; p->af_name; p++) {
			ifr.ifr_addr.sa_family = p->af_af;
			(*p->af_status)(0, name);
		}
	}

}

/* ARGSUSED */
void
in_status(int force, char *name)
{
	struct sockaddr_in *sin, sin2;
	int s;
	struct	sockaddr_in	netmask;

	netmask.sin_addr.s_addr = 0;	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return;
	}
	
	(void) strcpy(ifr.ifr_name, name);
	sin = (struct sockaddr_in *)&ifr.ifr_addr;

	/*
	 * We keep the interface address and reset it before each
	 * ioctl() so we can get ifaliases information (as opposed
	 * to the primary interface netmask/dstaddr/broadaddr, if
	 * the ifr_addr field is zero).
	 */
	memcpy(&sin2, &ifr.ifr_addr, sizeof(sin2));

	printf("\tinet %s", inet_ntoa(sin->sin_addr));
	(void) strcpy(ifr.ifr_name, name);
	if (ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			printf("SIOCGIFNETMASK");
		memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
	} else
		netmask.sin_addr =
		    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	if (flags & IFF_POINTOPOINT) {
		memcpy(&ifr.ifr_addr, &sin2, sizeof(sin2));
		if (ioctl(s, SIOCGIFDSTADDR, (caddr_t)&ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
			    memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
			else
			    printf("SIOCGIFDSTADDR");
		}
		(void) strcpy(ifr.ifr_name, name);
		sin = (struct sockaddr_in *)&ifr.ifr_dstaddr;
		printf(" --> %s", inet_ntoa(sin->sin_addr));
	}
	printf(" netmask 0x%x", ntohl(netmask.sin_addr.s_addr));
	if (flags & IFF_BROADCAST) {
		memcpy(&ifr.ifr_addr, &sin2, sizeof(sin2));
		if (ioctl(s, SIOCGIFBRDADDR, (caddr_t)&ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
			    memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
			else
			    perror("SIOCGIFBRDADDR");
		}
		(void) strcpy(ifr.ifr_name, name);
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		if (sin->sin_addr.s_addr != 0)
			printf(" broadcast %s", inet_ntoa(sin->sin_addr));
	}
	putchar('\n');
	close(s);
}


#define SIN(x) ((struct sockaddr_in *) &(x))
struct sockaddr_in *sintab[] = {
SIN(ridreq.ifr_addr), SIN(in_addreq.ifra_addr),
SIN(in_addreq.ifra_mask), SIN(in_addreq.ifra_broadaddr)};

void
in_getaddr(const char *s, int which)
{
	struct sockaddr_in *sin = sintab[which], tsin;
	//struct hostent *hp;
	//struct netent *np;
	int bits, l;
	char p[3];

	bzero(&tsin, sizeof(tsin));
	sin->sin_len = sizeof(*sin);
	if (which != MASK)
		sin->sin_family = AF_INET;
	if (which == ADDR && strrchr(s, '/') != NULL &&
	    (bits = inet_net_pton(AF_INET, s, &tsin.sin_addr,
	    sizeof(tsin.sin_addr))) != -1) {
		l = snprintf(p, sizeof(p), "%i", bits);
		if (l >= sizeof(p) || l == -1)
			printf("%i: bad prefixlen", bits);
		in_getprefix(p, MASK);
		memcpy(&sin->sin_addr, &tsin.sin_addr, sizeof(sin->sin_addr));
	} else if (inet_aton(s, &sin->sin_addr) == 0) {
		/*
		if ((hp = gethostbyname(s)))
			memcpy(&sin->sin_addr, hp->h_addr, hp->h_length);
		else if ((np = getnetbyname(s)))
			sin->sin_addr = inet_makeaddr((u_long)np->n_net, (u_long)INADDR_ANY);
		else*/
			printf("%s: bad value", s);
	}
}

/* ARGSUSED */
void
in_getprefix(const char *plen, int which)
{
	struct sockaddr_in *sin = sintab[which];
	const char *errmsg = NULL;
	u_char *cp;
	int len;

	//len = strtonum(plen, 0, 32, &errmsg);
	len = atoi(plen);
	if (errmsg)
		printf("prefix %s: %s", plen, errmsg);

	sin->sin_len = sizeof(*sin);
	if (which != MASK)
		sin->sin_family = AF_INET;
	if ((len == 0) || (len == 32)) {
		memset(&sin->sin_addr, 0xff, sizeof(struct in_addr));
		return;
	}
	memset((void *)&sin->sin_addr, 0x00, sizeof(sin->sin_addr));
	for (cp = (u_char *)&sin->sin_addr; len > 7; len -= 8)
		*cp++ = 0xff;
	if (len)
		*cp = 0xff << (8 - len);
}

/*
 * Print a value a la the %b format of the kernel's printf
 */
void
printb(char *s, unsigned short v, char *bits)
{
	int i, any = 0;
	char c;

	if (bits && *bits == 8)
		printf("%s=%o", s, v);
	else
		printf("%s=%x", s, v);

	if (bits) {
		bits++;
		putchar('<');
		while ((i = *bits++)) {
			if (v & (1 << (i-1))) {
				if (any)
					putchar(',');
				any = 1;
				for (; (c = *bits) > 32; bits++)
					putchar(c);
			} else
				for (; *bits > 32; bits++)
					;
		}
		putchar('>');
	}
}

