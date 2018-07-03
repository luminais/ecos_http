/*
 *      Utiltiy function to communicate with TCPIP stuffs
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: utility.c,v 1.18 2009/09/04 06:02:14 keith_huang Exp $
 *
 */

/*-- System inlcude files --*/
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h> /* Keith add for tr069 --start */
//#include <sys/sysinfo.h>
#include <stdlib.h>
//#include <memory.h>
#include <ctype.h>
// add by cairui
#include "prmt_igd.h"
#define ECHILD		10
#define IFNAMSIZ	16
#define SIGSTOP 	16
#define SIGTSTP		8
#define SIOCGIWNAME	0x8B01		/* get name == wireless protocol */
extern void formUpload(char *postData, int len);

#define ETHTOOL_GSET            0x00000001 /* Get settings. */
#ifndef SIOCETHTOOL
#define SIOCETHTOOL     0x8946
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <netinet/in.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
//#include <net/if.h>
//#include <linux/wireless.h>
#include <dirent.h>
#include <time.h>
//#include <linux/ethtool.h>
//#include <linux/sockios.h>
/*-- Local include files --*/
#include "apmib.h"
#include "prmt_utility.h"
//#include "../main.h"
//extern int tr069_start(unsigned int argc, unsigned char *argv[]);

/*-- Local constant definition --*/
#define _PATH_PROCNET_ROUTE	"/proc/net/route"
#define _PATH_PROCNET_DEV	"/proc/net/dev"
#define _PATH_RESOLV_CONF	"/etc/resolv.conf"

/* -- Below define MUST same as /linux2.4.18/drivers/net/rtl865x/eth865x.c */
#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
#define RTL8651_IOCTL_GETLANLINKSTATUS 2102
#define RTL8651_IOCTL_GET_ETHER_EEE_STATE 2105
#define RTL8651_IOCTL_GET_ETHER_BYTES_COUNT 2106

/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP			0x0001          /* route usable                 */
#define RTF_GATEWAY		0x0002          /* destination is a gateway     */

#define READ_BUF_SIZE	50
const char STR_DISABLE[] = "Disabled";
const char STR_ENABLE[] = "Enabled";
const char STR_AUTO[] = "Auto";
const char STR_MANUAL[] = "Manual";
const char STR_UNNUMBERED[] = "unnumbered";
const char STR_ERR[] = "err";
const char STR_NULL[] = "null";
const char EMPTY_MAC[MAC_ADDR_LEN] = {0};

/*-- Local routine declaration --*/
//static int get_dev_fields(int type, char *bp, struct user_net_device_stats *pStats);
static char *get_name(char *name, char *p);

#if defined(CONFIG_APP_TR069)
#if defined(_PRMT_TR143_)
#include <apmib.h>
char *strItf[]=
{
    "",     //ITF_ALL
    ALIASNAME_ETH1,     //ITF_WAN
    ALIASNAME_BR0,      //ITF_LAN

    ALIASNAME_ETH0,     //ITF_ETH0
    ALIASNAME_ETH0_SW0, //ITF_ETH0_SW0
    ALIASNAME_ETH0_SW1, //ITF_ETH0_SW1
    ALIASNAME_ETH0_SW2, //ITF_ETH0_SW2
    ALIASNAME_ETH0_SW3, //ITF_ETH0_SW3

    ALIASNAME_WLAN0,    //ITF_WLAN0
    ALIASNAME_WLAN0_VAP0,   //ITF_WLAN0_VAP0
    ALIASNAME_WLAN0_VAP1,   //ITF_WLAN0_VAP1
    ALIASNAME_WLAN0_VAP2,   //ITF_WLAN0_VAP2
    ALIASNAME_WLAN0_VAP3,   //ITF_WLAN0_VAP3

    ALIASNAME_WLAN1,    //ITF_WLAN0
    ALIASNAME_WLAN1_VAP0,   //ITF_WLAN0_VAP0
    ALIASNAME_WLAN1_VAP1,   //ITF_WLAN0_VAP1
    ALIASNAME_WLAN1_VAP2,   //ITF_WLAN0_VAP2
    ALIASNAME_WLAN1_VAP3,   //ITF_WLAN0_VAP3

    /*
       "br0",      //ITF_LAN

       "eth0",     //ITF_ETH0
       "eth0_sw0", //ITF_ETH0_SW0
       "eth0_sw1", //ITF_ETH0_SW1
       "eth0_sw2", //ITF_ETH0_SW2
       "eth0_sw3", //ITF_ETH0_SW3

       "wlan0",    //ITF_WLAN0
       "wlan0-vap0",   //ITF_WLAN0_VAP0
       "wlan0-vap1",   //ITF_WLAN0_VAP1
       "wlan0-vap2",   //ITF_WLAN0_VAP2
       "wlan0-vap3",   //ITF_WLAN0_VAP3

       "wlan1",    //ITF_WLAN0
       "wlan1-vap0",   //ITF_WLAN0_VAP0
       "wlan1-vap1",   //ITF_WLAN0_VAP1
       "wlan1-vap2",   //ITF_WLAN0_VAP2
       "wlan1-vap3",   //ITF_WLAN0_VAP3
       */

    "usb0",     //ITF_USB0

    ""      //ITF_END
};
#endif
#endif

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
int cwmp_getWlStaNum( char *interface, int *num )
{
#ifndef NO_ACTION
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
#else
	*num = 0 ;
#endif

	return 0;
}
#ifdef CONFIG_RTL_WAPI_SUPPORT
/*
 *   parameters:
 *   certsInfo (output): to store user cert information get from from CERTS_DATABASE
 *   count (input): number of entries in CERTS_DATABASE
 *
 *   return 0: success; return -1: failed
 */
#define SUCCESS 0
#define FAILED -1
int getCertsDb(CERTS_DB_ENTRY_Tp certsInfo, int count, int *realcount)
{
	FILE *fp;
	time_t  now, expired_tm;
	struct tm *tnow;
	struct tm tm_time;

	struct stat status;
	int readSize;
	int ret, toRet;
	int i,intVal;
	long longVal;
	char *p1, *p2, *ptr;

	char buffer[100];
	char tmpBuf[100];//Added for test
	char tmpBuf2[3];//Added for test

	if ( stat(CERTS_DATABASE, &status) < 0)
	{
		//printf("%s(%d): %s not exist.\n",__FUNCTION__,__LINE__, CERTS_DATABASE);//Added for test
		toRet=FAILED;
		goto err;
	}
	//        printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for tes
	fp = fopen(CERTS_DATABASE, "r");
	if (!fp) {
		//printf("open %s error.\n", CERTS_DATABASE);//Added for test
		toRet=FAILED;
		goto err;
	}

	p1=NULL;
	p2=NULL;
	for(i=0;i<count;i++)
	{
		*realcount=i;
		if(!fgets(buffer, sizeof(buffer), fp))
		{
			//			printf("%s(%d): file end.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=SUCCESS;
			goto err;
		}
		//                printf("%s(%d),i=%d,buffer=%s, len=%d\n",__FUNCTION__,__LINE__,i,buffer, strlen(buffer));//Added for test

		//To set cert type, 0: X.509 (only at preset)
		certsInfo[i].certType=0;

		//dumpHex(buffer, strlen(buffer)+1);
		if(buffer[0]=='E')
		{
			//Expired
			certsInfo[i].certStatus=1;
		}
		else if(buffer[0]=='R')
		{
			//Revoked
			certsInfo[i].certStatus=2;
		}
		else
		{
			//Valid
			certsInfo[i].certStatus=0;
		}
		//printf("%s(%d): certsInfo[i].certStatus=%d.\n",__FUNCTION__,__LINE__, certsInfo[i].certStatus);//Added for test

		//To parse exipred time
		p1=strchr(buffer,'\t');
		if(p1==NULL)
		{
			//printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
		p1++;
		p2=strchr(p1,'\t');
		if(p2==NULL)
		{
			//printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
		if(p2>p1)
		{
			memset(tmpBuf, 0, sizeof(tmpBuf));
			//memset(tmpBuf2, 0, sizeof(tmpBuf2));
			strncpy(tmpBuf, p1, p2-p1);
			//printf("%s(%d): tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
			memset(&tm_time, 0 , sizeof(tm_time));

			//?????
			//tm_time.tm_isdst=-1;

			//To get year value
			memset(tmpBuf2, 0, sizeof(tmpBuf2));
			strncpy(tmpBuf2, tmpBuf, 2);
			intVal=atoi(tmpBuf2);
			if(intVal>=70)
			{
				//year: 1970 ~ 1999
				//year - 1900
				tm_time.tm_year=(intVal+1900)-1900;
			}
			else
			{
				//year: 2000 ~ 2069
				//year - 1900
				tm_time.tm_year=(intVal+2000)-1900;
			}

			//To get month value
			memset(tmpBuf2, 0, sizeof(tmpBuf2));
			strncpy(tmpBuf2, &tmpBuf[2], 2);
			tm_time.tm_mon=atoi(tmpBuf2)-1;

			//To get day value
			memset(tmpBuf2, 0, sizeof(tmpBuf2));
			strncpy(tmpBuf2, &tmpBuf[4], 2);
			tm_time.tm_mday=atoi(tmpBuf2);

			//To get hour value
			memset(tmpBuf2, 0, sizeof(tmpBuf2));
			strncpy(tmpBuf2, &tmpBuf[6], 2);
			tm_time.tm_hour=atoi(tmpBuf2);

			//To get minute value
			memset(tmpBuf2, 0, sizeof(tmpBuf2));
			strncpy(tmpBuf2, &tmpBuf[8], 2);
			tm_time.tm_min=atoi(tmpBuf2);

			//To get second value
			memset(tmpBuf2, 0, sizeof(tmpBuf2));
			strncpy(tmpBuf2, &tmpBuf[10], 2);
			tm_time.tm_sec=atoi(tmpBuf2);

			//      printf("(0): %d %d %d %d %d %d, tm_isdst=%d\n", 1900+tm_time.tm_year,tm_time.tm_mon+1,tm_time.tm_mday,tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec, tm_time.tm_isdst);//Added for test

			expired_tm = mktime(&tm_time);
			if(expired_tm < 0){
				//printf("Error:set Time Error for tm!\n");//Added for test
				toRet=FAILED;
				goto err;
			}
			//      printf("%s(%d): expired_tm=%ld.\n",__FUNCTION__,__LINE__,expired_tm);//Added for test


			now=time(0);
			tnow=localtime(&now);
			//      printf("now=%ld, %d %d %d %d %d %d, tm_isdst=%d\n",now, 1900+tnow->tm_year,tnow->tm_mon+1,tnow->tm_mday,tnow->tm_hour,tnow->tm_min,tnow->tm_sec, tnow->tm_isdst);//Added for test

			longVal=difftime(expired_tm,now);
			//                        printf("The difference is: %ld seconds\n",longVal);
			if(longVal<=0)
				certsInfo[i].validDaysLeft=0;
			else
				certsInfo[i].validDaysLeft=(unsigned short)(longVal/ONE_DAY_SECONDS)+1;

			//      printf("%s(%d): certsInfo[%d].validDaysLeft=%d.\n",__FUNCTION__,__LINE__,i, certsInfo[i].validDaysLeft);//Added for test

			//                      printf("%s(%d): tmpBuf2=%s.\n",__FUNCTION__,__LINE__, tmpBuf2);//Added for test
		}

		//To parse revoked time(Not used now)
		p1=p2;
		p1++;
		p2=strchr(p1,'\t');
		if(p2==NULL)
		{
			diag_printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
		if(p2>p1)
		{
			memset(tmpBuf, 0, sizeof(tmpBuf));
			strncpy(tmpBuf, p1, p2-p1);
			//                     printf("%s(%d): tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
		}

		//To parse serial
		p1=p2;
		p1++;
		p2=strchr(p1,'\t');
		if(p2==NULL)
		{
			diag_printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
		if(p2>p1)
		{
			memset(tmpBuf, 0, sizeof(tmpBuf));
			strncpy(tmpBuf, p1, p2-p1);
			//                     printf("%s(%d):serial tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
			certsInfo[i].serial=strtol(tmpBuf, (char **)NULL,16);
			//                     ret=str2hex(tmpBuf, &certsInfo[i].serial);
			//                     printf("%s(%d), ret=%d, certsInfo[%d].serial=0x%x\n",__FUNCTION__,__LINE__, ret, i,certsInfo[i].serial);//Added for test
#if 0
			if(ret==FAILED)
			{
				printf("%s(%d), str2hex failed.\n",__FUNCTION__,__LINE__);//Added for test
				toRet=FAILED;
				goto err;
			}
#endif						
		}
		//To parse total valid days
		p1=p2;
		p1++;
		p2=strchr(p1,'\t');
		if(p2==NULL)
		{
			diag_printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
		if(p2>p1)
		{
			memset(tmpBuf, 0, sizeof(tmpBuf));
			strncpy(tmpBuf, p1, p2-p1);
			//printf("%s(%d):total valid days tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
			certsInfo[i].validDays=(unsigned short)atoi(tmpBuf);
			//printf("%s(%d):certsInfo[%d].validDays=%d.\n",__FUNCTION__,__LINE__, i, certsInfo[i].validDays);//Added for test
		}

		if((certsInfo[i].validDaysLeft>certsInfo[i].validDays)
				||((certsInfo[i].certStatus==1)&&certsInfo[i].validDaysLeft>0))
		{
			//printf("%s(%d), warning: system time setting is not correct.\n",__FUNCTION__,__LINE__);//Added for test
			//To indicate our system hasn't sync time yet
			sprintf(tmpBuf,"echo \"1\" > %s", SYS_TIME_NOT_SYNC_CA);
			system(tmpBuf);
			//End indication
			certsInfo[i].validDaysLeft=0;
		}

		//To parse user name
		p1=p2;
		p1++;
		p2=strchr(p1,'\n');
		if(p2==NULL)
		{
			//printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
		if(p2>p1)
		{
			memset(tmpBuf, 0, sizeof(tmpBuf));
			strncpy(tmpBuf, p1, p2-p1);
			//printf("%s(%d):user name tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
			ptr=NULL;
			ptr=strstr(p1, "CN=");
			if(p2==NULL)
			{
				//printf("%s(%d): strstr failed.\n",__FUNCTION__,__LINE__);//Added for test
				toRet=FAILED;
				goto err;
			}
			ptr+=3;//Point to user name
			memset(certsInfo[i].userName, 0, sizeof(certsInfo[i].userName));
			strncpy(certsInfo[i].userName, ptr, p2-ptr);
			//printf("%s(%d):certsInfo[%d].userName=%s.\n",__FUNCTION__,__LINE__, i, certsInfo[i].userName);//Added for test
		}

		//              p1=buffer;
		//              p2=strstr(p1,"\t");
		//              printf("%s(%d),i=%d,p2=%s, len=%d\n",__FUNCTION__,__LINE__,i,buffer, strlen(p2));//Added for test
	}

	toRet=SUCCESS;

err:
	if(fp!=NULL)
		fclose(fp);
	//        printf("%s(%d), toRet=%d\n",__FUNCTION__,__LINE__,toRet);//Added for tes
	return toRet;
}
static int searchCertStatus(CERTS_DB_ENTRY_Tp all, CERTS_DB_ENTRY_Tp cert,int status, int count)
{
	int i=0;
	int cnt=0;
	for(i=0;i<count;i++)
	{
		if(all[i].certStatus == status)
		{
			memcpy(cert+cnt,all+i,sizeof(CERTS_DB_ENTRY_T));
			cnt++;
		}
	}
	return cnt;
}

static int searchCertName(CERTS_DB_ENTRY_Tp all, CERTS_DB_ENTRY_Tp cert,char *buffer, int count)
{
	int i=0;
	int cnt=0;
	for(i=0;i<count;i++)
	{
		if(!strcmp(all[i].userName, buffer))
		{
			memcpy(cert+cnt,all+i,sizeof(CERTS_DB_ENTRY_T));
			cnt++;
		}
	}
	return cnt;
}
static int searchCertSerial(CERTS_DB_ENTRY_Tp  all, CERTS_DB_ENTRY_Tp cert, unsigned long serial, int count)
{
	int i=0;
	int cnt=0;
	for(i=0;i<count;i++)
	{
		if(all[i].serial==serial)
		{
			memcpy(cert+cnt,all+i,sizeof(CERTS_DB_ENTRY_T));
			cnt++;
		}
	}
	return cnt;
}

int searchWapiCert(CERTS_DB_ENTRY_Tp cert, int index, char *buffer)
{
	int status=0;
	int all=0;
	int count=0,searchCnt=0;
	int retVal;
	unsigned long serial=0;
	CERTS_DB_ENTRY_Tp allCert=(CERTS_DB_ENTRY_Tp)malloc(sizeof(CERTS_DB_ENTRY_T)*128);
	retVal=getCertsDb(allCert,128,&count);
	if(retVal < 0)
	{
		return 0;
	}
	if(count == 0)
	{
		free(allCert);
		return count;
	}
	switch (index)
	{
		case 5:
			/*0 actived, 2 revoked*/
			status=buffer[0]-'0';
			searchCnt=searchCertStatus(allCert,cert,status,count);
			break;
		case 4:
			/*now only support x.509*/
			all=1;
			break;
		case 3:
			/*username is in buffer*/
			searchCnt=searchCertName(allCert,cert,buffer,count);
			break;
		case 2:
			/*serail  in buffer in ASCII*/
			serial=strtol(buffer,(char **)NULL,16);
			searchCnt=searchCertSerial(allCert,cert,serial,count);
			break;
		case 1:		
			/*All*/
			all=1;
			break;
		default:
			/*all*/
			all=1;
			break;
	}
	if(all)
	{
		memcpy(cert,allCert,count*sizeof(CERTS_DB_ENTRY_T));
		searchCnt=count;
	}
	free(allCert);
	return searchCnt;
}
#endif
/////////////////////////////////////////////////////////////////////////////
int cwmp_getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
#ifndef NO_ACTION
	int skfd=0;
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
	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);
	memset(pInfo, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));

	if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0){
		close( skfd );
		return -1;
	}
	close( skfd );
#else
	return -1;
#endif
	return 0;
}

// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
#if 0//def CONFIG_RTK_MESH 
int setWlJoinMesh (char *interface, unsigned char* MeshId, int MeshId_Len, int Channel, int Reset) 
{ 
	int skfd; 
	struct iwreq wrq; 
	struct 
	{
		unsigned char *meshid;
		int meshid_len, channel, reset; 
	}mesh_identifier; 

	skfd = socket(AF_INET, SOCK_DGRAM, 0); 

	/* Get wireless name */ 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
		/* If no wireless name : no wireless extensions */ 
		return -1; 

	mesh_identifier.meshid = MeshId; 
	mesh_identifier.meshid_len = MeshId_Len; 
	mesh_identifier.channel = Channel;
	mesh_identifier.reset = Reset;
	wrq.u.data.pointer = (caddr_t)&mesh_identifier; 
	wrq.u.data.length = sizeof(mesh_identifier); 

	if (iw_get_ext(skfd, interface, SIOCJOINMESH, &wrq) < 0) 
		return -1; 

	close( skfd ); 

	return 0; 
} 

// This function might be removed when the mesh peerlink precedure has been completed
int getWlMeshLink (char *interface, unsigned char* MacAddr, int MacAddr_Len) 
{ 
	int skfd; 
	struct iwreq wrq; 

	skfd = socket(AF_INET, SOCK_DGRAM, 0); 

	/* Get wireless name */ 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
		/* If no wireless name : no wireless extensions */ 
		return -1; 

	wrq.u.data.pointer = (caddr_t)MacAddr; 
	wrq.u.data.length = MacAddr_Len; 

	if (iw_get_ext(skfd, interface, SIOCCHECKMESHLINK, &wrq) < 0) 
		return -1; 

	close( skfd ); 

	return 0; 
} 

// This function might be removed when the mesh peerlink precedure has been completed
int getWlMib (char *interface, unsigned char* Oid, int Oid_Len) 
{ 
	int skfd, ret = -1; 
	struct iwreq wrq; 

	skfd = socket(AF_INET, SOCK_DGRAM, 0); 

	/* Get wireless name */ 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
		/* If no wireless name : no wireless extensions */ 
		return -1; 

	wrq.u.data.pointer = (caddr_t)Oid; 
	wrq.u.data.length = Oid_Len; 
	ret = iw_get_ext(skfd, interface, RTL8190_IOCTL_GET_MIB, &wrq);

	close( skfd ); 
	if(ret < 0)
		return -1;
	return ret; 
} 
#endif 
// ==== GANTOE ==== 
/////////////////////////////////////////////////////////////////////////////
int cwmp_getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
#ifndef NO_ACTION
	int skfd=0;
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
	wrq.u.data.pointer = (caddr_t)pStatus;

	if ( pStatus->number == 0 )
		wrq.u.data.length = sizeof(SS_STATUS_T);
	else
		wrq.u.data.length = sizeof(pStatus->number);

	if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0){
		close( skfd );
		return -1;
	}
	close( skfd );
#else
	return -1 ;
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int cwmp_getWlJoinRequest(char *interface, pBssDscr pBss, unsigned char *res)
{
#ifndef NO_ACTION
	int skfd=0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
		/* If no wireless name : no wireless extensions */
		return -1;

	wrq.u.data.pointer = (caddr_t)pBss;
	wrq.u.data.length = sizeof(BssDscr);

	if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQ, &wrq) < 0)
		return -1;

	close( skfd );

	*res = *(unsigned char *)&wrq.u.data.pointer[0];
