/*
 * Separate alloc/free module for wlc_xxx.c files. Decouples
 * the code that does alloc/free from other code so data
 * structure changes don't affect ROMMED code as much.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_alloc.c,v 1.50.2.15 2010-10-30 01:28:21 Exp $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#include <proto/wpa.h>
#include <proto/vlan.h>
#include <wlioctl.h>
#if defined(BCMSUP_PSK)
#include <proto/eapol.h>
#endif 
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_alloc.h>


#define BUS_SET_LMAC(v)
#define BUS_SET_LMACPROTO(v)

static wlc_pub_t *wlc_pub_malloc(osl_t *osh, uint unit, uint devid);
static void wlc_pub_mfree(osl_t *osh, wlc_pub_t *pub);
static void wlc_tunables_init(wlc_tunables_t *tunables, uint devid);

static bool wlc_attach_malloc_high(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid);
static bool wlc_attach_malloc_bmac(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid);
static bool wlc_attach_malloc_misc(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid);
static void wlc_detach_mfree_high(wlc_info_t *wlc, osl_t *osh);
static void wlc_detach_mfree_misc(wlc_info_t *wlc, osl_t *osh);

#ifdef BCMPKTPOOL
extern pktpool_t pktpool_shared;
#endif

void *
wlc_calloc(osl_t *osh, uint unit, uint size)
{
	void *item;

	if ((item = MALLOC(osh, size)) == NULL)
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          unit, __FUNCTION__, MALLOCED(osh)));
	else
		bzero((char*)item, size);
	return item;
}

#ifndef NTXD_4319
#define NTXD_4319 0
#define NRPCTXBUFPOST_4319 0
#endif
#ifndef DNGL_MEM_RESTRICT_RXDMA     /* used only for BMAC low driver */
#define DNGL_MEM_RESTRICT_RXDMA 0
#endif

void
BCMATTACHFN(wlc_tunables_init)(wlc_tunables_t *tunables, uint devid)
{
	tunables->ntxd = NTXD;
	tunables->nrxd = NRXD;
	tunables->rxbufsz = RXBUFSZ;
	tunables->nrxbufpost = NRXBUFPOST;
	tunables->maxscb = MAXSCB;
	tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU;
	tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3STREAMS;
	tunables->maxpktcb = MAXPKTCB;
	tunables->maxdpt = WLC_MAXDPT;
	tunables->maxucodebss = WLC_MAX_UCODE_BSS;
	tunables->maxucodebss4 = WLC_MAX_UCODE_BSS4;
	tunables->maxbss = MAXBSS;
	tunables->datahiwat = WLC_DATAHIWAT;
	tunables->ampdudatahiwat = WLC_AMPDUDATAHIWAT;
	tunables->rxbnd = RXBND;
	tunables->txsbnd = TXSBND;
#ifdef WLC_HIGH_ONLY
	tunables->rpctxbufpost = NRPCTXBUFPOST;
	if (devid == BCM4319_CHIP_ID) {
		tunables->ntxd = NTXD_4319;
		tunables->rpctxbufpost = NRPCTXBUFPOST_4319;
	}
#endif /* WLC_HIGH_ONLY */
#if defined(WLC_LOW_ONLY)
	tunables->dngl_mem_restrict_rxdma = DNGL_MEM_RESTRICT_RXDMA;
#endif
}

static wlc_pub_t *
BCMATTACHFN(wlc_pub_malloc)(osl_t *osh, uint unit, uint devid)
{
	wlc_pub_t *pub;

	if ((pub = (wlc_pub_t*) wlc_calloc(osh, unit, sizeof(wlc_pub_t))) == NULL) {
		goto fail;
	}

	if ((pub->tunables = (wlc_tunables_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_tunables_t))) == NULL) {
		goto fail;
	}

	/* need to init the tunables now */
	wlc_tunables_init(pub->tunables, devid);

#ifdef WLCNT
	if ((pub->_cnt = (wl_cnt_t*) wlc_calloc(osh, unit, sizeof(wl_cnt_t))) == NULL) {
		goto fail;
	}

	if ((pub->_wme_cnt = (wl_wme_cnt_t*)
	     wlc_calloc(osh, unit, sizeof(wl_wme_cnt_t))) == NULL) {
		goto fail;
	}
#endif /* WLCNT */

	return pub;

fail:
	wlc_pub_mfree(osh, pub);
	return NULL;
}

