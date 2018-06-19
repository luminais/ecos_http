/*
 * Common interface to the 802.11 Station Control Block (scb) structure
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_scb.h,v 1.131.10.15 2011-01-07 22:52:13 Exp $
 */

#ifndef _wlc_scb_h_
#define _wlc_scb_h_

#include <proto/802.1d.h>

#define SCB_BSSCFG_BITSIZE ROUNDUP(32, NBBY)/NBBY
#if (WLC_MAXBSSCFG > 32)
#error "auth_bsscfg cannot handle WLC_MAXBSSCFG"
#endif

/* Information node for scb packet transmit path */
struct tx_path_node {
	txmod_tx_fn_t next_tx_fn;		/* Next function to be executed */
	void *next_handle;
	bool configured;		/* Whether this feature is configured */
};

#ifdef WLCNTSCB
typedef struct wlc_scb_stats {
	uint32 tx_pkts;			/* # of packets transmitted */
	uint32 tx_failures;		/* # of packets failed */
	uint32 rx_ucast_pkts;		/* # of unicast packets received */
	uint32 rx_mcast_pkts;		/* # of multicast packets received */
	ratespec_t tx_rate;		/* Rate of last successful tx frame */
	ratespec_t rx_rate;		/* Rate of last successful rx frame */
	uint32 rx_decrypt_succeeds;	/* # of packet decrypted successfully */
	uint32 rx_decrypt_failures;	/* # of packet decrypted unsuccessfully */
} wlc_scb_stats_t;
#endif /* WLCNTSCB */

#ifdef WLPKTDLYSTAT
#define WLPKTDLY_HIST_NBINS	16	/* number of bins used in the Delay histogram */

/* structure to store per-AC delay statistics */
struct wlc_scb_delay_stats {
	uint32 txmpdu_lost;	/* number of MPDUs lost */
	uint32 txmpdu_cnt[RETRY_SHORT_DEF]; /* retry times histogram */
	uint32 delay_sum[RETRY_SHORT_DEF]; /* cumulative packet latency */	
	uint32 delay_min;	/* minimum packet latency observed */
	uint32 delay_max;	/* maximum packet latency observed */
	uint32 delay_hist[WLPKTDLY_HIST_NBINS];	/* delay histogram */
};
#endif

/* station control block - one per remote MAC address */
struct scb {
	void *scb_priv;		/* internal scb data structure */
#ifdef MACOSX
	uint32 magic;
#endif
	uint32	flags;		/* various bit flags as defined below */
	uint32	flags2;		/* various bit flags2 as defined below */
	wsec_key_t	*key;		/* per station WEP key */
	wlc_bsscfg_t	*bsscfg;	/* bsscfg to which this scb belongs */
	uint8   auth_bsscfg[SCB_BSSCFG_BITSIZE]; /* authentication state w/ respect to bsscfg(s) */
	uint8	state; /* current state bitfield of auth/assoc process */
	bool		permanent;	/* scb should not be reclaimed */
	struct ether_addr ea;		/* station address */
	uint		used;		/* time of last use */
	uint32		assoctime;	/* time of association */
	uint		bandunit;	/* tha band it belongs to */
#if defined(IBSS_PEER_GROUP_KEY)
	wsec_key_t	*ibss_grp_keys[WSEC_MAX_DEFAULT_KEYS];	/* Group Keys for IBSS peer */
#endif /* defined(IBSS_PSK) */


	uint16	 WPA_auth;	/* WPA: authenticated key management */
	uint32	 wsec;	/* ucast security algo. should match key->algo. Needed before key is set */

	wlc_rateset_t	rateset;	/* operational rates for this remote station */

	void	*fragbuf[NUMPRIO];	/* defragmentation buffer per prio */
	uint	fragresid[NUMPRIO];	/* #bytes unused in frag buffer per prio */

	uint16	 seqctl[NUMPRIO];	/* seqctl of last received frame (for dups) */
	uint16	 seqctl_nonqos;		/* seqctl of last received frame (for dups) for
					 * non-QoS data and management
					 */
	uint16	 seqnum[NUMPRIO];	/* WME: driver maintained sw seqnum per priority */

