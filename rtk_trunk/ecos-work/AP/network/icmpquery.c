/*
 * icmpquery.c - send and receive ICMP queries for address mask
 *               and current time.
 *
 * Version 1.0.3
 *
 * Copyright 1998, 1999, 2000  David G. Andersen <angio@pobox.com>
 *                                        <danderse@cs.utah.edu>
 *                                        http://www.angio.net/
 *
 * All rights reserved.
 * This information is subject to change without notice and does not
 * represent a commitment on the part of David G. Andersen.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of David G. Andersen may not
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL DAVID G. ANDERSEN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *
 * Verified to work on:
 *    FreeBSD (2.x, 3.x)
 *    Linux 2.0.x, 2.2.0-pre1
 *    NetBSD 1.3
 *
 * Should work on Solaris and other platforms with BSD-ish stacks.
 *
 * If you compile it somewhere else, or it doesn't work somewhere,
 * please let me know.
 *
 * Compilation:  gcc icmpquery.c -o icmpquery
 *
 * One usage note:  In order to receive accurate time information,
 *                  the time on your computer must be correct; the
 *                  ICMP timestamp reply is a relative time figure.
 */


/* Some portions of this code are taken from FreeBSD's ping source.
 * Those portions are subject to the BSD copyright, which is appended
 * at the end of this file.  Namely, in_cksum.
 */

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

u_short in_cksum(u_short *addr, int len);

/*
 * We perform lookups on the hosts, and then store them in a chain
 * here.
 */

struct hostdesc {
	char *hostname;
	struct in_addr hostaddr;
	struct hostdesc *next;
};

struct hostdesc *hostnames;
struct hostdesc *hosttail;
unsigned short id=0x111;
unsigned short seq=0;


int recv_pack_num;
int send_pack_num;


void resolv_from(char *hostfrom, struct in_addr *fromaddr)
{
	struct hostent *hp;
	if (hostfrom == NULL) {
		fromaddr->s_addr = 0;
		return;
	}
	
	if ((hp = gethostbyname(hostfrom)) == NULL) {
		if ((fromaddr->s_addr = inet_addr(hostfrom)) == -1) {
			fprintf(stderr, "could not resolve from address\n");
			return;
		}
	} else {
		bcopy(hp->h_addr_list[0], &fromaddr->s_addr, hp->h_length);
	}
}

/*
	check ip is valid ipv4 address!!!
	return value.
	0: invalid
	1: valid
	add by dzh.
*/
int Valid_Ipv4_Addr(char *str)
{
        int i=0,count = 0;;		
        for ( ; i < strlen(str) ; i++ ) {
                if ( !(str[i] >= '0' && str[i] <= '9' )) {
                        if ( str[i] == '.' ) {
                                count++;
                                continue;
                        }                  
						return 0;	
                }
        }		
		if( 3 == count)
			return 1;
}

/*
 * Set up the list of hosts.  Return the count.
 */

int makehosts(char **hostlist)
{
	int i;
//	struct hostent *hp;
	struct in_addr tmpaddr;
	int hostcount = 0;
	
	for (i = 0; hostlist[i]; i++) {
#ifdef DEBUG
		if(hostlist[i])
			printf("Resolving %s\n", hostlist[i]);
#endif
		if(!Valid_Ipv4_Addr(hostlist[i]))
		{
			struct hostent *host;
			struct in_addr serverip;
			int ret;			
			host = gethostbyname(hostlist[i]);
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
			tmpaddr.s_addr = inet_addr(hostlist[i]);

		/* The host has been resolved.  Put it in the chain */
		/* We want to stick it on the end. */
		if (hostnames == NULL) {
			hostnames = (struct hostdesc *)
				malloc(sizeof(*hostnames));
			if (hostnames == NULL) {
				perror("hostnames malloc failed");
				return(-1);
			}
			hosttail = hostnames;
		} else {
			hosttail->next = (struct hostdesc *)
				malloc(sizeof(*hostnames));
			if (hosttail->next == NULL) {
				perror("hosttail->next malloc failed");
				return(-1);
			}
			hosttail = hosttail->next;
		}
		hosttail->hostname = strdup(hostlist[i]);
		if (hosttail->hostname == NULL) {
			perror("strdup failed");
			return(-1);
		}
		hosttail->hostaddr = tmpaddr;
		hosttail->next = NULL;
		hostcount++;
	}
	return hostcount;
}

static void usage(char *prog)
{
   fprintf(stderr,"%s  targets\n",prog);
}

/*
 * Set up a packet.  Returns the length of the ICMP portion.
 */

int initpacket(char *buf, int querytype, struct in_addr fromaddr)
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

	seq++;
   icmp->icmp_seq = seq;
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
	   icmp->icmp_id = id;
	   icmplen = 40;
	   break;
	   
   default:
	   fprintf(stderr, "eek: unknown query type\n");
	   return(0);
   }
   ip->ip_len = sizeof(struct ip) + icmplen;
   return icmplen;
}
   
