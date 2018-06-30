/*
 * Copyright (c) 2005, 2006
 *
 * James Hook (james@wmpp.com) 
 * Chris Zimman (chris@wmpp.com)
 *
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>
#include <cyg/io/devtab.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
//#include <cyg/hal/var_ints.h>
#include <cyg/hal/plf_io.h>
#include <cyg/kernel/kapi.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_arch.h>

#include <ctype.h>
#include <stdlib.h>
#include <shell.h>
#include <shell_err.h>
#include <shell_thread.h>
#include <commands.h>
#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
#include <cyg/hal/romeperf.h>
#endif
#include <cyg/hal/hal_if.h>
#include <time.h>
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif
#include <cyg/io/watchdog.h>
#include "../system/hw_settings.h"
#include "../system/sys_utility.h"
#include "../network/net_api.h"
#include "../apmib/apmib.h"

#ifdef HAVE_RLX_PROFILING
#include <rlx_library.h>
#endif

void get_thread_info(void);

extern void cause_exception(int type);
extern int set_mac_address( const char *interface, char *mac_address );
extern int get_mac_address( const char *interface, char *mac_address );
extern void flash_test( void );
extern int flash_main(unsigned int argc, unsigned char *argv[]);
#ifdef HAVE_BRIDGE
extern int brconfig_main(unsigned int argc, unsigned char *argv[]);
#endif
#ifdef ECOS_DBG_STAT
extern void dbg_stat_show();
#endif
#ifdef STATIC_ROUTE
extern int route_main(int argc,char **argv);
#endif
#ifdef CONFIG_IPV6_ROUTE6
extern int route6_main(int argc,char **argv);
#endif

#ifdef ROUTE_SUPPORT
extern int cyg_routed_start(int argc, char *argv[]);
#endif
#ifdef ROUTE6D_SUPPORT
extern int cyg_route6d_start(int argc,char **argv);
#endif

#ifdef CONFIG_MCASE_TOOLS
extern int pim6dd_main(int argc, char *argv[]);
#endif
#ifdef CONFIG_PING6
extern int ping6_main(argc, argv);
#endif
#ifdef CONFIG_IPV6_NDP
extern int ndp_main(argc, argv);
#endif
#ifdef HAVE_RADVD_SHELL
extern int create_radvd();
extern int radvdconf_main(int argc, char *argv[]);
#endif
extern int icmp_main(int argc, char **argv);
extern int ifconfig_main(unsigned int argc, unsigned char *argv[]);
extern int iwpriv_main(unsigned int argc, unsigned char *argv[]);
#ifdef HAVE_WLAN_SCHEDULE
extern int wlschedule_main(unsigned int argc, unsigned char *argv[]);
#endif

#ifdef HAVE_TELNETD
extern int telnetd_start(unsigned int argc, unsigned char *argv[]);
#endif

#ifdef HAVE_SYSTEM_REINIT
extern int reinit_test(unsigned int argc, unsigned char *argv[]);
#endif

#ifdef HAVE_IPV6FIREWALL
extern int ip6fw_init_main(unsigned int argc, unsigned char *argv[]);
#endif

#ifdef HAVE_DHCP6S
extern int dhcp6s_main(unsigned int argc, unsigned char *argv[]);
#endif
#ifdef HAVE_TR069
extern int tr069_start(unsigned int argc, unsigned char *argv[]);
#endif

#ifdef HAVE_NBSERVER
extern int nbserver_start(unsigned int argc, unsigned char *argv[]);
#endif

#if defined(CONFIG_RTL_ULINKER)
extern int set_ulinker_led(int action);
#endif

#if defined(CONFIG_RTL_819X)&&defined(HAVE_FIREWALL)	//jwj:20120626
extern int ipfw_init_main(unsigned int argc, unsigned char *argv[]);
#endif

#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
extern void rtl_readFastpathNaptEntry(void);
extern void rtl_fastpathOnOff(int value);
#if defined(QOS_BY_BANDWIDTH)
//extern void rtl_showQosForNatdEntry(void);
#endif
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT)
extern void rtl_hwNatOnOff(int value);
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)
extern int rtl_vlanTableShow(void);
extern void rtl_showPortDefVlanInfo(void);
#endif

#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
extern void rtl_enableTriggerPort(int value);
extern int rtl_setTriggerPort(unsigned int argc, unsigned char *argv[]);
extern void rtl_showTriggerPort(char *parm);
extern void rtl_flushTriggerPort(char *parm);
extern int rtl_delTriggerRuleByNum(char *parm);
#endif

extern int rtl_pppoe(int argc, char **argv);
extern void rtl8192cd_wlan_cmd_dispatch(char *name, int argc, char *argv[]);
extern int cyg_kmem_print_stats(void);
typedef void pr_fun(char *fmt, ...);
extern void show_network_tables(pr_fun *pr);
extern void show_arp_tables(pr_fun *pr);
extern void list_dir(char *name);
extern void cat_file(char *name, int hexdump);
extern void rtl8192cd_proc_help(char *name);
extern void set_ecos_pr_fun(pr_fun *pr);

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef HAVE_WPS
extern void dump_wsc_context(void);
extern void dump_wsc_flash_param(void);
extern int validate_pin_code(unsigned long code);
extern cyg_flag_t wsc_flag;
#endif
#endif
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
extern void show_ip_mr_vif(void);
extern void show_ip_mr_cache(void);
#endif

#ifdef CONFIG_MLD_PROXY
extern void showMF6C(void);
#endif
#ifdef CONFIG_TENDA_ATE_REALTEK
extern int td_flash_main(unsigned int argc, unsigned char *argv[]);
extern void mtest_cmd_main(int argc,char **argv);
#endif
#ifdef CONFIG_RTK_PWM
extern unsigned long rtk_pwm_request(int pwm_num);
extern int rtk_pwm_config( int pwm_num, int duty_ns, int period_ns);
extern int rtk_pwm_enable(int pwm_num);
extern void rtk_pwm_disable(int pwm_num);
#endif;

//extern int tx_num, tx_done_num;

shell_cmd("ps",
	 "Shows a list of threads",
	 "",
	 ps);

shell_cmd("pdump",
	 "dump a  thread",
	 "",
	 pdump);	

shell_cmd("dump",
	 "Shows a memory dump",
	 "",
	 hexdump);

shell_cmd("help",
	 "Displays a list of commands",
	 "",
	 help_func);

shell_cmd("?",
	 "Displays a list of commands",
	 "",
	 help_func2);

shell_cmd("kill",
	 "Kills a running thread",
	 "[thread ID]",
	 thread_kill);

shell_cmd("release",
	 "Break a thread out of any wait",
	 "[thread ID]",
	 thread_release);
	 
shell_cmd("sp",
	 "Sets a threads priority",
	 "[thread ID]",
	 set_priority);

shell_cmd("version",
	 "Shows build version",
	 "",
	 print_build_tag);

shell_cmd("reset",
	 "Reset the system",
	 "",
	 reset);

shell_cmd("reboot",
	 "Reset the system",
	 "",
	 reboot);

shell_cmd("uptime",
	 "Shows system uptime",
	 "",
	 uptime);

shell_cmd("date",
	  "show system local time",
	  "",
	  date);

shell_cmd("dw",
	 "dw <Address> <Len>",
	 "",
	 CmdDumpWord);

shell_cmd("db",
	 "db <Address> <Len>",
	 "",
	 CmdDumpByte);

shell_cmd("ew",
	 "ew <Address> <Value1> <Value2>...",
	 "",
	 CmdWriteWord);

shell_cmd("eb",
	 "eb <Address> <Value1> <Value2>...",
	 "",
	 CmdWriteByte);

shell_cmd("mac",
	 "mac <ifname> [mac addr]",
	 "",
	 mac_cmd);

#if defined(CONFIG_RTK_PWM)
shell_cmd("pwm",
	 "pwm",
	 "",
	 pwm_cmd);
#endif
#if 0			//add by yp 2016-2-27
shell_cmd("flash",
	 "flash",
	 "",
	 flash_cmd);
#endif

#ifdef HAVE_BRIDGE
shell_cmd("brconfig",
	 "brconfig",
	 "",
	 brconfig);
#endif
#ifdef ECOS_DBG_STAT
shell_cmd("dbg_stat",
	 "dbg statistic command",
	 "",
	 dbg_stat_cmd)
#endif

#ifdef STATIC_ROUTE
shell_cmd("route",
	 "set static route",
	 "",
	 route_cmd);
#endif

#ifdef CONFIG_IPV6_ROUTE6
shell_cmd("route6",
	 "set ipv6 static route",
	 "",
	 route6_cmd);
#endif

#ifdef ROUTE_SUPPORT
shell_cmd("routed",
	 "RIPv1/2 routing daemon",
	 "",
	 routed_cmd);
#endif

#ifdef ROUTE6D_SUPPORT
shell_cmd("route6d",
	 "RIPng routing daemon",
	 "",
	 route6d_cmd);
#endif

#ifdef HAVE_PPPOE
shell_cmd("pppoe",
	  "set pppoe wan type",
	  "",
	  pppoe_cmd);
#endif



#ifdef HAVE_L2TP
shell_cmd("l2tp",
	  "set l2tp wan type",
	  "",
	  l2tp_cmd);
#endif

#ifdef HAVE_PPTP
shell_cmd("pptp",
	  "set pptp wan type",
	  "",
	  pptp_cmd);
#endif

shell_cmd("ping",
	 "ping",
	 "",
	 ping);

#ifdef CONFIG_PING6
shell_cmd("ping6",
	 "ping6",
	 "",
	 ping6);
#endif

#ifdef CONFIG_IPV6_NDP
shell_cmd("ndp",
	 "ndp",
	 "",
	 ndp);
#endif

#ifdef CONFIG_MCASE_TOOLS
shell_cmd("pim6dd",
	 "pim6dd",
	 "",
	 pim6dd);
#endif

shell_cmd("ifconfig",
	 "ifconfig",
	 "",
	 ifconfig);
#if defined(CONFIG_RTL_819X) && defined(CONFIG_RTL_SORAPIDRECYCLE)

shell_cmd("tcpstats",
	 "tcpstats",
	 "",
	 tcpstats);
#endif

#if defined(CONFIG_RTL_ULINKER)
shell_cmd("uled",
	 "uled",
	 "",
	 uled);
#endif

#if defined(CONFIG_RTL_819X)&&defined(HAVE_FIREWALL)	//jwj:20120626
shell_cmd("ipfw",
	 "ipfw",
	 "",
	 ipfw);
#endif

#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
shell_cmd("fastpath",
	 "fastpath",
	 "",
	 fastpath);
#endif

#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
shell_cmd("trigger_port",
	 "trigger_port",
	 "",
	 trigger_port);
#endif

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
shell_cmd("port_fwd",
	 "port_fwd",
	 "",
	 port_fwd);
#endif


#if defined(CONFIG_RTL_HARDWARE_NAT)
shell_cmd("hw_nat",
	 "hw_nat",
	 "",
	 hw_nat);
#endif

#if defined(CONFIG_RTL_NETSNIPER_SUPPORT)
shell_cmd("netsniper",
	 "netsniper",
	 "",
	 netsniper);
#endif

#if HAVE_NAT_ALG
shell_cmd("alg",
	 "alg",
	 "",
	 alg);
#endif

#ifdef CONFIG_RTL_SPI_FIREWALL_SUPPORT
shell_cmd("spifirewall",
	 "spifirewall",
	 "",
	 spifirewall);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
shell_cmd("iwpriv",
	 "iwpriv",
	 "",
	 iwpriv_cmd);
#ifdef HAVE_WLAN_SCHEDULE
shell_cmd("wlschedule",
	 "wlschedule",
	 "",
	 wlschedule_cmd);
#endif
#endif
#ifdef HAVE_TELNETD
shell_cmd("telnetd",
	 "telnetd",
	 "",
	 telnetd_cmd);
#endif

#ifdef HAVE_SYSTEM_REINIT
shell_cmd("reinit_test",
	 "reinit_test",
	 "",
	 reinit_test_cmd);
#endif

#ifdef HAVE_IPV6FIREWALL
shell_cmd("ip6fw",
		"ip6fw",
		"",
		ip6fw_cmd);
#endif

#ifdef HAVE_DHCP6S
shell_cmd("dhcp6s",
	 "dhcp6s",
	 "",
	 dhcp6s_cmd);
#endif
#ifdef HAVE_TR069
shell_cmd("cwmpClient",
	 "start tr069",
	 "",
	 tr069d_start);
#endif

#ifdef HAVE_NBSERVER
shell_cmd("nmbserver",
	 "nmbserver",
	 "",
	 nmbserver_cmd);
#endif

#ifdef HAVE_HS2_SUPPORT
shell_cmd("hs2",
	 "hs2",
	 "",
	 hs2_cmd);
#endif

#ifndef HAVE_NOETH
shell_cmd("eth",
	 "eth",
	 "",
	 CmdEth);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
shell_cmd("wlan0",
	 "wlan0",
	 "",
	 CmdWlan0);

#ifdef CONFIG_RTL_VAP_SUPPORT
shell_cmd("wlan0-va0",
	 "wlan0-va0",
	 "",
	 CmdWlan0_va0);

#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
shell_cmd("wlan0-va1",
	 "wlan0-va1",
	 "",
	 CmdWlan0_va1);

shell_cmd("wlan0-va2",
	 "wlan0-va2",
	 "",
	 CmdWlan0_va2);

shell_cmd("wlan0-va3",
	 "wlan0-va3",
	 "",
	 CmdWlan0_va3);
#endif

#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
shell_cmd("wlan0-vxd",
	 "wlan0-vxd",
	 "",
	 CmdWlan0_vxd);
#endif

shell_cmd("rssi",
	 "rssi",
	 "",
	 rssi_cmd);

shell_cmd("getmib",
	 "getmib",
	 "",
	 getmib_cmd);

shell_cmd("setmib",
	 "setmib",
	 "",
	 setmib_cmd);

shell_cmd("ob",
	 "ob",
	 "",
	 ob_cmd);

shell_cmd("od",
	 "od",
	 "",
	 od_cmd);

shell_cmd("ow",
	 "ow",
	 "",
	 ow_cmd);

shell_cmd("ib",
	 "ib",
	 "",
	 ib_cmd);

shell_cmd("idd",
	 "idd",
	 "",
	 idd_cmd);

shell_cmd("iw",
	 "iw",
	 "",
	 iw_cmd);

shell_cmd("orf",
	 "orf",
	 "",
	 orf_cmd);

shell_cmd("irf",
	 "irf",
	 "",
	 irf_cmd);
#endif
	 
#ifdef HAVE_WPS
shell_cmd("wsc",
	 "wsc",
	 "",
	 wsc_cmd);
#endif
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN1
shell_cmd("wlan1",
	 "wlan1",
	 "",
	 CmdWlan1);

#ifdef CONFIG_RTL_VAP_SUPPORT
shell_cmd("wlan1-va0",
	 "wlan1-va0",
	 "",
	 CmdWlan1_va0);

#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
shell_cmd("wlan1-va1",
	 "wlan1-va1",
	 "",
	 CmdWlan1_va1);

shell_cmd("wlan0-va2",
	 "wlan0-va2",
	 "",
	 CmdWlan1_va2);

shell_cmd("wlan1-va3",
	 "wlan1-va3",
	 "",
	 CmdWlan1_va3);
#endif

#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
shell_cmd("wlan1-vxd",
	 "wlan1-vxd",
	 "",
	 CmdWlan1_vxd);
#endif
#endif
#endif

#ifdef HAVE_RTL_NFJROM_MP
shell_cmd("mpd",
	 "mpd",
	 "",
	 mpd_cmd);
#endif

shell_cmd("show",
	 "show",
	 "",
	 show_cmd);

#ifdef HAVE_WATCHDOG
shell_cmd("watchdog",
	 "on/off/res/reboot",
	 "",
	 watchdog_cmd);
#endif

shell_cmd("ll",
	 "",
	 "",
	 ll_cmd);

shell_cmd("cat",
	 "",
	 "",
	 cat_cmd);

shell_cmd("hexcat",
	 "",
	 "",
	 hexcat_cmd);

shell_cmd("reinit",
	 "reinit",
	 "",
	 reinit_cmd);

#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
shell_cmd("romeperf",
	 "romeperf",
	 "",
	 romeperf_cmd);
#endif

#ifdef CYGPKG_CPULOAD
shell_cmd("cpuload",
	 "cpuload",
	 "",
	 cpuload_cmd);
#endif

#ifdef HAVE_RLX_PROFILING
shell_cmd("rlxprof",
	 "rlxprof",
	 "",
	 rlxprof_cmd);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_HAVE_WRAPPER
shell_cmd("test_skb",
	 "test_skb",
	 "",
	 test_skb_cmd);

#ifdef CONFIG_RTL_819X_SUSPEND_CHECK
shell_cmd("suspendchk",
	 "suspendchk",
	 "",
	 suspend_check_cmd);
#endif
#endif
	 
shell_cmd("excep",
	 "cause exception",
	 "",
	 excep);
#ifdef HAVE_NAPT
shell_cmd("alias",
	 "show alias table",
	 "",
	 rtl_printAliaslink);
#endif
#if 0 //alarm_debug
int alarm_debug;
shell_cmd("alarm_debug",
	 "alarm_debug",
	 "",
	alarm_debug_cmd);
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)
shell_cmd("rtl_vlan",
	 "rtl_vlan",
	 "",
	 rtl_vlan);
#endif

#ifdef HAVE_RADVD_SHELL
shell_cmd("radvd",
	 "radvd",
	 "",
	 radvd);
shell_cmd("radvdconf",
	 "radvdconf",
	 "",
	 radvdconf);
#endif

#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE)

shell_cmd("mroute",
	 "mroute",
	 "",
	mroute_cmd);
#endif
//---------------------------------------------------------------------------
#ifdef CONFIG_TENDA_ATE_REALTEK
shell_cmd("td_flash",
        "td_flash <set> <get>",
        "",
        td_flash_cmd);


shell_cmd("mtest",
        "mtest <stop> <start>",
        "",
        mtest_cmd);

#endif

CMD_DECL(excep)
{
	int type = 0;
	
	if (argc > 0)
		type = 1;
	cause_exception(type);
	return SHELL_OK;
}
#if defined(CONFIG_RTL_819X) && defined(CONFIG_RTL_SORAPIDRECYCLE)

CMD_DECL(tcpstats)
{
	extern void dumptcpstats();
	dumptcpstats();
	return SHELL_OK;
}

#endif
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_HAVE_WRAPPER
CMD_DECL(test_skb_cmd)
{
	extern void test_skb(void);
	test_skb();
	return SHELL_OK;
}

#ifdef CONFIG_RTL_819X_SUSPEND_CHECK
CMD_DECL(suspend_check_cmd)
{
	extern void suspend_check_cmd_dispatch(int argc, char *argv[]);
	if(argc > 0) {
		suspend_check_cmd_dispatch(argc, (char **)argv);
	}
	else {
		printf("suspendchk on\n");
		printf("suspendchk off\n");
		printf("suspendchk winsize\n");
		printf("suspendchk high\n");
		printf("suspendchk low\n");
		printf("suspendchk info\n");
	}
	return SHELL_OK;
}
#endif
#endif

#ifdef HAVE_RLX_PROFILING
CMD_DECL(rlxprof_cmd)
{
	if (argc >= 1) {
		if (strcmp((char*)argv[0], "start") == 0)
			rlx_prof_start();
		else if (strcmp((char*)argv[0], "stop") == 0)
			rlx_prof_stop();
		else if (strcmp((char*)argv[0], "save") == 0)
		{
			unsigned long state;
			HAL_DISABLE_INTERRUPTS(state);
			rlx_prof_save_result();
			HAL_RESTORE_INTERRUPTS(state);
		}
		else if ((strcmp((char*)argv[0], "mode") == 0) && (argc >= 2))
			rlx_prof_set_cp3_ctrl(atoi((char*)argv[1]));
	}
	else {
		//extern int __gdb_io_parameter_base_address__;
		//printf("GDB IO Base=%p\n", &__gdb_io_parameter_base_address__);
		printf("GDB IO Base=0x%x\n", rlx_gdb_get_param_addr());
	}
	return SHELL_OK;
}
#endif

#ifdef CYGPKG_CPULOAD
CMD_DECL(cpuload_cmd)
{
	int sleep_time;
	
	if (argc >= 1) {
		if (strcmp((char*)argv[0], "start") == 0) {
			sleep_time = 1;
			if (argc >= 2) {
				sleep_time = atoi((char*)argv[1]);
				if (sleep_time < 1)
					sleep_time = 1;
			}
			cpuload_set_sleep_time(sleep_time);
			cpuload_measurement_start();
		}
		else if (strcmp((char*)argv[0], "end") == 0) {
			cpuload_measurement_end();
		}
	}
	return SHELL_OK;
}
#endif

#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
CMD_DECL(romeperf_cmd)
{
	int start=0, end=0;

	if (argc == 2) {
		if (strcmp(argv[0], "reset") == 0) {
			rlx_romeperfReset(atoi(argv[1]));
			return SHELL_OK;
		}
		else {
			start = atoi(argv[0]);
			end = atoi(argv[1]);
			if ((start > end) || (start<0) || (end<0))
				start=end=0;
		}
	}
	rlx_romeperfDump(start, end);
	return SHELL_OK;
}
#endif

CMD_DECL(reinit_cmd)
{
	extern void sys_reinit(void);
	sys_reinit();
	return SHELL_OK;
}

CMD_DECL(ll_cmd)
{
	char *folder;
	
	if (argc == 1)
		folder = (char*)argv[0];
	else
		folder = ".";
	list_dir(folder);
	return SHELL_OK;
}

CMD_DECL(cat_cmd)
{
	if (argc == 1)
		cat_file((char*)argv[0], 0);
	return SHELL_OK;
}

CMD_DECL(hexcat_cmd)
{
	if (argc == 1)
		cat_file((char*)argv[0], 1);
	return SHELL_OK;
}

#ifdef HAVE_WATCHDOG
CMD_DECL(watchdog_cmd)
{
	if (argc == 1) {
		if (strcmp((char*)argv[0], "on") == 0) {
			watchdog_reset();
			watchdog_start();
		}
		else if (strcmp((char*)argv[0], "off") == 0) {
			watchdog_reset();
			REG32(BSP_WDTCNR) |= (0xA5000000);
		}
		else if (strcmp((char*)argv[0], "res") == 0) {
			cyg_uint64 wres = watchdog_get_resolution();
			printf("%lld ms\n", wres/1000/1000);
		}
		else if (strcmp((char*)argv[0], "reboot") == 0) {
			unsigned long flags;			
			HAL_DISABLE_INTERRUPTS(flags);
			REG32(BSP_WDTCNR) = 0;
		}
	}
	return SHELL_OK;
}
#endif


CMD_DECL(show_cmd)
{
	if(argc == 0) {
		printf("show meminfo\n");
		printf("show cluster\n");
		printf("show route\n");
		printf("show mallinfo\n");
		printf("show intr\n");
		printf("show network\n");
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_HAVE_WRAPPER
		printf("show skb\n");
#endif
		printf("show spl\n");
		printf("show schlock\n");
		printf("show idle\n");
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
		printf("show mrvif\n");
		printf("show mrcache\n");
#endif
        #ifdef CONFIG_RTL_819X
		printf("show stack\n");
        #endif
		printf("show PHY\n");
#if defined(CONFIG_RTL_FREEBSD_FAST_PATH) &&defined(QOS_BY_BANDWIDTH)
		printf("show qos\n");
#endif
#ifdef CONFIG_MLD_PROXY
        printf("show mf6c\n");
#endif
		return SHELL_OK;
	}
    
	if (strcmp((char*)argv[0], "meminfo") == 0) {
		cyg_kmem_print_stats();
		//printf("     tx_num=%d\n", tx_num);
		//printf("tx_done_num=%d\n", tx_done_num);
	}
	else if (strcmp((char*)argv[0], "cluster") == 0) {
		extern void dump_cluster_stats(int type);
		int type=1;
		
		if (argc >= 2)
			type = atoi((char*)argv[1]);
		dump_cluster_stats(type);
	}
	else if (strcmp((char*)argv[0], "route") == 0) {
		show_network_tables((pr_fun *)printf);
		printf("\n");
	}
	else if(strcmp((char*)argv[0], "arp") == 0)
	{
		show_arp_tables((pr_fun *)printf);
		printf("\n");
	}
	else if (strcmp((char*)argv[0], "mallinfo") == 0) {
		struct mallinfo info;
		
		info = mallinfo();
		printf("arena: %d\n", info.arena);
		printf("ordblks: %d\n", info.ordblks);
		printf("smblks: %d\n", info.smblks);
		printf("hblks: %d\n", info.hblks);
		printf("hblkhd: %d\n", info.hblkhd);
		printf("usmblks: %d\n", info.usmblks);
		printf("fsmblks: %d\n", info.fsmblks);
		printf("uordblks: %d\n", info.uordblks);
		printf("fordblks: %d\n", info.fordblks);
		printf("keepcost: %d\n", info.keepcost);
		printf("maxfree: %d\n", info.maxfree);
	}
	else if (strcmp((char*)argv[0], "intr") == 0) {
		show_interrupt_table();
	}
	else if (strcmp((char*)argv[0], "network") == 0) {
		show_network();
	}
	else if (strcmp((char*)argv[0], "PHY") == 0) {
		show_phy_stats();
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_HAVE_WRAPPER
	else if (strcmp((char*)argv[0], "skb") == 0) {
		extern void dump_skb_info(void);
		dump_skb_info();
	}
#endif
	else if (strcmp((char*)argv[0], "spl") == 0) {
		extern void dump_spl_info(void);
		dump_spl_info();
	}
	else if (strcmp((char*)argv[0], "schlock") == 0) {
		extern cyg_ucount32 cyg_scheduler_read_lock(void);
		printf("scheduler lock=%x\n", cyg_scheduler_read_lock());
	}
	else if (strcmp((char*)argv[0], "idle") == 0) {
		extern cyg_uint32 idle_thread_loops[];
		printf("idle_thread_loops[0]=0x%x\n", idle_thread_loops[0]);
	}
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	else if (strcmp((char*)argv[0], "mrvif") == 0) {
		show_ip_mr_vif();
		
	}
	else if (strcmp((char*)argv[0], "mrcache") == 0) {
		show_ip_mr_cache();
		
	}
#endif
    #ifdef CONFIG_RTL_819X
    else if (strcmp((char*)argv[0], "stack") == 0) {
            show_stack_usage(argc, argv);
            
        }
    #endif
	#if defined(CONFIG_RTL_FREEBSD_FAST_PATH) &&defined(QOS_BY_BANDWIDTH)
	else if(strcmp((char*)argv[0], "qos") == 0){
		rtl_showQosForNatdEntry();
	}	
	#endif
    #ifdef CONFIG_MLD_PROXY
    else if(strcmp((char*)argv[0], "mf6c") == 0){
        showMF6C();
    }
    #endif
	/*
	else if(strcmp((char*)argv[0],"size")  == 0 ) {
		diag_printf("size %d\n",count_size(argv[1]));
	}
	*/
 	return SHELL_OK;
}

