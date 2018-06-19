/*
 * BCM dhcpc script functions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: bcmdhcpc.c,v 1.6 2010-08-09 14:11:29 Exp $
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
#include <rc.h>
#include <iflib.h>

//roy add
#include <router_net.h>
#ifdef __CONFIG_APCLIENT_DHCPC__
#include "pi_common.h"
extern PI_INFO api_set_apclient_dhcpc_addr(char * ip,char * mask);
extern PI_INFO api_apclient_lan_dhcp_action(PI_ACTION action);
#endif

#ifdef __CONFIG_NAT__
/*
 * lease [ifname] [expiry=expiry]
 *
 * Note: This is a special patch for NTP time.
 */
static int
lease_wan(char *ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;
	char *value;

	unit = wan_ifunit(ifname);
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if ((value = getenv("expiry")))
		nvram_set(strcat_r(prefix, "expiry", tmp), value);

	return 0;
}

//add for static route
void add_static_route(char *static_route)
{
	char *value;
	char *name, *p,*p2,*next;

	unsigned char net_mask[16];
	
	unsigned char dest_ip[16];
	
	unsigned char dest_gw[16];

	value = static_route;
	
	for (name = value, p = name;
	     name && name[0];
	     name = next, p = 0) {
			strtok_r(p, " ", &next);

			/* dest_ip/dest_gw/net_mask*/
			p2 = strchr(name,'/');
			if(p2){
				*p2 = '\0';
				strncpy(dest_ip,name,15);
				dest_ip[15]='\0';
				name = ++p2;
			}else{
				continue;
			}	
			
			/* dest_gw/net_mask*/
			p2 = strchr(name,'/');
			
			if(p2){
				*p2 = '\0';
				strncpy(dest_gw,name,15);
				dest_gw[15]='\0';
				name = ++p2;
			}else{
				continue;
			}	
	
			strncpy(net_mask,name,15);
			net_mask[15]='\0';

			route_add(NULL, 1,dest_ip, dest_gw, net_mask);
		}
}

//end

/*
 * bound [ifname] [ip=ipaddr] [subnet=netmask] [router=gateway]
 *	[dns=dns] [domain=domain] [wins=wins] [lease=lease]
 *	[t1=t1] [t2=t2]
 */
static int
bound_wan(char *ifname)
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
//roy+++,2010/09/14
	int dns_fix;
	int mtu;
//+++

	unit = wan_ifunit(ifname);
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* Get old address */
	if (iflib_getifaddr(ifname, &ip, &mask) == 0 &&
	    ip.s_addr != 0 && mask.s_addr != 0) {
		strcpy(ipaddr, inet_ntoa(ip));
		strcpy(netmask, inet_ntoa(mask));
		/*pxy revise, prevent exception*/
		strcpy(gateway, nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
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

		/* wan down */
		if (stop) {
			cprintf("%s(%s):ip setting changes, stop\n", __func__, ifname);
			wan_down(ifname);
		}
		else {
			cprintf("%s(%s):nothing changes\n", __func__, ifname);
			start = 0;
		}
	}

	/* Save bound variables */
	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask", tmp), value);
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);
	if ((value = getenv("dns"))) {
//roy+++ for dns,2010/09/14			
		dns_fix = atoi(nvram_safe_get(strcat_r(prefix, "dns_fix", tmp)));
		if(dns_fix){
			add_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)), NULL, ADD_NS_PUSH);
		}
		else
//+++
		{
			del_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)));
#ifdef __CONFIG_DNS_8_8_8_8_SUPPORT__
			sprintf(value,"%s %s",value,"8.8.8.8");//gong 20130619´Ë´¦ÊÇ·ñÐèÒªÌí¼Ó±¸ÓÃdns"8.8.8.8"È·±£ÉÏÍøå?å?
#endif
			nvram_set(strcat_r(prefix, "dns", tmp), value);
			add_ns(value, new_dns, ADD_NS_PUSH);
		}
	}
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), value);
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), value);
	if ((value = getenv("lease")))
		nvram_set(strcat_r(prefix, "lease", tmp), value);
	if ((value = getenv("expiry")))
		nvram_set(strcat_r(prefix, "expiry", tmp), value);

	/* Start wan */
	if (start) {
		ifconfig(ifname, IFUP,
			nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
			nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
		//set mtu
			mtu = atoi(nvram_safe_get(strcat_r(prefix, "mtu", tmp)));
			if(mtu > 0)
				ifconfig_mtu(ifname,mtu);
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
		wan_up(ifname);
	}
	return 0;
}