static void
BCMATTACHFN(wlc_pub_mfree)(osl_t *osh, wlc_pub_t *pub)
{
	if (pub == NULL)
		return;

	if (pub->tunables) {
		MFREE(osh, pub->tunables, sizeof(wlc_tunables_t));
		pub->tunables = NULL;
	}

#ifdef WLCNT
	if (pub->_cnt) {
		MFREE(osh, pub->_cnt, sizeof(wl_cnt_t));
		pub->_cnt = NULL;
	}

	if (pub->_wme_cnt) {
		MFREE(osh, pub->_wme_cnt, sizeof(wl_wme_cnt_t));
		pub->_wme_cnt = NULL;
	}
#endif /* WLCNT */

	MFREE(osh, pub, sizeof(wlc_pub_t));
}

#ifdef WLCHANIM
static void
BCMATTACHFN(wlc_chanim_mfree)(osl_t *osh, chanim_info_t *c_info)
{
	wlc_chanim_stats_t *headptr = c_info->stats;
	wlc_chanim_stats_t *curptr;

	while (headptr) {
		curptr = headptr;
		headptr = headptr->next;
		MFREE(osh, curptr, sizeof(wlc_chanim_stats_t));
	}
	c_info->stats = NULL;

	MFREE(osh, c_info, sizeof(chanim_info_t));
}
#endif /* WLCHANIM */

wlc_bsscfg_t *
wlc_bsscfg_malloc(osl_t *osh, uint unit)
{
	wlc_bsscfg_t *cfg;

	if ((cfg = (wlc_bsscfg_t*) wlc_calloc(osh, unit, sizeof(wlc_bsscfg_t))) == NULL)
		goto fail;


#ifdef MBSS
	if ((cfg->bcn_template = (wlc_spt_t*) wlc_calloc(osh, unit, sizeof(wlc_spt_t))) == NULL)
		goto fail;
#endif /* MBSS */

#ifdef WLC_HIGH
		if ((cfg->multicast = (struct ether_addr *)
			 wlc_calloc(osh, unit, (sizeof(struct ether_addr)*MAXMULTILIST))) == NULL) {
			goto fail;
		}
#endif /* WLC_HIGH */

	if ((cfg->assoc = (wlc_assoc_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_assoc_t))) == NULL)
		goto fail;
	if ((cfg->roam = (wlc_roam_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_roam_t))) == NULL)
		goto fail;
	if ((cfg->link = (wlc_link_qual_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_link_qual_t))) == NULL)
		goto fail;
#ifdef WLC_HIGH
	if ((cfg->link->rssi_pkt_window = (int *)
	     wlc_calloc(osh, unit, sizeof(int) * MA_WINDOW_SZ)) == NULL)
		goto fail;
	cfg->link->rssi_pkt_win_sz = MA_WINDOW_SZ;
#endif
	if ((cfg->link->rssi_event = (wl_rssi_event_t *)
	     wlc_calloc(osh, unit, sizeof(wl_rssi_event_t))) == NULL)
		goto fail;
	if ((cfg->current_bss = (wlc_bss_info_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_bss_info_t))) == NULL)
		goto fail;
	if ((cfg->target_bss = (wlc_bss_info_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_bss_info_t))) == NULL)
		goto fail;
#if defined(MBSS) && defined(WLCNT)
	if ((cfg->cnt = (wlc_mbss_cnt_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_mbss_cnt_t))) == NULL)
		goto fail;
#endif
#ifdef WL_BSSCFG_TX_SUPR
	if ((cfg->psq = (struct pktq *)
	     wlc_calloc(osh, unit, sizeof(struct pktq))) == NULL)
		goto fail;
#endif
	if ((cfg->pm = (wlc_pm_st_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_pm_st_t))) == NULL)
		goto fail;

	/* 11g/11n protections */
	if ((cfg->prot_cond = (wlc_prot_cond_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_prot_cond_t))) == NULL)
		goto fail;
	if ((cfg->prot_cfg = (wlc_prot_cfg_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_prot_cfg_t))) == NULL)
		goto fail;
	if ((cfg->prot_to = (wlc_prot_to_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_prot_to_t))) == NULL)
		goto fail;

	/* BRCM IE */
	if ((cfg->brcm_ie = (uint8 *)
	     wlc_calloc(osh, unit, WLC_MAX_BRCM_ELT)) == NULL)
		goto fail;

	/* CSA */
	if ((cfg->csa = (wlc_csa_t *)
	     wlc_calloc(osh, unit, sizeof(wlc_csa_t))) == NULL)
		goto fail;

	return cfg;