	/* APSD configuration */
	struct {
		uint16		maxsplen;   /* Maximum Service Period Length from assoc req */
		ac_bitmap_t	ac_defl;    /* Bitmap of ACs enabled for APSD from assoc req */
		ac_bitmap_t	ac_trig;    /* Bitmap of ACs currently trigger-enabled */
		ac_bitmap_t	ac_delv;    /* Bitmap of ACs currently delivery-enabled */
	} apsd;

#ifdef AP
	uint16		aid;		/* association ID */
	uint8		*challenge;	/* pointer to shared key challenge info element */
	uint16		tbtt;		/* count of tbtt intervals since last ageing event */
	uint8		auth_alg;	/* 802.11 authentication mode */
	bool		PS;		/* remote STA in PS mode */
	bool            PS_pend;        /* Pending PS state */
	uint		grace_attempts;	/* Additional attempts made beyond scb_timeout
					 * before scb is removed
					 */
#endif /* AP */
	uint8		*wpaie;		/* WPA IE */
	uint		wpaie_len;	/* Length of wpaie */
	wlc_if_t	*wds;		/* per-port WDS cookie */
	int		*rssi_window;	/* rssi samples */
	int		rssi_index;
	int		rssi_enabled;	/* enable rssi collection */
	uint16		cap;		/* sta's advertized capability field */
	uint16		listen;		/* minimum # bcn's to buffer PS traffic */

	uint16		amsdu_mtu_pref;	/* preferred AMSDU mtu in bytes */
#if defined(WL_AP_TPC)
	int8		sta_link_margin;	/* STAs present link margin */
	int8		ap_link_margin;		/* APs present link margin */
#endif
#ifdef WL11N
	bool		ht_mimops_enabled;	/* cached state: a mimo ps mode is enabled */
	bool		ht_mimops_rtsmode;	/* cached state: TRUE=RTS mimo, FALSE=no mimo */
	uint16		ht_capabilities;	/* current advertised capability set */
	uint8		ht_ampdu_params;	/* current adverised AMPDU config */
	uint8		rclen;			/* regulatory class length */
	uint8		rclist[MAXRCLISTSIZE];	/* regulatory class list */
#endif /* WL11N */

	struct tx_path_node	*tx_path; /* Function chain for tx path for a pkt */
	uint32		fragtimestamp[NUMPRIO];
#ifdef WLCNTSCB
	wlc_scb_stats_t scb_stats;
#endif /* WLCNTSCB */
#ifdef BCMWAPI_WPI
	wsec_key_t	*prev_key;	/* to support key rotation per station */
	uint32		prev_key_valid_time;
#endif /* BCMWAPI_WPI */
#ifdef MFP
	bool		sa_query_started;
	uint16		sa_query_id;
	uint32		sa_query_count;  /* SA query count */
	struct wl_timer *sa_query_timer; /* SA query timer */
#endif
	bool		stale_remove;
#ifdef WLPKTDLYSTAT
	struct wlc_scb_delay_stats	delay_stats[AC_COUNT];	/* per-AC delay stats */
#endif
};

#ifdef BCMWAPI_WPI
#define SCB_PREVOUS_KEY(scb)	((scb)->prev_key)
#else /* BCMWAPI_WPI */
#define SCB_PREVOUS_KEY(scb)	NULL
#endif /* BCMWAPI_WPI */


/* Iterator for scb list */
struct scb_iter {
	struct scb *next;
};

/* Initialize an scb iterator pre-fetching the next scb as it moves along the list */
void wlc_scb_iterinit(scb_module_t *scbstate, struct scb_iter *scbiter);
/* move the iterator */
struct scb *wlc_scb_iternext(struct scb_iter *scbiter);

#define FOREACHSCB(scbstate, scbiter, scb) \
	for (wlc_scb_iterinit((scbstate), (scbiter)); ((scb) = wlc_scb_iternext(scbiter)) != NULL; )

scb_module_t *wlc_scb_attach(wlc_pub_t *pub, wlc_info_t *wlc);
void wlc_scb_detach(scb_module_t *scbstate);

/* scb cubby cb functions */
typedef int (*scb_cubby_init_t)(void *, struct scb *);
typedef void (*scb_cubby_deinit_t)(void *, struct scb *);
typedef void (*scb_cubby_dump_t)(void *, struct scb *, struct bcmstrbuf *b);

