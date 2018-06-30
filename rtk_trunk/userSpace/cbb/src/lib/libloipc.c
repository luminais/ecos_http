/*
 * loopback ipc
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: libloipc.c,v 1.2 2010-06-30 05:21:42 Exp $
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdio.h>

extern int open_udp_socket(char *ipaddr, unsigned short port);

/* IPC through loopback socket */
int
loipc(char *cmd, int sport, int dport, char *resp, int *rlen)
{
	int rc = -1;
	int s = -1;
	struct sockaddr_in dst = {0};
	fd_set fds;
	struct timeval tval = {1, 0};

	/* Open command port */
	if ((s = open_udp_socket("127.0.0.1", sport)) < 0)
		return -1;

	/* Send to interface rpc */
	dst.sin_family = AF_INET;
	dst.sin_len = sizeof(dst);
	dst.sin_addr.s_addr = inet_addr("127.0.0.1");
	dst.sin_port = htons(dport);

	sendto(s, cmd, strlen(cmd)+1, 0, (struct sockaddr *)&dst, sizeof(dst));

	/* Read response, max wait 1 seconds */
	if (resp) {
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		if (select(s+1, &fds, NULL, NULL, &tval) > 0) {
			*rlen = read(s, resp, *rlen);
			if (*rlen <= 0)
				goto err_out;
			/* Done */
			rc = 0;
		}
	}
	else {
		/* No response needed,
		 * Command done.
		 */
		rc = 0;
	}

err_out:
	close(s);
	return rc;
}
