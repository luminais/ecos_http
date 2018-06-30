/*
 *      arping.c - Ping hosts by ARP requests/replies to do duplicate
 *                 address detection
 *
 *      Authors: Hung-Jen Kao	<kaohj@realtek.com.tw>
 *
 *
 */
#include "../../../../../ecos-3.0/packages/devs/eth/rltk/819x/wrapper/v3_0/include/skbuff.h"
#include <sys/ioctl.h>
#define USE_OLD_PING 0

#if !USE_OLD_PING
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/ethernet.h>

#ifndef NULL_FILE
#define NULL_FILE 0
#endif
#ifndef NULL_STR
#define NULL_STR ""
#endif
struct hostdesc
{
	char *hostname;
	struct in_addr hostaddr;
	struct hostdesc *next;
};
extern struct hostdesc *hostnames;
extern struct hostdesc *hosttail;
extern u_short in_cksum(u_short *addr, int len);
unsigned short cwmp_id=0x111;
unsigned short cwmp_seq=0;

struct timeval sendTime;
struct timeval recvTime;
char *dataBuffer;
unsigned int dataSize;
unsigned int uOK = 0, uFail = 0;
#endif

#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//by cairui
#if 0
#if defined(CONFIG_RTL_8196C)
#include "./../../../../96c-92c-gw_install/include/sys/socket.h"
#include "./../../../../96c-92c-gw_install/include/arpa/inet.h"
#include "./../../../../96c-92c-gw_install/include/net/if.h"
#endif
#if defined(CONFIG_RTL_8881AB)
#include "./../../../../8881ab-8367rb-92e-gw_install/include/sys/socket.h"
#include "./../../../../8881ab-8367rb-92e-gw_install/include/arpa/inet.h"
#include "./../../../../8881ab-8367rb-92e-gw_install/include/net/if.h"
#endif

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif

#include <netdb.h>
#include <netinet/ip_icmp.h>
#include "debug.h"

struct in_addr src;
struct in_addr dst;
//struct sockaddr_ll me;
//struct sockaddr_ll he;
struct timeval last;

#define MAX_COUNT	3
#define PINGINTERVAL	1	/* second */
#define MAXWAIT		3

static int count;
static int finished;
static int s;

#if !USE_OLD_PING
int cwmp_initpacket(char *buf, int querytype, struct in_addr fromaddr)
{
   struct ip *ip = (struct ip *)buf;
   struct icmp *icmp = (struct icmp *)(ip + 1);

   /* things we customize */
   int icmplen = 0;

   ip->ip_src = fromaddr;	/* if 0,  have kernel fill in */
   ip->ip_v = 4;		/* Always use ipv4 for now */
   ip->ip_hl = sizeof *ip >> 2;
   ip->ip_tos = 0;
   ip->ip_id = htons(4321);
   ip->ip_ttl = 255;
   ip->ip_p = 1;
   ip->ip_sum = 0;                 /* kernel fills in */

   cwmp_seq++;
   icmp->icmp_seq = cwmp_seq;
   icmp->icmp_cksum = 0;
   icmp->icmp_type = querytype;
   icmp->icmp_code = 0;

   switch(querytype) {
   case ICMP_TSTAMP:
	   gettimeofday( (struct timeval *)(icmp+8), NULL);
	   bzero( icmp+12, 8);
	   icmplen = 20;
	   break;
   case ICMP_MASKREQ:
	   *((char *)(icmp+8)) = 255;
	   icmplen = 12;
	   break;
	   
	case ICMP_ECHO:
	   icmp->icmp_id = cwmp_id;
	   memcpy(icmp->icmp_data, dataBuffer, dataSize);
	   icmplen = 40 + dataSize;
	   break;
	   
   default:
	   fprintf(stderr, "eek: unknown query type\n");
	   return(0);
   }
   ip->ip_len = sizeof(struct ip) + icmplen;
   //printf("ip->ip_len=%d\n", ip->ip_len);
   return icmplen;
}

int cwmp_makehosts(char *hostlist)
{
	//int i;
	struct in_addr tmpaddr;
	int hostcount = 0;

	//	for (i = 0; hostlist[i]; i++) {
#ifdef DEBUG
	if(hostlist)
		diga_printf("Resolving %s\n", hostlist);
#endif
	if(!Valid_Ipv4_Addr(hostlist))
	{
		struct hostent *host;
		struct in_addr serverip;
		int ret;			
		host = gethostbyname(hostlist);
		if(host !=NULL)
		{
			if( host->h_addrtype != AF_INET) {
				diag_printf("Error address\n");
			}		
			else
				memcpy(&tmpaddr.s_addr, host->h_addr, sizeof(tmpaddr.s_addr));
		}
	}
	else		
		tmpaddr.s_addr = inet_addr(hostlist);

	/* The host has been resolved.  Put it in the chain */
	/* We want to stick it on the end. */
	if (hostnames == NULL) {
		hostnames = (struct hostdesc *)malloc(sizeof(*hostnames));
		if (hostnames == NULL) {
			perror("hostnames malloc failed");
			return(-1);
		}
		hosttail = hostnames;
	} else {
		hosttail->next = (struct hostdesc *)malloc(sizeof(*hostnames));
		if (hosttail->next == NULL) {
			perror("hosttail->next malloc failed");
			return(-1);
		}
		hosttail = hosttail->next;
	}
	hosttail->hostname = strdup(hostlist);
	if (hosttail->hostname == NULL) {
		perror("strdup failed");
		return(-1);
	}
	hosttail->hostaddr = tmpaddr;
	hosttail->next = NULL;
	hostcount++;
	//}
	return hostcount;
}


