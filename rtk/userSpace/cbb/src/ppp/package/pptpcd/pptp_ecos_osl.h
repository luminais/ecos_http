/*
 * PPTP client include file for eCos platform.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: pptp_ecos_osl.h,v 1.3 2010-07-20 02:12:33 Exp $
 */

#ifndef __PPTP_ECOS_OSL_H__
#define __PPTP_ECOS_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ecos_oslib.h>
#include <sys/syslog.h>

#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

void pptp_start(char *pppname);
void pptp_stop(char *pppname);
void pptp_connect(char *pppname);
void pptp_disconnect(char *pppname);

#define	PPTP_LOG(c...)		syslog(LOG_USER|LOG_INFO, ##c)

#define	pptp_msleep(n)			cyg_thread_delay((n/10));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __PPTP_ECOS_OSL_H__ */
