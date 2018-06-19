/*
 * Network protocol CLI commands.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_net.c,v 1.21 2010-10-15 10:05:48 Exp $
 *
 */
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <cli.h>


extern char *ether_ntoa_r(struct ether_addr *, char *addr);
extern void cyg_kmem_print_stats(void);

/* MBUF and memory show command */
static int
net_mbuf_cmd(int argc, char *argv[])
{
	struct mallinfo mem_info;
	static char *mem_top = 0;

	cyg_kmem_print_stats();

	if (mem_top == NULL)
		mem_top = HAL_MEM_REAL_REGION_TOP(mem_top);

	mem_info = mallinfo();
	printf("\nMemory system:"
		"\n   Total %d bytes"    
		"\n   Heap %d bytes, Free %d bytes, Max %d bytes\n",
		(((int)mem_top) & ~0x80000000),
		mem_info.arena, mem_info.fordblks, mem_info.maxfree);

	return 0;
}

//roy +++
int net_mbuf_free_mem(void)
{
	struct mallinfo mem_info;

	mem_info = mallinfo();

	return mem_info.fordblks;
}
//

#define	SIN(a)	((struct sockaddr_in *)a)
/*
 * Print the status of the interface.  If an address family was
 * specified, show it and it only; otherwise, show them all.
 */
static int
if_status(char *ifname, int show_all)
{
	struct ifaddrs *ifaddrs;
	struct ifaddrs *ifa, *ifn;

	if (getifaddrs(&ifaddrs) < 0) {
		printf("%s::getifaddrs() error\n", __func__);
		return -1;
	}

	for (ifa = ifaddrs; ifa != 0; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_LINK) {
			int flags = ifa->ifa_flags;
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
			int type = sdl->sdl_type;
			char addr[20];

			/* Check solo shown */
			if (ifname && strcmp(ifname, ifa->ifa_name) != 0)
				continue;
			if (!(flags & IFF_UP) && !ifname && !show_all)
				continue;

			/* Show status */
			printf("%-8s", ifa->ifa_name);

			switch (type) {
			case IFT_ETHER:
				printf("Link encap:Ethernet  ");
				printf("HWaddr %s",
					(char *)ether_ntoa_r((struct ether_addr *)LLADDR(sdl), addr));
				break;
			case IFT_LOOP:
				printf("Link encap:Local Loopback  ");
				break;
			case IFT_PPP:
				printf("Link encap:Point to Point  ");
				break;
			case IFT_PROPVIRTUAL:
				printf("Link encap:Bridge  ");
				break;
			default:
				break;
			}
			printf("\n");

			/* Search for all inet address matched this interface name */
			for (ifn = ifaddrs; ifn != 0; ifn = ifn->ifa_next) {
				if (strcmp(ifn->ifa_name, ifa->ifa_name) == 0 &&
				    ifn->ifa_addr->sa_family == AF_INET) {
					printf("        ");
					printf("inet addr:%s  ",
						inet_ntoa_r(SIN(ifn->ifa_addr)->sin_addr, addr));

					if (flags & IFF_POINTOPOINT) {
						printf("dest addr:%s  ",
						inet_ntoa_r(SIN(ifn->ifa_dstaddr)->sin_addr,
						addr));
					}
					else if (flags & IFF_BROADCAST) {
						printf("Bcast:%s  ",
						inet_ntoa_r(SIN(ifn->ifa_dstaddr)->sin_addr,
							addr));
					}

					if (ifn->ifa_netmask) {
						printf("Mask:%s",
						inet_ntoa_r(SIN(ifn->ifa_netmask)->sin_addr,
						addr));
					}
					printf("\n");
				}
			}

			/* Print flags and other things */
			printf("        ");
			if ((flags & IFF_UP)) printf("UP ");
			if ((flags & IFF_POINTOPOINT)) printf("POINTTOPOINT ");
			if ((flags & IFF_BROADCAST)) printf("BROADCAST ");
			if ((flags & IFF_LOOPBACK)) printf("LOOPBACK ");
			if ((flags & IFF_RUNNING)) printf("RUNNING ");
			if ((flags & IFF_PROMISC)) printf("PROMISC ");
			if ((flags & IFF_MULTICAST)) printf("MULTICAST ");
			if ((flags & IFF_ALLMULTI)) printf("ALLMULTI ");

			printf(" MTU:%ld", ((struct if_data *)ifa->ifa_data)->ifi_mtu);
			printf(" Metric %ld\n", ((struct if_data *)ifa->ifa_data)->ifi_metric);

			/* More statistic value */
			printf("        ");
			printf(" Rx packets:%ld", ((struct if_data *)ifa->ifa_data)->ifi_ipackets);
			printf(" error:%ld", ((struct if_data *)ifa->ifa_data)->ifi_ierrors);
			printf(" drop:%ld", ((struct if_data *)ifa->ifa_data)->ifi_iqdrops);
			printf(" mcast:%ld\n", ((struct if_data *)ifa->ifa_data)->ifi_imcasts);
			printf("        ");
			printf(" Tx packets:%ld", ((struct if_data *)ifa->ifa_data)->ifi_opackets);
			printf(" error:%ld", ((struct if_data *)ifa->ifa_data)->ifi_oerrors);
			printf(" mcast:%ld\n", ((struct if_data *)ifa->ifa_data)->ifi_omcasts);
			printf("        ");
			printf(" Rx bytes:%ld", ((struct if_data *)ifa->ifa_data)->ifi_ibytes);
			printf(" Tx bytes:%ld\n", ((struct if_data *)ifa->ifa_data)->ifi_obytes);
			
			printf("\n");
		}
	}

	freeifaddrs(ifaddrs);
	return 0;
}

