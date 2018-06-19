/*
 * Code that controls the antenna/core/chain
 * Broadcom 802.11bang Networking Device Driver
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: wlc_stf.h,v 1.4.2.14 2011-01-21 20:26:52 Exp $
 */

#ifndef _wlc_stf_h_
#define _wlc_stf_h_

#define AUTO_SPATIAL_EXPANSION	-1
#define MIN_SPATIAL_EXPANSION	0
#define MAX_SPATIAL_EXPANSION	1

extern int wlc_stf_attach(wlc_info_t* wlc);
extern void wlc_stf_detach(wlc_info_t* wlc);

#ifdef WL11N
extern void wlc_pwrthrottle_upd(wlc_info_t *wlc);
extern void wlc_stf_txchain_set_complete(wlc_info_t *wlc);
extern void wlc_stf_tempsense_upd(wlc_info_t *wlc);
extern void wlc_stf_ss_algo_channel_get(wlc_info_t *wlc, uint16 *ss_algo_channel,
	chanspec_t chanspec);
extern int wlc_stf_ss_update(wlc_info_t *wlc, struct wlcband *band);
extern void wlc_stf_phy_txant_upd(wlc_info_t *wlc);
extern int wlc_stf_txchain_set(wlc_info_t* wlc, int32 int_val, bool force, uint16 id);
extern int wlc_stf_txchain_subval_get(wlc_info_t* wlc, uint id, uint *txchain);
extern int wlc_stf_rxchain_set(wlc_info_t* wlc, int32 int_val);
extern bool wlc_stf_stbc_rx_set(wlc_info_t* wlc, int32 int_val);
extern uint8 wlc_stf_txcore_get(wlc_info_t *wlc, ratespec_t rspec);
extern void wlc_stf_spatialpolicy_set_complete(wlc_info_t *wlc);
extern void wlc_stf_txcore_shmem_write(wlc_info_t *wlc, uint16 base, bool forcewr);
extern uint16 wlc_stf_spatial_expansion_get(wlc_info_t *wlc, ratespec_t rspec);
extern uint8 wlc_stf_get_pwrperrate(wlc_info_t *wlc, ratespec_t rspec, uint16 bw,
	uint16 spatial_map);
extern int8 wlc_stf_stbc_rx_get(wlc_info_t* wlc);
#ifdef RXCHAIN_PWRSAVE
extern uint8 wlc_stf_enter_rxchain_pwrsave(wlc_info_t *wlc);
extern void wlc_stf_exit_rxchain_pwrsave(wlc_info_t *wlc, uint8 ht_cap_rx_stbc);
#endif
#else
#define wlc_stf_spatial_expansion_get(a, b) 0
#define wlc_stf_get_pwrperrate(a, b, c, d) 0
#endif /* WL11N */

extern int wlc_stf_ant_txant_validate(wlc_info_t *wlc, int8 val);
extern void wlc_stf_phy_txant_upd(wlc_info_t *wlc);
extern void wlc_stf_phy_chain_calc(wlc_info_t *wlc);
extern uint16 wlc_stf_phytxchain_sel(wlc_info_t *wlc, ratespec_t rspec);
extern uint16 wlc_stf_d11hdrs_phyctl_txant(wlc_info_t *wlc, ratespec_t rspec);
extern void wlc_update_txppr_offset(wlc_info_t *wlc, int8 *txpwr);
#endif /* _wlc_stf_h_ */
