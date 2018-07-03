/*
 * RC header file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: rc.h,v 1.21 2010-08-11 02:31:09 Exp $
 *
 */

#ifndef __RC_H__
#define	__RC_H__
#include <cyg/kernel/kapi.h>

#define IFUP	(IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#define WAN_DBG	printf
#define	cprintf(args...)	printf(args);

/* rc.c */
void sys_reboot(void);
void sys_restart(void);

extern void cpu_idle_sleep(int);
extern int  syslog_init(void);
extern void lo_set_ip(void);
extern int  vlan_config(void);
extern void cli_start(void);
//tenda add
extern void thread_check_start(void);
//

/* wan_link */
extern int wan_link_check(void);
extern int tenda_wan_link_status(void);

/* network */
//extern void set_wl_pwr_percent(WLAN_RATE_TYPE wl_rate);
extern void start_lan(void);
extern void stop_lan(void);
extern void lan_up(char *ifname);
extern void lan_down(char *ifname);
extern void start_wan(void);
extern void stop_wan(void);
extern void wan_up(char *ifname);
extern void wan_down(char *ifname);
extern int wan_ifunit(char *ifname);
extern int wan_primary_ifunit(void);
extern void show_ip_set(char *prefix);
extern void init_services(void);
extern void start_services(void);
extern void stop_services(void);
extern void update_services(void);
extern int del_ns(char *deldns);
extern int del_dest_ns(char *deldns);
extern int add_ns(char *newdns, char *update , const int add_type);

/* firewall.c */
void tpi_firewall_update_tunnel(char *pppname);

/* interface */
extern int ifscrub(char *name);
extern int tenda_ifconfig(char *ifname, int flags, char *addr, char *netmask);
extern int ifconfig_mtu(char *ifname,int mtu);
extern int tenda_route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
extern int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
extern void route_flush(char *ifname);

extern int ppp_ifunit(char *ifname);

#ifdef __CONFIG_DHCPC__
extern void dhcpc_start(char *ifname, char *script, char *hostname);
extern void dhcpc_stop(char *ifname);
#endif /* __CONFIG_DHCPC__ */
#ifdef __CONFIG_PPPOE2__
extern int  wan_pppoe2_start(int unit);
extern void  wan_pppoe2_down(char *pppname);
#endif

#ifdef __CONFIG_PPPOE__
extern int  wan_pppoe_start(int unit);
extern int  wan_pppoe_start2();
extern void wan_pppoe_down(char *pppname);
extern void wan_pppoe_down2(char *pppname);
#ifndef __CONFIG_TENDA_HTTPD_NORMAL__
extern int  lan_pppoe_start(int unit);
extern void lan_pppoe_down(char *pppname);
#endif
extern void pppoe_connect(char *pppname);
extern void pppoe_disconnect(char *pppname);
#endif /* __CONFIG_PPPOE__ */
#ifdef __CONFIG_L2TP__
extern int  wan_l2tp_start(int unit);
extern void wan_l2tp_down(char *pppname);
#endif /* __CONFIG_L2TP__ */
#ifdef __CONFIG_PPTP__
extern int  wan_pptp_start(int unit);
extern void wan_pptp_down(char *pppname);
#endif /* __CONFIG_PPTP__ */
#ifdef __CONFIG_WPS__
extern void wps_start(void);
extern void wps_stop(void);
#endif /* __CONFIG_WPS__ */

/* rc functions */
extern int wandhcpc(int argc, char **argv);
extern int landhcpc(int argc, char **argv);
extern int pptpdhcpc(int argc, char **argv);
extern int l2tpdhcpc(int argc, char **argv);
extern int ipup_main(int argc, char **argv);
extern int ipdown_main(int argc, char **argv);

#define OLD_COOKIE 0

#define WL_RADIO_ON			1
#define WL_RADIO_OFF		0



//#ifdef __CONFIG_TENDA_HTTPD_UCD__
typedef struct{
	bool reset_button_hold;
	bool wifi_button_hold;
}mfg_button_hold_status;
//#endif

enum {
	ADD_NS_PUSH,/* 往前添加 */
	ADD_NS_PUT /* 往后追加*/
};

#endif	/* RC_H__ */