/* This function allocates an opaque cubby of the requested size in the scb container.
 * The cb functions fn_init/fn_deinit are called when a scb is allocated/freed.
 * The functions are called with the context passed in and a scb pointer.
 * It returns a handle that can be used in macro SCB_CUBBY to retrieve the cubby.
 * Function returns a negative number on failure
 */
int wlc_scb_cubby_reserve(wlc_info_t *wlc, uint size, scb_cubby_init_t fn_init,
	scb_cubby_deinit_t fn_deinit, scb_cubby_dump_t fn_dump, void *context);

/* macro to retrieve pointer to module specific opaque data in scb container */
#define SCB_CUBBY(scb, handle)	(void *)(((uint8 *)(scb)) + handle)

/*
 * Accessors
 */

struct wlcband * wlc_scbband(struct scb *scb);

/* Find station control block corresponding to the remote id */
struct scb *wlc_scbfind(wlc_info_t *wlc, const struct ether_addr *ea);

/* Lookup station control for ID. If not found, create a new entry. */
struct scb *wlc_scblookup(wlc_info_t *wlc, const struct ether_addr *ea);

/* Lookup station control for ID. If not found, create a new entry. */
struct scb *wlc_scblookupband(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit);

/* Get scb from band */
struct scb *wlc_scbfindband(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit);

