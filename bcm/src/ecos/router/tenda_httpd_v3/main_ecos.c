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

	return ;
}

/* shutdown httpd daemon */
void tenda_httpd_stop(void)
{
#if 0
	int pid;

	finished = 1;

	/* Wait until thread exit */
	pid = oslib_getpidbyname("httpd_main");
	if (pid) {
		while (oslib_waitpid(pid, NULL) != 0)
			cyg_thread_delay(1);
	}
	return;
#else 
	return;
#endif
}

#define MFG_STACK_SIZE (1024*8)
static cyg_uint8 mfg_stack[MFG_STACK_SIZE];
static cyg_handle_t mfg_thread_handle;
static cyg_thread mfg_thread_struct;

static int mfg_running = 0;

void mfg_start(void)
{
	if(mfg_running == 0){
		diag_printf("call_mfg.....(stack size=%d)\n",MFG_STACK_SIZE);

		cyg_thread_create(
					19,
		               	(cyg_thread_entry_t *)mfg_mainloop,
		               	0,
		               	"mfg_daemon",
		               	mfg_stack,
		               	sizeof(mfg_stack),
		               	&mfg_thread_handle,
		               	&mfg_thread_struct);

		mfg_running = 1;
		
		cyg_thread_resume( mfg_thread_handle);
	}
}
