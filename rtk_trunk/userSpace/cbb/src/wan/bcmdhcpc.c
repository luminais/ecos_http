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
#include "sys_option.h"

#ifdef __CONFIG_APCLIENT_DHCPC__

extern RET_INFO tpi_apclient_dhcpc_set_ip(PI8 *ip,PI8 *mask,PI8 *gateway);
extern RET_INFO tpi_apclient_dhcpc_lan_dhcp_action_handle(MODULE_COMMON_OP_TYPE action);
#endif

#ifdef __CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__
extern RET_INFO tpi_apclient_dhcpc_set_dhcpd(PI8 *ip,PI8 *mask);
#endif

#ifdef __CONFIG_A9__
#include "sys_module.h"
#include "rc_module.h"
#include "apclient_dhcpc.h"
extern char auto_conn_extend_prev_mac[24];
extern int sync_bridge_conf();
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

			tenda_route_add(NULL, 1,dest_ip, dest_gw, net_mask);
		}
}

//end

/*
 * bound [ifname] [ip=ipaddr] [subnet=netmask] [router=gateway]
 *	[dns=dns] [domain=domain] [wins=wins] [lease=lease]
 *	[t1=t1] [t2=t2]
 */
extern int arp_check(char *ifname, struct in_addr *tip, u_char *enaddr);
static int
get_ifmac(char *ifname, char ifmac[6])
{
	int s;
	struct ifreq ifr;
	int rc = -1;
	if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0 ||
	    memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", 6) == 0) {
		goto errout;
	}
	memcpy(ifmac, ifr.ifr_hwaddr.sa_data, 6);
	rc = 0;
errout:
	close(s);
	return 0;
}
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
 	struct in_addr wan_ip;
	char if_mac[6];
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
	{
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
		inet_aton(value, &wan_ip);
	}
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask", tmp), value);
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);

	/*删除之前DHCP的DNS*/
	strcpy(new_dns, nvram_safe_get(strcat_r(prefix, "dns", tmp)) );
	del_ns(new_dns);

	if ((value = getenv("dns"))) {
//roy+++ for dns,2010/09/14
		dns_fix = atoi(nvram_safe_get(strcat_r(prefix, "dns_fix", tmp)));
		if(dns_fix){
			add_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)), NULL, ADD_NS_PUSH);
		}
		else
