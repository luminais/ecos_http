/*
 * DHCP server ecos main entrance.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd_ecos.c,v 1.12 2010-08-27 10:04:59 Exp $
 */

#include <dhcpd.h>
#include <bcmnvram.h>
#include "../../tc/tc.h"

extern int loipc(char *cmd, int sport, int dport, char *resp, int *rlen);

/*
 * Functions needed by dhcpd.c
 */
char *
dhcpd_nvram_get(char *name, int unit, char *buf)
{
	char temp[100];
	char var[100];
	char *p;
	char *value;

	/* Get value of unit 0 directly */
	if (unit == 0) {
		strcpy(var, name);
	}
	else {
		/* Set default return value */
		buf[0] = 0;

		/* Search for '_' */
		strcpy(temp, name);
		if ((p = strchr(temp, '_')) == 0)
			return 0;
		*p++ = 0;

		/* Prinet new name */
		sprintf(var, "%s%d_%s", temp, unit, p);
	}

	value = nvram_get(var);
	if (value == 0 || strlen(value) == 0)
		return 0;

	strcpy(buf, value);
	return buf;
}

/* Delete an ARP entry */
void
dhcpd_osl_arpdel(struct in_addr ipaddr)
{
	/* delete ARP entry */
	int s;
	struct ecos_arpentry req;

	memset(&req, 0, sizeof(req));
	req.addr = ipaddr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
		return;

	//ioctl(s, SIOCDELARPRT, &req);//hqw add for arp

	close(s);
	return;
}

/* Get an ARP entry */
//heqiwei ARP表
void
dhcpd_osl_arpget(struct in_addr ipaddr, unsigned char *mac)
{
	/* Read ARP to MAC */
	int s;
	struct ecos_arpentry req;

	memset(&req, 0, sizeof(req));
	req.addr = ipaddr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
		return;

	ioctl(s, SIOCGARPRT, &req);

	memcpy(mac, req.enaddr, 6);

	close(s);
	return;
}

/* Get gateway ipaddr, otherwise lan ipaddr  */
int
dhcpd_osl_gateway_list(DHCPD_IFC *ifp, char *buf)
{
	char value[256];
	struct in_addr sa;

	//if (dhcpd_nvram_get("dhcp_gateway", ifp->unit, value) == 0) {
	//	if (dhcpd_nvram_get("lan_ipaddr", ifp->unit, value) == 0)
	if (dhcpd_nvram_get("lan_ipaddr", ifp->unit, value) == 0){
		if (dhcpd_nvram_get("dhcp_gateway", ifp->unit, value) == 0)
			value[0] = 0;
	}

	if (inet_aton(value, &sa) == 0 || sa.s_addr == 0)
		return -1;

	/* Copy the return value */
	strcpy(buf, value);
	return 0;
}

/* Get DNS list */
int
dhcpd_osl_dns_list(DHCPD_IFC *ifp, char *buf)
{
	buf[0] = 0;

	/*
	 * Don't enable DNS masquerate when
	 * router mode is not enabled.
	 */
#ifdef	__CONFIG_APCLIENT_DHCPC__
#ifdef __CONFIG_DNS_8_8_8_8_SUPPORT__
	sprintf(buf,"%s %s",inet_ntoa(ifp->ipaddr),"8.8.8.8");//hqw for DNS
#else
	sprintf(buf,"%s",inet_ntoa(ifp->ipaddr));//hqw for DNS
#endif/*__CONFIG_DNS_8_8_8_8_SUPPORT__*/
#else
#ifndef __CONFIG_TENDA_HTTPD_V3__//tenda remove  gong modify to 1
	if (!nvram_match("router_disable", "1")) {
		/* Use the router ip as the dns server */
		//strcpy(buf, inet_ntoa(ifp->ipaddr));
	
		#ifdef  __CONFIG_TENDA_HTTPD_NORMAL__
			strcpy(buf, nvram_safe_get("lan_dns"));
		#else
		#ifdef __CONFIG_DNS_8_8_8_8_SUPPORT__
			sprintf(buf,"%s %s",inet_ntoa(ifp->ipaddr),"8.8.8.8");
		#else
			sprintf(buf,"%s",inet_ntoa(ifp->ipaddr));
		#endif/*__CONFIG_DNS_8_8_8_8_SUPPORT__*/
		#endif/*__CONFIG_TENDA_HTTPD_NORMAL__*/

	}
	else 
#endif/*__CONFIG_TENDA_HTTPD_V3__*/
	{
		strcpy(buf, nvram_safe_get("resolv_conf"));
	}
#endif/*__CONFIG_APCLIENT_DHCPC__*/
					
#if 1//tenda add 
	if(strlen(buf)<4/*simple check*/){
		if (!nvram_match("router_disable", "1")) {
			/* Use the router ip as the dns server */
			strcpy(buf, inet_ntoa(ifp->ipaddr));
		}
	}
#endif
	
	/* Check the dns list string */
	if (buf[0] == 0)
		return -1;

	return 0;
}
//roy+++ 2010/09/03
int dhcpd_osl_static_lease(DHCPD_IFC *ifp, char *buf,int index)
{
	char value[100];
	char name[128];

	snprintf(name,sizeof(name),"dhcp_static_lease%d",index);
	if (dhcpd_nvram_get(name, ifp->unit, value) == 0) {
		value[0] = 0;
	}

	/* Copy the return value */
	strcpy(buf, value);
	return 0;
}
/*
roy ++ for:
lan ip 可以改成已被静态绑定的ip，重启后造成ip冲突
*/
int dhcpd_osl_lan_ip(DHCPD_IFC *ifp, char *buf)
{
	char *value;

	value = nvram_get("lan_ipaddr");
	if (value == 0 || strlen(value) == 0)
		return 0;
	
	/* Copy the return value */
	strcpy(buf, value);
	return 1;
}
//roy+++

