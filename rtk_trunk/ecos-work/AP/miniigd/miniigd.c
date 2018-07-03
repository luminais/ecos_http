/* project: miniUPnP
 * webpage: http://miniupnp.free.fr/
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file provided in the distribution */
/* $Id: miniigd.c,v 1.19 2009/10/26 12:09:23 bert Exp $ */
/* system or libc include : */
#ifdef __ECOS
#include <network.h>
#include "../apmib/apmib.h"
#endif
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#ifndef __ECOS
#include <sys/file.h>
#include <syslog.h>
#endif
#include <sys/time.h>
#include <time.h>
#include <signal.h>
/* for BSD's sysctl */
#include <sys/param.h>
#ifndef __ECOS
#include <sys/sysctl.h>
#endif
#include <errno.h>
#include <ctype.h>
#if 0
//#ifdef SUPPORT_HNAP
#include <sys/un.h>
#define UNIX_SOCK_PATH "/tmp/mysocket"
#endif
#include <sys/ioctl.h>
#ifndef __ECOS
#include <linux/wireless.h>
#include <sys/sysinfo.h>
#else
#include <net/if.h>			/* for IFNAMSIZ and co... */
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif

/* miniupnp includes : */
#include "upnpglobalvars.h"
#include "miniigdhttp.h"
#include "upnpdescgen.h"
#include "miniupnpdpath.h"
#include "getifaddr.h"
#include "daemonize.h"
#include "miniigdsoap.h"
#include "built_time"
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif

#define VERSION_STR	"v1.07"
#define DISPLAY_BANNER \
	printf("\nMiniIGD %s (%s).\n\n", VERSION_STR, BUILT_TIME)
#if defined(DEBUG)
#define DEBUG_ERR(fmt, args...) printf(fmt, ## args)
#else
#define DEBUG_ERR(fmt, args...)
#endif
#ifdef ENABLE_EVENTS
#include "upnpevents.h"
#endif

#define CYGNUM_MINIIGD_THREAD_PRIORITY 16
#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
extern int  dbg_igd_index1;
extern int  dbg_igd_index2;

#endif


#if defined(ENABLE_NATPMP)
#include "natpmp.h"
#endif

#define IGD_FILE "/tmp/igd_config"
#define IGD_FIREWALL "/tmp/firewall_igd"
#if defined(CONFIG_RTL8186_GR) || defined(CONFIG_RTL8186_GW)
#define UPNP_INIT "/tmp/upnp_init"
#endif
/* The amount of time before advertisements
   will expire */
#define DEFAULT_ADVR_EXPIRE 180
#define MAC_ADDR_LEN    6	//added by winfred_wang 

//Brad add 20081205
#define PICS_INIT_PORT		52869
#define WAN_IF_1 ("eth1")
#define WAN_IF_2 ("ppp0")
#define WAN_IF_2_1 ("ppp1")
#define WAN_IF_2_2 ("ppp2")
#define WAN_IF_2_3 ("ppp3")
#define WAN_IF_3 ("wlan")
#define CHECK_WAN_INTERVAL 1 /* seconds to check WAN status */
//typedef enum { DHCP_DISABLED=0, DHCP_CLIENT=1, DHCP_SERVER=2, PPPOE=3, PPTP=4, BIGPOND=5, L2TP=6, PPPOE_RU=7, PPTP_RU=8, L2TP_RU=9 } DHCP_T;

#ifdef STAND_ALONE
#define PICS_DESC_NAME "picsdesc"
#define MINIIGD_SERVER_STRING "OS 1.0 UPnP/1.0 Realtek/V1.3"
#endif
//#define PSIMPLECFG_SERVICE_DESC_DOC "simplecfgservice"
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

/* ip et port pour le SSDP */
#define PORT (1900)
#define UPNP_MCAST_ADDR ("239.255.255.250")

#define MAX_ADD_LISTEN_ADDR (4)

static volatile int quitting = 0;
static char internal_if[MAX_LEN_IF_NAME];
//static int wan_type = -1;
//static int wisp_interface_num = -1;
//static char wisp_if_name[16];
static char is_miniigd_runnig = 0;
int wan_type = -1;
int wisp_interface_num = -1;
char wisp_if_name[16];
static int need_add_iptables=0;
static int sessionSel=-1;	//0: ppp0; 1: ppp1; 2: ppp0 & ppp1.
#ifdef MULTI_PPPOE
static int sessionNumber = 1; //default one ppp connect
static int rule_set_number = 0; //
int cur_session = -1;	//used for loop pppx,eg: ppp0->ppp1->ppp2->ppp3->ppp0
char deviceName[4][32];
#endif
#ifdef __ECOS
unsigned char miniigd_stack[12*1024];
cyg_handle_t miniigd_thread_handle;
cyg_thread miniigd_thread_obj;
unsigned char wan_change_flag = 0;
#endif
#ifdef CONFIG_RTL8186_GR
int pppCutEnabled=1;	//(only used for pppoe) 0: disabled; 1: enabled (default)
#endif
#if 0
char igd_config[512];
#endif
#ifdef __ECOS
extern picsdesc_xml[];
extern picsdesc_skl[];

#endif
extern int is_wan_connected(void);
#ifndef __ECOS
extern int backup_rules(char *);
extern int recover_rules(void);
#endif


#ifndef __ECOS

static int read_config_file(char *filename, int port_num)
{

	FILE *fp;
	char line[200], token[40], value[100], *ptr=NULL;
	int i;
	char tmp_url[100];
	char url_port[10];
	fp = fopen(filename, "r");
	if (fp == NULL) {
		DEBUG_ERR("read config file [%s] failed!\n", filename);
		return -1;
	}
	while ( fgets(line, 200, fp) ) {
		if (line[0] == '#')
			continue;
		ptr = get_token(line, token);
		if (ptr == NULL)
			continue;
		if (get_value(ptr, value)==0)
			continue;
		else if (!strcmp(token, "uuid")) {
			if (strlen(value) != UUID_LEN) {
				DEBUG_ERR("Invalid uuid length!\n");
				return -1;
			}
			sprintf(uuidvalue, "uuid:%s", value);
		}	
		else if (!strcmp(token, "server_name")) {
			if (strlen(value) > (MAX_SERVER_LEN-1)) {
				DEBUG_ERR("Invalid manufacturer length [%d]!\n", strlen(value));
				return -1;
			}
			sprintf(server_id, "%s", value);
		}
		else if (!strcmp(token, "location_url")) {
			if (strlen(value) > (MAX_SERVER_LEN-1)) {
				DEBUG_ERR("Invalid location URL length [%d]!\n", strlen(value));
				return -1;
			}
			ptr=strstr(value, "PORT");
			if(ptr!=NULL){
			snprintf(tmp_url, (ptr-value+1), "%s",value);
			sprintf(url_port, "%d", port_num);
			strcat(tmp_url, url_port);
			strcat(tmp_url, ptr+4);
			}
			sprintf(location_url, "%s", tmp_url);
		}
		else if (!strcmp(token, "ssdp_ext")) {
			if (strlen(value) > (MAX_SERVER_LEN-1)) {
				DEBUG_ERR("Invalid SSDP EXT length [%d]!\n", strlen(value));
				return -1;
			}
			sprintf(ssdp_ext, "%s", value);
		}
		else if (!strcmp(token,"ssdp_mx")) 
					ssdp_mx = atoi(value);		
	}
	fclose(fp);

	return 0;

	//sprintf(server_id, "%s", "miniupnpd/1.0 UPnP/1.0");
	//sprintf(location_url, "%s", "http://192.168.1.254:52869/picsdesc.xml");
	//sprintf(ssdp_ext, "%s", " ");
	//ssdp_mx = 20;
	//return 0;

	
}
#endif





static int MiniigdOpenAndConfHTTPSocket(const char * addr, unsigned short port)
{
	int s;
	int i = 1;
	struct sockaddr_in listenname;

	s = socket(PF_INET, SOCK_STREAM, 0);
	if(s<0)
	{
		syslog(LOG_ERR, "socket(http): %m");
		return s;
	}
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif


	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) < 0)
	{
		//close(s);
		syslog(LOG_WARNING, "setsockopt(http, SO_REUSEADDR): %m");
	}

#if defined(CONFIG_RTL_819X) && defined(CONFIG_RTL_SORAPIDRECYCLE)
	if(setsockopt(s, SOL_SOCKET, SO_RAPIDRECYCLE, &i, sizeof(i)) < 0)
	{
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		syslog(LOG_WARNING, "setsockopt(http, SO_RAPIDRECYCLE): %m");
		return -1;
	}
