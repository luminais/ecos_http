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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <cyg/io/io.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h> 
#include <cyg/kernel/kapi.h> 

#include <shell.h>
#include <shell_thread.h>
#include <shell_err.h>
#include <thread_cleanup.h>

cleanup_t cleanup;
static cyg_mbox cleanup_mbox;

void
main_cleanup()
{
    /*
     * This might seem superfluous, but there needs to be 
     * another thread to cleanup after 'main'.  
     * It can't kill itself.
     */

    cyg_handle_t thandle = 0, *thandleptr = &thandle;
    cyg_uint16 tid;
    cyg_thread_info tinfo;

    cyg_thread_get_next(thandleptr, &tid);

    do {
	cyg_thread_get_info(*thandleptr, tid, &tinfo);
	if(!strcmp(tinfo.name, "main")) {
	    //SHELL_DEBUG_PRINT("Found TID for main [%d]\n", tinfo.handle);
	    printf("Found TID for main [0x%08x]\n", tinfo.handle);
	    cyg_thread_kill(thandle);
	    cyg_thread_delete(thandle);
	}
    }
    while(cyg_thread_get_next(thandleptr, &tid));
}

static void
cleanup_thread(cyg_addrword_t data)
{
    shell_thread_t *nt;

    /*
     * This thread sleeps on the cleanup semaphore
     * Any time that gets tagged, we wakeup, read from the mailbox
     * (which should contain the thread info about the thread(s) waiting
     * to be cleaned up, delete the thread, free up it's context,
     * and go back to sleep until more work comes in
     *
     * As a possible optimization for the future -- should I check
     * the mailbox for any more waiting entries?  If the semaphore gets 
     * incremented while in here, do we have a race?  It doesn't look 
     * like it, but check more carefully...
     */
    
    while(1) {
	cyg_semaphore_wait(&cleanup.cleanup_sem);
	
	do {
	    nt = cyg_mbox_get(cleanup.mbox_handle);
	    
	    if(nt) {
		//printf("Cleanup Thread 0x%08x...\n", nt->thread_handle);
		cyg_thread_kill(nt->thread_handle);
		cyg_thread_delete(nt->thread_handle);
		free(nt->name);
		if (nt->ext_stack_ptr == NULL)
			free(nt->stack_ptr);
		free(nt);
	    }
	    else SHELL_PRINT("Cleanup received a NULL in mbox?!\n");
	} while(nt);
    }
}

/*
 * Create a thread specifically to deal
 * with threads when they are done running and need to be
 * cleaned up
 */

void 
create_cleanup_thread(void)
{
    unsigned int err;

    cyg_mbox_create(&cleanup.mbox_handle, &cleanup_mbox);
    cyg_semaphore_init(&cleanup.cleanup_sem, 0);
    
    if((err = shell_create_thread(NULL,
				 5,
				 cleanup_thread,
				 0,
				 "Cleanup Thread",
				 NULL,
				 0,
				 NULL) != SHELL_OK)) {
	SHELL_ERROR("Failed to create Cleanup thread\n");
	HAL_PLATFORM_RESET();
    }
    
    SHELL_DEBUG_PRINT("Created Cleanup thread\n");
}