int cwmp_sendpings(int s, int querytype, struct hostdesc *head, int delay,
		struct in_addr fromaddr)
{
	char buf[1500];
	struct ip *ip = (struct ip *)buf;
	struct icmp *icmp = (struct icmp *)(ip + 1);
	struct sockaddr_in dst;
	int icmplen;

	bzero(buf, 1500);
	icmplen = cwmp_initpacket(buf, querytype, fromaddr);
	dst.sin_family = AF_INET;

	while (head != NULL) {
		//printf("==============send pings================\n");
#ifdef DEBUG
		if(head->hostname)
			diag_printf("pinging %s\n", head->hostname);
#endif
		ip->ip_dst.s_addr = head->hostaddr.s_addr;
		dst.sin_addr = head->hostaddr;
		icmp->icmp_cksum = 0;
		icmp->icmp_cksum = in_cksum((u_short *)icmp, icmplen);
		if (sendto(s, buf, ip->ip_len, 0,
					(struct sockaddr *)&dst,
					sizeof(dst)) < 0) {
			perror("sendto");
			return -1;
		}

		gettimeofday(&sendTime, NULL);

		if (delay)
			sleep(delay);
		/* Don't flood the pipeline..kind of arbitrary */
		head = head->next;
	}
	return 0;
}

/*
 * Listen for 'hostcount' pings, print out the information, and
 * then exit.
 */
void cwmp_recvpings(int s, int querytype, struct hostdesc *head, int hostcount,
		int broadcast)
{
	char buf[1500];
	struct ip *ip = (struct ip *)buf;
	struct icmp *icmp;
	//	int err = 0;
	int fromlen = 0;
	int hlen;
	struct timeval tv;
	struct tm *tmtime;
	int recvd = 0;
	char *hostto;
	char hostbuf[128], timebuf[128];
	struct hostdesc *foundhost;
	unsigned long int icmptime, icmpmask;

	gettimeofday(&tv, NULL);

	while (1) {
		if ((fromlen = recvfrom(s, buf, sizeof buf, 0, NULL,
						NULL)) < 0)
		{
			perror("icmp request");
			break;
		}
		gettimeofday(&recvTime, NULL);

		hlen = ip->ip_hl << 2;
		icmp = (struct icmp *)(buf + hlen);

		/* Find the host */
		hostto = 0;
		for (foundhost = head; foundhost != NULL;
				foundhost = foundhost->next) {
			if (foundhost->hostaddr.s_addr == ip->ip_src.s_addr) {
				hostto = foundhost->hostname;
				break;
			}
		}

		if (!hostto) {
			sprintf(hostbuf, "unknown (%s)",
					inet_ntoa(ip->ip_src));
			hostto = hostbuf;
		}

		/* For time */
		switch(icmp->icmp_type) {
			case ICMP_TSTAMPREPLY:
				icmptime = ntohl(icmp->icmp_ttime);
				/* ms since midnight. yuch. */
				tv.tv_sec -= tv.tv_sec%(24*60*60);
				tv.tv_sec += (icmptime/1000);
				tv.tv_usec = (icmptime%1000);
				tmtime = localtime(&tv.tv_sec);
				strftime(timebuf, 128, "%H:%M:%S", tmtime);
				//printf("%-40.40s:  %s\n", hostto, timebuf);
				break;

			case ICMP_MASKREPLY:
				icmpmask = ntohl(icmp->icmp_dun.id_mask);
				//printf("%-40.40s:  0x%lX\n", hostto, icmpmask);
				break;

			case ICMP_ECHOREPLY:
				if(cwmp_id!= ntohl(icmp->icmp_id))
					break;
				
				uOK++;
				diag_printf("%d bytes from %s: icmp_seq=%u", fromlen+ETHER_HDR_LEN,
						inet_ntoa(ip->ip_src),icmp->icmp_seq);
				//printf(" ttl=%u\n", ip->ip_ttl);
				return;

			default:
				//printf("Unknown ICMP message received (type %d)\n",icmp->icmp_type);
				break;
		}
		if (!broadcast)
			recvd++;
	}

}
#endif

