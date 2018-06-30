/*
 * URL filter include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: urlf.h,v 1.2 2010-07-30 06:38:09 Exp $
 *
 */
#ifndef	__WAN2LAN_H__
#define __WAN2LAN_H__

enum {
	WAN2LANF_MODE_DISABLED = 0,
	WAN2LANF_MODE_ENABLED = 1,
};

/* Functions */
int wan2lanf_set_ip(int ip);

int wan2lanf_set_mask(int mask);

int wan2lanfilter_set_mode(int mode);

#endif	/* __WAN2LAN_H__ */