#ifdef HAVE_RTL_NFJROM_MP
shell_cmd("mpd",
	 "mpd",
	 "",
	 mpd_cmd);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef HAVE_WPS
CMD_DECL(wsc_cmd)
{
	cyg_flag_value_t val;
	char *erptr = NULL;
	extern int wscd_state;

	if ((argc != 1) && (argc != 2)) {
		printf("wscd_state=%d\n", wscd_state);
		return SHELL_OK;
	}
	if (argc == 2) {
		if (strcmp((char*)argv[0], "pin") == 0) {
			if (strlen((char*)argv[1]) == 8) {
				char temp[16];
				sprintf(temp, "pin=%s", argv[1]);
				RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", temp, NULL_STR);
			}
		}
	}
	else if (argc == 1) {
		if (strcmp((char*)argv[0], "gen-pin") == 0) {
			char tmpbuf[16];
			generate_pin_code(tmpbuf);
			printf("PIN: %s\n", tmpbuf);
		}
		else if (strcmp((char*)argv[0], "ctx") == 0) {
			dump_wsc_context();
		}
		else if (strcmp((char*)argv[0], "flash_param") == 0) {
			dump_wsc_flash_param();
		}
		else if (strcmp((char*)argv[0], "pbc") == 0) {
			extern void wsc_sig_pbc(void);
			wsc_sig_pbc();
		}
		else if (strcmp((char*)argv[0], "reinit") == 0) {
			extern void wsc_reinit(void);
			wsc_reinit();
		}
		else {	
			val = strtoul((char*)argv[0], &erptr, 0);
		
			if(*erptr) {
				printf("Invalid value\n");
				return SHELL_OK;
			}
			cyg_flag_setbits(&wsc_flag, val);
		}
	}
	return SHELL_OK;
}
#endif
#endif

