/*
 * BSS Config related declarations and exported functions for
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
 * $Id: wlc_bsscfg.h,v 1.116.8.48 2011-01-07 22:52:12 Exp $
 */
#ifndef _WLC_BSSCFG_H_
#define _WLC_BSSCFG_H_

#include <wlc_types.h>

/* Check if a particular BSS config is AP or STA */
#if defined(AP) && !defined(STA)
#define BSSCFG_AP(cfg)		(1)
#define BSSCFG_STA(cfg)		(0)
#elif !defined(AP) && defined(STA)
#define BSSCFG_AP(cfg)		(0)
#define BSSCFG_STA(cfg)		(1)
#else
#define BSSCFG_AP(cfg)		((cfg)->_ap)
#define BSSCFG_STA(cfg)		(!(cfg)->_ap)
#endif /* defined(AP) && !defined(STA) */

#ifdef STA
#define BSSCFG_IBSS(cfg)	(/* BSSCFG_STA(cfg) && */!(cfg)->BSS)
#else
#define BSSCFG_IBSS(cfg)	(0)
#endif

/* forward declarations */
typedef struct wlc_bsscfg wlc_bsscfg_t;
struct scb;

#ifdef WLP2P
#include <proto/p2p.h>
#include <wlioctl.h>
#endif

#include <wlc_rate.h>

#include <wlioctl.h>

#ifdef WL11U
/* 802.11u Interworking(IW) IE size */
#define IW_IE_MAX_SIZE 		11
/* 802.11u IW Advertisement Protocol IE size */
#define IWAP_IE_MAX_SIZE 	4
#endif

#define NTXRATE		64	/* # tx MPDUs rate is reported for */

/* mac list */
#define MAXMACLIST	64     	/* max # source MAC matches */

#define BCN_TEMPLATE_COUNT 2

/* Iterator for all bsscfgs:  (wlc_info_t *wlc, int idx, wlc_bsscfg_t *cfg) */
#define FOREACH_BSS(wlc, idx, cfg) \
	for (idx = 0; (int) idx < WLC_MAXBSSCFG; idx++) \
		if ((cfg = (wlc)->bsscfg[idx]) != NULL)

/* Iterator for "associated" bsscfgs:  (wlc_info_t *wlc, int idx, wlc_bsscfg_t *cfg) */
#define FOREACH_AS_BSS(wlc, idx, cfg) \
	for (idx = 0; cfg = NULL, (int) idx < WLC_MAXBSSCFG; idx++) \
		if ((wlc)->bsscfg[idx] != NULL && \
		    (wlc)->bsscfg[idx]->associated && \
		    (cfg = (wlc)->bsscfg[idx]) != NULL)

/* Iterator for "ap" bsscfgs:  (wlc_info_t *wlc, int idx, wlc_bsscfg_t *cfg) */
#define FOREACH_AP(wlc, idx, cfg) \
	for (idx = 0; cfg = NULL, (int) idx < WLC_MAXBSSCFG; idx++) \
		if ((wlc)->bsscfg[idx] != NULL && BSSCFG_AP((wlc)->bsscfg[idx]) && \
		    (cfg = (wlc)->bsscfg[idx]) != NULL)

/* Iterator for "up" AP bsscfgs:  (wlc_info_t *wlc, int idx, wlc_bsscfg_t *cfg) */
#define FOREACH_UP_AP(wlc, idx, cfg) \
	for (idx = 0; cfg = NULL, (int) idx < WLC_MAXBSSCFG; idx++) \
		if ((wlc)->bsscfg[idx] != NULL && BSSCFG_AP((wlc)->bsscfg[idx]) && \
		    (wlc)->bsscfg[idx]->up && \
		    (cfg = (wlc)->bsscfg[idx]) != NULL)

/* Iterator for "associated" STA bsscfgs:  (wlc_info_t *wlc, int idx, wlc_bsscfg_t *cfg) */
#define FOREACH_AS_STA(wlc, idx, cfg) \
	for (idx = 0; cfg = NULL, (int) idx < WLC_MAXBSSCFG; idx++) \
		if ((wlc)->bsscfg[idx] != NULL && BSSCFG_STA((wlc)->bsscfg[idx]) && \
		    (wlc)->bsscfg[idx]->associated && \
		    (cfg = (wlc)->bsscfg[idx]) != NULL)

#define WLC_IS_CURRENT_BSSID(cfg, bssid) \
	(!bcmp((char*)(bssid), (char*)&((cfg)->BSSID), ETHER_ADDR_LEN))
#define WLC_IS_CURRENT_SSID(cfg, ssid, len) \
	(((len) == (cfg)->SSID_len) && \
	 !bcmp((char*)(ssid), (char*)&((cfg)->SSID), (len)))

#define AP_BSS_UP_COUNT(wlc) wlc_ap_bss_up_count(wlc)

/*
 * Software packet template (spt) structure; for beacons and (maybe) probe responses.
 */
#define WLC_SPT_COUNT_MAX 2
/* Turn on define to get stats on SPT */
/* #define WLC_SPT_DEBUG */

typedef struct wlc_spt
{
	uint32		in_use_bitmap;	/* Bitmap of entries in use by DMA */
	wlc_pkt_t	pkts[WLC_SPT_COUNT_MAX];	/* Pointer to array of pkts */
	int		latest_idx;	/* Most recently updated entry */
	int		dtim_count_offset; /* Where in latest_idx is dtim_count_offset */
	uint8		*tim_ie;	/* Pointer to start of TIM IE in current packet */
	bool		bcn_modified;	/* Ucode versions, TRUE: push out to shmem */

#if defined(WLC_SPT_DEBUG)
	uint32		tx_count;	/* Debug: Number of times tx'd */
	uint32		suppressed;	/* Debug: Number of times supressed */
#endif /* WLC_SPT_DEBUG */
} wlc_spt_t;

extern int wlc_spt_init(struct wlc_info *wlc, wlc_spt_t *spt, int count, int len);
extern void wlc_spt_deinit(struct wlc_info *wlc, wlc_spt_t *spt, int pkt_free_force);

/* In the case of 2 templates, return index of one not in use; -1 if both in use */
#define SPT_PAIR_AVAIL(spt) \
	(((spt)->in_use_bitmap == 0) || ((spt)->in_use_bitmap == 0x2) ? 0 : \
	((spt)->in_use_bitmap == 0x1) ? 1 : -1)

/* Is the given pkt index in use */
#define SPT_IN_USE(spt, idx) (((spt)->in_use_bitmap & (1 << (idx))) != 0)

#define SPT_LATEST_PKT(spt) ((spt)->pkts[(spt)->latest_idx])

/* MBSS debug counters */
typedef struct wlc_mbss_cnt {
	uint32		prq_directed_entries; /* Directed PRQ entries seen */
	uint32		prb_resp_alloc_fail;  /* Failed allocation on probe response */
	uint32		prb_resp_tx_fail;     /* Failed to TX probe response */
	uint32		prb_resp_retrx_fail;  /* Failed to TX probe response */
	uint32		prb_resp_retrx;       /* Retransmit suppressed probe resp */
	uint32		prb_resp_ttl_expy;    /* Probe responses suppressed due to TTL expry */
	uint32		bcn_tx_failed;	      /* Failed to TX beacon frame */

	uint32		mc_fifo_max;	/* Max number of BC/MC pkts in DMA queues */
	uint32		bcmc_count;	/* Total number of pkts sent thru BC/MC fifo */

	/* Timing and other measurements for PS transitions */
	uint32		ps_trans;	/* Total number of transitions started */
} wlc_mbss_cnt_t;

