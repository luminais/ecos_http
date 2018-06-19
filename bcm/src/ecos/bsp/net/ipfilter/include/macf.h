/*
 * MAC filter include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: macf.h,v 1.2 2010-05-11 10:27:01 Exp $
 *
 */
#ifndef	__MACF_H__
#define __MACF_H__

enum {
	MACF_MODE_DISABLED = 0,
	MACF_MODE_DENY,
	MACF_MODE_PASS
};

#define MACF_PASS	0
#define MACF_BLOCK	1

struct macfilter {
	unsigned char mac[6];
	struct macfilter *next;
};

#ifdef _KERNEL
/* Functions */
int macfilter_set_mode(int mode);
int macfilter_add(struct macfilter *macinfo);
int macfilter_del(struct macfilter *macinfo);
void macfilter_flush(void);
#endif
#endif	/* __MACF_H__ */
