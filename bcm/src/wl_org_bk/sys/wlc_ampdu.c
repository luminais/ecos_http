/*
 * A-MPDU (with extended Block Ack protocol) source file
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
 * $Id: wlc_ampdu.c,v 1.348.2.126 2011-02-09 06:27:53 Exp $
 */

#ifndef WLAMPDU
#error "WLAMPDU is not defined"
#endif	/* WLAMPDU */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#ifdef WLOVERTHRUSTER
#include <proto/ethernet.h>
#endif
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_scb.h>
#include <wlc_frmutil.h>
#include <wlc_scan.h>
#ifdef AP
#include <wlc_apps.h>
#endif
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#endif
#include <wlc_rate_sel.h>
#include <wl_export.h>
#include <wlc_rm.h>

#ifdef WLLMAC
#include <wllmacctl.h>
#include <wlc_lmac.h>
#endif /* WLLMAC */

#ifdef WLC_HIGH_ONLY
#include <bcm_rpc_tp.h>
#include <wlc_rpctx.h>
#endif

#ifdef WLAMPDU_PRECEDENCE
#define wlc_ampdu_pktq_plen(pq, tid) \
	(pktq_plen(pq, WLC_PRIO_TO_HI_PREC(tid))\
	+ pktq_plen(pq, WLC_PRIO_TO_PREC(tid)))
static bool wlc_ampdu_prec_enq(wlc_info_t *wlc, struct pktq *q, void *pkt, int tid);
static void * wlc_ampdu_pktq_pdeq(struct pktq *pq, int tid);
#ifdef WLOVERTHRUSTER
static void * wlc_ampdu_pktq_pdeq_tail(struct pktq *pq, int tid);
static void * wlc_ampdu_pktq_ppeek_tail(struct pktq *pq, int tid);
#endif /* WLOVERTHRUSTER */
static void
wlc_ampdu_pktq_pflush(osl_t *osh, struct pktq *pq, int tid, bool dir, ifpkt_cb_t fn, int arg);
#else
#define wlc_ampdu_prec_enq(wlc, q, pkt, prec) 	wlc_prec_enq_head(wlc, q, pkt, prec, FALSE)
#define wlc_ampdu_pktq_plen 					pktq_plen
#define wlc_ampdu_pktq_pdeq 					pktq_pdeq
#ifdef WLOVERTHRUSTER
#define wlc_ampdu_pktq_pdeq_tail 				pktq_pdeq_tail
#define wlc_ampdu_pktq_ppeek_tail 				pktq_ppeek_tail
#endif /* WLOVERTHRUSTER */
#define wlc_ampdu_pktq_pflush 					pktq_pflush
#endif /* WLAMPDU_PRECEDENCE */

/* iovar table */
enum {
	IOV_AMPDU,		/* enable/disable ampdu */
	IOV_AMPDU_TID,		/* enable/disable per-tid ampdu */
	IOV_AMPDU_SEND_ADDBA,	/* send addba req to specified scb and tid */
	IOV_AMPDU_SEND_DELBA,	/* send delba req to specified scb and tid */
	IOV_AMPDU_MANUAL_MODE,	/* addba tx to be driven by cmd line */
	IOV_AMPDU_NO_BAR,	/* do not send bars */
	IOV_AMPDU_MPDU,		/* max number of mpdus in an ampdu */
	IOV_AMPDU_DUR,		/* max duration of an ampdu (in msec) */
	IOV_AMPDU_RESP_TIMEOUT_B, /* timeout (ms) for left edge of win move for brcm peer */
	IOV_AMPDU_RESP_TIMEOUT_NB, /* timeout (ms) for left edge of win move for non-brcm peer */
	IOV_AMPDU_RESP_FLUSH,	/* flush the reorder q on timeout */
	IOV_AMPDU_RTS,		/* use rts for ampdu */
	IOV_AMPDU_BA_WSIZE,	/* ampdu ba window size :kept for backward compatibility */
	IOV_AMPDU_TX_BA_WSIZE,	/* ampdu TX ba window size */
	IOV_AMPDU_RX_BA_WSIZE,	/* ampdu RX ba window size */
	IOV_AMPDU_DENSITY,	/* ampdu density */
	IOV_AMPDU_RX_FACTOR,	/* ampdu rcv len */
	IOV_AMPDU_RETRY_LIMIT,	/* ampdu retry limit */
	IOV_AMPDU_RR_RETRY_LIMIT, /* regular rate ampdu retry limit */
	IOV_AMPDU_TX_LOWAT,	/* ampdu tx low wm */
	IOV_AMPDU_HIAGG_MODE,	/* agg mpdus with diff retry cnt */
	IOV_AMPDU_PROBE_MPDU,	/* number of mpdus/ampdu for rate probe frames */
	IOV_AMPDU_TXPKT_WEIGHT,	/* weight of ampdu in the txfifo; helps rate lag */
	IOV_AMPDU_CLEAR_DUMP,	/* clear ampdu counters */
	IOV_AMPDU_FFPLD_RSVD,	/* bytes to reserve in pre-loading info */
	IOV_AMPDU_FFPLD,        /* to display pre-loading info */
	IOV_AMPDU_MAX_TXFUNFL,   /* inverse of acceptable proportion of tx fifo underflows */
	IOV_AMPDU_RETRY_LIMIT_TID,	/* ampdu retry limit per-tid */
	IOV_AMPDU_RR_RETRY_LIMIT_TID, /* regular rate ampdu retry limit per-tid */
	IOV_AMPDU_RX_TID, 	/* RX BA Enable per-tid */
	IOV_AMPDU_MFBR,		/* Use multiple fallback rate */
	IOV_ACTIVATE_TEST,
#ifdef WLOVERTHRUSTER
	IOV_AMPDU_TCP_ACK_RATIO, /* max number of ack to suppress in a row. 0 deactivates feature */
#endif /* WLOVERTHRUSTER */
	IOV_AMPDU_DBG,
	IOV_AMPDUMAC,		/* Use ucode or hw assisted agregation */
	IOV_AMPDU_AGGMODE,	/* agregation mode: HOST or MAC */
	IOV_AMPDU_AGGFIFO,      /* aggfifo depth */
	IOV_FRAMEBURST_OVR,	/* Override framebursting in absense of ampdu */
	IOV_AMPDU_LAST
};

static const bcm_iovar_t ampdu_iovars[] = {
	{"ampdu", IOV_AMPDU, (IOVF_SET_DOWN), IOVT_BOOL, 0},	/* only if down */
	{"ampdu_tid", IOV_AMPDU_TID, (0), IOVT_BUFFER, sizeof(struct ampdu_tid_control)},
	{"ampdu_rx_tid", IOV_AMPDU_RX_TID, (0), IOVT_BUFFER, sizeof(struct ampdu_tid_control)},
	{"ampdu_density", IOV_AMPDU_DENSITY, (0), IOVT_UINT8, 0},
	{"ampdu_rx_factor", IOV_AMPDU_RX_FACTOR, (IOVF_SET_DOWN), IOVT_UINT32, 0},
	{"ampdu_send_addba", IOV_AMPDU_SEND_ADDBA, (0), IOVT_BUFFER, sizeof(struct ampdu_ea_tid)},
	{"ampdu_send_delba", IOV_AMPDU_SEND_DELBA, (0), IOVT_BUFFER, sizeof(struct ampdu_ea_tid)},
	{"ampdu_manual_mode", IOV_AMPDU_MANUAL_MODE, (IOVF_SET_DOWN), IOVT_BOOL, 0},
	{"ampdu_mpdu", IOV_AMPDU_MPDU, (0), IOVT_INT8, 0},
	{"ampdu_ba_wsize", IOV_AMPDU_BA_WSIZE, (0), IOVT_INT8, 0},
#ifdef WLOVERTHRUSTER
	{"ack_ratio", IOV_AMPDU_TCP_ACK_RATIO, (0), IOVT_UINT8, 0},
#endif /* WLOVERTHRUSTER */
#ifdef BCMDBG
	{"ampdu_aggfifo", IOV_AMPDU_AGGFIFO, 0, IOVT_UINT8, 0},
	{"ampdu_ffpld", IOV_AMPDU_FFPLD, (0), IOVT_UINT32, 0},
	{"ampdu_dbg", IOV_AMPDU_DBG, (0), IOVT_UINT32, 0},
#endif

#if defined(BCMDBG) || defined(WLTEST)
	{"ampdu_clear_dump", IOV_AMPDU_CLEAR_DUMP, 0, IOVT_VOID, 0},
#endif  /* defined(BCMDBG) || defined(WLTEST) */
	{"ampdumac", IOV_AMPDUMAC, (0), IOVT_UINT8, 0},
	{"ampdu_aggmode", IOV_AMPDU_AGGMODE, (IOVF_SET_DOWN), IOVT_INT8, 0},	/* only if down */
	{"frameburst_override", IOV_FRAMEBURST_OVR, (0), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

static const char ac_names[4][6] = {"AC_BE", "AC_BK", "AC_VI", "AC_VO"};

#define AMPDU_MAX_SCB_TID	(NUMPRIO)	/* max tid; currently 8; future 16 */
#define AMPDU_MAX_MPDU		32		/* max number of mpdus in an ampdu */
#define AMPDU_NUM_MPDU_LEGACY	16		/* max number of mpdus in an ampdu to a legacy */
#define AMPDU_DEF_PROBE_MPDU	2		/* def number of mpdus in a rate probe ampdu */
#define AMPDU_RX_BA_DEF_WSIZE_8   8              /* max Rx ba window size (in pdu) */
#ifdef DONGLEBUILD
#define AMPDU_TX_BA_MAX_WSIZE	64		/* max Tx ba window size (in pdu) */
#define AMPDU_TX_BA_DEF_WSIZE	64              /* default Tx ba window size (in pdu) */
#define AMPDU_RX_BA_DEF_WSIZE   16              /* max Rx ba window size (in pdu) */
#define AMPDU_RX_BA_MAX_WSIZE   16              /* default Rx ba window size (in pdu) */
#else
#define AMPDU_TX_BA_MAX_WSIZE	128		/* max Tx ba window size (in pdu) */
#define AMPDU_TX_BA_DEF_WSIZE	64              /* default Tx ba window size (in pdu) */
#define AMPDU_RX_BA_DEF_WSIZE   64              /* max Rx ba window size (in pdu) */
#define AMPDU_RX_BA_MAX_WSIZE   64              /* default Rx ba window size (in pdu) */
#endif /* DONGLEBUILD */
#define AMPDU_BA_BITS_IN_BITMAP	64              /* number of bits in bitmap */
#define	AMPDU_MAX_DUR		5		/* max dur of tx ampdu (in msec) */
#define AMPDU_MAX_RETRY_LIMIT	32		/* max tx retry limit */
#define AMPDU_DEF_RETRY_LIMIT	5		/* default tx retry limit */
#define AMPDU_DEF_RR_RETRY_LIMIT	2	/* default tx retry limit at reg rate */
#define AMPDU_DEF_TX_LOWAT	1		/* default low transmit wm */
#ifdef WLLMAC
#define AMPDU_DEF_TXPKT_WEIGHT	3		/* default weight of ampdu in txfifo */
#else
#define AMPDU_DEF_TXPKT_WEIGHT	2		/* default weight of ampdu in txfifo */
#endif
#define AMPDU_DEF_FFPLD_RSVD	2048		/* default ffpld reserved bytes */
#define AMPDU_MIN_FFPLD_RSVD	512		/* minimum ffpld reserved bytes */
#define AMPDU_BAR_RETRY_CNT	50		/* # of bar retries before delba */
#define AMPDU_INI_FREE		10		/* # of inis to be freed on detach */
#define AMPDU_ADDBA_REQ_RETRY_CNT	4	/* # of addbareq retries before delba */
#define AMPDU_INI_OFF_TIMEOUT	60		/* # of sec in off state */
#define	AMPDU_SCB_MAX_RELEASE	32		/* max # of mpdus released at a time */

#define AMPDU_INI_DEAD_TIMEOUT	2		/* # of sec without ini progress */
#define AMPDU_INI_CLEANUP_TIMEOUT	2	/* # of sec in pending off state */

#define AMPDU_RESP_TIMEOUT_B		1000	/* # of ms wo resp prog with brcm peer */
#define AMPDU_RESP_TIMEOUT_NB		200	/* # of ms wo resp prog with non-brcm peer */
#define AMPDU_RESP_TIMEOUT		100	/* timeout interval in msec for resp prog */

#define AMPDU_RESP_NO_BAPOLICY_TIMEOUT	3	/* # of sec rcving ampdu wo bapolicy */

/* internal BA states */
#define	AMPDU_TID_STATE_BA_OFF		0x00	/* block ack OFF for tid */
#define	AMPDU_TID_STATE_BA_ON		0x01	/* block ack ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_ON	0x02	/* block ack pending ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_OFF	0x03	/* block ack pending OFF for tid */

#define NUM_FFPLD_FIFO 4                        /* number of fifo concerned by pre-loading */
#define FFPLD_TX_MAX_UNFL   200                 /* default value of the average number of ampdu
						 * without underflows
						 */
#define FFPLD_MPDU_SIZE 1800                    /* estimate of maximum mpdu size */
#define FFPLD_MAX_MCS 23                        /* we don't deal with mcs 32 */
#define FFPLD_PLD_INCR 1000                     /* increments in bytes */
#define FFPLD_MAX_AMPDU_CNT 5000                /* maximum number of ampdu we
						 * accumulate between resets.
						 */

/* retry BAR in watchdog reason codes */
#define AMPDU_R_BAR_DISABLED		0	/* disabled retry BAR in watchdog */
#define AMPDU_R_BAR_NO_BUFFER		1	/* retry BAR due to out of buffer */
#define AMPDU_R_BAR_CALLBACK_FAIL	2	/* retry BAR due to callback register fail */
#define AMPDU_R_BAR_HALF_RETRY_CNT	3	/* retry BAR due to reach to half
						 * AMPDU_BAR_RETRY_CNT
						 */
#define AMPDU_R_BAR_BLOCKED		4	/* retry BAR due to blocked data fifo */

#define AMPDU_DEF_TCP_ACK_RATIO		4	/* default TCP ACK RATIO */

#ifndef AMPDU_SCB_DRAIN_CNT
#define AMPDU_SCB_DRAIN_CNT	8
#endif

int wl_ampdu_drain_cnt = AMPDU_SCB_DRAIN_CNT;

/* useful macros */
#define NEXT_SEQ(seq) MODINC_POW2((seq), SEQNUM_MAX)
#define NEXT_TX_INDEX(index) MODINC_POW2((index), AMPDU_TX_BA_MAX_WSIZE)
#define TX_SEQ_TO_INDEX(seq) ((seq) % AMPDU_TX_BA_MAX_WSIZE)
#define NEXT_RX_INDEX(index) MODINC_POW2((index), AMPDU_RX_BA_MAX_WSIZE)
#define RX_SEQ_TO_INDEX(seq) ((seq) % AMPDU_RX_BA_MAX_WSIZE)

#define AMPDU_VALIDATE_TID(ampdu, tid, str) \
	if (tid >= AMPDU_MAX_SCB_TID) { \
		WL_ERROR(("wl%d: %s: invalid tid %d\n", ampdu->wlc->pub->unit, str, tid)); \
		WLCNTINCR(ampdu->cnt->rxunexp); \
		return; \
	}

/* max possible overhead per mpdu in the ampdu; 3 is for roundup if needed */
#define AMPDU_MAX_MPDU_OVERHEAD (DOT11_FCS_LEN + DOT11_ICV_AES_LEN + AMPDU_DELIMITER_LEN + 3 \
	+ DOT11_A4_HDR_LEN + DOT11_QOS_LEN + DOT11_IV_MAX_LEN)

#ifdef BCMDBG
uint32 wl_ampdu_dbg = WL_AMPDU_ERR_VAL;
#endif

/* ampdu related stats */
typedef struct wlc_ampdu_cnt {
	/* initiator stat counters */
	uint32 txampdu;		/* ampdus sent */
#ifdef WLCNT
	uint32 txmpdu;		/* mpdus sent */
	uint32 txregmpdu;	/* regular(non-ampdu) mpdus sent */
	union {
		uint32 txs;		/* MAC agg: txstatus received */
		uint32 txretry_mpdu;	/* retransmitted mpdus */
	} u0;
	uint32 txretry_ampdu;	/* retransmitted ampdus */
	uint32 txfifofull;	/* release ampdu due to insufficient tx descriptors */
	uint32 txfbr_mpdu;	/* retransmitted mpdus at fallback rate */
	uint32 txfbr_ampdu;	/* retransmitted ampdus at fallback rate */
	union {
		uint32 txampdu_sgi;	/* ampdus sent with sgi */
		uint32 txmpdu_sgi;	/* ucode agg: mpdus sent with sgi */
	} u1;
	union {
		uint32 txampdu_stbc;	/* ampdus sent with stbc */
		uint32 txmpdu_stbc;	/* ucode agg: mpdus sent with stbc */
	} u2;
	uint32 txampdu_mfbr_stbc; /* ampdus sent at mfbr with stbc */
	uint32 txrel_wm;	/* mpdus released due to lookahead wm */
	uint32 txrel_size;	/* mpdus released due to max ampdu size */
#if WLAMPDU_MAC
	uint32 txrel_agfull;    /* mpdu cannot released because of aggfifo full */
#endif
	uint32 sduretry;	/* sdus retry returned by sendsdu() */
	uint32 sdurejected;	/* sdus rejected by sendsdu() */
	uint32 txdrop;		/* dropped packets */
	uint32 txr0hole;	/* lost packet between scb and sendampdu */
	uint32 txrnhole;	/* lost retried pkt */
	uint32 txrlag;		/* laggard pkt (was considered lost) */
	uint32 txreg_noack;	/* no ack for regular(non-ampdu) mpdus sent */
	uint32 txaddbareq;	/* addba req sent */
	uint32 rxaddbaresp;	/* addba resp recd */
	uint32 txbar;		/* bar sent */
	uint32 rxba;		/* ba recd */
	uint32 noba;            /* ba missing */
	uint32 txstuck;		/* watchdog bailout for stuck state */
	uint32 orphan;		/* orphan pkts where scb/ini has been cleaned */

#ifdef WLAMPDU_MAC
	uint32 epochdelta;	/* How many times epoch has changed */
	uint32 echgr1;          /* epoch change reason -- plcp */
	uint32 echgr2;          /* epoch change reason -- rate_probe */
	uint32 echgr3;          /* epoch change reason -- a-mpdu as regmpdu */
	uint32 echgr4;          /* epoch change reason -- regmpdu */
	uint32 echgr5;          /* epoch change reason -- dest/tid */
	uint32 tx_mrt, tx_fbr;  /* number of MPDU tx at main/fallback rates */
	uint32 txsucc_mrt;      /* number of successful MPDU tx at main rate */
	uint32 txsucc_fbr;      /* number of successful MPDU tx at fallback rate */
	uint32 enq;             /* totally enqueued into aggfifo */
	uint32 cons;            /* totally reported in txstatus */
	uint32 pending;         /* number of entries currently in aggfifo or txsq */
#endif /* WLAMPDU_MAC */
	/* responder side counters */
	uint32 rxampdu;		/* ampdus recd */
	uint32 rxmpdu;		/* mpdus recd in a ampdu */
	uint32 rxht;		/* mpdus recd at ht rate and not in a ampdu */
	uint32 rxlegacy;	/* mpdus recd at legacy rate */
	uint32 rxampdu_sgi;	/* ampdus recd with sgi */
	uint32 rxampdu_stbc; /* ampdus recd with stbc */
	uint32 rxnobapol;	/* mpdus recd without a ba policy */
	uint32 rxholes;		/* missed seq numbers on rx side */
	uint32 rxqed;		/* pdus buffered before sending up */
	uint32 rxdup;		/* duplicate pdus */
	uint32 rxstuck;		/* watchdog bailout for stuck state */
	uint32 rxoow;		/* out of window pdus */
	uint32 rxaddbareq;	/* addba req recd */
	uint32 txaddbaresp;	/* addba resp sent */
	uint32 rxbar;		/* bar recd */
	uint32 txba;		/* ba sent */

	/* general: both initiator and responder */
	uint32 rxunexp;		/* unexpected packets */
	uint32 txdelba;		/* delba sent */
	uint32 rxdelba;		/* delba recd */

	/* PER (per mcs) statistics */
	uint32 txmpdu_cnt[FFPLD_MAX_MCS+1];	/* MPDUs per mcs */
	uint32 txmpdu_succ_cnt[FFPLD_MAX_MCS+1]; /* acked MPDUs per MCS */

#endif /* WLCNT */
} wlc_ampdu_cnt_t;

/* structure to hold tx fifo information and pre-loading state
 * counters specific to tx underflows of ampdus
 * some counters might be redundant with the ones in wlc or ampdu structures.
 * This allows to maintain a specific state independantly of
 * how often and/or when the wlc counters are updated.
 */
typedef struct wlc_fifo_info {
	uint16 ampdu_pld_size;	/* number of bytes to be pre-loaded */
	uint8 mcs2ampdu_table[FFPLD_MAX_MCS+1]; /* per-mcs max # of mpdus in an ampdu */
	uint16 prev_txfunfl;	/* num of underflows last read from the HW macstats counter */
	uint32 accum_txfunfl;	/* num of underflows since we modified pld params */
	uint32 accum_txampdu;	/* num of tx ampdu since we modified pld params  */
	uint32 prev_txampdu;	/* previous reading of tx ampdu */
	uint32 dmaxferrate;	/* estimated dma avg xfer rate in kbits/sec */
} wlc_fifo_info_t;

#ifdef WLAMPDU_MAC
typedef struct ampdumac_info {
#ifdef WLAMPDU_UCODE
	uint16 txfs_addr_strt;
	uint16 txfs_addr_end;
	uint16 txfs_wptr;
	uint16 txfs_rptr;
	uint16 txfs_wptr_addr;
	uint8  txfs_wsize;
#endif
	bool epoch;
	bool change_epoch;
	/* any change of following elements will change epoch */
	struct scb *scb;
	uint8 tid; /* yet to think: do we care about it? */
	uint8 last_plcp0, last_plcp3;
	int in_queue;
	uint8 depth;
} ampdumac_info_t;
#endif /* WLAMPDU_MAC */

#ifdef WLOVERTHRUSTER
typedef struct wlc_tcp_ack_info {
	uint8 tcp_ack_ratio;
	uint32 tcp_ack_total;
	uint32 tcp_ack_dequeued;
	uint8 *prev_ip_header;
	uint8 *prev_tcp_header;
	uint8 prev_is_ack;
	uint32 prev_ack_num;
	uint32 current_dequeued;
} wlc_tcp_ack_info_t;
#endif

/* AMPDU module specific state */
struct ampdu_info {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	int scb_handle;		/* scb cubby handle to retrieve data from scb */
	bool manual_mode;	/* addba tx to be driven by user */
	bool no_bar;		/* do not send bar on failure */
	uint8 ini_enable[AMPDU_MAX_SCB_TID]; /* per-tid initiator enable/disable of ampdu */
	uint8 ba_policy;	/* ba policy; immediate vs delayed */
	uint8 ba_tx_wsize;      /* Tx ba window size (in pdu) */
	uint8 ba_rx_wsize;      /* Rx ba window size (in pdu) */
	uint8 retry_limit;	/* mpdu transmit retry limit */
	uint8 rr_retry_limit;	/* mpdu transmit retry limit at regular rate */
	uint8 retry_limit_tid[AMPDU_MAX_SCB_TID];	/* per-tid mpdu transmit retry limit */
	/* per-tid mpdu transmit retry limit at regular rate */
	uint8 rr_retry_limit_tid[AMPDU_MAX_SCB_TID];
	uint8 mpdu_density;	/* min mpdu spacing (0-7) ==> 2^(x-1)/8 usec */
	int8 max_pdu;		/* max pdus allowed in ampdu */
	uint8 dur;		/* max duration of an ampdu (in msec) */
	uint8 hiagg_mode;	/* agg mpdus with different retry cnt */
	uint8 probe_mpdu;	/* max mpdus allowed in ampdu for probe pkts */
	uint8 txpkt_weight;	/* weight of ampdu in txfifo; reduces rate lag */
	uint8 rx_factor;	/* maximum rx ampdu factor (0-3) ==> 2^(13+x) bytes */
	uint8 delba_timeout;	/* timeout after which to send delba (sec) */
	uint8 tx_rel_lowat;	/* low watermark for num of pkts in transit */
	uint8 ini_free_index;	/* index for next entry in ini_free */
	uint16 resp_timeout_b;	/* timeout (ms) for left edge of win move for brcm peer */
	uint16 resp_timeout_nb;	/* timeout (ms) for left edge of win move for non-brcm peer */
	uint16 resp_cnt;	/* count of resp reorder queues */
	struct wl_timer *resp_timer;	/* timer for resp reorder q flush */
	uint32 ffpld_rsvd;	/* number of bytes to reserve for preload */
	uint32 max_txlen[MCS_TABLE_SIZE][2][2];	/* max size of ampdu per mcs, bw and sgi */
	void *ini_free[AMPDU_INI_FREE];	/* array of ini's to be freed on detach */
#ifdef WLCNT
	wlc_ampdu_cnt_t *cnt;	/* counters/stats */
#endif
	bool mfbr;		/* enable multiple fallback rate */
#ifdef WLAMPDU_MAC
	ampdumac_info_t hagg[AC_COUNT];
#endif
#if defined(BCMDBG) || defined(WLTEST)
	uint32 retry_histogram[AMPDU_MAX_MPDU+1]; /* histogram for retried pkts */
	uint32 end_histogram[AMPDU_MAX_MPDU+1];	/* errs till end of ampdu */
	uint32 mpdu_histogram[AMPDU_MAX_MPDU+1]; /* mpdus per ampdu histogram */
	uint32 supr_reason[8];			/* reason for suppressed err code */
	uint32 txmcs[FFPLD_MAX_MCS+1];		/* mcs of tx pkts */
	uint32 rxmcs[FFPLD_MAX_MCS+1];		/* mcs of rx pkts */

#ifdef WLAMPDU_UCODE
	uint32 schan_depth_histo[AMPDU_MAX_MPDU+1]; /* side channel depth */
#endif /* WLAMPDU_UCODE */

	uint32 txmcssgi[FFPLD_MAX_MCS+1];		/* mcs of tx pkts */
	uint32 rxmcssgi[FFPLD_MAX_MCS+1];		/* mcs of rx pkts */

	uint32 txmcsstbc[FFPLD_MAX_MCS+1];		/* mcs of tx pkts */
	uint32 rxmcsstbc[FFPLD_MAX_MCS+1];		/* mcs of rx pkts */
#endif /* BCMDBG || WLTEST */
	uint32 tx_max_funl;              /* underflows should be kept such that
					  * (tx_max_funfl*underflows) < tx frames
					  */
	wlc_fifo_info_t fifo_tb[NUM_FFPLD_FIFO]; /* table of fifo infos  */
	uint8 rxba_enable[AMPDU_MAX_SCB_TID]; /* per-tid responder enable/disable of ampdu */

#ifdef WLC_HIGH_ONLY
	void *p;
	tx_status_t txs;
	bool waiting_status; /* To help sanity checks */
#endif
	bool txaggr;	  /* tx state of aggregation: ON/OFF */
	bool rxaggr;	  /* rx state of aggregation: ON/OFF */
	int8 txaggr_override;	/* config override to disable ampdu tx */
	int8 rxaggr_override;	/* config override to disable ampdu rx */
#ifdef WLOVERTHRUSTER
	wlc_tcp_ack_info_t tcp_ack_info;
#endif
	uint8  aggfifo_depth;   /* soft capability of AGGFIFO */
	int8 ampdu_aggmode;	/* aggregation mode, HOST or MAC */
	int8 default_pdu;	/* default pdus allowed in ampdu */
	bool	fb_override;		/* override for frame bursting in absense of ba session */
	bool	fb_override_enable; /* configuration to enable/disable ampd_no_frameburst */
};

#define AMPDU_CLEANUPFLAG_RX   (0x1)
#define AMPDU_CLEANUPFLAG_TX   (0x2)

static void wlc_ampdu_cleanup(wlc_info_t *wlc, ampdu_info_t *ampdu, uint flag);


/* structure to store per-tid state for the ampdu initiator */
typedef struct scb_ampdu_tid_ini {
	uint8 ba_state;		/* ampdu ba state */
	uint8 ba_wsize;		/* negotiated ba window size (in pdu) */
	uint8 tx_in_transit;	/* number of pending mpdus in transit in driver */
	uint8 tid;		/* initiator tid for easy lookup */
	uint8 txretry[AMPDU_TX_BA_MAX_WSIZE];	/* tx retry count; indexed by seq modulo */
	uint8 ackpending[AMPDU_TX_BA_MAX_WSIZE/NBBY];	/* bitmap: set if ack is pending */
	uint8 barpending[AMPDU_TX_BA_MAX_WSIZE/NBBY];	/* bitmap: set if bar is pending */
	uint16 start_seq;	/* seqnum of the first unacknowledged packet */
	uint16 max_seq;		/* max unacknowledged seqnum sent */
	uint16 tx_exp_seq;	/* next exp seq in sendampdu */
	uint16 rem_window;	/* remaining ba window (in pdus) that can be txed */
	uint16 bar_ackpending_seq; /* seqnum of bar for which ack is pending */
	uint16 retry_seq[AMPDU_TX_BA_MAX_WSIZE]; /* seq of released retried pkts */
	uint16 retry_head;	/* head of queue ptr for retried pkts */
	uint16 retry_tail;	/* tail of queue ptr for retried pkts */
	uint16 retry_cnt;	/* cnt of retried pkts */
	bool bar_ackpending;	/* true if there is a bar for which ack is pending */
	bool free_me;		/* true if ini is to be freed on receiving ack for bar */
	bool alive;		/* true if making forward progress */
	uint8 retry_bar;	/* reason code if bar to be retried at watchdog */
	uint8 bar_cnt;		/* number of bars sent with no progress */
	uint8 addba_req_cnt;	/* number of addba_req sent with no progress */
	uint8 cleanup_ini_cnt;	/* number of sec waiting in pending off state */
	uint8 dead_cnt;		/* number of sec without any progress */
	uint8 off_cnt;		/* number of sec in off state before next try */
	struct scb *scb;	/* backptr for easy lookup */
} scb_ampdu_tid_ini_t;

/* structure to store per-tid state for the ampdu responder */
typedef struct scb_ampdu_tid_resp {
	uint8 ba_state;		/* ampdu ba state */
	uint8 ba_wsize;		/* negotiated ba window size (in pdu) */
	uint8 queued;		/* number of queued packets */
	uint8 dead_cnt;		/* number of sec without any progress */
	bool alive;		/* true if making forward progress */
	uint16 exp_seq;		/* next expected seq */
	void *rxq[AMPDU_RX_BA_MAX_WSIZE];	/* rx reorder queue */
	void *wrxh[AMPDU_RX_BA_MAX_WSIZE];	/* saved rxh queue */
} scb_ampdu_tid_resp_t;

/* structure to store per-tid state for the ampdu resp when off. statically alloced */
typedef struct scb_ampdu_tid_resp_off {
	bool ampdu_recd;	/* TRUE is ampdu was recd in the 1 sec window */
	uint8 ampdu_cnt;	/* number of secs during which ampdus are recd */
} scb_ampdu_tid_resp_off_t;

/* scb cubby structure. ini and resp are dynamically allocated if needed */
typedef struct scb_ampdu {
	struct scb *scb;		/* back pointer for easy reference */
	uint8 mpdu_density;		/* mpdu density */
	uint8 max_pdu;			/* max pdus allowed in ampdu */
	uint8 release;			/* # of mpdus released at a time */
	uint16 min_len;			/* min mpdu len to support the density */
	uint32 max_rxlen;		/* max ampdu rcv length; 8k, 16k, 32k, 64k */
	struct pktq txq;		/* sdu transmit queue pending aggregation */
	scb_ampdu_tid_ini_t *ini[AMPDU_MAX_SCB_TID];	/* initiator info */
	scb_ampdu_tid_resp_t *resp[AMPDU_MAX_SCB_TID];	/* responder info */
	scb_ampdu_tid_resp_off_t resp_off[AMPDU_MAX_SCB_TID];	/* info when resp is off */
	uint16 min_lens[FFPLD_MAX_MCS+1];	/* min mpdu lens per mcs */
#ifdef BCMDBG
	uint32 txampdu;
	uint32 txmpdu;
	uint32 rxunexp;
	uint32 rxholes;
	uint32 rxstuck;
	uint32 txdrop;
	uint32 txstuck;
	uint32 txaddbareq;
	uint32 txaddbaresp;
	uint32 txrlag;
	uint32 sduretry;
	uint32 sdurejected;
	uint32 txbar;
	uint32 txreg_noack;
	uint32 noba;
	uint32 rxampdu;
	uint32 rxmpdu;
	uint32 rxlegacy;
	uint32 rxdup;
	uint32 rxoow;
	uint32 rxaddbaresp;
	uint32 rxdelba;
	uint32 rxbar;
	uint32 rxba;
#endif /* BCMDBG */
} scb_ampdu_t;

#ifdef BCMDBG
#define AMPDUSCBCNTADD(cnt, upd) ((cnt) += (upd))
#define AMPDUSCBCNTINCR(cnt) ((cnt)++)
#else
#define AMPDUSCBCNTADD(a, b) do { } while (0)
#define AMPDUSCBCNTINCR(a)  do { } while (0)
#endif

struct ampdu_cubby {
	scb_ampdu_t *scb_cubby;
};

#define SCB_AMPDU_INFO(ampdu, scb) (SCB_CUBBY((scb), (ampdu)->scb_handle))
#define SCB_AMPDU_CUBBY(ampdu, scb) (((struct ampdu_cubby *)SCB_AMPDU_INFO(ampdu, scb))->scb_cubby)

/* local prototypes */
static int scb_ampdu_init(void *context, struct scb *scb);
static void scb_ampdu_deinit(void *context, struct scb *scb);
static void scb_ampdu_txflush(void *context, struct scb *scb);
static void scb_ampdu_flush(ampdu_info_t *ampdu, struct scb *scb);
static int wlc_ampdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_ampdu_watchdog(void *hdl);
static int wlc_ampdu_down(void *hdl);
static void wlc_ampdu_init_min_lens(scb_ampdu_t *scb_ampdu);
static void wlc_ffpld_init(ampdu_info_t *ampdu);

static int wlc_ffpld_check_txfunfl(wlc_info_t *wlc, int f);
static void wlc_ffpld_calc_mcs2ampdu_table(ampdu_info_t *ampdu, int f);
#ifdef BCMDBG
static void wlc_ffpld_show(ampdu_info_t *ampdu);
#endif /* BCMDBG */
#ifdef WLOVERTHRUSTER
static void wlc_ampdu_tcp_ack_suppress(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
                                       void *p, uint8 tid);
#endif
static void wlc_ampdu_recv_addba_req(ampdu_info_t *ampdu, struct scb *scb,
	uint8 *body, int body_len);
static void wlc_ampdu_recv_addba_resp(ampdu_info_t *ampdu, struct scb *scb,
	uint8 *body, int body_len);
static void wlc_ampdu_recv_delba(ampdu_info_t *ampdu, struct scb *scb,
	uint8 *body, int body_len);
static void wlc_ampdu_recv_bar(ampdu_info_t *ampdu, struct scb *scb, uint8 *body,
	int body_len);
static void wlc_ampdu_recv_ba(ampdu_info_t *ampdu, struct scb *scb, uint8 *body,
	int body_len);

static void wlc_ampdu_send_delba(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu, uint8 tid,
	uint16 initiator, uint16 reason);
static void wlc_ampdu_send_bar(ampdu_info_t *ampdu, scb_ampdu_tid_ini_t *ini, bool start);

static void wlc_ampdu_agg(void *ctx, struct scb *scb, void *p, uint prec);
static scb_ampdu_tid_ini_t *wlc_ampdu_init_tid_ini(ampdu_info_t *ampdu,
	scb_ampdu_t *scb_ampdu, uint8 tid, bool override);
static void ampdu_cleanup_tid_ini(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
	uint8 tid, bool force);
static void ampdu_cleanup_tid_resp(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
	uint8 tid);
static void ampdu_ba_state_off(scb_ampdu_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);
static bool wlc_ampdu_txeval(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, bool force);
static void wlc_ampdu_release(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, uint16 release);
static void wlc_ampdu_free_chain(ampdu_info_t *ampdu, void *p, tx_status_t *txs, bool txs3_done);

static void wlc_send_bar_complete(wlc_info_t *wlc, uint txstatus, void *arg);
static void wlc_ampdu_update_ie_param(ampdu_info_t *ampdu);
static void ampdu_update_max_txlen(ampdu_info_t *ampdu, uint8 dur);
static void wlc_ampdu_resp_timeout(void *arg);

static void scb_ampdu_update_config(ampdu_info_t *ampdu, struct scb *scb);
static void scb_ampdu_update_config_all(ampdu_info_t *ampdu);

/* fns to be shared between ampdu and delayed ba. AMPDUXXX: move later */
static int wlc_send_addba_req(wlc_info_t *wlc, struct scb *scb, uint8 tid, uint16 wsize,
	uint8 ba_policy, uint8 delba_timeout);
static int wlc_send_addba_resp(wlc_info_t *wlc, struct scb *scb, uint16 status,
	uint8 token, uint16 timeout, uint16 param_set);
static void *wlc_send_bar(wlc_info_t *wlc, struct scb *scb, uint8 tid,
	uint16 start_seq, uint16 cf_policy, bool enq_only, bool *blocked);
static int wlc_send_delba(wlc_info_t *wlc, struct scb *scb, uint8 tid, uint16 initiator,
	uint16 reason);
static void ampdu_create_f(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f,
	void *p, wlc_d11rxhdr_t *wrxh);

#ifdef WLAMPDU_MAC
static int aggfifo_enque(ampdu_info_t *ampdu, int length, int qid);
static uint get_aggfifo_space(ampdu_info_t *ampdu, int qid);
static uint16 wlc_ampdu_calc_maxlen(ampdu_info_t *ampdu, uint8 plcp0, uint plcp3, uint32 txop);
static void wlc_print_ampdu_txstatus(ampdu_info_t *ampdu,
	tx_status_t *pkg1, uint32 s1, uint32 s2, uint32 s3, uint32 s4);
#endif /* WLAMPDU_MAC */

#if defined(DONGLEBUILD)
static void wlc_ampdu_txflowcontrol(wlc_info_t *wlc, scb_ampdu_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini);
#else
#define wlc_ampdu_txflowcontrol(a, b, c)	do {} while (0)
#endif

#if defined(BCMDBG) || defined(WLTEST)
static int wlc_ampdu_dump(ampdu_info_t *ampdu, struct bcmstrbuf *b);
#endif /* BCMDBG || WLTEST */

static void wlc_ampdu_dotxstatus_complete(ampdu_info_t *ampdu, struct scb *scb, void *p,
	tx_status_t *txs, uint32 frmtxstatus, uint32 frmtxstatus2, uint32 s3, uint32 s4);

#ifdef BCMDBG
static void ampdu_dump_ini(scb_ampdu_tid_ini_t *ini);
#else
#define	ampdu_dump_ini(a)
#endif
static uint wlc_ampdu_txpktcnt(void *hdl);

static void wlc_ampdu_activate(void *ctx, struct scb *scb);

static txmod_fns_t ampdu_txmod_fns = {
	wlc_ampdu_agg,
	wlc_ampdu_txpktcnt,
	scb_ampdu_txflush,
	wlc_ampdu_activate
};

static void wlc_ampdu_activate(void *ctx, struct scb *scb)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)ctx;
#ifdef MFP
	if (SCB_AUTHORIZED(scb))
