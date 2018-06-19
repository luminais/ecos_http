/*
 * Common [OS-independent] rate selection algorithm of Broadcom
 * 802.11 Networking Adapter Device Driver.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_rate_sel.c,v 1.192.2.21 2011-02-07 10:15:21 Exp $
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>

#include <proto/802.11.h>
#include <d11.h>

#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_rate_sel.h>

#include <wl_dbg.h>

#ifdef WLLMAC
#include <wlc_lmac.h>
#endif

/*
 * First nibble to rate;
 * Second nibble to spatial/extended spatial probing;
 * Third to rx antenna.
 */
static uint8 ratesel_msglevel = 0;
#define RL_INFO(args)	do {if (WL_RATE_ON() && (ratesel_msglevel & RATESEL_MSG_INFO_VAL)) \
				printf args;} while (0)
#define RL_MORE(args)	do {if (WL_RATE_ON() && (ratesel_msglevel & RATESEL_MSG_MORE_VAL)) \
				printf args;} while (0)
#define RL_SP0(args)	do {if (WL_RATE_ON() && (ratesel_msglevel & RATESEL_MSG_SP0_VAL)) \
				printf args;} while (0)
#define RL_SP1(args)	do {if (WL_RATE_ON() && (ratesel_msglevel & RATESEL_MSG_SP1_VAL)) \
				printf args;} while (0)
#define RL_RXA0(args)	do {if (WL_RATE_ON() && (ratesel_msglevel & RATESEL_MSG_RXA0_VAL)) \
				printf args;} while (0)
#define RL_RXA1(args)	do {if (WL_RATE_ON() && (ratesel_msglevel & RATESEL_MSG_RXA1_VAL)) \
				printf args;} while (0)

#ifdef WL11N
#define MAXRATERECS	224		/* Maximum rate updates */
#else
#define MAXRATERECS	128		/* Maximum rate updates */
#define RATEREC_BYTE(a)	((a) >> 5)	/* raterec is 32-bit uint */
#define RATEREC_BITS(a)	((a) & 0x1F)
#endif /* WL11N */

enum {
	SP_NONE,	/* no spatial probing */
	SP_NORMAL,	/* family probing without antenna select */
	SP_EXT		/* extended spatial probing (with antenna select) */
};

enum {
	TXS_REG,
	TXS_PROBE,
	TXS_DISC
};

#ifdef WL11N
#define	MPDU_MCSENAB(state)	(state->mpdu_allowmcs)
#else
#define	MPDU_MCSENAB(state)	(FALSE)
#endif /* WL11N */

#ifdef WL11N
#define MAXSPRECS		24	/* Maximum spatial AMPDU updates (1 bit values) */
#define MAX_EXTSPRECS		30	/* Maximum extended spatial AMPDU updates (1 bit values) */

#define SPATIAL_M		18	/* Spatial probe window size */
#define SPFLGS_EN_MASK		0x0001
#define SPFLGS_VAL_MASK		0x0010
#define EXTSPATIAL_M		18	/* Extented Spatial Probing window size */
#define EXTSPFLGS_EN_MASK	0x0001  /* requesting extended spatial probing frames */
#define EXTSPFLGS_XPR_VAL_MASK	0x0010  /* x-probe setup valid */
#define EXTSPFLGS_IPR_VAL_MASK	0x0100  /* i-probe setup valid */
#define ANT_SELCFG_MAX_NUM	4

#define I_PROBE          0       /* ext spatial toggle value for i-probe */
#define X_PROBE          1       /* ext spatial toggle value for x-probe */

#define MAXTXANTRECS		24	/* Maximum antenna history updated */
#define TXANTHIST_M		18	/* Antenna history window size */
#define TXANTHIST_K		14	/* Threshold, must be > TXANTHIST_M */
#define TXANTHIST_NMOD		10	/* Number of BA between history updates */

#define CUR_RATESPEC(state)	(known_rspec[state->select_rates[state->rateid]] | \
				(state->sgi ? RSPEC_SHORT_GI : 0))
#else  /* WL11N */
#define CUR_RATESPEC(state)	(known_rspec[state->select_rates[state->rateid]])
#endif /* WL11N */

#define MCS_STREAM_MAX_UNDEF	0	/* not specified */
#define MCS_STREAM_MAX_1       	1	/* pick mcs rates with one streams */
#define MCS_STREAM_MAX_2       	2	/* pick mcs rates with up to two streams */
#define MCS_STREAM_MAX_3       	3	/* pick mcs rates with up to three streams */
#define MCS_STREAM_MAX_4       	4	/* pick mcs rates with up to four streams */

#define MCS_STREAM_MAX_MATCH(ss, mcs)	(\
	((ss == MCS_STREAM_MAX_1) && (((mcs) <= 7) || (mcs) == 32)) || \
	((ss == MCS_STREAM_MAX_2) && (((mcs) <= 15) || (mcs) == 32)) || \
	((ss == MCS_STREAM_MAX_3) && (((mcs) <= 23) || (mcs) == 32)) || \
	((ss == MCS_STREAM_MAX_4) && (((mcs) <= 31) || (mcs) == 32)))

/* bit flag for rate selection scope restriction */
#define RATESEL_CCK		0x1	/* cck rates can be selected */
#define RATESEL_OFDM		0x2	/* ofdm rates can be selected */
#define RATESEL_MCS_S1		0x4	/* mcs stream1 rates can be selected */
#define RATESEL_MCS_S2		0x8	/* mcs stream2 rates can be selected */
#define RATESEL_MCS_S3		0x10	/* mcs stream3 rates can be selected */
#define RATESEL_MCS_S4		0x20	/* mcs stream4 rates can be selected */
#define RATESEL_MCS_DUP		0x40	/* mcs rate dup(mcs32) can be selected */

#define RATESEL_DEF_LEGACY	(RATESEL_CCK | RATESEL_OFDM)
#define RATESEL_DEF_HT		(RATESEL_MCS_S1 | RATESEL_MCS_S2 | RATESEL_MCS_DUP)
#define RATESEL_DEF_HT2		(RATESEL_MCS_S3 | RATESEL_MCS_S4)

/* MAX_MCS_NUM is the ultimate limit per spec but that wastes lots of memory, increase as needed */
#define MAX_MCS_NUM_NOW		(1 + 8 * MCS_STREAM_MAX_3) /* 1 for m32. Total 25 */
#define MAX_SHORT_AMPDUS	6	/* max number of short AMPDUs when switching to
					 * next higher rate in vertical rate control
					 */

#ifndef UINT32_MAX
#define UINT32_MAX		0xFFFFFFFF
#endif

#define NOW(state)		(state->rsi->pub->now)

#ifdef WL11N
#define RSSI_LMT_CCK		(-81)

/* This is how we update PSR (Packet Success Ratio, = 1 - PER).
 * Notation:
 *     Xn -- new value, Xn-1 -- old value, Xt -- current sample, all in [0, 1]
 *     NF -- normalize factor.
 * Update equation (original form):
 *   Xn <-- Xn-1 + (Xt - Xn-1) / 2^alpha
 *
 * (1) Scale up Xn, Xn-1 and Xt by 2^NF since we don't use float.
 *     Let Y <-- (X << NF), we have
 *     Yn <-- Yn-1 + Yt/2^alpha - (Yn-1)/2^alpha.
 * (2) We need to carefully choose NF over alpha to retain certain
 *     accuracy as well as to avoid overflowing the uint32 representation
 *     when multiplying the phy rate in unit of 500Kbps (maximum 540, 0x21C).
 * (2-1) Choose alpha = 4 that provides the actual alpha in EWA, 0.9375.
 * (2-2) Choose NF such that Xt = 1/64 (64 is the maximum MPDUs per A-MPDU)
 *     will be counted in the update. Hence, NF >= 6+alpha = 10.
 * (2-3) Meanwhile, NF shall satisfy 0x21C << NF <= 0xFFFFFFFF
 *       ==> NF < 20 is a stricter condition.
 * (2-4) As the result, choose NF = 12 to allow certain range of fine tuning alpha.
 */
/* exponential moving average coefficients. Actual alpha = 15/16 = 0.9375. */
#define RATESEL_EMA_NF			12	/* normalization factor */
#define RATESEL_EMA_ALPHA_INIT		2	/* value to use before gotpkts becomes true */
#define RATESEL_EMA_ALPHA_DEFAULT	4	/* stage 1 alpha value */
#define RATESEL_EMA_ALPHA0_DEFAULT	2	/* stage 0 alpha value */
#define RATESEL_EMA_ALPHA_MAX		8
#define RATESEL_EMA_ALPHA_MIN		2
#define RATESEL_EMA_NUPD_THRES_DEF	10
#define MIN_NUPD_DEFAULT		30	/* in align with alpha_default */

#define PSR_UNINIT			0xFFFFFFFF	/* Mark the PSR value as uninitialized */
#define PSR_MAX				(1 << RATESEL_EMA_NF)
#define AGE_LMT(state)			(state->rsi->age_lmt)	/* 1 seconds, max cache time. */
#define NCFAIL_TRLD			4

/* Moving average equation:
 *  Y_n <-- Y_n-1 + (Y_t - Y_n-1) >> alpha
 * Input:
 *   psr = Y_n-1
 *   psr_cur = Y_t (= ntx_succ << NF / ntx)
 */
#define UPDATE_MOVING_AVG(p, cur, alpha) {*p -= (*p) >> alpha; *p += cur >> alpha;}
#endif /* WL11N */

#define DUMMY_UINT8			(0xFF)

/* local macros */
#define MODDECR(i, k)		(((i) > 0)       ? (i)-1 : (k)-1)
#define MODINCR(i, k)		(((i) < ((k)-1)) ? (i)+1 : 0)
#define LIMDEC(i, k)		(((i) > ((k)+1)) ? (i)-1 : (k))
#define LIMINC(i, k)		(((i) < ((k)-1)) ? (i)+1 : (k)-1)
#define LIMSUB(i, k, s)		(((i) > ((k)+(s))) ? (i)-(s) : (k))
#define LIMADD(i, k, s)		(((i) < ((k)-(s))) ? (i)+(s) : (k))
#define LIMINC_UINT32(k)	if ((k) != UINT32_MAX) (k)++

#define RATES_NUM_CCK		(4)		/* cck */
#define RATES_NUM_CCKOFDM	(12)		/* cck/ofdm */
#define RATES_NUM_CCKOFDMSMCS	(21)		/* cck/ofdm/single stream mcs */
#define RATES_NUM_MCS		(1 + 8 * MCS_STREAM_MAX_3)	/* mcs0-23 + mcs32 */
#define RATES_NUM_ALL	(RATES_NUM_CCKOFDM + RATES_NUM_MCS)	/* cck + ofdm + mcs0-23 + mcs32 */

#define ABURN_GOOD_MASK		3 /* Currently unused */
#define ABURN_SFBL		1 /* Short Retry limit for fallback rate */
#define ABURN_LFBL		1 /* Long retry limit for fallback rate */
#define CCK_MAX_RATEID		5 /* Max rate id for CCK is 11Mbps */

#define IS_CCK_RATE(rate_id)	(rate_id <= CCK_MAX_RATEID) /* we ignore 6 and 9 in rate sel */

enum {
	RATE_UP_ROW,
	RATE_DN_ROW,
	RATE_UDSZ
};

enum {
	PSR_CUR_IDX,	/* current rate */
	PSR_DN_IDX,	/* current rateid-1 */
	PSR_UP_IDX,	/* current rateid+1 */
	PSR_FBR_IDX,	/* the fallback rate */
	PSR_ARR_SIZE	/* number of records */
} psr_info_array_index;

/* Size optimization macros to help with dead code elimination */
#ifdef WL11N_20MHZONLY
#define IS_40BW(state)	0
#else
#define IS_40BW(state)	((state)->is40bw)
#endif

#ifdef WL11N_SINGLESTREAM
#define	SP_MODE(state)	SP_NORMAL
#else
#define	SP_MODE(state)	((state)->spmode)
#endif

/* iovar table */
enum {
	IOV_RATESEL_DUMMY, /* dummy one in order to register the module */
	IOV_RATESEL_MSGLEVEL,
	IOV_RATESEL_SISO,
	IOV_RATESEL_MAXSHORTAMPDUS,
	IOV_RATESEL_USEFBR,
	IOV_RATESEL_SP_ALGO,
	IOV_RATESEL_EMA_ALPHA,
	IOV_RATESEL_EMA_ALPHA0,
	IOV_RATESEL_EMA_FIXED,
	IOV_RATESEL_EMA_NUPD,
	IOV_RATESEL_MIN_NUPD,
	IOV_RATESEL_AGE_LMT,
	IOV_RATESEL_MIN_PSR,
	IOV_RATESEL_MEASURE_MODE,
	IOV_RATESEL_REFRATEID,
	IOV_RATESEL_RSSI,
	IOV_RATESEL_RSSI_LMT,
	IOV_RATESEL_RXANT,
	IOV_RATESEL_RXANT_PON_TT,
	IOV_RATESEL_RXANT_PON_TR,
	IOV_RATESEL_RXANT_POFF_TT,
	IOV_RATESEL_RXANT_POFF_TR,
	IOV_RATESEL_RXANT_AVGRATE,
	IOV_RATESEL_RXANT_CLEAR_AVGRATE
};

