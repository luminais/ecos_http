/*
 * PPPOE variables
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: if_pppoe.h,v 1.2 2010-07-19 08:15:54 Exp $
 */
#ifndef __IFPPPOE_H__
#define __IFPPPOE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioccom.h>
#include "pppoe.h"

/*
 * PPPOE control request: add/delete interface.
 */
#define	PPPIOCSPPPOESTATE	_IOWR('t', 100, struct ifpppoereq)

struct ifpppoereq {
	char ethname[IFNAMSIZ];
	int state;
	unsigned short sid;
	unsigned char dhost[6];
	unsigned char shost[6];
	char unique[PPPOE_HOST_UNIQ_LEN];
};

/*
 * States for the session state machine.
 * These have no meaning if there is no hook attached yet.
 */
enum state {
	PPPOE_SNONE = 0,	/* Zombie state */
	PPPOE_SDWAIT,		/* Wait dialup notify */
	PPPOE_SINIT,		/* Sent discovery initiation */
	PPPOE_SREQ,		/* Sent a Request */
	PPPOE_CONNECTED,	/* Connection established, Data received */
	PPPOE_CLOSE,	/*pxy add, only for "ppp2" thread*/
};

#ifdef _KERNEL
#include "ppp_defs.h"
#include "if_ppp.h"

struct ifpppoe
{
	struct ifppp	if_ppp;
	struct ifpppoe	*next;
	struct ifnet	*eth_ifp;
	void *devtab;

	int state;
	unsigned short sid;
	unsigned char dhost[6];
	unsigned char shost[6];
	char unique[PPPOE_HOST_UNIQ_LEN];
};

int ifpppoe_input(struct ifnet *eth_ifp, struct ether_header *eh, struct mbuf *m);
int pppoe_ioctl(u_long cmd, caddr_t data);

#endif	/* _KERNEL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __IFPPPOE_H__ */
