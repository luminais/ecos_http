/*
 * Common interface to the 802.11 AP Power Save state per scb
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_apps.c,v 1.94.2.6 2010-09-18 00:18:30 Exp $
 */

#ifndef AP
#error "AP must be defined to include this module"
#endif  /* AP */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/wpa.h>
#include <wlioctl.h>
#include <epivers.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_ap.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <bcmwpa.h>
#include <wlc_bmac.h>
#ifdef WLC_HIGH_ONLY
#include <wlc_rpctx.h>
#endif

static void wlc_apps_ps_flush(wlc_info_t *wlc, struct scb *scb);
static void wlc_apps_ps_timedout(wlc_info_t *wlc, struct scb *scb);
static void wlc_apps_pvb_update(wlc_info_t *wlc, struct scb *scb);
static bool wlc_apps_ps_send(wlc_info_t *wlc, struct scb *scb, uint prec_bmp, uint32 flags);
static bool wlc_apps_ps_enq_resp(wlc_info_t *wlc, struct scb *scb, void *pkt, int prec);
static void wlc_apps_ps_enq(void *ctx, struct scb *scb, void *pkt, uint prec);
static int wlc_apps_apsd_delv_count(wlc_info_t *wlc, struct scb *scb);
static int wlc_apps_apsd_ndelv_count(wlc_info_t *wlc, struct scb *scb);
static void wlc_apps_apsd_send(wlc_info_t *wlc, struct scb *scb);
static void wlc_apps_txq_to_psq(wlc_info_t *wlc, struct scb *scb);
static int wlc_apps_down(void *hdl);

#if defined(MBSS)
static void wlc_apps_bss_ps_off_start(wlc_info_t *wlc, struct scb *bcmc_scb);
#else
#define wlc_apps_bss_ps_off_start(wlc, bcmc_scb)
#endif /* MBSS */

/* Flags for psp_flags */
#define PS_MORE_DATA		0x01	/* last PS resp pkt indicated more data */
#define PS_PSP_REQ_PEND		0x02	/* a pspoll req is pending */
/* an arbitary number for unbonded USP */
#define WLC_APSD_USP_UNB 0xfff

struct apps_scb_psinfo {
	struct pktq	psq;		/* PS-defer queue */
	bool		psp_pending;	/* whether uncompleted PS-POLL response is pending */
	uint8		psp_flags;	/* various ps mode bit flags defined below */
	bool		first_suppr_handled;	/* have we handled first supr'd frames ? */
	bool		in_pvb;		/* This STA already set in partial virtual bitmap */
	bool		apsd_usp;	/* APSD Unscheduled Service Period in progress */
	int		apsd_cnt;	/* Frames left in this service period */
	bool		ext_qos_null;	/* Send extra QoS NULL frame to indicate EOSP
	                                 * if last frame sent in SP was MMPDU 
					*/
};

typedef struct apps_bss_info
{
	char		pvb[251];		/* full partial virtual bitmap */
	uint16		aid_lo;			/* lowest aid with traffic pending */
	uint16		aid_hi;			/* highest aid with traffic pending */
	uint		ps_deferred;		/* cnt of all PS pkts buffered on unicast scbs */
	uint32		ps_nodes;		/* num of STAs in PS-mode */
#if defined(WLCNT)
	uint32		bcmc_pkts_seen;		/* Packets thru BC/MC SCB queue */
	uint32		bcmc_discards;		/* Packets discarded due to full queue */
#endif /* BCMDBG */
} apps_bss_info_t;

struct apps_wlc_psinfo
{
	apps_bss_info_t	*bss_info;	/* per-bsscfg apps data */
	uint		ps_deferred;		/* cnt of all PS pkts buffered on unicast scbs */
	uint32		ps_discard;		/* cnt of all PS pkts which were dropped */
	uint32		ps_aged;		/* cnt of all aged PS pkts */

	uint		psq_pkts_lo;		/* # ps pkts are always enq'able on scb */
	uint		psq_pkts_hi;		/* max # of ps pkts enq'able on a single scb */
	int		scb_handle;		/* scb cubby handle to retrieve data from scb */
	uint32		ps_nodes_all;		/* Count of nodes in PS across all BBSes */
	struct pktq	scratch;		/* temp queue to reorder the scb->psq */
};

/* AC bitmap to precedence bitmap mapping (constructed in wlc_attach) */
uint wlc_acbitmap2precbitmap[16] = { 0 };

/* Map AC bitmap to precedence bitmap */
#define WLC_ACBITMAP_TO_PRECBITMAP(ab)  wlc_acbitmap2precbitmap[(ab) & 0xf]


#define SCB_PSINFO(psinfo, scb) ((struct apps_scb_psinfo *)SCB_CUBBY(scb, (psinfo)->scb_handle))
#define BSS_INFO(wlc, idx) ((wlc)->psinfo->bss_info[idx])
#define BSS_PS_NODES(wlc, bsscfg) ((BSS_INFO(wlc, WLC_BSSCFG_IDX(bsscfg))).ps_nodes)

static int wlc_apps_scb_psinfo_init(void *context, struct scb *scb);
static void wlc_apps_scb_psinfo_deinit(void *context, struct scb *scb);
static uint wlc_apps_txpktcnt(void *context);

static txmod_fns_t apps_txmod_fns = {
	wlc_apps_ps_enq,
	wlc_apps_txpktcnt,
	NULL,
	NULL
};

#if defined(BCMDBG)
static void wlc_apps_scb_psinfo_dump(void *context, struct scb *scb, struct bcmstrbuf *b);

/* Limited dump routine for APPS SCB info */
static void
wlc_apps_scb_psinfo_dump(void *context, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_bss_info *bss_info;
	struct pktq *pktq;
	wlc_bsscfg_t *bsscfg;

	if (scb == NULL) return;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	if (scb_psinfo == NULL) {
		bcm_bprintf(b, "     APPS psinfo on SCB %p is NULL\n", scb);
		return;
	}

	bcm_bprintf(b, "     APPS psinfo on SCB %p is %p; scb-PS is %s\n", scb,
		scb_psinfo, SCB_PS(scb) ? "on" : "off");

	pktq = &scb_psinfo->psq;
	if (pktq == NULL) {
		bcm_bprintf(b, "       Packet queue is NULL\n");
		return;
	}

	bsscfg = scb->bsscfg;
	if (bsscfg == NULL)
		return;

	if (SCB_ISMULTI(scb)) {
		bcm_bprintf(b, "       SCB is multi. node count %d\n", BSS_PS_NODES(wlc, bsscfg));
	}
	bss_info = &BSS_INFO(wlc, WLC_BSSCFG_IDX(bsscfg));
	bcm_bprintf(b, "       Pkt Q %p. Que len %d. Max %d. Avail %d. Seen %d. Disc %d\n",
	            pktq, pktq_len(pktq), pktq_max(pktq), pktq_avail(pktq),
	            WLCNTVAL(bss_info->bcmc_pkts_seen), WLCNTVAL(bss_info->bcmc_discards));
}
#else
/* Use NULL to pass as reference on init */
#define wlc_apps_scb_psinfo_dump NULL
#endif /* BCMDBG */