#endif
	memset(&listenname, 0, sizeof(struct sockaddr_in));
	listenname.sin_family = AF_INET;
	listenname.sin_port = htons(port);
	listenname.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(s, (struct sockaddr *)&listenname, sizeof(struct sockaddr_in)) < 0)
	{
		syslog(LOG_ERR, "bind(http): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
	if(listen(s, 6) < 0)
	{
		syslog(LOG_ERR, "listen(http): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
	return s;
}

/*
 * response from a LiveBox (Wanadoo)
HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Thu, 01 Jan 1970 04:03:23 GMT
EXT:
LOCATION: http://192.168.0.1:49152/gatedesc.xml
SERVER: Linux/2.4.17, UPnP/1.0, Intel SDK for UPnP devices /1.2
ST: upnp:rootdevice
USN: uuid:75802409-bccb-40e7-8e6c-fa095ecce13e::upnp:rootdevice

 * response from a Linksys 802.11b :
HTTP/1.1 200 OK
Cache-Control:max-age=120
Location:http://192.168.5.1:5678/rootDesc.xml
Server:NT/5.0 UPnP/1.0
ST:upnp:rootdevice
USN:uuid:upnp-InternetGatewayDevice-1_0-0090a2777777::upnp:rootdevice
EXT:
 */

/* not really an SSDP "announce" as it is the response
 * to a SSDP "M-SEARCH" */
static void MiniigdSendSSDPAnnounce2(int s, struct sockaddr_in sockname,
                              const char * st, int st_len,
							  const char * host, unsigned short port)
{
	int l, n;
	char *buf;
	/* TODO :
	 * follow guideline from document "UPnP Device Architecture 1.0"
	 * put in uppercase.
	 * DATE: is recommended
	 * SERVER: OS/ver UPnP/1.0 miniupnpd/1.0
	 * */
	buf = (char *)malloc(512);
	if (buf == NULL) {
		syslog(LOG_ERR, "MiniigdSendSSDPAnnounce2: out of memory!");
		return;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 512);
#endif

	memset(buf, 0, 512);	
	l = snprintf(buf, 512, "HTTP/1.1 200 OK\r\n"
		"Cache-Control: max-age=120\r\n"
		"ST: %.*s\r\n"
		"USN: %s::%.*s\r\n"
		"EXT:\r\n"
		"Server: miniupnpd/1.0 UPnP/1.0\r\n"
		"Location: http://%s:%u" ROOTDESC_PATH "\r\n"
		"\r\n",
		st_len, st,
		uuidvalue, st_len, st,
		host, (unsigned int)port);
	n = sendto(s, buf, l, 0,
	           (struct sockaddr *)&sockname, sizeof(struct sockaddr_in) );
	if(n<0)
	{
		syslog(LOG_ERR, "sendto: %m");
	}
	free(buf);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 512);
#endif

}

static const char * const known_service_types[] =
{
	"upnp:rootdevice",
	"urn:schemas-upnp-org:device:InternetGatewayDevice:",
	"urn:schemas-upnp-org:device:WANConnectionDevice:",
	"urn:schemas-upnp-org:device:WANDevice:",
	"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:",
	"urn:schemas-upnp-org:service:WANIPConnection:",
	"urn:schemas-upnp-org:service:WANPPPConnection:",
	//"urn:schemas-upnp-org:service:Layer3Forwarding:",
	//"urn:schemas-microsoft-com:service:OSInfo:",
	0
};

#ifdef STAND_ALONE
void MiniigdSendSSDPNotifies(int s, const char * host, unsigned char type)
{
	struct sockaddr_in sockname;
	int n, i, j;
	char *bufr=NULL;

	if (host == NULL )
		return;
	bufr = (char *) malloc(512);
	if (bufr == NULL) {
		syslog(LOG_ERR, "MiniigdSendSSDPNotifies: out of memory!");
		return;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 512);
#endif
	memset(bufr, 0, 512);
	
	memset(&sockname, 0, sizeof(struct sockaddr_in));
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(PORT);
	sockname.sin_addr.s_addr = inet_addr(UPNP_MCAST_ADDR);

	for (j=0; j <2; j++) {
		i = 0;
		while(known_service_types[i])
		{
			if (i == 1) {
				sprintf(bufr,
					"NOTIFY * HTTP/1.1\r\n"
					"Host:%s:%d\r\n"
					"Cache-Control:max-age=%d\r\n"
					"Location:http://%s:%d/%s.xml" "\r\n"
					"Server:" MINIIGD_SERVER_STRING "\r\n"
					"NT:%s\r\n"
					"USN:%s\r\n"
					"NTS:ssdp:%s\r\n"
					"\r\n",
					UPNP_MCAST_ADDR, PORT, (ssdp_mx)?ssdp_mx:DEFAULT_ADVR_EXPIRE,
					host, PICS_INIT_PORT, PICS_DESC_NAME,
					uuidvalue, uuidvalue,
					(type==0?"alive":"byebye"));
				n = sendto(s, bufr, strlen(bufr), 0,
					(struct sockaddr *)&sockname, sizeof(struct sockaddr_in) );
				if(n<0)
				{
					syslog(LOG_ERR, "sendto: %m");
				}
			}
			sprintf(bufr,
				"NOTIFY * HTTP/1.1\r\n"
				"Host:%s:%d\r\n"
				"Cache-Control:max-age=%d\r\n"
				"Location:http://%s:%d/%s.xml" "\r\n"
				"Server:" MINIIGD_SERVER_STRING "\r\n"
				"NT:%s%s\r\n"
				"USN:%s::%s%s\r\n"
				"NTS:ssdp:%s\r\n"
				"\r\n",
				UPNP_MCAST_ADDR, PORT, (ssdp_mx)?ssdp_mx:DEFAULT_ADVR_EXPIRE,
				host, PICS_INIT_PORT, PICS_DESC_NAME,
				known_service_types[i], (i==0?"":"1"),
				uuidvalue, known_service_types[i], (i==0?"":"1"),
				(type==0?"alive":"byebye"));
			n = sendto(s, bufr, strlen(bufr), 0,
				(struct sockaddr *)&sockname, sizeof(struct sockaddr_in) );
			if(n<0)
			{
				syslog(LOG_ERR, "sendto: %m");
			}
			i++;
		}
	}

	free(bufr);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 512);
#endif

}

int MiniigdAddMulticastMembership(int s, const char * ifaddr)
{
	struct ip_mreq imr;	/* Ip multicast membership */

    	/* setting up imr structure */
    	imr.imr_multiaddr.s_addr = inet_addr(UPNP_MCAST_ADDR);
    	/*imr.imr_interface.s_addr = htonl(INADDR_ANY);*/
    	imr.imr_interface.s_addr = inet_addr(ifaddr);
	
	if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&imr, sizeof(struct ip_mreq)) < 0)
	{
        syslog(LOG_ERR, "setsockopt(udp, IP_ADD_MEMBERSHIP): %m");
		return -1;
    }

	return 0;
}

