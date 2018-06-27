/*
 * Network services
 *
 * Copyright (C) 2010, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: network.c,v 1.29 2011-01-06 03:08:20 Exp $
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

//add by z10312 加入kconfig.h 0105
#include <kconfig.h>

#include <nvparse.h>
#include <shutils.h>
#if 0  //add by z10312  临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -0105
#include <wlutils.h>
#endif
#include <etioctl.h>
//add by yp.2016-1-21
#include "sys_module.h"
#include "sys_option.h"
#include "wan.h"
//roy add
#include <router_net.h>
#include <sys/sockio.h>

#define NULL_FILE 0
#define NULL_STR ""
#define MAX_NO_BRIDGE 1	//Temporary add, solve compile error.

#ifdef __CONFIG_8021X__
extern int x8021_conn_successflag;
#endif

char lan_name[IFNAMSIZ];
char wan_name[IFNAMSIZ];
struct ip_set primary_lan_ip_set;
struct ip_set primary_wan_ip_set;
char wan_dns1[20];
char wan_dns2[20];

#ifdef __CONFIG_TENDA_HTTPD__
#if 0 //add by z10312 解决编译问题，先暂时屏蔽，后续按需打开 16-0120
extern void LoadWanListen(int load);
#endif
#endif

extern int g_cur_wl_radio_status;

#ifdef __CONFIG_APCLIENT_DHCPC__
extern int gpi_apclient_dhcpc_addr(char * ip,char * mask);
#endif

extern RET_INFO br_interfaces_action(BR_TYPE type,BR_ACTION action,PI8 *ifname);



extern int wlconf(char *name);
extern int wlconf_start(char *name);
extern int wlconf_down(char *name);
extern int br_add_if(int index, char *ifname);
extern int br_del_if(int index, char *ifname);
extern int dns_res_init(void);
extern void dnsmasq_restart(void);
extern int start_ntpc(void);
extern int stop_ntpc(void);
//extern void ddns_stop(void);
//extern void ddns_restart(void);
#ifdef __CONFIG_8021X__
extern void X8021_start(char *ifname);
extern void X8021_stop(char *ifname);
#endif
extern void DDNS_update(void);

#define isdigit(c) (c >= '0' && c <= '9')
#define NUM_DNS  __CONFIG_AUTO_DNS_NUM__ *2

#define WAN_PREFIX(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)

static int
add_routes(char *prefix, char *var, char *ifname)
{
	char word[80] = {0};
	char  *next = NULL;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];

	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
		
		tenda_route_add(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}
	return 0;
}

static int
del_routes(char *prefix, char *var, char *ifname)
{
	char word[80] = {0};
	char *next = NULL;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];
	
	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
		
		route_del(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}
	return 0;
}
static int
add_lan_routes(char *lan_ifname)
{
	return 0;//roy add,do nothing here
	return add_routes("lan_", "route", lan_ifname);
}

static int
del_lan_routes(char *lan_ifname)
{
	del_routes("lan_", "route", lan_ifname);
	return 0;
}
/* Set initial QoS mode for WAN et interfaces that are up. */
static int
wan_set_et_qos_mode(int unit)
{
	char name[80] = {0};
	char temp[256] = {0};
	char *next = NULL;
	char *wan_ifnames = NULL;
	int s, qos = 0;
	struct ifreq ifr;

	/* Get WME settings */
	qos = (strcmp(nvram_safe_get("wl_wme"), "off") != 0);

	/* Get WAN ifnames */
	snprintf(temp, sizeof(temp), "wan%x_ifnames", unit);
	wan_ifnames = nvram_safe_get(temp);

	/* Set qos for this interface */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	/* Set qos for each interface */
	foreach(name, wan_ifnames, next) {
		if (strncmp(name, "wl", 2) != 0) {
			strcpy(ifr.ifr_name, name);
			ifr.ifr_data = (char *) &qos;
			ioctl(s, SIOCSETQOS, &ifr);
		}
	}

	close(s);
	return 0;
}

/*
 * Carry out a socket request including openning and closing the socket
 * Return -1 if failed to open socket (and perror); otherwise return
 * result of ioctl
 */
static int
soc_req(const char *name, int action, struct ifreq *ifr)
{
	int s;
	int rv = 0;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror("socket");
		return -1;
	}
	strncpy(ifr->ifr_name, name, IFNAMSIZ);
	rv = ioctl(s, action, ifr);
	close(s);

	return rv;
}

/* Check NVRam to see if "name" is explicitly enabled */
static int
wl_vif_enabled(const char *name, char *tmp)
{
	return (atoi(nvram_safe_get(strcat_r(name, "_bss_enabled", tmp))));
}

