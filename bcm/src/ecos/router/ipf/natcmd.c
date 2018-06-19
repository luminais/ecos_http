/*
 * NAT commands.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: natcmd.c,v 1.11 2010-11-04 04:06:42 Exp $
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <iflib.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <net/if_var.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include "netinet/ip_nat.h"
#include "ipf.h"
#include "netinet/ip_state.h"
#include "netinet/ip_proxy.h"
#include "ip_filfast.h"

#include <netconf.h>
#include <nvparse.h>

#include <sys/syslog.h>


#define IPF_FASTNAT	1

#define	NAT_ICMP_ID_MAP_STRT	10000
#define	NAT_ICMP_ID_MAP_INC	((65536-NAT_ICMP_ID_MAP_STRT)/MAX_NO_BRIDGE)

#define MAX_NAT_ARGS		10

#define PMAP_MAX_NUM		50
#define VTS_MAX_NUM		50
#define PTRG_MAX_NUM		50
#define DMZ_MAX_NUM		WAN_IPALIAS_NUM+1

enum {
	PMAP_ENABLE,
	PMAP_DESCR,
	PMAP_PORTS,
	PMAP_PROTO,
	PMAP_IP,
	PMAP_ARGC
};

enum {
	TRIG_ENABLE,
	TRIG_DESCR,
	TRIG_EXT_PORTS,
	TRIG_EXT_PROTO,
	TRIG_PORTS,
	TRIG_PROTO,
	TRIG_ARGC
};

enum {
	VTS_ENABLE,
	VTS_DESCR,
	VTS_EXT_PORT,
	VTS_PROTO,
	VTS_INT_IP,
	VTS_INT_PORT,
	VTS_SCHE_EN,
	VTS_SCHE_TIME,
	VTS_SCH_DAY,
	VTS_ARGC
};

enum {
	DMZ_ENABLE,
	DMZ_EXT_IP,
	DMZ_HOST,
	DMZ_ARGC
};

extern int flushtable __P((int, int));
extern int nataddrule __P((char *fmt, ...));
extern int natdelrule __P((char *fmt, ...));

extern int wan_primary_ifunit(void);
extern int ppp_ifunit(char *ifname);
extern ipnat_t *natparse __P((char *, int, int *));

extern int countbits __P((u_32_t));

/* OS layer implementation */
static char *
ipnat_wanifname(char *wan_ifname)
{
	int unit = wan_primary_ifunit();
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
	    nvram_match(strcat_r(prefix, "proto", tmp), "static") ||
	    nvram_match(strcat_r(prefix, "proto", tmp), "8021x")) {
		strcpy(wan_ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	}
	else {
		sprintf(wan_ifname, "ppp%d", unit);
	}
	return wan_ifname;
}

static char *
ipnat_lanifname(int lanid, char *lan_ifname)
{
	char tmp[100];
	char *value;

	if (lan_ifname == 0)
		return 0;
	/* Get lan interface name */
	if (lanid == 0) {
		value = nvram_get("lan_ifname");
	} else {
		snprintf(tmp, sizeof(tmp), "lan%x_ifname", lanid);
		value = nvram_get(tmp);
	}
	if (value == 0)
		return 0;
	/* Get interface ip and netmask */
	strncpy(lan_ifname, value, IFNAMSIZ);
	lan_ifname[IFNAMSIZ] = 0;
	return lan_ifname;
}

static char *
ipnat_lanhost(char *cp, char *lanhost)
{
	int i;
	char lan_ifname[32];
	struct in_addr addr;
	struct in_addr lanip[MAX_NO_BRIDGE];
	struct in_addr lanmask[MAX_NO_BRIDGE];

	/* Check the input ip */
	if (!cp || inet_aton(cp, &addr) == 0)
		return 0;

	/* Match ip */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (ipnat_lanifname(i, lan_ifname) == 0)
			continue;

		if (iflib_getifaddr(lan_ifname, &lanip[i], &lanmask[i]) != 0)
			continue;

		/* Do matching */
		if ((addr.s_addr & lanmask[i].s_addr) ==
		    (lanip[i].s_addr & lanmask[i].s_addr))
			break;
	}

	/* Check ip */
	if (i == MAX_NO_BRIDGE) {
		if (lanip[0].s_addr == 0)
			return 0;

		addr.s_addr = (addr.s_addr & ~lanmask[0].s_addr) |
		              (lanip[0].s_addr & lanmask[0].s_addr);
	}

	/* Convert to ip string */
	strcpy(lanhost, inet_ntoa(addr));
	return lanhost;
}

