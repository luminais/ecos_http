/*
 * DHCP header file.
 *
 * eCos package always installs dhcp.h.
 * To avoid conflicting, change name to dhcp_var.h
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcp_var.h,v 1.2 2010-06-12 07:05:30 Exp $
 */

#ifndef __DHCP_VAR_H__
#define __DHCP_VAR_H__

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <unistd.h>

#define DHCP_SERVER_PORT	67
#define DHCP_CLIENT_PORT	68

#define DHCP_BOOTREQUEST	1
#define DHCP_BOOTREPLY		2
#define BROADCAST_FLAG		0x8000
#define DHCP_HTYPE_ETHERNET	1
#define DHCP_HLEN_ETHERNET	6

struct dhcphdr {
	unsigned char op;
	unsigned char htype;
	unsigned char hlen;
	unsigned char hops;
	unsigned long xid;
	unsigned short secs;
	unsigned short flags;
	struct in_addr ciaddr;
	struct in_addr yiaddr;
	struct in_addr siaddr;
	struct in_addr giaddr;
	unsigned char chaddr[16];
	unsigned char sname[64];
	unsigned char file[128];
	unsigned long cookie;
	unsigned char options[308];
};

struct dhcp_packet {
	struct ip ip;
	struct udphdr udp;
	struct dhcphdr dhcp;
};

#endif /* __DHCP_VAR_H__ */