fail:
	wlc_bsscfg_mfree(osh, cfg);
	return NULL;
}

void
wlc_bsscfg_mfree(osl_t *osh, wlc_bsscfg_t *cfg)
{
	if (cfg == NULL)
		return;

#ifdef WLC_HIGH
	if (cfg->multicast) {
		MFREE(osh, cfg->multicast, (sizeof(struct ether_addr) * MAXMULTILIST));
		cfg->multicast = NULL;
	}
#endif

#ifdef MBSS
	if (cfg->bcn_template) {
		MFREE(osh, cfg->bcn_template, sizeof(wlc_spt_t));
		cfg->bcn_template = NULL;
	}
#endif /* MBSS */

	if (cfg->maclist) {
		MFREE(osh, cfg->maclist,
		      (int)(OFFSETOF(struct maclist, ea) + cfg->nmac * ETHER_ADDR_LEN));
		cfg->maclist = NULL;
	}

	if (cfg->assoc != NULL) {
		wlc_assoc_t *as = cfg->assoc;
		if (as->ie != NULL)
			MFREE(osh, as->ie, as->ie_len);
		if (as->req != NULL)
			MFREE(osh, as->req, as->req_len);
		if (as->resp != NULL)
			MFREE(osh, as->resp, as->resp_len);
		MFREE(osh, as, sizeof(wlc_assoc_t));
		cfg->assoc = NULL;
	}
	if (cfg->roam != NULL) {
		MFREE(osh, cfg->roam, sizeof(wlc_roam_t));
		cfg->roam = NULL;
	}
	if (cfg->link != NULL) {
		wlc_link_qual_t *link = cfg->link;
#ifdef WLC_HIGH
		if (link->rssi_pkt_window != NULL)
			MFREE(osh, link->rssi_pkt_window, sizeof(int) * MA_WINDOW_SZ);
#endif
		if (link->rssi_event != NULL)
			MFREE(osh, link->rssi_event, sizeof(wl_rssi_event_t));
		MFREE(osh, link, sizeof(wlc_link_qual_t));
		cfg->link = NULL;
	}
	if (cfg->current_bss != NULL) {
		wlc_bss_info_t *current_bss = cfg->current_bss;
		if (current_bss->bcn_prb != NULL)
			MFREE(osh, current_bss->bcn_prb, current_bss->bcn_prb_len);
		MFREE(osh, current_bss, sizeof(wlc_bss_info_t));
		cfg->current_bss = NULL;
	}
	if (cfg->target_bss != NULL) {
		MFREE(osh, cfg->target_bss, sizeof(wlc_bss_info_t));
		cfg->target_bss = NULL;
	}

#if defined(MBSS) && defined(WLCNT)
	if (cfg->cnt) {
		MFREE(osh, cfg->cnt, sizeof(wlc_mbss_cnt_t));
		cfg->cnt = NULL;
	}
#endif

#ifdef WL_BSSCFG_TX_SUPR
	if (cfg->psq) {
		MFREE(osh, cfg->psq, sizeof(struct pktq));
		cfg->psq = NULL;
	}
#endif

	if (cfg->pm != NULL) {
		MFREE(osh, cfg->pm, sizeof(wlc_pm_st_t));
		cfg->pm = NULL;
	}

	/* 11g/11n protections */
	if (cfg->prot_cond != NULL) {
		MFREE(osh, cfg->prot_cond, sizeof(wlc_prot_cond_t));
		cfg->prot_cond = NULL;
	}
	if (cfg->prot_cfg != NULL) {
		MFREE(osh, cfg->prot_cfg, sizeof(wlc_prot_cfg_t));
		cfg->prot_cfg = NULL;
	}
	if (cfg->prot_to != NULL) {
		MFREE(osh, cfg->prot_to, sizeof(wlc_prot_to_t));
		cfg->prot_to = NULL;
	}

	/* BRCM IE */
	if (cfg->brcm_ie != NULL) {
		MFREE(osh, cfg->brcm_ie, WLC_MAX_BRCM_ELT);
		cfg->brcm_ie = NULL;
	}

	/* CSA */
	if (cfg->csa != NULL) {
		MFREE(osh, cfg->csa, sizeof(wlc_csa_t));
		cfg->csa = NULL;
	}

	MFREE(osh, cfg, sizeof(wlc_bsscfg_t));
}