#else
	return -1;
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int cwmp_getWlJoinResult(char *interface, unsigned char *res)
{
	int skfd;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
		/* If no wireless name : no wireless extensions */
		close( skfd );
		return -1;
	}
	wrq.u.data.pointer = (caddr_t)res;
	wrq.u.data.length = 1;

	if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQSTATUS, &wrq) < 0){
		close( skfd );
		return -1;
	}
	close( skfd );

	return 0;
}



/////////////////////////////////////////////////////////////////////////////
int cwmp_getWlSiteSurveyRequest(char *interface, int *pStatus)
{
#ifndef NO_ACTION
	int skfd=0;
	struct iwreq wrq;
	unsigned char result;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
		/* If no wireless name : no wireless extensions */
		close( skfd );
		return -1;
	}
	wrq.u.data.pointer = (caddr_t)&result;
	wrq.u.data.length = sizeof(result);

	if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0){
		close( skfd );
		return -1;
	}
	close( skfd );

	if ( result == 0xff )
		*pStatus = -1;
	else
		*pStatus = (int) result;
#else
	*pStatus = -1;
#endif
#ifdef CONFIG_RTK_MESH 
	// ==== modified by GANTOE for site survey 2008/12/26 ==== 
	return (int)*(char*)wrq.u.data.pointer; 
#else
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////
int cwmp_getWlBssInfo(char *interface, bss_info *pInfo)
{
#ifndef NO_ACTION
	int skfd=0;
	struct iwreq wrq;



	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
#ifdef VOIP_SUPPORT 
		aaaaa;
	{
		// rock: avoid status page error if no wlan interface
		memset(pInfo, 0, sizeof(bss_info));
		return 0;
	}
#else
	//bbbbbb;
	/* If no wireless name : no wireless extensions */
	{
		close( skfd );
		return -1;
	}
#endif

	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length = sizeof(bss_info);

	if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0){
		close( skfd );
		return -1;
	}
	close( skfd );
#else
	memset(pInfo, 0, sizeof(bss_info)); 
#endif

	return 0;
}



/////////////////////////////////////////////////////////////////////////////
#if 0 //by cairui use extern in common.c
int getInAddr( char *interface, ADDR_T type, void *pAddr )
{
	struct ifreq ifr;
	int skfd=0, found=0;
	struct sockaddr_in *addr;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return 0;

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
	else if (type == DST_IP_ADDR) {
		if (ioctl(skfd,SIOCGIFDSTADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
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
#endif

/////////////////////////////////////////////////////////////////////////////
extern pid_t find_pid_by_name( char* pidName)
{
	DIR *dir;
	struct dirent *next;

	dir = opendir("/proc");
	if (!dir) {
		diag_printf("Cannot open /proc");
		return 0;
	}

	while ((next = readdir(dir)) != NULL) {
		FILE *status;
		char filename[READ_BUF_SIZE];
		char buffer[READ_BUF_SIZE];
		char name[READ_BUF_SIZE];

		/* Must skip ".." since that is outside /proc */
		if (strcmp(next->d_name, "..") == 0)
			continue;

		/* If it isn't a number, we don't want it */
		if (!isdigit(*next->d_name))
			continue;

		sprintf(filename, "/proc/%s/status", next->d_name);
		if (! (status = fopen(filename, "r")) ) {
			continue;
		}
		if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(buffer, "%*s %s", name);
		if (strcmp(name, pidName) == 0) {
			//	pidList=xrealloc( pidList, sizeof(pid_t) * (i+2));
			return((pid_t)strtol(next->d_name, NULL, 0));

		}
	}
	if ( strcmp(pidName, "init")==0)
		return 1;

	return 0;
}


#if 0
/////////////////////////////////////////////////////////////////////////////
int setInAddr(char *interface, ADDR_T type, void *addr)
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int sock, cmd, ret;

	if ( type == IP_ADDR )
		cmd = SIOCSIFADDR;
	else if ( type == SUBNET_MASK )
		cmd = SIOCSIFNETMASK;
	else
		return -1;

	memset(&ifr, 0, sizeof(struct ifreq));

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, interface);
	memset(&sin, 0, sizeof(struct sockaddr));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = *((unsigned long *)addr);
	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
	if ( (ret=ioctl(sock, cmd, &ifr)) < 0)
		goto set_exit;

	// up interface
	strcpy(ifr.ifr_name, interface);
	if ((ret = ioctl(sock, SIOCGIFFLAGS, &ifr)) < 0)
		goto set_exit;

	strcpy(ifr.ifr_name, interface);
	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
	ret = ioctl(sock, SIOCSIFFLAGS, &ifr);

set_exit:
	close(sock);

	return ret;
}
#endif

/////////////////////////////////////////////////////////////////////////////
int cwmp_getDefaultRoute(char *interface, struct in_addr *route)
{
	char buff[1024], iface[16];
	char gate_addr[128], net_addr[128], mask_addr[128];
	int num, iflags, metric, refcnt, use, mss, window, irtt;
	FILE *fp = fopen(_PATH_PROCNET_ROUTE, "r");
	char *fmt;
	int found=0;
	unsigned long addr;

	if (!fp) {
		diag_printf("Open %s file error.\n", _PATH_PROCNET_ROUTE);
		return 0;
	}

	fmt = "%16s %128s %128s %X %d %d %d %128s %d %d %d";

	while (fgets(buff, 1023, fp)) {
		num = sscanf(buff, fmt, iface, net_addr, gate_addr,
				&iflags, &refcnt, &use, &metric, mask_addr, &mss, &window, &irtt);
		if (num < 10 || !(iflags & RTF_UP) || !(iflags & RTF_GATEWAY) || strcmp(iface, interface))
			continue;
		sscanf(gate_addr, "%lx", &addr );
		*route = *((struct in_addr *)&addr);

		found = 1;
		break;
	}

	fclose(fp);
	return found;
}

#if 0
/////////////////////////////////////////////////////////////////////////////
int deleteDefaultRoute(char *dev, void *route)
{
	struct rtentry rt;
	int skfd;
	struct sockaddr_in *pAddr;

	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));

	/* Fill in the other fields. */
	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = dev;
	pAddr= (struct sockaddr_in *)&rt.rt_dst;
	pAddr->sin_family = AF_INET;

	pAddr= (struct sockaddr_in *)&rt.rt_gateway;
	pAddr->sin_family = AF_INET;
	pAddr->sin_addr.s_addr = *((unsigned long *)route);

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Create socket error");
		return -1;
	}

	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
		printf("Delete route [SIOCDELRT] error!");
		close(skfd);
		return -1;;
	}

	/* Close the socket. */
	close(skfd);
	return (0);
}

/////////////////////////////////////////////////////////////////////////////
int addDefaultRoute(char *dev, void *route)
{
	struct rtentry rt;
	int skfd;
	struct sockaddr_in *pAddr;

	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));

	/* Fill in the other fields. */
	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = dev;
	pAddr= (struct sockaddr_in *)&rt.rt_dst;
	pAddr->sin_family = AF_INET;

	pAddr= (struct sockaddr_in *)&rt.rt_gateway;
	pAddr->sin_family = AF_INET;
	pAddr->sin_addr.s_addr = *((unsigned long *)route);

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Create socket error");
		return -1;
	}

	if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
		printf("Add route [SIOCADDRT] error!");
		close(skfd);
		return -1;
	}

	/* Close the socket. */
	close(skfd);
	return (0);
}
#endif

/////////////////////////////////////////////////////////////////////////////
#if 0//by cairui
int cwmp_getStats(char *interface, struct user_net_device_stats *pStats)
{
	FILE *fh;
	char buf[512];
	int type;

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		printf("Warning: cannot open %s\n",_PATH_PROCNET_DEV);
		return -1;
	}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);

	if (strstr(buf, "compressed"))
		type = 3;
	else if (strstr(buf, "bytes"))
		type = 2;
	else
		type = 1;

	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[40];
		s = get_name(name, buf);
		if ( strcmp(interface, name))
			continue;
		get_dev_fields(type, s, pStats);
		fclose(fh);
		return 0;
	}
	fclose(fh);
	return -1;
}
#endif
/////////////////////////////////////////////////////////////////////////////
static char *get_name(char *name, char *p)
{
	while (isspace(*p))
		p++;
	while (*p) {
		if (isspace(*p))
			break;
		if (*p == ':') {	/* could be an alias */
			char *dot = p, *dotname = name;
			*name++ = *p++;
			while (isdigit(*p))
				*name++ = *p++;
			if (*p != ':') {	/* it wasn't, backup */
				p = dot;
				name = dotname;
			}
			if (*p == '\0')
				return NULL;
			p++;
			break;
		}
		*name++ = *p++;
	}
	*name++ = '\0';
	return p;
}

////////////////////////////////////////////////////////////////////////////////
#if 0 //by cairui
static int get_dev_fields(int type, char *bp, struct user_net_device_stats *pStats)
{
	switch (type) {
		case 3:
			sscanf(bp,
					"%Lu %Lu %lu %lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu %lu",
					&pStats->rx_bytes,
					&pStats->rx_packets,
					&pStats->rx_errors,
					&pStats->rx_dropped,
					&pStats->rx_fifo_errors,
					&pStats->rx_frame_errors,
					&pStats->rx_compressed,
					&pStats->rx_multicast,

					&pStats->tx_bytes,
					&pStats->tx_packets,
					&pStats->tx_errors,
					&pStats->tx_dropped,
					&pStats->tx_fifo_errors,
					&pStats->collisions,
					&pStats->tx_carrier_errors,
					&pStats->tx_compressed);
			break;

		case 2:
			sscanf(bp, "%Lu %Lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu",
					&pStats->rx_bytes,
					&pStats->rx_packets,
					&pStats->rx_errors,
					&pStats->rx_dropped,
					&pStats->rx_fifo_errors,
					&pStats->rx_frame_errors,

					&pStats->tx_bytes,
					&pStats->tx_packets,
					&pStats->tx_errors,
					&pStats->tx_dropped,
					&pStats->tx_fifo_errors,
					&pStats->collisions,
					&pStats->tx_carrier_errors);
			pStats->rx_multicast = 0;
			break;

		case 1:
			sscanf(bp, "%Lu %lu %lu %lu %lu %Lu %lu %lu %lu %lu %lu",
					&pStats->rx_packets,
					&pStats->rx_errors,
					&pStats->rx_dropped,
					&pStats->rx_fifo_errors,
					&pStats->rx_frame_errors,

					&pStats->tx_packets,
					&pStats->tx_errors,
					&pStats->tx_dropped,
					&pStats->tx_fifo_errors,
					&pStats->collisions,
					&pStats->tx_carrier_errors);
			pStats->rx_bytes = 0;
			pStats->tx_bytes = 0;
			pStats->rx_multicast = 0;
			break;
	}
	return 0;
}
#endif


/////////////////////////////////////////////////////////////////////////////
int cwmp_getWdsInfo(char *interface, char *pInfo)
{

#ifndef NO_ACTION
	int skfd;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
		/* If no wireless name : no wireless extensions */
		return -1;

	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length = MAX_WDS_NUM*sizeof(WDS_INFO_T);

	if (iw_get_ext(skfd, interface, SIOCGIWRTLGETWDSINFO, &wrq) < 0)
		return -1;

	close( skfd );
#else
	memset(pInfo, 0, MAX_WDS_NUM*sizeof(WDS_INFO_T)); 
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int cwmp_getMiscData(char *interface, struct _misc_data_ *pData)
{

#ifndef NO_ACTION
	int skfd;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
		/* If no wireless name : no wireless extensions */
		return -1;

	wrq.u.data.pointer = (caddr_t)pData;
	wrq.u.data.length = sizeof(struct _misc_data_);

	if (iw_get_ext(skfd, interface, SIOCGMISCDATA, &wrq) < 0)
		return -1;

	close(skfd);
#else
	memset(pData, 0, sizeof(struct _misc_data_)); 
#endif

	return 0;
}

/*      IOCTL system call */
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
	{
		perror("fatal error socket\n");
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

#ifdef HOME_GATEWAY
int cwmp_getWanInfo(char *pWanIP, char *pWanMask, char *pWanDefIP, char *pWanHWAddr)
{
	DHCP_T dhcp;
	OPMODE_T opmode=-1;
	unsigned int wispWanId=0;
	char *iface=NULL;
	struct in_addr	intaddr;
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;

	if ( !apmib_get( MIB_WAN_DHCP, (void *)&dhcp) )
		return -1;

	if ( !apmib_get( MIB_OP_MODE, (void *)&opmode) )
		return -1;

	if( !apmib_get(MIB_WISP_WAN_ID, (void *)&wispWanId))
		return -1;

	if ( dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP || dhcp == USB3G ) { /* # keith: add l2tp support. 20080515 */
#ifdef MULTI_PPPOE
		if(dhcp == PPPOE){
			extern char  ppp_iface[32];
			iface = ppp_iface;
		}
#else
		iface = "ppp0";
#endif

		if ( !isConnectPPP() )
			iface = NULL;
	}
	else if (opmode == WISP_MODE){
		if(0 == wispWanId)
			iface = "wlan0";
		else if(1 == wispWanId)
			iface = "wlan1";
	}
	else
		iface = "eth1";

	if(opmode != WISP_MODE)
	{
		if(iface){
			if(getWanLink("eth1") < 0){
				sprintf(pWanIP,"%s","0.0.0.0");
			}
		}	
	}

	if ( iface && getInAddr(iface, IP_ADDR, (void *)&intaddr ) )
		sprintf(pWanIP,"%s",inet_ntoa(intaddr));
	else
		sprintf(pWanIP,"%s","0.0.0.0");

	if ( iface && getInAddr(iface, SUBNET_MASK, (void *)&intaddr ) )
		sprintf(pWanMask,"%s",inet_ntoa(intaddr));
	else
		sprintf(pWanMask,"%s","0.0.0.0");

	if ( iface && cwmp_getDefaultRoute(iface, &intaddr) )
		sprintf(pWanDefIP,"%s",inet_ntoa(intaddr));
	else
		sprintf(pWanDefIP,"%s","0.0.0.0");	

	//To get wan hw addr
	if(opmode == WISP_MODE) {
		if(0 == wispWanId)
			iface = "wlan0";
		else if(1 == wispWanId)
			iface = "wlan1";
	}	
	else
		iface = "eth1";

	if ( getInAddr(iface, HW_ADDR, (void *)&hwaddr ) ) 
	{
		pMacAddr = (unsigned char *)hwaddr.sa_data;
		sprintf(pWanHWAddr,"%02x:%02x:%02x:%02x:%02x:%02x",pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
	}
	else
		sprintf(pWanHWAddr,"%s","00:00:00:00:00:00");

	return 0;
}
#endif

/* ethernet port link status */
int getEth0PortLink(unsigned int port_index)
{
	int    ret=-1;
	unsigned int    args[0];

	ret = port_index;
	re865xIoctl("eth0", RTL8651_IOCTL_GETLANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret);
	if(ret ==0){
		return 1;
	}else if(ret < 0){
		return 0;
	}
	return 0;
}

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD)//only support in 8196c or RTL8198
/* RTL8651_IOCTL_GET_ETHER_EEE_STATE */
int getEthernetEeeState(unsigned int port_index)
{
	unsigned int    ret=0;
	unsigned int    args[0];

	re865xIoctl("eth0", RTL8651_IOCTL_GET_ETHER_EEE_STATE, (unsigned int)(args), 0, (unsigned int)&ret);      

	//if(ret & (1<<(3+port_index*4)))
	if(ret & (1<<(1+port_index*4)))
		return 1;
	else
		return 0;
}
#endif //#if defined(CONFIG_RTL_8196C)

/* RTL8651_IOCTL_GET_ETHER_BYTES_COUNT */
unsigned int getEthernetBytesCount(unsigned int port_index)
{
	unsigned int    ret=0;
	unsigned int    args[0];

	ret = port_index;
	re865xIoctl("eth0", RTL8651_IOCTL_GET_ETHER_BYTES_COUNT, (unsigned int)(args), 0, (unsigned int)&ret);      

	return ret;
}


#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
/* Wan link status detect */
#if 0 //use extern in common.c by cairui
int getWanLink(char *interface)
{
	unsigned int    ret;
	unsigned int    args[0];

	re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret) ;
	return ret;
}
#endif

#ifdef UNIVERSAL_REPEATER
/////////////////////////////////////////////////////////////////////////////
int isVxdInterfaceExist(char *interface)
{
#ifndef NO_ACTION	
	int skfd, ret;  
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, interface);
	if ((ret = ioctl(skfd, SIOCGIFFLAGS, &ifr)) < 0)
		ret = 0;
	else {
		if (!(ifr.ifr_flags & IFF_UP))
			ret = 0;	
		else
			ret = 1;
	}
	close(skfd);

	return ret;	
#else	

	return 0;
#endif	

}
#endif // UNIVERSAL_REPEATER

