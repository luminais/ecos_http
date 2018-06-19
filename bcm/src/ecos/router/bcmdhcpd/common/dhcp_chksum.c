/*
 * DHCPC checksum for write directly to device.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: dhcp_chksum.c,v 1.2 2010-06-07 06:06:23 Exp $
 *
 */

unsigned short
dhcp_chksum(unsigned short *ptr, int len)
{
	int sum = 0;
	int i;

	for (i = 0; i < len/2; i++)
		sum += *ptr++;

	/* Handle odd byte */
	if (len & 1)
		sum += *(unsigned char *)ptr;

	/* Perform one's complement */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}
