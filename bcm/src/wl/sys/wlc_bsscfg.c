/*
 * BSS Configuration routines for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_bsscfg.c,v 1.240.8.57 2011-02-02 18:30:47 Exp $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/wpa.h>
#include <sbconfig.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#include <wlioctl.h>
#include <epivers.h>
#if defined(BCMSUP_PSK)
#include <proto/eapol.h>
#endif
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <wlc_scb.h>
#if defined(BCMSUP_PSK)
#include <wlc_sup.h>
#endif
#if defined(BCMAUTH_PSK)
#include <wlc_auth.h>
#endif
#include <wl_export.h>
#include <wlc_channel.h>
#include <wlc_ap.h>
#ifdef WMF
#include <wlc_wmf.h>
#endif
#ifdef WIN7
#include <wlc_scan.h>
#endif /* WIN7 */
#include <wlc_alloc.h>
#include <wlc_assoc.h>
#ifdef WLP2P
#include <wlc_p2p.h>
#endif
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#ifdef AP
#include <wlc_bmac.h>
#endif

#ifdef SMF_STATS
/* the status/reason codes of interest */
uint16 const smfs_sc_table[] = {
	DOT11_SC_SUCCESS,
	DOT11_SC_FAILURE,
	DOT11_SC_CAP_MISMATCH,
	DOT11_SC_REASSOC_FAIL,
	DOT11_SC_ASSOC_FAIL,
	DOT11_SC_AUTH_MISMATCH,
	DOT11_SC_AUTH_SEQ,
	DOT11_SC_AUTH_CHALLENGE_FAIL,
	DOT11_SC_AUTH_TIMEOUT,
	DOT11_SC_ASSOC_BUSY_FAIL,
	DOT11_SC_ASSOC_RATE_MISMATCH,
	DOT11_SC_ASSOC_SHORT_REQUIRED,
	DOT11_SC_ASSOC_SHORTSLOT_REQUIRED
};

uint16 const smfs_rc_table[] = {
	DOT11_RC_RESERVED,
	DOT11_RC_UNSPECIFIED,
	DOT11_RC_AUTH_INVAL,
	DOT11_RC_DEAUTH_LEAVING,
	DOT11_RC_INACTIVITY,
	DOT11_RC_BUSY,
	DOT11_RC_INVAL_CLASS_2,
	DOT11_RC_INVAL_CLASS_3,
	DOT11_RC_DISASSOC_LEAVING,
	DOT11_RC_NOT_AUTH,
	DOT11_RC_BAD_PC
};

#define MAX_SCRC_EXCLUDED	16
#endif /* SMF_STATS */

/* Local Functions */

#ifdef MBSS
static int wlc_bsscfg_macgen(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
#endif

static int _wlc_bsscfg_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct ether_addr *ea, uint flags, bool ap);
#if defined(AP) || defined(STA)
static void _wlc_bsscfg_deinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
#endif

static int wlc_bsscfg_bcmcscbinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint bandindex);

#ifdef AP
static int wlc_bsscfg_ap_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
static void wlc_bsscfg_ap_deinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
#endif
#ifdef STA
static int wlc_bsscfg_sta_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
static void wlc_bsscfg_sta_deinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
#endif

static int wlc_bsscfg_init_ext(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int idx,
	struct ether_addr *ea, uint flags, bool ap);

#ifdef SMF_STATS
static int wlc_bsscfg_smfsfree(struct wlc_info *wlc, wlc_bsscfg_t *cfg);
#endif /* SMF_STATS */

/* Count the number of bsscfgs (any type) that are enabled */
static int
wlc_bsscfg_ena_cnt(wlc_info_t *wlc)
{
	int idx, count;

	for (count = idx = 0; idx < WLC_MAXBSSCFG; idx++)
		if (wlc->bsscfg[idx] && wlc->bsscfg[idx]->enable)
			count++;
	return count;
}

/* Return the number of AP bsscfgs that are UP */
int
wlc_ap_bss_up_count(wlc_info_t *wlc)
{
	uint16 i, apbss_up = 0;
	wlc_bsscfg_t *bsscfg;

	FOREACH_UP_AP(wlc, i, bsscfg) {
		apbss_up++;
	}

	return apbss_up;
}

/*
 * Allocate and set up a software packet template
 * @param count The number of packets to use; must be <= WLC_SPT_COUNT_MAX
 * @param len The length of the packets to be allocated
 *
 * Returns 0 on success, < 0 on error.
 */

int
wlc_spt_init(wlc_info_t *wlc, wlc_spt_t *spt, int count, int len)
{
	int idx;

	if (count > WLC_SPT_COUNT_MAX) {
		return -1;
	}

	ASSERT(spt != NULL);
	bzero(spt, sizeof(*spt));

	for (idx = 0; idx < count; idx++) {
		if ((spt->pkts[idx] = PKTGET(wlc->osh, len, TRUE)) == NULL) {
			wlc_spt_deinit(wlc, spt, TRUE);
			return -1;
		}
	}

	spt->latest_idx = -1;

	return 0;
}

/*
 * Clean up a software template object;
 * if pkt_free_force is TRUE, will not check if the pkt is in use before freeing.
 * Note that if "in use", the assumption is that some other routine owns
 * the packet and will free appropriately.
 */

void
wlc_spt_deinit(wlc_info_t *wlc, wlc_spt_t *spt, int pkt_free_force)
{
	int idx;

	if (spt != NULL) {
		for (idx = 0; idx < WLC_SPT_COUNT_MAX; idx++) {
			if (spt->pkts[idx] != NULL) {
				if (pkt_free_force || !SPT_IN_USE(spt, idx)) {
					PKTFREE(wlc->osh, spt->pkts[idx], TRUE);
				} else {
					WLPKTFLAG_BSS_DOWN_SET(WLPKTTAG(spt->pkts[idx]), TRUE);
				}
			}
		}
		bzero(spt, sizeof(*spt));
	}
}

#if defined(MBSS)
static void
mbss_ucode_set(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bool cur_val, new_val;

	/* Assumes MBSS_EN has same value in all cores */
	cur_val = ((wlc_mhf_get(wlc, MHF1, WLC_BAND_AUTO) & MHF1_MBSS_EN) != 0);
	new_val = (MBSS_ENAB(wlc->pub) != 0);

	if (cur_val != new_val) {
		wlc_suspend_mac_and_wait(wlc);
		/* enable MBSS in uCode */
		WL_MBSS(("%s MBSS mode\n", new_val ? "Enabling" : "Disabling"));
		(void)wlc_mhf(wlc, MHF1, MHF1_MBSS_EN, new_val ? MHF1_MBSS_EN : 0, WLC_BAND_ALL);
		wlc_enable_mac(wlc);
	}
}

/* BCMC_FID_SHM_COMMIT - Committing FID to SHM; move driver's value to bcmc_fid_shm */
void
bcmc_fid_shm_commit(wlc_bsscfg_t *bsscfg)
{
	bsscfg->bcmc_fid_shm = bsscfg->bcmc_fid;
	bsscfg->bcmc_fid = INVALIDFID;
}

/* BCMC_FID_INIT - Set driver and shm FID to invalid */
#define BCMC_FID_INIT(bsscfg) do { \
		(bsscfg)->bcmc_fid = INVALIDFID; \
		(bsscfg)->bcmc_fid_shm = INVALIDFID; \
	} while (0)

static int
mbss_bsscfg_up(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int result = 0;
	int idx;
	wlc_bsscfg_t *bsscfg;

	/* Assumes MBSS is enabled for this BSS config herein */

	/* Set pre TBTT interrupt timer to 10 ms for now; will be shorter */
	wlc_write_shm(wlc, SHM_MBSS_PRE_TBTT, wlc->ap->pre_tbtt_us);

	/* if the BSS configs hasn't been given a user defined address or
	 * the address is duplicated, we'll generate our own.
	 */
	FOREACH_BSS(wlc, idx, bsscfg) {
		if (bsscfg == cfg)
			continue;
		if (bcmp(&bsscfg->cur_etheraddr, &cfg->cur_etheraddr, ETHER_ADDR_LEN) == 0)
			break;
	}
	if (ETHER_ISNULLADDR(&cfg->cur_etheraddr) || idx < WLC_MAXBSSCFG) {
		result = wlc_bsscfg_macgen(wlc, cfg);
		if (result) {
			WL_ERROR(("wl%d.%d: mbss_bsscfg_up: unable to generate MAC address\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
			return result;
		}
	}

	/* Set the uCode index of this config */
	cfg->_ucidx = EADDR_TO_UC_IDX(cfg->cur_etheraddr, wlc->mbss_ucidx_mask);
	ASSERT(cfg->_ucidx <= wlc->mbss_ucidx_mask);
	wlc->hw2sw_idx[cfg->_ucidx] = WLC_BSSCFG_IDX(cfg);

	/* Allocate DMA space for beacon software template */
	result = wlc_spt_init(wlc, cfg->bcn_template, BCN_TEMPLATE_COUNT, BCN_TMPL_LEN);
	if (result < 0) {
		WL_ERROR(("wl%d.%d: mbss_bsscfg_up: unable to allocate beacon templates",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		return result;
	}
	/* Set the BSSCFG index in the packet tag for beacons */
	for (idx = 0; idx < BCN_TEMPLATE_COUNT; idx++) {
		WLPKTTAGBSSCFGSET(cfg->bcn_template->pkts[idx], WLC_BSSCFG_IDX(cfg));
	}

	/* Make sure that our SSID is in the correct uCode
	 * SSID slot in shared memory
	 */
	wlc_shm_ssid_upd(wlc, cfg);

	BCMC_FID_INIT(cfg);

	if (!MBSS_ENAB16(wlc->pub)) {
		cfg->flags &= ~(WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB);
		cfg->flags |= (WLC_BSSCFG_SW_BCN | WLC_BSSCFG_SW_PRB);
	} else {
		cfg->flags &= ~(WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB |
		                WLC_BSSCFG_SW_BCN | WLC_BSSCFG_SW_PRB);
		cfg->flags |= (WLC_BSSCFG_MBSS16);
	}

	return result;
}
#endif /* MBSS */

/* Clear non-persistant flags; by default, HW beaconing and probe resp */
#define WLC_BSSCFG_FLAGS_INIT(cfg) do { \
		(cfg)->flags &= WLC_BSSCFG_PERSIST_FLAGS; \
		(cfg)->flags |= (WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB); \
	} while (0)

int
wlc_bsscfg_up(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int ret = BCME_OK;

	ASSERT(cfg != NULL);
	ASSERT(cfg->enable);

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_up(%s): stas/aps/associated %d/%d/%d"
			"flags = 0x%x\n", wlc->pub->unit, (BSSCFG_AP(cfg) ? "AP" : "STA"),
			wlc->stas_associated, wlc->aps_associated, wlc->pub->associated,
			cfg->flags));

#ifdef AP
	if (BSSCFG_AP(cfg)) {
		char * bcn;
		bool radar_chan;

		/* wlc_ap_up() only deals with getting cfg->target_bss setup correctly.
		 * This should not have any affects that need to be undone even if we
		 * don't end up bring the AP up.
		 */
		wlc_ap_up(wlc->ap, cfg);

		radar_chan = wlc_radar_chanspec(wlc->cmi, cfg->target_bss->chanspec);

		/* for softap and extap, following special radar rules */
		/* return bad channel error if radar channel */
		/* when no station associated */
		/* won't allow soft/ext ap to be started on radar channel */
		if (BSS_11H_SRADAR_ENAB(wlc, cfg) &&
		    radar_chan &&
		    !wlc->stas_associated) {
			WL_ERROR(("no assoc STA and starting soft or ext AP on radar channel %d\n",
			  CHSPEC_CHANNEL(cfg->target_bss->chanspec)));
			cfg->up = FALSE;

			return BCME_BADCHAN;
		}

		/* for softap and extap with AP_NORADAR_CHAN flag set, don't allow
		 * bss to start if on a radar channel.
		 */
		if (BSS_11H_AP_NORADAR_CHAN_ENAB(wlc, cfg) && radar_chan) {
			WL_ERROR(("AP_NORADAR_CHAN flag set, disallow ap on radar channel %d\n",
			          CHSPEC_CHANNEL(cfg->target_bss->chanspec)));
			cfg->up = FALSE;
			return BCME_BADCHAN;
		}

		bcn = (char *) MALLOC(wlc->osh, BCN_TMPL_LEN);
		if (!bcn) {
			cfg->up = FALSE;
			return BCME_NOMEM;
		}

		if (WIN7_OS(wlc->pub) && cfg->bcn) {
			/* free old AP beacon info for restarting ap. note that when
			 * cfg->up cleared(below) bsscfg down does not free bcn memory
			 */
			MFREE(wlc->pub->osh, cfg->bcn, cfg->bcn_len);
			cfg->bcn = NULL;
			cfg->bcn_len = 0;
		}

		/* No SSID configured yet... */
		if (cfg->SSID_len == 0) {
			cfg->up = FALSE;
			if (bcn)
				MFREE(wlc->osh, (void *)bcn, BCN_TMPL_LEN);
			return BCME_ERROR;
		}

#ifdef WLAFTERBURNER
		if (APSTA_ENAB(wlc->pub) && wlc->afterburner) {
			WL_ERROR(("wl%d: Unable to bring up the AP bsscfg since ab is enabled\n",
				wlc->pub->unit));
			cfg->up = FALSE;
			if (bcn)
				MFREE(wlc->osh, (void *)bcn, BCN_TMPL_LEN);

			return BCME_ERROR;
		}
#endif /* WLAFTERBURNER */

#ifdef STA
		/* defer to any STA association in progress */
		if (APSTA_ENAB(wlc->pub) && !wlc_apup_allowed(wlc)) {
			WL_APSTA_UPDN(("wl%d: wlc_bsscfg_up: defer AP UP, STA associating: "
				       "stas/aps/associated %d/%d/%d, assoc_state/type %d/%d",
				       wlc->pub->unit, wlc->stas_associated, wlc->aps_associated,
				       wlc->pub->associated, cfg->assoc->state, cfg->assoc->type));
			cfg->up = FALSE;
			if (bcn)
				MFREE(wlc->osh, (void *)bcn, BCN_TMPL_LEN);
			return BCME_OK;
		}
#endif /* STA */

		/* AP mode operation must have the driver up before bringing
		 * up a configuration
		 */
		if (!wlc->pub->up)
			goto end;

		/* Init (non-persistant) flags */
		WLC_BSSCFG_FLAGS_INIT(cfg);

		WL_APSTA_UPDN(("wl%d: wlc_bsscfg_up(%s): flags = 0x%x\n",
			wlc->pub->unit, (BSSCFG_AP(cfg) ? "AP" : "STA"), cfg->flags));

		if (cfg->flags & WLC_BSSCFG_DYNBCN)
			cfg->flags &= ~WLC_BSSCFG_HW_BCN;

#ifdef MBSS
		if (MBSS_ENAB(wlc->pub)) {
			if ((ret = mbss_bsscfg_up(wlc, cfg)) < 0)
				goto end;

			wlc_write_mbss_basemac(wlc, &wlc->vether_base);
		}
		mbss_ucode_set(wlc, cfg);
#endif /* MBSS */

		cfg->up = TRUE;

		if (!BSS_P2P_ENAB(wlc, cfg) && (cfg->wlcif->flags & WLC_IF_VIRTUAL))
			wlc_set_mac(cfg);

		wlc_bss_up(wlc->ap, cfg, bcn, BCN_TMPL_LEN);

		wlc_bss_update_beacon(wlc, cfg);

		if (WIN7_OS(wlc->pub)) {
			uint8 *buf;
			int len = BCN_TMPL_LEN;
			int ch = CHSPEC_CHANNEL(wlc_get_home_chanspec(cfg));
			/* indicate AP starting with channel info */
			wlc_bss_mac_event(wlc, cfg, WLC_E_AP_STARTED, NULL,
				WLC_E_STATUS_SUCCESS, 0, 0, &ch, sizeof(ch));
			/* save current AP beacon info */
			if ((buf = MALLOC(wlc->pub->osh, BCN_TMPL_LEN)) != NULL) {
				wlc_bcn_prb_body(wlc, FC_BEACON, cfg, buf, &len, FALSE);
				if ((cfg->bcn = MALLOC(wlc->pub->osh, len)) != NULL) {
					bcopy(buf, cfg->bcn, len);
					cfg->bcn_len = len;
				}
				MFREE(wlc->pub->osh, buf, BCN_TMPL_LEN);
			}
		}

	end:
		if (cfg->up)
			WL_INFORM(("wl%d: BSS %d is up\n", wlc->pub->unit, cfg->_idx));

		if (bcn)
			MFREE(wlc->osh, (void *)bcn, BCN_TMPL_LEN);

#ifdef STA
		wlc_set_wake_ctrl(wlc);
#endif
	}
#endif /* AP */

#ifdef STA
	if (BSSCFG_STA(cfg)) {
		cfg->up = TRUE;
	}
#endif

	/* presence of multiple active bsscfgs changes defkeyvalid, so update on bsscfg up/down */
	if (wlc->pub->up) {
		uint16 defkeyvalid = wlc_key_defkeyflag(wlc);

		wlc_mhf(wlc, MHF1, MHF1_DEFKEYVALID, defkeyvalid, WLC_BAND_ALL);
		WL_WSEC(("wl%d: %s: updating MHF1_DEFKEYVALID to %d\n",
		         wlc->pub->unit, __FUNCTION__, defkeyvalid != 0));
	}

	return ret;
}

/* Enable: always try to force up */
int
wlc_bsscfg_enable(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_enable %p currently %s\n",
	          wlc->pub->unit, bsscfg, (bsscfg->enable ? "ENABLED" : "DISABLED")));

	ASSERT(bsscfg != NULL);

	/* If a bss is already enabled, then just up it */
	if (bsscfg->enable)
		return wlc_bsscfg_up(wlc, bsscfg);

	if (!MBSS_ENAB(wlc->pub)) {
		/* block simultaneous multiple AP connection */
		if (BSSCFG_AP(bsscfg) && AP_ACTIVE(wlc)) {
			WL_ERROR(("wl%d: Cannot enable multiple AP bsscfg\n", wlc->pub->unit));
			return BCME_ERROR;
		}

		/* block simultaneous IBSS and AP connection */
		if (BSSCFG_AP(bsscfg) && wlc->ibss_bsscfgs) {
			WL_ERROR(("wl%d: Cannot enable AP bsscfg with a IBSS\n", wlc->pub->unit));
			return BCME_ERROR;
		}
	}

	bsscfg->enable = TRUE;

	if (BSSCFG_AP(bsscfg)) {
#ifdef STA
		bool mpc_out = wlc->mpc_out;
#endif
		int err = BCME_OK;

#ifdef STA
		/* bringup the driver */
		wlc->mpc_out = TRUE;
		wlc_radio_mpc_upd(wlc);

		/* AP mode operation must have the driver up before bringing
		 * up a configuration
		 */
		if (!wlc->pub->up) {
			err = BCME_NOTUP;
			goto end;
		}
#endif

#ifdef MBSS
		/* make sure we don't exceed max */
		if (MBSS_ENAB16(wlc->pub) &&
		    ((uint32)AP_BSS_UP_COUNT(wlc) >= wlc->max_ap_bss)) {
			bsscfg->enable = FALSE;
			WL_ERROR(("wl%d: max %d ap bss allowed\n",
			          wlc->pub->unit, wlc->max_ap_bss));
			err = BCME_ERROR;
			goto end;
		}
#endif /* MBSS */

		err = wlc_bsscfg_up(wlc, bsscfg);

#if defined(STA) || defined(MBSS)
	end:
#endif
#ifdef STA
		wlc->mpc_out = mpc_out;
		wlc_radio_mpc_upd(wlc);
#endif
		return err;
	}

	/* wlc_bsscfg_up() will be called for STA assoication code:
	 * - for IBSS, in wlc_join_start_ibss() and in wlc_join_BSS()
	 * - for BSS, in wlc_assoc_complete()
	 */
	/*
	 * if (BSSCFG_STA(bsscfg)) {
	 *	return BCME_OK;
	 * }
	 */

	return BCME_OK;
}

