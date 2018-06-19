/*
 * Config data profile utility.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: nvram_ecos.c,v 1.8 2010-10-18 11:48:10 Exp $
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <typedefs.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <epivers.h>
#include <router_version.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <flashutl.h>
#include <shutils.h>
#include <netconf.h>
#include <bcmparams.h>
#include <nvparse.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <ezc.h>
#include "sys_backupcfg.h"


#define cfg_log     diag_printf
#ifdef __ECOS
#define	isdigit(x)	((x) >= '0' && (x) <= '9')
#endif

//roy+++,2010/09/26
#ifndef NVRAM_MAX_STRINGSIZE
#define NVRAM_MAX_STRINGSIZE 256 //from bcmcvar.h
#endif
//+++

#define CFG_INIT_LOCK()
#define CFG_UNLOCK()  /* cyg_scheduler_unlock() */
#define CFG_LOCK()    /* cyg_scheduler_lock() */

#ifdef	CONFIG_NETBOOT
#define RESTORE_DEFAULTS() \
	!nvram_match("restore_defaults", "0")
#else
	#define RESTORE_DEFAULTS() \
		(!nvram_match("restore_defaults", "0") || \
		nvram_invmatch("os_name", "linux") || \
		nvram_invmatch("ecos_name", "ecos"))
#endif//#ifdef	CONFIG_NETBOOT

#define	WL_PREFIX "wl0_"
#define	WLXdotY_PREFIX "wl0.1_"

extern struct nvram_tuple router_defaults[];
//extern void set_pin_code(void);
extern void board_reboot(void);
extern uint32 wps_gen_pin(char *devPwd, int devPwd_len);


int nvram_erase(void);

/*static*///roy remove,2010/09/26
int
find_nvram(char *var, bool full_match)
{
	struct nvram_tuple *t;
	int len = strlen(var);
	int found = 0;
	int i;
	char *c;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if (len == 0)
		return -1;

	if (!strncmp(var, "wan_", 4) && strcmp(var,"wan_ifnames") != 0)/*we need wan_ifnames*/
		goto done;

	/* check full name match first */
	for (t = router_defaults; t->name; t++) {
		if (!strcmp(var, t->name)) {
			found = 1;
			break;
		}
	}

//roy +++,2010/11/23
	if(!found){
			if(!strncmp(var,"wl0_",4) ||!strncmp(var,"wl0.1_",6) ){
				found = 1;
			}
	}
//过滤掉一些内容,不让客户看到
	if(found){
		if(/*strstr(var,"ifnames") || strstr(var,"ifname") || */strstr(var,"te_version") || 
			strstr(var,"wme") || strstr(var,"rtylimit") || strstr(var,"radarthrs") )
			found = 0;
	}
//+++

	if (full_match)
		goto done;
//这下面不会执行,因为我们传入的full_match为真
	/* check partial name match */
	if (found == 0) {
		/* Need to check wlX_YYY and wlX.Z_YYY case */
		for (t = router_defaults; t->name; t++) {
			if (!strncmp(t->name, "wl_", 3) && !strncmp(var, "wl", 2)) {
				/* wlX.Z_YYY */
				if (strstr(var, ".")) {
					if (!strcmp(&t->name[3], var+strlen(WLXdotY_PREFIX))) {
						found = 1;
						break;
					}
				}
				/* wlX_YYY */
				else {
					if (!strcmp(&t->name[3], var+strlen(WL_PREFIX))) {
						found = 1;
						break;
					}
				}
			}

			/* We need to take special care of the wan_ variables for multiple
			 * WAN configuration immigrated from the Linux platform.
			 */
			if (!strncmp(t->name, "wan_", 4) && !strncmp(var, "wan", 3)) {
				for (i = 3; i < len; i++) {
					c = var + i;
					if (*c < '0' || *c > '9')
						break;
				}
				if (*(var+i) != '_')
					break;
				strncpy(prefix, var, i + 1);
				prefix[i+1] = 0;
				if (strcmp(var + i + 1, "primary") != 0) {
					if (!nvram_match(strcat_r(prefix, "primary", tmp), "1"))
						break;
					if (!strcmp(var + i + 1, "ipaddr") ||
					    !strcmp(var + i + 1, "netmask") ||
					    !strcmp(var + i + 1, "gateway") ||
					    !strcmp(var + i + 1, "dns") ||
					    !strcmp(var + i + 1, "wins") ||
					    !strcmp(var + i + 1, "domain")) {
						if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
						    nvram_match(strcat_r(prefix, "proto", tmp), "pppoe"))
							break;
					}
				}
				else {
					/* We assume the first _primary variable will be set to 0 */
					if (!nvram_get(strcat_r(prefix, "primary", tmp))) {
						break;
					}
				}

				if (!strcmp(&t->name[3], var + i)) {
					found = 1;
					break;
				}
			}
		}
	}
