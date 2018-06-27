/*
 * et command.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: etu_ecos.c,v 1.3 2010-05-25 06:54:08 Exp $
 */

#include <stdio.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <typedefs.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <etioctl.h>
#include <proto/ethernet.h>

#include "pi_common.h"

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;


char buf[16 * 1024];

#define VECLEN		2

static void
usage(void)
{
	fprintf(stderr, "usage: et [ [ -a | -i ] interface ] and one of:\n"
		"\tup\n"
		"\tdown\n"
		"\tloop <0 or 1>\n"
		"\tdump\n"
		"\tmsglevel <bitvec> (error=1, trace=2, prhdr=4, prpkt=8)\n"
		"\tpromisc <0 or 1>\n"
		"\tqos <0 or 1>\n"
		"\ttxdown\n"
		"\tspeed <auto, 10half, 10full, 100half, 100full, 1000full>\n"
		"\tphyrd [<phyaddr>] <reg>\n"
		"\tphywr [<phyaddr>] <reg> <val>\n"
		"\trobord <page> <reg>\n"
		"\trobowr <page> <reg> <val>\n"
		);
}

static int
et_check(char *ifname)
{
	if (!strncmp(ifname, "et", 2))
		return (0);
	else if (!strncmp(ifname, "bcm57", 5))
		return (0);

	return (-1);
}

static int
et_find(char *etName)
{
	char *inbuf = NULL;
	struct ifconf ifc;
	struct ifreq *ifrp, ifreq;
	int i, s, len = 8192;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	while (1) {
		ifc.ifc_len = len;
		ifc.ifc_buf = inbuf = realloc(inbuf, len);
		if (inbuf == NULL) {
			fprintf(stderr, "malloc failed\n");
		}
		if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {
			fprintf(stderr, "ioctl (SIOCGIFCONF) failed\n");
		}
		if (ifc.ifc_len + sizeof(struct ifreq) < len)
			break;
		len *= 2;
	}

	ifrp = ifc.ifc_req;
	ifreq.ifr_name[0] = '\0';
	for (i = 0; i < ifc.ifc_len; ) {
		ifrp = (struct ifreq *)((caddr_t)ifc.ifc_req + i);
		i += sizeof(ifrp->ifr_name) +
		    (ifrp->ifr_addr.sa_len > sizeof(struct sockaddr) ?
		    ifrp->ifr_addr.sa_len : sizeof(struct sockaddr));
		if (ifrp->ifr_addr.sa_family != AF_LINK)
			continue;
		if (!et_check(ifrp->ifr_name))
			break;
		ifrp->ifr_name[0] = '\0';
	}

	close(s);

	if (!*ifrp->ifr_name) {
		free(ifc.ifc_buf);
		return -1;
	}

	strcpy(etName, ifrp->ifr_name);
	free(ifc.ifc_buf);

	return 0;
}


