/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abgn
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
 * $Id: wlc_phy_lcn40.c 280777 2011-08-31 03:35:43Z yesun $*
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
#include <wlc_phy_radio.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phy_lcn40.h>
#include <sbchipc.h>
#include <wlc_phyreg_lcn40.h>
#include <wlc_phytbl_lcn40.h>

/* contains settings from BCM2064_JTAG.xls */
#define PLL_2065_NDIV		65
#define PLL_2065_KVCO       50
#define PLL_2065_LOW_END_VCO 	3200
#define PLL_2065_LOW_END_KVCO_Q16 	1514349
#define PLL_2065_HIGH_END_VCO	4000
#define PLL_2065_HIGH_END_KVCO_Q16  3146447
#define PLL_2065_LOOP_BW_DESIRED	300
#define PLL_2065_LOOP_BW	266
#define PLL_2065_LF_R1     4250
#define PLL_2065_LF_R2     2000
#define PLL_2065_LF_R3     2000
#define PLL_2065_LF_C1     408
#define PLL_2065_LF_C2_X10     272
#define PLL_2065_LF_C3_X100     816
#define PLL_2065_LF_C4_X100     816
#define PLL_2065_CP_CURRENT     384
#define PLL_2065_D35        408
#define PLL_2065_D32        5178

#define TEMPSENSE 			1
#define VBATSENSE           2

#define PAPD_BLANKING_PROFILE 		0
#define PAPD2LUT			0
#define PAPD_CORR_NORM 			0
#define PAPD_NUM_SKIP_COUNT 39
#define PAPD_BLANKING_THRESHOLD 	0
#define PAPD_STOP_AFTER_LAST_UPDATE	0

#define LCN40_TARGET_TSSI  30
#define LCN40_TARGET_PWR  60

#define LCN40_VBAT_OFFSET_433X 34649679 /* 528.712121 * (2 ^ 16) */
#define LCN40_VBAT_SLOPE_433X  8258032 /* 126.007576 * (2 ^ 16) */

#define LCN40_VBAT_SCALE_NOM  53	/* 3.3 * (2 ^ 4) */
#define LCN40_VBAT_SCALE_DEN  432

#define LCN40_TEMPSENSE_OFFSET  80812 /* 78.918 << 10 */
#define LCN40_TEMPSENSE_DEN  2647	/* 2.5847 << 10 */

#define IDLE_TSSI_PER_CAL_EN 1

#define TWO_POWER_RANGE_TXPWR_CTRL 1

#define LCN40_BCK_TSSI_UCODE_POST 1
#define LCN40_BCK_TSSI_DEBUG_LUT_DIRECT !LCN40_BCK_TSSI_UCODE_POST
#if LCN40_BCK_TSSI_DEBUG_LUT_DIRECT
#define LCN40_BCK_TSSI_DEBUG_VIA_DUMMY 0
#endif
#define LCN40_BCK_TSSI_DEBUG 0

/* %%%%%% LCN40PHY macros/structure */

#define LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT \
	(LCN40PHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT + 8)
#define LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_MASK \
	(0xff << LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT)

#define LCN40PHY_stxtxgainctrlovrval1_pagain_ovr_val1_SHIFT \
	(LCN40PHY_stxtxgainctrlovrval1_stxtxgainctrl_ovr_val1_SHIFT + 8)
#define LCN40PHY_stxtxgainctrlovrval1_pagain_ovr_val1_MASK \
	(0x7f << LCN40PHY_stxtxgainctrlovrval1_pagain_ovr_val1_SHIFT)

#define wlc_lcn40phy_enable_tx_gain_override(pi) \
	wlc_lcn40phy_set_tx_gain_override(pi, TRUE)
#define wlc_lcn40phy_disable_tx_gain_override(pi) \
	wlc_lcn40phy_set_tx_gain_override(pi, FALSE)

/* Turn off all the crs signals to the MAC */
#define wlc_lcn40phy_iqcal_active(pi)	\
	(phy_reg_read((pi), LCN40PHY_iqloCalCmd) & \
	(LCN40PHY_iqloCalCmd_iqloCalCmd_MASK | LCN40PHY_iqloCalCmd_iqloCalDFTCmd_MASK))

#define txpwrctrl_off(pi) (0x7 != ((phy_reg_read(pi, LCN40PHY_TxPwrCtrlCmd) & 0xE000) >> 13))
#define wlc_lcn40phy_tempsense_based_pwr_ctrl_enabled(pi) \
	(pi->temppwrctrl_capable)
#define wlc_lcn40phy_tssi_based_pwr_ctrl_enabled(pi) \
	(pi->hwpwrctrl_capable)

#define wlc_radio_2065_rc_cal_done(pi) (0 != (read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC2) & 0x10))
#define wlc_radio_2065_rcal_done(pi) (0 != (read_radio_reg(pi, RADIO_2065_RCAL_CFG) & 0x8))
#define wlc_radio_2065_minipmu_cal_done(pi) (0 != (read_radio_reg(pi, RADIO_2065_PMU_STAT) & 0x1))
/* swctrl table */
#define LCN40PHY_I_WL_TX 0 /* PA1 PA0 Tx1 TX0 */
#define LCN40PHY_I_WL_RX 1 /* eLNARx1 eLNARx0 Rx1 RX0 */
#define LCN40PHY_I_WL_RX_ATTN 2 /* eLNAAttnRx1 eLNAAttnRx0 AttnRx1 AttnRx0 */
#define LCN40PHY_I_BT 3 /* Tx eLNARx Rx */
#define LCN40PHY_I_WL_MASK 4 /* ant(1 bit) ovr_en(1 bit) tdm(1 bit) mask(8 bits) */

#define LCN40PHY_MASK_BT_RX	0xff
#define LCN40PHY_MASK_BT_ELNARX	0xff00
#define LCN40PHY_SHIFT_BT_ELNARX	8
#define LCN40PHY_MASK_BT_TX	0xff0000
#define LCN40PHY_SHIFT_BT_TX	16

#define LCN40PHY_MASK_WL_MASK	0xff
#define LCN40PHY_MASK_TDM	0x100
#define LCN40PHY_MASK_OVR_EN	0x200
#define LCN40PHY_MASK_ANT	0x400

#define LCN40PHY_SW_CTRL_MAP_ANT 0x1
#define LCN40PHY_SW_CTRL_MAP_WL_RX 0x2
#define LCN40PHY_SW_CTRL_MAP_WL_TX 0x4
#define LCN40PHY_SW_CTRL_MAP_BT_TX 0x8
#define LCN40PHY_SW_CTRL_MAP_BT_PRIO 0x10
#define LCN40PHY_SW_CTRL_MAP_ELNA 0x20

#define LCN40PHY_SW_CTRL_TBL_LENGTH	64
#define LCN40PHY_SW_CTRL_TBL_WIDTH	16

#define LCN40PHY_DIV_BASED_INIT_H(ant, swidx) \
	(ant)?(swctrlmap[swidx] >> 24):((swctrlmap[swidx] & 0xff0000) >> 16)

#define LCN40PHY_DIV_BASED_INIT_L(ant, swidx) \
	(ant)?((swctrlmap[swidx] & 0xff00) >> 8):(swctrlmap[swidx] & 0xff)

#define SWCTRL_BT_TX		0x18
#define SWCTRL_OVR_DISABLE	0x40

#define	AFE_CLK_INIT_MODE_TXRX2X	1
#define	AFE_CLK_INIT_MODE_PAPD		0

#define LCN40PHY_TBL_ID_IQLOCAL			0x00
#define LCN40PHY_TBL_ID_TXPWRCTL 		0x07
#define LCN40PHY_TBL_ID_RFSEQ         0x08
#define LCN40PHY_TBL_ID_GAIN_IDX		0x0d
#define LCN40PHY_TBL_ID_SW_CTRL			0x0f
#define LCN40PHY_TBL_ID_NF_CTRL			0x10
#define LCN40PHY_TBL_ID_GAIN_TBL		0x12
#define LCN40PHY_TBL_ID_SPUR			0x14
#define LCN40PHY_TBL_ID_SAMPLEPLAY		0x15
#define LCN40PHY_TBL_ID_SAMPLEPLAY1		0x1b
#define LCN40PHY_TBL_ID_PAPDCOMPDELTATBL	0x18

#define LCN40PHY_TX_PWR_CTRL_RATE_OFFSET 	832
#define LCN40PHY_TX_PWR_CTRL_MAC_OFFSET 	128
#define LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET 	192
#define LCN40PHY_TX_PWR_CTRL_IQ_OFFSET		320
#define LCN40PHY_TX_PWR_CTRL_LO_OFFSET		448
#define LCN40PHY_TX_PWR_CTRL_PWR_OFFSET		576
#define LCN40PHY_TX_PWR_CTRL_EST_PWR_OFFSET	704

#define LCN40PHY_TX_PWR_CTRL_START_NPT		1
#define LCN40PHY_TX_PWR_CTRL_MAX_NPT			7

#define PHY_NOISE_SAMPLES_DEFAULT 5000

#define LCN40PHY_ACI_DETECT_START      1
#define LCN40PHY_ACI_DETECT_PROGRESS   2
#define LCN40PHY_ACI_DETECT_STOP       3

#define LCN40PHY_ACI_CRSHIFRMLO_TRSH 100
#define LCN40PHY_ACI_GLITCH_TRSH 2000
#define	LCN40PHY_ACI_TMOUT 250		/* Time for CRS HI and FRM LO (in micro seconds) */
#define LCN40PHY_ACI_DETECT_TIMEOUT  2	/* in  seconds */
#define LCN40PHY_ACI_START_DELAY 0

#define LCN40PHY_MAX_CAL_CACHE	   2	/* Max number of cal cache contexts reqd */

#define wlc_lcn40phy_tx_gain_override_enabled(pi) \
	(0 != (phy_reg_read((pi), LCN40PHY_AfeCtrlOvr) & LCN40PHY_AfeCtrlOvr_dacattctrl_ovr_MASK))

#define LCN40PHY_TSSI_CAL_DBG_EN 0

/* Init values for 2065 regs (autogenerated by 2065_regs_tcl2c.tcl)
 *   Entries: addr, init value A, init value G, do_init A, do_init G.
 *   Last line (addr FF is dummy as delimiter. This table has dual use
 *   between dumping and initializing.
 */
#if defined(BCMDBG) || defined(WLTEST)
lcn40phy_radio_regs_t lcn40phy_radio_regs_2067[] = {
{ 0x00,             0,             0,   0,   0  },
{ 0x01,             0,        0x2065,   1,   0  },
{ 0x02,             0,        0x8380,   1,   0  },
{ 0x03,             0,        0x1414,   1,   0  },
{ 0x04,             0,        0x3838,   1,   0  },
{ 0x05,             0,        0x1937,   1,   0  },
{ 0x06,             0,             0,   0,   0  },
{ 0x07,             0,         0x210,   1,   0  },
{ 0x08,             0,             0,   0,   0  },
{ 0x09,             0,             0,   0,   0  },
{ 0x0A,             0,             0,   0,   0  },
{ 0x0B,             0,             0,   0,   0  },
{ 0x0C,             0,             0,   0,   0  },
{ 0x0D,             0,             0,   0,   0  },
{ 0x0E,             0,           0x1,   1,   0  },
{ 0x0F,             0,             0,   0,   0  },
{ 0x10,             0,             0,   0,   0  },
{ 0x11,             0,         0xf80,   1,   0  },
{ 0x12,             0,         0xf54,   1,   0  },
{ 0x13,             0,             0,   0,   0  },
{ 0x14,             0,             0,   0,   0  },
{ 0x15,             0,             0,   0,   0  },
{ 0x16,             0,             0,   0,   0  },
{ 0x17,             0,             0,   0,   0  },
{ 0x18,             0,             0,   0,   0  },
{ 0x19,             0,             0,   0,   0  },
{ 0x1A,             0,             0,   0,   0  },
{ 0x1B,             0,             0,   0,   0  },
{ 0x1C,             0,          0x14,   1,   0  },
{ 0x1D,             0,         0x280,   1,   0  },
{ 0x1E,             0,        0x1558,   1,   0  },
{ 0x1F,             0,          0x14,   1,   0  },
{ 0x20,             0,          0x14,   1,   0  },
{ 0x21,             0,             0,   0,   0  },
{ 0x22,             0,             0,   0,   0  },
{ 0x23,             0,             0,   0,   0  },
{ 0x24,             0,         0x200,   1,   0  },
{ 0x25,             0,         0x180,   1,   0  },
{ 0x26,             0,        0x5008,   1,   0  },
{ 0x27,             0,        0x3600,   1,   0  },
{ 0x28,             0,          0x77,   1,   0  },
{ 0x29,             0,          0x42,   1,   0  },
{ 0x2A,             0,          0x88,   1,   0  },
{ 0x2B,             0,        0x6060,   1,   0  },
{ 0x2C,             0,             0,   0,   0  },
{ 0x2D,             0,        0x3404,   1,   0  },
{ 0x2E,             0,         0x330,   1,   0  },
{ 0x2F,             0,             0,   0,   0  },
{ 0x30,             0,          0x74,   1,   0  },
{ 0x31,             0,        0x757c,   1,   1  },
{ 0x32,             0,        0x6060,   1,   0  },
{ 0x33,             0,         0x110,   1,   0  },
{ 0x34,             0,         0x140,   1,   0  },
{ 0x35,             0,          0x88,   1,   0  },
{ 0x36,             0,         0x140,   1,   0  },
{ 0x37,             0,          0x88,   1,   0  },
{ 0x38,             0,             0,   0,   0  },
{ 0x39,             0,             0,   0,   0  },
{ 0x3A,             0,             0,   0,   0  },
{ 0x3B,             0,             0,   0,   0  },
{ 0x3C,             0,             0,   0,   0  },
{ 0x3D,             0,             0,   0,   0  },
{ 0x3E,             0,        0x4420,   1,   0  },
{ 0x3F,             0,        0x4842,   1,   1  },
{ 0x40,             0,           0xc,   1,   0  },
{ 0x41,             0,        0x6666,   1,   0  },
{ 0x42,             0,        0x8100,   1,   0  },
{ 0x43,             0,         0x478,   1,   1  },
{ 0x44,             0,             0,   0,   0  },
{ 0x45,             0,           0x3,   1,   0  },
{ 0x46,             0,             0,   0,   0  },
{ 0x47,             0,          0x44,   1,   0  },
{ 0x48,             0,         0x400,   1,   1  },
{ 0x49,             0,         0x400,   1,   1  },
{ 0x4A,             0,        0x4c00,   1,   0  },
{ 0x4B,             0,             0,   0,   0  },
{ 0x4C,             0,             0,   0,   0  },
{ 0x4D,             0,             0,   0,   0  },
{ 0x4E,             0,           0x2,   1,   0  },
{ 0x4F,             0,        0x3000,   0,   1  },
{ 0x50,             0,         0x303,   0,   1  },
{ 0x51,             0,        0x36e2,   0,   1  },
{ 0x52,             0,             0,   0,   0  },
{ 0x53,             0,        0x6000,   0,   1  },
{ 0x54,             0,           0x3,   0,   1  },
{ 0x55,             0,             0,   0,   0  },
{ 0x56,             0,             0,   0,   0  },
{ 0x57,             0,             0,   0,   0  },
{ 0x58,             0,        0x2370,   1,   0  },
{ 0x59,             0,        0xffff,   1,   0  },
{ 0x5A,             0,         0x26f,   1,   1  },
{ 0x5B,             0,         0x5af,   1,   1  },
{ 0x5C,             0,          0x77,   1,   0  },
{ 0x5D,             0,          0x77,   1,   0  },
{ 0x5E,             0,          0x77,   1,   0  },
{ 0x5F,             0,          0x77,   1,   0  },
{ 0x60,             0,          0x5e,   1,   1  },
{ 0x61,             0,         0x335,   1,   1  },
{ 0x62,             0,        0x1616,   1,   1  },
{ 0x63,             0,         0xb76,   1,   1  },
{ 0x64,             0,         0x7a0,   1,   1  },
{ 0x65,             0,        0x4000,   1,   0  },
{ 0x66,             0,        0x1515,   1,   1  },
{ 0x67,             0,          0x77,   1,   1  },
{ 0x68,             0,        0x4f32,   1,   1  },
{ 0x69,             0,         0x70b,   1,   0  },
{ 0x6A,             0,          0x77,   0,   1  },
{ 0x6B,             0,          0x75,   1,   1  },
{ 0x6C,             0,        0x7777,   1,   1  },
{ 0x6D,             0,        0xd032,   1,   0  },
{ 0x6E,             0,        0x780a,   1,   1  },
{ 0x6F,             0,             0,   0,   0  },
{ 0x70,             0,         0xac5,   1,   0  },
{ 0x71,             0,        0x7777,   1,   1  },
{ 0x72,             0,             0,   0,   0  },
{ 0x73,             0,             0,   0,   0  },
{ 0x74,             0,             0,   0,   0  },
{ 0x75,             0,             0,   0,   0  },
{ 0x76,             0,             0,   0,   0  },
{ 0x77,             0,             0,   0,   0  },
{ 0x78,             0,             0,   0,   0  },
{ 0x79,             0,             0,   0,   0  },
{ 0x7A,             0,             0,   0,   0  },
{ 0x7B,             0,        0x1f20,   1,   0  },
{ 0x7C,             0,        0x7e28,   1,   0  },
{ 0x7D,             0,           0x4,   1,   0  },
{ 0x7E,             0,        0x7002,   1,   0  },
{ 0x7F,             0,          0x80,   0,   1  },
{ 0x80,             0,             0,   0,   0  },
{ 0x81,             0,             0,   0,   0  },
{ 0x82,             0,        0x1240,   1,   0  },
{ 0x83,             0,         0x750,   1,   0  },
{ 0x84,         0xadd,             0,   1,   0  },
{ 0x85,         0xadd,             0,   1,   0  },
{ 0x86,             0,           0xf,   1,   0  },
{ 0x87,             0,        0x300a,   1,   0  },
{ 0x88,             0,          0x20,   1,   0  },
{ 0x89,             0,             0,   0,   0  },
{ 0x8A,             0,             0,   0,   0  },
{ 0x8B,             0,             0,   0,   0  },
{ 0x8C,             0,             0,   0,   0  },
{ 0x8D,             0,         0x580,   1,   1  },
{ 0x8E,             0,         0x877,   1,   0  },
{ 0x8F,             0,         0xc30,   1,   0  },
{ 0x90,             0,         0x200,   1,   0  },
{ 0x91,             0,           0x4,   1,   0  },
{ 0x92,             0,             0,   0,   0  },
{ 0x93,             0,           0x1,   1,   0  },
{ 0x94,             0,        0x5c60,   1,   0  },
{ 0x95,             0,        0x5c02,   1,   0  },
{ 0x96,             0,           0x9,   1,   0  },
{ 0x97,             0,        0x4000,   1,   0  },
{ 0x98,             0,         0x40a,   1,   0  },
{ 0x99,             0,             0,   0,   0  },
{ 0x9A,             0,             0,   0,   0  },
{ 0x9B,             0,             0,   0,   0  },
{ 0x9C,             0,             0,   0,   0  },
{ 0x9D,             0,        0x11c0,   1,   0  },
{ 0x9E,             0,        0x5540,   1,   1  },
{ 0x9F,             0,         0x430,   1,   0  },
{ 0xA0,             0,         0x430,   1,   0  },
{ 0xA1,             0,             0,   0,   0  },
{ 0xA2,             0,        0x7733,   1,   1  },
{ 0xA3,             0,        0x7734,   1,   1  },
{ 0xA4,             0,        0x7777,   1,   1  },
{ 0xA5,             0,          0xaa,   1,   0  },
{ 0xA6,             0,         0x777,   1,   0  },
{ 0xA7,             0,             0,   0,   0  },
{ 0xA8,             0,             0,   0,   0  },
{ 0xA9,             0,             0,   0,   0  },
{ 0xAA,             0,          0x85,   1,   0  },
{ 0xAB,             0,        0x2000,   1,   0  },
{ 0xAC,             0,             0,   0,   0  },
{ 0xAD,             0,           0x2,   1,   0  },
{ 0xAE,             0,             0,   0,   0  },
{ 0xAF,             0,        0x8888,   1,   0  },
{ 0xB0,             0,        0x8888,   1,   0  },
{ 0xB1,             0,        0x8888,   1,   0  },
{ 0xB2,             0,        0x8888,   1,   0  },
{ 0xB3,             0,             0,   0,   0  },
{ 0xB4,             0,             0,   0,   0  },
{ 0xB5,             0,             0,   0,   0  },
{ 0xB6,             0,             0,   0,   0  },
{ 0xB7,             0,             0,   0,   0  },
{ 0xB8,             0,             0,   0,   0  },
{ 0xB9,             0,             0,   0,   0  },
{ 0xBA,             0,             0,   0,   0  },
{ 0xBB,             0,             0,   0,   0  },
{ 0xBC,             0,             0,   0,   0  },
{ 0xBD,             0,             0,   0,   0  },
{ 0xBE,             0,             0,   0,   0  },
{ 0xBF,             0,             0,   0,   0  },
{ 0xC0,             0,             0,   0,   0  },
{ 0xC1,             0,             0,   0,   0  },
{ 0xC2,             0,             0,   0,   0  },
{ 0xC3,             0,             0,   0,   0  },
{ 0xC4,             0,             0,   0,   0  },
{ 0xC5,             0,             0,   0,   0  },
{ 0xC6,             0,             0,   0,   0  },
{ 0xC7,             0,             0,   0,   0  },
{ 0xC8,             0,             0,   0,   0  },
{ 0xC9,             0,        0x8000,   1,   0  },
{ 0xCA,             0,             0,   0,   0  },
{ 0xCB,             0,             0,   0,   0  },
{ 0xCC,             0,             0,   0,   0  },
{ 0xCD,             0,             0,   0,   0  },
{ 0xCE,             0,             0,   0,   0  },
{ 0xCF,             0,             0,   0,   0  },
{ 0xD0,             0,             0,   0,   0  },
{ 0xD1,             0,             0,   0,   0  },
{ 0xD2,             0,             0,   0,   0  },
{ 0xD3,             0,             0,   0,   0  },
{ 0xD4,             0,           0x7,   1,   0  },
{ 0xD5,             0,        0x6c9b,   1,   0  },
{ 0xD6,             0,             0,   0,   0  },
{ 0xD7,             0,          0x44,   1,   0  },
{ 0xFFFF,             0,             0,   0,   0  },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2065[] = {
	{ 0x00,           0x0,           0x0,   0,   0  },
	{ 0x01,           0x0,        0x2065,   1,   0  },
	{ 0x02,           0x0,        0x8380,   1,   0  },
	{ 0x03,           0x0,        0x1414,   1,   0  },
	{ 0x04,           0x0,        0x3838,   1,   0  },
	{ 0x05,           0x0,        0x1937,   1,   0  },
	{ 0x06,           0x0,           0x0,   0,   0  },
	{ 0x07,           0x0,         0x310,   1,   1  },
	{ 0x08,           0x0,           0x0,   0,   0  },
	{ 0x09,           0x0,           0x0,   0,   0  },
	{ 0x0A,           0x0,           0x0,   0,   0  },
	{ 0x0B,           0x0,           0x0,   0,   0  },
	{ 0x0C,           0x0,           0x0,   0,   0  },
	{ 0x0D,           0x0,           0x0,   0,   0  },
	{ 0x0E,           0x0,           0x1,   1,   0  },
	{ 0x0F,           0x0,           0x0,   0,   0  },
	{ 0x10,           0x0,           0x0,   0,   0  },
	{ 0x11,           0x0,         0xf80,   1,   0  },
	{ 0x12,           0x0,         0xf54,   1,   0  },
	{ 0x13,           0x0,           0x0,   0,   0  },
	{ 0x14,           0x0,           0x0,   0,   0  },
	{ 0x15,           0x0,           0x0,   0,   0  },
	{ 0x16,           0x0,           0x0,   0,   0  },
	{ 0x17,           0x0,           0x0,   0,   0  },
	{ 0x18,           0x0,           0x0,   0,   0  },
	{ 0x19,           0x0,           0x0,   0,   0  },
	{ 0x1A,           0x0,           0x0,   0,   0  },
	{ 0x1B,           0x0,           0x0,   0,   0  },
	{ 0x1C,           0x0,          0x14,   1,   0  },
	{ 0x1D,           0x0,         0x280,   1,   0  },
	{ 0x1E,           0x0,        0x1558,   1,   0  },
	{ 0x1F,           0x0,          0x14,   1,   0  },
	{ 0x20,           0x0,          0x14,   1,   0  },
	{ 0x21,           0x0,           0x0,   0,   0  },
	{ 0x22,           0x0,           0x0,   0,   0  },
	{ 0x23,           0x0,           0x0,   0,   0  },
	{ 0x24,           0x0,         0x200,   1,   0  },
	{ 0x25,           0x0,         0x180,   1,   0  },
	{ 0x26,           0x0,        0x5000,   1,   1  },
	{ 0x27,           0x0,        0x3600,   1,   0  },
	{ 0x28,           0x0,         0xf77,   1,   1  },
	{ 0x29,           0x0,          0x42,   1,   0  },
	{ 0x2A,           0x0,          0x88,   1,   0  },
	{ 0x2B,           0x0,        0x6060,   1,   0  },
	{ 0x2C,           0x0,           0x0,   0,   0  },
	{ 0x2D,           0x0,        0x3404,   1,   0  },
	{ 0x2E,           0x0,         0x330,   1,   0  },
	{ 0x2F,           0x0,           0x0,   0,   0  },
	{ 0x30,           0x0,          0x74,   1,   0  },
	{ 0x31,           0x0,        0x7774,   1,   0  },
	{ 0x32,           0x0,        0x6060,   1,   0  },
	{ 0x33,           0x0,         0x110,   1,   0  },
	{ 0x34,           0x0,         0x140,   1,   0  },
	{ 0x35,           0x0,          0x88,   1,   0  },
	{ 0x36,           0x0,         0x140,   1,   0  },
	{ 0x37,           0x0,          0x88,   1,   0  },
	{ 0x38,           0x0,           0x0,   0,   0  },
	{ 0x39,           0x0,           0xc,   0,   1  },
	{ 0x3A,           0x0,           0x0,   0,   0  },
	{ 0x3B,           0x0,           0x0,   0,   0  },
	{ 0x3C,           0x0,           0x0,   0,   0  },
	{ 0x3D,           0x0,           0x0,   0,   0  },
	{ 0x3E,           0x0,        0x4420,   1,   0  },
	{ 0x3F,           0x0,         0x842,   1,   0  },
	{ 0x40,           0x0,           0xc,   1,   0  },
	{ 0x41,           0x0,        0x6666,   1,   0  },
	{ 0x42,           0x0,        0x8100,   1,   0  },
	{ 0x43,           0x0,         0xc78,   1,   0  },
	{ 0x44,           0x0,           0x0,   0,   0  },
	{ 0x45,           0x0,           0x3,   1,   0  },
	{ 0x46,           0x0,           0x0,   0,   0  },
	{ 0x47,           0x0,          0x44,   1,   0  },
	{ 0x48,           0x0,         0xc00,   1,   0  },
	{ 0x49,           0x0,         0xc00,   1,   0  },
	{ 0x4A,           0x0,        0x4c00,   1,   0  },
	{ 0x4B,           0x0,           0x0,   0,   0  },
	{ 0x4C,           0x0,           0x0,   0,   0  },
	{ 0x4D,           0x0,           0x0,   0,   0  },
	{ 0x4E,           0x0,           0x2,   1,   0  },
	{ 0x4F,           0x0,        0x3000,   0,   1  },
	{ 0x50,           0x0,         0x303,   0,   1  },
	{ 0x51,           0x0,        0x36e2,   0,   1  },
	{ 0x52,           0x0,           0x0,   0,   0  },
	{ 0x53,           0x0,        0x6000,   0,   1  },
	{ 0x54,           0x0,           0x3,   0,   1  },
	{ 0x55,           0x0,           0x0,   0,   0  },
	{ 0x56,           0x0,           0x0,   0,   0  },
	{ 0x57,           0x0,           0x0,   0,   0  },
	{ 0x58,           0x0,        0x2370,   1,   0  },
	{ 0x59,           0x0,        0xffff,   1,   0  },
	{ 0x5A,           0x0,         0x3af,   0,   1  },
	{ 0x5B,           0x0,         0x58c,   1,   0  },
	{ 0x5C,           0x0,          0x77,   1,   0  },
	{ 0x5D,           0x0,          0x77,   1,   0  },
	{ 0x5E,           0x0,          0x77,   1,   0  },
	{ 0x5F,           0x0,          0x77,   1,   0  },
	{ 0x60,           0x0,          0x3c,   1,   1  },
	{ 0x61,           0x0,           0x1,   0,   1  },
	{ 0x62,           0x0,        0x1d1d,   0,   1  },
	{ 0x63,           0x0,        0x1c77,   1,   1  },
	{ 0x64,           0x0,         0x8a0,   1,   0  },
	{ 0x65,           0x0,        0x4000,   1,   0  },
	{ 0x66,           0x0,         0xf0f,   1,   0  },
	{ 0x67,           0x0,           0x7,   1,   0  },
	{ 0x68,           0x0,        0x7f32,   1,   0  },
	{ 0x69,           0x0,         0xf1c,   0,   1  },
	{ 0x6A,           0x0,           0x0,   0,   0  },
	{ 0x6B,           0x0,          0x77,   1,   1  },
	{ 0x6C,           0x0,        0x7474,   1,   1  },
	{ 0x6D,           0x0,        0xd032,   1,   0  },
	{ 0x6E,           0x0,        0x500a,   1,   0  },
	{ 0x6F,           0x0,           0x0,   0,   0  },
	{ 0x70,           0x0,         0xac5,   1,   0  },
	{ 0x71,           0x0,         0x707,   1,   0  },
	{ 0x72,           0x0,        0xff27,   1,   1  },
	{ 0x73,           0x0,        0x2e00,   1,   1  },
	{ 0x74,           0x0,        0xf300,   1,   1  },
	{ 0x75,           0x0,        0x1f17,   1,   1  },
	{ 0x76,           0x0,        0x2020,   0,   1  },
	{ 0x77,           0x0,        0x7474,   1,   1  },
	{ 0x78,           0x0,        0x4900,   1,   1  },
	{ 0x79,           0x0,          0x10,   0,   1  },
	{ 0x7A,           0x0,           0x0,   0,   0  },
	{ 0x7B,           0x0,        0x1f20,   1,   0  },
	{ 0x7C,           0x0,        0x7e28,   1,   0  },
	{ 0x7D,           0x0,           0x4,   1,   0  },
	{ 0x7E,           0x0,        0x7002,   1,   0  },
	{ 0x7F,           0x0,          0xf0,   1,   0  },
	{ 0x80,           0x0,           0x0,   0,   0  },
	{ 0x81,           0x0,           0x0,   0,   0  },
	{ 0x82,           0x0,        0x1240,   1,   0  },
	{ 0x83,           0x0,         0x750,   1,   0  },
	{ 0x84,           0x0,        0x6666,   1,   1  },
	{ 0x85,           0x0,          0xb7,   0,   1  },
	{ 0x86,           0x0,           0xf,   1,   0  },
	{ 0x87,           0x0,        0x3105,   1,   1  },
	{ 0x88,           0x0,          0x2a,   1,   1  },
	{ 0x89,           0x0,        0x1515,   0,   1  },
	{ 0x8A,           0x0,        0x1515,   0,   1  },
	{ 0x8B,           0x0,           0xe,   0,   1  },
	{ 0x8C,           0x0,         0xe1f,   0,   1  },
	{ 0x8D,           0x0,         0x580,   1,   1  },
	{ 0x8E,           0x0,         0x877,   1,   0  },
	{ 0x8F,           0x0,         0xc30,   1,   0  },
	{ 0x90,           0x0,         0x200,   1,   0  },
	{ 0x91,           0x0,           0x4,   1,   0  },
	{ 0x92,           0x0,           0x0,   0,   0  },
	{ 0x93,           0x0,        0x7271,   1,   1  },
	{ 0x94,           0x0,        0x5c60,   1,   0  },
	{ 0x95,           0x0,        0x5c02,   1,   0  },
	{ 0x96,           0x0,         0x709,   1,   1  },
	{ 0x97,           0x0,        0x4000,   1,   0  },
	{ 0x98,           0x0,         0x40a,   1,   0  },
	{ 0x99,           0x0,           0x0,   1,   1  },
	{ 0x9A,           0x0,         0x16e,   0,   1  },
	{ 0x9B,           0x0,        0x4ccd,   0,   1  },
	{ 0x9C,           0x0,           0x0,   0,   0  },
	{ 0x9D,           0x0,          0xc0,   1,   1  },
	{ 0x9E,           0x0,        0x5540,   1,   1  },
	{ 0x9F,           0x0,         0x430,   1,   0  },
	{ 0xA0,           0x0,         0x430,   1,   0  },
	{ 0xA1,           0x0,           0x0,   0,   0  },
	{ 0xA2,           0x0,        0x3333,   1,   0  },
	{ 0xA3,           0x0,        0x3334,   1,   0  },
	{ 0xA4,           0x0,        0x4444,   1,   0  },
	{ 0xA5,           0x0,          0xaa,   1,   0  },
	{ 0xA6,           0x0,         0x777,   1,   0  },
	{ 0xA7,           0x0,           0x0,   0,   0  },
	{ 0xA8,           0x0,           0x0,   0,   0  },
	{ 0xA9,           0x0,           0x0,   0,   0  },
	{ 0xAA,           0x0,          0x85,   1,   0  },
	{ 0xAB,           0x0,        0x2000,   1,   0  },
	{ 0xAC,           0x0,           0x0,   0,   0  },
	{ 0xAD,           0x0,           0x2,   1,   0  },
	{ 0xAE,           0x0,           0x0,   0,   0  },
	{ 0xAF,           0x0,        0x8888,   1,   0  },
	{ 0xB0,           0x0,        0x8888,   1,   0  },
	{ 0xB1,           0x0,        0x8888,   1,   0  },
	{ 0xB2,           0x0,        0x8888,   1,   0  },
	{ 0xB3,           0x0,           0x0,   0,   0  },
	{ 0xB4,           0x0,           0x0,   0,   0  },
	{ 0xB5,           0x0,           0x0,   0,   0  },
	{ 0xB6,           0x0,           0x0,   0,   0  },
	{ 0xB7,           0x0,           0x0,   0,   0  },
	{ 0xB8,           0x0,           0x0,   0,   0  },
	{ 0xB9,           0x0,           0x0,   0,   0  },
	{ 0xBA,           0x0,           0x0,   0,   0  },
	{ 0xBB,           0x0,           0x0,   0,   0  },
	{ 0xBC,           0x0,           0x0,   0,   0  },
	{ 0xBD,           0x0,           0x0,   0,   0  },
	{ 0xBE,           0x0,           0x0,   0,   0  },
	{ 0xBF,           0x0,           0x0,   0,   0  },
	{ 0xC0,           0x0,           0x0,   0,   0  },
	{ 0xC1,           0x0,           0x0,   0,   0  },
	{ 0xC2,           0x0,           0x0,   0,   0  },
	{ 0xC3,           0x0,           0x0,   0,   0  },
	{ 0xC4,           0x0,           0x0,   0,   0  },
	{ 0xC5,           0x0,           0x0,   0,   0  },
	{ 0xC6,           0x0,           0x0,   0,   0  },
	{ 0xC7,           0x0,           0x0,   0,   0  },
	{ 0xC8,           0x0,           0x0,   0,   0  },
	{ 0xC9,           0x0,        0x8000,   0,   1  },
	{ 0xCA,           0x0,           0x0,   0,   0  },
	{ 0xCB,           0x0,           0x0,   0,   0  },
	{ 0xCC,           0x0,           0x0,   0,   0  },
	{ 0xCD,           0x0,           0x0,   0,   0  },
	{ 0xCE,           0x0,           0x0,   0,   0  },
	{ 0xCF,           0x0,           0x0,   0,   0  },
	{ 0xD0,           0x0,           0x0,   0,   0  },
	{ 0xD1,           0x0,           0x0,   0,   0  },
	{ 0xD2,           0x0,           0x0,   0,   0  },
	{ 0xD3,           0x0,          0xcc,   0,   1  },
	{ 0xD4,           0x0,           0x7,   1,   0  },
	{ 0xD5,           0x0,        0x6c9b,   1,   0  },
	{ 0xD6,           0x0,           0x0,   0,   0  },
	{ 0xD7,           0x0,          0x44,   1,   0  },
	{ 0xD8,           0x0,           0x0,   0,   0  },
	{ 0xFFFF,           0x0,           0x0,   0,   0  },
};
#else /* defined(BCMDBG) || defined(WLTEST) */
lcn40phy_radio_regs_t lcn40phy_radio_regs_2067[] = {
{ 0x00,             0,             0,   0,   0  },
{ 0x01,             0,        0x2065,   1,   0  },
{ 0x02,             0,        0x8380,   1,   0  },
{ 0x03,             0,        0x1414,   1,   0  },
{ 0x04,             0,        0x3838,   1,   0  },
{ 0x05,             0,        0x1937,   1,   0  },
{ 0x06,             0,             0,   0,   0  },
{ 0x07,             0,         0x210,   1,   0  },
{ 0x08,             0,             0,   0,   0  },
{ 0x09,             0,             0,   0,   0  },
{ 0x0A,             0,             0,   0,   0  },
{ 0x0B,             0,             0,   0,   0  },
{ 0x0C,             0,             0,   0,   0  },
{ 0x0D,             0,             0,   0,   0  },
{ 0x0E,             0,           0x1,   1,   0  },
{ 0x0F,             0,             0,   0,   0  },
{ 0x10,             0,             0,   0,   0  },
{ 0x11,             0,         0xf80,   1,   0  },
{ 0x12,             0,         0xf54,   1,   0  },
{ 0x13,             0,             0,   0,   0  },
{ 0x14,             0,             0,   0,   0  },
{ 0x15,             0,             0,   0,   0  },
{ 0x16,             0,             0,   0,   0  },
{ 0x17,             0,             0,   0,   0  },
{ 0x18,             0,             0,   0,   0  },
{ 0x19,             0,             0,   0,   0  },
{ 0x1A,             0,             0,   0,   0  },
{ 0x1B,             0,             0,   0,   0  },
{ 0x1C,             0,          0x14,   1,   0  },
{ 0x1D,             0,         0x280,   1,   0  },
{ 0x1E,             0,        0x1558,   1,   0  },
{ 0x1F,             0,          0x14,   1,   0  },
{ 0x20,             0,          0x14,   1,   0  },
{ 0x21,             0,             0,   0,   0  },
{ 0x22,             0,             0,   0,   0  },
{ 0x23,             0,             0,   0,   0  },
{ 0x24,             0,         0x200,   1,   0  },
{ 0x25,             0,         0x180,   1,   0  },
{ 0x26,             0,        0x5008,   1,   0  },
{ 0x27,             0,        0x3600,   1,   0  },
{ 0x28,             0,          0x77,   1,   0  },
{ 0x29,             0,          0x42,   1,   0  },
{ 0x2A,             0,          0x88,   1,   0  },
{ 0x2B,             0,        0x6060,   1,   0  },
{ 0x2C,             0,             0,   0,   0  },
{ 0x2D,             0,        0x3404,   1,   0  },
{ 0x2E,             0,         0x330,   1,   0  },
{ 0x2F,             0,             0,   0,   0  },
{ 0x30,             0,          0x74,   1,   0  },
{ 0x31,             0,        0x757c,   1,   1  },
{ 0x32,             0,        0x6060,   1,   0  },
{ 0x33,             0,         0x110,   1,   0  },
{ 0x34,             0,         0x140,   1,   0  },
{ 0x35,             0,          0x88,   1,   0  },
{ 0x36,             0,         0x140,   1,   0  },
{ 0x37,             0,          0x88,   1,   0  },
{ 0x38,             0,             0,   0,   0  },
{ 0x39,             0,             0,   0,   0  },
{ 0x3A,             0,             0,   0,   0  },
{ 0x3B,             0,             0,   0,   0  },
{ 0x3C,             0,             0,   0,   0  },
{ 0x3D,             0,             0,   0,   0  },
{ 0x3E,             0,        0x4420,   1,   0  },
{ 0x3F,             0,        0x4842,   1,   1  },
{ 0x40,             0,           0xc,   1,   0  },
{ 0x41,             0,        0x6666,   1,   0  },
{ 0x42,             0,        0x8100,   1,   0  },
{ 0x43,             0,         0x478,   1,   1  },
{ 0x44,             0,             0,   0,   0  },
{ 0x45,             0,           0x3,   1,   0  },
{ 0x46,             0,             0,   0,   0  },
{ 0x47,             0,          0x44,   1,   0  },
{ 0x48,             0,         0x400,   1,   1  },
{ 0x49,             0,         0x400,   1,   1  },
{ 0x4A,             0,        0x4c00,   1,   0  },
{ 0x4B,             0,             0,   0,   0  },
{ 0x4C,             0,             0,   0,   0  },
{ 0x4D,             0,             0,   0,   0  },
{ 0x4E,             0,           0x2,   1,   0  },
{ 0x4F,             0,        0x3000,   0,   1  },
{ 0x50,             0,         0x303,   0,   1  },
{ 0x51,             0,        0x36e2,   0,   1  },
{ 0x52,             0,             0,   0,   0  },
{ 0x53,             0,        0x6000,   0,   1  },
{ 0x54,             0,           0x3,   0,   1  },
{ 0x55,             0,             0,   0,   0  },
{ 0x56,             0,             0,   0,   0  },
{ 0x57,             0,             0,   0,   0  },
{ 0x58,             0,        0x2370,   1,   0  },
{ 0x59,             0,        0xffff,   1,   0  },
{ 0x5A,             0,         0x26f,   1,   1  },
{ 0x5B,             0,         0x5af,   1,   1  },
{ 0x5C,             0,          0x77,   1,   0  },
{ 0x5D,             0,          0x77,   1,   0  },
{ 0x5E,             0,          0x77,   1,   0  },
{ 0x5F,             0,          0x77,   1,   0  },
{ 0x60,             0,          0x5e,   1,   1  },
{ 0x61,             0,         0x335,   1,   1  },
{ 0x62,             0,        0x1616,   1,   1  },
{ 0x63,             0,         0xb76,   1,   1  },
{ 0x64,             0,         0x7a0,   1,   1  },
{ 0x65,             0,        0x4000,   1,   0  },
{ 0x66,             0,        0x1515,   1,   1  },
{ 0x67,             0,          0x77,   1,   1  },
{ 0x68,             0,        0x4f32,   1,   1  },
{ 0x69,             0,         0x70b,   1,   0  },
{ 0x6A,             0,          0x77,   0,   1  },
{ 0x6B,             0,          0x75,   1,   1  },
{ 0x6C,             0,        0x7777,   1,   1  },
{ 0x6D,             0,        0xd032,   1,   0  },
{ 0x6E,             0,        0x780a,   1,   1  },
{ 0x6F,             0,             0,   0,   0  },
{ 0x70,             0,         0xac5,   1,   0  },
{ 0x71,             0,        0x7777,   1,   1  },
{ 0x72,             0,             0,   0,   0  },
{ 0x73,             0,             0,   0,   0  },
{ 0x74,             0,             0,   0,   0  },
{ 0x75,             0,             0,   0,   0  },
{ 0x76,             0,             0,   0,   0  },
{ 0x77,             0,             0,   0,   0  },
{ 0x78,             0,             0,   0,   0  },
{ 0x79,             0,             0,   0,   0  },
{ 0x7A,             0,             0,   0,   0  },
{ 0x7B,             0,        0x1f20,   1,   0  },
{ 0x7C,             0,        0x7e28,   1,   0  },
{ 0x7D,             0,           0x4,   1,   0  },
{ 0x7E,             0,        0x7002,   1,   0  },
{ 0x7F,             0,          0x80,   0,   1  },
{ 0x80,             0,             0,   0,   0  },
{ 0x81,             0,             0,   0,   0  },
{ 0x82,             0,        0x1240,   1,   0  },
{ 0x83,             0,         0x750,   1,   0  },
{ 0x84,         0xadd,             0,   1,   0  },
{ 0x85,         0xadd,             0,   1,   0  },
{ 0x86,             0,           0xf,   1,   0  },
{ 0x87,             0,        0x300a,   1,   0  },
{ 0x88,             0,          0x20,   1,   0  },
{ 0x89,             0,             0,   0,   0  },
{ 0x8A,             0,             0,   0,   0  },
{ 0x8B,             0,             0,   0,   0  },
{ 0x8C,             0,             0,   0,   0  },
{ 0x8D,             0,         0x580,   1,   1  },
{ 0x8E,             0,         0x877,   1,   0  },
{ 0x8F,             0,         0xc30,   1,   0  },
{ 0x90,             0,         0x200,   1,   0  },
{ 0x91,             0,           0x4,   1,   0  },
{ 0x92,             0,             0,   0,   0  },
{ 0x93,             0,           0x1,   1,   0  },
{ 0x94,             0,        0x5c60,   1,   0  },
{ 0x95,             0,        0x5c02,   1,   0  },
{ 0x96,             0,           0x9,   1,   0  },
{ 0x97,             0,        0x4000,   1,   0  },
{ 0x98,             0,         0x40a,   1,   0  },
{ 0x99,             0,             0,   0,   0  },
{ 0x9A,             0,             0,   0,   0  },
{ 0x9B,             0,             0,   0,   0  },
{ 0x9C,             0,             0,   0,   0  },
{ 0x9D,             0,        0x11c0,   1,   0  },
{ 0x9E,             0,        0x5540,   1,   1  },
{ 0x9F,             0,         0x430,   1,   0  },
{ 0xA0,             0,         0x430,   1,   0  },
{ 0xA1,             0,             0,   0,   0  },
{ 0xA2,             0,        0x7733,   1,   1  },
{ 0xA3,             0,        0x7734,   1,   1  },
{ 0xA4,             0,        0x7777,   1,   1  },
{ 0xA5,             0,          0xaa,   1,   0  },
{ 0xA6,             0,         0x777,   1,   0  },
{ 0xA7,             0,             0,   0,   0  },
{ 0xA8,             0,             0,   0,   0  },
{ 0xA9,             0,             0,   0,   0  },
{ 0xAA,             0,          0x85,   1,   0  },
{ 0xAB,             0,        0x2000,   1,   0  },
{ 0xAC,             0,             0,   0,   0  },
{ 0xAD,             0,           0x2,   1,   0  },
{ 0xAE,             0,             0,   0,   0  },
{ 0xAF,             0,        0x8888,   1,   0  },
{ 0xB0,             0,        0x8888,   1,   0  },
{ 0xB1,             0,        0x8888,   1,   0  },
{ 0xB2,             0,        0x8888,   1,   0  },
{ 0xB3,             0,             0,   0,   0  },
{ 0xB4,             0,             0,   0,   0  },
{ 0xB5,             0,             0,   0,   0  },
{ 0xB6,             0,             0,   0,   0  },
{ 0xB7,             0,             0,   0,   0  },
{ 0xB8,             0,             0,   0,   0  },
{ 0xB9,             0,             0,   0,   0  },
{ 0xBA,             0,             0,   0,   0  },
{ 0xBB,             0,             0,   0,   0  },
{ 0xBC,             0,             0,   0,   0  },
{ 0xBD,             0,             0,   0,   0  },
{ 0xBE,             0,             0,   0,   0  },
{ 0xBF,             0,             0,   0,   0  },
{ 0xC0,             0,             0,   0,   0  },
{ 0xC1,             0,             0,   0,   0  },
{ 0xC2,             0,             0,   0,   0  },
{ 0xC3,             0,             0,   0,   0  },
{ 0xC4,             0,             0,   0,   0  },
{ 0xC5,             0,             0,   0,   0  },
{ 0xC6,             0,             0,   0,   0  },
{ 0xC7,             0,             0,   0,   0  },
{ 0xC8,             0,             0,   0,   0  },
{ 0xC9,             0,        0x8000,   1,   0  },
{ 0xCA,             0,             0,   0,   0  },
{ 0xCB,             0,             0,   0,   0  },
{ 0xCC,             0,             0,   0,   0  },
{ 0xCD,             0,             0,   0,   0  },
{ 0xCE,             0,             0,   0,   0  },
{ 0xCF,             0,             0,   0,   0  },
{ 0xD0,             0,             0,   0,   0  },
{ 0xD1,             0,             0,   0,   0  },
{ 0xD2,             0,             0,   0,   0  },
{ 0xD3,             0,             0,   0,   0  },
{ 0xD4,             0,           0x7,   1,   0  },
{ 0xD5,             0,        0x6c9b,   1,   0  },
{ 0xD6,             0,             0,   0,   0  },
{ 0xD7,             0,          0x44,   1,   0  },
{ 0xFFFF,             0,             0,   0,   0  },
};

lcn40phy_radio_regs_t lcn40phy_radio_regs_2065[] = {
	{ 0x00,           0x0,           0x0,   0,   0  },
	{ 0x01,           0x0,        0x2065,   1,   0  },
	{ 0x02,           0x0,        0x8380,   1,   0  },
	{ 0x03,           0x0,        0x1414,   1,   0  },
	{ 0x04,           0x0,        0x3838,   1,   0  },
	{ 0x05,           0x0,        0x1937,   1,   0  },
	{ 0x06,           0x0,           0x0,   0,   0  },
	{ 0x07,           0x0,         0x310,   1,   1  },
	{ 0x08,           0x0,           0x0,   0,   0  },
	{ 0x09,           0x0,           0x0,   0,   0  },
	{ 0x0A,           0x0,           0x0,   0,   0  },
	{ 0x0B,           0x0,           0x0,   0,   0  },
	{ 0x0C,           0x0,           0x0,   0,   0  },
	{ 0x0D,           0x0,           0x0,   0,   0  },
	{ 0x0E,           0x0,           0x1,   1,   0  },
	{ 0x0F,           0x0,           0x0,   0,   0  },
	{ 0x10,           0x0,           0x0,   0,   0  },
	{ 0x11,           0x0,         0xf80,   1,   0  },
	{ 0x12,           0x0,         0xf54,   1,   0  },
	{ 0x13,           0x0,           0x0,   0,   0  },
	{ 0x14,           0x0,           0x0,   0,   0  },
	{ 0x15,           0x0,           0x0,   0,   0  },
	{ 0x16,           0x0,           0x0,   0,   0  },
	{ 0x17,           0x0,           0x0,   0,   0  },
	{ 0x18,           0x0,           0x0,   0,   0  },
	{ 0x19,           0x0,           0x0,   0,   0  },
	{ 0x1A,           0x0,           0x0,   0,   0  },
	{ 0x1B,           0x0,           0x0,   0,   0  },
	{ 0x1C,           0x0,          0x14,   1,   0  },
	{ 0x1D,           0x0,         0x280,   1,   0  },
	{ 0x1E,           0x0,        0x1558,   1,   0  },
	{ 0x1F,           0x0,          0x14,   1,   0  },
	{ 0x20,           0x0,          0x14,   1,   0  },
	{ 0x21,           0x0,           0x0,   0,   0  },
	{ 0x22,           0x0,           0x0,   0,   0  },
	{ 0x23,           0x0,           0x0,   0,   0  },
	{ 0x24,           0x0,         0x200,   1,   0  },
	{ 0x25,           0x0,         0x180,   1,   0  },
	{ 0x26,           0x0,        0x5000,   1,   1  },
	{ 0x27,           0x0,        0x3600,   1,   0  },
	{ 0x28,           0x0,         0xf77,   1,   1  },
	{ 0x29,           0x0,          0x42,   1,   0  },
	{ 0x2A,           0x0,          0x88,   1,   0  },
	{ 0x2B,           0x0,        0x6060,   1,   0  },
	{ 0x2C,           0x0,           0x0,   0,   0  },
	{ 0x2D,           0x0,        0x3404,   1,   0  },
	{ 0x2E,           0x0,         0x330,   1,   0  },
	{ 0x2F,           0x0,           0x0,   0,   0  },
	{ 0x30,           0x0,          0x74,   1,   0  },
	{ 0x31,           0x0,        0x7774,   1,   0  },
	{ 0x32,           0x0,        0x6060,   1,   0  },
	{ 0x33,           0x0,         0x110,   1,   0  },
	{ 0x34,           0x0,         0x140,   1,   0  },
	{ 0x35,           0x0,          0x88,   1,   0  },
	{ 0x36,           0x0,         0x140,   1,   0  },
	{ 0x37,           0x0,          0x88,   1,   0  },
	{ 0x38,           0x0,           0x0,   0,   0  },
	{ 0x39,           0x0,           0xc,   0,   1  },
	{ 0x3A,           0x0,           0x0,   0,   0  },
	{ 0x3B,           0x0,           0x0,   0,   0  },
	{ 0x3C,           0x0,           0x0,   0,   0  },
	{ 0x3D,           0x0,           0x0,   0,   0  },
	{ 0x3E,           0x0,        0x4420,   1,   0  },
	{ 0x3F,           0x0,         0x842,   1,   0  },
	{ 0x40,           0x0,           0xc,   1,   0  },
	{ 0x41,           0x0,        0x6666,   1,   0  },
	{ 0x42,           0x0,        0x8100,   1,   0  },
	{ 0x43,           0x0,         0xc78,   1,   0  },
	{ 0x44,           0x0,           0x0,   0,   0  },
	{ 0x45,           0x0,           0x3,   1,   0  },
	{ 0x46,           0x0,           0x0,   0,   0  },
	{ 0x47,           0x0,          0x44,   1,   0  },
	{ 0x48,           0x0,         0xc00,   1,   0  },
	{ 0x49,           0x0,         0xc00,   1,   0  },
	{ 0x4A,           0x0,        0x4c00,   1,   0  },
	{ 0x4B,           0x0,           0x0,   0,   0  },
	{ 0x4C,           0x0,           0x0,   0,   0  },
	{ 0x4D,           0x0,           0x0,   0,   0  },
	{ 0x4E,           0x0,           0x2,   1,   0  },
	{ 0x4F,           0x0,        0x3000,   0,   1  },
	{ 0x50,           0x0,         0x303,   0,   1  },
	{ 0x51,           0x0,        0x36e2,   0,   1  },
	{ 0x52,           0x0,           0x0,   0,   0  },
	{ 0x53,           0x0,        0x6000,   0,   1  },
	{ 0x54,           0x0,           0x3,   0,   1  },
	{ 0x55,           0x0,           0x0,   0,   0  },
	{ 0x56,           0x0,           0x0,   0,   0  },
	{ 0x57,           0x0,           0x0,   0,   0  },
	{ 0x58,           0x0,        0x2370,   1,   0  },
	{ 0x59,           0x0,        0xffff,   1,   0  },
	{ 0x5A,           0x0,         0x3af,   0,   1  },
	{ 0x5B,           0x0,         0x58c,   1,   0  },
	{ 0x5C,           0x0,          0x77,   1,   0  },
	{ 0x5D,           0x0,          0x77,   1,   0  },
	{ 0x5E,           0x0,          0x77,   1,   0  },
	{ 0x5F,           0x0,          0x77,   1,   0  },
	{ 0x60,           0x0,          0x3c,   1,   1  },
	{ 0x61,           0x0,           0x1,   0,   1  },
	{ 0x62,           0x0,        0x1d1d,   0,   1  },
	{ 0x63,           0x0,        0x1c77,   1,   1  },
	{ 0x64,           0x0,         0x8a0,   1,   0  },
	{ 0x65,           0x0,        0x4000,   1,   0  },
	{ 0x66,           0x0,         0xf0f,   1,   0  },
	{ 0x67,           0x0,           0x7,   1,   0  },
	{ 0x68,           0x0,        0x7f32,   1,   0  },
	{ 0x69,           0x0,         0xf1c,   0,   1  },
	{ 0x6A,           0x0,           0x0,   0,   0  },
	{ 0x6B,           0x0,          0x77,   1,   1  },
	{ 0x6C,           0x0,        0x7474,   1,   1  },
	{ 0x6D,           0x0,        0xd032,   1,   0  },
	{ 0x6E,           0x0,        0x500a,   1,   0  },
	{ 0x6F,           0x0,           0x0,   0,   0  },
	{ 0x70,           0x0,         0xac5,   1,   0  },
	{ 0x71,           0x0,         0x707,   1,   0  },
	{ 0x72,           0x0,        0xff27,   1,   1  },
	{ 0x73,           0x0,        0x2e00,   1,   1  },
	{ 0x74,           0x0,        0xf300,   1,   1  },
	{ 0x75,           0x0,        0x1f17,   1,   1  },
	{ 0x76,           0x0,        0x2020,   0,   1  },
	{ 0x77,           0x0,        0x7474,   1,   1  },
	{ 0x78,           0x0,        0x4900,   1,   1  },
	{ 0x79,           0x0,          0x10,   0,   1  },
	{ 0x7A,           0x0,           0x0,   0,   0  },
	{ 0x7B,           0x0,        0x1f20,   1,   0  },
	{ 0x7C,           0x0,        0x7e28,   1,   0  },
	{ 0x7D,           0x0,           0x4,   1,   0  },
	{ 0x7E,           0x0,        0x7002,   1,   0  },
	{ 0x7F,           0x0,          0xf0,   1,   0  },
	{ 0x80,           0x0,           0x0,   0,   0  },
	{ 0x81,           0x0,           0x0,   0,   0  },
	{ 0x82,           0x0,        0x1240,   1,   0  },
	{ 0x83,           0x0,         0x750,   1,   0  },
	{ 0x84,           0x0,        0x6666,   1,   1  },
	{ 0x85,           0x0,          0xb7,   0,   1  },
	{ 0x86,           0x0,           0xf,   1,   0  },
	{ 0x87,           0x0,        0x3105,   1,   1  },
	{ 0x88,           0x0,          0x2a,   1,   1  },
	{ 0x89,           0x0,        0x1515,   0,   1  },
	{ 0x8A,           0x0,        0x1515,   0,   1  },
	{ 0x8B,           0x0,           0xe,   0,   1  },
	{ 0x8C,           0x0,         0xe1f,   0,   1  },
	{ 0x8D,           0x0,         0x580,   1,   1  },
	{ 0x8E,           0x0,         0x877,   1,   0  },
	{ 0x8F,           0x0,         0xc30,   1,   0  },
	{ 0x90,           0x0,         0x200,   1,   0  },
	{ 0x91,           0x0,           0x4,   1,   0  },
	{ 0x92,           0x0,           0x0,   0,   0  },
	{ 0x93,           0x0,        0x7271,   1,   1  },
	{ 0x94,           0x0,        0x5c60,   1,   0  },
	{ 0x95,           0x0,        0x5c02,   1,   0  },
	{ 0x96,           0x0,         0x709,   1,   1  },
	{ 0x97,           0x0,        0x4000,   1,   0  },
	{ 0x98,           0x0,         0x40a,   1,   0  },
	{ 0x99,           0x0,           0x0,   1,   1  },
	{ 0x9A,           0x0,         0x16e,   0,   1  },
	{ 0x9B,           0x0,        0x4ccd,   0,   1  },
	{ 0x9C,           0x0,           0x0,   0,   0  },
	{ 0x9D,           0x0,          0xc0,   1,   1  },
	{ 0x9E,           0x0,        0x5540,   1,   1  },
	{ 0x9F,           0x0,         0x430,   1,   0  },
	{ 0xA0,           0x0,         0x430,   1,   0  },
	{ 0xA1,           0x0,           0x0,   0,   0  },
	{ 0xA2,           0x0,        0x3333,   1,   0  },
	{ 0xA3,           0x0,        0x3334,   1,   0  },
	{ 0xA4,           0x0,        0x4444,   1,   0  },
	{ 0xA5,           0x0,          0xaa,   1,   0  },
	{ 0xA6,           0x0,         0x777,   1,   0  },
	{ 0xA7,           0x0,           0x0,   0,   0  },
	{ 0xA8,           0x0,           0x0,   0,   0  },
	{ 0xA9,           0x0,           0x0,   0,   0  },
	{ 0xAA,           0x0,          0x85,   1,   0  },
	{ 0xAB,           0x0,        0x2000,   1,   0  },
	{ 0xAC,           0x0,           0x0,   0,   0  },
	{ 0xAD,           0x0,           0x2,   1,   0  },
	{ 0xAE,           0x0,           0x0,   0,   0  },
	{ 0xAF,           0x0,        0x8888,   1,   0  },
	{ 0xB0,           0x0,        0x8888,   1,   0  },
	{ 0xB1,           0x0,        0x8888,   1,   0  },
	{ 0xB2,           0x0,        0x8888,   1,   0  },
	{ 0xB3,           0x0,           0x0,   0,   0  },
	{ 0xB4,           0x0,           0x0,   0,   0  },
	{ 0xB5,           0x0,           0x0,   0,   0  },
	{ 0xB6,           0x0,           0x0,   0,   0  },
	{ 0xB7,           0x0,           0x0,   0,   0  },
	{ 0xB8,           0x0,           0x0,   0,   0  },
	{ 0xB9,           0x0,           0x0,   0,   0  },
	{ 0xBA,           0x0,           0x0,   0,   0  },
	{ 0xBB,           0x0,           0x0,   0,   0  },
	{ 0xBC,           0x0,           0x0,   0,   0  },
	{ 0xBD,           0x0,           0x0,   0,   0  },
	{ 0xBE,           0x0,           0x0,   0,   0  },
	{ 0xBF,           0x0,           0x0,   0,   0  },
	{ 0xC0,           0x0,           0x0,   0,   0  },
	{ 0xC1,           0x0,           0x0,   0,   0  },
	{ 0xC2,           0x0,           0x0,   0,   0  },
	{ 0xC3,           0x0,           0x0,   0,   0  },
	{ 0xC4,           0x0,           0x0,   0,   0  },
	{ 0xC5,           0x0,           0x0,   0,   0  },
	{ 0xC6,           0x0,           0x0,   0,   0  },
	{ 0xC7,           0x0,           0x0,   0,   0  },
	{ 0xC8,           0x0,           0x0,   0,   0  },
	{ 0xC9,           0x0,        0x8000,   0,   1  },
	{ 0xCA,           0x0,           0x0,   0,   0  },
	{ 0xCB,           0x0,           0x0,   0,   0  },
	{ 0xCC,           0x0,           0x0,   0,   0  },
	{ 0xCD,           0x0,           0x0,   0,   0  },
	{ 0xCE,           0x0,           0x0,   0,   0  },
	{ 0xCF,           0x0,           0x0,   0,   0  },
	{ 0xD0,           0x0,           0x0,   0,   0  },
	{ 0xD1,           0x0,           0x0,   0,   0  },
	{ 0xD2,           0x0,           0x0,   0,   0  },
	{ 0xD3,           0x0,          0xcc,   0,   1  },
	{ 0xD4,           0x0,           0x7,   1,   0  },
	{ 0xD5,           0x0,        0x6c9b,   1,   0  },
	{ 0xD6,           0x0,           0x0,   0,   0  },
	{ 0xD7,           0x0,          0x44,   1,   0  },
	{ 0xD8,           0x0,           0x0,   0,   0  },
	{ 0xFFFF,           0x0,           0x0,   0,   0  },
};
#endif /* defined(BCMDBG) || defined(WLTEST) */

/* a band channel info type for 2065 radio used in lcn40phy */
typedef struct _chan_info_2065_lcn40phy {
	uint   chan;            /* channel number */
	uint   freq;            /* in Mhz */
	uint8 logen1;					/* AWKWARD: different field */
	uint8 logen2;					/* offsets between 5g  */
	uint8 logen3;					/* and 2g */
	uint8 lna_freq1;
	uint8 lna_freq2;
	uint8 lna_tx;
	uint8 txmix;
	uint8 pga;
	uint8 pad;
} chan_info_2065_lcn40phy_t;

/* Autogenerated by 2065_chantbl_tcl2c.tcl */
static chan_info_2065_lcn40phy_t chan_info_2067_lcn40phy[] = {
{   1, 2412, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x03, 0x05, 0x04 },
{   2, 2417, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x03, 0x05, 0x04 },
{   3, 2422, 0x01, 0x00, 0x00, 0x07, 0x03, 0x00, 0x03, 0x05, 0x04 },
{   4, 2427, 0x01, 0x00, 0x00, 0x07, 0x02, 0x00, 0x03, 0x05, 0x04 },
{   5, 2432, 0x01, 0x00, 0x00, 0x06, 0x02, 0x00, 0x02, 0x05, 0x04 },
{   6, 2437, 0x01, 0x00, 0x00, 0x06, 0x01, 0x00, 0x02, 0x05, 0x04 },
{   7, 2442, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x04, 0x04 },
{   8, 2447, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x04, 0x04 },
{   9, 2452, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x04, 0x04 },
{  10, 2457, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x04, 0x04 },
{  11, 2462, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x04, 0x04 },
{  12, 2467, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x02, 0x04, 0x04 },
{  13, 2472, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x02, 0x04, 0x04 },
{  14, 2484, 0x01, 0x00, 0x00, 0x05, 0x00, 0x00, 0x01, 0x04, 0x04 },
#ifdef BAND5G
{  34, 5170, 0x0B, 0x0B, 0x05, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  36, 5180, 0x0B, 0x0B, 0x05, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  38, 5190, 0x0B, 0x0A, 0x04, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  40, 5200, 0x0B, 0x0A, 0x04, 0x07, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  42, 5210, 0x0A, 0x0A, 0x03, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{  44, 5220, 0x0A, 0x0A, 0x03, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{  46, 5230, 0x0A, 0x09, 0x03, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{  48, 5240, 0x0A, 0x09, 0x02, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{  52, 5260, 0x09, 0x09, 0x02, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x0A },
{  54, 5270, 0x09, 0x09, 0x02, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x09 },
{  56, 5280, 0x09, 0x09, 0x01, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x09 },
{  60, 5300, 0x08, 0x09, 0x01, 0x06, 0x05, 0x00, 0x0E, 0x0F, 0x08 },
{  62, 5310, 0x08, 0x08, 0x01, 0x06, 0x05, 0x00, 0x0E, 0x0F, 0x08 },
{  64, 5320, 0x08, 0x08, 0x01, 0x06, 0x04, 0x00, 0x0A, 0x0E, 0x08 },
{ 100, 5500, 0x05, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 102, 5510, 0x05, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 104, 5520, 0x04, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 108, 5540, 0x04, 0x07, 0x00, 0x04, 0x02, 0x00, 0x07, 0x0A, 0x07 },
{ 110, 5550, 0x04, 0x07, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x06 },
{ 112, 5560, 0x03, 0x07, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x05 },
{ 116, 5580, 0x03, 0x06, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x05 },
{ 118, 5590, 0x03, 0x06, 0x00, 0x03, 0x02, 0x00, 0x06, 0x09, 0x05 },
{ 120, 5600, 0x03, 0x06, 0x00, 0x02, 0x02, 0x00, 0x06, 0x09, 0x05 },
{ 124, 5620, 0x04, 0x05, 0x00, 0x02, 0x02, 0x00, 0x06, 0x08, 0x05 },
{ 126, 5630, 0x02, 0x05, 0x00, 0x02, 0x02, 0x00, 0x05, 0x08, 0x04 },
{ 128, 5640, 0x02, 0x05, 0x00, 0x02, 0x02, 0x00, 0x05, 0x08, 0x04 },
{ 132, 5660, 0x02, 0x05, 0x00, 0x02, 0x01, 0x00, 0x04, 0x08, 0x04 },
{ 134, 5670, 0x01, 0x05, 0x00, 0x02, 0x01, 0x00, 0x04, 0x08, 0x04 },
{ 136, 5680, 0x01, 0x05, 0x00, 0x02, 0x01, 0x00, 0x04, 0x08, 0x04 },
{ 140, 5700, 0x01, 0x05, 0x00, 0x02, 0x01, 0x00, 0x03, 0x08, 0x04 },
{ 149, 5745, 0x01, 0x05, 0x00, 0x01, 0x01, 0x00, 0x02, 0x08, 0x04 },
{ 151, 5755, 0x01, 0x05, 0x00, 0x01, 0x01, 0x00, 0x02, 0x08, 0x04 },
{ 153, 5765, 0x01, 0x04, 0x00, 0x01, 0x01, 0x00, 0x01, 0x08, 0x04 },
{ 157, 5785, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x07, 0x03 },
{ 159, 5795, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x06, 0x04 },
{ 161, 5805, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x05, 0x03 },
{ 165, 5825, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x05, 0x02 },
{ 184, 4920, 0x0F, 0x0E, 0x0A, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 185, 4925, 0x0F, 0x0D, 0x09, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 187, 4935, 0x0F, 0x0D, 0x09, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 188, 4940, 0x0F, 0x0D, 0x08, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 189, 4945, 0x0F, 0x0D, 0x08, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 192, 4960, 0x0F, 0x0D, 0x08, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 196, 4980, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 200, 5000, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 204, 5020, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 207, 5035, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 208, 5040, 0x0E, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0D },
{ 209, 5045, 0x0E, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0D },
{ 210, 5050, 0x0D, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0C },
{ 212, 5060, 0x0D, 0x0C, 0x06, 0x09, 0x07, 0x00, 0x0F, 0x0F, 0x0C },
{ 216, 5080, 0x0C, 0x0B, 0x06, 0x08, 0x07, 0x00, 0x0F, 0x0F, 0x0B },
#endif /* BAND5G */
};
static chan_info_2065_lcn40phy_t chan_info_2065_lcn40phy[] = {
{   1, 2412, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x01, 0x07 },
{   2, 2417, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x01, 0x07 },
{   3, 2422, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x01, 0x07 },
{   4, 2427, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x04, 0x01, 0x07 },
{   5, 2432, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{   6, 2437, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{   7, 2442, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{   8, 2447, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{   9, 2452, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  10, 2457, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x07 },
{  11, 2462, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x06 },
{  12, 2467, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x01, 0x06 },
{  13, 2472, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x00, 0x06 },
{  14, 2484, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x03, 0x00, 0x06 },
#ifdef BAND5G
{  34, 5170, 0x0B, 0x0B, 0x05, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  36, 5180, 0x0B, 0x0B, 0x05, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  38, 5190, 0x0B, 0x0A, 0x04, 0x08, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  40, 5200, 0x0B, 0x0A, 0x04, 0x07, 0x06, 0x00, 0x0F, 0x0F, 0x0B },
{  44, 5220, 0x0A, 0x0A, 0x03, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{  46, 5230, 0x0A, 0x09, 0x03, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{  48, 5240, 0x0A, 0x09, 0x02, 0x07, 0x05, 0x00, 0x0F, 0x0F, 0x0B },
{  52, 5260, 0x09, 0x09, 0x02, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x0A },
{  54, 5270, 0x09, 0x09, 0x02, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x09 },
{  56, 5280, 0x09, 0x09, 0x01, 0x06, 0x05, 0x00, 0x0F, 0x0F, 0x09 },
{  60, 5300, 0x08, 0x09, 0x01, 0x06, 0x05, 0x00, 0x0E, 0x0F, 0x08 },
{  62, 5310, 0x08, 0x08, 0x01, 0x06, 0x05, 0x00, 0x0E, 0x0F, 0x08 },
{  64, 5320, 0x08, 0x08, 0x01, 0x06, 0x04, 0x00, 0x0A, 0x0E, 0x08 },
{ 100, 5500, 0x05, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 102, 5510, 0x05, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 104, 5520, 0x04, 0x07, 0x00, 0x04, 0x03, 0x00, 0x08, 0x0A, 0x08 },
{ 108, 5540, 0x04, 0x07, 0x00, 0x04, 0x02, 0x00, 0x07, 0x0A, 0x07 },
{ 110, 5550, 0x04, 0x07, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x06 },
{ 112, 5560, 0x03, 0x07, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x05 },
{ 116, 5580, 0x03, 0x06, 0x00, 0x03, 0x02, 0x00, 0x07, 0x0A, 0x05 },
{ 118, 5590, 0x03, 0x06, 0x00, 0x03, 0x02, 0x00, 0x06, 0x09, 0x05 },
{ 120, 5600, 0x03, 0x06, 0x00, 0x02, 0x02, 0x00, 0x06, 0x09, 0x05 },
{ 124, 5620, 0x04, 0x05, 0x00, 0x02, 0x02, 0x00, 0x06, 0x08, 0x05 },
{ 128, 5640, 0x02, 0x05, 0x00, 0x02, 0x02, 0x00, 0x05, 0x08, 0x04 },
{ 132, 5660, 0x02, 0x05, 0x00, 0x02, 0x01, 0x00, 0x04, 0x08, 0x04 },
{ 136, 5680, 0x01, 0x05, 0x00, 0x02, 0x01, 0x00, 0x04, 0x08, 0x04 },
{ 140, 5700, 0x01, 0x05, 0x00, 0x02, 0x01, 0x00, 0x03, 0x08, 0x04 },
{ 149, 5745, 0x01, 0x05, 0x00, 0x01, 0x01, 0x00, 0x02, 0x08, 0x04 },
{ 153, 5765, 0x01, 0x04, 0x00, 0x01, 0x01, 0x00, 0x01, 0x08, 0x04 },
{ 157, 5785, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x07, 0x03 },
{ 159, 5795, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x06, 0x04 },
{ 161, 5805, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x05, 0x03 },
{ 165, 5825, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x01, 0x05, 0x02 },
{ 184, 4920, 0x0F, 0x0E, 0x0A, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 185, 4925, 0x0F, 0x0D, 0x09, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 187, 4935, 0x0F, 0x0D, 0x09, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 188, 4940, 0x0F, 0x0D, 0x08, 0x0B, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 189, 4945, 0x0F, 0x0D, 0x08, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 192, 4960, 0x0F, 0x0D, 0x08, 0x0A, 0x09, 0x00, 0x0F, 0x0F, 0x0E },
{ 196, 4980, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 207, 5035, 0x0E, 0x0D, 0x08, 0x0A, 0x08, 0x00, 0x0F, 0x0F, 0x0E },
{ 208, 5040, 0x0E, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0D },
{ 209, 5045, 0x0E, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0D },
{ 210, 5050, 0x0D, 0x0C, 0x07, 0x09, 0x08, 0x00, 0x0F, 0x0F, 0x0C },
{ 212, 5060, 0x0D, 0x0C, 0x06, 0x09, 0x07, 0x00, 0x0F, 0x0F, 0x0C },
{ 216, 5080, 0x0C, 0x0B, 0x06, 0x08, 0x07, 0x00, 0x0F, 0x0F, 0x0B },
#endif /* BAND5G */
};

#define LCN40PHY_NUM_DIG_FILT_COEFFS 17
#define LCN40PHY_NUM_TX_DIG_FILTERS_CCK 15
/* filter id, followed by coefficients */
uint16 LCN40PHY_txdigfiltcoeffs_cck[LCN40PHY_NUM_TX_DIG_FILTERS_CCK]
	[1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 1, 0x19f, 0xff52, 0x40, 0x80, 0x40, 0x318, 0xfe78, 0x40, 0x80, 0x40, 0x30a,
	0xfe2e, 0x40, 0x80, 0x40, 8},
	{ 1, 1, 0x192, 0xff37, 0x29f, 0xff02, 0x260, 0xff47, 0x103, 0x3b, 0x103, 0x44, 0x36,
	0x44, 0x5d, 0xa7, 0x5d, 8},
	{ 2, 1, 415, 1874, 64, 128, 64, 792, 1656, 192, 384, 192, 778, 1582, 64, 128, 64, 8},
	{ 3, 1, 302, 1841, 129, 258, 129, 658, 1720, 205, 410, 205, 754, 1760, 170, 340, 170, 8},
	{ 20, 1, 360, -164, 242, -314, 242, 752, -328, 205, -203, 205, 767, -288, 253, 183, 253, 8},
	{ 21, 1, 360, 1884, 149, 1874, 149, 752, 1720, 205, 1884, 205, 767, 1760, 256, 273, 256, 8},
	{ 22, 1, 360, 1884, 98, 1948, 98, 752, 1720, 205, 1924, 205, 767, 1760, 256, 352, 256, 8},
	{ 23, 1, 350, 1884, 116, 1966, 116, 752, 1720, 205, 2008, 205, 767, 1760, 129, 235, 129, 8},
	{ 24, 1, 325, 1884, 32, 40, 32, 756, 1720, 256, 471, 256, 766, 1760, 262, 1878, 262, 8},
	{ 25, 1, 299, 1884, 51, 64, 51, 736, 1720, 256, 471, 256, 765, 1760, 262, 1878, 262, 8},
	{ 26, 1, 277, 1943, 39, 117, 88, 637, 1838, 64, 192, 144, 614, 1864, 128, 384, 288, 8},
	{ 27, 1, 245, 1943, 49, 147, 110, 626, 1838, 162, 485, 363, 613, 1864, 62, 186, 139, 8},
	{ 30, 1, 302, 1841, 61, 122, 61, 658, 1720, 205, 410, 205, 754, 1760, 170, 340, 170, 8},
	{ 40, 1, 360, 1884, 242, 1734, 242, 752, 1720, 205, 1845, 205, 767, 1760, 511, 370, 511, 8},
	{ 50, 1, 0x1d9, 0xff0c, 0x20, 0x40, 0x20, 0x3a2, 0xfe41, 0x10, 0x20, 0x10, 0x3a1,
	0xfe58, 0x10, 0x20, 0x10, 8}
	};

#define LCN40PHY_NUM_TX_DIG_FILTERS_CCK_HIGH 9
uint16 LCN40PHY_txdigfiltcoeffs_cck_high /* high dacrate */
	[LCN40PHY_NUM_TX_DIG_FILTERS_CCK_HIGH][1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 1, 0x1d9, 0xff0c, 0x20, 0x40, 0x20, 0x3a2, 0xfe41, 0x10, 0x20, 0x10, 0x3a1,
	0xfe58, 0x10, 0x20, 0x10, 8},
	{ 21, 1, 447, 1841, 224, 1649, 224, 903, 1633, 117, 1852, 117, 896, 1656, 334,
	1856, 334, 8},
	{ 22, 1, 447, 1841, 98, 1877, 98, 903, 1633, 225, 1685, 225, 889, 1662, 268, 1955, 268, 8},
	{ 23, 1, 444, 1841, 116, 1858, 116, 903, 1633, 174, 1790, 174, 896, 1656, 129, 15, 129, 8},
	{ 24, 1, 436, 1841, 33, 2019, 33, 904, 1633, 121, 112, 121, 895, 1656, 307, 1548, 307, 8},
	{ 25, 1, 429, 1841, 72, 1986, 72, 898, 1633, 86, 79, 86, 895, 1656, 313, 1538, 313, 8},
	{ 26, 1, 403, 1876, 39, 2036, 39, 795, 1735, 180, 1992, 180, 819, 1720, 128, 2008, 128, 8},
	{ 27, 1, 393, 1876, 49, 2029, 49, 791, 1735, 256, 1968, 256, 814, 1725, 117, 2011, 117, 8},
	{ 30, 1, 443, 1817, 61, 2019, 61, 875, 1633, 154, 1976, 154, 892, 1656, 170, 1969, 170, 8}
	};

#define LCN40PHY_NUM_TX_DIG_FILTERS_OFDM 7
uint16 LCN40PHY_txdigfiltcoeffs_ofdm[LCN40PHY_NUM_TX_DIG_FILTERS_OFDM]
	[1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 0, 0xa2, 0, 0x100, 0x100, 0x0, 0x0, 0x0, 0x100, 0x0, 0x0,
	0x278, 0xfea0, 0x80, 0x100, 0x80, 8},
	{ 1, 0, 374, -135, 16, 32, 16, 799, -396, 50, 32, 50,
	0x750, -469, 212, -50, 212, 8},
	{ 2, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32,
	748, -270, 128, -30, 128, 8},
	{3, 0, 375, 0xFF16, 37, 76, 37, 799, 0xFE74, 32, 20, 32, 748,
	0xFEF2, 148, 0xFFDD, 148, 8},
	{4, 0, 307, 1966, 53, 106, 53, 779, 1669, 53, 2038, 53, 765,
	1579, 212, 1846, 212, 8},
	{5, 0, 0x1c5, 0xff1d, 0x20, 0x40, 0x20, 0, 0, 0x100, 0, 0, 0x36b,
	0xfe82, 0x14, 0x29, 0x14, 8},
	{10, 0, 0xa2, 0, 0x100, 0x100, 0x0, 0, 0, 511, 0, 0, 0x278,
	0xfea0, 256, 511, 256, 8}
	};

#define LCN40PHY_NUM_TX_DIG_FILTERS_OFDM40 3
uint16 LCN40PHY_txdigfiltcoeffs_ofdm40 /* high dacrate */
	[LCN40PHY_NUM_TX_DIG_FILTERS_OFDM40][1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{0, 0, 0x97, 0, 0x100, 0x100, 0, 0, 0, 0x100, 0, 0, 0x236, 0xfeb7, 0x80, 0x100, 0x80, 8},
	{2, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32, 748, -270, 72, -17, 72, 8},
	{3, 0, 366, -234, 37, 74, 37, 791, -396, 32, 20, 32, 748, -270, 81, -19, 81, 8},
	};

#define LCN40PHY_NUM_TX_DIG_FILTERS_OFDM_HIGH 3
uint16 LCN40PHY_txdigfiltcoeffs_ofdm_high /* high dacrate */
	[LCN40PHY_NUM_TX_DIG_FILTERS_OFDM_HIGH][1+LCN40PHY_NUM_DIG_FILT_COEFFS] = {
	{0, 0, 453, 1821, 32, 64, 32, 0, 0, 256, 0, 0, 875, 1666, 25, 52, 25, 8},
	{2, 0, 471, 1803, 18, 36, 18, 935, 1596, 20, 2025, 20, 882, 1669, 338, 1544, 338, 7},
	{4, 0, 408, 1888, 53, 106, 53, 924, 1605, 37, 1985, 37, 945, 1558, 284, 1627, 284, 7}
	};

/* LCN40PHY IQCAL parameters for various Tx gain settings */
/* table format: */
/*	target, gm, pga, pad, ncorr for each of 5 cal types */
typedef uint16 iqcal_gain_params_lcn40phy[9];

static const iqcal_gain_params_lcn40phy tbl_iqcal_gainparams_lcn40phy_2G[] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	};

#ifdef BAND5G
static const iqcal_gain_params_lcn40phy tbl_iqcal_gainparams_lcn40phy_5G[] = {
	{0, 7, 14, 14, 0, 0, 0, 0, 0},
	};
static const iqcal_gain_params_lcn40phy *tbl_iqcal_gainparams_lcn40phy[2] = {
	tbl_iqcal_gainparams_lcn40phy_2G,
	tbl_iqcal_gainparams_lcn40phy_5G
	};
static const uint16 iqcal_gainparams_numgains_lcn40phy[2] = {
	sizeof(tbl_iqcal_gainparams_lcn40phy_2G) / sizeof(*tbl_iqcal_gainparams_lcn40phy_2G),
	sizeof(tbl_iqcal_gainparams_lcn40phy_5G) / sizeof(*tbl_iqcal_gainparams_lcn40phy_5G)
	};
#else
static const iqcal_gain_params_lcn40phy *tbl_iqcal_gainparams_lcn40phy[1] = {
	tbl_iqcal_gainparams_lcn40phy_2G,
	};

static const uint16 iqcal_gainparams_numgains_lcn40phy[1] = {
	sizeof(tbl_iqcal_gainparams_lcn40phy_2G) / sizeof(*tbl_iqcal_gainparams_lcn40phy_2G),
	};
#endif /* BAND5G */

static const phy_sfo_cfg_t lcn40phy_sfo_cfg[] = {
	{965, 1087},
	{967, 1085},
	{969, 1082},
	{971, 1080},
	{973, 1078},
	{975, 1076},
	{977, 1073},
	{979, 1071},
	{981, 1069},
	{983, 1067},
	{985, 1065},
	{987, 1063},
	{989, 1060},
	{994, 1055}
};

/* LO Comp Gain ladder. Format: {m genv} */
static const
uint16 lcn40phy_iqcal_loft_gainladder[]  = {
	((2 << 8) | 0),
	((3 << 8) | 0),
	((4 << 8) | 0),
	((6 << 8) | 0),
	((8 << 8) | 0),
	((11 << 8) | 0),
	((16 << 8) | 0),
	((16 << 8) | 1),
	((16 << 8) | 2),
	((16 << 8) | 3),
	((16 << 8) | 4),
	((16 << 8) | 5),
	((16 << 8) | 6),
	((16 << 8) | 7),
	((23 << 8) | 7),
	((32 << 8) | 7),
	((45 << 8) | 7),
	((64 << 8) | 7),
	((91 << 8) | 7),
	((128 << 8) | 7)
};

/* Image Rejection Gain ladder. Format: {m genv} */
static const
uint16 lcn40phy_iqcal_ir_gainladder[] = {
	((1 << 8) | 0),
	((2 << 8) | 0),
	((4 << 8) | 0),
	((6 << 8) | 0),
	((8 << 8) | 0),
	((11 << 8) | 0),
	((16 << 8) | 0),
	((23 << 8) | 0),
	((32 << 8) | 0),
	((45 << 8) | 0),
	((64 << 8) | 0),
	((64 << 8) | 1),
	((64 << 8) | 2),
	((64 << 8) | 3),
	((64 << 8) | 4),
	((64 << 8) | 5),
	((64 << 8) | 6),
	((64 << 8) | 7),
	((91 << 8) | 7),
	((128 << 8) | 7)
};
static const
uint16 rxiq_cal_rf_reg[] = {
	RADIO_2065_LPF_CFG1,
	RADIO_2065_LPF_CFG2,
	RADIO_2065_LPF_RESP_BQ1,
	RADIO_2065_LPF_RESP_BQ2,
	RADIO_2065_AUXPGA_CFG1,
	RADIO_2065_OVR1,
	RADIO_2065_OVR2,
	RADIO_2065_OVR5,
	RADIO_2065_OVR6,
	RADIO_2065_OVR7,
	RADIO_2065_OVR8,
	RADIO_2065_OVR11,
	RADIO_2065_OVR12,
	RADIO_2065_LPF_BIAS0,
#ifdef BAND5G
#endif
	};

static const
uint16 rxiq_cal_phy_reg[] = {
	LCN40PHY_RFOverride0,
	LCN40PHY_RFOverrideVal0,
	LCN40PHY_rfoverride2,
	LCN40PHY_rfoverride2val,
	LCN40PHY_rfoverride4,
	LCN40PHY_rfoverride4val,
	LCN40PHY_rfoverride7,
	LCN40PHY_rfoverride7val,
	LCN40PHY_rfoverride8,
	LCN40PHY_rfoverride8val,
	LCN40PHY_Core1TxControl,
	LCN40PHY_AfeCtrlOvr1,
	LCN40PHY_AfeCtrlOvr1Val,
	LCN40PHY_sslpnCalibClkEnCtrl,
	LCN40PHY_sslpnRxFeClkEnCtrl,
	LCN40PHY_lpfgainlutreg,
	};
/* #define lcn40phy routines */
#define wlc_lcn40phy_common_read_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	wlc_phy_common_read_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcn40phy_read_table)

#define wlc_lcn40phy_common_write_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	wlc_phy_common_write_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcn40phy_write_table)

#define wlc_lcn40phy_set_start_tx_pwr_idx(pi, idx) \
	phy_reg_mod(pi, LCN40PHY_TxPwrCtrlCmd, \
		LCN40PHY_TxPwrCtrlCmd_pwrIndex_init_MASK, \
		(uint16)(idx*2) << LCN40PHY_TxPwrCtrlCmd_pwrIndex_init_SHIFT)

#define wlc_lcn40phy_set_tx_pwr_npt(pi, npt) \
	phy_reg_mod(pi, LCN40PHY_TxPwrCtrlNnum, \
		LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_MASK, \
		(uint16)(npt) << LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)

#define wlc_lcn40phy_get_tx_pwr_ctrl(pi) \
	(phy_reg_read((pi), LCN40PHY_TxPwrCtrlCmd) & \
			(LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
			LCN40PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK | \
			LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK))

#define wlc_lcn40phy_get_tx_pwr_npt(pi) \
	((phy_reg_read(pi, LCN40PHY_TxPwrCtrlNnum) & \
		LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_MASK) >> \
		LCN40PHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)

/* the bitsize of the register is 9 bits for lcn40phy */
#define wlc_lcn40phy_get_current_tx_pwr_idx_if_pwrctrl_on(pi) \
	(phy_reg_read(pi, LCN40PHY_TxPwrCtrlStatusExt) & 0x1ff)

#define wlc_lcn40phy_get_target_tx_pwr(pi) \
	((phy_reg_read(pi, LCN40PHY_TxPwrCtrlTargetPwr) & \
		LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_MASK) >> \
		LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

#define wlc_lcn40phy_set_target_tx_pwr(pi, target) \
	phy_reg_mod(pi, LCN40PHY_TxPwrCtrlTargetPwr, \
		LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_MASK, \
		(uint16)MAX(pi->u.pi_lcn40phy->lcnphycommon.tssi_minpwr_limit, \
		(MIN(pi->u.pi_lcn40phy->lcnphycommon.tssi_maxpwr_limit, \
		(uint16)(target)))) << LCN40PHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

#define LCN40PHY_IQLOCC_READ(val) ((uint8)(-(int8)(((val) & 0xf0) >> 4) + (int8)((val) & 0x0f)))
#define FIXED_TXPWR 78
#define LCN40PHY_TEMPSENSE(val) ((int16)((val > 255)?(val - 512):val))

/* can only be accessed by wlc_phy_cmn.c by function pointer */
static void wlc_phy_init_lcn40phy(phy_info_t *pi);
static void wlc_phy_cal_init_lcn40phy(phy_info_t *pi);
static void wlc_phy_detach_lcn40phy(phy_info_t *pi);
static void wlc_phy_chanspec_set_lcn40phy(phy_info_t *pi, chanspec_t chanspec);
static void wlc_phy_txpower_recalc_target_lcn40phy(phy_info_t *pi);

static void wlc_lcn40phy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode);
static void wlc_lcn40phy_set_tx_pwr_by_index(phy_info_t *pi, int indx);

static void wlc_lcn40phy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b);
static void wlc_lcn40phy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b);
static uint16 wlc_lcn40phy_get_tx_locc(phy_info_t *pi);
static void wlc_lcn40phy_set_tx_locc(phy_info_t *pi, uint16 didq);
static void wlc_lcn40phy_get_radio_loft(phy_info_t *pi, uint8 *ei0,
	uint8 *eq0, uint8 *fi0, uint8 *fq0);
static void wlc_lcn40phy_set_radio_loft(phy_info_t *pi, uint8, uint8, uint8, uint8);

#if defined(WLTEST)
static void wlc_phy_carrier_suppress_lcn40phy(phy_info_t *pi);
static void wlc_lcn40phy_reset_radio_loft(phy_info_t *pi);
#endif 
#if defined(BCMDBG) || defined(WLTEST)
static int wlc_phy_long_train_lcn40phy(phy_info_t *pi, int channel);
#endif 
/* LCN40PHY static function declaration */
static bool wlc_lcn40phy_txpwr_srom_read(phy_info_t *pi);
static void wlc_lcn40phy_noise_attach(phy_info_t *pi);
static void wlc_lcn40phy_radio_init(phy_info_t *pi);
static void wlc_lcn40phy_radio_reset(phy_info_t *pi);
static void wlc_lcn40phy_rcal(phy_info_t *pi);
static void wlc_lcn40phy_rc_cal(phy_info_t *pi);
static void wlc_lcn40phy_minipmu_cal(phy_info_t *pi);
static void wlc_lcn40phy_baseband_init(phy_info_t *pi);
static void	wlc_lcn40phy_tx_pwr_ctrl_init(phy_info_t *pi);
static void	wlc_lcn40phy_agc_temp_init(phy_info_t *pi);
static void	wlc_lcn40phy_temp_adj(phy_info_t *pi);
static void wlc_lcn40phy_noise_init(phy_info_t *pi);
static void wlc_lcn40phy_rev0_reg_init(phy_info_t *pi);
static void wlc_lcn40phy_rev1_reg_init(phy_info_t *pi);
static void wlc_lcn40phy_bu_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_tbl_init(phy_info_t *pi);
static void wlc_lcn40phy_clear_papd_comptable(phy_info_t *pi);
static void wlc_lcn40phy_force_pwr_index(phy_info_t *pi, int indx);
static void wlc_lcn40phy_txpower_recalc_target(phy_info_t *pi);
static void wlc_lcn40phy_set_tx_gain_override(phy_info_t *pi, bool bEnable);
static void wlc_lcn40phy_set_tx_gain(phy_info_t *pi,  phy_txgains_t *target_gains);
/* static uint8 wlc_lcn40phy_get_bbmult(phy_info_t *pi); */
static void wlc_lcn40phy_set_pa_gain(phy_info_t *pi, uint16 gain);
static void wlc_lcn40phy_set_dac_gain(phy_info_t *pi, uint16 dac_gain);
static uint16 wlc_lcn40phy_get_pa_gain(phy_info_t *pi);
void wlc_lcn40phy_set_bbmult(phy_info_t *pi, uint8 m0);
static void wlc_lcn40phy_txpower_reset_npt(phy_info_t *pi);
static void wlc_lcn40phy_set_chanspec_tweaks(phy_info_t *pi, chanspec_t chanspec);
static void wlc_lcn40phy_restore_calibration_results(phy_info_t *pi);
static void wlc_lcn40phy_agc_reset(phy_info_t *pi);
static void wlc_lcn40phy_radio_2065_channel_tune(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_decode_aa2g(phy_info_t *pi, uint8 val);
static bool wlc_lcn40phy_cal_reqd(phy_info_t *pi);
static void wlc_lcn40phy_periodic_cal(phy_info_t *pi);
static void wlc_phy_watchdog_lcn40phy(phy_info_t *pi);
static void wlc_lcn40phy_btc_adjust(phy_info_t *pi, bool btactive);
static void wlc_lcn40phy_txpwrtbl_iqlo_cal(phy_info_t *pi);
static bool wlc_lcn40phy_rx_iq_cal(phy_info_t *pi, const phy_rx_iqcomp_t *iqcomp, int iqcomp_sz,
	bool tx_switch, bool rx_switch, int module, int tx_gain_idx);
static void wlc_lcn40phy_papd_cal(phy_info_t *pi, phy_papd_cal_type_t cal_type,
	phy_txcalgains_t *txgains, bool frcRxGnCtrl, bool txGnCtrl, bool samplecapture,
	bool papd_dbg_mode, uint8 num_symbols, uint8 init_papd_lut);
static void wlc_lcn40phy_idle_tssi_est(phy_info_t *pi);
static void wlc_lcn40phy_set_estPwrLUT(phy_info_t *pi, int32 lut_num);
static void wlc_lcn40phy_restore_txiqlo_calibration_results(phy_info_t *pi, uint16 startidx,
	uint16 stopidx, uint8 index);
static void wlc_lcn40phy_restore_papd_calibration_results(phy_info_t *pi);
static void wlc_lcn40phy_set_rx_iq_comp(phy_info_t *pi, uint16 a, uint16 b);
static void wlc_lcn40phy_get_tx_gain(phy_info_t *pi,  phy_txgains_t *gains);
static uint8 wlc_lcn40phy_get_bbmult(phy_info_t *pi);
static void wlc_lcn40phy_tx_iqlo_cal(phy_info_t *pi, phy_txgains_t *target_gains,
	phy_cal_mode_t cal_mode, bool keep_tone);
static void wlc_2065_vco_cal(phy_info_t *pi, bool legacy);
static void wlc_lcn40phy_tx_farrow_init(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_rx_farrow_init(phy_info_t *pi, uint8 channel);
static chan_info_2065_lcn40phy_t *wlc_lcn40phy_find_channel(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_tx_iqlo_loopback(phy_info_t *pi, uint16 *values_to_save);
static void wlc_lcn40phy_tx_iqlo_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save);
static bool wlc_lcn40phy_iqcal_wait(phy_info_t *pi);
static void wlc_lcn40phy_set_tssi_pwr_limit(phy_info_t *pi, uint8 mode);
static void wlc_lcn40phy_tssi_setup(phy_info_t *pi);
static void wlc_lcn40phy_save_restore_dig_filt_state(phy_info_t *pi, bool save, uint16 *filtcoeffs);
static int wlc_lcn40phy_load_tx_iir_filter(phy_info_t *pi, phy_tx_iir_filter_mode_t filter_mode,
	int16 filt_type);
static void wlc_lcn40phy_rx_gain_override_enable(phy_info_t *pi, bool enable);
static void wlc_lcn40phy_rx_pu(phy_info_t *pi, bool bEnable);
static void wlc_lcn40phy_clear_trsw_override(phy_info_t *pi);

static uint32 wlc_lcn40phy_papd_rxGnCtrl(phy_info_t *pi, phy_papd_cal_type_t cal_type,
	bool frcRxGnCtrl, uint8 CurTxGain);

static void wlc_lcn40phy_GetpapdMaxMinIdxupdt(phy_info_t *pi,
	int16 *maxUpdtIdx, int16 *minUpdtIdx);

static void wlc_lcn40phy_save_papd_calibration_results(phy_info_t *pi);
static void wlc_lcn40phy_set_rx_gain_by_distribution(phy_info_t *pi, uint16 trsw, uint16 ext_lna,
	uint16 slna_byp, uint16 slna_rout, uint16 slna_gain, uint16 lna2_gain, uint16 lna2_rout,
	uint16 tia, uint16 biq1, uint16 biq2, uint16 digi_gain, uint16 digi_offset);

static void
wlc_lcn40phy_papd_cal_core(
	phy_info_t *pi,
	phy_papd_cal_type_t calType,
	bool rxGnCtrl,
	bool txGnCtrl,
	bool samplecapture,
	bool papd_dbg_mode,
	uint16 num_symbols,
	bool init_papd_lut,
	uint16 papd_bbmult_init,
	uint16 papd_bbmult_step,
	bool papd_lpgn_ovr,
	uint16 LPGN_I,
	uint16 LPGN_Q);

static void wlc_lcn40phy_papd_cal_setup_cw(phy_info_t *pi);
static void wlc_lcn40phy_epa_pd(phy_info_t *pi, bool disable);
static bool wlc_lcn40phy_rx_iq_est(phy_info_t *pi, uint16 num_samps, uint8 wait_time,
	phy_iq_est_t *iq_est);
static bool wlc_lcn40phy_calc_rx_iq_comp(phy_info_t *pi,  uint16 num_samps);
static void wlc_lcn40phy_run_samples(phy_info_t *pi, uint16 num_samps, uint16 num_loops,
	uint16 wait, bool iqcalmode);
static void wlc_lcn40phy_set_trsw_override(phy_info_t *pi, bool tx, bool rx);
static void wlc_lcn40phy_papd_calc_capindex(phy_info_t *pi, phy_txcalgains_t *txgains);
static void wlc_lcn40phy_load_txgainwithcappedindex(phy_info_t *pi, bool cap);
static void wlc_lcn40phy_switch_radio(phy_info_t *pi, bool on);
static void wlc_lcn40phy_anacore(phy_info_t *pi, bool on);
static void wlc_lcn40phy_filt_bw_set(phy_info_t *pi, uint16 bw);
static void wlc_lcn40phy_clkstall_WAR(phy_info_t *pi);
static void wlc_lcn40phy_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev0_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev1_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_rev3_agc_tweaks(phy_info_t *pi);
static void wlc_lcn40phy_tx_vco_freq_divider(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_set_sfo_chan_centers(phy_info_t *pi, uint8 channel);
static void wlc_lcn40phy_adc_init(phy_info_t *pi, phy_adc_mode_t adc_mode, bool cal_mode);
static bool wlc_lcn40phy_is_papd_block_enable(phy_info_t *pi);
static void wlc_lcn40phy_papd_block_enable(phy_info_t *pi, bool enable);
static void wlc_lcn40phy_play_sample_table1(phy_info_t *pi, int32 f_Hz, uint16 max_val);
static void wlc_lcn40phy_tx_tone_samples(phy_info_t *pi, int32 f_Hz, uint16 max_val,
	uint32 *data_buf, uint32 phy_bw, uint16 num_samps);
static uint16 wlc_lcn40phy_num_samples(phy_info_t *pi, int32 f_Hz, uint32 phy_bw);
/* ZYX */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  function implementation   					*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* ATTACH */
bool
wlc_phy_attach_lcn40phy(phy_info_t *pi)
{
	phy_info_lcnphy_t* pi_lcn;

	pi->u.pi_lcn40phy = (phy_info_lcn40phy_t*)MALLOC(pi->sh->osh, sizeof(phy_info_lcn40phy_t));
	if (pi->u.pi_lcn40phy == NULL) {
	PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	bzero((char *)pi->u.pi_lcn40phy, sizeof(phy_info_lcn40phy_t));

	pi_lcn = wlc_phy_getlcnphy_common(pi);

#if defined(PHYCAL_CACHING)
	/* Reset the var as no cal cache context should exist yet */
	pi->phy_calcache_num = 0;
#endif
	if (!NORADIO_ENAB(pi->pubpi)) {
		pi->hwpwrctrl = TRUE;
		pi->hwpwrctrl_capable = TRUE;
	}
	/* Get xtal frequency from PMU */
	pi->xtalfreq = si_alp_clock(pi->sh->sih);
	ASSERT((pi->xtalfreq % 1000) == 0);

	/* set papd_rxGnCtrl_init to 0 */
	pi_lcn->lcnphy_papd_rxGnCtrl_init = 0;

	PHY_INFORM(("wl%d: %s: using %d.%d MHz xtalfreq for RF PLL\n",
		pi->sh->unit, __FUNCTION__,
		pi->xtalfreq / 1000000, pi->xtalfreq % 1000000));

	pi->pi_fptr.detach = wlc_phy_detach_lcn40phy;
	pi->pi_fptr.init = wlc_phy_init_lcn40phy;
	pi->pi_fptr.calinit = wlc_phy_cal_init_lcn40phy;
	pi->pi_fptr.chanset = wlc_phy_chanspec_set_lcn40phy;
	pi->pi_fptr.txpwrrecalc = wlc_phy_txpower_recalc_target_lcn40phy;
	pi->pi_fptr.settxpwrctrl = wlc_lcn40phy_set_tx_pwr_ctrl;
	pi->pi_fptr.settxpwrbyindex = wlc_lcn40phy_set_tx_pwr_by_index;
	pi->pi_fptr.ishwtxpwrctrl = wlc_phy_tpc_iovar_isenabled_lcn40phy;

	pi->pi_fptr.txiqccget = wlc_lcn40phy_get_tx_iqcc;
	pi->pi_fptr.txiqccset = wlc_lcn40phy_set_tx_iqcc;
	pi->pi_fptr.txloccget = wlc_lcn40phy_get_tx_locc;
	pi->pi_fptr.txloccset = wlc_lcn40phy_set_tx_locc;
	pi->pi_fptr.radioloftget = wlc_lcn40phy_get_radio_loft;
	pi->pi_fptr.radioloftset = wlc_lcn40phy_set_radio_loft;
	pi->pi_fptr.phywatchdog = wlc_phy_watchdog_lcn40phy;
	pi->pi_fptr.phybtcadjust = wlc_lcn40phy_btc_adjust;
#if defined(BCMDBG) || defined(WLTEST)
	pi->pi_fptr.longtrn = wlc_phy_long_train_lcn40phy;
#endif
#if defined(WLTEST)
	pi->pi_fptr.carrsuppr = wlc_phy_carrier_suppress_lcn40phy;
#endif
	pi->pi_fptr.switchradio = wlc_lcn40phy_switch_radio;
	pi->pi_fptr.anacore = wlc_lcn40phy_anacore;
	pi->pi_fptr.phywritetable = wlc_lcn40phy_write_table;
	pi->pi_fptr.phyreadtable = wlc_lcn40phy_read_table;
	pi->pi_fptr.calibmodes = wlc_lcn40phy_calib_modes;

	if (!wlc_lcn40phy_txpwr_srom_read(pi))
		return FALSE;

	wlc_lcn40phy_noise_attach(pi);

	return TRUE;
}

static void
wlc_phy_detach_lcn40phy(phy_info_t *pi)
{
	MFREE(pi->sh->osh, pi->u.pi_lcn40phy, sizeof(phy_info_lcn40phy_t));
}

static void
wlc_lcn40phy_clkstall_WAR(phy_info_t *pi)
{
	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x0005);
	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x0);
}

/* INIT */
static void
WLBANDINITFN(wlc_phy_init_lcn40phy)(phy_info_t *pi)
{
	uint8 phybw40;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	phybw40 = CHSPEC_IS40(pi->radio_chanspec);

	PHY_TRACE(("%s:\n", __FUNCTION__));
	pi_lcn->lcnphy_cal_counter = 0;
	pi_lcn->lcnphy_capped_index = 0;
	pi_lcn->lcnphy_calreqd = 0;
	pi_lcn->lcnphy_CalcPapdCapEnable = 0;
#if !defined(PHYCAL_CACHING)
	pi_lcn->lcnphy_cal_temper = pi_lcn->lcnphy_rawtempsense;
#endif
	if ((CHSPEC_IS2G(pi->radio_chanspec) &&
		pi_lcn->extpagain2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) &&
		pi_lcn->extpagain5g))
		pi_lcn->ePA = 1;
	/* PAPD enabled for ePA */
	if (!pi_lcn->ePA)
		pi_lcn->lcnphy_papd_4336_mode = TRUE;
	wlc_btcx_override_enable(pi);
	/* reset the radio */
	wlc_lcn40phy_radio_reset(pi);

	/* Initialize baseband : bu_tweaks is a placeholder */
	wlc_lcn40phy_baseband_init(pi);

	/* Initialize radio */
	wlc_lcn40phy_radio_init(pi);
	/* run minipmu cal */
	wlc_lcn40phy_minipmu_cal(pi);
	/* run rcal */
	wlc_lcn40phy_rcal(pi);
	/* run rc_cal */
	wlc_lcn40phy_rc_cal(pi);
	/* Initialize power control */
	wlc_lcn40phy_tx_pwr_ctrl_init(pi);
	/* Tune to the current channel */
	/* mapped to lcn40phy_set_chan_raw minus the agc_temp_init, txpwrctrl init */
	wlc_phy_chanspec_set((wlc_phy_t*)pi, pi->radio_chanspec);

	if (LCN40REV_LE(pi->pubpi.phy_rev, 3))
		wlc_lcn40phy_clkstall_WAR(pi);

	/* save default params for AGC temp adjustments */
	wlc_lcn40phy_agc_temp_init(pi);

	wlc_lcn40phy_temp_adj(pi);

	pi_lcn->lcnphy_noise_samples = PHY_NOISE_SAMPLES_DEFAULT;

}

static void
wlc_lcn40phy_agc_tweaks(phy_info_t *pi)
{
	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 4))
		wlc_lcn40phy_rev0_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 1))
		wlc_lcn40phy_rev1_agc_tweaks(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 3))
		wlc_lcn40phy_rev3_agc_tweaks(pi);
	else
		wlc_lcn40phy_rev0_agc_tweaks(pi);
}

static void
wlc_lcn40phy_rev0_agc_tweaks(phy_info_t *pi)
{
	uint16 clip_gain_2g[] = {56, 45, 42, 37, 32, 27, 23, 14, 11};
	uint16 clip_thresh_2g[] = {15, 12, 10, 10, 10, 10, 10, 10, 10};
	uint16 clip_gain_2g_extlna[] = {66, 61, 56, 49, 41, 36, 32, 13, 10};
	uint16 clip_thresh_2g_extlna[] = {10, 10, 10, 10, 10, 19, 10, 10, 10};
#ifdef BAND5G
	uint16 clip_gain_5g[] = {67, 63, 57, 47, 41, 36, 30, 17, 10};
	uint16 clip_gain_5g_extlna[] = {58, 50, 43, 38, 34, 31, 25, 20, 10};
	uint16 clip_thresh_5g[] = {30, 30, 30, 10, 10, 10, 15, 20, 10};
	uint16 clip_thresh_5g_extlna[] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
#endif
	uint16 *clip_gains = NULL, *clip_threshs = NULL;

	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	PHY_REG_MOD(pi, LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 15);
	mod_radio_reg(pi, RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303);
	write_radio_reg(pi, RADIO_2065_NBRSSI_IB, 0x36e2);
	mod_radio_reg(pi, RADIO_2065_NBRSSI_CONFG, 0x7000, 0x3000);
	PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl0, matchFiltEn, 1);
	PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl0_new, matchFiltEn40, 1);
	PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0);
	PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt, 3);

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, ClipDetector, Clip_detecotr_thresh, 40);
	}
	/* Tuning to avoid being stuck at BPHY_CONFIRM, an FSM bug */
	PHY_REG_MOD(pi, LCN40PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 3);
	/* bit inversion for 4334 A0 and 4334 A2 only */
	if (LCN40REV_LE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_pkt_rcv_clip_det_abort_disable, 1);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			if (pi_lcn40->trGain != 0xFF) {
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh,
					pi_lcn40->trGain);
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
					pi_lcn->lcnphy_tr_isolation_mid);
			}
			clip_gains = clip_gain_2g_extlna;
			clip_threshs = clip_thresh_2g_extlna;
		} else {
			clip_gains = clip_gain_2g;
			clip_threshs = clip_thresh_2g;
		}

		write_radio_reg(pi, RADIO_2065_LNA2G_RSSI, 0x51f1);
		PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 62);
	}
#ifdef BAND5G
	else {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
			clip_gains = clip_gain_5g_extlna;
			clip_threshs = clip_thresh_5g_extlna;
			PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 60);
			PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
			PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, trGainThresh, 35);
			if (pi_lcn->triso5g[0] != 0xff)
				PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
					pi_lcn->triso5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_adj, 0);
		} else {
			clip_gains = clip_gain_5g;
			clip_threshs = clip_thresh_5g;
		}

		write_radio_reg(pi, RADIO_2065_LNA5G_RSSI, 0x51f1);
		PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 0);
		if (pi_lcn40->rx_iq_comp_5g[0])
		{
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa0, a0, pi_lcn40->rx_iq_comp_5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb0, b0, pi_lcn40->rx_iq_comp_5g[1]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa1, a1, pi_lcn40->rx_iq_comp_5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb1, b1, pi_lcn40->rx_iq_comp_5g[1]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa2, a2, pi_lcn40->rx_iq_comp_5g[0]);
			PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb2, b2, pi_lcn40->rx_iq_comp_5g[1]);
		}

		PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3);
	}
#endif  /* BAND5G */

	phy_reg_mod(pi, LCN40PHY_agcControl14, LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
		LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
		(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
	(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl15, LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
		LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
		(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
		(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl16, LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
		LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
		(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
		(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl17, LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
		LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
		(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
		(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);

	if (clip_threshs) {
		phy_reg_mod(pi, LCN40PHY_agcControl19,
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
			LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
			(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
			(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
		phy_reg_mod(pi, LCN40PHY_agcControl20,
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
			LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
			(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
			(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
		phy_reg_mod(pi, LCN40PHY_agcControl21,
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
			LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
			(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
			(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
		phy_reg_mod(pi, LCN40PHY_agcControl22,
			LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
			LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
			(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
			(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, clip_threshs[8]);
	}
	if (CHSPEC_IS2G(pi->radio_chanspec) && (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags)
		& BFL_EXTLNA) && CHSPEC_IS40(pi->radio_chanspec))
		PHY_REG_MOD(pi, LCN40PHY, agcControl21, rssi_clip_thresh_norm_5, 10);

	PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss, 99);
	if (LCN40REV_LE(pi->pubpi.phy_rev, 2))
		PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 94);
	if (LCN40REV_LE(pi->pubpi.phy_rev, 2) && pi->u.pi_lcn40phy->phycrs_war_en)
	{
		phy_reg_write(pi, LCN40PHY_SignalBlock_edet1, 0x5858);
		PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x58);
	}

	wlc_lcn40phy_agc_reset(pi);
}

static void
wlc_lcn40phy_rev3_agc_tweaks(phy_info_t *pi)
{
	uint16 clip_gain_2g_20mhz[] = {50, 46, 40, 36, 31, 26, 15, 9, 4};
	uint16 clip_thresh_2g_20mhz[] = {10, 10, 15, 15, 10, 10, 15, 25, 10};
	uint16 clip_gain_2g_40mhz[] = {50, 46, 40, 36, 30, 26, 18, 10, 4};
	uint16 clip_thresh_2g_40mhz[] = {10, 10, 10, 10, 10, 10, 10, 15, 10};


	uint16 *clip_gains = NULL, *clip_threshs = NULL;
	PHY_REG_MOD(pi, LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 15);
	mod_radio_reg(pi, RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303);
	write_radio_reg(pi, RADIO_2065_NBRSSI_IB, 0x36e2);
	mod_radio_reg(pi, RADIO_2065_NBRSSI_CONFG, 0x7000, 0x3000);
	PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl0, matchFiltEn, 1);
	PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl0_new, matchFiltEn40, 1);
	PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0);
	/* Set gainsettle delay, works better for 4314iPA */
	PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt, 3);

	PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 62);
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		clip_gains = clip_gain_2g_40mhz;
		clip_threshs = clip_thresh_2g_40mhz;
	} else {
		clip_gains = clip_gain_2g_20mhz;
		clip_threshs = clip_thresh_2g_20mhz;
	}

	PHY_REG_MOD(pi, LCN40PHY, agcControl27, data_mode_gain_db_adj_dsss, 0);
	PHY_REG_WRITE(pi, LCN40PHY, ClipThresh, 57);

	write_radio_reg(pi, RADIO_2065_LNA2G_RSSI, 0x51f1);
	PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, 0);
	PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_adj, 0);

	phy_reg_mod(pi, LCN40PHY_agcControl14, LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
		LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
		(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
	(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl15, LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
		LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
		(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
		(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl16, LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
		LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
		(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
		(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl17, LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
		LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
		(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
		(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);

	phy_reg_mod(pi, LCN40PHY_agcControl19,
		LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
		LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
		(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
		(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl20,
		LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
		LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
		(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
		(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl21,
		LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
		LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
		(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
		(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl22,
		LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
		LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
		(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
		(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, clip_threshs[8]);

	/* LOCK LPF HPC at 3 */
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3);

	phy_reg_write(pi, LCN40PHY_SignalBlock_edet1, 0x6161);
	PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x61);

	/* Not synch up with TCL, needs further investigation on actual cause */
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, afe_reset_ov_det_ovr, 0x1);
	PHY_REG_MOD(pi, LCN40PHY, SignalBlock_edet1, signalblk_det_thresh_dsss, 99);

	wlc_lcn40phy_agc_reset(pi);
}


static void
wlc_lcn40phy_rev1_agc_tweaks(phy_info_t *pi)
{
	uint16 clip_gain_2g_20mhz[] = {61, 55, 49, 46, 39, 32, 28, 21, 11};
	uint16 clip_gain_2g_40mhz[] = {60, 54, 49, 44, 39, 32, 28, 21, 11};
	uint16 clip_thresh_2g_20mhz[] = {10, 10, 10, 10, 13, 13, 8, 10, 0};
	uint16 clip_thresh_2g_40mhz[] = {10, 10, 10, 11, 10, 10, 7, 10, 0};

	uint16 *clip_gains = NULL, *clip_threshs = NULL;
	PHY_REG_MOD(pi, LCN40PHY, ClipCtrThresh, ClipCtrThreshHiGain, 30);
	mod_radio_reg(pi, RADIO_2065_NBRSSI_BIAS, 0xff03, 0x303);
	write_radio_reg(pi, RADIO_2065_NBRSSI_IB, 0x36e2);
	mod_radio_reg(pi, RADIO_2065_NBRSSI_CONFG, 0x7000, 0x3000);
	PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl0, matchFiltEn, 1);
	PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl0_new, matchFiltEn40, 1);
	PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0);
	/* Set gainsettle delay, works better for 4314iPA */
	PHY_REG_MOD(pi, LCN40PHY, agcControl11, gain_settle_dly_cnt, 3);

	PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 64);
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, agcControl38, rssi_no_clip_gain_normal, 63);
		clip_gains = clip_gain_2g_40mhz;
		clip_threshs = clip_thresh_2g_40mhz;
	} else {
		clip_gains = clip_gain_2g_20mhz;
		clip_threshs = clip_thresh_2g_20mhz;
	}

	write_radio_reg(pi, RADIO_2065_LNA2G_RSSI, 0x51c1);
	PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, 0);
	PHY_REG_MOD(pi, LCN40PHY, agcControl24, rssi_no_clip_gain_adj, -7);

	phy_reg_mod(pi, LCN40PHY_agcControl14, LCN40PHY_agcControl14_rssi_clip_gain_norm_0_MASK |
		LCN40PHY_agcControl14_rssi_clip_gain_norm_1_MASK,
		(clip_gains[0] << LCN40PHY_agcControl14_rssi_clip_gain_norm_0_SHIFT) |
	(clip_gains[1] << LCN40PHY_agcControl14_rssi_clip_gain_norm_1_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl15, LCN40PHY_agcControl15_rssi_clip_gain_norm_2_MASK |
		LCN40PHY_agcControl15_rssi_clip_gain_norm_3_MASK,
		(clip_gains[2] << LCN40PHY_agcControl15_rssi_clip_gain_norm_2_SHIFT) |
		(clip_gains[3] << LCN40PHY_agcControl15_rssi_clip_gain_norm_3_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl16, LCN40PHY_agcControl16_rssi_clip_gain_norm_4_MASK |
		LCN40PHY_agcControl16_rssi_clip_gain_norm_5_MASK,
		(clip_gains[4] << LCN40PHY_agcControl16_rssi_clip_gain_norm_4_SHIFT) |
		(clip_gains[5] << LCN40PHY_agcControl16_rssi_clip_gain_norm_5_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl17, LCN40PHY_agcControl17_rssi_clip_gain_norm_6_MASK |
		LCN40PHY_agcControl17_rssi_clip_gain_norm_7_MASK,
		(clip_gains[6] << LCN40PHY_agcControl17_rssi_clip_gain_norm_6_SHIFT) |
		(clip_gains[7] << LCN40PHY_agcControl17_rssi_clip_gain_norm_7_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, clip_gains[8]);

	phy_reg_mod(pi, LCN40PHY_agcControl19,
		LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_MASK |
		LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_MASK,
		(clip_threshs[0] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_0_SHIFT) |
		(clip_threshs[1] << LCN40PHY_agcControl19_rssi_clip_thresh_norm_1_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl20,
		LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_MASK |
		LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_MASK,
		(clip_threshs[2] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_2_SHIFT) |
		(clip_threshs[3] << LCN40PHY_agcControl20_rssi_clip_thresh_norm_3_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl21,
		LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_MASK |
		LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_MASK,
		(clip_threshs[4] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_4_SHIFT) |
		(clip_threshs[5] << LCN40PHY_agcControl21_rssi_clip_thresh_norm_5_SHIFT));
	phy_reg_mod(pi, LCN40PHY_agcControl22,
		LCN40PHY_agcControl22_rssi_clip_thres_norm_6_MASK |
		LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_MASK,
		(clip_threshs[6] << LCN40PHY_agcControl22_rssi_clip_thres_norm_6_SHIFT) |
		(clip_threshs[7] << LCN40PHY_agcControl22_rssi_clip_thresh_norm_7_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, agcControl23, rssi_clip_thresh_norm_8, clip_threshs[8]);

	PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x0);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, dc_bw_lock_en, 0);
	PHY_REG_MOD(pi, LCN40PHY, RxRadioControlFltrState, rx_flt_low_start_state, 0);
	PHY_REG_MOD(pi, LCN40PHY, RxRadioControlFltrState, rx_flt_high_start_state, 1);

	phy_reg_write(pi, LCN40PHY_SignalBlock_edet1, 0x4747);
	PHY_REG_MOD(pi, LCN40PHY, CFOblkConfig, signalblk_det_thresh_ofdm_40, 0x61);

	wlc_lcn40phy_agc_reset(pi);
}

/* CHANSPEC */
static void
wlc_lcn40phy_tx_vco_freq_divider(phy_info_t *pi, uint8 channel)
{
	uint freq;
	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);

	ASSERT(chi != NULL);

	freq = chi->freq;

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		if (freq <= 2484 && freq >= 2412)
			/* VCO/18 */
			write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x750);
		else if (freq <= 5320 && freq >= 4920)
			/* VCO/16 */
			write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x710);
		else if (freq <= 5825 && freq >= 5500)
			/* VCO/18 */
			write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x750);
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		if (freq <= 2462 && freq >= 2422)
			/* VCO/9 */
			write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
		else if (freq <= 5310 && freq >= 4930)
			/* VCO/8 */
			write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x700);
		else if (freq <= 5795 && freq >= 5510)
			/* VCO/9 */
			write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
	}
}

static void wlc_lcn40phy_set_sfo_chan_centers(phy_info_t *pi, uint8 channel)
{
	uint freq;
	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	uint tmp;

	ASSERT(chi != NULL);

	freq = chi->freq;
	/* sfo_chan_center_Ts20 = round(fc / 20e6*(ten_mhz+1) * 8), fc in Hz
	*                      = round($channel * 0.4 *($ten_mhz+1)), $channel in MHz
	*/
	 tmp = (freq * 4 / 5 + 1) >> 1;
	PHY_REG_MOD(pi, LCN40PHY, ptcentreTs20, centreTs20, tmp);
	/* sfo_chan_center_factor = round(2^17 ./ (fc/20e6)/(ten_mhz+1)), fc in Hz
	*                        = round(2621440 ./ $channel/($ten_mhz+1)), $channel in MHz
	*/
	tmp = (2621440 * 2 / freq + 1) >> 1;
	PHY_REG_MOD(pi, LCN40PHY, ptcentreFactor, centreFactor, tmp);
}

static void
wlc_lcn40phy_tx_init(phy_info_t *pi, uint8 channel)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	int16 ofdm_dig_type, cck_dig_type, ofdm40_dig_type;
	wlc_lcn40phy_tx_farrow_init(pi, channel);

	/* On 4334-A0, need to swap I/Q rail. Swap I/Q at LOFTcomp output */
	PHY_REG_MOD(pi, LCN40PHY, adcCompCtrl, flipiq_dacin, 1);
	PHY_REG_MOD(pi, LCN40PHY, txTailCountValue, TailCountValue, 0x45);

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		and_radio_reg(pi, RADIO_2065_DAC_CFG1, ~0x1000);
		wlc_lcn40phy_filt_bw_set(pi, 3);
		if (pi_lcn->ePA) {

			ofdm_dig_type = pi_lcn->lcnphy_ofdm_dig_filt_type;
			cck_dig_type = pi_lcn->lcnphy_cck_dig_filt_type;
			if (ofdm_dig_type == -1)
				ofdm_dig_type = 0;
			if (cck_dig_type == -1)
				cck_dig_type = 21;
			if (channel == 14)
				cck_dig_type = 30;
		} else {
			ofdm_dig_type = 0;
			cck_dig_type = 21;
		}
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_CCK, cck_dig_type);
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, ofdm_dig_type);
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM40, 0);
	} else {
		if (pi_lcn->ePA) {
			ofdm_dig_type = 5;
		} else {
			ofdm_dig_type = 2;
		}

		cck_dig_type = 50, ofdm40_dig_type = 3;
		or_radio_reg(pi, RADIO_2065_DAC_CFG1, 0x1000);
		wlc_lcn40phy_filt_bw_set(pi, 4);
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_CCK, cck_dig_type);
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, ofdm_dig_type);
		wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM40, ofdm40_dig_type);
	}

	pi_lcn40->tx_iir_filter_type_ofdm = ofdm_dig_type;

	PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, txfiltSelect, 2);
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		and_radio_reg(pi, RADIO_2065_OVR13, ~0x4);
		and_radio_reg(pi, RADIO_2065_OVR8, ~0x1);
		and_radio_reg(pi, RADIO_2065_PMU_CFG2, ~0x8);
		mod_radio_reg(pi, RADIO_2065_PMU_CFG2, 0x7, 0x3);
		mod_radio_reg(pi, RADIO_2065_PMU_CFG3, 0xcf, 0x80);
		and_radio_reg(pi, RADIO_2065_PMU_OP, (uint16)~0x8000);
		or_radio_reg(pi, RADIO_2065_TX2G_CFG1, 0x2);
		or_radio_reg(pi, RADIO_2065_OVR12, 0x20);
	}
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		/* 4334A0 logen5g idac currents */
		write_radio_reg(pi, RADIO_2065_LOGEN5G_IDAC1, 0x7333);
		write_radio_reg(pi, RADIO_2065_LOGEN5G_IDAC2, 0x7204);
		write_radio_reg(pi, RADIO_2065_LOGEN5G_IDAC3, 0x7744);
		mod_radio_reg(pi, RADIO_2065_TXGM_CFG1, 0x3700, 0);

		or_radio_reg(pi, RADIO_2065_OVR13, 0x4);
		or_radio_reg(pi, RADIO_2065_OVR8, 0x1);

		or_radio_reg(pi, RADIO_2065_PMU_CFG2, 0x8);
		mod_radio_reg(pi, RADIO_2065_PMU_CFG3, 0xf, 0xb);
		or_radio_reg(pi, RADIO_2065_PMU_OP, 0x8000);
		/* lower VCO related spurs */
		or_radio_reg(pi, RADIO_2065_TXMIX5G_CFG1, 0xf000);
		or_radio_reg(pi, RADIO_2065_PGA5G_CFG1, 0xf000);
		or_radio_reg(pi, RADIO_2065_TX5G_CFG1, 0x2);
		or_radio_reg(pi, RADIO_2065_OVR13, 0x4000);
	}
#endif /* BAND5G */
}

static void
wlc_phy_chanspec_set_lcn40phy(phy_info_t *pi, chanspec_t chanspec)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint8 channel = CHSPEC_CHANNEL(chanspec); /* see wlioctl.h */
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, chanspec);
#endif

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);
	/* Set the phy bandwidth as dictated by the chanspec */
	if (CHSPEC_BW(chanspec) != pi->bw)
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(chanspec));

	wlc_lcn40phy_set_sfo_chan_centers(pi, channel);
	/* spur and stuff */
	wlc_lcn40phy_set_chanspec_tweaks(pi, pi->radio_chanspec);

	/* lcn40phy_agc_reset */
	wlc_lcn40phy_agc_reset(pi);

	/* Tune radio for the channel */
	if (!NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn40phy_radio_2065_channel_tune(pi, channel);
		OSL_DELAY(1000);
	} else
		return;

	wlc_lcn40phy_tx_vco_freq_divider(pi, channel);
	wlc_lcn40phy_tx_init(pi, channel);
	wlc_lcn40phy_rx_farrow_init(pi, channel);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_pu_ovr, 0);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_pu_ovr, 1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, slna_pu_ovr_val, 1);
	}
	wlc_lcn40phy_noise_measure_start(pi, TRUE);
	/* Perform Tx IQ, Rx IQ and PAPD cal only if no scan in progress */
	if (!wlc_phy_no_cal_possible(pi))
	{
#if defined(PHYCAL_CACHING)
		/* Fresh calibration or restoration required */
		if (!ctx) {
			if (LCN40PHY_MAX_CAL_CACHE == pi->phy_calcache_num) {
				/* Already max num ctx exist, reuse oldest */
				ctx = wlc_phy_get_chanctx_oldest(pi);
				ASSERT(ctx);
				wlc_phy_reinit_chanctx(pi, ctx, chanspec);
			} else {
				/* Prepare a fresh calibration context */
				if (BCME_OK == wlc_phy_create_chanctx((wlc_phy_t *)pi,
					pi->radio_chanspec)) {
					ctx = pi->phy_calcache;
					pi->phy_calcache_num++;
				}
				else
					ASSERT(ctx);
			}
		}
#endif /* PHYCAL_CACHING */

#if defined(PHYCAL_CACHING)
		if (!ctx->valid)
#else
			if (pi_lcn->lcnphy_full_cal_channel != CHSPEC_CHANNEL(pi->radio_chanspec))
#endif
				wlc_lcn40phy_calib_modes(pi, PHY_FULLCAL);
			else
				wlc_lcn40phy_restore_calibration_results(pi);
	}

	wlc_lcn40phy_agc_tweaks(pi);
}

static chan_info_2065_lcn40phy_t *
wlc_lcn40phy_find_channel(phy_info_t *pi, uint8 channel)
{
	uint i, length = 0;
	chan_info_2065_lcn40phy_t *chan_info = NULL;

	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID) {
		length = ARRAYSIZE(chan_info_2067_lcn40phy);
		chan_info = chan_info_2067_lcn40phy;
	} else if (RADIOID(pi->pubpi.radioid) == BCM2065_ID) {
		length = ARRAYSIZE(chan_info_2065_lcn40phy);
		chan_info = chan_info_2065_lcn40phy;
	} else {
		length = ARRAYSIZE(chan_info_2067_lcn40phy);
		chan_info = chan_info_2067_lcn40phy;
	}

	/* find freqency given channel */
	for (i = 0; i < length; i++)
		if (chan_info[i].chan == channel) {
			return &chan_info[i];
		}
	return NULL;
}

static void
wlc_lcn40phy_tx_farrow_init(phy_info_t *pi, uint8 channel)
{
	int freq = 0;
	int fi = 0;

	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	ASSERT(chi != NULL);

	freq = chi->freq;

	if (CHSPEC_IS20(pi->radio_chanspec)) {
		if (freq <= 2484 && freq >= 2412)
			fi = 1920;
		else if (freq <= 5320 && freq >= 4920)
			fi = 3840;
		else if (freq <= 5825 && freq >= 5500)
			fi = 4320;
	} else {
		if (freq <= 2462 && freq >= 2422)
			fi = 1920;
		else if (freq <= 5310 && freq >= 4930)
			fi = 3840;
		else if (freq <= 5795 && freq >= 5510)
			fi = 4320;
	}
	ASSERT(fi != 0);
	/* rounding here */
	phy_reg_write(pi, LCN40PHY_tx_resampler1,
		(uint16)((((fi << 16) / freq + 1) >> 1) - (1 << 15)));
	phy_reg_write(pi, LCN40PHY_tx_resampler3, 0);
	phy_reg_write(pi, LCN40PHY_tx_resampler4, freq / wlc_phy_gcd(freq, fi));
}


static void
wlc_lcn40phy_rx_farrow_init(phy_info_t *pi, uint8 channel)
{
	uint32 freq = 0, farrow_mu, diff, highest_period, GCD, rxperiod;

	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	ASSERT(chi != NULL);

	freq = chi->freq;

	if (freq < 3500) {
		/* band G */
		highest_period = 1920;
		/* need rounding here */
		farrow_mu = (((freq << 9) / 3 / 5 + 1) >> 1) - (1 << 15);
	} else if (freq <= 5320) {
		highest_period = 3840;
		farrow_mu = (((freq << 8) / 3 / 5 + 1) >> 1) - (1 << 15);
	} else {
		highest_period = 4320;
		farrow_mu = (((freq << 11) / 27 / 5 + 1) >> 1) - (1 << 15);
	}

	diff = freq - highest_period;
	GCD = wlc_phy_gcd(highest_period, diff);
	rxperiod = highest_period / GCD;

	PHY_REG_MOD(pi, LCN40PHY, rxFarrowDriftPeriod,
		rx_farrow_drift_period, rxperiod);
	PHY_REG_MOD(pi, LCN40PHY, rxFarrowDeltaPhase,
		rx_farrow_mu_delta, farrow_mu);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 1) || LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		PHY_REG_MOD(pi, LCN40PHY, PwrDlyAdj0_new, ofdmfiltDlyAdjustment20, -1);
		PHY_REG_MOD(pi, LCN40PHY, agcControl, ofdm_sync_nom_en, 1);
		PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, 3);
		PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
		PHY_REG_MOD(pi, LCN40PHY, diversityReg, dsssDivTmOutCnt, 8);
		PHY_REG_MOD(pi, LCN40PHY, agcControl8, div_check_cnt, 1);
		PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, gainReqTrAttOnEnByCrs, 1);
		PHY_REG_MOD(pi, LCN40PHY, agcControl, pktRxActiveHoldoff, 4);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, IDLEafterPktRXTimeout, BPHYIdleAfterPktRxTimeOut, 8);
		PHY_REG_MOD(pi, LCN40PHY, crsSyncOffset, incSyncCnt20U, 2);
		PHY_REG_MOD(pi, LCN40PHY, crsSyncOffset, incSyncCnt20L, 2);
		PHY_REG_MOD(pi, LCN40PHY, clipCtrThreshLowGainEx,
			clip_counter_threshold_low_gain_40mhz, 60);
		PHY_REG_MOD(pi, LCN40PHY, ClipCtrThresh, clipCtrThreshLoGain, 30);
		PHY_REG_MOD(pi, LCN40PHY, ClipCtrDefThresh, clipCtrThresh, 40);
		PHY_REG_MOD(pi, LCN40PHY, agcControl27, data_mode_gain_db_adj_dsss, 3);
		PHY_REG_MOD(pi, LCN40PHY, agcControl18, rssi_clip_gain_norm_8, 3);
		PHY_REG_MOD(pi, LCN40PHY, radioTRCtrl, gainrequestTRAttnOnEn, 1);
		PHY_REG_MOD(pi, LCN40PHY, radioTRCtrlCrs1, gainReqTrAttOnEnByCrs, 1);
		PHY_REG_MOD(pi, LCN40PHY, agcSkipCnt, skip_cntr_adj_20L, 4);
		PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl1_new, lp_adj_20L, 1);
		PHY_REG_MOD(pi, LCN40PHY, crsMiscCtrl1_new, lp_adj_40mhz, -9);
		PHY_REG_MOD(pi, LCN40PHY, AgcSynchTiming, skip_out_adj_20L, 1);
		PHY_REG_MOD(pi, LCN40PHY, AgcSynchTiming, skip_out_adj_20U, -1);
		PHY_REG_MOD(pi, LCN40PHY, AgcSynchTiming, skip_out_adj_40mhz, -5);
	}

	PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 3);
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_rx_bw, 2);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride6val, lpf_bias_ovr_val, 0);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_rx_bw, 0);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride6val, lpf_bias_ovr_val, 2);
	}
	PHY_REG_MOD(pi, LCN40PHY, rfoverride6, lpf_bias_ovr, 0x1);

	wlc_lcn40phy_adc_init(pi, CHSPEC_IS20(pi->radio_chanspec) ? ADC_20M : ADC_40M, TRUE);

	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 1);
	/* remove lpf dc offset */
	or_radio_reg(pi, RADIO_2065_OVR7, 0x10);
	and_radio_reg(pi, RADIO_2065_LPF_CFG1, ~0x200);
	/* turn on tia dc loop */
	if (CHIPID(pi->sh->chip) != BCM43142_CHIP_ID) {
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7, tia_dc_loop_bypass_ovr, 1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, tia_dc_loop_bypass_ovr_val, 0);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7, tia_hpc_ovr, 1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, tia_hpc_ovr_val, 1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride8, tia_dc_loop_pu_ovr, 1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride8val, tia_dc_loop_pu_ovr_val, 1);
	}
}

/* CAL */
static void
WLBANDINITFN(wlc_phy_cal_init_lcn40phy)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	return;
}

/* TX power ctrl */
/* %%%%%% major flow operations */
static void
wlc_phy_txpower_recalc_target_lcn40phy(phy_info_t *pi)
{
	uint16 pwr_ctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	wlc_lcn40phy_txpower_recalc_target(pi);
	/* Restore power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, pwr_ctrl);
}

static void
wlc_lcn40phy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode)
{
	uint16 old_mode = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	ASSERT(
		(LCN40PHY_TX_PWR_CTRL_OFF == mode) ||
		(LCN40PHY_TX_PWR_CTRL_SW == mode) ||
		(LCN40PHY_TX_PWR_CTRL_HW == mode));

	/* Setting txfront end clock also along with hwpwr control */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, txFrontEndCalibClkEn,
		(LCN40PHY_TX_PWR_CTRL_HW == mode) ? 1 : 0);
	/* Feed back RF power level to PAPD block */
	PHY_REG_MOD(pi, LCN40PHY, papd_control2, papd_analog_gain_ovr,
		(LCN40PHY_TX_PWR_CTRL_HW == mode) ? 0 : 1);

	if (old_mode != mode) {
		if (LCN40PHY_TX_PWR_CTRL_HW == old_mode) {
			/* Clear out all power offsets */
			wlc_lcn40phy_clear_tx_power_offsets(pi);
			phy_reg_write(pi, LCN40PHY_BBmultCoeffSel, 0);
		}
		if (LCN40PHY_TX_PWR_CTRL_HW == mode) {
			/* Recalculate target power to restore power offsets */
			wlc_lcn40phy_txpower_recalc_target(pi);
			/* Set starting index & NPT to best known values for that target */
			wlc_lcn40phy_set_start_tx_pwr_idx(pi, pi_lcn->lcnphy_tssi_idx);
			wlc_lcn40phy_set_tx_pwr_npt(pi, pi_lcn->lcnphy_tssi_npt);
			phy_reg_write(pi, LCN40PHY_BBmultCoeffSel, 1);
			/* Reset frame counter for NPT calculations */
			pi_lcn->lcnphy_tssi_tx_cnt = PHY_TOTAL_TX_FRAMES(pi);
			/* Disable any gain overrides */
			wlc_lcn40phy_disable_tx_gain_override(pi);
			pi_lcn->lcnphy_tx_power_idx_override = -1;
		}
		else
			wlc_lcn40phy_enable_tx_gain_override(pi);

		/* Set requested tx power control mode */
		phy_reg_mod(pi, LCN40PHY_TxPwrCtrlCmd,
			(LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
			LCN40PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
			LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK),
			mode);

		PHY_INFORM(("wl%d: %s: %s \n", pi->sh->unit, __FUNCTION__,
			mode ? ((LCN40PHY_TX_PWR_CTRL_HW == mode) ? "Auto" : "Manual") : "Off"));
	}
}

static void
wlc_lcn40phy_force_pwr_index(phy_info_t *pi, int indx)
{
	phytbl_info_t tab;
	uint16 a, b;
	uint8 bb_mult;
	uint32 bbmultiqcomp, txgain, locoeffs, rfpower;
	phy_txgains_t gains;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	ASSERT(indx <= LCN40PHY_MAX_TX_POWER_INDEX);

	/* Save forced index */
	pi_lcn->lcnphy_tx_power_idx_override = (int8)indx;
	pi_lcn->lcnphy_current_index = (uint8)indx;

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmultiqcomp; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);

	/* Read index based tx gain from the table */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + indx; /* gainCtrlLuts */
	tab.tbl_width = 32;
	tab.tbl_ptr = &txgain; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);
	/* Apply tx gain */
	gains.gm_gain = (uint16)(txgain & 0xff);
	gains.pga_gain = (uint16)(txgain >> 8) & 0xff;
	gains.pad_gain = (uint16)(txgain >> 16) & 0xff;
	gains.dac_gain = (uint16)(bbmultiqcomp >> 28) & 0x07;
	wlc_lcn40phy_set_tx_gain(pi, &gains);
	wlc_lcn40phy_set_pa_gain(pi,  (uint16)(txgain >> 24) & 0xff);
	/* Apply bb_mult */
	bb_mult = (uint8)((bbmultiqcomp >> 20) & 0xff);
	wlc_lcn40phy_set_bbmult(pi, bb_mult);
	/* Enable gain overrides */
	wlc_lcn40phy_enable_tx_gain_override(pi);
	/* the reading and applying lo, iqcc coefficients is not getting done for 4313A0 */
	/* to be fixed */

	/* Apply iqcc */
	a = (uint16)((bbmultiqcomp >> 10) & 0x3ff);
	b = (uint16)(bbmultiqcomp & 0x3ff);

	if (pi_lcn->lcnphy_cal_results.txiqlocal_a[0]) {
	wlc_lcn40phy_set_tx_iqcc(pi, a, b);
	/* Read index based di & dq from the table */
	}
	if (pi_lcn->lcnphy_cal_results.txiqlocal_didq[0]) {
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + indx; /* loftCoefLuts */
	tab.tbl_ptr = &locoeffs; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);
	/* Apply locc */
	wlc_lcn40phy_set_tx_locc(pi, (uint16)locoeffs);
	}
	/* Apply PAPD rf power correction */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
	tab.tbl_ptr = &rfpower; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);
	PHY_REG_MOD(pi, LCN40PHY, papd_analog_gain_ovr_val,
		papd_analog_gain_ovr_val, rfpower * 8);
	/* set vlin only applies to ePA */
	if (pi_lcn->ePA)
		PHY_REG_MOD(pi, LCN40PHY, rfoverride8val, tx_vlin_ovr_val,
			(rfpower >> 10) & 1);
	else
		PHY_REG_MOD(pi, LCN40PHY, rfoverride8val, tx_vlin_ovr_val,
			(rfpower >> 10) & 0);
}

static void
wlc_lcn40phy_set_tx_pwr_by_index(phy_info_t *pi, int indx)
{
	/* Turn off automatic power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	/* Force tx power from the index */
	wlc_lcn40phy_force_pwr_index(pi, indx);
}
void
wlc_lcn40phy_tx_pwr_update_npt(phy_info_t *pi)
{
	uint16 tx_cnt, tx_total, npt;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	if (LCN40PHY_TX_PWR_CTRL_HW != wlc_lcn40phy_get_tx_pwr_ctrl((pi)))
		return;

	tx_total = PHY_TOTAL_TX_FRAMES(pi);
	tx_cnt = tx_total - pi_lcn->lcnphy_tssi_tx_cnt;
	npt = wlc_lcn40phy_get_tx_pwr_npt(pi);

	if (tx_cnt > (1 << npt)) {
		/* Reset frame counter */
		pi_lcn->lcnphy_tssi_tx_cnt = tx_total;

		/* Set new NPT */
		if (npt < pi_lcn->tssi_max_npt) {
			npt++;
			wlc_lcn40phy_set_tx_pwr_npt(pi, npt);
		}

		/* Update cached power index & NPT */
		pi_lcn->lcnphy_tssi_idx = wlc_lcn40phy_get_current_tx_pwr_idx(pi);
		pi_lcn->lcnphy_tssi_npt = npt;

		/* Also update starting index as an additional safeguard */
		wlc_lcn40phy_set_start_tx_pwr_idx(pi, pi_lcn->lcnphy_tssi_idx);

		PHY_INFORM(("wl%d: %s: Index: %d, NPT: %d, TxCount: %d\n",
			pi->sh->unit, __FUNCTION__, pi_lcn->lcnphy_tssi_idx, npt, tx_cnt));
	}
}

void
wlc_lcn40phy_clear_tx_power_offsets(phy_info_t *pi)
{
	uint32 data_buf[64];
	phytbl_info_t tab;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	/* Clear out buffer */
	bzero(data_buf, sizeof(data_buf));

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = data_buf; /* ptr to buf */
	/* Since 4313A0 uses the rate offset table to do tx pwr ctrl for cck, */
	/* we shouldn't be clearing the rate offset table */

	if (!pi_lcn->lcnphy_uses_rate_offset_table) {
		/* Per rate power offset */
		tab.tbl_len = 30; /* # values   */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcn40phy_write_table(pi, &tab);
	}
	/* Per index power offset */
	tab.tbl_len = 64; /* # values   */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_MAC_OFFSET;
	wlc_lcn40phy_write_table(pi, &tab);
}

int8
wlc_lcn40phy_get_current_tx_pwr_idx(phy_info_t *pi)
{
	int8 indx;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	/* for txpwrctrl_off and tempsense_based pwrctrl, return current_index */
	if (txpwrctrl_off(pi))
		indx = pi_lcn->lcnphy_current_index;
	else
		indx = (int8)(wlc_lcn40phy_get_current_tx_pwr_idx_if_pwrctrl_on(pi)/2);

	return indx;
}

static void
wlc_lcn40phy_switch_radio(phy_info_t *pi, bool on)
{
	/* reset rxafe before switching radio */
	if (LCN40REV_LE(pi->pubpi.phy_rev, 3))
		phy_reg_or(pi, LCN40PHY_resetCtrl, 0x0004);

	if (on) {
		phy_reg_and(pi, LCN40PHY_RFOverride0,
			~(LCN40PHY_RFOverride0_rfpll_pu_ovr_MASK	|
			LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK 	|
			LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
			LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_gmode_rx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK));

		phy_reg_and(pi,  LCN40PHY_rfoverride8,
			~(LCN40PHY_rfoverride8_rxrf_wrssi1_pwrup_ovr_MASK |
			LCN40PHY_rfoverride8_rxrf_wrssi2_pwrup_ovr_MASK |
			LCN40PHY_rfoverride8_lna2_pu_ovr_MASK));

		phy_reg_and(pi, LCN40PHY_rfoverride2,
			~(LCN40PHY_rfoverride2_slna_pu_ovr_MASK));

		phy_reg_and(pi, LCN40PHY_rfoverride3,
			~LCN40PHY_rfoverride3_rfactive_ovr_MASK);
	} else {
		phy_reg_and(pi,  LCN40PHY_RFOverrideVal0,
			~(LCN40PHY_RFOverrideVal0_rfpll_pu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_gmode_rx_pu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_amode_rx_pu_ovr_val_MASK));
		phy_reg_or(pi, LCN40PHY_RFOverride0,
			LCN40PHY_RFOverride0_rfpll_pu_ovr_MASK	|
			LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK |
			LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
			LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_gmode_rx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK);

		phy_reg_and(pi,  LCN40PHY_rfoverride8val,
			~(LCN40PHY_rfoverride8val_rxrf_wrssi1_pwrup_ovr_val_MASK |
			LCN40PHY_rfoverride8val_rxrf_wrssi2_pwrup_ovr_val_MASK |
			LCN40PHY_rfoverride8val_lna2_pu_ovr_val_MASK));

		phy_reg_or(pi,  LCN40PHY_rfoverride8,
			LCN40PHY_rfoverride8_rxrf_wrssi1_pwrup_ovr_MASK |
			LCN40PHY_rfoverride8_rxrf_wrssi2_pwrup_ovr_MASK |
			LCN40PHY_rfoverride8_lna2_pu_ovr_MASK);

		phy_reg_or(pi, LCN40PHY_RFOverride0,
			LCN40PHY_RFOverride0_rfpll_pu_ovr_MASK |
			LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK |
			LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
			LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_gmode_rx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK);

		phy_reg_and(pi, LCN40PHY_rfoverride2val,
			~(LCN40PHY_rfoverride2val_slna_pu_ovr_val_MASK));
		phy_reg_or(pi, LCN40PHY_rfoverride2,
			LCN40PHY_rfoverride2_slna_pu_ovr_MASK);
		phy_reg_and(pi, LCN40PHY_rfoverride3_val,
			~(LCN40PHY_rfoverride3_val_rfactive_ovr_val_MASK));
		phy_reg_or(pi,  LCN40PHY_rfoverride3,
			LCN40PHY_rfoverride3_rfactive_ovr_MASK);
	}
}
static void
wlc_lcn40phy_anacore(phy_info_t *pi, bool on)
{
	/* rxafe needs to be in reset before anacore operation */
	if (LCN40REV_LE(pi->pubpi.phy_rev, 3))
		phy_reg_or(pi, LCN40PHY_resetCtrl, 0x0004);
	if (on) {
		phy_reg_and(pi, LCN40PHY_AfeCtrlOvr1,
			~(LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_MASK));
	} else  {
		phy_reg_and(pi, LCN40PHY_AfeCtrlOvr1Val,
			~(LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_dac_pu_ovr_val_MASK));
		phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_MASK);
	}
}
/* TSSI */
void
wlc_lcn40phy_tssi_ucode_setup(phy_info_t *pi)
{
}

int
wlc_lcn40phy_tssi_cal(phy_info_t *pi)
{
	return 0;
}

static uint16
wlc_lcn40phy_get_tx_locc(phy_info_t *pi)
{	phytbl_info_t tab;
	uint16 didq;

	/* Update iqloCaltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL; /* iqloCaltbl		*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = 85;
	wlc_lcn40phy_read_table(pi, &tab);

	return didq;
}

static void
wlc_lcn40phy_set_tx_locc(phy_info_t *pi, uint16 didq)
{
	phytbl_info_t tab;

	/* Update iqloCaltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;			/* iqloCaltbl	*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = 85;
	wlc_lcn40phy_write_table(pi, &tab);
}

static void
wlc_lcn40phy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b)
{
	uint16 iqcc[2];
	phytbl_info_t tab;

	tab.tbl_ptr = iqcc; /* ptr to buf */
	tab.tbl_len = 2;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL; /* iqloCaltbl      */
	tab.tbl_offset = 80; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcn40phy_read_table(pi, &tab);

	*a = iqcc[0];
	*b = iqcc[1];
}
static void
wlc_lcn40phy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b)
{
	phytbl_info_t tab;
	uint16 iqcc[2];

	/* Fill buffer with coeffs */
	iqcc[0] = a;
	iqcc[1] = b;
	/* Update iqloCaltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;			/* iqloCaltbl	*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = iqcc;
	tab.tbl_len = 2;
	tab.tbl_offset = 80;
	wlc_lcn40phy_write_table(pi, &tab);
}


#define RADIO_REG_EI(pi) (RADIO_2065_TXGM_LOFT_FINE_I)
#define RADIO_REG_EQ(pi) (RADIO_2065_TXGM_LOFT_FINE_I)
#define RADIO_REG_FI(pi) (RADIO_2065_TXGM_LOFT_COARSE_I)
#define RADIO_REG_FQ(pi) (RADIO_2065_TXGM_LOFT_COARSE_Q)

static uint16
lcn40phy_iqlocc_write(phy_info_t *pi, uint8 data)
{
	int32 data32 = (int8)data;
	int32 rf_data32;
	int32 ip, in;
	ip = 8 + (data32 >> 1);
	in = 8 - ((data32+1) >> 1);
	rf_data32 = (in << 4) | ip;
	return (uint16)(rf_data32);
}

static void
wlc_lcn40phy_get_radio_loft(phy_info_t *pi,
	uint8 *ei0,
	uint8 *eq0,
	uint8 *fi0,
	uint8 *fq0)
{
	*ei0 = LCN40PHY_IQLOCC_READ(
		read_radio_reg(pi, RADIO_REG_EI(pi)));
	*eq0 = LCN40PHY_IQLOCC_READ(
		read_radio_reg(pi, RADIO_REG_EQ(pi)));
	*fi0 = LCN40PHY_IQLOCC_READ(
		read_radio_reg(pi, RADIO_REG_FI(pi)));
	*fq0 = LCN40PHY_IQLOCC_READ(
		read_radio_reg(pi, RADIO_REG_FQ(pi)));
}
static void
wlc_lcn40phy_set_radio_loft(phy_info_t *pi,
	uint8 ei0,
	uint8 eq0,
	uint8 fi0,
	uint8 fq0)
{
	write_radio_reg(pi, RADIO_REG_EI(pi), lcn40phy_iqlocc_write(pi, ei0));
	write_radio_reg(pi, RADIO_REG_EQ(pi), lcn40phy_iqlocc_write(pi, eq0));
	write_radio_reg(pi, RADIO_REG_FI(pi), lcn40phy_iqlocc_write(pi, fi0));
	write_radio_reg(pi, RADIO_REG_FQ(pi), lcn40phy_iqlocc_write(pi, fq0));
}

/* wlc_lcn40phy_tempsense_new: use mode 1 if tempsense registers
	might be used for other purposes such as background idle tssi measurement
*/
int16
wlc_lcn40phy_tempsense(phy_info_t *pi, bool mode)
{
	return 0;
}

/* return value is in volt Q4.4 */
int8
wlc_lcn40phy_vbatsense(phy_info_t *pi, bool mode)
{
	return 0;
}
void
wlc_lcn40phy_calib_modes(phy_info_t *pi, uint mode)
{
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif

	switch (mode) {
	case PHY_PERICAL_CHAN:
		/* right now, no channel based calibration */
		break;
	case PHY_FULLCAL:
	case PHY_PERICAL_WATCHDOG:
		wlc_lcn40phy_periodic_cal(pi);
		break;

	case PHY_PAPDCAL:
		wlc_lcn40phy_papd_recal(pi);
		break;
	default:
		ASSERT(0);
		break;
	}
}

static bool
wlc_lcn40phy_cal_reqd(phy_info_t *pi)
{
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	uint time_since_last_cal = (pi->sh->now >= ctx->cal_info.last_cal_time)?
		(pi->sh->now - ctx->cal_info.last_cal_time):
		(((uint)~0) - ctx->cal_info.last_cal_time + pi->sh->now);

	if (ctx->valid)
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint time_since_last_cal = (pi->sh->now >= pi->phy_lastcal)?
		(pi->sh->now - pi->phy_lastcal):
		(((uint)~0) - pi->phy_lastcal + pi->sh->now);

	if (pi_lcn->lcnphy_full_cal_channel == CHSPEC_CHANNEL(pi->radio_chanspec))
#endif
		return (time_since_last_cal >= pi->sh->glacial_timer);
	return TRUE;
}

static void
wlc_phy_watchdog_lcn40phy(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	if (pi->phy_forcecal || wlc_lcn40phy_cal_reqd(pi)) {
		if (!(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) ||
			ASSOC_INPROG_PHY(pi) || pi->carrier_suppr_disable ||
			(pi->measure_hold & PHY_HOLD_FOR_PKT_ENG) || pi->disable_percal))
			wlc_lcn40phy_calib_modes(pi, PHY_PERICAL_WATCHDOG);
	}

	if (!(SCAN_RM_IN_PROGRESS(pi) ||
		PLT_INPROG_PHY(pi)))
		wlc_lcn40phy_tx_pwr_update_npt(pi);

	if (!(SCAN_RM_IN_PROGRESS(pi))) {
		if ((pi_lcn->lcnphy_tssical_time) &&
			((pi->sh->now - pi_lcn->lcnphy_last_tssical)
			 >= pi_lcn->lcnphy_tssical_time)) {
			wlc_lcn40phy_tssi_ucode_setup(pi);
		}
	}
}

void
wlc_lcn40phy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr)
{
	int8 cck_offset;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	*cck_pwr = 0;
	*ofdm_pwr = 0;
	if (wlc_lcn40phy_tssi_based_pwr_ctrl_enabled(pi))	{
		if (phy_reg_read(pi, LCN40PHY_TxPwrCtrlStatus)
			& LCN40PHY_TxPwrCtrlStatus_estPwrValid_MASK)
			*ofdm_pwr =
				(int8)(PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatus, estPwr) >> 1);
		else if (phy_reg_read(pi, LCN40PHY_TxPwrCtrlStatusNew2)
			& LCN40PHY_TxPwrCtrlStatusNew2_estPwrValid1_MASK)
			*ofdm_pwr = (int8)(PHY_REG_READ(pi,
				LCN40PHY, TxPwrCtrlStatusNew2, estPwr1) >> 1);

		cck_offset = pi->tx_power_offset[TXP_FIRST_CCK];
		/* change to 6.2 */
		*cck_pwr = *ofdm_pwr + cck_offset;
	}
}


void
wlc_lcn40phy_tx_pu(phy_info_t *pi, bool bEnable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		/* remove all overrides */
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 0);

		phy_reg_and(pi, LCN40PHY_RFOverride0,
			~(uint16)(LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
			LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
			LCN40PHY_RFOverride0_ant_selp_ovr_MASK));

		phy_reg_and(pi, LCN40PHY_rfoverride4,
			~(uint16)(LCN40PHY_rfoverride4_papu_ovr_MASK));

	} else {
		/* Force on DAC */
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1);

		/* Force on the transmit chain */
		phy_reg_mod(pi, LCN40PHY_RFOverrideVal0,
			LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK |
			LCN40PHY_RFOverrideVal0_ant_selp_ovr_val_MASK,
			(1 << LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_SHIFT) |
			(0 << LCN40PHY_RFOverrideVal0_ant_selp_ovr_val_SHIFT));

		phy_reg_mod(pi, LCN40PHY_RFOverride0,
			LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK |
			LCN40PHY_RFOverride0_ant_selp_ovr_MASK,
			(1 << LCN40PHY_RFOverride0_internalrftxpu_ovr_SHIFT) |
			(1 << LCN40PHY_RFOverride0_ant_selp_ovr_SHIFT));

		/* Force the TR switch to transmit */
		wlc_lcn40phy_set_trsw_override(pi, TRUE, FALSE);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* Force on the Gband-ePA */
			phy_reg_mod(pi, LCN40PHY_RFOverrideVal0,
				LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK,
				(1 << LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_SHIFT) |
				(0 << LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT));

			phy_reg_mod(pi, LCN40PHY_RFOverride0,
				LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK,
				(1 << LCN40PHY_RFOverride0_gmode_tx_pu_ovr_SHIFT) |
				(1 << LCN40PHY_RFOverride0_amode_tx_pu_ovr_SHIFT));

			/* PA PU */
			PHY_REG_MOD(pi, LCN40PHY, rfoverride4val, papu_ovr_val, 1);
			PHY_REG_MOD(pi, LCN40PHY, rfoverride4, papu_ovr, 1);

		} else {
			/* Force off the Aband-ePA */
			phy_reg_mod(pi, LCN40PHY_RFOverrideVal0,
				LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK,
				(0 << LCN40PHY_RFOverrideVal0_gmode_tx_pu_ovr_val_SHIFT) |
				(1 << LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT));

			phy_reg_mod(pi, LCN40PHY_RFOverride0,
				LCN40PHY_RFOverride0_gmode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK,
				(1 << LCN40PHY_RFOverride0_gmode_tx_pu_ovr_SHIFT) |
				(1 << LCN40PHY_RFOverride0_amode_tx_pu_ovr_SHIFT));

			/* PA PU */
		}
	}
}

void
wlc_lcn40phy_write_table(phy_info_t *pi, const phytbl_info_t *pti)
{
	wlc_phy_write_table(pi, pti, LCN40PHY_TableAddress,
		LCN40PHY_TabledataHi, LCN40PHY_TabledataLo);
}

void
wlc_lcn40phy_read_table(phy_info_t *pi, phytbl_info_t *pti)
{
	wlc_phy_read_table(pi, pti, LCN40PHY_TableAddress,
	   LCN40PHY_TabledataHi, LCN40PHY_TabledataLo);
}
/* IOVAR */
bool
wlc_phy_tpc_iovar_isenabled_lcn40phy(phy_info_t *pi)
{
	return 	((phy_reg_read((pi), LCN40PHY_TxPwrCtrlCmd) &
		(LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
		LCN40PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
		LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK))
		== LCN40PHY_TX_PWR_CTRL_HW);
}

void
wlc_lcn40phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr)
{
	uint16 pwrctrl;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif

	pwrctrl = int_val ? LCN40PHY_TX_PWR_CTRL_HW : LCN40PHY_TX_PWR_CTRL_OFF;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phyreg_enter((wlc_phy_t *)pi);
	if (!int_val)
		wlc_lcn40phy_set_tx_pwr_by_index(pi, LCN40PHY_MAX_TX_POWER_INDEX);

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, pwrctrl);

#if defined(PHYCAL_CACHING)
	ctx->valid = FALSE;
#else
	pi_lcn->lcnphy_full_cal_channel = 0;
#endif
	pi->phy_forcecal = TRUE;
	wlc_phyreg_exit((wlc_phy_t *)pi);
	wlapi_enable_mac(pi->sh->physhim);
}

int
wlc_lcn40phy_idle_tssi_est_iovar(phy_info_t *pi, bool type)
{
	/* type = 0 will just do idle_tssi_est */
	/* type = 1 will do full tx_pwr_ctrl_init */

	if (!type)
		wlc_lcn40phy_idle_tssi_est(pi);
	else
		wlc_lcn40phy_tx_pwr_ctrl_init(pi);

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);

	return (phy_reg_read(pi, LCN40PHY_TxPwrCtrlIdleTssi));
}

uint8
wlc_lcn40phy_get_bbmult_from_index(phy_info_t *pi, int indx)
{	phytbl_info_t tab;
	uint8 bb_mult;
	uint32 bbmultiqcomp;

	ASSERT(indx <= LCN40PHY_MAX_TX_POWER_INDEX);

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmultiqcomp; /* ptr to buf */
	wlc_lcn40phy_read_table(pi,  &tab);

	/* Apply bb_mult */
	bb_mult = (uint8)((bbmultiqcomp >> 20) & 0xff);

	return bb_mult;
}

/* %%%%%% testing */
#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_long_train_lcn40phy(phy_info_t *pi, int channel)
{

	uint16 num_samps;
	phytbl_info_t tab;

	/* stop any test in progress */
	wlc_phy_test_stop(pi);

	/* channel 0 means restore original contents and end the test */
	if (channel == 0) {
		wlc_lcn40phy_stop_tx_tone(pi);
		wlc_lcn40phy_deaf_mode(pi, FALSE);
		return 0;
	}

	if (wlc_phy_test_init(pi, channel, TRUE)) {
		return 1;
	}

	wlc_lcn40phy_deaf_mode(pi, TRUE);

	num_samps = sizeof(ltrn_list)/sizeof(*ltrn_list);
	/* load sample table */
	tab.tbl_ptr = ltrn_list;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 16;
	wlc_lcn40phy_write_table(pi, &tab);

	wlc_lcn40phy_run_samples(pi, num_samps, 0xffff, 0, 0);

	return 0;
}

void
wlc_phy_init_test_lcn40phy(phy_info_t *pi)
{
	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);
	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	/* Recalibrate for this channel */
	wlc_lcn40phy_calib_modes(pi, PHY_FULLCAL);
}
#endif 

#if defined(WLTEST)
void
wlc_phy_carrier_suppress_lcn40phy(phy_info_t *pi)
{

	wlc_lcn40phy_reset_radio_loft(pi);

	wlc_lcn40phy_clear_tx_power_offsets(pi);
}
static void
wlc_lcn40phy_reset_radio_loft(phy_info_t *pi)
{
	write_radio_reg(pi, RADIO_REG_EI(pi), 0x88);
	write_radio_reg(pi, RADIO_REG_EQ(pi), 0x88);
	write_radio_reg(pi, RADIO_REG_FI(pi), 0x88);
	write_radio_reg(pi, RADIO_REG_FQ(pi), 0x88);
}

#endif 

void
wlc_lcn40phy_deaf_mode(phy_info_t *pi, bool mode)
{
	phy_reg_write(pi, LCN40PHY_crsgainCtrl_new, mode ? 0 : 0xFF);
	if (mode)
		PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !mode);
	phy_reg_or(pi, LCN40PHY_agcControl4, 1);
	if (mode)
		PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !mode);
	PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, crseddisable, mode);

#ifdef FIXME
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, ext_lna_gain_ovr, mode);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, ext_lna_gain_ovr_val, 0);
	phy_reg_write(pi, LCN40PHY_crsgainCtrl_new, mode ? 0 : 0xFF);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, !mode);

	phy_reg_mod((pi), LCN40PHY_crsgainCtrl,
		LCN40PHY_crsgainCtrl_DSSSDetectionEnable_MASK |
		LCN40PHY_crsgainCtrl_OFDMDetectionEnable_MASK,
		((CHSPEC_IS2G(pi->radio_chanspec)) ? (!mode) : 0) <<
		LCN40PHY_crsgainCtrl_DSSSDetectionEnable_SHIFT |
		(!mode) << LCN40PHY_crsgainCtrl_OFDMDetectionEnable_SHIFT);
	PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, crseddisable, mode);
#endif
}

static void
wlc_lcn40phy_tx_tone_samples(phy_info_t *pi, int32 f_Hz, uint16 max_val, uint32 *data_buf,
	uint32 phy_bw, uint16 num_samps)
{
	fixed theta = 0, rot = 0;
	uint16 i_samp, q_samp, t;
	cint32 tone_samp;
	/* set up params to generate tone */
	rot = FIXED((f_Hz * 36)/(phy_bw * 1000)) / 100; /* 2*pi*f/bw/1000  Note: f in KHz */
	theta = 0;			/* start angle 0 */

	/* tone freq = f_c MHz ; phy_bw = phy_bw MHz ; # samples = phy_bw (1us) ; max_val = 151 */
	/* TCL: set tone_buff [mimophy_gen_tone $f_c $phy_bw $phy_bw $max_val] */
	for (t = 0; t < num_samps; t++) {
		/* compute phasor */
		wlc_phy_cordic(theta, &tone_samp);
		/* update rotation angle */
		theta += rot;
		/* produce sample values for play buffer */
		i_samp = (uint16)(FLOAT(tone_samp.i * max_val) & 0x3ff);
		q_samp = (uint16)(FLOAT(tone_samp.q * max_val) & 0x3ff);
		data_buf[t] = (i_samp << 10) | q_samp;
	}

}

static uint16
wlc_lcn40phy_num_samples(phy_info_t *pi, int32 f_Hz, uint32 phy_bw)
{
	uint16 num_samps, k;
	uint32 bw;

	/* allocate buffer */
	if (f_Hz) {
		k = 1;
		do {
			bw = phy_bw * 1000 * k * 1000;
			num_samps = bw / ABS(f_Hz);
			ASSERT(num_samps <= 256);
			k++;
		} while ((num_samps * (uint32)(ABS(f_Hz))) !=  bw);
	} else
		num_samps = 2;

	return num_samps;
}
/*
* Given a test tone frequency, continuously play the samples. Ensure that num_periods
* specifies the number of periods of the underlying analog signal over which the
* digital samples are periodic
*/
/* equivalent to lcn40phy_play_tone */
void
wlc_lcn40phy_start_tx_tone(phy_info_t *pi, int32 f_Hz, uint16 max_val, bool iqcalmode)
{
	uint8 phy_bw;
	uint16 num_samps;
	uint32 *data_buf;

	phytbl_info_t tab;

	if ((data_buf = MALLOC(pi->sh->osh, sizeof(uint32) * 256)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Save active tone frequency */
	pi->phy_tx_tone_freq = f_Hz;

	PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 1);

	/* Turn off all the crs signals to the MAC */
	wlc_lcn40phy_deaf_mode(pi, TRUE);

	if (CHSPEC_IS20(pi->radio_chanspec))
		phy_bw = 40;
	else
		phy_bw = 80;

	num_samps = wlc_lcn40phy_num_samples(pi, f_Hz, phy_bw);

	PHY_INFORM(("wl%d: %s: %d Hz, %d samples\n",
		pi->sh->unit, __FUNCTION__,
		f_Hz, num_samps));

	if (num_samps > 256) {
		PHY_ERROR(("wl%d: %s: Too many samples to fit in SPB\n",
			pi->sh->unit, __FUNCTION__));
		MFREE(pi->sh->osh, data_buf, 256 * sizeof(uint32));
		return;
	}

	/* in LC40NPHY, we need to bring SPB out of standby before using it */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCtrl3, sram_stby, 0);

	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 1);

	wlc_lcn40phy_tx_tone_samples(pi, f_Hz, max_val, data_buf, phy_bw, num_samps);

	/* lcn40phy_load_sample_table */
	tab.tbl_ptr = data_buf;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcn40phy_write_table(pi, &tab);
	/* play samples from the sample play buffer */
	wlc_lcn40phy_run_samples(pi, num_samps, 0xffff, 0, iqcalmode);
	MFREE(pi->sh->osh, data_buf, 256 * sizeof(uint32));
}

void
wlc_lcn40phy_stop_tx_tone(phy_info_t *pi)
{
	int16 playback_status;

	pi->phy_tx_tone_freq = 0;

	/* Stop sample buffer playback */
	playback_status = phy_reg_read(pi, LCN40PHY_sampleStatus);
	if (playback_status & LCN40PHY_sampleStatus_NormalPlay_MASK) {
		wlc_lcn40phy_tx_pu(pi, 0);
		PHY_REG_MOD(pi, LCN40PHY, sampleCmd, stop, 1);
	} else if (playback_status & LCN40PHY_sampleStatus_iqlocalPlay_MASK)
		PHY_REG_MOD(pi, LCN40PHY, iqloCalCmdGctl, iqlo_cal_en, 0);

	/* put back SPB into standby */
	phy_reg_write(pi, LCN40PHY_sslpnCtrl3, 1);
	/* disable clokc to spb */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 0);
	/* disable clock to txFrontEnd */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 0);

	/* Restore all the crs signals to the MAC */
	wlc_lcn40phy_deaf_mode(pi, FALSE);
	PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 0);
}

static void
wlc_lcn40phy_play_sample_table1(phy_info_t *pi, int32 f_Hz, uint16 max_val)
{
	uint8 phy_bw;
	uint16 num_samps;
	uint32 *data_buf;

	phytbl_info_t tab;

	if ((data_buf = MALLOC(pi->sh->osh, sizeof(uint32) * 256)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Save active tone frequency */
	pi->phy_tx_tone_freq = f_Hz;

	if (CHSPEC_IS20(pi->radio_chanspec))
		phy_bw = 40;
	else
		phy_bw = 80;

	num_samps = wlc_lcn40phy_num_samples(pi, f_Hz, phy_bw);

	PHY_REG_MOD(pi, LCN40PHY, sampleCmd, buf2foriqlocal, 1);
	PHY_REG_MOD(pi, LCN40PHY, sampleDepthCount, DepthCount1, num_samps - 1);

	PHY_INFORM(("wl%d: %s: %d Hz, %d samples\n",
		pi->sh->unit, __FUNCTION__,
		f_Hz, num_samps));

	if (num_samps > 256) {
		PHY_ERROR(("wl%d: %s: Too many samples to fit in SPB\n",
			pi->sh->unit, __FUNCTION__));
		MFREE(pi->sh->osh, data_buf, 256 * sizeof(uint32));
		return;
	}

	wlc_lcn40phy_tx_tone_samples(pi, f_Hz, max_val, data_buf, phy_bw, num_samps);

	/* lcn40phy_load_sample_table */
	tab.tbl_ptr = data_buf;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY1;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcn40phy_write_table(pi, &tab);
	MFREE(pi->sh->osh, data_buf, 256 * sizeof(uint32));
}

void
wlc_lcn40phy_set_tx_tone_and_gain_idx(phy_info_t *pi)
{
	int8 curr_pwr_idx_val;

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	if (LCN40PHY_TX_PWR_CTRL_OFF != wlc_lcn40phy_get_tx_pwr_ctrl(pi)) {
		curr_pwr_idx_val = wlc_lcn40phy_get_current_tx_pwr_idx(pi);
		wlc_lcn40phy_set_tx_pwr_by_index(pi, (int)curr_pwr_idx_val);
}

	phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xffff);
	wlc_lcn40phy_start_tx_tone(pi, pi->phy_tx_tone_freq, 120, 0); /* play tone */
}
void
wlc_lcn40phy_crsuprs(phy_info_t *pi, int channel)
{
	uint16 afectrlovr, afectrlovrval;
	afectrlovr = phy_reg_read(pi, LCN40PHY_AfeCtrlOvr1);
	afectrlovrval = phy_reg_read(pi, LCN40PHY_AfeCtrlOvr1Val);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (channel != 0) {
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1);
		PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
		phy_reg_write(pi, LCN40PHY_ClkEnCtrl, 0xffff);
		wlc_lcn40phy_tx_pu(pi, 1);
		/* Turn ON bphyframe */
		PHY_REG_MOD(pi, LCN40PHY, BphyControl3, bphyFrmStartCntValue, 0);
		/* Turn on Tx Front End clks */
		phy_reg_or(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x0080);
		/* start the rfcs signal */
		phy_reg_or(pi, LCN40PHY_bphyTest, 0x228);
	} else {
		phy_reg_and(pi, LCN40PHY_bphyTest, ~(0x228));
		/* disable clk to txFrontEnd */
		phy_reg_and(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xFF7F);
		phy_reg_write(pi, LCN40PHY_AfeCtrlOvr1, afectrlovr);
		phy_reg_write(pi, LCN40PHY_AfeCtrlOvr1Val, afectrlovrval);
		}
}
/* NOISE */
void
wlc_phy_noise_measure_lcn40phy(phy_info_t *pi)
{
}

void
wlc_lcn40phy_noise_measure_start(phy_info_t *pi, bool adj_en)
{
}

void
wlc_lcn40phy_noise_measure_stop(phy_info_t *pi)
{
}

void
wlc_lcn40phy_noise_measure_disable(phy_info_t *pi, uint32 flag, uint32* p_flag)
{
}

void
wlc_lcn40phy_dummytx(wlc_phy_t *ppi, uint16 nframes, uint16 wait_delay)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	uint8 counter = 0;
	uint16 max_pwr_idx = 0;
	uint16 min_pwr_idx = 127;
	uint16 current_txidx = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	wlc_btcx_override_enable(pi);
	wlc_lcn40phy_deaf_mode(pi, TRUE);

	for (counter = 0; counter < nframes; counter ++) {
		wlc_phy_do_dummy_tx(pi, TRUE, OFF);
		OSL_DELAY(wait_delay);
		current_txidx = wlc_lcnphy_get_current_tx_pwr_idx(pi);
		if (current_txidx > max_pwr_idx)
			max_pwr_idx = current_txidx;
		if (current_txidx < min_pwr_idx)
			min_pwr_idx = current_txidx;
	}

	wlc_lcn40phy_deaf_mode(pi, FALSE);

	pi_lcn->lcnphy_start_idx = (uint8)current_txidx; 	/* debug information */
}

void
wlc_lcn40phy_papd_recal(phy_info_t *pi)
{

	uint16 tx_pwr_ctrl;
	bool suspend;
	uint16 current_txidx = 0;
	phy_txcalgains_t txgains;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint16 nframes;
	uint16 wait_delay;
	uint8 npt;

	nframes = 50;
	wait_delay = 10*100;
#ifndef PHYCAL_CACHING
	pi_lcn->lcnphy_full_cal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#endif
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* Set non-zero duration for CTS-to-self */
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Save npt */
	npt = wlc_lcn40phy_get_tx_pwr_npt(pi);

	/* Save tx power control mode */
	tx_pwr_ctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);

	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	/* Enable pwr ctrl */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);

	/* clear all offsets */
	wlc_lcn40phy_clear_tx_power_offsets(pi);

	/* set target pwr for papd */
	wlc_lcn40phy_set_target_tx_pwr(pi, 64);

	/* Setting npt to 0 for index settling with 30 frames */
	wlc_lcn40phy_set_tx_pwr_npt(pi, 0);

	PHY_TRACE(("dummy TX Start Called\n"));

	/* transmit dummy tx pkts and find the txidx */
	wlc_lcn40phy_dummytx((wlc_phy_t *)pi, nframes, wait_delay);


	current_txidx = pi_lcn->lcnphy_start_idx; 	/* debug information */

	wlc_lcn40phy_deaf_mode(pi, TRUE);

	wlc_btcx_override_enable(pi);

	/* Restore npt */
	wlc_lcn40phy_set_tx_pwr_npt(pi, npt);

	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	if (pi_lcn->lcnphy_CalcPapdCapEnable == 1)
	{
		wlc_lcn40phy_papd_calc_capindex(pi, &txgains);
		wlc_lcn40phy_load_txgainwithcappedindex(pi, 1);
		/* set target pwr for papd = 15.5 dbm */
		wlc_lcn40phy_set_target_tx_pwr(pi, 62);

		/* Setting npt to 0 for index settling with 30 frames */
		wlc_lcn40phy_set_tx_pwr_npt(pi, 0);

		PHY_TRACE(("dummy TX Start Called\n"));

		/* transmit dummy tx pkts and find the txidx */
		wlc_lcn40phy_dummytx((wlc_phy_t *)pi, nframes, wait_delay);


		current_txidx = pi_lcn->lcnphy_start_idx; 	/* debug information */
		if (current_txidx == 0)
		{
			pi_lcn->lcnphy_capped_index += 4;
			wlc_lcn40phy_load_txgainwithcappedindex(pi, 1);

		}
	}
	else
	{
		txgains.index = (uint8) current_txidx;
		txgains.useindex = 1;
		/* run papd corresponding to the target pwr */
		wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0, 219, 1);
	}

	/* Restore tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl);

	/* Reset radio ctrl and crs gain */
	phy_reg_or(pi, LCN40PHY_resetCtrl, 0x44);
	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x80);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	wlc_lcn40phy_deaf_mode(pi, FALSE);
}

static void
wlc_lcn40phy_btc_adjust(phy_info_t *pi, bool btactive)
{
	int btc_mode = wlapi_bmac_btc_mode_get(pi->sh->physhim);

	bool suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	bool override;
	uint qdbm;
	/* for dual antenna design, a gain table switch is to ensure good performance 
	 * in simultaneous WLAN RX 
	 */
	if (pi->aa2g > 2) {
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
			if (btactive && !pi->bt_active) {
				wlc_lcn40phy_write_table(pi, dot11lcn40_gain_tbl_2G_info_rev3_BT);
			} else if (!btactive && pi->bt_active) {
				wlc_lcn40phy_write_table(pi, dot11lcn40_gain_tbl_2G_info_rev3);
			}
		}

		if ((btc_mode == WL_BTC_PARALLEL) || (btc_mode == WL_BTC_LITE)) {
			if (btactive) {
				wlc_phy_txpower_get((wlc_phy_t *)pi, &qdbm, &override);
				pi->u.pi_lcn40phy->saved_user_qdbm = (uint16)qdbm;
				wlc_phy_txpower_set((wlc_phy_t *)pi, 40, FALSE);
			} else if (pi->u.pi_lcn40phy->saved_user_qdbm) {
				wlc_phy_txpower_set((wlc_phy_t *)pi,
					pi->u.pi_lcn40phy->saved_user_qdbm, FALSE);
			}
		}
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	}

}

void wlc_lcn40phy_read_papdepstbl(phy_info_t *pi, struct bcmstrbuf *b)
{

	phytbl_info_t tab;
	uint32 val, j;
	int32 eps_real, eps_imag;
	/* Save epsilon table */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_ptr = &val; /* ptr to buf */
	tab.tbl_width = 32;
	tab.tbl_len = 1;        /* # values   */

	bcm_bprintf(b, "\n");
	for (j = 0; j < PHY_PAPD_EPS_TBL_SIZE_LCNPHY; j++) {
		tab.tbl_offset = j;
		wlc_lcnphy_read_table(pi, &tab);
		eps_real = (val & 0x00fff000) << 8;
	        eps_imag = (val & 0x00000fff) << 20;
		eps_real = eps_real >> 20;
		eps_imag = eps_imag >> 20;
		bcm_bprintf(b, "%d \t %d \n", eps_real, eps_imag);
	}
	bcm_bprintf(b, "\n");
}

static void
WLBANDINITFN(wlc_lcn40phy_reg_init)(phy_info_t *pi)
{
	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) || LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 4))
		wlc_lcn40phy_rev0_reg_init(pi);
	else if (LCN40REV_IS(pi->pubpi.phy_rev, 1) || LCN40REV_IS(pi->pubpi.phy_rev, 3))
		wlc_lcn40phy_rev1_reg_init(pi);
	wlc_lcn40phy_bu_tweaks(pi);
}

static void
WLBANDINITFN(wlc_lcn40phy_baseband_init)(phy_info_t *pi)
{
	PHY_TRACE(("%s:***CHECK***\n", __FUNCTION__));
	/* Initialize LCN40PHY tables */
	wlc_lcn40phy_tbl_init(pi);
	wlc_lcn40phy_reg_init(pi);
	wlc_lcn40phy_set_tx_pwr_by_index(pi, 40);
	wlc_lcn40phy_noise_init(pi);
}

/* mapped onto lcn40phy_rev0_rf_init proc */
static void
WLBANDINITFN(wlc_radio_2065_init)(phy_info_t *pi)
{
	uint32 i;
	lcn40phy_radio_regs_t *lcn40phyregs = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* init radio regs */
	if (RADIOID(pi->pubpi.radioid) == BCM2067_ID)
		lcn40phyregs = lcn40phy_radio_regs_2067;
	else if (RADIOID(pi->pubpi.radioid) == BCM2065_ID)
		lcn40phyregs = lcn40phy_radio_regs_2065;

	ASSERT(lcn40phyregs != NULL);

	for (i = 0; lcn40phyregs[i].address != 0xffff; i++) {
		if (CHSPEC_IS5G(pi->radio_chanspec) &&
			lcn40phyregs[i].do_init_g)
			write_radio_reg(pi,
				((lcn40phyregs[i].address & 0x3fff) |
				RADIO_DEFAULT_CORE),
				(uint16)lcn40phyregs[i].init_g);
		else if (lcn40phyregs[i].do_init_g)
			write_radio_reg(pi,
				((lcn40phyregs[i].address & 0x3fff) |
				RADIO_DEFAULT_CORE),
				(uint16)lcn40phyregs[i].init_g);
	}
	/* tweaks for 4314/43142 only */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 1) || LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		/* Max out capless LDO output */
		mod_radio_reg(pi, RADIO_2065_TIA_CFG2, 0x7, 0x4);
		mod_radio_reg(pi, RADIO_2065_PMU_CFG2, 0x3333, 0x1103);
	}
	/* setting tx lo coefficients to 0 */
	wlc_lcn40phy_set_tx_locc(pi, 0);
}

static void
WLBANDINITFN(wlc_lcn40phy_radio_init)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (NORADIO_ENAB(pi->pubpi))
		return;
	/* Initialize 2065 radio */

	wlc_radio_2065_init(pi);
}

static void
wlc_lcn40phy_radio_reset(phy_info_t *pi)
{
	if (NORADIO_ENAB(pi->pubpi))
		return;
	/* reset the radio, bit 8. do not reset for rev0 due to HSIC issue */
	if (!(LCN40REV_IS(pi->pubpi.phy_rev, 0)))
	{
		phy_reg_or(pi, LCN40PHY_resetCtrl, 0x100);
		phy_reg_and(pi, LCN40PHY_resetCtrl, ~0x100);
	}
}

static void
wlc_lcn40phy_rcal(phy_info_t *pi)
{

	if (NORADIO_ENAB(pi->pubpi))
		return;
	mod_radio_reg(pi, RADIO_2065_OVR8, 0x8000, 0x0);
	or_radio_reg(pi, RADIO_2065_OVR2, 1 << 6);
	mod_radio_reg(pi, RADIO_2065_BG_CFG1, 0xf0, 0xa << 4);

#ifdef BRINGUPDONE
	mod_radio_reg(pi, RADIO_2065_BG_CFG1, 0xf0, 0xa << 4);
	mod_radio_reg(pi, RADIO_2065_OVR8, 0x8000, 0x0);
	or_radio_reg(pi, RADIO_2065_OVR2, 1 << 6);

	/* power down set reg(RF_rcal_cfg.pu) 0 */
	mod_radio_reg(pi, RADIO_2065_RCAL_CFG, 0x1, 0);
	/* Enable Pwrsw (Needed ?)
	   set reg(RF_pmu_op.misc_pwrsw_en) 1
	   set reg(RF_OVR7.ovr_misc_pwrsw_en) 1
	*/
	or_radio_reg(pi, RADIO_2065_PMU_OP, 0x40);
	or_radio_reg(pi, RADIO_2065_OVR7, 1);
	/* Enable RCAL Clock
	   set reg(RF_xtal_cfg1.pll_clock_rcal_clk_pu) 1
	*/
	or_radio_reg(pi, RADIO_2065_XTAL_CFG1, 0x8000);
	/* Power up RCAL
	   set reg(RF_rcal_cfg.pu) 1
	*/
	or_radio_reg(pi, RADIO_2065_RCAL_CFG, 1);
	OSL_DELAY(5000);
	SPINWAIT(!wlc_radio_2065_rcal_done(pi), 10 * 1000 * 1000);
	/* wait for RCAL valid bit to be set */
	if (wlc_radio_2065_rcal_done(pi)) {
		PHY_INFORM(("wl%d: %s:  Rx RCAL completed, code: %x\n",
			pi->sh->unit, __FUNCTION__,
			read_radio_reg(pi, RADIO_2065_RCAL_CFG) & 0x1f));
	} else {
		PHY_ERROR(("wl%d: %s: RCAL failed\n", pi->sh->unit, __FUNCTION__));
	}
	/* power down RCAL
	   set reg(RF_rcal_cfg.pu) 0
	*/
	mod_radio_reg(pi, RADIO_2065_RCAL_CFG, 0x1, 0);
	/* Disable RCAL Clock
	   set reg(RF_xtal_cfg1.pll_clock_rcal_clk_pu) 0
	*/
	mod_radio_reg(pi, RADIO_2065_XTAL_CFG1, 0x8000, 0);
#endif /* BRINGUPDONE */
}

static void
wlc_lcn40phy_rc_cal(phy_info_t *pi)
{
	uint16 old_xtal_clk, old_rccal_big, old_rccal_small, old_rccal_adc;
	uint16 old_rccal_hpc, hpc_code_big, hpc_code_small, hpc_code_adc, hpc_offset;
	uint16 b_cap_rc_code_raw, s_cap_rc_code_raw, a_cap_rc_code_raw;
	uint16 flt_val, trc, c1, c2, c3, c4;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (pi->xtalfreq / 1000 == 37400) {
		trc = 0x164;
		hpc_code_big = hpc_code_small = 0x8;
		hpc_code_adc = 2;
		hpc_offset = 0;
	} else if (pi->xtalfreq / 1000 == 20000) {
		trc = 0xc6;
		hpc_code_big = hpc_code_small = 0x8;
		hpc_code_adc = 2;
		hpc_offset = 0;
	} else {
		PHY_INFORM(("wl%d: %s:  RCCAL not run, xtal "
					"%d not known\n",
					pi->sh->unit, __FUNCTION__, pi->xtalfreq));
		return;
	}

	/*    #Save old value of rccal_clk_pu
		  set Old_xtal_cfg1_pll_clock_rccal_clk_pu
		  $reg(RF_xtal_cfg1.pll_clock_rccal_clk_pu)
	*/
	old_xtal_clk = read_radio_reg(pi, RADIO_2065_XTAL_CFG1) & 0x4000;
	/*     #Save old HPC value incase RCCal fails
		   set Old_RCCAL_HPC $reg(RF_tia_cfg3.rccal_hpc)
	*/
	old_rccal_hpc = read_radio_reg(pi, RADIO_2065_TIA_CFG3) & 0x1F;
	/* 	set reg(RF_rccal_cfg.rccal_mode) 1 */
	or_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x2);
	/* 	### Big cap
		#Save old Big Cap value incase RCCal fails
		set Old_RCCAL_big $reg(RF_rccal_logic5.rccal_raw_big)
	*/
	old_rccal_big = read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC5) & 0x1F00;
	/* 	#  power down rccal
		set reg(RF_rccal_cfg.pu) 0
	*/
	mod_radio_reg(pi, RADIO_2065_RCCAL_LOGIC1, 1, 0);
	mod_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x1, 0);
	or_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x1);
	/*     #Enable Pwrsw (Needed ?)
		   set reg(RF_pmu_op.misc_pwrsw_en) 1
		   set reg(RF_OVR7.ovr_misc_pwrsw_en) 1
	*/
	or_radio_reg(pi, RADIO_2065_PMU_OP, 0x40);
	or_radio_reg(pi, RADIO_2065_OVR7, 0x1);
	/* 	#Enable RCCal clock
		set reg(RF_xtal_cfg1.pll_clock_rccal_clk_pu) 1
	*/
	or_radio_reg(pi, RADIO_2065_XTAL_CFG1, 0x4000);

	/*     #Set Trc
		   set reg(RF_rccal_trc.rccal_Trc) $Trc
		   #RCCAL on Big Cap first
		   set reg(RF_rccal_cfg.sc) 1
	*/
	mod_radio_reg(pi, RADIO_2065_RCCAL_TRC, 0x1FFF, trc);
	mod_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x18, 0);
	/*     #setup to run RX RC Cal and setup R1/Q1/P1
		   set reg(RF_rccal_logic1.rccal_P1) 0x1

		   set reg(RF_rccal_logic1.rccal_Q1) 0x1

		   set reg(RF_rccal_logic1.rccal_R1) 0x1

		   #Set X1
		   set reg(RF_rccal_logic1.rccal_X1) 0x63
		   #Start RCCAL
		   set reg(RF_rccal_logic1.rccal_START) 1
	*/
	mod_radio_reg(pi, RADIO_2065_RCCAL_LOGIC1, 0xFFFD, 0x6355);

	/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2065_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2065_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: Big Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		b_cap_rc_code_raw = old_rccal_big;
		hpc_code_big = old_rccal_hpc;
	} else {
		/* RCCAL successful */
		b_cap_rc_code_raw = (read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC5) & 0x1F00) >> 8;
		mod_radio_reg(pi, RADIO_2065_TIA_CFG3, 0x1f, b_cap_rc_code_raw - 4);
		if (b_cap_rc_code_raw < 0x1E)
			hpc_code_big = b_cap_rc_code_raw + hpc_offset;
		PHY_INFORM(("wl%d: %s:  Big Rx RC Cal completed for "
			"Trc: %x, N0: %x, N1: %x, b_cap_code_raw: %x, hpc_code: %x\n",
			pi->sh->unit, __FUNCTION__, trc,
			read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC4) & 0x1FFF,
			b_cap_rc_code_raw, hpc_code_big));
	}
	/* RCCAL on small cap */
	/* Save old small cap value in case RCCal fails */
	old_rccal_small = read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC5) & 0x1F;
	/* Stop RCCAL */
	mod_radio_reg(pi, RADIO_2065_RCCAL_LOGIC1, 0x1, 0);
	/* Power down RC CAL */
	mod_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x1, 0);
	/* Power up RC Cal */
	/* 	#RCCAL on Small Cap */
	mod_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x19, 1 | (1 << 3));
	/* #Set Trc
	  set reg(RF_rccal_trc.rccal_Trc) $Trc
	*/
	mod_radio_reg(pi, RADIO_2065_RCCAL_TRC, 0x1FFF, trc);

	/* set reg(RF_rccal_logic1.rccal_X1) 0x6F
	set reg(RF_rccal_logic1.rccal_START) 1
	*/
	mod_radio_reg(pi, RADIO_2065_RCCAL_LOGIC1, 0xFF01, 0x6301);
	/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2065_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2065_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: Small Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		s_cap_rc_code_raw = old_rccal_small;
	} else {
		/* RCCAL successful */
		s_cap_rc_code_raw = read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC5) & 0x1F;
		if (s_cap_rc_code_raw < 0x1E)
			hpc_code_small = s_cap_rc_code_raw + hpc_offset;

		PHY_INFORM(("wl%d: %s:  Small Rx RC Cal completed for cap: "
			"N0: %x, N1: %x, s_cap_rc_code_raw: %x, hpc: %x\n",
			pi->sh->unit, __FUNCTION__,
			read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC4) & 0x1FFF,
			s_cap_rc_code_raw, hpc_code_small));
	}

	/*     #ADC Cap RC CAL
		   #Save old ADC Cap value incase RCCal fails
		   set Old_RCCAL_adc $reg(RF_rccal_logic2.rccal_adc_code)
	*/
	old_rccal_adc = read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC2) & 0xF;
	/* Stop RCCAL */
	mod_radio_reg(pi, RADIO_2065_RCCAL_LOGIC1, 0x1, 0);
	/* Power down RC CAL */
	mod_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x1, 0);
	/* Power up RC Cal */
	/* 	#RCCAL on Small Cap */
	mod_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x19, 1 | (2 << 3));
	/*    #Set Trc
		  set reg(RF_rccal_trc.rccal_Trc) $Trc
	*/
	mod_radio_reg(pi, RADIO_2065_RCCAL_TRC, 0x1FFF, trc);

	/* set reg(RF_rccal_logic1.rccal_X1) 0x6F
	   set reg(RF_rccal_logic1.rccal_START) 1
	*/
	mod_radio_reg(pi, RADIO_2065_RCCAL_LOGIC1, 0xFF01, 0x3001);
	/* check to see if RC CAL is done */
	OSL_DELAY(50);
	SPINWAIT(!wlc_radio_2065_rc_cal_done(pi), 10 * 1000 * 1000);
	if (!wlc_radio_2065_rc_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: ADC Cap RC Cal failed\n", pi->sh->unit, __FUNCTION__));
		a_cap_rc_code_raw = old_rccal_adc;
	} else {
		/* RCCAL successful */
		a_cap_rc_code_raw = read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC2) & 0xF;
		if (a_cap_rc_code_raw < 0x1E)
			hpc_code_adc = a_cap_rc_code_raw + hpc_offset;
		PHY_INFORM(("wl%d: %s:  ADC Rx RC Cal completed for cap: "
			"N0: %x, N1: %x, a_cap_rc_code_raw: %x, hpc_code: %x\n",
			pi->sh->unit, __FUNCTION__,
			read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			read_radio_reg(pi, RADIO_2065_RCCAL_LOGIC3) & 0x1FFF,
			a_cap_rc_code_raw, hpc_code_adc));
	}
	c2 = s_cap_rc_code_raw + 19;
	c4 = c2 * 2 + 14;
	c3 = (c2 + c4) / 2 - 3;
	c1 = c3 /2 - 5;
	c1 -= 3;
	c2 -= 4;
	c3 -= 6;
	c4 -= 9;

	/* Stop RCCAL */
	mod_radio_reg(pi, RADIO_2065_RCCAL_LOGIC1, 0x1, 0);
	/* Power down RC CAL */
	mod_radio_reg(pi, RADIO_2065_RCCAL_CFG, 0x1, 0);
	/* 	#disable RCCal clock
		set reg(RF_xtal_cfg1.pll_clock_rccal_clk_pu) 1
	*/
	mod_radio_reg(pi, RADIO_2065_XTAL_CFG1, 0x4000, 0);

	flt_val =
		(b_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (b_cap_rc_code_raw);
	phy_reg_write(pi, LCN40PHY_lpf_rccal_tbl_1, flt_val);
	phy_reg_write(pi, LCN40PHY_lpf_rccal_tbl_2, flt_val);

	flt_val =
		(s_cap_rc_code_raw << 10) | (b_cap_rc_code_raw << 5) | (b_cap_rc_code_raw);
	phy_reg_write(pi, LCN40PHY_lpf_rccal_tbl_3, flt_val);

	flt_val =
		(s_cap_rc_code_raw << 10) | (s_cap_rc_code_raw << 5) | (s_cap_rc_code_raw);
	phy_reg_write(pi, LCN40PHY_lpf_rccal_tbl_4, flt_val);

	flt_val = (hpc_code_big << 5) | (hpc_code_big);
	phy_reg_write(pi, LCN40PHY_lpf_rccal_tbl_5, (flt_val & 0x1FF));

	flt_val = (c2 << 7) | c1;
	phy_reg_write(pi, LCN40PHY_lpf_rccal_tbl_6, (flt_val & 0x3FFF));
	flt_val = (c4 << 7) | c3;
	phy_reg_write(pi, LCN40PHY_lpf_rccal_tbl_7, (flt_val & 0x3FFF));
}

static void
wlc_lcn40phy_minipmu_cal(phy_info_t *pi)
{
	/* set reg(RF_pmu_cfg3.selavg) 2 */
	mod_radio_reg(pi, RADIO_2065_PMU_CFG3, 0xc0, 2 << 6);
	/* set reg(RF_pmu_op.vref_select) 1 */
	or_radio_reg(pi, RADIO_2065_PMU_OP, 0x8000);
	/* set reg(RF_pmu_cfg2.wlpmu_cntl) 0 */
	mod_radio_reg(pi, RADIO_2065_PMU_CFG2, 0x8, 0);
	/* set reg(RF_pmu_op.wlpmu_ldobg_clk_en) 0 */
	and_radio_reg(pi, RADIO_2065_PMU_OP, ~0x2000);
	/* set reg(RF_pmu_op.ldoref_start_cal) 1 */
	or_radio_reg(pi, RADIO_2065_PMU_OP, 0x8000);
	/* set reg(RF_pmu_op.ldoref_start_cal) 0 */
	and_radio_reg(pi, RADIO_2065_PMU_OP, (uint16)~0x8000);
	/* set reg(RF_pmu_op.wlpmu_ldobg_clk_en) 1 */
	or_radio_reg(pi, RADIO_2065_PMU_OP, 0x2000);
	/* set reg(RF_pmu_op.ldoref_start_cal) 1 */
	or_radio_reg(pi, RADIO_2065_PMU_OP, 0x8000);

	SPINWAIT(!wlc_radio_2065_minipmu_cal_done(pi), 100 * 1000);

	if (!wlc_radio_2065_minipmu_cal_done(pi)) {
		PHY_ERROR(("wl%d: %s: minipmu cal failed\n", pi->sh->unit, __FUNCTION__));
	}
	/* set reg(RF_pmu_op.wlpmu_ldobg_clk_en) 0 */
	and_radio_reg(pi, RADIO_2065_PMU_OP, ~0x2000);
	/* set reg(RF_pmu_op.ldoref_start_cal) 0 */
	and_radio_reg(pi, RADIO_2065_PMU_OP, (uint16)~0x8000);
}

static void wlc_lcn40phy_load_tx_gain_table(phy_info_t *pi,
	const phy_tx_gain_tbl_entry * gain_table)
{
	uint32 j;
	phytbl_info_t tab;
	uint32 val;
	uint16 pa_gain;
	uint16 gm_gain;
	int16 dac;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	if (CHSPEC_IS5G(pi->radio_chanspec))
		pa_gain = 0x70;
	else {		/* 2g */
		if (!pi_lcn->ePA)
			pa_gain = 0xff;
		else
			pa_gain = 0x30;
	}

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */

	for (j = 0; j < 128; j++) {

		dac = gain_table[j].dac;
		gm_gain = gain_table[j].gm;

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (pi_lcn->gmgc2g != -1)
				gm_gain = pi_lcn->gmgc2g;
			if (pi_lcn->dacgc2g != -1)
				dac = pi_lcn->dacgc2g;
			if (pi_lcn->pa_gain_ovr_val_2g != -1)
				pa_gain = pi_lcn->pa_gain_ovr_val_2g;
		}
#ifdef BAND5G
		else {
			if (pi_lcn->gmgc5g != -1)
				gm_gain = pi_lcn->gmgc5g;
			if (pi_lcn->dacgc5g != -1)
				dac = pi_lcn->dacgc5g;
			if (pi_lcn->pa_gain_ovr_val_5g != -1)
				pa_gain = pi_lcn->pa_gain_ovr_val_5g;
		}
#endif
		val = (((uint32)pa_gain << 24) |
			(gain_table[j].pad << 16) |
			(gain_table[j].pga << 8) | gm_gain);

		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + j;
		wlc_lcn40phy_write_table(pi, &tab);

		tab.tbl_ptr = &val; /* ptr to buf */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + j;
		wlc_lcn40phy_read_table(pi, &tab);
		val = val & (0xFFFFF);
		val |= (dac << 28) |
			(gain_table[j].bb_mult << 20);
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + j;
		wlc_lcn40phy_write_table(pi, &tab);
	}
}

static void wlc_lcn40phy_load_rfpower(phy_info_t *pi)
{
	uint8 k;
	phytbl_info_t tab;
	uint32 val, bbmult, rfgain;
	uint8 indx;
	uint8 scale_factor = 1;
	uint8 papd_rf_pwr_scale; /* Q4, scale papd rf power scale adjustment */
	int16 temp, temp1, temp2, qQ, qQ1, qQ2, shift;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);


	if (pi_lcn->papd_rf_pwr_scale != 0) {
		papd_rf_pwr_scale = pi_lcn->papd_rf_pwr_scale;
	} else {
		papd_rf_pwr_scale = 8;
	}

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;        /* # values   */

	for (indx = 0; indx < 128; indx++) {
		tab.tbl_ptr = &bbmult; /* ptr to buf */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + indx;
		wlc_lcn40phy_read_table(pi, &tab);
		bbmult = bbmult >> 20;

		tab.tbl_ptr = &rfgain; /* ptr to buf */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + indx;
		wlc_lcn40phy_read_table(pi, &tab);

		qm_log10((int32)(bbmult), 0, &temp1, &qQ1);
		qm_log10((int32)(1<<6), 0, &temp2, &qQ2);

		if (qQ1 < qQ2) {
			temp2 = qm_shr16(temp2, qQ2-qQ1);
			qQ = qQ1;
		}
		else {
			temp1 = qm_shr16(temp1, qQ1-qQ2);
			qQ = qQ2;
		}
		temp = qm_sub16(temp1, temp2);

		if (qQ >= 4)
			shift = qQ-4;
		else
			shift = 4-qQ;
		/* now in q4 */
		val = (((indx << shift) + (5*temp) +
			(1<<(scale_factor+shift-3)))>>(scale_factor+shift-2));

		/* papd_rf_pwr_scale, q4 */
		/* factor of 2 implicit in previous */
		/* calculation */
		val = val*papd_rf_pwr_scale/8;
		/* add vlin at bit 10 */
		if (pi_lcn->ePA)
			val |= (1 << 10);
		else
			val |= (0 << 10);
		PHY_INFORM(("idx = %d, bb: %d, tmp = %d, qQ = %d, sh = %d, val = %d, rfgain = %x\n",
			indx, bbmult, temp, qQ, shift, val, rfgain));

		tab.tbl_ptr = &val;
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
		wlc_lcn40phy_write_table(pi, &tab);
	}
	k =  pi_lcn->lcnphy_capped_index;
	for (indx = 0; indx <=  pi_lcn->lcnphy_capped_index; indx++) {
		tab.tbl_ptr = &val; /* ptr to buf */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + k;
		wlc_lcn40phy_read_table(pi, &tab);

		tab.tbl_ptr = &val;
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
		wlc_lcn40phy_write_table(pi, &tab);
	}
}

static void
WLBANDINITFN(wlc_lcn40phy_tx_pwr_ctrl_init)(phy_info_t *pi)
{
	bool suspend;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (NORADIO_ENAB(pi->pubpi)) {
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		return;
	}

	if (!pi->hwpwrctrl_capable) {
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	} else {

		wlc_lcn40phy_idle_tssi_est(pi);

		/* Clear out all power offsets */
		wlc_lcn40phy_clear_tx_power_offsets(pi);

		/* Convert tssi to power LUT */
		wlc_lcn40phy_set_estPwrLUT(pi, 0);

		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable, 0);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrMinMaxVal, pwrMinVal, 0);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrMinMaxVal, pwrMaxVal, 0);

		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, txGainTable_mode, 0);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, interpol_en, 0);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			wlc_lcn40phy_set_estPwrLUT(pi, 1);

			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrRange2, pwrMin_range2, pi_lcn->pmin);
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrRange2, pwrMax_range2, pi_lcn->pmax);

			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable2, 0);
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrMinMaxVal2, pwrMinVal2, 0);
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlPwrMinMaxVal2, pwrMaxVal2, 0);

			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, interpol_en1, 0);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */

		wlc_lcn40phy_set_tssi_pwr_limit(pi, PHY_TSSI_SET_MIN_MAX_LIMIT);

		PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, crseddisable, 0);
		phy_reg_write(pi, LCN40PHY_TxPwrCtrlDeltaPwrLimit, 10);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset, pi_lcn->cckPwrOffset);

		/* 4314/43142, tssi-vs-power curve is 0 - 1  dB higher in BW40 compared to BW20 */
		/* For now, do not apply any correction. */
		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
				PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection, tempsenseCorr, 0);
		}
		wlc_lcn40phy_set_target_tx_pwr(pi, LCN40_TARGET_PWR);

#ifdef WLNOKIA_NVMEM
		/* update the cck power detector offset, these are in 1/8 dBs */
		{
			int8 cckoffset, ofdmoffset;
			int16 diff;
			/* update the cck power detector offset */
			wlc_phy_get_pwrdet_offsets(pi, &cckoffset, &ofdmoffset);
			PHY_INFORM(("cckoffset is %d and ofdmoffset is %d\n",
				cckoffset, ofdmoffset));

			diff = cckoffset - ofdmoffset;
			if (diff >= 0)
				diff &= 0x0FF;
			else
				diff &= 0xFF | 0x100;

			/* program the cckpwoffset bits(6-14) in reg 0x4d0 */
			PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset, diff);

			/* program the tempsense curr in reg 0x50c */
			if (ofdmoffset >= 0)
				diff = ofdmoffset;
			else
				diff = 0x100 | ofdmoffset;
			PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection, tempsenseCorr, diff);
		}
#endif /* WLNOKIA_NVMEM */

		if (CHSPEC_IS2G(pi->radio_chanspec) && pi_lcn->init_txpwrindex_2g)
			pi_lcn->lcnphy_tssi_idx = pi_lcn->init_txpwrindex_2g;
		else if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (pi_lcn->init_txpwrindex_5g)
				pi_lcn->lcnphy_tssi_idx = pi_lcn->init_txpwrindex_5g;
		} else
			pi_lcn->lcnphy_tssi_idx = 60;

		/* Enable hardware power control */
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_HW);
	}
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

static void
WLBANDINITFN(wlc_lcn40phy_sw_ctrl_tbl_init)(phy_info_t *pi)
{

	phytbl_info_t tab;
	uint16 *tbl_ptr;
	uint16 tblsz;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint8 idx;
	uint32 *swctrlmap;
	uint16 tdm, ovr_en;

	tblsz = LCN40PHY_SW_CTRL_TBL_LENGTH * (LCN40PHY_SW_CTRL_TBL_WIDTH >> 3);

	if ((tbl_ptr = MALLOC(pi->sh->osh, tblsz)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n",
			pi->sh->unit, __FUNCTION__));
		return;
	}

	swctrlmap = pi_lcn->swctrlmap_2g;
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec))
		swctrlmap = pi_lcn->swctrlmap_5g;
#endif

	tdm = (swctrlmap[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_TDM);
	ovr_en = (swctrlmap[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_OVR_EN);

	for (idx = 0; idx < LCN40PHY_SW_CTRL_TBL_LENGTH; idx++) {
		uint8	bt_pri = idx & LCN40PHY_SW_CTRL_MAP_BT_PRIO;
		uint16	ant = idx & LCN40PHY_SW_CTRL_MAP_ANT;

		tbl_ptr[idx] = 0;
		/* BT Prio */
		if (bt_pri) {
			/* Diasble diversity in WL to ensure
			both BT and WL recieve from the same ant
			*/
			if (ovr_en)
				ant = (swctrlmap[LCN40PHY_I_WL_MASK] & LCN40PHY_MASK_ANT);

			if (idx & LCN40PHY_SW_CTRL_MAP_BT_TX)
				/* BT Tx */
				tbl_ptr[idx] |=
				(swctrlmap[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_TX)
					>> LCN40PHY_SHIFT_BT_TX;
			else {
				/* BT Rx */
				if (idx & LCN40PHY_SW_CTRL_MAP_ELNA)
					tbl_ptr[idx] |=
					(swctrlmap[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_ELNARX)
					>> LCN40PHY_SHIFT_BT_ELNARX;
				else
					tbl_ptr[idx] |=
					swctrlmap[LCN40PHY_I_BT] & LCN40PHY_MASK_BT_RX;
			}
		}
		/* WL Tx/Rx */
		if (!tdm || !bt_pri) {
			if (idx & LCN40PHY_SW_CTRL_MAP_WL_TX) {
				/* PA on */
				if (idx & LCN40PHY_SW_CTRL_MAP_WL_RX)
					/* Rx with PA on */
					tbl_ptr[idx] |=
					LCN40PHY_DIV_BASED_INIT_H(ant, LCN40PHY_I_WL_TX);
				else
					/* WL Tx with PA on */
					tbl_ptr[idx] |=
					LCN40PHY_DIV_BASED_INIT_L(ant, LCN40PHY_I_WL_TX);
			} else {
				if (idx & LCN40PHY_SW_CTRL_MAP_ELNA) {
					if (idx & LCN40PHY_SW_CTRL_MAP_WL_RX)
						/* WL Rx eLNA */
						tbl_ptr[idx] |=
						LCN40PHY_DIV_BASED_INIT_H(ant, LCN40PHY_I_WL_RX);
					else
						/* WL Rx Attn eLNA */
						tbl_ptr[idx] |=
						LCN40PHY_DIV_BASED_INIT_H
						(ant, LCN40PHY_I_WL_RX_ATTN);
				} else { /* Without eLNA */
					if (idx & LCN40PHY_SW_CTRL_MAP_WL_RX)
						/* WL Rx */
						tbl_ptr[idx] |=
						LCN40PHY_DIV_BASED_INIT_L(ant, LCN40PHY_I_WL_RX);
					else
						/* WL Rx Attn */
						tbl_ptr[idx] |=
						LCN40PHY_DIV_BASED_INIT_L
						(ant, LCN40PHY_I_WL_RX_ATTN);
				}
			}
		}
	}

	/* Writing the fields into the LCN40PHY_swctrlconfig register */
	phy_reg_mod(pi, LCN40PHY_sw_ctrl_config, LCN40PHY_sw_ctrl_config_sw_ctrl_mask_MASK, 0x3ff);

	/* Write the populated sw ctrl table to the default switch ctrl table location */
	tab.tbl_len = LCN40PHY_SW_CTRL_TBL_LENGTH;
	tab.tbl_id = LCN40PHY_TBL_ID_SW_CTRL;
	tab.tbl_offset = 0;
	tab.tbl_width = LCN40PHY_SW_CTRL_TBL_WIDTH;
	tab.tbl_ptr = tbl_ptr;
	wlc_lcn40phy_write_table(pi, &tab);

	MFREE(pi->sh->osh, tbl_ptr, tblsz);

}

static void
WLBANDINITFN(wlc_lcn40phy_tbl_init)(phy_info_t *pi)
{

	uint idx, tbl_info_sz;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phytbl_info_t *tbl_info = NULL;
	phy_tx_gain_tbl_entry *gaintable = NULL;
	uint8 phybw40;
	phybw40 = CHSPEC_IS40(pi->radio_chanspec);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) || LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 4)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev0;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev0;
		if (CHSPEC_IS2G(pi->radio_chanspec) || NORADIO_ENAB(pi->pubpi))
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev0;
#ifdef BAND5G
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_5GHz_gaintable_rev0;
#endif
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 1)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev1;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev1;
		if (pi_lcn->ePA)
			gaintable = (phy_tx_gain_tbl_entry *)
						dot11lcn40phy_2GHz_extPA_gaintable_rev1;
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev1;
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev3;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev3;
		if (pi_lcn->ePA)
			gaintable = (phy_tx_gain_tbl_entry *)
						dot11lcn40phy_2GHz_extPA_gaintable_rev1;
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev3;
	} else {
		tbl_info_sz = dot11lcn40phytbl_info_sz_rev0;
		tbl_info = (phytbl_info_t *)dot11lcn40phytbl_info_rev0;
		if (CHSPEC_IS2G(pi->radio_chanspec) || NORADIO_ENAB(pi->pubpi))
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_2GHz_gaintable_rev0;
#ifdef BAND5G
		else
			gaintable = (phy_tx_gain_tbl_entry *)dot11lcn40phy_5GHz_gaintable_rev0;
#endif
	}

	for (idx = 0; idx < tbl_info_sz; idx++)
		wlc_lcn40phy_write_table(pi, &tbl_info[idx]);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 0) || LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
		LCN40REV_IS(pi->pubpi.phy_rev, 4)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_ext_lna_rx_gain_info_rev0[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_rx_gain_info_rev0[idx]);
				}
			}
		}
#ifdef BAND5G
		else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_ext_lna_rx_gain_info_rev0[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_rx_gain_info_rev0[idx]);
				}
			}
		}
#endif /* BAND5G */
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 1)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_40mhz_rx_gain_info_sz_rev1;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_40mhz_rx_gain_info_rev1[idx]);
				}
			} else {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev1;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_rx_gain_info_rev1[idx]);
				}
			}
		}
	} else if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev3;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_rx_gain_info_rev3[idx]);
				}
			} else {
				for (idx = 0;
				      idx < dot11lcn40phytbl_2G_rx_gain_info_sz_rev3;
				      idx++) {
					wlc_lcn40phy_write_table(pi,
						&dot11lcn40phytbl_2G_rx_gain_info_rev3[idx]);
				}
			}
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_2G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_2G_ext_lna_rx_gain_info_rev0[idx]);
				}
			}
		}
#ifdef BAND5G
		else {
			if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_ext_lna_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_ext_lna_rx_gain_info_rev0[idx]);
				}
			} else {
				for (idx = 0;
				     idx < dot11lcn40phytbl_5G_rx_gain_info_sz_rev0;
				     idx++) {
					wlc_lcn40phy_write_table(pi,
					    &dot11lcn40phytbl_5G_rx_gain_info_rev0[idx]);
				}
			}
		}
#endif /* BAND5G */
	}

	wlc_lcn40phy_load_tx_gain_table(pi, gaintable);

	/* Change switch table if neccessary based on chip and board type */
	wlc_lcn40phy_sw_ctrl_tbl_init(pi);

	wlc_lcn40phy_load_rfpower(pi);

	/* clear our PAPD Compensation table */
	wlc_lcn40phy_clear_papd_comptable(pi);
}

/* mapped to lcn40phy_rev0_reg_init */
static void
WLBANDINITFN(wlc_lcn40phy_rev0_reg_init)(phy_info_t *pi)
{
	if (NORADIO_ENAB(pi->pubpi)) {
		phy_reg_write(pi, LCN40PHY_tx_resampler1, 0xe666);
		phy_reg_write(pi, LCN40PHY_tx_resampler3, 0);
		phy_reg_write(pi, LCN40PHY_tx_resampler4, 0x32);
		phy_reg_write(pi, LCN40PHY_rxFarrowCtrl, 0x57);
		phy_reg_write(pi, LCN40PHY_rxFarrowDeltaPhase, 0x2000);
		phy_reg_write(pi, LCN40PHY_rxFarrowDriftPeriod, 0x780);
		PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 0);
	}

	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x0004);
	phy_reg_write(pi, LCN40PHY_AfeCtrlOvr, 0x0000);
	phy_reg_write(pi, LCN40PHY_AfeCtrlOvr1, 0x0000);
	phy_reg_write(pi, LCN40PHY_RFOverride0, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride2, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride3, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride4, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride7, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride8, 0x0000);
	phy_reg_write(pi, LCN40PHY_swctrlOvr, 0x0000);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0x1);

	PHY_REG_MOD(pi, LCN40PHY, wl_gain_tbl_offset, wl_gain_tbl_offset, 18);
	PHY_REG_MOD(pi, LCN40PHY, nftrAdj, bt_gain_tbl_offset, 6);

	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x0000);
	PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 1);
		mod_radio_reg(pi, RADIO_2065_OVR9, 0x8, 0x8);

		if (CHSPEC_IS40(pi->radio_chanspec))
			write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
		else
		    write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x750);

		write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x1b61);
		write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0x604);

		OSL_DELAY(1000);

	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, rfpll_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, rfpll_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1);
	PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 0);
	PHY_REG_MOD(pi, LCN40PHY, rxfe, swap_rxfe_iq, 1);
	/* band selection */
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 0);
	} else
#endif
	{
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 1);
	}
	/* bphy filter selection , for channel 14 it is 2 */
	PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, txfiltSelect, 1);

	if (NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn40phy_set_bbmult(pi, 32);
		return;
	}
}

/* mapped to lcn40phy_rev1_reg_init */
static void
WLBANDINITFN(wlc_lcn40phy_rev1_reg_init)(phy_info_t *pi)
{
	if (NORADIO_ENAB(pi->pubpi)) {
		phy_reg_write(pi, LCN40PHY_tx_resampler1, 0xe666);
		phy_reg_write(pi, LCN40PHY_tx_resampler3, 0);
		phy_reg_write(pi, LCN40PHY_tx_resampler4, 0x32);
		phy_reg_write(pi, LCN40PHY_rxFarrowCtrl, 0x57);
		phy_reg_write(pi, LCN40PHY_rxFarrowDeltaPhase, 0x2000);
		phy_reg_write(pi, LCN40PHY_rxFarrowDriftPeriod, 0x780);
		PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 0);
	}

	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x0004);
	phy_reg_write(pi, LCN40PHY_AfeCtrlOvr, 0x0000);
	phy_reg_write(pi, LCN40PHY_AfeCtrlOvr1, 0x0000);
	phy_reg_write(pi, LCN40PHY_RFOverride0, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride2, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride3, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride4, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride7, 0x0000);
	phy_reg_write(pi, LCN40PHY_rfoverride8, 0x0000);
	phy_reg_write(pi, LCN40PHY_swctrlOvr, 0x0000);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0x1);

	PHY_REG_MOD(pi, LCN40PHY, wl_gain_tbl_offset, wl_gain_tbl_offset, 18);
	PHY_REG_MOD(pi, LCN40PHY, nftrAdj, bt_gain_tbl_offset, 6);

	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x0000);
	PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 1);
	mod_radio_reg(pi, RADIO_2065_OVR9, 0x8, 0x8);

	if (CHSPEC_IS40(pi->radio_chanspec))
		write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x740);
	else
		write_radio_reg(pi, RADIO_2065_RFPLL_CFG2, 0x750);

	write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x1b61);
	write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0x604);

	OSL_DELAY(1000);

	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, rfpll_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, rfpll_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1);
	PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, disable_stalls, 0);
	PHY_REG_MOD(pi, LCN40PHY, rxfe, swap_rxfe_iq, 1);
	/* band selection */
#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 0);
	} else
#endif
	{
		PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, muxGmode, 1);
	}
	/* bphy filter selection , for channel 14 it is 2 */
	PHY_REG_MOD(pi, LCN40PHY, lpphyCtrl, txfiltSelect, 1);

	if (CHSPEC_IS20(pi->radio_chanspec))
		wlc_lcn40phy_filt_bw_set(pi, 3);
	else
		wlc_lcn40phy_filt_bw_set(pi, 4);

	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID) {
		PHY_ERROR(("%s: Fixing jtag2065 register init values.", __FUNCTION__));
		write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0xcccc);
		write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xba4);
		mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0x1f0, 0x70);
		mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0x1f, 0x4);
		mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0x1f1f, 0x1c1c);
		mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0x1f1f, 0x1c1c);
		mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0x3f, 0xf);
		mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0x3f3f, 0xf21);
		mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xe0, 0xa0);
		mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xfff0, 0xe8d0);
		mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0x1f00, 0xf00);
		mod_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0xfff, 0x174);
		write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x999a);
	}
	if (NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn40phy_set_bbmult(pi, 32);
		return;
	}
}

static void
wlc_lcn40phy_agc_temp_init(phy_info_t *pi)
{
}

static void
WLBANDINITFN(wlc_lcn40phy_bu_tweaks)(phy_info_t *pi)
{
}

/* Read band specific data from the SROM */
static bool
BCMATTACHFN(wlc_lcn40phy_txpwr_srom_read)(phy_info_t *pi)
{

	int8 txpwr = 0;
	int i;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;
	uint sromrev;

	uint16 cckpo = 0;
	uint32 offset_ofdm, offset_mcs;

	/* TR switch isolation */
	pi_lcn->lcnphy_tr_isolation_mid = (uint8)PHY_GETINTVAR(pi, "triso2g");

	pi_lcn40->trGain = (uint8)PHY_GETINTVAR_DEFAULT(pi, "gain", 0xff);
	/* Input power offset */
	pi_lcn->lcnphy_rx_power_offset = (uint8)PHY_GETINTVAR(pi, "rxpo2g");
	/* pa0b0 */
	pi->txpa_2g[0] = (int16)PHY_GETINTVAR(pi, "pa0b0");
	pi->txpa_2g[1] = (int16)PHY_GETINTVAR(pi, "pa0b1");
	pi->txpa_2g[2] = (int16)PHY_GETINTVAR(pi, "pa0b2");

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (PHY_GETVAR(pi, "pa0b0_lo"))
			pi_lcn->lcnphy_twopwr_txpwrctrl_en = 1;
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			pi->txpa_2g_lo[0] = (int16)PHY_GETINTVAR(pi, "pa0b0_lo");
			pi->txpa_2g_lo[1] = (int16)PHY_GETINTVAR(pi, "pa0b1_lo");
			pi->txpa_2g_lo[2] = (int16)PHY_GETINTVAR(pi, "pa0b2_lo");
			pi_lcn->pmin = (int8)PHY_GETINTVAR(pi, "pmin");
			pi_lcn->pmax = (int8)PHY_GETINTVAR(pi, "pmax");
		}
#endif

	pi_lcn->lcnphy_tssical_time = (uint32)PHY_GETINTVAR(pi, "tssitime");
	pi_lcn->init_txpwrindex_2g = (uint32)PHY_GETINTVAR(pi, "initxidx");
	pi_lcn->tssi_ladder_offset_maxpwr_2g =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmax", 8);
	pi_lcn->tssi_ladder_offset_minpwr_2g =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmin", 3);
	pi_lcn->tssi_ladder_offset_maxpwr_5glo = (
		uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmax5gl", 8);
	pi_lcn->tssi_ladder_offset_minpwr_5glo =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmin5gl", 3);
	pi_lcn->tssi_ladder_offset_maxpwr_5gmid =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmax5gm", 8);
	pi_lcn->tssi_ladder_offset_minpwr_5gmid =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmin5gm", 3);
	pi_lcn->tssi_ladder_offset_maxpwr_5ghi =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmax5gh", 8);
	pi_lcn->tssi_ladder_offset_minpwr_5ghi =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, "tssioffsetmin5gh", 3);

	pi_lcn->tssi_max_npt = (uint8)PHY_GETINTVAR_DEFAULT(pi, "tssimaxnpt",
		LCN40PHY_TX_PWR_CTRL_MAX_NPT);

	/* RSSI */
	pi_lcn->lcnphy_rssi_vf = (uint8)PHY_GETINTVAR(pi, "rssismf2g");
	pi_lcn->lcnphy_rssi_vc = (uint8)PHY_GETINTVAR(pi, "rssismc2g");
	pi_lcn->lcnphy_rssi_gs = (uint8)PHY_GETINTVAR(pi, "rssisav2g");

	{
		pi_lcn->lcnphy_rssi_vf_lowtemp = pi_lcn->lcnphy_rssi_vf;
		pi_lcn->lcnphy_rssi_vc_lowtemp = pi_lcn->lcnphy_rssi_vc;
		pi_lcn->lcnphy_rssi_gs_lowtemp = pi_lcn->lcnphy_rssi_gs;

		pi_lcn->lcnphy_rssi_vf_hightemp = pi_lcn->lcnphy_rssi_vf;
		pi_lcn->lcnphy_rssi_vc_hightemp = pi_lcn->lcnphy_rssi_vc;
		pi_lcn->lcnphy_rssi_gs_hightemp = pi_lcn->lcnphy_rssi_gs;
	}

	/* Max tx power */
	txpwr = (int8)PHY_GETINTVAR(pi, "maxp2ga0");
	pi->tx_srom_max_2g = txpwr;

	for (i = 0; i < PWRTBL_NUM_COEFF; i++) {
		pi->txpa_2g_low_temp[i] = pi->txpa_2g[i];
		pi->txpa_2g_high_temp[i] = pi->txpa_2g[i];
	}

	sromrev = (uint)PHY_GETINTVAR(pi, "sromrev");
	if (sromrev >= 9)
		cckpo = (uint16)PHY_GETINTVAR(pi, "cckbw202gpo");
	else
		cckpo = (uint16)PHY_GETINTVAR(pi, "cck2gpo");

	/* Extract offsets for 4 CCK rates. Remember to convert from
	 * .5 to .25 dbm units
	 */
	for (i = TXP_FIRST_CCK; i <= TXP_LAST_CCK; i++) {
		pi->tx_srom_max_rate_2g[i] = txpwr -
			((cckpo & 0xf) * 2);
		cckpo >>= 4;
	}

	/* Extract offsets for 8 OFDM rates */
	if (sromrev >= 9)
		offset_ofdm = (uint32)PHY_GETINTVAR(pi, "legofdmbw202gpo");
	else
		offset_ofdm = (uint32)PHY_GETINTVAR(pi, "ofdm2gpo");
	for (i = TXP_FIRST_OFDM; i <= TXP_LAST_OFDM; i++) {
		pi->tx_srom_max_rate_2g[i] = txpwr -
			((offset_ofdm & 0xf) * 2);
		offset_ofdm >>= 4;
	}

	/* Extract offsets for 8 MCS rates */
	/* mcs2gpo(x) are 16 bit numbers */
	if (sromrev >= 9)
		offset_mcs = (uint32)PHY_GETINTVAR(pi, "mcsbw202gpo");
	else
		offset_mcs =
			((uint16)PHY_GETINTVAR(pi, "mcs2gpo1") << 16) |
			(uint16)PHY_GETINTVAR(pi, "mcs2gpo0");
	pi_lcn->lcnphy_mcs20_po = offset_mcs;

	for (i = TXP_FIRST_MCS_20_SS; i <= TXP_LAST_MCS_20_SISO_SS;  i++) {
		pi->tx_srom_max_rate_2g[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}
	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	if (sromrev >= 9)
		offset_mcs = (uint32)PHY_GETINTVAR(pi, "mcsbw402gpo");
	else
		offset_mcs =
			((uint16)PHY_GETINTVAR(pi, "mcs2gpo3") << 16) |
			(uint16)PHY_GETINTVAR(pi, "mcs2gpo2");
	if (!offset_mcs)
		offset_mcs = pi_lcn->lcnphy_mcs20_po;

	for (i = TXP_FIRST_MCS_40_SS; i <= TXP_LAST_MCS_40_SISO_SS;  i++) {
		pi->tx_srom_max_rate_2g[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}

#ifdef BAND5G
	/* Max tx power for 5G */
	txpwr = (int8)PHY_GETINTVAR(pi, "maxp5ga0");
	pi->tx_srom_max_5g_mid = txpwr;

	/* Extract offsets for 8 OFDM mid rates */
	offset_ofdm = (uint32)PHY_GETINTVAR(pi, "ofdm5gpo");
	for (i = TXP_FIRST_OFDM; i <= TXP_LAST_OFDM; i++) {
		pi->tx_srom_max_rate_5g_mid[i] = txpwr -
			((offset_ofdm & 0xf) * 2);
		offset_ofdm >>= 4;
	}

	/* Extract offsets for 8 MCS mid rates */
	/* mcs2gpo(x) are 16 bit numbers */
	offset_ofdm = offset_mcs =
		((uint16)PHY_GETINTVAR(pi, "mcs5gpo1") << 16) |
		(uint16)PHY_GETINTVAR(pi, "mcs5gpo0");
	for (i = TXP_FIRST_MCS_20_SS; i <= TXP_LAST_MCS_20_SISO_SS;  i++) {
		pi->tx_srom_max_rate_5g_mid[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}
	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	offset_mcs =
		((uint16)PHY_GETINTVAR(pi, "mcs5ghpo3") << 16) |
		(uint16)PHY_GETINTVAR(pi, "mcs5ghpo2");
	if (!offset_mcs)
		offset_mcs = offset_ofdm;

	for (i = TXP_FIRST_MCS_40_SS; i <= TXP_LAST_MCS_40_SISO_SS;  i++) {
		pi->tx_srom_max_rate_5g_mid[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}

	txpwr = (int8)PHY_GETINTVAR(pi, "maxp5gla0");
	pi->tx_srom_max_5g_low = txpwr;

	/* Extract offsets for 8 OFDM low rates */
	offset_ofdm = (uint32)PHY_GETINTVAR(pi, "ofdm5glpo");
	for (i = TXP_FIRST_OFDM; i <= TXP_LAST_OFDM; i++) {
		pi->tx_srom_max_rate_5g_low[i] = txpwr -
			((offset_ofdm & 0xf) * 2);
		offset_ofdm >>= 4;
	}

	/* Extract offsets for 8 MCS low rates */
	/* mcs2gpo(x) are 16 bit numbers */
	offset_ofdm = offset_mcs =
		((uint16)PHY_GETINTVAR(pi, "mcs5glpo1") << 16) |
		(uint16)PHY_GETINTVAR(pi, "mcs5glpo0");
	for (i = TXP_FIRST_MCS_20_SS; i <= TXP_LAST_MCS_20_SISO_SS;  i++) {
		pi->tx_srom_max_rate_5g_low[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}
	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	offset_mcs =
		((uint16)PHY_GETINTVAR(pi, "mcs5glpo3") << 16) |
		(uint16)PHY_GETINTVAR(pi, "mcs5glpo2");
	if (!offset_mcs)
		offset_mcs = offset_ofdm;

	for (i = TXP_FIRST_MCS_40_SS; i <= TXP_LAST_MCS_40_SISO_SS;  i++) {
		pi->tx_srom_max_rate_5g_low[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}

	txpwr = (int8)PHY_GETINTVAR(pi, "maxp5gha0");
	pi->tx_srom_max_5g_hi = txpwr;

	/* Extract offsets for 8 OFDM high rates */
	offset_ofdm = (uint32)PHY_GETINTVAR(pi, "ofdm5ghpo");
	for (i = TXP_FIRST_OFDM; i <= TXP_LAST_OFDM; i++) {
		pi->tx_srom_max_rate_5g_hi[i] = txpwr -
			((offset_ofdm & 0xf) * 2);
		offset_ofdm >>= 4;
	}

	/* Extract offsets for 8 MCS high rates */
	/* mcs2gpo(x) are 16 bit numbers */
	offset_ofdm = offset_mcs =
		((uint16)PHY_GETINTVAR(pi, "mcs5ghpo1") << 16) |
		(uint16)PHY_GETINTVAR(pi, "mcs5ghpo0");
	for (i = TXP_FIRST_MCS_20_SS; i <= TXP_LAST_MCS_20_SISO_SS;	i++) {
		pi->tx_srom_max_rate_5g_hi[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}
	/* Extract offsets for 8 MCS 40MHz rates */
	/* mcs2gpo(x) are 16 bit numbers */
	offset_mcs =
		((uint16)PHY_GETINTVAR(pi, "mcs5ghpo3") << 16) |
		(uint16)PHY_GETINTVAR(pi, "mcs5ghpo2");
	if (!offset_mcs)
		offset_mcs = offset_ofdm;

	for (i = TXP_FIRST_MCS_40_SS; i <= TXP_LAST_MCS_40_SISO_SS;  i++) {
		pi->tx_srom_max_rate_5g_hi[i] = txpwr -
			((offset_mcs & 0xf) * 2);
		offset_mcs >>= 4;
	}
#endif /* #ifdef BAND5G */

	/* for tempcompensated tx power control */
	pi_lcn->lcnphy_rawtempsense = (uint16)PHY_GETINTVAR(pi, "rawtempsense");
	pi_lcn->lcnphy_measPower = (uint8)PHY_GETINTVAR(pi, "measpower");
	pi_lcn->lcnphy_measPower1 = (uint8)PHY_GETINTVAR(pi, "measpower1");
	pi_lcn->lcnphy_measPower2 = (uint8)PHY_GETINTVAR(pi, "measpower2");
	pi_lcn->lcnphy_tempsense_slope = (uint8)PHY_GETINTVAR(pi, "tempsense_slope");
	pi_lcn->lcnphy_hw_iqcal_en = (bool)PHY_GETINTVAR(pi, "hw_iqcal_en");
	pi_lcn->lcnphy_iqcal_swp_dis = (bool)PHY_GETINTVAR(pi, "iqcal_swp_dis");
	pi_lcn->lcnphy_tempcorrx = (int8)PHY_GETINTVAR(pi, "tempcorrx");
	pi_lcn->lcnphy_tempsense_option = (uint8)PHY_GETINTVAR(pi, "tempsense_option");
	pi_lcn->lcnphy_freqoffset_corr = (uint8)PHY_GETINTVAR(pi, "freqoffset_corr");
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, "aa2g");
	pi_lcn->extpagain2g = (uint8)PHY_GETINTVAR(pi, "extpagain2g");
	pi_lcn->extpagain5g = (uint8)PHY_GETINTVAR(pi, "extpagain5g");
#if defined(WLTEST)
	pi_lcn->txpwrindex_nvram = (uint8)PHY_GETINTVAR(pi, "txpwrindex");
#endif 
	if (pi->aa2g >= 1)
			wlc_lcn40phy_decode_aa2g(pi, pi->aa2g);

	pi_lcn->lcnphy_cck_dig_filt_type =
		(int16)PHY_GETINTVAR_DEFAULT(pi, "cckdigfilttype", -1);
	if (pi_lcn->lcnphy_cck_dig_filt_type < 0) {
		pi_lcn->lcnphy_cck_dig_filt_type = -1;
	}

	pi_lcn->lcnphy_ofdm_dig_filt_type =
		(int16)PHY_GETINTVAR_DEFAULT(pi, "ofdmdigfilttype", -1);
	if (pi_lcn->lcnphy_ofdm_dig_filt_type < 0) {
		pi_lcn->lcnphy_ofdm_dig_filt_type = -1;
	}

	/* pa gain override */
	pi_lcn->pa_gain_ovr_val_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, "pagc2g", -1);
	pi_lcn->pa_gain_ovr_val_5g = (int16)PHY_GETINTVAR_DEFAULT(pi, "pagc5g", -1);
	pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val = (int16)PHY_GETINTVAR(pi, "txiqlotf");

	/* Coefficients for Temperature Conversion to Centigrade */
	pi_lcn->temp_mult = (int32)PHY_GETINTVAR_DEFAULT(pi, "temp_mult", 414); /* .4043 << 10 */
	pi_lcn->temp_add = (int32)PHY_GETINTVAR_DEFAULT(pi, "temp_add", 32410); /* 31.657 << 10 */
	pi_lcn->temp_q = (int32)PHY_GETINTVAR_DEFAULT(pi, "temp_q", 10);

	/* Coefficients for vbat conversion to Volts */
	pi_lcn->vbat_mult = (int32)PHY_GETINTVAR_DEFAULT(pi, "vbat_mult", 8);  /* .008 << 10 */
	pi_lcn->vbat_add = (int32)PHY_GETINTVAR_DEFAULT(pi, "vbat_add", 4206); /* 4.1072 << 10 */
	pi_lcn->vbat_q = (int32)PHY_GETINTVAR_DEFAULT(pi, "vbat_q", 10);

	/* Offset for the CCK power detector */
	if (PHY_GETVAR(pi, "cckPwrOffset")) {
		pi_lcn->cckPwrOffset = (int16)PHY_GETINTVAR(pi, "cckPwrOffset");
	} else {
		uint boardtype, boardrev;
		boardtype = PHY_GETINTVAR(pi, "boardtype");
		boardrev = PHY_GETINTVAR(pi, "boardrev");

		if (boardtype == 0x5e0 && boardrev >= 0x1203)
			pi_lcn->cckPwrOffset = 16;
		else
			pi_lcn->cckPwrOffset = 10;

		PHY_INFORM(("Use driver defaults %d for cckPwrOffset.\n", pi_lcn->cckPwrOffset));
	}

	/* CCK Power Index Correction */
	pi_lcn->cckPwrIdxCorr = (int16)PHY_GETINTVAR(pi, "cckPwrIdxCorr");

	/* DAC rate */
	pi_lcn->dacrate = (uint8)PHY_GETINTVAR_DEFAULT(pi, "dacrate", 80);

	/* rfreg033 value */
	pi_lcn->rfreg033 = (uint8)PHY_GETINTVAR(pi, "rfreg033");
	pi_lcn->rfreg033_cck = (uint8)PHY_GETINTVAR(pi, "rfreg033_cck");

	/* PA Cal Idx 2g */
	pi_lcn->pacalidx_2g = (uint8)PHY_GETINTVAR(pi, "pacalidx2g");

	/* PA Cal Idx 5g */
	pi_lcn->pacalidx_5g = (uint8)PHY_GETINTVAR(pi, "pacalidx5g");
	pi_lcn->papd_rf_pwr_scale = (uint8)PHY_GETINTVAR(pi, "parfps");

	/* For 4314/43142 PCIE, the swctrlmap_2g come from here. For SDIO, it should be in nvram */
	if ((CHIPID(pi->sh->chip) == BCM4314_CHIP_ID || CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) &&
		BUSTYPE(pi->sh->bustype) == PCI_BUS) {
		if (PHY_GETVAR(pi, "swctrlmap_2g")) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmap_2g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, "swctrlmap_2g", i);
			}

		} else {
			PHY_ERROR((" No swctrlmap_2g found. Use driver defaults.\n"));

			if (PHY_GETINTVAR(pi, "extpagain2g")) {
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x06020602;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
			} else { /* iPA */
				uint boardtype, boardrev;
				boardtype = PHY_GETINTVAR(pi, "boardtype");
				boardrev = PHY_GETINTVAR(pi, "boardrev");

				if (boardtype == 0x5e0) { /* bcm943142hm */
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else if (boardtype == 0x5d1 && boardrev >= 0x1200 &&
					boardrev <= 0x1250) {	/* bcm94314hm, p200-p250 */
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x06020602;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else if (boardtype == 0x5d1 && boardrev >= 0x1251 &&
					boardrev < 0x1300) {	/* bcm94314hm, p251+ */
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x0c080c08;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x04000400;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00080808;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else if (boardtype == 0x5d1 && boardrev >= 0x1300) {
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x05060506;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00060606;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				} else {
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX] = 0x05060506;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN] = 0x090a090a;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_BT] = 0x00060606;
					pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK] = 0x6ff;
				}
			}

			PHY_INFORM(("swctrlmap_2g=0x%x,0x%x,0x%x,0x%x,0x%x\n",
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_TX],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_RX_ATTN],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_BT],
				pi_lcn->swctrlmap_2g[LCN40PHY_I_WL_MASK]));
		}
	} else {
		if (PHY_GETVAR(pi, "swctrlmap_2g")) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmap_2g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, "swctrlmap_2g", i);
			}
		} else {
			PHY_ERROR((" Switch control map(swctrlmap_2g) is NOT found"
				"in the NVRAM file %s \n", __FUNCTION__));
			return FALSE;
		}
#ifdef BAND5G
		if (PHY_GETVAR(pi, "swctrlmap_5g")) {
			for (i = 0; i < LCN40PHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_lcn->swctrlmap_5g[i] =
					(uint32) PHY_GETINTVAR_ARRAY(pi, "swctrlmap_5g", i);
			}
		} else {
			PHY_ERROR((" Switch control map(swctrlmap_5g) is NOT found"
				"in the NVRAM file %s \n", __FUNCTION__));
			return FALSE;
		}
#endif
	}


#ifdef BAND5G
	pi_lcn->dacgc5g = (int16)PHY_GETINTVAR_DEFAULT(pi, "dacgc5g", -1);
	pi_lcn->gmgc5g = (int16)PHY_GETINTVAR_DEFAULT(pi, "gmgc5g", -1);

	if (PHY_GETVAR(pi, "pa1lob0") && PHY_GETVAR(pi, "pa1lob1") && PHY_GETVAR(pi, "pa1lob2")) {
		pi->txpa_5g_low[0] = (int16)PHY_GETINTVAR(pi, "pa1lob0");
		pi->txpa_5g_low[1] = (int16)PHY_GETINTVAR(pi, "pa1lob1");
		pi->txpa_5g_low[2] = (int16)PHY_GETINTVAR(pi, "pa1lob2");
	} else {
		pi->txpa_5g_low[0] = -1;
		pi->txpa_5g_low[1] = -1;
		pi->txpa_5g_low[2] = -1;
	}

	if (PHY_GETVAR(pi, "pa1b0") && PHY_GETVAR(pi, "pa1b1") && PHY_GETVAR(pi, "pa1b2")) {
		pi->txpa_5g_mid[0] = (int16)PHY_GETINTVAR(pi, "pa1b0");
		pi->txpa_5g_mid[1] = (int16)PHY_GETINTVAR(pi, "pa1b1");
		pi->txpa_5g_mid[2] = (int16)PHY_GETINTVAR(pi, "pa1b2");
	} else {
		pi->txpa_5g_mid[0] = -1;
		pi->txpa_5g_mid[1] = -1;
		pi->txpa_5g_mid[2] = -1;
	}

	if (PHY_GETVAR(pi, "pa1hib0") && PHY_GETVAR(pi, "pa1hib1") && PHY_GETVAR(pi, "pa1hib2")) {
		pi->txpa_5g_hi[0] = (int16)PHY_GETINTVAR(pi, "pa1hib0");
		pi->txpa_5g_hi[1] = (int16)PHY_GETINTVAR(pi, "pa1hib1");
		pi->txpa_5g_hi[2] = (int16)PHY_GETINTVAR(pi, "pa1hib2");
	} else {
		pi->txpa_5g_hi[0] = -1;
		pi->txpa_5g_hi[1] = -1;
		pi->txpa_5g_hi[2] = -1;
	}

	pi_lcn->rssismf5g = (int16)PHY_GETINTVAR_DEFAULT(pi, "rssismf5g", -1);
	pi_lcn->rssismc5g = (int16)PHY_GETINTVAR_DEFAULT(pi, "rssismc5g", -1);
	pi_lcn->rssisav5g = (int16)PHY_GETINTVAR_DEFAULT(pi, "rssisav5g", -1);
	pi_lcn->init_txpwrindex_5g = (uint32)PHY_GETINTVAR(pi, "initxidx5g");

	if (PHY_GETVAR(pi, "rx_iq_comp_5g")) {
		for (i = 0; i < LCN40PHY_RXIQCOMP_PARAMS; i++) {
			pi_lcn40->rx_iq_comp_5g[i] = (uint16)
				getintvararray(pi->vars, "rx_iq_comp_5g", i);
		}
	}
#endif /* BAND5G */
	pi_lcn->dacgc2g = (int16)PHY_GETINTVAR_DEFAULT(pi, "dacgc2g", -1);
	pi_lcn->gmgc2g = (int16)PHY_GETINTVAR_DEFAULT(pi, "gmgc2g", -1);

	/* ofdm analog filt bw */
	pi->ofdm_analog_filt_bw_override = (int16)PHY_GETINTVAR_DEFAULT(pi, "ofdmanalogfiltbw", -1);

	if (PHY_GETVAR(pi, "txalpfbyp")) {
		pi->tx_alpf_bypass = (int16)PHY_GETINTVAR(pi, "txalpfbyp");
	}


	if (PHY_GETVAR(pi, "bphyscale")) {
		pi->bphy_scale = (int16)PHY_GETINTVAR(pi, "bphyscale");
	}

#if PHY_NOISE_DBG_HISTORY > 0
	pi_lcn->noise.nvram_dbg_noise = (bool)PHY_GETINTVAR(pi, "noise_dbg");
	if (pi_lcn->noise.nvram_dbg_noise) {
	  pi_lcn->noise.nvram_high_gain = (int)PHY_GETINTVAR(pi, "noise_high_gain");
	  pi_lcn->noise.nvram_input_pwr_offset = (int)PHY_GETINTVAR(pi, "noise_input_pwr_offset");
	  if (pi_lcn->noise.nvram_input_pwr_offset > 127)
	    pi_lcn->noise.nvram_input_pwr_offset -= 256;
	  pi_lcn->noise.nvram_nf_substract_val = (int)PHY_GETINTVAR(pi, "noise_nf_substract_val");
	}
#endif
	pi_lcn->noise.nvram_enable_2g =
		(bool)PHY_GETINTVAR_DEFAULT(pi, "noise_cal_enable_2g", TRUE);
	pi_lcn->noise.nvram_po_bias_2g = (int8)PHY_GETINTVAR(pi, "noise_cal_po_bias_2g");

		pi_lcn->noise.nvram_enable_5g =
		(bool)PHY_GETINTVAR_DEFAULT(pi, "noise_cal_enable_5g", TRUE);
	pi_lcn->noise.nvram_po_bias_5g = (int8)PHY_GETINTVAR(pi, "noise_cal_po_bias_5g");

	pi_lcn->noise.nvram_ref_2g = (int8)PHY_GETINTVAR_DEFAULT(pi, "noise_cal_ref_2g", -1);
	pi_lcn->noise.nvram_ref_5g = (int8)PHY_GETINTVAR_DEFAULT(pi, "noise_cal_ref_5g", -1);

	if (PHY_GETVAR(pi, "xtalmode")) {
		pi_lcn->xtal_mode[0] = 1;
		pi_lcn->xtal_mode[1] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, "xtalmode", 0);
		pi_lcn->xtal_mode[2] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, "xtalmode", 1);
		pi_lcn->xtal_mode[3] =
			(uint8) PHY_GETINTVAR_ARRAY(pi, "xtalmode", 2);
	} else {
		pi_lcn->xtal_mode[0] = 0;
	}

#ifdef BAND5G
	pi_lcn->triso5g[0] = (uint8)PHY_GETINTVAR_DEFAULT(pi, "triso5g", 0xff);
#endif /* #ifdef BAND5G */
	return TRUE;
}

static void
wlc_lcn40phy_noise_attach(phy_info_t *pi)
{
}

static void	wlc_lcn40phy_temp_adj(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_clear_papd_comptable(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint16 j;
	uint32 papdcompdeltatbl_init_val;
	tab.tbl_ptr = &papdcompdeltatbl_init_val; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;         /* papdcompdeltatbl */
	tab.tbl_width = 32;     /* 32 bit wide */
	tab.tbl_offset = 0; /* tbl offset */

	papdcompdeltatbl_init_val = 0x80000; /* lut init val */

	for (j = 0; j < 256; j++) {
	    wlc_lcn40phy_write_table(pi, &tab);
	    tab.tbl_offset++;
	}
	return;
}

static void
wlc_lcn40phy_noise_init(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_txpower_recalc_target(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint32 rate_table[WL_NUM_RATES_CCK + WL_NUM_RATES_OFDM + WL_NUM_RATES_MCS_1STREAM];
	uint i, j;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	/* Adjust rate based power offset */
	for (i = j = 0; i < ARRAYSIZE(rate_table); i++, j++) {
		/* load 40_SS for 40 MHz rates */
		if (CHSPEC_IS40(pi->radio_chanspec) && j == WL_NUM_RATES_CCK + WL_NUM_RATES_OFDM) {
			j += (TXP_FIRST_MCS_40_SS - TXP_FIRST_MCS_20_SS);
		}
		rate_table[i] = (uint32)((int32)(-pi->tx_power_offset[j]));
		PHY_TMP((" Rate %d, offset %d\n", i, rate_table[i]));
	}
	if (!pi_lcn->lcnphy_uses_rate_offset_table) {
		/* Preset txPwrCtrltbl */
		tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;	/* 32 bit wide	*/
		tab.tbl_len = ARRAYSIZE(rate_table); /* # values   */
		tab.tbl_ptr = rate_table; /* ptr to buf */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcn40phy_write_table(pi, &tab);
	}
	/* Set new target power */
	wlc_lcn40phy_set_target_tx_pwr(pi, pi->tx_power_min);
	/* Should reset power index cache */
	wlc_lcn40phy_txpower_reset_npt(pi);
}

static void
wlc_lcn40phy_set_tx_gain_override(phy_info_t *pi, bool bEnable)
{
	uint16 bit = bEnable ? 1 : 0;

	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, txgainctrl_ovr, bit);

	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr, dacattctrl_ovr, bit);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride8, tx_vlin_ovr, bit);
}

static void
wlc_lcn40phy_set_tx_gain(phy_info_t *pi,  phy_txgains_t *target_gains)
{
	uint16 pa_gain = wlc_lcn40phy_get_pa_gain(pi);

	PHY_REG_MOD(pi, LCN40PHY, txgainctrlovrval0, txgainctrl_ovr_val0,
		(target_gains->gm_gain) | (target_gains->pga_gain << 8));

	PHY_REG_MOD(pi, LCN40PHY, txgainctrlovrval1, txgainctrl_ovr_val1,
		(target_gains->pad_gain) | (pa_gain << 8));

	wlc_lcn40phy_set_dac_gain(pi, target_gains->dac_gain);
	/* Enable gain overrides */
	wlc_lcn40phy_enable_tx_gain_override(pi);
}

static void
wlc_lcn40phy_set_pa_gain(phy_info_t *pi, uint16 gain)
{
	PHY_REG_MOD(pi, LCN40PHY, txgainctrlovrval1, pagain_ovr_val1, gain);
}


static void
wlc_lcn40phy_set_dac_gain(phy_info_t *pi, uint16 dac_gain)
{
	uint16 dac_ctrl;

	dac_ctrl = (phy_reg_read(pi, LCN40PHY_AfeDACCtrl) >> LCN40PHY_AfeDACCtrl_dac_ctrl_SHIFT);
	dac_ctrl = dac_ctrl & 0xc7f;
	dac_ctrl = dac_ctrl | (dac_gain << 7);
	PHY_REG_MOD(pi, LCN40PHY, AfeDACCtrl, dac_ctrl, dac_ctrl);
}

static uint16
wlc_lcn40phy_get_pa_gain(phy_info_t *pi)
{
	uint16 pa_gain;

	pa_gain = (phy_reg_read(pi, LCN40PHY_txgainctrlovrval1) &
		LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_MASK) >>
		LCN40PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT;

	return pa_gain;
}

void
wlc_lcn40phy_set_bbmult(phy_info_t *pi, uint8 m0)
{
	uint16 m0m1 = (uint16)m0 << 8;
	phytbl_info_t tab;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	tab.tbl_ptr = &m0m1; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;         /* iqloCaltbl      */
	tab.tbl_offset = 87; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcn40phy_write_table(pi, &tab);
}

static void
wlc_lcn40phy_filt_bw_set(phy_info_t *pi, uint16 bw)
{
	phy_reg_mod(pi, LCN40PHY_lpfbwlutreg1, LCN40PHY_lpfbwlutreg1_lpf_ofdm_tx_bw_MASK |
		LCN40PHY_lpfbwlutreg1_lpf_cck_tx_bw_MASK, bw | (bw << 3));
	PHY_REG_MOD(pi, LCN40PHY, lpfofdmhtlutreg, lpf_ofdm_tx_ht_bw, bw);
}

static void
wlc_lcn40phy_txpower_reset_npt(phy_info_t *pi)
{
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	pi_lcn->lcnphy_tssi_npt = LCN40PHY_TX_PWR_CTRL_START_NPT;
}

static void
wlc_lcn40phy_set_chanspec_tweaks(phy_info_t *pi, chanspec_t chanspec)
{
}

static void
wlc_lcn40phy_restore_calibration_results(phy_info_t *pi)
{
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#endif
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

#if defined(PHYCAL_CACHING) && defined(BCMDBG)
	/* Print the params to be restored */
	wlc_phy_cal_cache_dbg_lcnphy(ctx);
#endif
	/* restore tx iq cal results */
	wlc_lcn40phy_restore_txiqlo_calibration_results(pi, 0, 127, 0);

	/* restore PAPD cal results */
	if (pi_lcn->lcnphy_papd_4336_mode) {
		wlc_lcn40phy_restore_papd_calibration_results(pi);
	}

	/* restore rx iq cal results */
#if defined(PHYCAL_CACHING)
	wlc_lcnphy_set_rx_iq_comp(pi, cache->rxiqcal_coeff_a0,
		cache->rxiqcal_coeff_b0);
#else
	wlc_lcn40phy_set_rx_iq_comp(pi, pi_lcn->lcnphy_cal_results.rxiqcal_coeff_a0,
		pi_lcn->lcnphy_cal_results.rxiqcal_coeff_b0);
#endif
}

static void
wlc_lcn40phy_agc_reset(phy_info_t *pi)
{
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 0);
	phy_reg_or(pi, LCN40PHY_resetCtrl, 0x44);
	phy_reg_write(pi, LCN40PHY_resetCtrl, 0x80);
	PHY_REG_MOD(pi, LCN40PHY, agcControl4, c_agc_fsm_en, 1);
}

static void
wlc_lcn40phy_radio_2065_channel_tune(phy_info_t *pi, uint8 channel)
{
	bool rfpll_doubler = 0;
	uint32 c7, c10, c11 = 0, d15, d16, f16, d18_int, d18_frac;
	uint32 d20, d21, d30_q16, d40, e32, d33, e33, g30, g35, g40, den;
	uint32 d34, e34, e35, e36, e37, e38, d41, e40, e41;
	uint32 d42, e42, e43;
	uint8 c11_divfact;

	chan_info_2065_lcn40phy_t *chi = wlc_lcn40phy_find_channel(pi, channel);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(chi != NULL);
	/* Calculate various input frequencies */
	if (rfpll_doubler) {
		c7 = pi->xtalfreq << 1;
		or_radio_reg(pi, RADIO_2065_XTAL_CFG3, 1);
	} else {
		c7 = pi->xtalfreq;
		and_radio_reg(pi, RADIO_2065_XTAL_CFG3, ~1);
	}
	/* xtal freq in MHz, C10 in KHz */
	c10 = pi->xtalfreq / 1000;
	/* VCO freq */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		c11 = chi->freq * 9;
	}
#ifdef BAND5G
	else {
		c11 = chi->freq * 4;
		mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0x7, 0);
	}
#endif
	c11_divfact = 6;
	/* PLL_delayBeforeOpenLoop */
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xff, 5);
	/* PLL_enableTimeOut */
	d15 = (((c10 * 4) +  5000)/ 10000) - 1;
/* round here */
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xff00, d15 << 8);
	/* PLL_cal_ref_timeout */
	if ((c10 * 4/1000) % (d15 + 1))
		d16 = c10 * 4 / ((d15 + 1)*1000);
	else
		d16 = (c10 * 4 / ((d15 + 1)*1000)) - 1;
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xff, (uint16)d16);
	/* PLL_calSetCount */
	f16 = (((d16 + 1) * (d15 + 1) * 1000) << 10)/ c10;
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xfff0,
		((((f16 * c11 / (8 * c11_divfact)) + (2 << 9)) >> 10) -1) << 4);
	/* ref_value vars and wrtie */
	den = (c10/100) * 8 * c11_divfact;
	d18_int = (c11 * 10)/den;
	d18_frac = (c11 * 10)%den;
	d18_frac = ((d18_frac << 17) + (den >> 4)) / (den >> 3);

	write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1,
		(uint16)((d18_int << 4) | ((d18_frac >> 16) & 0xf)));
	write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, (uint16)(d18_frac & 0xffff));
	/* cal_caps_sel vars and write */
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xe0, 3 << 5);

	/* divider, integer bits */
	den = (c7/100000) * c11_divfact;
	d20 = (c11 * 10)/den;
	d21 = (c11 * 10)%den;
	d21 = ((d21 << 20) + (den >> 1))/ den;

	/* divider (wide base) */
	write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, (uint16)d21);
	write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, (d21 >> 16) + (d20 << 4));

	/* PLL_lf_r1 */
	e32 = ((PLL_2065_D32 - 850) + 160) / 320;
	/* PLL_lf_r2 */
	g30 = (e32 * 320) + 850;
	d33 = (g30 * PLL_2065_LF_R2) / PLL_2065_LF_R1;
	e33 = ((d33 - 850) + 160)/ 320;
	/* PLL_lf_r3 */
	d34 = g30 * PLL_2065_LF_R3 / PLL_2065_LF_R1;
	e34 = ((d34 - 850) + 160) / 320;
	/* PLL_lf_c1 */
	e35 = ((PLL_2065_D35 - 36) + 6) / 12;
	/* PLL_lf_c2 */
	/* set g35 [expr ($e35*12.0)+36]
	 * set d36 [expr $g35/15.0]
	* set e36 [expr round(($d36-2.4)/0.8)]
	 */
	g35 = (e35 * 12) + 36;
	e36 = ((g35 * 10 - 24 * 15) + 60) / (15 * 8);

	/* PLL_lf_c3 */
	/* set d37 [expr $g35*$c37/$c35]
	* set e37 [expr round(($d37-0.72)/0.24)]
	 */
	e37 = ((g35 * PLL_2065_LF_C3_X100 - 72 * PLL_2065_LF_C1) + (12 * PLL_2065_LF_C1)) /
		(24 * PLL_2065_LF_C1);
	/* PLL_lf_c4 */
	/* set d38 [expr ($g35*$c38)/$c35]
	* set e38 [expr round(($d38-0.72)/0.24)]
	 */
	e38 = ((g35 * PLL_2065_LF_C4_X100 - 72 * PLL_2065_LF_C1) + (12 * PLL_2065_LF_C1)) /
		(24 * PLL_2065_LF_C1);

	/* PLL_cp_current */
	d30_q16 = ((((PLL_2065_HIGH_END_KVCO_Q16 - PLL_2065_LOW_END_KVCO_Q16) /
		(PLL_2065_HIGH_END_VCO - PLL_2065_LOW_END_VCO)) *
		(c11 - (PLL_2065_LOW_END_VCO*c11_divfact)))/c11_divfact) +
	PLL_2065_LOW_END_KVCO_Q16;
	d40 = ((d20 * 41 * (PLL_2065_LOOP_BW_DESIRED/100) * (PLL_2065_LOOP_BW_DESIRED/100)) << 16);
	d40 = d40 / (d30_q16 * 4);
	d40 = (d40 * 628 * 628)/100000;
	if (d40 > 200)
		d41 = 1;
	else
		d41 = 0;
	e41 = d41;
	den = 12 * (d41 * 4 + 1);
	e40 = (d40 + (den >> 1)) / den - 4;

	g40 = 12*(e40+4)*((d41*4)+1);
	d42 = 5 * g40 * (c7/100000) * c11_divfact;

	if (d42 / c11 / 10 > 60)
		e43 = 1;
	else
		e43 = 0;

	e42 = d42 / (2 * (e43 + 1) * c11 * 10);
	if (d42 % (2 * (e43 + 1) * c11 * 10))
		e42 += 1;

	mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0x3f, (uint16)e32);
	mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0x3f00, e33 << 8);
	mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0x3f, (uint16)e34);
	mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0x1f, (uint16)e35);
	mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0x1f00, e36 << 8);
	mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0x1f, (uint16)e37);
	mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0x1f00, e38 << 8);
	mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0x1f, (uint16)e40);
	mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0x20, e41 << 5);
	mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0x1f0, e42 << 4);
	mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0x200, e43 << 9);

	wlc_2065_vco_cal(pi, TRUE);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		or_radio_reg(pi, RADIO_2065_OVR3, 0x400);
		mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0x3300,
			(chi->logen1 << 8) | (chi->logen2 << 12));
		mod_radio_reg(pi, RADIO_2065_LNA2G_TUNE, 0xfff,
			(chi->lna_freq1) | (chi->lna_freq2 << 4) | (chi->lna_tx << 8));
		mod_radio_reg(pi, RADIO_2065_TXMIX2G_CFG1, 0xf00, chi->txmix << 8);
		mod_radio_reg(pi, RADIO_2065_PGA2G_CFG2, 0x7, chi->pga);
		mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0x7, chi->pad);
	}
#ifdef BAND5G
	else
	{
		and_radio_reg(pi, RADIO_2065_OVR3, ~0x400);
		mod_radio_reg(pi, RADIO_2065_LOGEN5G_TUNE, 0xfff,
			(chi->logen1) | (chi->logen2 << 4) | (chi->logen3 << 8));
		mod_radio_reg(pi, RADIO_2065_LNA5G_TUNE, 0xfff,
			(chi->lna_freq1) | (chi->lna_freq2 << 4) | (chi->lna_tx << 8));
		mod_radio_reg(pi, RADIO_2065_TXMIX5G_CFG1, 0xf00, chi->txmix << 8);
		mod_radio_reg(pi, RADIO_2065_PGA5G_CFG2, 0xf000, chi->pga << 12);
		mod_radio_reg(pi, RADIO_2065_PAD5G_TUNE, 0xf, chi->pad);
	}
#endif
	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID || CHIPID(pi->sh->chip) == BCM43142_CHIP_ID) {
		switch (channel) {
			case 1:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0028);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x3105);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7101);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0xcccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x0169);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb4e);
			break;
			case 2:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7141);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x8ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016a);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb54);
			break;
			case 3:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7181);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x4ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016b);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb5a);
			break;
			case 4:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0073);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x71b1);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x0ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016c);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb60);
			break;
			case 5:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x71f1);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0xcccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016c);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb66);
			break;
			case 6:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7231);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x8ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb6c);
			break;
			case 7:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7271);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x4ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016e);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb72);
			break;
			case 8:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x72a1);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x0ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb78);
			break;
			case 9:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x72e1);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0xcccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x016f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb7e);
			break;
			case 10:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7321);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x8ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x0170);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb84);
			break;
			case 11:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0072);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7361);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x4ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x0171);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb8a);
			break;
			case 12:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0071);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7391);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x0ccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x0172);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb90);
			break;
			case 13:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0071);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x73d1);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0xcccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x0172);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0x6666);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xb96);
			break;
			default:
			mod_radio_reg(pi, RADIO_2065_RFPLL_R2_R1, 0xffff, 0x0d1d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_R3, 0xff, 0x000d);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C2_C1, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_C4_C3, 0xffff, 0x1f1f);
			mod_radio_reg(pi, RADIO_2065_RFPLL_KPD, 0xff, 0x0027);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CP_IDAC, 0xffff, 0x30f5);
			mod_radio_reg(pi, RADIO_2065_PAD2G_TUNE, 0xffff, 0x0071);
			mod_radio_reg(pi, RADIO_2065_LOGEN2G_CFG1, 0xffff, 0x01c7);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS2, 0xf, 0x5);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_DELAYS3, 0xffff, 0x0709);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 0xffff, 0x7461);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL0, 0x9999);
			write_radio_reg(pi, RADIO_2065_RFPLL_REF_VAL1, 0x0174);
			mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0xffff, 0x0283);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE0, 0xcccc);
			write_radio_reg(pi, RADIO_2065_RFPLL_WILD_BASE1, 0xba4);
			break;
		}
		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID) {
			mod_radio_reg(pi, RADIO_2065_PGA2G_CFG2, 0xffff, 0x0005);
			mod_radio_reg(pi, RADIO_2065_TXMIX2G_CFG1, 0xf00, 0x100);
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				mod_radio_reg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x806);
				mod_radio_reg(pi, RADIO_2065_PA2G_CFG2, 0xff, 0x66);
			} else {
				mod_radio_reg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x404);
				mod_radio_reg(pi, RADIO_2065_PA2G_CFG2, 0xff, 0x0);
			}
		} else {
			mod_radio_reg(pi, RADIO_2065_PA2G_INCAP, 0xf0f, 0x606);
			mod_radio_reg(pi, RADIO_2065_PGA2G_CFG2, 0x7, 0x7);
		}
	}
}

static void
wlc_2065_vco_cal(phy_info_t *pi, bool legacy)
{
	if (legacy) {
		mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0x10, 0);
	} else {
		mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0x10, 1 << 4);
	}

	mod_radio_reg(pi, RADIO_2065_OVR10, 0x8210, 0x8210);

	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_OVR_COUNT, 1, 1);

	mod_radio_reg(pi, RADIO_2065_OVR10, 0x10, 1 << 4);
	mod_radio_reg(pi, RADIO_2065_OVR10, 0x8000, 1 << 15);
	mod_radio_reg(pi, RADIO_2065_OVR10, 0x200, 1 << 9);

	mod_radio_reg(pi, RADIO_2065_RFPLL_CFG1, 0x40, 0);
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 0x3, 0);

	OSL_DELAY(20);
	mod_radio_reg(pi, RADIO_2065_RFPLL_CFG1, 0x40, 1 << 6);
	OSL_DELAY(20);
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 1, 1);
	OSL_DELAY(20);
	mod_radio_reg(pi, RADIO_2065_RFPLL_CAL_CFG1, 2, 1 << 1);
	OSL_DELAY(100);

}

static void
wlc_lcn40phy_decode_aa2g(phy_info_t *pi, uint8 val)
{
	switch (val)
	{
		case 1:
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x00 << LCN40PHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DefaultAntenna_MASK,
				0x00 << LCN40PHY_crsgainCtrl_DefaultAntenna_SHIFT);
			pi->sh->rx_antdiv = 0;
		break;
		case 2:
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x00 << LCN40PHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DefaultAntenna_MASK,
				0x01 << LCN40PHY_crsgainCtrl_DefaultAntenna_SHIFT);
			pi->sh->rx_antdiv = 1;
		break;
		case 3:
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DiversityChkEnable_MASK,
				0x01 << LCN40PHY_crsgainCtrl_DiversityChkEnable_SHIFT);
			phy_reg_mod(pi, LCN40PHY_crsgainCtrl,
				LCN40PHY_crsgainCtrl_DefaultAntenna_MASK,
				0x00 << LCN40PHY_crsgainCtrl_DefaultAntenna_SHIFT);
			pi->sh->rx_antdiv = 3;
		break;
		default:
			PHY_ERROR(("wl%d: %s: AA2G = %d is Unsupported\n",
				pi->sh->unit, __FUNCTION__, val));
			ASSERT(0);

		break;

	}
	return;
}
/* periodic cal does tx iqlo cal, rx iq cal, (tempcompensated txpwrctrl for P200 4313A0 board) */
static void
wlc_lcn40phy_periodic_cal(phy_info_t *pi)
{
	bool suspend, full_cal;
	phy_txcalgains_t txgains;
	uint16 SAVE_pwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	int8 indx;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#endif

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (NORADIO_ENAB(pi->pubpi))
		return;

#if defined(PHYCAL_CACHING)
	ctx->cal_info.last_cal_time = pi->sh->now;
	ctx->cal_info.last_cal_temp = pi_lcn->lcnphy_rawtempsense;
	full_cal = !(ctx->valid);
#else
	pi->phy_lastcal = pi->sh->now;
	full_cal = (pi_lcn->lcnphy_full_cal_channel != CHSPEC_CHANNEL(pi->radio_chanspec));
	pi_lcn->lcnphy_full_cal_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#endif

	pi->phy_forcecal = FALSE;
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		/* Set non-zero duration for CTS-to-self */
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	wlc_lcn40phy_deaf_mode(pi, TRUE);

	wlc_btcx_override_enable(pi);

	wlc_lcn40phy_txpwrtbl_iqlo_cal(pi);

	if (LCN40REV_IS(pi->pubpi.phy_rev, 2) || LCN40REV_IS(pi->pubpi.phy_rev, 4))
		wlc_lcn40phy_rx_iq_cal(pi, NULL, 0, TRUE, FALSE, 1, 20);

	/* PAPD cal */
	if (!pi_lcn->ePA) {
		indx = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

		if (CHSPEC_IS2G(pi->radio_chanspec))
			txgains.index = 65;
		else
			txgains.index = 55;

		txgains.useindex = 1;

		if (CHSPEC_IS2G(pi->radio_chanspec) && pi_lcn->pacalidx_2g)
			txgains.index = pi_lcn->pacalidx_2g;
		else if (CHSPEC_IS5G(pi->radio_chanspec) && pi_lcn->pacalidx_5g)
			txgains.index = pi_lcn->pacalidx_5g;

		wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, &txgains, 0, 0, 0, 0, 219, 1);
		wlc_lcn40phy_set_tx_pwr_by_index(pi, indx);
	}
#if IDLE_TSSI_PER_CAL_EN
	/* Idle TSSI Estimate */
	wlc_lcn40phy_idle_tssi_est(pi);
#endif /* #if IDLE_TSSI_PER_CAL_EN */
	/* Convert tssi to power LUT */
	wlc_lcn40phy_set_estPwrLUT(pi, 0);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
			wlc_lcn40phy_set_estPwrLUT(pi, 1);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */

	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_pwrctrl);
	wlc_lcn40phy_deaf_mode(pi, FALSE);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

#if defined(PHYCAL_CACHING)
	/* Cache the power index used for this channel */
	cache->lcnphy_gain_index_at_last_cal = wlc_lcnphy_get_current_tx_pwr_idx(pi);
	/* Already cached the Tx, Rx IQ and PAPD Cal results */
	ctx->valid = TRUE;
#endif /* PHYCAL_CACHING */
}

/* Run iqlo cal and populate iqlo portion of tx power control table */
static void
wlc_lcn40phy_txpwrtbl_iqlo_cal(phy_info_t *pi)
{

	phy_txgains_t old_gains;
	uint8 save_bb_mult;
	uint16 a, b, didq, save_bbmult0, save_pa_gain = 0;
	uint idx, SAVE_txpwrindex = 0xFF;
	uint32 val;
	uint16 SAVE_txpwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	phytbl_info_t tab;
	uint8 ei0 = 0, eq0 = 0, fi0 = 0, fq0 = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	phy_info_lcn40phy_t *pi_lcn40 = pi->u.pi_lcn40phy;

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#endif
	uint8 index;

	wlc_lcn40phy_get_tx_gain(pi, &old_gains);
	save_pa_gain = wlc_lcn40phy_get_pa_gain(pi);

	/* Store state */
	save_bb_mult = wlc_lcn40phy_get_bbmult(pi);

	if (SAVE_txpwrctrl == LCN40PHY_TX_PWR_CTRL_OFF)
		SAVE_txpwrindex = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID || CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
		wlc_lcn40phy_set_tx_pwr_by_index(pi, 60);
	else {
		if (CHSPEC_IS5G(pi->radio_chanspec))
			wlc_lcn40phy_set_tx_pwr_by_index(pi, 60);
		else
			wlc_lcn40phy_set_tx_pwr_by_index(pi, 20);
	}
	save_bbmult0 = phy_reg_read(pi, LCN40PHY_bbmult0);
	phy_reg_write(pi, LCN40PHY_bbmult0, 0x1FF);
	/* load iir filter with high gain */
	wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, 10);
	wlc_lcn40phy_tx_iqlo_cal(pi, NULL, (pi_lcn->lcnphy_recal ?
		CAL_RECAL : CAL_FULL), FALSE);

	wlc_lcn40phy_get_radio_loft(pi, &ei0, &eq0, &fi0, &fq0);
	/* Get calibration results */
	wlc_lcn40phy_get_tx_iqcc(pi, &a, &b);
	PHY_INFORM(("TXIQCal: %d %d\n", a, b));

	didq = wlc_lcn40phy_get_tx_locc(pi);

	/* Populate tx power control table with coeffs */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &val; /* ptr to buf */

	/* Per rate power offset */
	tab.tbl_len = 1; /* # values   */

	for (idx = 0; idx < 128; idx++) {
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + idx;
		/* iq */
		wlc_lcn40phy_read_table(pi, &tab);
		val = (val & 0xfff00000) |
			((uint32)(a & 0x3FF) << 10) | (b & 0x3ff);
		wlc_lcn40phy_write_table(pi, &tab);

		/* loft */
		val = didq;
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + idx;
		wlc_lcn40phy_write_table(pi, &tab);
	}

	/* Save Cal Results */
	index = 0;
#if defined(PHYCAL_CACHING)
	cache->txiqlocal_a[index] = a;
	cache->txiqlocal_b[index] = b;
	cache->txiqlocal_didq[index] = didq;
	cache->txiqlocal_ei0 = ei0;
	cache->txiqlocal_eq0 = eq0;
	cache->txiqlocal_fi0 = fi0;
	cache->txiqlocal_fq0 = fq0;
#else
	pi_lcn->lcnphy_cal_results.txiqlocal_a[index] = a;
	pi_lcn->lcnphy_cal_results.txiqlocal_b[index] = b;
	pi_lcn->lcnphy_cal_results.txiqlocal_didq[index] = didq;
	pi_lcn->lcnphy_cal_results.txiqlocal_ei0 = ei0;
	pi_lcn->lcnphy_cal_results.txiqlocal_eq0 = eq0;
	pi_lcn->lcnphy_cal_results.txiqlocal_fi0 = fi0;
	pi_lcn->lcnphy_cal_results.txiqlocal_fq0 = fq0;
#endif /* PHYCAL_CACHING */

	/* Restore state */
	wlc_lcn40phy_set_bbmult(pi, save_bb_mult);
	wlc_lcn40phy_set_pa_gain(pi, save_pa_gain);
	wlc_lcn40phy_set_tx_gain(pi, &old_gains);

	if (SAVE_txpwrctrl != LCN40PHY_TX_PWR_CTRL_OFF)
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	else
		wlc_lcn40phy_set_tx_pwr_by_index(pi, SAVE_txpwrindex);
	phy_reg_write(pi, LCN40PHY_bbmult0, save_bbmult0);
	/* restore default iir filter */
	wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, pi_lcn40->tx_iir_filter_type_ofdm);
}
/*
* RX IQ Calibration
*/
static bool
wlc_lcn40phy_rx_iq_cal(phy_info_t *pi, const phy_rx_iqcomp_t *iqcomp, int iqcomp_sz,
	bool tx_switch, bool rx_switch, int module, int tx_gain_idx)
{
	phy_txgains_t old_gains;
	uint16 tx_pwr_ctrl, save_bbmult0, tone_amplitude = 200;
	uint8 tx_gain_index_old = 0, save_bbmult;
	bool result = FALSE, tx_gain_override_old = FALSE;
	uint16 i;
	int tia_gain, biq1_gain;
	uint16 values_to_save[ARRAYSIZE(rxiq_cal_rf_reg)];
	uint16 values_to_save_phy[ARRAYSIZE(rxiq_cal_phy_reg)];
	int16 *ptr;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	bool set_gain = FALSE, papd_en = wlc_lcn40phy_is_papd_block_enable(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if ((ptr = MALLOC(pi->sh->osh, sizeof(int16) * 131)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

	if (pi_lcn->ePA)
		wlc_lcn40phy_epa_pd(pi, 1);

	if (module == 2) {
		ASSERT(iqcomp_sz);

		while (iqcomp_sz--) {
			if (iqcomp[iqcomp_sz].chan == CHSPEC_CHANNEL(pi->radio_chanspec)) {
				/* Apply new coeffs */
				wlc_lcn40phy_set_rx_iq_comp(pi, (uint16)iqcomp[iqcomp_sz].a,
					(uint16)iqcomp[iqcomp_sz].b);
				result = TRUE;
				break;
			}
		}
		ASSERT(result);
		goto cal_done;
	}
	/* module : 1 = loopback */
	if (module == 1) {

		wlc_btcx_override_enable(pi);
		/* turn off tx power control */
		tx_pwr_ctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
		wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
		/* turn off papd */
		wlc_lcn40phy_papd_block_enable(pi, FALSE);

		/* save rf register states */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_rf_reg); i++) {
			values_to_save[i] = read_radio_reg(pi, rxiq_cal_rf_reg[i]);
		}
		for (i = 0; i < ARRAYSIZE(rxiq_cal_phy_reg); i++) {
			values_to_save_phy[i] = phy_reg_read(pi, rxiq_cal_phy_reg[i]);
		}
		/* disable aux path
		* set reg(RF_auxpga_cfg1.auxpga_bias_ctrl) 0
		* set reg(RF_auxpga_cfg1.auxpga_vcm_ctrl) 1
		* set reg(RF_OVR1.ovr_afe_auxpga_pu)  1
		* set reg(RF_auxpga_cfg1.auxpga_pu) 0
		*/
		mod_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, 0x3031, 0x10);
		or_radio_reg(pi, RADIO_2065_OVR1, 0x8000);
		/* loft comp , iqmm comp enable */
		phy_reg_or(pi, LCN40PHY_Core1TxControl, 0x0015);
		/* Save old tx gain settings */
		tx_gain_override_old = wlc_lcn40phy_tx_gain_override_enabled(pi);
		if (tx_gain_override_old) {
			wlc_lcn40phy_get_tx_gain(pi, &old_gains);
			tx_gain_index_old = pi_lcn->lcnphy_current_index;
		}
		/* Apply new tx gain */
		wlc_lcn40phy_set_tx_pwr_by_index(pi, tx_gain_idx);

		/* Force DAC/ADC on */
		phy_reg_mod(pi, LCN40PHY_AfeCtrlOvr1,
			(LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_MASK),
			((1 << LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1_dac_pu_ovr_SHIFT)));
		phy_reg_mod(pi, LCN40PHY_AfeCtrlOvr1Val,
			(LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_dac_pu_ovr_val_MASK),
			((31 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1Val_dac_pu_ovr_val_SHIFT)));
		/* TR switch */
		wlc_lcn40phy_set_trsw_override(pi, tx_switch, rx_switch);

		/* AMS configuration */
		mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0xcec0, 0x8a00);
		mod_radio_reg(pi, RADIO_2065_OVR7, 0xd4, 0x14);
		phy_reg_mod(pi, LCN40PHY_rfoverride7,
			(LCN40PHY_rfoverride7_lpf_sel_rx_buffer_ovr_MASK |
			LCN40PHY_rfoverride7_lpf_sel_byp_rxlpf_ovr_MASK),
			((1 << LCN40PHY_rfoverride7_lpf_sel_rx_buffer_ovr_SHIFT) |
			(1 << LCN40PHY_rfoverride7_lpf_sel_byp_rxlpf_ovr_SHIFT)));
		phy_reg_mod(pi, LCN40PHY_rfoverride7val,
			(LCN40PHY_rfoverride7val_lpf_sel_rx_buffer_ovr_val_MASK |
			LCN40PHY_rfoverride7val_lpf_sel_byp_rxlpf_ovr_val_MASK),
			((0 << LCN40PHY_rfoverride7val_lpf_sel_rx_buffer_ovr_val_SHIFT) |
			(2 << LCN40PHY_rfoverride7val_lpf_sel_byp_rxlpf_ovr_val_SHIFT)));

		mod_radio_reg(pi, RADIO_2065_LPF_CFG2, 0x7f, 0x1b);
		mod_radio_reg(pi, RADIO_2065_OVR6, 0x43c2, 0x43c2);
		mod_radio_reg(pi, RADIO_2065_OVR5, 0x12, 0x12);

		if (CHSPEC_IS20(pi->radio_chanspec)) {
			mod_radio_reg(pi, RADIO_2065_LPF_RESP_BQ1, 0x7, 0);
			mod_radio_reg(pi, RADIO_2065_LPF_RESP_BQ2, 0x7, 0);
		} else {
			mod_radio_reg(pi, RADIO_2065_LPF_RESP_BQ1, 0x7, 2);
			mod_radio_reg(pi, RADIO_2065_LPF_RESP_BQ2, 0x7, 2);
		}

		write_radio_reg(pi, RADIO_2065_LPF_BIAS0, 2);

		/* Enable loopback from PAD to MIXER */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			or_radio_reg(pi, RADIO_2065_OVR12, 0x10);
			or_radio_reg(pi, RADIO_2065_OVR11, 0x8000);
			or_radio_reg(pi, RADIO_2065_OVR2, 0x20);
			mod_radio_reg(pi, RADIO_2065_TXRX2G_CAL, 0x303, 0x103);

			phy_reg_mod(pi, LCN40PHY_RFOverrideVal0,
				(LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK),
				((1 << LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_SHIFT) |
				(1 << LCN40PHY_RFOverrideVal0_internalrftxpu_ovr_val_SHIFT)));
			phy_reg_mod(pi, LCN40PHY_RFOverride0,
				(LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK |
				LCN40PHY_RFOverride0_internalrftxpu_ovr_MASK),
				((1 << LCN40PHY_RFOverride0_internalrfrxpu_ovr_SHIFT) |
				(1 << LCN40PHY_RFOverride0_internalrftxpu_ovr_SHIFT)));
		} else {
			or_radio_reg(pi, RADIO_2065_OVR13, 0x2000);
			or_radio_reg(pi, RADIO_2065_OVR11, 0x4000);
			or_radio_reg(pi, RADIO_2065_OVR2, 0x10);
			mod_radio_reg(pi, RADIO_2065_TXRX5G_CAL, 0xf03, 0x303);
			/* Force off the Aband-ePA */

			phy_reg_mod(pi, LCN40PHY_RFOverride0,
				LCN40PHY_RFOverride0_amode_tx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_amode_rx_pu_ovr_MASK |
				LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK,
				(1 << LCN40PHY_RFOverride0_amode_tx_pu_ovr_SHIFT) |
				(1 << LCN40PHY_RFOverride0_amode_rx_pu_ovr_SHIFT) |
				(1 << LCN40PHY_RFOverride0_internalrfrxpu_ovr_SHIFT));
			phy_reg_mod(pi, LCN40PHY_RFOverrideVal0,
				LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_amode_rx_pu_ovr_val_MASK |
				LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_MASK,
				(1 << LCN40PHY_RFOverrideVal0_amode_tx_pu_ovr_val_SHIFT) |
				(1 << LCN40PHY_RFOverrideVal0_amode_rx_pu_ovr_val_SHIFT) |
				(1 << LCN40PHY_RFOverrideVal0_internalrfrxpu_ovr_val_SHIFT));
		}
		/* shut off LNA's */
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, slna_pu_ovr_val, 0);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_pu_ovr, 1);
		PHY_REG_MOD(pi,  LCN40PHY, rfoverride8val, lna2_pu_ovr_val, 0);
		PHY_REG_MOD(pi,  LCN40PHY, rfoverride8, lna2_pu_ovr, 1);
		phy_reg_mod(pi, LCN40PHY_rfoverride4val,
			LCN40PHY_rfoverride4val_lpf_dc1_pwrup_ovr_val_MASK |
			LCN40PHY_rfoverride4val_lpf_byp_dc_ovr_val_MASK,
			(1 << LCN40PHY_rfoverride4val_lpf_dc1_pwrup_ovr_val_SHIFT) |
			(0 << LCN40PHY_rfoverride4val_lpf_byp_dc_ovr_val_SHIFT));

		phy_reg_mod(pi, LCN40PHY_rfoverride4,
			LCN40PHY_rfoverride4_lpf_dc1_pwrup_ovr_MASK |
			LCN40PHY_rfoverride4_lpf_byp_dc_ovr_MASK,
			(1 << LCN40PHY_rfoverride4_lpf_dc1_pwrup_ovr_SHIFT) |
			(1 << LCN40PHY_rfoverride4_lpf_byp_dc_ovr_SHIFT));
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 1);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1);
		PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1);
		/* make sure tx/rx is powered up */
		phy_reg_or(pi,  LCN40PHY_rfoverride8val,
			LCN40PHY_rfoverride8val_fast_nap_bias_pu_ovr_val_MASK |
			LCN40PHY_rfoverride8val_logen_rx_pu_ovr_val_MASK);

		phy_reg_or(pi,  LCN40PHY_rfoverride8,
			LCN40PHY_rfoverride8_fast_nap_bias_pu_ovr_MASK |
			LCN40PHY_rfoverride8_logen_rx_pu_ovr_MASK);

		save_bbmult = wlc_lcn40phy_get_bbmult(pi);
		wlc_lcn40phy_set_bbmult(pi, 200);
		save_bbmult0 = phy_reg_read(pi, LCN40PHY_bbmult0);
		phy_reg_write(pi, LCN40PHY_bbmult0, 0x1FF);

		/* Run calibration */
		phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xffff);
		phy_reg_or(pi, LCN40PHY_sslpnRxFeClkEnCtrl, 0x3);

		PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 1);
		/* adjust rx power */
		#define k_lcnphy_rx_iq_est_n_samps 1024

		tia_gain = 5;
		while ((tia_gain >= 0) && !set_gain) {

			biq1_gain = 2;
			while ((biq1_gain >= 0) && !set_gain) {
				phy_iq_est_t iq_est_h;
				phy_iq_est_t iq_est_l;
				uint32 i_thresh_l, i_thresh_h;
				uint32 q_thresh_l, q_thresh_h;

				wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 0, 0, 0, 0, 0,
					(uint16)tia_gain, (uint16)biq1_gain, 0, 0, 0);
				/* PHY thinks LPF is in tx mode, need radio override */
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_ofdm_tx_biq2_gain, 0);
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_ofdm_tx_biq1_gain,
					(uint16)biq1_gain);
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_cck_tx_biq2_gain, 0);
				PHY_REG_MOD(pi, LCN40PHY, lpfgainlutreg, lpf_cck_tx_biq1_gain,
					(uint16)biq1_gain);

				wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);

				wlc_lcn40phy_start_tx_tone(pi, (2000 * 1000),
					(tone_amplitude>>1), 0);
				/* PA override */
				phy_reg_mod(pi, LCN40PHY_rfoverride4,
					LCN40PHY_rfoverride4_papu_ovr_MASK,
					1 << LCN40PHY_rfoverride4_papu_ovr_SHIFT);
				phy_reg_mod(pi, LCN40PHY_rfoverride4val,
					LCN40PHY_rfoverride4val_papu_ovr_val_MASK,
					0 << LCN40PHY_rfoverride4val_papu_ovr_val_SHIFT);

				if (!wlc_lcn40phy_rx_iq_est(pi,
					k_lcnphy_rx_iq_est_n_samps, 32, &iq_est_l))
				{
					printf("\n Error getting low iq est\n");
					break;
				}
				wlc_lcn40phy_stop_tx_tone(pi);
				wlc_lcn40phy_start_tx_tone(pi, (2000 * 1000),
					tone_amplitude, 0);
				/* PA override : tx tone will force PA on */
				phy_reg_mod(pi, LCN40PHY_rfoverride4,
					LCN40PHY_rfoverride4_papu_ovr_MASK,
					1 << LCN40PHY_rfoverride4_papu_ovr_SHIFT);
				phy_reg_mod(pi, LCN40PHY_rfoverride4val,
					LCN40PHY_rfoverride4val_papu_ovr_val_MASK,
					0 << LCN40PHY_rfoverride4val_papu_ovr_val_SHIFT);

				if (!wlc_lcn40phy_rx_iq_est(pi,
					k_lcnphy_rx_iq_est_n_samps, 32, &iq_est_h))
				{
					printf("\n Error getting high iq est\n");
					break;
				}

				i_thresh_l = (iq_est_l.i_pwr << 1) + iq_est_l.i_pwr;
				i_thresh_h = (iq_est_l.i_pwr << 2) + iq_est_l.i_pwr;

				q_thresh_l = (iq_est_l.q_pwr << 1) + iq_est_l.q_pwr;
				q_thresh_h = (iq_est_l.q_pwr << 2) + iq_est_l.q_pwr;


				/* Check that rx power drops by 6dB after pwr of */
				/* dac input drops by 6dB */
				if ((iq_est_h.i_pwr > i_thresh_l) &&
					(iq_est_h.i_pwr < i_thresh_h) &&
					(iq_est_h.q_pwr > q_thresh_l) &&
					(iq_est_h.q_pwr < q_thresh_h)) {
					set_gain = TRUE;

					PHY_INFORM(("wl%d: %s gains are "
								"tia=%d biq1=%d biq2=0",
								pi->sh->unit, __FUNCTION__,
								tia_gain, biq1_gain));
					break;
				}
				if ((iq_est_h.i_pwr < i_thresh_l) ||
					(iq_est_h.q_pwr < q_thresh_l))
					tone_amplitude = 100;

				biq1_gain--;
			}
			tia_gain--;
		}

		PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 0);

		if (set_gain)
			result = wlc_lcn40phy_calc_rx_iq_comp(pi, 0x4000);
		else {
				wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 0, 0, 0, 0, 0,
					5, 4, 0, 0, 0);
				phy_reg_write(pi, LCN40PHY_lpfgainlutreg, 0);
				wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
				wlc_lcn40phy_calc_rx_iq_comp(pi, 0x4000);
				result = FALSE;
		}

		/* clean up */
		wlc_lcn40phy_stop_tx_tone(pi);
		/* restore papd state */
		if (papd_en)
			wlc_lcn40phy_papd_block_enable(pi, TRUE);
		/* restore Core1TxControl */
		/* Restore PHY Registers */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_phy_reg); i++) {
			phy_reg_write(pi, rxiq_cal_phy_reg[i], values_to_save_phy[i]);
		}

		/* Restore RF Registers */
		for (i = 0; i < ARRAYSIZE(rxiq_cal_rf_reg); i++) {
			write_radio_reg(pi, rxiq_cal_rf_reg[i], values_to_save[i]);
		}
		/* Restore Tx gain */
		if (tx_gain_override_old) {
			wlc_lcn40phy_set_tx_pwr_by_index(pi, tx_gain_index_old);
		} else
			wlc_lcn40phy_disable_tx_gain_override(pi);

		wlc_lcn40phy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl);

		/* Clear various overrides */
		wlc_lcn40phy_rx_gain_override_enable(pi, FALSE);

		/* restore bbmult */
		phy_reg_write(pi, LCN40PHY_bbmult0, save_bbmult0);
	    wlc_lcn40phy_set_bbmult(pi, save_bbmult);
	}

cal_done:
	if (pi_lcn->ePA)
		wlc_lcn40phy_epa_pd(pi, 0);

	MFREE(pi->sh->osh, ptr, 131 * sizeof(int16));
	PHY_INFORM(("wl%d: %s: Rx IQ cal complete, coeffs: A0: %d, B0: %d\n",
		pi->sh->unit, __FUNCTION__,
		(int16)((phy_reg_read(pi, LCN40PHY_RxCompcoeffa0) & LCN40PHY_RxCompcoeffa0_a0_MASK)
		>> LCN40PHY_RxCompcoeffa0_a0_SHIFT),
		(int16)((phy_reg_read(pi, LCN40PHY_RxCompcoeffb0) & LCN40PHY_RxCompcoeffb0_b0_MASK)
		>> LCN40PHY_RxCompcoeffb0_b0_SHIFT)));
	return result;
}

static void
wlc_lcn40phy_amuxsel_get(phy_info_t *pi, uint16 *save_ovr, uint16 *save_ovr_val)
{
	/* AMUX SEL logic */
	*save_ovr =
		PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride);
	*save_ovr_val =
		PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal);
}

static void
wlc_lcn40phy_amuxsel_set(phy_info_t *pi, uint16 save_ovr, uint16 save_ovr_val)
{
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, save_ovr);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, save_ovr_val);

}

static void
wlc_lcn40phy_papd_block_enable(phy_info_t *pi, bool enable)
{
	/* Disable filters, turn off PAPD */
	PHY_REG_MOD(pi, LCN40PHY, papd_control, papdCompEn, enable);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, papdTxClkEn, enable);
	PHY_REG_MOD(pi, LCN40PHY, txfefilterconfig, cck_papden, enable);
	PHY_REG_MOD(pi, LCN40PHY, txfefilterconfig, ofdm_papden, enable);
	PHY_REG_MOD(pi, LCN40PHY, txfefilterconfig, ht_papden, enable);
}

static bool
wlc_lcn40phy_is_papd_block_enable(phy_info_t *pi)
{
	/* Disable filters, turn off PAPD */
	return (bool)(phy_reg_read(pi, LCN40PHY_papd_control) &
		LCN40PHY_papd_control_papdCompEn_MASK);
}

static void
wlc_lcn40phy_papd_cal(
	phy_info_t *pi,
	phy_papd_cal_type_t cal_type,
	phy_txcalgains_t *txgains,
	bool frcRxGnCtrl,
	bool txGnCtrl,
	bool samplecapture,
	bool papd_dbg_mode,
	uint8 num_symbols,
	uint8 init_papd_lut)
{
	uint8 bb_mult_old;
	uint16 AphyControlAddr_old, lcnCtrl3_old;
	uint16 Core1TxControl_old;
	uint16 save_amuxSelPortOverride, save_amuxSelPortOverrideVal;
	/* Save rx_gain_override_enable setting individually */
	uint16 trsw_rx_pu_ovr_old;
	uint16 ext_lna_gain_ovr_old;
	uint16 slna_byp_ovr_old;
	uint16 slna_rout_ctrl_ovr_old;
	uint16 slna_gain_ctrl_ovr_old;
	uint16 rxrf_lna2_gain_ovr_old;
	uint16 rxrf_lna2_rout_ovr_old;
	uint16 rxrf_tia_gain_ovr_old;
	uint16 lpf_bq1_gain_ovr_old;
	uint16 lpf_bq2_gain_ovr_old;
	/* uint16 SAVE_digi_gain_ovr_old; */
	uint16 SAVE_RFOverride0;
	uint16 SAVE_RFOverrideVal0;
	uint16 SAVE_RF_auxpga_cfg1;
	uint16 SAVE_RF_lpf_gain;
	uint16 SAVE_AfeCtrlOvr1;
	uint16 SAVE_AfeCtrlOvr1Val;
	uint16 SAVE_rfoverride2;
	uint16 SAVE_rfoverride2val;
	uint16 SAVE_RF_pa2g_cfg1;
	uint16 SAVE_RF_rxrf2g_cfg1;
	uint16 SAVE_RF_OVR1;
	uint16 SAVE_RF_OVR3;
	uint16 SAVE_RF_OVR4;
	uint16 SAVE_RF_OVR5;
	uint16 SAVE_RF_OVR6;
	uint16 SAVE_RF_OVR7;
	uint16 SAVE_RF_OVR11;
	uint16 SAVE_RF_OVR12;
	uint16 SAVE_RF_logen2g_txdiv;
	uint16 SAVE_RF_logen2g_rxdiv;
	uint16 SAVE_RF_lna2g_cfg2;
	uint16 SAVE_RF_lna2g_cfg1;
	uint16 SAVE_RF_lpf_cfg1;
	uint16 SAVE_RF_rxmix2g_cfg1;
	uint16 SAVE_RF_rx_reg_backup_1;
	uint16 SAVE_RF_tia_cfg1;
	uint16 SAVE_RF_top_spare1;
	uint16 SAVE_RF_txrx2g_cal;
	uint16 digi_gain_ovr_old;
	uint32 rxGnIdx;
	phytbl_info_t tab;
	uint16 sslpnCalibClkEnCtrl_old;
	uint32 tmpVar;
	uint32 refTxAnGn;
	uint16 lpfbwlut0, lpfbwlut1;
	uint8 CurTxGain;
	phy_txgains_t old_gains;
	uint16 lpf_ofdm_tx_bw;
	uint8 papd_peak_curr_mode = 0;
	int16 maxUpdtIdx = 1, minUpdtIdx = 1, j;

	bool suspend = FALSE;

	uint16 save_filtcoeffs[LCN40PHY_NUM_DIG_FILT_COEFFS+1];

	/* bool papd_lastidx_search_mode = TRUE; */
	uint32 min_papd_lut_val, max_papd_lut_val;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	ASSERT((cal_type == PHY_PAPD_CAL_CW));
	/* txgains should never be null */
	ASSERT(txgains != NULL);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_PAPD(("Running papd cal, channel: %d cal type: %d\n",
		CHSPEC_CHANNEL(pi->radio_chanspec),
		cal_type));

	bb_mult_old = wlc_lcn40phy_get_bbmult(pi); /* PAPD cal can modify this value */

	PHY_REG_MOD(pi, LCN40PHY, txfefilterctrl, txfefilterconfig_en, 0);
	/* AMUX SEL logic */
	wlc_lcn40phy_amuxsel_get(pi, &save_amuxSelPortOverride,
		&save_amuxSelPortOverrideVal);
	wlc_lcn40phy_amuxsel_set(pi, 1, 2);

	wlc_lcn40phy_papd_block_enable(pi, FALSE);

	/* Save Digital Filter and set to OFDM Coeffs */
	wlc_lcn40phy_save_restore_dig_filt_state(pi, TRUE, save_filtcoeffs);

	/*  In 40MHz bw mode, RTL picks the 20MHz OFDM filter */
	/*	which has 10MHz corner frequency and */
	/*  is used for 20-in-40 mode during sample play. */
	/*	load filter coefficients with 20MHz corner freq. into the 20MHz OFDM filter. */
	wlc_lcn40phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, 0);


	if (!txGnCtrl) {
		/* Disable CRS/Rx, MAC/Tx and BT */
		wlc_lcn40phy_deaf_mode(pi, TRUE);
		/* suspend the mac if it is not already suspended */
		suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
		if (!suspend) {
			/* Set non-zero duration for CTS-to-self */
			wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION, 10000);
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}
	}

	trsw_rx_pu_ovr_old = PHY_REG_READ(pi, LCN40PHY, RFOverride0, trsw_rx_pu_ovr);
	ext_lna_gain_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride2, ext_lna_gain_ovr);
	slna_byp_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride2, slna_byp_ovr);
	slna_rout_ctrl_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride3, slna_rout_ctrl_ovr);
	slna_gain_ctrl_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride2, slna_gain_ctrl_ovr);
	rxrf_lna2_gain_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride5, rxrf_lna2_gain_ovr);
	rxrf_lna2_rout_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride5, rxrf_lna2_rout_ovr);
	rxrf_tia_gain_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride5, rxrf_tia_gain_ovr);
	lpf_bq1_gain_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride6, lpf_bq1_gain_ovr);
	lpf_bq2_gain_ovr_old = PHY_REG_READ(pi, LCN40PHY, rfoverride6, lpf_bq2_gain_ovr);
	digi_gain_ovr_old  = PHY_REG_READ(pi, LCN40PHY, radioCtrl, digi_gain_ovr);
	/* Set Rx gain override */
	wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);

	wlc_lcn40phy_tx_pu(pi, TRUE);
	/* This register should be toggled for papd to work after enabling dac power up */
	PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 1);
	PHY_REG_MOD(pi, LCN40PHY, AphyControlAddr, phyloopbackEn, 0);

	wlc_lcn40phy_rx_pu(pi, TRUE);

	/* Save lcnphy filt bw */
	lpfbwlut0 = phy_reg_read(pi, LCN40PHY_lpfbwlutreg0);
	lpfbwlut1 = phy_reg_read(pi, LCN40PHY_lpfbwlutreg1);

	/* Widen tx filter */
	/* set filter bandwidth in lpphy_rev0_rf_init, so it's common b/n cal and packets tx */
	/* tones use cck setting, we want to cal with ofdm filter setting */

	lpf_ofdm_tx_bw = PHY_REG_READ(pi, LCN40PHY, lpfbwlutreg1, lpf_ofdm_tx_bw);
	PHY_REG_MOD(pi, LCN40PHY, lpfbwlutreg1, lpf_cck_tx_bw, lpf_ofdm_tx_bw);

	CurTxGain = pi_lcn->lcnphy_current_index;
	wlc_lcn40phy_get_tx_gain(pi, &old_gains);

	/* Set tx gain */
	if (txgains->useindex) {
		wlc_lcn40phy_set_tx_pwr_by_index(pi, txgains->index);
		CurTxGain = txgains->index;
		PHY_PAPD(("txgainIndex = %d\n", CurTxGain));
		PHY_PAPD(("papd_analog_gain_ovr_val = %d\n",
			PHY_REG_READ(pi, LCN40PHY, papd_analog_gain_ovr_val,
			papd_analog_gain_ovr_val)));
	} else {
		wlc_lcn40phy_set_tx_gain(pi, &txgains->gains);
	}

	AphyControlAddr_old = phy_reg_read(pi, LCN40PHY_AphyControlAddr);
	Core1TxControl_old = phy_reg_read(pi, LCN40PHY_Core1TxControl);
	lcnCtrl3_old = phy_reg_read(pi, LCN40PHY_sslpnCtrl3);
	sslpnCalibClkEnCtrl_old = phy_reg_read(pi, LCN40PHY_sslpnCalibClkEnCtrl);

	/* loft comp , iqmm comp enable */
	phy_reg_or(pi, LCN40PHY_Core1TxControl, 0x0015);

	/* we need to bring SPB out of standby before using it */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCtrl3, sram_stby, 0);

	/* enable clk (including forceTxfiltClkOn) to SPB, PAPD blks and cal */
	phy_reg_or(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x008f);

	/* Set PAPD reference analog gain */
	tab.tbl_ptr = &refTxAnGn; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;

	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + txgains->index; /* tbl offset */
	tab.tbl_width = 32;     /* 32 bit wide */
	wlc_lcnphy_read_table(pi, &tab);
	/* only the lower 10 bits */
	refTxAnGn = (refTxAnGn & 0x3FF);
	/* format change from x.4 to x.7 */
	refTxAnGn = refTxAnGn * 8;
	phy_reg_write(pi, LCN40PHY_papd_tx_analog_gain_ref, (uint16)refTxAnGn);
	PHY_PAPD(("refTxAnGn = %d\n", refTxAnGn));

	/* Force ADC on */
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 31);

	/* regs to be override in PAPD_loopback */
	SAVE_RFOverride0  = phy_reg_read(pi, LCN40PHY_RFOverride0);
	SAVE_RFOverrideVal0 = phy_reg_read(pi, LCN40PHY_RFOverrideVal0);
	SAVE_RF_auxpga_cfg1 = read_radio_reg(pi, RADIO_2065_AUXPGA_CFG1);
	SAVE_RF_lpf_gain = read_radio_reg(pi, RADIO_2065_LPF_GAIN);
	SAVE_AfeCtrlOvr1 = phy_reg_read(pi, LCN40PHY_AfeCtrlOvr1);
	SAVE_AfeCtrlOvr1Val = phy_reg_read(pi, LCN40PHY_AfeCtrlOvr1Val);
	SAVE_rfoverride2 = phy_reg_read(pi, LCN40PHY_rfoverride2);
	SAVE_rfoverride2val = phy_reg_read(pi, LCN40PHY_rfoverride2val);
	SAVE_RF_pa2g_cfg1 = read_radio_reg(pi, RADIO_2065_PA2G_CFG1);
	SAVE_RF_rxrf2g_cfg1 = read_radio_reg(pi, RADIO_2065_RXRF2G_CFG1);
	SAVE_RF_OVR1 = read_radio_reg(pi, RADIO_2065_OVR1);
	SAVE_RF_OVR3 = read_radio_reg(pi, RADIO_2065_OVR3);
	SAVE_RF_OVR4 = read_radio_reg(pi, RADIO_2065_OVR4);
	SAVE_RF_OVR5 = read_radio_reg(pi, RADIO_2065_OVR5);
	SAVE_RF_OVR6 = read_radio_reg(pi, RADIO_2065_OVR6);
	SAVE_RF_OVR7 = read_radio_reg(pi, RADIO_2065_OVR7);
	SAVE_RF_OVR11 = read_radio_reg(pi, RADIO_2065_OVR11);
	SAVE_RF_OVR12 = read_radio_reg(pi, RADIO_2065_OVR12);
	SAVE_RF_logen2g_txdiv = read_radio_reg(pi, RADIO_2065_LOGEN2G_TXDIV);
	SAVE_RF_logen2g_rxdiv = read_radio_reg(pi, RADIO_2065_LOGEN2G_RXDIV);
	SAVE_RF_lna2g_cfg2 = read_radio_reg(pi, RADIO_2065_LNA2G_CFG2);
	SAVE_RF_lna2g_cfg1 = read_radio_reg(pi, RADIO_2065_LNA2G_CFG1);
	SAVE_RF_lpf_cfg1 = read_radio_reg(pi, RADIO_2065_LPF_CFG1);
	SAVE_RF_rxmix2g_cfg1 = read_radio_reg(pi, RADIO_2065_RXMIX2G_CFG1);
	SAVE_RF_rx_reg_backup_1 = read_radio_reg(pi, RADIO_2065_RX_REG_BACKUP_1);
	SAVE_RF_tia_cfg1 = read_radio_reg(pi, RADIO_2065_TIA_CFG1);
	SAVE_RF_top_spare1 = read_radio_reg(pi, RADIO_2065_TOP_SPARE1);
	SAVE_RF_txrx2g_cal = read_radio_reg(pi, RADIO_2065_TXRX2G_CAL);

	/* LOOPBACK SETTINGS */
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7, lpf_hpc_ovr, 0x1);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride7val, lpf_hpc_ovr_val, 0x1);
	/* force tx and rx on */
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrftxpu_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrftxpu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1);
	/* PAPD loopback, attn */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		mod_radio_reg(pi, RADIO_2065_OVR12, 0X10, 1 << 4);
		mod_radio_reg(pi, RADIO_2065_TXRX2G_CAL, 0x2, 1 << 1);
		mod_radio_reg(pi, RADIO_2065_OVR11, 0x8000, 1 << 15);
		mod_radio_reg(pi, RADIO_2065_TXRX2G_CAL, 0x1, 1);
	} else {
		mod_radio_reg(pi, RADIO_2065_OVR12, 0X10, 1 << 4);
		mod_radio_reg(pi, RADIO_2065_TXRX2G_CAL, 0x2, 1 << 1);
		mod_radio_reg(pi, RADIO_2065_OVR11, 0x8000, 1 << 15);
		mod_radio_reg(pi, RADIO_2065_TXRX2G_CAL, 0x1, 1);
	}
	/* PAPD attenuation */
	if (CHSPEC_IS40(pi->radio_chanspec))
		write_radio_reg(pi, RADIO_2065_TXRX2G_CAL, 0xd3);
	else
		write_radio_reg(pi, RADIO_2065_TXRX2G_CAL, 0xd3);

	if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID || CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
		write_radio_reg(pi, RADIO_2065_RX_REG_BACKUP_1, 0X1);
	/* shut off LNA's */
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2val, slna_pu_ovr_val, 0);
	/* LPF gain */
	mod_radio_reg(pi, RADIO_2065_OVR6, 0x1000, 1 << 12);
	mod_radio_reg(pi, RADIO_2065_LPF_GAIN, 0x78, 0);
	mod_radio_reg(pi, RADIO_2065_OVR5, 0x1, 1);
	mod_radio_reg(pi, RADIO_2065_LPF_GAIN, 0x7, 0);
	or_radio_reg(pi, RADIO_2065_TOP_SPARE1, 0x0001);
	/* various Rx settings, ground LNA1 input etc. */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		mod_radio_reg(pi, RADIO_2065_TIA_CFG1, 0x1, 1);
		mod_radio_reg(pi, RADIO_2065_OVR12, 0x200, 1 << 9);
		mod_radio_reg(pi, RADIO_2065_TIA_CFG1, 0x4, 1 << 2);
		mod_radio_reg(pi, RADIO_2065_OVR12, 0x100, 1 << 8);
		mod_radio_reg(pi, RADIO_2065_TIA_CFG1, 0x2, 1 << 1);
		mod_radio_reg(pi, RADIO_2065_OVR12, 0x2000, 1 << 13);
		mod_radio_reg(pi, RADIO_2065_RXMIX2G_CFG1, 0x1, 1);
		mod_radio_reg(pi, RADIO_2065_OVR11, 0x800, 1 << 11);
		mod_radio_reg(pi, RADIO_2065_LNA2G_CFG1, 0x1, 0);
		mod_radio_reg(pi, RADIO_2065_OVR3, 0x80, 1 << 7);
		mod_radio_reg(pi, RADIO_2065_OVR3, 0x2, 1 << 1);
		mod_radio_reg(pi, RADIO_2065_LNA2G_CFG1, 0x8, 0);
		mod_radio_reg(pi, RADIO_2065_LNA2G_CFG2, 0x1, 0);
		mod_radio_reg(pi, RADIO_2065_OVR3, 0x8, 1 << 3);
		mod_radio_reg(pi, RADIO_2065_PA2G_CFG1, 0x20, 0);
		mod_radio_reg(pi, RADIO_2065_OVR12, 0x40, 1 << 6);
		mod_radio_reg(pi, RADIO_2065_OVR4, 0x8, 1 << 3);
		mod_radio_reg(pi, RADIO_2065_LOGEN2G_RXDIV, 0x1, 1);
		mod_radio_reg(pi, RADIO_2065_OVR4, 0x4, 1 << 2);
		mod_radio_reg(pi, RADIO_2065_LOGEN2G_RXDIV, 0x2, 1 << 1);
		mod_radio_reg(pi, RADIO_2065_OVR11, 0x40, 1 << 6);
		mod_radio_reg(pi, RADIO_2065_RXRF2G_CFG1, 0x1, 1);
	} else {
		mod_radio_reg(pi, RADIO_2065_TIA_CFG1, 0x1, 1);
		mod_radio_reg(pi, RADIO_2065_OVR12, 0x200, 1 << 9);
		mod_radio_reg(pi, RADIO_2065_RXMIX5G_CFG1, 0x1, 1);
		mod_radio_reg(pi, RADIO_2065_OVR11, 0x80, 1 << 7);
		mod_radio_reg(pi, RADIO_2065_LNA5G_CFG1, 0x1, 0);
		mod_radio_reg(pi, RADIO_2065_OVR4, 0x200, 1 << 9);
		mod_radio_reg(pi, RADIO_2065_LNA5G_CFG2, 0x1, 0);
		mod_radio_reg(pi, RADIO_2065_OVR4, 0x100, 1 << 8);
		/* FIX ME: update corresponding registers of "RF_pa2g_cfg1.trsw2g_pu" */
	}
	/* TR switch (internal) */
	if (CHSPEC_IS2G(pi->radio_chanspec))
		mod_radio_reg(pi, RADIO_2065_OVR12, 0x40, 1 << 6);
	/* disable aux (tssi etc.) path */
	mod_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, 0x3000, 0);
	mod_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, 0x30, 1 << 4);
	mod_radio_reg(pi, RADIO_2065_OVR1, 0x8000, 1 << 15);
	mod_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, 0x1, 0);
	/*  lpf_pu in AMS */
	mod_radio_reg(pi, RADIO_2065_OVR6, 0x10, 1 << 4);
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0xc, 0xc);
	mod_radio_reg(pi, RADIO_2065_OVR6, 0x8, 1 << 3);
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0x3, 0x3);
	/* lpf_rxbuf_pu in AMS */
	mod_radio_reg(pi, RADIO_2065_OVR7, 0x80, 1 << 7);
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0x80, 1 << 7);
	mod_radio_reg(pi, RADIO_2065_OVR7, 0x40, 1 << 6);
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0x40, 1 << 6);
	/* sel_tx_rx in AMS */
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0xc00, 0xc00);
	mod_radio_reg(pi, RADIO_2065_OVR7, 0x4, 1 << 2);
	/* sel_byp_txlpf in AMS */
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0x200, 0);
	mod_radio_reg(pi, RADIO_2065_OVR7, 0x10, 1 << 4);
	/* sel_byp_rxlpf in AMS */
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0xc000, 1 << 14);
	mod_radio_reg(pi, RADIO_2065_OVR7, 0x20, 1 << 5);
	/* sel_rx_buf in AMS */
	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0x3000, 1 << 12);
	mod_radio_reg(pi, RADIO_2065_OVR7, 0x8, 1 << 3);
	wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
	wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 0, 0, 2, 0, 0, 0,
		0, 0, 0, 0);
	/* End of PAPD loopback setting */

	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 0x1);
	/* Do Rx Gain Control */
	rxGnIdx = wlc_lcn40phy_papd_rxGnCtrl(pi, cal_type, frcRxGnCtrl, CurTxGain);

	/* Set Rx Gain */
	wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 0, 0, 0, 0, 0, (uint16)rxGnIdx,
		0, 0, 0, 0);

	/* Do PAPD Operation - All symbols in one go */
	wlc_lcn40phy_papd_cal_core(pi, cal_type,
		FALSE,
		txGnCtrl,
		samplecapture,
		papd_dbg_mode,
		num_symbols,
		init_papd_lut,
		1400,
		16640,
		0,
		512,
		0);

	wlc_lcn40phy_GetpapdMaxMinIdxupdt(pi, &maxUpdtIdx, &minUpdtIdx);

	PHY_PAPD(("wl%d: %s max: %d, min: %d\n",
		pi->sh->unit, __FUNCTION__, maxUpdtIdx, minUpdtIdx));

	if ((minUpdtIdx >= 0) && (minUpdtIdx < PHY_PAPD_EPS_TBL_SIZE_LCN40PHY) &&
		(maxUpdtIdx >= 0) && (maxUpdtIdx < PHY_PAPD_EPS_TBL_SIZE_LCN40PHY) &&
		(minUpdtIdx <= maxUpdtIdx)) {

		if (cal_type == PHY_PAPD_CAL_CW) {
			tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_offset = minUpdtIdx;
			tab.tbl_ptr = &tmpVar; /* ptr to buf */
			wlc_lcn40phy_read_table(pi, &tab);
			min_papd_lut_val = tmpVar;
			tab.tbl_offset = maxUpdtIdx;
			wlc_lcn40phy_read_table(pi, &tab);
			max_papd_lut_val = tmpVar;
			for (j = 0; j < minUpdtIdx; j++) {
				tmpVar = min_papd_lut_val;
				tab.tbl_offset = j;
				tab.tbl_ptr = &tmpVar;
				wlc_lcn40phy_write_table(pi, &tab);
			}
		}

		PHY_PAPD(("wl%d: %s: PAPD cal completed\n", pi->sh->unit, __FUNCTION__));
		if (papd_peak_curr_mode == 0) {
			tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
			tab.tbl_offset = 62;
			tab.tbl_ptr = &tmpVar;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = 63;
			wlc_lcn40phy_write_table(pi, &tab);
			tab.tbl_offset = 1;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = 0;
			wlc_lcn40phy_write_table(pi, &tab);
		}

		wlc_lcn40phy_save_papd_calibration_results(pi);
	}
	else
		PHY_PAPD(("Error in PAPD Cal. Exiting... \n"));


	/* restore saved registers */
	phy_reg_write(pi, LCN40PHY_lpfbwlutreg0, lpfbwlut0);
	phy_reg_write(pi, LCN40PHY_lpfbwlutreg1, lpfbwlut1);

	phy_reg_write(pi, LCN40PHY_AphyControlAddr, AphyControlAddr_old);
	phy_reg_write(pi, LCN40PHY_Core1TxControl, Core1TxControl_old);
	phy_reg_write(pi, LCN40PHY_sslpnCtrl3, lcnCtrl3_old);

	/* restore calib ctrl clk */
	/* switch on PAPD clk */
	phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl, sslpnCalibClkEnCtrl_old);

	/* Restore AMUX sel */
	wlc_lcn40phy_amuxsel_set(pi, save_amuxSelPortOverride, save_amuxSelPortOverrideVal);
	/* TR switch */
	wlc_lcn40phy_clear_trsw_override(pi);

	/* restore reg in papd_loopback */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_reg_write(pi, LCN40PHY_RFOverride0, SAVE_RFOverride0);
		phy_reg_write(pi, LCN40PHY_RFOverrideVal0, SAVE_RFOverrideVal0);
		write_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, SAVE_RF_auxpga_cfg1);
		write_radio_reg(pi, RADIO_2065_LPF_GAIN, SAVE_RF_lpf_gain);
		phy_reg_write(pi, LCN40PHY_AfeCtrlOvr1, SAVE_AfeCtrlOvr1);
		phy_reg_write(pi, LCN40PHY_AfeCtrlOvr1Val, SAVE_AfeCtrlOvr1Val);
		phy_reg_write(pi, LCN40PHY_rfoverride2, SAVE_rfoverride2);
		phy_reg_write(pi, LCN40PHY_rfoverride2val, SAVE_rfoverride2val);
		write_radio_reg(pi, RADIO_2065_PA2G_CFG1, SAVE_RF_pa2g_cfg1);
		write_radio_reg(pi, RADIO_2065_RXRF2G_CFG1, SAVE_RF_rxrf2g_cfg1);
		write_radio_reg(pi, RADIO_2065_OVR1, SAVE_RF_OVR1);
		write_radio_reg(pi, RADIO_2065_OVR3, SAVE_RF_OVR3);
		write_radio_reg(pi, RADIO_2065_OVR4, SAVE_RF_OVR4);
		write_radio_reg(pi, RADIO_2065_OVR5, SAVE_RF_OVR5);
		write_radio_reg(pi, RADIO_2065_OVR6, SAVE_RF_OVR6);
		write_radio_reg(pi, RADIO_2065_OVR7, SAVE_RF_OVR7);
		write_radio_reg(pi, RADIO_2065_OVR11, SAVE_RF_OVR11);
		write_radio_reg(pi, RADIO_2065_OVR12, SAVE_RF_OVR12);
		write_radio_reg(pi, RADIO_2065_LOGEN2G_TXDIV, SAVE_RF_logen2g_txdiv);
		write_radio_reg(pi, RADIO_2065_LOGEN2G_RXDIV, SAVE_RF_logen2g_rxdiv);
		write_radio_reg(pi, RADIO_2065_LNA2G_CFG2, SAVE_RF_lna2g_cfg2);
		write_radio_reg(pi, RADIO_2065_LNA2G_CFG1, SAVE_RF_lna2g_cfg1);
		write_radio_reg(pi, RADIO_2065_RXMIX2G_CFG1, SAVE_RF_rxmix2g_cfg1);
		write_radio_reg(pi, RADIO_2065_RX_REG_BACKUP_1, SAVE_RF_rx_reg_backup_1);
		write_radio_reg(pi, RADIO_2065_TIA_CFG1, SAVE_RF_tia_cfg1);
		write_radio_reg(pi, RADIO_2065_TOP_SPARE1, SAVE_RF_top_spare1);
		write_radio_reg(pi, RADIO_2065_TXRX2G_CAL, SAVE_RF_txrx2g_cal);
	}

	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, trsw_rx_pu_ovr, trsw_rx_pu_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, ext_lna_gain_ovr, ext_lna_gain_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_byp_ovr, slna_byp_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride3, slna_rout_ctrl_ovr, slna_rout_ctrl_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride2, slna_gain_ctrl_ovr, slna_gain_ctrl_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride5, rxrf_lna2_gain_ovr, rxrf_lna2_gain_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride5, rxrf_lna2_rout_ovr, rxrf_lna2_rout_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride5, rxrf_tia_gain_ovr, rxrf_tia_gain_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride6, lpf_bq1_gain_ovr, lpf_bq1_gain_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, rfoverride6, lpf_bq2_gain_ovr, lpf_bq2_gain_ovr_old);
	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr, digi_gain_ovr_old);

	/* Restore rx path mux and turn off PAPD mixer, bias filter settings */

	/* Clear rx PU override */
	phy_reg_mod(pi, LCN40PHY_RFOverride0,
		LCN40PHY_RFOverride0_internalrfrxpu_ovr_MASK,
		0 << LCN40PHY_RFOverride0_internalrfrxpu_ovr_SHIFT);

	wlc_lcn40phy_tx_pu(pi, FALSE);

	/* Clear rx gain override */
	wlc_lcn40phy_rx_gain_override_enable(pi, FALSE);

	/* Clear ADC override */
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 0);

	/* Turn OFF Tx Chain */
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
	/* force on the transmit chain */
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrftxpu_ovr_val, 0);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrftxpu_ovr, 0);
	/* Force the TR switch to transmit */
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, trsw_rx_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, trsw_rx_pu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, trsw_tx_pu_ovr_val, 0);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, trsw_tx_pu_ovr, 0);
	/* Force the antenna to 0 */
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, ant_selp_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, ant_selp_ovr, 0);
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* force off the Gband-ePA */
		PHY_REG_MOD(pi, LCN40PHY, RFOverride0, gmode_tx_pu_ovr, 0);
		PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, gmode_tx_pu_ovr_val, 0);
		/* pa pu */
		PHY_REG_MOD(pi, LCN40PHY, rfoverride4val, papu_ovr_val, 0);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride4, papu_ovr, 0);
	} else  {
		/* force off the Aband-ePA */
		PHY_REG_MOD(pi, LCN40PHY, RFOverride0, amode_tx_pu_ovr, 0);
		PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, amode_tx_pu_ovr_val, 0);
		/* pa pu */
		PHY_REG_MOD(pi, LCN40PHY, rfoverride4val, papu_ovr_val, 0);
		PHY_REG_MOD(pi, LCN40PHY, rfoverride4, papu_ovr, 0);
	}

	/* Restore CRS */
	wlc_lcn40phy_deaf_mode(pi, FALSE);

	/* Restore Digital Filter */
	wlc_lcn40phy_save_restore_dig_filt_state(pi, FALSE, save_filtcoeffs);

	/* Enable PAPD */
	wlc_lcn40phy_papd_block_enable(pi, TRUE);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	/* restore bbmult */
	wlc_lcn40phy_set_bbmult(pi, (uint8)bb_mult_old);

}

static void
wlc_lcn40phy_idle_tssi_est(phy_info_t *pi)
{
	bool suspend, tx_gain_override_old;
	phy_txgains_t old_gains;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	uint8 SAVE_bbmult;
	uint16 idleTssi, idleTssi0_2C, idleTssi0_OB, idleTssi0_regvalue_OB, idleTssi0_regvalue_2C;
#if TWO_POWER_RANGE_TXPWR_CTRL
	uint16 idleTssi1, idleTssi1_2C, idleTssi1_OB, idleTssi1_regvalue_OB, idleTssi1_regvalue_2C;
#endif
	uint16 SAVE_txpwrctrl = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	uint8 SAVE_indx = wlc_lcn40phy_get_current_tx_pwr_idx(pi);

	uint16 SAVE_lpfgainBIQ1 = read_radio_reg(pi, RADIO_2065_OVR5) & 1;
	uint16 SAVE_lpfgainBIQ2 = read_radio_reg(pi, RADIO_2065_OVR6) & 0x1000;

	idleTssi = phy_reg_read(pi, LCN40PHY_TxPwrCtrlStatus);
	suspend = !(R_REG(pi->sh->osh, &((phy_info_t *)pi)->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);
	wlc_btcx_override_enable(pi);
	/* Save old tx gains if needed */
	tx_gain_override_old = wlc_lcn40phy_tx_gain_override_enabled(pi);
	wlc_lcn40phy_get_tx_gain(pi, &old_gains);
	/* set txgain override */
	wlc_lcn40phy_enable_tx_gain_override(pi);
	wlc_lcn40phy_set_tx_pwr_by_index(pi, 127);

	wlc_lcn40phy_tssi_setup(pi);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 1);

	SAVE_bbmult = wlc_lcn40phy_get_bbmult(pi);
	wlc_lcn40phy_set_bbmult(pi, 0x0);
	wlc_phy_do_dummy_tx(pi, TRUE, OFF);

	idleTssi = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatus, estPwr);
	/* avgTssi value is in 2C (S9.0) format */
	idleTssi0_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew4, avgTssi);

	/* Convert idletssi1_2C from 2C to OB format by toggling MSB OB value */
	/* ranges from 0 to (2^9-1) = 511, 2C value ranges from -256 to (2^9-1-2^8) = 255 */
	/* Convert 9-bit idletssi1_2C to 9-bit idletssi1_OB. */
	if (idleTssi0_2C >= 256)
		idleTssi0_OB = idleTssi0_2C - 256;
	else
		idleTssi0_OB = idleTssi0_2C + 256;
	/* Convert 9-bit idletssi1_OB to 7-bit value for comparison with idletssi */
	if (idleTssi != (idleTssi0_OB >> 2))
	/* causing cstyle error */
	PHY_ERROR(("wl%d: %s, ERROR: idleTssi estPwr(OB): "
	           "0x%04x Register avgTssi(OB, 7MSB): 0x%04x\n",
	           pi->sh->unit, __FUNCTION__, idleTssi, idleTssi0_OB >> 2));

	idleTssi0_regvalue_OB = idleTssi0_OB;

	if (idleTssi0_regvalue_OB >= 256)
		idleTssi0_regvalue_2C = idleTssi0_regvalue_OB - 256;
	else
		idleTssi0_regvalue_2C = idleTssi0_regvalue_OB + 256;

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0, idleTssi0_regvalue_2C);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 0);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 2);

		wlc_phy_do_dummy_tx(pi, TRUE, OFF);
		idleTssi1 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew2, estPwr1);
		/* avgTssi value is in 2C (S9.0) format */
		idleTssi1_2C = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlStatusNew4Ext, avgTssi1);
		/* Convert idletssi1_2C from 2C to OB format by toggling MSB OB value */
		/* ranges from 0 to (2^9-1) = 511, 2C value ranges from -256 to (2^9-1-2^8) = 255 */
		/* Convert 9-bit idletssi1_2C to 9-bit idletssi1_OB. */
		if (idleTssi1_2C >= 256)
			idleTssi1_OB = idleTssi1_2C - 256;
		else
			idleTssi1_OB = idleTssi1_2C + 256;
		/* Convert 9-bit idletssi1_OB to 7-bit value for comparison with idletssi */
		if (idleTssi1 != (idleTssi1_OB >> 2))
		/* causing cstyle error */
		PHY_ERROR(("wl%d: %s, ERROR: idleTssi estPwr1(OB): "
		           "0x%04x Register avgTssi1(OB, 7MSB): 0x%04x\n",
		           pi->sh->unit, __FUNCTION__, idleTssi1, idleTssi1_OB >> 2));

		idleTssi1_regvalue_OB = idleTssi1_OB;

		if (idleTssi1_regvalue_OB >= 256)
			idleTssi1_regvalue_2C = idleTssi1_regvalue_OB - 256;
		else
			idleTssi1_regvalue_2C = idleTssi1_regvalue_OB + 256;

		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi1, idleTssi1, idleTssi1_regvalue_2C);

		/* Clear tssiRangeOverride */
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 0);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiRangeVal0, 1);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiRangeVal1, 0);
	}
#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	/* Clear tx PU override */
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrftxpu_ovr, 0);
	wlc_lcn40phy_set_bbmult(pi, SAVE_bbmult);
	/* restore txgain override */
	wlc_lcn40phy_set_tx_gain_override(pi, tx_gain_override_old);
	wlc_lcn40phy_set_tx_gain(pi, &old_gains);
	wlc_lcn40phy_set_tx_pwr_by_index(pi, SAVE_indx);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	/* restore radio registers */
	mod_radio_reg(pi, RADIO_2065_OVR5, 1, SAVE_lpfgainBIQ1);
	mod_radio_reg(pi, RADIO_2065_OVR6, 0x1000, SAVE_lpfgainBIQ2);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset, pi_lcn->cckPwrOffset);
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

/* Convert tssi to power LUT */
static void
wlc_lcn40phy_set_estPwrLUT(phy_info_t *pi, int32 lut_num)
{
	phytbl_info_t tab;
	int32 tssi, pwr;
	int32 a1 = 0, b0 = 0, b1 = 0;
	int32 maxtargetpwr, mintargetpwr;

	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	if (lut_num == 0)
		tab.tbl_offset = 0; /* estPwrLuts */
	else
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_EST_PWR_OFFSET; /* estPwrLuts1 */
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &pwr; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */


	if (lut_num == 0) {
		/* Get the PA params for the particular channel we are in */
		wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);
		maxtargetpwr = wlc_lcnphy_tssi2dbm(1, a1, b0, b1);
		mintargetpwr = 0;

	} else {
		b0 = pi->txpa_2g_lo[0];
		b1 = pi->txpa_2g_lo[1];
		a1 = pi->txpa_2g_lo[2];
		maxtargetpwr = wlc_lcnphy_tssi2dbm(1, a1, b0, b1);
		mintargetpwr = 0;
	}

	for (tssi = 0; tssi < 128; tssi++) {
		pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
		wlc_lcn40phy_write_table(pi,  &tab);
		tab.tbl_offset++;
	}
}

static void
wlc_lcn40phy_restore_txiqlo_calibration_results(phy_info_t *pi, uint16 startidx,
	uint16 stopidx, uint8 index)
{
	phytbl_info_t tab;
	uint16 a, b;
	uint16 didq;
	uint32 val;
	uint idx;
	uint8 ei0, eq0, fi0, fq0;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
	a = cache->txiqlocal_a[index];
	b = cache->txiqlocal_b[index];
	didq = cache->txiqlocal_didq[index];
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	a = pi_lcn->lcnphy_cal_results.txiqlocal_a[index];
	b = pi_lcn->lcnphy_cal_results.txiqlocal_b[index];
	didq = pi_lcn->lcnphy_cal_results.txiqlocal_didq[index];
#endif

	wlc_lcn40phy_set_tx_iqcc(pi, a, b);
	wlc_lcn40phy_set_tx_locc(pi, didq);

	/* restore iqlo portion of tx power control tables */
	/* remaining element */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32; /* 32 bit wide	*/
	tab.tbl_len = 1;		/* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */
	for (idx = startidx; idx <= stopidx; idx++) {
		/* iq */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + idx;
		wlc_lcnphy_read_table(pi,  &tab);
		val = (val & 0x0ff00000) |
			((uint32)(a & 0x3FF) << 10) | (b & 0x3ff);
		wlc_lcn40phy_write_table(pi,	&tab);
		/* loft */
		tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_LO_OFFSET + idx;
		val = didq;
		wlc_lcn40phy_write_table(pi,	&tab);
	}

	/* Do not move the below statements up */
	/* We need at least 2us delay to read phytable after writing radio registers */
	/* Apply analog LO */
#if defined(PHYCAL_CACHING)
	ei0 = (uint8)(cache->txiqlocal_ei0);
	eq0 = (uint8)(cache->txiqlocal_eq0);
	fi0 = (uint8)(cache->txiqlocal_fi0);
	fq0 = (uint8)(cache->txiqlocal_fq0);
#else
	ei0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_ei0);
	eq0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_eq0);
	fi0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_fi0);
	fq0 = (uint8)(pi_lcn->lcnphy_cal_results.txiqlocal_fq0);
#endif
	wlc_lcn40phy_set_radio_loft(pi, ei0, eq0, fi0, fq0);
}

static void
wlc_lcn40phy_restore_papd_calibration_results(phy_info_t *pi)
{
	phytbl_info_t tab;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif

	/* write eps table */
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_width = 32;
	tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
	tab.tbl_offset = 0;
#if defined(PHYCAL_CACHING)
	tab.tbl_ptr = cache->papd_eps_tbl;
	wlc_lcnphy_write_table(pi, &tab);

	phy_reg_write(pi, LCN40PHY_papd_tx_analog_gain_ref,
		cache->analog_gain_ref);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_begin,
		cache->lut_begin);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_step,
		cache->lut_step);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_end,
		cache->lut_end);
	phy_reg_write(pi, LCN40PHY_papd_rx_gain_comp_dbm,
		cache->rxcompdbm);
	phy_reg_write(pi, LCN40PHY_papd_control,
		cache->papdctrl);
	phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl,
		cache->sslpnCalibClkEnCtrl);

	/* Restore the last gain index for this channel */
	wlc_lcn40phy_set_tx_pwr_by_index(pi, cache->lcnphy_gain_index_at_last_cal);
#else
	tab.tbl_ptr = pi_lcn->lcnphy_cal_results.papd_eps_tbl;
	wlc_lcn40phy_write_table(pi, &tab);

	phy_reg_write(pi, LCN40PHY_papd_tx_analog_gain_ref,
		pi_lcn->lcnphy_cal_results.analog_gain_ref);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_begin,
		pi_lcn->lcnphy_cal_results.lut_begin);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_step,
		pi_lcn->lcnphy_cal_results.lut_step);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_end,
		pi_lcn->lcnphy_cal_results.lut_end);
	phy_reg_write(pi, LCN40PHY_papd_rx_gain_comp_dbm,
		pi_lcn->lcnphy_cal_results.rxcompdbm);
	phy_reg_write(pi, LCN40PHY_papd_control,
		pi_lcn->lcnphy_cal_results.papdctrl);
	phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl,
		pi_lcn->lcnphy_cal_results.sslpnCalibClkEnCtrl);
#endif /* PHYCAL_CACHING */
}
static void
wlc_lcn40phy_set_rx_iq_comp(phy_info_t *pi, uint16 a, uint16 b)
{
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa0, a0, a);
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb0, b0, b);

	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa1, a1, a);
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb1, b1, b);

	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffa2, a2, a);
	PHY_REG_MOD(pi, LCN40PHY, RxCompcoeffb2, b2, b);
}

static void
wlc_lcn40phy_get_tx_gain(phy_info_t *pi, phy_txgains_t *gains)
{
	uint16 dac_gain;

	dac_gain = phy_reg_read(pi, LCN40PHY_AfeDACCtrl) >>
		LCN40PHY_AfeDACCtrl_dac_ctrl_SHIFT;
	gains->dac_gain = (dac_gain & 0x380) >> 7;

	{
		uint16 rfgain0, rfgain1;

		rfgain0 = (phy_reg_read(pi, LCN40PHY_txgainctrlovrval0) &
			LCN40PHY_txgainctrlovrval0_txgainctrl_ovr_val0_MASK) >>
			LCN40PHY_txgainctrlovrval0_txgainctrl_ovr_val0_SHIFT;
		rfgain1 = (phy_reg_read(pi, LCN40PHY_txgainctrlovrval1) &
			LCN40PHY_txgainctrlovrval1_txgainctrl_ovr_val1_MASK) >>
			LCN40PHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT;

		gains->gm_gain = rfgain0 & 0xff;
		gains->pga_gain = (rfgain0 >> 8) & 0xff;
		gains->pad_gain = rfgain1 & 0xff;
	}
}
static uint8
wlc_lcn40phy_get_bbmult(phy_info_t *pi)
{
	uint16 m0m1;
	phytbl_info_t tab;

	tab.tbl_ptr = &m0m1; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_id = LCN40PHY_TBL_ID_IQLOCAL;         /* iqloCaltbl      */
	tab.tbl_offset = 87; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcn40phy_read_table(pi, &tab);

	return (uint8)((m0m1 & 0xff00) >> 8);
}

/* these rf registers need to be restored after iqlo_soft_cal_full */
static const
uint16 iqlo_loopback_rf_regs[] = {
	RADIO_2065_OVR12,
	RADIO_2065_TX2G_TSSI,
	RADIO_2065_IQCAL_CFG1,
	RADIO_2065_TESTBUF_CFG1,
	RADIO_2065_OVR1,
	RADIO_2065_AUXPGA_CFG1,
	RADIO_2065_LPF_CFG1,
	RADIO_2065_OVR7,
	RADIO_2065_PA2G_CFG1,
	RADIO_2065_OVR8,
	RADIO_2065_OVR13,
	RADIO_2065_AUXPGA_VMID,
	RADIO_2065_LPF_GAIN,
	RADIO_2065_IQCAL_IDAC,
	RADIO_2065_TX5G_TSSI,
	};

static void
wlc_lcn40phy_tx_iqlo_cal(
	phy_info_t *pi,
	phy_txgains_t *target_gains,
	phy_cal_mode_t cal_mode,
	bool keep_tone)
{

	/* starting values used in full cal
	 * -- can fill non-zero vals based on lab campaign (e.g., per channel)
	 * -- format: a0,b0,a1,b1,ci0_cq0_ci1_cq1,di0_dq0,di1_dq1,ei0_eq0,ei1_eq1,fi0_fq0,fi1_fq1
	 */
	phy_txgains_t cal_gains, temp_gains;
	uint16 hash;
	uint8 band_idx;
	int j;
	uint16 ncorr_override[5];
	uint16 syst_coeffs[] =
		{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

	/* cal commands full cal and recal */
	uint16 commands_fullcal[] =  { 0x8434, 0x8334, 0x8084, 0x8267, 0x8056, 0x8234 };

	/* do the recal with full cal cmds for now, re-visit if time becomes
	 * an issue.
	 */
	/* uint16 commands_recal[] =  { 0x8312, 0x8055, 0x8212 }; */
	uint16 commands_recal[] =  { 0x8434, 0x8334, 0x8084, 0x8267, 0x8056, 0x8234 };
	uint16 commands_iq_recal[] =  { 0x8067 };

	/* for populating tx power control table
	(like full cal but skip radio regs, since they don't exist in tx pwr ctrl table)
	*/
	uint16 commands_txpwrctrl[] = { 0x8084, 0x8267, 0x8056, 0x8234 };

	/* calCmdNum register: log2 of settle/measure times for search/gain-ctrl, 4 bits each */
	uint16 command_nums_fullcal[] = { 0x7a97, 0x7a97, 0x7a97, 0x7a87, 0x7a87, 0x7b97 };
	uint16 command_nums_txpwrctrl[] = { 0x7a97, 0x7a87, 0x7a87, 0x7b97 };

	/* do the recal with full cal cmds for now, re-visit if time becomes
	 * an issue.
	 */
	/* uint16 command_nums_recal[] = {  0x7997, 0x7987, 0x7a97 }; */
	uint16 command_nums_recal[] = { 0x7a97, 0x7a97, 0x7a97, 0x7a87, 0x7a87, 0x7b97 };
	uint16 *command_nums = command_nums_fullcal;
	uint16 command_nums_iq_recal[] = { 0x7b97 };

	uint16 *start_coeffs = NULL, *cal_cmds = NULL, cal_type, diq_start;
	uint16 tx_pwr_ctrl_old, save_txpwrctrlrfctrl2, save_txpwrctrlcmd;
	uint16 save_sslpnCalibClkEnCtrl, save_sslpnRxFeClkEnCtrl, save_ClkEnCtrl;
	bool tx_gain_override_old;
	phy_txgains_t old_gains = {0, 0, 0, 0};
	uint i, n_cal_cmds = 0, n_cal_start = 0;
	uint16 *values_to_save;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int16 tone_freq;
	uint8 ei0 = 0, eq0 = 0, fi0 = 0, fq0 = 0;
	uint8 genv_addr, bbi_addr, bbq_addr, rfi_addr, rfq_addr;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcn40phy_calcache_t *cache = &ctx->u.lcn40phy_cache;
#endif

	if (NORADIO_ENAB(pi->pubpi))
		return;

	values_to_save = MALLOC(pi->sh->osh, sizeof(uint16) *
		(ARRAYSIZE(iqlo_loopback_rf_regs) + 2));
	if (values_to_save == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	save_sslpnRxFeClkEnCtrl = phy_reg_read(pi, LCN40PHY_sslpnRxFeClkEnCtrl);
	save_sslpnCalibClkEnCtrl = phy_reg_read(pi, LCN40PHY_sslpnCalibClkEnCtrl);
	save_ClkEnCtrl = phy_reg_read(pi, LCN40PHY_ClkEnCtrl);
	/* turn on clk to iqlo block */
	phy_reg_or(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x40);

	/* Get desired start coeffs and select calibration command sequence */

	switch (cal_mode) {
		case CAL_FULL:
			start_coeffs = syst_coeffs;
			cal_cmds = commands_fullcal;
			n_cal_cmds = ARRAYSIZE(commands_fullcal);
			wlc_lcn40phy_set_radio_loft(pi, 0, 0, 0, 0);
			break;

		case CAL_RECAL:
#if defined(PHYCAL_CACHING)
			ASSERT(cache->txiqlocal_bestcoeffs_valid);
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
#endif

			/* start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs; */

			/* since re-cal is same as full cal */
			start_coeffs = syst_coeffs;

			cal_cmds = commands_recal;
			n_cal_cmds = ARRAYSIZE(commands_recal);
			command_nums = command_nums_recal;
			break;

		case CAL_IQ_RECAL:

#if defined(PHYCAL_CACHING)
			ASSERT(cache->txiqlocal_bestcoeffs_valid);
			start_coeffs = cache->txiqlocal_bestcoeffs;
#else
			ASSERT(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs_valid);
			start_coeffs = pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs;
#endif

			cal_cmds = commands_iq_recal;
			n_cal_cmds = ARRAYSIZE(commands_iq_recal);
			command_nums = command_nums_iq_recal;
			break;

		case CAL_TXPWRCTRL:

			wlc_lcn40phy_get_radio_loft(pi, &ei0, &eq0, &fi0, &fq0);
			start_coeffs = syst_coeffs;
			start_coeffs[7] = (((ei0 & 0xff) << 8) | (eq0 & 0xff));
			start_coeffs[8] = 0;
			start_coeffs[9] = (((fi0 & 0xff) << 8) | (fq0 & 0xff));
			start_coeffs[10] = 0;
			cal_cmds = commands_txpwrctrl;
			n_cal_cmds = ARRAYSIZE(commands_txpwrctrl);
			command_nums = command_nums_txpwrctrl;
			break;

		default:
			ASSERT(FALSE);
	}

	/* Fill in Start Coeffs */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		start_coeffs, 11, 16, 64);

	phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xffff);
	PHY_REG_MOD(pi, LCN40PHY, auxadcCtrl, iqlocalEn, 1);

	/* Save original tx power control mode */
	tx_pwr_ctrl_old = wlc_lcn40phy_get_tx_pwr_ctrl(pi);
	save_txpwrctrlcmd = phy_reg_read(pi, LCN40PHY_TxPwrCtrlCmd);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1);

	/* Disable tx power control */
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, LCN40PHY_TX_PWR_CTRL_OFF);

	save_txpwrctrlrfctrl2 = phy_reg_read(pi, LCN40PHY_TxPwrCtrlRfCtrl2);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl2, afeAuxpgaSelVmidVal0, 0x2a6);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl2, afeAuxpgaSelGainVal0, 2);

	genv_addr = 0xd7;
	bbi_addr = 0x5c;
	bbq_addr = 0x5d;
	rfi_addr = 0x5e;
	rfq_addr = 0x5f;

	PHY_REG_MOD(pi, LCN40PHY, iqlocalRadioRegAddr0, txrf_iqcal_gain_waddr, genv_addr);
	PHY_REG_MOD(pi, LCN40PHY, iqlocalRadioRegAdd1, tx_vos_mxi_waddr, bbi_addr);
	PHY_REG_MOD(pi, LCN40PHY, iqlocalRadioRegAddr2, tx_vos_mxq_waddr, bbq_addr);
	PHY_REG_MOD(pi, LCN40PHY, iqlocalRadioRegAddr2, tx_idac_lo_rfi_waddr9to4,
		(rfi_addr >> 4));
	PHY_REG_MOD(pi, LCN40PHY, iqlocalRadioRegAdd3, tx_idac_lo_rfi_waddr3to0,
		(rfi_addr & 0xf));
	PHY_REG_MOD(pi, LCN40PHY, iqlocalRadioRegAdd3, tx_idac_lo_rfq_waddr, rfq_addr);

	/* setup tx iq loopback path */
	wlc_lcn40phy_tx_iqlo_loopback(pi, values_to_save);

	/* Save old and apply new tx gains if needed */
	tx_gain_override_old = wlc_lcn40phy_tx_gain_override_enabled(pi);
	if (tx_gain_override_old)
		wlc_lcn40phy_get_tx_gain(pi, &old_gains);

	if (!target_gains) {
		if (!tx_gain_override_old)
			wlc_lcn40phy_set_tx_pwr_by_index(pi, pi_lcn->lcnphy_tssi_idx);
		wlc_lcn40phy_get_tx_gain(pi, &temp_gains);
		target_gains = &temp_gains;
	}

	hash = (target_gains->gm_gain << 8) |
		(target_gains->pga_gain << 4) |
		(target_gains->pad_gain);

	band_idx = (CHSPEC_IS5G(pi->radio_chanspec) ? 1 : 0);

	cal_gains = *target_gains;
	bzero(ncorr_override, sizeof(ncorr_override));
	for (j = 0; j < iqcal_gainparams_numgains_lcn40phy[band_idx]; j++) {
		if (hash == tbl_iqcal_gainparams_lcn40phy[band_idx][j][0]) {
			cal_gains.gm_gain = tbl_iqcal_gainparams_lcn40phy[band_idx][j][1];
			cal_gains.pga_gain = tbl_iqcal_gainparams_lcn40phy[band_idx][j][2];
			cal_gains.pad_gain = tbl_iqcal_gainparams_lcn40phy[band_idx][j][3];
			bcopy(&tbl_iqcal_gainparams_lcn40phy[band_idx][j][3], ncorr_override,
				sizeof(ncorr_override));
			break;
		}
	}
	/* apply cal gains */
	wlc_lcn40phy_set_tx_gain(pi, &cal_gains);

	PHY_INFORM(("wl%d: %s: target gains: %d %d %d %d, cal_gains: %d %d %d %d\n",
		pi->sh->unit, __FUNCTION__,
		target_gains->gm_gain,
		target_gains->pga_gain,
		target_gains->pad_gain,
		target_gains->dac_gain,
		cal_gains.gm_gain,
		cal_gains.pga_gain,
		cal_gains.pad_gain,
		cal_gains.dac_gain));

	/* set gain control parameters */
	phy_reg_write(pi, LCN40PHY_iqloCalCmdGctl, 0xaa9);
	phy_reg_write(pi, LCN40PHY_iqloCalGainThreshD2, 0xc0);

	/* Load the LO compensation gain table */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		(CONST void *)lcn40phy_iqcal_loft_gainladder,
		ARRAYSIZE(lcn40phy_iqcal_loft_gainladder), 16, 0);
	/* Load the IQ calibration gain table */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		(CONST void *)lcn40phy_iqcal_ir_gainladder, ARRAYSIZE(lcn40phy_iqcal_ir_gainladder),
		16, 32);

	if (pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val == 0) {
		if (CHIPID(pi->sh->chip) == BCM4314_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM43142_CHIP_ID)
			tone_freq = 2500;
		else
			tone_freq = 3750;
	} else
		tone_freq = pi_lcn->lcnphy_tx_iqlo_tone_freq_ovr_val;

	if (cal_mode == CAL_IQ_RECAL)
		tone_freq = -tone_freq;

	/* Send out calibration tone */
	if (pi->phy_tx_tone_freq) {
		/* if tone is already played out with iq cal mode zero then
		 * stop the tone and re-play with iq cal mode 1.
		 */
		wlc_lcn40phy_stop_tx_tone(pi);
		OSL_DELAY(5);
	}

	if (CHSPEC_IS40(pi->radio_chanspec) && LCN40REV_LE(pi->pubpi.phy_rev, 3)) {
		wlc_lcn40phy_start_tx_tone(pi, ((tone_freq + 10000) * 1000), 88, 1);
		wlc_lcn40phy_play_sample_table1(pi, tone_freq * 1000, 88);

	} else
		wlc_lcn40phy_start_tx_tone(pi, (tone_freq * 1000), 88, 1);

	/* FIX ME: re-enable all the phy clks. */
	phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0xffff);
	/*
	 * Cal Steps
	 */
	for (i = n_cal_start; i < n_cal_cmds; i++) {
		uint16 zero_diq = 0;
		uint16 best_coeffs[11];
		uint16 command_num;

		cal_type = (cal_cmds[i] & 0x0f00) >> 8;


		/* get & set intervals */
		command_num = command_nums[i];
		if (ncorr_override[cal_type])
			command_num = ncorr_override[cal_type] << 8 | (command_num & 0xff);
		/* enable IQLO select MUX */
		phy_reg_or(pi, LCN40PHY_sslpnCtrl3, 4);
		phy_reg_write(pi, LCN40PHY_iqloCalCmdNnum, command_num);

		PHY_TMP(("wl%d: %s: running cmd: %x, cmd_num: %x\n",
			pi->sh->unit, __FUNCTION__, cal_cmds[i], command_nums[i]));

		if (cal_type == 0 &&
			(LCN40REV_LE(pi->pubpi.phy_rev, 1)))
			continue;

		/* need to set di/dq to zero if analog LO cal */
		if ((cal_type == 3) || (cal_type == 4)) {
			/* store given dig LOFT comp vals */
			wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
				&diq_start, 1, 16, 69);
			/* Set to zero during analog LO cal */
			wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL, &zero_diq,
				1, 16, 69);
		}

		/* Issue cal command */
		phy_reg_write(pi, LCN40PHY_iqloCalCmd, cal_cmds[i]);

		/* Wait until cal command finished */
		if (!wlc_lcn40phy_iqcal_wait(pi)) {
			PHY_ERROR(("wl%d: %s: tx iqlo cal failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			/* No point to continue */
			goto cleanup;
		}

		/* Copy best coefficients to start coefficients */
		wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			best_coeffs, ARRAYSIZE(best_coeffs), 16, 96);
		wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL, best_coeffs,
			ARRAYSIZE(best_coeffs), 16, 64);

		/* restore di/dq in case of analog LO cal */
		if ((cal_type == 3) || (cal_type == 4)) {
			wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
				&diq_start, 1, 16, 69);
		}
#if defined(PHYCAL_CACHING)
		wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			cache->txiqlocal_bestcoeffs,
			ARRAYSIZE(cache->txiqlocal_bestcoeffs), 16, 96);
#else
		wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
			pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs,
			ARRAYSIZE(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs), 16, 96);
#endif /* PHYCAL_CACHING */
	}

	/*
	 * Apply Results
	 */

	/* Save calibration results */
#if defined(PHYCAL_CACHING)
	wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		cache->txiqlocal_bestcoeffs,
		ARRAYSIZE(cache->txiqlocal_bestcoeffs), 16, 96);
	cache->txiqlocal_bestcoeffs_valid = TRUE;

	/* Apply IQ Cal Results */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		&cache->txiqlocal_bestcoeffs[0], 4, 16, 80);
	/* Apply Digital LOFT Comp */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		&cache->txiqlocal_bestcoeffs[5], 2, 16, 85);
#else
	wlc_lcn40phy_common_read_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs,
		ARRAYSIZE(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs), 16, 96);

	/* Apply IQ Cal Results */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		&pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[0], 4, 16, 80);
	/* Apply Digital LOFT Comp */
	wlc_lcn40phy_common_write_table(pi, LCN40PHY_TBL_ID_IQLOCAL,
		&pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5], 2, 16, 85);

	/* only for rev0 and 1, not need for rev2 and above */
	if (LCN40REV_LE(pi->pubpi.phy_rev, 1)) {
		wlc_lcn40phy_set_tx_iqcc(pi, 0, 0);
		pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[0] =
			pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[1] = 0;
	}

	/* Dump results */
	PHY_INFORM(("wl%d: %s complete, IQ %d %d LO %d %d %d %d %d %d\n",
		pi->sh->unit, __FUNCTION__,
		(int16)pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[0],
		(int16)pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[1],
		(int8)((pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5] & 0xff00) >> 8),
		(int8)(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[5] & 0x00ff),
		(int8)((pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[7] & 0xff00) >> 8),
		(int8)(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[7] & 0x00ff),
		(int8)((pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[9] & 0xff00) >> 8),
		(int8)(pi_lcn->lcnphy_cal_results.txiqlocal_bestcoeffs[9] & 0x00ff)));

#endif /* PHYCAL_CACHING */

cleanup:
	/* Switch off test tone */
	if (!keep_tone)
		wlc_lcn40phy_stop_tx_tone(pi);
	/* stop sample play 1 */
	if (CHSPEC_IS40(pi->radio_chanspec) && LCN40REV_LE(pi->pubpi.phy_rev, 3))
		PHY_REG_MOD(pi, LCN40PHY, sampleCmd, buf2foriqlocal, 0);

	wlc_lcn40phy_tx_iqlo_loopback_cleanup(pi, values_to_save);
	MFREE(pi->sh->osh, values_to_save, sizeof(uint16) * (ARRAYSIZE(iqlo_loopback_rf_regs) + 2));

	/* restore tx power and reenable tx power control */
	phy_reg_write(pi, LCN40PHY_TxPwrCtrlRfCtrl2, save_txpwrctrlrfctrl2);

	/* Reset calibration  command register */
	phy_reg_write(pi, LCN40PHY_iqloCalCmdGctl, 0);

	/* Restore tx power and reenable tx power control */
	if (tx_gain_override_old)
		wlc_lcn40phy_set_tx_gain(pi, &old_gains);
	phy_reg_write(pi, LCN40PHY_TxPwrCtrlCmd, save_txpwrctrlcmd);
	wlc_lcn40phy_set_tx_pwr_ctrl(pi, tx_pwr_ctrl_old);

	/* Restoration RxFE clk */
	phy_reg_write(pi, LCN40PHY_sslpnCalibClkEnCtrl, save_sslpnCalibClkEnCtrl);
	phy_reg_write(pi, LCN40PHY_sslpnRxFeClkEnCtrl, save_sslpnRxFeClkEnCtrl);
	phy_reg_write(pi, LCN40PHY_ClkEnCtrl, save_ClkEnCtrl);
}

static void
wlc_lcn40phy_tx_iqlo_loopback(phy_info_t *pi, uint16 *values_to_save)
{
	uint i;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* save rf registers */
	for (i = 0; i < ARRAYSIZE(iqlo_loopback_rf_regs); i++) {
		values_to_save[i] = read_radio_reg(pi, iqlo_loopback_rf_regs[i]);
	}
	/* force tx on, rx off , force ADC on */
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrftxpu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrftxpu_ovr, 1);

	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1);

	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 1);

	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 0x1f);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 1);

	/* PA override */
	values_to_save[i++] = wlc_lcn40phy_get_pa_gain(pi);
	/* Enable clk */
	PHY_REG_MOD(pi, LCN40PHY, ClkEnCtrl, forcerxfrontendclk, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 1);
	/* Note: We need to turn on the tssi block for txiqcal also */
	or_radio_reg(pi, RADIO_2065_OVR1, 0x8000);
	/* set reg(RF_tx2g_tssi.tx2g_tssi_pu) 1 */
	/* set reg(RF_tx2g_tssi.tx2g_tssi_sel) 1 */
	mod_radio_reg(pi, RADIO_2065_TX2G_TSSI, 0x5, 5);
	/* set reg(RF_iqcal_cfg1.PU_iqcal) 1 */
	or_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 1 << 1);
	/* set reg(RF_testbuf_cfg1.PU) 1 */
	or_radio_reg(pi, RADIO_2065_TESTBUF_CFG1, 1);
	/* set reg(RF_auxpga_cfg1.auxpga_pu) 1 */
	or_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, 1);
	/* set reg(RF_pa2g_cfg1.pa2g_pu) 0 */
	/* set reg(RF_OVR8.ovr_pa2g_pu) 1 */
	mod_radio_reg(pi, RADIO_2065_PA2G_CFG1, 0x1, 0);
	or_radio_reg(pi, RADIO_2065_OVR8, 1 << 10);

	/* set test mux to iqcal */
	/* set reg(RF_testbuf_cfg1.sel_test_port) 0x0 */
	/* set reg(RF_OVR12.ovr_testbuf_sel_test_port) 0x1 */
	mod_radio_reg(pi, RADIO_2065_TESTBUF_CFG1, 0x70, 0);
	or_radio_reg(pi, RADIO_2065_OVR12, 1 << 15);

	mod_radio_reg(pi, RADIO_2065_LPF_CFG1, 0xfec0, 0x6cc0);
	/* set override */
	mod_radio_reg(pi, RADIO_2065_OVR7, 0xfc, 0xfc);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		mod_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 0xF4, 0x84);
		mod_radio_reg(pi, RADIO_2065_TX2G_TSSI, 0x5, 0x5);
		or_radio_reg(pi, RADIO_2065_OVR13, 0x8000);
		or_radio_reg(pi, RADIO_2065_OVR12, 0x2);
	}
#ifdef BAND5G
	else {
		mod_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 0xF4, 0xa4);
		mod_radio_reg(pi, RADIO_2065_TX5G_TSSI, 0x5, 0x5);
		or_radio_reg(pi, RADIO_2065_OVR13, 0xa00);
	}
#endif
	/* adc Vmid , range etc. */
	or_radio_reg(pi, RADIO_2065_OVR1, 0x6000);
	mod_radio_reg(pi, RADIO_2065_AUXPGA_VMID, 0x3FF, 0xB3);
	mod_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, 0x700, 0x2 << 8);
	mod_radio_reg(pi, RADIO_2065_LPF_GAIN, 0xf00, 0);
	mod_radio_reg(pi, RADIO_2065_IQCAL_IDAC, 0x3fff, 0x1558);
	values_to_save[i] = phy_reg_read(pi, LCN40PHY_TxPwrCtrlRfCtrlOverride0);
}

static void
wlc_lcn40phy_tx_iqlo_loopback_cleanup(phy_info_t *pi, uint16 *values_to_save)
{
	uint i;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* tx, rx PU rewrite to save memory */
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrftxpu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, dac_pu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, adc_pu_ovr, 0);
	PHY_REG_MOD(pi, LCN40PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 0);
	/* restore the state of radio regs */
	for (i = 0; i < ARRAYSIZE(iqlo_loopback_rf_regs); i++) {
		write_radio_reg(pi, iqlo_loopback_rf_regs[i], values_to_save[i]);
	}
	wlc_lcn40phy_set_pa_gain(pi, values_to_save[i++]);
	phy_reg_write(pi, LCN40PHY_TxPwrCtrlRfCtrlOverride0, values_to_save[i]);
}

static bool
wlc_lcn40phy_iqcal_wait(phy_info_t *pi)
{
	uint delay_count = 0;

	while (wlc_lcn40phy_iqcal_active(pi)) {
		OSL_DELAY(100);
		delay_count++;

		if (delay_count > (10 * 500)) /* 500 ms */
			break;
	}
	PHY_TMP(("wl%d: %s: %u us\n", pi->sh->unit, __FUNCTION__, delay_count * 100));

	return (wlc_lcn40phy_iqcal_active(pi) == 0);
}


typedef enum {
	LCN40PHY_TSSI_PRE_PA,
	LCN40PHY_TSSI_POST_PA,
	LCN40PHY_TSSI_EXT
} lcn40phy_tssi_mode_t;

static void
wlc_lcn40phy_set_tssi_mux(phy_info_t *pi, lcn40phy_tssi_mode_t pos)
{
	/* Set TSSI/RSSI mux */
	if (LCN40PHY_TSSI_POST_PA == pos) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 1);
		mod_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 0x4, 0);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			mod_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 0);
		} else {
			mod_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 2 << 4);
		}
	} else if (LCN40PHY_TSSI_PRE_PA == pos) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0x1);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 0);
	} else {
		or_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 1 << 2);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			mod_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 1 << 4);
		} else {
			mod_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 0xf0, 3 << 4);
		}
	}

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 0);
}

static uint16
wlc_lcn40phy_rfseq_tbl_adc_pwrup(phy_info_t *pi)
{
	uint16 N1, N2, N3, N4, N5, N6, N;

	N1 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_delay);
	N2 = 1 << PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_intg_log2);
	N3 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_delay);
	N4 = 1 << PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2);
	N5 = PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_temp, Ntemp_delay);
	N6 = 1 << PHY_REG_READ(pi, LCN40PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2);
	N = 2 * (N1 + N2 + N3 + N4 + 2 *(N5 + N6)) + 80;
	if (N < 1600)
		N = 1600; /* min 20 us to avoid tx evm degradation */
	return N;
}


static void
wlc_lcn40phy_pwrctrl_rssiparams(phy_info_t *pi)
{
	uint16 auxpga_vmid, auxpga_gain = 0;
	uint16 auxpga_vmid_temp, auxpga_gain_temp;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

#ifdef BAND5G
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		auxpga_vmid = (pi_lcn->rssismc5g << 4) | pi_lcn->rssismf5g;
		auxpga_gain = pi_lcn->rssisav5g;
	} else
#endif
	{
		auxpga_gain = pi_lcn->lcnphy_rssi_gs;
		auxpga_vmid = (pi_lcn->lcnphy_rssi_vc << 4) |
			pi_lcn->lcnphy_rssi_vf;
	}
	auxpga_vmid_temp = (0 << 8) | (0xa << 4) | 0xd;
	auxpga_gain_temp = 1;
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 0);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 0);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0);

	phy_reg_mod(pi, LCN40PHY_TxPwrCtrlRfCtrl2,
		LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelVmidVal0_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelGainVal0_MASK,
		(auxpga_vmid << LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelVmidVal0_SHIFT) |
		(auxpga_gain << LCN40PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelGainVal0_SHIFT));

	phy_reg_mod(pi, LCN40PHY_TxPwrCtrlRfCtrl3,
		LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelVmidVal1_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelGainVal1_MASK,
		(auxpga_vmid << LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelVmidVal1_SHIFT) |
		(auxpga_gain << LCN40PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelGainVal1_SHIFT));

	phy_reg_mod(pi, LCN40PHY_TxPwrCtrlRfCtrl4,
		LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelVmidVal2_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelGainVal2_MASK,
		(auxpga_vmid << LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelVmidVal2_SHIFT) |
		(auxpga_gain << LCN40PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelGainVal2_SHIFT));

	phy_reg_mod(pi, LCN40PHY_TxPwrCtrlRfCtrl6,
		LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelVmidVal4_MASK |
		LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelGainVal4_MASK,
		(auxpga_vmid_temp << LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelVmidVal4_SHIFT) |
		(auxpga_gain_temp << LCN40PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelGainVal4_SHIFT));

}

static void
wlc_lcn40phy_tssi_setup(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint32 ind;
	uint16 rfseq;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	/* Setup estPwrLuts for measuring idle TSSI */
	tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = &ind; /* ptr to buf */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_offset = 0;
	for (ind = 0; ind < 128; ind++) {
		wlc_lcnphy_write_table(pi,  &tab);
		tab.tbl_offset++;
	}
	tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_EST_PWR_OFFSET;
	for (ind = 0; ind < 128; ind++) {
		wlc_lcnphy_write_table(pi,  &tab);
		tab.tbl_offset++;
	}

	if (!pi_lcn->ePA) {
		wlc_lcn40phy_set_tssi_mux(pi, LCN40PHY_TSSI_POST_PA);
	} else
		wlc_lcn40phy_set_tssi_mux(pi, LCN40PHY_TSSI_EXT);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_en, 1);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, force_vbatTemp, 0);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, pwrIndex_init, 0);
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_delay, 255);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_intg_log2, 5);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_delay, 500);
		PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Ntssi_intg_log2, 6);
	}

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNnum, Npt_intg_log2, 0);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 4);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNum_temp, Ntemp_delay, 64);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 4);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlDeltaPwrLimit, DeltaPwrLimit, 0x1);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRangeCmd, cckPwrOffset, 0);

	PHY_REG_MOD(pi, LCN40PHY, TempSenseCorrection, tempsenseCorr, 0);

	/*  Set idleTssi to (2^9-1) in OB format = (2^9-1-2^8) = 0xff in 2C format */
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi, idleTssi0, 0xff);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlIdleTssi1, idleTssi1, 0xff);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 1);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, 2);

	wlc_lcn40phy_clear_tx_power_offsets(pi);

	/* Temporary fix for 4334, power down ADC before LTF */
	if (LCN40REV_IS(pi->pubpi.phy_rev, 2) || LCN40REV_GE(pi->pubpi.phy_rev, 4))
		rfseq = 600;
	else
		rfseq = wlc_lcn40phy_rfseq_tbl_adc_pwrup(pi);

	tab.tbl_id = LCN40PHY_TBL_ID_RFSEQ;
	tab.tbl_width = 16;	/* 12 bit wide	*/
	tab.tbl_ptr = &rfseq;
	tab.tbl_len = 1;
	tab.tbl_offset = 6;
	wlc_lcnphy_write_table(pi,  &tab);

	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1);
	/* Power up envelope detector and bias circuit, only needed for internal tssi */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		or_radio_reg(pi, RADIO_2065_TX2G_TSSI, 1);
		or_radio_reg(pi, RADIO_2065_OVR12, 0x2);
		and_radio_reg(pi, RADIO_2065_TX5G_TSSI, ~1);
		and_radio_reg(pi, RADIO_2065_OVR13, ~0x800);
	} else {
		or_radio_reg(pi, RADIO_2065_TX5G_TSSI, 1);
		or_radio_reg(pi, RADIO_2065_OVR13, 0x800);
		and_radio_reg(pi, RADIO_2065_TX2G_TSSI, ~1);
		and_radio_reg(pi, RADIO_2065_OVR12, ~0x2);
	}

	/* Power up tssi path after envelope detector, only needed for internal tssi */
	or_radio_reg(pi, RADIO_2065_IQCAL_CFG1, 1);
	or_radio_reg(pi, RADIO_2065_OVR2, 0x2);
	/* Power up AMUX (a.k.a. testbuf) */
	or_radio_reg(pi, RADIO_2065_TESTBUF_CFG1, 1);
	/*  Power up AUX PGA */
	or_radio_reg(pi, RADIO_2065_AUXPGA_CFG1, 1);
	or_radio_reg(pi, RADIO_2065_OVR1, 0x8000);

	/* AMS configuration */
	/* Power up RX buffer */
	or_radio_reg(pi, RADIO_2065_LPF_CFG1, 0xC0);
	or_radio_reg(pi, RADIO_2065_OVR7, 0xC0);
	/* increase envelope detector gain */
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverride, 1);
	PHY_REG_MOD(pi, LCN40PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverrideVal, 1);

	wlc_lcn40phy_pwrctrl_rssiparams(pi);
}

static
int32 wlc_lcn40phy_get_tssi_pwr(phy_info_t *pi, int32 a1, int32 b0,
	int32 b1, uint8 maxlimit, uint8 offset)
{
	int32 tssi, pwr, prev_pwr;
	int32 lcn40phy_tssi_pwr_limit;
	uint8 tssi_ladder_cnt = 0;

	if (maxlimit) {
		prev_pwr = (1<<30)-1;
		lcn40phy_tssi_pwr_limit = (1<<30)-1;
		for (tssi = 0; tssi < 128; tssi++) {
			pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
			if (pwr < prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					lcn40phy_tssi_pwr_limit = pwr;
					break;
				}
			}
		}
	} else {
		prev_pwr = (1<<31);
		lcn40phy_tssi_pwr_limit = (1<<31);
		for (tssi = 127; tssi >= 0; tssi--) {
			pwr = wlc_lcnphy_tssi2dbm(tssi, a1, b0, b1);
			if (pwr > prev_pwr) {
				prev_pwr = pwr;
				if (++tssi_ladder_cnt == offset) {
					lcn40phy_tssi_pwr_limit = pwr;
					break;
				}
			}
		}
}

	return lcn40phy_tssi_pwr_limit;

}

static
void wlc_lcn40phy_get_tssi_offset(phy_info_t *pi, uint8 *offset_maxpwr, uint8 *offset_minpwr)
{
	phy_info_lcnphy_t *pi_lcn = pi->u.pi_lcnphy;

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_2g;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_2g;
			break;
#ifdef BAND5G
	case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_5glo;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_5glo;
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_5gmid;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_5gmid;
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			*offset_maxpwr = pi_lcn->tssi_ladder_offset_maxpwr_5ghi;
			*offset_minpwr = pi_lcn->tssi_ladder_offset_minpwr_5ghi;
			break;
#endif /* BAND5G */
		default:
			ASSERT(FALSE);
			break;
	}
}


static
void wlc_lcn40phy_set_tssi_pwr_limit(phy_info_t *pi, uint8 mode)
{
	int32 a1 = 0, b0 = 0, b1 = 0;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	int32 lcn40phy_tssi_maxpwr_limit = (1<<30)-1;
	int32 lcn40phy_tssi_minpwr_limit = (1<<31);
	uint8 tssi_ladder_offset_maxpwr = 0, tssi_ladder_offset_minpwr = 0;

#if TWO_POWER_RANGE_TXPWR_CTRL
	int32 lcn40phy_tssi_maxpwr_limit_second = (1<<30)-1;
	int32 lcn40phy_tssi_minpwr_limit_second = (1<<31);
#endif

	wlc_phy_get_paparams_for_band(pi, &a1, &b0, &b1);
	wlc_lcn40phy_get_tssi_offset(pi, &tssi_ladder_offset_maxpwr, &tssi_ladder_offset_minpwr);
	lcn40phy_tssi_maxpwr_limit =
		wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 1, tssi_ladder_offset_maxpwr);
	lcn40phy_tssi_minpwr_limit =
		wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 0, tssi_ladder_offset_minpwr);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn->lcnphy_twopwr_txpwrctrl_en) {
		b0 = pi->txpa_2g_lo[0];
		b1 = pi->txpa_2g_lo[1];
		a1 = pi->txpa_2g_lo[2];
		lcn40phy_tssi_maxpwr_limit_second = wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 1,
			tssi_ladder_offset_maxpwr);
		lcn40phy_tssi_minpwr_limit_second = wlc_lcn40phy_get_tssi_pwr(pi, a1, b0, b1, 0,
			tssi_ladder_offset_minpwr);

		lcn40phy_tssi_maxpwr_limit =
			MAX(lcn40phy_tssi_maxpwr_limit, lcn40phy_tssi_maxpwr_limit_second);
		lcn40phy_tssi_minpwr_limit =
			MIN(lcn40phy_tssi_minpwr_limit, lcn40phy_tssi_minpwr_limit_second);
}
#endif /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	if ((mode == PHY_TSSI_SET_MAX_LIMIT) || (mode == PHY_TSSI_SET_MIN_MAX_LIMIT))
		pi_lcn->tssi_maxpwr_limit = lcn40phy_tssi_maxpwr_limit >> 1;

	if ((mode == PHY_TSSI_SET_MIN_LIMIT) || (mode == PHY_TSSI_SET_MIN_MAX_LIMIT))
		pi_lcn->tssi_minpwr_limit = lcn40phy_tssi_minpwr_limit >> 1;
}

/* Save/Restore digital filter state OFDM filter settings */
static void
wlc_lcn40phy_save_restore_dig_filt_state(phy_info_t *pi, bool save, uint16 *filtcoeffs)
{
	int j;

	uint16 addr_ofdm[] = {
		LCN40PHY_txfilt20Stg1Shft,
		LCN40PHY_txfilt20CoeffStg0A1,
		LCN40PHY_txfilt20CoeffStg0A2,
		LCN40PHY_txfilt20CoeffStg0B1,
		LCN40PHY_txfilt20CoeffStg0B2,
		LCN40PHY_txfilt20CoeffStg0B3,
		LCN40PHY_txfilt20CoeffStg1A1,
		LCN40PHY_txfilt20CoeffStg1A2,
		LCN40PHY_txfilt20CoeffStg1B1,
		LCN40PHY_txfilt20CoeffStg1B2,
		LCN40PHY_txfilt20CoeffStg1B3,
		LCN40PHY_txfilt20CoeffStg2A1,
		LCN40PHY_txfilt20CoeffStg2A2,
		LCN40PHY_txfilt20CoeffStg2B1,
		LCN40PHY_txfilt20CoeffStg2B2,
		LCN40PHY_txfilt20CoeffStg2B3,
		LCN40PHY_txfilt20CoeffStg0_leftshift /* This coeff is specific to DAC160 */
		};
	/* Assume 80 MHz Digital filter by default */
	uint8 max_filter_coeffs = LCN40PHY_NUM_DIG_FILT_COEFFS - 1;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	/* If DAC160 then program the Stg0 Leftshift coeff as well, otherwise ignore that */
	if (pi_lcn->dacrate == 160)
		max_filter_coeffs = LCN40PHY_NUM_DIG_FILT_COEFFS;

	if (save) {
		for (j = 0; j < max_filter_coeffs; j++)
			filtcoeffs[j] = phy_reg_read(pi, addr_ofdm[j]);
	} else {
		for (j = 0; j < max_filter_coeffs; j++)
			phy_reg_write(pi, addr_ofdm[j], filtcoeffs[j]);
	}
}

static void wlc_lcn40phy_reset_iir_filter(phy_info_t *pi)
{
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1);

	PHY_REG_MOD(pi, LCN40PHY, sslpnCtrl0, txSoftReset, 1);

	PHY_REG_MOD(pi, LCN40PHY, sslpnCtrl0, txSoftReset, 0);

	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 0);
}
/* set tx digital filter coefficients */
static int
wlc_lcn40phy_load_tx_iir_filter(phy_info_t *pi, phy_tx_iir_filter_mode_t mode, int16 filt_type)
{
	int16 filt_index = -1, j;
	uint16 (*dac_coeffs_table)[LCN40PHY_NUM_DIG_FILT_COEFFS+1];
	uint8 max_filter_type, max_filter_coeffs;
	uint16 *addr_coeff;

	uint16 addr_cck[] = {
		LCN40PHY_ccktxfilt20Stg1Shft,
		LCN40PHY_ccktxfilt20CoeffStg0A1,
		LCN40PHY_ccktxfilt20CoeffStg0A2,
		LCN40PHY_ccktxfilt20CoeffStg0B1,
		LCN40PHY_ccktxfilt20CoeffStg0B2,
		LCN40PHY_ccktxfilt20CoeffStg0B3,
		LCN40PHY_ccktxfilt20CoeffStg1A1,
		LCN40PHY_ccktxfilt20CoeffStg1A2,
		LCN40PHY_ccktxfilt20CoeffStg1B1,
		LCN40PHY_ccktxfilt20CoeffStg1B2,
		LCN40PHY_ccktxfilt20CoeffStg1B3,
		LCN40PHY_ccktxfilt20CoeffStg2A1,
		LCN40PHY_ccktxfilt20CoeffStg2A2,
		LCN40PHY_ccktxfilt20CoeffStg2B1,
		LCN40PHY_ccktxfilt20CoeffStg2B2,
		LCN40PHY_ccktxfilt20CoeffStg2B3,
		LCN40PHY_ccktxfilt20CoeffStg0_leftshift
		};

	uint16 addr_ofdm[] = {
		LCN40PHY_txfilt20Stg1Shft,
		LCN40PHY_txfilt20CoeffStg0A1,
		LCN40PHY_txfilt20CoeffStg0A2,
		LCN40PHY_txfilt20CoeffStg0B1,
		LCN40PHY_txfilt20CoeffStg0B2,
		LCN40PHY_txfilt20CoeffStg0B3,
		LCN40PHY_txfilt20CoeffStg1A1,
		LCN40PHY_txfilt20CoeffStg1A2,
		LCN40PHY_txfilt20CoeffStg1B1,
		LCN40PHY_txfilt20CoeffStg1B2,
		LCN40PHY_txfilt20CoeffStg1B3,
		LCN40PHY_txfilt20CoeffStg2A1,
		LCN40PHY_txfilt20CoeffStg2A2,
		LCN40PHY_txfilt20CoeffStg2B1,
		LCN40PHY_txfilt20CoeffStg2B2,
		LCN40PHY_txfilt20CoeffStg2B3,
		LCN40PHY_txfilt20CoeffStg0_leftshift
		};
	uint16 addr_ofdm_40[] = {
		LCN40PHY_txfilt40Stg1Shft,
		LCN40PHY_txfilt40CoeffStg0A1,
		LCN40PHY_txfilt40CoeffStg0A2,
		LCN40PHY_txfilt40CoeffStg0B1,
		LCN40PHY_txfilt40CoeffStg0B2,
		LCN40PHY_txfilt40CoeffStg0B3,
		LCN40PHY_txfilt40CoeffStg1A1,
		LCN40PHY_txfilt40CoeffStg1A2,
		LCN40PHY_txfilt40CoeffStg1B1,
		LCN40PHY_txfilt40CoeffStg1B2,
		LCN40PHY_txfilt40CoeffStg1B3,
		LCN40PHY_txfilt40CoeffStg2A1,
		LCN40PHY_txfilt40CoeffStg2A2,
		LCN40PHY_txfilt40CoeffStg2B1,
		LCN40PHY_txfilt40CoeffStg2B2,
		LCN40PHY_txfilt40CoeffStg2B3,
		LCN40PHY_txfilt40CoeffStg0_leftshift
		};

	switch (mode) {
	case (TX_IIR_FILTER_OFDM):
		addr_coeff = (uint16 *)addr_ofdm;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_ofdm;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_OFDM;
		break;
	case (TX_IIR_FILTER_OFDM40):
		addr_coeff = (uint16 *)addr_ofdm_40;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_ofdm40;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_OFDM40;
		break;
	case (TX_IIR_FILTER_CCK):
		addr_coeff = (uint16 *)addr_cck;
		dac_coeffs_table = LCN40PHY_txdigfiltcoeffs_cck;
		max_filter_type = LCN40PHY_NUM_TX_DIG_FILTERS_CCK;
		break;
	default:
		/* something wierd happened if coming here */
		addr_coeff = NULL;
		dac_coeffs_table = NULL;
		max_filter_type = 0;
		ASSERT(FALSE);
	}

	max_filter_coeffs = LCN40PHY_NUM_DIG_FILT_COEFFS - 1;

	/* Search for the right entry in the table */
	for (j = 0; j < max_filter_type; j++) {
		if (filt_type == dac_coeffs_table[j][0]) {
			filt_index = (int16)j;
			break;
		}
	}

	/* Grave problem if entry not found */
	if (filt_index == -1) {
		ASSERT(FALSE);
	} else {
		/* Apply the coefficients to the filter type */
		for (j = 0; j < max_filter_coeffs; j++)
			phy_reg_write(pi, addr_coeff[j], dac_coeffs_table[filt_index][j+1]);
	}

	/* Reset the iir filter after setting the coefficients */
	wlc_lcn40phy_reset_iir_filter(pi);

	return (filt_index != -1) ? 0 : -1;
}

static void
wlc_lcn40phy_rx_gain_override_enable(phy_info_t *pi, bool enable)
{
	uint16 ebit = enable ? 1 : 0;

	PHY_REG_MOD(pi, LCN40PHY, RFOverride0, trsw_rx_pu_ovr, ebit);

	phy_reg_mod(pi, LCN40PHY_rfoverride2,
		LCN40PHY_rfoverride2_ext_lna_gain_ovr_MASK |
		LCN40PHY_rfoverride2_slna_byp_ovr_MASK |
		LCN40PHY_rfoverride2_slna_gain_ctrl_ovr_MASK,
		(ebit << LCN40PHY_rfoverride2_ext_lna_gain_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride2_slna_byp_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride2_slna_gain_ctrl_ovr_SHIFT));

	PHY_REG_MOD(pi, LCN40PHY, rfoverride3, slna_rout_ctrl_ovr, ebit);

	phy_reg_mod(pi, LCN40PHY_rfoverride5,
		LCN40PHY_rfoverride5_rxrf_lna2_gain_ovr_MASK |
		LCN40PHY_rfoverride5_rxrf_lna2_rout_ovr_MASK |
		LCN40PHY_rfoverride5_rxrf_tia_gain_ovr_MASK,
		(ebit << LCN40PHY_rfoverride5_rxrf_lna2_gain_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride5_rxrf_lna2_rout_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride5_rxrf_tia_gain_ovr_SHIFT));

	phy_reg_mod(pi, LCN40PHY_rfoverride6,
		LCN40PHY_rfoverride6_lpf_bq1_gain_ovr_MASK |
		LCN40PHY_rfoverride6_lpf_bq2_gain_ovr_MASK,
		(ebit << LCN40PHY_rfoverride6_lpf_bq1_gain_ovr_SHIFT) |
		(ebit << LCN40PHY_rfoverride6_lpf_bq2_gain_ovr_SHIFT));

	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr, ebit);
}

static void
wlc_lcn40phy_set_rx_gain_by_distribution(phy_info_t *pi, uint16 trsw, uint16 ext_lna,
    uint16 slna_byp, uint16 slna_rout, uint16 slna_gain, uint16 lna2_gain, uint16 lna2_rout,
	uint16 tia, uint16 biq1, uint16 biq2, uint16 digi_gain, uint16 digi_offset)
{
	PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, trsw_rx_pu_ovr_val, trsw);

	phy_reg_mod(pi, LCN40PHY_rfoverride2val,
		LCN40PHY_rfoverride2val_ext_lna_gain_ovr_val_MASK |
		LCN40PHY_rfoverride2val_slna_byp_ovr_val_MASK |
		LCN40PHY_rfoverride2val_slna_gain_ctrl_ovr_val_MASK,
		(ext_lna << LCN40PHY_rfoverride2val_ext_lna_gain_ovr_val_SHIFT) |
		(slna_byp << LCN40PHY_rfoverride2val_slna_byp_ovr_val_SHIFT) |
		(slna_gain << LCN40PHY_rfoverride2val_slna_gain_ctrl_ovr_val_SHIFT));
	PHY_REG_MOD(pi, LCN40PHY, rfoverride3_val, slna_rout_ctrl_ovr_val, slna_rout);

	phy_reg_mod(pi, LCN40PHY_rfoverride5val,
		LCN40PHY_rfoverride5val_rxrf_lna2_gain_ovr_val_MASK |
		LCN40PHY_rfoverride5val_rxrf_lna2_rout_ovr_val_MASK |
		LCN40PHY_rfoverride5val_rxrf_tia_gain_ovr_val_MASK,
		(lna2_gain << LCN40PHY_rfoverride5val_rxrf_lna2_gain_ovr_val_SHIFT) |
		(lna2_rout << LCN40PHY_rfoverride5val_rxrf_lna2_rout_ovr_val_SHIFT) |
		(tia << LCN40PHY_rfoverride5val_rxrf_tia_gain_ovr_val_SHIFT));

	phy_reg_mod(pi, LCN40PHY_rfoverride6val,
		LCN40PHY_rfoverride6val_lpf_bq1_gain_ovr_val_MASK |
		LCN40PHY_rfoverride6val_lpf_bq2_gain_ovr_val_MASK,
		(biq1 << LCN40PHY_rfoverride6val_lpf_bq1_gain_ovr_val_SHIFT) |
		(biq2 << LCN40PHY_rfoverride6val_lpf_bq2_gain_ovr_val_SHIFT));

	PHY_REG_MOD(pi, LCN40PHY, radioCtrl, digi_gain_ovr_val, digi_gain);
}

static void
wlc_lcn40phy_rx_pu(phy_info_t *pi, bool bEnable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0);
			PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, amode_rx_pu_ovr_val, 0);
			PHY_REG_MOD(pi, LCN40PHY, RFOverride0, amode_rx_pu_ovr, 1);
		}
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 5, 0, 0, 6, 0, 4, 4, 6, 7, 0);
		wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
	} else {
		/* Force on the receive chain */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1);
			PHY_REG_MOD(pi, LCN40PHY, RFOverride0, internalrfrxpu_ovr, 1);
		} else {
			PHY_REG_MOD(pi, LCN40PHY, RFOverrideVal0, amode_rx_pu_ovr_val, 1);
			PHY_REG_MOD(pi, LCN40PHY, RFOverride0, amode_rx_pu_ovr, 1);
		}
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 0, 0, 5, 0, 0, 4, 0, 2, 0, 3, 0, 0);
		wlc_lcn40phy_rx_gain_override_enable(pi, TRUE);
	}
}

static void
wlc_lcn40phy_set_trsw_override(phy_info_t *pi, bool tx, bool rx)
{	/* Set TR switch */
	phy_reg_mod(pi, LCN40PHY_RFOverrideVal0,
		LCN40PHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK |
		LCN40PHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK,
		(tx ? LCN40PHY_RFOverrideVal0_trsw_tx_pu_ovr_val_MASK : 0) |
		(rx ? LCN40PHY_RFOverrideVal0_trsw_rx_pu_ovr_val_MASK : 0));

	/* Enable overrides */
	phy_reg_or(pi, LCN40PHY_RFOverride0,
		LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
		LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK);
}

static void
wlc_lcn40phy_clear_trsw_override(phy_info_t *pi)
{
	/* Clear overrides */
	phy_reg_and(pi, LCN40PHY_RFOverride0,
		(uint16)~(LCN40PHY_RFOverride0_trsw_tx_pu_ovr_MASK |
		LCN40PHY_RFOverride0_trsw_rx_pu_ovr_MASK));
}

static uint32
wlc_lcn40phy_papd_rxGnCtrl(
	phy_info_t *pi,
	phy_papd_cal_type_t cal_type,
	bool frcRxGnCtrl,
	uint8 CurTxGain)
{
	/* Square of Loop Gain (inv) target for CW (reach as close to tgt, but be more than it) */
	/* dB Loop gain (inv) target for OFDM (reach as close to tgt,but be more than it) */
	int32 rxGnInit = 5;
	uint8  bsStep = 3; /* Binary search initial step size */
	uint8  bsDepth = 4; /* Binary search depth */
	int32  cwLpGn2_min = 131072, cwLpGn2_max = 262144;
	uint8  bsCnt;
	int16  lgI, lgQ;
	int32  cwLpGn2;
	uint8  num_symbols4lpgn;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* frcRxGnCtrl conditional missing */
	for (bsCnt = 0; bsCnt < bsDepth; bsCnt++) {
		/* Running PAPD for Tx gain index:CurTxGain */
		/* Rx gain index : tia gain : rxGnInit */
		PHY_PAPD(("Running PAPD for Tx Gain Idx : %d ,Rx Gain Index %d\n",
			CurTxGain, rxGnInit));
		wlc_lcn40phy_set_rx_gain_by_distribution(pi, 1, 0, 0, 0, 0, 0, 0,
			(uint16)rxGnInit, 0, 0, 0, 0);

		num_symbols4lpgn = 219;
		wlc_lcn40phy_papd_cal_core(pi, 0,
		                         TRUE,
		                         0,
		                         0,
		                         0,
		                         num_symbols4lpgn,
		                         1,
		                         1400,
		                         16640,
		                         0,
		                         512,
		                         0);
		if (cal_type == PHY_PAPD_CAL_CW) {
			lgI = ((int16) phy_reg_read(pi, LCN40PHY_papd_loop_gain_cw_i)) << 4;
			lgI = lgI >> 4;
			lgQ = ((int16) phy_reg_read(pi, LCN40PHY_papd_loop_gain_cw_q)) << 4;
			lgQ = lgQ >> 4;
			cwLpGn2 = (lgI * lgI) + (lgQ * lgQ);

			PHY_PAPD(("LCN40PHY_papd_loop_gain_cw_i %x papd_loop_gain_cw_q %x\n",
				phy_reg_read(pi, LCN40PHY_papd_loop_gain_cw_i),
				phy_reg_read(pi, LCN40PHY_papd_loop_gain_cw_q)));
			PHY_PAPD(("loopgain %d lgI %d lgQ %d\n", cwLpGn2, lgI, lgQ));

			if (cwLpGn2 < cwLpGn2_min) {
				rxGnInit = rxGnInit - bsStep;
			} else if (cwLpGn2 >= cwLpGn2_max) {
				rxGnInit = rxGnInit + bsStep;
			} else {
				break;
			}
		}
		bsStep = bsStep - 1;
		if (rxGnInit > 9)
			rxGnInit = 9;
		if (rxGnInit < 0)
			rxGnInit = 0; /* out-of-range correction */
	}

	pi_lcn->lcnphy_papdRxGnIdx = rxGnInit;
	PHY_PAPD(("wl%d: %s Settled to rxGnInit: %d\n",
		pi->sh->unit, __FUNCTION__, rxGnInit));
	return rxGnInit;
}

static void
wlc_lcn40phy_GetpapdMaxMinIdxupdt(phy_info_t *pi,
	int16 *maxUpdtIdx,
	int16 *minUpdtIdx)
{
	uint16 papd_lut_index_updt_63_48, papd_lut_index_updt_47_32;
	uint16 papd_lut_index_updt_31_16, papd_lut_index_updt_15_0;
	int8 MaxIdx, MinIdx;
	uint8 MaxIdxUpdated, MinIdxUpdated;
	uint8 i;

	papd_lut_index_updt_63_48 = phy_reg_read(pi, LCN40PHY_papd_lut_index_updated_63_48);
	papd_lut_index_updt_47_32 = phy_reg_read(pi, LCN40PHY_papd_lut_index_updated_47_32);
	papd_lut_index_updt_31_16 = phy_reg_read(pi, LCN40PHY_papd_lut_index_updated_31_16);
	papd_lut_index_updt_15_0  = phy_reg_read(pi, LCN40PHY_papd_lut_index_updated_15_0);

	PHY_PAPD(("63_48  47_32  31_16  15_0\n"));
	PHY_PAPD((" %4x   %4x   %4x   %4x\n",
		papd_lut_index_updt_63_48, papd_lut_index_updt_47_32,
		papd_lut_index_updt_31_16, papd_lut_index_updt_15_0));

	MaxIdx = 63;
	MinIdx = 0;
	MinIdxUpdated = 0;
	MaxIdxUpdated = 0;

	for (i = 0; i < 16 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_15_0 & (1 << i)) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
	}
	for (; i < 32 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_31_16 & (1 << (i - 16))) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
	}
	for (; i < 48 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_47_32 & (1 << (i - 32))) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
	}
	for (; i < 64 && MinIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_63_48 & (1 << (i - 48))) == 0) {
			if (MinIdxUpdated == 0)
				MinIdx = MinIdx + 1;
		} else {
			MinIdxUpdated = 1;
		}
}

	/* loop for getting max index updated */
	for (i = 0; i < 16 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_63_48 & (1 << (15 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	for (; i < 32 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_47_32 & (1 << (31 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	for (; i < 48 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_31_16 & (1 << (47 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	for (; i < 64 && MaxIdxUpdated == 0; i++) {
		if ((papd_lut_index_updt_15_0 & (1 << (63 - i))) == 0) {
			if (MaxIdxUpdated == 0)
				MaxIdx = MaxIdx - 1;
		} else {
			MaxIdxUpdated = 1;
		}
	}
	*maxUpdtIdx = MaxIdx;
	*minUpdtIdx = MinIdx;
}

static void
wlc_lcn40phy_save_papd_calibration_results(phy_info_t *pi)
{
	phytbl_info_t tab;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif

	/* Save epsilon table */
	tab.tbl_len = PHY_PAPD_EPS_TBL_SIZE_LCNPHY;
	tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;

#if defined(PHYCAL_CACHING)
	tab.tbl_ptr = cache->papd_eps_tbl;
	wlc_lcnphy_read_table(pi, &tab);

	cache->analog_gain_ref =
		phy_reg_read(pi, LCN40PHY_papd_tx_analog_gain_ref);
	cache->lut_begin =
		phy_reg_read(pi, LCN40PHY_papd_track_pa_lut_begin);
	cache->lut_step  =
		phy_reg_read(pi, LCN40PHY_papd_track_pa_lut_step);
	cache->lut_end	 =
		phy_reg_read(pi, LCN40PHY_papd_track_pa_lut_end);
	cache->rxcompdbm =
		phy_reg_read(pi, LCN40PHY_papd_rx_gain_comp_dbm);
	cache->papdctrl  =
		phy_reg_read(pi, LCN40PHY_papd_control);
	cache->sslpnCalibClkEnCtrl =
		phy_reg_read(pi, LCN40PHY_sslpnCalibClkEnCtrl);
#else
	tab.tbl_ptr = pi_lcn->lcnphy_cal_results.papd_eps_tbl;
	wlc_lcnphy_read_table(pi, &tab);

	pi_lcn->lcnphy_cal_results.analog_gain_ref =
		phy_reg_read(pi, LCN40PHY_papd_tx_analog_gain_ref);
	pi_lcn->lcnphy_cal_results.lut_begin =
		phy_reg_read(pi, LCN40PHY_papd_track_pa_lut_begin);
	pi_lcn->lcnphy_cal_results.lut_step  =
		phy_reg_read(pi, LCN40PHY_papd_track_pa_lut_step);
	pi_lcn->lcnphy_cal_results.lut_end   =
		phy_reg_read(pi, LCN40PHY_papd_track_pa_lut_end);
	pi_lcn->lcnphy_cal_results.rxcompdbm =
		phy_reg_read(pi, LCN40PHY_papd_rx_gain_comp_dbm);
	pi_lcn->lcnphy_cal_results.papdctrl  =
		phy_reg_read(pi, LCN40PHY_papd_control);
	pi_lcn->lcnphy_cal_results.sslpnCalibClkEnCtrl =
		phy_reg_read(pi, LCN40PHY_sslpnCalibClkEnCtrl);
#endif /* PHYCAL_CACHING */
}

static void
wlc_lcn40phy_papd_cal_core(
	phy_info_t *pi,
	phy_papd_cal_type_t calType,
	bool rxGnCtrl,
	bool txGnCtrl,
	bool samplecapture,
	bool papd_dbg_mode,
	uint16 num_symbols,
	bool init_papd_lut,
	uint16 papd_bbmult_init,
	uint16 papd_bbmult_step,
	bool papd_lpgn_ovr,
	uint16 LPGN_I,
	uint16 LPGN_Q)
{
	phytbl_info_t tab;
	uint32 papdcompdeltatbl_init_val;
	uint32 j;

	uint32 papd_buf_20mhz[] = {
	0x7fc00, 0x5a569, 0x1ff, 0xa5d69, 0x80400, 0xa5e97, 0x201, 0x5a697};
	uint32 papd_buf_40mhz[] = {
	0x7d000, 0x2fdce, 0xa7962, 0x8cb41, 0x0020c, 0x73b41, 0x58962, 0xd05ce,
	0x83000, 0xd0632, 0x58a9e, 0x738bf, 0x001f4, 0x8c8bf, 0xa7a9e, 0x2fe32};
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Reset PAPD Hw to reset register values */
	phy_reg_or(pi, LCN40PHY_papd_control2, 0x1);
	phy_reg_and(pi, LCN40PHY_papd_control2, ~0x1);
	PHY_REG_MOD(pi, LCN40PHY, papd_control2, papd_loop_gain_cw_ovr, papd_lpgn_ovr);

	/* set PAPD registers to configure the PAPD calibration */
	if (init_papd_lut != 0) {
		/* Load papd comp delta table */
		papdcompdeltatbl_init_val = 0x80000;
		tab.tbl_ptr = &papdcompdeltatbl_init_val; /* ptr to init var */
		tab.tbl_len = 1;        /* # values   */
		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;         /* papdcompdeltatbl */
		tab.tbl_width = 32;     /* 32 bit wide */
		if (PAPD2LUT == 1)
			tab.tbl_offset = 64; /* tbl offset */
		else
			tab.tbl_offset = 0; /* tbl offset */
		for (j = 0; j < 64; j ++) {
			wlc_lcn40phy_write_table(pi, &tab);
			tab.tbl_offset++;
		}
}

	/* set PAPD registers to configure PAPD calibration */
	wlc_lcn40phy_papd_cal_setup_cw(pi);

	/* num_symbols is computed based on corr_norm */
	num_symbols = num_symbols * (PAPD_BLANKING_PROFILE + 1);

	/* override control params */
	phy_reg_write(pi, LCN40PHY_papd_loop_gain_ovr_cw_i, LPGN_I);
	phy_reg_write(pi, LCN40PHY_papd_loop_gain_ovr_cw_q, LPGN_Q);

	/* papd update */
	phy_reg_write(pi, LCN40PHY_papd_track_num_symbols_count, num_symbols);

	/* spb parameters */
	phy_reg_write(pi, LCN40PHY_papd_spb_num_vld_symbols_n_dly, 0x60);
	phy_reg_write(pi, LCN40PHY_sampleLoopCount, (num_symbols+1)*20-1);
	phy_reg_write(pi, LCN40PHY_papd_spb_rd_address, 0);

	/* load the spb */
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		tab.tbl_len = 16;
		tab.tbl_ptr = &papd_buf_40mhz;
	} else {
		tab.tbl_len = 8;
		tab.tbl_ptr = &papd_buf_20mhz;
	}
	tab.tbl_id = LCN40PHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcn40phy_write_table(pi, &tab);

	/* 20MHz v.s. 40MHz block */
	if (CHSPEC_IS40(pi->radio_chanspec)) {
		phy_reg_write(pi, LCN40PHY_sampleDepthCount, 0xf);
		phy_reg_write(pi, LCN40PHY_papd_bbmult_init, 0x2ee);
		phy_reg_write(pi, LCN40PHY_papd_bbmult_step, papd_bbmult_step);
	} else {
		phy_reg_write(pi, LCN40PHY_sampleDepthCount, 0x7);
		phy_reg_write(pi, LCN40PHY_papd_bbmult_init, 0x3e8);
		phy_reg_write(pi, LCN40PHY_papd_bbmult_step, 0x40cd);
	}
	/* BBMULT parameters */
	phy_reg_write(pi, LCN40PHY_papd_bbmult_num_symbols, 1-1);
	/* papd variables copied from C/RTL setup 25000/regw_cmd.tcl */
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_step, 0x500);
	/* set bbmult to 0 to remove DC current spike after cal */
	wlc_lcn40phy_set_bbmult(pi, 0);
	/* Run PAPD HW Cal */
	/* pdbypass 0, gain0mode 1 */
	phy_reg_write(pi, LCN40PHY_papd_rx_sm_iqmm_gain_comp, 0x00);
	phy_reg_write(pi, LCN40PHY_papd_control, 0xb8a1);

	/* Wait for completion, around 1s */
	SPINWAIT(phy_reg_read(pi, LCN40PHY_papd_control) & LCN40PHY_papd_control_papd_cal_run_MASK,
	1 * 1000 * 1000);
}

static void
wlc_lcn40phy_papd_cal_setup_cw(
	phy_info_t *pi)
{
	uint16 papd_num_skip_count;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_REG_MOD(pi, LCN40PHY, papd_blanking_control, papd_stop_after_last_update, 0);
	/* Tune the hardware delay */
	if (pi_lcn->papd_corr_norm > 0) {
		if (pi_lcn->dacrate == 160)
			phy_reg_write(pi, LCN40PHY_papd_spb2papdin_dly, 0x23);
		else
			phy_reg_write(pi, LCN40PHY_papd_spb2papdin_dly, 0x30);
		if (pi_lcn->papd_corr_norm == 2)
			PHY_REG_MOD(pi, LCN40PHY, papd_blanking_control,
				papd_stop_after_last_update, 1);
	}
	else
		phy_reg_write(pi, LCN40PHY_papd_spb2papdin_dly, 0x23);
	/* Set samples/cycle/4 for q delay */
	phy_reg_write(pi, LCN40PHY_papd_variable_delay, 3);
	/* Set LUT begin gain, step gain, and size (Reset values, remove if possible) */
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_begin, 6700);
	phy_reg_write(pi, LCN40PHY_papd_rx_gain_comp_dbm, 0);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_step, 0x500);
	phy_reg_write(pi, LCN40PHY_papd_track_pa_lut_end, 0x3f);
	/* set papd constants */
	phy_reg_write(pi, LCN40PHY_papd_dbm_offset, 0x681);
	/* Dc estimation samples */
	phy_reg_write(pi, LCN40PHY_papd_ofdm_dc_est, 0x49);
	/* lcnphy - newly added registers */
	phy_reg_write(pi, LCN40PHY_papd_cw_corr_norm, pi_lcn->papd_corr_norm);
	phy_reg_write(pi, LCN40PHY_papd_blanking_control,
		(PAPD_STOP_AFTER_LAST_UPDATE << 12 | PAPD_BLANKING_PROFILE << 9 |
		PAPD_BLANKING_THRESHOLD));
	phy_reg_write(pi, LCN40PHY_papd_num_skip_count, 0x27);
	phy_reg_write(pi, LCN40PHY_papd_num_samples_count, 255);
	phy_reg_write(pi, LCN40PHY_papd_sync_count, 319);

	/* Adjust the number of samples to be processed depending on the corr_norm */
	phy_reg_write(pi, LCN40PHY_papd_num_samples_count,
		(((255+1)/(1<<(pi_lcn->papd_corr_norm)))-1));
	phy_reg_write(pi, LCN40PHY_papd_sync_count,
		(((319+1)/(1<<(pi_lcn->papd_corr_norm)))-1));

	papd_num_skip_count = PAPD_NUM_SKIP_COUNT;
	switch (pi_lcn->papd_corr_norm) {
		case 0:
			papd_num_skip_count = MIN(PAPD_NUM_SKIP_COUNT, 52);
			break;
		case 1:
			papd_num_skip_count = MIN(PAPD_NUM_SKIP_COUNT, 24);
			break;
		case 2:
			papd_num_skip_count = MIN(PAPD_NUM_SKIP_COUNT, 8);
			break;
	}
	phy_reg_write(pi, LCN40PHY_papd_num_skip_count, papd_num_skip_count);

	phy_reg_write(pi, LCN40PHY_papd_switch_lut, PAPD2LUT);
	phy_reg_write(pi, LCN40PHY_papd_pa_off_control_1, 0);
	phy_reg_write(pi, LCN40PHY_papd_only_loop_gain, 0);
	phy_reg_write(pi, LCN40PHY_papd_pa_off_control_2, 0);

	phy_reg_write(pi, LCN40PHY_smoothenLut_max_thr, 0x7ff);
	phy_reg_write(pi, LCN40PHY_papd_dcest_i_ovr, 0x0000);
	phy_reg_write(pi, LCN40PHY_papd_dcest_q_ovr, 0x0000);
}

/* force epa off */
static void
wlc_lcn40phy_epa_pd(phy_info_t *pi, bool disable)
{
	if (!disable) {
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr, swCtrl_ovr, 0);
	} else {
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr_val, swCtrl_ovr_val, 0);
		PHY_REG_MOD(pi, LCN40PHY, swctrlOvr, swCtrl_ovr, 0xff);
	}
}

/*
* Get Rx IQ Imbalance Estimate from modem
*/
static bool
wlc_lcn40phy_rx_iq_est(phy_info_t *pi,
	uint16 num_samps,
	uint8 wait_time,
	phy_iq_est_t *iq_est)
{
	int wait_count = 0;
	bool result = TRUE;
	uint8 phybw40;
	phybw40 = CHSPEC_IS40(pi->radio_chanspec);

	/* Turn on clk to Rx IQ */
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, iqEstClkEn, 1);
	/* Force OFDM receiver on */
	PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, APHYGatingEnable, 0);
	PHY_REG_MOD(pi, LCN40PHY, IQNumSampsAddress, numSamps, num_samps);
	PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, waittimevalue,
		(uint16)wait_time);
	PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, iqmode, 0);
	PHY_REG_MOD(pi, LCN40PHY, IQEnableWaitTimeAddress, iqstart, 1);

	/* Wait for IQ estimation to complete */
	while (phy_reg_read(pi, LCN40PHY_IQEnableWaitTimeAddress) &
		LCN40PHY_IQEnableWaitTimeAddress_iqstart_MASK) {
		/* Check for timeout */
		if (wait_count > (10 * 500)) { /* 500 ms */
			PHY_ERROR(("wl%d: %s: IQ estimation failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			result = FALSE;
			goto cleanup;
		}
		OSL_DELAY(100);
		wait_count++;
	}

	/* Save results */
	iq_est->iq_prod = ((uint32)phy_reg_read(pi, LCN40PHY_IQAccHiAddress) << 16) |
		(uint32)phy_reg_read(pi, LCN40PHY_IQAccLoAddress);
	iq_est->i_pwr = ((uint32)phy_reg_read(pi, LCN40PHY_IQIPWRAccHiAddress) << 16) |
		(uint32)phy_reg_read(pi, LCN40PHY_IQIPWRAccLoAddress);
	iq_est->q_pwr = ((uint32)phy_reg_read(pi, LCN40PHY_IQQPWRAccHiAddress) << 16) |
		(uint32)phy_reg_read(pi, LCN40PHY_IQQPWRAccLoAddress);
	PHY_TMP(("wl%d: %s: IQ estimation completed in %d us,"
		"i_pwr: %d, q_pwr: %d, iq_prod: %d\n",
		pi->sh->unit, __FUNCTION__,
		wait_count * 100, iq_est->i_pwr, iq_est->q_pwr, iq_est->iq_prod));

cleanup:
	PHY_REG_MOD(pi, LCN40PHY, crsgainCtrl, APHYGatingEnable, 1);
	PHY_REG_MOD(pi, LCN40PHY, sslpnCalibClkEnCtrl, iqEstClkEn, 0);
	return result;
}

/*
* Compute Rx compensation coeffs
*   -- run IQ est and calculate compensation coefficients
*/
static bool
wlc_lcn40phy_calc_rx_iq_comp(phy_info_t *pi,  uint16 num_samps)
{
#define LCN40PHY_MIN_RXIQ_PWR 2
	bool result;
	uint16 a0_new, b0_new;
	phy_iq_est_t iq_est = {0, 0, 0};
	int32  a, b, temp;
	int16  iq_nbits, qq_nbits, arsh, brsh;
	int32  iq;
	uint32 ii, qq;
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = &ctx->u.lcnphy_cache;
#else
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
#endif

	/* Save original c0 & c1 */
	a0_new = ((phy_reg_read(pi, LCN40PHY_RxCompcoeffa0) & LCN40PHY_RxCompcoeffa0_a0_MASK) >>
		LCN40PHY_RxCompcoeffa0_a0_SHIFT);
	b0_new = ((phy_reg_read(pi, LCN40PHY_RxCompcoeffb0) & LCN40PHY_RxCompcoeffb0_b0_MASK) >>
		LCN40PHY_RxCompcoeffb0_b0_SHIFT);

	PHY_REG_MOD(pi, LCN40PHY, rxfe, bypass_iqcomp, 0);
	PHY_REG_MOD(pi, LCN40PHY, RxIqCoeffCtrl, RxIqComp11bEn, 1);
	/* Zero out comp coeffs and do "one-shot" calibration */
	wlc_lcn40phy_set_rx_iq_comp(pi, 0, 0);

	if (!(result = wlc_lcn40phy_rx_iq_est(pi, num_samps, 32, &iq_est)))
		goto cleanup;

	iq = (int32)iq_est.iq_prod;
	ii = iq_est.i_pwr;
	qq = iq_est.q_pwr;

	/* bounds check estimate info */
	if ((ii + qq) < LCN40PHY_MIN_RXIQ_PWR) {
		PHY_ERROR(("wl%d: %s: RX IQ imbalance estimate power too small\n",
			pi->sh->unit, __FUNCTION__));
		result = FALSE;
		goto cleanup;
	}

	/* Calculate new coeffs */
	iq_nbits = wlc_phy_nbits(iq);
	qq_nbits = wlc_phy_nbits(qq);

	arsh = 10-(30-iq_nbits);
	if (arsh >= 0) {
		a = (-(iq << (30 - iq_nbits)) + (ii >> (1 + arsh)));
		temp = (int32) (ii >>  arsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
			return FALSE;
		}
	} else {
		a = (-(iq << (30 - iq_nbits)) + (ii << (-1 - arsh)));
		temp = (int32) (ii << -arsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, arsh=%d\n", ii, arsh));
			return FALSE;
		}
}
	a /= temp;
	brsh = qq_nbits-31+20;
	if (brsh >= 0) {
		b = (qq << (31-qq_nbits));
		temp = (int32) (ii >>  brsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
			return FALSE;
		}
	} else {
		b = (qq << (31-qq_nbits));
		temp = (int32) (ii << -brsh);
		if (temp == 0) {
			PHY_ERROR(("Aborting Rx IQCAL! ii=%d, brsh=%d\n", ii, brsh));
			return FALSE;
		}
	}
	b /= temp;
	b -= a*a;
	b = (int32)wlc_phy_sqrt_int((uint32) b);
	b -= (1 << 10);
	a0_new = (uint16)(a & 0x3ff);
	b0_new = (uint16)(b & 0x3ff);

cleanup:
	/* Apply new coeffs */
	wlc_lcn40phy_set_rx_iq_comp(pi, a0_new, b0_new);
	/* enabling the hardware override to choose only a0, b0 coeff */
	PHY_REG_MOD(pi, LCN40PHY, RxIqCoeffCtrl, RxIqCrsCoeffOverRide, 1);
	PHY_REG_MOD(pi, LCN40PHY, RxIqCoeffCtrl,
		RxIqCrsCoeffOverRide11b, 1);

#if defined(PHYCAL_CACHING)
	cache->rxiqcal_coeff_a0 = a0_new;
	cache->rxiqcal_coeff_b0 = b0_new;
#else
	pi_lcn->lcnphy_cal_results.rxiqcal_coeff_a0 = a0_new;
	pi_lcn->lcnphy_cal_results.rxiqcal_coeff_b0 = b0_new;
#endif

	return result;

}

/*
 * Play samples from sample play buffer
 */
static void
wlc_lcn40phy_run_samples(phy_info_t *pi,
	uint16 num_samps,
	uint16 num_loops,
	uint16 wait,
	bool iqcalmode)
{
	/* enable clk to txFrontEnd */
	phy_reg_or(pi, LCN40PHY_sslpnCalibClkEnCtrl, 0x8080);

	phy_reg_mod(pi, LCN40PHY_sampleDepthCount,
		LCN40PHY_sampleDepthCount_DepthCount_MASK,
		(num_samps - 1) << LCN40PHY_sampleDepthCount_DepthCount_SHIFT);
	if (num_loops != 0xffff)
		num_loops--;
	phy_reg_mod(pi, LCN40PHY_sampleLoopCount,
		LCN40PHY_sampleLoopCount_LoopCount_MASK,
		num_loops << LCN40PHY_sampleLoopCount_LoopCount_SHIFT);

	phy_reg_mod(pi, LCN40PHY_sampleInitWaitCount,
		LCN40PHY_sampleInitWaitCount_InitWaitCount_MASK,
		wait << LCN40PHY_sampleInitWaitCount_InitWaitCount_SHIFT);

	if (iqcalmode) {
		/* Enable calibration */
		phy_reg_and(pi,
			LCN40PHY_iqloCalCmdGctl,
			(uint16)~LCN40PHY_iqloCalCmdGctl_iqlo_cal_en_MASK);
		phy_reg_or(pi, LCN40PHY_iqloCalCmdGctl, LCN40PHY_iqloCalCmdGctl_iqlo_cal_en_MASK);
	} else {
		phy_reg_write(pi, LCN40PHY_sampleCmd, 1);
		wlc_lcn40phy_tx_pu(pi, 1);
	}
}

static uint16 wlc_lcn40phy_papd_index_search(phy_info_t *pi, uint64 final_idx_thresh,
	phy_txcalgains_t *txgains)
{
	uint16 start_index = 0;
	uint16 stop_index = 100;
	uint16 mid_index = 0;
	phytbl_info_t tab;
	uint32 lastval;
	int32 lreal, limag;
	uint64 mag;
	txgains->useindex = 1;

	while (1)
{
		mid_index = (start_index + stop_index) >> 1;
		txgains->index = (uint8) mid_index;
		/* run papd corresponding to the target pwr */
		wlc_lcn40phy_papd_cal(pi, PHY_PAPD_CAL_CW, txgains, 0, 0, 0, 0, 219, 1);

		tab.tbl_id = LCN40PHY_TBL_ID_PAPDCOMPDELTATBL;
		tab.tbl_offset = 63;
		tab.tbl_ptr = &lastval; /* ptr to buf */
		tab.tbl_width = 32;
		tab.tbl_len = 1;        /* # values   */
		wlc_lcnphy_read_table(pi, &tab);
		lreal = lastval & 0x00fff000;
		limag = lastval & 0x00000fff;
		lreal = lreal << 8;
		limag = limag << 20;
		lreal = lreal >> 20;
		limag = limag >> 20;

		mag = (lreal * lreal) + (limag * limag);
		if (mag <= final_idx_thresh)
			stop_index = mid_index;
		else
			start_index = mid_index;
		if ((mag > (final_idx_thresh - 2000)) && (mag < (final_idx_thresh)))
			break;
		if (stop_index - start_index < 2)
			break;
}
	return (mid_index);
}

static void wlc_lcn40phy_papd_calc_capindex(phy_info_t *pi, phy_txcalgains_t *txgains)
{
	uint8 channel;
	uint16 papdAmamThrs = 1680; /* 1.68 *1000 */
	uint16 papdAmamSlope = 10; /* 0.01 *1000 */
	uint16 papdSlopeDivFreq = (papdAmamSlope)/5; /* 2 */
	uint16 lcnphytxindex = 0;
	uint32 Threshold = 0;
	uint32 temp = 0;
	uint64 final_idx_thresh = 0;
	uint freq;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);
	freq = wlc_phy_channel2freq(CHSPEC_CHANNEL(pi->radio_chanspec));
	channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	/* Papd Based Index Capping calculation */
	if (freq == 2484) {
		Threshold = (128 * (papdAmamThrs + (14 * papdAmamSlope)));
}
	else {
		if (channel == 1)
			Threshold = (128 * papdAmamThrs);
		else
			Threshold = (128 * (papdAmamThrs + ((freq - 2412) * papdSlopeDivFreq)));
	}
	temp =  wlc_phy_qdiv_roundup(Threshold, 1000, 0);
	final_idx_thresh = ((uint64)temp*temp);
	lcnphytxindex = wlc_lcn40phy_papd_index_search(pi, final_idx_thresh, txgains);
	if ((lcnphytxindex < 40) || (lcnphytxindex >= 70))
{
		if (lcnphytxindex < 40) {
			Threshold = (128 * (papdAmamThrs -240));
		}
		else {
			if (freq == 2484)
				Threshold = (128 * (papdAmamThrs + 100 + 14*papdAmamSlope));
			else
				Threshold = (128 * (papdAmamThrs + 100 +
					((freq - 2412) * papdSlopeDivFreq)));
}
		temp =  wlc_phy_qdiv_roundup(Threshold, 1000, 0);
		final_idx_thresh = ((uint64)temp*temp);
		lcnphytxindex = wlc_lcn40phy_papd_index_search(pi, final_idx_thresh, txgains);

}
	/* cap it to 1dB higher pwr as headroom  */
	pi_lcn->lcnphy_capped_index = lcnphytxindex - 4;

}

static void wlc_lcn40phy_load_txgainwithcappedindex(phy_info_t *pi, bool cap)
{
	uint8 k;
	phytbl_info_t tab;
	uint32 val;
	uint8 indx;
	phy_info_lcnphy_t *pi_lcn = wlc_phy_getlcnphy_common(pi);

	if (cap)
{
		k =  pi_lcn->lcnphy_capped_index;
		tab.tbl_id = LCN40PHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;     /* 32 bit wide  */
		tab.tbl_len = 1;
		tab.tbl_ptr = &val; /* ptr to buf */

		for (indx = 0; indx <=  pi_lcn->lcnphy_capped_index; indx++) {
			/* Cap the GainOffset table */
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + k;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_GAIN_OFFSET + indx;
			wlc_lcn40phy_write_table(pi, &tab);

			/* Cap the IQOffset table */
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + k;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_IQ_OFFSET + indx;
			wlc_lcn40phy_write_table(pi, &tab);

			/* Cap the RF PWR offset table */
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + k;
			wlc_lcn40phy_read_table(pi, &tab);
			tab.tbl_offset = LCN40PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
			wlc_lcn40phy_write_table(pi, &tab);
}

}
	else
	{

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (LCN40REV_IS(pi->pubpi.phy_rev, 0) ||
				LCN40REV_IS(pi->pubpi.phy_rev, 2) ||
				LCN40REV_IS(pi->pubpi.phy_rev, 4))
				wlc_lcn40phy_load_tx_gain_table(pi,
				        dot11lcn40phy_2GHz_gaintable_rev0);
			else if (LCN40REV_IS(pi->pubpi.phy_rev, 1)) {
				if (pi_lcn->ePA) {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_extPA_gaintable_rev1);
				}
				else {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_gaintable_rev1);
				}
			}
			else if (LCN40REV_IS(pi->pubpi.phy_rev, 3)) {
				if (pi_lcn->ePA) {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_extPA_gaintable_rev1);
				}
				else {
					wlc_lcn40phy_load_tx_gain_table(pi,
					        dot11lcn40phy_2GHz_gaintable_rev3);
				}
			}
		}
		wlc_lcn40phy_load_rfpower(pi);
	}
}

static void
wlc_lcn40phy_adc_init(phy_info_t *pi, phy_adc_mode_t adc_mode, bool cal_mode)
{
	or_radio_reg(pi, RADIO_2065_ADC_CFG3, 0x2000);

	phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1Val,
		LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_reset_ovr_val_MASK |
		LCN40PHY_AfeCtrlOvr1Val_afe_reset_ov_det_ovr_val_MASK);

	phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
		LCN40PHY_AfeCtrlOvr1_afe_iqadc_reset_ovr_MASK |
		LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK);

	OSL_DELAY(100);
	phy_reg_and(pi, LCN40PHY_AfeCtrlOvr1Val,
		~(LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_reset_ovr_val_MASK |
		LCN40PHY_AfeCtrlOvr1Val_afe_reset_ov_det_ovr_val_MASK));

	switch (adc_mode) {
	case ADC_20M:
		mod_radio_reg(pi, RADIO_2065_ADC_CFG4, 0x400, 0);
		write_radio_reg(pi, RADIO_2065_ADC_BIAS, 2);
		phy_reg_mod(pi, LCN40PHY_AfeCtrlOvr1Val,
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
			0x1f << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT);
		phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK);
		break;
	case ADC_40M:
		or_radio_reg(pi, RADIO_2065_ADC_CFG4, 0x400);
		write_radio_reg(pi, RADIO_2065_ADC_BIAS, 0);
		phy_reg_mod(pi, LCN40PHY_AfeCtrlOvr1Val,
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
			0x1f << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT);
		phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK);
		break;
	case ADC_20M_LP:
		mod_radio_reg(pi, RADIO_2065_ADC_CFG4, 0x400, 0);
		write_radio_reg(pi, RADIO_2065_ADC_BIAS, 2);
		phy_reg_mod(pi, LCN40PHY_AfeCtrlOvr1Val,
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
			(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_SHIFT) |
			(0x19 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT));
		phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK);
		break;
	case ADC_40M_LP:
		or_radio_reg(pi, RADIO_2065_ADC_CFG4, 0x400);
		write_radio_reg(pi, RADIO_2065_ADC_BIAS, 0);
		phy_reg_mod(pi, LCN40PHY_AfeCtrlOvr1Val,
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
			(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_SHIFT) |
			(0x19 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT));
		phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK);
		break;
	case ADC_FLASHONLY:
		or_radio_reg(pi, RADIO_2065_ADC_CFG4, 0x400);
		write_radio_reg(pi, RADIO_2065_ADC_BIAS, 0);
		phy_reg_mod(pi, LCN40PHY_AfeCtrlOvr1Val,
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_order_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_lf_lowif_dis_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_wl_lp_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_MASK |
			LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_MASK,
			(1 << LCN40PHY_AfeCtrlOvr1Val_afe_iqadc_flash_only_ovr_val_SHIFT) |
			(1 << LCN40PHY_AfeCtrlOvr1Val_adc_pu_ovr_val_SHIFT));
		phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_order_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_lf_lowif_dis_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_wl_lp_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_afe_iqadc_flash_only_ovr_MASK |
			LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK);
		break;
	default:
		PHY_ERROR(("wrong ADC mode %d\n", adc_mode));
	}

	if (cal_mode) {
		or_radio_reg(pi, RADIO_2065_XTAL_CFG1, 0x4000);
		or_radio_reg(pi, RADIO_2065_ADC_CFG3, 0x1000);
		mod_radio_reg(pi, RADIO_2065_ADC_CFG4, 0xff, 0x5f);
		or_radio_reg(pi, RADIO_2065_OVR2, 0x2400);
		OSL_DELAY(300);
		mod_radio_reg(pi, RADIO_2065_ADC_CFG4, 0xff, 0);
		and_radio_reg(pi, RADIO_2065_OVR2, ~0x2400);
		and_radio_reg(pi, RADIO_2065_XTAL_CFG1, ~0x4000);
		phy_reg_or(pi, LCN40PHY_AfeCtrlOvr1,
			LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK);
		OSL_DELAY(100);
		phy_reg_and(pi, LCN40PHY_AfeCtrlOvr1,
			~LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK);
	} else {
		and_radio_reg(pi, RADIO_2065_ADC_CFG3, ~0x1000);
		mod_radio_reg(pi, RADIO_2065_ADC_CFG4, 0x0c, 0);
	}

	phy_reg_and(pi, LCN40PHY_AfeCtrlOvr1,
		~(LCN40PHY_AfeCtrlOvr1_afe_iqadc_reset_ovr_MASK |
		LCN40PHY_AfeCtrlOvr1_afe_reset_ov_det_ovr_MASK |
		LCN40PHY_AfeCtrlOvr1_adc_pu_ovr_MASK));
}

#ifdef WIP
static void
wlc_lcn40phy_vbat_temp_sense_setup(phy_info_t *pi, uint8 mode)
{
}

static int16
wlc_lcn40phy_temp_sense_vbatTemp_on(phy_info_t *pi)
{
	return 0;
}

/* ********************** NOISE CAL ********************************* */

static uint16 wlc_lcn40phy_noise_get_reg(phy_info_t *pi, uint16 reg);
static int8 wlc_lcn40phy_noise_log(uint32 x);

#if PHY_NOISE_DBG_HISTORY > 0
static void
wlc_lcn40phy_noise_log_init(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_start(phy_info_t *pi)
{
}


static void
wlc_lcn40phy_noise_log_adj(phy_info_t *pi, uint16 noise, int16 adj,
	int16 gain, int16 po, bool gain_change, bool po_change)
{
}

static void
wlc_lcn40phy_noise_log_callback(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_tainted(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_ucode_data_reset(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_ucode_data_ok(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_ucode_data(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_bad_ucode_data(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_ucode_data_insert_time(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_log_state(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_reset_log(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_advance_log(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_dump_log(phy_info_t *pi)
{
}

static bool
wlc_lcn40phy_noise_log_dump_active(phy_info_t *pi)
{
	return 0;
}

static uint32
wlc_lcn40phy_noise_log_data(phy_info_t *pi)
{
	return 0;
}

static bool
wlc_lcn40phy_noise_log_ioctl(phy_info_t *pi, uint32 flag)
{
	return 0;
}

/*
*********************************************
*Call these 2 functions from wlc_bmac_pkteng()
*to log when per test starts v noise cal
********************************************
*/
extern void wlc_lcn40phy_noise_per_start(wlc_phy_t *ppi);
extern void wlc_lcn40phy_noise_per_stop(wlc_phy_t *pih);

void wlc_lcn40phy_noise_per_start(wlc_phy_t *pih)
{
}

void wlc_lcn40phy_noise_per_stop(wlc_phy_t *pih)
{
}

#else /* PHY_NOISE_DBG_HISTORY > 0 */

#define wlc_lcn40phy_noise_log_init(a)
#define wlc_lcn40phy_noise_log_start(a)
#define wlc_lcn40phy_noise_log_ucode_data_ok(a)
#define wlc_lcn40phy_noise_log_adj(a, b, c, d, e, f, g)
#define wlc_lcn40phy_noise_log_callback(a)
#define wlc_lcn40phy_noise_log_tainted(a)
#define wlc_lcn40phy_noise_log_ucode_data_reset(a)
#define wlc_lcn40phy_noise_log_ucode_data(a)
#define wlc_lcn40phy_noise_log_bad_ucode_data(a)
#define wlc_lcn40phy_noise_log_ucode_data_insert_time(a)
#define wlc_lcn40phy_noise_log_state(a)
#define wlc_lcn40phy_noise_reset_log(a)
#define wlc_lcn40phy_noise_advance_log(a)
#define wlc_lcn40phy_noise_dump_log(a)
#define wlc_lcn40phy_noise_log_dump_active(a) FALSE
#define wlc_lcn40phy_noise_log_data(a) 0
#define wlc_lcn40phy_noise_log_ioctl(a, b) ((b))

#endif /* PHY_NOISE_DBG_HISTORY > 0 */

static uint16
wlc_lcn40phy_noise_get_reg(phy_info_t *pi, uint16 reg)
{
}

static void
wlc_lcn40phy_noise_set_reg(phy_info_t *pi, uint16 reg, uint16 val)
{
}


static bool
wlc_lcn40phy_noise_sync_ucode(phy_info_t *pi, int timeout, int* p_timeout)
{
	return 0;
}

static void
wlc_lcn40phy_noise_ucode_ctrl(phy_info_t *pi, bool enable)
{
}


static void
wlc_lcn40phy_noise_save_phy_regs(phy_info_t *pi)
{
}

static void
wlc_lcn40phy_noise_restore_phy_regs(phy_info_t *pi)
{
}

static int8
wlc_lcn40phy_noise_log(uint32 x)
{
	return 0;
}

static bool
wlc_lcn40phy_noise_adj(phy_info_t *pi, uint32 metric)
{
	return 0;
}


static bool wlc_lcn40phy_noise_metric(phy_info_t* pi, uint32* metric, uint32* power)
{
	return TRUE;
}


static void
wlc_lcn40phy_noise_reset(phy_info_t *pi, bool restore_regs)
{
}

static void
wlc_lcn40phy_noise_reset_data(phy_info_t *pi)
{

}

static void
wlc_lcn40phy_noise_cb(phy_info_t *pi, uint32 metric)
{
}

#if defined(PHYCAL_CACHING) && defined(BCMDBG)
static void
wlc_phy_cal_cache_dbg_lcn40phy(ch_calcache_t *ctx)
{
}
#endif /* PHYCAL_CACHING && BCMDBG */

#endif /* WIP */