done:
	return found;
}

static int
build_ifnames(char *type, char *names, int *size)
{
	char name[32], *next;
	int len = 0;
	int s;

	/* open a raw scoket for ioctl */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	/*
	 * go thru all device names (wl<N> il<N> et<N> vlan<N>) and interfaces to 
	 * build an interface name list in which each i/f name coresponds to a device
	 * name in device name list. Interface/device name matching rule is device
	 * type dependant:
	 *
	 *	wl:	by unit # provided by the driver, for example, if eth1 is wireless
	 *		i/f and its unit # is 0, then it will be in the i/f name list if
	 *		wl0 is in the device name list.
	 *	il/et:	by mac address, for example, if et0's mac address is identical to
	 *		that of eth2's, then eth2 will be in the i/f name list if et0 is 
	 *		in the device name list.
	 *	vlan:	by name, for example, vlan0 will be in the i/f name list if vlan0
	 *		is in the device name list.
	 */
	foreach(name, type, next) {
		struct ifreq ifr;
		int i, unit;
		char var[32], *mac;
		unsigned char ea[ETHER_ADDR_LEN];

		/* vlan: add it to interface name list */
		if (!strncmp(name, "vlan", 4)) {
			/* append interface name to list */
			len += snprintf(&names[len], *size - len, "%s ", name);
			continue;
		}

		/* others: proceed only when rules are met */
		for (i = 1; i <= DEV_NUMIFS; i ++) {
			/* ignore i/f that is not ethernet */
			ifr.ifr_ifindex = i;
			if (ioctl(s, SIOCGIFNAME, &ifr))
				continue;
			if (ioctl(s, SIOCGIFHWADDR, &ifr))
				continue;
			if (ifr.ifr_hwaddr.sa_family != AF_LINK)
				continue;
			if (!strncmp(ifr.ifr_name, "vlan", 4))
				continue;

			/* wl: use unit # to identify wl */
			if (!strncmp(name, "wl", 2)) {
				if (wl_probe(ifr.ifr_name) ||
				    wl_ioctl(ifr.ifr_name, WLC_GET_INSTANCE, &unit, sizeof(unit)) ||
				    unit != atoi(&name[2]))
					continue;
			}
			/* et/il: use mac addr to identify et/il */
			else if (!strncmp(name, "et", 2) || !strncmp(name, "il", 2)) {
				snprintf(var, sizeof(var), "%smacaddr", name);
				if (!(mac = nvram_get(var)) || !ether_atoe(mac, ea) ||
				    bcmp(ea, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
					continue;
			}
			/* mac address: compare value */
			else if (ether_atoe(name, ea) &&
				!bcmp(ea, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
				;
			/* others: ignore */
			else
				continue;

			/* append interface name to list */
			len += snprintf(&names[len], *size - len, "%s ", ifr.ifr_name);
		}
	}
	
	close(s);

	*size = len;
	return 0;
}

static void
wps_restore_defaults(void)
{
	/* cleanly up nvram for WPS */
	nvram_unset("wps_seed");
	nvram_unset("wps_config_state");
	nvram_unset("wps_addER");
	nvram_unset("wps_device_pin");
	nvram_unset("wps_pbc_force");
	nvram_unset("wps_config_command");
	nvram_unset("wps_proc_status");
	nvram_unset("wps_status");
	nvram_unset("wps_method");
	nvram_unset("wps_proc_mac");
	nvram_unset("wps_sta_pin");
	nvram_unset("wps_currentband");
	nvram_unset("wps_restart");
	nvram_unset("wps_event");
	nvram_unset("wps_restart");

	nvram_unset("wps_enr_mode");
	nvram_unset("wps_enr_ifname");
	nvram_unset("wps_enr_ssid");
	nvram_unset("wps_enr_bssid");
	nvram_unset("wps_enr_wsec");

	nvram_unset("wps_unit");
}

static void
virtual_radio_restore_defaults(void)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXX_mssid_";
	int i, j;
	
	nvram_unset("unbridged_ifnames");
	nvram_unset("ure_disable");
	
	/* Delete dynamically generated variables */
	for (i = 0; i < MAX_NVPARSE; i++) {
		sprintf(prefix, "wl%d_", i);
		nvram_unset(strcat_r(prefix, "vifs", tmp));
		nvram_unset(strcat_r(prefix, "ssid", tmp));
		nvram_unset(strcat_r(prefix, "guest", tmp));
		nvram_unset(strcat_r(prefix, "ure", tmp));
		nvram_unset(strcat_r(prefix, "ipconfig_index", tmp));
		nvram_unset(strcat_r(prefix, "nas_dbg", tmp));
		sprintf(prefix, "lan%d_", i);
		nvram_unset(strcat_r(prefix, "ifname", tmp));
		nvram_unset(strcat_r(prefix, "ifnames", tmp));
		nvram_unset(strcat_r(prefix, "gateway", tmp));
		nvram_unset(strcat_r(prefix, "proto", tmp));
		nvram_unset(strcat_r(prefix, "ipaddr", tmp));
		nvram_unset(strcat_r(prefix, "netmask", tmp));
		nvram_unset(strcat_r(prefix, "lease", tmp));
		nvram_unset(strcat_r(prefix, "stp", tmp));
		nvram_unset(strcat_r(prefix, "hwaddr", tmp));
		sprintf(prefix, "dhcp%d_", i);
		nvram_unset(strcat_r(prefix, "start", tmp));
		nvram_unset(strcat_r(prefix, "end", tmp));
		
		/* clear virtual versions */
		for (j=0; j< 16;j++){
			sprintf(prefix, "wl%d.%d_", i,j);
			nvram_unset(strcat_r(prefix, "ssid", tmp));
			nvram_unset(strcat_r(prefix, "ipconfig_index", tmp));
			nvram_unset(strcat_r(prefix, "guest", tmp));
			nvram_unset(strcat_r(prefix, "closed", tmp));
			nvram_unset(strcat_r(prefix, "wpa_psk", tmp));
			nvram_unset(strcat_r(prefix, "auth", tmp));
			nvram_unset(strcat_r(prefix, "wep", tmp));
			nvram_unset(strcat_r(prefix, "auth_mode", tmp));
			nvram_unset(strcat_r(prefix, "crypto", tmp));
			nvram_unset(strcat_r(prefix, "akm", tmp));
			nvram_unset(strcat_r(prefix, "hwaddr", tmp));
			nvram_unset(strcat_r(prefix, "bss_enabled", tmp));
			nvram_unset(strcat_r(prefix, "bss_maxassoc", tmp));
			nvram_unset(strcat_r(prefix, "wme_bss_disable", tmp));
			nvram_unset(strcat_r(prefix, "ifname", tmp));
			nvram_unset(strcat_r(prefix, "unit", tmp));
			nvram_unset(strcat_r(prefix, "ap_isolate", tmp));
			nvram_unset(strcat_r(prefix, "macmode", tmp));
			nvram_unset(strcat_r(prefix, "maclist", tmp));
			nvram_unset(strcat_r(prefix, "maxassoc", tmp));
			nvram_unset(strcat_r(prefix, "mode", tmp));
			nvram_unset(strcat_r(prefix, "radio", tmp));
			nvram_unset(strcat_r(prefix, "radius_ipaddr", tmp));
			nvram_unset(strcat_r(prefix, "radius_port", tmp));
			nvram_unset(strcat_r(prefix, "radius_key", tmp));
			nvram_unset(strcat_r(prefix, "key", tmp));
			nvram_unset(strcat_r(prefix, "key1", tmp));
			nvram_unset(strcat_r(prefix, "key2", tmp));
			nvram_unset(strcat_r(prefix, "key3", tmp));
			nvram_unset(strcat_r(prefix, "key4", tmp));
			nvram_unset(strcat_r(prefix, "wpa_gtk_rekey", tmp));
			nvram_unset(strcat_r(prefix, "nas_dbg", tmp));
		}
	}
}

static void
upgrade_defaults(void)
{
	char temp[100];
	int i;
	bool bss_enabled = TRUE;
	char *val;

	/* Check whether upgrade is required or not
	 * If lan1_ifnames is not found in NVRAM , upgrade is required.
	 */
	//if (!nvram_get("lan1_ifnames") && !RESTORE_DEFAULTS()) {
	if ((!nvram_get("lan1_ifnames") || !nvram_get("boardflags")) && !RESTORE_DEFAULTS()) {
		cfg_log("NVRAM upgrade required.  Starting.\n");
/*
roy add for et_probe: et_attach() failed problem when we commit flash and power off,the nvram data is lose,
if boardflags is NULL, the interface can't be initial correct(check function: restore_default), so nvram is damaged, 
erase nvarm here,then the nvram will recover correct after reboot.
*/
/*
PCI: no core
PCI: Fixing up bus 0
et_probe: et_attach() failed
Restoring defaults...eth0 mac: 00:90:4C:00:25:C2
Generate new WPS PIN = 00096669, oldpin= 12345670
Generate new WPS SSID = Tenda_0025C2
Found an ST compatible serial flash with 256 4KB blocks; total size 1MB
done

eCos Router/AP V5.110.27.7 (Compiled at 15:16:26 on Jul 18 2011) 
sys_led_test_gpio=6
Using pin 6 for sys_led output
wps_led_test_gpio=7
System start
VLAN interfaces not supported
start_lan():: ifconfig up failed
start_lan():: ifconfig up failed
start_lan():: ifconfig up failed
start_lan():: ifconfig up failed
*/
		nvram_erase();
		board_reboot();
		return;
//end
		if (nvram_match("ure_disable", "1")) {
			nvram_set("lan1_ifname", "br1");
			nvram_set("lan1_ifnames", "wl0.1 wl0.2 wl0.3 wl1.1 wl1.2 wl1.3");
		}
		else {
			nvram_set("lan1_ifname", "");
			nvram_set("lan1_ifnames", "");
			for (i = 0; i < 2; i++) {
				snprintf(temp, sizeof(temp), "wl%d_ure", i);
				if (nvram_match(temp, "1")) {
					snprintf(temp, sizeof(temp), "wl%d.1_bss_enabled", i);
					nvram_set(temp, "1");
				}
				else {
					bss_enabled = FALSE;
					snprintf(temp, sizeof(temp), "wl%d.1_bss_enabled", i);
					nvram_set(temp, "0");
				}
			}
		}
		if (nvram_get("lan1_ipaddr")) {
			nvram_set("lan1_gateway", nvram_get("lan1_ipaddr"));
		}

		for (i = 0; i < 2; i++) {
			snprintf(temp, sizeof(temp), "wl%d_bss_enabled", i);
			nvram_set(temp, "1");
			snprintf(temp, sizeof(temp), "wl%d.1_guest", i);
			if (nvram_match(temp, "1")) {
				nvram_unset(temp);
				if (bss_enabled) {
					snprintf(temp, sizeof(temp), "wl%d.1_bss_enabled", i);
					nvram_set(temp, "1");
				}
			}

			snprintf(temp, sizeof(temp), "wl%d.1_net_reauth", i);
			val = nvram_get(temp);
			if (!val || (*val == 0))
				nvram_set(temp, nvram_default_get(temp));

			snprintf(temp, sizeof(temp), "wl%d.1_wpa_gtk_rekey", i);
			val = nvram_get(temp);
			if (!val || (*val == 0))
				nvram_set(temp, nvram_default_get(temp));
		}

		nvram_commit();

		cfg_log("NVRAM upgrade complete.\n");
	}
}


void set_ssid_mac(void){
	char wlanmac[64]={'\0'};
	char prev_ssid[64]={'\0'};
	char ssid_mac[64]={'\0'};
	
	int i = 0, l = 0;
	if(nvram_get("et0macaddr")){
		strcpy(wlanmac,nvram_get("et0macaddr"));
	}
	else{
		cfg_log("macaddr is empty!!!\n");
		return;
	}

	if(nvram_get("wl_ssid")){
		strcpy(prev_ssid, nvram_get("wl_ssid"));
	}else{
		cfg_log("wl_ssid is empty!!!\n");
		return;
	}

	for(i=0;  wlanmac[i]!='\0' && i<9; ++i){
		if (wlanmac[i+9] != ':'){		
				wlanmac[l]=wlanmac[i+9];
				++l;
			}
	}
	wlanmac[6]='\0';

	snprintf(ssid_mac, sizeof(ssid_mac),"%s_%s", prev_ssid, wlanmac);

	nvram_set("wl_ssid", ssid_mac);
	nvram_set("wl0_ssid", ssid_mac);
	
	return;
}

void set_ssid_default()
{
	char *value;
	
	if((value = nvram_get("wl_ssid"))==NULL)
		return;

	nvram_set("default_ssid", value);

	return;
}

#ifdef __CONFIG_AUTO_CONN__
int
mac_add(char *macaddr, char *result, unsigned int add_num)
{
	int i = 0;
	unsigned char mac_ocet[6] = {0};
	unsigned int mac_value = 0;

	if(macaddr == NULL || result == NULL)
		return 0;
	
	for (;;) {
		mac_ocet[i++] = (char) strtoul(macaddr, &macaddr, 16);
		if (!*macaddr++ || i == 6)
			break;
	}

	if(i == 6)
	{
		mac_value = (mac_ocet[3]&0XFF);
		mac_value = (mac_value<<8) + (mac_ocet[4]&0XFF);
		mac_value = (mac_value<<8) + (mac_ocet[5]&0XFF);

		mac_value += add_num;
		sprintf(result, "%02X:%02X:%02X:%02X:%02X:%02X", mac_ocet[0], mac_ocet[1], mac_ocet[2],
			(mac_value>>16)&0XFF, (mac_value>>8)&0XFF, (mac_value)&0XFF);

		return 1;
	}

	return 0;
}
#endif
void set_wan0_mac(void)
{
	char vlan2mac[64]={0};
	
	if(nvram_get("et0macaddr")){
		strcpy(vlan2mac,nvram_get("et0macaddr"));
	}
	else{
		cfg_log("macaddr is empty!!!\n");
		return;
	}

	#ifdef __CONFIG_AUTO_CONN__
	char new_mac[64] = {0};
	mac_add(vlan2mac, new_mac, 4);
	strcpy(vlan2mac, new_mac);
	#endif
	
	nvram_set("wan0_hwaddr", vlan2mac);
	
	return;
}

#ifdef __CONFIG_ALINKGW__
#define ALILINK_WLAN_TPSKLIST_MAX	128
void restore_tpsk_defaults()
{
	printf("Restore tpsk config defaults...\n");
	int i ;
	char name[32] = {0};
	
	nvram_set("alilink_wlan_tpsklist_num", "0");
	for (i = 1; i <= ALILINK_WLAN_TPSKLIST_MAX; i++) {
		sprintf(name, "%s%d", "alilink_wlan_tpsklist", i);
		nvram_unset(name);
	}

	return;
}
#endif

/*static*/ void
restore_defaults(void)
{
	
	struct nvram_tuple generic[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "eth0 eth2 eth3 eth4", 0 },
		{ "wan_ifname", "eth1", 0 },
		{ "wan_ifnames", "eth1", 0 },
		{ "lan1_ifname", "br1", 0 },
		{ "lan1_ifnames", "wl0.1 wl0.2 wl0.3 wl1.1 wl1.2 wl1.3", 0 },
		{ 0, 0, 0 }
	};
#ifdef CONFIG_VLAN
	struct nvram_tuple vlan[] = {
		{ "lan_ifname", "br0", 0 },
#if 0	//tenda modify		
		{ "lan_ifnames", "vlan0 eth1 eth2 eth3", 0 },
		{ "wan_ifname", "vlan1", 0 },
		{ "wan_ifnames", "vlan1", 0 },
#else
		{ "lan_ifnames", "vlan1 eth1", 0 },
		{ "wan_ifname", "vlan2", 0 },
		{ "wan_ifnames", "vlan2", 0 },
#endif		
		{ "lan1_ifname", "br1", 0 },
		{ "lan1_ifnames", "wl0.1 wl0.2 wl0.3 wl1.1 wl1.2 wl1.3", 0 },
		{ 0, 0, 0 }
	};
#endif	/* CONFIG_VLAN */
	struct nvram_tuple dyna[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "vlan1 eth1", 0 },
		{ "wan_ifname", "vlan2", 0 },
		{ "wan_ifnames", "vlan2", 0 },
		{ "lan1_ifname", "br1", 0 },
		{ "lan1_ifnames", "wl0.1 wl0.2 wl0.3 wl1.1 wl1.2 wl1.3", 0 },
		{ 0, 0, 0 }
	};

	struct nvram_tuple *linux_overrides;
	struct nvram_tuple *t, *u;
	int restore_defaults, i;
//	char *value;
//	value=nvram_get("wps_device_pin");
#ifdef CONFIG_VLAN
	uint boardflags;
#endif	/* CONFIG_VLAN */
	char *landevs, *wandevs;
	char lan_ifnames[128], wan_ifnames[128];
	char wan_ifname[32], *next;
	int len;
	int ap = 0;

	char old_pin[16];
	/* Restore defaults if told to or OS has changed */
	restore_defaults = RESTORE_DEFAULTS();

	if (restore_defaults){
		cfg_log("Restoring defaults...");
		strcpy(old_pin,nvram_safe_get("wps_device_pin"));
	}

	/* Delete dynamically generated variables */
	if (restore_defaults) {
		printf("start restore default....");
		char tmp[100], prefix[] = "wlXXXXXXXXXX_";
		for (i = 0; i < MAX_NVPARSE; i++) {
			snprintf(prefix, sizeof(prefix), "wl%d_", i);
			for (t = router_defaults; t->name; t ++) {
				if (!strncmp(t->name, "wl_", 3))
					nvram_unset(strcat_r(prefix, &t->name[3], tmp));
			}
#ifdef __CONFIG_NAT__
			snprintf(prefix, sizeof(prefix), "wan%d_", i);
			for (t = router_defaults; t->name; t ++) {
				if (!strncmp(t->name, "wan_", 4))
					nvram_unset(strcat_r(prefix, &t->name[4], tmp));
			}
#endif	/* __CONFIG_NAT__ */
		}
		wps_restore_defaults();
#ifdef __CONFIG_WAPI_IAS__
		nvram_unset("as_mode");
#endif /* __CONFIG_WAPI_IAS__ */
		virtual_radio_restore_defaults();
	}

	/* 
	 * Build bridged i/f name list and wan i/f name list from lan device name list
	 * and wan device name list. Both lan device list "landevs" and wan device list
	 * "wandevs" must exist in order to preceed.
	 */
	if ((landevs = nvram_get("landevs")) && (wandevs = nvram_get("wandevs"))) {
		/* build bridged i/f list based on nvram variable "landevs" */
		len = sizeof(lan_ifnames);
		if (!build_ifnames(landevs, lan_ifnames, &len) && len)
			//dyna[1].value = lan_ifnames; //remove by wwk 2014.08.06, 修复N304概率性无线灯不亮异常(V6.0.0.7)
			;
		else
			goto canned_config;
		/* build wan i/f list based on nvram variable "wandevs" */
		len = sizeof(wan_ifnames);
		if (!build_ifnames(wandevs, wan_ifnames, &len) && len) {
			dyna[3].value = wan_ifnames;
			foreach(wan_ifname, wan_ifnames, next) {
				dyna[2].value = wan_ifname;
				break;
			}
		}
		else
			ap = 1;
		linux_overrides = dyna;
	}
	/* override lan i/f name list and wan i/f name list with default values */
	else {
canned_config:
#ifdef CONFIG_VLAN
		boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
		if (boardflags & BFL_ENETVLAN)
			linux_overrides = vlan;
		else
#endif	/* CONFIG_VLAN*/
			linux_overrides = generic;
	}

	/* Check if nvram version is set, but old */
	if (nvram_get("nvram_version")) {
		int old_ver, new_ver;

		old_ver = atoi(nvram_get("nvram_version"));
		new_ver = atoi(NVRAM_SOFTWARE_VERSION);
		if (old_ver < new_ver) {
			cfg_log("NVRAM: Updating from %d to %d\n", old_ver, new_ver);
			nvram_set("nvram_version", NVRAM_SOFTWARE_VERSION);
		}
	}

	/* Restore defaults */
	for (t = router_defaults; t->name; t++) {
		if (restore_defaults || !nvram_get(t->name)) {
			for (u = linux_overrides; u && u->name; u++) {
				if (!strcmp(t->name, u->name)) {
					nvram_set(u->name, u->value);
					break;
				}
			}
			if (!u || !u->name)
				nvram_set(t->name, t->value);
		}
	}
#ifdef __CONFIG_ALINKGW__
	/*restore tpsk config defaults*/
	if (restore_defaults)
		restore_tpsk_defaults();
#endif
	/* Force to AP */
	if (ap)
		nvram_set("router_disable", "1");

	/* Always set OS defaults */
	nvram_set("os_name", "linux");
	nvram_set("ecos_name", "ecos");
	nvram_set("os_version", ROUTER_VERSION_STR);
	nvram_set("os_date", __DATE__);
	/* Always set WL driver version! */
	nvram_set("wl_version", EPI_VERSION_STR);

	nvram_set("is_modified", "0");
	nvram_set("ezc_version", EZC_VERSION_STR);

	/* Commit values */
	if (restore_defaults) {
		set_ssid_mac();
		set_wan0_mac();
		set_ssid_default();
#ifdef __CONFIG_WPS_LED__
		nvram_set("wps_device_pin", old_pin);
		nvram_set("wps_version2", "enabled");
#endif
		nvram_set("wl0.1_ap_isolate","");
		nvram_set("wl0.1_bss_enabled","");
		nvram_set("wl0.1_bss_maxassoc","");
		nvram_set("wl0.1_closed","");
		nvram_set("wl0.1_ifname","");
		nvram_set("wl0.1_infra","");
		nvram_set("wl0.1_mode","");
		nvram_set("wl0.1_radio","");
		nvram_set("wl0.1_ssid","");
		nvram_set("wl0.1_unit","");
		nvram_set("wl0_vifs","");

		nvram_set("wl0.1_akm","");
		nvram_set("wl0.1_auth_mode","");
		nvram_set("wl0.1_auth","");
		nvram_set("wl0.1_crypto","");
		nvram_set("wl0.1_key","");
		nvram_set("wl0.1_key1","");
		nvram_set("wl0.1_key2","");
		nvram_set("wl0.1_key3","");
		nvram_set("wl0.1_key4","");
		nvram_set("wl0.1_maclist","");
		nvram_set("wl0.1_macmode","");
		nvram_set("wl0.1_radius_ipaddr","");
		nvram_set("wl0.1_radius_key","");
		nvram_set("wl0.1_radius_port","");
		nvram_set("wl0.1_wep","");
		nvram_set("wl0.1_wpa_gtk_rekey","");
		nvram_set("wl0.1_wpa_psk","");
		nvram_set("wl0.1_wps_method","");
		nvram_set("wl0.1_wps_pin","");
		
		nvram_commit();
		cfg_log("done\n");
	}
}

#ifdef __CONFIG_NAT__
static void
set_wan0_vars(void)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	
	/* check if there are any connections configured */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_get(strcat_r(prefix, "unit", tmp)))
			break;
	}
	/* automatically configure wan0_ if no connections found */
	if (unit >= MAX_NVPARSE) {
		struct nvram_tuple *t;
		char *v;

		/* Write through to wan0_ variable set */
		snprintf(prefix, sizeof(prefix), "wan%d_", 0);
		for (t = router_defaults; t->name; t ++) {
			if (!strncmp(t->name, "wan_", 4)) {
				if (nvram_get(strcat_r(prefix, &t->name[4], tmp)))
					continue;
				v = nvram_get(t->name);
				nvram_set(tmp, v ? v : t->value);
			}
		}
		nvram_set(strcat_r(prefix, "unit", tmp), "0");
		nvram_set(strcat_r(prefix, "desc", tmp), "Default Connection");
		nvram_set(strcat_r(prefix, "primary", tmp), "1");
	}
}
#endif /* __CONFIG_NAT__ */

