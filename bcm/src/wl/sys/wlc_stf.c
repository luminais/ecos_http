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
 * $Id: wlc_stf.c,v 1.9.2.87 2011-02-10 02:16:45 Exp $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <bcmwifi.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_ap.h>
#include <wlc_scb.h>
#include <wlc_frmutil.h>
#include <wl_export.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <wlc_stf.h>

/* this macro define all PHYs REV that can NOT receive STBC with one RX core active */
#define WLC_STF_NO_STBC_RX_1_CORE(wlc) (WLCISNPHY(wlc->band) && \
	NREV_GT(wlc->band->phyrev, 3) && NREV_LE(wlc->band->phyrev, 6))
/* this macro define all PHYs REV that has Multiple Input/Output Capabilities */
#define WLCISMIMO (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band))

#define WLC_TEMPSENSE_PERIOD		10	/* 10 second timeout */


#ifdef WL11N
static int wlc_stf_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_stf_ss_get(wlc_info_t* wlc, int8 band);
static bool wlc_stf_ss_set(wlc_info_t* wlc, int32 int_val, int8 band);
static bool wlc_stf_ss_auto(wlc_info_t* wlc);
static int  wlc_stf_ss_auto_set(wlc_info_t* wlc, bool enable);
static int8 wlc_stf_mimops_get(wlc_info_t* wlc);
static bool wlc_stf_mimops_set(wlc_info_t* wlc, wlc_bsscfg_t *cfg, int32 int_val);
static int8 wlc_stf_stbc_tx_get(wlc_info_t* wlc);
static bool wlc_stf_stbc_tx_set(wlc_info_t* wlc, int32 int_val);
static void wlc_stf_txcore_set(wlc_info_t *wlc, uint8 idx, uint8 val);
#if defined(WLC_LOW)
static int  wlc_stf_txchain_pwr_offset_set(wlc_info_t *wlc, wl_txchain_pwr_offsets_t *offsets);
#endif
static void wlc_stf_stbc_rx_ht_update(wlc_info_t *wlc, int val);
static uint8 wlc_stf_core_mask_assign(wlc_stf_t *stf, uint core_count);
static uint8 wlc_stf_uint8_vec_max(uint8 *vec, uint count);
static int wlc_stf_get_ppr_offset(wlc_info_t *wlc, wl_txppr_t *ppr_buf);
static int wlc_stf_spatial_mode_upd(wlc_info_t *wlc, int8 *mode);
static int wlc_stf_spatial_policy_set(wlc_info_t *wlc, int val);
#ifdef BCMDBG
static int wlc_stf_pproff_shmem_get(wlc_info_t *wlc, int *retval);
static void wlc_stf_pproff_shmem_set(wlc_info_t *wlc, uint8 rate, uint8 val);
#endif /* BCMDBG */
#endif /* WL11N */

static void _wlc_stf_phy_txant_upd(wlc_info_t *wlc);
static uint16 _wlc_stf_phytxchain_sel(wlc_info_t *wlc, ratespec_t rspec);

const uint8 txcore_default[MAX_CORE_IDX][3] = {
	{1, 0x01, 0x02},	/* CCK */
	{1, 0x01, 0x02},	/* OFDM */
	{1, 0x01, 0x02},	/* For Nsts = 1, enable core 1 */
	{2, 0x03, 0x06},	/* For Nsts = 2, enable core 1 & 2 */
	{3, 0x07, 0x07},	/* For Nsts = 3, enable core 1, 2 & 3 */
	{4, 0x0F, 0x0F} 	/* For Nsts = 4, enable all cores */
};

enum {
	CCK_1M_IDX = 0,
	CCK_2M_IDX,
	CCK_5M5_IDX,
	CCK_11M_IDX
};

const uint8 cck_pwr_idx_table[12] = {
	0x80,
	CCK_1M_IDX,
	CCK_2M_IDX,
	0x80,
	0x80,
	CCK_5M5_IDX,
	CCK_5M5_IDX,
	0x80,
	0x80,
	0x80,
	0x80,
	CCK_11M_IDX
};

enum {
	OFDM_6M_IDX = 0,
	OFDM_9M_IDX,
	OFDM_12M_IDX,
	OFDM_18M_IDX,
	OFDM_24M_IDX,
	OFDM_36M_IDX,
	OFDM_48M_IDX,
	OFDM_54M_IDX
};

const uint8 ofdm_pwr_idx_table[19] = {
	0x80,
	0x80,
	OFDM_6M_IDX,
	OFDM_9M_IDX,
	OFDM_12M_IDX,
	0x80,
	OFDM_18M_IDX,
	0x80,
	OFDM_24M_IDX,
	0x80,
	0x80,
	0x80,
	OFDM_36M_IDX,
	0x80,
	0x80,
	0x80,
	OFDM_48M_IDX,
	0x80,
	OFDM_54M_IDX
};

/* iovar table */
enum {
	IOV_STF_SS,		/* MIMO STS coding for single stream mcs or legacy ofdm rates */
	IOV_STF_SS_AUTO,	/* auto selection of channel-based */
				/* MIMO STS coding for single stream mcs */
				/* OR LEGACY ofdm rates */
	IOV_STF_MIMOPS,		/* MIMO power savw mode */
	IOV_STF_STBC_RX,	/* MIMO, STBC RX */
	IOV_STF_STBC_TX,	/* MIMO, STBC TX */
	IOV_STF_TXSTREAMS,	/* MIMO, tx stream */
	IOV_STF_TXCHAIN,	/* MIMO, tx chain */
	IOV_STF_SISO_TX,	/* MIMO, SISO TX */
	IOV_STF_HW_TXCHAIN,	/* MIMO, HW tx chain */
	IOV_STF_RXSTREAMS,	/* MIMO, rx stream */
	IOV_STF_RXCHAIN,	/* MIMO, rx chain */
	IOV_STF_HW_RXCHAIN,	/* MIMO, HW rx chain */
	IOV_STF_TXCORE,		/* MIMO, tx core enable and selected */
	IOV_STF_TXCORE_OVRD,
	IOV_STF_SPATIAL_MODE,	/* spatial policy to use */
	IOV_STF_TEMPS_DISABLE,
	IOV_STF_TXCHAIN_PWR_OFFSET,
	IOV_STF_RSSI_PWRDN,
	IOV_STF_PPR_OFFSET,
	IOV_STF_PWR_THROTTLE_TEST, /* testing */
	IOV_STF_PWR_THROTTLE_MASK, /* core to enable/disable when thromal throttling kicks in */
	IOV_STF_PWR_THROTTLE,
	IOV_STF_PWR_THROTTLE_STATE,
	IOV_STF_RATETBL_PPR,
	IOV_STF_LAST
};

static const bcm_iovar_t stf_iovars[] = {
	{"mimo_ps", IOV_STF_MIMOPS,
	(IOVF_OPEN_ALLOW), IOVT_UINT8, 0
	},
	{"mimo_ss_stf", IOV_STF_SS,
	(IOVF_OPEN_ALLOW), IOVT_INT8, 0
	},
	{"stf_ss_auto", IOV_STF_SS_AUTO,
	(0), IOVT_INT8, 0
	},
	{"stbc_rx", IOV_STF_STBC_RX,
	(IOVF_OPEN_ALLOW), IOVT_UINT8, 0
	},
	{"stbc_tx", IOV_STF_STBC_TX,
	(IOVF_OPEN_ALLOW), IOVT_INT8, 0
	},
	{"siso_tx", IOV_STF_SISO_TX,
	(0), IOVT_BOOL, 0
	},
	{"txstreams", IOV_STF_TXSTREAMS,
	(0), IOVT_UINT8, 0
	},
	{"txchain", IOV_STF_TXCHAIN,
	(0), IOVT_UINT8, 0
	},
	{"hw_txchain", IOV_STF_HW_TXCHAIN,
	(0), IOVT_UINT8, 0
	},
	{"rxstreams", IOV_STF_RXSTREAMS,
	(0), IOVT_UINT8, 0
	},
	{"hw_rxchain", IOV_STF_HW_RXCHAIN,
	(0), IOVT_UINT8, 0
	},
	{"rxchain", IOV_STF_RXCHAIN,
	(0), IOVT_UINT8, 0
	},
	{"txcore", IOV_STF_TXCORE,
	(0), IOVT_BUFFER,  sizeof(uint32)*2
	},
	{"txcore_override", IOV_STF_TXCORE_OVRD,
	(0), IOVT_BUFFER,  sizeof(uint32)*2
	},
	{"tempsense_disable", IOV_STF_TEMPS_DISABLE,
	(0), IOVT_BOOL, 0
	},
	{"txchain_pwr_offset", IOV_STF_TXCHAIN_PWR_OFFSET,
	(0), IOVT_BUFFER, sizeof(wl_txchain_pwr_offsets_t),
	},
	{"rssi_pwrdn_disable", IOV_STF_RSSI_PWRDN,
	(0), IOVT_BOOL, 0
	},
	{"curppr", IOV_STF_PPR_OFFSET,
	(0), IOVT_BUFFER, 0
	},
	{"pwrthrottle_test", IOV_STF_PWR_THROTTLE_TEST,
	0, IOVT_UINT8, 0
	},
	{"pwrthrottle_mask", IOV_STF_PWR_THROTTLE_MASK,
	0, IOVT_UINT8, 0
	},
	{"pwrthrottle", IOV_STF_PWR_THROTTLE,
	0, IOVT_INT32, 0
	},
	{"pwrthrottle_state", IOV_STF_PWR_THROTTLE_STATE,
	0, IOVT_INT32, 0
	},
	{"spatial_policy", IOV_STF_SPATIAL_MODE,
	0, IOVT_BUFFER, (sizeof(int) * SPATIAL_MODE_MAX_IDX)
	},
	{"ratetbl_ppr", IOV_STF_RATETBL_PPR,
	0, IOVT_BUFFER, (sizeof(int) * 12)
	},
	{NULL, 0, 0, 0, 0}
};

#ifdef WL11N
/* handle STS related iovars */
static int
wlc_stf_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	int err = 0;
	wlc_bsscfg_t *bsscfg;

	/* lookup bsscfg from provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	ret_int_ptr = (int32 *)a;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	if (plen >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)p + sizeof(int_val)), &int_val2, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
		case IOV_GVAL(IOV_STF_SS):
			*ret_int_ptr = wlc_stf_ss_get(wlc, (int8)int_val);
			break;

		case IOV_SVAL(IOV_STF_SS):
			if (!wlc_stf_ss_set(wlc, int_val, (int8)int_val2)) {
				err = BCME_RANGE;
			}
			break;

		case IOV_GVAL(IOV_STF_SS_AUTO):
			*ret_int_ptr = (int)wlc_stf_ss_auto(wlc);
			break;

		case IOV_SVAL(IOV_STF_SS_AUTO):
			err = wlc_stf_ss_auto_set(wlc, bool_val);
			break;

		case IOV_GVAL(IOV_STF_MIMOPS):
			*ret_int_ptr = wlc_stf_mimops_get(wlc);
			break;

		case IOV_SVAL(IOV_STF_MIMOPS):
			if (!wlc_stf_mimops_set(wlc, bsscfg, int_val)) {
				err = BCME_RANGE;
			}
			break;

		case IOV_GVAL(IOV_STF_SISO_TX):
			*ret_int_ptr = wlc->stf->siso_tx;
			break;

		case IOV_SVAL(IOV_STF_SISO_TX):
			if  (wlc->stf->siso_tx  == bool_val)
				break;
			else
			{
				wlc->stf->siso_tx = bool_val;
			}

			wlc_stf_ss_auto_set(wlc, !bool_val);
			if (!wlc_stf_ss_set(wlc, !bool_val, -1)) {
				err = BCME_RANGE;
			}
			else
				wlc_scb_ratesel_init_all(wlc);
			break;

		case IOV_GVAL(IOV_STF_STBC_TX):
			*ret_int_ptr = wlc_stf_stbc_tx_get(wlc);
			break;

		case IOV_SVAL(IOV_STF_STBC_TX):
			if (!WLC_STBC_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			if (!wlc_stf_stbc_tx_set(wlc, int_val))
				err = BCME_RANGE;

			break;

		case IOV_GVAL(IOV_STF_STBC_RX):
#ifdef RXCHAIN_PWRSAVE
			/* need to get rx_stbc HT capability from saved value if
			 * in rxchain_pwrsave mode
			 */
			*ret_int_ptr = wlc_rxchain_pwrsave_stbc_rx_get(wlc);
#else
			*ret_int_ptr = wlc_stf_stbc_rx_get(wlc);
#endif
			break;

		case IOV_SVAL(IOV_STF_STBC_RX):
			if (!WLC_STBC_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}
#ifdef RXCHAIN_PWRSAVE
			/* need to exit rxchain_pwrsave mode(turn on all RXCHAIN)
			 * before enable STBC_RX
			 */
			/* PHY cannot receive STBC with only one rx core active */
			if (WLC_STF_NO_STBC_RX_1_CORE(wlc)) {
				if (wlc->ap != NULL)
					wlc_reset_rxchain_pwrsave_mode(wlc->ap);
			}
#endif /* RXCHAIN_PWRSAVE */
			if (!wlc_stf_stbc_rx_set(wlc, int_val))
				err = BCME_RANGE;

			break;

		case IOV_GVAL(IOV_STF_TXSTREAMS):
			*ret_int_ptr = wlc->stf->txstreams;
			break;

		case IOV_GVAL(IOV_STF_TXCHAIN):
			*ret_int_ptr = wlc->stf->txchain;
			break;

		case IOV_SVAL(IOV_STF_TXCHAIN):
			if (int_val == -1)
				int_val = wlc->stf->hw_txchain;

			err = wlc_stf_txchain_set(wlc, int_val, FALSE, WLC_TXCHAIN_ID_USR);
			break;

		case IOV_GVAL(IOV_STF_HW_TXCHAIN):
			*ret_int_ptr = wlc->stf->hw_txchain;
			break;

		case IOV_GVAL(IOV_STF_RXSTREAMS):
			*ret_int_ptr = (uint8)WLC_BITSCNT(wlc->stf->rxchain);
			break;

		case IOV_GVAL(IOV_STF_RXCHAIN):
			/* use SW Rx chain state */
			*ret_int_ptr = wlc->stf->rxchain;
			break;

		case IOV_GVAL(IOV_STF_HW_RXCHAIN):
			*ret_int_ptr = wlc->stf->hw_rxchain;
			break;

		case IOV_SVAL(IOV_STF_RXCHAIN):
			/* don't allow setting of rxchain if STA is
			 * associated, due to supported rateset needs
			 * to be updated
			 */
			if (wlc->pub->associated)
				return BCME_ASSOCIATED;

