/*
 * ppp scripts
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ppp.c,v 1.15 2010-08-04 10:57:50 Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <rc.h>
#include <netdb.h>
//roy add
#include <iflib.h>  
#include <rc.h>
#include <router_net.h>
#include "pi_common.h"
//
void pppoe_start(char *pppname);
void pppoe_stop(char *pppname);

#ifdef __CONFIG_CHINA_NET_CLIENT__
extern int pppoe_auth_encrypt_type;
extern int pppoe_auth_success_num;
#endif
/*
* parse ifname to retrieve unit #
*/
int
ppp_ifunit(char *ifname)
{
	if (strncmp( "ppp", ifname, 3))
		return -1;
	if (!isdigit(ifname[3]))
		return -1;
	return atoi(&ifname[3]);
}

/*
 * Called when link comes up
 */
int
ipup_main(int argc, char **argv)
{
	char *wan_ifname = getenv("IFNAME");
	char *value;
	char buf[256];
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int s;
	struct sockaddr_in *addrp;
	struct ifreq ifr;

	printf("%s\n", argv[0]);

	if (wan_ifname == NULL)
		return -1;

	if ((unit = ppp_ifunit(wan_ifname)) < 0)
		return -1;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* Setup local ip */
	if ((value = getenv("IPLOCAL"))) {
		tenda_ifconfig(wan_ifname, IFUP, value, "255.255.255.254");
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.254");
	}

	/* Setup remote ip */
	if ((value = getenv("IPREMOTE")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);
	else
		return -1;

	/* Set destination address */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	strcpy(ifr.ifr_name, wan_ifname);
	addrp = (struct sockaddr_in *)&ifr.ifr_dstaddr;
	memset(addrp, 0, sizeof(*addrp));
	addrp->sin_family = AF_INET;
	addrp->sin_len = sizeof(*addrp);
	addrp->sin_addr.s_addr = inet_addr(value);
	if (ioctl(s, SIOCSIFDSTADDR,  &ifr) < 0) {
		printf("Can't set %s destination address %s!\n", wan_ifname, value);
	}
	close(s);

	strcpy(buf, "");
	if (getenv("DNS1")){
		sprintf(buf, "%s", getenv("DNS1"));
	}
	if (getenv("DNS2")){
		sprintf(buf + strlen(buf), "%s%s", strlen(buf) ? " " : "", getenv("DNS2"));
	}
	
	wan_dns1[0]='\0';
	wan_dns2[0]='\0';
	del_ns(strcat_r(prefix, "autodns", tmp));
	
	int dns_fix = atoi(nvram_safe_get(strcat_r(prefix, "dns_fix", tmp)));
	if(dns_fix)
	{
		add_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)), NULL, ADD_NS_PUSH);
		sscanf(nvram_safe_get(strcat_r(prefix, "dns", tmp)),"%s %s",wan_dns1,wan_dns2);
		nvram_set(strcat_r(prefix, "autodns", tmp), nvram_safe_get(strcat_r(prefix, "dns", tmp)));
	}
	else
	{
		add_ns(buf, NULL, ADD_NS_PUSH);
		strcpy(wan_dns1,getenv("DNS1")?getenv("DNS1"):"");
		strcpy(wan_dns2,getenv("DNS2")?getenv("DNS2"):"");
		nvram_set(strcat_r(prefix, "autodns", tmp), buf);
	}
	
	wan_up(wan_ifname);

#if defined(__CONFIG_PPTP__) || defined(__CONFIG_L2TP__) || defined(__CONFIG_PPPOE2__)
	update_tunnel_firewall(wan_ifname);
#endif	/* __CONFIG_PPTP__ || __CONFIG_L2TP__ */

	printf("done\n");
	return 0;
}

/*
 * Called when link goes down
 */
int
ipdown_main(int argc, char **argv)
{
	char *wan_ifname = getenv("IFNAME");
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;
	char new_dns[256];		
	
	char autodns[256];
	
	if (wan_ifname == NULL)
		return -1;

	/* remove dns which assign from server */
	if ((unit = ppp_ifunit(wan_ifname)) < 0)
		return -1;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	strcpy(autodns, nvram_safe_get(strcat_r(prefix, "autodns", tmp)));
	del_ns(autodns);
	
	wan_down(wan_ifname);

#if defined(__CONFIG_PPTP__) || defined(__CONFIG_L2TP__) || defined(__CONFIG_PPPOE2__)
	update_tunnel_firewall(wan_ifname);
#endif	/* __CONFIG_PPTP__ || __CONFIG_L2TP__ */

	return 0;
}

/*
 * Start function used when the WAN mode
 * set to pppoe.
 */
