#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#ifndef __ECOS
#include <linux/wireless.h>
#include <netpacket/packet.h>
#else
#include <net/if.h>			/* for IFNAMSIZ and co... */
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif



#include "hs2.h"

pHS2CTX pGlobalHS2;

//HS2 FIX IE
unsigned char HS2IE[]={0x50,0x6f,0x9a,0x10,0x00}; //  OI: 0x50 6F 9A, Typ = 10 (Hotspot 2.0 element)
unsigned char ADVTIE[]={0x20,0x00}; // Advertisement Protocol (Element ID = 108, p.477 Table 8-54), 
         							// Query Response Info = 0x20, Protocol ID = 0x00 = ANQP, p.682 in 802.11-2012 pdf
unsigned char HS2_HDR[]={0x50,0x6f,0x9a,0x11}; // Refer to Hotspot Spec R2, P.26, Figure 4 (Hotspot 2.0 ANQP Element Format, OI + Type)

//SIGMA TEST PATTERN
unsigned char venuename_id1[2][92]={
	{0x65,0x6e,0x67,0x57,0x69,0x2d,0x46,0x69,0x20,0x41,
	 0x6c,0x6c,0x69,0x61,0x6e,0x63,0x65,0x0a,0x32,0x39,
	 0x38,0x39,0x20,0x43,0x6f,0x70,0x70,0x65,0x72,0x20,
	 0x52,0x6f,0x61,0x64,0x0a,0x53,0x61,0x6e,0x74,0x61,
	 0x20,0x43,0x6c,0x61,0x72,0x61,0x2c,0x20,0x43,0x41,
	 0x20,0x39,0x35,0x30,0x35,0x31,0x2c,0x20,0x55,0x53,
	 0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00 },
	{0x63,0x68,0x69,0x57,0x69,0x2d,0x46,0x69,0xe8,0x81,
	 0x94,0xe7,0x9b,0x9f,0xe5,0xae,0x9e,0xe9,0xaa,0x8c,
	 0xe5,0xae,0xa4,0x0a,0xe4,0xba,0x8c,0xe4,0xb9,0x9d,
	 0xe5,0x85,0xab,0xe4,0xb9,0x9d,0xe5,0xb9,0xb4,0xe5,
	 0xba,0x93,0xe6,0x9f,0x8f,0xe8,0xb7,0xaf,0x0a,0xe5,
	 0x9c,0xa3,0xe5,0x85,0x8b,0xe6,0x8b,0x89,0xe6,0x8b,
	 0x89,0x2c,0x20,0xe5,0x8a,0xa0,0xe5,0x88,0xa9,0xe7,
	 0xa6,0x8f,0xe5,0xb0,0xbc,0xe4,0xba,0x9a,0x39,0x35,
	 0x30,0x35,0x31,0x2c,0x20,0xe7,0xbe,0x8e,0xe5,0x9b,
	 0xbd,0x00}};
unsigned char opername_id1[2][20]={
	{0x65,0x6e,0x67,0x57,0x69,0x2d,0x46,0x69,0x20,0x41,0x6c,0x6c,0x69,0x61,0x6e,0x63,0x65,0x0,0x0,0x0},
	{0x63,0x68,0x69,0x57,0x69,0x2d,0x46,0x69,0xe8,0x81,0x94,0xe7,0x9b,0x9f,0x00,0x00,0x00,0x0,0x0,0x0}};
#ifdef __ECOS
#define WLAN_INF_LEN 16
#define CYGNUM_HS2_THREAD_PRIORITY 16
	
	unsigned char hs2_stack[16*1024];
	cyg_handle_t hs2_thread_handle;
	cyg_thread hs2_thread_obj;
	extern unsigned char hs2_wlan0_conf[];
	
	extern unsigned char hs2_wlan1_conf[];
	
	cyg_mbox hs2_mbox;
	cyg_handle_t hs2_mbox_hdl;
	
	static int hs2_run_flag=0;
	typedef struct _msg_hdr
	{
		int msg_len;
		char* msg_buf;
	}msg_hdr_t;
#endif

#ifndef __ECOS
static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		printf("pid==%d\n", getpid());
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}
#endif
static int get_mac_addr(int sk, char *interface, char *addr)
{
    struct ifreq ifr;

    strcpy(ifr.ifr_name, interface);

    if ( ioctl(sk, SIOCGIFHWADDR, &ifr) < 0) {
        printf("ioctl(SIOCGIFHWADDR) failed!\n");
        return -1;
    }

    memcpy(addr, ifr.ifr_hwaddr.sa_data, 6);
    return 0;
}

static int wlioctl_get_info(char *interface, char *data, unsigned int *len, int id, int flag)
{
    int skfd;
    int retVal=0;
    struct iwreq wrq;
	DOT11_REQUEST         req;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0) {
        printf("socket() error!\n");
        return -1;
    }

	wrq.u.data.pointer = (caddr_t)&req;
	req.EventId = id;
    strcpy(wrq.ifr_name, interface);
	wrq.u.data.length = sizeof(DOT11_REQUEST);


    if (ioctl(skfd, SIOCGIWIND, &wrq) < 0)	{
        printf("%s %d failed\n",__FUNCTION__,__LINE__);
        retVal = -1;
    }
	else	{
		memcpy(data, wrq.u.data.pointer, wrq.u.data.length);
		*len = wrq.u.data.length;
	}

    close(skfd);
    return retVal;
}

static int wlioctl_set_hs2_ie(char *interface, char *data, int len, int id, int flag)
{
	int skfd;
	int retVal=0;
	struct iwreq wrq;
	DOT11_SET_HS2 set_ie;	

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("socket() error!\n");
		return -1;
	}

	set_ie.EventId = id;
	set_ie.Flag = flag;
	set_ie.RSNIELen = len;
	memcpy(set_ie.RSNIE, data, len);

	strcpy(wrq.ifr_name, interface);	
	wrq.u.data.pointer = (caddr_t)&set_ie;
	wrq.u.data.length = sizeof(DOT11_SET_HS2);

	if (ioctl(skfd, SIOCGIWIND, &wrq) < 0)	{
		printf("%s %d failed\n",__FUNCTION__,__LINE__);
		retVal = -1;
	}

	close(skfd);
	return retVal;	
}

//static int wlioctl_send_hs2_rsp(char *interface, char *data, int len, int id)
static int wlioctl_send_hs2_rsp(char *interface, DOT11_HS2_GAS_RSP *gas_rsp, int id)
{
	int skfd;
	int retVal=0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("socket() error!\n");
		return -1;
	}

	gas_rsp->EventId = id;;
	//gas_rsp.Rsplen = len;
	//memcpy(gas_rsp.Rsp, data, len);

	strcpy(wrq.ifr_name, interface);	
	wrq.u.data.pointer = (caddr_t)gas_rsp;
	wrq.u.data.length = sizeof(DOT11_HS2_GAS_RSP);

	if (ioctl(skfd, SIOCGIWIND, &wrq) < 0)	{
		printf("%s %d failed\n",__FUNCTION__,__LINE__);
		retVal = -1;
	}

	close(skfd);
	return retVal;	
}

static int hs2_fill_ie(pHS2CTX pHS2)
{
	//unsigned char tmpaddr[MACADDRLEN];
	unsigned char zeromac[MACADDRLEN]={0x00,0x00,0x00,0x00,0x00,0x00};
	int i, j;
#ifdef __ECOS
	char wlan_inf[WLAN_INF_LEN];
#endif
	HS2_DEBUG_INFO("enter hs2_fill_ie\n");
	HS2_DEBUG_INFO("cfg_num====%d\n", pHS2->cfg_num);	
	for (i=0;i<pHS2->cfg_num;i++)	{
		//1.fill hs2 indication element
		memcpy(&pHS2->hs2conf[i].hs2_ie[0], HS2IE, sizeof(HS2IE));

		
		if (pHS2->hs2conf[i].dgaf_disable == 1) // enable / disable DGAF (Downstream Group-Addressed Forwarding)
			pHS2->hs2conf[i].hs2_ie[4] |= 0x01;
		else
			pHS2->hs2conf[i].hs2_ie[4] &= 0xFE;
				
		pHS2->hs2conf[i].hs2_ielen += sizeof(HS2IE);

		//2.fill interworking information element
		pHS2->hs2conf[i].iw_ie[0] = ((pHS2->hs2conf[i].internet & 0x01) << 4) | (pHS2->hs2conf[i].ant & 0x0f);
		pHS2->hs2conf[i].iw_ie[1] = pHS2->hs2conf[i].venue_group;
		pHS2->hs2conf[i].iw_ie[2] = pHS2->hs2conf[i].venue_type;
		pHS2->hs2conf[i].iw_ielen += 3;

		if (memcmp(pHS2->hs2conf[i].hessid, zeromac, MACADDRLEN))	{
			printf("hessid=%02x%02x%02x%02x%02x%02x\n", pHS2->hs2conf[i].hessid[0], pHS2->hs2conf[i].hessid[1], pHS2->hs2conf[i].hessid[2], pHS2->hs2conf[i].hessid[3], pHS2->hs2conf[i].hessid[4], pHS2->hs2conf[i].hessid[5]);
			memcpy(&pHS2->hs2conf[i].iw_ie[3], pHS2->hs2conf[i].hessid, MACADDRLEN);	
			pHS2->hs2conf[i].iw_ielen += MACADDRLEN;
		}		
		//3.fill advertisement element
		memcpy(&pHS2->hs2conf[i].advt_ie[0], ADVTIE, sizeof(ADVTIE));
		pHS2->hs2conf[i].advt_ielen += sizeof(ADVTIE);
	
		//4.fill roaming consortium information element
		if (pHS2->hs2conf[i].roi_cnt != 0) {
			pHS2->hs2conf[i].roam_ielen += 2;
			for (j=0;j<pHS2->hs2conf[i].roi_cnt;j++)	{
				if (j == 0)	{
					pHS2->hs2conf[i].roam_ie[1] = pHS2->hs2conf[i].roi_len[j] & 0x0f;
					memcpy(&pHS2->hs2conf[i].roam_ie[2], pHS2->hs2conf[i].roi[j], pHS2->hs2conf[i].roi_len[j]);
					pHS2->hs2conf[i].roam_ielen += pHS2->hs2conf[i].roi_len[j];
				}
				else if (j == 1)	{
					pHS2->hs2conf[i].roam_ie[1] |= ((pHS2->hs2conf[i].roi_len[j] & 0x0f) << 4);
					memcpy(&pHS2->hs2conf[i].roam_ie[2+pHS2->hs2conf[i].roi_len[0]], pHS2->hs2conf[i].roi[j], pHS2->hs2conf[i].roi_len[j]);
					pHS2->hs2conf[i].roam_ielen += pHS2->hs2conf[i].roi_len[j];
				}
				else if (j == 2)	{
				    memcpy(&pHS2->hs2conf[i].roam_ie[2+pHS2->hs2conf[i].roi_len[0]+pHS2->hs2conf[i].roi_len[1]], pHS2->hs2conf[i].roi[j], pHS2->hs2conf[i].roi_len[j]);
					pHS2->hs2conf[i].roam_ielen += pHS2->hs2conf[i].roi_len[j];
		        } 	
			}
			if (pHS2->hs2conf[i].roi_cnt <= 3)
				pHS2->hs2conf[i].roam_ie[0] = 0;
			else
				pHS2->hs2conf[i].roam_ie[0]	= pHS2->hs2conf[i].roi_cnt-3;
		}

#ifdef TIMEZONE_SUPPORT
		//5.fill time advertisement ie and time zone ie
		pHS2->hs2conf[i].timeadvt_ie[0] = 0;
	    pHS2->hs2conf[i].timeadvt_ielen = 1;
		memset(&pHS2->hs2conf[i].timezone_ie[0], 0, 4);
	    pHS2->hs2conf[i].timezone_ielen = 4;
#endif
		
		//6.set ie to wlan driver by ioctl
		if (pHS2->hs2conf[i].iw_enable == 1) {
			unsigned char tmpcmd[50];
			HS2_DEBUG_INFO("Set wlan0 hs2 ie to driver\n");
			if (pHS2->hs2conf[i].hs2_ielen != 0)
				wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, &pHS2->hs2conf[i].hs2_ie[0], pHS2->hs2conf[i].hs2_ielen, 
						DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_HS2);
			if (pHS2->hs2conf[i].iw_ielen != 0)
				wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, &pHS2->hs2conf[i].iw_ie[0], pHS2->hs2conf[i].iw_ielen, 
						DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_INTERWORKING);	
			if (pHS2->hs2conf[i].advt_ielen != 0)
				wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, &pHS2->hs2conf[i].advt_ie[0], pHS2->hs2conf[i].advt_ielen, 
						DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_ADVT_PROTO);	
			if (pHS2->hs2conf[i].roam_ielen != 0)
				wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, &pHS2->hs2conf[i].roam_ie[0], pHS2->hs2conf[i].roam_ielen, 
						DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_ROAMING);		
#ifdef TIMEZONE_SUPPORT
			wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, pHS2->hs2conf[i].timeadvt_ie, pHS2->hs2conf[i].timeadvt_ielen,
				        DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_TIMEADVT);
			wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, pHS2->hs2conf[i].timezone_ie, pHS2->hs2conf[i].timezone_ielen,
				        DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_TIMEZONE);
