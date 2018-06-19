/*
 * Network protocol CLI route debug commands.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_arp.c,v 1.2 2010/07/26 09:07:51 Exp $
 *
 */

 //roy +++2010/10/08

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iflib.h>
#include <ecos_oslib.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/sockio.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/if_var.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <cli.h>

/* ARP ioctl structure */
struct arp_arpentry {
	struct in_addr addr;
	char enaddr[ETHER_ADDR_LEN];
};

typedef void pr_fun(char *fmt, ...);
static int dump_arp(struct radix_node *rn, void *req);

int add_arptables();
extern int isinwireless_client(unsigned char *mac,in_addr_t ip);
extern void show_network_tables2(int (*mtenda_routing_entry)(struct radix_node *, void *),void *pr);
extern int    arpioctl(int req, caddr_t data, struct proc *p);
extern struct ether_addr *ether_aton(const char *asc);

/*pxy w316r_pxy 2013.1.4*/
int del_oneip_fromArpTable(char ip[])
{
	char pArp[20];
	memset(pArp,0,sizeof(pArp));
	struct arp_arpentry *arp_add;

	//printf("[del_oneip_fromArpTable],ip == %s\n",ip);	
	
	arp_add = (struct arp_arpentry *)pArp;
	arp_add->addr.s_addr = inet_addr(ip);
	if(arp_add->addr.s_addr > 0){	
		if(arpioctl(SIOCDELARPRT, arp_add, NULL) != 0){
			diag_printf("arpioctl error\n");
		}
	}
}

/*pxy revise again, 2013.11.28*/
int check_oneip(char *cmpip)
{
	char ipaddr[16], netmask[16];
	char *value;
	int v1, v2;

	memset(ipaddr, 0, sizeof(ipaddr));
	memset(netmask, 0, sizeof(netmask));

	if((value = nvram_get("lan_ipaddr")) != NULL)
		strcpy(ipaddr, value);
	
	if((value = nvram_get("lan_netmask")) != NULL)
		strcpy(netmask, value);

	/*get lan_ipaddr and lan_netmask correctly*/
	if(ipaddr[0] && netmask[0]){
		v1 = inet_addr(ipaddr) & inet_addr(netmask);
		v2 = inet_addr(cmpip) & inet_addr(netmask);
		if (v1 == v2)
			return 1;
		else
			return 0;
	}
	else{
		return -1;
	}
}

static int
ping_oneip(char ip[])
{
	int icmp_ping(unsigned int dst, int seq, int timeout);

	struct hostent *hostinfo;

	int loop = 1, i, rtt, dst;
	int to = 3000;

	hostinfo = gethostbyname(ip);
	dst = inet_addr(ip);
	/*
	if (!hostinfo || hostinfo->h_length != 4 ||
	    (dst = *(unsigned int *)(hostinfo->h_addr)) == 0) {
		printf("ping: %s: Unknown host\n", ip);
		return -1;
	}
	*/
	for (i = 0; i < loop; i++) {
		unsigned int currtime = (unsigned int)cyg_current_time();

		if (icmp_ping(htonl(dst), i, to) != 0) {
			//printf("ping %s:timeout\n",ip);
			del_oneip_fromArpTable(ip);
		}
		else {
			rtt = (unsigned int)cyg_current_time() - currtime;
			//printf("ping %s\nseq:%d rtt=%d\n", ip,i, rtt);
			return 1;
		}
	}
	
	return 0;
}

static int dump_arp2(struct radix_node *rn, void *req)
{
    struct rtentry *rt = (struct rtentry *)rn;
    struct sockaddr *dst;
    struct sockaddr_dl *sdl;
	char ip[20] = {0};
	char mac[20] = {0};
    dst = rt_key(rt);
	char *lan_ifname;

    sdl = (struct sockaddr_dl *)rt->rt_gateway;
//    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == (RTF_UP | RTF_LLINFO)) 
    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == (RTF_UP | RTF_LLINFO) && sdl->sdl_alen != 0) 
    {	
    		
			strcpy(ip,inet_ntoa(((struct sockaddr_in *)dst)->sin_addr));
			strcpy(mac,ether_ntoa((struct ether_addr *)LLADDR(sdl)));
			if(strcmp(mac, "00:00:00:00:00:00") == 0)
			{
				return 0;
			}
			
			{
				/*pxy rm*/
			#if 0
	   		//	diag_printf("%s at %s",ip, mac);
				if(check_oneip(ip))
				{
					if(ping_oneip(ip) == 0)
					{
						update_arptable(LLADDR(sdl),((struct sockaddr_in *)dst)->sin_addr.s_addr,2);
						goto update;
					}
				}
			//	printf("mac(%s) ip(%s)\n",mac,ip);
			#endif
				update_arptable(LLADDR(sdl),((struct sockaddr_in *)dst)->sin_addr.s_addr,1);	
update:
				lan_ifname = nvram_safe_get("lan_ifname");
				if (strcmp(lan_ifname, "")!=0) 
					copyHostname2TendaArp(lan_ifname);
    		}
    }

    return 0;
}
#define UNLOCK 		0
#define LOCKED   	1