#if 0
int displayPostDate(char *postDate)
{
	char *strP;
	int	len=0;
	char str[200];

	memset(str, 0x00, sizeof(str));

	if (postDate != NULL)
	{
		printf("\r\n --- postDate ---\r\n");
		strP = postDate;

		while(*strP != '\0')
		{
			while(*strP != '&' && *strP != '\0')
			{
				strP++;
				len++;
			}
			strncpy(str,postDate, len);
			printf(" %s\r\n",str);

			if(*strP == '\0')
			{

			}
			else
			{
				postDate = ++strP;
				len = 0;
				memset(str, 0x00, sizeof(str));
			}
		}

		printf(" ----------------\r\n");
	}
	return 0;
}
#endif

/* by cairui: use extern fwChecksumOk
   int fwChecksumOk(char *data, int len)
   {
   unsigned short sum=0;
   int i;

   for (i=0; i<len; i+=2) {
#ifdef _LITTLE_ENDIAN_
sum += WORD_SWAP( *((unsigned short *)&data[i]) );
#else
sum += *((unsigned short *)&data[i]);
#endif

}
return( (sum==0) ? 1 : 0);
}
*/
///////////////////////////////////////////////////////////////////////////////
int do_cmd(const char *filename, char *argv [], int dowait)
{
	//printf("ALTER: enter do_cmd in prmt_utility.c\n");
#if 0 // by cairui for debug
	pid_t pid, wpid;
	int stat=0, st;

	if((pid = vfork()) == 0) { 
		//if((pid = cyg_hal_sys_fork()) == 0) { 
		// the child
		char *env[3];

		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
#if 1
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
#endif
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	return st;
#endif
	if(!strcmp(filename, "cwmpClient")){
		tr069_start(0, NULL);
	}
#ifdef _PRMT_TR143_
	else if (dowait>0 && strcmp(filename, "udpechoserver")==0) {
		cyg_udpechoserver_start(dowait, argv); //use dowait as argc here
	}
#endif
	return 0;
}

	int va_cmd(const char *cmd, int num, int dowait, ...)
	{
		va_list ap;
		int k;
		char *s;
		char *argv[24];
		int status;

		va_start(ap, dowait);

		for (k=0; k<num; k++)
		{
			s = va_arg(ap, char *);
			argv[k+1] = s;
		}

		argv[k+1] = NULL;
		status = do_cmd(cmd, argv, dowait);
		va_end(ap);

		return status;
	}



	void kill_processes(void)
	{
#if 0 //by cairui

		printf("upgrade: killing tasks...\n");

		kill(1, SIGTSTP);		/* Stop init from reforking tasks */
		kill(1, SIGSTOP);		
		kill(2, SIGSTOP);		
		kill(3, SIGSTOP);		
		kill(4, SIGSTOP);		
		kill(5, SIGSTOP);		
		kill(6, SIGSTOP);		
		kill(7, SIGSTOP);		
		//atexit(restartinit);		/* If exit prematurely, restart init */
#if 0
		sync();
#endif // temporary! by cairui
		signal(SIGTERM,SIG_IGN);	/* Don't kill ourselves... */
#if 0
		setpgrp(); 			/* Don't let our parent kill us */
#endif //temporary! by cairui
		sleep(1);
		signal(SIGHUP, SIG_IGN);	/* Don't die if our parent dies due to
						 * a closed controlling terminal */
#endif	
	}


	static int daemonKilled = 0;
	void killDaemon(int wait)
	{
		if (daemonKilled)
			return;

		daemonKilled = 1;

		system("echo 1,0 > /proc/br_mCastFastFwd ");
#if 0	

		system("killall -9 sleep 2> /dev/null");
		//system("killsh.sh");	// kill all running script	
		system("killall -9 routed 2> /dev/null");

		system("killall -9 pppoe 2> /dev/null");
		system("killall -9 pppd 2> /dev/null");
		system("killall -9 pptp 2> /dev/null");
		system("killall -9 dnrd 2> /dev/null");
		system("killall -9 ntpclient 2> /dev/null");
		system("killall -9 miniigd 2> /dev/null");
		system("killall -9 lld2d 2> /dev/null");
		system("killall -9 l2tpd 2> /dev/null");	
		system("killall -9 udhcpc 2> /dev/null");	
		system("killall -9 udhcpd 2> /dev/null");	
		system("killall -9 reload 2> /dev/null");		
		system("killall -9 iapp 2> /dev/null");	
		system("killall -9 wscd 2> /dev/null");
		system("killall -9 mini_upnpd 2> /dev/null");
		system("killall -9 iwcontrol 2> /dev/null");
		system("killall -9 auth 2> /dev/null");
		system("killall -9 disc_server 2> /dev/null");
		system("killall -9 igmpproxy 2> /dev/null");

		system("killall -9 syslogd 2> /dev/null");
		system("killall -9 klogd 2> /dev/null");
		system("killall -9 ntfs-3g 2> /dev/null");
		system("killall -9 smbd 2> /dev/null");
		system("killall -9 boa 2> /dev/null");
		// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
		system("killall -9 snmpd 2> /dev/null");
		system("killall -9 solar_monitor 2> /dev/null");
		system("killall -9 solar 2> /dev/null");
		system("killall -9 dns_task 2> /dev/null");
		system("killall -9 ivrserver 2> /dev/null");
		system("killall -9 fwupdate 2> /dev/null");
		system("killall -9 netlink 2> /dev/null");
#endif
#ifdef CONFIG_IPV6
		system("killall -9 mldproxy 2> /dev/null");
		system("killall -9 dhcp6c 2> /dev/null");
		system("killall -9 dhcp6s 2> /dev/null");
		system("killall -9 radvd 2> /dev/null");
		system("killall -9 ecmh 2> /dev/null");
		system("killall -9 ripng 2> /dev/null");
		system("killall -9 zebra 2> /dev/null");	
		system("killall -9 dnsmasq 2> /dev/null");
#endif
#ifdef CONFIG_SNMP
		system("killall -9 snmpd 2> /dev/null");
#endif
		sleep(1);
		sleep(1);
#else
#if 0
		// by cairui
		va_cmd( "/bin/killall", 2, 1, "-9","sleep");
		va_cmd( "/bin/killall", 2, 1, "-9","routed");
		va_cmd("/bin/killall", 2, 1, "-9","pppoe");
		va_cmd("/bin/killall", 2, 1, "-9","pppd ");
		va_cmd("/bin/killall", 2, 1, "-9","pptp");
		va_cmd("/bin/killall", 2, 1, "-9","dnrd");
		va_cmd("/bin/killall", 2, 1, "-9","ntpclient");
		va_cmd("/bin/killall", 2, 1, "-9","miniigd");
		va_cmd("/bin/killall", 2, 1, "-9","lld2d");
		va_cmd("/bin/killall", 2, 1, "-9","l2tpd");	
		va_cmd("/bin/killall", 2, 1, "-9","udhcpc");	
		va_cmd("/bin/killall", 2, 1, "-9","udhcpd");	
		va_cmd("/bin/killall", 2, 1, "-9","reload");		
		va_cmd("/bin/killall", 2, 1, "-9","iapp");	
		va_cmd("/bin/killall", 2, 1, "-9","wscd");
		va_cmd("/bin/killall", 2, 1, "-9","mini_upnpd");
		va_cmd("/bin/killall", 2, 1, "-9","iwcontrol");
		va_cmd("/bin/killall", 2, 1, "-9","auth");
		va_cmd("/bin/killall", 2, 1, "-9","disc_server");
		va_cmd("/bin/killall", 2, 1, "-9","igmpproxy");
		va_cmd("/bin/killall", 2, 1, "-9","syslogd");
		va_cmd("/bin/killall", 2, 1, "-9","klogd");
		va_cmd("/bin/killall", 2, 1, "-9","ntfs-3g");
		va_cmd("/bin/killall", 2, 1, "-9","smbd");
		va_cmd("/bin/killall", 2, 1, "-9","boa");
		va_cmd("/bin/killall", 2, 1, "-9","timelycheck");

#ifdef VOIP_SUPPORT
		va_cmd("/bin/killall", 2, 1, "-9","snmpd");
		va_cmd("/bin/killall", 2, 1, "-9","solar_monitor");
		va_cmd("/bin/killall", 2, 1, "-9","solar");
		va_cmd("/bin/killall", 2, 1, "-9","dns_task");
		va_cmd("/bin/killall", 2, 1, "-9","ivrserver");
		va_cmd("/bin/killall", 2, 1, "-9","fwupdate");
		va_cmd("/bin/killall", 2, 1, "-9","netlink");
#endif	
#ifdef CONFIG_IPV6
		va_cmd("/bin/killall", 2, 1, "-9","mldproxy");
		va_cmd("/bin/killall", 2, 1, "-9","dhcp6c");
		va_cmd("/bin/killall", 2, 1, "-9","dhcp6s");
		va_cmd("/bin/killall", 2, 1, "-9","radvd");
		va_cmd("/bin/killall", 2, 1, "-9","ecmh");
		va_cmd("/bin/killall", 2, 1, "-9","ripng");
		va_cmd("/bin/killall", 2, 1, "-9","zebra");	
		va_cmd("/bin/killall", 2, 1, "-9","dnsmasq");
#endif	
#ifdef CONFIG_SNMP	
		va_cmd("/bin/killall", 2, 1, "-9","snmpd");
#endif
#else
		va_cmd( "kill", 2, 1, "-9","sleep");
		va_cmd( "kill", 2, 1, "-9","routed");
		va_cmd("kill", 2, 1, "-9","pppoe");
		va_cmd("kill", 2, 1, "-9","pppd ");
		va_cmd("kill", 2, 1, "-9","pptp");
		va_cmd("kill", 2, 1, "-9","dnrd");
		va_cmd("kill", 2, 1, "-9","ntpclient");
		va_cmd("kill", 2, 1, "-9","miniigd");
		va_cmd("kill", 2, 1, "-9","lld2d");
		va_cmd("kill", 2, 1, "-9","l2tpd");	
		va_cmd("kill", 2, 1, "-9","udhcpc");	
		va_cmd("kill", 2, 1, "-9","udhcpd");	
		va_cmd("kill", 2, 1, "-9","reload");		
		va_cmd("kill", 2, 1, "-9","iapp");	
		va_cmd("kill", 2, 1, "-9","wscd");
		va_cmd("kill", 2, 1, "-9","mini_upnpd");
		va_cmd("kill", 2, 1, "-9","iwcontrol");
		va_cmd("kill", 2, 1, "-9","auth");
		va_cmd("kill", 2, 1, "-9","disc_server");
		va_cmd("kill", 2, 1, "-9","igmpproxy");
		va_cmd("kill", 2, 1, "-9","syslogd");
		va_cmd("kill", 2, 1, "-9","klogd");
		va_cmd("kill", 2, 1, "-9","ntfs-3g");
		va_cmd("kill", 2, 1, "-9","smbd");
		va_cmd("kill", 2, 1, "-9","boa");
		va_cmd("kill", 2, 1, "-9","timelycheck");

#ifdef VOIP_SUPPORT
		va_cmd("kill", 2, 1, "-9","snmpd");
		va_cmd("kill", 2, 1, "-9","solar_monitor");
		va_cmd("kill", 2, 1, "-9","solar");
		va_cmd("kill", 2, 1, "-9","dns_task");
		va_cmd("kill", 2, 1, "-9","ivrserver");
		va_cmd("kill", 2, 1, "-9","fwupdate");
		va_cmd("kill", 2, 1, "-9","netlink");
#endif	
#ifdef CONFIG_IPV6
		va_cmd("kill", 2, 1, "-9","mldproxy");
		va_cmd("kill", 2, 1, "-9","dhcp6c");
		va_cmd("kill", 2, 1, "-9","dhcp6s");
		va_cmd("kill", 2, 1, "-9","radvd");
		va_cmd("kill", 2, 1, "-9","ecmh");
		va_cmd("kill", 2, 1, "-9","ripng");
		va_cmd("kill", 2, 1, "-9","zebra");	
		va_cmd("kill", 2, 1, "-9","dnsmasq");
#endif	
#ifdef CONFIG_SNMP	
		va_cmd("kill", 2, 1, "-9","snmpd");
#endif

#endif
#endif

	}

	int cmd_reboot()
	{
#if 0
		system("reboot");
		exit(0);
#else
		//printf("***%s:%s:%d***\n",__FILE__,__FUNCTION__,__LINE__);
		do_reset(1);
#endif
		return 0;
	}

	/* send the request to solar */
#if 0 // Vincent removed. Multiple definition in rtk_voip/tr104/cwmp_main_tr104.c
	void cwmpSendRequestToSolar(void){

	}

	/*open the connection from solar to cwmpclient*/
	void cwmp_solarOpen( void )
	{

	}

	/*close the connection from solar to cwmpclient*/
	void cwmp_solarClose(void){

	}
#endif


	const char *dhcp_mode[] = {
		"None", "DHCP Relay", "DHCP Server"
	};
	const char *wlan_band[] = {
		0, "2.4 GHz (B)", "2.4 GHz (G)", "2.4 GHz (B+G)", 0
			, 0, 0, 0, "2.4 GHz (N)", 0, "2.4 GHz (G+N)", "2.4 GHz (B+G+N)", 0
	};

	const char *wlan_mode[] = {
		//"AP", "Client", "AP+WDS"
		"AP", "Client", "WDS", "AP+WDS"
	};

	const char *wlan_rate[] = {
		"1M", "2M", "5.5M", "11M", "6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M"
			, "MCS0", "MCS1", "MCS2", "MCS3", "MCS4", "MCS5", "MCS6", "MCS7", "MCS8", "MCS9", "MCS10", "MCS11", "MCS12", "MCS13", "MCS14", "MCS15"
	};

	const char *wlan_auth[] = {
		"Open", "Shared", "Auto"
	};

	//modified  by xl_yue

	const char *wlan_preamble[] = {
		"Long", "Short"
	};


	const char *wlan_encrypt[] = {
		"None",
		"WEP",
		"WPA(TKIP)",
		"WPA(AES)",
		"WPA2(AES)",
		"WPA2(TKIP)",
		"WPA2 Mixed",
		"",
	};

	const char *wlan_pskfmt[] = {
		"Passphrase", "Hex"
	};

	const char *wlan_wepkeylen[] = {
		"Disable", "64-bit", "128-bit"
	};

	const char *wlan_wepkeyfmt[] = {
		"ASCII", "Hex"
	};

	const char *wlan_Cipher[] = {
		"TKIP", "AES", "Both"
	};

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length if match_length is greater than this */
	static unsigned char *text_buf;	/* ring buffer of size N, with extra F-1 bytes to facilitate string comparison */
#define LZSS_TYPE	unsigned short
#define NIL			N	/* index for root of binary search trees */
	struct lzss_buffer {
		unsigned char	text_buf[N + F - 1];
		LZSS_TYPE	lson[N + 1];
		LZSS_TYPE	rson[N + 257];
		LZSS_TYPE	dad[N + 1];
	};
	static LZSS_TYPE		match_position, match_length;  /* of longest match.  These are set by the InsertNode() procedure. */
	static LZSS_TYPE		*lson, *rson, *dad;  /* left & right children & parents -- These constitute binary search trees. */

#if 0
	void InsertNode(LZSS_TYPE r)
		/* Inserts string of length F, text_buf[r..r+F-1], into one of the
		   trees (text_buf[r]'th tree) and returns the longest-match position
		   and length via the global variables match_position and match_length.
		   If match_length = F, then removes the old node in favor of the new
		   one, because the old one will be deleted sooner.
		   Note r plays double role, as tree node and position in buffer. */
	{
		LZSS_TYPE  i, p, cmp;
		unsigned char  *key;

		cmp = 1;
		key = &text_buf[r];
		p = N + 1 + key[0];
		rson[r] = lson[r] = NIL;
		match_length = 0;
		while(1) {
			if (cmp >= 0) {
				if (rson[p] != NIL)
					p = rson[p];
				else {
					rson[p] = r;
					dad[r] = p;
					return;
				}
			} else {
				if (lson[p] != NIL)
					p = lson[p];
				else {
					lson[p] = r;
					dad[r] = p;
					return;
				}
			}
			for (i = 1; i < F; i++)
				if ((cmp = key[i] - text_buf[p + i]) != 0)
					break;
			if (i > match_length) {
				match_position = p;
				if ((match_length = i) >= F)
					break;
			}
		}
		dad[r] = dad[p];
		lson[r] = lson[p];
		rson[r] = rson[p];
		dad[lson[p]] = r;
		dad[rson[p]] = r;
		if (rson[dad[p]] == p)
			rson[dad[p]] = r;
		else
			lson[dad[p]] = r;
		dad[p] = NIL;  /* remove p */
	}

	void InitTree(void)  /* initialize trees */
	{
		int  i;

		/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
		   left children of node i.  These nodes need not be initialized.
		   Also, dad[i] is the parent of node i.  These are initialized to
		   NIL (= N), which stands for 'not used.'
		   For i = 0 to 255, rson[N + i + 1] is the root of the tree
		   for strings that begin with character i.  These are initialized
		   to NIL.  Note there are 256 trees. */

		for (i = N + 1; i <= N + 256; i++)
			rson[i] = NIL;
		for (i = 0; i < N; i++)
			dad[i] = NIL;
	}

	void DeleteNode(LZSS_TYPE p)  /* deletes node p from tree */
	{
		LZSS_TYPE  q;

		if (dad[p] == NIL)
			return;  /* not in tree */
		if (rson[p] == NIL)
			q = lson[p];
		else if (lson[p] == NIL)
			q = rson[p];
		else {
			q = lson[p];
			if (rson[q] != NIL) {
				do {
					q = rson[q];
				} while (rson[q] != NIL);
				rson[dad[q]] = lson[q];
				dad[lson[q]] = dad[q];
				lson[q] = lson[p];
				dad[lson[p]] = q;
			}
			rson[q] = rson[p];
			dad[rson[p]] = q;
		}
		dad[q] = dad[p];
		if (rson[dad[p]] == p)
			rson[dad[p]] = q;
		else
			lson[dad[p]] = q;
		dad[p] = NIL;
	}

	int Encode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)
	{
#if defined(CHEAT_COMPRESS_MIB_SETTING)
		memcpy(ucOutput, ucInput, inLen);
		return inLen;
#else	
		LZSS_TYPE  i, len, r, s, last_match_length, code_buf_ptr;
		unsigned char c;
		unsigned char  code_buf[17], mask;
		unsigned int ulPos=0;
		int enIdx=0;

		struct lzss_buffer *lzssbuf;

		if (0 != (lzssbuf = malloc(sizeof(struct lzss_buffer)))) {
			memset(lzssbuf, 0, sizeof(struct lzss_buffer));
			text_buf = lzssbuf->text_buf;
			rson = lzssbuf->rson;
			lson = lzssbuf->lson;
			dad = lzssbuf->dad;
		} else {
			return 0;
		}

		InitTree();  /* initialize trees */
		code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
				     code_buf[0] works as eight flags, "1" representing that the unit
				     is an unencoded letter (1 byte), "0" a position-and-length pair
				     (2 bytes).  Thus, eight units require at most 16 bytes of code. */
		code_buf_ptr = mask = 1;
		s = 0;
		r = N - F;
		for (i = s; i < r; i++)
			text_buf[i] = ' ';  /* Clear the buffer with
					       any character that will appear often. */

		for (len = 0; (len < F) && ulPos < inLen; len++)
			text_buf[r + len] = ucInput[ulPos++];  /* Read F bytes into the last F bytes of the buffer */

		//if ((textsize = len) == 0) return;  /* text of size zero */
		if (len == 0) {
			enIdx = 0;
			goto finished;
		}

		for (i = 1; i <= F; i++)
			InsertNode(r - i);  /* Insert the F strings,
					       each of which begins with one or more 'space' characters.  Note
					       the order in which these strings are inserted.  This way,
					       degenerate trees will be less likely to occur. */
		InsertNode(r);  /* Finally, insert the whole string just read.  The
				   global variables match_length and match_position are set. */
		do {
			if (match_length > len) match_length = len;  /* match_length
									may be spuriously long near the end of text. */
			if (match_length <= THRESHOLD) {
				match_length = 1;  /* Not long enough match.  Send one byte. */
				code_buf[0] |= mask;  /* 'send one byte' flag */
				code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
			} else {
				code_buf[code_buf_ptr++] = (unsigned char) match_position;
				code_buf[code_buf_ptr++] = (unsigned char)
					(((match_position >> 4) & 0xf0)
					 | (match_length - (THRESHOLD + 1)));  /* Send position and
										  length pair. Note match_length > THRESHOLD. */
			}
			if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
				for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
					ucOutput[enIdx++]=code_buf[i];
				//codesize += code_buf_ptr;
				code_buf[0] = 0;  code_buf_ptr = mask = 1;
			}
			last_match_length = match_length;

			for (i = 0; i< last_match_length && 
					ulPos < inLen; i++){
				c = ucInput[ulPos++];
				DeleteNode(s);		/* Delete old strings and */
				text_buf[s] = c;	/* read new bytes */
				if (s < F - 1)
					text_buf[s + N] = c;  /* If the position is near the end of buffer, extend the buffer to make string comparison easier. */
				s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the position
				   modulo N. */
				InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
			}

			while (i++ < last_match_length) {	/* After the end of text, */
				DeleteNode(s);					/* no need to read, but */
				s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				if (--len) InsertNode(r);		/* buffer may not be empty. */
			}
		} while (len > 0);	/* until length of string to be processed is zero */
		if (code_buf_ptr > 1) {		/* Send remaining code. */
			for (i = 0; i < code_buf_ptr; i++) 
				ucOutput[enIdx++]=code_buf[i];
			//codesize += code_buf_ptr;
		}
