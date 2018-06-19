/*
 * ABGPHY module header file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_phy_n.h 278446 2011-08-18 23:21:43Z lekunlin $
 */

#ifndef _wlc_phy_n_h_
#define _wlc_phy_n_h_


#include <typedefs.h>

/* interference mitigation rssi values */
#define NPHY_NOISE_PWR_FIFO_DEPTH 6
#define PHY_RSSI_WINDOW_SZ 16
#define PHY_INTF_RSSI_INIT_VAL -60 /* -60: any number between -30 and -90 */
#define PHY_CRSMIN_ARRAY_MAX 3
#define PHY_CRSMIN_IDX_MAX (PHY_CRSMIN_ARRAY_MAX - 1)
#define PHY_CRSMIN_RANGE 3

#define PHY_CRSMIN_GE7_ACIOFF_2G -86
#define PHY_CRSMIN_GE7_ACION_2G -79
#define PHY_CRSMIN_LT7_ACIOFF_2G -82
#define PHY_CRSMIN_LT7_ACION_2G -74

#define PHY_CRSMIN_ACI2G_PWR_0 6
#define PHY_CRSMIN_ACI2G_PWR_1 18
#define PHY_CRSMIN_ACI2G_PWR_2 30

#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_2G -77
#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_2G_MAX -62
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_2G -70
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_2G_MAX -46

#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_5G -77
#define PHY_RFGAIN_RSSI_AVG_GE7_ACIOFF_5G_MAX -62
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_5G -77
#define PHY_RFGAIN_RSSI_AVG_GE7_ACION_5G_MAX -62

#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_2G -73
#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_2G_MAX -61
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_2G -65
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_2G_MAX -44

#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_5G -73
#define PHY_RFGAIN_RSSI_AVG_LT7_ACIOFF_5G_MAX -61
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_5G -73
#define PHY_RFGAIN_RSSI_AVG_LT7_ACION_5G_MAX -61

#define NPHY_GAIN_VS_TEMP_SLOPE_2G 6   /* units: db/100C */
#define NPHY_GAIN_VS_TEMP_SLOPE_5G 6   /* units: db/100C */

typedef struct _nphy_txpwrindex {
	int8   index;
	int8   index_internal;     /* store initial or user specified txpwr index */
	int8   index_internal_save;
	uint16 AfectrlOverride;
	uint16 AfeCtrlDacGain;
	uint16 rad_gain;
	uint8  bbmult;
	uint16 iqcomp_a;
	uint16 iqcomp_b;
	uint16 locomp;
} phy_txpwrindex_t;

typedef struct _nphy_pwrctrl {
	int8    max_pwr_2g;
	int8    idle_targ_2g;
	int16   pwrdet_2g_a1;
	int16   pwrdet_2g_b0;
	int16   pwrdet_2g_b1;
	int8    max_pwr_5gm;
	int8    idle_targ_5gm;
	int8    max_pwr_5gh;
	int8    max_pwr_5gl;
	int16   pwrdet_5gm_a1;
	int16   pwrdet_5gm_b0;
	int16   pwrdet_5gm_b1;
	int16   pwrdet_5gl_a1;
	int16   pwrdet_5gl_b0;
	int16   pwrdet_5gl_b1;
	int16   pwrdet_5gh_a1;
	int16   pwrdet_5gh_b0;
	int16   pwrdet_5gh_b1;
#ifdef	WL_PPR_SUBBAND
	int16   pwrdet_5gll_a1;
	int16   pwrdet_5gll_b0;
	int16   pwrdet_5gll_b1;
	int16   pwrdet_5glh_a1;
	int16   pwrdet_5glh_b0;
	int16   pwrdet_5glh_b1;
	int16   pwrdet_5gml_a1;
	int16   pwrdet_5gml_b0;
	int16   pwrdet_5gml_b1;
	int16   pwrdet_5gmh_a1;
	int16   pwrdet_5gmh_b0;
	int16   pwrdet_5gmh_b1;
#endif	/* WL_PPR_SUBBAND */
	int8    idle_targ_5gl;
	int8    idle_targ_5gh;
#ifdef	WL_PPR_SUBBAND
	int8    idle_targ_5gll;
	int8    idle_targ_5glh;
	int8    idle_targ_5gml;
	int8    idle_targ_5gmh;
#endif	/* WL_PPR_SUBBAND */
	int8    idle_tssi_2g;
	int8    idle_tssi_5g;
	int8    idle_tssi;
	int16   a1;
	int16   b0;
	int16   b1;
#ifdef	WL_PPR_SUBBAND
/* Support Power Param Per Sub-band/Channel */
	chan_poparam_t	*poparam;
	int8		max_pwr_5gll;	/* CH36-48 */
	int8		max_pwr_5glh;	/* CH52-64 */
	int8		max_pwr_5gml;	/* CH100-120 */
	int8		max_pwr_5gmh;	/* CH124-140 */
#endif /* WL_PPR_SUBBAND */
} phy_pwrctrl_t;