/*
 * Convert NVRAM data to ipfilter rule
 * forward_portXXX
 *    [wanport start]-[wanport end]>[lanip addr]:[lanport start]-[lanport end],[protocol],[enable]
 * ex. 21-23>192.168.1.120:21-23,tcp,on
 *
 * autofw_portXXX
 *    [outbound proto]:[outbound port start]-[outbound port end],[inbound proto]:
 *	  [inbound port start]-[inbound port end]>[to start]-[to end],[enable]
 * ex. tcp:100-110,tcp:200-210>300-310,on
 *
 * dmz_ipaddr
 *    [internal ip addr]
 * ex. 192.168.1.230
 */
static void
ipnat_loadrule(void)
{
	int i;
	char *value;

	char lan_ifname[32];
	char wan_ifname[32];
	char wanipstr[16];
	struct in_addr wanip;

	char lanhost[16];
	char wan_port[20], trigger_port[20];
	netconf_nat_t nat;
	netconf_app_t app;

	if(ipnat_lanifname(0, lan_ifname) == 0)
		return ;
	/* Load wan ip */
	ipnat_wanifname(wan_ifname);
	if (iflib_getifaddr(wan_ifname, &wanip, 0) != 0)
		return;
	strcpy(wanipstr, inet_ntoa(wanip));
	/* Persistent (static) port forwards */
	for (i = 0; i < MAX_NVPARSE; i++) {
		memset(&nat, 0, sizeof(nat));
		if (get_forward_port(i, &nat) && !(nat.match.flags & NETCONF_DISABLED)) {
			if (nat.match.dst.ports[0] != nat.match.dst.ports[1])
				sprintf(wan_port, "%u-%u",
					ntohs(nat.match.dst.ports[0]),
					ntohs(nat.match.dst.ports[1]));
			else
				sprintf(wan_port, "%u", ntohs(nat.match.dst.ports[0]));

			nataddrule("rdr %s %s/32 port %s -> %s port %u %s\n",
				wan_ifname, wanipstr,
				wan_port,
				inet_ntoa(nat.ipaddr),
				ntohs(nat.ports[0]),
				//(nat.match.ipproto == IPPROTO_TCP)?"tcp":"udp");
				nat.match.ipproto_str);//roy modify,2010/10/13
				
			nataddrule("rdr %s %s/32 port %s -> %s port %u %s\n",
				lan_ifname, wanipstr,wan_port,
				inet_ntoa(nat.ipaddr),
				ntohs(nat.ports[0]),
				nat.match.ipproto_str);//add by zengmin for nat loopback @20140618
		}
	}
	/* Application specific port forwards */
	for (i = 0; i < MAX_NVPARSE; i++) {
		memset(&app, 0, sizeof(app));
		if (get_autofw_port(i, &app) && !(app.match.flags & NETCONF_DISABLED)) {
			if (app.match.dst.ports[0] != app.match.dst.ports[1])
				sprintf(trigger_port, "%u-%u",
					ntohs(app.match.dst.ports[0]),
					ntohs(app.match.dst.ports[1]));
			else
				sprintf(trigger_port, "%u", ntohs(app.match.dst.ports[0]));

			if (app.dport[0] != app.dport[1])
				sprintf(wan_port, "%u-%u",
					ntohs(app.dport[0]),
					ntohs(app.dport[1]));
			else
				sprintf(wan_port, "%u", ntohs(app.dport[0]));
			nataddrule("rdr %s %s/32 port %s -> any port %u %s autofw port %s %s\n",
				wan_ifname, wanipstr,
				wan_port,
				ntohs(app.to[0]),
				(app.proto == IPPROTO_TCP)?"tcp":"udp",
				trigger_port,
				(app.match.ipproto == IPPROTO_TCP)?"tcp":"udp");
		}
	}
	/* DMZ */
//roy +++,2010/09/18
	value = nvram_get("dmz_ipaddr_en");
	if(value && atoi(value)){
//+++
		value = nvram_get("dmz_ipaddr");
		if (value) {
			if (ipnat_lanhost(value, lanhost)) {
				nataddrule("rdr %s %s/32 port 0-65535 -> %s port 0 tcp/udp\n",
					wan_ifname, wanipstr, lanhost);
				//add by zengmin for nat loopback @20140618
				nataddrule("rdr %s %s/32 port 0-65535 -> %s port 0 tcp/udp\n",
					lan_ifname, wanipstr, lanhost);
				}
			}
//roy +++
	}
//+++	
	return;
}

