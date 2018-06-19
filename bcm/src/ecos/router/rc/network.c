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
#include <nvparse.h>
#include <shutils.h>
#include <wlutils.h>
#include <etioctl.h>

//roy add
#include <router_net.h>
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
extern void LoadWanListen(int load);
#endif
//
extern int g_cur_wl_radio_status;
#ifdef __CONFIG_APCLIENT_DHCPC__
extern int gpi_get_apclient_dhcpc_addr(char * ip,char * mask);
#endif

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
#if 1
//luminais mark
int is_wan_up = 0;
//luminais
#endif
//lvliang add for download webpage

extern void start_download_redirect_url(void);
extern void stop_download_redirect_url(void);
extern void url_record_start(void);
extern void url_record_stop(void);
extern int kill_sdw_download_thread(void) ;
extern int have_success_download  ;

char globle_wan_ip[20] = {0} ;

//end by add 

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
	char word[80], *next;
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
		
		route_add(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}
	return 0;
}

static int
del_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
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
	char name[80], temp[256];
	char *next, *wan_ifnames;
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
#ifdef __CONFIG_NAT__
	update_firewall();
#endif
	return;
}

static void
lan_inet_stop(void)
{
#ifdef __CONFIG_NAT__
	stop_firewall();
#endif
	return;
}