int
BCMATTACHFN(wlc_apps_attach)(wlc_info_t *wlc)
{
	struct apps_wlc_psinfo *wlc_psinfo;
	int i, len;

	len = sizeof(struct apps_wlc_psinfo) + (WLC_MAXBSSCFG * sizeof(apps_bss_info_t));
	if (!(wlc_psinfo = MALLOC(wlc->osh, len))) {
		WL_ERROR(("wl%d: wlc_apps_attach: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, MALLOCED(wlc->osh)));
		wlc->psinfo = NULL;
		return -1;
	}

	bzero((char *)wlc_psinfo, len);

	wlc_psinfo->bss_info = (apps_bss_info_t *)&wlc_psinfo[1];

	/* Set the psq watermarks */
	wlc_psinfo->psq_pkts_lo = PSQ_PKTS_LO;
	wlc_psinfo->psq_pkts_hi = PSQ_PKTS_HI;

	/* calculate the total ps pkts required */
	wlc->pub->psq_pkts_total = wlc_psinfo->psq_pkts_hi +
	    (wlc->pub->tunables->maxscb * wlc_psinfo->psq_pkts_lo);

	/* reserve cubby in the scb container for per-scb private data */
	wlc_psinfo->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct apps_scb_psinfo),
		wlc_apps_scb_psinfo_init, wlc_apps_scb_psinfo_deinit,
		wlc_apps_scb_psinfo_dump, (void *)wlc);

	if (wlc_psinfo->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_apps_attach: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit));
		wlc_apps_detach(wlc);
		return -2;
	}

	/* construct mapping from AC bitmap to precedence bitmap */
	for (i = 0; i < 16; i++) {
		wlc_acbitmap2precbitmap[i] = 0;
		if (AC_BITMAP_TST(i, AC_BE))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_BE;
		if (AC_BITMAP_TST(i, AC_BK))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_BK;
		if (AC_BITMAP_TST(i, AC_VI))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_VI;
		if (AC_BITMAP_TST(i, AC_VO))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_VO;
	}

	/* register module */
	wlc_module_register(wlc->pub, NULL, "apps", wlc, NULL, NULL, NULL, wlc_apps_down);

	wlc_txmod_fn_register(wlc, TXMOD_APPS, wlc, apps_txmod_fns);

	pktq_init(&wlc_psinfo->scratch, WLC_PREC_COUNT, PKTQ_LEN_DEFAULT);

	wlc->psinfo = wlc_psinfo;

	return 0;
}

void
BCMATTACHFN(wlc_apps_detach)(wlc_info_t *wlc)
{
	struct apps_wlc_psinfo *wlc_psinfo;
	int len;

	ASSERT(wlc);
	wlc_psinfo = wlc->psinfo;

	if (!wlc_psinfo)
		return;

	/* All PS packets shall have been flushed */
	ASSERT(wlc_psinfo->ps_deferred == 0);

	wlc_module_unregister(wlc->pub, "apps", wlc);

	len = sizeof(struct apps_wlc_psinfo) +
	    (WLC_MAXBSSCFG * sizeof(apps_bss_info_t));
	MFREE(wlc->osh, wlc_psinfo, len);
	wlc->psinfo = NULL;
}

static int
wlc_apps_down(void *hdl)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb_iter scbiter;
	struct scb *tscb;

	FOREACHSCB(wlc->scbstate, &scbiter, tscb) {
		scb_psinfo = SCB_PSINFO(wlc->psinfo, tscb);

		if (!pktq_empty(&scb_psinfo->psq))
			wlc_apps_ps_flush(wlc, tscb);
	}

	if (MBSS_ENAB(wlc->pub)) {
		int i;

		for (i = 0; (i < WLC_MAXBSSCFG) && wlc->bsscfg[i]; i++) {
			int j;
			struct scb *(*tbcmc_scb)[MAXBANDS] = &wlc->bsscfg[i]->bcmc_scb;

			for (j = 0; (j < MAXBANDS) && (*tbcmc_scb)[j]; j++)
				wlc_apps_ps_flush(wlc, (*tbcmc_scb)[j]);
		}
	}

	return 0;
}

/* Return the count of all the packets being held by APPS module */
static uint
wlc_apps_txpktcnt(void *context)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct apps_wlc_psinfo *wlc_psinfo = wlc->psinfo;

	return (wlc_psinfo->ps_deferred);
}

static int
wlc_apps_scb_psinfo_init(void *context, struct scb *scb)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	/* PS state init */
	pktq_init(&scb_psinfo->psq, WLC_PREC_COUNT, PKTQ_LEN_DEFAULT);
	return 0;
}

static void
wlc_apps_scb_psinfo_deinit(void *context, struct scb *remove)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, remove);

	if (!scb_psinfo)
		return;

	if (AP_ENAB(wlc->pub)) {
		uint8 ps = (wlc->block_datafifo & DATA_BLOCK_PS) ? 0x20 : (0x80 | 0x20);
		if (SCB_PS(remove) && !SCB_ISMULTI(remove))
			wlc_apps_scb_ps_off(wlc, remove, TRUE);
		else if (!pktq_empty(&scb_psinfo->psq))
			wlc_apps_ps_flush(wlc, remove);
		wlc_bmac_process_ps_switch(wlc->hw, &remove->ea, ps);
	}
}

/* This routine deals with all PS transitions from ON->OFF */
void
wlc_apps_scb_ps_off(wlc_info_t *wlc, struct scb *scb, bool discard)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	struct apps_wlc_psinfo *wlc_psinfo = wlc->psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb *bcmc_scb;
	apps_bss_info_t *bss_info;
	wlc_bsscfg_t *bsscfg;

	/* sanity */
	ASSERT(scb);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	bss_info = &wlc_psinfo->bss_info[WLC_BSSCFG_IDX(bsscfg)];
	ASSERT(bss_info->ps_nodes);

	/* process ON -> OFF PS transition */
	WL_PS(("wl%d: wlc_apps_scb_ps_off, %s aid %d\n", wlc->pub->unit,
	       bcm_ether_ntoa(&scb->ea, eabuf),
	       AID2PVBMAP(scb->aid)));

	/* update PS state info */
	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	bss_info->ps_nodes--;
	wlc_psinfo->ps_nodes_all--;
	scb->PS = FALSE;

	/* Unconfigure the APPS from the txpath */
	wlc_txmod_unconfig(wlc, scb, TXMOD_APPS);

	/* If this is last STA to leave PS mode,
	 * trigger BCMC FIFO drain and
	 * set BCMC traffic to go through regular fifo
	 */
	if (bss_info->ps_nodes == 0 && !(bsscfg->flags & WLC_BSSCFG_NOBCMC)) {
		WL_PS(("wl%d: wlc_apps_scb_ps_off - bcmc off\n", wlc->pub->unit));
		/*  Use the bsscfg pointer of this scb to help us locate the
		 *  correct bcmc_scb so that we can turn off PS
		 */
		bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
		ASSERT(bcmc_scb->bsscfg == bsscfg);

		if (MBSS_ENAB(wlc->pub)) { /* MBSS PS handling is a bit more complicated. */
			wlc_apps_bss_ps_off_start(wlc, bcmc_scb);
		} else {
			bcmc_scb->PS = FALSE;

			/* If packets are pending in TX_BCMC_FIFO,
			 * then ask ucode to transmit them immediately
			 */
			if (TXPKTPENDGET(wlc, TX_BCMC_FIFO) && !wlc->bcmcfifo_drain) {
				wlc->bcmcfifo_drain = TRUE;
				wlc_mhf(wlc, MHF2, MHF2_TXBCMC_NOW, MHF2_TXBCMC_NOW, WLC_BAND_AUTO);
			}
		}
	}

	scb_psinfo->psp_pending = FALSE;
	scb_psinfo->first_suppr_handled = FALSE;
	scb_psinfo->apsd_usp = FALSE;
	scb_psinfo->apsd_cnt = 0;

	/* Note: We do not clear up any pending PS-POLL pkts
	 * which may be enq'd with the IGNOREPMQ bit set. The
	 * relevant STA should stay awake until it rx's these
	 * response pkts
	 */

	/* Move pmq entries to Q1 (ctl) for immediate tx */
	if (discard == FALSE)
		while (wlc_apps_ps_send(wlc, scb, WLC_PREC_BMP_ALL, 0))
			;
	else /* free any pending frames */
		wlc_apps_ps_flush(wlc, scb);
}