int
wlc_bsscfg_down(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int callbacks = 0;

	ASSERT(cfg != NULL);

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_down %p currently %s %s; stas/aps/associated %d/%d/%d\n",
	          wlc->pub->unit, cfg, (cfg->up ? "UP" : "DOWN"), (BSSCFG_AP(cfg) ? "AP" : "STA"),
	          wlc->stas_associated, wlc->aps_associated, wlc->pub->associated));

	if (!cfg->up) {
		/* Are we in the process of an association? */
#ifdef STA
		if ((BSSCFG_STA(cfg) && cfg->assoc->state != AS_IDLE))
			wlc_assoc_abort(cfg);
#endif /* STA */
#ifdef AP
		if (BSSCFG_AP(cfg) && cfg->associated) {
			/* For AP, cfg->up can be 0 but down never called.
			 * Thus, it's best to check for both !up and !associated
			 * before we decide to skip the down procedures.
			 */
			WL_APSTA_UPDN(("wl%d: AP cfg up = %d but associated, "
			               "continue with down procedure.\n",
			               wlc->pub->unit, cfg->up));
		}
		else
#endif
		return callbacks;
	}

	/* cancel any csa timer */
	if (!wl_del_timer(wlc->wl, cfg->csa->csa_timer))
		callbacks++;

#ifdef AP
	if (BSSCFG_AP(cfg)) {

		/* bring down this config */
		cfg->up = FALSE;

		if (!BSS_P2P_ENAB(wlc, cfg) && (cfg->wlcif->flags & WLC_IF_VIRTUAL))
			wlc_set_mac(cfg);

		callbacks += wlc_ap_down(wlc->ap, cfg);

#ifdef MBSS
		{
		uint i, clear_len = FALSE;
		wlc_bsscfg_t *bsscfg;
		uint8 ssidlen = cfg->SSID_len;

		wlc_spt_deinit(wlc, cfg->bcn_template, FALSE);

		if (cfg->probe_template != NULL) {
			PKTFREE(wlc->osh, cfg->probe_template, TRUE);
			cfg->probe_template = NULL;
		}

#ifdef WLLPRS
		if (cfg->lprs_template != NULL) {
			PKTFREE(wlc->osh, cfg->lprs_template, TRUE);
			cfg->lprs_template = NULL;
		}
#endif /* WLLPRS */

		/* If we clear ssid length of all bsscfgs while doing
		 * a wl down the ucode can get into a state where it 
		 * will keep searching  for non-zero ssid length thereby
		 * causing mac_suspend_and_wait messages. To avoid that
		 * we will prevent clearing the ssid len of at least one BSS
		 */
		FOREACH_BSS(wlc, i, bsscfg) {
			if (bsscfg->up) {
				clear_len = TRUE;
				break;
			}
		}
		if (clear_len) {
			cfg->SSID_len = 0;

			/* update uCode shared memory */
			wlc_shm_ssid_upd(wlc, cfg);
			cfg->SSID_len = ssidlen;

			/* Clear the HW index */
			wlc->hw2sw_idx[cfg->_ucidx] = WLC_BSSCFG_IDX_INVALID;
		}
		}
#endif /* MBSS */

#ifdef BCMAUTH_PSK
		if (cfg->authenticator != NULL)
			wlc_authenticator_down(cfg->authenticator);
#endif

		if (!AP_ACTIVE(wlc) && wlc->pub->up) {
			wlc_suspend_mac_and_wait(wlc);
			wlc_mctrl(wlc, MCTL_AP, 0);
			wlc_enable_mac(wlc);
#ifdef STA
			if (APSTA_ENAB(wlc->pub)) {
				int idx;
				wlc_bsscfg_t *bc;
				FOREACH_AS_STA(wlc, idx, bc) {
					if (bc != wlc->cfg)
						continue;
					WL_APSTA_UPDN(("wl%d: wlc_bsscfg_down: last AP down,"
					               "sync STA: assoc_state %d type %d\n",
					               wlc->pub->unit, bc->assoc->state,
					               bc->assoc->type));
					if (bc->assoc->state == AS_IDLE) {
						ASSERT(bc->assoc->type == AS_IDLE);
						wlc_assoc_change_state(bc, AS_SYNC_RCV_BCN);
					}
				}
			}
#endif /* STA */
		}

#ifdef STA
		wlc_radio_mpc_upd(wlc);
		wlc_set_wake_ctrl(wlc);
#endif

		if (WIN7_OS(wlc->pub) && cfg->bcn) {
			/* free beacon info */
			MFREE(wlc->osh, cfg->bcn, cfg->bcn_len);
			cfg->bcn = NULL;
			cfg->bcn_len = 0;
		}
	}
#endif /* AP */

#ifdef STA
	if (BSSCFG_STA(cfg)) {
		/* cancel any apsd trigger timer */
		if (!wl_del_timer(wlc->wl, cfg->pm->apsd_trigger_timer))
			callbacks++;
		/* cancel any pspoll timer */
		if (!wl_del_timer(wlc->wl, cfg->pm->pspoll_timer))
			callbacks ++;
#if defined(BCMSUP_PSK) && defined(BCMINTSUP)
		callbacks += wlc_sup_down(cfg->sup);
#endif
		/* abort any assocaitions or roams in progress */
		callbacks += wlc_assoc_abort(cfg);
		cfg->up = FALSE;
	}
#endif /* STA */

#ifdef WL_BSSCFG_TX_SUPR
	pktq_flush(wlc->osh, cfg->psq, TRUE, NULL, 0);
#endif

	/* presence of multiple active bsscfgs changes defkeyvalid, so update on bsscfg up/down */
	if (wlc->pub->up) {
		uint16 defkeyvalid = wlc_key_defkeyflag(wlc);

		wlc_mhf(wlc, MHF1, MHF1_DEFKEYVALID, defkeyvalid, WLC_BAND_ALL);
		WL_WSEC(("wl%d: %s: updating MHF1_DEFKEYVALID to %d\n",
		         wlc->pub->unit, __FUNCTION__, defkeyvalid != 0));
	}

	return callbacks;
}

int
wlc_bsscfg_disable(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	struct scb_iter scbiter;
	int callbacks = 0;

	ASSERT(bsscfg != NULL);

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_disable %p currently %s\n",
	          wlc->pub->unit, bsscfg, (bsscfg->enable ? "ENABLED" : "DISABLED")));

	/* If a bss is already disabled, don't do anything */
	if (!bsscfg->enable) {
		ASSERT(!bsscfg->up);
		return 0;
	}

	if (BSSCFG_AP(bsscfg)) {
		struct scb *scb;
		int assoc_scb_count = 0;

		/* WIN7 cleans up stas in wlc_ap_down */
		if (!WIN7_OS(wlc->pub)) {
			FOREACHSCB(wlc->scbstate, &scbiter, scb) {
				if (SCB_ASSOCIATED(scb) && scb->bsscfg == bsscfg) {
					wlc_scbfree(wlc, scb);
					assoc_scb_count++;
				}
			}

			/* send up a broadcast deauth mac event if there were any
			 * associated STAs
			 */
			if (assoc_scb_count)
				wlc_deauth_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS, &ether_bcast,
					DOT11_RC_DEAUTH_LEAVING, 0);
		}
	}

	callbacks += wlc_bsscfg_down(wlc, bsscfg);
	ASSERT(!bsscfg->up);

#ifdef STA
	if (BSSCFG_STA(bsscfg)) {
		if (bsscfg->associated) {
			if (wlc->pub->up) {
				wlc_disassociate_client(bsscfg, TRUE, NULL, NULL);
			} else {
				wlc_sta_assoc_upd(bsscfg, FALSE);
			}
		}
#ifdef WLMCHAN
		else if (MCHAN_ENAB(wlc->pub) && (bsscfg->chan_context != NULL)) {
			WL_MCHAN(("wl%d.%d: %s: Delete chanctx for cfg disable, not associated\n",
			          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
			wlc_mchan_delete_bss_chan_context(wlc, bsscfg);
		}
#endif /* WLMCHAN */

		/* make sure we don't retry */
		if (bsscfg->assoc != NULL) {
			wlc_assoc_t *as = bsscfg->assoc;
			if (as->timer != NULL) {
				if (!wl_del_timer(wlc->wl, as->timer)) {
					bsscfg->as_rt = FALSE;
					callbacks ++;
				}
			}
		}
	}
#endif /* STA */

	bsscfg->flags &= ~WLC_BSSCFG_PRESERVE;

	bsscfg->enable = FALSE;

	/* do a full cleanup of scbs if all configs disabled */
	if (wlc_bsscfg_ena_cnt(wlc) == 0)
		wlc_scbclear(wlc, FALSE);

	return callbacks;
}

#ifdef AP
/* Mark all but the primary cfg as disabled */
void
wlc_bsscfg_disablemulti(wlc_info_t *wlc)
{
	int i;
	wlc_bsscfg_t * bsscfg;

	/* iterate along the ssid cfgs */
	for (i = 1; i < WLC_MAXBSSCFG; i++)
		if ((bsscfg = WLC_BSSCFG(wlc, i)))
			wlc_bsscfg_disable(wlc, bsscfg);
}

static int
wlc_bsscfg_ap_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_ap_init: bsscfg %p\n", wlc->pub->unit, bsscfg));

	/* Init flags: Beacons/probe resp in HW by default */
	bsscfg->flags |= (WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB);

	if (bsscfg->flags & WLC_BSSCFG_DYNBCN)
		bsscfg->flags &= ~WLC_BSSCFG_HW_BCN;

#if defined(MBSS)
	bsscfg->maxassoc = wlc->pub->tunables->maxscb;
	BCMC_FID_INIT(bsscfg);
	bsscfg->prb_ttl_us = WLC_PRB_TTL_us;
#endif

	bsscfg->wpa2_preauth = TRUE;

	bsscfg->_ap = TRUE;

#if defined(BCMAUTH_PSK)
	ASSERT(bsscfg->authenticator == NULL);
	if ((bsscfg->authenticator = wlc_authenticator_attach(wlc, bsscfg)) == NULL) {
		WL_ERROR(("wl%d: wlc_bsscfg_ap_init: wlc_authenticator_attach failed\n",
		          wlc->pub->unit));
		return BCME_ERROR;

	}
#endif	/* BCMAUTH_PSK */

	return BCME_OK;
}

