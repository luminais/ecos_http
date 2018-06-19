/*
 * Get filter rules and assign to ipfilter engine.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: filcmd.c,v 1.11 2010-08-19 08:51:39 Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if_var.h>

#include "ip_filfast.h"
#include "macf.h"
#include "urlf.h"
#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include "netinet/ip_nat.h"
#include "ipf.h"
//#include "ip_defense.h"

#include <iflib.h>
#include <bcmnvram.h>

/* foreach need */
#include <shutils.h>

#include <netconf.h>
#include <nvparse.h>

#include <router_net.h>//roy+++,2010/09/20

#define FLT_MAC_MAX_NUM		50
#define FLT_URL_MAX_NUM		50
#define FLT_CLN_MAX_NUM		50
#define FLT_PKT_MAX_NUM		50
#define FLT_DNS_MAX_NUM		50

//struct rate_limit sync_rate;

int opts = 0;

extern int udp_blackhole;
extern int tcp_blackhole;
int drop_portscan;
int auth_port_open;
//roy+++,2010/09/27
unsigned int ip_defense;
//+++
static int firewall_inited;

extern int wan_primary_ifunit(void);
extern int countbits __P((u_32_t));

//roy+++,2010/09/20

enum {
	MODE_DISABLED = 0,//禁用
	MODE_DENY,//仅禁止
	MODE_PASS//仅允许
};

enum {
	STA_UNUSE = 0,
	STA_ADDED,
	STA_DELETED
};
//+++

//add by ll
#define NIS_FASTCHECK_ENABLE 1 

enum {
	RECORD_MODE_DISABLED = 0,//禁用
	RECORD_MODE_ENABLE,//启用
	RECORD_MODE_UNDEFINE
};
//end by ll


static int mac_fil_status[MAX_NVPARSE];
static int client_fil_status[MAX_NVPARSE];
#ifdef __CONFIG_URLFILTER__
static int url_fil_status[MAX_NVPARSE];
#endif

static int client_default_action = 0;
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
extern int init_backlist(void);
extern void parent_control_config_update(void);
int mac_filter_mode = 0;
int url_filter_mode = 0;
#endif 

#ifdef __CONFIG_AL_SECURITY__

void al_security_start(int flag)
{
	int fd = -1;
	int mode= -1 ;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
	{
		printf("open IPL_NAME failed\n");
		return;
	}

	
	mode = flag;
	
	printf("#######al_security_start flag:%d\n" , mode);

	
	int rc = ioctl(fd, SIOCALSECURITY, &mode);
	if(rc) 
		printf("Activate al_security failed\n");
	
	close(fd);
	
	return;
}


#endif


	
static int get_filter_mode(char *filter_name)
{
	char value[16];

	strncpy(value, nvram_safe_get(filter_name), sizeof(value));
	if(strcmp(value,"deny") == 0){
		return MODE_DENY;//仅禁止
	}
	else if(strcmp(value,"pass") == 0){
		return MODE_PASS;//仅允许
	}
	else{
		return MODE_DISABLED;//禁用
	}
}
//+++

/* OS layer implementation */
static char *firewall_wanifname(char *wan_ifname)
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

