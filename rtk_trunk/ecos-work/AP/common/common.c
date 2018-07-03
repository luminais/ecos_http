/*
 *	Common functions to do common works, all AP modules may call them
 *	Author: sen_liu
 *	2012/06/20
 */

	 /*-- Local include files --*/
#include "apmib.h"
#include "common.h"

#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
//#include "cyg/io/eth/rltk/819x/wlan/ieee802_mib.h"
#endif

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <time.h>

#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <stdio.h>                     // sprintf().
#include <stdlib.h>
#include <stdarg.h>

#ifdef CONFIG_NET_STACK_FREEBSD
#include <net/if_var.h>
#else
#include <net/if.h>
#endif
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <net/if_bridge.h>

#ifdef HAVE_LZMA
#include "LzmaDecode.h"
#endif

#if 0
/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}


/////////////////////////////////////////////////////////////////////////////
int getWlStaNum( char *interface, int *num )
{
    int skfd=0;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
      return -1;
	}
    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTANUM, &wrq) < 0){
    	 close( skfd );
	return -1;
	}
    *num  = (int)staNum;

    close( skfd );
    return 0;
}
#endif
int getWlanNum()
{
#if 0
	int i=0,num=0;
	int wlanNum=0;
	char interface[10]={0};
	for(i=0;i<NUM_WLAN_INTERFACE;i++)
	{
		sprintf(interface, "wlan%d", i);
		if(getWlStaNum(interface, &num) == 0)
			wlanNum++;
	}
	return wlanNum;
#else
#if defined(CONFIG_RTL_92D_SUPPORT) || defined (CONFIG_RTL_8881A_SELECTIVE)
	int val=0;
	if(!apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void*)&val))
		return -1;
	if(val==BANDMODESINGLE)
		return 1;
	if(val==BANDMODEBOTH)
		return 2;
	return 2;
#else
	#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	return 1;
	#else
	return NUM_WLAN_INTERFACE;
	#endif
#endif
#endif
}

/////////////////////////////////////////////////////////////////////////////
int getInAddr( char *interface, ADDR_T type, void *pAddr )
{
    struct ifreq ifr;
    int skfd=0, found=0;
    struct sockaddr_in *addr;


    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
	{
		return 0;
	}
		
    strcpy(ifr.ifr_name, interface);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0){
    	close( skfd );
		return (0);
	}
    if (type == HW_ADDR) {
    	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) {
		memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
		found = 1;
	}
    }
    else if (type == IP_ADDR) {
	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }
#ifdef HAVE_TR069
    else if (type == DST_IP_ADDR) {
        if (ioctl(skfd,SIOCGIFDSTADDR, &ifr) == 0) { 
                addr = ((struct sockaddr_in *)&ifr.ifr_addr);
                *((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
                found = 1; 
        }    
    }
#endif
    else if (type == SUBNET_MASK) {
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }
    close( skfd );
    return found;

}
/*      IOCTL system call */
int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
  unsigned int args[4];
  struct ifreq ifr;
  int sockfd;

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("fatal error socket\n");
	  diag_printf("%s:%d\n",__FILE__,__LINE__);
      return -3;
    }
  
  strcpy((char*)&ifr.ifr_name, name);
  ((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;
#ifdef HAVE_NOWIFI
//this IOCTL defined at wrapper/v3.0/include/wireless.h, but ethernet will use it
#define SIOCDEVPRIVATE	0x89F0	/* to 89FF */
#endif

  if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
    {
      /* 1)enable upnp, 2)chariot/bt test cause console busy, upnp will print these error */
      //perror("device ioctl:");
      close(sockfd);
	  //diag_printf("%s:%d \n",__FILE__,__LINE__);
      return -1;
    }
  close(sockfd);
  return 0;
}
/* Wan link status detect */
int getWanLink(char *interface)
{
        unsigned int    ret;
        unsigned int    args[0];
//diag_printf("%s:%d interface=%s\n",__FILE__,__LINE__,interface);
        re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret) ;
//diag_printf("%s:%d ret=%d\n",__FILE__,__LINE__,ret);
        return ret;
}