static const bcm_iovar_t ratesel_iovars[] = {
	{"ratesel_dummy", IOV_RATESEL_DUMMY, (IOVF_SET_DOWN), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

typedef struct psr_info {
	ratespec_t	rspec;
	uint32	psr;		/* packet succ rate, 1-per */
	uint	timestamp;	/* cache time */
} psr_info_t;

/* principle ratesel module local structure per device instance */
struct ratesel_info {
	wlc_info_t	*wlc;		/* pointer to main wlc structure */
	wlc_pub_t	*pub;		/* public common code handler */
	int	scb_handle;		/* scb cubby handle to retrieve data from scb */

#ifdef WL11N
	uint16  min_psr[AC_COUNT];	/* target minimum PSR, i.e., 1 - PER */
	bool    ratesel_sp_algo;	/* true to indicate throughput based algo */
	bool    measure_mode;		/* default: FALSE */
	uint8	ref_rateid;		/* For measure mode: 0-29, rateid, 30- mcs_id */
	uint8	psr_ema_alpha;		/* default: 4. the EMA algo param for <psr> */
	uint8	psr_ema_alpha0;		/* default: 2. alpha value for the first x updates. */
	uint8   ema_nupd_thres;		/* default: 10. threshold afterwards use alpha */
	bool    ema_fixed;		/* default: TRUE. whether allows to change alpha0 */
	uint32	min_nupd;		/* default: 30 */
	uint32	age_lmt;		/* default: 1 */
	int16	rssi_lmt;
	bool	use_rssi;		/* use rssi to limit the lowest rate to go */
	bool    usefbr;
	bool    siso_only;
	uint8   max_short_ampdus;

	/* tx antenna selection */
	bool	txant_sel;		/* enable/valid for tx antenna selection */
	uint8	txant_stats[8];		/* mimo antenna selection stats */
	uint8	txant_stats_num;	/* number of used stats (mimo antconfig's) */
	uint8	txant_max_idx;		/* index to max mimo antconfig */
	uint8	txant_hist[MAXTXANTRECS];
	uint8	txant_hist_nupd;
	uint8   txant_hist_id;

	/* rx antenna selection */
	bool    rxant_sel;		/* enable rx antenna selection */
	uint8   rxant_id;		/* current default rx antenna */
	uint8   rxant_probe_id;		/* rx antenna in probing */
	uint32  rxant_stats;		/* rx'd throughput at rxant_id */
	uint32  rxant_probe_stats;	/* rx'd throughput at rxant_probe_id */
	uint32  rxant_rxcnt;		/* rxpkt counter */
	uint32  rxant_txcnt;		/* txpkt update counter */
	uint16  rxant_pon_tt;		/* txcnt threshold to start probe */
	uint16  rxant_pon_tr;		/* rxcnt threshold to start probe */
	uint16  rxant_poff_tt;		/* txcnt threshold to stop probe */
	uint16  rxant_poff_tr;		/* rxcnt threshold to stop probe */
	bool    rxant_rate_en;		/* enable history record */
	uint32  rxant_rate[ANT_SELCFG_MAX_NUM];		/* collect history rate info */
	uint32  rxant_rate_cnt[ANT_SELCFG_MAX_NUM];	/* counter for each bin */
#endif /* WL11N */
};

/* per scb cubby info */
typedef struct scb_ratesel {
	struct scb	*scb; /* back pointer to scb */
	ratesel_info_t 	*rsi;	/* back pointer to ratesel module local structure */
	uint8   select_rates[WLC_NUMRATES+MAX_MCS_NUM_NOW];	/* the rateset in use */
	uint8   select_fbrates[WLC_NUMRATES+MAX_MCS_NUM_NOW];	/* corresponding fallback rate */
			 /* it would be best to store rspec directly, but that needs 4 X space */
	uint8	active_rates_num;	/* total active rates */
	uint32	nskip;		/* number of txstatus skipped due to epoch mismatch */
	uint32	nupd;		/* number of txstatus used since last rate change */
	uint8	epoch;		/* flag to filter out stale rate_selection info */
	uint8	rateid;		/* current rate id */
	uint8	gotpkts;	/* this is a bit flag, other flags can go here if needed */

#ifndef WL11N
	uint32	nofb[RATEREC_BYTE(MAXRATERECS)];	/* bit array to store fb/nonfb history */
	int32	nofbtotdn;	/* num of nofb update for rate_down algorithm */
	int32	nofbtotup;	/* num of nofb update for rate_up algorithm */
	uint8	nofbid;		/* last position updated in nofb ring */
	uint8	Mup;		/* windows size for computing nofbtotup */
	uint8	Mdn;		/* windows size for computing nofbtotdn */
	uint8	Kup;		/* threshold to step up rate */
	uint8	Kdn;		/* threshold to step down rate */
#endif /* WL11N */
#ifdef WLAFTERBURNER
	uint8	aburn;		/* whether in afterburner mode */
	uint8	filter;		/* a filter appiled on successful transimssion in aburn mode */
#endif /* WLAFTERBURNER */
	uint	*clronupd;	/* pointer to external uint we clear on epoch/rateindex change */
#ifdef WL11N
	uint	spmode;		/* spatial probe mode common to all scb */
	uint8   mcs_baseid;	/* starting from this index the rate becomes MCS */
	bool	is40bw;		/* TRUE: select rate for 40Mhz channel, FALSE: 20Mhz channel */
	bool	mpdu_allowmcs;	/* mcs rate can be used for MPDU(non-AMPDU) */
	bool	has_cck;	/* rateset has at least one cck rate */
	bool   	has_ss_sgi;	/* supports single-stream sgi (must be NONGF_CAP) */
	bool	has_sdm_sgi;	/* supports dual-stream sgi */
	bool	sgi;		/* current rate is using SGI (must be mcs7 or mcs15) */
	/* BA (A-MPDU) rate selection */
	uint8   mcs_streams;	/* mcs rate streams */
	uint32	mcs_nupd;	/* count # of ampdu txstatus used since last rate change */
	uint8	mcs_flags;	/* flags */
	uint32	mcs_sp_nupd;	/* spatial probe functionality */
	uint32	sp_nskip;	/* number of txstatus skipped in spatial probing */
	uint8	mcs_sp_stats[MAXSPRECS >> 3];	/* bit array to store spatial history */
	int8    mcs_sp_statc;	/* total spatial stats */
	uint8   mcs_sp_statid;	/* last position updated in mcs_sp_stats ring */
	int8    mcs_sp_col;     /* spatial probing column */
	int8    mcs_sp;         /* the mcs being probed */
	uint8   mcs_sp_id;	/* current spatial mcs id */
	uint8   mcs_sp_flgs;	/* flags */
	uint8   mcs_sp_Pmod;	/* poll modulo */
	uint8   mcs_sp_Poff;	/* poll offset */
	uint8	mcs_sp_K;	/* threshold to switch to other family */
	bool    mcs_short_ampdu;
	/* antenna selection */
	uint8   active_antcfg_num;    /* number of antenna configs */
	uint8   antselid;             /* current antenna selection id */
	uint8   antselid_extsp_ipr;   /* antenna select id for intra-family probe */
	uint8   antselid_extsp_xpr;   /* antenna select id for cross-family probe */
	uint8   extsp_xpr_id;     /* current spatial mcs id for cross-probe */
	uint32	extsp_nupd;       /* spatial probe counter cross-probe */
	uint16  extsp_flgs;       /* flags for extended spatial probing */
	uint8   extsp_statid;
	int8    extsp_statc;      /* total extended spatial stats */
	uint8   extsp_K;          /* threshold to transition to next config */
	uint8	extsp_stats[MAX_EXTSPRECS >> 3];	/* bit array to store ext spatial history */
	uint8   extsp_ixtoggle; /* toggle between intra-probe and cross-probe */
	uint16  extsp_Pmod;	    /* poll modulo */
	uint16  extsp_Poff;	    /* poll offset */
	/* Antcfg history */
	uint16  txs_cnt;
	/* Throughput-based rate selection */
	psr_info_t	psri[PSR_ARR_SIZE];	/* PSR information used in the legacy rate algo. */
	uint8	ncfails; /* consecutive ampdu transmission failures since the rate changes. */
	uint32  mcs_sp_thrt0;	/* thruput across diff. rates in the spatial family being probed */
	uint32  mcs_sp_thrt1;	/* thrtuput across diff. rates in the primary spatial family */
	uint32  extsp_thrt0; /* thruput across diff. rates in the spatial family being probed */
	uint32  extsp_thrt1; /* thruput across diff. rates in the other spatial family */
	uint16  min_psr;		/* target minimum PSR, i.e., 1 - PER */
#endif /* WL11N */
} scb_ratesel_t;

struct ratesel_cubby {
	scb_ratesel_t *scb_cubby;
};

#define INVALIDATE_TXH_CACHE(state) { wlc_txc_invalidate_cache(state->clronupd); }
#define SCB_RATESEL_INFO(rsi, scb) ((SCB_CUBBY((scb), (rsi)->scb_handle)))

#if defined(WME_PER_AC_TX_PARAMS)
#define SCB_RATESEL_CUBBY(rsi, scb, ac) 	\
	(&((struct ratesel_cubby *)SCB_RATESEL_INFO(rsi, scb))->scb_cubby[ac])
#define FID2AC(pub, fid) \
	(WME_PER_AC_MAXRATE_ENAB(pub) ? wme_fifo2ac[(fid) & TXFID_QUEUE_MASK] : 0)
#else /* WME_PER_AC_TX_PARAMS */
#define SCB_RATESEL_CUBBY(rsi, scb, ac)	\
	(((struct ratesel_cubby *)SCB_RATESEL_INFO(rsi, scb))->scb_cubby)
#define FID2AC(pub, fid) (0)
#endif /* WME_PER_AC_TX_PARAMS */

#ifdef WL11N
#define IS_SP_PKT(state, cur_mcs, tx_mcs) \
	(SP_MODE(state) == SP_NORMAL && (cur_mcs) != (tx_mcs))
#define IS_EXTSP_PKT(state, cur_mcs, tx_mcs, antselid) (SP_MODE(state) == SP_EXT && \
	((cur_mcs) != (tx_mcs) || (antselid) != (state->antselid)))
#endif /* WL11N */

/* table of known rspecs for legacy rate selection (not montonically ordered) */
static const ratespec_t known_rspec[RATES_NUM_ALL] = {
	CCK_RSPEC(2),
	CCK_RSPEC(4),
	CCK_RSPEC(11),
	OFDM_RSPEC(12),
	OFDM_RSPEC(18),
	CCK_RSPEC(22),
	OFDM_RSPEC(24),
	OFDM_RSPEC(36),
	OFDM_RSPEC(48),
	OFDM_RSPEC(72),
	OFDM_RSPEC(96),
	OFDM_RSPEC(108),
	MCS_RSPEC(32), /* MCS 32: SS 1, MOD: BPSK,  CR 1/2 DUP 40MHz only */
	MCS_RSPEC(0),  /* MCS  0: SS 1, MOD: BPSK,  CR 1/2 */
	MCS_RSPEC(1),  /* MCS  1: SS 1, MOD: QPSK,  CR 1/2 */
	MCS_RSPEC(2),  /* MCS  2: SS 1, MOD: QPSK,  CR 3/4 */
	MCS_RSPEC(3),  /* MCS  3: SS 1, MOD: 16QAM, CR 1/2 */
	MCS_RSPEC(4),  /* MCS  4: SS 1, MOD: 16QAM, CR 3/4 */
	MCS_RSPEC(5),  /* MCS  5: SS 1, MOD: 64QAM, CR 2/3 */
	MCS_RSPEC(6),  /* MCS  6: SS 1, MOD: 64QAM, CR 3/4 */
	MCS_RSPEC(7),  /* MCS  7: SS 1, MOD: 64QAM, CR 5/6 */
	MCS_RSPEC(8),  /* MCS  8: SS 2, MOD: BPSK,  CR 1/2 */
	MCS_RSPEC(9),  /* MCS  9: SS 2, MOD: QPSK,  CR 1/2 */
	MCS_RSPEC(10), /* MCS 10: SS 2, MOD: QPSK,  CR 3/4 */
	MCS_RSPEC(11), /* MCS 11: SS 2, MOD: 16QAM, CR 1/2 */
	MCS_RSPEC(12), /* MCS 12: SS 2, MOD: 16QAM, CR 3/4 */
	MCS_RSPEC(13), /* MCS 13: SS 2, MOD: 64QAM, CR 2/3 */
	MCS_RSPEC(14), /* MCS 14: SS 2, MOD: 64QAM, CR 3/4 */
	MCS_RSPEC(15), /* MCS 15: SS 2, MOD: 64QAM, CR 5/6 */
	MCS_RSPEC(16), /* MCS 16: SS 3, MOD: BPSK,  CR 1/2 */
	MCS_RSPEC(17), /* MCS 17: SS 3, MOD: QPSK,  CR 1/2 */
	MCS_RSPEC(18), /* MCS 18: SS 3, MOD: QPSK,  CR 3/4 */
	MCS_RSPEC(19), /* MCS 19: SS 3, MOD: 16QAM, CR 1/2 */
	MCS_RSPEC(20), /* MCS 20: SS 3, MOD: 16QAM, CR 3/4 */
	MCS_RSPEC(21), /* MCS 21: SS 3, MOD: 64QAM, CR 2/3 */
	MCS_RSPEC(22), /* MCS 22: SS 3, MOD: 64QAM, CR 3/4 */
	MCS_RSPEC(23), /* MCS 23: SS 3, MOD: 64QAM, CR 5/6 */
	};

#ifndef WL11N
static uint8 M_params[RATE_UDSZ][RATES_NUM_ALL] = {
	/* window size for rate to move up */
	{ 16,  16,  24,  32,  32,  32,  32,  48,  48,  96,  96, 106,  24, /* CCK OFDM */
	  26,  36,  48,  58,  80, 102, 114, 126,  /* m0-7 */
	  36,  58,  80, 102, 148, 192, 214,  244, /* m8-15 */
	  58,  80, 102, 148, 192, 214, 255,  63}, /* m16-23: coin some numbers */
	/* window size for rate to move down */
	{ 16,  16,  16,  16,  16,  16,  16,  16,  16,  24,  32,  32,  10, /* CCK OFDM */
	  10,  13,  15,  18,  24,  29,  32,  35, /* m0-7 */
	  13,  18,  24,  29,  40,  51,  57,  63, /* m8-15 */
	  15,  24,  29,  40,  51,  57,  63,  70  /* m16-23 */
	}
};

static uint8 K_params[RATE_UDSZ][RATES_NUM_ALL] = {
	/* threshold for rate to move up */
	{  3,   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
	3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
	3,   3,   3,   3,   3,   3,   3,   3},
	/* threshold for rate to move down */
	{  3,   4,   4,   4,   4,   4,   4,   4,   4,   3,   2,   2,   4,   4,   4,
	4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
	4,   4,   4,   4,   4,   4,   4,   4}
};

#ifdef WLAFTERBURNER
/* following matrices are for afterburner rate selection */
static uint8 M_aburn_params[RATE_UDSZ][RATES_NUM_ALL] = {
	{ 16,  16,  24,  64,  64,  64,  64,  96,  64,  96,  96, 106,  24, /* CCK OFDM */
	  26,  36,  48,  58,  80, 102, 114, 126,  /* m0-7 */
	  36,  58,  80, 102, 148, 192, 214, 244,  /* m8-15 */
	  58,  80, 102, 148, 192, 214, 255,  63}, /* m16-23: coin some numbers */
	{ 16,  16,  16,  16,  16,  24,  24,  32,  24,  32,  24,  24,  10, /* CCK OFDM */
	  10,  13,  15,  18,  24,  29,  32,  35,  /* m0-7 */
	  13,  18,  24,  29,  40,  51,  57,  63,  /* m8-15 */
	  15,  24,  29,  40,  51,  57,  63,  70   /* m16-23 */
	}
};

static uint8 K_aburn_params[RATE_UDSZ][RATES_NUM_ALL] = {
	/* threshold for rate to move up */
	{  3,   3,   3,   3,   3,   2,   1,   0,   1,   2,   2,   3,   3,   3,   3,
	3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
	3,   3,   3,   3,   3,   3,   3,   3},
	/* threshold for rate to move down */
	{  3,   4,   4,   4,   3,   4,   3,   3,   3,   4,   3,   3,   4,   4,   4,
	4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
	4,   4,   4,   4,   4,   4,   4,   4}
};
#endif /* WLAFTERBURNER */
#endif /* !WL11N */

#ifdef WL11N
/* spatial_MCS, spatial_K, spatial_Pmod, spatial_Poff */
enum {
	SPATIAL_MCS_COL1,
	SPATIAL_MCS_COL2,
	SPATIAL_K_COL,
	SPATIAL_PMOD_COL,
	SPATIAL_POFF_COL,
	SPATIAL_PARAMS_NUM
};

/* mcs 32 and mcs 0...23 */
static int mcs_sp_params[RATES_NUM_MCS][SPATIAL_PARAMS_NUM] = {
	{-1, -1, 2, 16, 6}, /* mcs 32 and legacy rates */
	{ 8, -1, 2, 16, 6}, /* mcs 0 */
	{ 8, -1, 2, 16, 6},
	{ 9, -1, 2, 17, 6},
	{ 9, -1, 2, 17, 6},
	{10, -1, 2, 18, 6},
	{11, -1, 2, 19, 6},
	{12, -1, 2, 19, 6},
	{12, -1, 2, 19, 6},
	{ 1, -1, 2, 16, 6},  /* mcs 8 */
	{ 3, -1, 2, 17, 6},
	{ 4, 17, 2, 18, 7},
	{ 5, 18, 2, 19, 7},  /* mcs 11 */
	{ 7, 19, 2, 30, 7},
	{-1, 20, 2, 40, 7},
	{-1, 20, 2, 50, 7},
	{-1, -1, 2, 60, 7},  /* mcs 15 */
	/* don't use spatial probe for 3-stream yet */
	{-1, -1, 2, 18, 8},  /* mcs 16 */
	{-1, 10, 2, 18, 8},
	{-1, 11, 2, 22, 8},
	{-1, 12, 2, 40, 8},  /* mcs 19 */
	{-1, 13, 2, 60, 8},
	{-1, -1, 2, 21, 8},
	{-1, -1, 2, 21, 8},
	{-1, -1, 2, 21, 8}   /* mcs 23 */
};

/* 
 * extended spatial probing: extspatial_MCS,
 * extspatial_K, extspatial_Pmod, extspatial_Poff
 */
enum {
	EXTSPATIAL_MCS_COL,
	EXTSPATIAL_K_COL,
	EXTSPATIAL_PMOD_COL,
	EXTSPATIAL_POFF_COL,
	EXTSPATIAL_PARAMS_NUM
};
static int mcs_extsp_params[RATES_NUM_MCS][EXTSPATIAL_PARAMS_NUM] = {
	{-1, 2, 16, 6}, /* mcs 32 */
	{ 8, 2, 16, 6}, /* mcs 0 */
	{ 8, 2, 16, 6},
	{ 9, 2, 17, 6},
	{ 9, 2, 17, 6},
	{10, 2, 18, 6},
	{11, 2, 19, 6},
	{12, 2, 19, 6},
	{12, 2, 19, 6}, /* mcs 8 */
	{ 1, 2, 16, 6},
	{ 3, 2, 16, 6},
	{ 4, 2, 17, 7},
	{ 5, 2, 18, 7},
	{ 7, 2, 40, 7},
	{-1, 2, 60, 7},
	{-1, 2, 100, 7},
	{-1, 2, 200, 7}, /* mcs 15 */
	/* don't use antenna probe for 3-stream yet */
	{-1, 2, 300, 8}, /* mcs 16 */
	{-1, 2, 300, 8},
	{-1, 2, 300, 8},
	{-1, 2, 300, 8},
	{-1, 2, 300, 8},
	{-1, 2, 300, 8},
	{-1, 2, 300, 8},
	{-1, 2, 300, 8}  /* mcs 23 */
};

enum {
	RXANT_UPD_REASON_TXCHG,
	RXANT_UPD_REASON_TXUPD,
	RXANT_UPD_REASON_RXUPD
};

#define RXANT_PON_TXCNT_MIN	40
#define RXANT_PON_RXCNT_MIN	200
#define RXANT_PON_TXCNT_MAX	(RXANT_PON_TXCNT_MIN * 16)
#define RXANT_PON_RXCNT_MAX	(RXANT_PON_RXCNT_MIN * 16)
#define RXANT_POFF_TXCNT_THRD	10
#define RXANT_POFF_RXCNT_THRD	20
#define RXANT_POFF_RXCNT_M	5

#endif /* WL11N */

static int wlc_ratesel_scb_init(void *context, struct scb *scb);
static void wlc_ratesel_scb_deinit(void *context, struct scb *scb);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#ifdef BCMDBG
static void wlc_ratesel_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#endif
static int wlc_ratesel_dump(ratesel_info_t *rsi, struct bcmstrbuf *b);
#endif /* BCMDBG || BCMDBG_DUMP */

static int wlc_ratesel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif);

static void wlc_ratesel_filter(scb_ratesel_t *state, wlc_rateset_t *rateset, uint8 start_rate);
static void wlc_ratesel_load_params(scb_ratesel_t *state);
static ratespec_t wlc_ratesel_getfbrspec(scb_ratesel_t *state, uint8 rateid);
static void wlc_ratesel_clear_ratestat(scb_ratesel_t *state, bool change_epoch);

static bool wlc_ratesel_godown(scb_ratesel_t *state);
static bool wlc_ratesel_goup(scb_ratesel_t *state);
static void wlc_ratesel_pick_rate(scb_ratesel_t *state, bool is_probe);

static void wlc_ratesel_upd_txs_normal(scb_ratesel_t *state, tx_status_t *txs,
	uint16 sfbl, uint16 lfbl, uint8 tx_mcs, uint8 antselid, bool fbr);
#ifdef WLAFTERBURNER
static void wlc_ratesel_upd_txs_aburn(scb_ratesel_t *state, tx_status_t *txs, uint8 weight,
	uint8 tx_mcs, uint8 antselid);
#endif /* WLAFTERBURNER */
static int wlc_ratesel_use_txs(scb_ratesel_t *state, tx_status_t *txs, uint16 SFBL, uint16 LFBL,
	uint8 tx_mcs, uint8 antselid, bool fbr);

#ifdef WL11N
static int wlc_ratesel_get_rssi(scb_ratesel_t *state);
static void wlc_rateset_filter_mcsset(uint8 src[], uint8 dst[],
                                       bool spatial_probing, uint8 nstreams);
static int wlc_ratesel_mcs2id(scb_ratesel_t *state, uint8 mcs);
static void wlc_ratesel_init_defantsel(scb_ratesel_t *state, uint8 txant_num, uint8 antselid_init);
static void wlc_ratesel_sanitycheck_psr(scb_ratesel_t *state);
static bool wlc_ratesel_next_rate(scb_ratesel_t *state, int8 incdec, uint8 *next_rateid,
       ratespec_t *next_rspec);

static bool wlc_ratesel_upd_spstat(scb_ratesel_t *state, bool blockack, uint8 sp_mcs,
	uint8 ntx_succ, uint8 ntx);
static bool wlc_ratesel_upd_extspstat(scb_ratesel_t *state, bool blockack,
	uint8 sp_mcs, uint8 antselid, uint8 ntx_succ, uint8 ntx);

static bool wlc_ratesel_start_probe(scb_ratesel_t *state);
static bool wlc_ratesel_change_sp(scb_ratesel_t *state);
static bool wlc_ratesel_change_extsp(scb_ratesel_t *state);
static void wlc_ratesel_clear_spstat(scb_ratesel_t *state);

static int wlc_ratesel_use_txs_blockack(scb_ratesel_t *state, tx_status_t *txs, uint8 suc_mpdu,
	uint8 tot_mpdu, bool ba_lost, uint8 retry, uint8 fb_lim, uint8 mcs, bool tx_error,
	uint8 antselid);
#ifdef WLAMPDU_MAC
static int wlc_ratesel_use_txs_ampdu(scb_ratesel_t *state, uint16 frameid,
	uint8 mrt, uint8 mrt_succ, uint8 fbr, uint8 fbr_succ, bool tx_error,
	uint8 tx_mcs, uint8 antselid);
#endif

static void wlc_ratesel_upd_deftxant(scb_ratesel_t *state, uint8 txant_new_idx);
static void wlc_ratesel_upd_rxantprb(ratesel_info_t *rsi, int reason_code);
static void wlc_ratesel_rxant_pon(ratesel_info_t *rsi);
static void wlc_ratesel_rxant_poff(ratesel_info_t *rsi, bool force_off);

#define	print_psri(state) \
	RL_MORE(("time %d UP [%s%d t %d p %d] CUR [%s%d t %d p %d] " \
		"DN [%s%d t %d p %d] FB [%s%d t %d p %d]\n", \
		NOW(state), \
		/* UP */ \
		IS_MCS(upp->rspec) ? "m " : "", \
		RSPEC_RATE_MASK & upp->rspec, upp->timestamp, upp->psr, \
		/* CUR */ \
		IS_MCS(cur->rspec) ? "m " : "", \
		RSPEC_RATE_MASK & cur->rspec, cur->timestamp, cur->psr, \
		/* DN */ \
		IS_MCS(dnp->rspec) ? "m " : "", \
		RSPEC_RATE_MASK & dnp->rspec, dnp->timestamp, dnp->psr, \
		/* FBR */ \
		IS_MCS(fbr->rspec) ? "m " : "", \
		RSPEC_RATE_MASK & fbr->rspec, dnp->timestamp, dnp->psr))
#endif /* WL11N */

