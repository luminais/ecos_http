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
#include <cyg/infra/cyg_type.h>
//#include <cyg/hal/var_ints.h>
#include <cyg/kernel/kapi.h>

#include <commands.h>
#include <shell_err.h>

static const char *thread_state_str[] = {
    "RUN",	/* 0 - Running */
    "SLEEP",	/* 1 - Sleeping */
    "CNTSLP",	/* 2 - Counted sleep */
    "SLPSET",	/* 3 - Sleep set */	
    "SUSP",	/* 4 - Suspended */
    NULL,	/* 5 - INVALID */
    NULL,	/* 6 - INVALID */
    NULL,	/* 7 - INVALID */
    "CREAT",	/* 8 - Creating */
    NULL,	/* 9 - INVALID */
    NULL,	/* 10 - INVALID */
    NULL,	/* 11 - INVALID */
    NULL,	/* 12 - INVALID */
    NULL,	/* 13 - INVALID */
    NULL,	/* 14 - INVALID */
    NULL,	/* 15 - INVALID */
    "EXIT"	/* 16 - Exiting */
};

/*
 * This function produces a list of the threads
 * that are currently scheduled.  The output is roughly
 * analagous to 'ps', with the exception of VSS for obvious
 * reasons
 */
#if defined (CONFIG_RTL_819X)
void get_thread_info(void)
{
    cyg_handle_t thandle = 0, *thandleptr = &thandle;
    cyg_uint16 tid;
    cyg_thread_info tinfo;
    int total_stack_size = 0, total_stack_used = 0;
    /*
     * Because we're running this on ourselves,
     * it's basically guaranteed to be OK the first
     * time through
     */
    cyg_thread_get_next(thandleptr, &tid);

    printf("%-10s %-2s %-6s %-15s %-2s %-2s %-10s %-10s %-10s\n",
           "----------",
           "--",
           "------",
           "----------------------",
           "--",
           "--",
           "----------",
           "----------",
           "----------");
    printf("%-10s %-2s %-6s %-22s %-2s %-2s %-10s %-10s %-10s\n",
           "Handle",
           "ID",
           "State",
           "Name",
           "SP",
           "CP",
           "Stack Base",
           "Stack Size",
           "Stack Used");
    printf("%-10s %-2s %-6s %-22s %-2s %-2s %-10s %-10s %-10s\n",
           "----------",
           "--",
           "------",
           "----------------------",
           "--",
           "--",
           "----------",
           "----------",
           "----------");

    do {
    cyg_thread_get_info(*thandleptr, tid, &tinfo);
    
    printf("0x%08x %-2d %-6s %-22s %-2d %-2d 0x%08x 0x%08x 0x%08x\n",
           tinfo.handle,
           tinfo.id,
           thread_state_str[tinfo.state],
           tinfo.name,
           tinfo.set_pri,
           tinfo.cur_pri,
           tinfo.stack_base,
           tinfo.stack_size,
           tinfo.stack_used);

    total_stack_size += tinfo.stack_size;
    total_stack_used += tinfo.stack_used;
    } while(cyg_thread_get_next(thandleptr, &tid));

    printf("Total Stack Size:  %d\n", total_stack_size);
    printf("Total Stack Used:  %d\n", total_stack_used);
//#if defined(THREADINFO_DEBUG)
    printf("Here I am:\n");
    printf("Handle-> %d\n", tinfo.handle);
    printf("ID-> %d\n", tinfo.id);
    printf("State-> 0x%08x\n", tinfo.state);
    printf("Name-> '%s'\n", tinfo.name);
    printf("Set Priority-> 0x%08x\n", tinfo.set_pri);
    printf("Current Priority-> 0x%08x\n", tinfo.cur_pri);
    printf("Stack base-> 0x%08x\n", tinfo.stack_base);
    printf("Stack size-> 0x%08x\n", tinfo.stack_size);
    printf("Stack used-> 0x%08x\n", tinfo.stack_used);
//#endif	/* defined(THREADINFO_DEBUG) */
}