int MiniigdOpenAndConfUdpSocket(const char * ifaddr)
{
	int s, on=1;
	int opt=1;
	struct sockaddr_in sockname;
	if( (s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		syslog(LOG_ERR, "socket(udp): %m");
		return -1;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
	/*
	if( setsockopt( s, SOL_SOCKET, SO_REUSEADDR,(char *)&on, sizeof(on) ) != 0 ) {
                syslog(LOG_ERR, "setsockopt(udp): %m");
                return -1;
	}
	*/
	if( setsockopt( s, SOL_SOCKET, SO_REUSEPORT,(char *)&opt, sizeof(opt) ) != 0 ) {
        syslog(LOG_ERR, "setsockopt(udp): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
        return -1;
	}
	memset(&sockname, 0, sizeof(struct sockaddr_in));
    	sockname.sin_family = AF_INET;
    	sockname.sin_port = htons(PORT);
	/* NOTE : it seems it doesnt work when binding on the specific address */
    	/* sockname.sin_addr.s_addr = inet_addr(UPNP_MCAST_ADDR); */
    	sockname.sin_addr.s_addr = htonl(INADDR_ANY);
    	/* sockname.sin_addr.s_addr = inet_addr(ifaddr); */

    if(bind(s, (struct sockaddr *)&sockname, sizeof(struct sockaddr_in)) < 0)
	{
		syslog(LOG_ERR, "bind(udp): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
    }

	if(MiniigdAddMulticastMembership(s, ifaddr) < 0)
	{
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}

	return s;
}

/* open the UDP socket used to send SSDP notifications to
 * the multicast group reserved for them */
int MiniigdOpenAndConfNotifySocket(const char * addr)
{
	int s,on = 1;
	int bcast = 1;
	unsigned char loopchar = 0;
	struct in_addr mc_if;
	struct sockaddr_in sockname;
	if( (s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		syslog(LOG_ERR, "socket(udp_notify): %m");
		return -1;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
	mc_if.s_addr = inet_addr(addr);

	if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopchar, sizeof(loopchar)) < 0)
	{
		syslog(LOG_ERR, "setsockopt(udp_notify, IP_MULTICAST_LOOP): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}

	if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&mc_if, sizeof(mc_if)) < 0)
	{
		syslog(LOG_ERR, "setsockopt(udp_notify, IP_MULTICAST_IF): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
	if( setsockopt( s, SOL_SOCKET, SO_REUSEADDR,(char *)&on, sizeof(on) ) != 0 ) {
        syslog(LOG_ERR, "setsockopt(udp): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
        return -1;
	}
	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, &bcast, sizeof(bcast)) < 0)
	{
		syslog(LOG_ERR, "setsockopt(udp_notify, SO_BROADCAST): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
	/*if(setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, "eth0", strlen("eth0")) < 0)
	{
		syslog(LOG_ERR, "setsockopt(udp_notify, SO_BROADCAST): %m");
		close(s);
		return -1;
	}*/
	memset(&sockname, 0, sizeof(struct sockaddr_in));
	sockname.sin_family = AF_INET;
	//sockname.sin_addr.s_addr = inet_addr(addr);
	sockname.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (struct sockaddr *)&sockname, sizeof(struct sockaddr_in)) < 0)
	{
		syslog(LOG_ERR, "bind(udp_notify): %m");
		close(s);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
	
	return s;

}
#endif

void MiniigdProcessSSDPRequest(int s, const char * host, unsigned short port)
{
	int n;
	char *bufr;
	socklen_t len_r;
	struct sockaddr_in sendername;
	int i, l;
	char * st = 0;
	int st_len = 0;
	bufr = (char *)malloc(2048);
	if(bufr == NULL){
		syslog(LOG_ERR, "MiniigdProcessSSDPRequest: out of memory!");
		return;
	}
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 2048);
#endif
	memset(bufr, 0, 2048);
	len_r = sizeof(struct sockaddr_in);
	n = recvfrom(s, bufr, 2048, 0,
	             (struct sockaddr *)&sendername, &len_r);
	if(n<0)
	{
		syslog(LOG_ERR, "recvfrom: %m");
		free(bufr);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 2048);
#endif
		return;
	}
	if(memcmp(bufr, "NOTIFY", 6) == 0)
	{
		/* ignore NOTIFY packets. We could log the sender and device type */
		free(bufr);
#ifdef	ECOS_DBG_STAT
		dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 2048);
#endif
		return;
	}
	else if(memcmp(bufr, "M-SEARCH", 8) == 0)
	{
		i = 0;
		while(i<n)
		{
			while(bufr[i] != '\r' || bufr[i+1] != '\n')
			{
				//modify
				#if 1 //def MULTI_PPPOE
				if(i<n)
				i++;
				else{
					free(bufr);
#ifdef	ECOS_DBG_STAT
					dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 2048);
#endif
					return ;
				}
				#endif 
				// ori code
				/*
				i++;
				*/
			}
			i += 2;
			if(strncasecmp(bufr+i, "st:", 3) == 0)
			{
				st = bufr+i+3;
				st_len = 0;
				while(*st == ' ' || *st == '\t') st++;
				while(st[st_len]!='\r' && st[st_len]!='\n') st_len++;
				/*syslog(LOG_INFO, "ST: %.*s", st_len, st);*/
				/*j = 0;*/
				/*while(bufr[i+j]!='\r') j++;*/
				/*syslog(LOG_INFO, "%.*s", j, bufr+i);*/
			}
		}
		syslog(LOG_INFO, "SSDP M-SEARCH packet received from %s:%d",
	           inet_ntoa(sendername.sin_addr),
	           ntohs(sendername.sin_port) );
		if(st)
		{
			/* TODO : doesnt answer at once but wait for a random time */
			syslog(LOG_INFO, "ST: %.*s", st_len, st);
			i = 0;
			while(known_service_types[i])
			{
				l = (int)strlen(known_service_types[i]);
				if(l<=st_len && (0 == memcmp(st, known_service_types[i], l)))
				{
					MiniigdSendSSDPAnnounce2(s, sendername, st, st_len, host, port);
					break;
				}
				i++;
			}
			l = (int)strlen(uuidvalue);
			if(l==st_len && (0 == memcmp(st, uuidvalue, l)))
			{
				MiniigdSendSSDPAnnounce2(s, sendername, st, st_len, host, port);
			}
		}
	}
	else
	{
		syslog(LOG_NOTICE, "Unknown udp packet received from %s:%d",
		       inet_ntoa(sendername.sin_addr),
			   ntohs(sendername.sin_port) );
	}
	free(bufr);
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_DEL, 2048);
#endif

}

/* This will broadcast ssdp:byebye notifications to inform 
 * the network that UPnP is going down. */
void miniupnpdShutdown()
{
	struct sockaddr_in sockname;
    int n,i=0;
    char *bufr;
	int s;
	struct in_addr local_if;
	bufr = (char *)malloc(512);
	if(bufr == NULL){
		syslog(LOG_ERR, "miniupnpdShutdown: out of memory!");
		return;
	}
	memset(bufr, 0, 512);
	if((s = socket(PF_INET, SOCK_DGRAM, 0))<0)
	{		
		syslog(LOG_ERR, "socket error \n");
		return ;
	}
	
	local_if.s_addr = inet_addr(listen_addr);
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&local_if, sizeof(local_if)) < 0)
	{
		syslog(LOG_ERR, "setsockopt - IP_MULTICAST_IF: %m");
	}

    memset(&sockname, 0, sizeof(struct sockaddr_in));
    sockname.sin_family = AF_INET;
    sockname.sin_addr.s_addr = inet_addr(listen_addr);/*INADDR_ANY;*/
	if(bind(s, (struct sockaddr *)&sockname, sizeof(struct sockaddr_in)) < 0)
		syslog(LOG_ERR, "bind: %m");

    memset(&sockname, 0, sizeof(struct sockaddr_in));
    sockname.sin_family = AF_INET;
    sockname.sin_port = htons(PORT);
    sockname.sin_addr.s_addr = inet_addr(UPNP_MCAST_ADDR);

    while(known_service_types[i])
    {
        snprintf(bufr, 512,
                 "NOTIFY * HTTP/1.1\r\n"
                 "HOST:%s:%d\r\n"
                 "NT:%s%s\r\n"
                 "USN:%s::%s%s\r\n"
                 "NTS:ssdp:byebye\r\n"
                 "\r\n",
                 UPNP_MCAST_ADDR, PORT,
				 known_service_types[i], (i==0?"":"1"),
                 uuidvalue, known_service_types[i], (i==0?"":"1"));
        n = sendto(s, bufr, strlen(bufr), 0,
                   (struct sockaddr *)&sockname, sizeof(struct sockaddr_in) );
		if(n<0)
		{
			syslog(LOG_ERR, "shutdown: sendto: %m");
		}
        i++;
    }
	close(s);
	if(unlink(pidfilename) < 0)
	{
		syslog(LOG_ERR, "failed to remove %s : %m", pidfilename);
	}
	free(bufr);
#ifndef __ECOS
	closelog();
#endif
	//exit(0);
}
#ifndef __ECOS
/* Write the pid to a file */
static void
writepidfile(const char * fname, int pid)
{
	char *pidstring;
	int pidstringlen;
	int pidfile;

	if(!fname || (strlen(fname) == 0))
		return;
	
	pidfile = open(fname, O_WRONLY|O_CREAT, 0666);
	if(pidfile < 0)
	{
		syslog(LOG_ERR, "Unable to write to pidfile %s: %m", fname);
	}
	else
	{
		pidstringlen = asprintf(&pidstring, "%d\n", pid);
		if(pidstringlen < 0)
		{
			syslog(LOG_ERR,
			       "asprintf failed, Unable to write to pidfile %s",
				   fname);
		}
		else
		{
			write(pidfile, pidstring, pidstringlen);
			free(pidstring);
		}
		close(pidfile);
	}
}
#endif
#ifndef __ECOS
static int substr(char *docinpath, char *infile, char *docoutpath, char *outfile, char *str_from, char *str_to)
{
	FILE *fpi, *fpo;
	char pathi[256], patho[256];
	char buffi[4096], buffo[4096];
	int len_buff, len_from, len_to;
	int i, j;

	sprintf(pathi, "%s%s", docinpath, infile);

	if ((fpi = fopen(pathi,"r")) == NULL) {
		printf("input file can not open\n");
		return (-1);
	}

	sprintf(patho, "%s%s", docoutpath, outfile);
	if ((fpo = fopen(patho,"w")) == NULL) {
		printf("output file can not open\n");
		fclose(fpi);
		return (-1);
	}

	len_from = strlen(str_from);
	len_to   = strlen(str_to);

	while (fgets(buffi, 4096, fpi) != NULL) {
		len_buff = strlen(buffi);
		for (i=0, j=0; i <= len_buff-len_from; i++, j++) {
			if (strncmp(buffi+i, str_from, len_from)==0) {
				strcpy (buffo+j, str_to);
				i += len_from - 1;
				j += len_to - 1;
			} else
				*(buffo + j) = *(buffi + i);
		}
		strcpy(buffo + j, buffi + i);
		fputs(buffo, fpo);
	}

	fclose(fpo);
	fclose(fpi);
	return (0);
}
#else
static int substr( char *infile, char *outfile, char *str_from, char *str_to)
{

	char *buffi, *buffo;
	int len_buff, len_from, len_to;
	int i, j;
	len_from = strlen(str_from);
	len_to   = strlen(str_to);
	len_buff = strlen(infile);
	buffi = infile;
	buffo = outfile;
		for (i=0, j=0; i <= len_buff-len_from; i++, j++) {
			if (strncmp(buffi+i, str_from, len_from)==0) {
				strcpy (buffo+j, str_to);
				i += len_from - 1;
				j += len_to - 1;
			} else
				*(buffo + j) = *(buffi + i);
		}
		strcpy(buffo + j, buffi + i);
	return (0);
}	
#endif
/* === main === */
/* call procession of HTTP or SSDP requests */