int getWanSpeed(char *interface)
{        
	unsigned int    ret;        
	unsigned int    args[0];        
	re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKSPEED, (unsigned int)(args), 0, (unsigned int)&ret) ;
	//printf("\r\n %s(%d) getWanSpeed(), ret=%d",__FUNCTION__, __LINE__, ret);//Added for test
	//syslog(LOG_INFO, "%s(%d) getWanSpeed(), ret=%d",__FUNCTION__, __LINE__, ret);//Added for test
	return ret;
}

#if defined(HAVE_TR069)
int setWanSpeed(char *interface,int speed)
{        
	unsigned int    ret;
    unsigned int    args[1] = {0};
	switch(speed){
		case 10:
			ret = 4;
			break;
		case 100:
			ret = 5;
			break;
		case 1000:
			ret = 6;
			break;
		default:
			ret = 0;
			break;
	}
	//printf("speed arg3=0x%x, interface=%s\n", ret, interface);
    re865xIoctl(interface, RTL8651_IOCTL_SETWANLINKSPEED, (unsigned int)(args), 4, (unsigned int)&ret) ; 
    return ret;
}

int getLanSpeed(unsigned int port_index)
{        
	unsigned int    ret = port_index;        
	unsigned int    args[2]={0};        
	args[1] = port_index;
	re865xIoctl("eth0", RTL8651_IOCTL_GETLANLINKSPEED, (unsigned int)(args[0]),  (unsigned int)(args[1]), (unsigned int)&ret) ;
	//printf("\r\n %s(%d) getLanSpeed(), speed=%d\n",__FUNCTION__, __LINE__, ret);//Added for test
	//syslog(LOG_INFO, "%s(%d) getWanSpeed(), ret=%d",__FUNCTION__, __LINE__, ret);//Added for test
	
	return ret;
}

int setLanSpeed(unsigned int port_index, unsigned int speed)
{
	printf("<%s:%d>port number=%d, speed=%d\n", __FUNCTION__,__LINE__,port_index, speed);
	unsigned int    ret;
    unsigned int    args[2] = {0};
	switch(speed){
		case 10:
			ret = 4;
			break;
		case 100:
			ret = 5;
			break;
		case 1000:
			ret = 6;
			break;
		default:
			ret = 0;
			break;
	}
	args[1] = port_index;
	//printf("speed speed=0x%x\n", ret);
    re865xIoctl("eth0", RTL8651_IOCTL_SETLANLINKSPEED, (unsigned int)(args), (unsigned int)(args[1]), (unsigned int)&ret) ; 
    return 1;
}

int getLanDuplex(unsigned int port_index)
{
	//printf("<%s:%d>\n", __FUNCTION__, __LINE__);
    unsigned int    ret = port_index;
    unsigned int    args[2] = {0};
	args[1] = port_index;

    re865xIoctl("eth0", RTL8651_IOCTL_GETETHERLINKDUPLEX, (unsigned int)(args[0]),(unsigned int)(args[1]), (unsigned int)&ret);

	return ret;
}

int setLanDuplex(unsigned int port_index, unsigned int mode)
{
    unsigned int    ret = mode;
    unsigned int    args[2] = {0};
	args[1] = port_index;

    re865xIoctl("eth0", RTL8651_IOCTL_SETETHERLINKDUPLEX, (unsigned int)(args[0]),(unsigned int)(args[1]), (unsigned int)&ret);
	return 1;
}

int getWanDuplex(void)
{
	printf("<%s:%d>\n", __FUNCTION__, __LINE__);
    unsigned int    ret = 4;
    unsigned int    args =0;

    re865xIoctl("eth1", RTL8651_IOCTL_GETETHERLINKDUPLEX, (unsigned int)(args), 0, (unsigned int)&ret);
	//printf("<%s:%d>ret=%d\n", __FUNCTION__,__LINE__,ret);
    if(ret==0)
    	return 0;
    else 
    	return 1;
}

int setWanDuplex(unsigned int mode)
{
	//printf("<%s:%d>\n", __FUNCTION__, __LINE__);
    unsigned int    ret = mode; //0 for half, 1 for full
    unsigned int    args= 0;

    re865xIoctl("eth1", RTL8651_IOCTL_SETETHERLINKDUPLEX, (unsigned int)(args), 4, (unsigned int)&ret);
	return 1;
}
#endif
int isConnectPPP(void)
{
	struct in_addr intaddr;
	struct stat status;
	if ( stat("/etc/ppp_link", &status) < 0)
		return 0;

//	return 1;
#if 1
	if( getInAddr("ppp0", IP_ADDR, (void *)&intaddr ) )
	{
		if(intaddr.s_addr!=0)
			return 1;
	}
	return 0;
#endif 	
}

