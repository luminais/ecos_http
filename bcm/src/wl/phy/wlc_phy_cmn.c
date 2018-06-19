/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11 Networking Device Driver.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_phy_cmn.c 280945 2011-08-31 21:40:48Z pamini $
 */


#include <wlc_cfg.h>
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>

#include <d11.h>

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phyreg_abg.h>
#include <wlc_phyreg_n.h>
#include <wlc_phyreg_ht.h>
#include <wlc_phyreg_lp.h>
#include <wlc_phyreg_ssn.h>
#include <wlc_phyreg_lcn.h>
#include <wlc_phyreg_lcn40.h>
#include <wlc_phytbl_n.h>
#include <wlc_phytbl_ht.h>
#include <wlc_phy_radio.h>
#include <wlc_phy_lcn.h>
#include <wlc_phy_lcn40.h>
#include <wlc_phy_lp.h>
#include <wlc_phy_ssn.h>
#include <wlc_phy_abg.h>
#include <wlc_phy_n.h>
#include <wlc_phy_ht.h>
#include <bcmwifi.h>
#include <bcmotp.h>

#ifdef WLNOKIA_NVMEM
#include <wlc_phy_noknvmem.h>
#endif /* WLNOKIA_NVMEM */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  macro, typedef, enum, structure, global variable		*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */


uint32 phyhal_msg_level = PHYHAL_ERROR;

/* channel info structure */
typedef struct _chan_info_basic {
	uint16	chan;		/* channel number */
	uint16	freq;		/* in Mhz */
} chan_info_basic_t;

static chan_info_basic_t chan_info_all[] = {
	/* 11b/11g */
/* 0 */		{1,	2412},
/* 1 */		{2,	2417},
/* 2 */		{3,	2422},
/* 3 */		{4,	2427},
/* 4 */		{5,	2432},
/* 5 */		{6,	2437},
/* 6 */		{7,	2442},
/* 7 */		{8,	2447},
/* 8 */		{9,	2452},
/* 9 */		{10,	2457},
/* 10 */	{11,	2462},
/* 11 */	{12,	2467},
/* 12 */	{13,	2472},
/* 13 */	{14,	2484},

#ifdef BAND5G
/* 11a japan high */
/* 14 */	{34,	5170},
/* 15 */	{38,	5190},
/* 16 */	{42,	5210},
/* 17 */	{46,	5230},

/* 11a usa low */
/* 18 */	{36,	5180},
/* 19 */	{40,	5200},
/* 20 */	{44,	5220},
/* 21 */	{48,	5240},
/* 22 */	{52,	5260},
/* 23 */	{54,	5270},
/* 24 */	{56,	5280},
/* 25 */	{60,	5300},
/* 26 */	{62,	5310},
/* 27 */	{64,	5320},

/* 11a Europe */
/* 28 */	{100,	5500},
/* 29 */	{102,	5510},
/* 30 */	{104,	5520},
/* 31 */	{108,	5540},
/* 31 */	{110,	5550},
/* 32 */	{112,	5560},
/* 33 */	{116,	5580},
/* 34 */	{118,	5590},
/* 35 */	{120,	5600},
/* 36 */	{124,	5620},
/* 37 */	{126,	5630},
/* 38 */	{128,	5640},
/* 39 */	{132,	5660},
/* 39 */	{134,	5660},
/* 40 */	{136,	5680},
/* 41 */	{140,	5700},

/* 11a usa high, ref5 only */
/* 42 */	{149,	5745},
/* 43 */	{151,	5755},
/* 44 */	{153,	5765},
/* 45 */	{157,	5785},
/* 46 */	{159,	5795},
/* 47 */	{161,	5805},
/* 48 */	{165,	5825},

/* 11a japan */
/* 49 */	{184,	4920},
/* 50 */	{185,	4925},
/* 51 */	{187,	4935},
/* 52 */	{188,	4940},
/* 53 */	{189,	4945},
/* 54 */	{192,	4960},
/* 55 */	{196,	4980},
/* 56 */	{200,	5000},
/* 57 */	{204,	5020},
/* 58 */	{207,	5035},
/* 59 */	{208,	5040},
/* 60 */	{209,	5045},
/* 61 */	{210,	5050},
/* 62 */	{212,	5060},
/* 63 */	{216,	5080}
#endif /* BAND5G */
};

uint16 ltrn_list[PHY_LTRN_LIST_LEN] = {
	0x18f9, 0x0d01, 0x00e4, 0xdef4, 0x06f1, 0x0ffc,
	0xfa27, 0x1dff, 0x10f0, 0x0918, 0xf20a, 0xe010,
	0x1417, 0x1104, 0xf114, 0xf2fa, 0xf7db, 0xe2fc,
	0xe1fb, 0x13ee, 0xff0d, 0xe91c, 0x171a, 0x0318,
	0xda00, 0x03e8, 0x17e6, 0xe9e4, 0xfff3, 0x1312,
	0xe105, 0xe204, 0xf725, 0xf206, 0xf1ec, 0x11fc,
	0x14e9, 0xe0f0, 0xf2f6, 0x09e8, 0x1010, 0x1d01,
	0xfad9, 0x0f04, 0x060f, 0xde0c, 0x001c, 0x0dff,
	0x1807, 0xf61a, 0xe40e, 0x0f16, 0x05f9, 0x18ec,
	0x0a1b, 0xff1e, 0x2600, 0xffe2, 0x0ae5, 0x1814,
	0x0507, 0x0fea, 0xe4f2, 0xf6e6
};

const uint8 ofdm_rate_lookup[] = {
	    /* signal */
	WLC_RATE_48M, /* 8: 48Mbps */
	WLC_RATE_24M, /* 9: 24Mbps */
	WLC_RATE_12M, /* A: 12Mbps */
	WLC_RATE_6M,  /* B:  6Mbps */
	WLC_RATE_54M, /* C: 54Mbps */
	WLC_RATE_36M, /* D: 36Mbps */
	WLC_RATE_18M, /* E: 18Mbps */
	WLC_RATE_9M   /* F:  9Mbps */
};
#ifdef	WL_PPR_SUBBAND
/* Support PA parameters per subband/channel */
chan_poparam_t	poparam_5gchantbl_core0[MAX_5G_CHANS_PHY5GSB_HIGHPWR_5B] = {
	/* FCC BAND1 */
	{ 34, 0x0000, 0x0000, 0x0000 },
	{ 36, 0x0000, 0x0000, 0x0000 },
	{ 38, 0x0000, 0x0000, 0x0000 },
	{ 40, 0x0000, 0x0000, 0x0000 },
	{ 42, 0x0000, 0x0000, 0x0000 },
	{ 44, 0x0000, 0x0000, 0x0000 },
	{ 46, 0x0000, 0x0000, 0x0000 },
	{ 48, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND2 */
	{ 52, 0x0000, 0x0000, 0x0000 },
	{ 54, 0x0000, 0x0000, 0x0000 },
	{ 56, 0x0000, 0x0000, 0x0000 },
	{ 60, 0x0000, 0x0000, 0x0000 },
	{ 62, 0x0000, 0x0000, 0x0000 },
	{ 64, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND3 */
	{ 100, 0x0000, 0x0000, 0x0000 },
	{ 102, 0x0000, 0x0000, 0x0000 },
	{ 104, 0x0000, 0x0000, 0x0000 },
	{ 108, 0x0000, 0x0000, 0x0000 },
	{ 110, 0x0000, 0x0000, 0x0000 },
	{ 112, 0x0000, 0x0000, 0x0000 },
	{ 116, 0x0000, 0x0000, 0x0000 },
	{ 118, 0x0000, 0x0000, 0x0000 },
	{ 120, 0x0000, 0x0000, 0x0000 },
	{ 124, 0x0000, 0x0000, 0x0000 },
	{ 126, 0x0000, 0x0000, 0x0000 },
	{ 128, 0x0000, 0x0000, 0x0000 },
	{ 132, 0x0000, 0x0000, 0x0000 },
	{ 134, 0x0000, 0x0000, 0x0000 },
	{ 136, 0x0000, 0x0000, 0x0000 },
	{ 140, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND4 */
	{ 149, 0x0000, 0x0000, 0x0000 },
	{ 151, 0x0000, 0x0000, 0x0000 },
	{ 153, 0x0000, 0x0000, 0x0000 },
	{ 157, 0x0000, 0x0000, 0x0000 },
	{ 159, 0x0000, 0x0000, 0x0000 },
	{ 161, 0x0000, 0x0000, 0x0000 },
	{ 165, 0x0000, 0x0000, 0x0000 }
};
chan_poparam_t	poparam_5gchantbl_core1[MAX_5G_CHANS_PHY5GSB_HIGHPWR_5B] = {
	/* FCC BAND1 */
	{ 34, 0x0000, 0x0000, 0x0000 },
	{ 36, 0x0000, 0x0000, 0x0000 },
	{ 38, 0x0000, 0x0000, 0x0000 },
	{ 40, 0x0000, 0x0000, 0x0000 },
	{ 42, 0x0000, 0x0000, 0x0000 },
	{ 44, 0x0000, 0x0000, 0x0000 },
	{ 46, 0x0000, 0x0000, 0x0000 },
	{ 48, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND2 */
	{ 52, 0x0000, 0x0000, 0x0000 },
	{ 54, 0x0000, 0x0000, 0x0000 },
	{ 56, 0x0000, 0x0000, 0x0000 },
	{ 60, 0x0000, 0x0000, 0x0000 },
	{ 62, 0x0000, 0x0000, 0x0000 },
	{ 64, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND3 */
	{ 100, 0x0000, 0x0000, 0x0000 },
	{ 102, 0x0000, 0x0000, 0x0000 },
	{ 104, 0x0000, 0x0000, 0x0000 },
	{ 108, 0x0000, 0x0000, 0x0000 },
	{ 110, 0x0000, 0x0000, 0x0000 },
	{ 112, 0x0000, 0x0000, 0x0000 },
	{ 116, 0x0000, 0x0000, 0x0000 },
	{ 118, 0x0000, 0x0000, 0x0000 },
	{ 120, 0x0000, 0x0000, 0x0000 },
	{ 124, 0x0000, 0x0000, 0x0000 },
	{ 126, 0x0000, 0x0000, 0x0000 },
	{ 128, 0x0000, 0x0000, 0x0000 },
	{ 132, 0x0000, 0x0000, 0x0000 },
	{ 134, 0x0000, 0x0000, 0x0000 },
	{ 136, 0x0000, 0x0000, 0x0000 },
	{ 140, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND4 */
	{ 149, 0x0000, 0x0000, 0x0000 },
	{ 151, 0x0000, 0x0000, 0x0000 },
	{ 153, 0x0000, 0x0000, 0x0000 },
	{ 157, 0x0000, 0x0000, 0x0000 },
	{ 159, 0x0000, 0x0000, 0x0000 },
	{ 161, 0x0000, 0x0000, 0x0000 },
	{ 165, 0x0000, 0x0000, 0x0000 }
};
chan_poparam_t	poparam_5gchantbl_core2[MAX_5G_CHANS_PHY5GSB_HIGHPWR_5B] = {
	/* FCC BAND1 */
	{ 34, 0x0000, 0x0000, 0x0000 },
	{ 36, 0x0000, 0x0000, 0x0000 },
	{ 38, 0x0000, 0x0000, 0x0000 },
	{ 40, 0x0000, 0x0000, 0x0000 },
	{ 42, 0x0000, 0x0000, 0x0000 },
	{ 44, 0x0000, 0x0000, 0x0000 },
	{ 46, 0x0000, 0x0000, 0x0000 },
	{ 48, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND2 */
	{ 52, 0x0000, 0x0000, 0x0000 },
	{ 54, 0x0000, 0x0000, 0x0000 },
	{ 56, 0x0000, 0x0000, 0x0000 },
	{ 60, 0x0000, 0x0000, 0x0000 },
	{ 62, 0x0000, 0x0000, 0x0000 },
	{ 64, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND3 */
	{ 100, 0x0000, 0x0000, 0x0000 },
	{ 102, 0x0000, 0x0000, 0x0000 },
	{ 104, 0x0000, 0x0000, 0x0000 },
	{ 108, 0x0000, 0x0000, 0x0000 },
	{ 110, 0x0000, 0x0000, 0x0000 },
	{ 112, 0x0000, 0x0000, 0x0000 },
	{ 116, 0x0000, 0x0000, 0x0000 },
	{ 118, 0x0000, 0x0000, 0x0000 },
	{ 120, 0x0000, 0x0000, 0x0000 },
	{ 124, 0x0000, 0x0000, 0x0000 },
	{ 126, 0x0000, 0x0000, 0x0000 },
	{ 128, 0x0000, 0x0000, 0x0000 },
	{ 132, 0x0000, 0x0000, 0x0000 },
	{ 134, 0x0000, 0x0000, 0x0000 },
	{ 136, 0x0000, 0x0000, 0x0000 },
	{ 140, 0x0000, 0x0000, 0x0000 },
	/* FCC BAND4 */
	{ 149, 0x0000, 0x0000, 0x0000 },
	{ 151, 0x0000, 0x0000, 0x0000 },
	{ 153, 0x0000, 0x0000, 0x0000 },
	{ 157, 0x0000, 0x0000, 0x0000 },
	{ 159, 0x0000, 0x0000, 0x0000 },
	{ 161, 0x0000, 0x0000, 0x0000 },
	{ 165, 0x0000, 0x0000, 0x0000 }
};
#endif	/* WL_PPR_SUBBAND */

#ifdef BCMECICOEX	    /* BTC notifications */
#define NOTIFY_BT_CHL(sih, val) \
	si_eci_notify_bt((sih), ECI_OUT_CHANNEL_MASK(sih->ccrev), \
			 ((val) << ECI_OUT_CHANNEL_SHIFT(sih->ccrev)), TRUE)
#define NOTIFY_BT_BW_20(sih) \
	si_eci_notify_bt((sih), ECI_OUT_BW_MASK(sih->ccrev), \
			 ((ECI_BW_25) << ECI_OUT_BW_SHIFT(sih->ccrev)), TRUE)
#define NOTIFY_BT_BW_40(sih) \
	si_eci_notify_bt((sih), ECI_OUT_BW_MASK(sih->ccrev), \
			 ((ECI_BW_45) << ECI_OUT_BW_SHIFT(sih->ccrev)), TRUE)
#define NOTIFY_BT_NUM_ANTENNA(sih, val) \
	si_eci_notify_bt((sih), ECI_OUT_ANTENNA_MASK(sih->ccrev), \
			 ((val) << ECI_OUT_ANTENNA_SHIFT(sih->ccrev)), TRUE)
#define NOTIFY_BT_TXRX(sih, val) \
	si_eci_notify_bt((sih), ECI_OUT_SIMUL_TXRX_MASK(sih->ccrev), \
			 ((val) << ECI_OUT_SIMUL_TXRX_SHIFT(sih->ccrev), TRUE))
static void wlc_phy_update_bt_chanspec(phy_info_t *pi, chanspec_t chanspec);
#else
#define wlc_phy_update_bt_chanspec(pi, chanspec)
#endif /* BCMECICOEX */

#define PHY_WREG_LIMIT	24	/* number of consecutive phy register write before a readback */
#define PHY_WREG_LIMIT_VENDOR 1	/* num of consec phy reg write before a readback for vendor */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  local prototype						*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

void phy_reg_write_array(void* pi, const uint16* regp, int length);

/* %%%%%% major operations */
static void wlc_set_phy_uninitted(phy_info_t *pi);
static uint32 wlc_phy_get_radio_ver(phy_info_t *pi);
static void wlc_phy_timercb_phycal(void *arg);

/* %%%%%% Calibration, ACI, noise/rssi measurement */
static void wlc_phy_noise_calc(phy_info_t *pi, uint32 *cmplx_pwr, int8 *pwr_ant,
                               uint8 extra_gain_1dB);
static void wlc_phy_noise_calc_fine_resln(phy_info_t *pi, uint32 *cmplx_pwr, int16 *pwr_ant,
                                          uint8 extra_gain_1dB);
static void wlc_phy_noise_save(phy_info_t *pi, int8 *noise_dbm_ant, int8 *max_noise_dbm);
static uint8 wlc_phy_calc_extra_init_gain(phy_info_t *pi, uint8 extra_gain_3dB,
                                        rxgain_t rxgain[]);

static void wlc_phy_aci_upd(phy_info_t *pi);
#if defined(WLTEST) || defined(WL_PHYACIARGS)
static int  wlc_phy_aci_args(phy_info_t *pi, wl_aci_args_t *params, bool get, int len);
#endif 
static void wlc_phy_aci_enter(phy_info_t *pi);
static void wlc_phy_aci_exit(phy_info_t *pi);
static void wlc_phy_aci_update_ma(phy_info_t *pi);
void wlc_phy_aci_noise_reset_nphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc);
void wlc_phy_aci_noise_reset_htphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc);


static bool wlc_phy_interference(phy_info_t *pi, int wanted_mode, bool init);
static void wlc_phy_vco_cal(phy_info_t *pi);
static void wlc_phy_cal_perical_mphase_schedule(phy_info_t *pi, uint delay);
static void wlc_phy_noise_sample_request(wlc_phy_t *pih, uint8 reason, uint8 ch);
/* %%%%%% temperature-based fallback to 1-Tx */
static void wlc_phy_txcore_temp(phy_info_t *pi);


/* %%%%%% power control */
static void wlc_phy_txpower_reg_limit_calc(phy_info_t *pi, txppr_t *tp, chanspec_t);
static bool wlc_phy_cal_txpower_recalc_sw(phy_info_t *pi);

/* %%%%%% testing */
#if defined(BCMDBG) || defined(WLTEST)
static int wlc_phy_test_carrier_suppress(phy_info_t *pi, int channel);
static int wlc_phy_test_freq_accuracy(phy_info_t *pi, int channel);
static int wlc_phy_test_evm(phy_info_t *pi, int channel, uint rate, int txpwr);
#endif


static uint32 wlc_phy_rx_iq_est(phy_info_t *pi, uint8 samples, uint8 antsel, uint8 resolution,
	uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
	uint8 extra_gain_3dB, uint8 wait_for_crs);
#ifdef SAMPLE_COLLECT
static int wlc_phy_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *collect, void *buff);
static int wlc_phy_sample_data(phy_info_t *pi, wl_sampledata_t *sample_data, void *b);
static int wlc_phy_sample_collect_old(phy_info_t *pi, wl_samplecollect_args_t *collect, void *buff);
#endif

/* %%%%%% radar */
#if defined(AP) && defined(RADAR)
static void wlc_phy_radar_read_table(phy_info_t *pi, radar_work_t *rt, int min_pulses);
static void wlc_phy_radar_generate_tlist(uint32 *inlist, int *outlist, int length, int n);
static void wlc_phy_radar_generate_tpw(uint32 *inlist, int *outlist, int length, int n);
static void wlc_phy_radar_filter_list(int *inlist, int *length, int min_val, int max_val);
static void wlc_shell_sort(int len, int *vector);
static int  wlc_phy_radar_select_nfrequent(int *inlist, int length, int n, int *val,
	int *pos, int *f, int *vlist, int *flist);
static int  wlc_phy_radar_detect_run_aphy(phy_info_t *pi);
static void wlc_phy_radar_detect_init_aphy(phy_info_t *pi, bool on);
static int  wlc_phy_radar_detect_run_nphy(phy_info_t *pi);
static void wlc_phy_radar_detect_init_nphy(phy_info_t *pi, bool on);
static void wlc_phy_radar_detect_init_htphy(phy_info_t *pi, bool on);
static void wlc_phy_radar_attach_nphy(phy_info_t *pi);
#endif /* defined(AP) && defined(RADAR) */

static int wlc_phy_iovar_dispatch_old(phy_info_t *pi, uint32 actionid, void *p, void *a, int vsize,
	int32 int_val, bool bool_val);

static int8 wlc_user_txpwr_antport_to_rfport(phy_info_t *pi, uint chan, uint32 band, uint8 rate);
static void wlc_phy_upd_env_txpwr_rate_limits(phy_info_t *pi, uint32 band);
static int8 wlc_phy_env_measure_vbat(phy_info_t *pi);
static int8 wlc_phy_env_measure_temperature(phy_info_t *pi);
static void wlc_phy_btc_adjust(phy_info_t *pi, bool btactive, uint16 btperiod);
#if defined(WLMCHAN) && defined(BCMDBG)
static void wlc_phydump_chanctx(phy_info_t *phi, struct bcmstrbuf *b);
#endif /* WLMCHAN */
static void mat_print(int64 *a, int m, int n, const char *name);
static void mat_rho(int64 *n, int64 *p, int64 *rho, int m);
static void mat_transpose(int64 *a, int64 *b, int m, int n);
static void mat_mult(int64 *a, int64 *b, int64 *c, int m, int n, int r);
static void mat_inv_prod_det(int64 *a, int64 *b);
static void mat_det(int64 *a, int64 *det);
static void ratmodel_paparams_fix64(ratmodel_paparams_t* rsd, int m);
#if PHY_TSSI_CAL_DBG_EN
static void print_int64(int64 *a);
#endif
static uint16 tssi_cal_sweep(phy_info_t *pi);
int wlc_phy_tssi_cal(phy_info_t *pi);
#if defined(WLTEST)
static int wlc_phy_set_pa_phy(phy_info_t *pi, wl_pavars2_t *invar, uint *ptrj, uint16 *inpa);
static int wlc_phy_get_pa_phy(phy_info_t *pi, wl_pavars2_t *invar, uint *ptrj, uint16 *outpa);
#endif
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  function implementation   					*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  function implementation   					*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define MAX_FABID_CHARS	16
char *
#ifdef BCMDBG
phy_getvar(phy_info_t *pi, const char *name, const char *function)
#else
phy_getvar(phy_info_t *pi, const char *name)
#endif
{
#ifdef _MINOSL_
	return NULL;
#else
	char *vars = pi->vars;
	char *s;
	int len;

#ifdef BCMDBG
	/* Use vars pointing to itself as a flag that wlc_phy_attach is complete.
	 * Can't use NULL because it means there are no vars but we still
	 * need to potentially search NVRAM.
	 */

	if (pi->vars == (char *)&pi->vars)
		PHY_ERROR(("Usage of phy_getvar/phy_getintvar by %s after wlc_phy_attach\n",
			function));
#endif /* BCMDBG */
	ASSERT(pi->vars != (char *)&pi->vars);

	if (!name)
		return NULL;

	len = strlen(name);
	if (len == 0)
		return NULL;

	/* first look in vars[] */
	for (s = vars; s && *s;) {
		if ((bcmp(s, name, len) == 0) && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}

#ifdef LINUX_HYBRID
	/* Don't look elsewhere! */
	return NULL;
#else
	/* Query nvram */
	return (nvram_get(name));

#endif

#endif	/* _MINOSL_ */
}

char *
#ifdef BCMDBG
phy_getvar_fabid(phy_info_t *pi, const char *name, const char *function)
#else
phy_getvar_fabid(phy_info_t *pi, const char *name)
#endif
{
	char *val = NULL;

	if (pi->fabid) {
		uint16 sz = (strlen(name) + strlen(".fab.") + MAX_FABID_CHARS);
		char *fab_name = (char *) MALLOC(pi->sh->osh, sz);
		/* Prepare fab name */
		if (fab_name == NULL) {
			PHY_ERROR(("wl%d: %s: MALLOC failure\n",
				pi->sh->unit, __FUNCTION__));
			return FALSE;
		}
		snprintf(fab_name, sz, "%s.fab.%d", name, pi->fabid);

#ifdef BCMDBG
		val = phy_getvar(pi, (const char *)fab_name, function);
#else
		val = phy_getvar(pi, (const char *)fab_name);
#endif /* BCMDBG */
		MFREE(pi->sh->osh, fab_name, sz);
	}

	if (val == NULL) {
#ifdef BCMDBG
		val = phy_getvar(pi, (const char *)name, function);
#else
		val = phy_getvar(pi, (const char *)name);
#endif /* BCMDBG */
	}

	return val;
}


int
#ifdef BCMDBG
phy_getintvar(phy_info_t *pi, const char *name, const char *function)
#else
phy_getintvar(phy_info_t *pi, const char *name)
#endif
{
	return phy_getintvar_default(pi, name, 0);
}

int
phy_getintvar_default(phy_info_t *pi, const char *name, int default_value)
{
#ifdef _MINOSL_
	return 0;
#else
	char *val = PHY_GETVAR(pi, name);
	if (val != NULL)
		return (bcm_strtoul(val, NULL, 0));

	return (default_value);
#endif	/* _MINOSL_ */
}

int
#ifdef BCMDBG
phy_getintvararray(phy_info_t *pi, const char *name, int idx, const char *function)
#else
phy_getintvararray(phy_info_t *pi, const char *name, int idx)
#endif
{
	return PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, 0);
}

int
#ifdef BCMDBG
phy_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value,
	const char *function)
#else
phy_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value)
#endif
{
	int i, val;
	char *vars = pi->vars;
	char *fabspf = NULL;

	val = default_value;
	if (PHY_GETVAR(pi, name)) {
		/* Check if fab specific values are present in the NVRAM
		*  if present, replace the regular value with the fab specific
		*/
		if (pi->fabid) {
			uint16 sz = (strlen(name) + strlen(".fab.") + MAX_FABID_CHARS);
			char *fab_name = (char *) MALLOC(pi->sh->osh, sz);
			/* Prepare fab name */
			if (fab_name == NULL) {
				PHY_ERROR(("wl%d: %s: MALLOC failure\n",
					pi->sh->unit, __FUNCTION__));
				return FALSE;
			}
			snprintf(fab_name, sz, "%s.fab.%d", name, pi->fabid);
			/* Get the value from fabid specific first if present
			*  Assumption: param.fab.<fabid>.fab.<fabid> never exist
			*/
			fabspf = PHY_GETVAR(pi, (const char *)fab_name);
			if (fabspf != NULL) {
				i = getintvararraysize(vars, fab_name);
				if ((i == 0) || (i <= idx))
					val = default_value;
				else
					val = getintvararray(vars, fab_name, idx);
			}
			MFREE(pi->sh->osh, fab_name, sz);
		}

		if (fabspf == NULL) {
			i = getintvararraysize(vars, name);
			if ((i == 0) || (i <= idx))
				val = default_value;
			else
				val = getintvararray(vars, name, idx);
		}
	}
	return val;
}

/* coordinate with MAC before access PHY register */
void
wlc_phyreg_enter(wlc_phy_t *pih)
{
#ifdef STA
	phy_info_t *pi = (phy_info_t*)pih;
	wlapi_bmac_ucode_wake_override_phyreg_set(pi->sh->physhim);
#endif
}

void
wlc_phyreg_exit(wlc_phy_t *pih)
{
#ifdef STA
	phy_info_t *pi = (phy_info_t*)pih;
	wlapi_bmac_ucode_wake_override_phyreg_clear(pi->sh->physhim);
#endif
}

/* coordinate with MAC before access RADIO register */
void
wlc_radioreg_enter(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_LOCK_RADIO, MCTL_LOCK_RADIO);

	/* allow any ucode radio reg access to complete */
	OSL_DELAY(10);
}

void
wlc_radioreg_exit(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	volatile uint16 dummy;

	/* allow our radio reg access to complete */
	dummy = R_REG(pi->sh->osh, &pi->regs->phyversion);
	pi->phy_wreg = 0;
	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_LOCK_RADIO, 0);
}

/* All radio regs other than idcode are less than 16bits, so
 * {read, write}_radio_reg access the low 16bits only.
 * When reading the idcode use read_radio_id instead.
 */
uint16
read_radio_reg(phy_info_t *pi, uint16 addr)
{
	uint16 data;

	if ((addr == RADIO_IDCODE) && (!ISHTPHY(pi)))
		return 0xffff;

	if (NORADIO_ENAB(pi->pubpi))
		return (NORADIO_IDCODE & 0xffff);

	switch (pi->pubpi.phy_type) {
	case PHY_TYPE_A:
		CASECHECK(PHYTYPE, PHY_TYPE_A);
		addr |= RADIO_2060WW_READ_OFF;
		break;

	case PHY_TYPE_G:
		CASECHECK(PHYTYPE, PHY_TYPE_G);
		addr |= RADIO_2050_READ_OFF;
		break;

	case PHY_TYPE_N:
		CASECHECK(PHYTYPE, PHY_TYPE_N);
		if (NREV_GE(pi->pubpi.phy_rev, 7)) {
			if (NREV_IS(pi->pubpi.phy_rev, 19))
				addr |= RADIO_20671_READ_OFF;
			else
				addr |= RADIO_2057_READ_OFF;
		} else
			addr |= RADIO_2055_READ_OFF;  /* works for 2056 too */
		break;

	case PHY_TYPE_HT:
		CASECHECK(PHYTYPE, PHY_TYPE_HT);
		addr |= RADIO_2059_READ_OFF;
		break;

	case PHY_TYPE_LP:
		CASECHECK(PHYTYPE, PHY_TYPE_LP);
		if (BCM2063_ID == LPPHY_RADIO_ID(pi)) {
			addr |= RADIO_2063_READ_OFF;
		} else {
			addr |= RADIO_2062_READ_OFF;
		}
		break;

	case PHY_TYPE_SSN:
		CASECHECK(PHYTYPE, PHY_TYPE_SSN);
		addr |= RADIO_2063_READ_OFF;
		break;
	case PHY_TYPE_LCN:
		CASECHECK(PHYTYPE, PHY_TYPE_LCN);
		addr |= RADIO_2064_READ_OFF;
		break;
	case PHY_TYPE_LCN40:
		CASECHECK(PHYTYPE, PHY_TYPE_LCN40);
		addr |= RADIO_2065_READ_OFF;
		break;
	default:
		ASSERT(VALID_PHYTYPE(pi->pubpi.phy_type));
	}

	if ((D11REV_GE(pi->sh->corerev, 24) && !D11REV_IS(pi->sh->corerev, 27)) ||
	    (D11REV_IS(pi->sh->corerev, 22) && (pi->pubpi.phy_type != PHY_TYPE_SSN))) {

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, addr);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif
		data = R_REG(pi->sh->osh, &pi->regs->radioregdata);
	} else {
		W_REG(pi->sh->osh, &pi->regs->phy4waddr, addr);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->phy4waddr);
#endif

#ifdef __ARM_ARCH_4T__
		__asm__(" .align 4 ");
		__asm__(" nop ");
		data = R_REG(pi->sh->osh, &pi->regs->phy4wdatalo);
#else
		data = R_REG(pi->sh->osh, &pi->regs->phy4wdatalo);
#endif

	}
	pi->phy_wreg = 0;	/* clear reg write metering */

	return data;
}

void
write_radio_reg(phy_info_t *pi, uint16 addr, uint16 val)
{
	osl_t *osh;
	volatile uint16 dummy;


	if (NORADIO_ENAB(pi->pubpi))
		return;

	osh = pi->sh->osh;

	/* The nphy chips with with corerev 22 have the new i/f, the one with
	 * ssnphy (4319b0) does not.
	 */
	if (BUSTYPE(pi->sh->bustype) == PCI_BUS) {
		if (++pi->phy_wreg >= pi->phy_wreg_limit) {
			(void)R_REG(osh, &pi->regs->maccontrol);
			pi->phy_wreg = 0;
		}
	}
	if ((D11REV_GE(pi->sh->corerev, 24) && !D11REV_IS(pi->sh->corerev, 27)) ||
	    (D11REV_IS(pi->sh->corerev, 22) && (pi->pubpi.phy_type != PHY_TYPE_SSN))) {

		W_REG(osh, &pi->regs->radioregaddr, addr);
#ifdef __mips__
		(void)R_REG(osh, &pi->regs->radioregaddr);
#endif
		W_REG(osh, &pi->regs->radioregdata, val);
	} else {
		W_REG(osh, &pi->regs->phy4waddr, addr);
#ifdef __mips__
		(void)R_REG(osh, &pi->regs->phy4waddr);
#endif
		W_REG(osh, &pi->regs->phy4wdatalo, val);
	}

	if ((BUSTYPE(pi->sh->bustype) == PCMCIA_BUS) &&
	    (pi->sh->buscorerev <= 3)) {
		dummy = R_REG(osh, &pi->regs->phyversion);
	}
}

static uint32
read_radio_id(phy_info_t *pi)
{
	uint32 id;

	if (NORADIO_ENAB(pi->pubpi))
		return (NORADIO_IDCODE);

	if (ISLCN40PHY(pi) || (ISNPHY(pi) && NREV_IS(pi->pubpi.phy_rev, 19))) {
		uint32 rnum;

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 0);
		rnum = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
		rnum = (rnum >> 4) & 0xF;

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 1);
		id = (R_REG(pi->sh->osh, &pi->regs->radioregdata) << IDCODE_ID_SHIFT) | rnum;
	} else if (D11REV_GE(pi->sh->corerev, 24) && !D11REV_IS(pi->sh->corerev, 27)) {
		uint32 b0, b1, b2;

		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 0);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif
		b0 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 1);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif
		b1 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);
		W_REG(pi->sh->osh, &pi->regs->radioregaddr, 2);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->radioregaddr);
#endif
		b2 = (uint32)R_REG(pi->sh->osh, &pi->regs->radioregdata);

		id = ((b0  & 0xf) << 28) | (((b2 << 8) | b1) << 12) | ((b0 >> 4) & 0xf);
	} else {
		W_REG(pi->sh->osh, &pi->regs->phy4waddr, RADIO_IDCODE);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, &pi->regs->phy4waddr);
#endif
		id = (uint32)R_REG(pi->sh->osh, &pi->regs->phy4wdatalo);
		id |= (uint32)R_REG(pi->sh->osh, &pi->regs->phy4wdatahi) << 16;
	}
	pi->phy_wreg = 0;	/* clear reg write metering */
	return id;
}

void
and_radio_reg(phy_info_t *pi, uint16 addr, uint16 val)
{
	uint16 rval;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	rval = read_radio_reg(pi, addr);
	write_radio_reg(pi, addr, (rval & val));
}

void
or_radio_reg(phy_info_t *pi, uint16 addr, uint16 val)
{
	uint16 rval;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	rval = read_radio_reg(pi, addr);
	write_radio_reg(pi, addr, (rval | val));
}

void
xor_radio_reg(phy_info_t *pi, uint16 addr, uint16 mask)
{
	uint16 rval;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	rval = read_radio_reg(pi, addr);
	write_radio_reg(pi, addr, (rval ^ mask));
}

void
mod_radio_reg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val)
{
	uint16 rval;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	rval = read_radio_reg(pi, addr);
	write_radio_reg(pi, addr, (rval & ~mask) | (val & mask));
}

void
gen_radio_reg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data)
{
	if (NORADIO_ENAB(pi->pubpi))
		return;

	*orig_reg_addr = addr;
	*orig_reg_data = read_radio_reg(pi, addr);

	*updated_reg_addr = addr;
	*updated_reg_data = ((*orig_reg_data) & ~mask) | (val & mask);
}

void
write_phy_channel_reg(phy_info_t *pi, uint val)
{
	volatile uint16 dummy;

	W_REG(pi->sh->osh, &pi->regs->phychannel, val);

	if ((BUSTYPE(pi->sh->bustype) == PCMCIA_BUS) &&
	    (pi->sh->buscorerev <= 3)) {
		dummy = R_REG(pi->sh->osh, &pi->regs->phyversion);
	}
}

#if defined(BCMDBG) || defined(BCMASSERT_SUPPORT)
static bool
wlc_phy_war41476(phy_info_t *pi)
{
	uint32 mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);

	return ((mc & MCTL_EN_MAC) == 0) || ((mc & MCTL_PHYLOCK) == MCTL_PHYLOCK);
}
#endif /* BCMDBG || BCMASSERT_SUPPORT */

uint16
phy_reg_read(phy_info_t *pi, uint16 addr)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = pi->sh->osh;
	regs = pi->regs;

	W_REG(osh, &regs->phyregaddr, addr);
#ifdef __mips__
	(void)R_REG(osh, &regs->phyregaddr);
#endif

	ASSERT(!(D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) ||
	       wlc_phy_war41476(pi));

	pi->phy_wreg = 0;	/* clear reg write metering */
	return (R_REG(osh, &regs->phyregdata));
}

void phy_reg_write_array(void* pi, const uint16* regp, int length)
{
	ASSERT(regp != NULL && length > 0);
	while (length-- > 0)
	{
		uint16 addr = *regp & ~PHY_REG_MASK_TYPE;
		switch (*regp++ & PHY_REG_MASK_TYPE)
		{
			case PHY_REG_MOD_TYPE:
				phy_reg_mod(pi, addr, *regp, *(regp+1));
				++regp;
				--length;
				break;

			case PHY_REG_WRITE_TYPE:
				phy_reg_write(pi, addr, *regp);
				break;

			case PHY_REG_AND_TYPE:
				phy_reg_and(pi, addr, *regp);
				break;

			case PHY_REG_OR_TYPE:
				phy_reg_or(pi, addr, *regp);
				break;

			default: ASSERT(0);
		}
		++regp;
		--length;
	}
}

void
phy_reg_write(phy_info_t *pi, uint16 addr, uint16 val)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = pi->sh->osh;
	regs = pi->regs;

#ifdef __mips__
	W_REG(osh, &regs->phyregaddr, addr);
	(void)R_REG(osh, &regs->phyregaddr);
	W_REG(osh, &regs->phyregdata, val);
	if (addr == NPHY_TableAddress)
		(void)R_REG(osh, &regs->phyregdata);
#else
	if (BUSTYPE(pi->sh->bustype) == PCI_BUS) {
		if (++pi->phy_wreg >= pi->phy_wreg_limit) {
			pi->phy_wreg = 0;
			(void)R_REG(osh, &regs->phyversion);
		}
	}
	W_REG(osh, (volatile uint32 *)(uintptr)(&regs->phyregaddr), addr | (val << 16));
#endif /* __mips__ */
}

void
phy_reg_and(phy_info_t *pi, uint16 addr, uint16 val)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = pi->sh->osh;
	regs = pi->regs;

	W_REG(osh, &regs->phyregaddr, addr);
#ifdef __mips__
	(void)R_REG(osh, &regs->phyregaddr);
#endif

	ASSERT(!(D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) ||
	       wlc_phy_war41476(pi));

	W_REG(osh, &regs->phyregdata, (R_REG(osh, &regs->phyregdata) & val));
	pi->phy_wreg = 0;	/* clear reg write metering */
}

void
phy_reg_or(phy_info_t *pi, uint16 addr, uint16 val)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = pi->sh->osh;
	regs = pi->regs;

	W_REG(osh, &regs->phyregaddr, addr);
#ifdef __mips__
	(void)R_REG(osh, &regs->phyregaddr);
#endif

	ASSERT(!(D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) ||
	       wlc_phy_war41476(pi));

	W_REG(osh, &regs->phyregdata, (R_REG(osh, &regs->phyregdata) | val));
	pi->phy_wreg = 0;	/* clear reg write metering */
}

void
phy_reg_mod(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val)
{
	osl_t *osh;
	d11regs_t *regs;

	osh = pi->sh->osh;
	regs = pi->regs;

	W_REG(osh, &regs->phyregaddr, addr);
#ifdef __mips__
	(void)R_REG(osh, &regs->phyregaddr);
#endif

	ASSERT(!(D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) ||
	       wlc_phy_war41476(pi));

	W_REG(osh, &regs->phyregdata, ((R_REG(osh, &regs->phyregdata) & ~mask) | (val & mask)));
	pi->phy_wreg = 0;	/* clear reg write metering */
}


void
phy_reg_gen(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data)
{
	*orig_reg_addr = addr;
	*orig_reg_data = phy_reg_read(pi, addr);

	*updated_reg_addr = addr;
	*updated_reg_data = (((*orig_reg_data) & ~mask) | (val & mask));
}

static void
WLBANDINITFN(wlc_set_phy_uninitted)(phy_info_t *pi)
{
	int i = 0;
	/* Prepare for one-time initializations */
	pi->initialized = FALSE;
	pi->txpwridx = -1;
	pi->phy_spuravoid = SPURAVOID_AUTO;
	if (ISABGPHY(pi))
		wlc_set_uninitted_abgphy(pi);

	if (ISNPHY(pi)) {
		wlc_set_uninitted_nphy(pi);
	} else if (ISHTPHY(pi)) {
		for (i = 0; i < PHY_CORE_MAX; i++)
			pi->u.pi_htphy->txpwrindex[i] = 40;
	}
}

/* returns a pointer to per interface instance data */
shared_phy_t *
BCMATTACHFN(wlc_phy_shared_attach)(shared_phy_params_t *shp)
{
	shared_phy_t *sh;

	/* allocate wlc_info_t state structure */
	if ((sh = (shared_phy_t*) MALLOC(shp->osh, sizeof(shared_phy_t))) == NULL) {
		PHY_ERROR(("wl%d: wlc_phy_shared_state: out of memory, malloced %d bytes\n",
			shp->unit, MALLOCED(shp->osh)));
		return NULL;
	}
	bzero((char*)sh, sizeof(shared_phy_t));

	sh->osh = shp->osh;
	sh->sih = shp->sih;
	sh->physhim = shp->physhim;
	sh->unit = shp->unit;
	sh->corerev = shp->corerev;

	sh->vid = shp->vid;
	sh->did = shp->did;
	sh->chip = shp->chip;
	sh->chiprev = shp->chiprev;
	sh->chippkg = shp->chippkg;
	sh->sromrev = shp->sromrev;
	sh->boardtype = shp->boardtype;
	sh->boardrev = shp->boardrev;
	sh->boardvendor = shp->boardvendor;
	sh->boardflags = shp->boardflags;
	sh->boardflags2 = shp->boardflags2;
	sh->bustype = shp->bustype;
	sh->buscorerev = shp->buscorerev;

	/* create our timers */
	sh->fast_timer = PHY_SW_TIMER_FAST;
	sh->slow_timer = PHY_SW_TIMER_SLOW;
	sh->glacial_timer = PHY_SW_TIMER_GLACIAL;

	/* ACI mitigation mode is auto by default */
	sh->interference_mode = WLAN_AUTO;
	sh->rssi_mode = RSSI_ANT_MERGE_MAX;

#ifdef WLMEDIA_TXFILTER_OVERRIDE
	sh->txfilter_sm_override = WLC_TXFILTER_OVERRIDE_DISABLED;
#endif /* WLMEDIA_TXFILTER_OVERRIDE */

	return sh;
}

void
BCMATTACHFN(wlc_phy_shared_detach)(shared_phy_t *phy_sh)
{
	osl_t *osh;

	if (phy_sh) {
		osh = phy_sh->osh;

		/* phy_head must have been all detached */
		if (phy_sh->phy_head) {
			PHY_ERROR(("wl%d: %s non NULL phy_head\n", phy_sh->unit, __FUNCTION__));
			ASSERT(!phy_sh->phy_head);
		}
		MFREE(osh, phy_sh, sizeof(shared_phy_t));
	}
}

static int
BCMATTACHFN(wlc_phy_radio_attach)(phy_info_t *pi)
{
	int bcmerror = BCME_OK;
	/* read the radio idcode */
	if (ISSIM_ENAB(pi->sh->sih) && !ISHTPHY(pi)) {
		PHY_INFORM(("wl%d: Assuming NORADIO, chip 0x%x pkgopt 0x%x\n", pi->sh->unit,
		           pi->sh->chip, pi->sh->chippkg));
		pi->pubpi.radioid = NORADIO_ID;
		pi->pubpi.radiorev = 5;
	} else {
		uint32 idcode;
		if (!ISLCN40PHY (pi))
			wlc_phy_anacore((wlc_phy_t*)pi, ON);

		idcode = wlc_phy_get_radio_ver(pi);
		pi->pubpi.radioid = (idcode & IDCODE_ID_MASK) >> IDCODE_ID_SHIFT;
		pi->pubpi.radiorev = (idcode & IDCODE_REV_MASK) >> IDCODE_REV_SHIFT;
		pi->pubpi.radiover = (idcode & IDCODE_VER_MASK) >> IDCODE_VER_SHIFT;


		if (!VALID_RADIO(pi, pi->pubpi.radioid)) {
			PHY_ERROR(("wl%d: %s: Unknown radio ID: 0x%x rev 0x%x phy %d, phyrev %d\n",
			          pi->sh->unit, __FUNCTION__, pi->pubpi.radioid, pi->pubpi.radiorev,
			          pi->pubpi.phy_type, pi->pubpi.phy_rev));
			bcmerror = BCME_ERROR;
		}
		/* make sure the radio is off until we do an "up" */
		wlc_phy_switch_radio((wlc_phy_t*)pi, OFF);
	}
	return bcmerror;
}

static const char BCMATTACHDATA(rstr_interference)[] = "interference";
static const char BCMATTACHDATA(rstr_txpwrbckof)[] = "txpwrbckof";
static const char BCMATTACHDATA(rstr_tssilimucod)[] = "tssilimucod";
static const char BCMATTACHDATA(rstr_rssicorrnorm)[] = "rssicorrnorm";
static const char BCMATTACHDATA(rstr_rssicorratten)[] = "rssicorratten";

/* Figure out if we have a phy for the requested band and attach to it */
wlc_phy_t *
BCMATTACHFN(wlc_phy_attach)(shared_phy_t *sh, void *regs, int bandtype, char *vars)
{
	phy_info_t *pi;
	uint32 sflags = 0;
	uint phyversion;
	int i;
	osl_t *osh;

	osh = sh->osh;

	PHY_TRACE(("wl: %s(%p, %d, %p)\n", __FUNCTION__, regs, bandtype, sh));

	if (D11REV_IS(sh->corerev, 4))
		sflags = SISF_2G_PHY | SISF_5G_PHY;
	else
		sflags = si_core_sflags(sh->sih, 0, 0);

	if (BAND_5G(bandtype)) {
		if ((sflags & (SISF_5G_PHY | SISF_DB_PHY)) == 0) {
			PHY_ERROR(("wl%d: %s: No phy available for 5G\n",
			          sh->unit, __FUNCTION__));
			return NULL;
		}
	}

	if ((sflags & SISF_DB_PHY) && (pi = sh->phy_head)) {
		/* For the second band in dualband phys, load the band specific
		  * NVRAM parameters
		  * The second condition excludes UNO3 inorder to
		  * keep the device id as 0x4360 (dual band).
		  * Purely to be backward compatible to previous UNO3 NVRAM file.
		  *
		*/
		if (ISLCNPHY(pi) && !((pi->sh->boardtype == 0x0551) &&
		(CHIPID(pi->sh->chip) == BCM4330_CHIP_ID))) {
			if (!wlc_phy_txpwr_srom_read_lcnphy(pi, bandtype))
				goto err;
		}

		/* For the second band in dualband phys, just bring the core back out of reset */
		wlapi_bmac_corereset(pi->sh->physhim, pi->pubpi.coreflags);
		pi->refcnt++;
		return &pi->pubpi_ro;
	}

	/* ONLY common PI is allocated. pi->u.pi_xphy is not available yet */
	if ((pi = (phy_info_t *)MALLOC(osh, sizeof(phy_info_t))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes", sh->unit,
		          __FUNCTION__, MALLOCED(osh)));
		return NULL;
	}
	bzero((char *)pi, sizeof(phy_info_t));

	/* Assign the default cal info */
	pi->cal_info = &pi->def_cal_info;

#ifdef WLNOKIA_NVMEM
	pi->noknvmem = wlc_phy_noknvmem_attach(osh, pi);
	if (pi->noknvmem == NULL) {
		PHY_ERROR(("wl%d: %s: wlc_phy_noknvmem_attach failed ", sh->unit, __FUNCTION__));
		goto err;
	}
#endif /* WLNOKIA_NVMEM */
	pi->regs = (d11regs_t *)regs;
	pi->sh = sh;
	pi->phy_init_por = TRUE;
	if ((pi->sh->boardvendor == VENDOR_APPLE) &&
	    (pi->sh->boardtype == 0x0093)) {
		pi->phy_wreg_limit = PHY_WREG_LIMIT_VENDOR;
	}
	else {
		pi->phy_wreg_limit = PHY_WREG_LIMIT;
	}

	pi->vars = vars;

	/* set default power output percentage to 100 percent */
	pi->txpwr_percent = 100;


	/* this will get the value from the SROM */
	pi->phycal_tempdelta = 0;
	pi->papdcal_indexdelta = 4;

	if (BAND_2G(bandtype) && (sflags & SISF_2G_PHY)) {
		/* Set the sflags gmode indicator */
		pi->pubpi.coreflags = SICF_GMODE;
	}

	/* get the phy type & revision, enable the core */
	wlapi_bmac_corereset(pi->sh->physhim, pi->pubpi.coreflags);
	phyversion = R_REG(osh, &pi->regs->phyversion);

	/* Read the fabid */
	pi->fabid = si_fabid(GENERIC_PHY_INFO(pi)->sih);

	pi->pubpi.phy_type = PHY_TYPE(phyversion);
	pi->pubpi.phy_rev = phyversion & PV_PV_MASK;

	if (((pi->sh->chip == BCM43235_CHIP_ID) ||
		(pi->sh->chip == BCM43236_CHIP_ID) ||
		(pi->sh->chip == BCM43238_CHIP_ID) ||
		(pi->sh->chip == BCM43234_CHIP_ID)) && ((pi->sh->chiprev == 2) ||
		(pi->sh->chiprev == 3))) {
		pi->pubpi.phy_rev = 9;
	}

	/* LCNXN */
	if (pi->pubpi.phy_type == PHY_TYPE_LCNXN) {
		pi->pubpi.phy_type = PHY_TYPE_N;
		pi->pubpi.phy_rev += LCNXN_BASEREV;
	}
	pi->pubpi.phy_corenum = ISHTPHY(pi) ? PHY_CORE_NUM_3 :
		(ISNPHY(pi) ? PHY_CORE_NUM_2 : PHY_CORE_NUM_1);
	pi->pubpi.ana_rev = (phyversion & PV_AV_MASK) >> PV_AV_SHIFT;

	if (!VALID_PHYTYPE(pi->pubpi.phy_type)) {
		PHY_ERROR(("wl%d: %s: invalid phy_type = %d\n",
		          sh->unit, __FUNCTION__, pi->pubpi.phy_type));
		goto err;
	}
	if (BAND_5G(bandtype)) {
		if (!ISAPHY(pi) && !ISNPHY(pi) && !ISLPPHY(pi) && !ISSSLPNPHY(pi) &&
			!ISLCN40PHY(pi) && !ISHTPHY(pi) && !ISLCNPHY(pi)) {
			PHY_ERROR(("wl%d: %s: invalid phy_type = %d for band 5G\n",
			          sh->unit, __FUNCTION__, pi->pubpi.phy_type));
			goto err;
		}
	} else {
		if (!ISGPHY(pi) && !ISNPHY(pi) && !ISLPPHY(pi) && !ISSSLPNPHY(pi) &&
			!ISLCNCOMMONPHY(pi) && !ISHTPHY(pi))
		{
			PHY_ERROR(("wl%d: %s: invalid phy_type = %d for band 2G\n",
			          sh->unit, __FUNCTION__, pi->pubpi.phy_type));
			goto err;
		}
	}

	pi->aci_exit_check_period = (ISNPHY(pi) || ISHTPHY(pi)) ? 15 : 60;
	pi->aci_state = 0;

	/* default channel and channel bandwidth is 20 MHZ */
	pi->bw = WL_CHANSPEC_BW_20;
	pi->radio_chanspec = BAND_2G(bandtype) ? CH20MHZ_CHSPEC(1) : CH20MHZ_CHSPEC(36);
	pi->interf.curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (ISNPHY(pi) || ISHTPHY(pi)) {
		pi->sh->interference_mode_override = FALSE;
		if (NREV_GE(pi->pubpi.phy_rev, 3) || ISHTPHY(pi)) {
			if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) &&
				(ISHTPHY(pi))) {
				pi->sh->interference_mode_2G = INTERFERE_NONE;
			} else {
				pi->sh->interference_mode_2G = WLAN_AUTO;
			}
			pi->sh->interference_mode_5G = NON_WLAN;

			/* assign 2G default interference mode for 5357, 5358, ... */
			/*
			if ((pi->pubpi.radiorev == 5) && (pi->pubpi.radiover > 0)) {
				/ 5357B0 *
				pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
			}
			*/
			if (ISHTPHY(pi) && (IS_X12_BOARDTYPE(pi) || IS_X29_BOARDTYPE(pi))) {
				pi->sh->interference_mode_2G = INTERFERE_NONE;
				pi->sh->interference_mode_5G = INTERFERE_NONE;
			}

			if (pi->pubpi.phy_rev == LCNXN_BASEREV)
			{
				pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
				pi->sh->interference_mode_5G = NON_WLAN;
			}

			if (pi->pubpi.phy_rev == LCNXN_BASEREV + 2) {
				pi->sh->interference_mode_2G = INTERFERE_NONE;
				pi->sh->interference_mode_5G = INTERFERE_NONE;
#ifdef NOISE_CAL_LCNXNPHY
				pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
				pi->sh->interference_mode_5G = NON_WLAN;
#endif
			}

			if (((CHIPID(pi->sh->chip) == BCM4716_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM4748_CHIP_ID)) &&
				(pi->sh->chippkg == BCM4718_PKG_ID)) {
				pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
			}
			/* assign 2G default interference mode for 4323x chips */
			if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43235_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43238_CHIP_ID) ||
				(CHIPID(pi->sh->chip) == BCM43237_CHIP_ID)) {
				pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
			}
			/* Disable interference mode for 43237 chips, nintendo project */
			if (CHIPID(pi->sh->chip) == BCM43237_CHIP_ID) {
				pi->sh->interference_mode_5G = INTERFERE_NONE;
			}

			if (BAND_2G(bandtype)) {
				pi->sh->interference_mode =
					pi->sh->interference_mode_2G;
			} else {
				pi->sh->interference_mode =
					pi->sh->interference_mode_5G;
			}
			pi->interf.scanroamtimer = 0;
			pi->interf.rssi_index = 0;
			pi->rssi1_index = 0;
			pi->rssi0_index = 0;
			pi->interf.rssi = 0;

		} else {
			/* 4321 */
			pi->sh->interference_mode = WLAN_AUTO;
		}
	} else if (ISSSLPNPHY(pi) && SSLPNREV_IS(pi->pubpi.phy_rev, 3)) {
		pi->sh->interference_mode_2G = WLAN_AUTO;
		pi->sh->interference_mode_5G = WLAN_AUTO;
		pi->sh->interference_mode = WLAN_AUTO;
		if (BAND_2G(bandtype)) {
			pi->sh->interference_mode =
				pi->sh->interference_mode_2G;
		} else {
			pi->sh->interference_mode =
				pi->sh->interference_mode_5G;
		}
	}

	/* Re-init the interference value based on the nvram variables */
	if (PHY_GETVAR(pi, rstr_interference) != NULL) {
		pi->sh->interference_mode_2G = (int)PHY_GETINTVAR(pi, rstr_interference);
		pi->sh->interference_mode_5G = (int)PHY_GETINTVAR(pi, rstr_interference);

		if (BAND_2G(bandtype)) {
			pi->sh->interference_mode =
				pi->sh->interference_mode_2G;
		} else {
			pi->sh->interference_mode =
				pi->sh->interference_mode_5G;
		}
	}

	/* set default rx iq est antenna/samples */
	pi->phy_rxiq_samps = PHY_NOISE_SAMPLE_LOG_NUM_NPHY;
	pi->phy_rxiq_antsel = ANT_RX_DIV_DEF;

	/* initialize SROM "isempty" flags for rxgainerror */
	pi->srom_rxgainerr2g_isempty = FALSE;
	pi->srom_rxgainerr5gl_isempty = FALSE;
	pi->srom_rxgainerr5gm_isempty = FALSE;
	pi->srom_rxgainerr5gh_isempty = FALSE;
	pi->srom_rxgainerr5gu_isempty = FALSE;

	pi->phywatchdog_override = TRUE;

	/* pi->phy_rx_diglpf_default_coeffs are not set yet */
	pi->phy_rx_diglpf_default_coeffs_valid = FALSE;

	/* minimum reliable txpwr target is 8 dBm/mimo, 10 dbm/legacy  */
	if (ISNPHY(pi) || ISHTPHY(pi))
		pi->min_txpower = PHY_TXPWR_MIN_NPHY;
	else
		pi->min_txpower = PHY_TXPWR_MIN;

	/* Enable both cores by default */
	pi->sh->phyrxchain = 0x3;

#ifdef N2WOWL
	/* Reduce phyrxchain to 1 to save power in WOWL mode */
	if (CHIPID(pi->sh->chip) == BCM43237_CHIP_ID) {
		pi->sh->phyrxchain = 0x1;
	}
#endif /* N2WOWL */

#if defined(WLTEST)
	/* Initialize to invalid index values */
	pi->nphy_tbldump_minidx = -1;
	pi->nphy_tbldump_maxidx = -1;
	pi->nphy_phyreg_skipcnt = 0;
#endif

	/* Parameters for temperature-based fallback to 1-Tx chain */
	wlc_phy_txcore_temp(pi);

	pi->phy_tempsense_offset = 0;

	/* This is the temperature at which the last PHYCAL was done.
	 * Initialize to a very low value.
	 */
	pi->def_cal_info.last_cal_temp = -50;

#ifdef ENABLE_FCBS
	pi->phy_fcbs.start_template_offset[FCBS_CHAN_A] = FCBS_TMPLRAM_STARTADDR;
#endif /* ENABLE_FCBS */

	/* only NPHY/LPPHY support interrupt based noise measurement */
	pi->phynoise_polling = TRUE;
	if (ISNPHY(pi) || ISLPPHY(pi) || ISLCNCOMMONPHY(pi) || ISHTPHY(pi))
		pi->phynoise_polling = FALSE;

	/* initialize our txpwr limit to a large value until we know what band/channel
	 * we settle on in wlc_up() set the txpwr user override to the max
	 */
	for (i = 0; i < TXP_NUM_RATES; i++) {
		pi->txpwr_limit[i] = WLC_TXPWR_MAX;
		pi->txpwr_env_limit[i] = WLC_TXPWR_MAX;
		pi->tx_user_target[i] = WLC_TXPWR_MAX;
	}

	/* default radio power */
	pi->radiopwr_override = RADIOPWR_OVERRIDE_DEF;

	/* user specified power is at the ant port */
	pi->user_txpwr_at_rfport = FALSE;

#ifdef WLNOKIA_NVMEM
	/* user specified power is at the ant port */
	pi->user_txpwr_at_rfport = TRUE;
#endif

	/* WARNING: each individual PHY attach and it's pi->u.pi_xphy is allocated
	 * NO member pi->u.pi-xphy->xx should be dereferenced above this point
	 */
	if (ISABGPHY(pi)) {
		if (!wlc_phy_attach_abgphy(pi, bandtype))
			return NULL;
	} else if (ISNPHY(pi) || ISHTPHY(pi)) {
		/* only use for NPHY for now */
		if (!(pi->phycal_timer = wlapi_init_timer(pi->sh->physhim,
			wlc_phy_timercb_phycal, pi, "phycal"))) {
			PHY_ERROR(("wlc_timers_init: wlapi_init_timer for phycal_timer failed\n"));
			goto err;
		}

		if (ISNPHY(pi)) {
			if (!wlc_phy_attach_nphy(pi))
				goto err;
		} else if (ISHTPHY(pi)) {
			if (!wlc_phy_attach_htphy(pi))
				goto err;
		}
#if defined(AP) && defined(RADAR)
		wlc_phy_radar_attach_nphy(pi);
#endif
	} else if (ISLPPHY(pi)) {
		if (!wlc_phy_attach_lpphy(pi))
			goto err;

	} else if (ISSSLPNPHY(pi)) {
		if (!wlc_phy_attach_sslpnphy(pi))
			goto err;

	} else if (ISLCNPHY(pi)) {
		if (!wlc_phy_attach_lcnphy(pi, bandtype))
			goto err;

	} else if (ISLCN40PHY(pi)) {
		if (!wlc_phy_attach_lcn40phy(pi))
			goto err;
	} else	{
		/* This is here to complete the preceding if */
		PHY_ERROR(("wlc_phy_attach: unknown phytype\n"));
	}
	/* Prepare for one-time initializations */
	wlc_set_phy_uninitted(pi);

	if (wlc_phy_radio_attach(pi) != BCME_OK)
		goto err;

	pi->tx_pwr_backoff = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_txpwrbckof, 6);
	pi->ucode_tssi_limit_en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssilimucod, 1);
	pi->rssi_corr_normal = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssicorrnorm, 0);
	pi->rssi_corr_boardatten = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_rssicorratten, 7);

	/* Good phy, increase refcnt and put it in list */
	pi->refcnt++;
	pi->next = pi->sh->phy_head;
	sh->phy_head = pi;

#ifdef BCMECICOEX
	si_eci_init(pi->sh->sih);
	/* notify BT that there is a single antenna */
	if (CHIPID(pi->sh->chip) == BCM4325_CHIP_ID)
		NOTIFY_BT_NUM_ANTENNA(pi->sh->sih, 0);
#endif /* BCMECICOEX */

	/* Mark that they are not longer available so we can error/assert.  Use a pointer
	 * to self as a flag.
	 */
	pi->vars = (char *)&pi->vars;

	/* Make a public copy of the attach time constant phy attributes */
	bcopy(&pi->pubpi, &pi->pubpi_ro, sizeof(wlc_phy_t));

	return &pi->pubpi_ro;

err:
#ifdef WLNOKIA_NVMEM
	if (pi && pi->noknvmem)
		wlc_phy_noknvmem_detach(sh->osh, pi->noknvmem);
#endif 
	if (pi)
		MFREE(sh->osh, pi, sizeof(phy_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_phy_detach)(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	PHY_TRACE(("wl: %s: pi = %p\n", __FUNCTION__, pi));

	if (pih) {
		if (--pi->refcnt) {
			return;
		}

		if (ISABGPHY(pi))
			wlc_phy_detach_abgphy(pi);

		if (pi->phycal_timer) {
			wlapi_free_timer(pi->sh->physhim, pi->phycal_timer);
			pi->phycal_timer = NULL;
		}
#ifdef WLNOKIA_NVMEM
		if (pi->noknvmem)
			wlc_phy_noknvmem_detach(pi->sh->osh, pi->noknvmem);
#endif /* WLNOKIA_NVMEM */
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
		pi->phy_calcache_on = FALSE;
		wlc_phy_cal_cache_deinit((wlc_phy_t *)pi);
#endif

		/* Quick-n-dirty remove from list */
		if (pi->sh->phy_head == pi)
			pi->sh->phy_head = pi->next;
		else if (pi->sh->phy_head->next == pi)
			pi->sh->phy_head->next = NULL;
		else
			ASSERT(0);

		if (pi->pi_fptr.detach)
			(pi->pi_fptr.detach)(pi);

		MFREE(pi->sh->osh, pi, sizeof(phy_info_t));
	}
}

bool
wlc_phy_get_phyversion(wlc_phy_t *pih, uint16 *phytype, uint16 *phyrev,
	uint16 *radioid, uint16 *radiover)
{
	phy_info_t *pi = (phy_info_t *)pih;
	*phytype = (uint16)pi->pubpi.phy_type;
	*phyrev = (uint16)pi->pubpi.phy_rev;
	*radioid = pi->pubpi.radioid;
	*radiover = pi->pubpi.radiorev;

	return TRUE;
}

bool
wlc_phy_get_encore(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	return pi->pubpi.abgphy_encore;
}

uint32
wlc_phy_get_coreflags(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	return pi->pubpi.coreflags;
}

/* Break a lengthy algorithm into smaller pieces using 0-length timer */
static void
wlc_phy_timercb_phycal(void *arg)
{
	phy_info_t *pi = (phy_info_t*)arg;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint delay_val = 5;
#ifdef WLMCHAN
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif

	/* Increase delay between phases to be longer than 2 video frames interval 16.7*2 */
	if (CHIPID(pi->sh->chip) == BCM43237_CHIP_ID)
		delay_val = 40;

	if (PHY_PERICAL_MPHASE_PENDING(pi)) {

		/* PHY_CAL(("wlc_phy_timercb_phycal: phase_id %d\n", pi->mphase_cal_phase_id)); */

		if (!pi->sh->up) {
			wlc_phy_cal_perical_mphase_reset(pi);
			return;
		}

		if (SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi)) {
			/* delay percal until scan completed */
			PHY_CAL(("wlc_phy_timercb_phycal: scan in progress, delay 1 sec\n"));
			delay_val = 1000;	/* delay 1 sec */
			/* PHYCAL_CACHING does not interact with mphase */
#ifdef WLMCHAN
			if (!ctx)
#endif
				wlc_phy_cal_perical_mphase_restart(pi);
		} else {
			if (ISNPHY(pi)) {
				wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_AUTO);
			} else if (ISHTPHY(pi)) {
				/* pick up the search type from what the scheduler requested
				 * (INITIAL or INCREMENTAL) and call the calibration
				 */
				wlc_phy_cals_htphy(pi, pi->cal_info->cal_searchmode);
			} else {
				ASSERT(0); /* other phys not expected here */
			}
		}

		if (!(pi_nphy->ntd_papdcal_dcs == TRUE &&
			pi->cal_info->cal_phase_id == MPHASE_CAL_STATE_RXCAL))
			wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, delay_val, 0);
		else {
			pi_nphy->ntd_papdcal_dcs = FALSE;
			wlc_phy_cal_perical_mphase_reset(pi);
		}
		return;
	}

	PHY_CAL(("wlc_phy_timercb_phycal: mphase phycal is done\n"));
}

void
wlc_phy_anacore(wlc_phy_t *pih, bool on)
{
	phy_info_t *pi = (phy_info_t*)pih;

	PHY_TRACE(("wl%d: %s %i\n", pi->sh->unit, __FUNCTION__, on));

	if (ISNPHY(pi)) {
		if (on) {
			if (NREV_GE(pi->pubpi.phy_rev, 3)) {
				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore1,     0x0d)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride1, 0x0)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore2,     0x0d)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride2, 0x0)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
				phy_reg_write(pi, NPHY_AfectrlOverride, 0x0);
			}
		} else {
			if (NREV_GE(pi->pubpi.phy_rev, 3)) {
				PHY_REG_LIST_START
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride1, 0x07ff)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore1,     0x0fd)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlOverride2, 0x07ff)
					PHY_REG_WRITE_ENTRY(NPHY, AfectrlCore2,     0x0fd)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
				phy_reg_write(pi, NPHY_AfectrlOverride, 0x7fff);
			}
		}
	} else if (ISHTPHY(pi)) {
		wlc_phy_anacore_htphy(pi, on);
	} else if (ISLPPHY(pi)) {
		if (on)
			phy_reg_and(pi, LPPHY_AfeCtrlOvr,
				~(LPPHY_AfeCtrlOvr_pwdn_adc_ovr_MASK |
				LPPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
				LPPHY_AfeCtrlOvr_pwdn_rssi_ovr_MASK));
		else {
			PHY_REG_LIST_START
				PHY_REG_OR_ENTRY(LPPHY, AfeCtrlOvrVal,
					LPPHY_AfeCtrlOvrVal_pwdn_adc_ovr_val_MASK |
					LPPHY_AfeCtrlOvrVal_pwdn_dac_ovr_val_MASK |
					LPPHY_AfeCtrlOvrVal_pwdn_rssi_ovr_val_MASK)
				PHY_REG_OR_ENTRY(LPPHY, AfeCtrlOvr,
					LPPHY_AfeCtrlOvr_pwdn_adc_ovr_MASK |
					LPPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
					LPPHY_AfeCtrlOvr_pwdn_rssi_ovr_MASK)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else if (ISSSLPNPHY(pi))  {
		if (on) {
			PHY_REG_LIST_START
				PHY_REG_AND_ENTRY(SSLPNPHY, AfeCtrlOvrVal,
					~(SSLPNPHY_AfeCtrlOvrVal_pwdn_adc_ovr_val_MASK |
					SSLPNPHY_AfeCtrlOvrVal_pwdn_dac_ovr_val_MASK |
					SSLPNPHY_AfeCtrlOvrVal_pwdn_rssi_ovr_val_MASK))
				PHY_REG_AND_ENTRY(SSLPNPHY, AfeCtrlOvr,
					~(SSLPNPHY_AfeCtrlOvr_pwdn_adc_ovr_MASK |
					SSLPNPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
					SSLPNPHY_AfeCtrlOvr_pwdn_rssi_ovr_MASK))
			PHY_REG_LIST_EXECUTE(pi);
		} else  {
			PHY_REG_LIST_START
				PHY_REG_OR_ENTRY(SSLPNPHY, AfeCtrlOvrVal,
					SSLPNPHY_AfeCtrlOvrVal_pwdn_adc_ovr_val_MASK |
					SSLPNPHY_AfeCtrlOvrVal_pwdn_dac_ovr_val_MASK |
					SSLPNPHY_AfeCtrlOvrVal_pwdn_rssi_ovr_val_MASK)
				PHY_REG_OR_ENTRY(SSLPNPHY, AfeCtrlOvr,
					SSLPNPHY_AfeCtrlOvr_pwdn_adc_ovr_MASK |
					SSLPNPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
					SSLPNPHY_AfeCtrlOvr_pwdn_rssi_ovr_MASK)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else if (ISLCNPHY(pi)) {
		if (on) {
			phy_reg_and(pi, LCNPHY_AfeCtrlOvr,
				~(LCNPHY_AfeCtrlOvr_pwdn_adc_ovr_MASK |
				LCNPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
				LCNPHY_AfeCtrlOvr_pwdn_rssi_ovr_MASK));
		} else  {
			PHY_REG_LIST_START
				PHY_REG_OR_ENTRY(LCNPHY, AfeCtrlOvrVal,
					LCNPHY_AfeCtrlOvrVal_pwdn_adc_ovr_val_MASK |
					LCNPHY_AfeCtrlOvrVal_pwdn_dac_ovr_val_MASK |
					LCNPHY_AfeCtrlOvrVal_pwdn_rssi_ovr_val_MASK)
				PHY_REG_OR_ENTRY(LCNPHY, AfeCtrlOvr,
					LCNPHY_AfeCtrlOvr_pwdn_adc_ovr_MASK |
					LCNPHY_AfeCtrlOvr_pwdn_dac_ovr_MASK |
					LCNPHY_AfeCtrlOvr_pwdn_rssi_ovr_MASK)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else if (pi->pi_fptr.anacore) {
		pi->pi_fptr.anacore(pi, on);
	} else {
		if (on)
			W_REG(pi->sh->osh, &pi->regs->phyanacore, 0x0);
		else
			W_REG(pi->sh->osh, &pi->regs->phyanacore, 0xF4);
	}
}

static const char BCMATTACHDATA(rstr_tempthresh)[] = "tempthresh";
static const char BCMATTACHDATA(rstr_temps_hysteresis)[] = "temps_hysteresis";

void
BCMATTACHFN(wlc_phy_txcore_temp)(phy_info_t *pi)
{
	uint8 init_txrxchain = (1 << pi->pubpi.phy_corenum) - 1;

	pi->txcore_temp.disable_temp = (uint8)PHY_GETINTVAR(pi, rstr_tempthresh);
	if ((pi->txcore_temp.disable_temp == 0) || (pi->txcore_temp.disable_temp == 0xff)) {
		if (ISHTPHY(pi)) {
			pi->txcore_temp.disable_temp = HTPHY_CHAIN_TX_DISABLE_TEMP;
		} else {
			pi->txcore_temp.disable_temp = PHY_CHAIN_TX_DISABLE_TEMP;
		}
	}

	pi->txcore_temp.disable_temp_max_cap = pi->txcore_temp.disable_temp;

	pi->txcore_temp.hysteresis = (uint8)PHY_GETINTVAR(pi, rstr_temps_hysteresis);
	if ((pi->txcore_temp.hysteresis == 0) || (pi->txcore_temp.hysteresis == 0xf)) {
		pi->txcore_temp.hysteresis = PHY_HYSTERESIS_DELTATEMP;
	}

	pi->txcore_temp.enable_temp =
		pi->txcore_temp.disable_temp - pi->txcore_temp.hysteresis;

	pi->txcore_temp.heatedup = FALSE;
	pi->txcore_temp.degrade1RXen = FALSE;

	pi->txcore_temp.bitmap = (init_txrxchain << 4 | init_txrxchain);
	return;

}


uint32
wlc_phy_clk_bwbits(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	uint32 phy_bw_clkbits = 0;

	/* select the phy speed according to selected channel b/w applies to NPHY's only */
	if (pi && (ISNPHY(pi) || ISHTPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi))) {
		switch (pi->bw) {
			case WL_CHANSPEC_BW_10:
				phy_bw_clkbits = SICF_BW10;
				break;
			case WL_CHANSPEC_BW_20:
				phy_bw_clkbits = SICF_BW20;
				break;
			case WL_CHANSPEC_BW_40:
				phy_bw_clkbits = SICF_BW40;
				break;
			default:
				ASSERT(0); /* should never get here */
				break;
		}
	}

	return phy_bw_clkbits;
}

void
WLBANDINITFN(wlc_phy_por_inform)(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->phy_init_por = TRUE;
}

void
wlc_phy_btclock_war(wlc_phy_t *ppi, bool enable)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->btclock_tune = enable;
}

void
wlc_phy_preamble_override_set(wlc_phy_t *ppi, int8 val)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->n_preamble_override = val;
}

int8
wlc_phy_preamble_override_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->n_preamble_override;
}

/* increase the threshold to avoid false edcrs detection, non-11n only */
void
wlc_phy_edcrs_lock(wlc_phy_t *pih, bool lock)
{
	phy_info_t *pi = (phy_info_t *)pih;
	pi->edcrs_threshold_lock = lock;

	/* assertion: -59dB, deassertion: -67dB */
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh0, 0x46b)
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UAssertThresh1, 0x46b)
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh0, 0x3c0)
		PHY_REG_WRITE_ENTRY(NPHY, ed_crs20UDeassertThresh1, 0x3c0)
	PHY_REG_LIST_EXECUTE(pi);
}

void
wlc_phy_initcal_enable(wlc_phy_t *pih, bool initcal)
{
	phy_info_t *pi = (phy_info_t *)pih;
	if (ISNPHY(pi))
		pi->u.pi_nphy->do_initcal = initcal;
}

void
wlc_phy_hw_clk_state_upd(wlc_phy_t *pih, bool newstate)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!pi || !pi->sh)
		return;

	pi->sh->clk = newstate;
}

void
wlc_phy_hw_state_upd(wlc_phy_t *pih, bool newstate)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!pi || !pi->sh)
		return;

	pi->sh->up = newstate;
}

void
WLBANDINITFN(wlc_phy_init)(wlc_phy_t *pih, chanspec_t chanspec)
{
	uint32	mc;
	initfn_t phy_init = NULL;
	phy_info_t *pi = (phy_info_t *)pih;
#ifdef BCMDBG
	char chbuf[CHANSPEC_STR_LEN];
	PHY_TRACE(("wl%d: %s chanspec %s\n", pi->sh->unit, __FUNCTION__,
		wf_chspec_ntoa(chanspec, chbuf)));
#endif

	/* skip if this function is called recursively(e.g. when bw is changed) */
	if (pi->init_in_progress)
		return;

	pi->init_in_progress = TRUE;

	pi->radio_chanspec = chanspec;

	mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);
	if ((mc & MCTL_EN_MAC) != 0) {
		if (mc == 0xffffffff)
			PHY_ERROR(("wl%d: wlc_phy_init: chip is dead !!!\n", pi->sh->unit));
		else
			PHY_ERROR(("wl%d: wlc_phy_init:MAC running! mc=0x%x\n",
			          pi->sh->unit, mc));
		ASSERT((const char*)"wlc_phy_init: Called with the MAC running!" == NULL);
	}

	ASSERT(pi != NULL);

	/* clear during init. To be set by higher level wlc code */
	pi->cur_interference_mode = INTERFERE_NONE;

	/* init PUB_NOT_ASSOC */
	if (!(pi->measure_hold & PHY_HOLD_FOR_SCAN) &&
		!(pi->interf.aci.nphy.detection_in_progress)) {
		pi->measure_hold |= PHY_HOLD_FOR_NOT_ASSOC;
	}

	/* check D11 is running on Fast Clock */
	if (D11REV_GE(pi->sh->corerev, 5))
		ASSERT(si_core_sflags(pi->sh->sih, 0, 0) & SISF_FCLKA);

	phy_init = pi->pi_fptr.init;

	if (phy_init == NULL) {
		PHY_ERROR(("wl%d: %s: No phy_init found for phy_type %d, rev %d\n",
		          pi->sh->unit, __FUNCTION__, pi->pubpi.phy_type, pi->pubpi.phy_rev));
		ASSERT(phy_init != NULL);
		return;
	}

	wlc_phy_anacore(pih, ON);

	/* sanitize bw here to avoid later mess. wlc_set_bw will invoke phy_reset,
	 *  but phy_init recursion is avoided by using init_in_progress
	 */
	if (CHSPEC_BW(pi->radio_chanspec) != pi->bw)
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(pi->radio_chanspec));

	/* Reset gain_boost & aci on band-change */
	pi->nphy_gain_boost = TRUE;
	if (ISNPHY(pi) || ISHTPHY(pi)) {
		if (NREV_LT(pi->pubpi.phy_rev, 3)) {
			/* when scanning to different band, don't change aci_state */
			/* but keep phy rev < 3 the same as before */
			pi->aci_state &= ~ACI_ACTIVE;
		}
		/* Reset ACI internals if not scanning and not in aci_detection */
		if (!(SCAN_INPROG_PHY(pi) ||
		pi->interf.aci.nphy.detection_in_progress)) {
			if (ISNPHY(pi))
				wlc_phy_aci_sw_reset_nphy(pi);
			else if (ISHTPHY(pi))
				wlc_phy_aci_sw_reset_htphy(pi);
		}
	}

	/* radio on */
	wlc_phy_switch_radio((wlc_phy_t*)pi, ON);

	/* !! kick off the phy init !! */
	(*phy_init)(pi);

	/* Indicate a power on reset isn't needed for future phy init's */
	pi->phy_init_por = FALSE;

	if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
		wlc_phy_do_dummy_tx(pi, TRUE, OFF);

	/* Save the w/b frequency tracking registers */
	if (ISGPHY(pi)) {
		pi->freqtrack_saved_regs[0] = phy_reg_read(pi, BPHY_COEFFS);
		pi->freqtrack_saved_regs[1] = phy_reg_read(pi, BPHY_STEP);
	}

	/* init txpwr shared memory if sw or ucode are doing tx power control */
	if (!(ISNPHY(pi) || ISHTPHY(pi) || ISSSLPNPHY(pi)))
		wlc_phy_txpower_update_shm(pi);

	/* initialize rx antenna diversity */
	wlc_phy_ant_rxdiv_set((wlc_phy_t *)pi, pi->sh->rx_antdiv);


	/* initialize interference algorithms */
	if (((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi)) &&
		(SCAN_INPROG_PHY(pi))) {
		/* do not reinitialize interference mode, could be scanning */
	} else {

		if (pi->sh->interference_mode_override == TRUE) {
			/* keep the same values */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				pi->sh->interference_mode =
					pi->sh->interference_mode_2G_override;
			} else {
				/* for 5G, only values 0 and 1 are valid options */
				if (pi->sh->interference_mode_5G_override == 0 ||
					pi->sh->interference_mode_5G_override == 1) {
					pi->sh->interference_mode =
						pi->sh->interference_mode_5G_override;
				} else {
					/* used invalid value. so default to 0 */
					pi->sh->interference_mode = 0;
				}
			}
		} else {
		        if (CHSPEC_IS2G(pi->radio_chanspec)) {
			        pi->sh->interference_mode = pi->sh->interference_mode_2G;
		        } else {
			        pi->sh->interference_mode = pi->sh->interference_mode_5G;
		        }
		}
		wlc_phy_interference(pi, pi->sh->interference_mode, FALSE);
	}

	pi->init_in_progress = FALSE;

	pi->bt_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_BTCX_BLK_PTR);
}

/*
 * Do one-time phy initializations and calibration.
 *
 * Note: no register accesses allowed; we have not yet waited for PLL
 * since the last corereset.
 */
void
BCMINITFN(wlc_phy_cal_init)(wlc_phy_t *pih)
{
	int i;
	phy_info_t *pi = (phy_info_t *)pih;
	initfn_t cal_init = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT((R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC) == 0);

	if (ISAPHY(pi))
		pi->txpwridx = DEFAULT_11A_TXP_IDX;


	if (!pi->initialized) {
		/* glitch counter init */
		/* detection is called only if high glitches are observed */
		pi->interf.aci.glitch_ma = ACI_INIT_MA;
		pi->interf.aci.glitch_ma_previous = ACI_INIT_MA;
		pi->interf.aci.pre_glitch_cnt = 0;
		pi->interf.aci.ma_total = MA_WINDOW_SZ * ACI_INIT_MA;
		for (i = 0; i < MA_WINDOW_SZ; i++)
			pi->interf.aci.ma_list[i] = ACI_INIT_MA;

		for (i = 0; i < PHY_NOISE_MA_WINDOW_SZ; i++) {
			pi->interf.noise.ofdm_glitch_ma_list[i] = PHY_NOISE_GLITCH_INIT_MA;
			pi->interf.noise.ofdm_badplcp_ma_list[i] = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		}

		pi->interf.noise.ofdm_glitch_ma = PHY_NOISE_GLITCH_INIT_MA;
		pi->interf.noise.ofdm_glitch_ma_previous = PHY_NOISE_GLITCH_INIT_MA;
		pi->interf.noise.bphy_pre_glitch_cnt = 0;
		pi->interf.noise.ofdm_ma_total = PHY_NOISE_GLITCH_INIT_MA * PHY_NOISE_MA_WINDOW_SZ;

		pi->interf.badplcp_ma = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf.badplcp_ma_previous = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf.badplcp_ma_total = PHY_NOISE_GLITCH_INIT_MA_BADPlCP * MA_WINDOW_SZ;
		pi->interf.pre_badplcp_cnt = 0;
		pi->interf.bphy_pre_badplcp_cnt = 0;

		pi->interf.noise.ofdm_badplcp_ma = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf.noise.ofdm_badplcp_ma_previous = PHY_NOISE_GLITCH_INIT_MA_BADPlCP;
		pi->interf.noise.ofdm_badplcp_ma_total =
			PHY_NOISE_GLITCH_INIT_MA_BADPlCP * PHY_NOISE_MA_WINDOW_SZ;

		cal_init = pi->pi_fptr.calinit;
		if (cal_init)
			(*cal_init)(pi);

		pi->initialized = TRUE;
	}
}


int
BCMUNINITFN(wlc_phy_down)(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int callbacks = 0;


	/* all activate phytest should have been stopped */
	ASSERT(pi->phytest_on == FALSE);

	/* cancel phycal timer if exists */
	if (pi->phycal_timer && !wlapi_del_timer(pi->sh->physhim, pi->phycal_timer))
		callbacks++;

	if (ISNPHY(pi)) {
		pi->u.pi_nphy->nphy_iqcal_chanspec_2G = 0;
		pi->u.pi_nphy->nphy_iqcal_chanspec_5G = 0;
}

	return callbacks;
}

static uint32
wlc_phy_get_radio_ver(phy_info_t *pi)
{
	uint32 ver;

	ver = read_radio_id(pi);

	PHY_INFORM(("wl%d: %s: IDCODE = 0x%x\n", pi->sh->unit, __FUNCTION__, ver));
	return ver;
}

#if defined(WLTEST) || defined(WL_PHYACIARGS)
static int
wlc_phy_aci_args(phy_info_t *pi, wl_aci_args_t *params, bool get, int len)
{
	bool nphy_aciarg;

	if (len == WL_ACI_ARGS_LEGACY_LENGTH)
		nphy_aciarg = FALSE;
	else if (len == sizeof(wl_aci_args_t))
		nphy_aciarg = TRUE;
	else
		return BCME_BUFTOOSHORT;

	if (get) {
		params->enter_aci_thresh = pi->interf.aci.enter_thresh;
		params->exit_aci_thresh = pi->interf.aci.exit_thresh;
		params->usec_spin = pi->interf.aci.usec_spintime;
		params->glitch_delay = pi->interf.aci.glitch_delay;
	} else {
		if (params->enter_aci_thresh > 0)
			pi->interf.aci.enter_thresh = params->enter_aci_thresh;
		if (params->exit_aci_thresh > 0)
			pi->interf.aci.exit_thresh = params->exit_aci_thresh;
		if (params->usec_spin > 0)
			pi->interf.aci.usec_spintime = params->usec_spin;
		if (params->glitch_delay > 0)
			pi->interf.aci.glitch_delay = params->glitch_delay;
	}

	if (nphy_aciarg) {
		if (get) {
			params->nphy_adcpwr_enter_thresh = pi->interf.aci.nphy.adcpwr_enter_thresh;
			params->nphy_adcpwr_exit_thresh = pi->interf.aci.nphy.adcpwr_exit_thresh;
			params->nphy_repeat_ctr = pi->interf.aci.nphy.detect_repeat_ctr;
			params->nphy_num_samples = pi->interf.aci.nphy.detect_num_samples;
			params->nphy_undetect_window_sz = pi->interf.aci.nphy.undetect_window_sz;
			params->nphy_b_energy_lo_aci = pi->interf.aci.nphy.b_energy_lo_aci;
			params->nphy_b_energy_md_aci = pi->interf.aci.nphy.b_energy_md_aci;
			params->nphy_b_energy_hi_aci = pi->interf.aci.nphy.b_energy_hi_aci;

			params->nphy_noise_noassoc_glitch_th_up =
				pi->interf.noise.nphy_noise_noassoc_glitch_th_up;
			params->nphy_noise_noassoc_glitch_th_dn =
				pi->interf.noise.nphy_noise_noassoc_glitch_th_dn;
			params->nphy_noise_assoc_glitch_th_up =
				pi->interf.noise.nphy_noise_assoc_glitch_th_up;
			params->nphy_noise_assoc_glitch_th_dn =
				pi->interf.noise.nphy_noise_assoc_glitch_th_dn;
			params->nphy_noise_assoc_aci_glitch_th_up =
				pi->interf.noise.nphy_noise_assoc_aci_glitch_th_up;
			params->nphy_noise_assoc_aci_glitch_th_dn =
				pi->interf.noise.nphy_noise_assoc_aci_glitch_th_dn;
			params->nphy_noise_assoc_enter_th =
				pi->interf.noise.nphy_noise_assoc_enter_th;
			params->nphy_noise_noassoc_enter_th =
				pi->interf.noise.nphy_noise_noassoc_enter_th;
			params->nphy_noise_assoc_rx_glitch_badplcp_enter_th=
				pi->interf.noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th;
			params->nphy_noise_noassoc_crsidx_incr=
				pi->interf.noise.nphy_noise_noassoc_crsidx_incr;
			params->nphy_noise_assoc_crsidx_incr=
				pi->interf.noise.nphy_noise_assoc_crsidx_incr;
			params->nphy_noise_crsidx_decr=
				pi->interf.noise.nphy_noise_crsidx_decr;

		} else {
			pi->interf.aci.nphy.adcpwr_enter_thresh = params->nphy_adcpwr_enter_thresh;
			pi->interf.aci.nphy.adcpwr_exit_thresh = params->nphy_adcpwr_exit_thresh;
			pi->interf.aci.nphy.detect_repeat_ctr = params->nphy_repeat_ctr;
			pi->interf.aci.nphy.detect_num_samples = params->nphy_num_samples;
			pi->interf.aci.nphy.undetect_window_sz =
				MIN(params->nphy_undetect_window_sz,
				ACI_MAX_UNDETECT_WINDOW_SZ);
			pi->interf.aci.nphy.b_energy_lo_aci = params->nphy_b_energy_lo_aci;
			pi->interf.aci.nphy.b_energy_md_aci = params->nphy_b_energy_md_aci;
			pi->interf.aci.nphy.b_energy_hi_aci = params->nphy_b_energy_hi_aci;

			pi->interf.noise.nphy_noise_noassoc_glitch_th_up =
				params->nphy_noise_noassoc_glitch_th_up;
			pi->interf.noise.nphy_noise_noassoc_glitch_th_dn =
				params->nphy_noise_noassoc_glitch_th_dn;
			pi->interf.noise.nphy_noise_assoc_glitch_th_up =
				params->nphy_noise_assoc_glitch_th_up;
			pi->interf.noise.nphy_noise_assoc_glitch_th_dn =
				params->nphy_noise_assoc_glitch_th_dn;
			pi->interf.noise.nphy_noise_assoc_aci_glitch_th_up =
				params->nphy_noise_assoc_aci_glitch_th_up;
			pi->interf.noise.nphy_noise_assoc_aci_glitch_th_dn =
				params->nphy_noise_assoc_aci_glitch_th_dn;
			pi->interf.noise.nphy_noise_assoc_enter_th =
				params->nphy_noise_assoc_enter_th;
			pi->interf.noise.nphy_noise_noassoc_enter_th =
				params->nphy_noise_noassoc_enter_th;
			pi->interf.noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th =
				params->nphy_noise_assoc_rx_glitch_badplcp_enter_th;
			pi->interf.noise.nphy_noise_noassoc_crsidx_incr =
				params->nphy_noise_noassoc_crsidx_incr;
			pi->interf.noise.nphy_noise_assoc_crsidx_incr =
				params->nphy_noise_assoc_crsidx_incr;
			pi->interf.noise.nphy_noise_crsidx_decr =
				params->nphy_noise_crsidx_decr;
		}
	}

	return BCME_OK;
}
#endif 



/* Do the initial table address write given the phy specific table access register
 * locations, the table ID and offset to start read/write operations
 */
void
wlc_phy_table_addr(phy_info_t *pi, uint tbl_id, uint tbl_offset,
                   uint16 tblAddr, uint16 tblDataHi, uint16 tblDataLo)
{
	PHY_TRACE(("wl%d: %s ID %u offset %u\n", pi->sh->unit, __FUNCTION__, tbl_id, tbl_offset));

	phy_reg_write(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	pi->tbl_data_hi = tblDataHi;
	pi->tbl_data_lo = tblDataLo;

	if ((CHIPID(pi->sh->chip) == BCM43224_CHIP_ID ||
	     CHIPID(pi->sh->chip) == BCM43421_CHIP_ID) &&
	    (pi->sh->chiprev == 1)) {
		pi->tbl_addr = tblAddr;
		pi->tbl_save_id = tbl_id;
		pi->tbl_save_offset = tbl_offset;
	}
}

/* Write the given value to the phy table which has been set up with an earlier call
 * to wlc_phy_table_addr()
 */
void
wlc_phy_table_data_write(phy_info_t *pi, uint width, uint32 val)
{
	ASSERT((width == 8) || (width == 16) || (width == 32));

	PHY_TRACE(("wl%d: %s width %u val %u\n", pi->sh->unit, __FUNCTION__, width, val));

	if ((CHIPID(pi->sh->chip) == BCM43224_CHIP_ID ||
	     CHIPID(pi->sh->chip) == BCM43421_CHIP_ID) &&
	    (pi->sh->chiprev == 1) &&
	    (pi->tbl_save_id == NPHY_TBL_ID_ANTSWCTRLLUT)) {
		phy_reg_read(pi, pi->tbl_data_lo);
		/* roll back the address from the dummy read */
		phy_reg_write(pi, pi->tbl_addr, (pi->tbl_save_id << 10) | pi->tbl_save_offset);
		pi->tbl_save_offset++;
	}

	if (width == 32) {
		/* width is 32-bit */
		phy_reg_write(pi, pi->tbl_data_hi, (uint16)(val >> 16));
		phy_reg_write(pi, pi->tbl_data_lo, (uint16)val);
	} else {
		/* width is 16-bit or 8-bit */
		phy_reg_write(pi, pi->tbl_data_lo, (uint16)val);
	}
}

/* Takes the table name, list of entries, offset to load the table,
 * see xxxphyprocs.tcl, proc xxxphy_write_table
 */
void
wlc_phy_write_table(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDataLo)
{
	uint    idx;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint	tbl_width = ptbl_info->tbl_width;
	const uint8  *ptbl_8b    = (const uint8  *)ptbl_info->tbl_ptr;
	const uint16 *ptbl_16b   = (const uint16 *)ptbl_info->tbl_ptr;
	const uint32 *ptbl_32b   = (const uint32 *)ptbl_info->tbl_ptr;

	ASSERT((tbl_width == 8) || (tbl_width == 16) ||
		(tbl_width == 32));

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	phy_reg_write(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {

		if ((CHIPID(pi->sh->chip) == BCM43224_CHIP_ID ||
		     CHIPID(pi->sh->chip) == BCM43421_CHIP_ID) &&
		    (pi->sh->chiprev == 1) &&
		    (tbl_id == NPHY_TBL_ID_ANTSWCTRLLUT)) {
			phy_reg_read(pi, tblDataLo);
			/* roll back the address from the dummy read */
			phy_reg_write(pi, tblAddr, (tbl_id << 10) | (tbl_offset + idx));
		}

		if (tbl_width == 32) {
			/* width is 32-bit */
			phy_reg_write(pi, tblDataHi, (uint16)(ptbl_32b[idx] >> 16));
			phy_reg_write(pi, tblDataLo, (uint16)ptbl_32b[idx]);
		} else if (tbl_width == 16) {
			/* width is 16-bit */
			phy_reg_write(pi, tblDataLo, ptbl_16b[idx]);
		} else {
			/* width is 8-bit */
			phy_reg_write(pi, tblDataLo, ptbl_8b[idx]);
		}
	}
}

void
wlc_phy_read_table(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDataLo)
{
	uint    idx;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint	tbl_width = ptbl_info->tbl_width;
	uint8  *ptbl_8b    = (uint8  *)(uintptr)ptbl_info->tbl_ptr;
	uint16 *ptbl_16b   = (uint16 *)(uintptr)ptbl_info->tbl_ptr;
	uint32 *ptbl_32b   = (uint32 *)(uintptr)ptbl_info->tbl_ptr;

	ASSERT((tbl_width == 8) || (tbl_width == 16) ||
		(tbl_width == 32));

	phy_reg_write(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {

		if ((CHIPID(pi->sh->chip) == BCM43224_CHIP_ID ||
		     CHIPID(pi->sh->chip) == BCM43421_CHIP_ID) &&
		    (pi->sh->chiprev == 1)) {
			(void)phy_reg_read(pi, tblDataLo);
			/* roll back the address from the dummy read */
			phy_reg_write(pi, tblAddr, (tbl_id << 10) | (tbl_offset + idx));
		}

		if (tbl_width == 32) {
			/* width is 32-bit */
			ptbl_32b[idx]  =  phy_reg_read(pi, tblDataLo);
			ptbl_32b[idx] |= (phy_reg_read(pi, tblDataHi) << 16);
		} else if (tbl_width == 16) {
			/* width is 16-bit */
			ptbl_16b[idx]  =  phy_reg_read(pi, tblDataLo);
		} else {
			/* width is 8-bit */
			ptbl_8b[idx]   =  (uint8)phy_reg_read(pi, tblDataLo);
		}
	}
}

void
wlc_phy_common_read_table(phy_info_t *pi, uint32 tbl_id,
	const void *tbl_ptr, uint32 tbl_len, uint32 tbl_width,
	uint32 tbl_offset,
	void (*tbl_rfunc)(phy_info_t *, phytbl_info_t *))
{
	phytbl_info_t tab;
	tab.tbl_id = tbl_id;
	tab.tbl_ptr = tbl_ptr;	/* ptr to buf */
	tab.tbl_len = tbl_len;			/* # values   */
	tab.tbl_width = tbl_width;			/* gain_val_tbl_rev3 */
	tab.tbl_offset = tbl_offset;		/* tbl offset */
	tbl_rfunc(pi, &tab);
}

void
wlc_phy_common_write_table(phy_info_t *pi, uint32 tbl_id, const void *tbl_ptr,
	uint32 tbl_len, uint32 tbl_width, uint32 tbl_offset,
	void (*tbl_wfunc)(phy_info_t *, const phytbl_info_t *))
{

	phytbl_info_t tab;
	tab.tbl_id = tbl_id;
	tab.tbl_ptr = tbl_ptr;	/* ptr to buf */
	tab.tbl_len = tbl_len;			/* # values   */
	tab.tbl_width = tbl_width;			/* gain_val_tbl_rev3 */
	tab.tbl_offset = tbl_offset;		/* tbl offset */
	tbl_wfunc(pi, &tab);
}
/* Takes the table name, list of entries, offset to load the table,
 * see xxxphyprocs.tcl, proc xxxphy_write_table
 */
void
wlc_phy_table_write_lpphy(phy_info_t *pi, const lpphytbl_info_t *ptbl_info)
{
	uint    idx;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint32	u32temp;

	const uint8  *ptbl_8b    = (const uint8  *)ptbl_info->tbl_ptr;
	const uint16 *ptbl_16b   = (const uint16 *)ptbl_info->tbl_ptr;
	const uint32 *ptbl_32b   = (const uint32 *)ptbl_info->tbl_ptr;

	uint16 tblAddr = LPPHY_TableAddress;
	uint16 tblDataHi = LPPHY_TabledataHi;
	uint16 tblDatalo = LPPHY_TabledataLo;

	ASSERT((ptbl_info->tbl_phywidth == 8) || (ptbl_info->tbl_phywidth == 16) ||
		(ptbl_info->tbl_phywidth == 32));
	ASSERT((ptbl_info->tbl_width == 8) || (ptbl_info->tbl_width == 16) ||
		(ptbl_info->tbl_width == 32));

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_phy_table_lock_lpphy(pi);

	phy_reg_write(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {

		/* get the element from the table according to the table width */
		if (ptbl_info->tbl_width == 32) {
			/* tbl width is 32 bit */
			u32temp = (uint32)ptbl_32b[idx];
		} else if (ptbl_info->tbl_width == 16) {
			/* tbl width is 16 bit */
			u32temp = (uint32)ptbl_16b[idx];
		} else {
			/* tbl width is 8 bit */
			u32temp = (uint32)ptbl_8b[idx];
		}

		/* write the element into the phy table according phy address space width */
		if (ptbl_info->tbl_phywidth == 32) {
			/* phy width is 32-bit */
			phy_reg_write(pi, tblDataHi, (u32temp >> 16) & 0xffff);
			phy_reg_write(pi, tblDatalo, u32temp & 0xffff);
		} else if (ptbl_info->tbl_phywidth == 16) {
			/* phy width is 16-bit */
			phy_reg_write(pi, tblDatalo, u32temp & 0xffff);
		} else {
			/* phy width is 8-bit */
			phy_reg_write(pi, tblDatalo, u32temp & 0xffff);
		}
	}
	wlc_phy_table_unlock_lpphy(pi);
}

void
wlc_phy_table_read_lpphy(phy_info_t *pi, const lpphytbl_info_t *ptbl_info)
{
	uint    idx;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint32  u32temp;

	uint8  *ptbl_8b    = (uint8  *)(uintptr)ptbl_info->tbl_ptr;
	uint16 *ptbl_16b   = (uint16 *)(uintptr)ptbl_info->tbl_ptr;
	uint32 *ptbl_32b   = (uint32 *)(uintptr)ptbl_info->tbl_ptr;

	uint16 tblAddr = LPPHY_TableAddress;
	uint16 tblDataHi = LPPHY_TabledataHi;
	uint16 tblDatalo = LPPHY_TabledataLo;

	ASSERT((ptbl_info->tbl_phywidth == 8) || (ptbl_info->tbl_phywidth == 16) ||
		(ptbl_info->tbl_phywidth == 32));
	ASSERT((ptbl_info->tbl_width == 8) || (ptbl_info->tbl_width == 16) ||
		(ptbl_info->tbl_width == 32));

	wlc_phy_table_lock_lpphy(pi);

	phy_reg_write(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {

		/* get the element from phy according to the phy table element
		 * address space width
		 */
		if (ptbl_info->tbl_phywidth == 32) {
			/* phy width is 32-bit */
			u32temp  =  phy_reg_read(pi, tblDatalo);
			u32temp |= (phy_reg_read(pi, tblDataHi) << 16);
		} else if (ptbl_info->tbl_phywidth == 16) {
			/* phy width is 16-bit */
			u32temp  =  phy_reg_read(pi, tblDatalo);
		} else {
			/* phy width is 8-bit */
			u32temp   =  (uint8)phy_reg_read(pi, tblDatalo);
		}

		/* put the element into the table according to the table element width
		 * Note that phy table width is some times more than necessary while
		 * table width is always optimal.
		 */
		if (ptbl_info->tbl_width == 32) {
			/* tbl width is 32-bit */
			ptbl_32b[idx]  =  u32temp;
		} else if (ptbl_info->tbl_width == 16) {
			/* tbl width is 16-bit */
			ptbl_16b[idx]  =  (uint16)u32temp;
		} else {
			/* tbl width is 8-bit */
			ptbl_8b[idx]   =  (uint8)u32temp;
		}
	}
	wlc_phy_table_unlock_lpphy(pi);
}

/* prevent simultaneous phy table access by driver and ucode */
void
wlc_phy_table_lock_lpphy(phy_info_t *pi)
{
	uint32 mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);

	if (mc & MCTL_EN_MAC) {
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
		(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
		OSL_DELAY(5);
	}
}

void
wlc_phy_table_unlock_lpphy(phy_info_t *pi)
{
	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);
}

uint
wlc_phy_init_radio_regs_allbands(phy_info_t *pi, radio_20xx_regs_t *radioregs)
{
	uint i = 0;

	do {
		if (radioregs[i].do_init) {
			write_radio_reg(pi, radioregs[i].address,
			                (uint16)radioregs[i].init);
		}

		i++;
	} while (radioregs[i].address != 0xffff);

	return i;
}

uint
wlc_phy_init_radio_regs(phy_info_t *pi, radio_regs_t *radioregs, uint16 core_offset)
{
	uint i = 0;
	uint count = 0;

	do {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (radioregs[i].do_init_a) {
				write_radio_reg(pi, radioregs[i].address | core_offset,
					(uint16)radioregs[i].init_a);
				if (ISNPHY(pi) && (++count % 4 == 0))
					WLC_PHY_WAR_PR51571(pi);
			}
		} else {
			if (radioregs[i].do_init_g) {
				write_radio_reg(pi, radioregs[i].address | core_offset,
					(uint16)radioregs[i].init_g);
				if (ISNPHY(pi) && (++count % 4 == 0))
					WLC_PHY_WAR_PR51571(pi);
			}
		}

		i++;
	} while (radioregs[i].address != 0xffff);

	return i;
}

void
wlc_phy_do_dummy_tx(phy_info_t *pi, bool ofdm, bool pa_on)
{
#define	DUMMY_PKT_LEN	20 /* Dummy packet's length */
	d11regs_t *regs = pi->regs;
	int	i, count;
	uint8	ofdmpkt[DUMMY_PKT_LEN] = {
		0xcc, 0x01, 0x02, 0x00, 0x00, 0x00, 0xd4, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
	};
	uint8	cckpkt[DUMMY_PKT_LEN] = {
		0x6e, 0x84, 0x0b, 0x00, 0x00, 0x00, 0xd4, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
	};
	uint32 *dummypkt;

	ASSERT((R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC) == 0);

	dummypkt = (uint32 *)(ofdm ? ofdmpkt : cckpkt);
	wlapi_bmac_write_template_ram(pi->sh->physhim, 0, DUMMY_PKT_LEN, dummypkt);

	/* set up the TXE transfer */

	W_REG(pi->sh->osh, &regs->xmtsel, 0);
	/* Assign the WEP to the transmit path */
	if (D11REV_GE(pi->sh->corerev, 11))
		W_REG(pi->sh->osh, &regs->wepctl, 0x100);
	else
		W_REG(pi->sh->osh, &regs->wepctl, 0);

	/* Set/clear OFDM bit in PHY control word */
	W_REG(pi->sh->osh, &regs->txe_phyctl, (ofdm ? 1 : 0) | PHY_TXC_ANT_0);
	if (ISNPHY(pi) || ISHTPHY(pi) || ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi)) {
		ASSERT(ofdm);
		W_REG(pi->sh->osh, &regs->txe_phyctl1, 0x1A02);
	}

	W_REG(pi->sh->osh, &regs->txe_wm_0, 0);		/* No substitutions */
	W_REG(pi->sh->osh, &regs->txe_wm_1, 0);

	/* Set transmission from the TEMPLATE where we loaded the frame */
	W_REG(pi->sh->osh, &regs->xmttplatetxptr, 0);
	W_REG(pi->sh->osh, &regs->xmttxcnt, DUMMY_PKT_LEN);

	/* Set Template as source, length specified as a count and destination
	 * as Serializer also set "gen_eof"
	 */
	W_REG(pi->sh->osh, &regs->xmtsel, ((8 << 8) | (1 << 5) | (1 << 2) | 2));

	/* Instruct the MAC to not calculate FCS, we'll supply a bogus one */
	W_REG(pi->sh->osh, &regs->txe_ctl, 0);

	if (!pa_on) {
		if (ISNPHY(pi))
			wlc_phy_pa_override_nphy(pi, OFF);
		else if (ISHTPHY(pi)) {
			wlc_phy_pa_override_htphy(pi, OFF);
		}
	}

	/* Start transmission and wait until sendframe goes away */
	/* Set TX_NOW in AUX along with MK_CTLWRD */
	if (ISNPHY(pi) || ISHTPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi))
		W_REG(pi->sh->osh, &regs->txe_aux, 0xD0);
	else if (ISLPPHY(pi))
		W_REG(pi->sh->osh, &regs->txe_aux, ((1 << 6) | (1 << 4)));
	else
		W_REG(pi->sh->osh, &regs->txe_aux, ((1 << 5) | (1 << 4)));

	(void)R_REG(pi->sh->osh, &regs->txe_aux);

	/* Wait for 10 x ack time, enlarge it for vsim of QT */
	i = 0;
	count = ofdm ? 30 : 250;

#ifndef BCMQT_CPU
	if (ISSIM_ENAB(pi->sh->sih)) {
		count *= 100;
	}
#endif
	/* wait for txframe to be zero */
	while ((i++ < count) && (R_REG(pi->sh->osh, &regs->txe_status) & (1 << 7))) {
		OSL_DELAY(10);
	}
	if (i >= count)
		PHY_ERROR(("wl%d: %s: Waited %d uS for %s txframe\n",
		          pi->sh->unit, __FUNCTION__, 10 * i, (ofdm ? "ofdm" : "cck")));

	/* Wait for the mac to finish (this is 10x what is supposed to take) */
	i = 0;
	/* wait for txemend */
	while ((i++ < 10) && ((R_REG(pi->sh->osh, &regs->txe_status) & (1 << 10)) == 0)) {
		OSL_DELAY(10);
	}
	if (i >= 10)
		PHY_ERROR(("wl%d: %s: Waited %d uS for txemend\n",
		          pi->sh->unit, __FUNCTION__, 10 * i));

	/* Wait for the phy to finish */
	i = 0;
	/* wait for txcrs */
	while ((i++ < 10) && ((R_REG(pi->sh->osh, &regs->ifsstat) & (1 << 8)))) {
		OSL_DELAY(10);
	}
	if (i >= 10)
		PHY_ERROR(("wl%d: %s: Waited %d uS for txcrs\n",
		          pi->sh->unit, __FUNCTION__, 10 * i));
	if (!pa_on) {
		if (ISNPHY(pi))
			wlc_phy_pa_override_nphy(pi, ON);
		else if (ISHTPHY(pi)) {
			wlc_phy_pa_override_htphy(pi, ON);
		}
	}
}

static void
wlc_phy_scanroam_cache_cal(phy_info_t *pi, bool set)
{
	if (ISHTPHY(pi))
		wlc_phy_scanroam_cache_cal_htphy(pi, set);
}

void
wlc_phy_hold_upd(wlc_phy_t *pih, mbool id, bool set)
{
	phy_info_t *pi = (phy_info_t *)pih;
	ASSERT(id);

	PHY_TRACE(("%s: id %d val %d old pi->measure_hold 0%x\n", __FUNCTION__, id, set,
		pi->measure_hold));

	PHY_CAL(("wl%d: %s: %s %s flag\n", pi->sh->unit, __FUNCTION__,
		set ? "SET" : "CLR",
		(id == PHY_HOLD_FOR_ASSOC) ? "ASSOC" :
		((id == PHY_HOLD_FOR_SCAN) ? "SCAN" :
		((id == PHY_HOLD_FOR_SCAN) ? "SCAN" :
		((id == PHY_HOLD_FOR_RM) ? "RM" :
		((id == PHY_HOLD_FOR_PLT) ? "PLT" :
		((id == PHY_HOLD_FOR_MUTE) ? "MUTE" :
		((id == PHY_HOLD_FOR_PKT_ENG) ? "PKTENG" :
		((id == PHY_HOLD_FOR_NOT_ASSOC) ? "NOT-ASSOC" : "")))))))));

	if (set) {
		mboolset(pi->measure_hold, id);
	} else {
		mboolclr(pi->measure_hold, id);
	}
	if (id & PHY_HOLD_FOR_SCAN)
		wlc_phy_scanroam_cache_cal(pi, set);
	return;
}

bool
wlc_phy_ismuted(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	return PHY_MUTED(pi);
}

void
wlc_phy_mute_upd(wlc_phy_t *pih, bool mute, mbool flags)
{
	phy_info_t *pi = (phy_info_t *)pih;

	PHY_TRACE(("wlc_phy_mute_upd: flags 0x%x mute %d\n", flags, mute));

	if (mute) {
		mboolset(pi->measure_hold, PHY_HOLD_FOR_MUTE);
	} else {
		mboolclr(pi->measure_hold, PHY_HOLD_FOR_MUTE);
	}

	/* check if need to schedule a phy cal */
	if (!mute && (flags & PHY_MUTE_FOR_PREISM)) {
		pi->cal_info->last_cal_time = (pi->sh->now > pi->sh->glacial_timer) ?
			(pi->sh->now - pi->sh->glacial_timer) : 0;
	}
	return;
}

void
wlc_phy_clear_tssi(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (ISNPHY(pi) || ISHTPHY(pi)) {
		/* NPHY/HTPHY doesn't use sw or ucode powercontrol */
		return;
	} else if (ISAPHY(pi)) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_A_TSSI_0, NULL_TSSI_W);
		wlapi_bmac_write_shm(pi->sh->physhim, M_A_TSSI_1, NULL_TSSI_W);
	} else {
		wlapi_bmac_write_shm(pi->sh->physhim, M_B_TSSI_0, NULL_TSSI_W);
		wlapi_bmac_write_shm(pi->sh->physhim, M_B_TSSI_1, NULL_TSSI_W);
		wlapi_bmac_write_shm(pi->sh->physhim, M_G_TSSI_0, NULL_TSSI_W);
		wlapi_bmac_write_shm(pi->sh->physhim, M_G_TSSI_1, NULL_TSSI_W);
	}
}

static bool
wlc_phy_cal_txpower_recalc_sw(phy_info_t *pi)
{
	bool ret = TRUE;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* NPHY/HTPHY doesn't ever use SW power control */
	if (ISNPHY(pi) || ISHTPHY(pi))
		return FALSE;

	if (ISSSLPNPHY(pi))
		return FALSE;
	if (ISLCNCOMMONPHY(pi))
		return FALSE;

	if (ISLPPHY(pi)) {
		/* Adjust number of packets for TSSI averaging */
		if (wlc_phy_tpc_isenabled_lpphy(pi)) {
			wlc_phy_tx_pwr_update_npt_lpphy(pi);
		}
		return ret;
	}


	/* No need to do anything if the hw is doing pwrctrl for us */
	if (pi->hwpwrctrl) {

		/* Do nothing if radio pwr is being overridden */
		if (pi->radiopwr_override != RADIOPWR_OVERRIDE_DEF)
			return (ret);

		pi->hwpwr_txcur = wlapi_bmac_read_shm(pi->sh->physhim, M_TXPWR_CUR);
		return (ret);
	} else {
		if (ISABGPHY(pi))
			ret = wlc_phy_cal_txpower_recalc_sw_abgphy(pi);
	}

	return (ret);
}

void
wlc_phy_switch_radio(wlc_phy_t *pih, bool on)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (NORADIO_ENAB(pi->pubpi))
		return;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	{
		uint mc;

		mc = R_REG(pi->sh->osh, &pi->regs->maccontrol);
		if (mc & MCTL_EN_MAC) {
			PHY_ERROR(("wl%d: %s: maccontrol 0x%x has EN_MAC set\n",
			          pi->sh->unit, __FUNCTION__, mc));
		}
	}

#ifdef BCMECICOEX	    /* Allow BT on all channels if radio is off */
	if (BCMECISECICOEX_ENAB_PHY(pi)) {
		if (!on) {
			wlc_phy_update_bt_chanspec(pi, 0);
		}
		else {
			wlc_phy_update_bt_chanspec(pi, pi->radio_chanspec);
		}
	}
#endif

	if (ISNPHY(pi)) {
		wlc_phy_switch_radio_nphy(pi, on);

	} else if (ISHTPHY(pi)) {
		wlc_phy_switch_radio_htphy(pi, on);

	} else if (ISLPPHY(pi)) {
		if (on) {
			if (LPREV_GE(pi->pubpi.phy_rev, 2)) {
				PHY_REG_LIST_START
					PHY_REG_AND_ENTRY(LPPHY_REV2, RFOverride0,
						~(LPPHY_REV2_RFOverride0_rfpll_pu_ovr_MASK 	|
						LPPHY_REV2_RFOverride0_wrssi_pu_ovr_MASK 	|
						LPPHY_REV2_RFOverride0_nrssi_pu_ovr_MASK 	|
						LPPHY_REV2_RFOverride0_internalrfrxpu_ovr_MASK 	|
						LPPHY_REV2_RFOverride0_internalrftxpu_ovr_MASK))
					PHY_REG_AND_ENTRY(LPPHY_REV2, rfoverride2,
						~(LPPHY_REV2_rfoverride2_lna_pu_ovr_MASK |
						LPPHY_REV2_rfoverride2_slna_pu_ovr_MASK))
					PHY_REG_AND_ENTRY(LPPHY_REV2, rfoverride3,
						~LPPHY_REV2_rfoverride3_rfactive_ovr_MASK)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
				PHY_REG_LIST_START
					PHY_REG_AND_ENTRY(LPPHY, RFOverride0,
						~(LPPHY_RFOverride0_rfpll_pu_ovr_MASK 		|
						LPPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
						LPPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
						LPPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
						LPPHY_RFOverride0_internalrftxpu_ovr_MASK))
					PHY_REG_AND_ENTRY(LPPHY, rfoverride2,
						~(LPPHY_rfoverride2_lna1_pu_ovr_MASK |
						LPPHY_rfoverride2_lna2_pu_ovr_MASK))
				PHY_REG_LIST_EXECUTE(pi);
			}
			pi->radio_is_on = TRUE;
		} else {
			if (LPREV_GE(pi->pubpi.phy_rev, 2)) {
				PHY_REG_LIST_START
				PHY_REG_AND_ENTRY(LPPHY_REV2, RFOverrideVal0,
					~(LPPHY_REV2_RFOverrideVal0_rfpll_pu_ovr_val_MASK |
					LPPHY_REV2_RFOverrideVal0_wrssi_pu_ovr_val_MASK 	|
					LPPHY_REV2_RFOverrideVal0_nrssi_pu_ovr_val_MASK 	|
					LPPHY_REV2_RFOverrideVal0_internalrfrxpu_ovr_val_MASK 	|
					LPPHY_REV2_RFOverrideVal0_internalrftxpu_ovr_val_MASK))
				PHY_REG_OR_ENTRY(LPPHY_REV2, RFOverride0,
					LPPHY_REV2_RFOverride0_rfpll_pu_ovr_MASK 		|
					LPPHY_REV2_RFOverride0_wrssi_pu_ovr_MASK 		|
					LPPHY_REV2_RFOverride0_nrssi_pu_ovr_MASK 		|
					LPPHY_REV2_RFOverride0_internalrfrxpu_ovr_MASK 	|
					LPPHY_REV2_RFOverride0_internalrftxpu_ovr_MASK)
				PHY_REG_AND_ENTRY(LPPHY_REV2, rxlnaandgainctrl1ovrval,
					~(LPPHY_REV2_rxlnaandgainctrl1ovrval_lnapuovr_Val_MASK))
				PHY_REG_AND_ENTRY(LPPHY_REV2, rfoverride2val,
					~(LPPHY_REV2_rfoverride2val_slna_pu_ovr_val_MASK))
				PHY_REG_OR_ENTRY(LPPHY_REV2, rfoverride2,
					LPPHY_REV2_rfoverride2_lna_pu_ovr_MASK |
					LPPHY_REV2_rfoverride2_slna_pu_ovr_MASK)
				PHY_REG_AND_ENTRY(LPPHY_REV2, rfoverride3_val,
					~(LPPHY_REV2_rfoverride3_val_rfactive_ovr_val_MASK))
				PHY_REG_OR_ENTRY(LPPHY_REV2, rfoverride3,
					LPPHY_REV2_rfoverride3_rfactive_ovr_MASK)
				PHY_REG_LIST_EXECUTE(pi);
			} else {
				PHY_REG_LIST_START
					PHY_REG_AND_ENTRY(LPPHY, RFOverrideVal0,
						~(LPPHY_RFOverrideVal0_rfpll_pu_ovr_val_MASK	 |
						LPPHY_RFOverrideVal0_wrssi_pu_ovr_val_MASK	 |
						LPPHY_RFOverrideVal0_nrssi_pu_ovr_val_MASK	 |
						LPPHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
						LPPHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK))
					PHY_REG_OR_ENTRY(LPPHY, RFOverride0,
						LPPHY_RFOverride0_rfpll_pu_ovr_MASK 		|
						LPPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
						LPPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
						LPPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
						LPPHY_RFOverride0_internalrftxpu_ovr_MASK)
					PHY_REG_AND_ENTRY(LPPHY, rfoverride2val,
						~(LPPHY_rfoverride2val_lna1_pu_ovr_val_MASK |
						LPPHY_rfoverride2val_lna2_pu_ovr_val_MASK))
					PHY_REG_OR_ENTRY(LPPHY, rfoverride2,
						LPPHY_rfoverride2_lna1_pu_ovr_MASK |
						LPPHY_rfoverride2_lna2_pu_ovr_MASK)
				PHY_REG_LIST_EXECUTE(pi);
			}
			pi->radio_is_on = FALSE;
		}
	} else if (ISSSLPNPHY(pi)) {
		if (on) {
			PHY_REG_LIST_START
				PHY_REG_AND_ENTRY(SSLPNPHY, RFOverride0,
					~(SSLPNPHY_RFOverride0_rfpll_pu_ovr_MASK	|
					SSLPNPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
					SSLPNPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
					SSLPNPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
					SSLPNPHY_RFOverride0_internalrftxpu_ovr_MASK))
				PHY_REG_AND_ENTRY(SSLPNPHY, rfoverride2,
					~(SSLPNPHY_rfoverride2_lna_pu_ovr_MASK |
					SSLPNPHY_rfoverride2_slna_pu_ovr_MASK))
			PHY_REG_LIST_EXECUTE(pi);

			if (BCMECICOEX_ENAB_PHY(pi))
				phy_reg_and(pi, SSLPNPHY_rfoverride3,
					~SSLPNPHY_rfoverride3_rfactive_ovr_MASK);
		} else {
			PHY_REG_LIST_START
				PHY_REG_AND_ENTRY(SSLPNPHY, RFOverrideVal0,
					~(SSLPNPHY_RFOverrideVal0_rfpll_pu_ovr_val_MASK |
					SSLPNPHY_RFOverrideVal0_wrssi_pu_ovr_val_MASK 	|
					SSLPNPHY_RFOverrideVal0_nrssi_pu_ovr_val_MASK 	|
					SSLPNPHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
					SSLPNPHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK))
				PHY_REG_OR_ENTRY(SSLPNPHY, RFOverride0,
					SSLPNPHY_RFOverride0_rfpll_pu_ovr_MASK 		|
					SSLPNPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
					SSLPNPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
					SSLPNPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
					SSLPNPHY_RFOverride0_internalrftxpu_ovr_MASK)

				PHY_REG_AND_ENTRY(SSLPNPHY, rxlnaandgainctrl1ovrval,
					~(SSLPNPHY_rxlnaandgainctrl1ovrval_lnapuovr_Val_MASK))
				PHY_REG_AND_ENTRY(SSLPNPHY, rfoverride2val,
					~(SSLPNPHY_rfoverride2val_slna_pu_ovr_val_MASK))
				PHY_REG_OR_ENTRY(SSLPNPHY, rfoverride2,
					SSLPNPHY_rfoverride2_lna_pu_ovr_MASK |
					SSLPNPHY_rfoverride2_slna_pu_ovr_MASK)
			PHY_REG_LIST_EXECUTE(pi);

			if (BCMECICOEX_ENAB_PHY(pi)) {
				phy_reg_and(pi, SSLPNPHY_rfoverride3_val,
					~(SSLPNPHY_rfoverride3_val_rfactive_ovr_val_MASK));
			} else {
				phy_reg_or(pi, SSLPNPHY_rfoverride3_val,
					SSLPNPHY_rfoverride3_val_rfactive_ovr_val_MASK);
			}
			phy_reg_or(pi, SSLPNPHY_rfoverride3,
				SSLPNPHY_rfoverride3_rfactive_ovr_MASK);
		}
	} else if (ISLCNPHY(pi)) {
		if (on) {
			PHY_REG_LIST_START
				PHY_REG_AND_ENTRY(LCNPHY, RFOverride0,
					~(LCNPHY_RFOverride0_rfpll_pu_ovr_MASK	|
					LCNPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
					LCNPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
					LCNPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
					LCNPHY_RFOverride0_internalrftxpu_ovr_MASK))
				PHY_REG_AND_ENTRY(LCNPHY, rfoverride2,
					~(LCNPHY_rfoverride2_lna_pu_ovr_MASK |
					LCNPHY_rfoverride2_slna_pu_ovr_MASK))
				PHY_REG_AND_ENTRY(LCNPHY, rfoverride3,
					~LCNPHY_rfoverride3_rfactive_ovr_MASK)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			PHY_REG_LIST_START
				PHY_REG_AND_ENTRY(LCNPHY, RFOverrideVal0,
					~(LCNPHY_RFOverrideVal0_rfpll_pu_ovr_val_MASK |
					LCNPHY_RFOverrideVal0_wrssi_pu_ovr_val_MASK 	|
					LCNPHY_RFOverrideVal0_nrssi_pu_ovr_val_MASK 	|
					LCNPHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
					LCNPHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK))
				PHY_REG_OR_ENTRY(LCNPHY, RFOverride0,
					LCNPHY_RFOverride0_rfpll_pu_ovr_MASK 		|
					LCNPHY_RFOverride0_wrssi_pu_ovr_MASK 		|
					LCNPHY_RFOverride0_nrssi_pu_ovr_MASK 		|
					LCNPHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
					LCNPHY_RFOverride0_internalrftxpu_ovr_MASK)
				PHY_REG_AND_ENTRY(LCNPHY, rxlnaandgainctrl1ovrval,
					~(LCNPHY_rxlnaandgainctrl1ovrval_lnapuovr_Val_MASK))
				PHY_REG_AND_ENTRY(LCNPHY, rfoverride2val,
					~(LCNPHY_rfoverride2val_slna_pu_ovr_val_MASK))
				PHY_REG_OR_ENTRY(LCNPHY, rfoverride2,
					LCNPHY_rfoverride2_lna_pu_ovr_MASK |
					LCNPHY_rfoverride2_slna_pu_ovr_MASK)
				PHY_REG_AND_ENTRY(LCNPHY, rfoverride3_val,
					~(LCNPHY_rfoverride3_val_rfactive_ovr_val_MASK))
				PHY_REG_OR_ENTRY(LCNPHY, rfoverride3,
					LCNPHY_rfoverride3_rfactive_ovr_MASK)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else if (ISABGPHY(pi)) {
		wlc_phy_switch_radio_abgphy(pi, on);
	} else if (pi->pi_fptr.switchradio) {
		pi->pi_fptr.switchradio(pi, on);
	}
}

#ifdef BCMECICOEX
static void
wlc_phy_update_bt_chanspec(phy_info_t *pi, chanspec_t chanspec)
{
	si_t *sih = pi->sh->sih;

	if (BCMECISECICOEX_ENAB_PHY(pi) && !SCAN_INPROG_PHY(pi)) {
		 /* Inform BT about the channel change if we are operating in 2Ghz band */
		if (chanspec && CHSPEC_IS2G(chanspec)) {
			NOTIFY_BT_CHL(sih, CHSPEC_CHANNEL(chanspec));
			if (CHSPEC_IS40(chanspec))
				NOTIFY_BT_BW_40(sih);
			else
				NOTIFY_BT_BW_20(sih);
		} else {
			NOTIFY_BT_CHL(sih, 0);
		}
	}
}
#endif /* BCMECICOEX */

/* %%%%%% chanspec, reg/srom limit */
uint16
wlc_phy_bw_state_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return pi->bw;
}

void
wlc_phy_bw_state_set(wlc_phy_t *ppi, uint16 bw)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->bw = bw;
}

void
wlc_phy_chanspec_radio_set(wlc_phy_t *ppi, chanspec_t newch)
{
	uint16 m_cur_channel;
	phy_info_t *pi = (phy_info_t*)ppi;
	pi->radio_chanspec = newch;

	if (NORADIO_ENAB(pi->pubpi)) {
		/* on QT, update ucode channel value here since the real one is not invoked */

		PHY_TRACE(("wl%d: %s: chanspec %x\n", pi->sh->unit, __FUNCTION__, newch));
		ASSERT(!wf_chspec_malformed(newch));

		/* Update ucode channel value */
		m_cur_channel = CHSPEC_CHANNEL(newch);
		if (CHSPEC_IS5G(newch))
			m_cur_channel |= D11_CURCHANNEL_5G;
		if (CHSPEC_IS40(newch))
			m_cur_channel |= D11_CURCHANNEL_40;

		PHY_TRACE(("wl%d: %s: m_cur_channel %x\n", pi->sh->unit, __FUNCTION__,
			m_cur_channel));
		wlapi_bmac_write_shm(pi->sh->physhim, M_CURCHANNEL, m_cur_channel);
	}
}

chanspec_t
wlc_phy_chanspec_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return pi->radio_chanspec;
}

void
wlc_phy_set_filt_war(wlc_phy_t *ppi, bool filt_war)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISHTPHY(pi))
		wlc_phy_set_filt_war_htphy(pi, filt_war);
}

bool
wlc_phy_get_filt_war(wlc_phy_t *ppi)
{
	bool ret = FALSE;
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISHTPHY(pi))
		ret = wlc_phy_get_filt_war_htphy(pi);
	return ret;
}

void
wlc_phy_chanspec_set(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	uint16 m_cur_channel;
	chansetfn_t chanspec_set = NULL;
#if defined(WLMCHAN) || defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, chanspec);
#endif

	PHY_TRACE(("wl%d: %s: chanspec %x\n", pi->sh->unit, __FUNCTION__, chanspec));
	ASSERT(!wf_chspec_malformed(chanspec));

	/* Update ucode channel value */
	m_cur_channel = CHSPEC_CHANNEL(chanspec);
	if (CHSPEC_IS5G(chanspec))
		m_cur_channel |= D11_CURCHANNEL_5G;
	if (CHSPEC_IS40(chanspec))
		m_cur_channel |= D11_CURCHANNEL_40;
	PHY_TRACE(("wl%d: %s: m_cur_channel %x\n", pi->sh->unit, __FUNCTION__, m_cur_channel));
	wlapi_bmac_write_shm(pi->sh->physhim, M_CURCHANNEL, m_cur_channel);

	chanspec_set = pi->pi_fptr.chanset;

#if defined(WLMCHAN) || defined(PHYCAL_CACHING)
	/* If a channel context exists, retrieve the multi-phase info from there, else use
	 * the default one
	 */
	/* A context has to be explicitly created */
	if (ctx)
		pi->cal_info = &ctx->cal_info;
	else
		pi->cal_info = &pi->def_cal_info;
#endif

	ASSERT(pi->cal_info);
	if (chanspec_set)
		(*chanspec_set)(pi, chanspec);

#ifdef WLMCHAN
	/* Switched the context so restart a pending MPHASE cal, else clear the state */
	if (ctx) {
		if (wlc_phy_cal_cache_restore(pi) == BCME_ERROR)
			PHY_CAL((" %s chanspec 0x%x Failed\n", __FUNCTION__, pi->radio_chanspec));

		if (CHIPID(pi->sh->chip) != BCM43237_CHIP_ID) {
			/* Calibrate if now > last_cal_time + glacial */
			if (PHY_PERICAL_MPHASE_PENDING(pi)) {
				PHY_CAL(("%s: Restarting calibration for 0x%x phase %d\n",
				__FUNCTION__, chanspec, pi->cal_info->cal_phase_id));
				/* Delete any existing timer just in case */
				wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
				wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
			} else if ((pi->phy_cal_mode != PHY_PERICAL_DISABLE) &&
				(pi->phy_cal_mode != PHY_PERICAL_MANUAL) &&
			((pi->sh->now - pi->cal_info->last_cal_time) >= pi->sh->glacial_timer))
				wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_PERICAL_WATCHDOG);
		} else {
			if (PHY_PERICAL_MPHASE_PENDING(pi))
				wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
		}
	}
#endif /* WLMCHAN */

	wlc_phy_update_bt_chanspec(pi, chanspec);
}

/* don't use this directly. use wlc_get_band_range whenever possible */
int
wlc_phy_chanspec_freq2bandrange_lpssn(uint freq)
{
	int range = -1;
	if (freq < 2500)
		range = WL_CHAN_FREQ_RANGE_2G;
	else if (freq <= 5320)
		range = WL_CHAN_FREQ_RANGE_5GL;
	else if (freq <= 5700)
		range = WL_CHAN_FREQ_RANGE_5GM;
	else
		range = WL_CHAN_FREQ_RANGE_5GH;

	return range;
}

int
wlc_phy_chanspec_bandrange_get(phy_info_t *pi, chanspec_t chanspec)
{
	int range = -1;
	uint channel = CHSPEC_CHANNEL(chanspec);
	uint freq = wlc_phy_channel2freq(channel);

	if (ISNPHY(pi)) {
		range = wlc_phy_get_chan_freq_range_nphy(pi, channel);
	} else if (ISHTPHY(pi)) {
		range = wlc_phy_get_chan_freq_range_htphy(pi, channel);
	} else if (ISSSLPNPHY(pi) || ISLPPHY(pi) || ISLCNCOMMONPHY(pi)) {
		range = wlc_phy_chanspec_freq2bandrange_lpssn(freq);
	} else if (ISGPHY(pi)) {
		range = WL_CHAN_FREQ_RANGE_2G;
	} else if (ISAPHY(pi)) {
		range = wlc_get_a_band_range(pi);

		switch (range) {
		case A_LOW_CHANS:
			range = WL_CHAN_FREQ_RANGE_5GL;
			break;

		case A_MID_CHANS:
			range = WL_CHAN_FREQ_RANGE_5GM;
			break;

		case A_HIGH_CHANS:
			range = WL_CHAN_FREQ_RANGE_5GH;
			break;

		default:
			break;
		}
	} else
		ASSERT(0);

	return range;
}

void
wlc_phy_chanspec_ch14_widefilter_set(wlc_phy_t *ppi, bool wide_filter)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISABGPHY(pi))
		pi->u.pi_abgphy->channel_14_wide_filter = wide_filter;

}

/*
 * Converts channel number to channel frequency.
 * Returns 0 if the channel is out of range.
 * Also used by some code in wlc_iw.c
 */
int
wlc_phy_channel2freq(uint channel)
{
	uint i;

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++)
		if (chan_info_all[i].chan == channel)
			return (chan_info_all[i].freq);
	return (0);
}
/* Converts channel number into the wlc_phy_chan_info index */
uint
wlc_phy_channel2idx(uint channel)
{
	uint i;

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
		if (chan_info_all[i].chan == channel)
			return i;
	}

	ASSERT(FALSE);
	return (0);
}

/* fill out a chanvec_t with all the supported channels for the band. */
void
wlc_phy_chanspec_band_validch(wlc_phy_t *ppi, uint band, chanvec_t *channels)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint i;
	uint channel;

	ASSERT((band == WLC_BAND_2G) || (band == WLC_BAND_5G));

	bzero(channels, sizeof(chanvec_t));

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
		channel = chan_info_all[i].chan;

		/* disable the high band channels [149-165] for srom ver 1 */
		if ((pi->a_band_high_disable) && (channel >= FIRST_REF5_CHANNUM) &&
		    (channel <= LAST_REF5_CHANNUM))
			continue;

		if (((band == WLC_BAND_2G) && (channel <= CH_MAX_2G_CHANNEL)) ||
		    ((band == WLC_BAND_5G) && (channel > CH_MAX_2G_CHANNEL)))
			setbit(channels->vec, channel);
	}
}


/* returns the first hw supported channel in the band */
chanspec_t
wlc_phy_chanspec_band_firstch(wlc_phy_t *ppi, uint band)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint i;
	uint channel;
	chanspec_t chspec;

	ASSERT((band == WLC_BAND_2G) || (band == WLC_BAND_5G));

	for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
		channel = chan_info_all[i].chan;

		/* If 40MHX b/w then check if there is an upper 20Mhz adjacent channel */
		if ((ISNPHY(pi) || ISHTPHY(pi)|| ISSSLPNPHY(pi)) && IS40MHZ(pi)) {
			uint j;
			/* check if the upper 20Mhz channel exists */
			for (j = 0; j < ARRAYSIZE(chan_info_all); j++) {
				if (chan_info_all[j].chan == channel + CH_10MHZ_APART)
					break;
			}
			/* did we find an adjacent channel */
			if (j == ARRAYSIZE(chan_info_all))
				continue;
			/* Convert channel from 20Mhz num to 40 Mhz number */
			channel = UPPER_20_SB(channel);
			chspec = channel | WL_CHANSPEC_BW_40 | WL_CHANSPEC_CTL_SB_LOWER;
			if (band == WLC_BAND_2G)
				chspec |= WL_CHANSPEC_BAND_2G;
			else
				chspec |= WL_CHANSPEC_BAND_5G;
		}
		else
			chspec = CH20MHZ_CHSPEC(channel);

		/* disable the high band channels [149-165] for srom ver 1 */
		if ((pi->a_band_high_disable) && (channel >= FIRST_REF5_CHANNUM) &&
		    (channel <= LAST_REF5_CHANNUM))
			continue;

		if (((band == WLC_BAND_2G) && (channel <= CH_MAX_2G_CHANNEL)) ||
		    ((band == WLC_BAND_5G) && (channel > CH_MAX_2G_CHANNEL)))
			return chspec;
	}

	/* should never come here */
	ASSERT(0);

	/* to avoid warning */
	return (chanspec_t)INVCHANSPEC;
}

/* %%%%%% txpower */

/* user txpower limit: in qdbm units with override flag */
int
wlc_phy_txpower_get(wlc_phy_t *ppi, uint *qdbm, bool *override)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	ASSERT(qdbm != NULL);
	*qdbm = pi->tx_user_target[0];

	if (pi->openlp_tx_power_on) {
	  if (pi->txpwrnegative)
		*qdbm = (-1 * pi->openlp_tx_power_min) | WL_TXPWR_NEG;
	  else
		*qdbm = pi->openlp_tx_power_min;
	}

	if (override != NULL)
		*override = pi->txpwroverride;
	return (0);
}

void
wlc_phy_txpower_target_set(wlc_phy_t *ppi, txppr_t *txpwr)
{
	bool mac_enabled = FALSE;
	phy_info_t *pi = (phy_info_t*)ppi;
	bool ishtphy = ISHTPHY(pi);
	uint offset;
	uint8 *ptxpwr;

	/* fill the txpwr from the struct to the right offsets */
	/* cck */
	bcopy(txpwr->cck, &pi->tx_user_target[WL_TX_POWER_CCK_FIRST],
	      WL_NUM_RATES_CCK);

	/* ofdm 20MHz */
	bcopy(txpwr->ofdm, &pi->tx_user_target[WL_TX_POWER_OFDM20_FIRST],
	      WL_NUM_RATES_OFDM);
	bcopy(txpwr->ofdm_cdd, &pi->tx_user_target[WL_TX_POWER_OFDM20_CDD_FIRST],
	      WL_NUM_RATES_OFDM);

	/* ofdm 40MHz */
	bcopy(txpwr->ofdm_40, &pi->tx_user_target[WL_TX_POWER_OFDM40_FIRST],
	      WL_NUM_RATES_OFDM);
	bcopy(txpwr->ofdm_40_cdd, &pi->tx_user_target[WL_TX_POWER_OFDM40_CDD_FIRST],
	      WL_NUM_RATES_OFDM);

	/* mcs 20MHz */
	ptxpwr = ishtphy ? txpwr->u20.ht.s1x1 : txpwr->u20.n.siso;
	offset = ishtphy ? WL_TX_POWER_20_S1x1_FIRST : WL_TX_POWER_MCS20_SISO_FIRST;
	bcopy(txpwr->u20.n.siso, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	ptxpwr = ishtphy ? txpwr->u20.ht.s1x2 : txpwr->u20.n.cdd;
	offset = ishtphy ? WL_TX_POWER_20_S1x2_FIRST : WL_TX_POWER_MCS20_CDD_FIRST;
	bcopy(txpwr->u20.n.cdd, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	ptxpwr = ishtphy ? txpwr->ht.u20s1x3 : txpwr->u20.n.stbc;
	offset = ishtphy ? WL_TX_POWER_20_S1x3_FIRST : WL_TX_POWER_MCS20_STBC_FIRST;
	bcopy(txpwr->u20.n.stbc, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	ptxpwr = ishtphy ? txpwr->u20.ht.s2x2 : txpwr->u20.n.sdm;
	offset = ishtphy ? WL_TX_POWER_20_S2x2_FIRST : WL_TX_POWER_MCS20_SDM_FIRST;
	bcopy(txpwr->u20.n.sdm, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	if (ishtphy) {
		bcopy(txpwr->ht.u20s2x3, &pi->tx_user_target[WL_TX_POWER_20_S2x3_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
		bcopy(txpwr->u20.ht.s3x3, &pi->tx_user_target[WL_TX_POWER_20_S3x3_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
	}

	/* mcs 40MHz */
	ptxpwr = ishtphy ? txpwr->u40.ht.s1x1 : txpwr->u40.n.siso;
	offset = ishtphy ? WL_TX_POWER_40_S1x1_FIRST : WL_TX_POWER_MCS40_SISO_FIRST;
	bcopy(ptxpwr, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	ptxpwr = ishtphy ? txpwr->u40.ht.s1x2 : txpwr->u40.n.cdd;
	offset = ishtphy ? WL_TX_POWER_40_S1x2_FIRST : WL_TX_POWER_MCS20_CDD_FIRST;
	bcopy(txpwr->u40.n.cdd, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	ptxpwr = ishtphy ? txpwr->u40.ht.s2x2 : txpwr->u40.n.stbc;
	offset = ishtphy ? WL_TX_POWER_40_S2x2_FIRST : WL_TX_POWER_MCS20_STBC_FIRST;
	bcopy(txpwr->u40.n.stbc, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	ptxpwr = ishtphy ? txpwr->u40.ht.s3x3 : txpwr->u40.n.sdm;
	offset = ishtphy ? WL_TX_POWER_40_S3x3_FIRST : WL_TX_POWER_MCS20_SDM_FIRST;
	bcopy(txpwr->u40.n.sdm, &pi->tx_user_target[offset], WL_NUM_RATES_MCS_1STREAM);
	if (ishtphy) {
		bcopy(txpwr->ht.u40s1x3, &pi->tx_user_target[WL_TX_POWER_40_S1x3_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
		bcopy(txpwr->ht.u40s2x3, &pi->tx_user_target[WL_TX_POWER_40_S2x3_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);

		/* 20 in 40MHz */
		bcopy(txpwr->cck_20ul, &pi->tx_user_target[WL_TX_POWER_20UL_CCK_FIRST],
		      WL_NUM_RATES_CCK);
		bcopy(txpwr->ofdm_20ul, &pi->tx_user_target[WL_TX_POWER_20UL_OFDM_FIRST],
		      WL_NUM_RATES_OFDM);
		bcopy(txpwr->ofdm_20ul_cdd, &pi->tx_user_target[WL_TX_POWER_20UL_OFDM_CDD_FIRST],
		      WL_NUM_RATES_OFDM);
		bcopy(txpwr->ht20ul.s1x1, &pi->tx_user_target[WL_TX_POWER_20UL_S1x1_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
		bcopy(txpwr->ht20ul.s1x2, &pi->tx_user_target[WL_TX_POWER_20UL_S1x2_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
		bcopy(txpwr->ht.ul20s1x3, &pi->tx_user_target[WL_TX_POWER_20UL_S1x3_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
		bcopy(txpwr->ht20ul.s2x2, &pi->tx_user_target[WL_TX_POWER_20UL_S2x2_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
		bcopy(txpwr->ht.ul20s2x3, &pi->tx_user_target[WL_TX_POWER_20UL_S2x3_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
		bcopy(txpwr->ht20ul.s3x3, &pi->tx_user_target[WL_TX_POWER_20UL_S3x3_FIRST],
		      WL_NUM_RATES_MCS_1STREAM);
	}

	pi->tx_user_target[WL_TX_POWER_MCS_32] = txpwr->mcs32;

	if (R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC)
		mac_enabled = TRUE;

	if (mac_enabled)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	wlc_phy_txpower_recalc_target(pi);
	wlc_phy_cal_txpower_recalc_sw(pi);

	if (mac_enabled)
		wlapi_enable_mac(pi->sh->physhim);
}

/* user txpower limit: in qdbm units with override flag */
int
wlc_phy_txpower_set(wlc_phy_t *ppi, uint qdbm, bool override)
{
	uint8 tx_pwr_ctrl_state;
	phy_info_t *pi = (phy_info_t *)ppi;
	int i;

	if (qdbm > 127)
		return 5;

	/* No way for user to set maxpower on individual rates yet.
	 * Same max power is used for all rates
	 */
	for (i = 0; i < TXP_NUM_RATES; i++)
		pi->tx_user_target[i] = (uint8)qdbm;

	/* Restrict external builds to 100% Tx Power */
#if defined(WLTEST) || defined(WLMEDIA_N2DBG)
	pi->txpwroverride = override;
#else
	pi->txpwroverride = FALSE;
#endif


	if (pi->sh->up) {
		if (SCAN_INPROG_PHY(pi)) {
			PHY_TXPWR(("wl%d: Scan in progress, skipping txpower control\n",
				pi->sh->unit));
		} else {
			if (ISSSLPNPHY(pi)) {
				wlc_phy_txpower_recalc_target(pi);
				wlc_phy_cal_txpower_recalc_sw(pi);
			} else {
				bool suspend;

				suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) &
				            MCTL_EN_MAC);

				if (!suspend)
					wlapi_suspend_mac_and_wait(pi->sh->physhim);

				tx_pwr_ctrl_state = pi->nphy_txpwrctrl;

				if (ISNPHY(pi)) {
					/* Switch off power control to save previuos index */
					wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);
				}

				wlc_phy_txpower_recalc_target(pi);
				wlc_phy_cal_txpower_recalc_sw(pi);

				if (ISNPHY(pi)) {
					/* Restore power control back */
					wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
				}

				if (!suspend)
					wlapi_enable_mac(pi->sh->physhim);
			}
		}
	}
	return (0);
}

/* user txpower limit: in qdbm units with override flag */
int
wlc_phy_neg_txpower_set(wlc_phy_t *ppi, uint qdbm)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	initfn_t txpwr_recalc_fn = NULL;

	if (pi->sh->up) {
		if (SCAN_INPROG_PHY(pi)) {
			PHY_TXPWR(("wl%d: Scan in progress, skipping txpower control\n",
				pi->sh->unit));
		} else {
			bool suspend;
			suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);

			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);

			pi->openlp_tx_power_min = -1*qdbm;
			pi->txpwrnegative = 1;
			pi->txpwroverride = 1;

			txpwr_recalc_fn = pi->pi_fptr.txpwrrecalc;
			if (txpwr_recalc_fn)
				(*txpwr_recalc_fn)(pi);

			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);

		}
	}
	return (0);
}


static void
wlc_phy_txpower_sromlimit_nphy_rev16(phy_info_t *pi, uint8 *max_pwr, int txp_rate_idx)
{
	uint channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	if ((channel == 151) && (txp_rate_idx > 67) && (txp_rate_idx < 100)) {
		/* only m7,m15 rates */
		if ((txp_rate_idx - 51) % 8 == 0)
			*max_pwr = *max_pwr - 4;
	}
}
/* get sromlimit per rate for given channel. Routine does not account for ant gain */
void
wlc_phy_txpower_sromlimit(wlc_phy_t *ppi, uint channel, uint8 *min_pwr, uint8 *max_pwr,
	int txp_rate_idx)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint i;

#ifdef WLNOKIA_NVMEM
	{
		uint8 minlimit, maxlimit;
		wlc_phy_nokia_brdtxpwr_limits(pi, channel, txp_rate_idx, &minlimit, &maxlimit);
		if ((minlimit != 0) && (maxlimit != 0)) {
			*min_pwr = minlimit;
			*max_pwr = maxlimit;
			return;
		}
	}
#endif /* WLNOKIA_NVMEM */

	*min_pwr = pi->min_txpower * WLC_TXPWR_DB_FACTOR;

	if (ISNPHY(pi)) {
		if (txp_rate_idx < 0) {
			if (channel <= CH_MAX_2G_CHANNEL)
				txp_rate_idx = TXP_FIRST_CCK;
			else
				txp_rate_idx = TXP_FIRST_OFDM;
		}

		wlc_phy_txpower_sromlimit_get_nphy(pi, channel, max_pwr, (uint8)txp_rate_idx);
		/* Hack for LCNXNPHY  rev 0 */
		/* 2 channels are looking 2 dB off with respect to evm and SM performance */
		/* Dropping the srom powers only for those 2 channels */
		if (pi->pubpi.phy_rev == LCNXN_BASEREV)
			wlc_phy_txpower_sromlimit_nphy_rev16(pi, max_pwr, (uint8)txp_rate_idx);
	} else if (ISHTPHY(pi)) {
		if (txp_rate_idx < 0)
			txp_rate_idx = TXP_FIRST_CCK;
		wlc_phy_txpower_sromlimit_get_htphy(pi, channel, max_pwr, (uint8)txp_rate_idx);

	} else if (ISGPHY(pi) || (channel <= CH_MAX_2G_CHANNEL)) {
		/* until we cook the maxtxpwr value into the channel table,
		 * use the one global B band maxtxpwr
		 */
		if (txp_rate_idx < 0)
			txp_rate_idx = TXP_FIRST_CCK;

		/* legacy phys don't have valid MIMO rate entries
		 * SSLPNPHY does have 8 more entries for MCS 0-7
		 * Cannot re-use nphy logic right away
		 *
		 * SSLPNPHY, LPPHY, ASSERT(txp_rate_idx <= TXP_LAST_OFDM_20_CDD);
		 * other: ASSERT(txp_rate_idx <= TXP_LAST_OFDM);
		 */

		*max_pwr = pi->tx_srom_max_rate_2g[txp_rate_idx];
	} else {
		/* in case we fall out of the channel loop */
		*max_pwr = WLC_TXPWR_MAX;

		if (txp_rate_idx < 0)
			txp_rate_idx = TXP_FIRST_OFDM;
		/* max txpwr is channel dependent */
		for (i = 0; i < ARRAYSIZE(chan_info_all); i++) {
			if (channel == chan_info_all[i].chan) {
				break;
			}
		}
		ASSERT(i < ARRAYSIZE(chan_info_all));

		/* legacy phys don't have valid MIMO rate entries
		 * SSLPNPHY does have 8 more entries for MCS 0-7
		 * Cannot re-use nphy logic right away
		 *
		 *
		 * SSLPNPHY, LPPHY, ASSERT(txp_rate_idx <= TXP_LAST_OFDM_20_CDD);
		 * other: ASSERT(txp_rate_idx <= TXP_LAST_OFDM);
		 */

		if (ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi)) {
			/* SSLPNPHY has different sub-band range limts for the
			*  A-band compared to MIMOPHY
			* (see sslpnphy_get_paparams in sslpnphyprocs.tcl)
			*/
			if ((channel >= FIRST_LOW_5G_CHAN_SSLPNPHY) &&
				(channel <= LAST_LOW_5G_CHAN_SSLPNPHY)) {
		        *max_pwr = pi->tx_srom_max_rate_5g_low[txp_rate_idx];
			}
			if ((channel >= FIRST_MID_5G_CHAN_SSLPNPHY) &&
			    (channel <= LAST_MID_5G_CHAN_SSLPNPHY)) {
			        *max_pwr = pi->tx_srom_max_rate_5g_mid[txp_rate_idx];
			}
			if ((channel >= FIRST_HIGH_5G_CHAN_SSLPNPHY) &&
			    (channel <= LAST_HIGH_5G_CHAN_SSLPNPHY)) {
				*max_pwr = pi->tx_srom_max_rate_5g_hi[txp_rate_idx];
			}
		} else {
			if (pi->hwtxpwr) {
				*max_pwr = pi->hwtxpwr[i];
			} else {
				/* When would we get here?  B only? */
				if ((i >= FIRST_MID_5G_CHAN) && (i <= LAST_MID_5G_CHAN))
					*max_pwr = pi->tx_srom_max_rate_5g_mid[txp_rate_idx];
				if ((i >= FIRST_HIGH_5G_CHAN) && (i <= LAST_HIGH_5G_CHAN))
					*max_pwr = pi->tx_srom_max_rate_5g_hi[txp_rate_idx];
				if ((i >= FIRST_LOW_5G_CHAN) && (i <= LAST_LOW_5G_CHAN))
					*max_pwr = pi->tx_srom_max_rate_5g_low[txp_rate_idx];
			}
		}
	}
	PHY_NONE(("%s: chan %d rate idx %d, sromlimit %d\n", __FUNCTION__, channel, txp_rate_idx,
	          *max_pwr));
}


void
wlc_phy_txpower_sromlimit_max_get(wlc_phy_t *ppi, uint chan, uint8 *max_txpwr, uint8 *min_txpwr)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	uint8 tx_pwr_max = 0;
	uint8 tx_pwr_min = 255;
	uint max_num_rate;
	uint8 maxtxpwr, mintxpwr, pactrl, rate;

	pactrl = 0;
	if (ISGPHY(pi) && (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL))
		pactrl = 3;

	max_num_rate = (ISNPHY(pi) || ISHTPHY(pi)) ? TXP_NUM_RATES :
		((ISLCNPHY(pi) || ISSSLPNPHY(pi) || ISLCN40PHY(pi)) ?
		(TXP_LAST_MCS_20_SISO_SS + 1) : (TXP_LAST_OFDM + 1));

	for (rate = 0; rate < max_num_rate; rate++) {

		wlc_phy_txpower_sromlimit(ppi, chan, &mintxpwr, &maxtxpwr, rate);

		maxtxpwr = (maxtxpwr > pactrl) ? (maxtxpwr - pactrl) : 0;
		/* Subtract 6 (1.5db) to ensure we don't go over
		 * the limit given a noisy power detector
		 */
		maxtxpwr = (maxtxpwr > pi->tx_pwr_backoff) ? (maxtxpwr - pi->tx_pwr_backoff) : 0;

		tx_pwr_max = MAX(tx_pwr_max, maxtxpwr);
		tx_pwr_min = MIN(tx_pwr_min, maxtxpwr);
	}
	*max_txpwr = tx_pwr_max;
	*min_txpwr = tx_pwr_min;
}

void
wlc_phy_txpower_boardlimit_band(wlc_phy_t *ppi, uint bandunit, int32 *max_pwr,
	int32 *min_pwr, uint32 *step_pwr)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	int32 local_max;

	if (ISLPPHY(pi)) {
		if (bandunit == 1)
			*max_pwr = pi->tx_srom_max_2g;
		else {
			local_max = pi->tx_srom_max_5g_low;
			if (local_max <  pi->tx_srom_max_5g_mid)
				local_max =  pi->tx_srom_max_5g_mid;
			if (local_max <  pi->tx_srom_max_5g_hi)
				local_max =  pi->tx_srom_max_5g_hi;
			*max_pwr = local_max;
		}
		*min_pwr = 8;
		*step_pwr = 1;
	}
}

uint8
wlc_phy_txpower_get_target_min(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return pi->tx_power_min;
}

uint8
wlc_phy_txpower_get_target_max(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	return pi->tx_power_max;
}

/* Given a channel and phy type, return TRUE/FALSE to indicate if the rate index is
 * an applicable target.
 *
 * For example, on a 5G channel, the targets for 1,2,5.5,11 are not applicable, and
 * for a GPHY the MCS targets do not matter, and for a 20MHz channel the 40MHz targets
 * do not matter.
 */
static int
wlc_phy_valid_txp_index(phy_info_t *pi, chanspec_t chspec, int rate_idx)
{
	/* if we are on a 5GHz channel, the first 4 rates for DSSS/CCK,
	 * and the range TXP_FIRST_20UL_CCK - TXP_LAST_20UL_CCK for 20in40 DSSS/CCK,
	 * are not applicable.
	 */
	if (CHSPEC_IS5G(pi->radio_chanspec) &&
	    ((rate_idx <= TXP_LAST_CCK) ||
	     (rate_idx >= TXP_FIRST_20UL_CCK && rate_idx <= TXP_LAST_20UL_CCK) ||
	     (rate_idx >= TXP_FIRST_CCK_CDD_S1x2 && rate_idx <= TXP_LAST_CCK_20U_CDD_S1x3)))
		return FALSE;


	/* LCNPHY only has rates for DSSS, OFDM, and MCS 20MHz
	 * Skip anything higher
	 */
	if (ISLCNPHY(pi) &&
	    rate_idx > TXP_LAST_MCS_20_SISO_SS)
		return FALSE;

	/* SSLPNPHY has MCS 0-7 SS, for 20 or 40 */
	if (ISSSLPNPHY(pi) || ISLCN40PHY(pi)) {
		/* if we are on a 20MHz channel, stop after the last SS SISO MCS */
		if (CHSPEC_IS20(chspec)) {
			if (rate_idx >= TXP_FIRST_MCS_20_CDD_SS)
				return FALSE;
		}
		else
		/* Otherwise, on a 40MHz channel, skip the range of 20 CDD, and
		 * anything after the last 40 SISO rate (starts with 40 CDD
		 */
		{
			if ((rate_idx >= TXP_FIRST_MCS_20_CDD_SS &&
			     rate_idx <= TXP_LAST_MCS_20_SS) ||
			    (rate_idx >= TXP_FIRST_MCS_40_SISO_CDD_SS))
			    return FALSE;
		}
	}

	/* nothing past the last OFDM for these older phys */
	if ((ISGPHY(pi) || ISAPHY(pi)) &&
	    rate_idx > TXP_LAST_OFDM) {
		return FALSE;
	}

	/* NPHY has only up to the 2 stream rates which ends at MCS_32 */
	if (ISNPHY(pi) &&
	    rate_idx > TXP_MCS_32) {
		return FALSE;
	}

	/* any MIMO phy uses the same rate numbering for 20/40 rates.
	 * use a generic rule here to classify targets as valid for 20/40
	 * channel bandwidth.
	 */

	/* if we are on a 20MHz channel skip the 40HMz channel targets */
	if (CHSPEC_IS20(chspec)) {
		/* skip the 2 ranges of 40MHz channel targets */
		if ((rate_idx >= TXP_FIRST_OFDM_40_SISO &&
		     rate_idx <= TXP_LAST_20UL_S3x3) ||
		    (rate_idx >= TXP_FIRST_MCS_40_S1x3 &&
		     rate_idx <= TXP_LAST_20UL_S2x3) ||
		    (rate_idx >= TXP_FIRST_CCK_20U_CDD_S1x2 &&
		     rate_idx <= TXP_LAST_CCK_20U_CDD_S1x3)) {
			return FALSE;
		}
	}
	else
	/* Otherwise, on a 40MHz channel, we might just invert the range check above.
	 * However, the NPHY code uses the 20MHz targets as the 20in40 targets. So
	 * for a 40MHz channel, just treat all rate targets as valid.
	 */
	{
		return TRUE;
	}

	return TRUE;
}

/* Recalc target power all phys.  This internal/static function needs to be called whenever
 * the chanspec or TX power values (user target, regulatory limits or SROM/HW limits) change.
 * This happens thorough calls to the PHY public API.
 */
void
wlc_phy_txpower_recalc_target(phy_info_t *pi)
{
	uint8 rate, pactrl;
	uint target_chan;
	uint8 tx_pwr_target[TXP_NUM_RATES];
	uint8 tx_pwr_max = 0;
	uint8 tx_pwr_min = 255;
	uint8 tx_pwr_max_rate_ind = 0;
	uint max_num_rate;
	chanspec_t chspec;
	uint32 band = CHSPEC2WLC_BAND(pi->radio_chanspec);
	initfn_t txpwr_recalc_fn = NULL;
	int32 temp;
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	bool band5g_4319_flag = ((BOARDTYPE(pi->sh->boardtype) == BCM94319SDELNA6L_SSID) ||
	    (BOARDTYPE(pi->sh->boardtype) == BCM94319SDNA_SSID)) &&
		CHSPEC_IS5G(pi->radio_chanspec);

	memset(tx_pwr_target, 0, TXP_NUM_RATES);

	chspec = pi->radio_chanspec;
	if (CHSPEC_CTL_SB(chspec) == WL_CHANSPEC_CTL_SB_NONE)
		target_chan = CHSPEC_CHANNEL(chspec);
	else if (CHSPEC_CTL_SB(chspec) == WL_CHANSPEC_CTL_SB_UPPER)
		target_chan = UPPER_20_SB(CHSPEC_CHANNEL(chspec));
	else
		target_chan = LOWER_20_SB(CHSPEC_CHANNEL(chspec));

	if (ISSSLPNPHY(pi) && SSLPNREV_IS(pi->pubpi.phy_rev, 3)) {
		uint32 offset_mcs, i;

		if (CHSPEC_IS40(pi->radio_chanspec)) {
			offset_mcs = pi->sslpnphy_mcs40_po;
			for (i = TXP_FIRST_MCS_20_SS; i <=  TXP_LAST_MCS_20_SISO_SS;  i++) {
				pi->tx_srom_max_rate_2g[i] = pi->tx_srom_max_2g -
					((offset_mcs & 0xf) * 2);
				offset_mcs >>= 4;
			}
		} else {
			offset_mcs = pi->sslpnphy_mcs20_po;
			for (i = TXP_FIRST_MCS_20_SS; i <= TXP_LAST_MCS_20_SISO_SS;  i++) {
				pi->tx_srom_max_rate_2g[i] = pi->tx_srom_max_2g -
					((offset_mcs & 0xf) * 2);
				offset_mcs >>= 4;
			}
		}
	}
	pactrl = 0;
	if (ISGPHY(pi) && (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL))
		pactrl = 3;

#if WL11N
	max_num_rate = ISNPHY(pi) || ISHTPHY(pi) ? TXP_NUM_RATES :
		(ISLCNPHY(pi) ?
		TXP_LAST_MCS_20_SISO_SS + 1 : ((ISSSLPNPHY(pi) || ISLCN40PHY(pi)) ?
		TXP_LAST_MCS_40_SS + 1 : TXP_LAST_OFDM + 1));
#else
	max_num_rate = ((ISNPHY(pi) || ISHTPHY(pi)) ? (TXP_NUM_RATES) :
		(TXP_LAST_OFDM + 1));
#endif

	if (ISSSLPNPHY(pi) && SSLPNREV_IS(pi->pubpi.phy_rev, 3)) {
		max_num_rate = TXP_LAST_MCS_20_SS + 1;
	}
	/* get the per rate limits based on vbat and temp */
	wlc_phy_upd_env_txpwr_rate_limits(pi, band);

	/* Combine user target, regulatory limit, SROM/HW/board limit and power
	 * percentage to get a tx power target for each rate.
	 */

	for (rate = 0; rate < max_num_rate; rate++) {
		/* skip any rate index that is not appropriate for this channel
		 * Eg. skip DSSS for 5GHz band, 40MHz target powers for 20MHz channel
		 */
		if (!wlc_phy_valid_txp_index(pi, chspec, rate))
			continue;

		/* The user target is the starting point for determining the transmit
		 * power.  If pi->txoverride is true, then use the user target as the
		 * tx power target.
		 */
		tx_pwr_target[rate] = pi->tx_user_target[rate];

		/* this basically moves the user target power from rf port to antenna port */
		if (pi->user_txpwr_at_rfport) {
			tx_pwr_target[rate] += wlc_user_txpwr_antport_to_rfport(pi,
				target_chan, band, rate);
		}

#if defined(WLTEST) || defined(WLMEDIA_N2DBG)
		/* Only allow tx power override for internal or test builds. */
		if (!pi->txpwroverride)
#endif
		{
			uint8 maxtxpwr, mintxpwr;
			/* Get board/hw limit */
			wlc_phy_txpower_sromlimit((wlc_phy_t *)pi, target_chan, &mintxpwr,
				&maxtxpwr, rate);

			if (ISSSLPNPHY(pi) && SSLPNREV_IS(pi->pubpi.phy_rev, 3)) {
				if (CHSPEC_IS20(pi->radio_chanspec) && (target_chan == 14)) {
					tx_pwr_target[rate] = 44;
				}
			}
			/* Choose minimum of provided regulatory and board/hw limits */
			maxtxpwr = MIN(maxtxpwr, pi->txpwr_limit[rate]);

			/* Board specific fix reduction */
			maxtxpwr = (maxtxpwr > pactrl) ? (maxtxpwr - pactrl) : 0;

			/* Subtract 4 (1.0db) for 4313(IPA) as we are doing PA trimming
			 * otherwise subtract 6 (1.5 db)
			 * to ensure we don't go over
			 * the limit given a noisy power detector.  The result
			 * is now a target, not a limit.
			 */
			if (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID))
				maxtxpwr = wlc_lcnphy_modify_max_txpower(pi, maxtxpwr);
			else if (!(ISSSLPNPHY(pi) && SSLPNREV_IS(pi->pubpi.phy_rev, 4)))
				maxtxpwr = (maxtxpwr > pi->tx_pwr_backoff) ?
					(maxtxpwr - pi->tx_pwr_backoff) : 0;

			/* Choose least of user and now combined regulatory/hw
			 * targets.
			 */
			tx_pwr_target[rate] = MIN(maxtxpwr, tx_pwr_target[rate]);

			if (ISSSLPNPHY(pi))
				wlc_sslpnphy_txpwr_target_adj(pi, tx_pwr_target, rate);

			/* Apply power output percentage */
			if (pi->txpwr_percent <= 100)
				tx_pwr_target[rate] =
					(tx_pwr_target[rate] * pi->txpwr_percent) / 100;
			/* Enforce min power and save result as power target. */
			tx_pwr_target[rate] = MAX(tx_pwr_target[rate], mintxpwr);
		}
		/* check the per rate override based on environment vbat/temp */
		tx_pwr_target[rate] = MIN(tx_pwr_target[rate], pi->txpwr_env_limit[rate]);
		/* Identify the rate amongst the TXP_NUM_RATES rates with the max target power
		 * on this channel. If multiple rates have the max target power, the
		 * lowest rate index among them is stored.
		 */
		if (tx_pwr_target[rate] > tx_pwr_max)
			tx_pwr_max_rate_ind = rate;

		tx_pwr_max = MAX(tx_pwr_max, tx_pwr_target[rate]);
		tx_pwr_min = MIN(tx_pwr_min, tx_pwr_target[rate]);
	}

	/* Now calculate the tx_power_offset and update the hardware... */
	bzero(pi->tx_power_offset, sizeof(pi->tx_power_offset));
	pi->tx_power_max = tx_pwr_max;
	pi->tx_power_min = tx_pwr_min;
	pi->openlp_tx_power_min = tx_pwr_min;
	pi->txpwrnegative = 0;
	/* just to make sure */
	ASSERT(pi->tx_power_min != 0);

	pi->tx_power_max_rate_ind = tx_pwr_max_rate_ind;
	PHY_NONE(("wl%d: %s: channel %d min %d max %d\n", pi->sh->unit, __FUNCTION__,
	          target_chan, pi->tx_power_min, pi->tx_power_max));

	PHY_NONE(("wl%d: %s: channel %d rate - targets - offsets:\n", pi->sh->unit,
	          __FUNCTION__, target_chan));

	/* determinate the txpower offset by either of the following 2 methods:
	 * txpower_offset = txpower_max - txpower_target OR
	 * txpower_offset = txpower_target - txpower_min
	 */
	for (rate = 0; rate < max_num_rate; rate++) {
		/* For swpwrctrl, the offset is OFDM w.r.t. CCK.
		 * For hwpwrctl, other way around
		 */
		pi->tx_power_target[rate] = tx_pwr_target[rate];

		if (!pi->hwpwrctrl || ISNPHY(pi) || ISHTPHY(pi) || band5g_4319_flag) {
			pi->tx_power_offset[rate] = pi->tx_power_max - pi->tx_power_target[rate];
		} else {
			pi->tx_power_offset[rate] = pi->tx_power_target[rate] - pi->tx_power_min;
		}
#ifdef BCMDBG
		if (!wlc_phy_valid_txp_index(pi, chspec, rate))
			continue;
		PHY_NONE(("    %d:    %d    %d\n", rate, pi->tx_power_target[rate],
		          pi->tx_power_offset[rate]));
#endif
	}
	if (ISLCNPHY(pi) && (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID))
		wlc_lcnphy_modify_rate_power_offsets(pi);

	if (ISLCNPHY(pi) || ISLCN40PHY(pi)) {
		/* CCK Pwr Index Convergence Correction */
		for (rate = TXP_FIRST_CCK; rate <= TXP_LAST_CCK; rate++) {
			temp = (int32)(-pi->tx_power_offset[rate]);
			temp -= pi_lcn->cckPwrIdxCorr;
			pi->tx_power_offset[rate] = (uint8)(-temp);
		}
	}

	txpwr_recalc_fn = pi->pi_fptr.txpwrrecalc;
	if (txpwr_recalc_fn)
		(*txpwr_recalc_fn)(pi);
}

#ifdef BCMDBG
void
wlc_phy_txpower_limits_dump(txppr_t *txpwr, bool ishtphy)
{
	int i;
	char fraction[4][4] = {"   ", ".25", ".5 ", ".75"};
	uint8 *ptxpwr;

	printf("CCK                  ");
	for (i = 0; i < WL_NUM_RATES_CCK; i++) {
		printf(" %2d%s", txpwr->cck[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[txpwr->cck[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz OFDM SISO      ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++) {
		printf(" %2d%s", txpwr->ofdm[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[txpwr->ofdm[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz OFDM CDD       ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++) {
		printf(" %2d%s", txpwr->ofdm_cdd[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ofdm_cdd[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "20MHz 1 Nsts to 1 Tx " : "20MHz MCS 0-7 SISO   ");
	ptxpwr = ishtphy ? txpwr->u20.ht.s1x1 : txpwr->u20.n.siso;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "20MHz 1 Nsts to 2 Tx " : "20MHz MCS 0-7 CDD    ");
	ptxpwr = ishtphy ? txpwr->u20.ht.s1x2 : txpwr->u20.n.cdd;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "20MHz 1 Nsts to 3 Tx " : "20MHz MCS 0-7 STBC   ");
	ptxpwr = ishtphy ? txpwr->ht.u20s1x3 : txpwr->u20.n.stbc;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "20MHz 2 Nsts to 2 Tx " : "20MHz MCS 8-15 SDM   ");
	ptxpwr = ishtphy ? txpwr->u20.ht.s2x2 : txpwr->u20.n.sdm;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	if (ishtphy) {
		printf("20MHz 2 Nsts to 3 Tx ");
		ptxpwr = txpwr->ht.u20s2x3;
		for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
			printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
			       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("20MHz 3 Nsts to 3 Tx ");
		ptxpwr = txpwr->u20.ht.s3x3;
		for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
			printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
			       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");
	}

	printf("40MHz OFDM SISO      ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++) {
		printf(" %2d%s", txpwr->ofdm_40[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ofdm_40[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("40MHz OFDM CDD       ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++) {
		printf(" %2d%s", txpwr->ofdm_40_cdd[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ofdm_40_cdd[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 1 Nsts to 1 Tx " : "40MHz MCS 0-7 SISO   ");
	ptxpwr = ishtphy ? txpwr->u40.ht.s1x1 : txpwr->u40.n.siso;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 1 Nsts to 2 Tx " : "40MHz MCS 0-7 CDD    ");
	ptxpwr = ishtphy ? txpwr->u40.ht.s1x2 : txpwr->u40.n.cdd;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 1 Nsts to 3 Tx " : "40MHz MCS 0-7 STBC   ");
	ptxpwr = ishtphy ? txpwr->ht.u40s1x3 : txpwr->u40.n.stbc;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("%s", ishtphy ? "40MHz 2 Nsts to 2 Tx " : "40MHz MCS8-15 SDM    ");
	ptxpwr = ishtphy ? txpwr->u40.ht.s2x2 : txpwr->u40.n.sdm;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	if (ishtphy) {
		printf("40MHz 2 Nsts to 3 Tx ");
		ptxpwr = txpwr->ht.u40s2x3;
		for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
			printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
			       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("40MHz 3 Nsts to 3 Tx ");
		ptxpwr = txpwr->u40.ht.s3x3;
		for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
			printf(" %2d%s", ptxpwr[i]/ WLC_TXPWR_DB_FACTOR,
			       fraction[ptxpwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");
	}

	printf("MCS32                 %2d%s\n", txpwr->mcs32 / WLC_TXPWR_DB_FACTOR,
	       fraction[txpwr->mcs32 % WLC_TXPWR_DB_FACTOR]);

	if (!ishtphy)
		return;

	printf("20MHz UL CCK         ");
	for (i = 0; i < WL_NUM_RATES_CCK; i++) {
		printf(" %2d%s", txpwr->cck_20ul[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->cck_20ul[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz UL OFDM SISO   ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++) {
		printf(" %2d%s", txpwr->ofdm_20ul[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ofdm_20ul[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz UL OFDM CDD    ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++) {
		printf(" %2d%s", txpwr->ofdm_20ul_cdd[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ofdm_20ul_cdd[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 1 Nsts to 1 Tx  ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", txpwr->ht20ul.s1x1[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ht20ul.s1x1[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 1 Nsts to 2 Tx  ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", txpwr->ht20ul.s1x2[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ht20ul.s1x2[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 1 Nsts to 3 Tx  ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", txpwr->ht.ul20s1x3[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ht.ul20s1x3[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 2 Nsts to 2 Tx  ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", txpwr->ht20ul.s2x2[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ht20ul.s2x2[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 2 Nsts to 3 Tx  ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", txpwr->ht.ul20s2x3[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ht.ul20s2x3[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20UL 3 Nsts to 3 Tx  ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++) {
		printf(" %2d%s", txpwr->ht20ul.s3x3[i]/ WLC_TXPWR_DB_FACTOR,
		       fraction[txpwr->ht20ul.s3x3[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");
}
#endif /* BCMDBG */

/* Translates the regulatory power limit array into an array of length TXP_NUM_RATES,
 * which can match the board limit array obtained using the SROM. Moreover, since the NPHY chips
 * currently do not distinguish between Legacy OFDM and MCS0-7, the SISO and CDD regulatory power
 * limits of these rates need to be combined carefully.
 * This internal/static function needs to be called whenever the chanspec or regulatory TX power
 * limits change.
 */
void
wlc_phy_txpower_reg_limit_calc(phy_info_t *pi, txppr_t *txpwr, chanspec_t chanspec)
{
	uint8 tmp_txpwr_limit[2*WL_NUM_RATES_OFDM];
	uint8 *txpwr_ptr1 = NULL, *txpwr_ptr2 = NULL;
	int rate_start_index = 0, rate1, rate2, k;

	/* Obtain the regulatory limits for CCK rates */
	for (rate1 = WL_TX_POWER_CCK_FIRST, rate2 = 0; rate2 < WL_NUM_RATES_CCK;
	     rate1++, rate2++)
		pi->txpwr_limit[rate1] = txpwr->cck[rate2];

	/* Obtain the regulatory limits for Legacy OFDM SISO rates */
	for (rate1 = WL_TX_POWER_OFDM20_FIRST, rate2 = 0; rate2 < WL_NUM_RATES_OFDM;
	     rate1++, rate2++)
		pi->txpwr_limit[rate1] = txpwr->ofdm[rate2];

	/* Obtain the regulatory limits for Legacy OFDM and HT-OFDM 11n rates in NPHY chips */
	if (ISNPHY(pi)) {
		/* If NPHY is enabled, then use min of OFDM and MCS_20_SISO values as the regulatory
		 * limit for SISO Legacy OFDM and MCS0-7 rates. Similarly, for 40 MHz SIS0 Legacy
		 * OFDM  and MCS0-7 rates as well as for 20 MHz and 40 MHz CDD Legacy OFDM and
		 * MCS0-7 rates. This is because the current hardware implementation uses common
		 * powers for the 8 Legacy ofdm and 8 mcs0-7 rates, i.e. they share the same power
		 * table. The power table is populated based on the constellation, coding rate, and
		 * transmission mode (SISO/CDD/STBC/SDM). Therefore, care must be taken to match the
		 * constellation and coding rates of the Legacy OFDM and MCS0-7 rates since the 8
		 * Legacy OFDM rates and the 8 MCS0-7 rates do not have a 1-1 correspondence in
		 * these parameters.
		 */

		/* Regulatory limits for Legacy OFDM rates 20 and 40 MHz, SISO and CDD. The
		 * regulatory limits for the corresponding MCS0-7 20 and 40 MHz, SISO and
		 * CDD rates should also be mapped into Legacy OFDM limits and the minimum
		 * of the two limits should be taken for each rate.
		 */
		for (k = 0; k < 4; k++) {
			switch (k) {
			case 0:
				/* 20 MHz Legacy OFDM SISO */
				txpwr_ptr1 = txpwr->u20.n.siso;
				txpwr_ptr2 = txpwr->ofdm;
				rate_start_index = WL_TX_POWER_OFDM20_FIRST;
				break;
			case 1:
				/* 20 MHz Legacy OFDM CDD */
				txpwr_ptr1 = txpwr->u20.n.cdd;
				txpwr_ptr2 = txpwr->ofdm_cdd;
				rate_start_index = WL_TX_POWER_OFDM20_CDD_FIRST;
				break;
			case 2:
				/* 40 MHz Legacy OFDM SISO */
				txpwr_ptr1 = txpwr->u40.n.siso;
				txpwr_ptr2 = txpwr->ofdm_40;
				rate_start_index = WL_TX_POWER_OFDM40_FIRST;
				break;
			case 3:
				/* 40 MHz Legacy OFDM CDD */
				txpwr_ptr1 = txpwr->u40.n.cdd;
				txpwr_ptr2 = txpwr->ofdm_40_cdd;
				rate_start_index = WL_TX_POWER_OFDM40_CDD_FIRST;
				break;
			}

			for (rate2 = 0; rate2 < WL_NUM_RATES_OFDM; rate2++) {
				tmp_txpwr_limit[rate2] = 0;
				tmp_txpwr_limit[WL_NUM_RATES_OFDM+rate2] = txpwr_ptr1[rate2];
			}
			wlc_phy_mcs_to_ofdm_powers_nphy(tmp_txpwr_limit, 0,
				WL_NUM_RATES_OFDM-1, WL_NUM_RATES_OFDM);
			for (rate1 = rate_start_index, rate2 = 0;
			     rate2 < WL_NUM_RATES_OFDM; rate1++, rate2++)
				pi->txpwr_limit[rate1] =
					MIN(txpwr_ptr2[rate2], tmp_txpwr_limit[rate2]);
		}

		/* Regulatory limits for MCS0-7 rates 20 and 40 MHz, SISO and CDD. The
		 * regulatory limits for the corresponding Legacy OFDM 20 and 40 MHz, SISO and
		 * CDD rates should also be mapped into MCS0-7 limits and the minimum
		 * of the two limits should be taken for each rate.
		 */
		for (k = 0; k < 4; k++) {
			switch (k) {
			case 0:
				/* 20 MHz MCS 0-7 SISO */
				txpwr_ptr1 = txpwr->ofdm;
				txpwr_ptr2 = txpwr->u20.n.siso;
				rate_start_index = WL_TX_POWER_MCS20_SISO_FIRST;
				break;
			case 1:
				/* 20 MHz MCS 0-7 CDD */
				txpwr_ptr1 = txpwr->ofdm_cdd;
				txpwr_ptr2 = txpwr->u20.n.cdd;
				rate_start_index = WL_TX_POWER_MCS20_CDD_FIRST;
				break;
			case 2:
				/* 40 MHz MCS 0-7 SISO */
				txpwr_ptr1 = txpwr->ofdm_40;
				txpwr_ptr2 = txpwr->u40.n.siso;
				rate_start_index = WL_TX_POWER_MCS40_SISO_FIRST;
				break;
			case 3:
				/* 40 MHz MCS 0-7 CDD */
				txpwr_ptr1 = txpwr->ofdm_40_cdd;
				txpwr_ptr2 = txpwr->u40.n.cdd;
				rate_start_index = WL_TX_POWER_MCS40_CDD_FIRST;
				break;
			}
			for (rate2 = 0; rate2 < WL_NUM_RATES_OFDM; rate2++) {
				tmp_txpwr_limit[rate2] = 0;
				tmp_txpwr_limit[WL_NUM_RATES_OFDM+rate2] = txpwr_ptr1[rate2];
			}
			wlc_phy_ofdm_to_mcs_powers_nphy(tmp_txpwr_limit, 0,
			                                WL_NUM_RATES_OFDM-1, WL_NUM_RATES_OFDM);
			for (rate1 = rate_start_index, rate2 = 0;
			     rate2 < WL_NUM_RATES_MCS_1STREAM; rate1++, rate2++)
				pi->txpwr_limit[rate1] = MIN(txpwr_ptr2[rate2],
				                             tmp_txpwr_limit[rate2]);
		}

		/* Regulatory limits for MCS0-7 rates 20 and 40 MHz, STBC */
		for (k = 0; k < 2; k++) {
			switch (k) {
			case 0:
				/* 20 MHz MCS 0-7 STBC */
				rate_start_index = WL_TX_POWER_MCS20_STBC_FIRST;
				txpwr_ptr1 = txpwr->u20.n.stbc;
				break;
			case 1:
				/* 40 MHz MCS 0-7 STBC */
				rate_start_index = WL_TX_POWER_MCS40_STBC_FIRST;
				txpwr_ptr1 = txpwr->u40.n.stbc;
				break;
			}
			for (rate1 = rate_start_index, rate2 = 0;
			     rate2 < WL_NUM_RATES_MCS_1STREAM; rate1++, rate2++)
				pi->txpwr_limit[rate1] = txpwr_ptr1[rate2];
		}

		/* Regulatory limits for MCS8-15 rates 20 and 40 MHz, SDM */
		for (k = 0; k < 2; k++) {
			switch (k) {
			case 0:
				/* 20 MHz MCS 8-15 (MIMO) tx power limits */
				rate_start_index = WL_TX_POWER_MCS20_SDM_FIRST;
				txpwr_ptr1 = txpwr->u20.n.sdm;
				break;
			case 1:
				/* 40 MHz MCS 8-15 (MIMO) tx power limits */
				rate_start_index = WL_TX_POWER_MCS40_SDM_FIRST;
				txpwr_ptr1 = txpwr->u40.n.sdm;
				break;
			}
			for (rate1 = rate_start_index, rate2 = 0;
			     rate2 < WL_NUM_RATES_MCS_1STREAM; rate1++, rate2++)
				pi->txpwr_limit[rate1] = txpwr_ptr1[rate2];
		}

		/* MCS32 tx power limits */
		pi->txpwr_limit[WL_TX_POWER_MCS_32] = txpwr->mcs32;

		/* Set the 40 MHz MCS0 and MCS32 regulatory power limits to the minimum of the
		 * two limits
		 */
		pi->txpwr_limit[WL_TX_POWER_MCS40_CDD_FIRST] =
		        MIN(pi->txpwr_limit[WL_TX_POWER_MCS40_CDD_FIRST],
		            pi->txpwr_limit[WL_TX_POWER_MCS_32]);
		pi->txpwr_limit[WL_TX_POWER_MCS_32] = pi->txpwr_limit[WL_TX_POWER_MCS40_CDD_FIRST];
	} else if (ISHTPHY(pi)) {
		/* wlc_phy_txpower_reg_limit_calc_htphy(pi, txpwr); */
		bcopy((uint8 *)txpwr, (uint8 *)pi->txpwr_limit, WL_TX_POWER_RATES);
	}
}

void
wlc_phy_txpwr_percent_set(wlc_phy_t *ppi, uint8 txpwr_percent)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->txpwr_percent = txpwr_percent;
}

void
wlc_phy_machwcap_set(wlc_phy_t *ppi, uint32 machwcap)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->sh->machwcap = machwcap;
}


void
wlc_phy_runbist_config(wlc_phy_t *ppi, bool start_end)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	uint16 rxc;
	rxc = 0;

	if (start_end == ON) {
		if (!ISNPHY(pi) || ISHTPHY(pi))
			return;

		/* Keep pktproc in reset during bist run */
		if (NREV_IS(pi->pubpi.phy_rev, 3) || NREV_IS(pi->pubpi.phy_rev, 4)) {
			rxc = phy_reg_read(pi, NPHY_RxControl);
			phy_reg_write(pi, NPHY_RxControl,
			      NPHY_RxControl_dbgpktprocReset_MASK | rxc);
		}
	} else {
		if (NREV_IS(pi->pubpi.phy_rev, 3) || NREV_IS(pi->pubpi.phy_rev, 4)) {
			phy_reg_write(pi, NPHY_RxControl, rxc);
		}

		wlc_phy_por_inform(ppi);
	}
}

/* Set tx power limits */
/* BMAC_NOTE: this only needs a chanspec so that it can choose which 20/40 limits
 * to save in phy state. Would not need this if we ether saved all the limits and
 * applied them only when we were on the correct channel, or we restricted this fn
 * to be called only when on the correct channel.
 */
void
wlc_phy_txpower_limit_set(wlc_phy_t *ppi, txppr_t *txpwr, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	wlc_phy_txpower_reg_limit_calc(pi, txpwr, chanspec);

	if ((ISSSLPNPHY(pi) && !(SSLPNREV_IS(pi->pubpi.phy_rev, 3))) || ISLCNPHY(pi) ||
		ISLCN40PHY(pi)) {
		int i, j;
		for (i = TXP_FIRST_OFDM_20_CDD, j = 0; j < WL_NUM_RATES_MCS_1STREAM; i++, j++) {
			if (txpwr->u20.n.siso[j])
				pi->txpwr_limit[i] = txpwr->u20.n.siso[j];
			else
				pi->txpwr_limit[i] = txpwr->ofdm[j];
		}
	}

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* wlc_phy_print_txpwr_limits_structure(txpwr); */
	wlc_phy_txpower_recalc_target(pi);
	wlc_phy_cal_txpower_recalc_sw(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_phy_ofdm_rateset_war(wlc_phy_t *pih, bool war)
{
	phy_info_t *pi = (phy_info_t*)pih;

	pi->ofdm_rateset_war = war;
}

void
wlc_phy_bf_preempt_enable(wlc_phy_t *pih, bool bf_preempt)
{
	phy_info_t *pi = (phy_info_t*)pih;
	if (ISABGPHY(pi))
		pi->u.pi_abgphy->bf_preempt_4306 = bf_preempt;
}

/*
 * For SW based power control, target power represents CCK and ucode reduced OFDM by the opo.
 * For hw based power control, we lower the target power so it represents OFDM and
 * ucode boosts CCK by the opo.
 */
void
wlc_phy_txpower_update_shm(phy_info_t *pi)
{
	int j;
	if (ISNPHY(pi) || ISHTPHY(pi) || ISSSLPNPHY(pi)) {
		PHY_ERROR(("wlc_phy_update_txpwr_shmem is for legacy phy\n"));
		ASSERT(0);
		return;
	}

	if (!pi->sh->clk)
		return;

	if (pi->hwpwrctrl) {
		uint16 offset;

		/* Rate based ucode power control */
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_MAX, 63);
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_N, 1 << NUM_TSSI_FRAMES);

		/* Need to lower the target power to OFDM level, then add boost for CCK. */
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_TARGET,
			pi->tx_power_min << NUM_TSSI_FRAMES);

		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_CUR, pi->hwpwr_txcur);

		/* CCK */
		if (ISGPHY(pi)) {
			const uint8 ucode_cck_rates[] =
			        { /*	   1,    2,  5.5,   11 Mbps */
					0x02, 0x04, 0x0b, 0x16
				};
			PHY_TXPWR(("M_RATE_TABLE_B:\n"));
			for (j = TXP_FIRST_CCK; j <= TXP_LAST_CCK; j++) {
				offset = wlapi_bmac_rate_shm_offset(pi->sh->physhim,
					ucode_cck_rates[j]);

				wlapi_bmac_write_shm(pi->sh->physhim, offset + 6,
				                   pi->tx_power_offset[j]);
				PHY_TXPWR(("[0x%x] = 0x%x\n", offset + 6, pi->tx_power_offset[j]));

				wlapi_bmac_write_shm(pi->sh->physhim, offset + 14,
				                   -(pi->tx_power_offset[j] / 2));
				PHY_TXPWR(("[0x%x] = 0x%x\n", offset + 14,
				          -(pi->tx_power_offset[j] / 2)));
			}

			if (pi->ofdm_rateset_war) {
				offset = wlapi_bmac_rate_shm_offset(pi->sh->physhim,
					ucode_cck_rates[0]);
				wlapi_bmac_write_shm(pi->sh->physhim, offset + 6, 0);
				PHY_TXPWR(("[0x%x] = 0x%x (ofdm rateset war)\n", offset + 6, 0));
				wlapi_bmac_write_shm(pi->sh->physhim, offset + 14, 0);
				PHY_TXPWR(("[0x%x] = 0x%x (ofdm rateset war)\n", offset + 14, 0));
			}
		}

		/* OFDM */
		PHY_TXPWR(("M_RATE_TABLE_A:\n"));
		for (j = TXP_FIRST_OFDM; j <= TXP_LAST_OFDM; j++) {
			const uint8 ucode_ofdm_rates[] =
			        { /*	   6,   9,    12,   18,   24,   36,   48,   54 Mbps */
					0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c
				};
			offset = wlapi_bmac_rate_shm_offset(pi->sh->physhim,
				ucode_ofdm_rates[j - TXP_FIRST_OFDM]);
			wlapi_bmac_write_shm(pi->sh->physhim, offset + 6, pi->tx_power_offset[j]);
			PHY_TXPWR(("[0x%x] = 0x%x\n", offset + 6, pi->tx_power_offset[j]));
			wlapi_bmac_write_shm(pi->sh->physhim, offset + 14,
				-(pi->tx_power_offset[j] / 2));
			PHY_TXPWR(("[0x%x] = 0x%x\n", offset + 14, -(pi->tx_power_offset[j] / 2)));
		}

		wlapi_bmac_mhf(pi->sh->physhim, MHF2, MHF2_PPR_HWPWRCTL, MHF2_PPR_HWPWRCTL,
		               WLC_BAND_ALL);
	} else {
		int i;

		/* ucode has 2 db granularity when doing sw pwrctrl,
		 * so round up to next 8 .25 units = 2 db.
		 *   HW based power control has .25 db granularity
		 */
		/* Populate only OFDM power offsets, since ucode can only offset OFDM packets */
		for (i = TXP_FIRST_OFDM; i <= TXP_LAST_OFDM; i++)
			pi->tx_power_offset[i] = (uint8)ROUNDUP(pi->tx_power_offset[i], 8);
		wlapi_bmac_write_shm(pi->sh->physhim, M_OFDM_OFFSET,
			(uint16)((pi->tx_power_offset[TXP_FIRST_OFDM] + 7) >> 3));
	}
}


bool
wlc_phy_txpower_hw_ctrl_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISNPHY(pi)) {
		return pi->nphy_txpwrctrl;
	} else if (ISHTPHY(pi)) {
		return pi->txpwrctrl;
	} else {
		return pi->hwpwrctrl;
	}
}

void
wlc_phy_txpower_hw_ctrl_set(wlc_phy_t *ppi, bool hwpwrctrl)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	bool cur_hwpwrctrl = pi->hwpwrctrl;
	bool suspend;

	/* validate if hardware power control is capable */
	if (!pi->hwpwrctrl_capable) {
		PHY_ERROR(("wl%d: hwpwrctrl not capable\n", pi->sh->unit));
		return;
	}

	PHY_INFORM(("wl%d: setting the hwpwrctrl to %d\n", pi->sh->unit, hwpwrctrl));
	pi->hwpwrctrl = hwpwrctrl;
	pi->nphy_txpwrctrl = hwpwrctrl;
	pi->txpwrctrl = hwpwrctrl;

	/* if power control mode is changed, propagate it */
	if (ISNPHY(pi)) {
		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* turn on/off power control */
		wlc_phy_txpwrctrl_enable_nphy(pi, pi->nphy_txpwrctrl);
		if (pi->nphy_txpwrctrl == PHY_TPC_HW_OFF) {
			wlc_phy_txpwr_fixpower_nphy(pi);
		} else {
			/* restore the starting txpwr index */
			phy_reg_mod(pi, NPHY_TxPwrCtrlCmd, NPHY_TxPwrCtrlCmd_pwrIndex_init_MASK,
			            pi->saved_txpwr_idx);
		}

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	} else if (ISHTPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* turn on/off power control */
		wlc_phy_txpwrctrl_enable_htphy(pi, pi->txpwrctrl);

		wlapi_enable_mac(pi->sh->physhim);
	} else if (hwpwrctrl != cur_hwpwrctrl) {
		/* save, change and restore tx power control */

		if (ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi)) {
			return;

		} else if (ISABGPHY(pi)) {
			wlc_phy_txpower_hw_ctrl_set_abgphy(pi);
		}
	}
}

void
wlc_phy_txpower_ipa_upd(phy_info_t *pi)
{
	/* this should be expanded to work with all new PHY capable of iPA */
	if (NREV_GE(pi->pubpi.phy_rev, 3)) {
		pi->ipa2g_on = (pi->srom_fem2g.extpagain == 2);
		pi->ipa5g_on = (pi->srom_fem5g.extpagain == 2);
	} else {
		pi->ipa2g_on = FALSE;
		pi->ipa5g_on = FALSE;
	}
	PHY_INFORM(("wlc_phy_txpower_ipa_upd: ipa 2g %d, 5g %d\n", pi->ipa2g_on, pi->ipa5g_on));
}


void
wlc_phy_txpower_get_current(wlc_phy_t *ppi, tx_power_t *power, uint channel)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint rate, num_rates;
	uint8 min_pwr, max_pwr, core;

	if (ISNPHY(pi)) {
		power->rf_cores = 2;
		power->flags |= (WL_TX_POWER_F_MIMO);
		if (pi->nphy_txpwrctrl == PHY_TPC_HW_ON)
			power->flags |= (WL_TX_POWER_F_ENABLED | WL_TX_POWER_F_HW);
	} else if (ISHTPHY(pi)) {
		power->rf_cores = pi->pubpi.phy_corenum;
		power->flags |= (WL_TX_POWER_F_MIMO);
		if (pi->txpwrctrl == PHY_TPC_HW_ON)
			power->flags |= (WL_TX_POWER_F_ENABLED | WL_TX_POWER_F_HW);
	} else if (ISSSLPNPHY(pi)) {
		power->rf_cores = 1;
		power->flags |= (WL_TX_POWER_F_SISO);
		if (pi->radiopwr_override == RADIOPWR_OVERRIDE_DEF)
			power->flags |= WL_TX_POWER_F_ENABLED;
		if (pi->hwpwrctrl)
			power->flags |= WL_TX_POWER_F_HW;
	} else if (ISLCNCOMMONPHY(pi)) {
		power->rf_cores = 1;
		power->flags |= (WL_TX_POWER_F_SISO);
		if (pi->radiopwr_override == RADIOPWR_OVERRIDE_DEF)
			power->flags |= WL_TX_POWER_F_ENABLED;
		if (pi->hwpwrctrl)
			power->flags |= WL_TX_POWER_F_HW;
	} else {
		power->rf_cores = 1;
		if (pi->radiopwr_override == RADIOPWR_OVERRIDE_DEF)
			power->flags |= WL_TX_POWER_F_ENABLED;
		if (pi->hwpwrctrl)
			power->flags |= WL_TX_POWER_F_HW;
	}

	num_rates = (ISNPHY(pi) || ISHTPHY(pi)) ? TXP_NUM_RATES :
		(ISLCNPHY(pi) ? TXP_LAST_MCS_20_SISO_SS + 1 : ((ISSSLPNPHY(pi) || ISLCN40PHY(pi)) ?
		TXP_LAST_MCS_40_SISO_SS + 1 : TXP_LAST_OFDM + 1));

	for (rate = 0; rate < num_rates; rate++) {
		power->user_limit[rate] = pi->tx_user_target[rate];
		wlc_phy_txpower_sromlimit(ppi, channel, &min_pwr, &max_pwr, rate);
		power->board_limit[rate] = (uint8)max_pwr;
		power->target[rate] = pi->tx_power_target[rate];
	}

	/* fill the est_Pout, max target power, and rate index corresponding to the max
	 * target power fields
	 */
	if (ISNPHY(pi)) {
		uint32 est_pout;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		est_pout = wlc_phy_txpower_est_power_nphy(pi);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);

		/* Store the adjusted  estimated powers */
		power->est_Pout_act[0] = (est_pout >> 8) & 0xff;
		power->est_Pout_act[1] = est_pout & 0xff;

		/* Store the actual estimated powers without adjustment */
		power->est_Pout[0] = est_pout >> 24;
		power->est_Pout[1] = (est_pout >> 16) & 0xff;

		/* if invalid, return 0 */
		if (power->est_Pout[0] == 0x80)
			power->est_Pout[0] = 0;
		if (power->est_Pout[1] == 0x80)
			power->est_Pout[1] = 0;

		/* if invalid, return 0 */
		if (power->est_Pout_act[0] == 0x80)
			power->est_Pout_act[0] = 0;
		if (power->est_Pout_act[1] == 0x80)
			power->est_Pout_act[1] = 0;

		power->est_Pout_cck = 0;

		/* Store the maximum target power among all rates */
		power->tx_power_max[0] = pi->tx_power_max;
		power->tx_power_max[1] = pi->tx_power_max;

		/* Store the index of the rate with the maximum target power among all rates */
		power->tx_power_max_rate_ind[0] = pi->tx_power_max_rate_ind;
		power->tx_power_max_rate_ind[1] = pi->tx_power_max_rate_ind;
	} else if (ISHTPHY(pi)) {
		/* Get power estimates */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		wlc_phy_txpwr_est_pwr_htphy(pi, power->est_Pout, power->est_Pout_act);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);

		power->est_Pout_cck = 0;

		/* Store the maximum target power among all rates */
		FOREACH_CORE(pi, core) {
			power->tx_power_max[core] = pi->tx_power_max;
			power->tx_power_max_rate_ind[core] = pi->tx_power_max_rate_ind;
		}
	} else if (!pi->hwpwrctrl) {
		/* If sw power control, grab the stashed value */
		if (ISGPHY(pi)) {
			power->est_Pout_cck = pi->txpwr_est_Pout;
			power->est_Pout[0] = pi->txpwr_est_Pout -
				pi->tx_power_offset[TXP_FIRST_OFDM];
		}
		else if (ISAPHY(pi))
			power->est_Pout[0] = pi->txpwr_est_Pout;
	} else if (pi->sh->up) {
		/* If hw (ucode) based, read the hw based estimate in realtime */
		wlc_phyreg_enter(ppi);
		if (ISGPHY(pi)) {
			power->est_Pout_cck = phy_reg_read(pi, BPHY_TX_EST_PWR) & 0xff;
			power->est_Pout[0] =
			        phy_reg_read(pi, GPHY_TO_APHY_OFF + APHY_RSSI_FILT_A2) & 0xff;
		} else if (ISLPPHY(pi)) {
			if (wlc_phy_tpc_isenabled_lpphy(pi))
				power->flags |= (WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
			else
				power->flags &= ~(WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);

			wlc_phy_get_tssi_lpphy(pi, (int8*)&power->est_Pout[0],
				(int8*)&power->est_Pout_cck);
		} else if (ISSSLPNPHY(pi)) {
			/* Store the maximum target power among all rates */
			power->tx_power_max[0] = pi->tx_power_max;
			power->tx_power_max[1] = pi->tx_power_max;

			/* Store the index of the rate with the maximum target power among all
			 * rates.
			 */
			power->tx_power_max_rate_ind[0] = pi->tx_power_max_rate_ind;
			power->tx_power_max_rate_ind[1] = pi->tx_power_max_rate_ind;

			if (wlc_phy_tpc_isenabled_sslpnphy(pi))
				power->flags |= (WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
			else
				power->flags &= ~(WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);

			wlc_sslpnphy_get_tssi(pi, (int8*)&power->est_Pout[0],
				(int8*)&power->est_Pout_cck);
		} else if (ISLCNPHY(pi)) {
			/* Store the maximum target power among all rates */
			power->tx_power_max[0] = pi->tx_power_max;
			power->tx_power_max[1] = pi->tx_power_max;

			/* Store the index of the rate with the maximum target power among all
			 * rates.
			 */
			power->tx_power_max_rate_ind[0] = pi->tx_power_max_rate_ind;
			power->tx_power_max_rate_ind[1] = pi->tx_power_max_rate_ind;

			if (wlc_phy_tpc_isenabled_lcnphy(pi))
				power->flags |= (WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
			else
				power->flags &= ~(WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);

			wlc_lcnphy_get_tssi(pi, (int8*)&power->est_Pout[0],
				(int8*)&power->est_Pout_cck);
		} else if (ISLCN40PHY(pi)) {
			/* Store the maximum target power among all rates */
			power->tx_power_max[0] = pi->tx_power_max;
			power->tx_power_max[1] = pi->tx_power_max;

			/* Store the index of the rate with the maximum target power among all
			 * rates.
			 */
			power->tx_power_max_rate_ind[0] = pi->tx_power_max_rate_ind;
			power->tx_power_max_rate_ind[1] = pi->tx_power_max_rate_ind;
			if (pi->pi_fptr.ishwtxpwrctrl && pi->pi_fptr.ishwtxpwrctrl(pi))
				power->flags |= (WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
			else
				power->flags &= ~(WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);

			wlc_lcn40phy_get_tssi(pi, (int8*)&power->est_Pout[0],
				(int8*)&power->est_Pout_cck);
		} else if (ISAPHY(pi))
			power->est_Pout[0] = phy_reg_read(pi, APHY_RSSI_FILT_A2) & 0xff;
		wlc_phyreg_exit(ppi);
	}
}

#if defined(BCMDBG) || defined(WLTEST)
/* Return the current instantaneous est. power
 * For swpwr ctrl it's based on current TSSI value (as opposed to average)
 * Mainly used by mfg.
 */
static void
wlc_phy_txpower_get_instant(phy_info_t *pi, void *pwr)
{
	tx_inst_power_t *power = (tx_inst_power_t *)pwr;
	/* If sw power control, grab the instant value based on current TSSI Only
	 * If hw based, read the hw based estimate in realtime
	 */
	if (ISLPPHY(pi)) {
		if (!pi->hwpwrctrl)
			return;

		wlc_phy_get_tssi_lpphy(pi, (int8*)&power->txpwr_est_Pout_gofdm,
			(int8*)&power->txpwr_est_Pout[0]);
		power->txpwr_est_Pout[1] = power->txpwr_est_Pout_gofdm;

	} else if (ISSSLPNPHY(pi)) {
		if (!pi->hwpwrctrl)
			return;

		wlc_sslpnphy_get_tssi(pi, (int8*)&power->txpwr_est_Pout_gofdm,
			(int8*)&power->txpwr_est_Pout[0]);
		power->txpwr_est_Pout[1] = power->txpwr_est_Pout_gofdm;

	} else if (ISLCNPHY(pi)) {
		if (!pi->hwpwrctrl)
			return;

		wlc_lcnphy_get_tssi(pi, (int8*)&power->txpwr_est_Pout_gofdm,
			(int8*)&power->txpwr_est_Pout[0]);
		power->txpwr_est_Pout[1] = power->txpwr_est_Pout_gofdm;

	} else if (ISLCN40PHY(pi)) {
		if (!pi->hwpwrctrl)
			return;

		wlc_lcn40phy_get_tssi(pi, (int8*)&power->txpwr_est_Pout_gofdm,
			(int8*)&power->txpwr_est_Pout[0]);
		power->txpwr_est_Pout[1] = power->txpwr_est_Pout_gofdm;

	} else if (ISABGPHY(pi)) {
		wlc_phy_txpower_get_instant_abgphy(pi, pwr);
	}

}
#endif 

#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_test_init(phy_info_t *pi, int channel, bool txpkt)
{
	if (channel > MAXCHANNEL)
		return BCME_OUTOFRANGECHAN;

	wlc_phy_chanspec_set((wlc_phy_t*)pi, CH20MHZ_CHSPEC(channel));

	/* stop any requests from the stack and prevent subsequent thread */
	pi->phytest_on = TRUE;

	if (ISLPPHY(pi)) {

		wlc_phy_init_test_lpphy(pi);

	} else if (ISSSLPNPHY(pi)) {

		wlc_phy_init_test_sslpnphy(pi);

	} else if (ISLCNPHY(pi)) {

		wlc_phy_init_test_lcnphy(pi);

	} else if (ISLCN40PHY(pi)) {

		wlc_phy_init_test_lcn40phy(pi);

	} else if (ISGPHY(pi)) {
		/* Disable rx */
		pi->tr_loss_ctl = phy_reg_read(pi, BPHY_TR_LOSS_CTL);
		pi->rf_override = phy_reg_read(pi, BPHY_RF_OVERRIDE);
		phy_reg_or(pi, BPHY_RF_OVERRIDE, (uint16)0x8000);
		wlc_synth_pu_war(pi, channel);
	}

	return 0;
}

int
wlc_phy_test_stop(phy_info_t *pi)
{
	if (pi->phytest_on == FALSE)
		return 0;

	/* stop phytest mode */
	pi->phytest_on = FALSE;

	/* For NPHY, phytest register needs to be accessed only via phy and not directly */
	if (ISNPHY(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_reg_and(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), 0xfc00);
			if (NREV_GE(pi->pubpi.phy_rev, 3))
				/* BPHY_DDFS_ENABLE is removed in mimophy rev 3 */
				phy_reg_write(pi, NPHY_bphytestcontrol, 0x0);
			else
				phy_reg_write(pi, NPHY_TO_BPHY_OFF + BPHY_DDFS_ENABLE, 0);
		}
	} else if (ISHTPHY(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_LIST_START
				PHY_REG_AND_RAW_ENTRY(HTPHY_TO_BPHY_OFF + BPHY_TEST, 0xfc00)
				PHY_REG_WRITE_RAW_ENTRY(HTPHY_bphytestcontrol, 0x0)
			PHY_REG_LIST_EXECUTE(pi);
		}
	} else if (ISLPPHY(pi)) {
		/* Just ignore the phytest reg, it's not currently used for LPPHY */
	} else if (ISSSLPNPHY(pi)) {
		/* Do nothing */
	} else if (ISLCNPHY(pi)) {
		PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
	} else {
		AND_REG(pi->sh->osh, &pi->regs->phytest, 0xfc00);
		phy_reg_write(pi, BPHY_DDFS_ENABLE, 0);
	}

	/* Restore these registers */
	if (ISGPHY(pi)) {
		phy_reg_write(pi, BPHY_TR_LOSS_CTL, pi->tr_loss_ctl);
		phy_reg_write(pi, BPHY_RF_OVERRIDE, pi->rf_override);
	}

	return 0;
}

/*
 * Rate is number of 500 Kb units.
 */
static int
wlc_phy_test_evm(phy_info_t *pi, int channel, uint rate, int txpwr)
{
	d11regs_t *regs = pi->regs;
	uint16 reg = 0;
	int bcmerror = 0;
	phy_info_lpphy_t *pi_lp = pi->u.pi_lpphy;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);


	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		if (ISNPHY(pi))
			phy_reg_write(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST),
			              pi->evm_phytest);
		else if (ISLPPHY(pi)) {
			phy_reg_write(pi, LPPHY_bphyTest, pi->evm_phytest);
			phy_reg_write(pi, LPPHY_ClkEnCtrl, 0);
			wlc_phy_tx_pu_lpphy(pi, 0);
		} else if (ISSSLPNPHY(pi)) {
			phy_reg_write(pi, SSLPNPHY_bphyTest, pi->evm_phytest);
			phy_reg_write(pi, SSLPNPHY_ClkEnCtrl, 0);
			wlc_sslpnphy_tx_pu(pi, 0);
		} else if (ISLCNPHY(pi)) {
			phy_reg_write(pi, LCNPHY_bphyTest, pi->evm_phytest);
			phy_reg_write(pi, LCNPHY_ClkEnCtrl, 0);
			wlc_lcnphy_tx_pu(pi, 0);
		} else if (ISLCN40PHY(pi)) {
			phy_reg_write(pi, LCN40PHY_bphyTest, pi->evm_phytest);
			phy_reg_write(pi, LCN40PHY_ClkEnCtrl, 0);
			wlc_lcn40phy_tx_pu(pi, 0);
		} else 	if (ISHTPHY(pi))
			wlc_phy_bphy_testpattern_htphy(pi, HTPHY_TESTPATTERN_BPHY_EVM, reg, FALSE);
		 else
			W_REG(pi->sh->osh, &regs->phytest, pi->evm_phytest);

		pi->evm_phytest = 0;

		/* Restore 15.6 Mhz nominal fc Tx LPF */
		if (pi->u.pi_abgphy->sbtml_gm && (RADIOID(pi->pubpi.radioid) == BCM2050_ID)) {
			or_radio_reg(pi, RADIO_2050_TX_CTL0, 0x4);
		}

		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL) {
			W_REG(pi->sh->osh, &pi->regs->psm_gpio_out, pi->evm_o);
			W_REG(pi->sh->osh, &pi->regs->psm_gpio_oe, pi->evm_oe);
			OSL_DELAY(1000);
		}
		return 0;
	}

	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_PACTRL) {
		PHY_INFORM(("wl%d: %s: PACTRL boardflag set, clearing gpio 0x%04x\n",
			pi->sh->unit, __FUNCTION__, BOARD_GPIO_PACTRL));
		/* Store initial values */
		pi->evm_o = R_REG(pi->sh->osh, &pi->regs->psm_gpio_out);
		pi->evm_oe = R_REG(pi->sh->osh, &pi->regs->psm_gpio_oe);
		AND_REG(pi->sh->osh, &regs->psm_gpio_out, ~BOARD_GPIO_PACTRL);
		OR_REG(pi->sh->osh, &regs->psm_gpio_oe, BOARD_GPIO_PACTRL);
		OSL_DELAY(1000);
	}

	if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
		return bcmerror;

	if (pi->u.pi_abgphy->sbtml_gm && (RADIOID(pi->pubpi.radioid) == BCM2050_ID)) {
		/* Disable 15.6 Mhz nominal fc Tx LPF */
		and_radio_reg(pi, RADIO_2050_TX_CTL0, ~4);
	}

	reg = TST_TXTEST_RATE_2MBPS;
	switch (rate) {
	case 2:
		reg = TST_TXTEST_RATE_1MBPS;
		break;
	case 4:
		reg = TST_TXTEST_RATE_2MBPS;
		break;
	case 11:
		reg = TST_TXTEST_RATE_5_5MBPS;
		break;
	case 22:
		reg = TST_TXTEST_RATE_11MBPS;
		break;
	}
	reg = (reg << TST_TXTEST_RATE_SHIFT) & TST_TXTEST_RATE;

	PHY_INFORM(("wlc_evm: rate = %d, reg = 0x%x\n", rate, reg));

	/* Save original contents */
	if (pi->evm_phytest == 0 && !ISHTPHY(pi)) {
		if (ISNPHY(pi))
			pi->evm_phytest = phy_reg_read(pi,
			                               (NPHY_TO_BPHY_OFF + BPHY_TEST));
		else if (ISLPPHY(pi)) {
			pi->evm_phytest = phy_reg_read(pi, LPPHY_bphyTest);
			phy_reg_write(pi, LPPHY_ClkEnCtrl, 0xffff);
		} else if (ISSSLPNPHY(pi)) {
			pi->evm_phytest = phy_reg_read(pi, SSLPNPHY_bphyTest);
			phy_reg_write(pi, SSLPNPHY_ClkEnCtrl, 0xffff);
		} else if (ISLCNPHY(pi)) {
			pi->evm_phytest = phy_reg_read(pi, LCNPHY_bphyTest);
			phy_reg_write(pi, LCNPHY_ClkEnCtrl, 0xffff);
		} else if (ISLCN40PHY(pi)) {
			pi->evm_phytest = phy_reg_read(pi, LCN40PHY_bphyTest);
			phy_reg_write(pi, LCN40PHY_ClkEnCtrl, 0xffff);
		} else
			pi->evm_phytest = R_REG(pi->sh->osh, &regs->phytest);
	}

	/* Set EVM test mode */
	if (ISHTPHY(pi)) {
		wlc_phy_bphy_testpattern_htphy(pi, NPHY_TESTPATTERN_BPHY_EVM, reg, TRUE);
	} else if (ISNPHY(pi)) {
		phy_reg_and(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST),
		            ~(TST_TXTEST_ENABLE|TST_TXTEST_RATE|TST_TXTEST_PHASE));
		phy_reg_or(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST), TST_TXTEST_ENABLE | reg);
	} else if (ISLPPHY(pi)) {
		if (LPREV_GE(pi->pubpi.phy_rev, 2))
			wlc_phy_tx_dig_filt_cck_setup_lpphy(pi, TRUE);
		if (pi_lp->lpphy_use_cck_dig_loft_coeffs) {
#if defined(PHYCAL_CACHING)
			ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
			ASSERT(ctx);
			wlc_phy_set_tx_locc_lpphy(pi, ctx->u.lpphy_cache.didq_cck);
#else
			wlc_phy_set_tx_locc_lpphy(pi, pi_lp->lpphy_cal_results.didq_cck);
#endif
		}
		wlc_phy_tx_pu_lpphy(pi, 1);
		phy_reg_or(pi, LPPHY_bphyTest, 0x128);
	} else if (ISSSLPNPHY(pi)) {
		wlc_sslpnphy_tx_pu(pi, 1);
		phy_reg_or(pi, SSLPNPHY_bphyTest, 0x128);
	} else if (ISLCNPHY(pi)) {
		wlc_lcnphy_tx_pu(pi, 1);
		phy_reg_or(pi, LCNPHY_bphyTest, 0x128);
	} else if ISLCN40PHY(pi) {
		wlc_lcn40phy_tx_pu(pi, 1);
		phy_reg_or(pi, LCN40PHY_bphyTest, 0x128);
	} else {
		AND_REG(pi->sh->osh, &regs->phytest,
		        ~(TST_TXTEST_ENABLE|TST_TXTEST_RATE|TST_TXTEST_PHASE));
		OR_REG(pi->sh->osh, &regs->phytest, TST_TXTEST_ENABLE | reg);
	}
	return 0;
}

static int
wlc_phy_test_carrier_suppress(phy_info_t *pi, int channel)
{
	d11regs_t *regs = pi->regs;
	int bcmerror = 0;
	phy_info_lpphy_t *pi_lp = pi->u.pi_lpphy;
	phy_info_sslpnphy_t *pi_sslpn = pi->u.pi_sslpnphy;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);


	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		if (ISNPHY(pi)) {
			phy_reg_write(pi, (NPHY_TO_BPHY_OFF + BPHY_TEST),
			              pi->car_sup_phytest);
		} else if (ISLPPHY(pi)) {
			/* Disable carrier suppression */
			phy_reg_write(pi, LPPHY_ClkEnCtrl, 0);
			phy_reg_and(pi, LPPHY_bphyTest, pi->car_sup_phytest);

			wlc_phy_tx_pu_lpphy(pi, 0);
		} else if (ISSSLPNPHY(pi)) {
			/* Disable carrier suppression */
			phy_reg_write(pi, SSLPNPHY_ClkEnCtrl, 0);
			phy_reg_and(pi, SSLPNPHY_bphyTest, pi->car_sup_phytest);
			phy_reg_and(pi, SSLPNPHY_sslpnCalibClkEnCtrl, 0xff7f);
			phy_reg_write(pi, SSLPNPHY_BphyControl3, pi_sslpn->sslpnphy_bphyctrl);
			wlc_sslpnphy_tx_pu(pi, 0);
		} else if (ISLCNPHY(pi)) {
			/* release the gpio controls from cc */
			wlc_lcnphy_epa_switch(pi, 0);
			/* Disable carrier suppression */
			wlc_lcnphy_crsuprs(pi, channel);
		} else if (ISLCN40PHY(pi)) {
			/* Disable carrier suppression */
			wlc_lcn40phy_crsuprs(pi, channel);
		} else 	if (ISHTPHY(pi)) {
			wlc_phy_bphy_testpattern_htphy(pi, HTPHY_TESTPATTERN_BPHY_RFCS, 0, FALSE);
		} else
			W_REG(pi->sh->osh, &regs->phytest, pi->car_sup_phytest);

		pi->car_sup_phytest = 0;
		return 0;
	}

	if ((bcmerror = wlc_phy_test_init(pi, channel, TRUE)))
		return bcmerror;

	/* Save original contents */
	if (pi->car_sup_phytest == 0 && !ISHTPHY(pi)) {
	        if (ISNPHY(pi)) {
			pi->car_sup_phytest = phy_reg_read(pi,
			                                   (NPHY_TO_BPHY_OFF + BPHY_TEST));
		} else if (ISLPPHY(pi)) {
			pi->car_sup_phytest = phy_reg_read(pi, LPPHY_bphyTest);
		} else if (ISSSLPNPHY(pi)) {
			pi->car_sup_phytest = phy_reg_read(pi, SSLPNPHY_bphyTest);
			pi_sslpn->sslpnphy_bphyctrl = phy_reg_read(pi, SSLPNPHY_BphyControl3);
		} else if (ISLCNPHY(pi)) {
			pi->car_sup_phytest = phy_reg_read(pi, LCNPHY_bphyTest);
		} else
			pi->car_sup_phytest = R_REG(pi->sh->osh, &regs->phytest);
	}

	/* set carrier suppression test mode */
	if (ISHTPHY(pi)) {
		wlc_phy_bphy_testpattern_htphy(pi, HTPHY_TESTPATTERN_BPHY_RFCS, 0, TRUE);
	} else if (ISNPHY(pi)) {
		PHY_REG_LIST_START
			PHY_REG_AND_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_TEST, 0xfc00)
			PHY_REG_OR_RAW_ENTRY(NPHY_TO_BPHY_OFF + BPHY_TEST, 0x0228)
		PHY_REG_LIST_EXECUTE(pi);
	} else if (ISLPPHY(pi)) {
		if (LPREV_GE(pi->pubpi.phy_rev, 2))
			wlc_phy_tx_dig_filt_cck_setup_lpphy(pi, TRUE);
		if (pi_lp->lpphy_use_cck_dig_loft_coeffs) {
#if defined(PHYCAL_CACHING)
			ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
			ASSERT(ctx);
			wlc_phy_set_tx_locc_lpphy(pi, ctx->u.lpphy_cache.didq_cck);
#else
			wlc_phy_set_tx_locc_lpphy(pi, pi_lp->lpphy_cal_results.didq_cck);
#endif
		}
		wlc_phy_tx_pu_lpphy(pi, 1);

		/* Enable carrier suppression */
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(LPPHY, ClkEnCtrl, 0xffff)
			PHY_REG_OR_ENTRY(LPPHY, bphyTest, 0x228)
		PHY_REG_LIST_EXECUTE(pi);

	} else if (ISSSLPNPHY(pi)) {
		wlc_sslpnphy_tx_pu(pi, 1);

		/* Enable carrier suppression */
		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(SSLPNPHY, ClkEnCtrl, 0xffff)
			PHY_REG_OR_ENTRY(SSLPNPHY, bphyTest, 0x228)
		PHY_REG_LIST_EXECUTE(pi);

	} else if (ISLCNPHY(pi)) {
		/* get the gpio controls to cc */
		wlc_lcnphy_epa_switch(pi, 1);
		wlc_lcnphy_crsuprs(pi, channel);
	} else {
		AND_REG(pi->sh->osh, &regs->phytest, 0xfc00);
		OR_REG(pi->sh->osh, &regs->phytest, 0x0228);
	}

	return 0;
}

static int
wlc_phy_test_freq_accuracy(phy_info_t *pi, int channel)
{
	int bcmerror = 0;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means this is a request to end the test */
	if (channel == 0) {
		/* Restore original values */
		if (ISNPHY(pi)) {
			if ((bcmerror = wlc_phy_freq_accuracy_nphy(pi, channel)) != BCME_OK)
				return bcmerror;
		} else if (ISHTPHY(pi)) {
			if ((bcmerror = wlc_phy_freq_accuracy_htphy(pi, channel)) != BCME_OK)
				return bcmerror;
		} else if (ISLPPHY(pi)) {
			wlc_phy_stop_tx_tone_lpphy(pi);
			wlc_phy_clear_deaf_lpphy(pi, (bool)1);
		} else if (ISSSLPNPHY(pi)) {
			wlc_sslpnphy_stop_tx_tone(pi);
			wlc_sslpnphy_set_tx_pwr_ctrl(pi, SSLPNPHY_TX_PWR_CTRL_HW);
		} else if (ISLCNPHY(pi)) {
			wlc_lcnphy_stop_tx_tone(pi);
			if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
				wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_TEMPBASED);
			else
				wlc_lcnphy_set_tx_pwr_ctrl(pi, LCNPHY_TX_PWR_CTRL_HW);
			wlc_lcnphy_epa_switch(pi, 0);
		} else if (ISLCN40PHY(pi) && LCN40REV_LE(pi->pubpi.phy_rev, 3)) {
			/* For lcn40 rev 0-3, restore the 24dB scaler */
			PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_enable, 1);
			PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_coeff, 64);
			wlc_lcn40phy_set_bbmult(pi, 64);
			wlc_lcn40phy_stop_tx_tone(pi);
			pi->pi_fptr.settxpwrctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);
		} else if (ISABGPHY(pi)) {
			wlc_phy_test_freq_accuracy_prep_abgphy(pi);
		}

		return 0;
	}

	if ((bcmerror = wlc_phy_test_init(pi, channel, FALSE)))
		return bcmerror;

	if (ISNPHY(pi)) {
		if ((bcmerror = wlc_phy_freq_accuracy_nphy(pi, channel)) != BCME_OK)
			return bcmerror;
	} else if (ISHTPHY(pi)) {
		if ((bcmerror = wlc_phy_freq_accuracy_htphy(pi, channel)) != BCME_OK)
			return bcmerror;
	} else if (ISLPPHY(pi)) {
		wlc_phy_set_deaf_lpphy(pi, (bool)1);
		wlc_phy_start_tx_tone_lpphy(pi, 0, LPREV_GE(pi->pubpi.phy_rev, 2) ? 28: 100);
	} else if (ISSSLPNPHY(pi)) {
		wlc_sslpnphy_start_tx_tone(pi, 0, 112, 0);
		wlc_sslpnphy_set_tx_pwr_by_index(pi, (int)20);
	} else if (ISLCNPHY(pi)) {
		/* get the gpio controls to cc */
		wlc_lcnphy_epa_switch(pi, 1);
		wlc_lcnphy_start_tx_tone(pi, 0, 112, 0);
		wlc_lcnphy_set_tx_pwr_by_index(pi, (int)94);
	} else if (ISLCN40PHY(pi)) {
		/* For lcn40 rev 0-3, need to scale up tx tone by 24 dB */
		PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_enable, 1);
		PHY_REG_MOD(pi, LCN40PHY, bbmult0, bbmult0_coeff, 255);
		wlc_lcn40phy_set_bbmult(pi, 255);
		wlc_lcn40phy_start_tx_tone(pi, 0, 112, 0);
		pi->pi_fptr.settxpwrbyindex(pi, (int)94);
	} else if (ISABGPHY(pi)) {
		wlc_phy_test_freq_accuracy_run_abgphy(pi);
	}

	return 0;
}

#endif 

void
wlc_phy_antsel_type_set(wlc_phy_t *ppi, uint8 antsel_type)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->antsel_type = antsel_type;

	/* initialize flag to init HW Rx antsel if the board supports it */
	if ((pi->antsel_type == ANTSEL_2x3_HWRX) || (pi->antsel_type == ANTSEL_1x2_HWRX))
		pi->nphy_enable_hw_antsel = TRUE;
	else
		pi->nphy_enable_hw_antsel = FALSE;
}

bool
wlc_phy_test_ison(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return (pi->phytest_on);
}

bool
wlc_phy_ant_rxdiv_get(wlc_phy_t *ppi, uint8 *pval)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	*pval = pi->sh->rx_antdiv;
	return TRUE;
}

void
wlc_phy_ant_rxdiv_set(wlc_phy_t *ppi, uint8 val)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	bool suspend;

	pi->sh->rx_antdiv = val;

	if (ISHTPHY(pi))
		return;	/* no need to set phy reg for htphy */

	/* update ucode flag for non-4322(phy has antdiv by default) */
	if (!(ISNPHY(pi) && D11REV_IS(pi->sh->corerev, 16))) {
		if (val > ANT_RX_DIV_FORCE_1)
			wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, MHF1_ANTDIV,
				WLC_BAND_ALL);
		else
			wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_ANTDIV, 0, WLC_BAND_ALL);
	}

	if (ISNPHY(pi))
		return;	/* no need to set phy reg for nphy */

	if (!pi->sh->clk)
		return;

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (ISLPPHY(pi)) {
		if (val > ANT_RX_DIV_FORCE_1) {
			phy_reg_mod(pi, LPPHY_crsgainCtrl,
				LPPHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x01 << LPPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LPPHY_crsgainCtrl,
				LPPHY_crsgainCtrl_DefaultAntenna_MASK,
				((ANT_RX_DIV_START_1 == val) ? 1 : 0) <<
				LPPHY_crsgainCtrl_DefaultAntenna_SHIFT);
		} else {
			phy_reg_mod(pi, LPPHY_crsgainCtrl,
				LPPHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x00 << LPPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LPPHY_crsgainCtrl,
				LPPHY_crsgainCtrl_DefaultAntenna_MASK,
				(uint16)val << LPPHY_crsgainCtrl_DefaultAntenna_SHIFT);
		}
	} else if (ISSSLPNPHY(pi)) {
		if (val > ANT_RX_DIV_FORCE_1) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				phy_reg_mod(pi, SSLPNPHY_Rev2_crsgainCtrl_40,
				SSLPNPHY_Rev2_crsgainCtrl_40_DiversityChkEnable_MASK,
				0x01 << SSLPNPHY_Rev2_crsgainCtrl_40_DiversityChkEnable_SHIFT);

				phy_reg_mod(pi, SSLPNPHY_Rev2_crsgainCtrl_40,
					SSLPNPHY_Rev2_crsgainCtrl_40_DefaultAntenna_MASK,
					((ANT_RX_DIV_START_1 == val) ? 1 : 0) <<
					SSLPNPHY_Rev2_crsgainCtrl_40_DefaultAntenna_SHIFT);
			} else {
				phy_reg_mod(pi, SSLPNPHY_crsgainCtrl,
					SSLPNPHY_crsgainCtrl_DiversityChkEnable_MASK,
					0x01 << SSLPNPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
				phy_reg_mod(pi, SSLPNPHY_crsgainCtrl,
					SSLPNPHY_crsgainCtrl_DefaultAntenna_MASK,
					((ANT_RX_DIV_START_1 == val) ? 1 : 0) <<
					SSLPNPHY_crsgainCtrl_DefaultAntenna_SHIFT);
			}
		} else {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				phy_reg_mod(pi, SSLPNPHY_Rev2_crsgainCtrl_40,
				SSLPNPHY_Rev2_crsgainCtrl_40_DiversityChkEnable_MASK,
				0x00 << SSLPNPHY_Rev2_crsgainCtrl_40_DiversityChkEnable_SHIFT);

				phy_reg_mod(pi, SSLPNPHY_Rev2_crsgainCtrl_40,
				SSLPNPHY_Rev2_crsgainCtrl_40_DefaultAntenna_MASK,
				(uint16)val << SSLPNPHY_Rev2_crsgainCtrl_40_DefaultAntenna_SHIFT);
			} else {
				phy_reg_mod(pi, SSLPNPHY_crsgainCtrl,
					SSLPNPHY_crsgainCtrl_DiversityChkEnable_MASK,
					0x00 << SSLPNPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
				phy_reg_mod(pi, SSLPNPHY_crsgainCtrl,
					SSLPNPHY_crsgainCtrl_DefaultAntenna_MASK,
					(uint16)val << SSLPNPHY_crsgainCtrl_DefaultAntenna_SHIFT);
			}
		}

		/* Reset radio ctrl and crs gain */
		PHY_REG_LIST_START
			PHY_REG_OR_ENTRY(SSLPNPHY, resetCtrl, 0x44)
			PHY_REG_WRITE_ENTRY(SSLPNPHY, resetCtrl, 0x80)
		PHY_REG_LIST_EXECUTE(pi);

		/* Get pointer to the BTCX shm block */
		if (pi->bt_shm_addr)
			wlapi_bmac_write_shm(pi->sh->physhim,
				pi->bt_shm_addr + M_BTCX_DIVERSITY_SAVE, 0);

	} else if (ISLCNPHY(pi)) {
		if (val > ANT_RX_DIV_FORCE_1) {
			phy_reg_mod(pi, LCNPHY_crsgainCtrl,
				LCNPHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x01 << LCNPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LCNPHY_crsgainCtrl,
				LCNPHY_crsgainCtrl_DefaultAntenna_MASK,
				((ANT_RX_DIV_START_1 == val) ? 1 : 0) <<
				LCNPHY_crsgainCtrl_DefaultAntenna_SHIFT);
		} else {
			phy_reg_mod(pi, LCNPHY_crsgainCtrl,
				LCNPHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x00 << LCNPHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LCNPHY_crsgainCtrl,
				LCNPHY_crsgainCtrl_DefaultAntenna_MASK,
				(uint16)val << LCNPHY_crsgainCtrl_DefaultAntenna_SHIFT);
		}
	} else if (ISABGPHY(pi)) {
		wlc_phy_ant_rxdiv_set_abgphy(pi, val);
	} else if (ISLCN40PHY(pi)) {
		if (val > ANT_RX_DIV_FORCE_1) {
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x01 << LCN40PHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DefaultAntenna_MASK,
				((ANT_RX_DIV_START_1 == val) ? 1 : 0) <<
				LCN40PHY_crsgainCtrl_DefaultAntenna_SHIFT);
		} else {
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x00 << LCN40PHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DefaultAntenna_MASK,
				(uint16)val << LCN40PHY_crsgainCtrl_DefaultAntenna_SHIFT);
		}
	} else {
		PHY_ERROR(("wl%d: %s: PHY_TYPE= %d is Unsupported ",
		          pi->sh->unit, __FUNCTION__, pi->pubpi.phy_type));
		ASSERT(0);
	}

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return;
}


void
wlc_phy_interference_set(wlc_phy_t *pih, bool init)
{
	int wanted_mode;
	phy_info_t *pi = (phy_info_t *)pih;

	if (!(ISNPHY(pi) || ISHTPHY(pi)))
		return;

	if (pi->sh->interference_mode_override == TRUE) {
		/* keep the same values */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			wanted_mode = pi->sh->interference_mode_2G_override;
		} else {
			if (pi->sh->interference_mode_5G_override == 0 ||
				pi->sh->interference_mode_5G_override == 1) {
			wanted_mode = pi->sh->interference_mode_5G_override;
			} else {
				wanted_mode = 0;
			}
		}
	} else {

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			wanted_mode = pi->sh->interference_mode_2G;
		} else {
			wanted_mode = pi->sh->interference_mode_5G;
		}
	}

	if (CHSPEC_CHANNEL(pi->radio_chanspec) != pi->interf.curr_home_channel) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		wlc_phy_interference(pi, wanted_mode, init);
		pi->sh->interference_mode = wanted_mode;

		wlapi_enable_mac(pi->sh->physhim);
	}
}

/* %%%%%% interference */
static bool
wlc_phy_interference(phy_info_t *pi, int wanted_mode, bool init)
{
	if (init) {
		pi->interference_mode_crs_time = 0;
		pi->crsglitch_prev = 0;
		if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) {
			/* clear out all the state */
			wlc_phy_noisemode_reset_nphy(pi);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				wlc_phy_acimode_reset_nphy(pi);
			}
		} else if (ISHTPHY(pi)) {
			/* clear out all the state */
			wlc_phy_noisemode_reset_htphy(pi);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				wlc_phy_acimode_reset_htphy(pi);
			}
		}
	}

	/* NPHY 5G, supported for NON_WLAN and INTERFERE_NONE only */
	if (ISGPHY(pi) || ISLPPHY(pi) || ISSSLPNPHY(pi) ||
		((ISNPHY(pi) || ISHTPHY(pi)) &&
		(CHSPEC_IS2G(pi->radio_chanspec) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
	         (wanted_mode == NON_WLAN || wanted_mode == INTERFERE_NONE))))) {
		if (wanted_mode == INTERFERE_NONE) {	/* disable */

			switch (pi->cur_interference_mode) {
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
			case WLAN_MANUAL:
				if (ISLPPHY(pi))
					wlc_phy_aci_enable_lpphy(pi, FALSE);
				else if (ISNPHY(pi) &&
					CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_nphy(pi, FALSE,
						PHY_ACI_PWR_NOTPRESENT);
				} else if (ISHTPHY(pi) &&
					CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi, FALSE,
						PHY_ACI_PWR_NOTPRESENT);
				} else if (ISGPHY(pi)) {
					wlc_phy_aci_ctl_gphy(pi, FALSE);
				} else if (ISSSLPNPHY(pi)) {
					wlc_sslpnphy_force_adj_gain(pi, FALSE, wanted_mode);
				}
				pi->aci_state &= ~ACI_ACTIVE;
				break;
			case NON_WLAN:
				if (ISNPHY(pi)) {
					if (NREV_GE(pi->pubpi.phy_rev, 3) &&
						CHSPEC_IS2G(pi->radio_chanspec)) {
						wlc_phy_acimode_set_nphy(pi,
							FALSE,
							PHY_ACI_PWR_NOTPRESENT);
						pi->aci_state &= ~ACI_ACTIVE;
					}
				} else if (ISHTPHY(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_htphy(pi,
						FALSE,
						PHY_ACI_PWR_NOTPRESENT);
					pi->aci_state &= ~ACI_ACTIVE;
				} else if (ISSSLPNPHY(pi)) {
					wlc_sslpnphy_force_adj_gain(pi, FALSE, wanted_mode);
				} else {
					pi->interference_mode_crs = 0;
					if (ISABGPHY(pi))
						wlc_phy_aci_interf_nwlan_set_gphy(pi, FALSE);

				}
				break;
			}
		} else {	/* Enable */
			switch (wanted_mode) {
			case NON_WLAN:
				if (ISNPHY(pi)) {
					if (!NREV_GE(pi->pubpi.phy_rev, 3)) {
						PHY_ERROR(("NON_WLAN not supported for NPHY\n"));
					}
				} else if (ISSSLPNPHY(pi)) {
					wlc_sslpnphy_force_adj_gain(pi, FALSE, wanted_mode);
				} else {
					pi->interference_mode_crs = 1;
					if (ISABGPHY(pi)) {
						wlc_phy_aci_interf_nwlan_set_gphy(pi, TRUE);
					}

				}
				break;
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
				/* fall through */
				if (((pi->aci_state & ACI_ACTIVE) != 0) ||
					ISNPHY(pi) || ISHTPHY(pi))
					break;
				if (ISLPPHY(pi)) {
				  wlc_phy_aci_enable_lpphy(pi, FALSE);
				  break;
				}
				if (ISSSLPNPHY(pi)) {
					wlc_sslpnphy_aci(pi, FALSE);
					break;
				}

				/* FALLTHRU */
			case WLAN_MANUAL:
				if (ISLPPHY(pi))
					wlc_phy_aci_enable_lpphy(pi, TRUE);
				else if (ISNPHY(pi)) {
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						wlc_phy_acimode_set_nphy(pi, TRUE,
							PHY_ACI_PWR_HIGH);
					}
				} else if (ISHTPHY(pi)) {
					if (CHSPEC_IS2G(pi->radio_chanspec)) {
						wlc_phy_acimode_set_htphy(pi, TRUE,
							PHY_ACI_PWR_HIGH);
					}
				} else if (ISGPHY(pi)) {
					wlc_phy_aci_ctl_gphy(pi, TRUE);
					break;
				} else if (ISSSLPNPHY(pi)) {
					wlc_sslpnphy_force_adj_gain(pi, TRUE, wanted_mode);
				}
			}
		}
	}

	pi->cur_interference_mode = wanted_mode;
	return TRUE;
}

static void
wlc_phy_aci_enter(phy_info_t *pi)
{
	/* There are cases when the glitch count is continuously high
	 * but there is no ACI.  In this case we want to wait 'countdown' secs
	 * between scans
	 */
	ASSERT((pi->aci_state & ACI_ACTIVE) == 0);

	/* If we have glitches see if they are caused by ACI */
	if (pi->interf.aci.glitch_ma > pi->interf.aci.enter_thresh) {

		/* If we find ACI on our channel, go into ACI avoidance */
		if (pi->interf.aci.countdown == 0) {	/* Is it time to check? */
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

			if (ISGPHY(pi) && wlc_phy_aci_scan_gphy(pi)) {
				pi->aci_start_time = pi->sh->now;
				wlc_phy_aci_ctl_gphy(pi, TRUE);
				pi->interf.aci.countdown = 0;
			} else {
				/* Glitch count is high but no ACI, so wait
				 * hi_glitch_delay seconds before checking for ACI again.
				 */
				pi->interf.aci.countdown = pi->interf.aci.glitch_delay + 1;
			}
			wlapi_enable_mac(pi->sh->physhim);
		}
		if (pi->interf.aci.countdown)
			pi->interf.aci.countdown--;
	} else {
		pi->interf.aci.countdown = 0;	/* no glitches so cancel glitchdelay */
	}
}

static void
wlc_phy_aci_exit(phy_info_t *pi)
{
	if (pi->interf.aci.glitch_ma < pi->interf.aci.exit_thresh) {

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		if (ISGPHY(pi)) {
			if (!wlc_phy_aci_scan_gphy(pi)) {
			PHY_CAL(("%s: Absence of ACI, exiting ACI\n", __FUNCTION__));
			pi->aci_start_time = 0;
			wlc_phy_aci_ctl_gphy(pi, FALSE);
		} else {
				PHY_CAL(("%s: ACI is present - remain in ACI mode\n",
					__FUNCTION__));
			}
		}
		wlapi_enable_mac(pi->sh->physhim);
	}
}

/* update aci rx carrier sense glitch moving average */
static void
wlc_phy_aci_update_ma(phy_info_t *pi)
{
	int cur_glitch_cnt;
	uint16 delta;
	uint offset;
	int16 bphy_cur_glitch_cnt = 0;
	uint16 bphy_delta = 0;
	uint16 ofdm_delta = 0;
	uint16 badplcp_delta = 0;
	int16 cur_badplcp_cnt = 0;
	int16 bphy_cur_badplcp_cnt = 0;
	uint16 bphy_badplcp_delta = 0;
	uint16 ofdm_badplcp_delta = 0;

	/* determine delta number of rxcrs glitches */
	offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, rxcrsglitch);
	cur_glitch_cnt = wlapi_bmac_read_shm(pi->sh->physhim, offset);
	delta = cur_glitch_cnt - pi->interf.aci.pre_glitch_cnt;
	pi->interf.aci.pre_glitch_cnt = cur_glitch_cnt;

	if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi)) {

		/* compute the rxbadplcp  */
		offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, rxbadplcp);
		cur_badplcp_cnt = wlapi_bmac_read_shm(pi->sh->physhim, offset);
		badplcp_delta = cur_badplcp_cnt - pi->interf.pre_badplcp_cnt;
		pi->interf.pre_badplcp_cnt = cur_badplcp_cnt;

		/* determine delta number of bphy rx crs glitches */
		offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, bphy_rxcrsglitch);
		bphy_cur_glitch_cnt = wlapi_bmac_read_shm(pi->sh->physhim, offset);
		bphy_delta = bphy_cur_glitch_cnt - pi->interf.noise.bphy_pre_glitch_cnt;
		pi->interf.noise.bphy_pre_glitch_cnt = bphy_cur_glitch_cnt;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* ofdm glitches is what we will be using */
			ofdm_delta = delta - bphy_delta;
		} else {
			ofdm_delta = delta;
		}

		/* compute bphy rxbadplcp */
#ifdef BADPLCP_UCODE_SUPPORT
		offset = M_UCODE_MACSTAT + OFFSETOF(macstat_t, bphy_badplcp);
		bphy_cur_badplcp_cnt = wlapi_bmac_read_shm(pi->sh->physhim, offset);
#else
		bphy_cur_badplcp_cnt = 0;
#endif /* BADPLCP_UCODE_SUPPORT */
		bphy_badplcp_delta = bphy_cur_badplcp_cnt -
			pi->interf.bphy_pre_badplcp_cnt;
		pi->interf.bphy_pre_badplcp_cnt = bphy_cur_badplcp_cnt;

		/* ofdm bad plcps is what we will be using */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			ofdm_badplcp_delta = badplcp_delta - bphy_badplcp_delta;
		} else {
			ofdm_badplcp_delta = badplcp_delta;
		}
	}

	if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi)) {
		/* if we aren't suppose to update yet, don't */
		if (pi->interf.scanroamtimer != 0) {
			return;
		}

	}

	/* evict old value */
	pi->interf.aci.ma_total -= pi->interf.aci.ma_list[pi->interf.aci.ma_index];

	/* admit new value */
	pi->interf.aci.ma_total += delta;
	pi->interf.aci.glitch_ma_previous = pi->interf.aci.glitch_ma;
	pi->interf.aci.glitch_ma = pi->interf.aci.ma_total / MA_WINDOW_SZ;

	pi->interf.aci.ma_list[pi->interf.aci.ma_index++] = delta;
	if (pi->interf.aci.ma_index >= MA_WINDOW_SZ)
		pi->interf.aci.ma_index = 0;

	if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi)) {
		pi->interf.badplcp_ma_total -=
			pi->interf.badplcp_ma_list[pi->interf.badplcp_ma_index];
		pi->interf.badplcp_ma_total += badplcp_delta;
		pi->interf.badplcp_ma_previous = pi->interf.badplcp_ma;
		pi->interf.badplcp_ma =
			pi->interf.badplcp_ma_total / MA_WINDOW_SZ;

		pi->interf.badplcp_ma_list[pi->interf.badplcp_ma_index++] =
			badplcp_delta;
		if (pi->interf.badplcp_ma_index >= MA_WINDOW_SZ)
			pi->interf.badplcp_ma_index = 0;

		pi->interf.noise.ofdm_ma_total -=
			pi->interf.noise.ofdm_glitch_ma_list[pi->interf.noise.ofdm_ma_index];
		pi->interf.noise.ofdm_ma_total += ofdm_delta;
		pi->interf.noise.ofdm_glitch_ma_previous = pi->interf.noise.ofdm_glitch_ma;
		pi->interf.noise.ofdm_glitch_ma =
			pi->interf.noise.ofdm_ma_total / PHY_NOISE_MA_WINDOW_SZ;

		pi->interf.noise.ofdm_glitch_ma_list[pi->interf.noise.ofdm_ma_index++] = ofdm_delta;
		if (pi->interf.noise.ofdm_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
			pi->interf.noise.ofdm_ma_index = 0;


		pi->interf.noise.ofdm_badplcp_ma_total -=
		pi->interf.noise.ofdm_badplcp_ma_list[pi->interf.noise.ofdm_badplcp_ma_index];

		pi->interf.noise.ofdm_badplcp_ma_total += ofdm_badplcp_delta;
		pi->interf.noise.ofdm_badplcp_ma_previous = pi->interf.noise.ofdm_badplcp_ma;
		pi->interf.noise.ofdm_badplcp_ma =
			pi->interf.noise.ofdm_badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;

		pi->interf.noise.ofdm_badplcp_ma_list[pi->interf.noise.ofdm_badplcp_ma_index++] =
			ofdm_badplcp_delta;
		if (pi->interf.noise.ofdm_badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
			pi->interf.noise.ofdm_badplcp_ma_index = 0;
	}

	if (((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) || ISHTPHY(pi)) &&
		(pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
		pi->sh->interference_mode == NON_WLAN)) {
		PHY_ACI(("wlc_phy_aci_update_ma: ACI= %s, rxglitch_ma= %d,"
			" badplcp_ma= %d, ofdm_glitch_ma= %d,"
			" ofdm_badplcp_ma= %d, crsminpwr index= %d,"
			" init gain= 0x%x, channel= %d\n",
			(pi->aci_state & ACI_ACTIVE) ? "Active" : "Inactive",
			pi->interf.aci.glitch_ma,
			pi->interf.badplcp_ma,
			pi->interf.noise.ofdm_glitch_ma,
			pi->interf.noise.ofdm_badplcp_ma,
			pi->interf.crsminpwr_index,
			pi->interf.init_gain_core1, CHSPEC_CHANNEL(pi->radio_chanspec)));
	} else {
		PHY_ACI(("wlc_phy_aci_update_ma: ave glitch %d, ACI is %s, delta is %d\n",
		pi->interf.aci.glitch_ma,
		(pi->aci_state & ACI_ACTIVE) ? "Active" : "Inactive", delta));
	}
}

static void
wlc_phy_aci_upd(phy_info_t *pi)
{
	wlc_phy_aci_update_ma(pi);

	switch (pi->sh->interference_mode) {

		case NON_WLAN:
			/* NON_WLAN NPHY */
			if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) {
				/* run noise mitigation only */
				wlc_phy_noisemode_upd_nphy(pi);
			} else if (ISHTPHY(pi)) {
				/* run noise mitigation only */
				wlc_phy_noisemode_upd_htphy(pi);
			}
			break;
		case WLAN_AUTO:

			if (ISGPHY(pi)) {
				/* Attempt to enter ACI mode if not already active */
				if (!(pi->aci_state & ACI_ACTIVE)) {
					wlc_phy_aci_enter(pi);
				} else {
					if (((pi->sh->now - pi->aci_start_time) %
						pi->aci_exit_check_period) == 0) {
						wlc_phy_aci_exit(pi);
					}
				}
			}
			else if (ISLPPHY(pi)) {
				wlc_phy_aci_upd_lpphy(pi);
			}
			else if (ISSSLPNPHY(pi) && (SSLPNREV_IS(pi->pubpi.phy_rev, 3))) {
				wlc_sslpnphy_noise_measure((wlc_phy_t *)pi);
			}
			else if (ISNPHY(pi) || ISHTPHY(pi)) {
				if (ASSOC_INPROG_PHY(pi))
					break;
#ifdef NOISE_CAL_LCNXNPHY
				if ((NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
				NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 2))) {
					wlc_phy_aci_noise_measure_nphy(pi, TRUE);
				}
				else
#endif
				{
					/* 5G band not supported yet */
					if (CHSPEC_IS5G(pi->radio_chanspec))
						break;

					if (PUB_NOT_ASSOC(pi)) {
						/* not associated:  do not run aci routines */
						break;
					}

					/* Attempt to enter ACI mode if not already active */
					/* only run this code if associated */
					if (!(pi->aci_state & ACI_ACTIVE)) {
						if ((pi->sh->now  % NPHY_ACI_CHECK_PERIOD) == 0) {
							PHY_ACI(("Interf Mode 3,"
								" pi->interf.aci.glitch_ma = %d\n",
								pi->interf.aci.glitch_ma));
							if (pi->interf.aci.glitch_ma >=
								pi->interf.aci.enter_thresh) {
								if (ISNPHY(pi)) {
								wlc_phy_acimode_upd_nphy(pi);
								} else if (ISHTPHY(pi)) {
								wlc_phy_acimode_upd_htphy(pi);
								}
							}
						}
					} else {
						if (((pi->sh->now - pi->aci_start_time) %
							pi->aci_exit_check_period) == 0) {
							if (ISNPHY(pi)) {
								wlc_phy_acimode_upd_nphy(pi);
							} else if (ISHTPHY(pi)) {
								wlc_phy_acimode_upd_htphy(pi);
							}
						}
					}
				}
			}
			break;
		case WLAN_AUTO_W_NOISE:
			if (ISNPHY(pi) || ISHTPHY(pi)) {
#ifdef NOISE_CAL_LCNXNPHY
				if ((NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
				NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 2))) {
					wlc_phy_aci_noise_measure_nphy(pi, TRUE);
				}
				else
#endif
				{
					/* 5G band not supported yet */
					if (CHSPEC_IS5G(pi->radio_chanspec))
						break;


					/* only do this for 4322 and future revs */
					if (NREV_GE(pi->pubpi.phy_rev, 3)) {
						/* Attempt to enter ACI mode if
						 * not already active
						 */
						wlc_phy_aci_noise_upd_nphy(pi);
					} else if (ISHTPHY(pi)) {
						wlc_phy_aci_noise_upd_htphy(pi);
					}
				}
			}
			else if (ISSSLPNPHY(pi) && (SSLPNREV_IS(pi->pubpi.phy_rev, 3))) {
				wlc_sslpnphy_noise_measure((wlc_phy_t *)pi);
			}
			break;

		default:
			break;
	}
}

void
wlc_phy_acimode_noisemode_reset(wlc_phy_t *pih, uint channel,
	bool clear_aci_state, bool clear_noise_state, bool disassoc)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (pi->sh->interference_mode_override == TRUE)
		return;

	if ((disassoc) ||
		((CHSPEC_CHANNEL(pi->radio_chanspec) != pi->interf.curr_home_channel) &&
		(disassoc == FALSE))) {
        /* add by jack,2015-9-12,修复interference 2时，出现-63dbm~-68dbm无法接收的问题*/
        if (pi->sh->interference_mode == WLAN_MANUAL) {
            clear_noise_state = 0;
        }
        /* end by jack */

		/* not home channel... reset */
		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			wlc_phy_aci_noise_reset_nphy(pi, channel,
				clear_aci_state, clear_noise_state, disassoc);
		} else if (ISHTPHY(pi)) {
			wlc_phy_aci_noise_reset_htphy(pi, channel,
				clear_aci_state, clear_noise_state, disassoc);
		}
	}
}

static void
wlc_phy_noise_calc(phy_info_t *pi, uint32 *cmplx_pwr, int8 *pwr_ant, uint8 extra_gain_1dB)
{
	int8 cmplx_pwr_dbm[PHY_CORE_MAX];
	uint8 i;
	uint16 gain;

	bzero((uint8 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));
	ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);

	gain = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_BLKS+0xC);
	PHY_INFORM(("--> RXGAIN: %d\n", gain));

	wlc_phy_compute_dB(cmplx_pwr, cmplx_pwr_dbm, pi->pubpi.phy_corenum);

	FOREACH_CORE(pi, i) {
		if (NREV_GE(pi->pubpi.phy_rev, 3))
			cmplx_pwr_dbm[i] += (int8) (PHY_NOISE_OFFSETFACT_4322 - gain);
		else if (NREV_LT(pi->pubpi.phy_rev, 3))
			/* assume init gain 70 dB, 128 maps to 1V so
			 * 10*log10(128^2*2/128/128/50)+30=16 dBm
			 * WARNING: if the nphy init gain is ever changed,
			 * this formula needs to be updated
			*/
			cmplx_pwr_dbm[i] += (int8)(16 - (15) * 3 - (70 + extra_gain_1dB));
#if defined(WLTEST)
		else if (ISLCNPHY(pi) && LCNREV_GE(pi->pubpi.phy_rev, 2)) {
			int16 noise_offset_fact;
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			wlc_phy_get_noiseoffset_lcnphy(pi, &noise_offset_fact);
			cmplx_pwr_dbm[i] +=
				(int8)(noise_offset_fact - pi_lcn->rxpath_gain);
		}
#endif
		pwr_ant[i] = cmplx_pwr_dbm[i];
		PHY_INFORM(("wlc_phy_noise_calc_phy: pwr_ant[%d] = %d\n", i, pwr_ant[i]));
	}

	PHY_INFORM(("%s: samples %d ant %d\n", __FUNCTION__, pi->phy_rxiq_samps,
		pi->phy_rxiq_antsel));
}

static void
wlc_phy_noise_calc_fine_resln(phy_info_t *pi, uint32 *cmplx_pwr, int16 *pwr_ant,
                              uint8 extra_gain_1dB)
{
	int16 cmplx_pwr_dbm[PHY_CORE_MAX];
	uint8 i;

	/* lookup table for computing the dB contribution from the first
	 * 4 bits after MSB (most significant NONZERO bit) in cmplx_pwr[core]
	 * (entries in multiples of 0.25dB):
	 */
	uint8 dB_LUT[] = {0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 8, 9,
		10, 10, 11, 11};
	uint8 LUT_correction[] = {13, 12, 12, 13, 16, 20, 25,
		5, 12, 19, 2, 11, 20, 5, 15, 1};

	bzero((uint16 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));
	ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);

	/* Convert sample-power to dB scale: */
	FOREACH_CORE(pi, i) {
		uint8 shift_ct, lsb, msb_loc;
		uint8 msb2345 = 0x0;
		uint32 tmp;
		tmp = cmplx_pwr[i];
		shift_ct = msb_loc = 0;
		while (tmp != 0) {
			tmp = tmp >> 1;
			shift_ct++;
			lsb = (uint8)(tmp & 1);
			if (lsb == 1)
				msb_loc = shift_ct;
		}

		/* Store first 4 bits after MSB: */
		if (msb_loc <= 4) {
			msb2345 = (cmplx_pwr[i] << (4-msb_loc)) & 0xf;
		} else {
			/* Need to first round cmplx_pwr to 5 MSBs: */
			tmp = cmplx_pwr[i] + (1U << (msb_loc-5));
			/* Check if MSB has shifted in the process: */
			if (tmp & (1U << (msb_loc+1))) {
				msb_loc++;
			}
			msb2345 = (tmp >> (msb_loc-4)) & 0xf;
		}

		/* Power in 0.25 dB steps: */
		cmplx_pwr_dbm[i] = ((3*msb_loc) << 2) + dB_LUT[msb2345];

		/* Apply a possible +0.25dB (1 step) correction depending
		 * on MSB location in cmplx_pwr[core]:
		 */
		cmplx_pwr_dbm[i] += (int16)((msb_loc >= LUT_correction[msb2345]) ? 1 : 0);
	}

	if (ISHTPHY(pi)) {
		int16 assumed_gain;

		if (IS_X12_BOARDTYPE(pi)) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				assumed_gain = (int16)(HTPHY_NOISE_INITGAIN_X12_2G);
			} else {
				assumed_gain = (int16)(HTPHY_NOISE_INITGAIN_X12_5G);
			}
		} else if (IS_X29_BOARDTYPE(pi)) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				assumed_gain = (int16)(HTPHY_NOISE_INITGAIN_X29_2G);
			} else {
				assumed_gain = (int16)(HTPHY_NOISE_INITGAIN_X29_5G);
			}
		} else {
			assumed_gain = (int16)(HTPHY_NOISE_INITGAIN);
		}

		assumed_gain += extra_gain_1dB;

		FOREACH_CORE(pi, i) {
			/* Convert to analog input power at ADC and then
			 * backoff applied gain to get antenna input power:
			 */


			/* scale conversion factor by 4 as power is in 0.25dB steps */
			cmplx_pwr_dbm[i] += (int16) ((HTPHY_NOISE_SAMPLEPWR_TO_DBM -
			                              assumed_gain) << 2);
			pwr_ant[i] = cmplx_pwr_dbm[i];
			PHY_INFORM(("wlc_phy_noise_calc_fine_resln: pwr_ant[%d] = %d\n",
				i, pwr_ant[i]));
		}
#if defined(WLTEST)
	} else if (ISLCNPHY(pi) && LCNREV_GE(pi->pubpi.phy_rev, 2)) {
		int16 noise_offset_fact;
		int8 freq_offset_fact;
		phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
		wlc_phy_get_noiseoffset_lcnphy(pi, &noise_offset_fact);
		FOREACH_CORE(pi, i) {
			cmplx_pwr_dbm[i] +=
				(int16) ((noise_offset_fact - pi_lcn->rxpath_gain) << 2);
			pwr_ant[i] = cmplx_pwr_dbm[i];
			/* Frequency based variation correction */
			wlc_lcnphy_get_lna_freq_correction(pi, &freq_offset_fact);
			pwr_ant[i] = pwr_ant[i] - freq_offset_fact;
		}
#endif
	} else if (ISNPHY(pi) &&
		(NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 6))) {
		FOREACH_CORE(pi, i) {
			/* Convert to analog input power at ADC and then
			 * backoff applied gain to get antenna input power:
			 */
			cmplx_pwr_dbm[i] += (int16) ((NPHY_NOISE_SAMPLEPWR_TO_DBM -
				NPHY_NOISE_INITGAIN) << 2);
			/* scale conversion factor by 4 as power is in 0.25dB steps */
			pwr_ant[i] = cmplx_pwr_dbm[i];
			PHY_INFORM(("wlc_phy_noise_calc_fine_resln: pwr_ant[%d] = %d\n",
				i, pwr_ant[i]));
		}
	} else {
		FOREACH_CORE(pi, i) {
			/* assume init gain 70 dB, 128 maps to 1V so
			 * 10*log10(128^2*2/128/128/50)+30=16 dBm
			 *  WARNING: if the nphy init gain is ever changed,
			 * this formula needs to be updated
			 */
			cmplx_pwr_dbm[i] += ((int16)(16 << 2) - (int16)((15 << 2)*3)
			                     - (int16)((70 + extra_gain_1dB) << 2));
			pwr_ant[i] = cmplx_pwr_dbm[i];
		}
	}

}

static void
wlc_phy_noise_save(phy_info_t *pi, int8 *noise_dbm_ant, int8 *max_noise_dbm)
{
	uint8 i;

	FOREACH_CORE(pi, i) {
		/* save noise per core */
		pi->phy_noise_win[i][pi->phy_noise_index] = noise_dbm_ant[i];

		/* save the MAX for all cores */
		if (noise_dbm_ant[i] > *max_noise_dbm)
			*max_noise_dbm = noise_dbm_ant[i];
	}
	pi->phy_noise_index = MODINC_POW2(pi->phy_noise_index, PHY_NOISE_WINDOW_SZ);
}

static uint8 wlc_phy_calc_extra_init_gain(phy_info_t *pi, uint8 extra_gain_3dB,
                                         rxgain_t rxgain[])
{
	uint16 init_gain_code[4];
	uint8 core, MAX_DVGA, MAX_LPF, MAX_MIX;
	uint8 dvga, mix, lpf0, lpf1;
	uint8 dvga_inc, lpf0_inc, lpf1_inc;
	uint8 max_inc, gain_ticks = extra_gain_3dB;

	if (ISHTPHY(pi)) {
		MAX_DVGA = 4; MAX_LPF = 10; MAX_MIX = 4;
		wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_RFSEQ, 3, 0x106, 16, &init_gain_code);

		/* Find if the requested gain increase is possible */
		FOREACH_CORE(pi, core) {
			dvga = 0;
			mix = (init_gain_code[core] >> 4) & 0xf;
			lpf0 = (init_gain_code[core] >> 8) & 0xf;
			lpf1 = (init_gain_code[core] >> 12) & 0xf;
			max_inc = MAX(0, MAX_DVGA - dvga) + MAX(0, MAX_LPF - lpf0 - lpf1) +
			        MAX(0, MAX_MIX - mix);
			gain_ticks = MIN(gain_ticks, max_inc);
		}

		if (gain_ticks != extra_gain_3dB) {
			PHY_INFORM(("%s: Unable to find enough extra gain. Using extra_gain = %d\n",
			            __FUNCTION__, 3 * gain_ticks));
		}

		/* Do nothing if no gain increase is required/possible */
		if (gain_ticks == 0) {
			return gain_ticks;
		}

		/* Find the mix, lpf0, lpf1 gains required for extra INITgain */
		FOREACH_CORE(pi, core) {
			uint8 gain_inc = gain_ticks;

			dvga = 0;
			mix = (init_gain_code[core] >> 4) & 0xf;
			lpf0 = (init_gain_code[core] >> 8) & 0xf;
			lpf1 = (init_gain_code[core] >> 12) & 0xf;

			dvga_inc = MIN((uint8) MAX(0, MAX_DVGA - dvga), gain_inc);
			dvga += dvga_inc;
			gain_inc -= dvga_inc;

			lpf1_inc = MIN((uint8) MAX(0, MAX_LPF - lpf1 - lpf0), gain_inc);
			lpf1 += lpf1_inc;
			gain_inc -= lpf1_inc;

			lpf0_inc = MIN((uint8) MAX(0, MAX_LPF - lpf1 - lpf0), gain_inc);
			lpf0 += lpf0_inc;
			gain_inc -= lpf0_inc;

			mix += MIN((uint8) MAX(0, MAX_MIX - mix), gain_inc);

			rxgain[core].lna1 = init_gain_code[core] & 0x3;
			rxgain[core].lna2 = (init_gain_code[core] >> 2) & 0x3;
			rxgain[core].mix  = mix;
			rxgain[core].lpf0 = lpf0;
			rxgain[core].lpf1 = lpf1;
			rxgain[core].dvga = dvga;
		}
	} else {
		PHY_ERROR(("%s: Extra INITgain not supported\n", __FUNCTION__));
		return 0;
	}

	return gain_ticks;
}


static void
wlc_phy_rx_iq_est_aphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs)
{
	uint16 offset = (ISAPHY(pi)) ? 0 : GPHY_TO_APHY_OFF;

	/* Get Rx IQ Imbalance Estimate from modem */
	phy_reg_write(pi, (offset+APHY_IqestNumSamps), num_samps);
	phy_reg_mod(pi, (offset+APHY_IqestEnWaitTime), APHY_IqEnWaitTime_waitTime_MASK,
		(wait_time << APHY_IqEnWaitTime_waitTime_SHIFT));
	phy_reg_mod(pi, (offset+APHY_IqestEnWaitTime), APHY_IqMode,
		((wait_for_crs) ? APHY_IqMode : 0));
	phy_reg_mod(pi, (offset+APHY_IqestEnWaitTime), APHY_IqStart, APHY_IqStart);

	/* wait for estimate */
	SPINWAIT(((phy_reg_read(pi, offset+APHY_IqestEnWaitTime) & APHY_IqStart) != 0), 10000);
	ASSERT((phy_reg_read(pi, offset+APHY_IqestEnWaitTime) & APHY_IqStart) == 0);

	if ((phy_reg_read(pi, offset+APHY_IqestEnWaitTime) & APHY_IqStart) == 0) {
		est[0].i_pwr = (phy_reg_read(pi, offset+APHY_IqestIpwrAccHi) << 16) |
			phy_reg_read(pi, offset+APHY_IqestIpwrAccLo);
		est[0].q_pwr = (phy_reg_read(pi, offset+APHY_IqestQpwrAccHi) << 16) |
			phy_reg_read(pi, offset+APHY_IqestQpwrAccLo);
		est[0].iq_prod = (phy_reg_read(pi, offset+APHY_IqestIqAccHi) << 16) |
			phy_reg_read(pi, offset+APHY_IqestIqAccLo);
		PHY_CAL(("i_pwr = %u, q_pwr = %u, iq_prod = %d\n",
			est[0].i_pwr, est[0].q_pwr, est[0].iq_prod));
	} else {
		PHY_ERROR(("wlc_phy_rx_iq_est_aphy: IQ measurement timed out\n"));
	}
}

static uint32
wlc_phy_rx_iq_est(phy_info_t *pi, uint8 samples, uint8 antsel, uint8 resolution, uint8 lpf_hpc,
	uint8 dig_lpf, uint8 gain_correct, uint8 extra_gain_3dB, uint8 wait_for_crs)
{
	phy_iq_est_t est[PHY_CORE_MAX];
	uint32 cmplx_pwr[PHY_CORE_MAX];
	int8 noise_dbm_ant[PHY_CORE_MAX];
	int16 noise_dBm_ant_fine[PHY_CORE_MAX];
	uint16 log_num_samps, num_samps;
	uint16 classif_state = 0;
	uint16 org_antsel;
	uint8 wait_time = 32;
	bool sampling_in_progress = (pi->phynoise_state != 0);
	uint8 i, extra_gain_1dB = 0;
	uint32 result = 0;

	if (sampling_in_progress) {
		PHY_INFORM(("wlc_phy_rx_iq_est: sampling_in_progress %d\n", sampling_in_progress));
		return 0;
	}

	/* Extra INITgain is supported only for HTPHY currently */
	if (extra_gain_3dB > 0) {
		if (!ISHTPHY(pi)) {
			extra_gain_3dB = 0;
			PHY_ERROR(("%s: Extra INITgain not supported for this phy.\n",
			           __FUNCTION__));
		}
	}

	pi->phynoise_state |= PHY_NOISE_STATE_MON;
	/* choose num_samps to be some power of 2 */
	log_num_samps = samples;
	num_samps = 1 << log_num_samps;

	bzero((uint8 *)est, sizeof(est));
	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));
	bzero((uint16 *)noise_dBm_ant_fine, sizeof(noise_dBm_ant_fine));

	/* get IQ power measurements */
	if (ISNPHY(pi)) {
		uint16  phy_clip_state[2];
		uint16 clip_off[] = {0xffff, 0xffff};

		if (NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 6)) {
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);
			if (lpf_hpc) {
				/* Override the LPF high pass corners to their
				 * lowest values (0x1)
				 */
				wlc_phy_lpf_hpc_override_nphy(pi, TRUE);
			}
		} else {
			classif_state = wlc_phy_classifier_nphy(pi, 0, 0);
			wlc_phy_classifier_nphy(pi, 3, 0);
			wlc_phy_clip_det_nphy(pi, 0, &phy_clip_state[0]);
			wlc_phy_clip_det_nphy(pi, 1, clip_off);
		}

		/* get IQ power measurements */
		wlc_phy_rx_iq_est_nphy(pi, est, num_samps, wait_time, wait_for_crs);

		if (NREV_GE(pi->pubpi.phy_rev, 3) && NREV_LE(pi->pubpi.phy_rev, 6)) {
			if (lpf_hpc) {
				/* Restore LPF high pass corners to their original values */
				wlc_phy_lpf_hpc_override_nphy(pi, FALSE);
			}
			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
		} else {
			wlc_phy_clip_det_nphy(pi, 1, &phy_clip_state[0]);
			/* restore classifier settings and reenable MAC ASAP */
			wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
				classif_state);
		}
	} else if (ISHTPHY(pi)) {
		rxgain_t rxgain[PHY_CORE_MAX];
		rxgain_ovrd_t rxgain_ovrd[PHY_CORE_MAX];

		wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
		if (lpf_hpc) {
			/* Override the LPF high pass corners to their lowest values (0x1) */
			wlc_phy_lpf_hpc_override_htphy(pi, TRUE);
		}

		/* Overide the digital LPF */
		if (dig_lpf) {
			wlc_phy_dig_lpf_override_htphy(pi, dig_lpf);
		}

		/* Increase INITgain if requested */
		if (extra_gain_3dB > 0) {
			extra_gain_3dB = wlc_phy_calc_extra_init_gain(pi, extra_gain_3dB, rxgain);

			/* Override higher INITgain if possible */
			if (extra_gain_3dB > 0) {
				wlc_phy_rfctrl_override_rxgain_htphy(pi, 0, rxgain, rxgain_ovrd);
			}
		}

		/* get IQ power measurements */
		wlc_phy_rx_iq_est_htphy(pi, est, num_samps, wait_time, wait_for_crs);

		/* Disable the overrides if they were set */
		if (extra_gain_3dB > 0) {
			wlc_phy_rfctrl_override_rxgain_htphy(pi, 1, NULL, rxgain_ovrd);
		}

		if (lpf_hpc) {
			/* Restore LPF high pass corners to their original values */
			wlc_phy_lpf_hpc_override_htphy(pi, FALSE);
		}

		/* Restore the digital LPF */
		if (dig_lpf) {
			wlc_phy_dig_lpf_override_htphy(pi, 0);
		}

		wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
	} else if (ISLPPHY(pi)) {
		wlc_phy_set_deaf_lpphy(pi, (bool)0);
		noise_dbm_ant[0] = (int8)wlc_phy_rx_signal_power_lpphy(pi, 20);
		wlc_phy_clear_deaf_lpphy(pi, (bool)0);
		PHY_CAL(("channel = %d noise pwr=%d\n",
			(int)CHSPEC_CHANNEL(pi->radio_chanspec), noise_dbm_ant[0]));

		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		return (noise_dbm_ant[0] & 0xff);
	} else if (ISSSLPNPHY(pi)) {
		PHY_ERROR(("wlc_phy_rx_iq_est: SSLPNPHY not supported yet\n"));
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		return 0;
	} else if (ISLCNPHY(pi)) {
#if defined(WLTEST)
		uint8 save_antsel;
		(void)wlc_phy_ant_rxdiv_get((wlc_phy_t *)pi, &save_antsel);
		wlc_phy_ant_rxdiv_set((wlc_phy_t *)pi, antsel);
		wlc_lcnphy_rx_power(pi, num_samps, wait_time, wait_for_crs, est);
		wlc_phy_ant_rxdiv_set((wlc_phy_t *)pi, save_antsel);
#else
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		return 0;
#endif
	} else if (ISLCN40PHY(pi)) {
		PHY_ERROR(("wlc_phy_rx_iq_est: LCN40PHY not supported yet\n"));
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		return 0;
	} else {	/* APHY, GPHY */
		uint16 val;
		uint16 bbconfig_reg = APHY_BBCONFIG + ((ISAPHY(pi)) ? 0 : GPHY_TO_APHY_OFF);
		org_antsel = ((val = phy_reg_read(pi, bbconfig_reg)) & PHY_BBC_ANT_MASK);
		phy_reg_mod(pi, bbconfig_reg, PHY_BBC_ANT_MASK, antsel);
		phy_reg_write(pi, bbconfig_reg, val | BBCFG_RESETCCA);
		phy_reg_write(pi, bbconfig_reg, val & (~BBCFG_RESETCCA));
		wlc_phy_rx_iq_est_aphy(pi, est, num_samps, wait_time, wait_for_crs);
		phy_reg_mod(pi, bbconfig_reg, PHY_BBC_ANT_MASK, org_antsel);
		val = phy_reg_read(pi, bbconfig_reg);
		phy_reg_write(pi, bbconfig_reg, val | BBCFG_RESETCCA);
		phy_reg_write(pi, bbconfig_reg, val & (~BBCFG_RESETCCA));
	}

	/* sum I and Q powers for each core, average over num_samps with rounding */
	ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);
	FOREACH_CORE(pi, i) {
		cmplx_pwr[i] = ((est[i].i_pwr + est[i].q_pwr) +
			(1U << (log_num_samps-1))) >> log_num_samps;
	}

	/* convert in 1dB gain for gain adjustment */
	extra_gain_1dB = 3 * extra_gain_3dB;

	if (resolution == 0) {
		/* pi->phy_noise_win per antenna is updated inside */
		wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, extra_gain_1dB);

		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;

		for (i = pi->pubpi.phy_corenum; i >= 1; i--)
			result = (result << 8) | (noise_dbm_ant[i-1] & 0xff);

		return result;
	}
	else if (resolution == 1) {
		/* Reports power in finer resolution than 1 dB (currently 0.25 dB) */

		if (((ISHTPHY(pi)) && (pi->pubpi.phy_corenum == 3)) ||
			(ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 3) &&
			NREV_LE(pi->pubpi.phy_rev, 6))) ||
			(ISLCNPHY(pi) && LCNREV_GE(pi->pubpi.phy_rev, 2))) {

			int16 noisefloor;
			wlc_phy_noise_calc_fine_resln(pi, cmplx_pwr, noise_dBm_ant_fine,
			                              extra_gain_1dB);

			if (gain_correct) {
				int16 gainerr[PHY_CORE_MAX];
				int16 gain_err_temp_adj;
				wlc_phy_get_rxgainerr_phy(pi, gainerr);

				/* make and apply temperature correction */
				wlc_phy_upd_gain_wrt_temp_phy(pi, &gain_err_temp_adj);

				FOREACH_CORE(pi, i) {
					/* gainerr is in 0.5dB units;
					 * need to convert to 0.25dB units
					 */
					gainerr[i] = gainerr[i] << 1;
					/* Apply gain correction */
					noise_dBm_ant_fine[i] -= gainerr[i];

					/* Apply temperature gain correction */
					noise_dBm_ant_fine[i] += gain_err_temp_adj << 1;
				}
			}

			if (ISLCNPHY(pi))
				noisefloor = (CHSPEC_IS40(pi->radio_chanspec))?
					4*LCNPHY_NOISE_FLOOR_40M : 4*LCNPHY_NOISE_FLOOR_20M;
			else
				noisefloor = (CHSPEC_IS40(pi->radio_chanspec))?
					4*HTPHY_NOISE_FLOOR_40M : 4*HTPHY_NOISE_FLOOR_20M;

			FOREACH_CORE(pi, i) {
				if (noise_dBm_ant_fine[i] < noisefloor) {
				        noise_dBm_ant_fine[i] = noisefloor;
				}
			}

			for (i = pi->pubpi.phy_corenum; i >= 1; i--) {
				result = (result << 10) | (noise_dBm_ant_fine[i-1] & 0x3ff);
			}
			pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
			return result;
		}
		else {
			PHY_ERROR(("%s: Fine-resolution reporting not supported\n", __FUNCTION__));
			pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
			return 0;
		}
	}

	pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
	return 0;
}

bool
wlc_phy_get_rxgainerr_phy(phy_info_t *pi, int16 *gainerr)
{
	/*
	 * Retrieves rxgain error (read from srom) for current channel;
	 * Returns TRUE if no gainerror was written to SROM, FALSE otherwise
	 */
	uint8 core;
	bool srom_isempty;

	if (ISLCNPHY(pi)) {
#if defined(WLTEST)
		wlc_phy_get_rxgainerr_lcnphy(pi, gainerr);
#endif
		return 1;
	} else if (!((ISNPHY(pi) && (NREV_GE(pi->pubpi.phy_rev, 3) &&
		NREV_LE(pi->pubpi.phy_rev, 6))) || ISHTPHY(pi))) {
		return 0;
	} else {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 14) {
			/* 2G */
			FOREACH_CORE(pi, core) {
				gainerr[core] = (int16) pi->srom_rxgainerr_2g[core];
			}
			srom_isempty = pi->srom_rxgainerr2g_isempty;
		} else {
			/* 5G */
			if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 48) {
				/* 5G-low: channels 36 through 48 */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->srom_rxgainerr_5gl[core];
				}
				srom_isempty = pi->srom_rxgainerr5gl_isempty;
			} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 64) {
				/* 5G-mid: channels 52 through 64 */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->srom_rxgainerr_5gm[core];
				}
				srom_isempty = pi->srom_rxgainerr5gm_isempty;
			} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 128) {
				/* 5G-high: channels 100 through 128 */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->srom_rxgainerr_5gh[core];
				}
				srom_isempty = pi->srom_rxgainerr5gh_isempty;
			} else {
				/* 5G-upper: channels 132 and above */
				FOREACH_CORE(pi, core) {
					gainerr[core] = (int16) pi->srom_rxgainerr_5gu[core];
				}
				srom_isempty = pi->srom_rxgainerr5gu_isempty;
			}
		}
	}
	return srom_isempty;
}

#ifdef NOISE_CAL_LCNXNPHY
void wlc_phy_noise_trigger_ucode(phy_info_t *pi)
{
	/* ucode assumes these shm locations start with 0
	 * and ucode will not touch them in case of sampling expires
	 */
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP0, 0);
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP1, 0);
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP2, 0);
	wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP3, 0);
	if (ISHTPHY(pi)) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP4, 0);
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP5, 0);
	}
	if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 2)) {
		wlc_phy_noise_cal_ocl_enable_disable_nphy(pi, 0);
	}

	OR_REG(pi->sh->osh, &pi->regs->maccommand, MCMD_BG_NOISE);

}
#endif /* NOISE_CAL_LCNXNPHY */
static void
wlc_phy_noise_sample_request(wlc_phy_t *pih, uint8 reason, uint8 ch)
{
	phy_info_t *pi = (phy_info_t*)pih;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	bool sampling_in_progress = (pi->phynoise_state != 0);
	bool wait_for_intr = TRUE;

	PHY_NONE(("wlc_phy_noise_sample_request: state %d reason %d, channel %d\n",
	          pi->phynoise_state, reason, ch));

	if (NORADIO_ENAB(pi->pubpi)) {
		return;
	}

	/* since polling is atomic, sampling_in_progress equals to interrupt sampling ongoing
	 *  In these collision cases, always yield and wait interrupt to finish, where the results
	 *  maybe be sharable if channel matches in common callback progressing.
	 */
	if (sampling_in_progress)
		return;

	switch (reason) {
	case PHY_NOISE_SAMPLE_MON:

		pi->phynoise_chan_watchdog = ch;
		pi->phynoise_state |= PHY_NOISE_STATE_MON;

		break;

	case PHY_NOISE_SAMPLE_EXTERNAL:

		pi->phynoise_state |= PHY_NOISE_STATE_EXTERNAL;
		break;

	default:
		ASSERT(0);
		break;
	}

	/* start test, save the timestamp to recover in case ucode gets stuck */
	pi->phynoise_now = pi->sh->now;

	/* Fixed noise, don't need to do the real measurement */
	if (pi->phy_fixed_noise) {
		if (ISNPHY(pi) || ISHTPHY(pi)) {
			uint8 i;
			FOREACH_CORE(pi, i) {
				pi->phy_noise_win[i][pi->phy_noise_index] =
					PHY_NOISE_FIXED_VAL_NPHY;
			}
			pi->phy_noise_index = MODINC_POW2(pi->phy_noise_index,
				PHY_NOISE_WINDOW_SZ);
			/* legacy noise is the max of antennas */
			noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
		} else {
			/* all other PHY */
			noise_dbm = PHY_NOISE_FIXED_VAL;
		}

		wait_for_intr = FALSE;
		goto done;
	}

	if (ISLPPHY(pi)) {
		/* CQRM always use interrupt since ccx can issue many requests and
		 * suspend_mac can't finish intime
		 */
		if (!pi->phynoise_polling || (reason == PHY_NOISE_SAMPLE_EXTERNAL)) {
			wlapi_bmac_write_shm(pi->sh->physhim, M_JSSI_0, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_JSSI_1, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP0, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP1, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP2, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP3, 0);

			OR_REG(pi->sh->osh, &pi->regs->maccommand, MCMD_BG_NOISE);
		} else {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_set_deaf_lpphy(pi, (bool)0);
			noise_dbm = (int8)wlc_phy_rx_signal_power_lpphy(pi, 20);
			wlc_phy_clear_deaf_lpphy(pi, (bool)0);
			wlapi_enable_mac(pi->sh->physhim);
			PHY_CAL(("channel = %d noise pwr=%d\n",
				(int)CHSPEC_CHANNEL(pi->radio_chanspec), noise_dbm));
			wait_for_intr = FALSE;
		}
	} else if (ISSSLPNPHY(pi)) {
		noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	} else if (ISLCN40PHY(pi)) {
		/* FIX: TODO */
	} else if (ISLCNPHY(pi)) {
		/* Trigger noise cal but don't adjust anything */
		wlc_lcnphy_noise_measure_start(pi, FALSE);
	} else if (ISNPHY(pi) || ISHTPHY(pi)) {
		if (!pi->phynoise_polling || (reason == PHY_NOISE_SAMPLE_EXTERNAL)) {

			/* ucode assumes these shm locations start with 0
			 * and ucode will not touch them in case of sampling expires
			 */
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP0, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP1, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP2, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP3, 0);

			if (ISHTPHY(pi)) {
				wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP4, 0);
				wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP5, 0);
			}

			OR_REG(pi->sh->osh, &pi->regs->maccommand, MCMD_BG_NOISE);

		} else {

			/* polling mode */
			phy_iq_est_t est[PHY_CORE_MAX];
			uint32 cmplx_pwr[PHY_CORE_MAX];
			int8 noise_dbm_ant[PHY_CORE_MAX];
			uint16 log_num_samps, num_samps, classif_state = 0;
			uint8 wait_time = 32;
			uint8 wait_crs = 0;
			uint8 i;

			bzero((uint8 *)est, sizeof(est));
			bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
			bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));

			/* choose num_samps to be some power of 2 */
			log_num_samps = PHY_NOISE_SAMPLE_LOG_NUM_NPHY;
			num_samps = 1 << log_num_samps;

			/* suspend MAC, get classifier settings, turn it off
			 * get IQ power measurements
			 * restore classifier settings and reenable MAC ASAP
			*/
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

			if (ISNPHY(pi)) {
				classif_state = wlc_phy_classifier_nphy(pi, 0, 0);
				wlc_phy_classifier_nphy(pi, 3, 0);
				wlc_phy_rx_iq_est_nphy(pi, est, num_samps, wait_time, wait_crs);
				wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
					classif_state);
			} else if (ISHTPHY(pi)) {
				classif_state = wlc_phy_classifier_htphy(pi, 0, 0);
				wlc_phy_classifier_htphy(pi, 3, 0);
				wlc_phy_rx_iq_est_htphy(pi, est, num_samps, wait_time, wait_crs);
				wlc_phy_classifier_htphy(pi,
					HTPHY_ClassifierCtrl_classifierSel_MASK, classif_state);
			} else {
				ASSERT(0);
			}

			wlapi_enable_mac(pi->sh->physhim);

			/* sum I and Q powers for each core, average over num_samps */
			FOREACH_CORE(pi, i)
				cmplx_pwr[i] = (est[i].i_pwr + est[i].q_pwr) >> log_num_samps;

			/* pi->phy_noise_win per antenna is updated inside */
			wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, 0);
			wlc_phy_noise_save(pi, noise_dbm_ant, &noise_dbm);

			wait_for_intr = FALSE;
		}
	} else if (ISAPHY(pi)) {
		if (!pi->phynoise_polling || (reason == PHY_NOISE_SAMPLE_EXTERNAL)) {
			/* For A PHY we must set the radio to return RSSI data to RSSI ADC */
			uint16 reg;

			wlc_phyreg_enter(pih);
			wlc_radioreg_enter(pih);
			/* set temp reading delay to zero */
			reg = phy_reg_read(pi, APHY_RSSI_ADC_CTL);
			reg = (reg & 0xF0FF);
			phy_reg_write(pi, APHY_RSSI_ADC_CTL, reg);

			/* grab the default value for this radio */
			reg = read_radio_reg(pi, RADIO_2060WW_RX_SP_REG1);
			wlc_radioreg_exit(pih);
			wlc_phyreg_exit(pih);

			/* save the radio reg default value for ucode
			 * to restore after measurements
			 */
			wlapi_bmac_write_shm(pi->sh->physhim, M_RF_RX_SP_REG1, reg);

			wlapi_bmac_write_shm(pi->sh->physhim, M_JSSI_0, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_JSSI_1, 0);

			OR_REG(pi->sh->osh, &pi->regs->maccommand, MCMD_BG_NOISE);

		} else {
			noise_dbm = wlc_phy_noise_sample_aphy_meas(pi);
			wait_for_intr = FALSE;
		}
	} else if (ISGPHY(pi)) {
		/* B/GPHY */
		if (!pi->phynoise_polling || (reason == PHY_NOISE_SAMPLE_EXTERNAL)) {
			wlapi_bmac_write_shm(pi->sh->physhim, M_JSSI_0, 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_JSSI_1, 0);
			OR_REG(pi->sh->osh, &pi->regs->maccommand, MCMD_BG_NOISE);

		} else {
			noise_dbm = wlc_phy_noise_sample_gphy(pi);
			wait_for_intr = FALSE;
		}
	} else {
		PHY_TRACE(("Fallthru nophy\n"));
		/* No phy ?? */
		return;
	}

done:
	/* if no interrupt scheduled, populate noise results now */
	if (!wait_for_intr)
		wlc_phy_noise_cb(pi, ch, noise_dbm);

}

void
wlc_phy_noise_sample_request_external(wlc_phy_t *pih)
{
	uint8  channel;

	channel = CHSPEC_CHANNEL(wlc_phy_chanspec_get(pih));

	wlc_phy_noise_sample_request(pih, PHY_NOISE_SAMPLE_EXTERNAL, channel);
}

void
wlc_phy_noise_cb(phy_info_t *pi, uint8 channel, int8 noise_dbm)
{
	if (!pi->phynoise_state)
		return;

	PHY_NONE(("wlc_phy_noise_cb: state %d noise %d channel %d\n",
	          pi->phynoise_state, noise_dbm, channel));

	if (pi->phynoise_state & PHY_NOISE_STATE_MON) {
		if (pi->phynoise_chan_watchdog == channel) {
			pi->sh->phy_noise_window[pi->sh->phy_noise_index] = noise_dbm;
			pi->sh->phy_noise_index = MODINC(pi->sh->phy_noise_index, MA_WINDOW_SZ);
		}
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
	}

	if (pi->phynoise_state & PHY_NOISE_STATE_EXTERNAL) {
		pi->phynoise_state &= ~PHY_NOISE_STATE_EXTERNAL;

		wlapi_noise_cb(pi->sh->physhim, channel, noise_dbm);
	}

}

int8
wlc_phy_noise_read_shmem(phy_info_t *pi)
{
	uint32 cmplx_pwr[PHY_CORE_MAX];
	int8 noise_dbm_ant[PHY_CORE_MAX];
	uint16 lo, hi;
	uint32 cmplx_pwr_tot = 0;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	uint8 core;

	ASSERT(pi->pubpi.phy_corenum <= PHY_CORE_MAX);
	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));

	/* read SHM, reuse old corerev PWRIND since we are tight on SHM pool */
	FOREACH_CORE(pi, core) {
		lo = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(2*core));
		hi = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(2*core+1));
		cmplx_pwr[core] = (hi << 16) + lo;
		cmplx_pwr_tot += cmplx_pwr[core];
		if (cmplx_pwr[core] == 0) {
			noise_dbm_ant[core] = PHY_NOISE_FIXED_VAL_NPHY;
		} else
			cmplx_pwr[core] >>= PHY_NOISE_SAMPLE_LOG_NUM_UCODE;
	}

	if (cmplx_pwr_tot == 0)
		PHY_INFORM(("wlc_phy_noise_sample_nphy_compute: timeout in ucode\n"));
	else {
#ifdef NOISE_CAL_LCNXNPHY
		if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 2)) {
			wlc_phy_noise_cal_ocl_enable_disable_nphy(pi, 1);
		}
		if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
		NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 2)) {
			wlc_phy_noisepwr_nphy(pi, cmplx_pwr);
		}
#endif
		wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, 0);
	}

	wlc_phy_noise_save(pi, noise_dbm_ant, &noise_dbm);

	/* legacy noise is the max of antennas */
	return noise_dbm;

}
/* ucode finished phy noise measurement and raised interrupt */
void
wlc_phy_noise_sample_intr(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	int jssi[4];
	uint8 crs_state;
	uint16 jssi_pair, jssi_aux;
	uint8 channel = 0;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;

	if (ISLPPHY(pi)) {
		uint32 cmplx_pwr_tot, cmplx_pwr[PHY_CORE_MAX];
		int8 cmplx_pwr_dbm[PHY_CORE_MAX];
		uint16 lo, hi;
		int32 pwr_offset_dB, gain_dB;
		uint16 status_0, status_1;
		uint16 agc_state, gain_idx;
		uint8 core;

		/* copy of the M_CURCHANNEL, just channel number plus 2G/5G flag */
		jssi_aux = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_AUX);
		channel = jssi_aux & D11_CURCHANNEL_MAX;

		ASSERT(pi->pubpi.phy_corenum < PHY_CORE_MAX);
		bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
		bzero((uint8 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));

		/* read SHM, reuse old corerev PWRIND since we are tight on SHM pool */
		FOREACH_CORE(pi, core) {
			lo = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(2*core));
			hi = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(2*core+1));
			cmplx_pwr[core] = (hi << 16) + lo;
		}

		cmplx_pwr_tot = (cmplx_pwr[0] + cmplx_pwr[1]) >> 6;

		status_0 = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_0);
		if (LPREV_GE(pi->pubpi.phy_rev, 2)) {
			status_0 = ((status_0 & 0xc000) | ((status_0 & 0x3fff)>>1));
		}
		status_1 = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_1);

		agc_state = (status_0 >> 8) & 0xf;
		gain_idx  = (status_0 & 0xff);

		if ((cmplx_pwr_tot > 0) && ((status_0 & 0xc000) == 0x4000) &&
		    /* Filter out weird statuses -- probably hw bug in status reporting */
		    ((agc_state != 1) || (gain_idx >=  20)))
		    {
			/* Measurement finished o.k */
			wlc_phy_compute_dB(cmplx_pwr, cmplx_pwr_dbm, pi->pubpi.phy_corenum);

			pwr_offset_dB = (phy_reg_read(pi, LPPHY_InputPowerDB) & 0xFF);

			if (pwr_offset_dB > 127)
				pwr_offset_dB -= 256;

			/* 20*log10(1)+30-20*log10(512/0.5) ~= -30 */
			noise_dbm += (int8)(pwr_offset_dB - 30);

			if (LPREV_GE(pi->pubpi.phy_rev, 2)) {
				if ((status_0 & 0x0f00) == 0x0100) {
					gain_dB = (phy_reg_read(pi, LPPHY_HiGainDB) & 0xFF);
					noise_dbm -= (int8)(gain_dB);
				} else {
					/* There's a bug in 4325 where the gainIdx is not
					 * output to debug gpio's so unless the AGC is in
					 * HIGH_GAIN_STATE the actual gain can't be
					 * derived from the debug gpio.
					 */
					noise_dbm = -60;
				}
			} else {
				gain_dB = (gain_idx *3) - 6;
				noise_dbm -= (int8)(gain_dB);
			}

			PHY_INFORM(("lpphy noise is %d dBm ch %d(DBG %04x %04x)\n",
				noise_dbm, channel, status_0, status_1));
		} else {
			/* Measurement finished abnormally */
			PHY_INFORM(("lpphy noise measurement error: %04x %04x ch %d\n",
				status_0, status_1, channel));

			noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
		}
	} else if (ISSSLPNPHY(pi)) {
		noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;

	} else if (ISLCNPHY(pi)) {
	  wlc_lcnphy_noise_measure(pi);
	} else if (ISNPHY(pi) || ISHTPHY(pi)) {
		/* copy of the M_CURCHANNEL, just channel number plus 2G/5G flag */
		jssi_aux = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_AUX);
		channel = jssi_aux & D11_CURCHANNEL_MAX;
		noise_dbm = wlc_phy_noise_read_shmem(pi);

	} else if (ISABGPHY(pi)) {

		jssi_pair = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_0);
		jssi[0] = (int)(jssi_pair & 0xff);
		jssi[1] = (int)((jssi_pair >> 8) & 0xff);
		if (ISGPHY(pi)) {
			/* ucode grabs 4 samples for GPhy */
			jssi_pair = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_1);
			jssi[2] = (int)(jssi_pair & 0xff);
			jssi[3] = (int)((jssi_pair >> 8) & 0xff);
		}

		jssi_aux = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_AUX);
		channel = jssi_aux & D11_CURCHANNEL_MAX;

		crs_state = (jssi_aux >> 8) & 0x1f;

		if (ISAPHY(pi)) {
			/* APhy jssi value is signed */
			if (jssi[0] > 127)
				jssi[0] -= 256;

			/* check for a bad reading, ucode puts valid bit in jssi[1] */
			if (!(jssi[1] & 1)) {	/* bad sample, try again */
				PHY_NONE(("wl%d: wlc_phy_noise_int: sample invalid\n",
				          pi->sh->unit));
				/* special failure case for APHY: rerun if hasn't started */
				if (pi->phynoise_state == 0)
					wlc_phy_noise_sample_request(pih, pi->phynoise_state,
						channel);

			} else {
				/* APHY 4306C0s RSSI ADC output needs to be scaled up
				 * to match 4306B0
				 */
				if (AREV_GT(pi->pubpi.phy_rev, 2))
					jssi[0] = jssi[0] * 4;
			}
		}

		PHY_NONE(("wl%d: wlc_phy_noise_int: chan %d crs %d JSSI sample %d\n",
		         pi->sh->unit, channel, crs_state, jssi[0]));

		noise_dbm = wlc_jssi_to_rssi_dbm_abgphy(pi, crs_state, jssi, 4);
	} else {
		PHY_ERROR(("wlc_phy_noise_sample_intr, unsupported phy type\n"));
		ASSERT(0);
	}

	if (!ISLCNPHY(pi))
	  {
	    /* rssi dbm computed, invoke all callbacks */
	    wlc_phy_noise_cb(pi, channel, noise_dbm);
	  }
}

int8
wlc_phy_noise_avg(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int tot = 0;
	int i = 0;
	if (ISSSLPNPHY(pi)) {
		tot = (int)wlc_sslpnphy_noise_avg(pi);
	} else {
		for (i = 0; i < MA_WINDOW_SZ; i++)
			tot += pi->sh->phy_noise_window[i];

		tot /= MA_WINDOW_SZ;
	}

	return (int8)tot;

}

static uint8 nrssi_tbl_phy[PHY_RSSI_TABLE_SIZE] = {
	00,  1,  2,  3,  4,  5,  6,  7,
	8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63
};

int8 lcnphy_gain_index_offset_for_pkt_rssi[] = {
	8,	/* 0 */
	8,	/* 1 */
	8,	/* 2 */
	8,	/* 3 */
	8,	/* 4 */
	8,	/* 5 */
	8,	/* 6 */
	9,	/* 7 */
	10,	/* 8 */
	8,	/* 9 */
	8,	/* 10 */
	7,	/* 11 */
	7,	/* 12 */
	1,	/* 13 */
	2,	/* 14 */
	2,	/* 15 */
	2,	/* 16 */
	2,	/* 17 */
	2,	/* 18 */
	2,	/* 19 */
	2,	/* 20 */
	2,	/* 21 */
	2,	/* 22 */
	2,	/* 23 */
	2,	/* 24 */
	2,	/* 25 */
	2,	/* 26 */
	2,	/* 27 */
	2,	/* 28 */
	2,	/* 29 */
	2,	/* 30 */
	2,	/* 31 */
	1,	/* 32 */
	1,	/* 33 */
	0,	/* 34 */
	0,	/* 35 */
	0,	/* 36 */
	0	/* 37 */
};

void
wlc_phy_compute_dB(uint32 *cmplx_pwr, int8 *p_cmplx_pwr_dB, uint8 core)
{
	uint8 shift_ct, lsb, msb, secondmsb, i;
	uint32 tmp;

	ASSERT(core <= PHY_CORE_MAX);

	PHY_INFORM(("wlc_phy_compute_dB: compute_dB for %d cores\n", core));
	for (i = 0; i < core; i++) {
		tmp = cmplx_pwr[i];
		shift_ct = msb = secondmsb = 0;
		while (tmp != 0) {
			tmp = tmp >> 1;
			shift_ct++;
			lsb = (uint8)(tmp & 1);
			if (lsb == 1)
				msb = shift_ct;
		}
		secondmsb = (uint8)((cmplx_pwr[i] >> (msb - 1)) & 1);
		p_cmplx_pwr_dB[i] = (int8)(3*msb + 2*secondmsb);
		PHY_INFORM(("wlc_phy_compute_dB: p_cmplx_pwr_dB[%d] %d\n", i, p_cmplx_pwr_dB[i]));
	}
}

void BCMFASTPATH
wlc_phy_rssi_compute(wlc_phy_t *pih, void *ctx)
{
	wlc_d11rxhdr_t *wlc_rxhdr = (wlc_d11rxhdr_t *)ctx;
	d11rxhdr_t *rxh = &wlc_rxhdr->rxhdr;
	int rssi = ltoh16(rxh->PhyRxStatus_1) & PRXS1_JSSI_MASK;
	phy_info_t *pi = (phy_info_t *)pih;
	int16 rssi_avg = 0, rssi0_avg = 0, rssi1_avg = 0;
	int8  ctr;
	int8 rssi0, rssi1;
	int board_atten = (ltoh16(rxh->PhyRxStatus_0) >> 11) & 0x1;


	if (NORADIO_ENAB(pi->pubpi)) {
		rssi = WLC_RSSI_INVALID;
		wlc_rxhdr->do_rssi_ma = 1;	/* skip calc rssi MA */
		goto end;
	}
	/* intermediate mpdus in a AMPDU do not have a valid phy status */
	if ((pi->sh->corerev >= 11) && !(ltoh16(rxh->RxStatus2) & RXS_PHYRXST_VALID)) {
		rssi = WLC_RSSI_INVALID;
		wlc_rxhdr->do_rssi_ma = 1;	/* skip calc rssi MA */
		goto end;
	}

	if (ISSSLPNPHY(pi)) {
		rssi = wlc_sslpnphy_rssi_compute(pi, rssi, rxh);
	}

	if (ISLCNCOMMONPHY(pi)) {
		uint8 gidx = (ltoh16(rxh->PhyRxStatus_2) & 0xFC00) >> 10;
		int rssi_slope = wlc_phy_getlcnphy_common(pi)->lcnphy_pkteng_rssi_slope;
		if (rssi > 127)
			rssi -= 256;

		/* RSSI adjustment */
		rssi = rssi + lcnphy_gain_index_offset_for_pkt_rssi[gidx];

		if (board_atten)
			rssi = rssi + pi->rssi_corr_boardatten;
		else
			rssi = rssi + pi->rssi_corr_normal;

		/* temperature compensation */
		rssi = rssi + rssi_slope;
	}

	if (RADIOID(pih->radioid) == BCM2050_ID) {
		if ((ltoh16(rxh->PhyRxStatus_0) & PRXS0_OFDM) == 0) {

			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_ADCDIV) {
				/* Correct for the nrssi slope */
				if (rssi >= PHY_RSSI_TABLE_SIZE)
					rssi = PHY_RSSI_TABLE_SIZE - 1;

				rssi = nrssi_tbl_phy[rssi];
				rssi = (-131 * (31 - rssi) >> 7) - 67;
			} else {
				rssi = (-149 * (31 - rssi) >> 7) - 68;
			}

			if (ISGPHY(pi)) {
				uint16 val;
				/* Only 4306/B0 boards have T/R bit overloaded to also
				 * represent LNA for CCK
				 */
				if (ltoh16(rxh->PhyRxStatus_3) & PRXS3_TRSTATE) {
					rssi += 20;
				}
				val = (ltoh16(rxh->PhyRxStatus_2) & PRXS2_LNAGN_MASK) >>
				        PRXS2_LNAGN_SHIFT;
				switch (val) {
					case 0:	rssi -= -2;	break;
					case 1:	rssi -= 19;	break;
					case 2:	rssi -= 13;	break;
					case 3:	rssi -= 25;	break;
				}
				rssi += 25;
			}
		} else {
			/* Sign extend */
			if (rssi > 127)
				rssi -= 256;
			/* Check T-R/LNA bit */
			/* Adjust the calculated RSSI to match measured RX power
			 * as reported by DVT, to keep reported RSSI within +/-3dB of
			 * measured Rx power. This is needed for 2050 Radio only.
			 */
			if (ltoh16(rxh->PhyRxStatus_3) & PRXS3_TRSTATE)
				rssi += 17;
			else
				rssi -= 4;
		}
	} else if (RADIOID(pih->radioid) == BCM2060_ID) {
		/* Sign extend */
		if (rssi > 127)
			rssi -= 256;
	} else if (ISLPPHY(pi)) {
		/* Sign extend */
		if (rssi > 127)
			rssi -= 256;
	} else if (ISSSLPNPHY(pi)) {
		/* do nothing */
	} else if (ISLCNCOMMONPHY(pi)) {
		/* Sign extend */
		if (rssi > 127)
			rssi -= 256;
	} else if (ISNPHY(pi) &&
	           (RADIOID(pih->radioid) == BCM2055_ID ||
	            RADIOID(pih->radioid) == BCM2056_ID ||
	            RADIOID(pih->radioid) == BCM2057_ID ||
	            RADIOID(pih->radioid) == BCM20671_ID)) {
		ASSERT(ISNPHY(pi));
		rssi = wlc_phy_rssi_compute_nphy(pi, wlc_rxhdr);
	} else if (ISHTPHY(pi)) {
		ASSERT(pih->radioid == BCM2059_ID);
		rssi = wlc_phy_rssi_compute_htphy(pi, wlc_rxhdr);
	} else {
		ASSERT((const char *)"Unknown radio" == NULL);
	}
#if defined(WLNOKIA_NVMEM)
	rssi = wlc_phy_upd_rssi_offset(pi, (int8)rssi, CH20MHZ_CHSPEC(WLC_RX_CHANNEL(rxh)));
#endif 

end:
	wlc_rxhdr->rssi = (int8)rssi;
	if (ISLCNPHY(pi)) {
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			wlc_phy_lcn_updatemac_rssi(pi, (int8)rssi);
	}

	if (ISNPHY(pi) || ISHTPHY(pi)) {
		/* getting moving average of rssi values */
		pi->interf.rssi_buffer[ pi->interf.rssi_index] = (int8) rssi;
		pi->interf.rssi_index++;
		/* average over 10 values/packets */
		pi->interf.rssi_index %= 10;
		rssi_avg = 0;
		for (ctr = 0; ctr < 10; ctr++) {
			rssi_avg += pi->interf.rssi_buffer[ctr];
		}
		rssi_avg /= 10;
		pi->interf.rssi = (int8) rssi_avg;

		/* Compute RSSI for 16 samples per core */
		/* getting moving average of rssi values */
		rssi0 = wlc_rxhdr->rxpwr[0];
		rssi1 = wlc_rxhdr->rxpwr[1];

		pi->rssi0_buffer[pi->rssi0_index] = (int8) rssi0;
		pi->rssi1_buffer[pi->rssi1_index] = (int8) rssi1;
		pi->rssi0_index++;
		pi->rssi1_index++;
		/* average over 16 values/packets */
		pi->rssi1_index %= 16;
		pi->rssi0_index %= 16;
		rssi0_avg = 0;
		rssi1_avg = 0;
		for (ctr = 0; ctr < 16; ctr++) {
			rssi0_avg += pi->rssi0_buffer[ctr];
			rssi1_avg += pi->rssi1_buffer[ctr];
		}
		rssi0_avg /= 16;
		rssi1_avg /= 16;
		pi->rssi0_avg = (int8) rssi0_avg;
		pi->rssi1_avg = (int8) rssi1_avg;

	}
}

/* %%%%%% radar */

void
wlc_phy_radar_detect_enable(wlc_phy_t *pih, bool on)
{
#if defined(AP) && defined(RADAR)
	phy_info_t *pi = (phy_info_t *)pih;

	pi->sh->radar = on;

	if (!pi->sh->up)
		return;

	/* apply radar inits to hardware if we are on the A/LP/N/HTPHY */
	if (ISAPHY(pi) || ISLPPHY(pi) || ISNPHY(pi) || ISHTPHY(pi))
		wlc_phy_radar_detect_init(pi, on);

	return;
#endif /* defined(AP) && defined(RADAR) */
}


int
wlc_phy_radar_detect_run(wlc_phy_t *pih)
{
#if defined(AP) && defined(RADAR)
	phy_info_t *pi = (phy_info_t *)pih;

	if (ISAPHY(pi))
		return wlc_phy_radar_detect_run_aphy(pi);
	else if (ISNPHY(pi) || ISHTPHY(pi))
		return wlc_phy_radar_detect_run_nphy(pi);

	ASSERT(0);
#endif /* defined(AP) && defined(RADAR) */
	return (RADAR_TYPE_NONE);
}

void
wlc_phy_radar_detect_mode_set(wlc_phy_t *pih, phy_radar_detect_mode_t mode)
{
#if defined(AP) && defined(RADAR)
	phy_info_t *pi = (phy_info_t *)pih;

	PHY_TRACE(("wl%d: %s, Radar detect mode set done\n", pi->sh->unit, __FUNCTION__));

	if (pi->rdm == mode)
		return;
	else if ((mode != RADAR_DETECT_MODE_FCC) && (mode != RADAR_DETECT_MODE_EU)) {
		PHY_TRACE(("wl%d: bogus radar detect mode: %d\n", pi->sh->unit, mode));
		return;
	} else
		pi->rdm = mode;

	/* Change radar params based on radar detect mode for
	 * both 20Mhz (index 0) and 40Mhz (index 1) aptly
	 * feature_mask bit-11 is FCC-enable
	 * feature_mask bit-12 is EU-enable
	 */
	if (pi->rdm == RADAR_DETECT_MODE_FCC) {
		/* 20MHz */
		pi->rargs[0].radar_args.feature_mask = (pi->rargs[0].radar_args.feature_mask
			& 0xefff) | 0x800;
		if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
			pi->rargs[0].radar_args.st_level_time = 0x1591;
		} else {
			pi->rargs[0].radar_args.st_level_time = 0x1901;
		}

		/* 40MHz */
		pi->rargs[1].radar_args.feature_mask = (pi->rargs[1].radar_args.feature_mask
			& 0xefff) | 0x800;
		if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
			pi->rargs[1].radar_args.st_level_time = 0x1591;
		} else {
			pi->rargs[1].radar_args.st_level_time = 0x1901;
		}
	} else if (pi->rdm == RADAR_DETECT_MODE_EU) {
		/* 20MHz */
		pi->rargs[0].radar_args.feature_mask = (pi->rargs[0].radar_args.feature_mask
			& 0xf7ff) | 0x1000;
		if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
			pi->rargs[0].radar_args.st_level_time = 0x1591;
		} else {
			pi->rargs[0].radar_args.st_level_time = 0x1901;
		}


		/* 40MHz */
		pi->rargs[1].radar_args.feature_mask = (pi->rargs[1].radar_args.feature_mask
			& 0xf7ff) | 0x1000;
		if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
			pi->rargs[1].radar_args.st_level_time = 0x15c1;
		} else {
			pi->rargs[1].radar_args.st_level_time = 0x1c01;
		}
	}
#endif /* defined(AP) && defined(RADAR) */
}
#if defined(AP) && defined(RADAR)

struct {
	char name[7];
	int  pri;
	int  id;
} radar_class[] = {
	{ "ETSI_1", 28544, RADAR_TYPE_ETSI_1 },
	{ "ETSI_2", 11008, RADAR_TYPE_ETSI_2 },
	{ "ETSI_3", 60544, RADAR_TYPE_ETSI_3 },
	{ "ITU_E",  9984, RADAR_TYPE_ITU_E },
	{ "ITU_K",  6528,  RADAR_TYPE_ITU_K },
	{ "",	    0,     0 }
};

/* Sort vector (of length len) into ascending order */
static void
wlc_shell_sort(int len, int *vector)
{
	int i, j, incr;
	int v;

	incr = 4;

	while (incr < len) {
		incr *= 3;
		incr++;
	}

	while (incr > 1) {
		incr /= 3;
		for (i = incr; i < len; i++) {
			v = vector[i];
			j = i;
			while (vector[j-incr] > v) {
				vector[j] = vector[j-incr];
				j -= incr;
				if (j < incr)
					break;
			}
			vector[j] = v;
		}
	}
}

void
wlc_phy_radar_detect_init(phy_info_t *pi, bool on)
{
	PHY_TRACE(("wl%d: %s, RSSI LUT done\n", pi->sh->unit, __FUNCTION__));

	/* update parameter set */
	pi->rparams = IS20MHZ(pi) ? &pi->rargs[0] : &pi->rargs[1];

	if (ISAPHY(pi)) {
		wlc_phy_radar_detect_init_aphy(pi, on);
		return;
	} else if (ISNPHY(pi) || ISHTPHY(pi)) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (ISHTPHY(pi)) {
				wlc_phy_radar_detect_init_htphy(pi, on);
				wlc_phy_update_radar_detect_param_htphy(pi);
			} else {
				wlc_phy_radar_detect_init_nphy(pi, on);
				wlc_phy_update_radar_detect_param_nphy(pi);
			}
		}
		return;
	} else if (ISHTPHY(pi)) {
		/* still use the nphy init routine in this module */
		if (CHSPEC_IS5G(pi->radio_chanspec))
			wlc_phy_radar_detect_init_nphy(pi, on);
		wlc_phy_update_radar_detect_param_htphy(pi);
		return;
	}

	ASSERT(0);
}

/* read_radar_table */
static void
wlc_phy_radar_read_table(phy_info_t *pi, radar_work_t *rt, int min_pulses)
{
	int i;
	int16 w0, w1, w2;

	/* True? Maximum table size is 42 entries */
	bzero(rt->tstart_list, sizeof(rt->tstart_list));
	bzero(rt->width_list, sizeof(rt->width_list));

	rt->length = phy_reg_read(pi, APHY_RADAR_FIFO_CTL) & 0xff;
	if (rt->length < min_pulses)
		return;

	rt->length /= 3;	/* 3 words per pulse */

	for (i = 0; i < rt->length; i++) {	/* Read out the FIFO */
		w0 = phy_reg_read(pi, APHY_RADAR_FIFO);
		w1 = phy_reg_read(pi, APHY_RADAR_FIFO);
		w2 = phy_reg_read(pi, APHY_RADAR_FIFO);
		rt->tstart_list[i] = ((w0 << 8) + (w1 & 0xff)) << 4;
		rt->width_list[i] = w2 & 0xff;
	}
}

/* generate an n-th tier list (difference between nth pulses) */
static void
wlc_phy_radar_generate_tlist(uint32 *inlist, int *outlist, int length, int n)
{
	int i;

	for (i = 0; i < (length - n); i++) {
		outlist[i] = ABS((int32)(inlist[i + n] - inlist[i]));
	}
}

/* generate an n-th tier pw list */
static void
wlc_phy_radar_generate_tpw(uint32 *inlist, int *outlist, int length, int n)
{
	int i;

	for (i = 0; i < (length - n); i++) {
		outlist[i] = inlist[i + n - 1];
	}
}

/* remove outliers from a list */
static void
wlc_phy_radar_filter_list(int *inlist, int *length, int min_val, int max_val)
{
	int i, j;
	j = 0;
	for (i = 0; i < *length; i++) {
		if ((inlist[i] >= min_val) && (inlist[i] <= max_val)) {
			inlist[j] = inlist[i];
			j++;
		}
	}
	*length = j;
}

/*
 * select_nfrequent - crude for now
 * inlist - input array (tier list) that has been sorted into ascending order
 * length - length of input array
 * n - position of interval value/frequency to return
 * value - interval
 * frequency - number of occurrences of interval value
 * vlist - returned interval list
 * flist - returned frequency list
 */
static int
wlc_phy_radar_select_nfrequent(int *inlist, int length, int n, int *value,
	int *position, int *frequency, int *vlist, int *flist)
{
	/*
	 * needs declarations:
		int vlist[RDR_TIER_SIZE];
		int flist[RDR_TIER_SIZE];
	 * from calling routine
	 */
	int i, j, pointer, counter, newvalue, nlength;
	int plist[RDR_NTIER_SIZE];
	int f, v, p;

	vlist[0] = inlist[0];
	plist[0] = 0;
	flist[0] = 1;

	pointer = 0;
	counter = 0;

	for (i = 1; i < length; i++) {	/* find the frequencies */
		newvalue = inlist[i];
		if (newvalue != vlist[pointer]) {
			pointer++;
			vlist[pointer] = newvalue;
			plist[pointer] = i;
			flist[pointer] = 1;
			counter = 0;
		} else {
			counter++;
			flist[pointer] = counter;
		}
	}

	nlength = pointer + 1;

	for (i = 1; i < nlength; i++) {	/* insertion sort */
		f = flist[i];
		v = vlist[i];
		p = plist[i];
		j = i - 1;
		while ((j >= 0) && flist[j] > f) {
			flist[j + 1] = flist[j];
			vlist[j + 1] = vlist[j];
			plist[j + 1] = plist[j];
			j--;
		}
		flist[j + 1] = f;
		vlist[j + 1] = v;
		plist[j + 1] = p;
	}

	if (n < nlength) {
		*value = vlist[nlength - n - 1];
		*position = plist[nlength - n - 1];
		*frequency = flist[nlength - n - 1] + 1;
	} else {
		*value = 0;
		*position = 0;
		*frequency = 0;
	}
	return nlength;
}

/* radar detection */
static void
BCMATTACHFN(wlc_phy_radar_attach_nphy)(phy_info_t *pi)
{
	/* 20Mhz channel radar thresholds */
	if (ISHTPHY(pi)) {
		pi->rargs[0].radar_thrs.thresh0_20_lo = 0x6c8;
		pi->rargs[0].radar_thrs.thresh1_20_lo = 0x30;
		pi->rargs[0].radar_thrs.thresh0_20_hi = 0x6b8;
		pi->rargs[0].radar_thrs.thresh1_20_hi = 0x30;
	} else if (NREV_GE(pi->pubpi.phy_rev, 7)) {
		pi->rargs[0].radar_thrs.thresh0_20_lo = 0x6c8;
		pi->rargs[0].radar_thrs.thresh1_20_lo = 0x30;
		pi->rargs[0].radar_thrs.thresh0_20_hi = 0x6b8;
		pi->rargs[0].radar_thrs.thresh1_20_hi = 0x30;
	} else if (NREV_GE(pi->pubpi.phy_rev, 3)) {
		pi->rargs[0].radar_thrs.thresh0_20_lo = 0x698;
		pi->rargs[0].radar_thrs.thresh1_20_lo = 0x6c8;
		pi->rargs[0].radar_thrs.thresh0_20_hi = 0x6a0;
		pi->rargs[0].radar_thrs.thresh1_20_hi = 0x6d0;
	} else {
		pi->rargs[0].radar_thrs.thresh0_20_lo = 0x6a8;
		pi->rargs[0].radar_thrs.thresh1_20_lo = 0x6d0;
		pi->rargs[0].radar_thrs.thresh0_20_hi = 0x69c;
		pi->rargs[0].radar_thrs.thresh1_20_hi = 0x6c6;
	}

	/* 20Mhz channel radar params */
	pi->rargs[0].min_tint = 3000;  /* 0.15ms (6.67 kHz) */
	pi->rargs[0].max_tint = 120000; /* 6ms (167 Hz) - for Finland */
	pi->rargs[0].min_blen = 100000;
	pi->rargs[0].max_blen = 1500000; /* 75 ms */
	pi->rargs[0].sdepth_extra_pulses = 2;
	pi->rargs[0].min_deltat_lp = 19000; /* 1e-3*20e6 - small error */
	pi->rargs[0].max_deltat_lp = 130000;  /* 2*2e-3*20e6 + small error */
	pi->rargs[0].max_type1_pw = 35;	  /* fcc type1 1*20 + 15 */
	pi->rargs[0].jp2_1_intv = 27780;  /* jp-2-1 1389*20 */
	pi->rargs[0].type1_intv = 28571;  /* fcc type 1 1428.57*20 */
	pi->rargs[0].max_jp1_2_pw = 70;	  /* jp-1-2 2.5*20+20 */
	pi->rargs[0].jp1_2_intv = 76923;  /* jp-1-2 3846.15*20 */
	pi->rargs[0].jp2_3_intv = 80000;  /* jp-2-3 4000*20 */
	pi->rargs[0].max_type2_pw = 150;  /* fcc type 2, 5*20 + 50 */
	pi->rargs[0].min_type2_intv = 3000;
	pi->rargs[0].max_type2_intv = 4600;
	pi->rargs[0].max_type4_pw = 460;  /* fcc type 4, 20*20 + 60 */
	pi->rargs[0].min_type3_4_intv = 4000;
	pi->rargs[0].max_type3_4_intv = 10000;
	pi->rargs[0].radar_args.min_burst_intv_lp = 20000000;
	pi->rargs[0].radar_args.max_burst_intv_lp = 70000000;
	pi->rargs[0].radar_args.nskip_rst_lp = 2;
	pi->rargs[0].radar_args.quant = 128;
	pi->rargs[0].radar_args.npulses = 5;
	pi->rargs[0].radar_args.ncontig = 35;
	pi->rargs[0].radar_args.max_pw = 690;  /* 30us + 15% */
	pi->rargs[0].radar_args.thresh0 = pi->rargs[0].radar_thrs.thresh0_20_lo;
	pi->rargs[0].radar_args.thresh1 = pi->rargs[0].radar_thrs.thresh1_20_lo;
	pi->rargs[0].radar_args.fmdemodcfg = 0x7f09;
	pi->rargs[0].radar_args.autocorr = 0x1e;
	if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
		pi->rargs[0].radar_args.st_level_time = 0x1591;
		pi->rargs[0].radar_args.min_pw = 6;
		pi->rargs[0].radar_args.max_pw_tol = 6;
	} else {
		pi->rargs[0].radar_args.st_level_time = 0x1901;
		pi->rargs[0].radar_args.min_pw = 1;
		pi->rargs[0].radar_args.max_pw_tol = 4;
	}
	pi->rargs[0].radar_args.t2_min = 31552;	/* 0x7b40 */
	pi->rargs[0].radar_args.npulses_lp = 11;
	pi->rargs[0].radar_args.min_pw_lp = 180;
	pi->rargs[0].radar_args.max_pw_lp = 2000;
	pi->rargs[0].radar_args.min_fm_lp = 45;
	pi->rargs[0].radar_args.max_span_lp = 63568;   /* 0xf850; 15, 8, 80 */
	pi->rargs[0].radar_args.min_deltat = 2000;
	pi->rargs[0].radar_args.max_deltat = 3000000;
	pi->rargs[0].radar_args.version = WL_RADAR_ARGS_VERSION;
	pi->rargs[0].radar_args.fra_pulse_err = 10;
	pi->rargs[0].radar_args.npulses_fra = 3;
	pi->rargs[0].radar_args.npulses_stg2 = 4;
	pi->rargs[0].radar_args.npulses_stg3 = 5;
	pi->rargs[0].radar_args.percal_mask = 0x11;
	pi->rargs[0].radar_args.feature_mask = 0xa800;
	if (NREV_GE(pi->pubpi.phy_rev, 3) || ISHTPHY(pi)) {
		pi->rargs[0].radar_args.blank = 0x6419;
	} else {
		pi->rargs[0].radar_args.blank = 0x2c19;
	}

	/* 40Mhz channel radar thresholds */
	if (ISHTPHY(pi)) {
		pi->rargs[1].radar_thrs.thresh0_40_lo = 0x6c8;
		pi->rargs[1].radar_thrs.thresh1_40_lo = 0x30;
		pi->rargs[1].radar_thrs.thresh0_40_hi = 0x6b8;
		pi->rargs[1].radar_thrs.thresh1_40_hi = 0x30;
	} else if (NREV_GE(pi->pubpi.phy_rev, 7)) {
		pi->rargs[1].radar_thrs.thresh0_40_lo = 0x6c8;
		pi->rargs[1].radar_thrs.thresh1_40_lo = 0x30;
		pi->rargs[1].radar_thrs.thresh0_40_hi = 0x6b8;
		pi->rargs[1].radar_thrs.thresh1_40_hi = 0x30;
	} else if (NREV_GE(pi->pubpi.phy_rev, 3)) {
		pi->rargs[1].radar_thrs.thresh0_40_lo = 0x698;
		pi->rargs[1].radar_thrs.thresh1_40_lo = 0x6c8;
		pi->rargs[1].radar_thrs.thresh0_40_hi = 0x6a0;
		pi->rargs[1].radar_thrs.thresh1_40_hi = 0x6d0;
	} else {
		pi->rargs[1].radar_thrs.thresh0_40_lo = 0x6b8;
		pi->rargs[1].radar_thrs.thresh1_40_lo = 0x6e0;
		pi->rargs[1].radar_thrs.thresh0_40_hi = 0x6b8;
		pi->rargs[1].radar_thrs.thresh1_40_hi = 0x6e0;
	}

	/* 40Mhz channel radar params */
	pi->rargs[1].min_tint = 6000;
	pi->rargs[1].max_tint = 240000;
	pi->rargs[1].min_blen = 200000;
	pi->rargs[1].max_blen = 3000000;
	pi->rargs[1].sdepth_extra_pulses = 2;
	pi->rargs[1].min_deltat_lp = 38000;
	pi->rargs[1].max_deltat_lp = 260000;
	pi->rargs[1].max_type1_pw = 70;		/* fcc type 1 1*40 + 30 */
	pi->rargs[1].jp2_1_intv = 55560;	/* jp-2-1 1389*40 */
	pi->rargs[1].type1_intv = 57143;	/* fcc type 1 1428.57*40 */
	pi->rargs[1].max_jp1_2_pw = 140;	/* jp-1-2 2.5*40+40 */
	pi->rargs[1].jp1_2_intv = 153846;	/* jp-1-2 3846.15*40 */
	pi->rargs[1].jp2_3_intv = 160000;	/* jp-2-3 4000*40 */
	pi->rargs[1].max_type2_pw = 300;    /* fcc type 2, 5*40 + 100 */
	pi->rargs[1].min_type2_intv = 6000;
	pi->rargs[1].max_type2_intv = 9200;
	pi->rargs[1].max_type4_pw = 920;	 /* fcc type 4, 20*40 + 120 */
	pi->rargs[1].min_type3_4_intv = 8000;
	pi->rargs[1].max_type3_4_intv = 20000;
	pi->rargs[1].radar_args.min_burst_intv_lp =  40000000;
	pi->rargs[1].radar_args.max_burst_intv_lp = 140000000;
	pi->rargs[1].radar_args.nskip_rst_lp = 2;
	pi->rargs[1].radar_args.quant = 256;
	pi->rargs[1].radar_args.npulses = 5;
	pi->rargs[1].radar_args.ncontig = 35;
	pi->rargs[1].radar_args.max_pw = 1380; /* 30us + 15% */
	pi->rargs[1].radar_args.thresh0 = pi->rargs[1].radar_thrs.thresh0_40_lo;
	pi->rargs[1].radar_args.thresh1 = pi->rargs[1].radar_thrs.thresh1_40_lo;
	pi->rargs[1].radar_args.fmdemodcfg = 0x7f09;
	pi->rargs[1].radar_args.autocorr = 0x1e;
	if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
		pi->rargs[1].radar_args.st_level_time = 0x1591;
		pi->rargs[1].radar_args.min_pw = 12;
		pi->rargs[1].radar_args.max_pw_tol = 12;
	} else {
		pi->rargs[1].radar_args.st_level_time = 0x1901;
		pi->rargs[1].radar_args.min_pw = 1;
		pi->rargs[1].radar_args.max_pw_tol = 24;
	}
	pi->rargs[1].radar_args.t2_min = 31616;	/* 0x7b80 */
	pi->rargs[1].radar_args.npulses_lp = 11;
	pi->rargs[1].radar_args.min_pw_lp = 360;
	pi->rargs[1].radar_args.max_pw_lp = 4000;
	pi->rargs[1].radar_args.min_fm_lp = 45;
	pi->rargs[1].radar_args.max_span_lp = 63648;   /* 0xf8a0; 15, 8, 160 */
	pi->rargs[1].radar_args.min_deltat = 4000; /* determined by max prf */
	pi->rargs[1].radar_args.max_deltat = 6000000; /* how often radar is being probed (150 ms) */
	pi->rargs[1].radar_args.version = WL_RADAR_ARGS_VERSION;
	pi->rargs[1].radar_args.fra_pulse_err = 20;
	pi->rargs[1].radar_args.npulses_fra = 3;
	pi->rargs[1].radar_args.npulses_stg2 = 4;
	pi->rargs[1].radar_args.npulses_stg3 = 5;
	pi->rargs[1].radar_args.percal_mask = 0x11;
	pi->rargs[1].radar_args.feature_mask = 0xa800;
	if (NREV_GE(pi->pubpi.phy_rev, 3) || ISHTPHY(pi)) {
		pi->rargs[1].radar_args.blank = 0x6432;
	} else {
		pi->rargs[1].radar_args.blank = 0x2c19;
	}
	pi->rparams = IS20MHZ(pi) ? &pi->rargs[0] : &pi->rargs[1];
}

static void
wlc_phy_radar_detect_init_aphy(phy_info_t *pi, bool on)
{
	ASSERT(AREV_GE(pi->pubpi.phy_rev, 3));

	if (on) {
		/* WRITE_APHY_TABLE_ENT(pi, 0xf, 0xd, 0xff); */

		/* empirically refined Radar detect thresh1 */
		phy_reg_write(pi, APHY_RADAR_THRESH1, pi->rparams->radar_args.thresh1);

		/* Requested by dboldy 8/17/04 */
		phy_reg_write(pi, APHY_RADAR_THRESH0, pi->rparams->radar_args.thresh0);

		wlapi_bmac_write_shm(pi->sh->physhim, M_RADAR_REG, pi->rparams->radar_args.thresh1);
	}

	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_RADARWAR, (on ? MHF1_RADARWAR : 0),
		WLC_BAND_5G);
}

static int
wlc_phy_radar_detect_run_aphy(phy_info_t *pi)
{
	int i, j;
	int tiern_length[RDR_NTIERS];
	int value[RDR_NTIERS], freq[RDR_NTIERS], pos[RDR_NTIERS];
	uint32 *epoch_list;
	int epoch_length = 0;
	int epoch_detected;
	int pulse_interval;
	bool filter_pw = TRUE;
	radar_work_t *rt = &pi->radar_work;
	radar_params_t *rparams = pi->rparams;
	int val_list[RDR_TIER_SIZE];
	int freq_list[RDR_TIER_SIZE];

	if (!pi->rparams->radar_args.npulses) {
		PHY_ERROR(("radar params not initialized\n"));
		return (RADAR_TYPE_NONE);
	}

	pulse_interval = 0;
	wlc_phy_radar_read_table(pi, rt, rparams->radar_args.npulses);

	/*
	 * Reject if too few pulses recorded
	 */
	if (rt->length < rparams->radar_args.npulses) {
		return (RADAR_TYPE_NONE);
	}

	/*
	 * filter based on pulse width
	 */
	if (filter_pw) {
		j = 0;
		for (i = 0; i < rt->length; i++) {
			if ((rt->width_list[i] >= rparams->radar_args.min_pw) &&
				(rt->width_list[i] <= rparams->radar_args.max_pw)) {
				rt->width_list[j] = rt->width_list[i];
				rt->tstart_list[j] = rt->tstart_list[i];
				j++;
			}
		}
		rt->length = j;
	}
	ASSERT(rt->length <= 2*RDR_LIST_SIZE);

	/*
	 * Break pulses into epochs.
	 */
	rt->nepochs = 1;
	rt->epoch_start[0] = 0;
	for (i = 1; i < rt->length; i++) {
		if ((int32)(rt->tstart_list[i] - rt->tstart_list[i-1]) > (int32)rparams->max_blen) {
			rt->epoch_finish[rt->nepochs-1] = i - 1;
			rt->epoch_start[rt->nepochs] = i;
			rt->nepochs++;
		}
	}
	rt->epoch_finish[rt->nepochs - 1] = i;

	/*
	 * Run the detector for each epoch
	 */
	for (i = 0; i < rt->nepochs && !pulse_interval; i++) {
		/*
		 * Generate 0th order tier list (time delta between received pulses)
		 * Quantize and filter delta pulse times delta pulse times are
		 * returned in sorted order from smallest to largest.
		 */

		epoch_list = rt->tstart_list + rt->epoch_start[i];
		epoch_length = rt->epoch_finish[i] - rt->epoch_start[i] + 1;
		if (epoch_length >= RDR_TIER_SIZE) {
			PHY_RADAR(("WARNINIG: EPOCH LENGTH = %d EXCEEDED TIER_SIZE = %d !!\n",
				epoch_length, RDR_TIER_SIZE));
			return (RADAR_TYPE_NONE);
		}
		bzero(rt->tiern_list[0], sizeof(rt->tiern_list[0]));

		/*
		 * generate lists
		 */
		wlc_phy_radar_generate_tlist(epoch_list, rt->tiern_list[0], epoch_length, 1);
		for (j = 0; j < epoch_length; j++) {			/* quantize */
			rt->tiern_list[0][j] = rparams->radar_args.quant * ((rt->tiern_list[0][j] +
			(rparams->radar_args.quant >> 1)) / rparams->radar_args.quant);
		}
		tiern_length[0] = epoch_length;
		wlc_phy_radar_filter_list(rt->tiern_list[0], &(tiern_length[0]), rparams->min_tint,
		                rparams->max_tint);
		wlc_shell_sort(tiern_length[0], rt->tiern_list[0]);	/* sort */

		/*
		 * Detection
		 */

		/* Reject out of hand if the number of filtered pulses is too low */
		if (tiern_length[0] < rparams->radar_args.npulses) {
			continue;
		}

		/* measure most common pulse interval */
		wlc_phy_radar_select_nfrequent(rt->tiern_list[0], tiern_length[0], 0,
			&value[0], &pos[0], &freq[0], val_list, freq_list);

		if (freq[0] >= rparams->radar_args.npulses) {
			/* Paydirt: Equal spaced pulses, no gaps */
			pulse_interval = value[0];
			continue;
		}

		if (freq[0] < rparams->radar_args.ncontig) {
			continue;
		}

		/* Possible match - look for gaps */
		/* Check 2nd most frequent interval only on lowest tier */
		wlc_phy_radar_select_nfrequent(rt->tiern_list[0], tiern_length[0], 1,
			&value[1], &pos[1], &freq[1], val_list, freq_list);
		if ((value[1] == (2 * value[0])) && ((freq[0] + freq[1]) >=
			rparams->radar_args.npulses)) {
			/* twice the interval */
			pulse_interval = value[1];
			continue;
		}

		if ((value[0] == (2 * value[1])) && ((freq[0] + freq[1]) >=
			rparams->radar_args.npulses)) {
			/* half the interval */
			pulse_interval = value[1];
			continue;
		}

		/*
		 * Look for extra pulses
		 */

		/* Generate 2nd lowest tier */
		if (0) {	/* DCL this part is not functional, take out */
					/* to save memory and have RDR_NTIERS=1 */
		bzero(rt->tiern_list[1], sizeof(rt->tiern_list[1]));
		/* generate lists */
		wlc_phy_radar_generate_tlist(epoch_list, rt->tiern_list[1], epoch_length, 2);
		for (j = 0; j < epoch_length; j++) {	/* quantize */
			rt->tiern_list[1][j] = rparams->radar_args.quant * ((rt->tiern_list[1][j] +
				(rparams->radar_args.quant >> 1)) / rparams->radar_args.quant);
		}
		tiern_length[1] = epoch_length;
		wlc_phy_radar_filter_list(rt->tiern_list[1], &(tiern_length[1]),
			rparams->min_tint * 2, rparams->max_tint * 2);
		wlc_shell_sort(tiern_length[1], rt->tiern_list[1]);	/* sort */

		/* Look over multiple frequencies on 2nd lowest tier */
		for (j = 1; (j < RDR_SDEPTH_EXTRA_PULSES) && (j < tiern_length[1]); j++) {
			wlc_phy_radar_select_nfrequent(rt->tiern_list[1], tiern_length[1], j,
				&value[1], &pos[1], &freq[1], val_list, freq_list);
			if ((value[0] == value[1]) && ((freq[0]+freq[1]) >=
			                               rparams->radar_args.npulses)) {
				pulse_interval = value[1];
				continue;
			}
		}
		}
	}

	epoch_detected = i;

	if (pulse_interval) {
		int radar_type = RADAR_TYPE_UNCLASSIFIED;
		const char *radar_type_str = "UNCLASSIFIED";

		for (i = 0; i < radar_class[i].id; i++) {
			if (pulse_interval == radar_class[i].pri) {
				radar_type = radar_class[i].id;
				radar_type_str = (char *)radar_class[i].name;
				break;
			}
		}

		PHY_RADAR(("radar : %s Pulse Interval %d\n", radar_type_str, pulse_interval));

		PHY_RADAR(("Pulse Widths:  "));
		for (i = 0; i < rt->length; i++) {
			PHY_RADAR(("%i ", rt->width_list[i]));
		}
		PHY_RADAR(("\n"));

		PHY_RADAR(("Start Time:  "));
		for (i = 0; i < rt->length; i++) {
			PHY_RADAR(("%i ", rt->tstart_list[i]));
		}
		PHY_RADAR(("\n"));

		PHY_RADAR(("Epoch : nepochs %d, length %d detected %d",
		          rt->nepochs, epoch_length, epoch_detected));

		return (radar_type);
	}

	return (RADAR_TYPE_NONE);
}

static void
wlc_phy_radar_detect_init_nphy(phy_info_t *pi, bool on)
{
	if (on) {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) <= WL_THRESHOLD_LO_BAND) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_40_lo;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_40_lo;
			} else {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_20_lo;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_20_lo;
			}
		} else {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_40_hi;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_40_hi;
			} else {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_20_hi;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_20_hi;
			}
		}
		if (NREV_LT(pi->pubpi.phy_rev, 2))
			phy_reg_write(pi, NPHY_RadarBlankCtrl,
			              pi->rparams->radar_args.blank);
		else
			phy_reg_write(pi, NPHY_RadarBlankCtrl,
			              (pi->rparams->radar_args.blank | (0x0000)));

		if (NREV_LT(pi->pubpi.phy_rev, 3)) {
			phy_reg_write(pi, NPHY_RadarThresh0, pi->rparams->radar_args.thresh0);
			phy_reg_write(pi, NPHY_RadarThresh1, pi->rparams->radar_args.thresh1);
		} else {
			phy_reg_write(pi, NPHY_RadarThresh0,
				(uint16)((int16)pi->rparams->radar_args.thresh0));
			phy_reg_write(pi, NPHY_RadarThresh1,
				(uint16)((int16)pi->rparams->radar_args.thresh1));
/*
			phy_reg_write(pi, NPHY_Radar_t2_min, pi->rparams->radar_args.t2_min);
*/
			phy_reg_write(pi, NPHY_Radar_t2_min, 0);
		}

		phy_reg_write(pi, NPHY_StrAddress2u, pi->rparams->radar_args.st_level_time);
		phy_reg_write(pi, NPHY_StrAddress2l, pi->rparams->radar_args.st_level_time);
		phy_reg_write(pi, NPHY_crsControlu, (uint8)pi->rparams->radar_args.autocorr);
		phy_reg_write(pi, NPHY_crsControll, (uint8)pi->rparams->radar_args.autocorr);
		phy_reg_write(pi, NPHY_FMDemodConfig, pi->rparams->radar_args.fmdemodcfg);

		wlapi_bmac_write_shm(pi->sh->physhim, M_RADAR_REG, pi->rparams->radar_args.thresh1);

		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, RadarThresh0R, 0x7a8)
				PHY_REG_WRITE_ENTRY(NPHY, RadarThresh1R, 0x7d0)
			PHY_REG_LIST_EXECUTE(pi);
		} else if (!(NREV_GE(pi->pubpi.phy_rev, 7)) && !(ISHTPHY(pi))) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, RadarThresh0R, 0x7e8)
				PHY_REG_WRITE_ENTRY(NPHY, RadarThresh1R, 0x10)
			PHY_REG_LIST_EXECUTE(pi);
		}

		/* percal_mask to disable radar detection during selected period cals */
		pi->radar_percal_mask = pi->rparams->radar_args.percal_mask;
		if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
			PHY_REG_LIST_START
				PHY_REG_WRITE_ENTRY(NPHY, RadarSearchCtrl, 1)
				PHY_REG_WRITE_ENTRY(NPHY, RadarDetectConfig1, 0x3204)
				PHY_REG_WRITE_ENTRY(NPHY, RadarT3BelowMin, 0)
			PHY_REG_LIST_EXECUTE(pi);
		} else {
			/* Set radar frame search modes */
			phy_reg_write(pi, NPHY_RadarSearchCtrl, 7);
		}
	}

	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_RADARWAR, (on ? MHF1_RADARWAR : 0), FALSE);
}

static void
wlc_phy_radar_detect_init_htphy(phy_info_t *pi, bool on)
{
	if (on) {
		if (CHSPEC_CHANNEL(pi->radio_chanspec) <= WL_THRESHOLD_LO_BAND) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_40_lo;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_40_lo;
			} else {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_20_lo;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_20_lo;
			}
		} else {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_40_hi;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_40_hi;
			} else {
				pi->rparams->radar_args.thresh0 =
					pi->rparams->radar_thrs.thresh0_20_hi;
				pi->rparams->radar_args.thresh1 =
					pi->rparams->radar_thrs.thresh1_20_hi;
			}
		}
		phy_reg_write(pi, HTPHY_RadarBlankCtrl,
			(pi->rparams->radar_args.blank | (0x0000)));

		phy_reg_write(pi, HTPHY_RadarThresh0,
			(uint16)((int16)pi->rparams->radar_args.thresh0));
		phy_reg_write(pi, HTPHY_RadarThresh1,
			(uint16)((int16)pi->rparams->radar_args.thresh1));
/*
		phy_reg_write(pi, HTPHY_Radar_t2_min, pi->rparams->radar_args.t2_min);
*/
		phy_reg_write(pi, HTPHY_Radar_t2_min, 0);

		phy_reg_write(pi, HTPHY_StrAddress2u, pi->rparams->radar_args.st_level_time);
		phy_reg_write(pi, HTPHY_StrAddress2l, pi->rparams->radar_args.st_level_time);
		phy_reg_write(pi, HTPHY_crsControlu, (uint8)pi->rparams->radar_args.autocorr);
		phy_reg_write(pi, HTPHY_crsControll, (uint8)pi->rparams->radar_args.autocorr);
		phy_reg_write(pi, HTPHY_FMDemodConfig, pi->rparams->radar_args.fmdemodcfg);

		wlapi_bmac_write_shm(pi->sh->physhim, M_RADAR_REG, pi->rparams->radar_args.thresh1);

		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(HTPHY, RadarThresh0R, 0x7a8)
			PHY_REG_WRITE_ENTRY(HTPHY, RadarThresh1R, 0x7d0)
		PHY_REG_LIST_EXECUTE(pi);

		/* percal_mask to disable radar detection during selected period cals */
		pi->radar_percal_mask = pi->rparams->radar_args.percal_mask;

		PHY_REG_LIST_START
			PHY_REG_WRITE_ENTRY(HTPHY, RadarSearchCtrl, 1)
			PHY_REG_WRITE_ENTRY(HTPHY, RadarDetectConfig1, 0x3204)
			PHY_REG_WRITE_ENTRY(HTPHY, RadarT3BelowMin, 0)
		PHY_REG_LIST_EXECUTE(pi);
	}

	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_RADARWAR, (on ? MHF1_RADARWAR : 0), FALSE);
}

static void
wlc_phy_radar_read_table_nphy(phy_info_t *pi, radar_work_t *rt, int min_pulses)
{
	int i;
	uint16 w0, w1, w2;
	int max_fifo_size = 255;
	radar_params_t *rparams = pi->rparams;

	if (NREV_GE(pi->pubpi.phy_rev, 3))
		max_fifo_size = 510;

	/* True? Maximum table size is 85 entries for .11n */
	/* Format is different from earlier .11a PHYs */
	bzero(rt->tstart_list_n, sizeof(rt->tstart_list_n));
	bzero(rt->width_list_n, sizeof(rt->width_list_n));
	bzero(rt->fm_list_n, sizeof(rt->fm_list_n));

	if (NREV_GE(pi->pubpi.phy_rev, 3)) {
		rt->nphy_length[0] = phy_reg_read(pi, NPHY_Antenna0_radarFifoCtrl) & 0x3ff;
		rt->nphy_length[1] = phy_reg_read(pi, NPHY_Antenna1_radarFifoCtrl) & 0x3ff;
	} else {
		rt->nphy_length[0] = phy_reg_read(pi, NPHY_Antenna0_radarFifoCtrl) & 0x1ff;
		rt->nphy_length[1] = phy_reg_read(pi, NPHY_Antenna1_radarFifoCtrl) & 0x1ff;
	}
	/* Rev 3: nphy_length is <=510 because words are read/written in multiples of 3 */

	if (rt->nphy_length[0]  > max_fifo_size) {
		PHY_RADAR(("FIFO LENGTH in ant 0 is greater than max_fifo_size\n"));
		rt->nphy_length[0]  = 0;
	}

	if (rt->nphy_length[1]  > max_fifo_size) {
		PHY_RADAR(("FIFO LENGTH in ant 1 is greater than max_fifo_size\n"));
		rt->nphy_length[1]  = 0;
	}

	/* enable pulses received at each antenna messages if feature_mask bit-2 is set */
	if (rparams->radar_args.feature_mask & 0x4) {
		if ((rt->nphy_length[0] > 5) || (rt->nphy_length[1] > 5))
			PHY_RADAR(("ant 0:%d ant 1:%d\n", rt->nphy_length[0], rt->nphy_length[1]));
	}

	rt->nphy_length[0] /= 3;	/* 3 words per pulse */

	/* use the last sample for bin5 */
	rt->tstart_list_bin5[0][0] = rt->tstart_list_tail[0];
	rt->width_list_bin5[0][0] = rt->width_list_tail[0];
	rt->fm_list_n[0][0] = rt->fm_list_tail[0];
	for (i = 0; i < rt->nphy_length[0]; i++) {
		/* Read out FIFO 0 */
		w0 = phy_reg_read(pi, NPHY_Antenna0_radarFifoData);
		w1 = phy_reg_read(pi, NPHY_Antenna0_radarFifoData);
		w2 = phy_reg_read(pi, NPHY_Antenna0_radarFifoData);
		/* USE ONLY 255 of 511 FIFO DATA if feature_mask bit-15 set */
		if ((i < 128 && (rparams->radar_args.feature_mask & 0x8000)) ||
			((rparams->radar_args.feature_mask & 0x8000) == 0)) {
			rt->tstart_list_n[0][i] = (uint32) (((w0 << 12) + (w1 & 0x0fff)) << 4);
			rt->tstart_list_bin5[0][i+1] = rt->tstart_list_n[0][i];
			rt->width_list_n[0][i] = ((w2 & 0x00ff) << 4) + ((w1 >> 12) & 0x000f);
			rt->width_list_bin5[0][i+1] = rt->width_list_n[0][i];
			rt->fm_list_n[0][i+1] = (w2 >> 8) & 0x00ff;
		}
	}
	/* save the last (tail) sample */
	rt->tstart_list_tail[0] = rt->tstart_list_bin5[0][i+1];
	rt->width_list_tail[0] = rt->width_list_bin5[0][i+1];
	rt->fm_list_tail[0] = rt->fm_list_n[0][i+1];

	if (rt->nphy_length[0] > 128 && (rparams->radar_args.feature_mask & 0x8000))
		rt->nphy_length[0] = 128;
	rt->nphy_length_bin5[0] = rt->nphy_length[0] + 1;

	rt->nphy_length[1] /= 3;	/* 3 words per pulse */

	/* use the last sample for start of bin5 */
	rt->tstart_list_bin5[1][0] = rt->tstart_list_tail[1];
	rt->width_list_bin5[1][0] = rt->width_list_tail[1];
	rt->fm_list_n[1][0] = rt->fm_list_tail[1];
	for (i = 0; i < rt->nphy_length[1]; i++) {	/* Read out FIFO 0 */
		w0 = phy_reg_read(pi, NPHY_Antenna1_radarFifoData);
		w1 = phy_reg_read(pi, NPHY_Antenna1_radarFifoData);
		w2 = phy_reg_read(pi, NPHY_Antenna1_radarFifoData);
		/* USE ONLY 255 of 511 FIFO DATA if feature_mask bit-15 set */
		if ((i < 128 && (rparams->radar_args.feature_mask & 0x8000)) ||
			((rparams->radar_args.feature_mask & 0x8000) == 0)) {
			rt->tstart_list_n[1][i] = (uint32) (((w0 << 12) + (w1 & 0x0fff)) << 4);
			rt->tstart_list_bin5[1][i+1] = rt->tstart_list_n[1][i];
			rt->width_list_n[1][i] = ((w2 & 0x00ff) << 4) + ((w1 >> 12) & 0x000f);
			rt->width_list_bin5[1][i+1] = rt->width_list_n[1][i];
			rt->fm_list_n[1][i+1] = (w2 >> 8) & 0x00ff;
		}
	}
	/* save the last (tail) sample */
	rt->tstart_list_tail[1] = rt->tstart_list_bin5[1][i+1];
	rt->width_list_tail[1] = rt->width_list_bin5[1][i+1];
	rt->fm_list_tail[1] = rt->fm_list_n[1][i+1];

	if (rt->nphy_length[1] > 128 && (rparams->radar_args.feature_mask & 0x8000))
		rt->nphy_length[1] = 128;
	rt->nphy_length_bin5[1] = rt->nphy_length[1] + 1;
}

static void
wlc_phy_radar_read_table_nphy_rev7(phy_info_t *pi, radar_work_t *rt, int min_pulses)
{
	int i;
	uint16 w0, w1, w2, w3;
	int max_fifo_size = 512;
	radar_params_t *rparams = pi->rparams;

	/* Maximum table size is 128 entries for .11n rev7 forward */
	bzero(rt->tstart_list_n, sizeof(rt->tstart_list_n));
	bzero(rt->width_list_n, sizeof(rt->width_list_n));
	bzero(rt->fm_list_n, sizeof(rt->fm_list_n));

	if (ISHTPHY(pi)) {
		rt->nphy_length[0] = phy_reg_read(pi, HTPHY_Antenna0_radarFifoCtrl) & 0x3ff;
		rt->nphy_length[1] = phy_reg_read(pi, HTPHY_Antenna1_radarFifoCtrl) & 0x3ff;
	} else {
		rt->nphy_length[0] = phy_reg_read(pi, NPHY_Antenna0_radarFifoCtrl) & 0x3ff;
		rt->nphy_length[1] = phy_reg_read(pi, NPHY_Antenna1_radarFifoCtrl) & 0x3ff;
	}

	if (rt->nphy_length[0] > max_fifo_size) {
		PHY_RADAR(("FIFO LENGTH in ant 0 is greater than max_fifo_size of %d\n",
			max_fifo_size));
		rt->nphy_length[0]  = 0;
	}

	if (rt->nphy_length[1]  > max_fifo_size) {
		PHY_RADAR(("FIFO LENGTH in ant 1 is greater than max_fifo_size of %d\n",
			max_fifo_size));
		rt->nphy_length[1]  = 0;
	}

	/* enable pulses received at each antenna messages if feature_mask bit-2 is set */
	if ((rparams->radar_args.feature_mask & 0x4) &&
		((rt->nphy_length[0] > 5) || (rt->nphy_length[1] > 5))) {
		PHY_RADAR(("ant 0:%d ant 1:%d\n", rt->nphy_length[0], rt->nphy_length[1]));
	}

	rt->nphy_length[0] /= 4;	/* 4 words per pulse */

	/* use the last sample for bin5 */
	rt->tstart_list_bin5[0][0] = rt->tstart_list_tail[0];
	rt->width_list_bin5[0][0] = rt->width_list_tail[0];
	rt->fm_list_n[0][0] = rt->fm_list_tail[0];
	for (i = 0; i < rt->nphy_length[0]; i++) {
		/* Read out FIFO 0 */
		if (ISHTPHY(pi)) {
			w0 = phy_reg_read(pi, HTPHY_Antenna0_radarFifoData);
			w1 = phy_reg_read(pi, HTPHY_Antenna0_radarFifoData);
			w2 = phy_reg_read(pi, HTPHY_Antenna0_radarFifoData);
			w3 = phy_reg_read(pi, HTPHY_Antenna0_radarFifoData);
		} else {
			w0 = phy_reg_read(pi, NPHY_Antenna0_radarFifoData);
			w1 = phy_reg_read(pi, NPHY_Antenna0_radarFifoData);
			w2 = phy_reg_read(pi, NPHY_Antenna0_radarFifoData);
			w3 = phy_reg_read(pi, NPHY_Antenna0_radarFifoData);
		}

		rt->tstart_list_n[0][i] = (uint32)((w0 << 16) + ((w1 & 0x0fff) << 4) + (w3 & 0xf));
		rt->tstart_list_bin5[0][i + 1] = rt->tstart_list_n[0][i];
		rt->width_list_n[0][i] = ((w3 & 0x10) << 8) + ((w2 & 0x00ff) << 4) +
			((w1 >> 12) & 0x000f);
		rt->width_list_bin5[0][i + 1] = rt->width_list_n[0][i];
		rt->fm_list_n[0][i + 1] = ((w3 & 0x20) << 3) + ((w2 >> 8) & 0x00ff);
	}
	/* save the last (tail) sample */
	rt->tstart_list_tail[0] = rt->tstart_list_bin5[0][i+1];
	rt->width_list_tail[0] = rt->width_list_bin5[0][i+1];
	rt->fm_list_tail[0] = rt->fm_list_n[0][i+1];

	rt->nphy_length_bin5[0] = rt->nphy_length[0] + 1;

	rt->nphy_length[1] /= 4;	/* 4 words per pulse */

	/* use the last sample for start of bin5 */
	rt->tstart_list_bin5[1][0] = rt->tstart_list_tail[1];
	rt->width_list_bin5[1][0] = rt->width_list_tail[1];
	rt->fm_list_n[1][0] = rt->fm_list_tail[1];
	for (i = 0; i < rt->nphy_length[1]; i++) {	/* Read out FIFO 1 */
		if (ISHTPHY(pi)) {
			w0 = phy_reg_read(pi, HTPHY_Antenna1_radarFifoData);
			w1 = phy_reg_read(pi, HTPHY_Antenna1_radarFifoData);
			w2 = phy_reg_read(pi, HTPHY_Antenna1_radarFifoData);
			w3 = phy_reg_read(pi, HTPHY_Antenna1_radarFifoData);
		} else {
			w0 = phy_reg_read(pi, NPHY_Antenna1_radarFifoData);
			w1 = phy_reg_read(pi, NPHY_Antenna1_radarFifoData);
			w2 = phy_reg_read(pi, NPHY_Antenna1_radarFifoData);
			w3 = phy_reg_read(pi, NPHY_Antenna1_radarFifoData);
		}

		rt->tstart_list_n[1][i] = (uint32)((w0 << 16) + ((w1 & 0x0fff) << 4) + (w3 & 0xf));
		rt->tstart_list_bin5[1][i + 1] = rt->tstart_list_n[1][i];
		rt->width_list_n[1][i] = ((w3 & 0x10) << 8) + ((w2 & 0x00ff) << 4) +
			((w1 >> 12) & 0x000f);
		rt->width_list_bin5[1][i + 1] = rt->width_list_n[1][i];
		rt->fm_list_n[1][i + 1] = ((w3 & 0x20) << 3) + ((w2 >> 8) & 0x00ff);
	}
	/* save the last (tail) sample */
	rt->tstart_list_tail[1] = rt->tstart_list_bin5[1][i+1];
	rt->width_list_tail[1] = rt->width_list_bin5[1][i+1];
	rt->fm_list_tail[1] = rt->fm_list_n[1][i+1];

	rt->nphy_length_bin5[1] = rt->nphy_length[1] + 1;
}

static void
wlc_phy_radar_detect_run_epoch_nphy(phy_info_t *pi, uint i,
	radar_work_t *rt, radar_params_t *rparams,
	uint32 *epoch_list, int epoch_length,
	int fra_t1, int fra_t2, int fra_t3, int fra_err,
	int pw_2us, int pw15us, int pw20us, int pw30us,
	int i250us, int i500us, int i625us, int i5000us,
	int pw2us, int i833us, int i2500us, int i3333us,
	int i938us, int i3066us,
	bool *fra_det_p, uint *stg_det_p, int *pulse_interval_p, int *nconsecq_pulses_p,
	int *detected_pulse_index_p, int *min_detected_pw_p, int *max_detected_pw_p)
{
	int j;
	int k;
	bool radar_detected = FALSE;

	int ndetected_staggered;
	bool fra_det = FALSE;
	uint stg_det = 0;
	char stag_det_seq[32];
	int tiern_length[RDR_NTIERS];
	int detected_pulse_index = 0;
	int nconsecq_pulses = 0;
	int nconseq2even, nconseq2odd;
	int nconseq3a, nconseq3b, nconseq3c;
	int skip_cnt;
	int first_interval;
	int tol;
	int pulse_interval = 0;
	int pw1us, i518us;
	int i3030us;

	/* pri limits for fcc tdwr radars */
	if (IS40MHZ(pi)) {
		pw1us = 46;
		i518us = 20720;
		i3030us = 121212;	/* korean type 3 intv */
	} else {
		pw1us = 23;
		i518us = 10360;
		i3030us = 60606;  /* korean type 3 intv */
	}


	*min_detected_pw_p = rparams->radar_args.max_pw;
	*max_detected_pw_p = rparams->radar_args.min_pw;

	detected_pulse_index = 0;
	nconsecq_pulses = 0;
	nconseq2even = 0;
	nconseq2odd = 0;
	nconseq3a = 0;
	nconseq3b = 0;
	nconseq3c = 0;

	/* initialize French radar detection variables */
	snprintf(stag_det_seq, sizeof(stag_det_seq), "%s", "");

	bzero(rt->tiern_list[0], sizeof(rt->tiern_list[0]));
	bzero(rt->tiern_pw[0], sizeof(rt->tiern_pw[0]));

	/*
	 * generate lists
	 */
	wlc_phy_radar_generate_tlist(epoch_list, rt->tiern_list[0], epoch_length, 1);
	wlc_phy_radar_generate_tpw((uint32 *) (rt->width_list + rt->epoch_start[i]),
		rt->tiern_pw[0], epoch_length, 1);
	for (j = 0; j < epoch_length; j++) {			/* quantize */
		rt->tiern_list[0][j] = rparams->radar_args.quant *
			((rt->tiern_list[0][j] + (rparams->radar_args.quant >> 1)) /
			rparams->radar_args.quant);
	}

	/*
	 * Do French radar detection
	 */
	ndetected_staggered = 0; /* need to have npulses_fra in an epoch */

	for (j = 0; j+2 < epoch_length && !fra_det; j++) {
		/*
		PHY_RADAR(("S3 DEBUG: T1=%d, T2=%d, T3=%d\n", rt->tiern_list[0][j],
			rt->tiern_list[0][j+1], rt->tiern_list[0][j+2]));
		*/
		if (ABS(rt->tiern_list[0][j] - fra_t1) < fra_err ||
			ABS(rt->tiern_list[0][j] - fra_t3 - fra_t1) < fra_err) {
			if (ABS(rt->tiern_list[0][j+1] - fra_t2) < fra_err) {
				if (ABS(rt->tiern_list[0][j+2] - fra_t3) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j] -
						fra_t1) < fra_err ? "1 2 3" : "31 2 3"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] -fra_t3 - fra_t1) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t1) < fra_err ? "1 2 31" : "31 2 31"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t3 - fra_t1 - fra_t2)
						< fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t1) < fra_err ? "1 2 312" : "31 2 312"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				}
			} else if (ABS(rt->tiern_list[0][j+1] - fra_t2 - fra_t3) < fra_err) {
				if (ABS(rt->tiern_list[0][j+2] - fra_t1) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t1) < fra_err ? "1 23 1" : "31 23 1"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] -fra_t1 - fra_t2) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t1) < fra_err ? "1 23 12" : "31 23 12"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t1 - fra_t2 - fra_t3)
						< fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t1) < fra_err ? "1 23 123" : "31 23 123"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				}
			}
			if (ndetected_staggered >= rparams->radar_args.npulses_fra) {
				fra_det = TRUE;
				break;
			}
		} else if (ABS(rt->tiern_list[0][j] - fra_t2) < fra_err ||
			ABS(rt->tiern_list[0][j] - fra_t1 - fra_t2) < fra_err) {
			if (ABS(rt->tiern_list[0][j+1] - fra_t3) < fra_err) {
				if (ABS(rt->tiern_list[0][j+2] - fra_t1) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t2 < fra_err) ? "2 3 1" : "12 3 1"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t1 -fra_t2) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t2) < fra_err ? "2 3 12" : "12 3 12"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t1 - fra_t2
						- fra_t3) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t2) < fra_err ? "2 3 123" : "12 3 123"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				}
			} else if (ABS(rt->tiern_list[0][j+1] - fra_t3 - fra_t1) < fra_err) {
				if (ABS(rt->tiern_list[0][j+2] - fra_t2) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t2) < fra_err ? "2 31 2" : "12 31 2"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t2 -fra_t3) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t2) < fra_err ? "2 31 23" : "12 31 23"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t2 - fra_t3
						- fra_t1) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t2) < fra_err ? "2 31 231" : "12 31 231"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				}
			}
			if (ndetected_staggered >= rparams->radar_args.npulses_fra) {
				fra_det = TRUE;
				break;
			}
		} else if (ABS(rt->tiern_list[0][j] - fra_t3) < fra_err ||
			ABS(rt->tiern_list[0][j] - fra_t2 - fra_t3) < fra_err) {
			if (ABS(rt->tiern_list[0][j+1] - fra_t1) < fra_err) {
				if (ABS(rt->tiern_list[0][j+2] - fra_t2) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t3) < fra_err ? "3 1 2" : "23 1 2"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t2 -fra_t3) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t3) < fra_err ? "3 1 23" : "23 1 23"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t2 - fra_t3
						- fra_t1) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t3) < fra_err ? "3 1 231" : "23 1 231"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				}
			} else if (ABS(rt->tiern_list[0][j+1] - fra_t1 - fra_t2) < fra_err) {
				if (ABS(rt->tiern_list[0][j+2] - fra_t3) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t3) < fra_err ? "3 12 3" : "23 12 3"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t3 -fra_t1) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t3) < fra_err ? "3 12 31" : "23 12 31"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				} else if (ABS(rt->tiern_list[0][j+2] - fra_t3 - fra_t1
						- fra_t2) < fra_err) {
					snprintf(stag_det_seq, sizeof(stag_det_seq), "%s",
						(ABS(rt->tiern_list[0][j]
						- fra_t3) < fra_err ? "3 12 312" : "23 12 312"));
					PHY_RADAR(("S3 DETECT SEQ: %s\n", stag_det_seq));
					ndetected_staggered++;
					j = j + 2;
				}
			}
			if (ndetected_staggered >= rparams->radar_args.npulses_fra) {
				fra_det = TRUE;
				break;
			}
		} else
			continue;

	}	/* French radar for loop */

	if (fra_det)
		goto end;

	tiern_length[0] = epoch_length;
	wlc_phy_radar_filter_list(rt->tiern_list[0], &(tiern_length[0]), rparams->min_tint,
		rparams->max_tint);

	/* Detect contiguous only pulses */
	detected_pulse_index = 0;
	nconsecq_pulses = 0;
	nconseq2even = 0;
	nconseq2odd = 0;
	nconseq3a = 0;
	nconseq3b = 0;
	nconseq3c = 0;
	skip_cnt = 0;

	tol = rparams->radar_args.quant;
	first_interval = rt->tiern_list[0][0];

	for (j = 0; j < epoch_length - 2; j++) {
		/* contiguous pulse detection */
		if ((rt->tiern_list[0][j] == first_interval ||
			rt->tiern_list[0][j] == (first_interval >> 1) ||
			rt->tiern_list[0][j] == (first_interval << 1)) &&

			/* fcc filters */
			(((pi->rparams->radar_args.feature_mask & 0x800) &&
			(rt->tiern_list[0][j] >= pi->rparams->radar_args.min_deltat) &&

			/* type 2 filter */
			((rt->tiern_pw[0][j] <= pi->rparams->max_type2_pw &&
			rt->tiern_pw[0][j+1] <= pi->rparams->max_type2_pw &&
			rt->tiern_list[0][j] >= pi->rparams->min_type2_intv - tol &&
			rt->tiern_list[0][j] <= pi->rparams->max_type2_intv + tol) ||

			/* type 3,4 filter */
			(rt->tiern_pw[0][j] <= pi->rparams->max_type4_pw &&
			rt->tiern_pw[0][j+1] <= pi->rparams->max_type4_pw &&
			rt->tiern_list[0][j] >= pi->rparams->min_type3_4_intv - tol &&
			rt->tiern_list[0][j] <= pi->rparams->max_type3_4_intv + tol) ||

			/* fcc weather radar filter */
			(rt->tiern_pw[0][j] <= pw_2us &&
			rt->tiern_pw[0][j+1] <= pw_2us &&
			rt->tiern_list[0][j] >= pi->rparams->max_type3_4_intv - tol &&
			rt->tiern_list[0][j] <= i3066us + pw20us) ||

			/* korean type 3 filter */
			(rt->tiern_pw[0][j] <= pw_2us &&
			rt->tiern_pw[0][j+1] <= pw_2us &&
			rt->tiern_list[0][j] >= i3030us - tol &&
			rt->tiern_list[0][j] <= i3030us + tol) ||

			/* type 1, jp2+1 filter */
			(rt->tiern_pw[0][j] <= pi->rparams->max_type1_pw &&
			rt->tiern_pw[0][j+1] <= pi->rparams->max_type1_pw &&
			rt->tiern_list[0][j] >= pi->rparams->jp2_1_intv - tol &&
			rt->tiern_list[0][j] <= pi->rparams->type1_intv + tol) ||

			/* type jp1-2, jp2-3 filter */
			(rt->tiern_pw[0][j] <= pi->rparams->max_jp1_2_pw &&
			rt->tiern_pw[0][j+1] <= pi->rparams->max_jp1_2_pw &&
			rt->tiern_list[0][j] >= pi->rparams->jp1_2_intv - tol &&
			rt->tiern_list[0][j] <= pi->rparams->jp2_3_intv + tol))) ||

			/* etsi filters */
			((pi->rparams->radar_args.feature_mask & 0x1000) &&

			/* type 1, 2, 5, 6 filter */
			((rt->tiern_pw[0][j] <= pw15us &&
			rt->tiern_pw[0][j+1] <= pw15us &&
			rt->tiern_list[0][j] >= i625us - tol*3 &&
			rt->tiern_list[0][j] <= i5000us + tol) ||

			/* packet based staggered types 4, 5 */
			(rt->tiern_pw[0][j] <= pw2us &&
			rt->tiern_pw[0][j+1] <= pw2us &&
			rt->tiern_list[0][j] >= i833us - tol &&
			rt->tiern_list[0][j] <= i2500us + tol) ||

			/* type 3, 4 filter */
			(rt->tiern_pw[0][j] <= pw30us &&
			rt->tiern_pw[0][j+1] <= pw30us &&
			rt->tiern_list[0][j] >= i250us - tol/2 &&
			rt->tiern_list[0][j] <= i500us + tol))))) {
			nconsecq_pulses++;
			/* check detected pulse for pulse width tolerance */
			if (nconsecq_pulses >= rparams->radar_args.npulses-1) {
				*min_detected_pw_p = rparams->radar_args.max_pw;
				*max_detected_pw_p = rparams->radar_args.min_pw;
				for (k = 0; k <  rparams->radar_args.npulses; k++) {
					if (rt->tiern_pw[0][j-k] <= *min_detected_pw_p)
						*min_detected_pw_p = rt->tiern_pw[0][j-k];
					if (rt->tiern_pw[0][j-k] >= *max_detected_pw_p)
						*max_detected_pw_p = rt->tiern_pw[0][j-k];
				}
				if (*max_detected_pw_p - *min_detected_pw_p <=
					rparams->radar_args.max_pw_tol) {
					pulse_interval = rt->tiern_list[0][j];
					detected_pulse_index = j -
						rparams->radar_args.npulses + 1;
					break;	/* radar detected */
				}
			}
		} else {
			if (rparams->radar_args.feature_mask & 0x80) {
				PHY_RADAR(("IV:%d-%d ", rt->tiern_list[0][j], j));
					PHY_RADAR(("PW:%d,%d-%d ", rt->tiern_pw[0][j],
					rt->tiern_pw[0][j+1], j));
				}
				nconsecq_pulses = 0;
				first_interval = rt->tiern_list[0][j];
			}

			/* staggered 2/3 single filters */
			/* staggered 2 even */
			if (j >= 2 && j % 2 == 0) {
				if (rt->tiern_list[0][j] == rt->tiern_list[0][j-2] &&
				    ((rt->tiern_pw[0][j] <= pw1us &&
				      ((rt->tiern_list[0][j] >= i518us -
				        rparams->radar_args.quant &&
				        rt->tiern_list[0][j] <= i938us +
				        rparams->radar_args.quant) ||
				       (rt->tiern_list[0][j] >= i3066us -
				        rparams->radar_args.quant &&
				        rt->tiern_list[0][j] <= i3066us +
				        rparams->radar_args.quant)) &&
				      pi->rparams->radar_args.feature_mask & 0x800) || /* fcc */
				     (rt->tiern_list[0][j] >= i833us - tol &&
				      rt->tiern_list[0][j] <= i3333us + tol &&
				      pi->rparams->radar_args.feature_mask & 0x1000))) { /* etsi */
					nconseq2even++;
					/* check detected pulse for pulse width tolerance */
					if (nconseq2even + nconseq2odd >=
						rparams->radar_args.npulses_stg2-1) {
						radar_detected = 1;   /* preset */
						*min_detected_pw_p = rparams->radar_args.max_pw;
						*max_detected_pw_p = rparams->radar_args.min_pw;
						for (k = 0; k < nconseq2even; k++) {
							/*
							PHY_RADAR(("EVEN neven=%d, k=%d pw= %d\n",
							nconseq2even, k,rt->tiern_pw[0][j-2*k]));
							*/
							if (rt->tiern_pw[0][j-2*k] <=
								*min_detected_pw_p)
								*min_detected_pw_p =
								rt->tiern_pw[0][j-2*k];
							if (rt->tiern_pw[0][j-2*k] >=
								*max_detected_pw_p)
								*max_detected_pw_p =
								rt->tiern_pw[0][j-2*k];
						}
						if (*max_detected_pw_p - *min_detected_pw_p >
							rparams->radar_args.max_pw_tol) {
							radar_detected = 0;
						}
						if (nconseq2odd > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq2odd; k++) {
								/*
								PHY_RADAR(("EVEN nodd=%d,"
									"k=%d pw= %d\n",
									nconseq2odd, k,
									rt->tiern_pw[0][j-2*k-1]));
								*/
								if (rt->tiern_pw[0][j-2*k-1] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-2*k-1];
								if (rt->tiern_pw[0][j-2*k-1] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-2*k-1];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (radar_detected) {
							stg_det = 2;
							pulse_interval = rt->tiern_list[0][j];
							detected_pulse_index = j -
							2 * rparams->radar_args.npulses_stg2 + 1;
							break;	/* radar detected */
						}
					}
				} else {
					if (rparams->radar_args.feature_mask & 0x20)
						PHY_RADAR(("EVEN RESET neven=%d, j=%d "
						"intv=%d pw= %d\n",
						nconseq2even, j, rt->tiern_list[0][j],
						rt->tiern_pw[0][j]));
					nconseq2even = 0;
				}
			}

			/* staggered 2 odd */
			if (j >= 3 && j % 2 == 1) {
				if (rt->tiern_list[0][j] == rt->tiern_list[0][j-2] &&
				    ((rt->tiern_pw[0][j] <= pw1us &&
				      ((rt->tiern_list[0][j] >= i518us -
				        rparams->radar_args.quant &&
				        rt->tiern_list[0][j] <= i938us +
				        rparams->radar_args.quant) ||
				       (rt->tiern_list[0][j] >= i3066us -
				        rparams->radar_args.quant &&
				        rt->tiern_list[0][j] <= i3066us +
				        rparams->radar_args.quant)) &&
				      pi->rparams->radar_args.feature_mask & 0x800) || /* fcc */
				     (rt->tiern_list[0][j] >= i833us - tol &&
				      rt->tiern_list[0][j] <= i3333us + tol &&
				      pi->rparams->radar_args.feature_mask & 0x1000))) { /* etsi */
					nconseq2odd++;
					/* check detected pulse for pulse width tolerance */
					if (nconseq2even + nconseq2odd >=
						rparams->radar_args.npulses_stg2-1) {
						radar_detected = 1;   /* preset */
						*min_detected_pw_p = rparams->radar_args.max_pw;
						*max_detected_pw_p = rparams->radar_args.min_pw;
						for (k = 0; k < nconseq2odd; k++) {
							/*
							PHY_RADAR(("ODD nodd=%d, k=%d pw= %d\n",
							nconseq2odd, k,rt->tiern_pw[0][j-2*k]));
							*/
							if (rt->tiern_pw[0][j-2*k] <=
								*min_detected_pw_p)
								*min_detected_pw_p =
								rt->tiern_pw[0][j-2*k];
							if (rt->tiern_pw[0][j-2*k] >=
								*max_detected_pw_p)
								*max_detected_pw_p =
								rt->tiern_pw[0][j-2*k];
						}
						if (*max_detected_pw_p - *min_detected_pw_p >
							rparams->radar_args.max_pw_tol) {
							radar_detected = 0;
						}
						if (nconseq2even > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq2even; k++) {
								/*
								PHY_RADAR(("ODD neven=%d,"
								" k=%d pw= %d\n",
								nconseq2even, k,
								rt->tiern_pw[0][j-2*k-1]));
								*/
								if (rt->tiern_pw[0][j-2*k-1] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-2*k-1];
								if (rt->tiern_pw[0][j-2*k-1] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-2*k-1];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (radar_detected) {
							stg_det = 2;
							pulse_interval = rt->tiern_list[0][j];
							detected_pulse_index = j -
							2 * rparams->radar_args.npulses_stg2 + 1;
							break;	/* radar detected */
						}
					}
				} else {
					if (rparams->radar_args.feature_mask & 0x20)
						PHY_RADAR(("ODD RESET nodd=%d, j=%d "
						"intv=%d pw= %d\n",
						nconseq2odd, j, rt->tiern_list[0][j],
						rt->tiern_pw[0][j]));
					nconseq2odd = 0;
				}
			}

			/* staggered 3-a */
			if ((j >= 3 && j % 3 == 0) &&
				(pi->rparams->radar_args.feature_mask & 0x1000)) {
				if ((rt->tiern_list[0][j] == rt->tiern_list[0][j-3]) &&
					rt->tiern_pw[0][j] <= pw2us &&
					(rt->tiern_list[0][j] >= i833us - tol &&
					rt->tiern_list[0][j] <= i3333us + tol)) {
					nconseq3a++;
					/* check detected pulse for pulse width tolerance */
					if (nconseq3a + nconseq3b + nconseq3c >=
						rparams->radar_args.npulses_stg3-1) {
						radar_detected = 1;   /* preset */
						*min_detected_pw_p = rparams->radar_args.max_pw;
						*max_detected_pw_p = rparams->radar_args.min_pw;
						for (k = 0; k < nconseq3a; k++) {
							/*
							PHY_RADAR(("3A n3a=%d, k=%d pw= %d\n",
							nconseq3a, k,rt->tiern_pw[0][j-3*k]));
							*/
							if (rt->tiern_pw[0][j-3*k] <=
								*min_detected_pw_p)
								*min_detected_pw_p =
								rt->tiern_pw[0][j-3*k];
							if (rt->tiern_pw[0][j-3*k] >=
								*max_detected_pw_p)
								*max_detected_pw_p =
								rt->tiern_pw[0][j-3*k];
						}
						if (*max_detected_pw_p - *min_detected_pw_p >
							rparams->radar_args.max_pw_tol) {
							radar_detected = 0;
						}
						if (nconseq3c > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq3c; k++) {
								/*
								PHY_RADAR(("3A n3c=%d, k=%d "
								"pw= %d\n",
								nconseq3c, k,
								rt->tiern_pw[0][j-3*k-1]));
								*/
								if (rt->tiern_pw[0][j-3*k-1] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-3*k-1];
								if (rt->tiern_pw[0][j-3*k-1] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-3*k-1];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (nconseq3b > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq3b; k++) {
								/*
								PHY_RADAR(("3A n3b=%d, k=%d "
								"pw= %d\n",
								nconseq3b, k,
								rt->tiern_pw[0][j-3*k-2]));
								*/
								if (rt->tiern_pw[0][j-3*k-2] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-3*k-2];
								if (rt->tiern_pw[0][j-3*k-2] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-3*k-2];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (radar_detected) {
							stg_det = 3;
							pulse_interval = rt->tiern_list[0][j];
							detected_pulse_index = j -
							3 * rparams->radar_args.npulses_stg3 + 1;
							break;	/* radar detected */
						}
					}
				} else {
					if (rparams->radar_args.feature_mask & 0x20)
						PHY_RADAR(("3A RESET n3a=%d, j=%d intv=%d "
						"pw= %d\n",
						nconseq3a, j, rt->tiern_list[0][j],
						rt->tiern_pw[0][j]));
					nconseq3a = 0;
				}
			}

			/* staggered 3-b */
			if ((j >= 4 && j % 3 == 1) &&
				(pi->rparams->radar_args.feature_mask & 0x1000)) {
				if ((rt->tiern_list[0][j] == rt->tiern_list[0][j-3]) &&
					rt->tiern_pw[0][j] <= pw2us &&
					(rt->tiern_list[0][j] >= i833us - tol &&
					rt->tiern_list[0][j] <= i3333us + tol)) {
					nconseq3b++;
					/* check detected pulse for pulse width tolerance */
					if (nconseq3a + nconseq3b + nconseq3c >=
						rparams->radar_args.npulses_stg3-1) {
						radar_detected = 1;   /* preset */
						*min_detected_pw_p = rparams->radar_args.max_pw;
						*max_detected_pw_p = rparams->radar_args.min_pw;
						for (k = 0; k < nconseq3b; k++) {
							/*
							PHY_RADAR(("3B n3b=%d, k=%d pw= %d\n",
							nconseq3b, k,rt->tiern_pw[0][j-3*k]));
							*/
							if (rt->tiern_pw[0][j-3*k] <=
								*min_detected_pw_p)
								*min_detected_pw_p =
								rt->tiern_pw[0][j-3*k];
							if (rt->tiern_pw[0][j-3*k] >=
								*max_detected_pw_p)
								*max_detected_pw_p =
								rt->tiern_pw[0][j-3*k];
						}
						if (*max_detected_pw_p - *min_detected_pw_p >
							rparams->radar_args.max_pw_tol) {
							radar_detected = 0;
						}
						if (nconseq3a > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq3a; k++) {
								/*
								PHY_RADAR(("3B n3a=%d, k=%d "
								"pw= %d\n",
								nconseq3a, k,
								rt->tiern_pw[0][j-3*k-1]));
								*/
								if (rt->tiern_pw[0][j-3*k-1] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-3*k-1];
								if (rt->tiern_pw[0][j-3*k-1] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-3*k-1];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (nconseq3c > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq3c; k++) {
								/*
								PHY_RADAR(("3B n3c=%d, k=%d "
								"pw= %d\n",
								nconseq3c, k,
								rt->tiern_pw[0][j-3*k-2]));
								*/
								if (rt->tiern_pw[0][j-3*k-2] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-3*k-2];
								if (rt->tiern_pw[0][j-3*k-2] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-3*k-2];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (radar_detected) {
							stg_det = 3;
							pulse_interval = rt->tiern_list[0][j];
							detected_pulse_index = j -
							3 * rparams->radar_args.npulses_stg3 + 1;
							break;	/* radar detected */
						}
					}
				} else {
					if (rparams->radar_args.feature_mask & 0x20)
						PHY_RADAR(("3B RESET n3b=%d, j=%d intv=%d pw= %d\n",
						nconseq3b, j, rt->tiern_list[0][j],
						rt->tiern_pw[0][j]));
					nconseq3b = 0;
				}
			}

			/* staggered 3-c */
			if ((j >= 5 && j % 3 == 2) &&
				(pi->rparams->radar_args.feature_mask & 0x1000)) {
				if ((rt->tiern_list[0][j] == rt->tiern_list[0][j-3]) &&
					rt->tiern_pw[0][j] <= pw2us &&
					(rt->tiern_list[0][j] >= i833us - tol &&
					rt->tiern_list[0][j] <= i3333us + tol)) {
					nconseq3c++;
					/* check detected pulse for pulse width tolerance */
					if (nconseq3a + nconseq3b + nconseq3c >=
						rparams->radar_args.npulses_stg3-1) {
						radar_detected = 1;   /* preset */
						*min_detected_pw_p = rparams->radar_args.max_pw;
						*max_detected_pw_p = rparams->radar_args.min_pw;
						for (k = 0; k < nconseq3c; k++) {
							/*
							PHY_RADAR(("3C n3c=%d, k=%d pw= %d\n",
							nconseq3c, k,rt->tiern_pw[0][j-3*k]));
							*/
							if (rt->tiern_pw[0][j-3*k] <=
								*min_detected_pw_p)
								*min_detected_pw_p =
								rt->tiern_pw[0][j-3*k];
							if (rt->tiern_pw[0][j-3*k] >=
								*max_detected_pw_p)
								*max_detected_pw_p =
								rt->tiern_pw[0][j-3*k];
						}
						if (*max_detected_pw_p - *min_detected_pw_p >
							rparams->radar_args.max_pw_tol) {
							radar_detected = 0;
						}
						if (nconseq3b > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq3b; k++) {
								/*
								PHY_RADAR(("3C n3b=%d, k=%d "
								"pw= %d\n",
								nconseq3b, k,
								rt->tiern_pw[0][j-3*k-1]));
								*/
								if (rt->tiern_pw[0][j-3*k-1] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-3*k-1];
								if (rt->tiern_pw[0][j-3*k-1] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-3*k-1];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (nconseq3a > 0) {
							*min_detected_pw_p =
								rparams->radar_args.max_pw;
							*max_detected_pw_p =
								rparams->radar_args.min_pw;
							for (k = 0; k < nconseq3a; k++) {
								/*
								PHY_RADAR(("3C n3a=%d, k=%d "
								"pw= %d\n",
								nconseq3a, k,
								rt->tiern_pw[0][j-3*k-2]));
								*/
								if (rt->tiern_pw[0][j-3*k-2] <=
									*min_detected_pw_p)
									*min_detected_pw_p =
									rt->tiern_pw[0][j-3*k-2];
								if (rt->tiern_pw[0][j-3*k-2] >=
									*max_detected_pw_p)
									*max_detected_pw_p =
									rt->tiern_pw[0][j-3*k-2];
							}
							if (*max_detected_pw_p - *min_detected_pw_p
								> rparams->radar_args.max_pw_tol) {
								radar_detected = 0;
							}
						}
						if (radar_detected) {
							stg_det = 3;
							pulse_interval = rt->tiern_list[0][j];
							detected_pulse_index = j -
							3 * rparams->radar_args.npulses_stg3 + 1;
							break;	/* radar detected */
						}
					}
				} else {
					if (rparams->radar_args.feature_mask & 0x20)
						PHY_RADAR(("3C RESET n3c=%d, j=%d intv=%d pw= %d\n",
						nconseq3c, j, rt->tiern_list[0][j],
						rt->tiern_pw[0][j]));
					nconseq3c = 0;
				}
			}

			if (rt->tiern_list[0][j] < pi->rparams->radar_args.min_deltat) {
				if (0) {
					PHY_RADAR(("%d ", rt->tiern_list[0][j]));
				}
				skip_cnt++;
			}
	}  /* for (j = 0; j < epoch_length - 2; j++) */
	if (pulse_interval != 0)
		goto end;

end:
	*fra_det_p = fra_det;
	*stg_det_p = stg_det;
	*pulse_interval_p = pulse_interval;
	*nconsecq_pulses_p = nconsecq_pulses;
	*detected_pulse_index_p = detected_pulse_index;
}

static int
wlc_phy_radar_detect_run_nphy(phy_info_t *pi)
{
	int i, j, k;
	int wr_ptr;
	int ant, mlength;
	uint32 *epoch_list;
	int epoch_length = 0;
	int epoch_detected;
	int pulse_interval;
	uint32 tstart;
	int width, fm, valid_lp;
	bool filter_pw = TRUE;
	int32 deltat;
	radar_work_t *rt = &pi->radar_work;
	radar_params_t *rparams = pi->rparams;
	int fra_t1, fra_t2, fra_t3, fra_err;
	bool fra_det;
	uint stg_det;
	int skip_type = 0;
	int min_detected_pw;
	int max_detected_pw;
	int pw_2us, pw15us, pw20us, pw30us;
	int i250us, i500us, i625us, i5000us;
	int pw2us, i833us, i2500us, i3333us;
	int i938us, i3066us;
	int i633us, i658us;
	int nconsecq_pulses = 0;
	int detected_pulse_index = 0;
	uint32 max_lp_buffer_span;
	int32 deltat2 = 0;
	int32 salvate_intv = 0;
	uint32 tmp_uint32;
	int pw_dif, pw_tol, fm_dif, fm_tol;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	/* update dfs_timer */
	rt->dfs_timer++;

	fra_err = pi->rparams->radar_args.fra_pulse_err;
	if (IS40MHZ(pi)) {
		fra_t1 = FRA_T1_40MHZ;
		fra_t2 = FRA_T2_40MHZ;
		fra_t3 = FRA_T3_40MHZ;
		pw_2us  = 80+12;
		pw15us = 600+90;
		pw20us = 800+120;
		pw30us = 1200+180;
		i250us = 10000;
		i500us = 20000;
		i625us = 25000;
		i633us = 25316;   /* 1580 Hz */
		i658us = 26316;   /* 1520 Hz */
		i938us = 37520;   /* 1066.1 Hz */
		i3066us = 122624; /* 326.2 Hz */
		i5000us = 200000; /* 200 Hz */
		/* staggered */
		pw2us  = 120+18;
		i833us = 33333;   /* 1200 Hz */
		i2500us = 100000;  /* 400 Hz */
		i3333us = 133333; /* 300 Hz */
		max_lp_buffer_span = MAX_LP_BUFFER_SPAN_40MHZ;  /* 12sec */
	} else {
		fra_t1 = FRA_T1_20MHZ;
		fra_t2 = FRA_T2_20MHZ;
		fra_t3 = FRA_T3_20MHZ;
		pw_2us  = 40+6;
		pw15us = 300+45;
		pw20us = 400+60;
		pw30us = 600+90;
		i250us = 5000;    /* 4000 Hz */
		i500us = 10000;   /* 2000Hz */
		i625us = 12500;   /* 1600 Hz */
		i633us = 12658;   /* 1580 Hz */
		i658us = 13158;   /* 1520 Hz */
		i938us = 18760;   /* 1066.1 Hz */
		i3066us = 61312;  /* 326.2 Hz */
		i5000us = 100000; /* 200 Hz */
		/* staggered */
		pw2us  = 60+9;
		i833us = 16667;   /* 1200 Hz */
		i2500us = 50000;  /* 400 Hz */
		i3333us = 66667;  /* 300 Hz */
		max_lp_buffer_span = MAX_LP_BUFFER_SPAN_20MHZ;  /* 12sec */
	}

	/* clear LP buffer if requested, and print LP buffer count history */
	if (pi->dfs_lp_buffer_nphy != 0) {
		pi->dfs_lp_buffer_nphy = 0;
		printf("DFS LP buffer =  ");
		for (i = 0; i < rt->lp_len_his_idx; i++) {
			printf("%d, ", rt->lp_len_his[i]);
		}
		printf("%d; now CLEARED\n", rt->lp_length);
		rt->lp_length = 0;
		rt->lp_pw_fm_matched = 0;
		rt->lp_n_non_single_pulses = 0;
		rt->lp_cnt = 0;
		rt->lp_skip_cnt = 0;
		rt->lp_skip_tot = 0;
		rt->lp_csect_single = 0;
		rt->lp_len_his_idx = 0;
		rt->last_detection_time = rt->dfs_timer;
		rt->last_detection_time_lp = rt->dfs_timer;
	}

	/* quantize */
	fra_t1 = rparams->radar_args.quant * ((fra_t1 + (rparams->radar_args.quant >> 1))
		/ rparams->radar_args.quant);
	fra_t2 = rparams->radar_args.quant * ((fra_t2 + (rparams->radar_args.quant >> 1))
		/ rparams->radar_args.quant);
	fra_t3 = rparams->radar_args.quant * ((fra_t3 + (rparams->radar_args.quant >> 1))
		/ rparams->radar_args.quant);

	if (!pi->rparams->radar_args.npulses) {
		PHY_ERROR(("radar params not initialized\n"));
		return RADAR_TYPE_NONE;
	}

	/* initialize variable */
	pulse_interval = 0;

	min_detected_pw = rparams->radar_args.max_pw;
	max_detected_pw = rparams->radar_args.min_pw;

	/* suspend mac before reading phyregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
		(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
		OSL_DELAY(1);
	}

	if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi))
		wlc_phy_radar_read_table_nphy_rev7(pi, rt, 1);
	else
		wlc_phy_radar_read_table_nphy(pi, rt, 1);

	if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);

	/* restart mac after reading phyregs */
	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);

	/* skip radar detect if doing periodic cal
	 * (the test-tones utilized during cal can trigger
	 * radar detect)
	 * NEED TO BE HERE AFTER READING DATA FROM (CLEAR) THE FIFO
	 */
	if (pi_nphy->nphy_rxcal_active) {
		pi_nphy->nphy_rxcal_active = FALSE;
		PHY_RADAR(("DOING RXCAL, SKIP RADARS\n"));
		return RADAR_TYPE_NONE;
	}
	if (pi->u.pi_htphy->radar_cal_active) {
		pi->u.pi_htphy->radar_cal_active = FALSE;
		PHY_RADAR(("DOING CAL, SKIP RADARS\n"));
		return RADAR_TYPE_NONE;
	}

	/*
	 * Reject if no pulses recorded
	 */
	if ((rt->nphy_length[0] < 1) && (rt->nphy_length[1] < 1)) {
		return RADAR_TYPE_NONE;
	}

	/* t2_min[15:12] = x; if n_non_single >= x && lp_length > npulses_lp => bin5 detected */
	/* t2_min[11:10] = # times combining adjacent pulses < min_pw_lp  */
	/* t2_min[9] = fm_tol enable */
	/* t2_min[8] = skip_type 5 enable */
	/* t2_min[7:4] = y; bin5 remove pw <= 10*y  */
	/* t2_min[3:0] = t; non-bin5 remove pw <= 5*y */

	/* remove "noise" pulses with small pw */
	for (ant = 0; ant < RDR_NANTENNAS; ant++) {
		wr_ptr = 0;
		mlength = rt->nphy_length_bin5[ant];
		for (i = 0; i < rt->nphy_length_bin5[ant]; i++) {
			if (rt->width_list_bin5[ant][i]  >
				10*((pi->rparams->radar_args.t2_min >> 4) & 0xf) &&
				rt->fm_list_n[ant][i] > rparams->radar_args.min_fm_lp/2) {
				rt->tstart_list_bin5[ant][wr_ptr] = rt->tstart_list_bin5[ant][i];
				rt->fm_list_n[ant][wr_ptr] = rt->fm_list_n[ant][i];
				rt->width_list_bin5[ant][wr_ptr] = rt->width_list_bin5[ant][i];
				++wr_ptr;
			} else {
				mlength--;
			}
		}	/* for mlength loop */
		rt->nphy_length_bin5[ant] = mlength;
	}	/* for ant loop */

	/* output ant0 fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 0) { /* bin5 */
			if (rt->nphy_length_bin5[0] > 0) {
			PHY_RADAR(("\nBin5 after removing noise pulses with pw <= %d",
				((pi->rparams->radar_args.t2_min >> 4) & 0xf) * 10));
			PHY_RADAR(("\nAnt 0: %d pulses, ", rt->nphy_length_bin5[0]));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->nphy_length_bin5[0]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_bin5[0][i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->nphy_length_bin5[0]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_bin5[0][i] -
					rt->tstart_list_bin5[0][i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->nphy_length_bin5[0]; i++)
				PHY_RADAR(("%i-%d ", rt->width_list_bin5[0][i], i));

			PHY_RADAR(("\nFM:  "));
			for (i = 0; i < rt->nphy_length_bin5[0]; i++)
				PHY_RADAR(("%d-%d ", rt->fm_list_n[0][i], i));
			PHY_RADAR(("\n"));
			}
		} else { /* short pulse */
			if (rt->nphy_length[0] > 0) {
			PHY_RADAR(("\nShort pulses entering radar_detect_run "));
			PHY_RADAR(("\nAnt 0: %d pulses, ", rt->nphy_length[0]));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->nphy_length[0]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_n[0][i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->nphy_length[0]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_n[0][i] -
					rt->tstart_list_n[0][i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->nphy_length[0]; i++)
				PHY_RADAR(("%i-%d ", rt->width_list_n[0][i], i));

			PHY_RADAR(("\n"));
			}
		}
	}

	/* output ant1 fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 0) { /* bin5 */
			if (rt->nphy_length_bin5[1] > 0) {
			PHY_RADAR(("\nBin5 after removing noise pulses with pw <= %d",
				((pi->rparams->radar_args.t2_min >> 4) & 0xf) * 10));
			PHY_RADAR(("\nAnt 1: %d pulses, ", rt->nphy_length_bin5[1]));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->nphy_length_bin5[1]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_bin5[1][i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->nphy_length_bin5[1]; i++)
				PHY_RADAR(("%d ", rt->tstart_list_bin5[1][i] -
					rt->tstart_list_bin5[1][i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->nphy_length_bin5[1]; i++)
				PHY_RADAR(("%i-%d ", rt->width_list_bin5[1][i], i));

			PHY_RADAR(("\nFM:  "));
			for (i = 0; i < rt->nphy_length_bin5[1]; i++)
				PHY_RADAR(("%d-%d ", rt->fm_list_n[1][i], i));
			PHY_RADAR(("\n"));
			}
		} else { /* short pulse */
			if (rt->nphy_length[1] > 0) {
			PHY_RADAR(("\nShort pulses entering radar_detect_run "));
			PHY_RADAR(("\nAnt 1: %d pulses, ", rt->nphy_length[1]));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->nphy_length[1]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_n[1][i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->nphy_length[1]; i++)
				PHY_RADAR(("%d ", rt->tstart_list_n[1][i] -
					rt->tstart_list_n[1][i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->nphy_length[1]; i++)
				PHY_RADAR(("%i-%d ", rt->width_list_n[1][i], i));

			PHY_RADAR(("\n"));
			}
		}
	}

	if (rparams->radar_args.feature_mask & 0x800) {   /* if fcc */

	/* START LONG PULSES (BIN5) DETECTION */

	/* Now combine, sort, and remove duplicated pulses from the each antenna */
	rt->length = 0;
	for (ant = 0; ant < RDR_NANTENNAS; ant++) {
		for (i = 0; i < rt->nphy_length_bin5[ant]; i++) {
			rt->tstart_list[rt->length] = rt->tstart_list_bin5[ant][i];
			rt->width_list[rt->length] = rt->width_list_bin5[ant][i];
			rt->fm_list[rt->length] = rt->fm_list_n[ant][i];
			rt->length++;
		}
	}

	for (i = 1; i < rt->length; i++) {	/* insertion sort */
		tstart = rt->tstart_list[i];
		width = rt->width_list[i];
		fm = rt->fm_list[i];
		j = i - 1;
		while ((j >= 0) && rt->tstart_list[j] > tstart) {
			rt->tstart_list[j + 1] = rt->tstart_list[j];
			rt->width_list[j + 1] = rt->width_list[j];
			rt->fm_list[j + 1] = rt->fm_list[j];
			j--;
			}
			rt->tstart_list[j + 1] = tstart;
			rt->width_list[j + 1] = width;
			rt->fm_list[j + 1] = fm;
	}

	/* output fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 0) { /* bin5 */
			if ((rt->length > 0))  {
			PHY_RADAR(("\nBin5 after combining pulses from two antennas"));
			PHY_RADAR(("\n%d pulses, ", rt->length));

			PHY_RADAR(("\ntstart=[  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%u ", rt->tstart_list[i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->length; i++)
				PHY_RADAR(("%d ", rt->tstart_list[i] -
					rt->tstart_list[i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%i-%d ", rt->width_list[i], i));

			PHY_RADAR(("\nFM:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%d-%d ", rt->fm_list[i], i));
			PHY_RADAR(("\n"));
			}
		}
	}

	/* Combine pulses that are adjacent */
	for (k = 0; k < 2; k++) {
		mlength = rt->length;
		if (mlength > 1) {
			for (i = 1; i < mlength; i++) {
				deltat = ABS((int32)(rt->tstart_list[i] -
					rt->tstart_list[i-1]));

				if (deltat <= (int32)pi->rparams->radar_args.max_pw_lp) {
					rt->width_list[i-1] = deltat + rt->width_list[i];
					/* print pulse combining debug messages */
					if (0) {
						PHY_RADAR(("*%d,%d,%d ",
						rt->tstart_list[i] -
						rt->tstart_list[i-1],
						rt->width_list[i],
						rt->width_list[i-1]));
					}

					rt->fm_list[i-1] = rt->fm_list[i-1] +
						rt->fm_list[i];
					for (j = i; j < mlength - 1; j++) {
						rt->tstart_list[j] =
							rt->tstart_list[j+1];
						rt->width_list[j] =
							rt->width_list[j+1];
						rt->fm_list[j] = rt->fm_list[j+1];
					}
					mlength--;
					rt->length--;
				}	/* if deltat */
			}	/* for mlength loop */
		}	/* mlength > 1 */
	}

	/* output fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 0) { /* bin5 */
			if ((rt->length > 0))  {
			PHY_RADAR(("\nBin5 after combining pulses that are adjcent"));
			PHY_RADAR(("\n%d pulses, ", rt->length));

			PHY_RADAR(("\ntstart=[  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%u ", rt->tstart_list[i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->length; i++)
				PHY_RADAR(("%d ", rt->tstart_list[i] -
					rt->tstart_list[i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%i-%d ", rt->width_list[i], i));

			PHY_RADAR(("\nFM:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%d-%d ", rt->fm_list[i], i));
			PHY_RADAR(("\n"));
			}
		}
	}

	/* remove pulses that are spaced < quant (128/256) */
	for (i = 1; i < rt->length; i++) {
		deltat = ABS((int32)(rt->tstart_list[i] - rt->tstart_list[i-1]));
		if (deltat < rparams->radar_args.quant) {
			for (j = i - 1; j < (rt->length); j++) {
				rt->tstart_list[j] = rt->tstart_list[j+1];
				rt->width_list[j] = rt->width_list[j+1];
				rt->fm_list[j] = rt->fm_list[j+1];
			}
			rt->length--;
		}
	}

	/* output fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 0) {
			if ((rt->length > 0))  { /* bin5 */
			PHY_RADAR(("\nBin5 after removing pulses that are spaced < %d\n",
				rparams->radar_args.quant));
			PHY_RADAR(("%d pulses, ", rt->length));

			PHY_RADAR(("\ntstart=[  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%u ", rt->tstart_list[i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->length; i++)
				PHY_RADAR(("%d ", rt->tstart_list[i] -
					rt->tstart_list[i - 1]));
			PHY_RADAR(("\n"));

			PHY_RADAR(("Pulse Widths:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%i-%d ", rt->width_list[i], i));
			PHY_RADAR(("\n"));

			PHY_RADAR(("FM:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%d-%d ", rt->fm_list[i], i));
			PHY_RADAR(("\n"));
			}
		}
	}

	/* prune lp buffer */
	/* remove any entry outside the time max delta_t_lp */
	if (rt->lp_length > 1) {
		deltat = ABS((int32)(rt->lp_buffer[rt->lp_length - 1] - rt->lp_buffer[0]));
		i = 0;
		while ((i < (rt->lp_length - 1)) &&
			(deltat > (int32)max_lp_buffer_span)) {
			i++;
			deltat = ABS((int32)(rt->lp_buffer[rt->lp_length - 1] - rt->lp_buffer[i]));
		}

		if (i > 0) {
			for (j = i; j < rt->lp_length; j++)
				rt->lp_buffer[j-i] = rt->lp_buffer[j];

			rt->lp_length -= i;
		}
	}

	/* First perform bin 5 detection */
	/* add new pulses */

	/* process each new pulse */
	for (i = 0; i < rt->length; i++) {
		deltat = ABS((int32)(rt->tstart_list[i] - rt->last_tstart));
		salvate_intv = ABS((int32) (rt->tstart_list[i] - rt->last_skipped_time));
		valid_lp = (rt->width_list[i] >= rparams->radar_args.min_pw_lp) &&
			(rt->width_list[i] <= rparams->radar_args.max_pw_lp) &&
			(rt->fm_list[i] >= rparams->radar_args.min_fm_lp) &&
			(deltat >= rparams->radar_args.max_pw_lp);

		/* filter out: max_deltat_l < pburst_intv_lp < min_burst_intv_lp */
		/* this was skip-type = 2, now not skipping for this */
		valid_lp = valid_lp &&(deltat <= (int32) rparams->max_deltat_lp ||
			deltat >= (int32) rparams->radar_args.min_burst_intv_lp);
		if ((salvate_intv > (int32) rparams->max_deltat_lp &&
			salvate_intv < (int32) rparams->radar_args.min_burst_intv_lp)) {
			valid_lp = 0;
		}

		if (rt->lp_length > 0 && rt->lp_length < LP_BUFFER_SIZE) {
			valid_lp = valid_lp &&
				(rt->tstart_list[i] != rt->lp_buffer[rt->lp_length]);
		}

		skip_type = 0;
		if (valid_lp && deltat
			>= (int32) rparams->radar_args.min_burst_intv_lp &&
			deltat < (int32) rparams->radar_args.max_burst_intv_lp) {
			rt->lp_cnt = 0;
		}

		/* skip the pulse if outside of pulse interval range (1-2ms), */
		/* burst to burst interval not within range, more than 3 pulses in a */
		/* burst, and not skip salvated */

		if ((valid_lp && ((rt->lp_length != 0)) &&
			((deltat < (int32) rparams->min_deltat_lp) ||
			(deltat > (int32) rparams->max_deltat_lp &&
			deltat < (int32) rparams->radar_args.min_burst_intv_lp) ||
			(deltat > (int32) rparams->radar_args.max_burst_intv_lp) ||
			(rt->lp_cnt > 2)))) {	/* possible skip lp */

			/* get skip type */
			if (deltat < (int32) rparams->min_deltat_lp) {
				skip_type = 1;
			} else if (deltat > (int32) rparams->max_deltat_lp &&
				deltat < (int32) rparams->radar_args.min_burst_intv_lp) {
				skip_type = 2;
			} else if (deltat > (int32) rparams->radar_args.max_burst_intv_lp) {
				skip_type = 3;
				rt->lp_cnt = 0;
			} else if (rt->lp_cnt > 2) {
				skip_type = 4;
			} else {
				skip_type = 999;
			}

			/* skip_salvate */
			if (((salvate_intv > (int32) rparams->min_deltat_lp &&
				salvate_intv < (int32) rparams->max_deltat_lp)) ||
				((salvate_intv > (int32)rparams->radar_args.min_burst_intv_lp) &&
				(salvate_intv < (int32)rparams->radar_args.max_burst_intv_lp))) {
				/* note valid_lp is not reset in here */
				skip_type = -1;  /* salvated PASSED */
				if (salvate_intv >= (int32) rparams->radar_args.min_burst_intv_lp &&
					salvate_intv <
						(int32) rparams->radar_args.max_burst_intv_lp) {
					rt->lp_cnt = 0;
				}
			}
		} else {  /* valid lp not by skip salvate */
				skip_type = -2;
		}

		width = 0;
		fm = 0;
		pw_dif = 0;
		fm_dif = 0;
		pw_tol = rparams->radar_args.max_span_lp & 0xff;
		fm_tol = 0;
		/* monitor the number of pw and fm matching */
		/* max_span_lp[15:12] = skip_tot max */
		/* max_span_lp[11:8] = x, x/16 = % alowed fm tollerance */
		/* max_span_lp[7:0] = alowed pw tollerance */
		if (valid_lp && skip_type <= 0) {
			if (rt->lp_cnt == 0) {
				rt->lp_pw[0] = rt->width_list[i];
				rt->lp_fm[0] = rt->fm_list[i];
			} else if (rt->lp_cnt == 1) {
				width = rt->lp_pw[0];
				fm = rt->lp_fm[0];
				pw_dif = ABS(rt->width_list[i] - width);
				fm_dif = ABS(rt->fm_list[i] - fm);
				if (pi->rparams->radar_args.t2_min & 0x200) {
					fm_tol = (fm*((rparams->radar_args.max_span_lp >> 8)
						& 0xf))/16;
				} else {
					fm_tol = 999;
				}
				if (pw_dif < pw_tol && fm_dif < fm_tol) {
					rt->lp_pw[1] = rt->width_list[i];
					rt->lp_fm[1] = rt->fm_list[i];
					++rt->lp_n_non_single_pulses;
					++rt->lp_pw_fm_matched;
				} else if (rt->lp_just_skipped == 1) {
					width = rt->lp_skipped_pw;
					fm = rt->lp_skipped_fm;
					pw_dif = ABS(rt->width_list[i] - width);
					fm_dif = ABS(rt->fm_list[i] - fm);
					if (pi->rparams->radar_args.t2_min & 0x200) {
						fm_tol = (fm*((rparams->radar_args.max_span_lp >> 8)
							& 0xf))/16;
					} else {
						fm_tol = 999;
					}
					if (pw_dif < pw_tol && fm_dif < fm_tol) {
						rt->lp_pw[1] = rt->width_list[i];
						rt->lp_fm[1] = rt->fm_list[i];
						++rt->lp_n_non_single_pulses;
						++rt->lp_pw_fm_matched;
						skip_type = -1;  /* salvated PASSED */
					} else {
						if (pi->rparams->radar_args.t2_min & 0x100) {
							skip_type = 5;
						}
					}
				} else {
					if (pi->rparams->radar_args.t2_min & 0x100) {
						skip_type = 5;
					}
				}
			} else if (rt->lp_cnt == 2) {
				width = rt->lp_pw[1];
				fm = rt->lp_fm[1];
				pw_dif = ABS(rt->width_list[i] - width);
				fm_dif = ABS(rt->fm_list[i] - fm);
				if (pi->rparams->radar_args.t2_min & 0x200) {
					fm_tol = (fm*((rparams->radar_args.max_span_lp >> 8)
						& 0xf))/16;
				} else {
					fm_tol = 999;
				}
				if (pw_dif < pw_tol && fm_dif < fm_tol) {
					rt->lp_pw[2] = rt->width_list[i];
					rt->lp_fm[2] = rt->fm_list[i];
					++rt->lp_n_non_single_pulses;
					++rt->lp_pw_fm_matched;
				} else if (rt->lp_just_skipped == 1) {
					width = rt->lp_skipped_pw;
					fm = rt->lp_skipped_fm;
					pw_dif = ABS(rt->width_list[i] - width);
					fm_dif = ABS(rt->fm_list[i] - fm);
					if (pi->rparams->radar_args.t2_min & 0x200) {
						fm_tol = (fm*((rparams->radar_args.max_span_lp >> 8)
							& 0xf))/16;
					} else {
						fm_tol = 999;
					}
					if (pw_dif < pw_tol && fm_dif < fm_tol) {
						rt->lp_pw[2] = rt->width_list[i];
						rt->lp_fm[2] = rt->fm_list[i];
						++rt->lp_n_non_single_pulses;
						++rt->lp_pw_fm_matched;
						skip_type = -1;  /* salvated PASSED */
					} else {
						if (pi->rparams->radar_args.t2_min & 0x100) {
							skip_type = 5;
						}
					}
				} else {
					if (pi->rparams->radar_args.t2_min & 0x100) {
						skip_type = 5;
					}
				}
			}
		}

		if (valid_lp && skip_type != -1 && skip_type != -2) {	/* skipped lp */
			valid_lp = 0;
			rt->lp_skip_cnt++;
			rt->lp_skip_tot++;
			rt->lp_just_skipped = 1;
			rt->lp_skipped_pw = rt->width_list[i];
			rt->lp_skipped_fm = rt->fm_list[i];

			tmp_uint32 = rt->last_skipped_time;
			rt->last_skipped_time = rt->tstart_list[i];

			/* print "SKIPPED LP" debug messages */
			PHY_RADAR(("Skipped LP:"
/*
				" KTstart=%u Klast_ts=%u Klskip=%u"
*/
				" nLP=%d nSKIP=%d KIntv=%u"
				" Ksalintv=%d PW=%d FM=%d"
				" Type=%d pulse#=%d skip_tot=%d csect_single=%d\n",
/*
			(rt->tstart_list[i])/1000, rt->last_tstart/1000, tmp_uint32/1000,
*/
				rt->lp_length, rt->lp_skip_cnt, deltat/1000, salvate_intv/1000,
				rt->width_list[i], rt->fm_list[i],
				skip_type, rt->lp_cnt, rt->lp_skip_tot, rt->lp_csect_single));
			if (skip_type == 5) {
				PHY_RADAR(("           "
					" pw2=%d pw_dif=%d pw_tol=%d fm2=%d fm_dif=%d fm_tol=%d\n",
					width, pw_dif, pw_tol, fm, fm_dif, fm_tol));
			}
			if (skip_type == 999) {
				PHY_RADAR(("UNKOWN SKIP TYPE: %d\n", skip_type));
			}
			/* if a) 2 consecutive skips, or */
			/*    b) too many consective singles, or */
			/*    c) too many total skip so far */
			/*  then reset lp buffer ... */
			if (rt->lp_skip_cnt >= rparams->radar_args.nskip_rst_lp) {
				if (rt->lp_len_his_idx < LP_LEN_HIS_SIZE) {
					rt->lp_len_his[rt->lp_len_his_idx] = rt->lp_length;
					rt->lp_len_his_idx++;
				}
				rt->lp_length = 0;
				rt->lp_skip_tot = 0;
				rt->lp_skip_cnt = 0;
				rt->lp_csect_single = 0;
				rt->lp_pw_fm_matched = 0;
				rt->lp_n_non_single_pulses = 0;
				rt->lp_cnt = 0;
			}
		} else if (valid_lp && (rt->lp_length < LP_BUFFER_SIZE)) {	/* valid lp */
			/* reset consecutive singles counter if pulse # > 0 */
			if (rt->lp_cnt > 0) {
				rt->lp_csect_single = 0;
			} else {
				++rt->lp_csect_single;
			}

			rt->lp_just_skipped = 0;
			/* print "VALID LP" debug messages */
			rt->lp_skip_cnt = 0;
			PHY_RADAR(("Valid LP:"
/*
				" KTstart=%u KTlast_ts=%u Klskip=%u"
*/
				" KIntv=%u"
				" Ksalintv=%d PW=%d FM=%d pulse#=%d"
				" pw2=%d pw_dif=%d pw_tol=%d fm2=%d fm_dif=%d fm_tol=%d\n",
/*
				(rt->tstart_list[i])/1000, rt->last_tstart/1000,
					rt->last_skipped_time/1000,
*/
				deltat/1000, salvate_intv/1000,
				rt->width_list[i], rt->fm_list[i], rt->lp_cnt,
				width, pw_dif, pw_tol,
				fm, fm_dif, fm_tol));
			PHY_RADAR(("         "
				" nLP=%d nSKIP=%d skipped_salvate=%d"
				" pw_fm_matched=%d #non-single=%d skip_tot=%d csect_single=%d\n",
				rt->lp_length + 1, rt->lp_skip_cnt, (skip_type == -1 ? 1 : 0),
				rt->lp_pw_fm_matched,
				rt->lp_n_non_single_pulses, rt->lp_skip_tot, rt->lp_csect_single));

				rt->lp_buffer[rt->lp_length] = rt->tstart_list[i];
				rt->lp_length++;
				rt->last_tstart = rt->tstart_list[i];
				rt->last_skipped_time = rt->tstart_list[i];

				rt->lp_cnt++;

			if (rt->lp_csect_single >= ((rparams->radar_args.t2_min >> 12) & 0xf)) {
				rt->lp_length -= rparams->radar_args.npulses_lp/2;
				if (rt->lp_length < 0) {
					rt->lp_length = 0;

				}
			}
		}
	}

	if (rt->lp_length > LP_BUFFER_SIZE)
		PHY_ERROR(("WARNING: LP buffer size is too long"));

#ifdef RADAR_DBG
	PHY_RADAR(("\n Bin 5 \n"));
	for (i = 0; i < rt->lp_length; i++) {
		PHY_RADAR(("%u  ", rt->lp_buffer[i]));
	}
	PHY_RADAR(("\n"));
#endif
	if (rt->lp_length >= rparams->radar_args.npulses_lp) {
		/* reject detection spaced more than 3 minutes */
		deltat2 = (int32) ((rt->dfs_timer - rt->last_detection_time_lp) >> 1);
		PHY_ERROR(("last_detection_time_lp=%u, dfs_timer=%u, deltat2=%d, deltat2_min=%d\n",
			rt->last_detection_time_lp, rt->dfs_timer, deltat2,  deltat2*15/6000));
		rt->last_detection_time_lp = rt->dfs_timer;
		if (deltat2 < 0) {
			deltat2 =  deltat2 + 2147483647;
		}
		PHY_ERROR(("deltat2=%d, deltat2_min=%d\n", deltat2, deltat2*15/6000));
		tmp_uint32 = RADAR_TYPE_NONE;
		if (deltat2 < 600) {
			if (rt->lp_csect_single <= rparams->radar_args.npulses_lp - 2 &&
				rt->lp_skip_tot < ((rparams->radar_args.max_span_lp >> 12) & 0xf)) {
				PHY_ERROR(("Bin 5 Radar Detection. Time from last detection = %u,"
					" = %dmin %dsec\n",
					deltat2*2, deltat2*15/3000, (deltat2*3/10)%60));
				tmp_uint32 = RADAR_TYPE_BIN5;
			}
		} else {
			PHY_ERROR(("SKIPPED false Bin 5 Radar Detection."
				" Time from last detection = %u, = %dmin %dsec, ncsect_single=%d\n",
				deltat2*2, deltat2*15/3000,
				(deltat2*3/10)%60, rt->lp_csect_single));
		}
		if (rt->lp_len_his_idx < LP_LEN_HIS_SIZE) {
			rt->lp_len_his[rt->lp_len_his_idx] = rt->lp_length;
			rt->lp_len_his_idx++;
		}
		rt->lp_length = 0;
		rt->lp_pw_fm_matched = 0;
		rt->lp_n_non_single_pulses = 0;
		rt->lp_skip_tot = 0;
		rt->lp_csect_single = 0;
		return tmp_uint32;
	}
	}	/* end of if fcc */

	/*
	 * Reject if no pulses recorded
	 */
	if (rt->nphy_length[0] < (rparams->radar_args.npulses) &&
		rt->nphy_length[1] < (rparams->radar_args.npulses)) {
		return RADAR_TYPE_NONE;
	}

	/* START SHORT PULSES (NON-BIN5) DETECTION */

	/* Combine pulses that are adjacent */
	for (ant = 0; ant < RDR_NANTENNAS; ant++) {
		mlength = rt->nphy_length[ant];
		if (mlength > 1) {
			for (i = 1; i < mlength; i++) {
				deltat = ABS((int32)(rt->tstart_list_n[ant][i] -
				                  rt->tstart_list_n[ant][i-1]));
				if ((deltat < (int32)pi->rparams->radar_args.min_deltat && FALSE) ||
					(deltat <= (int32)pi->rparams->radar_args.max_pw && TRUE)) {

					if (NREV_GE(pi->pubpi.phy_rev, 7) || ISHTPHY(pi)) {
						if (((uint32)(rt->tstart_list_n[ant][i] -
							rt->tstart_list_n[ant][i-1]))
							<= (uint32) rparams->radar_args.max_pw) {
							rt->width_list_n[ant][i-1] =
							ABS((int32)(rt->tstart_list_n[ant][i] -
							rt->tstart_list_n[ant][i-1])) +
							rt->width_list_n[ant][i];
						} else {
							rt->width_list_n[ant][i-1] =
								(uint32) rparams->radar_args.max_pw;
						}
					} else {
						/* print pulse combining debug messages */
						if (0) {
							PHY_RADAR(("*%d,%d,%d ",
								rt->tstart_list_n[ant][i] -
								rt->tstart_list_n[ant][i-1],
								rt->width_list_n[ant][i],
								rt->width_list_n[ant][i-1]));
						}
						if (rparams->radar_args.feature_mask & 0x2000) {
							rt->width_list_n[ant][i-1] =
								(rt->width_list_n[ant][i-1] >
								rt->width_list_n[ant][i] ?
								rt->width_list_n[ant][i-1] :
								rt->width_list_n[ant][i]);
						} else {
							rt->width_list_n[ant][i-1] =
								rt->width_list_n[ant][i-1] +
								rt->width_list_n[ant][i];
						}
					}

					for (j = i; j < mlength - 1; j++) {
						rt->tstart_list_n[ant][j] =
							rt->tstart_list_n[ant][j+1];
						rt->width_list_n[ant][j] =
							rt->width_list_n[ant][j+1];
					}
					mlength--;
					rt->nphy_length[ant]--;
				}
			}
		}
	}

	/* output ant0 fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 1 &&
			(rt->nphy_length[0] > 0)) {	/* short pulses */
			PHY_RADAR(("\nNon-Bin 5 After combining adacent pulses"));
			PHY_RADAR(("\nAnt 0: %d pulses, ", rt->nphy_length[0]));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->nphy_length[0]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_n[0][i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->nphy_length[0]; i++)
				PHY_RADAR(("%d ", rt->tstart_list_n[0][i] -
					rt->tstart_list_n[0][i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->nphy_length[0]; i++)
				PHY_RADAR(("%i-%d ", rt->width_list_n[0][i], i));
			PHY_RADAR(("\n"));
		}
	}

	/* output ant1 fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 1 &&
			(rt->nphy_length[1] > 0)) {	/* short pulses */
			PHY_RADAR(("\nShort pulses after combining adacent pulses"));
			PHY_RADAR(("\nAnt 1: %d pulses, ", rt->nphy_length[1]));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->nphy_length[1]; i++)
				PHY_RADAR(("%u ", rt->tstart_list_n[1][i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->nphy_length[1]; i++)
				PHY_RADAR(("%d ", rt->tstart_list_n[1][i] -
					rt->tstart_list_n[1][i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->nphy_length[1]; i++)
				PHY_RADAR(("%i-%d ", rt->width_list_n[1][i], i));
			PHY_RADAR(("\n"));

		}
	}

	/* Now combine, sort, and remove duplicated pulses from the 2 antennas */
	bzero(rt->tstart_list, sizeof(rt->tstart_list));
	bzero(rt->width_list, sizeof(rt->width_list));
	rt->length = 0;
	for (ant = 0; ant < RDR_NANTENNAS; ant++) {
		for (i = 0; i < rt->nphy_length[ant]; i++) {
			rt->tstart_list[rt->length] = rt->tstart_list_n[ant][i];
			rt->width_list[rt->length] = rt->width_list_n[ant][i];
			rt->length++;
		}
	}

	for (i = 1; i < rt->length; i++) {	/* insertion sort */
		tstart = rt->tstart_list[i];
		width = rt->width_list[i];
		j = i - 1;
		while ((j >= 0) && rt->tstart_list[j] > tstart) {
			rt->tstart_list[j + 1] = rt->tstart_list[j];
			rt->width_list[j + 1] = rt->width_list[j];
			j--;
			}
			rt->tstart_list[j + 1] = tstart;
			rt->width_list[j + 1] = width;
	}

	/* output fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 1 &&
			(rt->length > 0)) {	/* short pulses */
			PHY_RADAR(("\nShort pulses after combining pulses from two antennas"));
			PHY_RADAR(("\n%d pulses, ", rt->length));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%u ", rt->tstart_list[i]));
			PHY_RADAR(("];"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->length; i++)
				PHY_RADAR(("%d ", rt->tstart_list[i] -
					rt->tstart_list[i - 1]));

			PHY_RADAR(("\nPulse Widths:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%i-%d ", rt->width_list[i], i));
			PHY_RADAR(("\n"));
		}
	}

	/* remove pulses spaced less than min_deltat */
	for (i = 1; i < rt->length; i++) {
		deltat = (int32)(rt->tstart_list[i] - rt->tstart_list[i-1]);
		if (deltat < (int32)pi->rparams->radar_args.min_deltat) {
			for (j = i; j < (rt->length - 1); j++) {
				rt->tstart_list[j] = rt->tstart_list[j+1];
				rt->width_list[j] = rt->width_list[j+1];
			}
			rt->length--;
		}
	}

	/* output fifo data */
	if (rparams->radar_args.feature_mask & 0x8)  {
		if ((rparams->radar_args.feature_mask & 0x1) == 1 &&
			(rt->length > 0)) {	/* short pulses */
			PHY_RADAR(("\nShort pulses after removing pulses that are"
				" space > min_deltat (1ms)\n"));
			PHY_RADAR(("%d pulses, ", rt->length));

			PHY_RADAR(("\ntstart0=[  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%u ", rt->tstart_list[i]));
			PHY_RADAR(("];\n"));

			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->length; i++)
				PHY_RADAR(("%d ", rt->tstart_list[i] -
					rt->tstart_list[i - 1]));
			PHY_RADAR(("\n"));

			PHY_RADAR(("Pulse Widths:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%i-%d ", rt->width_list[i], i));
			PHY_RADAR(("\n"));
		}
	}
	/* remove entries spaced greater than max_deltat */
	if (rt->length > 1) {
		deltat = ABS((int32)(rt->tstart_list[rt->length - 1] - rt->tstart_list[0]));
		i = 0;
		while ((i < (rt->length - 1)) &&
		       (ABS(deltat) > (int32)rparams->radar_args.max_deltat)) {
			i++;
			deltat = ABS((int32)(rt->tstart_list[rt->length - 1] - rt->tstart_list[i]));
		}
		if (i > 0) {
			for (j = i; j < rt->length; j++) {
				rt->tstart_list[j-i] = rt->tstart_list[j];
				rt->width_list[j-i] = rt->width_list[j];
			}
			rt->length -= i;
		}
	}

	/*
	 * filter based on pulse width
	 */
	if (filter_pw) {
		j = 0;
		for (i = 0; i < rt->length; i++) {
			if ((rt->width_list[i] >= rparams->radar_args.min_pw) &&
				(rt->width_list[i] <= rparams->radar_args.max_pw)) {
				rt->width_list[j] = rt->width_list[i];
				rt->tstart_list[j] = rt->tstart_list[i];
				j++;
			}
		}
		rt->length = j;
	}

	if (NREV_GE(pi->pubpi.phy_rev, 3)) { /* nphy rev >= 3 */
/*
		ASSERT(rt->length <= 2*RDR_LIST_SIZE);
*/
		if (rt->length > 2*RDR_LIST_SIZE) {
			rt->length = 2*RDR_LIST_SIZE;
			PHY_RADAR(("WARNING: radar rt->length = %d > 2*RDR_LIST_SIZE = %d\n",
				rt->length, 2*RDR_LIST_SIZE));
		}
	} else {
/*
		ASSERT(rt->length <= RDR_LIST_SIZE);
*/
		if (rt->length > RDR_LIST_SIZE) {
			rt->length = RDR_LIST_SIZE;
			PHY_RADAR(("WARNING: radar rt->length = %d > RDR_LIST_SIZE = %d\n",
				rt->length, RDR_LIST_SIZE));
		}
	}

	/*
	 * Break pulses into epochs.
	 */
	rt->nepochs = 1;
	rt->epoch_start[0] = 0;
	for (i = 1; i < rt->length; i++) {
		if ((int32)(rt->tstart_list[i] - rt->tstart_list[i-1]) > rparams->max_blen) {
			rt->epoch_finish[rt->nepochs-1] = i - 1;
			rt->epoch_start[rt->nepochs] = i;
			rt->nepochs++;
		}
	}
	if (rt->nepochs > RDR_EPOCH_SIZE) {
		if (rparams->radar_args.feature_mask & 0x100) {
			PHY_RADAR(("WARNING: number of epochs %d > epoch size = %d\n",
				rt->nepochs, RDR_EPOCH_SIZE));
		}
/*
		return RADAR_TYPE_NONE;
*/
		epoch_length = RDR_NTIER_SIZE;
	}

	rt->epoch_finish[rt->nepochs - 1] = i;

	fra_det = FALSE;
	stg_det = 0;

	/*
	 * Run the detector for each epoch
	 */
	for (i = 0; i < rt->nepochs && (pulse_interval == 0) && !fra_det && (stg_det == 0); i++) {

		/*
		 * Generate 0th order tier list (time delta between received pulses)
		 * Quantize and filter delta pulse times delta pulse times are
		 * returned in sorted order from smallest to largest.
		 */
		epoch_list = rt->tstart_list + rt->epoch_start[i];
		epoch_length = (rt->epoch_finish[i] - rt->epoch_start[i] + 1);
		if (epoch_length > RDR_NTIER_SIZE) {
			if (rparams->radar_args.feature_mask & 0x100) {
				PHY_RADAR(("WARNING: DFS epoch_length=%d > TIER_SIZE=%d!!\n",
					epoch_length, RDR_NTIER_SIZE));
			}
/*
			return RADAR_TYPE_NONE;
*/
			epoch_length = RDR_NTIER_SIZE;
		}

		wlc_phy_radar_detect_run_epoch_nphy(pi, i, rt, rparams, epoch_list, epoch_length,
			fra_t1, fra_t2, fra_t3, fra_err,
			pw_2us, pw15us, pw20us, pw30us,
			i250us, i500us, i625us, i5000us,
			pw2us, i833us, i2500us, i3333us,
			i938us, i3066us,
			&fra_det, &stg_det, &pulse_interval,
			&nconsecq_pulses, &detected_pulse_index,
			&min_detected_pw, &max_detected_pw);
	}

	if (rparams->radar_args.feature_mask & 0x2) {
		/*	Changed to display intervals instead of tstart
			PHY_RADAR(("Start Time:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%u ", rt->tstart_list[i]));
		*/
		PHY_RADAR(("\nUnquantized unquantized short pulses"));
		PHY_RADAR(("\nInterval:  "));
		for (i = 1; i < rt->length; i++)
			PHY_RADAR(("%d-%d ", rt->tstart_list[i] - rt->tstart_list[i - 1], i));
		PHY_RADAR(("\n"));

		PHY_RADAR(("Pulse Widths:  "));
		for (i = 0; i < rt->length; i++)
			PHY_RADAR(("%i-%d ", rt->width_list[i], i));
		PHY_RADAR(("\n"));
	}

	if (rparams->radar_args.feature_mask & 0x80) {
		PHY_RADAR(("\nQuantized Intv: "));
		for (i = 0; i < epoch_length-2; i++)
			PHY_RADAR(("%d-%d ", rt->tiern_list[0][i], i));
		PHY_RADAR(("\n"));

		PHY_RADAR(("Pruned PW:  "));
		for (i = 0; i <  epoch_length-1; i++)
			PHY_RADAR(("%i-%d ", rt->tiern_pw[0][i], i));
		PHY_RADAR(("\n"));
		PHY_RADAR(("nconsecq_pulses=%d max_pw_delta=%d min_pw=%d max_pw=%d \n",
			nconsecq_pulses, max_detected_pw - min_detected_pw, min_detected_pw,
			max_detected_pw));
	}

	epoch_detected = i;

	if (pulse_interval || (fra_det) || stg_det != 0) {
		int radar_type = RADAR_TYPE_UNCLASSIFIED;
		const char *radar_type_str = "UNCLASSIFIED";

		if (fra_det) {
			radar_type = RADAR_TYPE_FRA;
			radar_type_str = "S3F RADAR";
		} else if (stg_det == 2) {
			radar_type = RADAR_TYPE_STG2;
			radar_type_str = "S2 RADAR";
		} else if (stg_det == 3) {
			radar_type = RADAR_TYPE_STG3;
			radar_type_str = "S3 RADAR";
		} else for (i = 0; i < radar_class[i].id; i++) {
			if (pulse_interval == radar_class[i].pri) {
				radar_type = radar_class[i].id;
				radar_type_str = (char *)radar_class[i].name;
				break;
			}
		}

		if (rparams->radar_args.feature_mask & 0x8) {
			/*	Changed to display intervals instead of tstart
				PHY_RADAR(("Start Time:  "));
				for (i = 0; i < rt->length; i++)
					PHY_RADAR(("%u ", rt->tstart_list[i]));
			*/
			PHY_RADAR(("\nInterval:  "));
			for (i = 1; i < rt->length; i++)
				PHY_RADAR(("%d-%d ",
					rt->tstart_list[i] - rt->tstart_list[i - 1], i));
			PHY_RADAR(("\n"));

			PHY_RADAR(("Pulse Widths:  "));
			for (i = 0; i < rt->length; i++)
				PHY_RADAR(("%i-%d ", rt->width_list[i], i));
			PHY_RADAR(("\n"));
		}

		PHY_RADAR(("\nQuantized Intv: "));
		for (i = 0; i < epoch_length-1; i++)
			PHY_RADAR(("%d-%d ", rt->tiern_list[0][i], i));
		PHY_RADAR(("\n"));

		PHY_RADAR(("Pruned PW:  "));
		for (i = 0; i <  epoch_length-1; i++)
			PHY_RADAR(("%i-%d ", rt->tiern_pw[0][i], i));
		PHY_RADAR(("\n"));

		PHY_RADAR(("Nepochs=%d len=%d epoch_#=%d; det_idx=%d "
				"max_pw_delta=%d min_pw=%d max_pw=%d \n",
				rt->nepochs, epoch_length, epoch_detected, detected_pulse_index,
				max_detected_pw - min_detected_pw, min_detected_pw,
				max_detected_pw));

		deltat2 = (int32) ((rt->dfs_timer - rt->last_detection_time) >> 1);
		if (deltat2 < 0) {
			deltat2 =  deltat2 + 2147483647;
		}
		/* detection not valid if detected pulse index too large */
		if (detected_pulse_index < ((rparams->radar_args.ncontig) & 0xff) -
			rparams->radar_args.npulses) {
			rt->last_detection_time = rt->dfs_timer;

		}
		/* reject detection spaced more than 3 minutes and detected pulse index too larg */
		if ((deltat2 < 600) && (detected_pulse_index <
			((rparams->radar_args.ncontig) & 0xff)
			- rparams->radar_args.npulses)) {
			PHY_ERROR(("Unclassified Radar Detection."
				" Time from last detection = %u, = %dmin %dsec\n",
				deltat2*2, deltat2*15/3000, (deltat2*3/10)%60));
			return (radar_type + (min_detected_pw << 4) +  (pulse_interval << 14));
		} else {
			PHY_ERROR(("SKIPPED false Unclassified Radar Detection."
				" Time from last detection = %u, = %dmin %dsec",
				deltat2*2, deltat2*15/3000, (deltat2*3/10)%60));
			if (detected_pulse_index < ((rparams->radar_args.ncontig) & 0xff)
				- rparams->radar_args.npulses) {
				PHY_ERROR(("\n"));
			} else {
				PHY_ERROR((". Detected pulse index too high: %d\n",
					detected_pulse_index));
			}
			return (RADAR_TYPE_NONE);
		}
	}

	return (RADAR_TYPE_NONE);
}
#endif /* #if defined(AP) && defined(RADAR) */

/* Increase the loop bandwidth of the PLL in the demodulator.
 * Note that although this allows the demodulator to track the
 * received carrier frequency over a wider bandwidth, it may
 * cause the Rx sensitivity to decrease
 */
void
wlc_phy_freqtrack_start(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!ISGPHY(pi))
		return;

	wlc_phyreg_enter((wlc_phy_t*)pi);

	PHY_REG_LIST_START
		PHY_REG_WRITE_RAW_ENTRY(BPHY_COEFFS, 0xFFEA)
		PHY_REG_WRITE_RAW_ENTRY(BPHY_STEP, 0x0689)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_phyreg_exit((wlc_phy_t*)pi);
}


/* Restore the loop bandwidth of the PLL in the demodulator to the original value */
void
wlc_phy_freqtrack_end(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if (!ISGPHY(pi))
		return;

	wlc_phyreg_enter((wlc_phy_t*)pi);

	/* Restore the original values of the PHY registers */
	phy_reg_write(pi, BPHY_COEFFS, pi->freqtrack_saved_regs[0]);
	phy_reg_write(pi, BPHY_STEP, pi->freqtrack_saved_regs[1]);

	wlc_phyreg_exit((wlc_phy_t*)pi);
}

/* VCO calibration proc for 4318 */
static void
wlc_phy_vco_cal(phy_info_t *pi)
{
	/* check D11 is running on Fast Clock */
	if (D11REV_GE(pi->sh->corerev, 5))
		ASSERT(si_core_sflags(pi->sh->sih, 0, 0) & SISF_FCLKA);

	if ((RADIOID(pi->pubpi.radioid) == BCM2050_ID) &&
	    (RADIOREV(pi->pubpi.radiorev) == 8)) {
		chanspec_t old_chanspec = pi->radio_chanspec;
		wlc_phy_chanspec_set((wlc_phy_t*)pi, CHSPEC_CHANNEL(pi->radio_chanspec) > 7 ?
		CH20MHZ_CHSPEC(1) : CH20MHZ_CHSPEC(13));
		wlc_phy_chanspec_set((wlc_phy_t*)pi, old_chanspec);
	}
}

void
wlc_phy_set_deaf(wlc_phy_t *ppi, bool user_flag)
{
	phy_info_t *pi;
	pi = (phy_info_t*)ppi;

	if (ISLPPHY(pi))
		wlc_phy_set_deaf_lpphy(pi, (bool)1);
	else if (ISSSLPNPHY(pi))
		wlc_sslpnphy_deaf_mode(pi, TRUE);
	else if (ISLCNPHY(pi))
		wlc_lcnphy_deaf_mode(pi, TRUE);
	else if (ISLCN40PHY(pi))
		wlc_lcn40phy_deaf_mode(pi, TRUE);
	else if (ISNPHY(pi))
		wlc_nphy_deaf_mode(pi, TRUE);
	else if (ISHTPHY(pi))
		wlc_phy_deaf_htphy(pi, TRUE);
	else {
		PHY_ERROR(("%s: Not yet supported\n", __FUNCTION__));
		ASSERT(0);
	}
}

#if defined(WLTEST) || defined(AP)
static int
wlc_phy_iovar_perical_config(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr,	bool set)
{
	int err = BCME_OK;

	if (!set) {
		if (!ISNPHY(pi) && !ISHTPHY(pi) && !ISLCNPHY(pi))
			return BCME_UNSUPPORTED;	/* supported for n, ht and lcn phy only */

		*ret_int_ptr =  pi->phy_cal_mode;
	} else {
		if (!ISNPHY(pi) && !ISHTPHY(pi) && !ISLCNPHY(pi))
			return BCME_UNSUPPORTED;	/* supported for n, ht and lcn phy only */

		if (int_val == 0) {
			pi->phy_cal_mode = PHY_PERICAL_DISABLE;
		} else if (int_val == 1) {
			pi->phy_cal_mode = PHY_PERICAL_SPHASE;
		} else if (int_val == 2) {
			pi->phy_cal_mode = PHY_PERICAL_MPHASE;
		} else if (int_val == 3) {
			/* this mode is to disable normal periodic cal paths
			 *  only manual trigger(nphy_forcecal) can run it
			 */
			pi->phy_cal_mode = PHY_PERICAL_MANUAL;
		} else {
			err = BCME_RANGE;
			goto end;
		}
		wlc_phy_cal_perical_mphase_reset(pi);
	}
end:
	return err;
}
#endif	

#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX)
static int
wlc_phy_iovar_tempsense(phy_info_t *pi, int32 *ret_int_ptr)
{
	int err = BCME_OK;
	int32 int_val;

	if (ISNPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		*ret_int_ptr = (int32)wlc_phy_tempsense_nphy(pi);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
	} else if (ISHTPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		*ret_int_ptr = (int32)wlc_phy_tempsense_htphy(pi);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
	} else if (ISLPPHY(pi)) {
		int_val = wlc_phy_tempsense_lpphy(pi);
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else if (ISLCNPHY(pi)) {
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			int_val = (int32)wlc_lcnphy_tempsense(pi, 1);
		else
			int_val = wlc_lcnphy_tempsense_degree(pi, 1);
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else
		err = BCME_UNSUPPORTED;

	return err;
}

#endif	/* defined(BCMDBG) || defined(WLTEST) || defined(MACOSX) */

void
wlc_phy_clear_deaf(wlc_phy_t  *ppi, bool user_flag)
{
#if defined(WLTEST)
	phy_info_t *pi;
	pi = (phy_info_t*)ppi;

	if (ISLPPHY(pi))
		wlc_phy_clear_deaf_lpphy(pi, (bool)1);
	else if (ISSSLPNPHY(pi))
		wlc_sslpnphy_deaf_mode(pi, FALSE);
	else if (ISLCNPHY(pi))
		wlc_lcnphy_deaf_mode(pi, FALSE);
	else if (ISLCN40PHY(pi))
		wlc_lcn40phy_deaf_mode(pi, FALSE);
	else if (ISNPHY(pi))
		wlc_nphy_deaf_mode(pi, FALSE);
	else if (ISHTPHY(pi))
		wlc_phy_deaf_htphy(pi, FALSE);
	else {
		PHY_ERROR(("%s: Not yet supported\n", __FUNCTION__));
		ASSERT(0);
	}
#endif 
}

#if defined(WLTEST)
static int
wlc_phy_pkteng_stats_get(phy_info_t *pi, void *a, int alen)
{
	wl_pkteng_stats_t stats;
	uint16 rxstats_base;
	uint16 hi, lo;
	int i;

	if (D11REV_LT(pi->sh->corerev, 11))
		return BCME_UNSUPPORTED;

	if (!pi->sh->up) {
		return BCME_NOTUP;
	}

	PHY_INFORM(("Pkteng Stats Called\n"));

	/* Read with guard against carry */
	do {
		hi = wlapi_bmac_read_shm(pi->sh->physhim, M_PKTENG_FRMCNT_HI);
		lo = wlapi_bmac_read_shm(pi->sh->physhim, M_PKTENG_FRMCNT_LO);
	} while (hi != wlapi_bmac_read_shm(pi->sh->physhim, M_PKTENG_FRMCNT_HI));

	stats.lostfrmcnt = (hi << 16) | lo;

	if (ISLPPHY(pi) || ISNPHY(pi) || ISHTPHY(pi)) {
		stats.rssi = R_REG(pi->sh->osh, &pi->regs->rssi) & 0xff;
		if (stats.rssi > 127)
			stats.rssi -= 256;
		stats.snr = stats.rssi - (ISLPPHY(pi) ? PHY_NOISE_FIXED_VAL :
			PHY_NOISE_FIXED_VAL_NPHY);
	} else if (ISSSLPNPHY(pi)) {
		wlc_sslpnphy_pkteng_stats_get(pi, &stats);
	} else if (ISLCNPHY(pi)) {
		int16 rssi_lcn[4];
		int16 snr_a_lcn[4];
		int16 snr_b_lcn[4];
		uint8 gidx[4];
		int8 snr[4];
		phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
		uint16 lcnphyregs_shm_addr, rssi_addr[4], snr_a_addr[4], snr_b_addr[4];

		lcnphyregs_shm_addr = 2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCNPHYREGS_PTR);

		rssi_addr[0] = lcnphyregs_shm_addr + M_LCN_RSSI_0;
		rssi_addr[1] = lcnphyregs_shm_addr + M_LCN_RSSI_1;
		rssi_addr[2] = lcnphyregs_shm_addr + M_LCN_RSSI_2;
		rssi_addr[3] = lcnphyregs_shm_addr + M_LCN_RSSI_3;

		stats.rssi = 0;

		for (i = 0; i < 4; i++) {
			rssi_lcn[i] = (int8)wlapi_bmac_read_shm(pi->sh->physhim, rssi_addr[i]);
			gidx[i] =
				(wlapi_bmac_read_shm(pi->sh->physhim, rssi_addr[i]) & 0x7e00) >> 9;
			rssi_lcn[i] = rssi_lcn[i] + lcnphy_gain_index_offset_for_pkt_rssi[gidx[i]];

			if ((wlapi_bmac_read_shm(pi->sh->physhim, rssi_addr[i])) & 0x8000) {
				rssi_lcn[i] = rssi_lcn[i] + pi->rssi_corr_boardatten;
			}
			else
				rssi_lcn[i] = rssi_lcn[i] + pi->rssi_corr_normal;

			stats.rssi += rssi_lcn[i];
		}

		stats.rssi = stats.rssi >> 2;

		/* temperature compensation */
		stats.rssi = stats.rssi + pi_lcn->lcnphy_pkteng_rssi_slope;

		/* SNR */
		snr_a_addr[0] = lcnphyregs_shm_addr + M_LCN_SNR_A_0;
		snr_a_addr[1] = lcnphyregs_shm_addr + M_LCN_SNR_A_1;
		snr_a_addr[2] = lcnphyregs_shm_addr + M_LCN_SNR_A_2;
		snr_a_addr[3] = lcnphyregs_shm_addr + M_LCN_SNR_A_3;

		snr_b_addr[0] = lcnphyregs_shm_addr + M_LCN_SNR_B_0;
		snr_b_addr[1] = lcnphyregs_shm_addr + M_LCN_SNR_B_1;
		snr_b_addr[2] = lcnphyregs_shm_addr + M_LCN_SNR_B_2;
		snr_b_addr[3] = lcnphyregs_shm_addr + M_LCN_SNR_B_3;

		stats.snr = 0;
		for (i = 0; i < 4; i++) {
			snr_a_lcn[i] = wlapi_bmac_read_shm(pi->sh->physhim, snr_a_addr[i]);
			snr_b_lcn[i] = wlapi_bmac_read_shm(pi->sh->physhim, snr_b_addr[i]);
			snr[i] = ((snr_a_lcn[i] - snr_b_lcn[i])* 3) >> 5;
			if (snr[i] > 31)
				snr[i] = 31;
			stats.snr += snr[i];
		}
		stats.snr = stats.snr >> 2;
	} else {
		/* Not available */
		stats.rssi = stats.snr = 0;
	}

#if defined(WLNOKIA_NVMEM)
	/* Nokia NVMEM spec specifies the rssi offsets */
	stats.rssi = wlc_phy_upd_rssi_offset(pi, (int8)stats.rssi, pi->radio_chanspec);
#endif 

	/* rx pkt stats */
	rxstats_base = wlapi_bmac_read_shm(pi->sh->physhim, M_RXSTATS_BLK_PTR);
	for (i = 0; i <= NUM_80211_RATES; i++) {
		stats.rxpktcnt[i] = wlapi_bmac_read_shm(pi->sh->physhim, 2*(rxstats_base+i));
	}
	bcopy(&stats, a,
		(sizeof(wl_pkteng_stats_t) < (uint)alen) ? sizeof(wl_pkteng_stats_t) : (uint)alen);

	return BCME_OK;
}


static int
wlc_phy_table_get(phy_info_t *pi, int32 int_val, void *p, void *a)
{
	int32 tblInfo[4];
	lpphytbl_info_t tab;
	phytbl_info_t tab2;

	if (ISAPHY(pi) || ISGPHY(pi))
		return BCME_UNSUPPORTED;

	/* other PHY */
	bcopy(p, tblInfo, 3*sizeof(int32));

	if (ISLPPHY(pi)) {
		tab.tbl_ptr = &int_val;
		tab.tbl_len = 1;
		tab.tbl_id = (uint32)tblInfo[0];
		tab.tbl_offset = (uint32)tblInfo[1];
		tab.tbl_width = (uint32)tblInfo[2];
		tab.tbl_phywidth = (uint32)tblInfo[2];
		wlc_phy_table_read_lpphy(pi, &tab);
	} else {
		tab2.tbl_ptr = &int_val;
		tab2.tbl_len = 1;
		tab2.tbl_id = (uint32)tblInfo[0];
		tab2.tbl_offset = (uint32)tblInfo[1];
		tab2.tbl_width = (uint32)tblInfo[2];

		if (ISSSLPNPHY(pi)) {
			wlc_sslpnphy_read_table(pi, &tab2);
		} else if (ISNPHY(pi)) {
			if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
				wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK, MCTL_PHYLOCK);
			}
			wlc_phy_read_table_nphy(pi, &tab2);
			if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
				wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);
			}
		} else if (ISHTPHY(pi)) {
			wlc_phy_read_table_htphy(pi, &tab2);
		} else if (pi->pi_fptr.phyreadtable) {
			pi->pi_fptr.phyreadtable(pi, &tab2);
		}
	}
	bcopy(&int_val, a, sizeof(int_val));
	return BCME_OK;
}

static int
wlc_phy_table_set(phy_info_t *pi, int32 int_val, void *a)
{
	int32 tblInfo[4];
	lpphytbl_info_t tab;
	phytbl_info_t tab2;

	if (ISAPHY(pi) || ISGPHY(pi))
		return BCME_UNSUPPORTED;

	bcopy(a, tblInfo, 4*sizeof(int32));

	if (ISLPPHY(pi)) {
		int_val = tblInfo[3];
		tab.tbl_ptr = &int_val;
		tab.tbl_len = 1;
		tab.tbl_id = (uint32)tblInfo[0];
		tab.tbl_offset = (uint32)tblInfo[1];
		tab.tbl_width = (uint32)tblInfo[2];
		tab.tbl_phywidth = (uint32)tblInfo[2];
		wlc_phy_table_write_lpphy(pi, &tab);
	} else {
		int_val = tblInfo[3];
		tab2.tbl_ptr = &int_val;
		tab2.tbl_len = 1;
		tab2.tbl_id = (uint32)tblInfo[0];
		tab2.tbl_offset = (uint32)tblInfo[1];
		tab2.tbl_width = (uint32)tblInfo[2];

		if (ISSSLPNPHY(pi)) {
			wlc_sslpnphy_write_table(pi, &tab2);
		} else if (ISNPHY(pi)) {
			wlc_phy_write_table_nphy(pi, (&tab2));
		} else if (ISHTPHY(pi)) {
			wlc_phy_write_table_htphy(pi, (&tab2));
		} else if (pi->pi_fptr.phywritetable) {
			pi->pi_fptr.phywritetable(pi, &tab2);
		}
	}
	return BCME_OK;
}

static int
wlc_phy_iovar_idletssi(phy_info_t *pi, int32 *ret_int_ptr, bool type)
{
	/* no argument or type = 1 will do full tx_pwr_ctrl_init */
	/* type = 0 will do just idle_tssi_est */
	int err = BCME_OK;
	if (ISLCNPHY(pi))
		*ret_int_ptr = wlc_lcnphy_idle_tssi_est_iovar(pi, type);
	else if (ISLCN40PHY(pi))
		*ret_int_ptr = wlc_lcn40phy_idle_tssi_est_iovar(pi, type);
	return err;
}

static int
wlc_phy_iovar_txpwrindex_get(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr)
{
	int err = BCME_OK;
	phy_info_lpphy_t *pi_lp = pi->u.pi_lpphy;

	if (ISNPHY(pi)) {

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
			(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
			OSL_DELAY(1);
		}

		*ret_int_ptr = wlc_phy_txpwr_idx_get_nphy(pi);

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);

	} else if (ISHTPHY(pi)) {
		*ret_int_ptr = wlc_phy_txpwr_idx_get_htphy(pi);

	} else if (ISLPPHY(pi)) {
		if (wlc_phy_tpc_isenabled_lpphy(pi)) {
			/* Update current power index */
			wlc_phy_tx_pwr_update_npt_lpphy(pi);
			*ret_int_ptr = pi_lp->lpphy_tssi_idx;
		} else
			*ret_int_ptr = pi_lp->lpphy_tx_power_idx_override;
	} else if (ISSSLPNPHY(pi)) {
		*ret_int_ptr = wlc_sslpnphy_txpwr_idx_get(pi);

	} else if (ISLCNPHY(pi))
		*ret_int_ptr = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	else if (ISLCN40PHY(pi))
		*ret_int_ptr = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

	return err;
}


static int
wlc_phy_iovar_txpwrindex_set(phy_info_t *pi, void *p)
{
	int err = BCME_OK;
	uint32 txpwridx[PHY_CORE_MAX] = { 0x30 };
	int8 idx, core;
	int8 siso_int_val;
	phy_info_nphy_t *pi_nphy = NULL;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif

	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	bcopy(p, txpwridx, PHY_CORE_MAX * sizeof(uint32));
	siso_int_val = (int8)(txpwridx[0] & 0xff);

	if (ISNPHY(pi)) {
		FOREACH_CORE(pi, core) {
			idx = (int8)(txpwridx[core] & 0xff);
			pi_nphy->nphy_txpwrindex[core].index_internal = idx;
			wlc_phy_store_txindex_nphy(pi);
			wlc_phy_txpwr_index_nphy(pi, (1 << core), idx, TRUE);
		}
	} else if (ISHTPHY(pi)) {
		FOREACH_CORE(pi, core) {
			idx = (int8)(txpwridx[core] & 0xff);
			wlc_phy_txpwr_by_index_htphy(pi, (1 << core), idx);
		}
	} else if (ISLPPHY(pi)) {
		if (siso_int_val == -1) {
#if defined(PHYCAL_CACHING)
			ASSERT(ctx);
			wlc_phy_set_tx_pwr_ctrl_lpphy(pi, LPPHY_TX_PWR_CTRL_HW);
			/* Reset calibration */
			ctx->valid = FALSE;
#else
			phy_info_lpphy_t *pi_lp = pi->u.pi_lpphy;
			wlc_phy_set_tx_pwr_ctrl_lpphy(pi, LPPHY_TX_PWR_CTRL_HW);
			/* Reset calibration */
			pi_lp->lpphy_full_cal_channel = 0;
#endif
			pi->phy_forcecal = TRUE;
		} else if (siso_int_val >= 0) {
			wlc_phy_set_tx_pwr_by_index_lpphy(pi, siso_int_val);
		} else {
			err = BCME_RANGE;
		}
	} else if (ISSSLPNPHY(pi)) {
		phy_info_sslpnphy_t *ph = pi->u.pi_sslpnphy;
		if (siso_int_val == -1) {
			wlc_sslpnphy_set_tx_pwr_ctrl(pi, SSLPNPHY_TX_PWR_CTRL_HW);
			/* Reset calibration */
			ph->sslpnphy_full_cal_channel[CHSPEC_IS5G(pi->radio_chanspec) ? 1 : 0] = 0;
			pi->phy_forcecal = TRUE;
		} else if (siso_int_val >= 0) {
			wlc_sslpnphy_set_tx_pwr_by_index(pi, siso_int_val);
		} else {
			err = BCME_RANGE;
		}
	} else if (ISLCNCOMMONPHY(pi)) {
		err = wlc_iovar_txpwrindex_set_lcncommon(pi, siso_int_val);
	}

	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);

	return err;
}

static int
wlc_phy_iovar_vbatsense(phy_info_t *pi, int32 *ret_int_ptr)
{
	int err = BCME_OK;
	int32 int_val;

	if (ISLCNPHY(pi)) {
		int_val = wlc_lcnphy_vbatsense(pi, 1);
		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
	} else
		err = BCME_UNSUPPORTED;

	return err;
}
#endif 
//#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV)--delete by ll
static int
wlc_phy_iovar_forcecal(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set)
{
	int err = BCME_OK;
	void *a;
	phy_info_sslpnphy_t *pi_ssn = pi->u.pi_sslpnphy;
	uint8 mphase = 0, searchmode = 0;
	phy_info_nphy_t *pi_nphy = NULL;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif

	a = (int32*)ret_int_ptr;
	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;
	if (ISHTPHY(pi)) {
		/* for get with no argument, assume 0x00 */
		if (!set)
			int_val = 0x00;

		/* upper nibble: 0 = sphase,  1 = mphase */
		mphase = (((uint8) int_val) & 0xf0) >> 4;

		/* 0 = RESTART, 1 = REFINE, for Tx-iq/lo-cal */
		searchmode = ((uint8) int_val) & 0xf;

		PHY_CAL(("wlc_phy_iovar_forcecal (mphase = %d, refine = %d)\n",
			mphase, searchmode));

		/* call sphase or schedule mphase cal */
		wlc_phy_cal_perical_mphase_reset(pi);
		if (mphase) {
			pi->cal_info->cal_searchmode = searchmode;
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
		} else {
			wlc_phy_cals_htphy(pi, searchmode);
		}

	} else if (ISNPHY(pi)) {
		/* for get with no argument, assume 0x00 */
		if (!set)
			int_val = PHY_PERICAL_AUTO;

		if ((int_val == PHY_PERICAL_PARTIAL) ||
		    (int_val == PHY_PERICAL_AUTO) ||
		    (int_val == PHY_PERICAL_FULL)) {
			wlc_phy_cal_perical_mphase_reset(pi);
			pi->u.pi_nphy->cal_type_override = (uint8)int_val;
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
#ifdef NOISE_CAL_LCNXNPHY
			if ((NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
			NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 2))) {
				/* If multiphase cal is not completed wait for
				 * it to complete before noise cal is
				 * triggered
				 */
				wlc_phy_noise_cal_init_nphy(pi);
				wlc_phy_noise_cal_measure_nphy(pi);
			}
#endif
		} else
			err = BCME_RANGE;

	} else if (ISLPPHY(pi)) {
		/* don't care get or set with argument */
#if defined(PHYCAL_CACHING)
		ASSERT(ctx);
		ctx->valid = FALSE;
#else
		pi->u.pi_lpphy->lpphy_full_cal_channel = 0;
#endif
		wlc_phy_periodic_cal_lpphy(pi);
		int_val = 0;
		bcopy(&int_val, a, vsize);

	} else if (ISSSLPNPHY(pi)) {
		if (int_val != 0)
			pi->phy_forcecal = TRUE;
		pi_ssn->sslpnphy_full_cal_channel[CHSPEC_IS5G(pi->radio_chanspec) ? 1 : 0] = 0;
		wlc_sslpnphy_noise_measure((wlc_phy_t *)pi);

	} else if (ISLCNCOMMONPHY(pi) && pi->pi_fptr.calibmodes) {
#if defined(PHYCAL_CACHING)
		ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#else
		phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif
		if (!set)
			*ret_int_ptr = 0;

		/* Wl forcecal to perform cal even when no param specified */
#if defined(PHYCAL_CACHING)
		ASSERT(ctx);
		ctx->valid = FALSE;
#else
		pi_lcn->lcnphy_full_cal_channel = 0;
#endif
		pi->pi_fptr.calibmodes(pi, PHY_FULLCAL);
		int_val = 0;
		bcopy(&int_val, a, vsize);
	}

	return err;
}
//#endif -- delete by ll
static int
wlc_phy_iovar_oclscd(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	int err = BCME_OK;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;
	else if (pi->pubpi.phy_rev != LCNXN_BASEREV + 2)
		return BCME_UNSUPPORTED;

	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = pi->nphy_oclscd;

		}

	} else {
		if (ISNPHY(pi)) {
			/* if not up, we are done */
			if (pi->sh->up)
				return BCME_NOTDOWN;
			pi->nphy_oclscd = (uint8)int_val;

		}
	}
	return err;
}

#if 1//add by ll
static int
wlc_phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)

{
	int err = BCME_OK;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#else
	phy_info_lpphy_t *pi_lp = pi->u.pi_lpphy;
#endif


	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = pi->nphy_txpwrctrl;
		} else if (ISHTPHY(pi)) {
			*ret_int_ptr = pi->txpwrctrl;
		} else if (ISLPPHY(pi)) {
			*ret_int_ptr = wlc_phy_tpc_isenabled_lpphy(pi);
		} else if (ISSSLPNPHY(pi)) {
			*ret_int_ptr = wlc_phy_tpc_isenabled_sslpnphy(pi);
		} else if (ISLCNPHY(pi)) {
			*ret_int_ptr = wlc_phy_tpc_iovar_isenabled_lcnphy(pi);
		} else if (pi->pi_fptr.ishwtxpwrctrl) {
			*ret_int_ptr = pi->pi_fptr.ishwtxpwrctrl(pi);
		}

	} else {
		if (ISNPHY(pi) || ISHTPHY(pi)) {
			if ((int_val != PHY_TPC_HW_OFF) && (int_val != PHY_TPC_HW_ON)) {
				err = BCME_RANGE;
				goto end;
			}

			pi->nphy_txpwrctrl = (uint8)int_val;
			pi->txpwrctrl = (uint8)int_val;

			/* if not up, we are done */
			if (!pi->sh->up)
				goto end;

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);
			if (ISNPHY(pi))
				wlc_phy_txpwrctrl_enable_nphy(pi, (uint8) int_val);
			else if (ISHTPHY(pi))
				wlc_phy_txpwrctrl_enable_htphy(pi, (uint8) int_val);
			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);

		} else if (ISLPPHY(pi)) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);

			wlc_phy_set_tx_pwr_ctrl_lpphy(pi,
				int_val ? LPPHY_TX_PWR_CTRL_HW : LPPHY_TX_PWR_CTRL_SW);
			/* Reset calibration */
#if defined(PHYCAL_CACHING)
			ASSERT(ctx);
			ctx->valid = FALSE;
#else
			pi_lp->lpphy_full_cal_channel = 0;
#endif
			pi->phy_forcecal = TRUE;

			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);
		} else if (ISSSLPNPHY(pi)) {
			wlc_sslpnphy_iovar_txpwrctrl(pi, int_val);

		} else if (ISLCNPHY(pi)) {
			wlc_lcnphy_iovar_txpwrctrl(pi, int_val, ret_int_ptr);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_iovar_txpwrctrl(pi, int_val, ret_int_ptr);
		}
	}

end:
	return err;
}

#endif
#if defined(WLTEST)
static void
wlc_phy_iovar_set_deaf(phy_info_t *pi, int32 int_val)
{
	if (int_val) {
		wlc_phy_set_deaf((wlc_phy_t *) pi, TRUE);
	} else {
		wlc_phy_clear_deaf((wlc_phy_t *) pi, TRUE);
	}
}
#if 0
static int
wlc_phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	int err = BCME_OK;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#else
	phy_info_lpphy_t *pi_lp = pi->u.pi_lpphy;
#endif


	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = pi->nphy_txpwrctrl;
		} else if (ISHTPHY(pi)) {
			*ret_int_ptr = pi->txpwrctrl;
		} else if (ISLPPHY(pi)) {
			*ret_int_ptr = wlc_phy_tpc_isenabled_lpphy(pi);
		} else if (ISSSLPNPHY(pi)) {
			*ret_int_ptr = wlc_phy_tpc_isenabled_sslpnphy(pi);
		} else if (ISLCNPHY(pi)) {
			*ret_int_ptr = wlc_phy_tpc_iovar_isenabled_lcnphy(pi);
		} else if (pi->pi_fptr.ishwtxpwrctrl) {
			*ret_int_ptr = pi->pi_fptr.ishwtxpwrctrl(pi);
		}

	} else {
		if (ISNPHY(pi) || ISHTPHY(pi)) {
			if ((int_val != PHY_TPC_HW_OFF) && (int_val != PHY_TPC_HW_ON)) {
				err = BCME_RANGE;
				goto end;
			}

			pi->nphy_txpwrctrl = (uint8)int_val;
			pi->txpwrctrl = (uint8)int_val;

			/* if not up, we are done */
			if (!pi->sh->up)
				goto end;

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);
			if (ISNPHY(pi))
				wlc_phy_txpwrctrl_enable_nphy(pi, (uint8) int_val);
			else if (ISHTPHY(pi))
				wlc_phy_txpwrctrl_enable_htphy(pi, (uint8) int_val);
			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);

		} else if (ISLPPHY(pi)) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);

			wlc_phy_set_tx_pwr_ctrl_lpphy(pi,
				int_val ? LPPHY_TX_PWR_CTRL_HW : LPPHY_TX_PWR_CTRL_SW);
			/* Reset calibration */
#if defined(PHYCAL_CACHING)
			ASSERT(ctx);
			ctx->valid = FALSE;
#else
			pi_lp->lpphy_full_cal_channel = 0;
#endif
			pi->phy_forcecal = TRUE;

			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);
		} else if (ISSSLPNPHY(pi)) {
			wlc_sslpnphy_iovar_txpwrctrl(pi, int_val);

		} else if (ISLCNPHY(pi)) {
			wlc_lcnphy_iovar_txpwrctrl(pi, int_val, ret_int_ptr);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_iovar_txpwrctrl(pi, int_val, ret_int_ptr);
		}
	}

end:
	return err;
}
#endif
static int
wlc_phy_iovar_txrx_chain(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, bool set)
{
	int err = BCME_OK;

	if (ISHTPHY(pi))
		return BCME_UNSUPPORTED;

	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = (int)pi->nphy_txrx_chain;
		}
	} else {
		if (ISNPHY(pi)) {
			if ((int_val != AUTO) && (int_val != WLC_N_TXRX_CHAIN0) &&
				(int_val != WLC_N_TXRX_CHAIN1)) {
				err = BCME_RANGE;
				goto end;
			}

			if (pi->nphy_txrx_chain != (int8)int_val) {
				pi->nphy_txrx_chain = (int8)int_val;
				if (pi->sh->up) {
					wlapi_suspend_mac_and_wait(pi->sh->physhim);
					wlc_phyreg_enter((wlc_phy_t *)pi);
					wlc_phy_stf_chain_upd_nphy(pi);
					wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);
					wlc_phyreg_exit((wlc_phy_t *)pi);
					wlapi_enable_mac(pi->sh->physhim);
				}
			}
		}
	}
end:
	return err;
}

static void
wlc_phy_iovar_bphy_testpattern(phy_info_t *pi, uint8 testpattern, bool enable)
{
	bool existing_enable = FALSE;

	/* WL out check */
	if (pi->sh->up) {
		PHY_ERROR(("wl%d: %s: This function needs to be called after 'wl out'\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	/* confirm band is locked to 2G */
	if (!CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_ERROR(("wl%d: %s: Band needs to be locked to 2G (b)\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	if (NREV_LT(pi->pubpi.phy_rev, 2) || ISHTPHY(pi)) {
		PHY_ERROR(("wl%d: %s: This function is supported only for NPHY PHY_REV > 1\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	if (testpattern == NPHY_TESTPATTERN_BPHY_EVM) {    /* CW CCK for EVM testing */
		existing_enable = (bool) pi->phy_bphy_evm;
	} else if (testpattern == NPHY_TESTPATTERN_BPHY_RFCS) { /* RFCS testpattern */
		existing_enable = (bool) pi->phy_bphy_rfcs;
	} else {
		PHY_ERROR(("Testpattern needs to be between [0 (BPHY_EVM), 1 (BPHY_RFCS)]\n"));
		ASSERT(0);
	}

	if (ISNPHY(pi)) {
		wlc_phy_bphy_testpattern_nphy(pi, testpattern, enable, existing_enable);
	} else {
		PHY_ERROR(("support yet to be added\n"));
		ASSERT(0);
	}

	/* Return state of testpattern enables */
	if (testpattern == NPHY_TESTPATTERN_BPHY_EVM) {    /* CW CCK for EVM testing */
		pi->phy_bphy_evm = enable;
	} else if (testpattern == NPHY_TESTPATTERN_BPHY_RFCS) { /* RFCS testpattern */
		pi->phy_bphy_rfcs = enable;
	}
}

static void
wlc_phy_iovar_scraminit(phy_info_t *pi, int8 scraminit)
{
	pi->phy_scraminit = (int8)scraminit;
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	if (ISNPHY(pi)) {
		wlc_phy_test_scraminit_nphy(pi, scraminit);
	} else if (ISHTPHY(pi)) {
		wlc_phy_test_scraminit_htphy(pi, scraminit);
	} else {
		PHY_ERROR(("support yet to be added\n"));
		ASSERT(0);
	}

	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);
}

static void
wlc_phy_iovar_force_rfseq(phy_info_t *pi, uint8 int_val)
{
	wlc_phyreg_enter((wlc_phy_t *)pi);
	if (ISNPHY(pi)) {
		wlc_phy_force_rfseq_nphy(pi, int_val);
	} else if (ISHTPHY(pi)) {
		wlc_phy_force_rfseq_htphy(pi, int_val);
	}
	wlc_phyreg_exit((wlc_phy_t *)pi);
}

static void
wlc_phy_iovar_tx_tone(phy_info_t *pi, int32 int_val)
{
	pi->phy_tx_tone_freq = (int32) int_val;

	if (pi->phy_tx_tone_freq == 0) {
		if (ISNPHY(pi)) {
			/* Restore back PAPD settings after stopping the tone */
			if (pi->pubpi.phy_rev == LCNXN_BASEREV)
				wlc_phy_papd_enable_nphy(pi, TRUE);
			wlc_phy_stopplayback_nphy(pi);
			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
		} else if (ISHTPHY(pi)) {
			wlc_phy_stopplayback_htphy(pi);
			wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);
		} else if (ISLCNPHY(pi)) {
			wlc_lcnphy_stop_tx_tone(pi);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_stop_tx_tone(pi);
		}
	} else {
		if (ISNPHY(pi)) {
			/* use 151 since that should correspond to nominal tx output power */
			/* Can not play tone with papd bit enabled */
			if (pi->pubpi.phy_rev == LCNXN_BASEREV)
				wlc_phy_papd_enable_nphy(pi, FALSE);
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);
			wlc_phy_tx_tone_nphy(pi, (uint32)int_val, 151, 0, 0, TRUE); /* play tone */
		} else if (ISHTPHY(pi)) {
			wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);
			wlc_phy_tx_tone_htphy(pi, (uint32)int_val, 151, 0, 0, TRUE); /* play tone */
		} else if (ISLCNPHY(pi)) {
			pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
			wlc_lcnphy_set_tx_tone_and_gain_idx(pi);
		} else if (ISLCN40PHY(pi)) {
			pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_lcn40phy_set_tx_tone_and_gain_idx(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
	}
}

static void
wlc_phy_iovar_tx_tone_hz(phy_info_t *pi, int32 int_val)
{
	pi->phy_tx_tone_freq = (int32) int_val;

	if (pi->phy_tx_tone_freq == 0) {
		if (ISLCNPHY(pi)) {
			wlc_lcnphy_stop_tx_tone(pi);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_stop_tx_tone(pi);
		}
	} else {
		if (ISLCNPHY(pi)) {
			wlc_lcnphy_set_tx_tone_and_gain_idx(pi);
		} else if (ISLCN40PHY(pi)) {
			wlc_lcn40phy_set_tx_tone_and_gain_idx(pi);
		}
	}
}

static int8
wlc_phy_iovar_test_tssi(phy_info_t *pi, uint8 val, uint8 pwroffset)
{
	int8 tssi = 0;
	if (ISNPHY(pi)) {
		tssi = wlc_phy_test_tssi_nphy(pi, val, pwroffset);
	} else if (ISHTPHY(pi)) {
		tssi = wlc_phy_test_tssi_htphy(pi, val, pwroffset);
	}
	return tssi;
}

static void
wlc_phy_iovar_rxcore_enable(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	if (set) {
		if (ISNPHY(pi)) {
			wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, (uint8) int_val, 0);
		} else if (ISHTPHY(pi)) {
			wlc_phy_rxcore_setstate_htphy((wlc_phy_t *)pi, (uint8) int_val);
		}
	} else {
		if (ISNPHY(pi)) {
			*ret_int_ptr =  (uint32)wlc_phy_rxcore_getstate_nphy((wlc_phy_t *)pi);
		} else if (ISHTPHY(pi)) {
			*ret_int_ptr =  (uint32)wlc_phy_rxcore_getstate_htphy((wlc_phy_t *)pi);
		}
	}

	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);
}

#endif 

/* Dispatch phy iovars */
int
wlc_phy_iovar_dispatch(wlc_phy_t *pih, uint32 actionid, uint16 type, void *p, uint plen, void *a,
	int alen, int vsize)
{
	phy_info_t *pi = (phy_info_t*)pih;
	phy_info_nphy_t *pi_nphy = NULL;
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;
	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	switch (actionid) {
#if defined(AP) && defined(RADAR)
	case IOV_GVAL(IOV_RADAR_ARGS):
		bcopy((char*)&pi->rargs[0].radar_args, (char*)a, sizeof(wl_radar_args_t));
		break;

	case IOV_GVAL(IOV_RADAR_ARGS_40MHZ):
		/* any other phy supports 40Mhz channel ? */
		if (!(ISNPHY(pi) || ISHTPHY(pi))) {
			err = BCME_UNSUPPORTED;
			break;
		}
		bcopy((char*)&pi->rargs[1].radar_args, (char*)a, sizeof(wl_radar_args_t));
		break;

	case IOV_SVAL(IOV_RADAR_THRS):
		if (ISAPHY(pi) || ISLPPHY(pi) || ISNPHY(pi) || ISHTPHY(pi)) {
			wl_radar_thr_t radar_thr;

			/* len is check done before gets here */
			bzero((char*)&radar_thr, sizeof(wl_radar_thr_t));
			bcopy((char*)p, (char*)&radar_thr, sizeof(wl_radar_thr_t));
			if (radar_thr.version != WL_RADAR_THR_VERSION) {
				err = BCME_VERSION;
				break;
			}
			pi->rargs[0].radar_thrs.thresh0_20_lo = radar_thr.thresh0_20_lo;
			pi->rargs[0].radar_thrs.thresh1_20_lo = radar_thr.thresh1_20_lo;
			pi->rargs[0].radar_thrs.thresh0_20_hi = radar_thr.thresh0_20_hi;
			pi->rargs[0].radar_thrs.thresh1_20_hi = radar_thr.thresh1_20_hi;
			if (ISNPHY(pi) || ISHTPHY(pi)) {
				pi->rargs[1].radar_thrs.thresh0_40_lo = radar_thr.thresh0_40_lo;
				pi->rargs[1].radar_thrs.thresh1_40_lo = radar_thr.thresh1_40_lo;
				pi->rargs[1].radar_thrs.thresh0_40_hi = radar_thr.thresh0_40_hi;
				pi->rargs[1].radar_thrs.thresh1_40_hi = radar_thr.thresh1_40_hi;
			}
			wlc_phy_radar_detect_init(pi, pi->sh->radar);
		} else
			err = BCME_UNSUPPORTED;
		break;

#if defined(BCMDBG) || defined(WLTEST)
	case IOV_SVAL(IOV_RADAR_ARGS):
		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}

		if (ISAPHY(pi) || ISLPPHY(pi) || ISNPHY(pi) || ISHTPHY(pi)) {
			wl_radar_args_t radarargs;

			/* len is check done before gets here */
			bcopy((char*)p, (char*)&radarargs, sizeof(wl_radar_args_t));
			if (radarargs.version != WL_RADAR_ARGS_VERSION) {
				err = BCME_VERSION;
				break;
			}
			bcopy((char*)&radarargs, (char*)&pi->rargs[0].radar_args,
			      sizeof(wl_radar_args_t));
			/* apply radar inits to hardware if we are on the A/LP/NPHY */
			wlc_phy_radar_detect_init(pi, pi->sh->radar);
		} else
			err = BCME_UNSUPPORTED;

		break;

	case IOV_SVAL(IOV_RADAR_ARGS_40MHZ):
		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}

		/* any other phy supports 40Mhz channel ? */
		if (ISNPHY(pi) || ISHTPHY(pi)) {
			wl_radar_args_t radarargs;

			/* len check done before gets here */
			bcopy((char*)p, (char*)&radarargs, sizeof(wl_radar_args_t));
			if (radarargs.version != WL_RADAR_ARGS_VERSION) {
				err = BCME_VERSION;
				break;
			}
			bcopy((char*)&radarargs, (char*)&pi->rargs[1].radar_args,
			      sizeof(wl_radar_args_t));
			/* apply radar inits to hardware if we are NPHY */
			wlc_phy_radar_detect_init(pi, pi->sh->radar);
		} else
			err = BCME_UNSUPPORTED;

		break;
	case IOV_SVAL(IOV_PHY_DFS_LP_BUFFER): {
		if (ISNPHY(pi) || ISHTPHY(pi)) {
			pi->dfs_lp_buffer_nphy = bool_val;
		} else
			err = BCME_UNSUPPORTED;
		break;
	}
#endif 
#endif /* defined(AP) && defined(RADAR) */

#if defined(BCMDBG) || defined(WLTEST)
	case IOV_GVAL(IOV_FAST_TIMER):
		*ret_int_ptr = (int32)pi->sh->fast_timer;
		break;

	case IOV_SVAL(IOV_FAST_TIMER):
		pi->sh->fast_timer = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_SLOW_TIMER):
		*ret_int_ptr = (int32)pi->sh->slow_timer;
		break;

	case IOV_SVAL(IOV_SLOW_TIMER):
		pi->sh->slow_timer = (uint32)int_val;
		break;
#endif /* BCMDBG || WLTEST */
#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX)
	case IOV_GVAL(IOV_PHY_TEMPSENSE):
		wlc_phy_iovar_tempsense(pi, ret_int_ptr);
		break;
#endif /* BCMDBG || WLTEST || MACOSX */

#if defined(BCMDBG) || defined(WLTEST) || defined(WLMEDIA_CALDBG)
	case IOV_GVAL(IOV_GLACIAL_TIMER):
		*ret_int_ptr = (int32)pi->sh->glacial_timer;
		break;

	case IOV_SVAL(IOV_GLACIAL_TIMER):
		pi->sh->glacial_timer = (uint32)int_val;
		break;
#endif /* BCMDBG || WLTEST  || WLMEDIA_CALDBG */

#if defined(BCMDBG) || defined(WLTEST)
	case IOV_GVAL(IOV_TXINSTPWR):
		wlc_phyreg_enter((wlc_phy_t *)pi);
		/* Return the current instantaneous est. power
		 * For swpwr ctrl it's based on current TSSI value (as opposed to average)
		 */
		wlc_phy_txpower_get_instant(pi, a);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		break;
#endif 

#ifdef SAMPLE_COLLECT
	case IOV_GVAL(IOV_PHY_SAMPLE_COLLECT):
	{
		wl_samplecollect_args_t samplecollect_args;

		if (plen < (int)sizeof(wl_samplecollect_args_t)) {
			PHY_ERROR(("plen (%d) < sizeof(wl_samplecollect_args_t) (%d)\n",
				plen, (int)sizeof(wl_samplecollect_args_t)));
			err = BCME_BUFTOOSHORT;
			break;
		}
		bcopy((char*)p, (char*)&samplecollect_args, sizeof(wl_samplecollect_args_t));
		if (samplecollect_args.version != WL_SAMPLECOLLECT_T_VERSION) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				samplecollect_args.version, WL_SAMPLECOLLECT_T_VERSION));
			err = BCME_BADARG;
			break;
		}

		if (ltoh16(samplecollect_args.length) > (uint16)alen) {
			PHY_ERROR(("Bad length, length requested > buf len (%d > %d)\n",
				samplecollect_args.length, alen));
			err = BCME_BADLEN;
			break;
		}

		err = wlc_phy_sample_collect(pi, &samplecollect_args, a);
		break;
	}
	case IOV_GVAL(IOV_PHY_SAMPLE_DATA):
	{
		wl_sampledata_t sampledata_args;

		if (plen < (int)sizeof(wl_sampledata_t)) {
			PHY_ERROR(("plen (%d) < sizeof(wl_samplecollect_args_t) (%d)\n",
				plen, (int)sizeof(wl_sampledata_t)));
			err = BCME_BUFTOOSHORT;
			break;
		}
		bcopy((char*)p, (char*)&sampledata_args, sizeof(wl_sampledata_t));

		if ((ltoh16(sampledata_args.version) != WL_SAMPLEDATA_T_VERSION) &&
		     (ltoh16(sampledata_args.version) != WL_SAMPLEDATA_T_VERSION_SPEC_AN)) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				sampledata_args.version, WL_SAMPLEDATA_T_VERSION));
			err = BCME_BADARG;
			break;
		}

		if (ltoh16(sampledata_args.length) > (uint16)alen) {
			PHY_TRACE(("length requested > buf len (%d > %d) limiting to buf len\n",
				sampledata_args.length, alen));
			/* limit the user request to alen */
			sampledata_args.length = htol16((uint16)alen);
		}

		err = wlc_phy_sample_data(pi, &sampledata_args, a);
		break;
	}
#endif /* SAMPLE_COLLECT */

#if defined(WLTEST)

	case IOV_SVAL(IOV_TSSICAL_START_IDX):
	case IOV_SVAL(IOV_TSSICAL_START):
		if (ISLCNPHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			uint16 curr_anchor;
			if (!pi->ptssi_cal) {
				pi->ptssi_cal = (tssi_cal_info_t *)MALLOC(pi->sh->osh,
					sizeof(tssi_cal_info_t));
				if (pi->ptssi_cal == NULL) {
					PHY_ERROR(("wl%d: %s: MALLOC failure\n",
						pi->sh->unit, __FUNCTION__));
					err = BCME_UNSUPPORTED;
					break;
				}
				bzero((char *)pi->ptssi_cal, sizeof(tssi_cal_info_t));
			}
			curr_anchor = pi->ptssi_cal->curr_anchor;
			wlc_lcnphy_clear_tx_power_offsets(pi);
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlRangeCmd, cckPwrOffset,
				pi_lcn->cckPwrOffset);
			PHY_REG_MOD(pi, LCNPHY, TxPwrCtrlCmd, txPwrCtrl_en, 1);

			if (actionid == IOV_SVAL(IOV_TSSICAL_START_IDX))
				bcopy(p, &(pi->ptssi_cal->anchor_txidx[curr_anchor]),
					sizeof(uint16));
			else
				bcopy(p, &(pi->ptssi_cal->target_pwr_qdBm[curr_anchor]),
					sizeof(int));

			pi->ptssi_cal->paparams_calc_done = 0;
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_TSSICAL_POWER):
		if (ISLCNPHY(pi)) {
			uint16 curr_anchor;
			if (!pi->ptssi_cal) {
				err = BCME_UNSUPPORTED;
				break;
			}
			curr_anchor = pi->ptssi_cal->curr_anchor;
			bcopy(p, &(pi->ptssi_cal->measured_pwr_qdBm[curr_anchor]), sizeof(int));
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_TSSICAL_POWER):
		if (ISLCNPHY(pi)) {
			uint16 anchor_var[3];
			uint16 curr_anchor;
			if (!pi->ptssi_cal) {
				err = BCME_UNSUPPORTED;
				break;
			}
			curr_anchor = pi->ptssi_cal->curr_anchor;
			pi->ptssi_cal->anchor_txidx[curr_anchor] =
				wlc_lcnphy_get_current_tx_pwr_idx(pi);
			pi->ptssi_cal->anchor_tssi[curr_anchor] =
				PHY_REG_READ(pi, LCNPHY, TxPwrCtrlStatusNew4, avgTssi);
			pi->ptssi_cal->anchor_bbmult[curr_anchor] =
				wlc_lcnphy_get_bbmult_from_index(pi,
				pi->ptssi_cal->anchor_txidx[curr_anchor]);

			anchor_var[0] = pi->ptssi_cal->anchor_bbmult[curr_anchor];
			anchor_var[1] = pi->ptssi_cal->anchor_txidx[curr_anchor];
			anchor_var[2] = pi->ptssi_cal->anchor_tssi[curr_anchor];

			bcopy(anchor_var, a, 3*sizeof(uint16));

			pi->ptssi_cal->curr_anchor++;
			if (pi->ptssi_cal->curr_anchor >= MAX_NUM_ANCHORS)
				pi->ptssi_cal->curr_anchor = 0;

		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_TSSICAL_PARAMS):
		if (ISLCNPHY(pi)) {
			uint16 num_anchor;
			if (!pi->ptssi_cal) {
				err = BCME_UNSUPPORTED;
				break;
			}
			if ((!pi->ptssi_cal->paparams_calc_done) &&
				(!pi->ptssi_cal->paparams_calc_in_progress)) {

				if (pi->ptssi_cal->anchor_bbmult[0]) { /* Atleast One Anchor */

					pi->ptssi_cal->paparams_calc_in_progress = 1;
					wlc_phy_tssi_cal(pi);

					pi->ptssi_cal->paparams_new[0] =
						pi->ptssi_cal->rsd.c4[0][0];
					pi->ptssi_cal->paparams_new[1] =
						pi->ptssi_cal->rsd.c4[1][0];
					pi->ptssi_cal->paparams_new[2] =
						pi->ptssi_cal->rsd.c4[2][0];
					pi->ptssi_cal->paparams_new[3] =
						pi->ptssi_cal->rsd.det_c1;
				}
				else {
					pi->ptssi_cal->paparams_new[0] = 1;
					pi->ptssi_cal->paparams_new[1] = 1;
					pi->ptssi_cal->paparams_new[2] = 1;
					pi->ptssi_cal->paparams_new[3] = 1;
				}

				pi->ptssi_cal->paparams_calc_done = 1;
			}

			if (pi->ptssi_cal->paparams_calc_done) {
				bcopy(pi->ptssi_cal->paparams_new, a, 4*sizeof(int64));
				pi->ptssi_cal->paparams_calc_in_progress = 0;
				pi->ptssi_cal->curr_anchor = 0;
				for (num_anchor = 0; num_anchor < MAX_NUM_ANCHORS; num_anchor++)
					pi->ptssi_cal->anchor_bbmult[num_anchor] = 0;
			}
		} else
			err = BCME_UNSUPPORTED;
		break;


	case IOV_SVAL(IOV_PHY_TSSITXDELAY):
	{
		if (ISLCNPHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			pi_lcn->lcnphy_tssical_txdelay = (uint32)int_val;
		}
	}
	break;

	case IOV_GVAL(IOV_PHY_TSSITXDELAY):
	{
		if (ISLCNPHY(pi)) {
			phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;
			int_val = pi_lcn->lcnphy_tssical_txdelay;
			bcopy(&int_val, a, sizeof(int_val));
		}
	}
	break;

	case IOV_GVAL(IOV_PHY_TXIQCC):
		{
			int32 iqccValues[2];
			uint16 valuea = 0;
			uint16 valueb = 0;
			txiqccgetfn_t txiqcc_fn = pi->pi_fptr.txiqccget;
			if (txiqcc_fn) {
				(*txiqcc_fn)(pi, &valuea, &valueb);
				iqccValues[0] = valuea;
				iqccValues[1] = valueb;
				bcopy(iqccValues, a, 2*sizeof(int32));
			}
		}
		break;

	case IOV_SVAL(IOV_PHY_TXIQCC):
		{
			int32 iqccValues[2];
			uint16 valuea, valueb;
			txiqccsetfn_t txiqcc_fn = pi->pi_fptr.txiqccset;

			if (txiqcc_fn) {
				bcopy(p, iqccValues, 2*sizeof(int32));
				valuea = (uint16)(iqccValues[0]);
				valueb = (uint16)(iqccValues[1]);
				(*txiqcc_fn)(pi, valuea, valueb);
			}

		}
		break;

	case IOV_GVAL(IOV_PHY_TXLOCC):
		{
			uint16 di0dq0;
			uint8 *loccValues = a;
			txloccgetfn_t txlocc_fn = pi->pi_fptr.txloccget;
			radioloftgetfn_t radio_loft_fn = pi->pi_fptr.radioloftget;

			if ((txlocc_fn) && (radio_loft_fn))
			{
				/* copy the 6 bytes to a */
				di0dq0 = (*txlocc_fn)(pi);
				loccValues[0] = (uint8)(di0dq0 >> 8);
				loccValues[1] = (uint8)(di0dq0 & 0xff);
				(*radio_loft_fn)(pi, &loccValues[2], &loccValues[3],
					&loccValues[4], &loccValues[5]);
			}
		}
		break;

	case IOV_SVAL(IOV_PHY_TXLOCC):
		{
			/* copy 6 bytes from a to radio */
			uint16 di0dq0;
			uint8 *loccValues = p;
			di0dq0 = ((uint16)loccValues[0] << 8) | loccValues[1];
			if (ISLPPHY(pi)) {
				wlc_phy_set_tx_locc_lpphy(pi, di0dq0);
				wlc_phy_set_radio_loft_lpphy(pi, loccValues[2],
					loccValues[3], loccValues[4], loccValues[5]);
				wlc_phy_set_tx_locc_ucode_lpphy(pi, FALSE, di0dq0);
			} else if (pi->pi_fptr.txloccset && pi->pi_fptr.radioloftset) {
				pi->pi_fptr.txloccset(pi, di0dq0);
				pi->pi_fptr.radioloftset(pi, loccValues[2],
					loccValues[3], loccValues[4], loccValues[5]);
			} else
				return BCME_UNSUPPORTED;

		}
		break;

	case IOV_GVAL(IOV_PHYHAL_MSG):
		*ret_int_ptr = (int32)phyhal_msg_level;
		break;

	case IOV_SVAL(IOV_PHYHAL_MSG):
		phyhal_msg_level = (uint32)int_val;
		break;

#endif 
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV) || \
	defined(MACOSX)
#if 0//delete by ll
	case IOV_GVAL(IOV_PHY_WATCHDOG):
		*ret_int_ptr = (int32)pi->phywatchdog_override;
		break;

	case IOV_SVAL(IOV_PHY_WATCHDOG):
		pi->phywatchdog_override = bool_val;
		break;
#endif
#endif
#if defined(WLTEST)
	case IOV_SVAL(IOV_PHY_FIXED_NOISE):
		pi->phy_fixed_noise = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_FIXED_NOISE):
		int_val = (int32)pi->phy_fixed_noise;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_PHYNOISE_POLL):
		*ret_int_ptr = (int32)pi->phynoise_polling;
		break;

	case IOV_SVAL(IOV_PHYNOISE_POLL):
		pi->phynoise_polling = bool_val;
		break;
#endif 

#if defined(WLTEST) || defined(WLMEDIA_CALDBG)
	case IOV_GVAL(IOV_PHY_CAL_DISABLE):
		*ret_int_ptr = (int32)pi->disable_percal;
		break;

	case IOV_SVAL(IOV_PHY_CAL_DISABLE):
		pi->disable_percal = bool_val;
		break;
#endif  

#if defined(WLTEST)
	case IOV_GVAL(IOV_CARRIER_SUPPRESS):
		if (!(ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNPHY(pi)))
			return BCME_UNSUPPORTED;	/* lpphy, sslpnphy and lcnphy for now */
		*ret_int_ptr = (pi->carrier_suppr_disable == 0);
		break;

	case IOV_SVAL(IOV_CARRIER_SUPPRESS):
	{
		initfn_t carr_suppr_fn = pi->pi_fptr.carrsuppr;

		if (carr_suppr_fn) {
			pi->carrier_suppr_disable = bool_val;
			if (pi->carrier_suppr_disable) {
				(*carr_suppr_fn)(pi);
			}
			PHY_INFORM(("Carrier Suppress Called\n"));
		}
		else
			return BCME_UNSUPPORTED;

		break;
	}

	case IOV_GVAL(IOV_UNMOD_RSSI):
	{
		int32 input_power_db = 0;
		rxsigpwrfn_t rx_sig_pwr_fn = pi->pi_fptr.rxsigpwr;

		PHY_INFORM(("UNMOD RSSI Called\n"));

		if (!rx_sig_pwr_fn)
			return BCME_UNSUPPORTED;	/* lpphy and sslnphy support only for now */

		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}

		input_power_db = (*rx_sig_pwr_fn)(pi, -1);

#if defined(WLNOKIA_NVMEM)
		input_power_db = wlc_phy_upd_rssi_offset(pi,
			(int8)input_power_db, pi->radio_chanspec);
#endif 

		*ret_int_ptr = input_power_db;
		break;
	}

	case IOV_GVAL(IOV_PKTENG_STATS):
		wlc_phy_pkteng_stats_get(pi, a, alen);
		break;

	case IOV_GVAL(IOV_PHYTABLE):
		wlc_phy_table_get(pi, int_val, p, a);
		break;

	case IOV_SVAL(IOV_PHYTABLE):
		wlc_phy_table_set(pi, int_val, a);
		break;

	case IOV_SVAL(IOV_ACI_EXIT_CHECK_PERIOD):
		if (int_val == 0)
			err = BCME_RANGE;
		else
			pi->aci_exit_check_period = int_val;
		break;

	case IOV_GVAL(IOV_ACI_EXIT_CHECK_PERIOD):
		int_val = pi->aci_exit_check_period;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_PAVARS): {
		uint16 *outpa = a;
		uint16 inpa[WL_PHY_PAVARS_LEN];
		uint j = 3;	/* PA parameters start from offset 3 */

		bcopy(p, inpa, sizeof(inpa));

		outpa[0] = inpa[0]; /* Phy type */
		outpa[1] = inpa[1]; /* Band range */
		outpa[2] = inpa[2]; /* Chain */

		if (ISHTPHY(pi)) {
			if (inpa[0] != PHY_TYPE_HT) {
				outpa[0] = PHY_TYPE_NULL;
				break;
			}

			if (inpa[2] >= pi->pubpi.phy_corenum)
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
			case WL_CHAN_FREQ_RANGE_5GL:
			case WL_CHAN_FREQ_RANGE_5GM:
			case WL_CHAN_FREQ_RANGE_5GH:
				wlc_phy_pavars_get_htphy(pi, &outpa[j], inpa[1], inpa[2]);
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			}
		} else if (ISNPHY(pi)) {
			phy_pwrctrl_t	*pwrctl;

			if (inpa[0] != PHY_TYPE_N) {
				outpa[0] = PHY_TYPE_NULL;
				break;
			}

			if (inpa[2] == 0)
				pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_0];
			else if (inpa[2] == 1)
				pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_1];
			else
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
				outpa[j++] = pwrctl->pwrdet_2g_a1;	/* a1 */
				outpa[j++] = pwrctl->pwrdet_2g_b0;	/* b0 */
				outpa[j++] = pwrctl->pwrdet_2g_b1;	/* b1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				outpa[j++] = pwrctl->pwrdet_5gl_a1;	/* a1 */
				outpa[j++] = pwrctl->pwrdet_5gl_b0;	/* b0 */
				outpa[j++] = pwrctl->pwrdet_5gl_b1;	/* b1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				outpa[j++] = pwrctl->pwrdet_5gm_a1;	/* a1 */
				outpa[j++] = pwrctl->pwrdet_5gm_b0;	/* b0 */
				outpa[j++] = pwrctl->pwrdet_5gm_b1;	/* b1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				outpa[j++] = pwrctl->pwrdet_5gh_a1;	/* a1 */
				outpa[j++] = pwrctl->pwrdet_5gh_b0;	/* b0 */
				outpa[j++] = pwrctl->pwrdet_5gh_b1;	/* b1 */
				break;

			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[0]));
				break;
			}
		} else if (ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi)) {
			if ((inpa[0] != PHY_TYPE_LP) && (inpa[0] != PHY_TYPE_SSN) &&
				(inpa[0] != PHY_TYPE_LCN) && (inpa[0] != PHY_TYPE_LCN40)) {
				outpa[0] = PHY_TYPE_NULL;
				break;
			}

			if (inpa[2] != 0)
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
				outpa[j++] = pi->txpa_2g[0];		/* b0 */
				outpa[j++] = pi->txpa_2g[1];		/* b1 */
				outpa[j++] = pi->txpa_2g[2];		/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				outpa[j++] = pi->txpa_5g_low[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_low[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_low[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				outpa[j++] = pi->txpa_5g_mid[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_mid[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_mid[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				outpa[j++] = pi->txpa_5g_hi[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_hi[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_hi[2];	/* a1 */
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[0]));
				break;
			}
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_SVAL(IOV_PAVARS): {
		uint16 inpa[WL_PHY_PAVARS_LEN];
		uint j = 3;	/* PA parameters start from offset 3 */

		bcopy(p, inpa, sizeof(inpa));
		if (ISHTPHY(pi)) {
			if (inpa[0] != PHY_TYPE_HT) {
				break;
			}

			if (inpa[2] >= pi->pubpi.phy_corenum)
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
			case WL_CHAN_FREQ_RANGE_5GL:
			case WL_CHAN_FREQ_RANGE_5GM:
			case WL_CHAN_FREQ_RANGE_5GH:
				wlc_phy_pavars_set_htphy(pi, &inpa[j], inpa[1], inpa[2]);
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
				break;
			}
		} else if (ISNPHY(pi)) {
			phy_pwrctrl_t	*pwrctl;

			if (inpa[0] != PHY_TYPE_N)
				break;

			if (inpa[2] == 0)
				pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_0];
			else if (inpa[2] == 1)
				pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_1];
			else
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
				pwrctl->pwrdet_2g_a1 = inpa[j++];	/* a1 */
				pwrctl->pwrdet_2g_b0 = inpa[j++];	/* b0 */
				pwrctl->pwrdet_2g_b1 = inpa[j++];	/* b1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				pwrctl->pwrdet_5gl_a1 = inpa[j++];	/* a1 */
				pwrctl->pwrdet_5gl_b0 = inpa[j++];	/* b0 */
				pwrctl->pwrdet_5gl_b1 = inpa[j++];	/* b1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				pwrctl->pwrdet_5gm_a1 = inpa[j++];	/* a1 */
				pwrctl->pwrdet_5gm_b0 = inpa[j++];	/* b0 */
				pwrctl->pwrdet_5gm_b1 = inpa[j++];	/* b1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				pwrctl->pwrdet_5gh_a1 = inpa[j++];	/* a1 */
				pwrctl->pwrdet_5gh_b0 = inpa[j++];	/* b0 */
				pwrctl->pwrdet_5gh_b1 = inpa[j++];	/* b1 */
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[0]));
				break;
			}
		} else if (ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi)) {
			if ((inpa[0] != PHY_TYPE_LP) && (inpa[0] != PHY_TYPE_SSN) &&
				(inpa[0] != PHY_TYPE_LCN) && (inpa[0] != PHY_TYPE_LCN40))
				break;

			if (inpa[2] != 0)
				return BCME_BADARG;

			switch (inpa[1]) {
			case WL_CHAN_FREQ_RANGE_2G:
				pi->txpa_2g[0] = inpa[j++];	/* b0 */
				pi->txpa_2g[1] = inpa[j++];	/* b1 */
				pi->txpa_2g[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				pi->txpa_5g_low[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_low[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_low[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				pi->txpa_5g_mid[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_mid[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_mid[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				pi->txpa_5g_hi[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_hi[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_hi[2] = inpa[j++];	/* a1 */
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpa[0]));
				break;
			}
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_GVAL(IOV_PAVARS2): {
		wl_pavars2_t *invar = (wl_pavars2_t *)p;
		wl_pavars2_t *outvar = (wl_pavars2_t *)a;
		uint16 *outpa = outvar->inpa;
		uint j = 0;	/* PA parameters start from offset */

		if (invar->ver  != WL_PHY_PAVAR_VER) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				invar->ver, WL_PHY_PAVAR_VER));
			return BCME_BADARG;
		}

		outvar->ver = WL_PHY_PAVAR_VER;
		outvar->len = sizeof(wl_pavars2_t);
		if (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec) == invar->bandrange)
			outvar->inuse = 1;
		else
			outvar->inuse = 0;

		if (pi->sh->subband5Gver == PHY_SUBBAND_5BAND) {
			if ((invar->bandrange == WL_CHAN_FREQ_RANGE_5GL) ||
				(invar->bandrange == WL_CHAN_FREQ_RANGE_5GM) ||
				(invar->bandrange == WL_CHAN_FREQ_RANGE_5GH)) {
				outvar->phy_type = PHY_TYPE_NULL;
				break;
			}
		}

		if (pi->sh->subband5Gver == PHY_SUBBAND_3BAND_JAPAN) {
			if ((invar->bandrange == WL_CHAN_FREQ_RANGE_5GLL_5BAND) ||
				(invar->bandrange == WL_CHAN_FREQ_RANGE_5GLH_5BAND) ||
				(invar->bandrange == WL_CHAN_FREQ_RANGE_5GML_5BAND) ||
				(invar->bandrange == WL_CHAN_FREQ_RANGE_5GMH_5BAND) ||
				(invar->bandrange == WL_CHAN_FREQ_RANGE_5GH_5BAND)) {
				outvar->phy_type = PHY_TYPE_NULL;
				break;
			}
		}

		if (ISHTPHY(pi)) {
			if (invar->phy_type != PHY_TYPE_HT) {
				outvar->phy_type = PHY_TYPE_NULL;
				break;
			}

			if (invar->chain >= pi->pubpi.phy_corenum)
				return BCME_BADARG;

			switch (invar->bandrange) {
			case WL_CHAN_FREQ_RANGE_2G:
			case WL_CHAN_FREQ_RANGE_5GL:
			case WL_CHAN_FREQ_RANGE_5GM:
			case WL_CHAN_FREQ_RANGE_5GH:
			case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
			case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
			case WL_CHAN_FREQ_RANGE_5GML_5BAND:
			case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
			case WL_CHAN_FREQ_RANGE_5GH_5BAND:
				wlc_phy_pavars_get_htphy(pi, &outpa[j], invar->bandrange,
					invar->chain);
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", invar->bandrange));
				break;
			}
		} else if (ISNPHY(pi)) {
			phy_pwrctrl_t	*pwrctl;

			if (invar->phy_type != PHY_TYPE_N) {
				outvar->phy_type = PHY_TYPE_NULL;
				break;
			}
			/* PA parameters depend on the SROM revision */
			if (NREV_GE(pi->pubpi.phy_rev, 8) && (pi->sh->sromrev >= 9)) {
				err = wlc_phy_get_pa_phy(pi, invar, &j, outpa);
				if (err == BCME_ERROR)
					return BCME_BADARG;
			} else {
				if (invar->chain == 0)
					pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_0];
				else if (invar->chain == 1)
					pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_1];
				else
					return BCME_BADARG;

				switch (invar->bandrange) {
				case WL_CHAN_FREQ_RANGE_2G:
					outpa[j++] = pwrctl->pwrdet_2g_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_2g_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_2g_b1;	/* b1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GL:
					outpa[j++] = pwrctl->pwrdet_5gl_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5gl_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5gl_b1;	/* b1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GM:
					outpa[j++] = pwrctl->pwrdet_5gm_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5gm_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5gm_b1;	/* b1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GH:
					outpa[j++] = pwrctl->pwrdet_5gh_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5gh_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5gh_b1;	/* b1 */
					break;
#ifdef	WL_PPR_SUBBAND
				case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
					outpa[j++] = pwrctl->pwrdet_5gll_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5gll_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5gll_b1;	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
					outpa[j++] = pwrctl->pwrdet_5glh_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5glh_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5glh_b1;	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GML_5BAND:
					outpa[j++] = pwrctl->pwrdet_5gml_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5gml_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5gml_b1;	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
					outpa[j++] = pwrctl->pwrdet_5gmh_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5gmh_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5gmh_b1;	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GH_5BAND:
					outpa[j++] = pwrctl->pwrdet_5gh_a1;	/* a1 */
					outpa[j++] = pwrctl->pwrdet_5gh_b0;	/* b0 */
					outpa[j++] = pwrctl->pwrdet_5gh_b1;	/* b1 */
					break;
#endif	/* WL_PPR_SUBBAN	D */

				default:
					PHY_ERROR(("bandrange %d is out of scope\n",
					invar->bandrange));
					break;
				}
			}
		} else if (ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi)) {
			if ((invar->phy_type != PHY_TYPE_LP) && (invar->phy_type != PHY_TYPE_SSN) &&
				(invar->phy_type != PHY_TYPE_LCN) &&
				(invar->phy_type != PHY_TYPE_LCN40)) {
				outvar->phy_type = PHY_TYPE_NULL;
				break;
			}

			if (invar->chain != 0)
				return BCME_BADARG;

			switch (invar->bandrange) {
			case WL_CHAN_FREQ_RANGE_2G:
				outpa[j++] = pi->txpa_2g[0];		/* b0 */
				outpa[j++] = pi->txpa_2g[1];		/* b1 */
				outpa[j++] = pi->txpa_2g[2];		/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				outpa[j++] = pi->txpa_5g_low[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_low[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_low[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				outpa[j++] = pi->txpa_5g_mid[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_mid[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_mid[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				outpa[j++] = pi->txpa_5g_hi[0];	/* b0 */
				outpa[j++] = pi->txpa_5g_hi[1];	/* b1 */
				outpa[j++] = pi->txpa_5g_hi[2];	/* a1 */
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", invar->bandrange));
				break;
			}
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_SVAL(IOV_PAVARS2): {
		wl_pavars2_t *invar = (wl_pavars2_t *)p;
		uint16 *inpa = invar->inpa;
		uint j = 0;	/* PA parameters start from offset */

		if (invar->ver  != WL_PHY_PAVAR_VER) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				invar->ver, WL_PHY_PAVAR_VER));
			return BCME_BADARG;
		}

		if (ISHTPHY(pi)) {
			if (invar->phy_type != PHY_TYPE_HT) {
				break;
			}

			if (invar->chain >= pi->pubpi.phy_corenum)
				return BCME_BADARG;

			switch (invar->bandrange) {
			case WL_CHAN_FREQ_RANGE_2G:
			case WL_CHAN_FREQ_RANGE_5GL:
			case WL_CHAN_FREQ_RANGE_5GM:
			case WL_CHAN_FREQ_RANGE_5GH:
			case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
			case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
			case WL_CHAN_FREQ_RANGE_5GML_5BAND:
			case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
			case WL_CHAN_FREQ_RANGE_5GH_5BAND:
				wlc_phy_pavars_set_htphy(pi, &inpa[j], invar->bandrange,
					invar->chain);
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", invar->bandrange));
				break;
			}
		} else if (ISNPHY(pi)) {
			phy_pwrctrl_t	*pwrctl;

			if (invar->phy_type != PHY_TYPE_N)
				break;

			/* PA parameters depend on the SROM revision */
			if (NREV_GE(pi->pubpi.phy_rev, 8) && (pi->sh->sromrev >= 9)) {
				err = wlc_phy_set_pa_phy(pi, invar, &j, inpa);
				if (err == BCME_ERROR)
					return BCME_BADARG;

			} else {
				if (invar->chain == 0)
					pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_0];
				else if (invar->chain == 1)
					pwrctl = &pi_nphy->nphy_pwrctrl_info[PHY_CORE_1];
				else
					return BCME_BADARG;

				switch (invar->bandrange) {
				case WL_CHAN_FREQ_RANGE_2G:
					pwrctl->pwrdet_2g_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_2g_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_2g_b1 = inpa[j++];	/* b1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GL:
					pwrctl->pwrdet_5gl_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5gl_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5gl_b1 = inpa[j++];	/* b1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GM:
					pwrctl->pwrdet_5gm_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5gm_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5gm_b1 = inpa[j++];	/* b1 */
					break;

				case WL_CHAN_FREQ_RANGE_5GH:
					pwrctl->pwrdet_5gh_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5gh_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5gh_b1 = inpa[j++];	/* b1 */
					break;
#ifdef	WL_PPR_SUBBAND
				case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
					pwrctl->pwrdet_5gll_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5gll_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5gll_b1 = inpa[j++];	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
					pwrctl->pwrdet_5glh_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5glh_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5glh_b1 = inpa[j++];	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GML_5BAND:
					pwrctl->pwrdet_5gml_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5gml_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5gml_b1 = inpa[j++];	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
					pwrctl->pwrdet_5gmh_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5gmh_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5gmh_b1 = inpa[j++];	/* b1 */
					break;
				case WL_CHAN_FREQ_RANGE_5GH_5BAND:
					pwrctl->pwrdet_5gh_a1 = inpa[j++];	/* a1 */
					pwrctl->pwrdet_5gh_b0 = inpa[j++];	/* b0 */
					pwrctl->pwrdet_5gh_b1 = inpa[j++];	/* b1 */
					break;
#endif	/* WL_PPR_SUBBAN	D */
				default:
					PHY_ERROR(("bandrange %d is out of scope\n",
					invar->bandrange));
					break;
				}
			}
		} else if (ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNCOMMONPHY(pi)) {
			if ((invar->phy_type != PHY_TYPE_LP) && (invar->phy_type != PHY_TYPE_SSN) &&
				(invar->phy_type != PHY_TYPE_LCN) &&
				(invar->phy_type != PHY_TYPE_LCN40))
				break;

			if (invar->chain != 0)
				return BCME_BADARG;

			switch (invar->bandrange) {
			case WL_CHAN_FREQ_RANGE_2G:
				pi->txpa_2g[0] = inpa[j++];	/* b0 */
				pi->txpa_2g[1] = inpa[j++];	/* b1 */
				pi->txpa_2g[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				pi->txpa_5g_low[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_low[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_low[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				pi->txpa_5g_mid[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_mid[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_mid[2] = inpa[j++];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
				pi->txpa_5g_hi[0] = inpa[j++];	/* b0 */
				pi->txpa_5g_hi[1] = inpa[j++];	/* b1 */
				pi->txpa_5g_hi[2] = inpa[j++];	/* a1 */
				break;
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", invar->bandrange));
				break;
			}
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_GVAL(IOV_PHY_SUBBAND5GVER):
		/* Retrieve 5G subband version */
		int_val = (uint8)(pi->sh->subband5Gver);
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_POVARS): {
		wl_po_t tmppo;

		/* tmppo has the input phy_type and band */
		bcopy(p, &tmppo, sizeof(wl_po_t));
		if (ISHTPHY(pi)) {
			if ((tmppo.phy_type != PHY_TYPE_HT) && (tmppo.phy_type != PHY_TYPE_N))  {
				tmppo.phy_type = PHY_TYPE_NULL;
				break;
			}

			err = wlc_phy_get_po_htphy(pi, &tmppo);
			if (!err)
				bcopy(&tmppo, a, sizeof(wl_po_t));
			break;
		} else if (ISNPHY(pi)) {
			if (tmppo.phy_type != PHY_TYPE_N)  {
				tmppo.phy_type = PHY_TYPE_NULL;
				break;
			}

			/* Power offsets variables depend on the SROM revision */
			if (NREV_GE(pi->pubpi.phy_rev, 8) && (pi->sh->sromrev >= 9)) {
				err = wlc_phy_get_po_htphy(pi, &tmppo);

			} else {
				switch (tmppo.band) {
				case WL_CHAN_FREQ_RANGE_2G:
					tmppo.cckpo = pi->ppr.sr8.cck2gpo;
					tmppo.ofdmpo = pi->ppr.sr8.ofdm2gpo;
					bcopy(&pi->ppr.sr8.mcs2gpo, &tmppo.mcspo,
					      8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5GL:
					tmppo.ofdmpo = pi->ppr.sr8.ofdm5glpo;
					bcopy(&pi->ppr.sr8.mcs5glpo, &tmppo.mcspo,
					      8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5GM:
#ifdef	WL_PPR_SUBBAND
				case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
				case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
				case WL_CHAN_FREQ_RANGE_5GML_5BAND:
				case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
#endif	/* WL_PPR_SUBBAND */
					tmppo.ofdmpo = pi->ppr.sr8.ofdm5gpo;
					bcopy(&pi->ppr.sr8.mcs5gpo, &tmppo.mcspo,
					      8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5GH:
#ifdef	WL_PPR_SUBBAND
				case WL_CHAN_FREQ_RANGE_5GH_5BAND:
#endif	/* WL_PPR_SUBBAND */
					tmppo.ofdmpo = pi->ppr.sr8.ofdm5ghpo;
					bcopy(&pi->ppr.sr8.mcs5ghpo, &tmppo.mcspo,
					      8*sizeof(uint16));
					break;

				default:
					PHY_ERROR(("bandrange %d is out of scope\n", tmppo.band));
					err = BCME_BADARG;
					break;
				}
			}

			if (!err)
				bcopy(&tmppo, a, sizeof(wl_po_t));
		} else if (ISLCNPHY(pi)) {
			if (tmppo.phy_type != PHY_TYPE_LCN) {
				tmppo.phy_type = PHY_TYPE_NULL;
				break;
			}

			switch (tmppo.band) {
			case WL_CHAN_FREQ_RANGE_2G:
				tmppo.cckpo = pi->ppr.sr8.cck2gpo;
				tmppo.ofdmpo = pi->ppr.sr8.ofdm2gpo;
				bcopy(&pi->ppr.sr8.mcs2gpo, &tmppo.mcspo, 8*sizeof(uint16));

				break;

			default:
				PHY_ERROR(("bandrange %d is out of scope\n", tmppo.band));
				err = BCME_BADARG;
				break;
			}

			if (!err)
				bcopy(&tmppo, a, sizeof(wl_po_t));
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_SVAL(IOV_POVARS): {
		wl_po_t inpo;

		bcopy(p, &inpo, sizeof(wl_po_t));

		if (ISHTPHY(pi)) {
			if ((inpo.phy_type == PHY_TYPE_HT) || (inpo.phy_type == PHY_TYPE_N))
				err = wlc_phy_set_po_htphy(pi, &inpo);
			break;
		} else if (ISNPHY(pi)) {
			if (inpo.phy_type != PHY_TYPE_N)
				break;

			/* Power offsets variables depend on the SROM revision */
			if (NREV_GE(pi->pubpi.phy_rev, 8) && (pi->sh->sromrev >= 9)) {
				err = wlc_phy_set_po_htphy(pi, &inpo);

			} else {

				switch (inpo.band) {
				case WL_CHAN_FREQ_RANGE_2G:
					pi->ppr.sr8.cck2gpo = inpo.cckpo;
					pi->ppr.sr8.ofdm2gpo = inpo.ofdmpo;
					bcopy(inpo.mcspo, pi->ppr.sr8.mcs2gpo, 8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5GL:
					pi->ppr.sr8.ofdm5glpo = inpo.ofdmpo;
					bcopy(inpo.mcspo, pi->ppr.sr8.mcs5glpo, 8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5GM:
					pi->ppr.sr8.ofdm5gpo = inpo.ofdmpo;
					bcopy(inpo.mcspo, pi->ppr.sr8.mcs5gpo, 8*sizeof(uint16));
					break;

				case WL_CHAN_FREQ_RANGE_5GH:
					pi->ppr.sr8.ofdm5ghpo = inpo.ofdmpo;
					bcopy(inpo.mcspo, pi->ppr.sr8.mcs5ghpo, 8*sizeof(uint16));
					break;

				default:
					PHY_ERROR(("bandrange %d is out of scope\n", inpo.band));
					err = BCME_BADARG;
					break;
				}

				if (!err)
					wlc_phy_txpwr_apply_nphy(pi);
			}

		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_GVAL(IOV_PHY_IDLETSSI):
		wlc_phy_iovar_idletssi(pi, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_PHY_IDLETSSI):
		wlc_phy_iovar_idletssi(pi, ret_int_ptr, bool_val);
		break;

	case IOV_GVAL(IOV_PHY_VBATSENSE):
		wlc_phy_iovar_vbatsense(pi, ret_int_ptr);
		break;
#if 0//delete by ll
	case IOV_GVAL(IOV_PHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;
#endif 		
#endif 
	case IOV_GVAL(IOV_PHY_OCLSCDENABLE):
		wlc_phy_iovar_oclscd(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_OCLSCDENABLE):
		err = wlc_phy_iovar_oclscd(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;
#if defined(WLTEST) || defined(WLMEDIA_N2DBG) || defined(WLMEDIA_N2DEV)
#if 0//delete by ll
	case IOV_GVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;
#endif
#endif
#if 1//add by ll
	case IOV_GVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;
	case IOV_SVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_ACTIVECAL):
		*ret_int_ptr = (int32)((pi->cal_info->cal_phase_id !=
			MPHASE_CAL_STATE_IDLE)? 1 : 0);
		break;

	case IOV_GVAL(IOV_PHY_WATCHDOG):
		*ret_int_ptr = (int32)pi->phywatchdog_override;
		break;

	case IOV_SVAL(IOV_PHY_WATCHDOG):
		pi->phywatchdog_override = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;
#endif
#if defined(WLTEST)
	case IOV_SVAL(IOV_PHY_DEAF):
		wlc_phy_iovar_set_deaf(pi, int_val);
		break;

	case IOV_GVAL(IOV_PHY_TXRX_CHAIN):
		wlc_phy_iovar_txrx_chain(pi, int_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_TXRX_CHAIN):
		err = wlc_phy_iovar_txrx_chain(pi, int_val, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_PHY_BPHY_EVM):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_EVM, (bool) int_val);
		break;

	case IOV_GVAL(IOV_PHY_BPHY_RFCS):
		*ret_int_ptr = pi->phy_bphy_rfcs;
		break;

	case IOV_SVAL(IOV_PHY_BPHY_RFCS):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_RFCS, (bool) int_val);
		break;

	case IOV_GVAL(IOV_PHY_SCRAMINIT):
		*ret_int_ptr = pi->phy_scraminit;
		break;

	case IOV_SVAL(IOV_PHY_SCRAMINIT):
		wlc_phy_iovar_scraminit(pi, (uint8)int_val);
		break;

	case IOV_SVAL(IOV_PHY_RFSEQ):
		wlc_phy_iovar_force_rfseq(pi, (uint8)int_val);
		break;

	case IOV_GVAL(IOV_PHY_TX_TONE):
	case IOV_GVAL(IOV_PHY_TX_TONE_HZ):
		*ret_int_ptr = pi->phy_tx_tone_freq;
		break;

	case IOV_SVAL(IOV_PHY_TX_TONE):
		wlc_phy_iovar_tx_tone(pi, (int32)int_val);
		break;

	case IOV_SVAL(IOV_PHY_TX_TONE_HZ):
		wlc_phy_iovar_tx_tone_hz(pi, (int32)int_val);
		break;

	case IOV_GVAL(IOV_PHY_TEST_TSSI):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 0);
		break;

	case IOV_GVAL(IOV_PHY_TEST_TSSI_OFFS):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 12);
		break;

	case IOV_SVAL(IOV_PHY_5G_PWRGAIN):
		pi->phy_5g_pwrgain = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_5G_PWRGAIN):
		*ret_int_ptr = (int32)pi->phy_5g_pwrgain;
		break;

	case IOV_SVAL(IOV_PHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;
#if 0//delete by ll
	case IOV_GVAL(IOV_PHY_ACTIVECAL):
		*ret_int_ptr = (int32)((pi->cal_info->cal_phase_id !=
			MPHASE_CAL_STATE_IDLE)? 1 : 0);
		break;
#endif
	case IOV_SVAL(IOV_PHY_TXPWRINDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_iovar_txpwrindex_set(pi, p);
		break;

	case IOV_GVAL(IOV_PHY_TXPWRINDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		wlc_phy_iovar_txpwrindex_get(pi, int_val, bool_val, ret_int_ptr);
		break;
#endif 

#ifdef WLTEST
	case IOV_GVAL(IOV_PHY_FEM2G): {
		bcopy(&pi->srom_fem2g, a, sizeof(srom_fem_t));
		break;
	}

	case IOV_SVAL(IOV_PHY_FEM2G): {
		bcopy(p, &pi->srom_fem2g, sizeof(srom_fem_t));
		/* srom_fem2g.extpagain changed after attach time */
		wlc_phy_txpower_ipa_upd(pi);
		break;
	}

	case IOV_GVAL(IOV_PHY_FEM5G): {
		bcopy(&pi->srom_fem5g, a, sizeof(srom_fem_t));
		break;
	}

	case IOV_SVAL(IOV_PHY_FEM5G): {
		bcopy(p, &pi->srom_fem5g, sizeof(srom_fem_t));
		/* srom_fem5g.extpagain changed after attach time */
		wlc_phy_txpower_ipa_upd(pi);
		break;
	}

	case IOV_GVAL(IOV_PHY_MAXP): {
		if (ISNPHY(pi)) {
			uint8*	maxp = (uint8*)a;

			maxp[0] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_2g;
			maxp[1] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_2g;
			maxp[2] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_5gm;
			maxp[3] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_5gm;
			maxp[4] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_5gl;
			maxp[5] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_5gl;
			maxp[6] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_5gh;
			maxp[7] = pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_5gh;
		}
		break;
	}
	case IOV_SVAL(IOV_PHY_MAXP): {
		if (ISNPHY(pi)) {
			uint8*	maxp = (uint8*)p;

			pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_2g = maxp[0];
			pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_2g = maxp[1];
			pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_5gm = maxp[2];
			pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_5gm = maxp[3];
			pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_5gl = maxp[4];
			pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_5gl = maxp[5];
			pi_nphy->nphy_pwrctrl_info[PHY_CORE_0].max_pwr_5gh = maxp[6];
			pi_nphy->nphy_pwrctrl_info[PHY_CORE_1].max_pwr_5gh = maxp[7];
		}
		break;
	}
#endif /* WLTEST */

	case IOV_GVAL(IOV_PHY_RXIQ_EST):
	{
		bool suspend;

		if (ISSSLPNPHY(pi)) {
			return BCME_UNSUPPORTED;        /* lpphy support only for now */
		}

		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}

		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}

		wlc_btcx_override_enable(pi);

		wlc_phyreg_enter((wlc_phy_t *)pi);
		/* get IQ power measurements */
		*ret_int_ptr = wlc_phy_rx_iq_est(pi, pi->phy_rxiq_samps, pi->phy_rxiq_antsel,
		                                 pi->phy_rxiq_resln, pi->phy_rxiq_lpfhpc,
		                                 pi->phy_rxiq_diglpf,
		                                 pi->phy_rxiq_gain_correct,
		                                 pi->phy_rxiq_extra_gain_3dB, 0);
		wlc_phyreg_exit((wlc_phy_t *)pi);

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		break;
	}

	case IOV_SVAL(IOV_PHY_RXIQ_EST):
		if (ISSSLPNPHY(pi))
		{
			return BCME_UNSUPPORTED;        /* lpphy support only for now */
		}

		if (!ISLPPHY(pi)) {
			uint8 samples, antenna, resolution, lpf_hpc, dig_lpf;
			uint8 gain_correct, extra_gain_3dB;

			extra_gain_3dB = (int_val >> 28) & 0xf;
			gain_correct = (int_val >> 24) & 0xf;
			lpf_hpc = (int_val >> 20) & 0x3;
			dig_lpf = (int_val >> 22) & 0x3;
			resolution = (int_val >> 16) & 0xf;
			samples = (int_val >> 8) & 0xff;
			antenna = int_val & 0xff;

			if ((gain_correct != 0) && (gain_correct != 1)) {
				err = BCME_RANGE;
				break;
			}
			if ((lpf_hpc != 0) && (lpf_hpc != 1)) {
				err = BCME_RANGE;
				break;
			}
			if (dig_lpf > 2) {
				err = BCME_RANGE;
				break;
			}

			if ((resolution != 0) && (resolution != 1)) {
				err = BCME_RANGE;
				break;
			}

			if (samples < 10 || samples > 15) {
				err = BCME_RANGE;
				break;
			}

			if ((antenna != ANT_RX_DIV_FORCE_0) && (antenna != ANT_RX_DIV_FORCE_1) &&
			    (antenna != ANT_RX_DIV_DEF)) {
				err = BCME_RANGE;
				break;
			}
			pi->phy_rxiq_samps = samples;
			pi->phy_rxiq_antsel = antenna;
			pi->phy_rxiq_resln = resolution;
			pi->phy_rxiq_lpfhpc = lpf_hpc;
			pi->phy_rxiq_diglpf = dig_lpf;
			pi->phy_rxiq_gain_correct = gain_correct;
			pi->phy_rxiq_extra_gain_3dB = extra_gain_3dB;
		}
		break;

	case IOV_GVAL(IOV_PHYNOISE_SROM):
		if (ISHTPHY(pi) ||
		(ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3))) {

			int8 noiselvl[PHY_CORE_MAX];
			uint8 core;
			uint32 pkd_noise = 0;
			if (!pi->sh->up) {
				err = BCME_NOTUP;
				break;
			}
			wlc_phy_get_SROMnoiselvl_phy(pi, noiselvl);
			for (core = pi->pubpi.phy_corenum; core >= 1; core--) {
				pkd_noise = (pkd_noise << 8) | (uint8)(noiselvl[core-1]);
			}
			*ret_int_ptr = pkd_noise;
		} else if (ISLCNPHY(pi)) {
#if defined(WLTEST)
			int8 noiselvl;
			uint32 pkd_noise = 0;
			if (!pi->sh->up) {
				err = BCME_NOTUP;
				break;
			}
			wlc_phy_get_SROMnoiselvl_lcnphy(pi, &noiselvl);
			pkd_noise  = (uint8)noiselvl;
			*ret_int_ptr = pkd_noise;
#else
			return BCME_UNSUPPORTED;
#endif
		} else {
			return BCME_UNSUPPORTED;        /* only htphy supported for now */
		}
		break;

	case IOV_GVAL(IOV_NUM_STREAM):
		if (ISNPHY(pi)) {
			int_val = 2;
		} else if (ISHTPHY(pi)) {
			int_val = 3;
		} else if (ISAPHY(pi) || ISGPHY(pi) || ISLPPHY(pi) || ISSSLPNPHY(pi) ||
			ISLCNPHY(pi)) {
			int_val = 1;
		} else {
			int_val = -1;
		}
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_BAND_RANGE):
		int_val = wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec);
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_MIN_TXPOWER):
		pi->min_txpower = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_MIN_TXPOWER):
		int_val = pi->min_txpower;
		bcopy(&int_val, a, sizeof(int_val));
		break;
	case IOV_GVAL(IOV_PHY_MUTED):
		*ret_int_ptr = PHY_MUTED(pi) ? 1 : 0;
		break;
#ifdef PHYMON
	case IOV_GVAL(IOV_PHYCAL_STATE): {
		if (alen < (int)sizeof(wl_phycal_state_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (ISNPHY(pi))
			err = wlc_phycal_state_nphy(pi, a, alen);
		else
			err = BCME_UNSUPPORTED;

		break;
	}
#endif /* PHYMON */

#if defined(WLTEST) || defined(AP)
	case IOV_GVAL(IOV_PHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, TRUE);
		break;
#endif 

	case IOV_GVAL(IOV_PHY_PAPD_DEBUG):
		if (ISSSLPNPHY(pi))
		{
			wlc_sslpnphy_iovar_papd_debug(pi, a);
		}
		break;

	case IOV_GVAL(IOV_NOISE_MEASURE):
#if LCNCONF
		if (ISLCNPHY(pi))
		  wlc_lcnphy_noise_measure_start(pi, TRUE);
#endif
		int_val = 0;
		bcopy(&int_val, a, sizeof(int_val));
		break;
	case IOV_GVAL(IOV_NOISE_MEASURE_DISABLE):
#if LCNCONF
		if (ISLCNPHY(pi))
		  wlc_lcnphy_noise_measure_disable(pi, 0, (uint32*)&int_val);
		else
#endif
		   int_val = 0;
		bcopy(&int_val, a, sizeof(int_val));
		break;
	case IOV_SVAL(IOV_NOISE_MEASURE_DISABLE):
#if LCNCONF
		if (ISLCNPHY(pi))
		  wlc_lcnphy_noise_measure_disable(pi, int_val, NULL);
#endif
		break;

#ifdef WLMEDIA_TXFILTER_OVERRIDE
	case IOV_GVAL(IOV_PHY_TXFILTER_SM_OVERRIDE):
		int_val = pi->sh->txfilter_sm_override;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_PHY_TXFILTER_SM_OVERRIDE):
		if (int_val < WLC_TXFILTER_OVERRIDE_DISABLED ||
			int_val > WLC_TXFILTER_OVERRIDE_ENABLED) {
			err = BCME_RANGE;
		} else {
			pi->sh->txfilter_sm_override = int_val;
		}

		break;
#endif /* WLMEDIA_TXFILTER_OVERRIDE */

#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG)
	case IOV_GVAL(IOV_PHY_RXDESENS):
		if (ISNPHY(pi))
			err = wlc_nphy_get_rxdesens((wlc_phy_t *)pi, ret_int_ptr);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_PHY_RXDESENS):
		if (ISNPHY(pi))
			err = wlc_nphy_set_rxdesens((wlc_phy_t *)pi, int_val);
		else
			err = BCME_UNSUPPORTED;
		break;
	case IOV_GVAL(IOV_NTD_GDS_LOWTXPWR):
		if (ISNPHY(pi))
			err = wlc_nphy_get_lowtxpwr((wlc_phy_t *)pi, ret_int_ptr);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_NTD_GDS_LOWTXPWR):
		if (ISNPHY(pi))
			err = wlc_nphy_set_lowtxpwr((wlc_phy_t *)pi, int_val);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PHYCAL_TEMPDELTA):
		*ret_int_ptr = (int32)pi->phycal_tempdelta;
		break;

	case IOV_SVAL(IOV_PHYCAL_TEMPDELTA):
		pi->phycal_tempdelta = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_PAPDCAL_INDEXDELTA):
		*ret_int_ptr = (int32)pi->papdcal_indexdelta;
		break;

	case IOV_SVAL(IOV_PAPDCAL_INDEXDELTA):
		pi->papdcal_indexdelta = (uint8)int_val;
		break;


#endif /* defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG) */

#if defined(BCMDBG)
	case IOV_GVAL(IOV_LCNPHY_CWTXPWRCTRL):
		if (ISLCNPHY(pi))
			wlc_lcnphy_iovar_cw_tx_pwr_ctrl(pi, int_val, ret_int_ptr, FALSE);
		break;
	case IOV_SVAL(IOV_LCNPHY_CWTXPWRCTRL):
		if (ISLCNPHY(pi))
			wlc_lcnphy_iovar_cw_tx_pwr_ctrl(pi, int_val, ret_int_ptr, TRUE);
		break;
#endif
	case IOV_GVAL(IOV_PHY_RXANTSEL):
		if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 7))
			*ret_int_ptr = pi->nphy_enable_hw_antsel ? 1 : 0;
		break;
	case IOV_SVAL(IOV_PHY_RXANTSEL):
		if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 7)) {
			pi->nphy_enable_hw_antsel = bool_val;
			/* make sure driver is up (so clks are on) before writing to PHY regs */
			if (pi->sh->up) {
				wlc_phy_init_hw_antsel(pi);
			}
		}
		break;

#ifdef ENABLE_FCBS
	case IOV_SVAL(IOV_PHY_FCBSINIT):
		if (ISHTPHY(pi)) {
			if ((int_val >= FCBS_CHAN_A) && (int_val <= FCBS_CHAN_B)) {
				wlc_phy_fcbs_init(pi, int_val);
			} else {
				err = BCME_RANGE;
			}
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_SVAL(IOV_PHY_FCBS):
		if (ISHTPHY(pi)) {
			if ((int_val >= FCBS_CHAN_A) && (int_val <= FCBS_CHAN_B)) {
				wlc_phy_fcbs(pi, int_val, TRUE);
			} else {
				err = BCME_RANGE;
			}
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_PHY_FCBS):
		if (ISHTPHY(pi)) {
			*ret_int_ptr = (int32) wlc_phy_fcbs(pi, int_val, FALSE);
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
#endif /* ENABLE_FCBS */
	case IOV_SVAL(IOV_PHY_CRS_WAR):
		if (ISLCN40PHY(pi)) {
			pi->u.pi_lcn40phy->phycrs_war_en = (bool)int_val;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_PHY_CRS_WAR):
		if (ISLCN40PHY(pi)) {
			*ret_int_ptr = (int32) pi->u.pi_lcn40phy->phycrs_war_en;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	default:
		if (BCME_UNSUPPORTED == wlc_phy_iovar_dispatch_old(pi, actionid, p, a, vsize,
			int_val, bool_val))
			err = BCME_UNSUPPORTED;
	}

	return err;
}

int
wlc_phy_iovar_dispatch_old(phy_info_t *pi, uint32 actionid, void *p, void *a, int vsize,
	int32 int_val, bool bool_val)
{
	int err = BCME_OK;
	int32 *ret_int_ptr;

	phy_info_lpphy_t *pi_lp;
	phy_info_nphy_t *pi_nphy;

	pi_lp = pi->u.pi_lpphy;
	pi_nphy = pi->u.pi_nphy;

	ret_int_ptr = (int32 *)a;

	switch (actionid) {
#if NCONF
#if defined(BCMDBG)
	case IOV_SVAL(IOV_NPHY_INITGAIN):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		wlc_phy_setinitgain_nphy(pi, (uint16) int_val);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_SVAL(IOV_NPHY_HPVGA1GAIN):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		wlc_phy_sethpf1gaintbl_nphy(pi, (int8) int_val);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_SVAL(IOV_NPHY_TX_TEMP_TONE): {
		uint16 orig_BBConfig;
		uint16 m0m1;
		nphy_txgains_t target_gain;

		if ((uint32)int_val > 0) {
			pi->phy_tx_tone_freq = (uint32) int_val;
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

			/* Save the bbmult values,since it gets overwritten by mimophy_tx_tone() */
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

			/* Disable the re-sampler (in case we are in spur avoidance mode) */
			orig_BBConfig = phy_reg_read(pi, NPHY_BBConfig);
			phy_reg_mod(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK, 0);

			/* read current tx gain and use as target_gain */
			target_gain = wlc_phy_get_tx_gain_nphy(pi);

			PHY_ERROR(("Tx gain core 0: target gain: ipa = %d,"
			         " pad = %d, pga = %d, txgm = %d, txlpf = %d\n",
			         target_gain.ipa[0], target_gain.pad[0], target_gain.pga[0],
			         target_gain.txgm[0], target_gain.txlpf[0]));

			PHY_ERROR(("Tx gain core 1: target gain: ipa = %d,"
			         " pad = %d, pga = %d, txgm = %d, txlpf = %d\n",
			         target_gain.ipa[0], target_gain.pad[1], target_gain.pga[1],
			         target_gain.txgm[1], target_gain.txlpf[1]));

			/* play a tone for 10 secs and then stop it and return */
			wlc_phy_tx_tone_nphy(pi, (uint32)int_val, 250, 0, 0, FALSE);

			/* Now restore the original bbmult values */
			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m0m1);

			OSL_DELAY(10000000);
			wlc_phy_stopplayback_nphy(pi);

			/* Restore the state of the re-sampler
			   (in case we are in spur avoidance mode)
			*/
			phy_reg_write(pi, NPHY_BBConfig, orig_BBConfig);

			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);
			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		break;
	}
	case IOV_SVAL(IOV_NPHY_CAL_RESET):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		wlc_phy_cal_reset_nphy(pi, (uint32) int_val);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_EST_TONEPWR):
	case IOV_GVAL(IOV_PHY_EST_TONEPWR): {
		int32 dBm_power[2];
		uint16 orig_BBConfig;
		uint16 m0m1;

		if (ISNPHY(pi)) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);

			/* Save the bbmult values, since it gets overwritten
			   by mimophy_tx_tone()
			*/
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

			/* Disable the re-sampler (in case we are in spur avoidance mode) */
			orig_BBConfig = phy_reg_read(pi, NPHY_BBConfig);
			phy_reg_mod(pi, NPHY_BBConfig, NPHY_BBConfig_resample_clk160_MASK, 0);
			pi->phy_tx_tone_freq = (uint32) 4000;

			/* play a tone for 10 secs */
			wlc_phy_tx_tone_nphy(pi, (uint32)4000, 250, 0, 0, FALSE);

			/* Now restore the original bbmult values */
			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m0m1);

			OSL_DELAY(10000000);
			wlc_phy_est_tonepwr_nphy(pi, dBm_power, 128);
			wlc_phy_stopplayback_nphy(pi);

			/* Restore the state of the re-sampler
			   (in case we are in spur avoidance mode)
			*/
			phy_reg_write(pi, NPHY_BBConfig, orig_BBConfig);

			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);

			int_val = dBm_power[0]/4;
			bcopy(&int_val, a, vsize);
			break;
		} else {
			err = BCME_UNSUPPORTED;
			break;
		}
	}

	case IOV_GVAL(IOV_NPHY_RFSEQ_TXGAIN): {
		uint16 rfseq_tx_gain[2];
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, rfseq_tx_gain);
		int_val = (((uint32) rfseq_tx_gain[1] << 16) | ((uint32) rfseq_tx_gain[0]));
		bcopy(&int_val, a, vsize);
		break;
	}

	case IOV_SVAL(IOV_PHY_SPURAVOID):
		if ((int_val != SPURAVOID_DISABLE) && (int_val != SPURAVOID_AUTO) &&
		    (int_val != SPURAVOID_FORCEON) && (int_val != SPURAVOID_FORCEON2)) {
			err = BCME_RANGE;
			break;
		}

		pi->phy_spuravoid = (int8)int_val;
		break;

	case IOV_GVAL(IOV_PHY_SPURAVOID):
		int_val = pi->phy_spuravoid;
		bcopy(&int_val, a, vsize);
		break;
#endif /* defined(BCMDBG) */

#if defined(WLTEST)
	case IOV_SVAL(IOV_NPHY_CCK_PWR_OFFSET):
		pi_nphy->nphy_cck_pwr_err_adjust = (int8)int_val;
		wlc_phy_txpower_recalc_target(pi);
		break;
	case IOV_GVAL(IOV_NPHY_CCK_PWR_OFFSET):
		int_val =  pi_nphy->nphy_cck_pwr_err_adjust;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_GVAL(IOV_NPHY_CAL_SANITY):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		*ret_int_ptr = (uint32)wlc_phy_cal_sanity_nphy(pi);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_BPHY_EVM):
	case IOV_GVAL(IOV_PHY_BPHY_EVM):

		*ret_int_ptr = pi->phy_bphy_evm;
		break;

	case IOV_SVAL(IOV_NPHY_BPHY_EVM):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_EVM, (bool) int_val);
		break;

	case IOV_GVAL(IOV_NPHY_BPHY_RFCS):
		*ret_int_ptr = pi->phy_bphy_rfcs;
		break;

	case IOV_SVAL(IOV_NPHY_BPHY_RFCS):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_RFCS, (bool) int_val);
		break;

	case IOV_GVAL(IOV_NPHY_SCRAMINIT):
		*ret_int_ptr = pi->phy_scraminit;
		break;

	case IOV_SVAL(IOV_NPHY_SCRAMINIT):
		wlc_phy_iovar_scraminit(pi, pi->phy_scraminit);
		break;

	case IOV_SVAL(IOV_NPHY_RFSEQ):
		wlc_phy_iovar_force_rfseq(pi, (uint8)int_val);
		break;

	case IOV_GVAL(IOV_NPHY_TXIQLOCAL): {
		nphy_txgains_t target_gain;
		uint8 tx_pwr_ctrl_state;
		if (ISNPHY(pi)) {

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);

			/* read current tx gain and use as target_gain */
			target_gain = wlc_phy_get_tx_gain_nphy(pi);
			tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
			wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

			/* want outer (0,1) ants so T/R works properly for CB2 2x3 switch, */
			if (pi->antsel_type == ANTSEL_2x3) {
				wlc_phy_antsel_init_nphy((wlc_phy_t *)pi, TRUE);
			}

			wlc_phy_cal_txiqlo_nphy(pi, target_gain, TRUE, FALSE);

			wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		*ret_int_ptr = 0;
		break;
	}
	case IOV_SVAL(IOV_NPHY_RXIQCAL): {
		nphy_txgains_t target_gain;
		uint8 tx_pwr_ctrl_state;


		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);

		/* read current tx gain and use as target_gain */
		target_gain = wlc_phy_get_tx_gain_nphy(pi);
		tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

		wlc_phy_cal_rxiq_nphy(pi, target_gain, 0, (bool)int_val, 0x3);

		wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
		int_val = 0;
		bcopy(&int_val, a, vsize);
		break;
	}
	case IOV_GVAL(IOV_NPHY_RXCALPARAMS):
		*ret_int_ptr = pi_nphy->nphy_rxcalparams;
		break;

	case IOV_SVAL(IOV_NPHY_RXCALPARAMS):
		pi_nphy->nphy_rxcalparams = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_NPHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_NPHY_RSSISEL):
		*ret_int_ptr = pi->nphy_rssisel;
		break;

	case IOV_SVAL(IOV_NPHY_RSSISEL):
		pi->nphy_rssisel = (uint8)int_val;

		if (!pi->sh->up)
			break;

		if (pi->nphy_rssisel < 0) {
			wlc_phyreg_enter((wlc_phy_t *)pi);
			wlc_phy_rssisel_nphy(pi, RADIO_MIMO_CORESEL_OFF, 0);
			wlc_phyreg_exit((wlc_phy_t *)pi);
		} else {
			int32 rssi_buf[4];
			wlc_phyreg_enter((wlc_phy_t *)pi);
			wlc_phy_poll_rssi_nphy(pi, (uint8)int_val, rssi_buf, 1);
			wlc_phyreg_exit((wlc_phy_t *)pi);
		}
		break;

	case IOV_GVAL(IOV_NPHY_RSSICAL): {
		/* if down, return the value, if up, run the cal */
		if (!pi->sh->up) {
			int_val = pi->nphy_rssical;
			bcopy(&int_val, a, vsize);
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		/* run rssi cal */
		wlc_phy_rssi_cal_nphy(pi);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
		int_val = pi->nphy_rssical;
		bcopy(&int_val, a, vsize);
		break;
	}

	case IOV_SVAL(IOV_NPHY_RSSICAL): {
		pi->nphy_rssical = bool_val;
		break;
	}

	case IOV_GVAL(IOV_NPHY_GPIOSEL):
	case IOV_GVAL(IOV_PHY_GPIOSEL):
		*ret_int_ptr = pi->phy_gpiosel;
		break;

	case IOV_SVAL(IOV_NPHY_GPIOSEL):
	case IOV_SVAL(IOV_PHY_GPIOSEL):
		pi->phy_gpiosel = (uint16) int_val;

		if (!pi->sh->up)
			break;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);
		if (ISNPHY(pi))
			wlc_phy_gpiosel_nphy(pi, (uint16)int_val);
		else if (ISHTPHY(pi))
			wlc_phy_gpiosel_htphy(pi, (uint16)int_val);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_TX_TONE):
		*ret_int_ptr = pi->phy_tx_tone_freq;
		break;

	case IOV_SVAL(IOV_NPHY_TX_TONE):
		wlc_phy_iovar_tx_tone(pi, (uint32)int_val);
		break;

	case IOV_SVAL(IOV_NPHY_GAIN_BOOST):
		pi->nphy_gain_boost = bool_val;
		break;

	case IOV_GVAL(IOV_NPHY_GAIN_BOOST):
		*ret_int_ptr = (int32)pi->nphy_gain_boost;
		break;

	case IOV_SVAL(IOV_NPHY_ELNA_GAIN_CONFIG):
		pi->nphy_elna_gain_config = (int_val != 0) ? TRUE : FALSE;
		break;

	case IOV_GVAL(IOV_NPHY_ELNA_GAIN_CONFIG):
		*ret_int_ptr = (int32)pi->nphy_elna_gain_config;
		break;

	case IOV_GVAL(IOV_NPHY_TEST_TSSI):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 0);
		break;

	case IOV_GVAL(IOV_NPHY_TEST_TSSI_OFFS):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 12);
		break;

	case IOV_SVAL(IOV_NPHY_5G_PWRGAIN):
		pi->phy_5g_pwrgain = bool_val;
		break;

	case IOV_GVAL(IOV_NPHY_5G_PWRGAIN):
		*ret_int_ptr = (int32)pi->phy_5g_pwrgain;
		break;

	case IOV_GVAL(IOV_NPHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_PERICAL):
		wlc_phy_iovar_perical_config(pi, int_val, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_NPHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;

	case IOV_GVAL(IOV_NPHY_ACI_SCAN):
		if (SCAN_INPROG_PHY(pi)) {
			PHY_ERROR(("Scan in Progress, can execute %s\n", __FUNCTION__));
			*ret_int_ptr = -1;
		} else {
			if (pi->cur_interference_mode == INTERFERE_NONE) {
				PHY_ERROR(("interference mode is off\n"));
				*ret_int_ptr = -1;
				break;
			}

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			*ret_int_ptr = wlc_phy_aci_scan_nphy(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		break;

	case IOV_SVAL(IOV_NPHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_NPHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_PAPDCALTYPE):
		if (ISNPHY(pi))
			pi_nphy->nphy_papd_cal_type = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PAPDCAL):
		if (ISNPHY(pi))
			pi_nphy->nphy_force_papd_cal = TRUE;
		int_val = 0;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_NPHY_SKIPPAPD):
		if ((int_val != 0) && (int_val != 1)) {
			err = BCME_RANGE;
			break;
		}
		if (ISNPHY(pi))
			pi_nphy->nphy_papd_skip = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PAPDCALINDEX):
		if (ISNPHY(pi)) {
			*ret_int_ptr = (pi_nphy->nphy_papd_cal_gain_index[0] << 8) |
				pi_nphy->nphy_papd_cal_gain_index[1];
		}
		break;

	case IOV_SVAL(IOV_NPHY_CALTXGAIN): {
		uint8 tx_pwr_ctrl_state;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);

		if (ISNPHY(pi)) {
			pi_nphy->nphy_cal_orig_pwr_idx[0] =
				(uint8) ((phy_reg_read(pi, NPHY_Core0TxPwrCtrlStatus) >> 8) & 0x7f);
			pi_nphy->nphy_cal_orig_pwr_idx[1] =
				(uint8) ((phy_reg_read(pi, NPHY_Core1TxPwrCtrlStatus) >> 8) & 0x7f);
		}

		tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

		wlc_phy_cal_txgainctrl_nphy(pi, int_val, TRUE);

		wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);

		break;
	}
#endif 
	case IOV_GVAL(IOV_NPHY_TEMPTHRESH):
	case IOV_GVAL(IOV_PHY_TEMPTHRESH):
		*ret_int_ptr = (int32) pi->txcore_temp.disable_temp;
		break;

	case IOV_SVAL(IOV_NPHY_TEMPTHRESH):
	case IOV_SVAL(IOV_PHY_TEMPTHRESH):
		pi->txcore_temp.disable_temp = MIN((uint8) int_val,
			pi->txcore_temp.disable_temp_max_cap);
		pi->txcore_temp.enable_temp =
		    pi->txcore_temp.disable_temp - pi->txcore_temp.hysteresis;
		break;
#if defined(WLTEST)
	case IOV_GVAL(IOV_NPHY_TEMPOFFSET):
	case IOV_GVAL(IOV_PHY_TEMPOFFSET):
		*ret_int_ptr = (int32) pi->phy_tempsense_offset;
		break;

	case IOV_SVAL(IOV_NPHY_TEMPOFFSET):
	case IOV_SVAL(IOV_PHY_TEMPOFFSET):
		pi->phy_tempsense_offset = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_VCOCAL):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phy_radio205x_vcocal_nphy(pi);
		wlapi_enable_mac(pi->sh->physhim);
		*ret_int_ptr = 0;
		break;

	case IOV_GVAL(IOV_NPHY_TBLDUMP_MINIDX):
		*ret_int_ptr = (int32)pi->nphy_tbldump_minidx;
		break;

	case IOV_SVAL(IOV_NPHY_TBLDUMP_MINIDX):
		pi->nphy_tbldump_minidx = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_TBLDUMP_MAXIDX):
		*ret_int_ptr = (int32)pi->nphy_tbldump_maxidx;
		break;

	case IOV_SVAL(IOV_NPHY_TBLDUMP_MAXIDX):
		pi->nphy_tbldump_maxidx = (int8) int_val;
		break;

	case IOV_SVAL(IOV_NPHY_PHYREG_SKIPDUMP):
		if (pi->nphy_phyreg_skipcnt < 127) {
			pi->nphy_phyreg_skipaddr[pi->nphy_phyreg_skipcnt++] = (uint) int_val;
		}
		break;

	case IOV_GVAL(IOV_NPHY_PHYREG_SKIPDUMP):
		*ret_int_ptr = (pi->nphy_phyreg_skipcnt > 0) ?
			(int32) pi->nphy_phyreg_skipaddr[pi->nphy_phyreg_skipcnt-1] : 0;
		break;

	case IOV_SVAL(IOV_NPHY_PHYREG_SKIPCNT):
		pi->nphy_phyreg_skipcnt = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PHYREG_SKIPCNT):
		*ret_int_ptr = (int32)pi->nphy_phyreg_skipcnt;
		break;
#endif 
#endif /* NCONF */

#if defined(WLTEST)

#if LPCONF
	case IOV_SVAL(IOV_LPPHY_TX_TONE):
		pi->phy_tx_tone_freq = int_val;
		if (pi->phy_tx_tone_freq == 0) {
			wlc_phy_stop_tx_tone_lpphy(pi);
			wlc_phy_clear_deaf_lpphy(pi, (bool)1);
			wlc_phyreg_exit((wlc_phy_t *)pi);
			wlapi_enable_mac(pi->sh->physhim);
			pi->phywatchdog_override = TRUE;
		} else {
			pi->phywatchdog_override = FALSE;
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);
			wlc_phy_set_deaf_lpphy(pi, (bool)1);
			wlc_phy_start_tx_tone_lpphy(pi, int_val,
				LPREV_GE(pi->pubpi.phy_rev, 2) ? 28: 100); /* play tone */
		}
		break;

	case IOV_SVAL(IOV_LPPHY_PAPDCALTYPE):
		pi_lp->lpphy_papd_cal_type = (int16) int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_PAPDCAL):
		pi_lp->lpphy_force_papd_cal = TRUE;
		int_val = 0;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_GVAL(IOV_LPPHY_TXIQLOCAL):
	case IOV_GVAL(IOV_LPPHY_RXIQCAL):
		pi->phy_forcecal = TRUE;

		*ret_int_ptr = 0;
		break;

	case IOV_GVAL(IOV_LPPHY_FULLCAL):	/* conver to SVAL with parameters like NPHY ? */
		wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_GVAL(IOV_LPPHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_LPPHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_LPPHY_PAPD_SLOW_CAL):
		*ret_int_ptr = (uint32)pi_lp->lpphy_papd_slow_cal;
		break;

	case IOV_SVAL(IOV_LPPHY_PAPD_SLOW_CAL): {
		pi_lp->lpphy_papd_slow_cal = bool_val;
		break;
	}

	case IOV_GVAL(IOV_LPPHY_PAPD_RECAL_MIN_INTERVAL):
		*ret_int_ptr = (int32)pi_lp->lpphy_papd_recal_min_interval;
		break;

	case IOV_SVAL(IOV_LPPHY_PAPD_RECAL_MIN_INTERVAL):
		pi_lp->lpphy_papd_recal_min_interval = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_PAPD_RECAL_MAX_INTERVAL):
		*ret_int_ptr = (int32)pi_lp->lpphy_papd_recal_max_interval;
		break;

	case IOV_SVAL(IOV_LPPHY_PAPD_RECAL_MAX_INTERVAL):
		pi_lp->lpphy_papd_recal_max_interval = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_PAPD_RECAL_GAIN_DELTA):
		*ret_int_ptr = (int32)pi_lp->lpphy_papd_recal_gain_delta;
		break;

	case IOV_SVAL(IOV_LPPHY_PAPD_RECAL_GAIN_DELTA):
		pi_lp->lpphy_papd_recal_gain_delta = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_PAPD_RECAL_ENABLE):
		*ret_int_ptr = (uint32)pi_lp->lpphy_papd_recal_enable;
		break;

	case IOV_SVAL(IOV_LPPHY_PAPD_RECAL_ENABLE):
		pi_lp->lpphy_papd_recal_enable = bool_val;
		break;

	case IOV_GVAL(IOV_LPPHY_PAPD_RECAL_COUNTER):
		*ret_int_ptr = (int32)pi_lp->lpphy_papd_recal_counter;
		break;

	case IOV_SVAL(IOV_LPPHY_PAPD_RECAL_COUNTER):
		pi_lp->lpphy_papd_recal_counter = (uint32)int_val;
		break;

	case IOV_SVAL(IOV_LPPHY_CCK_DIG_FILT_TYPE):
		pi_lp->lpphy_cck_dig_filt_type = (int16)int_val;
		if (LPREV_GE(pi->pubpi.phy_rev, 2))
			wlc_phy_tx_dig_filt_cck_setup_lpphy(pi, TRUE);
		break;

	case IOV_GVAL(IOV_LPPHY_CCK_DIG_FILT_TYPE):
		int_val = (uint32)pi_lp->lpphy_cck_dig_filt_type;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_OFDM_DIG_FILT_TYPE):
		pi_lp->lpphy_ofdm_dig_filt_type = (int16)int_val;
		if (LPREV_GE(pi->pubpi.phy_rev, 2))
			wlc_phy_tx_dig_filt_ofdm_setup_lpphy(pi, TRUE);
		break;

	case IOV_GVAL(IOV_LPPHY_OFDM_DIG_FILT_TYPE):
		int_val = (uint32)pi_lp->lpphy_ofdm_dig_filt_type;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_TXRF_SP_9_OVR):
		pi_lp->lpphy_txrf_sp_9_override = int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_TXRF_SP_9_OVR):
		int_val = pi_lp->lpphy_txrf_sp_9_override;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_OFDM_ANALOG_FILT_BW_OVERRIDE):
		pi->ofdm_analog_filt_bw_override = (int16)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_OFDM_ANALOG_FILT_BW_OVERRIDE):
		int_val = pi->ofdm_analog_filt_bw_override;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_CCK_ANALOG_FILT_BW_OVERRIDE):
		pi->cck_analog_filt_bw_override = (int16)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_CCK_ANALOG_FILT_BW_OVERRIDE):
		int_val = pi->cck_analog_filt_bw_override;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_OFDM_RCCAL_OVERRIDE):
		pi->ofdm_rccal_override = (int16)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_OFDM_RCCAL_OVERRIDE):
		int_val = pi->ofdm_rccal_override;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_CCK_RCCAL_OVERRIDE):
		pi->cck_rccal_override = (int16)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_CCK_RCCAL_OVERRIDE):
		int_val = pi->cck_rccal_override;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_GVAL(IOV_LPPHY_TXPWRINDEX):	/* depreciated by PHY_TXPWRINDEX */
		wlc_phy_iovar_txpwrindex_get(pi, int_val, bool_val, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_LPPHY_TXPWRINDEX):	/* depreciated by PHY_TXPWRINDEX */
		err = wlc_phy_iovar_txpwrindex_set(pi, p);
		break;

	case IOV_GVAL(IOV_LPPHY_CRS):
	        *ret_int_ptr = ((phy_reg_read(pi, LPPHY_crsgainCtrl) &
	                         LPPHY_crsgainCtrl_crseddisable_MASK) == 0);
		break;

	case IOV_SVAL(IOV_LPPHY_CRS):
		if (int_val)
			wlc_phy_clear_deaf_lpphy(pi, (bool)1);
		else
			wlc_phy_set_deaf_lpphy(pi, (bool)1);
		break;

	case IOV_SVAL(IOV_LPPHY_ACI_ON_THRESH):
	        pi_lp->lpphy_aci.on_thresh = (int)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_ON_THRESH):
		int_val = pi_lp->lpphy_aci.on_thresh;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_OFF_THRESH):
		pi_lp->lpphy_aci.off_thresh = (int)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_OFF_THRESH):
		int_val = pi_lp->lpphy_aci.off_thresh;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_ON_TIMEOUT):
		pi_lp->lpphy_aci.on_timeout = (int)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_ON_TIMEOUT):
		int_val = pi_lp->lpphy_aci.on_timeout;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_OFF_TIMEOUT):
		pi_lp->lpphy_aci.off_timeout = (int)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_OFF_TIMEOUT):
		int_val = pi_lp->lpphy_aci.off_timeout;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_GLITCH_TIMEOUT):
		pi_lp->lpphy_aci.glitch_timeout = (int)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_GLITCH_TIMEOUT):
		int_val = pi_lp->lpphy_aci.glitch_timeout;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_CHAN_SCAN_CNT):
		pi_lp->lpphy_aci.chan_scan_cnt = (int32)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_CHAN_SCAN_CNT):
		int_val = pi_lp->lpphy_aci.chan_scan_cnt;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_CHAN_SCAN_PWR_THRESH):
		pi_lp->lpphy_aci.chan_scan_pwr_thresh = (int32)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_CHAN_SCAN_PWR_THRESH):
		int_val = pi_lp->lpphy_aci.chan_scan_pwr_thresh;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_CHAN_SCAN_CNT_THRESH):
		pi_lp->lpphy_aci.chan_scan_cnt_thresh = (int32)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_CHAN_SCAN_CNT_THRESH):
		int_val = pi_lp->lpphy_aci.chan_scan_cnt_thresh;
		bcopy(&int_val, a, vsize);
		break;
	case IOV_SVAL(IOV_LPPHY_ACI_CHAN_SCAN_TIMEOUT):
		pi_lp->lpphy_aci.chan_scan_timeout = (int32)int_val;
		wlc_phy_aci_init_lpphy(pi, FALSE);
		break;
	case IOV_GVAL(IOV_LPPHY_ACI_CHAN_SCAN_TIMEOUT):
		int_val = pi_lp->lpphy_aci.chan_scan_timeout;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_LPPHY_NOISE_SAMPLES):
		pi_lp->lpphy_noise_samples = (uint16)int_val;
		break;
	case IOV_GVAL(IOV_LPPHY_NOISE_SAMPLES):
		int_val = pi_lp->lpphy_noise_samples;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_LPPHY_PAPDEPSTBL):
	{
		lpphytbl_info_t tab;
		uint32 papdepstbl[PHY_PAPD_EPS_TBL_SIZE_LPPHY];

		/* Preset PAPD eps table */
		tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LPPHY;
		tab.tbl_id = LPPHY_TBL_ID_PAPD_EPS;
		tab.tbl_offset = 0;
		tab.tbl_width = 32;
		tab.tbl_phywidth = 32;
		tab.tbl_ptr = &papdepstbl[0];

		/* read the table */
		wlc_phy_table_read_lpphy(pi, &tab);
		bcopy(&papdepstbl[0], a, PHY_PAPD_EPS_TBL_SIZE_LPPHY*sizeof(uint32));
	}
	break;

	case IOV_GVAL(IOV_LPPHY_IDLE_TSSI_UPDATE_DELTA_TEMP):
		int_val = (int)pi_lp->lpphy_idle_tssi_update_delta_temp;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_LPPHY_IDLE_TSSI_UPDATE_DELTA_TEMP):
		pi_lp->lpphy_idle_tssi_update_delta_temp = (int16)int_val;
		break;
#endif /* LPCONF */

#if SSLPNCONF
	case IOV_SVAL(IOV_SSLPNPHY_TX_TONE):
		pi->phy_tx_tone_freq = int_val;
		if (pi->phy_tx_tone_freq == 0) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);
			wlc_sslpnphy_stop_tx_tone(pi);
			wlc_phyreg_exit((wlc_phy_t *)pi);

			wlapi_bmac_macphyclk_set(pi->sh->physhim, ON);

			wlapi_enable_mac(pi->sh->physhim);
			pi->phywatchdog_override = TRUE;
		} else {
			pi->phywatchdog_override = FALSE;
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phyreg_enter((wlc_phy_t *)pi);
			wlc_sslpnphy_start_tx_tone(pi, int_val, 112, 0); /* play tone */
			wlc_sslpnphy_set_tx_pwr_by_index(pi, (int)60);
			wlc_phyreg_exit((wlc_phy_t *)pi);

			wlapi_bmac_macphyclk_set(pi->sh->physhim, OFF);

			wlapi_enable_mac(pi->sh->physhim);
		}
		break;

	case IOV_GVAL(IOV_SSLPNPHY_PAPDCAL):
		pi->u.pi_sslpnphy->
			sslpnphy_full_cal_channel[CHSPEC_IS5G(pi->radio_chanspec) ? 1 : 0] = 0;
		/* fall through */
	case IOV_GVAL(IOV_SSLPNPHY_TXIQLOCAL):
	case IOV_GVAL(IOV_SSLPNPHY_RXIQCAL):
		pi->phy_forcecal = TRUE;

		*ret_int_ptr = 0;
		break;

	case IOV_GVAL(IOV_SSLPNPHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_SSLPNPHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_SSLPNPHY_TXPWRINDEX):
		wlc_phy_iovar_txpwrindex_get(pi, int_val, bool_val, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_SSLPNPHY_TXPWRINDEX):
		err = wlc_phy_iovar_txpwrindex_set(pi, p);
		break;

	case IOV_GVAL(IOV_SSLPNPHY_CRS):
	        *ret_int_ptr = ((phy_reg_read(pi, SSLPNPHY_crsgainCtrl) &
	                         SSLPNPHY_crsgainCtrl_crseddisable_MASK) == 0);
		break;

	case IOV_SVAL(IOV_SSLPNPHY_CRS):
		if (int_val)
			wlc_sslpnphy_deaf_mode(pi, FALSE);
		else
			wlc_sslpnphy_deaf_mode(pi, TRUE);
		break;

	case IOV_GVAL(IOV_SSLPNPHY_CARRIER_SUPPRESS):
		*ret_int_ptr = (pi->carrier_suppr_disable == 0);
		break;

	case IOV_GVAL(IOV_SSLPNPHY_UNMOD_RSSI):
	{
		int32 input_power_db = 0;

		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}
#if SSLPNCONF
		input_power_db = wlc_sslpnphy_rx_signal_power(pi, -1);
#endif
#if defined(WLNOKIA_NVMEM)
		input_power_db = wlc_phy_upd_rssi_offset(pi,
			(int8)input_power_db, pi->radio_chanspec);
#endif 
		*ret_int_ptr = input_power_db;
		break;
	}

	case IOV_SVAL(IOV_SSLPNPHY_CARRIER_SUPPRESS):
	{
		pi->carrier_suppr_disable = bool_val;
		if (pi->carrier_suppr_disable) {
			wlc_phy_carrier_suppress_sslpnphy(pi);
		}
		break;
	}

	case IOV_SVAL(IOV_SSLPNPHY_NOISE_SAMPLES):
		pi->u.pi_sslpnphy->sslpnphy_noise_samples = (uint16)int_val;
		break;
	case IOV_GVAL(IOV_SSLPNPHY_NOISE_SAMPLES):
		int_val = pi->u.pi_sslpnphy->sslpnphy_noise_samples;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_SSLPNPHY_PAPARAMS):
		{
			int32 paparams[3];
			uint freq;
			freq = wlc_phy_channel2freq(CHSPEC_CHANNEL(pi->radio_chanspec));
			switch (wlc_phy_chanspec_freq2bandrange_lpssn(freq)) {
			case WL_CHAN_FREQ_RANGE_2G:
				/* 2.4 GHz */
				paparams[0] = (int32)pi->txpa_2g[0];		/* b0 */
				paparams[1] = (int32)pi->txpa_2g[1];		/* b1 */
				paparams[2] = (int32)pi->txpa_2g[2];		/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				/* 5 GHz low */
				paparams[0] = (int32)pi->txpa_5g_low[0];	/* b0 */
				paparams[1] = (int32)pi->txpa_5g_low[1];	/* b1 */
				paparams[2] = (int32)pi->txpa_5g_low[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				/* 5 GHz middle */
				paparams[0] = (int32)pi->txpa_5g_mid[0];	/* b0 */
				paparams[1] = (int32)pi->txpa_5g_mid[1];	/* b1 */
				paparams[2] = (int32)pi->txpa_5g_mid[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
			default:
				/* 5 GHz high */
				paparams[0] = (int32)pi->txpa_5g_hi[0];		/* b0 */
				paparams[1] = (int32)pi->txpa_5g_hi[1];		/* b1 */
				paparams[2] = (int32)pi->txpa_5g_hi[2];		/* a1 */
				break;
			}
			bcopy(&paparams, a, 3*sizeof(int32));
		}
		break;

	case IOV_SVAL(IOV_SSLPNPHY_PAPARAMS):
		{
			int32 paparams[3];
			int32 a1, b0, b1;
			int32 tssi, pwr;
			phytbl_info_t tab;
			uint freq;

			bcopy(p, paparams, 3*sizeof(int32));
			b0 = paparams[0];
			b1 = paparams[1];
			a1 = paparams[2];

			tab.tbl_id = SSLPNPHY_TBL_ID_TXPWRCTL;
			tab.tbl_width = 32;	/* 32 bit wide	*/
			/* Convert tssi to power LUT */
			tab.tbl_ptr = &pwr; /* ptr to buf */
			tab.tbl_len = 1;        /* # values   */
			tab.tbl_offset = 0; /* estPwrLuts */
			for (tssi = 0; tssi < 64; tssi++) {
				pwr = wlc_sslpnphy_tssi2dbm(tssi, a1, b0, b1);
				wlc_sslpnphy_write_table(pi,  &tab);
				tab.tbl_offset++;
			}

			freq = wlc_phy_channel2freq(CHSPEC_CHANNEL(pi->radio_chanspec));
			switch (wlc_phy_chanspec_freq2bandrange_lpssn(freq)) {
			case WL_CHAN_FREQ_RANGE_2G:
				/* 2.4 GHz */
				pi->txpa_2g[0] = (int16)paparams[0];		/* b0 */
				pi->txpa_2g[1] = (int16)paparams[1];		/* b1 */
				pi->txpa_2g[2] = (int16)paparams[2];		/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GL:
				/* 5 GHz low */
				pi->txpa_5g_low[0] = (int16)paparams[0];	/* b0 */
				pi->txpa_5g_low[1] = (int16)paparams[1];	/* b1 */
				pi->txpa_5g_low[2] = (int16)paparams[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GM:
				/* 5 GHz middle */
				pi->txpa_5g_mid[0] = (int16)paparams[0];	/* b0 */
				pi->txpa_5g_mid[1] = (int16)paparams[1];	/* b1 */
				pi->txpa_5g_mid[2] = (int16)paparams[2];	/* a1 */
				break;

			case WL_CHAN_FREQ_RANGE_5GH:
			default:
				/* 5 GHz high */
				pi->txpa_5g_hi[0] = (int16)paparams[0];		/* b0 */
				pi->txpa_5g_hi[1] = (int16)paparams[1];		/* b1 */
				pi->txpa_5g_hi[2] = (int16)paparams[2];		/* a1 */
				break;
			}
		}
		break;

	case IOV_GVAL(IOV_SSLPNPHY_FULLCAL):	/* conver to SVAL with parameters like NPHY ? */
		wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_SSLPNPHY_FULLCAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;
#endif /* SSLPNCONF */
#if LCNCONF
	case IOV_GVAL(IOV_LCNPHY_PAPDEPSTBL):
	{
		lpphytbl_info_t tab;
		uint32 papdepstbl[PHY_PAPD_EPS_TBL_SIZE_LCNPHY];

		/* Preset PAPD eps table */
		tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
		tab.tbl_id = LCNPHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_offset = 0;
		tab.tbl_width = 32;
		tab.tbl_phywidth = 32;
		tab.tbl_ptr = &papdepstbl[0];

		/* read the table */
		wlc_phy_table_read_lpphy(pi, &tab);
		bcopy(&papdepstbl[0], a, PHY_PAPD_EPS_TBL_SIZE_LCNPHY*sizeof(uint32));
	}
	break;
#endif /* LCNCONF */
#endif 

#if LPCONF
	case IOV_GVAL(IOV_LPPHY_TEMPSENSE):
		int_val = wlc_phy_tempsense_lpphy(pi);
		bcopy(&int_val, a, sizeof(int_val));
		break;
#endif
	case IOV_GVAL(IOV_LPPHY_CAL_DELTA_TEMP):
		int_val = pi_lp->lpphy_cal_delta_temp;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_CAL_DELTA_TEMP):
		pi_lp->lpphy_cal_delta_temp = (int8)int_val;
		break;
#if LPCONF
	case IOV_GVAL(IOV_LPPHY_VBATSENSE):
		int_val = wlc_phy_vbatsense_lpphy(pi);
		bcopy(&int_val, a, sizeof(int_val));
		break;
#endif

#if LPCONF
	case IOV_SVAL(IOV_LPPHY_RX_GAIN_TEMP_ADJ_TEMPSENSE):
		pi_lp->lpphy_rx_gain_temp_adj_tempsense = (int8)int_val;
		break;

	case IOV_GVAL(IOV_LPPHY_RX_GAIN_TEMP_ADJ_TEMPSENSE):
		int_val = (int32)pi_lp->lpphy_rx_gain_temp_adj_tempsense;
		bcopy(&int_val, a, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_LPPHY_RX_GAIN_TEMP_ADJ_THRESH):
	  {
	    uint32 thresh = (uint32)int_val;
	    pi_lp->lpphy_rx_gain_temp_adj_thresh[0] = (thresh & 0xff);
	    pi_lp->lpphy_rx_gain_temp_adj_thresh[1] = ((thresh >> 8) & 0xff);
	    pi_lp->lpphy_rx_gain_temp_adj_thresh[2] = ((thresh >> 16) & 0xff);
	    wlc_phy_rx_gain_temp_adj_lpphy(pi);
	  }
	  break;

	case IOV_GVAL(IOV_LPPHY_RX_GAIN_TEMP_ADJ_THRESH):
	  {
	    uint32 thresh;
	    thresh = (uint32)pi_lp->lpphy_rx_gain_temp_adj_thresh[0];
	    thresh |= ((uint32)pi_lp->lpphy_rx_gain_temp_adj_thresh[1])<<8;
	    thresh |= ((uint32)pi_lp->lpphy_rx_gain_temp_adj_thresh[2])<<16;
	    bcopy(&thresh, a, sizeof(thresh));
	  }
	  break;

	case IOV_SVAL(IOV_LPPHY_RX_GAIN_TEMP_ADJ_METRIC):
		pi_lp->lpphy_rx_gain_temp_adj_metric = (int8)(int_val & 0xff);
		pi_lp->lpphy_rx_gain_temp_adj_tempsense_metric = (int8)((int_val >> 8) & 1);
		wlc_phy_rx_gain_temp_adj_lpphy(pi);
		break;

	case IOV_GVAL(IOV_LPPHY_RX_GAIN_TEMP_ADJ_METRIC):
		int_val = (int32)pi_lp->lpphy_rx_gain_temp_adj_metric;
		int_val |= (int32)((pi_lp->lpphy_rx_gain_temp_adj_tempsense_metric << 8) & 0x100);
		bcopy(&int_val, a, sizeof(int_val));
		break;
#endif /* LPCONF */
	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

uint32
wlc_phy_cap_get(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint32	cap = 0;

	switch (pi->pubpi.phy_type) {
	case PHY_TYPE_N:
		cap |= PHY_CAP_40MHZ;
		if (NREV_GE(pi->pubpi.phy_rev, 3))
			cap |= (PHY_CAP_SGI | PHY_CAP_STBC);
		break;

	case PHY_TYPE_HT:
		cap |= (PHY_CAP_40MHZ | PHY_CAP_SGI | PHY_CAP_STBC);
		break;

	case PHY_TYPE_SSN:
		if (SSLPNREV_GT(pi->pubpi.phy_rev, 2))
			cap |= PHY_CAP_40MHZ;
		cap |= PHY_CAP_SGI;
		break;

	case PHY_TYPE_LCN:
		cap |= PHY_CAP_SGI;
		break;
	case PHY_TYPE_LCN40:
		cap |= (PHY_CAP_SGI | PHY_CAP_40MHZ);
		break;

	default:
		break;
	}
	return cap;
}

/* %%%%%% IOCTL */
int
wlc_phy_ioctl(wlc_phy_t *pih, int cmd, int len, void *arg, bool *ta_ok)
{
	phy_info_t *pi = (phy_info_t *)pih;
	int bcmerror = 0;
	int val, *pval;
	bool bool_val;

	/* default argument is generic integer */
	pval = arg ? (int*)arg : NULL;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	else
		val = 0;

	/* bool conversion to avoid duplication below */
	bool_val = val != 0;

	switch (cmd) {

	case WLC_GET_PHY_NOISE:
		ASSERT(pval != NULL);
		*pval = wlc_phy_noise_avg(pih);
		break;

	case WLC_RESTART:
		/* Reset calibration results to uninitialized state in order to
		 * trigger recalibration next time wlc_init() is called.
		 */
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
		wlc_set_phy_uninitted(pi);
		break;

#if defined(BCMDBG)|| defined(WLTEST)
	case WLC_GET_RADIOREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}
		ASSERT(pval != NULL);

		wlc_phyreg_enter(pih);
		wlc_radioreg_enter(pih);
		if ((val == RADIO_IDCODE) && (!ISHTPHY(pi)))
			*pval = read_radio_id(pi);
		else
			*pval = read_radio_reg(pi, (uint16)val);
		wlc_radioreg_exit(pih);
		wlc_phyreg_exit(pih);
		break;

	case WLC_SET_RADIOREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		wlc_phyreg_enter(pih);
		wlc_radioreg_enter(pih);
		write_radio_reg(pi, (uint16)val, (uint16)(val >> NBITS(uint16)));
		wlc_radioreg_exit(pih);
		wlc_phyreg_exit(pih);
		break;
#endif 

#if defined(BCMDBG)
	case WLC_GET_TX_PATH_PWR:

		*pval = (read_radio_reg(pi, RADIO_2050_PU_OVR) & 0x84) ? 1 : 0;
		break;

	case WLC_SET_TX_PATH_PWR:

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		wlc_phyreg_enter(pih);
		wlc_radioreg_enter(pih);
		if (bool_val) {
			/* Enable overrides */
			write_radio_reg(pi, RADIO_2050_PU_OVR,
				0x84 | (read_radio_reg(pi, RADIO_2050_PU_OVR) &
				0xf7));
		} else {
			/* Disable overrides */
			write_radio_reg(pi, RADIO_2050_PU_OVR,
				read_radio_reg(pi, RADIO_2050_PU_OVR) & ~0x84);
		}
		wlc_radioreg_exit(pih);
		wlc_phyreg_exit(pih);
		break;
#endif /* BCMDBG */

#if defined(BCMDBG) || defined(WLTEST) || defined(WLMEDIA_N2DBG)
	case WLC_GET_PHYREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		wlc_phyreg_enter(pih);

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
			(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
			OSL_DELAY(1);
		}

		ASSERT(pval != NULL);
		*pval = phy_reg_read(pi, (uint16)val);

		if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
			wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  0);

		wlc_phyreg_exit(pih);
		break;

	case WLC_SET_PHYREG:
		*ta_ok = TRUE;

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		wlc_phyreg_enter(pih);
		phy_reg_write(pi, (uint16)val, (uint16)(val >> NBITS(uint16)));
		wlc_phyreg_exit(pih);
		break;
#endif 

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_TSSI: {

		if (!pi->sh->clk) {
			bcmerror = BCME_NOCLK;
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter(pih);
		ASSERT(pval != NULL);
		*pval = 0;
		switch (pi->pubpi.phy_type) {
		case PHY_TYPE_LP:
			CASECHECK(PHYTYPE, PHY_TYPE_LP);
			{
			int8 ofdm_pwr = 0, cck_pwr = 0;

			wlc_phy_get_tssi_lpphy(pi, &ofdm_pwr, &cck_pwr);
			*pval =  ((uint16)ofdm_pwr << 8) | (uint16)cck_pwr;
			break;

			}
		case PHY_TYPE_SSN:
			CASECHECK(PHYTYPE, PHY_TYPE_SSN);
			{
				int8 ofdm_pwr = 0, cck_pwr = 0;

				wlc_sslpnphy_get_tssi(pi, &ofdm_pwr, &cck_pwr);
				*pval =  ((uint16)ofdm_pwr << 8) | (uint16)cck_pwr;
				break;
			}
		case PHY_TYPE_LCN:
			PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
			CASECHECK(PHYTYPE, PHY_TYPE_LCN);
			{
				int8 ofdm_pwr = 0, cck_pwr = 0;

				wlc_lcnphy_get_tssi(pi, &ofdm_pwr, &cck_pwr);
				*pval =  ((uint16)ofdm_pwr << 8) | (uint16)cck_pwr;
				break;
			}
		case PHY_TYPE_LCN40:
			PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
			CASECHECK(PHYTYPE, PHY_TYPE_LCN40);
			{
				int8 ofdm_pwr = 0, cck_pwr = 0;

				wlc_lcn40phy_get_tssi(pi, &ofdm_pwr, &cck_pwr);
				*pval =  ((uint16)ofdm_pwr << 8) | (uint16)cck_pwr;
				break;
			}
		case PHY_TYPE_N:
			CASECHECK(PHYTYPE, PHY_TYPE_N);
			{
			*pval = (phy_reg_read(pi, NPHY_TSSIBiasVal1) & 0xff) << 8;
			*pval |= (phy_reg_read(pi, NPHY_TSSIBiasVal2) & 0xff);
			break;
			}
		case PHY_TYPE_A:
			CASECHECK(PHYTYPE, PHY_TYPE_A);
			*pval = (phy_reg_read(pi, APHY_TSSI_STAT) & 0xff) << 8;
			break;
		case PHY_TYPE_G:
			CASECHECK(PHYTYPE, PHY_TYPE_G);
			*pval = (phy_reg_read(pi,
				(GPHY_TO_APHY_OFF + APHY_TSSI_STAT)) & 0xff) << 8;
			*pval |= (phy_reg_read(pi, BPHY_TSSI) & 0xff);
			break;
		}

		wlc_phyreg_exit(pih);
		wlapi_enable_mac(pi->sh->physhim);
		break;
	}

	case WLC_GET_ATTEN: {
		atten_t *atten = (atten_t *)pval;
		ASSERT(pval != NULL);

		if (!ISGPHY(pi)) {
			bcmerror = BCME_BADBAND;
			break;
		}
		wlc_get_11b_txpower(pi, atten);
		break;
	}

	case WLC_SET_ATTEN: {
		atten_t *atten = (atten_t *)pval;
		ASSERT(pval != NULL);

		if (!pi->sh->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		if ((atten->auto_ctrl != WL_ATTEN_APP_INPUT_PCL_OFF) &&
			(atten->auto_ctrl != WL_ATTEN_PCL_ON) &&
			(atten->auto_ctrl != WL_ATTEN_PCL_OFF)) {
			bcmerror = BCME_BADARG;
			break;
		}

		if (!ISGPHY(pi)) {
			bcmerror = BCME_BADBAND;
			break;
		}

		wlc_phyreg_enter(pih);
		wlc_radioreg_enter(pih);
		wlc_phy_set_11b_txpower(pi, atten);
		wlc_radioreg_exit(pih);
		wlc_phyreg_exit(pih);
		break;
	}

	case WLC_GET_PWRIDX:
		if (!ISAPHY(pi)) {
			bcmerror = BCME_BADBAND;
			break;
		}

		ASSERT(pval != NULL);
		*pval = pi->txpwridx;
		break;

	case WLC_SET_PWRIDX:	/* set A band radio/baseband power index */
		if (!pi->sh->up) {
			bcmerror = BCME_NOTUP;
			break;
		}

		if (!ISAPHY(pi)) {
			bcmerror = BCME_BADBAND;
			break;
		}

		if ((val < WL_PWRIDX_LOWER_LIMIT) || (val > WL_PWRIDX_UPPER_LIMIT)) {
			bcmerror = BCME_RANGE;
			break;
		}

		{
		bool override;
		override = (val == WL_PWRIDX_PCL_ON) ? FALSE : TRUE;
		wlc_set_11a_txpower(pi, (int8)val, override);
		}
		break;

	case WLC_LONGTRAIN:
		{
		longtrnfn_t long_train_fn = NULL;

		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		long_train_fn = pi->pi_fptr.longtrn;
		if (long_train_fn)
			bcmerror = (*long_train_fn)(pi, val);
		else
			PHY_ERROR(("WLC_LONGTRAIN: unsupported phy type"));

			break;
		}

	case WLC_EVM:
		ASSERT(arg != NULL);
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}

		bcmerror = wlc_phy_test_evm(pi, val, *(((uint *)arg) + 1), *(((int *)arg) + 2));
		break;

	case WLC_FREQ_ACCURACY:
#if !SSLPNCONF
		/* SSLPNCONF transmits a few frames before running PAPD Calibration
		 * it does papd calibration each time it enters a new channel
		 * We cannot be down for this reason
		 */
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
#endif

		bcmerror = wlc_phy_test_freq_accuracy(pi, val);
		break;

	case WLC_CARRIER_SUPPRESS:
#if !SSLPNCONF
		if (pi->sh->up) {
			bcmerror = BCME_NOTDOWN;
			break;
		}
#endif
		bcmerror = wlc_phy_test_carrier_suppress(pi, val);
		break;
#endif 

#if defined(WLTEST) || defined(WL_PHYACIARGS)
	case WLC_GET_ACI_ARGS:
		ASSERT(arg != NULL);
		bcmerror = wlc_phy_aci_args(pi, arg, TRUE, len);
		break;

	case WLC_SET_ACI_ARGS:
		ASSERT(arg != NULL);
		bcmerror = wlc_phy_aci_args(pi, arg, FALSE, len);
		break;

#endif 

	case WLC_GET_INTERFERENCE_MODE:
		ASSERT(pval != NULL);
		*pval = pi->sh->interference_mode;
		if (pi->aci_state & ACI_ACTIVE)
			*pval |= AUTO_ACTIVE;
		break;

	case WLC_SET_INTERFERENCE_MODE:
		if (val < INTERFERE_NONE || val > WLAN_AUTO_W_NOISE) {
			bcmerror = BCME_RANGE;
			break;
		}

		if (pi->sh->interference_mode == val)
			break;

		/* push to sw state */
		pi->sh->interference_mode = val;

		if (!pi->sh->up)
			break;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* turn interference mode to off before entering another mode */
		if (val != INTERFERE_NONE)
			wlc_phy_interference(pi, INTERFERE_NONE, TRUE);

		if (!wlc_phy_interference(pi, pi->sh->interference_mode, TRUE))
			bcmerror = BCME_BADOPTION;

		wlapi_enable_mac(pi->sh->physhim);
		break;

	case WLC_GET_INTERFERENCE_OVERRIDE_MODE:
		if (!(ISNPHY(pi) || ISHTPHY(pi))) {
			break;
		}

		if (ISNPHY(pi) && !NREV_GE(pi->pubpi.phy_rev, 3)) {
			break;
		}

		ASSERT(pval != NULL);
		if (pi->sh->interference_mode_override == FALSE) {
			*pval = INTERFERE_OVRRIDE_OFF;
		} else {
			*pval = pi->sh->interference_mode;
		}
		break;

	case WLC_SET_INTERFERENCE_OVERRIDE_MODE:
		if (!(ISNPHY(pi) || ISHTPHY(pi))) {
			break;
		}

		if (ISNPHY(pi) && !NREV_GE(pi->pubpi.phy_rev, 3)) {
			break;
		}

		if (val < INTERFERE_OVRRIDE_OFF || val > WLAN_AUTO_W_NOISE) {
			bcmerror = BCME_RANGE;
			break;
		}

		if (val == INTERFERE_OVRRIDE_OFF) {
			/* this is a reset */
			pi->sh->interference_mode_override = FALSE;
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				pi->sh->interference_mode =
					pi->sh->interference_mode_2G;
			} else {
				pi->sh->interference_mode =
					pi->sh->interference_mode_5G;
			}
		} else {

			pi->sh->interference_mode_override = TRUE;

			/* push to sw state */
			if (ISHTPHY(pi) || (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3))) {
				pi->sh->interference_mode_2G_override = val;
				pi->sh->interference_mode_5G_override = val;

				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					/* for 2G, all values 0 thru 4 are valid */
					pi->sh->interference_mode =
						pi->sh->interference_mode_2G_override;
				} else {
					/* for 5G, only values 0 and 1 are valid options */
					if (val == 0 || val == 1) {
						pi->sh->interference_mode =
							pi->sh->interference_mode_5G_override;
					} else {
						/* default 5G interference value to 0 */
						pi->sh->interference_mode = 0;
					}
				}
			} else {
				pi->sh->interference_mode = val;
			}
		}


		if (!pi->sh->up)
			break;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* turn interference mode to off before entering another mode */
		if (val != INTERFERE_NONE)
			wlc_phy_interference(pi, INTERFERE_NONE, TRUE);

		if (!wlc_phy_interference(pi, pi->sh->interference_mode, TRUE))
			bcmerror = BCME_BADOPTION;

		wlapi_enable_mac(pi->sh->physhim);
		break;

	default:
		bcmerror = BCME_UNSUPPORTED;
	}

	return bcmerror;
}

/* WARNING: check (radioid != NORADIO_ID) before doing any radio related calibrations */
int32
wlc_phy_watchdog(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	bool delay_phy_cal = FALSE;	/* avoid more than one phy cal at same
					 * pi->sh->now instant
					 */
	phy_info_abgphy_t *pi_abg = NULL;

	if (ISABGPHY(pi))
		pi_abg = pi->u.pi_abgphy;

	if (ISLCNPHY(pi))
	  wlc_lcnphy_noise_measure_stop(pi);
	pi->sh->now++;

#ifdef TENDA_PHY_STATUS   
    td_phy_status_update(pi);
#endif
#ifdef NOISE_CAL_LCNXNPHY
	if ((NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
	NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV + 2))) {
		if ((!(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi))) &&
		(!(pi->measure_hold & PHY_HOLD_FOR_PKT_ENG)))
			wlc_phy_aci_noise_measure_nphy(pi, FALSE);
	}
#endif
	if (!pi->phywatchdog_override)
		return BCME_OK;

	/* defer interference checking, scan and update if RM is progress */
	if (!SCAN_RM_IN_PROGRESS(pi) &&
	    (ISNPHY(pi) || ISHTPHY(pi) || ISGPHY(pi) || ISSSLPNPHY(pi) || ISLPPHY(pi))) {
		/* interf.scanroamtimer counts transient time coming out of scan */
		if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) ||
			ISHTPHY(pi)) {
			if (pi->interf.scanroamtimer != 0)
				pi->interf.scanroamtimer -= 1;
		}
		wlc_phy_aci_upd(pi);
	} else {
		if ((ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 3)) ||
			ISHTPHY(pi)) {
			/* in a scan or radio meas, don't update moving average
			 * when we first come out of scan or roam
			*/
			pi->interf.scanroamtimer = 2;
		}
	}

	/* Remove the call here since its allready part of perical run */
	if ((pi->sh->now % pi->sh->fast_timer) == 0) {
		wlc_phy_update_bt_chanspec(pi, pi->radio_chanspec);

		if (ISLPPHY(pi) && (BCM2062_ID == LPPHY_RADIO_ID(pi))) {
			wlc_phy_radio_2062_check_vco_cal(pi);
		}
	}

	/* update phy noise moving average only if no scan or rm in progress */
	if (!(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi))) {
		wlc_phy_noise_sample_request((wlc_phy_t*)pi, PHY_NOISE_SAMPLE_MON,
			CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	/* reset phynoise state if ucode interrupt doesn't arrive for so long */
	if (pi->phynoise_state && (pi->sh->now - pi->phynoise_now) > 5) {
		PHY_TMP(("wlc_phy_watchdog: ucode phy noise sampling overdue\n"));
		pi->phynoise_state = 0;
	}

	/* fast_timer driven event */
	if ((!pi->phycal_txpower) ||
	    ((pi->sh->now - pi->phycal_txpower) >= pi->sh->fast_timer)) {
		/* SW power control: Keep attempting txpowr recalc until successfully */
		if (!SCAN_INPROG_PHY(pi) && wlc_phy_cal_txpower_recalc_sw(pi)) {
			pi->phycal_txpower = pi->sh->now;
		}
	}

	/* abort if no radio */
	if (NORADIO_ENAB(pi->pubpi))
		return BCME_OK;

	/* abort if cal should be blocked(e.g. not in home channel) */
	if ((SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || ASSOC_INPROG_PHY(pi)))
		return BCME_OK;

#ifdef WLNOKIA_NVMEM
	if (pi->sh->now % pi->sh->glacial_timer) {
		int8 vbat = wlc_phy_env_measure_vbat(pi);
		int8 temp = wlc_phy_env_measure_temperature(pi);
		if (wlc_phy_noknvmem_env_check(pi, vbat, temp))
			wlc_phy_txpower_recalc_target(pi);
	}

#endif /* WLNOKIA_NVMEM */

	if (ISGPHY(pi)) {
		/* glacial_timer driven event */
		if (!pi->disable_percal &&
		    (pi->sh->now - pi_abg->abgphy_cal.abgphy_cal_mlo) >= pi->sh->glacial_timer) {
			if ((GREV_GT(pi->pubpi.phy_rev, 1))) {
				wlc_phy_cal_measurelo_gphy(pi);
				delay_phy_cal = TRUE;
			}
		}

		/* slow_timer driven event */
		if ((pi->sh->now % pi->sh->slow_timer) == 0) {
			/* NOTE: wlc_phy_cal_txpower_stats_clr_gphy is to be called *after*
			 * wlc_gphy_measurelo in the case where both are called in
			 * the same pi->sh->now instant, otherwise the periodic
			 * calibration will skip all attenuator settings.
			 */
			if (!pi->hwpwrctrl)
				wlc_phy_cal_txpower_stats_clr_gphy(pi);
		}

		if (((pi->sh->now - pi_abg->abgphy_cal.abgphy_cal_noffset) >=
			pi->sh->slow_timer) &&
			(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_ADCDIV) &&
			(RADIOREV(pi->pubpi.radiorev) == 8) && !delay_phy_cal)
		    {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_cal_radio2050_nrssioffset_gmode1(pi);
			wlapi_enable_mac(pi->sh->physhim);
			delay_phy_cal = TRUE;
		}

		if (((pi->sh->now - pi_abg->abgphy_cal.abgphy_cal_nslope) >= pi->sh->slow_timer) &&
		    (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_ADCDIV) && !delay_phy_cal) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			wlc_phy_cal_radio2050_nrssislope(pi);
			wlc_phy_vco_cal(pi);
			wlapi_enable_mac(pi->sh->physhim);
			delay_phy_cal = TRUE;
		}
	}

	if ((ISNPHY(pi)) && !pi->disable_percal && !delay_phy_cal) {
		if (!((CHIPID(pi->sh->chip) == BCM43237_CHIP_ID) && DCS_INPROG_PHY(pi))) {


			/* 1) check to see if new cal needs to be activated */
			if ((pi->phy_cal_mode != PHY_PERICAL_DISABLE) &&
				(pi->phy_cal_mode != PHY_PERICAL_MANUAL) &&
			((pi->sh->now - pi->cal_info->last_cal_time) >= pi->sh->glacial_timer)) {
					wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_PERICAL_WATCHDOG);
					if (CHIPID(pi->sh->chip) == BCM43237_CHIP_ID)
						pi->cal_info->last_cal_time = pi->sh->now;
			}

			wlc_phy_txpwr_papd_cal_nphy(pi);

			wlc_phy_radio205x_check_vco_cal_nphy(pi);
		}
	}

	if (ISHTPHY(pi) && !pi->disable_percal)	{

		/* do we really want to look at disable_percal? we have an enable flag,
		 * so isn't this redundant? (nphy does this, but seems weird)
		 */

		/* check to see if a cal needs to be run */
		if ((pi->phy_cal_mode != PHY_PERICAL_DISABLE) &&
		    (pi->phy_cal_mode != PHY_PERICAL_MANUAL) &&
		    ((pi->sh->now - pi->cal_info->last_cal_time) >= pi->sh->glacial_timer)) {
			PHY_CAL(("wlc_phy_watchdog: watchdog fired (en=%d, now=%d,"
				"prev_time=%d, glac=%d)\n",
				pi->phy_cal_mode, pi->sh->now,
				pi->cal_info->last_cal_time,  pi->sh->glacial_timer));

			wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_PERICAL_WATCHDOG);
		}
	}


	if (ISLPPHY(pi)) {
		phy_info_lpphy_t *pi_lp = pi->u.pi_lpphy;
		if (pi->phy_forcecal || wlc_lpphy_txrxiq_cal_reqd(pi)) {
			if (!(pi->carrier_suppr_disable || pi->disable_percal)) {
				/* Perform Tx IQ and Rx IQ Calibrations */
				wlc_phy_periodic_cal_lpphy(pi);
			}
		}

		if (LPREV_GE(pi->pubpi.phy_rev, 2) &&
			((pi_lp->lpphy_force_papd_cal == TRUE) ||
			wlc_lpphy_papd_cal_reqd(pi))) {
			if (!(pi->carrier_suppr_disable || pi->disable_percal)) {
				/* Perform PAPD Calibration */
				wlc_phy_papd_cal_txpwr_lpphy(pi,
					pi_lp->lpphy_force_papd_cal);
			}
		}
	}

	if (pi->pi_fptr.phywatchdog)
		(*pi->pi_fptr.phywatchdog)(pi);

	if (wlapi_bmac_btc_mode_get(pi->sh->physhim))
	{
		uint16 btperiod;
		bool btactive;
		wlapi_bmac_btc_period_get(pi->sh->physhim, &btperiod, &btactive);
		wlc_phy_btc_adjust(pi, btactive, btperiod);
	}

	if (ISLCNPHY(pi) &&
	     !(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || ASSOC_INPROG_PHY(pi)))
	  wlc_lcnphy_noise_measure(pi);

	return BCME_OK;
}

void
wlc_phy_tx_pwr_limit_check(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	if (!pi->ucode_tssi_limit_en)
		return;

	if (ISLCNPHY(pi))
	  wlc_lcnphy_tx_pwr_limit_check(pi);
}


/* adjust phy setting based on bt states */
void
wlc_phy_btc_adjust(phy_info_t *pi, bool btactive, uint16 btperiod)
{
	if (btactive != pi->bt_active) {
		if (pi->pi_fptr.phybtcadjust) {
			(*pi->pi_fptr.phybtcadjust)(pi, btactive);
		}
	}

	pi->bt_period = btperiod;
	pi->bt_active = btactive;
}

#ifdef STA

#ifdef PR43338WAR
void
wlc_phy_11bap_reset(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	if (pi->war_b_ap) {
		pi->war_b_ap = FALSE;

		if (ISGPHY(pi))
			phy_reg_mod(pi, (GPHY_TO_APHY_OFF + APHY_CTHR_STHR_SHDIN),
				APHY_CTHR_CRS1_ENABLE, pi->war_b_ap_cthr_save);
		else if (ISNPHY(pi)) {

		}
	}
}

void
wlc_phy_11bap_restrict(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	if (!pi->war_b_ap) {
		pi->war_b_ap = TRUE;

		if (ISGPHY(pi)) {
			pi->war_b_ap_cthr_save = APHY_CTHR_CRS1_ENABLE;
			phy_reg_mod(pi, (GPHY_TO_APHY_OFF + APHY_CTHR_STHR_SHDIN),
				APHY_CTHR_CRS1_ENABLE, 0);
			/* TODO, block change to APHY_CTHR_STHR_SHDIN for the bit */
		}
	}
}
#endif	/* PR43338WAR */
#endif /* STA */

void
wlc_phy_BSSinit(wlc_phy_t *pih, bool bonlyap, int rssi)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint i;
	uint k;

	if (bonlyap) {
	}

	/* watchdog idle phy noise */
	for (i = 0; i < MA_WINDOW_SZ; i++) {
		pi->sh->phy_noise_window[i] = (int8)(rssi & 0xff);
	}
	if (ISLCNCOMMONPHY(pi)) {
		for (i = 0; i < MA_WINDOW_SZ; i++)
			pi->sh->phy_noise_window[i] = PHY_NOISE_FIXED_VAL_LCNPHY;
	}
	pi->sh->phy_noise_index = 0;

	if ((pi->sh->interference_mode == WLAN_AUTO) &&
	     (pi->aci_state & ACI_ACTIVE)) {
		/* Reset the clock to check again after the moving average buffer has filled
		 */
		pi->aci_start_time = pi->sh->now + MA_WINDOW_SZ;
	}

	for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
		for (k = WL_ANT_IDX_1; k < WL_ANT_RX_MAX; k++)
			pi->phy_noise_win[k][i] = PHY_NOISE_FIXED_VAL_NPHY;
	}
	pi->phy_noise_index = 0;
}


/* Convert epsilon table value to complex number */
void
wlc_phy_papd_decode_epsilon(uint32 epsilon, int32 *eps_real, int32 *eps_imag)
{
	if ((*eps_imag = (epsilon>>13)) > 0xfff)
		*eps_imag -= 0x2000; /* Sign extend */
	if ((*eps_real = (epsilon & 0x1fff)) > 0xfff)
		*eps_real -= 0x2000; /* Sign extend */
}

/* Atan table for cordic >> num2str(atan(1./(2.^[0:17]))/pi*180,8) */
static const fixed AtanTbl[] = {
	2949120,
	1740967,
	919879,
	466945,
	234379,
	117304,
	58666,
	29335,
	14668,
	7334,
	3667,
	1833,
	917,
	458,
	229,
	115,
	57,
	29
};

void
wlc_phy_cordic(fixed theta, cint32 *val)
{
	fixed angle, valtmp;
	unsigned iter;
	int signx = 1;
	int signtheta;

	val[0].i = CORDIC_AG;
	val[0].q = 0;
	angle    = 0;

	/* limit angle to -180 .. 180 */
	signtheta = (theta < 0) ? -1 : 1;
	theta = ((theta+FIXED(180)*signtheta)% FIXED(360))-FIXED(180)*signtheta;

	/* rotate if not in quadrant one or four */
	if (FLOAT(theta) > 90) {
		theta -= FIXED(180);
		signx = -1;
	} else if (FLOAT(theta) < -90) {
		theta += FIXED(180);
		signx = -1;
	}

	/* run cordic iterations */
	for (iter = 0; iter < CORDIC_NI; iter++) {
		if (theta > angle) {
			valtmp = val[0].i - (val[0].q >> iter);
			val[0].q = (val[0].i >> iter) + val[0].q;
			val[0].i = valtmp;
			angle += AtanTbl[iter];
		} else {
			valtmp = val[0].i + (val[0].q >> iter);
			val[0].q = -(val[0].i >> iter) + val[0].q;
			val[0].i = valtmp;
			angle -= AtanTbl[iter];
		}
	}

	/* re-rotate quadrant two and three points */
	val[0].i = val[0].i*signx;
	val[0].q = val[0].q*signx;
}

#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
int
wlc_phy_cal_cache_init(wlc_phy_t *ppi)
{
	return 0;
}

void
wlc_phy_cal_cache_deinit(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx = pi->phy_calcache;

	while (ctx) {
		pi->phy_calcache = ctx->next;
		MFREE(pi->sh->osh, ctx,
		      sizeof(ch_calcache_t));
		ctx = pi->phy_calcache;
	}

	pi->phy_calcache = NULL;

	/* No more per-channel contexts, switch in the default one */
	pi->cal_info = &pi->def_cal_info;
	/* Reset the parameters */
	pi->cal_info->last_cal_temp = -50;
	pi->cal_info->last_cal_time = 0;
}
#endif /* defined(PHYCAL_CACHING) || defined(WLMCHAN) */

#if defined(PHYCAL_CACHING) || defined(PHYCAL_CACHE_SMALL)
void
wlc_phy_cal_cache_set(wlc_phy_t *ppi, bool state)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->phy_calcache_on = state;
	if (!state)
		wlc_phy_cal_cache_deinit(ppi);
}

bool
wlc_phy_cal_cache_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->phy_calcache_on;
}
#endif /* defined(PHYCAL_CACHING) || defined(PHYCALCACHE_SMALL) */

void
wlc_phy_cal_perical_mphase_reset(phy_info_t *pi)
{
	phy_cal_info_t *cal_info = pi->cal_info;

#ifdef WLMCHAN
	PHY_CAL(("wlc_phy_cal_perical_mphase_reset chanspec 0x%x ctx: %p\n", pi->radio_chanspec,
	           wlc_phy_get_chanctx(pi, pi->radio_chanspec)));
#else
	PHY_CAL(("wlc_phy_cal_perical_mphase_reset\n"));
#endif

	if (pi->phycal_timer)
		wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);

	if (ISNPHY(pi))
		pi->u.pi_nphy->cal_type_override = PHY_PERICAL_AUTO;
	cal_info->cal_phase_id = MPHASE_CAL_STATE_IDLE;
	cal_info->txcal_cmdidx = 0; /* needed in nphy only */
}

static void
wlc_phy_cal_perical_mphase_schedule(phy_info_t *pi, uint delay_val)
{
	/* for manual mode, let it run */
	if ((pi->phy_cal_mode != PHY_PERICAL_MPHASE) &&
	    (pi->phy_cal_mode != PHY_PERICAL_MANUAL))
		return;

	PHY_CAL(("wlc_phy_cal_perical_mphase_schedule\n"));

	/* use timer to wait for clean context since this may be called in the middle of nphy_init
	 */
	wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);

	pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_INIT;
	wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, delay_val, 0);
}

/* policy entry */
void
wlc_phy_cal_perical(wlc_phy_t *pih, uint8 reason)
{
	int16 nphy_currtemp = 0;
	int16 delta_temp = 0;
	bool  suppress_cal = FALSE;
	phy_info_t *pi = (phy_info_t*)pih;
	phy_info_nphy_t *pi_nphy = NULL;
#if defined(WLMCHAN) || defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif

	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi) && !ISHTPHY(pi))
		return;

	if ((pi->phy_cal_mode == PHY_PERICAL_DISABLE) ||
	    (pi->phy_cal_mode == PHY_PERICAL_MANUAL))
		return;

	/* NPHY_IPA : disable PAPD cal for following calibration at least 4322A1? */

	PHY_CAL(("wlc_phy_cal_perical: reason %d chanspec 0x%x\n", reason,
	         pi->radio_chanspec));

	/* perical is enable: either single phase only, or mphase is allowed
	 *  dispatch to s-phase or m-phase based on reasons
	 */
	switch (reason) {
	case PHY_PERICAL_DRIVERUP:	/* always single phase ? */
		break;

	case PHY_PERICAL_PHYINIT:
		if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
#ifdef WLMCHAN
			if (ctx) {
				/* Switched the context so restart a pending MPHASE cal or
				 * restore stored calibration
				 */
				ASSERT(ctx->chanspec == pi->radio_chanspec);

				/* If it was pending last time, just restart it */
				if (PHY_PERICAL_MPHASE_PENDING(pi)) {
					/* Delete any existing timer just in case */
					PHY_CAL(("%s: Restarting calibration for 0x%x phase %d\n",
					         __FUNCTION__, ctx->chanspec,
					         pi->cal_info->cal_phase_id));
					wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
					wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
				} else if (wlc_phy_cal_cache_restore(pi) != BCME_ERROR)
					break;
			} else
#endif /* WLMCHAN */
			if (PHY_PERICAL_MPHASE_PENDING(pi)) {
				wlc_phy_cal_perical_mphase_reset(pi);
			}

			pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_RESTART;
			/* schedule mphase cal */
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_INIT_DELAY);
		}
		break;

	case PHY_PERICAL_JOIN_BSS:
	case PHY_PERICAL_START_IBSS:
	case PHY_PERICAL_UP_BSS:

		/* for these, want single phase cal to ensure immediately
		 *  clean Tx/Rx so auto-rate fast-start is promising
		 */
		if ((pi->phy_cal_mode == PHY_PERICAL_MPHASE) &&
			PHY_PERICAL_MPHASE_PENDING(pi)) {
			wlc_phy_cal_perical_mphase_reset(pi);
		}

		/* Always do idle TSSI measurement at the end of NPHY cal
		   while starting/joining a BSS/IBSS
		*/
		pi->first_cal_after_assoc = TRUE;
		if (ISNPHY(pi))
			pi->u.pi_nphy->cal_type_override = PHY_PERICAL_FULL; /* not used in htphy */


		if (pi->phycal_tempdelta) {
			if (ISNPHY(pi))
				pi->cal_info->last_cal_temp = wlc_phy_tempsense_nphy(pi);
			else if (ISHTPHY(pi))
				pi->cal_info->last_cal_temp = wlc_phy_tempsense_htphy(pi);
		}
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
		if (ctx) {
			PHY_CAL(("wl%d: %s: Attempting to restore cals on JOIN...\n",
			          pi->sh->unit, __FUNCTION__));
			if (wlc_phy_cal_cache_restore(pi) == BCME_ERROR) {
				if (ISNPHY(pi)) {
					wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_FULL);
				} else if (ISHTPHY(pi)) {
					wlc_phy_cals_htphy(pi, PHY_CAL_SEARCHMODE_RESTART);
				}
			}
		}
		else
#endif /* PHYCAL_CACHING */
			if (ISNPHY(pi)) {
				wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_FULL);
			} else if (ISHTPHY(pi)) {
				wlc_phy_cals_htphy(pi, PHY_CAL_SEARCHMODE_RESTART);
			}
		break;

	case PHY_PERICAL_WATCHDOG:
		if (pi->phycal_tempdelta && ISNPHY(pi)) {
			nphy_currtemp = wlc_phy_tempsense_nphy(pi);
			delta_temp =
			    (nphy_currtemp > pi->cal_info->last_cal_temp)?
			    nphy_currtemp - pi->cal_info->last_cal_temp :
			    pi->cal_info->last_cal_temp - nphy_currtemp;

			/* Only do WATCHDOG triggered (periodic) calibration if
			   the channel hasn't changed and if the temperature delta
			   is above the specified threshold
			*/
			if ((delta_temp < (int16)pi->phycal_tempdelta) &&
			    (pi->cal_info->u.ncal.txiqlocal_chanspec == pi->radio_chanspec)) {
				suppress_cal = TRUE;
			} else {
				pi->cal_info->last_cal_temp = nphy_currtemp;
			}
		}

		if (!suppress_cal) {

			/* if mphase is allowed, do it, otherwise, fall back to single phase */
			if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
				/* only schedule if it's not in progress */
				if (!PHY_PERICAL_MPHASE_PENDING(pi)) {
					pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_REFINE;
					wlc_phy_cal_perical_mphase_schedule(pi,
						PHY_PERICAL_WDOG_DELAY);
				}
			} else if (pi->phy_cal_mode == PHY_PERICAL_SPHASE) {
				if (ISNPHY(pi)) {
					wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_AUTO);
				} else if (ISHTPHY(pi)) {
					wlc_phy_cals_htphy(pi, PHY_CAL_SEARCHMODE_RESTART);
				}
			} else {
				ASSERT(0);
			}
		}
		break;

	case PHY_PERICAL_DCS:
		if (PHY_PERICAL_MPHASE_PENDING(pi)) {
			wlc_phy_cal_perical_mphase_reset(pi);
			if (pi->phycal_tempdelta) {
				nphy_currtemp = wlc_phy_tempsense_nphy(pi);
				pi->cal_info->last_cal_temp = nphy_currtemp;
			}
		} else if (pi->phycal_tempdelta) {
			nphy_currtemp = wlc_phy_tempsense_nphy(pi);
			delta_temp =
			    (nphy_currtemp > pi->cal_info->last_cal_temp)?
			    nphy_currtemp - pi->cal_info->last_cal_temp :
			    pi->cal_info->last_cal_temp - nphy_currtemp;

			if ((delta_temp < (int16)pi->phycal_tempdelta)) {
				suppress_cal = TRUE;
			} else {
				pi->cal_info->last_cal_temp = nphy_currtemp;
			}
		}

		if (suppress_cal) {
			wlc_phy_txpwr_papd_cal_nphy_dcs(pi);
		} else {
			/* only mphase is allowed */
			if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
				pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_REFINE;
				wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_WDOG_DELAY);
			} else {
				ASSERT(0);
			}
		}
		break;

	default:
		ASSERT(0);
		break;
	}
}

void
wlc_phy_cal_perical_mphase_restart(phy_info_t *pi)
{
	PHY_CAL(("wlc_phy_cal_perical_mphase_restart\n"));
	pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_INIT;
	pi->cal_info->txcal_cmdidx = 0;
}

uint8
wlc_phy_nbits(int32 value)
{
	int32 abs_val;
	uint8 nbits = 0;

	abs_val = ABS(value);
	while ((abs_val >> nbits) > 0) nbits++;

	return nbits;
}

uint32
wlc_phy_sqrt_int(uint32 value)
{
	uint32 root = 0, shift = 0;

	/* Compute integer nearest to square root of input integer value */
	for (shift = 0; shift < 32; shift += 2) {
		if (((0x40000000 >> shift) + root) <= value) {
			value -= ((0x40000000 >> shift) + root);
			root = (root >> 1) | (0x40000000 >> shift);
		} else {
			root = root >> 1;
		}
	}

	/* round to the nearest integer */
	if (root < value) ++root;

	return root;
}

void
wlc_phy_stf_chain_init(wlc_phy_t *pih, uint8 txchain, uint8 rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;

	pi->sh->hw_phytxchain = txchain;
	pi->sh->hw_phyrxchain = rxchain;
	pi->sh->phytxchain = txchain;
	pi->sh->phyrxchain = rxchain;
}

void
wlc_phy_stf_chain_set(wlc_phy_t *pih, uint8 txchain, uint8 rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;

	PHY_TRACE(("wlc_phy_stf_chain_set, new phy chain tx %d, rx %d", txchain, rxchain));

	pi->sh->phytxchain = txchain;

	if (ISNPHY(pi)) {
		if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
			NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV+1))
			wlc_phy_rxcore_setstate_nphy(pih, rxchain, 1);
		else
			wlc_phy_rxcore_setstate_nphy(pih, rxchain, 0);

	} else if (ISHTPHY(pi)) {
		wlc_phy_rxcore_setstate_htphy(pih, rxchain);
	}
}

void
wlc_phy_stf_chain_get(wlc_phy_t *pih, uint8 *txchain, uint8 *rxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;

	*txchain = pi->sh->phytxchain;
	*rxchain = pi->sh->phyrxchain;
}

uint8
wlc_phy_rssi_ant_compare(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	int8 rssi1, rssi0;
	uint8 chainmap;

	rssi1 = pi->rssi1_avg;
	rssi0 = pi->rssi0_avg;

	if (rssi1 >= rssi0) {
		chainmap = 2;
	} else {
		chainmap = 1;
	}
	return chainmap;
}


uint8
wlc_phy_stf_chain_active_get(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint8 chainmap;

	if (!pi->phywatchdog_override)
		return pi->txcore_temp.bitmap;

	if (ISHTPHY(pi)) {
		wlc_phy_stf_chain_temp_throttle_htphy(pi);

	} else if (NREV_IS(pi->pubpi.phy_rev, 6) || NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
		NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV+1) ||
		(NREV_GE(pi->pubpi.phy_rev, 7) && (((RADIOREV(pi->pubpi.radiorev) == 5) &&
		((RADIOVER(pi->pubpi.radiover) == 0x1) ||
		(RADIOVER(pi->pubpi.radiover) == 0x2))) || (((pi->sh->chip == BCM43235_CHIP_ID) ||
		(pi->sh->chip == BCM43236_CHIP_ID) || (pi->sh->chip == BCM43238_CHIP_ID)) &&
		(pi->sh->chiprev >= 2)) || (pi->sh->chip == BCM43237_CHIP_ID)))) {

		/* Degrade-to-single-txchain is enabled for:
		 *     - NPHY rev 6:  all chips
		 *     - NPHY rev 7+: 5357B0 (radio 5v1), 5357B1 (radio5v2), 4323XB0+
		 */

		int16 nphy_currtemp;
		uint8 active_bitmap = pi->txcore_temp.bitmap;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		nphy_currtemp = wlc_phy_tempsense_nphy(pi);
		wlapi_enable_mac(pi->sh->physhim);

		if (!pi->txcore_temp.heatedup) {
			if (nphy_currtemp >= pi->txcore_temp.disable_temp) {
				/* conditioning the RSSI compare chainmap to only Sulley board */
				if (NREV_GE(pi->pubpi.phy_rev, 8) &&
				(pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)) {
					chainmap = wlc_phy_rssi_ant_compare(pih);
					active_bitmap = (active_bitmap & 0xf0) | chainmap;
				} else {
					active_bitmap &= 0xFD;
				}
				pi->txcore_temp.heatedup = TRUE;
				pi->txcore_temp.bitmap = active_bitmap;
			}
		} else {
			if (nphy_currtemp <= pi->txcore_temp.enable_temp) {
				if (NREV_GE(pi->pubpi.phy_rev, 8) &&
				(pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)) {
					active_bitmap |= 0x33;
					pi->txcore_temp.degrade1RXen = FALSE;
				} else {
					active_bitmap |= 0x2;
				}
				pi->txcore_temp.heatedup = FALSE;
				pi->txcore_temp.bitmap = active_bitmap;
			} else {
				/* Sulley: degrade to 1-RX feature pending on MAC support */
				if (0) {
					if (!pi->txcore_temp.degrade1RXen) {
						/* if temperature is still greater  */
						/* than disable temp_thres then degrade 1RX */
						if (nphy_currtemp >= pi->txcore_temp.disable_temp) {
							chainmap = wlc_phy_rssi_ant_compare(pih);
							active_bitmap = (chainmap << 4) | chainmap;
							pi->txcore_temp.degrade1RXen = TRUE;
							pi->txcore_temp.bitmap = active_bitmap;
						}
					}
			}
			}
		}
	}
	return pi->txcore_temp.bitmap;
}

int8
wlc_phy_stf_ssmode_get(wlc_phy_t *pih, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint8 siso_mcs_id, cdd_mcs_id;

	PHY_TRACE(("wl%d: %s: chanspec %x\n", pi->sh->unit, __FUNCTION__, chanspec));

	/* For simplicity, only MCS0/Legacy OFDM 6 Mbps is checked to choose between CDD and SISO */
	siso_mcs_id = (CHSPEC_IS40(chanspec)) ? TXP_FIRST_MCS_40_SISO : TXP_FIRST_MCS_20_SISO;
	cdd_mcs_id = (CHSPEC_IS40(chanspec)) ? TXP_FIRST_MCS_40_CDD : TXP_FIRST_MCS_20_CDD;

	/* criteria to choose stf mode */

	/* the "+3dbm (12 0.25db units)" is to account for the fact that with CDD, tx occurs
	 * on both chains
	 */
	if (pi->tx_power_target[siso_mcs_id] > (pi->tx_power_target[cdd_mcs_id] + 12))
		return PHY_TXC1_MODE_SISO;
	else
		return PHY_TXC1_MODE_CDD;
}

#ifdef SAMPLE_COLLECT

static int
wlc_phy_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *collect, void *buff)
{
	int ret;

	/* driver must be "out" (not up but chip is alive) */
	if (pi->sh->up)
		return BCME_NOTDOWN;
	if (!pi->sh->clk)
		return BCME_NOCLK;

	if (ISHTPHY(pi)) {
		ret = wlc_phy_sample_collect_htphy(pi, collect, (uint32 *)buff);
		return ret;
	} else if (ISNPHY(pi)) {
		if (NREV_LT(pi->pubpi.phy_rev, 7)) {
			ret = wlc_phy_sample_collect_old(pi, collect, buff);
		} else {
			ret = wlc_phy_sample_collect_nphy(pi, collect, (uint32 *)buff);
		}
		return ret;
	}

	return BCME_UNSUPPORTED;
}

static int
wlc_phy_sample_data(phy_info_t *pi, wl_sampledata_t *sample_data, void *b)
{
	int ret;

	/* driver must be "out" (not up but chip is alive) */
	if (pi->sh->up)
		return BCME_NOTDOWN;
	if (!pi->sh->clk)
		return BCME_NOCLK;

	if (ISHTPHY(pi)) {
		ret = wlc_phy_sample_data_htphy(pi, sample_data, b);
		return ret;
	} else if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 7)) {
		ret = wlc_phy_sample_data_nphy(pi, sample_data, b);
		return ret;
	}

	return BCME_UNSUPPORTED;
}

static int
wlc_phy_sample_collect_old(phy_info_t *pi, wl_samplecollect_args_t *collect, void *buff)
{
	int cores;
	uint8 coll_us;
	uint16 *ptr = (uint16 *) buff;
	uint samp_freq_MHz, byte_per_samp, samp_count, byte_count;
	int start_core, end_core, core;
	uint16 mask, val, coll_type = 1;
	uint16 crsctl, crsctlu, crsctll;

	osl_t *osh;
	osh = pi->sh->osh;

	cores = collect->cores;
	coll_us = collect->coll_us;

	if (cores > 1)
		return BCME_RANGE;

	if (IS40MHZ(pi)) {
		samp_freq_MHz = 40;
		byte_per_samp = 2;
		if (cores < 0) {
			start_core = 0;
			end_core = 1;
		} else {
			start_core = end_core = cores;
		}
	} else {
		samp_freq_MHz = 20;
		byte_per_samp = 4;
		start_core = end_core = 0;
	}
	samp_count = coll_us * samp_freq_MHz;
	byte_count = samp_count * byte_per_samp;

	if (byte_count > WLC_SAMPLECOLLECT_MAXLEN) {
		PHY_ERROR(("wl%d: %s: Not sufficient memory for this collect, byte_count: %d\n",
			pi->sh->unit, __FUNCTION__, byte_count));
		return BCME_ERROR;
	}

	wlapi_ucode_sample_init(pi->sh->physhim);

	wlc_phy_sample_collect_start_nphy(pi, coll_us, &crsctl, &crsctlu, &crsctll);

	/* RUN it */

	/* The RxMacifMode phyreg is passed to ucode to start the collect */
	mask = NPHY_RxMacifMode_SampleCore_MASK	| NPHY_RxMacifMode_PassThrough_MASK;

	/* Clear the macintstatus bit that indicates ucode timeout */
	W_REG(osh, &pi->regs->macintstatus, (1 << 24));

	for (core = start_core; core <= end_core; ptr += 1 + (ltoh16(ptr[0]) >> 1), core++) {
		val = (core << NPHY_RxMacifMode_SampleCore_SHIFT) |
			(coll_type << NPHY_RxMacifMode_PassThrough_SHIFT);
		val |= (phy_reg_read(pi, NPHY_RxMacifMode) & ~mask);

		/* Use SHM 0 for phyreg value (will be overwritten by the samples */
		wlapi_bmac_write_shm(pi->sh->physhim, 0, val);

		/* Give the command and wait */
		OR_REG(osh, &pi->regs->maccommand, MCMD_SAMPLECOLL);
		SPINWAIT(((R_REG(osh, &pi->regs->maccommand) & MCMD_SAMPLECOLL) != 0), 50000);

		if ((R_REG(osh, &pi->regs->maccommand) & MCMD_SAMPLECOLL) != 0) {
			PHY_ERROR(("wl%d: %s: Failed to finish sample collect\n",
				pi->sh->unit, __FUNCTION__));
			return BCME_ERROR;
		}

		/* Verify that the sample collect didn't timeout in the ucode */
		if (R_REG(osh, &pi->regs->macintstatus) & (1 << 24)) {
			PHY_ERROR(("wl%d: %s: Sample Collect failed after 10 attempts\n",
				pi->sh->unit, __FUNCTION__));
			return BCME_ERROR;
		}

		/* RXE_RXCNT is stored in S_RSV1 */
		W_REG(osh, &pi->regs->objaddr, OBJADDR_SCR_SEL + S_RSV1);
		ptr[0] = R_REG(osh, &pi->regs->objdata) & ~0x1;
		/* Hardcode the peak-rxpower for the specific rx-gain */
		ptr[1] = htol16(0xB8B8);	/* 2's complement -72dBm */
		wlapi_copyfrom_objmem(pi->sh->physhim, 0, &ptr[2], ptr[0], OBJADDR_SHM_SEL);

		ptr[0] += 2;
		ptr[0] = htol16(ptr[0]);
	}

	wlc_phy_sample_collect_end_nphy(pi, crsctl, crsctlu, crsctll);

	return BCME_OK;
}
#endif	/* SAMPLE_COLLECT */


#ifdef WLTEST
void
wlc_phy_boardflag_upd(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;

	if ((ISNPHY(pi)) && NREV_GE(pi->pubpi.phy_rev, 3)) {
		/* Check if A-band spur WAR should be enabled for this board */
		if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_SPUR_WAR) {
			PHY_ERROR(("%s: aband_spurwar on\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_aband_spurwar_en = TRUE;
		} else {
			PHY_ERROR(("%s: aband_spurwar off\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_aband_spurwar_en = FALSE;
		}
	}

	if ((ISNPHY(pi)) && NREV_GE(pi->pubpi.phy_rev, 6)) {
		/* Check if extra G-band spur WAR for 40 MHz channels 3 through 10
		 * should be enabled for this board
		 */
		if (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_2G_SPUR_WAR) {
			PHY_ERROR(("%s: gband_spurwar2 on\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_gband_spurwar2_en = TRUE;
		} else {
			PHY_ERROR(("%s: gband_spurwar2 off\n", __FUNCTION__));
			pi->u.pi_nphy->nphy_gband_spurwar2_en = FALSE;
		}
	}

	pi->nphy_txpwrctrl = PHY_TPC_HW_OFF;
	pi->txpwrctrl = PHY_TPC_HW_OFF;
	pi->phy_5g_pwrgain = FALSE;

	if (pi->sh->boardvendor == VENDOR_APPLE &&
	    (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))) {

		pi->nphy_txpwrctrl =  PHY_TPC_HW_ON;
		pi->phy_5g_pwrgain = TRUE;

	} else if ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_TXPWRCTRL_EN) &&
		NREV_GE(pi->pubpi.phy_rev, 2) && (pi->sh->sromrev >= 4)) {

		pi->nphy_txpwrctrl = PHY_TPC_HW_ON;

	} else if ((pi->sh->sromrev >= 4) &&
		(BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_5G_PWRGAIN)) {
		pi->phy_5g_pwrgain = TRUE;
	}
}
#endif /* WLTEST */
#if defined(WLTEST)
/*  FA009736 - PD Test Failure WAR */
void
wlc_phy_resetcntrl_regwrite(wlc_phy_t *pih)
{
#if LCNCONF
	phy_info_t *pi = (phy_info_t *)pih;
	/* enable rfseqSoftReset bit */
	phy_reg_write(pi, LCNPHY_resetCtrl, 0x088);
	OSL_DELAY(5);
	/* disable rfseqSoftReset bit and write default value 0x80  */
	phy_reg_write(pi, LCNPHY_resetCtrl, 0x080);
	OSL_DELAY(5);
	wlc_lcnphy_4313war(pi);
#endif /* LCNCONF */
}

int
wlc_phy_set_po_htphy(phy_info_t *pi, wl_po_t *inpo)
{
	int err = BCME_OK;

	switch (inpo->band) {
		case WL_CHAN_FREQ_RANGE_2G:
			pi->ppr.sr9.cckbw202gpo = inpo->cckpo;
			pi->ppr.sr9.cckbw20ul2gpo = inpo->cckpo;
			pi->ppr.sr9.ofdm2g.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm2g.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm2g.bw40 = 0;
			pi->ppr.sr9.mcs2g.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs2g.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs2g.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;

		case WL_CHAN_FREQ_RANGE_5GL:
			pi->ppr.sr9.ofdm5gl.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gl.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gl.bw40 = 0;
			pi->ppr.sr9.mcs5gl.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs5gl.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs5gl.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			pi->ppr.sr9.ofdm5gm.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gm.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gm.bw40 = 0;
			pi->ppr.sr9.mcs5gm.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs5gm.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs5gm.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
#ifdef	WL_PPR_SUBBAND
		case WL_CHAN_FREQ_RANGE_5GH_5BAND:
#endif	/* WL_PPR_SUBBAND */
			pi->ppr.sr9.ofdm5gh.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gh.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gh.bw40 = 0;
			pi->ppr.sr9.mcs5gh.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs5gh.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs5gh.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;
#ifdef	WL_PPR_SUBBAND
		case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
			pi->ppr.sr9.ofdm5gll.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gll.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gll.bw40 = 0;
			pi->ppr.sr9.mcs5gll.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs5gll.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs5gll.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;
		case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
			pi->ppr.sr9.ofdm5glh.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5glh.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5glh.bw40 = 0;
			pi->ppr.sr9.mcs5glh.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs5glh.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs5glh.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;
		case WL_CHAN_FREQ_RANGE_5GML_5BAND:
			pi->ppr.sr9.ofdm5gml.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gml.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gml.bw40 = 0;
			pi->ppr.sr9.mcs5gml.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs5gml.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs5gml.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;
		case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
			pi->ppr.sr9.ofdm5gmh.bw20 = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gmh.bw20ul = inpo->ofdmpo;
			pi->ppr.sr9.ofdm5gmh.bw40 = 0;
			pi->ppr.sr9.mcs5gmh.bw20 =
				(inpo->mcspo[1] << 16) | inpo->mcspo[0];
			pi->ppr.sr9.mcs5gmh.bw20ul =
				(inpo->mcspo[3] << 16) | inpo->mcspo[2];
			pi->ppr.sr9.mcs5gmh.bw40 =
				(inpo->mcspo[5] << 16) | inpo->mcspo[4];
			break;
#endif	/* WL_PPR_SUBBAND */
		default:
			PHY_ERROR(("bandrange %d is out of scope\n", inpo->band));
			err = BCME_BADARG;
			break;
	}
	if (!err)
		wlc_phy_txpwr_apply(pi);
	return err;
}

static int
wlc_phy_get_pa_phy(phy_info_t *pi, wl_pavars2_t *invar, uint *ptrj, uint16 *outpa)
{
	int err = BCME_OK;
	phy_info_nphy_t *pi_nphy = NULL;
	srom9_pwrdet_t  *pwrdet  = &pi->pwrdet;
	uint b = 0;
	b = *ptrj;

	if (ISNPHY(pi)) {
		pi_nphy = pi->u.pi_nphy;
	} else {
		return BCME_ERROR;
	}

	if (invar->bandrange >= 4 + PHY_SUBBAND_NUM)
		return BCME_ERROR;

	if (invar->chain >= PHY_CORE_MAX)
		return BCME_ERROR;

	outpa[b++] = pwrdet->pwrdet_a1[invar->chain][invar->bandrange];
	outpa[b++] = pwrdet->pwrdet_b0[invar->chain][invar->bandrange];
	outpa[b++] = pwrdet->pwrdet_b1[invar->chain][invar->bandrange];

	*ptrj = b;
	return err;

}

static int
wlc_phy_set_pa_phy(phy_info_t *pi, wl_pavars2_t *invar, uint *ptrj, uint16 *inpa)
{
	int err = BCME_OK;
	phy_info_nphy_t *pi_nphy = NULL;
	srom9_pwrdet_t  *pwrdet  = &pi->pwrdet;
	uint b = 0;
	b = *ptrj;

	if (ISNPHY(pi)) {
		pi_nphy = pi->u.pi_nphy;
	} else {
		return BCME_ERROR;
	}

	if (invar->bandrange >= 4 + PHY_SUBBAND_NUM)
		return BCME_ERROR;

	if (invar->chain >= PHY_CORE_MAX)
		return BCME_ERROR;

	pwrdet->pwrdet_a1[invar->chain][invar->bandrange] = inpa[b++];
	pwrdet->pwrdet_b0[invar->chain][invar->bandrange] = inpa[b++];
	pwrdet->pwrdet_b1[invar->chain][invar->bandrange] = inpa[b++];
	*ptrj = b;
	return err;
}

int
wlc_phy_get_po_htphy(phy_info_t *pi, wl_po_t *outpo)
{
	int err = BCME_OK;

	switch (outpo->band) {
		case WL_CHAN_FREQ_RANGE_2G:
			outpo->cckpo = pi->ppr.sr9.cckbw202gpo;
			outpo->ofdmpo = pi->ppr.sr9.ofdm2g.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs2g.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs2g.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs2g.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs2g.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs2g.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs2g.bw40 >>16);
			break;

		case WL_CHAN_FREQ_RANGE_5GL:
			outpo->ofdmpo = pi->ppr.sr9.ofdm5gl.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs5gl.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs5gl.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs5gl.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs5gl.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs5gl.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs5gl.bw40 >>16);
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			outpo->ofdmpo = pi->ppr.sr9.ofdm5gm.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs5gm.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs5gm.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs5gm.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs5gm.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs5gm.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs5gm.bw40 >>16);
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
#ifdef	WL_PPR_SUBBAND
		case WL_CHAN_FREQ_RANGE_5GH_5BAND:
#endif	/* WL_PPR_SUBBAND */
			outpo->ofdmpo = pi->ppr.sr9.ofdm5gh.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs5gh.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs5gh.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs5gh.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs5gh.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs5gh.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs5gh.bw40 >>16);
			break;
#ifdef	WL_PPR_SUBBAND
		case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
			outpo->ofdmpo = pi->ppr.sr9.ofdm5gll.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs5gll.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs5gll.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs5gll.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs5gll.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs5gll.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs5gll.bw40 >>16);
			break;

		case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
			outpo->ofdmpo = pi->ppr.sr9.ofdm5glh.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs5glh.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs5glh.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs5glh.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs5glh.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs5glh.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs5glh.bw40 >>16);
			break;

		case WL_CHAN_FREQ_RANGE_5GML_5BAND:
			outpo->ofdmpo = pi->ppr.sr9.ofdm5gml.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs5gml.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs5gml.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs5gml.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs5gml.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs5gml.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs5gml.bw40 >>16);
			break;

		case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
			outpo->ofdmpo = pi->ppr.sr9.ofdm5gmh.bw20;
			outpo->mcspo[0] = (uint16)pi->ppr.sr9.mcs5gmh.bw20;
			outpo->mcspo[1] = (uint16)(pi->ppr.sr9.mcs5gmh.bw20 >>16);
			outpo->mcspo[2] = (uint16)pi->ppr.sr9.mcs5gmh.bw20ul;
			outpo->mcspo[3] = (uint16)(pi->ppr.sr9.mcs5gmh.bw20ul >>16);
			outpo->mcspo[4] = (uint16)pi->ppr.sr9.mcs5gmh.bw40;
			outpo->mcspo[5] = (uint16)(pi->ppr.sr9.mcs5gmh.bw40 >>16);
			break;
#endif	/* WL_PPR_SUBBAND */
		default:
			PHY_ERROR(("bandrange %d is out of scope\n", outpo->band));
			err = BCME_BADARG;
			break;
	}

	return err;
}
#endif 
/* %%%%%% dump */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)


/* Below wlc_phydump_xx should be generic, applicable to all PHYs.
 * each PHY section should have ISXPHY macro checking
 */
void
wlc_phydump_measlo(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	int padmix_en = 0;
	int j, z, bb_attn, rf_attn;
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_info_abgphy_t *pi_abg = NULL;
	if (ISABGPHY(pi))
		pi_abg = pi->u.pi_abgphy;

	if (!ISGPHY(pi))
		return;

	if (ISABGPHY(pi)) {
		bcm_bprintf(b, "bb_list_size= %d, rf_list_size= %d\n *RF* ", pi_abg->bb_list_size,
			pi_abg->rf_list_size);

		for (j = 0; j < pi_abg->bb_list_size; j++)
			bcm_bprintf(b, "  %d:i,q  ", pi_abg->bb_attn_list[j]);

	bcm_bprintf(b, "\n");

		for (z = 0; z < pi_abg->rf_list_size; z++) {
			rf_attn   = (int)((char)PHY_GET_RFATTN(pi_abg->rf_attn_list[z]));
			padmix_en = (int)((char)PHY_GET_PADMIX(pi_abg->rf_attn_list[z]));
			bcm_bprintf(b, "%2d %1d", rf_attn, padmix_en);

			for (j = 0; j < pi_abg->bb_list_size; j++) {
				int i, q;
				bb_attn = pi_abg->bb_attn_list[j];
				i = (int)((char)pi_abg->gphy_locomp_iq[PHY_GET_RFGAINID(rf_attn,
					padmix_en, pi_abg->rf_max)][bb_attn].i);
				q = (int)((char)pi_abg->gphy_locomp_iq[PHY_GET_RFGAINID(rf_attn,
					padmix_en, pi_abg->rf_max)][bb_attn].q);
				bcm_bprintf(b, " : %3d %2d", i, q);
			}
			bcm_bprintf(b, "\n");
		}
		bcm_bprintf(b, "\n");
	}
}

void
wlc_phydump_phycal(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	nphy_iq_comp_t rxcal_coeffs;
	int16 txcal_ofdm_coeffs[8];
	int16 txcal_bphy_coeffs[8];
	uint16 radio_regs[8] = { 0 };
	uint16 rccal_val[2] = { 0 };
	uint16 rcal_value = 0;
	int ci, cq;
	int ei, eq, fi, fq;
	int time_elapsed;
	phy_info_nphy_t *pi_nphy = NULL;
	phy_info_t *pi = (phy_info_t *)ppi;

	if (!pi->sh->up)
		return;

	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi) && !(ISHTPHY(pi)))
		return;

	/* for HTPHY, branch out to htphy phycal dump routine */
	if (ISHTPHY(pi)) {
		wlc_phy_cal_dump_htphy(pi, b);
	} else {
		/* for NPHY, carry out the following steps */
		time_elapsed = pi->sh->now - pi->cal_info->last_cal_time;
		if (time_elapsed < 0)
			time_elapsed = 0;

		bcm_bprintf(b, "time since last cal: %d (sec), mphase_cal_id: %d\n\n",
			time_elapsed, pi->cal_info->cal_phase_id);

		ASSERT(pi_nphy != NULL);

		bcm_bprintf(b, "Pre-calibration txpwr indices: %d, %d\n",
			pi_nphy->nphy_cal_orig_pwr_idx[0],
			pi_nphy->nphy_cal_orig_pwr_idx[1]);

		bcm_bprintf(b, "Tx Calibration pwr indices: %d, %d\n",
			pi_nphy->nphy_txcal_pwr_idx[0], pi_nphy->nphy_txcal_pwr_idx[1]);

		if (NREV_GE(pi->pubpi.phy_rev, 7)) {
			bcm_bprintf(b, "Calibration target txgain Core 0: txlpf=%d,"
				" txgm=%d, pga=%d, pad=%d, ipa=%d\n",
				pi_nphy->nphy_cal_target_gain.txlpf[0],
				pi_nphy->nphy_cal_target_gain.txgm[0],
				pi_nphy->nphy_cal_target_gain.pga[0],
				pi_nphy->nphy_cal_target_gain.pad[0],
				pi_nphy->nphy_cal_target_gain.ipa[0]);
		} else {
			bcm_bprintf(b, "Calibration target txgain Core 0: txgm=%d, pga=%d,"
				" pad=%d, ipa=%d\n",
				pi_nphy->nphy_cal_target_gain.txgm[0],
				pi_nphy->nphy_cal_target_gain.pga[0],
				pi_nphy->nphy_cal_target_gain.pad[0],
				pi_nphy->nphy_cal_target_gain.ipa[0]);
		}

		if (NREV_GE(pi->pubpi.phy_rev, 7)) {
			bcm_bprintf(b, "Calibration target txgain Core 1: txlpf=%d,"
				" txgm=%d, pga=%d, pad=%d, ipa=%d\n",
				pi_nphy->nphy_cal_target_gain.txlpf[1],
				pi_nphy->nphy_cal_target_gain.txgm[1],
				pi_nphy->nphy_cal_target_gain.pga[1],
				pi_nphy->nphy_cal_target_gain.pad[1],
				pi_nphy->nphy_cal_target_gain.ipa[1]);
		} else {
			bcm_bprintf(b, "Calibration target txgain Core 1: txgm=%d,"
				" pga=%d, pad=%d, ipa=%d\n",
				pi_nphy->nphy_cal_target_gain.txgm[1],
				pi_nphy->nphy_cal_target_gain.pga[1],
				pi_nphy->nphy_cal_target_gain.pad[1],
				pi_nphy->nphy_cal_target_gain.ipa[1]);
		}

		bcm_bprintf(b, "Calibration RFSEQ table txgains : 0x%x, 0x%x\n\n",
			pi_nphy->nphy_cal_orig_tx_gain[0],
			pi_nphy->nphy_cal_orig_tx_gain[1]);

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phyreg_enter((wlc_phy_t *)pi);

		if ((pi_nphy) && (pi_nphy->phyhang_avoid))
			wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

		/* Read Rx calibration co-efficients */
		wlc_phy_rx_iq_coeffs_nphy(pi, 0, &rxcal_coeffs);

		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			if (NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV+3)) {
			/* FIXME4324 */
			} else {
			radio_regs[0] =	read_radio_reg(pi,
				RADIO_2056_TX_LOFT_FINE_I | RADIO_2056_TX0);
			radio_regs[1] = read_radio_reg(pi,
				RADIO_2056_TX_LOFT_FINE_Q | RADIO_2056_TX0);
			radio_regs[2] = read_radio_reg(pi,
				RADIO_2056_TX_LOFT_FINE_I | RADIO_2056_TX1);
			radio_regs[3] = read_radio_reg(pi,
				RADIO_2056_TX_LOFT_FINE_Q | RADIO_2056_TX1);
			radio_regs[4] = read_radio_reg(pi,
				RADIO_2056_TX_LOFT_COARSE_I | RADIO_2056_TX0);
			radio_regs[5] = read_radio_reg(pi,
				RADIO_2056_TX_LOFT_COARSE_Q | RADIO_2056_TX0);
			radio_regs[6] = read_radio_reg(pi,
				RADIO_2056_TX_LOFT_COARSE_I | RADIO_2056_TX1);
			radio_regs[7] = read_radio_reg(pi,
				RADIO_2056_TX_LOFT_COARSE_Q | RADIO_2056_TX1);
			/* read rccal value */
			rccal_val[0] = read_radio_reg(pi,
				RADIO_2056_RX_RXLPF_RCCAL_LPC | RADIO_2056_RX0);
			rccal_val[1] = read_radio_reg(pi,
				RADIO_2056_RX_RXLPF_RCCAL_LPC | RADIO_2056_RX1);

			if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV)) {
				rccal_val[0] = read_radio_reg(pi, RADIO_2057_RCCAL_BCAP_VAL);
				rccal_val[1] = read_radio_reg(pi, RADIO_2057_RCCAL_SCAP_VAL);
				rcal_value = ((read_radio_reg(pi, RADIO_2057_RCAL_STATUS)) >> 1);
			}
			}
		} else {
			/* Read the LOFT compensation (Ci/Cq) values */
			radio_regs[0] = read_radio_reg(pi, RADIO_2055_CORE1_TX_VOS_CNCL);
			radio_regs[1] = read_radio_reg(pi, RADIO_2055_CORE2_TX_VOS_CNCL);
		}

		/* Read OFDM Tx calibration co-efficients */
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 80, 16, txcal_ofdm_coeffs);

		/* Read BPHY Tx calibration co-efficients */
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 88, 16, txcal_bphy_coeffs);

		if ((pi_nphy) && (pi_nphy->phyhang_avoid))
			wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

		/* reg access is done, enable the mac */
		wlc_phyreg_exit((wlc_phy_t *)pi);
		wlapi_enable_mac(pi->sh->physhim);

		bcm_bprintf(b, "Tx IQ/LO cal coeffs for OFDM PHY:\n");

		txcal_ofdm_coeffs[0] &= 0x3ff;
		txcal_ofdm_coeffs[1] &= 0x3ff;
		txcal_ofdm_coeffs[2] &= 0x3ff;
		txcal_ofdm_coeffs[3] &= 0x3ff;

		if (txcal_ofdm_coeffs[0] > 511)
			txcal_ofdm_coeffs[0] -= 1024;

		if (txcal_ofdm_coeffs[1] > 511)
			txcal_ofdm_coeffs[1] -= 1024;

		if (txcal_ofdm_coeffs[2] > 511)
			txcal_ofdm_coeffs[2] -= 1024;

		if (txcal_ofdm_coeffs[3] > 511)
			txcal_ofdm_coeffs[3] -= 1024;

		bcm_bprintf(b, "A0=%d, B0=%d, A1=%d, B1=%d\n",
			txcal_ofdm_coeffs[0],
			txcal_ofdm_coeffs[1],
			txcal_ofdm_coeffs[2],
			txcal_ofdm_coeffs[3]);

		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			ei = (int)(radio_regs[0] & 0xf) - (int)((radio_regs[0] & 0xf0) >> 4);
			eq = (int)(radio_regs[1] & 0xf) - (int)((radio_regs[1] & 0xf0) >> 4);
			fi = (int)(radio_regs[4] & 0xf) - (int)((radio_regs[4] & 0xf0) >> 4);
			fq = (int)(radio_regs[5] & 0xf) - (int)((radio_regs[5] & 0xf0) >> 4);
			bcm_bprintf(b, "Core 0: LOFT_FINE_I=0x%0x, LOFT_FINE_Q=0x%0x\n",
				radio_regs[0], radio_regs[1]);
			bcm_bprintf(b, "Core 0: LOFT_COARSE_I=0x%0x, LOFT_COARSE_Q=0x%0x\n",
				radio_regs[4], radio_regs[5]);
			bcm_bprintf(b, "Core 0: ei=%d, eq=%d, fi=%d, fq=%d\n",
				ei, eq, fi, fq);

			ei = (int)(radio_regs[2] & 0xf) - (int)((radio_regs[2] & 0xf0) >> 4);
			eq = (int)(radio_regs[3] & 0xf) - (int)((radio_regs[3] & 0xf0) >> 4);
			fi = (int)(radio_regs[6] & 0xf) - (int)((radio_regs[6] & 0xf0) >> 4);
			fq = (int)(radio_regs[7] & 0xf) - (int)((radio_regs[7] & 0xf0) >> 4);
			bcm_bprintf(b, "Core 1: LOFT_FINE_I=0x%0x, LOFT_FINE_Q=0x%0x\n",
				radio_regs[2], radio_regs[3]);
			bcm_bprintf(b, "Core 1: LOFT_COARSE_I=0x%0x, LOFT_COARSE_Q=0x%0x\n",
				radio_regs[6], radio_regs[7]);
			bcm_bprintf(b, "Core 1: ei=%d, eq=%d, fi=%d, fq=%d\n",
				ei, eq, fi, fq);
		} else {
			if (radio_regs[0] & 0xC0)
				ci = (radio_regs[0] & 0xC0) >> 6;
			else
				ci = ((radio_regs[0] & 0x30) >> 4) * -1;

			if (radio_regs[0] & 0x0C)
				cq = (radio_regs[0] & 0x0C) >> 3;
			else
				cq = (radio_regs[0] & 0x03) * -1;

			bcm_bprintf(b, "CORE1_VOS_CNCL=0x%0x, Ci0=%d, Cq0=%d\n",
				radio_regs[0], ci, cq);

			if (radio_regs[1] & 0xC0)
				ci = (radio_regs[1] & 0xC0) >> 6;
			else
				ci = ((radio_regs[1] & 0x30) >> 4) * -1;

			if (radio_regs[1] & 0x0C)
				cq = (radio_regs[1] & 0x0C) >> 3;
			else
				cq = (radio_regs[1] & 0x03) * -1;

			bcm_bprintf(b, "CORE2_VOS_CNCL=0x%0x, Ci1=%d, Cq1=%d\n",
				radio_regs[1], ci, cq);
		}

		bcm_bprintf(b, "Di0=%d, Dq0=%d, Di1=%d, Dq1=%d, m0=%d, m1=%d\n\n\n",
			(int8) ((txcal_ofdm_coeffs[5] & 0xFF00) >> 8),
			(int8) (txcal_ofdm_coeffs[5] & 0x00FF),
			(int8) ((txcal_ofdm_coeffs[6] & 0xFF00) >> 8),
			(int8) (txcal_ofdm_coeffs[6] & 0x00FF),
			(int8) ((txcal_ofdm_coeffs[7] & 0xFF00) >> 8),
			(int8) (txcal_ofdm_coeffs[7] & 0x00FF));

		bcm_bprintf(b, "Tx IQ/LO cal coeffs for BPHY:\n");

		txcal_bphy_coeffs[0] &= 0x3ff;
		txcal_bphy_coeffs[1] &= 0x3ff;
		txcal_bphy_coeffs[2] &= 0x3ff;
		txcal_bphy_coeffs[3] &= 0x3ff;

		if (txcal_bphy_coeffs[0] > 511)
			txcal_bphy_coeffs[0] -= 1024;

		if (txcal_bphy_coeffs[1] > 511)
			txcal_bphy_coeffs[1] -= 1024;

		if (txcal_bphy_coeffs[2] > 511)
			txcal_bphy_coeffs[2] -= 1024;

		if (txcal_bphy_coeffs[3] > 511)
			txcal_bphy_coeffs[3] -= 1024;

		bcm_bprintf(b, "A0=%d, B0=%d, A1=%d, B1=%d\n",
			txcal_bphy_coeffs[0],
			txcal_bphy_coeffs[1],
			txcal_bphy_coeffs[2],
			txcal_bphy_coeffs[3]);

		bcm_bprintf(b, "Di0=%d, Dq0=%d, Di1=%d, Dq1=%d, m0=%d, m1=%d\n\n\n",
			(int8) ((txcal_bphy_coeffs[5] & 0xFF00) >> 8),
			(int8) (txcal_bphy_coeffs[5] & 0x00FF),
			(int8) ((txcal_bphy_coeffs[6] & 0xFF00) >> 8),
			(int8) (txcal_bphy_coeffs[6] & 0x00FF),
			(int8) ((txcal_bphy_coeffs[7] & 0xFF00) >> 8),
			(int8) (txcal_bphy_coeffs[7] & 0x00FF));

		bcm_bprintf(b, "Rx IQ/LO cal coeffs:\n");

		/* Rx calibration coefficients are 10-bit signed integers */
		if (rxcal_coeffs.a0 > 511)
			rxcal_coeffs.a0 -= 1024;

		if (rxcal_coeffs.b0 > 511)
			rxcal_coeffs.b0 -= 1024;

		if (rxcal_coeffs.a1 > 511)
			rxcal_coeffs.a1 -= 1024;

		if (rxcal_coeffs.b1 > 511)
			rxcal_coeffs.b1 -= 1024;

		bcm_bprintf(b, "a0=%d, b0=%d, a1=%d, b1=%d\n\n",
			rxcal_coeffs.a0, rxcal_coeffs.b0, rxcal_coeffs.a1, rxcal_coeffs.b1);

		if (NREV_GE(pi->pubpi.phy_rev, 3)) {
			bcm_bprintf(b, "RC CAL value: %d, %d\n", rccal_val[0], rccal_val[1]);
		}
		if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV)) {
			bcm_bprintf(b, "\nR CAL value: %d, \n", rcal_value);
		}
	}

#if defined(WLMCHAN) && defined(BCMDBG)
	{
		if (!ISNPHY(pi) && !ISHTPHY(pi))
			return;

		wlc_phydump_chanctx(pi, b);
	}
#endif

}


/* Dump rssi values from aci scans */
void
wlc_phydump_aci(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	int channel, indx;
	int val;
	char *ptr;
	phy_info_t *pi = (phy_info_t *)ppi;

	val = pi->sh->interference_mode;
	if (pi->aci_state & ACI_ACTIVE)
		val |= AUTO_ACTIVE;

	if (val & AUTO_ACTIVE)
		bcm_bprintf(b, "ACI is Active\n");
	else {
		bcm_bprintf(b, "ACI is Inactive\n");
		return;
	}

	for (channel = 0; channel < ACI_LAST_CHAN; channel++) {
		bcm_bprintf(b, "Channel %d : ", channel + 1);
		for (indx = 0; indx < 50; indx++) {
			ptr = &(pi->interf.aci.rssi_buf[channel][indx]);
			if (*ptr == (char)-1)
				break;
			bcm_bprintf(b, "%d ", *ptr);
		}
		bcm_bprintf(b, "\n");
	}
}

void
wlc_phydump_papd(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	uint32 val, i, j;
	int32 eps_real, eps_imag;
	phy_info_t *pi = (phy_info_t *)ppi;

	eps_real = eps_imag = 0;

	if (!pi->sh->up)
		return;

	if (!(ISNPHY(pi) || ISLCNCOMMONPHY(pi)))
		return;


	if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12)) {
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK,  MCTL_PHYLOCK);
		(void)R_REG(pi->sh->osh, &pi->regs->maccontrol);
		OSL_DELAY(1);
	}



	if (ISNPHY(pi))
	{
		bcm_bprintf(b, "papd eps table:\n [core 0]\t\t[core 1] \n");
		for (j = 0; j < 64; j++) {
			for (i = 0; i < 2; i++) {
				wlc_phy_table_read_nphy(pi, ((i == 0) ? NPHY_TBL_ID_EPSILONTBL0 :
					NPHY_TBL_ID_EPSILONTBL1), 1, j, 32, &val);
				wlc_phy_papd_decode_epsilon(val, &eps_real, &eps_imag);
				bcm_bprintf(b, "{%d\t%d}\t\t", eps_real, eps_imag);
			}
		bcm_bprintf(b, "\n");
		}
		bcm_bprintf(b, "\n\n");
	}
	else if (ISLCNPHY(pi))
	{
		wlc_lcnphy_read_papdepstbl(pi, b);

	} else if (ISLCN40PHY(pi))
		wlc_lcn40phy_read_papdepstbl(pi, b);


	if (D11REV_IS(pi->sh->corerev, 11) || D11REV_IS(pi->sh->corerev, 12))
		wlapi_bmac_mctrl(pi->sh->physhim, MCTL_PHYLOCK, 0);
}

void
wlc_phydump_noise(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	uint32 i, idx, antidx;
	int32 tot;
	phy_info_t *pi = (phy_info_t *)ppi;

	if (!pi->sh->up)
		return;

	bcm_bprintf(b, "History and average of latest %d noise values:\n",
		PHY_NOISE_WINDOW_SZ);

	FOREACH_CORE(pi, antidx) {
		tot = 0;
		bcm_bprintf(b, "Ant%d: [", antidx);

		idx = pi->phy_noise_index;
		for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
			bcm_bprintf(b, "%4d", pi->phy_noise_win[antidx][idx]);
			tot += pi->phy_noise_win[antidx][idx];
			idx = MODINC_POW2(idx, PHY_NOISE_WINDOW_SZ);
		}
		bcm_bprintf(b, "]");

		tot /= PHY_NOISE_WINDOW_SZ;
		bcm_bprintf(b, " [%4d]\n", tot);
	}
}

void
wlc_phydump_state(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	char fraction[4][4] = {"  ", ".25", ".5", ".75"};
	int i;
	phy_info_t *pi = (phy_info_t *)ppi;
	bool ishtphy = ISHTPHY(pi);
	uint offset;

#define QDB_FRAC(x)	(x) / WLC_TXPWR_DB_FACTOR, fraction[(x) % WLC_TXPWR_DB_FACTOR]

	bcm_bprintf(b, "phy_type %d phy_rev %d ana_rev %d radioid 0x%x radiorev 0x%x\n",
	               pi->pubpi.phy_type, pi->pubpi.phy_rev, pi->pubpi.ana_rev,
	               pi->pubpi.radioid, pi->pubpi.radiorev);

	bcm_bprintf(b, "hw_power_control %d encore %d\n",
	               pi->hwpwrctrl, pi->pubpi.abgphy_encore);

	bcm_bprintf(b, "Power targets: ");
	/* CCK Power/Rate */
	bcm_bprintf(b, "\n\tCCK: ");
	for (i = 0; i < WL_NUM_RATES_CCK; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_CCK]));
	/* OFDM Power/Rate */
	bcm_bprintf(b, "\n\tOFDM 20MHz SISO: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM]));
	bcm_bprintf(b, "\n\tOFDM 20MHz CDD: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM_20_CDD]));
	/* 20MHz MCS Power/Rate */
	bcm_bprintf(b, ishtphy ? "\n\tMCS0-7 20MHz 1 Tx: " : "\n\tMCS0-7 20MHz SISO: ");
	offset = ishtphy ? TXP_FIRST_MCS_20_S1x1 : TXP_FIRST_MCS_20_SISO;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS0-7 20MHz 2 Tx: " : "\n\tMCS0-7 20MHz CDD: ");
	offset = ishtphy ? TXP_FIRST_MCS_20_S1x2 : TXP_FIRST_MCS_20_CDD;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS0-7 20MHz 3 Tx: " : "\n\tMCS0-7 20MHz STBC: ");
	offset = ishtphy ? TXP_FIRST_MCS_20_S1x3 : TXP_FIRST_MCS_20_STBC;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS8-15 20MHz 2 Tx: " : "\n\tMCS0-7 20MHz SDM: ");
	offset = ishtphy ? TXP_FIRST_MCS_20_S2x2 : TXP_FIRST_MCS_20_SDM;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS8-15 20MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && ishtphy; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_20_S2x3]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS16-23 20MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && ishtphy; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_20_S3x3]));

	/* 40MHz OFDM Power/Rate */
	bcm_bprintf(b, "\n\tOFDM 40MHz SISO: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM_40_SISO]));
	bcm_bprintf(b, "\n\tOFDM 40MHz CDD: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_OFDM_40_CDD]));
	/* 40MHz MCS Power/Rate */
	bcm_bprintf(b, ishtphy ? "\n\tMCS0-7 40MHz 1 Tx: " : "\n\tMCS0-7 40MHz SISO: ");
	offset = ishtphy ? TXP_FIRST_MCS_40_S1x1 : TXP_FIRST_MCS_40_SISO;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS0-7 40MHz 2 Tx: " : "\n\tMCS0-7 40MHz CDD: ");
	offset = ishtphy ? TXP_FIRST_MCS_40_S1x2 : TXP_FIRST_MCS_40_CDD;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS0-7 40MHz 3 Tx: " : "\n\tMCS0-7 40MHz STBC: ");
	offset = ishtphy ? TXP_FIRST_MCS_40_S1x3 : TXP_FIRST_MCS_40_STBC;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+offset]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS8-15 40MHz 2 Tx: " : "\n\tMCS0-7 40MHz SDM: ");
	offset = ishtphy ? TXP_FIRST_MCS_40_S2x2 : TXP_FIRST_MCS_40_SDM;
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS8-15 40MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && ishtphy; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_40_S2x3]));
	bcm_bprintf(b, ishtphy ? "\n\tMCS16-23 40MHz 3 Tx: " : "");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && ishtphy; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_MCS_40_S3x3]));

	/* MCS 32 Power */
	bcm_bprintf(b, "\n\tMCS32: %2d%s\n\n", QDB_FRAC(pi->tx_power_target[i]));

	if (ishtphy)
		goto next;

	/* CCK Power/Rate */
#if HTCONF
	bcm_bprintf(b, "\n\tCCK 20UL: ");
	for (i = 0; i < WL_NUM_RATES_CCK; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_CCK]));

	/* 20 in 40MHz OFDM Power/Rate */
	bcm_bprintf(b, "\n\tOFDM 20UL SISO: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_OFDM]));
	bcm_bprintf(b, "\n\tOFDM 40MHz CDD: ");
	for (i = 0; i < WL_NUM_RATES_OFDM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_OFDM_CDD]));

	/* 20 in 40MHz MCS Power/Rate */
	bcm_bprintf(b, "\n\tMCS0-7 20UL 1 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_S1x1]));
	bcm_bprintf(b, "\n\tMCS0-7 20UL 2 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_S1x2]));
	bcm_bprintf(b, "\n\tMCS0-7 20UL 3 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_FIRST_20UL_S1x3]));
	bcm_bprintf(b, "\n\tMCS8-15 20UL 2 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_LAST_20UL_S2x2]));
	bcm_bprintf(b, "\n\tMCS8-15 20UL 3 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && ishtphy; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_LAST_20UL_S2x3]));
	bcm_bprintf(b, "\n\tMCS16-23 20UL 3 Tx: ");
	for (i = 0; i < WL_NUM_RATES_MCS_1STREAM && ishtphy; i++)
		bcm_bprintf(b, "%2d%s ", QDB_FRAC(pi->tx_power_target[i+TXP_LAST_20UL_S3x3]));
#endif /* HTCONF */

next:
	if (ISNPHY(pi)) {
		bcm_bprintf(b, "antsel_type %d\n", pi->antsel_type);
		bcm_bprintf(b, "ipa2g %d ipa5g %d\n", pi->ipa2g_on, pi->ipa5g_on);

	} else if (ISLPPHY(pi) || ISSSLPNPHY(pi) || ISLCNPHY(pi) || ISLCN40PHY(pi) || ISHTPHY(pi)) {
		return;
	} else if (ISAPHY(pi)) {
		bcm_bprintf(b, "itssi low 0x%x, itssi mid 0x%x, itssi high 0x%x\n",
			pi->u.pi_abgphy->idle_tssi[A_LOW_CHANS],
			pi->u.pi_abgphy->idle_tssi[A_MID_CHANS],
			pi->u.pi_abgphy->idle_tssi[A_HIGH_CHANS]);

		bcm_bprintf(b, "Power limits low: ");
		for (i = 0; i <= TXP_LAST_OFDM; i++)
			bcm_bprintf(b, "%d ", pi->tx_srom_max_rate_5g_low[i]);

		bcm_bprintf(b, "\nPower limits mid: ");
		for (i = 0; i <= TXP_LAST_OFDM; i++)
			bcm_bprintf(b, "%d ", pi->tx_srom_max_rate_5g_mid[i]);

		bcm_bprintf(b, "\nPower limits hi: ");
		for (i = 0; i <= TXP_LAST_OFDM; i++)
			bcm_bprintf(b, "%d ", pi->tx_srom_max_rate_5g_hi[i]);
	} else { /* G PHY */
		bcm_bprintf(b, "itssi 0x%x\n", pi->u.pi_abgphy->idle_tssi[G_ALL_CHANS]);
		bcm_bprintf(b, "Power limits: ");
		for (i = 0; i <= TXP_LAST_OFDM; i++)
			bcm_bprintf(b, "%d ", pi->tx_srom_max_rate_2g[i]);
	}

	bcm_bprintf(b, "\ninterference_mode %d intf_crs %d\n",
		pi->sh->interference_mode, pi->interference_mode_crs);

}

void
wlc_phydump_lnagain(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	int core;
	uint16 lnagains[2][4];
	uint16 mingain[2];
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi))
		return;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

	/* Now, read the gain table */
	for (core = 0; core < 2; core++) {
		wlc_phy_table_read_nphy(pi, core, 4, 8, 16, &lnagains[core][0]);
	}

	mingain[0] =
		(phy_reg_read(pi, NPHY_Core1MinMaxGain) &
		NPHY_CoreMinMaxGain_minGainValue_MASK) >>
		NPHY_CoreMinMaxGain_minGainValue_SHIFT;
	mingain[1] =
		(phy_reg_read(pi, NPHY_Core2MinMaxGain) &
		NPHY_CoreMinMaxGain_minGainValue_MASK) >>
		NPHY_CoreMinMaxGain_minGainValue_SHIFT;

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);

	bcm_bprintf(b, "Core 0: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
		lnagains[0][0], lnagains[0][1], lnagains[0][2], lnagains[0][3]);
	bcm_bprintf(b, "Core 1: 0x%02x, 0x%02x, 0x%02x, 0x%02x\n\n",
		lnagains[1][0], lnagains[1][1], lnagains[1][2], lnagains[1][3]);
	bcm_bprintf(b, "Min Gain: Core 0=0x%02x,   Core 1=0x%02x\n\n",
		mingain[0], mingain[1]);
}

void
wlc_phydump_initgain(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	uint8 ctr;
	uint16 regval[2], tblregval[4];
	uint16 lna_gain[2], hpvga1_gain[2], hpvga2_gain[2];
	uint16 tbl_lna_gain[4], tbl_hpvga1_gain[4], tbl_hpvga2_gain[4];
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi))
		return;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

	regval[0] = phy_reg_read(pi, NPHY_Core1InitGainCode);
	regval[1] = phy_reg_read(pi, NPHY_Core2InitGainCode);

	wlc_phy_table_read_nphy(pi, 7, pi->pubpi.phy_corenum, 0x106, 16, tblregval);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);

	lna_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initLnaIndex_MASK) >>
		NPHY_CoreInitGainCode_initLnaIndex_SHIFT;
	hpvga1_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initHpvga1Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga1Index_SHIFT;
	hpvga2_gain[0] = (regval[0] & NPHY_CoreInitGainCode_initHpvga2Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga2Index_SHIFT;


	lna_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initLnaIndex_MASK) >>
		NPHY_CoreInitGainCode_initLnaIndex_SHIFT;
	hpvga1_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initHpvga1Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga1Index_SHIFT;
	hpvga2_gain[1] = (regval[1] & NPHY_CoreInitGainCode_initHpvga2Index_MASK) >>
		NPHY_CoreInitGainCode_initHpvga2Index_SHIFT;

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_lna_gain[ctr] = (tblregval[ctr] >> 2) & 0x3;
	}

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_hpvga1_gain[ctr] = (tblregval[ctr] >> 4) & 0xf;
	}

	for (ctr = 0; ctr < 4; ctr++) {
		tbl_hpvga2_gain[ctr] = (tblregval[ctr] >> 8) & 0x1f;
	}

	bcm_bprintf(b, "Core 0 INIT gain: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
		hpvga2_gain[0], hpvga1_gain[0], lna_gain[0]);
	bcm_bprintf(b, "Core 1 INIT gain: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
		hpvga2_gain[1], hpvga1_gain[1], lna_gain[1]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "INIT gain table:\n");
	bcm_bprintf(b, "----------------\n");
	for (ctr = 0; ctr < 4; ctr++) {
		bcm_bprintf(b, "Core %d: HPVGA2=%d, HPVGA1=%d, LNA=%d\n",
			ctr, tbl_hpvga2_gain[ctr], tbl_hpvga1_gain[ctr], tbl_lna_gain[ctr]);
	}

}

void
wlc_phydump_hpf1tbl(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	uint8 ctr, core;
	uint16 gain[2][NPHY_MAX_HPVGA1_INDEX+1];
	uint16 gainbits[2][NPHY_MAX_HPVGA1_INDEX+1];
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_info_nphy_t *pi_nphy = NULL;

	if (pi->u.pi_nphy)
		pi_nphy = pi->u.pi_nphy;

	if (!ISNPHY(pi))
		return;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, TRUE);

	/* Read from the HPVGA1 gaintable */
	wlc_phy_table_read_nphy(pi, 0, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gain[0][0]);
	wlc_phy_table_read_nphy(pi, 1, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gain[1][0]);
	wlc_phy_table_read_nphy(pi, 2, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gainbits[0][0]);
	wlc_phy_table_read_nphy(pi, 3, NPHY_MAX_HPVGA1_INDEX, 16, 16, &gainbits[1][0]);

	if ((pi_nphy) && (pi_nphy->phyhang_avoid))
		wlc_phy_stay_in_carriersearch_nphy(pi, FALSE);

	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);

	for (core = 0; core < 2; core++) {
		bcm_bprintf(b, "Core %d gain: ", core);
		for (ctr = 0; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
			bcm_bprintf(b, "%2d ", gain[core][ctr]);
		}
		bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, "\n");
	for (core = 0; core < 2; core++) {
		bcm_bprintf(b, "Core %d gainbits: ", core);
		for (ctr = 0; ctr <= NPHY_MAX_HPVGA1_INDEX; ctr++)  {
			bcm_bprintf(b, "%2d ", gainbits[core][ctr]);
		}
		bcm_bprintf(b, "\n");
	}
}

void
wlc_phydump_lpphytbl0(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	uint16 val16;
	lpphytbl_info_t tab;
	int i;
	phy_info_t *pi = (phy_info_t *)ppi;

	if (!ISLPPHY(pi))
		return;

	for (i = 0; i < 108; i ++) {
		tab.tbl_ptr = &val16;	/* ptr to buf */
		tab.tbl_len = 1;	/* # values */
		tab.tbl_id = 0;		/* iqloCaltbl */
		tab.tbl_offset = i;	/* tbl offset */
		tab.tbl_width = 16;	/* 16 bit wide */
		tab.tbl_phywidth = 16; /* width of table element in phy address space */
		wlc_phy_table_read_lpphy(pi, &tab);
		bcm_bprintf(b, "%i:\t%04x\n", i, val16);
	}
}

void
wlc_phydump_chanest(wlc_phy_t *ppi, struct bcmstrbuf *b)
{
	uint8 num_rx, num_sts, num_tones;
	uint32 ch;
	uint16 ch_re_ma, ch_im_ma;
	uint8 ch_re_si, ch_im_si;
	int16 ch_re, ch_im;
	int8 ch_exp;
	uint8 k, r, t, fftk;

	phy_info_t *pi = (phy_info_t *)ppi;

	if (!ISHTPHY(pi))
		return;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);

	/* Go deaf to prevent PHY channel writes while doing reads */
	wlc_phy_stay_in_carriersearch_htphy(pi, TRUE);

	num_rx = (uint8)pi->pubpi.phy_corenum;
	num_sts = 4;
	num_tones = (CHSPEC_IS40(pi->radio_chanspec) ? 128 : 64);

	for (k = 0; k < num_tones; k++) {
		for (r = 0; r < num_rx; r++) {
			for (t = 0; t < num_sts; t++) {
				wlc_phy_table_read_htphy(pi, HTPHY_TBL_ID_CHANEST(r), 1,
				                         t*128 + k, 32, &ch);
				ch_re_ma  = ((ch >> 18) & 0x7ff);
				ch_re_si  = ((ch >> 29) & 0x001);
				ch_im_ma  = ((ch >>  6) & 0x7ff);
				ch_im_si  = ((ch >> 17) & 0x001);
				ch_exp    = ((int8)((ch << 2) & 0xfc)) >> 2;

				ch_re = (ch_re_si > 0) ? -ch_re_ma : ch_re_ma;
				ch_im = (ch_im_si > 0) ? -ch_im_ma : ch_im_ma;

				fftk = ((k < num_tones/2) ? (k + num_tones/2) : (k - num_tones/2));

				bcm_bprintf(b, "chan(%d, %d, %d) = %d*2^%d + i*%d*2^%d;\n",
				            r+1, t+1, fftk+1, ch_re, ch_exp, ch_im, ch_exp);
			}
		}
	}

	/* Return from deaf */
	wlc_phy_stay_in_carriersearch_htphy(pi, FALSE);

	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* BCMDBG || BCMDBG_DUMP */

const uint8 *
wlc_phy_get_ofdm_rate_lookup(void)
{
	return ofdm_rate_lookup;
}

/* LPCONF || SSLPNCONF */
void
wlc_phy_radio_2063_vco_cal(phy_info_t *pi)
{
	uint8 calnrst;

	/* Power up VCO cal clock */
	mod_radio_reg(pi, RADIO_2063_PLL_SP_1, 1 << 6, 0);

	calnrst = read_radio_reg(pi, RADIO_2063_PLL_JTAG_CALNRST) & 0xf8;
	write_radio_reg(pi, RADIO_2063_PLL_JTAG_CALNRST, calnrst);
	OSL_DELAY(1);
	write_radio_reg(pi, RADIO_2063_PLL_JTAG_CALNRST, calnrst | 0x04);
	OSL_DELAY(1);
	write_radio_reg(pi, RADIO_2063_PLL_JTAG_CALNRST, calnrst | 0x06);
	OSL_DELAY(1);
	write_radio_reg(pi, RADIO_2063_PLL_JTAG_CALNRST, calnrst | 0x07);
	OSL_DELAY(300);

	/* Power down VCO cal clock */
	mod_radio_reg(pi, RADIO_2063_PLL_SP_1, 1 << 6, 1 << 6);
}
void
wlc_lcnphy_epa_switch(phy_info_t *pi, bool mode)
{
	if ((CHIPID(pi->sh->chip) == BCM4313_CHIP_ID) &&
		(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM)) {
		if (mode) {
			uint16 txant = 0;
			txant = wlapi_bmac_get_txant(pi->sh->physhim);
			if (txant == 1) {
				PHY_REG_MOD(pi, LCNPHY, RFOverrideVal0, ant_selp_ovr_val, 1);
				PHY_REG_MOD(pi, LCNPHY, RFOverride0, ant_selp_ovr, 1);
			}
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpiocontrol),
				~0x0, 0x0);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioout),
				0x40, 0x40);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioouten),
				0x40, 0x40);
		} else {
			PHY_REG_MOD(pi, LCNPHY, RFOverride0, ant_selp_ovr, 0);
			PHY_REG_MOD(pi, LCNPHY, RFOverrideVal0, ant_selp_ovr_val, 0);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioout),
				0x40, 0x00);
			si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, gpioouten),
				0x40, 0x0);
			si_corereg(pi->sh->sih, SI_CC_IDX,
				OFFSETOF(chipcregs_t, gpiocontrol), ~0x0, 0x40);
		}
	}
}

static int8
wlc_user_txpwr_antport_to_rfport(phy_info_t *pi, uint chan, uint32 band, uint8 rate)
{
	int8 offset = 0;

	if (!pi->user_txpwr_at_rfport)
		return offset;
#ifdef WLNOKIA_NVMEM
	offset += wlc_phy_noknvmem_antport_to_rfport_offset(pi, chan, band, rate);
#endif 
	return offset;
}

/* vbat measurement hook. VBAT is in volts Q4.4 */
static int8
wlc_phy_env_measure_vbat(phy_info_t *pi)
{
	if (ISLCNPHY(pi)) {
		if (CHIPID(pi->sh->chip) == BCM4313_CHIP_ID)
			return wlc_lcnphy_vbatsense(pi, 0);
		else
		return wlc_lcnphy_vbatsense(pi, 1);
	} else {
		return 0;
}
}

/* temperature measurement hook */
static int8
wlc_phy_env_measure_temperature(phy_info_t *pi)
{
	if (ISLCNPHY(pi))
		return wlc_lcnphy_tempsense_degree(pi, 1);
	else
		return 0;
}

bool
wlc_phy_get_tempsense_degree(wlc_phy_t *ppi, int8 *pval)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISLCNPHY(pi))
		*pval = wlc_lcnphy_tempsense_degree(pi, 1);
	else
		return FALSE;
	return TRUE;
}


/* get the per rate limits based on vbat and temp */
static void
wlc_phy_upd_env_txpwr_rate_limits(phy_info_t *pi, uint32 band)
{
	uint8 i;
	int8 temp, vbat;

	for (i = 0; i < TXP_NUM_RATES; i++)
		pi->txpwr_env_limit[i] = WLC_TXPWR_MAX;

	vbat = wlc_phy_env_measure_vbat(pi);
	temp = wlc_phy_env_measure_temperature(pi);

#ifdef WLNOKIA_NVMEM
	/* check the nvram settings and see if there is support for vbat/temp based rate limits */
	wlc_phy_noknvmem_env_txpwrlimit_upd(pi, vbat, temp, band);
#endif 
}

void
wlc_phy_ldpc_override_set(wlc_phy_t *ppi, bool ldpc)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISHTPHY(pi))
		wlc_phy_update_rxldpc_htphy(pi, ldpc);
	return;
}

void
wlc_phy_tkip_rifs_war(wlc_phy_t *ppi, uint8 rifs)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	if (ISNPHY(pi)) {
		wlc_phy_nphy_tkip_rifs_war(pi, rifs);
	}
}

void
wlc_phy_get_pwrdet_offsets(phy_info_t *pi, int8 *cckoffset, int8 *ofdmoffset)
{
	*cckoffset = 0;
	*ofdmoffset = 0;
#ifdef WLNOKIA_NVMEM
	if (ISLCNPHY(pi))
		wlc_phy_noknvmem_get_pwrdet_offsets(pi, cckoffset, ofdmoffset);
#endif /* WLNOKIA_NVMEM */
}

uint32
wlc_phy_qdiv_roundup(uint32 dividend, uint32 divisor, uint8 precision)
{
	uint32 quotient, remainder, roundup, rbit;

	ASSERT(divisor);

	quotient = dividend / divisor;
	remainder = dividend % divisor;
	rbit = divisor & 1;
	roundup = (divisor >> 1) + rbit;

	while (precision--) {
		quotient <<= 1;
		if (remainder >= roundup) {
			quotient++;
			remainder = ((remainder - roundup) << 1) + rbit;
		} else {
			remainder <<= 1;
		}
	}

	/* Final rounding */
	if (remainder >= roundup)
		quotient++;

	return quotient;
}
/* update the cck power detector offset */
int8
wlc_phy_upd_rssi_offset(phy_info_t *pi, int8 rssi, chanspec_t chanspec)
{

#ifdef WLNOKIA_NVMEM
	if (rssi != WLC_RSSI_INVALID)
		rssi = wlc_phy_noknvmem_modify_rssi(pi, rssi, chanspec);
#endif 
	return rssi;
}

bool
wlc_phy_txpower_ipa_ison(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISNPHY(pi))
		return (wlc_phy_n_txpower_ipa_ison(pi));
	else
		return 0;
}

#if defined(WLMCHAN) || defined(PHYCAL_CACHING)
int
wlc_phy_reinit_chanctx(phy_info_t *pi, ch_calcache_t *ctx, chanspec_t chanspec)
{
	ctx->valid = FALSE;
	ctx->chanspec = chanspec;
	ctx->creation_time = pi->sh->now;
#ifndef PHYCAL_CACHE_SMALL
	if (ISLPPHY(pi))
		ctx->u.lpphy_cache.txiqlocal_bestcoeffs_valid = FALSE;
#endif /* PHYCAL_CACHE_SMALL */
	ctx->cal_info.last_cal_time = 0;
	ctx->cal_info.last_papd_cal_time = 0;
	ctx->cal_info.last_cal_temp = -50;
	return 0;
}

int
wlc_phy_create_chanctx(wlc_phy_t *ppi, chanspec_t chanspec)
{
	ch_calcache_t *ctx;
	phy_info_t *pi = (phy_info_t *)ppi;

	/* Check for existing */
	if (wlc_phy_get_chanctx(pi, chanspec))
		return 0;

	if (!(ctx = (ch_calcache_t *)MALLOC(pi->sh->osh, sizeof(ch_calcache_t)))) {
		PHY_ERROR(("%s: out of memory %d\n", __FUNCTION__, MALLOCED(pi->sh->osh)));
		return BCME_NOMEM;
	}
	bzero(ctx, sizeof(ch_calcache_t));

	ctx->chanspec = chanspec;
	ctx->creation_time = pi->sh->now;
	ctx->cal_info.last_cal_temp = -50;
	ctx->cal_info.txcal_numcmds = pi->def_cal_info.txcal_numcmds;

	/* Add it to the list */
	ctx->next = pi->phy_calcache;

	/* For the first context, switch out the default context */
	if (pi->phy_calcache == NULL &&
	    (pi->radio_chanspec == chanspec))
		pi->cal_info = &ctx->cal_info;

	pi->phy_calcache = ctx;
	return 0;
}

void
wlc_phy_destroy_chanctx(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ch_calcache_t *ctx = pi->phy_calcache, *rem = pi->phy_calcache;

	while (rem) {
		if (rem->chanspec == chanspec) {
			if (rem == pi->phy_calcache)
				pi->phy_calcache = rem->next;
			else
				ctx->next = rem->next;

			/* If the current cal_info points to the one being removed
			 * then switch NULL it
			 */
			if (pi->cal_info == &rem->cal_info)
				pi->cal_info = NULL;

			MFREE(pi->sh->osh, rem,
			      sizeof(ch_calcache_t));
			rem = NULL;
			break;
		}
		ctx = rem;
		rem = rem->next;
	}

	/* Set the correct context if one exists, otherwise,
	 * switch in the default one
	 */
	if (pi->cal_info == NULL) {
		ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
		if (!ctx) {
			pi->cal_info = &pi->def_cal_info;
			/* Reset the parameters */
			pi->cal_info->last_cal_temp = -50;
			pi->cal_info->last_cal_time = 0;
		} else
			pi->cal_info = &ctx->cal_info;
	}
}

ch_calcache_t *
wlc_phy_get_chanctx_oldest(phy_info_t *phi)
{
	ch_calcache_t *ctx = phi->phy_calcache;
	ch_calcache_t *ctx_lo_ctime = ctx;
	while (ctx->next) {
		ctx = ctx->next;
		if (ctx_lo_ctime->creation_time > ctx->creation_time)
			ctx_lo_ctime = ctx;
	}
	return ctx_lo_ctime;
}

ch_calcache_t *
wlc_phy_get_chanctx(phy_info_t *phi, chanspec_t chanspec)
{
	ch_calcache_t *ctx = phi->phy_calcache;
	while (ctx) {
		if (ctx->chanspec == chanspec)
			return ctx;
		ctx = ctx->next;
	}
	return NULL;
}


#if defined(WLMCHAN) && defined(BCMDBG)
static void
wlc_phydump_chanctx(phy_info_t *phi, struct bcmstrbuf *b)
{
	ch_calcache_t *ctx = phi->phy_calcache;
	bcm_bprintf(b, "Current chanspec: 0x%x\n", phi->radio_chanspec);
	while (ctx) {
			bcm_bprintf(b, "%sContext found for chanspec: 0x%x\n",
			            (ctx->valid)? "Valid ":"",
			            ctx->chanspec);
		if (ISNPHY(phi)) {
			wlc_phydump_cal_cache_nphy(phi, ctx, b);
		}
		else if (ISHTPHY(phi)) {
			wlc_phydump_cal_cache_htphy(phi, ctx, b);
		}
		ctx = ctx->next;
	}
}
#endif /* BCMDBG */

int
wlc_phy_cal_cache_restore(phy_info_t *pi)
{
	if (ISNPHY(pi)) {
		return (wlc_phy_cal_cache_restore_nphy(pi));
	} else if (ISHTPHY(pi)) {
		return (wlc_phy_cal_cache_restore_htphy(pi));
	} else {
		return 0;
	}
}
#endif /* defined(WLMCHAN) || defined(PHYCAL_CACHING) */

/* Generic routines to read and write sets of phy regs */
/* Callers to make sure the clocks are on, memory for the save/restore space is valid */

typedef enum access_phy_or_radio {
	ACCESS_PHY = 0,
	ACCESS_RADIO = 1
} access_phy_or_radio;

/*
 * wlc_regs_bulkread - Table driven read of multiple PHY or RADIO registers
 *
 * Parameters:
 *   pi           - Phy Info for the call to read_XXX_reg()
 *   addrvals     - A series of uint16: addr1, value1, addr2, value2, ..., addrN, valueN
 *                  The register contents are written into the "valueX" elements.
 *   nregs        - Number of registers to read.
 *                  There are this many addresses *plus* this many values in the array parameter.
 *   which        - Whether reading PHY or RADIO registers
 */
static void
wlc_regs_bulkread(phy_info_t *pi, uint16 *addrvals, uint32 nregs, access_phy_or_radio which)
{
	/* Pointer to step through array of address/value pairs. */
	uint16 * reginfop = addrvals;

	/*
	 * Use a function pointer to choose between
	 * access PHY or RADIO registers.
	 */
	uint16 (*readfp)(phy_info_t*, uint16) = 0;

	if (which == ACCESS_PHY)
		readfp = phy_reg_read;
	else
		readfp = read_radio_reg;

	/*
	 * Read the registers using address from the array parameters,
	 * and store the results back into the array.
	 */
	while (reginfop < addrvals + (2 * nregs)) {
		reginfop[1] = readfp(pi, reginfop[0]);
		reginfop += 2;
	}
}

/*
 * wlc_phyregs_bulkwrite - Table driven write of multiple PHY registers
 *
 * Parameters:
 *   pi           - Phy Info for the call to phy_reg_read()
 *   addrvals     - A series of uint16: addr1, value1, addr2, value2, ..., addrN, valueN
 *                  The register contents are written into the "valueX" elements.
 *   nregs        - Number of registers to read.
 *                  There are this many addresses *plus* this many values in the array parameter.
 *   which        - Whether reading phy or radio registers (0=phy, 1=radio)
 */
static void
wlc_regs_bulkwrite(phy_info_t *pi, const uint16 *addrvals, uint32 nregs, access_phy_or_radio which)
{
	/* Pointer to step through array of address/value pairs. */
	const uint16 *reginfop = addrvals;

	/*
	 * Use a function pointer to choose between
	 * accessing PHY or RADIO registers.
	 */
	void (*writefp)(phy_info_t*, uint16 addr, uint16 value) = 0;

	if (which == ACCESS_PHY)
		writefp = phy_reg_write;
	else
		writefp = write_radio_reg;

	while (reginfop < addrvals + (2 * nregs)) {
		writefp(pi, reginfop[0], reginfop[1]);
		reginfop += 2;
	}
}

void
wlc_phyregs_bulkread(phy_info_t *pi, uint16 *addrvals, uint32 nregs)
{
	wlc_regs_bulkread(pi, addrvals, nregs, ACCESS_PHY);
}

void
wlc_phyregs_bulkwrite(phy_info_t *pi, const uint16 *addrvals, uint32 nregs)
{
	wlc_regs_bulkwrite(pi, addrvals, nregs, ACCESS_PHY);
}

void
wlc_radioregs_bulkread(phy_info_t *pi, uint16 *addrvals, uint32 nregs)
{
	wlc_regs_bulkread(pi, addrvals, nregs, ACCESS_RADIO);
}

void
wlc_radioregs_bulkwrite(phy_info_t *pi, const uint16 *addrvals, uint32 nregs)
{
	wlc_regs_bulkwrite(pi, addrvals, nregs, ACCESS_RADIO);
}

void
wlc_mod_phyreg_bulk(phy_info_t *pi, uint16 *regs, uint16 *mask, uint16 *val, uint32 nregs)
{
	uint32 i;

	for (i = 0; i < nregs; i++)
		phy_reg_mod(pi, regs[i], mask[i], val[i]);
}

int
wlc_phy_txpower_core_offset_set(wlc_phy_t *ppi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	int err = BCME_UNSUPPORTED;

	if (pi->pi_fptr.txcorepwroffsetset)
		err = (*pi->pi_fptr.txcorepwroffsetset)(pi, offsets);

	return err;
}

int
wlc_phy_txpower_core_offset_get(wlc_phy_t *ppi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	int err = BCME_UNSUPPORTED;

	if (pi->pi_fptr.txcorepwroffsetget)
		err = (*pi->pi_fptr.txcorepwroffsetget)(pi, offsets);

	return err;
}

static void
BCMNMIATTACHFN(wlc_phy_txpwr_srom9_convert)(phy_info_t *pi, uint8 *srom_max, uint32 pwr_offset,
	uint8 tmp_max_pwr, uint8 rate_start, uint8 rate_end, bool shift)
{
	uint8 rate;
	uint8 nibble;

	if (pi->sh->sromrev < 9)
		return;

	for (rate = rate_start; rate <= rate_end; rate++) {
		nibble = (uint8)(pwr_offset & 0xf);
		if (shift)
			pwr_offset >>= 4;
		/* nibble info indicates offset in 0.5dB units convert to 0.25dB */
		srom_max[rate] = tmp_max_pwr - (nibble << 1);
	}
}

void
wlc_phy_txpwr_apply(phy_info_t *pi)
{
	srom9_pwrdet_t *pwrdet = &pi->pwrdet;
	uint8 tmp_max_pwr = 0;
	uint8 *tx_srom_max_rate = NULL;
	uint32 ppr_offsets[PWR_OFFSET_SIZE];
	uint32 pwr_offsets;
	uint rate_cnt, band_num, rate;
	bool shift;

	for (band_num = 0; band_num < (CH_2G_GROUP + CH_5G_GROUP + PHY_SUBBAND_NUM); band_num++) {
		bzero((uint8 *)ppr_offsets, PWR_OFFSET_SIZE * sizeof(uint32));
		if ((pi->sh->subband5Gver != PHY_SUBBAND_5BAND) &&
			(band_num >= (CH_2G_GROUP + CH_5G_GROUP)))
			break;

		/* find MIN of all ALL cores, board limits */
		tmp_max_pwr = MIN(pwrdet->max_pwr[0][band_num], pwrdet->max_pwr[1][band_num]);
		if (ISHTPHY(pi))
			tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[2][band_num]);

		switch (band_num) {
			case WL_CHAN_FREQ_RANGE_2G:
				PHY_INFORM(("convert 2G power-offset:\n"));
				/* 2G band */
				tx_srom_max_rate = pi->tx_srom_max_rate_2g;

				wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
					pi->ppr.sr9.cckbw202gpo, tmp_max_pwr,
					TXP_FIRST_CCK, TXP_LAST_CCK, TRUE);

				if (ISHTPHY(pi)) {
					wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
						pi->ppr.sr9.cckbw202gpo, tmp_max_pwr,
						TXP_FIRST_CCK_CDD_S1x2,
						TXP_LAST_CCK_CDD_S1x2, TRUE);
					wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
						pi->ppr.sr9.cckbw202gpo, tmp_max_pwr,
						TXP_FIRST_CCK_CDD_S1x3,
						TXP_LAST_CCK_CDD_S1x3, TRUE);
					wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
						pi->ppr.sr9.cckbw20ul2gpo, tmp_max_pwr,
						TXP_FIRST_20UL_CCK, TXP_LAST_20UL_CCK, TRUE);
					wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
						pi->ppr.sr9.cckbw20ul2gpo, tmp_max_pwr,
						TXP_FIRST_CCK_20U_CDD_S1x2,
						TXP_LAST_CCK_20U_CDD_S1x2, TRUE);
					wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate,
						pi->ppr.sr9.cckbw20ul2gpo, tmp_max_pwr,
						TXP_FIRST_CCK_20U_CDD_S1x3,
						TXP_LAST_CCK_20U_CDD_S1x3, TRUE);
				}

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm2g.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm2g.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = pi->ppr.sr9.ofdm2g.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs2g.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs2g.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs2g.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po & 0xf);

				break;
			case WL_CHAN_FREQ_RANGE_5GL:
				PHY_INFORM(("convert 5G LOW-BAND power-offset:\n"));
				/* 5G lowband */
				tx_srom_max_rate = pi->tx_srom_max_rate_5g_low;

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm5gl.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm5gl.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = pi->ppr.sr9.ofdm5gl.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs5gl.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs5gl.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs5gl.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 4) & 0xf;

				break;
			case WL_CHAN_FREQ_RANGE_5GM:
				PHY_INFORM(("convert 5G MID-BAND power-offset:\n"));
				/* 5G band, mid */
				tx_srom_max_rate = pi->tx_srom_max_rate_5g_mid;

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm5gm.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm5gm.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = pi->ppr.sr9.ofdm5gm.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs5gm.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs5gm.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs5gm.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 8) & 0xf;

				break;
			case WL_CHAN_FREQ_RANGE_5GH:
#ifdef	WL_PPR_SUBBAND
			case WL_CHAN_FREQ_RANGE_5GH_5BAND:
#endif	/* WL_PPR_SUBBAND */
				PHY_INFORM(("convert 5G HIGH-BAND power-offset:\n"));
				/* 5G highband */
				tx_srom_max_rate = pi->tx_srom_max_rate_5g_hi;

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm5gh.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm5gh.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = (uint32)pi->ppr.sr9.ofdm5gh.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs5gh.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs5gh.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs5gh.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 12) & 0xf;

				break;
#ifdef	WL_PPR_SUBBAND
			case WL_CHAN_FREQ_RANGE_5GLL_5BAND:
				PHY_INFORM(("convert 5G LOW-lo-BAND power-offset:\n"));
				/* 5G LOW-lo */
				tx_srom_max_rate = pi->tx_srom_max_rate_5g_ll;

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm5gll.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm5gll.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = (uint32)pi->ppr.sr9.ofdm5gll.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs5gll.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs5gll.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs5gll.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 8) & 0xf;

				break;
			case WL_CHAN_FREQ_RANGE_5GLH_5BAND:
				PHY_INFORM(("convert 5G LOW-hi-BAND power-offset:\n"));
				/* 5G LOW-hi */
				tx_srom_max_rate = pi->tx_srom_max_rate_5g_lh;

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm5glh.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm5glh.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = (uint32)pi->ppr.sr9.ofdm5glh.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs5glh.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs5glh.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs5glh.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 8) & 0xf;

				break;
			case WL_CHAN_FREQ_RANGE_5GML_5BAND:
				PHY_INFORM(("convert 5G MID-lo-BAND power-offset:\n"));
				/* 5G MID-lo */
				tx_srom_max_rate = pi->tx_srom_max_rate_5g_ml;

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm5gml.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm5gml.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = (uint32)pi->ppr.sr9.ofdm5gml.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs5gml.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs5gml.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs5gml.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 8) & 0xf;

				break;
			case WL_CHAN_FREQ_RANGE_5GMH_5BAND:
				PHY_INFORM(("convert 5G MID-hi-BAND power-offset:\n"));
				/* 5G MID-hi */
				tx_srom_max_rate = pi->tx_srom_max_rate_5g_mh;

				ppr_offsets[OFDM_20_PO] = pi->ppr.sr9.ofdm5gmh.bw20;
				ppr_offsets[OFDM_20UL_PO] = pi->ppr.sr9.ofdm5gmh.bw20ul;
				ppr_offsets[OFDM_40DUP_PO] = (uint32)pi->ppr.sr9.ofdm5gmh.bw40;
				ppr_offsets[MCS_20_PO] = pi->ppr.sr9.mcs5gmh.bw20;
				ppr_offsets[MCS_20UL_PO] = pi->ppr.sr9.mcs5gmh.bw20ul;
				ppr_offsets[MCS_40_PO] = pi->ppr.sr9.mcs5gmh.bw40;
				ppr_offsets[MCS32_PO] = (uint32)(pi->ppr.sr9.mcs32po >> 8) & 0xf;

				break;
#endif	/* WL_PPR_SUBBAND */
		}

		for (rate = TXP_FIRST_CCK; rate < TXP_NUM_RATES; rate += rate_cnt) {
			shift = TRUE;
			pwr_offsets = 0;
			switch (rate) {
				case TXP_FIRST_CCK:
				case TXP_FIRST_20UL_CCK:
				case TXP_FIRST_CCK_CDD_S1x2:
				case TXP_FIRST_CCK_CDD_S1x3:
				case TXP_FIRST_CCK_20U_CDD_S1x2:
				case TXP_FIRST_CCK_20U_CDD_S1x3:
					rate_cnt = WL_NUM_RATES_CCK;
					continue;
				case TXP_FIRST_OFDM:
				case TXP_FIRST_OFDM_20_CDD:
					pwr_offsets = ppr_offsets[OFDM_20_PO];
					rate_cnt = WL_NUM_RATES_OFDM;
					break;
				case TXP_FIRST_MCS_20_S1x1:
				case TXP_FIRST_MCS_20_S1x2:
				case TXP_FIRST_MCS_20_S1x3:
				case TXP_FIRST_MCS_20_S2x2:
				case TXP_FIRST_MCS_20_S2x3:
				case TXP_FIRST_MCS_20_S3x3:
#ifdef NOT_YET
				case TXP_FIRST_HT_STBC_S2x2:
				case TXP_FIRST_HT_STBC_S2x3:
#endif
					pwr_offsets = ppr_offsets[MCS_20_PO];
					rate_cnt = WL_NUM_RATES_MCS_1STREAM;
					break;
				case TXP_FIRST_OFDM_40_SISO:
				case TXP_FIRST_OFDM_40_CDD:
					pwr_offsets = ppr_offsets[OFDM_40DUP_PO];
					rate_cnt = WL_NUM_RATES_OFDM;
					break;
				case TXP_FIRST_MCS_40_S1x1:
				case TXP_FIRST_MCS_40_S1x2:
				case TXP_FIRST_MCS_40_S1x3:
				case TXP_FIRST_MCS_40_S2x2:
				case TXP_FIRST_MCS_40_S2x3:
				case TXP_FIRST_MCS_40_S3x3:
#ifdef NOT_YET
				case TXP_FIRST_HT_STBC_40_S2x2:
				case TXP_FIRST_HT_STBC_40_S2x3:
#endif
					pwr_offsets = ppr_offsets[MCS_40_PO];
					rate_cnt = WL_NUM_RATES_MCS_1STREAM;
					break;
				case TXP_MCS_32:
					pwr_offsets = ppr_offsets[MCS32_PO];
					rate_cnt = WL_NUM_RATES_MCS32;
					break;
				case TXP_FIRST_20UL_OFDM:
				case TXP_FIRST_20UL_OFDM_CDD:
					pwr_offsets = ppr_offsets[OFDM_20UL_PO];
					rate_cnt = WL_NUM_RATES_OFDM;
					break;
				case TXP_FIRST_20UL_S1x1:
				case TXP_FIRST_20UL_S1x2:
				case TXP_FIRST_20UL_S1x3:
				case TXP_FIRST_20UL_S2x2:
				case TXP_FIRST_20UL_S2x3:
				case TXP_FIRST_20UL_S3x3:
#ifdef NOT_YET
				case TXP_FIRST_HT_STBC_UL20_S2x2:
				case TXP_FIRST_HT_STBC_UL20_S2x3:
#endif
					pwr_offsets = ppr_offsets[MCS_20UL_PO];
					rate_cnt = WL_NUM_RATES_MCS_1STREAM;
					break;
				default:
					PHY_ERROR(("Invalid rate %d\n", rate));
					rate_cnt = 1;
					ASSERT(0);
					break;
			}
			wlc_phy_txpwr_srom9_convert(pi, tx_srom_max_rate, pwr_offsets,
				tmp_max_pwr, (uint8)rate, (uint8)rate+rate_cnt-1, shift);
		}
	}
}

static const char BCMATTACHDATA(rstr_cckbw202gpo)[] = "cckbw202gpo";
static const char BCMATTACHDATA(rstr_cckbw20ul2gpo)[] = "cckbw20ul2gpo";
static const char BCMATTACHDATA(rstr_legofdmbw202gpo)[] = "legofdmbw202gpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul2gpo)[] = "legofdmbw20ul2gpo";
static const char BCMATTACHDATA(rstr_legofdmbw205glpo)[] = "legofdmbw205glpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5glpo)[] = "legofdmbw20ul5glpo";
static const char BCMATTACHDATA(rstr_legofdmbw205gmpo)[] = "legofdmbw205gmpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5gmpo)[] = "legofdmbw20ul5gmpo";
static const char BCMATTACHDATA(rstr_legofdmbw205ghpo)[] = "legofdmbw205ghpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5ghpo)[] = "legofdmbw20ul5ghpo";
static const char BCMATTACHDATA(rstr_legofdmbw205gllpo)[] = "legofdmbw205gllpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5gllpo)[] = "legofdmbw20ul5gllpo";
static const char BCMATTACHDATA(rstr_legofdmbw205glhpo)[] = "legofdmbw205glhpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5glhpo)[] = "legofdmbw20ul5glhpo";
static const char BCMATTACHDATA(rstr_legofdmbw205gmlpo)[] = "legofdmbw205gmlpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5gmlpo)[] = "legofdmbw20ul5gmlpo";
static const char BCMATTACHDATA(rstr_legofdmbw205gmhpo)[] = "legofdmbw205gmhpo";
static const char BCMATTACHDATA(rstr_legofdmbw20ul5gmhpo)[] = "legofdmbw20ul5gmhpo";
static const char BCMATTACHDATA(rstr_mcsbw202gpo)[] = "mcsbw202gpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul2gpo)[] = "mcsbw20ul2gpo";
static const char BCMATTACHDATA(rstr_mcsbw402gpo)[] = "mcsbw402gpo";
static const char BCMATTACHDATA(rstr_mcsbw205glpo)[] = "mcsbw205glpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5glpo)[] = "mcsbw20ul5glpo";
static const char BCMATTACHDATA(rstr_mcsbw405glpo)[] = "mcsbw405glpo";
static const char BCMATTACHDATA(rstr_mcsbw205gmpo)[] = "mcsbw205gmpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gmpo)[] = "mcsbw20ul5gmpo";
static const char BCMATTACHDATA(rstr_mcsbw405gmpo)[] = "mcsbw405gmpo";
static const char BCMATTACHDATA(rstr_mcsbw205ghpo)[] = "mcsbw205ghpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5ghpo)[] = "mcsbw20ul5ghpo";
static const char BCMATTACHDATA(rstr_mcsbw405ghpo)[] = "mcsbw405ghpo";
static const char BCMATTACHDATA(rstr_mcsbw205gllpo)[] = "mcsbw205gllpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gllpo)[] = "mcsbw20ul5gllpo";
static const char BCMATTACHDATA(rstr_mcsbw405gllpo)[] = "mcsbw405gllpo";
static const char BCMATTACHDATA(rstr_mcsbw205glhpo)[] = "mcsbw205glhpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5glhpo)[] = "mcsbw20ul5glhpo";
static const char BCMATTACHDATA(rstr_mcsbw405glhpo)[] = "mcsbw405glhpo";
static const char BCMATTACHDATA(rstr_mcsbw205gmlpo)[] = "mcsbw205gmlpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gmlpo)[] = "mcsbw20ul5gmlpo";
static const char BCMATTACHDATA(rstr_mcsbw405gmlpo)[] = "mcsbw405gmlpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gmho)[] = "mcsbw20ul5gmho";
static const char BCMATTACHDATA(rstr_mcsbw205gmhpo)[] = "mcsbw205gmhpo";
static const char BCMATTACHDATA(rstr_mcsbw20ul5gmhpo)[] = "mcsbw20ul5gmhpo";
static const char BCMATTACHDATA(rstr_mcsbw405gmhpo)[] = "mcsbw405gmhpo";
static const char BCMATTACHDATA(rstr_mcs32po)[] = "mcs32po";
static const char BCMATTACHDATA(rstr_legofdm40duppo)[] = "legofdm40duppo";

static void
BCMATTACHFN(wlc_phy_txpwr_srom9_read_ppr)(phy_info_t *pi)
{
	/* Read and interpret the power-offset parameters from the SROM for each band/subband */
	if (pi->sh->sromrev >= 9) {
		int i, j;

		PHY_INFORM(("Get SROM 9 Power Offset per rate\n"));
		/* 2G CCK */
		pi->ppr.sr9.cckbw202gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
		pi->ppr.sr9.cckbw20ul2gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw20ul2gpo);
		/* 2G & 5G OFDM power offsets */
		pi->ppr.sr9.ofdm2g.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw202gpo);
		pi->ppr.sr9.ofdm2g.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul2gpo);
		pi->ppr.sr9.ofdm5gl.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205glpo);
		pi->ppr.sr9.ofdm5gl.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5glpo);
		pi->ppr.sr9.ofdm5gm.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gmpo);
		pi->ppr.sr9.ofdm5gm.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gmpo);
		pi->ppr.sr9.ofdm5gh.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205ghpo);
		pi->ppr.sr9.ofdm5gh.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5ghpo);
#ifdef	WL_PPR_SUBBAND
		if (pi->sh->subband5Gver == PHY_SUBBAND_5BAND) {
			if (PHY_GETVAR(pi, rstr_legofdmbw205gllpo) != NULL) {
				pi->ppr.sr9.ofdm5gll.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gllpo);
				pi->ppr.sr9.ofdm5gll.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gllpo);
			} else {
				pi->ppr.sr9.ofdm5gll.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205glpo);
				pi->ppr.sr9.ofdm5gll.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5glpo);
			}
			if (PHY_GETVAR(pi, rstr_legofdmbw205glhpo) != NULL) {
				pi->ppr.sr9.ofdm5glh.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205glhpo);
				pi->ppr.sr9.ofdm5glh.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5glhpo);
			} else {
				pi->ppr.sr9.ofdm5glh.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205glpo);
				pi->ppr.sr9.ofdm5glh.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5glpo);
			}
			if (PHY_GETVAR(pi, rstr_legofdmbw205gmlpo) != NULL) {
				pi->ppr.sr9.ofdm5gml.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gmlpo);
				pi->ppr.sr9.ofdm5gml.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gmlpo);
			} else {
				pi->ppr.sr9.ofdm5gml.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gmpo);
				pi->ppr.sr9.ofdm5gml.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gmpo);
			}
			if (PHY_GETVAR(pi, rstr_legofdmbw205gmhpo) != NULL) {
				pi->ppr.sr9.ofdm5gmh.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gmhpo);
				pi->ppr.sr9.ofdm5gmh.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gmhpo);
			} else {
				pi->ppr.sr9.ofdm5gmh.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gmpo);
				pi->ppr.sr9.ofdm5gmh.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gmpo);
			}
		}
#endif	/* WL_PPR_SUBBAND	*/

		/* 2G & 5G MCS power offsets */
		pi->ppr.sr9.mcs2g.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
		pi->ppr.sr9.mcs2g.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul2gpo);
		pi->ppr.sr9.mcs2g.bw40 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);
		pi->ppr.sr9.mcs5gl.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
		pi->ppr.sr9.mcs5gl.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5glpo);
		pi->ppr.sr9.mcs5gl.bw40 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
		pi->ppr.sr9.mcs5gm.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
		pi->ppr.sr9.mcs5gm.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gmpo);
		pi->ppr.sr9.mcs5gm.bw40 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
		pi->ppr.sr9.mcs5gh.bw20 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);
		pi->ppr.sr9.mcs5gh.bw20ul = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5ghpo);
		pi->ppr.sr9.mcs5gh.bw40 = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);
#ifdef	WL_PPR_SUBBAND
		if (pi->sh->subband5Gver == PHY_SUBBAND_5BAND) {
			if (PHY_GETVAR(pi, rstr_mcsbw205gllpo) != NULL) {
				pi->ppr.sr9.mcs5gll.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gllpo);
				pi->ppr.sr9.mcs5gll.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gllpo);
				pi->ppr.sr9.mcs5gll.bw40 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gllpo);
			} else {
				pi->ppr.sr9.mcs5gll.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
				pi->ppr.sr9.mcs5gll.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5glpo);
				pi->ppr.sr9.mcs5gll.bw40 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
			}
			if (PHY_GETVAR(pi, rstr_mcsbw205glhpo) != NULL) {
				pi->ppr.sr9.mcs5glh.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glhpo);
				pi->ppr.sr9.mcs5glh.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5glhpo);
				pi->ppr.sr9.mcs5glh.bw40 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glhpo);
			} else {
				pi->ppr.sr9.mcs5glh.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
				pi->ppr.sr9.mcs5glh.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5glpo);
				pi->ppr.sr9.mcs5glh.bw40 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
			}
			if (PHY_GETVAR(pi, rstr_mcsbw205gmlpo) != NULL) {
				pi->ppr.sr9.mcs5gml.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmlpo);
				pi->ppr.sr9.mcs5gml.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gmlpo);
				pi->ppr.sr9.mcs5gml.bw40 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmlpo);
			} else {
				pi->ppr.sr9.mcs5gml.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
				pi->ppr.sr9.mcs5gml.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gmho);
				pi->ppr.sr9.mcs5gml.bw40 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
			}
			if (PHY_GETVAR(pi, rstr_mcsbw205gmhpo) != NULL) {
				pi->ppr.sr9.mcs5gmh.bw20 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmhpo);
				pi->ppr.sr9.mcs5gmh.bw20ul =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gmhpo);
				pi->ppr.sr9.mcs5gmh.bw40 =
					(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmhpo);
			} else {
				pi->ppr.sr9.mcs5gmh.bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
				pi->ppr.sr9.mcs5gmh.bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gmpo);
				pi->ppr.sr9.mcs5gmh.bw40 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
			}
		}
#endif	/* WL_PPR_SUBBAND */

		/* MCS32 */
		pi->ppr.sr9.mcs32po = (uint16)PHY_GETINTVAR(pi, rstr_mcs32po);
		/* 40 Dups */
		pi->ppr.sr9.ofdm40duppo = (uint16)PHY_GETINTVAR(pi, rstr_legofdm40duppo);
		pi->ppr.sr9.ofdm2g.bw40 = pi->ppr.sr9.ofdm2g.bw20ul;
		for (i = 0; i < (4 + PHY_SUBBAND_NUM); i++) {
			uint32 nibble, dup40_offset = 0;
			if ((pi->sh->subband5Gver != PHY_SUBBAND_5BAND) && (i > 3))
				break;

			nibble = pi->ppr.sr9.ofdm40duppo & 0xf;
			for (j = 0; j < WL_NUM_RATES_OFDM; j++) {
				dup40_offset |= nibble;
				nibble <<= 4;
			}
			if (i == 0)
				pi->ppr.sr9.ofdm2g.bw40 =
					pi->ppr.sr9.ofdm2g.bw20ul + dup40_offset;
			else if (i == 1)
				pi->ppr.sr9.ofdm5gl.bw40 =
					pi->ppr.sr9.ofdm5gl.bw20ul + dup40_offset;
			else if (i == 2)
				pi->ppr.sr9.ofdm5gm.bw40 =
					pi->ppr.sr9.ofdm5gm.bw20ul + dup40_offset;
			else if (i == 3)
				pi->ppr.sr9.ofdm5gh.bw40 =
					pi->ppr.sr9.ofdm5gh.bw20ul + dup40_offset;
#ifdef	WL_PPR_SUBBAND
			else if (i == 4)
				pi->ppr.sr9.ofdm5gll.bw40 =
					pi->ppr.sr9.ofdm5gll.bw20ul + dup40_offset;
			else if (i == 5)
				pi->ppr.sr9.ofdm5glh.bw40 =
					pi->ppr.sr9.ofdm5glh.bw20ul + dup40_offset;
			else if (i == 6)
				pi->ppr.sr9.ofdm5gml.bw40 =
					pi->ppr.sr9.ofdm5gml.bw20ul + dup40_offset;
			else if (i == 7)
				pi->ppr.sr9.ofdm5gmh.bw40 =
					pi->ppr.sr9.ofdm5gmh.bw20ul + dup40_offset;
#endif	/* WL_PPR_SUBBAND */
		}
	}

	PHY_INFORM(("CCK: 0x%04x 0x%04x\n", pi->ppr.sr9.cckbw202gpo, pi->ppr.sr9.cckbw202gpo));
	PHY_INFORM(("OFDM: 0x%08x 0x%08x 0x%02x\n",
		pi->ppr.sr9.ofdm2g.bw20, pi->ppr.sr9.ofdm2g.bw20ul, pi->ppr.sr9.ofdm2g.bw40));
	PHY_INFORM(("MCS: 0x%08x 0x%08x 0x%08x 0x%04x\n",
		pi->ppr.sr9.mcs2g.bw20, pi->ppr.sr9.mcs2g.bw20ul, pi->ppr.sr9.mcs2g.bw40,
		pi->ppr.sr9.mcs32po));

	/* Finished reading from SROM, calculate and apply powers */
	wlc_phy_txpwr_apply(pi);
}

static const char BCMATTACHDATA(rstr_elna2g)[] = "elna2g";
static const char BCMATTACHDATA(rstr_elna5g)[] = "elna5g";
static const char BCMATTACHDATA(rstr_antswitch)[] = "antswitch";
static const char BCMATTACHDATA(rstr_aa2g)[] = "aa2g";
static const char BCMATTACHDATA(rstr_aa5g)[] = "aa5g";
static const char BCMATTACHDATA(rstr_tssipos2g)[] = "tssipos2g";
static const char BCMATTACHDATA(rstr_extpagain2g)[] = "extpagain2g";
static const char BCMATTACHDATA(rstr_pdetrange2g)[] = "pdetrange2g";
static const char BCMATTACHDATA(rstr_triso2g)[] = "triso2g";
static const char BCMATTACHDATA(rstr_antswctl2g)[] = "antswctl2g";
static const char BCMATTACHDATA(rstr_tssipos5g)[] = "tssipos5g";
static const char BCMATTACHDATA(rstr_extpagain5g)[] = "extpagain5g";
static const char BCMATTACHDATA(rstr_pdetrange5g)[] = "pdetrange5g";
static const char BCMATTACHDATA(rstr_triso5g)[] = "triso5g";
static const char BCMATTACHDATA(rstr_antswctl5g)[] = "antswctl5g";
static const char BCMATTACHDATA(rstr_pcieingress_war)[] = "pcieingress_war";
static const char BCMATTACHDATA(rstr_maxp2ga0)[] = "maxp2ga0";
static const char BCMATTACHDATA(rstr_maxp2ga1)[] = "maxp2ga1";
static const char BCMATTACHDATA(rstr_maxp2ga2)[] = "maxp2ga2";
static const char BCMATTACHDATA(rstr_pa2gw0a0)[] = "pa2gw0a0";
static const char BCMATTACHDATA(rstr_pa2gw0a1)[] = "pa2gw0a1";
static const char BCMATTACHDATA(rstr_pa2gw0a2)[] = "pa2gw0a2";
static const char BCMATTACHDATA(rstr_pa2gw1a0)[] = "pa2gw1a0";
static const char BCMATTACHDATA(rstr_pa2gw1a1)[] = "pa2gw1a1";
static const char BCMATTACHDATA(rstr_pa2gw1a2)[] = "pa2gw1a2";
static const char BCMATTACHDATA(rstr_pa2gw2a0)[] = "pa2gw2a0";
static const char BCMATTACHDATA(rstr_pa2gw2a1)[] = "pa2gw2a1";
static const char BCMATTACHDATA(rstr_pa2gw2a2)[] = "pa2gw2a2";
static const char BCMATTACHDATA(rstr_maxp5gla0)[] = "maxp5gla0";
static const char BCMATTACHDATA(rstr_maxp5gla1)[] = "maxp5gla1";
static const char BCMATTACHDATA(rstr_maxp5gla2)[] = "maxp5gla2";
static const char BCMATTACHDATA(rstr_pa5glw0a0)[] = "pa5glw0a0";
static const char BCMATTACHDATA(rstr_pa5glw0a1)[] = "pa5glw0a1";
static const char BCMATTACHDATA(rstr_pa5glw0a2)[] = "pa5glw0a2";
static const char BCMATTACHDATA(rstr_pa5glw1a0)[] = "pa5glw1a0";
static const char BCMATTACHDATA(rstr_pa5glw1a1)[] = "pa5glw1a1";
static const char BCMATTACHDATA(rstr_pa5glw1a2)[] = "pa5glw1a2";
static const char BCMATTACHDATA(rstr_pa5glw2a0)[] = "pa5glw2a0";
static const char BCMATTACHDATA(rstr_pa5glw2a1)[] = "pa5glw2a1";
static const char BCMATTACHDATA(rstr_pa5glw2a2)[] = "pa5glw2a2";
static const char BCMATTACHDATA(rstr_maxp5ga0)[] = "maxp5ga0";
static const char BCMATTACHDATA(rstr_maxp5ga1)[] = "maxp5ga1";
static const char BCMATTACHDATA(rstr_maxp5ga2)[] = "maxp5ga2";
static const char BCMATTACHDATA(rstr_pa5gw0a0)[] = "pa5gw0a0";
static const char BCMATTACHDATA(rstr_pa5gw0a1)[] = "pa5gw0a1";
static const char BCMATTACHDATA(rstr_pa5gw0a2)[] = "pa5gw0a2";
static const char BCMATTACHDATA(rstr_pa5gw1a0)[] = "pa5gw1a0";
static const char BCMATTACHDATA(rstr_pa5gw1a1)[] = "pa5gw1a1";
static const char BCMATTACHDATA(rstr_pa5gw1a2)[] = "pa5gw1a2";
static const char BCMATTACHDATA(rstr_pa5gw2a0)[] = "pa5gw2a0";
static const char BCMATTACHDATA(rstr_pa5gw2a1)[] = "pa5gw2a1";
static const char BCMATTACHDATA(rstr_pa5gw2a2)[] = "pa5gw2a2";
static const char BCMATTACHDATA(rstr_maxp5gha0)[] = "maxp5gha0";
static const char BCMATTACHDATA(rstr_maxp5gha1)[] = "maxp5gha1";
static const char BCMATTACHDATA(rstr_maxp5gha2)[] = "maxp5gha2";
static const char BCMATTACHDATA(rstr_pa5ghw0a0)[] = "pa5ghw0a0";
static const char BCMATTACHDATA(rstr_pa5ghw0a1)[] = "pa5ghw0a1";
static const char BCMATTACHDATA(rstr_pa5ghw0a2)[] = "pa5ghw0a2";
static const char BCMATTACHDATA(rstr_pa5ghw1a0)[] = "pa5ghw1a0";
static const char BCMATTACHDATA(rstr_pa5ghw1a1)[] = "pa5ghw1a1";
static const char BCMATTACHDATA(rstr_pa5ghw1a2)[] = "pa5ghw1a2";
static const char BCMATTACHDATA(rstr_pa5ghw2a0)[] = "pa5ghw2a0";
static const char BCMATTACHDATA(rstr_pa5ghw2a1)[] = "pa5ghw2a1";
static const char BCMATTACHDATA(rstr_pa5ghw2a2)[] = "pa5ghw2a2";
static const char BCMATTACHDATA(rstr_maxp5gmha0)[] = "maxp5gmha0";
static const char BCMATTACHDATA(rstr_maxp5gmha1)[] = "maxp5gmha1";
static const char BCMATTACHDATA(rstr_maxp5gmha2)[] = "maxp5gmha2";
static const char BCMATTACHDATA(rstr_pa5gmhw0a0)[] = "pa5gmhw0a0";
static const char BCMATTACHDATA(rstr_pa5gmhw0a1)[] = "pa5gmhw0a1";
static const char BCMATTACHDATA(rstr_pa5gmhw0a2)[] = "pa5gmhw0a2";
static const char BCMATTACHDATA(rstr_pa5gmhw1a0)[] = "pa5gmhw1a0";
static const char BCMATTACHDATA(rstr_pa5gmhw1a1)[] = "pa5gmhw1a1";
static const char BCMATTACHDATA(rstr_pa5gmhw1a2)[] = "pa5gmhw1a2";
static const char BCMATTACHDATA(rstr_pa5gmhw2a0)[] = "pa5gmhw2a0";
static const char BCMATTACHDATA(rstr_pa5gmhw2a1)[] = "pa5gmhw2a1";
static const char BCMATTACHDATA(rstr_pa5gmhw2a2)[] = "pa5gmhw2a2";
static const char BCMATTACHDATA(rstr_maxp5glla0)[] = "maxp5glla0";
static const char BCMATTACHDATA(rstr_maxp5glla1)[] = "maxp5glla1";
static const char BCMATTACHDATA(rstr_maxp5glla2)[] = "maxp5glla2";
static const char BCMATTACHDATA(rstr_pa5gllw0a0)[] = "pa5gllw0a0";
static const char BCMATTACHDATA(rstr_pa5gllw0a1)[] = "pa5gllw0a1";
static const char BCMATTACHDATA(rstr_pa5gllw0a2)[] = "pa5gllw0a2";
static const char BCMATTACHDATA(rstr_pa5gllw1a0)[] = "pa5gllw1a0";
static const char BCMATTACHDATA(rstr_pa5gllw1a1)[] = "pa5gllw1a1";
static const char BCMATTACHDATA(rstr_pa5gllw1a2)[] = "pa5gllw1a2";
static const char BCMATTACHDATA(rstr_pa5gllw2a0)[] = "pa5gllw2a0";
static const char BCMATTACHDATA(rstr_pa5gllw2a1)[] = "pa5gllw2a1";
static const char BCMATTACHDATA(rstr_pa5gllw2a2)[] = "pa5gllw2a2";
static const char BCMATTACHDATA(rstr_maxp5glha0)[] = "maxp5glha0";
static const char BCMATTACHDATA(rstr_maxp5glha1)[] = "maxp5glha1";
static const char BCMATTACHDATA(rstr_maxp5glha2)[] = "maxp5glha2";
static const char BCMATTACHDATA(rstr_pa5glhw0a0)[] = "pa5glhw0a0";
static const char BCMATTACHDATA(rstr_pa5glhw0a1)[] = "pa5glhw0a1";
static const char BCMATTACHDATA(rstr_pa5glhw0a2)[] = "pa5glhw0a2";
static const char BCMATTACHDATA(rstr_pa5glhw1a0)[] = "pa5glhw1a0";
static const char BCMATTACHDATA(rstr_pa5glhw1a1)[] = "pa5glhw1a1";
static const char BCMATTACHDATA(rstr_pa5glhw1a2)[] = "pa5glhw1a2";
static const char BCMATTACHDATA(rstr_pa5glhw2a0)[] = "pa5glhw2a0";
static const char BCMATTACHDATA(rstr_pa5glhw2a1)[] = "pa5glhw2a1";
static const char BCMATTACHDATA(rstr_pa5glhw2a2)[] = "pa5glhw2a2";
static const char BCMATTACHDATA(rstr_maxp5gmla0)[] = "maxp5gmla0";
static const char BCMATTACHDATA(rstr_maxp5gmla1)[] = "maxp5gmla1";
static const char BCMATTACHDATA(rstr_maxp5gmla2)[] = "maxp5gmla2";
static const char BCMATTACHDATA(rstr_pa5gmlw0a0)[] = "pa5gmlw0a0";
static const char BCMATTACHDATA(rstr_pa5gmlw0a1)[] = "pa5gmlw0a1";
static const char BCMATTACHDATA(rstr_pa5gmlw0a2)[] = "pa5gmlw0a2";
static const char BCMATTACHDATA(rstr_pa5gmlw1a0)[] = "pa5gmlw1a0";
static const char BCMATTACHDATA(rstr_pa5gmlw1a1)[] = "pa5gmlw1a1";
static const char BCMATTACHDATA(rstr_pa5gmlw1a2)[] = "pa5gmlw1a2";
static const char BCMATTACHDATA(rstr_pa5gmlw2a0)[] = "pa5gmlw2a0";
static const char BCMATTACHDATA(rstr_pa5gmlw2a1)[] = "pa5gmlw2a1";
static const char BCMATTACHDATA(rstr_pa5gmlw2a2)[] = "pa5gmlw2a2";

bool
BCMATTACHFN(wlc_phy_txpwr_srom9_read)(phy_info_t *pi)
{
	srom9_pwrdet_t *pwrdet = &pi->pwrdet;
	uint b;

	ASSERT(pi->sh->sromrev >= 9);

	if (PHY_GETVAR(pi, rstr_elna2g)) {
		/* extlnagain2g entry exists, so use it. */
		pi->u.pi_nphy->elna2g = (uint8)PHY_GETINTVAR(pi, rstr_elna2g);
	}
	if (PHY_GETVAR(pi, rstr_elna5g)) {
		/* extlnagain5g entry exists, so use it. */
		pi->u.pi_nphy->elna5g = (uint8)PHY_GETINTVAR(pi, rstr_elna5g);
	}

	/* read in antenna-related config */
	pi->antswitch = (uint8) PHY_GETINTVAR(pi, rstr_antswitch);
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, rstr_aa2g);
	pi->aa5g = (uint8) PHY_GETINTVAR(pi, rstr_aa5g);

	/* read in FEM stuff */
	pi->srom_fem2g.tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos2g);
	pi->srom_fem2g.extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain2g);
	pi->srom_fem2g.pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange2g);
	pi->srom_fem2g.triso = (uint8)PHY_GETINTVAR(pi, rstr_triso2g);
	pi->srom_fem2g.antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl2g);

	pi->srom_fem5g.tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos5g);
	pi->srom_fem5g.extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain5g);
	pi->srom_fem5g.pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange5g);
	pi->srom_fem5g.triso = (uint8)PHY_GETINTVAR(pi, rstr_triso5g);
	pi->srom_fem5g.antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl5g);

	/* srom_fem2/5g.extpagain changed */
	wlc_phy_txpower_ipa_upd(pi);



	/* read pwrdet params for each band/subband */
	for (b = 0; b < (CH_2G_GROUP + CH_5G_GROUP + PHY_SUBBAND_NUM); b++) {
		if ((pi->sh->subband5Gver != PHY_SUBBAND_5BAND) &&
			(b >= (CH_2G_GROUP + CH_5G_GROUP)))
			break;
		switch (b) {
			case WL_CHAN_FREQ_RANGE_2G: /* 0 */
			/* 2G band */
				pwrdet->max_pwr[0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
				pwrdet->max_pwr[1][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga1);
				pwrdet->max_pwr[2][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga2);
				pwrdet->pwrdet_a1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw0a0);
				pwrdet->pwrdet_a1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw0a1);
				pwrdet->pwrdet_a1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw0a2);
				pwrdet->pwrdet_b0[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw1a0);
				pwrdet->pwrdet_b0[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw1a1);
				pwrdet->pwrdet_b0[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw1a2);
				pwrdet->pwrdet_b1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw2a0);
				pwrdet->pwrdet_b1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw2a1);
				pwrdet->pwrdet_b1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw2a2);
				break;
			case WL_CHAN_FREQ_RANGE_5GL: /* 1 */
			/* 5G band, low */
				pwrdet->max_pwr[0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gla0);
				pwrdet->max_pwr[1][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gla1);
				pwrdet->max_pwr[2][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gla2);
				pwrdet->pwrdet_a1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw0a0);
				pwrdet->pwrdet_a1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw0a1);
				pwrdet->pwrdet_a1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw0a2);
				pwrdet->pwrdet_b0[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw1a0);
				pwrdet->pwrdet_b0[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw1a1);
				pwrdet->pwrdet_b0[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw1a2);
				pwrdet->pwrdet_b1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw2a0);
				pwrdet->pwrdet_b1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw2a1);
				pwrdet->pwrdet_b1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw2a2);
				break;
			case WL_CHAN_FREQ_RANGE_5GM: /* 2 */
			/* 5G band, mid */
				pwrdet->max_pwr[0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);
				pwrdet->max_pwr[1][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga1);
				pwrdet->max_pwr[2][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga2);
				pwrdet->pwrdet_a1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw0a0);
				pwrdet->pwrdet_a1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw0a1);
				pwrdet->pwrdet_a1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw0a2);
				pwrdet->pwrdet_b0[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw1a0);
				pwrdet->pwrdet_b0[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw1a1);
				pwrdet->pwrdet_b0[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw1a2);
				pwrdet->pwrdet_b1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw2a0);
				pwrdet->pwrdet_b1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw2a1);
				pwrdet->pwrdet_b1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw2a2);
				break;
#ifdef	WL_PPR_SUBBAND
			case WL_CHAN_FREQ_RANGE_5GH_5BAND:
#endif	/* WL_PPR_SUBBAND */
			case WL_CHAN_FREQ_RANGE_5GH: /* 3 */
			/* 5G band, high */
				pwrdet->max_pwr[0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
				pwrdet->max_pwr[1][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
				pwrdet->max_pwr[2][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
				pwrdet->pwrdet_a1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
				pwrdet->pwrdet_a1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
				pwrdet->pwrdet_a1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
				pwrdet->pwrdet_b0[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
				pwrdet->pwrdet_b0[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
				pwrdet->pwrdet_b0[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
				pwrdet->pwrdet_b1[0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);
				pwrdet->pwrdet_b1[1][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
				pwrdet->pwrdet_b1[2][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
				break;
#ifdef	WL_PPR_SUBBAND
			/* can't program this parameter in sb/1, pci/1/1 but root folder */
			case WL_CHAN_FREQ_RANGE_5GLL_5BAND: /* 4 */
			/* 5G band, LOW-lo CH36-CH48 */
				if (PHY_GETVAR(pi, rstr_maxp5glla0) != NULL) {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5glla0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5glla1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5glla2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gllw2a2);
				} else {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a2);
				}
				break;
			case WL_CHAN_FREQ_RANGE_5GLH_5BAND: /* 5 */
			/* 5G band, LOW-hi CH52-CH64 */
				if (PHY_GETVAR(pi, rstr_maxp5glha0) != NULL) {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5glha0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5glha1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5glha2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5glhw2a2);
				} else {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5ga2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a2);
				}
				break;
			case WL_CHAN_FREQ_RANGE_5GML_5BAND: /* 6 */
			/* 5G band, MID-lo CH100-CH120 */
				if (PHY_GETVAR(pi, rstr_maxp5gmla0) != NULL) {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gmla0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gmla1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gmla2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmlw2a2);
				} else {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
				}
				break;
			case WL_CHAN_FREQ_RANGE_5GMH_5BAND: /* 7 */
			/* 5G band, MID-lo CH124-CH140 */
				if (PHY_GETVAR(pi, rstr_maxp5gmha0) != NULL) {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gmha0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gmha1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gmha2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5gmhw2a2);
				} else {
					pwrdet->max_pwr[0][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
					pwrdet->max_pwr[1][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
					pwrdet->max_pwr[2][b] =
						(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
					pwrdet->pwrdet_a1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
					pwrdet->pwrdet_a1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
					pwrdet->pwrdet_a1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
					pwrdet->pwrdet_b0[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
					pwrdet->pwrdet_b0[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
					pwrdet->pwrdet_b0[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
					pwrdet->pwrdet_b1[0][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);
					pwrdet->pwrdet_b1[1][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
					pwrdet->pwrdet_b1[2][b] =
						(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
				}
				break;
#endif	/* WL_PPR_SUBBAND */
		}
	}

	wlc_phy_txpwr_srom9_read_ppr(pi);
	return TRUE;
}

void
wlc_phy_antsel_init(wlc_phy_t *ppi, bool lut_init)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (ISNPHY(pi))
		wlc_phy_antsel_init_nphy(ppi, lut_init);
}

void
wlc_btcx_override_enable(phy_info_t *pi)
{
	if (pi->sh->machwcap & MCAP_BTCX) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC));

		/* Enable manual BTCX mode */
		W_REG(pi->sh->osh, &pi->regs->btcx_ctrl, 0x03);
		/* Force WLAN priority */
		W_REG(pi->sh->osh, &pi->regs->btcx_trans_ctrl, 0xff);
	}
}

bool
wlc_phy_no_cal_possible(phy_info_t *pi)
{
	return (SCAN_RM_IN_PROGRESS(pi));
}


#if PHY_TSSI_CAL_DBG_EN
static void
print_int64(int64 *a)
{
	void *llp = a;
	uint32 *lp_low = (uint32 *)llp;
	uint32 *lp_high = lp_low + 1;
	printf("0x%08x%08x ", *lp_high, *lp_low);
}
#endif


#if PHY_TSSI_CAL_DBG_EN
/*
 * matrix print
 * dimensions a (m x n)
 * name - matrix name
 */
static void
mat_print(int64 *a, int m, int n, const char *name)
{
	int i, j;

	printf("\n%s\n", name);
	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++)
			print_int64(a + (i * n) + j);
		printf("\n");
	}
}
#else /* PHY_TSSI_CAL_DBG_EN */
static void
mat_print(int64 *a, int m, int n, const char *name)
{
}
#endif /* PHY_TSSI_CAL_DBG_EN */


/*
 * Compute matrix rho ( m x 3)
 *
 * column 1 = all 1s
 * column 2 = n[i]
 * column 3 = - n[i] * P[i]
 */
static void
mat_rho(int64 *n, int64 *p, int64 *rho, int m)
{
	int i;
	int q1 = 2;

	for (i = 0; i < m; i++) {
		*(rho + (i * 3) + 0) = 1;
		*(rho + (i * 3) + 1) = *(n + (i * 1) + 0);
		*(rho + (i * 3) + 2) =
			- (*(n + (i * 1) + 0) * (*(p + (i * 1) + 0)));
		*(rho + (i * 3) + 2) = (*(rho + (i * 3) + 2) + (int64)(1<<(q1-1))) >> q1;
	}
}

/*
 * Matrix transpose routine
 * matrix a (m x n)
 * matrix b = a_transpose(n x m)
 */
static void
mat_transpose(int64 *a, int64 *b, int m, int n)
{
	int i, j;

	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++)
			/* b[j][i] = a[i][j]; */
			*(b + (j * m) + i) = *(a + (i * n) + j);
}

/*
 * Matrix multiply routine.
 * matrix a (m x n)
 * matrix b (n x r)
 * c = result matrix (m x r)
 * m = number of rows of matrix a
 * n = number of cols of matrix a and number of rows of matrix b
 * r = number of cols of matrix b
 * assumes matrixes are allocated in memory contiguously one row after
 * the other
 *
 */
static void
mat_mult(int64 *a, int64 *b, int64 *c, int m, int n, int r)
{
	int i, j, k;

	for (i = 0; i < m; i++)
		for (j = 0; j < r; j++) {
			*(c + (i * r) + j) = 0;
			for (k = 0; k < n; k++)
				/* c[i][j] += a[i][k] * b[k][j]; */
				*(c + (i * r) + j)
					+= *(a + (i * n) + k) *
					*(b + (k * r) + j);
		}
}


/*
 * Matrix inverse of a 3x3 matrix * det(matrix)
 * a and b: matrices of 3x3
 */
static void
mat_inv_prod_det(int64 *a, int64 *b)
{

	/* C2_calc = [	a22*a33 - a32*a23  a13*a32 - a12*a33  a12*a23 - a13*a22
					a23*a31 - a21*a33  a11*a33 - a13*a31  a13*a21 - a11*a23
					a21*a32 - a31*a22  a12*a31 - a11*a32  a11*a22 - a12*a21];
	*/

	int64 a11 = *(a + (0 * 3) + 0);
	int64 a12 = *(a + (0 * 3) + 1);
	int64 a13 = *(a + (0 * 3) + 2);

	int64 a21 = *(a + (1 * 3) + 0);
	int64 a22 = *(a + (1 * 3) + 1);
	int64 a23 = *(a + (1 * 3) + 2);

	int64 a31 = *(a + (2 * 3) + 0);
	int64 a32 = *(a + (2 * 3) + 1);
	int64 a33 = *(a + (2 * 3) + 2);

	*(b + (0 * 3) + 0) = a22 * a33 - a32 * a23;
	*(b + (0 * 3) + 1) = a13 * a32 - a12 * a33;
	*(b + (0 * 3) + 2) = a12 * a23 - a13 * a22;

	*(b + (1 * 3) + 0) = a23 * a31 - a21 * a33;
	*(b + (1 * 3) + 1) = a11 * a33 - a13 * a31;
	*(b + (1 * 3) + 2) = a13 * a21 - a11 * a23;

	*(b + (2 * 3) + 0) = a21 * a32 - a31 * a22;
	*(b + (2 * 3) + 1) =  a12 * a31 - a11 * a32;
	*(b + (2 * 3) + 2) = a11 * a22 - a12 * a21;
}

static void
mat_det(int64 *a, int64 *det)
{

	/* det_C1 = a11*a22*a33 + a12*a23*a31 + a13*a21*a32 -
				 a11*a23*a32 - a12*a21*a33 - a13*a22*a31;
	*/

	int64 a11 = *(a + (0 * 3) + 0);
	int64 a12 = *(a + (0 * 3) + 1);
	int64 a13 = *(a + (0 * 3) + 2);

	int64 a21 = *(a + (1 * 3) + 0);
	int64 a22 = *(a + (1 * 3) + 1);
	int64 a23 = *(a + (1 * 3) + 2);

	int64 a31 = *(a + (2 * 3) + 0);
	int64 a32 = *(a + (2 * 3) + 1);
	int64 a33 = *(a + (2 * 3) + 2);

	*det = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
		a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31;
}

/* ================================================================
function [b0 b1 a1] = ratmodel_paparams_fix64(n, P)

%This is the algorithm used for curve fitting to get PA Params
%n: Adjusted TSSI values
%P: Power in qdBm

q1 = 4;
n = reshape(n, length(n), 1);
P = (reshape(P, length(P), 1)*q1);
P = round(P);

rho = ones(length(n), 3)*q1;
rho(:,2) = n*q1;
rho(:,3) = -n.*P;
rho = (rho./q1);
rho = round(rho);

C1 = rho' * rho;

a11 = C1(1,1); a12 = C1(1,2); a13 = C1(1,3);
a21 = C1(2,1); a22 = C1(2,2); a23 = C1(2,3);
a31 = C1(3,1); a32 = C1(3,2); a33 = C1(3,3);

C2_calc = [a22*a33 - a32*a23  a13*a32 - a12*a33  a12*a23 - a13*a22
		a23*a31 - a21*a33  a11*a33 - a13*a31  a13*a21 - a11*a23
		a21*a32 - a31*a22  a12*a31 - a11*a32  a11*a22 - a12*a21];

det_C1 = a11*a22*a33 + a12*a23*a31 + a13*a21*a32
		- a11*a23*a32 - a12*a21*a33 - a13*a22*a31;

C3 = C2_calc * rho';

C4 = C3 * P

C4 = round(C4./q1)

C = C4./det_C1;

b0=round(2^8*C(1)) ;
b1=round(2^12*C(2));
a1=round(2^15*C(3));

return;
*/
static void
ratmodel_paparams_fix64(ratmodel_paparams_t* rsd, int m)
{
	int i, j, n, q;
	int64 *a, temp;

	mat_rho((int64 *)(&rsd->n), (int64 *)(&rsd->p),
		(int64 *)(&rsd->rho), m);
	mat_print((int64 *)(&rsd->rho), m, 3, "rho");

	mat_transpose((int64 *)(&rsd->rho),
		(int64 *)(&rsd->rho_t), m, 3);
	mat_print((int64 *)(&rsd->rho_t), 3, m, "rho_t");

	mat_mult((int64*)(&rsd->rho_t), (int64 *)(&rsd->rho),
		(int64*)(&rsd->c1), 3, m, 3);
	mat_print((int64 *)(&rsd->c1), 3, 3, "c1");

	mat_inv_prod_det((int64 *)(&rsd->c1),
		(int64 *)(&rsd->c2_calc));
	mat_print((int64 *)(&rsd->c2_calc), 3, 3, "c2_calc");

	mat_det((int64 *)(&rsd->c1), (int64 *)(&rsd->det_c1));

#if PHY_TSSI_CAL_DBG_EN
	printf("\ndet_c1 = ");
	print_int64(&rsd->det_c1);
	printf("\n");
#endif

	mat_mult((int64*)(&rsd->c2_calc), (int64 *)(&rsd->rho_t),
		(int64*)(&rsd->c3), 3, 3, m);
	mat_print((int64 *)(&rsd->c3), 3, m, "c3");

	mat_mult((int64*)(&rsd->c3), (int64 *)(&rsd->p),
		(int64*)(&rsd->c4), 3, m, 1);

	m = 3; n = 1; q = 2;
	a = (int64*)(&rsd->c4);
	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++) {
			temp = *(a + (i * n) + j);
			temp = (temp + (int64)(1 << (q-1)));
			temp = temp >> q;
			*(a + (i * n) + j) = temp;
		}

	mat_print((int64 *)(&rsd->c4), 3, 1, "c4");
}

int
wlc_phy_tssi_cal(phy_info_t *pi)
{
	uint16 count;
	count = tssi_cal_sweep(pi);
	ratmodel_paparams_fix64(&pi->ptssi_cal->rsd, count);
	return 0;
}

static uint16
tssi_cal_sweep(phy_info_t *pi)
{

	uint16 i, k = 0;

	uint16 count = 0;
	int8 *des_pwr = NULL;
	uint8 *adj_tssi = NULL;
	int *sort_pwr = NULL, avg_pwr;
	uint8 *sort_pwr_cnt = NULL;
	int16 MIN_PWR = 32; /* 8dBm */
	int16 MAX_PWR = 72; /* 18dBm */

	int64* tssi = pi->ptssi_cal->rsd.n;
	int64* pwr = pi->ptssi_cal->rsd.p;

	des_pwr = (int8*)MALLOC(pi->sh->osh, sizeof(int8)* 80 * MAX_NUM_ANCHORS);
	if (des_pwr == NULL)
		goto cleanup;

	adj_tssi = (uint8*)MALLOC(pi->sh->osh, sizeof(uint8) * 80 * MAX_NUM_ANCHORS);
	if (adj_tssi == NULL)
		goto cleanup;

	sort_pwr = (int*)MALLOC(pi->sh->osh, sizeof(int)*128);
	if (sort_pwr == NULL)
		goto cleanup;

	sort_pwr_cnt = (uint8*)MALLOC(pi->sh->osh, sizeof(uint8)*128);
	if (sort_pwr_cnt == NULL)
		goto cleanup;

	if (pi->pi_fptr.tssicalsweep)
		count = (*pi->pi_fptr.tssicalsweep)(pi, des_pwr, adj_tssi);
	else
		goto cleanup;

	for (i = 0; i < 128; i++) {
		sort_pwr[i] = 0xffffffff;
		sort_pwr_cnt[i] = 0;
	}

	for (i = 0; i < count; i++) {
		if (sort_pwr[adj_tssi[i]] == 0xffffffff)
			sort_pwr[adj_tssi[i]] = des_pwr[i];
		else {
			sort_pwr[adj_tssi[i]] += des_pwr[i];
		}
		sort_pwr_cnt[adj_tssi[i]]++;
	}

	k = 0;
	for (i = 0; i < 128; i++) {
		if (sort_pwr[i] != 0xffffffff) {
			avg_pwr =  sort_pwr[i]/sort_pwr_cnt[i];
			if ((avg_pwr >= MIN_PWR) && (avg_pwr <= MAX_PWR)) {
				tssi[k] = (int64) i;
				pwr[k] =  (int64) avg_pwr;
				k++;
			}
		}
	}

#if PHY_TSSI_CAL_DBG_EN
	printf("TSSI\tPWR, k = %d\n", k);
	for (i = 0; i < k; i++) {
		print_int64(&tssi[i]);
		printf("\t\t");
		print_int64(&pwr[i]);
		printf("\n");
	}
#endif

cleanup:
	if (des_pwr)
		MFREE(pi->sh->osh, des_pwr, sizeof(int8) * 80 * MAX_NUM_ANCHORS);
	if (adj_tssi)
		MFREE(pi->sh->osh, adj_tssi, sizeof(uint8) * 80 * MAX_NUM_ANCHORS);
	if (sort_pwr)
		MFREE(pi->sh->osh, sort_pwr, sizeof(int)*128);
	if (sort_pwr_cnt)
		MFREE(pi->sh->osh, sort_pwr_cnt, sizeof(uint8)*128);

	return k;
}


/* a simple implementation of gcd(greatest common divisor)
 * assuming argument 1 is bigger than argument 2, both of them
 * are positive numbers.
 */
uint32
wlc_phy_gcd(uint32 bigger, uint32 smaller)
{
	uint32 remainder;

	do {
		remainder = bigger % smaller;
		if (remainder) {
			bigger = smaller;
			smaller = remainder;
		} else {
			return smaller;
		}
	} while (TRUE);
}

void
wlc_phy_get_paparams_for_band(phy_info_t *pi, int32 *a1, int32 *b0, int32 *b1)
{
	/* On lcnphy, estPwrLuts0/1 table entries are in S6.3 format */
	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
	case WL_CHAN_FREQ_RANGE_2G:
			/* 2.4 GHz */
			ASSERT((pi->txpa_2g[0] != -1) && (pi->txpa_2g[1] != -1) &&
				(pi->txpa_2g[2] != -1));
			*b0 = pi->txpa_2g[0];
			*b1 = pi->txpa_2g[1];
			*a1 = pi->txpa_2g[2];
			break;
#ifdef BAND5G
	case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			ASSERT((pi->txpa_5g_low[0] != -1) && (pi->txpa_5g_low[1] != -1) &&
				(pi->txpa_5g_low[2] != -1));
			*b0 = pi->txpa_5g_low[0];
			*b1 = pi->txpa_5g_low[1];
			*a1 = pi->txpa_5g_low[2];
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			ASSERT((pi->txpa_5g_mid[0] != -1) && (pi->txpa_5g_mid[1] != -1) &&
				(pi->txpa_5g_mid[2] != -1));
			*b0 = pi->txpa_5g_mid[0];
			*b1 = pi->txpa_5g_mid[1];
			*a1 = pi->txpa_5g_mid[2];
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			ASSERT((pi->txpa_5g_hi[0] != -1) && (pi->txpa_5g_hi[1] != -1) &&
				(pi->txpa_5g_hi[2] != -1));
			*b0 = pi->txpa_5g_hi[0];
			*b1 = pi->txpa_5g_hi[1];
			*a1 = pi->txpa_5g_hi[2];
			break;
#endif /* BAND5G */
		default:
			ASSERT(FALSE);
			break;
	}
	return;
}

/* --------------------------------------------- */
/* this will evetually be moved to lcncommon.c */
phy_info_lcnphy_t *
wlc_phy_getlcnphy_common(phy_info_t *pi)
{
	if (ISLCNPHY(pi))
		return pi->u.pi_lcnphy;
	else if (ISLCN40PHY(pi))
		return (phy_info_lcnphy_t *)pi->u.pi_lcn40phy;
	else {
		ASSERT(FALSE);
		return NULL;
	}
}

uint16
wlc_txpwrctrl_lcncommon(phy_info_t *pi)
{
	if (ISLCNPHY(pi))
		return LCNPHY_TX_PWR_CTRL_HW;
	else if (ISLCN40PHY(pi))
		return LCN40PHY_TX_PWR_CTRL_HW;
	else {
		ASSERT(FALSE);
		return 0;
	}
}


#ifdef ENABLE_FCBS

void wlc_phy_fcbs_exit(phy_info_t *pi)
{
	int chanidx;

	for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
		pi->phy_fcbs.initialized[chanidx] = FALSE;
	}
}

bool wlc_phy_fcbs_init(phy_info_t *pi, int chanidx)
{
	int offset, bphy_offset;
	int len, bphy_len;
	fcbsinitfn_t fcbs_init = NULL;
	uint32 gpio4_mask_val = 0x10000; /* CCTRL4331_BT_SHD0_ON_GPIO4 */

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

	/* Chan A has to be initialized before Chan B */
	if ((chanidx == FCBS_CHAN_B) && (!pi->phy_fcbs.initialized[FCBS_CHAN_A])) {
		return FALSE;
	}

	if (chanidx == FCBS_CHAN_A) {
		si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		           gpio4_mask_val, 0);
		wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_BPHYCTRL, 0);
	}

	PHY_FCBS(("Entering wlc_phy_fcbs_init: chanidx=%d\n", chanidx));

	pi->phy_fcbs.chanspec[chanidx] = pi->radio_chanspec;

	/* If this flag is TRUE, the radio tuning table values in driver's memory is
	 * used while storing the radio registers in the template RAM. If the flag
	 * is FALSE, the radio tuning registers are explicitly read from the chip
	 * before storing the values in the template RAM
	 */
	pi->phy_fcbs.use_rftuning_mem = FALSE;

	/* Get the pointer to the PHY specfic FCBS init function */
	fcbs_init = pi->pi_fptr.fcbsinit;

	if (fcbs_init) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		wlc_phyreg_enter((wlc_phy_t *)pi);
		/* Read the PHY/Radio regs and tables into memory buffers */

		pi->phy_fcbs.initialized[chanidx] = (*fcbs_init)(pi, chanidx, pi->radio_chanspec);
		wlc_phyreg_exit((wlc_phy_t *)pi);

		if (pi->phy_fcbs.initialized[chanidx] == TRUE) {
			pi->phy_fcbs.curr_fcbs_chan = chanidx;

			offset = pi->phy_fcbs.start_template_offset[chanidx];
			len = pi->phy_fcbs.num_radio_regs * sizeof(uint32);
			pi->phy_fcbs.radioreg_template_offset[chanidx] = offset;
			wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
			    len, pi->phy_fcbs.radioreg_buf[chanidx]);
			wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_RADIOREG, (uint16)len);
			PHY_FCBS(("radio reg buf: start offset=%d, len=%d\n", offset, len));

			offset += len;
			len = pi->phy_fcbs.phytbl16_buflen;
			pi->phy_fcbs.phytbl16_template_offset[chanidx] = offset;
			/* buffer has to end at 4-byte boundary in the template RAM */
			if (((len/4) * 4) != len) {
				len += 2;
			}
			wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
			    len, pi->phy_fcbs.phytbl16_buf[chanidx]);
			wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_PHYTBL16, (uint16)len);
			PHY_FCBS(("phytbl16: start offset = %d, len = %d\n", offset, len));

			offset += len;
			len = pi->phy_fcbs.phytbl32_buflen;
			pi->phy_fcbs.phytbl32_template_offset[chanidx] = offset;
			wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
			    len, pi->phy_fcbs.phytbl32_buf[chanidx]);
			wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_PHYTBL32, (uint16)len);
			PHY_FCBS(("phytbl32: start offset = %d, len = %d\n", offset, len));

			offset += len;
			len = pi->phy_fcbs.num_phy_regs * sizeof(uint32);
			pi->phy_fcbs.phyreg_template_offset[chanidx] = offset;
			wlapi_bmac_write_template_ram(pi->sh->physhim, offset,
			    len, pi->phy_fcbs.phyreg_buf[chanidx]);

			if (pi->phy_fcbs.num_bphy_regs[chanidx] > 0) {
				bphy_offset = offset + len;
				bphy_len = pi->phy_fcbs.num_bphy_regs[chanidx] * sizeof(uint32);
				pi->phy_fcbs.bphyreg_template_offset[chanidx] = bphy_offset;
				wlapi_bmac_write_template_ram(pi->sh->physhim, bphy_offset,
				    bphy_len, pi->phy_fcbs.bphyreg_buf[chanidx]);
				len += bphy_len;
			}

			pi->phy_fcbs.phyreg_buflen[chanidx] = len;
			wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_PHYREG, (uint16)len);
			PHY_FCBS(("PHY reg buf: start offset=%d, len = %d\n", offset, len));

			if (chanidx == FCBS_CHAN_A) {
				pi->phy_fcbs.start_template_offset[FCBS_CHAN_B] = offset + len;
			}

			wlapi_enable_mac(pi->sh->physhim);
		}
	} else {
		pi->phy_fcbs.initialized[chanidx] = FALSE;
	}

	return TRUE;
}

int wlc_phy_fcbs(phy_info_t *pi, int chanidx, bool set)
{
	fcbspostfn_t post_fcbs = NULL;
	unsigned long tick1, tick2, tick_diff;
	int retval = -1;

	ASSERT((chanidx >= 0) && (chanidx < MAX_FCBS_CHANS));

	if (set && pi->phy_fcbs.initialized[chanidx]) {
		if (pi->phy_fcbs.curr_fcbs_chan != chanidx) {

			PHY_FCBS(("wlc_phy_fcbs: About to suspend MAC chanidx = %d\n", chanidx));

			PHY_FCBS(("template offset = %d\n",
			    pi->phy_fcbs.start_template_offset[chanidx] >> 2));

			OSL_GETCYCLES(tick1);


			if (CHSPEC_IS2G(pi->phy_fcbs.chanspec[pi->phy_fcbs.curr_fcbs_chan])) {
				if (CHSPEC_IS5G(pi->phy_fcbs.chanspec[chanidx])) {
					wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_BPHYCTRL,
					    FCBS_BPHY_OFF);
				}
			} else {
				if (CHSPEC_IS2G(pi->phy_fcbs.chanspec[chanidx])) {
					wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_BPHYCTRL,
					    FCBS_BPHY_ON);
				}
			}

			pi->phy_fcbs.curr_fcbs_chan = chanidx;

			if (pi->phy_fcbs.phyreg_buflen[FCBS_CHAN_A] !=
			                            pi->phy_fcbs.phyreg_buflen[FCBS_CHAN_B]) {
				wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_PHYREG,
				    pi->phy_fcbs.phyreg_buflen[chanidx]);
			}

			wlapi_bmac_write_shm(pi->sh->physhim, M_FCBS_TEMPLATE_PTR,
			    (uint16) (pi->phy_fcbs.start_template_offset[chanidx] >> 2));

			wlapi_suspend_mac_and_wait(pi->sh->physhim);

			if ((post_fcbs = pi->pi_fptr.postfcbs)) {
				(*post_fcbs)(pi, chanidx);
			}


			OSL_GETCYCLES(tick2);
			tick_diff = tick2 - tick1;

			PHY_FCBS(("wlc_phy_fcbs: MAC completed FCBS, tick_diff = %ld\n",
			    tick_diff));
			wlapi_enable_mac(pi->sh->physhim);
			retval = chanidx;
		}
	} else {
		retval = pi->phy_fcbs.curr_fcbs_chan;
	}

	return retval;
}
#endif /* ENABLE_FCS */

void
wlc_phy_get_SROMnoiselvl_phy(phy_info_t *pi, int8 *noiselvl)
{
	/* Returns noise level (read from srom) for current channel */
	uint8 core;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 14) {
		/* 2G */
		FOREACH_CORE(pi, core) {
			noiselvl[core] = pi->srom_noiselvl_2g[core];
		}
	} else {
		/* 5G */
		if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 48) {
			/* 5G-low: channels 36 through 48 */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->srom_noiselvl_5gl[core];
			}
		} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 64) {
			/* 5G-mid: channels 52 through 64 */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->srom_noiselvl_5gm[core];
			}
		} else if (CHSPEC_CHANNEL(pi->radio_chanspec) <= 128) {
			/* 5G-high: channels 100 through 128 */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->srom_noiselvl_5gh[core];
			}
		} else {
			/* 5G-upper: channels 132 and above */
			FOREACH_CORE(pi, core) {
				noiselvl[core] = pi->srom_noiselvl_5gu[core];
			}
		}
	}
}

#define PHY_TEMPSENSE_MIN 0
#define PHY_TEMPSENSE_MAX 105
void
wlc_phy_upd_gain_wrt_temp_phy(phy_info_t *pi, int16 *gain_err_temp_adj)
{
	*gain_err_temp_adj = 0;

	/* now, adjust for temperature */
	if ((ISNPHY(pi) &&
	     (NREV_GE(pi->pubpi.phy_rev, 3) &&
	      NREV_LE(pi->pubpi.phy_rev, 6))) || ISHTPHY(pi)) {
		/* read in the temperature */
		int16 temp_diff, curr_temp = 0, gain_temp_slope = 0;

		/* make sure mac is suspended before calling tempsense */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		if (ISNPHY(pi))
			curr_temp = wlc_phy_tempsense_nphy(pi);
		else if (ISHTPHY(pi))
			curr_temp = wlc_phy_tempsense_htphy(pi);
		wlapi_enable_mac(pi->sh->physhim);

		curr_temp = MIN(MAX(curr_temp, PHY_TEMPSENSE_MIN), PHY_TEMPSENSE_MAX);

		/* check that non programmed SROM for cal temp are not changed */
		if (pi->srom_rawtempsense != 255) {
			temp_diff = curr_temp - pi->srom_rawtempsense;
		} else {
			temp_diff = 0;
		}

		/* adjust gain based on the temperature difference now vs. calibration time:
		 * make gain diff rounded to nearest 0.5 dbm, where 1 tick is 0.5 dbm
		 */
		if (ISNPHY(pi)) {
			gain_temp_slope = CHSPEC_IS2G(pi->radio_chanspec) ?
			        NPHY_GAIN_VS_TEMP_SLOPE_2G : NPHY_GAIN_VS_TEMP_SLOPE_5G;
		} else if (ISHTPHY(pi)) {
			gain_temp_slope = CHSPEC_IS2G(pi->radio_chanspec) ?
			        HTPHY_GAIN_VS_TEMP_SLOPE_2G : HTPHY_GAIN_VS_TEMP_SLOPE_5G;
		}

		if (temp_diff >= 0) {
			*gain_err_temp_adj = (temp_diff * gain_temp_slope + 25)/50;
		} else {
			*gain_err_temp_adj = (temp_diff * gain_temp_slope - 25)/50;
		}
	}
}
