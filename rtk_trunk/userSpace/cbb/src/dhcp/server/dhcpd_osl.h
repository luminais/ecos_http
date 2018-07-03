/*
 * DHCP server os include file.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dhcpd_osl.h,v 1.1 2010-06-04 10:40:48 Exp $
 */
#ifndef __DHCPD_OSL_H__
#define __DHCPD_OSL_H__

#if defined(linux)
#include <dhcpd_linux_osl.h>
#elif defined(__ECOS)
#include <dhcpd_ecos_osl.h>
#else
#error "Unsupported OSL requested"
#endif

#endif	/* __DHCHD_OSL_H__ */
