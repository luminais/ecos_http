/*
 * Network services for IPv6
 *
 * Tenda add.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <bcmdevs.h>
#include <iflib.h>
#include <rc.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <nvparse.h>
#include <shutils.h>
#include <wlutils.h>
#include <etioctl.h>

#include "network6.h"

extern int dhcp6s_run;
extern int dhcp6c_run;
extern int rtadvd_run;
extern P_RTADVD_CFG rtadvd_cfg;
extern P_DHCP6C_CFG dhcp6c_cfg;
extern P_DHCP6S_CFG dhcp6s_cfg;
DHCP6C_STATUS dhcp6c_status;
LAN6_STATUS lan_ipv6_status;

extern int ifv6config(int mode, char *ifname, char * address, int plen);
extern void get_wan_addrv6(char *ifname,char *ip_addr);
extern void get_ppp_addrv6(char * ifname, char * ip_addr);
extern void dhcp6s_start();
extern void dhcp6s_stop();
extern void dhcp6c_start();
extern void dhcp6c_stop();
extern void start_rtadvd(void);
extern void stop_rtadvd(void);
extern int route6_add(char *name, int metric, char *ip1, char *ip2, int plen, char *ifname);
extern int route6_del(char *name, int metric, char *ip1, char *ip2, int plen, char *ifname);
extern void pppoe_start(char * pppname);

/* delete the flag of fe80 addr. */
void lan6addr(char *dst, char *src)
{
	char lanaddr[40] = {0}, *tmp = NULL;
	if(strncmp(src,"fe80",4)!=0)
	{
		strcpy(dst,src);
		return ;
	}

	memcpy(lanaddr, src, strlen(src));
		tmp = strstr(lanaddr, "::");
	lanaddr[strlen("fe80")] = '\0';
		sprintf(dst, "%s%s", lanaddr, tmp);
}

int count_ipv6_addr(char *addr6)
{
	int num=0;
	char *p=addr6;
	while(*p!='\0')
	{
		if(*p==':')
			num++;
		p++;
	}
	return num;
}

void set_br_addr(char *ifname)
{
	int s;
	char new_prefix[40];
	char new_addr[40];
	char id[40];
	struct in6_addr *in6;
	in6=(struct in6_addr *)malloc(sizeof(struct in6_addr));
	if(in6 == NULL)
		return;
	struct ifreq ifr;
	int addr_flag=0;
	memset((char *)in6,0,sizeof(struct in6_addr));
	memset(new_prefix,0,40);
	memset(new_addr,0,40);
	memset(id,0,40);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		perror("socket");
		free(in6);
		return ;
	}
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr)!=0)
	{
		free(in6);
		return;
	}
	in6->s6_addr[8] = ifr.ifr_hwaddr.sa_data[0]&0xff;

	in6->s6_addr[9] = ifr.ifr_hwaddr.sa_data[1]&0xff;

	in6->s6_addr[10] = ifr.ifr_hwaddr.sa_data[2]&0xff;

	in6->s6_addr[11] = 0xff;
	in6->s6_addr[12] = 0xfe;
	in6->s6_addr[13] = ifr.ifr_hwaddr.sa_data[3]&0xff;
	in6->s6_addr[14] = ifr.ifr_hwaddr.sa_data[4]&0xff;
	in6->s6_addr[15] = ifr.ifr_hwaddr.sa_data[5]&0xff;
	in6->s6_addr[8] &= ~EUI64_GBIT;
	in6->s6_addr[8] |= EUI64_UBIT;
	printf(" dhcp6s_cfg->prefix = %s \n",lan_ipv6_status.prefix);
	sprintf(id,"%02x%02x:%02x%02x:%02x%02x:%02x%02x",in6->s6_addr[8],in6->s6_addr[9],
		in6->s6_addr[10],in6->s6_addr[11],in6->s6_addr[12],in6->s6_addr[13],
		in6->s6_addr[14],in6->s6_addr[15]);
	addr_flag=count_ipv6_addr(lan_ipv6_status.prefix);
	if(addr_flag==5)
	{
		strncpy(new_prefix,lan_ipv6_status.prefix,strlen(lan_ipv6_status.prefix)-1);
		sprintf(new_addr,"%s%s",new_prefix,id);
	}
	else
	{
		sprintf(new_addr,"%s%s",lan_ipv6_status.prefix,id);
	}
	printf("new_addr = %s \n",new_addr);
	strcpy(dhcp6c_status.lan6_addr,new_addr);
	ifv6config(1,"br0",new_addr,64);

	free(in6);
	in6=NULL;

}

