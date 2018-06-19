/*
 * Common OS-independent driver header file for rate selection
 * algorithm of Broadcom 802.11b DCF-only Networking Adapter.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 *
 * $Id: wlc_rate_sel.h,v 1.50 2010-01-26 03:32:27 Exp $
 */

#ifndef	_WLC_RATE_SEL_H_
#define	_WLC_RATE_SEL_H_

#ifdef WLAFTERBURNER
#define ABURN_NACK_WEIGHT	2	/* afterburner NACK */
#define ABURN_RRTS_WEIGHT	1	/* afterburner re-tran RTS */
#define ABURN_DATA_WEIGHT	0	/* successful data pkt transmission */
#endif /* WLAFTERBURNER */

/* flags returned by wlc_ratesel_gettxrate() */
#define RATESEL_VRATE_PROBE	0x0001	/* vertical rate probe pkt; use small ampdu */

#ifdef WLLMAC
/* LMAC supports AC_COUNT as the MAX traffic classes */
#define WME_MAX_AC(wlc, scb) 	1
#else
#define WME_MAX_AC(wlc, scb) ((WME_PER_AC_MAXRATE_ENAB((wlc)->pub) && SCB_WME(scb)) ? \
				AC_COUNT : 1)
#endif

extern ratesel_info_t *wlc_ratesel_attach(wlc_info_t *wlc);
extern void wlc_ratesel_detach(ratesel_info_t *rsi);


/* initialize per-scb state utilized by rate selection */
extern void wlc_ratesel_init(ratesel_info_t *rsi, struct scb *scb, wlc_rateset_t *rateset,
	bool is40bw, int8 sgi_tx,
#ifdef WLAFTERBURNER
	bool aburn,
#endif /* WLAFTERBURNER */
	uint *clronupd, uint8 active_antcfg_num,
	uint8 antselid_init, int32 ac);

/* update per-scb state upon received tx status */
extern void wlc_ratesel_upd_txstatus_normalack(ratesel_info_t *rsi, struct scb *scb,
	tx_status_t *txs, uint16 sfbl, uint16 lfbl,
#ifdef WLAFTERBURNER
	bool aburn, uint8 aburn_weight,
#endif
	uint8 mcs, uint8 antselid, bool fbr);

#ifdef WL11N
/* change the throughput-based algo parameters upon ACI mitigation state change */
extern void wlc_ratesel_aci_change(ratesel_info_t *rsi, bool aci_state);

/* update per-scb state upon received tx status for ampdu */
extern void wlc_ratesel_upd_txs_blockack(ratesel_info_t *rsi, struct scb *scb,
	tx_status_t *txs, uint8 suc_mpdu, uint8 tot_mpdu,
	bool ba_lost, uint8 retry, uint8 fb_lim, bool tx_error,
	uint8 mcs, uint8 antselid);

#ifdef WLAMPDU_MAC
extern void wlc_ratesel_upd_txs_ampdu(ratesel_info_t *rsi, struct scb *scb, uint16 frameid,
	uint8 mrt, uint8 mrt_succ, uint8 fbr, uint8 fbr_succ,
	bool tx_error, uint8 tx_mcs, uint8 antselid);
#endif

/* update rate_sel if a PPDU (ampdu or a reg pkt) is created with probe values */
extern void wlc_ratesel_probe_ready(ratesel_info_t *rsi, struct scb *scb,
	uint16 frameid, bool is_ampdu, uint8 ampdu_txretry);

extern void wlc_ratesel_upd_rxstats(ratesel_info_t *rsi, ratespec_t rx_rspec, uint16 rxstatus2);
#endif /* WL11N */

/* select transmit rate given per-scb state */
extern void wlc_ratesel_gettxrate(ratesel_info_t *rsi, struct scb *scb,
	uint16 *frameid, ratespec_t *rate, ratespec_t *fbrate, uint8 *antselid,
	uint8 *fbantselid, uint16 *flags);

/* get the fallback rate of the specified mcs rate */
extern ratespec_t wlc_ratesel_getmcsfbr(ratesel_info_t *rsi, struct scb *scb,
	uint16 frameid, uint8 mcs);

extern bool wlc_ratesel_minrate(ratesel_info_t *rsi, struct scb *scb, tx_status_t *txs);

#define RATESEL_MSG_INFO_VAL	0x01 /* concise rate change msg in addition to WL_RATE */
#define RATESEL_MSG_MORE_VAL	0x02 /* verbose rate change msg */
#define RATESEL_MSG_SP0_VAL	0x04 /* concise spatial/tx_antenna probing msg */
#define RATESEL_MSG_SP1_VAL	0x08 /* verbose spatial/tx_antenna probing msg */
#define RATESEL_MSG_RXA0_VAL	0x10 /* concise rx atenna msg */
#define RATESEL_MSG_RXA1_VAL	0x20 /* verbose rx atenna msg */
#endif	/* _WLC_RATE_H_ */