#ifdef SUPPORT_HNAP
extern void HNAP_GetPortMappingsResponse(void);
extern void HNAP_AddPortMapping(const char *in, const int len);
extern void HNAP_DeletePortMapping(const char *in, const int len);
extern char *miniigd_UPnP_UploadXML(char *file_path);
#if 0
static int OpenAndConfUNIXSocket(void)
{
	int s, len;
	struct sockaddr_un local;

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            	perror("UNIXSocket");
            	return -1;
    	}
	
    	local.sun_family = AF_UNIX;
     	strcpy(local.sun_path, UNIX_SOCK_PATH);
     	unlink(local.sun_path);
     	//len = strlen(local.sun_path) + sizeof(local.sun_family);
     	len = sizeof(struct sockaddr_un);
    	if (bind(s, (struct sockaddr *)&local, len) == -1) {
            	perror("UNIXSocket bind");
            	return -1;
     	}

     	if (listen(s, 5) == -1) {
            	perror("UNIXSocket listen");
            	return -1;
     	}

	return s;
}
#endif
static void sigHandler_alarm(int signo)
{
	struct stat status;
	char *in;
	
	if (lstat("/tmp/hnap_igd_send", &status) == 0) {
		if ((in = miniigd_UPnP_UploadXML("/tmp/hnap_igd_send")) != NULL) {
			int len;
			char *p;
			char *p_end;
			char num[20];

			unlink("/tmp/hnap_igd_send");
			syslog(LOG_INFO, "remove /tmp/hnap_igd_send");
				
			if (memcmp("AddPortMapping", in, 14) == 0) {
				p = in + 15;
				p_end = p;
				while (*p_end != '\n')
					p_end++;
				memcpy(num, p, p_end-p);
				num[p_end-p] = 0;
				len = atoi(num);
				p_end++;
				HNAP_AddPortMapping(p_end, len);
			}
			else if (memcmp("DeletePortMapping", in, 17) == 0) {
				p = in + 18;
				p_end = p;
				while (*p_end != '\n')
					p_end++;
				memcpy(num, p, p_end-p);
				num[p_end-p] = 0;
				len = atoi(num);
				p_end++;
				HNAP_DeletePortMapping(p_end, len);
			}
			else if (memcmp("GetPortMappings", in, 15) == 0) {
				HNAP_GetPortMappingsResponse();
			}

			free(in);
		}
		else {
			unlink("/tmp/hnap_igd_send");
			syslog(LOG_INFO, "remove /tmp/hnap_igd_send");
		}
	}

	alarm(1);
}
#endif
#if 0
static int miniupnp_WriteConfigFile(const int port)
{
#ifndef __ECOS
	FILE *fp;
#endif
	char *buffo;
#ifndef __ECOS
	if ((fp = fopen(IGD_FILE,"w")) == NULL) {
		printf("%s %d : open %s failed\n", __FUNCTION__, __LINE__, IGD_FILE);
		return -1;
	}
#endif
	buffo = (char *) malloc(512);
	if (buffo == NULL) {
#ifndef __ECOS
		fclose(fp);
#endif
		return -1;
	}
	memset(buffo, 0, 512);
	sprintf(buffo, "port %d\n", port);
#ifndef __ECOS
	fputs(buffo, fp);
#else
	strcat(igd_config,buffo);
#endif
	sprintf(buffo, "max_age %d\n", DEFAULT_ADVR_EXPIRE);
#ifndef __ECOS
	fputs(buffo, fp);
#else
	strcat(igd_config,buffo);
#endif

	sprintf(buffo, "uuid %s\n", uuidvalue);
#ifndef __ECOS
	fputs(buffo, fp);
#else
	strcat(igd_config,buffo);
#endif


	if(isCfgFromFile == 1){
		if(location_url[0]){
			sprintf(buffo, "location_url %s\n", location_url);
#ifndef __ECOS
				fputs(buffo, fp);
#else
				strcat(igd_config,buffo);
#endif

		}
		if(server_id[0]){
			sprintf(buffo, "server_id %s\n", server_id);
#ifndef __ECOS
			fputs(buffo, fp);
#else
			strcat(igd_config,buffo);
#endif

		}
		if(ssdp_ext[0]){
			sprintf(buffo, "ssdp_ext %s\n", ssdp_ext);
#ifndef __ECOS
			fputs(buffo, fp);
#else
			strcat(igd_config,buffo);
#endif

		}
		if(ssdp_mx > 0){
			sprintf(buffo, "ssdp_mx %d\n", ssdp_mx);
#ifndef __ECOS
			fputs(buffo, fp);
#else
			strcat(igd_config,buffo);
#endif

		}
	}
#ifndef __ECOS
	fputs("root_desc_name picsdesc\n", fp);
	fputs("known_service_types upnp:rootdevice\n", fp);
	fputs("known_service_types urn:schemas-upnp-org:device:InternetGatewayDevice:\n",fp);
	fputs("known_service_types urn:schemas-upnp-org:device:WANConnectionDevice:\n",fp);
	fputs("known_service_types urn:schemas-upnp-org:device:WANDevice:\n",fp);
	fputs("known_service_types urn:schemas-upnp-org:service:WANCommonInterfaceConfig:\n",fp);
	fputs("known_service_types urn:schemas-upnp-org:service:WANIPConnection:\n",fp);
	//fputs("known_service_types urn:schemas-upnp-org:service:WANPPPConnection:\n",fp);
	//fputs("known_service_types urn:schemas-upnp-org:service:Layer3Forwarding:\n",fp);
	fclose(fp);
#else
#endif
	free(buffo);
	return 0;
}
#endif
#ifndef __ECOS
static void RemoveIPtables(void)
{
	syslog(LOG_NOTICE, "Remove IP tables");
	system("iptables -t nat -X MINIUPNPD");
	system("iptables -t filter -X MINIUPNPD");
}

/* Handler for the SIGTERM signal (kill) */
void sigterm(int sig)
{
	if (sig != SIGTERM)
		return;
	
	/*int save_errno = errno;*/
	signal(sig, SIG_IGN);

//	syslog(LOG_NOTICE, "received signal %d, exiting", sig);

	quitting = 1;
	/*errno = save_errno;*/

	//by brian, backup rules to file when terminated
	backup_rules(BACKUP_RULES_FILENAME);
#ifndef __ECOS
	RemoveIPtables();
#else
	shutdown_redirect();
#endif
}


static void AddToChain(const char *if_name, const char *ip)
{
	char tmp[200];
#if defined(CONFIG_RTL8186_GR) || defined(CONFIG_RTL8186_GW)
	char tmpBuf[200];
#endif

	syslog(LOG_INFO, "miniigd : initial iptables on MINIUPNPD");
#ifdef MULTI_PPPOE	
	strcpy(deviceName[rule_set_number],if_name);	//save the set iface
	rule_set_number = rule_set_number % sessionNumber + 1;
	
	if(1 ==rule_set_number) //when first time enter, set the rule
	{
#endif	
	system("iptables -t nat -F MINIUPNPD");
	system("iptables -t filter -F MINIUPNPD");
#ifdef MULTI_PPPOE		
	}
#endif	
	sprintf(tmp, "iptables -t nat -A PREROUTING -d %s -i %s -j MINIUPNPD", ip, if_name);
	system(tmp);	
#if defined(CONFIG_RTL8186_GR) || defined(CONFIG_RTL8186_GW)
	sprintf(tmpBuf,"echo -n \"%s,\" > %s",tmp,UPNP_INIT);
	system(tmpBuf);
#endif
	syslog(LOG_INFO, "miniigd : execute [%s]", tmp);
	sprintf(tmp, "iptables -t filter -A FORWARD -i %s -o ! %s -j MINIUPNPD", if_name, if_name);
	system(tmp);
#if defined(CONFIG_RTL8186_GR) || defined(CONFIG_RTL8186_GW)
	sprintf(tmpBuf,"echo -n \"%s,\" >> %s",tmp,UPNP_INIT);
	system(tmpBuf);
#endif
	syslog(LOG_INFO, "miniigd : execute [%s]", tmp);
}