/* This deals with all PS transitions from OFF->ON */
void
wlc_apps_scb_ps_on(wlc_info_t *wlc, struct scb *scb)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[18];
#endif
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_wlc_psinfo *wlc_psinfo;
	struct scb *bcmc_scb;
	apps_bss_info_t *bss_info;
	wlc_bsscfg_t *bsscfg;

	ASSERT(scb);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	if (BSSCFG_STA(bsscfg))
		return;

	/* process OFF -> ON PS transition */
	WL_PS(("wlc_apps_scb_ps_on, %s aid %d\n", bcm_ether_ntoa(&scb->ea, eabuf),
		AID2PVBMAP(scb->aid)));

	wlc_psinfo = wlc->psinfo;
	bss_info = &wlc_psinfo->bss_info[WLC_BSSCFG_IDX(bsscfg)];

	/* update PS state info */
	bss_info->ps_nodes++;
	wlc_psinfo->ps_nodes_all++;
	scb->PS = TRUE;
	scb->PS_pend = TRUE;
	scb->tbtt = 0;

	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	scb_psinfo->first_suppr_handled = FALSE;

	/* If this is first STA to enter PS mode, set BCMC traffic
	 * to go through BCMC Fifo. If bcmcfifo_drain is set, then clear
	 * the drain bit.
	 */
	if (bss_info->ps_nodes == 1 && !(bsscfg->flags & WLC_BSSCFG_NOBCMC)) {
#if defined(MBSS)
		if (scb->bsscfg->mc_fifo_pkts) {
			WL_PS(("wl%d.%d: START PS-ON; bcmc %d\n", wlc->pub->unit,
				WLC_BSSCFG_IDX(scb->bsscfg), scb->bsscfg->mc_fifo_pkts));
		}
#endif /* MBSS */

		/*  Use the bsscfg pointer of this scb to help us locate the
		 *  correct bcmc_scb so that we can turn on PS
		 */
		bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
		ASSERT(bcmc_scb->bsscfg == bsscfg);
		bcmc_scb->PS = TRUE;
		if (wlc->bcmcfifo_drain) {
			wlc->bcmcfifo_drain = FALSE;
			wlc_mhf(wlc, MHF2, MHF2_TXBCMC_NOW, 0, WLC_BAND_AUTO);
		}
	}

	/* Add the APPS to the txpath for this SCB */
	wlc_txmod_config(wlc, scb, TXMOD_APPS);

	/* ps enQ any pkts on the txq */
	wlc_apps_txq_to_psq(wlc, scb);

	/* If there is anything in the data fifo then allow it to drain */
	if (TXPKTPENDTOT(wlc))
		wlc->block_datafifo |= DATA_BLOCK_PS;
}

static void
wlc_apps_psq_norm(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_wlc_psinfo *wlc_psinfo = wlc->psinfo;
	struct pktq *psq;
	void *pkt;
	int prec;

	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	psq = &scb_psinfo->psq;

	if (pktq_empty(psq))
		return;

	PKTQ_PREC_ITER(psq, prec) {
		void *head_pkt = pktq_ppeek(psq, prec);

		ASSERT(pktq_empty(&wlc_psinfo->scratch));

		while ((pkt = pktq_ppeek_tail(psq, prec)) != NULL &&
		       WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt)) == prec) {
			struct pktq *pktq = psq;
			pkt = pktq_pdeq_tail(psq, prec);
			if (WLPKTTAG(pkt)->flags2 & WLF2_SUPR) {
				WLPKTTAG(pkt)->flags2 &= ~WLF2_SUPR;
				pktq = &wlc_psinfo->scratch;
			}
			pktq_penq_head(pktq, prec, pkt);
			if (pkt == head_pkt)
				break;
		}

		while ((pkt = pktq_pdeq_tail(&wlc_psinfo->scratch, prec)) != NULL)
			pktq_penq_head(psq, prec, pkt);

		ASSERT(pktq_empty(&wlc_psinfo->scratch));
	}
}

/* Process any pending PS states */
void
wlc_apps_process_pend_ps(wlc_info_t *wlc)
{
	struct scb_iter scbiter;
	struct scb *scb;
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	int txpktpendtot = TXPKTPENDTOT(wlc);


	if ((wlc->block_datafifo & DATA_BLOCK_PS) && !txpktpendtot) {
		WL_PS(("wlc_apps_process_pend_ps unblocking fifo\n"));
		wlc->block_datafifo &= ~DATA_BLOCK_PS;
		/* notify bmac to clear the PMQ */
		wlc_bmac_process_ps_switch(wlc->hw, NULL, TX_FIFO_FLUSHED| MSG_MAC_INVALID);
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_PS(scb) && !ETHER_ISMULTI(&scb->ea)) {
				WL_PS(("wlc_apps_process_pend_ps: Normalizing PSQ for STA %s\n",
				       bcm_ether_ntoa(&scb->ea, eabuf)));
				wlc_apps_psq_norm(wlc, scb);
			}
			if ((scb->PS_pend == FALSE) && SCB_PS(scb) && !ETHER_ISMULTI(&scb->ea)) {
				WL_PS(("wlc_apps_process_pend_ps: Allowing PS Off for STA %s\n",
				       bcm_ether_ntoa(&scb->ea, eabuf)));
				wlc_apps_scb_ps_off(wlc, scb, FALSE);
			}
		}
		if (MBSS_ENAB(wlc->pub)) {
			wlc_apps_bss_ps_on_done(wlc);
		}
		/* send a message to bmac PMQ manager so it can  delete appropriate entries. */
		/* wlc_bmac_txpkt_pend (wlc, FALSE);  */
	}
}


/* Free any pending PS packets for this STA */
static void
wlc_apps_ps_flush(wlc_info_t *wlc, struct scb *scb)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	void *pkt;
	struct ether_addr ea;
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_wlc_psinfo *wlc_psinfo;

	ASSERT(scb);
	ASSERT(wlc);

	/* save ea before call wlc_pkt_flush */
	ea = scb->ea;
	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	WL_PS(("wl%d: wlc_apps_ps_flush: flushing %d packets for %s aid %d\n",
	       wlc->pub->unit, pktq_len(&scb_psinfo->psq),
	       bcm_ether_ntoa(&scb->ea, eabuf), AID2PVBMAP(scb->aid)));

	/* Don't care about dequeue precedence */
	while ((pkt = pktq_deq(&scb_psinfo->psq, NULL))) {
		if (!SCB_ISMULTI(scb))
			wlc_psinfo->ps_deferred--;
		WLPKTTAG(pkt)->flags &= ~WLF_PSMARK; /* clear the timestamp */
		/* reclaim callbacks and free */
		PKTFREE(wlc->osh, pkt, TRUE);

		/* callback may have freed scb */
		if (!ETHER_ISMULTI(&ea) && (wlc_scbfind(wlc, &ea) == NULL)) {
			WL_PS(("wl%d: wlc_apps_ps_flush: exiting, scb for %s was freed",
				wlc->pub->unit, bcm_ether_ntoa(&ea, eabuf)));
			return;
		}
	}

	ASSERT(pktq_empty(&scb_psinfo->psq));

	/* If there is a valid aid (the bcmc scb wont have one) then ensure
	 * the PVB is cleared.
	 */
	if (scb->aid && scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);
}

/* Return TRUE if packet has been enqueued on a ps queue, FALSE otherwise */
bool
wlc_apps_psq(wlc_info_t *wlc, void *pkt, int prec)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	struct apps_wlc_psinfo *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb *scb;

	scb = WLPKTTAGSCBGET(pkt);
	ASSERT(SCB_PS(scb));
	ASSERT(wlc);

	/* Do not enq bcmc pkts on a psq, also
	 * ageing out packets may have disassociated the STA, so return FALSE if so
	 */
	if (!SCB_WDS(scb) && !SCB_ASSOCIATED(scb))
		return FALSE;

	ASSERT(!SCB_ISMULTI(scb));

	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	/* Deferred PS pkt flow control
	 * If this scb currently contains less than the minimum number of PS pkts
	 * per scb then enq it. If the total number of PS enq'd pkts exceeds the
	 * watermark and more than the minimum number of pkts are already enq'd
	 * for this STA then do not enq the pkt.
	 */
	if (pktq_len(&scb_psinfo->psq) > (int)wlc_psinfo->psq_pkts_lo &&
	    wlc_psinfo->ps_deferred > wlc_psinfo->psq_pkts_hi) {
		WL_PS(("wl%d: wlc_apps_psq: can't buffer packet for %s aid %d, %d "
		       "queued for scb, %d for WL\n",
		       wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf),
		       AID2PVBMAP(scb->aid), pktq_len(&scb_psinfo->psq),
		       wlc_psinfo->ps_deferred));
		return FALSE;
	}

	if (!wlc_prec_enq(wlc, &scb_psinfo->psq, pkt, prec))
		return FALSE;

	/* increment total count of PS pkts enqueued in WL driver */
	wlc_psinfo->ps_deferred++;

	/* Check if the PVB entry needs to be set */
	if (!scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);

	return (TRUE);
}

/*
 * Move a PS-buffered packet to the txq and send the txq.
 * Returns TRUE if a packet was available to dequeue and send.
 * extra_flags are added to packet flags (for SDU, only to last MPDU)
 */
