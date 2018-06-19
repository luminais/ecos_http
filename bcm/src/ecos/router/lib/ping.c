/*
 * Ping tool
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ping.c,v 1.1.1.1 2010-04-09 10:37:02 Exp $
 *
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#define DEFDATALEN    	(64 - 8)	/* default data length */
#define MAXIPLEN    	60
#define MAXICMPLEN    	76
#define	TIMEOUT		4
int ping_id = 0;

typedef struct {
	int s;
	int datalen;
	struct sockaddr_in whereto;
	unsigned short seq;
	unsigned short id;
	char outpack[256];
} pinger_t;

static int pinger(pinger_t *);

/* Compute INET checksum */
#define checksum inet_cksum

int
inet_cksum(u_short *addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register u_int sum = 0;
	u_short odd_byte = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum),
	 * we add sequential 16 bit words to it, and at the end, fold
	 * back all the carry bits from the top 16 bits into the lower
	 * 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&odd_byte) = *(u_char *)w;
		sum += odd_byte;
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0x0000ffff); /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */
	answer = ~sum;                          /* truncate to 16 bits */
	return (answer);
}

int
icmp_ping(unsigned int dst, int seq, int timeout)
{
	fd_set fdmask;
	int packetlen, hold, cc, fromlen;
	pinger_t pc;
	struct sockaddr_in from, *to;
	struct timeval select_timeout;
	struct ip *ip;
	struct icmp *icp;
	char *packet;

	if (timeout < 10 || timeout > 1000*30) {
		timeout = 1000*TIMEOUT; /* 4 seconds */
	}

	memset((char *)&pc, 0, sizeof(pc));
	pc.datalen = DEFDATALEN;

	to = (struct sockaddr_in *)&pc.whereto;
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = htonl(dst);

	packetlen = pc.datalen + MAXIPLEN + MAXICMPLEN;
	if (!(packet = (char *)malloc(packetlen))) {
		printf("\n\rping: out of memory\n");
		return -1;
	}

	if ((pc.s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		printf("\n\rping: socket error\n");
		free(packet);
		return -1;
	}

	hold = 4096;
	setsockopt(pc.s, SOL_SOCKET, SO_RCVBUF, (char *)&hold, sizeof(hold));

	pc.seq = seq & 0xffff;
	pc.id  =  cyg_thread_self() & 0xffff;

	/* send the ping message */
	if (pinger(&pc) < 0) {
		free(packet);
		close(pc.s);
		return -1;
	}

	FD_ZERO(&fdmask);
	FD_SET(pc.s, &fdmask);
	select_timeout.tv_sec = 0;
	select_timeout.tv_usec = timeout * 1000;
	if (select(pc.s + 1, (fd_set *)&fdmask,
		(fd_set *)NULL,	(fd_set *)NULL, &select_timeout) >= 1) {
		fromlen = sizeof(from);
		if ((cc = recvfrom(pc.s, (char *)packet, packetlen, 0,
		                   (struct sockaddr *)&from, &fromlen)) <= 0) {
			free(packet);
			close(pc.s);
			return -1;
		}
		else  {
			ip = (struct ip *) packet;
			icp = (struct icmp *) (((packet[0] & 0xf) << 2) + packet);

			if ((icp->icmp_seq == pc.seq) && (icp->icmp_id == pc.id)) {
				free(packet);
				close(pc.s);
				return 0;
			}

		}
	}

	free(packet);
	close(pc.s);
	return -1;
}

static int
pinger(pinger_t *pc)
{
	register struct icmp *icp;
	register int cc;

	icp = (struct icmp *)pc->outpack;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = pc->seq;
	icp->icmp_id = pc->id;            /* ID */

	cc = pc->datalen + 8;            /* skips ICMP portion */

	/* compute ICMP checksum here */
	icp->icmp_cksum = checksum((unsigned short *)icp, cc);

	/* Record sequence number for FW check smurf attack */
	ping_id = pc->id;
	if (sendto(pc->s, (char *)pc->outpack,
		cc, 0, (struct sockaddr *)&pc->whereto,
		sizeof(struct sockaddr_in)) < 0) {
		//printf("\n\rping: sendto error\n");
		return -1;
	}
	return (0);
}