static void AddIPtables(const int new_created, const char *if_name, const char *ip)
{
	struct stat status;
	//creat new chian 
	if (new_created) {
		system("iptables -t nat -N MINIUPNPD");
		system("iptables -t filter -N MINIUPNPD");
		//syslog(LOG_INFO, "miniigd : create nat filter iptables");
	}
	if (strcmp(if_name, "lo")) {

		if (stat(IGD_FIREWALL, &status) == 0) {

			syslog(LOG_INFO, "miniigd : add iptables now!");
			AddToChain(if_name, ip);
			//by brian, trigger recovery when rules cleaned
			recover_rules();
#ifdef MULTI_PPPOE			
			if(rule_set_number == sessionNumber)
#endif				
			remove(IGD_FIREWALL);
			need_add_iptables = 0;
		}
		else {
			syslog(LOG_INFO, "miniigd : add iptables later!");
			need_add_iptables = 1;
		}
	}

}
#endif

static void CheckInterfaceChanged(const char *if_name)
{
	char ext_ip_addr_tmp[INET_ADDRSTRLEN];

	strcpy(ext_ip_addr_tmp, "127.0.0.1");
	if(getifaddr(if_name, ext_ip_addr_tmp, INET_ADDRSTRLEN) < 0) {
		//syslog(LOG_INFO, "miniigd : Failed to get IP of ext_if_name [%s]; changed ext_if_name to [lo]", if_name);
		//printf("miniigd : Failed to get IP of ext_if_name [%s]; changed ext_if_name to [lo]", if_name);
		if(getifaddr("lo", ext_ip_addr_tmp, INET_ADDRSTRLEN) < 0) {
			//syslog(LOG_INFO, "miniigd : Failed to get IP of ext_if_name [lo].");
		}
		if (strcmp(ext_if_name, "lo") || strcmp(ext_ip_addr, ext_ip_addr_tmp)) {
			strcpy(ext_if_name, "lo");
			strcpy(ext_ip_addr, ext_ip_addr_tmp);
			//syslog(LOG_INFO, "miniigd : Changed ext_if_name [%s]; changed ext_if_ip to [%s]", ext_if_name, ext_ip_addr);
		}
		else {
			//syslog(LOG_INFO, "miniigd : Unchanged ext_if_name [%s]; Unchanged ext_if_ip to [%s]", ext_if_name, ext_ip_addr);
		}
	}
	else {
		if (strcmp(ext_if_name, if_name) || strcmp(ext_ip_addr, ext_ip_addr_tmp)) {
			strcpy(ext_if_name, if_name);
			strcpy(ext_ip_addr, ext_ip_addr_tmp);
			syslog(LOG_INFO, "miniigd : Changed ext_if_name [%s]; changed ext_if_ip to [%s]", ext_if_name, ext_ip_addr);
			//printf("miniigd : Changed ext_if_name [%s]; changed ext_if_ip to [%s]", ext_if_name, ext_ip_addr);
			//AddIPtables(0, ext_if_name, ext_ip_addr);
		}
		//else {
			//syslog(LOG_INFO, "miniigd : Unchanged ext_if_name [%s]; Unchanged ext_if_ip to [%s]", ext_if_name, ext_ip_addr);
		//}
	}
}

#ifdef MULTI_PPPOE
/*
static int sessionNumber = 1; //default one ppp connect
static int rule_set_number = 0; //
int cur_session = -1;	//used for loop pppx,eg: ppp0->ppp1->ppp2->ppp3->ppp0
char deviceName[4][32];

*/

int IsAlreadySet(const char * iface)
{
	int index ;

	for(index = 0 ; index < rule_set_number ; ++index)
	{
		if(!strcmp(deviceName[index],iface))
			return 1;
	}
	return 0;
}

#endif
static void CheckWanStatus(void)
{	
	int result=0;
#ifndef __ECOS
	struct sysinfo system_info;

	if (need_add_iptables) {
		struct stat status;
		
#ifdef MULTI_PPPOE		
		if(strcmp(ext_if_name,"lo")){
#endif

		if (stat(IGD_FIREWALL, &status) == 0) {
			AddToChain(ext_if_name, ext_ip_addr);
			//by brian, trigger recovery when rules cleaned
			recover_rules();

#ifdef MULTI_PPPOE			
				if(rule_set_number == sessionNumber)
#endif	
				remove(IGD_FIREWALL);
				need_add_iptables = 0;
			}
			else {
				//syslog(LOG_INFO, "miniigd : add iptables later!");
				need_add_iptables = 1;
			}
#ifdef MULTI_PPPOE
		}
#endif		
	}
#endif	
	//int WANconnected = getWanLink("wlan0");
	
	//if (WANconnected > 0) {
		//syslog(LOG_INFO, "miniigd : WAN [%d] connected", wan_type);
		//if (wisp_interface_num < 0)
		{
			switch (wan_type)
			{
				case PPPOE:
#ifdef MULTI_PPPOE						

					if(sessionSel >= 1)	//two pppoe connect
					{	
						cur_session = (cur_session+1)%sessionNumber;
						// 0 --->ppp0 : 1 --->ppp1 : 2 --->ppp2 : 3 --->ppp3
						if(0 == cur_session)
						{							
							if(IsAlreadySet(WAN_IF_2))
								return;								
							CheckInterfaceChanged(WAN_IF_2);						
						}
						else if(1 == cur_session)
						{		
							if(IsAlreadySet(WAN_IF_2_1))
								return;								
							CheckInterfaceChanged(WAN_IF_2_1);	
						}
						else if(2 == cur_session)
						{
							if(IsAlreadySet(WAN_IF_2_2))
								return;								
							CheckInterfaceChanged(WAN_IF_2_2);	
						}
						else if(3 == cur_session)
						{
							if(IsAlreadySet(WAN_IF_2_3))
								return;								
							CheckInterfaceChanged(WAN_IF_2_3);	
						}						
					}
					else 
#endif					
					if(sessionSel==1)
						CheckInterfaceChanged(WAN_IF_2_1);
					else
						CheckInterfaceChanged(WAN_IF_2);
					break;
				case L2TP:
				case PPTP:
#if 0
				case PPPOE_RU:
				case PPTP_RU:
				case L2TP_RU:
#endif

					CheckInterfaceChanged(WAN_IF_2);
					break;
				case DHCP_CLIENT:
				//case DHCP_DISABLED:
				//case BIGPOND:
				default:
					if (wisp_interface_num < 0)
						CheckInterfaceChanged(WAN_IF_1);
					else
						CheckInterfaceChanged(wisp_if_name);
					break;
			}
		}
		//else {
			//CheckInterfaceChanged(wisp_if_name);
		//}
	//}
	//else {
		//syslog(LOG_INFO, "miniigd : WAN [%d] disconnected", wan_type);
		//CheckInterfaceChanged("lo");
	//}
	
	result = is_wan_connected();
	if(wan_states != result){
		wan_states = result;
		//sysinfo(&system_info);
		if(result ==4){ //link up
			wan_uptime = time(NULL);//system_info.uptime;
		}else
		{
			wan_uptime = 0;
		}
#ifdef ENABLE_EVENTS		
	upnp_event_var_change_notify(EWanCFG);
	upnp_event_var_change_notify(EWanIPC);
#endif	
	}
}

