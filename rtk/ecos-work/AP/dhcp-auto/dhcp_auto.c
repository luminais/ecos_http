#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <network.h>


#include <sys/socket.h>


#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef __ECOS
#include <linux/if_packet.h>
#endif

#ifndef __ECOS
#include <linux/wireless.h>
#include <sys/sysinfo.h>
#else
#include <net/if.h>			/* for IFNAMSIZ and co... */
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#include <cyg/kernel/kapi.h>
#endif




#include "dhcp_auto.h"
#include "common.h"
#include "../system/sys_utility.h"
#include "../system/sys_init.h"

/*
*	dhcp auto, only work in birdge mode.
*	when eth port is off link, run dhcp server to offer ip_address 
* 	when eth port is on link, run dhcp client as a host
*	client pc use domain name to visit dut
*/

#define DHCPAUTO_THREAD_PRIORITY 16
#define DHCPAUTO_THREAD_STACK_SIZE 0x00005000
#define CYG_NET_TELNET_SERVEROPT_PORT 23

static char dhcpauto_started=0;
static char dhcpauto_running=0;
#ifdef HAVE_SYSTEM_REINIT
static char dhcpauto_cleaning=0;
#endif
static int offlinkCount=0;
cyg_uint8  dhcpauto_stack[DHCPAUTO_THREAD_STACK_SIZE];
cyg_handle_t dhcpauto_thread;
cyg_thread  dhcpauto_thread_object;


extern char dhcpd_started;
extern char dhcpd_running;
extern cyg_handle_t  dhcpd_thread;
extern char landhcpc_running;   
extern char landhcpc_started;
extern cyg_handle_t  landhcpc_thread;

int waitIPCount=0;

char * print_status(DHCP_AUTO_STATUS status)
{
	switch(status)
	{
		case DHCP_AUTO_STATUS_OFFLINK:
			return "DHCP_AUTO_STATUS_OFFLINK";
		case DHCP_AUTO_STATUS_HOLD_REQIP:
			return "DHCP_AUTO_STATUS_HOLD_REQIP";
		case DHCP_AUTO_STATUS_REQIP:
			return "DHCP_AUTO_STATUS_REQIP";
		case DHCP_AUTO_STATUS_HOLD_GETIP:
			return "DHCP_AUTO_STATUS_HOLD_GETIP";
		case DHCP_AUTO_STATUS_GETIP:
			return "DHCP_AUTO_STATUS_GETIP";
		default:
			return "DHCP_AUTO_STATUS_UNKNOWN";
	}
}



#define ETHER_ADDRLEN	6

 /* WLAN sta info structure */
 typedef struct wlan_sta_info 
{
	 unsigned short  aid;
	 unsigned char	 addr[6];
	 unsigned long	 tx_packets;
	 unsigned long	 rx_packets;
	 unsigned long	 expired_time;	 // 10 msec unit
	 unsigned short  flag;
	 unsigned char	 txOperaRates;
	 unsigned char	 rssi;
	 unsigned long	 link_time; 	 // 1 sec unit
	 unsigned long	 tx_fail;
	 unsigned long tx_bytes;
	 unsigned long rx_bytes;
	 unsigned char network;
	 unsigned char ht_info;  // bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	 unsigned char	 resv[6];
 } WLAN_STA_INFO_T, *WLAN_STA_INFO_Tp;

 typedef struct _DOT11_DISCONNECT_REQ
 {
        unsigned char   EventId;
        unsigned char   IsMoreEvent;
        unsigned short  Reason;
        char            MACAddr[ETHER_ADDRLEN];
}DOT11_DISCONNECT_REQ;


#define DOT11_EVENT_DISCONNECT_REQ  9
#define SIOCGIWIND      			0x89ff


#define MAX_STA_NUM 		64	// max support sta number

 extern int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo );

 //extern int IssueDisconnect(unsigned char *interfacename, unsigned char *pucMacAddr, unsigned short reason);