void
ecos_nvram_init(void)
{

	char buf[256];
	int restore_default_value;
	restore_default_value = nvram_match("restore_defaults", "1");
	if(restore_default_value){
		nvram_erase();
		board_reboot();
		return;
	}
	/* Upgrade NVRAM variables to MBSS mode */
	upgrade_defaults();
#if 0
	/* recover config from backup zone if necessary  add by cym*/	
	//if(  checkIfNeedRestorecfg() ==1 )
	 if ((!restore_default_value) && ( (!nvram_get("lan1_ifnames")) || ( checkIfNeedRestorecfg() ==1)))
	{
		restoreConfig();
		printf("\n==>restore Config sucess!!!\n");
	}
	/*add end*/
#endif
	/* Restore defaults if necessary */
	restore_defaults();

	
#ifdef __CONFIG_NAT__
	/* Setup wan0 variables if necessary */
	set_wan0_vars();
#endif /* __CONFIG_NAT__ */

	sprintf(buf, "%s", ROUTER_VERSION_STR);
	nvram_set("fw_version", buf);

	/* build info */
	sprintf(buf, "V%s (Compiled at " __TIME__ " on " __DATE__ ")", ROUTER_VERSION_STR);
	nvram_set("fw_info", buf);
//roy modify for reduce commit times,2010/09/21	
	//nvram_commit();

	printf("\neCos Router/AP %s \n", buf);
}

