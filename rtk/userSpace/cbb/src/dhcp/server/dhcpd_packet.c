/*
 * DHCP server send functions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd_packet.c,v 1.7 2010-07-05 08:18:39 Exp $
 */
#include <dhcpd.h>
#include <dhcpd_packet.h>

static int
dhcpd_send_to_relay(struct dhcpd_ifc *ifp)
{
	struct dhcphdr *dhcp = &ifp->sndpkt;

	int rc = -1;
	int s;
	int opt = 1;
	struct sockaddr_in sin;
	struct sockaddr_in dst;

	if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return -1;

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) == -1)
		goto out;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_port = htons(DHCP_SERVER_PORT);
	sin.sin_addr.s_addr = ifp->ipaddr.s_addr;

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1)
		goto out;

	memset(&dst, 0, sizeof(dst));
	dst.sin_family = AF_INET;
	dst.sin_len = sizeof(dst);
	dst.sin_port = htons(DHCP_SERVER_PORT);
	dst.sin_addr = dhcp->giaddr;

	rc = sendto(s, dhcp, sizeof(*dhcp), 0, (struct sockaddr *)&dst, sizeof(dst));
out:
	close(s);
	return rc;
}

int
dhcpd_send_to_client(struct dhcpd_ifc *ifp, int force_broadcast)
{
	struct dhcphdr *dhcp = &ifp->sndpkt;

	struct dhcp_packet packet;
	int dhcp_len;
	int packet_len;
	unsigned char bcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	unsigned char *chaddr;
	struct in_addr ciaddr;
	int result;

	if (force_broadcast || (ifp->cflags & htons(BROADCAST_FLAG))) {
		ciaddr.s_addr = INADDR_BROADCAST;
		chaddr = bcast;
	}
	else if (dhcp->ciaddr.s_addr) {
		ciaddr = dhcp->ciaddr;
		chaddr = dhcp->chaddr;
	}
	else {
		ciaddr.s_addr = INADDR_BROADCAST;
		chaddr = bcast;
	}

	/* construct IP+UDP */
	memset(&packet, 0, sizeof(packet));

#if 0
	dhcp_len = sizeof(*dhcp) - sizeof(dhcp->options) + dhcp_option_len(dhcp->options);
	dhcp_len = (dhcp_len + 3) & ~3;
#else
	dhcp_len = sizeof(*dhcp);
	dhcp_len = (dhcp_len + 3) & ~3;
#endif
	packet_len = sizeof(packet.ip) + sizeof(packet.udp) + dhcp_len;

	packet.ip.ip_p = IPPROTO_UDP;
	packet.ip.ip_src = ifp->ipaddr;
	packet.ip.ip_dst = ciaddr;
	packet.udp.uh_sport = htons(DHCP_SERVER_PORT);
	packet.udp.uh_dport = htons(DHCP_CLIENT_PORT);
	packet.udp.uh_ulen = htons(sizeof(packet.udp) + dhcp_len); /* cheat on the psuedo-header */
	packet.ip.ip_len = packet.udp.uh_ulen;
	memcpy(&(packet.dhcp), dhcp, dhcp_len);
	packet.udp.uh_sum = dhcp_chksum((unsigned short *)&packet, packet_len);

	packet.ip.ip_len = htons(packet_len);
	packet.ip.ip_hl = sizeof(packet.ip) >> 2;
	packet.ip.ip_v = IPVERSION;
	packet.ip.ip_ttl = IPDEFTTL;
	packet.ip.ip_sum = dhcp_chksum((unsigned short *)&(packet.ip), sizeof(packet.ip));

	/* send out to specified interface */
	result = write(ifp->fd, &packet, packet_len);
	if (result <= 0) {
		DHCPD_DBG("DHCPD_SendToClient(): sendto failed\n");
	}

	return result;
}

int
dhcpd_send_packet(struct dhcpd_ifc *ifp, int force_broadcast)
{
	struct dhcphdr *dhcp = &ifp->sndpkt;

	if (dhcp->giaddr.s_addr)
		dhcpd_send_to_relay(ifp);
	else
		dhcpd_send_to_client(ifp, force_broadcast);

	return 0;
}

int
dhcpd_read_packet(struct dhcpd_ifc *ifp, struct dhcphdr *dhcp)
{
	int fd = ifp->fd;
	int bytes;

	memset(dhcp, 0, sizeof(*dhcp));

	bytes = read(fd, dhcp, sizeof(*dhcp));
	if (bytes < 0) {
		DHCPD_DBG("couldn't read on listening socket, ignoring.\n");
		return -1;
	}

	return bytes;
}
