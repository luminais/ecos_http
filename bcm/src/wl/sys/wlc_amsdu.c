/*
 * MSDU aggregation protocol source file
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
 * $Id: wlc_amsdu.c,v 1.117.2.2 2010-05-19 02:13:40 Exp $
 */

#ifndef WLAMSDU
#error "WLAMSDU is not defined"
#endif

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_frmutil.h>
#ifdef WLAMSDU
#include <wlc_amsdu.h>
#endif

/* 
 * A-MSDU agg flow control
 * if txpend/scb/prio >= WLC_AMSDU_HIGH_WATERMARK, start aggregating
 * if txpend/scb/prio <= WLC_AMSDU_LOW_WATERMARK, stop aggregating
 */
#define WLC_AMSDU_LOW_WATERMARK		1
#define WLC_AMSDU_HIGH_WATERMARK	1

/* default values for tunables, iovars */

#define CCKOFDM_PLCP_MPDU_MAX_LENGTH	8096	/* max MPDU length: 13 bits len field in PLCP */
#define MIMO_PLCP_MPDU_MAX_LENGTH	64*1024	/* max MPDU length: 16 bits len field in PLCP */
#define AMSDU_MAX_MSDU_PKTLEN		1600	/* max pkt length to be aggregated */

#define AMSDU_AGGBYTES_MIN		500	/* the lowest aggbytes allowed */
#define MAX_TX_SUBFRAMES_LIMIT		16	/* the highest aggsf allowed */

#ifdef WLC_HIGH_ONLY
#define MAX_TX_SUBFRAMES		10	/* max num of MSDUs in one tx A-MSDU */
#define	MAX_RX_SUBFRAMES		15	/* max A-MSDU rx size / smallest frame bytes */

#else
#ifdef WLLMAC
#define MAX_RX_SUBFRAMES		(NRXD - 1)
#else
#define	MAX_RX_SUBFRAMES		100	/* max A-MSDU rx size / smallest frame bytes */
#endif

#define MAX_TX_SUBFRAMES		14	/* max num of MSDUs in on A-MSDU */
#endif	/* WLC_HIGH_ONLY */


/* sw rx private states */
#define WLC_AMSDU_DEAGG_IDLE 		0	/* idle */
#define WLC_AMSDU_DEAGG_FIRST		1	/* deagg first frame received */
#define WLC_AMSDU_DEAGG_LAST		3	/* deagg last frame received */

#define WLC_AMSDU_DEAGG_IN_PROGRESS(ami)	!((ami)->amsdu_deagg_state == WLC_AMSDU_DEAGG_IDLE)

#ifdef WLCNT
#define	WLC_AMSDU_CNT_VERSION	1	/* current version of wlc_amsdu_cnt_t */

/* block ack related stats */
typedef struct wlc_amsdu_cnt {
	uint16	version;	/* WLC_AMSDU_CNT_VERSION */
	uint16	length;		/* length of entire structure */

	uint32	agg_openfail;		/* num of MSDU open failure */
	uint32	agg_passthrough;	/* num of MSDU pass through w/o A-MSDU agg */
	uint32	agg_block;		/* num of MSDU blocked in A-MSDU agg */
	uint32	agg_amsdu;		/* num of A-MSDU released */
	uint32	agg_msdu;		/* num of MSDU aggregated in A-MSDU */
	uint32	deagg_msdu;		/* MSDU of deagged A-MSDU(in ucode) */
	uint32	deagg_amsdu;		/* valid A-MSDU deagged(exclude bad A-MSDU) */
	uint32	deagg_badfmt;		/* MPDU is bad */
	uint32	deagg_wrongseq;		/* MPDU of one A-MSDU doesn't follow sequence */
	uint32	deagg_badsflen;		/* MPDU of one A-MSDU has length mismatch */
	uint32	deagg_badsfalign;	/* MPDU of one A-MSDU is not aligned to 4 bytes boundary */
	uint32  deagg_badtotlen;	/* A-MSDU tot length doesn't match summation of all sfs */
	uint32	deagg_openfail;		/* A-MSDU deagg open failures */
	uint32	deagg_swdeagglong;	/* A-MSDU sw_deagg doesn't handle long pkt */
} wlc_amsdu_cnt_t;
#endif	/* WLCNT */

/* iovar table */
enum {
	IOV_AMSDU_SIM,
	IOV_AMSDU_HIWM,
	IOV_AMSDU_LOWM,
	IOV_AMSDU_AGGSF,	/* num of subframes in one A-MSDU */
	IOV_AMSDU_AGGBYTES,	/* num of bytes in one A-MSDU */
	IOV_AMSDU_RXMAX,	/* get/set HT_CAP_MAX_AMSDU in HT cap field */
	IOV_AMSDU_BLOCK,	/* block amsdu agg */
	IOV_AMSDU_FLUSH,	/* flush all amsdu agg queues */
	IOV_AMSDU_DEAGGDUMP,	/* dump deagg pkt */

	IOV_AMSDU_COUNTERS,
	IOV_AMSDU_CLEAR_COUNTERS,

	IOV_AMSDUNOACK,

	IOV_AMSDU
};

static const bcm_iovar_t amsdu_iovars[] = {
	{"amsdu", IOV_AMSDU, (IOVF_SET_DOWN), IOVT_BOOL, 0},	/* allow if we are down */
	{"amsdu_noack", IOV_AMSDUNOACK, (0), IOVT_BOOL, 0},
	{"amsdu_aggsf", IOV_AMSDU_AGGSF, (0), IOVT_UINT16, 0},
	{"amsdu_aggbytes", IOV_AMSDU_AGGBYTES, (0), IOVT_UINT32, 0},
	{"amsdu_rxmax", IOV_AMSDU_RXMAX, (IOVF_SET_DOWN), IOVT_BOOL, 0},
#ifdef BCMDBG
#ifdef WLCNT
	{"amsdu_counters", IOV_AMSDU_COUNTERS, (0), IOVT_BUFFER, sizeof(wlc_amsdu_cnt_t)},
	{"amsdu_clear_counters", IOV_AMSDU_CLEAR_COUNTERS, 0, IOVT_VOID, 0},
#endif
#endif	/* BCMDBG */
	{NULL, 0, 0, 0, 0}
};

/* principle amsdu module local structure per device instance */
struct amsdu_info {
	wlc_info_t	*wlc;			/* pointer to main wlc structure */
	wlc_pub_t	*pub;			/* public common code handler */
	int	scb_handle;			/* scb cubby handle to retrieve data from scb */

	uint16	mac_rcvfifo_limit;		/* max rx fifo in bytes */
	uint16	amsdu_rx_mtu;			/* amsdu MTU, depend on rx fifo limit */
	bool	amsdu_rxcap_big;		/* TRUE: rx big amsdu capable (HT_MAX_AMSDU) */

	uint16	fifo_lowm;			/* low watermark for tx queue precendence */
	uint16	fifo_hiwm;			/* high watermark for tx queue precendence */

	bool	amsdu_agg_block;		/* global override: disable amsdu tx */

	bool	amsdu_agg_allowprio[NUMPRIO];	/* A-MSDU agg. TRUE: allowed, FALSE: disalowed */
	uint	amsdu_agg_bytes_limit[NUMPRIO];	/* max AMSDU bytes per priority */
	uint	amsdu_agg_sframes_limit[NUMPRIO];	/* max number of subframes in one A-MSDU */

	/* rx: one stream per device */
	int	amsdu_deagg_state;		/* A-MSDU deagg statemachine per device */
	void	*amsdu_deagg_p;			/* pointer to first pkt buffer in A-MSDU chain */
	void	*amsdu_deagg_ptail;		/* pointer to last pkt buffer in A-MSDU chain */
#ifdef WLCNT
	wlc_amsdu_cnt_t *cnt;			/* counters/stats */
#endif /* WLCNT */
};

typedef struct {
	uint	amsdu_agg_bytes_max;	/* max AMSDU bytes negotiated */
	uint	amsdu_agg_bytes;	/* A-MSDU byte count */
	uint	amsdu_agg_sframes;	/* A-MSDU subframe count */
	void	*amsdu_agg_p;		/* A-MSDU pkt pointer to first MSDU */
	void	*amsdu_agg_ptail;	/* A-MSDU pkt pointer to last MSDU */
	uint	amsdu_agg_padlast;	/* pad bytes in the agg tail buffer */
	uint	amsdu_agg_txpending;
	bool	amsdu_agg_allowtid;	/* TRUE: agg is allowed, FALSE: agg is disallowed */
} amsdu_scb_prio;

/* per scb cubby info */
typedef struct scb_amsduinfo {
	amsdu_scb_prio prio[NUMPRIO];
} scb_amsdu_t;

#define SCB_AMSDU_CUBBY(ami, scb) (scb_amsdu_t *)SCB_CUBBY((scb), (ami)->scb_handle)

