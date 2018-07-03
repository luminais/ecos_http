/*
 * PPTP GRE include
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp_gre.h,v 1.4 2010-07-19 08:19:07 Exp $
 */
#ifndef __PPTP_GRE_H__
#define __PPTP_GRE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ioccom.h>

struct pptp_gre_hdr {
	unsigned char  flags;
	unsigned char  version;
	unsigned short protocol;
	unsigned short payload_len;
	unsigned short call_id;
	unsigned long  seq_num;
	unsigned long  ack_num;
};

#define	GREMINLEN				(sizeof(struct pptp_gre_hdr) - 4)

#define PPTP_GRE_FLAGS_C		0x80	/* Checksum Present */
#define PPTP_GRE_FLAGS_R		0x40	/* Routing Present */
#define PPTP_GRE_FLAGS_K		0x20	/* Key Present */
#define PPTP_GRE_FLAGS_S		0x10	/* Sequence Number Present */
#define PPTP_GRE_VER_ACK		0x80	/* Acknowledge Number Present */

#define PPTP_GRE_MAX_PKTSIZE	2048
#define PPTP_GRE_PROTO			0x880B
#define PPTP_GRE_VER			1

/*
 * PPTP_GRE control request: add/delete interface.
 */
#define	PPPIOCPPTPCONN		_IOWR('t', 110, struct ifpptpreq)	/* add pptp ifp */
#define PPPIOCPPTPWAIT		_IOWR('t', 111, struct ifpptpreq)	/* daemon hook */

struct ifpptpreq {
	char pppname[IFNAMSIZ];
	struct in_addr server_ip;
	struct in_addr tunnel_ip;
	unsigned short call_id;
	unsigned short peer_call_id;
	int wait;
};

#ifdef _KERNEL
#include <net/ppp_defs.h>
#include <net/if_ppp.h>

struct ifpptp {
	struct ifppp if_ppp;
	void *devtab;
	
	struct in_addr server_ip;
	struct in_addr tunnel_ip;

	/* PPTP_GRE */
	unsigned short call_id;
	unsigned short peer_call_id;

	unsigned long  ack_sent;
	unsigned long  ack_recv;
	unsigned long  seq_sent;
	unsigned long  seq_recv;
};

#endif	/* _KERNEL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PPTP_GRE_H__ */