static void
wlc_bsscfg_ap_deinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_ap_deinit: bsscfg %p\n", wlc->pub->unit, bsscfg));

#if defined(BCMAUTH_PSK)
	/* free the authenticator */
	if (bsscfg->authenticator) {
		wlc_authenticator_detach(bsscfg->authenticator);
		bsscfg->authenticator = NULL;
	}
#endif /* BCMAUTH_PSK */

	_wlc_bsscfg_deinit(wlc, bsscfg);

	bsscfg->flags &= ~(WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB);
}
#endif /* AP */

#ifdef STA
static int
wlc_bsscfg_sta_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	wlc_roam_t *roam = bsscfg->roam;
	wlc_assoc_t *as = bsscfg->assoc;
	wlc_pm_st_t *pm = bsscfg->pm;

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_sta_init: bsscfg %p\n", wlc->pub->unit, bsscfg));

	/* Init flags: Beacons/probe resp in HW by default (IBSS) */
	bsscfg->flags |= (WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB);

	bsscfg->_ap = FALSE;

	bzero(roam, sizeof(*roam));
	bzero(as, sizeof(*as));
	bzero(pm, sizeof(*pm));

	/* init beacon timeouts */
	roam->bcn_timeout = WLC_BCN_TIMEOUT;

	/* init beacon-lost roaming thresholds */
	roam->uatbtt_tbtt_thresh = WLC_UATBTT_TBTT_THRESH;
	roam->tbtt_thresh = WLC_ROAM_TBTT_THRESH;

	/* roam scan inits */
	roam->scan_block = 0;
	roam->partialscan_period = WLC_ROAM_SCAN_PERIOD;
	roam->fullscan_period = WLC_FULLROAM_PERIOD;
	roam->ap_environment = AP_ENV_DETECT_NOT_USED;
	roam->motion_timeout = ROAM_MOTION_TIMEOUT;

	/* create association timer */
	if ((as->timer =
	     wl_init_timer(wlc->wl, wlc_assoc_timeout, bsscfg, "assoc")) == NULL) {
		WL_ERROR(("wl%d: wl_init_timer for bsscfg %d assoc_timer failed\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		return BCME_NORESOURCE;
	}

	as->recreate_bi_timeout = WLC_ASSOC_RECREATE_BI_TIMEOUT;
	as->listen = WLC_ADVERTISED_LISTEN;

	/* default AP disassoc/deauth timeout */
	as->verify_timeout = WLC_ASSOC_VERIFY_TIMEOUT;

	/* join preference */
	if ((bsscfg->join_pref = (wlc_join_pref_t *)
	     wlc_calloc(wlc->osh, wlc->pub->unit, sizeof(wlc_join_pref_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	/* init join pref */
	bsscfg->join_pref->band = WLC_BAND_AUTO;

	/* create apsd trigger timer */
	if ((pm->apsd_trigger_timer =
	     wl_init_timer(wlc->wl, wlc_apsd_trigger_timeout, bsscfg, "apsd_trigger")) == NULL) {
		WL_ERROR(("wl%d: wl_init_timer for bsscfg %d wlc_apsd_trigger_timeout failed\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		return BCME_NORESOURCE;
	}

	/* create pspoll timer */
	if ((pm->pspoll_timer =
	     wl_init_timer(wlc->wl, wlc_pspoll_timer, bsscfg, "pspoll")) == NULL) {
		WL_ERROR(("wl%d: wl_init_timer for bsscfg %d pspoll_timer failed\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		return BCME_NORESOURCE;
	}

	/* allocate pm2_ret_timer object */
	if ((pm->pm2_ret_timer = wlc_hwtimer_alloc_timeout(wlc)) == NULL) {
		WL_ERROR(("wl%d.%d: %s: failed to alloc PM2 timeout\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		return BCME_NORESOURCE;
	}

	/* allocate pm2_rcv_timer object */
	if ((pm->pm2_rcv_timer = wlc_hwtimer_alloc_timeout(wlc)) == NULL) {
		WL_ERROR(("wl%d.%d: %s: failed to alloc PM2 timeout\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		return BCME_NORESOURCE;
	}

	/* Set the default PM2 return to sleep time */
	pm->pm2_sleep_ret_time = PM2_SLEEP_RET_MS_DEFAULT;
	pm->pm2_sleep_ret_time_left = pm->pm2_sleep_ret_time;


#if defined(BCMSUP_PSK) && defined(BCMINTSUP)
	ASSERT(bsscfg->sup == NULL);
	if ((bsscfg->sup = wlc_sup_attach(wlc, bsscfg)) == NULL) {
		WL_ERROR(("wl%d: wlc_bsscfg_sta_init: wlc_sup_attach failed\n", wlc->pub->unit));
		return BCME_ERROR;

	}
#endif	

	return BCME_OK;
}

static void
wlc_bsscfg_sta_deinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_sta_deinit: bsscfg %p\n", wlc->pub->unit, bsscfg));

	/* free the association timer */
	if (bsscfg->assoc != NULL) {
		wlc_assoc_t *as = bsscfg->assoc;

		if (as->timer != NULL) {
			wl_free_timer(wlc->wl, as->timer);
			as->timer = NULL;
		}
		/* Need to free the allocated memory here because if we were to init
		 * this bsscfg as a STA again, bsscfg->assoc will get zeroed out and
		 * the allocated memory elements would be lost and never freed.
		 */
		if (as->ie != NULL) {
			MFREE(wlc->osh, as->ie, as->ie_len);
			as->ie = NULL;
		}
		if (as->req != NULL) {
			MFREE(wlc->osh, as->req, as->req_len);
			as->req = NULL;
		}
		if (as->resp != NULL) {
			MFREE(wlc->osh, as->resp, as->resp_len);
			as->resp = NULL;
		}
	}

	if (bsscfg->join_pref != NULL) {
		MFREE(wlc->osh, bsscfg->join_pref, sizeof(wlc_join_pref_t));
		bsscfg->join_pref = NULL;
	}

	if (bsscfg->pm != NULL) {
		wlc_pm_st_t *pm = bsscfg->pm;

		if (pm->apsd_trigger_timer) {
			wl_free_timer(wlc->wl, pm->apsd_trigger_timer);
			pm->apsd_trigger_timer = NULL;
		}

		if (pm->pspoll_timer) {
			wl_free_timer(wlc->wl, pm->pspoll_timer);
			pm->pspoll_timer = NULL;
		}

		/* free the pm2_rcv_timer object */
		if (pm->pm2_rcv_timer != NULL) {
			wlc_hwtimer_del_timeout(wlc->gptimer, pm->pm2_rcv_timer);
			wlc_hwtimer_free_timeout(wlc, pm->pm2_rcv_timer);
			pm->pm2_rcv_timer = NULL;
		}

		/* free the pm2_rcv_timeout object */
		if (pm->pm2_ret_timer != NULL) {
			wlc_hwtimer_del_timeout(wlc->gptimer, pm->pm2_ret_timer);
			wlc_hwtimer_free_timeout(wlc, pm->pm2_ret_timer);
			pm->pm2_ret_timer = NULL;
		}
	}

	_wlc_bsscfg_deinit(wlc, bsscfg);

	wlc_bsscfg_scan_params_reset(wlc, bsscfg);
	wlc_bsscfg_assoc_params_reset(wlc, bsscfg);

#if defined(BCMSUP_PSK) && defined(BCMINTSUP)
	/* free the supplicant */
	if (bsscfg->sup != NULL) {
		wlc_sup_detach(bsscfg->sup);
		bsscfg->sup = NULL;
	}
#endif 

	bsscfg->flags &= ~(WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB);
}
#endif /* STA */

#ifdef WLBTAMP
int
wlc_bsscfg_bta_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_bta_init: bsscfg %p\n",
	          wlc->pub->unit, bsscfg));

	/* N.B.: QoS settings configured implicitly */
	bsscfg->flags |= WLC_BSSCFG_BTA;

	return wlc_bsscfg_init(wlc, bsscfg);
}
#endif /* WLBTAMP */

#if defined(WLP2P)
static int
wlc_bsscfg_bss_rsinit(wlc_info_t *wlc, wlc_bss_info_t *bi, uint8 rates, uint8 bw, bool allow_mcs)
{
	wlc_rateset_t *src = &wlc->band->hw_rateset;
	wlc_rateset_t *dst = &bi->rateset;

	wlc_rateset_filter(src, dst, FALSE, rates, RATE_MASK_FULL, allow_mcs);
	if (dst->count == 0)
		return BCME_NORESOURCE;
#ifdef WL11N
	wlc_rateset_bw_mcs_filter(dst, bw);
#endif
	wlc_rate_lookup_init(wlc, dst);

	return BCME_OK;
}

static int
wlc_bsscfg_rateset_init(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 rates, uint8 bw, bool allow_mcs)
{
	int err;

	if ((err = wlc_bsscfg_bss_rsinit(wlc, cfg->target_bss, rates, bw, allow_mcs)) != BCME_OK)
		return err;
	if ((err = wlc_bsscfg_bss_rsinit(wlc, cfg->current_bss, rates, bw, allow_mcs)) != BCME_OK)
		return err;

	return err;
}
#endif 


#ifdef WLP2P
int
wlc_bsscfg_p2p_init(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int ret;
	uint8 gmode = GMODE_AUTO;

	cfg->flags |= WLC_BSSCFG_P2P;

	if ((ret = wlc_bsscfg_init(wlc, cfg)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: cannot init bsscfg\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	/* make sure gmode is not GMODE_LEGACY_B */
	if (!IS_SINGLEBAND_5G(wlc->deviceid)) {
		gmode = wlc->bandstate[BAND_2G_INDEX]->gmode;
	}
	if (gmode == GMODE_LEGACY_B) {
		ret = BCME_BADRATESET;
		WL_ERROR(("wl%d: %s: gmode cannot be GMODE_LEGACY_B\n",
		  wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	if ((ret = wlc_bsscfg_rateset_init(wlc, cfg, WLC_RATES_OFDM,
	                wlc->band->mimo_cap_40 ? CHSPEC_WLC_BW(wlc->home_chanspec) : 0,
	                BSS_N_ENAB(wlc, cfg))) != BCME_OK) {
		WL_ERROR(("wl%d: %s: failed rateset init\n", wlc->pub->unit, __FUNCTION__));
		goto exit;
	}

	if (!BSS_P2P_DISC_ENAB(wlc, cfg)) {
		if ((cfg->p2p = wlc_p2p_info_alloc(wlc->p2p, cfg)) == NULL) {
			WL_ERROR(("wl%d: %s: out of memory for p2p info, malloced %d bytes\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			ret = BCME_NOMEM;
			goto exit;
		}
	}
	else {
		bcopy(&cfg->cur_etheraddr, &cfg->BSSID, ETHER_ADDR_LEN);
		cfg->current_bss->chanspec = wlc->home_chanspec;
		cfg->current_bss->infra = 0;
	}

	if (BSSCFG_AP(cfg)) {
		ASSERT(cfg->rcmta_bssid_idx < RCMTA_SIZE);
		cfg->rcmta_ra_idx = cfg->rcmta_bssid_idx;
	}
	else if ((ret = wlc_p2p_d11ra_alloc(wlc->p2p, &cfg->rcmta_ra_idx)) != BCME_OK) {
		WL_ERROR(("wl%d.%d: %s: failed to alloc RCMTA entry for RA\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		goto exit;
	}

exit:
	return ret;
}
#endif /* WLP2P */

int
wlc_bsscfg_vif_init(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int ret = 0;

	cfg->wlcif->flags |= WLC_IF_VIRTUAL;

	ret = wlc_bsscfg_init(wlc, cfg);
	if (ret)
		goto exit;

#ifdef WLP2P
	if (BSSCFG_AP(cfg) && (cfg->rcmta_bssid_idx < RCMTA_SIZE)) {
		cfg->rcmta_ra_idx = cfg->rcmta_bssid_idx;
	}
	else if ((ret = wlc_p2p_d11ra_alloc(wlc->p2p, &cfg->rcmta_ra_idx)) != BCME_OK) {
		WL_ERROR(("wl%d.%d: %s: failed to alloc RCMTA entry for RA\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
	}
#endif

exit:
	return ret;

}

int
wlc_bsscfg_get_free_idx(wlc_info_t *wlc)
{
	int idx;

	for (idx = 0; idx < WLC_MAXBSSCFG; idx++) {
		if (wlc->bsscfg[idx] == NULL)
			return idx;
	}

	return -1;
}

void
wlc_bsscfg_ID_assign(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	bsscfg->ID = wlc->next_bsscfg_ID;
	wlc->next_bsscfg_ID ++;
}

wlc_bsscfg_t *
wlc_bsscfg_alloc(wlc_info_t *wlc, int idx, uint flags, struct ether_addr *ea, bool ap)
{
	wlc_bsscfg_t *bsscfg;

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_alloc: index %d flags 0x%08x ap %d\n",
	               wlc->pub->unit, idx, flags, ap));

	ASSERT((idx > 0 && idx < WLC_MAXBSSCFG));

	if ((bsscfg = wlc_bsscfg_malloc(wlc->osh, wlc->pub->unit)) == NULL) {
		WL_ERROR(("wl%d: wlc_bsscfg_alloc: out of memory for bsscfg, malloc failed\n",
		          wlc->pub->unit));
		return NULL;
	}
	wlc_bsscfg_ID_assign(wlc, bsscfg);

	if (wlc_bsscfg_init_ext(wlc, bsscfg, idx,
	                        ea != NULL ? ea : &wlc->pub->cur_etheraddr,
	                        flags, ap) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_bsscfg_alloc: wlc_bsscfg_init_ext failed\n",
		          wlc->pub->unit));
		wlc_bsscfg_free(wlc, bsscfg);
		bsscfg = NULL;
	}

	return bsscfg;
}

int
wlc_bsscfg_vif_reset(wlc_info_t *wlc, int idx, uint flags, struct ether_addr *ea, bool ap)
{
	wlc_bsscfg_t *bsscfg;
	int err;

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_reset: index %d flags 0x%08x ap %d\n",
	               wlc->pub->unit, idx, flags, ap));

	ASSERT((idx > 0 && idx < WLC_MAXBSSCFG));
	if ((idx < 0) || (idx >= WLC_MAXBSSCFG)) {
		return BCME_RANGE;
	}

	if (wlc->bsscfg[idx] == NULL)
		return BCME_ERROR;

	bsscfg = wlc->bsscfg[idx];

	/* clear SSID */
	memset(bsscfg->SSID, 0, DOT11_MAX_SSID_LEN);
	bsscfg->SSID_len = 0;

	err = _wlc_bsscfg_init(wlc, bsscfg, ea != NULL ? ea : &wlc->pub->cur_etheraddr, flags, ap);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: wlc_bsscfg_vif_reset: _wlc_bsscfg_init() failed\n",
			wlc->pub->unit));
		return err;
	}

	err = wlc_bsscfg_vif_init(wlc, bsscfg);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: wlc_bsscfg_vif_reset: Cannot init bsscfg, err = %d\n",
			wlc->pub->unit, err));
	}

	bsscfg->up = FALSE;

	return err;

}

void
wlc_bsscfg_scbclear(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg, bool perm)
{
	struct scb_iter scbiter;
	struct scb *scb;
	int ii;

	if (wlc->scbstate == NULL)
		return;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->bsscfg != bsscfg)
			continue;
		if (scb->permanent) {
			if (!perm)
				continue;
			scb->permanent = FALSE;
		}
		wlc_scbfree(wlc, scb);
	}

	if (perm) {
		for (ii = 0; ii < MAXBANDS; ii++) {
			if (bsscfg->bcmc_scb[ii]) {
				WL_INFORM(("bcmc_scb: band %d: free internal scb for 0x%p\n",
					ii, bsscfg->bcmc_scb[ii]));
				wlc_internalscb_free(wlc, bsscfg->bcmc_scb[ii]);
				bsscfg->bcmc_scb[ii] = NULL;
			}
		}
	}
}

#if defined(AP) || defined(STA)
static void
_wlc_bsscfg_deinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	uint ii;

	WL_APSTA_UPDN(("wl%d: _wlc_bsscfg_deinit: bsscfg %p\n", wlc->pub->unit, bsscfg));

	/* free all but bcmc scbs */
	wlc_bsscfg_scbclear(wlc, bsscfg, FALSE);

	/* process event queue */
	wlc_eventq_flush(wlc->eventq);

#if defined(AP)
	wlc_vndr_ie_free(bsscfg);
#endif


	/*
	 * If the index into the wsec_keys table is less than WSEC_MAX_DEFAULT_KEYS,
	 * the keys were allocated statically, and should not be deleted or removed.
	 */
	if (bsscfg != wlc->cfg) {
		for (ii = 0; ii < ARRAYSIZE(bsscfg->bss_def_keys); ii ++) {
			if (bsscfg->bss_def_keys[ii] == NULL)
				continue;
			wlc_key_delete(wlc, bsscfg, bsscfg->bss_def_keys[ii]);
		}
	}

	/* free RCMTA keys if keys allocated */
	if (bsscfg->rcmta != NULL)
		wlc_rcmta_del_bssid(wlc, bsscfg);
}
#endif /* AP || STA */

void
wlc_bsscfg_deinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	if (bsscfg->_ap) {
#ifdef AP
		wlc_bsscfg_ap_deinit(wlc, bsscfg);
#endif
	}
	else {
#ifdef STA
		wlc_bsscfg_sta_deinit(wlc, bsscfg);
#endif
	}
}

void
wlc_bsscfg_free(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	int indx;

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_free: bsscfg %p, flags = 0x%x\n",
		wlc->pub->unit, bsscfg, bsscfg->flags));

	wlc_bsscfg_deinit(wlc, bsscfg);

#ifdef WLP2P
		/* free p2p related resources */
		if (P2P_ENAB(wlc->pub)) {
			if (bsscfg->p2p != NULL) {
				wlc_p2p_info_free(wlc->p2p, bsscfg->p2p);
				bsscfg->p2p = NULL;
			}
			if (bsscfg != wlc_bsscfg_primary(wlc))
				wlc_p2p_d11cb_free(wlc->p2p, bsscfg);
			if (bsscfg->rcmta_bssid_idx < RCMTA_SIZE) {
				if (wlc->pub->up)
					wlc_set_rcmta(wlc, bsscfg->rcmta_bssid_idx, &ether_null);
				wlc_p2p_d11ra_free(wlc->p2p, bsscfg->rcmta_bssid_idx);
				bsscfg->rcmta_bssid_idx = RCMTA_SIZE;
				if (BSSCFG_AP(bsscfg))
					bsscfg->rcmta_ra_idx = RCMTA_SIZE;
			}
			if (bsscfg->rcmta_ra_idx < RCMTA_SIZE) {
				if (wlc->pub->up)
					wlc_set_rcmta(wlc, bsscfg->rcmta_ra_idx, &ether_null);
				wlc_p2p_d11ra_free(wlc->p2p, bsscfg->rcmta_ra_idx);
				bsscfg->rcmta_ra_idx = RCMTA_SIZE;
			}
		}
		ASSERT(bsscfg->rcmta_ra_idx == RCMTA_SIZE);
		ASSERT(bsscfg->rcmta_bssid_idx == RCMTA_SIZE);
#endif /* WLP2P */

#ifdef WLMCHAN
		/* if context still exists here, delete it */
		if (MCHAN_ENAB(wlc->pub) && bsscfg->chan_context) {
			WL_MCHAN(("%s: context still exist, delete\n", __FUNCTION__));
			wlc_mchan_delete_bss_chan_context(wlc, bsscfg);
		}
#endif

	if (WIN7_OS(wlc->pub) && bsscfg->bcn) {
		/* free AP beacon info in case wlc_bsscfg_disable() not called */
		/* This memory is typically freed in wlc_bsscfg_down(), which is */
		/* called by wlc_bsscfg_disable() */
		MFREE(wlc->pub->osh, bsscfg->bcn, bsscfg->bcn_len);
		bsscfg->bcn = NULL;
		bsscfg->bcn_len = 0;
	}

	/* free all scbs */
	wlc_bsscfg_scbclear(wlc, bsscfg, TRUE);

#ifdef WMF
	/* Delete WMF instance if it created for this bsscfg */
	if (WMF_ENAB(bsscfg)) {
		wlc_wmf_instance_del(bsscfg);
	}
#endif

	if (!(bsscfg->flags & WLC_BSSCFG_P2P_RECREATE_BSSIDX)) {
#ifdef AP
		/* delete the upper-edge driver interface */
		if (bsscfg != wlc_bsscfg_primary(wlc)) {
			if (bsscfg->wlcif != NULL)
				wlc_if_event(wlc, WLC_E_IF_DEL, bsscfg->wlcif);
			/* process event queue */
			wlc_eventq_flush(wlc->eventq);
			if (bsscfg->wlcif != NULL &&
			    bsscfg->wlcif->wlif != NULL) {
				wl_del_if(wlc->wl, bsscfg->wlcif->wlif);
				bsscfg->wlcif->wlif = NULL;
			}
		}
#endif
		wlc_wlcif_free(wlc, wlc->osh, bsscfg->wlcif);
		bsscfg->wlcif = NULL;
	}
	else if (bsscfg != wlc_bsscfg_primary(wlc)) {
		/* process event queue */
		wlc_eventq_flush(wlc->eventq);
	}

#ifdef SMF_STATS
	wlc_bsscfg_smfsfree(wlc, bsscfg);
#endif

#ifdef STA
	wlc_bsscfg_wsec_key_buf_free(wlc, bsscfg);
#endif

	/* free the wlc_bsscfg struct if it was an allocated one */
	indx = bsscfg->_idx;
	if (bsscfg != wlc_bsscfg_primary(wlc)) {
		/* delete CSA timer */
		ASSERT(bsscfg->csa != NULL);
		if (bsscfg->csa->csa_timer != NULL)
			wl_free_timer(wlc->wl, bsscfg->csa->csa_timer);
		wlc_bsscfg_mfree(wlc->osh, bsscfg);
	}
	wlc->bsscfg[indx] = NULL;

	/* update txcache since bsscfg going away may change settings */
	if (WLC_TXC_ENAB(wlc))
		wlc_txc_upd(wlc);
}

int
wlc_bsscfg_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_init: bsscfg %p\n", wlc->pub->unit, bsscfg));

#if defined(AP) && defined(STA)
	if (bsscfg->_ap)
		return wlc_bsscfg_ap_init(wlc, bsscfg);
	return wlc_bsscfg_sta_init(wlc, bsscfg);
#elif defined(AP)
	return wlc_bsscfg_ap_init(wlc, bsscfg);
#elif defined(STA)
	return wlc_bsscfg_sta_init(wlc, bsscfg);
#else
	return BCME_OK;
#endif
}

int
wlc_bsscfg_reinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool ap)
{
#if defined(AP) && defined(STA)
	int ret;
#endif

	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_reinit: bsscfg %p ap %d\n", wlc->pub->unit, bsscfg, ap));

	if (bsscfg->_ap == ap)
		return BCME_OK;

#if defined(AP) && defined(STA)
	if (ap) {
		wlc_bsscfg_sta_deinit(wlc, bsscfg);
		ret = wlc_bsscfg_ap_init(wlc, bsscfg);
		if (ret != BCME_OK)
			return ret;
		if (bsscfg != wlc_bsscfg_primary(wlc))
			wlc_if_event(wlc, WLC_E_IF_CHANGE, bsscfg->wlcif);
		return ret;
	}
	wlc_bsscfg_ap_deinit(wlc, bsscfg);
	ret = wlc_bsscfg_sta_init(wlc, bsscfg);
	if (ret != BCME_OK)
		return ret;
	if (bsscfg != wlc_bsscfg_primary(wlc))
		wlc_if_event(wlc, WLC_E_IF_CHANGE, bsscfg->wlcif);
	return ret;
#else
	return BCME_OK;
#endif /* AP && STA */
}

