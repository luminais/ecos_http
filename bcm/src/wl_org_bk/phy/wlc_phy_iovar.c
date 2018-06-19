/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abg
 * PHY iovar processing of Broadcom BCM43XX 802.11abg
 * Networking Device Driver.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_phy_iovar.c,v 1.75.2.25 2011-02-09 02:17:03 Exp $
 */

/*
 * This file contains high portion PHY iovar processing and table.
 */

#include <wlc_phy_iovar.h>
#include <wlc_cfg.h>
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_phy_radio.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <hndpmu.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wl_dbg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wl_export.h>
#include <sbsprom.h>
#include <bcmwifi.h>

const bcm_iovar_t phy_iovars[] = {
	/* OLD, PHYTYPE specific iovars, to phase out, use unified ones at the end of this array */

#if NCONF	    /* move some to internal ?? */
#if defined(BCMDBG)
	{"nphy_initgain", IOV_NPHY_INITGAIN,
	IOVF_SET_UP, IOVT_UINT16, 0
	},
	{"nphy_hpv1gain", IOV_NPHY_HPVGA1GAIN,
	IOVF_SET_UP, IOVT_INT8, 0
	},
	{"nphy_tx_temp_tone", IOV_NPHY_TX_TEMP_TONE,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
	{"nphy_cal_reset", IOV_NPHY_CAL_RESET,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
	{"nphy_est_tonepwr", IOV_NPHY_EST_TONEPWR,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"nphy_rfseq_txgain", IOV_NPHY_RFSEQ_TXGAIN,
	IOVF_GET_UP, IOVT_INT32, 0
	},
#endif /* BCMDBG */
#endif /* NCONF */

#if defined(WLTEST)
#if NCONF
	{"nphy_rssisel", IOV_NPHY_RSSISEL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_txiqlocal", IOV_NPHY_TXIQLOCAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_rxiqcal", IOV_NPHY_RXIQCAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_rssical", IOV_NPHY_RSSICAL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_gain_boost", IOV_NPHY_GAIN_BOOST,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_spuravoid", IOV_PHY_SPURAVOID,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_cckpwr_offset", IOV_NPHY_CCK_PWR_OFFSET,
	(IOVF_SET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_papdcal", IOV_NPHY_PAPDCAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_pacaltype", IOV_NPHY_PAPDCALTYPE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_skippapd", IOV_NPHY_SKIPPAPD,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_pacalindex", IOV_NPHY_PAPDCALINDEX,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"nphy_aci_scan", IOV_NPHY_ACI_SCAN,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"nphy_tbldump_minidx", IOV_NPHY_TBLDUMP_MINIDX,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_tbldump_maxidx", IOV_NPHY_TBLDUMP_MAXIDX,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_phyreg_skipdump", IOV_NPHY_PHYREG_SKIPDUMP,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"nphy_phyreg_skipcount", IOV_NPHY_PHYREG_SKIPCNT,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_caltxgain", IOV_NPHY_CALTXGAIN,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"nphy_cal_sanity", IOV_NPHY_CAL_SANITY,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
#endif /* NCONF */

#if LCNCONF
	{"lcnphy_ldovolt", IOV_LCNPHY_LDOVOLT,
	(IOVF_SET_UP), IOVT_UINT32, 0
	},
	{"lcnphy_papdepstbl", IOV_LCNPHY_PAPDEPSTBL,
	(IOVF_GET_UP | IOVF_MFG), IOVT_BUFFER, 0
	},
#endif

#if LPCONF
	{"lpphy_tx_tone", IOV_LPPHY_TX_TONE,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"lpphy_txiqlocal", IOV_LPPHY_TXIQLOCAL,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"lpphy_rxiqcal", IOV_LPPHY_RXIQCAL,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"lpphy_fullcal", IOV_LPPHY_FULLCAL,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"lpphy_pacal", IOV_LPPHY_PAPDCAL,
	(IOVF_GET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"lpphy_pacaltype", IOV_LPPHY_PAPDCALTYPE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"lpphy_txpwrctrl", IOV_LPPHY_TXPWRCTRL,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"lpphy_txpwrindex", IOV_LPPHY_TXPWRINDEX,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"lpphy_crs", IOV_LPPHY_CRS,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_BOOL, 0
	},
	{"lpphy_pa_slow_cal", IOV_LPPHY_PAPD_SLOW_CAL,
	(IOVF_MFG), IOVT_BOOL, 0
	},
	{"lpphy_pa_recal_min_interval", IOV_LPPHY_PAPD_RECAL_MIN_INTERVAL,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"lpphy_pa_recal_max_interval", IOV_LPPHY_PAPD_RECAL_MAX_INTERVAL,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"lpphy_pa_recal_gain_delta", IOV_LPPHY_PAPD_RECAL_GAIN_DELTA,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"lpphy_pa_recal_enable", IOV_LPPHY_PAPD_RECAL_ENABLE,
	(IOVF_MFG), IOVT_BOOL, 0
	},
	{"lpphy_pa_recal_counter", IOV_LPPHY_PAPD_RECAL_COUNTER,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"lpphy_cck_dig_filt_type", IOV_LPPHY_CCK_DIG_FILT_TYPE,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"lpphy_ofdm_dig_filt_type", IOV_LPPHY_OFDM_DIG_FILT_TYPE,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"lpphy_txrf_sp_9", IOV_LPPHY_TXRF_SP_9_OVR,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"lpphy_aci_on_thresh", IOV_LPPHY_ACI_ON_THRESH,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"lpphy_aci_off_thresh", IOV_LPPHY_ACI_OFF_THRESH,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"lpphy_aci_on_timeout", IOV_LPPHY_ACI_ON_TIMEOUT,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"lpphy_aci_off_timeout", IOV_LPPHY_ACI_OFF_TIMEOUT,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"lpphy_aci_glitch_timeout", IOV_LPPHY_ACI_GLITCH_TIMEOUT,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"lpphy_aci_chan_scan_cnt", IOV_LPPHY_ACI_CHAN_SCAN_CNT,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"lpphy_aci_chan_scan_pwr_thresh", IOV_LPPHY_ACI_CHAN_SCAN_PWR_THRESH,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"lpphy_aci_chan_scan_cnt_thresh", IOV_LPPHY_ACI_CHAN_SCAN_CNT_THRESH,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"lpphy_aci_chan_scan_timeout", IOV_LPPHY_ACI_CHAN_SCAN_TIMEOUT,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"lpphy_noise_samples", IOV_LPPHY_NOISE_SAMPLES,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"lpphy_papdepstbl", IOV_LPPHY_PAPDEPSTBL,
	(IOVF_GET_UP | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"lpphy_txiqcc", IOV_LPPHY_TXIQCC,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER,  2*sizeof(int32)
	},
	{"lpphy_txlocc", IOV_LPPHY_TXLOCC,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 6
	},
	{"lpphy_idle_tssi_update_delta_temp", IOV_LPPHY_IDLE_TSSI_UPDATE_DELTA_TEMP,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"ofdm_analog_filt_bw_override", IOV_LPPHY_OFDM_ANALOG_FILT_BW_OVERRIDE,
	(IOVF_MFG), IOVT_INT16, 0
	},
	{"cck_analog_filt_bw_override", IOV_LPPHY_CCK_ANALOG_FILT_BW_OVERRIDE,
	(IOVF_MFG), IOVT_INT16, 0
	},
	{"ofdm_rccal_override", IOV_LPPHY_OFDM_RCCAL_OVERRIDE,
	(IOVF_MFG), IOVT_INT16, 0
	},
	{"cck_rccal_override", IOV_LPPHY_CCK_RCCAL_OVERRIDE,
	(IOVF_MFG), IOVT_INT16, 0
	},
#endif /* LPCONF */
#if SSLPNCONF
	{"sslpnphy_tx_tone", IOV_SSLPNPHY_TX_TONE,
	IOVF_SET_UP, IOVT_UINT32, 0
	},
	/*
	{"sslpnphy_cckfiltsel", IOV_SSLPNPHY_CCKFILTSEL,
	0, IOVT_UINT32, 0
	},
	*/
#ifdef BCMDBG
	{"sslpnphy_papd_cal", IOV_SSLPNPHY_PAPDCAL,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"sslpnphy_txpwr_init", IOV_SSLPNPHY_TXPWRINIT,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"sslpnphy_txiqcal", IOV_SSLPNPHY_TXIQLOCAL,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"sslpnphy_rxiqcal", IOV_SSLPNPHY_RXIQCAL,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"sslpnphy_percal", IOV_SSLPNPHY_PER_CAL,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"sslpnphy_vcocal", IOV_SSLPNPHY_VCO_CAL,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	/*
	{"sslpnphy_percal_debug", IOV_SSLPNPHY_PERCAL_DEBUG,
	IOVF_GET_UP, IOVT_BUFFER, sizeof(wl_sslpnphy_percal_debug_data_t)
	},
	*/
#endif /* BCMDBG */
	/*
	{"sslpnphy_rcpi", IOV_SSLPNPHY_RCPI,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"sslpnphy_tempsense", IOV_SSLPNPHY_TEMPSENSE,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"sslpnphy_vbatsense", IOV_SSLPNPHY_VBATSENSE,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"sslpnphy_papd_debug", IOV_SSLPNPHY_PAPD_DEBUG,
	IOVF_GET_UP, IOVT_BUFFER, sizeof(wl_sslpnphy_papd_debug_data_t)
	},
	{"sslpnphy_papd_dumplut", IOV_SSLPNPHY_PAPD_DUMPLUT,
	IOVF_GET_UP, IOVT_BUFFER, sizeof(wl_sslpnphy_debug_data_t)
	},
	{"sslpnphy_spbdump", IOV_SSLPNPHY_SPBDUMP,
	IOVF_GET_UP, IOVT_BUFFER, sizeof(wl_sslpnphy_spbdump_data_t)
	},
	*/
	{"sslpnphy_spbrun", IOV_SSLPNPHY_SPBRUN,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
	{"sslpnphy_spbrange", IOV_SSLPNPHY_SPBRANGE,
	IOVF_SET_UP, IOVT_BUFFER, 2*sizeof(uint16)
	},
	{"sslpnphy_txpwrindex", IOV_SSLPNPHY_TXPWRINDEX,
	IOVF_SET_UP | IOVF_GET_UP, IOVT_INT8, 0
	},
	{"sslpnphy_paparams", IOV_SSLPNPHY_PAPARAMS,
	IOVF_SET_UP, IOVT_BUFFER, 3*sizeof(int32)
	},
	{"sslpnphy_fullcal", IOV_SSLPNPHY_FULLCAL,
	IOVF_GET_UP, IOVT_BUFFER, 0
	},
	{"sslpnphy_cga", IOV_SSLPNPHY_CGA,
	IOVF_SET_UP | IOVF_GET_UP, IOVT_INT8, 0
	},
	{"sslpnphy_cga_5g", IOV_SSLPNPHY_CGA_5G,
	IOVF_SET_UP, IOVT_BUFFER, 24*sizeof(int8)
	},
	{"sslpnphy_cga_2g", IOV_SSLPNPHY_CGA_2G,
	IOVF_SET_UP, IOVT_BUFFER, 14*sizeof(int8)
	},
	{"sslpnphy_tx_iqcc", IOV_SSLPNPHY_TX_IQCC,
	IOVF_SET_UP, IOVT_BUFFER, 2*sizeof(int8)
	},
	{"sslpnphy_tx_locc", IOV_SSLPNPHY_TX_LOCC,
	IOVF_SET_UP, IOVT_BUFFER, 6*sizeof(int8)
	},
#endif /* SSLPNCONF */
#endif	

#if LPCONF
	{"lpphy_tempsense", IOV_LPPHY_TEMPSENSE,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lpphy_cal_delta_temp", IOV_LPPHY_CAL_DELTA_TEMP,
	0, IOVT_INT32, 0
	},
	{"lpphy_vbatsense", IOV_LPPHY_VBATSENSE,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"lpphy_rx_gain_temp_adj_tempsense", IOV_LPPHY_RX_GAIN_TEMP_ADJ_TEMPSENSE,
	(0), IOVT_INT8, 0
	},
	{"lpphy_rx_gain_temp_adj_thresh", IOV_LPPHY_RX_GAIN_TEMP_ADJ_THRESH,
	(0), IOVT_UINT32, 0
	},
	{"lpphy_rx_gain_temp_adj_metric", IOV_LPPHY_RX_GAIN_TEMP_ADJ_METRIC,
	(0), IOVT_INT16, 0
	},
#endif /* LPCONF */

	/* ==========================================
	 * unified phy iovar, independent of PHYTYPE
	 * ==========================================
	 */
#if defined(AP) && defined(RADAR)
	{"radarargs", IOV_RADAR_ARGS,
	(0), IOVT_BUFFER, sizeof(wl_radar_args_t)
	},
	{"radarargs40", IOV_RADAR_ARGS_40MHZ,
	(0), IOVT_BUFFER, sizeof(wl_radar_args_t)
	},
	{"radarthrs", IOV_RADAR_THRS,
	(IOVF_SET_UP), IOVT_BUFFER, sizeof(wl_radar_thr_t)
	},
#if defined(BCMDBG) || defined(WLTEST)
	{"phy_dfs_lp_buffer", IOV_PHY_DFS_LP_BUFFER,
	0, IOVT_UINT8, 0
	},
#endif 
#endif /* AP && RADAR */
#if defined(BCMDBG) || defined(WLTEST)
	{"fast_timer", IOV_FAST_TIMER,
	(IOVF_NTRL | IOVF_MFG), IOVT_UINT32, 0
	},
	{"slow_timer", IOV_SLOW_TIMER,
	(IOVF_NTRL | IOVF_MFG), IOVT_UINT32, 0
	},
#endif /* BCMDBG || WLTEST */
#if defined(BCMDBG) || defined(WLTEST) || defined(WLMEDIA_CALDBG)
	{"glacial_timer", IOV_GLACIAL_TIMER,
	(IOVF_NTRL | IOVF_MFG), IOVT_UINT32, 0
	},
#endif  /* BCMDBG || WLTEST || WLMEDIA_CALDBG */
#if defined(WLTEST)
	{"txinstpwr", IOV_TXINSTPWR,
	(IOVF_GET_CLK | IOVF_GET_BAND | IOVF_MFG), IOVT_BUFFER, sizeof(tx_inst_power_t)
	},
#endif

#ifdef SAMPLE_COLLECT
	{"sample_collect", IOV_PHY_SAMPLE_COLLECT,
	(IOVF_GET_DOWN | IOVF_GET_CLK), IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
	{"sample_data", IOV_PHY_SAMPLE_DATA,
	(IOVF_GET_DOWN | IOVF_GET_CLK), IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
#endif
	{"phy_muted", IOV_PHY_MUTED,
	0, IOVT_BOOL, 0
	},
#ifdef WLTEST
	{"fem2g", IOV_PHY_FEM2G,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"fem5g", IOV_PHY_FEM5G,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"maxpower", IOV_PHY_MAXP,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, 0
	},
#endif /* WLTEST */
#if defined(WLTEST) || defined(WLMEDIA_CALDBG)
	{"phy_cal_disable", IOV_PHY_CAL_DISABLE,
	(IOVF_MFG), IOVT_BOOL, 0
	},
#endif 
#if defined(WLTEST)
	{"phymsglevel", IOV_PHYHAL_MSG,
	(0), IOVT_UINT32, 0
	},
#endif
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV) || \
	defined(MACOSX)
	{"phy_watchdog", IOV_PHY_WATCHDOG,
	(IOVF_MFG), IOVT_BOOL, 0
	},
#endif
#if defined(WLTEST)
	{"phy_fixed_noise", IOV_PHY_FIXED_NOISE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phynoise_polling", IOV_PHYNOISE_POLL,
	(IOVF_MFG), IOVT_BOOL, 0
	},
	{"carrier_suppress", IOV_CARRIER_SUPPRESS,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BOOL, 0
	},
	{"unmod_rssi", IOV_UNMOD_RSSI,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"pkteng_stats", IOV_PKTENG_STATS,
	(IOVF_GET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(wl_pkteng_stats_t)
	},
	{"phytable", IOV_PHYTABLE,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 4*4
	},
	{"aci_exit_check_period", IOV_ACI_EXIT_CHECK_PERIOD,
	(IOVF_MFG), IOVT_UINT32, 0
	},
	{"pavars", IOV_PAVARS,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, WL_PHY_PAVARS_LEN * sizeof(uint16)
	},
	{"pavars2", IOV_PAVARS2,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, sizeof(wl_pavars2_t)
	},
	{"povars", IOV_POVARS,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_BUFFER, sizeof(wl_po_t)
	},
#endif 
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV)
	{"phy_forcecal", IOV_PHY_FORCECAL,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
#endif
#if defined(WLTEST)
	{"tssical_start_idx", IOV_TSSICAL_START_IDX,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(int)
	},
	{"tssical_start", IOV_TSSICAL_START,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(int)
	},
	{"tssical_power", IOV_TSSICAL_POWER,
	(IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, sizeof(int)
	},
	{"tssical_params", IOV_TSSICAL_PARAMS,
	(IOVF_GET_UP | IOVF_MFG), IOVT_BUFFER, 4*sizeof(int64)
	},
	{"tssical_txdelay", IOV_PHY_TSSITXDELAY,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_UINT32, 0
	},
	{"phy_deaf", IOV_PHY_DEAF,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_txpwrctrl", IOV_PHY_TXPWRCTRL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_txpwrindex", IOV_PHY_TXPWRINDEX,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 0
	},
	{"phy_txrx_chain", IOV_PHY_TXRX_CHAIN,
	(0), IOVT_INT8, 0
	},
	{"phy_txiqcc", IOV_PHY_TXIQCC,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER,  2*sizeof(int32)
	},
	{"phy_txlocc", IOV_PHY_TXLOCC,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), IOVT_BUFFER, 6
	},
	{"phy_bphy_evm", IOV_PHY_BPHY_EVM,
	(IOVF_SET_DOWN | IOVF_SET_BAND | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_bphy_rfcs", IOV_PHY_BPHY_RFCS,
	(IOVF_SET_DOWN | IOVF_SET_BAND | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_enrxcore", IOV_PHY_ENABLERXCORE,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_est_tonepwr", IOV_PHY_EST_TONEPWR,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"phy_gpiosel", IOV_PHY_GPIOSEL,
	(IOVF_MFG), IOVT_UINT16, 0
	},
	{"phy_5g_pwrgain", IOV_PHY_5G_PWRGAIN,
	(IOVF_SET_DOWN | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_rfseq", IOV_PHY_RFSEQ,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_scraminit", IOV_PHY_SCRAMINIT,
	(IOVF_SET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_tempoffset", IOV_PHY_TEMPOFFSET,
	(IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_test_tssi", IOV_PHY_TEST_TSSI,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_test_tssi_offs", IOV_PHY_TEST_TSSI_OFFS,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), IOVT_INT8, 0
	},
	{"phy_tx_tone", IOV_PHY_TX_TONE,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_tx_tone_hz", IOV_PHY_TX_TONE_HZ,
	(IOVF_SET_UP | IOVF_MFG), IOVT_UINT32, 0
	},
	{"phy_activecal", IOV_PHY_ACTIVECAL,
	IOVF_GET_UP, IOVT_BOOL, 0
	},
#endif 
	{"phy_tempthresh", IOV_PHY_TEMPTHRESH,
	(IOVF_MFG), IOVT_INT16, 0
	},
#ifdef PHYMON
	{"phycal_state", IOV_PHYCAL_STATE,
	IOVF_GET_UP, IOVT_UINT32, 0,
	},
#endif /* PHYMON */
#if defined(WLTEST) || defined(AP)
	{"phy_percal", IOV_PHY_PERICAL,
	(IOVF_MFG), IOVT_UINT8, 0
	},
#endif 

	{"phy_rxiqest", IOV_PHY_RXIQ_EST,
	IOVF_SET_UP, IOVT_UINT32, IOVT_UINT32
	},
	{"phynoise_srom", IOV_PHYNOISE_SROM,
	IOVF_GET_UP, IOVT_UINT32, 0
	},
	{"num_stream", IOV_NUM_STREAM,
	(0), IOVT_INT32, 0
	},
	{"band_range", IOV_BAND_RANGE,
	0, IOVT_INT8, 0
	},
	{"subband5gver", IOV_PHY_SUBBAND5GVER,
	0, IOVT_INT8, 0
	},
	{"min_txpower", IOV_MIN_TXPOWER,
	0, IOVT_UINT32, 0
	},
#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX)
	{"phy_tempsense", IOV_PHY_TEMPSENSE,
	IOVF_GET_UP, IOVT_INT16, 0
	},
#endif
#if defined(BCMDBG) || defined(WLTEST)
	{"phy_vbatsense", IOV_PHY_VBATSENSE,
	IOVF_GET_UP, IOVT_INT32, 0
	},
#endif
#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG)
	{"phy_rxdesens", IOV_PHY_RXDESENS,
	IOVF_GET_UP, IOVT_INT32, 0
	},
	{"ntd_gds_lowtxpwr", IOV_NTD_GDS_LOWTXPWR,
	IOVF_GET_UP, IOVT_BOOL, 0
	},
	{"phycal_tempdelta", IOV_PHYCAL_TEMPDELTA,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"papdcal_indexdelta", IOV_PAPDCAL_INDEXDELTA,
	IOVF_GET_UP, IOVT_UINT8, 0
	},
#endif
#ifdef WLNINTENDO2
	{"phy_txfilter_override", IOV_PHY_TXFILTER_SM_OVERRIDE,
	IOVF_SET_DOWN, IOVT_INT32, 0
	},
#endif /* WLNINTENDO2 */
#if defined(BCMDBG)
#if LCNCONF
	{"lcnphy_cwtxpwrctrl", IOV_LCNPHY_CWTXPWRCTRL,
	IOVF_MFG, IOVT_UINT8, 0
	},
#endif /* #if LCNCONF */
#endif 
	{"phy_oclscdenable", IOV_PHY_OCLSCDENABLE,
	(IOVF_MFG), IOVT_UINT8, 0
	},
	{"phy_rxantsel", IOV_PHY_RXANTSEL,
	(0), IOVT_BOOL, 0
	},
#ifdef ENABLE_FCBS
	{"phy_fcbs_init", IOV_PHY_FCBSINIT,
	(IOVF_SET_UP), IOVT_INT8, 0
	},
	{"phy_fcbs", IOV_PHY_FCBS,
	(IOVF_SET_UP | IOVF_GET_UP), IOVT_INT8, 0
	},
#endif /* ENABLE_FCBS */
	{"phy_crs_war", IOV_PHY_CRS_WAR,
	(0), IOVT_INT8, 0
	},
	/* terminating element, only add new before this */
	{NULL, 0, 0, 0, 0 }
};

static int
wlc_phy_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;

	return wlc_bmac_phy_iovar_dispatch(wlc->hw, actionid, vi->type, p, plen, a, alen, vsize);
}

/* PHY is no longer a wlc.c module. It's slave to wlc_bmac.c
 * still use module_register to move iovar processing to HIGH driver to save dongle space
 */
int
BCMATTACHFN(wlc_phy_iovar_attach)(void *pub)
{
	/* register module */
	if (wlc_module_register((wlc_pub_t*)pub, phy_iovars, "phy", ((wlc_pub_t*)pub)->wlc,
		wlc_phy_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("%s: wlc_phy_iovar_attach failed!\n", __FUNCTION__));
		return -1;
	}
	return 0;
}

void
BCMATTACHFN(wlc_phy_iovar_detach)(void *pub)
{
	wlc_module_unregister(pub, "phy", ((wlc_pub_t*)pub)->wlc);
}
