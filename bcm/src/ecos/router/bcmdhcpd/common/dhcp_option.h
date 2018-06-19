/*
 * DHCP option definitions
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: dhcp_option.h,v 1.1 2010-06-04 10:40:25 Exp $
 *
 */

#ifndef __DHCP_OPTION_H__
#define __DHCP_OPTION_H__

/* See RFC 2131 */
#define DHCP_MAGIC		0x63825363

/* DHCP option */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET		0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER		0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL		0x17
#define DHCP_MTU		0x1a
#define STATIC_ROUTER_33	0x21
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1			0x3a
#define DHCP_T2			0x3b
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID		0x3d
#define	DHCP_TFTP		0x42
#define	DHCP_BOOTFILE		0x43
#define STATIC_ROUTER_121	0x79
#define STATIC_ROUTER_249	0xF9
#define DHCP_END		0xFF

#define DHCP_DISCOVER		1
#define DHCP_OFFER		2
#define DHCP_REQUEST		3
#define DHCP_DECLINE		4
#define DHCP_ACK		5
#define DHCP_NAK		6
#define DHCP_RELEASE		7
#define DHCP_INFORM		8

/* Definition of options overload */
#define OVERLOAD_NONE		0
#define OVERLOAD_FILE		1
#define OVERLOAD_SNAME		2

#define OPTION_TAG		0
#define OPTION_LEN		1
#define OPTION_VALUE		2
#define	OPTION_HDRLEN		2

/* Functions */
unsigned char *dhcp_get_option(struct dhcphdr *dhcp, int code, int dest_len, void *dest);
int dhcp_add_option(struct dhcphdr *dhcp, unsigned char code, unsigned char len, void *data);
int dhcp_option_len(unsigned char *ptr);

#endif	/* __DHCP_OPTION__ */
