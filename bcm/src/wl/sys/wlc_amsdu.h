/*
 * MSDU aggregation related header file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_amsdu.h,v 1.22.88.1 2010-04-21 03:57:03 Exp $
*/


#ifndef _wlc_amsdu_h_
#define _wlc_amsdu_h_

extern amsdu_info_t *wlc_amsdu_attach(wlc_info_t *wlc);
extern void wlc_amsdu_detach(amsdu_info_t *ami);
extern bool wlc_amsdutx_cap(amsdu_info_t *ami);
extern bool wlc_amsdurx_cap(amsdu_info_t *ami);
extern uint16 wlc_amsdu_mtu_get(amsdu_info_t *ami);
extern int wlc_amsdu_set(amsdu_info_t *ami, bool val);

extern void wlc_recvamsdu(amsdu_info_t *ami, wlc_d11rxhdr_t *wrxh, void *p);
extern void wlc_amsdu_deagg_hw(amsdu_info_t *ami, struct scb *scb,
	struct wlc_frminfo *f, char *prx_ctxt, int len_rx_ctxt);
#ifdef WLAMSDU_SWDEAGG
extern void wlc_amsdu_deagg_sw(amsdu_info_t *ami, struct scb *scb,
	struct wlc_frminfo *f, char *prx_ctxt, int len_rx_ctxt);
#endif

#ifdef WLAMSDU_TX
extern void wlc_amsdu_agglimit_frag_upd(amsdu_info_t *ami);
extern void wlc_amsdu_txop_upd(amsdu_info_t *ami);
extern void wlc_amsdu_scb_agglimit_upd(amsdu_info_t *ami, struct scb *scb);
extern void wlc_amsdu_dotxstatus(amsdu_info_t *ami, struct scb *scb, void *p);
extern void wlc_amsdu_txpolicy_upd(amsdu_info_t *ami);
#else /* WLAMSDU_TX */
#define wlc_amsdu_agglimit_frag_upd(a)		do {} while (0)
#define wlc_amsdu_txop_upd(a)			do {} while (0)
#define wlc_amsdu_scb_agglimit_upd(a, b)	do {} while (0)
#define wlc_amsdu_dotxstatus(a, b, c)		do {} while (0)
#define wlc_amsdu_txpolicy_upd(a)		do {} while (0)
#endif /* WLAMSDU_TX */

#endif /* _wlc_amsdu_h_ */
