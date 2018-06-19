/*
 * AP Module
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_ap.c,v 1.362.2.60 2011-02-08 01:50:26 Exp $
 */
#ifndef AP
#error "AP must be defined to include this module"
#endif  /* AP */

#include <typedefs.h>
#include <bcmdefs.h>
#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#include <sbconfig.h>
#include <wlioctl.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#include <bcmcrypto/wep.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_led.h>
#include <wlc_security.h>
#include <wlc_event.h>
#include <wl_export.h>
#include <wlc_apcs.h>
#include <wlc_stf.h>
#include <wlc_dpt.h>
#include <wlc_ap.h>
#include <wlc_scan.h>
#include <wlc_ampdu.h>
#include <wlc_amsdu.h>
#ifdef	WLCAC
#include <wlc_cac.h>
#endif
#ifdef WLBTAMP
#include <wlc_bta.h>
#endif
#include <wlc_bmac.h>
#ifdef BCMAUTH_PSK
#include <wlc_auth.h>
#endif
#ifdef APCS
#include <wlc_apcs.h>
#endif
#include <wlc_assoc.h>
#ifdef WLP2P
#include <wlc_p2p.h>
#endif
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif
#ifdef WLMEDIA_IPTV
#include <wlc_wmf.h>
#endif

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)

#define PWRSAVE_RXCHAIN 1
#define PWRSAVE_RADIO 2

/*
 * This is a generic structure for power save implementations
 * Defines parameters for packets per second threshold based power save
 */
typedef struct wlc_pwrsave {
	bool	in_power_save;		/* whether we are in power save mode or not */
	uint8	power_save_check;	/* Whether power save mode check need to be done */
	uint8   stas_assoc_check;	/* check for associated STAs before going to power save */
	uint	in_power_save_counter;	/* how many times in the power save mode */
	uint	in_power_save_secs;	/* how many seconds in the power save mode */
	uint	quiet_time_counter;	/* quiet time before we enter the  power save mode */
	uint	prev_pktcount;		/* total pkt count from the previous second */
	uint	quiet_time;		/* quiet time in the network before we go to power save */
	uint	pps_threshold;		/* pps threshold for power save */
} wlc_pwrsave_t;

#endif /* RXCHAIN_PWRSAVE or RADIO_PWRSAVE */

#ifdef RXCHAIN_PWRSAVE

typedef struct wlc_rxchain_pwrsave {
#ifdef WL11N
	/* need to save rx_stbc HT capability before enter rxchain_pwrsave mode */
	uint8	ht_cap_rx_stbc;		/* configured rx_stbc HT capability */
#endif
	uint	rxchain;		/* configured rxchains */
	wlc_pwrsave_t pwrsave;
} wlc_rxchain_pwrsave_t;

#endif /* RXCHAIN_PWRSAVE */


#ifdef RADIO_PWRSAVE

#define RADIO_PWRSAVE_TIMER_LATENCY	15

typedef struct wlc_radio_pwrsave {
	uint8  level;			/* Low, Medium or High Power Savings */
	uint16 on_time;			/* number of  TUs radio is 'on' */
	uint16 off_time;		/* number of  TUs radio is 'off' */
	int radio_disabled;		/* Whether the radio needs to be disabled now */
	uint	pwrsave_state;		/* radio pwr save state */
	int32	tbtt_skip;		/* num of tbtt to skip */
	bool	cncl_bcn;		/* whether to stop bcn or not in level 1 & 2. */
	struct wl_timer *timer;		/* timer to keep track of duty cycle */
	wlc_pwrsave_t pwrsave;
} wlc_radio_pwrsave_t;

#endif /* RADIO_PWRSAVE */

#ifdef RXCHAIN_PWRSAVE

#define RXCHAIN_PWRSAVE_ENAB_BRCM_NONHT	1
#define RXCHAIN_PWRSAVE_ENAB_ALL	2

#endif

#ifdef RADIO_PWRSAVE
#define RADIO_PWRSAVE_LOW 	0
#define RADIO_PWRSAVE_MEDIUM 	1
#define RADIO_PWRSAVE_HIGH 	2
#endif

/* Private AP data structure */
typedef struct
{
	struct wlc_ap_info	appub;		/* Public AP interface: MUST BE FIRST */
	wlc_info_t		*wlc;
	wlc_pub_t		*pub;
	bool			goto_longslot;	/* Goto long slot on next beacon */
	bool			wdsactive;	/* There are one or more WDS i/f(s) */
	uint			cs_scan_timer;	/* periodic auto channel scan timer (seconds) */
	uint			wds_timeout;	/* inactivity timeout for WDS links */
	uint32			maxassoc;	/* Max # associations to allow */
	bool			dfs_cac_enabled;	/* set if dfs cac enabled */
#ifdef RXCHAIN_PWRSAVE
	wlc_rxchain_pwrsave_t rxchain_pwrsave;  /* rxchain reduction power save structure */
#endif
#ifdef RADIO_PWRSAVE
	wlc_radio_pwrsave_t radio_pwrsave;   /* radio duty cycle power save structure */
#endif
#ifdef BAND5G
	struct wl_timer		*dfs_timer;	/* timer for dfs cac handler */
	wlc_dfs_cac_t		dfs_cac;	/* channel availability check */
#endif /* BAND5G */
} wlc_ap_info_pvt_t;

/* IOVar table */

/* Parameter IDs, for use only internally to wlc -- in the wlc_ap_iovars
 * table and by the wlc_ap_doiovar() function.  No ordering is imposed:
 * the table is keyed by name, and the function uses a switch.
 */
enum {
	IOV_AP_ISOLATE = 1,
	IOV_SCB_ACTIVITY_TIME,
	IOV_WDS_WPA_ROLE,
	IOV_AUTHE_STA_LIST,
	IOV_AUTHO_STA_LIST,
	IOV_WME_STA_LIST,
	IOV_WDSTIMEOUT,
	IOV_WME_AC_AP,
	IOV_BSS,
	IOV_MBSS,
	IOV_APCSCHSPEC,
	IOV_MAXASSOC,
	IOV_BSS_MAXASSOC,
	IOV_CLOSEDNET,
	IOV_AP,
	IOV_APSTA,		/* enable simultaneously active AP/STA */
	IOV_DFS_PREISM,		/* preism cac time */
	IOV_DFS_POSTISM,	/* postism cac time */
	IOV_DFS_STATUS,		/* dfs cac status */
	IOV_DFS_ISM_MONITOR,    /* control the behavior of ISM state */
	IOV_DFS_CHANNEL_FORCED, /* next dfs channel forced */
	IOV_DFS_CHANNEL_SAFE,   /* next dfs channel to none-dfs channel */
	IOV_TXBURST_LIM_OVERRIDE, /* set the advertised tx burst limit */
	IOV_CHANSPEC_SWITCH,	/* send CSA with chanspec as input */
	IOV_PREF_CHANSPEC,      /* User supplied chanspec, for when we're not using AutoChannel */
	IOV_BCN_ROTATE,	/* enable/disable beacon rotation */
	IOV_WDS_ENABLE,	/* enable/disable wds link events */
	IOV_AP_ASSERT,		/* User forced crash */
#ifdef RXCHAIN_PWRSAVE
	IOV_RXCHAIN_PWRSAVE_ENABLE,		/* Power Save with single rxchain enable */
	IOV_RXCHAIN_PWRSAVE_QUIET_TIME,		/* Power Save with single rxchain quiet time */
	IOV_RXCHAIN_PWRSAVE_PPS,		/* single rxchain packets per second */
	IOV_RXCHAIN_PWRSAVE,		/* Current power save mode */
	IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK,	/* Whether to check for associated stas */
#endif
#ifdef RADIO_PWRSAVE
	IOV_RADIO_PWRSAVE_ENABLE,		/* Radio duty cycle Power Save enable */
	IOV_RADIO_PWRSAVE_QUIET_TIME,		/* Radio duty cycle Power Save */
	IOV_RADIO_PWRSAVE_PPS,		/* Radio power save packets per second */
	IOV_RADIO_PWRSAVE,		/* Whether currently in power save or not */
	IOV_RADIO_PWRSAVE_LEVEL,		/* Radio power save duty cycle on time */
	IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK,	/* Whether to check for associated stas */
#endif
	IOV_AP_RESET,	/* User forced reset */
	IOV_BCMDCS, 	/* dynamic channel switch (management) */
	IOV_DYNBCN,	/* Dynamic beaconing */
	IOV_AP_TPC_MODE,	/* ap tpc mode */
	IOV_AP_TPC_PERIOD,	/* ap tpc periodicity */
	IOV_AP_TPC_LM,	/* ap tpc link margins */
	IOV_SCB_LASTUSED,	/* time (s) elapsed since any of the associated scb is used */
	IOV_SCB_PROBE,		/* get/set scb probe parameters */
	IOV_SCB_ASSOCED,	/* if it has associated SCBs at phy if level */
	IOV_ACS_UPDATE,		/* update after acs_scan and chanspec selection */
	IOV_LAST		/* In case of a need to check max ID number */

};

/* AP IO Vars */
static const bcm_iovar_t wlc_ap_iovars[] = {
	{"ap", IOV_AP,
	0, IOVT_INT32, 0
	},
	{"ap_isolate", IOV_AP_ISOLATE,
	(0), IOVT_BOOL, 0
	},
	{"scb_activity_time", IOV_SCB_ACTIVITY_TIME,
	(IOVF_NTRL), IOVT_UINT32, 0
	},
	{"wds_wpa_role", IOV_WDS_WPA_ROLE,
	(IOVF_SET_UP), IOVT_BUFFER, ETHER_ADDR_LEN+1
	},
	{"authe_sta_list", IOV_AUTHE_STA_LIST,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(uint)
	},
	{"autho_sta_list", IOV_AUTHO_STA_LIST,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(uint)
	},
	{"wme_sta_list", IOV_WME_STA_LIST,
	(0), IOVT_BUFFER, sizeof(uint)
	},
	{"maxassoc", IOV_MAXASSOC,
	(IOVF_WHL), IOVT_UINT32, 0
	},
	{"bss_maxassoc", IOV_BSS_MAXASSOC,
	(IOVF_NTRL), IOVT_UINT32, 0
	},
	{"wdstimeout", IOV_WDSTIMEOUT,
	(IOVF_WHL), IOVT_UINT32, 0
	},
	{"wme_ac_ap", IOV_WME_AC_AP,
	(0), IOVT_BUFFER,
	(AC_COUNT * sizeof(edcf_acparam_t))
	},
	{"bss", IOV_BSS,
	(0), IOVT_INT32, 0
	},
	{"closednet", IOV_CLOSEDNET,
	(0), IOVT_BOOL, 0
	},
	{"mbss", IOV_MBSS,
	(IOVF_SET_DOWN), IOVT_BOOL, 0,
	},
	{"apcschspec", IOV_APCSCHSPEC,
	(0), IOVT_UINT16, 0
	},
#ifdef RXCHAIN_PWRSAVE
	{"rxchain_pwrsave_enable", IOV_RXCHAIN_PWRSAVE_ENABLE,
	(0), IOVT_UINT8, 0
	},
	{"rxchain_pwrsave_quiet_time", IOV_RXCHAIN_PWRSAVE_QUIET_TIME,
	(0), IOVT_UINT32, 0
	},
	{"rxchain_pwrsave_pps", IOV_RXCHAIN_PWRSAVE_PPS,
	(0), IOVT_UINT32, 0
	},
	{"rxchain_pwrsave", IOV_RXCHAIN_PWRSAVE,
	(0), IOVT_UINT8, 0
	},
	{"rxchain_pwrsave_stas_assoc_check", IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK,
	(0), IOVT_UINT8, 0
	},
#endif /* RXCHAIN_PWRSAVE */
#ifdef RADIO_PWRSAVE
	{"radio_pwrsave_enable", IOV_RADIO_PWRSAVE_ENABLE,
	(0), IOVT_UINT8, 0
	},
	{"radio_pwrsave_quiet_time", IOV_RADIO_PWRSAVE_QUIET_TIME,
	(0), IOVT_UINT32, 0
	},
	{"radio_pwrsave_pps", IOV_RADIO_PWRSAVE_PPS,
	(0), IOVT_UINT32, 0
	},
	{"radio_pwrsave_level", IOV_RADIO_PWRSAVE_LEVEL,
	(0), IOVT_UINT8, 0
	},
	{"radio_pwrsave", IOV_RADIO_PWRSAVE,
	(0), IOVT_UINT8, 0
	},
	{"radio_pwrsave_stas_assoc_check", IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK,
	(0), IOVT_UINT8, 0
	},
#endif /* RADIO_PWRSAVE */
#if defined(STA)     /* APSTA */
	{"apsta", IOV_APSTA,
	(IOVF_SET_DOWN), IOVT_BOOL, 0,
	},
#endif /* APSTA */
#ifdef RADAR
	{"dfs_preism", IOV_DFS_PREISM,
	0, IOVT_UINT32, 0
	},
	{"dfs_postism", IOV_DFS_POSTISM,
	0, IOVT_UINT32, 0
	},
	{"dfs_status", IOV_DFS_STATUS,
	(0), IOVT_BUFFER, 0
	},
	{"dfs_ism_monitor", IOV_DFS_ISM_MONITOR,
	(0), IOVT_UINT32, 0
	},
	{"dfs_channel_forced", IOV_DFS_CHANNEL_FORCED,
	(0), IOVT_UINT32, 0
	},
	{"dfs_channel_safe", IOV_DFS_CHANNEL_SAFE,
	(0), IOVT_UINT32, 0
	},
#endif /* RADAR */
	{"csa", IOV_CHANSPEC_SWITCH,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(wl_chan_switch_t)
	},
	{"pref_chanspec", IOV_PREF_CHANSPEC,
	(0), IOVT_UINT16, 0
	},
	{"bcn_rotate", IOV_BCN_ROTATE,
	(0), IOVT_BOOL, 0
	},
	{"wds_enable", IOV_WDS_ENABLE,
	(IOVF_SET_UP), IOVT_BOOL, 0
	},
#ifdef BCM_DCS
	{"bcm_dcs", IOV_BCMDCS,
	(0), IOVT_BOOL, 0
	},
#endif /* BCM_DCS */
	{"dynbcn", IOV_DYNBCN,
	(0), IOVT_BOOL, 0
	},
#ifdef WL_AP_TPC
	{"tpc_mode", IOV_AP_TPC_MODE, (0), IOVT_UINT8, 0},
	{"tpc_period", IOV_AP_TPC_PERIOD, (0), IOVT_UINT8, 0},
	{"tpc_lm", IOV_AP_TPC_LM, (0), IOVT_UINT16, 0},
#endif
	{"scb_lastused", IOV_SCB_LASTUSED, (0), IOVT_UINT32, 0},
	{"scb_probe", IOV_SCB_PROBE,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(wl_scb_probe_t)
	},
	{"scb_assoced", IOV_SCB_ASSOCED, (0), IOVT_BOOL, 0},
	{"acs_update", IOV_ACS_UPDATE,
	(IOVF_SET_UP), IOVT_BOOL, 0
	},
	{NULL, 0, 0, 0, 0 }
};

/* Local Prototypes */
static int wlc_ap_watchdog(void *arg);
#ifndef WIN7
static void wlc_assoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlif);
static void wlc_reassoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlif);
#endif /* WIN7 */
static bool wlc_scb_scan(wlc_bsscfg_t *cfg, uint32 flagmask, uint32 flagvalue);
static void wlc_ap_sta_probe(wlc_ap_info_t *ap, struct scb *scb);
static void wlc_ap_sta_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb);
static void wlc_ap_stas_timeout(wlc_ap_info_t *ap);
#ifdef WDS
static int wlc_wds_wpa_role_get(wlc_ap_info_t *ap, struct ether_addr *ea, uint8 *role);
static int wlc_wds_create_link_event(wlc_ap_info_t *ap, struct scb *scb);
static void wlc_ap_wds_timeout(wlc_ap_info_t *ap);
#ifdef WLAFTERBURNER
static void wlc_update_ab_wds_timeout(wlc_ap_info_t *ap);
static bool wlc_nonabcap_assoc_upd(wlc_ap_info_t *ap);
#endif
#endif /* WDS */
static int wlc_authenticated_sta_check_cb(struct scb *scb);
static int wlc_authorized_sta_check_cb(struct scb *scb);
static int wlc_wme_sta_check_cb(struct scb *scb);
#ifdef WME
static void wlc_wme_initparams_ap(wlc_ap_info_t *ap, wme_param_ie_t *pe);
#endif /* WME */
static int wlc_sta_list_get(wlc_ap_info_t *ap, wlc_bsscfg_t *cfg, uint8 *buf,
                            int len, int (*sta_check)(struct scb *scb));
static uint8 wlc_lowest_basicrate_get(wlc_bsscfg_t *cfg);
#ifdef WDS
static void wlc_ap_wds_probe(wlc_ap_info_t *ap, struct scb *scb);
static void wlc_ap_wds_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb);
#endif /* WDS */
static int wlc_check_wpaie(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, uint8 *wpaie, uint16 *auth,
        uint32 *wsec);
#ifdef BCMDBG
static int wlc_dump_ap(wlc_ap_info_t *ap, struct bcmstrbuf *b);
#ifdef RXCHAIN_PWRSAVE
static int wlc_dump_rxchain_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b);
#endif
#ifdef RADIO_PWRSAVE
static int wlc_dump_radio_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b);
#endif
#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
static void wlc_dump_pwrsave(wlc_pwrsave_t *pwrsave, struct bcmstrbuf *b);
#endif
#endif /* BCMDBG */

#ifdef WL_AP_TPC
static int wlc_ap_tpc_setup(wlc_info_t *wlc, uint8 mode);
#endif

static int wlc_ap_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);

static int wlc_check_wpa2ie(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, bcm_tlv_t *wpa2ie,
                            struct scb *scb);

static void wlc_ap_up_upd(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, bool state);

static void wlc_send_action_switch_channel(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg);
static void wlc_send_action_switch_channel_ex(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id);
static void wlc_send_public_action_switch_channel(wlc_ap_info_pvt_t *appvt, wlc_bsscfg_t *bsscfg);
static void wlc_ap_acs_update(wlc_info_t *wlc);

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
#ifdef RXCHAIN_PWRSAVE
static void wlc_disable_pwrsave(wlc_ap_info_t *ap, int type);
#endif /* RXCHAIN_PWRSAVE */
static void wlc_reset_pwrsave_mode(wlc_ap_info_t *ap, int type);
static void wlc_pwrsave_mode_check(wlc_ap_info_t *ap, int type);
#endif /* RXCHAIN_PWRSAVE || RADIO_PWRSAVE */
#ifdef RADIO_PWRSAVE
#ifdef WL11H
static void wlc_radio_pwrsave_update_quiet_ie(wlc_ap_info_t *ap, uint8 count);
#endif
static void wlc_reset_radio_pwrsave_mode(wlc_ap_info_t *ap);
static void wlc_radio_pwrsave_off_time_start(wlc_ap_info_t *ap);
static void wlc_radio_pwrsave_timer(void *arg);
#endif /* RADIO_PWRSAVE */

#ifdef RADAR
static void wlc_dfs_cacstate_init(wlc_ap_info_t *ap);
static int wlc_dfs_timer_init(wlc_ap_info_t *ap);
static bool wlc_dfs_timer_delete(wlc_ap_info_t *ap);
static void wlc_dfs_chanspec_oos(wlc_ap_info_t *ap, chanspec_t chanspec);
static chanspec_t wlc_dfs_chanspec(wlc_ap_info_t *ap);
static bool wlc_valid_ap_chanspec(wlc_info_t *wlc, chanspec_t chspec);
static bool wlc_radar_detected(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_idle_set(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_ism_set(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_ooc_set(wlc_ap_info_t *ap, uint target_state);
static void wlc_dfs_cacstate_idle(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_cac(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_ism(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_csa(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_ooc(wlc_ap_info_t *ap);
static void wlc_dfs_cacstate_handler(void *arg);

/* Local Data Structures */
static void (*wlc_dfs_cacstate_fn_ary[WL_DFS_CACSTATES])(wlc_ap_info_t *ap) = {
	wlc_dfs_cacstate_idle,
	wlc_dfs_cacstate_cac, /* preism_cac */
	wlc_dfs_cacstate_ism,
	wlc_dfs_cacstate_csa,
	wlc_dfs_cacstate_cac, /* postism_cac */
	wlc_dfs_cacstate_ooc, /* preism_ooc */
	wlc_dfs_cacstate_ooc /* postism_ooc */
};

#if defined(BCMDBG) || defined(WLTEST)
static const char *wlc_dfs_cacstate_str[WL_DFS_CACSTATES] = {
	"IDLE", "PRE-ISM Channel Availability Check", "In-Service Monitoring(ISM)",
	"Channel Switching Announcement(CSA)",
	"POST-ISM Channel Availability Check",
	"PRE-ISM Out Of Channels(OOC)", "POSTISM Out Of Channels(OOC)"
};
#endif /* BCMDBG || WLTEST */
#endif /* RADAR */

#ifdef MBSS
/* MBSS16 */
static void wlc_ap_mbss16_write_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static void wlc_mbss16_setup(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
#endif /* MBSS */

#ifdef WIN7
#define SCB_ASSOC_CUBBY(ap, scb) (assoc_decision_t*)SCB_CUBBY((scb), (ap)->scb_handle)
static void wlc_ap_scb_free_notify(void *context, struct scb *scb);
#endif /* WIN7 */

wlc_ap_info_t*
BCMATTACHFN(wlc_ap_attach)(wlc_pub_t *pub, wlc_info_t *wlc)
{
	wlc_ap_info_pvt_t *appvt;
	wlc_ap_info_t* ap;

	wlc->scb_timeout = SCB_TIMEOUT;
	wlc->scb_activity_time = SCB_ACTIVITY_TIME;
	wlc->scb_max_probe = SCB_GRACE_ATTEMPTS;

	appvt = (wlc_ap_info_pvt_t*)MALLOC(pub->osh, sizeof(wlc_ap_info_pvt_t));
	if (appvt == NULL) {
		WL_ERROR(("wlc_ap_attach: MALLOC wlc_ap_info_pvt_t failed\n"));
		return NULL;
	}

	bzero(appvt, sizeof(wlc_ap_info_pvt_t));
	ap = &appvt->appub;

	appvt->wlc = wlc;
	appvt->pub = pub;
	ap->shortslot_restrict = FALSE;
	appvt->maxassoc = wlc->pub->tunables->maxscb;

	if (wlc_apps_attach(wlc)) {
		WL_ERROR(("wlc_ap_attach: wlc_apps_attach failed\n"));
		MFREE(pub->osh, appvt, sizeof(wlc_ap_info_pvt_t));
		return NULL;
	}

#ifdef RADAR
	wlc_dfs_timer_init(ap);
#endif /* RADAR */

#ifdef WME
	wlc_wme_initparams_ap(ap, &ap->wme_param_ie_ap);
#endif /* WME */
#ifdef RXCHAIN_PWRSAVE
	wlc_reset_rxchain_pwrsave_mode(ap);
#ifdef BCMDBG
	wlc_dump_register(pub, "rxchain_pwrsave", (dump_fn_t)wlc_dump_rxchain_pwrsave, (void *)ap);
#endif
#endif /* RXCHAIN_PWRSAVE */
#ifdef RADIO_PWRSAVE
	if (!(appvt->radio_pwrsave.timer =
		wl_init_timer(wlc->wl, wlc_radio_pwrsave_timer, ap, "radio_pwrsave"))) {
		WL_ERROR(("wlc_ap_attach: wl_init_timer for radio powersave timer failed\n"));
		if (appvt->radio_pwrsave.timer)
			wl_free_timer(wlc->wl, appvt->radio_pwrsave.timer);
		return NULL;
	}
	wlc_reset_radio_pwrsave_mode(ap);
#ifdef BCMDBG
	wlc_dump_register(pub, "radio_pwrsave", (dump_fn_t)wlc_dump_radio_pwrsave, (void *)ap);
#endif
#endif /* BCMDBG */
#ifdef BCMDBG
	wlc_dump_register(pub, "ap", (dump_fn_t)wlc_dump_ap, (void *)ap);
#endif
	wlc_module_register(pub, wlc_ap_iovars, "wlc_ap_iovars", appvt,
		wlc_ap_doiovar, wlc_ap_watchdog, NULL, NULL);

#ifdef WIN7
	/* reserve cubby in the scb container for per-scb private data */
	ap->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(assoc_decision_t),
		NULL, wlc_ap_scb_free_notify, NULL, appvt);

	if (ap->scb_handle < 0) {
		WL_ERROR(("wlc_ap_attach: wlc_scb_cubby_reserve failed\n"));
		MFREE(pub->osh, appvt, sizeof(wlc_ap_info_pvt_t));
		return NULL;
	}
#endif /* WIN7 */

	return (wlc_ap_info_t*)appvt;
}

void
BCMATTACHFN(wlc_ap_detach)(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;

	wlc_apps_detach(wlc);

#ifdef RADAR
	if (appvt->dfs_timer) {
		wlc_dfs_timer_delete(ap); /* TBD */
		wl_free_timer(wlc->wl, appvt->dfs_timer);
		appvt->dfs_timer = NULL;
	}
#endif /* RADAR */

#ifdef RADIO_PWRSAVE
	wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
	wl_free_timer(wlc->wl, appvt->radio_pwrsave.timer);
	appvt->radio_pwrsave.timer = NULL;
#endif

	wlc_module_unregister(pub, "wlc_ap_iovars", appvt);

	MFREE(pub->osh, appvt, sizeof(wlc_ap_info_pvt_t));
}

void
wlc_ap_up(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*)ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	uint channel;
	wlcband_t *band;
	wlc_bss_info_t *target_bss = bsscfg->target_bss;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	/* update radio parameters from default_bss */
	/* adopt the default BSS parameters as our BSS */
	/* adopt the default BSS params as the target's BSS params */
	bcopy(wlc->default_bss, target_bss, sizeof(wlc_bss_info_t));

	/* set some values to be appropriate for AP operation */
	target_bss->infra = 1;
	target_bss->atim_window = 0;
	target_bss->capability = DOT11_CAP_ESS;

	bcopy((char*)&bsscfg->cur_etheraddr, (char*)&target_bss->BSSID, ETHER_ADDR_LEN);


#ifdef WLMCHAN
	/* Force ap/go to follow sta channel if mchan stago is disabled */
	if (MCHAN_ENAB(wlc->pub) && pub->associated && wlc_mchan_stago_is_disabled(wlc->mchan)) {
		bsscfg->chanspec = wlc->home_chanspec;
	}

	if (MCHAN_ENAB(wlc->pub) && bsscfg->chanspec) {
		target_bss->chanspec = bsscfg->chanspec;
	} else
#endif /* WLMCHAN */
	if (pub->associated) {
		WL_INFORM(("wl%d: share wlc->home_chanspec 0x%04x in BSS %s\n", pub->unit,
		          wlc->home_chanspec, bcm_ether_ntoa(&target_bss->BSSID, eabuf)));
		target_bss->chanspec = wlc->home_chanspec;
		return;
	}

#ifdef RADAR
	if (WL11H_AP_ENAB(wlc) && CHSPEC_IS5G(target_bss->chanspec) &&
	    !(BSSCFG_SRADAR_ENAB(bsscfg) || BSSCFG_AP_NORADAR_CHAN_ENAB(bsscfg))) {
		/* if chanspec_next is uninitialized, e.g. softAP with
		 * DFS_TPC country, pick a random one like wlc_restart_ap()
		 */
		if (appvt->dfs_cac.chanspec_next == 0)
			appvt->dfs_cac.chanspec_next = wlc_dfs_chanspec(ap);

		target_bss->chanspec = appvt->dfs_cac.chanspec_next;
#ifdef WLMCHAN
		if (MCHAN_ENAB(wlc->pub) && bsscfg->chanspec) {
			bsscfg->chanspec = target_bss->chanspec;
		}
#endif /* WLMCHAN */

	}
#endif /* RADAR */

	/* validate or fixup default channel value */
	if (!wlc_valid_chanspec_db(wlc->cmi, target_bss->chanspec) ||
	    wlc_restricted_chanspec(wlc->cmi, target_bss->chanspec)) {
		chanspec_t chspec = wlc_default_chanspec(wlc->cmi, FALSE);
		if ((chspec == INVCHANSPEC) ||
		    wlc_restricted_chanspec(wlc->cmi, chspec)) {
			WL_ERROR(("wl%d: cannot create BSS on channel %u\n",
			          pub->unit,
			          CHSPEC_CHANNEL(target_bss->chanspec)));
			bsscfg->up = FALSE;
			return;
		}
		target_bss->chanspec = chspec;
#ifdef WLMCHAN
		if (MCHAN_ENAB(wlc->pub) && bsscfg->chanspec) {
			bsscfg->chanspec = target_bss->chanspec;
		}
#endif /* WLMCHAN */
	}
	/* Validate the channel bandwidth */
	band = wlc->bandstate[CHSPEC_IS2G(target_bss->chanspec) ? BAND_2G_INDEX : BAND_5G_INDEX];
	if (CHSPEC_IS40(target_bss->chanspec) &&
	    (!N_ENAB(pub) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_40MHZ) ||
	     !band->mimo_cap_40)) {
		channel = wf_chspec_ctlchan(target_bss->chanspec);
		target_bss->chanspec = CH20MHZ_CHSPEC(channel);
#ifdef WLMCHAN
		if (MCHAN_ENAB(wlc->pub) && bsscfg->chanspec) {
			bsscfg->chanspec = target_bss->chanspec;
		}
#endif /* WLMCHAN */
	}
}

int
wlc_ap_down(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	int callback = 0; /* Need to fix this timer callback propagation; error prone right now */
	struct scb_iter scbiter;
	struct scb *scb;
	int assoc_scb_count = 0;

#ifdef RADAR
	/* cancel the radar timer */
	if (appvt->dfs_timer) {
		if (!wlc_dfs_timer_delete(ap))
			callback = 1;
	}
	appvt->dfs_cac_enabled = FALSE;
#endif /* RADAR */

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		wlc_p2p_bss_upd(wlc->p2p, bsscfg, FALSE);
	else
		wlc_p2p_ap_bss_upd(wlc->p2p, bsscfg, FALSE);
#endif

	if (WIN7_OS(wlc->pub)) {
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (scb->bsscfg == bsscfg && SCB_ASSOCIATED(scb)) {
				/* indicate sta leaving */
				wlc_bss_mac_event(wlc, bsscfg, WLC_E_DISASSOC_IND, &scb->ea,
					WLC_E_STATUS_SUCCESS, DOT11_RC_DISASSOC_LEAVING, 0, 0, 0);
						assoc_scb_count++;
				if (!scb->permanent)
					wlc_scbfree(wlc, scb);
			}
		}

		if (wlc->pub->up && assoc_scb_count)
			/* send broadcast disassoc request to all stas */
			wlc_senddisassoc(wlc, &ether_bcast,
				&bsscfg->cur_etheraddr, &bsscfg->BSSID,
				WLC_BCMCSCB_GET(wlc, bsscfg), DOT11_RC_DISASSOC_LEAVING);
	}

	/* adjust associated state(s) accordingly */
	wlc_ap_up_upd(ap, bsscfg, FALSE);

	WL_APSTA_UPDN(("Reporting link down on config %d (AP disabled)\n",
		WLC_BSSCFG_IDX(bsscfg)));

	wlc_link(wlc, FALSE, &bsscfg->cur_etheraddr, bsscfg, WLC_E_LINK_BSSCFG_DIS);


#ifdef RADIO_PWRSAVE
	wlc_reset_radio_pwrsave_mode(ap);
#endif
#if defined(WL_AP_TPC)
	/* clear the link margins when going down. */
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->bsscfg == bsscfg && SCB_ASSOCIATED(scb)) {
			scb->sta_link_margin = AP_TPC_MAX_LINK_MARGIN;
			scb->ap_link_margin = AP_TPC_MAX_LINK_MARGIN;
		}
	}
#endif

	/* Clear the BSSID */
	bzero(bsscfg->BSSID.octet, ETHER_ADDR_LEN);




	return callback;
}

#ifdef WME
/* Initialize a WME Parameter Info Element with default AP parameters from WMM Spec, Table 14 */
static void
BCMATTACHFN(wlc_wme_initparams_ap)(wlc_ap_info_t *ap, wme_param_ie_t *pe)
{
	static const wme_param_ie_t apdef = {
		WME_OUI,
		WME_TYPE,
		WME_SUBTYPE_PARAM_IE,
		WME_VER,
		0,
		0,
		{
			{ EDCF_AC_BE_ACI_AP, EDCF_AC_BE_ECW_AP, HTOL16(EDCF_AC_BE_TXOP_AP) },
			{ EDCF_AC_BK_ACI_AP, EDCF_AC_BK_ECW_AP, HTOL16(EDCF_AC_BK_TXOP_AP) },
			{ EDCF_AC_VI_ACI_AP, EDCF_AC_VI_ECW_AP, HTOL16(EDCF_AC_VI_TXOP_AP) },
			{ EDCF_AC_VO_ACI_AP, EDCF_AC_VO_ECW_AP, HTOL16(EDCF_AC_VO_TXOP_AP) }
		}
	};

	ASSERT(sizeof(*pe) == WME_PARAM_IE_LEN);
	memcpy(pe, &apdef, sizeof(*pe));
}
#endif /* WME */

#ifdef WDS
#ifdef WLAFTERBURNER
static void
wlc_update_ab_wds_timeout(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
	wlc_prot_cfg_t *prot;
	wlc_prot_to_t *to;

	if (!BSSCFG_AP(cfg))
		return;

	prot = cfg->prot_cfg;
	to = cfg->prot_to;

	/*
	 * Get our AB counter up to date:
	 * if we aren't AB capable:
	 *   ensure our count is at the max.
	 * else if there aren't any active wds links:
	 *   ensure our count is at the min.
	 * else:
	 *   decrement the count.
	 *
	 * Note that we're not allowed to set afterburner_override to ON or
	 * AUTO if the hardware itself is not AB capable hence no need to
	 * check our underlying hw capability.
	 *
	 *
	 */
	if (prot->erp_ie_nonerp || to->nonerp_ovlp_timeout ||
	    wlc->protection->nonabcap_assoc) {
		wlc->ab_wds_timeout = AB_WDS_TIMEOUT_MAX;
	} else if (appvt->wdsactive == FALSE) {
		wlc->ab_wds_timeout = AB_WDS_TIMEOUT_MIN;
	}
	else {
		/* Decrement ab counter, make sure we don't underflow. */
		wlc->ab_wds_timeout--;
		wlc->ab_wds_timeout = MAX(AB_WDS_TIMEOUT_MIN, wlc->ab_wds_timeout);
	}

	/* Be explicit:
	 * If afterburner_override has been selected to be ON or OFF
	 * enforce that now on the ab_wds_timeout.
	 *
	 */
	if (wlc->afterburner_override == OFF)
		wlc->ab_wds_timeout = AB_WDS_TIMEOUT_MAX;
	else if (wlc->afterburner_override == ON)
		wlc->ab_wds_timeout = AB_WDS_TIMEOUT_MIN;

	ASSERT((AB_WDS_TIMEOUT_MIN <= wlc->ab_wds_timeout) &&
	       (wlc->ab_wds_timeout <= AB_WDS_TIMEOUT_MAX));

}
#endif /* WLAFTERBURNER */

/*
 * Determine who is WPA supplicant and who is WPA authenticator over a WDS link.
 * The one that has the lower MAC address in numeric value is supplicant (802.11i D5.0).
 */
int
wlc_wds_wpa_role_set(wlc_ap_info_t *ap, struct scb *scb, uint8 role)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_pub_t* pub = appvt->pub;

	switch (role) {
	/* auto, based on mac address value, lower is supplicant */
	case WL_WDS_WPA_ROLE_AUTO:
		if (bcmp(pub->cur_etheraddr.octet, scb->ea.octet, ETHER_ADDR_LEN) > 0)
			scb->flags |= SCB_WPA_SUP;
		else
			scb->flags &= ~SCB_WPA_SUP;
		break;
	/* local is supplicant, remote is authenticator */
	case WL_WDS_WPA_ROLE_SUP:
		scb->flags &= ~SCB_WPA_SUP;
		break;
	/* local is authenticator, remote is supplicant */
	case WL_WDS_WPA_ROLE_AUTH:
		scb->flags |= SCB_WPA_SUP;
		break;
	/* invalid roles */
	default:
		WL_ERROR(("wl%d: invalid WPA role %u\n", pub->unit, role));
		return BCME_BADARG;
	}
	return 0;
}


/*
 * Set 'role' to WL_WDS_WPA_ROLE_AUTH if the remote end of the WDS link identified by
 * the given mac address is WPA supplicant; set 'role' to WL_WDS_WPA_ROLE_SUP otherwise.
 */
static int
wlc_wds_wpa_role_get(wlc_ap_info_t *ap,  struct ether_addr *ea, uint8 *role)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
	struct scb *scb;

	pub = appvt->pub;

	if (!(scb = wlc_scbfind(wlc, ea))) {
		WL_ERROR(("wl%d: failed to find SCB for %02x:%02x:%02x:%02x:%02x:%02x\n",
			pub->unit, ea->octet[0], ea->octet[1],
			ea->octet[2], ea->octet[3], ea->octet[4], ea->octet[5]));
		return BCME_NOTFOUND;
	}
	*role = SCB_WDS(scb) && (scb->flags & SCB_WPA_SUP) ?
		WL_WDS_WPA_ROLE_AUTH : WL_WDS_WPA_ROLE_SUP;
	return 0;
}

