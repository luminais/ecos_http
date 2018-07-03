#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/kernel/kapi.h>
#include <cyg/io/flash.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include "sys_utility.h"
#include "hw_settings.h"
#include "../network/net_api.h"

unsigned char onesec_stack[4*1024];
cyg_handle_t onesec_thread_handle;
cyg_thread onesec_thread_obj;

void onesec_main(cyg_addrword_t data)
{
	while (1) {
		sleep(1);
		//do something
	}
}

void create_onesec(void)
{
	/* Create the thread */
	cyg_thread_create(20,
		      onesec_main,
		      0,
		      "onesec",
		      &onesec_stack,
		      sizeof(onesec_stack),
		      &onesec_thread_handle,
		      &onesec_thread_obj);
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(onesec_thread_handle);
}
