/*
 * eCos 2.0 device driver tunables for
 * Broadcom BCM947XX 10/100/1000 Mbps Ethernet Device Driver
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: et_ecos.h,v 1.3.6.1 2010-12-21 02:35:38 Exp $
 */

#ifndef _et_ecos_h_
#define _et_ecos_h_

#include <sys/param.h>

/* tunables */
#define	NTXD		128		/* # tx dma ring descriptors (must be ^2) */
#define	NRXD		512		/* # rx dma ring descriptors (must be ^2) */
#define	NRXBUFPOST	160		/* try to keep this # rbufs posted to the chip */
#define	BUFSZ		2048		/* packet data buffer size */
#define	RXBUFSZ	(BUFSZ - 256)	/* receive buffer size */

#ifndef RXBND
#define RXBND		32		/* max # rx frames to process in dpc */
#endif

#if defined(ILSIM) || defined(__arch_um__)
#undef	NTXD
#define	NTXD		16
#undef	NRXD
#define	NRXD		16
#undef	NRXBUFPOST
#define	NRXBUFPOST	2
#endif


struct vlan_hdr {
	unsigned short	h_vlan_TCI;		/* Encapsulates priority and VLAN ID */
	unsigned short	h_vlan_encapsulated_proto; /* packet type ID field (or len) */
};

#ifdef BCMDBG
#undef ET_ERROR
#undef ET_TRACE
#undef ET_PRHDR
#undef ET_PRPKT
#define ET_ERROR(args)  if (!(et_msg_level & 1)); else diag_printf args
#define ET_TRACE(args)  if (!(et_msg_level & 2)); else diag_printf args
extern void etc_prhdr(char *msg, struct ether_header *eh, uint len, int unit);
extern void etc_prhex(char *msg, uchar *buf, uint nbytes, int unit);

#define	ET_PRHDR(msg, eh, len, unit)	etc_prhdr(msg, eh, len, unit)
#define	ET_PRPKT(msg, buf, len, unit)	etc_prhex(msg, buf, len, unit)

#endif

#endif	/* _et_ecos_h_ */