void dump_current_thread()
{
	unsigned long flag;
	/*dump current should be early, to avoid recursive interrupt*/
	HAL_DISABLE_INTERRUPTS(flag);
	cyg_thread_dump_info(cyg_thread_self(),cyg_thread_get_id(cyg_thread_self()),1);
       HAL_ENABLE_INTERRUPTS();
}
void dump_thread_info(cyg_uint16 pid)
{
    cyg_handle_t thandle = 0, *thandleptr = &thandle;
    cyg_uint16 tid;
    cyg_uint16 cur_tid;
    cyg_thread_info tinfo;
    /*
     * Because we're running this on ourselves,
     * it's basically guaranteed to be OK the first
     * time through
     */
     cur_tid=cyg_thread_get_id(cyg_thread_self());
     if(pid && pid == cur_tid) {
	 	diag_printf("dump myself\n");
		return;
     }		
    cyg_thread_get_next(thandleptr, &tid);


    do {
		if(pid==0 || pid == tid) {
			if(cur_tid == tid) {
				/*Do not dump myself to avoid failed*/
				continue;
			}		
			cyg_thread_get_info(*thandleptr, tid, &tinfo);
			diag_printf("\n");			
#if 1
			diag_printf("%-10s %-2s %-6s %-15s %-2s %-2s %-10s %-10s %-10s\n",
				   "----------",
				   "--",
				   "------",
				   "----------------------",
				   "--",
				   "--",
				   "----------",
				   "----------",
				   "----------");
			diag_printf("%-10s %-2s %-6s %-22s %-2s %-2s %-10s %-10s %-10s\n",
				   "Handle",
				   "ID",
				   "State",
				   "Name",
				   "SP",
				   "CP",
				   "Stack Base",
				   "Stack Size",
				   "Stack Used");
			diag_printf("%-10s %-2s %-6s %-22s %-2s %-2s %-10s %-10s %-10s\n",
				   "----------",
				   "--",
				   "------",
				   "----------------------",
				   "--",
				   "--",
				   "----------",
				   "----------",
				   "----------");
			diag_printf("0x%08x %-2d %-6s %-22s %-2d %-2d 0x%08x 0x%08x 0x%08x\n\n",
				tinfo.handle,
				tinfo.id,
				thread_state_str[tinfo.state],
				tinfo.name,
				tinfo.set_pri,
				tinfo.cur_pri,
				tinfo.stack_base,
				tinfo.stack_size,
				tinfo.stack_used);
#endif			
			cyg_thread_dump_info(*thandleptr, tid, 0);
			if(pid > 0)
				break;
		} 
    } while(cyg_thread_get_next(thandleptr, &tid));

}