static bool
wlc_apps_ps_send(wlc_info_t *wlc, struct scb *scb, uint prec_bmp, uint32 extra_flags)
{
	void *pkt = NULL;
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_wlc_psinfo *wlc_psinfo;
	int prec;
	struct pktq *psq;

	ASSERT(wlc);
	wlc_psinfo = wlc->psinfo;

	ASSERT(scb);
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	psq = &scb_psinfo->psq;

	/* Dequeue the packet with highest precedence out of a given set of precedences */
	if (!(pkt = pktq_mdeq(psq, prec_bmp, &prec)))
		return FALSE;		/* no traffic to send */

	/*
	 * If it's the first MPDU in a series of suppressed MPDUs that make up an SDU,
	 * enqueue all of them together before calling wlc_send_q.
	 */
	if (WLPKTTAG(pkt)->flags & WLF_TXHDR) {
		struct dot11_header *h;
		d11txh_t *txh;
		void *next_pkt;
		uint seq_num, next_seq_num;

		txh = (d11txh_t *)PKTDATA(wlc->osh, pkt);
		h = (struct dot11_header *)((uint8 *)(txh + 1) + D11_PHY_HDR_LEN);
		seq_num = ltoh16(h->seq) >> SEQNUM_SHIFT;

		while ((next_pkt = pktq_ppeek(psq, prec)) != NULL) {
			/* Stop if different SDU */
			if (!(WLPKTTAG(pkt)->flags & WLF_TXHDR))
				break;

			/* Stop if different sequence number */
			txh = (d11txh_t *)PKTDATA(wlc->osh, pkt);
			h = (struct dot11_header *)((uint8 *)(txh + 1) + D11_PHY_HDR_LEN);
			next_seq_num = ltoh16(h->seq) >> SEQNUM_SHIFT;
			if (next_seq_num != seq_num)
				break;

			/* Enqueue the PS-Poll response at higher precedence level */
			wlc_apps_ps_enq_resp(wlc, scb, pkt, WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt)));

			/* Dequeue the peeked packet */
			pkt = pktq_pdeq(psq, prec);
			ASSERT(pkt == next_pkt);
		}
	}

	/* Set additional flags on SDU or on final MPDU */
	WLPKTTAG(pkt)->flags |= extra_flags;

	/* Enqueue the PS-Poll response at higher precedence level */
	if (!wlc_apps_ps_enq_resp(wlc, scb, pkt, WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt))))
		scb_psinfo->apsd_usp = FALSE;

	/* Send to hardware (latency for first APSD-delivered frame is especially important) */
	wlc_send_q(wlc, SCB_WLCIFP(scb)->qi);

	/* Check if the PVB entry needs to be cleared */
	if (scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);

	return TRUE;
}

static bool
wlc_apps_ps_enq_resp(wlc_info_t *wlc, struct scb *scb, void *pkt, int prec)
{
	struct apps_wlc_psinfo *wlc_psinfo;
	wlc_txq_info_t *qi = SCB_WLCIFP(scb)->qi;

	wlc_psinfo = wlc->psinfo;

	/* Decrement the global ps pkt cnt */
	if (!SCB_ISMULTI(scb))
		wlc_psinfo->ps_deferred--;

	/* Ensure the pkt marker (used for ageing) is cleared */
	WLPKTTAG(pkt)->flags &= ~WLF_PSMARK;

	WL_PS(("wl%d: ps_enq_resp %p supr %d apsd %d\n",
	       wlc->pub->unit, pkt,
	       (WLPKTTAG(pkt)->flags & WLF_TXHDR) ? 1 : 0,
	       (WLPKTTAG(pkt)->flags & WLF_APSD) ? 1 : 0));

	/* Enqueue in order of precedence */
	if (!wlc_prec_enq(wlc, &qi->q, pkt, prec)) {
		WL_ERROR(("wl%d: wlc_apps_ps_enq_resp: txq full, frame discarded\n",
		          wlc->pub->unit));
		PKTFREE(wlc->osh, pkt, TRUE);
		return FALSE;
	}

	return TRUE;
}

/* Reclaim as many PS pkts as possible
 *	Reclaim from all STAs with pending traffic.
 */
void
wlc_apps_psq_ageing(wlc_info_t *wlc)
{
	struct apps_wlc_psinfo *wlc_psinfo = wlc->psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb_iter scbiter;
	struct scb *tscb;

	if (wlc_psinfo->ps_nodes_all == 0) {
		return; /* No one in PS */
	}

	FOREACHSCB(wlc->scbstate, &scbiter, tscb) {
		scb_psinfo = SCB_PSINFO(wlc->psinfo, tscb);
		if (!tscb->permanent && SCB_PS(tscb) && (tscb->tbtt >= tscb->listen)) {
			tscb->tbtt = 0;
			/* Initiate an ageing event per listen interval */
			if (!pktq_empty(&scb_psinfo->psq))
				wlc_apps_ps_timedout(wlc, tscb);
		}
	}
}

/* check if we should age pkts or not */
static void
wlc_apps_ps_timedout(wlc_info_t *wlc, struct scb *scb)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif
	void *pkt, *head_pkt;
	struct ether_addr ea;
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_wlc_psinfo *wlc_psinfo = wlc->psinfo;
	struct pktq *psq;
	int prec;

	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	psq = &scb_psinfo->psq;

	if (pktq_empty(psq))
		return;

	PKTQ_PREC_ITER(psq, prec) {
		/* Age out all pkts that have been
		 * through one previous listen interval
		 */
		head_pkt = NULL;
		while (pktq_ppeek(psq, prec) != head_pkt) {
			pkt = pktq_pdeq(psq, prec);

			/* If not marked just move on */
			if ((WLPKTTAG(pkt)->flags & WLF_PSMARK) == 0) {
				WLPKTTAG(pkt)->flags |= WLF_PSMARK;
				if (!head_pkt)
					head_pkt = pkt;
				pktq_penq(psq, prec, pkt);
				continue;
			}

			wlc_psinfo->ps_deferred--;
			wlc_psinfo->ps_aged++;

			WL_PS(("wl%d: wlc_apps_ps_timedout: timing out packet for %s aid %d, %d "
			       "remain\n",
			       wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf),
			       AID2PVBMAP(scb->aid), pktq_len(psq)));

			/* save ea before call wlc_pkt_flush */
			ea = scb->ea;

			/* call callback and free */
			PKTFREE(wlc->osh, pkt, TRUE);

			/* callback may have freed scb */
			if (wlc_scbfind(wlc, &ea) == NULL) {
				WL_PS(("wl%d: wlc_apps_ps_timedout: exiting, scb for %s was freed "
				       "after last packet timeout\n",
				       wlc->pub->unit, bcm_ether_ntoa(&ea, eabuf)));
				return;
			}
		}
	}

	/* Check if the PVB entry needs to be cleared */
	if (scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);
}

/* Try to PS enq a pkt, return false if we could not */
static void
wlc_apps_ps_enq(void *ctx, struct scb *scb, void *pkt, uint prec)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	struct apps_wlc_psinfo *wlc_psinfo = wlc->psinfo;

	ASSERT(!SCB_ISMULTI(scb));
	if (!wlc_apps_psq(wlc, pkt, prec)) {
		WL_PS(("wl%d: wlc_apps_ps_enq: ps pkt discarded\n", wlc->pub->unit));
		wlc_psinfo->ps_discard++;
		PKTFREE(wlc->osh, pkt, TRUE);
	}
}

/* Try to ps enq the pkts on the txq */
static void
wlc_apps_txq_to_psq(wlc_info_t *wlc, struct scb *scb)
{
	void *head_pkt = NULL, *pkt;
	int prec;
	struct pktq *txq = &SCB_WLCIFP(scb)->qi->q;

	ASSERT(AP_ENAB(wlc->pub));

	PKTQ_PREC_ITER(txq, prec) {
		head_pkt = NULL;
		/* PS enq all the pkts we can */
		while (pktq_ppeek(txq, prec) != head_pkt) {
			pkt = pktq_pdeq(txq, prec);
			if (scb != WLPKTTAGSCBGET(pkt)) {
				if (!head_pkt)
					head_pkt = pkt;
				pktq_penq(txq, prec, pkt);
				continue;
			}
			wlc_apps_ps_enq(wlc, scb, pkt, prec);
		}
	}
}