#endif

	wlc_ampdu_agg_state_update(ampdu->wlc, ON, ON);
}

static INLINE uint16
pkt_h_seqnum(wlc_info_t *wlc, void *p)
{
	struct dot11_header *h;
	h = (struct dot11_header *)PKTDATA(wlc->osh, p);
	return (ltoh16(h->seq) >> SEQNUM_SHIFT);
}

static INLINE uint16
pkt_txh_seqnum(wlc_info_t *wlc, void *p)
{
	d11txh_t *txh;
	struct dot11_header *h;
	txh = (d11txh_t *)PKTDATA(wlc->osh, p);
	h = (struct dot11_header *)((uint8*)(txh+1) + D11_PHY_HDR_LEN);
	return (ltoh16(h->seq) >> SEQNUM_SHIFT);
}

static INLINE void
wlc_ampdu_release_ordered(wlc_info_t *wlc, scb_ampdu_t *scb_ampdu, uint8 tid)
{
	void *p;
	struct wlc_frminfo f;
	uint16 indx;
	uint bandunit;
	struct ether_addr ea;
	struct scb *newscb;
	struct scb *scb = scb_ampdu->scb;
	scb_ampdu_tid_resp_t *resp = scb_ampdu->resp[tid];

	if (resp == NULL)
		return;

	for (indx = RX_SEQ_TO_INDEX(resp->exp_seq); (p = resp->rxq[indx]);
		indx = NEXT_RX_INDEX(indx))
	{
		resp->rxq[indx] = NULL;
		resp->queued--;
		ASSERT(resp->exp_seq == pkt_h_seqnum(wlc, p));

		WL_AMPDU_TX(("wl%d: wlc_ampdu_release_ordered: releasing seq 0x%x\n",
			wlc->pub->unit, resp->exp_seq));

		/* create the fields of frminfo f */
		ampdu_create_f(wlc, scb, &f, p, resp->wrxh[indx]);

		resp->alive = TRUE;
		resp->exp_seq = NEXT_SEQ(resp->exp_seq);

		bandunit = scb->bandunit;
		bcopy(&scb->ea, &ea, ETHER_ADDR_LEN);

		wlc_recvdata_ordered(wlc, scb, &f);

		/* validate that the scb is still around and some path in
		 * wlc_recvdata_ordered() did not free it
		 */
		newscb = wlc_scbfindband(wlc, &ea, bandunit);
		if ((newscb == NULL) || (newscb != scb)) {
			WL_ERROR(("wl%d: %s: scb freed; bail out\n",
				wlc->pub->unit, __FUNCTION__));
			return;
		}

		/* Make sure responder was not freed when we gave up the lock in sendup */
		if ((resp = scb_ampdu->resp[tid]) == NULL)
			return;
	}
}

/* release next n pending ordered packets starting from index going over holes */
static INLINE void
wlc_ampdu_release_n_ordered(wlc_info_t *wlc, scb_ampdu_t *scb_ampdu, uint8 tid, uint16 offset)
{
	void *p;
	struct wlc_frminfo f;
	uint16 indx;
	uint bandunit;
	struct ether_addr ea;
	struct scb *newscb;
	struct scb *scb = scb_ampdu->scb;
	scb_ampdu_tid_resp_t *resp = scb_ampdu->resp[tid];

	ASSERT(resp);
	if (resp == NULL)
		return;

	for (; offset; offset--) {
	        indx = RX_SEQ_TO_INDEX(resp->exp_seq);
		if ((p = resp->rxq[indx])) {
			resp->rxq[indx] = NULL;
			resp->queued--;
			ASSERT(resp->exp_seq == pkt_h_seqnum(wlc, p));

			WL_AMPDU_TX(("wl%d: wlc_ampdu_release_n_ordered: releasing seq 0x%x\n",
			             wlc->pub->unit, resp->exp_seq));

			/* set the fields of frminfo f */
			ampdu_create_f(wlc, scb, &f, p, resp->wrxh[indx]);

			bandunit = scb->bandunit;
			bcopy(&scb->ea, &ea, ETHER_ADDR_LEN);

			wlc_recvdata_ordered(wlc, scb, &f);

			/* validate that the scb is still around and some path in
			 * wlc_recvdata_ordered() did not free it
			 */
			newscb = wlc_scbfindband(wlc, &ea, bandunit);
			if ((newscb == NULL) || (newscb != scb)) {
				WL_ERROR(("wl%d: %s: scb freed; bail out\n",
					wlc->pub->unit, __FUNCTION__));
				return;
			}

			/* Make sure responder was not freed when we gave up the lock in sendup */
			if ((resp = scb_ampdu->resp[tid]) == NULL)
				return;

		} else {
			WLCNTINCR(wlc->ampdu->cnt->rxholes);
			AMPDUSCBCNTINCR(scb_ampdu->rxholes);
		}
		resp->alive = TRUE;
		resp->exp_seq = NEXT_SEQ(resp->exp_seq);
	}
}

/* release all pending ordered packets starting from index going over holes */
static INLINE void
wlc_ampdu_release_all_ordered(wlc_info_t *wlc, scb_ampdu_t *scb_ampdu, uint8 tid)
{
	uint16 seq, max_seq, offset, i;
	scb_ampdu_tid_resp_t *resp = scb_ampdu->resp[tid];

	ASSERT(resp);
	if (resp == NULL)
		return;

	for (i = 0, seq = resp->exp_seq, max_seq = resp->exp_seq;
	     i < AMPDU_RX_BA_MAX_WSIZE;
	     i++, seq = NEXT_SEQ(seq)) {
		if (resp->rxq[RX_SEQ_TO_INDEX(seq)])
			max_seq = seq;
	}

	offset = MODSUB_POW2(max_seq, resp->exp_seq, SEQNUM_MAX) + 1;
	wlc_ampdu_release_n_ordered(wlc, scb_ampdu, tid, offset);
}

ampdu_info_t *
BCMATTACHFN(wlc_ampdu_attach)(wlc_info_t *wlc)
{
	ampdu_info_t *ampdu;
	int i;

	/* some code depends on packed structures */
	ASSERT(sizeof(struct dot11_bar) == DOT11_BAR_LEN);
	ASSERT(sizeof(struct dot11_ba) == DOT11_BA_LEN + DOT11_BA_BITMAP_LEN);
	ASSERT(sizeof(struct dot11_ctl_header) == DOT11_CTL_HDR_LEN);
	ASSERT(sizeof(struct dot11_addba_req) == DOT11_ADDBA_REQ_LEN);
	ASSERT(sizeof(struct dot11_addba_resp) == DOT11_ADDBA_RESP_LEN);
	ASSERT(sizeof(struct dot11_delba) == DOT11_DELBA_LEN);
	ASSERT(DOT11_MAXNUMFRAGS == NBITS(uint16));
	ASSERT(ISPOWEROF2(AMPDU_TX_BA_MAX_WSIZE));
	ASSERT(ISPOWEROF2(AMPDU_RX_BA_MAX_WSIZE));
	ASSERT(wlc->pub->tunables->ampdunummpdu2streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu2streams > 0);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams > 0);

	if (!(ampdu = (ampdu_info_t *)MALLOC(wlc->osh, sizeof(ampdu_info_t)))) {
		WL_ERROR(("wl%d: wlc_ampdu_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}
	bzero((char *)ampdu, sizeof(ampdu_info_t));
	ampdu->wlc = wlc;

#ifdef WLCNT
	if (!(ampdu->cnt = (wlc_ampdu_cnt_t *)MALLOC(wlc->osh, sizeof(wlc_ampdu_cnt_t)))) {
		WL_ERROR(("wl%d: wlc_ampdu_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		goto fail;
	}
	bzero((char *)ampdu->cnt, sizeof(wlc_ampdu_cnt_t));
#endif /* WLCNT */

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++)
		ampdu->ini_enable[i] = TRUE;
	/* Disable ampdu for VO by default */
	ampdu->ini_enable[PRIO_8021D_VO] = FALSE;
	ampdu->ini_enable[PRIO_8021D_NC] = FALSE;

#ifndef MACOSX
	/* Disable ampdu for BK by default since not enough fifo space except for MACOS */
	ampdu->ini_enable[PRIO_8021D_NONE] = FALSE;
	ampdu->ini_enable[PRIO_8021D_BK] = FALSE;
#endif /* MACOSX */

	/* Enable aggregation for BK */
	if (D11REV_IS(wlc->pub->corerev, 28) ||
	    D11REV_IS(wlc->pub->corerev, 16) ||
	    D11REV_IS(wlc->pub->corerev, 17) ||
	    D11REV_IS(wlc->pub->corerev, 26) ||
	    (D11REV_IS(wlc->pub->corerev, 29) && WLCISHTPHY(wlc->band))) {
		ampdu->ini_enable[PRIO_8021D_NONE] = TRUE;
		ampdu->ini_enable[PRIO_8021D_BK] = TRUE;
	}

	/* enable rxba_enable on TIDs */
	for (i = 0; i < AMPDU_MAX_SCB_TID; i++)
		ampdu->rxba_enable[i] = TRUE;

	ampdu->ba_policy = DOT11_ADDBA_POLICY_IMMEDIATE;
	ampdu->ba_tx_wsize = AMPDU_TX_BA_DEF_WSIZE;
	ampdu->ba_rx_wsize = AMPDU_RX_BA_DEF_WSIZE;
#ifdef DONGLEBUILD
	ampdu->ba_rx_wsize = AMPDU_RX_BA_DEF_WSIZE_8;
#endif /* DONGLEBUILD */
	ampdu->mpdu_density = AMPDU_DEF_MPDU_DENSITY;
	ampdu->max_pdu = AUTO;
	ampdu->default_pdu = (wlc->stf->txstreams < 3) ? AMPDU_NUM_MPDU_LEGACY : AMPDU_MAX_MPDU;
	ampdu->dur = AMPDU_MAX_DUR;
	ampdu->resp_timeout_b = AMPDU_RESP_TIMEOUT_B;
	ampdu->resp_timeout_nb = AMPDU_RESP_TIMEOUT_NB;
	ampdu->hiagg_mode = FALSE;
	ampdu->rxaggr = ON;
	ampdu->txaggr = ON;
	ampdu->probe_mpdu = AMPDU_DEF_PROBE_MPDU;

	if (AMPDU_MAC_ENAB(ampdu->wlc->pub))
		ampdu->txpkt_weight = 1;
	else
		ampdu->txpkt_weight = AMPDU_DEF_TXPKT_WEIGHT;

	ampdu->txaggr_override = AUTO;
	ampdu->rxaggr_override = AUTO;

	ampdu->ffpld_rsvd = AMPDU_DEF_FFPLD_RSVD;
	/* bump max ampdu rcv size to 64k for all 11n devices except 4321A0 and 4321A1 */
	if (WLCISNPHY(wlc->band) && NREV_LT(wlc->band->phyrev, 2))
		ampdu->rx_factor = AMPDU_RX_FACTOR_32K;
	else
		ampdu->rx_factor = AMPDU_RX_FACTOR_64K;
#ifdef WLC_HIGH_ONLY
	/* Restrict to smaller rcv size for BMAC dongle */
	ampdu->rx_factor = AMPDU_RX_FACTOR_32K;
#endif
	ampdu->retry_limit = AMPDU_DEF_RETRY_LIMIT;
	ampdu->rr_retry_limit = AMPDU_DEF_RR_RETRY_LIMIT;

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		ampdu->retry_limit_tid[i] = ampdu->retry_limit;
		ampdu->rr_retry_limit_tid[i] = ampdu->rr_retry_limit;
	}

	ampdu->delba_timeout = 0; /* AMPDUXXX: not yet supported */
	ampdu->tx_rel_lowat = AMPDU_DEF_TX_LOWAT;

	ampdu_update_max_txlen(ampdu, ampdu->dur);

	wlc->ampdu_rts = TRUE;
	wlc_ampdu_update_ie_param(ampdu);

#ifdef WLOVERTHRUSTER
	if (WLCISHTPHY(wlc->band))
		ampdu->tcp_ack_info.tcp_ack_ratio = AMPDU_DEF_TCP_ACK_RATIO;
#endif

	/* reserve cubby in the scb container */
	ampdu->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct ampdu_cubby),
		scb_ampdu_init, scb_ampdu_deinit, NULL, (void *)ampdu);

	if (ampdu->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, ampdu_iovars, "ampdu",
		ampdu, wlc_ampdu_doiovar, wlc_ampdu_watchdog, NULL, wlc_ampdu_down)) {
		WL_ERROR(("wl%d: ampdu wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	if (!(ampdu->resp_timer = wl_init_timer(wlc->wl, wlc_ampdu_resp_timeout, ampdu, "resp"))) {
		WL_ERROR(("wl%d: ampdu wl_init_timer() failed\n", wlc->pub->unit));
		goto fail;
	}

	ampdu->mfbr = TRUE;

#if defined(BCMDBG) || defined(WLTEST)
	wlc_dump_register(wlc->pub, "ampdu", (dump_fn_t)wlc_ampdu_dump, (void *)ampdu);
#endif /*  defined(BCMDBG) || defined(WLTEST) */
	/* register txmod function */
	wlc_txmod_fn_register(wlc, TXMOD_AMPDU, ampdu, ampdu_txmod_fns);

	ampdu->ampdu_aggmode = AMPDU_AGG_AUTO;
	/* try to set ampdu to the default value */
	wlc_ampdu_set(ampdu, wlc->pub->_ampdu);

	ampdu->tx_max_funl = FFPLD_TX_MAX_UNFL;
	wlc_ffpld_init(ampdu);

	return ampdu;

fail:
#ifdef WLCNT
	if (ampdu->cnt)
		MFREE(wlc->osh, ampdu->cnt, sizeof(wlc_ampdu_cnt_t));
#endif /* WLCNT */
	MFREE(wlc->osh, ampdu, sizeof(ampdu_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_ampdu_detach)(ampdu_info_t *ampdu)
{
	int i;

	if (!ampdu)
		return;

	/* free all ini's which were to be freed on callbacks which were never called */
	for (i = 0; i < AMPDU_INI_FREE; i++) {
		if (ampdu->ini_free[i]) {
			MFREE(ampdu->wlc->osh, ampdu->ini_free[i],
				sizeof(scb_ampdu_tid_ini_t));
		}
	}

	ASSERT(ampdu->resp_cnt == 0);

	if (ampdu->resp_timer) {
		wl_free_timer(ampdu->wlc->wl, ampdu->resp_timer);
		ampdu->resp_timer = NULL;
	}

#ifdef WLCNT
	if (ampdu->cnt)
		MFREE(ampdu->wlc->osh, ampdu->cnt, sizeof(wlc_ampdu_cnt_t));
#endif
	wlc_module_unregister(ampdu->wlc->pub, "ampdu", ampdu);
	MFREE(ampdu->wlc->osh, ampdu, sizeof(ampdu_info_t));
}

/* scb cubby init fn */
static int
scb_ampdu_init(void *context, struct scb *scb)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)context;
	struct ampdu_cubby *cubby_info = (struct ampdu_cubby *)SCB_AMPDU_INFO(ampdu, scb);
	scb_ampdu_t *scb_ampdu;

	if (scb && !SCB_INTERNAL(scb)) {
		scb_ampdu = MALLOC(ampdu->wlc->osh, sizeof(scb_ampdu_t));
		if (!scb_ampdu)
			return BCME_NOMEM;
		bzero(scb_ampdu, sizeof(scb_ampdu_t));
		cubby_info->scb_cubby = scb_ampdu;
		scb_ampdu->scb = scb;

#ifdef WLAMPDU_PRECEDENCE
		pktq_init(&scb_ampdu->txq, WLC_PREC_COUNT, AMPDU_MAX_SCB_TID * PKTQ_LEN_DEFAULT);
#ifdef WLMEDIA_IPTV
		scb_ampdu->txq.max = AMPDU_MAX_SCB_TID * PKTQ_LEN_DEFAULT * 4;
		scb_ampdu->txq.q[WLC_PRIO_TO_HI_PREC(PRIO_8021D_BE)].max = scb_ampdu->txq.max;
		scb_ampdu->txq.q[WLC_PRIO_TO_HI_PREC(PRIO_8021D_EE)].max = scb_ampdu->txq.max;
		scb_ampdu->txq.q[WLC_PRIO_TO_HI_PREC(PRIO_8021D_CL)].max = scb_ampdu->txq.max;
		scb_ampdu->txq.q[WLC_PRIO_TO_HI_PREC(PRIO_8021D_VI)].max = scb_ampdu->txq.max;
#endif /* WLMEDIA_IPTV */
#else
#ifdef WLMEDIA_IPTV
		if (AP_ENAB(ampdu->wlc->pub))
			pktq_init_threshold(&scb_ampdu->txq, AMPDU_MAX_SCB_TID,
				AMPDU_MAX_SCB_TID * PKTQ_LEN_DEFAULT * 4,
				AMPDU_MAX_SCB_TID * PKTQ_LEN_DEFAULT);
		else
			pktq_init(&scb_ampdu->txq, AMPDU_MAX_SCB_TID,
				AMPDU_MAX_SCB_TID * PKTQ_LEN_DEFAULT * 4);
#else
		pktq_init(&scb_ampdu->txq, AMPDU_MAX_SCB_TID,
			AMPDU_MAX_SCB_TID * PKTQ_LEN_DEFAULT);
#endif /* WLMEDIA_IPTV */
#endif /* WLAMPDU_PRECEDENCE */
	}
	return 0;
}

/* scb cubby deinit fn */
static void
scb_ampdu_deinit(void *context, struct scb *scb)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)context;
	struct ampdu_cubby *cubby_info = (struct ampdu_cubby *)SCB_AMPDU_INFO(ampdu, scb);
	scb_ampdu_t *scb_ampdu = NULL;

	WL_AMPDU_UPDN(("scb_ampdu_deinit: enter\n"));

	ASSERT(cubby_info);

	if (cubby_info)
		scb_ampdu = cubby_info->scb_cubby;
	if (!scb_ampdu)
		return;

	scb_ampdu_flush(ampdu, scb);

	MFREE(ampdu->wlc->osh, scb_ampdu, sizeof(scb_ampdu_t));
	cubby_info->scb_cubby = NULL;
}

static void
scb_ampdu_txflush(void *context, struct scb *scb)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)context;

	scb_ampdu_flush(ampdu, scb);
}

static void
scb_ampdu_flush(ampdu_info_t *ampdu, struct scb *scb)
{
	scb_ampdu_t *scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	uint8 tid;

	WL_AMPDU_UPDN(("scb_ampdu_flush: enter\n"));

	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ampdu_cleanup_tid_resp(ampdu, scb_ampdu, tid);
		ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, TRUE);
	}

	/* free all buffered tx packets */
	pktq_flush(ampdu->wlc->osh, &scb_ampdu->txq, TRUE, NULL, 0);

#ifdef WLC_HIGH_ONLY
	ASSERT(ampdu->p == NULL);
#endif
}

void
scb_ampdu_cleanup(ampdu_info_t *ampdu, struct scb *scb)
{
	scb_ampdu_t *scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	uint8 tid;

	WL_AMPDU_UPDN(("scb_ampdu_cleanup: enter\n"));
	ASSERT(scb_ampdu);

	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ampdu_cleanup_tid_resp(ampdu, scb_ampdu, tid);
		ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, FALSE);
	}
}

void
scb_ampdu_cleanup_all(ampdu_info_t *ampdu)
{
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("scb_ampdu_cleanup_all: enter\n"));
	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_flush(ampdu, scb);
	}
}

/* reset the ampdu state machine so that it can gracefully handle packets that were
 * freed from the dma and tx queues during reinit
 */
void
wlc_ampdu_reset(ampdu_info_t *ampdu)
{
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	WL_AMPDU_UPDN(("wlc_ampdu_reset: enter\n"));
	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb))
			continue;
		scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;
			if ((ini->ba_state == AMPDU_TID_STATE_BA_ON) && ini->tx_in_transit)
				ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, TRUE);
		}
	}
}

static void
scb_ampdu_update_config(ampdu_info_t *ampdu, struct scb *scb)
{
	scb_ampdu_t *scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	int i;

	if ((scb->flags & SCB_BRCM) && !(scb->flags2 & SCB2_RX_LARGE_AGG) &&
	    !(scb->rateset.mcs[2] || scb->rateset.mcs[3]))
		scb_ampdu->max_pdu = MIN(ampdu->wlc->pub->tunables->ampdunummpdu2streams,
		                         AMPDU_NUM_MPDU_LEGACY);
	else {
		/* check if scb supports 3 or 4 streams */
		if (scb->rateset.mcs[2] || scb->rateset.mcs[3])
			scb_ampdu->max_pdu = (uint8)ampdu->wlc->pub->tunables->ampdunummpdu3streams;
		else
			scb_ampdu->max_pdu = (uint8)ampdu->wlc->pub->tunables->ampdunummpdu2streams;
	}
	ampdu->default_pdu = scb_ampdu->max_pdu;

	/* go back to legacy size if some preloading is occuring */
	for (i = 0; i < NUM_FFPLD_FIFO; i++) {
		if (ampdu->fifo_tb[i].ampdu_pld_size > FFPLD_PLD_INCR)
			scb_ampdu->max_pdu = ampdu->default_pdu;
	}

	/* apply user override */
	if (ampdu->max_pdu != AUTO)
		scb_ampdu->max_pdu = (uint8)ampdu->max_pdu;

	scb_ampdu->release = MIN(scb_ampdu->max_pdu, AMPDU_SCB_MAX_RELEASE);

	if (scb_ampdu->max_rxlen)
		scb_ampdu->release = MIN(scb_ampdu->release, scb_ampdu->max_rxlen/1600);

	if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
		scb_ampdu->release = MIN(scb_ampdu->release,
			ampdu->fifo_tb[TX_AC_BE_FIFO].mcs2ampdu_table[FFPLD_MAX_MCS]);
	}

	ASSERT(scb_ampdu->release);
}

void
scb_ampdu_update_config_all(ampdu_info_t *ampdu)
{
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("scb_ampdu_update_config_all: enter\n"));
	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_update_config(ampdu, scb);
	}
}

/* Return the transmit packets held by AMPDU */
static uint
wlc_ampdu_txpktcnt(void *hdl)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)hdl;
	struct scb *scb;
	struct scb_iter scbiter;
	int pktcnt = 0;
	scb_ampdu_t *scb_ampdu;

	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
			pktcnt += pktq_len(&scb_ampdu->txq);
		}
	}

	return pktcnt;
}

/* frees all the buffers and cleanup everything on down */
static int
wlc_ampdu_down(void *hdl)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)hdl;
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("wlc_ampdu_down: enter\n"));

	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_flush(ampdu, scb);
	}

	/* we will need to re-run the pld tuning */
	wlc_ffpld_init(ampdu);

	return 0;
}

static void
wlc_ampdu_resp_timeout(void *arg)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)arg;
	wlc_info_t *wlc = ampdu->wlc;
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;
	uint32 lim;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb))
			continue;

		scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			if ((resp = scb_ampdu->resp[tid]) == NULL)
				continue;

			/* check on resp forward progress */

			if (resp->alive) {
				resp->alive = FALSE;
				resp->dead_cnt = 0;
			} else {
				if (!resp->queued)
					continue;

				resp->dead_cnt++;
				lim = (scb->flags & SCB_BRCM) ?
					(ampdu->resp_timeout_b / AMPDU_RESP_TIMEOUT) :
					(ampdu->resp_timeout_nb / AMPDU_RESP_TIMEOUT);

				if (resp->dead_cnt >= lim) {
					WL_ERROR(("wl%d: ampdu_resp_timeout: cleaning up resp"
						" tid %d waiting for seq 0x%x for %d ms\n",
						wlc->pub->unit, tid, resp->exp_seq,
						lim*AMPDU_RESP_TIMEOUT));
					WLCNTINCR(ampdu->cnt->rxstuck);
					AMPDUSCBCNTINCR(scb_ampdu->rxstuck);
					wlc_ampdu_release_all_ordered(wlc, scb_ampdu, tid);
				}
			}
		}
	}
}


static void
wlc_ampdu_cleanup(wlc_info_t *wlc, ampdu_info_t *ampdu, uint flag)
{
	uint8 tid;
	scb_ampdu_t *scb_ampdu = NULL;
	struct scb *scb;
	struct scb_iter scbiter;

	scb_ampdu_tid_ini_t *ini;
	scb_ampdu_tid_resp_t *resp;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb))
			continue;

		if (!SCB_ASSOCIATED(scb))
			continue;

		scb_ampdu = SCB_AMPDU_CUBBY(wlc->ampdu, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			ini = scb_ampdu->ini[tid];
			resp = scb_ampdu->resp[tid];

			if ((flag & AMPDU_CLEANUPFLAG_TX) && (ini != NULL))
			{
				if ((ini->ba_state == AMPDU_TID_STATE_BA_ON) ||
					(ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON))
					wlc_ampdu_send_delba(ampdu, scb_ampdu, tid, TRUE,
						DOT11_RC_TIMEOUT);

				ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, FALSE);
			}

			if ((flag & AMPDU_CLEANUPFLAG_RX) && (resp != NULL)) {
				if ((resp->ba_state == AMPDU_TID_STATE_BA_ON) ||
					(resp->ba_state == AMPDU_TID_STATE_BA_PENDING_ON))
					wlc_ampdu_send_delba(ampdu, scb_ampdu, tid, FALSE,
						DOT11_RC_TIMEOUT);

				ampdu_cleanup_tid_resp(ampdu, scb_ampdu, tid);
			}
		}

		if (flag & AMPDU_CLEANUPFLAG_TX) {
			/* free all buffered tx packets */
			pktq_flush(ampdu->wlc->pub->osh, &scb_ampdu->txq, TRUE, NULL, 0);
		}
	}
}

bool
wlc_ampdu_frameburst_override(ampdu_info_t *ampdu)
{
	if (ampdu == NULL)
		return FALSE;
	else
		return ampdu->fb_override;
}

/* resends ADDBA-Req if the ADDBA-Resp has not come back */
static int
wlc_ampdu_watchdog(void *hdl)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)hdl;
	wlc_info_t *wlc = ampdu->wlc;
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	scb_ampdu_tid_resp_t *resp;
	scb_ampdu_tid_resp_off_t *resp_off;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;
	void *p;
	bool any_ba_state_on = FALSE;
	bool any_ht_cap = FALSE;


	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_HT_CAP(scb))any_ht_cap = TRUE;

		if (!SCB_AMPDU(scb))
			continue;
		scb_ampdu = SCB_AMPDU_CUBBY(wlc->ampdu, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			resp = scb_ampdu->resp[tid];
			resp_off = &scb_ampdu->resp_off[tid];
			if (resp) {
				resp_off->ampdu_cnt = 0;
				resp_off->ampdu_recd = FALSE;
			}
			if (resp_off->ampdu_recd) {
				resp_off->ampdu_recd = FALSE;
				resp_off->ampdu_cnt++;
				if (resp_off->ampdu_cnt >= AMPDU_RESP_NO_BAPOLICY_TIMEOUT) {
					resp_off->ampdu_cnt = 0;
					WL_ERROR(("wl%d: ampdu_watchdog: ampdus recd for"
						" tid %d with no BA policy in effect\n",
						wlc->pub->unit, tid));
					wlc_ampdu_send_delba(ampdu, scb_ampdu, tid,
						FALSE, DOT11_RC_SETUP_NEEDED);
				}
			}

			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;

			switch (ini->ba_state) {
			case AMPDU_TID_STATE_BA_ON:
				/* tickle the sm and release whatever pkts we can */
				wlc_ampdu_txeval(ampdu, scb_ampdu, ini, TRUE);

				any_ba_state_on = TRUE;

				if (ini->retry_bar) {
					/* Free queued packets when system is short of memory;
					 * release 1/4 buffered and at most wl_ampdu_drain_cnt pkts.
					 */
					if (ini->retry_bar == AMPDU_R_BAR_NO_BUFFER) {
						int rel_cnt, i;

						WL_ERROR(("wl%d: wlc_ampdu_watchdog: "
							"no memory\n", wlc->pub->unit));
						rel_cnt = wlc_ampdu_pktq_plen(&scb_ampdu->txq,
							ini->tid) >> 2;
						if (wl_ampdu_drain_cnt <= 0)
							wl_ampdu_drain_cnt = AMPDU_SCB_DRAIN_CNT;
						rel_cnt = MIN(rel_cnt, wl_ampdu_drain_cnt);
						for (i = 0; i < rel_cnt; i++) {
							p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq,
								ini->tid);
							ASSERT(PKTPRIO(p) == ini->tid);
							PKTFREE(wlc->pub->osh, p, TRUE);
							AMPDUSCBCNTINCR(scb_ampdu->txdrop);
							WLCNTINCR(ampdu->cnt->txdrop);
						}
					}

					ini->retry_bar = AMPDU_R_BAR_DISABLED;
					wlc_ampdu_send_bar(ampdu, ini, FALSE);
				}
				else if (ini->bar_cnt >= AMPDU_BAR_RETRY_CNT) {
					wlc_ampdu_send_delba(ampdu, scb_ampdu, tid,
						TRUE, DOT11_RC_TIMEOUT);
				}
				/* check on forward progress */
				else if (ini->alive) {
					ini->alive = FALSE;
					ini->dead_cnt = 0;
				} else {
					if (ini->tx_in_transit)
						ini->dead_cnt++;
					if (ini->dead_cnt == AMPDU_INI_DEAD_TIMEOUT) {
						WL_ERROR(("wl%d: ampdu_watchdog: cleaning up ini"
							" tid %d due to no progress for %d secs\n",
							wlc->pub->unit, tid, ini->dead_cnt));
						WLCNTINCR(ampdu->cnt->txstuck);
						AMPDUSCBCNTINCR(scb_ampdu->txstuck);
						/* AMPDUXXX: unknown failure, send delba */
						wlc_ampdu_send_delba(ampdu, scb_ampdu, tid,
							TRUE, DOT11_RC_TIMEOUT);
						ampdu_dump_ini(ini);
						ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, TRUE);
					}
				}
				break;

			case AMPDU_TID_STATE_BA_PENDING_ON:
				/* resend addba if not enough retries */
				if (ini->addba_req_cnt++ >= AMPDU_ADDBA_REQ_RETRY_CNT) {
					ampdu_ba_state_off(scb_ampdu, ini);
				} else {
#ifdef MFP
					if (SCB_AUTHORIZED(scb))
#endif
					wlc_send_addba_req(ampdu->wlc, scb, tid, ampdu->ba_tx_wsize,
						ampdu->ba_policy, ampdu->delba_timeout);
					WLCNTINCR(ampdu->cnt->txaddbareq);
					AMPDUSCBCNTINCR(scb_ampdu->txaddbareq);
				}
				break;

			case AMPDU_TID_STATE_BA_PENDING_OFF:
				if (ini->cleanup_ini_cnt++ >= AMPDU_INI_CLEANUP_TIMEOUT) {
					WL_ERROR(("wl%d: ampdu_watchdog: cleaning up tid %d "
						"from poff\n", wlc->pub->unit, tid));
					WLCNTINCR(ampdu->cnt->txstuck);
					AMPDUSCBCNTINCR(scb_ampdu->txstuck);
					ampdu_dump_ini(ini);
					ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, TRUE);
				} else
					ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, FALSE);
				break;

			case AMPDU_TID_STATE_BA_OFF:
				if (ini->off_cnt++ >= AMPDU_INI_OFF_TIMEOUT) {
					ini = wlc_ampdu_init_tid_ini(ampdu, scb_ampdu,
						tid, FALSE);
					/* make a single attempt only */
					if (ini)
						ini->addba_req_cnt = AMPDU_ADDBA_REQ_RETRY_CNT;
				}
				break;
			default:
				break;
			}
			/* dont do anything with ini here since it might be freed */

		}
	}

	if (ampdu->fb_override_enable && any_ht_cap && (!any_ba_state_on)) {
		ampdu->fb_override = TRUE;
	} else {
		ampdu->fb_override = FALSE;
	}

	return 0;
}