CMD_DECL(iwpriv_cmd)
{
	iwpriv_main(argc, argv);
	return SHELL_OK;
}
#ifdef HAVE_WLAN_SCHEDULE
CMD_DECL(wlschedule_cmd)
{
	wlschedule_main(argc, argv);
	return SHELL_OK;
}
#endif

#ifdef HAVE_TELNETD
CMD_DECL(telnetd_cmd)
{
	telnetd_start(argc, argv);
	return SHELL_OK;
}
#endif

#ifdef HAVE_SYSTEM_REINIT
CMD_DECL(reinit_test_cmd)
{
	reinit_test(argc, argv);
	return SHELL_OK;
}

#endif

#ifdef HAVE_TR069
CMD_DECL(tr069d_start)
{
	tr069_start(argc,argv);
	return SHELL_OK;
}
#endif

#ifdef HAVE_IPV6FIREWALL
CMD_DECL(ip6fw_cmd)
{
	int i, argc_tmp;	
	char *argv_tmp[15];
	argv_tmp[0] = "ip6fw";
	for(i=0; i<argc; i++)
	{
		argv_tmp[i+1] = argv[i];
	}
	argc_tmp = argc + 1;
	ip6fw_init_main(argc_tmp, argv_tmp);
	return SHELL_OK;
}
#endif


