/*
 * kdev include file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: kdev.h,v 1.4 2010-07-06 02:12:13 Exp $
 */

#ifndef	__KDEV_H__
#define __KDEV_H___

#include <cyg/hal/hal_tables.h>

#define	KDEV_NAMSIZ	16
typedef void *(*KDEV_OPEN)(void *, char *);
typedef int (*KDEV_WRITE)(void *, char *, int);
typedef int (*KDEV_IOCTL)(void *, unsigned long, char *);
typedef int (*KDEV_CLOSE)(void *);

typedef struct kdev_node {
	char node_name[KDEV_NAMSIZ];
	KDEV_OPEN open;
	KDEV_WRITE write;
	KDEV_IOCTL ioctl;
	KDEV_CLOSE close;
} CYG_HAL_TABLE_TYPE kdev_node_t;

/* External node */
#define KDEV_NODE(name, open, write, ioctl, close)  \
kdev_node_t kdev_node_##name CYG_HAL_TABLE_ENTRY(kdevtab) = { \
	#name, \
	(KDEV_OPEN)open, \
	(KDEV_WRITE)write, \
	(KDEV_IOCTL)ioctl, \
	(KDEV_CLOSE)close \
};

extern kdev_node_t __kdev_tab__[];
extern kdev_node_t __kdev_tab_end__;

/* External functions */
struct mbuf;
int kdev_input(void *devtab, struct mbuf *m);

#endif	/* __KDEV_H__ */
