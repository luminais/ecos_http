#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <network.h>

#include "ulink_dhcp_auto.h"
#include "common.h"
#include "../system/sys_utility.h"
/*
*	ulink dhcp auto, only work in birdge mode.
*	when eth port is off link, run dhcp server to offer ip_address 
* 	when eth port is on link, run dhcp client as a host
*	client pc use domain name to visit dut
*/

#define ULINK_DHCPAUTO_THREAD_PRIORITY 16
#define ULINK_DHCPAUTO_THREAD_STACK_SIZE 0x00005000
#define CYG_NET_TELNET_SERVEROPT_PORT 23

static char ulink_dhcpauto_started=0;
static char ulink_dhcpauto_running=0;
#ifdef HAVE_SYSTEM_REINIT
static char ulink_dhcpauto_cleaning=0;
#endif
cyg_uint8  ulink_dhcpauto_stack[ULINK_DHCPAUTO_THREAD_STACK_SIZE];
cyg_handle_t ulink_dhcpauto_thread;
cyg_thread  ulink_dhcpauto_thread_object;


extern char dhcpd_started;
extern char dhcpd_running;
extern cyg_handle_t  dhcpd_thread;
extern char landhcpc_running;   
extern char landhcpc_started;
extern cyg_handle_t  landhcpc_thread;

char * print_status(ULINK_DHCP_AUTO_STATUS status)
{
	switch(status)
	{
		case ULINK_DHCP_AUTO_STATUS_OFFLINK:
			return "ULINK_DHCP_AUTO_STATUS_OFFLINK";
		case ULINK_DHCP_AUTO_STATUS_REQIP:
			return "ULINK_DHCP_AUTO_STATUS_REQIP";
		case ULINK_DHCP_AUTO_STATUS_GETIP:
			return "ULINK_DHCP_AUTO_STATUS_GETIP";
		default:
			return "ULINK_DHCP_AUTO_STATUS_UNKNOWN";
	}
}
int kill_dhcpd(void)
{
	if(dhcpd_started && dhcpd_running)
	{
		cyg_thread_suspend(dhcpd_thread);
		dhcpd_running=0;
	}
	return 0;
}
extern int dhcpd_startup(cyg_uint8);
void dhcpd_restart(cyg_uint8 );
void dhcpc_reconnect(cyg_uint8 );

int run_dhcpd(void)
{
	ulink_dhcpauto_debug_printf("run_dhcpd\n");
	if(!dhcpd_started)
	{
		ulink_dhcpauto_debug_printf("dhcpd_startup\n");
		dhcpd_startup(0);
	}else
	{
		if(!dhcpd_running)
		{
			ulink_dhcpauto_debug_printf("dhcpd cyg_thread_resume\n");
			cyg_thread_resume(dhcpd_thread);
			dhcpd_running=1;
			dhcpd_restart(1);
		}
	}
	return 0;
}
int kill_dhcpc(void)
{
	ulink_dhcpauto_debug_printf("kill_dhcpc\n");
	if(landhcpc_started && landhcpc_running)
	{
		ulink_dhcpauto_debug_printf("dhcpc cyg_thread_suspend\n");
		cyg_thread_suspend(landhcpc_thread);
		landhcpc_running=0;
	}
	unlink(ULINK_DHCP_AUTO_CHECK_GETIP_FILE);
	return 0;
}
int run_dhcpc(void)
{
	ulink_dhcpauto_debug_printf("run_dhcpc\n");
	if(!landhcpc_started)
	{
		ulink_dhcpauto_debug_printf("dhcpd_startup\n");
		landhcpc_startup("eth0",0);
	}else
	{
		if(!landhcpc_running)
		{
			cyg_thread_resume(landhcpc_thread);
			landhcpc_running=1;
			dhcpc_reconnect(1);
		}
	}	
	return 0;
}
int haveGotIp()
{
	if(isFileExist(ULINK_DHCP_AUTO_CHECK_GETIP_FILE))
		return 1;
	else
		return 0;
}
extern int run_clicmd(char *command_buf);