static int
wlc_wds_create_link_event(wlc_ap_info_t *ap, struct scb *scb)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_event_t *e;

	/* create WDS LINK event */
	e = wlc_event_alloc(wlc->eventq);
	if (e == NULL) {
		WL_ERROR(("wl%d: wlc_wds_create wlc_event_alloc failed\n", wlc->pub->unit));
		return BCME_NOMEM;
	}

	e->event.event_type = WLC_E_LINK;
	e->event.flags = WLC_EVENT_MSG_LINK;

	wlc_event_if(wlc, SCB_BSSCFG(scb), e, &scb->ea);

	wlc_eventq_enq(wlc->eventq, e);

	return 0;
}
#endif /* WDS */

static int
wlc_authenticated_sta_check_cb(struct scb *scb)
{
	return SCB_AUTHENTICATED(scb);
}

static int
wlc_authorized_sta_check_cb(struct scb *scb)
{
	return SCB_AUTHORIZED(scb);
}

static int
wlc_wme_sta_check_cb(struct scb *scb)
{
	return SCB_WME(scb);
}

/* Returns a maclist of all scbs that pass the provided check function.
 * The buf is formatted as a struct maclist on return, and may be unaligned.
 * buf must be at least 4 bytes long to hold the maclist->count value.
 * If the maclist is too long for the supplied buffer, BCME_BUFTOOSHORT is returned
 * and the maclist->count val is set to the number of MAC addrs that would
 * have been returned. This allows the caller to allocate the needed space and
 * call again.
 */
static int
wlc_sta_list_get(wlc_ap_info_t *ap, wlc_bsscfg_t *cfg, uint8 *buf,
                 int len, int (*sta_check_cb)(struct scb *scb))
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	int err = 0;
	uint c = 0;
	uint8 *dst;
	struct scb *scb;
	struct scb_iter scbiter;
	ASSERT(len >= (int)sizeof(uint));

	/* make room for the maclist count */
	dst = buf + sizeof(uint);
	len -= sizeof(uint);
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (sta_check_cb(scb) && (cfg == scb->bsscfg)) {
			c++;
			if (len >= ETHER_ADDR_LEN) {
				bcopy(scb->ea.octet, dst, ETHER_ADDR_LEN);
				dst += sizeof(struct ether_addr);
				len -= sizeof(struct ether_addr);
			} else {
				err = BCME_BUFTOOSHORT;
			}
		}
	}

	/* copy the actual count even if the buffer is too short */
	bcopy(&c, buf, sizeof(uint));

	return err;
}

int
wlc_wpa_set_cap(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, uint8 *cap, int len)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pub_t *pub;
	int ret_val = -1;

	pub = appvt->pub;
	ASSERT(len >= WPA_CAP_LEN);

	if (len < WPA_CAP_LEN)
		return ret_val;

	if (BSSCFG_AP(bsscfg)) {
		if (cap[0] & WPA_CAP_WPA2_PREAUTH)
			bsscfg->wpa2_preauth = TRUE;
		else
			bsscfg->wpa2_preauth = FALSE;
		WL_INFORM(("wl%d: wlc_wpa_set_cap: setting the wpa2 preauth %d\n", pub->unit,
			bsscfg->wpa2_preauth));
		ret_val = 0;
	}
	return ret_val;
}

void
wlc_read_pmq_ea(wlc_ap_info_t *ap,  struct ether_addr *eaddr)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc;
	wlc_pub_t *pub;
	uint32 pat_hi, pat_lo;

	wlc = appvt->wlc;
	pub = appvt->pub;

	pat_lo = R_REG(pub->osh, &wlc->regs->pmqpatl);
	pat_hi = R_REG(pub->osh, &wlc->regs->pmqpath);
	eaddr->octet[5] = (pat_hi >> 8)  & 0xff;
	eaddr->octet[4] =  pat_hi	& 0xff;
	eaddr->octet[3] = (pat_lo >> 24) & 0xff;
	eaddr->octet[2] = (pat_lo >> 16) & 0xff;
	eaddr->octet[1] = (pat_lo >> 8)  & 0xff;
	eaddr->octet[0] =  pat_lo	& 0xff;
}

static void
wlc_send_action_switch_channel_ex(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	void *p;
	uint8* pbody;
	uint8* cp;
	uint body_len;
	struct dot11_action_frmhdr *action_hdr;
	bool ext_ie = FALSE;
	uint8 *bufend;

	/* Action switch_channel */
	body_len = DOT11_ACTION_HDR_LEN + TLV_HDR_LEN;
	if (action_id == DOT11_SM_ACTION_CHANNEL_SWITCH) {
		body_len += DOT11_SWITCH_IE_LEN;
		/* account for extension channel IE if operate in 40MHz */
		if (N_ENAB(appvt->pub) && (CHSPEC_IS40(csa->chspec))) {
				body_len += (TLV_HDR_LEN + DOT11_EXTCH_IE_LEN);
				ext_ie = TRUE;
		}
	} else
		body_len += DOT11_EXT_CSA_IE_LEN;

	if ((p = wlc_frame_get_mgmt(wlc, FC_ACTION, dst, &bsscfg->cur_etheraddr,
	                            &bsscfg->BSSID, body_len, &pbody)) == NULL) {
	        return;
	}

	/* mark the end of buffer */
	bufend = pbody + body_len;

	action_hdr = (struct dot11_action_frmhdr *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = action_id;

	if (action_id == DOT11_SM_ACTION_CHANNEL_SWITCH) {
		cp = wlc_write_csa_ie(csa, action_hdr->data, (body_len - DOT11_ACTION_HDR_LEN));
		if (ext_ie)
		    cp = wlc_write_extch_ie(csa->chspec, cp, BUFLEN(cp, bufend));
	} else
		cp = wlc_write_ext_csa_ie(csa, action_hdr->data);

	ASSERT((cp - pbody) == (int)body_len);

	WL_COEX(("wl%d: %s: Send CSA (id=%d) Action frame\n",
		appvt->pub->unit, __FUNCTION__, action_id));
	wlc_sendmgmt(wlc, p, bsscfg->wlcif->qi, NULL);
}

static void
wlc_send_public_action_switch_channel(wlc_ap_info_pvt_t *appvt, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = appvt->wlc;
	void *p;
	uint8* pbody;
	uint8* cp;
	uint body_len;
	struct dot11_action_frmhdr *action_hdr;

	/* Action switch_channel */
	body_len = sizeof(struct dot11y_action_ext_csa);

	if ((p = wlc_frame_get_action(wlc, FC_ACTION, &ether_bcast, &bsscfg->cur_etheraddr,
		&bsscfg->BSSID, body_len, &pbody, DOT11_ACTION_CAT_PUBLIC)) == NULL) {
		return;
	}

	action_hdr = (struct dot11_action_frmhdr *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_PUBLIC;
	action_hdr->action = DOT11_PUB_ACTION_CHANNEL_SWITCH;
	cp = wlc_write_csa_body(&bsscfg->csa->csa, action_hdr->data);

	ASSERT((cp - pbody) == (int)body_len);

	WL_COEX(("wl%d: %s: Send CSA Public Action frame\n", appvt->pub->unit, __FUNCTION__));
	wlc_sendmgmt(wlc, p, bsscfg->wlcif->qi, NULL);
}

static void
wlc_send_action_switch_channel(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*)ap;

	ASSERT(appvt);
	wlc_send_action_switch_channel_ex(ap, bsscfg, &ether_bcast,
		&bsscfg->csa->csa, DOT11_SM_ACTION_CHANNEL_SWITCH);
	if (N_ENAB(appvt->pub))
		wlc_send_action_switch_channel_ex(ap, bsscfg, &ether_bcast,
			&bsscfg->csa->csa, DOT11_SM_ACTION_EXT_CSA);
	wlc_send_public_action_switch_channel(appvt, bsscfg);
}

/* AP processes WME Setup Request Management frame */
/* Algorithm for accept/reject is still in flux. For now, reject all requests */
void
wlc_wme_setup_req(wlc_ap_info_t *ap, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
	void *outp;
	uint8* pbody;
	struct dot11_management_notification* mgmt_hdr;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif
	wlc_bsscfg_t *cfg = wlc->cfg;

	pub = appvt->pub;

	if ((outp = wlc_frame_get_action(wlc, FC_ACTION, &hdr->sa, &cfg->cur_etheraddr,
	        &cfg->BSSID, body_len, &pbody, DOT11_ACTION_CAT_PUBLIC)) == NULL) {
		WL_ERROR(("wl%d: failed to get WME response packet \n",
			pub->unit));
		return;
	}

	/*
	 *  duplicate incoming packet and change
	 *  the few necessary fields
	 */
	bcopy((char *)body, (char *)pbody, body_len);
	mgmt_hdr = (struct dot11_management_notification *)pbody;
	mgmt_hdr->action = WME_ADDTS_RESPONSE;

	/* for now, reject all setup requests */
	mgmt_hdr->status = WME_ADMISSION_REFUSED;
	WL_INFORM(("Sending WME_SETUP_RESPONSE, refusal to %s\n", sa));
	wlc_sendmgmt(wlc, outp, cfg->wlcif->qi, NULL);
}

/* return TRUE if there are any associated STAs with matching flag value, otherwise FALSE */
static bool
wlc_scb_scan(wlc_bsscfg_t *cfg, uint32 flagmask, uint32 flagvalue)
{
	wlc_info_t *wlc = cfg->wlc;
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_ASSOCIATED(scb) && SCB_BSSCFG(scb) == cfg &&
		    ((scb->flags & flagmask) == flagvalue))
			return (TRUE);
	}

	return (FALSE);
}

static void
wlc_ap_stas_timeout(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	struct scb *scb;
	struct scb_iter scbiter;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		wlc_bsscfg_t *cfg = SCB_BSSCFG(scb);

		/* Don't age out the permanent SCB and AP SCB */
		if (scb->permanent ||
		    cfg == NULL || !BSSCFG_AP(cfg))
			continue;

		/* kill any other band scb */
		if (TRUE &&
#ifdef WLMCHAN
		    !MCHAN_ENAB(wlc->pub) &&
#endif
		    wlc_scbband(scb) != wlc->band) {
			wlc_scbfree(wlc, scb);
			continue;
		}

		/* probe associated stas if idle for scb_activity_time or reprobe them */
		if (SCB_ASSOCIATED(scb) &&
		    (((pub->now - scb->used) >= wlc->scb_activity_time) ||
		     (wlc->reprobe_scb && scb->grace_attempts))) {
#ifdef BCMDBG
			WL_INFORM(("wlc_ap_sta_timeout: probing %s [attempt %d] (diff %d)\n",
			          bcm_ether_ntoa(&scb->ea, eabuf),
			          scb->grace_attempts, (pub->now - scb->used)));
#endif

			wlc_ap_sta_probe(ap, scb);
		}

		/* Authenticated but idle for long time free it now */
		//		if ((scb->state == AUTHENTICATED) &&
		//			((pub->now - scb->used) >= SCB_LONG_TIMEOUT))
		/* tenda add, fixed don't release offline station hqw add 同步掉线不释放问题*/
		if (pub->now - scb->used >= 600)
		{

			wlc_scbfree(wlc, scb);
			continue;
		}
	}
}

#ifdef WDS
static void
wlc_ap_wds_timeout(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	struct scb *scb;
	struct scb_iter scbiter;

	/* check wds link connectivity */
	if (!wlc->reprobe_wds &&
	    ((appvt->wdsactive && appvt->wds_timeout &&
	     ((pub->now % appvt->wds_timeout) == 0)) != TRUE))
		return;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_WDS(scb)) {
#ifdef WLBTAMP
			if (BSS_BTA_ENAB(wlc, SCB_BSSCFG(scb)))
				continue;
#endif
			/* mark the WDS link up if we have had recent traffic,
			 * or probe the WDS link if we have not.
			 */
			if ((pub->now - scb->used) >= wlc->scb_activity_time ||
			    !(scb->flags & SCB_WDS_LINKUP) || wlc->reprobe_wds)
				wlc_ap_wds_probe(ap, scb);
		}
	}
}

#ifdef WLAFTERBURNER
/*
 * This routine is used to set/clean nonabcap_assoc variable
 */
static bool
wlc_nonabcap_assoc_upd(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	struct scb *scb;
	struct scb_iter scbiter;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		/* any no-AB STA join */
		if (SCB_ASSOCIATED(scb) && !(scb->flags & SCB_ABCAP))
			return TRUE;
	}

	return (FALSE);
}
#endif /* WLAFTERBURNER */
#endif /* WDS */

uint
wlc_ap_stas_associated(wlc_ap_info_t *ap)
{
	wlc_info_t *wlc = ((wlc_ap_info_pvt_t *)ap)->wlc;
	int i;
	wlc_bsscfg_t *bsscfg;
	uint count = 0;

	FOREACH_UP_AP(wlc, i, bsscfg)
		count += wlc_bss_assocscb_getcnt(wlc, bsscfg);

	return count;
}

#if defined(MBSS) && defined(WLC_HIGH) && defined(WLC_LOW)

/* ******* Probe Request Fifo Handling: Generate Software Probe Responses ******* */

/*
 * Convert raw PRQ entry to info structure
 * Returns error if bsscfg not found in wlc structure
 */

static int
prq_entry_convert(wlc_info_t *wlc, shm_mbss_prq_entry_t *in, wlc_prq_info_t *out)
{
	int uc_idx, sw_idx;

	bzero(out, sizeof(*out));
	bcopy(in, &out->source, sizeof(shm_mbss_prq_entry_t));
	if (ETHER_ISNULLADDR(&out->source.ta)) {
		WL_ERROR(("prq_entry_convert: PRQ Entry for Transmitter Address is NULL\n"));
		return -1;
	}

	out->directed_ssid = SHM_MBSS_PRQ_ENT_DIR_SSID(in);
	out->directed_bssid = SHM_MBSS_PRQ_ENT_DIR_BSSID(in);
	out->is_directed = (out->directed_ssid || out->directed_bssid);
	out->frame_type = SHM_MBSS_PRQ_ENT_FRAMETYPE(in);
	out->up_band = SHM_MBSS_PRQ_ENT_UPBAND(in);
	out->plcp0 = SHM_MBSS_PRQ_ENT_PLCP0(in);
#ifdef WLLPRS
	out->is_htsta = SHM_MBSS_PRQ_ENT_HTSTA(in);
#endif /* WLLPRS */

	if (out->is_directed) {
		uc_idx = SHM_MBSS_PRQ_ENT_UC_BSS_IDX(in);
		sw_idx = WLC_BSSCFG_HW2SW_IDX(wlc, uc_idx);
		if (sw_idx < 0) {
			return -1;
		}
		out->bsscfg = wlc->bsscfg[sw_idx];
		ASSERT(out->bsscfg != NULL);
	}

	return 0;
}

#if defined(BCMDBG)
static void
prq_entry_dump(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint16 rd_ptr, shm_mbss_prq_entry_t *entry)
{
	uint8 *ptr;
	char ssidbuf[SSID_FMT_BUF_LEN];

	ptr = (uint8 *)entry;
	if (rd_ptr != 0) {
		WL_MBSS(("Dump of raw PRQ entry from offset 0x%x (word offset 0x%x)\n",
			rd_ptr * 2, rd_ptr));
	} else {
		WL_MBSS(("    Dump of raw PRQ entry\n"));
	}
	WL_MBSS(("    %02x%02x %02x%02x %02x%02x %02x%02x %04x\n",
		ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5],
		entry->prq_info[0], entry->prq_info[1], entry->time_stamp));
	WL_MBSS(("    %sdirected SSID. %sdirected BSSID. uc_idx: %d. type %d. upband %d.\n",
		SHM_MBSS_PRQ_ENT_DIR_SSID(entry) ? "" : "not ",
		SHM_MBSS_PRQ_ENT_DIR_BSSID(entry) ? "" : "not ",
		SHM_MBSS_PRQ_ENT_UC_BSS_IDX(entry),
		SHM_MBSS_PRQ_ENT_FRAMETYPE(entry),
		SHM_MBSS_PRQ_ENT_UPBAND(entry)));
	if (bsscfg != NULL) {
		wlc_format_ssid(ssidbuf, bsscfg->SSID, bsscfg->SSID_len);
		WL_MBSS(("    Entry mapped to bss %d, ssid %s\n", WLC_BSSCFG_IDX(bsscfg), ssidbuf));
	}
}
static void
prq_info_dump(wlc_info_t *wlc, wlc_prq_info_t *info)
{
	WL_MBSS(("Dump of PRQ info: dir %d. dir ssid %d. dir bss %d. bss cfg idx %d\n",
		info->is_directed, info->directed_ssid, info->directed_bssid,
		WLC_BSSCFG_IDX(info->bsscfg)));
	WL_MBSS(("    frame type %d, up_band %d, plcp0 0x%x\n",
		info->frame_type, info->up_band, info->plcp0));
	prq_entry_dump(wlc, info->bsscfg, 0, &info->source);
}
#else
#define prq_entry_dump(a, b, c, d)
#define prq_info_dump(a, b)
#endif /* BCMDBG */

/*
 * PRQ FIFO Processing
 */

static void
prb_pkt_final_setup(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_prq_info_t *info,
	uint8 *pkt_start, int len)
{
	d11txh_t *txh;
	uint8 *plcp_hdr;
	uint8 *d11_hdr;
	uint8 *d11_da;
	int plcp_len;
	uint32 exptime;
	uint16 exptime_low;
	uint16 mcl;

	txh = (d11txh_t *)pkt_start;
	plcp_hdr = &pkt_start[D11_TXH_LEN];
	d11_hdr = &pkt_start[D11_TXH_LEN + D11_PHY_HDR_LEN];
	d11_da = &d11_hdr[2 * sizeof(uint16)]; /* skip fc and duration/id */

	/* Set up the PHY header */
	plcp_len = len - D11_TXH_LEN - D11_PHY_HDR_LEN + DOT11_FCS_LEN;
	wlc_prb_resp_plcp_hdrs(wlc, info, plcp_len, plcp_hdr, txh, d11_hdr);

	if (bsscfg->prb_ttl_us > 0) { /* Set the packet expiry time */
		exptime = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
		exptime_low = (uint16)exptime;
		exptime = (exptime & 0xffff0000) | info->source.time_stamp;
		if (exptime_low < info->source.time_stamp) { /* Rollover occurred */
			exptime -= 0x10000; /* Decrement upper 16 bits. */
		}
		exptime += bsscfg->prb_ttl_us;

		txh->TstampLow = htol16(exptime & 0xffff);
		txh->TstampHigh = htol16((exptime >> 16) & 0xffff);
		mcl = ltoh16(txh->MacTxControlLow);
		mcl |= TXC_LIFETIME;
		txh->MacTxControlLow = htol16(mcl);
	}

	/* Set up the dest addr */
	bcopy(&info->source.ta, d11_da, sizeof(struct ether_addr));
}

#if defined(MBSS) && defined(WLLPRS)
/* This function does a selective copy of the PRQ template to the actual probe resp
 * packet. The idea is to intelligently form the "best" probe resp based on the
 * prq_info passed in by uCODE, based on parsing of the probe request. This is a change
 * to the existing "universal" probe response model.
 */
static void
wlc_prq_resp_form(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_prq_info_t *info,
                  uint8 *pkt_start, int *len)
{
	wlc_pkt_t template;

	template = bsscfg->probe_template;

	if (template == NULL)
		return;

	/* We need to change from the template */
	if (!info->is_htsta && PRB_HTIE(bsscfg).present) {
		uint8 *src_ptr = PKTDATA(wlc->osh, template);
		uint8 *dst_ptr = pkt_start;
		int copy_len = D11_PHY_HDR_LEN + D11_TXH_LEN + DOT11_MGMT_HDR_LEN +
		               PRB_HTIE(bsscfg).offset;

		/* Exclude ANA HT IEs while copying */
		bcopy(src_ptr, dst_ptr, copy_len);

		*len -= PRB_HTIE(bsscfg).length;
		src_ptr = PKTDATA(wlc->osh, template) + copy_len +
		          PRB_HTIE(bsscfg).length;
		dst_ptr = pkt_start + copy_len;
		copy_len = *len - copy_len;

		bcopy(src_ptr, dst_ptr, copy_len);
	} else
		bcopy(PKTDATA(wlc->osh, template), pkt_start, *len);

	return;
}
#endif /* MBSS  && WLLPRS */

/* Respond to the given PRQ entry on the given bss cfg */
static void
wlc_prq_directed(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_prq_info_t *info)
{
	wlc_pkt_t pkt, template;
	uint8 *pkt_start;
	int len;

	template = bsscfg->probe_template;
	if (template == NULL) {
		return;
	}
	len = PKTLEN(wlc->osh, template);

	/* Allocate a new pkt for DMA; copy from template; set up hdrs */
	pkt = PKTGET(wlc->osh, BCN_TMPL_LEN, TRUE);
	if (pkt == NULL) {
		WLCNTINCR(bsscfg->cnt->prb_resp_alloc_fail);
		return;
	}
	/* Template includes TX header, PLCP header and D11 header */
	pkt_start = PKTDATA(wlc->osh, pkt);
#if defined(MBSS) && defined(WLLPRS)
	if (N_ENAB(wlc->pub))
		wlc_prq_resp_form(wlc, bsscfg, info, pkt_start, &len);
#else /* MBSS && WLLPRS */
	bcopy(PKTDATA(wlc->osh, template), pkt_start, len);
#endif /* MBSS && WLLPRS */
	PKTSETLEN(wlc->osh, pkt, len);
	WLPKTTAGBSSCFGSET(pkt, WLC_BSSCFG_IDX(bsscfg));

	prb_pkt_final_setup(wlc, bsscfg, info, pkt_start, len);

	if (dma_txfast(wlc->hw->di[TX_ATIM_FIFO], pkt, TRUE) < 0) {
		WL_MBSS(("Failed to transmit probe resp for bss %d\n", WLC_BSSCFG_IDX(bsscfg)));
		WLCNTINCR(bsscfg->cnt->prb_resp_tx_fail);
	}
}

/* Process a PRQ entry, whether broadcast or directed, generating probe response(s) */

/* Is the given config up and responding to probe responses in SW? */
#define CFG_SOFT_PRB_RESP(cfg) \
	(((cfg) != NULL) && ((cfg)->up) && ((cfg)->enable) && SOFTPRB_ENAB(cfg))

static void
wlc_prq_response(wlc_info_t *wlc, wlc_prq_info_t *info)
{
	int idx;
	wlc_bsscfg_t *cfg;
	static int bcast_next_start = 0; /* For rotating probe responses to bcast requests */

#ifdef WLPROBRESP_SW
	return;
#endif

	if (info->is_directed) {
		ASSERT(info->bsscfg != NULL);
		wlc_prq_directed(wlc, info->bsscfg, info);
	} else { /* Broadcast probe response */
		for (idx = 0; idx < WLC_MAXBSSCFG; idx++) {
			cfg = wlc->bsscfg[(idx + bcast_next_start) % WLC_MAXBSSCFG];
			if (CFG_SOFT_PRB_RESP(cfg) && !cfg->closednet_nobcprbresp) {
				wlc_prq_directed(wlc, cfg, info);
			}
		}
		/* Move "next start" up to next BSS skipping inactive BSSes */

		for (idx = 0; idx < WLC_MAXBSSCFG; idx++) {
			if (++bcast_next_start == WLC_MAXBSSCFG) {
				bcast_next_start = 0;
			}
			if (CFG_SOFT_PRB_RESP(wlc->bsscfg[bcast_next_start])) {
				break;
			}
		}
	}
}

/*
 * Process the PRQ Fifo.
 * Note that read and write pointers are (uint16 *) in the ucode
 * Return TRUE if more entries to process.
 */

/*
 * After some investigation, it looks like there's never more than 1 PRQ
 * entry to be serviced at a time.  So the bound here is probably inconsequential.
 */
#define PRQBND 5

bool
wlc_prq_process(wlc_info_t *wlc, bool bounded)
{
	uint16 rd_ptr, wr_ptr, prq_base, prq_top;
	shm_mbss_prq_entry_t entry;
	wlc_prq_info_t info;
	int count = 0;
	bool rv = FALSE;  /* Default, no more to be done */
	bool set_rd_ptr = FALSE;

	if (!MBSS_ENAB(wlc->pub)) {
		return FALSE;
	}

	prq_base = wlc->prq_base;
	prq_top = prq_base + (SHM_MBSS_PRQ_TOT_BYTES / 2);

	rd_ptr = wlc->prq_rd_ptr;
	wr_ptr = wlc_read_shm(wlc, SHM_MBSS_PRQ_WRITE_PTR);

#if defined(BCMDBG)
	/* Debug checks for rd and wr ptrs */
	if (wr_ptr < prq_base || wr_ptr >= prq_top) {
		WL_ERROR(("Error: PRQ fifo write pointer 0x%x out of bounds (%d, %d)\n",
			wr_ptr, prq_base, prq_top));
		return FALSE;
	}
	if (rd_ptr < prq_base || rd_ptr >= prq_top) {
		WL_ERROR(("Error: PRQ read pointer 0x%x out of bounds; clearing fifo\n", rd_ptr));
		/* Reset read pointer to write pointer, emptying the fifo */
		rd_ptr = wr_ptr;
		set_rd_ptr = TRUE;
	}
#endif /* BCMDBG */

	while (rd_ptr != wr_ptr) {
		WLCNTINCR(wlc->pub->_cnt->prq_entries_handled);
		set_rd_ptr = TRUE;

		/* Copy entry from PRQ; convert and respond; update rd ptr */
		wlc_copyfrom_shm(wlc, rd_ptr * 2, &entry, sizeof(entry));
		if (prq_entry_convert(wlc, &entry, &info) < 0) {
			WL_ERROR(("Error reading prq fifo at offset 0x%x\n", rd_ptr));
			prq_entry_dump(wlc, NULL, rd_ptr, &entry);
			WLCNTINCR(wlc->pub->_cnt->prq_bad_entries);
		} else if (info.is_directed && !(info.bsscfg->up)) { /* Ignore rqst */
			WL_MBSS(("MBSS: Received PRQ entry on down BSS (%d)\n",
				WLC_BSSCFG_IDX(info.bsscfg)));
		} else {
#if defined(BCMDBG)
			if (info.is_directed && info.bsscfg != NULL) {
				if (0) { /* Extra dump for directed requests */
					prq_info_dump(wlc, &info);
				}
				WLCNTINCR(info.bsscfg->cnt->prq_directed_entries);
			} else {
				WLCNTINCR(wlc->pub->_cnt->prq_undirected_entries);
			}
#endif
			wlc_prq_response(wlc, &info);
		}

		/* Update the read pointer */
		rd_ptr += sizeof(entry) / 2;
		if (rd_ptr >= prq_top) {
			rd_ptr = prq_base;
		}

		if (bounded && (count++ >= PRQBND)) {
			rv = TRUE; /* Possibly more to do */
			break;
		}
	}

	if (set_rd_ptr) { /* Write the value back when done processing */
		wlc_write_shm(wlc, SHM_MBSS_PRQ_READ_PTR, rd_ptr);
		wlc->prq_rd_ptr = rd_ptr;
	}

	return rv;
}
#endif /* MBSS && WLC_HIGH && WLC_LOW */

void
wlc_ap_authresp(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, struct dot11_management_header *hdr,
	void *body, uint len, bool short_preamble, bool dec_attempt)
{
#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_BTA)
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
	struct dot11_auth *auth = (struct dot11_auth *) body;
	uint8 *frame_data = (uint8 *)body;
	struct scb *scb = NULL;
	uint16 auth_alg, auth_seq;
	uint16 status = DOT11_SC_SUCCESS;
	uint8 *challenge = NULL;
	int i;
	int key_index;
	uint16 fc;
	wsec_key_t *key;
	uint            nmac;           /* # of entries on maclist array */
	int             macmode;        /* allow/deny stations on maclist array */
	struct ether_addr* maclist;     /* pointer to the list of source MAC addrs to match */

	pub = appvt->pub;

	WL_TRACE(("wl%d: wlc_authresp_ap\n", pub->unit));

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		WL_ASSOC(("wl%d: AP Scan in progress, abort auth\n",
			pub->unit));
		status = SMFS_CODE_IGNORED;
		goto smf_stats;
	}

	/* Only handle Auth requests when the bsscfg is operational */
	if (BSSCFG_AP(bsscfg) && !bsscfg->up) {
		WL_ASSOC(("wl%d: BSS is not operational, abort auth\n", pub->unit));
		status = SMFS_CODE_IGNORED;
		goto smf_stats;
	}

	fc = ltoh16(hdr->fc);

	auth_alg = ltoh16(auth->alg);
	auth_seq = ltoh16(auth->seq);

	/* Fixup auth alg and seq for encrypted frames assuming this is a
	 * Shared Key challenge response
	 */
	if (fc & FC_WEP) {
		auth_alg = DOT11_SHARED_KEY;
		auth_seq = 3;
	}

#ifdef BAND5G
	/* Reject auth & assoc while  performing a channel switch */
	if (wlc->block_datafifo & DATA_BLOCK_QUIET) {
		WL_REGULATORY(("wl%d: wlc_authresp_ap: Authentication denied while in radar"
		               " avoidance mode\n", pub->unit));
		status = SMFS_CODE_IGNORED;
		goto send_result;
	}
#endif /* BAND5G */

	ASSERT(BSSCFG_AP(bsscfg));

	if (fc & FC_WEP) {
		/* Check if it is logical to have an encrypted packet here */
		if ((scb = wlc_scbfind(wlc, (struct ether_addr *) &hdr->sa)) == NULL) {
			WL_ERROR(("wl%d: wlc_authresp_ap: auth resp frame encrypted"
				  "from unknown STA %s\n", pub->unit, sa));
			status = SMFS_CODE_MALFORMED;
			goto smf_stats;
		}
		if (scb->challenge == NULL) {
			WL_ERROR(("wl%d: wlc_authresp_ap: auth resp frame encrypted "
				  "with no challenge recorded from %s\n", pub->unit, sa));
			status = SMFS_CODE_MALFORMED;
			goto smf_stats;
		}

		/* Processing a WEP encrypted AUTH frame:
		 * BSS config0 is allowed to use the HW default keys, all other BSS configs require
		 * software decryption of AUTH frames.  For simpler code:
		 *
		 * If the frame has been decrypted(a default HW key is present at the right index),
		 * always re-encrypt the frame with the key used by HW and then use the BSS config
		 * specific WEP key to decrypt. This means that all WEP encrypted AUTH frames will
		 * be decrypted in software.
		 */

		WL_ASSOC(("wl%d: %s: received wep from %sassociated scb\n",
		          pub->unit, __FUNCTION__, SCB_ASSOCIATED(scb) ? "" : "non-"));

		status = DOT11_SC_AUTH_CHALLENGE_FAIL;
		key_index = frame_data[3] >> DOT11_KEY_INDEX_SHIFT;

		if (dec_attempt) {
			uint8 key_data[16];
			rc4_ks_t ks;

			WL_ASSOC(("wl%d: %s: received wep decrypted frame\n",
			          pub->unit, __FUNCTION__));

			/* Use the default key in hardware to re-encrypt! */
			key = WSEC_KEY(wlc, key_index);
			/* RXS_DECATMPT flag will be set regardless of whether
			 * the key is valid.  So, if we don't really have a key
			 * there or the key's algo is "0" then the frame
			 * wasn't really decrypted by HW.
			 */
			if (key && (key->algo != 0)) {
				bcopy(frame_data, key_data, 3);
				bcopy(key->data, &key_data[3], key->len);

				/* prepare key */
				prepare_key(key_data, key->len + 3, &ks);
				rc4(frame_data+4, len - 4, &ks);
			}
		}

		/* At this point the frame is encrypted. Attempt decrypting
		 * using the bsscfg key.  Then compare the decrypted challenge
		 * text
		 */
		key = WLC_BSSCFG_DEFKEY(bsscfg, key_index);
		if (key) {
			if (wep_decrypt(len, frame_data, key->len, key->data)) {
				WL_ASSOC(("wl%d: %s: ICV pass : %s: BSSCFG = %d\n",
				          pub->unit, __FUNCTION__, sa,
				          WLC_BSSCFG_IDX(bsscfg)));
			} else {
				WL_ASSOC(("wl%d: %s: ICV fail : %s: BSSCFG = %d\n",
				          pub->unit, __FUNCTION__, sa,
				          WLC_BSSCFG_IDX(bsscfg)));
				goto clean_and_send;
			}
		}
		else {
			WL_ASSOC(("wl%d: %s: no key for WEP Auth Challenge from %s\n",
			          pub->unit, __FUNCTION__, sa));
			goto clean_and_send;
		}

		/* test if valid */
		auth = (struct dot11_auth *)(frame_data + 4);
		auth_alg = ltoh16(auth->alg);
		auth_seq = ltoh16(auth->seq);
		challenge = frame_data + sizeof(struct dot11_auth) + 4;
		if (auth_alg == DOT11_SHARED_KEY && auth_seq == 3 &&
		    challenge[0] == DOT11_MNG_CHALLENGE_ID &&
		    !bcmp(&scb->challenge[2], &challenge[2], challenge[1])) {
			wlc_scb_setstatebit_bsscfg(scb, AUTHENTICATED, WLC_BSSCFG_IDX(bsscfg));
			WL_ASSOC(("wl%d: wlc_authresp_ap: WEP Auth Challenge"
			          " success from : %s\n",
			          pub->unit, sa));
			scb->auth_alg = DOT11_SHARED_KEY;
			status = DOT11_SC_SUCCESS;
			wlc_scb_setstatebit_bsscfg(scb, AUTHENTICATED, 0);
		}
		else {
			wlc_scb_clearstatebit_bsscfg(scb, AUTHENTICATED, WLC_BSSCFG_IDX(bsscfg));
			WL_ERROR(("wl%d: wlc_authresp_ap: WEP Auth Challenge failure from %s\n",
			          pub->unit, sa));
		}

	clean_and_send:
		/* free the challenge text */
		MFREE(pub->osh, scb->challenge, 2 + scb->challenge[1]);
		scb->challenge = challenge = NULL;
		/* we are done with this frame */
		goto send_result;
	}


	switch (auth_seq) {
	case 1:
		macmode = bsscfg->macmode;
		nmac = (int)bsscfg->nmac;
		maclist = bsscfg->maclist;

		if (macmode != WLC_MACMODE_DISABLED) {
			/* check the source MAC list */
			for (i = 0; i < (int)nmac; i++) {
				if (bcmp((void*)&hdr->sa, (void*)&maclist[i], ETHER_ADDR_LEN)
					== 0)
				break;
			}
			/* toss if source MAC is on the deny list or not on the allow list */
			if ((macmode == WLC_MACMODE_DENY && i < (int)nmac) ||
			    (macmode == WLC_MACMODE_ALLOW && i >= (int)nmac)) {
				WL_ASSOC(("wl%d.%d mac restrict: %s %s\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
					macmode == WLC_MACMODE_DENY ? "Denying" : "Not allowing",
					sa));
				status = DOT11_SC_FAILURE;
				break;
			}
			WL_ASSOC(("wl%d.%d mac restrict: %s %s\n", pub->unit,
				WLC_BSSCFG_IDX(bsscfg),
				macmode == WLC_MACMODE_DENY ? "Not denying" : "Allowing", sa));
		}

		for (i = 0; i < (int)NBANDS(wlc); i++) {
			/* Use band 1 for single band 11a */
			if (IS_SINGLEBAND_5G(wlc->deviceid))
				i = BAND_5G_INDEX;

			scb = wlc_scbfindband(wlc, (struct ether_addr *)&hdr->sa, i);
			if (scb) {
				WL_ASSOC(("wl%d: wlc_authresp_ap: scb for the STA-%s"
					" already exists\n", pub->unit, sa));
#ifdef WLBTAMP
				if ((scb->bsscfg == bsscfg) && BSS_BTA_ENAB(wlc, bsscfg)) {
					WL_BTA(("wl%d: wlc_authresp_ap: preserving pre-existing "
						"BT-AMP peer %s\n", pub->unit, sa));
					goto btamp_sanity;
				}
#endif /* WLBTAMP */
				wlc_scbfree(wlc, scb);
			}
		}

		/* allocate an scb */
		if (!(scb = wlc_scblookup(wlc, (struct ether_addr *) &hdr->sa))) {
			WL_ERROR(("wl%d: wlc_authresp_ap: out of scbs for %s\n", pub->unit,
				sa));
			status = DOT11_SC_FAILURE;
			break;
		}

		wlc_scb_set_bsscfg(scb, bsscfg);

		if (scb->flags & SCB_MYAP) {
			if (APSTA_ENAB(pub)) {
				WL_APSTA(("wl%d: Reject AUTH request from AP %s\n", pub->unit, sa));
				status = DOT11_SC_FAILURE;
				break;
			}
			scb->flags &= ~SCB_MYAP;
		}

		wlc_scb_disassoc_cleanup(wlc, scb);

#ifdef WLBTAMP
	btamp_sanity:
#endif /* WLBTAMP */

		/* auth_alg is coming from the STA, not us */
		switch (auth_alg) {
		case DOT11_OPEN_SYSTEM:

			if ((WLC_BSSCFG_AUTH(bsscfg) == DOT11_OPEN_SYSTEM) && (!status)) {
				wlc_scb_setstatebit_bsscfg(scb, AUTHENTICATED,
				                           WLC_BSSCFG_IDX(bsscfg));
			}
			else {
				wlc_scb_clearstatebit_bsscfg(scb, AUTHENTICATED,
				                             WLC_BSSCFG_IDX(bsscfg));
			}

			/* At this point, we should have at least one valid authentication in open
			 * system
			 */
			if (!(SCB_AUTHENTICATED(scb))) {
				WL_ERROR(("wl%d: wlc_authresp_ap: Open System auth attempted "
					  "from %s but only Shared Key supported\n",
					  pub->unit, sa));
				status = DOT11_SC_AUTH_MISMATCH;
			}
			break;
		case DOT11_SHARED_KEY:
			if (scb->challenge) {
				MFREE(pub->osh, scb->challenge, 2 + scb->challenge[1]);
				scb->challenge = NULL;
			}
			if (bsscfg->WPA_auth == WPA_AUTH_DISABLED) {
				/* create the challenge text */
				if ((challenge = MALLOC(pub->osh, 2 + DOT11_CHALLENGE_LEN))) {
					challenge[0] = DOT11_MNG_CHALLENGE_ID;
					challenge[1] = DOT11_CHALLENGE_LEN;
					for (i = 0; i < DOT11_CHALLENGE_LEN; i++)
						challenge[i+2] = (uint8)(R_REG(pub->osh,
							&wlc->regs->tsf_random) & 0xFF);
					scb->challenge = challenge;
				} else {
					WL_ERROR(("wl%d: wlc_authresp_ap: out of memory,"
						" malloced %d bytes\n",
						pub->unit, MALLOCED(pub->osh)));
					status = DOT11_SC_FAILURE;
				}
				break;
			}
			/* fall through if configuration mismatch */
		default:
			WL_ERROR(("wl%d: wlc_authresp_ap: unhandled algorithm %d from %s\n",
				pub->unit, auth_alg, sa));
			status = DOT11_SC_AUTH_MISMATCH;
			break;
		}
		break;


	default:

		WL_ERROR(("wl%d: wlc_authresp_ap: unexpected authentication sequence %d from %s\n",
			pub->unit, auth_seq, sa));
		status = DOT11_SC_AUTH_SEQ;
		break;
	}

