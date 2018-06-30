/*
 * l2tp functions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp.c,v 1.6 2010-08-06 16:07:51 Exp $
 *
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <nvparse.h>
#include <rc.h>
#include <netdb.h>
#include <iflib.h>

extern void pppoe_start(char *pppname);
extern void pppoe_stop(char *pppname);
/*
*add for russia static route
*/
extern void add_static_route(char *static_route);


/*
 * Start function used when the WAN mode
 * set to pppoe
 */
int
wan_pppoe2_start(int unit)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_ifname;
	char pppname[IFNAMSIZ];
	int mtu;
	

	cprintf("%s::Connecting to pppoe2 server, please wait...unit=%d\n", __FUNCTION__,unit);

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));

	ifconfig(wan_ifname, IFUP,
		nvram_safe_get(strcat_r(prefix, "pppoe_ipaddr", tmp)),
		nvram_safe_get(strcat_r(prefix, "pppoe_netmask", tmp)));

	//set mtu
	mtu = atoi(nvram_safe_get(strcat_r(prefix, "mtu", tmp)));
	if(mtu > 0)
		ifconfig_mtu(wan_ifname,mtu);
#ifdef CONFIG_RTL_HARDWARE_NAT
#ifndef HAVE_NOETH
	extern int rtl_setWanNetifMtu(int mtu);
	rtl_setWanNetifMtu(mtu);
#endif
#endif

#ifdef __CONFIG_DHCPC__
	char *value;
	value = nvram_get(strcat_r(prefix, "pppoe_static", tmp));
	if (value && atoi(value) == 0) {
//add static route
		if((value = getenv("static_route"))){
			diag_printf("static_route: %s\n",value);
			add_static_route(value);
		}	
		
		if((value = getenv("static_route2"))){
			diag_printf("static_route2: %s\n",value);
			add_static_route(value);
		}

		if((value = getenv("static_route3"))){
			diag_printf("static_route3: %s\n",value);
			add_static_route(value);
		}
		
//end	
	}
#endif /* __CONFIG_DHCPC__ */	

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	sprintf(pppname, "ppp%d", unit);
	/* Start pppoe */
	pppoe_start(pppname);
	return 0;
}

/*
 * Stop function used when the WAN mode
 * set to pppoe
 */
void
wan_pppoe2_down(char *pppname)
{
	int unit;
	char *wan_ifname;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	/* Stop L2TP */
	if ((unit = ppp_ifunit(pppname)) < 0) {
		cprintf("%s is not a ppp interface\n", pppname);
		return;
	}

	pppoe_stop(pppname);

	/* Stop tunnel */
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	wan_ifname = nvram_get(strcat_r(prefix, "bind_ifname", tmp));
	if (wan_ifname) {
#ifdef __CONFIG_DHCPC__
		char *value;
		//value = nvram_get(strcat_r(prefix, "pppoe_static", tmp));
		value = nvram_get(strcat_r(prefix, "bind_pppoe_static", tmp));
		if (value && atoi(value) == 0)
			dhcpc_stop(wan_ifname);
		else
#endif /* __CONFIG_DHCPC__ */
			wan_down(wan_ifname);
	}

	return;
}

/*
 * bound pppoe2
 */
static int
bound_pppoe2(char *ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;
	char *value;
	int start = 1;
	int stop = 0;
	struct in_addr ip = {0};
	struct in_addr mask = {0};
	char ipaddr[sizeof("255.255.255.255")];
	char netmask[sizeof("255.255.255.255")];
	char gateway[sizeof("255.255.255.255")];
	char *pppname;

	unit = wan_ifunit(ifname);
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	
	/* Get old address */
	if (iflib_getifaddr(ifname, &ip, &mask) == 0 &&
	    ip.s_addr != 0 && mask.s_addr != 0) {
		strcpy(ipaddr, inet_ntoa(ip));
		strcpy(netmask, inet_ntoa(mask));
		/*pxy revise, prevent exception*/
		strcpy(gateway, nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp)));
		/* Check ipaddr */
		if ((value = getenv("ip")) != NULL &&
		    strcmp(ipaddr, value) != 0) {
			stop = 1;
		}
		/* Check netmask */
		if ((value = getenv("subnet")) != NULL &&
		    strcmp(netmask, value) != 0) {
			stop = 1;
		}
		/* Check gateway */
		if ((value = getenv("router")) != NULL &&
		    strcmp(gateway, value) != 0) {
			stop = 1;
		}

		/* Stop pppoe2, if changed */
		if (stop) {
			pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));

			cprintf("bound_pptp::ip changes, l2tp_stop(%s)\n", pppname);
			pppoe_stop(pppname);
		}
		else {
			/* Nothing changed */
			start = 0;
		}
	}

	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "pppoe_ipaddr", tmp), value);
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "pppoe_netmask", tmp), value);
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "pppoe_gateway", tmp), value);

	/* Start pppoe2 */
	if (start)
		wan_pppoe2_start(unit);

	return 0;
}

/*
 * deconfig: This is used when a leases is lost.
 */
static int
deconfig_pppoe2(char *ifname)
{
	char *wan_ifname;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *pppname = NULL;

	/* Get bind interface */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		wan_ifname = nvram_get(strcat_r(prefix, "bind_ifname", tmp));
		if (!wan_ifname)
			continue;

		if (strcmp(wan_ifname, ifname) == 0) {
			pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			break;
		}
	}

	/* Stop pppoe2 */
	if (pppname)
		pppoe_stop(pppname);

	wan_down(ifname);
	return 0;
}

/*
 *pppoedhcpc [deconfig/bound] [ifname] ...
 */
int
pppoe2dhcpc(int argc, char **argv)
{
	char *cmd = argv[1];
	char *ifname = getenv("interface");

	if (!cmd || !ifname)
		return EINVAL;

	if (strstr(cmd, "bound"))
		return bound_pppoe2(ifname);
	if (strstr(cmd, "deconfig"))
		return deconfig_pppoe2(ifname);

	return EINVAL;
}