/* Get a bsscfg pointer, failing if the bsscfg does not alreay exist.
 * Sets the bsscfg pointer in any event.
 * Returns BCME_RANGE if the index is out of range or BCME_NOTFOUND
 * if the wlc->bsscfg[i] pointer is null
 */
wlc_bsscfg_t *
wlc_bsscfg_find(wlc_info_t *wlc, int idx, int *perr)
{
	wlc_bsscfg_t *bsscfg;

	if ((idx < 0) || (idx >= WLC_MAXBSSCFG)) {
		*perr = BCME_RANGE;
		return NULL;
	}

	bsscfg = wlc->bsscfg[idx];
	*perr = bsscfg ? 0 : BCME_NOTFOUND;

	return bsscfg;
}

wlc_bsscfg_t *
wlc_bsscfg_primary(wlc_info_t *wlc)
{
	return wlc->cfg;
}

void
BCMATTACHFN(wlc_bsscfg_primary_cleanup)(wlc_info_t *wlc)
{
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_primary(wlc);

	/* delete CSA timer */
	ASSERT(bsscfg->csa != NULL);
	if (bsscfg->csa->csa_timer != NULL) {
		wl_free_timer(wlc->wl, bsscfg->csa->csa_timer);
		bsscfg->csa->csa_timer = NULL;
	}

	wlc_bsscfg_scbclear(wlc, bsscfg, TRUE);
}

static int
wlc_bsscfg_init_ext(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, int idx,
	struct ether_addr *ea, uint flags, bool ap)
{
	int err;

	wlc->bsscfg[idx] = bsscfg;
	bsscfg->_idx = (int8)idx;

	/* init CSA timer */
	ASSERT(bsscfg->csa != NULL);
	if ((bsscfg->csa->csa_timer =
	     wl_init_timer(wlc->wl, wlc_csa_timeout, bsscfg, "csa")) == NULL) {
		WL_ERROR(("wl%d: wlc_bsscfg_alloc: wl_init_timer failed\n",
		          wlc->pub->unit));
		return BCME_NORESOURCE;
	}

	if ((err = _wlc_bsscfg_init(wlc, bsscfg, ea, flags, ap)) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_bsscfg_init_ext: _wlc_bsscfg_init() failed\n",
		          wlc->pub->unit));
		return err;
	}

	return BCME_OK;
}

int
BCMATTACHFN(wlc_bsscfg_primary_init)(wlc_info_t *wlc)
{
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_primary(wlc);
	int err;

	if ((err = wlc_bsscfg_init_ext(wlc, bsscfg, 0,
	                               &wlc->pub->cur_etheraddr,
	                               0, wlc->pub->_ap)) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_bsscfg_primary_init: wlc_bsscfg_init_ext() failed\n",
		          wlc->pub->unit));
		wlc_bsscfg_free(wlc, bsscfg);
		return err;
	}

	return wlc_bsscfg_init(wlc, bsscfg);
}

/*
 * Find a bsscfg from matching cur_etheraddr, BSSID, SSID, or something unique.
 */

/* match wlcif */
wlc_bsscfg_t *
wlc_bsscfg_find_by_wlcif(wlc_info_t *wlc, wlc_if_t *wlcif)
{
	/* wlcif being NULL implies primary interface hence primary bsscfg */
	if (wlcif == NULL)
		return wlc_bsscfg_primary(wlc);

	switch (wlcif->type) {
	case WLC_IFTYPE_BSS:
		return wlcif->u.bsscfg;
#ifdef AP
	case WLC_IFTYPE_WDS:
		return SCB_BSSCFG(wlcif->u.scb);
#endif
	}

	WL_ERROR(("wl%d: Unknown wlcif %p type %d\n", wlc->pub->unit, wlcif, wlcif->type));
	return NULL;
}

/* match cur_etheraddr */
wlc_bsscfg_t * BCMFASTPATH
wlc_bsscfg_find_by_hwaddr(wlc_info_t *wlc, struct ether_addr *hwaddr)
{
	int i;
	wlc_bsscfg_t *bsscfg;

	if (ETHER_ISNULLADDR(hwaddr) || ETHER_ISMULTI(hwaddr))
		return NULL;

	FOREACH_BSS(wlc, i, bsscfg) {
		if (bcmp(hwaddr->octet, bsscfg->cur_etheraddr.octet, ETHER_ADDR_LEN) == 0 &&
		    (bsscfg->flags & WLC_BSSCFG_EXAMPT_FLAGS) == 0) {
			if ((bsscfg->wlcif->flags & WLC_IF_VIRTUAL) && !bsscfg->up)
				continue;
			return bsscfg;
		}
	}

	return NULL;
}

/* match BSSID */
wlc_bsscfg_t * BCMFASTPATH
wlc_bsscfg_find_by_bssid(wlc_info_t *wlc, struct ether_addr *bssid)
{
	int i;
	wlc_bsscfg_t *bsscfg;

	if (ETHER_ISNULLADDR(bssid) || ETHER_ISMULTI(bssid))
		return NULL;

	FOREACH_BSS(wlc, i, bsscfg) {
		if (bcmp(bssid->octet, bsscfg->BSSID.octet, ETHER_ADDR_LEN) == 0)
			return bsscfg;
	}

	return NULL;
}