send_result:
	/* if sequence number would be out of range, do nothing */
	if (auth_seq >= 4) {
		status = SMFS_CODE_MALFORMED;
		goto smf_stats;
	}

#ifndef WIN7	    /* Win7 indicates auth event when recv assoc request */
	/* send authentication response */
	if (status == DOT11_SC_SUCCESS && scb && SCB_AUTHENTICATED(scb)) {
		WL_ASSOC(("wl%d.%d wlc_authresp_ap: %s authenticated\n", pub->unit,
			WLC_BSSCFG_IDX(bsscfg), sa));
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_AUTH_IND, &hdr->sa, WLC_E_STATUS_SUCCESS,
			DOT11_SC_SUCCESS, auth_alg, 0, 0);
	}
#endif /* WIN7 */

#ifdef RXCHAIN_PWRSAVE
	/* fast switch back from rxchain_pwrsave state upon authentication */
	if ((status == DOT11_SC_SUCCESS) && scb && SCB_AUTHENTICATED(scb)) {
		wlc_reset_rxchain_pwrsave_mode(ap);
	}
#endif /* RXCHAIN_PWRSAVE */

	wlc_sendauth(bsscfg, &hdr->sa, &bsscfg->BSSID, scb,
		auth_alg, auth_seq + 1, status, NULL, challenge, short_preamble);

smf_stats:
	wlc_smfstats_update(wlc, bsscfg, SMFS_TYPE_AUTH, status);
}

/* Return non-zero if cipher is enabled. */
static int
wpa_cipher_enabled(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, int cipher)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pub_t *pub;
	int ret = 0;
	uint32 wsec = bsscfg->wsec;

	pub = appvt->pub;

	switch (cipher) {
	case WPA_CIPHER_NONE:
		ret = !WSEC_ENABLED(wsec);
		break;
	case WPA_CIPHER_WEP_40:
	case WPA_CIPHER_WEP_104:
		ret = WSEC_WEP_ENABLED(wsec);
		if (ret && bsscfg->tkip_countermeasures) {
			WL_WSEC(("wl%d: TKIP countermeasures in effect\n", pub->unit));
			ret = 0;
		}
		break;
	case WPA_CIPHER_TKIP:
		ret = WSEC_TKIP_ENABLED(wsec);
		if (ret && bsscfg->tkip_countermeasures) {
			WL_WSEC(("wl%d: TKIP countermeasures in effect\n", pub->unit));
			ret = 0;
		}
		break;
	case WPA_CIPHER_AES_CCM:
		ret = WSEC_AES_ENABLED(wsec);
		break;
	case WPA_CIPHER_AES_OCB:
	default:
		ret = 0;
		break;
	}
	return ret;
}

#define WPA_OUI_OK(oui) (bcmp(WPA_OUI, (oui), 3) == 0)

static int
wlc_check_wpaie(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, uint8 *wpaie, uint16 *auth, uint32 *wsec)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pub_t *pub;
	uint8 ie_len = wpaie[1];
	uint16 ver;
	int n_suites, offset;
	uint32 WPA_auth = bsscfg->WPA_auth;

	pub = appvt->pub;

	if (ie_len < WPA_IE_TAG_FIXED_LEN) {
		WL_ERROR(("wl%d: WPA IE illegally short\n", pub->unit));
		return 1;
	}
	ver = wpaie[6];
	if ((ver != WPA_VERSION) || !WPA_OUI_OK(wpaie+2)) {
		WL_ERROR(("wl%d: unexpected WPA IE version %d or OUI %02x:%02x:%02x\n",
			pub->unit, ver, wpaie[2], wpaie[3], wpaie[4]));
		return 1;
	}
	/* All the rest is optional.  Test the length to see what's there. */
	if (ie_len < 10) {
		/* Group suite (and all the rest) defaulted. */
		if (!(WPA_auth & WPA_AUTH_UNSPECIFIED) ||
		    (!wpa_cipher_enabled(ap, bsscfg, WPA_CIPHER_TKIP))) {
			WL_ERROR(("wl%d: WPA IE defaults not enabled\n",
				pub->unit));
			return 1;
		}
		*auth = WPA_AUTH_UNSPECIFIED;
		*wsec = TKIP_ENABLED;
		return 0;
	}
	/* There's a group suite, so check it. */
	if (!WPA_OUI_OK(wpaie+8) || !wpa_cipher_enabled(ap, bsscfg, wpaie[11])) {
		WL_ERROR(("wl%d: WPA group cipher %02x:%02x:%02x:%d not enabled\n",
			pub->unit, wpaie[8], wpaie[9], wpaie[10], wpaie[11]));
		return 1;
	}

	/* Update the potential crypto being used for the SCB */
	bcmwpa_cipher2wsec(&wpaie[8], wsec);

	/* Is enough IE left for a pairwise suite? */
	if (ie_len >= 16) {
		/* Check that *the* pairwise suite is enabled. */
		n_suites = wpaie[12];
		offset = 14;
		if (!WPA_OUI_OK(wpaie+offset) ||
		    !wpa_cipher_enabled(ap, bsscfg, wpaie[offset+3]) || n_suites != 1) {
			WL_ERROR(("wl%d: bad pairwise suite in WPA IE.\n",
				pub->unit));
			return 1;
		}
		bcmwpa_cipher2wsec(&wpaie[offset], wsec);
	}
	/* Is enough IE left for a key management suite?
	 * (Remember IE length doesn't include first 2 bytes of the IE.)
	 */
	if (ie_len < 22) {
		/* It's defaulted.  Check that we default, too. */
		if (!(WPA_auth & WPA_AUTH_UNSPECIFIED)) {
			WL_ERROR(("wl%d: default WPA IE auth mode not enabled\n",
				pub->unit));
			return 1;
		}
		*auth = WPA_AUTH_UNSPECIFIED;
	} else {
		n_suites = wpaie[18];
		offset = 20;
		if ((n_suites != 1) ||
		    !(WPA_OUI_OK(wpaie+offset) &&
		      (((wpaie[offset+3] == RSN_AKM_UNSPECIFIED) &&
			(WPA_auth & WPA_AUTH_UNSPECIFIED)) ||
		       ((wpaie[offset+3] == RSN_AKM_PSK) &&
			(WPA_auth & WPA_AUTH_PSK))))) {
			WL_ERROR(("wl%d: bad key management suite in WPA IE.\n",
				pub->unit));
			return 1;
		}
		if (!bcmwpa_akm2WPAauth(&wpaie[offset], &WPA_auth, FALSE)) {
			WL_ERROR(("wl%d: bcmwpa_akm2WPAauth: can't convert AKM %02x%02x%02x%02x.\n",
				pub->unit, wpaie[offset], wpaie[offset + 1],
				wpaie[offset + 2], wpaie[offset + 3]));
			return 1;
		}
		*auth = (uint16)WPA_auth;
	}
	/* Reach this only if the IE looked okay.
	 * Note that capability bits of the IE have no use here yet.
	 */
	return 0;
}

#undef WPA_OUI_OK

static int
wlc_check_wpa2ie(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, bcm_tlv_t *wpa2ie,
	struct scb *scb)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pub_t *pub;
	uint8 len = wpa2ie->len;
	uint32 WPA_auth = bsscfg->WPA_auth;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint16 count;
#ifdef MFP
	uint8 *cap;
	scb->flags2 &= ~(SCB2_MFP | SCB2_SHA256);
#endif

	pub = appvt->pub;

	/* Check min length and version */
	if ((len < WPA2_VERSION_LEN) ||
	    (ltoh16_ua(wpa2ie->data) != WPA2_VERSION)) {
		WL_ERROR(("wl%d: unsupported WPA2 version %d\n", pub->unit,
			ltoh16_ua(wpa2ie->data)));
		return 1;
	}
	len -= WPA2_VERSION_LEN;

	/* All the rest is optional.  Test the length to see what's there. */
	if (len < WPA_SUITE_LEN) {
		/* Group suite (and all the rest) defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED) ||
		    !wpa_cipher_enabled(ap, bsscfg, WPA_CIPHER_AES_CCM)) {
			WL_ERROR(("wl%d: WPA2 IE defaults not enabled\n", pub->unit));
			return 1;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		scb->wsec = AES_ENABLED;
		return 0;
	}

	/* There's an mcast cipher, so check it. */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	if (bcmp(mcast->oui, WPA2_OUI, DOT11_OUI_LEN) ||
		!wpa_cipher_enabled(ap, bsscfg, mcast->type)) {
		WL_ERROR(("wl%d: WPA2 mcast cipher %02x:%02x:%02x:%d not enabled\n",
			pub->unit, mcast->oui[0], mcast->oui[1], mcast->oui[2],
			mcast->type));
		return 1;
	}
	len -= WPA_SUITE_LEN;

	/* Update the potential crypto being used for the SCB */
	bcmwpa_cipher2wsec(mcast->oui, &scb->wsec);

	/* Again, the rest is optional.  Test the length to see what's there. */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		/* ucast cipher and AKM defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED) ||
		    !wpa_cipher_enabled(ap, bsscfg, WPA_CIPHER_AES_CCM)) {
			WL_ERROR(("wl%d: WPA2 IE defaults not enabled\n", pub->unit));
			return 1;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		scb->wsec = AES_ENABLED;
		return 0;
	}

	/* Check the unicast cipher */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	if (count != 1 ||
		bcmp(ucast->list[0].oui, WPA2_OUI, DOT11_OUI_LEN) ||
		!wpa_cipher_enabled(ap, bsscfg, ucast->list[0].type)) {
		WL_ERROR(("wl%d: bad pairwise suite in WPA2 IE.\n", pub->unit));
		return 1;
	}
	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);
	bcmwpa_cipher2wsec(ucast->list[0].oui, &scb->wsec);

	/* Again, the rest is optional, so test len to see what's there */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		/* AKM defaulted. */
		if (!(WPA_auth & WPA2_AUTH_UNSPECIFIED)) {
			WL_ERROR(("wl%d: default WPA2 IE auth mode not enabled\n",
				pub->unit));
			return 1;
		}
		scb->WPA_auth = WPA2_AUTH_UNSPECIFIED;
		return 0;
	}

	/* Check the AKM */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[1];
	count = ltoh16_ua(&mgmt->count);
	if ((count != 1) ||
	    !((bcmp(mgmt->list[0].oui, WPA2_OUI, DOT11_OUI_LEN) == 0) &&
	      (((mgmt->list[0].type == RSN_AKM_UNSPECIFIED) &&
		(WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
#ifdef MFP
	       ((mgmt->list[0].type == RSN_AKM_MFP_1X) &&
		(WPA_auth & RSN_AKM_UNSPECIFIED)) ||
	       ((mgmt->list[0].type == RSN_AKM_MFP_PSK) &&
		(WPA_auth & WPA2_AUTH_PSK)) ||
#endif
	       ((mgmt->list[0].type == RSN_AKM_PSK) &&
		(WPA_auth & WPA2_AUTH_PSK))))) {
		WL_ERROR(("wl%d: bad AKM in WPA2 IE.\n", pub->unit));
		return 1;
	}
#ifdef MFP
	if ((mgmt->list[0].type == RSN_AKM_MFP_1X) || (mgmt->list[0].type == RSN_AKM_MFP_PSK)) {
		scb->flags2 |= SCB2_SHA256;
	}
	if ((bsscfg->wsec & MFP_REQUIRED) && !SCB_SHA256(scb)) {
		WL_ERROR(("wl%d: MFP requred but peer selects SHA-1, reject\n", pub->unit));
		return 1;
	}
#endif
	if (!bcmwpa_akm2WPAauth((uint8 *)&mgmt->list[0], &WPA_auth, FALSE)) {
		WL_ERROR(("wl%d: bcmwpa_akm2WPAauth: can't convert AKM %02x%02x%02x%02x.\n",
			pub->unit, mgmt->list[0].oui[0], mgmt->list[0].oui[1],
			mgmt->list[0].oui[2], mgmt->list[0].type));
		return 1;
	}
	scb->WPA_auth = (uint16)WPA_auth;

#ifdef MFP
	len -= (WPA_IE_SUITE_COUNT_LEN + WPA_SUITE_LEN);
	if (len < RSN_CAP_LEN) {
		if (bsscfg->wsec & MFP_REQUIRED) {
			WL_ERROR(("wl%d: requested sta MFP capabilities not exist\n",
				pub->unit));
			return 1;
		}
		else {
			WL_ERROR(("wl%d: no MFP cap involved\n",
				pub->unit));
			return 0;
		}
	}
	/* pick up MFP flags. */
	cap = (uint8 *)&mgmt->list[count];
	WL_ERROR(("wl: %s: peer advertised capabilities of 0x%02x 0x%02x\n",
		__FUNCTION__, cap[1], cap[0]));
	/* reject invalid setting */
	if ((cap[0] & RSN_CAP_MFPR) && !(cap[0] & RSN_CAP_MFPC)) {
		WL_ERROR(("wl%d: invalid sta MFP setting\n",
			pub->unit));
		return 1;
	}
	if (SCB_SHA256(scb) && !(cap[0] & RSN_CAP_MFPC)) {
		WL_ERROR(("wl%d: invalid sta MFP setting\n",
			pub->unit));
		return 1;
	}
	WL_ERROR(("wl: %s: AP flags 0x%x\n", __FUNCTION__, bsscfg->wsec));
	/* see Table 8-1a of 11w for details */
	if ((bsscfg->wsec & MFP_CAPABLE) && (cap[0] & RSN_CAP_MFPC)) {
		if (SCB_SHA256(scb)) {
			scb->flags2 |= SCB2_MFP;
			WL_ERROR(("wl%d: turn sta MFP setting on\n",
				pub->unit));
		} else {
			WL_ERROR(("wl%d: peer is MFP capable, but sha-1, reject\n",
				pub->unit));
			return 1;
		}
	}
	else if ((bsscfg->wsec & MFP_REQUIRED) && !(cap[0] & RSN_CAP_MFPC)) {
		WL_ERROR(("wl%d: requested sta MFP cap not exist\n",
			pub->unit));
		return 1;
	}
	else if (!(bsscfg->wsec & MFP_CAPABLE) && (cap[0] & RSN_CAP_MFPR)) {
		WL_ERROR(("wl%d: AP does not support requested MFP cap\n",
			pub->unit));
		return 1;
	}
#endif /* MFP */

	/* Reach this only if the IE looked okay.
	 * Note that capability bits of the IE have no use here yet.
	 */
	return 0;
}


#define PROT_COND_OWNER	0x80	/* protection condition owner when condition is set */

#ifdef WLMCHAN
#define CHCTX_OVLP(ctx1, ctx2, sep)	(ctx1 == NULL || ctx2 == NULL || ctx1 == ctx2 || \
					CHSPEC_CTLOVLP((ctx1)->chanspec, (ctx2)->chanspec, sep))
#endif

static bool
wlc_ap_get_prot_cond(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint offset)
{
	bool owner = (*((uint8 *)cfg->prot_cond + offset) & PROT_COND_OWNER) != 0;
	if (owner)
		ASSERT((*((uint8 *)cfg->prot_cond + offset) & 1) != 0);
	return owner;
}

static void
wlc_ap_set_prot_cond(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint offset, bool set)
{
	int i;
	wlc_bsscfg_t *bc;

	*((uint8 *)cfg->prot_cond + offset) = set ? (1 | PROT_COND_OWNER) : 0;

	/* set: set all adjacent BSSs */
	if (set) {
		FOREACH_UP_AP(wlc, i, bc) {
			if (bc == cfg)
				continue;
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub) &&
			    !CHCTX_OVLP(bc->chan_context, cfg->chan_context, CH_10MHZ_APART))
				continue;
#endif
			*((uint8 *)bc->prot_cond + offset) |= 1;
		}
	}
	/* clear: clear all non-owner adjacent BSSs,
	 * re-set adjacent BSSs for other owners.
	 */
	else {
		FOREACH_UP_AP(wlc, i, bc) {
			if (bc == cfg)
				continue;
			if (*((uint8 *)bc->prot_cond + offset) & PROT_COND_OWNER)
				continue;
			*((uint8 *)bc->prot_cond + offset) = 0;
		}
		FOREACH_UP_AP(wlc, i, bc) {
			if (!(*((uint8 *)bc->prot_cond + offset) & PROT_COND_OWNER))
				continue;
			wlc_ap_set_prot_cond(wlc, bc, offset, TRUE);
		}
	}
}

static void
wlc_ap_update_gbss_modes(wlc_info_t *wlc)
{
	int i;
	wlc_bsscfg_t *cfg;

	FOREACH_UP_AP(wlc, i, cfg) {
		wlc_update_gbss_modes(cfg);
	}
}

static void
wlc_ap_update_n_protection(wlc_info_t *wlc)
{
	int i;
	wlc_bsscfg_t *cfg;

	FOREACH_UP_AP(wlc, i, cfg) {
		wlc_update_n_protection(cfg);
	}
}

#ifdef MFP
static uint8 *
wlc_mfp_write_ie_assoc(wlc_info_t *wlc, struct scb *scb, uint8 *buf)
{
	dot11_timeout_ie_t *pi = (dot11_timeout_ie_t *)buf;
	int len = 0;

	printf("%s\n", __FUNCTION__);

	ASSERT(scb != NULL);

	pi->id = DOT11_MNG_FT_TI_ID;
	pi->type = TIE_TYPE_ASSOC_COMEBACK;
	/* value in TUs: should be larger than 1000*200 ms used for sa query */
	pi->value = 1000*200 + 500;
	len += sizeof(dot11_timeout_ie_t);
	pi->len = len - TLV_HDR_LEN;

	return buf + len;
}

#endif /* MFP */

/* respond to association and reassociation requests */
void
wlc_ap_process_assocreq(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, void *body, uint body_len,
	struct scb *scb, bool short_preamble)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
#if defined(BCMDBG_ERR) || defined(WLMSG_ASSOC) || defined(WLMSG_WSEC)
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char ssidbuf[SSID_FMT_BUF_LEN];
#endif
	struct dot11_assoc_req *req = (struct dot11_assoc_req *) body;
	wlc_rateset_t req_rates;
	wlc_rateset_t sup_rates, ext_rates;
	uint8 req_rates_lookup[WLC_MAXRATE+1];
	bool reassoc;
	bool erp_sta;
	uint16 capability;
	uint16 listen;
	struct ether_addr *reassoc_ap = NULL;
	uint16 status = DOT11_SC_SUCCESS, aid = 0;
	bcm_tlv_t *ssid;
	bcm_tlv_t *wme_ie;
	wpa_ie_fixed_t *wpaie = NULL;
	uint8 *tlvs, *parse, *pbody = NULL, *bufend;
	uint tlvs_len, parse_len;
	uint i;
	uint8 r;
	void *p;
	struct dot11_assoc_resp *resp;
	uint len;
	int idx;
	uint8 rclist[MAXRCLISTSIZE], rclen;
#ifdef WIN7
	assoc_info_t assoc_info;
	assoc_decision_t *assoc_decision = NULL;
	int ind_status;
#endif /* WIN7 */
	/* Used to encapsulate data to a generate event */
	void *e_data = NULL;
	int e_datalen = 0;
	bool akm_ie_included = FALSE;
	wlc_bss_info_t *current_bss;
	uint8 type;
	wlc_prot_cond_t *cond;
	uint8 rates;
	uint8 ext_cap[DOT11_EXTCAP_LEN_MAX];
	uint8 ext_cap_len = 0;

	WL_TRACE(("wl%d: wlc_process_assocreq\n", pub->unit));

	ASSERT(bsscfg != NULL);
	ASSERT(bsscfg->up);
	ASSERT(BSSCFG_AP(bsscfg));

#ifdef RXCHAIN_PWRSAVE
	/* fast switch back from rxchain_pwrsave state upon association */
	wlc_reset_rxchain_pwrsave_mode(ap);
#endif /* RXCHAIN_PWRSAVE */

	current_bss = bsscfg->current_bss;
	cond = bsscfg->prot_cond;

	bzero(ext_cap, sizeof(ext_cap));
	bzero(&req_rates, sizeof(wlc_rateset_t));

	reassoc = ((ltoh16(hdr->fc) & FC_KIND_MASK) == FC_REASSOC_REQ);
	capability = ltoh16(req->capability);
	listen = ltoh16(req->listen);

#ifdef MFP
	if (SCB_MFP(scb) && SCB_AUTHENTICATED(scb) && SCB_ASSOCIATED(scb)) {
		status = DOT11_SC_ASSOC_TRY_LATER;
		goto done;
	}
#endif

	if (WIN7_OS(pub) && scb && SCB_AUTHENTICATED(scb))
		/* indicate auth event here to avoid multiple auth indications to
		 * Win7 when recv multiple auth requests
		 */
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_AUTH_IND, &hdr->sa, WLC_E_STATUS_SUCCESS,
			DOT11_SC_SUCCESS, scb->auth_alg, 0, 0);

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		WL_ASSOC(("wl%d: AP Scan in progress, abort association\n", pub->unit));
		status = DOT11_SC_ASSOC_BUSY_FAIL;
		if (WIN7_OS(pub))
			goto exit;
		else
			goto smf_stats;
	}

	if ((reassoc && body_len < DOT11_REASSOC_REQ_FIXED_LEN) ||
		(!reassoc && body_len < DOT11_ASSOC_REQ_FIXED_LEN)) {
		status = DOT11_SC_FAILURE;
		if (WIN7_OS(pub))
			goto exit;
		else
			goto smf_stats;
	}

	/* set up some locals to hold info from the (re)assoc packet */
	if (!reassoc) {
	        tlvs = (uint8 *)req + DOT11_ASSOC_REQ_FIXED_LEN;
	        tlvs_len = body_len - DOT11_ASSOC_REQ_FIXED_LEN;
	} else {
	        struct dot11_reassoc_req *reassoc_req = (struct dot11_reassoc_req *) body;
	        tlvs = (uint8 *)req + DOT11_REASSOC_REQ_FIXED_LEN;
	        tlvs_len = body_len - DOT11_REASSOC_REQ_FIXED_LEN;

	        reassoc_ap = &reassoc_req->ap;
	}

	/* send up all IEs in the WLC_E_ASSOC_IND/WLC_E_REASSOC_IND event */
	e_data = tlvs;
	e_datalen = tlvs_len;

	/* SSID is first tlv */
	ssid = (bcm_tlv_t*)tlvs;
	if (!bcm_valid_tlv(ssid, tlvs_len)) {
		status = DOT11_SC_FAILURE;
		if (WIN7_OS(pub))
			goto exit;
		else
			goto smf_stats;
	}

	ASSERT(bsscfg == scb->bsscfg);

	/* Assoc req must have rateset IE inorder to be valid.
	 * We need to parse the rateset(s) from the packet first,
	 * since req_rates is used later even when there is an error
	 * with the assocreq processing
	 */
	if (wlc_parse_rates(wlc, tlvs, tlvs_len, NULL, &req_rates)) {
		WL_ERROR(("wl%d: could not parse the rateset from (Re)Assoc Request packet from"
				  " %s\n", pub->unit, sa));
		status = DOT11_SC_ASSOC_RATE_MISMATCH;
		goto done;
	}
	if (wlc->htphy_membership != req_rates.htphy_membership) {
		WL_ASSOC(("wl%d: mismatch Membership from (Re)Assoc Request packet from"
			  " %s\n", pub->unit, sa));
		status = DOT11_SC_ASSOC_RATE_MISMATCH;
		goto done;
	}

	/* failure if the SSID does not match any active AP config */
	if ((ssid->len != bsscfg->SSID_len) || bcmp(ssid->data, bsscfg->SSID, ssid->len)) {
		WL_ASSOC(("wl%d: %s attempted association with incorrect SSID \"%s\" "
			"or BSS is Down\n", pub->unit, sa,
			(wlc_format_ssid(ssidbuf, ssid->data, ssid->len), ssidbuf)));
		status = DOT11_SC_ASSOC_FAIL;
		goto done;
	}

#ifdef MBSS
#ifdef BCMDBG_ERR
	{
		char eabuf1[ETHER_ADDR_STR_LEN];
		char eabuf2[ETHER_ADDR_STR_LEN];

		if (bsscfg != scb->bsscfg) {
			bcm_ether_ntoa(&scb->bsscfg->BSSID, eabuf1);
			bcm_ether_ntoa(&bsscfg->BSSID, eabuf2);
			WL_ERROR(("wl%d: ASSOC BSS mismatch, scb %s and ssid %s\n",
				pub->unit, eabuf1, eabuf2));
		}
	}
#endif /* BCMDBG_ERR */
#endif /* MBSS */


	/* catch the case of an already assoc. STA */
	if (SCB_ASSOCIATED(scb)) {
	        /* If we are in PS mode then return to non PS mode as there is a state mismatch
	         * between the STA and the AP
	         */
	        if (SCB_PS(scb))
	                wlc_apps_scb_ps_off(wlc, scb, TRUE);

	        /* return STA to auth state and check the (re)assoc pkt */
	        wlc_scb_clearstatebit(scb, ~AUTHENTICATED);
	}

	/* check if we are authenticated w/ this particular bsscfg */
	/*
	  get the index for that bsscfg. Would be nicer to find a way to represent
	  the authentication status directly.
	*/
	idx = WLC_BSSCFG_IDX(bsscfg);
	if (idx < 0) {
	        WL_ERROR(("wl%d: %s: association request for non existent bsscfg\n",
	                pub->unit, sa));
	        status = DOT11_SC_ASSOC_FAIL;
	        goto done;
	}

	if (!SCB_AUTHENTICATED_BSSCFG(scb, idx)) {
	        WL_ASSOC(("wl%d: %s: association request for non-authenticated station\n",
	                pub->unit, sa));
	        status = DOT11_SC_ASSOC_FAIL;
	        goto done;
	}

	if ((WL11H_ENAB(wlc) == SPECT_MNGMT_STRICT_11H) &&
		((capability & DOT11_CAP_SPECTRUM) == 0)) {

	        /* send a failure association response to this STA */
	        WL_REGULATORY(("wl%d: %s: association denied as spectrum management is required\n",
	                pub->unit, sa));
	        status = DOT11_SC_ASSOC_SPECTRUM_REQUIRED;
	        goto done;
	}

	scb->bsscfg = bsscfg;

	/* store the advertised capability field */
	scb->cap = capability;

#if defined(WL_AP_TPC)
	/* set to max */
	scb->sta_link_margin = AP_TPC_MAX_LINK_MARGIN;
	scb->ap_link_margin = AP_TPC_MAX_LINK_MARGIN;
#endif

#ifdef BAND5G
	/* Previously detected radar and are in the
	 * process of switching to a new channel
	 */
	if (wlc->block_datafifo & DATA_BLOCK_QUIET) {
	        /* send a failure association response to this node */
	        WL_REGULATORY(("wl%d: %s: association denied while in radar avoidance mode\n",
	                pub->unit, sa));
	        status = DOT11_SC_ASSOC_FAIL;
	        goto done;
	}
#endif /* BAND5G */

	/* get the requester's rates into a lookup table and record ERP capability */
	erp_sta = FALSE;
	bzero(req_rates_lookup, sizeof(req_rates_lookup));
	for (i = 0; i < req_rates.count; i++) {
	        r = req_rates.rates[i] & RATE_MASK;
		if ((r > WLC_MAXRATE) || (rate_info[r] == 0)) {
			WL_INFORM(("wlc_process_assocreq: bad rate in rate set 0x%x\n", r));
			continue;
		}
	        req_rates_lookup[r] = r;
	        if (IS_OFDM(r))
	                erp_sta = TRUE;
	}

	/* update the scb's capability flags */
	scb->flags &= ~(SCB_NONERP | SCB_LONGSLOT | SCB_SHORTPREAMBLE);
	if (wlc->band->gmode && !erp_sta)
	        scb->flags |= SCB_NONERP;
	if (wlc->band->gmode && (!(capability & DOT11_CAP_SHORTSLOT)))
	        scb->flags |= SCB_LONGSLOT;
	if (capability & DOT11_CAP_SHORT)
	        scb->flags |= SCB_SHORTPREAMBLE;

	/* check the required rates */
	for (i = 0; i < current_bss->rateset.count; i++) {
	        /* check if the rate is required */
	        r = current_bss->rateset.rates[i];
	        if (r & WLC_RATE_FLAG) {
	                if (req_rates_lookup[r & RATE_MASK] == 0) {
	                        /* a required rate was not available */
	                        WL_ERROR(("wl%d: %s does not support required rate %d\n",
	                                pub->unit, sa, r & RATE_MASK));
	                        status = DOT11_SC_ASSOC_RATE_MISMATCH;
	                        goto done;
	                }
	        }
	}

	if (N_ENAB(pub)) {
		ht_cap_ie_t *cap_ie;
#ifdef WL11N
		bcm_tlv_t *rc_ie_tlv;
#endif /* WL11N */

	        /* find the HT cap IE, if found copy the mcs set into the requested rates */
		cap_ie = wlc_read_ht_cap_ie(wlc, tlvs, tlvs_len);

	        wlc_ht_update_scbstate(wlc, scb, cap_ie, NULL, NULL);
	        if (cap_ie) {
	                bcopy(&cap_ie->supp_mcs[0], &req_rates.mcs[0], MCSSET_LEN);

	                /* verify mcs basic rate settings */
	                for (i = 0; i < MCSSET_LEN; i++) {
	                        if ((wlc->ht_add.basic_mcs[i] & req_rates.mcs[i]) !=
	                            wlc->ht_add.basic_mcs[i]) {
	                                /* a required rate was not available */
	                                WL_ERROR(("wl%d: %s does not support required mcs idx %d\n",
	                                pub->unit, sa, i));
	                                status = DOT11_SC_ASSOC_RATE_MISMATCH;
	                                goto done;
	                        }
	                }

	        } else if (N_REQD(pub)) {
	                /* non N client trying to associate, reject them */
	                status = DOT11_SC_ASSOC_RATE_MISMATCH;
	                goto done;
	        }
		wlc_process_coex_mgmt_ie(wlc, tlvs, tlvs_len, scb);
#ifdef WL11N
		/* find regulatory class IE, if found copy the regclass
		 * list into the requested regclass
		 */
		rc_ie_tlv = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_REGCLASS_ID);
		if (rc_ie_tlv) {
			if (rc_ie_tlv->len > 0 && rc_ie_tlv->len < MAXRCLISTSIZE) {
				scb->rclen = rc_ie_tlv->len;
				bcopy(rc_ie_tlv->data, scb->rclist, rc_ie_tlv->len);
			} else
				WL_INFORM(("wl%d: improper regulatory class list size %d\n",
					pub->unit, rc_ie_tlv->len));
		}
#endif /* WL11N */
	}

#ifdef WLBTAMP
	/* Handle 802.11e BT-AMP association */
	if (BSS_BTA_ENAB(wlc, bsscfg) && wlc_bta_qos(wlc->bta)) {
		if (bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_QOS_CAP_ID)) {
			WL_BTA(("wl%d: QoS Capability IE present in assocreq\n", pub->unit));

			wlc_qosinfo_update(scb, 0);     /* Clear Qos Info by default */
	                scb->flags |= SCB_11ECAP;
	        }
	}