int make_client_renew()
{
	char cmdbuff[128]={0};
	sprintf(cmdbuff,"ifconfig wlan0 down");
	run_clicmd(cmdbuff);
	ulink_dhcpauto_debug_printf(cmdbuff);
	ulink_dhcpauto_debug_printf("\n");
	sleep(10);
	
	sprintf(cmdbuff,"ifconfig wlan0 up");
	run_clicmd(cmdbuff);
	ulink_dhcpauto_debug_printf(cmdbuff);
	ulink_dhcpauto_debug_printf("\n");
	return 0;
}
static struct in_addr intaddr,subnet;
int save_orig_ip()
{
	getInAddr("eth0",IP_ADDR,(void*)&intaddr);
	getInAddr("eth0",SUBNET_MASK,(void*)&subnet);
	ulink_dhcpauto_debug_printf("%s:%d ipAddr=%s\n",__FUNCTION__,__LINE__,inet_ntoa(intaddr));
	//ulink_dhcpauto_debug_printf("%s:%d subnetMask=%s\n",__FUNCTION__,__LINE__,inet_ntoa(subnet));
}
int recover_orig_ip()
{
	char ipAddr[16]={0};
	char subnetMask[16]={0};
	char cmdbuff[128]={0};
	strcpy(ipAddr,inet_ntoa(intaddr));
	strcpy(subnetMask,inet_ntoa(subnet));
	ulink_dhcpauto_debug_printf("%s:%d ipAddr=%s\n",__FUNCTION__,__LINE__,ipAddr);
	ulink_dhcpauto_debug_printf("%s:%d subnetMask=%s\n",__FUNCTION__,__LINE__,subnetMask);
	sprintf(cmdbuff,"ifconfig %s %s netmask %s","eth0",ipAddr,subnetMask);
	run_clicmd(cmdbuff);
}

int ulink_dhcp_auto_main(cyg_addrword_t data)
{
	ULINK_DHCP_AUTO_STATUS status=ULINK_DHCP_AUTO_STATUS_OFFLINK;
	int linkStatus=-1;	
	int opt=-1;
	char dhcpdConfFile[64]={0};
	char iface[32]={0};

	kill_dhcpc();
	kill_dhcpd();
	
	ulink_dhcpauto_debug_printf("startup status=%s\n",print_status(status));
	run_dhcpd();
	save_orig_ip();

	while(1)
	{
	
#ifdef HAVE_SYSTEM_REINIT
			if(ulink_dhcpauto_cleaning)
			{
				ulink_dhcpauto_cleaning=0;
				break;
			}
#endif
		linkStatus=getWanLink("eth1");
		switch(status)
		{
			case ULINK_DHCP_AUTO_STATUS_OFFLINK:
				if(linkStatus>=0)
				{//on link
					status=ULINK_DHCP_AUTO_STATUS_REQIP;
					ulink_dhcpauto_debug_printf("linkStatus=%d change to %s\n",linkStatus,print_status(status));
					kill_dhcpd();					
					make_client_renew();
					run_dhcpc();
				}
				break;
			case ULINK_DHCP_AUTO_STATUS_REQIP:
				if(linkStatus<0)
				{//off link
					status=ULINK_DHCP_AUTO_STATUS_OFFLINK;
					ulink_dhcpauto_debug_printf("linkStatus=%d change to %s\n",linkStatus,print_status(status));
					kill_dhcpc();
					make_client_renew();
					run_dhcpd();
					recover_orig_ip();
				}else
				if(haveGotIp())
				{
					status=ULINK_DHCP_AUTO_STATUS_GETIP;
					ulink_dhcpauto_debug_printf("change to %s\n",print_status(status));
				}
				break;
			case ULINK_DHCP_AUTO_STATUS_GETIP:
				if(linkStatus<0)
				{//off link
					status=ULINK_DHCP_AUTO_STATUS_OFFLINK;
					ulink_dhcpauto_debug_printf("linkStatus=%d change to %s\n",linkStatus,print_status(status));
					kill_dhcpc();
					make_client_renew();
					run_dhcpd();
					recover_orig_ip();
				}
				break;
			default:
				break;
		}
		sleep(ULINK_DHCPAUTO_CHECK_INTERVAL);
	}
	
}

int ulink_dhcp_auto_startup(unsigned int argc, unsigned char *argv[])
{
	diag_printf("enter ulink_dhcp_auto_startup\n");
	if (ulink_dhcpauto_started==1)
	{
		diag_printf("ulink_dhcp_auto has already been startup\n");
		return(-1);
	}
	if (ulink_dhcpauto_running==0)
	{
		cyg_thread_create(ULINK_DHCPAUTO_THREAD_PRIORITY,
		ulink_dhcp_auto_main,
		0,
		"ulink_dhcp_auto",
		ulink_dhcpauto_stack,
		sizeof(ulink_dhcpauto_stack),
		&ulink_dhcpauto_thread,
		&ulink_dhcpauto_thread_object);
		
		diag_printf("Starting ulink_dhcp_auto thread\n");
		cyg_thread_resume(ulink_dhcpauto_thread);
		ulink_dhcpauto_started=1;
		ulink_dhcpauto_running=1;
		return(0);
	}
	else
	{
		diag_printf("ulink_dhcp_auto is already running\n");
		return(-1);
	}
}
#ifdef HAVE_SYSTEM_REINIT
void clean_ulink_dhcp_auto()
{
	if(ulink_dhcpauto_running)
	{
		ulink_dhcpauto_cleaning=1;
		while(ulink_dhcpauto_cleaning){
			cyg_thread_delay(20);
		}

		ulink_dhcpauto_running=0;
		ulink_dhcpauto_started=0;
		cyg_thread_kill(ulink_dhcpauto_thread);
		cyg_thread_delete(ulink_dhcpauto_thread);
	}
}

#endif


