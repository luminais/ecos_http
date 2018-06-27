/*
 * kconfig.h - ecos bsp configuration include file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: kconfig.h,v 1.6 2010-06-12 10:07:31 Exp $
 */
#ifndef	__SYS_KCONFIG_H__
#define	__SYS_KCONFIG_H__

#include <kautoconf.h>
#include <pkgconf/system.h>

#ifndef __GNU_ASM__
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#endif

#define INET 1
#define NBPFILTER 0

#define MAX_NO_BRIDGE   NBRIDGE

#define NGIF 0
#define NLOOP 1

#endif	/* __SYS_KCONFIG_H__ */