int sendpings(int s, int querytype, struct hostdesc *head, int delay,
	       struct in_addr fromaddr)
     
{
	char buf[1500];
	struct ip *ip = (struct ip *)buf;
	struct icmp *icmp = (struct icmp *)(ip + 1);
	struct sockaddr_in dst;
	int icmplen;


	++send_pack_num;


	bzero(buf, 1500);
	icmplen = initpacket(buf, querytype, fromaddr);
	dst.sin_family = AF_INET;

	while (head != NULL) {
#ifdef DEBUG
		if(head->hostname)
			printf("pinging %s\n", head->hostname);
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

void recvpings(int s, int querytype, struct hostdesc *head, int hostcount,
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
			printf("%-40.40s:  %s\n", hostto, timebuf);
			break;

		case ICMP_MASKREPLY:
			icmpmask = ntohl(icmp->icmp_dun.id_mask);
			printf("%-40.40s:  0x%lX\n", hostto, icmpmask);
			break;

		case ICMP_ECHOREPLY:
			 if(id!= icmp->icmp_id)
			 	break;
			printf("%d bytes from %s: icmp_seq=%u", fromlen+ETHER_HDR_LEN,
			   inet_ntoa(ip->ip_src),icmp->icmp_seq);
			printf(" ttl=%u\n", ip->ip_ttl);
			// +++++
			++recv_pack_num;
			// +++++
			return;
			
		default:
			//printf("Unknown ICMP message received (type %d)\n",icmp->icmp_type);
			break;
		}
		if (!broadcast)
			recvd++;
	}
	
}


static void ping_statistics(char *ping_hostname)
{
	float precision = 0.0;
	if(send_pack_num)
		precision = (float)(100 * (send_pack_num - recv_pack_num) / send_pack_num);
	printf("--- %s ping statistics ---\n", ping_hostname);
	printf("%d packets transmitted, %d received, %.2f%% packet loss.",  
		send_pack_num, 
		recv_pack_num, 
		precision);
}


int
icmp_main(int argc, char **argv)
{
   int s;

   char *progname;
//   char *optarg;         /* getopt variable declarations */
//   char *hostfrom = NULL;
//    int optind;
//   int optopt;
//    int opterr;
//   char ch;                     /* Holds the getopt result */
   int on = 1;
   int hostcount;
   int delay = 0;
   int querytype = ICMP_ECHO;
   struct in_addr fromaddr;
   int timeout = 1;  /* Default to 1 seconds */
   int broadcast = 0; /* Should we wait for all responses? */
   int ret;
   int i;
   fromaddr.s_addr = 0;
   struct timeval tv;
   struct hostdesc *head;

  seq = 0;
  recv_pack_num = 0;
  send_pack_num = 0;

   progname = argv[0];
#if 0
   while ((ch = getopt(argc, argv, "Btmf:d:T:")) != EOF) 
      switch(ch)
      {
      case 'B':
	      broadcast = 1;
	      break;
      case 'd':
	      delay = (int) strtol(optarg, NULL, 10);
	      break;
      case 't': /* timestamp request */
	      querytype = ICMP_TSTAMP;
	      break;
      case 'm': /* address mask request */
	      querytype = ICMP_MASKREQ;
	      break;
      case 'f':
	      hostfrom = optarg;
	      resolv_from(hostfrom, &fromaddr);
	      break;
      case 'T':
	      timeout = (int) strtol(optarg, NULL, 10);
	      break;
      default:
	      usage(progname);
	      return(-1);
      }
   argc -= optind;
   argv += optind;
#endif
   if (!argv[0] || !strlen(argv[0])) 
   {
      usage(progname);
      return(-1);
   }

   hostcount = makehosts(argv);
   if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
      perror("socket");
      return(1);
   }
   if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
      perror("IP_HDRINCL");
	  close(s);
      return(1);
   }

	/*set timeout for sendto() and recvfrom()*/
   tv.tv_sec = 1;
   tv.tv_usec = 0;	
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

	for(i=0; i<4; i++){
		sleep(timeout);
		ret=sendpings(s, querytype, hostnames, delay, fromaddr);
   		if(!ret)
   			recvpings(s, querytype, hostnames, hostcount, broadcast);
	}


	ping_statistics(argv[0]);

	
   while(hostnames){
   		head=hostnames;
		hostnames=hostnames->next;
		if(head->hostname)
			free(head->hostname);
		free(head);
   }
   close(s);
   return(0);
}
   
/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 *      From FreeBSD's ping.c
 */

u_short
in_cksum(addr, len)
	u_short *addr;
	int len;
{
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}


/*
 * Copyright (c) 1989, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
