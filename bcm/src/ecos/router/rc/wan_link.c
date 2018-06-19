/*
 * Interface link command
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wan_link.c,v 1.4 2011-01-06 03:08:20 Exp $
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <shutils.h>
#include <bcmdevs.h>
#include <iflib.h>
#include <bcmnvram.h>
#include <wlutils.h>

extern int wan_primary_ifunit(void);
extern int wl_probe(char *name);
extern void start_wan(void);
extern void stop_wan(void);
#ifdef __CONFIG_DDNS__
extern void ddns_start(void);
extern void ddns_stop(void);
#endif

int
ifr_link_check(char *ifn)
{
	struct phystatus ps;

#ifdef CONFIG_VLAN
	uint boardflags;
	char wan_ifname[256], name[256], *lan_ifnames, *next;
	char vports_var[256], buf[256], *bufp;
	int pos;
	int is_lanif, i;
#endif	/* CONFIG_VLAN */

	memset(&ps, 0, sizeof(ps));
#ifdef CONFIG_VLAN
	/* retrieve the WAN port number if VLAN is used */
	boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
	if (boardflags & BFL_ENETVLAN) {
		lan_ifnames = nvram_safe_get("lan_ifnames");
		/* assume maximum VLAN id 16 */
		for (i = 0; i < 16; i++) {
			sprintf(wan_ifname, "vlan%d", i);
			is_lanif = 0;
			foreach(name, lan_ifnames, next) {
				if (strcmp(name, wan_ifname) == 0) {
					is_lanif = 1;
					break;
				}
			}
			if (is_lanif)
				continue;

			/* get the port number from nvram */
			sprintf(vports_var, "vlan%dports", i);
			if ((bufp = nvram_get(vports_var)) != NULL) {
				strcpy(buf, bufp);
				pos = strcspn(buf, " ");
				buf[pos] = 0;
				ps.ps_port = atoi(buf);
				break;
			}
		}
	}
#endif	/* CONFIG_VLAN */

	/* assume port in unlink state if ioctl fails */
	if (iflib_ioctl(ifn, SIOCGIFLINK, &ps) < 0)
		ps.ps_link = 0;

	return (ps.ps_link);
}

int
ifr_link_set(char *ifn, int mode)
{
	struct phystatus ps;
	char buf[256];
	int pos;
	char *value;

	/* set port0 status by default */
	memset(&ps, 0, sizeof(ps));

	/* get the port number from nvram */
	value  = nvram_get("vlan1ports");
	if (value) {
		strcpy(buf, value);
		pos = strcspn(buf, " ");
		buf[pos] = 0;
		ps.ps_port = atoi(buf);
	}

	ps.ps_link = mode;

	return iflib_ioctl(ifn, SIOCSIFLINK, &ps);
}

int
ifr_link_baud(char *ifn)
{
	struct phystatus ps;
	char buf[256];
	int pos;
	char *value;

	/* get port0 status by default */
	memset(&ps, 0, sizeof(ps));

	/* get the port number from nvram */
	value = nvram_get("vlan1ports");
	if (value) {
		strcpy(buf, value);
		pos = strcspn(buf, " ");
		buf[pos] = 0;
		ps.ps_port = atoi(buf);
	}

	if (iflib_ioctl(ifn, SIOCGIFBAUD, &ps) < 0) {
		/* assume baudrate in 100Mbps if ioctl fails */
		ps.ps_baud = 100;
	}

	return (ps.ps_baud);
}

int
ifr_link_speed(char *ifn)
{
	struct phystatus ps;
	char buf[256];
	int pos;
	char *value;

	/* get port0 status by default */
	memset(&ps, 0, sizeof(ps));

	/* get the port number from nvram */
	value = nvram_get("vlan1ports");
	if (value) {
		strcpy(buf, value);
		pos = strcspn(buf, " ");
		buf[pos] = 0;
		ps.ps_port = atoi(buf);
	}

	if (iflib_ioctl(ifn, SIOCGIFSPEED, &ps) < 0) {
		ps.ps_speed = PORT_SPEED_100FULL;
	}

	return (ps.ps_speed);
}