#endif
			printf("\n\n\n\nAPP set Proxy ARP\n\n\n\n");
			wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, &pHS2->hs2conf[i].proxy_arp, 1,
		                DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_PROXYARP);			

			wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, &pHS2->hs2conf[i].ICMPv4ECHO, 1,
		                DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_ICMPv4ECHO);			
			
			printf("\n\n\n\nl2_inspect=%d\n\n\n\n",pHS2->hs2conf[i].l2_inspect);
			// l2_inspect = 1, redirect packet to portal
			if (pHS2->hs2conf[i].l2_inspect == 1) {
				printf("set block_relay = 3\n");
#ifdef __ECOS
				sprintf(wlan_inf,"%s",pHS2->hs2conf[i].wlan_iface);
				RunSystemCmd(NULL,"iwpriv",wlan_inf,"set_mib","block_relay=3","");
#else
				sprintf(tmpcmd, "iwpriv %s set_mib block_relay=3", pHS2->hs2conf[i].wlan_iface);	
				system(tmpcmd);
#endif
				printf("set redir_mac\n");
				memset(tmpcmd, 0x00, sizeof(tmpcmd));
#ifdef __ECOS
				sprintf(wlan_inf,"%s",pHS2->hs2conf[i].wlan_iface);
				sprintf(tmpcmd, "redir_mac=%02x%02x%02x%02x%02x%02x", pHS2->hs2conf[i].redir_mac[0], pHS2->hs2conf[i].redir_mac[1], pHS2->hs2conf[i].redir_mac[2], pHS2->hs2conf[i].redir_mac[3], pHS2->hs2conf[i].redir_mac[4], pHS2->hs2conf[i].redir_mac[5]);
				RunSystemCmd(NULL,"iwpriv",wlan_inf,"set_mib",tmpcmd,"");
#else
				sprintf(tmpcmd, "iwpriv %s set_mib redir_mac=%02x%02x%02x%02x%02x%02x", pHS2->hs2conf[i].wlan_iface, pHS2->hs2conf[i].redir_mac[0], pHS2->hs2conf[i].redir_mac[1], pHS2->hs2conf[i].redir_mac[2], pHS2->hs2conf[i].redir_mac[3], pHS2->hs2conf[i].redir_mac[4], pHS2->hs2conf[i].redir_mac[5]);
				system(tmpcmd);
#endif
			} else {
				printf("set block_relay = 0\n");
#ifdef __ECOS
				sprintf(wlan_inf,"%s",pHS2->hs2conf[i].wlan_iface);
				RunSystemCmd(NULL,"iwpriv",wlan_inf,"set_mib","block_relay=0","");
#else
				sprintf(tmpcmd, "iwpriv %s set_mib block_relay=0", pHS2->hs2conf[i].wlan_iface);	
				system(tmpcmd);
#endif
			}
			//wlioctl_set_hs2_ie(pHS2->hs2conf[i].wlan_iface, &pHS2->hs2conf[i].mmpdu_limit,sizeof(unsigned int),
			//			DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_MMPDULIMIT);
		}
	}
}
#ifndef __ECOS	
static int hs2_init_fifo(pHS2CTX pHS2)
{
	int flags, i;
	struct stat status;
	for(i=0;i<pHS2->cfg_num;i++)	{
		sprintf(pHS2->hs2conf[i].fifo_name, "%s-%s.fifo", DEFAULT_FIFO_FILENAME, pHS2->hs2conf[i].wlan_iface);

		if(stat(pHS2->hs2conf[i].fifo_name, &status) == 0)
			unlink(pHS2->hs2conf[i].fifo_name);
		if((mkfifo(pHS2->hs2conf[i].fifo_name, FILE_MODE) < 0))	{
			printf("mkfifo %s fifo error: %s!\n", pHS2->hs2conf[i].fifo_name, strerror(errno));
			return -1;
		}
		pHS2->hs2conf[i].readfifo = open(pHS2->hs2conf[i].fifo_name, O_RDONLY, 0);
	
	}
	return 0;
}
#endif
static pSTA_CTX get_avaiable_sta(pHS2CTX pHS2, int fifo_index)
{
    int i;
	
    for (i=0;i<MAX_STA_NUM;i++)	{
		if (pHS2->hs2conf[fifo_index].sta[i] == NULL)	{
	        pHS2->hs2conf[fifo_index].sta[i] = calloc(1, sizeof(STA_CTX));
		    memset(pHS2->hs2conf[fifo_index].sta[i], 0, sizeof(STA_CTX));
			return pHS2->hs2conf[fifo_index].sta[i];
        }
	}
    	
	return NULL;
}
	
static void reset_sta(pHS2CTX pHS2, STA_CTX *psta, int fifo_index)
{
    int i;
    for (i=0;i<MAX_STA_NUM;i++)	{
        if (psta == pHS2->hs2conf[fifo_index].sta[i])	{
            free(psta);
            pHS2->hs2conf[fifo_index].sta[i] = NULL;
        }
    }
}

static pSTA_CTX get_comeback_sta(pHS2CTX pHS2, unsigned char comeback_token, unsigned char *comeback_mac, int fifo_index)
{
	int i;
	for (i=0;i<MAX_STA_NUM;i++)	{
		if (pHS2->hs2conf[fifo_index].sta[i])	{
			if (!memcmp(pHS2->hs2conf[fifo_index].sta[i]->gas_rsp.MACAddr, comeback_mac, MACADDRLEN))	{
				pHS2->hs2conf[fifo_index].sta[i]->gas_rsp.Dialog_token = comeback_token;
				return pHS2->hs2conf[fifo_index].sta[i];
				//printf("Dialog_token=%d, comeback_token=%d\n",pHS2->hs2conf[fifo_index].sta[i]->gas_rsp.Dialog_token,comeback_token);
				//if (pHS2->hs2conf[fifo_index].sta[i]->gas_rsp.Dialog_token == comeback_token)	{
				//	printf("compare token ok\n");
				//	return pHS2->hs2conf[fifo_index].sta[i];
				//}
			}
		}
	}
	return NULL;
}

void tz_transfer(char *time_zone)
{
	if(strcmp(time_zone,"12 1") == 0)
	{
		strcpy(time_zone, "MHT12");
	}
	else if(strcmp(time_zone,"11 1") == 0)
    {
        strcpy(time_zone, "SST11");
    }
	else if(strcmp(time_zone,"10 1") == 0)
    {
        strcpy(time_zone, "HAST10");
    }
	else if(strcmp(time_zone,"9 1") == 0)
    {
        strcpy(time_zone, "AKDT9");
    }
	else if(strcmp(time_zone,"8 1") == 0)
    {
        strcpy(time_zone, "PST8");
    }
	else if(strcmp(time_zone,"7 1") == 0)
    {
        strcpy(time_zone, "MST7");
    }
	else if(strcmp(time_zone,"7 2") == 0)
    {
        strcpy(time_zone, "PDT7");
    }
	else if(strncmp(time_zone,"6", 1) == 0)
    {
        strcpy(time_zone, "CST6");
    }
	else if(strcmp(time_zone,"5 1") == 0)
    {
        strcpy(time_zone, "ECT5");
    }
	else if(strcmp(time_zone,"5 2") == 0)
    {
        strcpy(time_zone, "EST5");
    }
	else if(strcmp(time_zone,"5 3") == 0)
    {
        strcpy(time_zone, "EDT5");
    }
	else if(strcmp(time_zone,"4 1") == 0)
    {
        strcpy(time_zone, "EDT4");
    }
	else if(strcmp(time_zone,"4 2") == 0)
    {
        strcpy(time_zone, "VET4");
    }
	else if(strcmp(time_zone,"4 3") == 0)
    {
        strcpy(time_zone, "AST4");
    }
	else if(strcmp(time_zone,"3 1") == 0)
    {
        strcpy(time_zone, "NDT3");
    }
	else if(strcmp(time_zone,"3 2") == 0)
    {
        strcpy(time_zone, "BRST3");
    }
	else if(strcmp(time_zone,"3 3") == 0)
    {
        strcpy(time_zone, "ART3");
    }
	else if(strcmp(time_zone,"2 1") == 0)
    {
        strcpy(time_zone, "MAT2");
    }
	else if(strcmp(time_zone,"1 1") == 0)
    {
        strcpy(time_zone, "AZOST1");
    }
	else if(strncmp(time_zone,"0",1) == 0)
    {
        strcpy(time_zone, "GMT0");
    }
	else if(strncmp(time_zone,"-1",2) == 0)
    {
        strcpy(time_zone, "CEST-1");
    }
	else if(strcmp(time_zone,"-2 1") == 0)
    {
        strcpy(time_zone, "EEST-2");
    }
	else if(strcmp(time_zone,"-2 2") == 0)
    {
        strcpy(time_zone, "EEST-2");
    }
	else if(strcmp(time_zone,"-2 3") == 0)
    {
        strcpy(time_zone, "EEST-2");
    }
	else if(strcmp(time_zone,"-2 4") == 0)
    {
        strcpy(time_zone, "CAT-2");
    }
	else if(strcmp(time_zone,"-2 5") == 0)
    {
        strcpy(time_zone, "EEST-2");
    }
	else if(strcmp(time_zone,"-2 6") == 0)
    {
        strcpy(time_zone, "IST-2");
    }
	else if(strcmp(time_zone,"-3 1") == 0)
    {
        strcpy(time_zone, "AST-3");
    }
	else if(strcmp(time_zone,"-3 2") == 0)
    {
        strcpy(time_zone, "MSD-3");
    }
	else if(strcmp(time_zone,"-3 3") == 0)
    {
        strcpy(time_zone, "WAT-3");
    }
	else if(strcmp(time_zone,"-3 4") == 0)
    {
        strcpy(time_zone, "IRST-3");
    }
	else if(strcmp(time_zone,"-4 1") == 0)
    {
        strcpy(time_zone, "GST-4");
    }
	else if(strcmp(time_zone,"-4 2") == 0)
    {
        strcpy(time_zone, "GET-4");
    }
	else if(strcmp(time_zone,"-4 3") == 0)
    {
        strcpy(time_zone, "AFT-4");
    }
	else if(strcmp(time_zone,"-5 1") == 0)
    {
        strcpy(time_zone, "YEKT-5");
    }
	else if(strcmp(time_zone,"-5 2") == 0)
    {
        strcpy(time_zone, "PKT-5");
    }
	else if(strcmp(time_zone,"-5 3") == 0)
    {
        strcpy(time_zone, "IST-5");
    }
	else if(strcmp(time_zone,"-6 1") == 0)
    {
        strcpy(time_zone, "ALMT-6");
    }
	else if(strcmp(time_zone,"-6 2") == 0)
    {
        strcpy(time_zone, "IST-6");
    }
	else if(strcmp(time_zone,"-7 1") == 0)
    {
        strcpy(time_zone, "ICT-7");
    }
	else if(strcmp(time_zone,"-8 1") == 0)
    {
        strcpy(time_zone, "CST-8");
    }
	else if(strcmp(time_zone,"-8 2") == 0)
    {
        strcpy(time_zone, "WST-8");
    }
	else if(strcmp(time_zone,"-8 3") == 0)
    {
        strcpy(time_zone, "SGT-8");
    }
	else if(strcmp(time_zone,"-8 4") == 0)
    {
        strcpy(time_zone, "CST-8");
    }
	else if(strcmp(time_zone,"-9 1") == 0)
    {
        strcpy(time_zone, "JST-9");
    }
	else if(strcmp(time_zone,"-9 2") == 0)
    {
        strcpy(time_zone, "KST-9");
    }
	else if(strcmp(time_zone,"-9 3") == 0)
    {
        strcpy(time_zone, "YAKT-9");
    }
	else if(strcmp(time_zone,"-9 4") == 0)
    {
        strcpy(time_zone, "CDT-4");
    }
	else if(strcmp(time_zone,"-9 5") == 0)
    {
        strcpy(time_zone, "CST-9");
    }
	else if(strcmp(time_zone,"-10 1") == 0)
    {
        strcpy(time_zone, "EST-10");
    }
	else if(strcmp(time_zone,"-10 2") == 0)
    {
        strcpy(time_zone, "EDT-10");
    }
	else if(strcmp(time_zone,"-10 3") == 0)
    {
        strcpy(time_zone, "PGT-10");
    }
	else if(strcmp(time_zone,"-10 4") == 0)
    {
        strcpy(time_zone, "EDT-10");
    }
	else if(strcmp(time_zone,"-10 5") == 0)
    {
        strcpy(time_zone, "VLAT-10");
    }
	else if(strcmp(time_zone,"-11 1") == 0)
    {
        strcpy(time_zone, "MAGT-11");
    }
	else if(strcmp(time_zone,"-12 1") == 0)
    {
        strcpy(time_zone, "NZDT-12");
    }
	else if(strcmp(time_zone,"-12 2") == 0)
    {
        strcpy(time_zone, "FJST-12");
    }
	else
	{
		printf("no timezone match !!!\n");
		strcpy(time_zone, "GMT0");
	}
}

#ifdef TIMEZONE_SUPPORT

