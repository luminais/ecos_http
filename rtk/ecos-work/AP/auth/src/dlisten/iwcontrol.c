#include <stdio.h>
#ifndef __ECOS
#include <sys/time.h>
#include <signal.h>
#include <dirent.h>
#endif
#ifdef __ECOS
#include <network.h>
#include "../../../apmib/apmib.h"
#endif

#ifndef __ASUS_DVD__
#define RTL_WPA2_CLIENT

#define RTL_WPA2
#define AUTO_CONFIG
#endif

#ifdef CONFIG_WPS_RTK
#define WIFI_SIMPLE_CONFIG
#else
#undef WIFI_SIMPLE_CONFIG
#endif

#include "iwcommon.h"
#include "../../include/1x_fifo.h"
#include "../../include/1x_ioctl.h"

#ifdef ECOS_DBG_STAT
#include "../../../system/sys_utility.h"
#endif

#ifndef __ASUS_DVD__

#define INTERVAL  1
#define WLAN_CHR_MISC
cyg_flag_t iw_flag;

//----------------------------------------------------------
// Local variables
//----------------------------------------------------------
static char *pidfile="/var/run/iwcontrol.pid";



Dot1x_RTLDListener RTLDListenerAuth[MAX_WLAN_INTF];
Dot1x_RTLDListener RTLDListenerIapp;
static char dlisten_SendBuf[RWFIFOSIZE];

int link_auth = FALSE;
int link_iapp = FALSE;
int iw_wlan_num = 0 ;
char iw_wlan_tbl[MAX_WLAN_INTF][20] ;
#ifdef AUTO_CONFIG
static int link_autoconf = FALSE;
static Dot1x_RTLDListener RTLDListenerAutoconf[MAX_WLAN_INTF];
#endif

#ifdef WIFI_SIMPLE_CONFIG
 int link_wscd = FALSE;
 Dot1x_RTLDListener RTLDListenerWscd[MAX_WLAN_INTF];
#endif

#ifdef WLAN_CHR_MISC
static int wl_chr_fd;
#endif

#ifdef ECOS_DBG_STAT
extern int dbg_iw_index;
#endif
#ifdef HAVE_HS2_SUPPORT
 int link_hs2 = FALSE;
 Dot1x_RTLDListener RTLDListenerHS2[MAX_WLAN_INTF];
#endif
#ifdef HAVE_SYSTEM_REINIT
static int iw_quitting = 0;
#endif

//----------------------------------------------------------
// Functions
//----------------------------------------------------------
int get_info()
{
	int skfd = -1;	/* generic raw socket desc */

	/* Create a channel to the NET kernel */
	if((skfd = iwcontrol_sockets_open()) < 0)
	{
		perror("socket");
		exit(-1);
	}

	return skfd;
}