/* match target_BSSID */
wlc_bsscfg_t *
wlc_bsscfg_find_by_target_bssid(wlc_info_t *wlc, struct ether_addr *bssid)
{
	int i;
	wlc_bsscfg_t *bsscfg;

	if (ETHER_ISNULLADDR(bssid) || ETHER_ISMULTI(bssid))
		return NULL;

	FOREACH_BSS(wlc, i, bsscfg) {
		if (!BSSCFG_STA(bsscfg))
			continue;
		if (bcmp(bssid->octet, bsscfg->target_bss->BSSID.octet, ETHER_ADDR_LEN) == 0)
			return bsscfg;
	}

	return NULL;
}

/* match SSID */
wlc_bsscfg_t *
wlc_bsscfg_find_by_ssid(wlc_info_t *wlc, uint8 *ssid, int ssid_len)
{
	int i;
	wlc_bsscfg_t *bsscfg;

	FOREACH_BSS(wlc, i, bsscfg) {
		if (ssid_len > 0 &&
		    ssid_len == bsscfg->SSID_len && bcmp(ssid, bsscfg->SSID, ssid_len) == 0)
			return bsscfg;
	}

	return NULL;
}

/* match ID */
wlc_bsscfg_t *
wlc_bsscfg_find_by_ID(wlc_info_t *wlc, uint16 id)
{
	int i;
	wlc_bsscfg_t *bsscfg;

	FOREACH_BSS(wlc, i, bsscfg) {
		if (bsscfg->ID == id)
			return bsscfg;
	}

	return NULL;
}

static void
wlc_bsscfg_bss_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	wlc_bss_info_t * bi = wlc->default_bss;

	bcopy((char*)bi, (char*)bsscfg->target_bss, sizeof(wlc_bss_info_t));
	bcopy((char*)bi, (char*)bsscfg->current_bss, sizeof(wlc_bss_info_t));
}

static int
_wlc_bsscfg_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct ether_addr *ea, uint flags, bool ap)
{
	wlc_prot_cfg_t *prot;
	brcm_ie_t *brcm_ie;

	ASSERT(bsscfg != NULL);
	ASSERT(ea != NULL);

	bsscfg->wlc = wlc;

	bsscfg->flags = flags;
	bsscfg->_ap = ap;

	bcopy(ea, &bsscfg->cur_etheraddr, ETHER_ADDR_LEN);

#ifdef WLP2P
	if (!(flags & WLC_BSSCFG_P2P_RESET)) {
		/* by default they don't require RCMTA entries */
		bsscfg->rcmta_ra_idx = RCMTA_SIZE;
		bsscfg->rcmta_bssid_idx = RCMTA_SIZE;
		if (P2P_ENAB(wlc->pub) && (bsscfg != wlc_bsscfg_primary(wlc))) {
			/* GO/Client/STA need an BSSID entry anyway. AP doesn't really need one
			 * but allocate it and init it with the EPROM MAC address and hopefully
			 * it'll resolve AP's not responding unicast probe request issue. Even
			 * if it doesn't and needs an ucode fix I believe this is what we should
			 * do anyway.
			 */
			if (!BSS_P2P_DISC_ENAB(wlc, bsscfg)) {
				if (wlc_p2p_d11ra_alloc(wlc->p2p, &bsscfg->rcmta_bssid_idx)
					!= BCME_OK) {
					WL_ERROR(("wl%d.%d: %s: failed to alloc RCMTA entry for"
						 "BSSID\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
						 __FUNCTION__));
					return BCME_NORESOURCE;
				}
			}
			/* GO/Client/AP/STA/Device all need an BSS entry. */
			if (wlc_p2p_d11cb_alloc(wlc->p2p, bsscfg) != BCME_OK) {
				WL_ERROR(("wl%d: %s: failed to alloc d11 shm control block\n",
				          wlc->pub->unit, __FUNCTION__));
				return BCME_NORESOURCE;
			}
		}
	}
#endif /* WLP2P */

#ifdef WL_BSSCFG_TX_SUPR
	ASSERT(bsscfg->psq != NULL);
	pktq_init(bsscfg->psq, WLC_PREC_COUNT, PKTQ_LEN_DEFAULT);
#endif

	bsscfg->BSS = TRUE;	/* set the mode to INFRA */

	/* initialize security state */
	bsscfg->wsec_index = -1;
	bsscfg->wsec = 0;

	/* Match Wi-Fi default of true for aExcludeUnencrypted,
	 * instead of 802.11 default of false.
	 */
	bsscfg->wsec_restrict = TRUE;

	/* disable 802.1X authentication by default */
	bsscfg->eap_restrict = FALSE;


	/* disable WPA by default */
	bsscfg->WPA_auth = WPA_AUTH_DISABLED;

	/* Allocate a broadcast SCB for each band */
	if (!(bsscfg->flags & WLC_BSSCFG_NOBCMC)) {
		if (!IS_SINGLEBAND_5G(wlc->deviceid)) {
			if (wlc_bsscfg_bcmcscbinit(wlc, bsscfg, BAND_2G_INDEX))
				return BCME_NOMEM;
		}

		if (NBANDS(wlc) > 1 || IS_SINGLEBAND_5G(wlc->deviceid)) {
			if (wlc_bsscfg_bcmcscbinit(wlc, bsscfg, BAND_5G_INDEX))
				return BCME_NOMEM;
		}
	}

#ifdef SMF_STATS
	if (wlc_bsscfg_smfsinit(wlc, bsscfg))
		return BCME_NOMEM;
#endif

	/* create a new upper-edge driver interface */
	if (!(flags & WLC_BSSCFG_P2P_RESET)) {
		bsscfg->wlcif = wlc_wlcif_alloc(wlc, wlc->osh, WLC_IFTYPE_BSS, wlc->active_queue);
		if (bsscfg->wlcif == NULL) {
			WL_ERROR(("wl%d: %s: failed to alloc wlcif\n",
			          wlc->pub->unit, __FUNCTION__));
			return BCME_NOMEM;
		}
		bsscfg->wlcif->u.bsscfg = bsscfg;

		/* create an OS interface */
		if (bsscfg == wlc_bsscfg_primary(wlc)) {
			/* primary interface has an implicit wlif which is assumed when 
			 * the wlif pointer is NULL.
			 */
			bsscfg->wlcif->flags |= WLC_IF_LINKED;
		}
#ifdef AP
		else {
			if (!BSSCFG_HAS_NOIF(bsscfg)) {
				uint idx = WLC_BSSCFG_IDX(bsscfg);
				bsscfg->wlcif->wlif = wl_add_if(wlc->wl, bsscfg->wlcif, idx, NULL);
				if (bsscfg->wlcif->wlif == NULL) {
					WL_ERROR(("wl%d: _wlc_bsscfg_init: wl_add_if failed for"
						" index %d\n", wlc->pub->unit, idx));
					return BCME_ERROR;
				}
				bsscfg->wlcif->flags |= WLC_IF_LINKED;
			}
			wlc_if_event(wlc, WLC_E_IF_ADD, bsscfg->wlcif);
		}
#endif /* AP */
	}

	wlc_bsscfg_bss_init(wlc, bsscfg);

	/* 11g/11n protections */
	wlc_protection_upd(bsscfg, WLC_PROT_G_OVR, WLC_PROTECTION_AUTO);
	wlc_protection_upd(bsscfg, WLC_PROT_G_SPEC, FALSE);
	wlc_protection_upd(bsscfg, WLC_PROT_N_CFG_OVR, WLC_PROTECTION_AUTO);
	wlc_protection_upd(bsscfg, WLC_PROT_N_CFG, WLC_N_PROTECTION_OFF);
	wlc_protection_upd(bsscfg, WLC_PROT_N_NONGF_OVR, WLC_PROTECTION_AUTO);
	wlc_protection_upd(bsscfg, WLC_PROT_N_NONGF, FALSE);
	wlc_protection_upd(bsscfg, WLC_PROT_OVERLAP, WLC_PROTECTION_CTL_OVERLAP);

	prot = bsscfg->prot_cfg;
	/* initialize CCK preamble mode to unassociated state */
	prot->shortpreamble = FALSE;
	prot->barker_overlap_control = TRUE;
	prot->barker_preamble = WLC_BARKER_SHORT_ALLOWED;
	/* 802.11g draft 4.0 NonERP elt advertisement */
	prot->include_legacy_erp = TRUE;

	/* initialize our proprietary elt */
	brcm_ie = (brcm_ie_t *)&bsscfg->brcm_ie[0];
	bzero((char*)brcm_ie, sizeof(brcm_ie_t));
	brcm_ie->id = DOT11_MNG_PROPR_ID;
	brcm_ie->len = BRCM_IE_LEN - TLV_HDR_LEN;
	bcopy(BRCM_OUI, &brcm_ie->oui[0], DOT11_OUI_LEN);
	brcm_ie->ver = BRCM_IE_VER;

	wlc_bss_update_brcm_ie(wlc, bsscfg);

#ifdef WL11U
	/* add Interworking(IW) IE by application */
	/* if you want to add default IW IE, using the following code */
#ifdef AP
	/* add IW Advertisement Protocol(IWAP) IE by application */
	/* if you want to add default IWAP IE, using the following code */
	/* add IW Roaming Consortium(IWRC) IE by application */
	/* if you want to add default IWRC IE, using the following code */
#endif /* AP */
#endif /* WL11U */
	return BCME_OK;
}

static int
wlc_bsscfg_bcmcscbinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint band)
{
	ASSERT(bsscfg != NULL);
	ASSERT(wlc != NULL);

	if (!bsscfg->bcmc_scb[band]) {
		bsscfg->bcmc_scb[band] =
		        wlc_internalscb_alloc(wlc, &ether_bcast, wlc->bandstate[band]);
		WL_INFORM(("wl%d: wlc_bsscfg_bcmcscbinit: band %d: alloc internal scb 0x%p "
		           "for bsscfg 0x%p\n",
		           wlc->pub->unit, band, bsscfg->bcmc_scb[band], bsscfg));
	}
	if (!bsscfg->bcmc_scb[band]) {
		WL_ERROR(("wl%d: wlc_bsscfg_bcmcscbinit: fail to alloc scb for bsscfg 0x%p\n",
		          wlc->pub->unit, bsscfg));
		return BCME_NOMEM;
	}

	/* make this scb point to bsscfg */
	bsscfg->bcmc_scb[band]->bsscfg = bsscfg;
	bsscfg->bcmc_scb[band]->bandunit = band;

	return  0;
}

#ifdef MBSS
/* Write the base MAC/BSSID into shared memory.  For MBSS, the MAC and BSSID
 * are required to be the same.
 */
int
wlc_write_mbss_basemac(wlc_info_t *wlc, const struct ether_addr *addr)
{
	uint16 mac_l;
	uint16 mac_m;
	uint16 mac_h;

	mac_l = addr->octet[0] | (addr->octet[1] << 8);
	mac_m = addr->octet[2] | (addr->octet[3] << 8);
	/* Mask low bits of BSSID base */
	mac_h = addr->octet[4] | ((addr->octet[5] & ~(wlc->mbss_ucidx_mask)) << 8);

	wlc_write_shm(wlc, SHM_MBSS_BSSID0, mac_l);
	wlc_write_shm(wlc, SHM_MBSS_BSSID1, mac_m);
	wlc_write_shm(wlc, SHM_MBSS_BSSID2, mac_h);

	return BCME_OK;
}

/* Generate a MAC address for the MBSS AP BSS config */
static int
wlc_bsscfg_macgen(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int ii, jj;
	bool collision = TRUE;
	int cfg_idx = WLC_BSSCFG_IDX(cfg);
	struct ether_addr newmac;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	if (ETHER_ISNULLADDR(&wlc->vether_base)) {
		/* initialize virtual MAC base for MBSS
		 * the base should come from an external source,
		 * this initialization is in case one isn't provided
		 */
		bcopy(&wlc->pub->cur_etheraddr, &wlc->vether_base, ETHER_ADDR_LEN);
		/* avoid collision */
		wlc->vether_base.octet[5] += 1;

		/* force locally administered address */
		ETHER_SET_LOCALADDR(&wlc->vether_base);
	}

	bcopy(&wlc->vether_base, &newmac, ETHER_ADDR_LEN);

	/* brute force attempt to make a MAC for this interface,
	 * the user didn't provide one.
	 * outside loop limits the # of times we increment the low byte of
	 * the MAC address we're attempting to create, and the inner loop
	 * checks for collisions with other configs.
	 */
	for (ii = 0; (ii < WLC_MAXBSSCFG) && (collision == TRUE); ii++) {
		collision = FALSE;
		for (jj = 0; jj < WLC_MAXBSSCFG; jj++) {
			/* don't compare with the bss config we're updating */
			if (jj == cfg_idx || (!wlc->bsscfg[jj]))
				continue;
			if (EADDR_TO_UC_IDX(wlc->bsscfg[jj]->cur_etheraddr, wlc->mbss_ucidx_mask) ==
			    EADDR_TO_UC_IDX(newmac, wlc->mbss_ucidx_mask)) {
				collision = TRUE;
				break;
			}
		}
		if (collision == TRUE) /* increment and try again */
			newmac.octet[5] = (newmac.octet[5] & ~(wlc->mbss_ucidx_mask))
			        | (wlc->mbss_ucidx_mask & (newmac.octet[5]+1));
		else
			bcopy(&newmac, &cfg->cur_etheraddr, ETHER_ADDR_LEN);
	}

	if (ETHER_ISNULLADDR(&cfg->cur_etheraddr)) {
		WL_MBSS(("wl%d.%d: wlc_bsscfg_macgen couldn't generate MAC address\n",
		         wlc->pub->unit, cfg_idx));

		return BCME_BADADDR;
	}
	else {
		WL_MBSS(("wl%d.%d: wlc_bsscfg_macgen assigned MAC %s\n",
		         wlc->pub->unit, cfg_idx,
		         bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf)));
		return BCME_OK;
	}
}
#endif /* MBSS */

#if defined(AP)
uint16
wlc_bsscfg_newaid(wlc_bsscfg_t *cfg)
{
	int pos;

	ASSERT(cfg);

	/* get an unused number from aidmap */
	for (pos = 0; pos < cfg->wlc->pub->tunables->maxscb; pos++) {
		if (isclr(cfg->aidmap, pos)) {
			WL_ASSOC(("wlc_bsscfg_newaid marking bit = %d for "
			          "bsscfg %d AIDMAP\n", pos,
			          WLC_BSSCFG_IDX(cfg)));
			/* mark the position being used */
			setbit(cfg->aidmap, pos);
			break;
		}
	}
	ASSERT(pos < cfg->wlc->pub->tunables->maxscb);

	return ((uint16)AIDMAP2AID(pos));
}
#endif /* AP */

