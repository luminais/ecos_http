/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    cmd_os.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
*/

#include <sys/param.h>
#include <pkgconf/system.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <cli.h>


char help_thread [] = {"thread <show...>\nthread id pri pri-val\nthread id pri run/suspend/release"};


static int os_show_threads(int argc , char* argv[])
{
            cyg_handle_t thread = 0;
            cyg_uint16 id = 0;
            int more;
        

        	diag_printf("id    state    Pri(Set) Name                      StackBase   Size   usage\n\r");
        	diag_printf("---------------------------------------------------------------------------\n\r");

            /* Loop over the threads, and generate a table row for
             * each.
             */
            do 
            {
                cyg_thread_info info;
                char *state_string;

                more=cyg_thread_get_next( &thread, &id );
                if (thread==0)
                    break;

                cyg_thread_get_info( thread, id, &info );
                
                if( info.name == NULL )
                    info.name = "----------";

                /* Translate the state into a string.
                 */
                if( info.state == 0 )
                    state_string = "RUN";
                else if( info.state & 0x04 )
                    state_string = "SUSP";
                else switch( info.state & 0x1b )
                {
                case 0x01: state_string = "SLEEP"; break;
                case 0x02: state_string = "CNTSLEEP"; break;
                case 0x08: state_string = "CREATE"; break;
                case 0x10: state_string = "EXIT"; break;
                default: state_string = "????"; break;
                }

                /* Now generate the row.
                 */
				diag_printf("%04d  %-8s %-2d ( %-2d) ", id, state_string, info.cur_pri, info.set_pri);
				diag_printf("%-25s 0x%08x  %-6d %-6d", 	info.name, info.stack_base , info.stack_size, 
                                                cyg_thread_measure_stack_usage(thread));
				diag_printf("\r\n");
        	} while (more==true) ;
    return 0;
}

static int os_thread_cmd(int argc , char* argv[])
{
    /* Get the thread id from the hidden control */
    cyg_handle_t thread = 0;
    cyg_uint16 id;

    argc = argc -1;
	
    if ((argc==0) || !strcmp(argv[0], "show"))
    {
        os_show_threads(0,0);
        return 0;
    }

    if (argc > 2)
    {
                if (sscanf( argv[0], "%04d", &id )==0 || 
                ((thread = cyg_thread_find( id ))==0))
                {
                        printf("thread id %s not exist\n", argv[0]);
                        return 0;
                }

        if (!strcmp(argv[1], "pri"))
        {
            cyg_priority_t pri;

            sscanf( argv[2], "%d", &pri );

            cyg_thread_set_priority( thread, pri );
            return 0;
        }

            /* If there is a state field, change the thread state */
        if (!strcmp(argv[1], "state"))
        {
            if( strcmp( argv[2], "run" ) == 0 )
                cyg_thread_resume( thread );
                        else if( strcmp( argv[2], "suspend" ) == 0 )
                                cyg_thread_suspend( thread );
                        else if( strcmp( argv[2], "release" ) == 0 )
                                cyg_thread_release( thread );
                        else if( strcmp( argv[2], "delete" ) == 0 )
                                cyg_thread_delete( thread );
                        else if( strcmp( argv[2], "kill" ) == 0 )
                                cyg_thread_kill( thread );
                        else
                                goto help;

                        cyg_thread_delay(10);
                        os_show_threads(0,0);
                        return 0;
        }
    }
help:
    printf("thread <show...>\n");
    printf("thread id pri pri-val\n");
        printf("thread id state run/suspend/release/delete\n");
    return 0;
}

//CLI_NODE("thread",	os_thread_cmd,	help_thread);

