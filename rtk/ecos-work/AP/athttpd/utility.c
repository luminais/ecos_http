/*utility.c for misc functions*/
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



#include "asp.h"
#include "fmget.h"
#include "http.h"
#include "socket.h"
#include "asp_form.h"
#include "apmib.h"
#include "utility.h"
#include "auth.h"
#include "common.h"


#if defined(HTTP_FILE_SERVER_SUPPORTED)
#define HEX_TO_DECIMAL(char1, char2)    \
    (((char1 >= 'A') ? (((char1 & 0xdf) - 'A') + 10) : (char1 - '0')) * 16) + \
    (((char2 >= 'A') ? (((char2 & 0xdf) - 'A') + 10) : (char2 - '0')))

int unescape_uri(char *uri, char **query_string)
{
    char c, d;
    char *uri_old;

    uri_old = uri;

    while ((c = *uri_old)) {
        if (c == '%') {
            uri_old++;
            if ((c = *uri_old++) && (d = *uri_old++)) {
                *uri = HEX_TO_DECIMAL(c, d);
#ifndef HTTP_FILE_SERVER_SUPPORTED
                if (*uri < 32 || *uri > 126) {
                    /* control chars in URI */
                    *uri = '\0';
                    return 0;
                }
#endif
            } else {
                *uri = '\0';
                return 0;
            }
            ++uri;
        } else if (c == '?') {  /* query string */
            if (query_string)
                *query_string = ++uri_old;
            /* stop here */
            *uri = '\0';
            return (1);
        } else if (c == '#') {  /* fragment */
            /* legal part of URL, but we do *not* care.
             * However, we still have to look for the query string */
            if (query_string) {
                ++uri_old;
                while ((c = *uri_old)) {
                    if (c == '?') {
                        *query_string = ++uri_old;
                        break;
                    }
                    ++uri_old;
                }
            }
            break;
        } else {
            *uri++ = c;
            uri_old++;
        }
    }

    *uri = '\0';
    return 1;
}
#endif

#if 0  //collison with system/sys_utility.c
int SetWlan_idx(char * wlan_iface_name)
{
	int idx;

	idx = atoi(&wlan_iface_name[4]);
	if (idx >= NUM_WLAN_INTERFACE) {
			printf("invalid wlan interface index number!\n");
			return 0;
	}
	apmib_set_wlanidx(idx);
	apmib_set_vwlanidx(0);

#ifdef MBSSID
	if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
			wlan_iface_name[6] == 'v' && wlan_iface_name[7] == 'a') {
			idx = atoi(&wlan_iface_name[8]);
			if (idx >= NUM_VWLAN_INTERFACE) {
				printf("invalid virtual wlan interface index number!\n");
				return 0;
			}

			apmib_set_vwlanidx (idx+1);
			idx = atoi(&wlan_iface_name[4]);
			apmib_set_wlanidx (idx);
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
			!memcmp(&wlan_iface_name[6], "vxd", 3)) {
		apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
		idx = atoi(&wlan_iface_name[4]);
		apmib_set_wlanidx(idx);
	}
#endif

	//printf("\r\n wlan_iface_name=[%s],wlan_idx=[%u],vwlan_idx=[%u],__[%s-%u]\r\n",wlan_iface_name,wlan_idx,vwlan_idx,__FILE__,__LINE__);

	return 1;
}
#endif


#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
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
#endif

short whichWlanIfIs(PHYBAND_TYPE_T phyBand)
{
	int i;
	int ori_wlan_idx=apmib_get_wlanidx();
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
	
	apmib_set_wlanidx(ori_wlan_idx);
	return ret;		
}

/*
  * web_write_chunked write chunked data to socket
  * note. the inner buf is only TMP_BUF_SIZE
  */
int web_write_chunked(char *fmt, ...)
{
	int len;
	va_list args;
	char *temp;
	if (!fmt)
		return 0;
	temp=(char *)malloc(TMP_BUF_SIZE);
	if(NULL == temp)
		return 0;
	
	memset(temp,0,TMP_BUF_SIZE);
	va_start(args, fmt);
	len=vsnprintf(temp,TMP_BUF_SIZE, fmt, args);
	va_end(args);
	if(len <0)
		return 0;
	cyg_httpd_write_chunked(temp,len);
	if(temp)
	{
		free(temp);
		temp=NULL;
	}
	return len;
}