void
start_lan(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char lan_ifname_save[16] = {0};
	char name[80], *next;
	char tmp[100];
	int i, s;
	struct ifreq ifr;
	char buf[255],*ptr;
	char lan_stp[10];
	char *lan_ifnames;
	char lan_ifnames_save[32] = {0};
	char lan_dhcp[10];
	char lan_ipaddr[15];
	char lan_netmask[15];
	char lan_hwaddr[15];
	char hwaddr[ETHER_ADDR_LEN];

	/*
	 * The NVRAM variable lan_ifnames contains all the available interfaces.
	 * This is used to build the unbridged interface list. Once the unbridged list
	 * is built lan_interfaces is rebuilt with only the interfaces in the bridge
	 */
	nvram_unset("unbridged_ifnames");
	nvram_unset("br0_ifnames");
	nvram_unset("br1_ifnames");

	/*
	 * If we're a travel router... then we need to make sure we get
	 * the primary wireless interface up before trying to attach slave
	 * interfaces to the bridge
	 */
	if (nvram_match("ure_disable", "0") && nvram_match("router_disable", "0")) {
		/* start wlireless */
		wlconf(nvram_safe_get("wan0_ifname"));
	}

	/* Bring up all bridged interfaces */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if(!i) {
			memset(lan_ifname_save, 0x0, sizeof(lan_ifname_save));
			lan_ifname = nvram_safe_get("lan_ifname");
			strncpy(lan_ifname_save, lan_ifname, sizeof(lan_ifname_save));
			snprintf(lan_stp, sizeof(lan_stp), "lan_stp" );
			snprintf(lan_dhcp, sizeof(lan_dhcp), "lan_dhcp" );
			snprintf(lan_ipaddr, sizeof(lan_ipaddr), "lan_ipaddr" );
			snprintf(lan_hwaddr, sizeof(lan_hwaddr), "lan_hwaddr" );
			snprintf(lan_netmask, sizeof(lan_netmask), "lan_netmask" );
			memset(lan_ifnames_save, 0x0, sizeof(lan_ifnames_save));
			lan_ifnames = nvram_safe_get("lan_ifnames");
			strncpy(lan_ifnames_save, lan_ifnames, sizeof(lan_ifnames_save));
		} 
		else {
			memset(lan_ifname_save, 0x0, sizeof(lan_ifname_save));
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);
			lan_ifname = nvram_safe_get( tmp);
			strncpy(lan_ifname_save, lan_ifname, sizeof(lan_ifname_save));
			snprintf(lan_stp, sizeof(lan_stp), "lan%x_stp", i);
			snprintf(lan_dhcp, sizeof(lan_dhcp), "lan%x_dhcp",i );
			snprintf(lan_ipaddr, sizeof(lan_ipaddr), "lan%x_ipaddr",i );
			snprintf(lan_hwaddr, sizeof(lan_hwaddr), "lan%x_hwaddr",i );
			snprintf(lan_netmask, sizeof(lan_netmask), "lan%x_netmask",i );
			snprintf(tmp, sizeof(tmp), "lan%x_ifnames", i);
			memset(lan_ifnames_save, 0x0, sizeof(lan_ifnames_save));
			lan_ifnames = nvram_safe_get( tmp);
			strncpy(lan_ifnames_save, lan_ifnames, sizeof(lan_ifnames_save));
		}
		if (!strncmp(lan_ifname_save, "br", 2)) {
			/* br? interface already add in BRG_start */
			memset(hwaddr, 0, sizeof(hwaddr));
			foreach(name, lan_ifnames_save, next) {
				if (!strncmp(name, "wl", 2)) {
					if (!wl_vif_enabled(name, tmp)) {
						continue; /* Ignore disabled WL VIF */
					}
					wl_vif_hwaddr_set(name);
				}

				/*
				 * Bring up interface. Ignore any bogus/unknown
				 * interfaces on the NVRAM list
				 */
				ifconfig(name, 0, NULL, NULL);
				cyg_thread_delay(10);
				if (ifconfig(name, IFUP, NULL, NULL)){
					printf("%s():: ifconfig up failed\n", __FUNCTION__);
					continue;
				}

				/*
				 * Set the logical bridge address to that
				 * of the first interface
				 */
				if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
					printf("%s()::socket\n", __FUNCTION__);
					continue;
				}

				strncpy(ifr.ifr_name, lan_ifname_save, IFNAMSIZ);
				if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0 &&
					memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN) == 0) {
					strncpy(ifr.ifr_name, name, IFNAMSIZ);
					if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
						strncpy(ifr.ifr_name, lan_ifname_save, IFNAMSIZ);
						ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
						ioctl(s, SIOCSIFHWADDR, &ifr);

						memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
					}
				}
				close(s);

				/* If not a wl i/f then simply add it to the bridge */
				if (wlconf(name)) {
					/* add not wl i/f to bridge */
					if (br_add_if(i, name)) {
						if (errno == EEXIST)
							;
					}
					else {
						snprintf(tmp, sizeof(tmp), "br%x_ifnames", i);
						ptr = nvram_get(tmp);
						if (ptr)
							snprintf(buf, sizeof(buf),
								"%s %s", ptr, name);
						else
							strncpy(buf, name, sizeof(buf));
						nvram_set(tmp, buf);
					}
				}
				/* wl i/f */
				else {
					char mode[] = "wlXXXXXXXXXX_mode";
					int unit;

					/* get the instance number of the wl i/f */
					wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));

					snprintf(mode, sizeof(mode), "wl%d_mode", unit);

					/* WET specific configurations */
					if (nvram_match(mode, "wet")) {
						/*
						 * Bring up and set receive all multicast
						 * frames in WET mode
						 */
						ifconfig(name, IFUP, NULL, NULL);

						/* Enable host DHCP relay */
						if (nvram_match("lan_dhcp", "1"))
							wl_iovar_set(name, "wet_host_mac",
								ifr.ifr_hwaddr.sa_data,
								ETHER_ADDR_LEN);

						/* Turn off mpc */
						wl_iovar_setint(name, "mpc", 0);
					}
					/* Dont attach the main wl i/f in wds */
					if ((strncmp(name, "wl", 2) != 0) &&
					    (nvram_match(mode, "wds"))) {
						/*
						 * Save this interface name in
						 * unbridged_ifnames. This behaviour
						 * is consistent with BEARS release.
						 */
						ptr = nvram_get("unbridged_ifnames");
						if (ptr)
							snprintf(buf, sizeof(buf),
								"%s %s", ptr, name);
						else
							strncpy(buf, name, sizeof(buf));
						nvram_set("unbridged_ifnames", buf);
						continue;
					}

					/* add wl i/f to bridge */
					br_add_if(i, name);
					snprintf(tmp, sizeof(tmp), "br%x_ifnames", i);
					ptr = nvram_get(tmp);
					if (ptr)
						snprintf(buf, sizeof(buf), "%s %s", ptr, name);
					else
						strncpy(buf, name, sizeof(buf));
					nvram_set(tmp, buf);
				} /* if (eval("wlconf", na... */
			} /* foreach()... */
			if (memcmp(hwaddr, "\0\0\0\0\0\0", ETHER_ADDR_LEN) &&
			    (s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
				strncpy(ifr.ifr_name, lan_ifname_save, IFNAMSIZ);
				ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
				memcpy(ifr.ifr_hwaddr.sa_data, hwaddr, ETHER_ADDR_LEN);
				ioctl(s, SIOCSIFHWADDR, &ifr);
				close(s);
			}			
		} /* if (strncmp(lan_ifname.... */
		/* specific non-bridged lan i/f */
		else if (strcmp(lan_ifname_save, "")) {
			/* Bring up interface */
			ifconfig(lan_ifname_save, IFUP, NULL, NULL);
			/* config wireless i/f */
			wlconf(lan_ifname_save);
		}
		else
			continue; /* lanX_ifname is empty string, so donot do anything */

		/* Get current LAN hardware address */
		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
			char eabuf[32];
			strncpy(ifr.ifr_name, lan_ifname_save, IFNAMSIZ);
			if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0)
				nvram_set(lan_hwaddr, ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
			close(s);
		}
		/* We need shutdown/reinitialize the bridge interface before configuring
		  * interface address, such that the bridge interface could be attached
		  * correctly.
		  */
		if (!strncmp(lan_ifname_save, "br", 2))
			ifconfig(lan_ifname_save, 0, NULL, NULL);

#ifdef __CONFIG_DHCPC__			
		/* Launch DHCP client - AP only */
		if (nvram_match("router_disable", "1") && nvram_match(lan_dhcp, "1")) {
			dhcpc_start(lan_ifname_save, "landhcpc", "");
		}
		/* Handle static IP address - AP or Router */
		else
