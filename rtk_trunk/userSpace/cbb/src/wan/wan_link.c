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
#include <sys/syslog.h>
#include <wlutils.h>
#include "pi_common.h"
#include "rc_module.h"
#include "sys_module.h"

#if defined(__CONFIG_WAN_PORT__)
#define WAN_PORT_NUM __CONFIG_WAN_PORT__
#else
#define WAN_PORT_NUM 0
#endif

extern RET_INFO msg_send(PIU8 center,PIU8 id,PI8 *msg);
extern int wan_primary_ifunit(void);
extern int wl_probe(char *name);
extern void start_wan(void);
extern void stop_wan(void);
#ifdef __CONFIG_IPV6__
extern void wan6_start();
extern void wan6_stop();
#endif



//llm add
#ifdef REALTEK

#if 0
struct phystatus {
	int ps_port;
	union {
		int baudrate;
		int linkstate;
		int speedduplex;
	} port_state;
#define ps_baud		port_state.baudrate
#define ps_link		port_state.linkstate
#define ps_speed	port_state.speedduplex
};


#define PORT_SPEED_AUTO         -1
#define PORT_SPEED_10HALF       0
#define PORT_SPEED_10FULL       1
#define PORT_SPEED_100HALF      2
#define PORT_SPEED_100FULL      3

#endif

#define RTL8651_IOCTL_GETWANLINKSPEED 	2100
#define RTL8651_IOCTL_SETWANLINKSPEED 	2101
#define RTL8651_IOCTL_GETETHERLINKDUPLEX 2104
#define RTL8651_IOCTL_SETETHERLINKDUPLEX 2109

#define TENDA_RTL8651_IOCTL_SETWANLINKSPEED 2110