/* Set the DTIM count in the latest beacon */
#define BCN_TIM_DTIM_COUNT_OFFSET (TLV_BODY_OFF + DOT11_MNG_TIM_DTIM_COUNT) /* 2 */
#define BCN_TIM_BITMAPCTRL_OFFSET (TLV_BODY_OFF + DOT11_MNG_TIM_BITMAP_CTL) /* 4 */
#define BCN_TIM_DTIM_COUNT_SET(tim, value) (tim)[BCN_TIM_DTIM_COUNT_OFFSET] = (value)
#define BCN_TIM_BCMC_FLAG_SET(tim)         (tim)[BCN_TIM_BITMAPCTRL_OFFSET] |= 0x1
#define BCN_TIM_BCMC_FLAG_RESET(tim)       (tim)[BCN_TIM_BITMAPCTRL_OFFSET] &= 0xfe

/* Vendor IE definitions */
#define VNDR_IE_EL_HDR_LEN	(sizeof(void *))
#define VNDR_IE_MAX_TOTLEN	256	/* Space for vendor IEs in Beacons and Probe Responses */

typedef struct vndr_ie_listel {
	struct vndr_ie_listel *next_el;
	vndr_ie_info_t vndr_ie_infoel;
} vndr_ie_listel_t;

/* association states */
typedef struct wlc_assoc {
	struct wl_timer *timer;		/* timer for auth/assoc request timeout */
	uint	type;			/* roam, association, or recreation */
	uint	state;			/* current state in assoc process */
	uint	flags;			/* control flags for assoc */

	bool	preserved;		/* Whether association was preserved */
	uint	recreate_bi_timeout;	/* bcn intervals to wait to detect our former AP
					 * when performing an assoc_recreate
					 */
	uint	verify_timeout;		/* ms to wait to allow an AP to respond to class 3
					 * data if it needs to disassoc/deauth
					 */
	uint8	retry_max;		/* max. retries */
	uint8	ess_retries;		/* number of ESS retries */
	uint8	bss_retries;		/* number of BSS retries */

	uint16	capability;		/* next (re)association request overrides */
	uint16	listen;
	struct ether_addr bssid;
	uint8	*ie;
	uint	ie_len;

	struct dot11_assoc_req *req;	/* last (re)association request */
	uint	req_len;
	bool	req_is_reassoc;		/* is a reassoc */
	struct dot11_assoc_resp *resp;	/* last (re)association response */
	uint	resp_len;
} wlc_assoc_t;

/* roam states */
#define ROAM_CACHELIST_SIZE 	4
typedef struct wlc_roam {
	uint	bcn_timeout;		/* seconds w/o bcns until loss of connection */
	bool	assocroam;		/* roam to preferred assoc band in oid bcast scan */
	bool	off;			/* disable roaming */
	uint8	time_since_bcn;		/* second count since our last beacon from AP */
	uint8	tbtt_since_bcn;		/* tbtt count since our last beacon from AP */
	uint8	uatbtt_tbtt_thresh;	/* tbtts w/o bcns until unaligned tbtt checking */
	uint8	tbtt_thresh;		/* tbtts w/o bcns until roaming attempt */
	uint8	minrate_txfail_cnt;	/* tx fail count at min rate */
	uint8	minrate_txpass_cnt;	/* number of consecutive frames at the min rate */
	uint	minrate_txfail_thresh;	/* min rate tx fail threshold for roam */
	uint	minrate_txpass_thresh;	/* roamscan threshold for being stuck at min rate */
	uint	txpass_cnt;		/* turn off roaming if we get a better tx rate */
	uint	txpass_thresh;		/* turn off roaming after x many packets */
	uint32	tsf_h;			/* TSF high bits (to detect retrograde TSF) */
	uint32	tsf_l;			/* TSF low bits (to detect retrograde TSF) */
	uint	scan_block;		/* roam scan frequency mitigator */
	uint	ratebased_roam_block;	/* limits mintxrate/txfail roaming frequency */
	uint	partialscan_period;	/* user-specified roam scan period */
	uint	fullscan_period; 	/* time between full roamscans */
	uint	reason;			/* cache so we can report w/WLC_E_ROAM event */
	bool	active;			/* RSSI based roam in progress */
	bool	cache_valid;		/* RSSI roam cache is valid */
	uint	time_since_upd;		/* How long since our update? */
	uint	fullscan_count;		/* Count of full rssiroams */
	int	prev_rssi;		/* Prior RSSI, used to invalidate cache */
	uint	cache_numentries;	/* # of rssiroam APs in cache */
	bool	thrash_block_active;	/* Some/all of our APs are unavaiable to reassoc */
	bool	motion;			/* We are currently moving */
	int	RSSIref;		/* trigger for motion detection */
	uint16	motion_dur;		/* How long have we been moving? */
	uint16	motion_timeout;		/* Time left using modifed values */
	uint8	motion_rssi_delta;	/* Motion detect RSSI delta */
	bool	roam_scan_piggyback;	/* Use periodic broadcast scans as roam scans */
	bool	piggyback_enab;		/* Turn on/off roam scan piggybacking */
	struct {			/* Roam cache info */
		chanspec_t chanspec;
		struct ether_addr BSSID;
		uint16 time_left_to_next_assoc;
	} cached_ap[ROAM_CACHELIST_SIZE];
	uint8	ap_environment;		/* Auto-detect the AP density of the environment */
	bool	bcns_lost;		/* indicate if the STA can see the AP */
	uint8	consec_roam_bcns_lost;	/* counter to keep track of consecutive calls
					 * to wlc_roam_bcns_lost function
					 */
	uint8	roam_on_wowl; /* trigger roam scan (on bcn loss) for prim cfg from wowl watchdog */
} wlc_roam_t;

typedef struct wlc_link_qual {
	/* RSSI moving average */
	int	*rssi_pkt_window;	/* RSSI moving average window */
	int	rssi_pkt_index;		/* RSSI moving average window index */
	int	rssi_pkt_tot;		/* RSSI moving average total */
	uint16  rssi_pkt_count;		/* RSSI moving average count (maximum MA_WINDOW_SZ) */
	uint16  rssi_pkt_win_sz;	/* RSSI moving average window size (maximum MA_WINDOW_SZ) */
	int	rssi;			/* RSSI moving average */

	/* SNR moving average */
	int	snr;			/* SNR moving average */
	int32 	snr_ma;			/* SNR moving average total */
	uint8	snr_window[MA_WINDOW_SZ];	/* SNR moving average window */
	int	snr_index;		/* SNR moving average window index */
	int8 	snr_no_of_valid_values;	/* number of valid values in the window */

	/* RSSI event notification */
	wl_rssi_event_t *rssi_event;	/* RSSI event notification configuration. */
	uint8	rssi_level;		/* current rssi based on notification configuration */
	struct wl_timer *rssi_event_timer;	/* timer to limit event notifications */
	bool	is_rssi_event_timer_active;	/* flag to indicate timer active */
} wlc_link_qual_t;

/* PM=2 receive duration duty cycle states */
typedef enum _pm2rd_state_t {
	PM2RD_IDLE,		/* In an idle DTIM period with no data to receive and
				 * no receive duty cycle active.  ie. the last
				 * received beacon had a cleared TIM bit.
				 */
	PM2RD_WAIT_BCN,		/* In the OFF part of the receive duty cycle.
				 * In PS mode, waiting for next beacon.
				 */
	PM2RD_WAIT_TMO,		/* In the ON part of the receive duty cycle.
				 * Out of PS mode, waiting for pm2_rcv timeout.
				 */
	PM2RD_WAIT_RTS_ACK	/* Transitioning from the ON part to the OFF part of
				 * the receive duty cycle.
				 * Started entering PS mode, waiting for a
				 * PM-indicated ACK from AP to complete entering
				 * PS mode.
				 */
} pm2rd_state_t;

/* defines for ps mode requestor module id */
#define WLC_BSSCFG_PS_REQ_MCHAN		(0x00000001)

