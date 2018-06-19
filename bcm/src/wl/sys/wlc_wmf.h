/*
 * Wireless Multicast Forwarding
 *
 *   Copyright (C) 2010, Broadcom Corporation
 *   All Rights Reserved.
 *   
 *   This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *   the contents of this file may not be disclosed to third parties, copied
 *   or duplicated in any form, in whole or in part, without the prior
 *   written permission of Broadcom Corporation.
 *
 *  $Id: wlc_wmf.h,v 1.1 2008-08-11 06:28:53 Exp $
 */

#ifndef _wlc_wmf_h_
#define _wlc_wmf_h_

/* Packet handling decision code */
#define WMF_DROP 0
#define WMF_NOP 1
#define WMF_TAKEN 2

/* WMF instance specific information */
struct wlc_wmf_instance {
	wlc_info_t *wlc; /* Pointer to wlc structure */
	void *emfci;	/* Pointer to emfc instance */
	void *igsci;	/* Pointer to igsc instance */
};

/* Module attach and detach functions */
extern wmf_info_t *wlc_wmf_attach(wlc_info_t *wlc);
extern void wlc_wmf_detach(wmf_info_t *wmfi);

/* Add wmf instance to a bsscfg */
extern int32 wlc_wmf_instance_add(wlc_info_t *wlc, struct wlc_bsscfg *bsscfg);

/* Delete wmf instance from bsscfg */
extern void wlc_wmf_instance_del(wlc_bsscfg_t *bsscfg);

/* Start WMF on the bsscfg */
extern int wlc_wmf_start(wlc_bsscfg_t *bsscfg);

/* Stop WMF on the bsscfg */
extern void wlc_wmf_stop(wlc_bsscfg_t *bsscfg);

/* Add a station to the WMF interface list */
extern int wlc_wmf_sta_add(wlc_bsscfg_t *bsscfg, struct scb *scb);

/* Delete a station from the WMF interface list */
extern int wlc_wmf_sta_del(wlc_bsscfg_t *bsscfg, struct scb *scb);

/* WMF packet handler */
extern int wlc_wmf_packets_handle(wlc_bsscfg_t *bsscfg, struct scb *scb, void *p, bool frombss);

#endif	/* _wlc_wmf_h_ */