void check_timeie(pHS2CTX pHS2, unsigned char *ifname, int fifo_index)
{
#define NTPTMP_FILE "/tmp/ntp_tmp"
#define TZTMP_FILE  "/tmp/timezone"

	if(isFileExist(NTPTMP_FILE))	{
        FILE *fp,*fp2=NULL;
        unsigned char ntptmp_str[100], buffer[100], pbuf[10];
		unsigned short yeartmp;		
		
        memset(ntptmp_str,0x00,sizeof(ntptmp_str));

        fp=fopen(NTPTMP_FILE, "r");
		fp2=fopen(TZTMP_FILE, "r");
        if((fp!=NULL) && (fp2!=NULL))	{
            fgets((char *)ntptmp_str,sizeof(ntptmp_str),fp);
            fclose(fp);
			
			if (pHS2->hs2conf[fifo_index].utc_countdown == 0)	{
				if(strlen((char *)ntptmp_str) != 0)	{
					// success
					struct tm *tm_time;
				    time_t current_time;
					unsigned int tmplen;

	                time(&current_time);
		            tm_time = localtime(&current_time);

					pHS2->hs2conf[fifo_index].timeadvt_ie[0] = 2;

					//get TSF secs
					wlioctl_get_info(ifname, &pHS2->hs2conf[fifo_index].tsf, &tmplen,
						DOT11_EVENT_HS2_GET_TSF, 0);

					
					current_time -= pHS2->hs2conf[fifo_index].tsf;
					//get UTC time
					tm_time = localtime(&current_time);

					sprintf(buffer,"%d", (tm_time->tm_year+ 1900));
					yeartmp = convert_atob_sh(buffer, 4);
					memcpy(&pHS2->hs2conf[fifo_index].timeadvt_ie[1], &yeartmp, 2);
					sprintf(buffer,"%d", (tm_time->tm_mon+1));
					pbuf[0] = convert_atob(buffer, 2);
					memcpy(&pHS2->hs2conf[fifo_index].timeadvt_ie[3], pbuf, 1);
					sprintf(buffer,"%d", (tm_time->tm_mday));
					pbuf[0] = convert_atob(buffer, 2);
					memcpy(&pHS2->hs2conf[fifo_index].timeadvt_ie[4], pbuf, 1);
					sprintf(buffer,"%d", (tm_time->tm_hour));
					pbuf[0] = convert_atob(buffer, 2);
					memcpy(&pHS2->hs2conf[fifo_index].timeadvt_ie[5], pbuf, 1);
					sprintf(buffer,"%d", (tm_time->tm_min));
					pbuf[0] = convert_atob(buffer, 2);
					memcpy(&pHS2->hs2conf[fifo_index].timeadvt_ie[6], pbuf, 1);
					sprintf(buffer,"%d", (tm_time->tm_sec));
					pbuf[0] = convert_atob(buffer, 2);
					memcpy(&pHS2->hs2conf[fifo_index].timeadvt_ie[7], pbuf, 1);
					memset(&pHS2->hs2conf[fifo_index].timeadvt_ie[8], 0, 11);
					pHS2->hs2conf[fifo_index].timeadvt_ielen = 20;

					//get timezone
					{
						fgets((char *)ntptmp_str,sizeof(ntptmp_str),fp2);
						fclose(fp2);

				        if(strlen((char *)ntptmp_str) != 0)
						{
							*(ntptmp_str+strlen(ntptmp_str)-1) = '\0';
							tz_transfer(ntptmp_str);
							strcpy(&pHS2->hs2conf[fifo_index].timezone_ie[0], ntptmp_str);
							pHS2->hs2conf[fifo_index].timezone_ielen = strlen(ntptmp_str);
						}					
					}

					//set time advertisement ie and time zone ie
					wlioctl_set_hs2_ie(ifname, pHS2->hs2conf[fifo_index].timeadvt_ie, pHS2->hs2conf[fifo_index].timeadvt_ielen,
			            DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_TIMEADVT);
					wlioctl_set_hs2_ie(ifname, pHS2->hs2conf[fifo_index].timezone_ie, pHS2->hs2conf[fifo_index].timezone_ielen,
					    DOT11_EVENT_HS2_SET_IE, SET_IE_FLAG_TIMEZONE);
				}
			}
			else
			{
				pHS2->hs2conf[fifo_index].utc_countdown--;
			}
		}
	}
	else	{
		pHS2->hs2conf[fifo_index].timeadvt_ie[0] = 0;
		pHS2->hs2conf[fifo_index].timeadvt_ielen = 1;
		memset(&pHS2->hs2conf[fifo_index].timezone_ie[0], 0, 4);
        pHS2->hs2conf[fifo_index].timezone_ielen = 4;
    }
}
#endif

static void timeout_handler(pHS2CTX pHS2)
{
	int i, k;

	//check sta alive	
	for (k=0;k<pHS2->cfg_num;k++)	{
	    for (i=0;i<MAX_STA_NUM;i++)		{
			if (pHS2->hs2conf[k].sta[i] != NULL)	{
				if (pHS2->hs2conf[k].sta[i]->expire_time == 0)	{
					printf("sta[%d]:MAC:%02x%02x%02x%02x%02x%02x, timeout in %s interface\n", i, pHS2->hs2conf[k].sta[i]->gas_rsp.MACAddr[0], pHS2->hs2conf[k].sta[i]->gas_rsp.MACAddr[1], pHS2->hs2conf[k].sta[i]->gas_rsp.MACAddr[2], pHS2->hs2conf[k].sta[i]->gas_rsp.MACAddr[3], pHS2->hs2conf[k].sta[i]->gas_rsp.MACAddr[4], pHS2->hs2conf[k].sta[i]->gas_rsp.MACAddr[5], pHS2->hs2conf[k].wlan_iface);
	        		free(pHS2->hs2conf[k].sta[i]);
		        	pHS2->hs2conf[k].sta[i] = NULL;
				}
				else	{
		        	pHS2->hs2conf[k].sta[i]->expire_time--;
				}
			}
        }
    }

#ifdef TIMEZONE_SUPPORT
	//check UTC time
	for (k=0;k<pHS2->cfg_num;k++) {
		if (pHS2->hs2conf[k].timeadvt_ie[0] != 2)
			check_timeie(pHS2, pHS2->hs2conf[k].wlan_iface, k);
	}
#endif
}

static int handle_hs_query_list(pHS2CTX pHS2, unsigned char *Query_ID, int  len, unsigned char *Query_Rsp, int fifo_index)
{
    int		rsp_len = 0, i = 0, cur_len = 0, realm_data_len = 0, method_len = 0;
	struct  proto_port		*proto;
	struct	hs2_realm		*realm;
	struct	hs2_eap_method  *eap_method;
	unsigned char statusCode;

	//printf("hs query:len=%d\n",len);
   
	// parser hs2 anqp payload 
	while(len > 0)
    {
        switch(*Query_ID)
		{
			case HS2_CAP:
				Query_Rsp[rsp_len++] = VENDOR_LIST & 0xff;
                Query_Rsp[rsp_len++] = (VENDOR_LIST >> 8) & 0xff;
                cur_len = rsp_len;
                rsp_len += 2;	
				memcpy(&Query_Rsp[rsp_len], HS2_HDR, sizeof(HS2_HDR));
	            rsp_len += sizeof(HS2_HDR);
				Query_Rsp[rsp_len++] = HS2_CAP;
				Query_Rsp[rsp_len++] = 0;
				Query_Rsp[rsp_len++] = HS2_CAP;
				if (pHS2->hs2conf[fifo_index].op_cnt != 0)
	                Query_Rsp[rsp_len++] = HS2_OP_NAME;
				if (pHS2->hs2conf[fifo_index].wan_ie.waninfo != 0)
	                Query_Rsp[rsp_len++] = HS2_WAN;
				if (pHS2->hs2conf[fifo_index].proto != 0)
	                Query_Rsp[rsp_len++] = HS2_CONN_CAP;
                Query_Rsp[rsp_len++] = OP_BAND;
				Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				break;
			case HS2_OP_NAME: // Operator Friendly Name element
				Query_Rsp[rsp_len++] = VENDOR_LIST & 0xff;
                Query_Rsp[rsp_len++] = (VENDOR_LIST >> 8) & 0xff;
                cur_len = rsp_len;
                rsp_len += 2;   
                memcpy(&Query_Rsp[rsp_len], HS2_HDR, sizeof(HS2_HDR));
                rsp_len += sizeof(HS2_HDR);
				Query_Rsp[rsp_len++] = HS2_OP_NAME;
				Query_Rsp[rsp_len++] = 0;
				for (i=0;i<pHS2->hs2conf[fifo_index].op_cnt;i++) {
                    Query_Rsp[rsp_len++] = strlen(&pHS2->hs2conf[fifo_index].op_name[i][3])+3;
                    memcpy(&Query_Rsp[rsp_len], pHS2->hs2conf[fifo_index].op_name[i], 3);
                    rsp_len += 3;
                    memcpy(&Query_Rsp[rsp_len], &pHS2->hs2conf[fifo_index].op_name[i][3], strlen(&pHS2->hs2conf[fifo_index].op_name[i][3]));
                    rsp_len += strlen(&pHS2->hs2conf[fifo_index].op_name[i][3]);
                }	
				//memcpy(&Query_Rsp[rsp_len], hotspot_operator_name, sizeof(hotspot_operator_name));
				//rsp_len += sizeof(hotspot_operator_name);
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				break;
			case HS2_WAN:
				Query_Rsp[rsp_len++] = VENDOR_LIST & 0xff;
                Query_Rsp[rsp_len++] = (VENDOR_LIST >> 8) & 0xff;
                cur_len = rsp_len;
                rsp_len += 2;
                memcpy(&Query_Rsp[rsp_len], HS2_HDR, sizeof(HS2_HDR));
                rsp_len += sizeof(HS2_HDR);
                Query_Rsp[rsp_len++] = HS2_WAN;
				Query_Rsp[rsp_len++] = 0;
				if (pHS2->hs2conf[fifo_index].wan_ie.dlspeed == pHS2->hs2conf[fifo_index].wan_ie.ulspeed) {
					pHS2->hs2conf[fifo_index].wan_ie.waninfo |= 0x04;
				}
				Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].wan_ie.waninfo;
				Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].wan_ie.dlspeed & 0xff;
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.dlspeed >> 8) & 0xff;
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.dlspeed >> 16) & 0xff;
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.dlspeed >> 24) & 0xff;
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.ulspeed) & 0xff;	
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.ulspeed >> 8) & 0xff;
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.ulspeed >> 16) & 0xff;
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.ulspeed >> 24) & 0xff;
				Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].wan_ie.dlload;
				Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].wan_ie.ulload;
				Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].wan_ie.lmd & 0xff;
				Query_Rsp[rsp_len++] = (pHS2->hs2conf[fifo_index].wan_ie.lmd >> 8) & 0xff;
				
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				break;
			case HS2_CONN_CAP:
				Query_Rsp[rsp_len++] = VENDOR_LIST & 0xff;
                Query_Rsp[rsp_len++] = (VENDOR_LIST >> 8) & 0xff;
                cur_len = rsp_len;
                rsp_len += 2;
				memcpy(&Query_Rsp[rsp_len], HS2_HDR, sizeof(HS2_HDR));
                rsp_len += sizeof(HS2_HDR);
                Query_Rsp[rsp_len++] = HS2_CONN_CAP;
				Query_Rsp[rsp_len++] = 0;
				proto = pHS2->hs2conf[fifo_index].proto;
				while (proto != NULL) {
#if 0
					memcpy(&Query_Rsp[rsp_len], &proto->ip_proto, 1);
					rsp_len += 1;
					memcpy(&Query_Rsp[rsp_len], &proto->port, 2);
                    rsp_len += 2;
					memcpy(&Query_Rsp[rsp_len], &proto->status, 1);
                    rsp_len += 1;
					proto = proto->next;
#endif
					Query_Rsp[rsp_len++] = proto->ip_proto;
					Query_Rsp[rsp_len++] = proto->port & 0xff;
					Query_Rsp[rsp_len++] = (proto->port >> 8) & 0xff;
					Query_Rsp[rsp_len++] = proto->status;
					proto = proto->next;
				}	
				//memcpy(&Query_Rsp[rsp_len], hotspot_connection_cap, sizeof(hotspot_connection_cap));
	            //rsp_len += sizeof(hotspot_connection_cap);
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				break;
			case NAI_QUERY:
			{	
				unsigned char *realm_name_len, *realm_name;
				unsigned char *realm_cnt = (Query_ID+1+1); //+1 reserved byte
				unsigned char tmp_name[256];
				int k = 0, j = 0, m = 0, total_cnt = 0;
				
				if (*realm_cnt == 0)	
				{
					//payload=payload-subtype(1)-NAI_Realm_Count(1)-reserved(1)
					len = len-2-1;
					Query_Rsp[rsp_len++] = NAI_LIST & 0xff;
                    Query_Rsp[rsp_len++] = (NAI_LIST >> 8) & 0xff;
					cur_len = rsp_len;
					rsp_len += 2;
					Query_ID+= 2; //subtype(1)+reserved(1)
				}
				else	
				{
					//len = len-4-*realm_name_len;
					len = len-2-1; //hs anqp=6 subtype(1)+realm cnt(1)+reserved(1)
					Query_Rsp[rsp_len++] = NAI_LIST & 0xff;
	                Query_Rsp[rsp_len++] = (NAI_LIST >> 8) & 0xff;
		            cur_len = rsp_len;
			        rsp_len += 4;
					realm_name = realm_cnt+3;
					//Query_ID++;
					Query_ID+= 2; //subtype(1)+reserved(1)					
					while(*realm_cnt != 0)	
					{
						realm_name_len = realm_name-1;						
						Query_ID = Query_ID+2+*realm_name_len;

						len = len-2-*realm_name_len; //encoding,name len, name
						
						for(k=0;k<*realm_name_len;k++)	
						{
							if (realm_name[k] != ';')	
							{
								tmp_name[j++] = realm_name[k];
							}
						
							if ((realm_name[k] == ';')||(j == *realm_name_len))
							{
								tmp_name[j] = '\0';
								realm = pHS2->hs2conf[fifo_index].realm;
								//for (m=0;m<4;m++)
								while (realm != NULL)
								{									
									if (!strcmp(tmp_name, realm->name))
									{
										realm_data_len = rsp_len;
										rsp_len+=2;
										Query_Rsp[rsp_len++] = 0x00;
										Query_Rsp[rsp_len++] = strlen(realm->name);
										memcpy(&Query_Rsp[rsp_len], realm->name, strlen(realm->name));
										rsp_len += strlen(realm->name);
										Query_Rsp[rsp_len++] = realm->eap_method_cnt;
										eap_method = realm->eap_method;
						                while (eap_method != NULL) {
											method_len = rsp_len;
						                    rsp_len++;
											Query_Rsp[rsp_len++] = eap_method->method;
						                    Query_Rsp[rsp_len++] = eap_method->auth_cnt;
						                    for (i=0;i<eap_method->auth_cnt;i++) {
											    Query_Rsp[rsp_len++] = eap_method->auth_id[i];
						                        Query_Rsp[rsp_len++] = 1;
											    Query_Rsp[rsp_len++] = eap_method->auth_val[i][0];
											}
						                    Query_Rsp[method_len] = rsp_len-method_len-1;
											method_len = 0;
						                    eap_method = eap_method->next;
										}
					                    Query_Rsp[realm_data_len] = (rsp_len-realm_data_len-2) & 0xff;
										Query_Rsp[realm_data_len+1] = ((rsp_len-realm_data_len-2) >> 8) & 0xff;
										realm_data_len = 0;
					                    realm = realm->next;
										total_cnt++;										
									}
									else
										realm = realm->next;
								}
								j = 0;
							}
						} 
						(*realm_cnt)--;
						realm_name += *realm_name_len+2;						
					} //end while
				} //end else
				Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
                Query_Rsp[cur_len+2] = total_cnt & 0xff;
                Query_Rsp[cur_len+3] = (total_cnt >> 8) & 0xff;
				break;
			}
			case OP_BAND:
			{
				Query_Rsp[rsp_len++] = VENDOR_LIST & 0xff;
                Query_Rsp[rsp_len++] = (VENDOR_LIST >> 8) & 0xff;
                cur_len = rsp_len;
                rsp_len += 2;
				memcpy(&Query_Rsp[rsp_len], HS2_HDR, sizeof(HS2_HDR));
	            rsp_len += sizeof(HS2_HDR);
                Query_Rsp[rsp_len++] = OP_BAND;
				Query_Rsp[rsp_len++] = 0;
				memcpy(&Query_Rsp[rsp_len], pHS2->hs2conf[fifo_index].opband, pHS2->hs2conf[fifo_index].opband_len);
				rsp_len += pHS2->hs2conf[fifo_index].opband_len;
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				break;
			}									
			default:
				printf("unknown anqp hs id:%d, skip len 1\n", *Query_ID);
				break;
		}
		
		if (len > 0)
		{
			Query_ID++;
        	len -= 1;
        }
        
        if (rsp_len > MAX_GAS_CONTENTS_LEN)
		{
			printf("hs query:rsp len > support max gas len\n");
            return -1;
		}
	}
	return rsp_len;
}	