/************************************************************
Function:	 if_TX_RX_status               
Description:  从ifname中获取上传下载数据包的大小

Input:         

Output: 	    

Return:     

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

int if_TX_RX_status(char *ifname,u_long * tx,u_long * rx)
{
	struct ifaddrs *ifaddrs;
	struct ifaddrs *ifa, *ifn;

	if (getifaddrs(&ifaddrs) < 0) {
		printf("%s::getifaddrs() error\n", __func__);
		return -1;
	}

	for (ifa = ifaddrs; ifa != 0; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr->sa_family == AF_LINK) 
		{
			int flags = ifa->ifa_flags;

			/* Check solo shown */
			if (ifname && strcmp(ifname, ifa->ifa_name) != 0)
				continue;
			if (!(flags & IFF_UP) && !ifname)
				continue;

			//(*rx) = ((struct if_data *)ifa->ifa_data)->ifi_ipackets;
			//(*tx) = ((struct if_data *)ifa->ifa_data)->ifi_opackets;
			(*rx) = ((struct if_data *)ifa->ifa_data)->ifi_ibytes;
			(*tx) = ((struct if_data *)ifa->ifa_data)->ifi_obytes;
		}
	}

	freeifaddrs(ifaddrs);
	return 0;
}

/*
 * Should use getoption to process, but now only handle:
 *
 * ifconfig
 * ifconfig [ifname]
 * ifconfig [ifname] ipaddr
 */
extern int ifconfig(char *name, int flags, char *addr, char *netmask);

static int
ifconfig_cmd(int argc, char *argv[])
{
	char *ifname = NULL;
	char *address = NULL;
	char *netmask = "255.255.255.0";
	int show_all = 0;

	/* Skip program name */
	--argc;
	++argv;
	if (*argv) {
		/* Parse argument */
		for (; *argv; ++argv) {
			if (!strcmp(*argv, "-a")) {
				show_all = 1;
			}
			else if (ifname == NULL) {
				ifname = *argv;
			}
			else if (address == NULL) {
				address = *argv;
			}
			else if (!strcmp(*argv, "netmask")) {
				if (!address)
					goto usage;
				argv++;
				if (!*argv)
					goto usage;

				netmask = *argv;
			}
			else {
				goto usage;
			}
		}
	}

	if (!address)
		return if_status(ifname, show_all);

	/* Config ipaddress */
	if (!ifname)
		goto usage;

	return ifconfig(ifname, IFF_UP, address, netmask);

usage:
	printf("Usage: ifconfig [-a] [ifname] [address] [netmask mask] [up/down]");
	return -1;
}

