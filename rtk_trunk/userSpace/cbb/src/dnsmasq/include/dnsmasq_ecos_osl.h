/*
 * DNSMASQ server ecos include files.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: dnsmasq_ecos_osl.h,v 1.5 2010-08-10 06:58:39 Exp $
 */

#ifndef __DNSMASQ_ECOS_OSL_H__
#define __DNSMASQ_ECOS_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <sys/syslog.h>
#include <autoconf.h>	//llm add, for compile

void dnsmasq_start(void);
void dnsmasq_stop(void);

#if 0
#define DNSMASQ_DEBUG_OFF		0
#define DNSMASQ_DEBUG_ERROR		1
extern int DNSMASQDebugLevel;
#define DNSMASQ_DBG(args...)   \
{                                           \
    if (DNSMASQDebugLevel == DNSMASQ_DEBUG_ERROR)                      \
    {                                       \
		syslog(LOG_DEBUG,##args);        \
    }                                       \
}

#define DNSMASQ_LOG(args...)   \
{                                           \
    if (DNSMASQDebugLevel == DNSMASQ_DEBUG_ERROR)                      \
    {                                       \
		syslog(LOG_DEBUG,##args);        \
    }                                       \
}

#define DNSMASQ_PRINT(c...)

/*llm add, for debug*/
#undef DNSMASQ_DBG
#undef DNSMASQ_LOG
#define DNSMASQ_DBG(format, args...) printf("[dns debug] "format"\n", ##args)
#define DNSMASQ_LOG(format, args...) printf("[dns log] "format"\n", ##args)

#else

#define DNSMASQ_DEBUG_OFF		0
#define DNSMASQ_DEBUG_ERROR		1
extern int DNSMASQDebugLevel;
#define DNSMASQ_DBG(args...) 
#define DNSMASQ_LOG(args...)
#define DNSMASQ_PRINT(c...)
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __DNSMASQ_ECOS_OSL_H__ */