/* power management */
typedef struct wlc_pm_st {
	/* states */
	uint8	PM;			/* power-management mode (CAM, PS or FASTPS) */
	bool	PM_override;		/* no power-save flag, override PM(user input) */
	mbool	PMenabledModuleId;	/* module id that enabled pm mode */
	bool	PMenabled;		/* current power-management state (CAM or PS) */
	bool	PMawakebcn;		/* bcn recvd during current waking state */
	bool	PMpending;		/* waiting for tx status with PM indicated set */
	bool	priorPMstate;		/* Detecting PM state transitions */
	bool	PSpoll;			/* whether there is an outstanding PS-Poll frame */
	bool	check_for_unaligned_tbtt;	/* check unaligned tbtt */

	/* periodic polling */
	uint16	pspoll_prd;		/* pspoll interval in milliseconds, 0 == disable */
	struct wl_timer *pspoll_timer;	/* periodic pspoll timer */
	uint16	apsd_trigger_timeout;	/* timeout value for apsd_trigger_timer (in ms)
					 * 0 == disable
					 */
	struct wl_timer *apsd_trigger_timer;	/* timer for wme apsd trigger frames */
	bool	apsd_sta_usp;		/* Unscheduled Service Period in progress on STA */
	bool	WME_PM_blocked;		/* Can STA go to PM when in WME Auto mode */

	/* PM2 Receive Throttle Duty Cycle */
	uint16	pm2_rcv_percent;	/* Duty cycle ON % in each bcn interval */
	pm2rd_state_t pm2_rcv_state;	/* Duty cycle state */
	uint16	pm2_rcv_time;		/* Duty cycle ON time in ms */

	/* PM2 Return to Sleep */
	uint	pm2_sleep_ret_time;	/* configured time to return to sleep in ms */
	uint	pm2_sleep_ret_time_left; /* return to sleep countdown timer in ms */
	uint	pm2_last_wake_time;	/* last tx/rx activity tim in gptimer ticks(uSec) */
	bool	pm2_refresh_badiv;	/* PM2 timeout refresh with bad iv frames */

#ifdef ADV_PS_POLL
	/* send pspoll after TX */
	bool	adv_ps_poll;		/* enable/disable 'send_pspoll_after_tx' */
	bool	send_pspoll_after_tx;   /* send pspoll frame after last TX frame, to check
					 * any buffered frames in AP, during PM = 1,
					 * (or send ps poll in advance after last tx)
					 */
#endif
	wlc_hwtimer_to_t *pm2_rcv_timer; /* recv duration timeout object used with 
					 * multiplexed hw timers
					 */
	wlc_hwtimer_to_t *pm2_ret_timer; /* return to sleep timeout object used with 
					 * multiplexed hw timers
					 */
} wlc_pm_st_t;

/* join targets sorting preference */
#define MAXJOINPREFS		5	/* max # user-supplied join prefs */
#define MAXWPACFGS		16	/* max # wpa configs */
typedef struct {
	struct {
		uint8 type;		/* type */
		uint8 start;		/* offset */
		uint8 bits;		/* width */
		uint8 reserved;
	} field[MAXJOINPREFS];		/* preference field, least significant first */
	uint fields;			/* # preference fields */
	uint prfbmp;			/* preference types bitmap */
	struct {
		uint8 akm[WPA_SUITE_LEN];	/* akm oui */
		uint8 ucipher[WPA_SUITE_LEN];	/* unicast cipher oui */
		uint8 mcipher[WPA_SUITE_LEN];	/* multicast cipher oui */
	} wpa[MAXWPACFGS];		/* wpa configs, least favorable first */
	uint wpas;			/* # wpa configs */
	uint8 band;			/* 802.11 band pref */
} wlc_join_pref_t;

#ifdef SMF_STATS
typedef struct wlc_smfs_elem {
	struct wlc_smfs_elem *next;
	wl_smfs_elem_t smfs_elem;
} wlc_smfs_elem_t;

typedef struct wlc_smf_stats {
	wl_smf_stats_t smfs_main;
	uint32 count_excl; /* counts for those sc/rc code excluded from the interested group */
	wlc_smfs_elem_t *stats;
} wlc_smf_stats_t;

typedef struct wlc_smfs_info {
	uint32 enable;
	wlc_smf_stats_t smf_stats[SMFS_TYPE_MAX];
} wlc_smfs_info_t;
#endif /* SMF_STATS */

#ifdef MBSS
/* 
 * probe response element record
 * This is designed to capture certain IE's in the probe response that could be removed
 * based on the incoming probe request. For now, the need is just for ht ie's. However, more
 * elements can be added in the same fashion.
 */
typedef struct elmt_info {
	bool present;	/* is present */
	uint8 offset;	/* offset from the start of frame body */
	uint8 length;	/* the size of this element */
} elmt_info_t;

#define PRB_HTIE(cfg)	(cfg)->prb_ieinfo.ht_ie

/* please follow the order in which the ies are added in the prb resp */
typedef struct prb_ie_info {
	elmt_info_t ht_ie;
} prb_ie_info_t;
#endif /* MBSS */

#ifdef WLP2P
/* As of now, on GO there is only one peridic schedule max at any given time;
 * on Client there is one periodic schedule plus one one-tiem requested schedule max.
 */
#define WLC_P2P_MAX_SCHED	2	/* two concurrent Absence schedules */

/* On GO the start time in schedule descriptors is in local TSF time,
 * on Client the start time in schedule descriptors is in remote TSF time.
 */
typedef struct wlc_p2p_sched {
	uint8 idx;			/* current descriptor index in 'desc' */
	uint8 cnt;			/* the number of descriptors in 'desc' */
	uint8 flags;			/* see WLC_P2P_SCHED_FLAG_XXX */
	uint8 action;			/* see WL_P2P_SCHED_ACTION_XXXX */
	uint8 option;			/* see WL_P2P_SCHED_OPTION_XXXX */
	uint8 start;			/* absence start time in percent of beacon interval */
	uint8 dur;			/* absense period in percent of beacon interval */
	wl_p2p_sched_desc_t *desc;	/* len = cnt * sizeof(wl_p2p_sched_desc_t) */
} wlc_p2p_sched_t;

#define WLC_P2P_SCHED_FLAG_NORM	0x1	/* the schedule is normalized */
#define WLC_P2P_SCHED_FLAG_RUN	0x2	/* the schedule is running */

typedef struct wlc_p2p_info {
	bool enable;			/* enable/disable p2p */
	bool ps;			/* GO is in PS state */
	uint16 flags;			/* see 'flags' below */
	/* schedules - see 'sched' index below */
	wlc_p2p_sched_t sched[WLC_P2P_MAX_SCHED];
	/* current OppPS and CTWindow */
	bool ops;
	uint8 ctw;
	/* current absence schedule */
	uint8 id;			/* NoA index in beacons/prbrsps */
	uint8 action;			/* see WL_P2P_SCHED_ACTION_XXXX */
	wl_p2p_sched_desc_t cur;	/* active schedule in h/w */
	uint32 start;			/* schedule commence time */
	uint16 count;			/* schedule count cache */
	uint8 sidx;			/* schedule index into 'sched' array */
	/* back pointer to bsscfg */
	wlc_bsscfg_t *bsscfg;
	/* relay part of pretbtt interrupt work (resume tx) to tbtt time */
	wlc_hwtimer_to_t *tbtt_relay_timer;
} wlc_p2p_info_t;

/* p2p info 'flags' */
#define WLC_P2P_INFO_FLAG_CUR	0x01	/* 'cur' is valid */
#define WLC_P2P_INFO_FLAG_OPS	0x02	/* 'ops' is valid */
#define WLC_P2P_INFO_FLAG_ID	0x04	/* 'id' is valid */
#define WLC_P2P_INFO_FLAG_NET	0x08	/* network parms valid (Client assoc'd or GO is up) */
#define WLC_P2P_INFO_FLAG_STRT	0x10	/* 'start' is valid */
#ifdef WLMCHAN
#define WLC_P2P_INFO_FLAG_MCHAN_NOA	0x20 /* mchan noa schedule in use */
#endif
#define WLC_P2P_INFO_FLAG_DLY	0x40	/* delay absence start init in SHM BSS block */