/* Set the HW address for interface "name" if present in NVRam */
static int
wl_vif_hwaddr_set(const char *name)
{
	int rc;
	char *ea;
	char hwaddr[20];
	struct ifreq ifr;

	snprintf(hwaddr, sizeof(hwaddr), "%s_hwaddr", name);
	ea = nvram_get(hwaddr);
	if (ea == NULL) {
		return -1;
	}

	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	ether_atoe(ea, (unsigned char *)ifr.ifr_hwaddr.sa_data);
	rc = soc_req((char*)name, SIOCSIFHWADDR, &ifr);

	return rc;
}

/* Start the LAN modules */
static void
lan_inet_start(void)
{
	char msg_param[PI_BUFLEN_256] = {0};
#ifdef __CONFIG_NAT__
	//update_firewall();
	sprintf(msg_param, "op=%d", OP_RESTART);
	msg_send(MODULE_RC,RC_FIREWALL_MODULE,msg_param);    //change by yp,2016-1-21
#endif
	return;
}
static void
lan_inet_stop(void)
{
	char msg_param[PI_BUFLEN_256] = {0};

#ifdef __CONFIG_NAT__
	//stop_firewall();
	sprintf(msg_param, "op=%d", OP_STOP);
	msg_send(MODULE_RC,RC_FIREWALL_MODULE,msg_param);    //change by yp,2016-1-21
#endif
	return;
}

int tenda_set_mac_address( const char *interface, char *mac_address )
{
    int s, i;
    struct ifreq ifr;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 0;
    }

    strcpy(ifr.ifr_name, interface);

    for ( i = 0; i < ETHER_ADDR_LEN; i++ )
        ifr.ifr_hwaddr.sa_data[i] = mac_address[i];

    /*printf( "Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",
                 ifr.ifr_hwaddr.sa_data[0],
                 ifr.ifr_hwaddr.sa_data[1],
                 ifr.ifr_hwaddr.sa_data[2],
                 ifr.ifr_hwaddr.sa_data[3],
                 ifr.ifr_hwaddr.sa_data[4],
                 ifr.ifr_hwaddr.sa_data[5] );
    */
    
	if (ioctl(s, SIOCSIFHWADDR, &ifr)) 
	{
		perror("SIOCSIFHWADDR");
		close( s );
		return 0;
	}
	
	close( s );
	return 1;
}

//如"192.168.0.0/24"

	
		
		
		
		