#if 0//roy,2010/09/26

/* Get all configuration */
int
CFG_write_prof(char * file, int max)
{
	char *p;
	int len;

	nvram_getall(file, max);
	for (p = file; *p; p += len+1) {
		len = strlen(p);
	}

	return (int)(p-file);
}

/* Write all configuration to NV */
int
CFG_read_prof(char * file, int size)
{
	char line[1024], *sp, *ep;
	char *var, *val;
	int rc = 0;
	int len;
	int num = 0;
	char *buf, *ptr;

	/* unset old variables */
	buf = malloc(NVRAM_SPACE);
	if (buf == NULL)
		return -1;

	nvram_getall(buf, NVRAM_SPACE);
	for (sp = buf; *sp; sp += len+1) {
		ptr = sp;
		len = strlen(ptr);
		while (*ptr != '=')
			ptr++;
		*ptr = 0;
		if (find_nvram(sp, false)) {
			nvram_unset(sp);
			printf("nvram_unset: %s\n", sp);
		}
	}
	free(buf);

	for (sp = ep = file; (*sp && sp < file+size); sp += len+1) {
		len = strlen(sp);
		if (len > 1023)
			break;

		memcpy(line, sp, len);
		line[len] = 0;

		while (*ep && (*ep == 0))
			ep++; /* eat to end of line */

		var = line;
		/* skip comment */
		if (*var == '#')
			continue;
		/* found the '=' */
		val = strchr((const char*)var, '=');
		/* cut the var name */
		*val = 0;
		val++;

		nvram_set(var, val);
		num++;
	}

	nvram_commit();

	if (rc)
		cfg_log("CFG_read_prof err %x\n", rc);
	else
		cfg_log("CFG_read_prof %d items\n", num);

	return rc;
}
#else