static int IssueDisconnectClient(unsigned char *interfacename, unsigned char *pucMacAddr, unsigned short reason)
{
	int skfd;
	int retVal = 0;
	struct iwreq wrq;
	DOT11_DISCONNECT_REQ Disconnect_Req;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		diag_printf("socket() error!\n");
		return -1;
	}

	Disconnect_Req.EventId = DOT11_EVENT_DISCONNECT_REQ;
	Disconnect_Req.IsMoreEvent = 0;
	Disconnect_Req.Reason = reason;
	memcpy(Disconnect_Req.MACAddr,  pucMacAddr, ETHER_ADDRLEN);

	strcpy((char *)wrq.ifr_name, (char *)interfacename);	

	wrq.u.data.pointer = (caddr_t)&Disconnect_Req;
	wrq.u.data.length = sizeof(DOT11_DISCONNECT_REQ);


	if(ioctl(skfd, SIOCGIWIND, &wrq) < 0)
	{
		diag_printf("\n%s:%d issues disassociation : ioctl error!\n", __FUNCTION__,__LINE__);
          	retVal = -1;
	}

	close(skfd);
   	return retVal;

}
 void disconnectAssociateClient()
{
	int i;
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
	char wlanIfname[16]={0};
	strcpy(wlanIfname, "wlan0");
	unsigned char zore_mac[]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 ) 
	{
		printf("Allocate buffer failed!\n");
		return ;
	}	
	
	if ( getWlStaInfo(wlanIfname,  (WLAN_STA_INFO_Tp)buff ) < 0 ) 
	{
		printf("Read wlan sta info failed!\n");
		free(buff);
		return ;
	}

	for (i=1; i<=MAX_STA_NUM; i++)
	{
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];

		if(memcmp(pInfo->addr, zore_mac, 6)==0)
			continue;
		
		diag_printf("\n%s:%d disconnect client:%02x:%02x:%02x:%02x:%02x:%02x\n",__FUNCTION__,__LINE__,
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5]);
		
		IssueDisconnectClient(wlanIfname, pInfo->addr, 5);			
	}

	free(buff);
}

int ducpauto_kill_dhcpd(void)
{
	if(dhcpd_started && dhcpd_running)
	{
	dhcpauto_debug_printf("kill dhcpd\n");
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
	dhcpauto_debug_printf("run_dhcpd\n");
	if(!dhcpd_started)
	{
		dhcpauto_debug_printf("dhcpd_startup\n");
		dhcpd_startup(0);
	}else
	{
		if(!dhcpd_running)
		{
			dhcpauto_debug_printf("dhcpd cyg_thread_resume\n");
			cyg_thread_resume(dhcpd_thread);
			dhcpd_running=1;
			dhcpd_restart(1);
		}
	}
	dhcpauto_debug_printf("run_dhcpd end\n");
	return 0;
}
int ducpauto_kill_dhcpc(void)
{
	dhcpauto_debug_printf("ducpauto_kill_dhcpc\n");
	if(landhcpc_started && landhcpc_running)
	{
		dhcpauto_debug_printf("dhcpc cyg_thread_suspend\n");
		cyg_thread_suspend(landhcpc_thread);
		landhcpc_running=0;
	}
	unlink(DHCP_AUTO_CHECK_GETIP_FILE);
	return 0;
}
int run_dhcpc(void)
{
	dhcpauto_debug_printf("run_dhcpc\n");
	if(!landhcpc_started)
	{
		dhcpauto_debug_printf("dhcpc_startup\n");
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
	if(isFileExist(DHCP_AUTO_CHECK_GETIP_FILE))
		unlink(DHCP_AUTO_CHECK_GETIP_FILE);
	dhcpauto_debug_printf("run_dhcpc end\n");
	return 0;
}
int haveGotIp()
{
	if(isFileExist(DHCP_AUTO_CHECK_GETIP_FILE))
		return 1;
	else
		return 0;
}
extern int run_clicmd(char *command_buf);
extern void get_wlan_mode(int *wlan_mode);

int make_client_renew()
{
	int wlan_mode;
	char cmdbuff[128]={0};
	get_wlan_mode(&wlan_mode);

	//RunSystemCmd(NULL_FILE, "ifconfig", "eth0", "down", NULL_STR);
#ifdef CONFIG_RTL_PHY_POWER_CTRL
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "0", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "1", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "2", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "3", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "4", "off", NULL_STR);

	sleep(3);

	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "0", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "1", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "2", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "3", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "4", "on", NULL_STR);
#endif
	//RunSystemCmd(NULL_FILE, "ifconfig", "eth0", "up", NULL_STR);

	if(wlan_mode==1)
		return 0;

#if 0
	sprintf(cmdbuff,"ifconfig wlan0 down");
	dhcpauto_debug_printf(cmdbuff);
	dhcpauto_debug_printf("\n");
	run_clicmd(cmdbuff);
	
	sleep(10);
	
	sprintf(cmdbuff,"ifconfig wlan0 up");
	dhcpauto_debug_printf(cmdbuff);
	dhcpauto_debug_printf("\n");
	run_clicmd(cmdbuff);
#endif	

	disconnectAssociateClient();

	return 0;
}
static struct in_addr intaddr={0},subnet={0};
int save_orig_ip()
{
	if(intaddr.s_addr==0)
		getInAddr("eth0",IP_ADDR,(void*)&intaddr);
	if(subnet.s_addr==0)
		getInAddr("eth0",SUBNET_MASK,(void*)&subnet);
	dhcpauto_debug_printf("%s:%d ipAddr=%s\n",__FUNCTION__,__LINE__,inet_ntoa(intaddr));
	//dhcpauto_debug_printf("%s:%d subnetMask=%s\n",__FUNCTION__,__LINE__,inet_ntoa(subnet));
}
int recover_orig_ip()
{
	char ipAddr[16]={0};
	char subnetMask[16]={0};
	char cmdbuff[128]={0};
	strcpy(ipAddr,inet_ntoa(intaddr));
	strcpy(subnetMask,inet_ntoa(subnet));
	dhcpauto_debug_printf("%s:%d ipAddr=%s\n",__FUNCTION__,__LINE__,ipAddr);
	dhcpauto_debug_printf("%s:%d subnetMask=%s\n",__FUNCTION__,__LINE__,subnetMask);
	sprintf(cmdbuff,"ifconfig %s %s netmask %s","eth0",ipAddr,subnetMask);
	run_clicmd(cmdbuff);
}