#ifdef STA
/* Set/reset association parameters */
int
wlc_bsscfg_assoc_params_set(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len)
{
	ASSERT(wlc != NULL);
	ASSERT(bsscfg != NULL);

	if (bsscfg->assoc_params != NULL) {
		MFREE(wlc->osh, bsscfg->assoc_params, bsscfg->assoc_params_len);
		bsscfg->assoc_params = NULL;
		bsscfg->assoc_params_len = 0;
	}
	if (assoc_params == NULL || assoc_params_len == 0)
		return BCME_OK;
	if ((bsscfg->assoc_params = MALLOC(wlc->osh, assoc_params_len)) == NULL) {
		WL_ERROR(("wl%d: wlc_bsscfg_assoc_params_set: out of memory for bsscfg, "
		          "malloced %d bytes\n", wlc->pub->unit, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}
	bcopy(assoc_params, bsscfg->assoc_params, assoc_params_len);
	bsscfg->assoc_params_len = (uint16)assoc_params_len;

	return BCME_OK;
}

void
wlc_bsscfg_assoc_params_reset(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	if (bsscfg != NULL)
		wlc_bsscfg_assoc_params_set(wlc, bsscfg, NULL, 0);
}

/* Set/reset scan parameters */
int
wlc_bsscfg_scan_params_set(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wl_join_scan_params_t *scan_params)
{
	ASSERT(bsscfg != NULL);

	if (scan_params == NULL) {
		if (bsscfg->scan_params != NULL) {
			MFREE(wlc->osh, bsscfg->scan_params, sizeof(wl_join_scan_params_t));
			bsscfg->scan_params = NULL;
		}
		return BCME_OK;
	}
	else if (bsscfg->scan_params != NULL ||
	         (bsscfg->scan_params = MALLOC(wlc->osh, sizeof(wl_join_scan_params_t))) != NULL) {
		bcopy(scan_params, bsscfg->scan_params, sizeof(wl_join_scan_params_t));
		return BCME_OK;
	}

	WL_ERROR(("wl%d: wlc_bsscfg_scan_params_set: out of memory for bsscfg, "
		          "malloced %d bytes\n", wlc->pub->unit, MALLOCED(wlc->osh)));
	return BCME_NOMEM;
}

void
wlc_bsscfg_scan_params_reset(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	if (bsscfg != NULL)
		wlc_bsscfg_scan_params_set(wlc, bsscfg, NULL);
}
#endif /* STA */

void
wlc_bsscfg_SSID_set(wlc_bsscfg_t *bsscfg, uint8 *SSID, int len)
{
	ASSERT(bsscfg != NULL);
	ASSERT(len <= DOT11_MAX_SSID_LEN);

	if ((bsscfg->SSID_len = (uint8)len) > 0) {
		ASSERT(SSID != NULL);
		/* need to use memove here to handle overlapping copy */
		memmove(bsscfg->SSID, SSID, len);

		if (len < DOT11_MAX_SSID_LEN)
			bzero(&bsscfg->SSID[len], DOT11_MAX_SSID_LEN - len);
		return;
	}

	bzero(bsscfg->SSID, DOT11_MAX_SSID_LEN);
}

/*
 * Vendor IE lists
 */

#if defined(AP)
void
wlc_vndr_ie_free(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc;
	vndr_ie_listel_t *curr_list_el;
	vndr_ie_listel_t *next_list_el;
	int freelen;

	wlc = bsscfg->wlc;
	curr_list_el = bsscfg->vndr_ie_listp;

	while (curr_list_el != NULL) {
		next_list_el = curr_list_el->next_el;

		freelen =
			VNDR_IE_EL_HDR_LEN +
			sizeof(uint32) +
			VNDR_IE_HDR_LEN +
			curr_list_el->vndr_ie_infoel.vndr_ie_data.len;

		MFREE(wlc->osh, curr_list_el, freelen);
		curr_list_el = next_list_el;
	}

	bsscfg->vndr_ie_listp = NULL;
}
#endif 

/* return the total length of the buffer when it >= 0
 * otherwise return value is error BCME_XXXX
 */
int
wlc_vndr_ie_buflen(vndr_ie_buf_t *ie_buf, int len, int *bcn_ielen, int *prbrsp_ielen)
{
	int totie, ieindex;
	vndr_ie_info_t *ie_info;
	vndr_ie_t *vndr_iep;
	int ie_len, info_len;
	char *bufaddr;
	uint32 pktflag;
	int bcn_len, prbrsp_len;

	if (len < (int) sizeof(vndr_ie_buf_t) - 1) {
		return BCME_BUFTOOSHORT;
	}

	bcn_len = prbrsp_len = 0;

	bcopy(&ie_buf->iecount, &totie, (int) sizeof(int));

	bufaddr = (char *) &ie_buf->vndr_ie_list;
	len -= (int) sizeof(int);       /* reduce by the size of iecount */

	for (ieindex = 0; ieindex < totie; ieindex++) {
		if (len < (int) sizeof(vndr_ie_info_t) - 1) {
			return BCME_BUFTOOSHORT;
		}

		ie_info = (vndr_ie_info_t *) bufaddr;
		bcopy(&ie_info->pktflag, &pktflag, (int) sizeof(uint32));

		vndr_iep = &ie_info->vndr_ie_data;
		ie_len = (int) (vndr_iep->len + VNDR_IE_HDR_LEN);
		info_len = (int) sizeof(uint32) + ie_len;

		if (pktflag & VNDR_IE_BEACON_FLAG) {
			bcn_len += ie_len;
		}

		if (pktflag & VNDR_IE_PRBRSP_FLAG) {
			prbrsp_len += ie_len;
		}

		/* reduce the bufer length by the size of this vndr_ie_info */
		len -= info_len;

		/* point to the next vndr_ie_info */
		bufaddr += info_len;
	}

	if (len < 0) {
		return BCME_BUFTOOSHORT;
	}

	if (bcn_ielen) {
		*bcn_ielen = bcn_len;
	}

	if (prbrsp_ielen) {
		*prbrsp_ielen = prbrsp_len;
	}

	return (int)((uint8 *)bufaddr - (uint8 *)ie_buf);
}

int
wlc_vndr_ie_getlen_ext(wlc_bsscfg_t *bsscfg, vndr_ie_filter_fn_t filter,
	uint32 pktflag, int *totie)
{
	vndr_ie_listel_t *curr;
	int ie_count = 0;
	int tot_ielen = 0;

	for (curr = bsscfg->vndr_ie_listp; curr != NULL; curr = curr->next_el) {
		if ((curr->vndr_ie_infoel.pktflag & pktflag) &&
		    (filter == NULL ||
		     (filter)(bsscfg, &curr->vndr_ie_infoel.vndr_ie_data))) {
			ie_count++;
			tot_ielen += curr->vndr_ie_infoel.vndr_ie_data.len + VNDR_IE_HDR_LEN;
		}
	}

	if (totie) {
		*totie = ie_count;
	}

	return tot_ielen;
}

uint8 *
wlc_vndr_ie_write_ext(wlc_bsscfg_t *bsscfg, vndr_ie_write_filter_fn_t filter,
	uint type, uint8 *cp, int buflen, uint32 pktflag)
{
	vndr_ie_listel_t *curr;
	uint8 *old_cp;

	for (curr = bsscfg->vndr_ie_listp; curr != NULL; curr = curr->next_el) {
		if (curr->vndr_ie_infoel.pktflag & pktflag) {
			old_cp = cp;
			if (curr->vndr_ie_infoel.pktflag & VNDR_IE_CUSTOM_FLAG) {
#ifdef BCMDBG
				WL_ERROR(("CUSTOMER IE: id=%d, len=%d\n",
					curr->vndr_ie_infoel.vndr_ie_data.id,
					curr->vndr_ie_infoel.vndr_ie_data.len));
#endif /* BCMDBG */
				cp = wlc_write_info_elt_safe(cp, buflen,
					curr->vndr_ie_infoel.vndr_ie_data.id,
					curr->vndr_ie_infoel.vndr_ie_data.len,
					&curr->vndr_ie_infoel.vndr_ie_data.oui[0]);
			}
			else if (filter == NULL ||
			         (filter)(bsscfg, type, &curr->vndr_ie_infoel.vndr_ie_data)) {
				cp = wlc_write_info_elt_safe(cp, buflen,
					DOT11_MNG_PROPR_ID,
					curr->vndr_ie_infoel.vndr_ie_data.len,
					&curr->vndr_ie_infoel.vndr_ie_data.oui[0]);
			}
			buflen -= (int)(cp - old_cp);
		}
	}

	return cp;
}

#ifdef WLP2P
static bool
wlc_p2p_vndr_ie_filter(wlc_bsscfg_t *cfg, vndr_ie_t *ie)
{
	uint8 *parse;
	uint parse_len;

	return bcm_is_p2p_ie((uint8 *)ie, &parse, &parse_len);
}

int
wlc_p2p_vndr_ie_getlen(wlc_bsscfg_t *bsscfg, uint32 pktflag, int *totie)
{
	return wlc_vndr_ie_getlen_ext(bsscfg, wlc_p2p_vndr_ie_filter, pktflag, totie);
}

static bool
wlc_p2p_vndr_ie_write_filter(wlc_bsscfg_t *cfg, uint type, vndr_ie_t *ie)
{
	uint8 *parse;
	uint parse_len;

	if (type != FC_PROBE_RESP ||
	    (cfg->p2p_flags & WLC_BSSCFG_P2P_P2P_IE) ||
	    !bcm_is_p2p_ie((uint8 *)ie, &parse, &parse_len))
		return TRUE;

	return FALSE;
}

uint8 *
wlc_p2p_vndr_ie_write(wlc_bsscfg_t *bsscfg, uint type, uint8 *cp, int buflen, uint32 pktflag)
{
	return wlc_vndr_ie_write_ext(bsscfg, wlc_p2p_vndr_ie_write_filter, type,
	                             cp, buflen, pktflag);
}
#endif /* WLP2P */

/*
 * Create a vendor IE information element object and add to the list.
 * Return value: address of the new object.
 */
vndr_ie_listel_t *
wlc_vndr_ie_add_elem(wlc_bsscfg_t *bsscfg, uint32 pktflag, vndr_ie_t *vndr_iep)
{
	wlc_info_t *wlc;
	vndr_ie_listel_t *new_list_el;
	int info_len, ie_len;
	vndr_ie_listel_t *last;

	wlc = bsscfg->wlc;
	ie_len = (int) (vndr_iep->len + VNDR_IE_HDR_LEN);
	info_len = (int) (VNDR_IE_INFO_HDR_LEN + ie_len);

	if ((new_list_el = (vndr_ie_listel_t *) MALLOC(wlc->osh, info_len +
		VNDR_IE_EL_HDR_LEN)) == NULL) {
		WL_ERROR(("wl%d: wlc_vndr_ie_add_elem: out of memory\n", wlc->pub->unit));
		return NULL;
	}

	new_list_el->vndr_ie_infoel.pktflag = pktflag;
	bcopy((char *)vndr_iep, (char *)&new_list_el->vndr_ie_infoel.vndr_ie_data, ie_len);

	/* Add to the tail of the list */
	for (last = bsscfg->vndr_ie_listp;
	     last != NULL && last->next_el != NULL;
	     last = last->next_el)
		;
	new_list_el->next_el = NULL;
	if (last != NULL)
		last->next_el = new_list_el;
	else
		bsscfg->vndr_ie_listp = new_list_el;

	return (new_list_el);
}

int
wlc_vndr_ie_add(wlc_bsscfg_t *bsscfg, vndr_ie_buf_t *ie_buf, int len)
{
	wlc_info_t *wlc;
	vndr_ie_info_t *ie_info;
	vndr_ie_t *vndr_iep;
	int info_len;
	int totie, ieindex;
	uint32 currflag;
	char *bufaddr;

	wlc = bsscfg->wlc;

	bcopy(&ie_buf->iecount, &totie, sizeof(int));
	bufaddr = (char *) &ie_buf->vndr_ie_list;

	for (ieindex = 0; ieindex < totie; ieindex++) {
		ie_info = (vndr_ie_info_t *) bufaddr;
		bcopy((char *)&ie_info->pktflag, (char *)&currflag, (int) sizeof(uint32));

		vndr_iep = &ie_info->vndr_ie_data;

		if (!(currflag & VNDR_IE_CUSTOM_FLAG))
			vndr_iep->id = DOT11_MNG_PROPR_ID;

		info_len = (int) (VNDR_IE_INFO_HDR_LEN + VNDR_IE_HDR_LEN + vndr_iep->len);
		if (wlc_vndr_ie_add_elem(bsscfg, currflag, vndr_iep) == NULL) {
			return BCME_NORESOURCE;
		}
		/* point to the next vndr_ie_info */
		bufaddr += info_len;
	}

	return 0;
}

int
wlc_vndr_ie_del(wlc_bsscfg_t *bsscfg, vndr_ie_buf_t *ie_buf, int len)
{
	wlc_info_t *wlc;
	vndr_ie_listel_t *prev_list_el;
	vndr_ie_listel_t *curr_list_el;
	vndr_ie_listel_t *next_list_el;
	vndr_ie_info_t *ie_info;
	vndr_ie_t *vndr_iep;
	int ie_len, info_len;
	int totie, ieindex;
	uint32 currflag;
	char *bufaddr;
	bool found;
	int err = 0;

	wlc = bsscfg->wlc;

	bcopy(&ie_buf->iecount, &totie, sizeof(int));
	bufaddr = (char *) &ie_buf->vndr_ie_list;

	for (ieindex = 0; ieindex < totie; ieindex++) {
		ie_info = (vndr_ie_info_t *) bufaddr;
		bcopy((char *)&ie_info->pktflag, (char *)&currflag, (int) sizeof(uint32));

		vndr_iep = &ie_info->vndr_ie_data;
		if (!(currflag & VNDR_IE_CUSTOM_FLAG))
			vndr_iep->id = DOT11_MNG_PROPR_ID;

		ie_len = (int) (vndr_iep->len + VNDR_IE_HDR_LEN);
		info_len = (int) sizeof(uint32) + ie_len;

		curr_list_el = bsscfg->vndr_ie_listp;
		prev_list_el = NULL;

		found = FALSE;

		while (curr_list_el != NULL) {
			next_list_el = curr_list_el->next_el;

			if (vndr_iep->len == curr_list_el->vndr_ie_infoel.vndr_ie_data.len) {
				if (!bcmp((char*)&curr_list_el->vndr_ie_infoel, (char*) ie_info,
					info_len)) {
					if (bsscfg->vndr_ie_listp == curr_list_el) {
						bsscfg->vndr_ie_listp = next_list_el;
					} else {
						prev_list_el->next_el = next_list_el;
					}
					MFREE(wlc->osh, curr_list_el, info_len +
					      VNDR_IE_EL_HDR_LEN);
					curr_list_el = NULL;
					found = TRUE;
					break;
				}
			}

			prev_list_el = curr_list_el;
			curr_list_el = next_list_el;
		}

		if (!found) {
			WL_ERROR(("wl%d: wlc_del_ie: IE not in list\n", wlc->pub->unit));
			err = BCME_NOTFOUND;
		}

		/* point to the next vndr_ie_info */
		bufaddr += info_len;
	}

	return err;
}

int
wlc_vndr_ie_get(wlc_bsscfg_t *bsscfg, vndr_ie_buf_t *ie_buf, int len, uint32 pktflag)
{
	wlc_info_t *wlc;
	int copylen;
	int totie;
	vndr_ie_listel_t *curr_list_el;
	vndr_ie_info_t *ie_info;
	vndr_ie_t *vndr_iep;
	char *bufaddr;
	int ie_len, info_len;

	wlc = bsscfg->wlc;
	pktflag &= (~(VNDR_IE_CUSTOM_FLAG));
	/* Vendor IE data */
	copylen = wlc_vndr_ie_getlen(bsscfg, pktflag, &totie);

	if (totie != 0) {
		/* iecount */
		copylen += (int) sizeof(int);

		/* pktflag for each vndr_ie_info struct */
		copylen += (int) sizeof(uint32) * totie;
	} else {
		copylen = (int) sizeof(vndr_ie_buf_t) - sizeof(vndr_ie_info_t);
	}
	if (len < copylen) {
		WL_ERROR(("wl%d: wlc_vndr_ie_get: buf too small (copylen=%d, buflen=%d)\n",
			wlc->pub->unit, copylen, len));
		/* Store the required buffer size value in the buffer provided */
		bcopy((char *) &copylen, (char *)ie_buf, sizeof(int));
		return BCME_BUFTOOSHORT;
	}

	bcopy(&totie, &ie_buf->iecount, sizeof(int));

	if (totie == 0)
		return BCME_OK;

	bufaddr = (char *) &ie_buf->vndr_ie_list;

	curr_list_el = bsscfg->vndr_ie_listp;

	while (curr_list_el != NULL) {
		if (curr_list_el->vndr_ie_infoel.pktflag & pktflag) {
			ie_info = (vndr_ie_info_t *) bufaddr;
			vndr_iep = &curr_list_el->vndr_ie_infoel.vndr_ie_data;

			ie_len = (int) (vndr_iep->len + VNDR_IE_HDR_LEN);
			info_len = (int) sizeof(uint32) + ie_len;

			bcopy((char*)&curr_list_el->vndr_ie_infoel, (char*) ie_info, info_len);

			/* point to the next vndr_ie_info */
			bufaddr += info_len;
		}
		curr_list_el = curr_list_el->next_el;
	}

	return BCME_OK;
}

/*
 * Modify the data in the previously added vendor IE info.
 */
vndr_ie_listel_t *
wlc_vndr_ie_mod_elem(wlc_bsscfg_t *bsscfg, vndr_ie_listel_t *old_listel,
	uint32 pktflag, vndr_ie_t *vndr_iep)
{
	wlc_info_t *wlc;
	vndr_ie_listel_t *curr_list_el, *prev_list_el;

	wlc = bsscfg->wlc;
	curr_list_el = bsscfg->vndr_ie_listp;
	prev_list_el = NULL;

	while (curr_list_el != NULL) {
		/* found list element, update the vendor info */
		if (curr_list_el == old_listel) {
			/* reuse buffer if length of current elem is same as the new */
			if (curr_list_el->vndr_ie_infoel.vndr_ie_data.len == vndr_iep->len) {
				curr_list_el->vndr_ie_infoel.pktflag = pktflag;
				bcopy((char *)vndr_iep,
				      (char *)&curr_list_el->vndr_ie_infoel.vndr_ie_data,
				      (vndr_iep->len + VNDR_IE_HDR_LEN));

				return (curr_list_el);
			} else {
				/* Delete the old one from the list and free it */
				if (bsscfg->vndr_ie_listp == curr_list_el) {
					bsscfg->vndr_ie_listp = curr_list_el->next_el;
				} else {
					prev_list_el->next_el = curr_list_el->next_el;
				}
				MFREE(wlc->osh, curr_list_el,
				      (curr_list_el->vndr_ie_infoel.vndr_ie_data.len +
				       VNDR_IE_HDR_LEN + VNDR_IE_INFO_HDR_LEN +
				       VNDR_IE_EL_HDR_LEN));

				/* Add a new elem to the list */
				return wlc_vndr_ie_add_elem(bsscfg, pktflag, vndr_iep);
			}
		}

		prev_list_el = curr_list_el;
		curr_list_el = curr_list_el->next_el;
	}

	/* Should not come here */
	ASSERT(0);

	return 0;
}

int
wlc_vndr_ie_mod_elem_by_type(wlc_bsscfg_t *bsscfg, uint8 type,
	uint32 pktflag, vndr_ie_t *vndr_iep)
{
	wlc_info_t *wlc;
	vndr_ie_listel_t *curr_list_el, *prev_list_el;

	wlc = bsscfg->wlc;
	curr_list_el = bsscfg->vndr_ie_listp;
	prev_list_el = NULL;

	while (curr_list_el != NULL) {
		/* found list element, update the IE */
		if (type == curr_list_el->vndr_ie_infoel.vndr_ie_data.id) {
			/* reuse buffer if length of current elem is same as the new */
			if (curr_list_el->vndr_ie_infoel.vndr_ie_data.len == vndr_iep->len) {
				curr_list_el->vndr_ie_infoel.pktflag = pktflag;
				bcopy((char *)vndr_iep,
					(char *)&curr_list_el->vndr_ie_infoel.vndr_ie_data,
					(vndr_iep->len + VNDR_IE_HDR_LEN));
				return BCME_OK;
			} else {
				/* Delete the old one from the list and free it */
				if (bsscfg->vndr_ie_listp == curr_list_el) {
					bsscfg->vndr_ie_listp = curr_list_el->next_el;
				} else {
					prev_list_el->next_el = curr_list_el->next_el;
				}
				MFREE(wlc->osh, curr_list_el,
					(curr_list_el->vndr_ie_infoel.vndr_ie_data.len +
					VNDR_IE_HDR_LEN + VNDR_IE_INFO_HDR_LEN +
					VNDR_IE_EL_HDR_LEN));
				break;
			}
		}
		prev_list_el = curr_list_el;
		curr_list_el = curr_list_el->next_el;
	}

	/* Add a new elem to the list */
	if (!wlc_vndr_ie_add_elem(bsscfg, pktflag, vndr_iep))
		return BCME_NOMEM;
	return BCME_OK;
}

int
wlc_vndr_ie_del_by_type(wlc_bsscfg_t *bsscfg, uint8 type)
{
	wlc_info_t *wlc;
	vndr_ie_listel_t *prev_list_el;
	vndr_ie_listel_t *curr_list_el;
	vndr_ie_listel_t *next_list_el;
	bool found = FALSE;
	int err = BCME_OK;

	wlc = bsscfg->wlc;
	curr_list_el = bsscfg->vndr_ie_listp;
	prev_list_el = NULL;

	while (curr_list_el != NULL) {
		next_list_el = curr_list_el->next_el;
		if (type == curr_list_el->vndr_ie_infoel.vndr_ie_data.id) {
			if (bsscfg->vndr_ie_listp == curr_list_el) {
				bsscfg->vndr_ie_listp = next_list_el;
			} else {
				prev_list_el->next_el = next_list_el;
			}
			MFREE(wlc->osh, curr_list_el,
				(curr_list_el->vndr_ie_infoel.vndr_ie_data.len +
				VNDR_IE_HDR_LEN + VNDR_IE_INFO_HDR_LEN +
				VNDR_IE_EL_HDR_LEN));
			found = TRUE;
			break;
		}
		prev_list_el = curr_list_el;
		curr_list_el = next_list_el;
	}

	if (!found) {
		err = BCME_NOTFOUND;
	}

	return err;
}

uint8 *
wlc_vndr_ie_find_by_type(wlc_bsscfg_t *bsscfg, uint8 type)
{
	vndr_ie_listel_t *curr_list_el;

	curr_list_el = bsscfg->vndr_ie_listp;

	while (curr_list_el != NULL) {
		if (type == curr_list_el->vndr_ie_infoel.vndr_ie_data.id) {
			return (uint8 *)&curr_list_el->vndr_ie_infoel.vndr_ie_data;
		}
		curr_list_el = curr_list_el->next_el;
	}

	return NULL;
}

uint8 *
wlc_bsscfg_get_ie(wlc_bsscfg_t *bsscfg, uint8 ie_type)
{
	uint8 *ie_data = NULL;

	switch (ie_type) {
#ifdef WL11U
		case DOT11_MNG_INTERWORKING_ID:
			ie_data = bsscfg->iw_ie;
			break;
#endif /* WL11U */
		default:
			ie_data = wlc_vndr_ie_find_by_type(bsscfg, ie_type);
			break;
	}
	return ie_data;
}

int
wlc_bsscfg_set_ie(wlc_bsscfg_t *bsscfg, uint8 *ie_data,
	bool *bcn_upd, bool *prbresp_upd)
{
	int err = BCME_OK;
	int ie_len;
	uint8 ie_type;

	ie_type = ie_data[TLV_TAG_OFF];
	ie_len = ie_data[TLV_LEN_OFF] + TLV_HDR_LEN;

	switch (ie_type) {
#ifdef WL11U
		case DOT11_MNG_INTERWORKING_ID:
			*bcn_upd = TRUE;
			*prbresp_upd = TRUE;
			break;
#ifdef AP
		case DOT11_MNG_ADVERTISEMENT_ID:
		case DOT11_MNG_ROAM_CONSORT_ID:
			if (bsscfg->iw_ie != NULL) {
				*bcn_upd = TRUE;
				*prbresp_upd = TRUE;
			}
			break;
#endif /* AP */
#endif /* WL11U */
		default:
			break;
	}

	if (ie_len == TLV_HDR_LEN) {
		/* delete the IE if len is zero */
		wlc_vndr_ie_del_by_type(bsscfg, ie_type);
	} else {
		/* update the IE */
		err = wlc_vndr_ie_mod_elem_by_type(bsscfg, ie_type,
			VNDR_IE_CUSTOM_FLAG, (vndr_ie_t *)ie_data);
	}

	/* update the pointer to the TLV field in the list for quick access */
	switch (ie_type) {
#ifdef WL11U
		case DOT11_MNG_INTERWORKING_ID:
			bsscfg->iw_ie = wlc_vndr_ie_find_by_type(bsscfg,
				DOT11_MNG_INTERWORKING_ID);
			break;
#endif /* WL11U */
		default:
			break;
	}

	return err;
}


#ifdef SMF_STATS
static void
_wlc_bsscfg_smfsinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	uint8 i;
	wlc_smf_stats_t *smf_stats;

	ASSERT(bsscfg->smfs_info);

	bzero(bsscfg->smfs_info, sizeof(wlc_smfs_info_t));

	bsscfg->smfs_info->enable = 1;

	for (i = 0; i < SMFS_TYPE_MAX; i++) {
		smf_stats = &bsscfg->smfs_info->smf_stats[i];

		smf_stats->smfs_main.type = i;
		smf_stats->smfs_main.version = SMFS_VERSION;

		if ((i == SMFS_TYPE_AUTH) || (i == SMFS_TYPE_ASSOC) ||
			(i == SMFS_TYPE_REASSOC))
			smf_stats->smfs_main.codetype = SMFS_CODETYPE_SC;
		else
			smf_stats->smfs_main.codetype = SMFS_CODETYPE_RC;
	}

}
int
wlc_bsscfg_smfsinit(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg)
{
	if (!bsscfg->smfs_info) {
		bsscfg->smfs_info = MALLOC(wlc->osh, sizeof(wlc_smfs_info_t));
		if (!bsscfg->smfs_info) {
			WL_ERROR(("wl%d: %s out of memory for bsscfg, "
				"malloced %d bytes\n", wlc->pub->unit, __FUNCTION__,
				MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
	}

	_wlc_bsscfg_smfsinit(wlc, bsscfg);

	return 0;

}

static int
smfs_elem_free(struct wlc_info *wlc, wlc_smf_stats_t *smf_stats)
{
	wlc_smfs_elem_t *headptr = smf_stats->stats;
	wlc_smfs_elem_t *curptr;

	while (headptr) {
		curptr = headptr;
		headptr = headptr->next;
		MFREE(wlc->osh, curptr, sizeof(wlc_smfs_elem_t));
	}
	smf_stats->stats = NULL;
	return 0;
}

static int
wlc_bsscfg_smfsfree(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg)
{
	int i;

	if (!bsscfg->smfs_info)
		return 0;

	for (i = 0; i < SMFS_TYPE_MAX; i++) {
		wlc_smf_stats_t *smf_stats = &bsscfg->smfs_info->smf_stats[i];
		smfs_elem_free(wlc, smf_stats);
	}
	MFREE(wlc->osh, bsscfg->smfs_info, sizeof(wlc_smfs_info_t));
	bsscfg->smfs_info = NULL;

	return 0;
}

static int
linear_search_u16(const uint16 array[], uint16 key, int size)
{
	int n;
	for (n = 0; n < size; ++n) {
		if (array[ n ] == key) {
			return n;
		}
	}
	return -1;
}

static wlc_smfs_elem_t *
smfs_elem_create(osl_t *osh, uint16 code)
{
	wlc_smfs_elem_t *elem = NULL;
	elem = MALLOC(osh, sizeof(wlc_smfs_elem_t));

	if (elem) {
		elem->next = NULL;
		elem->smfs_elem.code = code;
		elem->smfs_elem.count = 0;
	}

	return elem;
}

static wlc_smfs_elem_t *
smfs_elem_find(uint16 code, wlc_smfs_elem_t *start)
{
	while (start != NULL) {
		if (code == start->smfs_elem.code)
			break;
		start = start->next;
	}
	return start;
}

/* sort based on code define */
static void
smfs_elem_insert(wlc_smfs_elem_t **rootp, wlc_smfs_elem_t *new)
{
	wlc_smfs_elem_t *curptr;
	wlc_smfs_elem_t *previous;

	curptr = *rootp;
	previous = NULL;

	while (curptr && (curptr->smfs_elem.code < new->smfs_elem.code)) {
		previous = curptr;
		curptr = curptr->next;
	}
	new->next = curptr;

	if (previous == NULL)
		*rootp = new;
	else
		previous->next = new;
}

static bool
smfstats_codetype_included(uint16 code, uint16 codetype)
{
	bool included = FALSE;
	int indx = -1;

	if (codetype == SMFS_CODETYPE_SC)
		indx = linear_search_u16(smfs_sc_table, code,
		  sizeof(smfs_sc_table)/sizeof(uint16));
	else
		indx = linear_search_u16(smfs_rc_table, code,
		  sizeof(smfs_rc_table)/sizeof(uint16));

	if (indx != -1)
		included = TRUE;

	return included;
}

static int
smfstats_update(wlc_info_t *wlc, wlc_smf_stats_t *smf_stats, uint16 code)
{
	uint8 codetype = smf_stats->smfs_main.codetype;
	uint32 count_excl = smf_stats->count_excl;
	wlc_smfs_elem_t *elem = smf_stats->stats;
	wlc_smfs_elem_t *new_elem = NULL;
	bool included = smfstats_codetype_included(code, codetype);
	osl_t *osh;

	if (!included && (count_excl > MAX_SCRC_EXCLUDED)) {
		WL_INFORM(("%s: sc/rc  outside the scope, discard\n", __FUNCTION__));
		return 0;
	}

	osh = wlc->osh;
	new_elem = smfs_elem_find(code, elem);

	if (!new_elem) {
		new_elem = smfs_elem_create(osh, code);

		if (!new_elem) {
			WL_ERROR(("wl%d: %s: out of memory for smfs_elem, "
					  "malloced %d bytes\n", wlc->pub->unit, __FUNCTION__,
					  MALLOCED(osh)));
			return BCME_NOMEM;
		}
		else {
			smfs_elem_insert(&smf_stats->stats, new_elem);
			if (!included)
				smf_stats->count_excl++;
			smf_stats->smfs_main.count_total++;
		}
	}
	new_elem->smfs_elem.count++;

	return 0;
}

int
wlc_smfstats_update(struct wlc_info *wlc, wlc_bsscfg_t *cfg, uint8 smfs_type, uint16 code)
{
	wlc_smf_stats_t *smf_stats;
	int err = 0;

	ASSERT(cfg->smfs_info);

	if (!SMFS_ENAB(cfg))
		return err;

	smf_stats = &cfg->smfs_info->smf_stats[smfs_type];

	if (code == SMFS_CODE_MALFORMED) {
		smf_stats->smfs_main.malformed_cnt++;
		return 0;
	}

	if (code == SMFS_CODE_IGNORED) {
		smf_stats->smfs_main.ignored_cnt++;
		return 0;
	}

	err = smfstats_update(wlc, smf_stats, code);

	return err;
}

int
wlc_bsscfg_get_smfs(wlc_bsscfg_t *cfg, int idx, char *buf, int len)
{
	wlc_smf_stats_t *smf_stat;
	wlc_smfs_elem_t *elemt;
	int used_len = 0;
	int err = 0;

	ASSERT((uint)len >= sizeof(wl_smf_stats_t));

	if (idx < 0 || idx >= SMFS_TYPE_MAX) {
		err = BCME_RANGE;
		return err;
	}

	smf_stat =  &cfg->smfs_info->smf_stats[idx];
	bcopy(&smf_stat->smfs_main, buf, sizeof(wl_smf_stats_t));

	buf += WL_SMFSTATS_FIXED_LEN;
	used_len += WL_SMFSTATS_FIXED_LEN;

	elemt = smf_stat->stats;

	while (elemt) {
		used_len += sizeof(wl_smfs_elem_t);
		if (used_len > len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		bcopy(&elemt->smfs_elem, buf, sizeof(wl_smfs_elem_t));
		elemt = elemt->next;
		buf += sizeof(wl_smfs_elem_t);
	}
	return err;
}

int
wlc_bsscfg_clear_smfs(struct wlc_info *wlc, wlc_bsscfg_t *cfg)
{
	int i;

	if (!cfg->smfs_info)
		return 0;

	for (i = 0; i < SMFS_TYPE_MAX; i++) {
		wlc_smf_stats_t *smf_stats = &cfg->smfs_info->smf_stats[i];
		smfs_elem_free(wlc, smf_stats);

		smf_stats->smfs_main.length = 0;
		smf_stats->smfs_main.ignored_cnt = 0;
		smf_stats->smfs_main.malformed_cnt = 0;
		smf_stats->smfs_main.count_total = 0;
		smf_stats->count_excl = 0;
	}
	return 0;
}
#endif /* SMF_STATS */

#ifdef WL_BSSCFG_TX_SUPR
void
wlc_bsscfg_tx_stop(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = bsscfg->wlc;

	/* Nothing to do */
	if (BSS_TX_SUPR(bsscfg))
		return;

	bsscfg->flags |= WLC_BSSCFG_TX_SUPR;

	/* If there is anything in the data fifo then allow it to drain */
	if (TXPKTPENDTOT(wlc))
		wlc->block_datafifo |= DATA_BLOCK_TX_SUPR;
}

/* Call after the FIFO has drained */
void
wlc_bsscfg_tx_check(wlc_info_t *wlc)
{
	ASSERT(!TXPKTPENDTOT(wlc));

	if (wlc->block_datafifo & DATA_BLOCK_TX_SUPR) {
		int i;
		wlc_bsscfg_t *bsscfg;

		wlc->block_datafifo &= ~DATA_BLOCK_TX_SUPR;

		/* Now complete all the pending transitions */
		FOREACH_BSS(wlc, i, bsscfg) {
			if (bsscfg->tx_start_pending) {
				bsscfg->tx_start_pending = FALSE;
				wlc_bsscfg_tx_start(bsscfg);
			}
		}
	}
}

void
wlc_bsscfg_tx_start(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = bsscfg->wlc;
	struct pktq *txq;
	void *pkt;
	int prec;

	/* Nothing to do */
	if (!BSS_TX_SUPR(bsscfg))
		return;

	if (wlc->block_datafifo & DATA_BLOCK_TX_SUPR) {
		/* Finish the transition first to avoid reordering frames */
		if (TXPKTPENDTOT(bsscfg->wlc)) {
			bsscfg->tx_start_pending = TRUE;
			return;
		}
		wlc->block_datafifo &= ~DATA_BLOCK_TX_SUPR;
	}

	bsscfg->flags &= ~WLC_BSSCFG_TX_SUPR;

	/* Dump all the packets from bsscfg->psq to txq but to the front */
	/* This is done to preserve the ordering w/o changing the precedence level
	 * since AMPDU module keeps track of sequence numbers according to their
	 * precedence!
	 */
	txq = &bsscfg->wlcif->qi->q;
	while ((pkt = pktq_deq_tail(bsscfg->psq, &prec))) {
		if (!wlc_prec_enq_head(wlc, txq, pkt, prec, TRUE)) {
			WL_P2P(("wl%d: wlc_bsscfg_tx_start: txq full, frame discarded\n",
			          wlc->pub->unit));
			PKTFREE(wlc->osh, pkt, TRUE);
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
		}
	}

	if (!pktq_empty(txq))
		wlc_send_q(wlc, bsscfg->wlcif->qi);
}

bool
wlc_bsscfg_txq_enq(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, void *sdu, uint prec)
{
	/* Caller should free the packet if it cannot be accomodated */
	if (!wlc_prec_enq(wlc, bsscfg->psq, sdu, prec)) {
		WL_P2P(("wl%d: %s: txq full, frame discarded\n",
		        wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return TRUE;
	}

	return FALSE;
}
#endif /* WL_BSSCFG_TX_SUPR */

#ifdef WLMCHAN
/* This function is used for ap bsscfg setup and assumes STA is associated first.
 * The ap bsscfg will take over the tsf registers but takes into account primary STA's
 * tbtt and bcn_period info.
 * The AP tsf will have its 0 time located in the middle of the primary STA's current beacon
 * period.
 * Per BSS pretbtt block will be setup for both AP and primary STA, based on the new tsf.
 */
void wlc_bsscfg_ap_tbtt_setup(wlc_info_t *wlc, wlc_bsscfg_t *ap_cfg)
{
	d11regs_t *regs = wlc->regs;
	osl_t *osh;
	uint32 tbtt_l, tbtt_h, sta_bcn_period, ap_bcn_period, tsf_l, tsf_h;
	wlc_bsscfg_t *sta_cfg = NULL;
	wlc_bsscfg_t *cfg;
	chanspec_t ap_chanspec = ap_cfg->current_bss->chanspec;
	uint32 bcn_factor = 1;
	int i;
	int bss;

	/* get rid of warning in macosx */
	osh = wlc->osh;

	/* make sure we're not STA */
	if (BSSCFG_STA(ap_cfg)) {
		WL_ERROR(("wl%d: %s cannot setup tbtt for STA bsscfg!\n",
		  wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* find a sta that we can use as reference point */
	FOREACH_AS_STA(wlc, i, cfg) {
		sta_cfg = cfg;
		if (!WLC_MCHAN_SAME_CHANSPEC(cfg->current_bss->chanspec, ap_chanspec)) {
			break;
		}
	}

	ASSERT(sta_cfg != NULL);
	if (sta_cfg == NULL) {
		return;
	}

	if (!WLC_MCHAN_SAME_CHANSPEC(sta_cfg->current_bss->chanspec, ap_chanspec)) {
		WL_MCHAN(("wl%d %s: ap bsscfg adopting assoc sta's bcn period %d\n",
		          wlc->pub->unit, __FUNCTION__, sta_cfg->current_bss->beacon_period));
		ap_cfg->current_bss->beacon_period = sta_cfg->current_bss->beacon_period;
	}

	/* want last STA tbtt - 1/2 STA bcn period to be time (n*ap_bcn_period) for new tsf
	 * The idea is for n to be large enough so n * ap_bcn_period >= 1/2 sta bcn peirod.
	 * This ensures that our new tsf is always positive.
	 */
	wlc_read_tsf(wlc, &tsf_l, &tsf_h);
	/* get sta's next tbtt */
	bss = wlc_p2p_d11cb_idx(wlc->p2p, sta_cfg);
	tbtt_l = wlc_p2p_read_shm(wlc->p2p, M_P2P_BSS_PRE_TBTT(bss)) << 5;

	/* convert pretbtt from 21 bits to 64 bits */
	wlc_p2p_shm_tbtt_to_tbtt64(tsf_h, tsf_l, &tbtt_h, &tbtt_l);

	/* convert pretbtt to tbtt */
	wlc_uint64_add(&tbtt_h, &tbtt_l, 0, (uint32)wlc_pretbtt_calc(sta_cfg));

	/* get the sta's bcn period */
	sta_bcn_period = sta_cfg->current_bss->beacon_period << 10;

	/* get the ap's bcn period */
	ap_bcn_period = (ap_cfg->current_bss->beacon_period << 10);

	/* calculate the tsf offset between new and old tsf times */
	WL_MCHAN(("%s: old sta tbtt = 0x %08x %08x, sta_bcn_period = %d\n",
	          __FUNCTION__, tbtt_h, tbtt_l, sta_bcn_period));
	wlc_uint64_sub(&tbtt_h, &tbtt_l, 0, (sta_bcn_period >> 1));
	while ((bcn_factor * ap_bcn_period) < (sta_bcn_period >> 1)) {
		bcn_factor++;
	}
	wlc_uint64_sub(&tbtt_h, &tbtt_l, 0, (bcn_factor * ap_bcn_period));
	/* tbtt_h and tbtt_l now contain the offset between new and old tsf times */

	/* new tsf = old tsf - offset (tsf - tbtt) */
	WL_MCHAN(("%s: old tsf = 0x %08x %08x, offset = 0x %08x %08x, bcn_factor = %d\n",
	          __FUNCTION__, tsf_h, tsf_l, tbtt_h, tbtt_l, bcn_factor));
	wlc_uint64_sub(&tsf_h, &tsf_l, tbtt_h, tbtt_l);
	/* tsf_h and tsf_l now contain the new tsf value */

	/* setup the AP tbtt */
	/* reset tsf timer */
	W_REG(osh, &regs->tsf_timerlow, tsf_l);
	W_REG(osh, &regs->tsf_timerhigh, tsf_h);
	WL_MCHAN(("%s: curr tsf = 0x %08x %08x\n", __FUNCTION__, tsf_h, tsf_l));

#ifdef WLP2P
	/* update P2P links' tbtt */
	if (P2P_ENAB(wlc->pub)) {
		tsf_l = tsf_h = 0;
		/* tbtt_l and tbtt_h still contains offset, negate it */
		wlc_uint64_sub(&tsf_h, &tsf_l, tbtt_h, tbtt_l);
		wlc_p2p_tbtt_adj(wlc->p2p, tsf_h, tsf_l);
	}
#endif

	/* write the beacon interval (to TSF) */
	W_REG(osh, &regs->tsf_cfprep, (ap_bcn_period << CFPREP_CBI_SHIFT));

	/* CFP start is the next beacon interval after timestamp */
	W_REG(osh, &regs->tsf_cfpstart, ((bcn_factor+1) * ap_bcn_period));

	/* setup per bss block for ap */
	/* This will be done when wlc_p2p_bss_upd is called */
}
#endif /* WLMCHAN */

#ifdef AP
void
wlc_bsscfg_bcn_disable(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_bcn_disable %p #of stas %d\n",
	          wlc->pub->unit, cfg, wlc_bss_assocscb_getcnt(wlc, cfg)));

	cfg->flags &= ~WLC_BSSCFG_HW_BCN;
	if (cfg->up) {
		wlc_suspend_mac_and_wait(wlc);
		wlc_bmac_write_ihr(wlc->hw, 0x47, 3);
		wlc_enable_mac(wlc);
	}
}

void
wlc_bsscfg_bcn_enable(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	WL_APSTA_UPDN(("wl%d: wlc_bsscfg_bcn_enable %p #of stas %d\n",
	          wlc->pub->unit, cfg, wlc_bss_assocscb_getcnt(wlc, cfg)));

	cfg->flags |= WLC_BSSCFG_HW_BCN;
	wlc_bss_update_beacon(wlc, cfg);
}
#endif /* AP */

#ifdef STA
int
wlc_bsscfg_wsec_key_buf_init(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg)
{
	if (!bsscfg->wsec_key_buf_info) {
		bsscfg->wsec_key_buf_info = MALLOC(wlc->osh, sizeof(wsec_key_buf_info_t));
		if (!bsscfg->wsec_key_buf_info) {
			WL_ERROR(("wl%d: %s out of memory for bsscfg, "
				"malloced %d bytes\n", wlc->pub->unit, __FUNCTION__,
				MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
	}

	memset(bsscfg->wsec_key_buf_info, 0, sizeof(wsec_key_buf_info_t));

	return 0;
}

int
wlc_bsscfg_wsec_key_buf_free(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg)
{
	if (!bsscfg->wsec_key_buf_info)
		return 0;

	MFREE(wlc->osh, bsscfg->wsec_key_buf_info, sizeof(wsec_key_buf_info_t));
	bsscfg->wsec_key_buf_info = NULL;

	return 0;
}


void
wlc_bsscfg_wsec_session_reset(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wsec_key_buf_info_t *buf_info = cfg->wsec_key_buf_info;

	(void)wlc;

	if (buf_info == NULL)
		return;

	buf_info->eapol_4way_m1_rxed = TRUE;
	buf_info->eapol_4way_m4_txed = FALSE;
	memset(buf_info->key_buffered, 0, sizeof(bool) * BSSCFG_BUF_KEY_NUM);
}
#endif /* STA */