#endif	/* __CONFIG_DHCPC__ */
#ifdef __CONFIG_APCLIENT_DHCPC__
		{	
			char apclient_dhcpc_ip[17] = {0},apclient_dhcpc_mask[17] = {0};
			gpi_get_apclient_dhcpc_addr(apclient_dhcpc_ip,apclient_dhcpc_mask);
			if(0 == strcmp(apclient_dhcpc_ip,"") || 0 == strcmp(apclient_dhcpc_mask,""))
			{
				strcpy(apclient_dhcpc_ip,nvram_safe_get(lan_ipaddr));
				strcpy(apclient_dhcpc_mask,nvram_safe_get(lan_netmask));
			}
			/* Bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP,apclient_dhcpc_ip, apclient_dhcpc_mask);
			/* We are done configuration */
			lan_up(lan_ifname);

			if(!i){//primary lan -->br0
				buid_ip_set(&primary_lan_ip_set, lan_ifname, inet_addr(apclient_dhcpc_ip), inet_addr(apclient_dhcpc_mask)
				, 0, 0, 0, 0, STATICMODE, NULL, NULL );
				//diag_printf("[%s]:get lanip %s\n",__FUNCTION__,nvram_safe_get(lan_ipaddr));
				primary_lan_ip_set.up = CONNECTED;
				strncpy(lan_name,lan_ifname,IFNAMSIZ);
			}
		}
#else
		{
			/* Bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP,
				nvram_safe_get(lan_ipaddr), nvram_safe_get(lan_netmask));
			/* We are done configuration */
			lan_up(lan_ifname_save);
#if 0 //roy+++
			/* Bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP,
				nvram_safe_get(lan_ipaddr), nvram_safe_get(lan_netmask));
			/* We are done configuration */
			lan_up(lan_ifname);
#endif
//roy +++
			if(!i){//primary lan -->br0
				buid_ip_set(&primary_lan_ip_set, lan_ifname_save, inet_addr(nvram_safe_get(lan_ipaddr)), inet_addr(nvram_safe_get(lan_netmask))
				, 0, 0, 0, 0, STATICMODE, NULL, NULL );
				//diag_printf("[%s]:get lanip %s\n",__FUNCTION__,nvram_safe_get(lan_ipaddr));
				primary_lan_ip_set.up = CONNECTED;
				strncpy(lan_name,lan_ifname_save,IFNAMSIZ);
			}
//+++
		}
#endif
	}
	return;
}

void
stop_lan(void)
{
	int i;
	char name[80];
	char buf[256];
	char *lan_ifname = buf;
	char lan_ifname_save[16] = {0};
	char br_prefix[20];
	char *lan_ifnames;
	char *next;
	char tmp[20];
	/* Stop protocols */
	lan_inet_stop();

	/* Remove static routes */
	del_lan_routes(lan_ifname);

	/* Bring down unbridged interfaces,if any */
	foreach(name, nvram_safe_get("unbridged_ifnames"), next) {
		wlconf_down(name);
		ifconfig(name, 0, NULL, NULL);
	}

	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if(!i) {
			memset(lan_ifname_save, 0x0, sizeof(lan_ifname_save));
			lan_ifname = nvram_safe_get("lan_ifname");
			strncpy(lan_ifname_save, lan_ifname, sizeof(lan_ifname_save));
			snprintf(br_prefix, sizeof(br_prefix), "br0_ifnames");
		}
		else {
			memset(lan_ifname_save, 0x0, sizeof(lan_ifname_save));
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);
			lan_ifname = nvram_safe_get( tmp);
			strncpy(lan_ifname_save, lan_ifname, sizeof(lan_ifname_save));
			snprintf(br_prefix, sizeof(br_prefix), "br%x_ifnames",i);
		}
		if (!strcmp(lan_ifname_save, "")) 
			continue;

		/* Bring down LAN interface */
#ifdef __CONFIG_DHCPC__
		/* Launch DHCP client - AP only */
		if (nvram_match("router_disable", "1") && nvram_match("lan_dhcp", "1")) {
			dhcpc_stop(lan_ifname_save);
			continue;
		}
#endif /* __CONFIG_DHCPC__ */

		/* Bring down LAN interface */
		ifconfig(lan_ifname_save, 0, NULL, NULL);

		/* Bring down bridged interfaces */
		if (strncmp(lan_ifname_save, "br", 2) == 0) {
			lan_ifnames = nvram_safe_get(br_prefix);
			foreach(name, lan_ifnames, next) {
				wlconf_down(name);
				/* Bring down and clear receive all multicast */
				ifconfig(name, 0, NULL, NULL);
				br_del_if(i, name);
			}
		}
		/* Bring down specific interface */
		else if (strcmp(lan_ifname_save, ""))
			wlconf_down(lan_ifname_save);
	}
	return;
}