/* A-MSDU general */
static int wlc_amsdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *arg, int alen, int val_size, struct wlc_if *wlcif);

static void wlc_amsdu_mtu_init(amsdu_info_t *ami);
static int wlc_amsdu_down(void *hdl);

#ifdef WLAMSDU_TX
static int wlc_amsdu_scb_init(void *cubby, struct scb *scb);
static void wlc_amsdu_scb_deinit(void *cubby, struct scb *scb);
/* A-MSDU aggregation */
static void  wlc_amsdu_agg(void *ctx, struct scb *scb, void *p, uint prec);
static void* wlc_amsdu_agg_open(amsdu_info_t *ami, struct ether_addr *ea, void *p);
static bool  wlc_amsdu_agg_append(amsdu_info_t *ami, struct scb *scb, void *p, uint tid);
static void  wlc_amsdu_agg_close(amsdu_info_t *ami, struct scb *scb, uint tid);
static void  wlc_amsdu_agg_flush(amsdu_info_t *ami);
static void  wlc_amsdu_scb_deactive(void *ctx, struct scb *scb);
static uint  wlc_amsdu_txpktcnt(void *ctx);

#ifdef BCMDBG
static void wlc_amsdu_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#endif

static txmod_fns_t amsdu_txmod_fns = {
	wlc_amsdu_agg,
	wlc_amsdu_txpktcnt,
	wlc_amsdu_scb_deactive,
	NULL
};
#endif /* WLAMSDU_TX */

#ifdef BCMDBG
static int wlc_amsdu_dump(amsdu_info_t *ami, struct bcmstrbuf *b);
#endif

/* A-MSDU deaggregation */
static bool wlc_amsdu_deagg_open(amsdu_info_t *ami, void *p);
static bool wlc_amsdu_deagg_verify(amsdu_info_t *ami, uint16 fc, void *h);
static void wlc_amsdu_deagg_flush(amsdu_info_t *ami);
static int BCMATTACHFN(wlc_amsdu_tx_attach)(amsdu_info_t *ami, wlc_info_t *wlc);


#if defined(BCMDBG) && defined(WLCNT)
void wlc_amsdu_dump_cnt(amsdu_info_t *ami, struct bcmstrbuf *b);
#endif	/* defined(BCMDBG) && defined(WLCNT) */

