#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <network.h>
#include <string.h>
#include <netdb.h>
#include <bcmnvram.h>
#include "elink.h"
#include "elink_log_debug.h"



static ELINK_INFO_STRUCT elink_info = {0, 0};


static RET_INFO tpi_elink_start();
static RET_INFO tpi_elink_restart();
static RET_INFO tpi_elink_stop();



int tpi_elink_set_connect_st(int st)
{
    elink_info.status = st;
    
    printf("%s [%d] elink_info.status = %d\n", __FUNCTION__, __LINE__, st);
    
    return 0;
}

/*以下函数用于api调用*/
RET_INFO tpi_elink_update_info()
{
	elink_info.enable = atoi(nvram_safe_get(ADVANCE_ELINK_ENABLE));

    // Update by elink thread.
    //elink_info.status = 1; // need to update from elink status.
    
	return RET_SUC;
}


/*called by gpi*/
P_ELINK_INFO_STRUCT tpi_elink_get_info()
{
	tpi_elink_update_info();
    
	return &elink_info;
}


extern int elinksdk_main();
RET_INFO tpi_elink_first_init()
{
    elinksdk_main();
    return tpi_elink_start();
}


RET_INFO tpi_elink_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_elink_start();
            break;
		case OP_STOP:
            tpi_elink_stop();
            break;
		case OP_RESTART:
            tpi_elink_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}



static void elink_main_loop()
{

	proto_elink_init();

	/* Real work . */	
	proto_elink_main_loop();
	
	return 0;
}


static int elink_thread_running = 0;
static char elink_daemon_stack[1024*100];
static cyg_handle_t elink_daemon_handle;
static cyg_thread elink_daemon_thread;
static RET_INFO tpi_elink_start()
{
    char *value = NULL;
    
	value = nvram_get(ADVANCE_ELINK_ENABLE);
	if (value == 0 || atoi(value) == 0) 
    {
		printf("%s [%d] - elink not enabled.\n", __FUNCTION__, __LINE__);
        
		return RET_SUC;
	}

    
    if(elink_thread_running == 0)
    {
        cyg_thread_create( 8,
                           (cyg_thread_entry_t *)elink_main_loop,
                           0,
                           "Elink",
                           &elink_daemon_stack,
                           sizeof(elink_daemon_stack),
                           &elink_daemon_handle,
                           &elink_daemon_thread);
        cyg_thread_resume(elink_daemon_handle);
        elink_thread_running = 1;
    }
    return RET_SUC;
}




static RET_INFO tpi_elink_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if(elink_thread_running != 0)
	{
		proto_elink_main_loop_stop();

		/* Wait until thread exit */
		pid = oslib_getpidbyname("Elink");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
        elink_thread_running = 0;
		PI_PRINTF(TPI,"stop success!\n");		
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");
	}

	return ret;
}

static RET_INFO tpi_elink_restart()
{
	RET_INFO ret = RET_SUC;
	
	printf("Elink restart ...\n");
	if(RET_ERR == tpi_elink_stop() || RET_ERR == tpi_elink_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}

	return ret;
}