/* generate the dhcp6s address pool from host id. */
int lan6id2addr(char *id1, char *id2, char *prefix)
{
	int prefix_l = 0, id_len = 0;

	if(strlen(id1) < 3 || strlen(id2) < 3)
		return -1;

	if(id1[0] == ':')
	{
		sprintf(dhcp6s_cfg->start_addr, "%s%s", prefix, id1);
		sprintf(dhcp6s_cfg->end_addr, "%s%s", prefix, id2);
	}
	else
	{
		prefix_l=count_ipv6_addr(prefix);
		id_len=count_ipv6_addr(id1);
		printf("************ prefix_l = %d ,id_len = %d \n", prefix_l, id_len);
		if((prefix_l+id_len) >= 6)
		{
			sprintf(dhcp6s_cfg->start_addr,"%s:%s", prefix, id1);
			sprintf(dhcp6s_cfg->end_addr,"%s:%s", prefix, id2);
		}
		else
		{
			sprintf(dhcp6s_cfg->start_addr,"%s::%s", prefix, id1);
			sprintf(dhcp6s_cfg->end_addr,"%s::%s", prefix, id2);
		}
	}
	 return 0;
}

void get_status_v6(DHCP6C_STATUS *status)
{
	if(atoi(nvram_safe_get("ipv6_enable"))==0)//if ipv6 is not enabled ,do not show anything in web
		return ;

	if(atoi(nvram_safe_get("wan6_con_type"))==2)
	{
		get_wan_addrv6(dhcp6c_status.ifname,dhcp6c_status.ipv6_addr);//get global address
	}
	if(strlen(dhcp6c_status.lan6_addr)<3)
	{
		get_wan_addrv6("br0",dhcp6c_status.lan6_addr);
		if(strlen(dhcp6c_status.lan6_addr)<3)
			get_ppp_addrv6("br0",dhcp6c_status.lan6_addr);

	}
	if(atoi(nvram_safe_get("wan6_con_type"))==0)
	{
		if(strlen(dhcp6c_status.ipv6_addr)<3)
			get_wan_addrv6("vlan2",dhcp6c_status.ipv6_addr);//get global address
	}

	if(atoi(nvram_safe_get("err_check")) == 1)//wan 口物理链接断开
	{
		strcpy(status->ifname,"");
		strcpy(status->ipv6_addr,"");
		strcpy(status->ipv6_prefix,"");
		strcpy(status->gateway_v6,"");
		strcpy(status->pri_dns,"");
		strcpy(status->sec_dns,"");
		strcpy(status->aftr_name,"");
	}
	else
	{
		strcpy(status->ifname,dhcp6c_status.ifname);
		strcpy(status->ipv6_addr,dhcp6c_status.ipv6_addr);
		strcpy(status->ipv6_prefix,dhcp6c_status.ipv6_prefix);
		strcpy(status->gateway_v6,dhcp6c_status.gateway_v6);
		strcpy(status->pri_dns,dhcp6c_status.pri_dns);
		strcpy(status->sec_dns,dhcp6c_status.sec_dns);
		strcpy(status->aftr_name,dhcp6c_status.aftr_name);
	}
	lan6addr(status->lan6_addr,dhcp6c_status.lan6_addr);

}

