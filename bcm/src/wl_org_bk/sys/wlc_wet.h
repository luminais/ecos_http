/*
 * Wireless Ethernet (WET) interface
 *
 *   Copyright (C) 2010, Broadcom Corporation
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom Corporation.
 *
 *   $Id: wlc_wet.h,v 1.8 2006-01-31 07:45:23 Exp $
 */

#ifndef _wlc_wet_h_
#define _wlc_wet_h_

/* forward declaration */
typedef struct wlc_wet_info wlc_wet_info_t;

/*
 * Initialize wet private context.It returns a pointer to the
 * wet private context if succeeded. Otherwise it returns NULL.
 */
extern wlc_wet_info_t *wlc_wet_attach(wlc_pub_t *pub);

/* Cleanup wet private context */
extern void wlc_wet_detach(wlc_wet_info_t *weth);

/* Process frames in transmit direction */
extern int wlc_wet_send_proc(wlc_wet_info_t *weth, void *sdu, void **new);

/* Process frames in receive direction */
extern int wlc_wet_recv_proc(wlc_wet_info_t *weth, void *sdu);

#ifdef WET_TUNNEL
/* Parse wet tunnel ie for tunneling capability */
extern void wlc_wet_parse_tunnel_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len);

/* Check if multicast frame forwarded back by AP-WET Tunnel */
extern int wlc_wet_multicast_forwarded_back(wlc_info_t *wlc, uint8 *eaddr);
#endif /* WET_TUNNEL */

#ifdef BCMDBG
extern int wlc_wet_dump(wlc_wet_info_t *weth, struct bcmstrbuf *b);
#endif /* BCMDBG */

#endif	/* _wlc_wet_h_ */