typedef struct _nphy_noise_cal {
	/* Noise cal related variables */
	bool nphy_init_noise_cal_done;
	uint16 nphy_noise_measure_window;
	uint8 nphy_NPwr_MinLmt;
	uint8 nphy_NPwr_MaxLmt;
	uint32 nphy_noisepwr_fifo_Min[NPHY_NOISE_PWR_FIFO_DEPTH][PHY_CORE_MAX];
	uint32 nphy_noisepwr_fifo_Max[NPHY_NOISE_PWR_FIFO_DEPTH][PHY_CORE_MAX];
	uint8  nphy_noisepwr_fifo_filled;
	uint32 cmplx_pwr[PHY_CORE_MAX];
	uint8 nphy_NPwr_LGC_MinLmt;
	uint8 nphy_NPwr_LGC_MaxLmt;
	uint16 nphy_biq1_gain1_bfrNoiseCal;
	uint16 nphy_biq1_gain2_bfrNoiseCal;
	int16 nphy_biq1_gain1_afrNoiseCal;
	int16 nphy_biq1_gain2_afrNoiseCal;
	int8 nphy_nvar_baseline_offset0_bfrNoiseCal;
	int8 nphy_nvar_baseline_offset1_bfrNoiseCal;
	uint8 nphy_max_rxpo_change_lmt;
	uint16 nphy_biq1_gain1_Base;
	uint16 nphy_biq1_gain2_Base;
	int8 nphy_nvar_baseline_offset0_Base;
	int8 nphy_nvar_baseline_offset1_Base;
	uint16  crsminpwrthld_20L_Base;
	uint16  crsminpwrthld_20U_Base;
	uint16  crsminpwrthld_20L_AfrNoiseCal;
	uint16  crsminpwrthld_20U_AfrNoiseCal;
	uint8 nphy_max_listen_gain_change_lmt;
	int16 listen_rf_gain;
	int32 nv_offset[26];

} phy_noisecal_t;

struct phy_info_nphy {
	uint32 pstart; /* sample collect fifo begins */
	uint32 pstop;  /* sample collect fifo ends */
	uint32 pfirst; /* sample collect trigger begins */
	uint32 plast;  /* sample collect trigger ends */
	uint16  rfctrlIntc1_save;
	uint16  rfctrlIntc2_save;
	bool    phyhang_avoid;  /* nphy rev 3, make PHY receiver deaf before accessing tables */

	/* Entry Id in Rx2Tx seq table that has the CLR_RXTX_BIAS opcode */

	uint8   adj_pwr_tbl_nphy[ADJ_PWR_TBL_LEN];      /* Adjusted power table of NPHY */
	bool    nphy_papd_kill_switch_en; /* flag to indicate if lna kill switch is enabled */

	bool    nphy_anarxlpf_adjusted;
	uint16  nphy_rccal_value;
	uint16  nphy_crsminpwr[3];

	bool    nphy_crsminpwr_adjusted;
	bool    nphy_noisevars_adjusted;
	bool    nphy_base_nvars_adjusted;

#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG)
	bool    ntd_crs_adjusted;
	uint16  ntd_crsminpwr;
	uint16  ntd_initgain;
	bool    ntd_lowtxpwr;
#endif
	bool 	ntd_papdcal_dcs;

	bool    nphy_sample_play_lpf_bw_ctl_ovr;
	uint32  nphy_bb_mult_save;
	uint16  tx_rx_cal_radio_saveregs[24]; /* htphy uses this, too (can we use uint8?) */

	/* new flag to signal periodic_cal is running to blank radar */
	bool    nphy_rxcal_active;
	uint32  nphy_rxcalparams;
	bool    nphy_force_papd_cal;
	uint16  nphy_papd_tx_gain_at_last_cal[2]; /* Tx gain index at time of last papd cal */
	uint    nphy_papd_last_cal;     /* time of last papd cal */
	uint32  nphy_papd_recal_counter;
	uint8   nphy_papd_cal_gain_index[2];    /* Tx gain pga index used during last papd cal
						* For REVs>=7, the PAD index is stored in the
						* 2G band and the PGA index is stored in the
						* 5G band
						*/
	bool    nphy_papdcomp;

	uint8   nphy_txpid2g[PHY_CORE_MAX];
	uint8   nphy_txpid5g[PHY_CORE_MAX];
	uint8   nphy_txpid5gl[PHY_CORE_MAX];
	uint8   nphy_txpid5gh[PHY_CORE_MAX];
#ifdef	WL_PPR_SUBBAND
	uint8   nphy_txpid5gll[PHY_CORE_MAX];
	uint8   nphy_txpid5glh[PHY_CORE_MAX];
	uint8   nphy_txpid5gml[PHY_CORE_MAX];
	uint8   nphy_txpid5gmh[PHY_CORE_MAX];
#endif	/* WL_PPR_SUBBAND */
	uint8   tx_precal_tssi_radio_saveregs[PHY_CORE_NUM_2][4];

