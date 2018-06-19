/*
 * VLAN configuration functions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: vlan_config.c,v 1.3 2010-06-11 11:07:21 Exp $
 *
 */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/if_vlan_var.h>
#include <errno.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <rc.h>
#include <stdio.h>
#include <stdlib.h>
#include <bcmparams.h>

int wan_primary_ifunit(void);

int
clone_create(char *ifname)
{
	int s;
	struct ifreq ifr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		printf("%s: socket(): %s\n", __func__, strerror(errno));
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCIFCREATE, &ifr) < 0) {
		printf("%s: ioctl(SIOCIFCREATE) failed: %s\n", __func__, strerror(errno));
		close(s);
		return -1;
	}

	if (strcmp(ifname, ifr.ifr_name) != 0)
		printf("%s created instead of %s\n", ifr.ifr_name, ifname);

	close(s);
	return 0;
}

int
vlan_set(char *ifname, char *parent, unsigned short tag)
{
	int s;
	struct ifreq ifr;
	struct vlanreq vreq;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		printf("%s: socket(): %s\n", __func__, strerror(errno));
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

	strncpy(vreq.vlr_parent, parent, sizeof(vreq.vlr_parent));
	vreq.vlr_tag = tag;

	ifr.ifr_data = (caddr_t)&vreq;
	if (ioctl(s, SIOCSETVLAN, &ifr) < 0) {
		printf("%s: ioctl(SIOCSETVLAN) failed: %s\n", __func__, strerror(errno));
		close(s);
		return -1;
	}

	close(s);
	return 0;
}

static int ea_to_ifname(char *ea, char *buf)
{
	struct ifreq ifr;
	int rc = -1;
	int s, i;
	
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return errno;
	
	for (i = 1; i <= DEV_NUMIFS; i++) {
		ifr.ifr_ifindex = i;
		if (ioctl(s, SIOCGIFNAME, &ifr))
			continue;
		if (ioctl(s, SIOCGIFHWADDR, &ifr))
			continue;
		if (ifr.ifr_hwaddr.sa_family != AF_LINK)
			continue;
		if (!bcmp(ifr.ifr_hwaddr.sa_data, ea, ETHER_ADDR_LEN)) {
			strcpy(buf, ifr.ifr_name);
			rc = 0;
			break;
		}
	}
	close(s);
	return rc;
}

int
vlan_config(void)
{
	int ret = 0;
	uint boardflags;
	char name[256];
	char hwname[64] = {0};
	char *lan_ifnames, *wan_ifnames, *next;
	char *vlan_ifname = NULL, *vlan_hwname = NULL;
	int vid = -1;
	char vlan_parent[32];
	char nvvar_name[16];
	char *hwaddr = NULL;
	unsigned char ea[ETHER_ADDR_LEN];

	boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
	if ((boardflags & BFL_ENETVLAN) != BFL_ENETVLAN)
	{
		printf("VLAN interfaces not supported\n");
		return 0;
	}

	/* retrieve VLAN id from the LAN interface name */
	lan_ifnames = nvram_safe_get("lan_ifnames");
	foreach(name, lan_ifnames, next) {
		if (!strncmp(name, "vlan", 4))
		{
			vlan_ifname = name;
			vid = atoi(&name[4]);
			break;
		}
	}

	if (vlan_ifname == NULL)
		return 0;
	sprintf(hwname, "%shwname", vlan_ifname);
	if ((vlan_hwname = nvram_get(hwname)) == NULL)
		return 0;
	snprintf(nvvar_name, sizeof(nvvar_name), "%smacaddr", vlan_hwname);
	if (!(hwaddr = nvram_get(nvvar_name)))
		return 0;
	ether_atoe(hwaddr, ea);
	if (ea_to_ifname((char *)ea, vlan_parent) != 0)
		return 0;
	
	printf("%s: VLAN interface created\n", vlan_ifname);
	ret = clone_create(vlan_ifname);
	if (ret != 0)
		return (ret);
	ret = vlan_set(vlan_ifname, vlan_parent, vid);
	if (ret != 0)
		return (ret);

	/* retrieve VLAN id from the WAN interface name */
	vlan_ifname = vlan_hwname = NULL;
	wan_ifnames = nvram_safe_get("wan_ifnames");
	foreach(name, wan_ifnames, next) {
		if (!strncmp(name, "vlan", 4))
		{
			vlan_ifname = name;
			vid = atoi(&name[4]);
			break;
		}
	}

	if (vlan_ifname == NULL)
		return 0;
	sprintf(hwname, "%shwname", vlan_ifname);
	if ((vlan_hwname = nvram_get(hwname)) == NULL)
		return 0;
	snprintf(nvvar_name, sizeof(nvvar_name), "%smacaddr", vlan_hwname);
	if (!(hwaddr = nvram_get(nvvar_name)))
		return 0;
	ether_atoe(hwaddr, ea);
	if (ea_to_ifname((char *)ea, vlan_parent) != 0)
		return 0;

	printf("%s: VLAN interface created\n", vlan_ifname);
	ret = clone_create(vlan_ifname);
	if (ret != 0)
		return (ret);
	ret = vlan_set(vlan_ifname, vlan_parent, vid);
	if (ret != 0)
		return (ret);

	return (ret);
}