/* Ping command */
static int
net_ping_cmd(int argc, char *argv[])
{
	int icmp_ping(unsigned int dst, int seq, int timeout);

	struct hostent *hostinfo;

	int loop = 1, i, rtt, dst;
	int to = 3000;

	if (argc < 2) {
		printf("Usage: ping dst_ip <timeout> <loop>.\n");
		return -1;
	}

	if (argc > 3)
		loop = atoi(argv[3]);

	if (argc > 2)
		to = atoi(argv[2]);

	hostinfo = gethostbyname(argv[1]);
	if (!hostinfo || hostinfo->h_length != 4 ||
	    (dst = *(unsigned int *)(hostinfo->h_addr)) == 0) {
		printf("ping: %s: Unknown host\n", argv[1]);
		return -1;
	}

	for (i = 0; i < loop; i++) {
		unsigned int currtime = (unsigned int)cyg_current_time();

		if (icmp_ping(htonl(dst), i, to) != 0) {
			printf("ping %s:timeout\n",argv[1]);
		}
		else {
			rtt = (unsigned int)cyg_current_time() - currtime;
			printf("seq:%d rtt=%d\n", i, rtt);
		}
	}
	return 0;
}

#if 0
#include "zlib.h"
#include <sys/stat.h>
static int
tt_cmd(int argc, char *argv[])
{
	char sss1[] = "this is a test string;";
	char sss2[] = "whosyourdaddy.";
	struct stat s;
	int data_len =0;
	gzFile *filep = NULL;

	filep = gzopen("/test.gz", "w");
	gzwrite(filep,sss1, strlen(sss1));
	gzwrite(filep,sss2, strlen(sss2));
	gzclose(filep);
	memset(&s, 0x0, sizeof(struct stat));
	if(stat("/test.gz", &s) < 0)
	{
		diag_printf("[%s]stat(JS_FAIL_FILE, &s) < 0\n", __FUNCTION__);
		return -1;
	}
	data_len = s.st_size;
	diag_printf("[%s][%d] data_len = %d\n", __FUNCTION__, __LINE__, data_len);

	int fd = -1, read_len, i;
	unsigned char buf[128] = {0};

	fd = open("/test.gz", O_RDONLY);
	if(fd < 0)
	{
		diag_printf("open failure ---------\n") ;
		return -1;
	}
	read_len = read(fd, buf, data_len);
	close(fd);
	if(read_len != data_len)
	{
		diag_printf("read failure ---------\n") ;
		return -1;
	}
	for(i=0; i<data_len; i++)
	{
		printf("%02x ", buf[i]);
		if((i+1)%16==0)
			printf("\n");
	}
	printf("\n");
	
	return 0;
}
#endif