amsdu_info_t *
BCMATTACHFN(wlc_amsdu_attach)(wlc_info_t *wlc)
{
	amsdu_info_t *ami;

	if (!(ami = (amsdu_info_t *)MALLOC(wlc->osh, sizeof(amsdu_info_t)))) {
		WL_ERROR(("wl%d: wlc_amsdu_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}
	bzero((char *)ami, sizeof(amsdu_info_t));
	ami->wlc = wlc;
	ami->pub = wlc->pub;

#ifdef WLCNT
	if (!(ami->cnt = (wlc_amsdu_cnt_t *)MALLOC(wlc->osh, sizeof(wlc_amsdu_cnt_t)))) {
		WL_ERROR(("wl%d: wlc_amsdu_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		goto fail;
	}
	bzero((char *)ami->cnt, sizeof(wlc_amsdu_cnt_t));
#endif /* WLCNT */

	/* register module */
	if (wlc_module_register(ami->pub, amsdu_iovars, "amsdu",
		ami, wlc_amsdu_doiovar, NULL, NULL, wlc_amsdu_down)) {
		WL_ERROR(("wl%d: wlc_amsdu_attach: wlc_module_register failed\n", wlc->pub->unit));
		goto fail;
	}
#ifdef BCMDBG
	wlc_dump_register(ami->pub, "amsdu", (dump_fn_t)wlc_amsdu_dump, (void *)ami);
#endif

	ami->fifo_lowm = (uint16)WLC_AMSDU_LOW_WATERMARK;
	ami->fifo_hiwm = (uint16)WLC_AMSDU_HIGH_WATERMARK;

	if (wlc_amsdu_tx_attach(ami, wlc) < 0) {
		WL_ERROR(("wl%d: wlc_amsdu_attach: Error initing the amsdu tx\n", wlc->pub->unit));
		goto fail;
	}
	wlc_amsdu_mtu_init(ami);

	/* to be compatible with spec limit */
	if (wlc->pub->tunables->nrxd < MAX_RX_SUBFRAMES) {
		WL_ERROR(("NRXD %d is too small to fit max amsdu rxframe\n",
		          (uint)wlc->pub->tunables->nrxd));
	}

	return ami;

fail:
	wlc_amsdu_detach(ami);
	return NULL;
}

static int
BCMATTACHFN(wlc_amsdu_tx_attach)(amsdu_info_t *ami, wlc_info_t *wlc)
{
#ifdef WLAMSDU_TX
	uint i;

	/* reserve cubby in the scb container for per-scb private data */
	ami->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(scb_amsdu_t),
		wlc_amsdu_scb_init, wlc_amsdu_scb_deinit,
#ifdef BCMDBG
		wlc_amsdu_dump_scb,
#else
		NULL,
#endif
		(void *)ami);

	if (ami->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_amsdu_attach: wlc_scb_cubby_reserve failed\n",
		          wlc->pub->unit));
		return -1;
	}

	/* register txmod call back */
	wlc_txmod_fn_register(wlc, TXMOD_AMSDU, ami, amsdu_txmod_fns);

	WLCNTSET(ami->cnt->version, WLC_AMSDU_CNT_VERSION);
	WLCNTSET(ami->cnt->length, sizeof(ami->cnt));

	/* init tunables */
	for (i = 0; i < NUMPRIO; i++) {
		uint fifo_size;

		ami->amsdu_agg_allowprio[i] = TRUE;

		/* set agg_bytes_limit to standard maximum if hw fifo allows
		 *  this value can be changed via iovar or fragthreshold later
		 *  it can never exceed hw fifo limit since A-MSDU is not streaming
		 */
		fifo_size = wlc->xmtfifo_szh[prio2fifo[i]];
		fifo_size = fifo_size * TXFIFO_SIZE_UNIT;	/* blocks to bytes */

		ami->amsdu_agg_bytes_limit[i] = MIN(HT_MAX_AMSDU, fifo_size);

		ami->amsdu_agg_sframes_limit[i] = MAX_TX_SUBFRAMES;
		/* DMA: leave empty room for DMA descriptor table */
		if (ami->amsdu_agg_sframes_limit[i] > (uint)(wlc->pub->tunables->ntxd/3)) {
			WL_ERROR(("NTXD %d is too small to fit max amsdu txframe\n",
			          (uint)wlc->pub->tunables->ntxd));
			ASSERT(0);
		}

		/* TODO: PIO */
	}
#endif /* WLAMSDU_TX */
	return 0;
}


void
BCMATTACHFN(wlc_amsdu_detach)(amsdu_info_t *ami)
{
	if (!ami)
		return;

	wlc_amsdu_deagg_flush(ami);

	wlc_module_unregister(ami->pub, "amsdu", ami);

#ifdef WLCNT
	if (ami->cnt)
		MFREE(ami->pub->osh, ami->cnt, sizeof(wlc_amsdu_cnt_t));
#endif /* WLCNT */
	MFREE(ami->pub->osh, ami, sizeof(amsdu_info_t));
}

static void
wlc_amsdu_mtu_init(amsdu_info_t *ami)
{
	ami->mac_rcvfifo_limit = wlc_rcvfifo_limit_get(ami->wlc);

	ami->amsdu_rxcap_big = ((ami->mac_rcvfifo_limit - ami->wlc->hwrxoff - 100) >= HT_MAX_AMSDU);

	ami->amsdu_rx_mtu = ami->amsdu_rxcap_big ? HT_MAX_AMSDU : HT_MIN_AMSDU;
}

uint16
wlc_amsdu_mtu_get(amsdu_info_t *ami)
{
	return ami->amsdu_rx_mtu;
}

/* AMSDU tx is optional, sw can turn it on or off even HW supports */
bool
wlc_amsdutx_cap(amsdu_info_t *ami)
{
#ifdef WLC_HIGH_ONLY	    /* complicate to support */
	return FALSE;
#else

#if defined(WL11N) && defined(WLAMSDU_TX)
	if (ami->pub->phy_11ncapable)
		return (TRUE);
#endif
	return (FALSE);
#endif /* WLC_HIGH_ONLY */
}

/* AMSDU rx is mandatory for NPHY */
bool
wlc_amsdurx_cap(amsdu_info_t *ami)
{
#ifdef WL11N
	if (ami->pub->phy_11ncapable)
		return (TRUE);
#endif

	return (FALSE);
}

int
wlc_amsdu_set(amsdu_info_t *ami, bool on)
{
	wlc_info_t *wlc = ami->wlc;

	WL_AMSDU(("wlc_amsdu_set val=%d\n", on));

	if (on) {
		if (!N_ENAB(wlc->pub)) {
			WL_AMSDU(("wl%d: driver not nmode enabled\n", wlc->pub->unit));
			return BCME_UNSUPPORTED;
		}
		if (!wlc_amsdutx_cap(ami)) {
			WL_AMSDU(("wl%d: device not amsdu capable\n", wlc->pub->unit));
			return BCME_UNSUPPORTED;
		} else if (AMPDU_ENAB(wlc->pub)) {
			WL_AMSDU(("wl%d: A-MSDU doesn't work with AMPDU\n", wlc->pub->unit));
			return BCME_UNSUPPORTED;
		}
	}

	/* This controls AMSDU agg only, AMSDU deagg is on by default per spec */
	wlc->pub->_amsdu_tx = on;
	wlc_update_brcm_ie(ami->wlc);
#ifdef WLAFTERBURNER
	wlc->pub->txmaxpkts = wlc->pub->_amsdu_tx ? MAXTXPKTS_AB : MAXTXPKTS;
#else
	wlc->pub->txmaxpkts = MAXTXPKTS;
#endif /* WLAFTERBURNER */
#ifdef WLAMSDU_TX
	if (!wlc->pub->_amsdu_tx)
		wlc_amsdu_agg_flush(ami);
#endif /* WLAMSDU_TX */

	return (0);
}
#ifdef WLAMSDU_TX
static int
wlc_amsdu_scb_init(void *context, struct scb *scb)
{
	uint i;
	amsdu_info_t *ami = (amsdu_info_t *)context;
	scb_amsdu_t *scb_amsdu = SCB_AMSDU_CUBBY(ami, scb);
	amsdu_scb_prio *amsduprio;

	WL_AMSDU(("wlc_amsdu_scb_init scb %p\n", scb));

	ASSERT(scb_amsdu);
	for (i = 0; i < NUMPRIO; i++) {
		amsduprio = &scb_amsdu->prio[i];
		amsduprio->amsdu_agg_p = NULL;
		amsduprio->amsdu_agg_ptail = NULL;
		amsduprio->amsdu_agg_sframes = 0;
		amsduprio->amsdu_agg_bytes = 0;
		amsduprio->amsdu_agg_bytes_max = AMSDU_MAX_MSDU_PKTLEN;
		amsduprio->amsdu_agg_padlast = 0;
		amsduprio->amsdu_agg_txpending = 0;
		amsduprio->amsdu_agg_allowtid = TRUE;
	}
	return 0;
}

static void
wlc_amsdu_scb_deinit(void *context, struct scb *scb)
{
	uint i;
	amsdu_info_t *ami = (amsdu_info_t *)context;
	scb_amsdu_t *scb_amsdu = SCB_AMSDU_CUBBY(ami, scb);

	WL_AMSDU(("wlc_amsdu_scb_deinit scb %p\n", scb));

	ASSERT(scb_amsdu);

	/* release tx agg pkts */
	for (i = 0; i < NUMPRIO; i++) {
		if (scb_amsdu->prio[i].amsdu_agg_p)
			PKTFREE(ami->wlc->osh, scb_amsdu->prio[i].amsdu_agg_p, TRUE);
	}
}
#endif /* WLAMSDU_TX */
/* handle AMSDU related items when going down */
static int wlc_amsdu_down(void *hdl)
{
	amsdu_info_t *ami = (amsdu_info_t *)hdl;

	WL_AMSDU(("wlc_amsdu_down: entered\n"));

	/* Flush the deagg Q, there may be packets there */
	wlc_amsdu_deagg_flush(ami);

	return 0;
}

/* handle AMSDU related iovars */
static int
wlc_amsdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif)
{
	amsdu_info_t *ami = (amsdu_info_t *)hdl;
	int32 int_val = 0;
	uint32 uint_val;
	bool bool_val;
	int err = 0, i;
	wlc_info_t *wlc;

	i = 0;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	uint_val = (uint)int_val;
	bool_val = (int_val != 0) ? TRUE : FALSE;
	wlc = ami->wlc;
	ASSERT(ami == wlc->ami);

	switch (actionid) {
	case IOV_GVAL(IOV_AMSDU):
		int_val = wlc->pub->_amsdu_tx;
		bcopy(&int_val, a, val_size);
		break;

	case IOV_SVAL(IOV_AMSDU):
		err = wlc_amsdu_set(ami, bool_val);
		break;

	case IOV_GVAL(IOV_AMSDUNOACK):
		int_val = wlc->_amsdu_noack;
		bcopy(&int_val, a, val_size);
		break;

	case IOV_SVAL(IOV_AMSDUNOACK):
		wlc->_amsdu_noack = bool_val;
		break;

#ifdef BCMDBG
#ifdef WLCNT
	case IOV_GVAL(IOV_AMSDU_COUNTERS):
		bcopy(&ami->cnt, a, sizeof(ami->cnt));
		break;

	case IOV_SVAL(IOV_AMSDU_CLEAR_COUNTERS):
		bzero(ami->cnt, sizeof(ami->cnt));
		break;
#endif /* WLCNT */
#endif /* BCMDBG */

#ifdef WLAMSDU_TX
	case IOV_GVAL(IOV_AMSDU_AGGBYTES):
		/* TODO, support all priorities ? */
		int_val = ami->amsdu_agg_bytes_limit[PRIO_8021D_BE];
		bcopy(&int_val, a, val_size);
		break;

	case IOV_SVAL(IOV_AMSDU_AGGBYTES): {
		struct scb *scb;
		struct scb_iter scbiter;

		if (uint_val > (uint)(((WLC_PHY_11N_CAP(wlc->band))) ?
			MIMO_PLCP_MPDU_MAX_LENGTH : CCKOFDM_PLCP_MPDU_MAX_LENGTH)) {
			err = BCME_RANGE;
			break;
		}

		if (uint_val < AMSDU_AGGBYTES_MIN) {
			err = BCME_RANGE;
			break;
		}

		/* if smaller, flush existing aggregation, care only BE for now */
		if (uint_val < ami->amsdu_agg_bytes_limit[PRIO_8021D_BE])
			wlc_amsdu_agg_flush(ami);

		for (i = 0; i < NUMPRIO; i++) {
			uint fifo_size;
			fifo_size = wlc->xmtfifo_szh[prio2fifo[i]];
			fifo_size = fifo_size * TXFIFO_SIZE_UNIT;	/* blocks to bytes */
			ami->amsdu_agg_bytes_limit[i] = MIN(uint_val, fifo_size);
		}

		/* update amsdu agg bytes for ALL scbs */
		FOREACHSCB(wlc->scbstate, &scbiter, scb)
			wlc_amsdu_scb_agglimit_upd(ami, scb);

		break;
	}
	case IOV_GVAL(IOV_AMSDU_AGGSF):
		/* TODO, support all priorities ? */
		*(uint*)a = ami->amsdu_agg_sframes_limit[PRIO_8021D_BE];
		break;

	case IOV_SVAL(IOV_AMSDU_AGGSF):
		if ((int_val > MAX_TX_SUBFRAMES_LIMIT) || (int_val > wlc->pub->tunables->ntxd/2)) {
			err = BCME_RANGE;
			break;
		}

		for (i = 0; i < NUMPRIO; i++)
			ami->amsdu_agg_sframes_limit[i] = int_val;
		break;
#endif /* WLAMSDU_TX */

	case IOV_GVAL(IOV_AMSDU_RXMAX):
		int_val = (int8)((wlc->ht_cap.cap & HT_CAP_MAX_AMSDU) != 0);
		bcopy(&int_val, a, val_size);
		break;

	case IOV_SVAL(IOV_AMSDU_RXMAX):
		wlc->ht_cap.cap &= ~HT_CAP_MAX_AMSDU;

		ASSERT(wlc_amsdu_mtu_get(ami) >= HT_MIN_AMSDU);

		if (bool_val) {
			if (wlc_amsdu_mtu_get(ami) < HT_MAX_AMSDU) {
				err = BCME_RANGE;
				break;
			} else
				wlc->ht_cap.cap |= HT_CAP_MAX_AMSDU;
		}

		break;

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

#ifdef WLAMSDU_TX
/* 
 * called from fragthresh changes ONLY: update agg bytes limit, toss buffered A-MSDU
 * This is expected to happen very rarely since user should use very standard 802.11 fragthreshold
 *  to "disabled" fragmentation when enable A-MSDU. We can even ignore that. But to be
 *  full spec compliant, we reserve this capability.
 *   ??? how to inform user the requirement that not changing FRAGTHRESHOLD to screw up A-MSDU
 */
void
wlc_amsdu_agglimit_frag_upd(amsdu_info_t *ami)
{
	uint i;
	wlc_info_t *wlc = ami->wlc;
	struct scb *scb;
	struct scb_iter scbiter;
	bool flush = FALSE;

	WL_AMSDU(("wlc_amsdu_agg_limit_upd\n"));

	if (!(WLC_PHY_11N_CAP(wlc->band)))
		return;

	for (i = 0; i < NUMPRIO; i++) {
		/* default value means no fragmentation */
		if (wlc->fragthresh[prio2fifo[i]] == DOT11_MAX_FRAG_LEN)
			continue;

		if (wlc->fragthresh[prio2fifo[i]] < ami->amsdu_agg_bytes_limit[i]) {
			flush = TRUE;
			ami->amsdu_agg_bytes_limit[i] = wlc->fragthresh[prio2fifo[i]];
			WL_INFORM(("wlc_amsdu_agg_frag_upd: amsdu_aggbytes[%d] = %d due to frag!\n",
				i, ami->amsdu_agg_bytes_limit[i]));
		}

		ami->amsdu_agg_allowprio[i] = (ami->amsdu_agg_bytes_limit[i] > AMSDU_AGGBYTES_MIN);
		if (!ami->amsdu_agg_allowprio[i])
			WL_INFORM(("wlc_amsdu_agg_frag_upd: fragthresh is too small for AMSDU %d\n",
				i));
	}

	/* toss A-MSDU since bust it up is very expensive, can't push through */
	if (flush)
		wlc_amsdu_agg_flush(ami);

	/* update all scb limit */
	FOREACHSCB(wlc->scbstate, &scbiter, scb)
		wlc_amsdu_scb_agglimit_upd(ami, scb);
}

/* deal with WME txop dynamically shrink */
void
wlc_amsdu_txop_upd(amsdu_info_t *ami)
{
}

void
wlc_amsdu_scb_agglimit_upd(amsdu_info_t *ami, struct scb *scb)
{
	uint i;
	scb_amsdu_t *scb_ami;

	WL_AMSDU(("wlc_amsdu_scb_agglimit_upd, scb->amsdu_mtu_pref %d\n",
		scb->amsdu_mtu_pref));

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);

	for (i = 0; i < NUMPRIO; i++) {
		scb_ami->prio[i].amsdu_agg_bytes_max = MIN(scb->amsdu_mtu_pref,
			ami->amsdu_agg_bytes_limit[i]);
	}
}

/* A-MSDU admission control, per-scb-tid. 
 * called from tx completion, to decrement agg_txpend, compare with LOWM/HIWM
 * - this is called regardless the tx frame is AMSDU or not. the amsdu_agg_txpending
 *   increment/decrement for any traffic for scb-tid.
 * - work on best-effort traffic only for now, can be expanded to other in the future
 * - amsdu_agg_txpending never go below 0
 * - amsdu_agg_txpending may not be accurate before/after A-MSDU agg is added to txmodule
 *   config/unconfig dynamically
 */
void
wlc_amsdu_dotxstatus(amsdu_info_t *ami, struct scb *scb, void* p)
{
	uint tid;
	scb_amsdu_t *scb_ami;

	WL_AMSDU(("wlc_amsdu_dotxstatus\n"));

	ASSERT(scb && SCB_AMSDU(scb));

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	ASSERT(scb_ami);

	tid = (uint8)PKTPRIO(p);
	if (PRIO_8021D_BE != tid) {
		WL_AMSDU(("wlc_amsdu_dotxstatus, tid %d\n", tid));
		return;
	}

	if (scb_ami->prio[tid].amsdu_agg_txpending > 0)
		scb_ami->prio[tid].amsdu_agg_txpending--;
	WL_AMSDU(("wlc_amsdu_dotxstatus: scb txpending reduce to %d\n",
		scb_ami->prio[tid].amsdu_agg_txpending));

	if ((scb_ami->prio[tid].amsdu_agg_txpending < ami->fifo_lowm)) {
		if (scb_ami->prio[tid].amsdu_agg_p) {
			WL_AMSDU(("wlc_amsdu_dotxstatus: release amsdu due to low watermark!!\n"));
			wlc_amsdu_agg_close(ami, scb, tid);
		}
	}
}

/* centralize A-MSDU tx policy */
void
wlc_amsdu_txpolicy_upd(amsdu_info_t *ami)
{
	WL_AMSDU(("wlc_amsdu_txpolicy_upd\n"));

	if (PIO_ENAB(ami->pub))
		ami->amsdu_agg_block = TRUE;
	else {
		int idx;
		wlc_bsscfg_t *cfg;
		FOREACH_BSS(ami->wlc, idx, cfg) {
			if (!cfg->BSS)
				ami->amsdu_agg_block = TRUE;
		}
	}

	ami->amsdu_agg_block = FALSE;
}

/*
 * MSDU aggregation, per-A1-TID(priority)
 * Return TRUE if packet consumed, otherwise FALSE
 */
static void
wlc_amsdu_agg(void *ctx, struct scb *scb, void *p, uint prec)
{
	amsdu_info_t *ami;
	struct ether_header *eh;
	osl_t *osh;
	wlc_info_t *wlc;
	uint tid = 0;
	scb_amsdu_t *scb_ami;
	wlc_bsscfg_t *bsscfg;
	uint totlen;

	ami = (amsdu_info_t *)ctx;
	osh = ami->pub->osh;
	wlc = ami->wlc;
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);

	WL_AMSDU(("wlc_amsdu_agg\n"));

	if (ami->amsdu_agg_block)
		goto passthrough;

	/* doesn't handle MPDU,  */
	if (WLPKTTAG(p)->flags & WLF_MPDU)
		goto passthrough;

	/* non best-effort, skip for now */
	tid = PKTPRIO(p);
	ASSERT(tid <= NUMPRIO);
	if (tid != PRIO_8021D_BE)
		goto passthrough;

	/* admission control */
	if (!ami->amsdu_agg_allowprio[tid])
		goto passthrough;

	if (WLPKTTAG(p)->flags & WLF_8021X)
		goto passthrough;


	eh = (struct ether_header*) PKTDATA(osh, p);

	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb);

	/* the scb must be A-MSDU capable */
	ASSERT(SCB_AMSDU(scb));
	ASSERT(SCB_QOS(scb));

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	if (WSEC_ENABLED(bsscfg->wsec) && !WSEC_AES_ENABLED(bsscfg->wsec)) {
		WL_AMSDU(("wlc_amsdu_agg: target scb %p is has wrong WSEC\n", scb));
		goto passthrough;
	}

	WL_AMSDU(("wlc_amsdu_agg: txpend %d\n", scb_ami->prio[tid].amsdu_agg_txpending));
	if (scb_ami->prio[tid].amsdu_agg_txpending < ami->fifo_hiwm) {
		goto passthrough;
	} else {
		WL_AMSDU(("wlc_amsdu_agg: Starts aggregation due to hiwm %d reached\n",
			ami->fifo_hiwm));
	}

	totlen = pkttotlen(osh, p) + scb_ami->prio[tid].amsdu_agg_bytes;

	if ((totlen > scb_ami->prio[tid].amsdu_agg_bytes_max - ETHER_HDR_LEN) ||
		(scb_ami->prio[tid].amsdu_agg_sframes + 1 >
			  ami->amsdu_agg_sframes_limit[tid])) {
		WL_AMSDU(("wlc_amsdu_agg: terminte A-MSDU for txbyte %d or txframe %d\n",
			scb_ami->prio[tid].amsdu_agg_bytes_max,
			ami->amsdu_agg_sframes_limit[tid]));
		wlc_amsdu_agg_close(ami, scb, tid);

		/* if the new pkt itself is more than aggmax, can't continue with agg_append
		 *   add here to avoid per pkt checking for this rare case
		 */
		if (pkttotlen(osh, p) > scb_ami->prio[tid].amsdu_agg_bytes_max - ETHER_HDR_LEN) {
			WL_AMSDU(("wlc_amsdu_agg: A-MSDU aggmax is smaller than pkt %d, pass\n",
				pkttotlen(osh, p)));

			goto passthrough;
		}
	}


	/* agg this one and return on success */
	if (wlc_amsdu_agg_append(ami, scb, p, tid))
		return;

passthrough:
	/* A-MSDU agg rejected, pass through to next tx module */

	/* release old first before passthrough new one to maintain sequence */
	if (scb_ami->prio[tid].amsdu_agg_p) {
		WL_AMSDU(("wlc_amsdu_agg: release amsdu due to low watermark!!\n"));
		wlc_amsdu_agg_close(ami, scb, tid);
	}

	scb_ami->prio[tid].amsdu_agg_txpending++;
	WLCNTINCR(ami->cnt->agg_passthrough);
	WL_AMSDU(("wlc_amsdu_agg: passthrough scb %p txpending %d\n", scb,
		scb_ami->prio[tid].amsdu_agg_txpending));

	SCB_TX_NEXT(TXMOD_AMSDU, scb, p, WLC_PRIO_TO_PREC(tid));
	return;
}

