/*
 * Author : Patrick Cai
 * Purpose:
 * 	Port TR069 from Linux to eCos. This file is for main.c to create eCos thread.
 * Date   : 2013.08.19
 * Version: 0.0
 */

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/kernel/ktypes.h>         // base kernel types.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>                     // sprintf().
#include <time.h>                      // sprintf().

// imitate telnetd.c and dhcpd/dhcps.h
#define TR069_THREAD_PRIORITY 8
#define TR069_THREAD_STACK_SIZE 0x00004000
static char tr069_startd = 0;
//static char tr069_running = 0;

cyg_uint8	    tr069_stack[TR069_THREAD_STACK_SIZE];
cyg_handle_t	tr069_thread;
cyg_thread  	tr069_thread_object;

typedef struct main_args_T {
	int argc;
	char *argv[24];
} main_args;
//static main_args client_args;

int tr069_start(int argc, char *argv[]);
int cwmpClient_main(main_args *client_args);
void reset_tr069_startd(void){ tr069_startd = 0;}
#ifdef HAVE_SYSTEM_REINIT
extern void cwmp_webClient_exit();
#endif
