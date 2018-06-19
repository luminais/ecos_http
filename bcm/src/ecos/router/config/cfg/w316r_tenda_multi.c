/*
 * Router default NVRAM values
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: defaults.c,v 1.199 2010/09/09 17:51:19 Exp $
 */

#include <epivers.h>
#include "router_version.h"
#include <typedefs.h>
#include <string.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <wlioctl.h>
#include <stdio.h>
#include <ezc.h>
#include <bcmconfig.h>

#define XSTR(s) STR(s)
#define STR(s) #s

struct nvram_tuple router_defaults[] = {
	/* OS parameters */
	{ "os_name", "", 0 },			 /* OS name string */
	{ "os_version", ROUTER_VERSION_STR, 0 }, /* OS revision */
	{ "os_date", __DATE__, 0 },		 /* OS date */
	{ "wl_version", EPI_VERSION_STR, 0 },	 /* OS revision */
	{ "te_version", "en", 0 },	 /* tenda revision */
	{ "os_language", "en", 0 },	 /* language */

	/* Version */
	{ "nvram_version", NVRAM_SOFTWARE_VERSION, 0 },

	/* Miscellaneous parameters */
	{ "timer_interval", "3600", 0 },	/* Timer interval in seconds */
	{ "log_level", "0", 0 },		/* Bitmask 0:off 1:denied 2:accepted */
	{"watchdog","45000",0},
	{"boot_wait", "off", 0 },
	
#ifdef __CONFIG_DLNA__
	{ "dlna_enable", "1", 0 },		/* Start DLNA */
#endif
	{ "ezc_enable", "1", 0 },		/* Enable EZConfig updates */
	{ "ezc_version", EZC_VERSION_STR, 0 },	/* EZConfig version */
	{ "is_default", "1", 0 },		/* is it default setting: 1:yes 0:no */
	{ "os_server", "", 0 },			/* URL for getting upgrades */
	{ "stats_server", "", 0 },		/* URL for posting stats */
	{ "console_loglevel", "1", 0 },		/* Kernel panics only */

	/* Big switches */
	{ "router_disable", "0", 0 },		/* lan_proto=static lan_stp=0 wan_proto=disabled */
	{ "ure_disable", "1", 0 },		/* sets APSTA for radio and puts wirelesss
						 * interfaces in correct lan
						 */
	{ "fw_disable", "0", 0 },		/* Disable firewall (allow new connections from the
						 * WAN)
						 */

	{ "log_ipaddr", "", 0 },		/* syslog recipient */
#ifdef BCMQOS
	{ "wan_mtu",			"1500"			},
#endif
	/* LAN H/W parameters */
	{ "lan_ifname", "br0", 0 },		/* LAN interface name */
	{ "lan_ifnames", "vlan1 eth1", 0 },		/* Enslaved LAN interfaces */
	{ "lan_hwnames", "", 0 },		/* LAN driver names (e.g. et0) */
	{ "lan_hwaddr", "", 0 },		/* LAN interface MAC address */
	{ "clnway", "1", 0 },/*add for auto mac web page */
	