/* handle AMPDU related iovars */
static int
wlc_ampdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)hdl;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *) a;
	bool bool_val;
	int err = 0;
	wlc_info_t *wlc;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	wlc = ampdu->wlc;
	ASSERT(ampdu == wlc->ampdu);

	switch (actionid) {
	case IOV_GVAL(IOV_AMPDU):
		*ret_int_ptr = (int32)wlc->pub->_ampdu;
		break;

	case IOV_SVAL(IOV_AMPDU):
		return wlc_ampdu_set(ampdu, (bool)int_val);

	case IOV_GVAL(IOV_AMPDU_TID):
	case IOV_GVAL(IOV_AMPDU_RX_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)p;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		if (actionid == IOV_GVAL(IOV_AMPDU_TID))
			ampdu_tid->enable = ampdu->ini_enable[ampdu_tid->tid];
		else
			ampdu_tid->enable = ampdu->rxba_enable[ampdu_tid->tid];
		bcopy(ampdu_tid, a, sizeof(*ampdu_tid));
		break;
		}

	case IOV_SVAL(IOV_AMPDU_TID):
	case IOV_SVAL(IOV_AMPDU_RX_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)a;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		if (actionid == IOV_SVAL(IOV_AMPDU_TID))
			ampdu->ini_enable[ampdu_tid->tid] = ampdu_tid->enable ? TRUE : FALSE;
		else
			ampdu->rxba_enable[ampdu_tid->tid] = ampdu_tid->enable ? TRUE : FALSE;
		break;
		}

	case IOV_GVAL(IOV_AMPDU_DENSITY):
		*ret_int_ptr = (int32)ampdu->mpdu_density;
		break;

	case IOV_SVAL(IOV_AMPDU_DENSITY):
		if (int_val > AMPDU_MAX_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}
		if (int_val < AMPDU_DEF_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}
		ampdu->mpdu_density = (uint8)int_val;
		wlc_ampdu_update_ie_param(ampdu);
		break;

	case IOV_GVAL(IOV_AMPDU_RX_FACTOR):
		*ret_int_ptr = (int32)ampdu->rx_factor;
		break;

	case IOV_SVAL(IOV_AMPDU_RX_FACTOR):
		/* limit to the max aggregation size possible based on chip
		 * limitations
		 */
		if ((int_val > AMPDU_RX_FACTOR_64K) ||
		    (int_val > AMPDU_RX_FACTOR_32K &&
		     D11REV_LE(wlc->pub->corerev, 11))) {
			err = BCME_RANGE;
			break;
		}
		ampdu->rx_factor = (uint8)int_val;
		wlc_ampdu_update_ie_param(ampdu);
		break;

	case IOV_SVAL(IOV_AMPDU_SEND_ADDBA):
	{
		struct ampdu_ea_tid *ea_tid = (struct ampdu_ea_tid *)a;
		struct scb *scb;
		scb_ampdu_t *scb_ampdu;

		if (!AMPDU_ENAB(ampdu->wlc->pub) || (ea_tid->tid >= AMPDU_MAX_SCB_TID)) {
			err = BCME_BADARG;
			break;
		}

		if (!(scb = wlc_scbfind(wlc, &ea_tid->ea))) {
			err = BCME_NOTFOUND;
			break;
		}

		scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
		if (!scb_ampdu || !SCB_AMPDU(scb)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (scb_ampdu->ini[ea_tid->tid]) {
			err = BCME_NOTREADY;
			break;
		}

		if (!wlc_ampdu_init_tid_ini(ampdu, scb_ampdu, ea_tid->tid, TRUE)) {
			err = BCME_ERROR;
			break;
		}

		break;
	}

	case IOV_SVAL(IOV_AMPDU_SEND_DELBA):
	{
		struct ampdu_ea_tid *ea_tid = (struct ampdu_ea_tid *)a;
		struct scb *scb;
		scb_ampdu_t *scb_ampdu;

		if (!AMPDU_ENAB(ampdu->wlc->pub) || (ea_tid->tid >= AMPDU_MAX_SCB_TID)) {
			err = BCME_BADARG;
			break;
		}

		if (!(scb = wlc_scbfind(wlc, &ea_tid->ea))) {
			err = BCME_NOTFOUND;
			break;
		}

		scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
		if (!scb_ampdu || !SCB_AMPDU(scb)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		wlc_ampdu_send_delba(ampdu, scb_ampdu, ea_tid->tid, TRUE,
			DOT11_RC_BAD_MECHANISM);

		break;
	}

	case IOV_GVAL(IOV_AMPDU_MANUAL_MODE):
		*ret_int_ptr = (int32)ampdu->manual_mode;
		break;

	case IOV_SVAL(IOV_AMPDU_MANUAL_MODE):
		ampdu->manual_mode = (uint8)int_val;
		break;

#ifdef WLOVERTHRUSTER
	case IOV_SVAL(IOV_AMPDU_TCP_ACK_RATIO):
		ampdu->tcp_ack_info.tcp_ack_ratio = (uint8)int_val;
		break;
	case IOV_GVAL(IOV_AMPDU_TCP_ACK_RATIO):
		*ret_int_ptr = (uint32)ampdu->tcp_ack_info.tcp_ack_ratio;
		break;
#endif

	case IOV_GVAL(IOV_AMPDU_MPDU):
		*ret_int_ptr = (int32)ampdu->max_pdu;
		break;

	case IOV_SVAL(IOV_AMPDU_MPDU):
		if (int_val > AMPDU_MAX_MPDU) {
			err = BCME_RANGE;
			break;
		}
		ampdu->max_pdu = (int8)int_val;
		scb_ampdu_update_config_all(ampdu);
		break;

	case IOV_GVAL(IOV_AMPDU_BA_WSIZE):
		*ret_int_ptr = (int32)ampdu->ba_tx_wsize;
		break;
	case IOV_SVAL(IOV_AMPDU_BA_WSIZE):
		if ((int_val == 0) || (int_val > AMPDU_TX_BA_MAX_WSIZE) ||
		 (int_val > AMPDU_RX_BA_MAX_WSIZE)) {
			err = BCME_BADARG;
			break;
		}
		ampdu->ba_tx_wsize = (uint8)int_val;
		ampdu->ba_rx_wsize = (uint8)int_val;
		break;

#ifdef BCMDBG
#ifdef WLC_LOW
	case IOV_GVAL(IOV_AMPDU_FFPLD):
		wlc_ffpld_show(ampdu);
		*ret_int_ptr = 0;
		break;
#endif
#endif /* BCMDBG */

#if defined(BCMDBG) || defined(WLTEST) || defined(WLPKTDLYSTAT)
#ifdef WLCNT
	case IOV_SVAL(IOV_AMPDU_CLEAR_DUMP):
	{

		/* zero the counters */
		bzero(ampdu->cnt, sizeof(wlc_ampdu_cnt_t));
		/* reset the histogram as well */
		bzero(ampdu->retry_histogram, sizeof(ampdu->retry_histogram));
		bzero(ampdu->end_histogram, sizeof(ampdu->end_histogram));
		bzero(ampdu->mpdu_histogram, sizeof(ampdu->mpdu_histogram));
		bzero(ampdu->supr_reason, sizeof(ampdu->supr_reason));
		bzero(ampdu->txmcs, sizeof(ampdu->txmcs));
		bzero(ampdu->rxmcs, sizeof(ampdu->rxmcs));
		bzero(ampdu->txmcssgi, sizeof(ampdu->txmcssgi));
		bzero(ampdu->rxmcssgi, sizeof(ampdu->rxmcssgi));
		bzero(ampdu->txmcsstbc, sizeof(ampdu->txmcsstbc));
		bzero(ampdu->rxmcsstbc, sizeof(ampdu->rxmcsstbc));
#ifdef WLOVERTHRUSTER
		ampdu->tcp_ack_info.tcp_ack_total = 0;
		ampdu->tcp_ack_info.tcp_ack_dequeued = 0;
#endif /* WLOVERTHRUSTER */
#ifdef WLPKTDLYSTAT
	{
		struct scb *scb;
		struct scb_iter scbiter;
		/* reset the per-SCB delay statistics */
		FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
			bzero(scb->delay_stats, sizeof(scb->delay_stats));
		}
	}
#endif

	if (D11REV_IS(ampdu->wlc->pub->corerev, 26) ||
	    D11REV_IS(ampdu->wlc->pub->corerev, 29)) {
		int i;
		uint16 stat_addr = 2 * wlc_read_shm(ampdu->wlc, M_HWAGG_STATS_PTR);
		for (i = 0; i < C_AGGBURST_STATS_WSZ; i++)
			wlc_write_shm(ampdu->wlc, stat_addr + i * 2, 0);
	}
#ifdef WLAMPDU_MAC
#ifdef WLAMPDU_UCODE
		bzero(ampdu->schan_depth_histo, sizeof(ampdu->schan_depth_histo));
#endif
		/* zero out shmem counters */
		if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
			wlc_write_shm(ampdu->wlc, M_TXMPDU_CNT, 0);
			wlc_write_shm(ampdu->wlc, M_TXAMPDU_CNT, 0);
			wlc_write_shm(ampdu->wlc, M_RXBA_CNT, 0);
		}
#endif /* WLAMPDU_MAC */
		break;
	}
#endif /* WLCNT */
#endif /* defined(BCMDBG) || defined(WLTEST) */

#ifdef BCMDBG
	case IOV_GVAL(IOV_AMPDU_DBG):
		*ret_int_ptr = wl_ampdu_dbg;
		break;

	case IOV_SVAL(IOV_AMPDU_DBG):
	        wl_ampdu_dbg = (uint32)int_val;
		break;
#ifdef WLAMPDU_HW
	case IOV_GVAL(IOV_AMPDU_AGGFIFO):
		*ret_int_ptr = ampdu->aggfifo_depth;
		break;
	case IOV_SVAL(IOV_AMPDU_AGGFIFO): {
		uint8 depth = (uint8)int_val;
		if (depth < 8 || depth > AGGFIFO_CAP) {
			WL_ERROR(("aggfifo depth has to be in [8, %d]\n", AGGFIFO_CAP));
			return BCME_BADARG;
		}
		ampdu->aggfifo_depth = depth;
	}
		break;
#endif /* WLAMPDU_HW */
#endif /* BCMDBG */
	case IOV_GVAL(IOV_AMPDUMAC):
		*ret_int_ptr = (int32)wlc->pub->_ampdumac;
		break;

	case IOV_GVAL(IOV_AMPDU_AGGMODE):
		*ret_int_ptr = (int32)ampdu->ampdu_aggmode;
		break;

	case IOV_SVAL(IOV_AMPDU_AGGMODE):
		if (int_val != AMPDU_AGG_AUTO &&
		    int_val != AMPDU_AGGMODE_HOST &&
		    int_val != AMPDU_AGGMODE_MAC) {
			err = BCME_RANGE;
			break;
		}
		ampdu->ampdu_aggmode = (int8)int_val;
		wlc_ampdu_set(ampdu, wlc->pub->_ampdu);
		break;

	case IOV_GVAL(IOV_FRAMEBURST_OVR):
		*ret_int_ptr = ampdu->fb_override_enable;
		break;

	case IOV_SVAL(IOV_FRAMEBURST_OVR):
		ampdu->fb_override_enable = bool_val;
		break;

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

static void
wlc_ffpld_init(ampdu_info_t *ampdu)
{
	int i, j;
	wlc_fifo_info_t *fifo;

	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = (ampdu->fifo_tb + j);
		fifo->ampdu_pld_size = 0;
		for (i = 0; i <= FFPLD_MAX_MCS; i++)
			fifo->mcs2ampdu_table[i] = 255;
		fifo->dmaxferrate = 0;
		fifo->accum_txampdu = 0;
		fifo->prev_txfunfl = 0;
		fifo->accum_txfunfl = 0;

	}

}

/* evaluate the dma transfer rate using the tx underflows as feedback.
 * If necessary, increase tx fifo preloading. If not enough,
 * decrease maximum ampdu size for each mcs till underflows stop
 * Return 1 if pre-loading not active, -1 if not an underflow event,
 * 0 if pre-loading module took care of the event.
 */
static int
wlc_ffpld_check_txfunfl(wlc_info_t *wlc, int fid)
{
	ampdu_info_t *ampdu = wlc->ampdu;
	uint32 phy_rate = MCS_RATE(FFPLD_MAX_MCS, TRUE, FALSE);
	uint32 txunfl_ratio;
	uint8  max_mpdu;
	uint32 current_ampdu_cnt = 0;
	uint16 max_pld_size;
	uint32 new_txunfl;
	wlc_fifo_info_t *fifo = (ampdu->fifo_tb + fid);
	uint xmtfifo_sz;
	uint16 cur_txunfl;

	/* return if we got here for a different reason than underflows */
	cur_txunfl = wlc_read_shm(wlc, M_UCODE_MACSTAT + OFFSETOF(macstat_t, txfunfl[fid]));
	new_txunfl = (uint16)(cur_txunfl - fifo->prev_txfunfl);
	if (new_txunfl == 0) {
		WL_FFPLD(("check_txunfl : TX status FRAG set but no tx underflows\n"));
		return -1;
	}
	fifo->prev_txfunfl = cur_txunfl;

	if (!ampdu->tx_max_funl)
		return 1;

	/* check if fifo is big enough */
	xmtfifo_sz = wlc->xmtfifo_szh[fid];

	if ((TXFIFO_SIZE_UNIT * (uint32)xmtfifo_sz) <= ampdu->ffpld_rsvd)
		return 1;

	max_pld_size = TXFIFO_SIZE_UNIT * xmtfifo_sz - ampdu->ffpld_rsvd;
#ifdef WLCNT
#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(ampdu->wlc->pub))
		ampdu->cnt->txampdu = wlc_read_shm(ampdu->wlc, M_TXAMPDU_CNT);
#endif
	current_ampdu_cnt = ampdu->cnt->txampdu - fifo->prev_txampdu;
#endif /* WLCNT */
	fifo->accum_txfunfl += new_txunfl;

	/* we need to wait for at least 10 underflows */
	if (fifo->accum_txfunfl < 10)
		return 0;

	WL_FFPLD(("ampdu_count %d  tx_underflows %d\n",
		current_ampdu_cnt, fifo->accum_txfunfl));

	/*
	   compute the current ratio of tx unfl per ampdu.
	   When the current ampdu count becomes too
	   big while the ratio remains small, we reset
	   the current count in order to not
	   introduce too big of a latency in detecting a
	   large amount of tx underflows later.
	*/

	txunfl_ratio = current_ampdu_cnt/fifo->accum_txfunfl;

	if (txunfl_ratio > ampdu->tx_max_funl) {
		if (current_ampdu_cnt >= FFPLD_MAX_AMPDU_CNT) {
#ifdef WLCNT
			fifo->prev_txampdu = ampdu->cnt->txampdu;
#endif
			fifo->accum_txfunfl = 0;
		}
		return 0;
	}
	max_mpdu = MIN(fifo->mcs2ampdu_table[FFPLD_MAX_MCS], ampdu->default_pdu);

	/* In case max value max_pdu is already lower than
	   the fifo depth, there is nothing more we can do.
	*/

	if (fifo->ampdu_pld_size >= max_mpdu * FFPLD_MPDU_SIZE) {
		WL_FFPLD(("tx fifo pld : max ampdu fits in fifo\n)"));
#ifdef WLCNT
		fifo->prev_txampdu = ampdu->cnt->txampdu;
#endif
		fifo->accum_txfunfl = 0;
		return 0;
	}

	if (fifo->ampdu_pld_size < max_pld_size) {

		/* increment by TX_FIFO_PLD_INC bytes */
		fifo->ampdu_pld_size += FFPLD_PLD_INCR;
		if (fifo->ampdu_pld_size > max_pld_size)
			fifo->ampdu_pld_size = max_pld_size;

		/* update scb release size */
		scb_ampdu_update_config_all(ampdu);

		/*
		   compute a new dma xfer rate for max_mpdu @ max mcs.
		   This is the minimum dma rate that
		   can acheive no unferflow condition for the current mpdu size.
		*/
		/* note : we divide/multiply by 100 to avoid integer overflows */
		fifo->dmaxferrate =
		        (((phy_rate/100)*(max_mpdu*FFPLD_MPDU_SIZE-fifo->ampdu_pld_size))
		         /(max_mpdu * FFPLD_MPDU_SIZE))*100;

		WL_FFPLD(("DMA estimated transfer rate %d; pre-load size %d\n",
		          fifo->dmaxferrate, fifo->ampdu_pld_size));
	} else {

		/* decrease ampdu size */
		if (fifo->mcs2ampdu_table[FFPLD_MAX_MCS] > 1) {
			if (fifo->mcs2ampdu_table[FFPLD_MAX_MCS] == 255)
				fifo->mcs2ampdu_table[FFPLD_MAX_MCS] = ampdu->default_pdu - 1;
			else
				fifo->mcs2ampdu_table[FFPLD_MAX_MCS] -= 1;

			/* recompute the table */
			wlc_ffpld_calc_mcs2ampdu_table(ampdu, fid);

			/* update scb release size */
			scb_ampdu_update_config_all(ampdu);
		}
	}
#ifdef WLCNT
	fifo->prev_txampdu = ampdu->cnt->txampdu;
#endif
	fifo->accum_txfunfl = 0;
	return 0;
}

static void
wlc_ffpld_calc_mcs2ampdu_table(ampdu_info_t *ampdu, int f)
{
	int i;
	uint32 phy_rate, dma_rate, tmp;
	uint8 max_mpdu;
	wlc_fifo_info_t *fifo = (ampdu->fifo_tb + f);

	/* recompute the dma rate */
	/* note : we divide/multiply by 100 to avoid integer overflows */
	max_mpdu = MIN(fifo->mcs2ampdu_table[FFPLD_MAX_MCS], ampdu->default_pdu);
	phy_rate = MCS_RATE(FFPLD_MAX_MCS, TRUE, FALSE);
	dma_rate = (((phy_rate/100) * (max_mpdu*FFPLD_MPDU_SIZE-fifo->ampdu_pld_size))
	         /(max_mpdu*FFPLD_MPDU_SIZE))*100;
	fifo->dmaxferrate = dma_rate;

	/* fill up the mcs2ampdu table; do not recalc the last mcs */
	dma_rate = dma_rate >> 7;
	for (i = 0; i < FFPLD_MAX_MCS; i++) {
		/* shifting to keep it within integer range */
		phy_rate = MCS_RATE(i, TRUE, FALSE) >> 7;
		if (phy_rate > dma_rate) {
			tmp = ((fifo->ampdu_pld_size * phy_rate) /
				((phy_rate - dma_rate) * FFPLD_MPDU_SIZE)) + 1;
			tmp = MIN(tmp, 255);
			fifo->mcs2ampdu_table[i] = (uint8)tmp;
		}
	}

#ifdef BCMDBG
	wlc_ffpld_show(ampdu);
#endif /* BCMDBG */
}

#ifdef BCMDBG
static void
wlc_ffpld_show(ampdu_info_t *ampdu)
{
	int i, j;
	wlc_fifo_info_t *fifo;

	WL_ERROR(("MCS to AMPDU tables:\n"));
	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = ampdu->fifo_tb + j;
		WL_ERROR(("  FIFO %d : Preload settings: size %d dmarate %d kbps\n",
		          j, fifo->ampdu_pld_size, fifo->dmaxferrate));
		for (i = 0; i <= FFPLD_MAX_MCS; i++) {
			WL_ERROR(("  %d", fifo->mcs2ampdu_table[i]));
		}
		WL_ERROR(("\n\n"));
	}
}
#endif /* BCMDBG */


#if defined(DONGLEBUILD)
static void
wlc_ampdu_txflowcontrol(wlc_info_t *wlc, scb_ampdu_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	wlc_txq_info_t *qi = SCB_WLCIFP(scb_ampdu->scb)->qi;
#ifdef WLLMAC
	if (LMAC_ENAB(wlc->pub))
		return;
#endif /* WLLMAC */

	if (wlc_txflowcontrol_override_isset(wlc, qi, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL)) {
		if ((pktq_len(&scb_ampdu->txq) + ini->tx_in_transit) <=
		    wlc->pub->tunables->ampdudatahiwat) {
			wlc_txflowcontrol_override(wlc, qi, OFF, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL);
		}
	} else {
		if ((pktq_len(&scb_ampdu->txq) + ini->tx_in_transit) >=
		    wlc->pub->tunables->ampdudatahiwat) {
			wlc_txflowcontrol_override(wlc, qi, ON, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL);
		}
	}
}
#endif	/* DONGLEBUILD && !WLLMAC */

/* configuration to disable/enable AMPDU..cases where iovars may not be available */
void
BCMATTACHFN(wlc_ampdu_agg_state_override)(wlc_info_t *wlc, int8 txaggr, int8 rxaggr)
{
	ampdu_info_t *ampdu = wlc->ampdu;

	ampdu->txaggr_override = txaggr;
	ampdu->rxaggr_override = rxaggr;
	if (txaggr != AUTO)
		ampdu->txaggr = txaggr;
	if (rxaggr != AUTO)
		ampdu->rxaggr = rxaggr;
}

/* Changes txaggr/rxaggr global states
	currently supported values:
	txaggr 	rxaggr
	ON		ON
	OFF		OFF
	ON		OFF
		XXXOthers are NOT TESTED
 */
void
wlc_ampdu_agg_state_update_tx(wlc_info_t *wlc, bool txaggr)
{
	ampdu_info_t *ampdu = wlc->ampdu;

	if (ampdu->txaggr_override != AUTO)
		return;

	if (ampdu->txaggr != txaggr) {
		if (txaggr == OFF) {
			wlc_ampdu_cleanup(wlc, ampdu, AMPDU_CLEANUPFLAG_TX);
		}
		ampdu->txaggr = txaggr;
	}
}

void
wlc_ampdu_agg_state_update_rx(wlc_info_t *wlc, bool rxaggr)
{
	ampdu_info_t *ampdu = wlc->ampdu;

	if (ampdu->rxaggr_override != AUTO)
		return;

	if (ampdu->rxaggr != rxaggr) {
		if (rxaggr == OFF) {
			wlc_ampdu_cleanup(wlc, ampdu, AMPDU_CLEANUPFLAG_RX);
		}
		ampdu->rxaggr = rxaggr;
	}
}


/* AMPDU aggregation function called through txmod */
static void BCMFASTPATH
wlc_ampdu_agg(void *ctx, struct scb *scb, void *p, uint prec)
{
	ampdu_info_t *ampdu = (ampdu_info_t *)ctx;
	scb_ampdu_t *scb_ampdu;
	wlc_info_t *wlc;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	ASSERT(ampdu);
	wlc = ampdu->wlc;

#ifdef WLLMAC
	/* LMAC send only data packets through this path */
	if (LMAC_ENAB(wlc->pub)) {
		struct dot11_header *h;
		uint16	fc;
		h = (struct dot11_header *)PKTDATA(wlc->osh, p);
		fc = ltoh16(h->fc);
		if (FC_TYPE(fc) != FC_TYPE_DATA) {
			SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
			return;
		}
	}
#endif /* WLLMAC */

	if (ampdu->txaggr == OFF) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	if (tid >= AMPDU_MAX_SCB_TID) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

	ASSERT(SCB_AMPDU(scb));
	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	/* initialize initiator on first packet; sends addba req */
	if (!(ini = scb_ampdu->ini[tid])) {
		ini = wlc_ampdu_init_tid_ini(ampdu, scb_ampdu, tid, FALSE);
		if (!ini) {
			SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
			return;
		}
	}

	if (ini->ba_state == AMPDU_TID_STATE_BA_OFF) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

#ifdef WLOVERTHRUSTER
	wlc_ampdu_tcp_ack_suppress(ampdu, scb_ampdu, p, tid);
#endif
	if (wlc_ampdu_prec_enq(ampdu->wlc, &scb_ampdu->txq, p, tid)) {
		wlc_ampdu_txflowcontrol(wlc, scb_ampdu, ini);
		wlc_ampdu_txeval(ampdu, scb_ampdu, ini, FALSE);
		return;
	}

	WL_AMPDU_ERR(("wl%d: wlc_ampdu_agg: txq overflow\n", wlc->pub->unit));
	PKTFREE(wlc->osh, p, TRUE);
	WLCNTINCR(wlc->pub->_cnt->txnobuf);
	WLCNTINCR(ampdu->cnt->txdrop);
	AMPDUSCBCNTINCR(scb_ampdu->txdrop);
}

#ifdef WLOVERTHRUSTER
static void BCMFASTPATH
wlc_ampdu_tcp_ack_suppress(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
                           void *p, uint8 tid)
{
	wlc_tcp_ack_info_t *tcp_info = &(ampdu->tcp_ack_info);
	uint8 *ip_header;
	uint8 *tcp_header;
	uint8 is_ack = FALSE;
	uint32 ack_num;
	uint8 prev_dequeued = FALSE;
	void *prev_p;
	uint16 totlen;
	uint32 ip_hdr_len;
	uint32 tcp_hdr_len;
	uint32 pktlen;
	uint16 ethtype;
	osl_t *osh = ampdu->wlc->pub->osh;

	if (tcp_info->tcp_ack_ratio == 0)
		return;

	ethtype = wlc_sdu_etype(ampdu->wlc, p);
	if (ethtype != ntoh16(ETHER_TYPE_IP))
		return;

	pktlen = pkttotlen(osh, p);

	/* move to IP header and verify we have enough data for at least the IP header */
	ip_header = wlc_sdu_data(ampdu->wlc, p);
	ip_hdr_len = 4*(ip_header[0] & 0x0f);

	/* hmm ... not sure that could happen if the packet is valid though ... */
	if (pktlen < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + ip_hdr_len)
		return;

	/* not tcp */
	if (ip_header[9] != 0x06)
		return;

	tcp_header = ip_header + ip_hdr_len;

	/* is it an ack ? */
	if (tcp_header[13] & 0x10) {
		totlen =  (ip_header[3] & 0xff) + (ip_header[2] << 8);
		tcp_hdr_len = 4*((tcp_header[12] & 0xf0) >> 4);
		/* zero length ? */
		if (totlen ==  ip_hdr_len + tcp_hdr_len) {
			is_ack = TRUE;
			tcp_info->tcp_ack_total++;
		}
	}

	/* ack number */
	ack_num = (tcp_header[8] << 24) + (tcp_header[9] << 16) +
	        (tcp_header[10] << 8) +  tcp_header[11];

	/* compare with cache to see if we can replace a previous ack
	 * by this one
	 */
	if (is_ack && tcp_info->prev_is_ack &&
	    (tcp_info->current_dequeued < tcp_info->tcp_ack_ratio) &&
	    (ack_num > tcp_info->prev_ack_num)) {

		/* check that the frame is still in the queue.
		 * If there is one, it has to be the previous ack we queued.
		 */

			if ((prev_p = wlc_ampdu_pktq_ppeek_tail(&scb_ampdu->txq, tid))) {

			/* is it the same dest/src IP addr, port # etc ??
			 * IPs: compare 8 bytes at IP hdrs offset 12
			 * compare tcp hdrs for 8 bytes : includes both ports and
			 * sequence number.
			 */
			if (!memcmp(tcp_info->prev_ip_header+12, ip_header+12, 8)) {
				if  (!memcmp(tcp_info->prev_tcp_header, tcp_header, 4)) {
					/* dequeue the previous ack */
					prev_p = wlc_ampdu_pktq_pdeq_tail(&scb_ampdu->txq, tid);
					PKTFREE(ampdu->wlc->pub->osh, prev_p, TRUE);
					tcp_info->tcp_ack_dequeued++;
					tcp_info->current_dequeued++;
					prev_dequeued = TRUE;
				}
			}
		}
	}

	/* if we kept the last ack and this is an ack, reset the number currently dequeued.
	 * If we reached the limit, "forget" the ack we are enqueing so it won't get
	 * suppressed the next time around.
	 */
	if ((prev_dequeued == FALSE) &&
	    (tcp_info->current_dequeued == tcp_info->tcp_ack_ratio || is_ack)) {
		if (tcp_info->current_dequeued == tcp_info->tcp_ack_ratio)
			is_ack = 0;
		tcp_info->current_dequeued = 0;
	}

	/* if current packet is an ack : add its header infos in cache
	 * else clear the cache. We only consolidate contiguous acks.
	 */
	tcp_info->prev_is_ack = is_ack;
	if (is_ack) {
		tcp_info->prev_ip_header = ip_header;
		tcp_info->prev_tcp_header = tcp_header;
		tcp_info->prev_ack_num = ack_num;
	} else {
		tcp_info->prev_ip_header = NULL;
		tcp_info->prev_tcp_header = NULL;
	}

}
#endif /* WLOVERTHRUSTER */

/* function which contains the smarts of when to release the pdu. Can be
 * called from various places
 * Returns TRUE if released else FALSE
 */
static bool BCMFASTPATH
wlc_ampdu_txeval(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, bool force)
{
	uint16 qlen, release;
	uint8 decision = 0;

	ASSERT(scb_ampdu);
	ASSERT(ini);

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON)
		return FALSE;

	qlen = wlc_ampdu_pktq_plen(&scb_ampdu->txq, ini->tid);

	/* for hwagg, scb_ampdu->release serves as high watermark */
	if (AMPDU_HW_ENAB(ampdu->wlc->pub))
		release = MIN(ini->rem_window, qlen);
	else
		release = MIN(ini->rem_window, MIN(qlen, scb_ampdu->release));

	if (release == 0)
		goto exit;

	if (force) {
		decision = 1;
	} else if (ini->tx_in_transit < ampdu->tx_rel_lowat) {
		decision = 2;
	} else if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
		decision = (release >= scb_ampdu->release) ? 3 : 0;
	} else { /* swagg */
		decision = (release == scb_ampdu->release) ? 4 : 0;
	}

	/* release mpdus if any one of the following conditions are met */
	if (decision > 0) {
#if WLAMPDU_MAC
		if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
			uint16 aggfifo_space;
			aggfifo_space = (uint16) get_aggfifo_space(ampdu, prio2fifo[ini->tid]);
			if (release > aggfifo_space) {

				WLCNTADD(ampdu->cnt->txrel_agfull, release);
				release = aggfifo_space;
			}
			if (release == 0)
				goto exit;
		}
#endif /* WLAMPDU_MAC */

		if (decision == 2)
			WLCNTADD(ampdu->cnt->txrel_wm, release);
		else if (decision >= 3)
			WLCNTADD(ampdu->cnt->txrel_size, release);

		WL_AMPDU_TX(("wl%d: wlc_ampdu_txeval: Releasing %d mpdus: in_transit %d tid %d "
			"wm %d rem_wdw %d qlen %d force %d\n",
			ampdu->wlc->pub->unit, release, ini->tx_in_transit,
			ini->tid, ampdu->tx_rel_lowat, ini->rem_window, qlen, force));

		wlc_ampdu_release(ampdu, scb_ampdu, ini, release);
		return TRUE;
	}

exit:
	WL_AMPDU_TX(("wl%d: wlc_ampdu_txeval: Cannot release: in_transit %d tid %d wm %d "
	             "rem_wdw %d qlen %d release %d\n",
	             ampdu->wlc->pub->unit, ini->tx_in_transit, ini->tid, ampdu->tx_rel_lowat,
	             ini->rem_window, qlen, release));

	return FALSE;
}

static void BCMFASTPATH
wlc_ampdu_release(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, uint16 release)
{
	void *p;
	uint16 seq = 0, indx, delta;

	seq = SCB_SEQNUM(ini->scb, ini->tid) & (SEQNUM_MAX - 1);
	delta = MODSUB_POW2(seq, MODADD_POW2(ini->max_seq, 1, SEQNUM_MAX), SEQNUM_MAX);

	/* do not release past window (seqnum based check). There might be pkts
	 * in txq which are not marked AMPDU so will use up a seq number
	 */
	if ((delta + release) > ini->rem_window) {
		WL_AMPDU_ERR(("wl%d: wlc_ampdu_release: cannot release %d (out of window)\n",
			ampdu->wlc->pub->unit, release));
		return;
	}

	ini->rem_window -= (release + delta);
	ini->tx_in_transit += release;

	for (; release != 0; release--) {
		p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid);
		ASSERT(p);
		ASSERT(PKTPRIO(p) == ini->tid);

		WLPKTTAG(p)->flags |= WLF_AMPDU_MPDU;

		/* assign seqnum and save in pkttag */
		seq = SCB_SEQNUM(ini->scb, ini->tid) & (SEQNUM_MAX - 1);
		SCB_SEQNUM(ini->scb, ini->tid)++;
		WLPKTTAG(p)->seq = seq;

		indx = TX_SEQ_TO_INDEX(seq);
		ASSERT(!isset(ini->ackpending, indx));
		ASSERT(ini->txretry[indx] == 0);
		setbit(ini->ackpending, indx);
		ini->max_seq = seq;

		SCB_TX_NEXT(TXMOD_AMPDU, ini->scb, p, WLC_PRIO_TO_PREC(ini->tid));
	}
}

/* returns TRUE if receives the next expected sequence
 * protects against pkt frees in the txpath by others
 */
static bool BCMFASTPATH
ampdu_is_exp_seq(ampdu_info_t *ampdu, scb_ampdu_tid_ini_t *ini, uint16 seq)
{
	uint16 txretry, offset, indx;
	uint i;
	bool found;
#ifdef BCMDBG
	scb_ampdu_t *scb_ampdu = SCB_AMPDU_CUBBY(ampdu, ini->scb);
	ASSERT(scb_ampdu);
#endif

	txretry = ini->txretry[TX_SEQ_TO_INDEX(seq)];
	if (txretry == 0) {
		if (ini->tx_exp_seq != seq) {
			offset = MODSUB_POW2(seq, ini->tx_exp_seq, SEQNUM_MAX);
			WL_AMPDU_ERR(("wl%d: r0hole: tx_exp_seq 0x%x, seq 0x%x\n",
				ampdu->wlc->pub->unit, ini->tx_exp_seq, seq));

			/* pkts bypassed (or lagging) on way down */
			if (offset > (SEQNUM_MAX >> 1)) {
				WL_ERROR(("wl%d: rlag: tx_exp_seq 0x%x, seq 0x%x\n",
					ampdu->wlc->pub->unit, ini->tx_exp_seq, seq));
				WLCNTINCR(ampdu->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->txrlag);
				return FALSE;
			}

			if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
				WL_ERROR(("wl%d: unexpected seq: start_seq 0x%x, seq 0x%x\n",
					ampdu->wlc->pub->unit, ini->start_seq, seq));
				WLCNTINCR(ampdu->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->txrlag);
				return FALSE;
			}

			WLCNTADD(ampdu->cnt->txr0hole, offset);
			/* send bar to move the window */
			indx = TX_SEQ_TO_INDEX(ini->tx_exp_seq);
			for (i = 0; i < offset; i++, indx = NEXT_TX_INDEX(indx))
				setbit(ini->barpending, indx);
			wlc_ampdu_send_bar(ampdu, ini, FALSE);
		}

		ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
	} else {
		if (ini->retry_cnt && (ini->retry_seq[ini->retry_head] == seq)) {
			ini->retry_seq[ini->retry_head] = 0;
			ini->retry_head = NEXT_TX_INDEX(ini->retry_head);
			ini->retry_cnt--;
			ASSERT(ini->retry_cnt <= AMPDU_TX_BA_MAX_WSIZE);
		} else {
			found = FALSE;
			indx = ini->retry_head;
			for (i = 0; i < ini->retry_cnt; i++, indx = NEXT_TX_INDEX(indx)) {
				if (ini->retry_seq[indx] == seq) {
					found = TRUE;
					break;
				}
			}
			if (!found) {
				WL_ERROR(("wl%d: rnlag: tx_exp_seq 0x%x, seq 0x%x\n",
					ampdu->wlc->pub->unit, ini->tx_exp_seq, seq));
				WLCNTINCR(ampdu->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->txrlag);
				return FALSE;
			} else {
				while (ini->retry_seq[ini->retry_head] != seq) {
					WL_AMPDU_ERR(("wl%d: rnhole: retry_seq 0x%x, seq 0x%x\n",
						ampdu->wlc->pub->unit,
						ini->retry_seq[ini->retry_head], seq));
					setbit(ini->barpending,
						TX_SEQ_TO_INDEX(ini->retry_seq[ini->retry_head]));
					WLCNTINCR(ampdu->cnt->txrnhole);
					ini->retry_seq[ini->retry_head] = 0;
					ini->retry_head = NEXT_TX_INDEX(ini->retry_head);
					ini->retry_cnt--;
				}
				wlc_ampdu_send_bar(ampdu, ini, FALSE);
				ini->retry_seq[ini->retry_head] = 0;
				ini->retry_head = NEXT_TX_INDEX(ini->retry_head);
				ini->retry_cnt--;
				ASSERT(ini->retry_cnt < AMPDU_TX_BA_MAX_WSIZE);
			}
		}
	}

	/* discard the frame if it is a retry and we are in pending off state */
	if (txretry && (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF)) {
		setbit(ini->barpending, TX_SEQ_TO_INDEX(seq));
		return FALSE;
	}

	if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
		ampdu_dump_ini(ini);
		ASSERT(MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) < ini->ba_wsize);
	}
	return TRUE;
}