/* close A-MSDU
 * ??? cck rate is not supported in hw, how to restrict rate algorithm later
 */
static void
wlc_amsdu_agg_close(amsdu_info_t *ami, struct scb *scb, uint tid)
{
	scb_amsdu_t *scb_ami;
	void *ptid;
	osl_t *osh;
	uint16 etype;
	struct ether_header *eh;
	amsdu_scb_prio *amsduprio;

	WL_AMSDU(("wlc_amsdu_agg_close\n"));

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	osh = ami->pub->osh;
	amsduprio = &scb_ami->prio[tid];
	ptid = amsduprio->amsdu_agg_p;

	if (ptid == NULL)
		return;

	ASSERT(WLPKTFLAG_AMSDU(WLPKTTAG(ptid)));

	/* wlc_pkt_callback_register(wlc, wlc_amsdu_tx_complete, ami, ptid); */

	/* check */
	ASSERT(PKTLEN(ami->pub->osh, ptid) >= ETHER_HDR_LEN);
	ASSERT(tid == (uint)PKTPRIO(ptid));

	/* FIXUP lastframe pad --- the last subframe must not be padded, 
	 * reset pktlen to the real length(strip off pad) using previous
	 * saved value.
	 * amsduprio->amsdu_agg_ptail points to the last buf(not last pkt)
	 */
	if (amsduprio->amsdu_agg_padlast) {
		PKTSETLEN(osh, amsduprio->amsdu_agg_ptail,
			PKTLEN(osh, amsduprio->amsdu_agg_ptail) -
			amsduprio->amsdu_agg_padlast);
		eh = (struct ether_header*) PKTDATA(osh, ptid);
		etype = ntoh16(eh->ether_type);
		ASSERT(etype + (uint)ETHER_HDR_LEN == amsduprio->amsdu_agg_bytes);

		amsduprio->amsdu_agg_bytes -= amsduprio->amsdu_agg_padlast;
		eh->ether_type = hton16(etype - amsduprio->amsdu_agg_padlast);

		WL_AMSDU(("wlc_amsdu_agg_close: strip off padlast %d\n",
			amsduprio->amsdu_agg_padlast));
	}


	WLCNTINCR(ami->cnt->agg_amsdu);
	WLCNTADD(ami->cnt->agg_msdu, amsduprio->amsdu_agg_sframes);

	amsduprio->amsdu_agg_txpending++;

	WL_AMSDU(("wlc_amsdu_agg_close: valid AMSDU, add to txq %d bytes, scb %p, txpending %d\n",
		amsduprio->amsdu_agg_bytes, scb, scb_ami->prio[tid].amsdu_agg_txpending));

	SCB_TX_NEXT(TXMOD_AMSDU, scb, ptid, WLC_PRIO_TO_PREC(tid));

	amsduprio->amsdu_agg_p = NULL;
	amsduprio->amsdu_agg_ptail = NULL;
	amsduprio->amsdu_agg_sframes = 0;
	amsduprio->amsdu_agg_bytes = 0;

}

