/*
 * dhcpc output functions.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: dhcpc_output.h,v 1.2 2010-06-07 06:11:26 Exp $
 *
 */

#ifndef __DHCPC_OUTPUT_H__
#define __DHCPC_OUTPUT_H__

extern unsigned short dhcp_chksum(unsigned short *ptr, int len);

struct dhcpc_ifc;
int dhcpc_write(struct dhcpc_ifc *difp);
int dhcpc_send(struct dhcpc_ifc *difp);

#endif	/* __DHCPC_OUTPUT_H__ */