finished:
		free(lzssbuf);
		return enIdx;
#endif //#if defined(CHEAT_COMPRESS_MIB_SETTING)
	}

	int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)	/* Just the reverse of Encode(). */
	{
#if defined(CHEAT_COMPRESS_MIB_SETTING)
		memcpy(ucOutput, ucInput, inLen);
		return inLen;
#else	
		int  i, j, k, r, c;
		unsigned int  flags;
		unsigned int ulPos=0;
		unsigned int ulExpLen=0;

		if ((text_buf = malloc( N + F - 1 )) == 0) {
			//fprintf(stderr, "fail to get mem %s:%d\n", __FUNCTION__, __LINE__);
			return 0;
		}

		for (i = 0; i < N - F; i++)
			text_buf[i] = ' ';
		r = N - F;
		flags = 0;
		while(1) {
			if (((flags >>= 1) & 256) == 0) {
				c = ucInput[ulPos++];
				if (ulPos>inLen)
					break;
				flags = c | 0xff00;		/* uses higher byte cleverly */
			}							/* to count eight */
			if (flags & 1) {
				c = ucInput[ulPos++];
				if ( ulPos > inLen )
					break;
				ucOutput[ulExpLen++] = c;
				text_buf[r++] = c;
				r &= (N - 1);
			} else {
				i = ucInput[ulPos++];
				if ( ulPos > inLen ) break;
				j = ucInput[ulPos++];
				if ( ulPos > inLen ) break;

				i |= ((j & 0xf0) << 4);
				j = (j & 0x0f) + THRESHOLD;
				for (k = 0; k <= j; k++) {
					c = text_buf[(i + k) & (N - 1)];
					ucOutput[ulExpLen++] = c;
					text_buf[r++] = c;
					r &= (N - 1);
				}
			}
		}

		free(text_buf);
		return ulExpLen;
#endif //#if defined(CHEAT_COMPRESS_MIB_SETTING)
	}