/* IOCTL system call */
static int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	unsigned int args[4];
	struct ifreq ifr;
	int sockfd;

	args[0] = arg0;
	args[1] = arg1;
	args[2] = arg2;
	args[3] = arg3;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("fatal error socket\n");
		diag_printf("%s:%d\n",__FILE__,__LINE__);
		return -3;
	}

	strcpy((char*)&ifr.ifr_name, name);
	((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;

	//this IOCTL defined at wrapper/v3.0/include/wireless.h, but ethernet will use it
	//always define it!
#define SIOCDEVPRIVATE	0x89F0	/* to 89FF */

	if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
	{
		/* 1)enable upnp, 2)chariot/bt test cause console busy, upnp will print these error */
		//perror("device ioctl:");
		close(sockfd);
		diag_printf("error %s:%d \n",__FILE__,__LINE__);
		return -1;
	}
	close(sockfd);
	return 0;
}

/* llm add, 无返回值，驱动里总是返回success, 没有意义 */
void set_wan_speed(char *wan_ifname, int speed, int duplex)
{
	if(speed < 0 || duplex < 0)
	{
		printf("%s():%d, error, invalid args!\n", __func__, __LINE__);
		return;
	}
	
	re865xIoctl(wan_ifname, TENDA_RTL8651_IOCTL_SETWANLINKSPEED, 0, 
			(unsigned int)speed, (unsigned int)duplex) ;
}

static int getWanSpeed(char *ifname)
{        
	unsigned int    ret;        
	unsigned int    args[0];        
	re865xIoctl(ifname, RTL8651_IOCTL_GETWANLINKSPEED, (unsigned int)(args), 0, (unsigned int)&ret) ;
	//printf("%s(%d), ret=%d\n",__FUNCTION__, __LINE__, ret);//Added for test
	//syslog(LOG_INFO, "%s(%d) getWanSpeed(), ret=%d",__FUNCTION__, __LINE__, ret);//Added for test
	return ret;
}

static int getWanDuplex(char *ifname)
{
    unsigned int    ret = __CONFIG_WAN_PORT__;
    unsigned int    args =0;

    re865xIoctl(ifname, RTL8651_IOCTL_GETETHERLINKDUPLEX, (unsigned int)(args), 0, (unsigned int)&ret);
	//printf("<%s:%d>ret=%d\n", __FUNCTION__,__LINE__,ret);
    if(ret==0)
    	return 0;
    else 
    	return 1;
}

#endif

int
ifr_link_check(char *ifn)
{
	struct phystatus ps;

#ifdef CONFIG_VLAN
	uint boardflags;
	char wan_ifname[256] = {0}, name[256] = {0}, *lan_ifnames = NULL, *next = NULL;
	char vports_var[256] = {0}, buf[256] = {0}, *bufp = NULL;
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

#ifdef BCM
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
	value = nvram_get("vlan2ports");
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
#else
#ifdef REALTEK
/* llm add, 从上面函数的实现来看这个函数只是获取wan口连接速度，故下面只考虑wan口 */
int ifr_link_speed(char *ifn)
{
	int speed, duplex_mode;
	
	speed = getWanSpeed(ifn);
	duplex_mode = getWanDuplex(ifn);
	if(speed < 0 || duplex_mode < 0)
	{
		return PORT_SPEED_100FULL;
	}
	if(duplex_mode == 0)	//Half
	{
		if(speed == 0)	//10M
			return PORT_SPEED_10HALF;
		else
			return PORT_SPEED_100HALF;
	}
	else
	{
		if(speed == 0)
			return PORT_SPEED_10FULL;
		else
			return PORT_SPEED_100FULL;
	}
}
#endif
#endif
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

/* llm add, 设置wan口连接速度 */
int ifr_set_link_speed2(int speed)
{
	char wan_ifnames[32];
	
	strncpy(wan_ifnames, nvram_safe_get("wan_ifnames"), sizeof(wan_ifnames));
	/* 页面传过来的值和PORT_SPEED_10HALF定义对不上号，下面做简单处理 */
	speed = speed - 1;
	switch(speed)
	{
		case PORT_SPEED_10HALF:
			set_wan_speed(wan_ifnames, 10, 0);
			break;
		case PORT_SPEED_10FULL:
			set_wan_speed(wan_ifnames, 10, 1);
			break;
		case PORT_SPEED_100HALF:
			set_wan_speed(wan_ifnames, 100, 0);
			break;
		case PORT_SPEED_100FULL:
			set_wan_speed(wan_ifnames, 100, 1);
			break;
		default:
			set_wan_speed(wan_ifnames, 0, 1);
			break;
	}
	
	return 0;
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
	
	if (nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
	{
		return 0;
	}
	if (nvram_match(SYSCONFIG_WORKMODE, "bridge") && nvram_match("wan0_ifname","eth0"))
	{
		//AP模式使用vlan实现时，任何lan/wan口插上网线都启动dhcpc
		if(wan_link_status(WAN_PORT_NUM) || wan_link_status(1) || wan_link_status(2) || wan_link_status(3))
		{
			return 1;
		}
	}
 
	//wisp模式的时候检测无线与上级的链路连接情况
	if(nvram_match(SYSCONFIG_WORKMODE, "wisp"))
	{
		if(1 == getWlanLink(wan_name))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
 
	return (wan_link_status(WAN_PORT_NUM) ? 1 : 0); 
}

static int chk_link = 0;

int wan_link_check(void)
{
	static int link_check_cnt = 0;
	int link;

	link = wan_link();
	if (link != chk_link)
	{
		link_check_cnt++;
		if (link_check_cnt < 3)
			return chk_link;

		link_check_cnt = 0;
	}
	else 
	{
		link_check_cnt = 0;
		return chk_link;
	}

	/* link up */
	if (chk_link == 0 && link) 
	{
		printf("wan link up!\n");
		syslog(LOG_INFO,"wan up!");
		if(nvram_match(SYSCONFIG_WORKMODE, "route") 
			|| nvram_match(SYSCONFIG_WORKMODE, "wisp"))
		{
			msg_send(MODULE_RC,RC_WAN_MODULE,"op=1");
		}
#ifdef __CONFIG_IPV6__
		wan6_start();
#endif

	}
	/* link down */
	else if (chk_link == 1 && link == 0) 
	{
#ifdef __CONFIG_IPV6__		
		wan6_stop();
#endif
		printf("wan link down!\n");
		syslog(LOG_INFO,"wan down!");
		if(nvram_match(SYSCONFIG_WORKMODE, "route") 
			|| nvram_match(SYSCONFIG_WORKMODE, "wisp"))
		{
			msg_send(MODULE_RC,RC_WAN_MODULE,"op=2");
		}

	}

	chk_link = link;

	return chk_link;
}

/* ldf revise temporary, solve compile error. */
int
tenda_wan_link_status(void)
{
	return chk_link;
}
#endif /* __CONFIG_NAT__ */
