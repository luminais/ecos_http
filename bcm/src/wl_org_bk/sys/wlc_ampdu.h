/*
 * A-MPDU (with extended Block Ack) related header file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_ampdu.h,v 1.24.2.5 2010-11-24 04:32:01 Exp $
*/


#ifndef _wlc_ampdu_h_
#define _wlc_ampdu_h_

extern ampdu_info_t *wlc_ampdu_attach(wlc_info_t *wlc);
extern void wlc_ampdu_detach(ampdu_info_t *ampdu);
extern bool wlc_ampdu_cap(ampdu_info_t *ampdu);
extern int wlc_ampdu_set(ampdu_info_t *ampdu, bool on);
extern void wlc_ampdu_recvdata(ampdu_info_t *ampdu, struct scb *scb, struct wlc_frminfo *f);
extern void wlc_ampdu_recv_ctl(ampdu_info_t *ampdu, struct scb *scb, uint8 *body,
	int body_len, uint16 fk);
extern void wlc_frameaction_ampdu(ampdu_info_t *ampdu, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
extern int wlc_sendampdu(ampdu_info_t *ampdu, wlc_txq_info_t *qi, void **aggp, int prec);
extern bool wlc_ampdu_dotxstatus(ampdu_info_t *ampdu, struct scb *scb, void *p,
	tx_status_t *txs);
extern void wlc_ampdu_dotxstatus_regmpdu(ampdu_info_t *ampdu, struct scb *scb, void *p,
	tx_status_t *txs);
extern void wlc_ampdu_pkt_free(ampdu_info_t *ampdu, void *p);
extern void wlc_ampdu_reset(ampdu_info_t *ampdu);
extern void wlc_ampdu_macaddr_upd(wlc_info_t *wlc);
extern void wlc_ampdu_shm_upd(ampdu_info_t *ampdu);

extern uint8 wlc_ampdu_null_delim_cnt(ampdu_info_t *ampdu, struct scb *scb,
	ratespec_t rspec, int phylen, uint16* minbytes);
extern bool wlc_ampdu_frameburst_override(ampdu_info_t *ampdu);
extern void scb_ampdu_cleanup(ampdu_info_t *ampdu, struct scb *scb);
extern void scb_ampdu_cleanup_all(ampdu_info_t *ampdu);
#ifdef WLC_HIGH_ONLY
extern void wlc_ampdu_txstatus_complete(ampdu_info_t *ampdu, uint32 s1, uint32 s2);
#endif

extern void wlc_ampdu_agg_state_update_tx(wlc_info_t *wlc, bool txaggr);
extern void wlc_ampdu_agg_state_update_rx(wlc_info_t *wlc, bool rxaggr);

extern void wlc_ampdu_agg_state_override(wlc_info_t *wlc, int8 txaggr, int8 rxaggr);

#define wlc_ampdu_agg_state_update(wlc, tx, rx) \
	do {\
		wlc_ampdu_agg_state_update_tx(wlc, tx); \
		wlc_ampdu_agg_state_update_rx(wlc, rx); \
	} while (0)

extern int wlc_ampdumac_set(ampdu_info_t *ampdu, uint8 on);

#ifdef WLAMPDU_MAC
#define AMU_EPOCH_CHG_PLCP		0	/* epoch change due to plcp */
#define AMU_EPOCH_CHG_FID		1	/* epoch change due to rate flag in frameid */
#define AMU_EPOCH_CHG_NAGG		2	/* epoch change due to ampdu off */
#define AMU_EPOCH_CHG_MPDU		3	/* epoch change due to mpdu */
#define AMU_EPOCH_CHG_DSTTID		4	/* epoch change due to dst/tid */
extern void wlc_ampdu_change_epoch(ampdu_info_t *ampdu, int fifo, int reason_code);
extern void wlc_dump_aggfifo(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif /* WLAMPDU_MAC */
extern void wlc_sidechannel_init(ampdu_info_t *ampdu);

#endif /* _wlc_ampdu_h_ */