int handle_query_list(pHS2CTX pHS2, unsigned char *pQuery_ID, int len, unsigned char *Query_Rsp, int fifo_index)
{
    int rsp_len = 0, i = 0, cur_len = 0, plmn_cnt = 0, realm_data_len = 0, realm_cnt = 0, method_len = 0;
    unsigned short qid, vendor_len;
	struct  hs2_plmn	*plmn;	
	struct	hs2_realm	*realm;
	struct hs2_eap_method   *eap_method;

	printf("handle_query_list ==> id=%d,len=%d\n", *pQuery_ID, len);
    while(len > 0)
	{
		qid = (*pQuery_ID)  | (*(pQuery_ID+1) << 8);
		switch(qid)
        {
			//Query Capability List
			case ANQP_CAP:
				HS2_DEBUG_INFO("\t==> anqp cap\n");
				Query_Rsp[rsp_len++] = ANQP_CAP & 0xff;
                Query_Rsp[rsp_len++] = (ANQP_CAP >> 8) & 0xff;
				cur_len = rsp_len;
				rsp_len += 2;
				//standard ANQP
				if (pHS2->hs2conf[fifo_index].venue_cnt != 0)	{
					Query_Rsp[rsp_len] = VENUE_INFO & 0xff;
					Query_Rsp[rsp_len+1] = (VENUE_INFO >> 8) & 0xff;
					rsp_len += 2;
				}
				if (pHS2->hs2conf[fifo_index].netauth_cnt != 0)   {
                    Query_Rsp[rsp_len] = NET_AUTH_INFO & 0xff;
                    Query_Rsp[rsp_len+1] = (NET_AUTH_INFO >> 8) & 0xff;
                    rsp_len += 2;
                }
				if (pHS2->hs2conf[fifo_index].roi_cnt != 0)   {
                    Query_Rsp[rsp_len] = ROAM_LIST & 0xff;
                    Query_Rsp[rsp_len+1] = (ROAM_LIST >> 8) & 0xff;
                    rsp_len += 2;
                }	
				Query_Rsp[rsp_len] = IP_TYPE & 0xff;
                Query_Rsp[rsp_len+1] = (IP_TYPE >> 8) & 0xff;
                rsp_len += 2;
				if (pHS2->hs2conf[fifo_index].realm != 0)   {
                    Query_Rsp[rsp_len] = NAI_LIST & 0xff;
                    Query_Rsp[rsp_len+1] = (NAI_LIST >> 8) & 0xff;
                    rsp_len += 2;
                }
				if (pHS2->hs2conf[fifo_index].plmn != 0)   {
                    Query_Rsp[rsp_len] = CELL_NET & 0xff;
                    Query_Rsp[rsp_len+1] = (CELL_NET >> 8) & 0xff;
                    rsp_len += 2;
                }
				if (pHS2->hs2conf[fifo_index].domain_cnt != 0)   {
                    Query_Rsp[rsp_len] = DOMAIN_LIST & 0xff;
                    Query_Rsp[rsp_len+1] = (DOMAIN_LIST >> 8) & 0xff;
                    rsp_len += 2;
                }			
				Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
				Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
                break;	                			
			case ROAM_LIST:
				HS2_DEBUG_INFO("\t==> roam list\n");
				Query_Rsp[rsp_len++] = ROAM_LIST & 0xff;
                Query_Rsp[rsp_len++] = (ROAM_LIST >> 8) & 0xff;
				cur_len = rsp_len;
				rsp_len += 2;
				for (i=0;i<pHS2->hs2conf[fifo_index].roi_cnt;i++) {
					Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].roi_len[i];
					memcpy(&Query_Rsp[rsp_len], pHS2->hs2conf[fifo_index].roi[i], pHS2->hs2conf[fifo_index].roi_len[i]);
                    rsp_len += pHS2->hs2conf[fifo_index].roi_len[i];
				}
				Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				break;
			case CELL_NET:
				HS2_DEBUG_INFO("\t==> cell net\n");
				Query_Rsp[rsp_len++] = CELL_NET & 0xff;
                Query_Rsp[rsp_len++] = (CELL_NET >> 8) & 0xff;
                cur_len = rsp_len;
				rsp_len += 7;
				plmn = pHS2->hs2conf[fifo_index].plmn;
				while (plmn != NULL) {
					Query_Rsp[rsp_len++] = ((plmn->mcc[1] & 0x0f) << 4) | (plmn->mcc[0] & 0x0f);
					Query_Rsp[rsp_len++] = ((plmn->mnc[2] & 0x0f) << 4) | (plmn->mcc[2] & 0x0f);
					Query_Rsp[rsp_len++] = ((plmn->mnc[1] & 0x0f) << 4) | (plmn->mnc[0] & 0x0f);
					
					plmn_cnt++;
					plmn = plmn->next;
				}

				Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				Query_Rsp[cur_len+2] = 0;
				Query_Rsp[cur_len+3] = (rsp_len-cur_len-4) & 0xff;
				Query_Rsp[cur_len+4] = 0;
				Query_Rsp[cur_len+5] = (rsp_len-cur_len-6) & 0xff;
				Query_Rsp[cur_len+6] = plmn_cnt & 0xff;
				break;
			case DOMAIN_LIST:
				HS2_DEBUG_INFO("\t==> domain list\n");
				Query_Rsp[rsp_len++] = DOMAIN_LIST & 0xff;
                Query_Rsp[rsp_len++] = (DOMAIN_LIST >> 8) & 0xff;
                cur_len = rsp_len;
				rsp_len += 2;
				for (i=0;i<pHS2->hs2conf[fifo_index].domain_cnt;i++) {
					Query_Rsp[rsp_len++] = strlen(pHS2->hs2conf[fifo_index].domain_name[i]);
					memcpy(&Query_Rsp[rsp_len], pHS2->hs2conf[fifo_index].domain_name[i], strlen(pHS2->hs2conf[fifo_index].domain_name[i]));
					rsp_len += strlen(pHS2->hs2conf[fifo_index].domain_name[i]);
				}
				Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
                break;
			case VENUE_INFO:
				HS2_DEBUG_INFO("\t==> venue info\n");
				Query_Rsp[rsp_len++] = VENUE_INFO & 0xff;
                Query_Rsp[rsp_len++] = (VENUE_INFO >> 8) & 0xff;
                cur_len = rsp_len;
				rsp_len += 2;
				Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].venue_group;
				Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].venue_type;
				for (i=0;i<pHS2->hs2conf[fifo_index].venue_cnt;i++) {
					Query_Rsp[rsp_len++] = strlen(&pHS2->hs2conf[fifo_index].venue_name[i][3])+3;
					memcpy(&Query_Rsp[rsp_len], pHS2->hs2conf[fifo_index].venue_name[i], 3);
					rsp_len += 3;
					memcpy(&Query_Rsp[rsp_len], &pHS2->hs2conf[fifo_index].venue_name[i][3], strlen(&pHS2->hs2conf[fifo_index].venue_name[i][3]));
					rsp_len += strlen(&pHS2->hs2conf[fifo_index].venue_name[i][3]);
				}
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				break;
			case NET_AUTH_INFO:
				HS2_DEBUG_INFO("\t==> net auth info\n");
				Query_Rsp[rsp_len++] = NET_AUTH_INFO & 0xff;
                Query_Rsp[rsp_len++] = (NET_AUTH_INFO >> 8) & 0xff;
                cur_len = rsp_len;
				rsp_len += 2;
				for (i=0;i<pHS2->hs2conf[fifo_index].netauth_cnt;i++) {
					Query_Rsp[rsp_len++] = pHS2->hs2conf[fifo_index].netauth_type[i];
					//if (pHS2->hs2conf[fifo_index].netauth_type[i] == 2) {
					if ( pHS2->hs2conf[fifo_index].redirectURL[i] && strlen(pHS2->hs2conf[fifo_index].redirectURL[i]) != 0) {
						Query_Rsp[rsp_len++] = (strlen(pHS2->hs2conf[fifo_index].redirectURL[i])) & 0xff;
		                Query_Rsp[rsp_len++] = (strlen(pHS2->hs2conf[fifo_index].redirectURL[i]) >> 8) & 0xff;

						memcpy(&Query_Rsp[rsp_len], pHS2->hs2conf[fifo_index].redirectURL[i], strlen(pHS2->hs2conf[fifo_index].redirectURL[i]));
						rsp_len += strlen(pHS2->hs2conf[fifo_index].redirectURL[i]);
					}
					else	{
						Query_Rsp[rsp_len++] = 0;
						Query_Rsp[rsp_len++] = 0;
					}
				}
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
                break;
			case IP_TYPE:
				HS2_DEBUG_INFO("\t==> ip type\n");
				Query_Rsp[rsp_len++] = IP_TYPE & 0xff;
                Query_Rsp[rsp_len++] = (IP_TYPE >> 8) & 0xff;
                cur_len = rsp_len;
                rsp_len += 2;
				Query_Rsp[rsp_len++] = ((pHS2->hs2conf[fifo_index].ipv4type & 0x3f) << 2) | pHS2->hs2conf[fifo_index].ipv6type & 0x03;
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
                break;
            case NAI_LIST:
            	HS2_DEBUG_INFO("\t==> nai list\n");
            	Query_Rsp[rsp_len++] = NAI_LIST & 0xff;
                Query_Rsp[rsp_len++] = (NAI_LIST >> 8) & 0xff;
                cur_len = rsp_len;
                rsp_len += 4;
				realm = pHS2->hs2conf[fifo_index].realm;
				while (realm != NULL) {
					realm_data_len = rsp_len;
					rsp_len+=2;
					Query_Rsp[rsp_len++] = 0;
					Query_Rsp[rsp_len++] = strlen(realm->name);
					memcpy(&Query_Rsp[rsp_len], realm->name, strlen(realm->name));
					rsp_len += strlen(realm->name);
					Query_Rsp[rsp_len++] = realm->eap_method_cnt;
					eap_method = realm->eap_method;
					while (eap_method != NULL) {
						method_len = rsp_len;
						rsp_len++;
						Query_Rsp[rsp_len++] = eap_method->method;		
						Query_Rsp[rsp_len++] = eap_method->auth_cnt;
						for (i=0;i<eap_method->auth_cnt;i++) {
							Query_Rsp[rsp_len++] = eap_method->auth_id[i];
							Query_Rsp[rsp_len++] = 1;
							Query_Rsp[rsp_len++] = eap_method->auth_val[i][0];	
						}
						Query_Rsp[method_len] = rsp_len-method_len-1;
						method_len = 0;
						eap_method = eap_method->next;
					}
					Query_Rsp[realm_data_len] = (rsp_len-realm_data_len-2) & 0xff;
					Query_Rsp[realm_data_len+1] = ((rsp_len-realm_data_len-2) >> 8) & 0xff;
					realm_data_len = 0;
					realm_cnt++;
					realm = realm->next;
				}
                //{
	            //    memcpy(&Query_Rsp[rsp_len], nai_realm_list, sizeof(nai_realm_list)); //for eap-ttls
		        //    rsp_len += sizeof(nai_realm_list);
		        //    memcpy(&Query_Rsp[rsp_len], nai_realm_list2, sizeof(nai_realm_list2)); //bcm ap4
                //    rsp_len += sizeof(nai_realm_list2);
				//		memcpy(&Query_Rsp[rsp_len], nai_realm_nolist, sizeof(nai_realm_nolist)); //for eap-sim
                //    rsp_len += sizeof(nai_realm_nolist);
                
				//}
                Query_Rsp[cur_len] = (rsp_len-cur_len-2) & 0xff;
                Query_Rsp[cur_len+1] = ((rsp_len-cur_len-2) >> 8) & 0xff;
				Query_Rsp[cur_len+2] = realm_cnt & 0xff;
				Query_Rsp[cur_len+3] = (realm_cnt >> 8) & 0xff;
            	break;
            case VENDOR_LIST:
            	{
            		HS2_DEBUG_INFO("vendor list\n");
            		pQuery_ID += 2; //dd dd
            		vendor_len = (*pQuery_ID)  | (*(pQuery_ID+1) << 8);

					// HS2_QUERY subtype = length ptr(2)+OI(3)+type(1)	
					if(*(pQuery_ID+6) == HS2_QUERY) {
                        HS2_DEBUG_INFO("\t==> hs2 query\n");
						// payload len = length-OI(3)-type(1)-subtype(1)-reserved(1)
                        vendor_len -= 6; 
                        //tmp = 5 + 1; //+1 reserved
						// query id = length ptr+length(2)+OI(3)+type(1)+subtype(1)+reserved(1)
                        pQuery_ID += 8; //lenl lenh 50 6f 9a type11 queryid ,+1 reserved
						rsp_len += handle_hs_query_list(pHS2, pQuery_ID, vendor_len, &Query_Rsp[rsp_len], fifo_index); 

						// remain len=len-Info(2)-length(2)-OI(3)-type(1)-subtype(1)-reserved(1)-payload
						len = len-4-vendor_len-6;
						// query id pointer to next anqp element
						pQuery_ID += vendor_len;
                    }
                    else if (*(pQuery_ID+6) == NAI_QUERY) {
                        HS2_DEBUG_INFO("\t==> nai query\n");
						// payload len = length-OI(3)-type(1)
						vendor_len -= 4;
						//tmp = 4;
						// query id = length ptr+length(2)+OI(3)+type(1)
						pQuery_ID += 6;
						rsp_len += handle_hs_query_list(pHS2, pQuery_ID, vendor_len, &Query_Rsp[rsp_len], fifo_index); 

						// remain len=len-InfoID(2)-length(2)-OI(3)-type(1)-(payload+reserved+subtype)
						len = len-4-vendor_len-4;
						// query id pointer to next anqp element
						pQuery_ID += vendor_len;
                    }					
					else {
						HS2_DEBUG_INFO("\t==> hs2 unknown query\n");
						//vendor_len -= 5 - 1; //-1 reserved
						//tmp = 5 + 1; //+1 reserved
						//pQuery_ID += vendor_len+5+2 +1; //+1 reserved
						// remain len=len-InfoID(2)-length(2)-(payload+type+subtype+reserved)
						len = len-4-vendor_len;
						pQuery_ID += vendor_len+2;
					}
            	}
            	break;
            default:
            	HS2_DEBUG_INFO("\t ==> unknown anqp id:%d, skip len 2\n", qid);
            	break; 
		}	
        
        if (len > 0)
        {
        	//if (qid == VENDOR_LIST)
        	//{
        	//	len = len-2-2-tmp-vendor_len;
        	//}
        	//else
        	if (qid != VENDOR_LIST)
        	{
        		len -= 2;
        		pQuery_ID += 2;
        	}
        }
        	
        if (rsp_len > MAX_GAS_CONTENTS_LEN)
		{
			HS2_DEBUG_INFO("query:rsp len > support max gas len\n");
            return -1;
		}
    }
    return rsp_len;
}