void
start_lan(void)
{
	
	char name[80], *next;
	char tmp[100];
	int i, s;
	struct ifreq ifr;
	char buf[255],*ptr;
	char lan_stp[10];
	char lan_dhcp[10];
	char lan_ipaddr[15];
	char lan_netmask[15];
	char lan_hwaddr[15];
	char hwaddr[ETHER_ADDR_LEN];
	unsigned char mac_addr[6];
	char *lan_ifname = nvram_safe_get("lan_br");
	char msg_param[PI_BUFLEN_128] = {0};
	/*
	 * The NVRAM variable lan_ifnames contains all the available interfaces.
	 * This is used to build the unbridged interface list. Once the unbridged list
	 * is built lan_interfaces is rebuilt with only the interfaces in the bridge
	 */
	nvram_unset("unbridged_ifnames");
	nvram_unset("br0_ifnames");
	nvram_unset("br1_ifnames");
	
	/* Bring up all bridged interfaces */
	for (i = 0; i < MAX_NO_BRIDGE; i++) 
	{
		if(!i) 
		{
			lan_ifname = nvram_safe_get("lan_br");
			snprintf(lan_stp, sizeof(lan_stp), "lan_stp" );
			snprintf(lan_dhcp, sizeof(lan_dhcp), "lan_dhcp" );
			snprintf(lan_ipaddr, sizeof(lan_ipaddr), "lan_ipaddr" );
			snprintf(lan_hwaddr, sizeof(lan_hwaddr), "lan_hwaddr" );
			snprintf(lan_netmask, sizeof(lan_netmask), "lan_netmask" );
		} 
		else 
		{
			//目前只有一个接口, 若要扩展后期开发 add by z10312 2016-0112
			lan_ifname = nvram_safe_get( "lan_br"); 
			snprintf(lan_stp, sizeof(lan_stp), "lan%x_stp", i);
			snprintf(lan_dhcp, sizeof(lan_dhcp), "lan%x_dhcp",i );
			snprintf(lan_ipaddr, sizeof(lan_ipaddr), "lan%x_ipaddr",i );
			snprintf(lan_hwaddr, sizeof(lan_hwaddr), "lan%x_hwaddr",i );
			snprintf(lan_netmask, sizeof(lan_netmask), "lan%x_netmask",i );
		}

		if(NULL == lan_ifname && 0 == strcmp(lan_ifname,""))
			continue;
		
		if(RET_ERR == br_interfaces_action(BR_WIRED,BR_UP,"eth0"))
			continue;

		if(nvram_match(SYSCONFIG_WORKMODE,"route") 
			&& nvram_match(ADVANCE_IPTV_ENABLE,"1"))
		{
			nvram_set(LAN_IFNAMES,"eth0 eth2 eth3 eth4 eth7 wlan0 wlan1");
			if(RET_ERR == br_interfaces_action(BR_WIRED,BR_UP,"eth2"))
				continue;
			if(RET_ERR == br_interfaces_action(BR_WIRED,BR_UP,"eth3"))
				continue;
			if(RET_ERR == br_interfaces_action(BR_WIRED,BR_UP,"eth4"))
				continue;
			if(RET_ERR == br_interfaces_action(BR_WIRED,BR_UP,"eth7"))
				continue;
		}
#ifdef __CONFIG_DHCPC__			
		/* Launch DHCP client - AP only */
		if ((nvram_match(SYSCONFIG_WORKMODE, "bridge") 
			||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
		&& nvram_match(lan_dhcp, "1"))
		{
			dhcpc_start(lan_ifname, "landhcpc", "");
		}
		/* Handle static IP address - AP or Router */
		else
#endif	/* __CONFIG_DHCPC__ */
#ifdef __CONFIG_APCLIENT_DHCPC__
		{	
			char apclient_dhcpc_ip[17] = {0},apclient_dhcpc_mask[17] = {0};
			gpi_apclient_dhcpc_addr(apclient_dhcpc_ip,apclient_dhcpc_mask);
			if(0 == strcmp(apclient_dhcpc_ip,"") || 0 == strcmp(apclient_dhcpc_mask,""))
			{
				strcpy(apclient_dhcpc_ip,nvram_safe_get(lan_ipaddr));
				strcpy(apclient_dhcpc_mask,nvram_safe_get(lan_netmask));
			}
#ifdef __CONFIG_GUEST__
			check_guest_net_with_other(0,0);
#endif
			/* Bring up and configure LAN interface */
			tenda_ifconfig("eth0", IFUP,apclient_dhcpc_ip, apclient_dhcpc_mask);
			#ifdef CONFIG_RTL_HARDWARE_NAT	
			tapf_vlan_config();
			
			/* Bring up and configure LAN interface */
			tenda_ifconfig("eth0", IFUP,apclient_dhcpc_ip, apclient_dhcpc_mask);
			#endif
			/* We are done configuration */
			lan_up("eth0");

			#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
			extern void rtl_getBrIpAndMask(unsigned long ip, unsigned long mask);
			struct in_addr addr, mask;
			inet_aton(nvram_safe_get(lan_ipaddr), &addr);
			inet_aton(nvram_safe_get(lan_netmask), &mask);
			rtl_getBrIpAndMask(addr.s_addr, mask.s_addr);
			#endif

			
			if(!i)
			{//primary lan -->bridge0
				buid_ip_set(&primary_lan_ip_set, lan_ifname, inet_addr(apclient_dhcpc_ip), inet_addr(apclient_dhcpc_mask)
				, 0, 0, 0, 0, STATICMODE, NULL, NULL );
				//diag_printf("[%s]:get lanip %s\n",__FUNCTION__,nvram_safe_get(lan_ipaddr));
				primary_lan_ip_set.up = CONNECTED;
				strncpy(lan_name,lan_ifname,IFNAMSIZ);
			}
		}
#else		
		{	
		
		#ifdef CONFIG_RTL_HARDWARE_NAT
			/* 配置eth0 ip  */
			tenda_ifconfig("eth0", IFUP,
				nvram_safe_get(lan_ipaddr), nvram_safe_get(lan_netmask));
		     /* ifconfig ip 会重新更新HW_NAT 相关信息,而此时HW_NAT 已清零，需重新配置 */
			tapf_vlan_config();
		#endif
			/* 配置eth0 ip  */
			tenda_ifconfig("eth0", IFUP,
				nvram_safe_get(lan_ipaddr), nvram_safe_get(lan_netmask));
			
			/* We are done configuration */
			lan_up("eth0");

			#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
			extern void rtl_getBrIpAndMask(unsigned long ip, unsigned long mask);
			struct in_addr addr, mask;
			inet_aton(nvram_safe_get(lan_ipaddr), &addr);
			inet_aton(nvram_safe_get(lan_netmask), &mask);
			rtl_getBrIpAndMask(addr.s_addr, mask.s_addr);
			#endif
			
#if 0 
			/* Bring up and configure LAN interface */
			tenda_ifconfig(lan_ifname, IFUP,
				nvram_safe_get(lan_ipaddr), nvram_safe_get(lan_netmask));
			/* We are done configuration */
			lan_up(lan_ifname);
#endif
			if(!i)
			{//primary lan -->bridge0
				buid_ip_set(&primary_lan_ip_set, lan_ifname, inet_addr(nvram_safe_get(lan_ipaddr)), inet_addr(nvram_safe_get(lan_netmask))
				, 0, 0, 0, 0, STATICMODE, NULL, NULL );
				//diag_printf("[%s]:get lanip %s\n",__FUNCTION__,nvram_safe_get(lan_ipaddr));
				primary_lan_ip_set.up = CONNECTED;
				strncpy(lan_name,lan_ifname,IFNAMSIZ);
			}
		}

#endif
	}
		
	#ifdef CONFIG_RTL_HARDWARE_NAT
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_hwNatOnOffByApp();
	#endif
	#endif
	return;
}

void
stop_lan(void)
{
	int i;
	char name[80];
	char buf[256];
	char *lan_ifname = buf;
	char br_prefix[20];
	char *next;
	char tmp[20];
	
	/* Stop protocols */
	lan_inet_stop();

	/* Remove static routes */
	del_lan_routes(lan_ifname);

	for (i = 0; i < MAX_NO_BRIDGE; i++) 
	{
		if(!i) 
		{
			lan_ifname = nvram_safe_get("lan_br");
			snprintf(br_prefix, sizeof(br_prefix), "br0_ifnames");
		}
		else 
		{
			//目前只有一个接口, 若要扩展后期开发 add by z10312 2016-0112
			lan_ifname = nvram_safe_get( "lan_br");
			snprintf(br_prefix, sizeof(br_prefix), "br%x_ifnames",i);
		}
		if (NULL == lan_ifname && 0 == strcmp(lan_ifname, "")) 
			continue;

		/* Bring down LAN interface */
#ifdef __CONFIG_DHCPC__
		/* Launch DHCP client - AP only */
		if ((nvram_match(SYSCONFIG_WORKMODE, "bridge") 
			||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
		&& nvram_match("lan_dhcp", "1")) 
		{
			dhcpc_stop(lan_ifname);
			continue;
		}
#endif /* __CONFIG_DHCPC__ */
	
		if(RET_ERR == br_interfaces_action(BR_WIRED,BR_DOWN,"eth0"))
			continue;
	}

	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_hwNatOnOffByApp();
	#endif


	return;
}


#ifdef __CONFIG_AUTO_CONN_SERVER__
extern int is_doing_status();	
extern void auto_conn_vif_control(unsigned int on);
extern void add_vif_ap_ifnames();
#endif


#ifdef __CONFIG_NAT__
static int
wan_prefix(char *ifname, char *prefix)
{
	int unit;
	
	if ((unit = wan_ifunit(ifname)) < 0)
		return -1;

	sprintf(prefix, "wan%d_", unit);
	return 0;
}

static int
add_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return add_routes(prefix, "route", wan_ifname);
}
static int
wan_valid(char *ifname)
{
	char name[80], *next;
	
	foreach(name, nvram_safe_get("wan_ifnames"), next)
		if (ifname && !strcmp(ifname, name))
			return 1;
	return 0;
}

/* Start the wan inet protocols */
static void
wan_inet_start(void)
{
	char msg_param[PI_BUFLEN_256] = {0};
	//update_firewall();
	sprintf(msg_param, "op=%d", OP_RESTART);
	msg_send(MODULE_RC,RC_FIREWALL_MODULE,msg_param);
	//change by yp,统一以发消息形式，2016-1-21
	//dnsmasq_restart();
	msg_send(MODULE_RC,RC_DNSMASQ_MODULE,msg_param);
#ifdef __CONFIG_IPTV__
	if(!nvram_match(WAN0_PROTO, "pppoe"))
	{
		sprintf(msg_param, "op=%d", OP_RESTART);
		msg_send(MODULE_RC, RC_IGMP_MODULE,msg_param);
	}
#endif
#ifdef __CONFIG_DDNS__
	//ddns_restart();
	DDNS_update();
#endif

	
	if(nvram_match("rm_web_en", "1"))
		LoadWanListen(1);
	else 
		LoadWanListen(0);
	
	return;
}

/* Stop the wan inet protocols */
static void
wan_inet_stop(void)
{
	char msg_param[PI_BUFLEN_128] = {0};
	//change by yp,2016-1-21
	//stop_firewall();						
	//dnsmasq_restart();	
	sprintf(msg_param, "op=%d", OP_STOP);
	msg_send(MODULE_RC,RC_FIREWALL_MODULE,msg_param);
	sprintf(msg_param, "op=%d", OP_RESTART);
	msg_send(MODULE_RC,RC_DNSMASQ_MODULE,msg_param);	
#ifdef __CONFIG_IPTV__
	if(!nvram_match(WAN0_PROTO, "pppoe"))
	{
		sprintf(msg_param, "op=%d", OP_STOP);
		msg_send(MODULE_RC, RC_IGMP_MODULE,msg_param);
	}
#endif
#ifdef __CONFIG_DDNS__	
	//ddns_stop();
	DDNS_update();
#endif
	return;
}
int
add_ns(char *newdns, char *update , const int add_type)
{
	struct in_addr save_dns[NUM_DNS];
	char value[256], str[17*NUM_DNS] = {0}, str2[17*NUM_DNS];//cppcheck
	struct in_addr addr;
	int save = 0,before_save = 0, i= 0, j = 0,k = 0;
	char *name, *p, *next;

	char newdns2[20*3]= {0},newdns_add[20*3] = {0},tmp[20*3] = {0};
	
	snprintf(newdns2,sizeof(newdns2),"%s",newdns);
	strncpy(value, nvram_safe_get("resolv_conf"), sizeof(value));
	strcpy(str, value);

	if(0 != strcmp(str,""))
	{
		for (name = value, p = name, save=0;
		     name && name[0] && save<NUM_DNS;
		     name = next, p = 0) {
			strtok_r(p, " ", &next);
			if (inet_aton(name, &addr) == 0)
				continue;

			save_dns[save++].s_addr = addr.s_addr;
			before_save = save;
		}

		k = 0;
		for (name = newdns2, p = name, i = save;
		     name && name[0] && i<NUM_DNS;
		     name = next, p = 0, i++) {
			strtok_r(p, " ", &next);
			if (inet_aton(name, &addr) == 0)
				continue;
			for (j=0; j<save; j++) {
				if (save_dns[j].s_addr == addr.s_addr)
				{
					break;				
				}
			}
			if (j == save) {
				save_dns[save++].s_addr = addr.s_addr;
				if(0 == k)
				{
					sprintf(newdns_add,"%s",inet_ntoa(*(struct in_addr *)&addr));
					k++;
				}
				else
				{
					sprintf(tmp,"%s %s",newdns_add,inet_ntoa(*(struct in_addr *)&addr));
					strncpy(newdns_add,tmp,strlen(tmp));
				}
			}
		}
	}
	else
	{	
		snprintf(newdns_add,sizeof(newdns_add),"%s",newdns);
	}

 	if (before_save == 0)
	{
		sprintf(str, "%s", newdns_add);
	}
	else
	{
		if(add_type == ADD_NS_PUSH )
		{
			if(0 != strcmp(newdns_add,""))
			{
				sprintf(str2,"%s %s",newdns_add, str);
			}
			else
			{
				sprintf(str2,"%s", str);
			}
		}
		else
		{
			sprintf(str2,"%s %s",str,newdns_add);
		}
		sprintf(str, "%s", str2);
	}
	
	if (update) 
	{
		strncpy(update,str,strlen(str));
	}
	
#if 0
	//hqw add for static 8.8.8.8
	if(strstr(str,"8.8.8.8") == NULL)
	{
		strcat(str, " 8.8.8.8");
		if(update && strstr(update,"8.8.8.8") == NULL)
		{
			strcat(update, " 8.8.8.8");
		}
	}
	//end
#endif

	nvram_set("resolv_conf", str);

	/* Init here */
	dns_res_init();

	//dnsmasq_restart();
	return 0;
}

int
del_dest_ns(char *deldns)
{
	struct in_addr save_dns[NUM_DNS];
	int strlen =  17*NUM_DNS;
	char *value, str[strlen];
	char temp[17*NUM_DNS] = {0};
	struct in_addr addr;
	int save, i, j, k;
	char *name, *p, *next;

	value = nvram_safe_get("resolv_conf");

	for (name = deldns, p = name, save=0;
		 name && name[0] && save<NUM_DNS;
		 name = next, p = 0) {
		strtok_r(p, " ", &next);
		if (inet_aton(name, &addr) == 0)
			continue;
		save_dns[save].s_addr = addr.s_addr;
		save++;
	}
		 
	memset(str, 0, strlen);
	memset(temp,0,strlen);
	k = 0;
	for (name = value, p = name, i=0;
		 name && name[0] && i<NUM_DNS;
		 name = next, p = 0, i++) {
		strtok_r(p, " ", &next);
		if (inet_aton(name, &addr) == 0)
			continue;
		for (j=0; j<save; j++)
		{
			if (save_dns[j].s_addr == addr.s_addr)
				break;
		}
		if (j != save)
			continue;

		if (k == 0)
		{
			sprintf(str, "%s", inet_ntoa(*(struct in_addr *)&addr));
		}
		else
		{
			snprintf(temp, sizeof(str),"%s %s", str, inet_ntoa(*(struct in_addr *)&addr));
			strcpy(str,temp);
		}
		k++;
	}
	nvram_set("resolv_conf", str);

	/* Init here */
	dns_res_init();

	//dnsmasq_restart();
	return 0;
}


int
del_ns(char *deldns)
{
	struct in_addr save_dns[NUM_DNS];
	int strlen =  17*NUM_DNS;
	char value[256], str[strlen];
	char temp[17*NUM_DNS] = {0};
	struct in_addr addr;
	int save, i, j, k;
	char *name, *p, *next;
	strcpy(value, nvram_safe_get("resolv_conf"));

	/*如果resolv_conf为空的话，没必要继续删除*/
	if(0 == strcmp(value,""))
	{
		return 0 ;
	}

	/*从resolv_conf里面取出所有的dns列表，存在save_dns里面*/
	for (name = value, p = name, save=0;
	     name && name[0] && save<NUM_DNS;
	     name = next, p = 0) {
		strtok_r(p, " ", &next);
		if (inet_aton(name, &addr) == 0)
			continue;
		save_dns[save].s_addr = addr.s_addr;
		save++;
	}

	/*从save_dns里面查找去掉deldns存放到str里面*/
	memset(str, 0, strlen);
	k = 0;
	for (name = deldns, p = name, i=0;
	     name && name[0] && i<NUM_DNS;
	     name = next, p = 0, i++) {
		strtok_r(p, " ", &next);
		if(inet_aton(name, &addr) == 0)
			continue;
		
		for(j=0; j<save; j++)
		{
			if (save_dns[j].s_addr == addr.s_addr)
				break;
		}
		
		if (j != save)
			continue;

		if (k == 0)
			sprintf(str, "%s", inet_ntoa(*(struct in_addr *)&addr));
		else
		{
			snprintf(temp, sizeof(temp),"%s %s", str, inet_ntoa(*(struct in_addr *)&addr));
			strcpy(str,temp);
		}
		k++;
	}

	/*更新DNS*/
	nvram_set("resolv_conf", str);

	/* Init here */
	dns_res_init();

	//dnsmasq_restart();
	return 0;
}

void del_all_ns()
{
	nvram_unset("resolv_conf");
	nvram_unset("wan0_dns");
	nvram_unset("wan0_autodns");

	/* Init here */
	dns_res_init();

	return;
}


/* WAN interface up */

#ifdef __CONFIG_CHINA_NET_CLIENT__
	extern int pppoe_auth_encrypt_type;
	extern int pppoe_auth_success_num;
#endif

extern int
ifr_set_link_speed2(int speed);

static bool
wan_def_gateway_validation(char *gateway)
{
	int i;
	char *lan_ifname;
	char tmp[100];
	char lan_ipaddr[15];
	char lan_netmask[15];

	struct in_addr in_ipaddr = {0};
	struct in_addr in_netmask = {0};
	struct in_addr in_gateway = {0};


	/* Translate default gateway */
	if (!inet_aton(gateway, &in_gateway))
		return FALSE;

	/* Check each valid LAN bridge interface */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if (!i) {
			lan_ifname = nvram_safe_get("lan_ifname");
			snprintf(lan_ipaddr, sizeof(lan_ipaddr), "lan_ipaddr" );
			snprintf(lan_netmask, sizeof(lan_netmask), "lan_netmask" );
		} 
		else {
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);
			lan_ifname = nvram_safe_get( tmp);
			snprintf(lan_ipaddr, sizeof(lan_ipaddr), "lan%x_ipaddr",i );
			snprintf(lan_netmask, sizeof(lan_netmask), "lan%x_netmask",i );
		}

		if (strncmp(lan_ifname, "br", 2) == 0) {
			/* Translate LAN ip address and netmask */
			if (!inet_aton(nvram_safe_get(lan_ipaddr), &in_ipaddr))
				continue;
			if (!inet_aton(nvram_safe_get(lan_netmask), &in_netmask))
				continue;

			/* Destination check */
			if ((in_gateway.s_addr & in_netmask.s_addr) ==
			    (in_ipaddr.s_addr & in_netmask.s_addr))
			    return FALSE;
		} /* if (strncmp(lan_ifname.... */
	}

	return TRUE;
}