	/* Tx power indices/gains during nphy cal */
	uint8   nphy_cal_orig_pwr_idx[2];
	uint8   nphy_txcal_pwr_idx[2];
	uint8   nphy_rxcal_pwr_idx[2];
	uint16  nphy_cal_orig_tx_gain[2];
	nphy_txgains_t  nphy_cal_target_gain;
	uint16  nphy_txcal_bbmult;
	uint16  nphy_gmval;

	uint16  nphy_saved_bbconf;
	uint    nphy_deaf_count;
	/* Variable to store the value of phyreg NPHY_fineclockgatecontrol */
	uint16  nphy_fineclockgatecontrol;
	uint8   nphy_pabias;        /* override PA bias value, 0: no override */
	uint8   nphy_txpwr_idx_2G[3];    /* to store the power control txpwr index for 2G band */
	uint8   nphy_txpwr_idx_5G[3];    /* to store the power control txpwr index for 5G band */
	int16   nphy_papd_epsilon_offset[2];
	phy_txpwrindex_t nphy_txpwrindex[PHY_CORE_NUM_2]; /* independent override per core */
	phy_pwrctrl_t nphy_pwrctrl_info[PHY_CORE_NUM_2]; /* Tx pwr ctrl info per Tx chain */

	/* Draconian Power Limits for Sulley */ 
	int16 tssi_maxpwr_limit;
	int16 tssi_minpwr_limit;
	uint8 tssi_ladder_offset_maxpwr;
	uint8 tssi_ladder_offset_minpwr;

	chanspec_t      nphy_rssical_chanspec_2G; /* 0: invalid, other: last valid cal chanspec */
	chanspec_t      nphy_rssical_chanspec_5G; /* 0: invalid, other: last valid cal chanspec */
	bool nphy_use_int_tx_iqlo_cal; /* Flag to determine if the Tx IQ/LO cal should be performed
					* using the radio's internal envelope detectors.
					*/
	bool nphy_int_tx_iqlo_cal_tapoff_intpa; /* Flag to determine whether the internal Tx
						* IQ/LO cal would be use the signal from the
						* radio's internal envelope detector at the
						* PAD tapoff or the intPA tapoff point.
						*/
	int8 nphy_cck_pwr_err_adjust;
	/* interference mitigation rssi vars */
	int16 intf_rssi_vals[PHY_RSSI_WINDOW_SZ];
	int16 intf_rssi_window_idx;
	int16 intf_rssi_avg;

	int16 crsmin_rssi_avg_acioff_2G;
	int16 crsmin_rssi_avg_acion_2G;
	int16 crsmin_pwr_aci2g[PHY_CRSMIN_ARRAY_MAX];

	int16 rfgain_rssi_avg_acioff_2G;
	int16 rfgain_rssi_avg_acioff_2G_max;
	int16 rfgain_rssi_avg_acioff_5G;
	int16 rfgain_rssi_avg_acioff_5G_max;

	int16 rfgain_rssi_avg_acion_2G;
	int16 rfgain_rssi_avg_acion_2G_max;
	int16 rfgain_rssi_avg_acion_5G;
	int16 rfgain_rssi_avg_acion_5G_max;

	uint8	elna2g;
	uint8	elna5g;

	bool	phy_isspuravoid;	/* TRUE if spur avoidance is ON for the current channel,
					 * else FALSE
					 */

	txiqcal_cache_t 		nphy_calibration_cache;
	rssical_cache_t			nphy_rssical_cache;

	uint16	tx_rx_cal_phy_saveregs[16];
	/* buffers used for ch11 40MHz spur avoidance WAR for 4322 */
	bool	nphy_gband_spurwar_en;
	bool	do_initcal;		/* to enable/disable phy init cal */
	uint8	cal_type_override;      /* cal override set from command line */
	uint8	nphy_papd_skip;     /* skip papd calibration for IPA case */

	chanspec_t	nphy_iqcal_chanspec_2G;	/* 0: invalid, other: last valid cal chanspec */
	chanspec_t	nphy_iqcal_chanspec_5G;	/* 0: invalid, other: last valid cal chanspec */

	uint8	nphy_papd_cal_type;
	bool	nphy_gband_spurwar2_en;
	bool	nphy_aband_spurwar_en;
	phy_noisevar_buf_t	nphy_saved_noisevars;

	/* Entry Id in Rx2Tx seq table that has the CLR_RXTX_BIAS opcode */
	int8	rx2tx_biasentry;


	uint16 orig_rfctrloverride[2];
	uint16 orig_rfctrlauxreg[2];
	uint16 orig_rxlpf_rccal_hpc_ovr_val;
	bool is_orig;
#ifdef NOISE_CAL_LCNXNPHY
	phy_noisecal_t nphy_noisecalvars;
#endif
};


#endif /* _wlc_phy_n_h_ */