#ifdef RXCHAIN_PWRSAVE
			if (RXCHAIN_PWRSAVE_ENAB(wlc->ap)) {
				if (int_val == wlc->stf->hw_rxchain)
					wlc_reset_rxchain_pwrsave_mode(wlc->ap);
				else
					wlc_disable_rxchain_pwrsave(wlc->ap);
			}
#endif
			err = wlc_stf_rxchain_set(wlc, int_val);
			/* Update op_rxstreams */
			wlc->stf->op_rxstreams = wlc->stf->rxstreams;

			break;

		case IOV_GVAL(IOV_STF_TXCORE):
		{
			uint32 core[2] = {0, 0};

			core[0] |= wlc->stf->txcore[NSTS4_IDX][1] << 24;
			core[0] |= wlc->stf->txcore[NSTS3_IDX][1] << 16;
			core[0] |= wlc->stf->txcore[NSTS2_IDX][1] << 8;
			core[0] |= wlc->stf->txcore[NSTS1_IDX][1];
			core[1] |= wlc->stf->txcore[OFDM_IDX][1] << 8;
			core[1] |= wlc->stf->txcore[CCK_IDX][1];
			bcopy(core, a, sizeof(uint32)*2);
			break;
		}

		case IOV_SVAL(IOV_STF_TXCORE):
		{
			uint32 core[2];
			uint8 i, Nsts = 0;
			uint8 txcore_ovrd[MAX_CORE_IDX] = {0, 0, 0, 0, 0, 0};

			if (!WLCISHTPHY(wlc->band)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			bcopy(p, core, sizeof(uint32)*2);
			if (core[0] == 0 && core[1] == 0) {
				bzero(wlc->stf->txcore_user, MAX_CORE_IDX);
				wlc_stf_spatial_policy_set(wlc, wlc->stf->spatialpolicy);
				break;
			}

			/* core[0] contains mcs txcore mask setting
			 * core[1] contains cck & ofdm txcore mask setting
			 */
			if (core[0]) {
				for (i = 0; i < 4; i++) {
					Nsts = ((uint8)core[0] & 0x0f);
					if (Nsts > MAX_STREAMS_SUPPORTED) {
						WL_ERROR(("wl%d: %s: Nsts(%d) out of range\n",
							wlc->pub->unit, __FUNCTION__, Nsts));
						return BCME_RANGE;
					}
					txcore_ovrd[Nsts+OFDM_IDX] =
						((uint8)core[0] & 0xf0) >> 4;

					if (Nsts > WLC_BITSCNT(txcore_ovrd[Nsts+OFDM_IDX])) {
						WL_ERROR(("wl%d: %s: Nsts(%d) >"
							" # of core enabled (0x%x)\n",
							wlc->pub->unit, __FUNCTION__,
							Nsts, txcore_ovrd[Nsts+OFDM_IDX]));
						return BCME_BADARG;
					}
					core[0] >>= 8;
				}
			}
			if (core[1]) {
				txcore_ovrd[CCK_IDX] = core[1] & 0x0f;
				if (WLC_BITSCNT(txcore_ovrd[CCK_IDX]) > wlc->stf->txstreams) {
					WL_ERROR(("wl%d: %s: cck core (0x%x) > HW core (0x%x)\n",
						wlc->pub->unit, __FUNCTION__,
						txcore_ovrd[CCK_IDX], wlc->stf->hw_txchain));
					return BCME_BADARG;
				}
				txcore_ovrd[OFDM_IDX] = (core[1] >> 8) & 0x0f;
				if (WLC_BITSCNT(txcore_ovrd[OFDM_IDX]) > wlc->stf->txstreams) {
					WL_ERROR(("wl%d: %s: ofdm core (0x%x) > HW core (0x%x)\n",
						wlc->pub->unit, __FUNCTION__,
						txcore_ovrd[OFDM_IDX], wlc->stf->hw_txchain));
					return BCME_BADARG;
				}
			}

			bcopy(txcore_ovrd, wlc->stf->txcore_user, MAX_CORE_IDX);
			wlc_stf_spatial_policy_set(wlc, wlc->stf->spatialpolicy);
			break;
		}

		case IOV_GVAL(IOV_STF_TXCORE_OVRD):
		{
			uint32 core[2] = {0, 0};

			core[0] |= wlc->stf->txcore_user[NSTS4_IDX] << 24;
			core[0] |= wlc->stf->txcore_user[NSTS3_IDX] << 16;
			core[0] |= wlc->stf->txcore_user[NSTS2_IDX] << 8;
			core[0] |= wlc->stf->txcore_user[NSTS1_IDX];
			core[1] |= wlc->stf->txcore_user[OFDM_IDX] << 8;
			core[1] |= wlc->stf->txcore_user[CCK_IDX];
			bcopy(core, a, sizeof(uint32)*2);
			break;
		}

		case IOV_GVAL(IOV_STF_TEMPS_DISABLE):
			*ret_int_ptr = wlc->stf->tempsense_disable;
			break;

		case IOV_SVAL(IOV_STF_TEMPS_DISABLE):
			wlc->stf->tempsense_disable = (uint8)int_val;
			break;

#if defined(WLC_LOW) && defined(WL11N)
		case IOV_GVAL(IOV_STF_TXCHAIN_PWR_OFFSET):
			memcpy(a, &wlc->stf->txchain_pwr_offsets, sizeof(wl_txchain_pwr_offsets_t));
			break;

		case IOV_SVAL(IOV_STF_TXCHAIN_PWR_OFFSET):
			err = wlc_stf_txchain_pwr_offset_set(wlc, (wl_txchain_pwr_offsets_t*)p);
			break;
#endif /* defined(WLC_LOW) && defined(WL11N) */
		case IOV_GVAL(IOV_STF_RSSI_PWRDN):
			*ret_int_ptr = (int)wlc->stf->rssi_pwrdn_disable;
			break;

		case IOV_SVAL(IOV_STF_RSSI_PWRDN):
			wlc->stf->rssi_pwrdn_disable = bool_val;
			wlc_mhf(wlc, MHF5, MHF5_HTPHY_RSSI_PWRDN,
				(bool_val ? MHF5_HTPHY_RSSI_PWRDN : 0), WLC_BAND_ALL);
			break;

		case IOV_GVAL(IOV_STF_PPR_OFFSET):
		{
			wl_txppr_t *pbuf = (wl_txppr_t *)p;

			if (pbuf->len < WL_TXPPR_LENGTH || plen < sizeof(wl_txppr_t))
				return BCME_BUFTOOSHORT;

			if (pbuf->ver != WL_TXPPR_VERSION)
				return BCME_VERSION;

			if (alen < (int)sizeof(wl_txppr_t))
				return BCME_BADLEN;

			pbuf = (wl_txppr_t *)a;
			err = wlc_stf_get_ppr_offset(wlc, pbuf);
			break;
		}

		case IOV_GVAL(IOV_STF_PWR_THROTTLE_TEST):
			*ret_int_ptr = (int)wlc->stf->pwr_throttle_test;
			break;

		case IOV_SVAL(IOV_STF_PWR_THROTTLE_TEST):
			if (int_val < 0 || int_val >= wlc->stf->hw_txchain)
				return BCME_RANGE;
			wlc->stf->pwr_throttle_test = (uint8) int_val;
			break;

		case IOV_GVAL(IOV_STF_PWR_THROTTLE_MASK):
			*ret_int_ptr = (int)wlc->stf->pwr_throttle_mask;
			break;

		case IOV_SVAL(IOV_STF_PWR_THROTTLE_MASK):
			if (int_val < 0 || int_val > wlc->stf->hw_txchain)
				return BCME_RANGE;
			wlc->stf->pwr_throttle_mask = (uint8) int_val;
			break;

		case IOV_GVAL(IOV_STF_PWR_THROTTLE):
			*ret_int_ptr = wlc->stf->pwr_throttle;
			break;

		case IOV_SVAL(IOV_STF_PWR_THROTTLE):
			if (int_val != OFF && int_val != ON &&
			    int_val != AUTO)
				return BCME_RANGE;
			wlc->stf->pwr_throttle = int_val;
			break;

		case IOV_GVAL(IOV_STF_PWR_THROTTLE_STATE):
			*ret_int_ptr = wlc->stf->pwr_throttle_state;
			break;

		case IOV_GVAL(IOV_STF_SPATIAL_MODE):
		{
			int i, *ptr = (int *)a;
			if (alen < (int)(sizeof(int) * SPATIAL_MODE_MAX_IDX))
				return BCME_BUFTOOSHORT;
			for (i = 0; i < SPATIAL_MODE_MAX_IDX; i++)
				ptr[i] = (int)wlc->stf->spatial_mode_config[i];
			break;
		}

		case IOV_SVAL(IOV_STF_SPATIAL_MODE):
		{
			int i;
			int8 mode[SPATIAL_MODE_MAX_IDX];
			int *ptr = (int *)p;

			if (plen < (sizeof(int) * SPATIAL_MODE_MAX_IDX))
				return BCME_BUFTOOSHORT;
			for (i = 0; i < SPATIAL_MODE_MAX_IDX; i++) {
				mode[i] = (int8)ptr[i];
				if (mode[i] != ON && mode[i] != OFF && mode[i] != AUTO)
					return BCME_RANGE;
			}
		        err = wlc_stf_spatial_mode_upd(wlc, mode);
			break;
		}
#ifdef BCMDBG
		case IOV_GVAL(IOV_STF_RATETBL_PPR):
		{
			int *ptr = (int *)a;
			if (alen < (int)(sizeof(int) * 12))
				return BCME_BUFTOOSHORT;

			bzero(ptr, (sizeof(int) * 12));
			err = wlc_stf_pproff_shmem_get(wlc, ptr);
			break;
		}

		case IOV_SVAL(IOV_STF_RATETBL_PPR):
		{
			uint8 rate, val;
			int *ptr = (int *)a;
			rate = (ptr[0] & 0xff);
			val = (ptr[1] & 0xff);
			wlc_stf_pproff_shmem_set(wlc, rate, val);
			break;
		}
#endif /* BCMDBG */
		default:
			err = BCME_UNSUPPORTED;
	}
	return err;
}

static void
wlc_stf_stbc_rx_ht_update(wlc_info_t *wlc, int val)
{
	ASSERT((val == HT_CAP_RX_STBC_NO) || (val == HT_CAP_RX_STBC_ONE_STREAM));

	/* PHY cannot receive STBC with only one rx core active */
	if (WLC_STF_NO_STBC_RX_1_CORE(wlc)) {
		if ((wlc->stf->rxstreams == 1) && (val != HT_CAP_RX_STBC_NO))
			return;
	}

	wlc->ht_cap.cap &= ~HT_CAP_RX_STBC_MASK;
	wlc->ht_cap.cap |= (val << HT_CAP_RX_STBC_SHIFT);

	if (wlc->pub->up) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}
}

#if defined(BCMDBG_DUMP)
static void
wlc_dump_stf(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint i;
	wlc_stf_t *stf = wlc->stf;

	bcm_bprintf(b, "STF dump:\n");

	bcm_bprintf(b, "hw_txchain 0X%x hw_rxchain 0x%x\n",
		wlc->stf->hw_txchain, wlc->stf->hw_rxchain);
	bcm_bprintf(b, "txchain 0x%x txstreams %d \nrxchain 0x%x rxstreams %d\n",
		wlc->stf->txchain, wlc->stf->txstreams, wlc->stf->rxchain, wlc->stf->rxstreams);
	bcm_bprintf(b, "txchain subvals:\n");
	for (i = 0; i < WLC_TXCHAIN_ID_COUNT; i++)
		bcm_bprintf(b, "\tid:%d 0x%x\n", i, stf->txchain_subval[i]);

	bcm_bprintf(b, "txant %d ant_rx_ovr %d phytxant 0x%x\n",
		wlc->stf->txant, wlc->stf->ant_rx_ovr, wlc->stf->phytxant);
	bcm_bprintf(b, "txcore CCK %d, OFDM %d, MCS Nsts[4..1] %d %d %d %d\n",
		wlc->stf->txcore[CCK_IDX][1], wlc->stf->txcore[OFDM_IDX][1],
		wlc->stf->txcore[NSTS4_IDX][1], wlc->stf->txcore[NSTS3_IDX][1],
		wlc->stf->txcore[NSTS2_IDX][1], wlc->stf->txcore[NSTS1_IDX][1]);
	bcm_bprintf(b, "stf_ss_auto: %d ss_opmode %d ss_algo_channel 0x%x no_cddstbc %d\n",
		wlc->stf->ss_algosel_auto, wlc->stf->ss_opmode, wlc->stf->ss_algo_channel,
		wlc->stf->no_cddstbc);
	bcm_bprintf(b, "op_stf_ss: %s", wlc->band->band_stf_stbc_tx == AUTO ? "STBC" :
		(wlc->stf->ss_opmode == PHY_TXC1_MODE_SISO ? "SISO" : "CDD"));

	for (i = 0; i < NBANDS(wlc); i++) {
		bcm_bprintf(b, "\tband%d stf_ss: %s ", i,
			wlc->bandstate[i]->band_stf_stbc_tx == AUTO ? "STBC" :
			(wlc->bandstate[i]->band_stf_ss_mode == PHY_TXC1_MODE_SISO ?
			"SISO" : "CDD"));
	}

	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "ipaon %d", wlc->stf->ipaon);

	bcm_bprintf(b, "pwr_throttle_mask 0X%x pwr_throttle_test 0x%x\n",
		wlc->stf->pwr_throttle_mask, wlc->stf->pwr_throttle_test);

	bcm_bprintf(b, "\n");
}

