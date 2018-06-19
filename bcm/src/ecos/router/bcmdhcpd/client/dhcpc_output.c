/*
 * DHCPC output functions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: dhcpc_output.c,v 1.3 2010-06-12 07:05:29 Exp $
 *
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/ip_var.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dhcpc.h>
#include <dhcpc_osl.h>
#include <dhcpc_output.h>


int
dhcpc_write(struct dhcpc_ifc *difp)
{
	struct dhcphdr *dhcp = &difp->txpkt;
	struct dhcp_packet packet;
	int dhcp_len;
	int packet_len;
	int result;

	/* construct IP+UDP */
	memset(&packet, 0, sizeof(packet));

#if 0
	dhcp_len = sizeof(*dhcp) - sizeof(dhcp->options) + dhcp_option_len(dhcp->options);
	dhcp_len = (dhcp_len + 3) & ~3;
	//add pading for hongkong isp, the linksys dhcp discover pkt len is 328 Byte
	if((sizeof(packet.ip) + sizeof(packet.udp) + dhcp_len) < 328)
	{
		dhcp_len = 328 - (sizeof(packet.ip) + sizeof(packet.udp));
		dhcp_len = (dhcp_len + 3) & ~3;
	}
	//end
#else
	dhcp_len = sizeof(*dhcp);
	dhcp_len = (dhcp_len + 3) & ~3;
#endif
	packet_len = sizeof(packet.ip) + sizeof(packet.udp) + dhcp_len;

	packet.ip.ip_p = IPPROTO_UDP;
	packet.ip.ip_src.s_addr = INADDR_ANY;
	packet.ip.ip_dst.s_addr = INADDR_BROADCAST;
	packet.udp.uh_sport = htons(DHCP_CLIENT_PORT);
	packet.udp.uh_dport = htons(DHCP_SERVER_PORT);
	packet.udp.uh_ulen = htons(sizeof(packet.udp) + dhcp_len); /* cheat on the psuedo-header */
	packet.ip.ip_len = packet.udp.uh_ulen;
	memcpy(&(packet.dhcp), dhcp, dhcp_len);
	packet.udp.uh_sum = dhcp_chksum((unsigned short *)&packet, packet_len);

	packet.ip.ip_len = htons(packet_len);
	packet.ip.ip_hl = sizeof(packet.ip) >> 2;
	packet.ip.ip_v = IPVERSION;
	packet.ip.ip_ttl = IPDEFTTL;
	packet.ip.ip_sum = dhcp_chksum((unsigned short *)&(packet.ip), sizeof(packet.ip));

	result = write(difp->dev_fd, &packet, packet_len);
	if (result <= 0) {
		printf("dhcpc: raw send failed");
	}

	return result;
}

int
dhcpc_send(struct dhcpc_ifc *difp)
{
	struct dhcphdr *dhcp = &difp->txpkt;
	int rc = -1;
	int s;
	int opt = 1;
	struct sockaddr_in sin;
	struct sockaddr_in dst;
	int len;

	if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return -1;

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) == -1)
		goto out;

	/* Bind to source socket pair */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_port = htons(DHCP_CLIENT_PORT);
	sin.sin_addr.s_addr = difp->req_ip.s_addr;
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1)
		goto out;

	/* Send to peer */
	memset(&dst, 0, sizeof(dst));
	dst.sin_family = AF_INET;
	dst.sin_len = sizeof(dst);
	dst.sin_port = htons(DHCP_SERVER_PORT);
	dst.sin_addr.s_addr = difp->server.s_addr;

	len = sizeof(*dhcp) - sizeof(dhcp->options) + dhcp_option_len(dhcp->options);
	rc = sendto(s, dhcp, len, 0, (struct sockaddr *)&dst, sizeof(dst));
out:
	close(s);
	return rc;
}