ratesel_info_t *
BCMATTACHFN(wlc_ratesel_attach)(wlc_info_t *wlc)
{
	ratesel_info_t *rsi;
#ifdef WL11N
	int i;
#endif

	if (!(rsi = (ratesel_info_t *)MALLOC(wlc->osh, sizeof(ratesel_info_t)))) {
		WL_ERROR(("wl%d: wlc_ratesel_attach: out of mem, malloced %d bytes\n",
			wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)rsi, sizeof(ratesel_info_t));
	rsi->wlc = wlc;
	rsi->pub = wlc->pub;

	/* reserve cubby in the scb container for per-scb-ac private data */
	rsi->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct ratesel_cubby),
	                                        wlc_ratesel_scb_init,
	                                        wlc_ratesel_scb_deinit,
#ifdef BCMDBG
	                                        wlc_ratesel_dump_scb,
#else
	                                        NULL,
#endif
	                                        (void *)rsi);

	if (rsi->scb_handle < 0) {
		WL_ERROR(("wl%d: %s:wlc_scb_cubby_reserve failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(rsi->pub, ratesel_iovars, "ratesel", rsi, wlc_ratesel_doiovar,
		NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s:wlc_module_register failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(rsi->pub, "ratesel", (dump_fn_t)wlc_ratesel_dump, (void *)rsi);
#endif

#ifdef WL11N
	rsi->siso_only = FALSE;
	rsi->usefbr = TRUE;
	rsi->ratesel_sp_algo = TRUE;
	rsi->min_nupd = MIN_NUPD_DEFAULT;
	rsi->age_lmt = 1;
	rsi->psr_ema_alpha = RATESEL_EMA_ALPHA_DEFAULT;
	rsi->psr_ema_alpha0 = RATESEL_EMA_ALPHA0_DEFAULT;
	rsi->ema_fixed = FALSE;
	rsi->ema_nupd_thres = RATESEL_EMA_NUPD_THRES_DEF;
	rsi->measure_mode = FALSE;
	rsi->ref_rateid = 0;

	rsi->rssi_lmt = RSSI_LMT_CCK;
	rsi->use_rssi = TRUE;

	for (i = 0; i < AC_COUNT; i++) {
		rsi->min_psr[i] = 0;
	}

	WL_RATE(("%s: rsi %p psr_ema_alpha0 %d psr_ema_alpha %d rssi_lmt %d\n",
		__FUNCTION__, rsi, rsi->psr_ema_alpha0, rsi->psr_ema_alpha, rsi->rssi_lmt));

#endif /* WL11N */
	return rsi;

fail:
	MFREE(wlc->osh, rsi, sizeof(ratesel_info_t));
	return NULL;
}


void
BCMATTACHFN(wlc_ratesel_detach)(ratesel_info_t *rsi)
{
	if (!rsi)
		return;

	wlc_module_unregister(rsi->pub, "ratesel", rsi);

	MFREE(rsi->pub->osh, rsi, sizeof(ratesel_info_t));
}

/* nothing to do for scb init and deinit, wlc_ratesel_init() is the work fuc */
static int
wlc_ratesel_scb_init(void *context, struct scb *scb)
{
	ratesel_info_t *rsi = (ratesel_info_t *)context;
	struct ratesel_cubby *cubby_info = SCB_RATESEL_INFO(rsi, scb);
	scb_ratesel_t *scb_rate_cubby;
	int cubby_size;

#if defined(WME_PER_AC_TX_PARAMS)
	cubby_size = AC_COUNT * sizeof(scb_ratesel_t);
#else
	cubby_size = sizeof(scb_ratesel_t);
#endif

	WL_RATE(("%s scb %p\n", __FUNCTION__, scb));
	if (scb && !SCB_INTERNAL(scb)) {
		scb_rate_cubby = (scb_ratesel_t *)MALLOC(rsi->pub->osh, cubby_size);
		if (!scb_rate_cubby)
			return BCME_NOMEM;
		bzero(scb_rate_cubby, cubby_size);
		cubby_info->scb_cubby = scb_rate_cubby;
	}
	return 0;
}

static void
wlc_ratesel_scb_deinit(void *context, struct scb *scb)
{
	ratesel_info_t *rsi = (ratesel_info_t *)context;
	struct ratesel_cubby *cubby_info = SCB_RATESEL_INFO(rsi, scb);
	int cubby_size;

#if defined(WME_PER_AC_TX_PARAMS)
	cubby_size = (AC_COUNT * sizeof(scb_ratesel_t));
#else
	cubby_size = sizeof(scb_ratesel_t);
#endif

	if (rsi && cubby_info && !SCB_INTERNAL(scb) && cubby_info->scb_cubby) {
		MFREE(rsi->pub->osh, cubby_info->scb_cubby, cubby_size);
		cubby_info->scb_cubby = NULL;
	}
}

/* handle RATESEL related iovars */
static int
wlc_ratesel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif)
{
	ratesel_info_t *rsi = (ratesel_info_t *)hdl;
	int32 int_val = 0;
	bool bool_val = FALSE;
	uint8 uint8_val = 0;
	uint16 uint16_val = 0;
	uint32 uint32_val = 0;
	int err = 0;
	wlc_info_t *wlc;
	int32 *ret_int_ptr;

	ret_int_ptr = (int32 *)a;

	if (plen >= (int)sizeof(int_val)) {
		bcopy(p, &int_val, sizeof(int_val));
		bool_val = (bool)int_val;
		uint8_val = (uint8)int_val;
		uint16_val = (uint16)int_val;
		uint32_val = (uint32)int_val;
	}

	wlc = rsi->wlc;
	ASSERT(rsi == wlc->rsi);

	switch (actionid) {
		/* Note that if the IOV_GVAL() case returns an error, wl 
		   will call wlc_ioctrl() and come here again. Doesn't hurt.
		*/
	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

#ifdef BCMDBG
/* avoid adding too much for default scb dumping */
static void
wlc_ratesel_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	ratesel_info_t *rsi = (ratesel_info_t *)ctx;
	int32 ac;
	scb_ratesel_t *scb_ratesel;
	uint8 rateid;

	for (ac = 0; ac < WME_MAX_AC(rsi->wlc, scb); ac++) {
		scb_ratesel = SCB_RATESEL_CUBBY(rsi, scb, ac);

		if (!scb_ratesel)
			break;

		bcm_bprintf(b, "\tAC[%d] --- ", ac);

		rateid = scb_ratesel->rateid;

#ifndef WL11N
		if (scb_ratesel->rateid < scb_ratesel->active_rates_num) {
			bcm_bprintf(b,  "%s %d epoch %d skips %u nupds %u\n",
			IS_MCS(known_rspec[scb_ratesel->select_rates[rateid]]) ? "mcs" : "rate",
			RSPEC_RATE_MASK & known_rspec[scb_ratesel->select_rates[rateid]],
			scb_ratesel->epoch, scb_ratesel->nskip, scb_ratesel->nupd);
		} else {
			/* if current rateid is not in current select_rate_set */
			bcm_bprintf(b,  "rate NA epoch %d skips %u nupds %u\n",
			    scb_ratesel->epoch, scb_ratesel->nskip, scb_ratesel->nupd);
		}
#else
		bcm_bprintf(b, "spmode %d ", scb_ratesel->spmode);
		if (scb_ratesel->rateid < scb_ratesel->active_rates_num) {
			bcm_bprintf(b,  "%s %d sgi %d epoch %d skips %u nupds %u\n",
			IS_MCS(known_rspec[scb_ratesel->select_rates[rateid]]) ? "mcs" : "rate",
			RSPEC_RATE_MASK & known_rspec[scb_ratesel->select_rates[rateid]],
			scb_ratesel->sgi, scb_ratesel->epoch, scb_ratesel->nskip,
			scb_ratesel->nupd);
		} else {
			/* if current rateid is not in current select_rate_set */
			bcm_bprintf(b,  "rate NA sgi %d epoch %d skips %u nupds %u\n",
			    scb_ratesel->sgi, scb_ratesel->epoch, scb_ratesel->nskip,
			    scb_ratesel->nupd);
		}
#endif /* WL11N */
	}

	return;
}
#endif /* BCMDBG */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_ratesel_dump(ratesel_info_t *rsi, struct bcmstrbuf *b)
{

#ifdef WL11N
	uint8 i;

	if (rsi->measure_mode)
		bcm_bprintf(b, "In the measure mode. Ref_rateid %d\n", rsi->ref_rateid);

	bcm_bprintf(b, "EMA parameters: NF = %d alpha0 = %d alpha %d\n",
		RATESEL_EMA_NF, rsi->psr_ema_alpha0, rsi->psr_ema_alpha);

	if (rsi->txant_sel) {
		bcm_bprintf(b, "txant_stats = [");
		for (i = 0; i < rsi->txant_stats_num; i++) {
			bcm_bprintf(b, " %d", rsi->txant_stats[i]);
		}
		bcm_bprintf(b, "]\n");
		bcm_bprintf(b, "txant_max_idx = %d\n", rsi->txant_max_idx);
	}

	if (rsi->rxant_sel) {
		bcm_bprintf(b, "rxant: cnt_rx/tx %d %d id_main/probe %d %d "
			    "stats_main/probe %d %d\n", rsi->rxant_rxcnt,
			    rsi->rxant_txcnt, rsi->rxant_id, rsi->rxant_probe_id,
			    rsi->rxant_stats, rsi->rxant_probe_stats);
		if (rsi->rxant_rate_en) {
			for (i = 0; i < ANT_SELCFG_MAX_NUM; i++) {
				bcm_bprintf(b, "\nantid %d: cnt %d hist_rxrate %d",
					i, rsi->rxant_rate_cnt[i], rsi->rxant_rate[i]);
			}
		}
	}
#endif /* WL11N */
	return 0;
}

#endif /* BCMDGB || BCMDBG_DUMP */


/* return index of rspec in array known_rspec[] */
static uint8
wlc_ratesel_rspec2idx(scb_ratesel_t *state, ratespec_t rspec)
{
	uint8 i;
	uint8 limit;

#ifdef WL11N
	limit = RATES_NUM_ALL;
#else /* WL11N */
	limit = RATES_NUM_CCKOFDM;
#endif /* WL11N */

	/* search mcs in known_rspec to find the row number */
	for (i = 0; i < limit; i++) {
		if (known_rspec[i] == rspec)
			break;
	}

	if (i >= limit) {
		ASSERT(0);
		return 0;
	}

	return i;
}

static uint8
wlc_ratesel_find_startidx(scb_ratesel_t *state, uint8 start_rate)
{
	uint8 rate_entry = 0;
	ratespec_t rspec;
	uint32 phy_rate;

	/* Default is start_rate == 0, indicates start with highest rate */
	if (start_rate) {
		/* get the best match */
		/* state->selectrates are sorted */
		for (rate_entry = 0; rate_entry < state->active_rates_num; rate_entry++) {
			rspec = known_rspec[state->select_rates[rate_entry]];
#ifdef WL11N
			/* We would always endup with a single stream rate */
			if (IS_MCS(rspec)) {
				uint8 mcs = rspec & RSPEC_RATE_MASK;
				phy_rate = IS_40BW(state) ?
					((mcs_table[mcs].phy_rate_40 * 2) / 1000) :
					((mcs_table[mcs].phy_rate_20 * 2) / 1000);
			} else
#endif /* WL11N */
				{
					phy_rate =  rspec & RSPEC_RATE_MASK;
				}
			if (start_rate <= phy_rate)
				break;
		}
	}
	else {
		/* highest rate */
		rate_entry = state->active_rates_num;
	}

	/* index into the select_rates array */
	if (rate_entry)
		rate_entry--;

	return rate_entry;
}

/* Get the fallback rate of the <rateid>-th rate.
 * It is read from a look-up table constructed during the ratesel_init;
 * with an exception:
 *     If we just start (the main rate is the highest available mcs),
 *     use the lowest rate, which is 1 Mbps.
 */
static ratespec_t
wlc_ratesel_getfbrspec(scb_ratesel_t *state, uint8 rateid)
{


	return (state->gotpkts ? known_rspec[state->select_fbrates[rateid]] :
		known_rspec[state->select_rates[0]]);
}

#ifdef WL11N
/*
 * init and tool functions for WL11N
 */
/* return <mcs>'s offset in select_rates[] */
static int
wlc_ratesel_mcs2id(scb_ratesel_t *state, uint8 mcs)
{
	uint8 i;
	for (i = state->mcs_baseid; i < state->active_rates_num; i++) {
		if ((RSPEC_RATE_MASK & known_rspec[state->select_rates[i]]) == mcs)
			return i;
	}
	return -1;
}

static
int wlc_ratesel_get_rssi(scb_ratesel_t *state)
{
#if defined(AP)
	if (state->scb->rssi_enabled <= 0)
		WL_ERROR(("%s: scb %p rssi_enabled %d\n",
			__FUNCTION__, state->scb, state->scb->rssi_enabled));
	ASSERT(state->scb->rssi_enabled > 0);
	return wlc_scb_rssi(state->scb);
#else
	if (BSSCFG_STA(state->scb->bsscfg))
		return state->scb->bsscfg->link->rssi;
	else
		return 0;
#endif
}

static void
wlc_rateset_filter_mcsset(uint8 src[], uint8 dst[], bool spatial_probing, uint8 nstreams)
{
	int i;
	bzero(dst, MCSSET_LEN);

	for (i = 0; i < MIN(MCSSET_LEN, nstreams); i++) {
		dst[i] = src[i];
	}

	if (!spatial_probing) {
		/* For 2-stream:
		 * only include MCS 8 - 11 if spatial probing is enabled,
		 * otherwise just 12 - 15
		 * For 3-stream:
		 * on top of that, only include mcs21-23
		 */
		if (nstreams >= 2) {
			dst[1] = src[1] & 0xF0;
			if (nstreams >= 3)
				dst[2] = src[2] & 0xE0; /* m21-23 */
		}
	}

	/* MCS 32 */
	/* dst[4] = (src[4] & 0x01) */

	return;
}

/* default antenna configuration code */
static void
wlc_ratesel_init_defantsel(scb_ratesel_t *state, uint8 txant_num, uint8 antselid_init)
{
	/* initialize if we haven't already */
	ratesel_info_t *rsi = state->rsi;
	if ((txant_num <= 1) || rsi->txant_sel) {
		return;
	} else {
		int i;
		/* txant selection */
		rsi->txant_sel = TRUE;
		bzero((uchar *)rsi->txant_stats, sizeof(rsi->txant_stats));
		rsi->txant_stats_num = txant_num;
		rsi->txant_max_idx = antselid_init;

		rsi->txant_hist_id = 0;
		state->txs_cnt = 0;

		/* rxant selection */
		rsi->rxant_sel = FALSE;
		rsi->rxant_probe_id = rsi->rxant_id = antselid_init;
		rsi->rxant_pon_tt = RXANT_PON_TXCNT_MIN;
		rsi->rxant_pon_tr = RXANT_PON_RXCNT_MIN;
		rsi->rxant_poff_tt = RXANT_POFF_TXCNT_THRD;
		rsi->rxant_poff_tr = RXANT_POFF_RXCNT_THRD;

		rsi->rxant_rate_en = FALSE;
		for (i = 0; i < ANT_SELCFG_MAX_NUM; i++) {
			rsi->rxant_rate_cnt[i] = 0;
			rsi->rxant_rate[i] = 0;
		}
	}
}

/*
 * Return the fallback rate of the specified mcs rate.
 * Ensure that is a mcs rate too.
 */
ratespec_t
wlc_ratesel_getmcsfbr(ratesel_info_t *rsi, struct scb *scb, uint16 frameid, uint8 mcs)
{
	scb_ratesel_t *state;
	int ac, mcsid;
	ratespec_t mcsfbr;

	ASSERT(rsi != NULL);

	ac = FID2AC(rsi->pub, frameid);
	ASSERT(ac < AC_COUNT);
	state = SCB_RATESEL_CUBBY(rsi, scb, ac);
	ASSERT(state);

	mcsid = wlc_ratesel_mcs2id(state, mcs);
	if (mcsid != -1)
		mcsfbr = wlc_ratesel_getfbrspec(state, (uint8)mcsid);

	if (mcsid == -1 || !IS_MCS(mcsfbr))
		mcsfbr = known_rspec[state->select_rates[state->mcs_baseid]];

	return mcsfbr;
}

/* 
 * Function: return the rateid of the next up/down rate but doesn't change it.
 * <incdec> decides the up/down direction.
 */
static bool BCMFASTPATH
wlc_ratesel_next_rate(scb_ratesel_t *state, int8 incdec, uint8 *next_rateid, ratespec_t *next_rspec)
{
	bool set_sgi = FALSE;
	ratespec_t rspec = known_rspec[state->select_rates[state->rateid]];
	uint8 max_rate = known_rspec[state->select_rates[state->active_rates_num-1]]
		& RSPEC_RATE_MASK;

	/* if sgi bit is not allowed, then the internal bit shall be zero. */
	if (!state->has_ss_sgi && !state->has_sdm_sgi) {
		ASSERT(!state->sgi);
	}

	*next_rateid = state->rateid;

	if (incdec == +1) {
		if (IS_MCS(rspec)) {
			uint8 mcs = RSPEC_RATE_MASK & rspec;
			if (mcs == 7 && SP_MODE(state) != SP_NONE) {
				/* stay at mcs 7 (highest single stream rate) if no sgi */
				if (state->has_ss_sgi && !state->sgi)
					set_sgi = TRUE;
				else
					return FALSE;
			} else if (mcs == 15 && mcs != max_rate) {
				/* if mcs 15 jump to mcs 21 if supported */
				uint8 next = state->rateid + 1;
				for (; next < state->active_rates_num; next++) {
					if ((known_rspec[state->select_rates[next]]
					     & RSPEC_RATE_MASK) >= 21)
						break;
				}
				*next_rateid = next;
			} else if (mcs == max_rate) {
				/* stay at mcs 15 (highest double stream rate) if no sgi */
				if (state->has_sdm_sgi && !state->sgi)
					set_sgi = TRUE;
				else
					return FALSE;
			}
		}
		if (!set_sgi) {
			if (state->rateid == state->active_rates_num -1)
				return FALSE;
			else if (*next_rateid == state->rateid)
				*next_rateid = state->rateid + 1;
		}
	}
	else if (incdec == -1) {
		if (state->rateid == 0) return FALSE;

		*next_rateid = state->rateid - 1;
		if (IS_MCS(rspec)) {
			uint8 mcs = RSPEC_RATE_MASK & rspec;
			if (SP_MODE(state) != SP_NONE) {
				if (mcs == 8) {
					/* jump mcs 8 to 0 */
					*next_rateid = MAX(state->rateid - 8, 0);
				} else if (mcs == 16)
					/* jump mcs 16 to 1 */
					*next_rateid = MAX(state->rateid - 15, 0);
			} else if (state->sgi) {
				ASSERT(mcs == 7 || mcs == max_rate);
				*next_rateid = state->rateid; /* restore rateid */
			}
		}
		/* use rssi */
		else if (state->rsi->use_rssi && state->mpdu_allowmcs) {
			if (IS_CCK(rspec)) {
				if (wlc_ratesel_get_rssi(state) >= state->rsi->rssi_lmt) {
					*next_rateid = state->rateid; /* restore rateid */
					return FALSE;
				}
			}
		}
	} else
		/* shall not be called at all */
		return FALSE;

	/* generate new rspec */
	*next_rspec = known_rspec[state->select_rates[*next_rateid]];
	if (set_sgi)
		*next_rspec |= RSPEC_SHORT_GI;
	else
		*next_rspec &= (~RSPEC_SHORT_GI);

	return TRUE;

}
#endif /* WL11N init functions */

static void
wlc_ratesel_filter(scb_ratesel_t *state, wlc_rateset_t *rateset, uint8 start_rate)
{
	bool found_11mbps = FALSE;
	uint i, k;

	/* ordered list of legacy rates with/without 11Mbps */
	uint8 tbl_legacy_with11Mbps[] = {2, 4, 11, 22, 36, 48, 72, 96, 108};
	uint8 tbl_legacy_no11Mbps[] = {2, 4, 11, 12, 18, 24, 36, 48, 72, 96, 108};
	uint8 *ptbl;
	uint8 tbl_size;

	ASSERT(rateset->count > 0);

	/* check if we have 11Mbps CCK */
	for (i = 0; i < rateset->count; i++) {
		ASSERT(!(rateset->rates[i] & WLC_RATE_FLAG));
		if (rateset->rates[i] == WLC_RATE_11M) {
			found_11mbps = TRUE;
			break;
		}
	}

	/* init legacy rate adaptation for legacy and mcs rates :
	 *
	 * 1) scrub out legacy rateset entries that often aren't worth using.
	 *    Note: implemented as table lookup
	 * 2) initialize select_rates[] array with monotonic legacy rate set
	 */

	state->active_rates_num = 0;

#ifdef WL11N
	if (MPDU_MCSENAB(state)) {
		uint8 tbl_legacy_withMcs_20Mhz[] = {2, 4, 11};
		uint8 tbl_legacy_withMcs_40Mhz[] = {2, 4, 11, 22};
		uint8 filter_mcs_bitarray[MCSSET_LEN];

		if ((found_11mbps) && IS_40BW(state)) {
			tbl_size = ARRAYSIZE(tbl_legacy_withMcs_40Mhz);
			ptbl = tbl_legacy_withMcs_40Mhz;
		} else {
			tbl_size = ARRAYSIZE(tbl_legacy_withMcs_20Mhz);
			ptbl = tbl_legacy_withMcs_20Mhz;
		}

		for (k = 0; k < tbl_size; k++) {
			/* check if rate is available */
			for (i = 0; i < rateset->count; i++) {
				if (rateset->rates[i] == ptbl[k]) {
					state->select_rates[state->active_rates_num++] =
						wlc_ratesel_rspec2idx(state, LEGACY_RSPEC(ptbl[k]));
					break;
				}
			}
		}

		state->mcs_baseid = state->active_rates_num;

		/* 3) scrub out mcs set entries that often aren't worth using */
#ifdef WL11N
		wlc_rateset_filter_mcsset(rateset->mcs, filter_mcs_bitarray,
			SP_MODE(state) != SP_NONE,
			state->rsi->siso_only ? MCS_STREAM_MAX_1 : state->mcs_streams);
#else
		wlc_rateset_filter_mcsset(rateset->mcs, filter_mcs_bitarray,
			SP_MODE(state) != SP_NONE, state->mcs_streams);
#endif /* WL11N */
		/* 4) extend select_rates[] array with monotonic mcs rate set */

		if (isset(filter_mcs_bitarray, 32) && IS_40BW(state)) {
			state->select_rates[state->active_rates_num++] =
				wlc_ratesel_rspec2idx(state, MCS_RSPEC(32));
		}

		/* 4b) include mcs 0 to 15, if available */
		for (i = 0; i < (uint)(8 * state->mcs_streams); i++) {
			if (isset(filter_mcs_bitarray, i))
				state->select_rates[state->active_rates_num++] =
					wlc_ratesel_rspec2idx(state, MCS_RSPEC(i));
		}
	} else /* MPDU_MCSENAB */
#endif	/* WL11N */
	{
		if (found_11mbps) {
			tbl_size = ARRAYSIZE(tbl_legacy_with11Mbps);
			ptbl = tbl_legacy_with11Mbps;
		} else {
			tbl_size = ARRAYSIZE(tbl_legacy_no11Mbps);
			ptbl = tbl_legacy_no11Mbps;
		}

		for (k = 0; k < tbl_size; k++) {
			/* check if rate is available */
			for (i = 0; i < rateset->count; i++) {
				if (rateset->rates[i] == ptbl[k]) {
					state->select_rates[state->active_rates_num++] =
						wlc_ratesel_rspec2idx(state, LEGACY_RSPEC(ptbl[k]));
					break;
				}
			}
		}
	}

	/* make sure we have at least one rate available */
	ASSERT(state->active_rates_num > 0);

	/* init the fallback rate look-up table */
	for (i = 0; i < state->active_rates_num; i++) {
		int fbr_id = LIMSUB(i, 0, 2);
#ifdef WL11N
		if (IS_MCS(known_rspec[state->select_rates[i]])) {
			int k;
			uint8 mcs, fbr_mcs;
			mcs = RSPEC_RATE_MASK & known_rspec[state->select_rates[i]];

			/* if mcs is 0 or 1, use rateid - 2 */
			if (i > 0 && mcs >= 2) {
				fbr_mcs = mcs - 2;
				switch (mcs) {
				case 20:
				case 19:
				case 18:
				case 17: fbr_mcs = mcs - 9;  break;
				case 16: fbr_mcs = 0; break;
				case 13: fbr_mcs = 4; break;
				case 12: /* mcs 4 */
				case 11: /* mcs 3 */
				case 10: /* mcs 2 */
				case  9: /* mcs 1 */
				case  8: fbr_mcs = mcs - 8; break;
				case  7: fbr_mcs = 4; break;
				}

				/* search the largest index whose corresponding mcs/rate
				 * is lower than fbr_mcs/rate
				 */
				for (k = i-1; k >= 0; k--) {
					ratespec_t tmp_rspec = known_rspec[state->select_rates[k]];
					if (IS_MCS(tmp_rspec) &&
					    (RSPEC_RATE_MASK & tmp_rspec) <= fbr_mcs)
						break;
					else if (!IS_MCS(tmp_rspec)) {
						uint32 phy_rate;
						phy_rate = IS_40BW(state) ?
						  ((mcs_table[fbr_mcs].phy_rate_40 * 2) / 1000) :
						  ((mcs_table[fbr_mcs].phy_rate_20 * 2) / 1000);
						if ((tmp_rspec & RSPEC_RATE_MASK) <= phy_rate)
							break;
					}
				}
				fbr_id = k;
			}
		}
#endif /* WL11N */
		state->select_fbrates[i] = state->select_rates[fbr_id];
#ifdef WL11N
		ASSERT(RSPEC2RATE500K(known_rspec[state->select_rates[i]])
			>= RSPEC2RATE500K(known_rspec[state->select_fbrates[i]]));
#endif
		ASSERT(state->select_rates[i] > state->select_fbrates[i] || i == 0);
	}

	/* default behaviour: start at high rate, relying on ability to collapse quickly */
	state->rateid = wlc_ratesel_find_startidx(state, start_rate);
	state->gotpkts = 0;

	/* lookup the new up/down decision thresholds */
	wlc_ratesel_load_params(state);

#ifdef WL11N
	/* check if we have at least one CCK rate by checking lowest rate */
	state->has_cck = IS_CCK(known_rspec[state->select_rates[0]]) ? TRUE : FALSE;
	if (!MPDU_MCSENAB(state))
		state->mcs_baseid = state->active_rates_num;
#endif	/* WL11N */

	WL_RATE(("%s: init rateid = %d %d (500K)\n", __FUNCTION__,
		state->rateid, RSPEC2RATE500K(CUR_RATESPEC(state))));

	/* debug output of selected rate set */
	for (i = 0; i < state->active_rates_num; i++) {
		WL_RATE(("%s: select_rates[%d] = %s%d rspec 0x%x prate %d fbrates[%d] = %s%d\n",
			__FUNCTION__, i, (IS_MCS(known_rspec[state->select_rates[i]]) ? "m" : ""),
			RSPEC_RATE_MASK & known_rspec[state->select_rates[i]],
			known_rspec[state->select_rates[i]],
			RSPEC2RATE500K(known_rspec[state->select_rates[i]]),
			i, (IS_MCS(known_rspec[state->select_fbrates[i]]) ? "m" : ""),
			RSPEC_RATE_MASK & known_rspec[state->select_fbrates[i]]));
	}
}

#if defined(WME_PER_AC_TX_PARAMS)
/* Rates above the specified input value are removed from rateset */
static void
wlc_ratesel_filter_rateset(ratesel_info_t *rsi, wlc_rateset_t *rateset, wlc_rateset_t *new_rateset,
	bool is40bw, uint16 max_rate, uint16 min_rate)
{
	uint i, j = 0;

#ifdef WLLMAC
	if (LMAC_ENAB(rsi->wlc->pub))
		return;
#endif /* WLLMAC */

	bcopy(rateset, new_rateset, sizeof(wlc_rateset_t));

	/* No rate limiting required when max rate is zero */
	if (max_rate == 0)
		return;

	/* Eliminate the CCK/OFDM rates greater than max rate, less than min rate */
	for (i = 0; ((i < rateset->count) && (rateset->rates[i] != 0)); i++) {
		if ((rateset->rates[i] <= max_rate) && (rateset->rates[i] >= min_rate))
			new_rateset->rates[j++] = rateset->rates[i];
	}

	if (j > 0)
		new_rateset->count = j;

#ifdef WL11N
	/* Eliminate the MCS rates greater than max rate */
	for (i = 0; i < MCSSET_LEN; i++) {
		if (isset(rateset->mcs, i)) {
			uint32 phy_rate;

			if (is40bw)
				phy_rate = ((mcs_table[i].phy_rate_40 * 2) / 1000);
			else
				phy_rate = ((mcs_table[i].phy_rate_20 * 2) / 1000);

			if ((phy_rate > max_rate) || (phy_rate < min_rate))
				clrbit(new_rateset->mcs, i);
		}
	}
#endif /* WL11N */

	return;
}
#else /* WME_PER_AC_TX_PARAMS */
#define wlc_ratesel_filter_rateset(rsi, rateset, new_rateset, is40bw, max_rate, min_rate)
#endif /* WME_PER_AC_TX_PARAMS */

bool
wlc_ratesel_minrate(ratesel_info_t *rsi, struct scb *scb, tx_status_t *txs)
{
	scb_ratesel_t *state;
	uint8 epoch;
	int ac;

	ac = FID2AC(rsi->pub, txs->frameid);
	ASSERT(ac < AC_COUNT);
	state = SCB_RATESEL_CUBBY(rsi, scb, ac);
	ASSERT(state);
	epoch = (txs->frameid & TXFID_RATE_MASK) >> TXFID_RATE_SHIFT;

	return ((epoch == state->epoch) && (state->rateid == 0)) ? TRUE : FALSE;
}

/* initialize per-scb state utilized by rate selection
 *   ATTEN: this fcn can be called to "reinit", avoid dup MALLOC
 *   this new design makes this function the single entry points for any select_rates changes
 *   this function should be called when any its parameters changed: like bw or stream
 *   this function will build select_rspec[] with all constraint and rateselection will
 *      be operating on this constant array with reference to known_rspec[] for threshold
 */

void
wlc_ratesel_init(ratesel_info_t *rsi, struct scb *scb, wlc_rateset_t *rateset, bool is40bw,
	int8 sgi_tx,
#ifdef WLAFTERBURNER
	bool aburn,
#endif /* WLAFTERBURNER */
	uint *clronupd, uint8 active_antcfg_num,
	uint8 antselid_init, int32 ac)
{
	wlc_rateset_t new_rateset;
	uint8 init_rate = 0;
	scb_ratesel_t *state = SCB_RATESEL_CUBBY(rsi, scb, ac);

	if (state == NULL) {
		ASSERT(0);
		return;
	}

	/* Rates above per AC max rate, below per AC min are removed from the rateset */
#ifdef WLLMAC
	if (LMAC_ENAB(rsi->wlc->pub)) {
		if (rsi->wlc->lmac_info != NULL) {
			lmac_info_t *lmac = rsi->wlc->lmac_info;
			ASSERT(lmac != NULL);
			ASSERT(ac < AC_COUNT);
			if (lmac->lmac_tx_rate[ac].inited)
				bcopy(&lmac->lmac_tx_rate[ac].rateset, &new_rateset,
				      sizeof(wlc_rateset_t));
			else if (lmac->lmac_tx_rate[0].inited)
				bcopy(&lmac->lmac_tx_rate[0].rateset, &new_rateset,
				      sizeof(wlc_rateset_t));
			else
				bcopy(rateset, &new_rateset, sizeof(wlc_rateset_t));
			/* Nok UMAC may include mcs rates even for legacy assoc! */
			if (!lmac->lmac_bss_htcapable)
				bzero(&new_rateset.mcs[0], MCSSET_LEN);
		} else
			bcopy(rateset, &new_rateset, sizeof(wlc_rateset_t));
	}
	else
#endif /* WLLMAC */
	{
		if (WME_PER_AC_MAXRATE_ENAB(rsi->pub) && SCB_WME(scb))
			wlc_ratesel_filter_rateset(rsi, rateset, &new_rateset, is40bw,
				rsi->wlc->wme_max_rate[ac], 0);
		else
			bcopy(rateset, &new_rateset, sizeof(wlc_rateset_t));
	}

	bzero((char *)state, sizeof(scb_ratesel_t));

	/* store pointer to ratesel module */
	state->rsi = rsi;
	state->scb = scb;

	/* clronupd is a pointer to txc_cubby structure */
	state->clronupd = clronupd;
	INVALIDATE_TXH_CACHE(state);

#ifdef WLAFTERBURNER
	state->aburn = aburn;
#endif /* WLAFTERBURNER */


#ifdef WL11N
	if (BSS_N_ENAB(rsi->wlc, scb->bsscfg)) {
		if (((scb->ht_mimops_enabled && !scb->ht_mimops_rtsmode) ||
		     (rsi->wlc->stf->txstreams == 1) ||
		     (rsi->wlc->stf->siso_tx == 1)))
			state->mcs_streams = MCS_STREAM_MAX_1;
		else if ((new_rateset.mcs[2] & 0xff) != 0 && rsi->wlc->stf->txstreams > 2)
			state->mcs_streams = MCS_STREAM_MAX_3;
		else
			state->mcs_streams = MCS_STREAM_MAX_2;
	}

	state->is40bw = is40bw;
	state->mpdu_allowmcs = FALSE;
	state->spmode = SP_NONE;
	if (new_rateset.mcs[0] || new_rateset.mcs[1] || new_rateset.mcs[2]) {
		state->mpdu_allowmcs = TRUE;
		state->spmode = SP_NORMAL; /* enable SISO/MIMO spatial probing */
	}

	/* in extended spatial probing, set probe antennas based on initial antselid */
	state->active_antcfg_num = active_antcfg_num;
	if (active_antcfg_num > 1) {
		state->spmode = SP_EXT;
		state->antselid = antselid_init;
		state->antselid_extsp_ipr =
			MODINCR(antselid_init, state->active_antcfg_num);
		state->antselid_extsp_xpr = antselid_init;
		state->extsp_ixtoggle = I_PROBE;
	}

	/* init default antenna config selection */
	wlc_ratesel_init_defantsel(state, active_antcfg_num, antselid_init);

	/* init max number of short AMPDUs when going up in rate */
	state->rsi->max_short_ampdus = MAX_SHORT_AMPDUS;

	/* SGI: Init whether we can use SGI for single and dual stream rates */
	if (sgi_tx == AUTO) {
		state->has_ss_sgi = TRUE;
		state->has_sdm_sgi = TRUE;
	}

	/* init psri to zeros */
	bzero((uchar*) state->psri, PSR_ARR_SIZE * sizeof(psr_info_t));
	state->psri[PSR_CUR_IDX].psr = PSR_UNINIT;
	state->min_psr = state->rsi->min_psr[ac];
#endif /* WL11N */

	/* init rate set */
	wlc_ratesel_filter(state, &new_rateset, init_rate);

#ifdef WL11N
	/* init spatial probing family */
	if (state->spmode == SP_NORMAL) {
		ratespec_t curspec = known_rspec[state->select_rates[state->rateid]];
		state->mcs_sp_col = SPATIAL_MCS_COL1;
		state->mcs_sp = -1;
		if (IS_MCS(curspec)) {
			uint8 mcs = RSPEC_RATE_MASK & curspec;
			/* if start from 2 or 3 stream rates, probe between 2 and 3 stream rates */
			if (mcs & 0x38)
				state->mcs_sp_col = SPATIAL_MCS_COL2;
		}
	}
#endif /* WL11N */
	WL_RATE(("%s: scb 0x%p ac %d state 0x%p is40bw %d\n",
		__FUNCTION__, scb, ac, state, is40bw));
	WL_RATE(("\tactive_rates_num %d start_rateid %d sgi %s\n",
		state->active_rates_num, state->rateid, (sgi_tx == AUTO) ? "AUTO" : "OFF"));
	WL_RATE(("\t%s auto antenna selection. Init [num, id] = [%d, %d]\n",
		(active_antcfg_num >= 1) ? "have" : "no", active_antcfg_num, antselid_init));
#ifdef WL11N
	WL_RATE(("\tmcs_streams %d spmode %d ac %d min_psr %d\n",
		state->mcs_streams, state->spmode, ac, state->min_psr));
#endif

}

static void
wlc_ratesel_clear_ratestat(scb_ratesel_t *state, bool change_epoch)
{
	/*
	 * - bump the epoch
	 * - clear the skip count
	 * - clear the update count
	 * - set the bitfield to all 0's
	 * - set the counts to 0
	 * - lookup the new up/down decision thresholds
	 */
	if (change_epoch)
		state->epoch = MODINCR(state->epoch, 1 + (TXFID_RATE_MASK >> TXFID_RATE_SHIFT));

	state->nskip = 0;
	state->nupd = 0;
#ifdef WL11N
	state->mcs_nupd = 0;
#else
	bzero((uchar *)state->nofb, sizeof(state->nofb));
	state->nofbtotdn = 0;
	state->nofbtotup = 0;
	state->nofbid = 0;
#endif /* WL11N */

	wlc_ratesel_load_params(state);

	/* clear the external 'clear-on-update' variable */
	INVALIDATE_TXH_CACHE(state);

	return;
}

static void
wlc_ratesel_load_params(scb_ratesel_t *state)
{
#ifdef WL11N
	psr_info_t *upp = NULL, *cur = NULL, *dnp = NULL, *fbr = NULL;
	ratespec_t cur_rspec;
	int mcs, tblidx;

	mcs = IS_MCS(CUR_RATESPEC(state)) ? (int)(RSPEC_RATE_MASK & CUR_RATESPEC(state)) : -1;
	tblidx = (mcs == 32) ? 0 : (mcs + 1); /* offset by 1 */
	ASSERT(tblidx >= 0 && tblidx < RATES_NUM_MCS);

	/* get params for spatial probing */
	if (SP_MODE(state) == SP_NORMAL) {
		int mcs_sp = -1, mcs_sp_id = -1;
		uint8 tmp_col = state->mcs_sp_col;
		if (mcs != -1) {
			ASSERT(state->mcs_sp_col == SPATIAL_MCS_COL1 ||
			       state->mcs_sp_col == SPATIAL_MCS_COL2);
			/* switch only if the other col is valid */
			mcs_sp = mcs_sp_params[tblidx][state->mcs_sp_col];
			if (mcs_sp == -1) {
				tmp_col = (state->mcs_sp_col == SPATIAL_MCS_COL1) ?
					SPATIAL_MCS_COL2 : SPATIAL_MCS_COL1;
				mcs_sp = mcs_sp_params[tblidx][tmp_col];
			}
		}
		if (mcs_sp != -1) {
			mcs_sp_id = wlc_ratesel_mcs2id(state, (uint8) mcs_sp);
		}

		state->mcs_sp_flgs = 0; /* clear all the flags */
		if (mcs_sp_id > 0) {
			/* if the mcs stream family being probed has changed, clear stats */
			if ((mcs_sp & 0x78) != (state->mcs_sp & 0x78)) {
				RL_SP0(("%s: switching probing stream family: %d --> %d\n",
					__FUNCTION__, state->mcs_sp, mcs_sp));
				wlc_ratesel_clear_spstat(state);
			}
			state->mcs_sp_col = tmp_col;
			state->mcs_sp = (uint8)mcs_sp;
			state->mcs_sp_id = (uint8) mcs_sp_id;
			state->mcs_sp_flgs |= SPFLGS_VAL_MASK;
			state->mcs_sp_K = (uint8) mcs_sp_params[tblidx][SPATIAL_K_COL];
			state->mcs_sp_Pmod = (uint8) mcs_sp_params[tblidx][SPATIAL_PMOD_COL];
			state->mcs_sp_Poff = (uint8) mcs_sp_params[tblidx][SPATIAL_POFF_COL];
			RL_SP1(("%s: mcs %d sp_col %d mcs_sp %d sp_id %d\n", __FUNCTION__,
				mcs, state->mcs_sp_col, state->mcs_sp, mcs_sp_id));

		} else { /* don't probe at legacy rates or this mcs is not allowed */
			state->mcs_sp_id = state->rateid;
			state->mcs_sp_flgs &= ~SPFLGS_VAL_MASK;
		}

	} else if (SP_MODE(state) == SP_EXT) {
		/* X-Probe */
		int mcs_extsp_xpr = -1, mcs_extsp_xpr_sel = -1;
		if (mcs != -1)
			mcs_extsp_xpr = mcs_extsp_params[tblidx][EXTSPATIAL_MCS_COL];
		if (mcs_extsp_xpr > 0)
			mcs_extsp_xpr_sel = wlc_ratesel_mcs2id(state, (uint8) mcs_extsp_xpr);

		state->extsp_flgs = 0; /* clear all the flags */

		/* ext spatial x-probe only if corresp rate available */
		if (mcs_extsp_xpr_sel > 0) {
			state->extsp_xpr_id = (uint8) mcs_extsp_xpr_sel;
			state->extsp_flgs |= EXTSPFLGS_XPR_VAL_MASK;
		} else {
			state->extsp_xpr_id = 255; /* this is dummy entry */
			state->extsp_flgs &= ~EXTSPFLGS_XPR_VAL_MASK;
		}
		/* I-Probe */
		state->extsp_flgs |= EXTSPFLGS_IPR_VAL_MASK;
		state->extsp_K = (uint8) mcs_extsp_params[tblidx][EXTSPATIAL_K_COL];
		state->extsp_Pmod = (uint16) mcs_extsp_params[tblidx][EXTSPATIAL_PMOD_COL];
		state->extsp_Poff = (uint16) mcs_extsp_params[tblidx][EXTSPATIAL_POFF_COL];
	}

	/* init per info for the new rate */
	upp = (psr_info_t*)(state->psri + PSR_UP_IDX);
	cur = (psr_info_t*)(state->psri + PSR_CUR_IDX);
	dnp = (psr_info_t*)(state->psri + PSR_DN_IDX);
	fbr = (psr_info_t*)(state->psri + PSR_FBR_IDX);

	print_psri(state);

	/*
	 * It is not always true that cur becomes up or down rate,
	 * e.g. when doing the spatial probing: mcs 6/7 <--> mcs 12.
	 * The validity of up/dn will be checked when using it.
	 */
	cur_rspec = CUR_RATESPEC(state);
	if (cur->psr != PSR_UNINIT && RSPEC2RATE500K(cur_rspec) > RSPEC2RATE500K(cur->rspec)) {
		/* rate up : dn->fbr, cur->dn, up->cur and reset up */
		if (dnp->psr != PSR_UNINIT && (dnp->timestamp - NOW(state)) < AGE_LMT(state))
			bcopy((char*)dnp, (char*)fbr, sizeof(psr_info_t));
		else {
			fbr->psr = PSR_UNINIT;
		}
		bcopy((char*)cur, (char*)dnp, sizeof(psr_info_t));
		dnp->timestamp = NOW(state);
		if (upp->psr != PSR_UNINIT && (upp->timestamp - NOW(state)) < AGE_LMT(state) &&
		    upp->rspec == cur_rspec)
			bcopy((char*)upp, (char*)cur, sizeof(psr_info_t));
		else {
			cur->psr = PSR_MAX;
		}
		upp->psr = PSR_UNINIT;
	} else if (cur->psr != PSR_UNINIT &&
		RSPEC2RATE500K(cur_rspec) < RSPEC2RATE500K(cur->rspec)) {
		/* rate down : cur->up, dn->cur, fbr->dn and reset fbr */
		bcopy((char*)cur, (char*)upp, sizeof(psr_info_t));
		upp->timestamp = NOW(state);
		if (dnp->psr != PSR_UNINIT && (dnp->timestamp - NOW(state)) < AGE_LMT(state) &&
		    dnp->rspec == cur_rspec)
			bcopy((char*)dnp, (char*)cur, sizeof(psr_info_t));
		else {
			cur->psr = PSR_MAX;
		}
		bcopy((char*)fbr, (char*)dnp, sizeof(psr_info_t));
		fbr->psr = PSR_UNINIT;
	} else {
		/* First time come here OR the new rateid is the same as the current one 
		 * Init <cur>, reset <up>, <dn> and <fbr>
		 */
		cur->psr = PSR_MAX;
		fbr->psr = PSR_UNINIT;
		dnp->psr = PSR_UNINIT;
		upp->psr = PSR_UNINIT;
	}

	cur->rspec = cur_rspec;

	/* reset failed tx counter */
	state->ncfails = 0;

	print_psri(state);

#else /* WL11N */
	{
	uint8 rateidx = state->select_rates[state->rateid];

	/* udpate decision parameters */
#ifdef WLAFTERBURNER
	if (state->aburn) {
		state->Mup = M_aburn_params[RATE_UP_ROW][rateidx];
		state->Mdn = M_aburn_params[RATE_DN_ROW][rateidx];
		state->Kup = K_aburn_params[RATE_UP_ROW][rateidx];
		state->Kdn = K_aburn_params[RATE_DN_ROW][rateidx];
	} else
#endif /* WLAFTERBURNER */
	{
		state->Mup = M_params[RATE_UP_ROW][rateidx];
		state->Mdn = M_params[RATE_DN_ROW][rateidx];
		state->Kup = K_params[RATE_UP_ROW][rateidx];
		state->Kdn = K_params[RATE_DN_ROW][rateidx];
	}
	{
	uint8 up = 0, dn = 0;
	dn = state->select_rates[LIMDEC(state->rateid, 0)];
	up = state->select_rates[LIMINC((int)state->rateid, RATES_NUM_CCKOFDM)];
	RL_MORE(("%s: cr %s%d dn %s%d up %s%d rateidx %d"
		 " Mdn %d Mup %d Kdn %d Kup %d e %d\n", __FUNCTION__,
		 IS_MCS(known_rspec[rateidx]) ? "m" : "",
		 RSPEC_RATE_MASK & known_rspec[rateidx],
		 IS_MCS(known_rspec[dn]) ? "m" : "", RSPEC_RATE_MASK & known_rspec[dn],
		 IS_MCS(known_rspec[up]) ? "m" : "", RSPEC_RATE_MASK & known_rspec[up],
		 rateidx, state->Mdn, state->Mup,
		 state->Kdn, state->Kup, state->epoch));
	}
	}
#endif /* WL11N */
	return;
}

#ifdef WL11N
void
wlc_ratesel_aci_change(ratesel_info_t *rsi, bool aci_state)
{
	if (rsi->ema_fixed)
		return;

	if (aci_state) {
		RL_INFO(("%s: aci ON. alpha0: %d --> 4, alpha: %d --> 5\n",
			__FUNCTION__, rsi->psr_ema_alpha0, rsi->psr_ema_alpha));
	} else {
		RL_INFO(("%s: aci OFF. alpha0: %d --> %d, alpha: %d --> %d\n",
			__FUNCTION__, rsi->psr_ema_alpha0, RATESEL_EMA_ALPHA0_DEFAULT,
			rsi->psr_ema_alpha, RATESEL_EMA_ALPHA_DEFAULT));
	}

	if (aci_state) {
		rsi->psr_ema_alpha0 = 4;
		rsi->psr_ema_alpha = 5;
	} else {
		rsi->psr_ema_alpha0 = RATESEL_EMA_ALPHA0_DEFAULT;
		rsi->psr_ema_alpha = RATESEL_EMA_ALPHA_DEFAULT;
	}
}

/*
 * The sanity check on cur_rspec and fbr_rspec to consider
 * fallback rate frames after switching the rate without
 * pumping up the epoch (as the result of spatial probing).
 */
static void BCMFASTPATH
wlc_ratesel_sanitycheck_psr(scb_ratesel_t *state)
{
	ratespec_t fbrspec;
	fbrspec = wlc_ratesel_getfbrspec(state, state->rateid);

	if (state->psri[PSR_CUR_IDX].rspec != CUR_RATESPEC(state)) {
		RL_MORE(("%s: mismatch cur_rspec 0x%x record 0x%x rateid %d sgi %d\n",
			__FUNCTION__, CUR_RATESPEC(state),
			state->psri[PSR_CUR_IDX].rspec, state->rateid, state->sgi));
		state->psri[PSR_CUR_IDX].rspec = CUR_RATESPEC(state);
		state->psri[PSR_CUR_IDX].psr = PSR_MAX;
	}

	if (state->psri[PSR_FBR_IDX].rspec != fbrspec ||
	    state->psri[PSR_FBR_IDX].psr == PSR_UNINIT) {
		RL_MORE(("%s: mismatch/uninit fbrspec 0x%x record: fbrspec 0x%x psr %d\n",
			__FUNCTION__, fbrspec, state->psri[PSR_FBR_IDX].rspec,
			state->psri[PSR_FBR_IDX].psr));
		state->psri[PSR_FBR_IDX].rspec = fbrspec;
		state->psri[PSR_FBR_IDX].psr = PSR_MAX;
	}
}

static void
wlc_ratesel_clear_spstat(scb_ratesel_t *state)
{
	/* clear AMPDU spatial stats */
	state->sp_nskip = 0;
	state->mcs_sp_nupd = 0;
	bzero((uchar *)state->mcs_sp_stats, sizeof(state->mcs_sp_stats));
	state->mcs_sp_statc = 0;
	state->mcs_sp_statid = 0;

	/* throughput-based algorithm */
	state->mcs_sp_thrt0 = 0;
	state->mcs_sp_thrt1 = 0;

	RL_SP1(("%s\n", __FUNCTION__));
}

static void
wlc_ratesel_clear_extspstat(scb_ratesel_t *state)
{
	/* clear AMPDU spatial stats for the currently active probe (i or x) */
	state->sp_nskip = 0;
	state->extsp_nupd = 0;
	bzero((uchar *)state->extsp_stats, sizeof(state->extsp_stats));
	state->extsp_statc = 0;
	state->extsp_statid = 0;

	/* throughput-based algorithm */
	state->extsp_thrt0 = 0;
	state->extsp_thrt1 = 0;
}

/*
 * Update the spatial probing statistics
 * Return TRUE if the returned txstatus is a spatial probing frame.
 * Input:
 *   <blockack> : indicates whether it's an update on blockack
 */
static bool
wlc_ratesel_upd_spstat(scb_ratesel_t *state, bool blockack, uint8 sp_mcs, uint8 ntx_succ, uint8 ntx)
{
	if (ntx == 0 || sp_mcs == MCS_INVALID)
		return FALSE;

	if (state->rsi->ratesel_sp_algo) { /* throughput-based sp algo */

		uint32 cu_rate, sp_rate;
		uint32 thrt0, thrt1;
		ratespec_t sp_rspec;

		/* update throughput estimate. Don't count sgi in computing cu_rate */
		sp_rspec = MCS_RSPEC(sp_mcs);
		sp_rate = RSPEC2RATE500K(sp_rspec);
		cu_rate = RSPEC2RATE500K(known_rspec[state->select_rates[state->rateid]]);

		thrt0 = (ntx_succ << RATESEL_EMA_NF) * sp_rate / ntx;
		thrt1 = state->psri[PSR_CUR_IDX].psr * cu_rate;

		/* In case of different rate, scale down the higher one
		 * since big PER difference can lead to poor aggregation density.
		 */
		if (cu_rate > sp_rate)
			thrt1 = (thrt1 * 29) >> 5; /* 29/32 ~= 0.90625 */
		else if (cu_rate < sp_rate)
			thrt0 = (thrt0 * 29) >> 5;

		if (state->mcs_sp_nupd == 0) {
			state->mcs_sp_thrt0 = thrt0;
			state->mcs_sp_thrt1 = thrt1;
		} else if (state->mcs_sp_nupd < SPATIAL_M) {
			/* use accumulative average */
			state->mcs_sp_thrt0 = (state->mcs_sp_thrt0 * (state->mcs_sp_nupd - 1)
					       + thrt0) / state->mcs_sp_nupd;
			state->mcs_sp_thrt1 = (state->mcs_sp_thrt1 * (state->mcs_sp_nupd - 1)
					       + thrt1)	/ state->mcs_sp_nupd;
		} else {
			/* use moving average */
			UPDATE_MOVING_AVG(&state->mcs_sp_thrt0, thrt0, state->rsi->psr_ema_alpha);
			UPDATE_MOVING_AVG(&state->mcs_sp_thrt1, thrt1, state->rsi->psr_ema_alpha);
		}
		RL_SP0(("sp_nupd %u : (%s%d %d) -> (m%d %d) thrt_pri/sp 0x%x 0x%x sp_thrt_pri/sp "
			"0x%x 0x%x\n", state->mcs_sp_nupd, IS_MCS(CUR_RATESPEC(state)) ? "m" : "",
			RSPEC_RATE_MASK & CUR_RATESPEC(state), cu_rate, RSPEC_RATE_MASK & sp_rspec,
			sp_rate, thrt1, thrt0, state->mcs_sp_thrt1, state->mcs_sp_thrt0));
	} else { /* absolute PER base spatial probing algo */
		uint8 idx, shift, mask, val;
		uint16 stat = (ntx == ntx_succ) ? 1 : 0;
		int id;

		idx = state->mcs_sp_statid;
		shift = (idx & 0x7);
		mask = (0x1 << shift);
		val = ((uint8) (stat & 0x1)) << shift;
		state->mcs_sp_stats[idx >> 3] =
			(state->mcs_sp_stats[idx >> 3] & ~mask) | (val & mask);
		state->mcs_sp_statid = MODINC(state->mcs_sp_statid, MAXSPRECS);

		/* update spatial window totals */
		state->mcs_sp_statc += stat;

		id = state->mcs_sp_statid - SPATIAL_M - 1;
		if (id < 0)
			id += MAXSPRECS;
		shift = (id & 0x7);
		mask = (0x1 << shift);
		val = (state->mcs_sp_stats[id >> 3] & mask) >> shift;
		state->mcs_sp_statc -= val;
	}

	/* update the count of updates since the last state flush, which
	 * saturates at max word val
	 */
	LIMINC_UINT32(state->mcs_sp_nupd);
	return TRUE;
}


/*
 * Update the extended spatial probing statistics
 * Return TRUE if the returned txstatus is a spatial probing frame.
 * Input:
 *   <blockack> : indicates whether update on blockack
 *   <sp_mcs> : mcs if the pkt was tx'd at MCS rate, otherwise mark it as invalid (MCS_INVALID)
 */
static bool
wlc_ratesel_upd_extspstat(scb_ratesel_t *state, bool blockack, uint8 sp_mcs, uint8 antselid,
	uint8 ntx_succ, uint8 ntx)
{
	bool valid_probe = FALSE;
	uint8 cur_mcs = MCS_INVALID; /* sp_mcs will be the same if not MCS */
	ratespec_t cur_rspec = CUR_RATESPEC(state);
	uint8 extsp_rateid;

	if (ntx == 0)
		return FALSE;

	if (blockack)
		ASSERT(sp_mcs != MCS_INVALID && IS_MCS(cur_rspec));

	if (IS_MCS(cur_rspec))
		cur_mcs = RSPEC_RATE_MASK & cur_rspec;
	else
		ASSERT(state->extsp_ixtoggle == I_PROBE);

	/* is probe ACK of the type it needs to be? */
	if (state->extsp_ixtoggle == I_PROBE)
		valid_probe = (cur_mcs == sp_mcs && antselid == state->antselid_extsp_ipr);
	else if (state->extsp_ixtoggle == X_PROBE) {
		/* the later case in assertion should've been filter out by epoch */
		if (cur_mcs == MCS_INVALID || sp_mcs == MCS_INVALID) {
			WL_ERROR(("%s: cur_mcs %d sp_mcs %d antselid %d %d ntx_succ %d ntx %d e %d",
				__FUNCTION__, cur_mcs, sp_mcs, state->antselid, antselid,
				ntx_succ, ntx, state->epoch));
		}
		ASSERT(cur_mcs != MCS_INVALID && sp_mcs != MCS_INVALID);
		valid_probe = (cur_mcs != sp_mcs && antselid == state->antselid_extsp_xpr);
	}

	if (!valid_probe)
		return FALSE;

	extsp_rateid = (state->extsp_ixtoggle == I_PROBE) ?
		state->rateid : (state->extsp_xpr_id);

	if (state->rsi->ratesel_sp_algo) { /* throughput-based sp algo */

		uint32 cu_rate, sp_rate;
		uint32 thrt0, thrt1;

		/* update throughput estimate. Don't count sgi in computing cu_rate */
		sp_rate = RSPEC2RATE500K(known_rspec[state->select_rates[extsp_rateid]]);
		cu_rate = RSPEC2RATE500K(known_rspec[state->select_rates[state->rateid]]);

		thrt0 = (ntx_succ << RATESEL_EMA_NF) * sp_rate / ntx;
		thrt1 = state->psri[PSR_CUR_IDX].psr * cu_rate;

		/* In case of inequal rate scale down the higher one
		 * since big PER difference can lead to poor aggregation density.
		 */
		if (cu_rate > sp_rate)
			thrt1 = (thrt1 * 29) >> 5; /* 29/32 ~= 0.90625 */
		else if (cu_rate < sp_rate)
			thrt0 = (thrt0 * 29) >> 5;

		if (state->extsp_nupd == 0) {
			state->extsp_thrt0 = thrt0;
			state->extsp_thrt1 = thrt1;
		} else if (state->extsp_nupd < EXTSPATIAL_M) {
			/* use accumulative average */
			state->extsp_thrt0 = (state->extsp_thrt0 * (state->extsp_nupd - 1)
					      + thrt0)/state->extsp_nupd;
			state->extsp_thrt1 = (state->extsp_thrt1 * (state->extsp_nupd - 1)
					      + thrt1)/state->extsp_nupd;
		} else {
			/* use moving average -- actually due to early abortion, won't come here */
			UPDATE_MOVING_AVG(&state->extsp_thrt0, thrt0, state->rsi->psr_ema_alpha);
			UPDATE_MOVING_AVG(&state->extsp_thrt1, thrt1, state->rsi->psr_ema_alpha);
		}
		RL_SP0(("extsp_nupd %u : (%s%u %d %d) -> (%s%u %d %d) thrt_pri/sp 0x%x 0x%x "
			"extsp_thrt_pri/sp 0x%x 0x%x\n", state->extsp_nupd,
			(cur_mcs != MCS_INVALID) ? "m" : "", cur_mcs, cu_rate, state->antselid,
			(sp_mcs != MCS_INVALID) ? "m" : "", sp_mcs, sp_rate, antselid,
			thrt1, thrt0, state->extsp_thrt1, state->extsp_thrt0));
	} else {
		/* update extended spatial history */
		uint16 stat;
		uint8 idx, shift, mask, val;
		int id;

		stat = (ntx == ntx_succ) ? 1 : 0; /* stat=1 when o.k. */

		idx = state->extsp_statid;
		shift = (idx & 0x7);
		mask = (0x1 << shift);
		val = ((uint8) (stat & 0x1)) << shift;
		state->extsp_stats[idx >> 3] =
			(state->extsp_stats[idx >> 3] & ~mask) | (val & mask);
		state->extsp_statid = MODINC(state->extsp_statid, MAX_EXTSPRECS);
		state->extsp_statc += stat;

		id = state->extsp_statid - EXTSPATIAL_M - 1;
		if (id < 0)
			id += MAX_EXTSPRECS;
		shift = (id & 0x7);
		mask = (0x1 << shift);
		val = (state->extsp_stats[id >> 3] & mask) >> shift;
		state->extsp_statc -= val;

		/* Comment: Here, could consider immediately reenabling
		 * the probe for the case that the probe was successul
		 * ie for stat == 1; this might be even more useful if
		 * extended spatial probing targets higher than current
		 * rates.
		 */
	}

	/* update the count of updates since the last state flush, which
	 * saturates at max byte val
	 */
	LIMINC_UINT32(state->extsp_nupd);

	return TRUE;
}

/* Function to determine whether to move up in the rateset select_rates[] */
static bool
wlc_ratesel_godown(scb_ratesel_t *state)
{
	bool decision = FALSE;
	uint32 psr_dn = 0, prate_cur = 0, prate_dn = 0, prate_fbr = 0;
	uint8 down_rateid;
	ratespec_t down_rspec;
	psr_info_t *fbr, *cur, *dnp;

	if (!wlc_ratesel_next_rate(state, -1, &down_rateid, &down_rspec))
		return FALSE;

	prate_cur = RSPEC2RATE500K(CUR_RATESPEC(state));
	prate_dn = RSPEC2RATE500K(down_rspec);

	cur = (psr_info_t*)(state->psri + PSR_CUR_IDX);
	dnp = (psr_info_t*)(state->psri + PSR_DN_IDX);
	fbr = (psr_info_t*)(state->psri + PSR_FBR_IDX);

	if (state->ncfails >= NCFAIL_TRLD ||
	    (state->nupd >= state->rsi->min_nupd && cur->psr < state->min_psr)) {
		decision = TRUE;
		goto make_decision;
	}

	/* estimate the nominal throughput at the next rate */
	ASSERT(fbr->psr != PSR_UNINIT);

	/* it is possible when a spatial probe happened. */
	if (dnp->rspec != down_rspec && dnp->psr != PSR_UNINIT)
		dnp->psr = PSR_UNINIT;

	/* derive psr_dn from the fallback rate using linear interpolation */
	prate_fbr = RSPEC2RATE500K(fbr->rspec);

	if (prate_cur == prate_fbr) {
		RL_MORE(("%s: current rate == fallback rate 0x%x\n", __FUNCTION__, prate_cur));
		return FALSE;
	}

	ASSERT((prate_cur > prate_dn) && (prate_dn >= prate_fbr));
	if (prate_cur <= prate_dn || prate_dn < prate_fbr) {
		psr_dn = PSR_MAX;
	} else {
		psr_dn = (cur->psr * (prate_dn - prate_fbr) + fbr->psr * (prate_cur - prate_dn)) /
			(prate_cur - prate_fbr);
	}

	/* use the smaller of estimate and recent (if avail.) */
	if (dnp->psr != PSR_UNINIT && (NOW(state) - dnp->timestamp) < AGE_LMT(state) &&
	    psr_dn > dnp->psr) {
		RL_MORE(("%s: psr_dn use recent %d instead of estimate %d\n",
			__FUNCTION__, dnp->psr, psr_dn));
		psr_dn = dnp->psr;
	}
	ASSERT(psr_dn <= PSR_MAX);

	/* compare nominal throughputs at current and down rate */
	decision = (prate_dn * psr_dn > prate_cur * cur->psr);

make_decision:
	/* action time */
	if (decision) {
		/* change rate FIRST and do clean up work if necessary */
		ratespec_t prv_rspec = CUR_RATESPEC(state);
		state->rateid = down_rateid;
		state->sgi = RSPEC_ISSGI(down_rspec) ? TRUE : FALSE;
		if (IS_MCS(prv_rspec) && (RSPEC_RATE_MASK & prv_rspec) == 8) {
			if (SP_MODE(state) == SP_NORMAL)
				; //wlc_ratesel_clear_spstat(state); /* clear on way to mcs 0 */
			else if (SP_MODE(state) == SP_EXT) {
				wlc_ratesel_clear_extspstat(state);
				state->antselid_extsp_ipr = MODINCR(state->antselid,
					state->active_antcfg_num);
				state->antselid_extsp_xpr = state->antselid; /* reset prb */
			}
		}

#if defined(AP)
		if (state->rsi->use_rssi) {
			if ((IS_40BW(state) && prate_cur > 22 && prate_dn <= 22) ||
			     (!IS_40BW(state) && prate_cur > 11 && prate_dn <= 11))
			/* start to compute rssi */
			state->scb->rssi_enabled ++;
		}
#endif
		/* stop sending short A-MPDUs (if we still do) */
		state->mcs_short_ampdu = FALSE;

		if (state->ncfails >= NCFAIL_TRLD)
			RL_INFO(("%s: psr_cur %d DOWN upon %d consecutive failed tx\n",
				__FUNCTION__, cur->psr, state->ncfails));
		else
			RL_INFO(("%s: rate_cur/fbr/dn %d %d %d psr_cur/fbr/dn %d %d %d "
				"nthrt_cur/dn 0x%x 0x%x DOWN\n", __FUNCTION__,
				prate_cur, prate_fbr, prate_dn, cur->psr, fbr->psr,
				psr_dn, prate_cur * cur->psr, prate_dn * psr_dn));
	}
	else
		RL_MORE(("%s: rate_cur/fbr/dn %d %d %d psr_cur/fbr/dn %d %d %d nthrt_cur/dn "
			"0x%x 0x%x\n", __FUNCTION__, prate_cur, prate_fbr, prate_dn, cur->psr,
			fbr->psr, psr_dn, prate_cur * cur->psr, prate_dn * psr_dn));

	return decision;
}

static bool
wlc_ratesel_goup(scb_ratesel_t *state)
{
	bool decision = FALSE, use_hist = FALSE;
	uint32 psr_up, prate_cur, prate_up, prate_fbr;
	uint8 up_rateid = 0xFF;
	ratespec_t up_rspec, cur_rspec;
	psr_info_t *fbr, *cur, *upp;

	cur = (psr_info_t*)(state->psri + PSR_CUR_IDX);
	upp = (psr_info_t*)(state->psri + PSR_UP_IDX);
	fbr = (psr_info_t*)(state->psri + PSR_FBR_IDX);

	ASSERT(fbr->psr != PSR_UNINIT);

	/* have to collect enough sample for EMA to follow up */
	if (state->nupd < state->rsi->min_nupd)
		return FALSE;

	/* obtain the next-up rate first. If no change, return false. */
	if (!wlc_ratesel_next_rate(state, +1, &up_rateid, &up_rspec))
		return FALSE;

	cur_rspec = CUR_RATESPEC(state);
	prate_cur = RSPEC2RATE500K(cur_rspec);
	prate_up = RSPEC2RATE500K(up_rspec);
	prate_fbr = RSPEC2RATE500K(fbr->rspec);

	/* it is possible when a spatial probe happened. */
	if (upp->rspec != up_rspec && upp->psr != PSR_UNINIT)
		upp->psr = PSR_UNINIT;

	/* use the up rate psri if it is still recent */
	if (state->rateid != 0)
		ASSERT(fbr->rspec != cur_rspec);

	psr_up = upp->psr;
	if (psr_up != PSR_UNINIT && (NOW(state) - upp->timestamp) <= AGE_LMT(state))
		use_hist = TRUE;

	if ((prate_cur != prate_fbr) && !use_hist) {
		if (fbr->psr <= cur->psr) {
			/* if PSR at fbr is even lower than that at the current rate */
			psr_up = cur->psr;
		} else {
			/* derive psr_up from fallback rate using linear extrapolation */

			/* have to assume certain monitonicity */
			ASSERT(prate_up > prate_cur && prate_cur > prate_fbr);
			psr_up = (cur->psr * (prate_up - prate_fbr)
				  - fbr->psr * (prate_up - prate_cur)) / (prate_cur - prate_fbr);

			if (psr_up > PSR_MAX)
				psr_up = 0; /* negative */
		}
	}
	else if (use_hist) {
		RL_MORE(("%s: psr_up_recent %u\n", __FUNCTION__, psr_up));
	}

	if (prate_fbr == prate_cur && !use_hist) {
		/* probe once a while regardless the PSR. */
		decision = TRUE;
	} else {
		/* throughput criteria */
		decision = (prate_up * psr_up) > prate_cur * cur->psr;
		if (psr_up > PSR_MAX)
			WL_ERROR(("%s: psr_up %d exceeds maximum %d\n",
				__FUNCTION__, psr_up, PSR_MAX));
		ASSERT(psr_up <= PSR_MAX);
	}

	/* action time */
	if (decision) {
		/* change rate FIRST and do clean up work if necessary */
		ratespec_t prv_rspec = CUR_RATESPEC(state);
		state->rateid = up_rateid;
		state->sgi = RSPEC_ISSGI(up_rspec) ? TRUE : FALSE;
		if (IS_MCS(prv_rspec) && ((RSPEC_RATE_MASK & prv_rspec) == 12)) {
			if (SP_MODE(state) == SP_NORMAL) {
			}
			else if (SP_MODE(state) == SP_EXT &&
				state->extsp_ixtoggle == X_PROBE) {
				/* always go from X to fresh I probe on way to mcs13 */
				WL_RATE(("%s: clearing extsp stats on way to m13\n", __FUNCTION__));
				wlc_ratesel_clear_extspstat(state);
				state->extsp_ixtoggle = I_PROBE;
			}
		}

#if defined(AP)
		if (state->rsi->use_rssi) {
			if ((IS_40BW(state) && prate_cur <= 22 && prate_up > 22) ||
			     (!IS_40BW(state) && prate_cur <= 11 && prate_up > 11))
			/* stop to compute rssi */
			state->scb->rssi_enabled --;
		}
#endif

		/* start with short A-MPDUs */
		state->mcs_short_ampdu = TRUE;

		if (fbr->rspec == cur_rspec) {
			if (use_hist)
				RL_INFO(("%s: rate_cur/up %d %d psr_cur/up %d %d nthrt_cur/up "
					"0x%x 0x%x\n", __FUNCTION__, prate_cur, prate_up, cur->psr,
					psr_up, prate_cur * cur->psr, prate_up * psr_up));
			else
				RL_INFO(("%s: rate_cur/up %d %d psr_cur %d\n", __FUNCTION__,
					prate_cur, prate_up, cur->psr));
		} else {
			prate_fbr = RSPEC2RATE500K(fbr->rspec); /* wasn't init'd if using history */
			RL_INFO(("%s: rate_cur/fbr/up %d %d %d psr_cur/fbr/up %d %d %d "
				"nthrt_cur/up 0x%x 0x%x \n", __FUNCTION__, prate_cur, prate_fbr,
				prate_up, cur->psr, fbr->psr, psr_up, prate_cur * cur->psr,
				prate_up * psr_up));
		}
	}
	else {
		if (fbr->rspec == cur_rspec) {
			if (use_hist)
				RL_MORE(("%s: rate_cur/up %d %d psr_cur/up %d %d nthrt_cur/up "
					"0x%x 0x%x\n", __FUNCTION__, prate_cur, prate_up, cur->psr,
					psr_up, prate_cur * cur->psr, prate_up * psr_up));
			else
				RL_MORE(("%s: rate_cur/up %d %d psr_cur %d\n", __FUNCTION__,
					prate_cur, prate_up, cur->psr));
		} else {
			prate_fbr = RSPEC2RATE500K(fbr->rspec); /* wasn't init'd if using history */
			RL_MORE(("%s: rate_cur/fbr/up %d %d %d psr_cur/fbr/up %d %d %d "
				"nthrt_cur/up 0x%x 0x%x \n", __FUNCTION__, prate_cur, prate_fbr,
				prate_up, cur->psr, fbr->psr, psr_up, prate_cur * cur->psr,
				prate_up * psr_up));
		}
	}
	return decision;
}

static bool
wlc_ratesel_start_probe(scb_ratesel_t *state)
{
	if (SP_MODE(state) == SP_NORMAL) {
		/* inject spatial probe packet for current mcs_id, if available */
		if (state->mcs_sp_flgs & SPFLGS_VAL_MASK) {
			ASSERT(state->mcs_sp_Pmod != 0);
			ASSERT(state->mcs_sp_id < state->active_rates_num);
			state->mcs_sp_flgs &= ~SPFLGS_EN_MASK;
			if ((state->nupd % state->mcs_sp_Pmod) == state->mcs_sp_Poff) {
				state->mcs_sp_flgs |= SPFLGS_EN_MASK;
				RL_SP1(("start spatial probe to m%d (pri: m%d) "
					"[%d, %d %d]\n", state->mcs_sp_id - state->mcs_baseid,
					state->rateid - state->mcs_baseid, state->mcs_sp_K,
					state->mcs_sp_Pmod, state->mcs_sp_Poff));
				return TRUE;
			}
		}
	} else if (SP_MODE(state) == SP_EXT) {
		ASSERT(state->extsp_Pmod != 0);
		if (state->extsp_Pmod == 0)
			return FALSE;
		if ((state->nupd % state->extsp_Pmod) == state->extsp_Poff) {
			/* inject extended spatial probe packet for current mcs_id, if available */
			uint16 valid;

			state->extsp_flgs &= ~EXTSPFLGS_EN_MASK;
			if (state->extsp_ixtoggle == I_PROBE)
				valid = state->extsp_flgs & EXTSPFLGS_IPR_VAL_MASK;
			else
				valid = state->extsp_flgs & EXTSPFLGS_XPR_VAL_MASK;
			if (valid) {
				state->extsp_flgs |= EXTSPFLGS_EN_MASK;
				if (WL_RATE_ON()) { /* no enough room to indent */

				ratespec_t cur_rspec = CUR_RATESPEC(state);
				uint32 cur_rate = RSPEC_RATE_MASK & cur_rspec;
				bool is_mcs = IS_MCS(cur_rspec);
				ratespec_t extsp_rspec =
					known_rspec[state->select_rates[state->extsp_xpr_id]];
				if (state->extsp_ixtoggle == I_PROBE)
					RL_SP1(("start ext_spatial I-probe to ant %d "
						 "(pri: %s%d %d) [%d, %d %d]\n",
						 state->antselid_extsp_ipr, is_mcs ? "m" : "",
						 cur_rate, state->antselid, state->extsp_K,
						 state->extsp_Pmod, state->extsp_Poff));
				else {
					RL_SP1(("start ext_spatial X-probe to m%d ant %d "
						 "(pri: m%d %d) [%d, %d %d]\n",
						 RSPEC_RATE_MASK & extsp_rspec,
						 state->antselid_extsp_xpr, cur_rate,
						 state->antselid, state->extsp_K,
						 state->extsp_Pmod, state->extsp_Poff));
				}
				}
			return TRUE;
			}
		}
	}

	return FALSE;
}

void
wlc_ratesel_probe_ready(ratesel_info_t *rsi, struct scb *scb, uint16 frameid,
	bool is_ampdu, uint8 ampdu_txretry)
{
	scb_ratesel_t *state;
	int ac;

	ASSERT(rsi != NULL);

	ac = FID2AC(rsi->pub, frameid);
	ASSERT(ac < AC_COUNT);
	state = SCB_RATESEL_CUBBY(rsi, scb, ac);
	if (state == NULL) {
		ASSERT(0);
		return;
	}

	/* stop probing as soon as first probe is ready for DMA */
	if (ampdu_txretry == 0) {

		/* clear the external 'clear-on-update' variable */
		if ((state->mcs_sp_flgs & SPFLGS_EN_MASK) == 1)
			INVALIDATE_TXH_CACHE(state);

		/* stop spatial probe */
		state->mcs_sp_flgs &= ~SPFLGS_EN_MASK;

		/* clear the external 'clear-on-update' variable */
		if ((state->extsp_flgs & EXTSPFLGS_EN_MASK) == 1)
			INVALIDATE_TXH_CACHE(state);

		/* stop spatial probe */
		state->extsp_flgs &= ~EXTSPFLGS_EN_MASK;
	}
}

static void
wlc_ratesel_toggle_probe(scb_ratesel_t *state)
{
	/* clear spatial stats */
	wlc_ratesel_clear_extspstat(state);

	/* toggle probe */
	state->extsp_ixtoggle = (1 - state->extsp_ixtoggle);

	/* enforce i-probe if x-probe not valid */
	if ((state->extsp_ixtoggle == X_PROBE) &&
	    ((state->extsp_flgs & EXTSPFLGS_XPR_VAL_MASK) == 0)) {
		state->extsp_ixtoggle = I_PROBE;
	}

}

/*
 * Normal spatial probing between SISO and MIMO rates.
 * Return: TRUE if decide to switch the family.
 */
static bool
wlc_ratesel_change_sp(scb_ratesel_t *state)
{
	/* fill spatial window before any family switch decision */
	if (state->mcs_sp_nupd < SPATIAL_M || state->mcs_sp_id == state->rateid ||
		state->mcs_sp_id >= state->active_rates_num)
		return FALSE;

	/* switch to other family if prev M A-MPDUs statuses <= K */
	if ((state->rsi->ratesel_sp_algo && (state->mcs_sp_thrt0 > state->mcs_sp_thrt1)) ||
	   (!state->rsi->ratesel_sp_algo && (SPATIAL_M - state->mcs_sp_statc) <= state->mcs_sp_K)) {
		/* switch */
		ratespec_t prv_rspec, cur_rspec;
		prv_rspec = known_rspec[state->select_rates[state->rateid]];
		state->rateid = state->mcs_sp_id;
		cur_rspec = known_rspec[state->select_rates[state->rateid]];

		if (state->rsi->ratesel_sp_algo)
			WL_RATE(("rate_changed(spatially): %s%d to m%d (%d %d), after %2u (%u) txs "
				"with %u (%u) skipped. nthrt_pri/sp 0x%x 0x%x\n",
				IS_MCS(prv_rspec) ? "m" : "", RSPEC_RATE_MASK & prv_rspec,
				RSPEC_RATE_MASK & cur_rspec, state->rateid,
				RSPEC2RATE500K(cur_rspec), state->nupd, state->mcs_sp_nupd,
				state->nskip, state->sp_nskip, state->mcs_sp_thrt1,
				state->mcs_sp_thrt0));
		else
			WL_RATE(("rate_changed(spatially): %s%d to m%d (%d %d), after %2u (%u) txs "
				"with %u (%u) skipped. mcs_sp_statc %d M %d K %d\n",
				IS_MCS(prv_rspec) ? "m" : "", RSPEC_RATE_MASK & prv_rspec,
				RSPEC_RATE_MASK & cur_rspec, state->rateid,
				RSPEC2RATE500K(cur_rspec), state->nupd, state->mcs_sp_nupd,
				state->nskip, state->sp_nskip, state->mcs_sp_statc,
				SPATIAL_M, state->mcs_sp_K));
		return TRUE;
	} else {
		/* check whether we need to switch probing stream */
		int mcs, tblidx, mcs_sp, mcs_sp_id = -1;
		uint8 tmp_col;
		mcs = known_rspec[state->select_rates[state->rateid]] & RSPEC_RATE_MASK;
		tblidx = (mcs == 32) ? 0 : (mcs + 1);
		ASSERT(tblidx >= 0 && tblidx < RATES_NUM_MCS);
		tmp_col = (state->mcs_sp_col == SPATIAL_MCS_COL1) ?
			SPATIAL_MCS_COL2 : SPATIAL_MCS_COL1;
		if ((mcs_sp = mcs_sp_params[tblidx][tmp_col]) != -1) {
			mcs_sp_id = wlc_ratesel_mcs2id(state, (uint8) mcs_sp);
		}
		if (mcs_sp_id > 0) {
			/* switch */
			state->mcs_sp_col = tmp_col;
			state->mcs_sp = (uint8)mcs_sp;
			state->mcs_sp_id = (uint8) mcs_sp_id;
			RL_SP0(("%s: switching probing stream family: %d --> %d\n",
				__FUNCTION__, state->mcs_sp, mcs_sp));
			wlc_ratesel_clear_spstat(state);
		}
	}
	return FALSE;
}

/*
 * Extended spatial probing between SISO and MIMO rates.
 * Return: TRUE if decide to switch the family.
 */
static bool
wlc_ratesel_change_extsp(scb_ratesel_t *state)
{
	/*
	 *  check for early abort: stop this probing run, config probe to next ant choice,
	 *  flush spatial probe, toggle probe, and goto ratehold.
	 */
	if ((state->rsi->ratesel_sp_algo && state->extsp_thrt0 < state->extsp_thrt1) ||
	    (!state->rsi->ratesel_sp_algo &&
	     (state->extsp_nupd - state->extsp_statc) > state->extsp_K)) {

		if (state->extsp_ixtoggle == I_PROBE) {
			if (state->rsi->ratesel_sp_algo)
				RL_SP1(("%s: aborting I-PROBE, probe antselid = %d nupd %d "
					 "thrt_pri/sp = 0x%x 0x%xd\n", __FUNCTION__,
					 state->antselid_extsp_ipr, state->extsp_nupd,
					 state->extsp_thrt1, state->extsp_thrt0));
			else
				RL_SP1(("%s: aborting I-PROBE, probe antselid = %d nupd/statc/K = "
					 "%d %d %d\n", __FUNCTION__, state->antselid_extsp_ipr,
					 state->extsp_nupd, state->extsp_statc, state->extsp_K));
			state->antselid_extsp_ipr = MODINCR(state->antselid_extsp_ipr,
				state->active_antcfg_num);
			/* avoid i-probe to self: */
			if (state->antselid_extsp_ipr == state->antselid)
				state->antselid_extsp_ipr = MODINCR(state->antselid,
					state->active_antcfg_num);
		} else {
			if (state->rsi->ratesel_sp_algo)
				RL_SP1(("%s: aborting X-PROBE, probe mcs/antselid = %d %d nupd %d "
					"thrt_pri/sp = 0x%x 0x%x\n", __FUNCTION__, RSPEC_RATE_MASK &
					known_rspec[state->select_rates[state->extsp_xpr_id]],
					state->antselid_extsp_xpr, state->extsp_nupd,
					state->extsp_thrt1, state->extsp_thrt0));
			else
				RL_SP1(("%s: aborting X-PROBE, probe mcs/antselid = %d %d "
					 "nupd/statc/K = %d %d %d\n", __FUNCTION__, RSPEC_RATE_MASK&
					 known_rspec[state->select_rates[state->extsp_xpr_id]],
					 state->antselid_extsp_xpr, state->extsp_nupd,
					 state->extsp_statc, state->extsp_K));
			state->antselid_extsp_xpr = MODINCR(state->antselid_extsp_xpr,
				state->active_antcfg_num);
		}
		if (state->extsp_ixtoggle == I_PROBE)
			RL_SP1(("%s: start I-Probe antselid %d\n",
				__FUNCTION__, state->antselid_extsp_ipr));
		else
			RL_SP1(("%s: start X-Probe antselid %d sp_mcs %d\n",
				__FUNCTION__, state->antselid_extsp_xpr, RSPEC_RATE_MASK &
				known_rspec[state->select_rates[state->extsp_xpr_id]]));
		wlc_ratesel_toggle_probe(state); /* clears stats and toggle probe if possible */

		return FALSE;
	}

	/*
	 *  if window filled & no abort in previous section,
	 *  do full-blown transition
	 */
	if  (state->extsp_nupd >= EXTSPATIAL_M) { /* window filled */
		uint8 prv_rateid = state->rateid;
		uint8 prv_antselid = state->antselid;

		/* looks promising, carry out transition */
		if (state->extsp_ixtoggle == I_PROBE) {
			state->antselid = state->antselid_extsp_ipr;
		} else {
			state->rateid = state->extsp_xpr_id;
			state->antselid = state->antselid_extsp_xpr;
		}
		state->antselid_extsp_ipr = MODINCR(state->antselid, state->active_antcfg_num);
		state->antselid_extsp_xpr = state->antselid;

		if (prv_rateid || prv_antselid) {
			;  /* to please the compiler */
		}

		if (WL_RATE_ON()) {
			uint32 prv_rspec, cur_rspec;
			prv_rspec = known_rspec[state->select_rates[prv_rateid]];
			cur_rspec = known_rspec[state->select_rates[state->rateid]];
			if (prv_antselid == state->antselid) {

			RL_INFO(("rate_changed(ext_spat): %s%d to %s%d (%d %d) after %2u (%u) "
				 "txs with %u (%u) skipped. nthrt_pri/extsp 0x%x 0x%x\n",
				 IS_MCS(prv_rspec) ? "m" : "", RSPEC_RATE_MASK & prv_rspec,
				 IS_MCS(cur_rspec) ? "m" : "", RSPEC_RATE_MASK & cur_rspec,
				 state->rateid, RSPEC2RATE500K(cur_rspec), state->nupd,
				 state->extsp_nupd, state->nskip, state->sp_nskip,
				 state->extsp_thrt1, state->extsp_thrt0));
			} else {

			RL_INFO(("rate_changed(ext_spat): antid %d to %d after %2u (%u) "
				 "txs with %u (%u) skipped. nthrt_pri/extsp 0x%x 0x%x\n",
				 prv_antselid, state->antselid, state->nupd,
				 state->extsp_nupd, state->nskip, state->sp_nskip,
				 state->extsp_thrt1, state->extsp_thrt0));
			}
		}
		/* toggle probe if possible (includes clearing statistics) */
		wlc_ratesel_toggle_probe(state);

		return TRUE;
	}
	return FALSE;
}

#else /* WL11N state clear-up, update, change rate */

static void
wlc_ratesel_upd_windows(scb_ratesel_t *state, bool up)
{
	uint32 old_valid;
	int byte_id, bit_id, id;
	uint8 window_size;

	window_size = up ? state->Mup : state->Mdn;
	id = state->nofbid - window_size - 1;
	if (id < 0)
		id += MAXRATERECS;

	byte_id   = RATEREC_BYTE(id);
	bit_id    = RATEREC_BITS(id);
	old_valid =  state->nofb[byte_id] & (1 << bit_id);
	if (old_valid) {
		if (up)
			state->nofbtotup--;
		else
			state->nofbtotdn--;
	}

	RL_MORE(("%s: id %d byte_id %d bit_id %d old_valid %u tot %d\n", __FUNCTION__,
		id, byte_id, bit_id, old_valid, (up ? state->nofbtotup : state->nofbtotdn)));
}

/* Function to first determine whether to go down in the rateset select_rates[]; and then do it. */
static bool
wlc_ratesel_godown(scb_ratesel_t *state)
{
	if ((state->rateid > 0) &&
	    ((MIN(state->Mdn, state->nupd) - state->nofbtotdn) >= state->Kdn)) {
		state->rateid --;
		RL_MORE(("%s: rate DOWN. nupd %u nofbtotdn %d Mdn %d Kdn %d\n",
			__FUNCTION__,
			state->nupd, state->nofbtotdn, state->Mdn, state->Kdn));
		return TRUE;
	}
	return FALSE;
}

static bool
wlc_ratesel_goup(scb_ratesel_t *state)
{
	if ((state->rateid < state->active_rates_num-1) && (state->nupd >= state->Mup) &&
	    ((state->Mup - state->nofbtotup) <= state->Kup)) {
			state->rateid ++;
			RL_MORE(("%s: rate UP. nupd %u nofbtotup %d Mup %d Kup %d\n", __FUNCTION__,
				state->nupd, state->nofbtotup, state->Mup, state->Kup));
			return TRUE;
	}
	return FALSE;
}
#endif /* WL11N */

/* 
 * <is_probe>: Indicate this calling is due to a probe packet or not.
 */
static void BCMFASTPATH
wlc_ratesel_pick_rate(scb_ratesel_t *state, bool is_probe)
{
	uint8 prv_rateid;
	ratespec_t cur_rspec, prv_rspec;
#ifdef WL11N
	bool prv_sgi;
#endif
	bool change_epoch = FALSE, change_rate = FALSE;

	/* sanity check */
	ASSERT(state->rateid < state->active_rates_num);

	prv_rateid = state->rateid;
	prv_rspec = CUR_RATESPEC(state);

#ifdef WL11N
	prv_sgi = state->sgi;
	if (is_probe) {
		if (SP_MODE(state) == SP_NORMAL)
			change_rate = wlc_ratesel_change_sp(state);
		else if (SP_MODE(state) == SP_EXT) {
			if ((change_rate = wlc_ratesel_change_extsp(state))) {
				/* check if need to change epoch */
				cur_rspec = CUR_RATESPEC(state);
				if (!IS_MCS(prv_rspec) || !IS_MCS(cur_rspec))
					change_epoch = TRUE;
			}
		}
	} else
#endif /* WL11N */
	{
		/* rate down */
		if (wlc_ratesel_godown(state)) {
			change_rate = TRUE;
		}
		/* rate up */
		else if (state->gotpkts && wlc_ratesel_goup(state)) {
			change_rate = TRUE;
		}

		/* Force to pump up the epoch if rate has changed */
		change_epoch = change_rate;

		if (WL_RATE_ON() && change_rate) {
			/* the rate has changed, vertically */
			uint8 cur_rate;
			cur_rspec = CUR_RATESPEC(state);

			ASSERT(cur_rspec != prv_rspec);
			cur_rate = RSPEC_RATE_MASK & cur_rspec;

			if (prv_rateid != state->rateid) {
				/* the rate has just changed */
				WL_RATE(("rate_changed: %s%d %s %s%d (%d %d) after %2u used "
					"and %u skipped txs\n", IS_MCS(prv_rspec) ? "m" : "",
					RSPEC_RATE_MASK & prv_rspec,
					(prv_rateid > state->rateid) ? "DN" : "UP",
					IS_MCS(cur_rspec) ? "m" : "", cur_rate, state->rateid,
					RSPEC2RATE500K(cur_rspec), state->nupd, state->nskip));
#ifdef WL11N
			} else if (prv_sgi != state->sgi) {
				/* the rate is changed due to sgi on/off */
				WL_RATE(("rate_changed(sgi): m%d (%d %d) sgi %d to %d "
					"after %2u used and %u skipped txs\n", cur_rate,
					state->rateid, RSPEC2RATE500K(cur_rspec), prv_sgi,
					state->sgi, state->nupd, state->nskip));
#endif /* WL11N */
			} else {
				/* the rate has just changed due to a rateset change */
				WL_RATE(("rate_changed(from-out-of-bound): %s%d DN %s%d (%d %d) "
					"after %2u used and %u skipped txs\n",
					IS_MCS(prv_rspec) ? "m" : "", RSPEC_RATE_MASK & prv_rspec,
					IS_MCS(cur_rspec) ? "m" : "", cur_rate, state->rateid,
					RSPEC2RATE500K(cur_rspec), state->nupd, state->nskip));
			}
		}
	}

	if (change_rate) {
#ifdef WL11N
		cur_rspec = known_rspec[state->select_rates[state->rateid]];

		if (IS_MCS(cur_rspec)) {
			uint8 mcs = RSPEC_RATE_MASK & cur_rspec;
			if (mcs != 7 && mcs != 15 && mcs != 23)
				state->sgi = FALSE;
		}
#endif /* WL11N */
		wlc_ratesel_clear_ratestat(state, change_epoch);
	} else {
#ifdef WL11N
		/* clear the external 'clear-on-update' variable if start the spatial probe */
		if (wlc_ratesel_start_probe(state))
			INVALIDATE_TXH_CACHE(state);
#endif
	}
	return;
}

/* all of the following should only apply to directed traffic,
 * do not call me on group-addressed frames.
 * SFBL/LFBL is short/long retry limit for fallback rate
 */
static int
wlc_ratesel_use_txs(scb_ratesel_t *state, tx_status_t *txs, uint16 SFBL, uint16 LFBL,
	uint8 tx_mcs, uint8 antselid, bool fbr)
{
	uint8 nftx, nrtx;
	bool nofb, acked;
	int ret_val = TXS_DISC;

#ifdef WL11N
	int ntx_start_fb;
	uint32 *p, *pfbr;
	uint8 alpha;
	uint8 cur_mcs, ntx_mrt;
	ratespec_t cur_rspec;

	/* Do sanity check before changing gotpkts */
	wlc_ratesel_sanitycheck_psr(state);
#else
	uint32 bit_mask, new_valid, old_valid;
	int byte_id, bit_id;
#endif /* WL11N */

	ASSERT(!TX_STATUS_UNEXP(txs->status));

	/* extract fields and bail if nothing was transmitted
	 * if the frame indicates <fbr> fake nrtx
	 */
	nrtx = fbr ? (SFBL + 1) :
		(uint8) ((txs->status & TX_STATUS_RTS_RTX_MASK) >> TX_STATUS_RTS_RTX_SHIFT);
	nftx = (uint8) ((txs->status & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT);
	if ((nftx == 0) && (nrtx == 0))
		return ret_val;

	/* if this packet was received at the target rate, we
	 * can switch to the steady state rate fallback calculation of target rate -1
	 * AND it needs to clean the external 'clear-on-update' variable
	 */
	nofb = (nrtx > 0) ? (nrtx <= SFBL && nftx <= LFBL) : (nftx <= SFBL);
	acked = (txs->status & TX_STATUS_ACK_RCV) ? TRUE : FALSE;
	if (nofb && acked && state->gotpkts == 0) {
		INVALIDATE_TXH_CACHE(state);
		state->gotpkts = 1;
	}

#ifdef WL11N
	/* Determine after which transmission the fall-back rate is used. */
	ntx_start_fb = (nrtx > SFBL) ? 0 : LFBL;

	cur_rspec = CUR_RATESPEC(state);
	cur_mcs = IS_MCS(cur_rspec) ? RSPEC_RATE_MASK & cur_rspec : MCS_INVALID;
	if (IS_SP_PKT(state, cur_mcs, tx_mcs)) {
		uint8 ntx_succ;
		ntx_mrt = MIN(nftx, ntx_start_fb); /* can be 0 */
		ntx_succ = (acked && nftx <= ntx_start_fb) ? 1 : 0;
		if (wlc_ratesel_upd_spstat(state, FALSE, tx_mcs, ntx_succ, ntx_mrt))
			ret_val = TXS_PROBE;
		RL_SP1(("spatial probe rcvd: m%d (pri: m%d) [nrtx nftx ack] = [%d %d %d] "
			"Upd: %c\n", tx_mcs, cur_mcs, nrtx, nftx, acked,
			(ret_val == TXS_PROBE) ? 'Y' : 'N'));
		return ret_val;
	}
	else if (IS_EXTSP_PKT(state, cur_mcs, tx_mcs, antselid)) {
		uint8 ntx_succ;
		ntx_mrt = MIN(nftx, ntx_start_fb); /* can be 0 */
		ntx_succ = (acked && nftx <= ntx_start_fb) ? 1 : 0;
		if (wlc_ratesel_upd_extspstat(state, FALSE, tx_mcs, antselid, ntx_succ, ntx_mrt))
			ret_val = TXS_PROBE;
		RL_SP1(("txs_ack: ext_spatial probe rcvd: %s%d ant %d (pri: %s%d ant %d) "
			"[nrtx nftx ack] = [%d %d %d] Upd: %c\n",
			(tx_mcs == MCS_INVALID) ? "" : "m", tx_mcs, antselid,
			(cur_mcs == MCS_INVALID) ? "" : "m", cur_mcs, state->antselid,
			nrtx, nftx, acked, (ret_val == TXS_PROBE) ? 'Y' : 'N'));
		return ret_val;
	}


	/* All transmissions before the last are failures. */
	alpha = state->gotpkts ? ((state->nupd < state->rsi->ema_nupd_thres) ?
		state->rsi->psr_ema_alpha0 : state->rsi->psr_ema_alpha) : RATESEL_EMA_ALPHA_INIT;
	pfbr = (state->psri[PSR_CUR_IDX].rspec == state->psri[PSR_FBR_IDX].rspec) ?
		&(state->psri[PSR_CUR_IDX].psr) : &(state->psri[PSR_FBR_IDX].psr);
	p = &(state->psri[PSR_CUR_IDX].psr);
	if (nftx == 0) {
		/* handle the case of RTS-CTS-DATA and RTS-CTS fails: only treat as one failure */
		ASSERT(!acked);
		*p -= *p >> alpha;
	} else {
		int k;
		for (k = 1; k <= nftx; k++) {
			if (k > ntx_start_fb)
				p = pfbr;
			*p -= *p >> alpha;
		}
		if (acked)
			/* Count the successful tx, which must be the last one. */
			*p += 1 << (RATESEL_EMA_NF - alpha);
	}

	/* fbr is not always updated so need to timestamp it every time update it */
	if (pfbr == &(state->psri[PSR_FBR_IDX].psr))
		state->psri[PSR_FBR_IDX].timestamp = NOW(state);

	/* for fast drop */
	state->ncfails = (nofb && acked) ? 0 : state->ncfails + 1;

	RL_MORE(("%s: rate %d %d psr_cur/fbr %d %d [nrtx nftx acked ant fbr] = [%d %d %d %d %d] "
		"nupd %u\n", __FUNCTION__, state->rateid, RSPEC2RATE500K(CUR_RATESPEC(state)),
		state->psri[PSR_CUR_IDX].psr, state->psri[PSR_FBR_IDX].psr, nrtx, nftx,
		acked, antselid, fbr ? 1 : 0, state->nupd));

#else /* WL11N */
	/* Still PER-based for non-WL11N case */
	/* update bitfield, [0, 1] -> [fallback, no-fallback] */
	byte_id   = RATEREC_BYTE(state->nofbid);
	bit_id    = RATEREC_BITS(state->nofbid);
	bit_mask  = 1 << bit_id;

	new_valid = (nofb) ? bit_mask : 0;
	old_valid =  state->nofb[byte_id] & bit_mask;
	if (new_valid != old_valid)
		state->nofb[byte_id] ^= bit_mask;
	/* move pointer to next update position */
	state->nofbid = MODINCR(state->nofbid, MAXRATERECS);

	/* update up/down window totals */
	if (new_valid) {
		state->nofbtotdn++;
		state->nofbtotup++;
	}

	/* update down window totals */
	wlc_ratesel_upd_windows(state, FALSE);

	/* update up window totals */
	wlc_ratesel_upd_windows(state, TRUE);

	RL_MORE(("%s: nupd %u nrtx %u nftx %u nofb %d new_valid %u ACKed %d\n",
		__FUNCTION__, state->nupd, nrtx, nftx, nofb, new_valid, acked));
#endif /* WL11N */

	/* update the count of updates since the last state flush, saturating at max byte val */
	LIMINC_UINT32(state->nupd);
	return TXS_REG;
}

#ifdef WLAFTERBURNER
/* update per-scb rate state upon tx status received in aburn mode */
static void
wlc_ratesel_upd_txs_aburn(scb_ratesel_t *state, tx_status_t *txs, uint8 weight,
	uint8 tx_mcs, uint8 antselid)
{
	uint8 epoch, k;
	int txs_res = TXS_DISC;

	ASSERT(state != NULL);

	/* extract epoch cookie from FrameID */
	epoch = (txs->frameid & TXFID_RATE_MASK) >> TXFID_RATE_SHIFT;

	if (epoch == state->epoch) {
#ifdef WL11N
		/* always update antenna histogram */
		if (WLANTSEL_ENAB(state->rsi->wlc))
			wlc_ratesel_upd_deftxant(state, state->antselid);
#endif
		/*
		 * upon NACK or RTS failure rate update should be called with (weight >= 1),
		 * essentially (weight == 0) map to successful transmission since there is no
		 * MAC layer retransmission for afterburner mode
		 */
		if (weight > 1) {
			for (k = 0; k < weight; k++)
				txs_res = wlc_ratesel_use_txs(state, txs,
					ABURN_SFBL, ABURN_LFBL, tx_mcs, antselid, FALSE);
			if (txs_res != TXS_DISC)
				wlc_ratesel_pick_rate(state, txs_res == TXS_PROBE);
		} else if (IS_CCK_RATE(state->rateid) || !(state->filter++ % 2)) {
			txs_res = wlc_ratesel_use_txs(state, txs, ABURN_SFBL, ABURN_LFBL, tx_mcs,
				antselid, FALSE);
			if (txs_res != TXS_DISC)
				wlc_ratesel_pick_rate(state, txs_res == TXS_PROBE);
		}
	}
	if (txs_res == TXS_DISC)
		LIMINC_UINT32(state->nskip);
}
#endif /* WLAFTERBURNER */

/* update per-scb state upon received tx status */
static void
wlc_ratesel_upd_txs_normal(scb_ratesel_t *state, tx_status_t *txs, uint16 sfbl, uint16 lfbl,
	uint8 tx_mcs, uint8 antselid, bool fbr)
{
	uint8 epoch;
	int txs_res = TXS_DISC;
	ASSERT(state != NULL);


	/* extract epoch cookie from FrameID */
	epoch = (txs->frameid & TXFID_RATE_MASK) >> TXFID_RATE_SHIFT;

	if (epoch == state->epoch) {
#ifdef WL11N
		/* always update antenna histogram */
		wlc_ratesel_upd_deftxant(state, state->antselid);
#endif
		txs_res = wlc_ratesel_use_txs(state, txs, sfbl, lfbl, tx_mcs, antselid, fbr);
		if (txs_res != TXS_DISC)
			wlc_ratesel_pick_rate(state, txs_res == TXS_PROBE);
	}
	else {
		RL_MORE(("%s: frm_e %d (id %x) [nrtx nftx acked ant] = [%u %u %d %d] nupd %u\n",
			__FUNCTION__, epoch, txs->frameid,
			(uint8) ((txs->status & TX_STATUS_RTS_RTX_MASK) >> TX_STATUS_RTS_RTX_SHIFT),
			(uint8) ((txs->status & TX_STATUS_FRM_RTX_MASK) >> TX_STATUS_FRM_RTX_SHIFT),
			(txs->status & TX_STATUS_ACK_RCV) ? 1 : 0, antselid, state->nupd));
	}

	if (txs_res == TXS_DISC)
		LIMINC_UINT32(state->nskip);
}

/* non-AMPDU txstatus rate update, default to use non-mcs rates only */
void
wlc_ratesel_upd_txstatus_normalack(ratesel_info_t *rsi, struct scb *scb, tx_status_t *txs,
	uint16 sfbl, uint16 lfbl,
#ifdef WLAFTERBURNER
	bool aburn, uint8 aburn_weight,
#endif
	uint8 tx_mcs, uint8 antselid, bool fbr)

{
	scb_ratesel_t *state;
	int ac;

	ASSERT(rsi != NULL);

	ac = FID2AC(rsi->pub, txs->frameid);
	ASSERT(ac < AC_COUNT);
	state = SCB_RATESEL_CUBBY(rsi, scb, ac);
	if (state == NULL) {
		ASSERT(0);
		return;
	}

#ifdef WLAFTERBURNER
	/* lookup the new up/down decision thresholds */
	if (state->aburn != aburn) {
		state->aburn = aburn;
		wlc_ratesel_load_params(state);

		WL_RATE(("%s: set to %s\n", __FUNCTION__, aburn ? "afterburner" : "normal"));
	}
#endif /* WLAFTERBURNER */

	/* update history */
#ifdef WLAFTERBURNER
	if (aburn)
		wlc_ratesel_upd_txs_aburn(state, txs, aburn_weight, tx_mcs, antselid);
	else
#endif /* WLAFTERBURNER */
		wlc_ratesel_upd_txs_normal(state, txs, sfbl, lfbl, tx_mcs, antselid, fbr);
}

#ifdef WL11N
/* all of the following should only apply to directed traffic,
 * do not call me on group-addressed frames.
 * Return TRUE to indicate that this is a blockack for probe.
 */
static int BCMFASTPATH
wlc_ratesel_use_txs_blockack(scb_ratesel_t *state, tx_status_t *txs, uint8 suc_mpdu, uint8 tot_mpdu,
	bool ba_lost, uint8 retry, uint8 fb_lim, uint8 tx_mcs, bool tx_error, uint8 antselid)
{
	uint8 cur_mcs;
	uint32 *p, cur_succ;
	uint8 alpha;
	ratespec_t cur_rspec;
	int ret_val;

	if (!tx_error) {
		ASSERT(!TX_STATUS_UNEXP_AMPDU(txs->status));

		/* assert that this is an A-MPDU status */
		ASSERT((txs->status & TX_STATUS_AMPDU) == TX_STATUS_AMPDU);
	} else {
		/* tx fifo underflow error, treat as missed block ack */
		ba_lost = TRUE;
	}

	/* Do sanity check before changing gotpkts */
	wlc_ratesel_sanitycheck_psr(state);

	/* spatial probe functionality
	 * check if this is spatial probe packet block ack:
	 * if true: update spatial statistics and return
	 * else: continue with regular block ack processing
	 */
	cur_rspec = CUR_RATESPEC(state);
	ASSERT(IS_MCS(cur_rspec));
	cur_mcs = RSPEC_RATE_MASK & cur_rspec;
	ret_val = TXS_DISC;
	if (IS_SP_PKT(state, cur_mcs, tx_mcs)) {
		RL_SP1(("%s: spatial probe rcvd: m%d (pri: m%d) [bal suc/tot_mpdu r l] = "
			"[%s %d/%d %d %d]\n", __FUNCTION__, tx_mcs, cur_mcs,
			(ba_lost ? "yes" : "no"), suc_mpdu, tot_mpdu, retry, fb_lim));
		if (retry >= fb_lim) {
			LIMINC_UINT32(state->sp_nskip);
		} else {
			if (wlc_ratesel_upd_spstat(state, TRUE, tx_mcs, suc_mpdu, tot_mpdu))
				ret_val = TXS_PROBE;
		}
		return ret_val;
	}

	/* extended spatial probe functionality 
	 * check if this is extended spatial probe packet block ack:
	 * if true: update spatial statistics and return
	 * else: continue with regular block ack processing
	 */
	if (IS_EXTSP_PKT(state, cur_mcs, tx_mcs, antselid)) {
		RL_SP1(("txs_ba: ext_spatial probe rcvd: m%d ant %d (pri: m%d ant %d) "
			"[bal suc/tot_mpdu r l] = [%s %d/%d %d %d]\n",
			tx_mcs, antselid, cur_mcs, state->antselid,
			(ba_lost ? "yes" : "no"), suc_mpdu, tot_mpdu, retry, fb_lim));
		/* disregard if this was a fallback probe transmission */
		if (retry >= fb_lim) {
			LIMINC_UINT32(state->sp_nskip);
		} else {
			if (wlc_ratesel_upd_extspstat(state, TRUE, tx_mcs, antselid,
				suc_mpdu, tot_mpdu))
				ret_val = TXS_PROBE;
		}
		return ret_val;
	} /* end extended spatial probing blockack eval */

	RL_MORE(("gotpkts = %d; [bal suc/tot_mpdu r l e] = [%s %d %d %d %d %d] nupd %d\n",
		state->gotpkts, (ba_lost ? "yes" : "no"), suc_mpdu, tot_mpdu,
		retry, fb_lim, state->epoch, state->nupd));
	ASSERT(tx_mcs == cur_mcs);

	if (!state->rsi->usefbr && retry >= fb_lim)
		return TXS_DISC;

	/* for fast drop */
	if ((suc_mpdu << 1) < tot_mpdu)
		state->ncfails ++;
	else if (retry < fb_lim) {
		/* if the tx uses the fallback of fallback, and at least half of AMPDU are received,
		 * don't use it; otherwise, it may slow the dropping of the rate.
		 */
		state->ncfails = 0;
	}

	/* for retry = fb_lim + 1, +2 etc, uses fallback rate of fallback. Discard it if failed. */
	if (retry > fb_lim)
		return ((suc_mpdu << 1) < tot_mpdu) ? TXS_REG : TXS_DISC;

	/* this packet was received at the target rate, so we
	 * can switch to the steady state rate fallback calculation
	 * of target rate - 1
	 */
	if ((!ba_lost) && (tot_mpdu > 0) && (suc_mpdu == tot_mpdu) && (retry < fb_lim)) {
		if (state->gotpkts == 0) {
			INVALIDATE_TXH_CACHE(state);
			state->gotpkts = 1;
		}
	}

	/* stop sending short A-MPDUs (if we received L frames) */
	if (state->mcs_short_ampdu && state->mcs_nupd >= state->rsi->max_short_ampdus) {
		state->mcs_short_ampdu = FALSE;
		INVALIDATE_TXH_CACHE(state);
	}


	/* Update per A-MPDU */
	p = ((retry < fb_lim) || (state->psri[PSR_FBR_IDX].rspec == cur_rspec)) ?
		&(state->psri[PSR_CUR_IDX].psr)
		: &(state->psri[PSR_FBR_IDX].psr);

	cur_succ = (suc_mpdu << RATESEL_EMA_NF) / tot_mpdu;
	alpha = state->gotpkts ? ((state->nupd <= state->rsi->ema_nupd_thres) ?
		state->rsi->psr_ema_alpha0 : state->rsi->psr_ema_alpha) : RATESEL_EMA_ALPHA_INIT;
	UPDATE_MOVING_AVG(p, cur_succ, alpha);

	/* fbr is not always updated so need to timestamp it every time update it */
	if (p == &(state->psri[PSR_FBR_IDX].psr))
		state->psri[PSR_FBR_IDX].timestamp = NOW(state);

	/* update the count of updates since the last state flush, which
	 * saturates at max word val
	 */
	LIMINC_UINT32(state->mcs_nupd); /* for purpose of mcs_short_ampdu */
	LIMINC_UINT32(state->nupd);

	ASSERT(state->psri[PSR_CUR_IDX].psr <= PSR_MAX);
	ASSERT(state->psri[PSR_FBR_IDX].psr <= PSR_MAX);

	RL_MORE(("%s: rate %d %d psr_cur/fbr %d %d\n",
		__FUNCTION__, state->rateid, RSPEC2RATE500K(state->psri[PSR_CUR_IDX].rspec),
		state->psri[PSR_CUR_IDX].psr, state->psri[PSR_FBR_IDX].psr));

	return TXS_REG;
}

/* update state upon received BA */
void BCMFASTPATH
wlc_ratesel_upd_txs_blockack(ratesel_info_t *rsi, struct scb *scb, tx_status_t *txs,
	uint8 suc_mpdu, uint8 tot_mpdu, bool ba_lost, uint8 retry, uint8 fb_lim, bool tx_error,
	uint8 mcs, uint8 antselid)
{
	scb_ratesel_t *state;
	uint8 epoch;
	int ac, txs_res;

	ASSERT(rsi != NULL);

	ac = FID2AC(rsi->pub, txs->frameid);
	ASSERT(ac < AC_COUNT);
	state = SCB_RATESEL_CUBBY(rsi, scb, ac);

	/* sanity check: mcs set may not be empty */
	if ((state == NULL) || (state->active_rates_num == 0)) {
		ASSERT(0);
		return;
	}

	/* update history. extract epoch cookie from FrameID */
	epoch = (txs->frameid & TXFID_RATE_MASK) >> TXFID_RATE_SHIFT;
	txs_res = TXS_DISC;
	if (epoch == state->epoch) {
#ifdef BCMDBG
		/* to accommodate the discrepancy due to "wl nrate" in the debug driver */
		if (!IS_MCS(CUR_RATESPEC(state))) {
			WL_RATE(("%s: blockack received for non-MCS rate.\n", __FUNCTION__));
			return;
		}
#endif

		/* always update antenna histogram */
		if (WLANTSEL_ENAB(rsi->wlc))
			wlc_ratesel_upd_deftxant(state, state->antselid);

		txs_res = wlc_ratesel_use_txs_blockack(state, txs, suc_mpdu, tot_mpdu,
			ba_lost, retry, fb_lim, mcs, tx_error, antselid);
		if (txs_res != TXS_DISC)
			wlc_ratesel_pick_rate(state, txs_res == TXS_PROBE);
	}
	else {
		RL_MORE(("%s: frm_e %d (id %x) [bal suc tot_mpdu r l e] = [%s %d %d %d %d %d] "
			"mcs %d ant %d\n", __FUNCTION__, epoch, txs->frameid,
			ba_lost ? "yes" : "no", suc_mpdu, tot_mpdu, retry, fb_lim,
			state->epoch, mcs, antselid));
	}

	if (txs_res == TXS_DISC) {
		LIMINC_UINT32(state->nskip);
	}
}

#ifdef WLAMPDU_MAC
/*
 * Following two functions are for the ampdu ucode/hw aggregation case
 */
static int
wlc_ratesel_use_txs_ampdu(scb_ratesel_t *state, uint16 frameid, uint8 mrt, uint8 mrt_succ,
	uint8 fbr, uint8 fbr_succ, bool tx_error, uint8 tx_mcs, uint8 antselid)
{
	uint8 cur_mcs;
	uint32 cur_succ;
	uint8 alpha;
	ratespec_t cur_rspec;
	int ret_val;

	/* tx fifo underflow error, treat as failure at mrt */
	if (tx_error)
	    mrt_succ = 0;

	/* Do sanity check before changing gotpkts */
	wlc_ratesel_sanitycheck_psr(state);

	/* spatial probe functionality
	 * check if this is spatial probe packet block ack:
	 * if true: update spatial statistics and return
	 * else: continue with regular block ack processing
	 */
	cur_rspec = CUR_RATESPEC(state);
	ASSERT(IS_MCS(cur_rspec));
	cur_mcs = RSPEC_RATE_MASK & cur_rspec;
	ret_val = TXS_DISC;
	if (IS_SP_PKT(state, cur_mcs, tx_mcs)) {
		RL_SP1(("%s: spatial probe rcvd: m%d (pri: m%d) [mrt succ fbr succ] = "
			"[%d %d %d %d]\n", __FUNCTION__, tx_mcs, cur_mcs,
			mrt, mrt_succ, fbr, fbr_succ));
		if (mrt == 0) {
			LIMINC_UINT32(state->sp_nskip);
		} else {
			if (wlc_ratesel_upd_spstat(state, TRUE, tx_mcs, mrt_succ, mrt))
				ret_val = TXS_PROBE;
		}
		return ret_val;
	}

	/* extended spatial probe functionality 
	 * check if this is extended spatial probe packet block ack:
	 * if true: update spatial statistics and return
	 * else: continue with regular block ack processing
	 */
	if (IS_EXTSP_PKT(state, cur_mcs, tx_mcs, antselid)) {
		RL_SP1(("txs_ba: ext_spatial probe rcvd: m%d ant %d (pri: m%d ant %d) "
			"[mrt succ fbr fbr_succ] = [%d %d %d %d]\n",
			tx_mcs, antselid, cur_mcs, state->antselid,
			mrt, mrt_succ, fbr, fbr_succ));
		/* disregard if this was no tx at mrt */
		if (mrt == 0) {
			LIMINC_UINT32(state->sp_nskip);
		} else {
			if (wlc_ratesel_upd_extspstat(state, TRUE, tx_mcs, antselid, mrt_succ, mrt))
				ret_val = TXS_PROBE;
		}
		return ret_val;
	} /* end extended spatial probing blockack eval */

	RL_MORE(("gotpkts = %d fid 0x%04x [mrt succ fbr succ e] = [%d %d %d %d %d] nupd %d\n",
		state->gotpkts, frameid, mrt, mrt_succ, fbr, fbr_succ, state->epoch, state->nupd));
	ASSERT(tx_mcs == cur_mcs);

	if (!state->rsi->usefbr && mrt == 0)
		return TXS_DISC;

	/* for fast drop */
	if ((mrt_succ << 1) < mrt || (fbr_succ << 1) < fbr) {
		state->ncfails ++;
	}
	if (mrt != 0 && (mrt_succ << 1) >= mrt)
		state->ncfails = 0;

	/* this packet was received at the target rate, so we
	 * can switch to the steady state rate fallback calculation
	 * of target rate - 1
	 */
	if (mrt > 0 && (mrt_succ == mrt)) {
		if (state->gotpkts == 0) {
			INVALIDATE_TXH_CACHE(state);
			state->gotpkts = 1;
		}
	}

	/* stop sending short A-MPDUs (if we received L frames) */
	if (state->mcs_short_ampdu && state->mcs_nupd >= state->rsi->max_short_ampdus) {
		state->mcs_short_ampdu = FALSE;
		INVALIDATE_TXH_CACHE(state);
	}

	/* Update per A-MPDU */
	alpha = state->gotpkts ? ((state->nupd <= state->rsi->ema_nupd_thres) ?
		state->rsi->psr_ema_alpha0 : state->rsi->psr_ema_alpha) : RATESEL_EMA_ALPHA_INIT;
	if (mrt > 0) {
		cur_succ = (mrt_succ << RATESEL_EMA_NF) / mrt;
		UPDATE_MOVING_AVG(&(state->psri[PSR_CUR_IDX].psr), cur_succ, alpha);
	}
	if (fbr > 0) {
		cur_succ = (fbr_succ << RATESEL_EMA_NF) / fbr;
		UPDATE_MOVING_AVG(&(state->psri[PSR_FBR_IDX].psr), cur_succ, alpha);
		/* fbr is not always updated so need to timestamp it every time update it */
		state->psri[PSR_FBR_IDX].timestamp = NOW(state);
	}

	/* update the count of updates at primary rate since the last state flush
	 * which saturates at max word val
	 */
	if (mrt > 0) {
		LIMINC_UINT32(state->mcs_nupd); /* for purpose of mcs_short_ampdu */
		LIMINC_UINT32(state->nupd);
	}

	ASSERT(state->psri[PSR_CUR_IDX].psr <= PSR_MAX);
	ASSERT(state->psri[PSR_FBR_IDX].psr <= PSR_MAX);

	RL_MORE(("%s: rate %d %d psr_cur/fbr %d %d\n",
		__FUNCTION__, state->rateid, RSPEC2RATE500K(state->psri[PSR_CUR_IDX].rspec),
		state->psri[PSR_CUR_IDX].psr, state->psri[PSR_FBR_IDX].psr));

	return TXS_REG;
}

/*
 * The case that (mrt+fbr) == 0 is handled as RTS transmission failure.
 */
void
wlc_ratesel_upd_txs_ampdu(ratesel_info_t *rsi, struct scb *scb, uint16 frameid,
	uint8 mrt, uint8 mrt_succ, uint8 fbr, uint8 fbr_succ,
	bool tx_error, uint8 tx_mcs, uint8 antselid)
{
	scb_ratesel_t *state;
	uint8 epoch;
	int ac, txs_res;

	ASSERT(rsi != NULL);

	ac = FID2AC(rsi->pub, frameid);
	ASSERT(ac < AC_COUNT);
	state = SCB_RATESEL_CUBBY(rsi, scb, ac);

	/* sanity check: mcs set may not be empty */
	if ((state == NULL) || (state->active_rates_num == 0)) {
		ASSERT(0);
		return;
	}

	if (mrt + fbr == 0) {
		/* fake one transmission at main rate to obtain the expected update */
		mrt = 1;
		ASSERT(mrt_succ + fbr_succ == 0);
	}


	/* update history. extract epoch cookie from FrameID */
	epoch = (frameid & TXFID_RATE_MASK) >> TXFID_RATE_SHIFT;
	txs_res = TXS_DISC;
	if (epoch == state->epoch) {
#ifdef BCMDBG
		/* to accommodate the discrepancy due to "wl nrate" in the debug driver */
		if (!IS_MCS(CUR_RATESPEC(state))) {
			WL_RATE(("%s: blockack received for non-MCS rate.\n", __FUNCTION__));
			return;
		}
#endif

		/* always update antenna histogram */
		if (WLANTSEL_ENAB(rsi->wlc))
			wlc_ratesel_upd_deftxant(state, state->antselid);

		txs_res = wlc_ratesel_use_txs_ampdu(state, frameid, mrt, mrt_succ, fbr, fbr_succ,
			tx_error, tx_mcs, antselid);
		if (txs_res != TXS_DISC)
			wlc_ratesel_pick_rate(state, txs_res == TXS_PROBE);
	}
	else {
		RL_MORE(("%s: frm_e %d (fid %x) [mrt succ fbr succ e] = [%d %d %d %d %d] "
			"mcs %d ant %d\n", __FUNCTION__, epoch, frameid,
			mrt, mrt_succ, fbr, fbr_succ, state->epoch, tx_mcs, antselid));
	}

	if (txs_res == TXS_DISC) {
		LIMINC_UINT32(state->nskip);
	}
}
#endif /* WLAMPDU_MAC */

static void
wlc_ratesel_upd_deftxant(scb_ratesel_t *state, uint8 txant_new_idx)
{
	uint8 idx;
	int id;
	uint8 txant_old_idx;
	uint8 new_max_idx;
	uint8 new_cnt;
	ratesel_info_t *rsi = state->rsi;

	if (rsi->txant_stats_num <= 1)
		return;

	state->txs_cnt += 1;
	if (state->txs_cnt <= TXANTHIST_NMOD)
		return;

	state->txs_cnt = 0;

	/* update txant history */
	idx = rsi->txant_hist_id;
	rsi->txant_hist[idx] = (txant_new_idx | 0x80);
	rsi->txant_hist_id = MODINC(rsi->txant_hist_id, MAXTXANTRECS);

	/* update txant history totals */
	rsi->txant_stats[(txant_new_idx & 0x07)] += 1;
	id = rsi->txant_hist_id - TXANTHIST_M - 1;
	if (id < 0)
		id += MAXTXANTRECS;
	txant_old_idx = rsi->txant_hist[id];
	if ((txant_old_idx & 0x80) != 0) {
		txant_old_idx &= ~0x80;
		rsi->txant_stats[(txant_old_idx & 0x07)] -= 1;
	}

	/* find best configuration (maximum) */
	new_cnt = rsi->txant_stats[(txant_new_idx & 0x07)];
	new_max_idx = rsi->txant_max_idx;
	if (new_cnt >= TXANTHIST_K) new_max_idx = (txant_new_idx & 0x07);

	/* check if default config has to be changed */
	if (rsi->txant_max_idx != new_max_idx) {
		/* update default antenna config and call phy update function */
		rsi->txant_max_idx = new_max_idx;
		if (rsi->rxant_sel)
			wlc_ratesel_upd_rxantprb(rsi, RXANT_UPD_REASON_TXCHG);
		else {
			wlc_antsel_upd_dflt(rsi->wlc->asi, new_max_idx);
			WL_RATE(("%s: new default txantid %d\n", __FUNCTION__, rsi->txant_max_idx));
		}

	} else if (rsi->rxant_sel) {
		rsi->rxant_txcnt ++;
		wlc_ratesel_upd_rxantprb(rsi, RXANT_UPD_REASON_TXUPD);
	}
}

/*
 * Run the rxant probe state machine:
 * basically to determine whether to start/stop the probing,
 * and set the default rxant accordingly.
 * <reason_code> represents three reasons to call this function:
 * = 0, the dominating txant changes
 * = 1, tx cnt increments (actually every X tx)
 * = 2, rx cnt increments
 */
void
wlc_ratesel_upd_rxantprb(ratesel_info_t *rsi, int reason_code)
{
	bool not_probing = TRUE;

	ASSERT(rsi->rxant_sel);

	not_probing = rsi->rxant_id == rsi->rxant_probe_id;

	if (not_probing) {
		/* Do we need to start probing? */
		if (rsi->rxant_id == rsi->txant_max_idx)
			return;

		switch (reason_code) {
		case RXANT_UPD_REASON_TXCHG:
			/* reset threshold and start immediately */
			rsi->rxant_pon_tt = RXANT_PON_TXCNT_MIN;
			rsi->rxant_pon_tr = RXANT_PON_RXCNT_MIN;
			wlc_ratesel_rxant_pon(rsi);
			RL_RXA1(("%s: reset pon threshold: tx/rx %d %d\n",
				__FUNCTION__, rsi->rxant_pon_tt, rsi->rxant_pon_tr));
			break;
		case RXANT_UPD_REASON_TXUPD:
			if (rsi->rxant_txcnt >= rsi->rxant_pon_tt)
				wlc_ratesel_rxant_pon(rsi);
			break;
		case RXANT_UPD_REASON_RXUPD:
			if (rsi->rxant_rxcnt >= rsi->rxant_pon_tr)
				wlc_ratesel_rxant_pon(rsi);
			break;
		default:
			ASSERT(0);
		}
	} else {
		/* Do we need to stop probing? */
		switch (reason_code) {
		case RXANT_UPD_REASON_TXCHG:
			/* Hey, txant switches too fast. Let's skip this one */
			RL_RXA1(("%s: dominating txant switches while rx in probing.\n",
				__FUNCTION__));
			break;
		case RXANT_UPD_REASON_TXUPD:
			if (rsi->rxant_txcnt >= rsi->rxant_poff_tt)
				wlc_ratesel_rxant_poff(rsi, TRUE);
			break;
		case RXANT_UPD_REASON_RXUPD:
			if (rsi->rxant_rxcnt >= rsi->rxant_poff_tr)
				wlc_ratesel_rxant_poff(rsi, TRUE);
			else if (rsi->rxant_rxcnt >= RXANT_POFF_RXCNT_M)
				wlc_ratesel_rxant_poff(rsi, FALSE);
			break;
		default:
			ASSERT(0);
		}
	}
}

static void
wlc_ratesel_rxant_pon(ratesel_info_t *rsi)
{
	rsi->rxant_probe_id = rsi->txant_max_idx;

	RL_RXA0(("%s : cnt_tx/rx %d %d start to probe rxant id %d\n", __FUNCTION__,
		rsi->rxant_txcnt, rsi->rxant_rxcnt, rsi->rxant_probe_id));

	rsi->rxant_txcnt = 0;
	rsi->rxant_rxcnt = 0;
	rsi->rxant_probe_stats = 0;
	wlc_antsel_upd_dflt(rsi->wlc->asi, rsi->rxant_probe_id);
}

/*
 * <force_off> is set to FALSE when doing rxcnt periodic check
 *             is set to TRUE when either txcnt or rxcnt reach threshold.
 * <force_off> == FALSE
 *             abort probing if probe_stats < stats (primary) by 10%
 *             == TRUE
 *             abort probing regardlessly
 *             if probe_stats >= stats (primary), switch
 */
static void
wlc_ratesel_rxant_poff(ratesel_info_t *rsi, bool force_off)
{
	bool probe_done, probe_succ = FALSE;
	probe_done = force_off;

	if (force_off)
		/* final decision time */
		probe_succ = (rsi->rxant_probe_stats >= rsi->rxant_stats);
	else
		/* early abort probing */
		probe_done = ((rsi->rxant_probe_stats << 5) < rsi->rxant_stats * 29);

	if (!probe_done) {
		RL_RXA0(("%s: cnt_tx/rx %d %d stats_prb/pri %d %d. Probe %d continues.\n",
			__FUNCTION__, rsi->rxant_txcnt, rsi->rxant_rxcnt, rsi->rxant_probe_stats,
			rsi->rxant_stats, rsi->rxant_probe_id));
		return;
	}

	if (probe_succ) {
		/* use the new one */
		RL_RXA0(("%s: cnt_tx/rx %d %d stats_prb/pri %d %d. Probe %d succ. Leave %d\n",
			__FUNCTION__, rsi->rxant_txcnt, rsi->rxant_rxcnt, rsi->rxant_probe_stats,
			rsi->rxant_stats, rsi->rxant_probe_id, rsi->rxant_id));
		rsi->rxant_id = rsi->rxant_probe_id;
		rsi->rxant_stats = 0;

	} else {
		/* go back to our old one: but don't clear the old stats */
		RL_RXA0(("%s: cnt_tx/rx %d %d stats_prb/pri %d < %d. Probe %d fails. Stay@ %d\n",
			__FUNCTION__, rsi->rxant_txcnt, rsi->rxant_rxcnt, rsi->rxant_probe_stats,
			rsi->rxant_stats, rsi->rxant_probe_id, rsi->rxant_id));
		rsi->rxant_probe_id = rsi->rxant_id;
		wlc_antsel_upd_dflt(rsi->wlc->asi, rsi->rxant_id);
		rsi->rxant_pon_tt = MIN(rsi->rxant_pon_tt*2, RXANT_PON_TXCNT_MAX);
		rsi->rxant_pon_tr = MIN(rsi->rxant_pon_tr*2, RXANT_PON_RXCNT_MAX);
	}

	rsi->rxant_rxcnt = 0;
	rsi->rxant_txcnt = 0;
}

/* collect statistics for rx antenna selection */
void
wlc_ratesel_upd_rxstats(ratesel_info_t *rsi, ratespec_t rx_rspec, uint16 rxstatus2)
{
	uint32 rxrate, *pstats;
	bool probe;
	uint8 rxant_id;

	if (!rsi->rxant_sel)
		return;

	rxant_id = (rxstatus2 >> RXS_RXANT_SHIFT) & RXS_RXANT_MASK;
	probe = (rsi->rxant_id != rsi->rxant_probe_id);

	/* filter out frames received with unmatched ant due to queuing delay */
	if ((probe && rxant_id != rsi->rxant_probe_id) || (!probe && rxant_id != rsi->rxant_id))
		return;

	pstats = probe ? &rsi->rxant_probe_stats : &rsi->rxant_stats;
	rxrate = CRSPEC2RATE500K(rx_rspec);

	/* collect history info */
	if (rsi->rxant_rate_en) {
		ASSERT(rxant_id < ANT_SELCFG_MAX_NUM);
		if (rsi->rxant_rate_cnt[rxant_id] == 0)
			rsi->rxant_rate[rxant_id] = rxrate;
		else
			UPDATE_MOVING_AVG(&rsi->rxant_rate[rxant_id], rxrate, rsi->psr_ema_alpha);
	}

	if (rsi->rxant_rxcnt == 0)
		*pstats = rxrate;
	else {
		uint32 old = *pstats;
		UPDATE_MOVING_AVG(pstats, rxrate, rsi->psr_ema_alpha);
		if (*pstats == old && rxrate > old)
			(*pstats) ++;
	}
	LIMINC_UINT32(rsi->rxant_rxcnt);

	if (probe)
		RL_RXA1(("Upd_rxstats: prb rxant %d rxcnt %d rspec 0x%x rate %d stats %d "
			"rxant %d\n", rsi->rxant_probe_id, rsi->rxant_rxcnt,
			rx_rspec, rxrate, rsi->rxant_probe_stats, rxant_id));
	else
		RL_RXA1(("Upd_rxstats: cur rxant %d rxcnt %d rspec 0x%x rate %d stats %d "
			"rxant %d\n", rsi->rxant_id, rsi->rxant_rxcnt,
			rx_rspec, rxrate, rsi->rxant_stats, rxant_id));

	wlc_ratesel_upd_rxantprb(rsi, RXANT_UPD_REASON_RXUPD);
}
#endif /* WL11N */

/* select transmit rate given per-scb state */
void BCMFASTPATH
wlc_ratesel_gettxrate(ratesel_info_t *rsi, struct scb *scb, uint16 *frameid,
	ratespec_t *rate, ratespec_t *fbrate, uint8 *antselid,
	uint8 *fbantselid, uint16 *flags)
{
	scb_ratesel_t *state;
	uint8 rateid;
	int ac;
#ifdef WL11N
	uint32 rspec_sgi;
#endif
	ASSERT(rsi != NULL);

	ac = FID2AC(rsi->pub, *frameid);
	ASSERT(ac < AC_COUNT);
	state = SCB_RATESEL_CUBBY(rsi, scb, ac);
	if (state == NULL) {
		ASSERT(0);
		return;
	}

	/* clear all flags */
	*flags = 0;

#ifdef WL11N
	/* get initial values for antenna selection, prim might be overridden for probe */
	*antselid = state->antselid;
	*fbantselid = state->antselid;

	rspec_sgi = state->sgi ? RSPEC_SHORT_GI : 0;

#else /* WL11N */
	/* zero values if no antenna diversity */
	*antselid = 0;
	*fbantselid = 0;
#endif /* WL11N */

	rateid = state->rateid;

	/* drop a cookie containing the current rate epoch in the
	 * appropriate portion of the FrameID
	 */
	*frameid = *frameid & (~TXFID_RATE_MASK);
	*frameid = *frameid | ((state->epoch << TXFID_RATE_SHIFT) & TXFID_RATE_MASK);

	/* clear probe flag */
	*frameid = *frameid & (~TXFID_RATE_PROBE_MASK);

	/* sanity check */
	if (rateid >= state->active_rates_num) {
		rateid = state->active_rates_num - 1;
		state->rateid = rateid;
	}

	*rate = known_rspec[state->select_rates[rateid]];
	*fbrate = wlc_ratesel_getfbrspec(state, rateid);

#ifdef WL11N

	/* fill in rates & antennas for the case of probes */
	if ((SP_MODE(state) == SP_NORMAL) && ((state->mcs_sp_flgs & SPFLGS_EN_MASK) == 1)) {

		rateid = state->mcs_sp_id; /* select spatial probe rate */
		*frameid = *frameid | TXFID_RATE_PROBE_MASK; /* flag probes */
		rspec_sgi = 0; /* don't probe with SGI in general */

	} else if ((SP_MODE(state) == SP_EXT) &&
		((state->extsp_flgs & EXTSPFLGS_EN_MASK) == 1)) {
		if (state->extsp_ixtoggle == I_PROBE) {
			*antselid = state->antselid_extsp_ipr;
		} else {
			rateid = state->extsp_xpr_id;
			*antselid = state->antselid_extsp_xpr;
			rspec_sgi = 0; /* don't x-probe with SGI */
		}

		/* flag probes */
		*frameid = *frameid | TXFID_RATE_PROBE_MASK;
	}
	if (rateid != state->rateid && rateid >= state->active_rates_num) {
		rateid = state->active_rates_num - 1;
		state->rateid = rateid;
	}

	/* mark frames that should go out as short A-MPDUs */
	if (state->mcs_short_ampdu) {
		*flags |= RATESEL_VRATE_PROBE;
	}

	/* set primary and secondary rate */
	*rate = known_rspec[state->select_rates[rateid]];
	if (SP_MODE(state) == SP_EXT)
		/* fallback based on primary rate of non-probe frames */
		*fbrate = wlc_ratesel_getfbrspec(state, state->rateid);
	else
		*fbrate = wlc_ratesel_getfbrspec(state, rateid);

	*rate |= rspec_sgi;

	if (SP_MODE(state) == SP_EXT)
		RL_MORE(("%s: rspec_pri/fbr 0x%08x 0x%08x rate_pri/fbr %u %u ant_pri/fbr %d %d "
			"sgi %d\n", __FUNCTION__, *rate, *fbrate, RSPEC2RATE500K(*rate),
			RSPEC2RATE500K(*fbrate), *antselid, *fbantselid, state->sgi));
	else
		RL_MORE(("%s: rspec_pri/fbr 0x%08x 0x%08x rate_pri/fbr %u %u sgi %d\n",
			__FUNCTION__, *rate, *fbrate, RSPEC2RATE500K(*rate),
			RSPEC2RATE500K(*fbrate), state->sgi));
#else /* WL11N */
	RL_MORE(("%s: rspec_pri/fbr 0x%08x 0x%08x rate_pri/fbr %u %u\n",
		__FUNCTION__, *rate, *fbrate, RSPEC2RATE500K(*rate), RSPEC2RATE500K(*fbrate)));
#endif /* WL11N */
	return;
}