/* Get domain name */
int
dhcpd_osl_get_domain(DHCPD_IFC *ifp, char *valstr)
{
	char buf[256];
	char *value;

	if (dhcpd_nvram_get("dhcp_domain", ifp->unit, buf) && strcmp(buf, "lan") == 0) {
		value = nvram_get("lan_domain");
	}
	else {
		value = nvram_get("wan_domain");
	}

	/* Check the return value */
	if (value && strlen(value) > 0) {
		strcpy(valstr, value);
		return 0;
	}

	return -1;
}

/* 
 * Read configuration from NVRAM to
 * dhcpd interface structure.
 */

/*
 * Set interface ipaddr
 */
static int
dhcpd_getifaddr(char *ifname, struct in_addr *ipaddr, struct in_addr *netmask)
{
	int s;
	struct ifreq ifr;
	int error = -1;

	s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (s < 0)
		return -1;

	/* Retrieve settings */
	memset(&ifr, 0, sizeof(struct ifreq));

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, ifname);

	/*
	 * Upper layer want to test the interface
	 * status by this ioctl, call anyway.
	 */
	if (ioctl(s, SIOCGIFADDR, &ifr) != 0)
		goto quit;

	/* get ip */
	if (ipaddr)
		*ipaddr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

	/* get mask */
	if (netmask) {
		if (ioctl(s, SIOCGIFNETMASK, &ifr) != 0)
			goto quit;

		*netmask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	}

	/* Return with no error */
	error = 0;
quit:
	if (error) {
		if (ipaddr)
			ipaddr->s_addr = 0;
		if (netmask)
			netmask->s_addr = 0;
	}

	close(s);
	return error;
}

static int lease_time_user_value;

