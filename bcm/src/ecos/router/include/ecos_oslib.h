/*
 * ecos osl include files.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: ecos_oslib.h,v 1.3 2010-07-21 03:42:45 Exp $
 */

#ifndef __ECOS_OSLIB_H__
#define __ECOS_OSLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

char *oslib_make_nvname(char *prefix, int ifunit, char *name);
int oslib_ifname_list(char *ifname_list);

void oslib_ticks_sleep(int knl_ticks);
int oslib_pid(void);
int oslib_getpidbyname(char *tname);
int oslib_waitpid(int pid, int *status);
int oslib_getnamebypid(pid_t pid, char *name);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __ECOS_OSLIB_H__ */
