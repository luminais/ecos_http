/*
 * pptp functions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp.c,v 1.6 2010-08-06 16:08:06 Exp $
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

extern int CGI_same_net(unsigned int one_ip,unsigned int other_ip,unsigned int net_mask);
int pptp_start(char *pppname);
void pptp_stop(char *pppname);
/*
*add for russia static route
*/
extern void add_static_route(char *static_route);

/*
 * Start function used when the WAN mode
 * set to pptp
 */
int
wan_pptp_start(int unit)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_ifname;
	char pppname[IFNAMSIZ];
	char dns[256] = {0};
	char *name, *p, *next;
	struct hostent *hostinfo;
	char *server, ip[] = "xxx.xxx.xxx.xxx";
	struct in_addr addr;
	int i;

	cprintf("%s::Connecting to PPTP server, please wait...\n", __FUNCTION__);

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));

	ifconfig(wan_ifname, IFUP,
		nvram_safe_get(strcat_r(prefix, "pptp_ipaddr", tmp)),
		nvram_safe_get(strcat_r(prefix, "pptp_netmask", tmp)));

#ifdef __CONFIG_DHCPC__
	char *value;
	value = nvram_get(strcat_r(prefix, "pptp_static", tmp));
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

	/* Add a host route for DNS */
	strncpy(dns, nvram_safe_get(strcat_r(prefix, "dns", tmp)), sizeof(dns)-1);
	for (name = dns, p = name, i = 0;
	     name && name[0] && i < __CONFIG_AUTO_DNS_NUM__;
	     name = next, p = 0) {
		strtok_r(p, " ", &next);
		if(!CGI_same_net(inet_addr(name), inet_addr(nvram_safe_get(strcat_r(prefix, "pptp_gateway", tmp))), inet_addr(nvram_safe_get(strcat_r(prefix, "pptp_netmask", tmp)))))
		{//tenda modify
			route_add(wan_ifname, 0, name, 
				nvram_safe_get(strcat_r(prefix, "pptp_gateway", tmp)), "255.255.255.255");
		}
	}

	/* connect to peer */
	hostinfo = gethostbyname(nvram_safe_get(strcat_r(prefix, "pptp_server_name", tmp)));
	if (hostinfo && (hostinfo->h_length == 4)) {
		memcpy(&addr, hostinfo->h_addr_list[0], 4);
		server = inet_ntoa(*(struct in_addr *)&addr);
		memcpy(ip, server, sizeof(ip));
	}
	else {
		cprintf("PPTP: looking up %s\n",
			nvram_safe_get(strcat_r(prefix, "pptp_server_name", tmp)));
#if 0//roy modify,2010/11/04
		return -1;
#else
//server is ip address
		snprintf(ip,sizeof(ip),"%s",nvram_safe_get(strcat_r(prefix, "pptp_server_name", tmp)));
#endif
	}
//roy +++
	if(strlen(nvram_safe_get(strcat_r(prefix, "pptp_gateway", tmp)))> 6)//simple check
//+++
	/* Add a host route for pptp server */
	if(!CGI_same_net(inet_addr(ip), inet_addr(nvram_safe_get(strcat_r(prefix, "pptp_gateway", tmp))), inet_addr(nvram_safe_get(strcat_r(prefix, "pptp_netmask", tmp)))))
	{//tenda modify
		route_add(wan_ifname, 1, ip,
			nvram_safe_get(strcat_r(prefix, "pptp_gateway", tmp)), "255.255.255.255");
	}
	/* Should set firewall and nat rules here for tunnel only */
	update_tunnel_firewall(pppname);

	/* Start pptp */
	pptp_start(pppname);
	return 0;
}

/*
 * Stop function used when the WAN mode
 * set to pptp.
 */
void
wan_pptp_down(char *pppname)
{
	int unit;
	char *wan_ifname;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	/* Stop PPTP */
	if ((unit = ppp_ifunit(pppname)) < 0) {
		cprintf("%s is not a ppp interface\n", pppname);
		return;
	}

	pptp_stop(pppname);

	/* Stop tunnel */
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	wan_ifname = nvram_get(strcat_r(prefix, "bind_ifname", tmp));
	if (wan_ifname) {
#ifdef __CONFIG_DHCPC__
		char *value;
		value = nvram_get(strcat_r(prefix, "bind_pptp_static", tmp));
		if (value && atoi(value) == 0)
			dhcpc_stop(wan_ifname);
		else
#endif /* __CONFIG_DHCPC__ */
			wan_down(wan_ifname);
	}

	return;
}

/*
 * bound pptp
 */
static int
bound_pptp(char *ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;
	char *value;
	char new_dns[256];
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
		strcpy(gateway, nvram_safe_get(strcat_r(prefix, "pptp_gateway", tmp)));
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

		/* Stop pptp, if changed */
		if (stop) {
			pppname = nvram_safe_get(strcat_r(prefix, "pptp_ifname", tmp));

			cprintf("bound_pptp::ip changes, pptp_stop(%s)\n", pppname);
			pptp_stop(pppname);
		}
		else {
			/* Nothing changed */
			start = 0;
		}
	}

	/* Save environ variables */
	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "pptp_ipaddr", tmp), value);
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "pptp_netmask", tmp), value);
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "pptp_gateway", tmp), value);
	if ((value = getenv("dns"))) {
		del_dest_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)));
		nvram_set(strcat_r(prefix, "dns", tmp), value);
		add_ns(value, new_dns, ADD_NS_PUT);
	}
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), value);
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), value);
	if ((value = getenv("lease")))
		nvram_set(strcat_r(prefix, "lease", tmp), value);
	if ((value = getenv("expiry")))
		nvram_set(strcat_r(prefix, "expiry", tmp), value);

	/* Start pptp */
	if (start)
		wan_pptp_start(unit);

	return 0;
}

/*
 * deconfig: This is used when a leases is lost.
 */
static int
deconfig_pptp(char *ifname)
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
			pppname = nvram_safe_get(strcat_r(prefix, "pptp_ifname", tmp));
			break;
		}
	}

	/* Stop pptp */
	if (pppname)
		pptp_stop(pppname);

	wan_down(ifname);
	return 0;
}

/*
 * pptpdhcpc [deconfig/bound] [ifname] ...
 */
int
pptpdhcpc(int argc, char **argv)
{
	char *cmd = argv[1];
	char *ifname = getenv("interface");

	if (!cmd || !ifname)
		return EINVAL;
	cyg_thread_delay(10);
	if (strstr(cmd, "bound"))
		return bound_pptp(ifname);
	if (strstr(cmd, "deconfig"))
		return deconfig_pptp(ifname);

	return EINVAL;
}