//only convent tm type to time_t
time_t mktime_rewrite( const struct tm* tm_time )
{
	Cyg_libc_time_dst states;
	time_t stdOffset={0},dstOffset={0},addVal={0};
	if(!tm_time) return -1;
	states=cyg_libc_time_getzoneoffsets(&stdOffset,&dstOffset);
	//diag_printf("%s:%d stdOffset=%d dstOffset=%d\n",__FILE__,__LINE__,stdOffset,dstOffset);
	addVal=stdOffset;
#ifndef DAYLIGHT_SAVING_TIME
	if(states==CYG_LIBC_TIME_DSTON)
		addVal=dstOffset;
#endif
	return mktime(tm_time)-addVal;
}

//////////////////////////////////////////////////////////////////////
 char *get_token(char *data, char *token)
{
	char *ptr=data;
	int len=0, idx=0;

	while (*ptr && *ptr != '\n' ) {
		if (*ptr == '=') {
			if (len <= 1)
				return NULL;
			memcpy(token, data, len);

			/* delete ending space */
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx] !=  ' ')
					break;
			}
			token[idx+1] = '\0';
			
			return ptr+1;
		}
		len++;
		ptr++;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////
 int get_value(char *data, char *value)
{
	char *ptr=data;	
	int len=0, idx, i;

	while (*ptr && *ptr != '\n' && *ptr != '\r') {
		len++;
		ptr++;
	}

	/* delete leading space */
	idx = 0;
	while (len-idx > 0) {
		if (data[idx] != ' ') 
			break;	
		idx++;
	}
	len -= idx;

	/* delete bracing '"' */
	if (data[idx] == '"') {
		for (i=idx+len-1; i>idx; i--) {
			if (data[i] == '"') {
				idx++;
				len = i - idx;
			}
			break;
		}
	}

	if (len > 0) {
		memcpy(value, &data[idx], len);
		value[len] = '\0';
	}
	return len;
}
 unsigned char convert_atob(char *data, int base)
{
	char tmpbuf[10];
	int bin;

	memcpy(tmpbuf, data, 2);
	tmpbuf[2]='\0';
	if (base == 16)
		sscanf(tmpbuf, "%02x", &bin);
	else
		sscanf(tmpbuf, "%02d", &bin);
	return((unsigned char)bin);
}

int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}
int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

int is_zero_ether_addr(const unsigned char *a)
{
	return !(a[0] | a[1] | a[2] | a[3] | a[4] | a[5]);
}

int getShellOutput(char* shellCmd, char *outputBuf, int outBufLen)
{
	FILE *fileTmp=NULL;
	int readSize=0;

	if(!shellCmd || !outputBuf)
	{
		fprintf(stderr,"input invalid!\n");
		return -1;
	}

	//write output to file
	extern int run_clicmd(char *command_buf);
	fileTmp=stdout;
	if((stdout=fopen(_PATH_TMP_LOG,"w"))==NULL)
	{
		fprintf(stderr,"redirect output fail!\n");
		stdout=fileTmp;
		return -1;
	}
	
	run_clicmd(shellCmd);
	
	fclose(stdout);
	stdout=fileTmp;

	//filter
	if((fileTmp=fopen(_PATH_TMP_LOG,"r"))==NULL)
	{
		diag_printf("open output file(%s) fail!\n",_PATH_TMP_LOG);
		return -1;
	}
	//printf("%s:%d outputBuf=%p outBufLen=%d fileTmp=%p\n",__FUNCTION__,__LINE__,outputBuf,outBufLen,fileTmp);
	//return 0;
	readSize=fread(outputBuf, 1, outBufLen-1, fileTmp);
	//return 0;
	fclose(fileTmp);
	return readSize;
}

