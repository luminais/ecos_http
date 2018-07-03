#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern  int get_wlan_switch_state(char *buff, unsigned int buff_sz);
extern int set_wlan_switch_state(const char *json_in) ;
extern int check_and_report_clientlist_state(void);


#define STACK_SIZE	(32*1024)

static cyg_handle_t tai_daemon_handle;
static cyg_handle_t tai_daemon_handle;
static char tai_daemon_stack[STACK_SIZE];
static cyg_thread tai_daemon_thread;



static int tai_loop(void)
{
	char buff[4] = {0} ;
	
	while(1)
	{

		//get_wlan_switch_state(buff , 4);

		//set_wlan_switch_state("0");

		 check_and_report_clientlist_state() ;

		
	#if 0
		if(check_wlanswitch_state())
		{
			report_wlanswitch_state();
		}
	#endif
		
		
		//–›√ﬂ30√Î
		cyg_thread_delay(60*30);

	}
	return 0 ;
}


static void tai_main(void)
{
	
	tai_loop() ;

	return ;
}


int tai_test_start(void)
{
	//if (tai_running == 0) 
	{
		cyg_thread_create(
			8, 
			(cyg_thread_entry_t *)tai_main,
			0, 
			"tai_test",
			tai_daemon_stack, 
			sizeof(tai_daemon_stack), 
			&tai_daemon_handle, 
			&tai_daemon_thread);
		cyg_thread_resume(tai_daemon_handle);
	}
}