void hs2_send_gasrsp(pHS2CTX pHS2, unsigned char *da, STA_CTX *psta, DOT11_HS2_GAS_REQ *pgas_req, unsigned short status, unsigned char *ifname, int fifo_index)
{
	DOT11_HS2_GAS_RSP   gas_rsp, *pcur_gas_rsp;
	int tmplen;	
	pcur_gas_rsp = &gas_rsp;
	
	//fill gas rsp contents
	if (psta != NULL)
	{
		if (psta->gas_rsp.Rsplen > pHS2->hs2conf[fifo_index].mmpdu_limit)
		{
			if (pgas_req->EventId == DOT11_EVENT_GAS_INIT_REQ)
			{
				pcur_gas_rsp->Action = _GAS_INIT_RSP_ACTION_ID_;
				pcur_gas_rsp->Comeback_delay = pHS2->hs2conf[fifo_index].comeback_delay;
				pcur_gas_rsp->Rsplen = 0;
				psta->need_free = 0;
				psta->rsp_index = 0; // add!!
				psta->gas_rsp.Rsp_fragment_id = 0;// add!!
				pcur_gas_rsp->Rsp_fragment_id = psta->gas_rsp.Rsp_fragment_id;
			}
			else if(pgas_req->EventId == DOT11_EVENT_GAS_COMEBACK_REQ)
			{
				// for pass HS2 certification
				
				pcur_gas_rsp->Action = _GAS_COMBACK_RSP_ACTION_ID_;
				if(psta->gas_rsp.Rsplen > 600)
					pcur_gas_rsp->Rsplen = 600; //tmplen; //pHS2->mmpdu_limit;
				else
					pcur_gas_rsp->Rsplen = psta->gas_rsp.Rsplen;
				
				memcpy(pcur_gas_rsp->Rsp, &psta->gas_rsp.Rsp[psta->rsp_index], pcur_gas_rsp->Rsplen);
				
				psta->rsp_index += pcur_gas_rsp->Rsplen;
				if(psta->gas_rsp.Rsplen > 600)
					psta->gas_rsp.Rsplen -= 600; //tmplen ;//pHS2->mmpdu_limit;
				else
					psta->gas_rsp.Rsplen = 0;
				
				HS2_DEBUG_INFO("comback rsp remain len=%d\n", psta->gas_rsp.Rsplen);
				if (psta->gas_rsp.Rsplen == 0)
				{
					pcur_gas_rsp->Rsp_fragment_id = 0x00 | psta->gas_rsp.Rsp_fragment_id;
                    psta->gas_rsp.Rsp_fragment_id = 0;
                    pcur_gas_rsp->Comeback_delay = 0; //pHS2->comeback_delay;
                    psta->need_free = 1; 
				}
				else
				{
					pcur_gas_rsp->Rsp_fragment_id = 0x80 | psta->gas_rsp.Rsp_fragment_id;
					psta->gas_rsp.Rsp_fragment_id++;
					pcur_gas_rsp->Comeback_delay = 0; //pHS2->comeback_delay;
					psta->need_free = 0;		
				}
			}
			else
			{
				HS2_DEBUG_INFO("ERR: unknow eventId, should not here!!!\n");
			}
		}
		else
		{
			//printf("rsp=%d,%d \n", psta->rsp_index,psta->gas_rsp.Rsplen);
			if (pgas_req->EventId == DOT11_EVENT_GAS_INIT_REQ)
			{
				pcur_gas_rsp->Action = _GAS_INIT_RSP_ACTION_ID_;
			}
			else
			{
				pcur_gas_rsp->Action = _GAS_COMBACK_RSP_ACTION_ID_;
			}
    		pcur_gas_rsp->Rsplen = psta->gas_rsp.Rsplen;
    		memcpy(pcur_gas_rsp->Rsp, &psta->gas_rsp.Rsp[psta->rsp_index], pcur_gas_rsp->Rsplen);
    		psta->need_free = 1;
    		pcur_gas_rsp->Comeback_delay = 0;
    		//printf("frg id=%d\n", psta->gas_rsp.Rsp_fragment_id);
    		pcur_gas_rsp->Rsp_fragment_id = psta->gas_rsp.Rsp_fragment_id;
    	}
    }
    else
    {
		HS2_DEBUG_INFO("psta is NULL\n");
		HS2_DEBUG_INFO("pgas_req->EventId=%x\n",pgas_req->EventId);
		if (pgas_req->EventId == DOT11_EVENT_GAS_INIT_REQ)
        {
	        pcur_gas_rsp->Action = _GAS_INIT_RSP_ACTION_ID_;
        }
        else
        {
			pcur_gas_rsp->Action = _GAS_COMBACK_RSP_ACTION_ID_;
		}
    	pcur_gas_rsp->Rsplen = 0;
    	pcur_gas_rsp->Comeback_delay = 0;
    	pcur_gas_rsp->Rsp_fragment_id = 0;
	}
    pcur_gas_rsp->StatusCode = status;
	memcpy(pcur_gas_rsp->MACAddr, da, MACADDRLEN);
    pcur_gas_rsp->Dialog_token = pgas_req->Dialog_token;
    pcur_gas_rsp->Advt_proto = pgas_req->Advt_proto;
               
    //printf("ioctl len=%d\n", sizeof(DOT11_HS2_GAS_RSP)-MAX_GAS_CONTENTS_LEN+pcur_gas_rsp->Rsplen);
	wlioctl_send_hs2_rsp(ifname, pcur_gas_rsp, DOT11_EVENT_HS2_GAS_RSP);
}

static void process_GAS_INIT_REQ_event(pHS2CTX pHS2, unsigned char *ifname, int fifo_index)
{
	ANQP_FORMAT         *anqp_msg;
	DOT11_HS2_GAS_REQ   *pgas_req;
	STA_CTX             *psta;
	unsigned short 		infoID;
	int					reqLen;
	
	//pcur_gas_rsp = &gas_rsp;
	pgas_req = (DOT11_HS2_GAS_REQ *)(pHS2->RecvBuf + FIFO_HEADER_LEN);
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n" ,pgas_req->MACAddr[0],pgas_req->MACAddr[1],pgas_req->MACAddr[2],pgas_req->MACAddr[3],pgas_req->MACAddr[4],pgas_req->MACAddr[5]);
	
	if (pgas_req->Advt_proto == ADVTIE[1]) {  // ANQP elements
	       anqp_msg = (ANQP_FORMAT *)pgas_req->Req;
           infoID = (anqp_msg->InfoIDH << 8) | anqp_msg->InfoIDL;
			//reqLen: gas_init_req_len-info_id(2)-length(2)
           reqLen = pgas_req->Reqlen-4;
                       
           switch(infoID)
           {
	           case ANQP_QUERY:
			case VENDOR_LIST:
                   //printf("got query list.req=%d,id=%d\n", reqLen, infoID);
                   psta = get_comeback_sta(pHS2, pgas_req->Dialog_token, pgas_req->MACAddr, fifo_index);
				if (psta == NULL)
	  	                psta = get_avaiable_sta(pHS2, fifo_index);
				if (psta != NULL)
				{
					unsigned char *qid;								
						
					memcpy(psta->gas_rsp.MACAddr, pgas_req->MACAddr, MACADDRLEN);
					psta->gas_rsp.Dialog_token = pgas_req->Dialog_token;
									
					if (infoID == ANQP_QUERY)
					{
						HS2_DEBUG_INFO("req len=%d\n", reqLen);
						psta->gas_rsp.Rsplen = handle_query_list(pHS2, anqp_msg->contents, reqLen, psta->gas_rsp.Rsp, fifo_index);
						HS2_DEBUG_INFO("rsp len=%d\n", psta->gas_rsp.Rsplen);
					}									
					else  // VENDOR_LIST
					{
						int vendor_len;
						unsigned char *ptr = &anqp_msg->LengthL;
							while (reqLen > 0)	{
							vendor_len = *ptr | (*(ptr+1) << 8);
							HS2_DEBUG_INFO("hs vendor\n");
								// HS2_QUERY subtype = length ptr(2)+OI(3)+type(1)
							if(*(ptr+6) == HS2_QUERY)	{
								HS2_DEBUG_INFO("hs2 query\n");
									// query id = length ptr(2)+OI(3)+type(1)+subtype(1)+reserved(1)
								qid = ptr+7+1;
								// payload len = length-OI(3)-type(1)-subtype(1)-reserved(1)
								vendor_len -= 6; 
	                            psta->gas_rsp.Rsplen += handle_hs_query_list(pHS2, qid, vendor_len, &psta->gas_rsp.Rsp[psta->gas_rsp.Rsplen], fifo_index);
								// gas_init_req_len = gas_init_req_len-payload-OI(3)-type(1)-subtype(1)-reserved(1)
								reqLen = reqLen-5-vendor_len-1;
								// ptr = qid+payload = next anqp element
								ptr = qid+vendor_len;
							}
							else if (*(ptr+6) == NAI_QUERY)	{
								HS2_DEBUG_INFO("nai query or icon request\n");
								
								// nai query id = length ptr(2)+OI(3)+type(1)
								qid = ptr+6; 
								// payload len = length-OI(3)-type(1)
								vendor_len -= 4;
	                            psta->gas_rsp.Rsplen += handle_hs_query_list(pHS2, qid, vendor_len, &psta->gas_rsp.Rsp[psta->gas_rsp.Rsplen], fifo_index);
								// gas_init_req_len = gas_init_req_len-payload-OI(3)-type(1)
								reqLen = reqLen-4-vendor_len; 
								ptr = qid+vendor_len;
							}								
							else {
								HS2_DEBUG_ERR("hs2 unknown query\n");
									// gas_init_req_len = gas_init_req_len-this unknown anqp length
								reqLen = reqLen-vendor_len;
								// ptr = ptr+this unknown anqp length+anqp length(2)
								ptr += vendor_len+2;
							}
								// if remain gas_init_req_len
							if (reqLen > 0)	{
									// ptr pointer next anqp length
								ptr = ptr+2;
								reqLen -= 4;
							}
						}
					}
									
					if (psta->gas_rsp.Rsplen < 0)
					{
						HS2_DEBUG_ERR("rsp len < 0 !!\n");
						psta->gas_rsp.Rsplen = 0;
						hs2_send_gasrsp(pHS2, pgas_req->MACAddr, psta, pgas_req, GAS_RSP_LARGER_REQ_LIMIT, ifname, fifo_index);
					}
					else
						hs2_send_gasrsp(pHS2, pgas_req->MACAddr, psta, pgas_req, SUCCESS, ifname, fifo_index);
									
                  	    if (psta)	                                	
		            {
		               	if (psta->need_free == 1)
			               	reset_sta(pHS2, psta, fifo_index);
			            else
			               	psta->expire_time = 60;//180; //1 mins timeout	
				    }
				}
				else
				{
					HS2_DEBUG_ERR("DOT11_EVENT_GAS_INIT_REQ:no sta space!!\n");
					hs2_send_gasrsp(pHS2, pgas_req->MACAddr, psta, pgas_req, TX_FAIL, ifname, fifo_index);
				}
				break;
		}
	}else { // not ANQP element
		HS2_DEBUG_ERR("advertisement protocol not match!!\n");
		//pcur_gas_rsp->StatusCode = GAS_ADVT_PROTO_NOT_SUPPORT;
		hs2_send_gasrsp(pHS2, pgas_req->MACAddr, NULL, pgas_req, GAS_ADVT_PROTO_NOT_SUPPORT, ifname, fifo_index);
	} 
}