#define MS_TDIFF(tv1,tv2) ( ((tv1).tv_sec-(tv2).tv_sec)*1000 + \
		((tv1).tv_usec-(tv2).tv_usec)/1000 )


#define ERR_RESOURCE -1 
#define ERR_UNREACHABLE -2

//ICMP echo request test

/*
   INPUT
intf: name of the outgoing interface
host: name of the destination host.
count: number of request to test.
timeout: time in mil sec to wait
datasize: size of the payload
tos: specify the IP ToS
OUTPUT
cntOK: number of successful PING
cntFail: number of failed PING
timeAvg: average response time
timeMin: minimum response time
timeMax: maximum response time
RETURN
0: success
-1: resource error
-2: host unreachable     

 **/
int icmp_test(char *intf, char *host, unsigned int count, unsigned int timeout, unsigned int datasize, unsigned char tos,
		unsigned int *cntOK, unsigned int *cntFail, unsigned int *timeAvg, unsigned int *timeMin, unsigned int *timeMax, unsigned int needWaitRsp) 
{
	//printf("enter icmp_test\n");
#if USE_OLD_PING
	//struct protoent *proto;
	struct sockaddr_in sockaddr;
	//struct hostent *h;
	struct icmp *icmppkt;
	struct timeval tv;

	unsigned char *buffer;
	fd_set rset;	
	int sock, bufsize, ret, pingid, cnt;
	//unsigned int attempt;
	int int_op;
#endif
	unsigned short uSequence = 0, uExpected;
	unsigned int tAvg = 0, tMin = timeout, tMax = 0;
	uOK=0;
	uFail=0;
	int durationMS; 

	//printf("icmp_test: count=%d\n", count);
	int s;
	char *progname;
	int on = 1;
	int hostcount;
	int delay = 0;
	int querytype = ICMP_ECHO;
	struct in_addr fromaddr;
	int broadcast = 0; /* Should we wait for all responses? */
	int ret;
	int i;
	fromaddr.s_addr = 0;
	struct timeval tv;
	struct hostdesc *head;
	int int_op;

	progname = host;
	dataSize = datasize;
	dataBuffer = (char *)malloc(datasize);
	if(NULL == dataBuffer) return -1;
	memset(dataBuffer, '\0', datasize);

	//printf("icmp_test: 1\n");
	if (!host || !strlen(host)) 
	{
		diag_printf("%s targets\n",progname);
		return(-1);
	}

	//printf("icmp_test: 2\n");
	hostcount = cwmp_makehosts(host);
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		perror("socket");
		return(1);
	}
	if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		perror("IP_HDRINCL");
		close(s);
		return(1);
	}
        int_op = tos;
        if ((ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&int_op, sizeof (int_op))) < 0) {
                //printf("set QoS %d returns %d\n", int_op, ret);
        }

	/*set timeout for sendto() and recvfrom()*/
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;	
	if(setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))<0){
		perror("SO_SNDTIMEO");
		close(s);
		return(1);
	}

	if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0){
		perror("SO_RCVTIMEO");
		close(s);		
		return(1);		
	}

	//printf("icmp_test: 3, count=%d\n", count);
	for(i=0;i<count;i++){
		//printf("icmp_test: 4, i=%d, timeout=%d s\n", i, timeout/1000);
		sleep(timeout/1000);
		ret=cwmp_sendpings(s, querytype, hostnames, delay, fromaddr);
		//printf("icmp_test: 4, ret=%d\n", ret);
		if(!ret)
			cwmp_recvpings(s, querytype, hostnames, hostcount, broadcast);
		durationMS = MS_TDIFF(recvTime, sendTime);
		//printf("\tduring time: %ld micro secs\n", durationMS);

		// deal with min/max/ave
		if((durationMS<tMin)) tMin=durationMS;

		if((durationMS>tMax)&&(durationMS<=timeout)) {
			tMax=durationMS;
		}else if(durationMS>timeout)
			durationMS = 0;

		// firstly, we get the sum, and then Avg=sum/uOK
		tAvg += durationMS;
	}
	
	uFail = count - uOK;
	tAvg = tAvg/uOK;
	//printf("===============\n");
	//printf("uOK=%d, uFail=%d, tmax=%d, tmin=%d, tAvg=%d\n", uOK, uFail, tMax, tMin, tAvg);
	//printf("===============\n");
	*timeMax = tMax;
	*timeMin = tMin;
	*timeAvg = tAvg;
	*cntOK = uOK;
	*cntFail = uFail;
	
	//printf("icmp_test: 5\n");
	while(hostnames){
		head=hostnames;
		hostnames=hostnames->next;
		if(head->hostname)
			free(head->hostname);
		free(head);
	}
	//printf("icmp_test: 6\n");
	//} //end while
	close(s);
	free(dataBuffer);
	return(0);
} //end icmp_test

