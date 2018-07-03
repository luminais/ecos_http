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
#ifdef MULTI_PPPOE
#define WAN_IF_2 ("ppp0")
#define WAN_IF_2_1 ("ppp1")
#define WAN_IF_2_2 ("ppp2")
#define WAN_IF_2_3 ("ppp3")
#endif
#define PortStatusLinkSpeed10M              (0<<0)      /* 10M */
#define PortStatusLinkSpeed100M             (1<<0)      /* 100M */
#define PortStatusLinkSpeed1000M            (2<<0)      /* 1000M */
typedef enum {LINK_INIT=0, LINK_NO, LINK_DOWN, LINK_WAIT, LINK_UP}WAN_LINK;
typedef enum { DHCP_DISABLED=0, DHCP_CLIENT=1, DHCP_SERVER=2, PPPOE=3, PPTP=4, BIGPOND=5, L2TP=6, PPPOE_RU=7, PPTP_RU=8, L2TP_RU=9 } DHCP_T;
typedef enum { IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T;
extern int wan_type;
extern int wisp_interface_num;
int isConnectPPP(void);
int is_wan_connected(void);
int getWanSpeed(char *interface);
static int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{  
unsigned int args[4];  
struct ifreq ifr;  
int sockfd;  
args[0] = arg0;  
args[1] = arg1;  
args[2] = arg2;  
args[3] = arg3;  
if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)    
	{      perror("fatal error socket\n");      
		return -3;    
	}    
strcpy((char*)&ifr.ifr_name, name);  
((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;  
if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)    
	{      
	perror("device ioctl:");      
	close(sockfd);     
	return -1;    
	}  
close(sockfd);  
return 0;
} /* end re865xIoctl */
#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
#define RTL8651_IOCTL_GETWANLINKSPEED 2100
#define RTL8651_IOCTL_SETWANLINKSPEED 2101
int getIfaceInAddr( char *interface, ADDR_T type, void *pAddr )
{    
	struct ifreq ifr;    
	int skfd=0, found=0;    
	struct sockaddr_in *addr;    
	int retValue=0;    
	skfd = socket(AF_INET, SOCK_DGRAM, 0);    
	if(skfd==-1)
		return 0;
	strcpy(ifr.ifr_name, interface);    
	retValue = ioctl(skfd, SIOCGIFFLAGS, &ifr);    
	if ( retValue < 0){
			close( skfd );    
			return retValue;    
	}	
	if (type == IP_ADDR) {	
		if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {		
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);		
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);		
			found = 1;	
		}    
	}    
	close( skfd );    
	return found;
}
int isConnectPPP()
{	
	struct stat status;	
#ifdef MULTI_PPPOE
	char cur_link_file[50];
	extern int cur_session;
	if(0 == cur_session)
	{
		strcpy(cur_link_file,"/etc/ppp/link");
	}
	else if(1 == cur_session)
	{
		strcpy(cur_link_file,"/etc/ppp/link2");	
	}
	else if(2 == cur_session)
	{
		strcpy(cur_link_file,"/etc/ppp/link3");	
	}
	else if(3 == cur_session)
	{
		strcpy(cur_link_file,"/etc/ppp/link4");	
	}
	else 
	{
		strcpy(cur_link_file,"/etc/ppp/link");
	}
	if ( stat(cur_link_file, &status) < 0)		
		return 0;
#else
	if ( stat("/etc/ppp/link", &status) < 0)		
		return 0;	
#endif

	return 1;
}
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