#ifdef HAVE_DHCP6S
CMD_DECL(dhcp6s_cmd)
{
	int i, argc_tmp;
	
	char *argv_tmp[15];
	argv_tmp[0] = "dhcp6s";

	for(i=0; i<argc; i++)
	{
		argv_tmp[i+1] = argv[i];
	}

	argc_tmp = argc + 1;

	//diag_printf("----%s[%d], argc_tmp is %d, argv[0] is %s, argv[1] is %s, argv[2] is %s, argv[3] is %s, argv[4] is %s, argv[5] is %s, argv[6] is %s, argv[7] is %s, argv[8] is %s, argv[9] is %s----\n", __FUNCTION__, __LINE__, 
		//argc_tmp, argv_tmp[0], argv_tmp[1], argv_tmp[2], argv_tmp[3], argv_tmp[4], argv_tmp[5], argv_tmp[6], argv_tmp[7], argv_tmp[8], argv_tmp[9]);
	dhcp6s_main(argc_tmp, argv_tmp);
	return SHELL_OK;
}
#endif


#ifdef HAVE_NBSERVER
CMD_DECL(nmbserver_cmd)
{
	nbserver_start(argc, argv);
	return SHELL_OK;
}
#endif
#ifdef HAVE_HS2_SUPPORT
CMD_DECL(hs2_cmd)
{
	hs2_start(argc, argv);
	return SHELL_OK;
}
#endif


CMD_DECL(ifconfig)
{
	ifconfig_main(++argc, --argv); // ifconfig_main use standard main behavior
	return SHELL_OK;
}

#ifdef CONFIG_PING6
CMD_DECL(ping6)
{
	ping6_main(++argc, --argv); // ping6_main use standard main behavior
	return SHELL_OK;
}
#endif

#ifdef CONFIG_IPV6_NDP
CMD_DECL(ndp)
{
	ndp_main(++argc, --argv); 
	return SHELL_OK;
}
#endif

#ifdef CONFIG_MCASE_TOOLS
CMD_DECL(pim6dd)
{
	pim6dd_main(++argc, --argv);
	return SHELL_OK;
}
#endif

CMD_DECL(ping)
{
	icmp_main(argc, argv);
	return SHELL_OK;
}

#ifdef HAVE_BRIDGE
CMD_DECL(brconfig)
{
	brconfig_main(argc, argv);
	return SHELL_OK;
}
#endif

#ifdef ECOS_DBG_STAT
CMD_DECL(dbg_stat_cmd)
{
	dbg_stat_show();
	return SHELL_OK;
}
#endif

#ifdef STATIC_ROUTE
CMD_DECL(route_cmd)
{
	route_main(argc, argv); 
	return SHELL_OK;
}
#endif
#ifdef CONFIG_IPV6_ROUTE6
CMD_DECL(route6_cmd)
{
	route6_main(++argc, --argv); //route_main use standard main behavior
	return SHELL_OK;
}
#endif

#ifdef ROUTE_SUPPORT
CMD_DECL(routed_cmd)
{
	cyg_routed_start(argc, argv);
	return SHELL_OK;
}
#endif

#ifdef ROUTE6D_SUPPORT
CMD_DECL(route6d_cmd)
{
	cyg_route6d_start(argc, argv);
	return SHELL_OK;
}
#endif

#ifdef HAVE_RADVD_SHELL
CMD_DECL(radvd)
{
	create_radvd();
	return SHELL_OK;
}
CMD_DECL(radvdconf)
{
	radvdconf_main(argc,argv);
	return SHELL_OK;
}
#endif


#ifdef HAVE_PPPOE
int pppoe_start_on_intf(char *intname,int id,unsigned int flag, char *servicename, char *acname, char *username, char *passwd);
int pppoe_stop_on_intf(char *inttnmae, int id);

CMD_DECL(pppoe_cmd)
{
	//rtl_pppoe(argc, argv);
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}	
	if(!strcmp(argv[0],"init"))
		pppoe_init(0);
	else if(!strcmp(argv[0],"start"))
	{
		if(argc<4)
		{
			printf("pppoe start interfacename username password");
			return SHELL_OK;
		}
		pppoe_start_on_intf(argv[1],0,0,NULL,NULL,argv[2],argv[3]);
	}
	else if(!strcmp(argv[0],"stop")) 
	{
		/*argv[1]:intfname*/	
		if(argc<2)
		{
			printf("pppoe stop interfacename");
			return SHELL_OK;
		}
		pppoe_stop_on_intf(argv[1],0);
	}
	else if(!strcmp(argv[0],"connect"))
	{
		if(argc <2)
		{
			printf("pppoe connect connect-type");
			return SHELL_OK;
		}
		if(!strcmp(argv[1],"demand"))
			ppp_dail_on_demand(0);		
	}
	return SHELL_OK;
}
#endif