#include <bcmcrypto/aeskeywrap.h>
static int
tt_cmd(int argc, char *argv[])
{
	unsigned char aes_str[] = {0x14, 0x7f, 0xb9, 0xfa, 0x7f, 0xbc, 0xfb, 0xcd, 0xa3, 0x2b, 0x38, 0x0f, 0xcf, 0xd6, 0x98, 0x2a, 0x93, 0x41, 0xcd, 0xe3, 0x99, 0x3e, 0x24, 0xe0, 0xf6, 0x61, 0xd7, 0x62, 0x1d, 0x4d, 0x46, 0x08, 0x02, 0xfa, 0x41, 0x5b, 0x17, 0x23, 0x89, 0xb5, 0x48, 0xac, 0x15, 0xec, 0x11, 0xbd, 0xee, 0x9a};
	unsigned char key[16] = {0xcf, 0xdc, 0x96, 0x86, 0x35, 0x32, 0x91, 0x3c, 0x92, 0x85};
	unsigned char buf[48] = {0};
	int buf_len;
	int aes_str_len = 48;
	int ret = -1;
	
	ret = aes_unwrap(16, key, aes_str_len, aes_str, buf);
	if(ret != 0)
	{
		printf("aes_unwrap failed\n");
		//return -1;
	}

	buf_len = strlen(buf);
	printf("buf_len = %d\n", buf_len);
	printf("buf = %s\n", buf);
	return 0;
}

#if 0
struct lm_keep_version {
	uint8 type;
	uint8 action;
	uint8 verlen;
	int8 *version;
	uint8 urllen;
	int8 *url;
	uint8 md5len;
	int8* md5;
} __attribute__ ((aligned(1), packed));
#define LM_RT_VER	"V11.13.01.15_cn"
#define LM_RT_URL	"www.luminais.com/upgrade"
#define LM_RT_MD5	"299563cea591e3b6064e813fc87babe2"
extern void upgrade_online_start(struct lm_keep_version *keep_version);
extern void print_keep_version(struct lm_keep_version *keep_version);
static int tt_cmd()
{
	struct lm_keep_version *keep_version = NULL;
	int8 *buf_p;

	keep_version = (struct lm_keep_version *)malloc(sizeof(struct lm_keep_version));
	if(NULL == keep_version)
	{
		perror("malloc keep_version failed\n");
		return -1;
	}
	memset(keep_version, 0x0, sizeof(keep_version));
	
	keep_version->type = 1;
	keep_version->action = 2;
	keep_version->verlen = strlen(LM_RT_VER);
	buf_p = (int8 *)malloc(keep_version->verlen+1);
	if(NULL == buf_p)
	{
		diag_printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		free(keep_version);
		return -1;
	}
	memcpy(buf_p, LM_RT_VER, keep_version->verlen);
	buf_p[keep_version->verlen] = '\0';
	keep_version->version = buf_p;
	
	keep_version->urllen = strlen(LM_RT_URL);
	buf_p = (int8 *)malloc(keep_version->urllen+1);
	if(NULL == buf_p)
	{
		diag_printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		free(keep_version->version);
		free(keep_version);
		return -1;
	}
	memcpy(buf_p, LM_RT_URL, keep_version->urllen);
	buf_p[keep_version->urllen] = '\0';
	keep_version->url = buf_p;

	keep_version->md5len= strlen(LM_RT_MD5);
	buf_p = (int8 *)malloc(keep_version->md5len+1);
	if(NULL == buf_p)
	{
		diag_printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		free(keep_version->url);
		free(keep_version->version);
		free(keep_version);
		return -1;
	}
	memcpy(buf_p, LM_RT_MD5, keep_version->md5len);
	buf_p[keep_version->md5len] = '\0';
	keep_version->md5 = buf_p;

	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	print_keep_version(keep_version);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	upgrade_online_start(keep_version);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	return 0;
}
#endif

/* cli node */
CLI_NODE("mbuf",	net_mbuf_cmd,	"mbuf command");
CLI_NODE("ifconfig",	ifconfig_cmd,	"ifconfig command");
CLI_NODE("ping",	net_ping_cmd,	"ping dst to loop");
CLI_NODE("tt",	tt_cmd,	"tt");

#ifdef __CONFIG_NAT__
extern int ipnat_cmd(int argc, char *argv[]);
extern int firewall_cmd(int argc, char *argv[]);
CLI_NODE("ipnat",	ipnat_cmd,	"ipnat command");
CLI_NODE("fw",		firewall_cmd,	"fw command");
#endif /* __CONFIG_NAT__ */