#ifdef __CONFIG_AUTO_CONN__
extern int is_doing_status();	
extern void auto_conn_vif_control(unsigned int on);
extern void add_vif_ap_ifnames();
#endif
/* Bring up wireless interfaces */
void
start_wl(void)
{
	int i;
	char name[80];
	char *lan_ifname;
	char lan_ifname_save[16] = {0};
	char *lan_ifnames;
	char lan_ifnames_save[32] = {0};
	char *next;
	char tmp[100];

	#ifdef __CONFIG_AUTO_CONN__
	add_vif_ap_ifnames();
	#endif
	
	int radio_disable_val = 0;
	if (WL_RADIO_OFF == g_cur_wl_radio_status)
	{
		radio_disable_val = 1;
		radio_disable_val += WL_RADIO_SW_DISABLE << 16;
		wl_ioctl("eth1", WLC_SET_RADIO, &radio_disable_val, sizeof(radio_disable_val));
		return;
	}
	else
	{
		radio_disable_val = 0;
		radio_disable_val += WL_RADIO_SW_DISABLE << 16;
		wl_ioctl("eth1", WLC_SET_RADIO, &radio_disable_val, sizeof(radio_disable_val));
	}
	
	/*
	 * If we're a travel router... then we need to make sure we get
	 * the primary wireless interface up before trying to attach slave
	 * interfaces to the bridge
	 */
	if (nvram_match("ure_disable", "0") && nvram_match("router_disable", "0")) {
		/* start wlireless */
		wlconf_start(nvram_safe_get("wan0_ifname"));
	}

	/* Bring up bridged interfaces */
	for (i = 0; i < MAX_NO_BRIDGE; i++) {
		if(!i) {
			memset(lan_ifname_save, 0x0, sizeof(lan_ifname_save));
			lan_ifname = nvram_safe_get("lan_ifname");
			strncpy(lan_ifname_save, lan_ifname, sizeof(lan_ifname_save));
			memset(lan_ifnames_save, 0x0, sizeof(lan_ifnames_save));
			lan_ifnames = nvram_safe_get("lan_ifnames");
			strncpy(lan_ifnames_save, lan_ifnames, sizeof(lan_ifnames_save));
		} 
		else {
			memset(lan_ifname_save, 0x0, sizeof(lan_ifname_save));
			snprintf(tmp, sizeof(tmp), "lan%x_ifname", i);
			lan_ifname = nvram_safe_get( tmp);
			strncpy(lan_ifname_save, lan_ifname, sizeof(lan_ifname_save));
			memset(lan_ifnames_save, 0x0, sizeof(lan_ifnames_save));
			snprintf(tmp, sizeof(tmp), "lan%x_ifnames", i);
			lan_ifnames = nvram_safe_get( tmp);
			strncpy(lan_ifnames_save, lan_ifnames, sizeof(lan_ifnames_save));
		}
		if (strncmp(lan_ifname_save, "br", 2) == 0) {
			/*
			 * Read all the bridge members
			 * of this interface
			 */
			foreach(name, lan_ifnames_save, next) {
				if (strncmp(name, "wl", 2) == 0) {
					if (!wl_vif_enabled(name, tmp)) {
						continue; /* Ignore disabled WL VIF */
					}
				}
				/* If a wl i/f, start it */
				wlconf_start(name);
			}/* foreach().... */
		} /* if (strncmp(lan_ifname....*/
		/* specific non-bridged lan i/f */
		else if (strcmp(lan_ifname_save, "")) {
			/* start wireless i/f */
			wlconf_start(lan_ifname_save);
		}
	} /* For loop */

	#ifdef __CONFIG_AUTO_CONN__
	if(!is_doing_status())
	{
		auto_conn_vif_control(0);
	}
	#endif

	return;
}


#define WLN0_PWR_PERCENT				"wl_ctv_pwr_percent"
void set_wl_pwr_percent(void)
{
	int pwr_percent = 100;
	
	char *p_pwr_percent = NULL;

	if (WL_RADIO_OFF == g_cur_wl_radio_status)
	{
		return;
	}
	
	p_pwr_percent = nvram_safe_get(WLN0_PWR_PERCENT);

	if(strcmp(p_pwr_percent,"") != 0)
	{
		pwr_percent = atoi(p_pwr_percent);
	}

	wl_ioctl("eth1", WLC_SET_PWROUT_PERCENTAGE, &pwr_percent, sizeof(pwr_percent));

	return;
}

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
	update_firewall();

// Simon test
	dnsmasq_restart();
#ifdef __CONFIG_DDNS__
	//ddns_restart();
	DDNS_update();
#endif

//roy+++,2010/09/27
#ifdef __CONFIG_TENDA_HTTPD__
	if(nvram_match("rm_web_en", "1"))
		LoadWanListen(1);
	else 
		LoadWanListen(0);
#endif
//+++

//add by ll 20140605
	if(0 == nvram_match("tenda_ate_test", "1"))
	{
		url_record_start() ;
	}
//end by ll 20140605
	diag_printf("is_wan_up = 1\n");
	is_wan_up = 1;

}

/* Stop the wan inet protocols */
static void
wan_inet_stop(void)
{
	diag_printf("is_wan_up = 0\n");
	is_wan_up = 0;
//add by ll
	if(0 == nvram_match("tenda_ate_test", "1"))
	{
		url_record_stop();
	}
//end by ll
	
	stop_firewall();

	dnsmasq_restart();
#ifdef __CONFIG_DDNS__
	//ddns_stop();
#endif
	return;
}

