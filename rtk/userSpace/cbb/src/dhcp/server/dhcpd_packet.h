/*
 * Raw packet function include file of DHCP server module.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd_packet.h,v 1.4 2010-06-08 00:56:55 Exp $
 */
#ifndef __DHCPD_PACKET_H__
#define __DHCPD_PACKET_H__

extern unsigned short dhcp_chksum(unsigned short *ptr, int len);

struct dhcpd_ifc;
int dhcpd_send_packet(struct dhcpd_ifc *ifp, int force_broadcast);
int dhcpd_read_packet(struct dhcpd_ifc *ifp, struct dhcphdr *packet);

#endif	/* __DHCPD_PACKET_H__ */