static bool
BCMATTACHFN(wlc_attach_malloc_bmac)(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid)
{
#ifdef WLC_LOW

	if ((wlc->hw->btc = (wlc_hw_btc_info_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_hw_btc_info_t))) == NULL) {
		*err = 1005;
		return FALSE;
	}

	if ((wlc->hw->bandstate[0] = (wlc_hwband_t*)
	     wlc_calloc(osh, unit, (sizeof(wlc_hwband_t) * MAXBANDS))) == NULL) {
		*err = 1006;
		return FALSE;
	}
	else {
		int i;

		for (i = 1; i < MAXBANDS; i++) {
			wlc->hw->bandstate[i] = (wlc_hwband_t *)
				((uintptr)wlc->hw->bandstate[0] + (sizeof(wlc_hwband_t) * i));
		}
	}
#endif /* WLC_LOW */

	return TRUE;
}

static bool
BCMATTACHFN(wlc_attach_malloc_high)(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid)
{
#ifdef WLC_HIGH
	if ((wlc->modulecb = (modulecb_t*)
	     wlc_calloc(osh, unit, sizeof(modulecb_t) * WLC_MAXMODULES)) == NULL) {
		*err = 1012;
		goto fail;
	}

	if ((wlc->scan_results = (wlc_bss_list_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_bss_list_t))) == NULL) {
		*err = 1007;
		goto fail;
	}

	if ((wlc->custom_scan_results = (wlc_bss_list_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_bss_list_t))) == NULL) {
		*err = 1008;
		goto fail;
	}

	if ((wlc->default_bss = (wlc_bss_info_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_bss_info_t))) == NULL) {
		*err = 1010;
		goto fail;
	}

	if ((wlc->cfg = wlc_bsscfg_malloc(osh, unit)) == NULL) {
		*err = 1011;
		goto fail;
	}
	wlc_bsscfg_ID_assign(wlc, wlc->cfg);

	if ((wlc->obss = (obss_info_t*)
	     wlc_calloc(osh, unit, sizeof(obss_info_t))) == NULL) {
		*err = 1012;
		goto fail;
	}

	if ((wlc->pkt_callback = (pkt_cb_t*)
	     wlc_calloc(osh, unit,
	                (sizeof(pkt_cb_t) * (wlc->pub->tunables->maxpktcb + 1)))) == NULL) {
		*err = 1013;
		goto fail;
	}

	if ((wlc->txmod_fns = (txmod_info_t*)
	     wlc_calloc(osh, unit, sizeof(txmod_info_t) * TXMOD_LAST)) == NULL) {
		*err = 1014;
		goto fail;
	}

	if ((wlc->wsec_def_keys[0] = (wsec_key_t*)
	     wlc_calloc(osh, unit, (sizeof(wsec_key_t) * WLC_DEFAULT_KEYS))) == NULL) {
		*err = 1015;
		goto fail;
	}
	else {
		int i;
		for (i = 1; i < WLC_DEFAULT_KEYS; i++) {
			wlc->wsec_def_keys[i] = (wsec_key_t *)
				((uintptr)wlc->wsec_def_keys[0] + (sizeof(wsec_key_t) * i));
		}
	}

	if ((wlc->protection = (wlc_protection_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_protection_t))) == NULL) {
		*err = 1016;
		goto fail;
	}

	if ((wlc->stf = (wlc_stf_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_stf_t))) == NULL) {
		*err = 1017;
		goto fail;
	}

	if ((wlc->btch = (wlc_btc_config_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_btc_config_t))) == NULL) {
		*err = 1017;
		goto fail;
	}

#ifdef STA
	if ((wlc->join_targets = (wlc_bss_list_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_bss_list_t))) == NULL) {
		*err = 1018;
		goto fail;
	}

	if ((wlc->quiet = (wlc_quiet_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_quiet_t))) == NULL) {
		*err = 1021;
		goto fail;
	}
#endif /* STA */

	if ((wlc->corestate->macstat_snapshot = (macstat_t*)
	     wlc_calloc(osh, unit, sizeof(macstat_t))) == NULL) {
		*err = 1027;
		goto fail;
	}

#if defined(DELTASTATS)
	if ((wlc->delta_stats = (delta_stats_info_t*)
	     wlc_calloc(osh, unit, sizeof(delta_stats_info_t))) == NULL) {
		*err = 1023;
		goto fail;
	}
#endif /* DELTASTATS */

#ifdef CCA_STATS
	if ((wlc->cca_info = (cca_info_t*)
	     wlc_calloc(osh, unit, sizeof(cca_info_t))) == NULL) {
		*err = 1028;
		goto fail;
	}