int
add_ns(char *newdns, char *update , const int add_type)
{
	struct in_addr save_dns[NUM_DNS];
	char *value, str[17*NUM_DNS], str2[17*NUM_DNS];
	struct in_addr addr;
	int save, i, j, ret;
	char *name, *p, *next;

	char newdns2[20*3];

	snprintf(newdns2,sizeof(newdns2),"%s",newdns);//roy add

	value = nvram_safe_get("resolv_conf");

	strcpy(str, value);
	for (name = value, p = name, save=0;
	     name && name[0] && save<NUM_DNS;
	     name = next, p = 0) {
		strtok_r(p, " ", &next);
		if (inet_aton(name, &addr) == 0)
			continue;

		save_dns[save++].s_addr = addr.s_addr;
	}
	ret = 0;
	for (name = /*newdns*/newdns2, p = name, i = save;
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
			if (save == 0)
			{
				sprintf(str, "%s", inet_ntoa(*(struct in_addr *)&addr));
			}
			else{
				if(add_type == ADD_NS_PUSH ){
					sprintf(str2,"%s %s",inet_ntoa(*(struct in_addr *)&addr), str);
				}else{
					sprintf(str2,"%s %s",str,inet_ntoa(*(struct in_addr *)&addr));
				}
				sprintf(str, "%s", str2);
			}

			save_dns[save++].s_addr = addr.s_addr;
			if (update) {
				if (ret == 0)
					sprintf(update, "%s", inet_ntoa(*(struct in_addr *)&addr));
				else
					sprintf(update, "%s %s", update, inet_ntoa(*(struct in_addr *)&addr));
				ret ++;
			}
		}
	}
#ifdef __CONFIG_DNS_8_8_8_8_SUPPORT__
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
			sprintf(str, "%s", inet_ntoa(*(struct in_addr *)&addr));
		else
			snprintf(str, sizeof(str),"%s %s", str, inet_ntoa(*(struct in_addr *)&addr));
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
	char *value, str[strlen];
	struct in_addr addr;
	int save, i, j, k;
	char *name, *p, *next;

	value = nvram_safe_get("resolv_conf");

	for (name = value, p = name, save=0;
	     name && name[0] && save<NUM_DNS;
	     name = next, p = 0) {
		strtok_r(p, " ", &next);
		if (inet_aton(name, &addr) == 0)
			continue;
		save_dns[save].s_addr = addr.s_addr;
		save++;
	}

	memset(str, 0, strlen);
	k = 0;
	for (name = deldns, p = name, i=0;
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
		if (j == save)
			continue;

		if (k == 0)
			sprintf(str, "%s", inet_ntoa(*(struct in_addr *)&addr));
		else
			snprintf(str, sizeof(str),"%s %s", str, inet_ntoa(*(struct in_addr *)&addr));
		k++;
	}
	nvram_set("resolv_conf", str);

	/* Init here */
	dns_res_init();

	//dnsmasq_restart();
	return 0;
}

/* WAN interface up */

#ifdef CONFIG_CHINA_NET_CLIENT
	extern int pppoe_auth_encrypt_type;
#endif

extern int
ifr_set_link_speed2(int speed);
void
start_wan(void)
{
	int s;
	char *wan_ifname;
	char *wan_proto;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	struct ifreq ifr;
	char eabuf[32];
#if defined(__CONFIG_DHCPC__) || defined(__CONFIG_L2TP__) || defined(__CONFIG_PPTP__)
	char *value;
#endif

	int mtu;
   	/*add by gong*/
	struct in_addr g_in_ipaddr = {0};
	struct in_addr g_in_netmask = {0};
	struct in_addr g_in_gateway = {0};
	char buf[64];
 	/*end*/

	/* Don't start WAN when in bridge mode */
	if (nvram_match("router_disable", "1"))
		return;
	
	nvram_set("err_check","0");
	nvram_set("wan0_isonln","no");
	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		/* Clear bound_ifname */
		nvram_unset(strcat_r(prefix, "bind_ifname", tmp));
		nvram_unset(strcat_r(prefix, "bind_proto", tmp));
		nvram_unset(strcat_r(prefix, "bind_pptp_static", tmp));
		nvram_unset(strcat_r(prefix, "bind_l2tp_static", tmp));
		nvram_unset(strcat_r(prefix, "bind_pppoe_static", tmp));
		
//roy +++,2010/09/30		
		nvram_unset(strcat_r(prefix, "connect", tmp));
//+++		
		/* make sure the connection exists and is enabled */ 
		wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		if (!wan_ifname)
			continue;
		wan_proto = nvram_get(strcat_r(prefix, "proto", tmp));
		if (!wan_proto || !strcmp(wan_proto, "disabled"))
			continue;

		/* disable the connection if the i/f is not in wan_ifnames */
		if (!wan_valid(wan_ifname)) {
			nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
			continue;
		}
//tenda add,解决MAC地址clone不起作用的问题 
		if (wl_probe(wan_ifname) == 0)
			ifconfig(wan_ifname, IFF_UP, "0.0.0.0", NULL);
		else
			ifconfig(wan_ifname, 0, NULL, NULL);

		/* Set i/f hardware address before bringing it up */
		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
			printf("%s()::socket\n", __FUNCTION__);
			continue;
		}		
		strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
		ifr.ifr_hwaddr.sa_family = AF_LINK;//roy add

		/* Configure i/f only once, specially for wl i/f shared by multiple connections */
		if (ioctl(s, SIOCGIFFLAGS, &ifr)) {
			close(s);
			continue;
		}

		if (!(ifr.ifr_flags & IFF_UP)) {
			/* Sync connection nvram address and i/f hardware address */
			memset(ifr.ifr_hwaddr.sa_data, 0, ETHER_ADDR_LEN);
			if (!nvram_invmatch(strcat_r(prefix, "hwaddr", tmp), "") ||
			    !ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)),
					(unsigned char *)ifr.ifr_hwaddr.sa_data) ||
			    !memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN)) {
				if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
					close(s);
					continue;
				}
				nvram_set(strcat_r(prefix, "hwaddr", tmp),
					  ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
			} else {
				ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
				ioctl(s, SIOCSIFHWADDR, &ifr);
			}
			/* Bring up i/f */
			ifconfig(wan_ifname, IFUP, NULL, NULL);

			/* do wireless specific config */
			if (nvram_match("ure_disable", "1")) {
				wlconf(wan_ifname);
				wlconf_start(wan_ifname);
			}
		}
		close(s);

		/* Set initial QoS mode again now that WAN port is ready. */
		wan_set_et_qos_mode(unit);

		nvram_unset("resolv_conf");
		
		//roy remove,2010/09/14	
		//add_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)), NULL);

		/* Set bound ifname and proto before up the wan interface */
		nvram_set(strcat_r(prefix, "bind_ifname", tmp), wan_ifname);
		nvram_set(strcat_r(prefix, "bind_proto", tmp), wan_proto);