int getShellOutput_telnetd(char* shellCmd,char *outputBuf,int outBufLen)
{
	FILE *fileTmp=NULL;
	FILE *fileTmp0=NULL;
	int readSize=0;
	if(!shellCmd||!outputBuf)
	{
		fprintf(stderr,"input invalid!\n");
		return -1;
	}
//write output to file
	extern int run_clicmd(char *command_buf);
	fileTmp=stdout;
	fileTmp0=stderr;
	if((stdout=fopen(_PATH_TMP_LOG,"w"))==NULL)
	{
		fprintf(stderr,"redirect output fail!\n");
		stdout=fileTmp;
		return -1;
	}		
	if((stderr=fopen(_PATH_TMP_LOG0,"w"))==NULL)
    {
        fprintf(fileTmp0,"redirect output fail!\n");
        stderr=fileTmp0;
        return -1;
    }		

	run_clicmd(shellCmd);
	fclose(stdout);
	fclose(stderr);
	stdout=fileTmp;
	stderr=fileTmp0;

//filter
	if((fileTmp=fopen(_PATH_TMP_LOG,"r"))==NULL)
	{
		diag_printf("open output file fail!%s\n",_PATH_TMP_LOG);
		return -1;
	}
	//printf("%s:%d outputBuf=%p outBufLen=%d fileTmp=%p\n",__FUNCTION__,__LINE__,outputBuf,outBufLen,fileTmp);
	//return 0;
	readSize=fread(outputBuf,1,outBufLen-1,fileTmp);
	//return 0;
	fclose(fileTmp);
	return readSize;
}
#ifdef HAVE_LZMA
int lzma_decode_to_buf(char *in, int in_len, char *out, int out_len)
{	
	unsigned char *startBuf= (unsigned char *)in;
	unsigned char *outBuf = (unsigned char *)out;
	unsigned int inLen=in_len; 

	SizeT compressedSize;
	UInt32 outSize = 0;
	UInt32 outSizeHigh = 0; 
	SizeT outSizeFull;
	int res;
	SizeT inProcessed;
	SizeT outProcessed;
	char tmpbuf[100];
	CLzmaDecoderState state;  /* it's about 24-80 bytes structure, if int is 32-bit */
	unsigned char properties[LZMA_PROPERTIES_SIZE];
	
	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	//unsigned long pending_len = *((unsigned long *)in);
	//startBuf += 8;
	//inLen -= (8+pending_len);

	compressedSize = (SizeT)(inLen - (LZMA_PROPERTIES_SIZE + 8));

	memcpy(properties, startBuf, sizeof(properties));
	startBuf += sizeof(properties);

	memcpy((char *)&outSize, startBuf, 4);
	outSize=le32_to_cpu(outSize);

	memcpy((char *)&outSizeHigh, startBuf+4, 4);	
	outSizeHigh = le32_to_cpu(outSizeHigh);

	outSizeFull = (SizeT)outSize;
	if (outSizeHigh != 0 || (UInt32)(SizeT)outSize != outSize) {
		puts("LZMA: Too big uncompressed stream\n");
		return -1;
	}	
	startBuf += 8;	  
	
	/* Decode LZMA properties and allocate memory */  
	if (LzmaDecodeProperties(&state.Properties, properties, LZMA_PROPERTIES_SIZE) != LZMA_RESULT_OK) {
		puts("LZMA: Incorrect stream properties\n");
		return -1;
	}
	state.Probs = (CProb *)malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));

	res = LzmaDecode(&state, startBuf, compressedSize, &inProcessed,
						  outBuf, outSizeFull, &outProcessed);	
	if(state.Probs){
		free(state.Probs);
		state.Probs = NULL;
	}
	if (res != 0) {
#ifndef __DO_QUIET__
	  sprintf(tmpbuf, "LZMA: Decoding error = %d\n", res);
#else
	  memcpy(tmpbuf, "LZMA: Decoding error =  ", 23);
	  memcpy((tmpbuf+23), &res, 4);
#endif
	  puts(tmpbuf);
	  return -1;
	}

	if(outProcessed  > out_len )
	{
		puts("LZMA:over size\n");
	}
	return 0;
}
#endif
//#if	1//def HOME_GATEWAY
#ifdef 	HOME_GATEWAY
int getDefaultRoute(char *interface, char *route,DHCP_T dhcp)
{	
	if(!interface || !route)
	{
		fprintf(stderr,"invalid input!!\n");
		return 0;
	}
	switch(dhcp)
	{
		case STATIC_IP:
			{
				struct in_addr	intaddr;
				if(!apmib_get(MIB_WAN_DEFAULT_GATEWAY,(void*)&intaddr))
					return 0;
				else 
				{
					sprintf(route,"%s",inet_ntoa(intaddr));
					return 1;
				}
			}
		case DHCP_CLIENT:
		case PPPOE:
		case PPTP:
		case L2TP:   	
			{
				/*
				char *buffer=(char*)malloc(ROUTER_BUFFER_SIZE);				
				bzero(buffer,ROUTER_BUFFER_SIZE);

				if(getShellOutput("show route",buffer,ROUTER_BUFFER_SIZE)>0)
				{
					char * tmpStr=buffer;
					char * token=NULL;
					char dst[16]={0};
					char gw[16]={0};
					char msk[16]={0};
					char flgs[9]={0};
					char iface[12]={0};
					tmpStr+=strlen("Routing tables\n")+1;
					tmpStr+=strlen("Destination     Gateway         Mask            Flags    Interface\n")+1;
					if(token=strstr(tmpStr,"Interface statistics"))
						token[0]='\0';
					else
						buffer[ROUTER_BUFFER_SIZE-1]='\0';
					//diag_printf("%s:%d tmpStr=%s\n",__FILE__,__LINE__,tmpStr);	
					token=strtok(tmpStr,"\n");
					while(token!=NULL)
					{
						sscanf(token,"%15s %15s %15s %8s %12s ",&dst,&gw,&msk,&flgs,&iface);
						printf("token is %s\n", token);
						sleep(1);
						if(memcmp(iface,interface, strlen(interface))==0 && strchr(flgs,'U')&&strchr(flgs,'G'))
						{//get default gw
							strcpy(route,gw);
							free(buffer);
							diag_printf("%s:%d gw=%s\n",__FILE__,__LINE__,gw);	
							return 1;
						}
						token=strtok(NULL,"\n");
					}
					free(buffer);
					return 0;
				}
				*/
				FILE *fp;	
				char *buffer;				
				char *name;
				char *value;
				fp=fopen("/etc/wan_info","r");		
				if(fp != NULL)
				{
					buffer=(char*)malloc(256);
					bzero(buffer,256);
					while(fgets(buffer,256,fp) !=NULL)
					{
						
						name=strtok(buffer,":");
						value=strtok(NULL,":");
						/*overrid \n*/
						value[strlen(value)-1]='\0';
						if(!strcmp("gateway",name))
						{
							strcpy(route, value);
							fclose(fp);
							free(buffer);
							return 1;
						}
					}
				}
				else
				{
					//printf("%s:%d get cmd output fail!\n",__FILE__,__LINE__);
					free(buffer);
					return 0;
				}
			}
			break;		
		default:
			return 0;
	}
}