#else
void get_thread_info(void)
{
    cyg_handle_t thandle = 0, *thandleptr = &thandle;
    cyg_uint16 tid;
    cyg_thread_info tinfo;
    int total_stack_size = 0, total_stack_used = 0;
    /*
     * Because we're running this on ourselves,
     * it's basically guaranteed to be OK the first
     * time through
     */
    cyg_thread_get_next(thandleptr, &tid);

    SHELL_PRINT("%-10s %-2s %-6s %-15s %-2s %-2s %-10s %-10s %-10s\n",
           "----------",
           "--",
           "------",
           "----------------------",
           "--",
           "--",
           "----------",
           "----------",
           "----------");
    SHELL_PRINT("%-10s %-2s %-6s %-22s %-2s %-2s %-10s %-10s %-10s\n",
           "Handle",
           "ID",
           "State",
           "Name",
           "SP",
           "CP",
           "Stack Base",
           "Stack Size",
           "Stack Used");
    SHELL_PRINT("%-10s %-2s %-6s %-22s %-2s %-2s %-10s %-10s %-10s\n",
           "----------",
           "--",
           "------",
           "----------------------",
           "--",
           "--",
           "----------",
           "----------",
           "----------");

    do {
    cyg_thread_get_info(*thandleptr, tid, &tinfo);
    
    SHELL_PRINT("0x%08x %-2d %-6s %-22s %-2d %-2d 0x%08x 0x%08x 0x%08x\n",
           tinfo.handle,
           tinfo.id,
           thread_state_str[tinfo.state],
           tinfo.name,
           tinfo.set_pri,
           tinfo.cur_pri,
           tinfo.stack_base,
           tinfo.stack_size,
           tinfo.stack_used);

    total_stack_size += tinfo.stack_size;
    total_stack_used += tinfo.stack_used;
    } while(cyg_thread_get_next(thandleptr, &tid));

    SHELL_PRINT("Total Stack Size:  %d\n", total_stack_size);
    SHELL_PRINT("Total Stack Used:  %d\n", total_stack_used);
//#if defined(THREADINFO_DEBUG)
    printf("Here I am:\n");
    printf("Handle-> %d\n", tinfo.handle);
    printf("ID-> %d\n", tinfo.id);
    printf("State-> 0x%08x\n", tinfo.state);
    printf("Name-> '%s'\n", tinfo.name);
    printf("Set Priority-> 0x%08x\n", tinfo.set_pri);
    printf("Current Priority-> 0x%08x\n", tinfo.cur_pri);
    printf("Stack base-> 0x%08x\n", tinfo.stack_base);
    printf("Stack size-> 0x%08x\n", tinfo.stack_size);
    printf("Stack used-> 0x%08x\n", tinfo.stack_used);
//#endif	/* defined(THREADINFO_DEBUG) */
}
#endif

#ifdef CONFIG_RTL_819X
typedef void (timeout_fun)(void *);
typedef struct callout {
    struct callout *next, *prev;
    unsigned int     delta;  // Number of "ticks" in the future for this timeout
    timeout_fun  *fun;    // Function to execute when it expires
    void         *arg;    // Argument to pass when it does
    int           flags;  // Info about this item
} timeout_entry;
externC int cyg_hz;
static int time_interval = 0;
struct callout stack_timer;
void handle_stackusage_timeout(void)
{
    get_thread_info();
	cyg_callout_reset(&stack_timer, time_interval, handle_stackusage_timeout, 0);
}
void stop_show_stack_usage(void)
{
    cyg_callout_stop(&stack_timer);
}

void show_stack_usage(int argc, char *argv[])
{
    int time = 0;
    int str_len = 0, i = 0;

    if (argc == 1)
    {   
        //diag_printf("%s:argv0 %s\n", __FUNCTION__, argv[0]);
        get_thread_info();
        return;

    }
    else if (argc == 2)
    {
        //diag_printf("%s:argv0 %s argv1 %s \n", __FUNCTION__, argv[0], argv[1]);
        if(argv[1]) 
        {
            if(!strcmp(argv[1],"stop"))
            {
                stop_show_stack_usage();
                return;
            }
            str_len = strlen(argv[1]);
            for (i = 0; i < str_len; i++)
            {
                if (!isdigit(argv[1][i]))
                {
                    printf("usage example:  show stack(display only once)\n\t\tshow stack 5(unit second)\n\t\tshow stack stop\n");
                    return;
                }
                    
            }
            time = strtoul((const char*)(argv[1]), (char **)NULL, 16);
            if(time <= 0)
            {      
                printf("Invalid time interval!\n");
                return;
            }
            time_interval = time*cyg_hz;
            get_thread_info();
            cyg_callout_init(&stack_timer);
            cyg_callout_reset(&stack_timer, time_interval, handle_stackusage_timeout, 0);           
            return;
        }
        else 
        {
            printf("usage example:  show stack(display only once)\n\t\tshow stack 5(unit second)\n\t\tshow stack stop\n");
            return;        
        }
    }
    else
    {
        printf("usage example:  show stack(display only once)\n\t\tshow stack 5(unit second)\n\t\tshow stack stop\n");
        return; 
    }

}
#endif