#endif /* WLBTAMP */

	/* init per scb WPA_auth and narrow it down */
	scb->WPA_auth = bsscfg->WPA_auth;
	scb->wsec = 0;

	/*
	 * If in an auth mode that wants a WPA info element, look for one.
	 * If found, check to make sure what it requests is supported.
	 */
	if ((bsscfg->WPA_auth != WPA_AUTH_DISABLED && WSEC_ENABLED(bsscfg->wsec)) ||
	    WSEC_SES_OW_ENABLED(bsscfg->wsec)) {
	        parse = tlvs;
	        parse_len = tlvs_len;
	        if ((wpaie = bcm_find_wpsie(parse, parse_len)) != NULL) {
	            if (wpaie->length < 5) {
	                WL_ERROR(("wl%d: wlc_assocresp: unsupported request in WPS IE from"
	                " %s\n", pub->unit, sa));
	                status = DOT11_SC_ASSOC_FAIL;
	                goto done;
	            }
	        } else if ((wpaie = bcm_find_wpaie(parse, parse_len)) != NULL) {
	                akm_ie_included = TRUE;
	                /* check WPA AKM */
	                if (wlc_check_wpaie(ap, bsscfg, (uint8 *)wpaie, &scb->WPA_auth,
	                                    &scb->wsec)) {
	                        WL_ERROR(("wl%d: wlc_process_assocreq: "
	                                  "unsupported request in WPA IE from %s\n",
	                                  pub->unit, sa));
	                        status = DOT11_SC_ASSOC_FAIL;
	                        goto done;
	                }
	        } else if ((wpaie = (wpa_ie_fixed_t *)bcm_parse_tlvs(tlvs, tlvs_len,
	                DOT11_MNG_RSN_ID)) != NULL) {
	                akm_ie_included = TRUE;
	                /* check RSN AKM */
	                if (wlc_check_wpa2ie(ap, bsscfg, (bcm_tlv_t *)wpaie, scb)) {
	                        WL_ERROR(("wl%d: wlc_process_assocreq: "
	                                  "unsupported request in WPA2 IE from %s\n",
	                                  pub->unit, sa));
	                        status = DOT11_SC_ASSOC_FAIL;
	                        goto done;
	                }
	        } else {
	                /* check for transition mode */
	                if (!WSEC_WEP_ENABLED(bsscfg->wsec) && !WSEC_SES_OW_ENABLED(bsscfg->wsec)) {
	                        WL_ERROR(("wl%d: wlc_process_assocreq: "
	                                  "deny transition mode assoc req from %s... "
	                                  "transition mode not enabled on AP\n",
	                                  pub->unit, sa));
	                        status = DOT11_SC_ASSOC_FAIL;
	                        goto done;
	                }
	                scb->WPA_auth = WPA_AUTH_DISABLED;
	        }
	        WL_WSEC(("wl%d: wlc_process_assocreq: %s WPA_auth 0x%x\n", pub->unit, sa,
	                scb->WPA_auth));
	}

	/* Handle WME association */
	scb->flags &= ~(SCB_WMECAP | SCB_APSDCAP);
	if (BSS_WME_ENAB(wlc, bsscfg)) {
	        parse = tlvs;
	        parse_len = tlvs_len;
	        while ((wme_ie = bcm_parse_tlvs(parse, parse_len, DOT11_MNG_PROPR_ID)))
	                if (wlc_is_wme_ie(wlc, (uint8 *)wme_ie, &parse, &parse_len))
	                        break;
	        wlc_qosinfo_update(scb, 0);     /* Clear Qos Info by default */
	        if (wme_ie) {
	                wme_param_ie_t *wmeie_body = (wme_param_ie_t *)&wme_ie->data;
	                scb->flags |= SCB_WMECAP;
	                /* Note requested APSD parameters if AP supporting APSD */
	                if (wlc->wme_apsd) {
	                        wlc_qosinfo_update(scb, wmeie_body->qosinfo);
	                        if (scb->apsd.ac_trig & AC_BITMAP_ALL)
	                                scb->flags |= SCB_APSDCAP;
	                }
	        }
	}

	/* See if this a BRCM STA */
	wlc_process_brcm_ie(wlc, scb, (brcm_ie_t*)wlc_find_vendor_ie(tlvs, tlvs_len,
	        (const uchar*)BRCM_OUI, NULL, 0));

	/* check the capabilities.
	 * In case OW_ENABLED, allow privacy bit to be set even if !WSEC_ENABLED if
	 * there is no wpa or rsn IE in request.
	 * This covers Microsoft doing WPS association with sec bit set even when we are
	 * in open mode..
	 */
	if ((capability & DOT11_CAP_PRIVACY) && !WSEC_ENABLED(bsscfg->wsec) &&
	    (!WSEC_SES_OW_ENABLED(bsscfg->wsec) || akm_ie_included)) {
	        WL_ERROR(("wl%d: %s is requesting privacy but encryption is not enabled on the"
	                " AP. SES OW %d akm ie %d\n", pub->unit, sa,
	                WSEC_SES_OW_ENABLED(bsscfg->wsec), akm_ie_included));
	        status = DOT11_SC_ASSOC_FAIL;
	        goto done;
	}


	/* If not APSTA, deny association to stations unable to support 802.11b short preamble
	 * if short network.  In APSTA, we don't enforce short preamble on the AP side since it
	 * might have been set by the STA side.  We need an extra flag for enforcing short
	 * preamble independently.
	 */
	if (!APSTA_ENAB(pub) && BAND_2G(wlc->band->bandtype) &&
	    bsscfg->PLCPHdr_override == WLC_PLCP_SHORT && !(capability & DOT11_CAP_SHORT)) {
	        WL_ERROR(("wl%d: %s does not support short preambles\n", pub->unit, sa));
	        status = DOT11_SC_ASSOC_SHORT_REQUIRED;
	        goto done;
	}
	/* deny association to stations unable to support 802.11g short slot timing
	 * if shortslot exclusive network
	 */
	if ((BAND_2G(wlc->band->bandtype)) &&
	    ap->shortslot_restrict && !(capability & DOT11_CAP_SHORTSLOT)) {
	        WL_ERROR(("wl%d: %s does not support ShortSlot\n", pub->unit, sa));
	        status = DOT11_SC_ASSOC_SHORTSLOT_REQUIRED;
	        goto done;
	}
	/* check the max association limit */
	if (wlc_assocscb_getcnt(wlc) >= appvt->maxassoc) {
	        WL_ERROR(("wl%d: %s denied association due to max association limit\n",
	                  pub->unit, sa));
	        status = DOT11_SC_ASSOC_BUSY_FAIL;
	        goto done;
	}

#if defined(MBSS)
	if (wlc_bss_assocscb_getcnt(wlc, bsscfg) >= bsscfg->maxassoc) {
		WL_ERROR(("wl%d.%d %s denied association due to max BSS association "
		          "limit\n", pub->unit, WLC_BSSCFG_IDX(bsscfg), sa));
		status = DOT11_SC_ASSOC_BUSY_FAIL;
		goto done;
	}
#endif /* MBSS */

#ifdef WIN7
	if (WIN7_OS(pub) && BSSCFG_HAS_NATIVEIF(bsscfg)) {
		/* init assoc decision variables */
		assoc_decision = SCB_ASSOC_CUBBY(ap, scb);
		assoc_decision->assoc_approved = FALSE;
		assoc_decision->reject_reason = DOT11_SC_FAILURE;
		/* indicate pre-(re)association event to OS and get association
		 * decision from OS from the same thread
		 */
		wlc_bss_mac_event(wlc, bsscfg,
			reassoc ? WLC_E_PRE_REASSOC_IND : WLC_E_PRE_ASSOC_IND,
			&hdr->sa, WLC_E_STATUS_SUCCESS, status, 0, body, body_len);
		if (!assoc_decision->assoc_approved) {
			status = assoc_decision->reject_reason;
			WL_ERROR(("wl%d.%d %s denied association due to Win7\n",
				pub->unit, WLC_BSSCFG_IDX(bsscfg), sa));
			goto done;
		}
	}
#endif /* WIN7 */

	if (N_ENAB(pub) && WSEC_ENABLED(bsscfg->wsec)) {
		/* When a HT STA using TKIP-only unicast chiper suite or WEP tries
		 * to associate, exclude HT IEs from assoc response to force
		 * the HT STA to operate in legacy mode.
		 */
		if ((((wlc->ht_wsec_restriction & WLC_HT_TKIP_RESTRICT) &&
		      SCB_HT_CAP(scb) && (scb->wsec == TKIP_ENABLED)) ||
		     ((wlc->ht_wsec_restriction & WLC_HT_WEP_RESTRICT) &&
		      SCB_HT_CAP(scb) && ((scb->wsec == WEP_ENABLED))))) {
			/* Clear HT state of the scb */
	                ASSERT(req_rates.count > 0);
			wlc_ht_update_scbstate(wlc, scb, NULL, NULL, NULL);
		}
	}

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg)) {
		if (wlc_p2p_process_assocreq(wlc->p2p, scb, tlvs, tlvs_len) != BCME_OK) {
			status = DOT11_SC_ASSOC_FAIL;
			goto done;
		}
	}
#endif

	/*
	 * here if association is successful
	 */

	/* Check for association of a NonERP STA */
	if ((scb->flags & SCB_NONERP))
		wlc_ap_set_prot_cond(wlc, bsscfg,
			OFFSETOF(wlc_prot_cond_t, nonerp_assoc), TRUE);

	/* Check for association of an OFDM STA */
	if ((scb->flags & (SCB_NONERP | SCB_HTCAP)) == 0)
	        cond->ofdm_assoc = TRUE;

	/* Check for association of a Non Green Field STA */
	if ((scb->flags & SCB_NONGF))
	        cond->non_gf_assoc = TRUE;

	/* Check for association of a 20MHz only HT STA in 40MHz operating */
	if (!(scb->flags & SCB_IS40) &&
	    CHSPEC_IS40(current_bss->chanspec))
	        cond->ht20in40_assoc = TRUE;

	/* Check for association of a non-short slot capable STA */
	if ((scb->flags & SCB_LONGSLOT))
		wlc_ap_set_prot_cond(wlc, bsscfg,
			OFFSETOF(wlc_prot_cond_t, longslot_assoc), TRUE);

	/* Check for association of a non-short preamble capable STA */
	if (!(scb->flags & SCB_SHORTPREAMBLE))
		wlc_ap_set_prot_cond(wlc, bsscfg,
			OFFSETOF(wlc_prot_cond_t, longpre_assoc), TRUE);

	/* Check for APSD association STA */
	if ((scb->flags & SCB_APSDCAP))
	        cond->wme_apsd_assoc = TRUE;

#ifdef RADIO_PWRSAVE
	if (RADIO_PWRSAVE_ENAB(wlc->ap) && wlc_radio_pwrsave_in_power_save(wlc->ap)) {
		wlc_radio_pwrsave_exit_mode(wlc->ap);
		WL_INFORM(("We have an assoc request, going right out of the power save mode!\n"));
	}
#endif
#ifdef WL11N
	/* Check for association of a 40 intolerant STA */
	if ((scb->flags & SCB_HT40INTOLERANT)) {
		cond->ht40intolerant_assoc = TRUE;
		if (N_ENAB(pub) && COEX_ENAB(pub)) {
			wlc_ht_upd_coex_state(wlc, DOT11_OBSS_COEX_40MHZ_INTOLERANT);

			if (CHSPEC_IS40(current_bss->chanspec)) {
				wlc_ht_coex_update_fid_time(wlc);
				wlc_ht_coex_switch_bw(wlc, TRUE);
			}
		}
	}
#endif /* WL11N */

	/* Check for non 11n APSD association STA */
	if ((scb->flags & SCB_APSDCAP) && ((scb->flags & SCB_HTCAP) == 0))
		cond->non11n_apsd_assoc = TRUE;

	if (wlc->band->gmode)
	        wlc_ap_update_gbss_modes(wlc);

	if (N_ENAB(pub))
	        wlc_ap_update_n_protection(wlc);

	WL_ASSOC(("AP: Checking if WEP key needs to be inserted\n"));
	/* If Multi-SSID is enabled, and Legacy WEP is in use for this bsscfg, a "pairwise" key must
	   be created by copying the default key from the bsscfg.
	*/
	if ((bsscfg->WPA_auth == WPA_AUTH_DISABLED))
	        WL_ASSOC(("WPA disabled \n"));

	/* Since STA is declaring privacy w/o WPA IEs => WEP */
	if ((scb->wsec == 0) && (capability & DOT11_CAP_PRIVACY) &&
	    WSEC_WEP_ENABLED(bsscfg->wsec))
	        scb->wsec = WEP_ENABLED;

	if (WSEC_WEP_ENABLED(bsscfg->wsec))
	        WL_ASSOC(("WEP enabled \n"));
	if (MBSS_ENAB(pub))
	        WL_ASSOC(("MBSS on \n"));
	if ((MBSS_ENAB(pub) || bsscfg != wlc->cfg) &&
	    bsscfg->WPA_auth == WPA_AUTH_DISABLED && WSEC_WEP_ENABLED(bsscfg->wsec)) {
	        wsec_key_t *defkey = WSEC_BSS_DEFAULT_KEY(bsscfg);
	        if (defkey)
	                WL_ASSOC(("Def key installed \n"));
		if (defkey &&
		    (defkey->algo == CRYPTO_ALGO_WEP1 ||
		     defkey->algo == CRYPTO_ALGO_WEP128)) {
	                int bcmerror;
	                WL_ASSOC(("wl%d: %s Inserting key \n",
	                        pub->unit, sa));
	                bcmerror = wlc_key_insert(wlc, bsscfg, defkey->len, defkey->id,
	                        defkey->flags,
	                        defkey->algo, defkey->data, &scb->ea, NULL, NULL);
	                ASSERT(!bcmerror);
	        }
	}

	/* Get the AID, assign a new one if needed */
	if (!scb->aid)
	        scb->aid = wlc_bsscfg_newaid(scb->bsscfg);

	aid = scb->aid;


	/* Based on wsec for STA, update AMPDU feature
	 * By spec, 11n device can send AMPDU only with Open or CCMP crypto
	 */
	if (N_ENAB(pub)) {
	        /* scb->wsec is the specific unicast algo being used.
	         * It should be a subset of the whole bsscfg wsec
	         */
	        ASSERT((bsscfg->wsec & scb->wsec) == scb->wsec);
	        if (((scb->wsec == WEP_ENABLED) || (scb->wsec == TKIP_ENABLED)) && SCB_AMPDU(scb))
	                wlc_txmod_unconfig(wlc, scb, TXMOD_AMPDU);
	        else if (SCB_AMPDU(scb))
	                wlc_txmod_config(wlc, scb, TXMOD_AMPDU);
	}

done:
	/* create the supported rates and extended supported rates elts */
	bzero(&sup_rates, sizeof(wlc_rateset_t));
	bzero(&ext_rates, sizeof(wlc_rateset_t));
	/* check for a supported rates override */
	if (wlc->sup_rates_override.count > 0)
	        bcopy(&wlc->sup_rates_override, &sup_rates, sizeof(wlc_rateset_t));
	wlc_rateset_elts(wlc, bsscfg, &current_bss->rateset, &sup_rates, &ext_rates);


	len = sizeof(struct dot11_assoc_resp);
	len += 2 + sup_rates.count;
	if (ext_rates.count > 0)
	        len += 2 + ext_rates.count;

#ifdef WLBTAMP
	if (SCB_11E(scb))
		len += TLV_HDR_LEN + EDCA_PARAM_IE_LEN;
#endif /* WLBTAMP */

	ASSERT(bsscfg != NULL);
	len += wlc_vndr_ie_getlen(bsscfg, VNDR_IE_ASSOCRSP_FLAG, NULL);

	if (bsscfg->brcm_ie[TLV_LEN_OFF] > 0)
	        len += TLV_HDR_LEN + bsscfg->brcm_ie[TLV_LEN_OFF];

	if (SCB_WME(scb))
	        len += TLV_HDR_LEN + WME_PARAM_IE_LEN;

#ifdef WL11U
	if (bsscfg->iw_ie) {
		/* 11u interworking, extended capabilities */
		setbit(ext_cap, DOT11_EXT_CAP_IW);
		if (ext_cap_len < DOT11_EXTCAP_LEN_IW)
			ext_cap_len = DOT11_EXTCAP_LEN_IW;
	}
#endif /* WL11U */

	rclen = 0;
	bzero(rclist, MAXRCLISTSIZE);
	if (SCB_HT_CAP(scb)) {
#ifdef NOT_YET
		rclen = wlc_get_regclass_list(wlc->cmi, rclist, MAXRCLISTSIZE,
			current_bss->chanspec, TRUE);
		len += (TLV_HDR_LEN + rclen);
#endif /* NOT_YET */
		/* account for the length of both the prop ie and ana-assigned ie */
		len += 2*TLV_HDR_LEN + HT_CAP_IE_LEN + HT_ADD_IE_LEN;

		if (COEX_ACTIVE(wlc)) {
			setbit(ext_cap, DOT11_EXT_CAP_OBSS_COEX_MGMT);
			if (ext_cap_len < DOT11_EXTCAP_LEN_COEX)
				ext_cap_len = DOT11_EXTCAP_LEN_COEX;
			len += TLV_HDR_LEN + DOT11_OBSS_SCAN_IE_LEN;
		}
	}

#ifdef WLP2P
	if (SCB_P2P(scb))
		len += wlc_p2p_write_ie_assoc_len(wlc->p2p, scb, status);
#endif

#ifdef MFP
	if (SCB_MFP(scb) && SCB_AUTHENTICATED(scb) && SCB_ASSOCIATED(scb)) {
		len += sizeof(dot11_timeout_ie_t);
	}
#endif

	if (ext_cap_len)
		len += TLV_HDR_LEN + ext_cap_len;


	/* alloc a packet */
	if ((p = wlc_frame_get_mgmt(wlc, (uint16)(reassoc ? FC_REASSOC_RESP : FC_ASSOC_RESP),
		&hdr->sa, &bsscfg->cur_etheraddr, &bsscfg->BSSID, len, &pbody)) == NULL) {
		status = DOT11_SC_ASSOC_BUSY_FAIL;
		if (WIN7_OS(pub))
			goto exit;
		else
			goto smf_stats;
	}
	ASSERT(pbody && ISALIGNED((uintptr)pbody, sizeof(uint16)));
	resp = (struct dot11_assoc_resp *) pbody;

	/* save bufend location */
	bufend = pbody + len;

	/* fill out the association response body */
	resp->capability = DOT11_CAP_ESS;
	if (BAND_2G(wlc->band->bandtype) &&
	    bsscfg->PLCPHdr_override == WLC_PLCP_SHORT)
	        resp->capability |= DOT11_CAP_SHORT;
	if (WSEC_ENABLED(bsscfg->wsec) && bsscfg->wsec_restrict)
	        resp->capability |= DOT11_CAP_PRIVACY;
	if (wlc->shortslot && wlc->band->gmode)
	        resp->capability |= DOT11_CAP_SHORTSLOT;

	resp->capability = htol16(resp->capability);
	resp->status = htol16(status);
	resp->aid = htol16(aid);

	tlvs = (uint8 *) &resp[1];

	/* Supported Rates */
	tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_RATES_ID, sup_rates.count, sup_rates.rates);

#ifdef WLBTAMP
	/* EDCA Parameter Set */
	if (SCB_11E(scb)) {
		WL_BTA(("wl%d: Adding EDCA Parameter Set IE to assocresp\n", pub->unit));

		tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_EDCA_PARAM_ID, sizeof(edca_param_ie_t),
		     (char *)((uintptr)&wlc->wme_param_ie + OFFSETOF(wme_param_ie_t, qosinfo)));
	}
#endif /* WLBTAMP */

	/* Extended Supported Rates */
	if (ext_rates.count > 0)
		tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_EXT_RATES_ID, ext_rates.count,
			ext_rates.rates);

	/* filter rateset for the BSS supported rates */
	wlc_rate_hwrs_filter_sort_validate(&req_rates, &current_bss->rateset, FALSE,
		wlc->stf->txstreams);

	/* add HT Information Elements */
	if (SCB_HT_CAP(scb)) {
		ht_cap_ie_t cap_ie;
		ht_add_ie_t add_ie;

		wlc_write_ht_cap_ie(bsscfg, &cap_ie,
		                    &req_rates.mcs[0],
		                    BAND_2G(wlc->band->bandtype));
		wlc_write_ht_add_ie(bsscfg, &add_ie);
		tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_HT_CAP, HT_CAP_IE_LEN, &cap_ie);
#ifdef NOT_YET
		if (rclen)
			tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_REGCLASS_ID, rclen, rclist);
#endif
		tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_HT_ADD, HT_ADD_IE_LEN, &add_ie);

		/* Support for HT Information 20/40MHz Exchange */
		if (COEX_ACTIVE(wlc)) {
			obss_params_t params;
			/* need to convert to 802.11 little-endian format */
			bcopy((uint8 *)&wlc->obss->ap_params, (uint8 *)&params,
				DOT11_OBSS_SCAN_IE_LEN);
			wlc_ht_obss_scanparams_hostorder(wlc, &params, FALSE);
			tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_HT_OBSS_ID,
				DOT11_OBSS_SCAN_IE_LEN,	&params);
		}
	}

	if (ext_cap_len) {
		/* extended capabilities */
		tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_EXT_CAP_ID, ext_cap_len, ext_cap);
	}

#ifdef MFP
	if (SCB_MFP(scb) && SCB_AUTHENTICATED(scb) && SCB_ASSOCIATED(scb)) {
		tlvs = wlc_mfp_write_ie_assoc(wlc, scb, tlvs);
	}
#endif

	/* Write the Vendor IEs */
	tlvs = wlc_vndr_ie_write(bsscfg, tlvs, BUFLEN(tlvs, bufend), VNDR_IE_ASSOCRSP_FLAG);

	/* BRCM proprietary elt */
	if (bsscfg->brcm_ie[TLV_LEN_OFF] > 0)
		tlvs = wlc_copy_info_elt(tlvs, bsscfg->brcm_ie);

	/* Add WME IE */
	if (SCB_WME(scb))
		tlvs = wlc_write_info_elt(tlvs, DOT11_MNG_PROPR_ID,
			sizeof(wlc->wme_param_ie), (char*)&wlc->wme_param_ie);

#ifdef WLP2P
	if (SCB_P2P(scb))
		tlvs = wlc_p2p_write_ie_assoc(wlc->p2p, scb, status, tlvs);
#endif

	/* make sure length calc to allocate the packet was correct */
	ASSERT(len == (uint) (tlvs - (uint8*)resp));

	/* send the association response */
	wlc_queue_80211_frag(wlc, p, bsscfg->wlcif->qi, scb, short_preamble, NULL, 0);

#ifdef MFP
	if (SCB_MFP(scb) && SCB_AUTHENTICATED(scb) && SCB_ASSOCIATED(scb)) {
		wlc_scb_start_sa_query(wlc, bsscfg, scb);
		goto smf_stats;
	}
#endif

	/* if error, we're done */
	if (status != DOT11_SC_SUCCESS) {
		if (WIN7_OS(pub))
			goto exit;
		else
			goto smf_stats;
	}


	/* update scb state */

	WL_ASSOC(("wl%d.%d %s %sassociated\n", pub->unit, WLC_BSSCFG_IDX(bsscfg), sa,
		reassoc ? "re" : ""));

	scb->aid = aid;

	/*
	 * scb->listen is used by the AP for timing out PS pkts,
	 * ensure pkts are held for at least one dtim period
	 */
	scb->listen = MAX(current_bss->dtim_period, listen);
	wlc_scb_setstatebit(scb, ASSOCIATED);

	wlc_scb_set_bsscfg(scb, bsscfg);

	/* Start beaconing if this is first STA */
	if (DYNBCN_ENAB(bsscfg) &&
	    wlc_bss_assocscb_getcnt(wlc, bsscfg) == 1) {
		wlc_bsscfg_bcn_enable(wlc, bsscfg);
	}

	scb->assoctime = pub->now;

#ifndef WIN7
	/* notify other APs on the DS that this station has roamed */
	if (reassoc && bcmp((char*)&bsscfg->BSSID, reassoc_ap->octet, ETHER_ADDR_LEN))
	        wlc_reassoc_notify(ap, &hdr->sa, bsscfg->wlcif);

	/* 802.11f assoc. announce pkt */
	wlc_assoc_notify(ap, &hdr->sa, bsscfg->wlcif);
#endif /* WIN7 */

	/* copy sanitized set to scb */
#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		rates = WLC_RATES_OFDM;
	else
#endif
	rates = WLC_RATES_CCK_OFDM;
	wlc_rateset_filter(&req_rates, &scb->rateset, FALSE, rates, RATE_MASK,
	                   (BSS_N_ENAB(wlc, bsscfg) && SCB_HT_CAP(scb)));

	/* re-initialize rate info
	 * Note that this wipes out any previous rate stats on the STA. Since this
	 * being called at Association time, it does not seem like a big loss.
	 */
	wlc_scb_ratesel_init(wlc, scb);

	/* Free old WPA IE if one exists */
	if (scb->wpaie) {
	        MFREE(pub->osh, scb->wpaie, scb->wpaie_len);
	        scb->wpaie_len = 0;
	        scb->wpaie = NULL;
	}

	/*
	 * Send driver association indication up so that NAS can initiate
	 * key exchange if it needs to.
	 *
	 * When WEP is enabled along with WPA, if STA associates with an empty
	 * WPA IE and a plain 802.11 open or shared authentication, we also send
	 * driver indication up so that NAS can enable the 802.1x port.
	 * However if STA attempts with WPA IE and shared 802.11 authentication,
	 * we'll deny such request by deauthenticating the STA.
	 */
	if ((bsscfg->WPA_auth != WPA_AUTH_DISABLED) && WSEC_ENABLED(bsscfg->wsec) &&
	    (WSEC_WEP_ENABLED(bsscfg->wsec) || wpaie)) {
	        /*
	         * WPA with 802.11 open authentication is required, or 802.11 shared
	         * key authentication without WPA authentication attempt (no WPA IE).
	         */
	        if (scb->auth_alg == DOT11_OPEN_SYSTEM || !wpaie) {

	                /* Store the WPA IE for later retrieval */
	                if (wpaie) {
	                        if ((scb->wpaie = MALLOC(pub->osh, wpaie->length + 2))) {
	                                scb->wpaie_len = wpaie->length + 2;
	                                bcopy((char *)wpaie, (char *)scb->wpaie, scb->wpaie_len);
	                        }
	                }
	        }
	        /* attempt WPA auth with 802.11 shared key authentication */
	        else {
	                WL_ERROR(("wl%d: WPA auth attempt 802.11 shared key auth\n", pub->unit));
			(void)wlc_senddeauth(wlc, &scb->ea, &bsscfg->BSSID,
			                     &bsscfg->cur_etheraddr,
			                     scb, DOT11_RC_AUTH_INVAL);
	                status = DOT11_RC_AUTH_INVAL;
	        }
	}

	wlc_update_txba_burst_war(wlc, NULL);

#ifdef WLBTAMP
	if (BSS_BTA_ENAB(wlc, bsscfg) && !reassoc)
		wlc_bta_join_complete(wlc->bta, scb, 0);
#endif /* WLBTAMP */

#if defined(BCMAUTH_PSK)
	/* kick off 4 way handshaking */
	if (bsscfg->authenticator != NULL)
		wlc_auth_join_complete(bsscfg->authenticator, &scb->ea, TRUE);
#endif /* BCMAUTH_PSK */

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		wlc_p2p_enab_upd(wlc->p2p, bsscfg);
#endif

	/* Enable BTCX PS protection */
	wlc_enable_btc_ps_protection(wlc, bsscfg, TRUE);


exit:
	/* send WLC_E_REASSOC_IND/WLC_E_ASSOC_IND to interested App and/or non-WIN7 OS */
	wlc_bss_mac_event(wlc, bsscfg, reassoc ? WLC_E_REASSOC_IND : WLC_E_ASSOC_IND,
	                  &hdr->sa, WLC_E_STATUS_SUCCESS, status, 0, e_data, e_datalen);
#ifdef WIN7
	if (assoc_decision && !assoc_decision->assoc_approved)
		ind_status = WLC_E_STATUS_ABORT;
	else if (status == DOT11_SC_SUCCESS) {
		assoc_info.assoc_req = body;
		assoc_info.assoc_req_len = body_len;
		assoc_info.assoc_rsp = pbody;
		assoc_info.assoc_rsp_len = len;
		assoc_info.bcn = bsscfg->bcn;
		assoc_info.bcn_len = bsscfg->bcn_len;
		assoc_info.auth_alg = scb->auth_alg;
		assoc_info.WPA_auth = (uint8)scb->WPA_auth;
		assoc_info.wsec = scb->wsec;
		assoc_info.wpaie = (wpa_ie_fixed_t*)scb->wpaie;
		assoc_info.ewc_cap = (N_ENAB(pub) && (scb->flags & SCB_HTCAP)) ? TRUE : FALSE;
		assoc_info.ofdm = wlc_rateset_isofdm(scb->rateset.count, scb->rateset.rates);
		ind_status = WLC_E_STATUS_SUCCESS;
	} else
		ind_status = WLC_E_STATUS_FAIL;

	/* send WLC_E_REASSOC_IND_NDIS/WLC_E_ASSOC_IND_NDIS to NDIS */
	wlc_bss_mac_event(wlc, bsscfg, reassoc ? WLC_E_REASSOC_IND_NDIS: WLC_E_ASSOC_IND_NDIS,
		&hdr->sa, ind_status, status, 0,
		ind_status == WLC_E_STATUS_SUCCESS ? &assoc_info : 0,
		ind_status == WLC_E_STATUS_SUCCESS ? sizeof(assoc_info) : 0);

#else
#endif /* WIN7 */

smf_stats:
	type = (reassoc ? SMFS_TYPE_REASSOC : SMFS_TYPE_ASSOC);
	wlc_smfstats_update(wlc, bsscfg, type, status);
}

#ifdef RXCHAIN_PWRSAVE
/*
 * Reset the rxchain power save related counters and modes
 */
void
wlc_reset_rxchain_pwrsave_mode(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	WL_INFORM(("Resetting the rxchain power save counters\n"));
	/* Come out of the power save mode if we are in it */
	if (appvt->rxchain_pwrsave.pwrsave.in_power_save) {
		wlc_stf_rxchain_set(wlc, appvt->rxchain_pwrsave.rxchain);
#ifdef WL11N
		/* need to restore rx_stbc HT capability after exit rxchain_pwrsave mode */
		wlc_stf_exit_rxchain_pwrsave(wlc, appvt->rxchain_pwrsave.ht_cap_rx_stbc);
#endif /* WL11N */
	}
	wlc_reset_pwrsave_mode(ap, PWRSAVE_RXCHAIN);
}

void
wlc_disable_rxchain_pwrsave(wlc_ap_info_t *ap)
{
	wlc_disable_pwrsave(ap, PWRSAVE_RXCHAIN);

	return;
}

#ifdef WL11N
/*
 * get rx_stbc HT capability, if in rxchain_pwrsave mode, return saved rx_stbc value,
 * because rx_stbc capability may be changed when enter rxchain_pwrsave mode
 */
uint8
wlc_rxchain_pwrsave_stbc_rx_get(wlc_info_t *wlc)
{
	uint8 ht_cap_rx_stbc = wlc_stf_stbc_rx_get(wlc);
	wlc_ap_info_t *ap = wlc->ap;
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	if ((appvt != NULL) && appvt->rxchain_pwrsave.pwrsave.in_power_save) {
		ht_cap_rx_stbc = appvt->rxchain_pwrsave.ht_cap_rx_stbc;
	}
	return ht_cap_rx_stbc;
}
#endif /* WL11N */
#endif /* RXCHAIN_PWRSAVE */

#ifdef RADIO_PWRSAVE
/*
 * Reset the radio power save related counters and modes
 */
static void
wlc_reset_radio_pwrsave_mode(wlc_ap_info_t *ap)
{
	uint8 dtim_period;
	uint16 beacon_period;
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bsscfg_t *bsscfg = wlc->cfg;

	if (bsscfg->associated) {
		dtim_period = bsscfg->current_bss->dtim_period;
		beacon_period = bsscfg->current_bss->beacon_period;
	} else {
		dtim_period = wlc->default_bss->dtim_period;
		beacon_period = wlc->default_bss->beacon_period;
	}

	wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
	wlc_reset_pwrsave_mode(ap, PWRSAVE_RADIO);
	appvt->radio_pwrsave.pwrsave_state = 0;
	appvt->radio_pwrsave.radio_disabled = FALSE;
	appvt->radio_pwrsave.cncl_bcn = FALSE;
	switch (appvt->radio_pwrsave.level) {
	case RADIO_PWRSAVE_LOW:
		appvt->radio_pwrsave.on_time = 2*beacon_period*dtim_period/3;
		appvt->radio_pwrsave.off_time = beacon_period*dtim_period/3;
		break;
	case RADIO_PWRSAVE_MEDIUM:
		appvt->radio_pwrsave.on_time = beacon_period*dtim_period/2;
		appvt->radio_pwrsave.off_time = beacon_period*dtim_period/2;
		break;
	case RADIO_PWRSAVE_HIGH:
		appvt->radio_pwrsave.on_time = beacon_period*dtim_period/3;
		appvt->radio_pwrsave.off_time = 2*beacon_period*dtim_period/3;
		break;
	default:
		ASSERT(0);
		break;
	}
}
#endif /* RADIO_PWRSAVE */

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
/*
 * Reset power save related counters and modes
 */
void
wlc_reset_pwrsave_mode(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pwrsave_t *pwrsave = NULL;
	int enable = 0;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			enable = ap->rxchain_pwrsave_enable;
			pwrsave = &appvt->rxchain_pwrsave.pwrsave;
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			enable = ap->radio_pwrsave_enable;
			pwrsave = &appvt->radio_pwrsave.pwrsave;
			appvt->radio_pwrsave.pwrsave_state = 0;
			break;
#endif
		default:
			break;
	}

	WL_INFORM(("Resetting the rxchain power save counters\n"));
	if (pwrsave) {
		pwrsave->quiet_time_counter = 0;
		pwrsave->in_power_save = FALSE;
		pwrsave->power_save_check = enable;
	}
}

#ifdef RXCHAIN_PWRSAVE
static void
wlc_disable_pwrsave(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pwrsave_t *pwrsave = NULL;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			pwrsave = &appvt->rxchain_pwrsave.pwrsave;
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			pwrsave = &appvt->radio_pwrsave.pwrsave;
			appvt->radio_pwrsave.pwrsave_state = 0;
			break;
#endif
		default:
			break;
	}

	WL_INFORM(("Disabling power save mode\n"));
	if (pwrsave) {
		pwrsave->in_power_save = FALSE;
		pwrsave->power_save_check = FALSE;
	}
}
#endif /* RXCHAIN_PWRSAVE */

#endif /* RXCHAIN_PWRSAVE || RADIO_PWRSAVE */

void
wlc_restart_ap(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	int i;
	wlc_bsscfg_t *bsscfg;
#ifdef RADAR
	bool sradar_ap = FALSE;
#endif /* RADAR */

	WL_TRACE(("wl%d: %s:\n", appvt->pub->unit, __FUNCTION__));

#ifdef RADAR
	FOREACH_AP(wlc, i, bsscfg) {
		if (BSS_11H_SRADAR_ENAB(wlc, bsscfg) ||
		     BSS_11H_AP_NORADAR_CHAN_ENAB(wlc, bsscfg)) {
			sradar_ap = TRUE;
			break;
		}
	}

	if (WL11H_AP_ENAB(wlc) && !sradar_ap &&
	    (wlc_channel_locale_flags_in_band(wlc->cmi, BAND_5G_INDEX) & WLC_DFS_TPC)) {
		/* random channel to start with */
		appvt->dfs_cac.chanspec_next = wlc_dfs_chanspec(ap);
		WL_REGULATORY(("wl%d: %s: dfs selected channel %d\n",
		               appvt->pub->unit, __FUNCTION__,
		               CHSPEC_CHANNEL(appvt->dfs_cac.chanspec_next)));
	}
#endif /* RADAR */

	if (MBSS_ENAB(wlc->pub)) {
		ap->pre_tbtt_us = MBSS_PRE_TBTT_DEFAULT_us;
		ap->pre_tbtt_max_lat_us = MBSS_PRE_TBTT_MAX_LATENCY_us;
	} else {
		ap->pre_tbtt_us = PRE_TBTT_DEFAULT_us;
	}

	/* Bring up any enabled AP configs which aren't up yet */
	FOREACH_BSS(wlc, i, bsscfg) {
		if (BSSCFG_AP(bsscfg) && bsscfg->enable) {
			WL_APSTA_UPDN(("wl%d: wlc_restart_ap -> wlc_bsscfg_up on bsscfg %d%s\n",
			               appvt->pub->unit, i, (bsscfg->up ? "(already up)" : "")));
			wlc_bsscfg_up(wlc, bsscfg);
		}
	}

#ifdef RADAR
	if (WL11H_AP_ENAB(wlc) && AP_ACTIVE(wlc)) {
		if (!sradar_ap)
			wlc_set_dfs_cacstate(wlc->ap, ON);
	}
#endif /* RADAR */

#ifdef RXCHAIN_PWRSAVE
	wlc_reset_rxchain_pwrsave_mode(ap);
#endif
#ifdef RADIO_PWRSAVE
	wlc_reset_radio_pwrsave_mode(ap);
#endif
}

void
wlc_bss_up(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, char *bcn, int len)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bss_info_t *target_bss = bsscfg->target_bss;

	WL_TRACE(("wl%d: %s:\n", wlc->pub->unit, __FUNCTION__));