#ifdef WLAMPDU_MAC
void
wlc_ampdu_change_epoch(ampdu_info_t *ampdu, int fifo, int reason_code)
{
#ifdef BCMDBG
	const char *str = "Undefined";

	switch (reason_code) {
	case AMU_EPOCH_CHG_PLCP:
		str = "PLCP";
		WLCNTINCR(ampdu->cnt->echgr1);
		break;
	case AMU_EPOCH_CHG_FID:
		WLCNTINCR(ampdu->cnt->echgr2);
		str = "FRAMEID";
		break;
	case AMU_EPOCH_CHG_NAGG:
		WLCNTINCR(ampdu->cnt->echgr3);
		str = "AMPDU OFF";
		break;
	case AMU_EPOCH_CHG_MPDU:
		WLCNTINCR(ampdu->cnt->echgr4);
		str = "reg mpdu";
		break;
	case AMU_EPOCH_CHG_DSTTID:
		WLCNTINCR(ampdu->cnt->echgr5);
		str = "diff dest/tid";
		break;
	}
	WL_AMPDU_HWDBG(("wl%d: %s: fifo %d: change epoch for %s\n",
		ampdu->wlc->pub->unit, __FUNCTION__, fifo, str));
#endif /* BCMDBG */
	ASSERT(fifo < AC_COUNT);
	ampdu->hagg[fifo].change_epoch = TRUE;
}

static uint16
wlc_ampdu_calc_maxlen(ampdu_info_t *ampdu, uint8 plcp0, uint plcp3, uint32 txop)
{
	uint8 is40, sgi;
	uint8 mcs = 0;
	uint16 txoplen = 0;
	uint16 maxlen = 0xffff;

	is40 = (plcp0 & MIMO_PLCP_40MHZ) ? 1 : 0;
	sgi = PLCP3_ISSGI(plcp3) ? 1 : 0;
	mcs = plcp0 & ~MIMO_PLCP_40MHZ;
	ASSERT(mcs < MCS_TABLE_SIZE);

	if (txop) {
		/* rate is in Kbps; txop is in usec
		 * ==> len = (rate * txop) / (1000 * 8)
		 */
		txoplen = (MCS_RATE(mcs, is40, sgi) >> 10) * (txop >> 3);

		maxlen = MIN((txoplen),
			(ampdu->max_txlen[mcs][is40][sgi] *
			ampdu->ba_tx_wsize));
	}
	WL_AMPDU_HW(("wl%d: %s: txop %d txoplen %d maxlen %d\n",
		ampdu->wlc->pub->unit, __FUNCTION__, txop, txoplen, maxlen));

	return maxlen;
}
#endif /* WLAMPDU_MAC */

/*
 * ampdu transmit routine
 * Returns BCM error code
 */
int BCMFASTPATH
wlc_sendampdu(ampdu_info_t *ampdu, wlc_txq_info_t *qi, void **pdu, int prec)
{
	wlc_info_t *wlc;
	osl_t *osh;
	void *p;
#ifdef WLAMPDU_HW
	void *pkt[AGGFIFO_CAP];
#else
	void *pkt[AMPDU_MAX_MPDU];
#endif
	uint8 tid, ndelim;
	int err = 0, txretry = -1;
	uint8 preamble_type = WLC_GF_PREAMBLE;
	uint8 fbr_preamble_type = WLC_GF_PREAMBLE;
	uint8 rts_preamble_type = WLC_LONG_PREAMBLE;
	uint8 rts_fbr_preamble_type = WLC_LONG_PREAMBLE;

	bool rr = FALSE, fbr = FALSE, vrate_probe = FALSE;
	uint i, count = 0, fifo, npkts, seg_cnt = 0;
	uint16 plen, len, seq, mcl, mch, indx, frameid, dma_len = 0;
	uint32 ampdu_len, maxlen = 0;
	d11txh_t *txh = NULL;
	uint8 *plcp;
	struct dot11_header *h;
	struct scb *scb;
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 mcs = 0;
	bool regmpdu = FALSE;
	bool use_rts = FALSE, use_cts = FALSE;
	ratespec_t rspec = 0, rspec_fallback = 0;
	ratespec_t rts_rspec = 0, rts_rspec_fallback = 0;
	uint16 mimo_ctlchbw = PHY_TXC1_BW_20MHZ;
	struct dot11_rts_frame *rts;
	uint8 rr_retry_limit;
	wlc_fifo_info_t *f;
	bool fbr_iscck, use_mfbr = FALSE;
	uint32 txop = 0;
#ifdef WLAMPDU_MAC
	ampdumac_info_t *hagg;
	int sc; /* Side channel index */
	uint aggfifo_space = 0;
#endif
	wlc_bsscfg_t *cfg;

	wlc = ampdu->wlc;
	osh = wlc->osh;
	p = *pdu;

	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	f = ampdu->fifo_tb + prio2fifo[tid];

	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* SCB setting may have changed during transition, so just discard the frame */
	if (!SCB_AMPDU(scb)) {
		WLCNTINCR(ampdu->cnt->orphan);
		PKTFREE(osh, p, TRUE);
		*pdu = NULL;

		ASSERT(0);
		return err;
	}

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[tid];
	rr_retry_limit = ampdu->rr_retry_limit_tid[tid];

	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WLCNTINCR(ampdu->cnt->orphan);
		PKTFREE(osh, p, TRUE);
		*pdu = NULL;
		return err;
	}

	/* Something is blocking data packets */
	if (wlc->block_datafifo)
		return BCME_BUSY;

	ASSERT(ini->scb == scb);

#ifdef WLAMPDU_MAC
	sc = prio2fifo[tid];
	hagg = &ampdu->hagg[sc];

	/* agg-fifo space */
	if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
		aggfifo_space = get_aggfifo_space(ampdu, sc);
		WL_AMPDU_HWDBG(("%s: aggfifo_space %d txfifo %d\n", __FUNCTION__, aggfifo_space,
			R_REG(ampdu->wlc->osh, &ampdu->wlc->regs->xmtfifo_frame_cnt)));
#if defined(BCMDBG) && defined(WLAMPDU_UCODE)
		ampdu->schan_depth_histo[MIN(AMPDU_MAX_MPDU, aggfifo_space)]++;
#endif
		if (aggfifo_space == 0) {
			return BCME_BUSY;
		}

		/* check if there are enough descriptors available */
		if (TXAVAIL(wlc, sc) <= (pktsegcnt(osh, p))) {
			WLCNTINCR(ampdu->cnt->txfifofull);
			return BCME_BUSY;
		}
	}
#endif /* WLAMPDU_MAC */

	/* compute txop once for all */
	txop = wlc->edcf_txop[wme_fifo2ac[prio2fifo[tid]]];
	if (txop && wlc->txburst_limit)
		txop = MIN(txop, wlc->txburst_limit);
	else if (wlc->txburst_limit)
		txop = wlc->txburst_limit;

	rr_retry_limit = ampdu->rr_retry_limit_tid[tid];
	ampdu_len = 0;
	dma_len = 0;
	while (p) {
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);
		seq = WLPKTTAG(p)->seq;

		if (WLPKTTAG(p)->flags & WLF_MPDU)
			err = wlc_prep_pdu(wlc, p, &fifo);
		else
			err = wlc_prep_sdu(wlc, &p, (int*)&npkts, &fifo);

		if (err) {
			if (err == BCME_BUSY) {
				WL_AMPDU_TX(("wl%d: wlc_sendampdu: prep_xdu retry; seq 0x%x\n",
					wlc->pub->unit, seq));
				WLCNTINCR(ampdu->cnt->sduretry);
				*pdu = p;
				break;
			}

			/* error in the packet; reject it */
			WL_AMPDU_ERR(("wl%d: wlc_sendampdu: prep_xdu rejected; seq 0x%x\n",
				wlc->pub->unit, seq));
			WLCNTINCR(ampdu->cnt->sdurejected);
			AMPDUSCBCNTINCR(scb_ampdu->sdurejected);

			/* send bar since this may be blocking pkts in scb */
			if (ini->tx_exp_seq == seq) {
				setbit(ini->barpending, TX_SEQ_TO_INDEX(seq));
				wlc_ampdu_send_bar(ampdu, ini, FALSE);
				ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
			}
			*pdu = NULL;
			break;
		}

		WL_AMPDU_TX(("wl%d: wlc_sendampdu: prep_xdu success; seq 0x%x tid %d\n",
			wlc->pub->unit, seq, tid));

		/* pkt is good to be aggregated */

		ASSERT(WLPKTTAG(p)->flags & WLF_MPDU);
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		txh = (d11txh_t *)PKTDATA(osh, p);
		plcp = (uint8 *)(txh + 1);
		h = (struct dot11_header*)(plcp + D11_PHY_HDR_LEN);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;
		indx = TX_SEQ_TO_INDEX(seq);

		/* during roam we get pkts with null a1. kill it here else
		 * does not find scb on dotxstatus and things gets out of sync
		 */
		if (ETHER_ISNULLADDR(&h->a1)) {
			WL_AMPDU_ERR(("wl%d: sendampdu; dropping frame with null a1\n",
				wlc->pub->unit));
			WLCNTINCR(ampdu->cnt->txdrop);
			if (ini->tx_exp_seq == seq) {
				setbit(ini->barpending, indx);
				wlc_ampdu_send_bar(ampdu, ini, FALSE);
				ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
			}
			PKTFREE(osh, p, TRUE);
			err = BCME_ERROR;
			*pdu = NULL;
			break;
		}

		/* If the packet was not retrieved from HW FIFO before attempting
		 * over the air for multi-queue
		 */
		if (!(WLPKTTAG(p)->flags & WLF_FIFOPKT) &&
		    !ampdu_is_exp_seq(ampdu, ini, seq)) {
			PKTFREE(osh, p, TRUE);
			err = BCME_ERROR;
			*pdu = NULL;
			break;
		}

		if (WLPKTTAG(p)->flags & WLF_FIFOPKT)
			WLPKTTAG(p)->flags &= ~WLF_FIFOPKT;

		if (!isset(ini->ackpending, indx)) {
			ampdu_dump_ini(ini);
			ASSERT(isset(ini->ackpending, indx));
		}

		/* check mcl fields and test whether it can be agg'd */
		mcl = ltoh16(txh->MacTxControlLow);
		mcl &= ~TXC_AMPDU_MASK;
		fbr_iscck = !(ltoh16(txh->XtraFrameTypes) & 0x3);
		txh->PreloadSize = 0; /* always default to 0 */

		/*  Handle retry limits */
		if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
			if (ini->txretry[indx] < rr_retry_limit) {
				rr = TRUE;
				ASSERT(!fbr);
			} else {
				/* send as regular mpdu if not a mimo frame or in ps mode
				 * or we are in pending off state
				 * make sure the fallback rate can be only HT or CCK
				 */
				fbr = TRUE;
				ASSERT(!rr);

				regmpdu = fbr_iscck;
				if (regmpdu) {
					mcl &= ~(TXC_SENDRTS | TXC_SENDCTS | TXC_LONGFRAME);
					mcl |= TXC_STARTMSDU;
					txh->MacTxControlLow = htol16(mcl);

					mch = ltoh16(txh->MacTxControlHigh);
					mch |= TXC_AMPDU_FBR;
					txh->MacTxControlHigh = htol16(mch);
					break;
				}
			}

			/* first packet in ampdu */
			if (txretry == -1) {
				txretry = ini->txretry[indx];
				vrate_probe = (WLPKTTAG(p)->flags & WLF_VRATE_PROBE) ? TRUE : FALSE;
			}
		} /* AMPDU_HOST_ENAB */

		/* extract the length info */
		len = fbr_iscck ? WLC_GET_CCK_PLCP_LEN(txh->FragPLCPFallback)
			: WLC_GET_MIMO_PLCP_LEN(txh->FragPLCPFallback);

		if (!(WLPKTTAG(p)->flags & WLF_MIMO) ||
		    (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
		    SCB_PS(scb)) {
			/* clear ampdu related settings if going as regular frame */
			if ((WLPKTTAG(p)->flags & WLF_MIMO) &&
			    (SCB_PS(scb) || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF))) {
				WLC_CLR_MIMO_PLCP_AMPDU(plcp);
				WLC_SET_MIMO_PLCP_LEN(plcp, len);
			}
			regmpdu = TRUE;
			mcl &= ~(TXC_SENDRTS | TXC_SENDCTS | TXC_LONGFRAME);
			txh->MacTxControlLow = htol16(mcl);
			break;
		}

		/* pkt is good to be aggregated */

		if (ini->txretry[indx] && AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
			WLCNTINCR(ampdu->cnt->u0.txretry_mpdu);
		}

		/* retrieve null delimiter count */
		ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
		seg_cnt += pktsegcnt(osh, p);

		if (!WL_AMPDU_HW_ON())
			WL_AMPDU_TX(("wl%d: wlc_sendampdu: mpdu %d plcp_len %d\n",
				wlc->pub->unit, count, len));

#ifdef WLAMPDU_MAC
		/*
		 * aggregateable mpdu. For ucode/hw agg,
		 * test whether need to break or change the epoch
		 */
		if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
			uint8 max_pdu;

			ASSERT((uint)sc == fifo);

			/* check if link or tid has changed */
			if (scb != hagg->scb || tid != hagg->tid) {
				hagg->scb = scb;
				hagg->tid = tid;
				wlc_ampdu_change_epoch(ampdu, sc, AMU_EPOCH_CHG_DSTTID);
			}

			/* mark every MPDU's plcp to indicate ampdu */
			WLC_SET_MIMO_PLCP_AMPDU(plcp);

			/* rate/sgi/bw/etc phy info */
			if (plcp[0] != hagg->last_plcp0 || (plcp[3] & 0xf0) != hagg->last_plcp3) {
				hagg->last_plcp0 = plcp[0];
				hagg->last_plcp3 = plcp[3] & 0xf0;
				wlc_ampdu_change_epoch(ampdu, sc, AMU_EPOCH_CHG_PLCP);
			}
			/* frameid -- force change by the rate selection algo */
			frameid = ltoh16(txh->TxFrameID);
			if (frameid & TXFID_RATE_PROBE_MASK) {
				wlc_ampdu_change_epoch(ampdu, sc, AMU_EPOCH_CHG_FID);
				wlc_ratesel_probe_ready(wlc->rsi, scb, frameid, TRUE, 0);
			}

			/* Set bits 9:10 to any nonzero value to indicate to
			 * ucode that this packet can be aggregated
			 */
			mcl |= (TXC_AMPDU_FIRST << TXC_AMPDU_SHIFT);
			/* set sequence number in tx-desc */
			txh->AmpduSeqCtl = htol16(h->seq);

			/*
			 * compute aggregation parameters
			 */
			/* limit on number of mpdus per ampdu */
			mcs = plcp[0] & ~MIMO_PLCP_40MHZ;
			max_pdu = scb_ampdu->max_pdu;

			/* MaxAggSize in duration(usec) and bytes for hw/ucode agg, respectively */
			if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
				/* use duration limit in usec */
				uint32 maxdur = ampdu->dur ? (ampdu->dur * 1000) : 0xffff;
				if (txop && txop < maxdur)
					maxdur = txop;
				if ((maxdur & 0xffff0000) != 0)
					maxdur = 0xffff;
				txh->u1.MaxAggDur = htol16(maxdur & 0xffff);
				ASSERT(txh->u1.MaxAggDur != 0);
				/* hack for now :
				 * should compute from xmtfifo_sz[fifo] * 256 / max_bytes_per_mpdu
				 * e.g. fifo1: floor(255 * 256, 1650)
				 */
				txh->u2.MaxRNum = htol16(ampdu->wlc->xmtfifo_frmmaxh[fifo]);
				ASSERT(txh->u2.MaxRNum > 0);
			} else {
				if (!AMPDU_HW_ENAB(ampdu->wlc->pub)) {
					if (MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) {
						uint16 preloadsize = f->ampdu_pld_size;
						if (D11REV_IS(wlc->pub->corerev, 29))
							preloadsize >>= 2;
						txh->PreloadSize = htol16(preloadsize);
						if (max_pdu > f->mcs2ampdu_table[mcs])
							max_pdu = f->mcs2ampdu_table[mcs];
					}
				}
				txh->u1.MaxAggLen = htol16(wlc_ampdu_calc_maxlen(ampdu,
					plcp[0], plcp[3], txop));
				if (!fbr_iscck)
					txh->u2.MaxAggLen_FBR = htol16(wlc_ampdu_calc_maxlen(ampdu,
						txh->FragPLCPFallback[0], txh->FragPLCPFallback[3],
						txop));
				else
					txh->u2.MaxAggLen_FBR = 0;
			}

			txh->MaxNMpdus = htol16((max_pdu << 8) | max_pdu);
			/* retry limits - overload MModeFbrLen */
			if (WLPKTTAG(p)->flags & WLF_VRATE_PROBE) {
				txh->MModeFbrLen = htol16((1 << 8) |
					(ampdu->retry_limit_tid[tid] & 0xff));
				txh->MaxNMpdus = htol16((max_pdu << 8) | ampdu->probe_mpdu);
			} else {
				txh->MModeFbrLen = htol16(((rr_retry_limit & 0xff) << 8) |
					(ampdu->retry_limit_tid[tid] & 0xff));
			}

			WL_AMPDU_HW(("wl%d: %s len %d MaxNMpdus 0x%x null delim %d MinMBytes %d "
				     "MaxAggDur %d MaxRNum %d rr_lmnt 0x%x\n", wlc->pub->unit,
				     __FUNCTION__, len, txh->MaxNMpdus, ndelim, txh->MinMBytes,
				     txh->u1.MaxAggDur, txh->u2.MaxRNum, txh->MModeFbrLen));

			/* Ucode agg:
			 * length to be written into side channel includes:
			 * leading delimiter, mac hdr to fcs, padding, null delimiter(s)
			 * i.e. (len + 4 + 4*num_paddelims + padbytes)
			 * padbytes is num required to round to 32 bits
			 */
			if (AMPDU_UCODE_ENAB(ampdu->wlc->pub)) {
				if (ndelim) {
					len = ROUNDUP(len, 4);
					len += (ndelim  + 1) * AMPDU_DELIMITER_LEN;
				} else {
					len += AMPDU_DELIMITER_LEN;
				}
			}

			/* flip the epoch? */
			if (hagg->change_epoch) {
				hagg->epoch = !hagg->epoch;
				hagg->change_epoch = FALSE;
				WL_AMPDU_HWDBG(("sendampdu: fifo %d new epoch: %d frameid %x\n",
					sc, hagg->epoch, frameid));
				WLCNTINCR(ampdu->cnt->epochdelta);
			}
			aggfifo_enque(ampdu, len, sc);
		} else
#endif /* WLAMPDU_MAC */
		{ /* host agg */
			if (count == 0) {
				uint16 fc;
				mcl |= (TXC_AMPDU_FIRST << TXC_AMPDU_SHIFT);
				/* refill the bits since might be a retx mpdu */
				mcl |= TXC_STARTMSDU;
				rts = (struct dot11_rts_frame*)&txh->rts_frame;
				fc = ltoh16(rts->fc);
				if ((fc & FC_KIND_MASK) == FC_RTS) {
					mcl |= TXC_SENDRTS;
					use_rts = TRUE;
				}
				if ((fc & FC_KIND_MASK) == FC_CTS) {
					mcl |= TXC_SENDCTS;
					use_cts = TRUE;
				}
			} else {
				mcl |= (TXC_AMPDU_MIDDLE << TXC_AMPDU_SHIFT);
				mcl &= ~(TXC_STARTMSDU | TXC_SENDRTS | TXC_SENDCTS);
			}

			len = ROUNDUP(len, 4);
			ampdu_len += (len + (ndelim + 1) * AMPDU_DELIMITER_LEN);
			ASSERT(ampdu_len <= scb_ampdu->max_rxlen);
			dma_len += (uint16)pkttotlen(osh, p);

			WL_AMPDU_TX(("wl%d: wlc_sendampdu: ampdu_len %d seg_cnt %d null delim %d\n",
				wlc->pub->unit, ampdu_len, seg_cnt, ndelim));
		}
		txh->MacTxControlLow = htol16(mcl);

		/* this packet is added */
		pkt[count++] = p;

		/* patch the first MPDU */
		if (AMPDU_HOST_ENAB(ampdu->wlc->pub) && count == 1) {
			uint8 plcp0, plcp3, is40, sgi;
			uint32 txoplen = 0;

			if (rr) {
				plcp0 = plcp[0];
				plcp3 = plcp[3];
			} else {
				plcp0 = txh->FragPLCPFallback[0];
				plcp3 = txh->FragPLCPFallback[3];

				/* Support multiple fallback rate:
				 * For txtimes > rr_retry_limit, use fallback -> fallback.
				 * To get around the fix-rate case check if primary == fallback rate
				 */
				if (ini->txretry[indx] > rr_retry_limit && ampdu->mfbr &&
				    plcp[0] != plcp0) {
					ratespec_t fbrspec;
					uint8 fbr_mcs;
					uint16 phyctl1;

					use_mfbr = TRUE;
					fbrspec = wlc_ratesel_getmcsfbr(wlc->rsi, scb,
						ltoh16(txh->TxFrameID), plcp0 & ~MIMO_PLCP_40MHZ);
					fbr_mcs = RSPEC_RATE_MASK & fbrspec;

					/* restore bandwidth */
					fbrspec &= ~RSPEC_BW_MASK;
					fbrspec |= (ltoh16(txh->PhyTxControlWord_1_Fbr)
					         & PHY_TXC1_BW_MASK) << RSPEC_BW_SHIFT;

					if (IS_SINGLE_STREAM(fbr_mcs)) {
						fbrspec &= ~(RSPEC_STF_MASK | RSPEC_STC_MASK);

						/* For SISO MCS use STBC if possible */
						if (IS_MCS(fbrspec) &&
						    WLC_STF_SS_STBC_TX(wlc, scb)) {
							uint8 stc, mcs_rate;

							ASSERT(WLC_STBC_CAP_PHY(wlc));
							mcs_rate = fbrspec & RSPEC_RATE_MASK;
							/* Nss for single stream is always 1 */
							stc = 1;
							fbrspec |=
							   (PHY_TXC1_MODE_STBC << RSPEC_STF_SHIFT) |
							    (stc << RSPEC_STC_SHIFT);

							/* also set plcp bits 29:28 */
							plcp3 = (plcp3 & ~PLCP3_STC_MASK) |
							        (stc << PLCP3_STC_SHIFT);
							txh->FragPLCPFallback[3] = plcp3;
							WLCNTINCR(ampdu->cnt->txampdu_mfbr_stbc);
						} else
							fbrspec |=
							     wlc->stf->ss_opmode << RSPEC_STF_SHIFT;
					}
					/* rspec returned from getmcsfbr() doesn't have SGI info. */
					if (wlc->sgi_tx == ON)
						fbrspec |= RSPEC_SHORT_GI;
					phyctl1 = wlc_phytxctl1_calc(wlc, fbrspec);
					txh->PhyTxControlWord_1_Fbr = htol16(phyctl1);

					txh->FragPLCPFallback[0] = fbr_mcs |
						(plcp0 & MIMO_PLCP_40MHZ);
					plcp0 = txh->FragPLCPFallback[0];
				}
			}
			is40 = (plcp0 & MIMO_PLCP_40MHZ) ? 1 : 0;
			sgi = PLCP3_ISSGI(plcp3) ? 1 : 0;
			mcs = plcp0 & ~MIMO_PLCP_40MHZ;
			ASSERT(mcs < MCS_TABLE_SIZE);
			maxlen = MIN(scb_ampdu->max_rxlen, ampdu->max_txlen[mcs][is40][sgi]);

			/* take into account txop and tx burst limit restriction */
			if (txop) {
				/* rate is in Kbps; txop is in usec
				 * ==> len = (rate * txop) / (1000 * 8)
				 */
				txoplen = (MCS_RATE(mcs, is40, sgi) >> 10) * (txop >> 3);
				maxlen = MIN(maxlen, txoplen);
			}

			if (is40)
				mimo_ctlchbw = CHSPEC_SB_UPPER(WLC_BAND_PI_RADIO_CHANSPEC) ?
					PHY_TXC1_BW_20MHZ_UP : PHY_TXC1_BW_20MHZ;

			/* rebuild the rspec and rspec_fallback */
			rspec = RSPEC_MIMORATE;
			rspec |= plcp[0] & ~MIMO_PLCP_40MHZ;
			if (plcp[0] & MIMO_PLCP_40MHZ)
				rspec |= (PHY_TXC1_BW_40MHZ << RSPEC_BW_SHIFT);

			if (fbr_iscck) /* CCK */
				rspec_fallback =
					CCK_RSPEC(CCK_PHY2MAC_RATE(txh->FragPLCPFallback[0]));
			else { /* MIMO */
				rspec_fallback = RSPEC_MIMORATE;
				rspec_fallback |= txh->FragPLCPFallback[0] & ~MIMO_PLCP_40MHZ;
				if (txh->FragPLCPFallback[0] & MIMO_PLCP_40MHZ)
					rspec_fallback |= (PHY_TXC1_BW_40MHZ << RSPEC_BW_SHIFT);
			}

			if (use_rts || use_cts) {
				rts_rspec = wlc_rspec_to_rts_rspec(cfg, rspec, FALSE,
					mimo_ctlchbw);
				rts_rspec_fallback = wlc_rspec_to_rts_rspec(cfg, rspec_fallback,
					FALSE, mimo_ctlchbw);
				/* need to reconstruct plcp and phyctl words for rts_fallback */
				if (use_mfbr) {
					int rts_phylen;
					uint8 rts_plcp_fallback[D11_PHY_HDR_LEN], old_ndelim;
					uint16 phyctl1, xfts;

					old_ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
					phyctl1 = wlc_phytxctl1_calc(wlc, rts_rspec_fallback);
					txh->PhyTxControlWord_1_FbrRts = htol16(phyctl1);
					if (use_cts)
						rts_phylen = DOT11_CTS_LEN + DOT11_FCS_LEN;
					else
						rts_phylen = DOT11_RTS_LEN + DOT11_FCS_LEN;
					wlc_compute_plcp(wlc, rts_rspec_fallback, rts_phylen,
						rts_plcp_fallback);
					bcopy(rts_plcp_fallback, (char*)&txh->RTSPLCPFallback,
					      sizeof(txh->RTSPLCPFallback));
					/* restore it */
					txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM] = old_ndelim;

					/* update XtraFrameTypes in case it has changed */
					xfts = ltoh16(txh->XtraFrameTypes);
					xfts &= ~(PHY_TXC_FT_MASK << XFTS_FBRRTS_FT_SHIFT);
					xfts |= ((IS_CCK(rts_rspec_fallback) ? FT_CCK : FT_OFDM)
						<< XFTS_FBRRTS_FT_SHIFT);
					txh->XtraFrameTypes = htol16(xfts);
				}
			}
		} /* if (first mpdu for host agg) */

		/* test whether to add more */
		if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
			if ((MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) &&
			    (count == f->mcs2ampdu_table[mcs])) {
				WL_AMPDU_ERR(("wl%d: PR 37644: stopping ampdu at %d for mcs %d",
					wlc->pub->unit, count, mcs));
				break;
			}

			/* limit the size of ampdu for probe packets */
			if (vrate_probe && (count == ampdu->probe_mpdu))
				break;

			if (count == scb_ampdu->max_pdu)
				break;
		}
#ifdef WLAMPDU_MAC
		else {
			if (count >= aggfifo_space) {
				WL_AMPDU_HW(("sendampdu: break due to aggfifo full."
					     "count %d space %d\n", count, aggfifo_space));
				break;
			}
		}
#endif /* WLAMPDU_MAC */

		/* check to see if the next pkt is a candidate for aggregation */
		p = pktq_ppeek(&qi->q, prec);

		if (ampdu->hiagg_mode) {
			if (!p && (prec & 1)) {
				prec = prec & ~1;
				p = pktq_ppeek(&qi->q, prec);
			}
		}
		if (p) {
			if (WLPKTFLAG_AMPDU(WLPKTTAG(p)) &&
				(WLPKTTAGSCBGET(p) == scb) &&
				((uint8)PKTPRIO(p) == tid)) {
				uint nsegs;
				if (BCM4331_CHIP_ID == CHIPID(wlc->pub->sih->chip))
					nsegs = pktsegcnt_war(osh, p) << 1;
				else
					nsegs = pktsegcnt(osh, p);
				WL_NONE(("%s: txavail %d  < seg_cnt %d nsegs %d\n", __FUNCTION__,
					TXAVAIL(wlc, fifo), seg_cnt, nsegs));
				/* check if there are enough descriptors available */
				if (TXAVAIL(wlc, fifo) <= (seg_cnt + nsegs)) {
					WLCNTINCR(ampdu->cnt->txfifofull);
					p = NULL;
					continue;
				}

				if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
					plen = pkttotlen(osh, p) + AMPDU_MAX_MPDU_OVERHEAD;
					plen = MAX(scb_ampdu->min_len, plen);
					if ((plen + ampdu_len) > maxlen) {
						p = NULL;
						continue;
					}

					ASSERT((rr && !fbr) || (!rr && fbr));
					indx = TX_SEQ_TO_INDEX(WLPKTTAG(p)->seq);
					if (ampdu->hiagg_mode) {
						if (((ini->txretry[indx] < rr_retry_limit) &&
						     fbr) ||
						    ((ini->txretry[indx] >= rr_retry_limit) &&
						     rr)) {
							p = NULL;
							continue;
						}
					} else {
						if (txretry != ini->txretry[indx]) {
							p = NULL;
							continue;
						}
					}
				}

				p = pktq_pdeq(&qi->q, prec);
				ASSERT(p);
			} else {
#ifdef WLAMPDU_MAC
				if (AMPDU_MAC_ENAB(ampdu->wlc->pub))
					wlc_ampdu_change_epoch(ampdu, sc, AMU_EPOCH_CHG_DSTTID);
#endif
				p = NULL;
			}
		}
	}  /* end while(p) */

	if (count) {

		WL_AMPDU_HWDBG(("%s: tot_mpdu %d txfifo %d\n", __FUNCTION__, count,
			R_REG(ampdu->wlc->osh, &ampdu->wlc->regs->xmtfifo_frame_cnt)));
		WLCNTADD(ampdu->cnt->txmpdu, count);
		AMPDUSCBCNTADD(scb_ampdu->txmpdu, count);
		if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
#ifdef WLCNT
			ampdu->cnt->txampdu++;
#endif
#if defined(BCMDBG) || defined(WLTEST)
			ampdu->txmcs[mcs]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
			WLCNTADD(ampdu->cnt->txmpdu_cnt[mcs], count);

			/* patch up the last txh */
			txh = (d11txh_t *)PKTDATA(osh, pkt[count - 1]);
			mcl = ltoh16(txh->MacTxControlLow);
			mcl &= ~TXC_AMPDU_MASK;
			mcl |= (TXC_AMPDU_LAST << TXC_AMPDU_SHIFT);
			txh->MacTxControlLow = htol16(mcl);

			/* remove the null delimiter after last mpdu */
			ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
			txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM] = 0;
			ampdu_len -= ndelim * AMPDU_DELIMITER_LEN;

			/* remove the pad len from last mpdu */
			fbr_iscck = ((ltoh16(txh->XtraFrameTypes) & 0x3) == 0);
			len = fbr_iscck ? WLC_GET_CCK_PLCP_LEN(txh->FragPLCPFallback)
				: WLC_GET_MIMO_PLCP_LEN(txh->FragPLCPFallback);
			ampdu_len -= ROUNDUP(len, 4) - len;

			/* patch up the first txh & plcp */
			txh = (d11txh_t *)PKTDATA(osh, pkt[0]);
			plcp = (uint8 *)(txh + 1);

			WLC_SET_MIMO_PLCP_LEN(plcp, ampdu_len);
			/* mark plcp to indicate ampdu */
			WLC_SET_MIMO_PLCP_AMPDU(plcp);

			/* reset the mixed mode header durations */
			if (txh->MModeLen) {
				uint16 mmodelen = wlc_calc_lsig_len(wlc, rspec, ampdu_len);
				txh->MModeLen = htol16(mmodelen);
				preamble_type = WLC_MM_PREAMBLE;
			}
			if (txh->MModeFbrLen) {
				uint16 mmfbrlen = wlc_calc_lsig_len(wlc, rspec_fallback, ampdu_len);
				txh->MModeFbrLen = htol16(mmfbrlen);
				fbr_preamble_type = WLC_MM_PREAMBLE;
			}

			/* set the preload length */
			if (MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) {
				dma_len = MIN(dma_len, f->ampdu_pld_size);
				if (D11REV_IS(wlc->pub->corerev, 29))
					dma_len >>= 2;
				txh->PreloadSize = htol16(dma_len);
			} else
				txh->PreloadSize = 0;

			mch = ltoh16(txh->MacTxControlHigh);

			/* update RTS dur fields */
			if (use_rts || use_cts) {
				uint16 durid;
				rts = (struct dot11_rts_frame*)&txh->rts_frame;
				if ((mch & TXC_PREAMBLE_RTS_MAIN_SHORT) ==
				    TXC_PREAMBLE_RTS_MAIN_SHORT)
					rts_preamble_type = WLC_SHORT_PREAMBLE;

				if ((mch & TXC_PREAMBLE_RTS_FB_SHORT) == TXC_PREAMBLE_RTS_FB_SHORT)
					rts_fbr_preamble_type = WLC_SHORT_PREAMBLE;

				durid = wlc_compute_rtscts_dur(wlc, use_cts, rts_rspec, rspec,
					rts_preamble_type, preamble_type, ampdu_len, TRUE);
				rts->durid = htol16(durid);
				durid = wlc_compute_rtscts_dur(wlc, use_cts,
					rts_rspec_fallback, rspec_fallback, rts_fbr_preamble_type,
					fbr_preamble_type, ampdu_len, TRUE);
				txh->RTSDurFallback = htol16(durid);
				/* set TxFesTimeNormal */
				txh->TxFesTimeNormal = rts->durid;
				/* set fallback rate version of TxFesTimeNormal */
				txh->TxFesTimeFallback = txh->RTSDurFallback;
			}

			/* set flag and plcp for fallback rate */
			if (fbr) {
				WLCNTADD(ampdu->cnt->txfbr_mpdu, count);
				WLCNTINCR(ampdu->cnt->txfbr_ampdu);
				mch |= TXC_AMPDU_FBR;
				txh->MacTxControlHigh = htol16(mch);
				WLC_SET_MIMO_PLCP_AMPDU(plcp);
				WLC_SET_MIMO_PLCP_AMPDU(txh->FragPLCPFallback);
				if (PLCP3_ISSGI(txh->FragPLCPFallback[3])) {
					WLCNTINCR(ampdu->cnt->u1.txampdu_sgi);
#if defined(BCMDBG) || defined(WLTEST)
					ampdu->txmcssgi[mcs]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
				}
				if (PLCP3_ISSTBC(txh->FragPLCPFallback[3])) {
					WLCNTINCR(ampdu->cnt->u2.txampdu_stbc);
#if defined(BCMDBG) || defined(WLTEST)
					ampdu->txmcsstbc[mcs]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
				}
			} else {
				if (PLCP3_ISSGI(plcp[3])) {
					WLCNTINCR(ampdu->cnt->u1.txampdu_sgi);
#if defined(BCMDBG) || defined(WLTEST)
					ampdu->txmcssgi[mcs]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
				}
				if (PLCP3_ISSTBC(plcp[3])) {
					WLCNTINCR(ampdu->cnt->u2.txampdu_stbc);
#if defined(BCMDBG) || defined(WLTEST)
					ampdu->txmcsstbc[mcs]++;
#endif
				}
			}

			if (txretry)
				WLCNTINCR(ampdu->cnt->txretry_ampdu);

			WL_AMPDU_TX(("wl%d: wlc_sendampdu: count %d ampdu_len %d\n",
				wlc->pub->unit, count, ampdu_len));

			/* inform rate_sel if it this is a rate probe pkt */
			frameid = ltoh16(txh->TxFrameID);
			if (frameid & TXFID_RATE_PROBE_MASK) {
				wlc_ratesel_probe_ready(wlc->rsi, scb, frameid, TRUE,
					(uint8)txretry);
			}
		}