int
ifr_set_link_speed(char *ifn, int port, int speed)
{
	struct phystatus ps;

	/* get port0 status by default */
	memset(&ps, 0, sizeof(ps));

	ps.ps_port = port;
	ps.ps_speed = speed;

	return iflib_ioctl(ifn, SIOCSIFSPEED, &ps);
}

//huangxiaoli modify
#define PORT_SPEED_MAX_IND  4
#define TRP_FAILED                      -1
#define TRP_SUCCESS                     0
int
ifr_set_link_speed2(int speed)
{
	char *ifn,*value;
	char buf[256];
	int pos;
	struct phystatus ps;
	int rc;
	int iIndex2Speed[PORT_SPEED_MAX_IND +1]= {PORT_SPEED_AUTO, PORT_SPEED_10HALF,  PORT_SPEED_10FULL, PORT_SPEED_100HALF, PORT_SPEED_100FULL};
	if (PORT_SPEED_MAX_IND  < speed)
	{        
		return TRP_FAILED;
	}
	/* get port0 status by default */
	memset(&ps, 0, sizeof(ps));

	value= nvram_get("vlan2ports");
	if (value) {
		strcpy(buf, value);
		pos = strcspn(buf, " ");
		buf[pos] = 0;
		ps.ps_port = atoi(buf);
	}

	memset(buf,0,sizeof(buf));
	value= nvram_get("wan_ifnames");
	if (value) {
		strcpy(buf, value);
		pos = strcspn(buf, " ");
		buf[pos] = 0;
		ifn = buf;
	}
	    
	ps.ps_speed = iIndex2Speed[speed];
	 
	rc = iflib_ioctl(ifn, SIOCSIFSPEED, &ps);
	
	return rc;
}
#ifdef __CONFIG_NAT__
/* WAN link check here */
static int
wan_link(void)
{
	char *wan_name, wan_ifname[] = "wanXXXXXXXXXX_";
	int ret;
	struct ether_addr bssid = {{0}};

	snprintf(wan_ifname, sizeof(wan_ifname), "wan%d_ifname", wan_primary_ifunit());
	wan_name = nvram_safe_get(wan_ifname);
	
	if (nvram_match("router_disable", "1"))
		return 0;

	/*
	 * Here we should extend to different mode
	 * For example, Wireless WAN mode should
	 * be considered.
	 */
	if ((ret = wl_probe(wan_name)) == 0) {
		if ((wl_ioctl(wan_name, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN) != 0) ||
		    (memcmp(&bssid, "\x00\x00\x00\x00\x00\x00", ETHER_ADDR_LEN) == 0))
			return 0;
		else
			return 1;
	}

	return (ifr_link_check(wan_name) ? 1 : 0);
}

static int chk_link = 0;

int
wan_link_check(void)
{
	static int link_check_cnt = 0;
	int link;

	link = wan_link();
	if (link != chk_link) {
		link_check_cnt++;
		if (link_check_cnt < 3)
			return chk_link;

		link_check_cnt = 0;
	}
	else {
		link_check_cnt = 0;
		return chk_link;
	}

	/* link up */
	if (chk_link == 0 && link) {
		start_wan();
#ifdef __CONFIG_DDNS__
		if(strcmp(nvram_safe_get("ddns_enable"),"1") == 0)
			ddns_start();
#endif
	}
	/* link down */
	else if (chk_link == 1 && link == 0) {
		stop_wan();
#ifdef __CONFIG_DDNS__
		if(strcmp(nvram_safe_get("ddns_enable"),"1") == 0)
			ddns_stop();
#endif
	}

	chk_link = link;

	return chk_link;
}

int
wan_link_status(void)
{
	return chk_link;
}
#endif /* __CONFIG_NAT__ */
