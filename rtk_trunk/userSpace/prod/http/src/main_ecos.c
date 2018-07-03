/*
 * HTTPD ecos main entrance.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: httpd_ecos.c,v 1.6 2010/05/28 11:03:15 Exp $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <ecos_oslib.h>

/*
 * Daemon of HTTPD module.
 * This function calls the portable
 * main loop of the httpd functions.
 */
//#define STACK_SIZE	(64*1024)
#define STACK_SIZE	(32*1024)

static cyg_handle_t httpd_daemon_handle;
static char http_daemon_stack[STACK_SIZE];
static cyg_thread httpd_daemon_thread;
static int httpd_running = 0;

int		finished = 0;			/* Finished flag */

#ifdef __CONFIG_A9__
extern void reset_static_wds_scan();
#endif


extern void httpd_mainloop(void);
extern void mfg_mainloop(void);

static void
httpd_main(void)
{
	httpd_running = 1;

	/* Enter os independent main loop */
	httpd_mainloop();

	httpd_running = 0;
	return;
}

/*
 * Functions to raise the httpd daemon,
 * called by application main entry and
 * the mointor task.
 */
void tenda_httpd_start(void)
{

	if (httpd_running == 0) {
		cyg_thread_create(
			6, 
			(cyg_thread_entry_t *)httpd_main,
			0, 
			"httpd_main",
			http_daemon_stack, 
			sizeof(http_daemon_stack), 
			&httpd_daemon_handle, 
			&httpd_daemon_thread);
		cyg_thread_resume(httpd_daemon_handle);
	}

	return ;
}

/* shutdown httpd daemon */
void tenda_httpd_stop(void)
{

#ifdef __CONFIG_A9__
	cyg_thread_kill (httpd_daemon_handle);
	cyg_thread_delete (httpd_daemon_handle);
	httpd_running = 0;
	reset_static_wds_scan();//static_wds_scan = callwdsscan
#endif
	return;
}