#ifdef WL11N
	/* Adjust target bss rateset according to target channel bandwidth */
	wlc_rateset_bw_mcs_filter(&target_bss->rateset,
		wlc->band->mimo_cap_40?CHSPEC_WLC_BW(target_bss->chanspec):0);
#endif /* WL11N */

	wlc_suspend_mac_and_wait(wlc);
	wlc_BSSinit(wlc, target_bss, bsscfg, bcn, BCN_TMPL_LEN, WLC_BSS_START);
	wlc_enable_mac(wlc);

	/* update the AP association count */
	wlc_ap_up_upd(ap, bsscfg, TRUE);

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, bsscfg))
		wlc_p2p_bss_upd(wlc->p2p, bsscfg, TRUE);
	else
		wlc_p2p_ap_bss_upd(wlc->p2p, bsscfg, TRUE);
#endif

	wlc_led_event(wlc->ledh);

	if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band))
		wlc_full_phy_cal(wlc, bsscfg, PHY_PERICAL_UP_BSS);

	/* Indicate AP is now up */
	WL_APSTA_UPDN(("Reporting link up on config %d (AP enabled)\n",
		WLC_BSSCFG_IDX(bsscfg)));
	wlc_link(wlc, TRUE, &bsscfg->cur_etheraddr, bsscfg, 0);
}

static void
wlc_ap_up_upd(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, bool state)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_pub_t *pub = appvt->pub;
	wlc_info_t *wlc = appvt->wlc;

	bsscfg->associated = state;

#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub)) {
		if (state == TRUE) {
			wlc_mchan_create_bss_chan_context(wlc, bsscfg,
			                                  bsscfg->current_bss->chanspec);
		}
		else {
			wlc_mchan_delete_bss_chan_context(wlc, bsscfg);
		}
	}
#endif /* WLMCHAN */

	wlc->aps_associated = (uint8)AP_BSS_UP_COUNT(wlc);
	wlc->pub->associated = wlc->aps_associated > 0 || wlc->stas_associated > 0;

	wlc_phy_hold_upd(wlc->band->pi, PHY_HOLD_FOR_NOT_ASSOC,
		wlc->pub->associated ? FALSE : TRUE);

	/* Win7: Enable AP if it wasn't enabled */
	if (WIN7_OS(pub) && wlc->aps_associated && !AP_ENAB(pub))
		pub->_ap = 1;

	pub->associated = wlc->aps_associated > 0 || wlc->stas_associated > 0;
}

/* known reassociation magic packets */

struct lu_reassoc_pkt {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 unknown_field[2];
	uint8 data[36];
};

static
const struct lu_reassoc_pkt lu_reassoc_template = {
	{ { 0x01, 0x60, 0x1d, 0x00, 0x01, 0x00 },
	{ 0 },
	HTON16(sizeof(struct lu_reassoc_pkt) - sizeof(struct ether_header)) },
	{ 0xaa, 0xaa, 0x03, { 0x00, 0x60, 0x1d }, HTON16(0x0001) },
	{ 0x00, 0x04 },
	"Lucent Technologies Station Announce"
};

struct csco_reassoc_pkt {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint8 unknown_field[4];
	struct ether_addr ether_dhost, ether_shost, a1, a2, a3;
	uint8 pad[4];
};
/* WES - I think the pad[4] at the end of the struct above should be
 * dropped, it appears to just be the ethernet padding to 64
 * bytes. This would fix the length calculation below, (no more -4).
 * It matches with the 0x22 field in the 'unknown_field' which appears
 * to be the length of the encapsulated packet starting after the snap
 * header.
 */

static
const struct csco_reassoc_pkt csco_reassoc_template = {
	{ { 0x01, 0x40, 0x96, 0xff, 0xff, 0x00 },
	{ 0 },
	HTON16(sizeof(struct csco_reassoc_pkt) - sizeof(struct ether_header) - 4) },
	{ 0xaa, 0xaa, 0x03, { 0x00, 0x40, 0x96 }, HTON16(0x0000) },
	{ 0x00, 0x22, 0x02, 0x02 },
	{ { 0x01, 0x40, 0x96, 0xff, 0xff, 0x00 } },
	{ { 0 } },
	{ { 0 } },
	{ { 0 } },
	{ { 0 } },
	{ 0x00, 0x00, 0x00, 0x00 }
};

bool
wlc_roam_check(wlc_ap_info_t *ap, wlc_bsscfg_t *bsscfg, struct ether_header *eh, uint len)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */
	struct lu_reassoc_pkt *lu = (struct lu_reassoc_pkt *) eh;
	struct csco_reassoc_pkt *csco = (struct csco_reassoc_pkt *) eh;
	struct ether_addr *sta = NULL;
	struct scb *scb;

	pub = appvt->pub;

	/* check for Lucent station announce packet */
	if (!bcmp(eh->ether_dhost, (const char*)lu_reassoc_template.eth.ether_dhost,
	          ETHER_ADDR_LEN) &&
	    len >= sizeof(struct lu_reassoc_pkt) &&
	    !bcmp((const char*)&lu->snap, (const char*)&lu_reassoc_template.snap,
	              DOT11_LLC_SNAP_HDR_LEN))
		sta = (struct ether_addr *) lu->eth.ether_shost;

	/* check for Cisco station announce packet */
	else if (!bcmp(eh->ether_dhost, (const char*)csco_reassoc_template.eth.ether_dhost,
	               ETHER_ADDR_LEN) &&
	         len >= sizeof(struct csco_reassoc_pkt) &&
	         !bcmp((const char*)&csco->snap, (const char*)&csco_reassoc_template.snap,
	               DOT11_LLC_SNAP_HDR_LEN))
		sta = &csco->a1;

	/* not a magic packet */
	else
		return (FALSE);

	/* disassociate station */
	if ((scb = wlc_scbfind(wlc, sta)) &&
	    SCB_ASSOCIATED(scb) && (scb->bsscfg == bsscfg)) {
		WL_ERROR(("wl%d: %s roamed\n", pub->unit, bcm_ether_ntoa(sta, eabuf)));
		if (APSTA_ENAB(pub) && (scb->flags & SCB_MYAP)) {
			WL_APSTA(("wl%d: Ignoring roam report from my AP.\n", pub->unit));
			return (FALSE);
		}

		wlc_senddisassoc(wlc, sta, &bsscfg->BSSID,
		                 &bsscfg->cur_etheraddr,
		                 scb, DOT11_RC_NOT_AUTH);
		wlc_scb_resetstate(scb);
		wlc_scb_setstatebit(scb, AUTHENTICATED);

		wlc_bss_mac_event(wlc, bsscfg, WLC_E_DISASSOC_IND, sta, WLC_E_STATUS_SUCCESS,
			DOT11_RC_DISASSOC_LEAVING, 0, 0, 0);
	}

	return (TRUE);
}

#ifndef WIN7
static void
wlc_assoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlcif)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;

	uchar pkt80211f[] = {0x00, 0x01, 0xAF, 0x81, 0x01, 0x00};
	struct ether_header *eh;
	uint16 len = sizeof(pkt80211f);
	void *p;

	/* prepare 802.11f IAPP announce packet. This should look like a wl rx packet since it sent
	   along the same path. Some work should be done to evaluate the real need for
	   extra headroom (how much), but the alignement enforced by wlc->hwrxoff must be preserved.
	*/
	if ((p = PKTGET(pub->osh, sizeof(pkt80211f) + BCMEXTRAHDROOM + wlc->hwrxoff + ETHER_HDR_LEN,
	                FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED((uintptr)PKTDATA(pub->osh, p), sizeof(uint32)));
	PKTPULL(pub->osh, p, BCMEXTRAHDROOM + wlc->hwrxoff);

	eh = (struct ether_header *) PKTDATA(pub->osh, p);
	bcopy(&ether_bcast, eh->ether_dhost, ETHER_ADDR_LEN);
	bcopy(sta, eh->ether_shost, ETHER_ADDR_LEN);
	eh->ether_type = hton16(len);
	bcopy(pkt80211f, &eh[1], len);
	WL_PRPKT("802.11f assoc", PKTDATA(pub->osh, p), PKTLEN(pub->osh, p));
	wl_sendup(wlc->wl, wlcif->wlif, p, 1);
	return;

err:
	WL_ERROR(("wl%d: wlc_assoc_notify: pktget error\n", pub->unit));
	WLCNTINCR(pub->_cnt->rxnobuf);
	WLCNTINCR(wlcif->_cnt.rxnobuf);
	return;
}

static void
wlc_reassoc_notify(wlc_ap_info_t *ap, struct ether_addr *sta, struct wlc_if *wlcif)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	void *p;
	osl_t *osh;
	struct lu_reassoc_pkt *lu;
	struct csco_reassoc_pkt *csco;
	int len;
	wlc_bsscfg_t *cfg;

	osh = pub->osh;

	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);

	/* prepare Lucent station announce packet */
	len = sizeof(struct lu_reassoc_pkt);
	if ((p = PKTGET(osh, len + BCMEXTRAHDROOM + wlc->hwrxoff + ETHER_HDR_LEN, FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED((uintptr)PKTDATA(osh, p), sizeof(uint32)));
	PKTPULL(osh, p, BCMEXTRAHDROOM + wlc->hwrxoff);

	lu = (struct lu_reassoc_pkt*) PKTDATA(osh, p);
	bcopy((const char*)&lu_reassoc_template, (char*) lu, sizeof(struct lu_reassoc_pkt));
	bcopy((const char*)sta, (char*)&lu->eth.ether_shost, ETHER_ADDR_LEN);
	WL_PRPKT("lu", PKTDATA(osh, p), PKTLEN(osh, p));
	wl_sendup(wlc->wl, wlcif->wlif, p, 1);

	/* prepare Cisco station announce packets */
	len = sizeof(struct csco_reassoc_pkt);
	if ((p = PKTGET(osh, len + BCMEXTRAHDROOM + wlc->hwrxoff + ETHER_HDR_LEN, FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED((uintptr)PKTDATA(osh, p), sizeof(uint32)));
	PKTPULL(osh, p, BCMEXTRAHDROOM + wlc->hwrxoff);

	csco = (struct csco_reassoc_pkt *) PKTDATA(osh, p);
	bcopy((const char*)&csco_reassoc_template, (char*)csco, len);
	bcopy((char*)sta, (char*)&csco->eth.ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)sta, (char*)&csco->ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)sta, (char*)&csco->a1, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a2, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a3, ETHER_ADDR_LEN);
	WL_PRPKT("csco1", PKTDATA(osh, p), PKTLEN(osh, p));
	wl_sendup(wlc->wl, wlcif->wlif, p, 1);

	len = sizeof(struct csco_reassoc_pkt);
	if ((p = PKTGET(osh, len + BCMEXTRAHDROOM + wlc->hwrxoff + ETHER_HDR_LEN, FALSE)) == NULL)
		goto err;
	ASSERT(ISALIGNED((uintptr)PKTDATA(osh, p), sizeof(uint32)));
	PKTPULL(osh, p, BCMEXTRAHDROOM + wlc->hwrxoff);

	csco = (struct csco_reassoc_pkt *) PKTDATA(osh, p);
	bcopy((const char*)&csco_reassoc_template, (char*)csco, len);
	bcopy((char*)&cfg->BSSID, (char*)&csco->eth.ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->ether_shost, ETHER_ADDR_LEN);
	bcopy((char*)sta, (char*)&csco->a1, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a2, ETHER_ADDR_LEN);
	bcopy((char*)&cfg->BSSID, (char*)&csco->a3, ETHER_ADDR_LEN);
	WL_PRPKT("csco2", PKTDATA(osh, p), PKTLEN(osh, p));
	wl_sendup(wlc->wl, wlcif->wlif, p, 1);
	return;

err:
	WL_ERROR(("wl%d: wlc_reassoc_notify: pktget error\n", pub->unit));
	WLCNTINCR(pub->_cnt->rxnobuf);
	WLCNTINCR(wlcif->_cnt.rxnobuf);
	return;
}
#endif /* WIN7 */

static uint8
wlc_lowest_basicrate_get(wlc_bsscfg_t *cfg)
{
	uint8 i, rate = 0;
	wlc_bss_info_t *current_bss = cfg->current_bss;

	for (i = 0; i < current_bss->rateset.count; i++) {
		if (current_bss->rateset.rates[i] & WLC_RATE_FLAG) {
			rate = current_bss->rateset.rates[i] & RATE_MASK;
			break;
		}
	}

	return rate;
}

void
wlc_ap_probe_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;

	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL)
		return;

#ifdef WDS
	if (SCB_WDS(scb))
		wlc_ap_wds_probe_complete(wlc, txs, scb);
	else
#endif
	wlc_ap_sta_probe_complete(wlc, txs, scb);
}

static void
wlc_ap_sta_probe(wlc_ap_info_t *ap, struct scb *scb)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	void *pkt;
	uint8 rate_override;

	/* If a probe is still pending, don't send another one */
	if (scb->flags & SCB_PENDING_PROBE)
		return;

	/* use the lowest basic rate */
	rate_override = wlc_lowest_basicrate_get(scb->bsscfg);

	ASSERT(VALID_RATE(wlc, rate_override));

	pkt = wlc_sendnulldata(wlc, &scb->ea, rate_override, 0, PRIO_8021D_BE);

	if (pkt == NULL) {
		WL_ERROR(("wl%d: wlc_ap_sta_probe: wlc_sendnulldata failed\n",
		          wlc->pub->unit));
		return;
	}

	WLPKTFLAG2_TYPESET(pkt, WLF2_TYPE_PRB);

	scb->flags |= SCB_PENDING_PROBE;
}

static void
wlc_ap_sta_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb)
{
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	ASSERT(scb != NULL);

	scb->flags &= ~SCB_PENDING_PROBE;

	/* ack indicates the sta should not be removed or we might have missed the ACK but if there
	 * was some activity after sending the probe then it indicates there is life out there in
	 * scb.
	 */
	if (((txstatus & TX_STATUS_MASK) != TX_STATUS_NO_ACK) ||
	    (wlc->pub->now - scb->used < wlc->scb_activity_time)) {
		scb->grace_attempts = 0;
		return;
	}

	/* If still more grace_attempts are left, then probe the STA again */
	if (++scb->grace_attempts < wlc->scb_max_probe) {
		wlc->reprobe_scb = TRUE;
		return;
	}

	WL_INFORM(("wl%d: wlc_ap_sta_probe_complete: no ACK from %s for Null Data\n",
		wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));

	if (SCB_AUTHENTICATED(scb)) {
		wlc_deauth_complete(wlc, SCB_BSSCFG(scb), WLC_E_STATUS_SUCCESS, &scb->ea,
			DOT11_RC_INACTIVITY, 0);
	}

#if defined(BCMDBG)
	WL_INFORM(("wlc_ap_sta_probe_complete: %s is dead!!!\n", bcm_ether_ntoa(&scb->ea, eabuf)));
#endif

	/* free the scb */
	wlc_scbfree(wlc, scb);
}

#ifdef WDS
/* Send null packets to wds partner and check for response */
static void
wlc_ap_wds_probe(wlc_ap_info_t *ap, struct scb *scb)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t* wlc = appvt->wlc;
	void *pkt;
	uint8 rate_override;

	/* use the lowest basic rate */
	rate_override = wlc_lowest_basicrate_get(scb->bsscfg);

	ASSERT(VALID_RATE(wlc, rate_override));

	pkt = wlc_sendnulldata(wlc, &scb->ea, rate_override, 0, PRIO_8021D_BE);

	if (pkt == NULL) {
		WL_ERROR(("wl%d: wlc_ap_wds_probe: wlc_sendnulldata failed\n",
		          wlc->pub->unit));
		return;
	}

	WLPKTFLAG2_TYPESET(pkt, WLF2_TYPE_PRB);
}

/*  Check for ack, if there is no ack, reset the rssi value */
static void
wlc_ap_wds_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb)
{
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	ASSERT(scb != NULL);

	wlc->reprobe_wds = FALSE;

	/* ack indicates the sta is there */
	if (txstatus & TX_STATUS_MASK) {
		scb->flags |= SCB_WDS_LINKUP;
		return;
	}


	WL_INFORM(("wl%d: wlc_ap_wds_probe_complete: no ACK from %s for Null Data\n",
	           wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));

	scb->flags &= ~SCB_WDS_LINKUP;
}
#endif /* WDS */

#ifdef RADAR
#ifdef WIN7
static wlc_bsscfg_t*
wlc_get_ap_bsscfg(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bsscfg_t *bsscfg = NULL;
	int i;

	if (AP_ACTIVE(wlc)) {
		for (i = 0; i < 2; i++) {
			if (wlc->bsscfg[i] && wlc->bsscfg[i]->_ap && wlc->bsscfg[i]->up) {
				bsscfg = wlc->bsscfg[i];
				/* one ap supported in Win7 */
				break;
			}
		}
	}

	ASSERT(bsscfg);
	return bsscfg;
}
#endif /* WIN7 */

static int
wlc_dfs_timer_init(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t* wlc = appvt->wlc;
	wlc_pub_t* pub;

	pub = appvt->pub;
	appvt->dfs_cac.ism_monitor = FALSE; /* put it to normal mode */

	appvt->dfs_cac.timer_running = FALSE;

	if (!(appvt->dfs_timer = wl_init_timer(wlc->wl,
		wlc_dfs_cacstate_handler, appvt, "dfs"))) {
		WL_ERROR(("wl%d: wlc_dfs_timer_init failed\n", pub->unit));
		return -1;
	}
	appvt->dfs_cac.cactime_preism =
	appvt->dfs_cac.cactime_postism =
		WLC_DFS_CAC_TIME_SEC_DEFAULT;

	appvt->dfs_cac.nop_sec = WLC_DFS_NOP_SEC_DEFAULT;

	return 0;
}

void
wlc_dfs_timer_add(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	if (appvt->dfs_cac.timer_running == FALSE) {
		appvt->dfs_cac.timer_running = TRUE;
		wl_add_timer(wlc->wl, appvt->dfs_timer, WLC_DFS_RADAR_CHECK_INTERVAL, TRUE);
	}
}

static bool
wlc_dfs_timer_delete(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	bool canceled = TRUE;

	if (appvt->dfs_cac.timer_running == TRUE) {
		canceled = wl_del_timer(wlc->wl, appvt->dfs_timer);
		ASSERT(canceled == TRUE);
		appvt->dfs_cac.timer_running = FALSE;
	}
	return canceled;
}

static void
wlc_dfs_chanspec_oos(wlc_ap_info_t *ap, chanspec_t chanspec)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t* wlc = appvt->wlc;
	bool is_block = FALSE;
	wlc_pub_t *pub;
	uint8 ctrl_ch, ext_ch;

	pub = appvt->pub;
	if (N_ENAB(pub) && CHSPEC_IS40(chanspec)) {
		ctrl_ch = LOWER_20_SB(CHSPEC_CHANNEL(chanspec));
		ext_ch = UPPER_20_SB(CHSPEC_CHANNEL(chanspec));
		ap->chan_blocked[ctrl_ch] = appvt->dfs_cac.nop_sec;
		ap->chan_blocked[ext_ch] = appvt->dfs_cac.nop_sec;

		WL_DFS(("wl%d: dfs : channel %d & %d put out of service\n", wlc->pub->unit,
			ctrl_ch, ext_ch));
	} else {
		ctrl_ch = CHSPEC_CHANNEL(chanspec);
		ext_ch = 0;
		ap->chan_blocked[ctrl_ch] = appvt->dfs_cac.nop_sec;

		WL_DFS(("wl%d: dfs : channel %d put out of service\n", wlc->pub->unit, ctrl_ch));
	}

	wlc_set_quiet_chanspec(wlc->cmi, chanspec);

	if (!bcmp("US", wlc_channel_country_abbrev(wlc->cmi), 2) ||
		!bcmp("CA", wlc_channel_country_abbrev(wlc->cmi), 2)) {
		if ((ctrl_ch >= 120 && ctrl_ch <= 128) ||
		   (N_ENAB(pub) && CHSPEC_IS40(chanspec) && ext_ch >= 120 && ext_ch <= 128))
			is_block = TRUE;
	}

	/* Special US and CA handling, remove set of channels 120, 124, 128 if
	 * any get a radar pulse.  For CA they will be blocked for uptime of the driver.
	 */
	if (is_block) {
		uint32  block_time = !bcmp("CA", wlc_channel_country_abbrev(wlc->cmi), 2) ?
		    WLC_CHANBLOCK_FOREVER : appvt->dfs_cac.nop_sec;

		wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(120));
		ap->chan_blocked[120] = block_time;
		wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(124));
		ap->chan_blocked[124] = block_time;
		wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(128));
		ap->chan_blocked[128] = block_time;
	}
}

/*
 * Random channel selection for DFS
 * Returns a valid chanspec of a valid radar free channel, using the AP configuration
 * to choose 20 or 40 MHz bandwidth and side-band
 * Returns 0 if there are no valid radar free channels available
 */
static chanspec_t
wlc_dfs_chanspec(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	chanvec_t channels20, channels40, *chanvec;
	chanspec_t chspec;
	uint chan20_cnt, chan40_cnt;
	uint chan, rand_idx, rand_channel;

	(void)pub; /* eliminate possible unused variable compiler warning */

	chan20_cnt = chan40_cnt = 0;
	bzero(channels20.vec, sizeof(channels20.vec));
	bzero(channels40.vec, sizeof(channels40.vec));

	/* walk the channels looking for good 20MHz channels */
	for (chan = 0; chan < MAXCHANNEL; chan++) {
		chspec = CH20MHZ_CHSPEC(chan);
		if (wlc_valid_ap_chanspec(wlc, chspec)) {
			setbit(channels20.vec, chan);
			chan20_cnt++;
		}
	}

	/* check for 40MHz channels only if we are capable of 40MHz, the default
	 * bss was configured for 40MHz, and the locale allows 40MHz
	 */
	if (N_ENAB(pub) &&
	    CHSPEC_IS40(wlc->default_bss->chanspec) &&
	    (wlc->band->mimo_cap_40) &&
	    !(wlc_channel_locale_flags(wlc->cmi) & WLC_NO_40MHZ)) {
		/* walk the channels looking for good 40MHz channels */
		for (chan = 0; chan < MAXCHANNEL; chan++) {
			chspec = CH40MHZ_CHSPEC(chan, CHSPEC_CTL_SB(wlc->default_bss->chanspec));
			if (wlc_valid_ap_chanspec(wlc, chspec)) {
				setbit(channels40.vec, chan);
				chan40_cnt++;
			}
		}
	}

	if (!chan20_cnt) {
	        /* no channel found */
	        return 0;
	}

	rand_idx = R_REG(pub->osh, &wlc->regs->tsf_random);

	if (chan40_cnt) {
		rand_idx = rand_idx % chan40_cnt;
	        chanvec = &channels40;
	} else {
		rand_idx = rand_idx % chan20_cnt;
	        chanvec = &channels20;
	}

	/* choose 'rand_idx'th channel */
	for (rand_channel = 0, chan = 0; chan < MAXCHANNEL; chan++) {
	        if (isset(chanvec->vec, chan)) {
	                if (rand_idx == 0) {
	                        rand_channel = chan;
	                        break;
	                }
	                rand_idx--;
	        }
	}

	ASSERT(rand_channel);

	if (chan40_cnt)
		chspec = CH40MHZ_CHSPEC(rand_channel, CHSPEC_CTL_SB(wlc->default_bss->chanspec));
	else
		chspec = CH20MHZ_CHSPEC(rand_channel);

	ASSERT(wlc_valid_chanspec_db(wlc->cmi, chspec));

	if (appvt->dfs_cac.chanspec_safe &&
		!wlc_radar_chanspec(wlc->cmi, appvt->dfs_cac.chanspec_safe)) {
		chspec = appvt->dfs_cac.chanspec_safe;
		WL_DFS(("dfs : set dfs channel forced to safe\n"));
	}

	if (appvt->dfs_cac.chanspec_forced &&
		wlc_valid_chanspec_db(wlc->cmi, appvt->dfs_cac.chanspec_forced)) {
		chspec = appvt->dfs_cac.chanspec_forced;
		WL_DFS(("dfs : set dfs channel forced to 0\n"));
		appvt->dfs_cac.chanspec_forced = 0; /* no more forcing */
	}

	WL_DFS(("wl%d: %s: dfs selected channel %d%s\n", pub->unit, __FUNCTION__,
	        wf_chspec_ctlchan(chspec),
	        CHSPEC_IS20(chspec) ? "" : (CHSPEC_SB_UPPER(chspec) ? "U" : "L")));

	return chspec;
}

/* check for a chanspec on which an AP can set up a BSS
 * Returns TRUE if the chanspec is valid for the local, not restricted, and
 * has not been blocked by a recent radar pulse detection.
 * Otherwise will return FALSE.
 */
static bool
wlc_valid_ap_chanspec(wlc_info_t *wlc, chanspec_t chspec)
{
	uint channel = CHSPEC_CHANNEL(chspec);

	if (!wlc_valid_chanspec_db(wlc->cmi, chspec) ||
	    wlc_restricted_chanspec(wlc->cmi, chspec))
		return FALSE;

	if (CHSPEC_IS40(chspec)) {
		if (wlc->ap->chan_blocked[LOWER_20_SB(channel)] ||
		    wlc->ap->chan_blocked[UPPER_20_SB(channel)])
			return FALSE;
	} else if (wlc->ap->chan_blocked[channel]) {
		return FALSE;
	}

	return TRUE;
}

static bool
wlc_radar_detected(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
	int radar_type;
	int	radar_interval;
	int	min_pw;
#if defined(BCMDBG) || defined(WLMSG_DFS)
	uint i;
	char radar_type_str[24];
	char chanbuf[CHANSPEC_STR_LEN];
	static const struct {
		int radar_type;
		const char *radar_type_name;
	} radar_names[] = {
		{RADAR_TYPE_NONE, "NONE"},
		{RADAR_TYPE_ETSI_1, "ETSI_1"},
		{RADAR_TYPE_ETSI_2, "ETSI_2"},
		{RADAR_TYPE_ETSI_3, "ETSI_3"},
		{RADAR_TYPE_ITU_E, "ITU_E"},
		{RADAR_TYPE_ITU_K, "ITU_K"},
		{RADAR_TYPE_UNCLASSIFIED, "UNCLASSIFIED"},
		{RADAR_TYPE_BIN5, "BIN5"},
		{RADAR_TYPE_STG2, "S2"},
		{RADAR_TYPE_STG3, "S3"},
		{RADAR_TYPE_FRA, "S3F"}
	};
#endif /* BCMDBG || WLMSG_DFS */
	wlc_bsscfg_t *cfg = wlc->cfg;
	wlc_csa_t *csa = cfg->csa;

	/* Should not detect radar when operating in non radar channel */
	if (wlc_radar_chanspec(wlc->cmi, wlc->home_chanspec) != TRUE)
		return FALSE;

	pub = appvt->pub;
	radar_type = wlc_phy_radar_detect_run(wlc->band->pi);
	radar_interval = radar_type >> 14;
	min_pw = radar_type >> 4 & 0x1ff;
	radar_type = radar_type & 0xf;
	/* Pretend we saw radar - for testing */
	if ((csa->spect_state & RADAR_SIM) ||
	    radar_type != RADAR_TYPE_NONE) {

#if defined(BCMDBG) || defined(WLMSG_DFS)
		snprintf(radar_type_str, sizeof(radar_type_str),
			"%s", "UNKNOWN");
		for (i = 0; i < ARRAYSIZE(radar_names); i++) {
			if (radar_names[i].radar_type == radar_type)
				snprintf(radar_type_str, sizeof(radar_type_str),
					"%s", radar_names[i].radar_type_name);
		}

		WL_DFS(("WL%d: DFS: %s ########## RADAR DETECTED ON CHANNEL %s"
			" ########## Intv=%d, min_pw=%d, AT %dMS\n", pub->unit,
			radar_type_str,
			wf_chspec_ntoa(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf),
			radar_interval, min_pw,
			(appvt->dfs_cac.cactime - appvt->dfs_cac.duration) *
			WLC_DFS_RADAR_CHECK_INTERVAL));
#endif /* BCMDBG || WLMSG_DFS */

		csa->spect_state &= ~RADAR_SIM; /* clear one-shot radar simulator */
		return TRUE;
	} else
		return FALSE;
}

/* set cacstate to IDLE and un-mute */
static void
wlc_dfs_cacstate_idle_set(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;

	pub = appvt->pub;
	appvt->dfs_cac.status.state = WL_DFS_CACSTATE_IDLE;
	wlc_mute(wlc, OFF, PHY_MUTE_FOR_PREISM);

	WL_DFS(("wl%d: dfs : state to %s channel %d at %dms\n",
		pub->unit, wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
		CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC),
		(appvt->dfs_cac.cactime -
		appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));

	appvt->dfs_cac.cactime =  /* unit in WLC_DFS_RADAR_CHECK_INTERVAL */
	appvt->dfs_cac.duration =
		(appvt->dfs_cac.cactime_postism*1000)/WLC_DFS_RADAR_CHECK_INTERVAL;
}

/* set cacstate to ISM and un-mute */
static void
wlc_dfs_cacstate_ism_set(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
	int  cal_mode;

	pub = appvt->pub;
	appvt->dfs_cac.status.chanspec_cleared = WLC_BAND_PI_RADIO_CHANSPEC;
	 /* clear the channel */
	wlc_clr_quiet_chanspec(wlc->cmi, appvt->dfs_cac.status.chanspec_cleared);

	appvt->dfs_cac.status.state = WL_DFS_CACSTATE_ISM;
	wlc_mute(wlc, OFF, PHY_MUTE_FOR_PREISM);

	wlc_iovar_getint(wlc, "phy_percal", (int *)&cal_mode);
	wlc_iovar_setint(wlc, "phy_percal", PHY_PERICAL_SPHASE);
	wlc_phy_cal_perical(wlc->band->pi, PHY_PERICAL_UP_BSS);
	wlc_iovar_setint(wlc, "phy_percal", cal_mode);

	WL_DFS(("wl%d: dfs : state to %s channel %d at %dms\n",
		pub->unit, wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
		CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC),
		(appvt->dfs_cac.cactime -
		appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));

	appvt->dfs_cac.cactime =  /* unit in WLC_DFS_RADAR_CHECK_INTERVAL */
	appvt->dfs_cac.duration =
		(appvt->dfs_cac.cactime_postism*1000)/WLC_DFS_RADAR_CHECK_INTERVAL;


#ifdef WIN7
	{
		wlc_bsscfg_t *bsscfg;
		bsscfg = wlc_get_ap_bsscfg(ap);
		if (bsscfg)
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_DFS_AP_RESUME, NULL,
				WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);
	}
#endif /* WIN7 */
}

/* set cacstate to OOC and mute */
static void
wlc_dfs_cacstate_ooc_set(wlc_ap_info_t *ap, uint target_state)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;

	pub = appvt->pub;
	wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);

	appvt->dfs_cac.status.state = target_state;

	WL_DFS(("wl%d: dfs : state to %s at %dms\n",
		pub->unit, wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
		(appvt->dfs_cac.cactime -
		appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));

	appvt->dfs_cac.duration = appvt->dfs_cac.cactime; /* reset it */

#ifdef WIN7
	{
		wlc_bsscfg_t *bsscfg;
		bsscfg = wlc_get_ap_bsscfg(ap);
		if (bsscfg)
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_DFS_AP_STOP, NULL,
				WLC_E_STATUS_NOCHANS, 0, 0, 0, 0);
	}
#endif /* WIN7 */
}

static void
wlc_dfs_cacstate_idle(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;

	wlc_dfs_timer_delete(ap);

	/* clear cac_enable flag */
	appvt->dfs_cac_enabled = FALSE;
}

static void
wlc_dfs_cacstate_cac(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t* wlc = appvt->wlc;
	wlc_pub_t *pub;
	uint target_state;

	pub = appvt->pub;
	if (wlc_radar_detected(ap) == TRUE) {
		wlc_dfs_chanspec_oos(ap, WLC_BAND_PI_RADIO_CHANSPEC);

		if (!(appvt->dfs_cac.chanspec_next = wlc_dfs_chanspec(ap))) {
			/* out of channels */
			if (appvt->dfs_cac.status.state == WL_DFS_CACSTATE_PREISM_CAC) {
				target_state = WL_DFS_CACSTATE_PREISM_OOC;
			} else {
				target_state = WL_DFS_CACSTATE_POSTISM_OOC;
			}
			wlc_dfs_cacstate_ooc_set(ap, target_state);
			return;
		}

		wlc_do_chanswitch(wlc->cfg, appvt->dfs_cac.chanspec_next);

		if (wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) == TRUE) {
			/* do cac with new channel */
			WL_DFS(("wl%d: dfs : state to %s channel %d at %dms\n",
				pub->unit,
				wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
				CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC),
				(appvt->dfs_cac.cactime -
				appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));
			appvt->dfs_cac.duration =
				appvt->dfs_cac.cactime; 	/* use same cactime. already
								 * in proper unit
								 */
			return;
		}
		else {
			wlc_dfs_cacstate_idle_set(ap); /* set to IDLE */
			return;
		}
	}

	if (!appvt->dfs_cac.duration) {
		/* cac completed. un-mute all. resume normal bss operation */
		wlc_dfs_cacstate_ism_set(ap);
	}
}

static void
wlc_dfs_cacstate_ism(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t* wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	wlc_bsscfg_t *cfg = wlc->cfg;
	wlc_csa_t *csa = cfg->csa;

	(void)pub;

	if (wlc_radar_detected(ap) == FALSE)
	        return;

	/* radar has been detected */

	if (appvt->dfs_cac.ism_monitor == TRUE) {
	        /* channel switching is disabled */
	        WL_DFS(("wl%d: dfs : current channel %d is maintained as channel switching is"
	                " disabled.\n",
	                pub->unit, CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC)));
	        return;
	}

	/* radar detected. mark the channel back to QUIET channel */
	wlc_set_quiet_chanspec(wlc->cmi, appvt->dfs_cac.status.chanspec_cleared);
	appvt->dfs_cac.status.chanspec_cleared = 0; /* cleare it */

	/* continue with CSA */
	wlc_dfs_chanspec_oos(ap, WLC_BAND_PI_RADIO_CHANSPEC);
	appvt->dfs_cac.chanspec_next = wlc_dfs_chanspec(ap); /* it will be included in csa */

	/* send csa */
	if (!appvt->dfs_cac.chanspec_next) {
	        /* out of channels */
	        /* just use the current channel for csa */
	        csa->csa.chspec = WLC_BAND_PI_RADIO_CHANSPEC;
	} else {
	        csa->csa.chspec = appvt->dfs_cac.chanspec_next;
	}
	csa->csa.mode = DOT11_CSA_MODE_NO_TX;
	csa->csa.count = MAX((WLC_DFS_CSA_MSEC/cfg->current_bss->beacon_period),
	        WLC_DFS_CSA_BEACONS);

	csa->csa.reg = wlc_get_regclass(wlc->cmi, csa->csa.chspec);

	/* need to send legacy CSA and new 11n Ext-CSA if is n-enabled */
	wlc_send_action_switch_channel(ap, cfg);

	/* dpc handles NEED_TO_UPDATE_BCN, NEED_TO_SWITCH_CHANNEL */
	csa->spect_state |= (NEED_TO_UPDATE_BCN | NEED_TO_SWITCH_CHANNEL);
	wlc->block_datafifo |= DATA_BLOCK_QUIET;        /* block data traffic but allow control */
	wlc_bss_update_beacon(wlc, cfg);
	wlc_bss_update_probe_resp(wlc, cfg, TRUE);

	appvt->dfs_cac.status.state = WL_DFS_CACSTATE_CSA;        /* next state */

	WL_DFS(("wl%d: dfs : state to %s channel current %d next %d at %dms, starting CSA"
		" process\n",
		pub->unit, wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
		CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC), CHSPEC_CHANNEL(csa->csa.chspec),
		(appvt->dfs_cac.cactime -
			appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));

	appvt->dfs_cac.duration =
	appvt->dfs_cac.cactime =
	        appvt->dfs_cac.cactime_postism;
}

