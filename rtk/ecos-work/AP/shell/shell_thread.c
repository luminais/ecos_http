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

#include <stdio.h>                      /* printf */
#include <string.h>                     /* strlen */
#include <stdlib.h>
#include <semaphore.h>
#include <cyg/infra/cyg_type.h>		/* Atomic type */
#include <cyg/io/io.h>                  /* I/O functions */
#include <cyg/hal/hal_arch.h>           /* CYGNUM_HAL_STACK_SIZE_TYPICAL */
#include <cyg/hal/hal_diag.h>
#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */

#include <shell.h>
#include <shell_err.h>
#include <shell_thread.h>
#include <thread_cleanup.h>

static void
shell_thread_cleanup(void *data)
{
    shell_thread_t *nt = (shell_thread_t *)data;

    /*
     * This is the generic thread cleanup function
     * When a thread is done it's work, it wakes the
     * cleanup thread to clean up its resources and
     * shut it down
     */

    if(nt) {
	//SHELL_DEBUG_PRINT("Thread %d going to cleanup\n", nt->thread_handle);
	//printf("Thread 0x%08x going to cleanup\n", nt->thread_handle);
	cyg_mbox_put(cleanup.mbox_handle, nt);
	cyg_semaphore_post(&cleanup.cleanup_sem);
    }
}

unsigned int
shell_create_thread(shell_thread_t **thread,
		   unsigned int priority,
		   void (*func)(cyg_addrword_t arg),
		   cyg_addrword_t arg,
		   const char *name,
		   void *stack,
		   unsigned int stack_size,
		   void (*cleanup_func)(void *arg))
{
    shell_thread_t	*nt;
    unsigned char	thname[64];
    unsigned int	len;

    /*
     * Be careful, priorities are inverted -- 
     * eg. 31 is lowest, 1 is highest
     */
    if((priority > SHELL_THREAD_MIN_PRIORITY) ||
       (priority < SHELL_THREAD_MAX_PRIORITY))
	return SHELL_PRIORITY_ERROR;

    /* Did we pass in "" for a name? */
    if(name && !*name)
	return SHELL_INVALID_THREAD_NAME;

    /* Make sure we have a function pointer */
    if(!func)
	return SHELL_INVALID_FUNC_PTR;

    /* Allocate space for the thread structure */
    SHELL_ASSERT((nt = malloc(sizeof(shell_thread_t))), SHELL_ALLOC_ERROR);
    memset(nt, 0, sizeof(shell_thread_t));

    if (stack && (stack_size > 0)) { //use external thread stack
    	nt->ext_stack_ptr = stack;
    	nt->stack_ptr = stack;
	nt->stack_size = stack_size;
    }
    else {
    	nt->ext_stack_ptr = NULL;
	/* Allocate space for the thread stack */
	if (stack_size == 0)
		stack_size = SHELL_STACK_DEFAULT;
#if 0
	SHELL_ASSERT((nt->stack_ptr = malloc(stack_size)), SHELL_ALLOC_ERROR);
#else
	if(!(nt->stack_ptr = malloc(stack_size))){
		free(nt);
		SHELL_ERROR(" :%s\n", shell_errors[SHELL_ALLOC_ERROR].err_msg);
		return SHELL_ALLOC_ERROR;
	}
#endif
	nt->stack_size = stack_size;
    }
    memset(nt->stack_ptr, 0, stack_size);
    
    /* Set the thread priority */
    nt->priority = priority;

    /* Give the thread a name if we didn't */
    if(!name) {
	shell_global_thnum++;
	snprintf((char *)thname, sizeof(thname) - 1, "Shell Thread[%d]", (unsigned int)shell_global_thnum);
	name = (char *)thname;
    }

    /*
     * Set the thread name
     */
    len = strlen(name);
#if 0
    SHELL_ASSERT((nt->name = (unsigned char *)malloc(len + 1)), SHELL_ALLOC_ERROR);
#else
	if(!(nt->name = (unsigned char *)malloc(len + 1))){
		free(nt);
		SHELL_ERROR(" :%s\n", shell_errors[SHELL_ALLOC_ERROR].err_msg);
		return SHELL_ALLOC_ERROR;
	}
#endif
    memcpy(nt->name, name, len);
    nt->name[len] = '\0';
    
    /*
     * If a cleanup function is provided, use that
     * If not, use the default
     */
    if(cleanup_func) nt->cleanup = cleanup_func;
    else nt->cleanup = &shell_thread_cleanup;

    /* Attach any args */
    nt->arg = arg;

    /* 
     * If we passed in a pointer and want to maintain reference
     * to the tread, then set the handle here
     */
    if(thread && *thread) *thread = nt;

    /* Create the thread */
    cyg_thread_create(nt->priority,
		      func,
		      (cyg_addrword_t)nt,
		      (char *)nt->name,
		      (void *)nt->stack_ptr,
		      nt->stack_size,
		      &nt->thread_handle,
		      &nt->thread);

    /* Start the thread running */
    cyg_thread_resume(nt->thread_handle);
    
    return SHELL_OK;
}
