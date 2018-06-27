/*
 * HTTPD ecos main entrance.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: httpd_ecos.c,v 1.6 2010-05-28 11:03:15 Exp $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <httpd.h>

/*
 * Daemon of HTTPD module.
 * This function calls the portable
 * main loop of the httpd functions.
 */
#define STACK_SIZE	65535

static cyg_handle_t httpd_daemon_handle;
static char http_daemon_stack[STACK_SIZE];
static cyg_thread httpd_daemon_thread;
static int httpd_running = 0;
extern int milli(int argc, char **argv);
extern void tarbase_init();

static void
httpd_main(void)
{
	httpd_running = 1;

	/* Enter os independent main loop */
	tarbase_init();
	milli(0, 0);

	httpd_running = 0;
	return;
}

/*
 * Functions to raise the httpd daemon,
 * called by application main entry and
 * the mointor task.
 */
int
httpd_start(void)
{

	if (httpd_running == 0) {
		cyg_thread_create(
			8, 
			(cyg_thread_entry_t *)httpd_main,
			0, 
			"httpd_main",
			http_daemon_stack, 
			sizeof(http_daemon_stack), 
			&httpd_daemon_handle, 
			&httpd_daemon_thread);
		cyg_thread_resume(httpd_daemon_handle);
	}

	return 0;
}

/* shutdown httpd daemon */
void httpd_stop(void)
{
	return;
}