void translate_control_code_sprintf(char *buffer)
{
	char tmpBuf[200], *p1 = buffer, *p2 = tmpBuf;


	while (*p1) {
		if(*p1 == '\\'){
			memcpy(p2, "\\\\", 2);
			p2 += 2;
		}
		else if (*p1 == '%') {
                        memcpy(p2, "%%", 2);
                        p2 += 2;
		}
		else if(*p1 =='\"'){
			memcpy(p2, "\\\"", 2);
			p2 += 2;
		}
		else if(*p1 =='\''){
			memcpy(p2, "\\\'", 2);
			p2 += 2;
		}
		else if(*p1 =='\`'){
			memcpy(p2, "\\\`", 2);
			p2 += 2;
		}
		else
			*p2++ = *p1;
		p1++;
	}
	*p2 = '\0';

	strcpy(buffer, tmpBuf);
}

void translate_control_code(char *buffer)
{
	char tmpBuf[200], *p1 = buffer, *p2 = tmpBuf;


	while (*p1) {
		if (*p1 == '"') {
			memcpy(p2, "&quot;", 6);
			p2 += 6;
		}
		else if (*p1 == '\x27') {
			memcpy(p2, "&#39;", 5);
			p2 += 5;
		}
		else if (*p1 == '\x5c') {
			memcpy(p2, "&#92;", 5);
			p2 += 5;
		}
		else if (*p1 == '\x3c') {
			memcpy(p2, "&#60;", 5);
			p2 += 5;
		}		
		else if (*p1 == '\x3e') {
			memcpy(p2, "&#62;", 5);
			p2 += 5;
		}
		else if (*p1 == '\x25') {
			memcpy(p2, "&#37;", 5);
			p2 += 5;
		}
		else if (*p1 == '\x26') {
			memcpy(p2, "&#38;", 5);
			p2 += 5;
		}
		else
			*p2++ = *p1;
		p1++;
	}
	*p2 = '\0';

	strcpy(buffer, tmpBuf);
}


int htmlSpecialCharReplace(char*input,char* output,int bufLen)
{
	int i=0,j=0;
	bzero(output,bufLen);
	for(i=0;i<bufLen;i++)
	{
		switch(input[i])
		{
			case '\0':
				output[j]='\0';
				return 0;
			case '<'://&lt;
				if(j+5>=bufLen)
					return -1;
				strcpy(output+j,"&lt;");
				j+=strlen("&lt;");
				break;
			case '>'://&gt;
				if(j+5>=bufLen)
					return -1;
				strcpy(output+j,"&gt;");
				j+=strlen("&gt;");
				break;
			case '"'://&quot;
				if(j+7>=bufLen)
					return -1;
				strcpy(output+j,"&quot;");
				j+=strlen("&quot;");
				break;
#if 0
			case ' '://&nbsp;
				if(j+7>=bufLen)
					return -1;
				strcpy(output+j,"&nbsp;");
				j+=strlen("&nbsp;");
				break;
#endif
			default:
				if(j+2>=bufLen) return -1;
				output[j++]=input[i];
				break;
		}
	}
	return -1;
}
unsigned int getWLAN_ChipVersion()
{
	FILE *stream;
	char buffer[128];
	typedef enum { CHIP_UNKNOWN=0, CHIP_RTL8188C=1, CHIP_RTL8192C=2, CHIP_RTL8192D=3} CHIP_VERSION_T;
	CHIP_VERSION_T chipVersion = CHIP_UNKNOWN;	

	/*FIX ME for ecos*/
	sprintf(buffer,"/proc/wlan%d/mib_rf",apmib_get_wlanidx());
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


int getWdsInfo(char *interface, char *pInfo)
{

#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd;
    struct iwreq wrq;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) <0)
	{
		perror("socket error\n");
		return -1;			
	}
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      /* If no wireless name : no wireless extensions */
	close(skfd);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = MAX_WDS_NUM*sizeof(WDS_INFO_T);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETWDSINFO, &wrq) < 0) {
	close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
   }
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
    memset(pInfo, 0, MAX_WDS_NUM*sizeof(WDS_INFO_T)); 
#endif
#endif
    return 0;
}