static void process_hs2_event(pHS2CTX pHS2, unsigned char *ifname, int fifo_index)
{
	ANQP_FORMAT         *anqp_msg;
	DOT11_HS2_GAS_REQ   *pgas_req;
	STA_CTX             *psta;
	unsigned short 		infoID;
	int					reqLen;
	unsigned char		EventID;
	
	EventID = *(unsigned char *)(pHS2->RecvBuf + FIFO_HEADER_LEN);
	
    HS2_DEBUG_INFO("HS2 RCV Fifo msg type: %d\n", EventID);
    //printf("%02x:%02x:%02x:%02x:%02x:%02x\n" ,pgas_req->MACAddr[0],pgas_req->MACAddr[1],pgas_req->MACAddr[2],pgas_req->MACAddr[3],pgas_req->MACAddr[4],pgas_req->MACAddr[5]);

	if (pHS2->hs2conf[fifo_index].anqp_enable == 0) {
		pgas_req = (DOT11_HS2_GAS_REQ *)(pHS2->RecvBuf + FIFO_HEADER_LEN);	
		HS2_DEBUG_INFO("send not reachable,pgas_req=%x\n",pgas_req);
		hs2_send_gasrsp(pHS2, pgas_req->MACAddr, NULL, pgas_req, ADVTSERVER_NOT_REACHABLE, ifname, fifo_index);
	} else if (EventID == DOT11_EVENT_GAS_INIT_REQ) 
	{
		process_GAS_INIT_REQ_event(pHS2,ifname,fifo_index);
	}
	else if (EventID == DOT11_EVENT_GAS_COMEBACK_REQ)
	{
		pgas_req = (DOT11_HS2_GAS_REQ *)(pHS2->RecvBuf + FIFO_HEADER_LEN);
		HS2_DEBUG_INFO("%02x:%02x:%02x:%02x:%02x:%02x\n" ,pgas_req->MACAddr[0],pgas_req->MACAddr[1],pgas_req->MACAddr[2],pgas_req->MACAddr[3],pgas_req->MACAddr[4],pgas_req->MACAddr[5]);
		
		psta = get_comeback_sta(pHS2, pgas_req->Dialog_token, pgas_req->MACAddr, fifo_index);
		if (psta != NULL)
		{
			hs2_send_gasrsp(pHS2, pgas_req->MACAddr, psta, pgas_req, SUCCESS, ifname, fifo_index);
			if (psta)	                                	
	        {
		       	if (psta->need_free == 1)
	               	reset_sta(pHS2, psta, fifo_index);
	            else
	               	psta->expire_time = 60; //1 mins timeout	
		    }
		}
		else
		{
			HS2_DEBUG_INFO("got gas comeback req, no gas init req:no sta!!\n");
			hs2_send_gasrsp(pHS2, pgas_req->MACAddr, psta, pgas_req, NO_OUTSTANDING_GAS_REQ, ifname, fifo_index);
		}
	}
	else
	{
		HS2_DEBUG_INFO("unknown eventId: %d\n", pgas_req->EventId);
	}
}
#ifdef __ECOS
static int generate_hs2_config_file(pHS2CTX pHS2)
{
	int index,len;
	unsigned char *buf;
	FILE *fp;
	for (index=0;index<pHS2->cfg_num;index++)
	{
		if(strcmp(pHS2->cfg_filename[index],"/etc/hs2_wlan0.conf") == 0)
			buf = hs2_wlan0_conf;
		else if(strcmp(pHS2->cfg_filename[index],"/etc/hs2_wlan1.conf") == 0)
			buf = hs2_wlan1_conf;
		else 
			return -1;
		len = strlen(buf);
		fp = fopen(pHS2->cfg_filename[index], "w");
		if (fp == NULL) {
			HS2_DEBUG_ERR("generate hs2 config file [%s] failed!\n", pHS2->cfg_filename[index]);
			return -1;
		}
		fputs(buf,fp);
		fclose(fp);
	}
	return 0;
}

void hs2_recv()
{
	pHS2CTX pHS2;
	msg_hdr_t *msg_hdr = NULL;
	char msg_type,i;
	pHS2 = pGlobalHS2;

	while(1){
		msg_hdr = (msg_hdr_t *)cyg_mbox_timed_get(hs2_mbox_hdl,cyg_current_time()+100);
		if (!msg_hdr){
			// timeout
			timeout_handler(pHS2);
		}else{
			for (i=0;i<pHS2->cfg_num;i++) {
				if(!strcmp(pHS2 ->hs2conf->wlan_iface,msg_hdr->msg_buf) ){
					memcpy(pHS2->RecvBuf,msg_hdr->msg_buf+WLAN_INF_LEN ,msg_hdr->msg_len-WLAN_INF_LEN);
					if (pHS2->hs2conf[i].iw_enable == 1) {	
						diag_printf("Got HS2 Event\n");
						process_hs2_event(pHS2, pHS2->hs2conf[i].wlan_iface, i);
					}
					
				}
			}
			free(msg_hdr);
		}
	}

}
void creat_hs2()
{
	
	
	/*Creat messag box*/
	cyg_mbox_create(&hs2_mbox_hdl,&hs2_mbox);
	/* Create the thread */
	cyg_thread_create(CYGNUM_HS2_THREAD_PRIORITY,
				  hs2_recv,
				  0,
				  "hs2",
				  &hs2_stack,
				  sizeof(hs2_stack),
				  &hs2_thread_handle,
				  &hs2_thread_obj );
		/* Let the thread run when the scheduler starts */
	cyg_thread_resume(hs2_thread_handle);
}
#endif

#ifndef __ECOS
static void hs2_daemon(pHS2CTX pHS2)
{
	//STA_CTX             *psta;
	fd_set 				netFD;
    int 				selret, nRead, max_sock, i;
    struct timeval 		timeout;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	// while loop to listen socket and check event
    while (1) {
	    max_sock = 0;
	    FD_ZERO(&netFD);
	
		for (i=0;i<pHS2->cfg_num;i++) {
			FD_SET(pHS2->hs2conf[i].readfifo, &netFD);
	        max_sock = (max_sock > pHS2->hs2conf[i].readfifo)? max_sock : pHS2->hs2conf[i].readfifo;
		}
		selret = select(max_sock+1, &netFD, NULL, NULL, &timeout);
        if (selret >= 0) {
			if (selret == 0) {
				// timeout
				timeout_handler(pHS2);
				// reset timer
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
				continue;
			}
			for (i=0;i<pHS2->cfg_num;i++) {
		        if (FD_ISSET(pHS2->hs2conf[i].readfifo, &netFD)) {					
			        nRead = read(pHS2->hs2conf[i].readfifo, pHS2->RecvBuf, MAX_MSG_SIZE);					
					//printf("nRead=%d\n",nRead);
				    if (nRead > 0) {
						if (pHS2->hs2conf[i].iw_enable == 1) {	
							printf("Got Event\n");
							process_hs2_event(pHS2, pHS2->hs2conf[i].wlan_iface, i);
						}
					}
				}
			}
		}
	}                         
}
#endif
void dump_config(struct hs2_config *pconfig)
{
	int i, j;
	struct hs2_realm		*prealm;
	struct hs2_eap_method   *pmethod;
	struct proto_port		*proto;
	struct hs2_plmn			*plmn;
	
	printf("========dump hs2 config========\n");
	printf("wlan iface		=%s\n", pconfig->wlan_iface);
	printf("fifo name		=%s\n", pconfig->fifo_name);
	printf("mmpdu			=%d\n", pconfig->mmpdu_limit);
	printf("comback dealy		=%d\n", pconfig->comeback_delay);
	printf("ant			=%d\n", pconfig->ant);
	printf("int			=%d\n", pconfig->internet);
	printf("venue group		=%d\n", pconfig->venue_group);
	printf("venue type		=%d\n", pconfig->venue_type);
	printf("advt id			=%d\n", pconfig->advtid);
	for (i=0;i<pconfig->venue_cnt;i++)
		printf("venuename%d		=%s, len=%d\n", i+1, pconfig->venue_name[i], strlen(pconfig->venue_name[i]));
	for (i=0;i<pconfig->op_cnt;i++)
        printf("op friendly name%d		=%s, len=%d\n", i+1, pconfig->op_name[i], strlen(pconfig->op_name[i]));
	for (i=0;i<pconfig->domain_cnt;i++)
        printf("domain%d			=%s\n", i+1, pconfig->domain_name[i]);
	for (i=0;i<pconfig->netauth_cnt;i++)
        printf("netauth%d type:%d, URL:%s\n", i+1, pconfig->netauth_type[i], pconfig->redirectURL[i]);
	printf("ipv4 type		=%d\n", pconfig->ipv4type);
	printf("ipv6 type		=%d\n", pconfig->ipv6type);
	printf("HESSID			=%02x%02x%02x%02x%02x%02x\n", pconfig->hessid[0], pconfig->hessid[1], pconfig->hessid[2], pconfig->hessid[3], pconfig->hessid[4], pconfig->hessid[5]);

	for (i=0;i<pconfig->roi_cnt;i++)
	{
		printf("roi%d			=",i+1);
		for (j=0;j<pconfig->roi_len[i];j++) {
			printf("%02x", pconfig->roi[i][j]);
		}
		printf("\n");
	}

	prealm = pconfig->realm;
	while (prealm != NULL) {
		printf("realm name		=%s\n", prealm->name);
		printf("eap method cnt		=%d\n", prealm->eap_method_cnt);

		pmethod = prealm->eap_method;		
		for (i=0;i<prealm->eap_method_cnt;i++) {
			printf("eap method		=%d\n", pmethod->method);
			for (j=0;j<pmethod->auth_cnt;j++) {
				printf("auth id			=%d\n", pmethod->auth_id[j]);
				printf("auth value		=%d\n", pmethod->auth_val[j][0]);
			}			
			pmethod = pmethod->next;
		}
		prealm = prealm->next;
	}

	proto = pconfig->proto;
	while (proto != NULL) {
		printf("ip proto		=%d\n", proto->ip_proto);
		printf("port			=%d\n", proto->port);
		printf("status			=%d\n", proto->status);
		proto = proto->next;
	}

	printf("wan info		=%d\n", pconfig->wan_ie.waninfo);
	printf("wan dlspeed		=%d\n", pconfig->wan_ie.dlspeed);
	printf("wan ulspeed		=%d\n", pconfig->wan_ie.ulspeed);
	printf("wan dlload		=%d\n", pconfig->wan_ie.dlload);
	printf("wan ulload		=%d\n", pconfig->wan_ie.ulload);
	printf("wan lmd			=%d\n", pconfig->wan_ie.lmd);

	plmn = pconfig->plmn;
    while (plmn != NULL) {
        printf("mcc			=%d.%d.%d\n", plmn->mcc[0], plmn->mcc[1], plmn->mcc[2]);
        printf("mnc			=%d.%d.%d\n", plmn->mnc[0], plmn->mnc[1], plmn->mnc[2]);
        plmn = plmn->next;
    }

	printf("========dump end========\n");
	sleep(3);
}