/* p2p info 'sched' index */
#define WLC_P2P_SCHED_ABS	0	/* scheduled Absence */
#define WLC_P2P_SCHED_REQ_ABS	1	/* requested Absence */
#endif /* WLP2P */

/* 11g/11n protection conditions */
typedef struct wlc_prot_cond {
	/* 11g */
	uint8	nonerp_assoc;		/* NonERP STAs associated */
	uint8	longslot_assoc;		/* Long Slot timing only STAs associated */
	uint8	longpre_assoc;		/* bss/ibss: cck long preamble only STAs associated */
	/* 11n */
	bool	ofdm_assoc;		/* OFDM STAs associated */
	bool	non_gf_assoc;		/* non GF STAs associated */
	bool	ht20in40_assoc;		/* 20MHz only HT STAs associated in a 40MHz */
	bool	non11n_apsd_assoc;	/* an associated STA is non-11n APSD */
	bool	ht40intolerant_assoc;	/* an associated STA that is 40 intolerant */
	bool	wme_apsd_assoc;		/* an associated STA is using APSD */
} wlc_prot_cond_t;

/* 11g/11n protection configurations */
typedef	struct wlc_prot_cfg {
	/* 11g */
	bool	_g;			/* use g spec protection, driver internal */
	int8	g_override;		/* override for use of g spec protection */
	bool	shortpreamble;		/* currently operating with CCK ShortPreambles */
	bool	erp_ie_use_protection;	/* currently advertising Use_Protection */
	bool	erp_ie_nonerp;		/* currently advertising NonERP_Present */
	int8	barker_preamble;	/* current Barker Preamble Mode */
	bool	include_legacy_erp;	/* include Legacy ERP info elt ID 47 as well as g ID 42 */
	bool	barker_overlap_control; /* TRUE: be aware of overlapping BSSs for barker */
	/* 11n */
	int8	n_cfg;			/* use OFDM protection on MIMO frames */
	int8	n_cfg_override;		/* override for use of N protection */
	bool	nongf;			/* non-GF present protection */
	int8	nongf_override;		/* override for use of GF protection */
	bool	n_obss;			/* indicated OBSS Non-HT STA present */
	/* shared */
	int8	overlap;		/* Overlap BSS/IBSS protection for both 11g and 11n */
} wlc_prot_cfg_t;

/* 11g/11n protection timers */
typedef struct wlc_prot_to {
	/* 11g */
	uint	longpre_detect_timeout;	/* #sec until long preamble bcns gone */
	uint	barker_detect_timeout;	/* #sec until bcns signaling Barker long preamble
					 * only is gone
					 */
	uint	nonerp_ibss_timeout;	/* #sec until nonerp IBSS beacons gone */
	uint	nonerp_ovlp_timeout;	/* #sec until nonerp overlapping BSS bcns gone */
	uint	g_ibss_timeout;		/* #sec until bcns signaling Use_Protection gone */
	/* 11n */
	uint	ofdm_ibss_timeout;	/* #sec until ofdm IBSS beacons gone */
	uint	ofdm_ovlp_timeout;	/* #sec until ofdm overlapping BSS bcns gone */
	uint	n_ibss_timeout;		/* #sec until bcns signaling Use_OFDM_Protection gone */
	uint	ht20in40_ovlp_timeout;	/* #sec until 20MHz overlapping OPMODE gone */
	uint	ht20in40_ibss_timeout;	/* #sec until 20MHz-only HT station bcns gone */
	uint	non_gf_ibss_timeout;	/* #sec until non-GF bcns gone */
} wlc_prot_to_t;


#define BSSCFG_BUF_KEY_NUM	2
#define BSSCFG_BUF_KEY_PAIR_ID	0
#define BSSCFG_BUF_KEY_GRP_ID	1

#ifdef STA
typedef struct wsec_key_buf_info {
	bool eapol_4way_m1_rxed;
	bool eapol_4way_m4_txed;
	bool key_buffered[BSSCFG_BUF_KEY_NUM];
	wsec_iv_t buf_key_iv[BSSCFG_BUF_KEY_NUM];
	wl_wsec_key_t buf_key[BSSCFG_BUF_KEY_NUM];
} wsec_key_buf_info_t;
#endif /* STA */

#ifdef STA
#define BSSCFG_WSEC_BUF_KEY_B4_M4(cfg)	((cfg) && (cfg)->wsec_buf_key_b4_m4)
#else
#define BSSCFG_WSEC_BUF_KEY_B4_M4(cfg)	0
#endif

typedef struct wlc_csa {
	wl_chan_switch_t csa;
	struct wl_timer *csa_timer;	/* time to switch channel after last beacon */
	uint8		spect_state; 	/* Keep various 11h states */
	struct {
		uint32	secs;			/* seconds until channel switch */
		chanspec_t chanspec;		/* target chanspec */
	} channel_sw;
	dot11_quiet_t	quiet_cmd;
} wlc_csa_t;

/* BSS configuration state */
struct wlc_bsscfg {
	struct wlc_info	*wlc;		/* wlc to which this bsscfg belongs to. */
	bool		up;		/* is this configuration up operational */
	bool		enable;		/* is this configuration enabled */
	bool		_ap;		/* is this configuration an AP */
	bool		associated;	/* is BSS in ASSOCIATED state */
	struct wlc_if	*wlcif;		/* virtual interface, NULL for primary bsscfg */
	void		*sup;		/* pointer to supplicant state */
	int8		sup_type;	/* type of supplicant */
	bool		sup_enable_wpa;	/* supplicant WPA on/off */
	bool		BSS;		/* infraustructure or adhac */
	bool		dtim_programmed;
	void		*authenticator;	/* pointer to authenticator state */
	bool		sup_auth_pending;	/* flag for auth timeout */
	uint8		SSID_len;	/* the length of SSID */
	uint8		SSID[DOT11_MAX_SSID_LEN];	/* SSID string */
	bool		closednet_nobcnssid;	/* hide ssid info in beacon */
	bool		closednet_nobcprbresp;	/* Don't respond to broadcast probe requests */
	bool		ap_isolate;	/* true if isolating associated STA devices */
	struct scb *bcmc_scb[MAXBANDS]; /* one bcmc_scb per band */
	int8		_idx;		/* the index of this bsscfg,
					 * assigned at wlc_bsscfg_alloc()
					 */
	/* MAC filter */
	uint		nmac;		/* # of entries on maclist array */
	int		macmode;	/* allow/deny stations on maclist array */
	struct ether_addr *maclist;	/* list of source MAC addrs to match */

	/* Multicast filter list */
	bool		allmulti;		/* enable all multicasts */
	uint		nmulticast;		/* # enabled multicast addresses */
	struct ether_addr	*multicast; 	/* ptr to list of multicast addresses */