#ifdef ISID_STATS
	if ((wlc->itfr_info = (itfr_info_t*)
	     wlc_calloc(osh, unit, sizeof(itfr_info_t))) == NULL) {
		*err = 1030;
		goto fail;
	}
#endif /* ISID_STATS */
#endif /* CCA_STATS */

#ifdef WLCHANIM
	if ((wlc->chanim_info = (chanim_info_t*)
	     wlc_calloc(osh, unit, sizeof(chanim_info_t))) == NULL) {
		*err = 1029;
		goto fail;
	}
#endif

#endif /* WLC_HIGH */

	return TRUE;

#ifdef WLC_HIGH
fail:
	return FALSE;
#endif
}

static bool
BCMATTACHFN(wlc_attach_malloc_misc)(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid)
{
	return TRUE;
}

/*
 * The common driver entry routine. Error codes should be unique
 */
wlc_info_t *
BCMATTACHFN(wlc_attach_malloc)(osl_t *osh, uint unit, uint *err, uint devid)
{
	wlc_info_t *wlc;

	if ((wlc = (wlc_info_t*) wlc_calloc(osh, unit, sizeof(wlc_info_t))) == NULL) {
		*err = 1002;
		goto fail;
	}
	wlc->hwrxoff = WL_HWRXOFF;

	/* allocate wlc_pub_t state structure */
	if ((wlc->pub = wlc_pub_malloc(osh, unit, devid)) == NULL) {
		*err = 1003;
		goto fail;
	}
	wlc->pub->wlc = wlc;
#ifdef WLLMAC
#  ifdef WLLMAC_ENABLED
	wlc->pub->_lmac = TRUE;
	BUS_SET_LMAC(TRUE);
#  endif
#  ifdef WLLMACPROTO
#    ifdef WLLMACPROTO_ENABLED
	wlc->pub->_lmacproto = TRUE;
	BUS_SET_LMACPROTO(TRUE);
#    endif
#  endif /* WLLMACPROTO */
#endif /* WLLMAC */



#ifdef BCMPKTPOOL
	wlc->pub->pktpool = &pktpool_shared;
#endif

	/* allocate wlc_hw_info_t state structure */

	if ((wlc->hw = (wlc_hw_info_t*)
	     wlc_calloc(osh, unit, sizeof(wlc_hw_info_t))) == NULL) {
		*err = 1004;
		goto fail;
	}
	wlc->hw->wlc = wlc;

	if (!wlc_attach_malloc_bmac(wlc, osh, unit, err, devid))
		goto fail;

	if ((wlc->bandstate[0] = (wlcband_t*)
	     wlc_calloc(osh, unit, (sizeof(wlcband_t) * MAXBANDS))) == NULL) {
		*err = 1010;
		goto fail;
	}
	else {
		int i;

		for (i = 1; i < MAXBANDS; i++) {
			wlc->bandstate[i] =
				(wlcband_t *)((uintptr)wlc->bandstate[0] + (sizeof(wlcband_t) * i));
		}
	}

	if ((wlc->corestate = (wlccore_t*)
	     wlc_calloc(osh, unit, sizeof(wlccore_t))) == NULL) {
		*err = 1011;
		goto fail;
	}

	if (!wlc_attach_malloc_high(wlc, osh, unit, err, devid))
		goto fail;

	if (!wlc_attach_malloc_misc(wlc, osh, unit, err, devid))
		goto fail;

	return wlc;

fail:
	wlc_detach_mfree(wlc, osh);
	return NULL;
}