static int parse_WAN(struct wan_metric *wanie, FILE*fp)
{
	char line[200], token[40], value[150], *ptr;
	
	while (fgets(line, 200, fp)) {
        if (!strncmp(line, "}", 1))
            break;
        else
        {
            ptr = get_token(line, token);
            if (ptr == NULL)
                continue;

            if (get_value(ptr, value)==0){
                continue;
            }
            else if (!strcmp(token, "link_status")) {
                wanie->waninfo |= atoi(value) & 0x03;
            }
            else if (!strcmp(token, "at_capacity")) {
                wanie->waninfo |= atoi(value) & 0x08;
            }
            else if (!strcmp(token, "dl_speed")) {
                wanie->dlspeed = atoi(value);
            }
			else if (!strcmp(token, "ul_speed")) {
                wanie->ulspeed = atoi(value);
            }
			else if (!strcmp(token, "dl_load")) {
                wanie->dlload = atoi(value);
            }
			else if (!strcmp(token, "ul_load")) {
                wanie->ulload = atoi(value);
            }
			else if (!strcmp(token, "lmd")) {
                wanie->lmd = atoi(value);
            }
        }
    }
	return 0;	
}

static int parse_PLMN(struct hs2_plmn **plmn_list, FILE *fp)
{
    struct hs2_plmn *plist, *pnewlist;
    char line[200], token[40], value[150], *ptr;

	plist = *plmn_list;
    if (plist == NULL) {
        plist = (struct hs2_plmn *) calloc(1, sizeof(struct hs2_plmn));
        if (plist == NULL) {
            printf("allocate plmn failed!!!\n");
            return -1;
        }
        *plmn_list = plist;        
    }
    else {
        pnewlist = plist->next;
        while (1) {
            if (pnewlist == NULL)
                break;
            else {
                plist = pnewlist;
                pnewlist = plist->next;
            }
        }
        pnewlist = (struct hs2_plmn *) calloc(1, sizeof(struct hs2_plmn));
        if (pnewlist == NULL) {
            printf("allocate plmn failed!!!\n");
            return -1;
        }
        plist->next = pnewlist;
        plist = pnewlist;
    }

	memset(plist, 0, sizeof(struct hs2_plmn));
    while (fgets(line, 200, fp)) {
        if (!strncmp(line, "}", 1))
            break;
        else
        {
            ptr = get_token(line, token);
            if (ptr == NULL)
                continue;

            if (get_value(ptr, value)==0){
                continue;
            }
            else if (!strcmp(token, "MCC")) {
                plist->mcc[0] = value[0]-0x30;
				plist->mcc[1] = value[1]-0x30;	
				plist->mcc[2] = value[2]-0x30;
            }
			else if (!strcmp(token, "MNC")) {
                plist->mnc[0] = value[0]-0x30;
                plist->mnc[1] = value[1]-0x30;
				if (strlen(value) == 2)
					plist->mnc[2] = 0xff;
				else
					plist->mnc[2] = value[2]-0x30;
            }
		}
	}
	return 0;
}


static int parse_PROTO(struct hs2_realm **proto_list, FILE *fp, int protocol, int port, int status)
{
	struct proto_port *plist, *pnewlist;
	char line[200], token[40], value[150], *ptr;
	
	plist = *proto_list;
	if (plist == NULL) {
		plist = (struct proto_port *) calloc(1, sizeof(struct proto_port));
		if (plist == NULL) {
			printf("allocate proto failed!!!\n");
			return -1;
		}
		*proto_list = plist;
	}
	else {
		pnewlist = plist->next;
		while (1) {
			if (pnewlist == NULL)
				break;
			else {
				plist = pnewlist;
				pnewlist = plist->next;
			}
		}
		pnewlist = (struct proto_port *) calloc(1, sizeof(struct proto_port));
		if (pnewlist == NULL) {
			printf("allocate proto failed!!!\n");
			return -1;
		}
		plist->next = pnewlist;
        plist = pnewlist;        
    }

    memset(plist, 0, sizeof(struct proto_port));
	if (fp != NULL) {
	    while (fgets(line, 200, fp)) {
			if (!strncmp(line, "}", 1))
			    break;
	        else
		    {
			    ptr = get_token(line, token);
				if (ptr == NULL)
	                continue;

		        if (get_value(ptr, value)==0){
			        continue;
				}
	            else if (!strcmp(token, "ip_protocol")) {
		            plist->ip_proto = atoi(value);
			    }
				else if (!strcmp(token, "port")) {
	                plist->port = atoi(value);
		        }
				else if (!strcmp(token, "status")) {
				    plist->status = atoi(value);
	            }
			}
		}
	}
	else {
		plist->ip_proto = protocol;
		plist->port = port;
		plist->status = status;
	}
	return 0;
}

static int parse_NAI(struct hs2_realm **porinrealm, FILE *fp)
{
	struct hs2_realm *prealm, *pnewrealm;
    struct hs2_eap_method *pmethod, *pnewmethod;
	char line[200], token[40], value[150], *pch, *ptr;

	prealm = *porinrealm;
	if (prealm == NULL) {
	    prealm = (struct hs2_realm *) calloc(1, sizeof(struct hs2_realm));
        if (prealm == NULL) {
			HS2_DEBUG_ERR("allocate realm failed!!!\n");
            return -1;
        }
        *porinrealm = prealm;        
    }
    else
    {
		pnewrealm = prealm->next;
        while (1) {
			if (pnewrealm == NULL)
				break;
            else {
				prealm = pnewrealm;
                pnewrealm = prealm->next;
            }
        }
        pnewrealm = (struct hs2_realm *) calloc(1, sizeof(struct hs2_realm));
        if (pnewrealm == NULL) {
			HS2_DEBUG_ERR("allocate realm failed!!!\n");
            return -1;
        }
        prealm->next = pnewrealm;
        prealm = pnewrealm;
    }

	memset(prealm, 0, sizeof(struct hs2_realm));
    while (fgets(line, 200, fp))
    {
	    if (!strncmp(line, "}", 1))
		    break;
        else
        {
			ptr = get_token(line, token);
            if (ptr == NULL)
				continue;

            if (get_value(ptr, value)==0){
				continue;
            }
            else if (!strncmp(token, "NAIRealm", 8)) {
				strcpy(prealm->name, value);
            }
            else if (!strncmp(token, "EAPMethod", 9)) {
				if ((pmethod = prealm->eap_method) == NULL)
                {
					pmethod = (struct hs2_eap_method *) calloc(1, sizeof(struct hs2_eap_method));
                    if (pmethod == NULL) {
						printf("allocate eap method failed 1!!!\n");
                    }
                    prealm->eap_method = pmethod;
                }
                else
                {
					pnewmethod = pmethod->next;
                    while(1)
                    {
						if (pnewmethod == NULL)
							break;
						else
                        {
                            pmethod = pnewmethod;
                            pnewmethod = pmethod->next;
                        }
                    }

                    pnewmethod = (struct hs2_eap_method *) calloc(1, sizeof(struct hs2_eap_method));
                    if (pnewmethod == NULL) {
						printf("allocate eap method failed 2!!!\n");
                    }
                    pmethod->next = pnewmethod;
                    pmethod = pnewmethod;
                }
                memset(pmethod, 0, sizeof(struct hs2_eap_method));
                if (!strcmp(value, "EAP-TLS"))
					pmethod->method = 13;
                else if (!strcmp(value, "EAP-TTLS"))
					pmethod->method = 21;
                else if (!strcmp(value, "PEAP"))
					pmethod->method = 25;
                else if (!strcmp(value, "EAP-SIM"))
					pmethod->method = 18;
                else if (!strcmp(value, "EAP-AKA"))
					pmethod->method = 23;
                else if (!strcmp(value, "EAP-MSCHAP-V2"))
                    pmethod->method = 29;
                else
					printf("unknown EAP method, %s\n", value);
                
                prealm->eap_method_cnt++;
            }
			else if (!strncmp(token, "AuthParam", 9)) {
	            pch = strtok(value, ";");
                if (*pch != NULL)
                {
		            pmethod->auth_id[pmethod->auth_cnt] = convert_atob(pch, 2);
                    pch = strtok(NULL, ";");
                    if (*pch != NULL)
			            pmethod->auth_val[pmethod->auth_cnt][0] = convert_atob(pch, 2);
                }
                pmethod->auth_cnt++;
            }
		}
	}
	return 0;
}



