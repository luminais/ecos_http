/*
 * ARP Offload interface
 *
 *   Copyright (C) 2010, Broadcom Corporation
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom Corporation.
 *
 *   $Id: wl_arpoe.h,v 1.3 2007-03-07 02:36:44 Exp $
 */

#ifndef _wl_arpoe_h_
#define _wl_arpoe_h_

/* Forward declaration */
typedef struct wl_arp_info wl_arp_info_t;

/* Return values */
#define ARP_REPLY_PEER 		0x1	/* Reply was sent to service ARP request from peer */
#define ARP_REPLY_HOST		0x2	/* Reply was sent to service ARP request from host */
#define ARP_REQ_SINK		0x4	/* Input packet should be discarded */

#ifdef ARPOE

/*
 * Initialize ARP private context.
 * Returns a pointer to the ARP private context, NULL on failure.
 */
extern wl_arp_info_t *wl_arp_attach(wlc_info_t *wlc);

/* Cleanup ARP private context */
extern void wl_arp_detach(wl_arp_info_t *arpi);

/* Process frames in transmit direction */
extern int wl_arp_send_proc(wl_arp_info_t *arpi, void *sdu);

/* Process frames in receive direction */
extern int wl_arp_recv_proc(wl_arp_info_t *arpi, void *sdu);

#else	/* stubs */

#define wl_arp_attach(a)		(wl_arp_info_t *)0x0dadbeef
#define	wl_arp_detach(a)		do {} while (0)
#define wl_arp_send_proc(a, b)		(-1)
#define wl_arp_recv_proc(a, b)		(-1)

#endif /* ARPOE */

#endif	/* _wl_arpoe_h_ */