/* Set/clear PVB entry according to current state of power save queues */
static void
wlc_apps_pvb_update(wlc_info_t *wlc, struct scb *scb)
{
	uint16 aid;
	struct apps_wlc_psinfo *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	int ps_count;
	apps_bss_info_t *bss_info;

	ASSERT(wlc);
	wlc_psinfo = wlc->psinfo;

	ASSERT(scb);
	ASSERT(scb->bsscfg);
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	bss_info = &wlc_psinfo->bss_info[WLC_BSSCFG_IDX(scb->bsscfg)];

	aid = AID2PVBMAP(scb->aid);
	ASSERT(aid);

	/*
	 * WMM/APSD 3.6.1.4: if no ACs are delivery-enabled (legacy), or all ACs are
	 * delivery-enabled (special case), the PVB should indicate if any packet is
	 * buffered.  Otherwise, the PVB should indicate if any packets are buffered
	 * for non-delivery-enabled ACs only.
	 */

	ps_count = ((scb->apsd.ac_delv == AC_BITMAP_NONE ||
	             scb->apsd.ac_delv == AC_BITMAP_ALL) ?
	            pktq_len(&scb_psinfo->psq) :
	            wlc_apps_apsd_ndelv_count(wlc, scb));

	if (ps_count > 0) {
		if (scb_psinfo->in_pvb)
			return;

		WL_PS(("wl%d: wlc_apps_pvb_update, setting aid %d\n", wlc->pub->unit, aid));
		/* set the bit in the pvb */
		setbit(bss_info->pvb, aid);

		/* reset the aid range */
		if ((aid < bss_info->aid_lo) || !bss_info->aid_lo)
			bss_info->aid_lo = aid;
		if (aid > bss_info->aid_hi)
			bss_info->aid_hi = aid;

		scb_psinfo->in_pvb = TRUE;
	} else {
		if (!scb_psinfo->in_pvb)
			return;

		WL_PS(("wl%d: wlc_apps_pvb_entry, clearing aid %d\n", wlc->pub->unit, aid));
		/* clear the bit in the pvb */
		clrbit(bss_info->pvb, aid);

		if (bss_info->ps_nodes == 0) {
			bss_info->aid_lo = bss_info->aid_hi = 0;
		} else {
			/* reset the aid range */
			if (aid == bss_info->aid_hi) {
				/* find the next lowest aid value with PS pkts pending */
				for (aid = aid - 1; aid; aid--)
					if (isset(bss_info->pvb, aid)) {
						bss_info->aid_hi = aid;
						break;
					}
				/* no STAs with pending traffic ? */
				if (aid == 0)
					bss_info->aid_hi = bss_info->aid_lo = 0;
			} else if (aid == bss_info->aid_lo) {
				/* find the next highest aid value with PS pkts pending */
				for (aid = aid + 1; aid < wlc->pub->tunables->maxscb; aid++)
					if (isset(bss_info->pvb, aid)) {
						bss_info->aid_lo = aid;
						break;
					}
				ASSERT(aid != wlc->pub->tunables->maxscb);
			}
		}

		scb_psinfo->in_pvb = FALSE;
	}

	/* Update the PVB in the bcn template */
	WL_APSTA_BCN(("wl%d: wlc_apps_pvb_entry -> wlc_bss_update_beacon\n", wlc->pub->unit));
	wlc_bss_update_beacon(wlc, scb->bsscfg);
}

void
wlc_bss_apps_tbtt_update(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	struct scb *scb;
	struct scb_iter scbiter;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_AP(cfg));


	/* increment the tbtt count on all PS scbs */
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!scb->permanent && SCB_PS(scb) &&
		    SCB_BSSCFG(scb) == cfg)
			if (scb->tbtt < 0xFFFF) /* do not wrap around */
				scb->tbtt++;
	}
}

void
wlc_apps_tbtt_update(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;


	FOREACH_UP_AP(wlc, idx, cfg)
	        wlc_bss_apps_tbtt_update(cfg);
}

/* called from bmac when a PS state switch is detected from the transmitter.
 * On PS ON switch, directly call wlc_apps_scb_ps_on(wlc, scb);
 *  On PS OFF, check if there are tx packets pending. If so, set the PS_pend flag
 *  and wait for the drain. Otherwise, switch to PS OFF.
 *  Sends a message to the bmac pmq manager to signal that we detected this switch.
 *  PMQ manager will delete entries when switch states are in sync and the queue is drained.
 *  return 1 if a switch occured. This allows the caller to invalidate
 *  the header cache.
 */
int BCMFASTPATH
wlc_apps_process_ps_switch(wlc_info_t *wlc, struct ether_addr *ea, int8 ps_on)
{

	struct scb *scb;

	/* only process ps transitions for associated sta's */
	scb = wlc_scbfind(wlc, ea);
	if (!scb || (!SCB_WDS(scb) && !SCB_ASSOCIATED(scb))) {
		/* send notification to bmac that this entry doesn't exist 
		   up here.
		 */
		uint8 ps = STA_REMOVED;

		ps |= (wlc->block_datafifo & DATA_BLOCK_PS) ?  0 : TX_FIFO_FLUSHED;
		wlc_bmac_process_ps_switch(wlc->hw, ea, ps);
		return 0;
	}

	if (ps_on) {
		if (!SCB_PS(scb)) {
			wlc_apps_scb_ps_on(wlc, scb);
		}
		else {
			scb->PS_pend = TRUE; /* STA is already in PS, update pend only */
		}
	}
	else {
		if ((wlc->block_datafifo & DATA_BLOCK_PS)) {
			/* Prevent ON -> OFF transitions while data fifo is blocked.
			 * We need to finish flushing HW and reque'ing before we
			 * can allow the STA to come out of PS.
			 */
			scb->PS_pend = FALSE;
		}
		else if (SCB_PS(scb))  {
			wlc_apps_scb_ps_off(wlc, scb, FALSE);
		}
	}

	/* indicate fifo state  */
	if (!(wlc->block_datafifo & DATA_BLOCK_PS))
		ps_on |= TX_FIFO_FLUSHED;
	wlc_bmac_process_ps_switch(wlc->hw, &scb->ea, ps_on);
	return 0;

}

void
wlc_apps_pspoll_resp_prepare(wlc_info_t *wlc, struct scb *scb,
                             void *pkt, struct dot11_header *h, bool last_frag)
{
	struct apps_scb_psinfo *scb_psinfo;

	ASSERT(scb);
	ASSERT(SCB_PS(scb));

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/*
	 * FC_MOREDATA is set for every response packet being sent while STA is in PS.
	 * This forces STA to send just one more PS-Poll.  If by that time we actually
	 * have more data, it'll be sent, else a Null data frame without FC_MOREDATA will
	 * be sent.  This technique often improves TCP/IP performance.  The last NULL Data
	 * frame is sent with the WLF_PSDONTQ flag.
	 */

	h->fc |= htol16(FC_MOREDATA);

	/* Register pkt callback for PS-Poll response */
	if (last_frag && !SCB_ISMULTI(scb)) {
		WLPKTFLAG2_TYPESET(pkt, WLF2_TYPE_PSP_RSP);
		scb_psinfo->psp_pending = TRUE;
	}

	scb_psinfo->psp_flags |= PS_MORE_DATA;
}

