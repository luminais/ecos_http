/* TODO */
#include "../openbsd/getifstats.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include "../../athttpd/utility.h"
#include "../../apmib/apmib.h"
#include "common.h"

#ifdef __ECOS
#include <net/if.h>			/* for IFNAMSIZ and co... */
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif
#ifdef 	ECOS_DBG_STAT
#include "../../system/sys_utility.h"
#endif
#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
#endif

#ifdef MULTI_PPPOE
#define WAN_IF_2 ("ppp0")
#define WAN_IF_2_1 ("ppp1")
#define WAN_IF_2_2 ("ppp2")
#define WAN_IF_2_3 ("ppp3")
#endif
#define PortStatusLinkSpeed10M              (0<<0)      /* 10M */
#define PortStatusLinkSpeed100M             (1<<0)      /* 100M */
#define PortStatusLinkSpeed1000M            (2<<0)      /* 1000M */
//typedef enum {LINK_INIT=0, LINK_NO, LINK_DOWN, LINK_WAIT, LINK_UP}WAN_LINK;
//typedef enum { DHCP_DISABLED=0, DHCP_CLIENT=1, DHCP_SERVER=2, PPPOE=3, PPTP=4, BIGPOND=5, L2TP=6, PPPOE_RU=7, PPTP_RU=8, L2TP_RU=9 } DHCP_T;
//typedef enum { IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T;
extern int wan_type;
extern int wisp_interface_num;
extern char wisp_if_name[16];
int is_wan_connected(void);


#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
#define RTL8651_IOCTL_GETWANLINKSPEED 2100
#define RTL8651_IOCTL_SETWANLINKSPEED 2101


int is_wan_connected(void)
{		

char *iface;	
struct in_addr intaddr;	
int retValue=0;	
int wanip1=0;	
int wanip2=0;	
int wanip3=0; 	

#ifdef MULTI_PPPOE
	char cur_iface[50];
	extern int cur_session;
	if(0 == cur_session)
	{
		strcpy(cur_iface,WAN_IF_2);
	}
	else if(1 == cur_session)
	{
		strcpy(cur_iface,WAN_IF_2_1);	
	}
	else if(2 == cur_session)
	{
		strcpy(cur_iface,WAN_IF_2_2);	
	}
	else if(3 == cur_session)
	{
		strcpy(cur_iface,WAN_IF_2_3);	
	}
	else
	{
		strcpy(cur_iface,WAN_IF_2);
	}

#endif
if(wisp_interface_num == -1){
	if (getWanLink("eth1") < 0){  // eth1 link down
		return(LINK_DOWN);	//Brad modify from LINK_WAIT to LINK_DOWN		
		}
}
if(wan_type == STATIC_IP)
	return (LINK_UP);
if (wan_type == PPPOE || wan_type == PPTP || wan_type ==L2TP ) {		
#ifdef MULTI_PPPOE
	iface = cur_iface;
#else
	iface = "ppp0";		
#endif
	if ( !isConnectPPP())			
		iface = NULL;							
	}else{		
		if(wisp_interface_num == -1)
			iface = "eth1";	
		else
			iface = wisp_if_name;//iface= "wlan0";
	}
	
	

	if (iface && getInAddr(iface, IP_ADDR, (void *)&intaddr)){ 				
		if (wan_type == PPPOE || wan_type == PPTP || wan_type ==L2TP ){			
			wanip1 = (intaddr.s_addr & 0xFF000000);			
			wanip2 = (intaddr.s_addr & 0x00FF0000);			
			wanip3 = (intaddr.s_addr & 0x0000FF00);			
			if((wanip1 == 0x0A000000) && (wanip2 == 0x00400000) && (wanip3 == 0x00004000)){				
				return(LINK_WAIT);		
			}		
		}		
		return(LINK_UP);	
	}else{		
		if (wan_type == PPPOE || wan_type == PPTP || wan_type ==L2TP ){			
#ifdef MULTI_PPPOE
			if(iface != NULL)
				retValue = getInAddr(iface, IP_ADDR, (void *)&intaddr);	
			else
				retValue = getInAddr("ppp0", IP_ADDR, (void *)&intaddr);				
#else
			retValue = getInAddr("ppp0", IP_ADDR, (void *)&intaddr);	
#endif					
			wanip1 = (intaddr.s_addr & 0xFF000000);			
			wanip2 = (intaddr.s_addr & 0x00FF0000);			
			wanip3 = (intaddr.s_addr & 0x0000FF00);			
			if((wanip1 == 0x0A000000) && (wanip2 == 0x00400000) && (wanip3 == 0x00004000)){								
				return(LINK_DOWN);			
			}		
		}else if(wan_type ==DHCP_CLIENT)						
				return(LINK_DOWN);	
		}
	return (LINK_DOWN);
}
int getWanCurrSpeed(void)
{		
	int retValue=-1; 	
	retValue = getWanSpeed("eth1");  // eth1 link speed	return retValue;	
	return retValue;
}