if (wan_type == PPPOE || wan_type == PPTP || wan_type ==L2TP || wan_type ==PPPOE_RU || wan_type == PPTP_RU) {		
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
			iface= "wlan0";
	}
	
	

	if (iface && getIfaceInAddr(iface, IP_ADDR, (void *)&intaddr)){ 				
		if (wan_type == PPPOE || wan_type == PPTP || wan_type ==L2TP || wan_type ==PPPOE_RU || wan_type == PPTP_RU){			
			wanip1 = (intaddr.s_addr & 0xFF000000);			
			wanip2 = (intaddr.s_addr & 0x00FF0000);			
			wanip3 = (intaddr.s_addr & 0x0000FF00);			
			if((wanip1 == 0x0A000000) && (wanip2 == 0x00400000) && (wanip3 == 0x00004000)){				
				return(LINK_WAIT);		
			}		
		}		
		return(LINK_UP);	
	}else{		
		if (wan_type == PPPOE || wan_type == PPTP || wan_type ==L2TP || wan_type ==PPPOE_RU || wan_type == PPTP_RU){			
#ifdef MULTI_PPPOE
			if(iface != NULL)
				retValue = getIfaceInAddr(iface, IP_ADDR, (void *)&intaddr);	
			else
				retValue = getIfaceInAddr("ppp0", IP_ADDR, (void *)&intaddr);				
#else
			retValue = getIfaceInAddr("ppp0", IP_ADDR, (void *)&intaddr);	
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

/* Wan link status detect */
int getWanLink(char *interface)
{        
	unsigned int    ret;        
	unsigned int    args[0];        
	re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret) ;        

	return ret;
}
/* Wan link speed detect */
int getWanSpeed(char *interface)
{        
	unsigned int    ret;        
	unsigned int    args[0];        
	re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKSPEED, (unsigned int)(args), 0, (unsigned int)&ret) ;
	//printf("\r\n %s(%d) getWanSpeed(), ret=%d",__FUNCTION__, __LINE__, ret);//Added for test
	//syslog(LOG_INFO, "%s(%d) getWanSpeed(), ret=%d",__FUNCTION__, __LINE__, ret);//Added for test
	return ret;
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
	
	
        char dev[80];
	FILE *stream;
	long  unsigned count=0, total=0;

        stream = fopen ( "/proc/net/dev", "r" );
        if ( stream != NULL )
        {
               while ( getc ( stream ) != '\n' );
               while ( getc ( stream ) != '\n' );

               while ( !feof( stream ) )
               	 {
		 switch (i)
		   {
		   case 1:	
                        fscanf ( stream, "%[^:]:%*u %*u %*u %*u %*u %*u %*u %*u %lu %*u %*u %*u %*u %*u %*u %*u\n", dev, &count );
			break;
		   case 2:	
                        fscanf ( stream, "%[^:]:%lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", dev, &count ); 
			break;
		   case 3:	
		        fscanf ( stream, "%[^:]:%*u %*u %*u %*u %*u %*u %*u %*u %*u %lu %*u %*u %*u %*u %*u %*u\n", dev, &count );
			break;
		   case 4:	
                        fscanf ( stream, "%[^:]:%*u %lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", dev, &count );
			break;
		    }	
			
		  if(strcmp ( dev, ifname )==0)
			total += count;
				
                  }					
			fclose ( stream );
      	}
        if(i==1)
  	   data->obytes=total;
        else if(i==2)
  	   data->ibytes=total;
        else if(i==3)
  	   data->opackets=total;
        else 
  	   data->ipackets=total;
        
	return 0;
}
long 
getifstats_all(const char * ifname, struct ifdata * data)
		   {
	FILE *f;
	char line[512];
	char * p;
	int i;
	int intVal=0;
	unsigned long r = -1;
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
	f = fopen("/proc/net/dev", "r");
	if(!f) {
		syslog(LOG_ERR, "getifstats() : cannot open /proc/net/dev : %m");
		return -1;
      	}
	/* discard the two header lines */
	fgets(line, sizeof(line), f);
	fgets(line, sizeof(line), f);
	while(fgets(line, sizeof(line), f)) {
		p = line;
		while(*p==' ') p++;
		i = 0;
		while(ifname[i] == *p) {
			p++; i++;
		}
		/* TODO : how to handle aliases ? */
		if(ifname[i] || *p != ':')
			continue;
		p++;
		while(*p==' ') p++;
		data->ibytes = strtoul(p, &p, 0);
		while(*p==' ') p++;
		data->ipackets = strtoul(p, &p, 0);
		/* skip 6 columns */
		for(i=6; i>0 && *p!='\0'; i--) {
			while(*p==' ') p++;
			while(*p!=' ' && *p) p++;
		}
		while(*p==' ') p++;
		data->obytes = strtoul(p, &p, 0);
		while(*p==' ') p++;
		data->opackets = strtoul(p, &p, 0);
		r = 0;
		break;
	}
	fclose(f);
#ifdef ENABLE_GETIFSTATS_CACHING
	if(r==0 && current_time!=((time_t)-1)) {
		cache_timestamp = current_time;
		memcpy(&cache_data, data, sizeof(struct ifdata));
	}
#endif
	return r;
}