/***
  *	lan6_dhcp---dhcp6s
  *
******/
int lan6_dhcp_config()
{
	char *lan_prefix = NULL;

	if(dhcp6s_cfg==NULL)
		dhcp6s_cfg = (P_DHCP6S_CFG)malloc(sizeof(DHCP6S_CFG));
	if(dhcp6s_cfg==NULL)
		return -1;

	memset(dhcp6s_cfg,0,sizeof(DHCP6S_CFG));
	dhcp6s_cfg->dhcp6s_mode=atoi(nvram_safe_get("lan6_dhcp_ifid_mode"));
	/* dns */
	dhcp6s_cfg->d6ns_mode=atoi(nvram_safe_get("lan6_dhcp_dns_mode"));
	if(dhcp6s_cfg->d6ns_mode == 1)
	{
		strcpy(dhcp6s_cfg->pri_dns,nvram_safe_get("lan6_dhcp_pri_dns"));
		strcpy(dhcp6s_cfg->sec_dns,nvram_safe_get("lan6_dhcp_sec_dns"));
	}
	else
	{
		strcpy(dhcp6s_cfg->pri_dns,dhcp6c_status.pri_dns);
		strcpy(dhcp6s_cfg->sec_dns,dhcp6c_status.sec_dns);
	}

	/* prefix */
	dhcp6s_cfg->prefix_len=atoi(nvram_safe_get("lan6_dhcp_prefix_len"));
	lan_prefix = nvram_safe_get("lan6_dhcp_prefix");
	strncpy(dhcp6s_cfg->prefix,lan_prefix,strlen(lan_prefix)-2);

	/* addr pool */
	lan6id2addr(nvram_safe_get("lan6_dhcp_start_id"), nvram_safe_get("lan6_dhcp_end_id"), dhcp6s_cfg->prefix);

	strcpy(dhcp6s_cfg->d6sname,"");
	strcpy(dhcp6s_cfg->ifname,nvram_safe_get("lan6_dhcp_ifname"));
	strcpy(dhcp6s_cfg->domain,"");

	//show_dhcp6s_cfg(cfg);

	return 0;
}

int lan6_dhcp_unconfig()
{
	if(dhcp6s_cfg)
	{
		free(dhcp6s_cfg);
		dhcp6s_cfg=NULL;
	}
	else
		return -1;
	return 0;
}

void lan6_dhcp_start()
{
	if(lan6_dhcp_config() == 0)
		dhcp6s_start();

	return;
}

void lan6_dhcp_stop()
{
	if(dhcp6s_run == 1)
	{
		dhcp6s_stop();
		lan6_dhcp_unconfig();
	}

	return ;
}

/***
  *	rtadvd
  *
******/
int lan6_rtadvd_config()
{
	int i;

	if(rtadvd_cfg==NULL)
		rtadvd_cfg = (P_RTADVD_CFG)malloc(sizeof(RTADVD_CFG));
	if(rtadvd_cfg==NULL)
		return -1;
	memset(rtadvd_cfg,0,sizeof(RTADVD_CFG));

	if(atoi(nvram_safe_get("lan6_dhcp_ifid_mode"))==1)
	{
		rtadvd_cfg->rtadvd_mFlag=1;
		rtadvd_cfg->rtadvd_oFlag=1;
		rtadvd_cfg->rapfopt[0].rtadvd_aFlag=0;
	}
	else
	{
		rtadvd_cfg->rtadvd_mFlag=0;
		rtadvd_cfg->rtadvd_oFlag=1;
		rtadvd_cfg->rapfopt[0].rtadvd_aFlag=1;
	}
	rtadvd_cfg->ra_maxinterval=10;
	rtadvd_cfg->ra_mininterval=0;
	strcpy(rtadvd_cfg->ra_interface,nvram_safe_get("lan6_dhcp_ifname"));

	/* Rtadvd supports RA_MAXPREFIX prefixs at most.
	  * Here we take one prefix defaultly.
	**/
	for(i = 0; i< 1; i++)
	{
		rtadvd_cfg->rapfopt[i].pref_lifetime=-1;
		rtadvd_cfg->rapfopt[i].valid_lifetime=-1;

		strcpy(rtadvd_cfg->rapfopt[i].prefix_addr,nvram_safe_get("lan6_dhcp_prefix"));
		rtadvd_cfg->rapfopt[i].prefix_len=atoi(nvram_safe_get("lan6_dhcp_prefix_len"));

		if(atoi(nvram_safe_get("lan6_dhcp_ifid_mode"))==1)
			rtadvd_cfg->rapfopt[i].rtadvd_aFlag=0;
		else
			rtadvd_cfg->rapfopt[i].rtadvd_aFlag=1;
	}

	return 0;
}

