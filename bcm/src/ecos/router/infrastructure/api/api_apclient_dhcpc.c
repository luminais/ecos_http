#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi_common.h"

#define APCLIENT_DHCPC_SIZE		8*1024

static cyg_handle_t apclient_dhcpc_daemon_handle;
static char apclient_dhcpc_daemon_stack[APCLIENT_DHCPC_SIZE];
static cyg_thread apclient_dhcpc_daemon_thread;

extern PI_INFO init_apclient_dhcpc_info();
extern void tpi_apclient_main(void);
extern PI_INFO tpi_set_apclient_dhcpc_info(char *ip,char *mask);
extern PI_INFO tpi_apclient_lan_dhcp_action(PI_ACTION action);

PI_INFO api_apclient_lan_dhcp_action(PI_ACTION action)
{
	return tpi_apclient_lan_dhcp_action(action);
}

PI_INFO api_set_apclient_dhcpc_addr(char *ip,char *mask)
{
	if(NULL == ip || NULL == mask)
		return PI_ERR;
		
	return tpi_set_apclient_dhcpc_info(ip,mask);	
}

int api_apclient_dhcpc_main_loop()
{	
	if(PI_SUC != init_apclient_dhcpc_info())
	{
		return -1;
	}
	
	cyg_thread_create(
		8, 
		(cyg_thread_entry_t *)tpi_apclient_main,
		0, 
		"apclient_dhcpc",
		apclient_dhcpc_daemon_stack, 
		sizeof(apclient_dhcpc_daemon_stack), 
		&apclient_dhcpc_daemon_handle, 
		&apclient_dhcpc_daemon_thread);
	cyg_thread_resume(apclient_dhcpc_daemon_handle);
       
	cyg_thread_delay(1);
	PI_PRINTF(API,"start end\n");
	return 0;
}
