/*
 * CLI debug commands.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: cli_basic.c,v 1.10 2010-11-30 07:39:25 Exp $
 */
#include <string.h>
#include <stdio.h>
#include <typedefs.h>
#include <osl.h>
#include <sbchipc.h>
#include <hndsoc.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmnvram.h>
#include <rc.h>
#include <cli.h>
#include <time.h>

extern void cyg_fd_thread_dump(cyg_int16 *thread_id_list);

static int
cli_reboot(int argc, char *argv[])
{
	sys_reboot();
	return 0;
}

static int
cli_restart(int argc, char *argv[])
{
	sys_restart();
	return 0;
}

/* Debug commands */
static int
show_stack_usage(char *base, char *top)
{
	register char *p;

	for (p = base; p < top; p++)
		if (*p)
			break;

	return (top - p);
}

static int 
show_thread_fd(void)
{
	cyg_handle_t t_handle = 0;
	cyg_uint16 id = 0;
	int more;
	cyg_uint16 fd_thread_list[CYGNUM_FILEIO_NFD] = {0};
	int i, j;

	/* Get thread id list */
	cyg_fd_thread_dump(fd_thread_list);

	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	do {
		cyg_thread_info info;

		more = cyg_thread_get_next(&t_handle, &id);
		if (t_handle == 0)
			break;

		cyg_thread_get_info(t_handle, id, &info);
		if (info.name == NULL)
			info.name = "(null thread)";

		printf("\r\n%-22s:", info.name);
		for (i = 0, j = 0; i < CYGNUM_FILEIO_NFD; i++) {
			if (fd_thread_list[i] == id) {
				printf(" [%d]", i);
				j++;
				if ((j%10) == 0)
					printf("\r\n%-22s:", "");
			}
		}
	}
	while (more == true);

	printf("\r\n");
	return 0;
}

static int 
show_threads(void)
{
	cyg_handle_t t_handle = 0;
	cyg_uint16 id = 0;
	int more;
	int stack_usage;

	printf("id    state    Pri(Set) Name                   StackBase"
		    "   Size   usage\n\r");

	/* Loop over the threads, and generate a table row for
	 * each.
	 */
	do {
		cyg_thread_info info;
		char *state_string;

		more = cyg_thread_get_next(&t_handle, &id);
		if (t_handle == 0)
			break;

		cyg_thread_get_info(t_handle, id, &info);
		if (info.name == NULL)
			info.name = "(null thread)";

		/*
		 * Translate the state into a string.
		 */
		if (info.state == 0)
			state_string = "RUN";
		else if (info.state & 0x04)
			state_string = "SUSP";
		else {
			switch (info.state & 0x1b) {
			case 0x01:
				state_string = "SLEEP";
				break;
			case 0x02:
				state_string = "CNTSLEEP";
				break;
			case 0x08:
				state_string = "CREATE";
				break;
			case 0x10:
				state_string = "EXIT";
				break;
			default:
				state_string = "????";
				break;
			}
		}

		/*
		 * Now generate the row.
		 */
		printf("%04d  %-8s %-2d ( %-2d) ",
			id, state_string, info.cur_pri, info.set_pri);
//roy modify
		stack_usage = cyg_thread_measure_stack_usage(t_handle);

		/*show_stack_usage((char *)info.stack_base,
		                                 (char *)(info.stack_base+info.stack_size));*/
		printf("%-22s 0x%08x  %-6d %-6d",
			info.name, info.stack_base, info.stack_size,
			stack_usage);
		printf("\r\n");
	}
	while (more == true);

	return 0;
}