static char *firewall_lanifname(int lanid, char *lan_ifname)
{
	char tmp[100];
	char *value;

	if (lan_ifname == 0)
		return 0;

	/* Get lan interface name */
	if (lanid == 0) {
		value = nvram_get("lan_ifname");
	}
	else {
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

/*
 * Set firewall rule to kernel
 */
void fwaddrule(char *fmt, ...)
{
	va_list args;
	char fwcmd[256];
	struct frentry	*fr;
	int status;

	/* Rebuild nat command */
	va_start(args, fmt);
	vsprintf(fwcmd, fmt, args);
	va_end(args);
	/* Parse command */
	fr = parse(fwcmd, 0, &status);
	if (fr) {
		int fd = open(IPL_NAME, O_RDWR);
		if (fd >= 0) {
			ioctl(fd, SIOCINAFR, (CYG_ADDRWORD)&fr);
			close(fd);
		}
	}
	return;
}

/*
 * Delete firewall rule
 */
void fwdelrule(char *fmt, ...)
{
	va_list args;
	char fwcmd[256];
	struct frentry	*fr;
	int status;

	/* Rebuild nat command */
	va_start(args, fmt);
	vsprintf(fwcmd, fmt, args);
	va_end(args);
	/* Parse command */
	fr = parse(fwcmd, 0, &status);
	if (fr) {
		int fd = open(IPL_NAME, O_RDWR);
		if (fd >= 0) {
			ioctl(fd, SIOCRMAFR, (CYG_ADDRWORD)&fr);
			close(fd);
		}
	}
	return;
}

/*
 * Flush out all firewall rule
 */
void firewall_ruleflush(void)
{
	int fl = 0;
	int fd;

	fd = open(IPL_NAME, O_RDWR);
	if (fd >= 0) {
		fl |= (FR_OUTQUE|FR_INQUE);
		if (ioctl(fd, SIOCIPFFL, (CYG_ADDRWORD)&fl) != 0) {
			perror("ioctl(SIOCIPFFL)");
		}
		close(fd);
	}
}
//roy +++,2010/09/20

void firewall_mac_default_mode()
{
	char mode_name[]="filter_mac_mode";
	int fd, mode;

	if (!firewall_inited)
		return;

	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	//set mac filter mode here
	mode = get_filter_mode(mode_name);
	
	ioctl(fd, SIOCSMACFIL, &mode);

	close(fd);

	return;
}

void firewall_mac_update(void)
{
	/*
	* MAC filter format:
	*	 nvram name: filter_macmode, filter_maclist
	*    [MAC address] [MAC address] [MAC address] .....
	*
	*    ex. "00:11:22:33:44:55 00:11:22:33:44:56 00:11:22:33:44:57 ....."
	*
	*/
	//mac,1-0,3600-0,on,desc
	struct macfilter macinfo;
	char mode_name[]="filter_mac_mode";
	int i, fd = -1, mode = 0;

	time_t now;
	struct tm *time_local;
	uint now_sec;

	if (!firewall_inited)
		return;
	
	//set mac filter mode here
	mode = get_filter_mode(mode_name);
	
	//if (ioctl(fd, SIOCSMACFIL, &mode) != 0)
	//	goto exit;

	if(mode == MODE_DISABLED)
		return;


	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;


	now = time(0);
	time_local = localtime(&now);
	now_sec = time_local->tm_hour * 3600 + time_local->tm_min * 60 + time_local->tm_sec;

	//set mac filter
	/* Loop for all entries */
	for (i = 0; i < MAX_NVPARSE; i++) {
		netconf_mac_t start, end;

		if (get_filter_mac(i, &start, &end) && !(start.match.flags & NETCONF_DISABLED)) {
			memcpy(macinfo.mac, start.mac, ETHER_ADDR_LEN);
			macinfo.next = NULL;
			if (time_local->tm_wday >= start.match.days[0] && time_local->tm_wday <= start.match.days[1] &&
			    ((now_sec >= start.match.secs[0] && now_sec <= start.match.secs[1]) || (start.match.secs[0] == 0 && start.match.secs[1] == 0)/*all time*/)) {
//roy+++,2010/09/20
				if(mac_fil_status[i] == STA_UNUSE || mac_fil_status[i] == STA_DELETED){
//+++
					/* Add rule */
					if (ioctl(fd, SIOCADMACFR, &macinfo) != 0)
						break;
//roy+++,2010/09/20
					mac_fil_status[i] = STA_ADDED;
				}
//+++
			} else {
//roy+++,2010/09/20
				if(mac_fil_status[i] == STA_ADDED){
//+++
					/* Remove rule */
					if (ioctl(fd, SIOCRMMACFR, &macinfo) != 0)
						break;
//roy+++,2010/09/20
					mac_fil_status[i] = STA_ADDED;
				}
//+++
			}
		}
	}


	close(fd);	
}
//+++
void macf_init(void)
{
	memset(mac_fil_status,0,sizeof(mac_fil_status));
	firewall_mac_default_mode();
	firewall_mac_update();
}

void macf_flush(void)
{
	int fd,mac_enable;

	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;
	ioctl(fd, SIOCFLMACFR, NULL);
//roy+++,2010/09/20
	mac_enable = MODE_DISABLED;
	ioctl(fd, SIOCSMACFIL, &mac_enable);
//+++
	close(fd);
}

struct rule_str {
	int str_len;
	char *str;
	struct rule_str *next;
};

void clear_rule(struct rule_str *rule_hash[], int rule_hash_len);
extern int init_data_rule(char *data_rule);
extern void print_data_rule(void);
#define DATA_RULE_HASH_LEN (30)
#define DATA_FILE_HASH_LEN (10)
extern struct rule_str *data_rule_hash[DATA_RULE_HASH_LEN];
extern struct rule_str *data_file_hash[DATA_FILE_HASH_LEN];
extern char url_log_server[1024];
extern char post_server_ip[16];

//add by ll
void nis_fastcheck_mode(int enable)
{
	int fd = -1;
	int mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
	{
		printf("open IPL_NAME failed\n");
		return;
	}

	mode = enable;
	int rc = ioctl(fd, SIOCSNISFASTCHECK, &mode);
	if(rc) 
		printf("Activate fast NAT failed\n");
	
	close(fd);
	return;
}

int start_data_rule(char *data_rule)
{
	clear_rule(data_rule_hash, DATA_RULE_HASH_LEN);
	clear_rule(data_file_hash, DATA_FILE_HASH_LEN);
	init_data_rule(data_rule);
	print_data_rule();

	nis_fastcheck_mode(1);
	return 0;
}

/*void url_record_init(void)
{
	int fd,mode;

	mode = RECORD_MODE_UNDEFINE;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;
	
	ioctl(fd, SIOCSURLRECORDINIT, &mode);

	close(fd);
	
	return;
}*/

void url_record_start(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = RECORD_MODE_ENABLE;//启用

	ioctl(fd, SIOCSURLRECORDMODE, &mode);

	close(fd);
	
	return;
}

void url_record_stop(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = RECORD_MODE_DISABLED;//禁用
	
	ioctl(fd, SIOCSURLRECORDMODE, &mode);

	close(fd);
	
	return;
}

void print_url_log_serv()
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	printf("url_log_server = %s\n", url_log_server);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	return;
}

void url_log_start(char *server_path)
{
	memset(url_log_server, 0x0, sizeof(url_log_server));
	strcpy(url_log_server, server_path);
	memset(post_server_ip, 0x0, sizeof(post_server_ip));
	url_record_start();
	print_url_log_serv();

	return;
}

//end by ll

#if 1
//luminais mark
void js_inject_start(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = 1;//启用

	ioctl(fd, SIOCJSINJECTCHECK, &mode);

	close(fd);
	
	return;
}

void js_inject_stop(void)
{
	int fd,mode;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	mode = 0;//禁用

	ioctl(fd, SIOCJSINJECTCHECK, &mode);

	close(fd);
	
	return;
}
//luminais
#endif

#ifdef __CONFIG_URLFILTER__

void firewall_urlf_default_mode()
{
	int fd,mode;
	char mode_name[]="filter_url_mode";

	if (!firewall_inited)
		return;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	//set mac filter mode here
	mode = get_filter_mode(mode_name);
	
	ioctl(fd, SIOCSURLFIL, &mode);

	close(fd);
	return;
}
/*
 * URL filter format:
 *    [client IP range]:[URL string],[enable]
 * ex. "192.168.2.50-192.168.2.100:urlString0;on"
 */
//192.168.1.7-192.168.1.8:www.baidu.com,1-0,3600-0,on,desc
void firewall_urlf_update(void)
{
	int i;
	struct urlfilter urlinfo;
	int fd = -1,mode = 0 ;//url_enable;
	time_t now;
	struct tm *time_local;
	uint now_sec;
	char mode_name[]="filter_url_mode";

	if (!firewall_inited)
		return;
	
	//set mac filter mode here
	mode = get_filter_mode(mode_name);

	//if (ioctl(fd, SIOCSURLFIL, &mode) != 0)
	//	goto exit;

	if(mode == MODE_DISABLED)
		return;


	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	now = time(0);
	time_local = localtime(&now);
	now_sec = time_local->tm_hour * 3600 + time_local->tm_min * 60 + time_local->tm_sec;

	/* Loop for all entries */
	for (i = 0; i < MAX_NVPARSE; i++) {
		netconf_urlfilter_t start, end;

		if (get_filter_url(i, &start, &end) && !(start.match.flags & NETCONF_DISABLED)) {
			urlinfo.sip = ntohl(start.match.src.ipaddr.s_addr);
			urlinfo.eip = ntohl(end.match.src.ipaddr.s_addr);
			strcpy(urlinfo.url, start.url);
			urlinfo.next = NULL;
			if (time_local->tm_wday >= start.match.days[0] && time_local->tm_wday <= start.match.days[1] &&
			    ((now_sec >= start.match.secs[0] && now_sec <= start.match.secs[1]) || (start.match.secs[0] == 0 && start.match.secs[1] == 0)/*all time*/)) {

//roy+++,2010/09/20
				if(url_fil_status[i] == STA_UNUSE || url_fil_status[i] == STA_DELETED){
//+++
					/* Add rule */
					if (ioctl(fd, SIOCADURLFR, &urlinfo) != 0)
						break;
//roy+++,2010/09/20
					url_fil_status[i] = STA_ADDED;
				}
//+++
			} else {
//roy+++,2010/09/20
				if(url_fil_status[i] == STA_ADDED){
//+++
					/* Remove rule */
					if (ioctl(fd, SIOCRMURLFR, &urlinfo) != 0)
						break;
//roy+++,2010/09/20
					url_fil_status[i] = STA_DELETED;
				}
//+++
			}
		}
	}

	//roy remove
	//url_enable = 1;
	//ioctl(fd, SIOCSURLFIL, &url_enable);


	close(fd);	
}


void firewall_urlf_init(void)
{
	//roy modify,2010/09/20
	memset(url_fil_status,0,sizeof(url_fil_status));
	firewall_urlf_default_mode();
	firewall_urlf_update();
}

void firewall_urlf_flush(void)
{
	int fd, url_enable;

	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;
	ioctl(fd, SIOCFLURLFR, NULL);
	url_enable = MODE_DISABLED;
	ioctl(fd, SIOCSURLFIL, &url_enable);
	close(fd);
}
#endif /* __CONFIG_URLFILTER__ */

void firewall_client_update(void)
{
	/*
	 * Client Filter Format:
	 *	 nvram name: filter_clientXXX
	 *
	 *	 [lanip start]-[lanip end]:[port star]-[port end],[protocol],
	 *	 [from day]-[to day],[from hour]-[to hour],[enable]
	 *
	 *    ex. "192.168.1.103-192.168.1.105:80-90,tcp,1-1,0-86399,on"
	 */
	int i,mode;
	char wan_ifname[IFNAMSIZ];
	char sip[] = "255.255.255.255", eip[] = "255.255.255.255";
	char *filter_cmd[]={"","block out","pass out"};
	char mode_name[]="filter_client_mode";
	char portbuf[32];
	time_t now;
	struct tm *time_local;
	uint now_sec;

	if (!firewall_inited)
		return;

	mode = get_filter_mode(mode_name);

	if(mode == MODE_DISABLED)
		return;

	now = time(0);
	time_local = localtime(&now);
	now_sec = time_local->tm_hour * 3600 + time_local->tm_min * 60 + time_local->tm_sec;

	firewall_wanifname(wan_ifname);
	for (i = 0; i < MAX_NVPARSE; i++) {
		netconf_filter_t start, end;

		if (get_filter_client(i, &start, &end) && !(start.match.flags & NETCONF_DISABLED)) {
			strcpy(sip, inet_ntoa(start.match.src.ipaddr));
			strcpy(eip, inet_ntoa(end.match.src.ipaddr));
			if (start.match.dst.ports[0] != start.match.dst.ports[1] )
				sprintf(portbuf, "port %d >< %d", (ntohs(start.match.dst.ports[0]) -1) <0?0:ntohs(start.match.dst.ports[0])-1, 
											   (ntohs(start.match.dst.ports[1]) +1) >65535?65535:ntohs(start.match.dst.ports[1])+1);
			else
				sprintf(portbuf, "port = %d", ntohs(start.match.dst.ports[0]));
//			diag_printf("tm_day=[%d],match_day0=[%d],match_day1=[%d],now_sec=%d,start.match.secs0=%d,secs1=%d\n",
//				time_local->tm_wday,start.match.days[0],start.match.days[1],now_sec, start.match.secs[0], start.match.secs[1]);
			if (time_local->tm_wday >= start.match.days[0] && time_local->tm_wday <= start.match.days[1] &&
			    ((now_sec >= start.match.secs[0] && now_sec <= start.match.secs[1]) || (start.match.secs[0] == 0 && start.match.secs[1] == 0)/*all time*/)) {

//roy+++,2010/09/20
				if(client_fil_status[i] == STA_UNUSE || client_fil_status[i] == STA_DELETED){
//+++
					/* Add rule */
					fwaddrule("%s quick on %s proto %s from %s><%s to any %s\n",
						filter_cmd[mode],wan_ifname, /*(start.match.ipproto == IPPROTO_TCP)?"tcp":"udp"*/start.match.ipproto_str,
						sip, eip, portbuf);
//roy+++,2010/09/20
					client_fil_status[i] = STA_ADDED;
				}
//+++
			} else {
//roy+++,2010/09/20
				if(client_fil_status[i] == STA_ADDED){
//+++
					/* Remove rule */
					fwdelrule("%s quick on %s proto %s from %s><%s to any %s\n",
						filter_cmd[mode],wan_ifname, /*(start.match.ipproto == IPPROTO_TCP)?"tcp":"udp"*/start.match.ipproto_str,
						sip, eip, portbuf);
//roy+++,2010/09/20
					client_fil_status[i] = STA_DELETED;
				}
//+++
			}
		}
	}
	if(!client_default_action){
		if(mode == MODE_PASS){
			//仅允许,加一条规则,其他的不能PASS
			fwaddrule("block out on %s proto %s from %s/%i to any\n",
					wan_ifname, "tcp/udp",NSTR(SYS_lan_ip),countbits(SYS_lan_mask));
		}
		client_default_action = 1;
	}
}

void firewall_client_init(void)
{
	memset(client_fil_status,0,sizeof(client_fil_status));
	client_default_action = 0;
	firewall_client_update();
}

/*
 * In our implementation, we install firewall rules after the WAN interface is up;
 * however, some rules must be present even WAN is absent.
 */
void firewall_consistent_rules(void)
{
	int index;
	int i;
	char lan_ifname[32];

	/* Create restriction filters among bridges */
	for (index = 1; index < MAX_NO_BRIDGE; index++) {
		char isolate_ifname[32];

		/* Check if the isolate lan is up? */
		if (firewall_lanifname(index, isolate_ifname) == 0 ||
		    iflib_getifaddr(isolate_ifname, 0, 0) != 0)
			continue;
		/* Isolte it from other lans */
		for (i = 0; i < MAX_NO_BRIDGE; i++) {
			if (i == index ||
			    firewall_lanifname(i, lan_ifname) == 0 ||
			    iflib_getifaddr(lan_ifname, 0, 0) != 0)
				continue;
			fwaddrule("block out on %s in-via %s all keep state\n",
				isolate_ifname, lan_ifname);
		}
	}
}

/* Basic rules of firewall */
void firewall_basic_rule(char *wan_ifname)
{
	int i;
	char lan_ifname[32];
	struct in_addr lanip;
	struct in_addr wanip;
//roy+++,2010/09/27
	ip_defense=0;
//+++	
	if (iflib_getifaddr(wan_ifname, &wanip, 0) != 0)
		return;
	
	/*
	 * Block ping from WAN
	 */
//roy+++,2010/09/27
	if (nvram_match("ping_dis_wan", "1"))
//+++		
		fwaddrule("block in quick on %s proto icmp from any to %s/32 icmp-type 8\n",
			wan_ifname, inet_ntoa(wanip));
	/*
	 * Block access to httpd default port (80) from WAN
	 */
	//fwaddrule("block in quick on %s proto tcp from any to %s/32 port = 80\n",
	//	wan_ifname, inet_ntoa(wanip));
//roy+++,2010/09/27
#if 0//2011/11/04,roy remove
	if(nvram_match("hacker_att", "1"))
	{
		SET_DEF_IP_SPOOFING;
		SET_DEF_SHORT_PACKET;
		SET_DEF_PING_OF_DEATH;
		SET_DEF_LAND_ATTACK;
		SET_DEF_SNORK_ATTACK;
		SET_DEF_UDP_PORT_LOOP;
		SET_DEF_TCP_NULL_SCAN;
		SET_DEF_SMURF_ATTACK;
		SET_DEF_SYN_FLOODING;
	}
#endif	
//+++
	/*
	 * Set spoofing rule to all interfaces
	 */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		/* Make sure the interface is up */
		if (firewall_lanifname(i, lan_ifname) == 0 ||
		    iflib_getifaddr(lan_ifname, &lanip, 0) != 0)
			continue;
		fwaddrule("block in quick on %s from %s/24 to any\n",
			wan_ifname, inet_ntoa(lanip));
	}
	/* Block UPnP multicast frames */
	if (!nvram_match("router_disabled", "1"))
		fwaddrule("block out quick on %s from any to 239.255.255.250\n", wan_ifname);
}

