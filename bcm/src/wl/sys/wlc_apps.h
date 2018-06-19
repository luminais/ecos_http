/**
 * AP Powersave state related code
 * This file aims to encapsulating the Power save state of sbc,wlc structure.
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_apps.h,v 1.25.10.4 2010-09-18 00:18:30 Exp $
*/


#ifndef _wlc_apps_h_
#define _wlc_apps_h_

#ifdef WLC_HIGH
#include <wlc_frmutil.h>
#endif


#define DATA_PSQ 1	/* Enumeration to indicate DATA psq */
#define DATA_SUPR 2	/* Suppresed data packets */
#define MGMT_SUPR 3	/* Suppresed and PSQed MGMT packets */
#define DATA_BLOCK_PS		(1 << 3)

#ifdef AP

/* these flags are used when exchanging messages 
 * about PMQ state between BMAC and HIGH
*/
#define TX_FIFO_FLUSHED 0x80
#define MSG_MAC_INVALID 0x40
#define STA_REMOVED 0x20
extern int wlc_apps_process_ps_switch(wlc_info_t *wlc, struct ether_addr *ea, int8 ps_on);
extern void wlc_apps_scb_ps_on(wlc_info_t *wlc, struct scb *scb);
extern void wlc_apps_scb_ps_off(wlc_info_t *wlc, struct scb *scb, bool discard);
extern void wlc_apps_process_pend_ps(wlc_info_t *wlc);
extern void wlc_apps_process_pmqdata(wlc_info_t *wlc, uint32 pmqdata);
extern void wlc_apps_pspoll_resp_prepare(wlc_info_t *wlc, struct scb *scb,
                                         void *pkt, struct dot11_header *h, bool last_frag);
extern void wlc_apps_send_psp_response(wlc_info_t *wlc, struct scb *scb, uint16 fc);

extern int wlc_apps_attach(wlc_info_t *wlc);
extern void wlc_apps_detach(wlc_info_t *wlc);
extern void wlc_apps_tim_create(wlc_info_t *wlc, uchar *tim, int timlen, uint bss_idx);

extern void wlc_apps_psq_ageing(wlc_info_t *wlc);
extern bool wlc_apps_psq(wlc_info_t *wlc, void *pkt, int prec);
struct wlc_bsscfg;
extern void wlc_bss_apps_tbtt_update(struct wlc_bsscfg *cfg);
extern void wlc_apps_tbtt_update(wlc_info_t *wlc);
extern bool wlc_apps_suppr_frame_enq(wlc_info_t *wlc, void *pkt, tx_status_t *txs, bool lastframe);
extern void wlc_apps_ps_prep_mpdu(wlc_info_t *wlc, void *pkt);
extern void wlc_apps_apsd_trigger(wlc_info_t *wlc, struct scb *scb, int ac);
extern void wlc_apps_apsd_prepare(wlc_info_t *wlc, struct scb *scb, void *pkt,
                                  struct dot11_header *h, bool last_frag);
extern void wlc_apps_apsd_delv_stop(wlc_info_t *wlc, struct scb *scb);

extern void wlc_apps_apsd_complete(wlc_info_t *wlc, void *pkt, uint txs);
extern void wlc_apps_psp_resp_complete(wlc_info_t *wlc, void *pkt, uint txs);

#else /* AP */

#define wlc_apps_attach(a) FALSE
#define wlc_apps_psq(a, b, c) FALSE
#define wlc_apps_suppr_frame_enq(a, b, c, d) FALSE

#define wlc_apps_scb_ps_on(a, b) do {} while (0)
#define wlc_apps_scb_ps_off(a, b, c) do {} while (0)
#define wlc_apps_process_pend_ps(a) do {} while (0)

#define wlc_apps_process_pmqdata(a, b) do {} while (0)
#define wlc_apps_pspoll_resp_prepare(a, b, c, d, e) do {} while (0)
#define wlc_apps_send_psp_response(a, b, c) do {} while (0)

#define wlc_apps_detach(a) do {} while (0)
#define wlc_apps_tim_create(a, b, c, d) do {} while (0)
#define wlc_apps_process_ps_switch(a, b, c) do {} while (0)
#define wlc_apps_psq_ageing(a) do {} while (0)
#define wlc_apps_tbtt_update(a) do {} while (0)
#define wlc_apps_ps_prep_mpdu(a, b) do {} while (0)
#define wlc_apps_apsd_trigger(a, b, c) do {} while (0)
#define wlc_apps_apsd_prepare(a, b, c, d, e) do {} while (0)
#define wlc_apps_apsd_delv_stop(a, b) do {} while (0)

#endif /* AP */

#if defined(WLC_HIGH) && defined(MBSS)
extern void wlc_apps_bss_ps_off_done(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
extern void wlc_apps_bss_ps_on_done(wlc_info_t *wlc);
extern void wlc_apps_update_bss_bcmc_fid(wlc_info_t *wlc);
extern int wlc_apps_bcmc_ps_enqueue(wlc_info_t *wlc, struct scb *bcmc_scb, void *pkt);
#else
#define wlc_apps_bss_ps_off_done(wlc, bsscfg)
#define wlc_apps_bss_ps_on_done(wlc)
#define wlc_apps_update_bss_bcmc_fid(wlc)
#endif /* WLC_HIGH && MBSS */

#endif /* _wlc_apps_h_ */