//roy+++ for pppoe xkjs		
#ifdef CONFIG_CHINA_NET_CLIENT
		if(strcmp(wan_proto, "pppoe") == 0){
			//pppoe_auth_encrypt_type = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_prio_xkjs", tmp)));
			pppoe_auth_encrypt_type = 0;
		}else{
			pppoe_auth_encrypt_type = 0;
		}
#endif
//huangxiaoli add for wan speed
	/*	if(nvram_get("wan_speed")){
			ifr_set_link_speed2(atoi(nvram_get("wan_speed")));
		}*/
		/* Read wan mode from NVRAM */
		if (strcmp(wan_proto, "static") == 0) {
			/* Assign static IP address to i/f */
			/*    gong modify 
			         ifconfig(wan_ifname, IFUP,
				 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
				 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
			*/
			//gong modify start ,解决变长子网掩码客诉 
			inet_aton(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), &g_in_ipaddr);
			inet_aton( nvram_safe_get(strcat_r(prefix, "netmask", tmp)), &g_in_netmask);
			inet_aton( nvram_safe_get(strcat_r(prefix, "gateway", tmp)), &g_in_gateway);
			if((g_in_ipaddr.s_addr & g_in_netmask.s_addr) == (g_in_gateway.s_addr & g_in_netmask.s_addr)){
				ifconfig(wan_ifname, IFUP,
				nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
  				nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
			}else{
				ifconfig(wan_ifname, IFUP,
  				nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), "255.0.0.0");
 			}
			//gong modify end
//roy+++ for dns,2010/09/14	
			//set mtu
			mtu = atoi(nvram_safe_get(strcat_r(prefix, "mtu", tmp)));
			if(mtu > 0)
				ifconfig_mtu(wan_ifname,mtu);
			/*
			value = nvram_safe_get(strcat_r(prefix, "dns", tmp));
			add_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)), NULL);
			*/
			//gong modify 添加默认dns服务器 8.8.8.8，不显示到页面，只添到resolv_conf
			value = nvram_safe_get(strcat_r(prefix, "dns", tmp));
#ifdef __CONFIG_DNS_8_8_8_8_SUPPORT__
			sprintf(buf,"%s %s",value,"8.8.8.8");
#else
			sprintf(buf,"%s",value);		
#endif
			//printf("------%s %d %s %s\n",__func__,__LINE__,value,buf);
			add_ns(buf, NULL, ADD_NS_PUSH);
			//gong modify end
//+++			
			/* We are done configuration */
			wan_up(wan_ifname);
		}
#ifdef __CONFIG_DHCPC__
		else if (strcmp(wan_proto, "dhcp") == 0) {
			value = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
//roy+++,2010/09/30
			nvram_set("wan0_isonln","yes-no");
			nvram_set(strcat_r(prefix, "connect", tmp), "Connecting");
//+++
			dhcpc_start(wan_ifname, "wandhcpc", value);

			//add for nex 201210,添加dhcp option12选项解决上级连接cable modem获取不到正常IP问题
			//dhcpc_start(wan_ifname, "wandhcpc", "nex_net05");
			//end add

		}
#endif
#ifdef __CONFIG_8021X__
		else if (strcmp(wan_proto, "8021x") == 0) {
			X8021_start(wan_ifname);
		}
#endif
#ifdef __CONFIG_PPPOE__
		else if (strcmp(wan_proto, "pppoe") == 0){			
			wan_pppoe_start(unit);
		}