#ifdef HAVE_L2TP
CMD_DECL(l2tp_cmd)
{
	//rtl_pppoe(argc, argv);
	if(argc < 1) {
		 printf("no parameter\n");
		 return SHELL_OK;
	}	
	else if(!strcmp(argv[0],"start"))
	{
		l2tp_connect();
	}
	else if(!strcmp(argv[0],"stop")) 
	{
		l2tp_disconnect();
	}
	return SHELL_OK;
}
#endif

#ifdef HAVE_PPTP
CMD_DECL(pptp_cmd)
{
	//rtl_pppoe(argc, argv);
	if(argc < 1) {
		 printf("no parameter\n");
		 return SHELL_OK;
	}	
	else if(!strcmp(argv[0],"start"))
	{
		pptp_connect();
	}
	else if(!strcmp(argv[0],"stop")) 
	{
		pptp_disconnect();
	}
	return SHELL_OK;
}

#endif

#if defined(CONFIG_RTL_ULINKER)
CMD_DECL(uled)
{
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}

	if(!strcmp(argv[0],"togOn"))
		set_ulinker_led(1);
	else if(!strcmp(argv[0],"togOff")){
		set_ulinker_led(0);
	}
	
	return SHELL_OK;
}
#endif


#if defined(CONFIG_RTL_819X)&&defined(HAVE_FIREWALL)	//jwj:20120626
CMD_DECL(ipfw)
{
	int i, argc_tmp;
	
	char *argv_tmp[15];
	argv_tmp[0] = "ipfw";

	for(i=0; i<argc; i++)
	{
		argv_tmp[i+1] = argv[i];
	}

	argc_tmp = argc + 1;

	//diag_printf("----%s[%d], argc_tmp is %d, argv[0] is %s, argv[1] is %s, argv[2] is %s, argv[3] is %s, argv[4] is %s, argv[5] is %s, argv[6] is %s, argv[7] is %s, argv[8] is %s, argv[9] is %s----\n", __FUNCTION__, __LINE__, 
		//argc_tmp, argv_tmp[0], argv_tmp[1], argv_tmp[2], argv_tmp[3], argv_tmp[4], argv_tmp[5], argv_tmp[6], argv_tmp[7], argv_tmp[8], argv_tmp[9]);
	ipfw_init_main(argc_tmp, argv_tmp);
	return SHELL_OK;
}
#endif

#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
CMD_DECL(fastpath)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}

	if(!strcmp(argv[0],"show"))
		rtl_readFastpathNaptEntry();
	else if(!strcmp(argv[0],"on")){
		rtl_fastpathOnOff(1);
	}
	else if(!strcmp(argv[0],"off"))
		rtl_fastpathOnOff(0);
	else if(!strcmp(argv[0],"skb"))
	{
		if(!strcmp(argv[1],"show"))
			rtl_readSkbFastpath();
		else if(!strcmp(argv[1],"on")){
			rtl_skbfastpathOnOff(1);
		}
		else if(!strcmp(argv[1],"off"))
			rtl_skbfastpathOnOff(0);
	}
	return SHELL_OK;
}
#endif
#if defined(CONFIG_RTL_HARDWARE_NAT)
CMD_DECL(hw_nat)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}

	if(!strcmp(argv[0],"on"))
		rtl_hwNatOnOff(1);
	else if(!strcmp(argv[0],"off"))
		rtl_hwNatOnOff(0);

	return SHELL_OK;
}
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)
CMD_DECL(rtl_vlan)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}

	if(!strcmp(argv[0],"show"))
		rtl_vlanTableShow();
	else if(!strcmp(argv[0],"show-pvid"))
		rtl_showPortDefVlanInfo();
    else if(!strcmp(argv[0],"set-vid"))
		rtl_setVlanInfo(argc, argv);
    else if(!strcmp(argv[0],"set-pvid"))
		rtl_setVlanPvidInfo(argc, argv);
    else if(!strcmp(argv[0],"del"))
		rtl_delVlanInfo(argc, argv);

	return SHELL_OK;
}
#endif

#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
CMD_DECL(netsniper)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}

	if(!strcmp(argv[0],"show"))
		rtl_shownetsniperEntry();
	else if(!strcmp(argv[0],"on"))
		rtl_netsniperOnOff(0xffffffff);
	else if(!strcmp(argv[0],"off"))
		rtl_netsniperOnOff(0);
    
    else if (!strcmp(argv[0],"ip_on"))
        rtl_netsniperFunctionOnOff("ip", 1);
    else if (!strcmp(argv[0],"ip_off"))
        rtl_netsniperFunctionOnOff("ip", 0); 

    else if (!strcmp(argv[0],"tcp_on"))
        rtl_netsniperFunctionOnOff("tcp", 1);
    else if (!strcmp(argv[0],"tcp_off"))
        rtl_netsniperFunctionOnOff("tcp", 0);

    else if (!strcmp(argv[0],"http_on"))
        rtl_netsniperFunctionOnOff("http", 1);
    else if (!strcmp(argv[0],"http_off"))
        rtl_netsniperFunctionOnOff("http", 0); 

    else if (!strcmp(argv[0],"ntp_on"))
        rtl_netsniperFunctionOnOff("ntp", 1);
    else if (!strcmp(argv[0],"ntp_off"))
        rtl_netsniperFunctionOnOff("ntp", 0); 

	return SHELL_OK;
}
#endif

#if HAVE_NAT_ALG
CMD_DECL(alg)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}
	if(!strcmp(argv[0],"show"))
		rtl_showAlgStatus();
    
	else if(!strcmp(argv[0],"ftp_on"))
		rtl_algOnOff("ftp", 1);
	else if(!strcmp(argv[0],"ftp_off"))
		rtl_algOnOff("ftp", 0);
    
    else if(!strcmp(argv[0],"tftp_on"))
		rtl_algOnOff("tftp", 1);
	else if(!strcmp(argv[0],"tftp_off"))
		rtl_algOnOff("tftp", 0);
    
    else if(!strcmp(argv[0],"rtsp_on"))
		rtl_algOnOff("rtsp", 1);
	else if(!strcmp(argv[0],"rtsp_off"))
		rtl_algOnOff("rtsp", 0);

    else if(!strcmp(argv[0],"pptp_on"))
		rtl_algOnOff("pptp", 1);
	else if(!strcmp(argv[0],"pptp_off"))
		rtl_algOnOff("pptp", 0);

    else if(!strcmp(argv[0],"l2tp_on"))
		rtl_algOnOff("l2tp", 1);
	else if(!strcmp(argv[0],"l2tp_off"))
		rtl_algOnOff("l2tp", 0);

    else if(!strcmp(argv[0],"sip_on"))
		rtl_algOnOff("sip", 1);
	else if(!strcmp(argv[0],"sip_off"))
		rtl_algOnOff("sip", 0);

    else if(!strcmp(argv[0],"h323_on"))
		rtl_algOnOff("h323", 1);
	else if(!strcmp(argv[0],"h323_off"))
		rtl_algOnOff("h323", 0);
    
	return SHELL_OK;
}
#endif

#ifdef CONFIG_RTL_SPI_FIREWALL_SUPPORT
CMD_DECL(spifirewall)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}
	if(!strcmp(argv[0],"show"))
		rtl_show_spifirewall_entry();
    
	else if(!strcmp(argv[0],"on"))
		rtl_spifirewall_onoff(1);
	else if(!strcmp(argv[0],"off"))
		rtl_spifirewall_onoff(0);
    
	return SHELL_OK;
}
#endif

#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
CMD_DECL(trigger_port)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}

	if (!strcmp(argv[0],"on"))
		rtl_enableTriggerPort(1);
	else if (!strcmp(argv[0],"off"))
		rtl_enableTriggerPort(0);
	else if (!strcmp(argv[0],"set"))
		rtl_setTriggerPort(argc, argv);
	else if (!strcmp(argv[0],"show"))
		rtl_showTriggerPort(argv[1]);
	//else if (!strcmp(argv[0],"flush"))
		//rtl_flushTriggerPort(argv[1]);
	//else if (!strcmp(argv[0],"delRule"))
		//rtl_delTriggerRuleByNum(argv[1]);
		
	return SHELL_OK;
}
#endif

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
CMD_DECL(port_fwd)
{
	int value = 0;
	
	if(argc < 1) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}
    
	if (!strcmp(argv[0],"show"))
		rtl_showRtlPortFwdEntry();
		
	return SHELL_OK;
}
#endif