	/* security */
	uint32		wsec;		/* wireless security bitvec */
	int16		auth;		/* 802.11 authentication: Open, Shared Key, WPA */
	int16		openshared;	/* try Open auth first, then Shared Key */
	bool		wsec_restrict;	/* drop unencrypted packets if wsec is enabled */
	bool		eap_restrict;	/* restrict data until 802.1X auth succeeds */
	uint16		WPA_auth;	/* WPA: authenticated key management */
	bool		wpa2_preauth;	/* default is TRUE, wpa_cap sets value */
	bool		wsec_portopen;	/* indicates keys are plumbed */
	wsec_iv_t	wpa_none_txiv;	/* global txiv for WPA_NONE, tkip and aes */
	int		wsec_index;	/* 0-3: default tx key, -1: not set */
	wsec_key_t	*bss_def_keys[WLC_DEFAULT_KEYS];	/* default key storage */

#ifdef MBSS
	wlc_pkt_t	probe_template;	/* Probe response master packet, including PLCP */
#ifdef WLLPRS
	wlc_pkt_t	lprs_template;	/* Legacy probe response master packet */
	prb_ie_info_t	prb_ieinfo;	/* information of certain ies of interest */
#endif /* WLLPRS */
	bool		prb_modified;	/* Ucode version: push to shm if true */
	wlc_spt_t	*bcn_template;	/* Beacon DMA template */
	uint32		maxassoc;	/* Max associations for this bss */
	int8		_ucidx;		/* the uCode index of this bsscfg,
					 * assigned at wlc_bsscfg_up()
					 */
	uint32		mc_fifo_pkts;	/* Current number of BC/MC pkts sent to DMA queues */
	uint32		prb_ttl_us;     /* Probe rsp time to live since req. If 0, disabled */
#ifdef WLCNT
	wlc_mbss_cnt_t *cnt;		/* MBSS debug counters */
#endif
#if defined(BCMDBG_MBSS_PROFILE)
	uint32		ps_start_us;	/* When last PS (off) transition started */
	uint32		max_ps_off_us;	/* Max delay time for out-of-PS transitions */
	uint32		tot_ps_off_us;	/* Total time delay for out-of-PS transitions */
	uint32		ps_off_count;	/* Number of deferred out-of-PS transitions completed */
	bool		bcn_tx_done;	/* TX done on sw beacon */
#endif /* BCMDBG_MBSS_PROFILE */
#endif /* MBSS */

	/* TKIP countermeasures */
	bool		tkip_countermeasures;	/* flags TKIP no-assoc period */
	uint32		tk_cm_dt;	/* detect timer */
	uint32		tk_cm_bt;	/* blocking timer */
	uint32		tk_cm_bt_tmstmp;    /* Timestamp when TKIP BT is activated */
	bool		tk_cm_activate;	/* activate countermeasures after EAPOL-Key sent */
#ifdef AP
	uint8		aidmap[AIDMAPSZ];	/* aid map */
#endif
#ifdef WMF
	bool		wmf_enable;	/* WMF is enabled or not */
	bool		wmf_ucast_igmp;	/* 1 to enable, 0 by default */
	struct wlc_wmf_instance	*wmf_instance; /* WMF instance handle */
#endif
#ifdef MCAST_REGEN
	bool		mcast_regen_enable;	/* Multicast Regeneration is enabled or not */
#endif
	vndr_ie_listel_t	*vndr_ie_listp;	/* dynamic list of Vendor IEs */
	struct ether_addr	BSSID;		/* BSSID (associated) */
	struct ether_addr	cur_etheraddr;	/* h/w address */
	uint16                  bcmc_fid;	/* the last BCMC FID queued to TX_BCMC_FIFO */
	uint16                  bcmc_fid_shm;	/* the last BCMC FID written to shared mem */

	uint32		flags;		/* WLC_BSSCFG flags; see below */
#ifdef STA
	/* Association parameters. Used to limit the scan in join process. Saved before
	 * starting a join process and freed after finishing the join process regardless
	 * if the join is succeeded or failed.
	 */
	wl_join_assoc_params_t	*assoc_params;
	uint16			assoc_params_len;
#endif
	struct ether_addr	prev_BSSID;	/* MAC addr of last associated AP (BSS) */


#if defined(MACOSX)
	bool	sendup_mgmt;		/* sendup mgmt per packet filter setting */
#endif

	/* for Win7 */
	uint8		*bcn;		/* AP beacon */
	uint		bcn_len;	/* AP beacon length */
	bool		ar_disassoc;	/* disassociated in associated recreation */

	int		auth_atmptd;	/* auth type (open/shared) attempted */

	/* PMKID caching */
	pmkid_cand_t	pmkid_cand[MAXPMKID];	/* PMKID candidate list */
	uint		npmkid_cand;	/* num PMKID candidates */
	pmkid_t		pmkid[MAXPMKID];	/* PMKID cache */
	uint		npmkid;		/* num cached PMKIDs */

	wlc_bss_info_t	*target_bss;	/* BSS parms during tran. to ASSOCIATED state */
	wlc_bss_info_t	*current_bss;	/* BSS parms in ASSOCIATED state */

	wlc_assoc_t	*assoc;		/* association mangement */
	wlc_roam_t	*roam;		/* roam states */
	wlc_link_qual_t	*link;		/* link quality monitor */
	wlc_pm_st_t	*pm;		/* power management */
	bool		as_rt;		/* true if sta retry timer active */

	/* join targets sorting preference */
	wlc_join_pref_t *join_pref;
	/* Give RSSI score of APs in preferred band a boost
	 * to make them fare better instead of always preferring
	 * the band. This is maintained separately from regular
	 * join pref as no order can be imposed on this preference
	 */
	struct {
		uint8 band;
		uint8 rssi;
	} join_pref_rssi_delta;

#ifdef WLP2P
	/* p2p info */
	wlc_p2p_info_t	*p2p;
	/* p2p flags */
	uint16		p2p_flags;
	/* base TSF offset (for clock drift detect) */
	int16		tsfo_base;
	/* multiple MAC address manipulation */
	uint8		rcmta_ra_idx;		/* RCMTA idx for RA to be used for P2P */
	uint8		rcmta_bssid_idx;	/* RCMTA idx for BSSID to be used for P2P */
	/* d11 info - see 'M_P2P_BSS_BLK' in d11.h */
	uint8		d11cbi;			/* d11 SHM per BSS control block index */
	/* network parms */
	uint32		tbtt_h;			/* remote tbtt in local TSF time */
	uint32		tbtt_l;			/* remote tbtt in local TSF time */
	int32		tsfo_h;			/* TSF offset (local - remote) */
	int32		tsfo_l;			/* TSF offset (local - remote) */
	bool		p2p_bcn_offset_ok;
	bool		p2p_bcn_offset_cache;
	uint8		p2p_num_bad_bcn_offset;
	uint32		p2p_bcn_offset;
#endif /* WLP2P */
	/* BSSID entry in RCMTA, use the wsec key management infrastructure to
	 * manage the RCMTA entries.
	 */
	wsec_key_t	*rcmta;

#ifdef SMF_STATS
	wlc_smfs_info_t *smfs_info;
#endif /* SMF_STATS */
	int8	PLCPHdr_override;	/* 802.11b Preamble Type override */

	/* 'unique' ID of this bsscfg, assigned at bsscfg allocation */
	uint16		ID;

	uint		txrspecidx;		/* index into tx rate circular buffer */
	ratespec_t     	txrspec[NTXRATE][2];	/* circular buffer of prev MPDUs tx rates */

#ifdef WL_BSSCFG_TX_SUPR
	/* tx suppression handling */
	struct pktq	*psq;			/* defer queue */
	bool		tx_start_pending;	/* defer on/off */
#endif


#ifdef WLMCHAN
	wlc_mchan_context_t *chan_context;	/* chanspec context for bsscfg */
	uint16 sw_dtim_cnt;			/* dtim cnt kept upto date by sw */
	chanspec_t chanspec;			/* chanspec specified when starting AP */
	uint8 mchan_tbtt_since_bcn;		/* number of tbtt since last bcn */
#endif /* WLMCHAN */

#ifdef STA
	/* Scan parameters. Used to modify the scan parameters in join process.
	 * Saved before starting a join process and freed after finishing the join
	 * regardless if the join is succeeded or failed.
	 */
	wl_join_scan_params_t	*scan_params;
#endif

#ifdef WL11N
	/* SM PS */
	uint8	mimops_PM;
	uint8	mimops_ActionPM;
	uint8  	mimops_ActionRetry;
	bool    mimops_ActionPending;
#endif /* WL11N */