static void
wlc_dump_ppr(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint i;
	uint n = WLCISHTPHY(wlc->band) ? WL_NUM_3x3_ELEMENTS : WL_NUM_2x2_ELEMENTS;
	uint offset = WLCISHTPHY(wlc->band) ? WL_NUM_3x3_ELEMENTS : 0;
	uint8 *ptr;
	const char *str = "";

	bcm_bprintf(b, "Power/Rate Dump (in 1/2dB): Channel %d\n", CHSPEC_CHANNEL(wlc->chanspec));
	bcm_bprintf(b, "20MHz:\n");
	if (CHSPEC_IS2G(wlc->home_chanspec)) {
		bcm_bprintf(b, "CCK          %2d %2d %2d %2d\n",
			wlc->stf->txpwr_ctl.cck[0], wlc->stf->txpwr_ctl.cck[1],
			wlc->stf->txpwr_ctl.cck[2], wlc->stf->txpwr_ctl.cck[3]);
	}
	bcm_bprintf(b, "OFDM         %2d %2d %2d %2d %2d %2d %2d %2d\n",
		wlc->stf->txpwr_ctl.ofdm[0], wlc->stf->txpwr_ctl.ofdm[1],
		wlc->stf->txpwr_ctl.ofdm[2], wlc->stf->txpwr_ctl.ofdm[3],
		wlc->stf->txpwr_ctl.ofdm[4], wlc->stf->txpwr_ctl.ofdm[5],
		wlc->stf->txpwr_ctl.ofdm[6], wlc->stf->txpwr_ctl.ofdm[7]);
	bcm_bprintf(b, "OFDM-CDD     %2d %2d %2d %2d %2d %2d %2d %2d\n",
		wlc->stf->txpwr_ctl.ofdm_cdd[0], wlc->stf->txpwr_ctl.ofdm_cdd[1],
		wlc->stf->txpwr_ctl.ofdm_cdd[2], wlc->stf->txpwr_ctl.ofdm_cdd[3],
		wlc->stf->txpwr_ctl.ofdm_cdd[4], wlc->stf->txpwr_ctl.ofdm_cdd[5],
		wlc->stf->txpwr_ctl.ofdm_cdd[6], wlc->stf->txpwr_ctl.ofdm_cdd[7]);
	for (i = 0; i < n; i++) {
		switch (i + offset) {
			case 0:
				str = "MCS-SISO    ";
				ptr = wlc->stf->txpwr_ctl.u20.n.siso;
				break;
			case 1:
				str = "MCS-CDD     ";
				ptr = wlc->stf->txpwr_ctl.u20.n.cdd;
				break;
			case 2:
				str = "MCS STBC    ";
				ptr = wlc->stf->txpwr_ctl.u20.n.stbc;
				break;
			case 3:
				str = "MCS 8~15    ";
				ptr = wlc->stf->txpwr_ctl.u20.n.sdm;
				break;
			case 4:
			case 5:
				ptr = NULL;
				ASSERT(ptr);
				break;
			case 6:
				str = "1 Nsts 1 Tx ";
				ptr = wlc->stf->txpwr_ctl.u20.ht.s1x1;
				break;
			case 7:
				str = "1 Nsts 2 Tx ";
				ptr = wlc->stf->txpwr_ctl.u20.ht.s1x2;
				break;
			case 8:
				str = "1 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht.u20s1x3;
				break;
			case 9:
				str = "2 Nsts 2 Tx ";
				ptr = wlc->stf->txpwr_ctl.u20.ht.s2x2;
				break;
			case 10:
				str = "2 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht.u20s2x3;
				break;
			case 11:
				str = "3 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.u20.ht.s3x3;
				break;
			default:
				ptr = NULL;
				ASSERT(ptr);
				break;
		}
		bcm_bprintf(b, "%s %2d %2d %2d %2d %2d %2d %2d %2d\n",
			str, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);
	}

	bcm_bprintf(b, "\n40MHz:\n");
	bcm_bprintf(b, "OFDM         %2d %2d %2d %2d %2d %2d %2d %2d\n",
		wlc->stf->txpwr_ctl.ofdm_40[0], wlc->stf->txpwr_ctl.ofdm_40[1],
		wlc->stf->txpwr_ctl.ofdm_40[2], wlc->stf->txpwr_ctl.ofdm_40[3],
		wlc->stf->txpwr_ctl.ofdm_40[4], wlc->stf->txpwr_ctl.ofdm_40[5],
		wlc->stf->txpwr_ctl.ofdm_40[6], wlc->stf->txpwr_ctl.ofdm_40[7]);
	bcm_bprintf(b, "OFDM-CDD     %2d %2d %2d %2d %2d %2d %2d %2d\n",
		wlc->stf->txpwr_ctl.ofdm_40_cdd[0], wlc->stf->txpwr_ctl.ofdm_40_cdd[1],
		wlc->stf->txpwr_ctl.ofdm_40_cdd[2], wlc->stf->txpwr_ctl.ofdm_40_cdd[3],
		wlc->stf->txpwr_ctl.ofdm_40_cdd[4], wlc->stf->txpwr_ctl.ofdm_40_cdd[5],
		wlc->stf->txpwr_ctl.ofdm_40_cdd[6], wlc->stf->txpwr_ctl.ofdm_40_cdd[7]);
	for (i = 0; i < n; i++) {
		switch (i + offset) {
			case 0:
				str = "MCS-SISO    ";
				ptr = wlc->stf->txpwr_ctl.u40.n.siso;
				break;
			case 1:
				str = "MCS-CDD     ";
				ptr = wlc->stf->txpwr_ctl.u40.n.cdd;
				break;
			case 2:
				str = "MCS STBC    ";
				ptr = wlc->stf->txpwr_ctl.u40.n.stbc;
				break;
			case 3:
				str = "MCS 8~15    ";
				ptr = wlc->stf->txpwr_ctl.u40.n.sdm;
				break;
			case 4:
			case 5:
				ptr = NULL;
				ASSERT(ptr);
				break;
			case 6:
				str = "1 Nsts 1 Tx ";
				ptr = wlc->stf->txpwr_ctl.u40.ht.s1x1;
				break;
			case 7:
				str = "1 Nsts 2 Tx ";
				ptr = wlc->stf->txpwr_ctl.u40.ht.s1x2;
				break;
			case 8:
				str = "1 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht.u40s1x3;
				break;
			case 9:
				str = "2 Nsts 2 Tx ";
				ptr = wlc->stf->txpwr_ctl.u40.ht.s2x2;
				break;
			case 10:
				str = "2 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht.u40s2x3;
				break;
			case 11:
				str = "3 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.u40.ht.s3x3;
				break;
			default:
				ptr = NULL;
				ASSERT(ptr);
				break;
		}
		bcm_bprintf(b, "%s %2d %2d %2d %2d %2d %2d %2d %2d\n",
			str, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);
	}
	bcm_bprintf(b, "MCS32        %2d\n", wlc->stf->txpwr_ctl.mcs32);

	bcm_bprintf(b, "\n20 in 40MHz:\n");
	if (CHSPEC_IS2G(wlc->home_chanspec)) {
		bcm_bprintf(b, "CCK          %2d %2d %2d %2d\n",
			wlc->stf->txpwr_ctl.cck_20ul[0], wlc->stf->txpwr_ctl.cck_20ul[1],
			wlc->stf->txpwr_ctl.cck_20ul[2], wlc->stf->txpwr_ctl.cck_20ul[3]);
	}
	bcm_bprintf(b, "OFDM         %2d %2d %2d %2d %2d %2d %2d %2d\n",
		wlc->stf->txpwr_ctl.ofdm_20ul[0], wlc->stf->txpwr_ctl.ofdm_20ul[1],
		wlc->stf->txpwr_ctl.ofdm_20ul[2], wlc->stf->txpwr_ctl.ofdm_20ul[3],
		wlc->stf->txpwr_ctl.ofdm_20ul[4], wlc->stf->txpwr_ctl.ofdm_20ul[5],
		wlc->stf->txpwr_ctl.ofdm_20ul[6], wlc->stf->txpwr_ctl.ofdm_20ul[7]);
	bcm_bprintf(b, "OFDM-CDD     %2d %2d %2d %2d %2d %2d %2d %2d\n",
		wlc->stf->txpwr_ctl.ofdm_20ul_cdd[0], wlc->stf->txpwr_ctl.ofdm_20ul_cdd[1],
		wlc->stf->txpwr_ctl.ofdm_20ul_cdd[2], wlc->stf->txpwr_ctl.ofdm_20ul_cdd[3],
		wlc->stf->txpwr_ctl.ofdm_20ul_cdd[4], wlc->stf->txpwr_ctl.ofdm_20ul_cdd[5],
		wlc->stf->txpwr_ctl.ofdm_20ul_cdd[6], wlc->stf->txpwr_ctl.ofdm_20ul_cdd[7]);
	for (i = 0; i < n; i++) {
		switch (i) {
			case 0:
				str = "1 Nsts 1 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht20ul.s1x1;
				break;
			case 1:
				str = "1 Nsts 2 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht20ul.s1x2;
				break;
			case 2:
				str = "1 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht.ul20s1x3;
				break;
			case 3:
				str = "2 Nsts 2 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht20ul.s2x2;
				break;
			case 4:
				str = "2 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht.ul20s2x3;
				break;
			case 5:
				str = "3 Nsts 3 Tx ";
				ptr = wlc->stf->txpwr_ctl.ht20ul.s3x3;
				break;
			default:
				ptr = NULL;
				ASSERT(ptr);
				break;
		}
		bcm_bprintf(b, "%s %2d %2d %2d %2d %2d %2d %2d %2d\n",
			str, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);
	}
	bcm_bprintf(b, "\n");
}
#endif 

static int
wlc_stf_get_ppr_offset(wlc_info_t *wlc, wl_txppr_t *pbuf)
{
	bzero(pbuf, sizeof(wl_txppr_t));

	pbuf->len = WL_TXPPR_LENGTH;
	pbuf->ver = WL_TXPPR_VERSION;

	pbuf->chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
	if (wlc->pub->associated)
		pbuf->local_chanspec = wlc->home_chanspec;

	pbuf->flags = WL_TX_POWER_F_HT | WL_TX_POWER_F_MIMO;

	bcopy((uint8 *)&wlc->stf->txpwr_ctl, pbuf->ppr, WL_TX_POWER_RATES);
	return 0;
}

/* every WLC_TEMPSENSE_PERIOD seconds temperature check to decide whether to turn on/off txchain */
void
wlc_stf_tempsense_upd(wlc_info_t *wlc)
{
	wlc_phy_t *pi = wlc->band->pi;
	uint active_chains, txchain;

	if (!WLCISMIMO || wlc->stf->tempsense_disable) {
		return;
	}

	if ((wlc->pub->now - wlc->stf->tempsense_lasttime) < wlc->stf->tempsense_period) {
		return;
	}

	wlc->stf->tempsense_lasttime = wlc->pub->now;

	active_chains = wlc_phy_stf_chain_active_get(pi);
	txchain = active_chains & 0xf;

	/* update the tempsense txchain setting */
	wlc_stf_txchain_set(wlc, txchain, TRUE, WLC_TXCHAIN_ID_TEMPSENSE);
}

void
wlc_stf_ss_algo_channel_get(wlc_info_t *wlc, uint16 *ss_algo_channel, chanspec_t chanspec)
{
	tx_power_t *power;
	uint8 siso_mcs_id, cdd_mcs_id, stbc_mcs_id;

	/* Clear previous settings */
	*ss_algo_channel = 0;

	if (!wlc->pub->up) {
		*ss_algo_channel = (uint16)-1;
		return;
	}
	if ((power = (tx_power_t*)MALLOC(wlc->osh, sizeof(tx_power_t))) == NULL) {
		ASSERT("OUT-OF-MEMORY" && 0);
		*ss_algo_channel = (uint16)-1;
		return;
	}

	bzero(power, sizeof(tx_power_t));
	wlc_phy_txpower_get_current(wlc->band->pi, power, CHSPEC_CHANNEL(chanspec));

	siso_mcs_id = (CHSPEC_IS40(chanspec)) ?
		WL_TX_POWER_MCS40_SISO_FIRST : WL_TX_POWER_MCS20_SISO_FIRST;
	cdd_mcs_id = (CHSPEC_IS40(chanspec)) ?
		WL_TX_POWER_MCS40_CDD_FIRST : WL_TX_POWER_MCS20_CDD_FIRST;
	stbc_mcs_id = (CHSPEC_IS40(chanspec)) ?
		WL_TX_POWER_MCS40_STBC_FIRST : WL_TX_POWER_MCS20_STBC_FIRST;

	/* criteria to choose stf mode */

	/* the "+3dbm (12 0.25db units)" is to account for the fact that with CDD, tx occurs
	 * on both chains
	 */
	if (power->target[siso_mcs_id] > (power->target[cdd_mcs_id] + 12))
		setbit(ss_algo_channel, PHY_TXC1_MODE_SISO);
	else
		setbit(ss_algo_channel, PHY_TXC1_MODE_CDD);

	/* STBC is ORed into to algo channel as STBC requires per-packet SCB capability check
	 * so cannot be default mode of operation. One of SISO, CDD have to be set
	 */
	if (power->target[siso_mcs_id] <= (power->target[stbc_mcs_id] + 12))
		setbit(ss_algo_channel, PHY_TXC1_MODE_STBC);

	MFREE(wlc->osh, power, sizeof(tx_power_t));
}

bool
wlc_stf_stbc_rx_set(wlc_info_t* wlc, int32 int_val)
{
	if ((int_val != HT_CAP_RX_STBC_NO) && (int_val != HT_CAP_RX_STBC_ONE_STREAM)) {
		return FALSE;
	}

	/* PHY cannot receive STBC with only one rx core active */
	if (WLC_STF_NO_STBC_RX_1_CORE(wlc)) {
		if ((int_val != HT_CAP_RX_STBC_NO) && (wlc->stf->rxstreams == 1))
			return FALSE;
	}

	wlc_stf_stbc_rx_ht_update(wlc, int_val);
	return TRUE;
}

#ifdef RXCHAIN_PWRSAVE
/* called when enter rxchain_pwrsave mode */
uint8
wlc_stf_enter_rxchain_pwrsave(wlc_info_t *wlc)
{
	uint8 ht_cap_rx_stbc = wlc_stf_stbc_rx_get(wlc);
	/* need to save and disable rx_stbc HT capability before enter rxchain_pwrsave mode */
	/* PHY cannot receive STBC with only one rx core active */
	if (WLC_STF_NO_STBC_RX_1_CORE(wlc) && WLC_STBC_CAP_PHY(wlc) &&
		(ht_cap_rx_stbc != HT_CAP_RX_STBC_NO)) {
		wlc_stf_stbc_rx_set(wlc, HT_CAP_RX_STBC_NO);
	}
	return ht_cap_rx_stbc;
}