/*
  Initialize the SendBuf
  _________________________________________________
  | pid (4 bytes) | fifo type (1 byte) | data (*) |
  -------------------------------------------------
*/
void iw_init_sendBuf(char *ptr)
{
	pid_t pid;
	pid = getpid();
	iw_L2N((long)pid, ptr);
	*(ptr + sizeof(long)) = FIFO_TYPE_DLISTEN;
}
#ifndef __ECOS
void iw_init_fifo(Dot1x_RTLDListener *listen, char *fifo_name)
{

	while(1){
		if((listen->WriteFIFO = open(fifo_name, O_WRONLY, 0)) < 0)
		{
			//iw_message(MESS_DBG_CONTROL, "open fifo %s error: %s", fifo_name, strerror(errno));
			iw_message(MESS_DBG_CONTROL, "wait %s to create", fifo_name);
			sleep(1);
			//exit(0);
		}
		else
			break;
	}
}
#endif
int ProcessRequestEvent(char *wlan_name)
{
	int outlen = 0;
	int retVal = 0;
	int alloc_len = 0;
	unsigned char szEvent[64];
	int iSend = FALSE;	// iSend = 1 for test
	int iRet = 0;
	int i;
	char *ptr;
	msg_hdr_t *msg;
#ifdef HAVE_AUTH
	extern cyg_handle_t auth0_mbox_hdl;
#endif
#ifdef HAVE_IAPP
	extern cyg_handle_t iapp_mbox_hdl;
#endif
#ifdef AUTO_CONFIG
	int isAutoconfEvt=0;
#endif

#ifdef WIFI_SIMPLE_CONFIG
	int isWscdEvt = FALSE;
	extern cyg_handle_t wscd0_mbox_hdl;
#endif

#ifdef HAVE_HS2_SUPPORT
	int isHS2Evt = FALSE;
#ifdef __ECOS
	extern cyg_handle_t hs2_mbox_hdl;
#endif
#endif
	// Get message from wlan ioctl
	if(RequestIndication(RTLDListenerAuth[0].Iffd, wlan_name, &dlisten_SendBuf[5], &outlen) < 0)
	{
		iw_message(MESS_DBG_CONTROL, "RequestIndication return Fail");
		return 0;
	}

	// Process message
	if(dlisten_SendBuf[5] != 0)
	{
		memset(szEvent, 0, sizeof szEvent);
		switch(dlisten_SendBuf[5])
		{
		case	DOT11_EVENT_ASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "ASSOCIATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_REASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "REASSOCIATION_IND");
			iSend = TRUE;
			break;

		case 	DOT11_EVENT_AUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_REAUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "REAUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_DEAUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "DEAUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_DISASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "DISASSOCIATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_MIC_FAILURE:
			sprintf(szEvent, (char*)"Receive Event %s", "MIC_FAILURE");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAPOLSTART:
			sprintf(szEvent, (char*)"Receive Event %s", "EAPOL_START");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAP_PACKET:
			sprintf(szEvent, (char*)"Receive Event %s", "EAP_PACKET");
#ifdef WIFI_SIMPLE_CONFIG
			isWscdEvt = TRUE;
#endif
			iSend = TRUE;
			break;
#ifdef RTL_WPA2
		case	DOT11_EVENT_EAPOLSTART_PREAUTH:
			sprintf(szEvent, (char*)"Receive Event %s", "EAPOLSTART_PREAUTH");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAP_PACKET_PREAUTH:
			sprintf(szEvent, (char*)"Receive Event %s", "EAP_PACKET_PREAUTH");
			iSend = TRUE;
			break;
#endif

#ifdef RTL_WPA2_CLIENT
		case	DOT11_EVENT_WPA2_MULTICAST_CIPHER:
			sprintf(szEvent, (char*)"Receive Event %s", "WPA2_MULTICAST_CIPHER");
			iSend = TRUE;
			break;
#endif

		case	DOT11_EVENT_WPA_MULTICAST_CIPHER:
			sprintf(szEvent, (char*)"Receive Event %s", "WPA_MULTICAST_CIPHER");
			iSend = TRUE;
			break;

#ifdef AUTO_CONFIG
		case	DOT11_EVENT_AUTOCONF_ASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_ASSOC_IND");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_ASSOCIATION_CONFIRM:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_ASSOC_CONFIRM");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_PACKET:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_PACKET");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_LINK_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_LINK_IND");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;
#endif

#ifdef WIFI_SIMPLE_CONFIG
		case	DOT11_EVENT_WSC_PIN_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_PIN_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_SWITCH_MODE:	// for P2P_SUPPORT
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_P2P_IND");
			isWscdEvt = TRUE;
			break;			
		case	DOT11_EVENT_WSC_STOP:	// for P2P_SUPPORT
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_P2P_STOP");
			isWscdEvt = TRUE;
			break;			
	/* support  Assigned MAC Addr,Assigned SSID,dymanic change STA's PIN code, 2011-0505 */				
		case	DOT11_EVENT_WSC_SET_MY_PIN:	
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_SET_MY_PIN");
			isWscdEvt = TRUE;
			break;			
		case	DOT11_EVENT_WSC_SPEC_SSID:	
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_SPEC_SSID");
			isWscdEvt = TRUE;
			break;			
		case	DOT11_EVENT_WSC_SPEC_MAC_IND:	
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_SPEC_MAC");
			isWscdEvt = TRUE;
			break;			
	/* support  Assigned MAC Addr,Assigned SSID,dymanic change STA's PIN code, 2011-0505 */				
#ifdef CONFIG_IWPRIV_INTF
		case	DOT11_EVENT_WSC_START_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_START_IND");
			isWscdEvt = TRUE;
			break;
		//EV_MODE, EV_STATUS, EV_MEHOD, EV_STEP, EV_OOB
		case	DOT11_EVENT_WSC_MODE_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_MODE_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_STATUS_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_STATUS_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_METHOD_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_METHOD_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_STEP_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_STEP_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_OOB_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_OOB_IND");
			isWscdEvt = TRUE;
			break;
#endif  //ifdef CONFIG_IWPRIV_INTF
		case	DOT11_EVENT_WSC_PROBE_REQ_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_PROBE_REQ_IND");
			isWscdEvt = TRUE;
			break;

		case	DOT11_EVENT_WSC_ASSOC_REQ_IE_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_ASSOC_REQ_IE_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_RM_PBC_STA:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_ASSOC_REQ_IE_IND");
			isWscdEvt = TRUE;
			break;
#endif
#ifdef HAVE_HS2_SUPPORT
		case DOT11_EVENT_GAS_INIT_REQ:
		case DOT11_EVENT_GAS_COMEBACK_REQ:	
			sprintf(szEvent, (char*)"Receive Event %s", "GAS_QUERY_IND");
            isHS2Evt = TRUE;
            break;
#endif            
		default:
			sprintf(szEvent, (char*)"Receive Invalid or Unhandled Event %d",
				dlisten_SendBuf[5]);
			iSend = FALSE;
			break;
		}

		iw_message(MESS_DBG_CONTROL, "[iwcontrol]: %s", szEvent);
		if(iSend)
		{
#if 0
#ifdef AUTO_CONFIG
			if (link_autoconf && isAutoconfEvt) {
				for(i=0; i < link_autoconf; i++){
					if(!strcmp(RTLDListenerAutoconf[i].wlanName,wlan_name)){
						if((iRet = write(RTLDListenerAutoconf[i].WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
							iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
						else
							iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
					}
				}
				goto ret_process;
			}
#endif
#endif
#ifdef HAVE_AUTH
			if(link_auth)
			{
				for(i=0; i < link_auth; i++)
					{

					if(!strcmp(RTLDListenerAuth[i].wlanName,wlan_name))
						{
#ifdef __ECOS
						alloc_len = outlen + sizeof(msg_hdr_t)+WLAN_INTF_LEN;
						ptr = malloc(alloc_len);
						
						if(ptr != NULL){
							msg = (msg_hdr_t *) ptr;
							msg->msg_buf = ptr + sizeof(msg_hdr_t);
							msg->msg_len = outlen+WLAN_INTF_LEN;
							memcpy(msg->msg_buf,wlan_name,strlen(wlan_name));
							msg->msg_buf[strlen(wlan_name)] = '\0';
							char * dest =msg->msg_buf+WLAN_INTF_LEN;
							memcpy(dest,dlisten_SendBuf,outlen);
							if(cyg_mbox_tryput(auth0_mbox_hdl,msg) == 0){
								free(msg);
							}
						}
#else	
						if((iRet = write(RTLDListenerAuth[i].WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
							iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
						else
							iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
#endif
					}
				}
			}
#endif
#ifdef HAVE_IAPP 

			if(link_iapp)
			{
			#ifdef __ECOS
				alloc_len = outlen + sizeof(msg_hdr_t);
					ptr = malloc(alloc_len);
					
					if(ptr != NULL){
						msg = (msg_hdr_t *) ptr;
						msg->msg_buf = ptr + sizeof(msg_hdr_t);
						msg->msg_len = outlen;
						
						char * dest =msg->msg_buf;
						memcpy(dest,dlisten_SendBuf,outlen);
						if(cyg_mbox_tryput(iapp_mbox_hdl,msg) == 0){
							free(msg);
						}
					}
			#else
				if((iRet = write(RTLDListenerIapp.WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
					iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
				else
					iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
			#endif
			}
#endif
#ifdef AUTO_CONFIG
ret_process:
#endif
			retVal = (dlisten_SendBuf[6] == TRUE)? TRUE : FALSE;	//If more event
		}
#if 1
#ifdef WIFI_SIMPLE_CONFIG
		if (isWscdEvt)
		{
			if (link_wscd && isWscdEvt) {
				for(i=0; i < link_wscd; i++){
					if(!strcmp(RTLDListenerWscd[i].wlanName,wlan_name)){
						#ifdef __ECOS
						alloc_len = outlen + sizeof(msg_hdr_t)+WLAN_INTF_LEN;
						ptr = malloc(alloc_len);
						
						if(ptr != NULL){
							msg = (msg_hdr_t *) ptr;
							msg->msg_buf = ptr + sizeof(msg_hdr_t);
							msg->msg_len = outlen+WLAN_INTF_LEN;
							memcpy(msg->msg_buf,wlan_name,strlen(wlan_name));
							msg->msg_buf[strlen(wlan_name)]= 0;
							memcpy(msg->msg_buf+WLAN_INTF_LEN,dlisten_SendBuf,outlen);
							if(cyg_mbox_tryput(wscd0_mbox_hdl,msg) == 0){
								
								free(msg);
								
								/* 解决概率性wps连接不上问题*/
								extern int wscd_state;
								unsigned int wscd0_mbox_hdl_peek;
								wscd0_mbox_hdl_peek = cyg_mbox_peek(wscd0_mbox_hdl);

								printf ("%s %d wscd0_mbox_hdl_peek=%d=\n", __FUNCTION__, __LINE__, wscd0_mbox_hdl_peek);	
								if (wscd_state == 1 && wscd0_mbox_hdl_peek > 1) /* 一般mbox 最大长度为10, 这里>1  判断也可以*/
								{
									printf ("%s %d \n", __FUNCTION__, __LINE__);	
									cleanup_wscd_mbox();										
								}							
							}
							else{
								extern cyg_flag_t wsc_flag;
 								cyg_flag_setbits(&wsc_flag, 0x1);
							}
						}
#else	
						if((iRet = write(RTLDListenerWscd[i].WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
							iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
						else
							iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
#endif
					}
				}
			}
			retVal = (dlisten_SendBuf[6] == TRUE)? TRUE : FALSE;	//If more event
		}
#endif
#endif
#ifdef HAVE_HS2_SUPPORT
		if (isHS2Evt)
		{
#ifdef __ECOS
			if(link_hs2){
				
				for(i=0; i < link_hs2; i++){
            		if(!strcmp(RTLDListenerHS2[i].wlanName,wlan_name)){
	            		alloc_len = outlen + sizeof(msg_hdr_t)+WLAN_INTF_LEN;
						ptr = malloc(alloc_len);
					
						if(ptr != NULL){
							msg = (msg_hdr_t *) ptr;
							msg->msg_buf = ptr + sizeof(msg_hdr_t);
							msg->msg_len = outlen+WLAN_INTF_LEN;
							memcpy(msg->msg_buf,wlan_name,strlen(wlan_name));
							msg->msg_buf[strlen(wlan_name)]= 0;
							memcpy(msg->msg_buf+WLAN_INTF_LEN,dlisten_SendBuf,outlen);
							if(cyg_mbox_tryput(hs2_mbox_hdl,msg) == 0){
								free(msg);
							}
						}
					}
				}
		}
	
#else
			if(link_hs2 && isHS2Evt)
            {
            	for(i=0; i < link_hs2; i++)
            	{
            		if(!strcmp(RTLDListenerHS2[i].wlanName,wlan_name))
            		{
						//printf("i=%d,name=%s\n",i, wlan_name);
                		if((iRet = write(RTLDListenerHS2[i].WriteFIFO, dlisten_SendBuf, RWFIFOSIZE)) < 0)
                    		iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
                		else
                    		iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
                    }
                }
            }
#endif
			retVal = (dlisten_SendBuf[6] == TRUE)? TRUE : FALSE;    //If more event
		}
#endif
	}

	return retVal;
}


void RequestEvent(/*int sig_no*/)
{
	u_long	ulMoreEvent;
	int i ;
	for(i=0 ;i < iw_wlan_num; i ++){
		do {
			ulMoreEvent = ProcessRequestEvent(iw_wlan_tbl[i]);
		} while(ulMoreEvent);
	}
}
#ifndef __ECOS
// david ----------------------------------------
static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0)
	{
		printf("Unable to open pidfile %s\n", pidfile);
	}
	else
	{
		lockf(pid_fd, F_LOCK, 0);
	}

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL)
	{
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}

	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

//-----------------------------------------------
/* parsing var pid file for fifo create */
int parsing_var_pid(Dot1x_RTLDListener *auth, Dot1x_RTLDListener *autoconf, Dot1x_RTLDListener *wscd)
{
       DIR *dir;
        struct dirent *next;
        int i;
        dir = opendir("/var/run");
        if (!dir) {
                printf("Cannot open %s", "/var/run");
                return 0;
        }
        while ((next = readdir(dir)) != NULL){
                iw_message(MESS_DBG_CONTROL,"iwcontrol:/var/run/%s\n", next->d_name);
		if(!strncmp(next->d_name, "auth", strlen("auth"))){
			sscanf(next->d_name, "auth-%s.pid", auth[link_auth].wlanName);
// when repeater mode is used, its name length may be extended to 9 chars, 2005-8-8 david
//			auth[link_auth++].wlanName[5] = '\0' ;
			for (i=0; i<16; i++) {
				if (auth[link_auth].wlanName[i] == '.')
					auth[link_auth].wlanName[i] = 0;
			}
			link_auth++;
			iw_message(MESS_DBG_CONTROL,"auth[%d].wlanName=%s\n", link_auth-1, auth[link_auth-1].wlanName);
		}
		if(!strncmp(next->d_name, "iapp", strlen("iapp"))){
			link_iapp = TRUE;
			iw_message(MESS_DBG_CONTROL,"link_iapp =true\n");
		}
#ifdef AUTO_CONFIG
		if(!strncmp(next->d_name, "autoconf", strlen("autoconf"))){
			sscanf(next->d_name, "autoconf-%s.pid", autoconf[link_autoconf].wlanName);
			autoconf[link_autoconf++].wlanName[5] = '\0' ;
			iw_message(MESS_DBG_CONTROL,"autoconf[%d].wlanName=%s\n", link_autoconf-1, autoconf[link_autoconf-1].wlanName);
		}
#endif
#ifdef WIFI_SIMPLE_CONFIG
#if		1	//def FOR_DUAL_BAND
		if(!strncmp(next->d_name, "wscd", strlen("wscd"))){

			if(!strncmp(next->d_name, "wscd-wlan0-wlan1.pid", strlen("wscd-wlan0-wlan1.pid")))
			{			
				iw_message(MESS_DBG_CONTROL,"next->d_name=%s\n", next->d_name);	

				strcpy(wscd[0].wlanName,"wlan0");
				strcpy(wscd[1].wlanName,"wlan1");
				//printf("AP mode-->> \n\n");
				link_wscd=2;				
			}
            else
            {
	            sscanf(next->d_name, "wscd-%s.pid", wscd[link_wscd].wlanName);
	            //wscd[link_wscd++].wlanName[5] = '\0' ;
	            for (i=0; i<16; i++) {
	                    if (wscd[link_wscd].wlanName[i] == '.')
	                            wscd[link_wscd].wlanName[i] = 0;
            	}
            	link_wscd++;
				
            	iw_message(MESS_DBG_CONTROL,"wscd[%d].wlanName=%s\n", link_wscd-1, wscd[link_wscd-1].wlanName);
            }

			int idx;
			for(idx=0 ; idx<link_wscd ; idx++)
				iw_message(MESS_DBG_CONTROL,"wscd[%d].wlanName=%s\n", idx, wscd[idx].wlanName);

		}
#else
		if(!strncmp(next->d_name, "wscd", strlen("wscd"))){
			sscanf(next->d_name, "wscd-%s.pid", wscd[link_wscd].wlanName);
			//wscd[link_wscd++].wlanName[5] = '\0' ;
			for (i=0; i<16; i++) {
				if (wscd[link_wscd].wlanName[i] == '.')
					wscd[link_wscd].wlanName[i] = 0;
			}
			link_wscd++;
			iw_message(MESS_DBG_CONTROL,"wscd[%d].wlanName=%s\n", link_wscd-1, wscd[link_wscd-1].wlanName);
		}
#endif		
#endif
	}
	return 0;
}
#endif
#ifdef AUTO_CONFIG
#define AUTOCONFIG_FIFO 	"/var/autoconf-%s.fifo"
#endif

#ifdef WIFI_SIMPLE_CONFIG
#define	WSCD_FIFO			"/var/wscd-%s.fifo"
#endif
int parse_mbox_hdl(Dot1x_RTLDListener *auth, Dot1x_RTLDListener *autoconf, Dot1x_RTLDListener *wscd)
{
	int auth_enabled,iapp_disabled,wsc_disabled,wsc0,wsc1;
	wsc0 = 1;
	wsc1 = 1;
#ifdef HAVE_AUTH
	/*get mib?  get thread info?*/
	auth_enabled = 0;
	apmib_get_ext(MIB_WLAN_ENABLE_1X,&auth_enabled,0,0);
	if(auth_enabled){
		extern cyg_handle_t auth0_mbox_hdl;
		strcpy(auth[link_auth].wlanName,"wlan0");
		auth[link_auth].mbox_hdl = auth0_mbox_hdl;
		wsc0 = 0;
	}
	link_auth++;
#if 0
	auth_enabled = 0;
	apmib_get_ext(MIB_WLAN_ENABLE_1X,&auth_enabled,1,0);
	if(auth_enabled){
		strcpy(auth[link_auth].wlanName,"wlan1");
		auth[link_auth].mbox_hdl = auth1_mbox_hdl;
		wsc1 = 0;
	}
	link_auth++;
#endif
#endif
#ifdef HAVE_IAPP
#if 0
	if(link_iapp)
		RTLDListenerIapp.mbox_hdl = iapp_mbox_hdl;
#endif
#endif
#if 0
#ifdef AUTO_CONFIG/*To Do*/
	if(!strncmp(next->d_name, "autoconf", strlen("autoconf"))){
		sscanf(next->d_name, "autoconf-%s.pid", autoconf[link_autoconf].wlanName);
		autoconf[link_autoconf++].wlanName[5] = '\0' ;
		iw_message(MESS_DBG_CONTROL,"autoconf[%d].wlanName=%s\n", link_autoconf-1, autoconf[link_autoconf-1].wlanName);
	}
#endif
#endif
#if 1
#ifdef WIFI_SIMPLE_CONFIG
#ifdef HAVE_WPS
	extern cyg_handle_t wscd0_mbox_hdl;
	wsc_disabled = 1;
	if(wsc0){
		apmib_get_ext(MIB_WLAN_WSC_DISABLE,&wsc_disabled,0,0);
		if(!wsc_disabled){
			strcpy(wscd[link_wscd].wlanName,"wlan0");
			wscd[link_wscd].mbox_hdl = wscd0_mbox_hdl;
			link_wscd++;
		}
	}
#if 0
	wsc_disabled = 1;
	if(wsc1){
		apmib_get_ext(MIB_WLAN_WSC_DISABLE,&wsc_disabled,0,0);
		if(!wsc_disabled){
			strcpy(wscd[link_wscd].wlanName,"wlan1");
			wscd[link_wscd].mbox_hdl = wscd1_mbox_hdl;
			link_wscd++;
		}
	}
#endif
#endif
#endif
#endif
	return 0;

}
int get_iw_param(char *param)
{
	char *p,*s;
	s = param;
	while(1){
		if(*s != ' ' || *s == NULL)
			break;
		s++;
	}
	if(*s == NULL)
		return -1;
	while(1){
		p = strpbrk(s," ");
		if(p == NULL)
			break;
		memcpy(iw_wlan_tbl[iw_wlan_num],s,p-s);
		iw_wlan_num++;
		s = p;
		while(1){
			if(*s != ' ' || *s == NULL)
				break;
			s++;
		}
		if(*s == NULL)
			break;
	}
	if(*s == NULL)
		return 0;
	p = s;
	while(1){
		if(*s == NULL)
			break;
		s++;
	}
	memcpy(iw_wlan_tbl[iw_wlan_num],p,s-p);
	iw_wlan_num++;
	return 0;
}
#ifdef __ECOS
int iw_main(cyg_addrword_t data)
#else
int main(int argc, char *argv[])
#endif
{
	int i,val;
#ifndef __ECOS
	char *iapp_fifo = "/var/iapp.fifo";
	char fifo_buf[30];
	int poll = 0; //david
#endif
	//char *param;
	//param = (char *)data;
#ifndef __ECOS
	// destroy old process
	{
		FILE *fp;
		char line[20];
		pid_t pid;

		if((fp = fopen(pidfile, "r")) != NULL)
		{
			fgets(line, sizeof(line), fp);
			if(sscanf(line, "%d", &pid))
			{
				if(pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
		}
	}

	setsid(); // david, requested from abocom

	// depend on arguments to decide link fifo
	//printf("argv=");
	for(i=1; i<argc; i++)
	{
		//printf("%s",argv[i]);		
		if(!strncmp("wlan", argv[i], strlen("wlan"))){
			if(wlan_num >= MAX_WLAN_INTF){
				printf("Only  %d Wlan interace is supported\n", MAX_WLAN_INTF);
				return -1;
			}
			printf("Register to %s\n",argv[i]);
			strcpy(wlan_tbl[wlan_num], argv[i]);
			wlan_num++ ;
		}
// david ------------------------------------------------------
		else if(!strncmp("poll", argv[i], strlen("poll"))){
			poll = 1;
		}
//-------------------------------------------------------------
		else
		{
			printf("Usage: %s [wlan0] [wlan1] ... \n", argv[0]);
			return -1;
		}
	}
#endif
	//if(get_iw_param(param) < 0){
		//diag_printf("iwcontrol: invalide arguments!\n");
		//return -1;
	//}
	//printf("\n");
	if(iw_wlan_num == 0){
		printf("At least one wlan interface should be binded\n");
		return -1 ;
	}
#ifdef __ECOS
	//parse_mbox_hdl(RTLDListenerAuth, RTLDListenerAutoconf, RTLDListenerWscd);
#else
	/* parsing /var/iapp.pid or /var/auth*.pid , /var/autoconf*.conf */
	if(parsing_var_pid(RTLDListenerAuth, RTLDListenerAutoconf, RTLDListenerWscd) < 0){
		printf("parsing pid failed\n");
		return -1;
	}
#endif
#ifndef __ECOS
	// david ------------------------------------------
	// delete old fifo and create filo
	{
		int pid_fd;

		pid_fd = pidfile_acquire(pidfile);
		if(daemon(0,1) == -1)
		{
			printf("fork iwcontrol error!\n");
			exit(1);
		}
		pidfile_write_release(pid_fd);
	}
	//---------------------------------------------------
#endif
	// init send buffer pid header
	iw_init_sendBuf(dlisten_SendBuf);
#ifndef __ECOS
	// init fifo and socket
	if(link_auth){
		for(i=0 ; i < link_auth; i++){
			sprintf(fifo_buf, DAEMON_FIFO, RTLDListenerAuth[i].wlanName);
			iw_message(MESS_DBG_CONTROL,"open auth fifo %s\n", fifo_buf);
			iw_init_fifo(&RTLDListenerAuth[i], fifo_buf);
		}
	}
	if(link_iapp){
		iw_message(MESS_DBG_CONTROL,"open iapp fifo %s\n", iapp_fifo);
		iw_init_fifo(&RTLDListenerIapp, iapp_fifo);
	}

#ifdef AUTO_CONFIG
	if(link_autoconf){
		for(i=0 ; i < link_autoconf; i++){
			sprintf(fifo_buf, AUTOCONFIG_FIFO, RTLDListenerAutoconf[i].wlanName);
			iw_message(MESS_DBG_CONTROL,"open autoconfig fifo %s\n", fifo_buf);
			iw_init_fifo(&RTLDListenerAutoconf[i], fifo_buf);
		}
	}
#endif

#ifdef WIFI_SIMPLE_CONFIG
	if(link_wscd){
		for(i=0 ; i < link_wscd; i++){
			sprintf(fifo_buf, WSCD_FIFO, RTLDListenerWscd[i].wlanName);
			iw_message(MESS_DBG_CONTROL,"open wscd fifo %s\n", fifo_buf);
			iw_init_fifo(&RTLDListenerWscd[i], fifo_buf);
		}
	}
#endif
#endif

	RTLDListenerAuth[0].Iffd = get_info();
	if(RTLDListenerAuth[0].Iffd <= 0)
	{
		perror("Socket fd return 0");
		exit(0);
	}

	#ifdef ECOS_DBG_STAT
	dbg_stat_add(dbg_iw_index, DBG_TYPE_SOCKET, DBG_ACTION_ADD, 0);
	#endif

#ifndef __ECOS
	// infinite loop
#ifdef WLAN_CHR_MISC
	if (!poll)	// david
	{
		int fdflags, idx=0, wlan0_up=0, wlan1_up=0;
		char dev_name[20];
		for(i=0; i<wlan_num; i++)
		{
			if (strncmp("wlan0", wlan_tbl[i], 5) == 0) {
				if (wlan0_up)
					continue;
				else
					wlan0_up = 1;
				idx = 0;
			}

			if (strncmp("wlan1", wlan_tbl[i], 5) == 0) {
				if (wlan1_up)
					continue;
				else
					wlan1_up = 1;
				idx = 1;
			}

			sprintf(dev_name, "/dev/wl_chr%d", idx);

			if((wl_chr_fd = open(dev_name, O_RDWR, 0)) < 0)
			{
				int retval;
				if (idx == 0){
					retval = RegisterPID(RTLDListenerAuth[0].Iffd, "wlan0");
					printf("iwcontrol RegisterPID to (wlan0)\n");					
				}else{
					retval = RegisterPID(RTLDListenerAuth[0].Iffd, "wlan1");
					printf("iwcontrol RegisterPID to (wlan1)\n");
				}

				if (retval > 0)
					signal(SIGIO, RequestEvent);
				else
				{
					printf("Warning: unable to open an wl_chr device and PID registration fail.(wlan%d)(%d %s)\n",idx,__LINE__ , __FUNCTION__);
					//exit(1);
				}
			}
			else
			{
				signal(SIGIO, RequestEvent);

				fcntl(wl_chr_fd, F_SETOWN, getpid());
				fdflags = fcntl(wl_chr_fd, F_GETFL);
				fcntl(wl_chr_fd, F_SETFL, fdflags | FASYNC);
			}
		}

		while(1)
			pause();
	}
#endif
#endif
	cyg_flag_init(&iw_flag);
	while( 1 )
	{
		val = cyg_flag_wait(&iw_flag, 0x7f, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
		if(val & 0x01)
			RequestEvent(/*SIGIO*/);
#ifdef HAVE_SYSTEM_REINIT
		if(val & 0x02){ // cleanup for reinit
			iw_cleanup();
			cyg_flag_destroy(&iw_flag);
			iw_quitting = 0;
			break;
		}
#endif
		//sleep(INTERVAL);
	}

	return 0;
}


#else
//#include "1x_common.h"
//#include "wireless.h"

#define MAXDATALEN      1560	// jimmylin: org:256, enlarge for pass EAP packet by event queue

int read_wlan_evt(	int skfd, char *ifname, char *out)
{
	struct iwreq wrq;
	DOT11_REQUEST *req;

  	/* Get wireless name */
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
  	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

	req = (DOT11_REQUEST *)malloc(MAXDATALEN);
	wrq.u.data.pointer = (caddr_t)req;
	req->EventId = DOT11_EVENT_REQUEST;
	wrq.u.data.length = sizeof(DOT11_REQUEST);

  	if (ioctl(skfd, SIOCGIWIND, &wrq) < 0) {
    	// If no wireless name : no wireless extensions
		free(req);
		strerror(errno);
   		return(-1);
	}
  	else
		memcpy(&out[5], wrq.u.data.pointer, wrq.u.data.length);

	free(req);

	if (out[5] != 0) {
		out[4] = FIFO_TYPE_DLISTEN;
		return wrq.u.data.length+5;;
	}

	return 0;
}
#endif

/*add for iw init*/
#ifdef WIFI_SIMPLE_CONFIG

#ifndef nvram_safe_get
#define nvram_safe_get(name) (nvram_get(name) ? : "")
#endif

void iw_init()
{   
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
	char ifname_5G[32] = {0};
#endif
    char ifname_2G[32] = {0};
    
	if(!strcmp(nvram_safe_get("wl0_mode"), "ap"))
	{
	    #ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
		strcpy(ifname_2G,"wlan1");
        #else
		strcpy(ifname_2G,"wlan0");
        #endif
	}
	else
	{
	    #ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
		strcpy(ifname_2G, "wlan1-vxd0");
        #else
        strcpy(ifname_2G, "wlan0-vxd0");
        #endif
	}
    
    #ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
    if(!strcmp(nvram_safe_get("wl1_mode"), "ap"))
    {
        strcpy(ifname_5G, "wlan0");
    }
    else
    {
        strcpy(ifname_5G, "wlan0-vxd0");
    }
    #endif
    
	printf("%s,%d,ifname2G:%s,ifname5G:%s\n",__func__,__LINE__,ifname_2G, ifname_5G);
    #ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
	sprintf(iw_wlan_tbl[0],ifname_5G);
	sprintf(RTLDListenerWscd[0].wlanName,ifname_5G);
    sprintf(iw_wlan_tbl[1],ifname_2G);
	sprintf(RTLDListenerWscd[1].wlanName,ifname_2G);
	link_wscd = 2;
	iw_wlan_num = 2;
    #else
    sprintf(iw_wlan_tbl[0],ifname_2G);
	sprintf(RTLDListenerWscd[0].wlanName,ifname_2G);
	link_wscd = 1;
	iw_wlan_num = 1;
    #endif
}
#endif
/*add end*/

#define CYGNUM_IWCONTROL_THREAD_PRIORITY 16
unsigned char iw_stack[8*1024];
cyg_handle_t iw_thread_handle;
cyg_thread iw_thread_obj;

void creat_iw()
{
	/* Create the thread */
		cyg_thread_create(CYGNUM_IWCONTROL_THREAD_PRIORITY,
			      iw_main,
			      0,
			      "iw",
			      &iw_stack,
			      sizeof(iw_stack),
			      &iw_thread_handle,
			      &iw_thread_obj);
		/* Let the thread run when the scheduler starts */
		cyg_thread_resume(iw_thread_handle);
}
#ifdef HAVE_SYSTEM_REINIT
void kill_iw()
{
	cyg_thread_info tinfo;
	if(get_thread_info_by_name("iw", &tinfo)){
		iw_quitting = 1;
		cyg_flag_setbits(&iw_flag, 0x02);
		while(iw_quitting){
			cyg_thread_delay(5);
		}
		/*cleanup thread info*/
		iw_exit();
	}
}
void iw_cleanup()
{
/*It shoudle be cleanup here,when add new feature in iwcontrol*/
	close(RTLDListenerAuth[0].Iffd);
	iw_wlan_num = 0 ;
	memset(iw_wlan_tbl,0,MAX_WLAN_INTF*20);
	memset(dlisten_SendBuf,0,RWFIFOSIZE);
	memset((char *)RTLDListenerAuth,0,sizeof(Dot1x_RTLDListener)*MAX_WLAN_INTF);
	memset((char *)&RTLDListenerIapp,0,sizeof(Dot1x_RTLDListener));

	link_auth = FALSE;
	link_iapp = FALSE;
	
#ifdef WIFI_SIMPLE_CONFIG
	link_wscd = FALSE;
	memset((char *)RTLDListenerWscd,0,sizeof(Dot1x_RTLDListener)*MAX_WLAN_INTF);
#endif

#ifdef HAVE_HS2_SUPPORT
	link_hs2 = FALSE;
	memset((char *)RTLDListenerHS2,0,sizeof(Dot1x_RTLDListener)*MAX_WLAN_INTF);
#endif
    

}
void iw_exit()
{
	cyg_thread_kill(iw_thread_handle);
	cyg_thread_delete(iw_thread_handle);
}
#endif