static int
cli_thread_show(int argc, char* argv[])
{
	/* Get the thread id from the hidden control */
	cyg_handle_t t_handle = 0;
	cyg_uint16 id;

	/* Show all threads */
	if (argc == 1 || !strcmp(argv[1], "show")) {
		show_threads();
		return 0;
	}

	if (!strcmp(argv[1], "fd")) {
		show_thread_fd();
		return 0;
	}

	/* Show a specific thread with id */
	if (sscanf(argv[1], "%04hx", &id) == 0 ||
	    (t_handle = cyg_thread_find(id)) == 0) {
		printf("thread id not exist: %s\n", argv[1]);
		goto usage;
	}

	/* Set thread priority */
	if (!strcmp(argv[2], "pri")) {
		cyg_priority_t pri;

		sscanf(argv[3], "%d", &pri);
		cyg_thread_set_priority(t_handle, pri);
		return 0;
	}

	/* If there is a state field, change the thread state */
	if (!strcmp(argv[2], "state")) {
		if (strcmp(argv[3], "run") == 0)
			cyg_thread_resume(t_handle);
		if (strcmp(argv[3], "suspend") == 0)
			cyg_thread_suspend(t_handle);
		if (strcmp(argv[3], "release") == 0)
			cyg_thread_release(t_handle);
		if (strcmp(argv[3], "kill") == 0)
			cyg_thread_kill( t_handle );
		
		return 0;
	}

usage:
	printf("thread <show>\n");
	printf("thread fd\n");
	printf("thread id pri pri-val\n");
	printf("thread id pri run/suspend/release\n");
	return 0;
}

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

/* NVRAM related commands */
static int
cli_nvram_get(int argc, char* argv[])
{
	char *var = NULL;

	if (argc < 3)
		return -1;

	var = nvram_get(argv[2]);
	if (var)
		printf("%s\n", var);

	return 0;
}

static int
cli_nvram_set(int argc, char* argv[])
{
	char *name;
	char *value;

	if (argc < 3)
		return -1;

	name = argv[2];
	value = strchr(name, '=');
	if (value == NULL)
		return 0;

	*value++ = 0;

	nvram_set(name, value);
	return 0;
}

static int
cli_nvram_unset(int argc, char* argv[])
{
	if (argc < 3)
		return -1;

	nvram_unset(argv[2]);
	return 0;
}

static int
cli_nvram_commit(int argc, char* argv[])
{
	nvram_commit();
	return 0;
}

static int
cli_nvram_show(int argc, char* argv[])
{
	char *fb, *sp;
	int len;

	if ((fb = malloc(NVRAM_SPACE)) <= 0) {
		printf("error alloc buf!\n");
		return -1;
	}

	nvram_getall(fb, NVRAM_SPACE);

	if (argc < 3) {
		for (sp = fb; *sp; sp += strlen(sp)+1) {
			printf("%s\n", sp);
		}
	}
	else {
		len = strlen(argv[2]);
		for (sp = fb; *sp; sp += strlen(sp)+1) {
			if (strncmp(sp, argv[2], len) == 0)
				printf("%s\n", sp);
		}
	}

	free(fb);
	return 0;
}

extern int nvram_erase(void);

static int
cli_nvram(int argc, char *argv[])
{
	int rc = -1;

	if (argc < 2)
		goto usage;

	if (!strcmp(argv[1], "get")) {
		rc = cli_nvram_get(argc, argv);
	}
	else if (!strcmp(argv[1], "set")) {
		rc = cli_nvram_set(argc, argv);
	}
	else if (!strcmp(argv[1], "unset")) {
		rc = cli_nvram_unset(argc, argv);
	}
	else if (!strcmp(argv[1], "show")) {
		rc = cli_nvram_show(argc, argv);
	}
	else if (!strcmp(argv[1], "commit")) {
		rc = cli_nvram_commit(argc, argv);
	}
	else if (!strcmp(argv[1], "erase")) {
		rc = nvram_erase();
	}

	if (rc == 0)
		return 0;

usage:
	printf("nvram [get name] [set name=value] [unset name] [show] [commit]...\n");
	return rc;
}