#ifdef WLC_HIGH_ONLY
		if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU)
			bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU,
				TRUE);
#endif

		/* send all mpdus atomically to the dma engine */
		if (AMPDU_MAC_ENAB(wlc->pub)) {
			for (i = 0; i < count; i++)
				wlc_txfifo(wlc, fifo, pkt[i], TRUE, 1); /* always commit */
		} else {
			for (i = 0; i < count; i++)
				wlc_txfifo(wlc, fifo, pkt[i], i == (count-1), ampdu->txpkt_weight);
		}
#ifdef WLC_HIGH_ONLY
		if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU)
			bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU,
				FALSE);
#endif

	} /* endif (count) */

	if (regmpdu) {
		WL_AMPDU_TX(("wl%d: wlc_sendampdu: sending regular mpdu\n", wlc->pub->unit));

		/* inform rate_sel if it this is a rate probe pkt */
		txh = (d11txh_t *)PKTDATA(osh, p);
		frameid = ltoh16(txh->TxFrameID);
		if (frameid & TXFID_RATE_PROBE_MASK) {
			wlc_ratesel_probe_ready(wlc->rsi, scb, frameid, FALSE, 0);
		}

		WLCNTINCR(ampdu->cnt->txregmpdu);
		wlc_txfifo(wlc, fifo, p, TRUE, 1);

#ifdef WLAMPDU_MAC
		if (AMPDU_MAC_ENAB(ampdu->wlc->pub))
			wlc_ampdu_change_epoch(ampdu, sc, AMU_EPOCH_CHG_NAGG);
#endif
	}

	return err;
}

/* send bar if needed to move the window forward;
 * if start is set move window to start_seq
 */
static void
wlc_ampdu_send_bar(ampdu_info_t *ampdu, scb_ampdu_tid_ini_t *ini, bool start)
{
	int indx, i, offset = -1;
	scb_ampdu_t *scb_ampdu;
	void *p;
	uint16 seq;

	if (ini->bar_ackpending)
		return;

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON)
		return;

	if (ini->bar_cnt >= AMPDU_BAR_RETRY_CNT)
		return;

	if (ini->bar_cnt == (AMPDU_BAR_RETRY_CNT >> 1)) {
		ini->bar_cnt++;
		ini->retry_bar = AMPDU_R_BAR_HALF_RETRY_CNT;
		return;
	}

	ASSERT(ini->ba_state == AMPDU_TID_STATE_BA_ON);

	if (start)
		seq = ini->start_seq;
	else {
		indx = TX_SEQ_TO_INDEX(ini->start_seq);
		for (i = 0; i < (ini->ba_wsize - ini->rem_window); i++) {
			if (isset(ini->barpending, indx))
				offset = i;
			indx = NEXT_TX_INDEX(indx);
		}

		if (offset == -1) {
			ini->bar_cnt = 0;
			return;
		}

		seq = MODADD_POW2(ini->start_seq, offset + 1, SEQNUM_MAX);
	}

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, ini->scb);
	ASSERT(scb_ampdu);

	if (ampdu->no_bar == FALSE) {
		bool blocked;
		p = wlc_send_bar(ampdu->wlc, ini->scb, ini->tid, seq,
		                 DOT11_BA_CTL_POLICY_NORMAL, TRUE, &blocked);
		if (blocked) {
			ini->retry_bar = AMPDU_R_BAR_BLOCKED;
			return;
		}

		if (p == NULL) {
			ini->retry_bar = AMPDU_R_BAR_NO_BUFFER;
			return;
		}

		if (wlc_pkt_callback_register(ampdu->wlc, wlc_send_bar_complete, ini, p)) {
			ini->retry_bar = AMPDU_R_BAR_CALLBACK_FAIL;
			return;
		}

		WLCNTINCR(ampdu->cnt->txbar);
		AMPDUSCBCNTINCR(scb_ampdu->txbar);
	}

	ini->bar_cnt++;
	ini->bar_ackpending = TRUE;
	ini->bar_ackpending_seq = seq;

	if (ampdu->no_bar == TRUE) {
		wlc_send_bar_complete(ampdu->wlc, TX_STATUS_ACK_RCV, ini);
	}
}

/* bump up the start seq to move the window */
static INLINE void
wlc_ampdu_ini_move_window(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini)
{
	uint16 indx, i, range;

	/* ba_wsize can be zero in AMPDU_TID_STATE_BA_OFF or PENDING_ON state */
	if (ini->ba_wsize == 0)
		return;

	range = MODSUB_POW2(MODADD_POW2(ini->max_seq, 1, SEQNUM_MAX), ini->start_seq, SEQNUM_MAX);
	for (i = 0, indx = TX_SEQ_TO_INDEX(ini->start_seq);
		(i < range) && (!isset(ini->ackpending, indx)) &&
		(!isset(ini->barpending, indx));
		i++, indx = NEXT_TX_INDEX(indx));

	ini->start_seq = MODADD_POW2(ini->start_seq, i, SEQNUM_MAX);
	ini->rem_window += i;
	/* mark alive only if window moves forward */
	if (i)
		ini->alive = TRUE;

	/* if possible, release some buffered pdus */
	wlc_ampdu_txeval(ampdu, scb_ampdu, ini, FALSE);

	WL_AMPDU_TX(("wl%d: wlc_ampdu_ini_move_window: tid %d start_seq bumped to 0x%x\n",
		ampdu->wlc->pub->unit, ini->tid, ini->start_seq));

	if (ini->rem_window > ini->ba_wsize) {
		WL_ERROR(("wl%d: wlc_ampdu_ini_move_window: rem_window %d, ba_wsize %d "
			"i %d range %d sseq 0x%x\n",
			ampdu->wlc->pub->unit, ini->rem_window, ini->ba_wsize,
			i, range, ini->start_seq));
		ASSERT(0);
	}
}

static void
wlc_send_bar_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	scb_ampdu_tid_ini_t *ini = (scb_ampdu_tid_ini_t *)arg;
	ampdu_info_t *ampdu = wlc->ampdu;
	uint16 indx, endindex, i;

	WL_AMPDU_CTL(("wl%d: wlc_send_bar_complete for tid %d status 0x%x\n",
		wlc->pub->unit, ini->tid, txstatus));

	if (ini->free_me) {
		for (i = 0; i < AMPDU_INI_FREE; i++) {
			if (ampdu->ini_free[i] == ini) {
				ampdu->ini_free[i] = NULL;
				break;
			}
		}
		ASSERT(i != AMPDU_INI_FREE);
		MFREE(wlc->osh, ini, sizeof(scb_ampdu_tid_ini_t));
		return;
	}

	ASSERT(ini->bar_ackpending);
	ini->bar_ackpending = FALSE;

	/* ack received */
	if (txstatus & TX_STATUS_ACK_RCV) {
		for (indx = TX_SEQ_TO_INDEX(ini->start_seq),
			endindex = TX_SEQ_TO_INDEX(ini->bar_ackpending_seq);
			indx != endindex;
			indx = NEXT_TX_INDEX(indx)) {

			if (isset(ini->barpending, indx)) {
				clrbit(ini->barpending, indx);
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					ini->txretry[indx] = 0;
					ini->tx_in_transit--;
				}
			}
		}

		/* bump up the start seq to move the window */
		wlc_ampdu_ini_move_window(ampdu, SCB_AMPDU_CUBBY(ampdu, ini->scb), ini);
	}

	wlc_ampdu_txflowcontrol(wlc, SCB_AMPDU_CUBBY(ampdu, ini->scb), ini);


	wlc_ampdu_send_bar(wlc->ampdu, ini, FALSE);
}

/* function to process tx completion for a reg mpdu */
void
wlc_ampdu_dotxstatus_regmpdu(ampdu_info_t *ampdu, struct scb *scb, void *p, tx_status_t *txs)
{
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 indx, seq;

	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

	if (!scb || !SCB_AMPDU(scb))
		return;

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[PKTPRIO(p)];
	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_ERR(("wl%d: ampdu_dotxstatus_regmpdu: NULL ini or pon state for tid %d\n",
			ampdu->wlc->pub->unit, PKTPRIO(p)));
		return;
	}
	ASSERT(ini->scb == scb);

	seq = pkt_txh_seqnum(ampdu->wlc, p);

	if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
		WL_ERROR(("wl%d: ampdu_dotxstatus_regmpdu: unexpected completion: seq 0x%x, "
			"start seq 0x%x\n",
			ampdu->wlc->pub->unit, seq, ini->start_seq));
		return;
	}

	indx = TX_SEQ_TO_INDEX(seq);
	if (!isset(ini->ackpending, indx)) {
		WL_ERROR(("wl%d: ampdu_dotxstatus_regmpdu: seq 0x%x\n",
			ampdu->wlc->pub->unit, seq));
		ampdu_dump_ini(ini);
		ASSERT(isset(ini->ackpending, indx));
		return;
	}

	if (txs->status & TX_STATUS_ACK_RCV) {
		clrbit(ini->ackpending, indx);
		if (isset(ini->barpending, indx)) {
			clrbit(ini->barpending, indx);
		}
		ini->txretry[indx] = 0;
		ini->tx_in_transit--;
		wlc_ampdu_ini_move_window(ampdu, SCB_AMPDU_CUBBY(ampdu, ini->scb), ini);
	} else {
		WLCNTINCR(ampdu->cnt->txreg_noack);
		AMPDUSCBCNTINCR(scb_ampdu->txreg_noack);
		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus_regmpdu: ack not recd for "
			"seq 0x%x tid %d status 0x%x\n",
			ampdu->wlc->pub->unit, seq, ini->tid, txs->status));
		/* suppressed pkts are resent; so dont move window */
		if (AP_ENAB(ampdu->wlc->pub) &&
			(txs->status & TX_STATUS_SUPR_MASK) == TX_STATUS_SUPR_PMQ) {
			ini->txretry[indx]++;
			ASSERT(ini->retry_cnt < AMPDU_TX_BA_MAX_WSIZE);
			ASSERT(ini->retry_seq[ini->retry_tail] == 0);
			ini->retry_seq[ini->retry_tail] = seq;
			ini->retry_tail = NEXT_TX_INDEX(ini->retry_tail);
			ini->retry_cnt++;

			return;
		}

		/* send bar to move the window */
		setbit(ini->barpending, indx);
		wlc_ampdu_send_bar(ampdu, ini, FALSE);
	}

	/* Check whether the packets on tx side is less than watermark level and disable flow */
	wlc_ampdu_txflowcontrol(ampdu->wlc, scb_ampdu, ini);
}

static void
wlc_ampdu_free_chain(ampdu_info_t *ampdu, void *p, tx_status_t *txs, bool txs3_done)
{
	wlc_info_t *wlc = ampdu->wlc;
	d11txh_t *txh;
	uint16 mcl;
	uint8 queue;
#ifdef WLAMPDU_MAC
	uint16 ncons = 0, count = 0;
	ncons = txs->sequence;
#endif

	WL_AMPDU_TX(("wl%d: wlc_ampdu_free_chain: free ampdu chain\n", wlc->pub->unit));

	queue = txs->frameid & TXFID_QUEUE_MASK;

	/* loop through all pkts in the dma ring and free */
	while (p) {

		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		mcl = ltoh16(txh->MacTxControlLow);

		WLCNTINCR(ampdu->cnt->orphan);
		PKTFREE(wlc->osh, p, TRUE);

		if (AMPDU_HOST_ENAB(wlc->pub)) {
			/* if not the last ampdu, take out the next packet from dma chain */
			if (((mcl & TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT) == TXC_AMPDU_LAST)
				break;
		} else {
#ifdef WLAMPDU_MAC
			/* if ucode/hw agg, ncons marks where we stop */
			wlc_txfifo_complete(wlc, queue, 1);
			count ++;
			if (count == ncons)
				break;
#endif
		}

		p = GETNEXTTXP(wlc, queue);
	}

	if (AMPDU_HOST_ENAB(wlc->pub))
		wlc_txfifo_complete(wlc, queue, ampdu->txpkt_weight);

#ifdef WLC_LOW
	/* retrieve the next status if it is there */
	if (txs->status & TX_STATUS_ACK_RCV) {
		uint32 s1;
		uint8 status_delay = 0;

		ASSERT(txs->status & TX_STATUS_INTERMEDIATE);

		/* wait till the next 8 bytes of txstatus is available */
		while (((s1 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V)
		       == 0) {
			OSL_DELAY(1);
			status_delay++;
			if (status_delay > 10) {
				ASSERT(0);
				return;
			}
		}

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		ASSERT(s1 & TX_STATUS_AMPDU);

#ifdef WLAMPDU_HW
		if (!txs3_done && AMPDU_HW_ENAB(wlc->pub)) {
			/* yet another txs to retrieve */
			status_delay = 0;
			while (((s1 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
				OSL_DELAY(1);
				status_delay++;
				if (status_delay > 10) {
					ASSERT(0);
					return;
				}
			}
		}
#endif /* WLAMPDU_HW */
	}
#endif /* WLC_LOW */
}

bool BCMFASTPATH
wlc_ampdu_dotxstatus(ampdu_info_t *ampdu, struct scb *scb, void *p, tx_status_t *txs)
{
	scb_ampdu_t *scb_ampdu;
	wlc_info_t *wlc = ampdu->wlc;
	scb_ampdu_tid_ini_t *ini;
	uint32 s1 = 0, s2 = 0, s3 = 0, s4 = 0;
#ifdef WLC_LOW
	/* For low driver, we don't want this function to return TRUE for certain err conditions
	 * returning true will cause a wl_init() to get called.
	 * we want to reserve wl_init() call for hw related err conditions.
	 */
	bool need_reset = FALSE;
#else
	/* For high only driver, we need to return TRUE for all error conditions. */
	bool need_reset = TRUE;
#endif /* WLC_LOW */

	ASSERT(p);
	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);
	ASSERT(txs->status & TX_STATUS_AMPDU);

#ifdef WLAMPDU_MAC
	/* update aggfifo/side_channel upfront in case we bail out early */
	if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
		ampdumac_info_t *hagg;
		uint16 max_entries = 0;
		uint16 queue, ncons;

		queue = txs->frameid & TXFID_QUEUE_MASK;
		ASSERT(queue < AC_COUNT);
		hagg = &(ampdu->hagg[queue]);
		ncons = txs->sequence;
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
			max_entries = ampdu->aggfifo_depth + 1;
		} else
#endif
		{
#ifdef WLAMPDU_UCODE
			max_entries = hagg->txfs_addr_end - hagg->txfs_addr_strt + 1;
			hagg->txfs_rptr += ncons;
			if (hagg->txfs_rptr > hagg->txfs_addr_end)
				hagg->txfs_rptr -= max_entries;
#endif
		}
		ASSERT(ncons > 0 && ncons < max_entries && ncons <= hagg->in_queue);
		hagg->in_queue -= ncons;
	}
#endif /* WLAMPDU_MAC */

	if (!scb || !SCB_AMPDU(scb)) {
		if (!scb)
			WL_ERROR(("wl%d: wlc_ampdu_dotxstatus: scb is null\n",
				wlc->pub->unit));

		wlc_ampdu_free_chain(ampdu, p, txs, FALSE);
		return need_reset;
	}

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[(uint8)PKTPRIO(p)];

	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		wlc_ampdu_free_chain(ampdu, p, txs, FALSE);
		return need_reset;
	}

	ASSERT(ini->scb == scb);
	ASSERT(WLPKTTAGSCBGET(p) == scb);

	/* BMAC_NOTE: For the split driver, second level txstatus comes later
	 * So if the ACK was received then wait for the second level else just
	 * call the first one
	 */
	if (txs->status & TX_STATUS_ACK_RCV) {
#ifdef WLC_LOW
		uint8 status_delay = 0;

		scb->used = wlc->pub->now;
		/* wait till the next 8 bytes of txstatus is available */
		while (((s1 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
			OSL_DELAY(1);
			status_delay++;
			if (status_delay > 10) {
				ASSERT(0);
				wlc_ampdu_free_chain(ampdu, p, txs, FALSE);
				return TRUE;
			}
		}

		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: 2nd status in delay %d\n",
			wlc->pub->unit, status_delay));

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		ASSERT(s1 & TX_STATUS_AMPDU);

		s2 = R_REG(wlc->osh, &wlc->regs->frmtxstatus2);

#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub)) {
			/* wait till the next 8 bytes of txstatus is available */
			status_delay = 0;
			while (((s3 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
				OSL_DELAY(1);
				status_delay++;
				if (status_delay > 10) {
					ASSERT(0);
					wlc_ampdu_free_chain(ampdu, p, txs, TRUE);
					return TRUE;
				}
			}
			s4 = R_REG(wlc->osh, &wlc->regs->frmtxstatus2);
		}
#endif /* WLAMPDU_HW */
#else
		scb->used = wlc->pub->now;

		/* Store the relevant information in ampdu structure */
		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: High Recvd \n", wlc->pub->unit));

		ASSERT(!ampdu->p);
		ampdu->p = p;
		bcopy(txs, &ampdu->txs, sizeof(tx_status_t));
		ampdu->waiting_status = TRUE;
		return FALSE;
#endif /* WLC_LOW */
	}

	wlc_ampdu_dotxstatus_complete(ampdu, scb, p, txs, s1, s2, s3, s4);
	wlc_ampdu_txflowcontrol(wlc, scb_ampdu, ini);

	return FALSE;
}

#ifdef WLC_HIGH_ONLY
void
wlc_ampdu_txstatus_complete(ampdu_info_t *ampdu, uint32 s1, uint32 s2)
{
	WL_AMPDU_TX(("wl%d: wlc_ampdu_txstatus_complete: High Recvd 0x%x 0x%x p:%p\n",
	          ampdu->wlc->pub->unit, s1, s2, ampdu->p));

	ASSERT(ampdu->waiting_status);

	/* The packet may have been freed if the SCB went away, if so, then still free the
	 * DMA chain
	 */
	if (ampdu->p) {
		wlc_ampdu_dotxstatus_complete(ampdu, WLPKTTAGSCBGET(ampdu->p), ampdu->p,
			&ampdu->txs, s1, s2, 0, 0);
		ampdu->p = NULL;
	}

	ampdu->waiting_status = FALSE;
}
#endif /* WLC_HIGH_ONLY */

static void BCMFASTPATH
wlc_ampdu_dotxstatus_complete(ampdu_info_t *ampdu, struct scb *scb, void *p, tx_status_t *txs,
	uint32 s1, uint32 s2, uint32 s3, uint32 s4)
{
	scb_ampdu_t *scb_ampdu;
	wlc_info_t *wlc = ampdu->wlc;
	scb_ampdu_tid_ini_t *ini;
	uint8 bitmap[8], queue, tid, requeue = 0;
	d11txh_t *txh;
	uint8 *plcp;
	struct dot11_header *h;
	uint16 seq, start_seq = 0, bindex, indx, mcl;
	uint8 mcs = 0, mcs_fbr = 0;
	bool ba_recd = FALSE, ack_recd = FALSE, send_bar = FALSE, vrate_probe = FALSE;
	uint8 suc_mpdu = 0, tot_mpdu = 0;
	uint supr_status;
	bool update_rate = TRUE, retry = TRUE;
	int txretry = -1;
	uint16 mimoantsel = 0;
	uint8 antselid = 0;
	uint8 retry_limit, rr_retry_limit;
	wlc_bsscfg_t *bsscfg;
	uint8 ac;
	bool pkt_freed = FALSE;
#ifdef WLPKTDLYSTAT
	uint32 delay, now;
	wlc_scb_delay_stats_t *delay_stats;
#endif

#ifdef WLAMPDU_MAC
	/* used for ucode/hw agg */
	int pdu_count = 0;
	uint16 first_frameid = 0xffff, fid, ncons = 0;
	uint8 tx_mrt = 0, txsucc_mrt = 0, tx_fbr = 0, txsucc_fbr = 0;
	uint16 txh_rr = 0;
#endif /* WLAMPDU_MAC */

#ifdef BCMDBG
	uint8 hole[AMPDU_MAX_MPDU];
	uint8 idx = 0;
	bzero(hole, sizeof(hole));
#endif

	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);
	ASSERT(txs->status & TX_STATUS_AMPDU);

	if (!scb || !SCB_AMPDU(scb)) {
		if (!scb)
			WL_ERROR(("wl%d: wlc_ampdu_dotxstatus: scb is null\n",
				wlc->pub->unit));

		wlc_ampdu_free_chain(ampdu, p, txs, FALSE);
		return;
	}

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	tid = (uint8)PKTPRIO(p);
	ac = WME_PRIO2AC(tid);
#ifdef WLPKTDLYSTAT
	delay_stats = &scb->delay_stats[ac];
#endif
	ini = scb_ampdu->ini[tid];
	retry_limit = ampdu->retry_limit_tid[tid];
	rr_retry_limit = ampdu->rr_retry_limit_tid[tid];

	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		wlc_ampdu_free_chain(ampdu, p, txs, FALSE);
		return;
	}

	ASSERT(ini->scb == scb);
	ASSERT(WLPKTTAGSCBGET(p) == scb);

	bzero(bitmap, sizeof(bitmap));
	queue = txs->frameid & TXFID_QUEUE_MASK;
	ASSERT(queue < AC_COUNT);

	update_rate = (WLPKTTAG(p)->flags & WLF_RATE_AUTO) ? TRUE : FALSE;
	supr_status = txs->status & TX_STATUS_SUPR_MASK;

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

#if WLAMPDU_HW
	if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
		WL_AMPDU_HWDBG(("%s: cons %d burst 0x%04x aggfifo1_space %d txfifo_cnt %d\n",
			__FUNCTION__, txs->sequence, (s4 >> 16) & 0xffff, /* s4 is overloaded */
			get_aggfifo_space(ampdu, 1),
			R_REG(ampdu->wlc->osh, &ampdu->wlc->regs->xmtfifo_frame_cnt)));
	}
#endif
	if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
		if (txs->status & TX_STATUS_ACK_RCV) {
			/*
			 * Underflow status is reused for BTCX to indicate AMPDU preemption.
			 * This prevents un-necessary rate fallback.
			 */
			if (TX_STATUS_SUPR_UF == supr_status) {
				WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: BT preemption, skip rate "
					     "fallback\n", wlc->pub->unit));
				update_rate = FALSE;
			}

#ifdef WLP2P
			if (P2P_ENAB(wlc->pub) &&
			    TX_STATUS_SUPR_NACK_ABS == supr_status) {
				WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: absence period start,"
				             "skip rate fallback\n", wlc->pub->unit));
				update_rate = FALSE;
			}
#endif

			ASSERT(txs->status & TX_STATUS_INTERMEDIATE);
			start_seq = txs->sequence >> SEQNUM_SHIFT;
			bitmap[0] = (txs->status & TX_STATUS_BA_BMAP03_MASK) >>
				TX_STATUS_BA_BMAP03_SHIFT;

			ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
			ASSERT(s1 & TX_STATUS_AMPDU);

			bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
			bitmap[1] = (s1 >> 8) & 0xff;
			bitmap[2] = (s1 >> 16) & 0xff;
			bitmap[3] = (s1 >> 24) & 0xff;

			/* remaining 4 bytes in s2 */
			bitmap[4] = s2 & 0xff;
			bitmap[5] = (s2 >> 8) & 0xff;
			bitmap[6] = (s2 >> 16) & 0xff;
			bitmap[7] = (s2 >> 24) & 0xff;

			ba_recd = TRUE;

			WL_AMPDU_TX(("%s: Block ack received: start_seq is 0x%x bitmap "
				"%02x %02x %02x %02x %02x %02x %02x %02x\n",
				__FUNCTION__, start_seq, bitmap[0], bitmap[1], bitmap[2],
				bitmap[3], bitmap[4], bitmap[5], bitmap[6], bitmap[7]));

		}
	}
#ifdef WLAMPDU_MAC
	else {
		/* AMPDU_HW txstatus package */

		ASSERT(txs->status & TX_STATUS_INTERMEDIATE);

		if (WL_AMPDU_HWTXS_ON()) {
			wlc_print_ampdu_txstatus(ampdu, txs, s1, s2, s3, s4);
		}

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		start_seq = ini->start_seq;
		ncons = txs->sequence;
		bitmap[0] = (txs->status & TX_STATUS_BA_BMAP03_MASK) >> TX_STATUS_BA_BMAP03_SHIFT;
		bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
		bitmap[1] = (s1 >> 8) & 0xff;
		bitmap[2] = (s1 >> 16) & 0xff;
		bitmap[3] = (s1 >> 24) & 0xff;
		ba_recd = bitmap[0] || bitmap[1] || bitmap[2] || bitmap[3];
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
			/* 3rd txstatus */
			bitmap[4] = (s3 >> 16) & 0xff;
			bitmap[5] = (s3 >> 24) & 0xff;
			bitmap[6] = s4 & 0xff;
			bitmap[7] = (s4 >> 8) & 0xff;
			ba_recd = ba_recd || bitmap[4] || bitmap[5] || bitmap[6] || bitmap[7];
		}
#endif
		/* remaining 4 bytes in s2 */
		tx_mrt 		= (uint8)(s2 & 0xff);
		txsucc_mrt 	= (uint8)((s2 >> 8) & 0xff);
		tx_fbr 		= (uint8)((s2 >> 16) & 0xff);
		txsucc_fbr 	= (uint8)((s2 >> 24) & 0xff);

		WLCNTINCR(ampdu->cnt->u0.txs);
		WLCNTADD(ampdu->cnt->tx_mrt, tx_mrt);
		WLCNTADD(ampdu->cnt->txsucc_mrt, txsucc_mrt);
		WLCNTADD(ampdu->cnt->tx_fbr, tx_fbr);
		WLCNTADD(ampdu->cnt->txsucc_fbr, txsucc_fbr);

		if ((txs->status & (TX_STATUS_ACK_RCV | TX_STATUS_VALID)) !=
			(TX_STATUS_ACK_RCV|TX_STATUS_VALID)) {
			WL_ERROR(("Status: "));
			if ((txs->status & TX_STATUS_ACK_RCV) == 0)
				WL_ERROR(("Not ACK_RCV "));
			if ((txs->status & TX_STATUS_VALID) == 0)
				WL_ERROR(("Not VALID "));
			WL_ERROR(("\n"));
			WL_ERROR(("Total attempts %d succ %d\n", tx_mrt, txsucc_mrt));
		}
	}
#endif /* WLAMPDU_MAC */

	if (!ba_recd || AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
		if (!ba_recd) {
			WLCNTINCR(ampdu->cnt->noba);
			AMPDUSCBCNTINCR(scb_ampdu->noba);
			if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
				WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: error txstatus 0x%x\n",
					wlc->pub->unit, txs->status));
			}
		}
		if (supr_status) {
			update_rate = FALSE;
#ifdef BCMDBG
			ampdu->supr_reason[supr_status >> TX_STATUS_SUPR_SHIFT]++;
#endif
			WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: supr_status 0x%x\n",
				wlc->pub->unit, supr_status));
			/* no need to retry for badch; will fail again */
			if (supr_status == TX_STATUS_SUPR_BADCH && !SCAN_IN_PROGRESS(wlc->scan)) {
				retry = FALSE;
				WLCNTINCR(wlc->pub->_cnt->txchanrej);
			} else if (supr_status == TX_STATUS_SUPR_EXPTIME) {

				WLCNTINCR(wlc->pub->_cnt->txexptime);

				/* Interference detected */
				if (wlc->rfaware_lifetime)
					wlc_exptime_start(wlc);
			/* TX underflow : try tuning pre-loading or ampdu size */
			} else if (supr_status == TX_STATUS_SUPR_FRAG) {
				/* if there were underflows, but pre-loading is not active,
				   notify rate adaptation.
				*/
				if (wlc_ffpld_check_txfunfl(wlc, prio2fifo[tid]) > 0) {
#ifdef WLC_HIGH_ONLY
					/* With BMAC, TX Underflows should not happen */
					WL_ERROR(("wl%d: BMAC TX Underflow?", wlc->pub->unit));
#endif
				}
			}
#ifdef WLP2P
			else if (P2P_ENAB(wlc->pub) &&
			         supr_status == TX_STATUS_SUPR_NACK_ABS) {
				/* We want to retry */
			}
#endif
		} else if (txs->phyerr) {
			update_rate = FALSE;
			WLCNTINCR(wlc->pub->_cnt->txphyerr);
			WL_ERROR(("wl%d: wlc_ampdu_dotxstatus: tx phy error (0x%x)\n",
				wlc->pub->unit, txs->phyerr));

#ifdef BCMDBG
			if (WL_ERROR_ON()) {
				prpkt("txpkt (AMPDU)", wlc->osh, p);
				if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
					wlc_print_txdesc((d11txh_t*)PKTDATA(wlc->osh, p));
					wlc_print_txstatus(txs);
				}
#ifdef WLAMPDU_MAC
				else {
					wlc_print_ampdu_txstatus(ampdu, txs, s1, s2, s3, s4);
					wlc_dump_aggfifo(ampdu->wlc, NULL);
				}
#endif /* WLAMPDU_MAC */
			}
#endif /* BCMDBG */
		}
	}

	/* Check if interference is still there */
	if (wlc->rfaware_lifetime && wlc->exptime_cnt && (supr_status != TX_STATUS_SUPR_EXPTIME))
		wlc_exptime_check_end(wlc);

#ifdef WLPKTDLYSTAT
	/* Get the current time */
	now = WLC_GET_CURR_TIME(wlc);
#endif

	/* loop through all pkts and retry if not acked */
	while (p) {
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		mcl = ltoh16(txh->MacTxControlLow);
		plcp = (uint8 *)(txh + 1);
		h = (struct dot11_header*)(plcp + D11_PHY_HDR_LEN);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;
		pkt_freed = FALSE;
#ifdef WLAMPDU_MAC
		fid = ltoh16(txh->TxFrameID);
#endif
		if (tot_mpdu == 0) {
			mcs = plcp[0] & MIMO_PLCP_MCS_MASK;
			mimoantsel = ltoh16(txh->ABI_MimoAntSel);
			if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
				/* this is only needed for host agg */
				vrate_probe = (WLPKTTAG(p)->flags & WLF_VRATE_PROBE) ? TRUE : FALSE;
				mcs_fbr = txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK;
			}
#ifdef WLAMPDU_MAC
			else {
				first_frameid = ltoh16(txh->TxFrameID);
				txh_rr = ltoh16(txh->MModeFbrLen);
#if defined(BCMDBG) || defined(WLTEST)
				if (PLCP3_ISSGI(plcp[3])) {
					WLCNTADD(ampdu->cnt->u1.txmpdu_sgi, tx_mrt);
					WLCNTADD(ampdu->txmcssgi[mcs], tx_mrt);
				}
				if (PLCP3_ISSTBC(plcp[3])) {
					WLCNTADD(ampdu->cnt->u2.txmpdu_stbc, tx_mrt);
					WLCNTADD(ampdu->txmcsstbc[mcs], tx_mrt);
				}
				WLCNTADD(ampdu->txmcs[mcs], tx_mrt);
				WLCNTADD(ampdu->cnt->txmpdu_cnt[mcs], tx_mrt);
				WLCNTADD(ampdu->cnt->txmpdu_succ_cnt[mcs], txsucc_mrt);
				if ((ltoh16(txh->XtraFrameTypes) & 0x3) != 0) {
					/* if not fbr_cck */
					mcs_fbr = txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK;
					WLCNTADD(ampdu->txmcs[mcs_fbr], tx_fbr);
					WLCNTADD(ampdu->cnt->txmpdu_cnt[mcs_fbr], tx_fbr);
					WLCNTADD(ampdu->cnt->txmpdu_succ_cnt[mcs_fbr], txsucc_fbr);
					if (PLCP3_ISSGI(txh->FragPLCPFallback[3])) {
						WLCNTADD(ampdu->cnt->u1.txmpdu_sgi, tx_fbr);
						WLCNTADD(ampdu->txmcssgi[mcs_fbr], tx_fbr);
					}
					if (PLCP3_ISSTBC(txh->FragPLCPFallback[3])) {
						WLCNTADD(ampdu->cnt->u2.txmpdu_stbc, tx_fbr);
						WLCNTADD(ampdu->txmcsstbc[mcs_fbr], tx_fbr);
					}
				}
#endif /* BCMDBG || WLTEST */
			}
#endif /* WLAMPDU_MAC */
		}

		indx = TX_SEQ_TO_INDEX(seq);
		if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
			txretry = ini->txretry[indx];
		}

		if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
			WL_ERROR(("wl%d: ampdu_dotxstatus: unexpected completion: seq 0x%x, "
				"start seq 0x%x\n",
				ampdu->wlc->pub->unit, seq, ini->start_seq));
			pkt_freed = TRUE;
#ifdef WLAMPDU_MAC
			wlc_print_ampdu_txstatus(ampdu, txs, s1, s2, s3, s4);