/* Determine if any SCB associated to ap cfg */
bool wlc_scb_associated_to_ap(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

/* Move the scb's band info */
void wlc_scb_update_band_for_cfg(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, chanspec_t chanspec);

struct scb *wlc_internalscb_alloc(wlc_info_t *wlc, const struct ether_addr *ea,
	struct wlcband *band);
void wlc_internalscb_free(wlc_info_t *wlc, struct scb *scb);

struct scb *wlc_userscb_alloc(wlc_info_t *wlc, struct wlcband *band);

bool wlc_scbfree(wlc_info_t *wlc, struct scb *remove);

/* * "|" operation */
void wlc_scb_setstatebit(struct scb *scb, uint8 state);

/* * "& ~" operation . */
void wlc_scb_clearstatebit(struct scb *scb, uint8 state);

/* * "|" operation . idx = position of the bsscfg in the wlc array of multi ssids. */

void wlc_scb_setstatebit_bsscfg(struct scb *scb, uint8 state, int idx);

/* * "& ~" operation . idx = position of the bsscfg in the wlc array of multi ssids. */
void wlc_scb_clearstatebit_bsscfg(struct scb *scb, uint8 state, int idx);

/* * reset all state. the multi ssid array is cleared as well. */
void wlc_scb_resetstate(struct scb *scb);

void wlc_scb_ratesel_init(wlc_info_t *wlc, struct scb *scb);
void wlc_scb_ratesel_init_bss(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

void wlc_scb_reinit(wlc_info_t *wlc);

/* free all scbs, unless permanent. Force indicates reclaim permanent as well */
void wlc_scbclear(struct wlc_info *wlc, bool force);

/* (de)authorize/(de)authenticate single station */
void wlc_scb_set_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb, bool enable,
                      uint32 flag, int rc);

/* sort rates for a single scb */
void wlc_scb_sortrates(wlc_info_t *wlc, struct scb *scb);

/* sort rates for all scb in wlc */
void BCMINITFN(wlc_scblist_validaterates)(wlc_info_t *wlc);

#ifdef AP
void wlc_scb_wds_free(struct wlc_info *wlc);
#else
#define wlc_scb_wds_free(a) do {} while (0)
#endif /* AP */

extern void wlc_scb_set_bsscfg(struct scb *scb, wlc_bsscfg_t *cfg);

/* average rssi over window */
#if defined(AP)
int wlc_scb_rssi(struct scb *scb);
void wlc_scb_rssi_init(struct scb *scb, int rssi);
#else
#define wlc_scb_rssi(a) 0
#define wlc_scb_rssi_init(a, b) 0
#endif 


/* SCB flags */
#define SCB_NONERP		0x0001		/* No ERP */
#define SCB_LONGSLOT		0x0002		/* Long Slot */
#define SCB_SHORTPREAMBLE	0x0004		/* Short Preamble ok */
#define SCB_8021XHDR		0x0008		/* 802.1x Header */
#define SCB_WPA_SUP		0x0010		/* 0 - authenticator, 1 - supplicant */
#define SCB_DEAUTH		0x0020		/* 0 - ok to deauth, 1 - no (just did) */
#define SCB_WMECAP		0x0040		/* WME Cap; may ONLY be set if WME_ENAB(wlc) */
#ifdef WLAFTERBURNER
#define SCB_ABCAP		0x0080		/* afterburner capable */
#endif /* WLAFTERBURNER */
#define SCB_BRCM		0x0100		/* BRCM AP or STA */
#define SCB_WDS_LINKUP		0x0200		/* WDS link up */
#define SCB_LEGACY_AES		0x0400		/* legacy AES device */
#define SCB_LEGACY_CRAM		0x0800
#define SCB_MYAP		0x1000		/* We are associated to this AP */
#define SCB_PENDING_PROBE	0x2000		/* Probe is pending to this SCB */
#define SCB_AMSDUCAP		0x4000		/* A-MSDU capable */
#define SCB_BACAP		0x8000		/* pre-n blockack capable */
#define SCB_HTCAP		0x10000		/* HT (MIMO) capable device */
#define SCB_RECV_PM		0x20000		/* state of PM bit in last data frame recv'd */
#define SCB_AMPDUCAP		0x40000		/* A-MPDU capable */
#define SCB_IS40		0x80000		/* 40MHz capable */
#define SCB_NONGF		0x100000	/* Not Green Field capable */
#define SCB_APSDCAP		0x200000	/* APSD capable */
#define SCB_PENDING_FREE	0x400000	/* marked for deletion - clip recursion */
#define SCB_PENDING_PSPOLL	0x800000	/* PS-Poll is pending to this SCB */
#define SCB_RIFSCAP		0x1000000	/* RIFS capable */
#define SCB_HT40INTOLERANT	0x2000000	/* 40 Intolerant */
#define SCB_WMEPS		0x4000000	/* PS + WME w/o APSD capable */
#define SCB_SENT_APSD_TRIG	0x8000000	/* APSD Trigger Null Frame was recently sent */
#define SCB_COEX_MGMT		0x10000000	/* Coexistence Management supported */
#define SCB_IBSS_PEER		0x20000000	/* Station is an IBSS peer */
#define SCB_STBCCAP		0x40000000	/* STBC Capable */
#ifdef WLBTAMP
#define SCB_11ECAP		0x80000000	/* 802.11e Cap; ONLY set if BTA_ENAB(wlc->pub) */
#endif

/* scb flags2 */
#define SCB2_SGI20_CAP		0x00000001	/* 20MHz SGI Capable */
#define SCB2_SGI40_CAP		0x00000002	/* 40MHz SGI Capable */
#define SCB2_RX_LARGE_AGG	0x00000004	/* device can rx large aggs */
#define SCB2_INTERNAL		0x00000008	/* This scb is an internal scb */
#define SCB2_IN_ASSOC		0x00000010	/* Incoming assocation in progress */
#define SCB2_P2P		0x00000040	/* WiFi P2P */
#define SCB2_LDPCCAP		0x00000080	/* LDPC Cap */
#define SCB2_BCMDCS		0x00000100 /* BCM_DCS */
#define SCB2_MFP		0x00000200 	/* MFP_ENABLE */
#define SCB2_SHA256		0x00000400 	/* sha256 for AKM */

/* scb association state bitfield */
#define UNAUTHENTICATED		0	/* unknown */
#define AUTHENTICATED		1	/* 802.11 authenticated (open or shared key) */
#define ASSOCIATED		2	/* 802.11 associated */
#define PENDING_AUTH		4	/* Waiting for 802.11 authentication response */
#define PENDING_ASSOC		8	/* Waiting for 802.11 association response */
#define AUTHORIZED		0x10	/* 802.1X authorized */
#define TAKEN4IBSS		0x80	/* Taken */

/* scb association state helpers */
#define SCB_ASSOCIATED(a)	((a)->state & ASSOCIATED)
#define SCB_AUTHENTICATED(a)	((a)->state & AUTHENTICATED)
#define SCB_AUTHORIZED(a)	((a)->state & AUTHORIZED)

/* flag access */
#define SCB_ISMYAP(a)           ((a)->flags & SCB_MYAP)
#define SCB_ISPERMANENT(a)      ((a)->permanent)
#define	SCB_INTERNAL(a) 	((a)->flags2 & SCB2_INTERNAL)
/* scb association state helpers w/ respect to ssid (in case of multi ssids)
 * The bit set in the bit field is relative to the current state (i.e. if
 * the current state is "associated", a 1 at the position "i" means the
 * sta is associated to ssid "i"
 */
#define SCB_ASSOCIATED_BSSCFG(a, i)	\
	(((a)->state & ASSOCIATED) && isset(&(scb->auth_bsscfg), i))

#define SCB_AUTHENTICATED_BSSCFG(a, i)	\
	(((a)->state & AUTHENTICATED) && isset(&(scb->auth_bsscfg), i))

#define SCB_AUTHORIZED_BSSCFG(a, i)	\
	(((a)->state & AUTHORIZED) && isset(&(scb->auth_bsscfg), i))

#define SCB_LONG_TIMEOUT	3600	/* # seconds of idle time after which we proactively
					 * free an authenticated SCB
					 */
#define SCB_SHORT_TIMEOUT	  60	/* # seconds of idle time after which we will reclaim an
					 * authenticated SCB if we would otherwise fail
					 * an SCB allocation.
					 */
#define SCB_TIMEOUT		  60	/* # seconds: interval to probe idle STAs */
#define SCB_ACTIVITY_TIME	   2	/* # seconds: skip probe if activity during this time */
#define SCB_GRACE_ATTEMPTS	   5	/* # attempts to probe sta beyond scb_activity_time */

/* scb_info macros */
#ifdef AP
#define SCB_PS(a)		((a) && (a)->PS)
#ifdef WDS
#define SCB_WDS(a)		((a)->wds)
#else
#define SCB_WDS(a)		NULL
#endif
#define SCB_INTERFACE(a)        ((a)->wds ? (a)->wds->wlif : (a)->bsscfg->wlcif->wlif)
#define SCB_WLCIFP(a)           ((a)->wds ? (a)->wds : ((a)->bsscfg->wlcif))
#define WLC_BCMC_PSMODE(wlc, bsscfg) (SCB_PS(WLC_BCMCSCB_GET(wlc, bsscfg)))
#else
#define SCB_PS(a)		NULL
#define SCB_WDS(a)		NULL
#define SCB_INTERFACE(a)        ((a)->bsscfg->wlcif->wlif)
#define SCB_WLCIFP(a)           (((a)->bsscfg->wlcif))
#define WLC_BCMC_PSMODE(wlc, bsscfg) (TRUE)
#endif /* AP */

#ifdef WME
#define SCB_WME(a)		((a)->flags & SCB_WMECAP)	/* Also implies WME_ENAB(wlc) */
#else
#define SCB_WME(a)		FALSE
#endif

#ifdef WLBA
#define SCB_BA(a)		((a)->flags & SCB_BACAP)
#else
#define SCB_BA(a)		FALSE
#endif

#ifdef WLAMPDU
#define SCB_AMPDU(a)		((a)->flags & SCB_AMPDUCAP)
#else
#define SCB_AMPDU(a)		FALSE
#endif

#ifdef WLAMSDU
#define SCB_AMSDU(a)		((a)->flags & SCB_AMSDUCAP)
#else
#define SCB_AMSDU(a)		FALSE
#endif

#ifdef WL11N
#define SCB_HT_CAP(a)		((a)->flags & SCB_HTCAP)
#define SCB_ISGF_CAP(a)		(((a)->flags & (SCB_HTCAP | SCB_NONGF)) == SCB_HTCAP)
#define SCB_NONGF_CAP(a)	(((a)->flags & (SCB_HTCAP | SCB_NONGF)) == \
					(SCB_HTCAP | SCB_NONGF))