int fastfilter_activate(int activate)
{
	int fd, rc;

	int lan_ip,lan_mask;

	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return -1;
	rc = ioctl(fd, SIOCSFILFAST, &activate);
	if (rc)
		printf("%s fast filtering failed\n", activate?"Activate":"Deactivate");
//add for quebb	
#ifdef __CONFIG_TENDA_MULTI__
	if(nvram_match("nvram_oem_quebb","1")){
		rc =  ioctl(fd, SIOCSFIL_QUEBB, &activate);
		if (rc)
			printf("%s quebb filtering failed\n", activate?"Activate":"Deactivate");
	}
#endif
//add 2011/11/04
/*block package like this:
  *SRC_MAC:any SRC:any|DST_MAC:wan mac DST:lan side ip
  */
	if(activate){
		lan_ip = SYS_lan_ip;
		lan_mask = SYS_lan_mask;
		rc =  ioctl(fd, SIOCADWAN2LAN_IP, &lan_ip);
		if (rc)
			printf("%s wan2lan set lan ip failed\n", activate?"Activate":"Deactivate");

		rc =  ioctl(fd, SIOCADWAN2LAN_MASK, &lan_mask);
		if (rc)
			printf("%s wan2lan set lan mask failed\n", activate?"Activate":"Deactivate");
	}
	
	rc =  ioctl(fd, SIOCSWAN2LANFIL, &activate);
	if (rc)
		printf("%s wan2lan filtering failed\n", activate?"Activate":"Deactivate");
//end
	
	close(fd);
	return rc;
}

