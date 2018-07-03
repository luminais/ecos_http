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

#include <cyg/hal/hal_diag.h>
#include <cyg/infra/cyg_type.h>		/* Atomic type */
#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */
#ifdef CYGPKG_CPULOAD
#include <cyg/cpuload/cpuload.h>
#endif
#if defined(SHELL_TCPIP)
#include <network.h>			/* For the TCP/IP stack */
#endif	/* defined(SHELL_TCPIP) */

#define SHELL_DEFINES_GLOBALS
#include <shell.h>
#include <shell_err.h>
#include <shell_thread.h>

#if defined(SHELL_DEBUG)
unsigned int shell_debug = 1;
#else
unsigned int shell_debug = 0;
#endif	/* defined(SHELL_DEBUG) */

/* Global thread count */
cyg_atomic shell_global_thnum = 0;
#ifdef CYGPKG_CPULOAD
cyg_uint32 cpuload_calibration_value;
#endif

void print_build_tag(void);

extern void create_sys_init_thread(void);

static void shell_init_thread(cyg_addrword_t data)
{
	shell_thread_t *nt = (shell_thread_t *)data;

#ifdef CYGPKG_CPULOAD
	cyg_cpuload_calibrate(&cpuload_calibration_value);
	//cpuload_calibration_value = 2277350; //for 8196c
	/*解决CPU占用率统计不准，实际空闲时
	cpuload_calibration_value的值大约为cyg_cpuload_calibrate计算结果增加3-5%*/
	cpuload_calibration_value = cpuload_calibration_value + cpuload_calibration_value * 0.03;
	printf("cpuload_calibration_value=%d\n", cpuload_calibration_value);
#endif

	create_sys_init_thread();
	
	/*
	 * Create the 'init' equivalent thread
	 * At least for the purposes of zombie prevention...
	 */
	create_cleanup_thread();

	/*
	 * Create the interactive shell thread
	 */
	//create_shell_thread();
	
	//main_cleanup();

	nt->cleanup(nt);
}

void cyg_user_start(void)
{
    /*
     * This is the entry point from the OS
     * Set everything up in here
     */
    diag_printf("Creating Init Thread...\n");
    
    if (shell_create_thread(NULL,
			  8,
			  shell_init_thread,
			  0,
			  "Init Thread",
			  0,
			  0,
		 	  NULL) != SHELL_OK) {
	diag_printf("Failed to create Init Thread\n");
	return;
    }
}
