/*
 * NAT device for eCos platform
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: nat_dev.c,v 1.3 2010-07-06 02:12:15 Exp $
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
#ifdef INET6
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#endif

#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include <kdev.h>

static int
natdev_ioctl(void *devpriv, unsigned long cmd, char *data)
{
	int s, rc;

	s = splnet();
	rc = IPL_EXTERN(ioctl)(IPL_LOGNAT, cmd, data, FWRITE|FREAD);
	splx(s);
	return rc;
}

/* Declare kdev */
KDEV_NODE(ipnat,
	NULL,
	NULL,
	natdev_ioctl,
	NULL);