int
wan_pppoe_start(int unit)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_ifname;
	char *pppname;

	WAN_DBG("%s::Connecting to PPPOE server, please wait...\n", __FUNCTION__);

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
	if (!wan_ifname) {
		printf("%s: no interface\n", __func__);
		return -1;
	}

	ifscrub(wan_ifname);

	tenda_ifconfig(wan_ifname, IFUP, NULL, NULL);

	/* ppp interface name is referenced from this point on */
	pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	
	
	if (strlen(pppname) == 0) {
		printf("No PPP ifname set!\n");
		return -1;
	}

/*add by yp 2016-3-17,for the information :"bad user name or password" when we use the correct user name and password */
#ifdef __CONFIG_CHINA_NET_CLIENT__
	pppoe_auth_encrypt_type	= 0;
	pppoe_auth_success_num = 0;/* wan口每次UP都对记录保存成功的账号密码的拨号次数清零 */
#endif

	pppoe_start(pppname);

	return 0;
}

//hqw for pppoe检测
int
wan_pppoe_start2()
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_ifname;
	char *pppname;

	WAN_DBG("%s::Connecting to PPPOE server, please wait...\n", __FUNCTION__);

	snprintf(prefix, sizeof(prefix), "wan%d_", 0);
	wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
	if (!wan_ifname) {
		printf("%s: no interface\n", __func__);
		return -1;
	}

	//ifscrub(wan_ifname);

	tenda_ifconfig(wan_ifname, IFUP, NULL, NULL);

	/* ppp interface name is referenced from this point on */
	pppname = nvram_safe_get(strcat_r("wan1_", "pppoe_ifname", tmp));
	
	
	if (strlen(pppname) == 0) {
		printf("No PPP ifname set!\n");
		return -1;
	}
	pppoe_start(pppname);

	return 0;
}

#ifdef __CONFIG_PPPOE_SERVER__
/*****************************************************************************
 函 数 名  : init_lan_pppoe_service
 功能描述  : 初始化启动pppoe server
 输入参数  : 
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月23日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void init_lan_pppoe_service(void)
{

	/*判断是否启动pppoe sever*/
	if((nvram_match(SYSCONFIG_WORKMODE, "route") 
		||nvram_match(SYSCONFIG_WORKMODE, "wisp"))
		&& nvram_match("restore_quick_set","1") 
		&& nvram_match("wan0_pppoe_username", "") && nvram_match("wan0_pppoe_passwd", ""))
	{
		set_synchro_type(AUTO_SYNCHRO);
		lan_pppoe_start(0);	
	}
	else
		set_synchro_type(MANUAL_SYNCHRO);
    return ;
}


int
lan_pppoe_start(int unit)//gong  拉起lan pppoe server
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *lan_ifname;
	char *pppname;

	WAN_DBG("%s::Start PPPOE server, please wait...\n", __FUNCTION__);

	snprintf(prefix, sizeof(prefix), "%s", "lan_");
	lan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
	if (!lan_ifname) {
		printf("%s: no interface\n", __func__);
		return -1;
	}

	ifscrub(lan_ifname);

	tenda_ifconfig(lan_ifname, IFUP,nvram_safe_get("lan_ipaddr"),nvram_safe_get("lan_netmask") );

	/* ppp interface name is referenced from this point on */
	pppname = nvram_safe_get("lan0_pppoe_ifname");
	
	
	if (strlen(pppname) == 0) {
		printf("No PPP ifname set!\n");
		return -1;
	}
	/* Save wan status */
	pppoe_start(pppname);

	printf("%s::start lan-pppoe-server sucess!!!\n", __FUNCTION__);

	return 0;
}
#endif

/*
 * Stop function used when the WAN mode
 * set to pppoe.
 */
void
wan_pppoe_down(char *pppname)
{
	if (ppp_ifunit(pppname) < 0) {
		printf("%s is not a ppp interface\n", pppname);
		return;
	}

	/* Stop PPPoE */
	pppoe_stop(pppname);
	return;
}
void
wan_pppoe_down2(char *pppname)
{
	if (ppp_ifunit(pppname) < 0) {
		printf("%s is not a ppp interface\n", pppname);
		return;
	}

	/* Stop PPPoE */
	pppoe_stop(pppname);
	return;
}

#ifdef __CONFIG_PPPOE_SERVER__
void
lan_pppoe_down(char *pppname)
{
	if (ppp_ifunit(pppname) < 0) {
		printf("%s is not a ppp interface\n", pppname);
		return;
	}

	/* Stop PPPoE */
	pppoe_stop(pppname);
	return;
}
#endif
