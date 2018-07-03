/*
 * OSL application level socket functions
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: libsocket.c,v 1.2 2010-06-30 05:17:29 Exp $
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdio.h>

int
open_udp_socket(char *ipaddr, unsigned short port)
{
	int s;
	struct sockaddr_in sin;
	int val = 1;

	if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("osl_open_udp_socket failed: %s\n", strerror(errno));
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char *)&val, sizeof(val)) == -1) {
		printf("osl_open_udp_socket: setsockopt SO_REUSEPORT failed\n");
		close(s);
		return -1;
	}

	/* Bind to the server port */
	memset(&sin, 0, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(ipaddr);
	if (bind(s, (struct sockaddr *)&sin, sizeof(struct sockaddr)) == -1) {
		printf("osl_open_udp_socket: bind failed: %s\n", strerror(errno));
		close(s);
		return -1;
	}

	return s;
}