#if 0 //Brad disabled since un-used currently
/* Wan link speed set */
int setWanSpeed(char *interface, int speed)
{        
unsigned int    ret;        
unsigned int    args[0];				
switch(speed)				
	{					
	case 0:						
		ret = 0x00;						
		break;										
	case 10:						
		ret = 0x08;						
		break;											
	case 100:						
		ret = 0x09;						
		break;											
	case 1000:						
		ret = 0x0a;						
		break;											
	default:						
		ret = 0x00;						
		break;				
	}        
re865xIoctl(interface, RTL8651_IOCTL_SETWANLINKSPEED, (unsigned int)(args), 0, (unsigned int)&ret) ;                      
return ret;
}
#endif

long getifstats(const char * ifname, struct ifdata * data,int i)
{
	struct user_net_device_stats stats;
	if(getStats(ifname, &stats) < 0){
		return -1;
	}
       if(i==1)
  	   data->obytes= stats.tx_bytes;
        else if(i==2)
  	   data->ibytes=stats.rx_bytes;
        else if(i==3)
  	   data->opackets=stats.tx_packets;
        else 
  	   data->ipackets=stats.rx_packets;
        
	return 0;
}
long 
getifstats_all(const char * ifname, struct ifdata * data)
{
	char line[512];
	char * p;
	int i;
	int intVal=0;
	unsigned long r = -1;
	struct user_net_device_stats stats;
#ifdef ENABLE_GETIFSTATS_CACHING
	static time_t cache_timestamp = 0;
	static struct ifdata cache_data;
	time_t current_time;
#endif
	//data->baudrate = 4200000;
	//brad modify for check wan phy link20081028
	if(wisp_interface_num ==-1){
		if (getWanLink("eth1") ==0 ){  // eth1 link down/up	
			
			intVal = getWanCurrSpeed();						
			//printf("%s(%d) the link speed =%d\n", __FUNCTION__,__LINE__,intVal);//Added for test
			//syslog(LOG_INFO, "%s(%d) the link speed =%d\n", __FUNCTION__,__LINE__,intVal);//Added for test
			switch(intVal)			
			{	
			case PortStatusLinkSpeed10M:
				data->baudrate = 10000000;
				break;
			case PortStatusLinkSpeed100M:
				data->baudrate = 100000000;
				break;
			case PortStatusLinkSpeed1000M:
				data->baudrate = 1000000000;
				break;
			}
		}else{
			data->baudrate = 0;
		}
	}
	else
		data->baudrate = 0;
	
	data->opackets = 0;
	data->ipackets = 0;
	data->obytes = 0;
	data->ibytes = 0;
#ifdef ENABLE_GETIFSTATS_CACHING
	current_time = time(NULL);
	if(current_time == ((time_t)-1)) {
		syslog(LOG_ERR, "getifstats() : time() error : %m");
	} else {
		if(current_time < cache_timestamp + GETIFSTATS_CACHING_DURATION) {
			memcpy(data, &cache_data, sizeof(struct ifdata));
			return 0;
		 }	
                  }					
#endif
	
	if(getStats(ifname, &stats) < 0){
		return -1;
	}
    data->obytes= stats.tx_bytes;
    data->ibytes=stats.rx_bytes;
    data->opackets=stats.tx_packets;
    data->ipackets=stats.rx_packets;
#ifdef ENABLE_GETIFSTATS_CACHING
	if(current_time!=((time_t)-1)) {
		cache_timestamp = current_time;
		memcpy(&cache_data, data, sizeof(struct ifdata));
	}
#endif
	return 0;
}