/* csa transmission */
static void
wlc_dfs_cacstate_csa(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
	wlc_bsscfg_t *cfg = wlc->cfg;
	wlc_csa_t *csa = cfg->csa;

	pub = appvt->pub;
	if ((csa->spect_state & NEED_TO_SWITCH_CHANNEL) ||
	    (csa->spect_state & NEED_TO_UPDATE_BCN) ||
	    (wlc->block_datafifo & DATA_BLOCK_QUIET))
	        return;

	/* csa completed - TBTT dpc switched channel */

	if (!(appvt->dfs_cac.chanspec_next)) {
	        /* ran out of channels, goto OOC */
	        wlc_dfs_cacstate_ooc_set(ap, WL_DFS_CACSTATE_POSTISM_OOC);
	        return;
	}

	if (wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) == TRUE) {
		if (appvt->dfs_cac.cactime_postism) {
			appvt->dfs_cac.status.state = WL_DFS_CACSTATE_POSTISM_CAC;
			WL_DFS(("wl%d: dfs : state to %s channel %d at %dms\n",
				pub->unit,
				wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
			        CHSPEC_CHANNEL(csa->csa.chspec),
			        (appvt->dfs_cac.cactime -
			         appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));

			appvt->dfs_cac.duration =
			appvt->dfs_cac.cactime =
				(appvt->dfs_cac.cactime_postism*1000)/
				WLC_DFS_RADAR_CHECK_INTERVAL;
	        }
	        else {
	                wlc_dfs_cacstate_ism_set(ap);
	        }
	}
	else {
	        wlc_dfs_cacstate_idle_set(ap);
	}

	wlc_update_beacon(wlc);
	wlc_update_probe_resp(wlc, TRUE);
}


/*
 * dfs has run Out Of Channel.
 * wait for a channel to come out of Non-Occupancy Period.
 */
static void
wlc_dfs_cacstate_ooc(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub;
	uint    current_time;

	pub = appvt->pub;
	if (!(appvt->dfs_cac.chanspec_next = wlc_dfs_chanspec(ap))) {
		/* still no channel out of channels. Nothing to do */
		return;
	}

	wlc_do_chanswitch(wlc->cfg, appvt->dfs_cac.chanspec_next);

	if (wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) == TRUE) {
		current_time = (appvt->dfs_cac.cactime -
			appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL;

		/* unit of cactime is WLC_DFS_RADAR_CHECK_INTERVAL */
		if (appvt->dfs_cac.status.state == WL_DFS_CACSTATE_PREISM_OOC) {
			appvt->dfs_cac.cactime =
			appvt->dfs_cac.duration =
				(appvt->dfs_cac.cactime_preism*1000)/
				WLC_DFS_RADAR_CHECK_INTERVAL;
			appvt->dfs_cac.status.state = WL_DFS_CACSTATE_PREISM_CAC;
		} else {
			appvt->dfs_cac.cactime =
			appvt->dfs_cac.duration =
				(appvt->dfs_cac.cactime_postism*1000)/
				WLC_DFS_RADAR_CHECK_INTERVAL;
			appvt->dfs_cac.status.state = WL_DFS_CACSTATE_POSTISM_CAC;
		}

		if (appvt->dfs_cac.cactime) {
			wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);

			WL_DFS(("wl%d: dfs : state to %s channel %d at %dms\n",
				pub->unit,
				wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
				CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC), current_time));
		} else {
			/* corresponding cac is disabled */
			wlc_dfs_cacstate_ism_set(ap);
		}
	} else {
		wlc_dfs_cacstate_idle_set(ap); /* set to idle */
	}
}

static void
wlc_dfs_cacstate_handler(void *arg)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) arg;
	wlc_ap_info_t* ap = &appvt->appub;
	wlc_info_t *wlc;
	wlc_pub_t *pub = appvt->pub;

	wlc = appvt->wlc;
	if (!pub->up || !appvt->dfs_cac_enabled)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

	if (WIN7_OS(pub) && SCAN_IN_PROGRESS(wlc->scan))
		return;

	ASSERT(appvt->dfs_cac.status.state < WL_DFS_CACSTATES);

	appvt->dfs_cac.duration--;

	wlc_dfs_cacstate_fn_ary[appvt->dfs_cac.status.state](ap);
}

static void
wlc_dfs_cacstate_init(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	wlc_bsscfg_t *cfg = wlc->cfg;
	wlc_csa_t *csa = cfg->csa;

	ASSERT(WL11H_AP_ENAB(wlc));

	if (!pub->up)
		return;

	if (wlc_radar_chanspec(wlc->cmi, appvt->dfs_cac.status.chanspec_cleared) == TRUE) {
		 /* restore QUIET setting */
		wlc_set_quiet_chanspec(wlc->cmi, appvt->dfs_cac.status.chanspec_cleared);
	}
	appvt->dfs_cac.status.chanspec_cleared = 0; /* clear it */

	csa->quiet_cmd.count = csa->csa.count = 0;

	if (wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) == TRUE) {
		wlc_dfs_timer_add(ap);

		/* unit of cactime is WLC_DFS_RADAR_CHECK_INTERVAL */
		appvt->dfs_cac.cactime =
			(appvt->dfs_cac.cactime_preism*1000)/WLC_DFS_RADAR_CHECK_INTERVAL;
		if (appvt->dfs_cac.cactime) {
			/* preism cac is enabled */
			appvt->dfs_cac.status.state = WL_DFS_CACSTATE_PREISM_CAC;
			appvt->dfs_cac.duration = appvt->dfs_cac.cactime;
			wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);
		} else {
			/* preism cac is disabled */
			wlc_dfs_cacstate_ism_set(ap);
		}

		wlc_radar_detected(ap); /* refresh detector */

	} else {
		wlc_dfs_cacstate_idle_set(ap); /* set to idle */
	}

	WL_REGULATORY(("wl%d: %s: state to %s channel %d \n",
		pub->unit, __FUNCTION__,
		wlc_dfs_cacstate_str[appvt->dfs_cac.status.state],
		CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC)));
}

void
wlc_dfs_setchanspec(wlc_ap_info_t *ap, chanspec_t chanspec)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;

	if (WL11H_AP_ENAB(appvt->wlc) && CHSPEC_IS5G(chanspec)) {
		appvt->dfs_cac.chanspec_forced = chanspec;
		WL_DFS(("dfs : next dfs channel is forced to %d%s\n",
			wf_chspec_ctlchan(chanspec), CHSPEC_IS20(chanspec) ?
			"" : (CHSPEC_SB_UPPER(chanspec) ? "U" : "L")));
	}
}

void
wlc_set_dfs_cacstate(wlc_ap_info_t *ap, int state)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;

	if (!AP_ENAB(appvt->pub))
		return;

	if (state == OFF) {
		if (appvt->dfs_cac_enabled) {
			wlc_dfs_cacstate_idle_set(ap);
			wlc_dfs_cacstate_idle(ap);
			appvt->dfs_cac_enabled = FALSE;
		}
	} else {
		if (!appvt->dfs_cac_enabled) {
			wlc_dfs_cacstate_init(ap);
			if (appvt->dfs_cac.status.state != WL_DFS_CACSTATE_IDLE)
				appvt->dfs_cac_enabled = TRUE;
		}
	}
}
#endif /* RADAR */

#ifdef BCMDBG
static int
wlc_dump_ap(wlc_ap_info_t *ap, struct bcmstrbuf *b)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;

	bcm_bprintf(b, "\n");

	bcm_bprintf(b, " shortslot_restrict %d scb_timeout %d\n",
		ap->shortslot_restrict, wlc->scb_timeout);

	bcm_bprintf(b, "tbtt %d pre-tbtt-us %u. max latency %u. block datafifo %d "
		"lazywds %d\n",
		WLCNTVAL(wlc->pub->_cnt->tbtt),
		ap->pre_tbtt_us, ap->pre_tbtt_max_lat_us,
		wlc->block_datafifo, ap->lazywds);

	return 0;
}

#ifdef RXCHAIN_PWRSAVE
static int
wlc_dump_rxchain_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;

	wlc_dump_pwrsave(&appvt->rxchain_pwrsave.pwrsave, b);

	return 0;
}
#endif /* RXCHAIN_PWRSAVE */

#ifdef RADIO_PWRSAVE
static int
wlc_dump_radio_pwrsave(wlc_ap_info_t *ap, struct bcmstrbuf *b)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;

	wlc_dump_pwrsave(&appvt->radio_pwrsave.pwrsave, b);

	return 0;
}
#endif /* RADIO_PWRSAVE */

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
static void
wlc_dump_pwrsave(wlc_pwrsave_t *pwrsave, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, " in_power_save %d\n",
		pwrsave->in_power_save);

	bcm_bprintf(b, " no: of times in power save mode %d\n",
		pwrsave->in_power_save_counter);

	bcm_bprintf(b, " power save time (in secs) %d\n",
		pwrsave->in_power_save_secs);

	return;
}
#endif /* RXCHAIN_PWRSAVE or RADIO_PWRSAVE */
#endif /* BCMDBG */

static int
wlc_ap_watchdog(void *arg)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) arg;
	wlc_ap_info_t *ap = &appvt->appub;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
#ifdef RXCHAIN_PWRSAVE
	bool done = 0;
#endif
	int idx;
	wlc_bsscfg_t *cfg;

	/* part 1 */
	if (AP_ENAB(wlc->pub)) {
		struct scb *scb;
		struct scb_iter scbiter;
		/* deauth rate limiting - enable sending one deauth every second */
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
#ifdef RXCHAIN_PWRSAVE
			if ((ap->rxchain_pwrsave_enable == RXCHAIN_PWRSAVE_ENAB_BRCM_NONHT) &&
			    !done) {
				if (SCB_ASSOCIATED(scb) && !(scb->flags & SCB_BRCM) &&
				    (scb->flags & SCB_HTCAP)) {
					if (appvt->rxchain_pwrsave.pwrsave.in_power_save) {
						wlc_stf_rxchain_set(wlc,
							appvt->rxchain_pwrsave.rxchain);
#ifdef WL11N
						/* need to restore rx_stbc HT capability
						 * after exit rxchain_pwrsave mode
						 */
						wlc_stf_exit_rxchain_pwrsave(wlc,
							appvt->rxchain_pwrsave.ht_cap_rx_stbc);
#endif /* WL11N */
						appvt->rxchain_pwrsave.pwrsave.in_power_save =
							FALSE;
					}
					appvt->rxchain_pwrsave.pwrsave.power_save_check = 0;
					done = 1;
				}
			}
#endif /* RXCHAIN_PWRSAVE */
			/* clear scb deauth. sent flag so new deauth allows to be sent */
			scb->flags &= ~SCB_DEAUTH;
		}
#ifdef RXCHAIN_PWRSAVE
		if (!done)
			appvt->rxchain_pwrsave.pwrsave.power_save_check =
				ap->rxchain_pwrsave_enable;
#endif
#ifdef RADAR
		/* Restore channels 30 minutes after radar detect */
		if (WL11H_ENAB(wlc) && wlc->radar) {
			int chan;
			for (chan = 0; chan < MAXCHANNEL; chan++) {
				if (ap->chan_blocked[chan] &&
				    ap->chan_blocked[chan] != WLC_CHANBLOCK_FOREVER) {
					ap->chan_blocked[chan]--;
					if (!ap->chan_blocked[chan]) {
						WL_REGULATORY(("\t** DFS *** Channel %d is"
							       " clean after 30 minutes\n", chan));
					}
				}
			}
		}
#endif /* RADAR */

#ifdef WLMEDIA_IPTV
	if (wlc->pub->videotag_scb_auto)
	{
		struct scb* scb_new_videotag = NULL;
		int rssi_best = -100;
		int rssi_worst = -1;
		int rssi;
		char eabuf[ETHER_ADDR_STR_LEN];
		int scb_count = 0;

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			rssi = wlc_scb_rssi(scb);
			bcm_ether_ntoa(&scb->ea, eabuf);
			if ((wlc->pub->videotag_scb_auto == 1 ||
				wlc->pub->videotag_scb_auto == 2) &&
				(rssi_best < rssi) && (rssi < -1)) {
				rssi_best = rssi;
				scb_new_videotag = scb;
			}
			else if ((wlc->pub->videotag_scb_auto == 3 ||
				wlc->pub->videotag_scb_auto == 4) &&
				(rssi_worst > rssi) && (rssi > -100)) {
				rssi_worst = rssi;
				scb_new_videotag = scb;
			}
			scb_count++;
		}
		if (scb_count <= 1) scb_new_videotag = NULL;
		if (scb_new_videotag && (scb_new_videotag != wlc->videotag_scb)) {
			wlc->videotag_scb = scb_new_videotag;
			bcm_ether_ntoa(&wlc->videotag_scb->ea, eabuf);
			printf("New videotag for %s\n", eabuf);
		}
	}
#endif /* WLMEDIA_IPTV */

	}

	/* part 2 */
	FOREACH_UP_AP(wlc, idx, cfg) {
		wlc_prot_cond_t *cond = cfg->prot_cond;

		/* update our ofdm flag based on associated STAs */
		if (cond->ofdm_assoc && !wlc_scb_scan(cfg, (SCB_HTCAP | SCB_NONERP), 0))
			cond->ofdm_assoc = FALSE;

		if (cond->ht20in40_assoc &&
		    !wlc_scb_scan(cfg, (SCB_HTCAP | SCB_IS40), SCB_HTCAP))
			cond->ht20in40_assoc = FALSE;

		if (cond->non_gf_assoc && !wlc_scb_scan(cfg, SCB_NONGF, SCB_NONGF))
			cond->non_gf_assoc = FALSE;

		if (cond->wme_apsd_assoc && !wlc_scb_scan(cfg, SCB_APSDCAP, SCB_APSDCAP))
			cond->wme_apsd_assoc = FALSE;

		if (cond->ht40intolerant_assoc &&
		    !wlc_scb_scan(cfg, SCB_HT40INTOLERANT, SCB_HT40INTOLERANT))
			cond->ht40intolerant_assoc = FALSE;

		/* update non 11n APSD association status */
		if (cond->non11n_apsd_assoc &&
		    !wlc_scb_scan(cfg, (SCB_APSDCAP | SCB_HTCAP), SCB_APSDCAP))
			cond->non11n_apsd_assoc = FALSE;

		if (wlc->band->gmode) {
			/* update our nonerp flag based on associated STAs */
			if (wlc_ap_get_prot_cond(wlc, cfg,
				OFFSETOF(wlc_prot_cond_t, nonerp_assoc)) &&
			    !wlc_scb_scan(cfg, SCB_NONERP, SCB_NONERP))
				wlc_ap_set_prot_cond(wlc, cfg,
					OFFSETOF(wlc_prot_cond_t, nonerp_assoc), FALSE);

			/* update our longslot flag based on associated STAs */
			if (wlc_ap_get_prot_cond(wlc, cfg,
				OFFSETOF(wlc_prot_cond_t, longslot_assoc)) &&
			    !wlc_scb_scan(cfg, SCB_LONGSLOT, SCB_LONGSLOT))
				wlc_ap_set_prot_cond(wlc, cfg,
					OFFSETOF(wlc_prot_cond_t, longslot_assoc), FALSE);

			/* update our long preamble flag based on associated STAs */
			if (wlc_ap_get_prot_cond(wlc, cfg,
				OFFSETOF(wlc_prot_cond_t, longpre_assoc)) &&
			    !wlc_scb_scan(cfg, SCB_SHORTPREAMBLE, 0))
				wlc_ap_set_prot_cond(wlc, cfg,
					OFFSETOF(wlc_prot_cond_t, longpre_assoc), FALSE);
		}
#ifdef WL11N
		/* OBSS COEX check for possible upgrade
		 * only do it when it is currently 20MHz
		 */
		if (COEX_ACTIVE(wlc) && CHSPEC_IS20(cfg->current_bss->chanspec)) {
			uint32 delay_val;
			/* if any 40 intolerant still associated, don't upgrade */
			if (cond->ht40intolerant_assoc)
				wlc_ht_coex_update_fid_time(wlc);
			else {
				delay_val = wlc->obss->ap_params.bss_widthscan_interval *
				        wlc->obss->ap_params.chanwidth_transition_dly;
				if ((wlc->pub->now - wlc->obss->fid_time) > delay_val) {
					/* The AP udates the fid_time everytime a new trigger
					 * happens. If we get here, we are good to upgrade.
					 */
					wlc_ht_coex_update_permit(wlc, TRUE);
					wlc_ht_coex_switch_bw(wlc, FALSE);
				}
			}
		}
#endif /* WL11N */
	}

	/* part 3 */
	if (AP_ENAB(wlc->pub)) {
#ifdef WDS
		wlc_ap_wds_timeout(ap);

#ifdef WLAFTERBURNER
		/* update nonabcap_assoc based on not-ab-sta links status */
		wlc->protection->nonabcap_assoc = wlc_nonabcap_assoc_upd(ap);

		/* update the ab_wds_timeout based on previously processed stuff */
		wlc_update_ab_wds_timeout(ap);

		/* update our afterburner mode
		 * No need to update for override values of ON, OFF because
		 * we do that at the time of selection
		 *
		 *
		 */
		if (AP_ONLY(wlc->pub) && wlc->afterburner_override == AUTO) {
			bool ab = (wlc->ab_wds_timeout > AB_WDS_TIMEOUT_MIN) ? FALSE : TRUE;

#ifdef WL11N
			if (wlc->afterburner == ab) {
				/* no change needed: just clear counter */
				wlc->guard_count = 0;

			} else if (wlc->protection->nmode_user == 0) {
				/* normal behavior if nmode not selected */
				wlc_afterburner(wlc, ab);

			} else {
				/* Asymmetrical:
				 * If we can't do afterburner switch out immediately
				 * If we can wait AB_GUARDCOUNT to switch into afterburner
				 */
				wlc->guard_count++;
				if (ab == FALSE || wlc->guard_count >= AB_GUARDCOUNT) {
					wlc_switch_ab_11n(wlc, ab, FALSE);
				}
			}
#else /* WL11N */
			if (wlc->afterburner != ab)
				wlc_afterburner(wlc, ab);
#endif /* WL11N */
		}
#endif /* WLAFTERBURNER */
#endif /* WDS */

		/* process age-outs only when not in scan progress */
		if (!SCAN_IN_PROGRESS(wlc->scan)) {
			/* age out ps queue packets */
			wlc_apps_psq_ageing(wlc);

			/* age out stas */
			if ((wlc->scb_timeout &&
			     ((pub->now % wlc->scb_timeout) == 0)) ||
			    wlc->reprobe_scb) {
				wlc_ap_stas_timeout(ap);
				wlc->reprobe_scb = FALSE;
			}
		}

		if (WLC_CHANIM_ENAB(wlc) && WLC_CHANIM_MODE_ACT(wlc->chanim_info))
			wlc_chanim_upd_act(wlc);

#ifdef APCS
		/* periodic auto channel selection timer */
		if (!WL11H_ENAB(wlc) &&
		    /* this channel selection disabled for 11h-enabled AP */
		    AP_ENAB(pub) &&
		    appvt->cs_scan_timer &&
		    !(wlc->block_datafifo & DATA_BLOCK_QUIET) &&
		    (((wlc->pub->now % appvt->cs_scan_timer) == 0) ||
		     wlc->cs_scan_ini || WLC_CHANIM_ACT(wlc->chanim_info))) {
			wlc_cs_scan_timer(wlc);
		}
#endif /* APCS */

#ifdef RXCHAIN_PWRSAVE
		/* Do the wl power save checks */
		if (appvt->rxchain_pwrsave.pwrsave.power_save_check)
		    wlc_pwrsave_mode_check(ap, PWRSAVE_RXCHAIN);

		if (appvt->rxchain_pwrsave.pwrsave.in_power_save)
		    appvt->rxchain_pwrsave.pwrsave.in_power_save_secs++;
#endif

#ifdef RADIO_PWRSAVE
		if (appvt->radio_pwrsave.pwrsave.power_save_check)
		    wlc_pwrsave_mode_check(ap, PWRSAVE_RADIO);

		if (appvt->radio_pwrsave.pwrsave.in_power_save)
		    appvt->radio_pwrsave.pwrsave.in_power_save_secs++;
#endif
	}

	/* Part 4 */
	FOREACH_UP_AP(wlc, idx, cfg) {
		/* update brcm_ie and our beacon */
		if (wlc_bss_update_brcm_ie(wlc, cfg)) {
			WL_APSTA_BCN(("wl%d.%d: wlc_watchdog() calls wlc_update_beacon()\n",
			              pub->unit, WLC_BSSCFG_IDX(cfg)));
			wlc_bss_update_beacon(wlc, cfg);
			wlc_bss_update_probe_resp(wlc, cfg, TRUE);
		}
	}

	return BCME_OK;
}

#if defined(MBSS)
/* Assumes SW beaconing active */
#define BSS_BEACONING(cfg) ((cfg) && BSSCFG_AP(cfg) && (cfg)->up)

/* MBSS4 MI_TBTT and MI_DTIM_TBTT handler */
int
wlc_ap_mbss4_tbtt(wlc_info_t *wlc, uint32 macintstatus)
{
	int i, idx;
	wlc_bsscfg_t *cfg;
	uint16 beacon_count = 0;
	wlc_pkt_t pkt;
	bool dtim;
	uint32 delay_val;
#ifdef RADIO_PWRSAVE
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) wlc->ap;
#endif /* RADIO_PWRSAVE */

	dtim = ((macintstatus & MI_DTIM_TBTT) != 0);
	/* Update our dtim count and determine which BSS config will beacon first */
	if (dtim) {
		wlc->cur_dtim_count = 0x00;
		/* disabled beacon rotate when beacon_bssidx < 0  */
		if (wlc->beacon_bssidx >= 0) {
			/* Bump the "starting" bss index up for bss rotation */
			for (i = 0; i < WLC_MAXBSSCFG; i++) {
				if (++wlc->beacon_bssidx >= WLC_MAXBSSCFG) {
					wlc->beacon_bssidx = 0;
				}
				if (BSS_BEACONING(wlc->bsscfg[wlc->beacon_bssidx])) {
					/* Found the next beaconing BSS index; break */
					break;
				}
			}
		}
	}
	else  {
		if (wlc->cur_dtim_count)
			wlc->cur_dtim_count--;
		else
			wlc->cur_dtim_count = wlc->default_bss->dtim_period - 1;
	}

	/* If we've taken too long to get beacons ready, don't bother queuing them */
	delay_val = (R_REG(wlc->osh, &wlc->regs->tsf_timerlow) -
	         wlc->last_tbtt_us);
	if (delay_val > wlc->ap->pre_tbtt_max_lat_us) {
		WLCNTINCR(wlc->pub->_cnt->late_tbtt_dpc);
		WL_MBSS(("wl%d: ERROR: TBTT latency: %u; skipping bcn\n",
		         wlc->pub->unit, delay_val));
		return 0;
	}

#ifdef RADIO_PWRSAVE
	/* just out of pwr save with a need to skip bcn in level 1/2. */
	if (!dtim && appvt->radio_pwrsave.cncl_bcn) {
		WL_INFORM(("wl%d: radio pwrsave skipping bcn.\n", wlc->pub->unit));
		return 0;
	}
#endif /* RADIO_PWRSAVE */

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		WL_MBSS(("wl%d: WARNING: MBSS Not beaconing due to scan in progress.\n",
		         wlc->pub->unit));
		return 0;
	}

	for (i = 0; i < WLC_MAXBSSCFG; i++) {
		idx = i;
		if (wlc->beacon_bssidx >= 0)
			idx = (i + wlc->beacon_bssidx) % WLC_MAXBSSCFG;
		cfg = wlc->bsscfg[idx];
		if (!BSS_BEACONING(cfg)) {
			continue; /* Skip cfgs not present or not AP or not up */
		}
		ASSERT(cfg->bcn_template->latest_idx >= 0);
		ASSERT(cfg->bcn_template->latest_idx < WLC_SPT_COUNT_MAX);
		if (cfg->bcn_template->in_use_bitmap != 0) {
			WLCNTINCR(wlc->pub->_cnt->bcn_template_not_ready);
#if defined(BCMDBG_MBSS_PROFILE)
			if (cfg->bcn_tx_done) {
				WLCNTINCR(wlc->pub->_cnt->bcn_template_not_ready_done);
			}
#endif
			continue;
		}

		pkt = SPT_LATEST_PKT(cfg->bcn_template);
		ASSERT(pkt != NULL);

		/* Update DTIM count */
		BCN_TIM_DTIM_COUNT_SET(cfg->bcn_template->tim_ie, wlc->cur_dtim_count);

		/*
		 * Update BCMC flag in the beacon.
		 * At this point, the driver has not yet written the last FID SHM locations;
		 * so either bcmc_fid or bcmc_fid_shm may be indicate pkts in the BC/MC fifo
		 */
		if (BCMC_PKTS_QUEUED(cfg)) {
			BCN_TIM_BCMC_FLAG_SET(cfg->bcn_template->tim_ie);
		} else {
			BCN_TIM_BCMC_FLAG_RESET(cfg->bcn_template->tim_ie);
		}

		if (dma_txfast(wlc->hw->di[TX_ATIM_FIFO], pkt, TRUE) < 0) {
			WLCNTINCR(cfg->cnt->bcn_tx_failed);
		} else {
			++beacon_count;
			cfg->bcn_template->in_use_bitmap |= (1 << cfg->bcn_template->latest_idx);
#if defined(BCMDBG_MBSS_PROFILE)
			cfg->bcn_tx_done = FALSE;
#endif /* BCMDBG_MBSS_PROFILE */
#if defined(WLC_SPT_DEBUG)
			cfg->bcn_template->tx_count++;
#endif /* WLC_SPT_DEBUG */
		}
	}
	wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, beacon_count);

	if (dtim) {
		wlc_apps_update_bss_bcmc_fid(wlc);
	}

	return 0;
}

/* MBSS16 MI_TBTT and MI_DTIM_TBTT handler */
int
wlc_ap_mbss16_tbtt(wlc_info_t *wlc, uint32 macintstatus)
{
	bool dtim;
	int cfgidx;
	int ucidx;
	wlc_bsscfg_t *cfg = NULL;
	uint16 beacon_count = 0;
	uint16 dtim_map = 0;
#ifdef RADIO_PWRSAVE
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) wlc->ap;
#endif /* RADIO_PWRSAVE */

#ifndef WLC_HIGH_ONLY
	{
		uint32 delay_val;
		/* If we've taken too long to get ready, skip */
		delay_val = (R_REG(wlc->osh, &wlc->regs->tsf_timerlow) - wlc->last_tbtt_us);
		if (delay_val > wlc->ap->pre_tbtt_max_lat_us) {
			WLCNTINCR(wlc->pub->_cnt->late_tbtt_dpc);
			WL_MBSS(("wl%d: ERROR: TBTT latency: %u; skipping bcn update\n",
			         wlc->pub->unit, delay_val));
			/* No beacons: didn't update */
			wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, 0);
			return 0;
		}
	}
#endif
	dtim = ((macintstatus & MI_DTIM_TBTT) != 0);

#ifdef RADIO_PWRSAVE
	if (!dtim && appvt->radio_pwrsave.cncl_bcn) {
		wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, 0);
		WL_INFORM(("wl%d: radio pwrsave skipping bcn.\n", wlc->pub->unit));
		return 0;
	}
#endif /* RADIO_PWRSAVE */

	/* Traverse the bsscfg's
	 * create a count of "active" bss's
	 *
	 * if we're at a DTIM:
	 * create a DTIM map,  push "last" bc/mc fid's to shm
	 *
	 * if a beacon has been modified push to shm
	 */
	for (cfgidx = 0; cfgidx < WLC_MAXBSSCFG; cfgidx++) {
		cfg = wlc->bsscfg[cfgidx];
		if (!BSS_BEACONING(cfg))
			continue;

		ASSERT(cfg->bcn_template->latest_idx >= 0);
		ASSERT(cfg->bcn_template->latest_idx < WLC_SPT_COUNT_MAX);

		++beacon_count;

		ucidx = WLC_BSSCFG_UCIDX(cfg);
		ASSERT(ucidx != WLC_BSSCFG_IDX_INVALID);
		/* Update BCMC flag in the beacon. */
		if (dtim && (cfg->bcmc_fid != INVALIDFID)) {
			uint fid_addr;

			dtim_map |= NBITVAL(ucidx);
			fid_addr = SHM_MBSS_BC_FID_ADDR16(ucidx);
			wlc_write_shm((wlc), fid_addr, cfg->bcmc_fid);
			BCMC_FID_SHM_COMMIT(cfg);
		}
		/* Update the HW beacon template */
		wlc_ap_mbss16_write_beacon(wlc, cfg);

	} /* cfgidx loop */

	wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, beacon_count);
	wlc_write_shm(wlc, SHM_MBSS_BSSID_NUM, beacon_count);

	if (dtim)
		wlc_write_shm(wlc, SHM_MBSS_BC_BITMAP, dtim_map);

	return 0;
}

/* Write the BSSCFG's beacon template into HW */
static void
wlc_ap_mbss16_write_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_pkt_t pkt;
	uint32 ucidx;
	int start;
	uint16 len;
	uint8 * pt;

	ucidx = WLC_BSSCFG_UCIDX(cfg);
	ASSERT(ucidx != (uint32)WLC_BSSCFG_IDX_INVALID);

	ASSERT(cfg->bcn_template->latest_idx >= 0);
	ASSERT(cfg->bcn_template->latest_idx < WLC_SPT_COUNT_MAX);

	pkt = SPT_LATEST_PKT(cfg->bcn_template);
	ASSERT(pkt != NULL);

	/* beacon */
	if (cfg->bcn_template->bcn_modified == TRUE) {

		WL_MBSS(("%s: wl%d.%d\n", __FUNCTION__, wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));

		start = T_BCN0_TPL_BASE + (ucidx * BCN_TMPL_LEN);
		pt = ((uint8 *)(PKTDATA(wlc->osh, pkt)) + D11_TXH_LEN);
		len = PKTLEN(wlc->osh, pkt) - D11_TXH_LEN;

		ASSERT(len <= BCN_TMPL_LEN);

		wlc_write_template_ram(wlc, start, (len + 3) & (~3), pt);

		/* bcn len */
		wlc_write_shm(wlc, SHM_MBSS_BCNLEN0 + (2 * ucidx), len);
		wlc_mbss16_setup(wlc, cfg);
		cfg->bcn_template->bcn_modified = FALSE;
	}
}

/* Write the BSSCFG's probe response template into HW, suspend MAC if requested */
void
wlc_ap_mbss16_write_prbrsp(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool suspend)
{
	wlc_pkt_t pkt;
	uint32 ucidx;
	int start;
	uint16 len;
	uint8 * pt;

	ucidx = WLC_BSSCFG_UCIDX(cfg);
	ASSERT(ucidx != (uint32)WLC_BSSCFG_IDX_INVALID);

	pkt = cfg->probe_template;
	ASSERT(pkt != NULL);

	WL_MBSS(("%s: wl%d.%d %smodified %d\n", __FUNCTION__, wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	         suspend ? "w/suspend " : "", cfg->prb_modified));

	/* probe response */
	if (cfg->prb_modified == TRUE) {
		if (suspend)
			wlc_suspend_mac_and_wait(wlc);

		start = MBSS_PRS_BLKS_START(wlc->max_ap_bss) + (ucidx * BCN_TMPL_LEN);

		pt = ((uint8 *)(PKTDATA(wlc->osh, pkt)) + D11_TXH_LEN);
		len = PKTLEN(wlc->osh, pkt) - D11_TXH_LEN;

		ASSERT(len <= BCN_TMPL_LEN);

		wlc_write_template_ram(wlc, start, (len + 3) & (~3), pt);
		/* probe response len */
		wlc_write_shm(wlc, SHM_MBSS_PRSLEN0 + (2 * ucidx), len);

#ifdef WLLPRS
		if (N_ENAB(wlc->pub)) {
			wlc_pkt_t prspkt;
			uint16 lgcyprs_len_ptr;

			prspkt = cfg->lprs_template;
			ASSERT(prspkt != NULL);

			/* 11g probe resp, which follows the ht probe resp */
			start = MBSS_PRS_BLKS_START(wlc->max_ap_bss) +
				(wlc->max_ap_bss * BCN_TMPL_LEN) + (ucidx * LPRS_TMPL_LEN);

			pt = ((uint8 *)(PKTDATA(wlc->osh, prspkt)) + D11_TXH_LEN);
			len = PKTLEN(wlc->osh, prspkt) - D11_TXH_LEN;

			ASSERT(len <= LPRS_TMPL_LEN);

			wlc_write_template_ram(wlc, start, (len + 3) & (~3), pt);

			lgcyprs_len_ptr = wlc_read_shm(wlc, SHM_MBSS_BC_FID3);

			wlc_write_shm(wlc, ((lgcyprs_len_ptr + ucidx) * 2), len);
		}
#endif /* WLLPRS */

		cfg->prb_modified = FALSE;

		if (suspend)
			wlc_enable_mac(wlc);
	}
}

void
wlc_mbss16_updssid(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint32 start;
	uint16 val;
	uint32 ssidlen = cfg->SSID_len;
	uint32 swplen;
	int8 ucidx = WLC_BSSCFG_UCIDX(cfg);
	uint8 ssidbuf[DOT11_MAX_SSID_LEN];

	ASSERT((ucidx >= 0) && (ucidx <= wlc->mbss_ucidx_mask));


	/* push ssid, ssidlen out to ucode Search Engine */
	start = SHM_MBSS_SSIDSE_BASE_ADDR + (ucidx * SHM_MBSS_SSIDSE_BLKSZ);
	/* search mem length field is always little endian */
	swplen = htol32(ssidlen);
	/* invent new function like wlc_write_shm using OBJADDR_SRCHM_SEL */
	wlc_bmac_copyto_objmem(wlc->hw, start, &swplen, SHM_MBSS_SSIDLEN_BLKSZ, OBJADDR_SRCHM_SEL);

	bzero(ssidbuf, DOT11_MAX_SSID_LEN);
	bcopy(cfg->SSID, ssidbuf, cfg->SSID_len);

	start += SHM_MBSS_SSIDLEN_BLKSZ;
	wlc_bmac_copyto_objmem(wlc->hw, start, ssidbuf, SHM_MBSS_SSID_BLKSZ, OBJADDR_SRCHM_SEL);


	start = SHM_MBSS_SSIDLEN0 + (ucidx & 0xFE);
	val = wlc_read_shm(wlc, start);
	/* set bit indicating closed net if appropriate */
	if (cfg->closednet_nobcnssid)
		ssidlen |= SHM_MBSS_CLOSED_NET;

	if (ucidx & 0x01) {
		val &= 0xff;
		val |= ((uint8)ssidlen << 8);

	} else {
		val &= 0xff00;
		val |= (uint8)ssidlen;
	}

	wlc_write_shm(wlc, start, val);
}

static void
wlc_mbss16_setup(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint8 *bcn;
	void *pkt;
	uint16 tim_offset;

	/* find the TIM elt offset in the bcn template, push to shm */
	pkt = SPT_LATEST_PKT(cfg->bcn_template);
	bcn = (uint8 *)(PKTDATA(wlc->osh, pkt));
	tim_offset = (uint16)(cfg->bcn_template->tim_ie - bcn);
	/* we want it less the actual ssid length */
	tim_offset -= cfg->SSID_len;
	/* and less the D11_TXH_LEN too */
	tim_offset -= D11_TXH_LEN;

	wlc_write_shm(wlc, M_TIMBPOS_INBEACON, tim_offset);
}

#undef BSS_BEACONING

#endif /* MBSS */