#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) ||  defined(CONFIG_RTL8186_TR) /* In order to accept empty character in HostName. Keith */	
#define MAXWORDLEN 256
int getword(FILE *f, char *word)
{
    int c, len, escape;
    int quoted, comment;
    int value, got;

    len = 0;
    escape = 0;
    comment = 0;

    /*
     * First skip white-space and comments.
     */
     
    for (;;) {
	c = getc(f);
	if (c == EOF)
	    break;

	/*
	 * A newline means the end of a comment; backslash-newline
	 * is ignored.  Note that we cannot have escape && comment.
	 */
	if (c == '\n') {
	    if (!escape) {
		comment = 0;
	    } else
		escape = 0;
	    continue;
	}

	/*
	 * Ignore characters other than newline in a comment.
	 */
	if (comment)
	    continue;

	/*
	 * If this character is escaped, we have a word start.
	 */
	if (escape)
	    break;

	/*
	 * If this is the escape character, look at the next character.
	 */
	if (c == '\\') {
	    escape = 1;
	    continue;
	}

	/*
	 * If this is the start of a comment, ignore the rest of the line.
	 */
	//if (c == '#') {
	//    comment = 1;
	//    continue;
	//}
	/*
	 * A non-whitespace character is the start of a word.
	 */
	if (!isspace(c))
	    break;
	
    }

    /*
     * Save the delimiter for quoted strings.
     */
    if (!escape && (c == '"' || c == '\'')) {
        quoted = c;
	c = getc(f);
    } else
        quoted = 0;

    /*
     * Process characters until the end of the word.
     */
    while (c != EOF) {
	if (escape) {
	    /*
	     * This character is escaped: backslash-newline is ignored,
	     * various other characters indicate particular values
	     * as for C backslash-escapes.
	     */
	    escape = 0;
	    if (c == '\n') {
	        c = getc(f);
		continue;
}

	    got = 0;
	    switch (c) {
	    case 'a':
		value = '\a';
		break;
	    case 'b':
		value = '\b';
		break;
	    case 'f':
		value = '\f';
		break;
	    case 'n':
		value = '\n';
		break;
	    case 'r':
		value = '\r';
		break;
	    case 's':
		value = ' ';
		break;
	    case 't':
		value = '\t';
		break;

	    default:
		/*
		 * Otherwise the character stands for itself.
		 */
		value = c;
		break;
	    }

	    /*
	     * Store the resulting character for the escape sequence.
	     */
	    if (len < MAXWORDLEN-1)
		word[len] = value;
	    ++len;

	    if (!got)
		c = getc(f);
	    continue;

	}

	/*
	 * Not escaped: see if we've reached the end of the word.
	 */
	if (quoted) {
	    if (c == quoted)
		break;
	} else {
	    //if (isspace(c) || c == '#') {
	    if (isspace(c)) {	
		ungetc (c, f);
		break;
	    }
	}

	/*
	 * Backslash starts an escape sequence.
	 */
	if (c == '\\') {
	    escape = 1;
	    c = getc(f);
	    continue;
	}

	/*
	 * An ordinary character: store it in the word and get another.
	 */
	if (len < MAXWORDLEN-1)
	    word[len] = c;
	++len;

	c = getc(f);
    }

    /*
     * End of the word: check for errors.
     */
    if (c == EOF) {
	if (ferror(f)) {
	    if (errno == 0)
		errno = EIO;
	   
	}
	/*
	 * If len is zero, then we didn't find a word before the
	 * end of the file.
	 */
	if (len == 0)
	    return 0;
    }

    
    word[len] = 0;


    return 1;



}

#endif
#ifdef ENABLE_EVENTS	
extern void upnpevents_removeSubscriber_shutdown(void);
#endif

#ifndef __ECOS
int main(int argc, char * * argv)
#else
int miniigd_main(cyg_addrword_t data)
#endif
{
	int i;
	int pid;
	int shttpl;
#ifdef STAND_ALONE
    int sudp=-1, snotify=-1;
#endif
#if defined(ENABLE_NATPMP)
	int snatpmp = -1;
#endif
	LIST_HEAD(httplisthead, upnphttp) upnphttphead;
	struct upnphttp * e = 0;
	struct upnphttp * next;
	fd_set readset;	/* for select() */
	int notify_interval = 0;
	//int notify_interval = CHECK_WAN_INTERVAL;	/* seconds to check WAN status */
	struct timeval timeout={0,0}, timeofday={0,0}, lasttimeofday = {0, 0};
	int debug_flag = 0;
	int openlog_option;
	struct sigaction sa;
	int port = -1;
	int max_fd = -1;
	int enablenatpmp = 0;
#ifndef __ECOS
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) ||  defined(CONFIG_RTL8186_TR) /* Add host name in Upnp. Keith */	
	char host_name[100];
	FILE	*fp;
#endif
#endif
#ifdef ENABLE_EVENTS
	fd_set writeset;
#endif
#ifndef __ECOS
	struct sysinfo system_info;
#else
#endif
#ifndef __ECOS	
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_KB_N)
	FILE	*fp1;
	char tmpBuf[50];
	int tmpLen;
#endif
#endif
	char mac_addr[MAC_ADDR_LEN];
	char line[200];
	int n = 0;

	memset(location_url, 0x00, MAX_SERVER_LEN);
	memset(server_id, 0x00, MAX_SERVER_LEN);
	memset(ssdp_ext, 0x00, MAX_SERVER_LEN);
	ssdp_mx=0;
	isCfgFromFile=0;

	memset(internal_if, 0, MAX_LEN_IF_NAME);
#ifndef __ECOS
	/* command line arguments processing */
	for(i=1; i<argc; i++)
	{
		if(argv[i][0]!='-')
		{
			fprintf(stderr, "%s: Unknown option\n", argv[i]);
		}
		else switch(argv[i][1])
		{
		case 'e':
			wan_type = atoi(argv[++i]);
			break;
		case 'i':
			strcpy(internal_if, argv[++i]);
			break;
		case 'u':
			strncpy(uuidvalue+5, argv[++i], strlen(uuidvalue+5) + 1);
			break;
		case 'U':
			sysuptime = 1;
			break;
		case 'L':
			logpackets = 1;
			break;
		case 'P':
			pidfilename = argv[++i];
			break;
		case 'd':
			debug_flag = 1;
			break;
		case 'w':
			wisp_interface_num = atoi(argv[++i]);
			sprintf(wisp_if_name, "%s%d", WAN_IF_3, wisp_interface_num);
			break;
		case 'B':
			downstream_bitrate = strtoul(argv[++i], 0, 0);
			upstream_bitrate = strtoul(argv[++i], 0, 0);
			break;
		case 's':
			if(i<(argc-1))
			{
				sessionSel=atoi(argv[++i]);
#ifdef 	MULTI_PPPOE
				sessionNumber = sessionSel;
#endif
			}
			break;
#ifdef CONFIG_RTL8186_GR
		case 'c':
			if(i<(argc-1))
			{
				pppCutEnabled=atoi(argv[++i]);
			}
			break;
#endif
#if defined(ENABLE_NATPMP)
		case 'N':
			enablenatpmp = 1;
			break;
#endif

		default:
			fprintf(stderr, "%s: Unknown option\n", argv[i]);
		}
	}


	if (wan_type < 0) {
		printf("Please specify the WAN type\n");
		return 1;
	}
	
#else
#ifdef HAVE_SYSTEM_REINIT
	quitting = 0;
	wan_type = -1;
	wisp_interface_num = -1;
	wan_change_flag = 0;
	memset(wisp_if_name,0,16);
	memset(wan_if_name,0,16);
	memset(internal_if,0,MAX_LEN_IF_NAME);
#endif

	int op_mode,wlan_mode;
	apmib_get(MIB_WAN_DHCP,&wan_type);
	apmib_get(MIB_OP_MODE,&op_mode);
	if(op_mode == WISP_MODE){
		apmib_get(MIB_WISP_WAN_ID,&wisp_interface_num);
		apmib_get_ext(MIB_WLAN_MODE,&wlan_mode,wisp_interface_num,0);
		if(wlan_mode == CLIENT_MODE){
					sprintf(wisp_if_name,"wlan%d",wisp_interface_num);
					sprintf(wan_if_name,"wlan%d",wisp_interface_num);
				}
				else{
					sprintf(wisp_if_name,"wlan%d-vxd0",wisp_interface_num);
					sprintf(wan_if_name,"wlan%d-vxd",wisp_interface_num);
				}
		/*
		switch(wan_type){
			case PPPOE:
			case PPTP:
			case L2TP:
				sprintf(wisp_if_name,"%s",WAN_IF_2);
				break;
			case DHCP_CLIENT:
			default:
				if(wlan_mode == CLIENT_MODE){
					sprintf(wisp_if_name,"wlan%d",wisp_interface_num);
				}
				else{
					sprintf(wisp_if_name,"wlan%d-vxd0",wisp_interface_num);
				}
				break;
		}
		*/
	}
	else{
		sprintf(wan_if_name,"eth1");
	}
#endif
	/* record the startup time, for returning uptime */
	startup_time = time(NULL);


	//srand((unsigned)time(NULL));
	//port = 50000 + (rand() % 10000);
	port = PICS_INIT_PORT;
	
	if (!internal_if[0])
		strcpy(internal_if, "eth0");
	if(getifaddr(internal_if, listen_addr, INET_ADDRSTRLEN) < 0) {
			printf("miniigd : Failed to get internal_if IP. EXITING!\n");
			return 1;
	}
	