int
dhcpd_osl_init_config(DHCPD_IFC *ifp)
{
	struct lease_pool *pool = ifp->container.pool;
	struct in_addr start;
	struct in_addr end;

	unsigned int lan_ip;
	unsigned int lan_mask;
	unsigned int lan_net;
	unsigned int bufip[4];
	int use_default = 0;
	char buf[256];
	int ival;

	if (dhcpd_nvram_get("lan_proto", ifp->unit, buf) == 0 ||
	    strcmp(buf, "dhcp") != 0)
		return -1;

	/* we use first listen interface as server IP */
	if (dhcpd_getifaddr(ifp->ifname, &ifp->ipaddr, &ifp->netmask) != 0)
		return -1;

	lan_ip = ntohl(ifp->ipaddr.s_addr);		/* lan_ip, lan1_ip */
	lan_mask = ntohl(ifp->netmask.s_addr);		/* lan_mask, lan1_mask */
	lan_net = (lan_ip & lan_mask);

	/* start address */
	ifp->container.pool_num = 1;
#if 0//roy modify
	if (dhcpd_nvram_get("dhcp_start", ifp->unit, buf))
		inet_aton(buf, &start);
	else
		inet_aton(DEFAULT_START, &start);

	pool->start.s_addr = htonl(lan_net | (ntohl(start.s_addr) & ~lan_mask));

	/* end address */
	if (dhcpd_nvram_get("dhcp_end", ifp->unit, buf))
		inet_aton(buf, &end);
	else
		inet_aton(DEFAULT_END, &end);

	pool->end.s_addr = htonl(lan_net | (ntohl(end.s_addr) & ~lan_mask));
#else
	if (dhcpd_nvram_get("dhcp_start", ifp->unit, buf)){
		if (sscanf(buf, "%d.%d.%d.%d", &bufip[0], &bufip[1], &bufip[2], &bufip[3]) != 4){
			use_default = 1;
			goto pool_default;
		}
		pool->start.s_addr = htonl((bufip[0] << 24) | (bufip[1] << 16) | (bufip[2] << 8) | bufip[3]);
	}else{
		use_default = 1;
		goto pool_default;
	}
	if (dhcpd_nvram_get("dhcp_end", ifp->unit, buf)){
		if (sscanf(buf, "%d.%d.%d.%d", &bufip[0], &bufip[1], &bufip[2], &bufip[3]) != 4){
			use_default = 1;
			goto pool_default;
		}
		pool->end.s_addr = htonl((bufip[0] << 24) | (bufip[1] << 16) | (bufip[2] << 8) | bufip[3]);
	}else{
		use_default = 1;
		goto pool_default;
	}
pool_default:
	if(use_default){
		inet_aton(DEFAULT_START, &start);
		pool->start.s_addr = htonl(lan_net | (ntohl(start.s_addr) & ~lan_mask));
		inet_aton(DEFAULT_END, &end);
		pool->end.s_addr = htonl(lan_net | (ntohl(end.s_addr) & ~lan_mask));
	}
#endif

	/* lease time */
	if (dhcpd_nvram_get("lan_lease", ifp->unit, buf))
		ival = atoi(buf);
	else
		ival = DEFAULT_LEASE_TIME;
//diag_printf("[%s]:: ival=%d\n",__FUNCTION__, ival);
	if(lease_time_user_value == 0){//为了获得WAN口DNS
		lease_time_user_value = ival;
		#ifndef __CONFIG_AUTO_CONN__
		if(!nvram_match("wan0_connect","Connected"))
			ival = 30;
		#endif
	}
		
	ifp->container.lease_time = ival;
	ifp->container.lease_min = DEFAULT_LEASE_MIN_TIME;
	ifp->container.offer_time = DEFAULT_LEASE_MIN_TIME;
	ifp->container.reserve_time = DEFAULT_RESERVE_TIME;
	return 0;
}

int
dhcpd_osl_renew_lease(DHCPD_IFC *ifp)
{
	if(lease_time_user_value !=0)
		ifp->container.lease_time = lease_time_user_value;
	return 0;
}

int
dhcpd_osl_ifname_list(char *ifname_list)
{
	int i;
	char lan_ifname[256];
	char buf[256];

	ifname_list[0] = 0;

	/* Cat all lan interface together */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (dhcpd_nvram_get("lan_ifname", i, lan_ifname) &&
		    dhcpd_nvram_get("lan_proto", i, buf) &&
		    strcmp(buf, "dhcp") == 0) {
			/* Check interface */
			if (dhcpd_getifaddr(lan_ifname, 0, 0) != 0)
				continue;

			if (ifname_list[0] != 0)
				strcat(ifname_list, " ");

			/* For example, the ifnames_list will be 0=br0 1=br1 ... */
			sprintf(buf, "%d=%s", i, lan_ifname);
			strcat(ifname_list, buf);
		}
	}

	return 0;
}

/*
 * Functions up/down the dhcpd thread.
 */
#define STACK_SIZE 12*1024

cyg_handle_t dhcpd_thread_h = 0;
cyg_thread   dhcpd_thread;
static cyg_uint8 dhcpd_stack[STACK_SIZE];
static int dhcpd_running = 0;
static int dhcpd_down = 0;

/* dhcpd daemon */
void
dhcpd_main(void)
{
	dhcpd_running = 1;

	/* Enter os independent main loop */
	dhcpd_mainloop();

	dhcpd_running = 0;
	dhcpd_down = 1;
	return;
}

/* Terminate dhcpd */
void
dhcpd_stop(void)
{
	int pid;

	/* Check thread status */
	if (oslib_getpidbyname("DHCP_server") == 0)
		return;

	/* Issue stop event */
	if (loipc("STOP", 0, DHCPD_IPC_PORT, 0, 0) < 0)
		return;

	/* Wait until thread exit */
	pid = oslib_getpidbyname("DHCP_server");
	if (pid) {
		while (oslib_waitpid(pid, NULL) != 0)
			cyg_thread_delay(1);
	}
	return;
}

