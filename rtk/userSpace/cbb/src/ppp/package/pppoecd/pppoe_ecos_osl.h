/*
 * PPPOE client include file for eCos platform.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pppoe_ecos_osl.h,v 1.1 2010-06-25 16:16:52 Exp $
 */

#ifndef __PPPOE_ECOS_OSL_H__
#define __PPPOE_ECOS_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ecos_oslib.h>
#include <sys/syslog.h>

#ifndef	TRUE
#define	TRUE	1
#endif

void pppoe_start(char *pppname);
void pppoe_stop(char *pppname);
void pppoe_connect(char *pppname);
void pppoe_disconnect(char *pppname);

#define	PPPOE_PRINTF(a, b...)		printf(a, ##b)

#define	PPPOE_LOG(c...) syslog(LOG_USER|LOG_INFO, ##c)

#define	pppoe_msleep(n)			cyg_thread_delay((n/10));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __PPPOE_ECOS_OSL_H__ */