	/* 11g/11n protections */
	wlc_prot_cond_t	*prot_cond;	/* conditions */
	wlc_prot_cfg_t	*prot_cfg;	/* configurations */
	wlc_prot_to_t	*prot_to;	/* timeouts */

	/* Channel Switch Announcement */
	wlc_csa_t	*csa;


#ifdef STA
	bool wsec_buf_key_b4_m4;
	wsec_key_buf_info_t *wsec_key_buf_info;
#endif

	/* Broadcom proprietary information element */
	uint8		*brcm_ie;

#ifdef WL11U
	uint8		*iw_ie; /* 802.11u interworking(IW) IE */
#endif /* WL11U */

#ifdef MFP
	/* integrated group key stuff */
	wsec_igtk_info_t igtk;
#endif

	/* LEAVE THESE AT THE END */
#ifdef BCMDBG
	/* Rapid PM transition */
	wlc_hwtimer_to_t *rpmt_timer;
	uint32	rpmt_1_prd;
	uint32	rpmt_0_prd;
	uint8	rpmt_n_st;
#endif
	/* LEAVE THESE AT THE END */
};

/* wlc_bsscfg_t flags */
#define WLC_BSSCFG_PRESERVE     0x1		/* preserve STA association on driver down/up */
#define WLC_BSSCFG_WME_DISABLE	0x2		/* Do not advertise WME for this BSS */
#define WLC_BSSCFG_PS_OFF_TRANS	0x4		/* BSS is in transition to PS-OFF */
#define WLC_BSSCFG_SW_BCN	0x8		/* The BSS is generating beacons in SW */
#define WLC_BSSCFG_SW_PRB	0x10		/* The BSS is generating probe responses in SW */
#define WLC_BSSCFG_HW_BCN	0x20		/* The BSS is generating beacons in HW */
#define WLC_BSSCFG_HW_PRB	0x40		/* The BSS is generating probe responses in HW */
#define WLC_BSSCFG_DPT		0x80		/* The BSS is for DPT link */
#define WLC_BSSCFG_MBSS16	0x100		/* The BSS creates bcn/prbresp template for ucode */
#define WLC_BSSCFG_BTA		0x200		/* The BSS is for BTA link */
#define WLC_BSSCFG_NOBCMC	0x400		/* The BSS has no broadcast/multicast traffic */
#define WLC_BSSCFG_NOIF		0x800		/* The BSS has no OS presentation */
#define WLC_BSSCFG_11N_DISABLE	0x1000		/* Do not advertise .11n IEs for this BSS */
#define WLC_BSSCFG_P2P		0x2000		/* The BSS is for p2p link */
#define WLC_BSSCFG_11H_DISABLE	0x4000		/* Do not follow .11h rules for this BSS */
#define WLC_BSSCFG_NATIVEIF	0x8000		/* The BSS uses native OS if */
#define WLC_BSSCFG_P2P_DISC	0x10000		/* The BSS is for p2p discovery */
#define WLC_BSSCFG_TX_SUPR	0x20000		/* The BSS is in absence mode */
#define WLC_BSSCFG_SRADAR_ENAB	0x40000		/* follow special radar rules for soft/ext ap */
#define WLC_BSSCFG_DYNBCN	0x80000		/* Do not beacon if no client is associated */
#define WLC_BSSCFG_P2P_RESET	0x100000	/* reset the existing P2P bsscfg to default */
#define WLC_BSSCFG_P2P_RECREATE_BSSIDX 0x200000	/* alloc new bssid_idx in wlc_bsscfg_p2p_init */
#define WLC_BSSCFG_AP_NORADAR_CHAN 0x400000	/* disallow ap to start on radar channel */


/* Init strings for flags */
#define WLC_BSSCFG_FLAGS_STR_INIT \
	"PRESERVE_ASSOC", \
	"WME_DISABLE", \
	"PS_OFF_TRANS", \
	"SW_BCN", \
	"SW_PRB", \
	"HW_BCN", \
	"HW_PRB", \
	"DPT", \
	"MBSS16", \
	"BTA", \
	"NOBCMC", \
	"NOIF", \
	"11N_DISABLE", \
	"P2P", \
	"11H_DISABLE", \
	"NATIVEIF", \
	"P2P_DISC", \
	"TX_SUPR", \
	"SRADAR", \
	"DynBcn", \
	"P2P_RESET", \
	"P2P_RECREATE_BSSIDX", \
	"AP_NORADAR_CHAN"

/* Flags that should not be cleared on AP bsscfg up */
#define WLC_BSSCFG_PERSIST_FLAGS (0 | \
		WLC_BSSCFG_WME_DISABLE | \
		WLC_BSSCFG_PRESERVE | \
		WLC_BSSCFG_BTA | \
		WLC_BSSCFG_NOBCMC | \
		WLC_BSSCFG_NOIF | \
		WLC_BSSCFG_11N_DISABLE | \
		WLC_BSSCFG_P2P | \
		WLC_BSSCFG_11H_DISABLE | \
		WLC_BSSCFG_P2P_DISC | \
		WLC_BSSCFG_NATIVEIF | \
		WLC_BSSCFG_SRADAR_ENAB | \
		WLC_BSSCFG_DYNBCN | \
		WLC_BSSCFG_AP_NORADAR_CHAN | \
	0)

/* Flags for special bsscfgs */
#define WLC_BSSCFG_EXAMPT_FLAGS (0 | \
		WLC_BSSCFG_DPT | \
	0)

#ifdef WLP2P
/* p2p_flags */
#define WLC_BSSCFG_P2P_P2P_IE		0x1	/* Include P2P IEs */
#define WLC_BSSCFG_P2P_VAL_TBTT		0x2	/* tbtt info is init'd and valid */
#define WLC_BSSCFG_P2P_SHM_TSF		0x4	/* tsf info in SHM TSF block is init'd */
#define WLC_BSSCFG_P2P_SHM_TBTT		0x8	/* tbtt info in SHM BSS block is init'd */
#define WLC_BSSCFG_P2P_UPD_BSS		0x10	/* request to update the SHM BSS block */
#define WLC_BSSCFG_P2P_IGN_SMPS		0x20	/* don't process SM PS in beacons */
#define WLC_BSSCFG_P2P_APSD_MORE	0x40	/* SP is interrupted by NoA absence */
#endif

#define WLC_BSSCFG(wlc, idx) \
	(((idx) < WLC_MAXBSSCFG && (idx) >= 0) ? ((wlc)->bsscfg[idx]) : NULL)

#define HWBCN_ENAB(cfg)		(((cfg)->flags & WLC_BSSCFG_HW_BCN) != 0)
#define HWPRB_ENAB(cfg)		(((cfg)->flags & WLC_BSSCFG_HW_PRB) != 0)
#define DYNBCN_ENAB(cfg)	(((cfg)->flags & WLC_BSSCFG_DYNBCN) != 0)

#define BSSCFG_HAS_NOIF(cfg)	(((cfg)->flags & WLC_BSSCFG_NOIF) != 0)
#define BSSCFG_HAS_NATIVEIF(cfg)	(((cfg)->flags & WLC_BSSCFG_NATIVEIF) != 0)


#ifdef WMF
#define WMF_ENAB(cfg)	((cfg)->wmf_enable)
#else
#define WMF_ENAB(cfg)	FALSE
#endif

#ifdef MCAST_REGEN
#define MCAST_REGEN_ENAB(cfg)	((cfg)->mcast_regen_enable)
#else
#define MCAST_REGEN_ENAB(cfg)	FALSE
#endif

#ifdef SMF_STATS
#define SMFS_ENAB(cfg) ((cfg)->smfs_info->enable)
#else
#define SMFS_ENAB(cfg) FALSE
#endif /* SMF_STATS */