int lan6_rtadvd_unconfig()
{
	if(rtadvd_cfg)
	{
		free(rtadvd_cfg);
		rtadvd_cfg=NULL;
	}
	else
		return -1;
	return 0;
}

void lan6_rtadvd_start()
{
	if(lan6_rtadvd_config() == 0)
		start_rtadvd();

	return ;
}

void lan6_rtadvd_stop()
{
	if(rtadvd_run == 1)
	{
		stop_rtadvd();
		lan6_rtadvd_unconfig();
	}

	return ;
}

/***
  *	lan ipv6 entry.
  *
******/
void lan6_status_config()
{
	memset(&lan_ipv6_status,0,sizeof(LAN6_STATUS));
	lan_ipv6_status.lan_prefix_mode = atoi(nvram_safe_get("lan6_dhcp_prefix_mode"));
	if(lan_ipv6_status.lan_prefix_mode == 1)	// manu
		strcpy(lan_ipv6_status.prefix, nvram_safe_get("lan6_dhcp_prefix"));

	lan_ipv6_status.lan_ipv6_enable = atoi(nvram_safe_get("lan6_dhcp_enable"));
	lan_ipv6_status.lan_dhcp6_type = atoi(nvram_safe_get("lan6_dhcp_ifid_mode"));

	strcpy(lan_ipv6_status.lan_start_id, nvram_safe_get("lan6_dhcp_start_id"));
	strcpy(lan_ipv6_status.lan_end_id, nvram_safe_get("lan6_dhcp_end_id"));

	lan_ipv6_status.lan_dns_type = atoi(nvram_safe_get("lan6_dhcp_dns_mode"));
	if(lan_ipv6_status.lan_dns_type == 1)
	{
		strcpy(lan_ipv6_status.lan_pri_dns, nvram_safe_get("lan6_dhcp_pri_dns"));
		strcpy(lan_ipv6_status.lan_sec_dns, nvram_safe_get("lan6_dhcp_sec_dns"));
	}

	return ;
}

void lan6_start()
{
	char *str = NULL;

	lan6_status_config();

	if(atoi(nvram_safe_get("lan6_type")) == 1)		//manu
	{
		str = nvram_safe_get("lan6_ipaddr");
		if(strlen(str)>0)
		{
			ifv6config(IFV6CONFIG_ADD,"br0", str, 64);
			strcpy(dhcp6c_status.lan6_addr, str);
		}
	}
	else
	{
		//strcpy(cfg->lan_ip6_addr,dhcp6c_status.lan6_addr);
	}
	if(atoi(nvram_safe_get("lan6_dhcp_prefix_mode")) == 0)
		return ;

	set_br_addr("br0");

	if(atoi(nvram_safe_get("lan6_dhcp_enable")) == 0)
	{
		lan6_dhcp_stop();
		lan6_rtadvd_stop();
		return ;
	}

	lan6_dhcp_start();
	lan6_rtadvd_start();
	return ;
}

void lan6_stop()
{
	lan6_dhcp_stop();
	lan6_rtadvd_stop();
	if(strncmp(dhcp6c_status.lan6_addr, "fe80", sizeof("fe80")) != 0)
		ifv6config(IFV6CONFIG_DEL,"br0",dhcp6c_status.lan6_addr,64);
	else
		printf("br0: link local address, don't remove!");

	return ;
}

