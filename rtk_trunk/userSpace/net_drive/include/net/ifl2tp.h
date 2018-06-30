/*
 * interface l2tp
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ifl2tp.h,v 1.3 2010-07-19 08:20:57 Exp $
 */
#ifndef	__IFL2TP_H__
#define __IFL2TP_H___

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ioccom.h>

/*
 * L2TP control request: add/delete interface.
 */
#define	PPPIOCIFL2TPCONN		_IOWR('t', 120, struct ifl2tpreq)
#define	PPPIOCIFL2TPWAIT		_IOWR('t', 121, struct ifl2tpreq)


struct ifl2tpreq {
	char pppname[IFNAMSIZ];
	struct in_addr	tunnel_addr;
	struct in_addr	server_addr;
	unsigned short	server_port;
	unsigned short	assigned_tid;
	unsigned short	assigned_sid;
	unsigned short  tid;
	unsigned short  sid;
	int wait;
};

#ifdef _KERNEL
#include <net/ppp_defs.h>
#include <net/if_ppp.h>

struct ifl2tp
{
	/* common ppp structures */
	struct ifppp	if_ppp;
	void *devtab;

	/* L2TP information */
	struct in_addr	tunnel_addr;
	struct in_addr	server_addr;
	unsigned short	server_port;
	unsigned short	assigned_tid;
	unsigned short	assigned_sid;
	unsigned short  tid;
	unsigned short  sid;
};
#endif	/* _KERNEL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* 	__IFL2TP_H__ */