/* create a psudo ether_header,
 *   the len field will be udpated when aggregating more frames
 */
static void *
wlc_amsdu_agg_open(amsdu_info_t *ami, struct ether_addr *ea, void *p)
{
	struct ether_header *eh;
	uint headroom;
	void *pkt;
	osl_t *osh;
	wlc_info_t *wlc = ami->wlc;
	wlc_pkttag_t *pkttag;

	osh = wlc->osh;

	/* allocate enough room once for all cases */
	headroom = TXOFF;


	/* alloc new frame buffer */
	if ((pkt = PKTGET(osh, headroom, TRUE)) == NULL) {
		WL_ERROR(("wl%d: wlc_amsdu_agg_open, PKTGET headroom %d failed\n",
			wlc->pub->unit, headroom));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return NULL;
	}

	ASSERT(ISALIGNED((uintptr)PKTDATA(osh, pkt), sizeof(uint32)));

	/* construct AMSDU frame as
	 * | DA SA LEN | Sub-Frame1 | Sub-Frame2 | ...
	 * its header is not converted to 8023hdr, it will be replaced by dot11 hdr directly
	 * the len is the totlen of the whole aggregated AMSDU, including padding
	 * need special flag for later differentiation
	 */


	/* adjust the data point for correct pkttotlength */
	PKTPULL(osh, pkt, headroom);
	PKTSETLEN(osh, pkt, 0);

	/* init ether_header */
	eh = (struct ether_header*) PKTPUSH(osh, pkt, ETHER_HDR_LEN);

	bcopy((char*)ea, eh->ether_dhost, ETHER_ADDR_LEN);
	bcopy((char*)&wlc->pub->cur_etheraddr, eh->ether_shost, ETHER_ADDR_LEN);
	eh->ether_type = hton16(0);	/* no payload bytes yet */

	/* transfer pkttag, scb, add AMSDU flag */
	/* ??? how many are valid and should be transferred */
	wlc_pkttag_info_move(wlc->pub, p, pkt);
	PKTSETPRIO(pkt, PKTPRIO(p));
	WLPKTTAGSCBSET(pkt, WLPKTTAGSCBGET(p));
	pkttag = WLPKTTAG(pkt);
	ASSERT(!WLPKTFLAG_AMSDU(pkttag));
	pkttag->flags |= WLF_AMSDU;

	return pkt;
}

/* return true on consumed, false others if 
 *      -- first header buffer allocation failed
 *      -- no enough tailroom for pad bytes
 *      -- tot size goes beyond A-MSDU limit
 *
 *  amsdu_agg_p[tid] points to the header lbuf, amsdu_agg_ptail[tid] points to the tail lbuf
 *
 * The A-MSDU format typically will be below
 *   | A-MSDU header(ethernet like) |
 *	|subframe1 8023hdr |
 *		|subframe1 body | pad |
 *			|subframe2 8023hdr |
 *				|subframe2 body | pad |
 *					...
 *						|subframeN 8023hdr |
 *							|subframeN body |
 * It's not required to have pad bytes on the last frame
*/
static bool
wlc_amsdu_agg_append(amsdu_info_t *ami, struct scb *scb, void *p, uint tid)
{
	uint len, totlen, pad;
	osl_t *osh;
	scb_amsdu_t *scb_ami;
	void *ptid;
	struct ether_header *eh;
	amsdu_scb_prio *amsduprio;

	WL_AMSDU(("wlc_amsdu_agg_append\n"));

	osh = ami->pub->osh;
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	amsduprio = &scb_ami->prio[tid];

	/* to apply admission control */

	/* alloc new pack tx buffer if necessary */
	if (amsduprio->amsdu_agg_p == NULL) {

		if ((ptid = wlc_amsdu_agg_open(ami, &scb->ea, p)) == NULL) {
			WLCNTINCR(ami->cnt->agg_openfail);
			return FALSE;
		}

		amsduprio->amsdu_agg_p = ptid;
		amsduprio->amsdu_agg_ptail = ptid;
		amsduprio->amsdu_agg_sframes = 0;
		amsduprio->amsdu_agg_bytes = PKTLEN(osh, ptid);
		amsduprio->amsdu_agg_padlast = 0;

		WL_AMSDU(("wlc_amsdu_agg_append: open a new AMSDU, hdr %d bytes\n",
			amsduprio->amsdu_agg_bytes));
	}

	/* use short name for convenience */
	ptid = amsduprio->amsdu_agg_p;

	len = pkttotlen(osh, p);
	totlen = len + amsduprio->amsdu_agg_bytes;

	/* caller already makes sure this frame fits */
	ASSERT(totlen < amsduprio->amsdu_agg_bytes_max);

	/* chain the pkts at the end of current one, update totlen */
	ASSERT(amsduprio->amsdu_agg_ptail != NULL);
	PKTSETNEXT(osh, amsduprio->amsdu_agg_ptail, p);
	amsduprio->amsdu_agg_ptail = pktlast(osh, p);
	eh = (struct ether_header*) PKTDATA(osh, ptid);
	eh->ether_type = hton16((uint16)(totlen - ETHER_HDR_LEN));

	/* Append any packet callbacks from p to *ptid */
	wlc_pkt_callback_append(ami->wlc, ptid, p);

	/* If padding(for 4 bytes alignment) is needed and feasible(enough tailroom and 
	 * totlen does not exceed limit, then add it, adjust length and continue;
	 * Otherwise, close A-MSDU
	 */
	amsduprio->amsdu_agg_padlast = 0;
	pad = (len % 4) ? (4 - len % 4) : 0;
	if (pad != 0) {
		if (((uint)PKTTAILROOM(osh, amsduprio->amsdu_agg_ptail) >= pad) &&
		    (totlen + pad < amsduprio->amsdu_agg_bytes_max)) {

			amsduprio->amsdu_agg_padlast = pad;
			PKTSETLEN(osh, amsduprio->amsdu_agg_ptail,
				PKTLEN(osh, amsduprio->amsdu_agg_ptail) + pad);
			totlen += pad;
			eh->ether_type = hton16((uint16)(totlen - ETHER_HDR_LEN));

		} else {
			WL_AMSDU(("wlc_amsdu_agg_append: terminate A-MSDU for tailroom/aggmax\n"));

			wlc_amsdu_agg_close(ami, scb, tid);
		}
	}

	WL_AMSDU(("wlc_amsdu_agg_append: add one more frame len %d pad %d\n", len, pad));

	ASSERT(totlen == pkttotlen(osh, ptid));
	amsduprio->amsdu_agg_sframes++;
	amsduprio->amsdu_agg_bytes = totlen;
	return (TRUE);
}