int ip_to_mac(struct in_addr ip, char *mac)
{
	int i = 0;
	struct arp_arpentry tmp_arp;
	char mac_null[8] = {0};

	if(NULL == mac)
		return 0;
	memset(&tmp_arp,0x0,sizeof(tmp_arp));
	tmp_arp.addr.s_addr = ip.s_addr;
	if(arpioctl(SIOCGARPRT, &tmp_arp, NULL) != 0){
		diag_printf("arpioctl error\n");
		return 0;
	}else{
		if(0 != memcmp(tmp_arp.enaddr, mac_null, 6)){
			memcpy(mac, tmp_arp.enaddr, 6);
#if 0
			printf("ip : %s\n", inet_ntoa(ip));
			printf("tmp_arp MAC : ");
			for(i=0; i<6; i++)
				printf("%02x",(tmp_arp.enaddr[i])&0xff);
			printf("\n");
			printf("MAC : ");
			for(i=0; i<6; i++)
				printf("%02x",(mac[i])&0xff);
			printf("\n");
#endif
			return 1;
		}else{
			printf("[%s][%d] no mac use %s\n", __FUNCTION__, __LINE__, inet_ntoa(ip));
			return 0;
		}
	}
}

int ip_mac_cmd(int argc, char* argv[])
{
	char mac[8] = {0};
	int i;

	if(argc != 2)
	{
		printf("ipmac\t<IP address:xxx.xxx.xxx.xxx>\n");
		return -1;
	}
	
	struct in_addr ip = {inet_addr(argv[1])};
	ip_to_mac(ip, mac);
	
	printf("MAC : ");
	for(i=0; i<6; i++)
		printf("%02x",(mac[i])&0xff);
	printf("\n");
	
	return 0;
}

/**/

int arp_cmd(int argc, char* argv[])
{
	struct arp_arpentry *arp_add;
	struct ether_addr *hw_addr;
	char pArp[20];
	memset(pArp,0,sizeof(pArp));
	
	if(argc == 1){
		//arp
//		printf("hello_11\n");
		show_network_tables2(dump_arp,NULL);
//		printf("hello_12\n");
	}
	else if(argc == 2){
		//arp ip
		arp_add = (struct arp_arpentry *)pArp;
		arp_add->addr.s_addr = inet_addr(argv[1]);
		if(arp_add->addr.s_addr > 0){
			if(arpioctl(SIOCDELARPRT, arp_add, NULL) != 0){
				diag_printf("[%s %s] arpioctl error\n",argv[0],argv[1]);
			}
		}else{
			goto usage;
		}
	}
	else if(argc == 3){
		//arp ip mac
		arp_add = (struct arp_arpentry *)pArp;
		arp_add->addr.s_addr = inet_addr(argv[1]);
		hw_addr = ether_aton(argv[2]);
		
		//diag_printf("[%s]::argv[0]=%s, argv[1]=%s, argv[2]=%s\n", __FUNCTION__,argv[0], argv[1], argv[2]);

		if(hw_addr && arp_add->addr.s_addr > 0){
			memcpy(arp_add->enaddr,hw_addr->octet,ETHER_ADDR_LEN);
					
			if(arpioctl(SIOSGARPRT, arp_add, NULL) != 0){
				diag_printf("[%s %s %s] arpioctl error\n",argv[0],argv[1],argv[2]);
			}
		}else{
			goto usage;
		}
	}
	else{
		goto usage;
	}

	return 0;
usage:
	diag_printf("Usage:"
			"\n\t arp ->show arp table"
			"\n\t arp [ip] ->del one related [ip] from arp table"
			"\n\t arp [ip] [mac] ->add one in arp table\n");
	
	return -1;
}

static int dump_arp(struct radix_node *rn, void *req)
{
    struct rtentry *rt = (struct rtentry *)rn;
    struct sockaddr *dst;
    struct sockaddr_dl *sdl;

	

    dst = rt_key(rt);
//	printf("hello_31\n");
    sdl = (struct sockaddr_dl *)rt->rt_gateway;
    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == (RTF_UP | RTF_LLINFO) && sdl->sdl_alen != 0) 
    {
    		
    		{
    			diag_printf("%s at ", inet_ntoa(((struct sockaddr_in *)dst)->sin_addr));
    			diag_printf("%s\n", ether_ntoa((struct ether_addr *)LLADDR(sdl)));
    		}
    }
//	printf("hello_32\n");

    return 0;
}

int add_arptables()
{
	show_network_tables2(dump_arp2,NULL);
}

/* cli node */
CLI_NODE("arp",	arp_cmd,	"arp command");
CLI_NODE("ipmac",	ip_mac_cmd,	"ip_to_mac");

 //roy+++
 