/***
  *	wan6_static
  *
******/
void wan6_static_config()
{
	char *value1 = NULL, *value2 =  NULL;
	strcpy(dhcp6c_status.ifname,"vlan2");

	if((value1 = nvram_safe_get("wan6_static_addr")) && (value2 = nvram_safe_get("wan6_static_plen")))
	{
		ifv6config(IFV6CONFIG_ADD,"vlan2", value1, atoi(value2));
		strcpy(dhcp6c_status.ipv6_addr, value1);
	}

	if((value1 = nvram_safe_get("wan6_static_gateway")))
	{
		strcpy(dhcp6c_status.gateway_v6, value1);

		unsigned int ifid;
		char iaddr6[64];
		ifid = if_nametoindex(dhcp6c_status.ifname);
		if(strncmp(dhcp6c_status.gateway_v6, "fe80", 4) == 0)
			sprintf(iaddr6, "fe80:%u%s", ifid, dhcp6c_status.gateway_v6+4);
		else
			strcpy(iaddr6, dhcp6c_status.gateway_v6);
		route6_del((void *)1, 0, "0:0:0:0::0", iaddr6, 0, 0);
		route6_add((void *)1, 0, "0:0:0:0::0", iaddr6, 0, 0);
	}
	if((value1 = nvram_safe_get("wan6_static_pri_dns")))
	{
		printf("cfg->pri_dns = %s \n", value1);
		strcpy(dhcp6c_status.pri_dns, value1);
	}
	if((value1 = nvram_safe_get("wan6_static_sec_dns")))
	{
		printf("cfg->sec_dns= %s \n", value1);
		strcpy(dhcp6c_status.sec_dns, value1);
	}
	if((value1 = nvram_safe_get("wan6_static_mtu")))
	{
		struct ifreq ifr;
		int sockfd;
		sockfd  = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, "vlan2");
		ifr.ifr_mtu = atoi(value1);
		ioctl(sockfd, SIOCSIFMTU, &ifr);
		close(sockfd);
	}

	return;
}

/***
  *	wan6_dhcp---dhcp6c
  *
******/
int wan6_dhcp_config( )
{
	char *value = NULL;
	if(dhcp6c_cfg== NULL)
		dhcp6c_cfg = (P_DHCP6C_CFG)malloc(sizeof(DHCP6C_CFG));
	if(dhcp6c_cfg==NULL)
		return -1;
	memset(dhcp6c_cfg, 0x0, sizeof(DHCP6C_CFG));
	if(strlen((value = nvram_safe_get("wan6_dhcp_ifname"))))
		strcpy(dhcp6c_cfg->ifname, value);
	else
		strcpy(dhcp6c_cfg->ifname,"vlan2");
	if(atoi((value = nvram_safe_get("wan6_dhcp_iaid"))))
		dhcp6c_cfg->iaid= atoi(value);
	else
		dhcp6c_cfg->iaid=100;
	if(atoi((value = nvram_safe_get("wan6_dhcp_iapd"))))
		dhcp6c_cfg->iapd= atoi(value);
	else
		dhcp6c_cfg->iapd=100;
	if(atoi((value = nvram_safe_get("wan6_dhcp_slid"))))
		dhcp6c_cfg->slid= atoi(value);
	else
		dhcp6c_cfg->slid=64;

	dhcp6c_cfg->stless=1 - atoi(nvram_safe_get("wan6_dhcp_nontemporary"));
	dhcp6c_cfg->gpd=atoi(nvram_safe_get("wan6_dhcp_delegation"));

	if(!atoi((value = nvram_safe_get("wan6_dhcp_aftr"))))
		dhcp6c_cfg->aftr=0;
	else
		dhcp6c_cfg->aftr=1;

	//dhcp6c_dump(cfg);

	return 0;
}

static int wan6_dhcp_unconfig()
{
	if(dhcp6c_cfg)
	{
		free(dhcp6c_cfg);
		dhcp6c_cfg=NULL;
	}
	else
		return -1;
	return 0;
}

void wan6_dhcp_start()
{
	if(wan6_dhcp_config() == 0)
		dhcp6c_start();

	return;
}
void wan6_dhcp_stop()
{
	if(dhcp6c_run==1)
	{
		if(strncmp(dhcp6c_status.ipv6_addr,"fe80",4))
			ifv6config(2,"vlan2",dhcp6c_status.ipv6_addr,64);
		wan6_dhcp_unconfig();
		printf("Dhcp6c_STOP \n");
		dhcp6c_stop();
	}

	return ;
}