/* Fix PDU that is being sent as a PS-Poll response or APSD delivery frame. */
void
wlc_apps_ps_prep_mpdu(wlc_info_t *wlc, void *pkt)
{
	bool last_frag;
	struct dot11_header *h;
	d11txh_t *txh;
	uint16	macCtlLow, frameid;
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;
	wsec_key_t *key;

	scb = WLPKTTAGSCBGET(pkt);

	txh = (d11txh_t *)PKTDATA(wlc->osh, pkt);
	h = (struct dot11_header *)((uint8 *)(txh + 1) + D11_PHY_HDR_LEN);

	WL_PS(("wl%d: wlc_apps_ps_prep_mpdu: pkt %p flg %d fc 0x%x\n",
	       wlc->pub->unit, pkt, WLPKTTAG(pkt)->flags, h->fc));

	/*
	 * Set the IGNOREPMQ bit.
	 *
	 * PS bcast/mcast pkts have following differences from ucast:
	 *    1. use the BCMC fifo
	 *    2. FC_MOREDATA is set by ucode (except for the kludge)
	 *    3. Don't set IGNOREPMQ bit as ucode ignores PMQ when draining
	 *       during DTIM, and looks at PMQ when draining through
	 *       MHF2_TXBCMC_NOW
	 */
	if (ETHER_ISMULTI(txh->TxFrameRA)) {
		frameid = ltoh16(txh->TxFrameID);
		frameid &= ~TXFID_QUEUE_MASK;
		frameid |= TX_BCMC_FIFO;
		txh->TxFrameID = htol16(frameid);

		ASSERT(!SCB_WDS(scb));

		bsscfg = SCB_BSSCFG(scb);

		if (!IS_WPA_AUTH(bsscfg->WPA_auth) ||
		    ((key = WSEC_BSS_DEFAULT_KEY(bsscfg)) == NULL) ||
		    (key->algo != CRYPTO_ALGO_AES_CCM))
			h->fc |= htol16(FC_MOREDATA);
	} else {
		last_frag = (ltoh16(h->fc) & FC_MOREFRAG) == 0;
		/* Set IGNOREPMQ bit (otherwise, it may be suppressed again) */
		macCtlLow = ltoh16(txh->MacTxControlLow);
		macCtlLow |= TXC_IGNOREPMQ;
		txh->MacTxControlLow = htol16(macCtlLow);

		/*
		 * Set FC_MOREDATA and EOSP bit and register callback.  WLF_APSD is set
		 * for all APSD delivery frames.  WLF_PSDONTQ is set only for the final
		 * Null frame of a series of PS-Poll responses.
		 */
		if (WLPKTTAG(pkt)->flags & WLF_APSD)
			wlc_apps_apsd_prepare(wlc, scb, pkt, h, last_frag);
		else if (!(WLPKTTAG(pkt)->flags & WLF_PSDONTQ))
			wlc_apps_pspoll_resp_prepare(wlc, scb, pkt, h, last_frag);
	}
}

void
wlc_apps_psp_resp_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;
	struct apps_scb_psinfo *scb_psinfo;

	/* Is this scb still around */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL)
		return;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	if (scb_psinfo->psp_pending) {
		scb_psinfo->psp_pending = FALSE;
		if (scb_psinfo->psp_flags & PS_PSP_REQ_PEND) {
			/* send the next ps pkt if requested */
			scb_psinfo->psp_flags &= ~(PS_MORE_DATA | PS_PSP_REQ_PEND);
			wlc_apps_ps_send(wlc, scb, WLC_PREC_BMP_ALL, 0);
		}
	}
}

void
wlc_apps_send_psp_response(wlc_info_t *wlc, struct scb *scb, uint16 fc)
{
	struct apps_scb_psinfo *scb_psinfo;
	void *pkt;

	ASSERT(scb);
	ASSERT(wlc);

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/*
	 * Send a null data frame if there are no PS buffered
	 * frames on APSD non-delivery-enabled ACs (WMM/APSD 3.6.1.6).
	 */
	if (pktq_empty(&scb_psinfo->psq) || wlc_apps_apsd_ndelv_count(wlc, scb) == 0) {
		/* Ensure pkt is not queued on psq */
		if ((pkt = wlc_sendnulldata(wlc, &scb->ea, 0, WLF_PSDONTQ,
		                            PRIO_8021D_BE)) == NULL) {
			WL_ERROR(("wl%d: %s: PS-Poll null data response failed\n",
			          wlc->pub->unit, __FUNCTION__));
			scb_psinfo->psp_pending = FALSE;
		} else {
			WLPKTFLAG2_TYPESET(pkt, WLF2_TYPE_PSP_RSP);
			scb_psinfo->psp_pending = TRUE;
		}

		scb_psinfo->psp_flags &= ~PS_MORE_DATA;
	} else {
		/* Check if we should ignore the ps poll */
		if (scb_psinfo->psp_pending && !SCB_ISMULTI(scb)) {
			/* Reply to a non retried PS Poll pkt after the current
			 * psp_pending has completed (if that pending pkt indicated "more
			 * data"). This aids the stalemate introduced if a STA acks a ps
			 * poll response but the AP misses that ack
			 */
			if ((scb_psinfo->psp_flags & PS_MORE_DATA) && !(fc & FC_RETRY))
				scb_psinfo->psp_flags |= PS_PSP_REQ_PEND;
		} else
			wlc_apps_ps_send(wlc, scb, WLC_PREC_BMP_ALL, 0);
	}
}

/* Fill in the TIM element for the specified bsscfg */
void
wlc_apps_tim_create(wlc_info_t *wlc, uchar *tim, int timlen, uint bss_idx)
{
	struct apps_wlc_psinfo *wlc_psinfo;
	apps_bss_info_t *bss_psinfo;
	uint8 n1 = 0, n2 = 0;
	int16 delta;

	ASSERT(wlc);
	wlc_psinfo = wlc->psinfo;
	bss_psinfo = &wlc_psinfo->bss_info[bss_idx];

	if (!bss_psinfo->ps_nodes)
		return;

	n1 = (uint8)bss_psinfo->aid_lo/8;
	/* n1 must be highest even number */
	n1 &= ~1;
	n2 = (uint8)bss_psinfo->aid_hi/8;

	delta = n2 - n1;

	/* sanity */
	ASSERT((delta > -1) && (delta < (wlc->pub->tunables->maxscb + 1)));

	/* perform length check to make sure tim buffer is big enough */
	/* if not, don't write anything and just return (void return, no return value) */
	/* the ie header length is 2 bytes, ie datalen is (delta + 4) */
	BUFLEN_CHECK_AND_RETURN_VOID((delta + 4 + 2), timlen);

	/* set the length of the TIM */
	tim[1] = delta + 4;

	/* set the offset field of the TIM */
	tim[4] = n1;

	/* copy the PVB into the TIM */
	bcopy((char *)&bss_psinfo->pvb[n1], (char *)&tim[5], delta+1);
}

/* Enqueue a suppressed PDU to psq after fixing up the PDU */
bool
wlc_apps_suppr_frame_enq(wlc_info_t *wlc, void *pkt, tx_status_t *txs, bool last_frag)
{
	uint16 frag = (txs->frameid >> TXFID_SEQ_SHIFT) & FRAGNUM_MASK;
	uint16 retries = txs->status & (TX_STATUS_FRM_RTX_MASK |
	                                TX_STATUS_RTS_RTX_MASK);
	uint16 seq_num;
	struct scb *scb = WLPKTTAGSCBGET(pkt);
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_wlc_psinfo *wlc_psinfo = wlc->psinfo;

	d11txh_t *txh = (d11txh_t *)PKTDATA(wlc->osh, pkt);
	struct dot11_header *h;

	ASSERT(scb != NULL);

	h = (struct dot11_header *) (((char *)txh) + D11_TXH_LEN + D11_PHY_HDR_LEN);

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	/* Is this the first suppressed frame, and either is partial
	 * MSDU or has been retried at least once, driver needs to
	 * preserve the retry count and sequence number in the PDU so that
	 * next time it is transmitted, the receiver can put it in order
	 * or discard based on retries. For partial MSDU, reused sequence
	 * number will allow reassembly
	 */
	if (!scb_psinfo->first_suppr_handled && (frag || retries)) {
		/* If the seq num was hw generated then get it from the
		 * status pkt otherwise get it from the original pkt
		 */
		if (ltoh16(txh->MacTxControlLow) & TXC_HWSEQ)
			seq_num = txs->sequence;
		else
			seq_num = ltoh16(h->seq) >> SEQNUM_SHIFT;

		txh->MacTxControlLow &= ~htol16(TXC_HWSEQ);
		h->seq = htol16((seq_num << SEQNUM_SHIFT) | (frag & FRAGNUM_MASK));
		/* set the retry counts */
		txh->TxStatus = htol16(retries);
		WL_PS(("Partial MSDU PDU %p - frag:%d seq_num:%d retries: %d\n", pkt,
		       frag, seq_num, retries));
	}

	/* This ensures that all the MPDUs of the same SDU get
	 * same seq_num. This is a case when first fragment was retried
	 */
	if (last_frag || !(frag || retries))
		scb_psinfo->first_suppr_handled = TRUE;

#ifdef BCMDBG
	{
	char eabuf[ETHER_ADDR_STR_LEN];
	WL_PS(("SUPPRESSED packet %p - %s %s PS:%d \n", pkt,
	       (FC_TYPE(ltoh16(txh->MacFrameControl)) == FC_TYPE_DATA) ? "data" :
	       (FC_TYPE(ltoh16(txh->MacFrameControl)) == FC_TYPE_MNG) ? "mgmt" :
	       (FC_TYPE(ltoh16(txh->MacFrameControl)) == FC_TYPE_CTL) ? "ctrl" :
	       "unknown",
	       bcm_ether_ntoa((struct ether_addr *)txh->TxFrameRA,  eabuf), SCB_PS(scb)));
	}
#endif	/* BCMDBG_ERR */

	/* If in PS mode, enqueue the suppressed PDU to PSQ for ucast SCB otherwise txq */
	if (SCB_PS(scb) && !SCB_ISMULTI(scb)) {
		WLPKTTAG(pkt)->flags2 |= WLF2_SUPR;
		/* If enqueue to psq successfully, return FALSE so that PDU is not freed */
		/* Enqueue at higher precedence as these are suppressed packets */
		if (wlc_apps_psq(wlc, pkt, WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt))))
			return FALSE;
		WL_ERROR(("wl%d: wlc_apps_suppr_frame_enq: ps suppr pkt discarded\n",
		          wlc->pub->unit));
		wlc_psinfo->ps_discard++;
	} else
		if (wlc_prec_enq(wlc, &SCB_WLCIFP(scb)->qi->q, pkt,
		                 WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt))))
			return FALSE;

	return TRUE;
}
static uint8 acbitmap2maxprio[] = {
	PRIO_8021D_BE, PRIO_8021D_BE, PRIO_8021D_BK, PRIO_8021D_BK,
	PRIO_8021D_VI, PRIO_8021D_VI, PRIO_8021D_VI, PRIO_8021D_VI,
	PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC,
	PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC
};