#endif // BY cairui, they are included in apmib.c

	int getWlanMib(int wlanRootIndex, int wlanValIndex, int id, void *value)
	{
		int root_old, val_old;

		root_old = wlan_idx;
		val_old = vwlan_idx;

		wlan_idx = wlanRootIndex;
		vwlan_idx = wlanValIndex;

		apmib_get(id, (void *)value);

		wlan_idx = root_old;
		vwlan_idx = val_old;

		return 0;
	}

	int setWlanMib(int wlanRootIndex, int wlanValIndex, int id, void *value)
	{
		int root_old, val_old;

		root_old = wlan_idx;
		val_old = vwlan_idx;

		wlan_idx = wlanRootIndex;
		vwlan_idx = wlanValIndex;

		apmib_set(id, (void *)value);

		wlan_idx = root_old;
		vwlan_idx = val_old;

		return 0;
	}

	char wlan_ifname[20]={0};
	int getWlanBssInfo(int wlanRootIndex, int wlanValIndex, void *bss)
	{
		int root_old, val_old;

		root_old = wlan_idx;
		val_old = vwlan_idx;

		wlan_idx = wlanRootIndex;
		vwlan_idx = wlanValIndex;

		if (vwlan_idx == 0)
		{
			sprintf(wlan_ifname, "wlan%d", wlan_idx);
		}
		else if (vwlan_idx > 0 && vwlan_idx < 5) //repeater is 5; wds is 6
		{
			sprintf(wlan_ifname, "wlan%d-va%d", wlan_idx, vwlan_idx-1);
		}
		else if (vwlan_idx == 5) //repeater is 5; wds is 6
		{
			sprintf(wlan_ifname, "wlan%d-vxd", wlan_idx);
		}
		else if (vwlan_idx == 6) //repeater is 5; wds is 6
		{
			sprintf(wlan_ifname, "wlan%d-wds", wlan_idx);
		}

		if( cwmp_getWlBssInfo(wlan_ifname, bss) !=0 )
		{
			wlan_idx = root_old;
			vwlan_idx = val_old;
			return -1;		
		}

		wlan_idx = root_old;
		vwlan_idx = val_old;

		return 0;
	}


	static int updateConfigIntoFlash(unsigned char *data, int total_len, int *pType, int *pStatus)
	{
		//printf("<%s:%d>\n",__FUNCTION__,__LINE__);
		int len=0, status=1, type=0, ver, force;
		PARAM_HEADER_Tp pHeader;
#ifdef COMPRESS_MIB_SETTING
		COMPRESS_MIB_HEADER_Tp pCompHeader;
		unsigned char *expFile=NULL;
		unsigned int expandLen=0;
		int complen=0;
#endif
		char *ptr;

		//printf("<%s:%d>, enter while\n",__FUNCTION__,__LINE__);
		do {
#ifdef COMPRESS_MIB_SETTING
			pCompHeader =(COMPRESS_MIB_HEADER_Tp)&data[complen];
#ifdef _LITTLE_ENDIAN_
			pCompHeader->compRate = WORD_SWAP(pCompHeader->compRate);
			pCompHeader->compLen = DWORD_SWAP(pCompHeader->compLen);
#endif
			/*decompress and get the tag*/
			expFile=malloc(pCompHeader->compLen*pCompHeader->compRate);
			if(NULL==expFile)
			{
				//printf("malloc for expFile error!!\n");
				return 0;
			}
			expandLen = Decode(data+complen+sizeof(COMPRESS_MIB_HEADER_T), pCompHeader->compLen, expFile);
			pHeader = (PARAM_HEADER_Tp)expFile;
#else
			pHeader = (PARAM_HEADER_Tp)&data[len];
#endif

			//printf("<%s:%d>, 1\n",__FUNCTION__,__LINE__);
#ifdef _LITTLE_ENDIAN_
			pHeader->len = WORD_SWAP(pHeader->len);
#endif
			len += sizeof(PARAM_HEADER_T);

			if ( sscanf(&pHeader->signature[TAG_LEN], "%02d", &ver) != 1)
				ver = -1;

			//printf("<%s:%d>, len=%d\n",__FUNCTION__,__LINE__, len);
			force = -1;
			if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) )
				force = 1; // update
			else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN))
				force = 2; // force
			else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN))
				force = 0; // upgrade

			//printf("<%s:%d>, force=%d\n",__FUNCTION__,__LINE__, force);
			if ( force >= 0 ) {
#if 0
				if ( !force && (ver < CURRENT_SETTING_VER || // version is less than current
							(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
						status = 0;
						break;
						}
#endif

#ifdef COMPRESS_MIB_SETTING
						ptr = expFile+sizeof(PARAM_HEADER_T);
#else
						ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
						DECODE_DATA(ptr, pHeader->len);
#endif
						if ( !CHECKSUM_OK(ptr, pHeader->len)) {
						status = 0;
						break;
						}
#ifdef _LITTLE_ENDIAN_
						swap_mib_word_value((APMIB_Tp)ptr);
#endif

						// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
						flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMib->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
						apmib_updateFlash(CURRENT_SETTING, &data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
						apmib_updateFlash(CURRENT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
						complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
						if(expFile)
						{
							free(expFile);
							expFile=NULL;
						}
#else
						len += pHeader->len;
#endif
						type |= CURRENT_SETTING;
						continue;
			}


			if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) )
				force = 1;	// update
			else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) )
				force = 2;	// force
			else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) )
				force = 0;	// upgrade

			if ( force >= 0 ) {
#if 0
				if ( (ver < DEFAULT_SETTING_VER) || // version is less than current
						(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
					status = 0;
					break;
				}
#endif

#ifdef COMPRESS_MIB_SETTING
				ptr = expFile+sizeof(PARAM_HEADER_T);
#else
				ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
				DECODE_DATA(ptr, pHeader->len);
#endif
				if ( !CHECKSUM_OK(ptr, pHeader->len)) {
					status = 0;
					break;
				}

#ifdef _LITTLE_ENDIAN_
				swap_mib_word_value((APMIB_Tp)ptr);
#endif

				// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
				flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMibDef->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
				apmib_updateFlash(DEFAULT_SETTING, &data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
				apmib_updateFlash(DEFAULT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
				complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
				if(expFile)
				{
					free(expFile);
					expFile=NULL;
				}	
#else
				len += pHeader->len;
#endif
				type |= DEFAULT_SETTING;
				continue;
			}

			if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_TAG, TAG_LEN) )
				force = 1;	// update
			else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) )
				force = 2;	// force
			else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) )
				force = 0;	// upgrade

			if ( force >= 0 ) {
#if 0
				if ( (ver < HW_SETTING_VER) || // version is less than current
						(pHeader->len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
					status = 0;
					break;
				}
#endif
#ifdef COMPRESS_MIB_SETTING
				ptr = expFile+sizeof(PARAM_HEADER_T);
#else
				ptr = &data[len];
#endif


#ifdef COMPRESS_MIB_SETTING
#else
				DECODE_DATA(ptr, pHeader->len);
#endif
				if ( !CHECKSUM_OK(ptr, pHeader->len)) {
					status = 0;
					break;
				}
#ifdef COMPRESS_MIB_SETTING
				apmib_updateFlash(HW_SETTING, &data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
				apmib_updateFlash(HW_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
				complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
				if(expFile)
				{
					free(expFile);
					expFile=NULL;
				}
#else
				len += pHeader->len;
#endif

				type |= HW_SETTING;
				continue;
			}
		}
#ifdef COMPRESS_MIB_SETTING	
		while (complen < total_len);
#else
		while (len < total_len);
#endif
		if(expFile)
		{
			free(expFile);
			expFile=NULL;
		}

		*pType = type;
		*pStatus = status;
#ifdef COMPRESS_MIB_SETTING	
		return complen;
#else
		return len;
#endif
	}

	int doUpdateConfigIntoFlash(unsigned char *data, int total_len, int *pType, int *pStatus)
	{
		//printf("<%s %d>, data=%s, total_len=%d\n", __FUNCTION__,__LINE__, data, total_len);
		int len=0, status=1, type=0, ver, force;
		PARAM_HEADER_Tp pHeader;
#ifdef COMPRESS_MIB_SETTING
		COMPRESS_MIB_HEADER_Tp pCompHeader;
		unsigned char *expFile=NULL;
		unsigned int expandLen=0;
		int complen=0;
#endif
		char *ptr;
		unsigned char isValidfw = 0;
		//printf("total_len=%d\n",total_len);
		do {
			if (
#ifdef COMPRESS_MIB_SETTING
					memcmp(&data[complen], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) &&
					memcmp(&data[complen], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) &&
					memcmp(&data[complen], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else
					memcmp(&data[len], CURRENT_SETTING_HEADER_TAG, TAG_LEN) &&
					memcmp(&data[len], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) &&
					memcmp(&data[len], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) &&
					memcmp(&data[len], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) &&
					memcmp(&data[len], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) &&
					memcmp(&data[len], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) &&
					memcmp(&data[len], HW_SETTING_HEADER_TAG, TAG_LEN) &&
					memcmp(&data[len], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) &&
					memcmp(&data[len], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) 
#endif
			   ) {
				if (isValidfw == 1)
					break;
			}

#ifdef COMPRESS_MIB_SETTING
			//printf("<%s %d>\n", __FUNCTION__, __LINE__);
			pCompHeader =(COMPRESS_MIB_HEADER_Tp)&data[complen];
#ifdef _LITTLE_ENDIAN_
			pCompHeader->compRate = WORD_SWAP(pCompHeader->compRate);
			pCompHeader->compLen = DWORD_SWAP(pCompHeader->compLen);
#endif
			/*decompress and get the tag*/
			expFile=malloc(pCompHeader->compLen*pCompHeader->compRate);
			if (NULL==expFile) {
				//printf("malloc for expFile error!!\n");
				return 0;
			}
			//printf("<%s %d>\n", __FUNCTION__, __LINE__);
			expandLen = Decode(data+complen+sizeof(COMPRESS_MIB_HEADER_T), pCompHeader->compLen, expFile);
			pHeader = (PARAM_HEADER_Tp)expFile;
			//printf("<%s %d>, expandLen=%d\n", __FUNCTION__, __LINE__, expandLen);
#else
			pHeader = (PARAM_HEADER_Tp)&data[len];
#endif

#ifdef _LITTLE_ENDIAN_
			pHeader->len = WORD_SWAP(pHeader->len);
#endif
			len += sizeof(PARAM_HEADER_T);

			//printf("<%s %d>\n", __FUNCTION__, __LINE__);
			if ( sscanf((char *)&pHeader->signature[TAG_LEN], "%02d", &ver) != 1)
				ver = -1;

			force = -1;
			if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) ) {
				isValidfw = 1;
				force = 1; // update
			}
			else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN)) {
				isValidfw = 1;
				force = 2; // force
			}
			else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN)) {
				isValidfw = 1;
				force = 0; // upgrade
			}

			//printf("<%s %d>, 1: force=%d\n", __FUNCTION__, __LINE__, force);
			if ( force >= 0 ) {
#if 0
				if ( !force && (ver < CURRENT_SETTING_VER || // version is less than current
							(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
						status = 0;
						break;
						}
#endif

#ifdef COMPRESS_MIB_SETTING
						ptr = (char *)(expFile+sizeof(PARAM_HEADER_T));
#else
						ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
						DECODE_DATA(ptr, pHeader->len);
#endif
						if ( !CHECKSUM_OK((unsigned char *)ptr, pHeader->len)) {
						status = 0;
						break;
						}
#ifdef _LITTLE_ENDIAN_
						swap_mib_word_value((APMIB_Tp)ptr);
#endif

						// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
						flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMib->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
						//printf("%s:call  apmib_updateFlash CURRENT_SETTING\n", __FUNCTION__);
						apmib_updateFlash(CURRENT_SETTING, (char *)&data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
						apmib_updateFlash(CURRENT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
						complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
						if (expFile) {
							free(expFile);
							expFile=NULL;
						}
#else
						len += pHeader->len;
#endif
						type |= CURRENT_SETTING;
						continue;
			}


			if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ) {
				isValidfw = 1;
				force = 1;	// update
			}
			else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ) {
				isValidfw = 1;
				force = 2;	// force
			}
			else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ) {
				isValidfw = 1;
				force = 0;	// upgrade
			}

			//printf("<%s %d>, 2: force=%d\n", __FUNCTION__, __LINE__, force);
			if ( force >= 0 ) {
#if 0
				if ( (ver < DEFAULT_SETTING_VER) || // version is less than current
						(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
					status = 0;
					break;
				}
#endif

#ifdef COMPRESS_MIB_SETTING
				ptr = (char *)(expFile+sizeof(PARAM_HEADER_T));
#else
				ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
				DECODE_DATA(ptr, pHeader->len);
#endif
				if ( !CHECKSUM_OK((unsigned char *)ptr, pHeader->len)) {
					status = 0;
					break;
				}

#ifdef _LITTLE_ENDIAN_
				swap_mib_word_value((APMIB_Tp)ptr);
#endif

				// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
				flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMibDef->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
				apmib_updateFlash(DEFAULT_SETTING, (char *)&data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
				apmib_updateFlash(DEFAULT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
				complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
				if (expFile) {
					free(expFile);
					expFile=NULL;
				}	
#else
				len += pHeader->len;
#endif
				type |= DEFAULT_SETTING;
				continue;
			}

			if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_TAG, TAG_LEN) ) {
				isValidfw = 1;
				force = 1;	// update
			}
			else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ) {
				isValidfw = 1;
				force = 2;	// force
			}
			else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ) {
				isValidfw = 1;
				force = 0;	// upgrade
			}

			//printf("<%s %d> 3: force=%d\n", __FUNCTION__, __LINE__, force);
			if ( force >= 0 ) {
#if 0
				if ( (ver < HW_SETTING_VER) || // version is less than current
						(pHeader->len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
					status = 0;
					break;
				}
#endif
#ifdef COMPRESS_MIB_SETTING
				ptr = (char *)(expFile+sizeof(PARAM_HEADER_T));
#else
				ptr = &data[len];
#endif


#ifdef COMPRESS_MIB_SETTING
#else
				DECODE_DATA(ptr, pHeader->len);
#endif
				if ( !CHECKSUM_OK((unsigned char *)ptr, pHeader->len)) {
					status = 0;
					break;
				}
#ifdef COMPRESS_MIB_SETTING
				apmib_updateFlash(HW_SETTING, (char *)&data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
				apmib_updateFlash(HW_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
				complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
				if (expFile) {
					free(expFile);
					expFile=NULL;
				}
#else
				len += pHeader->len;
#endif

				type |= HW_SETTING;
				continue;
			}
		}
#ifdef COMPRESS_MIB_SETTING	
		while (complen < total_len); // end do-while

		if (expFile) {
			free(expFile);
			expFile=NULL;
		}
#else
		while (len < total_len);
#endif

		*pType = type;
		*pStatus = status;
		if(type != 0 && status != 0){
			apmib_reinit();
		}
#ifdef COMPRESS_MIB_SETTING	
		//printf("<%s %d>, complen=%d\n", __FUNCTION__, __LINE__, complen);
		return complen;
#else
		return len;
#endif
	} //end doUpdateConfigIntoFlash

	char *get_ifname(char *name, char *p)
	{
		while (isspace(*p))
			p++;
		while (*p) {
			if (isspace(*p))
				break;
			if (*p == ':') {	/* could be an alias */
				char *dot = p, *dotname = name;

				*name++ = *p++;
				while (isdigit(*p))
					*name++ = *p++;
				if (*p != ':') {	/* it wasn't, backup */
					p = dot;
					name = dotname;
				}
				if (*p == '\0')
					return NULL;
				p++;
				break;
			}
			*name++ = *p++;
		}
		*name++ = '\0';
		return p;
	}
#define _PATH_PROCNET_DEV "/proc/net/dev"
	int cwmp_shutdown_netdev(void)
	{
		FILE *fh;
		char buf[512];
		char *s, name[16], tmp_str[16];
		int iface_num=0;
		fh = fopen(_PATH_PROCNET_DEV, "r");
		if (!fh) {
			return 0;
		}
		fgets(buf, sizeof buf, fh);	/* eat line */
		fgets(buf, sizeof buf, fh);
		while (fgets(buf, sizeof buf, fh)) {
			s = get_ifname(name, buf);
			if(strstr(name, "eth") || strstr(name, "wlan") || strstr(name, "ppp")|| strstr(name, "sit")){
				iface_num++;
				va_cmd( "ifconfig", 2, 1, name,"down");
			}
		}

		fclose(fh);
		return iface_num;
	}
	int doFirmwareUpgrade(char *upload_data, int upload_len, int is_root, char *buffer)
	{
		int head_offset=0 ;
		int isIncludeRoot=0;
		int		 len;
		int          locWrite;
		int          numLeft;
		int          numWrite;
		IMG_HEADER_Tp pHeader;
		int flag=0, startAddr=-1, startAddrWeb=-1;
		int update_fw=0, update_cfg=0;
#ifdef __mips__
		int fh;
#else
		FILE *       fp;
		char_t *     bn = NULL;
#endif
		unsigned char cmdBuf[30];
		//printf("<%s:%d>\n",__FUNCTION__, __LINE__);
#if 0
		int *test = (int *)mem_chain_upgrade_mem_str_convert(upload_data, 10);
#endif

		//printf("enter <%s %d>: &upload_data=%p, upload_len=%d\n", __FUNCTION__, __LINE__, &upload_data, upload_len);
		while(head_offset < upload_len) {
			//printf("<%s:%d>head_offset=%d\n", __FUNCTION__, __LINE__, head_offset);
			locWrite = 0;
#if defined(ECOS_MEM_CHAIN_API)
			//pHeader =(IMG_HEADER_Tp)mem_chain_upgrade_mem_str_convert(&upload_data[head_offset],sizeof(IMG_HEADER_T));     
			//printf("==================USE MEM_CHAIN_API===============\n");
			pHeader =(IMG_HEADER_Tp)mem_chain_upgrade_mem_str_convert(upload_data+head_offset,sizeof(IMG_HEADER_T));     
                        if(pHeader == NULL){
                                //printf("size=%d, do not have enough space to malloc in heap!!\n", sizeof(IMG_HEADER_T));
                                goto ret_upload;
                        } 
#else
			pHeader = (IMG_HEADER_Tp)&upload_data[head_offset];
#endif

			//printf("pHeader=%p\n", pHeader);
			len = pHeader->len;
#ifdef _LITTLE_ENDIAN_
			len  = DWORD_SWAP(len);
#endif    
			numLeft = len + sizeof(IMG_HEADER_T) ;

			//printf("<%s %d>, len=%d, head_offset=%d\n", __FUNCTION__, __LINE__, len, head_offset);
			//printf("FW_HEADER=%s, WEB_HEADER=%s, ROOT_HEADER=%s\n", FW_HEADER, WEB_HEADER, ROOT_HEADER);
			// check header and checksum
#if defined(ECOS_MEM_CHAIN_API)
			if (!mem_chain_upgrade_memcmp(&upload_data[head_offset], FW_HEADER, SIGNATURE_LEN) || !mem_chain_upgrade_memcmp(&upload_data[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN))
#else
			if (!memcmp(&upload_data[head_offset], FW_HEADER, SIGNATURE_LEN) || !memcmp(&upload_data[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN))
#endif
				flag = 1;
#if defined(ECOS_MEM_CHAIN_API)
			else if (!mem_chain_upgrade_memcmp(&upload_data[head_offset], WEB_HEADER, SIGNATURE_LEN))
#else
			else if (!memcmp(&upload_data[head_offset], WEB_HEADER, SIGNATURE_LEN))
#endif
				flag = 2;
#if defined(ECOS_MEM_CHAIN_API)
			else if (!mem_chain_upgrade_memcmp(&upload_data[head_offset], ROOT_HEADER, SIGNATURE_LEN))
#else
			else if (!memcmp(&upload_data[head_offset], ROOT_HEADER, SIGNATURE_LEN))
#endif
			{
				flag = 3;
				isIncludeRoot = 1;
			}
			else if (
#if defined(ECOS_MEM_CHAIN_API)

#ifdef COMPRESS_MIB_SETTING
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else	
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], HW_SETTING_HEADER_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
					!mem_chain_upgrade_memcmp(&upload_data[head_offset], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN)
#endif				

#else //if not defined ECOS_MEM_CHAIN_API

#ifdef COMPRESS_MIB_SETTING
					!memcmp(&upload_data[head_offset], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) ||
					!memcmp(&upload_data[head_offset], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) ||
					!memcmp(&upload_data[head_offset], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else	
					!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
					!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN)
#endif				
#endif
				)
			{ //for config file
#if defined(ECOS_MEM_CHAIN_API)
				goto ret_upload;
#else
				int type, status, cfg_len;

				cfg_len = updateConfigIntoFlash(&upload_data[head_offset], 0, &type, &status);

				if (status == 0 || type == 0) { // checksum error
					strcpy(buffer, "Invalid configuration file!");
					goto ret_upload;
				}
				else { // upload success
					strcpy(buffer, "Update successfully!");
					head_offset += cfg_len;
					update_cfg = 1;
				}    	
				continue;
#endif
			}
			else {
				sprintf(buffer, "Invalid file format [%s-%d]!",__FILE__,__LINE__);
				goto ret_upload;
			}
			//printf("<%s %d>: flag=%d, len=%d\n", __FUNCTION__, __LINE__, flag, len);


			if(len > 0x700000){ //len check by sc_yang
				sprintf(buffer, "Image len exceed max size 0x700000 ! len=0x%x</b><br>", len);
				goto ret_upload;
			}
			if ( (flag == 1) || (flag == 3)) {
				//printf("<%s:%d>flag=%d\n", __FUNCTION__, __LINE__, flag);
#if defined(ECOS_MEM_CHAIN_API)
				if ( !mem_chain_upgrade_fwChecksumOk(&upload_data[sizeof(IMG_HEADER_T)+head_offset], len)) 
#else
				if ( !fwChecksumOk(&upload_data[sizeof(IMG_HEADER_T)+head_offset], len)) 
#endif
				{
					sprintf(buffer, "Image checksum mismatched! len=0x%x, checksum=0x%x</b><br>", len,
							*((unsigned short *)&upload_data[len-2]) );
					goto ret_upload;
				}
			}
			else {
				//printf("<%s:%d>flag=%d\n", __FUNCTION__, __LINE__, flag);
				char *ptr = &upload_data[sizeof(IMG_HEADER_T)+head_offset];
				if ( !CHECKSUM_OK(ptr, len) ) 
				{
					sprintf(buffer, "Image checksum mismatched! len=0x%x</b><br>", len);
					goto ret_upload;
				}
			}
			if(flag == 3)
				fh = open(FLASH_DEVICE_NAME1, O_RDWR);
			else
				fh = open(FLASH_DEVICE_NAME, O_RDWR);

			//printf("<%s %d>: flag=%d, fh=%d\n", __FUNCTION__, __LINE__, flag, fh);
#if 0
			if ( fh == -1 ) {
				strcpy(buffer, "File open failed!");
				goto ret_upload;
			} else 
#endif
			{ 
#ifdef __mips__
				if (flag == 1) {
					if ( startAddr == -1){
						//startAddr = CODE_IMAGE_OFFSET;
						startAddr = pHeader->burnAddr ;
#ifdef _LITTLE_ENDIAN_
						startAddr = DWORD_SWAP(startAddr);
#endif
					}
				}
				else if (flag == 3) {
					if ( startAddr == -1){
						startAddr = 0; // always start from offset 0 for 2nd FLASH partition
					}
				}
				else { // flag == 2
					if ( startAddrWeb == -1){
						//startAddr = WEB_PAGE_OFFSET;
						startAddr = pHeader->burnAddr ;
#ifdef _LITTLE_ENDIAN_
						startAddr = DWORD_SWAP(startAddr);
#endif
					}
					else
						startAddr = startAddrWeb;
				}
				//printf("<%s %d>: startAddr=0x%x, pHeader->burnAddr=0x%x \n", __FUNCTION__, __LINE__, startAddr, pHeader->burnAddr);
				//printf("<%s %d>: locWrite=%d, numWrite=%d, numLeft=%d\n", __FUNCTION__,__LINE__,locWrite, numWrite, numLeft);
				
#if 0
				lseek(fh, startAddr, SEEK_SET);
				if(flag == 3){
					locWrite += sizeof(IMG_HEADER_T); // remove header
					numLeft -=  sizeof(IMG_HEADER_T);

					kill_processes();
					sleep(2);
				}
				numWrite = write(fh, &(upload_data[locWrite+head_offset]), numLeft);
#else
				if(flag == 3){
					locWrite += sizeof(IMG_HEADER_T); // remove header
					numLeft -=  sizeof(IMG_HEADER_T);
				}
				numWrite = numLeft;
#if defined(ECOS_MEM_CHAIN_API)
				if(mem_chain_support_rtk_flash_write(&(upload_data[locWrite+head_offset]),pHeader->burnAddr,numLeft)==0)
#else
				if(rtk_flash_write(&(upload_data[locWrite+head_offset]),pHeader->burnAddr,numLeft)==0)
#endif
				{
                        		sprintf(buffer, "<b>Image fw write to flash fail!</b><br>");
                        		goto ret_upload;
                		}    
#endif

#endif

#if 0
				if (numWrite < numLeft) {
#ifdef __mips__
					sprintf(buffer, "File write failed. locWrite=%d numLeft=%d numWrite=%d Size=%d bytes.", locWrite, numLeft, numWrite, upload_len);

#else
					sprintf(buffer, "File write failed. ferror=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes.", ferror(fp), locWrite, numLeft, numWrite, upload_len);
#endif
					goto ret_upload;
				}
#endif
				locWrite += numWrite;
				numLeft -= numWrite;
#if defined(ECOS_MEM_CHAIN_API)
				mem_chain_upgrade_free(pHeader);
#endif
				//printf("<%s %d>: locWrite=%d, numWrite=%d, numLeft=%d\n", __FUNCTION__,__LINE__,locWrite, numWrite, numLeft);
#if 0
				sync();
#ifdef __mips__
				//if(flag != 3)
				close(fh);
#else
				fclose(fp);
#endif
#endif // remove these by cairui
				head_offset += len + sizeof(IMG_HEADER_T) ;
				startAddr = -1 ; //by sc_yang to reset the startAddr for next image
				update_fw = 1;
			}
			//printf("<%s %d>: flag=%d, fh=%d\n", __FUNCTION__, __LINE__, flag, fh);
		} //end while //sc_yang   
#ifndef NO_ACTION
			//alarm(2);
			//system("reboot");
			//for(;;);
#else
#ifdef VOIP_SUPPORT
			// rock: for x86 simulation
			if (update_cfg && !update_fw) {
				if (apmib_reinit()) {
					reset_user_profile();  // re-initialize user password
				}
				if(FW_Data)
					free(FW_Data);
			}
#endif
#endif
			return 1;
ret_upload:	
			fprintf(stderr, "%s\n", buffer);	
			return 0;
	} //end doFirmwareUpgrade

		//#define BACKUP_RAW_CSCONF temporary by cairui!

#if defined(BACKUP_RAW_CSCONF)
		static unsigned char *compFile = NULL;
#endif

		int mib_backup(int backup_target)
		{
			int ret = 1;

			//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);

#if defined(BACKUP_RAW_CSCONF)
			ret = flash_read_raw_mib(&compFile); /* only backup current setting, don't care backup_target*/
#endif

			return ret;
		}

		int mib_restore(int restore)
		{
			int ret = 1;

			//fprintf(stderr,"\r\nrestore=[%d], __[%s-%u]",restore,__FILE__,__LINE__);

#if defined(BACKUP_RAW_CSCONF)
			if (restore) {
				ret = flash_write_raw_mib(&compFile);
				if (ret == 1) {
					if (1!= apmib_reinit())
						ret = -1;
				}
			}
#endif
			return ret;
		}

		//by cairui
		//#if defined(MOD_FOR_TR098_LANDEVICE)
		int getInFlags(char *interface, int *flags)
		{
			int skfd, ret = 0;
			struct ifreq ifr;

			skfd = socket(AF_INET, SOCK_DGRAM, 0);

			strcpy(ifr.ifr_name, interface);
			if ((ret = ioctl(skfd, SIOCGIFFLAGS, &ifr)) < 0)
				ret = 0;
			else {
				*flags = ifr.ifr_flags;
				ret = 1;
			}

			close(skfd);
			return ret;	
		}
		//#endif

#define	RTL8651_IOCTL_GETLANLINKUP			2300
#define 	RTL8651_IOCTL_GETWANLINKUP			2301
#define   RTL8651_IOCTL_GETWANSPEED 			2306


		int getLanLinkUpFlag(char *interface, int *flags)
		{
			int flags_tmp;
			int ret = -1;
			ret = re865xIoctl(interface, RTL8651_IOCTL_GETLANLINKUP, 0, 0, (unsigned int)&flags_tmp);
			*flags = flags_tmp;
			return ret;
		}

		int getWanLinkUpFlag(char *interface, int *flags)
		{
			int flags_tmp;
			int ret = -1;
			ret = re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKUP, 0, 0, (unsigned int)&flags_tmp);
			*flags = flags_tmp;
			return ret;
		}

		int cwmp_getWanSpeed(char *interface, unsigned int *speed)
		{
			unsigned int speed_tmp;
			int ret = -1;
			ret = re865xIoctl(interface, RTL8651_IOCTL_GETWANSPEED, 0, 0, (unsigned int)&speed_tmp);
			*speed = speed_tmp;
			return ret;
		}


		int getWanStats(char *interface, unsigned int type, unsigned long *stats)
		{
			unsigned long stats_tmp = 0;
			int ret = -1;
			ret = re865xIoctl(interface, type, 0, 0, (unsigned int)&stats_tmp);
			*stats = stats_tmp;
			return ret;
		}

		unsigned char *gettoken(const unsigned char *str,unsigned int index,unsigned char symbol)
		{
			static char tmp[50];
			unsigned char tk[50]; //save symbol index
			char *ptmp;
			int i,j,cnt=1,start,end;
			//scan symbol and save index

			memset(tmp, 0x00, sizeof(tmp));

			for (i=0;i<strlen((char *)str);i++)
			{          
				if (str[i]==symbol)
				{
					tk[cnt]=i;
					cnt++;
				}
			}

			if (index>cnt-1)
			{
				return NULL;
			}

			tk[0]=0;
			tk[cnt]=strlen((char *)str);

			if (index==0)
				start=0;
			else
				start=tk[index]+1;

			end=tk[index+1];

			j=0;
			for(i=start;i<end;i++)
			{
				tmp[j]=str[i];
				j++;
			}

			return (unsigned char *)tmp;
		}

		/* remove by cairui: use extern
		   unsigned int getWLAN_ChipVersion()
		   {
		   FILE *stream;
		   char buffer[128];
		   typedef enum { CHIP_UNKNOWN=0, CHIP_RTL8188C=1, CHIP_RTL8192C=2, CHIP_RTL8192D=3} CHIP_VERSION_T;
		   CHIP_VERSION_T chipVersion = CHIP_UNKNOWN;	


		   sprintf(buffer,"/proc/wlan%d/mib_rf",wlan_idx);
		   stream = fopen (buffer, "r" );
		   if ( stream != NULL )
		   {		
		   char *strtmp;
		   char line[100];

		   while (fgets(line, sizeof(line), stream))
		   {

		   strtmp = line;
		   while(*strtmp == ' ')
		   {
		   strtmp++;
		   }


		   if(strstr(strtmp,"RTL8192SE") != 0)
		   {
		   chipVersion = CHIP_UNKNOWN;
		   }
		   else if(strstr(strtmp,"RTL8188C") != 0)
		   {
		   chipVersion = CHIP_RTL8188C;
		   }
		   else if(strstr(strtmp,"RTL8192C") != 0)
		   {
		   chipVersion = CHIP_RTL8192C;
		   }
		   else if(strstr(strtmp,"RTL8192D") !=0)
		   {
		   chipVersion = CHIP_RTL8192D;
		   }
		   }			
		   fclose ( stream );
		   }

		   return chipVersion;

		   }
		   */

#if 0// by cairui use extern
		int isFileExist(char *file_name)
		{
			struct stat status;

			if ( stat(file_name, &status) < 0)
				return 0;

			return 1;
		}
#endif

#if 0 //remove by cairui, use extern
		int SetWlan_idx(char * wlan_iface_name)
		{
			int idx;

			idx = atoi(&wlan_iface_name[4]);
			if (idx >= NUM_WLAN_INTERFACE) {
				printf("invalid wlan interface index number!\n");
				return 0;
			}
			wlan_idx = idx;
			vwlan_idx = 0;

#ifdef MBSSID		

			if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
					wlan_iface_name[6] == 'v' && wlan_iface_name[7] == 'a') {
				idx = atoi(&wlan_iface_name[8]);
				if (idx >= NUM_VWLAN_INTERFACE) {
					printf("invalid virtual wlan interface index number!\n");
					return 0;
				}

				vwlan_idx = idx+1;
				idx = atoi(&wlan_iface_name[4]);
				wlan_idx = idx;
			}
#endif		

#ifdef UNIVERSAL_REPEATER
			if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
					!memcmp(&wlan_iface_name[6], "vxd", 3)) {
				vwlan_idx = NUM_VWLAN_INTERFACE;
				idx = atoi(&wlan_iface_name[4]);
				wlan_idx = idx;
			}
