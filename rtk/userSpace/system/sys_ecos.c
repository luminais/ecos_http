/*add by lqz 2015-12-17.
*when the system is up then go into this file.
*main funciton is sys_main().
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_ecos.h"

extern void init_before_module_init_services();
extern void init_after_module_init_services();
extern void tapf_board_reboot(void);
extern void user_exception_init(void);

int work_mode_is_route = 1;
// added for traffic control with the fastnat of realtek, by zhuhuan on 2016.03.01
#ifndef CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#define CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#endif

#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
unsigned int	br0_ip_addr = 0;
unsigned int	br0_ip_mask = 0;
unsigned int	guest_ip_addr = 0;
unsigned int	guest_ip_mask = 0;
#endif 

#ifndef nvram_safe_get
#define nvram_safe_get(name) (nvram_get(name) ? : "")
#endif
//end added (2016.03.01)

/*定义一个全局变量用来存放各个接收消息中心的相关信息*/
struct moudle_center_struct moudle_center_struct_list[] =
{
	{
		.center = MODULE_RC,
		.name = RC,
		.init = rc_module_init,
		.msg_2_tlv_func = rc_msg_2_tlv,
		.rcv_msg_handle = rc_rcv_msg_handle,
		.callback = rc_callbak,
	},
};

/*定义一个函数指针，用来挂载钩子函数*/
RET_INFO (*sys_msg_init)() __attribute__((weak));
RET_INFO (*sys_msg_loop)() __attribute__((weak));

/*this function is used for get msg center num*/
PIU8 sys_get_center_num()
{
	return ARRAY_SIZE(moudle_center_struct_list);
}

/*this function is only by main().this function is used for init module and mbox main*/
void sys_main()
{
	RET_INFO ret = RET_SUC;
	PIU8 i = 0;

	init_before_module_init_services();
	
	/*mbox init*/
	if(NULL != sys_msg_init)
	{
		if(RET_ERR == (*sys_msg_init)())
		{
			PI_ERROR(MAIN,"msg init fail,reboot!\n");
		}
	}

	/*module init*/
	for(i = 0;i < ARRAY_SIZE(moudle_center_struct_list);i++)
	{
		if(RET_ERR == moudle_center_struct_list[i].init())
		{
			PI_ERROR(MAIN,"center:%d,module:%s init fail!\n",
				moudle_center_struct_list[i].center,moudle_center_struct_list[i].name);
		}
	}

	init_after_module_init_services();

	// added for traffic control with the fastnat of realtek, by zhuhuan on 2016.03.01
	#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	inet_aton(nvram_safe_get("lan_ipaddr"), (struct in_addr *)&br0_ip_addr);
	inet_aton(nvram_safe_get("lan_netmask"), (struct in_addr *)&br0_ip_mask);

	inet_aton(nvram_safe_get("lan1_ipaddr"), (struct in_addr *)&guest_ip_addr);
	inet_aton(nvram_safe_get("lan1_netmask"), (struct in_addr *)&guest_ip_mask);
	#endif

	#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
	#ifndef __CONFIG_A9__
	if(strcmp(nvram_safe_get(SYSCONFIG_WORKMODE), "bridge") == 0
		|| strcmp(nvram_safe_get(SYSCONFIG_WORKMODE), "client+ap") == 0)
	{
		work_mode_is_route = 0;
	}
	else
	{
		work_mode_is_route = 1;
	}
	#endif
	#endif
	
	//general_mib_check();
	
	/*mbox main*/
	if(NULL != sys_msg_loop)
	{
		if(RET_ERR == (*sys_msg_loop)())
		{
			PI_ERROR(MAIN,"msg main init fail reboot!\n");
		}
	}

	return ;
}	
	
void tapf_sys_init(cyg_addrword_t data)
{	
	#if 1
	/*exception handler*/
	install_exception_handler();//del
	#endif

	/* init user exception handler */
	//user_exception_init();
	
	#if 0
	/*init event flag*/
	cyg_flag_init(&sys_flag);///
	
	/*mount ramfs*/
	ramfs_init();///
	syslogAll_printk("Mount ramfs ..\n");
	#endif 
	ramfs_init();

	nvram_init();	
	
	#if 0 //add by z10312  临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -1224
	ecos_nvram_init();
	#endif
	
	/* Entery user main */
	sys_main();
	/* Reboot */
	tapf_board_reboot();
#if 0
	
	/*Start Timer*/
	start_sys_timer(); //del
#ifdef HAVE_SYSTEM_REINIT
	system_reinit_mutex_init();
#endif
	
#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
	rlx_romeperfInit(1);
#endif
	
#ifdef 	ECOS_DBG_STAT
	dbg_stat_init();
#endif
	
	
					
	/*check if mib validate*/	
	syslogAll_printk("Check if mib validate ..\n");
	general_mib_check();
	
	/* init apmib*/
	syslogAll_printk("Init mib ...\n");
	general_mib_init();	
#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) 
#if defined(CONFIG_CUTE_MAHJONG_RTK_UI)
	{
		int val=2;
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);
		cutemj_check_opmode();
	}	
