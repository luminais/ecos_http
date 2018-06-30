/*************************************************************************
	> Copyright (C) 2015, Tenda Tech. Co., All Rights Reserved.
	> File Name: cli_tenda.c
	> Description: cli commands of cli console
	> Author: zhuhuan
	> Mail: zhuhuan_IT@outlook.com
	> Version: 1.0
	> Created Time: Wed 23 Dec 2015 09:01:11 AM CST
	> Function List: 

	> History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include <stdio.h>
#include <cli.h>
#include <nvram.h>
#include <time.h>
#include "pi_common.h"

/* cli node */
CYG_HAL_TABLE_BEGIN(__cli_tab__, clitab);
CYG_HAL_TABLE_END(__cli_tab_end__, clitab);

typedef void pr_fun(char *fmt, ...);

/* Functions declaration */
//extern int run_clicmd(char *command_buf);
extern void do_reset(int type);
extern void show_arp_tables(pr_fun *pr);
extern int icmp_main(int argc, char **argv);
extern void show_network_tables(pr_fun *pr);
//extern int ipnat_cmd(int argc, char *argv[]);
#ifndef __CONFIG_A9__
extern int firewall_cmd(int argc, char *argv[]);
#endif
//extern int wl(int argc, char **argv);
//extern int et(int argc, char **argv);
extern int route_main(int argc, char **argv);
extern void show_splx(void);
extern int ifconfig_main(argc, argv);
//extern int ipnat_cmd(int argc, char *argv[]);
extern int et(int ac, char *av[]);

static int cli_reboot(int argc, char *argv[])
{
	//run_clicmd(argv[0]); 	// argv[0] =="reboot"
	do_reset(1);
	
	return 0;
}

static int cli_restart(int argc, char *argv[])
{
	// sys_restart();
	printf("The implementation of this command is not ported yet!\n");
	
	return 0;
}

static int cli_nvram(int argc, char *argv[])
{
	int retval = -1;
	
	retval = nvram_cmd_main(argc, argv);
	
	return retval;
}

static int cli_envram(int argc, char* argv[])
{
	int retval = -1;

	retval = envram_cmd_main(argc, argv);
	
	return retval;
}

static void cli_arp(void)
{
	show_arp_tables((pr_fun *)printf);
	//run_clicmd("show arp");
}

static void cli_tenda_arp(int argc, char* argv[])
{
	tenda_arp_show(argc, argv);
}

//llm add，debug使用，time可以设置时间
#if 1
static int
cli_time_get()
{
	 unsigned long long sec, mn, hr, day;
	 sec = cyg_current_time();

	 printf("Up time: %llu ,", sec);
	 //uptime的单位不是秒，而是10ms
	 sec = sec / 100;
	 day = sec / 86400;
	 sec %= 86400;
	 hr = sec / 3600;
	 sec %= 3600;
	 mn = sec / 60;
	 sec %= 60;
	 printf("Up time: %lluday %lluh %llum %llus\n", day, hr, mn, sec);


 
	 time_t timer;
	 struct tm *tblock;
	 timer = time(NULL);
	 tblock = localtime(&timer);
	 printf("Local time: %s\n",asctime(tblock));
	 
	 return 0;
}

static int
cli_time_set(int argc, char *argv[])
{
	char time_now[6][12];
	struct tm TM;
	time_t manual_time;
	
	if(argc != 4)
	{
		return -1;
	}

	if(3 != sscanf(argv[2],"%[^-]-%[^-]-%s",time_now[0],time_now[1],time_now[2]))
		return -1;

	if(3 != sscanf(argv[3],"%[^:]:%[^:]:%s",time_now[3],time_now[4],time_now[5]))
		return -1;

	TM.tm_year = atoi(time_now[0]) - 1900;
	TM.tm_mon = atoi(time_now[1]) - 1;
	TM.tm_mday = atoi(time_now[2]);
	
	TM.tm_hour = atoi(time_now[3]);
	TM.tm_min = atoi(time_now[4]);
	TM.tm_sec = atoi(time_now[5]);

	manual_time	= mktime(&TM);
	cyg_libc_time_settime(manual_time);

	return 0;
}