int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pStatus;

    if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0){
    	close( skfd );
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
	}
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
	return -1 ;
#endif
#endif
    return 0;
}


#if !defined(HAVE_WPS) && !defined(HAVE_NOWIFI)
int getWlJoinResult(char *interface, unsigned char *res)
{
#ifndef HAVE_NOWIFI
    int skfd;
    struct iwreq wrq;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
	}
    wrq.u.data.pointer = (caddr_t)res;
    wrq.u.data.length = 1;

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQSTATUS, &wrq) < 0){
    	close( skfd );
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
	}
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
#endif
    return 0;
}
#endif

int getWlSiteSurveyRequest(char *interface, int *pStatus)
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;
    unsigned char result;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
#ifdef ECOS_DBG_STAT
		   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
	}
    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0){
    	close( skfd );
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
	}
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

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
#endif
	return 0;
}


int getWlBssInfo(char *interface, bss_info *pInfo)
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
	int skfd=0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1) {
			printf("%s:socket fail\n", __FUNCTION__);
			return -1;
	}
#ifdef ECOS_DBG_STAT
	   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif
	/* Get wireless name */
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
	{
			/* If no wireless name : no wireless extensions */
			// rock: avoid status page error if no wlan interface
			memset(pInfo, 0, sizeof(bss_info));
			//printf("interface=%s\n", interface);
			//printf("%s:iw_get_ext fail\n", __FUNCTION__);
			close( skfd );
#ifdef ECOS_DBG_STAT
				dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

			return 0;
	}

	wrq.u.data.pointer = (caddr_t)pInfo;
	wrq.u.data.length = sizeof(bss_info);

	if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0){
			printf("%s:iw_get_ext2 fail\n", __FUNCTION__);
			close( skfd );
#ifdef ECOS_DBG_STAT
				dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

			return -1;
	}
	close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
	memset(pInfo, 0, sizeof(bss_info));
#endif
#else
	memset(pInfo, 0, sizeof(bss_info));
#endif
#endif
    return 0;
}

int getWlanLink(char* interface)
{
	int connected=-1;
	bss_info bss={0};
	getWlBssInfo(interface, &bss);
		switch (bss.state) {
		case STATE_DISABLED:
			
			break;
		case STATE_IDLE:
			
			break;
		case STATE_STARTED:
			
			break;
		case STATE_CONNECTED:
			connected=1;
			break;
		case STATE_WAITFORKEY:
			
			break;
		case STATE_SCANNING:
			
			break;
		default:
			break;
		}

	return connected;
}

/*Stub Function*/
void run_init_script(char *arg)
{
	/*ADD TODO*/
	return;
}

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

#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
/*if usename and password is set*/
int useAuth()
{
	char user[32];
	memset(user,0,sizeof(user));
	if (!apmib_get(MIB_USER_NAME, (void *)user) ) {
	}
	if(!strcmp(user,""))
		return 0;
	else
		return 1;
}

/*Set user name and password to http server*/
int setAuthUP(char *path, char *username, char *password)
{
	cyg_httpd_auth_table_entry* entry;
	entry=cyg_httpd_auth_entry_from_path(path);
	if(entry!=0)
	{
		memset(entry->auth_username,0,strlen(DUMMY_STRING));
		strcpy(entry->auth_username,username);
		memset(entry->auth_password,0,strlen(DUMMY_STRING));
		strcpy(entry->auth_password,password);
		return 0;
	}
	return -1;
}

void cyg_httpd_auth_init()
{
	char user[32];
	char passwd[32];
	if (!apmib_get(MIB_USER_NAME, (void *)user) ) {
	}
	if (!apmib_get(MIB_USER_PASSWORD, (void *)passwd) ) {
	}
	setAuthUP(WEB_ROOT,user,passwd);
}
#endif
#ifndef ROUTER_BUFFER_SIZE
#define ROUTER_BUFFER_SIZE 512
#endif
/////////////////////////////////////////////////////////////////////////////