//To generate different uuid for different device according to /tmp/eth1_mac_addr
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_KB_N)
#ifndef __ECOS
	if ((fp1 = fopen("/tmp/eth1_mac_addr", "r")) != NULL) 
	{
		if(fgets(tmpBuf, 50, fp1) != NULL)
		{
			tmpLen=strlen(tmpBuf);
			tmpBuf[tmpLen-1]='\0';//To remove \n
			sprintf(uuidvalue,"uuid:82350000-4000-0000-0000-%s",tmpBuf);
		}
		fclose(fp1);
    	}
	system("rm -f /tmp/eth1_mac_addr");
#else
	get_mac_address("eth1",mac_addr);
	sprintf(uuidvalue,"uuid:82350000-4000-0000-0000-%s",mac_addr);
#endif

#endif

#ifndef __ECOS
	substr("/etc/linuxigd/","picsdesc.skl","/etc/linuxigd/","picsdesc.xml","!ADDR!", listen_addr);
#else
	substr(picsdesc_skl,picsdesc_xml,"!ADDR!",listen_addr);
#endif
#ifndef __ECOS

	if(read_config_file(DEFAULT_CONFIG_FILENAME, port) == 0)
		isCfgFromFile = 1;	

	if(isCfgFromFile){	
		system("cp /etc/linuxigd/picsdesc.xml /tmp/picsdesc.skl");
		substr("/tmp/","picsdesc.skl","/etc/linuxigd/", "picsdesc.xml","!UUID!", uuidvalue);
		system("rm /tmp/picsdesc.skl -rf");
	}
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) ||  defined(CONFIG_RTL8186_TR) /* Add host name in Upnp. Keith */
	memset(host_name, '\0', 100);
		
    if ((fp = fopen("/var/hostname", "r")) != NULL) 
	{
		fseek(fp,10L, SEEK_SET);
		getword(fp, host_name);
		fclose(fp);
    }


	if(host_name[0])
	{
		system("cp /etc/linuxigd/picsdesc.xml /tmp/picsdesc.skl");

		substr("/tmp/","picsdesc.skl","/etc/linuxigd/", "picsdesc.xml","!HOST_NAME!", host_name);
		system("rm /tmp/picsdesc.skl -rf");
	}
#endif
#endif
	DISPLAY_BANNER;
#ifndef __ECOS
	if(debug_flag)
		pid = getpid();
	else
		pid = daemonize();
#endif
	//Patch: waiting for br0 entering forwarding state, then send out ssdp notify pkts
	sleep(3);
	sleep(3);
#ifndef __ECOS
	/* TODO : change LOG_LOCAL0 to LOG_DAEMON */
	if(debug_flag) {
		openlog_option = LOG_PID|LOG_CONS;
		openlog_option |= LOG_PERROR;	/* also log on stderr */
		openlog("miniigd", openlog_option, LOG_USER/*LOG_LOCAL0*/);
	}

	writepidfile(pidfilename, pid);
#endif	
#if defined(ENABLE_NATPMP)

	if(enablenatpmp)

	{
		snatpmp = OpenAndConfNATPMPSocket();
		if(snatpmp < 0)
		{
			syslog(LOG_ERR, "Failed to open socket for NAT PMP.");
			/*syslog(LOG_ERR, "Failed to open socket for NAT PMP. EXITING");
			return 1;*/
		} else {
			syslog(LOG_NOTICE, "Listening for NAT-PMP traffic on port %u",
				   NATPMP_PORT);
		}
		ScanNATPMPforExpiration();
	}
#endif

	
	/* socket d'ecoute des connections HTTP */
	shttpl = MiniigdOpenAndConfHTTPSocket(listen_addr, port);
	if(shttpl < 0)
	{
		printf("miniigd : Failed to open socket for HTTP. EXITING!\n");
		return 1;
	}
#ifdef STAND_ALONE
   	/* socket d'ecoute pour le SSDP */
   	sudp = MiniigdOpenAndConfUdpSocket(listen_addr);
   	if (sudp < 0)
   	{
   		printf("Failed to open socket for SSDP. EXITING\n");
		close(shttpl);
   		return -1;
   	}
   	/* open socket for sending notifications */
   	snotify = MiniigdOpenAndConfNotifySocket(listen_addr);
   	if (snotify < 0)
   	{
		close(shttpl);
   		close(sudp);
   		printf("Failed to open socket for SSDP notify messages\n");
   		return -1;
   	}
#endif

#ifdef SUPPORT_HNAP
#if 0
	int  unix_http;
	unix_http = OpenAndConfUNIXSocket();
	if (unix_http < 0) {
		printf("miniigd : Failed to open socket for Unix Socket. EXITING!\n");
		return 1;
	}
#endif
	signal(SIGALRM, sigHandler_alarm);
	alarm(1);
#endif
#ifndef __ECOS
	/* set signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = sigterm;
	if (sigaction(SIGTERM, &sa, NULL))
	{
		printf("miniigd : Failed to set SIGTERM handler\n");
		return 1;
	}
	if (sigaction(SIGINT, &sa, NULL))
	{
		printf("miniigd : Failed to set SIGTERM handler\n");
		return 1;
	}
#endif	
	strcpy(ext_if_name, "lo");
	strcpy(ext_ip_addr, "127.0.0.1");
	//AddIPtables(1, ext_if_name, ext_ip_addr);
	//init_redirect();
	CheckWanStatus();
#if 0
	if (miniupnp_WriteConfigFile(port) < 0)
		return 1;
#endif
	LIST_INIT(&upnphttphead);

#ifdef HAVE_SYSTEM_REINIT
	load_mappings();
#endif
	while (!quitting)
	{
#ifndef __ECOS
			sysinfo(&system_info);
			timeofday.tv_sec = system_info.uptime;
#else
			time(&timeofday.tv_sec);
#endif
			if(timeofday.tv_sec >= (lasttimeofday.tv_sec + CHECK_WAN_INTERVAL))
			{
				CheckWanStatus();
				//diag_printf("ext_if_name:%s,ext_ip_addr:%s\n",ext_if_name,ext_ip_addr);
				memcpy(&lasttimeofday, &timeofday, sizeof(struct timeval));
				timeout.tv_sec = CHECK_WAN_INTERVAL;
				timeout.tv_usec = 0;
#ifdef STAND_ALONE
				if( snotify >= 0 && notify_interval==0 )
				    MiniigdSendSSDPNotifies(snotify, listen_addr, 0);
#endif
				if(notify_interval>0)
					notify_interval--;
				else
					notify_interval=(ssdp_mx)?(ssdp_mx/CHECK_WAN_INTERVAL-1):(DEFAULT_ADVR_EXPIRE/CHECK_WAN_INTERVAL-1);
			}
			else
			{
				//timeout.tv_sec = lasttimeofday.tv_sec + notify_interval - timeofday.tv_sec;
				timeout.tv_sec = lasttimeofday.tv_sec + CHECK_WAN_INTERVAL - timeofday.tv_sec;
				if(timeofday.tv_usec > lasttimeofday.tv_usec)
				{
					timeout.tv_usec = 1000000 + lasttimeofday.tv_usec - timeofday.tv_usec;
					timeout.tv_sec--;
				}
				else
				{
					timeout.tv_usec = lasttimeofday.tv_usec - timeofday.tv_usec;
				}
			}
			if(wan_change_flag){
				CheckWanStatus();
				clear_and_reset();
				wan_change_flag = 0;
			}
//		}
#if defined(ENABLE_NATPMP)
		/* Remove expired NAT-PMP mappings */

		if(enablenatpmp)

		{
			while( nextnatpmptoclean_timestamp && (timeofday.tv_sec >= nextnatpmptoclean_timestamp + startup_time))
			{
				//syslog(LOG_DEBUG, "cleaning expired NAT-PMP mappings");
				if(CleanExpiredNATPMP() < 0) {
					syslog(LOG_ERR, "CleanExpiredNATPMP() failed");
					break;
				}
			}
			if(nextnatpmptoclean_timestamp && timeout.tv_sec >= (nextnatpmptoclean_timestamp + startup_time - timeofday.tv_sec))
			{
				/*syslog(LOG_DEBUG, "setting timeout to %d sec", nextnatpmptoclean_timestamp + startup_time - timeofday.tv_sec);*/
				timeout.tv_sec = nextnatpmptoclean_timestamp + startup_time - timeofday.tv_sec;
				timeout.tv_usec = 0;
			}
		}