#endif
#ifdef __CONFIG_PPTP__
		else if (strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "pptp2") == 0) {
#ifdef __CONFIG_DHCPC__
			value = nvram_get(strcat_r(prefix, "pptp_static", tmp));
			if (value && atoi(value) == 0) {
				/* Save tunnel bind proto */
				nvram_set(strcat_r(prefix, "bind_pptp_static", tmp), "0");
				
				/* Start dhcp */
				value = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
				//diag_printf("start dhcp %s: %s\n",  tmp, value);
				nvram_set(strcat_r(prefix, "connect", tmp), "Connecting");   //pxy 2012.10.27
				
				dhcpc_start(wan_ifname, "pptpdhcpc", value);
			}
			else
#endif /* __CONFIG_DHCPC__ */
			{
//roy+++ for dns,2010/09/14
			//pptp_static
				//if ((value = nvram_safe_get(strcat_r(prefix, "pptp_dns", tmp)))) {
				if ((value = nvram_safe_get(strcat_r(prefix, "dns", tmp)))) {
				//tenda modify
					//del_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)));
					//nvram_set(strcat_r(prefix, "dns", tmp), value);
					add_ns(value, NULL, ADD_NS_PUSH);
					//diag_printf("pptp_static pptp_dns: %s\n", value);
				}
//+++
				nvram_set(strcat_r(prefix, "connect", tmp), "Connecting");   //pxy 2012.10.27
				wan_pptp_start(unit);
			}
		}
#endif
#ifdef __CONFIG_L2TP__
		else if (strcmp(wan_proto, "l2tp") == 0) {
#ifdef __CONFIG_DHCPC__
			value = nvram_get(strcat_r(prefix, "l2tp_static", tmp));
			if (value && atoi(value) == 0) {
				/* Save tunnel bind proto */
				nvram_set(strcat_r(prefix, "bind_l2tp_static", tmp), "0");

				/* Start dhcp */
				value = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
				nvram_set(strcat_r(prefix, "connect", tmp), "Connecting");   //pxy 2012.10.27
				
				dhcpc_start(wan_ifname, "l2tpdhcpc", value);
			}
			else
#endif
			{
//roy+++ for dns,2010/09/14
			//l2tp_static
				//if ((value = nvram_safe_get(strcat_r(prefix, "l2tp_dns", tmp)))) {
				if ((value = nvram_safe_get(strcat_r(prefix, "dns", tmp)))) {
				//tenda modify
					//del_ns(nvram_safe_get(strcat_r(prefix, "dns", tmp)));
					//nvram_set(strcat_r(prefix, "dns", tmp), value);
					add_ns(value, NULL, ADD_NS_PUSH);
				}
//+++
				nvram_set(strcat_r(prefix, "connect", tmp), "Connecting");   //pxy 2012.10.27

				wan_l2tp_start(unit);
			}
		}
#endif /* __CONFIG_L2TP__ */
#ifdef __CONFIG_PPPOE2__
		else if (strcmp(wan_proto, "pppoe2") == 0) {
#ifdef __CONFIG_DHCPC__
			value = nvram_get(strcat_r(prefix, "pppoe_static", tmp));
			if (value && atoi(value) == 0) {
				/* Start dhcp */
				nvram_set(strcat_r(prefix, "bind_pppoe_static", tmp), "0");
				value = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
				dhcpc_start(wan_ifname, "pppoe2dhcpc", value);
			}
			else
#endif
			{
				nvram_set(strcat_r(prefix, "bind_pppoe_static", tmp), "1");
				wan_pppoe2_start(unit);
			}
		}
#endif /* __CONFIG_PPPOE2__ */

		/* Save wan status */
		//nvram_set(strcat_r(prefix, "connect", tmp), "Connected");//roy remove to wan_up
	}
}

/* WAN interface down */
void
stop_wan(void)
{
	char *wan_ifname;
	char *wan_proto;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		wan_ifname = nvram_get(strcat_r(prefix, "bind_ifname", tmp));
		if (!wan_ifname)
			continue;

		wan_proto = nvram_get(strcat_r(prefix, "bind_proto", tmp));
		if (!wan_proto || !strcmp(wan_proto, "disabled"))
			continue;

		if (strcmp(wan_proto, "static") == 0) {
			wan_down(wan_ifname);
		}
#ifdef __CONFIG_DHCPC__
		else if (strcmp(wan_proto, "dhcp") == 0) {
			dhcpc_stop(wan_ifname);
			//char *pppname_server = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			//wan_pppoe_down(pppname_server);//gong
		}
#endif
#ifdef __CONFIG_8021X__
		else if (strcmp(wan_proto, "8021x") == 0) {
			X8021_stop(wan_ifname);

			if(1 == atoi(nvram_safe_get("wan0_1x_bind_ardmode")))
			{
				wan_down(wan_ifname);
			}else{
				dhcpc_stop(wan_ifname);
			}
		}
#endif
#ifdef __CONFIG_PPPOE__
		else if (strcmp(wan_proto, "pppoe") == 0) {
			char *pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			wan_pppoe_down(pppname);
		}
#endif
#ifdef __CONFIG_PPTP__
		else if (strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "pptp2") == 0) {
			char *pppname = nvram_safe_get(strcat_r(prefix, "pptp_ifname", tmp));
			wan_pptp_down(pppname);
		}