#if defined(CONFIG_RTK_PWM)
CMD_DECL(pwm_cmd)
{
	int pwm_num, duty, period;
	if(argc < 2) {
	     printf("no parameter\n");
	     return SHELL_OK;
	}
    	
	if (argc == 2) {
		pwm_num = atoi((char*)argv[1]);
		if (strcmp((char*)argv[0], "request") == 0) {
			rtk_pwm_request(pwm_num);
			return SHELL_OK;
		}
		else if (strcmp((char*)argv[0], "enable") == 0) {
			rtk_pwm_enable(pwm_num);
			return SHELL_OK;
		}
		else if (strcmp(argv[0], "disable") == 0) {
			rtk_pwm_disable(pwm_num);
			return SHELL_OK;
		}
		
	}
	else if (argc == 4) {
		if (strcmp((char*)argv[0], "config") == 0) {
			pwm_num = atoi((char*)argv[1]);
			duty = atoi((char*)argv[2]);
			period = atoi((char*)argv[3]);
			
			//count with ticks
			rtk_pwm_config(pwm_num, duty*60000, period*60000);
			
			//count with ns, resolution is 60000ns
			//rtk_pwm_config(pwm_num, duty, period);
			
			return SHELL_OK;
		}
	}
	return SHELL_OK;
}
#endif

CMD_DECL(flash_cmd)
{
	int i;
	unsigned char macaddr[6], tmp[4];
	
	if (argc == 1) {
		if (strcmp((char*)argv[0], "hwdump") == 0) {
			dump_hw_settings();
			return SHELL_OK;
		}
		else if (strcmp((char*)argv[0], "hwdefault") == 0) {
			write_hw_settings_to_default();
			return SHELL_OK;
		}
		/*
		else if (strcmp(argv[0], "test") == 0) {
			flash_test();
			return SHELL_OK;
		}*/
	}
	else if (argc == 2) {
		if (strcmp((char*)argv[0], "hwmac") == 0) {
			for (i=0; i<6; i++) {
				strncpy((char*)tmp, (char*)(argv[1]+i*2), 2);
				tmp[2] = '\0'; 
				macaddr[i] = strtoul((char*)tmp, (char **)NULL, 16);
			}
			if (modify_mac_addr_of_hw_settings((char*)macaddr))
				printf("OK\n");
			else
				printf("FAIL\n");
			return SHELL_OK;
		}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef HAVE_WPS
		else if (strcmp((char*)argv[0], "hwpin") == 0) {
			int code;
			
			if (strlen((char*)argv[1]) != 8) {
				printf("PIN hould have 8 digits!\n");
				printf("FAIL\n");
				return SHELL_OK;
			}
			code = atoi((char*)argv[1]);
			if (!validate_pin_code(code)) {
				printf("PIN checksum error!\n");
				printf("FAIL\n");
				return SHELL_OK;
			}
			
			if (modify_pin_code_of_hw_settings((char*)argv[1]))
				printf("OK\n");
			else
				printf("FAIL\n");
			return SHELL_OK;
		}
#endif
#endif
		/*
		else if (strcmp((char*)argv[0], "erase") == 0) {
			void *err_addr, *erase_addr;
			int len;

		        len = 4*1024;
			erase_addr = (void *)strtoul((const char*)(argv[1]), (char **)NULL, 16);
			if (CYG_FLASH_ERR_OK != cyg_flash_erase((cyg_flashaddr_t)erase_addr, len, (cyg_flashaddr_t *)&err_addr))
				diag_printf("FAIL\n");
			else
				diag_printf("OK\n");
			return SHELL_OK;
		}
		*/
	}
#ifdef HAVE_APMIB
	flash_main(argc, argv);
#endif
	return SHELL_OK;
}

CMD_DECL(mac_cmd)
{
	int i;
	unsigned char macaddr[6], tmp[4];
	
	if(argc == 1) {
		if (get_mac_address((char*)argv[0], (char*)macaddr))
			printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
				macaddr[0],macaddr[1],macaddr[2],
				macaddr[3],macaddr[4],macaddr[5]);
			else
				printf( "FAIL\n");
	}
	else if(argc == 2) {
		for (i=0; i<6; i++) {
			strncpy((char*)tmp, (char*)(argv[1]+i*2), 2);
			tmp[2] = '\0'; 
			macaddr[i] = strtoul((char*)tmp, (char **)NULL, 16);
		}
		if (set_mac_address((char*)argv[0], (char*)macaddr))
			printf( "OK\n");
		else
			printf( "FAIL\n");
	}
	return SHELL_OK;
}
extern void kick_event(unsigned int event);

CMD_DECL(reset)
{
	//kick_event(RESET_EVENT);
	//return SHELL_OK;
	do_reset(1);
	return SHELL_OK;
}

CMD_DECL(reboot)
{
	
	
	//kick_event(RESET_EVENT);
	do_reset(1);
	return SHELL_OK;
	//return(reset(argc, argv));
}

CMD_DECL(uptime)
{
	unsigned long sec, mn, hr, day;

	sec = (unsigned long)get_uptime();
	
	day = sec / 86400;
	sec %= 86400;
	hr = sec / 3600;
	sec %= 3600;
	mn = sec / 60;
	sec %= 60;
		
	SHELL_PRINT("%ldday %ldh %ldm %lds\n", day, hr, mn, sec);
	return SHELL_OK;
}

CMD_DECL(date)
{
	time_t timer;
	struct tm *tblock;
	timer = time(NULL);
	tblock = localtime(&timer);
	SHELL_PRINT("Local time is: %s\n",asctime(tblock));
	return SHELL_OK;
}
//---------------------------------------------------------------------------
CMD_DECL(CmdDumpWord)
{
	unsigned int src;
	unsigned int len,i;

	if (argc<1) {
		printf("Wrong argument number!\r\n");
		return SHELL_OK;
	}

	if(argv[0]) {
		src = strtoul((const char*)(argv[0]), (char **)NULL, 16);
		if(src <0x80000000)
			src|=0x80000000;
	}
	else {
		printf("Wrong argument number!\r\n");
		return SHELL_OK;		
	}

	if (argc<2)
		len = 1;
	else
		len= strtoul((const char*)(argv[1]), (char **)NULL, 0);			

	if(len == 0)
		len = 4;
				
	while ( (src) & 0x03)
		src++;

	for(i=0; i< len ; i+=4,src+=16) {
		printf("%08X:	%08X	%08X	%08X	%08X\n",
		src, *(unsigned int *)(src), *(unsigned int *)(src+4), 
		*(unsigned int *)(src+8), *(unsigned int *)(src+12));
	}
	return SHELL_OK;
}

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
static char wlan_interface[40];

CMD_DECL(rssi_cmd)
{
	char buff[256];

	if (argc > 0) {
		if (atoi((char*)argv[0]) >= 0) {
			sprintf(buff, "rssi_dump=%d", atoi((char*)argv[0]));
			RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "set_mib", buff, NULL_STR);
		}
	}
	return SHELL_OK;
}