int check_wlan_downup(char wlanIndex)
{
	int sock=0,flags=0;
	struct	ifreq	ifr={0};
	snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),"wlan%d",wlanIndex);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		diag_printf("socket error!\n");
		return -1;
	}
	if (ioctl(sock, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) 
	{		
		//diag_printf("SIOCGIFFLAGS error!\n");
		close(sock);
		return (-1);
	}
	close(sock);
	if((ifr.ifr_flags & IFF_UP) != 0)
	{
		return 1;
	}	
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
int getWlStaNum( char *interface, int *num )
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
    int skfd=0;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
#ifdef ECOS_DBG_STAT
		   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

      return -1;
	}
    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTANUM, &wrq) < 0){
    	 close( skfd );
#ifdef ECOS_DBG_STAT
			 dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
	}
    *num  = (int)staNum;

    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
    *num = 0 ;
#endif
#endif
    return 0;
}


int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
#ifndef HAVE_NOWIFI
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
#ifdef ECOS_DBG_STAT
		   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
#ifdef ECOS_DBG_STAT
		  dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);
    memset(pInfo, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0){
    	close( skfd );
#ifdef ECOS_DBG_STAT
			dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

		return -1;
	}
    close( skfd );
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif
#endif
    return 0;
}
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
#ifdef CONFIG_RTK_MESH 
int setWlJoinMesh (char *interface, unsigned char* MeshId, int MeshId_Len, int Channel, int offset, int Reset) 
{ 
    int skfd; 
    struct iwreq wrq; 
    struct 
    {
        unsigned char *meshid;
        int meshid_len, channel, offset, reset; 
    }mesh_identifier; 
  
    skfd = socket(AF_INET, SOCK_DGRAM, 0); 
    
    /* Get wireless name */ 
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
    /* If no wireless name : no wireless extensions */ 
        return -1; 

    mesh_identifier.meshid = MeshId; 
    mesh_identifier.meshid_len = MeshId_Len; 
    mesh_identifier.channel = Channel;
    mesh_identifier.offset = offset;    
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

int getWlMeshNeighborTableInfo (char *interface, unsigned char * pmesh_neighbor_table_info,unsigned short len) 
{ 
	
    int skfd; 
    struct iwreq wrq; 
    skfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		close( skfd ); 
        return -1; 
    }
	
    wrq.u.data.pointer = (caddr_t)pmesh_neighbor_table_info; 
    wrq.u.data.length = len; 

    if (iw_get_ext(skfd, interface, SIOCMESHNEIGHBORTABLEINFO, &wrq) < 0){ 
		close( skfd ); 
        return -1; 
    }
    close( skfd ); 
    return 0; 
	
} 

int getWlMeshRouteTableInfo (char *interface, unsigned char * pmesh_route_table_info,unsigned short len) 
{ 
	
	int skfd; 
	struct iwreq wrq; 
	skfd = socket(AF_INET, SOCK_DGRAM, 0); 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		close( skfd ); 
		return -1; 
	}
	
	wrq.u.data.pointer = (caddr_t)pmesh_route_table_info; 
	wrq.u.data.length = len; 

	if (iw_get_ext(skfd, interface, SIOCMESHROUTETABLEINFO, &wrq) < 0){ 
		close( skfd ); 
		return -1; 
	}
	close( skfd ); 
	return 0; 
	
} 

int getWlMeshPortalTableInfo (char *interface, unsigned char * pmesh_portal_table_info,unsigned short len) 
{ 
	
	int skfd; 
	struct iwreq wrq; 
	skfd = socket(AF_INET, SOCK_DGRAM, 0); 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		close( skfd ); 
		return -1; 
	}
	
	wrq.u.data.pointer = (caddr_t)pmesh_portal_table_info; 
	wrq.u.data.length = len; 

	if (iw_get_ext(skfd, interface, SIOCMESHPORTALTABLEINFO, &wrq) < 0){ 
		close( skfd ); 
		return -1; 
	}
	close( skfd ); 
	return 0; 
	
}

int getWlMeshProxyTableInfo (char *interface, unsigned char * pmesh_proxy_table_info,unsigned short len) 
{ 
	
	int skfd; 
	struct iwreq wrq; 
	skfd = socket(AF_INET, SOCK_DGRAM, 0); 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		close( skfd ); 
		return -1; 
	}
	
	wrq.u.data.pointer = (caddr_t)pmesh_proxy_table_info; 
	wrq.u.data.length = len; 

	if (iw_get_ext(skfd, interface, SIOCMESHPROXYTABLEINFO, &wrq) < 0){ 
		close( skfd ); 
		return -1; 
	}
	close( skfd ); 
	return 0; 
	
}

