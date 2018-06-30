/*
 * PPPOE protocol include file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pppoe.h,v 1.1 2010-07-15 11:08:18 Exp $
 */
#ifndef __PPPOE_H__
#define __PPPOE_H__

#include <sys/param.h>
#include <net/ethernet.h>

#define ETHERTYPE_PPPOE_DISC	0x8863	/* pppoe discovery packets     */
#define ETHERTYPE_PPPOE_SESS	0x8864	/* pppoe session packets       */

/* Codes to identify message types */
#define PADI_CODE		0x09
#define PADO_CODE		0x07
#define PADR_CODE		0x19
#define PADS_CODE		0x65
#define PADT_CODE		0xa7

/* Tag identifiers */
#define PTT_EOL			(0x0000)
#define PTT_SRV_NAME		(0x0101)
#define PTT_AC_NAME		(0x0102)
#define PTT_HOST_UNIQ		(0x0103)
#define PTT_AC_COOKIE		(0x0104)
#define PTT_VENDOR		(0x0105)
#define PTT_RELAY_SID		(0x0106)
#define PTT_RELAY_SESSION_ID		(0x0110) //wxy
#define PTT_SRV_ERR		(0x0201)
#define PTT_SYS_ERR  		(0x0202)
#define PTT_GEN_ERR  		(0x0203)

struct pppoe_tag {
	unsigned short tag_type;
	unsigned short tag_len;
	unsigned char tag_data[0];
} __attribute__((packed));

struct pppoe_hdr {
	unsigned char ver:4;
	unsigned char type:4;
	unsigned char code;
	unsigned short sid;
	unsigned short length;
	struct pppoe_tag tag[0];
} __attribute__((packed));

struct pppoe_full_hdr {
	struct ether_header eh;
	struct pppoe_hdr ph;
} __attribute__((packed));

/*
 * Define the order in which we will place tags in packets
 * this may be ignored
 */
/* for PADI */
#define TAGI_SVC		0
#define TAGI_HUNIQ		1

/* for PADO */
#define TAGO_ACNAME		0
#define TAGO_SVC		1
#define TAGO_COOKIE		2
#define TAGO_HUNIQ		3

/* for PADR */
#define TAGR_SVC		0
#define TAGR_HUNIQ		1
#define TAGR_COOKIE		2

/* for PADS */
#define TAGS_ACNAME		0
#define TAGS_SVC		1
#define TAGS_COOKIE		2
#define TAGS_HUNIQ		3

//#define	PPPOE_HOST_UNIQ_LEN	8
#define	PPPOE_HOST_UNIQ_LEN	12//gong  mark ºÊ»›win7 64Œª

/* for PADT */
#define PADT_SIGNOFF	"session closed"

#endif /* __PPPOE_H__ */