void return_to_offlink(int *status)
{
	*status=DHCP_AUTO_STATUS_OFFLINK;
	dhcpauto_debug_printf("change to %s\n",print_status(*status));
	ducpauto_kill_dhcpc();
	make_client_renew();
	recover_orig_ip();
	run_dhcpd();	
	offlinkCount=0;
}

void set_waitIPCount_value(int value)
{
	waitIPCount=value;
}

int get_waitIPCount_value()
{
	return waitIPCount;
}


int dhcp_auto_main(cyg_addrword_t data)
{
	DHCP_AUTO_STATUS status=DHCP_AUTO_STATUS_OFFLINK;
	int linkStatus=-1;	
	int opt=-1;
	char dhcpdConfFile[64]={0};
	char iface[32]={0};
	int holdCount=0;
	//int waitIPCount=0;

	if(data!=NULL)
		strcpy(iface,(char*)data);
	ducpauto_kill_dhcpc();
	ducpauto_kill_dhcpd();
	
	run_dhcpd();
	save_orig_ip();

	while(1)
	{
//		linkStatus=getWanLink("eth1");
#ifdef HAVE_SYSTEM_REINIT
		if(dhcpauto_cleaning)
		{
			dhcpauto_cleaning=0;
			break;
		}
#endif
		/*
		if(siteSurveyStatus==SITE_SURVEY_STATUS_RUNNING) 
		{
			sleep(DHCPAUTO_CHECK_INTERVAL);
			continue;
		}*/
		if(data==NULL)
		{
			linkStatus=getWanLink("eth0");
			//dhcpauto_debug_printf("%s:%d %d\n",__FUNCTION__,__LINE__,linkStatus);
			if(linkStatus<0)
				linkStatus=getWlanLink("wlan0-vxd0");
			if(linkStatus<0)
				linkStatus=getWlanLink("wlan0");
			//dhcpauto_debug_printf("%s:%d %d\n",__FUNCTION__,__LINE__,linkStatus);
		}
		else if(strcmp(iface,"eth0")==0)
			linkStatus=getWanLink("eth0");
		else if(strcmp(iface,"wlan0-vxd0")==0)
			linkStatus=getWlanLink("wlan0-vxd0");
		else
			linkStatus=getWlanLink("wlan0");
		//printf("*******%s %d: status=%d %d %d******\n",__func__,__LINE__,status,waitIPCount,holdCount);
	
		if(waitIPCount > WAIT_IP_COUNT_MAX)
			linkStatus = -1;

		
		//diag_printf("*******%s %d: status=%d %d %d******\n",__FUNCTION__,__LINE__,status,waitIPCount,holdCount);
		
		if(linkStatus>=0){
			//dhcpauto_debug_printf("wlan0-vxd0 link on\n",__FUNCTION__,__LINE__,linkStatus);
		}
		else{			
			//dhcpauto_debug_printf("wlan0-vxd0 link off\n",__FUNCTION__,__LINE__,linkStatus);
		}
		switch(status)
		{
			case DHCP_AUTO_STATUS_OFFLINK:
				if(linkStatus>=0)
				{//on link
					status=DHCP_AUTO_STATUS_REQIP;
					waitIPCount = 0;
					dhcpauto_debug_printf("linkStatus=%d change to %s\n",linkStatus,print_status(status));
					ducpauto_kill_dhcpd();					
					
					run_dhcpc();
				}
				break;
			
			case DHCP_AUTO_STATUS_REQIP:
				if(linkStatus<0)
				{//off link
					if(++offlinkCount<OFF_LINK_COUNT_MAX)
						return_to_offlink(&status);
				}else{
					if(haveGotIp())
					{
						status=DHCP_AUTO_STATUS_HOLD_GETIP;
						make_client_renew();
						holdCount=0;
						dhcpauto_debug_printf("change to %s\n",print_status(status));
					}
					else
					{	
					
#if 1
						if(waitIPCount++ >= WAIT_IP_COUNT_MAX) 
						{
							return_to_offlink(&status);
							break;
						}
#endif
					}
				}
				break;
			
			case DHCP_AUTO_STATUS_HOLD_GETIP:
				if(linkStatus<0||!haveGotIp())
				{
					holdCount++;
					if(holdCount>=HOLD_COUNT_MAX)
					{//fail
						return_to_offlink(&status);
						break;
					}
				}else
				{
					status=DHCP_AUTO_STATUS_GETIP;
					dhcpauto_debug_printf("change to %s\n",print_status(status));
				
#ifdef CONFIG_ECOS_AP_SUPPORT
					kick_reinit_m(SYS_AP_LAN_M);
#endif
				}
				break;
			case DHCP_AUTO_STATUS_GETIP:
				//if(linkStatus<0)
				if(linkStatus<0||!haveGotIp())
				{//off link
					if(++offlinkCount<OFF_LINK_COUNT_MAX)
						return_to_offlink(&status);
					
				}
				break;
			default:
				break;
		}
		sleep(DHCPAUTO_CHECK_INTERVAL);
	}
	
}

