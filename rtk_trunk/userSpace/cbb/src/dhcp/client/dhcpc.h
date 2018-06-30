/*
 * DHCP client include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpc.h,v 1.7 2010-07-24 12:48:31 Exp $
 */

#ifndef __DHCPC_H__
#define __DHCPC_H__

#include <dhcp_var.h>
#include <dhcp_option.h>
#include <dhcpc_osl.h>

#define DHCPC_STATE_INIT			1
#define DHCPC_STATE_SELECTING			2
#define DHCPC_STATE_REQUESTING			3
#define DHCPC_STATE_BOUND			4
#define DHCPC_STATE_RENEWING			5
#define DHCPC_STATE_REBINDING			6

#define DHCPC_LEASE_T1				1
#define DHCPC_LEASE_T2				2
#define DHCPC_LEASE_EX				4

struct dhcp_lease {
	unsigned int time_stamp;
	unsigned int time_last;			/* For NTP */
	unsigned int expiry;
	unsigned int t1;
	unsigned int t2;
	unsigned int timeout;
	volatile int curr;
	volatile int next;
};

struct dhcpc_config {
	char ifname[16];
	struct in_addr req_ip;
	char hostname[256];
	int mtu;
	char script[128];
};

#define	DHCPC_IPC_PORT		6868
#define DHCPC_DEVNAME		"/dev/net/dhcpc"

struct dhcpc_ifc {
	int event;
	int efd;
	int dev_fd;
	char ifname[16];
	int state;
	struct in_addr server;
	char *hostname;
	int mtu;
	struct in_addr req_ip;
	char script[128];

	char macaddr[6];
	unsigned int xid;
	struct dhcphdr txpkt;
	struct dhcphdr rxpkt;

	int countdown;
	int secs;
	int rmsecs;			/* remaiming seconds */
	struct dhcp_lease lease;
};

/*
 * Functions
 */
void dhcpc_mainloop(struct dhcpc_config *config);

extern int open_udp_socket(char *ipaddr, unsigned short port);

#endif /* __DHCPC_H__ */