/* Flush all firewall entries */
void firewall_flush(void)
{
	firewall_inited = 0;

	macf_flush();
#ifdef __CONFIG_URLFILTER__
	firewall_urlf_flush();
#endif /* __CONFIG_URLFILTER__ */
	/* Flush all ipfilter firewall rules */
	firewall_ruleflush();

	/* re-apply consistent rules */
	firewall_consistent_rules();

	/* Disable and unhook fast filter */
	fastfilter_activate(0);

}
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__

	

/*
	return 0 :	当前时间不在日程范围内
	return 1 :	当前时间在日程范围内，应添加到过滤
*/
int is_time_in_area(struct tm *pt, int stime, int etime, unsigned char *wday)
{
	int yesterday;

	if(pt->tm_wday == 0)
		yesterday = 6;
	else
		yesterday = pt->tm_wday - 1;
	
	int now_time = pt->tm_hour * 3600 + pt->tm_min * 60 + pt->tm_sec;
	
	if( stime < etime )//没有跨天
	{
		if(wday[pt->tm_wday] == 1){
			if((now_time >= stime) && (now_time <= etime))
				return 1;	
			else
				return 0;
		}
		else
			return 0;
	}
	else if( stime >= etime )//跨天
	{
		if((now_time >= stime)&&(wday[pt->tm_wday] == 1))
			return 1;
		else if((now_time <= etime)&&(wday[yesterday] == 1))
		{
			return 1;
		}	
		else
			return 0;	
	}
	return 0;
}