#define SCB_COEX_CAP(a)		((a)->flags & SCB_COEX_MGMT)
#define SCB_STBC_CAP(a)		((a)->flags & SCB_STBCCAP)
#define SCB_LDPC_CAP(a)		(SCB_HT_CAP(a) && ((a)->flags2 & SCB2_LDPCCAP))
#else /* WL11N */
#define SCB_HT_CAP(a)		FALSE
#define SCB_ISGF_CAP(a)		FALSE
#define SCB_NONGF_CAP(a)	FALSE
#define SCB_COEX_CAP(a)		FALSE
#define SCB_STBC_CAP(a)		FALSE
#define SCB_LDPC_CAP(a)		FALSE
#endif /* WL11N */

#define SCB_IS_IBSS_PEER(a)	((a)->flags & SCB_IBSS_PEER)
#define SCB_SET_IBSS_PEER(a)	((a)->flags |= SCB_IBSS_PEER)
#define SCB_UNSET_IBSS_PEER(a)	((a)->flags &= ~SCB_IBSS_PEER)

#ifdef WLBTAMP
#define SCB_11E(a)		((a)->flags & SCB_11ECAP)
#else
#define SCB_11E(a)		FALSE
#endif

#ifdef WLBTAMP
#define SCB_QOS(a)		((a)->flags & (SCB_WMECAP | SCB_HTCAP | SCB_11ECAP))
#else
#define SCB_QOS(a)		((a)->flags & (SCB_WMECAP | SCB_HTCAP))
#endif /* WLBTAMP */