#ifdef WL11N
void
wlc_ht_coex_switch_bw(wlc_info_t *wlc, bool downgrade)
{
	chanspec_t chanspec, newchanspec;
	wlc_bsscfg_t *cfg = wlc->cfg;
	wlc_csa_t *csa = cfg->csa;

	if (!cfg->associated)
		return;

	chanspec = cfg->current_bss->chanspec;

	ASSERT(cfg->BSS);
	ASSERT(wlc->obss->coex_enab);

	if ((!CHSPEC_IS40(chanspec) && downgrade) ||
	    (CHSPEC_IS40(chanspec) && !downgrade)) {
		WL_COEX(("wl%d: %s: cannot %s already at BW %s\n", wlc->pub->unit, __FUNCTION__,
			downgrade ? "downgrade":"upgrade",
			(CHSPEC_IS40(chanspec)? "40MHz":"20MHz")));
		return;
	}

	/* return if  a channel switch is already scheduled */
	if (csa->spect_state & NEED_TO_SWITCH_CHANNEL)
		return;

	/* upgrade AP only if previous has been downgraded */
	if (!downgrade && !CHSPEC_IS40(wlc->default_bss->chanspec))
		return;

	if (downgrade) {
		newchanspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan(chanspec));
		WL_COEX(("wl%d: wlc_ht_coex_downgrad_bw\n", wlc->pub->unit));
	}
	else {
		/* If the default bss chanspec is now invalid then pick a valid one */
		if (!wlc_valid_chanspec(wlc->cmi, wlc->default_bss->chanspec))
			wlc->default_bss->chanspec = wlc_default_chanspec(wlc->cmi, TRUE);
		newchanspec = wlc->default_bss->chanspec;
	}

	if (newchanspec == chanspec)
		return;

	WL_COEX(("wl%d: %s: switching chanspec 0x%x to new chanspec 0x%x\n",
		wlc->pub->unit, __FUNCTION__, chanspec, newchanspec));

	csa->csa.mode = DOT11_CSA_MODE_ADVISORY;
	/* downgrade/upgrade with same control channel, do it immediately */
	csa->csa.count =
		(wf_chspec_ctlchan(chanspec) == wf_chspec_ctlchan(newchanspec)) ? 0 : 10;
	csa->csa.chspec = newchanspec;
	csa->csa.reg = wlc_get_regclass(wlc->cmi, newchanspec);
	if (csa->csa.count == 0) {
		wlc_do_chanswitch(cfg, newchanspec);
	} else {
		/* dpc handles NEED_TO_UPDATE_BCN, NEED_TO_SWITCH_CHANNEL */
		csa->spect_state |= (NEED_TO_UPDATE_BCN | NEED_TO_SWITCH_CHANNEL);
		/* block data traffic but allow control */
		if (WL11H_ENAB(wlc))
			wlc->block_datafifo |= DATA_BLOCK_QUIET;
	}

	wlc_bss_update_beacon(wlc, cfg);
	wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	wlc_send_action_switch_channel(wlc->ap, cfg);
#if NCONF
	/* Update edcrs on bw change */
	wlc_bmac_ifsctl_edcrs_set(wlc->hw,
#ifdef WLAFTERBURNER
	wlc->afterburner,
#else
	FALSE,
#endif
	WLCISHTPHY(wlc->band));
#endif /* NCONF */
}

void
wlc_ht_coex_update_permit(wlc_info_t *wlc, bool permit)
{
	ASSERT(COEX_ACTIVE(wlc));
	wlc->obss->coex_permit = permit;
}

void
wlc_ht_coex_update_fid_time(wlc_info_t *wlc)
{
	ASSERT(COEX_ACTIVE(wlc));
	wlc->obss->fid_time = wlc->pub->now;
}

/* check for coex trigger event A (TE-A)
 * a non-ht beacon
 */
bool
wlc_ht_ap_coex_tea_chk(wlc_info_t *wlc, ht_cap_ie_t *cap_ie)
{
	wlc_bsscfg_t *cfg = wlc->cfg;

	if (!CHSPEC_IS2G(wlc->home_chanspec))
		return FALSE;

	/* get a non-ht beacon when the bss is up */
	if (!cap_ie && cfg->associated && cfg->BSS &&
		!(wlc->obss->coex_te_mask & COEX_MASK_TEA))
		return TRUE;

	return FALSE;
}

/*
 * process trigger event B/C (TE-B, TE-C)
 */
void
wlc_ht_ap_coex_tebc_proc(wlc_info_t *wlc)
{
	if (!CHSPEC_IS2G(wlc->home_chanspec))
		return;

	wlc_ht_coex_update_fid_time(wlc);

	if (CHSPEC_IS40(wlc->home_chanspec))
		wlc_ht_coex_switch_bw(wlc, TRUE);
}

/* check for coex trigger event D (TE-D)
 * a 20/40 BSS coex frame
 */
bool
wlc_ht_ap_coex_ted_chk(wlc_info_t *wlc, bcm_tlv_t *bss_chanlist_tlv, uint8 coex_bits)
{
	dot11_obss_chanlist_t *chanlist_ie = (dot11_obss_chanlist_t *)bss_chanlist_tlv;
	uint8 chlist_len, chan, i;
	uint8 ch_min, ch_max, ch_ctl;

	WL_TRACE(("wl%d: wlc_ht_upd_coex_map\n", wlc->pub->unit));

	ASSERT(COEX_ACTIVE(wlc));

	if (!CHSPEC_IS2G(wlc->home_chanspec))
		return FALSE;

	if (!chanlist_ie) {
		WL_ERROR(("wl%d: %s: 20/40 BSS Intolerant Channel Report IE NOT found\n",
			wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	/* minimum len is 3 bytes (IE Hdr + regulatory class) */
	if (bss_chanlist_tlv->len < DOT11_OBSS_CHANLIST_FIXED_LEN) {
		WL_ERROR(("wl%d: %s: Invalid 20/40 BSS Intolerant Channel Report IE len %d\n",
			wlc->pub->unit, __FUNCTION__, bss_chanlist_tlv->len));
		return FALSE;
	}
	/* this is a trigger irrelevant of the channel in use  */
	if (coex_bits & ~WL_COEX_INFO_REQ) {
	  return TRUE;
	}
	/* exclude regulatory class */
	chlist_len = chanlist_ie->len - DOT11_OBSS_CHANLIST_FIXED_LEN;

	WL_COEX(("wl%d: wlc_ht_upd_coex_map : %d 20MHZ channels\n", wlc->pub->unit, chlist_len));

	/* retrieve exclusion range and ctl channel */
	wlc_ht_coex_exclusion_range(wlc, &ch_min, &ch_max, &ch_ctl);
	/* If we do not do 40 mhz */
	if (!ch_min)
	  return FALSE;
	for (i = 0; i < chlist_len; i++) {
		chan = chanlist_ie->chanlist[i];
		if (chan <= CH_MAX_2G_CHANNEL) {
		  /*  Here we should store the date for each channel
		      It could be used in the future for auto channel
		      purposes.
		   */

		  WL_COEX(("wl%d: %s: chan %d flag 0x%x\n", wlc->pub->unit, __FUNCTION__,
		           chan, WL_COEX_WIDTH20));
		  if (chan >= ch_min && chan <= ch_max && chan != ch_ctl) {
		    /* trigger */
		    return TRUE;
		  }
		}
	}
	return FALSE;
}
#endif /* WL11N */

int
wlc_ap_ioctl(wlc_ap_info_t *ap, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t *) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;
	int val, *pval;
	bool bool_val;
	int bcmerror = 0;
#ifdef WDS
	uint i;
#endif
	struct maclist *maclist;
	wlc_bsscfg_t *bsscfg;
	struct scb_iter scbiter;
	struct scb *scb = NULL;

	/* update bsscfg pointer */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;

	/* default argument is generic integer */
	pval = arg ? (int *) arg : NULL;
	val = pval ? *pval : 0;
	/* bool conversion to avoid duplication below */
	bool_val = val != 0;

	switch (cmd) {

#ifdef WDS
	case WLC_SET_WDSLIST:
		ASSERT(arg != NULL);
		maclist = (struct maclist *) arg;
		if (maclist->count > (uint) wlc->pub->tunables->maxscb) {
			bcmerror = BCME_RANGE;
			break;
		}

		if (len < (int)(OFFSETOF(struct maclist, ea) + maclist->count * ETHER_ADDR_LEN)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}

		if (WIN7_OS(wlc->pub)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		/* Mark current wds nodes for reclamation */
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (scb->wds)
				scb->permanent = FALSE;
		}

		if (maclist->count == 0)
			appvt->wdsactive = FALSE;

		/* set new WDS list info */
		for (i = 0; i < maclist->count; i++) {
			if (!(scb = wlc_scblookup(wlc, &maclist->ea[i]))) {
				bcmerror = BCME_NOMEM;
				break;
			}
			/* force ownership */
			wlc_scb_set_bsscfg(scb, bsscfg);

			bcmerror = wlc_wds_create(wlc, scb, 0);
			if (bcmerror) {
				wlc_scbfree(wlc, scb);
				break;
			}

			/* WDS creation was successful so mark the scb permanent and
			 * note that WDS is active
			 */
			appvt->wdsactive = TRUE;
		}

		/* free (only) stale wds entries */
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (scb->wds && !scb->permanent)
				wlc_scbfree(wlc, scb);
		}

#ifdef STA
		wlc_radio_mpc_upd(wlc);
#endif

		/* if we are "associated" as an AP, we have already founded the BSS
		 * and adjusted aCWmin. If not associated, then we need to adjust
		 * aCWmin for the WDS link
		 */
		if (pub->up && !pub->associated && BAND_2G(wlc->band->bandtype)) {
			wlc_suspend_mac_and_wait(wlc);

			if (maclist->count > 0)
				/* Update aCWmin based on basic rates. */
				wlc_cwmin_gphy_update(wlc, &bsscfg->current_bss->rateset, TRUE);
			else
				/* Unassociated gphy CWmin */
				wlc_set_cwmin(wlc, APHY_CWMIN);

			wlc_enable_mac(wlc);
		}
		break;

	case WLC_GET_WDSLIST:
		ASSERT(arg != NULL);
		maclist = (struct maclist *) arg;
		/* count WDS stations */
		val = 0;
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (scb->wds)
				val++;
		}
		if (maclist->count < (uint)val) {
			bcmerror = BCME_RANGE;
			break;
		}
		if (len < ((int)maclist->count - 1)* (int)sizeof(struct ether_addr)
			+ (int)sizeof(struct maclist)) {
			bcmerror = BCME_BUFTOOSHORT;
			break;
		}
		maclist->count = 0;

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (scb->wds)
				bcopy((void*)&scb->ea, (void*)&maclist->ea[maclist->count++],
					ETHER_ADDR_LEN);
		}
		ASSERT(maclist->count == (uint)val);
		break;

	case WLC_GET_LAZYWDS:
		ASSERT(pval != NULL);
		*pval = (int) ap->lazywds;
		break;

	case WLC_SET_LAZYWDS:
		ap->lazywds = bool_val;
		if (wlc->aps_associated && wlc_update_brcm_ie(wlc)) {
			WL_APSTA_BCN(("wl%d: WLC_SET_LAZYWDS -> wlc_update_beacon()\n",
				pub->unit));
			wlc_update_beacon(wlc);
			wlc_update_probe_resp(wlc, TRUE);
		}
		break;

	case WLC_WDS_GET_REMOTE_HWADDR:
		if (wlcif == NULL || wlcif->type != WLC_IFTYPE_WDS) {
			WL_ERROR(("invalid interface type for WLC_WDS_GET_REMOTE_HWADDR\n"));
			bcmerror = BCME_NOTFOUND;
			break;
		}
		ASSERT(arg != NULL);
		bcopy(&wlcif->u.scb->ea, arg, ETHER_ADDR_LEN);
		break;
#endif /* WDS */

	case WLC_GET_SHORTSLOT_RESTRICT:
		if (AP_ENAB(pub)) {
			ASSERT(pval != NULL);
			*pval = ap->shortslot_restrict;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_SET_SHORTSLOT_RESTRICT:
		if (AP_ENAB(pub))
			ap->shortslot_restrict = bool_val;
		else
			bcmerror = BCME_NOTAP;
		break;

#ifdef BCMDBG
	case WLC_GET_IGNORE_BCNS:
		if (AP_ENAB(pub)) {
			ASSERT(pval != NULL);
			*pval = wlc->ignore_bcns;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_SET_IGNORE_BCNS:
		if (AP_ENAB(pub))
			wlc->ignore_bcns = bool_val;
		else
			bcmerror = BCME_NOTAP;
		break;
#endif /* BCMDBG */

	case WLC_GET_SCB_TIMEOUT:
		if (AP_ENAB(pub)) {
			ASSERT(pval != NULL);
			*pval = wlc->scb_timeout;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_SET_SCB_TIMEOUT:
		if (AP_ENAB(pub))
			wlc->scb_timeout = val;
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_GET_ASSOCLIST:
		ASSERT(arg != NULL);
		maclist = (struct maclist *) arg;

		/* returns a list of STAs associated with a specific bsscfg */
		if (len < (int)(sizeof(maclist->count) + (maclist->count * ETHER_ADDR_LEN))) {
			bcmerror = BCME_BADARG;
			break;
		}
		val = 0;
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if ((SCB_ASSOCIATED(scb)) && (scb->bsscfg == bsscfg)) {
				val++;
				if (maclist->count >= (uint)val) {
					bcopy((void*)&scb->ea, (void*)&maclist->ea[val-1],
					ETHER_ADDR_LEN);
				} else {
					bcmerror = BCME_BUFTOOSHORT;
					break;
				}
			}

		}
		if (!bcmerror)
			maclist->count = val;
		break;

	case WLC_TKIP_COUNTERMEASURES:
		if (BSSCFG_AP(bsscfg) && WSEC_TKIP_ENABLED(bsscfg->wsec))
			bsscfg->tkip_countermeasures = (val != 0);
		else
			bcmerror = BCME_BADARG;
		break;

#ifdef APCS
	case WLC_GET_CHANNEL_SEL:
		if (pub->up && AP_ENAB(pub) &&
			!pub->associated && !SCAN_IN_PROGRESS(wlc->scan)) {
			ASSERT(pval != NULL);
			*pval = wlc->chanspec_selected;
		}
		else {
			bcmerror = BCME_BADARG;
		}
		break;

	case WLC_START_CHANNEL_SEL:
		/* irrelevant to 802.11h-enabled AP */
		if (WL11H_AP_ENAB(wlc)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		if (!pub->up || !AP_ENAB(pub) || pub->associated ||
		    SCAN_IN_PROGRESS(wlc->scan)) {
			bcmerror = BCME_BADARG;
			break;
		}

		ASSERT(arg != NULL);
		bcmerror = wlc_cs_scan_start(wlc, arg, TRUE, FALSE, wlc->band->bandtype,
		                             APCS_IOCTL, NULL, NULL);

		break;

	case WLC_SET_CS_SCAN_TIMER:
		if (AP_ENAB(pub)) {
			/* for 802.11h-enabled AP, cs_scan is disallowed */
			appvt->cs_scan_timer = WL11H_ENAB(wlc) ? 0 : val * 60;
		}
		else
			bcmerror = BCME_NOTAP;
		break;

	case WLC_GET_CS_SCAN_TIMER:
		ASSERT(pval != NULL);
		*pval = appvt->cs_scan_timer / 60;
		break;
#endif /* APCS */

#ifdef WDS
	case WLC_WDS_GET_WPA_SUP: {
		uint8 sup;
		ASSERT(pval != NULL);
		bcmerror = wlc_wds_wpa_role_get(ap, (struct ether_addr *)pval, &sup);
		if (!bcmerror)
			*pval = sup;
		break;
	}
#endif /* WDS */

#ifdef RADAR
	case WLC_SET_RADAR: {
		wlcband_t *band5G;

		if (val < 0 || val > WL_RADAR_SIMULATED) {
			bcmerror = BCME_RANGE;
			break;
		}

		/*
		 * WL_RADAR_SIMULATED is required for Wi-Fi testing.
		 */

		/* Radar must be enabled to pull test trigger */
		if (val == WL_RADAR_SIMULATED) {
			if (wlc->radar != 1) {
				bcmerror = BCME_BADARG;
				break;
			}

			/* Can't do radar detect on non-radar channel */
			if (wlc_radar_chanspec(wlc->cmi, bsscfg->current_bss->chanspec) != TRUE) {
				bcmerror = BCME_BADCHAN;
				break;
			}
			bsscfg->csa->spect_state |= RADAR_SIM;
			break;
		}

		if (wlc->radar == (uint32)val)
			break;

		/* Check there is a 5G band available */
		if (BAND_5G(wlc->band->bandtype)) {
			band5G = wlc->band;
		} else if (NBANDS(wlc) > 1 &&
			BAND_5G(wlc->bandstate[OTHERBANDUNIT(wlc)]->bandtype)) {
			band5G = wlc->bandstate[OTHERBANDUNIT(wlc)];
		} else {
			bcmerror = BCME_BADBAND;
			break;
		}

		/* bcmerror if APhy rev 3+ support in any bandunit */
		if (WLCISAPHY(band5G) && AREV_LT(band5G->phyrev, 3)) {
			bcmerror = BCME_UNSUPPORTED;
			break;
		}

		wlc->radar = (uint32)val;

		wlc_phy_radar_detect_enable(wlc->band->pi, wlc->radar != 0);

		/* if we are not currently on the APhy, then radar detect
		 * will be initialized in the phy init
		 */

		break;
	}

	case WLC_GET_RADAR:
		ASSERT(pval != NULL);
		*pval = wlc->radar;
		break;
#endif /* RADAR */

	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

	return (bcmerror);
}

static int
wlc_ap_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) hdl;
	wlc_ap_info_t* ap = &appvt->appub;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pub_t *pub = appvt->pub;

	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	if (bsscfg == NULL) {
		ASSERT(bsscfg != NULL);
		return BCME_ERROR;
	}

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_WME_AC_AP):
		if (WME_ENAB(pub)) {
			edcf_acparam_t acp_all[AC_COUNT];
			wlc_wme_acp_get_all(wlc, &ap->wme_param_ie_ap, acp_all);
			memcpy(arg, acp_all, sizeof(acp_all));
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_WME_AC_AP):
		if (WME_ENAB(pub)) {
			edcf_acparam_t acp;
			memcpy(&acp, arg, sizeof(acp));
			err = wlc_wme_acp_set(wlc, &ap->wme_param_ie_ap, &acp);
		} else
			err = BCME_UNSUPPORTED;
		break;

#ifdef WDS
	case IOV_GVAL(IOV_WDS_WPA_ROLE): {
		/* params buf is an ether addr */
		uint8 role = 0;
		if (p_len < ETHER_ADDR_LEN) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		err = wlc_wds_wpa_role_get(ap, params, &role);
		*(uint8*)arg = role;
		break;
	}

	case IOV_SVAL(IOV_WDS_WPA_ROLE): {
		/* arg format: <mac><role> */
		struct scb *scb;
		uint8 *mac = (uint8 *)arg;
		uint8 *role = mac + ETHER_ADDR_LEN;
		if (!(scb = wlc_scbfind(wlc, (struct ether_addr *)mac))) {
			err = BCME_NOTFOUND;
			goto exit;
		}
		err = wlc_wds_wpa_role_set(ap, scb, *role);
		break;
	}
#endif /* WDS */

	case IOV_GVAL(IOV_AUTHE_STA_LIST):
		/* return buf is a maclist */
		err = wlc_sta_list_get(ap, bsscfg, (uint8*)arg, len,
		                       wlc_authenticated_sta_check_cb);
		break;

	case IOV_GVAL(IOV_AUTHO_STA_LIST):
		/* return buf is a maclist */
		err = wlc_sta_list_get(ap, bsscfg, (uint8*)arg, len,
		                       wlc_authorized_sta_check_cb);
		break;

	case IOV_GVAL(IOV_WME_STA_LIST):	/* Deprecated; use IOV_STA_INFO */
		/* return buf is a maclist */
		err = wlc_sta_list_get(ap, bsscfg, (uint8*)arg, len,
		                       wlc_wme_sta_check_cb);
		break;

	case IOV_GVAL(IOV_MAXASSOC):
		*(uint32*)arg = appvt->maxassoc;
		break;

	case IOV_SVAL(IOV_MAXASSOC):
		if (int_val > wlc->pub->tunables->maxscb) {
			err = BCME_RANGE;
			goto exit;
		}
		appvt->maxassoc = int_val;
		break;

#if defined(MBSS)
	case IOV_GVAL(IOV_BSS_MAXASSOC):
		*(uint32*)arg = bsscfg->maxassoc;
		break;

	case IOV_SVAL(IOV_BSS_MAXASSOC):
		if (int_val > wlc->pub->tunables->maxscb) {
			err = BCME_RANGE;
			goto exit;
		}
		bsscfg->maxassoc = int_val;
		break;
	case IOV_GVAL(IOV_BCN_ROTATE):
		if (wlc->beacon_bssidx < 0)
			*ret_int_ptr = 0;
		else
			*ret_int_ptr = 1;
		break;

	case IOV_SVAL(IOV_BCN_ROTATE):
		if (bool_val)
			wlc->beacon_bssidx = 0;
		else
			wlc->beacon_bssidx = -1;
		break;
#endif /* MBSS */

	case IOV_GVAL(IOV_AP_ISOLATE):
		*ret_int_ptr = (int32)bsscfg->ap_isolate;
		break;

	case IOV_GVAL(IOV_AP):
		*((uint*)arg) = BSSCFG_AP(bsscfg);
		break;

	case IOV_SVAL(IOV_AP_ISOLATE):
		bsscfg->ap_isolate = bool_val;
		break;
	case IOV_GVAL(IOV_SCB_ACTIVITY_TIME):
		*ret_int_ptr = (int32)wlc->scb_activity_time;
		break;

	case IOV_SVAL(IOV_SCB_ACTIVITY_TIME):
		wlc->scb_activity_time = (uint32)int_val;
		break;

#ifdef WDS
	case IOV_GVAL(IOV_WDSTIMEOUT):
		*ret_int_ptr = (int32)appvt->wds_timeout;
		break;

	case IOV_SVAL(IOV_WDSTIMEOUT):
		appvt->wds_timeout = (uint32)int_val;
		break;
#endif /* WDS */

	case IOV_GVAL(IOV_CLOSEDNET):
		*ret_int_ptr = bsscfg->closednet_nobcnssid;
		break;

	case IOV_SVAL(IOV_CLOSEDNET):
		/* "closednet" control two functionalities: hide ssid in bcns
		 * and don't respond to broadcast probe requests
		 */
		bsscfg->closednet_nobcnssid = bool_val;
		bsscfg->closednet_nobcprbresp = bool_val;
		if (BSSCFG_AP(bsscfg) && bsscfg->up) {
			wlc_bss_update_beacon(wlc, bsscfg);
			wlc_mctrl(wlc, MCTL_CLOSED_NETWORK,
				(bool_val ? MCTL_CLOSED_NETWORK : 0));
		}
		break;

	case IOV_GVAL(IOV_MBSS):
		*ret_int_ptr = pub->_mbss ? TRUE : FALSE;
		break;

	case IOV_SVAL(IOV_MBSS): {
#ifdef MBSS
		bool curstat = (pub->_mbss != 0);
		bool rev16 = D11REV_ISMBSS16(pub->corerev) ? TRUE : FALSE;
		bool rev4 = D11REV_ISMBSS4(pub->corerev) ? TRUE : FALSE;

		/* No change requested */
		if (curstat == bool_val)
			break;

		if (!rev4 && !rev16) {
			err = BCME_UNSUPPORTED;
			break;
		}

		/* Reject if insufficient template memory */
		if (rev16 && (wlc_bmac_ucodembss_hwcap(wlc->hw) == FALSE)) {
			err = BCME_NORESOURCE;
			break;
		}

		if (curstat) {
			/* if turning off mbss, disable extra bss configs */
			wlc_bsscfg_disablemulti(wlc);
			wlc->defmacintmask &= ~MI_PRQ;
			wlc->defmacintmask &= ~MI_DTIM_TBTT;
			pub->_mbss = 0;
		}
		else {
			if (!rev16)
				wlc->defmacintmask |= MI_PRQ;
			wlc->defmacintmask |= MI_DTIM_TBTT;
			pub->_mbss = rev4 ? MBSS4_ENABLED : MBSS16_ENABLED;
		}

		/* Compacity fixup... */
		if (pub->_mbss) {
			wlc->max_ap_bss = pub->tunables->maxucodebss;
			if (rev4 || D11REV_IS(pub->corerev, 24))
				wlc->max_ap_bss = 4;
			wlc->mbss_ucidx_mask = wlc->max_ap_bss - 1;
		}
#ifdef WLLPRS
		/* Enable/disable legacy prs support in ucode based on mbss
		 * state.
		 */
		wlc_mhf(wlc, MHF5, MHF5_LEGACY_PRS, (pub->_mbss ? MHF5_LEGACY_PRS : 0),
			WLC_BAND_ALL);
#endif /* WLLPRS */
#else
		err = BCME_UNSUPPORTED;
#endif	/* MBSS */
		break;
	}

	case IOV_GVAL(IOV_BSS):
		if (p_len < (int)sizeof(int)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (int_val >= 0) {
			bsscfg = wlc_bsscfg_find(wlc, int_val, &err);
		} /* otherwise, use the value from the wlif object */

		if (bsscfg)
			*ret_int_ptr = bsscfg->up;
		else if (err == BCME_NOTFOUND)
			*ret_int_ptr = 0;
		else
			break;
		break;

	case IOV_SVAL(IOV_BSS): {
		bool sta = FALSE;

		if (len < (int)(2 * sizeof(int))) {
			err = BCME_BUFTOOSHORT;
			break;
		}
#ifdef STA
		if (int_val2 == 3)
			sta = FALSE;
		else if (int_val2 == 2)
			sta = TRUE;
#endif
		/* On an 'up', use wlc_bsscfg_alloc() to create a bsscfg if it does not exist,
		 * but on a 'down', just find the bsscfg if it already exists
		 */
		if (int_val >= 0) {
			bsscfg = wlc_bsscfg_find(wlc, int_val, &err);
			if (int_val2 > 0) {
				if (bsscfg == NULL && err == BCME_NOTFOUND) {
					bsscfg = wlc_bsscfg_alloc(wlc, int_val, 0, NULL, !sta);
					if (bsscfg == NULL)
						err = BCME_NOMEM;
					else if ((err = wlc_bsscfg_init(wlc, bsscfg))) {
						WL_ERROR(("wl%d: wlc_bsscfg_init %s failed (%d)\n",
						          wlc->pub->unit, sta ? "STA" : "AP", err));
						wlc_bsscfg_free(wlc, bsscfg);
					}
				}
			}
		}

#ifdef STA
		if (int_val2 == 3)
			break;
		else if (int_val2 == 2)
			break;
#endif
		if (err) {
			/* do not error on a 'down' of a nonexistent bsscfg */
			if (err == BCME_NOTFOUND && int_val2 == 0)
				err = 0;
			break;
		}

		if (int_val2 > 0) {
			if (bsscfg->up) {
				WL_APSTA_UPDN(("wl%d: Ignoring UP, bsscfg %d already UP\n",
					pub->unit, int_val));
				break;
			}
			if (mboolisset(wlc->pub->radio_disabled, WL_RADIO_HW_DISABLE) ||
				mboolisset(wlc->pub->radio_disabled, WL_RADIO_SW_DISABLE)) {
				WL_APSTA_UPDN(("wl%d: Ignoring UP, bsscfg %d; radio off\n",
					pub->unit, int_val));
				err = BCME_RADIOOFF;
				break;
			}

			WL_APSTA_UPDN(("wl%d: BSS up cfg %d (%s) -> wlc_bsscfg_enable()\n",
				pub->unit, int_val, (BSSCFG_AP(bsscfg) ? "AP" : "STA")));
			if (BSSCFG_AP(bsscfg))
				err = wlc_bsscfg_enable(wlc, bsscfg);
#ifdef WLP2P
			else if (BSS_P2P_ENAB(wlc, bsscfg))
				err = BCME_ERROR;
#endif
#ifdef STA
			else if (BSSCFG_STA(bsscfg))
				wlc_join(wlc, bsscfg, bsscfg->SSID, bsscfg->SSID_len,
				         NULL, NULL, 0);
#endif
			if (err)
				break;
#ifdef RADAR
			if (WL11H_AP_ENAB(wlc)) {
				if (!(BSSCFG_SRADAR_ENAB(bsscfg) ||
				      BSSCFG_AP_NORADAR_CHAN_ENAB(bsscfg)))
					wlc_set_dfs_cacstate(ap, ON);
			}
#endif /* RADAR */
		} else {
			if (!bsscfg->enable) {
				WL_APSTA_UPDN(("wl%d: Ignoring DOWN, bsscfg %d already DISABLED\n",
					pub->unit, int_val));
				break;
			}
			WL_APSTA_UPDN(("wl%d: BSS down on %d (%s) -> wlc_bsscfg_disable()\n",
				pub->unit, int_val, (BSSCFG_AP(bsscfg) ? "AP" : "STA")));
			wlc_bsscfg_disable(wlc, bsscfg);
		}
		break;
	}

	case IOV_GVAL(IOV_APCSCHSPEC):
		if (pub->up && AP_ENAB(pub) &&
		    !SCAN_IN_PROGRESS(wlc->scan)) {
			*ret_int_ptr = (int32)wlc->chanspec_selected;
		}
		else {
			err = BCME_BADARG;
		}
		break;

#if defined(STA)     /* APSTA */
	case IOV_GVAL(IOV_APSTA):
		*ret_int_ptr = APSTA_ENAB(wlc->pub);
		break;

	case IOV_SVAL(IOV_APSTA):
		/* Flagged for no set while up */
		if (bool_val == APSTA_ENAB(wlc->pub)) {
			WL_APSTA(("wl%d: No change to APSTA mode\n", wlc->pub->unit));
			break;
		}

		bsscfg = wlc_bsscfg_primary(wlc);
		if (bool_val) {
			/* Turning on APSTA, force various other items:
			 *   Global AP, cfg (wlc->cfg) STA, not IBSS.
			 *   Make beacon/probe AP config bsscfg[1].
			 *   Force off 11D.
			 */
			WL_APSTA(("wl%d: Enabling APSTA mode\n", wlc->pub->unit));
			if (bsscfg->enable)
				wlc_bsscfg_disable(wlc, bsscfg);
	        }
	        else {
	                /* Turn off APSTA: make global AP and cfg[0] same */
	                WL_APSTA(("wl%d: Disabling APSTA mode\n", wlc->pub->unit));
	        }
		err = wlc_bsscfg_reinit(wlc, bsscfg, bool_val == 0);
		if (err)
			break;
		wlc->pub->_ap = TRUE;
		if (bool_val) {
	                wlc->default_bss->infra = 1;
#ifdef WLAFTERBURNER
			/*  Also force off Afterburner if it was forced ON */
			if (wlc->afterburner_override == ON)
				wlc_abo(wlc, OFF);
#endif /* WLAFTERBURNER */

		}
		wlc->pub->_apsta = bool_val;

	        /* Act similarly to WLC_SET_AP */
	        wlc_ap_upd(wlc, bsscfg);
	        wlc->wet = FALSE;
	        wlc_radio_mpc_upd(wlc);
	        break;

#endif /* APSTA */

#ifdef RADAR
	case IOV_GVAL(IOV_DFS_PREISM):
		*ret_int_ptr = (int)appvt->dfs_cac.cactime_preism;
		break;

	case IOV_SVAL(IOV_DFS_PREISM):
		if (int_val >= WLC_DFS_CAC_TIME_SEC_MAX) {
			err = BCME_RANGE;
			break;
		}
		appvt->dfs_cac.cactime_preism = (uint)int_val;

		break;

	case IOV_GVAL(IOV_DFS_POSTISM):
		*ret_int_ptr = (int)appvt->dfs_cac.cactime_postism;
		break;

	case IOV_SVAL(IOV_DFS_POSTISM):
		if (int_val >= WLC_DFS_CAC_TIME_SEC_MAX) {
			err = BCME_RANGE;
			break;
		}

		appvt->dfs_cac.cactime_postism = (uint)int_val;
		break;

	case IOV_GVAL(IOV_DFS_STATUS):
		appvt->dfs_cac.status.duration =
		(appvt->dfs_cac.cactime -
		appvt->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL;
		bcopy((char *)&appvt->dfs_cac.status, (char *)arg, sizeof(wl_dfs_status_t));
		break;

	case IOV_SVAL(IOV_DFS_ISM_MONITOR):
		appvt->dfs_cac.ism_monitor = bool_val;
		break;

	case IOV_GVAL(IOV_DFS_ISM_MONITOR):
		*ret_int_ptr = (int32)appvt->dfs_cac.ism_monitor;
		break;

	/* IOV_DFS_CHANNEL_FORCED is required for regulatory testing */
	case IOV_SVAL(IOV_DFS_CHANNEL_FORCED):
		if (CHSPEC_CHANNEL((chanspec_t)int_val) == 0) {
			appvt->dfs_cac.chanspec_forced = wlc_create_chspec(wlc, (uint8)int_val);
			break;
		}

		if (!N_ENAB(pub)) {
			int chan = CHSPEC_CHANNEL((chanspec_t)int_val);
			if (chan < 0 || chan > MAXCHANNEL) {
				err = BCME_OUTOFRANGECHAN;
				break;
			}
			appvt->dfs_cac.chanspec_forced = wlc_create_chspec(wlc, (uint8)int_val);
			break;
		}

		if (wf_chspec_malformed((chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}

		if (!wlc_valid_chanspec_db(wlc->cmi, (chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}

		if (!N_ENAB(pub) && CHSPEC_IS40((chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}

		appvt->dfs_cac.chanspec_forced = (chanspec_t)int_val;
		break;

	case IOV_GVAL(IOV_DFS_CHANNEL_FORCED):
		if (!N_ENAB(pub))
			*ret_int_ptr = CHSPEC_CHANNEL(appvt->dfs_cac.chanspec_forced);
		else
			*ret_int_ptr = appvt->dfs_cac.chanspec_forced;
		break;

	case IOV_SVAL(IOV_DFS_CHANNEL_SAFE): {
		uint16 channel = (chanspec_t)int_val & WL_CHANSPEC_CHAN_MASK;

		if (!N_ENAB(pub)) {
			int chan = CHSPEC_CHANNEL((chanspec_t)int_val);
			if (chan < 0 || chan > MAXCHANNEL) {
				err = BCME_OUTOFRANGECHAN;
				break;
			}
			appvt->dfs_cac.chanspec_safe = wlc_create_chspec(wlc, (uint8)int_val);
			break;
		}

		if (wf_chspec_malformed((chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}

		if (!VALID_CHANNEL20_IN_BAND(wlc, CHANNEL_BANDUNIT(wlc, channel), channel)) {
			err = BCME_BADCHAN;
			break;
		}

		if (wlc_radar_chanspec(wlc->cmi, (chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}

		if (!N_ENAB(pub) && CHSPEC_IS40((chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}
		appvt->dfs_cac.chanspec_safe = (chanspec_t)int_val;
		break;
	}

	case IOV_GVAL(IOV_DFS_CHANNEL_SAFE):
		if (!N_ENAB(pub))
			*ret_int_ptr = CHSPEC_CHANNEL(appvt->dfs_cac.chanspec_safe);
		else
			*ret_int_ptr = (uint32)appvt->dfs_cac.chanspec_safe;
		break;
#endif /* RADAR */


	case IOV_SVAL(IOV_CHANSPEC_SWITCH): {
		wl_chan_switch_t *csa;

		csa = (wl_chan_switch_t *)arg;
		if (wf_chspec_malformed(csa->chspec)) {
			err = BCME_BADCHAN;
			break;
		}

#ifdef BCMDBG
		if (BSSCFG_STA(bsscfg)) {
			if (!bsscfg->up) {
				err = BCME_NOTREADY;
				break;
			}

			wlc_send_action_switch_channel_ex(ap, bsscfg, &bsscfg->BSSID,
				csa, DOT11_SM_ACTION_CHANNEL_SWITCH);
			break;
		}
#endif /* BCMDBG */

		err = wlc_ap_switch_channel(bsscfg, csa->chspec,
		                            csa->mode, csa->count, csa->reg);

		break;
	}

	case IOV_GVAL(IOV_PREF_CHANSPEC):
	        *ret_int_ptr = (int32)wlc->ap->pref_chanspec;
		break;

	case IOV_SVAL(IOV_PREF_CHANSPEC):
	        /* Special case to clear user preferred setting.
		 * 0 - pref_chanspec means we're using Auto Channel.
		 */
	        if (int_val == 0) {
			wlc->ap->pref_chanspec = 0;
			break;
		}

		if (!wlc_valid_chanspec_db(wlc->cmi, (chanspec_t)int_val)) {
			err = BCME_BADCHAN;
			break;
		}

		wlc->ap->pref_chanspec = (chanspec_t)int_val;
		break;

#ifdef WDS
	case IOV_GVAL(IOV_WDS_ENABLE):
		/* do nothing */
		break;

	case IOV_SVAL(IOV_WDS_ENABLE):
		if (wlcif == NULL || wlcif->type != WLC_IFTYPE_WDS) {
			WL_ERROR(("invalid interface type for IOV_WDS_ENABLE\n"));
			err = BCME_NOTFOUND;
			goto exit;
		}
		err = wlc_wds_create_link_event(ap, wlcif->u.scb);
		break;
#endif /* WDS */
#ifdef RXCHAIN_PWRSAVE
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_ENABLE):
	        *ret_int_ptr = ap->rxchain_pwrsave_enable;
		break;

	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_ENABLE):
		ap->rxchain_pwrsave_enable =
			appvt->rxchain_pwrsave.pwrsave.power_save_check = int_val;
		if (!int_val)
			wlc_reset_rxchain_pwrsave_mode(ap);
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_QUIET_TIME):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.quiet_time;
		break;

	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_QUIET_TIME):
		appvt->rxchain_pwrsave.pwrsave.quiet_time = int_val;
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_PPS):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.pps_threshold;
		break;

	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_PPS):
		appvt->rxchain_pwrsave.pwrsave.pps_threshold = int_val;
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.in_power_save;
		break;
	case IOV_GVAL(IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK):
	        *ret_int_ptr = appvt->rxchain_pwrsave.pwrsave.stas_assoc_check;
		break;
	case IOV_SVAL(IOV_RXCHAIN_PWRSAVE_STAS_ASSOC_CHECK):
		appvt->rxchain_pwrsave.pwrsave.stas_assoc_check = int_val;
		if (int_val && ap->rxchain_pwrsave_enable &&
		    appvt->rxchain_pwrsave.pwrsave.in_power_save &&
		    wlc_ap_stas_associated(wlc->ap)) {
			wlc_reset_rxchain_pwrsave_mode(ap);
		}
		break;
#endif /* RXCHAIN_PWRSAVE */
#ifdef RADIO_PWRSAVE
	case IOV_GVAL(IOV_RADIO_PWRSAVE_ENABLE):
	        *ret_int_ptr = ap->radio_pwrsave_enable;
		break;

	case IOV_SVAL(IOV_RADIO_PWRSAVE_ENABLE):
		if (!MBSS_ENAB(wlc->pub)) {
			err = BCME_EPERM;
			WL_ERROR(("wl%d: Radio pwrsave not supported in non-mbss case yet.\n",
				wlc->pub->unit));
			break;
		}
		ap->radio_pwrsave_enable = appvt->radio_pwrsave.pwrsave.power_save_check = int_val;
		wlc_reset_radio_pwrsave_mode(ap);
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE_QUIET_TIME):
	        *ret_int_ptr = appvt->radio_pwrsave.pwrsave.quiet_time;
		break;

	case IOV_SVAL(IOV_RADIO_PWRSAVE_QUIET_TIME):
		appvt->radio_pwrsave.pwrsave.quiet_time = int_val;
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE_PPS):
	        *ret_int_ptr = appvt->radio_pwrsave.pwrsave.pps_threshold;
		break;

	case IOV_SVAL(IOV_RADIO_PWRSAVE_PPS):
		appvt->radio_pwrsave.pwrsave.pps_threshold = int_val;
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE):
	        *ret_int_ptr = appvt->radio_pwrsave.pwrsave.in_power_save;
		break;
	case IOV_SVAL(IOV_RADIO_PWRSAVE_LEVEL):{
		uint8 dtim_period;
		uint16 beacon_period;

		bsscfg = wlc->cfg;

		if (bsscfg->associated) {
			dtim_period = bsscfg->current_bss->dtim_period;
			beacon_period = bsscfg->current_bss->beacon_period;
		} else {
			dtim_period = wlc->default_bss->dtim_period;
			beacon_period = wlc->default_bss->beacon_period;
		}

		if (int_val > RADIO_PWRSAVE_HIGH) {
			err = BCME_RANGE;
			goto exit;
		}

		if (dtim_period == 1) {
			err = BCME_ERROR;
			goto exit;
		}

		appvt->radio_pwrsave.level = int_val;
		switch (appvt->radio_pwrsave.level) {
		case RADIO_PWRSAVE_LOW:
			appvt->radio_pwrsave.on_time = 2*beacon_period*dtim_period/3;
			appvt->radio_pwrsave.off_time = beacon_period*dtim_period/3;
			break;
		case RADIO_PWRSAVE_MEDIUM:
			appvt->radio_pwrsave.on_time = beacon_period*dtim_period/2;
			appvt->radio_pwrsave.off_time = beacon_period*dtim_period/2;
			break;
		case RADIO_PWRSAVE_HIGH:
			appvt->radio_pwrsave.on_time = beacon_period*dtim_period/3;
			appvt->radio_pwrsave.off_time = 2*beacon_period*dtim_period/3;
			break;
		}
		break;
	}
	case IOV_GVAL(IOV_RADIO_PWRSAVE_LEVEL):
	        *ret_int_ptr = appvt->radio_pwrsave.level;
		break;
	case IOV_SVAL(IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK):
		appvt->radio_pwrsave.pwrsave.stas_assoc_check = int_val;
		if (int_val && RADIO_PWRSAVE_ENAB(wlc->ap) &&
		    wlc_radio_pwrsave_in_power_save(wlc->ap) &&
		    wlc_ap_stas_associated(wlc->ap)) {
			wlc_radio_pwrsave_exit_mode(wlc->ap);
			WL_INFORM(("Going out of power save as there are associated STASs!\n"));
		}
		break;
	case IOV_GVAL(IOV_RADIO_PWRSAVE_STAS_ASSOC_CHECK):
	        *ret_int_ptr = appvt->radio_pwrsave.pwrsave.stas_assoc_check;
		break;
#endif /* RADIO_PWRSAVE */
#ifdef BCM_DCS
	case IOV_GVAL(IOV_BCMDCS):
		*ret_int_ptr = ap->dcs_enabled ? TRUE : FALSE;
		break;

	case IOV_SVAL(IOV_BCMDCS):
		ap->dcs_enabled = bool_val;
		break;

#endif /* BCM_DCS */
	case IOV_GVAL(IOV_DYNBCN):
	        *ret_int_ptr = (int32)((bsscfg->flags & WLC_BSSCFG_DYNBCN) == WLC_BSSCFG_DYNBCN);
		break;
	case IOV_SVAL(IOV_DYNBCN):
		if (!BSSCFG_AP(bsscfg)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (int_val && !DYNBCN_ENAB(bsscfg)) {
			bsscfg->flags |= WLC_BSSCFG_DYNBCN;

			/* Disable beacons if no sta is associated */
			if (wlc_bss_assocscb_getcnt(wlc, bsscfg) == 0)
				wlc_bsscfg_bcn_disable(wlc, bsscfg);
		} else if (!int_val && DYNBCN_ENAB(bsscfg)) {
			bsscfg->flags &= ~WLC_BSSCFG_DYNBCN;
			wlc_bsscfg_bcn_enable(wlc, bsscfg);
		}
		break;
#if defined(WL_AP_TPC)
	case IOV_GVAL(IOV_AP_TPC_MODE):
		*ret_int_ptr = wlc->ap_tpc;
		break;
	case IOV_SVAL(IOV_AP_TPC_MODE):
		err = wlc_ap_tpc_setup(wlc, int_val);
		break;
	case IOV_GVAL(IOV_AP_TPC_PERIOD):
		*ret_int_ptr = wlc->ap_tpc_interval;
		break;
	case IOV_SVAL(IOV_AP_TPC_PERIOD):
		if (WL11H_ENAB(wlc))
			wlc->ap_tpc_interval = (uint8)int_val;
		else
			err = BCME_UNSUPPORTED;
		break;
	case IOV_GVAL(IOV_AP_TPC_LM):
		if (!wlc->ap_tpc) {
			err = BCME_EPERM;
			break;
		}
		*ret_int_ptr = (uint16) wlc->ap_tpc_lm;
		break;
#endif /* WL_AP_TPC */

	case IOV_GVAL(IOV_SCB_LASTUSED): {
		uint elapsed = 0;
		uint min_val = (uint)-1;
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_ASSOCIATED(scb)) {
				elapsed = wlc->pub->now - scb->used;
				if (elapsed < min_val)
					min_val = elapsed;
			}
		}
		*ret_int_ptr = min_val;
		break;
	}

	case IOV_GVAL(IOV_SCB_PROBE): {
		wl_scb_probe_t scb_probe;

		scb_probe.scb_timeout = wlc->scb_timeout;
		scb_probe.scb_activity_time = wlc->scb_activity_time;
		scb_probe.scb_max_probe = wlc->scb_max_probe;

		bcopy((char *)&scb_probe, (char *)arg, sizeof(wl_scb_probe_t));
		break;
	}

	case IOV_SVAL(IOV_SCB_PROBE): {
		wl_scb_probe_t *scb_probe = (wl_scb_probe_t *)arg;

		if (!scb_probe->scb_timeout || (!scb_probe->scb_max_probe)) {
			err = BCME_BADARG;
			break;
		}

		wlc->scb_timeout = scb_probe->scb_timeout;
		wlc->scb_activity_time = scb_probe->scb_activity_time;
		wlc->scb_max_probe = scb_probe->scb_max_probe;
		break;
	}

	case IOV_GVAL(IOV_SCB_ASSOCED): {

		bool assoced = TRUE;
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_ASSOCIATED(scb))
				break;
		}

		if (!scb)
			assoced = FALSE;

		*ret_int_ptr = (uint32)assoced;
		break;
	}

	case IOV_SVAL(IOV_ACS_UPDATE):

		if (SCAN_IN_PROGRESS(wlc->scan)) {
			err = BCME_BUSY;
			break;
		}

		if (wlc->pub->up && (wlc->chanspec_selected != 0) &&
		    (WLC_BAND_PI_RADIO_CHANSPEC != wlc->chanspec_selected))
			wlc_ap_acs_update(wlc);

		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

exit:
	return err;
}

static void
wlc_ap_acs_update(wlc_info_t *wlc)
{

	WL_INFORM(("wl%d: %s: changing chanspec to %d\n",
		wlc->pub->unit, __FUNCTION__, wlc->chanspec_selected));
	wlc_set_home_chanspec(wlc, wlc->chanspec_selected);
	wlc_suspend_mac_and_wait(wlc);

	wlc_set_chanspec(wlc, wlc->chanspec_selected);
	if (AP_ENAB(wlc->pub)) {
		wlc->bcn_rspec = wlc_lowest_basic_rspec(wlc,
			&wlc->cfg->current_bss->rateset);
		ASSERT(wlc_valid_rate(wlc, wlc->bcn_rspec,
			CHSPEC_IS2G(wlc->cfg->current_bss->chanspec) ?
			WLC_BAND_2G : WLC_BAND_5G, TRUE));
		wlc_beacon_phytxctl(wlc, wlc->bcn_rspec);
	}

	if (wlc->pub->associated) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, FALSE);
	}
	wlc_enable_mac(wlc);
}