#endif				

			//printf("\r\n wlan_iface_name=[%s],wlan_idx=[%u],vwlan_idx=[%u],__[%s-%u]\r\n",wlan_iface_name,wlan_idx,vwlan_idx,__FILE__,__LINE__);

			return 1;		
		}
#endif

#if 0// remove by cairui, use extern
		short whichWlanIfIs(PHYBAND_TYPE_T phyBand)
		{
			int i;
			int ori_wlan_idx=wlan_idx;
			int ret=-1;

			for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
			{
				unsigned char wlanif[10];
				memset(wlanif,0x00,sizeof(wlanif));
				sprintf((char *)wlanif, "wlan%d",i);
				if(SetWlan_idx((char *)wlanif))
				{
					int phyBandSelect;
					apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyBandSelect);
					if(phyBandSelect == phyBand)
					{
						ret = i;
						break;			
					}
				}						
			}

			wlan_idx=ori_wlan_idx;
			return ret;		
		}
#endif

		int getOUTfromMAC(char *ouiname)
		{
			char oui[10];
			int i;
			unsigned char tmpBuff[32]={0};

			mib_get(MIB_ELAN_MAC_ADDR,  (void *)tmpBuff);

			if(!memcmp(tmpBuff, "\x00\x00\x00\x00\x00\x00", 6)){
				mib_get(MIB_HW_NIC0_ADDR,  (void *)tmpBuff);
				if(!memcmp(tmpBuff, "\x00\x00\x00\x00\x00\x00", 6))
					sprintf(oui,"%s", "00E04C");
				else{
					sprintf(oui,"%02x%02x%02x",tmpBuff[0],tmpBuff[1],tmpBuff[2]);
				}	
			}else{
				sprintf(oui,"%02x%02x%02x",tmpBuff[0],tmpBuff[1],tmpBuff[2]);
			}	
			for(i=0;i<strlen(oui);i++)
			{
				ouiname[i]=toupper(oui[i]);
			}
			ouiname[i]=0;

		}