static void
BCMATTACHFN(wlc_detach_mfree_high)(wlc_info_t *wlc, osl_t *osh)
{
#ifdef WLC_HIGH
	if (wlc->scan_results) {
		MFREE(osh, wlc->scan_results, sizeof(wlc_bss_list_t));
		wlc->scan_results = NULL;
	}

	if (wlc->custom_scan_results) {
		MFREE(osh, wlc->custom_scan_results, sizeof(wlc_bss_list_t));
		wlc->custom_scan_results = NULL;
	}

	if (wlc->modulecb) {
		MFREE(osh, wlc->modulecb, sizeof(modulecb_t) * WLC_MAXMODULES);
		wlc->modulecb = NULL;
	}

	if (wlc->default_bss) {
		MFREE(osh, wlc->default_bss, sizeof(wlc_bss_info_t));
		wlc->default_bss = NULL;
	}

	if (wlc->cfg) {
		wlc_bsscfg_mfree(osh, wlc->cfg);
		wlc->cfg = NULL;
	}

	if (wlc->obss) {
		MFREE(osh, wlc->obss, sizeof(obss_info_t));
		wlc->obss = NULL;
	}

	if (wlc->pkt_callback && wlc->pub && wlc->pub->tunables) {
		MFREE(osh,
		      wlc->pkt_callback, sizeof(pkt_cb_t) * (wlc->pub->tunables->maxpktcb + 1));
		wlc->pkt_callback = NULL;
	}

	if (wlc->txmod_fns)
	     MFREE(osh, wlc->txmod_fns, sizeof(txmod_info_t) * TXMOD_LAST);

	if (wlc->wsec_def_keys[0])
	     MFREE(osh, wlc->wsec_def_keys[0], (sizeof(wsec_key_t) * WLC_DEFAULT_KEYS));

	if (wlc->protection) {
		MFREE(osh, wlc->protection, sizeof(wlc_protection_t));
		wlc->protection = NULL;
	}

	if (wlc->stf) {
		MFREE(osh, wlc->stf, sizeof(wlc_stf_t));
		wlc->stf = NULL;
	}

	if (wlc->btch) {
		MFREE(osh, wlc->btch, sizeof(wlc_btc_config_t));
		wlc->btch = NULL;
	}

#ifdef STA
	if (wlc->join_targets) {
		MFREE(osh, wlc->join_targets, sizeof(wlc_bss_list_t));
		wlc->join_targets = NULL;
	}

	if (wlc->quiet) {
		MFREE(osh, wlc->quiet, sizeof(wlc_quiet_t));
		wlc->quiet = NULL;
	}
#endif /* STA */

#if defined(DELTASTATS)
	if (wlc->delta_stats) {
		MFREE(osh, wlc->delta_stats, sizeof(delta_stats_info_t));
		wlc->delta_stats = NULL;
	}
#endif /* DELTASTATS */
#endif /* WLC_HIGH */
}

static void
BCMATTACHFN(wlc_detach_mfree_misc)(wlc_info_t *wlc, osl_t *osh)
{
}

void
BCMATTACHFN(wlc_detach_mfree)(wlc_info_t *wlc, osl_t *osh)
{
	if (wlc == NULL)
		return;

	wlc_detach_mfree_misc(wlc, osh);

	wlc_detach_mfree_high(wlc, osh);

	if (wlc->bandstate[0])
	     MFREE(osh, wlc->bandstate[0], (sizeof(wlcband_t) * MAXBANDS));

	if (wlc->corestate) {
#ifdef WLC_HIGH
		if (wlc->corestate->macstat_snapshot) {
		     MFREE(osh, wlc->corestate->macstat_snapshot, sizeof(macstat_t));
			 wlc->corestate->macstat_snapshot = NULL;
		}
#endif
		MFREE(osh, wlc->corestate, sizeof(wlccore_t));
		wlc->corestate = NULL;
	}

#ifdef CCA_STATS
	if (wlc->cca_info) {
		MFREE(osh, wlc->cca_info, sizeof(cca_info_t));
		wlc->cca_info = NULL;
	}

#ifdef ISID_STATS
	if (wlc->itfr_info) {
		MFREE(osh, wlc->itfr_info, sizeof(itfr_info_t));
		wlc->itfr_info = NULL;
	}
#endif /* ISID_STATS */
#endif /* CCA_STATS */

#ifdef WLCHANIM
	if (wlc->chanim_info) {
		wlc_chanim_mfree(osh, wlc->chanim_info);
		wlc->chanim_info = NULL;
	}
#endif

	if (wlc->pub) {
		/* free pub struct */
		wlc_pub_mfree(osh, wlc->pub);
		wlc->pub = NULL;
	}

	if (wlc->hw) {
#ifdef WLC_LOW
		if (wlc->hw->btc) {
			MFREE(osh, wlc->hw->btc, sizeof(wlc_hw_btc_info_t));
			wlc->hw->btc = NULL;
		}

		if (wlc->hw->bandstate[0]) {
			MFREE(osh, wlc->hw->bandstate[0], (sizeof(wlc_hwband_t) * MAXBANDS));
			wlc->hw->bandstate[0] = NULL;
		}
#endif

		/* free hw struct */
		MFREE(osh, wlc->hw, sizeof(wlc_hw_info_t));
		wlc->hw = NULL;
	}

	/* free the wlc */
	MFREE(osh, wlc, sizeof(wlc_info_t));
	wlc = NULL;
}