static int
cli_time(int argc, char *argv[])
{
	int rc = -1;

	if (argc == 1 || 0 == strcmp(argv[1], "show")) {
		rc = cli_time_get(argc, argv);
	}
	else if (0 == strcmp(argv[1], "set")) {
		rc = cli_time_set(argc, argv);
	}

	if (rc == 0)
		return 0;

usage:
	printf("time [show] [set 2011-01-01 08:00:00]\n");
	return rc;
}
#else
static int
cli_time(int argc, char *argv[])
{
	extern cyg_uint64 board_current_usec(void);
	cyg_uint64 cur_usec = board_current_usec();
	cyg_uint64 cur_time = cyg_current_time();
	time_t now = time(0);

	printf("Up time = %llu, usec=%llu\n", cur_time, cur_usec);
	printf("time = %d, %s\n", now, asctime(localtime(&now)));
	return 0;
}
#endif

#if 0
static int cli_time(int argc, char *argv[])
{
	int retval = -1;

	retval = time_cmd(argc, argv);
	
	return 0;
}
#endif

static int cli_syslog(int argc, char *argv[])
{
	int retval = -1;

	retval = syslog_cmd(argc, argv);
		
	return 0;
}



static int cli_ifconfig(int argc, char *argv[])
{
	int retval = -1;

	retval = ifconfig_cmd(argc, argv);

	return retval;
}

/* Ping command */
static int cli_ping(int argc, char *argv[])
{
	int retval = -1;
	
	retval = icmp_main(--argc, ++argv);
	
	return retval;
}
#if 0
static int cli_ipnat(int argc, char *argv[])
{
	ipnat_cmd(argc, argv);
	
	return 0;
}
#endif
static int cli_firewall(int argc, char *argv[])
{
#ifndef __CONFIG_A9__
	firewall_cmd(argc, argv);
#endif
	return 0;
}

static int cli_wlconf(int argc, char *argv[])
{
	// wlconf_cmd(argc, argv);
	//printf("The implementation of this command is not ported yet!\n");
	wlconf_cmd_main(argc,argv);
	
	return 0;
}

static int cli_et(int argc, char *argv[])
{
	//et(argc, argv);
	//printf("The implementation of this command is not ported yet!\n");
	//yepeng add for shooter debug，2016,6,6
	et_cmd_ifconfig(argc,argv);
	return 0;
}

static int cli_route(int argc, char *argv[])
{
	int retval = -1;
	if(1 == argc)
	{
		show_network_tables((pr_fun *)printf);
		retval = 0;
	}
	else
	{
		retval = route_main(--argc, ++argv);
	}
	
	return retval;
}

static int cli_debug(int argc, char *argv[])
{
	debug_cmd(argc, argv);
	//printf("The implementation of this command is not ported yet!\n");
	
	return 0;
}

static int cli_mbuf(int argc, char *argv[])
{
	int retval = -1;

	retval = mbuf_cmd(argc, argv);
	
	return retval;
}

static int cli_thread_show(int argc, char *argv[])
{
	int retval = -1;

	retval = thread_cmd(argc, argv);
	
	return retval;
}

static void cli_splx(int argc, char *argv[])
{
	show_splx();
}

static void cli_realtek(int argc, char *argv[])
{
	ifconfig_main(argc, argv);
}

