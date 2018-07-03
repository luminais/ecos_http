/*
 * DHCP client os include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: dhcpc_osl.h,v 1.1 2010-06-04 10:38:57 Exp $
 */

#ifndef __DHCPC_OSL_H__
#define __DHCPC_OSL_H__

#if defined(linux)
#include <dhcpc_linux_osl.h>
#elif defined(__ECOS)
#include <dhcpc_ecos_osl.h>
#else
#error "Unsupported OSL requested"
#endif

#endif	/* __DHCPC_OSL_H__ */