int getwlMeshRootInfo (char *interface, unsigned char * pmesh_route_info,unsigned short len) 
{ 
	
	int skfd; 
	struct iwreq wrq; 
	skfd = socket(AF_INET, SOCK_DGRAM, 0); 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
		close( skfd ); 
		return -1; 
	}
	
	wrq.u.data.pointer = (caddr_t)pmesh_route_info; 
	wrq.u.data.length = len; 

	if (iw_get_ext(skfd, interface, SIOCMESHROOTINFO, &wrq) < 0){ 
		close( skfd ); 
		return -1; 
	}
	close( skfd ); 
	return 0; 
	
}

#endif 

#if defined(CONFIG_RTL_92D_SUPPORT)
void swapWlanMibSetting(unsigned char wlanifNumA, unsigned char wlanifNumB)
{
	unsigned char *wlanMibBuf=NULL;
	unsigned int totalSize = sizeof(CONFIG_WLAN_SETTING_T)*(NUM_VWLAN_INTERFACE+1); // 4vap+1rpt+1root
	wlanMibBuf = malloc(totalSize); 
	if(wlanMibBuf != NULL)
	{
		memcpy(wlanMibBuf, pMib->wlan[wlanifNumA], totalSize);
		memcpy(pMib->wlan[wlanifNumA], pMib->wlan[wlanifNumB], totalSize);
		memcpy(pMib->wlan[wlanifNumB], wlanMibBuf, totalSize);
	
		free(wlanMibBuf);
	}
	
#if defined(WLAN_PROFILE)
	int profile_enabled_id1, profile_enabled_id2;
	int profile_num_id1, profile_num_id2;
	unsigned char *wlProfileBuf;

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

	apmib_get(MIB_PROFILE_ENABLED1, (void *)&profile_enabled_id1);
	apmib_get(MIB_PROFILE_ENABLED2, (void *)&profile_enabled_id2);
	apmib_get(MIB_PROFILE_NUM1, (void *)&profile_num_id1);
	apmib_get(MIB_PROFILE_NUM2, (void *)&profile_num_id2);

	apmib_set(MIB_PROFILE_ENABLED1, (void *)&profile_enabled_id2);
	apmib_set(MIB_PROFILE_ENABLED2, (void *)&profile_enabled_id1);
	apmib_set(MIB_PROFILE_NUM1, (void *)&profile_num_id2);
	apmib_set(MIB_PROFILE_NUM2, (void *)&profile_num_id1);

	totalSize = sizeof(WLAN_PROFILE_T)*MAX_WLAN_PROFILE_NUM;
	wlProfileBuf = malloc(totalSize);
	if(wlProfileBuf != NULL)
	{
		memcpy(wlProfileBuf, pMib->wlan_profile_arrary1, totalSize);
		memcpy(pMib->wlan_profile_arrary1, pMib->wlan_profile_arrary2, totalSize);
		memcpy(pMib->wlan_profile_arrary2, wlProfileBuf, totalSize);

		free(wlProfileBuf);
	}

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);
	
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
}
#endif

int getMiscData(char *interface, struct _misc_data_ *pData)
{
#ifndef HAVE_NOWIFI
#ifndef NO_ACTION
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
    int skfd;
    struct iwreq wrq;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) <0)
	{
		perror("socket error\n");
		return -1;
	}
#ifdef ECOS_DBG_STAT
		   dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
#endif

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
	/* If no wireless name : no wireless extensions */
	close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

        return -1;
    }

    wrq.u.data.pointer = (caddr_t)pData;
    wrq.u.data.length = sizeof(struct _misc_data_);

    if (iw_get_ext(skfd, interface, SIOCGMISCDATA, &wrq) < 0) {
	close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

	return -1;
    }

    close(skfd);
#ifdef ECOS_DBG_STAT
		dbg_stat_add(dbg_athttpd_index, DBG_TYPE_SOCKET, DBG_ACTION_DEL, 0);
#endif

#else
    memset(pData, 0, sizeof(struct _misc_data_)); 
#endif
#else
    memset(pData, 0, sizeof(struct _misc_data_)); 
#endif
#endif
    return 0;
}