int
et(int ac, char *av[])
{
	char *interface = NULL;
	struct ifreq ifr;
	char *endptr;
	int arg, ret = 0;
	int vecarg[VECLEN];
	int s = -1;
	static int optind;

	if (ac == 1) {
		usage();
		goto out;
	}

	ac--;
	av++;

	optind = 0;

	if (av[0][0] == '-') {
		if ((av[0][1] != 'a') && (av[0][1] != 'i')) {
			usage();
			goto out;
		}
		if (ac < 3) {
			usage();
			goto out;
		}
		interface = av[1];
		optind += 2;
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("open socket failed\n");
		goto out;
	}

	if (interface)
		strncpy(ifr.ifr_name, interface, sizeof (ifr.ifr_name));
	else
		ret = et_find(ifr.ifr_name);

	if (ret != 0 || !*ifr.ifr_name) {
		printf("et interface not found\n");
		goto out;
	}

	if (strcmp(av[optind], "up") == 0) {
		if (ioctl(s, SIOCSETCUP, (caddr_t)&ifr) < 0) {
			printf("Error: etcup\n");
			goto out;
		}
	} else if (strcmp(av[optind], "down") == 0) {
		if (ioctl(s, SIOCSETCDOWN, (caddr_t)&ifr) < 0) {
			printf("Error: etcdown\n");
			goto out;
		}
	} else if (strcmp(av[optind], "loop") == 0) {
		if (optind >= (ac -1)) {
			usage();
			goto out;
		}
		arg = atoi(av[optind + 1]);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCLOOP, (caddr_t)&ifr) < 0) {
			printf("Error: etcloop");
			goto out;
		}
	} else if (strcmp(av[optind], "dump") == 0) {
		ifr.ifr_data = buf;
		if (ioctl(s, SIOCGETCDUMP, (caddr_t)&ifr) < 0) {
			printf("Error: etcdump");
			goto out;
		}
		printf("%s\n", buf);
	} else if (strcmp(av[optind], "msglevel") == 0) {
		if (optind >= (ac -1)) {
			usage();
			goto out;
		}
		arg = strtol(av[optind + 1], &endptr, 0);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCSETMSGLEVEL, (caddr_t)&ifr) < 0) {
			printf("Error: etcsetmsglevel");
			goto out;
		}
	} else if (strcmp(av[optind], "promisc") == 0) {
		if (optind >= (ac -1)) {
			usage();
			goto out;
		}
		arg = atoi(av[optind + 1]);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCPROMISC, (caddr_t)&ifr) < 0) {
			printf("Error: etcpromisc");
			goto out;
		}
	} else if (strcmp(av[optind], "qos") == 0) {
		if (optind >= (ac -1)) {
			usage();
			goto out;
		}
		arg = atoi(av[optind + 1]);
		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCQOS, (caddr_t)&ifr) < 0) {
			printf("Error: etcqos");
			goto out;
		}
	} else if (strcmp(av[optind], "speed") == 0) {
		if (optind >= (ac -1)) {
			usage();
			goto out;
		}
		if (strcmp(av[optind+1], "auto") == 0)
			arg = -1;
		else if (strcmp(av[optind+1], "10half") == 0)
			arg = 0;
		else if (strcmp(av[optind+1], "10full") == 0)
			arg = 1;
		else if (strcmp(av[optind+1], "100half") == 0)
			arg = 2;
		else if (strcmp(av[optind+1], "100full") == 0)
			arg = 3;
		else if (strcmp(av[optind+1], "1000full") == 0)
			arg = 5;
		else {
			usage();
			goto out;
		}

		ifr.ifr_data = (caddr_t) &arg;
		if (ioctl(s, SIOCSETCSPEED, (caddr_t)&ifr) < 0) {
			printf("Error: etcspeed");
			goto out;
		}
	}
	else if (strcmp(av[optind], "phyrd") == 0) {
		int cmd = -1;

		if ((ac < (optind + 2)) || (ac > (optind + 3))) {
			usage();
			goto out;
		} else if (ac == (optind + 3)) {
			/* PHY address provided */
			vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
			vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;
			cmd = SIOCGETCPHYRD2;
		} else {
			/* "My" PHY address implied */
			vecarg[0] = strtoul(av[optind + 1], NULL, 0);
			cmd = SIOCGETCPHYRD;
		}
		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, cmd, (caddr_t)&ifr) < 0) {
			printf("Error: etcphyrd");
			goto out;
		}

		printf("0x%04x\n", vecarg[1]);
	} else if (strcmp(av[optind], "phywr") == 0) {
		int cmd = -1;

		if ((ac < (optind + 3)) || (ac > (optind + 4))) {
			usage();
			goto out;
		} else if (ac == (optind + 4)) {
			vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
			vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;
			vecarg[1] = strtoul(av[optind + 3], NULL, 0);
			cmd = SIOCSETCPHYWR2;
		} else {
			vecarg[0] = strtoul(av[optind + 1], NULL, 0);
			vecarg[1] = strtoul(av[optind + 2], NULL, 0);
			cmd = SIOCSETCPHYWR;
		}
		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, cmd, (caddr_t)&ifr) < 0) {
			printf("Error: etcphywr");
			goto out;
		}
	} else if (strcmp(av[optind], "robord") == 0) {
		if (ac != (optind + 3)) {
			usage();
			goto out;
		}

		vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
		vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;

		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, SIOCGETCROBORD, (caddr_t)&ifr) < 0) {
			printf("Error: etcrobord");
			goto out;
		}

		printf("0x%04x\n", vecarg[1]);
	} else if (strcmp(av[optind], "robowr") == 0) {
		if (ac != (optind + 4)) {
			usage();
			goto out;
		}

		vecarg[0] = strtoul(av[optind + 1], NULL, 0) << 16;;
		vecarg[0] |= strtoul(av[optind + 2], NULL, 0) & 0xffff;
		vecarg[1] = strtoul(av[optind + 3], NULL, 0);

		ifr.ifr_data = (caddr_t) vecarg;
		if (ioctl(s, SIOCSETCROBOWR, (caddr_t)&ifr) < 0) {
			printf("Error: etcrobowr");
			goto out;
		}
	} else {
		usage();
		goto out;
	}

out:
	if (s >= 0)
		close(s);

	return (0);
}


et_cmd_ifconfig(int argc, char *argv[])
{
	char *ifname = NULL;
	char *address = NULL;
	char *netmask = "255.255.255.0";
	int flags = IFF_UP;
	
	/* Skip program name */
	--argc;
	++argv;
	if (*argv) {
		/* Parse argument */
		for (; *argv; ++argv) {
			if (ifname == NULL) {
				ifname = "wlan0";
			}
			if (!strcmp(*argv, "0")) {
				flags = 0;
			}

			else if (!strcmp(*argv, "1")) {
				flags = 1;
			}
			else {
				goto usage;
			}
		}
	}
	
	/* Config ipaddress */
	if (!ifname)
		goto usage;
	
	//add by z10312 支持cli ifconfig interface down相关接口,以便于bsp调试 16-0125
	if (!flags) 
		return tenda_ifconfig(ifname, flags, 0, 0);

	
	return tenda_ifconfig(ifname, flags, address, netmask); 
usage:
	printf("Usage: et [0/1]");
	return -1;
}


/*eth phyreset*/

int et_restart_lan_port(int lan_port)
{
	int ret = 0;
#if defined(BCM535X)
	struct ifreq ifr;
	int vecarg[VECLEN];
	int s = -1;	
	int cmd = -1;

	if(lan_port < 0 || lan_port > 4)
	{
		printf("%s:invalid number!\n", __FUNCTION__);
		return -1;
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("open socket failed\n");
		return -1;;
	}

	ret = et_find(ifr.ifr_name);

	vecarg[0] = lan_port << 16;;
	vecarg[0] |= 0 & 0xffff;
	vecarg[1] = 0x1200;
	cmd = SIOCSETCPHYWR2;
	
	ifr.ifr_data = (caddr_t) vecarg;
	if (ioctl(s, cmd, (caddr_t)&ifr) < 0) 
	{
		printf("Error: etcphywr");
		close(s);
		return -1;
	}
	
	close(s);
#elif defined(RTL819X)
	extern tenda_reset_port();

	printf("[%s][%d][zpt]: restarting ports...\n", __FUNCTION__, __LINE__);
	tenda_reset_ports();
#endif
	return ret;
}