static void
wlc_amsdu_agg_flush(amsdu_info_t *ami)
{
	wlc_info_t *wlc;
	uint i;
	struct scb *scb;
	struct scb_iter scbiter;
	scb_amsdu_t *scb_ami;
	amsdu_scb_prio *amsduprio;

	WL_AMSDU(("wlc_amsdu_agg_flush\n"));

	wlc = ami->wlc;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		for (i = 0; i < NUMPRIO; i++) {
			scb_ami = SCB_AMSDU_CUBBY(ami, scb);
			amsduprio = &scb_ami->prio[i];

			if (amsduprio->amsdu_agg_p)
				PKTFREE(ami->wlc->osh, scb_ami->prio[i].amsdu_agg_p, TRUE);

			amsduprio->amsdu_agg_p = NULL;
			amsduprio->amsdu_agg_ptail = NULL;
			amsduprio->amsdu_agg_sframes = 0;
			amsduprio->amsdu_agg_bytes = 0;
			amsduprio->amsdu_agg_padlast = 0;
		}
	}
}

/* Return the transmit packets held by AMSDU */
static uint
wlc_amsdu_txpktcnt(void *ctx)
{
	amsdu_info_t *ami = (amsdu_info_t *)ctx;
	uint i;
	scb_amsdu_t *scb_ami;
	int pktcnt = 0;
	struct scb_iter scbiter;
	wlc_info_t *wlc = ami->wlc;
	struct scb *scb;

	FOREACHSCB(wlc->scbstate, &scbiter, scb)
		if (SCB_AMSDU(scb)) {
			scb_ami = SCB_AMSDU_CUBBY(ami, scb);
			for (i = 0; i < NUMPRIO; i++) {
				if (scb_ami->prio[i].amsdu_agg_p)
					pktcnt++;
			}
		}

	return pktcnt;
}

static void
wlc_amsdu_scb_deactive(void *ctx, struct scb *scb)
{
	amsdu_info_t *ami;
	uint i;
	scb_amsdu_t *scb_ami;

	WL_AMSDU(("wlc_amsdu_scb_deactive scb %p\n", scb));

	ami = (amsdu_info_t *)ctx;
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	for (i = 0; i < NUMPRIO; i++) {

		if (scb_ami->prio[i].amsdu_agg_p)
			PKTFREE(ami->wlc->osh, scb_ami->prio[i].amsdu_agg_p, TRUE);

		scb_ami->prio[i].amsdu_agg_p = NULL;
		scb_ami->prio[i].amsdu_agg_ptail = NULL;
		scb_ami->prio[i].amsdu_agg_sframes = 0;
		scb_ami->prio[i].amsdu_agg_bytes = 0;
	}
}

#endif /* WLAMSDU_TX */
/* return FALSE if filter failed
 *   caller needs to toss all buffered A-MSDUs and p
 *   Enhancement: in case of out of sequences, try to restart to
 *     deal with lost of last MSDU, which can occur frequently due to fcs error
 */
