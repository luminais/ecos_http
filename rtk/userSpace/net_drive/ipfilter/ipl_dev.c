/*
 * Firewall device for eCos platform
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ipl_dev.c,v 1.4 2010-07-06 02:12:15 Exp $
 */
#include <pkgconf/io_fileio.h>
#include <pkgconf/io.h>
#include <sys/types.h>
#include <cyg/io/file.h>
#include <cyg/io/io.h>
#include <cyg/fileio/fileio.h>
#include <cyg/io/devtab.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#ifdef INET6
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#endif

#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include <kdev.h>

int filfast_ioctl(int cmd, caddr_t data) __attribute__((weak));

int
filfast_ioctl(int cmd, caddr_t data)
{
	return EINVAL;
}

static int
ipldev_ioctl(void *devpriv, unsigned long cmd, char *data)
{
	int s = 0;
	int rc = EINVAL;
	s = splnet();
	if (IOCGROUP(cmd) == 'f')
		rc = filfast_ioctl(cmd, data);
		/*È¥µôipnatµÄioctl*/
#if 0
	else
		rc = IPL_EXTERN(ioctl)(IPL_LOGIPF, cmd, data, FWRITE | FREAD);
#endif
	splx(s);
	return rc;
}

/* Declare kdev */
KDEV_NODE(ipl,
	NULL,
	NULL,
	ipldev_ioctl,
	NULL);