/*
 * deconfig: This is used when a leases is lost.
 */
static int
deconfig_wan(char *ifname)
{
	{//roy add
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;

	unit = wan_ifunit(ifname);
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	
	/* Save wan status */
	nvram_set(strcat_r(prefix, "connect", tmp), "Disconnected");
	}//
	wan_down(ifname);
	return 0;
}

/*
 * wandhcpc [deconfig/bound] [ifname] ...
 */
int
wandhcpc(int argc, char **argv)
{
	char *cmd = argv[1];
	char *ifname = getenv("interface");

	if (!cmd || !ifname)
		return EINVAL;

	if (strstr(cmd, "bound"))
		return bound_wan(ifname);
	if (strstr(cmd, "lease"))
		return lease_wan(ifname);
	if (strstr(cmd, "deconfig"))
		return deconfig_wan(ifname);

	return EINVAL;
}
#endif	/* __CONFIG_NAT__ */

static int
lan_unit(char *ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int i;
	char *lan_ifname;
	char *lan_dhcp;
	
#ifdef __CONFIG_APCLIENT_DHCPC__
	return (MAX_NO_BRIDGE+1);
#endif

	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (!i)
			snprintf(prefix, sizeof(prefix), "lan_");
		else
			snprintf(prefix, sizeof(prefix), "lan%x_", i);

		/* make sure the connection exists and is enabled */ 
		lan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		if (!lan_ifname || strcmp(lan_ifname, ifname) != 0)
			continue;
		lan_dhcp = nvram_get(strcat_r(prefix, "dhcp", tmp));
		if (!lan_dhcp || strcmp(lan_dhcp, "dhcp") != 0)
			continue;
		/* Find the unit */
		break;
	}
	if (i == MAX_NO_BRIDGE) {
		/* Neither wan, nor lan */
		return -1;
	}

	return i;
}

/*
 * bound [ifname] [ip=ipaddr] [subnet=netmask] [router=gateway]
 *	[dns=dns] [domain=domain] [wins=wins] [lease=lease]
 *	[t1=t1] [t2=t2]
 */
static int
bound_lan(char *ifname)
{
	char tmp[100], prefix[] = "lanXXXXXXXXXX_";
	int i;
	char *value;
	int start = 1;
	int stop = 0;
	struct in_addr ip = {0};
	struct in_addr mask = {0};
	char ipaddr[sizeof("255.255.255.255")];
	char netmask[sizeof("255.255.255.255")];
	char gateway[sizeof("255.255.255.255")];

	i = lan_unit(ifname);
	if (!i)
		snprintf(prefix, sizeof(prefix), "lan_");
	else
		snprintf(prefix, sizeof(prefix), "lan%x_", i);

	/* Get old address */
	if (iflib_getifaddr(ifname, &ip, &mask) == 0 &&
	    ip.s_addr != 0 && mask.s_addr != 0) {
		strcpy(ipaddr, inet_ntoa(ip));
		strcpy(netmask, inet_ntoa(mask));
		/*pxy revise, prevent exception*/
		strcpy(gateway, nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
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

		/* lan down */
		if (stop)
			lan_down(ifname);
		else
			start = 0;
	}

	/* Save bound variables */
	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask", tmp), value);
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);

	/* Start lan */
	if (start) {
		ifconfig(ifname, IFUP,
			 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
			 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

#ifdef __CONFIG_APCLIENT_DHCPC__
		api_set_apclient_dhcpc_addr(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
		api_apclient_lan_dhcp_action(PI_STOP);
#endif
 		lan_up(ifname);
#ifdef __CONFIG_WPS__
		/* re-start wps */
		wps_stop();
		wps_start();
#endif
	}
	return 0;
}

/*
 * deconfig: This is used when a leases is lost.
 */
static int
deconfig_lan(char *ifname)
{
	lan_down(ifname);
	return 0;
}

/*
 * landhcpc [deconfig/bound] [ifname] ...
 */
int
landhcpc(int argc, char **argv)
{
	char *cmd = argv[1];
	char *ifname = getenv("interface");

	if (!cmd || !ifname)
		return EINVAL;

	if (strstr(cmd, "bound"))
		return bound_lan(ifname);
	if (strstr(cmd, "deconfig"))
		return deconfig_lan(ifname);

	return EINVAL;
}