#endif
			
		/* select open sockets (SSDP, HTTP listen and all HTTP soap sockets) */
		FD_ZERO(&readset);
		//FD_SET(shttpl, &readset); //brad disable
		if (shttpl >= 0){
		    FD_SET(shttpl, &readset);
			max_fd = MAX(max_fd, shttpl);
		}		

		i = 0;        // active HTTP connections count
        for(e = upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
        {
	        if((e->socket >= 0) && (e->state <= 2))
	        {
	            FD_SET(e->socket, &readset);
	            max_fd = MAX( max_fd, e->socket);
	            i++;
	        }
        }
		if(i>1)
        {
#ifndef __ECOS
			sprintf(line,"echo \"\%d active incoming HTTP connections\" >> /tmp/miniigd_debug", i);
			system(line);
#endif
			syslog(LOG_WARNING, "%d active incoming HTTP connections", i);
		}
#if defined(ENABLE_NATPMP)
		if(snatpmp >= 0) {
			FD_SET(snatpmp, &readset);
			max_fd = MAX( max_fd, snatpmp);
		}
#endif
#ifdef STAND_ALONE
        if(sudp >= 0)
		{
            FD_SET(sudp, &readset);
			max_fd = MAX( max_fd, sudp);
		}
#endif
		clear_expired_redirect_rule();

#if 0
//#ifdef SUPPORT_HNAP
		//FD_SET(unix_http, &readset);
#endif

		/*//i = 0;	// active HTTP connections count
		for(e = upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
		{
			if((e->socket >= 0)&&(e->state <= 2)){
				FD_SET(e->socket, &readset);
				max_fd = MAX( max_fd, e->socket);
				//i++;
			}
		}
		*/	//by brian, execute this block later (debug fd contension while UPnP handling busily)
		/* for debug */
		/*
		if(i>1)
		{
			syslog(LOG_WARNING, "%d active incoming HTTP connections", i);
		}
		*/
#ifdef ENABLE_EVENTS
		FD_ZERO(&writeset);
		
		upnpevents_selectfds(&readset, &writeset, &max_fd);
#endif
#ifdef ENABLE_EVENTS
		if( (n=select(max_fd+1, &readset, &writeset, 0, &timeout)) < 0)
#else
		if((n =select(max_fd+1, &readset, 0, 0, &timeout)) < 0)
#endif	
		{
			if(quitting) goto shutdown;
			syslog(LOG_ERR, "select: %m");
			syslog(LOG_ERR, "Exiting...");
			//exit(1);	/* very serious cas of error */
		}
		if(n == 0) continue;
#if 0
//#ifdef SUPPORT_HNAP
		if(FD_ISSET(unix_http, &readset)) {
			int s2, t;
			struct sockaddr_un remote;
			struct upnphttp * tmp = 0;

			t = sizeof(remote);
			if ((s2 = accept(unix_http, (struct sockaddr *)&remote, &t)) == -1) {
                		syslog(LOG_ERR, "accept: %m");
            		}
			else {
				tmp = New_upnphttp(s2);
				LIST_INSERT_HEAD(&upnphttphead, tmp, entries);
			}
		}
#endif
#ifdef ENABLE_EVENTS
				upnpevents_processfds(&readset, &writeset);

#endif
		/* process active HTTP connections */
		/* LIST_FOREACH is not available under linux */
		for(e = upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
		{
			if((e->socket >= 0)&&(e->state <= 2)){
				if(FD_ISSET(e->socket, &readset))
					MiniigdProcess_upnphttp(e);					
			}
		}

		/* process incoming HTTP connections */
		if(FD_ISSET(shttpl, &readset))
		{
			int shttp;
			socklen_t	clientnamelen;
			struct sockaddr_in clientname;
			struct upnphttp * tmp = 0;
			clientnamelen = sizeof(struct sockaddr_in);
			shttp = accept(shttpl, (struct sockaddr *)&clientname, &clientnamelen);
			if(shttp<0)
			{
				syslog(LOG_ERR, "accept: %m");
			}
			else
			{
				
				//syslog(LOG_INFO, "HTTP connection from %s:%d",
				       //inet_ntoa(clientname.sin_addr),
				   	   //ntohs(clientname.sin_port) );
					  
				//if (fcntl(shttp, F_SETFL, O_NONBLOCK) < 0) {
					//syslog(LOG_ERR, "fcntl F_SETFL, O_NONBLOCK");
				//}
				/* Create a new upnphttp object and add it to
				 * the active upnphttp object list */
#ifdef	ECOS_DBG_STAT
				dbg_stat_add(dbg_igd_index1, DBG_TYPE_SOCKET,DBG_ACTION_ADD, 0);
#endif
				tmp = MiniigdNew_upnphttp(shttp);
#if 0
				/*accpeted socket keep listening socket options flag except SO_ACCEPTCONN*/
#if defined(CONFIG_RTL_819X) && defined(CONFIG_RTL_SORAPIDRECYCLE)
				int val = 1;
				if(setsockopt(shttp, SOL_SOCKET, SO_RAPIDRECYCLE, &val, sizeof(val)) < 0)
					diag_printf("set recycle fail...\n");
#endif
#endif
				if(tmp)
					LIST_INSERT_HEAD(&upnphttphead, tmp, entries);
				else{
					if(close(shttp)==0){
#ifdef	ECOS_DBG_STAT
					dbg_stat_add(dbg_igd_index1, DBG_TYPE_SOCKET,DBG_ACTION_DEL, 0);
#endif
					
				}else
					diag_printf("miniigd can't close http accept socket!\n");
				}
			}
		}

#if defined(ENABLE_NATPMP)
		/* process NAT-PMP packets */
		if((snatpmp >= 0) && FD_ISSET(snatpmp, &readset))
		{
			ProcessIncomingNATPMPPacket(snatpmp);
		}

#endif
#ifdef STAND_ALONE
        if(sudp >= 0 && FD_ISSET(sudp, &readset))
		{
			/*syslog(LOG_INFO, "Received UDP Packet");*/
			MiniigdProcessSSDPRequest(sudp, listen_addr, port);
		}
#endif
		/* delete finished HTTP connections */
		for(e = upnphttphead.lh_first; e != NULL; )
		{
			next = e->entries.le_next;
			if(e->state >= 100)
			{
				LIST_REMOVE(e, entries);
				MiniigdDelete_upnphttp(e);
			}
			e = next;
		}

	}
	
shutdown:
	/* close out open sockets */
	while(upnphttphead.lh_first != NULL)
	{
		e = upnphttphead.lh_first;
		LIST_REMOVE(e, entries);
		MiniigdDelete_upnphttp(e);
	}
#if defined(ENABLE_NATPMP)
	if(snatpmp>=0)
	{
		close(snatpmp);
		snatpmp = -1;
	}
#endif
#ifdef STAND_ALONE
        if(sudp>=0)
		close(sudp);
#endif
        if(shttpl)
	close(shttpl);
	if(snotify)
		close(snotify);
#ifdef ENABLE_EVENTS		
	upnpevents_removeSubscriber_shutdown();
#endif
	miniupnpdShutdown();

#ifdef HAVE_SYSTEM_REINIT
	save_mappings();
	clear_mappings();
	quitting = 0;
	is_miniigd_runnig = 0;
#endif
	return 0;
}
void create_miniigd(void)
{
#ifdef HAVE_SYSTEM_REINIT
	int cnt = 0;
#endif
	if(!is_miniigd_runnig
#ifdef HAVE_SYSTEM_REINIT
		|| quitting
#endif
	 )
	{
#ifdef HAVE_SYSTEM_REINIT
		while(quitting){
			if(cnt++ >5){
				diag_printf("miniigd error:[%s] wait cleanup failed\n",__FUNCTION__);
				return 0;
			}
			sleep(1);
		}
		if(is_miniigd_runnig){
			miniigd_exit();
		}
#endif
		is_miniigd_runnig = 1;
#ifdef	ECOS_DBG_STAT
		dbg_igd_index=dbg_stat_register("miniigd");
		dbg_igd_index1=dbg_stat_register("miniigd-http");
		dbg_igd_index2=dbg_stat_register("miniigd-event");

#endif
		/* Create the thread */
		cyg_thread_create(CYGNUM_MINIIGD_THREAD_PRIORITY,
			      miniigd_main,
			      0,
			      "miniigd",
			      &miniigd_stack,
			      sizeof(miniigd_stack),
			      &miniigd_thread_handle,
			      &miniigd_thread_obj);
		/* Let the thread run when the scheduler starts */
		cyg_thread_resume(miniigd_thread_handle);
	}
	else{
		wan_change_flag = 1;
	}
	return;
}
#ifdef HAVE_SYSTEM_REINIT
void kill_miniigd()
{	
	if(is_miniigd_runnig){
		diag_printf("kill_miniigd\n");
		quitting = 1;
		/*Waiting miniigd thread shutdown and cleanup resource*/
		while(quitting){
			cyg_thread_delay(10);
		}
		miniigd_exit();
		is_miniigd_runnig = 0;
	}
}
void miniigd_exit()
{
	cyg_thread_kill(miniigd_thread_handle);
	cyg_thread_delete(miniigd_thread_handle);
}
#endif