#endif
			goto nextp;
		}

		if (!isset(ini->ackpending, indx)) {
			WL_ERROR(("wl%d: ampdu_dotxstatus: seq 0x%x\n",
				ampdu->wlc->pub->unit, seq));
			ampdu_dump_ini(ini);
			ASSERT(isset(ini->ackpending, indx));
		}

		ack_recd = FALSE;
		if (ba_recd || AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
			if (AMPDU_MAC_ENAB(ampdu->wlc->pub))
				/* use position bitmap */
				bindex = tot_mpdu;
			else
				bindex = MODSUB_POW2(seq, start_seq, SEQNUM_MAX);

			WL_AMPDU_TX(("%s: tid %d seq is 0x%x, start_seq is 0x%x, "
			          "bindex is %d set %d, txretry %d, index %d\n",
			          __FUNCTION__, tid, seq, start_seq, bindex,
			          isset(bitmap, bindex), txretry, indx));

			/* if acked then clear bit and free packet */
			if ((bindex < AMPDU_BA_BITS_IN_BITMAP) && isset(bitmap, bindex)) {
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					if (isset(ini->barpending, indx)) {
						clrbit(ini->barpending, indx);
					}
					ini->txretry[indx] = 0;
					ini->tx_in_transit--;
				}
#ifdef WLLMAC
				if (LMAC_ENAB(wlc->pub)) {
					/* LMAC handles txstatus as per the LMAC-UMAC protocol */
					wlc_lmac_txstatus(wlc->lmac_info, p, txs, supr_status,
						txretry, 0, FALSE);
				}
#endif /* WLLMAC */
				/* call any matching pkt callbacks */
				if (WLPKTTAG(p)->callbackidx) {
					wlc_pkt_callback((void *)wlc, p, TX_STATUS_ACK_RCV);
				}
#ifdef WLTXMONITOR
				if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub))
					wlc_tx_monitor(wlc, txh, txs, p, NULL);
#endif
				pkt_freed = TRUE;
				ack_recd = TRUE;
				suc_mpdu++;
			}
		}
		/* either retransmit or send bar if ack not recd */
		if (!ack_recd) {
			WLPKTTAGSCBSET(p, scb);
			if (AMPDU_HOST_ENAB(ampdu->wlc->pub) &&
			    retry && (txretry < (int)retry_limit)) {

				/* Set retry bit */
				h->fc |= htol16(FC_RETRY);

				/* bump up retry cnt; for rate probe, go directly to fbr */
				if (ampdu->probe_mpdu && vrate_probe &&
				    (ini->txretry[indx] < rr_retry_limit))
					ini->txretry[indx] = rr_retry_limit;
				else
					ini->txretry[indx]++;
				requeue++;
				ASSERT(ini->retry_cnt < AMPDU_TX_BA_MAX_WSIZE);
				ASSERT(ini->retry_seq[ini->retry_tail] == 0);
				ini->retry_seq[ini->retry_tail] = seq;
				ini->retry_tail = NEXT_TX_INDEX(ini->retry_tail);
				ini->retry_cnt++;

				SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_HI_PREC(tid));
			} else {
				setbit(ini->barpending, indx);
				send_bar = TRUE;
#ifdef WLLMAC
				if (LMAC_ENAB(wlc->pub)) {
					/* LMAC handles txstatus as per the LMAC-UMAC protocol */
					wlc_lmac_txstatus(wlc->lmac_info, p, txs, 0, retry_limit,
						0, TRUE);
				}
#endif /* WLLMAC */
#ifdef WLNINTENDO2
				if (WLNIN2_ENAB(ampdu->wlc->pub)) {
					wlc_bss_mac_event(wlc, bsscfg, WLC_E_TXFAIL,
						(struct ether_addr *)txh->TxFrameRA,
						WLC_E_STATUS_TIMEOUT, 0, 0, 0, 0);
				}
#endif
#ifdef WLTXMONITOR
				if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub))
					wlc_tx_monitor(wlc, txh, txs, p, NULL);
#endif
				pkt_freed = TRUE;
			}
		} else if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
			ba_recd = TRUE;
		}
#ifdef BCMDBG
		/* tot_mpdu may exceeds AMPDU_MAX_MPDU if doing ucode/hw agg */
		if (AMPDU_HOST_ENAB(ampdu->wlc->pub) && ba_recd && !ack_recd)
			hole[idx]++;
#endif

nextp:
#ifdef BCMDBG
		if ((idx = tot_mpdu) >= AMPDU_MAX_MPDU) {
			WL_ERROR(("%s: idx out-of-bound to array size (%d)\n", __FUNCTION__, idx));
			idx = AMPDU_MAX_MPDU - 1;
		}
#endif
		tot_mpdu++;

		/* calculate latency and packet loss statistics */
		if (pkt_freed) {

#ifdef WLPKTDLYSTAT
			/* Ignore wrap-around case and error case */
			if (now > WLPKTTAG(p)->shared.enqtime) {
				uint tr;
				if (AMPDU_HOST_ENAB(ampdu->wlc->pub))
					tr = (txretry >= AMPDU_DEF_RETRY_LIMIT) ?
						AMPDU_DEF_RETRY_LIMIT : txretry;
				else
					tr = 0;

				delay = now - WLPKTTAG(p)->shared.enqtime;
				wlc_delay_stats_upd(delay_stats, delay, tr, ack_recd);
				WL_AMPDU_STAT(("Enq %d retry %d cnt %d: acked %d delay/min/"
					"max/sum %d %d %d %d\n", WLPKTTAG(p)->shared.enqtime,
					 tr, delay_stats->txmpdu_cnt[tr], ack_recd, delay,
					delay_stats->delay_min, delay_stats->delay_max,
					delay_stats->delay_sum[tr]));
			}
#endif /* WLPKTDLYSTAT */

			PKTFREE(wlc->osh, p, TRUE);
		}

		/* break out if last packet of ampdu */
		if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
#ifdef WLAMPDU_MAC
			WLCNTDECR(ampdu->cnt->pending);
			WLCNTINCR(ampdu->cnt->cons);

			wlc_txfifo_complete(wlc, queue, 1);
			if (tot_mpdu == ncons) {
				ASSERT(txs->frameid == fid);
				break;
			}
			if (++pdu_count >= AMPDU_TX_BA_MAX_WSIZE) {
				WL_ERROR(("%s: Reach max num of MPDU without finding frameid\n",
					__FUNCTION__));
				ASSERT(pdu_count >= AMPDU_TX_BA_MAX_WSIZE);
			}
#endif /* WLAMPDU_MAC */
		} else {
			if (((mcl & TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT) == TXC_AMPDU_LAST)
				break;
		}

		p = GETNEXTTXP(wlc, queue);
		/* For external use big hammer to restore the sync */
		if (p == NULL) {
#ifdef WLAMPDU_MAC
			WL_ERROR(("%s: p is NULL. tot_mpdu %d suc %d pdu %d first_frameid %x "
				"fifocnt %d\n", __FUNCTION__, tot_mpdu, suc_mpdu, pdu_count,
				first_frameid, R_REG(wlc->osh, &wlc->regs->xmtfifo_frame_cnt)));
			wlc_print_ampdu_txstatus(ampdu, txs, s1, s2, s3, s4);
			wlc_dump_aggfifo(ampdu->wlc, NULL);
#endif
			ASSERT(p);
			break;
		}
	}

#ifdef BCMDBG
	/* post-process the statistics */
	if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
		int i, j;
		ampdu->mpdu_histogram[idx]++;
		for (i = 0; i < idx; i++) {
			if (hole[i])
				ampdu->retry_histogram[i]++;
			if (hole[i]) {
				for (j = i + 1; (j < idx) && hole[j]; j++)
					;
				if (j == idx)
					ampdu->end_histogram[i]++;
			}
		}

		if (txretry < rr_retry_limit)
			WLCNTADD(ampdu->cnt->txmpdu_succ_cnt[mcs], suc_mpdu);
		else
			WLCNTADD(ampdu->cnt->txmpdu_succ_cnt[mcs_fbr], suc_mpdu);
	}
#endif /* BCMDBG */

	/* send bar to move the window */
	if (send_bar)
		wlc_ampdu_send_bar(ampdu, ini, FALSE);

	/* update rate state */
	if (WLANTSEL_ENAB(wlc))
		antselid = wlc_antsel_antsel2id(wlc->asi, mimoantsel);

#ifdef WLAMPDU_MAC
	WL_AMPDU_HWDBG(("%s: consume %d txfifo_cnt %d\n", __FUNCTION__, tot_mpdu,
		R_REG(ampdu->wlc->osh, &ampdu->wlc->regs->xmtfifo_frame_cnt)));

	if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
		if (tx_mrt + tx_fbr == 0) {
			ASSERT(txsucc_mrt + txsucc_fbr == 0);
			if (txsucc_mrt + txsucc_fbr != 0) {
				printf("%s: wrong condition\n", __FUNCTION__);
				wlc_print_ampdu_txstatus(ampdu, txs, s1, s2, s3, s4);
			}
			if (ba_recd)
				update_rate = FALSE;
			else if (WL_AMPDU_ERR_ON()) {
				wlc_print_ampdu_txstatus(ampdu, txs, s1, s2, s3, s4);
			}
		}

		if (update_rate)
			wlc_ratesel_upd_txs_ampdu(wlc->rsi, scb, first_frameid,
				tx_mrt, txsucc_mrt, tx_fbr, txsucc_fbr,
				FALSE, mcs, antselid);
	} else
#endif /* WLAMPDU_MAC */
		if (update_rate) {
			wlc_ratesel_upd_txs_blockack(wlc->rsi, scb,
				txs, suc_mpdu, tot_mpdu, !ba_recd, (uint8)txretry,
				rr_retry_limit, FALSE, mcs & MIMO_PLCP_MCS_MASK, antselid);
	}

	WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: requeueing %d packets\n",
		wlc->pub->unit, requeue));

	/* Call only once per AMPDU for SW agg */
	if (AMPDU_HOST_ENAB(ampdu->wlc->pub))
		wlc_txfifo_complete(wlc, queue, ampdu->txpkt_weight);

	/* bump up the start seq to move the window */
	wlc_ampdu_ini_move_window(ampdu, scb_ampdu, ini);

#ifdef STA
	/* PM state change */
	wlc_update_pmstate(bsscfg, txs->status);
#endif /* STA */
}

#ifdef WLAMPDU_MAC
static void
wlc_print_ampdu_txstatus(ampdu_info_t *ampdu,
	tx_status_t *pkg1, uint32 s1, uint32 s2, uint32 s3, uint32 s4)
{
	printf("%s: txstatus 0x%04x\n", __FUNCTION__, pkg1->status);
	if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
		uint8 bitmap[8];
		uint16 txcnt_mrt, succ_mrt, txcnt_fbr, succ_fbr;
		uint16 ncons = pkg1->sequence;

		bitmap[0] = (pkg1->status & TX_STATUS_BA_BMAP03_MASK) >> TX_STATUS_BA_BMAP03_SHIFT;
		bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
		bitmap[1] = (s1 >> 8) & 0xff;
		bitmap[2] = (s1 >> 16) & 0xff;
		bitmap[3] = (s1 >> 24) & 0xff;
		txcnt_mrt = s2 & 0xff;
		succ_mrt = (s2 >> 8) & 0xff;
		txcnt_fbr = (s2 >> 16) & 0xff;
		succ_fbr = (s2 >> 24) & 0xff;
		printf("\t\t ncons %d last frameid: 0x%x\n", ncons, pkg1->frameid);
		printf("\t\t txcnt: mrt %2d succ %2d fbr %2d succ %2d\n",
		       txcnt_mrt, succ_mrt, txcnt_fbr, succ_fbr);

		if (AMPDU_UCODE_ENAB(ampdu->wlc->pub))
			printf("\t\t bitmap: %02x %02x %02x %02x\n",
			       bitmap[0], bitmap[1], bitmap[2], bitmap[3]);
		else {
			bitmap[4] = (s3 >> 16) & 0xff;
			bitmap[5] = (s3 >> 24) & 0xff;
			bitmap[6] = s4 & 0xff;
			bitmap[7] = (s4 >> 8) & 0xff;
			printf("\t\t bitmap: %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       bitmap[0], bitmap[1], bitmap[2], bitmap[3],
			       bitmap[4], bitmap[5], bitmap[6], bitmap[7]);
			printf("\t\t timestamp 0x%04x\n", (s4 >> 16) & 0xffff);
		}
#ifdef WLAMPDU_UCODE
		if (AMPDU_UCODE_ENAB(ampdu->wlc->pub) && txcnt_mrt + txcnt_fbr == 0) {
			uint16 strt, end, wptr, rptr, rnum, qid;
			uint16 base = (wlc_read_shm(ampdu->wlc, M_TXFS_PTR) + C_TXFSD_WOFFSET) * 2;
			qid = pkg1->frameid & TXFID_QUEUE_MASK;
			strt = wlc_read_shm(ampdu->wlc, C_TXFSD_STRT_POS(base, qid));
			end = wlc_read_shm(ampdu->wlc, C_TXFSD_END_POS(base, qid));
			wptr = wlc_read_shm(ampdu->wlc, C_TXFSD_WPTR_POS(base, qid));
			rptr = wlc_read_shm(ampdu->wlc, C_TXFSD_RPTR_POS(base, qid));
			rnum = wlc_read_shm(ampdu->wlc, C_TXFSD_RNUM_POS(base, qid));
			printf("%s: side channel %d ---- \n", __FUNCTION__, qid);
			printf("\t\t strt : shm(0x%x) = 0x%x\n", C_TXFSD_STRT_POS(base, qid), strt);
			printf("\t\t end  : shm(0x%x) = 0x%x\n", C_TXFSD_END_POS(base, qid), end);
			printf("\t\t wptr : shm(0x%x) = 0x%x\n", C_TXFSD_WPTR_POS(base, qid), wptr);
			printf("\t\t rptr : shm(0x%x) = 0x%x\n", C_TXFSD_RPTR_POS(base, qid), rptr);
			printf("\t\t rnum : shm(0x%x) = 0x%x\n", C_TXFSD_RNUM_POS(base, qid), rnum);
		}
#endif /* WLAMPDU_UCODE */
	}
}

void
wlc_dump_aggfifo(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	wlc_ampdu_cnt_t* cnt;
	int i;
	d11regs_t *regs;
	regs = wlc->regs;
	cnt = wlc->ampdu->cnt;

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(wlc->pub)) {
		ampdu_info_t* ampdu = wlc->ampdu;
		printf("%s:\n", __FUNCTION__);
		for (i = 0; i < 4; i++) {
			int k = 0, addr;
			printf("fifo %d: rptr %x wptr %x\n",
			       i, ampdu->hagg[i].txfs_rptr, ampdu->hagg[i].txfs_wptr);
			for (addr = ampdu->hagg[i].txfs_addr_strt;
			     addr <= ampdu->hagg[i].txfs_addr_end; addr++) {
				printf("\tentry %d addr 0x%x: 0x%x\n",
				       k, addr, wlc_read_shm(ampdu->wlc, addr * 2));
				k++;
			}
		}
	}
#endif /* WLAMPDU_UCODE */

#if defined(WLCNT) && defined(WLAMPDU_MAC)
	printf("driver statistics: aggfifo pending %d enque/cons %d %d\n",
	       cnt->pending, cnt->enq, cnt->cons);
#endif

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		printf("AGGFIFO regs: availcnt 0x%x txfifo fr_count %d by_count %d\n",
		       R_REG(wlc->osh, &regs->aggfifocnt),
		       R_REG(wlc->osh, &regs->xmtfifo_frame_cnt),
		       R_REG(wlc->osh, &regs->xmtfifo_byte_cnt));
		printf("cmd 0x%04x stat 0x%04x cfgctl 0x%04x cfgdata 0x%04x mpdunum 0x%02x"
		       " len 0x%04x bmp 0x%04x ackedcnt %d\n",
		       R_REG(wlc->osh, &regs->aggfifo_cmd),
		       R_REG(wlc->osh, &regs->aggfifo_stat),
		       R_REG(wlc->osh, &regs->aggfifo_cfgctl),
		       R_REG(wlc->osh, &regs->aggfifo_cfgdata),
		       R_REG(wlc->osh, &regs->aggfifo_mpdunum),
		       R_REG(wlc->osh, &regs->aggfifo_len),
		       R_REG(wlc->osh, &regs->aggfifo_bmp),
		       R_REG(wlc->osh, &regs->aggfifo_ackedcnt));

		/* aggfifo */
		printf("AGGFIFO dump: \n");
		for (i = 1; i < 2; i ++) {
			int j;
			printf("AGGFIFO %d:\n", i);
			for (j = 0; j < AGGFIFO_CAP; j ++) {
				uint16 entry;
				W_REG(wlc->osh, &regs->aggfifo_sel, (i << 6) | j);
				W_REG(wlc->osh, &regs->aggfifo_cmd, (6 << 2) | i);
				entry = R_REG(wlc->osh, &regs->aggfifo_data);
				if (j % 4 == 0) {
					printf("\tEntry 0x%02x: 0x%04x  ", j, entry);
				} else if (j % 4 == 3) {
					printf("0x%04x\n", entry);
				} else {
					printf("0x%04x  ", entry);
				}
			}
		}
	}
#endif /* WLAMPDU_HW */
}
#endif /* WLAMPDU_MAC */

/* AMPDUXXX: this has to be kept upto date as fields get added to f */
static void BCMFASTPATH
ampdu_create_f(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f, void *p,
	wlc_d11rxhdr_t *wrxh)
{
	uint16 offset = DOT11_A3_HDR_LEN;


	bzero((void *)f, sizeof(struct wlc_frminfo));
	f->p = p;
	f->h = (struct dot11_header *) PKTDATA(wlc->osh, f->p);
	f->fc = ltoh16(f->h->fc);
	f->type = FC_TYPE(f->fc);
	f->subtype = (f->fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT;
	f->ismulti = ETHER_ISMULTI(&(f->h->a1));
	f->len = PKTLEN(wlc->osh, f->p);
	f->seq = ltoh16(f->h->seq);
	f->ividx = 0;
	f->wds = ((f->fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
	if (f->wds)
		offset += ETHER_ADDR_LEN;
	f->pbody = (uchar*)(f->h) + offset;

	/* account for QoS Control Field */
	f->qos = (f->type == FC_TYPE_DATA && FC_SUBTYPE_ANY_QOS(f->subtype));
	if (f->qos) {
		uint16 qoscontrol = ltoh16_ua(f->pbody);
		f->isamsdu = (qoscontrol & QOS_AMSDU_MASK) != 0;
		f->prio = (uint8)QOS_PRIO(qoscontrol);
		f->ac = WME_PRIO2AC(f->prio);
		f->apsd_eosp = QOS_EOSP(qoscontrol);
		f->pbody += DOT11_QOS_LEN;
		offset += DOT11_QOS_LEN;
		f->ividx = (uint8)PRIO2IVIDX(f->prio);
		ASSERT(f->ividx < WLC_NUMRXIVS);
	}
	f->ht = ((wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
		((f->fc & FC_ORDER) && FC_SUBTYPE_ANY_QOS(f->subtype));
	if (f->ht) {
		f->pbody += DOT11_HTC_LEN;
		offset += DOT11_HTC_LEN;
	}

	f->body_len = f->len - offset;
	f->totlen = pkttotlen(wlc->osh, p) - offset;
	/* AMPDUXXX: WPA_auth may not be valid for wds */
	f->WPA_auth = scb->WPA_auth;
	f->wrxh = wrxh;
	f->rxh = &wrxh->rxhdr;
	f->rx_wep = 0;
	f->key = NULL;
}

void BCMFASTPATH
wlc_ampdu_recvdata(ampdu_info_t *ampdu, struct scb *scb, struct wlc_frminfo *f)
{
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;
	wlc_info_t *wlc;
	uint16 seq, offset, indx, delta;
	uint8 *plcp;
	uint8 tid = f->prio;

	wlc = ampdu->wlc;

	if (f->subtype != FC_SUBTYPE_QOS_DATA) {
		wlc_recvdata_ordered(wlc, scb, f);
		return;
	}

	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	ASSERT(tid < AMPDU_MAX_SCB_TID);
	ASSERT(!f->ismulti);

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	plcp = ((uint8 *)(f->h)) - D11_PHY_HDR_LEN;

	if ((f->rxh->PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) {
		if (WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
			WLCNTINCR(ampdu->cnt->rxampdu);
			WLCNTINCR(ampdu->cnt->rxmpdu);
			AMPDUSCBCNTINCR(scb_ampdu->rxampdu);
			AMPDUSCBCNTINCR(scb_ampdu->rxmpdu);
#if defined(BCMDBG) || defined(WLTEST)
			if ((plcp[0] & 0x3f) <= FFPLD_MAX_MCS)
				ampdu->rxmcs[plcp[0] & 0x3f]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
			if (PLCP3_ISSGI(plcp[3])) {
				WLCNTINCR(ampdu->cnt->rxampdu_sgi);
#if defined(BCMDBG) || defined(WLTEST)
				if ((plcp[0] & 0x3f) <= FFPLD_MAX_MCS)
					ampdu->rxmcssgi[plcp[0] & 0x1f]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
			}
			if (PLCP3_ISSTBC(plcp[3])) {
				WLCNTINCR(ampdu->cnt->rxampdu_stbc);
#if defined(BCMDBG) || defined(WLTEST)
				if ((plcp[0] & 0x3f) <= FFPLD_MAX_MCS)
					ampdu->rxmcsstbc[plcp[0] & 0x3f]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
			}
		} else if (!(plcp[0] | plcp[1] | plcp[2]))
			WLCNTINCR(ampdu->cnt->rxmpdu);
		else
			WLCNTINCR(ampdu->cnt->rxht);
	} else {
		WLCNTINCR(ampdu->cnt->rxlegacy);
		AMPDUSCBCNTINCR(scb_ampdu->rxlegacy);
	}

	resp = scb_ampdu->resp[tid];

	/* return if AMPDU not enabled on TID */
	if (resp == NULL) {
		if (((f->rxh->PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
			WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
			scb_ampdu->resp_off[tid].ampdu_recd = TRUE;
			WLCNTINCR(ampdu->cnt->rxnobapol);
		}
		wlc_recvdata_ordered(wlc, scb, f);
		return;
	}

	/* track if receiving aggregates from non-HT device */
	if (!SCB_HT_CAP(scb) &&
	    ((f->rxh->PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
	    WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
		scb_ampdu->resp_off[tid].ampdu_recd = TRUE;
		WLCNTINCR(ampdu->cnt->rxnobapol);
	}

	/* fragments not allowed */
	if (f->seq & FRAGNUM_MASK) {
		WL_ERROR(("wl%d: wlc_ampdu_recvdata: unexp frag seq 0x%x, exp seq 0x%x\n",
			wlc->pub->unit, f->seq, resp->exp_seq));
		goto toss;
	}

	seq = f->seq >> SEQNUM_SHIFT;
	indx = RX_SEQ_TO_INDEX(seq);

	WL_AMPDU_RX(("wl%d: %s: receiving seq 0x%x tid %d, exp seq %d\n",
	          wlc->pub->unit, __FUNCTION__, seq, tid, resp->exp_seq));

	/* send up if expected seq */
	if (seq == resp->exp_seq) {
		uint bandunit;
		struct ether_addr ea;
		struct scb *newscb;

		ASSERT(resp->exp_seq == pkt_h_seqnum(wlc, f->p));
		ASSERT(!resp->rxq[indx]);

		resp->alive = TRUE;
		resp->exp_seq = NEXT_SEQ(resp->exp_seq);

		bandunit = scb->bandunit;
		bcopy(&scb->ea, &ea, ETHER_ADDR_LEN);

		wlc_recvdata_ordered(wlc, scb, f);

		/* validate that the scb is still around and some path in
		 * wlc_recvdata_ordered() did not free it
		 */
		newscb = wlc_scbfindband(wlc, &ea, bandunit);
		if ((newscb == NULL) || (newscb != scb)) {
			WL_ERROR(("wl%d: %s: scb freed; bail out\n",
				wlc->pub->unit, __FUNCTION__));
			return;
		}

		/* release pending ordered packets */
		WL_AMPDU_TX(("wl%d: %s: Releasing pending packets\n",
		          wlc->pub->unit, __FUNCTION__));
		wlc_ampdu_release_ordered(wlc, scb_ampdu, tid);

		return;
	}

	/* out of order packet; validate and enq */

	offset = MODSUB_POW2(seq, resp->exp_seq, SEQNUM_MAX);

	/* check for duplicate or beyond half the sequence space */
	if (((offset < resp->ba_wsize) && resp->rxq[indx]) ||
		(offset > (SEQNUM_MAX >> 1))) {
		ASSERT(seq == pkt_h_seqnum(wlc, f->p));
		WL_AMPDU_RX(("wl%d: wlc_ampdu_recvdata: duplicate seq 0x%x(dropped)\n",
			wlc->pub->unit, seq));
		PKTFREE(wlc->osh, f->p, FALSE);
		WLCNTINCR(ampdu->cnt->rxdup);
		AMPDUSCBCNTINCR(scb_ampdu->rxdup);
		return;
	}

	/* move the start of window if acceptable out of window pkts */
	if (offset >= resp->ba_wsize) {
		delta = offset - resp->ba_wsize + 1;
		WL_AMPDU_RX(("wl%d: wlc_ampdu_recvdata: out of window pkt with"
			" seq 0x%x delta %d (exp seq 0x%x): moving window fwd\n",
			wlc->pub->unit, seq, delta, resp->exp_seq));

		wlc_ampdu_release_n_ordered(wlc, scb_ampdu, tid, delta);

		/* recalc resp since may have been freed while releasing frames */
		if ((resp = scb_ampdu->resp[tid])) {
			ASSERT(!resp->rxq[indx]);
			resp->rxq[indx] = f->p;
			resp->wrxh[indx] = f->wrxh;
			resp->queued++;
		}
		wlc_ampdu_release_ordered(wlc, scb_ampdu, tid);

		WLCNTINCR(ampdu->cnt->rxoow);
		AMPDUSCBCNTINCR(scb_ampdu->rxoow);
		return;
	}

	WL_AMPDU_RX(("wl%d: wlc_ampdu_recvdata: q out of order seq 0x%x(exp 0x%x)\n",
		wlc->pub->unit, seq, resp->exp_seq));

	ASSERT(!resp->rxq[indx]);
	resp->rxq[indx] = f->p;
	resp->wrxh[indx] = f->wrxh;
	resp->queued++;

	WLCNTINCR(ampdu->cnt->rxqed);

	return;

toss:
	WL_AMPDU_RX(("wl%d: %s: Received some unexpected packets\n", wlc->pub->unit, __FUNCTION__));
	PKTFREE(wlc->osh, f->p, FALSE);
	WLCNTINCR(ampdu->cnt->rxunexp);

	/* AMPDUXXX: protocol failure, send delba */
	wlc_ampdu_send_delba(ampdu, scb_ampdu, tid, FALSE, DOT11_RC_UNSPECIFIED);
}

void
wlc_frameaction_ampdu(ampdu_info_t *ampdu, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	uint8 action_id;

	ASSERT((body[0] & DOT11_ACTION_CAT_MASK) == DOT11_ACTION_CAT_BLOCKACK);

	if (!scb) {
		WL_AMPDU_CTL(("wl%d: wlc_frameaction_ampdu: scb not found\n",
			ampdu->wlc->pub->unit));
		return;
	}

	if (body_len < 2)
		goto err;

	action_id = body[1];

	switch (action_id) {

	case DOT11_BA_ACTION_ADDBA_REQ:
		if (body_len < DOT11_ADDBA_REQ_LEN)
			goto err;
		wlc_ampdu_recv_addba_req(ampdu, scb, body, body_len);
		break;

	case DOT11_BA_ACTION_ADDBA_RESP:
		if (body_len < DOT11_ADDBA_RESP_LEN)
			goto err;
		wlc_ampdu_recv_addba_resp(ampdu, scb, body, body_len);
		break;

	case DOT11_BA_ACTION_DELBA:
		if (body_len < DOT11_DELBA_LEN)
			goto err;
		wlc_ampdu_recv_delba(ampdu, scb, body, body_len);
		break;

	default:
		WL_ERROR(("wl%d: FC_ACTION: Invalid BA action id %d\n",
			ampdu->wlc->pub->unit, action_id));
		goto err;
	}

	return;

err:
	WL_ERROR(("wl%d: wlc_frameaction_ampdu: recd invalid frame of length %d\n",
		ampdu->wlc->pub->unit, body_len));
	WLCNTINCR(ampdu->cnt->rxunexp);
	wlc_send_action_err(ampdu->wlc, hdr, body, body_len);
}

void
wlc_ampdu_recv_ctl(ampdu_info_t *ampdu, struct scb *scb, uint8 *body, int body_len, uint16 fk)
{

	if (!scb || !SCB_AMPDU(scb)) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_ctl: AMPDU not advertized by remote\n",
			ampdu->wlc->pub->unit));
		return;
	}

	if (fk == FC_BLOCKACK_REQ) {
		if (body_len < DOT11_BAR_LEN)
			goto err;
		wlc_ampdu_recv_bar(ampdu, scb, body, body_len);
	} else if (fk == FC_BLOCKACK) {
		if (body_len < (DOT11_BA_LEN + DOT11_BA_CMP_BITMAP_LEN))
			goto err;
		wlc_ampdu_recv_ba(ampdu, scb, body, body_len);
	} else {
		ASSERT(0);
	}

	return;

err:
	WL_ERROR(("wl%d: wlc_ampdu_recv_ctl: recd invalid frame of length %d\n",
		ampdu->wlc->pub->unit, body_len));
	WLCNTINCR(ampdu->cnt->rxunexp);
}

static void
ampdu_ba_state_off(scb_ampdu_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	void *p;

	ini->ba_state = AMPDU_TID_STATE_BA_OFF;

	/* release all buffered pkts */
	while ((p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid))) {
		ASSERT(PKTPRIO(p) == ini->tid);
		SCB_TX_NEXT(TXMOD_AMPDU, ini->scb, p, WLC_PRIO_TO_PREC(ini->tid));
	}
}

static void
ampdu_cleanup_tid_resp(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu, uint8 tid)
{
	scb_ampdu_tid_resp_t *resp;

	if (scb_ampdu->resp[tid] == NULL)
		return;

	WL_AMPDU_CTL(("wl%d: ampdu_cleanup_tid_resp: tid %d\n", ampdu->wlc->pub->unit, tid));

	/* send up all the pending pkts in order from the rx reorder q going over holes */
	wlc_ampdu_release_n_ordered(ampdu->wlc, scb_ampdu, tid, AMPDU_RX_BA_MAX_WSIZE);

	/* recheck scb_ampdu->resp[] since it may have been freed while releasing */
	if ((resp = scb_ampdu->resp[tid])) {
		ASSERT(resp->queued == 0);
		MFREE(ampdu->wlc->osh, resp, sizeof(scb_ampdu_tid_resp_t));
		scb_ampdu->resp[tid] = NULL;
	}

	ampdu->resp_cnt--;
	if (ampdu->resp_cnt == 0)
		wl_del_timer(ampdu->wlc->wl, ampdu->resp_timer);
}

static void
ampdu_cleanup_tid_ini(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu, uint8 tid, bool force)
{
	scb_ampdu_tid_ini_t *ini;
	uint16 indx, i;

	if (!(ini = scb_ampdu->ini[tid]))
		return;

	ini->ba_state = AMPDU_TID_STATE_BA_PENDING_OFF;

	WL_AMPDU_CTL(("wl%d: ampdu_cleanup_tid_ini: tid %d\n", ampdu->wlc->pub->unit, tid));

	/* cleanup stuff that was to be done on bar send complete */
	if (!ini->bar_ackpending) {
		for (indx = 0; indx < AMPDU_TX_BA_MAX_WSIZE; indx++) {
			if (isset(ini->barpending, indx)) {
				clrbit(ini->barpending, indx);
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					ini->txretry[indx] = 0;
					ini->tx_in_transit--;
				}
				wlc_ampdu_ini_move_window(ampdu, scb_ampdu, ini);
			}
		}
	}

	if (ini->tx_in_transit && !force)
		return;

	if (ini->tx_in_transit == 0)
		ASSERT(ini->rem_window == ini->ba_wsize);

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, ini->scb);

	ASSERT(ini == scb_ampdu->ini[ini->tid]);

	scb_ampdu->ini[ini->tid] = NULL;

	/* free all buffered tx packets */
	wlc_ampdu_pktq_pflush(ampdu->wlc->osh, &scb_ampdu->txq, ini->tid, TRUE, NULL, 0);

	if (!ini->bar_ackpending)
		MFREE(ampdu->wlc->osh, ini, sizeof(scb_ampdu_tid_ini_t));
	else {
		/* save ini in case bar complete cb is not called */
		i = ampdu->ini_free_index;
		if (ampdu->ini_free[i])
			MFREE(ampdu->wlc->osh, ampdu->ini_free[i],
				sizeof(scb_ampdu_tid_ini_t));
		ampdu->ini_free[i] = (void *)ini;
		ampdu->ini_free_index = MODINC(ampdu->ini_free_index, AMPDU_INI_FREE);
		ini->free_me = TRUE;
	}

	wlc_txflowcontrol_override(ampdu->wlc, SCB_WLCIFP(scb_ampdu->scb)->qi,
	                           OFF, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL);
}

static void
wlc_ampdu_recv_addba_req(ampdu_info_t *ampdu, struct scb *scb, uint8 *body, int body_len)
{
	scb_ampdu_t *scb_ampdu;
	wlc_info_t *wlc;
	dot11_addba_req_t *addba_req;
	scb_ampdu_tid_resp_t *resp;
	scb_ampdu_tid_ini_t *ini;
	uint16 param_set, timeout, start_seq;
	uint8 tid, wsize, policy;

	wlc = ampdu->wlc;
	ASSERT(scb);

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	addba_req = (dot11_addba_req_t *)body;

	timeout = ltoh16_ua(&addba_req->timeout);
	start_seq = ltoh16_ua(&addba_req->start_seqnum);
	param_set = ltoh16_ua(&addba_req->addba_param_set);

	/* accept the min of our and remote timeout */
	timeout = MIN(timeout, ampdu->delba_timeout);

	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu, tid, "wlc_ampdu_recv_addba_req");

	if (ampdu->rxaggr == OFF) {
		wlc_send_addba_resp(wlc, scb, DOT11_SC_DECLINED,
			addba_req->token, timeout, param_set);
		return;
	}

	/* check if it is action err frame */
	if (addba_req->category & DOT11_ACTION_CAT_ERR_MASK) {
		ini = scb_ampdu->ini[tid];
		if ((ini != NULL) && (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
			ampdu_ba_state_off(scb_ampdu, ini);
			WL_ERROR(("wl%d: wlc_ampdu_recv_addba_req: error action frame\n",
				wlc->pub->unit));
		} else {
			WL_ERROR(("wl%d: wlc_ampdu_recv_addba_req: unexp error action "
				"frame\n", wlc->pub->unit));
		}
		WLCNTINCR(ampdu->cnt->rxunexp);
		return;
	}

	if (!AMPDU_ENAB(ampdu->wlc->pub) || (scb->bsscfg->BSS && !SCB_HT_CAP(scb))) {
		wlc_send_addba_resp(wlc, scb, DOT11_SC_DECLINED,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu->cnt->txaddbaresp);
		return;
	}

	if (!ampdu->rxba_enable[tid]) {
		wlc_send_addba_resp(wlc, scb, DOT11_SC_DECLINED,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu->cnt->txaddbaresp);
		return;
	}

	policy = (param_set & DOT11_ADDBA_PARAM_POLICY_MASK) >> DOT11_ADDBA_PARAM_POLICY_SHIFT;
	if (policy != ampdu->ba_policy) {
		wlc_send_addba_resp(wlc, scb, DOT11_SC_INVALID_PARAMS,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu->cnt->txaddbaresp);
		return;
	}

	/* cleanup old state */
	ampdu_cleanup_tid_resp(ampdu, scb_ampdu, tid);

	ASSERT(scb_ampdu->resp[tid] == NULL);

	resp = MALLOC(wlc->osh, sizeof(scb_ampdu_tid_resp_t));

	if (resp == NULL) {
		wlc_send_addba_resp(wlc, scb, DOT11_SC_FAILURE,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu->cnt->txaddbaresp);
		return;
	}
	bzero((char *)resp, sizeof(scb_ampdu_tid_resp_t));

	wsize =	(param_set & DOT11_ADDBA_PARAM_BSIZE_MASK) >> DOT11_ADDBA_PARAM_BSIZE_SHIFT;
	/* accept the min of our and remote wsize if remote has the advisory set */
	if (wsize)
		wsize = MIN(wsize, ampdu->ba_rx_wsize);
	else
		wsize = ampdu->ba_rx_wsize;
	WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_req: BA ON: seq 0x%x tid %d wsize %d\n",
		wlc->pub->unit, start_seq, tid, wsize));

	param_set &= ~DOT11_ADDBA_PARAM_BSIZE_MASK;
	param_set |= (wsize << DOT11_ADDBA_PARAM_BSIZE_SHIFT) & DOT11_ADDBA_PARAM_BSIZE_MASK;

	/* clear the A-MSDU supported field */
	param_set &= ~DOT11_ADDBA_PARAM_AMSDU_SUP;

	scb_ampdu->resp[tid] = resp;
	resp->exp_seq = start_seq >> SEQNUM_SHIFT;
	resp->ba_wsize = wsize;
	resp->ba_state = AMPDU_TID_STATE_BA_ON;

	WLCNTINCR(ampdu->cnt->rxaddbareq);

	wlc_send_addba_resp(wlc, scb, DOT11_SC_SUCCESS, addba_req->token,
		timeout, param_set);
	WLCNTINCR(ampdu->cnt->txaddbaresp);
	AMPDUSCBCNTINCR(scb_ampdu->txaddbaresp);

	if (ampdu->resp_cnt == 0)
		wl_add_timer(wlc->wl, ampdu->resp_timer, AMPDU_RESP_TIMEOUT, TRUE);
	ampdu->resp_cnt++;
}

static void
wlc_ampdu_recv_addba_resp(ampdu_info_t *ampdu, struct scb *scb, uint8 *body, int body_len)
{
	scb_ampdu_t *scb_ampdu;
	wlc_info_t *wlc;
	struct pktq *txq;
	dot11_addba_resp_t *addba_resp;
	scb_ampdu_tid_ini_t *ini;
	uint16 param_set, timeout, status;
	uint8 tid, wsize, policy;
	uint16 current_wlctxq_len = 0, i = 0;
	void *p = NULL;

	wlc = ampdu->wlc;
	ASSERT(scb);

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	addba_resp = (dot11_addba_resp_t *)body;

	if (addba_resp->category & DOT11_ACTION_CAT_ERR_MASK) {
		WL_ERROR(("wl%d: wlc_ampdu_recv_addba_resp: unexp error action frame\n",
			wlc->pub->unit));
		WLCNTINCR(ampdu->cnt->rxunexp);
		return;
	}

	status = ltoh16_ua(&addba_resp->status);
	timeout = ltoh16_ua(&addba_resp->timeout);
	param_set = ltoh16_ua(&addba_resp->addba_param_set);

	wsize =	(param_set & DOT11_ADDBA_PARAM_BSIZE_MASK) >> DOT11_ADDBA_PARAM_BSIZE_SHIFT;
	policy = (param_set & DOT11_ADDBA_PARAM_POLICY_MASK) >> DOT11_ADDBA_PARAM_POLICY_SHIFT;
	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu, tid, "wlc_ampdu_recv_addba_resp");

	ini = scb_ampdu->ini[tid];

	if ((ini == NULL) || (ini->ba_state != AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_resp: unsolicited packet\n",
			wlc->pub->unit));
		WLCNTINCR(ampdu->cnt->rxunexp);
		return;
	}

	if ((status != DOT11_SC_SUCCESS) ||
	    (policy != ampdu->ba_policy) ||
	    (wsize > AMPDU_TX_BA_MAX_WSIZE)) {
		WL_ERROR(("wl%d: wlc_ampdu_recv_addba_resp: Failed. status %d wsize %d"
			" policy %d\n", wlc->pub->unit, status, wsize, policy));
		ampdu_ba_state_off(scb_ampdu, ini);
		WLCNTINCR(ampdu->cnt->rxunexp);
		return;
	}

	if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
		ini->ba_wsize = AMPDU_TX_BA_MAX_WSIZE;
		ini->rem_window = ini->ba_wsize;
	} else {
		ini->ba_wsize = wsize;
		ini->rem_window = wsize;
	}
	ini->start_seq = (SCB_SEQNUM(scb, tid) & (SEQNUM_MAX - 1));
	ini->tx_exp_seq = ini->start_seq;
	ini->max_seq = MODSUB_POW2(ini->start_seq, 1, SEQNUM_MAX);
	ini->ba_state = AMPDU_TID_STATE_BA_ON;

	WLCNTINCR(ampdu->cnt->rxaddbaresp);
	AMPDUSCBCNTINCR(scb_ampdu->rxaddbaresp);

	WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_resp: Turning BA ON: tid %d wsize %d\n",
		wlc->pub->unit, tid, wsize));

	/* send bar to set initial window */
	wlc_ampdu_send_bar(wlc->ampdu, ini, TRUE);

	/* if packets already exist in wlcq, conditionally transfer them to ampduq
		to avoid barpending stuck issue.
	*/
	txq = &SCB_WLCIFP(scb)->qi->q;

	current_wlctxq_len = pktq_plen(txq, WLC_PRIO_TO_PREC(tid));

	for (i = 0; i < current_wlctxq_len; i++) {
		p = pktq_pdeq(txq, WLC_PRIO_TO_PREC(tid));

		if (!p) break;
		/* omit control/mgmt frames and queue them to the back
		 * only the frames relevant to this scb should be moved to ampduq;
		 * otherwise, queue them back.
		 */
		if (!(WLPKTTAG(p)->flags & WLF_DATA) || 	/* mgmt/ctl */
		    scb != WLPKTTAGSCBGET(p) || /* frame belong to some other scb */
		    (WLPKTTAG(p)->flags & WLF_AMPDU_MPDU)) { /* possibly retried AMPDU */
			wlc_prec_enq(ampdu->wlc, txq, p, WLC_PRIO_TO_PREC(tid));
		} else { /* XXXCheck if Flow control/watermark issues */
			wlc_ampdu_prec_enq(ampdu->wlc, &scb_ampdu->txq, p, tid);
		}
	}

	/* release pkts */
	wlc_ampdu_txeval(ampdu, scb_ampdu, ini, FALSE);
}