void
wlc_recvamsdu(amsdu_info_t *ami, wlc_d11rxhdr_t *wrxh, void *p)
{
	osl_t *osh;
	uint aggtype;

	osh = ami->pub->osh;
	aggtype = (wrxh->rxhdr.RxStatus2 & RXS_AGGTYPE_MASK) >> RXS_AGGTYPE_SHIFT;

	WLCNTINCR(ami->cnt->deagg_msdu);


	WL_AMSDU(("wlc_recvamsdu: aggtype %d\n", aggtype));

	switch (aggtype) {
	case RXS_AMSDU_FIRST:
		/* PKTDATA starts with PLCP */
		if (ami->amsdu_deagg_state != WLC_AMSDU_DEAGG_IDLE) {
			WL_AMSDU(("wlc_recvamsdu: wrong A-MSDU deagg sequence, cur_state=%d\n",
				ami->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			wlc_amsdu_deagg_flush(ami);
			/* keep this valid one and reset to improve throughput */
		}

		ami->amsdu_deagg_state = WLC_AMSDU_DEAGG_FIRST;

		if (!wlc_amsdu_deagg_open(ami, p)) {
			wlc_amsdu_deagg_flush(ami);
			goto abort;
		}

		WL_AMSDU(("wlc_recvamsdu: first A-MSDU buffer\n"));
		break;

	case RXS_AMSDU_INTERMEDIATE:
		/* PKTDATA starts with subframe header */
		if (ami->amsdu_deagg_state != WLC_AMSDU_DEAGG_FIRST) {
			WL_ERROR(("wlc_recvamsdu: wrong A-MSDU deagg sequence, cur_state=%d\n",
				ami->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			wlc_amsdu_deagg_flush(ami);
			goto abort;
		}

		/* intermediate frames have no padding bytes due to wlc->hwrxoff = 30 */
		ASSERT(!(wrxh->rxhdr.RxStatus1 & RXS_PBPRES));

		if (PKTLEN(osh, p) <= ETHER_HDR_LEN) {
			WL_ERROR(("wlc_recvamsdu: rxrunt\n"));
			WLCNTINCR(ami->pub->_cnt->rxrunt);
			wlc_amsdu_deagg_flush(ami);
			goto abort;
		}

		ASSERT(ami->amsdu_deagg_ptail);
		PKTSETNEXT(osh, ami->amsdu_deagg_ptail, p);
		ami->amsdu_deagg_ptail = p;
		WL_AMSDU(("wlc_recvamsdu:   mid A-MSDU buffer\n"));
		break;

	case RXS_AMSDU_LAST:
		/* PKTDATA starts with last subframe header */
		if (ami->amsdu_deagg_state != WLC_AMSDU_DEAGG_FIRST) {
			WL_ERROR(("wlc_recvamsdu: wrong A-MSDU deagg sequence, cur_state=%d\n",
				ami->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			wlc_amsdu_deagg_flush(ami);
			goto abort;
		}

		ami->amsdu_deagg_state = WLC_AMSDU_DEAGG_LAST;

		/* last frame have no padding bytes due to wlc->hwrxoff = 30 */
		ASSERT(!(wrxh->rxhdr.RxStatus1 & RXS_PBPRES));

		if (PKTLEN(osh, p) < (ETHER_HDR_LEN + DOT11_FCS_LEN)) {
			WL_ERROR(("wlc_recvamsdu: rxrunt\n"));
			WLCNTINCR(ami->pub->_cnt->rxrunt);
			wlc_amsdu_deagg_flush(ami);
			goto abort;
		}

		ASSERT(ami->amsdu_deagg_ptail);
		PKTSETNEXT(osh, ami->amsdu_deagg_ptail, p);
		ami->amsdu_deagg_ptail = p;
		WL_AMSDU(("wlc_recvamsdu: last A-MSDU buffer\n"));
		break;


	case RXS_AMSDU_N_ONE:
		/* this frame IS AMSDU, checked by caller */

		if (ami->amsdu_deagg_state != WLC_AMSDU_DEAGG_IDLE) {
			WL_AMSDU(("wlc_recvamsdu: wrong A-MSDU deagg sequence, cur_state=%d\n",
				ami->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			wlc_amsdu_deagg_flush(ami);

			/* keep this valid one and reset to improve throughput */
		}

		ASSERT((ami->amsdu_deagg_p == NULL) && (ami->amsdu_deagg_ptail == NULL));
		ami->amsdu_deagg_state = WLC_AMSDU_DEAGG_LAST;

		if (!wlc_amsdu_deagg_open(ami, p)) {
			wlc_amsdu_deagg_flush(ami);
			goto abort;
		}

		break;

	default:
		/* can't be here */
		ASSERT(0);
		wlc_amsdu_deagg_flush(ami);
		goto abort;
	}

	WL_AMSDU(("wlc_recvamsdu: add one more A-MSDU buffer %d bytes, accumulated %d bytes\n",
		PKTLEN(osh, p), pkttotlen(osh, ami->amsdu_deagg_p)));

	if (ami->amsdu_deagg_state == WLC_AMSDU_DEAGG_LAST) {
		void *pp;
		pp = ami->amsdu_deagg_p;
		ami->amsdu_deagg_p = ami->amsdu_deagg_ptail = NULL;
		ami->amsdu_deagg_state = WLC_AMSDU_DEAGG_IDLE;

		/* ucode/hw deagg happened */
		WLPKTTAG(pp)->flags |= WLF_HWAMSDU;

		/* First frame has fully defined Receive Frame Header, 
		 * handle it to normal MPDU process.
		 */
		WLCNTINCR(ami->pub->_cnt->rxfrag);
		WLCNTINCR(ami->cnt->deagg_amsdu);
		wlc_recvdata(ami->wlc, ami->pub->osh, wrxh, pp);
	}

	/* all other cases needs no more action, just return */
	return;

abort:
	PKTFREE(osh, p, FALSE);
}


/* return FALSE if A-MSDU verification failed */
static bool
wlc_amsdu_deagg_verify(amsdu_info_t *ami, uint16 fc, void *h)
{
	bool is_wds;
	uint16 *pqos;
	uint16 qoscontrol;

	/* it doesn't make sense to aggregate other type pkts, toss them */
	if ((fc & FC_KIND_MASK) != FC_QOS_DATA) {
		WL_AMSDU(("wlc_amsdu_deagg_verify fail: fc 0x%x is not QoS data type\n", fc));
		return FALSE;
	}

	is_wds = ((fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
	pqos = (uint16*)((uchar*)h + (is_wds ? DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN));
	qoscontrol = ltoh16_ua(pqos);

	if (qoscontrol & QOS_AMSDU_MASK)
		return TRUE;

	WL_ERROR(("wlc_amsdu_deagg_verify fail: qos field 0x%x\n", *pqos));
	return FALSE;
}

static bool
wlc_amsdu_deagg_open(amsdu_info_t *ami, void *p)
{
	osl_t *osh;
	struct dot11_header *h;
	uint16 fc;

	osh = ami->pub->osh;

	if (PKTLEN(osh, p) < D11_PHY_HDR_LEN + DOT11_MAC_HDR_LEN + DOT11_QOS_LEN + ETHER_HDR_LEN) {
		WL_ERROR(("wlc_recvamsdu: rxrunt\n"));
		WLCNTINCR(ami->pub->_cnt->rxrunt);
		goto fail;
	}

	h = (struct dot11_header *)(PKTDATA(osh, p) + D11_PHY_HDR_LEN);
	fc = ltoh16(h->fc);

	if (!wlc_amsdu_deagg_verify(ami, fc, h)) {
		WL_ERROR(("wlc_recvamsdu: AMSDU verification failed, toss\n"));
		WLCNTINCR(ami->cnt->deagg_badfmt);
		goto fail;
	}

	/* explicitly test bad src address to avoid sending bad deauth */
	if ((ETHER_ISNULLADDR(&h->a2) || ETHER_ISMULTI(&h->a2))) {
		WL_ERROR(("wlc_recvamsdu: wrong address 2\n"));
		WLCNTINCR(ami->pub->_cnt->rxbadsrcmac);
		goto fail;
	}

	ami->amsdu_deagg_p = p;
	ami->amsdu_deagg_ptail = p;
	return TRUE;

fail:
	WLCNTINCR(ami->cnt->deagg_openfail);
	return FALSE;
}

static void
wlc_amsdu_deagg_flush(amsdu_info_t *ami)
{
	WL_AMSDU(("wlc_amsdu_deagg_flush\n"));

	if (ami->amsdu_deagg_p)
		PKTFREE(ami->pub->osh, ami->amsdu_deagg_p, FALSE);

	ami->amsdu_deagg_state = WLC_AMSDU_DEAGG_IDLE;
	ami->amsdu_deagg_p = ami->amsdu_deagg_ptail = NULL;
}

/*
 * A-MSDU decomposition: break A-MSDU(chained buffer) to individual buffers
 *
 *    | 80211 MAC HEADER | subFrame 1 |
 *			               --> | subFrame 2 |
 *			                                 --> | subFrame 3... |
 * where, 80211 MAC header also includes QOS and/or IV fields
 *        f->pbody points to beginning of subFrame 1,
 *        f->totlen is the total body len(chained, after mac/qos/iv header) w/o icv and FCS
 *
 *        each subframe is in the form of | 8023hdr | body | pad |
 *                subframe other than the last one may have pad bytes
*/
void
wlc_amsdu_deagg_hw(amsdu_info_t *ami, struct scb *scb, struct wlc_frminfo *f,
	char *prx_ctxt, int len_rx_ctxt)
{
	osl_t *osh;
	void *sf[MAX_RX_SUBFRAMES], *newpkt;
	struct ether_header *eh;
	uint16 body_offset, sflen = 0, len = 0;
	uint num_sf = 0, i;
	int resid;

	ASSERT(WLPKTTAG(f->p)->flags & WLF_HWAMSDU);
	osh = ami->pub->osh;

	/* strip mac header, move to start from A-MSDU body */
	body_offset = (uint)(f->pbody - (uchar*)PKTDATA(osh, f->p));
	PKTPULL(osh, f->p, body_offset);

	WL_AMSDU(("wlc_amsdu_deagg_hw: body_len(exclude icv and FCS) %d\n", f->totlen));

	resid = f->totlen;
	newpkt = f->p;

	/* break chained AMSDU into N independent MSDU */
	while (newpkt != NULL) {
		/* there must be a limit to stop in order to prevent memory/stack overflow */
		if (num_sf >= MAX_RX_SUBFRAMES) {
			WL_ERROR(("wlc_amsdu_deagg_hw: more than %d MSDUs !\n", num_sf));
			break;
		}

		/* each subframe is 802.3 frame */
		eh = (struct ether_header*) PKTDATA(osh, newpkt);
		len = (uint16)PKTLEN(osh, newpkt);

		sflen = ntoh16(eh->ether_type) + ETHER_HDR_LEN;

		if ((((uintptr)eh + (uint)ETHER_HDR_LEN) % 4)  != 0) {
			WL_ERROR(("wlc_amsdu_deagg_hw: sf body is not 4 bytes aligned!\n"));
			WLCNTINCR(ami->cnt->deagg_badsfalign);
			goto toss;
		}

		/* last MSDU: has FCS, but no pad, other MSDU: has pad, but no FCS */
		if (len != (PKTNEXT(osh, newpkt) ? ROUNDUP(sflen, 4) : sflen)) {
			WL_ERROR(("wlc_amsdu_deagg_hw: len mismatch buflen %d sflen %d, sf %d\n",
				len, sflen, num_sf));
			WLCNTINCR(ami->cnt->deagg_badsflen);
			goto toss;
		}

		/* strip trailing optional pad */
		PKTSETLEN(osh, newpkt, sflen);

		/* convert 8023hdr to ethernet if necessary */
		if (!WLEXTSTA_ENAB(ami->wlc->pub))
			wlc_8023_etherhdr(ami->wlc, osh, newpkt);

		/* propogate prio, NO need to transfer other tags, it's plain stack packet now */
		PKTSETPRIO(newpkt, f->prio);

		WL_AMSDU(("wlc_amsdu_deagg_hw: deagg MSDU buffer %d, frame %d\n", len, sflen));

		sf[num_sf] = newpkt;
		num_sf++;
		newpkt = PKTNEXT(osh, newpkt);

		resid -= len;
	}

	if (resid != 0) {
		ASSERT(0);
		WLCNTINCR(ami->cnt->deagg_badtotlen);
		goto toss;
	}

	/* cut the chain: set PKTNEXT to NULL */
	for (i = 0; i < num_sf; i++)
		PKTSETNEXT(osh, sf[i], NULL);

	/* toss the remaining MSDU, which we couldn't handle */
	if (newpkt != NULL) {
		WL_ERROR(("wlc_amsdu_deagg_hw: toss MSDUs > %d !\n", num_sf));
		PKTFREE(osh, newpkt, FALSE);
	}

	/* sendup */
	for (i = 0; i < num_sf; i++) {
		eh = (struct ether_header *) PKTDATA(osh, sf[i]);
		WL_AMSDU(("wlc_amsdu_deagg_hw: sendup subframe %d\n", i));
		wlc_recvdata_sendup(ami->wlc, scb, f->wds, (struct ether_addr *)eh->ether_dhost,
			sf[i]);
	}


	WL_AMSDU(("wlc_amsdu_deagg_hw: this A-MSDU has %d MSDU, done\n", num_sf));

	return;

toss:
	/* 
	 * toss the whole A-MSDU since we don't know where the error starts
	 *  e.g. a wrong subframe length for mid frame can slip through the ucode
	 *       and the only syptom may be the last MSDU frame has the mismatched length.
	 */
	for (i = 0; i < num_sf; i++)
		sf[i] = NULL;
	PKTFREE(osh, f->p, FALSE);
}

#ifdef WLAMSDU_SWDEAGG
/* A-MSDU sw deaggragation - for testing only due to lower performance to align payload.
 *
 *    | 80211 MAC HEADER | subFrame 1 | subFrame 2 | subFrame 3 | ... |
 * where, 80211 MAC header also includes WDS and/or QOS and/or IV fields
 *        f->pbody points to beginning of subFrame 1,
 *        f->body_len is the total length of all sub frames, exclude ICV and/or FCS
 *
 *        each subframe is in the form of | 8023hdr | body | pad |
 *                subframe other than the last one may have pad bytes
*/
/* 
 * Note: This headroom calculation comes out to 10 byte.
 * Arithmetically, this amounts to two 4-byte blocks plus
 * 2. 2 bytes are needed anyway to achieve 4-byte alignment.
 */
#define HEADROOM  DOT11_A3_HDR_LEN-ETHER_HDR_LEN
void
wlc_amsdu_deagg_sw(amsdu_info_t *ami, struct scb *scb, struct wlc_frminfo *f,
	char *prx_ctxt, int len_rx_ctxt)
{
	osl_t *osh;
	struct ether_header *eh;
	uchar *data;
	void *newpkt;
	int resid;
	uint16 body_offset, sflen, len;

	osh = ami->pub->osh;

	/* all in one buffer, no chain */
	ASSERT(PKTNEXT(osh, f->p) == NULL);

	/* throw away mac header all together, start from A-MSDU body */
	body_offset = (uint)(f->pbody - (uchar*)PKTDATA(osh, f->p));
	PKTPULL(osh, f->p, body_offset);
	ASSERT(f->pbody == (uchar *)PKTDATA(osh, f->p));
	data = f->pbody;
	resid = f->totlen;

	WL_AMSDU(("wlc_amsdu_deagg_sw: body_len(exclude ICV and FCS) %d\n", resid));

	/* loop over orig unpacking and copying frames out into new packet buffers */
	while (resid > 0) {
		if (resid < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN)
			break;

		/* each subframe is 802.3 frame */
		eh = (struct ether_header*) data;
		sflen = ntoh16(eh->ether_type) + ETHER_HDR_LEN;

		/* swdeagg is mainly for testing, not intended to support big buffer.
		 *  there are also the 2K hard limit for rx buffer we posted.
		 *  We can increase to 4K, but it wastes memory and A-MSDU often goes
		 *  up to 8K. HW deagg is the preferred way to handle large A-MSDU.
		 */
		if (sflen > ETHER_MAX_DATA + DOT11_LLC_SNAP_HDR_LEN + ETHER_HDR_LEN) {
			WL_ERROR(("wlc_amsdu_deagg_sw: unexpected long pkt, toss!"));
			WLCNTINCR(ami->cnt->deagg_swdeagglong);
			goto done;
		}

		/* 
		 * Alloc new rx packet buffer, add headroom bytes to
		 * achieve 4-byte alignment and to allow for changing
		 * the hdr from 802.3 to 802.11 (EXT_STA only)
		 */
		if ((newpkt = PKTGET(osh, sflen + HEADROOM, FALSE)) == NULL) {
			WL_ERROR(("wl: wlc_amsdu_deagg: pktget error\n"));
			WLCNTINCR(ami->pub->_cnt->rxnobuf);
			goto done;
		}
		PKTPULL(osh, newpkt, HEADROOM);
		/* copy next frame into new rx packet buffer, pad bytes are dropped */
		bcopy(data, PKTDATA(osh, newpkt), sflen);
		PKTSETLEN(osh, newpkt, sflen);


		/* convert 8023hdr to ethernet if necessary */
		if (!WLEXTSTA_ENAB(ami->wlc->pub))
			wlc_8023_etherhdr(ami->wlc, osh, newpkt);

		eh = (struct ether_header *) PKTDATA(osh, newpkt);

		/* transfer prio, NO need to transfer other tags, it's plain stack packet now */
		PKTSETPRIO(newpkt, f->prio);
		wlc_recvdata_sendup(ami->wlc, scb, f->wds, (struct ether_addr *)eh->ether_dhost,
			newpkt);

		/* account padding bytes */
		len = ROUNDUP(sflen, 4);

		WL_AMSDU(("wlc_amsdu_deagg_sw: deagg one frame datalen=%d, buflen %d\n",
			sflen, len));

		data += len;
		resid -= len;

		/* last MSDU doesn't have pad, may overcount */
		if (resid < -4) {
			WL_ERROR(("wl: wlc_amsdu_deagg_sw: error: resid %d\n", resid));
			break;
		}
	}

done:
	/* all data are copied, free the original amsdu frame */
	PKTFREE(osh, f->p, FALSE);
}
#endif /* WLAMSDU_SWDEAGG */


#ifdef BCMDBG
static int
wlc_amsdu_dump(amsdu_info_t *ami, struct bcmstrbuf *b)
{
	uint i;

	bcm_bprintf(b, "amsdu_agg_block %d amsdu_rx_mtu %d rcvfifo_limit %d\n",
		ami->amsdu_agg_block, ami->amsdu_rx_mtu, ami->mac_rcvfifo_limit);
	bcm_bprintf(b, "amsdu_rxcap_big %d fifo_lowm %d fifo_hiwm %d\n",
		ami->amsdu_rxcap_big, ami->fifo_lowm, ami->fifo_hiwm);

	bcm_bprintf(b, "amsdu_deagg_state %d\n", ami->amsdu_deagg_state);

	for (i = 0; i < NUMPRIO; i++) {
		bcm_bprintf(b, "%d agg_allowprio %d agg_bytes_limit %d agg_sf_limit %d\n",
			i, ami->amsdu_agg_allowprio[i], ami->amsdu_agg_bytes_limit[i],
			ami->amsdu_agg_sframes_limit[i]);
	}

#ifdef WLCNT
	wlc_amsdu_dump_cnt(ami, b);
#endif

	return 0;
}

#ifdef WLCNT
void
wlc_amsdu_dump_cnt(amsdu_info_t *ami, struct bcmstrbuf *b)
{
	wlc_amsdu_cnt_t *cnt = ami->cnt;

	bcm_bprintf(b, "agg_openfail %d\n", cnt->agg_openfail);
	bcm_bprintf(b, "agg_passthrough %d\n", cnt->agg_passthrough);
	bcm_bprintf(b, "agg_block %d\n", cnt->agg_openfail);
	bcm_bprintf(b, "agg_amsdu %d\n", cnt->agg_amsdu);
	bcm_bprintf(b, "agg_msdu %d\n", cnt->agg_msdu);
	bcm_bprintf(b, "deagg_msdu %d\n", cnt->deagg_msdu);
	bcm_bprintf(b, "deagg_amsdu %d\n", cnt->deagg_amsdu);
	bcm_bprintf(b, "deagg_badfmt %d\n", cnt->deagg_badfmt);
	bcm_bprintf(b, "deagg_wrongseq %d\n", cnt->deagg_wrongseq);
	bcm_bprintf(b, "deagg_badsflen %d\n", cnt->deagg_badsflen);
	bcm_bprintf(b, "deagg_badsfalign %d\n", cnt->deagg_badsfalign);
	bcm_bprintf(b, "deagg_badtotlen %d\n", cnt->deagg_badtotlen);
	bcm_bprintf(b, "deagg_openfail %d\n", cnt->deagg_openfail);
	bcm_bprintf(b, "deagg_swdeagglong %d\n", cnt->deagg_swdeagglong);
}
#endif	/* WLCNT */
#ifdef WLAMSDU_TX
static void
wlc_amsdu_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	amsdu_info_t *ami = (amsdu_info_t *)ctx;
	scb_amsdu_t *scb_amsdu = SCB_AMSDU_CUBBY(ami, scb);
	amsdu_scb_prio *amsduprio;

	if (!scb_amsdu || !SCB_AMSDU(scb))
		return;

	bcm_bprintf(b, "\n");

	/* add \t to be aligned with other scb stuff */
	bcm_bprintf(b, "\tAMSDU scb best-effort\n");

	amsduprio = &scb_amsdu->prio[PRIO_8021D_BE];

	bcm_bprintf(b, "\tamsdu_agg_sframes %d amsdu_agg_bytes %d amsdu_agg_txpending %d\n",
		amsduprio->amsdu_agg_sframes, amsduprio->amsdu_agg_bytes,
		amsduprio->amsdu_agg_txpending);
	bcm_bprintf(b, "\tamsdu_agg_bytes_max %d amsdu_agg_allowtid %d\n",
		amsduprio->amsdu_agg_bytes_max, amsduprio->amsdu_agg_allowtid);

	bcm_bprintf(b, "\n");
}
#endif /* WLAMSDU_TX */
#endif	/* BCMDBG */
