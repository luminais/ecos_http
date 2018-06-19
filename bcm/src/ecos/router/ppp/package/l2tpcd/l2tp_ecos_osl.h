/*
 * L2TP client include file for eCos platform.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: l2tp_ecos_osl.h,v 1.3 2010-07-08 09:13:59 Exp $
 */

#ifndef __L2TP_ECOS_OSL_H__
#define __L2TP_ECOS_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include <ecos_oslib.h>
#include <sys/syslog.h>

#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

void l2tp_start(char *pppname);
void l2tp_stop(char *pppname);
void l2tp_connect(char *pppname);
void l2tp_disconnect(char *pppname);
char errmsg[255];

#define	L2TP_PRINTF(a, b...)		printf(a, ##b)
#define ERRMSG				errmsg
#define L2TP_SET_ERRMSG(a, b...)	\
{	\
	sprintf(errmsg, a, ##b); \
	printf(errmsg); \
	printf("\n");	\
}

#define	L2TP_LOG(c...)	syslog(LOG_USER|LOG_INFO, ##c)

#define	l2tp_msleep(n)			cyg_thread_delay((n/10));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __L2TP_ECOS_OSL_H__ */