#ifdef WLP2P
#define SCB_P2P(a)		((a)->flags2 & SCB2_P2P)
#endif

#ifdef MFP
#define SCB_MFP(a)		((a) && ((a)->flags2 & SCB2_MFP))
#define SCB_SHA256(a)		((a) && ((a)->flags2 & SCB2_SHA256))
#endif

#define SCB_BSSCFG(a)           ((a)->bsscfg)

#define SCB_SEQNUM(scb, prio)	(scb)->seqnum[(prio)]

#define SCB_ISMULTI(a)	ETHER_ISMULTI((a)->ea.octet)

#ifdef WLCNTSCB
#define WLCNTSCBINCR(a)			((a)++)	/* Increment by 1 */
#define WLCNTSCBDECR(a)			((a)--)	/* Decrement by 1 */
#define WLCNTSCBADD(a,delta)		((a) += (delta)) /* Increment by specified value */
#define WLCNTSCBSET(a,value)		((a) = (value)) /* Set to specific value */
#define WLCNTSCBVAL(a)			(a)	/* Return value */
#define WLCNTSCB_COND_SET(c, a, v)	do { if (c) (a) = (v); } while (0)
#define WLCNTSCB_COND_ADD(c, a, d)	do { if (c) (a) += (d); } while (0)
#define WLCNTSCB_COND_INCR(c, a)	do { if (c) (a) += (1); } while (0)
#else /* WLCNTSCB */
#define WLCNTSCBINCR(a)			/* No stats support */
#define WLCNTSCBDECR(a)			/* No stats support */
#define WLCNTSCBADD(a,delta)		/* No stats support */
#define WLCNTSCBSET(a,value)		/* No stats support */
#define WLCNTSCBVAL(a)		0	/* No stats support */
#define WLCNTSCB_COND_SET(c, a, v)	/* No stats support */
#define WLCNTSCB_COND_ADD(c, a, d) 	/* No stats support */
#define WLCNTSCB_COND_INCR(c, a)	/* No stats support */
#endif /* WLCNTSCB */

/* Given the 'feature', invoke the next stage of transmission in tx path */
#define SCB_TX_NEXT(fid, scb, pkt, prec) \
	(scb->tx_path[(fid)].next_tx_fn((scb->tx_path[(fid)].next_handle), (scb), (pkt), (prec)))

/* Is the feature currently in the path to handle transmit. ACTIVE implies CONFIGURED */
#define SCB_TXMOD_ACTIVE(scb, fid) (scb->tx_path[(fid)].next_tx_fn != NULL)

/* Is the feature configured? */
#define SCB_TXMOD_CONFIGURED(scb, fid) (scb->tx_path[(fid)].configured)

extern void wlc_scb_txmod_activate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid);
extern void wlc_scb_txmod_deactivate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid);

#ifdef WDS
extern int wlc_wds_create(wlc_info_t *wlc, struct scb *scb, uint flags);
#else
#define wlc_wds_create(a, b, c)	0
#endif
/* flags for wlc_wds_create() */
#define WDS_INFRA_BSS	1	/* WDS link is part of the infra mode BSS */

#ifdef MFP
extern void *wlc_scb_send_action_sa_query(wlc_info_t *wlc, struct scb *scb,
	uint8 action, uint16 id);
extern void wlc_scb_start_sa_query(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct scb *scb);
extern void wlc_scb_sa_query(wlc_info_t *wlc, uint action_id, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, int body_len);
#endif /* MFP */

#endif /* _wlc_scb_h_ */