#else
	auto_opmode(1);
#endif
#endif
#if defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER) && (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) || defined(CONFIG_RTL_ULINKER))
	int op_mode;
	unsigned char buffer[32];
	apmib_get(MIB_OP_MODE,(void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif

	if(op_mode == BRIDGE_MODE){
		sprintf(buffer, "wlan0-vxd led 4");
		run_clicmd(buffer);//set wlan led
		create_twinkle_rssi();
	}	
	else{
		sprintf(buffer, "wlan0-vxd led 3");
		run_clicmd(buffer);//set wlan led
	}	
#endif	
			
#if defined(CONFIG_RTL_XDG)
	{
		int val=3;
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);
	}		
#endif	
		
	

	/*set day time. Time Zone and flash time*/
	syslogAll_printk("Init system time ...\n");
	set_time();
#ifdef HAVE_MP_DAEMON
	start_mp();
#else
#ifdef SYSLOG_SUPPORT
	/*init log setting*/
	set_log();
#endif
#ifdef HAVE_WPS
	set_WPS_pin();
#endif

	/*Set up WIFI MIB*/
#ifndef HAVE_NOWIFI
	syslogAll_printk("Init wifi mib ...\n");
	configure_wifi();
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
	syslogAll_printk("config vlan ...\n");
	configure_vlan();
#endif
	
	/*Set up NIC MIB or vlan*/
	syslogAll_printk("config nic ...\n");
	configure_nic();
		
		
		
	/*configure bridge used to setup the bridge and assign ip address*/
	/*and bring the interface up*/
	syslogAll_printk("config bridge ...\n");
	configure_bridge();

	/*configure opmode, this must after configure_bridge
	   since it will deal with NIC asic*/
	syslogAll_printk("config opmode ...\n");
	configure_opmode();
	
#ifdef HAVE_CERT_MNG
	/*
 	 * Start certificate management system
 	 */
	diag_printf("Certificate management ...\n");
	extern int cert_mng_init(int reinit);
	int cert_init_ret=cert_mng_init(0);
	if(cert_init_ret!=1)
		diag_printf("Certificate Management init failed %d!!!\n", cert_init_ret);
	
#endif
	/*start all lan app here
	  * lan app: 
	  *	  web server
	  *     dhcpd
	  *     miniigd
	  *     miniupnp
	  *     ...
	  */
	
	syslogAll_printk("start lan applications ...\n");
	start_lan();

	
#ifdef HAVE_SIMPLE_CONFIG
	start_simple_config();
#endif

//	start_lan_app();

	/*start all wlan app here
	  * wlan app:
	  *       wps
	  *       iwcontrol
	  *       iapp
	  *       ...
	  */
////	syslogAll_printk("start wlan applications ...\n");
////	start_wlan_app();

	/*Doing Wan configuration. if gateway mode
	  * init according to different wan type
	  * configure nat
	  * configure firewall
	  *
	  */
	syslogAll_printk("Config wan ...\n");
#ifdef HOME_GATEWAY
#ifdef CONFIG_RTL_DHCP_PPPOE
	config_wan(SYS_WAN_M);
#else
	config_wan();
#endif
#endif

#ifdef CONFIG_IPV6
	extern void set_ipv6();
	set_ipv6();
#endif
#ifdef HAVE_TR069
	start_tr069();
#endif
#ifdef ULINK_DHCP_AUTO
	start_ulink_dhcpauto();
#endif
#ifdef DHCP_AUTO_SUPPORT
	start_dhcpauto();
#endif
	/*we will enter a while loop, do according to the coming flag*/
	
#if defined(HAVE_DNSPROXY) && !defined(HAVE_RTL_DNSMASQ)
#if defined(ECOS_DOMAIN_NAME_QUERY_SUPPORT)
	cyg_dns_proxy_start();
#endif
#endif
#ifdef DAYLIGHT_SAVING_TIME
	diag_printf("start dst loop...\n");
	start_dst_time_loop();
#endif
	enter_loop();
#endif

#endif
	return;
}