static int parse_configfile(pHS2CTX pHS2)
{
	int index=0,i, vallen=0;
	FILE *fp;
	struct hs2_config *config;
	char line[200], linebak[200], token[40], value[200], *ptr, *pch;
	for (index=0;index<pHS2->cfg_num;index++)
	{

		fp = fopen(pHS2->cfg_filename[index], "r");
		if (fp == NULL) {
			HS2_DEBUG_ERR("read config file [%s] failed!\n", pHS2->cfg_filename[index]);
			return -1;
		}

		config = &pHS2->hs2conf[index];
		while ( fgets(line, 200, fp) )
		{
			if (line[0] == '#')
		        continue;
			memcpy(linebak, line, 200);
			ptr = get_token(line, token);
	        if (ptr == NULL)
		        continue;
			if ((get_value(ptr, value))==0){
				continue;
	        }
			else if (!strcmp(token, "interworking")) {
                config->iw_enable = atoi(value);
            }
			else if (!strcmp(token, "l2_traffic_inspect")) {
                config->l2_inspect = atoi(value);
            }
			else if (!strncmp(token, "NAIRealmData", 12)) {
				parse_NAI(&config->realm, fp);
            }
			else if (!strncmp(token, "proto_port", 10)) {
				if ((config->sigma_test == 1) && (!strcmp(value, "id1"))) {
					parse_PROTO(&config->proto, NULL, 1, 0, 0);
					parse_PROTO(&config->proto, NULL, 6, 20, 1);
					parse_PROTO(&config->proto, NULL, 6, 22, 0);
					parse_PROTO(&config->proto, NULL, 6, 80, 1);
					parse_PROTO(&config->proto, NULL, 6, 443, 1);
					parse_PROTO(&config->proto, NULL, 6, 1723, 0);
					parse_PROTO(&config->proto, NULL, 6, 5060, 0);
					parse_PROTO(&config->proto, NULL, 17, 500, 1);
					parse_PROTO(&config->proto, NULL, 17, 5060, 0);
					parse_PROTO(&config->proto, NULL, 17, 4500, 1);
					parse_PROTO(&config->proto, NULL, 50, 0, 1);
				}
				else {
					parse_PROTO(&config->proto, fp, 0, 0, 0);
				}
			}
			else if (!strcmp(token, "ICMPv4ECHO")) {
				config->ICMPv4ECHO = atoi(value);
			}
			else if (!strcmp(token, "wan_metrics")) {
                parse_WAN(&config->wan_ie, fp);
			} 
			else if (!strncmp(token, "PLMN", 4)) {
                parse_PLMN(&config->plmn, fp);
            }
			else if (!strncmp(token, "sigma_test", 10)) {
                config->sigma_test = atoi(value);
            }
			else if (!strcmp(token, "mmpdu_size")) {
                config->mmpdu_limit = atoi(value);
            }
			else if (!strcmp(token, "anqp_enable")) {
				config->anqp_enable = atoi(value);
			}
			else if (!strcmp(token, "operation_band")) {
				ptr = &value[0];
                for (i=0;i<strlen(value)/2;i++, ptr+=2)
                {
                    config->opband[i] = convert_atob(ptr, 16);
					printf("operation_band=%d\n",config->opband[i]);
                }					
				config->opband_len = (strlen(value))/2;
			}
			else if (!strcmp(token, "proxy_arp")) {
				printf("proxy_arp=%s\n",value);
				config->proxy_arp = atoi(value);
			}

			else if (!strcmp(token, "dgaf_disabled")) {
				unsigned char tmpbuf[100];
#ifdef __ECOS
				FILE *file;
				sprintf(tmpbuf, "/tmp/dgaf_%s", config->wlan_iface);
				file = fopen(tmpbuf,"w");
				if (file == NULL) {
					HS2_DEBUG_ERR("write dgaf_disabled config file failed!\n");
					fclose(fp);
					return -1;
				}
#endif

				config->dgaf_disable = atoi(value);
				
				if (config->dgaf_disable == 1)	{
#ifdef __ECOS
					fputc('1',file);
#else
					sprintf(tmpbuf, "echo 1 >/tmp/dgaf_%s", config->wlan_iface);
					system(tmpbuf);
#endif
				}
				else	{
#ifdef __ECOS
					fputc('0',file);
#else
					sprintf(tmpbuf, "echo 0 >/tmp/dgaf_%s", config->wlan_iface);
                    system(tmpbuf);
#endif
				}
#ifdef __ECOS
				fclose(file);
#endif
			}
			else if (!strcmp(token, "comeback_delay")) {
                config->comeback_delay = atoi(value);
            }
			else if (!strcmp(token, "interface")) {
				strcpy(config->wlan_iface, value);
				if (strncmp(config->wlan_iface, "wlan", 4)) {
					HS2_DEBUG_ERR("interface name error\n");
					return -1;
				}
	        } else if (!strcmp(token, "access_network_type")) {
				config->ant = atoi(value);
			}
			else if (!strcmp(token, "internet")) {
                config->internet = atoi(value);
            }			
            else if (!strcmp(token, "venuegroup")) {
                config->venue_group = atoi(value);
            }
			else if (!strcmp(token, "venuetype")) {
                config->venue_type = atoi(value);
            }
            else if (!strncmp(token, "venuename", 9)) {
				int pstr = 0;
				int cnt = 0;
				unsigned char lang[3];
				unsigned char tmpbuf[500];
				if ((config->sigma_test == 1) && (!strcmp(value, "id1"))) {
					if(config->venue_cnt >= MAX_VENUE_NUM) {
						printf("num of venue name > %d (MAX_VENUE_NUM)\n",MAX_VENUE_NUM);
						return -1;
					}
					config->venue_name[config->venue_cnt] = (unsigned char *) calloc(strlen(venuename_id1[0])+1, sizeof(unsigned char));
					strcpy(config->venue_name[config->venue_cnt], venuename_id1[0]);
					config->venue_cnt++;
					config->venue_name[config->venue_cnt] = (unsigned char *) calloc(strlen(venuename_id1[1])+1, sizeof(unsigned char));
					strcpy(config->venue_name[config->venue_cnt], venuename_id1[1]);
					config->venue_cnt++;
				}
				else {
					if (linebak[strlen(value)+strlen("venuename")+1] == 0x0a) {
						cnt = strlen(value);
			            value[cnt] = 0x0a;
				        value[cnt+1] = '\0';
					    memset(linebak,0,200);
	                }

					do {
						if ((value[0] == 0x0a) || (value[0] == ' ') || (value[0] == '  ')) {
							tmpbuf[--pstr] = '\0';
							break;
						}
						memcpy(&tmpbuf[pstr], value, strlen(value));
						pstr += strlen(value);
					} while(fgets(value, 200, fp));
	
					cnt = 0;
					pch = strtok(tmpbuf, ";");
				    if (pch != NULL) {
					    //strncpy(&config->venue_name[config->venue_cnt][pstr], pch, 3);
						//cnt++;
	                    //pstr += 3;
	                    while(1) {
			                pch = strtok(NULL, ";");
			                if (pch != NULL) {
								if (cnt%2) {
									if(config->venue_cnt >= MAX_VENUE_NUM) {
										printf("num of venue name > %d (MAX_VENUE_NUM)\n",MAX_VENUE_NUM);
										return -1;
									}
									config->venue_name[config->venue_cnt] = (unsigned char *) calloc(strlen(pch)+3+1, sizeof(unsigned char));									
									strncpy(&config->venue_name[config->venue_cnt][0], lang, 3);
					                strncpy(&config->venue_name[config->venue_cnt][3], pch, strlen(pch));
							        config->venue_cnt++;
				                }
								else {
									strncpy(lang, pch, 3);							        
								}
						        cnt++;
							}
							else
								break;
						}
					}
				}				

            }
			else if (!strcmp(token, "hessid")) {
				ptr = &value[0];
	            for (i=0;i<strlen(value)/2;i++, ptr+=2)
		        {
			        config->hessid[i] = convert_atob(ptr, 16);
				}
				config->hessid[i] = '\0';
			}
			else if (!strcmp(token, "redirect_dst")) {
                ptr = &value[0];
                for (i=0;i<strlen(value)/2;i++, ptr+=2)
                {
                    config->redir_mac[i] = convert_atob(ptr, 16);
                }
                config->redir_mac[i] = '\0';
            }	
			else if (!strcmp(token, "roamingconsortiumoi")) {
				char tmpstr[16];
				
				pch = strtok(value, ";");
				while (pch != NULL)
				{
					ptr = tmpstr;
					strcpy(tmpstr, pch);
					config->roi_len[config->roi_cnt] = strlen(tmpstr)/2;
					for (i=0;i<strlen(tmpstr)/2;i++, ptr+=2)
					{
						config->roi[config->roi_cnt][i] = convert_atob(ptr, 16);
					}
					config->roi_cnt++;
					pch = strtok(NULL, ";");
				}
            }
			else if (!strcmp(token, "advertisementproid")) {
				config->advtid = atoi(value);
			}
			else if (!strncmp(token, "domainname", 10)) {
				pch = strtok(value,";");
                if (pch != NULL)
                {
                	if(config->domain_name[config->domain_cnt] == NULL)
						config->domain_name[config->domain_cnt] = (unsigned char *) calloc(strlen(value)+1, sizeof(unsigned char));	
					else
						printf("domain_name error (should not be here)\n");
					
					strcpy(config->domain_name[config->domain_cnt], value);
					config->domain_cnt++;
					while(1) {
                        pch = strtok(NULL, ";");
                        if (pch != NULL)
                        {
                        	if(config->domain_cnt >= MAX_DOMAIN_NUM) {
								printf("num of domain name > %d (MAX_DOMAIN_NUM)\n",MAX_DOMAIN_NUM);
								return -1;
							}
                        	if(config->domain_name[config->domain_cnt] == NULL)
								config->domain_name[config->domain_cnt] = (unsigned char *) calloc(strlen(value)+1, sizeof(unsigned char));	
							else
								printf("2. domain_name error (should not be here)\n");
							strcpy(config->domain_name[config->domain_cnt], value);
		                    config->domain_cnt++;
						}
						else
							break;
					}
				}
            }
			else if (!strncmp(token, "networkauthtype", 15)) {
				int cnt = 0;
				pch = strtok(value,";");
                if (pch != NULL)
				{
					config->netauth_type[config->netauth_cnt] = atoi(pch);
					config->netauth_cnt++;
					cnt++;

					while(1) {
						pch = strtok(NULL, ";");					
						if (pch != NULL)
						{
							if (cnt%2) {
								if(config->redirectURL[config->netauth_cnt-1]==NULL)
									config->redirectURL[config->netauth_cnt-1] = calloc(strlen(pch)+1,sizeof(unsigned char));
								else
									printf("should not be here (redirectURL)\n");
								
								strcpy(config->redirectURL[config->netauth_cnt-1], pch);								
							}
							else {
								if(config->netauth_cnt >= MAX_REDIR_URL_NUM) {
									printf("num of redirectURL > %d (MAX_REDIR_URL_NUM)\n",MAX_REDIR_URL_NUM);
									return -1;
								}
								config->netauth_type[config->netauth_cnt] = atoi(pch);
								config->netauth_cnt++;
							}
							cnt++;
						}
						else
							break;
					}
				}
            }
			else if (!strcmp(token, "ipv4type")) {
                config->ipv4type = atoi(value);
            }
			else if (!strcmp(token, "ipv6type")) {
                config->ipv6type = atoi(value);
            }
			else if (!strncmp(token, "operatorfriendlyname", 20)) {
				int pstr = 0;
                int cnt = 0;
				unsigned char lang[3];
                unsigned char tmpbuf[500];
				

				if ((config->sigma_test == 1) && (!strcmp(value, "id1"))) {
					config->op_name[config->op_cnt] = calloc(strlen(opername_id1[0])+1,sizeof(unsigned char));
					strcpy(config->op_name[config->op_cnt], opername_id1[0]);
					config->op_cnt++;
					config->op_name[config->op_cnt] = calloc(strlen(opername_id1[1])+1,sizeof(unsigned char));
					strcpy(config->op_name[config->op_cnt], opername_id1[1]);
					config->op_cnt++;
				}
				else {
	                if (linebak[strlen(value)+strlen("operatorfriendlyname")+1] == 0x0a) {
		                cnt = strlen(value);
			            value[cnt] = 0x0a;
				        value[cnt+1] = '\0';
					    memset(linebak,0,200);
	                }

		            do {
			            if ((value[0] == 0x0a) || (value[0] == ' ') || (value[0] == '  ')) {
				            tmpbuf[--pstr] = '\0';
					        break;
						}
	                    memcpy(&tmpbuf[pstr], value, strlen(value));
		                pstr += strlen(value);
			        } while(fgets(value, 200, fp));

	                pstr = 0;
					cnt = 0;
		            pch = strtok(tmpbuf, ";");
					if (pch != NULL) {
	                    while(1) {
		                    pch = strtok(NULL, ";");
			                if (pch != NULL) {
				                if (cnt%2) {
									if(config->op_cnt >= MAX_OPNAME_NUM) {
										printf("num of op name > %d (MAX_OPNAME_NUM)\n",MAX_OPNAME_NUM);
										return -1;
									}
									config->op_name[config->op_cnt] = calloc(strlen(pch)+3+1,sizeof(unsigned char));
									strncpy(&config->op_name[config->op_cnt][0], lang, 3);
					                strncpy(&config->op_name[config->op_cnt][3], pch, strlen(pch));
							        config->op_cnt++;
	                            }
		                        else {
			                        strncpy(lang, pch, 3);
					            }
						        cnt++;
							}
	                        else
		                        break;
						}
                    }
                }                
            }
		}
#ifdef __ECOS
		fclose(fp);
#endif
	}

	return 0;
}

static int parse_argument(pHS2CTX pHS2, int argc, char *argv[])
{
	int argNum=0;

	pHS2->cfg_num=0;
	while (argNum < argc) {
        if ( !strcmp(argv[argNum], "-c")) {
            if (++argNum >= argc)
                break;
			if (pHS2->cfg_num == MAX_CONF_NUM) {
				printf("MAXIMUM CONFIG NUMBER is %d\n", MAX_CONF_NUM);
				return -1;
			}	
			pHS2->cfg_filename[pHS2->cfg_num] = calloc(strlen(argv[argNum]),sizeof(unsigned char));
            strcpy(pHS2->cfg_filename[pHS2->cfg_num++], argv[argNum]);
        }
		argNum++;
	}

	if (pHS2->cfg_num == 0)
		return -1;
	return 0;
}

static void sigHandler_user(int signo)
{
	pHS2CTX pHS2 = pGlobalHS2;
	struct hs2_realm        *prealm, *poldrealm;;
    struct hs2_eap_method   *pmethod, *poldmethod;
    struct proto_port       *proto, *poldproto;
    struct hs2_plmn         *plmn, *poldplmn;
	int i,j;

	printf("kill hs2 daemon\n");

	for(i=0;i<pHS2->cfg_num;i++)	{
		free(pHS2->cfg_filename[i]);
		prealm = pHS2->hs2conf[i].realm;
		while (prealm != NULL)	{
			pmethod = prealm->eap_method;
			while (pmethod != NULL)	{
				poldmethod = pmethod;
				pmethod = pmethod->next;
				free(poldmethod);
			}		
			poldrealm = prealm;
			prealm = prealm->next;
			free(poldrealm);
		}

		proto = pHS2->hs2conf[i].proto;
		while (proto != NULL)	{
			poldproto = proto;
			proto = proto->next;
			free(poldproto);
		}
		
		plmn = pHS2->hs2conf[i].plmn;
		while (plmn != NULL)	{
			poldplmn = plmn;
			plmn = plmn->next;
			free(poldplmn);
		}
		for (j=0;j<MAX_STA_NUM;j++) {
	        if (pHS2->hs2conf[i].sta[j] != NULL)   {
				free(pHS2->hs2conf[i].sta[j]);
			}
		}
		for (j=0;j<MAX_VENUE_NUM;j++) {
			if (pHS2->hs2conf[i].venue_name[j]!= NULL)   {
				free(pHS2->hs2conf[i].venue_name[j]);
			}
		}
		for (j=0;j<MAX_VENUE_NUM;j++) {
			if (pHS2->hs2conf[i].domain_name[j]!= NULL)   {
				free(pHS2->hs2conf[i].domain_name[j]);
			}
		}
		for (j=0;j<MAX_REDIR_URL_NUM;j++) {
			if (pHS2->hs2conf[i].redirectURL[j]!= NULL)   {
				free(pHS2->hs2conf[i].redirectURL[j]);
			}
		}
		for (j=0;j<MAX_OPNAME_NUM;j++) {
			if (pHS2->hs2conf[i].op_name[j]!= NULL)   {
				free(pHS2->hs2conf[i].op_name[j]);
			}
		}
	}
		
	free(pHS2);
}
#ifdef __ECOS
int hs2_start(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int argNum=1, i=0, ifindex, pid_fd;
	struct ifreq ifr;
	//struct sockaddr_ll addr;
	pHS2CTX pHS2;
	pHS2 = (pHS2CTX) calloc(1, sizeof(HS2CTX));
	
	if (pHS2 == NULL) {
        printf("allocate context failed!\n");
        return -1;
    }

	pGlobalHS2 = pHS2;
    memset(pHS2, 0, sizeof(pHS2));
	if (parse_argument(pHS2, argc, argv) < 0) {
		printf("parse argument error!, should \"-C config_files\"\n");
		return -1;
	}
	if(generate_hs2_config_file(pHS2) < 0 ){
		printf("generate config file failed\n");
		return -1;
	}
	if (parse_configfile(pHS2) < 0) {
		printf("parse config file error!\n");
		return -1;
	}
#ifndef __ECOS

	if (daemon(0,1) == -1) {
        printf("fork error!\n");
        exit(1);
    }

	//dump_config(&pHS2->hs2conf[0]);

	signal(SIGTERM, sigHandler_user);

	{
		unsigned char tmpstr[100];
		FILE *out;
		
		tmpstr[0] = '\0';
		for(i=0;i<pHS2->cfg_num;i++)
		{
			strcat(tmpstr, "_");
			HS2_DEBUG_INFO("iface=%s\n", pHS2->hs2conf[i].wlan_iface);
			strcat(tmpstr, pHS2->hs2conf[i].wlan_iface);
		}
		sprintf(pHS2->pid_filename, "%s%s.pid", DEFAULT_PID_FILENAME, tmpstr);

		if((out = fopen("/var/hs2_pidname", "w")) != NULL) {
			fprintf(out, "%s", pHS2->pid_filename);
	        fclose(out);
		}
    }
	
	printf("pid name=%s\n", pHS2->pid_filename);
	pid_fd = pidfile_acquire(pHS2->pid_filename);
    if (pid_fd < 0)
	    return 0;

    pidfile_write_release(pid_fd);
	
	if (hs2_init_fifo(pHS2) < 0) 
	{
		printf("HS2 Init fifo fail.\n");
		return -1;
	}
#endif
	//fill hs2 ie in beacon and probe resp
	hs2_fill_ie(pHS2);

#ifndef __ECOS
	//process gas query and reply
	hs2_daemon(pHS2);
#else
	creat_hs2();
#endif
	return 0;
}