static void
wlc_apps_apsd_send(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint prec_bmp;
	int delv;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);
	ASSERT((scb_psinfo->ext_qos_null) || (scb_psinfo->apsd_cnt > 0));

	delv = wlc_apps_apsd_delv_count(wlc, scb);


	/*
	 * If there are no buffered frames, send a QoS Null on the highest delivery-enabled AC
	 * (which AC to use is not specified by WMM/APSD).
	 */
	if (delv == 0 || scb_psinfo->ext_qos_null)
	{
		int prio = (int)acbitmap2maxprio[scb->apsd.ac_delv];

		WL_ERROR(("wl%d: wlc_apps_apsd_send: send QoS Null prio=%d\n",
		       wlc->pub->unit, prio));

		ASSERT((scb->apsd.ac_delv & ~0xf) == 0);
		scb_psinfo->apsd_cnt = 0;
		if (!wlc_sendnulldata(wlc, &scb->ea, 0, (WLF_PSDONTQ | WLF_APSD), prio)) {
			WL_ERROR(("wl%d: wlc_apps_apsd_trigger: could not send QoS Null EOSP\n",
			          wlc->pub->unit));
			scb_psinfo->apsd_usp = FALSE;
		}
		scb_psinfo->ext_qos_null = FALSE;
		return;
	}

	scb_psinfo->ext_qos_null = FALSE;
	prec_bmp = WLC_ACBITMAP_TO_PRECBITMAP(scb->apsd.ac_delv);

	/* Indicate EOSP when there is only one packet left in the psq */
	if (delv == 1) {
		void *pkt = pktq_mpeek(&scb_psinfo->psq, prec_bmp, NULL);

		if (WLPKTTAG(pkt)->flags & WLF_TXHDR) {
			d11txh_t *txh = (d11txh_t *)PKTDATA(wlc->osh, pkt);
			struct dot11_header *h = (struct dot11_header *)
						((uint8 *)(txh + 1) + D11_PHY_HDR_LEN);
			scb_psinfo->ext_qos_null =
			!((htol16(h->fc) & FC_KIND_MASK) == FC_QOS_DATA ||
			(htol16(h->fc) & FC_KIND_MASK) == FC_QOS_NULL);
		}
		else
			scb_psinfo->ext_qos_null =
			(WLPKTTAG(pkt)->flags & WLF_MPDU)?TRUE:FALSE;

		WL_PS(("wl%d: last frame to deliver: ext_qos_null = %d \n",
			wlc->pub->unit, scb_psinfo->ext_qos_null));
		scb_psinfo->apsd_cnt = 1;
	}
	/* Decrement count of packets left in service period */
	if (scb_psinfo->apsd_cnt != WLC_APSD_USP_UNB)
		scb_psinfo->apsd_cnt--;

	WL_PS(("wlc_apps_apsd_send: cnt=%d\n", scb_psinfo->apsd_cnt));

	/*
	 * Send a delivery frame.  When the frame goes out, the wlc_apsd_complete
	 * callback will attempt to send the next delivery frame.
	 */
	if (!wlc_apps_ps_send(wlc, scb, prec_bmp, WLF_APSD))
		scb_psinfo->apsd_usp = FALSE;
}

/* Send frames in a USP, called in response to receiving a trigger frame */
void
wlc_apps_apsd_trigger(wlc_info_t *wlc, struct scb *scb, int ac)
{
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);
	/* Ignore trigger frames received during an existing USP */
	if (scb_psinfo->apsd_usp) {
		WL_PS(("wl%d: wlc_apps_apsd_trigger: already in USP; ignoring trigger\n",
		       wlc->pub->unit));
		return;
	}

	WL_PS(("wl%d: wlc_apps_apsd_trigger: ac %d buffered %d delv %d\n",
	       wlc->pub->unit, ac, pktq_plen(&scb_psinfo->psq, ac),
		wlc_apps_apsd_delv_count(wlc, scb)));

	scb_psinfo->apsd_usp = TRUE;
	/* Clear this flag at the beginning USP */
	scb_psinfo->ext_qos_null = FALSE;

	if (scb_psinfo->apsd_cnt == 0)
		scb_psinfo->apsd_cnt = scb->apsd.maxsplen;
	/*
	 * Send the first delivery frame.  Subsequent delivery frames will be sent by the
	 * completion callback of each previous frame.  This is not very efficient, but if
	 * we were to queue a bunch of frames to different FIFOs, there would be no
	 * guarantee that the MAC would send the EOSP last.
	 */
	wlc_apps_apsd_send(wlc, scb);
}

void
wlc_apps_apsd_prepare(wlc_info_t *wlc, struct scb *scb, void *pkt,
                      struct dot11_header *h, bool last_frag)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint16 *pqos;

	WL_PS(("wl%d: wlc_apps_apsd_prepare: pkt %p lastfrag %d\n",
		wlc->pub->unit, pkt, last_frag));

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/* Set MoreData if there are still buffered delivery frames */
	if (wlc_apps_apsd_delv_count(wlc, scb) > 0)
		h->fc |= htol16(FC_MOREDATA);
	else if (!(WLPKTTAG(pkt)->flags & WLF_MPDU))
		h->fc &= ~htol16(FC_MOREDATA);

	if (((htol16(h->fc) & FC_KIND_MASK) == FC_QOS_DATA) ||
	    ((htol16(h->fc) & FC_KIND_MASK) == FC_QOS_NULL)) {
		pqos = (uint16 *)((uchar *)h +
			(SCB_WDS(scb) ? DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN));
		ASSERT(ISALIGNED((uintptr)pqos, sizeof(*pqos)));

		/* Set EOSP if this is the last frame in the Service Period */
		if (scb_psinfo->apsd_cnt == 0 && last_frag)
			*pqos |= htol16(QOS_EOSP_MASK);
		else
			*pqos &= ~htol16(QOS_EOSP_MASK);
	}

	/* Register callback to end service period after this frame goes out */
	if (last_frag) {
		WLPKTFLAG2_TYPESET(pkt, WLF2_TYPE_APSD);
	}
}

/* End the USP when the EOSP has gone out */
void
wlc_apps_apsd_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;
	struct apps_scb_psinfo *scb_psinfo;

#ifdef BCMDBG
	if (txs & TX_STATUS_ACK_RCV)
		WL_PS(("wl%d: delivery frame sent\n", wlc->pub->unit));
	else
		WL_PS(("wl%d: delivery frame sent (no ACK)\n", wlc->pub->unit));