	/* LAN TCP/IP parameters */
	{ "lan_dhcp", "0", 0 },			/* DHCP client [static|dhcp] */
	{ "lan_ipaddr", "192.168.0.1", 0 },	/* LAN IP address */
	{ "lan_netmask", "255.255.255.0", 0 },	/* LAN netmask */
	{ "lan_gateway", "192.168.0.1", 0 },	/* LAN gateway */
	{ "lan_proto", "dhcp", 0 },		/* DHCP server [static|dhcp] */
	{ "lan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "lan_domain", "", 0 },		/* LAN domain name */
	{ "lan_lease", "86400", 0 },		/* LAN lease time in seconds */
	{ "lan_stp", "1", 0 },			/* LAN spanning tree protocol */
	{ "lan_route", "", 0 },			/* Static routes
						 * (ipaddr:netmask:gateway:metric:ifname ...)
						 */
	/* Guest H/W parameters */
	{ "lan1_ifname", "", 0 },		/* LAN interface name */
	{ "lan1_ifnames", "", 0 },		/* Enslaved LAN interfaces */
	{ "lan1_hwnames", "", 0 },		/* LAN driver names (e.g. et0) */
	{ "lan1_hwaddr", "", 0 },		/* LAN interface MAC address */

	/* Guest TCP/IP parameters */
	{ "lan1_dhcp", "0", 0 },			/* DHCP client [static|dhcp] */
	{ "lan1_ipaddr", "192.168.2.1", 0 },	/* LAN IP address */
	{ "lan1_netmask", "255.255.255.0", 0 },	/* LAN netmask */
	{ "lan1_gateway", "192.168.2.1", 0 },	/* LAN gateway */
/*roy midify*///{ "lan1_proto", "dhcp", 0 },		/* DHCP server [static|dhcp] */
	{ "lan1_proto", "static", 0 },		/* DHCP server [static|dhcp] */
	{ "lan1_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "lan1_domain", "", 0 },		/* LAN domain name */
	{ "lan1_lease", "86400", 0 },		/* LAN lease time in seconds */
	{ "lan1_stp", "1", 0 },			/* LAN spanning tree protocol */
	{ "lan1_route", "", 0 },			/* Static routes
						 * (ipaddr:netmask:gateway:metric:ifname ...)
						 */
	/*sntp*/
	{ "ntp_server", "192.5.41.40 192.5.41.41 133.100.9.2", 0 },		/* NTP server */
	//{ "time_zone", "PST8PDT", 0 },		/* Time zone (GNU TZ format) */
	{ "time_zone", "57", 0 },	//roy modify,57->china zone
	{ "time_mode", "0", 0 },
	{"err_check","0",0},
	{"pppoe_index","0",0},
	{"config_index","0",0},
	{"wan0_check","1",0},  // pppoe<-->dhcp auto change 0:disable 1:enable
	
#ifdef __CONFIG_NAT__
	/*common*/
	{ "wan_speed", "0", 0},	/* huangxiaoli modify*/
	{ "wan_ifnames", "vlan2", 0 },		/* Enslaved WAN interfaces */
	{ "wan_primary","1",0},/*read only*/
	{ "wan0_primary","1",0},/*read only*/
	{ "wan0_ifname","vlan2",0},/*read only*/
	{ "wan0_ifnames","vlan2",0},/*read only*/
	{ "wan0_domain","",0},
	{ "wan0_route","",0},
	{ "wan0_wins","",0},
	{ "wan0_desc","Default Connection",0}, /*read only*/
	{ "wan0_unit","0",0},
	{ "wan0_proto","dhcp",0},
	{ "wan0_proto_index","pppoe",0},
	{ "wan0_connect","Disconnect",0},

	/*static*/
	{ "wan0_ipaddr","0.0.0.0",0},
	{ "wan0_netmask","0.0.0.0",0},
	{ "wan0_gateway","0.0.0.0",0},
	{ "wan0_dns","",0},
	{ "wan0_dns_fix","0",0},
	
	/*dhcp*/
	{ "wan0_hostname","",0}, /*dhcp only*/
	{ "wan0_mtu","1500",0}, /*dhcp || static*/

	/*pppoe*/
	{ "wan0_pppoe_prio_xkjs","0",0},/*add for pppoe xkjs,save the best one*/
	{ "wan0_pppoe_ifname","ppp0",0},/*read only*/
	{ "wan0_pppoe_username","",0},
	{ "wan0_pppoe_passwd","",0},
	{ "wan0_pppoe_ac","",0},
	{ "wan0_pppoe_service","",0},
	{ "wan0_pppoe_mtu","1492",0},
	{ "wan0_pppoe_mru","1492",0},
	{ "wan0_pppoe_demand","0",0},
	{ "wan0_pppoe_idletime","60",0},
	{ "wan0_pppoe_keepalive","1",0},
	{ "wan0_pppoe_st","0",0},
	{ "wan0_pppoe_et","0",0},
	
	/*pptp*/
	{ "wan0_pptp_ifname","ppp0",0},/*read only*/
	{ "wan0_pptp_username","",0},
	{ "wan0_pptp_passwd","",0},
	{ "wan0_pptp_server_name","",0},
	{ "wan0_pptp_static","0",0},
	{ "wan0_pptp_ipaddr","0.0.0.0",0},
	{ "wan0_pptp_netmask","0.0.0.0",0},
	{ "wan0_pptp_gateway","0.0.0.0",0},
	{ "wan0_pptp_dns","",0},
	{ "wan0_pptp_mtu","1452",0},
	{ "wan0_pptp_mru","1452",0},
	{ "wan0_pptp_demand","0",0},
	{ "wan0_pptp_idletime","0",0},
	{ "wan0_pptp_keepalive","1",0},
	{ "wan0_pptp_mppe_en","1",0},
	

	/*l2tp*/
	{ "wan0_l2tp_ifname","ppp0",0},/*read only*/
	{ "wan0_l2tp_username","",0},
	{ "wan0_l2tp_passwd","",0},
	{ "wan0_l2tp_server_name","",0},
	{ "wan0_l2tp_static","0",0},
	{ "wan0_l2tp_ipaddr","0.0.0.0",0},
	{ "wan0_l2tp_netmask","0.0.0.0",0},
	{ "wan0_l2tp_gateway","0.0.0.0",0},
	{ "wan0_l2tp_dns","",0},
	{ "wan0_l2tp_mtu","1452",0},
	{ "wan0_l2tp_mru","1452",0},
	{ "wan0_l2tp_demand","0",0},
	{ "wan0_l2tp_idletime","0",0},
	{ "wan0_l2tp_keepalive","1",0},

	/*802.1x*/
	{ "c8021x_en", "", 0 },
	{ "c8021x_user", "", 0 },
	{ "c8021x_pswd", "", 0 },

	/*nat*/
	{ "nat_type", "sym", 0 },               /* sym: Symmetric NAT, cone: Cone NAT */
	
						 
	/*  add by zliang  ********/					 
	{"hacker_att", "1", 0},

	{"rm_web_en", "0", 0},
	{"rm_web_port", "8080", 0},
	{"rm_web_ip", "", 0},

	{"ping_dis_wan", "0", 0},
	

	{ "filter_client_mode", "disable", 0},
	{ "filter_client_cur_nu", "1", 0},
	{ "filter_client0", "", 0 },	
	{ "filter_client1", "", 0 },	
	{ "filter_client2", "", 0 },	
	{ "filter_client3", "", 0 },	
	{ "filter_client4", "", 0 },	
	{ "filter_client5", "", 0 },	
	{ "filter_client6", "", 0 },	
	{ "filter_client7", "", 0 },	
	{ "filter_client8", "", 0 },	
	{ "filter_client9", "", 0 },	


	{"filter_url_mode", "disable", 0},
	{"filter_url_cur_nu", "1", 0},
	{"filter_url0", "", 0},
	{"filter_url1", "", 0},
	{"filter_url2", "", 0},
	{"filter_url3", "", 0},
	{"filter_url4", "", 0},
	{"filter_url5", "", 0},
	{"filter_url6", "", 0},
	{"filter_url7", "", 0},
	{"filter_url8", "", 0},
	{"filter_url9", "", 0},
	
	{"filter_mac_mode", "disable", 0},
	{"filter_mac_cur_nu", "1", 0},
	{"filter_mac0", "", 0},
	{"filter_mac1", "", 0},
	{"filter_mac2", "", 0},
	{"filter_mac3", "", 0},
	{"filter_mac4", "", 0},
	{"filter_mac5", "", 0},
	{"filter_mac6", "", 0},
	{"filter_mac7", "", 0},
	{"filter_mac8", "", 0},
	{"filter_mac9", "", 0},
	
	/*wirtual server*/
	{ "dmz_ipaddr_en", "", 0},
	{ "dmz_ipaddr", "", 0 },		/* x.x.x.x (equivalent to 0-60999>dmz_ipaddr:
						 	* 0-60999)
						 	*/
		
	{ "upnp_enable", "1", 0 },		/* Start UPnP */



	{ "tc_stream_stat_en", "0", 0},
	{ "tc_enable", "0", 0},			/*start stream control*/
	{ "tc_0", "", 0},
	{ "tc_1", "", 0},
	{ "tc_2", "", 0},
	{ "tc_3", "", 0},
	{ "tc_4", "", 0},
	{ "tc_5", "", 0},
	{ "tc_6", "", 0},
	{ "tc_7", "", 0},
	{ "tc_8", "", 0},
	{ "tc_9", "", 0},

	{ "tc_isp_uprate", "1024", 0},
	{ "tc_isp_downrate", "1024", 0},
	{ "forward_port0", "", 0 },		/* wan_port0-wan_port1>lan_ipaddr:
						 		* lan_port0-lan_port1[:,]proto[:,]enable[:,]desc
						 		*/
	{ "forward_port1", "", 0 },
	{ "forward_port2", "", 0 },
	{ "forward_port3", "", 0 },
	{ "forward_port4", "", 0 },
	{ "forward_port5", "", 0 },
	{ "forward_port6", "", 0 },
	{ "forward_port7", "", 0 },
	{ "forward_port8", "", 0 },
	{ "forward_port9", "", 0 },
	
	{ "autofw_port0", "", 0 },		/* out_proto:out_port,in_proto:in_port0-in_port1>
						 		* to_port0-to_port1,enable,desc
						 		*/
#ifdef BCMQOS
	{ "qos_orates",	"80-100,10-100,5-100,3-100,2-95,0-0,0-0,0-0,0-0,0-0", 0 },
	{ "qos_irates",	"0,0,0,0,0,0,0,0,0,0", 0 },
	{ "qos_enable",			"0"				},
	{ "qos_method",			"0"				},
	{ "qos_sticky",			"1"				},
	{ "qos_ack",			"1"				},
	{ "qos_icmp",			"0"				},
	{ "qos_reset",			"0"				},
	{ "qos_obw",			"384"			},
	{ "qos_ibw",			"1500"			},
	{ "qos_orules",			"" },
	{ "qos_burst0",			""				},
	{ "qos_burst1",			""				},
	{ "qos_default",		"3"				},
#endif /* BCMQOS */
	/* DHCP server parameters */
	//{ "dhcp_en", "", 0},
	{ "dhcp_start", "192.168.0.100", 0 },	/* First assignable DHCP address */
	{ "dhcp_end", "192.168.0.150", 0 },	/* Last assignable DHCP address */
	{ "dhcp_domain", "wan", 0 },		/* Use WAN domain name first if available (wan|lan)
						 			*/
	{ "dhcp_static_lease1", "", 0 },
	{ "dhcp_static_lease2", "", 0 },
	{ "dhcp_static_lease3", "", 0 },
	{ "dhcp_static_lease4", "", 0 },
	{ "dhcp_static_lease5", "", 0 },
	{ "dhcp_static_lease6", "", 0 },
	{ "dhcp_static_lease7", "", 0 },
	{ "dhcp_static_lease8", "", 0 },
	{ "dhcp_static_lease9", "", 0 },
	{ "dhcp_static_lease10", "", 0 },
	{ "dhcp_static_lease11", "", 0 },
	{ "dhcp_static_lease12", "", 0 },
	{ "dhcp_static_lease13", "", 0 },
	{ "dhcp_static_lease14", "", 0 },
	{ "dhcp_static_lease15", "", 0 },
	{ "dhcp_static_lease16", "", 0 },
	{ "dhcp_wins", "wan", 0 },		/* Use WAN WINS first if available (wan|lan) */
#endif	/* __CONFIG_NAT__ */

	/* Web server parameters */
	{ "http_username", "admin", 0 },		/* Username */
	{ "http_passwd", "admin", 0 },		/* Password */
	{"http_defaultpwd","admin",0},  /*产测添加为产测免COOKIE认证补丁*/
	{ "http_wanport", "", 0 },		/* WAN port to listen on */
	{ "http_lanport", "80", 0 },		/* LAN port to listen on */

	/*ddns parameters*/
	{ "ddns_enable", "0", 0 },		
	{ "ddns_set1", "", 0 },		

	/* Wireless parameters */

	/*add by stanley for  test HT_BW 20/40MHz, 2010/10/18*/
	{ "wl_mimo_preamble", "auto", 0},
	{ "wl_rifs", "auto", 0},		
//	{ "wl_frameburst", "on", 0},
	
	/*end*/

	{ "wl_ifname", "", 0 },			/* Interface name */
	{ "wl_hwaddr", "", 0 },			/* MAC address */
	{ "wl_phytype", "n", 0 },		/* Current wireless band ("a" (5 GHz),
						 * "b" (2.4 GHz), or "g" (2.4 GHz))
						 */
	{ "wl_corerev", "", 0 },		/* Current core revision */
	{ "wl_phytypes", "", 0 },		/* List of supported wireless bands (e.g. "ga") */
	{ "wl_radioids", "", 0 },		/* List of radio IDs */
	{ "wl_ssid", "Tenda", 0 },		/* Service set ID (network name) */
	{ "wl_bss_enabled", "1", 0 },		/* Service set ID (network name) */
						/* See "default_get" below. */
	{ "wl_country_code", "US", 0 },		/* Country Code (default obtained from driver) */
	{ "wl_radio", "1", 0 },			/* Enable (1) or disable (0) radio */
	{ "wl_closed", "0", 0 },		/* Closed (hidden) network */
	{ "wl_ap_isolate", "0", 0 },            /* AP isolate mode */
	{ "wl_wmf_bss_enable", "0", 0 },	/* WMF Enable/Disable */
	{ "wl_mcast_regen_bss_enable", "1", 0 },	/* MCAST REGEN Enable/Disable */
	{ "wl_rxchain_pwrsave_enable", "1", 0 },	/* Rxchain powersave enable */
	{ "wl_rxchain_pwrsave_quiet_time", "1800", 0 },	/* Quiet time for power save */
	{ "wl_rxchain_pwrsave_pps", "10", 0 },	/* Packets per second threshold for power save */
	{ "wl_rxchain_pwrsave_stas_assoc_check", "0", 0 }, /* STAs associated before powersave */
	{ "wl_radio_pwrsave_enable", "0", 0 },	/* Radio powersave enable */
	{ "wl_radio_pwrsave_quiet_time", "1800", 0 },	/* Quiet time for power save */
	{ "wl_radio_pwrsave_pps", "10", 0 },	/* Packets per second threshold for power save */
	{ "wl_radio_pwrsave_level", "0", 0 },	/* Radio power save level */
	{ "wl_radio_pwrsave_stas_assoc_check", "0", 0 }, /* STAs associated before powersave */
	{ "wl_mode", "ap", 0 },			/* AP mode (ap|sta|wds) */
	{ "wl_lazywds", "0", 0 },		/* Enable "lazy" WDS mode (0|1) */
	{ "wl_wds", "", 0 },			/* xx:xx:xx:xx:xx:xx ... */
	{ "wl_wds_timeout", "0", 0 },		//for tenda
	{ "wl_wep", "disabled", 0 },		/* WEP data encryption (enabled|disabled) */
	{ "wl_auth", "0", 0 },			/* Shared key authentication optional (0) or
						 * required (1)
						 */
	{ "wl_key", "1", 0 },			/* Current WEP key */
	{ "wl_key1", "ASCII", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key2", "ASCII", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key3", "ASCII", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key4", "ASCII", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_maclist", "", 0 },		/* xx:xx:xx:xx:xx:xx ... */
	{ "wl_macmode", "disabled", 0 },	/* "allow" only, "deny" only, or "disabled"
						 * (allow all)
						 */
	{ "wl_channel", "0", 0 },		/* Channel number */
	{ "wl_reg_mode", "off", 0 },		/* Regulatory: 802.11H(h)/802.11D(d)/off(off) */
	{ "wl_dfs_preism", "60", 0 },		/* 802.11H pre network CAC time */
	{ "wl_dfs_postism", "60", 0 },		/* 802.11H In Service Monitoring CAC time */
	/* Radar thrs params format: version thresh0_20 thresh1_20 thresh0_40 thresh1_40 */
	{ "wl_radarthrs", "0 0x6a8 0x6c8 0x6ac 0x6c7", 0 },
	{ "wl_rate", "0", 0 },			/* Rate (bps, 0 for auto) */
	{ "wl_mrate", "0", 0 },			/* Mcast Rate (bps, 0 for auto) */
	{ "wl_rateset", "12", 0 },		/* "default" or "all" or "12" */
	{ "wl_frag", "2346", 0 },		/* Fragmentation threshold */
	{ "wl_rts", "2347", 0 },		/* RTS threshold */
	{ "wl_dtim", "3", 0 },			/* DTIM period */
	{ "wl_bcn", "100", 0 },			/* Beacon interval */
	{ "wl_bcn_rotate", "1", 0 },		/* Beacon rotation */
	{ "wl_plcphdr", "long", 0 },		/* 802.11b PLCP preamble type */
	{ "wl_gmode", XSTR(GMODE_AUTO), 0 },	/* 54g mode */
	{ "wl_gmode_protection", "auto", 0 },	/* 802.11g RTS/CTS protection (off|auto) */
	{ "wl_afterburner", "off", 0 },		/* AfterBurner */
	{ "wl_frameburst", "off", 0 },		/* BRCM Frambursting mode (off|on) */
	{ "wl_wme", "auto", 0 },		/* WME mode (off|on|auto) */
	{ "wl_wme_bss_disable", "0", 0 },	/* WME BSS disable advertising (off|on) */
	{ "wl_antdiv", "-1", 0 },		/* Antenna Diversity (-1|0|1|3) */
	{ "wl_infra", "1", 0 },			/* Network Type (BSS/IBSS) */
	{ "wl_nbw_cap", "1", 0},		/* BW Cap; def 20inB and 40inA */
	{ "wl_nctrlsb", "none", 0},		/* N-CTRL SB */
	{ "wl_nband", "2", 0},			/* N-BAND */
	{ "wl_nmcsidx", "-1", 0},		/* MCS Index for N - rate */
	{ "wl_nmode", "-1", 0},			/* N-mode */
	{ "wl_rifs_advert", "auto", 0},		/* RIFS mode advertisement */
	{ "wl_nreqd", "0", 0},			/* Require 802.11n support */
	{ "wl_vlan_prio_mode", "off", 0},	/* VLAN Priority support */
	{ "wl_leddc", "0x640000", 0},		/* 100% duty cycle for LED on router */
	{ "wl_rxstreams", "0", 0},              /* 802.11n Rx Streams, 0 is invalid, WLCONF will
						 * change it to a radio appropriate default
						 */
	{ "wl_txstreams", "0", 0},              /* 802.11n Tx Streams 0, 0 is invalid, WLCONF will
						 * change it to a radio appropriate default
						 */
	{ "wl_stbc_tx", "auto", 0 },		/* Default STBC TX setting */
	{ "wl_ampdu", "auto", 0 },		/* Default AMPDU setting */
	/* Default AMPDU retry limit per-tid setting */
	{ "wl_ampdu_rtylimit_tid", "5 5 5 5 5 5 5 5", 0 },
	/* Default AMPDU regular rate retry limit per-tid setting */
	{ "wl_ampdu_rr_rtylimit_tid", "2 2 2 2 2 2 2 2", 0 },
	{ "wl_amsdu", "auto", 0 },		/* Default AMSDU setting */
	//{ "wl_obss_coex", "1", 0 },		/* Default OBSS Coexistence setting - OFF */
	{ "wl_obss_coex", "0", 0 },		//for tenda

	/* WPA parameters */
	{ "wl_auth_mode", "none", 0 },		/* Network authentication mode (radius|none) */
	{ "wl_wpa_psk", "12345678", 0 },		/* WPA pre-shared key */
	{ "wl_wpa_gtk_rekey", "0", 0 },		/* GTK rotation interval */
	{ "wl_radius_ipaddr", "", 0 },		/* RADIUS server IP address */
	{ "wl_radius_key", "", 0 },		/* RADIUS shared secret */
	{ "wl_radius_port", "1812", 0 },	/* RADIUS server UDP port */
	{ "wl_crypto", "tkip+aes", 0 },		/* WPA data encryption */
	{ "wl_net_reauth", "36000", 0 },	/* Network Re-auth/PMK caching duration */
	{ "wl_akm", "", 0 },			/* WPA akm list */

#ifdef __CONFIG_SYS_LED__
	{ "sb/1/ledbh6", "11", 0},
	{ "gpio6", "sys_led", 0},
#endif
	{ "wl_aporwds", "0", 0},

#ifdef __CONFIG_WPS_LED__	
	/* WSC parameters */
	{ "wl_wps_mode", "enabled", 0 }, /* enabled wps */
	{ "wps_mode", "enabled", 0 },	 /* enabled wps */
		/*add by stanley*/
	{ "wl_wps_method", "pbc", 0},	/*enabled or disabled wps*/
#else
//N3没有WPS功能
	/* WSC parameters */
	{ "wl_wps_mode", "enabled", 0 }, /* enabled wps */
	{ "wps_mode", "disabled", 0 },	 /* enabled wps */
		/*add by stanley*/
	{ "wl_wps_method", "pbc", 0},	/*enabled or disabled wps*/
#endif
	/* pxy revise, 2013.12.03, ssid will be change Tenda_(10 random char) after useing wps*/
	{ "wps_random_ssid_prefix", "Tenda", 0},	
		/*end stanley*/
	{ "wl_wps_config_state", "0", 0 },	/* config state unconfiged */
	{ "wps_device_pin", "12345670", 0 },
	{ "wps_modelname", "Tenda", 0 },
	{ "wps_mfstring", "Tenda", 0 },
	{ "wps_device_name", "Tenda Wireless AP", 0 },
	{ "wl_wps_reg", "enabled", 0 },
	{ "wps_sta_pin", "00000000", 0 },
	{ "wps_modelnum", "123456", 0 },
	{ "wps_config_method", "0x84", 0 },	/* for DTM 1.1 test */
	{ "wps_timeout_enable", "0", 0 },
	/* Allow or Deny Wireless External Registrar get or configure AP security settings */
	{ "wps_wer_mode", "allow", 0 },

	{ "lan_wps_oob", "enabled", 0 },	/* OOB state */
	{ "lan_wps_reg", "enabled", 0 },	/* For DTM 1.4 test */

	{ "lan1_wps_oob", "enabled", 0 },
	{ "lan1_wps_reg", "enabled", 0 },
#ifdef __CONFIG_WFI__
	{ "wl_wfi_enable", "0", 0 },	/* 0: disable, 1: enable WifiInvite */
	{ "wl_wfi_pinmode", "0", 0 },	/* 0: auto pin, 1: manual pin */
#endif /* __CONFIG_WFI__ */
#ifdef __CONFIG_WAPI_IAS__
	/* WAPI parameters */
	{ "wl_wai_cert_name", "", 0 },		/* AP certificate name */
	{ "wl_wai_cert_index", "1", 0 },	/* AP certificate index. 1:X.509, 2:GBW */
	{ "wl_wai_cert_status", "0", 0 },	/* AP certificate status */
	{ "wl_wai_as_ip", "", 0 },		/* ASU server IP address */
	{ "wl_wai_as_port", "3810", 0 },	/* ASU server UDP port */
#endif /* __CONFIG_WAPI_IAS__ */
	/* WME parameters (cwmin cwmax aifsn txop_b txop_ag adm_control oldest_first) */
	/* EDCA parameters for STA */
	{ "wl_wme_sta_be", "15 1023 3 0 0 off off", 0 },	/* WME STA AC_BE parameters */
	{ "wl_wme_sta_bk", "15 1023 7 0 0 off off", 0 },	/* WME STA AC_BK parameters */
	{ "wl_wme_sta_vi", "7 15 2 6016 3008 off off", 0 },	/* WME STA AC_VI parameters */
	{ "wl_wme_sta_vo", "3 7 2 3264 1504 off off", 0 },	/* WME STA AC_VO parameters */

	/* EDCA parameters for AP */
	{ "wl_wme_ap_be", "15 63 3 0 0 off off", 0 },		/* WME AP AC_BE parameters */
	{ "wl_wme_ap_bk", "15 1023 7 0 0 off off", 0 },		/* WME AP AC_BK parameters */
	{ "wl_wme_ap_vi", "7 15 1 6016 3008 off off", 0 },	/* WME AP AC_VI parameters */
	{ "wl_wme_ap_vo", "3 7 1 3264 1504 off off", 0 },	/* WME AP AC_VO parameters */

	{ "wl_wme_no_ack", "off", 0},		/* WME No-Acknowledgment mode */
	//{ "wl_wme_apsd", "on", 0},		/* WME APSD mode */
	{ "wl_wme_apsd", "off", 0},			//for tenda
	
	/* Per AC Tx parameters */
	{ "wl_wme_txp_be", "7 3 4 2 0", 0 },	/* WME AC_BE Tx parameters */
	{ "wl_wme_txp_bk", "7 3 4 2 0", 0 },	/* WME AC_BK Tx parameters */
	{ "wl_wme_txp_vi", "7 3 4 2 0", 0 },	/* WME AC_VI Tx parameters */
	{ "wl_wme_txp_vo", "7 3 4 2 0", 0 },	/* WME AC_VO Tx parameters */

	{ "wl_maxassoc", "128", 0},		/* Max associations driver could support */
	{ "wl_bss_maxassoc", "128", 0},		/* Max associations driver could support */

	{ "wl_unit", "0", 0 },			/* Last configured interface */
	{ "wl_ure", "0", 0 },			/* Last configured interface */
	{ "wl_sta_retry_time", "5", 0 }, /* Seconds between association attempts */

#ifdef __CONFIG_EMF__
	/* EMF defaults */
	{ "emf_entry", "", 0 },			/* Static MFDB entry (mgrp:if) */
	{ "emf_uffp_entry", "", 0 },		/* Unreg frames forwarding ports */
	{ "emf_rtport_entry", "", 0 },		/* IGMP frames forwarding ports */
	{ "emf_enable", "0", 0 },		/* Disable EMF by default */
#endif /* __CONFIG_EMF__ */
#ifdef __CONFIG_IPV6__
	{ "lan_ipv6_mode", "3", 0 },		/* 0=disable 1=6to4 2=native 3=6to4+native! */
	{ "lan_ipv6_dns", "", 0  },
	{ "lan_ipv6_6to4id", "0", 0  }, /* 0~65535 */
	{ "lan_ipv6_prefix", "2001:db8:1:0::/64", 0  },
	{ "wan_ipv6_prefix", "2001:db0:1:0::/64", 0  },
#endif /* __CONFIG_IPV6__ */
#ifdef __CONFIG_NETBOOT__
	{ "netboot_url", "", 0 },		/* netboot url */
	{ "netboot_username", "", 0 },	/* netboot username */
	{ "netboot_passwd", "", 0 },	/* netboor password */
#endif /* __CONFIG_NETBOOT__ */
	/* Restore defaults */
	{ "restore_defaults", "0", 0 },		/* Set to 0 to not restore defaults on boot */
#ifdef __CONFIG_EXTACS__
	{ "acs_ifnames", "", 0  },
#endif
#ifdef __CONFIG_SAMBA__
	{ "samba_mode", "", 0  },
	{ "samba_passwd", "", 0  },
#endif
	{ 0, 0, 0 }
};

/* Translates from, for example, wl0_ (or wl0.1_) to wl_. */
/* Only single digits are currently supported */

static void
fix_name(const char *name, char *fixed_name)
{
	char *pSuffix = NULL;

	/* Translate prefix wlx_ and wlx.y_ to wl_ */
	/* Expected inputs are: wld_root, wld.d_root, wld.dd_root
	 * We accept: wld + '_' anywhere
	 */
	pSuffix = strchr(name, '_');

	if ((strncmp(name, "wl", 2) == 0) && isdigit(name[2]) && (pSuffix != NULL)) {
		strcpy(fixed_name, "wl");
		strcpy(&fixed_name[2], pSuffix);
		return;
	}

	/* No match with above rules: default to input name */
	strcpy(fixed_name, name);
}


/* 
 * Find nvram param name; return pointer which should be treated as const
 * return NULL if not found.
 *
 * NOTE:  This routine special-cases the variable wl_bss_enabled.  It will
 * return the normal default value if asked for wl_ or wl0_.  But it will
 * return 0 if asked for a virtual BSS reference like wl0.1_.
 */
char *
nvram_default_get(const char *name)
{
	int idx;
	char fixed_name[NVRAM_MAX_VALUE_LEN];

	fix_name(name, fixed_name);
	if (strcmp(fixed_name, "wl_bss_enabled") == 0) {
		if (name[3] == '.' || name[4] == '.') { /* Virtual interface */
			return "0";
		}
	}

	for (idx = 0; router_defaults[idx].name != NULL; idx++) {
		if (strcmp(router_defaults[idx].name, fixed_name) == 0) {
			return router_defaults[idx].value;
		}
	}

	return NULL;
}