int
wlc_ap_switch_channel(wlc_bsscfg_t *bsscfg, chanspec_t chanspec,
                      uint8 mode, uint8 count, uint8 reg_class)
{
	wlc_info_t *wlc = bsscfg->wlc;
	wlc_csa_t *csa = bsscfg->csa;

	if (!BSSCFG_AP(bsscfg))
		return BCME_NOTAP;

	if (!bsscfg->up) {
		return BCME_NOTREADY;
	}

	ASSERT(!wf_chspec_malformed(chanspec));

	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
		return BCME_BADCHAN;
	}

	csa->csa.mode = (mode != 0) ? 1 : 0;
	csa->csa.count = count;
	csa->csa.chspec = chanspec;

	if (reg_class != 0)
		csa->csa.reg = reg_class;
	else
		csa->csa.reg = wlc_get_regclass(wlc->cmi, chanspec);

	csa->spect_state |= NEED_TO_SWITCH_CHANNEL;
	/* adds NEED_TO_UPDATE_BCN to wlc->spect_state, send csa action frames, */
	/* and update beacon and probe response for the specified bsscfg */
	wlc_ap_apply_channel_switch(bsscfg);

	return 0;
}

/* This function applies the parameters of channel switch set else where */
/* It is assumed that the csa parameters have been set else where */
/* We send out the necessary csa action frames */
/* We mark wlc->spect_state with NEED_TO_UPDATE_BCN flag */
/* We only want the beacons and probe responses updated for the specified bss */
/* The actual channel switch will be initiated else where */
int
wlc_ap_apply_channel_switch(wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = bsscfg->wlc;

	if (!BSSCFG_AP(bsscfg))
		return BCME_NOTAP;

	if (!bsscfg->up) {
		return BCME_NOTREADY;
	}

	wlc_send_action_switch_channel(wlc->ap, bsscfg);

	bsscfg->csa->spect_state |= NEED_TO_UPDATE_BCN;
	wlc_bss_update_beacon(wlc, bsscfg);
	wlc_bss_update_probe_resp(wlc, bsscfg, TRUE);

	return 0;
}

#if defined(WL11N) && defined(WLAFTERBURNER)
void
wlc_switch_ab_11n(wlc_info_t *wlc, bool ab, bool init)
{
	int err;
	int nmode = 0;
	bool wasup = wlc->pub->up;

	/*
	 * -- afterburner_override == ON, set N mode OFF
	 *
	 * -- afterburner_override == OFF, set N mode according to
	 * wlc->protection->nmode_user
	 *
	 * -- afterburner_override == AUTO, set N mode according to
	 * wlc->afterburner, wlc->protection->nmode_user
	 *
	 */

	/* store away some settings we might need on subsequent invocations */
	if (init) {
		wlc->wme_saved = wlc->pub->_wme;
		wlc->ampdu_saved = wlc->pub->_ampdu;
#ifdef WLAMSDU
		wlc->amsdu_tx_saved = wlc->pub->_amsdu_tx;
#endif
	}

	switch (wlc->afterburner_override) {

		case ON:
			nmode = OFF;
			break;

		case OFF:
			nmode = wlc->protection->nmode_user;
			break;

		case AUTO:
			nmode = ab ? OFF : wlc->protection->nmode_user;
			break;

		default:
			ASSERT(0);
			return;
	}

	wasup = wlc->pub->up;
	if (wasup)
		wlc_down(wlc);

	wlc->guard_count = 0;
	err = wlc_set_nmode(wlc, nmode);
	if (err)
		WL_ERROR(("wl%d: %s: ERROR setting N mode %d\n",
			wlc->pub->unit, __FUNCTION__, nmode));

	if (ab) { /* turn off amsdu, ampdu, wme */
		wlc->pub->_wme = OFF;
		err = wlc_ampdu_set(wlc->ampdu, OFF);
		if (err)
			WL_ERROR(("wl%d: %s ERROR %d setting ampdu OFF\n",
				wlc->pub->unit, __FUNCTION__, err));
#ifdef WLAMSDU
		err = wlc_amsdu_set(wlc->ami, OFF);
		if (err)
			WL_ERROR(("wl%d: %s ERROR %d setting amsdu OFF\n",
				wlc->pub->unit, __FUNCTION__, err));
#endif

	} else { /* restore amsdu, ampdu, wme */
		wlc->pub->_wme = wlc->wme_saved;
		err = wlc_ampdu_set(wlc->ampdu, wlc->ampdu_saved);
		if (err)
			WL_ERROR(("wl%d: %s ERROR %d restoring ampdu to %d\n",
				wlc->pub->unit, __FUNCTION__, err, wlc->ampdu_saved));
#ifdef WLAMSDU
		err = wlc_amsdu_set(wlc->ami, wlc->amsdu_tx_saved);
		if (err)
			WL_ERROR(("wl%d: %s ERROR %d restoring amsdu to %d\n",
				wlc->pub->unit, __FUNCTION__, err, wlc->amsdu_tx_saved));
#endif

	}
	wlc_afterburner(wlc, ab);

	/* Called during init e.g. from wlconf, stay down */
	if (!wasup)
		return;

	wlc_up(wlc);


	/*************************
	*	Deauthenticate all STAs:
	*************************
	*/
	{	struct scb *scb;
		struct scb_iter scbiter;

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_ASSOCIATED(scb)) {
				WL_TRACE(("\n   DEAUTH: %02x:%02x:%02x:%02x:%02x:%02x!",
					scb->ea.octet[0], scb->ea.octet[1], scb->ea.octet[2],
					scb->ea.octet[3], scb->ea.octet[4], scb->ea.octet[5]));
				wlc_scb_set_auth(wlc, SCB_BSSCFG(scb), scb,
					FALSE, AUTHENTICATED, DOT11_RC_AUTH_INVAL);
			}
		}
	}

	return;
}

#endif /* defined(WL11N) && defined(WAFTERBURNER) */

#ifdef WIN7
static void
wlc_ap_scb_free_notify(void *context, struct scb *scb)
{
	wlc_ap_info_pvt_t *appvt = (wlc_ap_info_pvt_t*)context;
	wlc_info_t *wlc = appvt->wlc;
	wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN], *ea = bcm_ether_ntoa(&scb->ea, eabuf);
#endif /* BCMDBG_ERR */

	if (bsscfg && BSSCFG_AP(bsscfg) && SCB_ASSOCIATED(scb) && wlc->eventq) {
		WL_ASSOC(("wl%d: AP: scb free: indicate disassoc for the STA-%s\n",
			wlc->pub->unit, ea));
		wlc_bss_mac_event(wlc, scb->bsscfg, WLC_E_DISASSOC_IND, &scb->ea,
			WLC_E_STATUS_SUCCESS, DOT11_RC_DISASSOC_LEAVING, 0, 0, 0);
	}
}
#endif /* WIN7 */

void
wlc_ap_scb_cleanup(wlc_info_t *wlc)
{
	wlc_bsscfg_t *apcfg;
	int idx;

	FOREACH_UP_AP(wlc, idx, apcfg) {
		wlc_txflowcontrol_override(wlc, apcfg->wlcif->qi, OFF, TXQ_STOP_FOR_PKT_DRAIN);
		wlc_scb_update_band_for_cfg(wlc, apcfg, WLC_BAND_PI_RADIO_CHANSPEC);
	}
}

#if defined(RXCHAIN_PWRSAVE) || defined(RADIO_PWRSAVE)
/*
 * Returns true if check for associated STAs is enabled
 * and there are STAs associated
 */
static bool
wlc_pwrsave_stas_associated_check(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	bool check_assoc_stas = FALSE;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			check_assoc_stas = appvt->rxchain_pwrsave.pwrsave.stas_assoc_check;
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			check_assoc_stas = appvt->radio_pwrsave.pwrsave.stas_assoc_check;
			break;
#endif
		default:
			break;
	}
	return (check_assoc_stas && (wlc_ap_stas_associated(ap) > 0));
}

/*
 * At every watchdog tick we update the power save
 * data structures and see if we can go into a power
 * save mode
 */
static void
wlc_pwrsave_mode_check(wlc_ap_info_t *ap, int type)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	wlc_pwrsave_t *pwrsave = NULL;
	uint pkts_per_second, total_pktcount;

	switch (type) {
#ifdef RXCHAIN_PWRSAVE
		case PWRSAVE_RXCHAIN:
			pwrsave = &appvt->rxchain_pwrsave.pwrsave;
			break;
#endif
#ifdef RADIO_PWRSAVE
		case PWRSAVE_RADIO:
			pwrsave = &appvt->radio_pwrsave.pwrsave;
			break;
#endif
		default:
			break;
	}

	/* Total pkt count - forwarded packets + packets the os has given + sendup packets */
	total_pktcount =  wlc->pub->_cnt->rxframe + wlc->pub->_cnt->txframe;

	/* Calculate the packets per second */
	pkts_per_second = total_pktcount - pwrsave->prev_pktcount;

	/* Save the current packet count for next second */
	pwrsave->prev_pktcount =  total_pktcount;

	if (pkts_per_second < pwrsave->pps_threshold) {
		/* When the packets are below the threshold we just
		 * increment our timeout counter
		 */
		if (!pwrsave->in_power_save) {
			if ((pwrsave->quiet_time_counter >= pwrsave->quiet_time) &&
			    (! wlc_pwrsave_stas_associated_check(ap, type))) {
				WL_INFORM(("Entering power save mode pps is %d\n",
					pkts_per_second));
#ifdef RXCHAIN_PWRSAVE
				if (type == PWRSAVE_RXCHAIN) {
					/* Save current configured rxchains */
					appvt->rxchain_pwrsave.rxchain = wlc->stf->rxchain;
#ifdef WL11N
					/* need to save and disable rx_stbc HT capability
					 * before enter rxchain_pwrsave mode
					 */
					appvt->rxchain_pwrsave.ht_cap_rx_stbc =
						wlc_stf_enter_rxchain_pwrsave(wlc);
#endif /* WL11N */
					wlc_stf_rxchain_set(wlc, 0x1);
				}
#endif /* RXCHAIN_PWRSAVE */
				pwrsave->in_power_save = TRUE;
				pwrsave->in_power_save_counter++;
				return;
			}
		}
		pwrsave->quiet_time_counter++;
	} else {
		/* If we are already in the wait mode counting
		 * up then just reset the counter since
		 * packets have gone above threshold
		 */
		pwrsave->quiet_time_counter = 0;
		WL_INFORM(("Resetting quiet time\n"));
		if (pwrsave->in_power_save) {
			if (type == PWRSAVE_RXCHAIN) {
#ifdef RXCHAIN_PWRSAVE
				wlc_stf_rxchain_set(wlc, appvt->rxchain_pwrsave.rxchain);
#ifdef WL11N
				/* need to restore rx_stbc HT capability
				 * after exit rxchain_pwrsave mode
				 */
				wlc_stf_exit_rxchain_pwrsave(wlc,
					appvt->rxchain_pwrsave.ht_cap_rx_stbc);
#endif /* WL11N */
#endif /* RXCHAIN_PWRSAVE */
			} else if (type == PWRSAVE_RADIO) {
#ifdef RADIO_PWRSAVE
				wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
				if (appvt->radio_pwrsave.radio_disabled) {
					wlc_bmac_radio_hw(wlc->hw, TRUE,
						(CHIPID(wlc->hw->sih->chip) == BCM5356_CHIP_ID) ?
						TRUE : FALSE);
					appvt->radio_pwrsave.radio_disabled = FALSE;
				}
				appvt->radio_pwrsave.pwrsave_state = 0;
				appvt->radio_pwrsave.cncl_bcn = FALSE;
#endif
			}
			WL_INFORM(("Exiting power save mode pps is %d\n", pkts_per_second));
			pwrsave->in_power_save = FALSE;
		}
	}
}
#endif /* RXCHAIN_PWRSAVE || RADIO_PWRSAVE */

#ifdef RADIO_PWRSAVE

/*
 * Routine that enables/disables the radio for the duty cycle
 */
static void
wlc_radio_pwrsave_timer(void *arg)
{
	wlc_ap_info_t *ap = (wlc_ap_info_t*)arg;
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;

	if (appvt->radio_pwrsave.radio_disabled) {
		WL_INFORM(("wl power timer OFF period end. enabling radio\n"));

		if (appvt->radio_pwrsave.level) {
			appvt->radio_pwrsave.cncl_bcn = TRUE;
		}

		wlc_bmac_radio_hw(wlc->hw, TRUE,
			(CHIPID(wlc->hw->sih->chip) == BCM5356_CHIP_ID) ?  TRUE : FALSE);
		appvt->radio_pwrsave.radio_disabled = FALSE;
		/* Re-enter power save state */
		appvt->radio_pwrsave.pwrsave_state = 2;
	} else {
		WL_INFORM(("wl power timer OFF period starts. disabling radio\n"));
		wlc_radio_pwrsave_off_time_start(ap);
		wlc_bmac_radio_hw(wlc->hw, FALSE,
			(CHIPID(wlc->hw->sih->chip) == BCM5356_CHIP_ID) ?  TRUE : FALSE);
		appvt->radio_pwrsave.radio_disabled = TRUE;
	}
}

/*
 * Start the on time of the beacon interval
 */
void
wlc_radio_pwrsave_on_time_start(wlc_ap_info_t *ap, bool dtim)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	uint32 on_time, state;

	/* Update quite ie only the first time we enter power save mode.
	 * If we are entering power save for the first time set the count
	 * such that quiet time starts after 3 bcn intervals.
	 *
	 * state 0 - initial state / just exited pwr save
	 * state 1 - entering pwr save - sent quiet ie with count n
	 * state 2 - re-entering pwr save - sent quiet ie with count 1
	 * state 3 - in pwr save - radio on/off
	 *
	 * ---> 0 ----> 1 ------> 3
	 *      ^       |    ^    |
	 *      |       |    |    |
	 *      |       v    |    v
	 *      +<---------- 2 <--+
	 *      |                 |
	 *      |                 |
	 *      |                 |
	 *      +<----------------+
	 */
	state = appvt->radio_pwrsave.pwrsave_state;
	if (state == 0) {
#ifdef WL11H
		wlc_radio_pwrsave_update_quiet_ie(ap, 3);
#endif
		/* Enter power save state */
		appvt->radio_pwrsave.tbtt_skip = 3;
		appvt->radio_pwrsave.pwrsave_state = 1;
	}

	/* We are going to start radio on/off only after counting
	 * down tbtt_skip bcn intervals.
	 */
	if (appvt->radio_pwrsave.tbtt_skip-- > 0) {
		WL_INFORM(("wl%d: tbtt skip %d\n", wlc->pub->unit,
		           appvt->radio_pwrsave.tbtt_skip));
		return;
	}

	if (!dtim)
		return;

	if (appvt->radio_pwrsave.level) {
		appvt->radio_pwrsave.cncl_bcn = FALSE;
	}

	/* Schedule the timer to turn off the radio after on_time msec */
	on_time = appvt->radio_pwrsave.on_time * DOT11_TU_TO_US;
	on_time += ap->pre_tbtt_us;

	appvt->radio_pwrsave.tbtt_skip = ((on_time / DOT11_TU_TO_US) /
	                                          wlc->cfg->current_bss->beacon_period);
	on_time /= 1000;
	/* acc for extra phy and mac suspend delays, it seems to be huge. Need
	 * to extract out the exact delays.
	 */
	on_time += RADIO_PWRSAVE_TIMER_LATENCY;

	WL_INFORM(("wl%d: adding timer to disable phy after %d ms state %d, skip %d\n",
	           wlc->pub->unit, on_time, appvt->radio_pwrsave.pwrsave_state,
	           appvt->radio_pwrsave.tbtt_skip));

	/* In case pre-tbtt intr arrived before the timer that disables the radio */
	wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);

	wl_add_timer(wlc->wl, appvt->radio_pwrsave.timer, on_time, FALSE);

	/* Update bcn and probe resp to send quiet ie starting from next
	 * tbtt intr.
	 */
	wlc_radio_pwrsave_update_quiet_ie(ap, appvt->radio_pwrsave.tbtt_skip);
}

/*
 * Start the off time of the beacon interval
 */
static void
wlc_radio_pwrsave_off_time_start(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	uint32 off_time, bcn_period;

	/* Calcuate the delay after which to schedule timer to turn on 
	 * the radio. Also take in to account the phy enabling latency.
	 * We have to make sure the phy is enabled by the next pre-tbtt
	 * interrupt time.
	 */
	bcn_period = wlc->cfg->current_bss->beacon_period;
	off_time = appvt->radio_pwrsave.off_time;

	off_time *= DOT11_TU_TO_US;
	off_time -= (ap->pre_tbtt_us + PHY_DISABLE_MAX_LATENCY_us);

	/* In power save state */
	appvt->radio_pwrsave.pwrsave_state = 3;
	off_time /= 1000;

	/* acc for extra phy and mac suspend delays, it seems to be huge. Need
	 * to extract out the exact delays.
	 */
	off_time -= RADIO_PWRSAVE_TIMER_LATENCY;

	WL_INFORM(("wl%d: add timer to enable phy after %d msec state %d\n",
	           wlc->pub->unit, off_time, appvt->radio_pwrsave.pwrsave_state));

	/* Schedule the timer to turn on the radio after off_time msec */
	wl_add_timer(wlc->wl, appvt->radio_pwrsave.timer, off_time, FALSE);
}

/*
 * Check whether we are in radio power save mode
 */
int
wlc_radio_pwrsave_in_power_save(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;

	return (appvt->radio_pwrsave.pwrsave.in_power_save);
}

/*
 * Enter radio power save
 */
void
wlc_radio_pwrsave_enter_mode(wlc_info_t *wlc, bool dtim)
{
	/* If AP is in radio power save mode, we need to start the duty
	 * cycle with TBTT
	 */
	if (AP_ENAB(wlc->pub) && RADIO_PWRSAVE_ENAB(wlc->ap) &&
	    wlc_radio_pwrsave_in_power_save(wlc->ap)) {
		wlc_radio_pwrsave_on_time_start(wlc->ap, dtim);
	}
}


/*
 * Exit out of the radio power save if we are in it
 */
void
wlc_radio_pwrsave_exit_mode(wlc_ap_info_t *ap)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;

	appvt->radio_pwrsave.pwrsave.quiet_time_counter = 0;
	appvt->radio_pwrsave.pwrsave.in_power_save = FALSE;
	wl_del_timer(wlc->wl, appvt->radio_pwrsave.timer);
	if (appvt->radio_pwrsave.radio_disabled) {
		wlc_bmac_radio_hw(wlc->hw, TRUE,
			(CHIPID(wlc->hw->sih->chip) == BCM5356_CHIP_ID) ?  TRUE : FALSE);
		appvt->radio_pwrsave.radio_disabled = FALSE;
	}
	appvt->radio_pwrsave.pwrsave_state = 0;
	appvt->radio_pwrsave.cncl_bcn = FALSE;
}

#ifdef WL11H
/*
 * Update the beacon with quiet IE
 */
static void
wlc_radio_pwrsave_update_quiet_ie(wlc_ap_info_t *ap, uint8 count)
{
	wlc_ap_info_pvt_t* appvt = (wlc_ap_info_pvt_t*) ap;
	wlc_info_t *wlc = appvt->wlc;
	uint32 duration;
	wlc_bsscfg_t *cfg = wlc->cfg;
	wlc_csa_t *csa = cfg->csa;

	duration = appvt->radio_pwrsave.off_time;
	duration *= DOT11_TU_TO_US;
	duration -= (ap->pre_tbtt_us + PHY_ENABLE_MAX_LATENCY_us +
	             PHY_DISABLE_MAX_LATENCY_us);
	duration /= DOT11_TU_TO_US;

	/* Setup the quiet command */
	csa->quiet_cmd.count = count;
	csa->quiet_cmd.duration = (uint16)duration;
	csa->quiet_cmd.offset = appvt->radio_pwrsave.on_time % cfg->current_bss->beacon_period;
	WL_INFORM(("wl%d: quiet cmd: count %d, dur %d, offset %d\n",
	            wlc->pub->unit, csa->quiet_cmd.count,
	            csa->quiet_cmd.duration, csa->quiet_cmd.offset));

	/* Update the beacon/probe response */
	csa->spect_state |= NEED_TO_UPDATE_BCN;
	wlc_bss_update_beacon(wlc, cfg);
	wlc_bss_update_probe_resp(wlc, cfg, TRUE);
#endif /* WL11H */
}
#endif /* RADIO_PWRSAVE */

#ifdef WL_AP_TPC
static int
wlc_ap_tpc_setup(wlc_info_t *wlc, uint8 mode)
{

	if (WL11H_ENAB(wlc)) {
		if (mode > 3)
			return BCME_RANGE;

		if (!wlc->ap_tpc_interval)
			wlc->ap_tpc_interval = 3;

		wlc->ap_tpc = mode;

		/* reset BSS pwr and AP pwr before enabling new mode. */
		if (AP_ONLY(wlc->pub)) {
			wlc->pwr_constraint = 0;
			wlc->ap_tpc_lm = (AP_TPC_MAX_LINK_MARGIN << 8) | AP_TPC_MAX_LINK_MARGIN;
			if (wlc->pub->associated) {
				wlc_update_beacon(wlc);
				wlc_update_probe_resp(wlc, TRUE);
			}

			wlc->txpwr_local_constraint = 0;
			if (wlc->pub->up && wlc->pub->associated &&
			    (wlc->chanspec == wlc->home_chanspec)) {
				wlc_channel_set_txpower_limit(wlc->cmi,
					wlc_local_constraint_qdbm(wlc));
			}
		}
		return BCME_OK;
	} else {
		return BCME_UNSUPPORTED;
	}
}

void
wlc_ap_tpc(wlc_info_t *wlc)
{
	wlc_bsscfg_t *cfg = wlc->cfg;
	struct scb_iter scbiter;
	struct scb *scb = NULL;
	uint8 prev_pwr_constraint, prev_txpwr_local_constraint;
	int8 sta_least_link_margin, ap_least_link_margin;

	sta_least_link_margin = AP_TPC_MAX_LINK_MARGIN;
	ap_least_link_margin = AP_TPC_MAX_LINK_MARGIN;

	if (WL11H_ENAB(wlc) && wlc->ap_tpc && BSSCFG_AP(cfg) && cfg->up && AP_ONLY(wlc->pub)) {

		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if ((SCB_ASSOCIATED(scb)) && (scb->bsscfg == cfg) &&
				(scb->cap & DOT11_CAP_SPECTRUM)) {

				/* send TPC request to all assoc STAs.
				 */
				if (!ETHER_ISMULTI(&scb->ea))
					wlc_send_tpc_request(wlc, &scb->ea);

				/* reset the margins when channel switch is
				 * pending.
				 */
				if (cfg->csa->spect_state & NEED_TO_SWITCH_CHANNEL) {
					scb->sta_link_margin = AP_TPC_MAX_LINK_MARGIN;
					scb->ap_link_margin = AP_TPC_MAX_LINK_MARGIN;
				}

				/* now record the least link margin
				 * from previous reports.
				 */
				if (sta_least_link_margin >= scb->sta_link_margin)
					sta_least_link_margin = scb->sta_link_margin;

				/* find the least link margin AP has
				 * with respect all the associated
				 * STAs.
				 */
				if (ap_least_link_margin >= scb->ap_link_margin)
					ap_least_link_margin = scb->ap_link_margin;

			}
		}

		/* record the link margin info for record keeping. */
		wlc->ap_tpc_lm = ((ap_least_link_margin << 8) & 0xff00) |
			(sta_least_link_margin & 0xff);

		/* reduce the AP power if stas have better link
		 * margin.
		 */
		if (wlc->ap_tpc == AP_TPC_AP_PWR || wlc->ap_tpc == AP_TPC_AP_BSS_PWR) {

			prev_txpwr_local_constraint = wlc->txpwr_local_constraint;

			/* now update the bcn and probe responses with new pwr
			 * constriant.
			 */
			if (sta_least_link_margin == AP_TPC_MAX_LINK_MARGIN) {
				wlc->txpwr_local_constraint = 0;
			} else if (sta_least_link_margin >= 9) {
				wlc->txpwr_local_constraint = 6;
			} else if (sta_least_link_margin >= 6) {
				wlc->txpwr_local_constraint = 3;
			} else {
				wlc->txpwr_local_constraint = 0;
			}

			WL_REGULATORY(("wl%d:%s STAs least link margin:%d "
				"txpwr_local_constraint:%d \n", wlc->pub->unit, __FUNCTION__,
				sta_least_link_margin, wlc->txpwr_local_constraint));

			if (wlc->txpwr_local_constraint != prev_txpwr_local_constraint) {

				wlc->txpwr_local_constraint &= 0xff;

				/* only update power targets if we are up and on the BSS
				 * home channel.
				 */
				if (wlc->chanspec == wlc->home_chanspec) {

					wlc_channel_set_txpower_limit(wlc->cmi,
						wlc_local_constraint_qdbm(wlc));
				}
			}
		}

		/* reduce the BS pwr based on how best link margin AP
		 * has.
		 */
		if (wlc->ap_tpc == AP_TPC_BSS_PWR || wlc->ap_tpc == AP_TPC_AP_BSS_PWR) {

			prev_pwr_constraint = wlc->pwr_constraint;

			if ((ap_least_link_margin == AP_TPC_MAX_LINK_MARGIN)) {
				wlc->pwr_constraint = 0;
			} else {
				if (ap_least_link_margin >= 9)
					wlc->pwr_constraint = 6;
				else if (ap_least_link_margin >= 6)
					wlc->pwr_constraint = 3;
				else
					wlc->pwr_constraint = 0;
			}

			WL_REGULATORY(("wl%d:%s APs least link margin:%d pwr_constraint:%d\n",
				wlc->pub->unit, __FUNCTION__,
				ap_least_link_margin, wlc->pwr_constraint));

			if (wlc->pwr_constraint != prev_pwr_constraint) {

				wlc->pwr_constraint &= 0xff;
				wlc_update_beacon(wlc);
				wlc_update_probe_resp(wlc, TRUE);
			}
		}
	}
}

#endif /* WL_AP_TPC */