int dhcp_auto_startup(unsigned int argc, unsigned char *argv[])
{
	diag_printf("enter dhcp_auto_startup\n");
	if (dhcpauto_started==1)
	{
		diag_printf("dhcp_auto has already been startup\n");
		return(-1);
	}
	//if argv[0] is eth1 or wlan0-vxd0 mean use the interface to connect dhcp server 
	//if  argv[0] is null mean use eth1 or wlan0-vxd0 to connect dhcp server 
	if(argv[0]!=NULL && strcmp(argv[0],"eth0")!=0 && strcmp(argv[0],"wlan0-vxd0")!=0 && strcmp(argv[0],"wlan0")!=0)
	{
		diag_printf("dhcp auto must have client interface eth1/wlan0-vxd0 or no interface!\n");
		return -1;
	}
	if (dhcpauto_running==0)
	{
		cyg_thread_create(DHCPAUTO_THREAD_PRIORITY,
		dhcp_auto_main,
		argv[0],
		"dhcp_auto",
		dhcpauto_stack,
		sizeof(dhcpauto_stack),
		&dhcpauto_thread,
		&dhcpauto_thread_object);
		
		diag_printf("Starting dhcp_auto thread\n");
		cyg_thread_resume(dhcpauto_thread);
		dhcpauto_started=1;
		dhcpauto_running=1;
		return(0);
	}
	else
	{
		diag_printf("dhcp_auto is already running\n");
		return(-1);
	}
}

#ifdef HAVE_SYSTEM_REINIT
void clean_dhcp_auto()
{
	if(dhcpauto_running)
	{
		dhcpauto_cleaning=1;
		while(dhcpauto_cleaning){
			cyg_thread_delay(20);
		}
		dhcpauto_running=0;
		dhcpauto_started=0;
		cyg_thread_kill(dhcpauto_thread);
		cyg_thread_delete(dhcpauto_thread);
	}
}

#endif