#if 0 //defined(CONFIG_APP_TR069) && defined(WLAN_SUPPORT)
		int swapWLANIdxForCwmp(unsigned char wlanifNumA, unsigned char wlanifNumB)
		{		
			int ret=-1;
			unsigned int i,num;
			CWMP_WLANCONF_T *pwlanConf, wlanConf;
			CWMP_WLANCONF_T target[2];
			pwlanConf = &wlanConf;
			mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);
			for( i=1; i<=num;i++ )	
			{		
				*((char *)pwlanConf) = (char)i;
				if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
					continue;

				memcpy(&target[0], &wlanConf, sizeof(CWMP_WLANCONF_T));	

				if( pwlanConf->RootIdx==0)
					pwlanConf->RootIdx=1;
				else if(pwlanConf->RootIdx==1)
					pwlanConf->RootIdx=0;
				if(pwlanConf->RfBandAvailable ==PHYBAND_5G)
					pwlanConf->RfBandAvailable =PHYBAND_2G;
				else if(pwlanConf->RfBandAvailable ==PHYBAND_2G)
					pwlanConf->RfBandAvailable =PHYBAND_5G;

				memcpy(&target[1], &wlanConf, sizeof(CWMP_WLANCONF_T));
				mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);		
			}		
			return ret;
		}


		void swapWlanMibSetting(unsigned char wlanifNumA, unsigned char wlanifNumB)
		{
			unsigned char *wlanMibBuf=NULL;
			unsigned int totalSize = sizeof(CONFIG_WLAN_SETTING_T)*(NUM_VWLAN_INTERFACE+1); // 4vap+1rpt+1root

#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_lock();
#endif

			wlanMibBuf = malloc(totalSize); 
#if 0	
			printf("\r\n wlanifNumA=[%u],__[%s-%u]\r\n",wlanifNumA,__FILE__,__LINE__);
			printf("\r\n wlanifNumB=[%u],__[%s-%u]\r\n",wlanifNumB,__FILE__,__LINE__);

			printf("\r\n pMib->wlan[wlanifNumA]=[0x%x],__[%s-%u]\r\n",pMib->wlan[wlanifNumA],__FILE__,__LINE__);
			printf("\r\n pMib->wlan[wlanifNumB]=[0x%x],__[%s-%u]\r\n",pMib->wlan[wlanifNumB],__FILE__,__LINE__);

			printf("\r\n pMib->wlan[0][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].wlanDisabled,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[0][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].phyBandSelect,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[0][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].channel,__FILE__,__LINE__);

			printf("\r\n pMib->wlan[1][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].wlanDisabled,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[1][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].phyBandSelect,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[1][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].channel,__FILE__,__LINE__);
#endif			
			if(wlanMibBuf != NULL)
			{
				memcpy(wlanMibBuf, pMib->wlan[wlanifNumA], totalSize);
				memcpy(pMib->wlan[wlanifNumA], pMib->wlan[wlanifNumB], totalSize);
				memcpy(pMib->wlan[wlanifNumB], wlanMibBuf, totalSize);

				free(wlanMibBuf);
			}

#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_unlock();
#endif

#if 0	
			printf("\r\n pMib->wlan[0][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].wlanDisabled,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[0][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].phyBandSelect,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[0][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].channel,__FILE__,__LINE__);

			printf("\r\n pMib->wlan[1][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].wlanDisabled,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[1][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].phyBandSelect,__FILE__,__LINE__);
			printf("\r\n pMib->wlan[1][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].channel,__FILE__,__LINE__);
#endif	
#ifdef UNIVERSAL_REPEATER
			int rptEnable1, rptEnable2;
			char rptSsid1[MAX_SSID_LEN], rptSsid2[MAX_SSID_LEN];

			memset(rptSsid1, 0x00, MAX_SSID_LEN);
			memset(rptSsid2, 0x00, MAX_SSID_LEN);

			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnable1);
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnable2);
			apmib_get(MIB_REPEATER_SSID1, (void *)rptSsid1);
			apmib_get(MIB_REPEATER_SSID2, (void *)rptSsid2);

			apmib_set(MIB_REPEATER_ENABLED1, (void *)&rptEnable2);
			apmib_set(MIB_REPEATER_ENABLED2, (void *)&rptEnable1);
			apmib_set(MIB_REPEATER_SSID1, (void *)rptSsid2);
			apmib_set(MIB_REPEATER_SSID2, (void *)rptSsid1);
#endif
#if VLAN_CONFIG_SUPPORTED 
			unsigned char *vlanMibBuf=NULL;
			totalSize = sizeof(VLAN_CONFIG_T)*5; // 4vap+1root

#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_lock();
#endif

			vlanMibBuf = malloc(totalSize);
			if(vlanMibBuf != NULL)
			{
				memcpy(vlanMibBuf, pMib->VlanConfigArray+4, totalSize);
				memcpy(pMib->VlanConfigArray+4, pMib->VlanConfigArray+9, totalSize);
				memcpy(pMib->VlanConfigArray+9, vlanMibBuf, totalSize);

				free(vlanMibBuf);
			}

#if CONFIG_APMIB_SHARED_MEMORY == 1
			apmib_sem_unlock();
#endif

#endif
		}
#endif
#if defined(CONFIG_REPEATER_WPS_SUPPORT) || defined(POWER_CONSUMPTION_SUPPORT)
		WLAN_STATE_T lastWanState = WLAN_OFF;
		WLAN_STATE_T updateWlanifState(char *wlanif_name)
		{
			FILE *stream;
			int debug_check = 0;
			WLAN_STATE_T wlanState = WLAN_NO_LINK;
			unsigned char tmpStr[100];

			memset(tmpStr,0x00,sizeof(tmpStr));
			sprintf((char *)tmpStr,"/proc/%s/sta_info",wlanif_name);

			stream = fopen ((char *)tmpStr, "r" );
			if ( stream != NULL )
			{		
				char *strtmp;
				char line[100];
				while (fgets(line, sizeof(line), stream))
				{
					unsigned char *p;
					strtmp = line;

					while(*strtmp == ' ')
						strtmp++;

					if(strstr(strtmp,"active") != 0)
					{
						unsigned char str1[10], str2[10];

						//-- STA info table -- (active: 1)
						if(debug_check)
							fprintf(stderr,"\r\n [%s]",strtmp);

						sscanf(strtmp, "%*[^:]:%[^)]",str1);

						p = str1;
						while(*p == ' ')
							p++;										

						if(strcmp((char *)p,"0") == 0)
						{
							wlanState = WLAN_NO_LINK;
						}
						else
						{
							wlanState = WLAN_LINK;						
						}										

						break;
					}

				}
				fclose(stream );

			}

			if(wlanState == WLAN_LINK)
			{
				bss_info bss;

				cwmp_getWlBssInfo(wlanif_name, &bss);

				if(bss.state != STATE_CONNECTED && bss.state != STATE_STARTED)
					wlanState = WLAN_NO_LINK;

			}

			memset(tmpStr,0x00,sizeof(tmpStr));
			if(lastWanState != wlanState)
			{
				lastWanState = wlanState;
				if(wlanState == WLAN_LINK)
					sprintf((char *)tmpStr,"echo \"%s LINK\" > /var/wlan_state",wlanif_name);
				else
					sprintf((char *)tmpStr,"echo \"%s NO_LINK\" > /var/wlan_state",wlanif_name);

				system((char *)tmpStr);
			}

			return wlanState;

		}
#endif //#if defined(CONFIG_REPEATER_WPS_SUPPORT) || defined(POWER_CONSUMPTION_SUPPORT)

#if defined(CONFIG_RTL_P2P_SUPPORT)

		int getWlP2PScanResult(char *interface, SS_STATUS_Tp pStatus )
		{
#ifndef NO_ACTION
			int skfd=0;
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

			wrq.u.data.pointer = (caddr_t)pStatus;

			if ( pStatus->number == 0 )
				wrq.u.data.length = sizeof(SS_STATUS_T);
			else
				wrq.u.data.length = sizeof(pStatus->number);

			if (iw_get_ext(skfd, interface, SIOCP2PGETRESULT, &wrq) < 0){
				close( skfd );
				return -1;
			}
			close( skfd );
#else
			return -1 ;
#endif

			return 0;
		}

		int getWlP2PScanRequest(char *interface, int *pStatus)
		{
#ifndef NO_ACTION
			int skfd=0;
			struct iwreq wrq;
			unsigned char result;

			//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			if(skfd==-1)
				return -1;

			//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

			/* Get wireless name */
			if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
				/* If no wireless name : no wireless extensions */
				close( skfd );
				return -1;
			}
			wrq.u.data.pointer = (caddr_t)&result;
			wrq.u.data.length = sizeof(result);

			//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

			if (iw_get_ext(skfd, interface, SIOCP2PSCANREQ, &wrq) < 0)
			{
				close( skfd );
				return -1;
			}
			close( skfd );


			if ( result == 0xff )
				*pStatus = -1;
			else
				*pStatus = (int) result;
#else
			*pStatus = -1;
#endif // #ifndef NO_ACTION

			return 0;

		}

		int getWlP2PStateEvent( char *interface, P2P_SS_STATUS_Tp pP2PStatus)
		{
#ifndef NO_ACTION
			int skfd=0;
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
			wrq.u.data.pointer = (caddr_t)pP2PStatus;
			wrq.u.data.length = sizeof(P2P_SS_STATUS_T);

			if (iw_get_ext(skfd, interface, SIOCP2PPGETEVNIND, &wrq) < 0){
				close( skfd );

				return -1;
			}     


			close( skfd );
#else
			*num = 0 ;
#endif

			return 0;
		}


		int getClientConnectState(void)
		{

			static struct __p2p_state_event P2PStatus_t;
			memset(&P2PStatus_t , 0 ,sizeof(struct __p2p_state_event));

			int skfd=0;
			struct iwreq wrq;

			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			if(skfd==-1)
				return 0;

			/* Get wireless name */
			if ( iw_get_ext(skfd, "wlan0", SIOCGIWNAME, &wrq) < 0){
				/* If no wireless name : no wireless extensions */
				close( skfd );
				return 0;
			}

			wrq.u.data.pointer = (caddr_t)&P2PStatus_t;
			wrq.u.data.length = sizeof(struct __p2p_state_event);

			if (iw_get_ext(skfd, "wlan0", SIOCP2P_REPORT_CLIENT_STATE, &wrq) < 0){
				close( skfd );  	 
				return 0;
			}     
			close( skfd );

			return P2PStatus_t.p2p_status;

		}


		int sendP2PProvisionCommInfo( char *interface, P2P_PROVISION_COMM_Tp pP2PProvisionComm)
		{
#ifndef NO_ACTION
			int skfd=0;
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
			wrq.u.data.pointer = (caddr_t)pP2PProvisionComm;
			wrq.u.data.length = sizeof(P2P_PROVISION_COMM_T);

			if (iw_get_ext(skfd, interface, SIOCP2PPROVREQ, &wrq) < 0){
				close( skfd );

				return -1;
			}     


			close( skfd );
#else
			*num = 0 ;
#endif

			return 0;
		}


		int sendP2PWscConfirm( char *interface, P2P_WSC_CONFIRM_Tp pP2PWscConfirm)
		{
#ifndef NO_ACTION
			int skfd=0;
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
			wrq.u.data.pointer = (caddr_t)pP2PWscConfirm;
			wrq.u.data.length = sizeof(P2P_WSC_CONFIRM_T);

			if (iw_get_ext(skfd, interface, SIOCP2WSCMETHODCONF, &wrq) < 0){
				close( skfd );

				return -1;
			}     


			close( skfd );
#else
			*num = 0 ;
#endif

			return 0;
		}
#endif

#ifdef CONFIG_APP_TR069
		const char IPTABLES[] = "/bin/iptables";
		const char ARG_T[] = "-t";
		const char FW_DEL[] = "-D";
		const char FW_PREROUTING[] = "PREROUTING";
		const char ARG_I[] = "-i";
		const char LANIF[] = "br0";
		const char ARG_TCP[] = "TCP";
		const char ARG_UDP[] = "UDP";
		const char FW_DPORT[] = "--dport";
		const char RMACC_MARK[] = "0x1000";
		const char FW_ADD[] = "-A";

#ifdef _PRMT_TR143_
		#if !defined(ECOS)
		const char gUDPEchoServerName[]="/bin/udpechoserver";
		const char gUDPEchoServerPid[] = "/var/run/udpechoserver.pid";
		#else
		const char gUDPEchoServerName[]="udpechoserver";
		#endif
	
        int read_pid(char *filename)
		{
			int fh;
			FILE *in;
			int pid;

			fh = open(filename, O_RDWR);
			if ( fh == -1 ) return -1;
			if ((in = fdopen(fh, "r")) == NULL) return -1;
			fscanf(in, "%d", &pid);
			fclose(in);
			close(fh);

			return pid;
		}

		void UDPEchoConfigSave(struct TR143_UDPEchoConfig *p)
		{
			if(p)
			{
				unsigned char itftype;
				mib_get( MIB_TR143_UDPECHO_ENABLE, (void *)&p->Enable );
				mib_get( MIB_TR143_UDPECHO_SRCIP, (void *)p->SourceIPAddress );
				mib_get( MIB_TR143_UDPECHO_PORT, (void *)&p->UDPPort );
				mib_get( MIB_TR143_UDPECHO_PLUS, (void *)&p->EchoPlusEnabled );

				mib_get( MIB_TR143_UDPECHO_ITFTYPE, (void *)&itftype );
				if(itftype==ITF_WAN)
				{
#if 0	//No ATM PVC Support 		 
					int total,i;
					MIB_CE_ATM_VC_T *pEntry, vc_entity;

					p->Interface[0]=0;
					total = mib_chain_total(MIB_ATM_VC_TBL);
					for( i=0; i<total; i++ )
					{
						pEntry = &vc_entity;
						if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
							continue;
						if(pEntry->TR143UDPEchoItf)
						{
							ifGetName(pEntry->ifIndex, p->Interface, sizeof(p->Interface));
						}
					}
#endif			
				}else if(itftype<ITF_END)
				{
					strcpy( p->Interface, strItf[itftype] );
					LANDEVNAME2BR0(p->Interface);
				}else
					p->Interface[0]=0;

			}
			return;
		}

		int UDPEchoConfigStart( struct TR143_UDPEchoConfig *p )
		{
			if(!p) return -1;
			
			if( p->Enable )
			{
				char strPort[16], strAddr[32];
				char *argv[10];
				int  i;

				if(p->UDPPort==0)
				{
					fprintf( stderr, "UDPEchoConfigStart> error p->UDPPort=0\n" );
					return -1;
				}
				sprintf( strPort, "%u", p->UDPPort );
				va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", FW_ADD, (char *)FW_PREROUTING,
						(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

				i=0;
				argv[i]=(char *)gUDPEchoServerName;
				i++;
				argv[i]="-port";
				i++;
				argv[i]=strPort;
				i++;
				if( strlen(p->Interface) > 0 )
				{
					argv[i]="-i";
					i++;
					argv[i]=p->Interface;
					i++;
				}
				if( p->SourceIPAddress[0]!=0 ||
						p->SourceIPAddress[1]!=0 ||
						p->SourceIPAddress[2]!=0 ||
						p->SourceIPAddress[3]!=0  )
				{
					struct in_addr *pSIP = (struct in_addr *)p->SourceIPAddress;
					argv[i]="-addr";
					i++;
					sprintf( strAddr, "%s", inet_ntoa( *pSIP ) );
					argv[i]=strAddr;
					i++;
				}
				if( p->EchoPlusEnabled )
				{
					argv[i]="-plus";
					i++;
				}

				argv[i]=NULL;
#if 1 //!defined(ECOS)
				do_cmd( gUDPEchoServerName, argv, 0 );
#else
				do_cmd( gUDPEchoServerName, argv, i-- );
#endif

			}

			return 0;
		}

		int UDPEchoConfigStop( struct TR143_UDPEchoConfig *p )
		{
			char strPort[16];
			int pid;
			int status;
			
			if(!p)
				return -1;

			sprintf( strPort, "%u", p->UDPPort );
			va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", FW_DEL, (char *)FW_PREROUTING,
					(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_UDP,
					(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

#if 0 //!defined(ECOS)
			pid = read_pid((char *)gUDPEchoServerPid);
			if (pid >= 1)
			{
				status = kill(pid, SIGTERM);
				if (status != 0)
				{
					//printf("Could not kill UDPEchoServer's pid '%d'\n", pid);
					return -1;
				}
			}
#else
			//cyg_udpechoserver_exit();
#endif
			return 0;
		}

		int apply_UDPEchoConfig( int action_type, int id, void *olddata )
		{
			struct TR143_UDPEchoConfig newdata;
			int ret=0;
			
			UDPEchoConfigSave(&newdata);
			switch( action_type )
			{
				case CWMP_RESTART:
					if(olddata)
						UDPEchoConfigStop(olddata);
				case CWMP_START:
					UDPEchoConfigStart(&newdata);
					break;
				case CWMP_STOP:
					if(olddata)
						UDPEchoConfigStop(olddata);
					break;
				default:
					ret=-1;
			}

			return ret;
		}
		int alias_name_are_eq(char *orig_name,char *alias1,char *alias2)
		{


			int rtnValue=1;
			rtnValue=strncmp(orig_name,alias1,strlen(alias1));

			if(rtnValue)
				rtnValue&=strncmp(orig_name,alias2,strlen(alias2));

#if 0//def CONFIG_RTL_ALIASNAME_DEBUG
			if(rtnValue!=strncmp(orig_name,cmp_name,strlen(cmp_name)))
			{
				printk("====CONFIG_RTL_ALIASNAME_DEBUG====\n\ncmp_name : %s\norig_name :%s\n",cmp_name,orig_name);
				printk("rtnValue %d\n",rtnValue);
			}
#endif
			return !rtnValue;

		}
		int IfName2ItfId( char *s )
		{
			int i;
			if( !s || s[0]==0 ) return ITF_ALL;

			//	if( (strncmp(s, "ppp", 3)==0) || (strncmp(s, "vc", 2)==0) )
			if(alias_name_are_eq(s,ALIASNAME_PPP,ALIASNAME_VC))
				return ITF_WAN;

			for( i=0;i<ITF_END;i++ )
			{
				if( strcmp( strItf[i],s )==0 ) return i;
			}

			return -1;
		}
#endif //_PRMT_TR143_

		static int procnetdev_version(char *buf)
		{
			if (strstr(buf, "compressed"))
				return 2;
			if (strstr(buf, "bytes"))
				return 1;
			return 0;
		}

#if 0
		static const char *const ss_fmt[] = {
			"%n%lu%lu%lu%lu%lu%n%n%n%lu%lu%lu%lu%lu%lu",
			"%lu%lu%lu%lu%lu%lu%n%n%lu%lu%lu%lu%lu%lu%lu",
			"%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu"
		};

		static void get_dev_fields(char *bp, struct net_device_stats *nds, int procnetdev_vsn)
		{
			memset(nds, 0, sizeof(*nds));

			sscanf(bp, ss_fmt[procnetdev_vsn],
					&nds->rx_bytes, /* missing for 0 */
					&nds->rx_packets,
					&nds->rx_errors,
					&nds->rx_dropped,
					&nds->rx_fifo_errors,
					&nds->rx_frame_errors,
					&nds->rx_compressed, /* missing for <= 1 */
					&nds->multicast, /* missing for <= 1 */
					&nds->tx_bytes, /* missing for 0 */
					&nds->tx_packets,
					&nds->tx_errors,
					&nds->tx_dropped,
					&nds->tx_fifo_errors,
					&nds->collisions,
					&nds->tx_carrier_errors,
					&nds->tx_compressed /* missing for <= 1 */
			      );

			if (procnetdev_vsn <= 1) {
				if (procnetdev_vsn == 0) {
					nds->rx_bytes = 0;
					nds->tx_bytes = 0;
				}
				nds->multicast = 0;
				nds->rx_compressed = 0;
				nds->tx_compressed = 0;
			}
		}
#endif
		/**
		 * list_net_device_with_flags - list network devices with the specified flags
		 * @flags: input argument, the network device flags
		 * @nr_names: input argument, number of elements in @names
		 * @names: output argument, constant pointer to the array of network device names
		 *
		 * Returns the number of resulted elements in @names for success
		 * or negative errno values for failure.
		 */
		int list_net_device_with_flags(short flags, int nr_names,
				char (* const names)[IFNAMSIZ])
		{
			FILE *fh;
			char buf[512];
			struct ifreq ifr;
			int nr_result, skfd;

			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			if (skfd < 0)
				goto out;

			fh = fopen(_PATH_PROCNET_DEV, "r");
			if (!fh)
				goto out_close_skfd;
			fgets(buf, sizeof(buf), fh);	/* eat line */
			fgets(buf, sizeof(buf), fh);

			nr_result = 0;
			while (fgets(buf, sizeof(buf), fh) && nr_result < nr_names) {
				char name[128];

				get_name(name, buf);

				strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
				if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
					goto out_close_fh;

				if (ifr.ifr_flags & flags) {
					strncpy(names[nr_result++], name, ARRAY_SIZE(names[0]));
				}
			}

			if (ferror(fh))
				goto out_close_fh;

			fclose(fh);
			close(skfd);

			return nr_result;

out_close_fh:
			fclose(fh);
out_close_skfd:
			close(skfd);
out:
			//printf("%s():%d", __FUNCTION__, __LINE__);

			return -errno;
		}
		// Kaohj
		/*
		 *	Get the link status about device.
		 *	Return:
		 *		-1 on error
		 *		0 on link down
		 *		1 on link up
		 */
		int get_net_link_status(const char *ifname)
		{


			int ret=0;
#if 0	
			struct ethtool_value edata;
			struct ifreq ifr;
#ifdef CONFIG_DEV_xDSL
			Modem_LinkSpeed vLs;
#endif

#ifdef CONFIG_DEV_xDSL
			if (!strcmp(ifname, ALIASNAME_DSL0)) {
				if ( adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
							RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0)
					ret = 1;
				else
					ret = 0;
				return ret;
			}
#endif
			strcpy(ifr.ifr_name, ifname);
			edata.cmd = ETHTOOL_GLINK;
			ifr.ifr_data = (caddr_t)&edata;

			ret = do_ioctl(SIOCETHTOOL, &ifr);
			if (ret == 0)
				ret = edata.data;
#endif		
			return ret;
		}
		int do_ioctl(unsigned int cmd, struct ifreq *ifr)
		{
			int skfd, ret;

			if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("socket");
				return (-1);
			}

			ret = ioctl(skfd, cmd, ifr);
			close(skfd);
			return ret;
		}
		/*
		 *	Get the link information about device.
		 *	Return:
		 *		-1 on error
		 *		0 on success
		 */
		int get_net_link_info(const char *ifname, struct net_link_info *info)
		{
			struct ifreq ifr;
			struct ethtool_cmd ecmd;
			int ret=-1;

			memset(info, 0, sizeof(struct net_link_info));
			strcpy(ifr.ifr_name, ifname);
			ecmd.cmd = ETHTOOL_GSET;
			ifr.ifr_data = (caddr_t)&ecmd;

			ret = do_ioctl(SIOCETHTOOL, &ifr);
			if (ret == 0) {
				info->supported = ecmd.supported; // ports, link modes, auto-negotiation
				info->advertising = ecmd.advertising; // link modes, pause frame use, auto-negotiation
				info->speed = ecmd.speed; // 10Mb, 100Mb, gigabit
				info->duplex = ecmd.duplex; // Half, Full, Unknown
				info->phy_address = ecmd.phy_address;
				info->transceiver = ecmd.transceiver;
				info->autoneg = ecmd.autoneg;
			}
			return ret;
		}


		// Kaohj -- specific for pvc channel
		// map: bit map of used interface, ppp index (0~15) is mapped into high 16 bits,
		// while vc index (0~15) is mapped into low 16 bits.
		// return: interface index, byte1 for PPP index and byte0 for vc index.
		//		0xefff(NA_PPP): PPP not available
		//		0xffff(NA_VC) : vc not available
		unsigned int if_find_index(int cmode, unsigned int map)
		{
			int i;
			unsigned int index, vc_idx, ppp_idx;

			// find the first available vc index (mpoa interface)
			i = 0;
			for (i=0; i<MAX_VC_NUM; i++)
			{
				if (!((map>>i) & 1))
					break;
			}

			if (i != MAX_VC_NUM)
				vc_idx = i;
			else
				return NA_VC;

			//fprintf(stderr, "<%s:%d>vc_idx:0x%08X\n", __FUNCTION__, __LINE__, vc_idx);

			if (cmode == IP_PPP)
			{
				// find an available PPP index
				map >>= 16;
				i = 0;
				while (map & 1)
				{
					map >>= 1;
					i++;
				}
				ppp_idx = i;
				if (ppp_idx<=(MAX_PPP_NUM-1))
					index = TO_IFINDEX(MEDIA_ATM, ppp_idx, vc_idx);
				else
					return NA_PPP;

				//if (cmode == ADSL_PPPoA)
				//index |= 0x0f;	// PPPoA doesn't use mpoa interface, set to 0x0f (don't care)
				//	index = TO_IFINDEX(MEDIA_ATM, ppp_idx, DUMMY_VC_INDEX);
			}
			else
			{
				// don't care the PPP index
				//index |= 0xf0;
				index = TO_IFINDEX(MEDIA_ATM, DUMMY_PPP_INDEX, vc_idx);
			}
			return index;
		}


		/*
		 * Convert ifIndex to system interface name, e.g. eth0,vc0...
		 */
		char *ifGetName(int ifindex, char *buffer, unsigned int len)
		{
			MEDIA_TYPE_T mType;

			if ( ifindex == DUMMY_IFINDEX )
				return 0;
			if (PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
			{
				mType = MEDIA_INDEX(ifindex);
				if (mType == MEDIA_ATM)
					snprintf( buffer, len,  "%s%u",ALIASNAME_VC, VC_INDEX(ifindex) );
				else if (mType == MEDIA_ETH)
#ifdef CONFIG_RTL_MULTI_ETH_WAN
					snprintf( buffer, len, "%s%d",ALIASNAME_MWNAS, ETH_INDEX(ifindex));
#else
				snprintf( buffer, len,  "%s%u",ALIASNAME_ETH, ETH_INDEX(ifindex) );
#endif
				else
					return 0;
			}else{
				snprintf( buffer, len,  "%s%u",ALIASNAME_PPP, PPP_INDEX(ifindex) );
			}
			return buffer;
		}
#if 0 //keith no support
		int generateWanName(WANIFACE_T *entry, char* wanname)
		{
#ifdef CONFIG_ETHWAN
			char vid[16];
#else
			char vpistr[6];
			char vcistr[6];
#endif
			int i, mibtotal;
			WANIFACE_T tmpEntry;

			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&mibtotal);
			for(i=1; i<= mibtotal; i++)
			{
				*((char *)&tmpEntry) = (char)i;

				mib_get(MIB_WANIFACE_TBL, (void *)&tmpEntry);
				if(tmpEntry.ifIndex == entry->ifIndex 
#ifdef BR_ROUTE_ONEPVC
						&&tmpEntry.cmode == entry->cmode
#endif
				  )
					break;
			}
			if(i==mibtotal)
				return -1;
			i++;

			sprintf(wanname, "%d_", i);
			if(entry==NULL || wanname ==NULL)
				return -1;

			if (entry->applicationtype&X_CT_SRV_TR069)
				strcat(wanname, "TR069_");
#ifdef CONFIG_USER_RTK_VOIP
			if (entry->applicationtype&X_CT_SRV_VOICE)
				strcat(wanname, "VOICE_");
#endif
			if (entry->applicationtype&X_CT_SRV_INTERNET)
				strcat(wanname, "INTERNET_");
			if (entry->applicationtype&X_CT_SRV_OTHER)
				strcat(wanname, "Other_");
			if(entry->cmode == IP_BRIDGE)
				strcat(wanname, "B_");
			else
				strcat(wanname, "R_");
#ifdef CONFIG_ETHWAN
			strcat(wanname, "VID_");
			if(entry->vlan==1){
				sprintf(vid, "%d", entry->vlanid);
				strcat(wanname, vid);
			}
#else
			memset(vpistr, 0, sizeof(vpistr));
			memset(vcistr, 0, sizeof(vcistr));
			sprintf(vpistr, "%d", entry->vpi);
			sprintf(vcistr, "%d", entry->vci);
			strcat(wanname, vpistr);
			strcat(wanname, "_");
			strcat(wanname, vcistr);
#endif
			//star: for multi-ppp in one pvc
			/*
			   if(entry->cmode == ADSL_PPPoE || entry->cmode == ADSL_PPPoA)
			   {
			   char pppindex[6];
			   int intindex;
			   intindex = getpppindex(entry);
			   if(intindex != -1){
			   snprintf(pppindex,6,"%u",intindex);
			   strcat(wanname, "_");
			   strcat(wanname, pppindex);
			   }
			   }
			   */

			return 0;
		}
#endif //#if 0 //keith no support

#if defined(MULTI_WAN_SUPPORT)
		int  getATMVCByInstNum( unsigned int devnum, unsigned int ipnum, unsigned int pppnum, WANIFACE_T *p, unsigned int *chainid )
		{
			unsigned int i,num;

			if( (p==NULL) || (chainid==NULL) ) return NULL;

			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			for( i=1; i<=num;i++ )
			{
				*((char *)p) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)p))
					continue;


				if( (p->ConDevInstNum==devnum) &&
						(p->ConIPInstNum==ipnum) &&
						(p->ConPPPInstNum==pppnum) )
				{
					*chainid=i;
					return 1;
				}
			}

			return 0;
		}
		int getWanName(WANIFACE_T *pEntry, char* name)
		{
			if(pEntry==NULL || name==NULL)
				return 0;
			if(*(pEntry->WanName))
				strcpy(name, pEntry->WanName);
			else
			{//if not set by ACS. then generate automaticly.
				char wanname[MAX_WAN_NAME_LEN];
				memset(wanname, 0, sizeof(wanname));
				generateWanName(pEntry, wanname);
				strcpy(name, wanname);
			}
			return 1;
		}
#endif //#if defined(MULTI_WAN_SUPPORT)

		int getMIBtoStr(unsigned int id, char *strbuf)
		{
			unsigned char buffer[64];

			if (!strbuf)
				return -1;

			switch (id) {
				// INET address
				case MIB_IP_ADDR:
				case MIB_SUBNET_MASK:
				case MIB_DMZ_HOST:
				case MIB_DHCP_CLIENT_START:
				case MIB_DHCP_CLIENT_END:
				case MIB_DNS1:
				case MIB_DNS2:
				case MIB_DNS3:
				case MIB_DEFAULT_GATEWAY:
#if defined(_PRMT_X_CT_COM_DHCP_)
				case MIB_CT_STB_MINADDR:
				case MIB_CT_STB_MAXADDR:
				case MIB_CT_PHN_MINADDR:
				case MIB_CT_PHN_MAXADDR:
				case MIB_CT_CMR_MINADDR:
				case MIB_CT_CMR_MAXADDR:
				case MIB_CT_PC_MINADDR:
				case MIB_CT_PC_MAXADDR:
				case MIB_CT_HGW_MINADDR:
				case MIB_CT_HGW_MAXADDR:
#endif //_PRMT_X_CT_COM_DHCP_

#ifdef _PRMT_TR143_
				case MIB_TR143_UDPECHO_SRCIP:
#endif //_PRMT_TR143_

					if(!mib_get( id, (void *)buffer))
						return -1;
					// Mason Yu
					if ( ((struct in_addr *)buffer)->s_addr == INADDR_NONE ) {
						sprintf(strbuf, "%s", "");
					} else {
						sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
					}
					break;
					// Ethernet address
				case MIB_ELAN_MAC_ADDR:
				case MIB_WLAN_WLAN_MAC_ADDR:
					if(!mib_get( id,  (void *)buffer))
						return -1;

					sprintf(strbuf, "%02x%02x%02x%02x%02x%02x", buffer[0], buffer[1],
							buffer[2], buffer[3], buffer[4], buffer[5]);

					break;
				case MIB_WLAN_CHANNEL:
				case MIB_WLAN_WLAN_DISABLED:
				case MIB_WLAN_ENABLE_1X:
				case MIB_WLAN_ENCRYPT:
				case MIB_WLAN_WPA_AUTH:
				case MIB_WLAN_NETWORK_TYPE:
					if(!mib_get( id,  (void *)buffer))
						return -1;
					sprintf(strbuf, "%u", *(unsigned char *)buffer);
					break;
#ifdef WLAN_SUPPORT
				case MIB_WLAN_FRAG_THRESHOLD:
				case MIB_WLAN_RTS_THRESHOLD:
				case MIB_WLAN_BEACON_INTERVAL:
#endif
					if(!mib_get( id,  (void *)buffer))
						return -1;
					sprintf(strbuf, "%u", *(unsigned short *)buffer);
					break;

				case MIB_DHCP_LEASE_TIME:
					if(!mib_get( id,  (void *)buffer))
						return -1;
					// if MIB_ADSL_LAN_DHCP_LEASE=0xffffffff, it indicate an infinate lease
					if ( *(unsigned long *)buffer == 0xffffffff )
						sprintf(strbuf, "-1");
					else
						sprintf(strbuf, "%u", *(unsigned int *)buffer);
					break;

				case MIB_CWMP_CONREQ_PORT:
				case MIB_CWMP_INFORM_INTERVAL:

					if(!mib_get( id,  (void *)buffer))
						return -1;
					sprintf(strbuf, "%u", *(unsigned int *)buffer);
					break;
#ifdef WLAN_SUPPORT
				case MIB_WLAN_SSID:
#endif
				case MIB_CWMP_PROVISIONINGCODE:
				case MIB_CWMP_ACS_URL:
				case MIB_CWMP_ACS_USERNAME:
				case MIB_CWMP_ACS_PASSWORD:
				case MIB_CWMP_CONREQ_USERNAME:
				case MIB_CWMP_CONREQ_PASSWORD:
				case MIB_CWMP_CONREQ_PATH:
				case MIB_CWMP_LAN_CONFIGPASSWD:
				case MIB_CWMP_SERIALNUMBER:
				case MIB_CWMP_DL_COMMANDKEY:
				case MIB_CWMP_RB_COMMANDKEY:
				case MIB_CWMP_ACS_PARAMETERKEY:
				case MIB_CWMP_CERT_PASSWORD:
#ifdef _PRMT_USERINTERFACE_
				case MIB_UIF_AUTOUPDATESERVER:
				case MIB_UIF_USERUPDATESERVER:
#endif
				case MIB_CWMP_SI_COMMANDKEY:
				case MIB_CWMP_ACS_KICKURL:
				case MIB_CWMP_ACS_DOWNLOADURL:
				case MIB_SUPER_NAME:
				case MIB_USER_NAME:
				case MIB_DOMAIN_NAME:
#ifdef _PRMT_X_CT_COM_USERINFO_
				case MIB_CWMP_USERINFO_USERID:
				case MIB_CWMP_USERINFO_USERNAME:
#endif
					if(!mib_get( id,  (void *)strbuf)){
						return -1;
					}
					break;


				default:
					return -1;
			}

			return 0;
		}
#if 0
		int getSYS2Str(SYSID_T id, char *strbuf)
		{
			unsigned char buffer[128], vChar;
			struct sysinfo info;
			int updays, uphours, upminutes, len, i;
			time_t tm;
			struct tm tm_time, *ptm_time;
			FILE *fp;
			unsigned char tmpBuf[64], *pStr;
			unsigned short vUShort;
			unsigned int vUInt;

			if (!strbuf)
				return -1;

			strbuf[0] = '\0';

			switch (id) {
				case SYS_UPTIME:
					sysinfo(&info);
					updays = (int) info.uptime / (60*60*24);
					if (updays)
						sprintf(strbuf, "%d day%s, ", updays, (updays != 1) ? "s" : "");
					len = strlen(strbuf);
					upminutes = (int) info.uptime / 60;
					uphours = (upminutes / 60) % 24;
					upminutes %= 60;
					if(uphours)
						sprintf(&strbuf[len], "%2d:%02d", uphours, upminutes);
					else
						sprintf(&strbuf[len], "%d min", upminutes);
					break;
				case SYS_DATE:
					time(&tm);
					memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
					strftime(strbuf, 200, "%a %b %e %H:%M:%S %Z %Y", &tm_time);
					break;
				case SYS_YEAR:
					time(&tm);
					ptm_time = localtime(&tm);
					snprintf(strbuf, 64, "%d", (ptm_time->tm_year+ 1900));
					break;
				case SYS_MONTH:
					time(&tm);
					ptm_time = localtime(&tm);
					snprintf(strbuf, 64, "%d", (ptm_time->tm_mon+ 1));
					break;
				case SYS_DAY:
					time(&tm);
					ptm_time = localtime(&tm);
					snprintf(strbuf, 64, "%d", (ptm_time->tm_mday));
					break;
				case SYS_HOUR:
					time(&tm);
					ptm_time = localtime(&tm);
					snprintf(strbuf, 64, "%d", (ptm_time->tm_hour));
					break;
				case SYS_MINUTE:
					time(&tm);
					ptm_time = localtime(&tm);
					snprintf(strbuf, 64, "%d", (ptm_time->tm_min));
					break;
				case SYS_SECOND:
					time(&tm);
					ptm_time = localtime(&tm);
					snprintf(strbuf, 64, "%d", (ptm_time->tm_sec));
					break;
				case SYS_LAN_DHCP:
					if ( !mib_get( MIB_DHCP, (void *)&vUInt) )
						return -1;
					if (DHCP_SERVER == vUInt)
						strcpy(strbuf, STR_ENABLE);
					else
						strcpy(strbuf, STR_DISABLE);
					break;
				case SYS_DHCP_LAN_IP:
					getMIBtoStr(MIB_IP_ADDR, strbuf);
					break;
				case SYS_DHCP_LAN_SUBNET:
					getMIBtoStr(MIB_SUBNET_MASK, strbuf);
					break;
#ifdef WLAN_SUPPORT
				case SYS_WLAN:
					if ( !mib_get( MIB_WLAN_WLAN_DISABLED, (void *)&vUInt) )
						return -1;
					if (0 == vUInt)
						strcpy(strbuf, STR_ENABLE);
					else
						strcpy(strbuf, STR_DISABLE);
					break;
				case SYS_WLAN_BCASTSSID:
					if ( !mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vUInt) )
						return -1;
					if (0 == vUInt)
						strcpy(strbuf, STR_ENABLE);
					else
						strcpy(strbuf, STR_DISABLE);
					break;
				case SYS_WLAN_BAND:
					if ( !mib_get( MIB_WLAN_BAND, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_band[vUInt]);
					break;
				case SYS_WLAN_AUTH:
					if ( !mib_get( MIB_WLAN_AUTH_TYPE, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_auth[vUInt]);
					break;
				case SYS_WLAN_PREAMBLE:
					if ( !mib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_preamble[vUInt]);
					break;
				case SYS_WLAN_ENCRYPT:
					if ( !mib_get( MIB_WLAN_ENCRYPT, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_encrypt[vUInt]);
					break;
				case SYS_WLAN_WPA_CIPHER:
					if ( !mib_get( MIB_WLAN_WPA_CIPHER_SUITE,  (void *)&vUInt) )
						return -1;
					if ( vUInt == 0 )
						strcpy(strbuf, "");
					else
						strcpy(strbuf, wlan_Cipher[vUInt-1]);
					break;
				case SYS_WLAN_WPA2_CIPHER:
					if ( !mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vUInt) )
						return -1;
					if ( vUInt == 0 )
						strcpy(strbuf, "");
					else
						strcpy(strbuf, wlan_Cipher[vUInt-1]);
					break;
				case SYS_WLAN_PSKFMT:
					if ( !mib_get( MIB_WLAN_PSK_FORMAT, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_pskfmt[vUInt]);
					break;
				case SYS_WLAN_PSKVAL:
					if ( !mib_get( MIB_WLAN_WPA_PSK, (void *)buffer) )
						return -1;
					for (len=0; len<strlen(buffer); len++)
						strbuf[len]='*';
					strbuf[len]='\0';
					break;
				case SYS_WLAN_WEP_KEYLEN:
					if ( !mib_get( MIB_WLAN_WEP, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_wepkeylen[vUInt]);
					break;
				case SYS_WLAN_WEP_KEYFMT:
					if ( !mib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_wepkeyfmt[vUInt]);
					break;
				case SYS_WLAN_WPA_MODE:
					if (!mib_get(MIB_WLAN_WPA_AUTH, (void *)&vUInt))
						return -1;
					if (vUInt == WPA_AUTH_AUTO)
						strcpy(strbuf, "Enterprise (RADIUS)");
					else if (vUInt == WPA_AUTH_PSK)
						strcpy(strbuf, "Personal (Pre-Shared Key)");
					break;
				case SYS_WLAN_RSPASSWD:
					if (!mib_get(MIB_WLAN_RS_PASSWORD, (void *)buffer))
						return -1;
					for (len=0; len<strlen(buffer); len++)
						strbuf[len]='*';
					strbuf[len]='\0';
					break;
				case SYS_WLAN_MODE:
					if ( !mib_get( MIB_WLAN_MODE, (void *)&vUInt) )
						return -1;
					strcpy(strbuf, wlan_mode[vUInt]);
					break;
				case SYS_WLAN_TXRATE:
					if ( !mib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vUInt) )
						return -1;
					if (0 == vUInt){
						if ( !mib_get( MIB_WLAN_FIX_RATE, (void *)&vUShort) )
							return -1;
						for (i=0; i<12; i++)
							if (1<<i == vUShort)
								strcpy(strbuf, wlan_rate[i]);
					}
					else if (1 == vUInt)
						strcpy(strbuf, STR_AUTO);
					break;
				case SYS_WLAN_BLOCKRELAY:
					if ( !mib_get( MIB_WLAN_BLOCK_RELAY, (void *)&vUInt) )
						return -1;
					if (0 == vUInt)
						strcpy(strbuf, STR_DISABLE);
					else
						strcpy(strbuf, STR_ENABLE);
					break;
				case SYS_WLAN_AC_ENABLED:
					if ( !mib_get( MIB_WLAN_MACAC_ENABLED, (void *)&vUInt) )
						return -1;
					if (0 == vUInt)
						strcpy(strbuf, STR_DISABLE);
					else if(1 == vUInt)
						strcpy(strbuf, "Allow Listed");
					else if(2 == vUInt)
						strcpy(strbuf, "Deny Listed");
					else
						strcpy(strbuf, STR_ERR);
					break;
				case SYS_WLAN_WDS_ENABLED:
					if ( !mib_get( MIB_WLAN_WDS_ENABLED, (void *)&vUInt) )
						return -1;
					if (0 == vUInt)
						strcpy(strbuf, STR_DISABLE);
					else
						strcpy(strbuf, STR_ENABLE);
					break;
#endif
				case SYS_DNS_MODE:
					if ( !mib_get( MIB_DNS_MODE, (void *)&vUInt) )
						return -1;
					if (0 == vUInt)
						strcpy(strbuf, STR_AUTO);
					else
						strcpy(strbuf, STR_MANUAL);
					break;

				case SYS_DHCP_MODE:
					if(!mib_get( MIB_DHCP, (void *)&vUInt) )
						return -1;

					if(DHCP_DISABLED== vUInt)
						strcpy(strbuf, dhcp_mode[0]);
					else if(DHCP_SERVER == vUInt)
						strcpy(strbuf, dhcp_mode[2]);
					else if(DHCP_RELAY == vUInt)
						strcpy(strbuf, dhcp_mode[1]);

					break;

				default:
					return -1;
			}

			return 0;
		}
#endif


#endif