extern void
dhcpd_renew_lease(void);



extern int gHwNatEnabled ;
void
wan_up(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto;
	char msg_param[PI_BUFLEN_128] = {0};
//roy+++,2010/09/14
	char wan_ip[20],wan_mask[20],wan_gateway[20];
//+++

	
	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return;

	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));

	/* Set default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1")) {
		/* Ignore default gateway setting if it ip address is in our LAN subnet */
		if (wan_def_gateway_validation(nvram_safe_get(strcat_r(prefix, "gateway", tmp)))) {
			tenda_route_add(wan_ifname, 0, "0.0.0.0",
				nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
				"0.0.0.0");
		}
	}

	/* Install interface dependent static routes */
	add_wan_routes(wan_ifname);

//roy+++,2010/09/14
	memset(wan_ip,0,sizeof(wan_ip));
	memset(wan_mask,0,sizeof(wan_mask));
	memset(wan_gateway,0,sizeof(wan_gateway));
	strcpy(wan_ip,nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));
	strcpy(wan_mask,nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
	strcpy(wan_gateway,nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
	
	buid_ip_set(&primary_wan_ip_set, wan_ifname, inet_addr(wan_ip), inet_addr(wan_mask)
				, 0, inet_addr(wan_gateway), 0, 0, STATICMODE, NULL, NULL );
	primary_wan_ip_set.up = CONNECTED;
	strncpy(wan_name,wan_ifname,IFNAMSIZ);
	primary_wan_ip_set.conn_time = cyg_current_time();
	
#ifndef __CONFIG_NETBOOT__
	/* Sync time */
	sprintf(msg_param,"op=%d",OP_START);
	if (nvram_match("time_mode", "0"))//roy add
	{
		//start_ntpc();
		msg_send(MODULE_RC,RC_SNTP_MODULE,msg_param);
	}
#endif
	/* Start inet */
	wan_inet_start();
//roy+++ for pppoe xkjs		
#ifdef __CONFIG_CHINA_NET_CLIENT__
		if(strcmp(wan_proto, "pppoe") == 0){
			char pppoe_prio_xkjs[12]={0};
			if(pppoe_auth_encrypt_type != atoi(nvram_safe_get(strcat_r(prefix, "pppoe_prio_xkjs", tmp)))){
				sprintf(pppoe_prio_xkjs,"%d",pppoe_auth_encrypt_type);
				nvram_set(strcat_r(prefix, "pppoe_prio_xkjs", tmp),pppoe_prio_xkjs);
				nvram_commit();
			}
		}
#endif

	nvram_set("wan0_connect", "Connected");

#ifdef __CONFIG_WAN_SURF_CHECK__
	if(strcmp(wan_proto, "static") == 0)
	{
			/*static需求是获取到IP之后显示链接中*/
		tpi_wan_surf_check_set_result(INTERNET_TRY);
	}else
	{
			/*需求是获取到IP之后立马显示已脸网*/
		tpi_wan_surf_check_set_result(INTERNET_YES);
	}
#endif
	
//tenda add
	/*获取到IP之后重新启动硬件加速，重新写回arp表*/
	extern int tenda_arp_update_hw_table(void);
	if (!gHwNatEnabled) 
	{
		//rtl_hwNatOnOff(1);
		//tenda_arp_update_hw_table();
	}

	dhcpd_renew_lease();
//---
}

void
wan_down(char *wan_ifname)
{
	/* Stop wan serverices */
	wan_inet_stop();

	if(WL_ROUTE_MODE != gpi_wifi_get_mode())
		tenda_ifconfig(wan_ifname, IFF_UP, "0.0.0.0", NULL);
	else
		tenda_ifconfig(wan_ifname, 0, NULL, NULL);

/*lq debug 服务器端断开，状态显示不正确*/
	nvram_set("err_check","0");
	nvram_set("wan0_connect","Disconnected");
#ifdef __CONFIG_WAN_SURF_CHECK__
	tpi_wan_surf_check_set_result(INTERNET_NO);
#endif

	/* Flush routes belong to this interface */
	route_flush(wan_ifname);

	return;
}
#endif	/* __CONFIG_NAT__ */

void
lan_up(char *lan_ifname)
{
	/* Install default route to gateway - AP only */
	if ((nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
		&& nvram_invmatch("lan2_gateway", ""))
		tenda_route_add(lan_ifname, 0, "0.0.0.0", nvram_safe_get("lan2_gateway"), "0.0.0.0");

	/* Install interface dependent static routes */
	add_lan_routes(lan_ifname);
	
	/* Restart modules */
	lan_inet_start();	
}

void
lan_down(char *lan_ifname)
{
	/* Remove default route to gateway - AP only */
	if ((nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
		&& nvram_invmatch("lan2_gateway", ""))
		route_del(lan_ifname, 0, "0.0.0.0", nvram_safe_get("lan2_gateway"), "0.0.0.0");

	/* Remove interface dependent static routes */
	del_lan_routes(lan_ifname);
}

#ifdef __CONFIG_NAT__
int
wan_ifunit(char *wan_ifname)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	
#ifdef __CONFIG_PPPOE__			//added by yp,2016-3-17
	if ((unit = ppp_ifunit(wan_ifname)) >= 0)
		return unit;
	else 
#endif
	{
		for (unit = 0; unit < MAX_NVPARSE; unit ++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname) &&
			    (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
			     nvram_match(strcat_r(prefix, "proto", tmp), "static")
#ifdef	__CONFIG_L2TP__
				|| nvram_match(strcat_r(prefix, "proto", tmp), "l2tp")
#endif
#ifdef	__CONFIG_PPTP__
				|| nvram_match(strcat_r(prefix, "proto", tmp), "pptp")
				|| nvram_match(strcat_r(prefix, "proto", tmp), "pptp2")
#endif
#ifdef	__CONFIG_8021X__
				|| nvram_match(strcat_r(prefix, "proto", tmp), "8021x")
#endif
#ifdef	__CONFIG_PPPOE2__
				|| nvram_match(strcat_r(prefix, "proto", tmp), "pppoe2")
#endif
				)) {
				return unit;
			}
		}
	}
	return -1;
}