/*
 * Set NAT (private network) flag to interface
 */
static int
nat_flag_set(char *ifname, int flag, int on)
{
	struct ifreq ifr;
	int rc = 0;
	int s;

	s = socket(AF_INET, SOCK_RAW, 0);
	if (s < 0)
		return -1;
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(s, SIOCGIFFLTFLAGS, &ifr) != 0) {
		rc = -1;
		goto out;
	}
	ifr.ifr_flags &= ~flag;
	if (on)
		ifr.ifr_flags |= flag;
	if (ioctl(s, SIOCSIFFLTFLAGS, &ifr)) {
		rc = -1;
		goto out;
	}
out:
	close(s);
	return rc;
}

/*
 * Initialize NAPT rules for
 * all the private interfaces
 */
void
ipnat_napt_init(char *wan_ifname)
{
	int i;
	char lan_ifname[32];
	int mtu;
	char mssclampstr[20];
	int ids, ide;
	struct in_addr lanip;
	struct in_addr lanmask;
	char ifname_mtu[64] = {0};

	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		/*
		 * Make sure this lan
		 * interface is on.
		 */
		if (ipnat_lanifname(i, lan_ifname) == 0)
			continue;
		if (iflib_getifaddr(lan_ifname, &lanip, &lanmask) != 0)
			continue;
		/* Set default TCP/UDP rules */
		if (iflib_ioctl(wan_ifname, SIOCGIFMTU, (void *)&mtu) != 0)
			mtu = 1500;
		//hqw add for mtu log
		sprintf(ifname_mtu,"%s mtu %d",wan_ifname,mtu);
		syslog(LOG_INFO,ifname_mtu);
		//end
		sprintf(mssclampstr, "mssclamp %d", mtu - 40);
		nataddrule("map %s %s/%i -> 0/32 portmap tcp/udp auto %s\n",
			wan_ifname, inet_ntoa(lanip), countbits(lanmask.s_addr), mssclampstr);
		/* Set default ICMP rules */
		ids = NAT_ICMP_ID_MAP_STRT + i * NAT_ICMP_ID_MAP_INC;
		ide = ids + NAT_ICMP_ID_MAP_INC - 1;
		nataddrule("map %s %s/%d -> 0/32 portmap icmp auto\n",
			wan_ifname, inet_ntoa(lanip), countbits(lanmask.s_addr), ids, ide);
		//add by zengmin for nat loopback @20140618
		nataddrule("map %s %s/%d -> 0/32", lan_ifname, inet_ntoa(lanip), countbits(lanmask.s_addr));

	}
}

#ifdef IPF_FASTNAT
int
ipnat_fastnat_activate(int activate)
{
	int fd, rc;

	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return -1;
	rc = ioctl(fd, SIOCSFASTNAT, &activate);
	if (rc)
		printf("%s fast NAT failed\n", activate?"Activate":"Deactivate");
	close(fd);
	return rc;
}
#endif

/* Start NAT */
void
ipnat_init(void)
{
	int nat_en;
	int i;
	char lan_ifname[32];
	char wan_ifname[32];

	/* Get NAT enable flag */
	if (!nvram_match("router_disable", "1"))
		nat_en = 1;
	else
		nat_en = 0;
	/* Set NAT flags to each lan intefaces */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (ipnat_lanifname(i, lan_ifname) == 0)
			continue;

		nat_flag_set(lan_ifname, IFFLT_NAT, nat_en);
	}
	/* Quit if NAT is not enabled */
	if (nat_en == 0)
		return;

	/*
	 * Now check the WAN interface and
	 * set the rules.
	 */
	ipnat_wanifname(wan_ifname);
	if (iflib_getifaddr(wan_ifname, 0, 0) != 0)
		return;

	/* Reconstruct all rule from nvram */
	ipnat_loadrule();
	ipnat_napt_init(wan_ifname);
#ifdef IPF_FASTNAT
	ipnat_fastnat_activate(1);
#endif
}

static void
ipnat_flush(void)
{
	int fd;

	fd = open(IPL_NAT, O_RDWR);
	if (fd <= 0)
		return;
	/* flush all rule */
	flushtable(fd, OPT_CLEAR|OPT_FLUSH);
	close(fd);
}

void
ipnat_deinit(void)
{
#ifdef IPF_FASTNAT
	ipnat_fastnat_activate(0);
#endif
	ipnat_flush();
}