/* renew lease time */
void
dhcpd_renew_lease(void)
{
	char cmd[64]={0};
	char *ifname;

	ifname = nvram_get("lan_ifname");
	
	if (!ifname)
		return ;
	
	/* Check thread status */
	if (oslib_getpidbyname("DHCP_server") == 0)
		return;

	sprintf(cmd, "RENEW_LEASE %s", ifname);

	/* Issue stop event */
	if (loipc(cmd, 0, DHCPD_IPC_PORT, 0, 0) < 0)
		return;

	return;
}

#ifdef __CONFIG_APCLIENT_DHCPC__
extern int gpi_get_apclient_dhcpc_addr(char *ip,char *mask);
#endif

/* Initialize dhcpd */
int
dhcpd_start(void)
{
	/*
	 * Don't enable DHCPD when
	 * router mode is disable.
	 */
#ifndef __CONFIG_APCLIENT_DHCPC__
	if (nvram_match("router_disable", "1")) {
		/* Terminate it anyway */
		dhcpd_stop();
		return -1;
	}
#endif

#ifdef __CONFIG_APCLIENT_DHCPC__
	char ip[17] = {0},mask[17] = {0};
	gpi_get_apclient_dhcpc_addr(ip,mask);
	if(0 != strcmp(ip,"") && 0 != strcmp(mask,""))
	{
		return -1;
	}
#endif

	lease_time_user_value = 0;
	
	if (dhcpd_running == 0) {
		dhcpd_down = 0;
		cyg_thread_create(
			8,
			(cyg_thread_entry_t *)dhcpd_main,
			0,
			"DHCP_server",
			dhcpd_stack,
			sizeof(dhcpd_stack),
			&dhcpd_thread_h,
			&dhcpd_thread);
		cyg_thread_resume(dhcpd_thread_h);

		/* Wait until thread scheduled */
		while (!dhcpd_running && !dhcpd_down)
			cyg_thread_delay(1);
	}

	return 0;
}

/* DHCPD event here */
struct lease_t *
dhcpd_lease_dump(char *ifname)
{
	char cmd[64] = {0};
	struct lease_t *dump;
	int len;

	/* Check thread status */
	if (oslib_getpidbyname("DHCP_server") == 0)
		return NULL;

	/* Allocate buffer to dump, a C class need less than 24K bytes */
	len = sizeof(struct lease_t) * 253;
	dump = (struct lease_t *)malloc(len);
	if (dump == NULL)
		return NULL;

	/* Write event */
	sprintf(cmd, "LEASE DUMP %s", ifname);
	if (loipc(cmd, 0, DHCPD_IPC_PORT, (char *)dump, &len) < 0) {
		free(dump);
		return NULL;
	}

	return dump;
}

int show_lan_leases(void *wp, void (*dhcpd_entry2)(char *buf, int id, void *arg1))
{
	int i;
	int index;
	int ret = 0;

	char *ifname,lease_buf[128];
	struct lease_t *lease, *dump;
	int last,n,s;

	
	ifname = nvram_get("lan_ifname");
	
	if (!ifname)
		return 0;

	/* dump lease */
	dump = lease = dhcpd_lease_dump(ifname);
	if (!dump)
		return 0;	

	/* Write HTML until last */
	last = 0;
	index=1;
	while (last == 0) {
		/* Get the last flag for next time qualification */
		last = lease->last;

		if (lease->flag & RESERVED) {
			lease++;
			continue;
		}
//tenda modify
		if(lease->expiry < (unsigned long) time(0) && !(lease->flag & STATICL))
		{
			lease++;
			continue;
		}
//++end
		n = 0;
		s = 0;
		
		n = sprintf(&lease_buf[s], "%s;", lease->hostname);
		s+=n;

		n = sprintf(&lease_buf[s], "%s;", inet_ntoa(lease->ipaddr));
		s+=n;
		
		for (i = 0; i < 6; i++) {
			n = sprintf(&lease_buf[s], "%02X", lease->mac[i]);
			s+=n;
			if (i != 5){
				n = sprintf(&lease_buf[s],":");
				s+=n;
			}
		}

		n = sprintf(&lease_buf[s], ";%d;", lease->flag);
		s+=n;

		n = sprintf(&lease_buf[s], "%u",(lease->expiry-time(0)));

		if (dhcpd_entry2)
			dhcpd_entry2(lease_buf, index, wp);
		/* Advaince to next */
		lease++;
		index++;
	}

	/* Free buffer */
	free(dump);
	return ret;
}