int syslog_open();
int syslog_close();
char *syslog_get();
void syslog(int level, const char *fmtStr, ...);
static int
cli_syslog(int argc, char *argv[])
{
	int len = 0;
	int a = 1;
	char buf[1024] = "0";
	char *msg;
	int max, i = 0;

	if (argc >= 3 && !strcmp(argv[1], "set")) {
		max = atoi(argv[2]);
		printf("Set %d entries to syslog\n", max);
		while (i++ <= max) {
			snprintf(buf, sizeof(buf), "%s%d", buf, a);
			len += strlen(buf);
			syslog(0, buf);
			if (a ==9)
				a = 0;
			else
				a++;
		}
	}
	else if (argc < 2 || !strcmp(argv[1], "show")) {
		syslog_open();
		while((msg = syslog_get())) {
			printf("%s\n", msg);
		}
		syslog_close();
	}
	else {
		printf("syslog [set num] [show]\n");
		return -1;
	}
		
	return 0;
}

extern void HTTP_debug_level(int level);
extern void wl_restart_check_debug_level(int level);
#ifdef __CONFIG_ALINKGW__
extern void probe_info_debug_level(int level);
extern void tpsk_debug_level(int level);
#endif
#ifdef __CONFIG_AUTO_CONN__
extern void auto_conn_debug_level(int level);
#endif
static int cli_debug(int argc, char *argv[])
{
	int debugValue;
	
	if(argc == 3){
		debugValue = atoi(argv[2]);
		if(strcmp(argv[1],"httpd") == 0){
			HTTP_debug_level(debugValue);
		}
		else if(strcmp(argv[1],"wl_restart") == 0){
			wl_restart_check_debug_level(debugValue);
		}
	#ifdef __CONFIG_ALINKGW__
		else if(strcmp(argv[1],"probe") == 0){
			probe_info_debug_level(debugValue);
		}
		else if(strcmp(argv[1],"tpsk") == 0){
			tpsk_debug_level(debugValue);
		}
	#endif
	#ifdef __CONFIG_AUTO_CONN__
		else if(strcmp(argv[1],"auto_conn_route") == 0)
		{
			auto_conn_debug_level(debugValue);
		}
	#endif
		//to be added
	}else{
	#ifdef __CONFIG_ALINKGW__
		printf("debug <httpd | wl_restart | probe | tpsk > <level>\n");
	#else
		printf("debug <httpd | wl_restart> <level>\n");
	#endif
		return -1;
	}

	return 0;
}
#ifdef __CONFIG_ALINKGW__
static int cli_speedtest(int argc, char *argv[])
{
	int duration;
	
	if(argc == 2){
		duration = atoi(argv[1]);
		wan_speed_test(duration);
	}
	else
		printf("speed test need duration.\n");

	return 0;
}
extern void show_probe_info();
static int cli_probe_show(int argc, char *argv[])
{
	int duration;
	
	if(argc == 2){
		if(strcmp(argv[1],"show") == 0)
		show_probe_info();
	}
	else
		printf("show probe info\n");

	return 0;
}
#endif
/* cli node */
CYG_HAL_TABLE_BEGIN(__cli_tab__, clitab);
CYG_HAL_TABLE_END(__cli_tab_end__, clitab);

CLI_NODE("reboot",	cli_reboot,		"reboot");
CLI_NODE("restart",	cli_restart,		"restart command");
CLI_NODE("thread",	cli_thread_show,	"show threads");
CLI_NODE("time",	cli_time,		"show current time");
CLI_NODE("nvram",	cli_nvram,		"nvram [get name] [set name=value] [unset name] [show] [commit] ...");
CLI_NODE("syslog",	cli_syslog,		"syslog test");
CLI_NODE("debug",	cli_debug,		"debug <httpd/...> <level>");
#ifdef __CONFIG_ALINKGW__
CLI_NODE("speedtest",	cli_speedtest,		"speed test");
CLI_NODE("probe",	cli_probe_show,		"show probe info");
#endif
extern void show_splx(void);
static int show_splx_2(int argc, char *argv[])
{
	show_splx();
}

CLI_NODE("splx",	show_splx_2,		"show splx");
#if 0
extern void show_timeouts(void);
static int show_timeouts_2(int argc, char *argv[])
{
	show_timeouts();
}


CLI_NODE("timeouts",show_timeouts_2,		"show timeouts");
#endif