static void
wlc_ampdu_recv_delba(ampdu_info_t *ampdu, struct scb *scb, uint8 *body, int body_len)
{
	scb_ampdu_t *scb_ampdu;
	dot11_delba_t *delba;
	uint16 param_set;
	uint8 tid;
	uint16 reason, initiator;

	ASSERT(scb);

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	delba = (dot11_delba_t *)body;

	if (delba->category & DOT11_ACTION_CAT_ERR_MASK) {
		WL_ERROR(("wl%d: wlc_ampdu_recv_delba: unexp error action frame\n",
			ampdu->wlc->pub->unit));
		WLCNTINCR(ampdu->cnt->rxunexp);
		return;
	}

	param_set = ltoh16(delba->delba_param_set);
	reason = ltoh16(delba->reason);

	tid = (param_set & DOT11_DELBA_PARAM_TID_MASK) >> DOT11_DELBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu, tid, "wlc_ampdu_recv_delba");

	initiator = (param_set & DOT11_DELBA_PARAM_INIT_MASK) >> DOT11_DELBA_PARAM_INIT_SHIFT;

	if (initiator)
		ampdu_cleanup_tid_resp(ampdu, scb_ampdu, tid);
	else
		ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, FALSE);

	WLCNTINCR(ampdu->cnt->rxdelba);
	AMPDUSCBCNTINCR(scb_ampdu->rxdelba);

	WL_ERROR(("wl%d: wlc_ampdu_recv_delba: AMPDU OFF: tid %d initiator %d reason %d\n",
		ampdu->wlc->pub->unit, tid, initiator, reason));
}

/* moves the window forward on receipt of a bar */
static void
wlc_ampdu_recv_bar(ampdu_info_t *ampdu, struct scb *scb, uint8 *body, int body_len)
{
	scb_ampdu_t *scb_ampdu;
	wlc_info_t *wlc;
	struct dot11_bar *bar = (struct dot11_bar *)body;
	scb_ampdu_tid_resp_t *resp;
	uint8 tid;
	uint16 seq, tmp, offset;

	wlc = ampdu->wlc;
	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	tmp = ltoh16(bar->bar_control);
	tid = (tmp & DOT11_BA_CTL_TID_MASK) >> DOT11_BA_CTL_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu, tid, "wlc_ampdu_recv_bar");

	if (tmp & DOT11_BA_CTL_MTID) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: multi tid not supported\n",
			wlc->pub->unit));
		WLCNTINCR(ampdu->cnt->rxunexp);
		return;
	}

	resp = scb_ampdu->resp[tid];
	if (resp == NULL) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: uninitialized tid %d\n",
			wlc->pub->unit, tid));
		WLCNTINCR(ampdu->cnt->rxunexp);
		return;
	}

	WLCNTINCR(ampdu->cnt->rxbar);
	AMPDUSCBCNTINCR(scb_ampdu->rxbar);

	seq = (ltoh16(bar->seqnum)) >> SEQNUM_SHIFT;

	WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: length %d tid %d seq 0x%x\n",
		wlc->pub->unit, body_len, tid, seq));

	offset = MODSUB_POW2(seq, resp->exp_seq, SEQNUM_MAX);

	/* ignore if it is in the "old" half of sequence space */
	if (offset > (SEQNUM_MAX >> 1)) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: ignore bar with offset 0x%x\n",
			wlc->pub->unit, offset));
		return;
	}

	/* release all received pkts till the seq */
	wlc_ampdu_release_n_ordered(wlc, scb_ampdu, tid, offset);

	/* release more pending ordered packets if possible */
	wlc_ampdu_release_ordered(wlc, scb_ampdu, tid);
}

static void
wlc_ampdu_recv_ba(ampdu_info_t *ampdu, struct scb *scb, uint8 *body, int body_len)
{
	/* AMPDUXXX: silently ignore the ba since we dont handle it for immediate ba */
	WLCNTINCR(ampdu->cnt->rxba);
}

/* initialize the initiator code for tid */
static scb_ampdu_tid_ini_t*
wlc_ampdu_init_tid_ini(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu, uint8 tid, bool override)
{
	scb_ampdu_tid_ini_t *ini;
	uint8 peer_density;

	ASSERT(scb_ampdu);
	ASSERT(scb_ampdu->scb);
	ASSERT(SCB_AMPDU(scb_ampdu->scb));
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	if (ampdu->manual_mode && !override)
		return NULL;

	/* check for per-tid control of ampdu */
	if (!ampdu->ini_enable[tid])
		return NULL;

	/* AMPDUXXX: No support for dynamic update of density/len from peer */
	/* retrieve the density and max ampdu len from scb */

	peer_density = (scb_ampdu->scb->ht_ampdu_params &
		HT_PARAMS_DENSITY_MASK) >> HT_PARAMS_DENSITY_SHIFT;

	/* our density requirement is for tx side as well */
	scb_ampdu->mpdu_density = MAX(peer_density, ampdu->mpdu_density);

	/* should call after mpdu_density is init'd */
	wlc_ampdu_init_min_lens(scb_ampdu);

	scb_ampdu->max_rxlen = AMPDU_RX_FACTOR_BASE <<
		(scb_ampdu->scb->ht_ampdu_params & HT_PARAMS_RX_FACTOR_MASK);

	scb_ampdu_update_config(ampdu, scb_ampdu->scb);

	if (!scb_ampdu->ini[tid]) {
		ini = MALLOC(ampdu->wlc->osh, sizeof(scb_ampdu_tid_ini_t));

		if (ini == NULL)
			return NULL;

		scb_ampdu->ini[tid] = ini;
	} else
		ini = scb_ampdu->ini[tid];

	bzero((char *)ini, sizeof(scb_ampdu_tid_ini_t));

	ini->ba_state = AMPDU_TID_STATE_BA_PENDING_ON;
	ini->tid = tid;
	ini->scb = scb_ampdu->scb;
	ini->retry_bar = AMPDU_R_BAR_DISABLED;

	wlc_send_addba_req(ampdu->wlc, ini->scb, tid, ampdu->ba_tx_wsize,
		ampdu->ba_policy, ampdu->delba_timeout);

	WLCNTINCR(ampdu->cnt->txaddbareq);
	AMPDUSCBCNTINCR(scb_ampdu->txaddbareq);

	return ini;
}

static void
wlc_ampdu_send_delba(ampdu_info_t *ampdu, scb_ampdu_t *scb_ampdu, uint8 tid,
	uint16 initiator, uint16 reason)
{
	ASSERT(scb_ampdu);
	ASSERT(scb_ampdu->scb);
	ASSERT(SCB_AMPDU(scb_ampdu->scb));
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	/* cleanup state */
	if (initiator)
		ampdu_cleanup_tid_ini(ampdu, scb_ampdu, tid, FALSE);
	else
		ampdu_cleanup_tid_resp(ampdu, scb_ampdu, tid);

	WL_ERROR(("wl%d: wlc_ampdu_send_delba: tid %d initiator %d reason %d\n",
		ampdu->wlc->pub->unit, tid, initiator, reason));

	wlc_send_delba(ampdu->wlc, scb_ampdu->scb, tid, initiator, reason);

	WLCNTINCR(ampdu->cnt->txdelba);
}


int
wlc_ampdu_set(ampdu_info_t *ampdu, bool on)
{
	wlc_info_t *wlc = ampdu->wlc;
	uint8 ampdu_mac_agg = AMPDU_AGG_OFF;
	uint16 value = 0;
	int err = BCME_OK;
	int8 aggmode;

	wlc->pub->_ampdu = FALSE;

	if (ampdu->ampdu_aggmode == AMPDU_AGG_AUTO) {
		if (D11REV_IS(wlc->pub->corerev, 31) && WLCISNPHY(wlc->band)) {
			aggmode = AMPDU_AGGMODE_HOST;
		} else if ((D11REV_IS(wlc->pub->corerev, 26) ||
			D11REV_IS(wlc->pub->corerev, 29)) &&
			WLCISHTPHY(wlc->band)) {
			aggmode = AMPDU_AGGMODE_HOST;
		}
#ifdef WLP2P
		else if (D11REV_IS(wlc->pub->corerev, 24) && WLCISLCNPHY(wlc->band))
			aggmode = AMPDU_AGGMODE_HOST;
#endif
		else if (D11REV_GE(wlc->pub->corerev, 16) && D11REV_LT(wlc->pub->corerev, 26))
			aggmode = AMPDU_AGGMODE_MAC;
		else
			aggmode = AMPDU_AGGMODE_HOST;
	} else
		aggmode = ampdu->ampdu_aggmode;

	if (on) {
		if (!N_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver not nmode enabled\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
		if (!wlc_ampdu_cap(ampdu)) {
			WL_AMPDU_ERR(("wl%d: device not ampdu capable\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
#ifdef WLAFTERBURNER
		if (wlc->afterburner) {
			WL_AMPDU_ERR(("wl%d: driver is afterburner enabled\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
#endif /* WLAFTERBURNER */
		if (PIO_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver is pio mode\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}

#ifdef WLAMPDU_MAC
		if (aggmode == AMPDU_AGGMODE_MAC) {
			if (D11REV_GE(wlc->pub->corerev, 16) && D11REV_LT(wlc->pub->corerev, 26)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: 26 > Corerev >= 16 required for ucode agg\n",
					wlc->pub->unit));
#endif
			}
			if (D11REV_IS(wlc->pub->corerev, 24) && WLCISNPHY(wlc->band)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: Corerev is 26 NPHY support ucode agg only\n",
				wlc->pub->unit));
#endif
			}
			if ((D11REV_IS(wlc->pub->corerev, 26) ||
				D11REV_IS(wlc->pub->corerev, 29)) && WLCISHTPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 26 or 29 support hw agg only\n",
				wlc->pub->unit));
			}
			if (D11REV_IS(wlc->pub->corerev, 31) && WLCISNPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 31 NPHY support hw agg only\n",
					wlc->pub->unit));
			}
		}
#endif /* WLAMPDU_MAC */
	}

	if (aggmode == AMPDU_AGGMODE_MAC) {
		wlc->pub->txmaxpkts = MAXTXPKTS_AMPDUMAC;
		value = MHF3_UCAMPDU_RETX;
	} else {
		wlc->pub->txmaxpkts = MAXTXPKTS;
	}

	if (wlc->pub->_ampdu != on) {
		bzero(ampdu->cnt, sizeof(ampdu->cnt));
		wlc->pub->_ampdu = on;
	}

exit:
	if (wlc->pub->_ampdumac != ampdu_mac_agg) {
		wlc->pub->_ampdumac = ampdu_mac_agg;
		wlc_mhf(wlc, MHF3, MHF3_UCAMPDU_RETX, value, WLC_BAND_ALL);
		/* just differentiate HW or not for now */
		wlc_ampdu_mode_upd(wlc, AMPDU_HW_ENAB(wlc->pub) ?
			AMPDU_AGG_HW : AMPDU_AGGMODE_HOST);
	}

	return err;
}

bool
wlc_ampdu_cap(ampdu_info_t *ampdu)
{
	if (WLC_PHY_11N_CAP(ampdu->wlc->band))
		return TRUE;
	else
		return FALSE;
}

static void
ampdu_update_max_txlen(ampdu_info_t *ampdu, uint8 dur)
{
	uint32 rate, mcs;

	for (mcs = 0; mcs < MCS_TABLE_SIZE; mcs++) {
		/* rate is in Kbps; dur is in msec ==> len = (rate * dur) / 8 */
		/* 20MHz, No SGI */
		rate = MCS_RATE(mcs, FALSE, FALSE);
		ampdu->max_txlen[mcs][0][0] = (rate * dur) >> 3;
		/* 40 MHz, No SGI */
		rate = MCS_RATE(mcs, TRUE, FALSE);
		ampdu->max_txlen[mcs][1][0] = (rate * dur) >> 3;
		/* 20MHz, SGI */
		rate = MCS_RATE(mcs, FALSE, TRUE);
		ampdu->max_txlen[mcs][0][1] = (rate * dur) >> 3;
		/* 40 MHz, SGI */
		rate = MCS_RATE(mcs, TRUE, TRUE);
		ampdu->max_txlen[mcs][1][1] = (rate * dur) >> 3;
	}
}

static void
wlc_ampdu_update_ie_param(ampdu_info_t *ampdu)
{
	uint8 params;
	wlc_info_t *wlc = ampdu->wlc;

	params = wlc->ht_cap.params;
	params &= ~(HT_PARAMS_RX_FACTOR_MASK | HT_PARAMS_DENSITY_MASK);
	params |= (ampdu->rx_factor & HT_PARAMS_RX_FACTOR_MASK);
	params |= (ampdu->mpdu_density << HT_PARAMS_DENSITY_SHIFT) & HT_PARAMS_DENSITY_MASK;
	wlc->ht_cap.params = params;

	if (AP_ENAB(wlc->pub) && wlc->clk) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}

}

static void
wlc_ampdu_init_min_lens(scb_ampdu_t *scb_ampdu)
{
	int mcs, tmp;
	tmp = 1 << (17 - scb_ampdu->mpdu_density);
	for (mcs = 0; mcs <= FFPLD_MAX_MCS; mcs++) {
		scb_ampdu->min_lens[mcs] = 0;
		if (scb_ampdu->mpdu_density > 6)
			continue;
		if (mcs >= 12 && mcs <= 15) {
			/* use mcs21 */
			scb_ampdu->min_lens[mcs] = CEIL(MCS_RATE(21, 1, 0), tmp);
		} else if (mcs == 21 || mcs == 22) {
			/* use mcs23 */
			scb_ampdu->min_lens[mcs] = CEIL(MCS_RATE(23, 1, 0), tmp);
		}
	}
	scb_ampdu->min_len = CEIL(MCS_RATE(FFPLD_MAX_MCS, 1, 1), tmp);
}

uint8 BCMFASTPATH
wlc_ampdu_null_delim_cnt(ampdu_info_t *ampdu, struct scb *scb, ratespec_t rspec,
	int phylen, uint16* minbytes)
{
	scb_ampdu_t *scb_ampdu;
	int bytes = 0, cnt, tmp;
	uint8 tx_density;

	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	if (scb_ampdu->mpdu_density == 0)
		return 0;

	/* RSPEC2RATE is in kbps units ==> ~RSPEC2RATE/2^13 is in bytes/usec
	   density x is in 2^(x-4) usec
	   ==> # of bytes needed for req density = rate/2^(17-x)
	   ==> # of null delimiters = ceil(ceil(rate/2^(17-x)) - phylen)/4)
	 */

	tx_density = scb_ampdu->mpdu_density;

	if (wlc_btc_wire_get(ampdu->wlc) >= WL_BTC_3WIRE)
		tx_density = AMPDU_MAX_MPDU_DENSITY;

	ASSERT(tx_density <= AMPDU_MAX_MPDU_DENSITY);
	if (tx_density < 7 && ampdu->wlc->frameburst && RSPEC_IS40MHZ(rspec)) {
		int mcs = rspec & RSPEC_RATE_MASK;
		ASSERT(mcs <= FFPLD_MAX_MCS);
		bytes = scb_ampdu->min_lens[mcs];
	}
	if (bytes == 0) {
		tmp = 1 << (17 - tx_density);
		bytes = CEIL(RSPEC2RATE(rspec), tmp);
	}
	*minbytes = (uint16)bytes;

	if (bytes > phylen) {
		cnt = CEIL(bytes - phylen, AMPDU_DELIMITER_LEN);
		ASSERT(cnt <= 255);

		return (uint8)cnt;
	} else
		return 0;
}

void
wlc_ampdu_macaddr_upd(wlc_info_t *wlc)
{
	char template[T_RAM_ACCESS_SZ*2];

	/* driver needs to write the ta in the template; ta is at offset 16 */
	bzero(template, sizeof(template));
	bcopy((char*)wlc->pub->cur_etheraddr.octet, template, ETHER_ADDR_LEN);
	wlc_write_template_ram(wlc, (T_BA_TPL_BASE + 16), (T_RAM_ACCESS_SZ*2), template);
}

void
wlc_ampdu_shm_upd(ampdu_info_t *ampdu)
{
	wlc_info_t *wlc = ampdu->wlc;

	if (AMPDU_ENAB(ampdu->wlc->pub) && (WLC_PHY_11N_CAP(ampdu->wlc->band)))	{
		/* Extend ucode internal watchdog timer to match larger received frames */
		if ((ampdu->rx_factor & HT_PARAMS_RX_FACTOR_MASK) == AMPDU_RX_FACTOR_64K) {
			wlc_write_shm(wlc, M_MIMO_MAXSYM, MIMO_MAXSYM_MAX);
			wlc_write_shm(wlc, M_WATCHDOG_8TU, WATCHDOG_8TU_MAX);
		} else {
			wlc_write_shm(wlc, M_MIMO_MAXSYM, MIMO_MAXSYM_DEF);
			wlc_write_shm(wlc, M_WATCHDOG_8TU, WATCHDOG_8TU_DEF);
		}
	}
}

void
wlc_ampdu_pkt_free(ampdu_info_t *ampdu, void *p)
{
	wlc_info_t *wlc = ampdu->wlc;
	struct scb *scb;
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 indx;

	if (!(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU))
		return;

	scb = WLPKTTAGSCBGET(p);
	/* scb pointer may be NULL if pkt is on DMA ring */
	if (!scb) {
		d11txh_t *txh;
		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		scb = wlc_scbfind(wlc, (struct ether_addr*)txh->TxFrameRA);
	}

	if (!scb || !SCB_AMPDU(scb))
		return;

	scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[PKTPRIO(p)];
	if (!ini)
		return;
	ASSERT(ini->scb == scb);

	indx = TX_SEQ_TO_INDEX(WLPKTTAG(p)->seq);
	ASSERT(isset(ini->ackpending, indx));

	setbit(ini->barpending, indx);
	/* send bar to move the window */
	wlc_ampdu_send_bar(ampdu, ini, FALSE);
}

#if defined(BCMDBG) || defined(WLTEST)
static int
wlc_ampdu_dump(ampdu_info_t *ampdu, struct bcmstrbuf *b)
{
	wlc_ampdu_cnt_t *cnt = ampdu->cnt;
	int i, j, last;
	uint32 max_val, total = 0;
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	int resp = 0, inic = 0, ini_on = 0, ini_off = 0, ini_pon = 0, ini_poff = 0;
	int nbuf = 0;
	wlc_fifo_info_t *fifo;
	char eabuf[ETHER_ADDR_STR_LEN];
#ifdef WLPKTDLYSTAT
	uint ac;
	uint32 sum;
	wlc_scb_delay_stats_t *delay_stats;
#endif
#ifdef  WLOVERTHRUSTER
	wlc_tcp_ack_info_t *tcp_ack_info = &ampdu->tcp_ack_info;
#endif  /* WLOVERTHRUSTER */

	bcm_bprintf(b, "HOST_ENAB %d MAC_ENAB %d: HW_ENAB %d UCODE_ENAB %d\n",
		AMPDU_HOST_ENAB(ampdu->wlc->pub), AMPDU_MAC_ENAB(ampdu->wlc->pub),
		AMPDU_HW_ENAB(ampdu->wlc->pub), AMPDU_UCODE_ENAB(ampdu->wlc->pub));

#ifdef WLAMPDU_MAC
	if (ampdu->wlc->clk && AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
		uint32 hwampdu, hwrxba, hwnoba;
		hwampdu = wlc_read_shm(ampdu->wlc, M_TXAMPDU_CNT);
		hwrxba = wlc_read_shm(ampdu->wlc, M_RXBA_CNT);
		hwnoba = (hwampdu < hwrxba) ? 0 : (hwampdu - hwrxba);
		bcm_bprintf(b, "%s: txampdu %d txmpdu(enq) %d txs %d noba %d (%d%%)\n",
			AMPDU_UCODE_ENAB(ampdu->wlc->pub) ? "Ucode" : "HW",
			hwampdu, cnt->txmpdu, cnt->u0.txs, hwnoba,
		        hwampdu ? CEIL(hwnoba*100, hwampdu) : 0);
		bcm_bprintf(b, "txmrt/succ %d %d (f %d%%) "
			"txfbr/succ %d %d (f %d%%)\n",
			cnt->tx_mrt, cnt->txsucc_mrt,
			cnt->tx_mrt ? CEIL((cnt->tx_mrt - cnt->txsucc_mrt)*100, cnt->tx_mrt) : 0,
			cnt->tx_fbr, cnt->txsucc_fbr,
			cnt->tx_fbr ? CEIL((cnt->tx_fbr - cnt->txsucc_fbr)*100, cnt->tx_fbr) : 0);
	} else
#endif /* WLAMPDU_MAC */
	{
		bcm_bprintf(b, "txampdu %d txmpdu %d txmpduperampdu %d noba %d (%d%%)\n",
			cnt->txampdu, cnt->txmpdu,
			cnt->txampdu ? CEIL(cnt->txmpdu, cnt->txampdu) : 0, cnt->noba,
			cnt->txampdu ? CEIL(cnt->noba*100, cnt->txampdu) : 0);
		bcm_bprintf(b, "retry_ampdu %d retry_mpdu %d (%d%%) fbr_ampdu %d fbr_mpdu %d\n",
			cnt->txretry_ampdu, cnt->u0.txretry_mpdu,
			(cnt->txmpdu ? CEIL(cnt->u0.txretry_mpdu*100, cnt->txmpdu) : 0),
			cnt->txfbr_ampdu, cnt->txfbr_mpdu);
	}
	bcm_bprintf(b, "txregmpdu %d txreg_noack %d txfifofull %d\n",
		cnt->txregmpdu, cnt->txreg_noack, cnt->txfifofull);

#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(ampdu->wlc->pub)) {
		bcm_bprintf(b, "txrel_wm %d txrel_size %d txrel_agfull %d sduretry %d "
			   "sdurejected %d\n",
			   cnt->txrel_wm, cnt->txrel_size, cnt->txrel_agfull,
			   cnt->sduretry, cnt->sdurejected);
		bcm_bprintf(b, "aggfifo_w %d epochdeltas %d mpduperepoch %d\n",
			cnt->enq, cnt->epochdelta,
			(cnt->epochdelta+1) ? CEIL(cnt->enq, (cnt->epochdelta+1)) : 0);
		bcm_bprintf(b, "epoch_change reason: plcp %d rate %d fbr %d reg %d link %d\n",
			cnt->echgr1, cnt->echgr2, cnt->echgr3, cnt->echgr4, cnt->echgr5);
	}
	else
#endif
		{
			bcm_bprintf(b, "txrel_wm %d txrel_size %d sduretry %d sdurejected %d\n",
				cnt->txrel_wm, cnt->txrel_size, cnt->sduretry, cnt->sdurejected);
			bcm_bprintf(b, "txdrop %d txstuck %d orphan %d\n",
				cnt->txdrop, cnt->txstuck, cnt->orphan);
		}
	bcm_bprintf(b, "txr0hole %d txrnhole %d txrlag %d\n",
		cnt->txr0hole, cnt->txrnhole, cnt->txrlag);
	bcm_bprintf(b, "txaddbareq %d rxaddbaresp %d txbar %d rxba %d\n",
		cnt->txaddbareq, cnt->rxaddbaresp, cnt->txbar, cnt->rxba);

	bcm_bprintf(b, "txdelba %d rxdelba %d rxunexp %d\n",
		cnt->txdelba, cnt->rxdelba, cnt->rxunexp);

#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(ampdu->wlc->pub))
		bcm_bprintf(b, "txmpdu_sgi %d rxampdu_sgi %d txmpdu_stbc %d rxampdu_stbc %d\n",
		  cnt->u1.txmpdu_sgi, cnt->rxampdu_sgi, cnt->u2.txmpdu_stbc, cnt->rxampdu_stbc);
	else
#endif
		bcm_bprintf(b, "txampdu_sgi %d rxampdu_sgi %d txampdu_stbc %d rxampdu_stbc %d "
			"txampdu_mfbr_stbc %d\n", cnt->u1.txampdu_sgi, cnt->rxampdu_sgi,
			cnt->u2.txampdu_stbc, cnt->rxampdu_stbc, cnt->txampdu_mfbr_stbc);

	bcm_bprintf(b, "\nrxampdu %d rxmpdu %d rxmpduperampdu %d rxht %d rxlegacy %d\n",
		cnt->rxampdu, cnt->rxmpdu,
		cnt->rxampdu ? CEIL(cnt->rxmpdu, cnt->rxampdu) : 0,
		cnt->rxht, cnt->rxlegacy);
	bcm_bprintf(b, "rxholes %d rxqed %d rxdup %d rxnobapol %d "
		"rxstuck %d rxoow %d\n",
		cnt->rxholes, cnt->rxqed, cnt->rxdup, cnt->rxnobapol,
		cnt->rxstuck, cnt->rxoow);
	bcm_bprintf(b, "rxaddbareq %d txaddbaresp %d rxbar %d txba %d\n",
		cnt->rxaddbareq, cnt->txaddbaresp, cnt->rxbar, cnt->txba);


	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
			ASSERT(scb_ampdu);
			for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
				if (scb_ampdu->resp[i])
					resp++;
				if ((ini = scb_ampdu->ini[i])) {
					inic++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_OFF)
						ini_off++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_ON)
						ini_on++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF)
						ini_poff++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)
						ini_pon++;
				}
			}
			nbuf += pktq_len(&scb_ampdu->txq);
		}
	}

	bcm_bprintf(b, "resp %d ini %d ini_off %d ini_on %d ini_poff %d ini_pon %d nbuf %d\n",
		resp, inic, ini_off, ini_on, ini_poff, ini_pon, nbuf);

#ifdef WLOVERTHRUSTER
	if (tcp_ack_info->tcp_ack_ratio) {
		bcm_bprintf(b, "tcp_ack_ratio %d tcp_ack_total %d tcp_ack_dequeued %d\n",
		            tcp_ack_info->tcp_ack_ratio, tcp_ack_info->tcp_ack_total,
		            tcp_ack_info->tcp_ack_dequeued);
	}
#endif /* WLOVERTHRUSTER */

	bcm_bprintf(b, "Supr Reason: pmq(%d) flush(%d) frag(%d) badch(%d) exptime(%d) uf(%d)",
		ampdu->supr_reason[TX_STATUS_SUPR_PMQ >> TX_STATUS_SUPR_SHIFT],
		ampdu->supr_reason[TX_STATUS_SUPR_FLUSH >> TX_STATUS_SUPR_SHIFT],
		ampdu->supr_reason[TX_STATUS_SUPR_FRAG >> TX_STATUS_SUPR_SHIFT],
		ampdu->supr_reason[TX_STATUS_SUPR_BADCH >> TX_STATUS_SUPR_SHIFT],
		ampdu->supr_reason[TX_STATUS_SUPR_EXPTIME >> TX_STATUS_SUPR_SHIFT],
		ampdu->supr_reason[TX_STATUS_SUPR_UF >> TX_STATUS_SUPR_SHIFT]);
#ifdef WLP2P
	if (P2P_ENAB(ampdu->wlc->pub))
		bcm_bprintf(b, " abs(%d)",
		            ampdu->supr_reason[TX_STATUS_SUPR_NACK_ABS >> TX_STATUS_SUPR_SHIFT]);
#endif
	bcm_bprintf(b, "\n");

	for (i = 0, max_val = 0, last = 0; i <= FFPLD_MAX_MCS; i++) {
		max_val += ampdu->rxmcs[i];
		if (ampdu->rxmcs[i]) last = i;
	}
	last = 8 * (last/8 + 1) - 1;
	bcm_bprintf(b, "RX MCS  :");
	if (max_val) {
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu->rxmcs[i],
				(ampdu->rxmcs[i] * 100) / max_val);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
	bcm_bprintf(b, "\n");

#ifdef WLAMPDU_HW
	if (ampdu->wlc->clk && AMPDU_HW_ENAB(ampdu->wlc->pub)) {
		uint16 stat_addr = 2 * wlc_read_shm(ampdu->wlc, M_HWAGG_STATS_PTR);
		stat_addr += C_HWAGG_STATS_MPDU_WSZ * 2;
		for (i  = 0; i <= C_HWAGG_STATS_TXMCS_WSZ && i <= FFPLD_MAX_MCS; i++)
			ampdu->txmcs[i] = wlc_read_shm(ampdu->wlc, stat_addr + i * 2);
	}
#endif

	for (i = 0, max_val = 0, last = 0; i <= FFPLD_MAX_MCS; i++) {
		max_val += ampdu->txmcs[i];
		if (ampdu->txmcs[i]) last = i;
	}
	last = 8 * (last/8 + 1) - 1;
	bcm_bprintf(b, "TX MCS  :");
	if (max_val) {
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu->txmcs[i],
				(ampdu->txmcs[i] * 100) / max_val);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
	bcm_bprintf(b, "\n");