/* called when exit rxchain_pwrsave mode */
void
wlc_stf_exit_rxchain_pwrsave(wlc_info_t *wlc, uint8 ht_cap_rx_stbc)
{
	/* need to restore rx_stbc HT capability after exit rxchain_pwrsave mode */
	/* PHY cannot receive STBC with only one rx core active */
	if (WLC_STF_NO_STBC_RX_1_CORE(wlc) && WLC_STBC_CAP_PHY(wlc) &&
		(ht_cap_rx_stbc != HT_CAP_RX_STBC_NO)) {
		wlc_stf_stbc_rx_set(wlc, ht_cap_rx_stbc);
	}
}
#endif /* RXCHAIN_PWRSAVE */

static int
wlc_stf_ss_get(wlc_info_t* wlc, int8 band)
{
	wlcband_t *wlc_band = NULL;

	if (band == -1)
		wlc_band = wlc->band;
	else if ((band < 0) || (band > (int)NBANDS(wlc)))
		return BCME_RANGE;
	else
		wlc_band = wlc->bandstate[band];

	return (int)(wlc_band->band_stf_ss_mode);
}

static bool
wlc_stf_ss_set(wlc_info_t* wlc, int32 int_val, int8 band)
{
	wlcband_t *wlc_band = NULL;

	if (band == -1)
		wlc_band = wlc->band;
	else if ((band < 0) || (band > (int)NBANDS(wlc)))
		return FALSE;
	else
		wlc_band = wlc->bandstate[band];

	if ((int_val == PHY_TXC1_MODE_CDD) && (wlc->stf->txstreams == 1)) {
		return FALSE;
	}

	if (int_val != PHY_TXC1_MODE_SISO && int_val != PHY_TXC1_MODE_CDD)
		return FALSE;

	wlc_band->band_stf_ss_mode = (int8)int_val;
	wlc_stf_ss_update(wlc, wlc_band);

	return TRUE;
}

static bool
wlc_stf_ss_auto(wlc_info_t* wlc)
{
	return wlc->stf->ss_algosel_auto;
}


static int
wlc_stf_ss_auto_set(wlc_info_t *wlc, bool enable)
{
	if (wlc->stf->ss_algosel_auto == enable)
		return 0;

	if (WLC_STBC_CAP_PHY(wlc) && enable)
		wlc_stf_ss_algo_channel_get(wlc, &wlc->stf->ss_algo_channel, wlc->chanspec);

	wlc->stf->ss_algosel_auto = enable;
	wlc_stf_ss_update(wlc, wlc->band);

	return 0;
}

static int8
wlc_stf_mimops_get(wlc_info_t* wlc)
{
	return (int8)((wlc->ht_cap.cap & HT_CAP_MIMO_PS_MASK) >> HT_CAP_MIMO_PS_SHIFT);
}

static bool
wlc_stf_mimops_set(wlc_info_t* wlc, wlc_bsscfg_t *cfg, int32 int_val)
{
	if ((int_val < 0) || (int_val > HT_CAP_MIMO_PS_OFF) || (int_val == 2)) {
		return FALSE;
	}

	wlc_ht_mimops_cap_update(wlc, (uint8)int_val);

	if (cfg->associated)
		wlc_send_action_ht_mimops(wlc, cfg, (uint8)int_val);

	return TRUE;
}

static void
wlc_stf_spatial_mode_set(wlc_info_t *wlc, chanspec_t chanspec)
{
	uint8 channel = CHSPEC_CHANNEL(wlc->chanspec);
	int8 mode = AUTO;

	if (CHSPEC_IS2G(chanspec))
		mode = wlc->stf->spatial_mode_config[SPATIAL_MODE_2G_IDX];
	else {
		if (channel < CHANNEL_5G_MID_START)
			mode = wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_LOW_IDX];
		else if (channel < CHANNEL_5G_HIGH_START)
			mode = wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_MID_IDX];
		else if (channel < CHANNEL_5G_UPPER_START)
			mode = wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_HIGH_IDX];
		else
			mode = wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_UPPER_IDX];
	}
	WL_INFORM(("wl%d: %s: channel %d mode %d\n", wlc->pub->unit, __FUNCTION__, channel, mode));

	wlc_stf_spatial_policy_set(wlc, mode);
}

static int
wlc_stf_spatial_mode_upd(wlc_info_t *wlc, int8 *mode)
{
	WL_TRACE(("wl%d: %s: update Spatial Policy\n", wlc->pub->unit, __FUNCTION__));

	if (!WLCISHTPHY(wlc->band))
		return BCME_UNSUPPORTED;

	bcopy(mode, wlc->stf->spatial_mode_config, SPATIAL_MODE_MAX_IDX);
	WL_INFORM(("wl%d: %s mode %d %d %d %d %d\n", wlc->pub->unit, __FUNCTION__,
		wlc->stf->spatial_mode_config[SPATIAL_MODE_2G_IDX],
		wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_LOW_IDX],
		wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_MID_IDX],
		wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_HIGH_IDX],
		wlc->stf->spatial_mode_config[SPATIAL_MODE_5G_UPPER_IDX]));

	wlc_stf_spatial_mode_set(wlc, wlc->chanspec);
	return BCME_OK;
}

static int8
wlc_stf_stbc_tx_get(wlc_info_t* wlc)
{
	return wlc->band->band_stf_stbc_tx;
}

int8
wlc_stf_stbc_rx_get(wlc_info_t* wlc)
{
	return (wlc->ht_cap.cap & HT_CAP_RX_STBC_MASK) >> HT_CAP_RX_STBC_SHIFT;
}

static bool
wlc_stf_stbc_tx_set(wlc_info_t* wlc, int32 int_val)
{
	if ((int_val != AUTO) && (int_val != OFF) && (int_val != ON)) {
		return FALSE;
	}

	if ((int_val == ON) && (wlc->stf->txstreams == 1))
		return FALSE;

	if ((int_val == OFF) || (wlc->stf->txstreams == 1) || !WLC_STBC_CAP_PHY(wlc))
		wlc->ht_cap.cap &= ~HT_CAP_TX_STBC;
	else
		wlc->ht_cap.cap |= HT_CAP_TX_STBC;

	wlc->bandstate[BAND_2G_INDEX]->band_stf_stbc_tx = (int8)int_val;
	wlc->bandstate[BAND_5G_INDEX]->band_stf_stbc_tx = (int8)int_val;

	return TRUE;
}

static uint8
wlc_stf_spatial_map(wlc_info_t *wlc, uint8 idx)
{
	uint8 ncores = (uint8)WLC_BITSCNT(wlc->stf->txcore[idx][1]);
	uint8 Nsts = wlc->stf->txcore[idx][0];

	if (wlc->stf->txstreams < Nsts)
		return 0;

	ASSERT(ncores <= wlc->stf->txstreams);
	/* ncores can be 0 for non-supported Nsts */
	if (ncores == 0)
		return 0;

	if (Nsts == ncores) return 0;
	else if (Nsts == 1 && ncores == 2) return 1;
	else if (Nsts == 1 && ncores == 3) return 2;
	else if (Nsts == 2 && ncores == 3) return 3;
	else ASSERT(0);
	return 0;
}

static void
wlc_stf_txcore_set(wlc_info_t *wlc, uint8 idx, uint8 core_mask)
{
	WL_TRACE(("wl%d: wlc_stf_txcore_set\n", wlc->pub->unit));

	ASSERT(idx < MAX_CORE_IDX);

	WL_NONE(("wl%d: %s: Nsts %d core_mask %x\n",
		wlc->pub->unit, __FUNCTION__, wlc->stf->txcore[idx][0], core_mask));

	if (WLC_BITSCNT(core_mask) > wlc->stf->txstreams) {
		WL_NONE(("wl%d: %s: core_mask(0x%x) > #tx stream(%d) supported, disable it\n",
			wlc->pub->unit, __FUNCTION__, core_mask, wlc->stf->txstreams));
		core_mask = 0;
	}

	if ((WLC_BITSCNT(core_mask) == wlc->stf->txstreams) &&
	    ((core_mask & ~wlc->stf->txchain) || !(core_mask & wlc->stf->txchain))) {
		WL_INFORM(("wl%d: %s: core_mask(0x%x) mismatch #txchain(0x%x), force to txchain\n",
			wlc->pub->unit, __FUNCTION__, core_mask, wlc->stf->txchain));
		core_mask = wlc->stf->txchain;
	}

	wlc->stf->txcore[idx][1] = core_mask;
	if ((wlc->band->bandtype == WLC_BAND_5G && idx == OFDM_IDX) ||
	    (wlc->band->bandtype == WLC_BAND_2G && idx == CCK_IDX)) {
		/* Needs to update beacon and ucode generated response
		 * frames when 1 stream core map changed
		 */
		wlc->stf->phytxant = core_mask << PHY_TXC_ANT_SHIFT;
		wlc_bmac_txant_set(wlc->hw, wlc->stf->phytxant);
		if (wlc->clk &&
		    wlc_valid_rate(wlc, wlc->bcn_rspec, wlc->band->bandtype, FALSE)) {
			wlc_suspend_mac_and_wait(wlc);
			wlc_beacon_phytxctl_txant_upd(wlc, wlc->bcn_rspec);
			wlc_beacon_phytxctl(wlc, wlc->bcn_rspec);
			wlc_enable_mac(wlc);
		}
	}

	WL_INFORM(("wl%d: %s: IDX %d: Nsts %d Core mask 0x%x\n",
		wlc->pub->unit, __FUNCTION__, idx, wlc->stf->txcore[idx][0], core_mask));

	/* invalid tx cache due to core mask change */
	if (WLC_TXC_ENAB(wlc))
		wlc->txcgen++;
	return;
}

static void
wlc_stf_init_txcore_default(wlc_info_t *wlc)
{
	uint8 i;

	switch (wlc->stf->txchain) {
		case 0x06:
			wlc->stf->txcore_idx = 2;
			break;
		default:
			wlc->stf->txcore_idx = 1;
			break;
	}
	for (i = 0; i < MAX_CORE_IDX; i++) {
		/* fill in the Nsts */
		wlc->stf->txcore[i][0] = txcore_default[i][0];
		/* fill in the txcore bit map */
		wlc->stf->txcore[i][1] = txcore_default[i][wlc->stf->txcore_idx];
	}
}

/* Return a core mask with the given number of cores, core_count, set.
 * Will return the prefered core mask if there are more cores
 * available for use than requested.
 * Will return a mask of 0 if the core_count is more than the number
 * of cores availible.
 */
static uint8
wlc_stf_core_mask_assign(wlc_stf_t *stf, uint core_count)
{
	uint8 txchain = stf->txchain;
	uint8 mask;

	if (core_count > stf->txstreams)
		return 0;

	/* if we want one core, just return core 0, 1, or 2,
	 * in that order of preference as available in the txchain mask
	 */
	if (core_count == 1) {
		mask = 1;
		if ((txchain & mask) == 0)
			mask = 2;
		if ((txchain & mask) == 0)
			mask = 4;
		return mask;
	}

	/* if we want 2 cores, return core numbers {0, 2}, {0, 1}, {1, 2},
	 * in that order of preference as available in the txchain mask
	 */
	if (core_count == 2) {
		mask = 5;
		if ((txchain & mask) != mask)
			mask = 3;
		if ((txchain & mask) != mask)
			mask = 6;
		return mask;
	}

	/* must be 3 cores */
	return 7;
}

/* return the max of an array of uint8 values */
static uint8
wlc_stf_uint8_vec_max(uint8 *vec, uint count)
{
	uint i;
	uint8 _max;
	uint8 v;

	_max = vec[0];
	for (i = 1; i < count; i++) {
		v = vec[i];
		if (v > _max)
			_max = v;
	}
	return _max;
}