int
wan_primary_ifunit(void)
{
	int unit;
	
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		char tmp[100], prefix[] = "wanXXXXXXXXXX_";
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
			return unit;
	}

	return 0;
}
#endif /* __CONFIG_NAT__ */

void
lo_set_ip(void)
{
	/* Bring up loopback interface */
	tenda_ifconfig("lo0", IFUP, "127.0.0.1", "255.0.0.0");
}

extern int dns_query_on_idel_time;

#define DNS_QUERY_ON_IDEL_AT_LEAST 1

extern int CGI_do_wan_connect_tenda(int action);

//roy add for pppoe dial on demand
void pppoe_on_demand(int wan_link)
{
	int unit,demand;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";

	time_t now;
	struct tm *time_local;
	unsigned int now_sec,idel_time,pppoe_st,pppoe_et;

	static int dial_on_traffic_check_time = 0;
	
	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	WAN_PREFIX(unit, prefix);

	if (! nvram_match(strcat_r(prefix, "proto", tmp), "pppoe")) {
		return;
	}

	if(! wan_link){
		//WAN 没插网线
		return;
	}
	demand = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_demand", tmp)));

	if(demand == 0){
		//auto
		return;
	}else if(demand == 1){

		idel_time = (unsigned int)strtol(nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp)), NULL, 10);
		if(dial_on_traffic_check_time >= idel_time){
			if(dns_query_on_idel_time >= DNS_QUERY_ON_IDEL_AT_LEAST){
				CGI_do_wan_connect_tenda(3) ;
				dns_query_on_idel_time = 0;
			}else if(! dns_query_on_idel_time){
				CGI_do_wan_connect_tenda(4) ;
			}
			//重新计数
			dial_on_traffic_check_time = 0;
		}else{
			dial_on_traffic_check_time+=1;
		}
	}
	else if(demand == 2){
		//dial by hand
		return;
	}else if(demand == 3){
		//dial by time
		now = time(0);
		time_local = localtime(&now);
		now_sec = time_local->tm_hour * 3600 + time_local->tm_min * 60 + time_local->tm_sec;
		pppoe_st = (unsigned int)strtol(nvram_safe_get(strcat_r(prefix, "pppoe_st", tmp)), NULL, 10);
		pppoe_et = (unsigned int)strtol(nvram_safe_get(strcat_r(prefix, "pppoe_et", tmp)), NULL, 10);

		if(pppoe_st == 0 && pppoe_et == 0)
			return;
		if(now_sec>=pppoe_st && now_sec <= pppoe_et){
			CGI_do_wan_connect_tenda(3) ;
		}else{
			CGI_do_wan_connect_tenda(4) ;
		}
	}
	return ;
}

void pppoe_connect_rc(void)
{
	int unit;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";
	
	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	WAN_PREFIX(unit, prefix);

	char *pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	
	pppoe_connect(pppname);
}
void pppoe_disconnect_rc(void)
{
	int unit;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";
	
	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	WAN_PREFIX(unit, prefix);

	char *pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	
	pppoe_disconnect(pppname);
}