typedef struct wlc_prq_info_s {
	shm_mbss_prq_entry_t source;   /* To get ta addr and timestamp directly */
	bool is_directed;         /* Non-broadcast (has bsscfg associated with it) */
	bool directed_ssid;       /* Was request directed to an SSID? */
	bool directed_bssid;      /* Was request directed to a BSSID? */
	wlc_bsscfg_t *bsscfg;     /* The BSS Config associated with the request (if not bcast) */
	shm_mbss_prq_ft_t frame_type;  /* 0: cck; 1: ofdm; 2: mimo; 3 rsvd */
	bool up_band;             /* Upper or lower sideband of 40 MHz for MIMO phys */
	uint8 plcp0;              /* First byte of PLCP */
#ifdef WLLPRS
	bool is_htsta;		/* is from an HT sta */
#endif /* WLLPRS */
} wlc_prq_info_t;

extern wlc_bsscfg_t *wlc_bsscfg_primary(struct wlc_info *wlc);
extern int wlc_bsscfg_primary_init(struct wlc_info *wlc);
extern void wlc_bsscfg_primary_cleanup(struct wlc_info *wlc);
extern wlc_bsscfg_t *wlc_bsscfg_alloc(struct wlc_info *wlc, int idx, uint flags,
                                      struct ether_addr *ea, bool ap);
extern int wlc_bsscfg_vif_reset(wlc_info_t *wlc, int idx, uint flags,
	struct ether_addr *ea, bool ap);