//+++
		{
			/*添加DHCP的DNS*/
			nvram_set(strcat_r(prefix, "dns", tmp), value);
			add_ns(value, NULL, ADD_NS_PUSH);
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
		tenda_ifconfig(ifname, IFUP,
			nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
			nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
		//set mtu
			mtu = atoi(nvram_safe_get(strcat_r(prefix, "mtu", tmp)));
			if(mtu > 0)
				ifconfig_mtu(ifname,mtu);
			#ifndef HAVE_NOETH
			extern int rtl_setWanNetifMtu(int mtu);
			rtl_setWanNetifMtu(mtu);
			#endif

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
/*lq add do arp conflict check*/
		get_ifmac(ifname,if_mac);
		if(!arp_check(ifname, &wan_ip, if_mac))
		{
			printf("coming...............\n");
			dhcpc_decline(ifname);
		}
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
	char dns_tmp[128];
	char if_mac[6];
	struct in_addr lan_ip;
	char ipaddr[sizeof("255.255.255.255")];
	char netmask[sizeof("255.255.255.255")];
	char gateway[sizeof("255.255.255.255")];

	i = lan_unit(ifname);
	if (!i)
		snprintf(prefix, sizeof(prefix), "lan_");
	else
		snprintf(prefix, sizeof(prefix), "lan%x_", i);

#if defined(__CONFIG_APCLIENT_DHCPC__) && defined(__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__)
	char now_ip[16] = {0},now_mask[16] = {0};
	unsigned char connect_tag = 0;
	gpi_apclient_dhcpc_addr(now_ip,now_mask);
	if (0 == strcmp(now_ip,"") || 0 == strcmp(now_mask,""))
	{
		connect_tag = 0;
	}
	else
	{
		connect_tag = 1;
	}
#endif
	/* Get old address */
#if defined(__CONFIG_APCLIENT_DHCPC__) && defined(__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__)
	if (connect_tag && iflib_getifaddr(ifname, &ip, &mask) == 0 &&
	    ip.s_addr != 0 && mask.s_addr != 0)
#else
	if (iflib_getifaddr(ifname, &ip, &mask) == 0 &&
	    ip.s_addr != 0 && mask.s_addr != 0)
#endif
	{
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
	{
		inet_aton(value,&lan_ip);
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
	}
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask", tmp), value);
	if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);

	if ((value = getenv("dns"))) {

		strcpy(dns_tmp, nvram_safe_get(strcat_r(prefix, "dns", tmp)) );
		del_ns(dns_tmp);

		nvram_set(strcat_r(prefix, "dns", tmp), value);
		add_ns(value, NULL, ADD_NS_PUT);
	}

	/* Start lan */
	if (start) {
#ifdef __CONFIG_APCLIENT_DHCPC__
		tpi_apclient_dhcpc_set_ip(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),nvram_safe_get(strcat_r(prefix, "netmask", tmp)),nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
#endif
#ifdef __CONFIG_A9__
		/*为了再次确保页面能够获取扩展状态，这里再次延迟一段时间*/
		extern PIU8 tpi_apclient_dhcpc_get_web_wait_tag();
		extern PIU8 tpi_apclient_dhcpc_get_web_get_return_tag();
		extern void tpi_apclient_dhcpc_set_web_wait(PIU8 set_wait,PIU8 get_return);
		cyg_tick_count_t time = cyg_current_time();

		if (1 == tpi_apclient_dhcpc_get_web_wait_tag())
		{
			while (1 != tpi_apclient_dhcpc_get_web_get_return_tag())
			{
				if ((cyg_current_time() - time) >= APCLIENT_DHCPC_WEB_WAIT_WRITE_IP_TIME)
				{
					break;
				}
				cyg_thread_delay(10);
			}
		}
		printf("##########################[%s.%d]:wait_time:%lld\n",__func__,__LINE__,cyg_current_time() - time);
		if (1 == tpi_apclient_dhcpc_get_web_wait_tag())
		{
			/*这里当页面获取到扩展状态之后等待3s，确保页面正常*/
			cyg_thread_delay(300);
		}
		tpi_apclient_dhcpc_set_web_wait(0,0);
#endif
		/* 桥模式下设置eth1接口从上级获取ip，将获取到的ip设置到lan口eth0 ，此处是设置lan口eth0的ip*/
		if(nvram_match(SYSCONFIG_WORKMODE, "bridge") )
		{
			tenda_ifconfig(nvram_safe_get("lan_ifname"), IFUP,
			 	nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
			 	nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
		}
		else
		{
			tenda_ifconfig(ifname, IFUP,
				 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
				 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
		}
		
		
#ifdef __CONFIG_APCLIENT_DHCPC__
#ifdef __CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__
		tpi_apclient_dhcpc_set_dhcpd(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
#endif
		tpi_apclient_dhcpc_lan_dhcp_action_handle(OP_STOP);
#endif
		lan_up(ifname);

#ifdef __CONFIG_A9__
	/*lq 后期需要同步到主线,apclient模式下进行ip冲突检测*/
	get_ifmac(ifname,if_mac);
	if(!arp_check(ifname, &lan_ip, if_mac))
	{
		printf("decline................................\n");
		dhcpc_decline(ifname);
	}
	//fh add 桥接成功获取到IP重启httpd 2016、5、5
	msg_send(MODULE_RC, RC_HTTP_MODULE, "op=3");
	/*lq 保存自动桥接成功后mac 地址*/
	if(0 == strcmp(nvram_safe_get("wl0_relaytype"),"auto"))
	{
		nvram_set("auto_conn_prev_mac", auto_conn_extend_prev_mac);
	}
#endif
#ifdef __CONFIG_A9__
	if(sync_bridge_conf())
	{
		char msg_param[16] = {0};
		sprintf(msg_param, "op=%d", OP_RESTART);
		msg_send(MODULE_RC, RC_WIFI_MODULE, msg_param);
	}
#endif

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
	del_all_ns();

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