void firewall_parent_control_update(void)
{
	int fd;
	int mode;
	int mode_default = MODE_DISABLED;
	int ret = 0;
	time_t now = 0;
	struct tm *time_local = NULL;
	struct macfilter macinfo;
	struct parentCtl_devices_list  *devlist;
	struct backlist_device_list *backlist;

	if (!firewall_inited)
		return;
	
	fd = open(IPL_NAME, O_RDWR);
	if (fd < 0)
		return;

	/*黑名单处理*/
	for(backlist = backlist_device_list_head; backlist != NULL; backlist = backlist->next)
	{
		if(mac_filter_mode != MODE_DENY)
		{
			mode = MODE_DENY;
			ioctl(fd, SIOCSMACFIL, &mode);
		}	
		
		if(backlist->flag == STA_ADDED )
			continue;
		
		memcpy(macinfo.mac, backlist->mac, ETHER_ADDR_LEN);
		macinfo.next = NULL;
		
		if (ioctl(fd, SIOCADMACFR, &macinfo) == 0)
		{
			printf("blacklist ADD mac filter success. [%02x:%02x:%02x:%02x:%02x:%02x]\n",backlist->mac[0],backlist->mac[1],backlist->mac[2],backlist->mac[3],
			backlist->mac[4],backlist->mac[5]);
			backlist->flag = STA_ADDED;
		}
	}

	/*家长控制处理*/
	if(gParentCtlConfig.devlist == NULL )
		goto EXIT;
	
	now = time(0);
	time_local = localtime(&now);
	if(time_local == NULL)
		goto EXIT;
	if(time_local->tm_year == 70)
	{
		goto EXIT;
	}
	
	ret = is_time_in_area(time_local, gParentCtlConfig.stime, gParentCtlConfig.etime, gParentCtlConfig.wday);
	/*0 不在时间段内，执行MAC地址过滤
	   1 若在时间段内，执行URL过滤	
	*/
	if(0 == ret)
	{
	
		if(url_filter_mode != mode_default)
		{
			printf("time is not in area. set url filter disable.\n");
			ioctl(fd, SIOCSURLFIL, &mode_default);
		}
		
		if(mac_filter_mode != MODE_DENY)
		{
			printf("time is not in area. set MAC filter deny.\n");
			mode = MODE_DENY;
			ioctl(fd, SIOCSMACFIL, &mode);
		}	
		for(devlist = gParentCtlConfig.devlist;devlist != NULL; devlist = devlist->next)
		{
			if(devlist->mac_filter_status == STA_ADDED )
				continue;
			
			memcpy(macinfo.mac, devlist->mac, ETHER_ADDR_LEN);
			macinfo.next = NULL;
			
			if (ioctl(fd, SIOCADMACFR, &macinfo) == 0)
			{
				printf("ParentCtl ADD mac filter success. [%02x:%02x:%02x:%02x:%02x:%02x]\n",devlist->mac[0],devlist->mac[1],devlist->mac[2],devlist->mac[3],
				devlist->mac[4],devlist->mac[5]);
				devlist->mac_filter_status = STA_ADDED;
			}
		}
	}
	else if(1 == ret)
	{
		if(backlist_device_list_head == NULL  && mac_filter_mode != mode_default)
		{
			printf("time is in area. set MAC filter disable.\n");
			ioctl(fd, SIOCSMACFIL, &mode_default);
		}	
		mode = gParentCtlConfig.mode;
		if(url_filter_mode != mode)
		{
			printf("time is in area. set url filter.mode=%d\n",mode);
			ioctl(fd, SIOCSURLFIL, &mode);
		}
		if(gParentCtlConfig.url_filter_status == STA_ADDED)
			goto EXIT;	
		if (ioctl(fd, SIOCADURLFR, NULL) == 0)
			gParentCtlConfig.url_filter_status = STA_ADDED;
	}

EXIT:
	close(fd);	
}
#endif