/* Get all configuration */
int
CFG_read_prof(char * file, int max)
{
	char *p;
	int len;

	nvram_getall(file, max);
	for (p = file; *p; p += len+1) {
		len = strlen(p);
	}

	return (int)(p-file);
}


/* Write all configuration to NV */
int
CFG_write_prof(char * file, int size)
{
	char line[2*NVRAM_MAX_STRINGSIZE], *sp, *ep;
	char *var, *val;
	int rc = 0;
	int len;
	int num = 0;
	
	for (sp = ep = file; (*sp && sp < file+size); sp++) {
		if(*sp!='\r' && *sp!='\n')
			continue;
		*sp='\0';
		
		len = strlen(ep);
		if (len > (2*NVRAM_MAX_STRINGSIZE -1) || len <3)
			break;
		
		if(*ep == '\n'){//*sp=='\r'
			ep++;
			len--;
			sp++;//now *sp='\n'
		}
		
		memcpy(line, ep/*sp*/, len);
		line[len] = 0;

		sp++;
		if(!*sp)
			break;
		
		/*new line start*/
		ep = sp;

		var = line;
		/* skip comment */
		if (*var == '#')
			continue;
		/* found the '=' */
		val = strchr((const char*)var, '=');
		if(!val)
			break;
		/* cut the var name */
		*val = 0;
		val++;
		
		//diag_printf("\n[%s=%s]\n",var,val);
		if (find_nvram(var, true)) {
			nvram_set(var, val);
			num++;
			//printf("nvram_set: %s\n", var);
		}
	}

	nvram_commit();

	if (rc)
		cfg_log("CFG_write_prof err %x\n", rc);
	else
		cfg_log("CFG_write_prof %d items\n", num);

	return rc;
}
#endif

int
nvram_erase(void)
{
	char *file;
	int size;

	size = NVRAM_SPACE;
	file = malloc(size);
	if (file == 0)
		return -1;

	CFG_LOCK();
	memset(file, 0xff, NVRAM_SPACE);
	if (sysFlashInit(NULL) == 0) {
		/* set/write invalid MAGIC 	*/
		nvWrite((unsigned short *)file, size);
		diag_printf("Erase NVRAM to blank! \n");
	}
	free(file);
	CFG_UNLOCK();

	/* Reboot */
	//reboot by hand,roy modify,2010/10/09
	//sys_reboot();
	return 0;
}