#endif

	/* Is this scb still around */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL)
		return;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/* Send more frames until the End Of Service Period */
	if ((scb_psinfo->apsd_cnt) || scb_psinfo->ext_qos_null)
		wlc_apps_apsd_send(wlc, scb);
	else
		scb_psinfo->apsd_usp = FALSE;
}

void
wlc_apps_apsd_delv_stop(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;

	ASSERT(scb != NULL);

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	scb_psinfo->apsd_cnt = 0;
}

/*
 * Return the number of frames pending on delivery-enabled ACs.
 */
static int
wlc_apps_apsd_delv_count(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint32 precbitmap;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	if (scb->apsd.ac_delv == AC_BITMAP_NONE)
		return 0;

	precbitmap = WLC_ACBITMAP_TO_PRECBITMAP(scb->apsd.ac_delv);

	return pktq_mlen(&scb_psinfo->psq, precbitmap);
}

/*
 * Return the number of frames pending on non-delivery-enabled ACs.
 */
static int
wlc_apps_apsd_ndelv_count(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	ac_bitmap_t ac_non_delv;
	uint32 precbitmap;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	if (scb->apsd.ac_delv == AC_BITMAP_ALL)
		return 0;

	ac_non_delv = ~scb->apsd.ac_delv & AC_BITMAP_ALL;
	precbitmap = WLC_ACBITMAP_TO_PRECBITMAP(ac_non_delv);

	return pktq_mlen(&scb_psinfo->psq, precbitmap);
}

#if defined(MBSS)

/* DTIM pre-TBTT interrupt has occurred: Update SHM last_fid registers. */
void
wlc_apps_update_bss_bcmc_fid(wlc_info_t *wlc)
{
	int i = 0;
	uint fid_addr = SHM_MBSS_BC_FID0;
	wlc_bsscfg_t *bsscfg;

	if (!MBSS_ENAB(wlc->pub)) {
		return;
	}

	FOREACH_UP_AP(wlc, i, bsscfg) {
		fid_addr = SHM_MBSS_BC_FID_ADDR(WLC_BSSCFG_UCIDX(bsscfg));

		/* If BC/MC packets have been written, update shared mem */
		if (bsscfg->bcmc_fid != INVALIDFID) {
			wlc_write_shm((wlc), fid_addr, bsscfg->bcmc_fid);
			BCMC_FID_SHM_COMMIT(bsscfg);
		}
	}
}

/* Enqueue a BC/MC packet onto it's BSS's PS queue */
int
wlc_apps_bcmc_ps_enqueue(wlc_info_t *wlc, struct scb *bcmc_scb, void *pkt)
{
	struct apps_scb_psinfo *scb_psinfo;
	apps_bss_info_t *bss_info;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, bcmc_scb);
	bss_info = &BSS_INFO(wlc, WLC_BSSCFG_IDX(bcmc_scb->bsscfg));

	/* Check that packet queue length is not exceeded */
	if (pktq_full(&scb_psinfo->psq) || pktq_pfull(&scb_psinfo->psq, MAXPRIO)) {
		WL_NONE(("wlc_apps_bcmc_ps_enqueue: queue full.\n"));
		WLCNTINCR(bss_info->bcmc_discards);
		return BCME_ERROR;
	}
	(void)pktq_penq(&scb_psinfo->psq, MAXPRIO, pkt);
	WLCNTINCR(bss_info->bcmc_pkts_seen);

	return BCME_OK;
}

/* Last STA has gone out of PS.  Check state of its BSS */

static void
wlc_apps_bss_ps_off_start(wlc_info_t *wlc, struct scb *bcmc_scb)
{
	wlc_bsscfg_t *bsscfg;

	bsscfg = bcmc_scb->bsscfg;
	ASSERT(bsscfg != NULL);

	if (!BCMC_PKTS_QUEUED(bsscfg)) {
		/* No pkts in BCMC fifo */
		wlc_apps_bss_ps_off_done(wlc, bsscfg);
	} else { /* Mark in transition */
		ASSERT(bcmc_scb->PS); /* Should only have BCMC pkts if in PS */
		bsscfg->flags |= WLC_BSSCFG_PS_OFF_TRANS;
		WL_PS(("wl%d.%d: START PS-OFF. last fid 0x%x. shm fid 0x%x\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), bsscfg->bcmc_fid,
			bsscfg->bcmc_fid_shm));
#if defined(BCMDBG_MBSS_PROFILE)     /* Start transition timing */
		if (bsscfg->ps_start_us == 0) {
			bsscfg->ps_start_us = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
		}
#endif /* BCMDBG_MBSS_PROFILE */
	}
}

#if defined(WLC_HIGH) && defined(MBSS)
/*
 * Due to a STA transitioning to PS on, all packets have been drained from the
 * data fifos.  Update PS state of all BSSs (if not in PS-OFF transition).
 *
 * Note that it's possible that a STA has come out of PS mode during the
 * transition, so we may return to PS-OFF (abort the transition).  Since we
 * don't keep state of which STA and which BSS started the transition, we
 * simply check them all.
 */

void
wlc_apps_bss_ps_on_done(wlc_info_t *wlc)
{
	wlc_bsscfg_t *bsscfg;
	struct scb *bcmc_scb;
	int i;

	FOREACH_UP_AP(wlc, i, bsscfg) {
		if (!(bsscfg->flags & WLC_BSSCFG_PS_OFF_TRANS)) { /* Ignore BSS in PS-OFF trans */
			bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
			if (BSS_PS_NODES(wlc,  bsscfg) != 0) {
				if (!SCB_PS(bcmc_scb)) {
#if defined(MBSS)
					/* PS off, MC pkts to data fifo should be cleared */
					ASSERT(bsscfg->mc_fifo_pkts == 0);
#endif
					WLCNTINCR(bsscfg->cnt->ps_trans);
					WL_NONE(("wl%d.%d: DONE PS-ON\n",
						wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
				}
				bcmc_scb->PS = TRUE;
			} else { /* Unaffected BSS or transition aborted for this BSS */
				bcmc_scb->PS = FALSE;
			}
		}
	}
}

/*
 * Last STA for a BSS exitted PS; BSS has no pkts in BC/MC fifo.
 * Check whether other stations have entered PS since and update
 * state accordingly.
 *
 * That is, it is possible that the BSS state will remain PS
 * TRUE (PS delivery mode enabled) if a STA has changed to PS-ON
 * since the start of the PS-OFF transition.
 */

void
wlc_apps_bss_ps_off_done(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	struct scb *bcmc_scb;

	ASSERT(bsscfg->bcmc_fid_shm == INVALIDFID);
	ASSERT(bsscfg->bcmc_fid == INVALIDFID);

	bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
	ASSERT(SCB_PS(bcmc_scb));

	if (BSS_PS_NODES(wlc, bsscfg) != 0) {
		/* Aborted transtion:  Set PS delivery mode */
		bcmc_scb->PS = TRUE;
	} else { /* Completed transition: Clear PS delivery mode */
		bcmc_scb->PS = FALSE;
		WLCNTINCR(bsscfg->cnt->ps_trans);
		if (bsscfg->flags & WLC_BSSCFG_PS_OFF_TRANS) {
			WL_PS(("wl%d.%d: DONE PS-OFF.\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		}
	}

	bsscfg->flags &= ~WLC_BSSCFG_PS_OFF_TRANS; /* Clear transition flag */

	/* Forward any packets in MC-PSQ according to new state */
	while (wlc_apps_ps_send(wlc, bcmc_scb, WLC_PREC_BMP_ALL, 0))
		/* Repeat until queue empty */
		;

#if defined(BCMDBG_MBSS_PROFILE)
	if (bsscfg->ps_start_us != 0) {
		uint32 diff_us;

		diff_us = R_REG(wlc->osh, &wlc->regs->tsf_timerlow) - bsscfg->ps_start_us;
		if (diff_us > bsscfg->max_ps_off_us) bsscfg->max_ps_off_us = diff_us;
		bsscfg->tot_ps_off_us += diff_us;
		bsscfg->ps_off_count++;
		bsscfg->ps_start_us = 0;
	}
#endif /* BCMDBG_MBSS_PROFILE */
}

#endif /* WLC_HIGH && MBSS */
#endif /* MBSS */