static void
wlc_stf_txcore_select(wlc_info_t *wlc, uint8 *txcore)
{
	txppr_t *txpwr = &wlc->stf->txpwr_ctl;
	uint core_count[MAX_CORE_IDX];
	uint8 idx;
	int min1, min2, min3;
	bool isbw20 = CHSPEC_IS20(wlc->chanspec);
	uint8 *s1x1, *s1x2, *s1x3, *s2x2, *s2x3;

	/* initialize core_count to just the matching Nsts to be the minimum cores for TX */
	for (idx = 0; idx < MAX_CORE_IDX; idx++)
		core_count[idx] = txcore_default[idx][0];

	/* if there is only one (or none) cores available, use the minimum core count
	 * for all modulations, and just jump to the end to get the mask assignments
	 */
	if (wlc->stf->txstreams < 2)
		goto assign_masks;

	/* The txpwr array is 1/2 dB (hdB) offsets from a max power.  The power
	 * calculations that need to find the minimum power among modulation
	 * types take the max over the power offsets, then negate to find the
	 * minimum power target for the modulation type. The calculation is
	 * essentially treating the power offsets as offsets from zero.  Negating
	 * the offsets to actual power values keeps the greater-than/less-than
	 * calculations more straightforward instead of having to invert the
	 * sence of the comparison.
	 */

	s1x1 = txpwr->cck;
	s1x2 = txpwr->cck_cdd.s1x2;
	s1x3 = txpwr->cck_cdd.s1x3;
	min1 = - wlc_stf_uint8_vec_max(s1x1, WL_NUM_RATES_CCK);
	min2 = - wlc_stf_uint8_vec_max(s1x2, WL_NUM_RATES_CCK);

	if ((min2 + 6) > min1) /* 3 dB = (3 * 2) = 6 hdB */
		core_count[CCK_IDX] = 2;	/* use CDD */

	if (wlc->stf->txstreams == 3) {
		/* check if 3 cores is better than 1 or 2 for Nsts==1 */
		min3 = - wlc_stf_uint8_vec_max(s1x3, WL_NUM_RATES_CCK);
		if ((min3 + 10) > min1 && /* 4.8 dB = (4.8 * 2) = 10 hdB */
		(min3 + 10) > (min2 + 6))
			core_count[CCK_IDX] = 3;

		WL_NONE(("++++++++SET CCK to %d cores, pwr SISO %d CDD %d 1x3 %d\n",
			core_count[CCK_IDX], min1, min2, min3));
	} else {
		WL_NONE(("++++++++SET CCK to %d cores, pwr SISO %d CDD %d\n",
			core_count[CCK_IDX], min1, min2));
	}

	/*
	 * OFDM: SISO > CDD + 3dB, then use SISO
	 */
	if (isbw20) {
		s1x1 = txpwr->ofdm;
		s1x2 = txpwr->ofdm_cdd;
		s1x3 = txpwr->ht.u20s1x3;
	} else {
		s1x1 = txpwr->ofdm_40;
		s1x2 = txpwr->ofdm_40_cdd;
		s1x3 = txpwr->ht.u40s1x3;
	}
	min1 = - wlc_stf_uint8_vec_max(s1x1, WL_NUM_RATES_OFDM);
	min2 = - wlc_stf_uint8_vec_max(s1x2, WL_NUM_RATES_OFDM);

	if ((min2 + 6) > min1) /* 3 dB = (3 * 2) = 6 hdB */
		core_count[OFDM_IDX] = 2;	/* use CDD */

	if (wlc->stf->txstreams == 3) {
		/* check if 3 cores is better than 1 or 2 for Nsts==1 */
		min3 = - wlc_stf_uint8_vec_max(s1x3, WL_NUM_RATES_OFDM);
		if ((min3 + 10) > min1 && /* 4.8 dB = (4.8 * 2) = 10 hdB */
		(min3 + 10) > (min2 + 6))
			core_count[OFDM_IDX] = 3;

		WL_NONE(("++++++++SET OFDM to %d cores, pwr SISO %d CDD %d 1x3 %d\n",
			core_count[OFDM_IDX], min1, min2, min3));
	} else {
		WL_NONE(("++++++++SET OFDM to %d cores, pwr SISO %d CDD %d\n",
			core_count[OFDM_IDX], min1, min2));
	}

	/* 
	 * Nsts 1: 1Tx > 2Tx + 3dB > 3Tx + 4.8dB, then use 1Tx
	 */
	if (isbw20) {
		s1x1 = txpwr->u20.ht.s1x1;
		s1x2 = txpwr->u20.ht.s1x2;
		s1x3 = txpwr->ht.u20s1x3;
	} else {
		s1x1 = txpwr->u40.ht.s1x1;
		s1x2 = txpwr->u40.ht.s1x2;
		s1x3 = txpwr->ht.u40s1x3;
	}
	min1 = - wlc_stf_uint8_vec_max(s1x1, WL_NUM_RATES_OFDM);
	min2 = - wlc_stf_uint8_vec_max(s1x2, WL_NUM_RATES_OFDM);

	/* check if 2 cores is better than 1 */
	if ((min2 + 6) > min1) /* 3 dB = (3 * 2) = 6 hdB */
		core_count[NSTS1_IDX] = 2;

	if (wlc->stf->txstreams == 2) {
		WL_NONE(("++++++++SET Nsts1 to %d cores, pwr 1x1 %d 1x2 %d 1x3 N/A\n",
		         core_count[NSTS1_IDX], min1, min2));
		goto assign_masks;
	}

	/* check if 3 cores is better than 1 or 2 for Nsts==1 */
	min3 = - wlc_stf_uint8_vec_max(s1x3, WL_NUM_RATES_MCS_1STREAM);
	if ((min3 + 10) > min1 && /* 4.8 dB = (4.8 * 2) = 10 hdB */
	    (min3 + 10) > (min2 + 6))
		core_count[NSTS1_IDX] = 3;

	WL_NONE(("++++++++SET Nsts1 to %d cores, pwr 1x1 %d 1x2 %d 1x3 %d\n",
	          core_count[NSTS1_IDX], min1, min2, min3));

	/*
	 * Nsts 2: 2Tx > 3Tx + 1.8dB, then use 2Tx
	 */
	if (isbw20) {
		s2x2 = txpwr->u20.ht.s2x2;
		s2x3 = txpwr->ht.u20s2x3;
	} else {
		s2x2 = txpwr->u40.ht.s2x2;
		s2x3 = txpwr->ht.u40s2x3;
	}
	min2 = - wlc_stf_uint8_vec_max(s2x2, WL_NUM_RATES_MCS_1STREAM);
	min3 = - wlc_stf_uint8_vec_max(s2x3, WL_NUM_RATES_MCS_1STREAM);

	if ((min3 + 4) > min2)
		core_count[NSTS2_IDX] = 3;

	WL_NONE(("++++++++SET Nsts2 to %d cores, pwr 2x2 %d 2x3 %d\n",
	          core_count[NSTS2_IDX], min2, min3));

	/*
	 * Nsts 3: defalut is already set to 3 cores, nothing more to do
	 */

assign_masks:
	/* assign the core masks based on the core count and available cores */
	for (idx = 0; idx < MAX_CORE_IDX; idx++)
		txcore[idx] = wlc_stf_core_mask_assign(wlc->stf, core_count[idx]);

	WL_NONE(("++++++++txstreams %d txchains 0x%x\n", wlc->stf->txstreams, wlc->stf->txchain));
	WL_NONE(("++++++++SET CCK   to 0x%x\n", txcore[CCK_IDX]));
	WL_NONE(("++++++++SET OFDM  to 0x%x\n", txcore[OFDM_IDX]));
	WL_NONE(("++++++++SET Nsts1 to 0x%x\n", txcore[NSTS1_IDX]));
	WL_NONE(("++++++++SET Nsts2 to 0x%x\n", txcore[NSTS2_IDX]));
	WL_NONE(("++++++++SET Nsts3 to 0x%x\n", txcore[NSTS3_IDX]));
}

void
wlc_stf_spatialpolicy_set_complete(wlc_info_t *wlc)
{
	uint8 idx, Nsts;
	uint8 core_mask = 0;
	uint8 txcore[MAX_CORE_IDX];

	wlc->stf->spatialpolicy = wlc->stf->spatialpolicy_pending;
	wlc->stf->spatialpolicy_pending = 0;
	if (wlc->stf->spatialpolicy == AUTO_SPATIAL_EXPANSION) {
		/* set txcore based on txpower for Nsts */
		wlc_stf_txcore_select(wlc, txcore);
	}
	else if (wlc->stf->spatialpolicy == MIN_SPATIAL_EXPANSION) {
		/* set txcore to maximum spatial policy, use all
		 * antenna for any Nsts
		 */
		for (idx = 0; idx < MAX_CORE_IDX; idx++)
			txcore[idx] = txcore_default[idx][wlc->stf->txcore_idx];
	} else {
		/* set txcore to minimum spatial policy, use less
		 * amount of antenna for each Nsts
		 */
		for (idx = 0; idx < MAX_CORE_IDX; idx++)
			txcore[idx] = wlc->stf->txchain;
	}

	for (idx = 0; idx < MAX_CORE_IDX; idx++) {
		core_mask = wlc->stf->txcore_user[idx] ? wlc->stf->txcore_user[idx] : txcore[idx];
		Nsts = wlc->stf->txcore[idx][0];
		/* only initial mcs_txcore to max hw supported */
		if (Nsts > wlc->stf->txstreams) {
			WL_NONE(("wl%d: %s: Nsts (%d) > # of streams hw supported (%d)\n",
			         wlc->pub->unit, __FUNCTION__, Nsts, wlc->stf->txstreams));
			core_mask = 0;
		}

		if (WLC_BITSCNT(core_mask) > wlc->stf->txstreams) {
			WL_NONE(("wl%d: %s: core_mask (0x%02x) > # of HW core enabled (0x%x)\n",
			         wlc->pub->unit, __FUNCTION__, core_mask, wlc->stf->hw_txchain));
			core_mask = 0;
		}

		wlc_stf_txcore_set(wlc, idx, core_mask);
	}

	wlc_stf_txcore_shmem_write(wlc, M_COREMASK_BLK, FALSE);

	/* invalidate txcache since rates are changing */
	wlc_txc_upd(wlc);
	wlc->txcgen++;
}

static int
wlc_stf_spatial_policy_set(wlc_info_t *wlc, int val)
{
	WL_TRACE(("wl%d: %s: val %d\n", wlc->pub->unit, __FUNCTION__, val));

	if (!WLCISHTPHY(wlc->band))
		return BCME_UNSUPPORTED;

	wlc->stf->spatialpolicy_pending = (int8)val;

	/* If packets are enqueued, then wait for it to drain only if switching to fewer chains */
	if (wlc->stf->spatialpolicy_pending != wlc->stf->spatialpolicy) {
		if (TXPKTPENDTOT(wlc)) {
			wlc->block_datafifo |= DATA_BLOCK_SPATIAL;
			return BCME_OK;
		}
	}

	wlc_stf_spatialpolicy_set_complete(wlc);

	return BCME_OK;
}

int
wlc_stf_txchain_subval_get(wlc_info_t* wlc, uint id, uint *txchain)
{
	if (id >= WLC_TXCHAIN_ID_COUNT) {
		return BCME_RANGE;
	}

	*txchain = wlc->stf->txchain_subval[id];

	return BCME_OK;
}

/* store a new value for the given txchain_subval and return the
 * a recalculated AND of all the txchain_subval masks with the 
 * available hw chains.
 */
static uint8
wlc_stf_txchain_subval_update(wlc_stf_t *stf, uint id, uint8 txchain_subval)
{
	int i;
	uint8 txchain = stf->hw_txchain;

	stf->txchain_subval[id] = txchain_subval;

	for (i = 0; i < WLC_TXCHAIN_ID_COUNT; i++)
		txchain = txchain & stf->txchain_subval[i];

	return txchain;
}

int
wlc_stf_txchain_set(wlc_info_t *wlc, int32 int_val, bool force, uint16 id)
{
	uint8 txchain_subval = (uint8)int_val;
	uint8 prev_subval;
	uint8 txchain_pending;
	uint current_streams, new_streams;
	uint i;

	/* save the previous subval in case we need to back out the change */
	prev_subval = wlc->stf->txchain_subval[id];

	/* store the new subval and calculate the resulting overall txchain */
	txchain_pending = wlc_stf_txchain_subval_update(wlc->stf, id, txchain_subval);

	/* if the overall value does not change, just return OK */
	if (wlc->stf->txchain == txchain_pending)
		return BCME_OK;

	/* make sure the value does not have bits outside the range of chains, and
	 * has at least one chain on
	 */
	if ((txchain_pending & ~wlc->stf->hw_txchain) ||
	    !(txchain_pending & wlc->stf->hw_txchain)) {
		wlc->stf->txchain_subval[id] = prev_subval;
		return BCME_RANGE;
	}

	current_streams = WLC_BITSCNT(wlc->stf->txchain);
	new_streams = WLC_BITSCNT(txchain_pending);

	/* if nrate override is configured to be non-SISO STF mode, reject reducing txchain to 1 */
	if (new_streams == 1 && current_streams > 1) {
		for (i = 0; i < NBANDS(wlc); i++) {
			if ((RSPEC_STF(wlc->bandstate[i]->rspec_override) != PHY_TXC1_MODE_SISO) ||
			    (RSPEC_STF(wlc->bandstate[i]->mrspec_override) != PHY_TXC1_MODE_SISO)) {
				if (!force) {
					wlc->stf->txchain_subval[id] = prev_subval;
					return BCME_ERROR;
				}

				/* over-write the override rspec */
				if (RSPEC_STF(wlc->bandstate[i]->rspec_override) !=
				    PHY_TXC1_MODE_SISO) {
					wlc->bandstate[i]->rspec_override = 0;
					WL_ERROR(("%s(): temp sense override non-SISO"
						" rspec_override.\n", __FUNCTION__));
				}
				if (RSPEC_STF(wlc->bandstate[i]->mrspec_override) !=
				    PHY_TXC1_MODE_SISO) {
					wlc->bandstate[i]->mrspec_override = 0;
					WL_ERROR(("%s(): temp sense override non-SISO"
						" mrspec_override.\n", __FUNCTION__));
				}
			}
		}
		if (wlc_stf_stbc_tx_get(wlc) == ON) {
			wlc->bandstate[BAND_2G_INDEX]->band_stf_stbc_tx = OFF;
			wlc->bandstate[BAND_5G_INDEX]->band_stf_stbc_tx = OFF;
		}
	}

	wlc->stf->txchain_pending = txchain_pending;

	/* If packets are enqueued, then wait for it to drain only if switching to fewer chains */
	if ((wlc->stf->txchain & txchain_pending) != wlc->stf->txchain) {
		if (TXPKTPENDTOT(wlc)) {
			wlc->block_datafifo |= DATA_BLOCK_TXCHAIN;
			return BCME_OK;
		}
	}

	wlc->block_datafifo &= ~DATA_BLOCK_TXCHAIN;
	wlc_stf_txchain_set_complete(wlc);

	return BCME_OK;
}

void
wlc_stf_txchain_set_complete(wlc_info_t *wlc)
{
	uint8 txstreams = (uint8)WLC_BITSCNT(wlc->stf->txchain_pending);

	wlc->stf->txchain = wlc->stf->txchain_pending;
	wlc->stf->txchain_pending = 0;
	wlc->stf->txstreams = txstreams;
	wlc_stf_stbc_tx_set(wlc, wlc->band->band_stf_stbc_tx);
	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_2G_INDEX]);
	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_5G_INDEX]);

	if ((wlc->stf->txstreams == 1) && (!WLCISHTPHY(wlc->band))) {
		if (wlc->stf->txchain == 1) {
			wlc->stf->txant = ANT_TX_FORCE_0;
		} else if (wlc->stf->txchain == 2) {
			wlc->stf->txant = ANT_TX_FORCE_1;
		} else {
			ASSERT(0);
		}
	} else {
		wlc->stf->txant = ANT_TX_DEF;
	}

	/* push the updated txant to phytxant (used for txheader) */
	_wlc_stf_phy_txant_upd(wlc);

	/* initialize txcore and spatial policy */
	wlc_stf_init_txcore_default(wlc);
	wlc_stf_spatial_mode_set(wlc, wlc->chanspec);

	/* we need to take care of wlc_rate_init for every scb here */
	wlc_scb_ratesel_init_all(wlc);

	/* invalidate txcache since rates are changing */
	wlc_txc_upd(wlc);
	wlc->txcgen++;

	wlc_phy_stf_chain_set(wlc->band->pi, wlc->stf->txchain, wlc->stf->rxchain);
}

#if defined(WLC_LOW) && defined(WLC_HIGH) && defined(WL11N)
/* Reset the chains back to original values */
static void
wlc_stf_txchain_reset(wlc_info_t *wlc, uint16 id)
{
	/* reset this ID subval to full hw chains */
	wlc_stf_txchain_set(wlc, wlc->stf->hw_txchain, FALSE, id);
}
#endif /* defined(WLC_LOW) && defined(WLC_HIGH) && defined(WL11N) */