extern int wlc_bsscfg_init(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
extern int wlc_bsscfg_reinit(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg, bool ap);
extern void wlc_bsscfg_deinit(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
extern int wlc_bsscfg_bta_init(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
extern void wlc_bsscfg_dpt_init(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
#ifdef WLP2P
extern int wlc_bsscfg_p2p_init(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
#endif
extern int wlc_bsscfg_vif_init(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
extern void wlc_bsscfg_free(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
extern void wlc_bsscfg_disablemulti(struct wlc_info *wlc);
extern int wlc_bsscfg_disable(struct wlc_info *wlc, wlc_bsscfg_t *cfg);
extern int wlc_bsscfg_down(struct wlc_info *wlc, wlc_bsscfg_t *cfg);
extern int wlc_bsscfg_up(struct wlc_info *wlc, wlc_bsscfg_t *cfg);
extern int wlc_bsscfg_enable(struct wlc_info *wlc, wlc_bsscfg_t *cfg);
extern int wlc_bsscfg_get_free_idx(struct wlc_info *wlc);
extern wlc_bsscfg_t *wlc_bsscfg_find(struct wlc_info *wlc, int idx, int *perr);
extern wlc_bsscfg_t *wlc_bsscfg_find_by_hwaddr(struct wlc_info *wlc, struct ether_addr *hwaddr);
extern wlc_bsscfg_t *wlc_bsscfg_find_by_bssid(struct wlc_info *wlc, struct ether_addr *bssid);
extern wlc_bsscfg_t *wlc_bsscfg_find_by_target_bssid(struct wlc_info *wlc,
	struct ether_addr *bssid);
extern wlc_bsscfg_t *wlc_bsscfg_find_by_ssid(struct wlc_info *wlc, uint8 *ssid, int ssid_len);
extern wlc_bsscfg_t *wlc_bsscfg_find_by_wlcif(struct wlc_info *wlc, wlc_if_t *wlcif);
extern wlc_bsscfg_t *wlc_bsscfg_find_by_ID(struct wlc_info *wlc, uint16 id);
extern int wlc_ap_bss_up_count(struct wlc_info *wlc);
#ifdef STA
extern int wlc_bsscfg_assoc_params_set(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg,
	wl_join_assoc_params_t *assoc_params, int assoc_params_len);
extern void wlc_bsscfg_assoc_params_reset(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
#define wlc_bsscfg_assoc_params(bsscfg)	(bsscfg)->assoc_params
extern int wlc_bsscfg_scan_params_set(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg,
	wl_join_scan_params_t *scan_params);
extern void wlc_bsscfg_scan_params_reset(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);
#define wlc_bsscfg_scan_params(bsscfg)	(bsscfg)->scan_params
#endif
extern void wlc_bsscfg_SSID_set(wlc_bsscfg_t *bsscfg, uint8 *SSID, int len);
extern void wlc_bsscfg_scbclear(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg, bool perm);
extern void wlc_bsscfg_ID_assign(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg);

extern void wlc_bsscfg_bcn_disable(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_bsscfg_bcn_enable(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

#define WLC_BCMCSCB_GET(wlc, bsscfg) \
	(((bsscfg)->flags & WLC_BSSCFG_NOBCMC) ? \
	 NULL : \
	 (bsscfg)->bcmc_scb[CHSPEC_WLCBANDUNIT((wlc)->home_chanspec)])
#define WLC_BSSCFG_IDX(bsscfg) ((bsscfg)->_idx)

typedef bool (*vndr_ie_filter_fn_t)(wlc_bsscfg_t *bsscfg, vndr_ie_t *ie);
extern int wlc_vndr_ie_getlen_ext(wlc_bsscfg_t *bsscfg, vndr_ie_filter_fn_t filter,
	uint32 pktflag, int *totie);
#define wlc_vndr_ie_getlen(cfg, pktflag, totie) \
		wlc_vndr_ie_getlen_ext(cfg, NULL, pktflag, totie)
typedef bool (*vndr_ie_write_filter_fn_t)(wlc_bsscfg_t *bsscfg, uint type, vndr_ie_t *ie);
extern uint8 *wlc_vndr_ie_write_ext(wlc_bsscfg_t *bsscfg, vndr_ie_write_filter_fn_t filter,
	uint type, uint8 *cp, int buflen, uint32 pktflag);
#define wlc_vndr_ie_write(cfg, cp, buflen, pktflag) \
		wlc_vndr_ie_write_ext(cfg, NULL, -1, cp, buflen, pktflag)
#ifdef WLP2P
extern int wlc_p2p_vndr_ie_getlen(wlc_bsscfg_t *bsscfg, uint32 pktflag, int *totie);
extern uint8 *wlc_p2p_vndr_ie_write(wlc_bsscfg_t *bsscfg, uint type,
	uint8 *cp, int buflen, uint32 pktflag);
#endif
extern vndr_ie_listel_t *wlc_vndr_ie_add_elem(wlc_bsscfg_t *bsscfg, uint32 pktflag,
	vndr_ie_t *vndr_iep);
extern vndr_ie_listel_t *wlc_vndr_ie_mod_elem(wlc_bsscfg_t *bsscfg, vndr_ie_listel_t *old_listel,
	uint32 pktflag, vndr_ie_t *vndr_iep);
extern int wlc_vndr_ie_add(wlc_bsscfg_t *bsscfg, vndr_ie_buf_t *ie_buf, int len);
extern int wlc_vndr_ie_del(wlc_bsscfg_t *bsscfg, vndr_ie_buf_t *ie_buf, int len);
extern int wlc_vndr_ie_get(wlc_bsscfg_t *bsscfg, vndr_ie_buf_t *ie_buf, int len,
	uint32 pktflag);
extern void wlc_vndr_ie_free(wlc_bsscfg_t *bsscfg);
extern int wlc_vndr_ie_buflen(vndr_ie_buf_t *ie_buf, int len, int *bcn_ielen, int *prbrsp_ielen);

extern int wlc_vndr_ie_mod_elem_by_type(wlc_bsscfg_t *bsscfg, uint8 type,
	uint32 pktflag, vndr_ie_t *vndr_iep);
extern int wlc_vndr_ie_del_by_type(wlc_bsscfg_t *bsscfg, uint8 type);
extern uint8 *wlc_vndr_ie_find_by_type(wlc_bsscfg_t *bsscfg, uint8 type);

extern uint8 *wlc_bsscfg_get_ie(wlc_bsscfg_t *bsscfg, uint8 ie_type);
extern int wlc_bsscfg_set_ie(wlc_bsscfg_t *bsscfg, uint8 *ie_data,
	bool *bcn_upd, bool *prbresp_upd);

#if defined(AP)
extern uint16 wlc_bsscfg_newaid(wlc_bsscfg_t *cfg);
#endif /* AP */

#ifdef SMF_STATS
extern int wlc_smfstats_update(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg, uint8 smfs_type,
	uint16 code);
extern int wlc_bsscfg_smfsinit(struct wlc_info *wlc, wlc_bsscfg_t *cfg);
extern int wlc_bsscfg_get_smfs(wlc_bsscfg_t *cfg, int idx, char* buf, int len);
extern int wlc_bsscfg_clear_smfs(struct wlc_info *wlc, wlc_bsscfg_t *cfg);
#else
#define wlc_smfstats_update(a, b, c, d) do {} while (0)
#define wlc_bsscfg_smfsinit(a, b) do {} while (0)
#define wlc_bsscfg_get_smfs(a, b, c, d) do {} while (0)
#define wlc_bsscfg_clear_smfs(a, b) do {} while (0)
#endif

#define WLC_BSSCFG_AUTH(cfg) ((cfg)->auth)

#define WLC_BSSCFG_DEFKEY(cfg, idx) \
	(((idx) < WLC_DEFAULT_KEYS && (int)(idx) >= 0) ? \
	((cfg)->bss_def_keys[idx]) : 0)

/* Extend WME_ENAB to per-BSS */
#define BSS_WME_ENAB(wlc, cfg) \
	(WME_ENAB((wlc)->pub) && !((cfg)->flags & WLC_BSSCFG_WME_DISABLE))

#ifdef WLBTAMP
/* Extend BTA_ENAB to per-BSS */
#define BSS_BTA_ENAB(wlc, cfg) \
	(BTA_ENAB((wlc)->pub) && (cfg) && ((cfg)->flags & WLC_BSSCFG_BTA))
#endif

/* Extend N_ENAB to per-BSS */
#define BSS_N_ENAB(wlc, cfg) \
	(N_ENAB((wlc)->pub) && !((cfg)->flags & WLC_BSSCFG_11N_DISABLE))

/* Extend WL11H_ENAB to per-BSS */
#define BSS_WL11H_ENAB(wlc, cfg) \
	(WL11H_ENAB(wlc) && !((cfg)->flags & WLC_BSSCFG_11H_DISABLE))

/* Extend P2P_ENAB to per-BSS */
#ifdef WLP2P
#define BSS_P2P_ENAB(wlc, cfg) \
	(P2P_ENAB((wlc)->pub) && ((cfg)->flags & WLC_BSSCFG_P2P) != 0)
#define BSS_P2P_DISC_ENAB(wlc, cfg) \
	(P2P_ENAB((wlc)->pub) && ((cfg)->flags & WLC_BSSCFG_P2P_DISC) != 0)
#else
#define BSS_P2P_ENAB(wlc, cfg)	(FALSE)
#define BSS_P2P_DISC_ENAB(wlc, cfg)	(FALSE)
#endif

#ifdef WL_BSSCFG_TX_SUPR
#define BSS_TX_SUPR(cfg) ((cfg)->flags & WLC_BSSCFG_TX_SUPR)
#else
#define BSS_TX_SUPR(cfg) 0
#endif

/* Macros related to Multi-BSS. */
#if defined(MBSS)
#define SOFTBCN_ENAB(cfg)		(((cfg)->flags & WLC_BSSCFG_SW_BCN) != 0)
#define SOFTPRB_ENAB(cfg)		(((cfg)->flags & WLC_BSSCFG_SW_PRB) != 0)
#define UCTPL_MBSS_ENAB(cfg)		(((cfg)->flags & WLC_BSSCFG_MBSS16) != 0)
/* Define as all bits less than and including the msb shall be one's */
#define EADDR_TO_UC_IDX(eaddr, mask)		((eaddr).octet[5] & (mask))
#define WLC_BSSCFG_UCIDX(bsscfg)	((bsscfg)->_ucidx)
#define MBSS_BCN_ENAB(cfg)		(SOFTBCN_ENAB(cfg) || UCTPL_MBSS_ENAB(cfg))
#define MBSS_PRB_ENAB(cfg)		(SOFTPRB_ENAB(cfg) || UCTPL_MBSS_ENAB(cfg))

/*
 * BC/MC FID macros.  Only valid under MBSS
 *
 *    BCMC_FID_SHM_COMMIT  Committing FID to SHM; move driver's value to bcmc_fid_shm
 *    BCMC_FID_QUEUED	   Are any packets enqueued on the BC/MC fifo?
 */

extern void bcmc_fid_shm_commit(wlc_bsscfg_t *bsscfg);
#define BCMC_FID_SHM_COMMIT(bsscfg) bcmc_fid_shm_commit(bsscfg)

#define BCMC_PKTS_QUEUED(bsscfg) \
	(((bsscfg)->bcmc_fid_shm != INVALIDFID) || ((bsscfg)->bcmc_fid != INVALIDFID))

extern int wlc_write_mbss_basemac(struct wlc_info *wlc, const struct ether_addr *addr);

#else

#define SOFTBCN_ENAB(pub)    (0)
#define SOFTPRB_ENAB(pub)    (0)
#define WLC_BSSCFG_UCIDX(bsscfg) 0
#define wlc_write_mbss_basemac(wlc, addr) (0)

#define BCMC_FID_SHM_COMMIT(bsscfg)
#define BCMC_PKTS_QUEUED(bsscfg) 0
#define MBSS_BCN_ENAB(cfg)       0
#define MBSS_PRB_ENAB(cfg)       0
#define UCTPL_MBSS_ENAB(cfg)         0
#endif /* defined(MBSS) */

#define BSS_11H_SRADAR_ENAB(wlc, cfg)	(WL11H_ENAB(wlc) && BSSCFG_SRADAR_ENAB(cfg))
#define BSSCFG_SRADAR_ENAB(cfg)	((cfg)->flags & WLC_BSSCFG_SRADAR_ENAB)
#define BSS_11H_AP_NORADAR_CHAN_ENAB(wlc, cfg) (WL11H_ENAB(wlc) && BSSCFG_AP_NORADAR_CHAN_ENAB(cfg))
#define BSSCFG_AP_NORADAR_CHAN_ENAB(cfg)	((cfg)->flags & WLC_BSSCFG_AP_NORADAR_CHAN)

#ifdef WL_BSSCFG_TX_SUPR
extern void wlc_bsscfg_tx_stop(wlc_bsscfg_t *bsscfg);
extern void wlc_bsscfg_tx_start(wlc_bsscfg_t *bsscfg);
extern bool wlc_bsscfg_txq_enq(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg, void *sdu, uint prec);
extern void wlc_bsscfg_tx_check(struct wlc_info *wlc);
#else
#define wlc_bsscfg_tx_stop(a) do { } while (0)
#define wlc_bsscfg_tx_start(a) do { } while (0)
#define wlc_bsscfg_txq_enq(a, b, c, d) FALSE
#define wlc_bsscfg_tx_check(a) do { } while (0)
#endif
#define BSSCFG_SAFEMODE(cfg)	0
#ifdef WLMCHAN
void wlc_bsscfg_ap_tbtt_setup(wlc_info_t *wlc, wlc_bsscfg_t *ap_cfg);
#endif /* WLMCHAN */
#ifdef STA
extern void wlc_bsscfg_wsec_session_reset(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern int wlc_bsscfg_wsec_key_buf_init(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
extern int wlc_bsscfg_wsec_key_buf_free(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
#endif /* STA */
#endif	/* _WLC_BSSCFG_H_ */