#endif
#ifdef __CONFIG_L2TP__
		else if (strcmp(wan_proto, "l2tp") == 0) {
			char *pppname = nvram_safe_get(strcat_r(prefix, "l2tp_ifname", tmp));
			wan_l2tp_down(pppname);
		}
#endif
#ifdef __CONFIG_PPPOE2__
		else if (strcmp(wan_proto, "pppoe2") == 0) {
			char *pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			wan_pppoe2_down(pppname);
		}
#endif

		/* Save wan status */
		nvram_set(strcat_r(prefix, "connect", tmp), "Disconnected");
		//nvram_set("onlncheck","yes");
		nvram_set("err_check","0");
		nvram_set("wan0_isonln","no");
	} /* for */
}

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

void
wan_up(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto;

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
			route_add(wan_ifname, 0, "0.0.0.0",
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
	primary_wan_ip_set.conn_time= time(0);
//+++	

	nvram_set("wan0_connect", "Connected");
	nvram_set("wan0_isonln", "yes-no");

#ifndef __CONFIG_NETBOOT__
	/* Sync time */
	if (nvram_match("time_mode", "0"))//roy add
		start_ntpc();
#endif

	/* Start inet */
	wan_inet_start();

//roy+++ for pppoe xkjs		
#ifdef CONFIG_CHINA_NET_CLIENT
		if(strcmp(wan_proto, "pppoe") == 0){
			char pppoe_prio_xkjs[12]={0};
			if(pppoe_auth_encrypt_type != atoi(nvram_safe_get(strcat_r(prefix, "pppoe_prio_xkjs", tmp)))){
				sprintf(pppoe_prio_xkjs,"%d",pppoe_auth_encrypt_type);
				nvram_set(strcat_r(prefix, "pppoe_prio_xkjs", tmp),pppoe_prio_xkjs);
				nvram_commit();
			}
		}
#endif

//nvram_set("onlncheck","yes");
nvram_set("wan0_isonln","yes-no");
//tenda add
	dhcpd_renew_lease();
//---
}

void
wan_down(char *wan_ifname)
{
	/* pxy rm, 2014.01.23 */
#if 0
#ifndef __CONFIG_NETBOOT__//roy add
	/* Sync time */
	stop_ntpc();
#endif
#endif
	/* Stop wan serverices */
	wan_inet_stop();

	if (wl_probe(wan_ifname) == 0)
		ifconfig(wan_ifname, IFF_UP, "0.0.0.0", NULL);
	else
		ifconfig(wan_ifname, 0, NULL, NULL);

	/* Flush routes belong to this interface */
	route_flush(wan_ifname);
	
	/* tenda add the next */
#if 1
	char prefix[] = "wanXXXXXXXXXX_";
	char tmp[100];
	snprintf(prefix, sizeof(prefix), "wan%d_", 0);
	nvram_unset(strcat_r(prefix, "connect", tmp));
	nvram_set("wan0_isonln","no");
#endif
}
#endif	/* __CONFIG_NAT__ */

void
lan_up(char *lan_ifname)
{
	/* Install default route to gateway - AP only */
	if (nvram_match("router_disable", "1") && nvram_invmatch("lan_gateway", ""))
		route_add(lan_ifname, 0, "0.0.0.0", nvram_safe_get("lan_gateway"), "0.0.0.0");

	/* Install interface dependent static routes */
	add_lan_routes(lan_ifname);

	/* Restart modules */
	lan_inet_stop();
	lan_inet_start();
}

void
lan_down(char *lan_ifname)
{
	/* Remove default route to gateway - AP only */
	if (nvram_match("router_disable", "1") && nvram_invmatch("lan_gateway", ""))
		route_del(lan_ifname, 0, "0.0.0.0", nvram_safe_get("lan_gateway"), "0.0.0.0");

	/* Remove interface dependent static routes */
	del_lan_routes(lan_ifname);
}

#ifdef __CONFIG_NAT__
int
wan_ifunit(char *wan_ifname)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if ((unit = ppp_ifunit(wan_ifname)) >= 0)
		return unit;
	else {
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
	ifconfig("lo0", IFUP, "127.0.0.1", "255.0.0.0");
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
	//wan0_pppoe_demand
	demand = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_demand", tmp)));

	if(demand == 0){
		//auto
		return;
	}else if(demand == 1){
		//dial on traffic
		//check every idel_time
		idel_time = (unsigned int)strtol(nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp)), NULL, 10);
		if(dial_on_traffic_check_time >= idel_time){
			//diag_printf("====dns_query_on_idel_time=%d=========\n",dns_query_on_idel_time);
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

void pppoe_connect_rc()
{
	int unit;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";
	
	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	WAN_PREFIX(unit, prefix);

	char *pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	
	pppoe_connect(pppname);
}
void pppoe_disconnect_rc()
{
	int unit;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";
	
	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	WAN_PREFIX(unit, prefix);

	char *pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	
	pppoe_disconnect(pppname);
}
//