#if defined(WLC_LOW) && defined(WL11N)
static int
wlc_stf_txchain_pwr_offset_set(wlc_info_t *wlc, wl_txchain_pwr_offsets_t *offsets)
{
	wlc_stf_t *stf = wlc->stf;
	struct phy_txcore_pwr_offsets phy_offsets;
	struct phy_txcore_pwr_offsets prev_offsets;
	int i;
	int err;
	int8 chain_offset;

	memset(&phy_offsets, 0, sizeof(struct phy_txcore_pwr_offsets));

	for (i = 0; i < WL_NUM_TXCHAIN_MAX; i++) {
		chain_offset = offsets->offset[i];
		if (chain_offset > 0)
			return BCME_RANGE;
		if (chain_offset != 0 && i >= PHY_MAX_CORES) {
			return BCME_BADARG;
		}

		WL_NONE(("wl%d: %s: setting chain %d to chain_offset %d\n",
		         wlc->pub->unit, __FUNCTION__, i, chain_offset));

		phy_offsets.offset[i] = chain_offset;
	}

	/* the call to wlc_phy_txpower_core_offset_set() leads to wlc_update_txppr_offset()
	 * which references the stf->txchain_pwr_offsets values. Store the values
	 * but keep the previous values in case we need to back out the setting on err
	 */

	/* remember the current offsets in case of error */
	memcpy(&prev_offsets, &stf->txchain_pwr_offsets, sizeof(struct phy_txcore_pwr_offsets));
	/* update stf copy to the new offsets */
	memcpy(&stf->txchain_pwr_offsets, offsets, sizeof(struct phy_txcore_pwr_offsets));

	err = wlc_phy_txpower_core_offset_set(wlc->band->pi, &phy_offsets);

	/* restore the settings in our state if error */
	if (err)
		memcpy(&stf->txchain_pwr_offsets,
		       &prev_offsets, sizeof(struct phy_txcore_pwr_offsets));

	return err;
}
#endif /* defined(WLC_LOW) && defined(WL11N) */

int
wlc_stf_rxchain_set(wlc_info_t* wlc, int32 int_val)
{
	uint8 rxchain_cnt;
	uint8 rxchain = (uint8)int_val;
	uint8 mimops_mode;
	uint8 old_rxchain, old_rxchain_cnt;

	if (wlc->stf->rxchain == rxchain)
		return BCME_OK;

	if ((rxchain & ~wlc->stf->hw_rxchain) || !(rxchain & wlc->stf->hw_rxchain))
		return BCME_RANGE;

	rxchain_cnt = (uint8)WLC_BITSCNT(rxchain);

	/* PHY cannot receive STBC with only one rx core active */
	if (WLC_STF_NO_STBC_RX_1_CORE(wlc)) {
		if ((rxchain_cnt == 1) && (wlc_stf_stbc_rx_get(wlc) != HT_CAP_RX_STBC_NO))
			return BCME_RANGE;
	}

	if (APSTA_ENAB(wlc->pub) && (wlc->pub->associated))
		return BCME_ASSOCIATED;

	old_rxchain = wlc->stf->rxchain;
	old_rxchain_cnt = wlc->stf->rxstreams;

	wlc->stf->rxchain = rxchain;
	wlc->stf->rxstreams = rxchain_cnt;

	if (rxchain_cnt != old_rxchain_cnt) {
		mimops_mode = (rxchain_cnt == 1) ? HT_CAP_MIMO_PS_ON : HT_CAP_MIMO_PS_OFF;
		wlc->cfg->mimops_PM = mimops_mode;
		if (AP_ENAB(wlc->pub)) {
			wlc_phy_stf_chain_set(wlc->band->pi, wlc->stf->txchain, wlc->stf->rxchain);
			wlc_ht_mimops_cap_update(wlc, mimops_mode);
			if (wlc->pub->associated)
				wlc_mimops_action_ht_send(wlc, wlc->cfg, mimops_mode);
			return BCME_OK;
		}
		if (wlc->pub->associated) {
			if (mimops_mode == HT_CAP_MIMO_PS_OFF) {
				/* if mimops is off, turn on the Rx chain first */
				wlc_phy_stf_chain_set(wlc->band->pi, wlc->stf->txchain,
					wlc->stf->rxchain);
				wlc_ht_mimops_cap_update(wlc, mimops_mode);
			}
			wlc_mimops_action_ht_send(wlc, wlc->cfg, mimops_mode);
		}
		else {
			wlc_phy_stf_chain_set(wlc->band->pi, wlc->stf->txchain, wlc->stf->rxchain);
			wlc_ht_mimops_cap_update(wlc, mimops_mode);
		}
	}
	else if (old_rxchain != rxchain)
		wlc_phy_stf_chain_set(wlc->band->pi, wlc->stf->txchain, wlc->stf->rxchain);

	return BCME_OK;
}

/* update wlc->stf->ss_opmode which represents the operational stf_ss mode we're using */
int
wlc_stf_ss_update(wlc_info_t *wlc, wlcband_t *band)
{
	int ret_code = 0;
	uint8 prev_stf_ss;
	uint8 upd_stf_ss;
	uint8 mhf4_bphytx;

	prev_stf_ss = wlc->stf->ss_opmode;

	/* NOTE: opmode can only be SISO or CDD as STBC is decided on a per-packet basis */
	if (WLC_STBC_CAP_PHY(wlc) &&
	    wlc->stf->ss_algosel_auto && (wlc->stf->ss_algo_channel != (uint16)-1)) {
		ASSERT(isset(&wlc->stf->ss_algo_channel, PHY_TXC1_MODE_CDD) ||
		       isset(&wlc->stf->ss_algo_channel, PHY_TXC1_MODE_SISO));
		upd_stf_ss = (wlc->stf->no_cddstbc || (wlc->stf->txstreams == 1) ||
			isset(&wlc->stf->ss_algo_channel, PHY_TXC1_MODE_SISO)) ?
			PHY_TXC1_MODE_SISO : PHY_TXC1_MODE_CDD;
	} else {
		if (wlc->band != band)
			return ret_code;
		upd_stf_ss = (wlc->stf->no_cddstbc || (wlc->stf->txstreams == 1)) ?
			PHY_TXC1_MODE_SISO : band->band_stf_ss_mode;
	}
	if (prev_stf_ss != upd_stf_ss) {
		wlc->stf->ss_opmode = upd_stf_ss;
		wlc_bmac_band_stf_ss_set(wlc->hw, upd_stf_ss);
	}

	/* Support for 11b single antenna. */
	/* If SISO operating mode or boardflags indicate that it is allowed
	 * to transmit bphy frames on only one core then force bphy Tx on a
	 * single core.
	 */
	mhf4_bphytx = ((wlc->stf->siso_tx) ||
	               ((wlc->pub->boardflags2 & BFL2_BPHY_ALL_TXCORES) == 0) ?
	               MHF4_BPHY_2TXCORES : 0);
	wlc_mhf(wlc, MHF4, MHF4_BPHY_2TXCORES, mhf4_bphytx, WLC_BAND_AUTO);

	return ret_code;
}

uint8
wlc_stf_txcore_get(wlc_info_t *wlc, ratespec_t rspec)
{
	uint8 idx;
	/* MCS_TXS returns # of streams based 0 {0..3} */
	uint8 Nss = IS_MCS(rspec) ? (MCS_TXS(rspec & RSPEC_RATE_MASK) + 1) : 1;
	uint8 Nstbc = (Nss == 1 && RSPEC_STF(rspec) == PHY_TXC1_MODE_STBC) ? 1 : 0;

	if (IS_CCK(rspec)) {
		idx = CCK_IDX;
		return wlc->stf->txcore[CCK_IDX][1];
	} else if (IS_OFDM(rspec)) {
		idx = OFDM_IDX;
		return wlc->stf->txcore[OFDM_IDX][1];
	} else {
		idx = Nss + Nstbc + OFDM_IDX;
	}
	WL_NONE(("wl%d: %s: Nss %d  Nstbc %d\n", wlc->pub->unit, __FUNCTION__, Nss, Nstbc));
	ASSERT(idx < MAX_CORE_IDX);
	WL_NONE(("wl%d: %s: wlc->stf->txcore[%d] 0x%02x\n",
		wlc->pub->unit, __FUNCTION__, idx, wlc->stf->txcore[idx][1]));
	return wlc->stf->txcore[idx][1];
}