/* Initialize firewall */
void firewall_init(void)
{
	char wan_ifname[32];

	/* apply consistent rules anyway*/
	firewall_consistent_rules();

	/*
	 * Now check the WAN interface and
	 * set the rules.
	 */
	firewall_wanifname(wan_ifname);
	if (iflib_getifaddr(wan_ifname, 0, 0) != 0)
		return;

	/* Enable fast filter */
	fastfilter_activate(1);

	firewall_inited = 1;
	/*
	 * Forward drops
	 */
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	init_backlist();
	parent_control_config_update();
	firewall_parent_control_update();
#else	
	firewall_client_init();

	/* Filter by MAC address */
	macf_init();
#ifdef __CONFIG_URLFILTER__
	firewall_urlf_init();
#endif /* __CONFIG_URLFILTER__ */
#endif
	/* Create restriction filters among bridges */
	firewall_basic_rule(wan_ifname);
}

/* Print frentry list */
static void
printlist(frentry_t *fp)
{
	for (; fp; fp = fp->fr_next) {
		printf("[%u hits] ", (uint)fp->fr_hits);
		printfr(fp);
		if (fp->fr_grp)
			printlist(fp->fr_grp);
	}
}

/* Firewall command */
int
firewall_cmd(int argc, char* argv[])
{
	friostat_t fio;
	friostat_t *fiop = &fio;
	struct	frentry	*fp = NULL;
	int set;

	/* Skip command name */
	argc--;
	argv++;

	if (argc < 1) {
#ifdef __CONFIG_URLFILTER__
		printf("fw rule/mac/url/add/flush\n");
#else
		printf("fw rule/mac/add/flush\n");
#endif /* __CONFIG_URLFILTER__ */
		return -1;
	}
	if (!strcmp(argv[0], "rule")) {
		int fd;

		fd = open(IPL_NAME, O_RDONLY);
		if (fd < 0)
			return -1;
		if (ioctl(fd, SIOCGETFS, (CYG_ADDRWORD)&fiop) != 0) {
			perror("ioctl(SIOCGETFS)");
			close(fd);
			return -1;
		}
		close(fd);
		set = fiop->f_active;
		printf("(out):\n");
		fp = (struct frentry *)fiop->f_fout[set];
		printlist(fp);
		printf("(in):\n");
		fp = (struct frentry *)fiop->f_fin[set];
		printlist(fp);
	} else if (!strcmp(argv[0], "mac")) {
		if (argc < 2) {
			printf("fw mac init/flush\n");
			return -1;
		}
		if (!strcmp(argv[1], "init"))
			macf_init();
		else if (!strcmp(argv[1], "flush"))
			macf_flush();
		else {
			printf("fw mac init/flush\n");
			return -1;
		}
#ifdef __CONFIG_URLFILTER__
	} else if (!strcmp(argv[0], "url")) {
		if (argc < 2) {
			printf("fw url init/flush\n");
			return -1;
		}
		if (!strcmp(argv[0], "init"))
			firewall_urlf_init();
		else if (!strcmp(argv[0], "flush"))
			firewall_urlf_flush();
		else {
			printf("fw url init/flush\n");
			return -1;
		}
#endif
	} else if (!strcmp(argv[0], "add")) {
		fwaddrule(argv[1]);
	} else if (!strcmp(argv[0], "flush")) {
		firewall_ruleflush();
	}
	return 0;
}