CMD_DECL(getmib_cmd)
{
	if (argc > 0) {
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "get_mib", argv[0], NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(setmib_cmd)
{
	char buff[256];

	if (argc > 1) {
		sprintf(buff, "%s=%s", argv[0], argv[1]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "set_mib", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(ob_cmd)
{
	char buff[256];

	if (argc > 1) {
		sprintf(buff, "b,%s,%s", argv[0], argv[1]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_reg", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(od_cmd)
{
	char buff[256];

	if (argc > 1) {
		sprintf(buff, "dw,%s,%s", argv[0], argv[1]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_reg", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(ow_cmd)
{
	char buff[256];

	if (argc > 1) {
		sprintf(buff, "w,%s,%s", argv[0], argv[1]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_reg", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(ib_cmd)
{
	char buff[256];

	if (argc > 0) {
		sprintf(buff, "b,%s", argv[0]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_reg", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(idd_cmd)
{
	char buff[256];

	if (argc > 0) {
		sprintf(buff, "dw,%s", argv[0]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_reg", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(iw_cmd)
{
	char buff[256];

	if (argc > 0) {
		sprintf(buff, "w,%s", argv[0]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_reg", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(orf_cmd)
{
	char buff[256];

	if (argc > 2) {
		sprintf(buff, "%s,%s,%s", argv[0], argv[1], argv[2]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_rf", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(irf_cmd)
{
	char buff[256];

	if (argc > 1) {
		sprintf(buff, "%s,%s", argv[0], argv[1]);
		RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_rf", buff, NULL_STR);
	}
	return SHELL_OK;
}

CMD_DECL(CmdWlan)
{
	char buff[256];
	
	if (argc > 0) {
		if (strcmp((char*)argv[0], "rssi") == 0) {
			if (argc > 1) {
				if (atoi((char*)argv[1]) >= 0) {
					sprintf(buff, "rssi_dump=%d", atoi((char*)argv[1]));
					RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "set_mib", buff, NULL_STR);
				}
			}
		}
		else if (strcmp((char*)argv[0], "getmib") == 0) {
			if (argc > 1) {
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "get_mib", argv[1], NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "setmib") == 0) {
			if (argc > 2) {
				sprintf(buff, "%s=%s", argv[1], argv[2]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "set_mib", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "ob") == 0) {
			if (argc > 2) {
				sprintf(buff, "b,%s,%s", argv[1], argv[2]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_reg", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "od") == 0) {
			if (argc > 2) {
				sprintf(buff, "dw,%s,%s", argv[1], argv[2]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_reg", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "ow") == 0) {
			if (argc > 2) {
				sprintf(buff, "w,%s,%s", argv[1], argv[2]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_reg", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "ib") == 0) {
			if (argc > 1) {
				sprintf(buff, "b,%s", argv[1]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_reg", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "idd") == 0) {
			if (argc > 1) {
				sprintf(buff, "dw,%s", argv[1]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_reg", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "iw") == 0) {
			if (argc > 1) {
				sprintf(buff, "w,%s", argv[1]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_reg", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "orf") == 0) {
			if (argc > 3) {
				sprintf(buff, "%s,%s,%s", argv[1], argv[2], argv[3]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "write_rf", buff, NULL_STR);
			}
		}
		else if (strcmp((char*)argv[0], "irf") == 0) {
			if (argc > 2) {
				sprintf(buff, "%s,%s", argv[1], argv[2]);
				RunSystemCmd(NULL_FILE, "iwpriv", wlan_interface, "read_rf", buff, NULL_STR);
			}
		}
		else {
			set_ecos_pr_fun((pr_fun *)printf);
			rtl8192cd_wlan_cmd_dispatch(wlan_interface, argc, (char **)argv);
		}
	}
	else {
		set_ecos_pr_fun((pr_fun *)printf);
		rtl8192cd_proc_help(wlan_interface);
		printf("%s rssi <1|0>\n", wlan_interface);
		printf("%s getmib <mib>\n", wlan_interface);
		printf("%s setmib <mib> <data>\n", wlan_interface);
		printf("%s ob <offset> <data>\n", wlan_interface);
		printf("%s od <offset> <data>\n", wlan_interface);
		printf("%s ow <offset> <data>\n", wlan_interface);
		printf("%s ib <offset>\n", wlan_interface);
		printf("%s idd <offset>\n", wlan_interface);
		printf("%s iw <offset>\n", wlan_interface);
		printf("%s orf <path> <offset> <data>\n", wlan_interface);
		printf("%s irf <path> <offset>\n", wlan_interface);
		#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
		printf("%s passthru_wlan\n",wlan_interface);
		#endif
	}
	return SHELL_OK;
}


CMD_DECL(CmdWlan0)
{
	strcpy(wlan_interface, "wlan0");
	return CmdWlan(argc, argv);
}

#ifdef CONFIG_RTL_VAP_SUPPORT
CMD_DECL(CmdWlan0_va0)
{
	strcpy(wlan_interface, "wlan0-va0");
	return CmdWlan(argc, argv);
}

#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
CMD_DECL(CmdWlan0_va1)
{
	strcpy(wlan_interface, "wlan0-va1");
	return CmdWlan(argc, argv);
}
CMD_DECL(CmdWlan0_va2)
{
	strcpy(wlan_interface, "wlan0-va2");
	return CmdWlan(argc, argv);
}

CMD_DECL(CmdWlan0_va3)
{
	strcpy(wlan_interface, "wlan0-va3");
	return CmdWlan(argc, argv);
}
#endif
#endif

#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
CMD_DECL(CmdWlan0_vxd)
{
	strcpy(wlan_interface, "wlan0-vxd");
	return CmdWlan(argc, argv);
}
#endif
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN1
CMD_DECL(CmdWlan1)
{
	strcpy(wlan_interface, "wlan1");
	return CmdWlan(argc, argv);
}

#ifdef CONFIG_RTL_VAP_SUPPORT
CMD_DECL(CmdWlan1_va0)
{
	strcpy(wlan_interface, "wlan1-va0");
	return CmdWlan(argc, argv);
}

#if RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM >= 4
CMD_DECL(CmdWlan1_va1)
{
	strcpy(wlan_interface, "wlan1-va1");
	return CmdWlan(argc, argv);
}
CMD_DECL(CmdWlan1_va2)
{
	strcpy(wlan_interface, "wlan1-va2");
	return CmdWlan(argc, argv);
}

CMD_DECL(CmdWlan1_va3)
{
	strcpy(wlan_interface, "wlan1-va3");
	return CmdWlan(argc, argv);
}
#endif
#endif

#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
CMD_DECL(CmdWlan1_vxd)
{
	strcpy(wlan_interface, "wlan1-vxd");
	return CmdWlan(argc, argv);
}
#endif
#endif

#ifndef HAVE_NOETH
CMD_DECL(CmdEth)
{
	extern void eth_cmd_dispatch(int argc, char *argv[]);
	if(argc > 0) {
		eth_cmd_dispatch(argc, (char **)argv);
	}
	else {
		printf("eth stats <clear>\n");
		printf("eth swreset\n");
		printf("eth phyreset\n");
		printf("eth desc\n");
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		printf("eth mcat\n");
		#endif
		//printf("eth asiccounter <clear>\n");
		printf("eth opmode\n");
		printf("eth l2\n");
		printf("eth vlan\n");
		printf("eth netif\n");
		printf("eth info\n");
		#if 1//def CONFIG_RTL_VLAN_SUPPORT
		printf("eth acl\n");
		#endif
		
		//printf("eth mcat\n");
		printf("eth l3\n");
		printf("eth diagnostic\n");
		printf("eth portStatus <read>\n");
		printf("eth asicCounter <port all/port port_num/clear>\n");
		printf("eth pvid <read/write port pvid>\n");
		printf("eth phy <read all/read id page reg/write id page reg value>\n");
		#if defined (CONFIG_RTL_IGMP_SNOOPING)
		printf("eth igmp\n");
		#endif
		#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
		printf("eth passthru <read/write mask_value>\n");
		#endif
		#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
		printf("eth ppp\n");
		printf("eth nexthop\n");
		printf("eth ip\n");
		printf("eth arp\n");
		printf("eth event\n");
		#endif
		#ifdef CONFIG_RTL_HARDWARE_NAT
		printf("eth napt\n");
		#endif
		#if defined(CONFIG_RTL_RXTX_STATS_DEBUG)
		printf("eth fullstats\n");
		#endif
        #ifdef CONFIG_RTL_PHY_POWER_CTRL
		printf("eth phypower <show>/<port portnum on/off>\n");
        #endif
	}
	return SHELL_OK;
}
#endif
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE)

int mroute_cmd_dispatch(int argc, char *argv[])
{
	int i;
	unsigned int tmp;
	
	if(strcmp(argv[0],"start")==0)
	{
		if(argc == 3)
		{
			printf("[%s][%d][luminais] create_mrouted(\"%s\", \"%s\")\n", __FUNCTION__, __LINE__, argv[1], argv[2]);
			create_mrouted(argv[1], argv[2]);
		}
		else
			printf("mroute start [wan ifname] [lan ifname]\n");
	}
	else if(strcmp(argv[0], "stop")==0)
	{
		printf("[%s][%d][luminais] clean_mrouted\n", __FUNCTION__, __LINE__);
		clean_mrouted();
	}
	else if(strcmp(argv[0],"set")==0)
	{
		if(strcmp(argv[1],"reserved")==0)
		{
			
			tmp =strtoul((const char*)(argv[2]), (char **)NULL, 16);	
			printf("set mcastReservedEnabled:%d.\n",tmp);
			rtl_set_mrouteReserveEnable(tmp);
		}
	}
	else if(strcmp(argv[0],"get")==0)
	{
		if(strcmp(argv[1],"reserved")==0)
		{
			tmp=rtl_get_mrouteReserveEnable();
			
			printf("mcastReservedEnabled:%d.\n",tmp);
		}
	}
	else
	{
		printf("invalid argument!\n");
	}
		

	
	return SHELL_OK;
}


CMD_DECL(mroute_cmd)
{
	
	if(argc > 0) {
		mroute_cmd_dispatch(argc, (char **)argv);
	}
	else {
		printf("mroute start [wan ifname] [lan ifname]\n");
		printf("mroute stop\n");
		printf("mroute set reserved 1/0\n");
		printf("mroute get reserved \n");
		
	}
	return SHELL_OK;
}
#endif

#ifdef CONFIG_TENDA_ATE_REALTEK
CMD_DECL(td_flash_cmd)
{
       td_flash_main(argc, argv);
       return SHELL_OK;
}

CMD_DECL(mtest_cmd)
{
       mtest_cmd_main(argc, argv);
       return SHELL_OK;
}

#endif

//---------------------------------------------------------------------------
void ddump(unsigned char * pData, int len)
{
	unsigned char *sbuf = pData;	
	int length=len;

	int i=0,j,offset;
	printf(" [Addr]   .0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .A .B .C .D .E .F\r\n" );

	while(i< length)
	{		
			printf("%08X: ", (unsigned int)(sbuf+i) );

			if(i+16 < length)
				offset=16;
			else			
				offset=length-i;
			

			for(j=0; j<offset; j++)
				printf("%02x ", sbuf[i+j]);	

			for(j=0;j<16-offset;j++)	//a last line
			printf("   ");


			printf("    ");		//between byte and char
			
			for(j=0;  j<offset; j++)
			{	
				if( ' ' <= sbuf[i+j]  && sbuf[i+j] <= '~')
					printf("%c", sbuf[i+j]);
				else
					printf(".");
			}
			printf("\n\r");
			i+=16;
	}
}

CMD_DECL(CmdDumpByte)
{
	unsigned int src;
	unsigned int len;

	if (argc<1) {
		printf("Wrong argument number!\r\n");
		return SHELL_OK;
	}
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		

	if (argc<2)
		len = 1;
	else
		len= strtoul((const char*)(argv[1]), (char **)NULL, 0);			

	if(len == 0)
		len = 1;
	
	ddump((unsigned char *)src,len);
	return SHELL_OK;
}

//---------------------------------------------------------------------------
CMD_DECL(CmdWriteWord)
{
	unsigned int src;
	unsigned int value,i;

	if (argc<2) {
		printf("Wrong argument number!\r\n");
		return SHELL_OK;
	}
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	while ( (src) & 0x03)
		src++;

	for(i=0;i<argc-1;i++,src+=4) {
		value= strtoul((const char*)(argv[i+1]), (char **)NULL, 0);	
		*(volatile unsigned int *)(src) = value;
	}
	return SHELL_OK;
}

//---------------------------------------------------------------------------
CMD_DECL(CmdWriteByte)
{
	unsigned int src;
	unsigned char value,i;

	if (argc<2) {
		printf("Wrong argument number!\r\n");
		return SHELL_OK;
	}
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		

	for(i=0;i<argc-1;i++,src++) {
		value= strtoul((const char*)(argv[i+1]), (char **)NULL, 0);	
		*(volatile unsigned char *)(src) = value;
	}
	return SHELL_OK;
}

//---------------------------------------------------------------------------
CMD_DECL(ps)
{
    get_thread_info();
    
    return SHELL_OK;
}

CMD_DECL(pdump)
{
	if(argc != 1) {
		printf("pid needed\n");
		return SHELL_INVALID_ARGUMENT;
	}	
	dump_thread_info(atoi(argv[0]));
    	return SHELL_OK;
}

CMD_DECL(set_priority)
{
    cyg_handle_t thandle = 0;
    cyg_priority_t cur_pri, set_pri;
    //unsigned int tid, pri;
    char *erptr = NULL;

    if(argc == 2) {
	thandle = strtoul((char*)argv[0], &erptr, 0);
	
	if(erptr && *erptr) {
	    SHELL_PRINT("Value '%s' is not a valid thread ID\n", argv[1]);
	    return SHELL_OK;
	}   	    

	set_pri = strtol((char*)argv[1], &erptr, 0);
	
	if(erptr && *erptr) {
	    SHELL_PRINT("Value '%s' is not a valid thread priority\n", argv[2]);
	    return SHELL_OK;
	}

	cur_pri = cyg_thread_get_current_priority(thandle);
	
	SHELL_PRINT("Changing thread %d priority from %d to %d\n", thandle, cur_pri, set_pri);

	cyg_thread_set_priority(thandle, set_pri);

	cur_pri = cyg_thread_get_current_priority(thandle);
	SHELL_PRINT("Thread %d priority now @ %d\n", thandle, cur_pri);
    }
    else SHELL_PRINT("Usage: sp [tid] [priority]\n");

    return SHELL_OK;
}
	

CMD_DECL(help_func)
{
    ncommand_t *shell_cmd = __shell_CMD_TAB__;

    const char cmds[] = "Commands", dsr[] = "Descriptions";
    const char usage[] = "Usage";
    unsigned char i;
    
    SHELL_PRINT("%-11s %-40s %-20s\n", cmds, dsr, usage);

    for(i = 0; i < sizeof(cmds) - 1; i++) putchar('-');
    SHELL_PRINT("    ");
    for(i = 0; i < sizeof(dsr) - 1; i++) putchar('-');
    SHELL_PRINT("                             ");
    for(i = 0; i < sizeof(usage) - 1; i++) putchar('-');
    putchar('\n');

    while(shell_cmd != &__shell_CMD_TAB_END__) {
    	
	SHELL_PRINT("%-11s %-40s %-20s\n",
		   shell_cmd->name,
		   shell_cmd->descr,
		   shell_cmd->usage);
	shell_cmd++;
    }

    return SHELL_OK;
}

CMD_DECL(help_func2)
{
    return(help_func(argc, argv));
}

CMD_DECL(hexdump)
{
    unsigned int i = 0, j = 0;
    unsigned int len = 100, width = 16;
    unsigned char *buf;
    char *cp = NULL;

    switch(argc) {
    case 1:
	buf = (unsigned char *)strtoul((char*)argv[0], &cp, 0);

	if(cp && *cp) {
	    SHELL_PRINT("Value '%s' is not a valid address\n", argv[0]);
	    return SHELL_OK;
	}

	break;

    case 2:
	buf = (unsigned char *)strtoul((char*)argv[0], &cp, 0);

	if(cp && *cp) {
	    SHELL_PRINT("Value '%s' is not a valid address\n", argv[0]);
	    return SHELL_OK;
	}   

	len = strtol((char*)argv[1], &cp, 0);

	if(cp && *cp) {
	    SHELL_PRINT("Value '%s' is not a valid length\n", argv[1]);
	    return SHELL_OK;
	}   	    

	break;

    case 3:
	buf = (unsigned char *)strtoul((char*)argv[0], &cp, 0);

	if(cp && *cp) {
	    SHELL_PRINT("Value '%s' is not a valid address\n", argv[0]);
	    return SHELL_OK;
	}   

	len = strtol((char*)argv[1], &cp, 0);

	if(cp && *cp) {
	    SHELL_PRINT("Value '%s' is not a valid length\n", argv[1]);
	    return SHELL_OK;
	}   	    

	width = strtol((char*)argv[2], &cp, 0);

	if(cp && *cp) {
	    SHELL_PRINT("Value '%s' is not a valid width\n", argv[2]);
	    return SHELL_OK;
	}   	    	
	
	break;

    default:
	SHELL_PRINT("Usage: dump [address] [length] [width]\n");
	return SHELL_OK;
    }

    SHELL_PRINT("%08X: ", (unsigned int)buf);
    
    for(i = 0; i < len; i++) {
	if(i && !(i % width)) {
	    j = i - width;
	    SHELL_PRINT("\t\t");
	    for(; j < i; j++) SHELL_PRINT("%c", isprint(buf[j]) ? buf[j] : '.');
	    SHELL_PRINT("\n%08X: ", (unsigned int)buf + i);
	    j = 0;
	}
	SHELL_PRINT("%02X ", buf[i]);
	j++;
    }
    
    if(j) {
	for(i = width - j; i > 0; i--) SHELL_PRINT("   ");
	SHELL_PRINT("\t\t"); 
	for(i = len - j; i < len; i++) SHELL_PRINT("%c", isprint(buf[i]) ? buf[i] : '.');
    }
    SHELL_PRINT("\n");
    return SHELL_OK;
}

CMD_DECL(thread_kill)
{
    cyg_handle_t th;
    char *erptr = NULL;

    if(argc != 1) {
	SHELL_DEBUG_PRINT("Usage: kill [tid]\n");
	return SHELL_OK;
    }

    th = strtoul((char*)argv[0], &erptr, 0);

    if(*erptr) {
	SHELL_PRINT("Invalid thread handle\n");
	return SHELL_OK;
    }

    cyg_thread_kill(th);
    cyg_thread_delete(th);

    return SHELL_OK;
}

CMD_DECL(thread_release)
{
    cyg_handle_t th;
    char *erptr = NULL;

    if(argc != 1) {
	SHELL_DEBUG_PRINT("Usage: release [tid]\n");
	return SHELL_OK;
    }

    th = strtoul((char*)argv[0], &erptr, 0);

    if(*erptr) {
	SHELL_PRINT("Invalid thread handle\n");
	return SHELL_OK;
    }

    cyg_thread_release(th);
    return SHELL_OK;
}
#if 0 //alarm_debug
int alarm_debug;
void init_debug_buf(unsigned char *ptr,int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		*(ptr+i)=0xa5;
	}
}


CMD_DECL(alarm_debug_cmd)
{
	if(argc != 1)
	{
		diag_printf("invalid input\n");
		return SHELL_OK;
	}
	
	if(!strcmp(argv[0],"enable"))
	{
		alarm_debug = 1;

	}
	else if(!strcmp(argv[0],"disable"))
	{
		alarm_debug = 0;
	}
	else if(!strcmp(argv[0],"dump"))
	{
		alarm_debug = 2;
	}

    return SHELL_OK;
}
#endif