void
wlc_stf_txcore_shmem_write(wlc_info_t *wlc, uint16 base, bool forcewr)
{
	uint16 offset;
	uint16 map;
	uint8 idx;

	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));
	if (!wlc->clk && !forcewr) {
		WL_ERROR(("wl%d: %s: No clock\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (D11REV_LT(wlc->pub->corerev, 26) || !WLCISHTPHY(wlc->band)) {
		WL_INFORM(("wl%d: %s: For now txcore shmem only supported"
			" by HT PHY for corerev >= 26\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (base != M_COREMASK_BLK && base != M_COREMASK_BLK_WOWL) {
		ASSERT("BAD shmem base address" && 0);
		return;
	}

	for (offset = 0, idx = 0; offset < MAX_COREMASK_BLK; offset++, idx++) {
		map = ((wlc_stf_spatial_map(wlc, idx) << SPATIAL_SHIFT) |
			(wlc->stf->txcore[idx][1] & TXCOREMASK));
		WL_NONE(("%s: Write Spatial mapping to SHMEM 0x%04x map 0x%04x\n", __FUNCTION__,
		      (base+offset)*2, map));
		wlc_write_shm(wlc, (base+offset)*2, map);
	}
}
#endif	/* WL11N */

int
BCMATTACHFN(wlc_stf_attach)(wlc_info_t* wlc)
{
	uint temp;
#ifdef WL11N
	/* register module */
	if (wlc_module_register(wlc->pub, stf_iovars, "stf", wlc, wlc_stf_doiovar,
		NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: stf wlc_stf_iovar_attach failed\n", wlc->pub->unit));
		return -1;
	}

#if defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "stf", (dump_fn_t)wlc_dump_stf, (void *)wlc);
	wlc_dump_register(wlc->pub, "ppr", (dump_fn_t)wlc_dump_ppr, (void *)wlc);
#endif
	wlc->bandstate[BAND_2G_INDEX]->band_stf_ss_mode = PHY_TXC1_MODE_SISO;
	wlc->bandstate[BAND_5G_INDEX]->band_stf_ss_mode = PHY_TXC1_MODE_CDD;

	if ((WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) &&
	    (wlc_phy_txpower_hw_ctrl_get(wlc->band->pi) != PHY_TPC_HW_ON))
		wlc->bandstate[BAND_2G_INDEX]->band_stf_ss_mode = PHY_TXC1_MODE_CDD;
	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_2G_INDEX]);
	wlc_stf_ss_update(wlc, wlc->bandstate[BAND_5G_INDEX]);

	wlc_stf_stbc_rx_ht_update(wlc, HT_CAP_RX_STBC_NO);
	wlc->bandstate[BAND_2G_INDEX]->band_stf_stbc_tx = OFF;
	wlc->bandstate[BAND_5G_INDEX]->band_stf_stbc_tx = OFF;

	if (WLC_STBC_CAP_PHY(wlc)) {
		wlc->stf->ss_algosel_auto = TRUE;
		wlc->stf->ss_algo_channel = (uint16)-1; /* Init the default value */
#ifdef LATER
		wlc_stf_stbc_rx_ht_update(wlc, HT_CAP_RX_STBC_ONE_STREAM);
		if (wlc->stf->txstreams > 1) {
			wlc->bandstate[BAND_2G_INDEX]->band_stf_stbc_tx = AUTO;
			wlc->bandstate[BAND_5G_INDEX]->band_stf_stbc_tx = AUTO;
			wlc->ht_cap.cap |= HT_CAP_TX_STBC;
		}
#endif
	}
	bzero((uint8 *)&wlc->stf->txpwr_ctl, WL_TX_POWER_RATES);
	/* default Spatial mode to AUTO */
	memset(wlc->stf->spatial_mode_config, -1, SPATIAL_MODE_MAX_IDX);
#endif /* WL11N */

	wlc->stf->tempsense_period = WLC_TEMPSENSE_PERIOD;
	temp = getintvar(wlc->pub->vars, "temps_period");
	/* valid range is 1-14. ignore 0 and 0xf to work with old srom/nvram */
	if ((temp != 0) && (temp < 0xf))
		wlc->stf->tempsense_period = temp;

	if (!WLCISHTPHY(wlc->band))
		wlc->stf->rssi_pwrdn_disable = TRUE;

	wlc->stf->pwr_throttle_mask = wlc->stf->hw_txchain;

	return 0;
}

void
BCMATTACHFN(wlc_stf_detach)(wlc_info_t* wlc)
{
#ifdef WL11N
	wlc_module_unregister(wlc->pub, "stf", wlc);
#endif /* WL11N */
}

int
wlc_stf_ant_txant_validate(wlc_info_t *wlc, int8 val)
{
	int bcmerror = BCME_OK;

	/* when there is only 1 tx_streams, don't allow to change the txant */
	if (WLCISNPHY(wlc->band) && (wlc->stf->txstreams == 1))
		return ((val == wlc->stf->txant) ? bcmerror : BCME_RANGE);

	switch (val) {
		case -1:
			val = ANT_TX_DEF;
			break;
		case 0:
			val = ANT_TX_FORCE_0;
			break;
		case 1:
			val = ANT_TX_FORCE_1;
			break;
		case 3:
			val = ANT_TX_LAST_RX;
			break;
		default:
			bcmerror = BCME_RANGE;
			break;
	}

	if (bcmerror == BCME_OK)
		wlc->stf->txant = (int8)val;

	return bcmerror;

}

/*
 * Centralized txant update function. call it whenever wlc->stf->txant and/or wlc->stf->txchain
 *  change
 *
 * Antennas are controlled by ucode indirectly, which drives PHY or GPIO to
 *   achieve various tx/rx antenna selection schemes
 *
 * legacy phy, bit 6 and bit 7 means antenna 0 and 1 respectively, bit6+bit7 means auto(last rx)
 * for NREV<3, bit 6 and bit 7 means antenna 0 and 1 respectively, bit6+bit7 means last rx and
 *    do tx-antenna selection for SISO transmissions
 * for NREV=3, bit 6 and bit _8_ means antenna 0 and 1 respectively, bit6+bit7 means last rx and
 *    do tx-antenna selection for SISO transmissions
 * for NREV>=7, bit 6 and bit 7 mean antenna 0 and 1 respectively, bit6+bit7 means both cores active
*/
static void
_wlc_stf_phy_txant_upd(wlc_info_t *wlc)
{
	int8 txant;

	txant = (int8)wlc->stf->txant;
	ASSERT(txant == ANT_TX_FORCE_0 || txant == ANT_TX_FORCE_1 || txant == ANT_TX_LAST_RX);

	if (WLCISHTPHY(wlc->band)) {
		/* phytxant is not use by HT phy, preserved what latest
		 * setting via txcore (update beacon)
		 */
	} else if (WLC_PHY_11N_CAP(wlc->band) || WLCISLPPHY(wlc->band)) {
		if (txant == ANT_TX_FORCE_0) {
			wlc->stf->phytxant = PHY_TXC_ANT_0;
		} else if (txant == ANT_TX_FORCE_1) {
			wlc->stf->phytxant = PHY_TXC_ANT_1;

			if (WLCISNPHY(wlc->band) &&
			    NREV_GE(wlc->band->phyrev, 3) && NREV_LT(wlc->band->phyrev, 7)) {
				wlc->stf->phytxant = PHY_TXC_ANT_2;
			}
		} else {
			/* For LPPHY: specific antenna must be selected, ucode would set last rx */
			if (WLCISLPPHY(wlc->band) || WLCISSSLPNPHY(wlc->band) ||
			    WLCISLCNPHY(wlc->band))
				wlc->stf->phytxant = PHY_TXC_LPPHY_ANT_LAST;
			else {
				/* keep this assert to catch out of sync wlc->stf->txcore */
				ASSERT(wlc->stf->txchain > 0);
				wlc->stf->phytxant = wlc->stf->txchain << PHY_TXC_ANT_SHIFT;
			}
		}
	} else {
		if (txant == ANT_TX_FORCE_0)
			wlc->stf->phytxant = PHY_TXC_OLD_ANT_0;
		else if (txant == ANT_TX_FORCE_1)
			wlc->stf->phytxant = PHY_TXC_OLD_ANT_1;
		else
			wlc->stf->phytxant = PHY_TXC_OLD_ANT_LAST;
	}

	WL_INFORM(("wl%d: _wlc_stf_phy_txant_upd: set core mask 0x%04x\n",
		wlc->pub->unit, wlc->stf->phytxant));
	wlc_bmac_txant_set(wlc->hw, wlc->stf->phytxant);
}

void
wlc_stf_phy_txant_upd(wlc_info_t *wlc)
{
	_wlc_stf_phy_txant_upd(wlc);
}

void
BCMATTACHFN(wlc_stf_phy_chain_calc)(wlc_info_t *wlc)
{
	int i;

	/* get available rx/tx chains */
	wlc->stf->hw_txchain = (uint8)getintvar(wlc->pub->vars, "txchain");
	wlc->stf->hw_rxchain = (uint8)getintvar(wlc->pub->vars, "rxchain");

	if (CHIPID(wlc->pub->sih->chip) == BCM43221_CHIP_ID && wlc->stf->hw_txchain == 3) {
		WL_ERROR(("wl%d: %s: wrong txchain setting %x for 43221. Correct it to 1\n",
			wlc->pub->unit, __FUNCTION__, wlc->stf->hw_txchain));
		wlc->stf->hw_txchain = 1;
	}

	if (CHIPID(wlc->pub->sih->chip) == BCM43131_CHIP_ID) {
		if (wlc->stf->hw_txchain != 2) {
			WL_ERROR(("wl%d: %s: wrong txchain setting %x for 43131. Correct it to 2\n",
			          wlc->pub->unit, __FUNCTION__, wlc->stf->hw_txchain));
			wlc->stf->hw_txchain = 2;
		}

		if (wlc->stf->hw_rxchain != 2) {
			WL_ERROR(("wl%d: %s: wrong rxchain setting %x for 43131. Correct it to 2\n",
			          wlc->pub->unit, __FUNCTION__, wlc->stf->hw_rxchain));
			wlc->stf->hw_rxchain = 2;
		}
	}

	/* these parameter are intended to be used for all PHY types */
	if (wlc->stf->hw_txchain == 0 || wlc->stf->hw_txchain == 0xf) {
		if (WLCISHTPHY(wlc->band)) {
			wlc->stf->hw_txchain = TXCHAIN_DEF_HTPHY;
		} else if (WLCISNPHY(wlc->band)) {
			wlc->stf->hw_txchain = TXCHAIN_DEF_NPHY;
		} else {
			wlc->stf->hw_txchain = TXCHAIN_DEF;
		}
	}

	wlc->stf->txchain = wlc->stf->hw_txchain;
	for (i = 0; i < WLC_TXCHAIN_ID_COUNT; i++)
		wlc->stf->txchain_subval[i] = wlc->stf->hw_txchain;

	wlc->stf->txstreams = (uint8)WLC_BITSCNT(wlc->stf->hw_txchain);

	if (wlc->stf->hw_rxchain == 0 || wlc->stf->hw_rxchain == 0xf) {
		if (WLCISHTPHY(wlc->band)) {
			wlc->stf->hw_rxchain = RXCHAIN_DEF_HTPHY;
		} else if (WLCISNPHY(wlc->band)) {
			wlc->stf->hw_rxchain = RXCHAIN_DEF_NPHY;
		} else {
			wlc->stf->hw_rxchain = RXCHAIN_DEF;
		}
	}

	wlc->stf->rxchain = wlc->stf->hw_rxchain;
	wlc->stf->op_rxstreams = (uint8)WLC_BITSCNT(wlc->stf->hw_rxchain);
	wlc->stf->rxstreams = wlc->stf->op_rxstreams;

#ifdef WL11N
	/* initialize the txcore table */
	wlc_stf_init_txcore_default(wlc);
	/* default spatial policy */
	wlc_stf_spatial_mode_set(wlc, wlc->chanspec);
#endif /* WL11N */
}

static uint16
_wlc_stf_phytxchain_sel(wlc_info_t *wlc, ratespec_t rspec)
{
	uint16 phytxant = wlc->stf->phytxant;

#ifdef WL11N
	if (WLCISHTPHY(wlc->band)) {
		phytxant = wlc_stf_txcore_get(wlc, rspec) << PHY_TXC_ANT_SHIFT;
	} else
#endif /* WL11N */
	{
		if (RSPEC_STF(rspec) != PHY_TXC1_MODE_SISO) {
			ASSERT(wlc->stf->txstreams > 1);
			phytxant = wlc->stf->txchain << PHY_TXC_ANT_SHIFT;
		} else if (wlc->stf->txant == ANT_TX_DEF)
			phytxant = wlc->stf->txchain << PHY_TXC_ANT_SHIFT;
		phytxant &= PHY_TXC_ANT_MASK;
	}
	return phytxant;
}

uint16
wlc_stf_phytxchain_sel(wlc_info_t *wlc, ratespec_t rspec)
{
	return _wlc_stf_phytxchain_sel(wlc, rspec);
}

uint16
wlc_stf_d11hdrs_phyctl_txant(wlc_info_t *wlc, ratespec_t rspec)
{
	uint16 phytxant = wlc->stf->phytxant;
	uint16 mask = PHY_TXC_ANT_MASK;

	/* for non-siso rates or default setting, use the available chains */
	if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) {
		ASSERT(wlc->stf->txchain != 0);
		phytxant = _wlc_stf_phytxchain_sel(wlc, rspec);
		mask = PHY_TXC_HTANT_MASK;
	}
	phytxant |= phytxant & mask;
	return phytxant;
}

#ifdef WL11N
uint16
wlc_stf_spatial_expansion_get(wlc_info_t *wlc, ratespec_t rspec)
{
	uint16 spatial_map = 0;
	uint8 Nsts;
	uint8 idx = 0;
	/* MCS_TXS returns # of streams based 0 {0..3} */
	uint8 Nss = IS_MCS(rspec) ? (MCS_TXS(rspec & RSPEC_RATE_MASK) + 1) : 1;
	uint8 Nstbc = (Nss == 1 && RSPEC_STF(rspec) == PHY_TXC1_MODE_STBC) ? 1 : 0;

	if (!WLCISHTPHY(wlc->band))
		ASSERT(0);

	WL_NONE(("wlc_stf_spatial_expansion_get: Nss %d  Nstbc %d\n", Nss, Nstbc));
	Nsts = Nss + Nstbc;
	ASSERT(Nsts <= wlc->stf->txstreams);
	if (IS_CCK(rspec))
		idx = CCK_IDX;
	else if (IS_OFDM(rspec))
		idx = OFDM_IDX;
	else
		idx = Nsts + OFDM_IDX;
	ASSERT(idx < MAX_CORE_IDX);
	spatial_map = wlc_stf_spatial_map(wlc, idx);
	return spatial_map;
}

#ifdef BCMDBG
#define WLC_PPR_STR(a, b) a = b
#else
#define WLC_PPR_STR(a, b)
#endif

uint8
wlc_stf_get_pwrperrate(wlc_info_t *wlc, ratespec_t rspec, uint16 bw, uint16 spatial_map)
{
	uint8 rate;
	uint8 Nss = IS_MCS(rspec) ? (MCS_TXS(rspec & RSPEC_RATE_MASK) + 1) : 1;
	uint8 Nstbc = (Nss == 1 && RSPEC_STF(rspec) == PHY_TXC1_MODE_STBC) ? 1 : 0;
	uint8 Nsts = Nss + Nstbc;
	uint8 *offset = NULL;
	bool is40MHz;
#ifdef BCMDBG
	const char *str = "";
#endif

	rate = (rspec & RSPEC_RATE_MASK);
	is40MHz = (bw == PHY_TXC1_BW_40MHZ) ? TRUE : FALSE;
	if (IS_MCS(rspec)) {
		/* rate should be one of mcs0 ~ mcs23 or mcs32 */
		ASSERT(rate < 24 || rate == 32);
		if (rate == 32) {
			return wlc->stf->txpwr_ctl.mcs32;
		}
#ifdef NOT_YET
		if (IS_STBC(rspec)) {
			if (is40MHz) {
				offset = wlc->stf->txpwr_ctl.stbc.u40_s2x2;
				WLC_PPR_STR(str, "STBC 2: stbc.u40_s2x2");
				if (spatial_map == 3) {
					offset = wlc->stf->txpwr_ctl.stbc.u40_s2x3;
					WLC_PPR_STR(str, "spatial_map 3: stbc.u40s2x3");
				}
			} else {
				offset = wlc->stf->txpwr_ctl.stbc.s2x2;
				WLC_PPR_STR(str, "STBC 2: stbc.s2x2");
				if (spatial_map == 3) {
					offset = wlc->stf->txpwr_ctl.stbc.s2x3;
					WLC_PPR_STR(str, "spatial_map 3: stbc.u20s2x3");
				}
			}
		} else {
#endif /* NOT_YET */
			switch (Nsts) {
				case 1:
					if (is40MHz) {
					   offset = wlc->stf->txpwr_ctl.u40.ht.s1x1;
					   WLC_PPR_STR(str, "Nsts 1: u40.ht.s1x1");
					   if (spatial_map == 1) {
					      offset = wlc->stf->txpwr_ctl.u40.ht.s1x2;
					      WLC_PPR_STR(str, "spatial_map 1: u40.ht.s1x2");
					   } else if (spatial_map == 2) {
					      offset = wlc->stf->txpwr_ctl.ht.u40s1x3;
					      WLC_PPR_STR(str, "spatial_map 2: ht.u40s1x3");
					   }
					} else {
					   offset = wlc->stf->txpwr_ctl.u20.ht.s1x1;
					   WLC_PPR_STR(str, "Nsts 1: u20.ht.s1x1");
					   if (spatial_map == 1) {
					      offset = wlc->stf->txpwr_ctl.u20.ht.s1x2;
					      WLC_PPR_STR(str, "spatial_map 1: u20.ht.s1x2");
					   } else if (spatial_map == 2) {
					      offset = wlc->stf->txpwr_ctl.ht.u20s1x3;
					      WLC_PPR_STR(str, "spatial_map 2: ht.u20s1x3");
					   }
					}
					break;
				case 2:
					if (is40MHz) {
						offset = wlc->stf->txpwr_ctl.u40.ht.s2x2;
						WLC_PPR_STR(str, "Nsts 2: u40.ht.s2x2");
						if (spatial_map == 3) {
						   offset = wlc->stf->txpwr_ctl.ht.u40s2x3;
						   WLC_PPR_STR(str, "spatial_map 3: ht.u40s2x3");
						}
					} else {
						offset = wlc->stf->txpwr_ctl.u20.ht.s2x2;
						WLC_PPR_STR(str, "Nsts 2: u20.ht.s2x2");
						if (spatial_map == 3) {
						   offset = wlc->stf->txpwr_ctl.ht.u20s2x3;
						   WLC_PPR_STR(str, "spatial_map 3: ht.u20s2x3");
						}
					}
					break;
				case 3:
					if (is40MHz) {
						offset = wlc->stf->txpwr_ctl.u40.ht.s3x3;
						WLC_PPR_STR(str, "Nsts 3: u40.ht.s3x3");
					} else {
						offset = wlc->stf->txpwr_ctl.u20.ht.s3x3;
						WLC_PPR_STR(str, "Nsts 3: u20.ht.s3x3");
					}
					break;
				default:
					ASSERT(0);
					break;
			}
#ifdef NOT_YET
		}
#endif
		rate = rate % WL_NUM_RATES_MCS_1STREAM;
	} else if (IS_OFDM(rspec)) {
		if (is40MHz) {
			offset = wlc->stf->txpwr_ctl.ofdm_40;
			WLC_PPR_STR(str, "OFDM: ofdm_40");
			if (spatial_map == 1) {
				offset = wlc->stf->txpwr_ctl.ofdm_40_cdd;
				WLC_PPR_STR(str, "spatial_map 1: ofdm_40_cdd");
			} else if (spatial_map == 2) {
				offset = wlc->stf->txpwr_ctl.ht.u40s1x3;
				WLC_PPR_STR(str, "spatial_map 2: ht.u40s1x3");
			}
		} else {
			offset = wlc->stf->txpwr_ctl.ofdm;
			WLC_PPR_STR(str, "OFDM: ofdm");
			if (spatial_map == 1) {
				offset = wlc->stf->txpwr_ctl.ofdm_cdd;
				WLC_PPR_STR(str, "spatial_map 1: ofdm_cdd");
			} else if (spatial_map == 2) {
				offset = wlc->stf->txpwr_ctl.ht.u20s1x3;
				WLC_PPR_STR(str, "spatial_map 2: ht.u20s1x3");
			}
		}
		rate = ofdm_pwr_idx_table[rate/6];
		ASSERT(rate != 0x80);
	} else if (IS_CCK(rspec)) {
		offset = wlc->stf->txpwr_ctl.cck;
		WLC_PPR_STR(str, "CCK: cck");
		if (is40MHz) {
			if (spatial_map == 1) {
				offset = wlc->stf->txpwr_ctl.cck_20ul_cdd.s1x2;
				WLC_PPR_STR(str, "spatial_map 1: cck_20ul_cdd.s1x2");
			} else if (spatial_map == 2) {
				offset = wlc->stf->txpwr_ctl.cck_20ul_cdd.s1x3;
				WLC_PPR_STR(str, "spatial_map 2: cck_20ul_cdd.s1x2");
			}
		} else {
			WLC_PPR_STR(str, "CCK: cck cdd 20");
			if (spatial_map == 1) {
				offset = wlc->stf->txpwr_ctl.cck_cdd.s1x2;
				WLC_PPR_STR(str, "spatial_map 1: cck_cdd.s1x2");
			} else if (spatial_map == 2) {
				offset = wlc->stf->txpwr_ctl.cck_cdd.s1x3;
				WLC_PPR_STR(str, "spatial_map 2: cck_cdd.s1x2");
			}
		}
		rate = cck_pwr_idx_table[rate >> 1];
		ASSERT(rate != 0x80);
	} else {
		WL_ERROR(("INVALID rspec %x\n", rspec));
		ASSERT("INVALID rspec" && 0);
	}

	if (offset == NULL || rate == 0x80) {
		ASSERT(0);
		return 0;
	}
#ifdef BCMDBG
#define DIV_QUO(num, div) ((num)/div)  /* Return the quotient of division to avoid floats */
#define DIV_REM(num, div) (((num%div) * 100)/div) /* Return the remainder of division */
	WL_NONE(("wl%d: %s: %s[%d] %2d.%-2d\n", wlc->pub->unit, __FUNCTION__,
		str, rate, DIV_QUO(offset[rate], 2), DIV_REM(offset[rate], 2)));
#endif
	/* return the ppr offset in 0.5dB */
	return (offset[rate]);
}

#ifdef BCMDBG
static int
wlc_stf_pproff_shmem_get(wlc_info_t *wlc, int *retval)
{
	uint16 entry_ptr;
	uint8 rate, idx;
	const wlc_rateset_t *rs_dflt;
	wlc_rateset_t rs;

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: No clock\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NOCLK;
	}

	if (D11REV_LT(wlc->pub->corerev, 22)) {
		WL_INFORM(("wl%d: %s: For now PPR shmem only supported"
				   " by PHY for corerev >= 22\n", wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	rs_dflt = &cck_ofdm_rates;
	wlc_rateset_copy(rs_dflt, &rs);

	if (rs.count > 12) {
		WL_ERROR(("wl%d: %s: rate count %d\n", wlc->pub->unit, __FUNCTION__, rs.count));
		return BCME_BADARG;
	}

	for (idx = 0; idx < rs.count; idx++) {
		rate = rs.rates[idx] & RATE_MASK;
		entry_ptr = wlc_rate_shm_offset(wlc, rate);
		retval[idx] = (int)wlc_read_shm(wlc, (entry_ptr + M_RT_TXPWROFF_POS));
		if (IS_OFDM(rate))
			retval[idx] |= 0x80;
	}
	return BCME_OK;
}

static void
wlc_stf_pproff_shmem_set(wlc_info_t *wlc, uint8 rate, uint8 val)
{
	uint16 entry_ptr;

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: No clock\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (D11REV_LT(wlc->pub->corerev, 22)) {
		WL_INFORM(("wl%d: %s: For now PPR shmem only supported by PHY for corerev >= 22\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	entry_ptr = wlc_rate_shm_offset(wlc, rate);
	wlc_write_shm(wlc, (entry_ptr + M_RT_TXPWROFF_POS), val);
}
#endif /* BCMDBG */

static void
wlc_stf_pproff_shmem_write(wlc_info_t *wlc)
{
	uint8 idx, rate;
	uint8 *cck_ptr, *ofdm_ptr;
	uint16 entry_ptr, val;
	bool is_40 = FALSE;
	const wlc_rateset_t *rs_dflt;
	wlc_rateset_t rs;

	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: No clock\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (D11REV_LT(wlc->pub->corerev, 22)) {
		WL_INFORM(("wl%d: %s: For now PPR shmem only supported by PHY for corerev >= 22\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	rs_dflt = &cck_ofdm_rates;
	wlc_rateset_copy(rs_dflt, &rs);

	if (CHSPEC_IS40(wlc->chanspec))
		is_40 = TRUE;

	/* CCK rates */
	cck_ptr = is_40 ? wlc->stf->txpwr_ctl.cck_20ul : wlc->stf->txpwr_ctl.cck;
#ifdef NOT_YET
	idx = (uint8)WLC_BITSCNT(wlc->stf->txcore[CCK_IDX][1]);
	if (idx == 2) {
		cck_ptr = is_40 ?
			wlc->stf->txpwr_ctl.cck20ul_cdd.s1x2 : wlc->stf->txpwr_ctl.cck_cdd.s1x2;
	} else if (idx == 3) {
		cck_ptr = is_40 ?
			wlc->stf->txpwr_ctl.cck20ul_cdd.s1x3 : wlc->stf->txpwr_ctl.cck_cdd.s1x3;
	}
#endif /* NOT_YET */

	ofdm_ptr = is_40 ? wlc->stf->txpwr_ctl.ofdm_40 : wlc->stf->txpwr_ctl.ofdm;
	idx = (uint8)WLC_BITSCNT(wlc->stf->txcore[OFDM_IDX][1]);
	if (idx == 2) {
		ofdm_ptr = is_40 ? wlc->stf->txpwr_ctl.ofdm_40_cdd : wlc->stf->txpwr_ctl.ofdm_cdd;
	}
	else if (idx == 3) {
		ofdm_ptr = is_40 ? wlc->stf->txpwr_ctl.ht.u40s1x3 : wlc->stf->txpwr_ctl.ht.u20s1x3;
	}

	for (idx = 0; idx < rs.count; idx++) {
		rate = rs.rates[idx] & RATE_MASK;
		entry_ptr = wlc_rate_shm_offset(wlc, rate);
		if (IS_OFDM(rate))
			val = (uint16)*ofdm_ptr++;
		else
			val = (uint16)*cck_ptr++;
		wlc_write_shm(wlc, (entry_ptr + M_RT_TXPWROFF_POS), val);
		WL_NONE(("%s: Write PPR to Rate table in SHMEM %s (%s MHz) %3d(500K): 0x%x = %d\n",
			__FUNCTION__, IS_OFDM(rate) ? "OFDM":"CCK ",
			is_40 ? "40":"20", rate, (entry_ptr + M_RT_TXPWROFF_POS), val));
	}
}
#endif /* WL11N */

void
wlc_update_txppr_offset(wlc_info_t *wlc, int8 *txpwr)
{
#ifdef WL11N
	uint i;
	wlc_stf_t *stf = wlc->stf;
	uint8 *ptr = (uint8 *)&stf->txpwr_ctl;
	int chain;
	uint8 txchain;
	int min_txpwr_limit;
	int min_target;

	WL_TRACE(("wl%d: %s: update txpwr\n", wlc->pub->unit, __FUNCTION__));

	/* need to convert from 0.25 dB to 0.5 dB for use in phy ctl word */
	for (i = WL_TX_POWER_CCK_FIRST; i < WL_TX_POWER_RATES; i++, ptr++) {
		*ptr = txpwr[i] >> 1;
	}

	/* If the minimum tx power target with the per-chain offset applied
	 * is below the min tx power target limit, disable the core for tx.
	 */
	wlc_iovar_getint(wlc, "min_txpower", &min_txpwr_limit);
	min_txpwr_limit = min_txpwr_limit * WLC_TXPWR_DB_FACTOR;

	min_target = (int)wlc_phy_txpower_get_target_min(wlc->band->pi);

	/* if the txchain offset brings the chain below the lower limit
	 * disable the chain
	 */
	txchain = stf->hw_txchain;
	for (chain = 0; chain < WL_NUM_TXCHAIN_MAX; chain++) {
		WL_NONE(("wl%d: %s: chain %d, "
		         "min_target %d min_limit %d offset %d\n",
		         wlc->pub->unit, __FUNCTION__, chain,
		         min_target, min_txpwr_limit,
		         stf->txchain_pwr_offsets.offset[chain]));

		if (min_target + stf->txchain_pwr_offsets.offset[chain] <
		    min_txpwr_limit) {
			WL_NONE(("wl%d: %s: disable chain %d\n",
			         wlc->pub->unit, __FUNCTION__, chain));

			txchain &= ~(1<<chain);
		}
	}

	wlc_stf_txchain_set(wlc, txchain, TRUE, WLC_TXCHAIN_ID_PWR_LIMIT);
	wlc_stf_spatial_mode_set(wlc, wlc->chanspec);
	wlc_stf_pproff_shmem_write(wlc);
#endif /* WL11N */
}

#if defined(WLC_LOW) && defined(WLC_HIGH) && defined(WL11N)
static uint8
wlc_stf_get_target_core(wlc_info_t *wlc)
{
	/* don't disable anymore chains, if we already have one or more
	 * chain(s) disabled
	 */
	if (wlc->stf->txstreams < WLC_BITSCNT(wlc->stf->hw_txchain) || !WLCISHTPHY(wlc->band))
		return wlc->stf->txchain;

	/* priority is disable core 1 (middle), then core 0, then core 2
	 * if core 1 is set and allow to be disable, then disable core 1
	 * if core 0 is set and allow to be disable, then disable core 0
	 * if core 2 is set and allow to be disable, then disable core 2
	 */
	if ((wlc->stf->pwr_throttle_mask & 2) && (wlc->stf->txchain && 2))
		return (wlc->stf->txchain & ~2);
	else if ((wlc->stf->pwr_throttle_mask & 1) && (wlc->stf->txchain && 1))
		return (wlc->stf->txchain & ~1);
	else if ((wlc->stf->pwr_throttle_mask & 4) && (wlc->stf->txchain && 4))
		return (wlc->stf->txchain & ~4);
	return wlc->stf->txchain;
}

void
wlc_pwrthrottle_upd(wlc_info_t *wlc)
{
	bool shared_ant0 = ((wlc->pub->boardflags2 & BFL2_BT_SHARE_ANT0) ==
	                    BFL2_BT_SHARE_ANT0);
	uint8 chain = wlc->stf->hw_rxchain;
	uint32 gpioin = BOARD_GPIO_1_WLAN_PWR;
	uint32 mask = BOARD_GPIO_1_WLAN_PWR;
	uint8 pwr_throttle_req; /* Power throttle request according to GPIO state */
	bool isx21;

	isx21 = ((wlc->pub->sih->boardtype == BCM943224X21) ||
	         (wlc->pub->sih->boardvendor == VENDOR_APPLE &&
	          ((wlc->pub->sih->boardtype == BCM943224X21_FCC) ||
	           (wlc->pub->sih->boardtype == BCM943224X21B))));

	if (!isx21 && wlc->pub->sih->boardtype != BCM94331X19)
		return;

	if (wlc->pub->sih->boardtype == BCM94331X19)
		gpioin = mask = BOARD_GPIO_4_WLAN_PWR;

	/* Read GPIO only for AUTO mode */
	if (wlc->stf->pwr_throttle == AUTO)
		gpioin = si_gpioin(wlc->pub->sih);

	/* WLAN_PWR active LOW, (gpioin & mask) == 0 */
	pwr_throttle_req = ((gpioin & mask) == 0) || (wlc->stf->pwr_throttle == ON);

	WL_NONE(("wl%d: %s: pwr_throttle:%d gpioin:%d pwr_throttle_test:%d"
	         " pwr_throttle_state:%d pwr_throttle_req:%d\n", wlc->pub->unit,
	         __FUNCTION__, wlc->stf->pwr_throttle, gpioin, wlc->stf->pwr_throttle_test,
	         wlc->stf->pwr_throttle_state, pwr_throttle_req));

	if (wlc->stf->pwr_throttle_state == pwr_throttle_req)
		return;

	wlc->stf->pwr_throttle_state = pwr_throttle_req;

	ASSERT(wlc->pub->up);
	if (wlc->stf->pwr_throttle_state == WLC_PWRTHROTTLE_ON) {
		if (isx21)
			chain = shared_ant0? 0x2:0x1;
		else
			chain = wlc_stf_get_target_core(wlc);

#ifdef BCMDBG
		/* For experimentations, use specific chain else board default */
		if (wlc->stf->pwr_throttle_test &&
		    (wlc->stf->pwr_throttle_test < wlc->stf->hw_txchain)) {
			chain = wlc->stf->pwr_throttle_test;
		}
#endif /* BCMDBG */
		if (isx21) {
			wlc_duty_cycle_set(wlc, wlc->stf->tx_duty_cycle_pwr, TRUE, TRUE);
			wlc_duty_cycle_set(wlc, wlc->stf->tx_duty_cycle_pwr, FALSE, TRUE);
			wlc_stf_rxchain_set(wlc, chain);
		}
		wlc_stf_txchain_set(wlc, chain, TRUE, WLC_TXCHAIN_ID_PWRTHROTTLE);
	} else {
		wlc_duty_cycle_set(wlc, wlc->tx_duty_cycle_ofdm, TRUE, TRUE);
		wlc_duty_cycle_set(wlc, wlc->tx_duty_cycle_cck, FALSE, TRUE);
		if (isx21)
			wlc_stf_rxchain_set(wlc, chain);
		wlc_stf_txchain_reset(wlc, WLC_TXCHAIN_ID_PWRTHROTTLE);
	}
}
#endif /* defined(WLC_LOW) && defined(WLC_HIGH) && defined(WL11N) */