int getWanInfo(char *pWanIP, char *pWanMask, char *pWanDefIP, char *pWanHWAddr)
{
	DHCP_T dhcp;
	OPMODE_T opmode=-1;
	WLAN_MODE_T wlan_mode = -1;
	char *iface=NULL;
	struct in_addr	intaddr;
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	int isWanPhyLink = 0;
	
	//fix the issue of when change wan type from pppoe to dhcp, the wan interface is still ppp0, not eth1 (amy reported, realsil can not reproduce)
	static char eth_ifname[8]="eth1";
	static char ppp_ifname[8]="ppp0";
	
	int wisp_wan_id=0;
	char tmpbuf[16];
	
	if ( !apmib_get( MIB_WAN_DHCP, (void *)&dhcp) )
		return -1;
  
  	if ( !apmib_get( MIB_OP_MODE, (void *)&opmode) )
		return -1;	
	
	if ( dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP || dhcp == USB3G ) { /* # keith: add l2tp support. 20080515 */
		//iface = "ppp0";
		iface=ppp_ifname;
		if ( !isConnectPPP() )
			iface = NULL;
	}
	else if (opmode == WISP_MODE){		
		apmib_get(MIB_WISP_WAN_ID,(void *)&wisp_wan_id);
#ifdef CONFIG_WLANIDX_MUTEX
		int s = apmib_save_idx();
#else
		apmib_save_idx();
#endif
		apmib_set_wlanidx(wisp_wan_id);
		
		apmib_get(MIB_WLAN_MODE,(void *)&wlan_mode);
		
		if(wlan_mode == CLIENT_MODE)			
			sprintf(tmpbuf, "wlan%d", wisp_wan_id);
		else			
			sprintf(tmpbuf, "wlan%d-vxd0", wisp_wan_id);

		iface=tmpbuf;

#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif
	}
	else
		//iface = "eth1";
		iface=eth_ifname;
	
	if(opmode != WISP_MODE)
	{
		if(iface){
			//if(getWanLink("eth1") < 0){
			if((isWanPhyLink = getWanLink(eth_ifname)) < 0){
				if(pWanIP)
					sprintf(pWanIP,"%s","0.0.0.0");
			}
		}	
	}
	
	if(pWanIP)
	{
		if ( iface && getInAddr(iface, IP_ADDR, (void *)&intaddr ) && isWanPhyLink >=0)
			sprintf(pWanIP,"%s",inet_ntoa(intaddr));
		else
			sprintf(pWanIP,"%s","0.0.0.0");
	}
	if(pWanMask)
	{
		if ( iface && getInAddr(iface, SUBNET_MASK, (void *)&intaddr ) && isWanPhyLink >=0)
			sprintf(pWanMask,"%s",inet_ntoa(intaddr));
		else
			sprintf(pWanMask,"%s","0.0.0.0");
	}
	if(pWanDefIP)
	{
		//if(opmode != WISP_MODE && getWanLink("eth1") < 0)
		if(opmode != WISP_MODE && getWanLink(eth_ifname) < 0)
		{
			sprintf(pWanDefIP,"%s","0.0.0.0");
		}
		else if( iface && getInAddr(iface, IP_ADDR, (void *)&intaddr ) )
		{
			if ( !iface || !getDefaultRoute(iface, pWanDefIP,dhcp) )
			{
				sprintf(pWanDefIP,"%s","0.0.0.0");
			}
		}
		else
			sprintf(pWanDefIP,"%s","0.0.0.0");
	}
	//To get wan hw addr
	if(opmode == WISP_MODE){
		//iface = tmpbuf;		
		apmib_get(MIB_WISP_WAN_ID,(void *)&wisp_wan_id);
#ifdef CONFIG_WLANIDX_MUTEX
		int s = apmib_save_idx();
#else
		apmib_save_idx();
#endif
		apmib_set_wlanidx(wisp_wan_id);
		
		apmib_get(MIB_WLAN_MODE,(void *)&wlan_mode);
		
		if(wlan_mode == CLIENT_MODE)			
			sprintf(tmpbuf, "wlan%d", wisp_wan_id);
		else			
			sprintf(tmpbuf, "wlan%d-vxd0", wisp_wan_id);

		iface=tmpbuf;
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif

	}
	else
		//iface = "eth1";
		iface=eth_ifname;

	if(pWanHWAddr)
	{
		if ( getInAddr(iface, HW_ADDR, (void *)&hwaddr ) ) 
		{
			pMacAddr = hwaddr.sa_data;
			sprintf(pWanHWAddr,"%02x:%02x:%02x:%02x:%02x:%02x",pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
		}
		else
			sprintf(pWanHWAddr,"%s","00:00:00:00:00:00");
	}
	return 0;
}
#endif
/////////////////////////////////////////////////////////////////////////////
int getStats(char *interface, struct user_net_device_stats *pStats)
{
	struct net_device_stats *netdev_stats;
	extern struct net_device_stats *wrapper_get_stats(char *ifname);
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_HAVE_WRAPPER
	if(interface){
		netdev_stats = wrapper_get_stats(interface);
		if (netdev_stats) {
			memcpy(pStats, netdev_stats, sizeof(struct user_net_device_stats));
			return 0;
		}
	}
#endif
	return -1;
}

#ifdef IPCONFLICT_UPDATE_FIREWALL
static unsigned int g_conflictIp_new=0,g_conflictMask=0;
void set_conflict_ip_mask(unsigned int conflictIp_new,unsigned int conflictMask)
{
	g_conflictIp_new=conflictIp_new;
	g_conflictMask=conflictMask;	
}
unsigned int get_conflict_ip(unsigned int myIp)
{
	struct in_addr lanIpAddr, netMaskAddr;
	if (!((getInAddr("eth0", IP_ADDR, (void *)&lanIpAddr)>0) && (lanIpAddr.s_addr>0)))
	    lanIpAddr.s_addr = g_conflictIp_new;    
	if (!((getInAddr("eth0", SUBNET_MASK, (void *)&netMaskAddr)>0) && (netMaskAddr.s_addr>0)))
        netMaskAddr.s_addr = g_conflictMask;

    //diag_printf("%s %d lanIpAddr.s_addr=0x%x netMaskAddr.s_addr=0x%x \n",__FUNCTION__ , __LINE__, lanIpAddr.s_addr,  netMaskAddr.s_addr);
    
	if((myIp != 0x00) && ((myIp & netMaskAddr.s_addr) != (lanIpAddr.s_addr & netMaskAddr.s_addr)))
		myIp=(lanIpAddr.s_addr & netMaskAddr.s_addr) | (myIp & (~(netMaskAddr.s_addr)));

    return myIp;
}
#endif
#ifdef WIFI_SIMPLE_CONFIG
void set_wps_interface(void)
{
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	int wlan0_mode=1, wlan1_mode=1;
	int wlan0_disable=0, wlan1_disable=0;
	int wlan0_wsc_disable=0, wlan1_wsc_disable=0;
	int intf_idx=0, intf_idx_bak=0;
	unsigned char tmpbuf[32];
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	SetWlan_idx("wlan0");
	apmib_get(MIB_WLAN_MODE, &wlan0_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, &wlan0_disable);
	apmib_get(MIB_WLAN_WSC_DISABLE, &wlan0_wsc_disable);
	SetWlan_idx("wlan1");
	apmib_get(MIB_WLAN_MODE, &wlan1_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, &wlan1_disable);
	apmib_get(MIB_WLAN_WSC_DISABLE, &wlan1_wsc_disable);

	sprintf(tmpbuf, "wlan%d", apmib_get_wlanidx());
	SetWlan_idx(tmpbuf);
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
	apmib_get(MIB_WSC_INTF_INDEX, (void *)&intf_idx_bak);
	intf_idx = intf_idx_bak;
	if((wlan0_wsc_disable == 0) && (wlan1_wsc_disable == 0) && (wlan0_disable == 0) && (wlan1_disable == 0))
	{
		if((wlan0_mode == AP_MODE || wlan0_mode == AP_WDS_MODE) && wlan1_mode == CLIENT_MODE)
			intf_idx = 0;
 		else if((wlan1_mode == AP_MODE || wlan1_mode == AP_WDS_MODE) && wlan0_mode == CLIENT_MODE) 
			intf_idx = 2;
 		else if((wlan1_mode == AP_MODE || wlan1_mode == AP_WDS_MODE) && (wlan0_mode == AP_MODE || wlan0_mode == AP_WDS_MODE)) 
 		{
			intf_idx = 5;
 		}
		if(intf_idx_bak != intf_idx)
		{
			apmib_set(MIB_WSC_INTF_INDEX, (void *)&intf_idx);
			apmib_update(CURRENT_SETTING);
		}
	}
	if(wlan0_disable == 0 && wlan1_disable == 1)/*wlan1 disabled*/
	{
		intf_idx = 0;
	}
	else if(wlan0_disable == 1 && wlan1_disable == 0)/*wlan0 disabled*/
	{
		intf_idx = 2;
	}
	
	if(intf_idx_bak != intf_idx)
	{
		apmib_set(MIB_WSC_INTF_INDEX, (void *)&intf_idx);
		apmib_update(CURRENT_SETTING);
	}
#else
	return;
#endif
}
#endif

int accept_timeout(int fd, struct sockaddr_in *addr, socklen_t *addrlen, int time)
{
  int ret=0;
  int selectRet=0;
  if(time > 0) {
    fd_set rSet={0};
	struct timeval timeout={0};
    FD_ZERO(&rSet);
    FD_SET(fd, &rSet);

    
    timeout.tv_sec = time;
    timeout.tv_usec = 0;

   
    do {
      selectRet = select(fd + 1, &rSet, NULL, NULL, &timeout);
    }while(selectRet < 0 && selectRet == EINTR);
    if(selectRet < 0 ) {
      return -1;
    } else if(selectRet == 0) {
      errno = ETIMEDOUT;
      return -ETIMEDOUT;
    }	
  }
  if(addr) {
    ret = accept(fd, (struct sockaddr *)addr, addrlen);
  } else {
    ret = accept(fd, NULL, NULL);
  }
  return ret;
}