/***
  *	wan6_dhcp config for pppoe, ppp link is up.
  *
******/
int wan6_dhcp_config_for_pppoe( )
{
	char *value = NULL;
	if(dhcp6c_cfg== NULL)
		dhcp6c_cfg = (P_DHCP6C_CFG)malloc(sizeof(DHCP6C_CFG));
	if(dhcp6c_cfg==NULL)
		return -1;
	memset(dhcp6c_cfg, 0x0, sizeof(DHCP6C_CFG));
	if(strlen((value = nvram_safe_get("wan0_pppoe_ifname"))))
		strcpy(dhcp6c_cfg->ifname, value);
	else
		strcpy(dhcp6c_cfg->ifname,"ppp0");

	dhcp6c_cfg->iaid=100;
	dhcp6c_cfg->iapd=101;
	dhcp6c_cfg->slid=102;
	dhcp6c_cfg->stless=1 - atoi(nvram_safe_get("wan6_pppoe_nontemporary"));
	dhcp6c_cfg->gpd=atoi(nvram_safe_get("wan6_pppoe_delegation"));

	if(!atoi((value = nvram_safe_get("wan6_dhcp_aftr"))))
		dhcp6c_cfg->aftr=0;
	else
		dhcp6c_cfg->aftr=1;

	return 0;
}

void wan6_dhcp_start_for_pppoe()
{
	if(wan6_dhcp_config_for_pppoe() == 0)
		dhcp6c_start();

	return;
}

void wan6_dhcp_stop_for_pppoe()
{
	if(dhcp6c_run==1)
	{
		if(strncmp(dhcp6c_status.ipv6_addr,"fe80",4))
			ifv6config(2,"ppp0",dhcp6c_status.ipv6_addr,64);
		wan6_dhcp_unconfig();
		printf("Dhcp6c_STOP for pppoe\n");
		dhcp6c_stop();
	}

	return ;
}


/***
  *	wan ipv6 entry.
  *
******/
void wan6_start()
{
	int ipv6_enable, con_type;
	ipv6_enable = atoi(nvram_safe_get("ipv6_enable"));
	con_type = atoi(nvram_safe_get("wan6_con_type"));

	if(ipv6_enable)
	{
		switch(con_type)
		{
			case 0:
				wan6_dhcp_start();
				break;
			case 1:
				wan6_static_config();
				break;
			case 2:
				pppoe_start("ppp0");
				break;
			default:
				break;
		}

		lan6_start();
	}
	return;
}

void wan6_stop()
{
	char *ipv6_enable,*wan_con_type_v6,*prefix_mode;
	wan_con_type_v6  = nvram_safe_get("wan6_con_type");
	ipv6_enable = nvram_safe_get("ipv6_enable");
	prefix_mode =  nvram_safe_get("lan6_dhcp_prefix_mode");

	if(0 == atoi(ipv6_enable))
		return;

	if(strcmp(wan_con_type_v6, "1") == 0)
	{
		ifv6config(IFV6CONFIG_DEL,"vlan2",dhcp6c_status.ipv6_addr,64);
	}
	else if(strcmp(wan_con_type_v6, "0") == 0)
	{
		wan6_dhcp_stop();
		if(strcmp(prefix_mode, "0") == 0)
			lan6_stop();
	}
	else if(strcmp(wan_con_type_v6, "2") == 0)
	{
		wan_pppoe_down("ppp0");
		wan6_dhcp_stop();
		if(strcmp(prefix_mode, "0") == 0)
			lan6_stop();
	}
	strcpy(dhcp6c_status.ifname,"");
	strcpy(dhcp6c_status.ipv6_addr,"");
	strcpy(dhcp6c_status.ipv6_prefix,"");
	strcpy(dhcp6c_status.gateway_v6,"");
	strcpy(dhcp6c_status.pri_dns,"");
	strcpy(dhcp6c_status.sec_dns,"");
	strcpy(dhcp6c_status.aftr_name,"");
	return;
}