#ifdef WLAMPDU_HW
	if (ampdu->wlc->clk && AMPDU_HW_ENAB(ampdu->wlc->pub)) {
		uint16 stat_addr = 2 * wlc_read_shm(ampdu->wlc, M_HWAGG_STATS_PTR);
		stat_addr += C_HWAGG_STATS_MPDU_WSZ * 2;
		for (i  = 0; i <= FFPLD_MAX_MCS; i++) {
			cnt->txmpdu_cnt[i] = wlc_read_shm(ampdu->wlc, stat_addr + i * 2);
			cnt->txmpdu_succ_cnt[i] = wlc_read_shm(ampdu->wlc,
				stat_addr + i * 2 + C_HWAGG_STATS_TXMCS_WSZ * 2);
		}
	}
#endif

	/* Report PER statistics (for each MCS) */
	for (i = 0, max_val = 0, last = 0; i <= FFPLD_MAX_MCS; i++) {
		max_val += cnt->txmpdu_cnt[i];
		if (cnt->txmpdu_cnt[i]) last = i;
	}
	last = 8 * (last/8 + 1) - 1;
	if (max_val) {
		bcm_bprintf(b, "PER : ");
		for (i = 0; i <= last; i++) {
			uint32 loss = 0;
			if (cnt->txmpdu_cnt[i] > cnt->txmpdu_succ_cnt[i])
				loss = cnt->txmpdu_cnt[i] - cnt->txmpdu_succ_cnt[i];
			if (cnt->txmpdu_cnt[i])
				bcm_bprintf(b, "  %d/%d (%d%%)", loss, cnt->txmpdu_cnt[i],
					(loss*100 / cnt->txmpdu_cnt[i]));
			else
				bcm_bprintf(b, "  0");
			if ((i % 8) == 7 && i != last)
			bcm_bprintf(b, "\n    : ");
		}
		bcm_bprintf(b, "\n");
	}

	if (ampdu->wlc->clk) {
		int cnt1 = 0;
		uint16 val, stat_addr = 2 * wlc_read_shm(ampdu->wlc, M_HWAGG_STATS_PTR);
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
			uint16 rempty, ucode_ampdu;
			for (i = 0, total = 0; i < 64; i++) {
				val = wlc_read_shm(ampdu->wlc, stat_addr + i * 2);
				ampdu->mpdu_histogram[i+1] = val;
				total += val;
				cnt1 += (val * (i+1));
			}
			rempty = wlc_read_shm(ampdu->wlc, stat_addr + C_HWAGG_RDEMP_WOFF * 2);
			ucode_ampdu = wlc_read_shm(ampdu->wlc, stat_addr + C_HWAGG_NAMPDU_WOFF * 2);
			bcm_bprintf(b, "--------------------------\n");
			bcm_bprintf(b, "tot_mpdus %d tot_ampdus %d mpduperampdu %d "
				    "ucode_ampdu %d rempty %d (%d%%)\n",
				    cnt1, total,
				    (total == 0) ? 0 : CEIL(cnt1, total),
				    ucode_ampdu, rempty,
				    (ucode_ampdu == 0) ? 0 : CEIL(rempty * 100, ucode_ampdu));
		}
#endif /* WLAMPDU_HW */
		if (ampdu->wlc->frameburst && (D11REV_IS(ampdu->wlc->pub->corerev, 26) ||
		    D11REV_IS(ampdu->wlc->pub->corerev, 29))) {
			/* burst size */
			int cnt2 = 0;
			cnt1 = 0;
			stat_addr += (C_MBURST_WOFF * 2);
			bcm_bprintf(b, "Frameburst histogram:");
			for (i = 0; i < C_MBURST_WSZ; i++) {
				val = wlc_read_shm(ampdu->wlc, stat_addr + i * 2);
				cnt1 += val;
				cnt2 += (i+1) * val;
				bcm_bprintf(b, "  %d", val);
			}
			bcm_bprintf(b, " Avg = %d\n", (cnt1 == 0) ? 0 : CEIL(cnt2, cnt1));
			bcm_bprintf(b, "--------------------------\n");
		}
	}

	for (i = 1, last = 0, total = 0; i <= AMPDU_MAX_MPDU; i++) {
		total += ampdu->mpdu_histogram[i];
		if (ampdu->mpdu_histogram[i])
			last = i;
	}
	bcm_bprintf(b, "MPDUdens:");
	for (i = 1; i <= last; i++) {
		bcm_bprintf(b, " %3d (%d%%)", ampdu->mpdu_histogram[i],
			(total == 0) ? 0 :
			(ampdu->mpdu_histogram[i] * 100 / total));
		if ((i % 8) == 0 && i != last)
			bcm_bprintf(b, "\n        :");
	}
	bcm_bprintf(b, "\n");

	if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
		for (i = 0, last = 0; i <= AMPDU_MAX_MPDU; i++)
			if (ampdu->retry_histogram[i]) last = i;
		bcm_bprintf(b, "Retry   :");
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, " %3d", ampdu->retry_histogram[i]);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
		bcm_bprintf(b, "\n");

		for (i = 0, last = 0; i <= AMPDU_MAX_MPDU; i++)
			if (ampdu->end_histogram[i]) last = i;
		bcm_bprintf(b, "Till End:");
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, " %3d", ampdu->end_histogram[i]);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
		bcm_bprintf(b, "\n");
	}

	if (WLC_SGI_CAP_PHY(ampdu->wlc)) {
		for (i = 0, max_val = 0, last = 0; i <= FFPLD_MAX_MCS; i++) {
			max_val += ampdu->rxmcssgi[i];
			if (ampdu->rxmcssgi[i]) last = i;
		}
		bcm_bprintf(b, "RX MCS SGI:");
		if (max_val) {
			for (i = 0; i <= 7 && i <= last; i++)
				bcm_bprintf(b, "  %d(%d%%)", ampdu->rxmcssgi[i],
				            (ampdu->rxmcssgi[i] * 100) / max_val);
			for (i = 8; i <= last; i++) {
				if ((i % 8) == 0)
					bcm_bprintf(b, "\n          :");
				bcm_bprintf(b, "  %d(%d%%)", ampdu->rxmcssgi[i],
				            (ampdu->rxmcssgi[i] * 100) / max_val);
			}
		}
		bcm_bprintf(b, "\n");

		bcm_bprintf(b, "TX MCS SGI:");
		for (i = 0, max_val = 0, last = 0; i <= FFPLD_MAX_MCS; i++) {
			max_val += ampdu->txmcssgi[i];
			if (ampdu->txmcssgi[i]) last = i;
		}
		if (max_val) {
			for (i = 0; i <= 7 && i <= last; i++)
				bcm_bprintf(b, "  %d(%d%%)", ampdu->txmcssgi[i],
				            (ampdu->txmcssgi[i] * 100) / max_val);
			for (i = 8; i <= last; i++) {
				if ((i % 8) == 0)
					bcm_bprintf(b, "\n          :");
				bcm_bprintf(b, "  %d(%d%%)", ampdu->txmcssgi[i],
				            (ampdu->txmcssgi[i] * 100) / max_val);
			}
		}
		bcm_bprintf(b, "\n");

		if (WLCISLCNPHY(ampdu->wlc->band) || (NREV_GT(ampdu->wlc->band->phyrev, 3) &&
			NREV_LE(ampdu->wlc->band->phyrev, 6)))
		{
			bcm_bprintf(b, "RX MCS STBC:");
			for (i = 0, max_val = 0, last = 0; i <= FFPLD_MAX_MCS; i++) {
				max_val += ampdu->rxmcsstbc[i];
				if (ampdu->rxmcsstbc[i]) last = i;
			}
			if (max_val) {
				for (i = 0; i <= 7 && i <= last; i++)
					bcm_bprintf(b, "  %d(%d%%)", ampdu->rxmcsstbc[i],
					            (ampdu->rxmcsstbc[i] * 100) / max_val);
			}
			bcm_bprintf(b, "\n");

			bcm_bprintf(b, "TX MCS STBC:");
			for (i = 0, max_val = 0, last = 0; i <= FFPLD_MAX_MCS; i++) {
				max_val += ampdu->txmcsstbc[i];
				if (ampdu->txmcsstbc[i]) last = i;
			}
			if (max_val) {
				for (i = 0; i <= 7 && i <= last; i++)
					bcm_bprintf(b, "  %d(%d%%)", ampdu->txmcsstbc[i],
					            (ampdu->txmcsstbc[i] * 100) / max_val);
			}
			bcm_bprintf(b, "\n");
		}
	}

	if (AMPDU_HOST_ENAB(ampdu->wlc->pub)) {
		bcm_bprintf(b, "MCS to AMPDU tables:\n");
		for (j = 0; j < NUM_FFPLD_FIFO; j++) {
			fifo = ampdu->fifo_tb + j;
			if (fifo->ampdu_pld_size || fifo->dmaxferrate) {
				bcm_bprintf(b, " FIFO %d: Preload settings: size %d dmarate %d "
					    "kbps\n", j, fifo->ampdu_pld_size, fifo->dmaxferrate);
				bcm_bprintf(b, "        ");
				for (i = 0; i <= FFPLD_MAX_MCS; i++) {
					bcm_bprintf(b, " %d", fifo->mcs2ampdu_table[i]);
					if ((i % 8) == 7 && i != FFPLD_MAX_MCS)
						bcm_bprintf(b, "\n       :");
				}
				bcm_bprintf(b, "\n");
			}
		}
		bcm_bprintf(b, "\n");
	}

	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_CUBBY(ampdu, scb);
			bcm_bprintf(b, "%s: max_pdu %d release %d\n",
				bcm_ether_ntoa(&scb->ea, eabuf),
				scb_ampdu->max_pdu, scb_ampdu->release);
#ifdef BCMDBG
			bcm_bprintf(b, "\trxholes %d rxstuck %d txdrop %d txstuck %d\n"
			            "\ttxaddbareq %d txaddbaresp %d txrlag %d sdurejected %d "
			            "txmpdu %d txbar %d\n\ttxreg_noack %d noba %d rxampdu %d "
			            "rxmpdu %d rxlegacy %d rxdup %d\n"
			            "\trxoow %d rxaddbaresp %d rxdelba %d rxbar %d\n",
			            scb_ampdu->rxholes, scb_ampdu->rxstuck, scb_ampdu->txdrop,
			            scb_ampdu->txstuck, scb_ampdu->txaddbareq,
			            scb_ampdu->txaddbaresp, scb_ampdu->txrlag,
			            scb_ampdu->sdurejected, scb_ampdu->txmpdu, scb_ampdu->txbar,
			            scb_ampdu->txreg_noack, scb_ampdu->noba, scb_ampdu->rxampdu,
			            scb_ampdu->rxmpdu, scb_ampdu->rxlegacy, scb_ampdu->rxdup,
			            scb_ampdu->rxoow, scb_ampdu->rxaddbaresp, scb_ampdu->rxdelba,
			            scb_ampdu->rxbar);
#endif /* BCMDBG */
		}
#ifdef WLPKTDLYSTAT
		/* Report Latency and packet loss statistics (for each AC) */
		for (ac = 0; ac < AC_COUNT; ac++) {

			delay_stats = &scb->delay_stats[ac];
			for (i = 0, total = 0, sum = 0; i < RETRY_SHORT_DEF; i++) {
				total += delay_stats->txmpdu_cnt[i];
				sum += delay_stats->delay_sum[i];
			}
			if (total) {
				bcm_bprintf(b, "%s:  PLoss %d/%d (%d%%)\n",
				 ac_names[ac], delay_stats->txmpdu_lost, total,
				  (delay_stats->txmpdu_lost * 100) / total);
				bcm_bprintf(b, "\tDelay stats in usec (avg/min/max): %d %d %d\n",
				 CEIL(sum, total), delay_stats->delay_min, delay_stats->delay_max);
				bcm_bprintf(b, "\tTxcnt Hist    :");
				for (i = 0; i < RETRY_SHORT_DEF; i++) {
					bcm_bprintf(b, "  %d", delay_stats->txmpdu_cnt[i]);
				}
				bcm_bprintf(b, "\n\tDelay vs Txcnt:");
				for (i = 0; i < RETRY_SHORT_DEF; i++) {
					if (delay_stats->txmpdu_cnt[i])
						bcm_bprintf(b, "  %d", delay_stats->delay_sum[i]/
						delay_stats->txmpdu_cnt[i]);
					else
						bcm_bprintf(b, "  -1");
				}
				bcm_bprintf(b, "\n");

				for (i = 0, max_val = 0, last = 0; i < WLPKTDLY_HIST_NBINS; i++) {
					max_val += delay_stats->delay_hist[i];
					if (delay_stats->delay_hist[i]) last = i;
				}
				last = 8 * (last/8 + 1) - 1;
				if (max_val) {
					bcm_bprintf(b, "\tDelay Hist \n\t\t:");
					for (i = 0; i <= last; i++) {
						bcm_bprintf(b, " %d(%d%%)", delay_stats->
						delay_hist[i], (delay_stats->delay_hist[i] * 100)
						 / max_val);
						if ((i % 8) == 7 && i != last)
							bcm_bprintf(b, "\n\t\t:");
					}
				}
				bcm_bprintf(b, "\n");
			}
		}
		bcm_bprintf(b, "\n");
#endif /* WLPKTDLYSTAT */
	}

#ifdef WLAMPDU_HW
	if (ampdu->wlc->clk && AMPDU_HW_ENAB(ampdu->wlc->pub) && b) {
		wlc_info_t* wlc = ampdu->wlc;
		d11regs_t* regs = wlc->regs;
		bcm_bprintf(b, "AGGFIFO regs: availcnt 0x%x\n", R_REG(wlc->osh, &regs->aggfifocnt));
		bcm_bprintf(b, "cmd 0x%04x stat 0x%04x cfgctl 0x%04x cfgdata 0x%04x mpdunum 0x%02x "
			"len 0x%04x bmp 0x%04x ackedcnt %d\n", R_REG(wlc->osh, &regs->aggfifo_cmd),
			R_REG(wlc->osh, &regs->aggfifo_stat),
			R_REG(wlc->osh, &regs->aggfifo_cfgctl),
			R_REG(wlc->osh, &regs->aggfifo_cfgdata),
			R_REG(wlc->osh, &regs->aggfifo_mpdunum),
			R_REG(wlc->osh, &regs->aggfifo_len),
			R_REG(wlc->osh, &regs->aggfifo_bmp),
			R_REG(wlc->osh, &regs->aggfifo_ackedcnt));
#if defined(WLCNT) && defined(WLAMPDU_MAC)
		bcm_bprintf(b, "driver statistics: aggfifo pending %d enque/cons %d %d",
			cnt->pending, cnt->enq, cnt->cons);
#endif
	}
#endif /* WLAMPDU_HW */
	bcm_bprintf(b, "\n");

	return 0;
}
#endif /* BCMDBG || WLTEST */

#ifdef BCMDBG
static void
ampdu_dump_ini(scb_ampdu_tid_ini_t *ini)
{
	printf("ba_state %d ba_wsize %d tx_in_transit %d tid %d rem_window %d\n",
		ini->ba_state, ini->ba_wsize, ini->tx_in_transit, ini->tid, ini->rem_window);
	printf("start_seq 0x%x max_seq 0x%x tx_exp_seq 0x%x bar_ackpending_seq 0x%x\n",
		ini->start_seq, ini->max_seq, ini->tx_exp_seq, ini->bar_ackpending_seq);
	printf("bar_ackpending %d free_me %d alive %d retry_bar %d\n",
		ini->bar_ackpending, ini->free_me, ini->alive, ini->retry_bar);
	printf("retry_head %d retry_tail %d retry_cnt %d\n",
	       ini->retry_head, ini->retry_tail, ini->retry_cnt);
	prhex("ackpending", &ini->ackpending[0], sizeof(ini->ackpending));
	prhex("barpending", &ini->barpending[0], sizeof(ini->barpending));
	prhex("txretry", &ini->txretry[0], sizeof(ini->txretry));
	prhex("retry_seq", (uint8 *)&ini->retry_seq[0], sizeof(ini->retry_seq));
}
#endif /* BCMDBG */

/* FOLLOWING FUNCTIONS 2B MOVED TO COMMON (ampdu and ba) CODE ONCE DELAYED BA IS REVIVED */

/* function to send addba req
 * Does not have any dependency on ampdu, so can be used for delayed ba as well
 */
static int
wlc_send_addba_req(wlc_info_t *wlc, struct scb *scb, uint8 tid, uint16 wsize,
	uint8 ba_policy, uint8 delba_timeout)
{
	dot11_addba_req_t *addba_req;
	uint16 tmp, start_seq;
	void *p;
	uint8 *pbody;
#if defined(STA) && defined(WME)
	wlc_bsscfg_t *cfg;
#endif /* defined(STA) && defined(WME) */

	ASSERT(wlc);
	ASSERT(scb);
	ASSERT(scb->bsscfg);
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	ASSERT(wsize <= AMPDU_TX_BA_MAX_WSIZE);

	if (wlc->block_datafifo)
		return BCME_NOTREADY;

	p = wlc_frame_get_action(wlc, FC_ACTION, &scb->ea, &scb->bsscfg->cur_etheraddr,
		&scb->bsscfg->BSSID, DOT11_ADDBA_REQ_LEN, &pbody, DOT11_ACTION_CAT_BLOCKACK);
	if (p == NULL)
		return BCME_NOMEM;

	addba_req = (dot11_addba_req_t *)pbody;
	addba_req->category = DOT11_ACTION_CAT_BLOCKACK;
	addba_req->action = DOT11_BA_ACTION_ADDBA_REQ;
	addba_req->token = (uint8)wlc->counter;
	/* token cannot be zero */
	if (!addba_req->token) {
		wlc->counter++;
		addba_req->token++;
	}

	tmp = ((tid << DOT11_ADDBA_PARAM_TID_SHIFT) & DOT11_ADDBA_PARAM_TID_MASK) |
		((wsize << DOT11_ADDBA_PARAM_BSIZE_SHIFT) & DOT11_ADDBA_PARAM_BSIZE_MASK) |
		((ba_policy << DOT11_ADDBA_PARAM_POLICY_SHIFT) & DOT11_ADDBA_PARAM_POLICY_MASK);
	htol16_ua_store(tmp, (uint8 *)&addba_req->addba_param_set);
	htol16_ua_store(delba_timeout, (uint8 *)&addba_req->timeout);
	start_seq = (SCB_SEQNUM(scb, tid) & (SEQNUM_MAX - 1)) << SEQNUM_SHIFT;
	htol16_ua_store(start_seq, (uint8 *)&addba_req->start_seqnum);

	WL_AMPDU_CTL(("wl%d: wlc_send_addba_req: seq 0x%x tid %d wsize %d\n",
		wlc->pub->unit, start_seq, tid, wsize));

	/* set same priority as tid */
	PKTSETPRIO(p, tid);
	wlc_sendmgmt(wlc, p, SCB_WLCIFP(scb)->qi, scb);

#if defined(STA) && defined(WME)
	/* If STA has PM with APSD enabled, then send null data frame as the trigger frame
	 * so that the addba response can be delivered
	 */
	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	if (BSSCFG_STA(cfg) && cfg->BSS && cfg->pm->PM != PM_OFF &&
	    AC_BITMAP_TST(scb->apsd.ac_delv, WME_PRIO2AC(tid))) {

		WL_ERROR(("wl%d: wlc_send_addba_req: sending null data frame on tid %d\n",
			wlc->pub->unit, tid));
		wlc_sendnulldata(wlc, &scb->ea, 0, 0, tid);
	}
#endif /* defined(STA) && defined(WME) */

	return 0;
}

/* function to send addba resp
 * Does not have any dependency on ampdu, so can be used for delayed ba as well
 */
static int
wlc_send_addba_resp(wlc_info_t *wlc, struct scb *scb, uint16 status,
	uint8 token, uint16 timeout, uint16 param_set)
{
	dot11_addba_resp_t *addba_resp;
	void *p;
	uint8 *pbody;
	uint16 tid;

	ASSERT(wlc);
	ASSERT(scb);
	ASSERT(scb->bsscfg);

	if (wlc->block_datafifo)
		return BCME_NOTREADY;

	p = wlc_frame_get_action(wlc, FC_ACTION, &scb->ea, &scb->bsscfg->cur_etheraddr,
		&scb->bsscfg->BSSID, DOT11_ADDBA_RESP_LEN, &pbody, DOT11_ACTION_CAT_BLOCKACK);
	if (p == NULL)
		return BCME_NOMEM;

	addba_resp = (dot11_addba_resp_t *)pbody;
	addba_resp->category = DOT11_ACTION_CAT_BLOCKACK;
	addba_resp->action = DOT11_BA_ACTION_ADDBA_RESP;
	addba_resp->token = token;
	htol16_ua_store(status, (uint8 *)&addba_resp->status);
	htol16_ua_store(param_set, (uint8 *)&addba_resp->addba_param_set);
	htol16_ua_store(timeout, (uint8 *)&addba_resp->timeout);

	WL_AMPDU_CTL(("wl%d: wlc_send_addba_resp: status %d param_set 0x%x\n",
		wlc->pub->unit, status, param_set));


	/* set same priority as tid */
	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	PKTSETPRIO(p, tid);

	wlc_sendmgmt(wlc, p, SCB_WLCIFP(scb)->qi, scb);

	return 0;
}

/* function to send delba.
 * Does not have any dependency on ampdu, so can be used for delayed ba as well
 */
static int
wlc_send_delba(wlc_info_t *wlc, struct scb *scb, uint8 tid, uint16 initiator, uint16 reason)
{
	dot11_delba_t *delba;
	uint16 tmp;
	void *p;
	uint8 *pbody;

	ASSERT(wlc);
	ASSERT(scb);
	ASSERT(scb->bsscfg);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	if (wlc->block_datafifo)
		return BCME_NOTREADY;

	p = wlc_frame_get_action(wlc, FC_ACTION, &scb->ea, &scb->bsscfg->cur_etheraddr,
		&scb->bsscfg->BSSID, DOT11_DELBA_LEN, &pbody, DOT11_ACTION_CAT_BLOCKACK);
	if (p == NULL)
		return BCME_NOMEM;

	delba = (dot11_delba_t *)pbody;
	delba->category = DOT11_ACTION_CAT_BLOCKACK;
	delba->action = DOT11_BA_ACTION_DELBA;
	tmp = ((tid << DOT11_DELBA_PARAM_TID_SHIFT) & DOT11_DELBA_PARAM_TID_MASK) |
		((initiator << DOT11_DELBA_PARAM_INIT_SHIFT) & DOT11_DELBA_PARAM_INIT_MASK);
	delba->delba_param_set = htol16(tmp);
	delba->reason = htol16(reason);

	WL_AMPDU_CTL(("wl%d: wlc_send_delba: tid %d initiator %d reason %d\n",
		wlc->pub->unit, tid, initiator, reason));

	/* set same priority as tid */
	PKTSETPRIO(p, tid);

	wlc_sendmgmt(wlc, p, SCB_WLCIFP(scb)->qi, scb);

	return 0;
}

/* function to send a bar.
 * Does not have any dependency on ampdu, so can be used for delayed ba as well
 */
static void *
wlc_send_bar(wlc_info_t *wlc, struct scb *scb, uint8 tid, uint16 start_seq,
	uint16 cf_policy, bool enq_only, bool *blocked)
{
	struct dot11_ctl_header *hdr;
	struct dot11_bar *bar;
	void *p;
	uint16 tmp;

	ASSERT(wlc);
	ASSERT(scb);
	ASSERT(scb->bsscfg);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	if (wlc->block_datafifo) {
		*blocked = TRUE;
		return NULL;
	}

	*blocked = FALSE;

	p = wlc_frame_get_ctl(wlc, DOT11_CTL_HDR_LEN + DOT11_BAR_LEN);
	if (p == NULL)
		return NULL;

	hdr = (struct dot11_ctl_header *)PKTDATA(wlc->osh, p);
	hdr->fc = htol16(FC_BLOCKACK_REQ);
	hdr->durid = 0;
	bcopy(&scb->ea, &hdr->ra, ETHER_ADDR_LEN);
	bcopy(&scb->bsscfg->cur_etheraddr, &hdr->ta, ETHER_ADDR_LEN);

	bar = (struct dot11_bar *)&hdr[1];
	tmp = tid << DOT11_BA_CTL_TID_SHIFT;
	tmp |= (cf_policy & DOT11_BA_CTL_POLICY_MASK);
	tmp |= DOT11_BA_CTL_COMPRESSED;
	bar->bar_control = htol16(tmp);
	bar->seqnum = htol16(start_seq << SEQNUM_SHIFT);

	WL_ERROR(("wl%d: wlc_send_bar: seq 0x%x tid %d\n", wlc->pub->unit, start_seq, tid));

	/* set same priority as tid */
	PKTSETPRIO(p, tid);

	if (wlc_sendctl(wlc, p, SCB_WLCIFP(scb)->qi, scb, TX_CTL_FIFO, 0, enq_only))
		return p;
	else
		return NULL;
}

void
wlc_sidechannel_init(ampdu_info_t *ampdu)
{
	wlc_info_t *wlc = ampdu->wlc;
#ifdef WLAMPDU_UCODE
	uint16 txfs_waddr; /* side channel base addr in 16 bit units */
	uint16 txfsd_addr; /* side channel descriptor base addr */
	const uint8 txfs_wsz[AC_COUNT] =
	        {10, 32, 4, 4}; /* Size of side channel fifos in 16 bit units */
	ampdumac_info_t *hagg;
	int i;
#endif /* WLAMPDU_UCODE */

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!(D11REV_IS(wlc->pub->corerev, 26) || D11REV_IS(wlc->pub->corerev, 29)) ||
	    !AMPDU_MAC_ENAB(wlc->pub)) {
		WL_INFORM(("wl%d: %s; NOT UCODE or HW aggregation not supported\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	ASSERT(ampdu);

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(wlc->pub)) {
		ASSERT(txfs_wsz[0] + txfs_wsz[1] + txfs_wsz[2] + txfs_wsz[3] <= TOT_TXFS_WSIZE);
		txfs_waddr = wlc_read_shm(ampdu->wlc, M_TXFS_PTR);
		txfsd_addr = (txfs_waddr + C_TXFSD_WOFFSET) * 2;
		for (i = 0; i < AC_COUNT; i++) {
			/* 16 bit word arithmetic */
			hagg = &(ampdu->hagg[i]);
			hagg->txfs_addr_strt = txfs_waddr;
			hagg->txfs_addr_end = txfs_waddr + txfs_wsz[i] - 1;
			hagg->txfs_wptr = hagg->txfs_addr_strt;
			hagg->txfs_rptr = hagg->txfs_addr_strt;
			hagg->txfs_wsize = txfs_wsz[i];
			/* write_shmem takes a 8 bit address and 16 bit pointer */
			wlc_write_shm(ampdu->wlc, C_TXFSD_STRT_POS(txfsd_addr, i),
				hagg->txfs_addr_strt);
			wlc_write_shm(ampdu->wlc, C_TXFSD_END_POS(txfsd_addr, i),
				hagg->txfs_addr_end);
			wlc_write_shm(ampdu->wlc, C_TXFSD_WPTR_POS(txfsd_addr, i), hagg->txfs_wptr);
			wlc_write_shm(ampdu->wlc, C_TXFSD_RPTR_POS(txfsd_addr, i), hagg->txfs_rptr);
			wlc_write_shm(ampdu->wlc, C_TXFSD_RNUM_POS(txfsd_addr, i), 0);
			WL_AMPDU_HW(("%d: start 0x%x 0x%x, end 0x%x 0x%x, w 0x%x 0x%x,"
				" r 0x%x 0x%x, sz 0x%x 0x%x\n",
				i,
				C_TXFSD_STRT_POS(txfsd_addr, i), hagg->txfs_addr_strt,
				C_TXFSD_END_POS(txfsd_addr, i),  hagg->txfs_addr_end,
				C_TXFSD_WPTR_POS(txfsd_addr, i), hagg->txfs_wptr,
				C_TXFSD_RPTR_POS(txfsd_addr, i), hagg->txfs_rptr,
				C_TXFSD_RNUM_POS(txfsd_addr, i), 0));
			hagg->txfs_wptr_addr = C_TXFSD_WPTR_POS(txfsd_addr, i);
			txfs_waddr += txfs_wsz[i];
		}
	}
#endif /* WLAMPDU_UCODE */

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		ampdu->aggfifo_depth = AGGFIFO_CAP - 1;
	}
#endif
}

#ifdef WLAMPDU_MAC
/* For ucode agg:
 * enqueue an entry to side channel (a.k.a. aggfifo) and inc the wptr
 * The actual capacity of the buffer is one less than the actual length
 * of the buffer so that an empty and a full buffer can be
 * distinguished.  An empty buffer will have the readPostion and the
 * writePosition equal to each other.  A full buffer will have
 * the writePosition one less than the readPostion.
 *
 * The Objects available to be read go from the readPosition to the writePosition,
 * wrapping around the end of the buffer.  The space available for writing
 * goes from the write position to one less than the readPosition,
 * wrapping around the end of the buffer.
 *
 * For hw agg: it simply enqueues an entry to aggfifo
 */
static int
aggfifo_enque(ampdu_info_t *ampdu, int length, int qid)
{
	ampdumac_info_t *hagg = &(ampdu->hagg[qid]);
	uint16 epoch = hagg->epoch;
	uint32 entry;

	if (length > (MPDU_LEN_MASK >> MPDU_LEN_SHIFT))
		WL_ERROR(("%s: Length too long %d\n", __FUNCTION__, length));

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
		d11regs_t *regs = ampdu->wlc->regs;

		entry = length |
			((epoch ? 1 : 0) << MPDU_EPOCH_HW_SHIFT) | (qid << MPDU_FIFOSEL_SHIFT);

		W_REG(ampdu->wlc->osh, &regs->aggfifodata, entry);
		WLCNTINCR(ampdu->cnt->enq);
		WLCNTINCR(ampdu->cnt->pending);
		hagg->in_queue ++;
		WL_AMPDU_HW(("%s: aggfifo %d entry 0x%04x\n", __FUNCTION__, qid, entry));
	}
#endif /* WLAMPDU_HW */

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(ampdu->wlc->pub)) {
		uint16 rptr = hagg->txfs_rptr;
		uint16 wptr_next;

		entry = length | (((epoch ? 1 : 0)) << MPDU_EPOCH_SHIFT);

		/* wptr always points to the next available entry to be written */
		if (hagg->txfs_wptr == hagg->txfs_addr_end) {
			wptr_next = hagg->txfs_addr_strt;	/* wrap */
		} else {
			wptr_next = hagg->txfs_wptr + 1;
		}

		if (wptr_next == rptr) {
			WL_ERROR(("write_mpduinfo_to_txfs: side channel %d is full !!!\n", qid));
			ASSERT(0);
			return -1;
		}

		/* Convert word addr to byte addr */
		wlc_write_shm(ampdu->wlc, hagg->txfs_wptr * 2, entry);

		WL_AMPDU_HW(("%s: aggfifo %d rptr 0x%x wptr 0x%x entry 0x%04x\n",
			__FUNCTION__, qid, rptr, hagg->txfs_wptr, entry));

		hagg->txfs_wptr = wptr_next;
		wlc_write_shm(ampdu->wlc, hagg->txfs_wptr_addr, hagg->txfs_wptr);
	}
#endif /* WLAMPDU_UCODE */

	WLCNTINCR(ampdu->cnt->enq);
	WLCNTINCR(ampdu->cnt->pending);
	hagg->in_queue ++;
	return 0;
}

/* For ucode agg:
 * Side channel queue is setup with a read and write ptr.
 * - R == W means empty.
 * - (W + 1 % size) == R means full.
 * - Max buffer capacity is size - 1 elements (one element remains unused).
 * For hw agg:
 * simply read ihr reg
 */
static uint
get_aggfifo_space(ampdu_info_t *ampdu, int qid)
{
	uint ret = 0;
	d11regs_t *regs;
	regs = ampdu->wlc->regs;

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(ampdu->wlc->pub)) {
		uint actual;
		ret = R_REG(ampdu->wlc->osh, &regs->aggfifocnt);
		switch (qid) {
			case 3:
				ret >>= 8;
			case 2:
				ret >>= 8;
			case 1:
				ret >>= 8;
			case 0:
				ret &= 0x7f;
				break;
			default:
				ASSERT(0);
		}
		actual = ret;

		if (ret >= (uint)(AGGFIFO_CAP - ampdu->aggfifo_depth))
			ret -= (AGGFIFO_CAP - ampdu->aggfifo_depth);
		else
			ret = 0;

		/* due to the txstatus can only hold 32 bits in bitmap, limit the size to 32 */
		WL_AMPDU_HW(("%s: fifo %d fifo availcnt %d ret %d\n", __FUNCTION__, qid, actual,
			ret));
	}
#endif	/* WLAMPDU_HW */
#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(ampdu->wlc->pub)) {
		if (ampdu->hagg[qid].txfs_wptr < ampdu->hagg[qid].txfs_rptr)
			ret = ampdu->hagg[qid].txfs_rptr - ampdu->hagg[qid].txfs_wptr - 1;
		else
			ret = (ampdu->hagg[qid].txfs_wsize - 1) -
			      (ampdu->hagg[qid].txfs_wptr - ampdu->hagg[qid].txfs_rptr);
		ASSERT(ret < ampdu->hagg[qid].txfs_wsize);
		WL_AMPDU_HW(("%s: fifo %d rptr %04x wptr %04x availcnt %d\n", __FUNCTION__,
			qid, ampdu->hagg[qid].txfs_rptr, ampdu->hagg[qid].txfs_wptr, ret));
	}
#endif /* WLAMPDU_UCODE */
	return ret;
}
#endif /* WLAMPDU_MAC */


#ifdef WLAMPDU_PRECEDENCE
static bool BCMFASTPATH
wlc_ampdu_prec_enq(wlc_info_t *wlc, struct pktq *q, void *pkt, int tid)
{
	uint8 prec;

	if (WLPKTTAG(pkt)->flags2 & WLF2_LOW_IMPORTANCE)
		prec = WLC_PRIO_TO_PREC(tid);
	else
		prec = WLC_PRIO_TO_HI_PREC(tid);

	return (wlc_prec_enq_head(wlc, q, pkt, prec, FALSE));
}

static void * BCMFASTPATH
wlc_ampdu_pktq_pdeq(struct pktq *pq, int tid)
{
	void *p;

	p = pktq_pdeq(pq, WLC_PRIO_TO_HI_PREC(tid));

	if (p == NULL)
		p = pktq_pdeq(pq, WLC_PRIO_TO_PREC(tid));

	return p;
}

#ifdef WLOVERTHRUSTER
static void * BCMFASTPATH
wlc_ampdu_pktq_pdeq_tail(struct pktq *pq, int tid)
{
	void *p;

	p = pktq_pdeq_tail(pq, WLC_PRIO_TO_PREC(tid));

	if (p == NULL)
		p = pktq_pdeq_tail(pq, WLC_PRIO_TO_HI_PREC(tid));

	return p;
}

static void * BCMFASTPATH
wlc_ampdu_pktq_ppeek_tail(struct pktq *pq, int tid)
{
	void *p;

	p = pktq_ppeek_tail(pq, WLC_PRIO_TO_PREC(tid));

	if (p == NULL)
		p = pktq_ppeek_tail(pq, WLC_PRIO_TO_HI_PREC(tid));

	return p;
}
#endif /* WLOVERTHRUSTER */

static void
wlc_ampdu_pktq_pflush(osl_t *osh, struct pktq *pq, int tid, bool dir, ifpkt_cb_t fn, int arg)
{
	pktq_pflush(osh, pq, WLC_PRIO_TO_HI_PREC(tid), dir, fn, arg);
	pktq_pflush(osh, pq, WLC_PRIO_TO_PREC(tid), dir, fn, arg);
}

#endif /* WLAMPDU_PRECEDENCE */