/* add by jack,2015-12-31, add iwpriv cmd for realtek */
extern int iwpriv_main(unsigned int argc, unsigned char *argv[]);
static void cli_iwpriv(int argc, char *argv[])
{
	if (argc > 1)
        iwpriv_main(argc-1,argv+1);

}
/* end add by jack */
CLI_NODE("reboot",	cli_reboot,	"reboot");
CLI_NODE("restart",	cli_restart, "restart command");
CLI_NODE("nvram",	cli_nvram,	"nvram command");
CLI_NODE("envram",	cli_envram,	"nvram command");
CLI_NODE("arp", cli_arp, "show arp tables");
CLI_NODE("tenda_arp", cli_tenda_arp, "show tenda_arp tables");
CLI_NODE("time", cli_time, "show current time");
CLI_NODE("syslog",	cli_syslog,	"syslog test");
CLI_NODE("ifconfig", cli_ifconfig,	"broadcom ifconfig command");
CLI_NODE("ping",	cli_ping,	"ping dst to loop");
//#ifdef __CONFIG_NAT__
//CLI_NODE("ipnat", cli_ipnat, "ipnat command");
CLI_NODE("fw",	cli_firewall, "fw command");
//#endif /* __CONFIG_NAT__ */
CLI_NODE("wlconf",	cli_wlconf,	"wlconf <ifname> <up|down|start>");
CLI_NODE("et",	cli_et,	"et");
CLI_NODE("route",	cli_route,	"route cmd");
CLI_NODE("debug",	cli_debug,	"debug <httpd/...> <level>");
CLI_NODE("mbuf", cli_mbuf, "mbuf command");
CLI_NODE("thread",	cli_thread_show,	"show threads");
CLI_NODE("splx", cli_splx,	"show splx");

CLI_NODE("realtek", cli_realtek,	"ifconfig command of realtek");
CLI_NODE("iwpriv",	cli_iwpriv,	"iwpriv <ifname> <name=val>");

//llm add, 调试使用，显示端口状态
extern void show_phy_stats();
static void link_status(int argc, char *argv[])
{
	show_phy_stats();
}
CLI_NODE("link_status",	link_status,	"iwpriv <ifname> <name=val>");

// llm add, debug
extern int malloc_link_failed_counter;
static void stat_link(int argc, char *argv[])
{
	printf("malloc_link_failed_counter %d\n", malloc_link_failed_counter);
}

CLI_NODE("stat_link",	stat_link,	"iwpriv <ifname> <name=val>");


extern int show_build_info(int argc, char *argv[]);
static void build_log(int argc, char *argv[])
{
	show_build_info(argc, argv);
}

CLI_NODE("build",build_log,	"iwpriv <ifname> <name=val>");

#ifdef __TENDA_MEM_H__
static void tenda_mem(int argc, char *argv[])
{
	show_block(argc,argv);
}
CLI_NODE("tenda_mem",	tenda_mem,	"tenda_mem [show/fun] [fun_name]");
#endif
/*lq*/
int APP_DEBUG;
static void app_debug(int argc, char *argv[])
{
	APP_DEBUG = 1;
	if(argc > 1)
	{
		if(atoi(argv[1]) == 0)
		{
			APP_DEBUG = 0;
		}
		else
			APP_DEBUG = 1;
	}
	else
		APP_DEBUG = 1;
}
CLI_NODE("app_debug",	app_debug,	"tenda_mem [show/fun] [fun_name]");
//lq 临时关闭
#if 0
void adjust_priority(int argc,char* argv[]);
static void adjust_data(int argc, char *argv[])
{
	if(argc == 5 || argc == 2)
	{
		adjust_priority(argc,argv);
	}
	else
	{
		printf("for example:\n adj_pri proto(0:tcp 1:udp) index(0,1,2) data_len priority\n");
	}
}
CLI_NODE("adj_pri",	adjust_data,	"tenda_mem [show/fun] [fun_name]");
#endif

//内存数据断点工具
#ifdef __CONFIG_WATCH_POINT__
extern mwacth(int argc, char *argv[]);
static void watchpoint(int argc, char *argv[])
{
	mwatch(argc,argv);
}
CLI_NODE("mwatch",	watchpoint,	"mwatch addr type");
#endif
#ifdef __CONFIG_CE_POWER__
extern void set_ce_power_cli_flag(int value);
extern int ce_power_cli_flag;
static void CE_power_limit_exit(int argc, char *argv[])
{
	if(2 != argc)
	{
		return ;
	}
	if(0 == strcmp("off",argv[1]))
	{
		set_ce_power_cli_flag(1);
	}
	else if(0 == strcmp("on",argv[1]))
	{
		set_ce_power_cli_flag(0);
	}
	return ;
}
CLI_NODE("ce_power",	CE_power_limit_exit,	"iwpriv <ifname> <name=val>");
#endif