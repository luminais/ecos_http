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

#if !defined(_SHELL_THREAD_H_)
#define _SHELL_THREAD_H_

#if defined(__cplusplus)
extern "C" {
#endif	/* defined(__cplusplus) */

#define SHELL_THREAD_MAX_PRIORITY	1
#define SHELL_THREAD_MIN_PRIORITY	31
#define SHELL_STACK_MIN			(1024 *  2)	/* 2K minimum stack */
#define SHELL_STACK_MAX			(1024 * 16)	/* 16K maxiumum stack */
#define SHELL_STACK_DEFAULT		(1024 * 4)	/* 4K default stack */

typedef struct _shell_thread_t {
    cyg_priority_t priority;		/* Thread priority */
    cyg_thread thread;			/* Thread object */
    cyg_handle_t thread_handle;		/* Thread handle */
    cyg_addrword_t arg;			/* Argument to thread function */
    unsigned int stack_size;		/* Size of the stack */
    unsigned char *stack_ptr;		/* Pointer to the stack */
    unsigned char *ext_stack_ptr;	/* external thread stack */
    unsigned char *name;		/* Name of thread */
    void (*cleanup)(void *data);	/* Thread specific cleanup callback */
} shell_thread_t;

/* Creates threads */
unsigned int shell_create_thread(shell_thread_t **nt,
				unsigned int priority,
				void (*func)(cyg_addrword_t arg),
				cyg_addrword_t arg,
				const char *name,
				void *stack,
				unsigned int stack_size,
				void (*cleanup_func)(void *arg));
    

#if defined(__cplusplus)
}
#endif	/* defined(__cplusplus) */

#endif	/* _SHELL_THREAD_H_ */
