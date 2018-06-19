/*
 * PHY module internal interface crossing different PHY types
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_phy_int.h 279526 2011-08-24 21:50:30Z caldana $
 */

#ifndef _wlc_phy_int_h_
#define _wlc_phy_int_h_

#include <typedefs.h>
#include <bcmwifi.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <siutils.h>

#include <bcmsrom_fmt.h>
#include <wlc_phy_hal.h>

#define PHYHAL_ERROR	0x0001
#define PHYHAL_TRACE	0x0002
#define PHYHAL_INFORM	0x0004
#define PHYHAL_TMP	0x0008
#define PHYHAL_TXPWR	0x0010
#define PHYHAL_CAL	0x0020
#define PHYHAL_ACI	0x0040
#define PHYHAL_RADAR	0x0080
#define PHYHAL_THERMAL	0x0100
#define PHYHAL_PAPD     0x0200
#define PHYHAL_FCBS     0x0400

extern uint32 phyhal_msg_level;

#ifdef BCMDBG
#define	PHY_ERROR(args)		do {if (phyhal_msg_level & PHYHAL_ERROR) printf args;} while (0)
#define	PHY_TRACE(args)		do {if (phyhal_msg_level & PHYHAL_TRACE) printf args;} while (0)
#define	PHY_INFORM(args)	do {if (phyhal_msg_level & PHYHAL_INFORM) printf args;} while (0)
#define	PHY_TMP(args)		do {if (phyhal_msg_level & PHYHAL_TMP) printf args;} while (0)
#define	PHY_TXPWR(args)		do {if (phyhal_msg_level & PHYHAL_TXPWR) printf args;} while (0)
#define	PHY_CAL(args)		do {if (phyhal_msg_level & PHYHAL_CAL) printf args;} while (0)
#define	PHY_ACI(args)		do {if (phyhal_msg_level & PHYHAL_ACI) printf args;} while (0)
#define	PHY_RADAR(args)		do {if (phyhal_msg_level & PHYHAL_RADAR) printf args;} while (0)
#define PHY_THERMAL(args)	do {if (phyhal_msg_level & PHYHAL_THERMAL) printf args;} while (0)
#define PHY_PAPD(args)          do {if (phyhal_msg_level & PHYHAL_PAPD) printf args;} while (0)
#define PHY_FCBS(args)          do {if (phyhal_msg_level & PHYHAL_FCBS) printf args;} while (0)

#define	PHY_NONE(args)		do {} while (0)
#else
#define	PHY_ERROR(args)
#define	PHY_TRACE(args)
#define	PHY_INFORM(args)
#define	PHY_TMP(args)
#define	PHY_TXPWR(args)
#define	PHY_CAL(args)
#define	PHY_ACI(args)
#define	PHY_RADAR(args)
#define PHY_THERMAL(args)
#define PHY_PAPD(args)
#define PHY_FCBS(args)
#define	PHY_NONE(args)
#endif /* BCMDBG */

#define PHY_INFORM_ON()		(phyhal_msg_level & PHYHAL_INFORM)
#define PHY_THERMAL_ON()	(phyhal_msg_level & PHYHAL_THERMAL)
#define PHY_CAL_ON()		(phyhal_msg_level & PHYHAL_CAL)

#ifdef BOARD_TYPE
#define BOARDTYPE(_type) BOARD_TYPE
#else
#define BOARDTYPE(_type) _type
#endif

#define LCNXN_BASEREV		16

/* #define ENABLE_FCBS */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  inter-module connection					*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* forward declarations */
struct wlc_hw_info;
typedef struct phy_info phy_info_t;
typedef struct phytbl_info phytbl_info_t;

typedef void (*initfn_t)(phy_info_t *);
typedef void (*chansetfn_t)(phy_info_t *, chanspec_t);
typedef int (*longtrnfn_t)(phy_info_t *, int);
typedef void (*txiqccgetfn_t)(phy_info_t *, uint16 *, uint16 *);
typedef void (*txiqccsetfn_t)(phy_info_t *, uint16, uint16);
typedef uint16 (*txloccgetfn_t)(phy_info_t *);
typedef void (*txloccsetfn_t)(phy_info_t *pi, uint16 didq);
typedef void (*radioloftgetfn_t)(phy_info_t *, uint8 *, uint8 *, uint8 *, uint8 *);
typedef void (*radioloftsetfn_t)(phy_info_t *, uint8, uint8, uint8, uint8);
typedef int32 (*rxsigpwrfn_t)(phy_info_t *, int32);
typedef void (*detachfn_t)(phy_info_t *);
typedef int (*txcorepwroffsetfn_t)(phy_info_t *, struct phy_txcore_pwr_offsets*);
typedef void (*settxpwrctrlfn_t)(phy_info_t *, uint16);
typedef uint16 (*gettxpwrctrlfn_t)(phy_info_t *);
typedef void (*settxpwrbyindexfn_t)(phy_info_t *, int);
typedef bool (*ishwtxpwrctrlfn_t)(phy_info_t *);
typedef void (*phywatchdogfn_t)(phy_info_t *);
typedef void (*btcadjustfn_t)(phy_info_t *, bool);
typedef uint16 (*tssicalsweepfn_t)(phy_info_t *, int8 *, uint8 *);
typedef void (*switchradiofn_t)(phy_info_t *, bool);
typedef void (*anacorefn_t)(phy_info_t *, bool);
typedef void (*phywritetablefn_t)(phy_info_t *pi, const phytbl_info_t *pti);
typedef void (*phyreadtablefn_t)(phy_info_t *pi, phytbl_info_t *pti);
typedef void (*calibmodesfn_t)(phy_info_t *pi, uint mode);

#ifdef ENABLE_FCBS
typedef bool (*fcbsinitfn_t)(phy_info_t *pi, int chanidx, chanspec_t chanspec);
typedef bool (*fcbsprefn_t)(phy_info_t *pi, int chanidx);
typedef bool (*fcbspostfn_t)(phy_info_t *pi, int chanidx);
#endif /* ENABLE_FCBS */

/* redefine some wlc_cfg.h macros to take the internal phy_info_t instead of wlc_phy_t */
#undef ISAPHY
#undef ISGPHY
#undef ISNPHY
#undef ISLPPHY
#undef ISSSLPNPHY
#undef ISLCNPHY
#undef ISLCN40PHY
#undef ISHTPHY
#undef ISLCNCOMMONPHY
#define ISAPHY(pi)	PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_A)
#define ISGPHY(pi)	PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_G)
#define ISNPHY(pi)	PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_N)
#define ISLPPHY(pi)	PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_LP)
#define ISSSLPNPHY(pi)  PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_SSN)
#define ISLCNPHY(pi)  	PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_LCN)
#define ISLCN40PHY(pi) 	PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_LCN40)
#define ISHTPHY(pi)  	PHYTYPE_IS((pi)->pubpi.phy_type, PHY_TYPE_HT)

#define ISABGPHY(pi)	(ISAPHY(pi) || ISGPHY(pi))
#define ISLPSSNPHY(pi)	(ISLPPHY(pi) || ISSSLPNPHY(pi))

#define ISPHY_11N_CAP(pi)	(ISNPHY(pi) || ISSSLPNPHY(pi) || ISLCNPHY(pi) || ISHTPHY(pi) ||\
				ISLCN40PHY(pi))
#define ISLCNCOMMONPHY(pi)    (ISLCNPHY(pi) || ISLCN40PHY(pi))

#define IS20MHZ(pi)	((pi)->bw == WL_CHANSPEC_BW_20)
#define IS40MHZ(pi)	((pi)->bw == WL_CHANSPEC_BW_40)


/* defines to optimize the code size */
#ifdef BCMRADIOREV
#define RADIOREV(rev)	BCMRADIOREV
#else /* BCMRADIOREV */
#define RADIOREV(rev)	(rev)
#endif /* BCMRADIOREV */

#ifdef BCMRADIOVER
#define RADIOVER(ver)	BCMRADIOVER
#else /* BCMRADIOVER */
#define RADIOVER(ver)	(ver)
#endif /* BCMRADIOVER */

#ifdef BCMRADIOID
#define RADIOID(id)	BCMRADIOID
#else /* BCMRADIOID */
#define RADIOID(id)	(id)
#endif /* BCMRADIOID */

#ifdef XTAL_FREQ
#define PHY_XTALFREQ(_freq)	XTAL_FREQ
#else
#define PHY_XTALFREQ(_freq)	(_freq)
#endif

#define GENERIC_PHY_INFO(pi)	((pi)->sh)
#ifdef BOARD_FLAGS
#define BOARDFLAGS(flag)	(BOARD_FLAGS)
#else
#define BOARDFLAGS(flag)	(flag)
#endif

#ifdef BOARD_FLAGS2
#define BOARDFLAGS2(flag)	(BOARD_FLAGS2)
#else
#define BOARDFLAGS2(flag)	(flag)
#endif

#define IS_X12_BOARDTYPE(pi) ((pi->sh->boardtype == BCM94331PCIEDUAL_SSID) || \
			      (pi->sh->boardtype == BCM94331X12_2G_SSID) || \
			      (pi->sh->boardtype == BCM94331X12_5G_SSID))

#define IS_X28_BOARDTYPE(pi) ((CHIPID(pi->sh->chip) == BCM4331_CHIP_ID) && \
			      (pi->sh->boardtype == BCM94331PCIEBT3Ax_SSID))

#define IS_X29_BOARDTYPE(pi) (((CHIPID(pi->sh->chip) == BCM4331_CHIP_ID) || \
			       (CHIPID(pi->sh->chip) == BCM43431_CHIP_ID)) && \
			      ((pi->sh->boardtype == BCM94331CS_SSID) || \
			       (pi->sh->boardtype == BCM94331CSAX_SSID)))

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  macro, typedef, enum, structure, global variable		*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* %%%%%% shared */
#define PHY_GET_RFATTN(rfgain)	((rfgain) & 0x0f)
#define PHY_GET_PADMIX(rfgain)	(((rfgain) & 0x10) >> 4)
#define PHY_GET_RFGAINID(rfattn, padmix, width)	((rfattn) + ((padmix)*(width)))
#define PHY_SAT(x, n)		((x) > ((1<<((n)-1))-1) ? ((1<<((n)-1))-1) : \
				((x) < -(1<<((n)-1)) ? -(1<<((n)-1)) : (x)))
#define PHY_SHIFT_ROUND(x, n)	((x) >= 0 ? ((x)+(1<<((n)-1)))>>(n) : (x)>>(n))
#define PHY_HW_ROUND(x, s)		((x >> s) + ((x >> (s-1)) & (s != 0)))

/* channels */
#define CH_5G_GROUP	3	/* A band, channel groups: low, mid, high in A band */
#define A_LOW_CHANS	0	/* Index for low channels in A band */
#define A_MID_CHANS	1	/* Index for mid channels in A band */
#define A_HIGH_CHANS	2	/* Index for high channels in A band */
#define CH_2G_GROUP	1	/* B band, channel groups, just one */
#define G_ALL_CHANS	0	/* Index for all channels in G band */

#define FIRST_REF5_CHANNUM	149	/* Lower bound of disable channel-range for srom rev 1 */
#define LAST_REF5_CHANNUM	165	/* Upper bound of disable channel-range for srom rev 1 */
#define	FIRST_5G_CHAN		14	/* First allowed channel index for 5G band */
#define	LAST_5G_CHAN		50	/* Last allowed channel for 5G band */
#define	FIRST_MID_5G_CHAN	14	/* Lower bound of channel for using m_tssi_to_dbm */
#define	LAST_MID_5G_CHAN	35	/* Upper bound of channel for using m_tssi_to_dbm */
#define	FIRST_HIGH_5G_CHAN	36	/* Lower bound of channel for using h_tssi_to_dbm */
#define	LAST_HIGH_5G_CHAN	41	/* Upper bound of channel for using h_tssi_to_dbm */
#define	FIRST_LOW_5G_CHAN	42	/* Lower bound of channel for using l_tssi_to_dbm */
#define	LAST_LOW_5G_CHAN	50	/* Upper bound of channel for using l_tssi_to_dbm */

/* SSLPNPHY has different sub-band range limts for the A-band compared to MIMOPHY
 * (see sslpnphy_get_paparams in sslpnphyprocs.tcl)
 */
#define FIRST_LOW_5G_CHAN_SSLPNPHY      34
#define LAST_LOW_5G_CHAN_SSLPNPHY       64
#define FIRST_MID_5G_CHAN_SSLPNPHY      100
#define LAST_MID_5G_CHAN_SSLPNPHY       140
#define FIRST_HIGH_5G_CHAN_SSLPNPHY     149
#define LAST_HIGH_5G_CHAN_SSLPNPHY      165


#define PHY_SUBBAND_3BAND_EMBDDED	0
#define PHY_SUBBAND_3BAND_HIGHPWR	1
#define PHY_SUBBAND_5BAND		2
#define PHY_SUBBAND_3BAND_JAPAN		7

#define JAPAN_LOW_5G_CHAN	4900
#define JAPAN_MID_5G_CHAN	5100
#define JAPAN_HIGH_5G_CHAN	5500

#define EMBEDDED_LOW_5G_CHAN	5170
#define EMBEDDED_MID_5G_CHAN	5500
#define EMBEDDED_HIGH_5G_CHAN	5745

#define HIGHPWR_LOW_5G_CHAN	5170
#define HIGHPWR_MID_5G_CHAN	5250
#define HIGHPWR_HIGH_5G_CHAN	5745

#define	MAX_5G_CHANS_PHY5GSB_HIGHPWR_5B	37	/* Max 5G CHs */
#define HIGHPWR_5B_LOW_LO_5G_CHAN	5180	/* new starting frequency of 5G_LOW-LO */
#define HIGHPWR_5B_LOW_HI_5G_CHAN	5260	/* new starting frequency of 5G_LOW-HI */
#define HIGHPWR_5B_MID_LO_5G_CHAN	5500	/* new starting frequency of 5G_MID-LO */
#define HIGHPWR_5B_MID_HI_5G_CHAN	5620	/* new starting frequency of 5G_MID-HI */
#define HIGHPWR_5B_HIGH_5G_CHAN		5745	/* new starting frequency of 5G_HI */

#ifdef	WL_PPR_SUBBAND
#define	PHY_SUBBAND_NUM		5	/* Ext band_num */
#else
#define	PHY_SUBBAND_NUM		0	/* Ext band_num */
#endif

#define CHAN5G_FREQ(chan)  (5000 + chan*5)
#define CHAN2G_FREQ(chan)  (2407 + chan*5)

/* power per rate array index */
#define CCK_20_PO		0
#define CCK_20UL_PO		1
#define OFDM_20_PO		2
#define OFDM_20UL_PO		3
#define OFDM_40DUP_PO		4
#define MCS_20_PO		5
#define MCS_20UL_PO		6
#define MCS_40_PO		7
#define MCS32_PO		8
#define PWR_OFFSET_SIZE		9

/* power rate */
#define TXP_FIRST_CCK		0	/* Index for first CCK rate */
#define TXP_LAST_CCK		3	/* Index for last CCK rate */
#define TXP_FIRST_OFDM	        4	/* Index for first 20MHz OFDM SISO rate */
#define TXP_LAST_OFDM	        11	/* Index for last 20MHz OFDM SISO rate */
#define TXP_FIRST_MCS_20_SS 12	/* Index for first 20MHz MCS SISO rate single stream */
#define TXP_LAST_MCS_20_SISO_SS  19	/* Index for last 20MHz MCS SISO rate single stream */
#define TXP_FIRST_MCS_20_CDD_SS 20 /* Index for first MCS_CDD at 20 MHz single stream */
#define TXP_LAST_MCS_20_SS		27	/* Index for last MCS at 20 MHz single stream */
#define TXP_FIRST_MCS_40_SS	28	/* Index for first MCS at 40 MHz single stream */
#define TXP_LAST_MCS_40_SISO_SS	35	/* Index for last SISO MCS at 40 MHz single stream */
#define TXP_FIRST_MCS_40_SISO_CDD_SS  36 /* Index for first MCS_CDD at 40 MHz single stream */
#define TXP_LAST_MCS_40_SS		44	/* Index for last MCS at 40 MHz single stream */
#define TXP_FIRST_OFDM_20_CDD   12	/* Index for first 20MHz OFDM CDD rate */
#define TXP_LAST_OFDM_20_CDD    19	/* Index for last 20 MHz OFDM CDD rate */
#define TXP_FIRST_MCS_20_SISO   20	/* Index for first 20MHz MCS SISO rate */
#define TXP_LAST_MCS_20_SISO    27	/* Index for last 20MHz MCS SISO rate */
#define TXP_FIRST_MCS_20_CDD    28	/* Index for first 20MHz MCS CDD rate */
#define TXP_LAST_MCS_20_CDD     35	/* Index for last 20MHz MCS CDD rate */
#define TXP_FIRST_MCS_20_STBC   36	/* Index for first 20MHz MCS STBC rate */
#define TXP_LAST_MCS_20_STBC    43	/* Index for last 20MHz MCS STBC rate */
#define TXP_FIRST_MCS_20_SDM    44	/* Index for first 20MHz MCS SDM rate */
#define TXP_LAST_MCS_20_SDM     51	/* Index for last 20MHz MCS SDM rate */
#define TXP_FIRST_OFDM_40_SISO  52	/* Index for first 40MHz OFDM SISO rate */
#define TXP_LAST_OFDM_40_SISO   59	/* Index for last 40MHz OFDM SISO rate */
#define TXP_FIRST_OFDM_40_CDD   60	/* Index for first 40MHz OFDM CDD rate */
#define TXP_LAST_OFDM_40_CDD    67	/* Index for last 40 MHz OFDM CDD rate */
#define TXP_FIRST_MCS_40_SISO   68	/* Index for first 40MHz MCS SISO rate */
#define TXP_LAST_MCS_40_SISO    75	/* Index for last 40MHz MCS SISO rate */
#define TXP_FIRST_MCS_40_CDD	76	/* Index for first 40MHz MCS CDD rate */
#define TXP_LAST_MCS_40_CDD	83	/* Index for last 40MHz MCS CDD rate */
#define TXP_FIRST_MCS_40_STBC	84	/* Index for first 40MHz MCS STBC rate */
#define TXP_LAST_MCS_40_STBC	91	/* Index for last 40MHz MCS STBC rate */
#define TXP_FIRST_MCS_40_SDM	92	/* Index for first 40MHz MCS SDM rate */
#define TXP_LAST_MCS_40_SDM	99	/* Index for last 40MHz MCS SDM rate */
#define TXP_MCS_32	        100	/* Index for 40MHz rate MCS 32 */
#define TXP_NUM_RATES_MIMO	101	/* number of rates in MIMO mode */
#define TXP_FIRST_20UL_CCK      101	/* Index for first 20in40MHz CCK rate */
#define TXP_LAST_20UL_CCK	104	/* Index for last 20in40MHz CCK rate */
#define TXP_FIRST_20UL_OFDM     105	/* Index for first 20in40MHz OFDM rate */
#define TXP_LAST_20UL_OFDM	112	/* Index for last 20in40MHz OFDM rate */
#define TXP_FIRST_20UL_OFDM_CDD	113	/* Index for first 20in40MHz OFDM rate */
#define TXP_LAST_20UL_OFDM_CDD	120	/* Index for last 20in40MHz OFDM rate */
#define TXP_FIRST_20UL_S1x1     121	/* Index for first 20in40MHz MCS rate */
#define TXP_LAST_20UL_S1x1	128	/* Index for last 20in40MHz MCS rate */
#define TXP_FIRST_20UL_S1x2     129	/* Index for first 20in40MHz MCS rate */
#define TXP_LAST_20UL_S1x2	136	/* Index for last 20in40MHz MCS rate */
#define TXP_FIRST_20UL_S2x2     137	/* Index for first 20in40MHz MCS rate */
#define TXP_LAST_20UL_S2x2	144	/* Index for last 20in40MHz MCS rate */
#define TXP_FIRST_20UL_S3x3     145	/* Index for first 20in40MHz MCS rate */
#define TXP_LAST_20UL_S3x3	152	/* Index for last 20in40MHz MCS rate */
#define TXP_FIRST_MCS_20_S1x3	153	/* Index for first 20MHz 1 Nsts to 3 Tx Chain rate */
#define TXP_LAST_MCS_20_S1x3	160	/* Index for last 20MHz 1 Nsts to 3 Tx Chain rate */
#define TXP_FIRST_MCS_20_S2x3	161	/* Index for first 20MHz 2 Nsts to 3 Tx Chain rate */
#define TXP_LAST_MCS_20_S2x3	168	/* Index for last 20MHz 2 Nsts to 3 Tx Chain rate */
#define TXP_FIRST_MCS_40_S1x3	169	/* Index for first 40MHz 1 Nsts to 3 Tx Chain rate */
#define TXP_LAST_MCS_40_S1x3	176	/* Index for last 40MHz 1 Nsts to 3 Tx Chain rate */
#define TXP_FIRST_MCS_40_S2x3	177	/* Index for first 40MHz 2 Nsts to 3 Tx Chain rate */
#define TXP_LAST_MCS_40_S2x3	184	/* Index for last 40MHz 2 Nsts to 3 Tx Chain rate */
#define TXP_FIRST_20UL_S1x3     185	/* Index for first 20in40MHz 1 Nsts to 3 Tx Chain rate */
#define TXP_LAST_20UL_S1x3	192	/* Index for last 20in40MHz 1 Nsts to 3 Tx Chain rate */
#define TXP_FIRST_20UL_S2x3     193	/* Index for first 20in40MHz 2 Nsts to 3 Tx Chain rate */
#define TXP_LAST_20UL_S2x3	200	/* Index for last 20in40MHz 2 Nsts to 3 Tx Chain rate */
#define TXP_FIRST_CCK_CDD_S1x2	201	/* Index for first CCK CDD 1 Nsts to 2 Tx Chain rate */
#define TXP_LAST_CCK_CDD_S1x2	204	/* Index for last CCK  CDD 1 Nsts to 2 Tx Chain rate */
#define TXP_FIRST_CCK_CDD_S1x3	205	/* Index for first CCK CDD 1 Nsts to 3 Tx Chain rate */
#define TXP_LAST_CCK_CDD_S1x3	208	/* Index for last CCK CDD 1 Nsts to 3 Tx Chain rate */
#define TXP_FIRST_CCK_20U_CDD_S1x2  209	/* Index for first CCK 20in40MHz 1x2 (1Nsts x 2Tx) rate */
#define TXP_LAST_CCK_20U_CDD_S1x2   212	/* Index for last CCK 20in40MHz 1x2 rate */
#define TXP_FIRST_CCK_20U_CDD_S1x3  213	/* Index for first CCK 20in40MHz 1x3 rate */
#define TXP_LAST_CCK_20U_CDD_S1x3   216	/* Index for last CCK 20in40MHz 1x3 rate */
#ifdef NOT_YET
#define TXP_FIRST_HT_STBC_S2x2	217	/* Index for first HT STBC 20MHz 2x2 rate */
#define TXP_LAST_HT_STBC_S2x2	224	/* Index for last HT STBC 20MHz 2x2 rate */
#define TXP_FIRST_HT_STBC_S2x3	225	/* Index for first HT STBC 20MHz 2x3 rate */
#define TXP_LAST_HT_STBC_S2x3	232	/* Index for last HT STBC 20MHz 2x3 rate */
#define TXP_FIRST_HT_STBC_40_S2x2   233	/* Index for first HT STBC 40MHz 2x2 rate */
#define TXP_LAST_HT_STBC_40_S2x2    240	/* Index for last HT STBC 40MHz 2x2 rate */
#define TXP_FIRST_HT_STBC_40_S2x3   241	/* Index for first HT STBC 40MHz 2x3 rate */
#define TXP_LAST_HT_STBC_40_S2x3    248	/* Index for last HT STBC 40MHz 2x3 rate */
#define TXP_FIRST_HT_STBC_UL20_S2x2 249	/* Index for first STBC 20in40MHz 2x2 rate */
#define TXP_LAST_HT_STBC_UL20_S2x2  256	/* Index for last HT STBC 20in40MHz 2x2 rate */
#define TXP_FIRST_HT_STBC_UL20_S2x3 257	/* Index for first HT STBC 20in40MHz 2x3 rate */
#define TXP_LAST_HT_STBC_UL20_S2x3  264	/* Index for last HT STBC 20in40MHz 2x3 rate */
#endif /* NOT_YET */
#define TXP_NUM_RATES_HT        217	/* number of rates in HT mode */

/* alias to SISO, CDD, SDM */
#define TXP_FIRST_MCS_20_S1x1	TXP_FIRST_MCS_20_SISO
#define TXP_LAST_MCS_20_S1x1	TXP_LAST_MCS_20_SISO
#define TXP_FIRST_MCS_20_S1x2	TXP_FIRST_MCS_20_CDD
#define TXP_LAST_MCS_20_S1x2   	TXP_LAST_MCS_20_CDD
#define TXP_FIRST_MCS_20_S2x2	TXP_FIRST_MCS_20_STBC
#define TXP_LAST_MCS_20_S2x2   	TXP_LAST_MCS_20_STBC
#define TXP_FIRST_MCS_20_S3x3	TXP_FIRST_MCS_20_SDM
#define TXP_LAST_MCS_20_S3x3	TXP_LAST_MCS_20_SDM

#define TXP_FIRST_MCS_40_S1x1	TXP_FIRST_MCS_40_SISO
#define TXP_LAST_MCS_40_S1x1	TXP_LAST_MCS_40_SISO
#define TXP_FIRST_MCS_40_S1x2	TXP_FIRST_MCS_40_CDD
#define TXP_LAST_MCS_40_S1x2	TXP_LAST_MCS_40_CDD
#define TXP_FIRST_MCS_40_S2x2	TXP_FIRST_MCS_40_STBC
#define TXP_LAST_MCS_40_S2x2	TXP_LAST_MCS_40_STBC
#define TXP_FIRST_MCS_40_S3x3	TXP_FIRST_MCS_40_SDM
#define TXP_LAST_MCS_40_S3x3	TXP_LAST_MCS_40_SDM

#define PHY_TOTAL_TX_FRAMES(pi) \
	wlapi_bmac_read_shm((pi)->sh->physhim, M_UCODE_MACSTAT + OFFSETOF(macstat_t, txallfrm))
#define PHY_TSSI_CAL_DBG_EN 0

#if HTCONF
#define TXP_NUM_RATES		TXP_NUM_RATES_HT
#else
#define TXP_NUM_RATES		TXP_NUM_RATES_MIMO
#endif

#define ADJ_PWR_TBL_LEN		84	/* number of phy-capable rates */

/* sslpnphy/ for siso devices */
#define TXP_FIRST_SISO_MCS_20	20	/* Index for first SISO MCS at 20 MHz */
#define TXP_LAST_SISO_MCS_20	27	/* Index for last SISO MCS at 20 MHz */

/* PHY/RADIO core(chains) */
#define PHY_CORE_NUM_1	1	/* 1 stream */
#define PHY_CORE_NUM_2	2	/* 2 streams */
#define PHY_CORE_NUM_3	3	/* 3 streams */
#define PHY_CORE_NUM_4	4	/* 4 streams */
#define PHY_CORE_MAX	PHY_CORE_NUM_4
#define PHY_CORE_0	0	/* array index for core 0 */
#define PHY_CORE_1	1	/* array index for core 1 */
#define PHY_CORE_2	2	/* array index for core 2 */
#define PHY_CORE_3	3	/* array index for core 3 */

/* macros to loop over TX/RX cores */
#define FOREACH_ACTV_CORE(pi, coremask, idx)	\
	for (idx = 0; (int) idx < pi->pubpi.phy_corenum; idx++) \
		if ((coremask >> idx) & 0x1)

#define FOREACH_CORE(pi, idx)	\
	for (idx = 0; (int) idx < pi->pubpi.phy_corenum; idx++)

/* Frequency Tones in Different Bandwidth */
#define NTONES_BW20 64
#define NTONES_BW40 128

/* aci_state state bits */
#define ACI_ACTIVE	1	/* enabled either manually or automatically */
#define ACI_CHANNEL_DELTA 5	/* How far a signal can bleed */
#define ACI_CHANNEL_SKIP 2	/* Num of immediately surrounding channels to skip */
#define ACI_FIRST_CHAN 1 /* Index for first channel */
#define ACI_LAST_CHAN 13 /* Index for last channel */
#define ACI_INIT_MA 100 /* Initial moving average for glitch for ACI */
#define ACI_SAMPLES 100 /* Number of samples for ACI */
#define ACI_MAX_UNDETECT_WINDOW_SZ 40

#define MA_WINDOW_SZ		8	/* moving average window size */

/* aci scan period */
#define NPHY_ACI_CHECK_PERIOD 2

/* noise only scan period */
#define NPHY_NOISE_CHECK_PERIOD 2

/* noise/RSSI state */
#define PHY_NOISE_SAMPLE_MON		1	/* sample phy noise for watchdog */
#define PHY_NOISE_SAMPLE_EXTERNAL	2	/* sample phy noise for scan, CQ/RM */
#define PHY_NOISE_WINDOW_SZ	16	/* NPHY noisedump window size */
#define PHY_NOISE_GLITCH_INIT_MA 10	/* Initial moving average for glitch cnt */
#define PHY_NOISE_GLITCH_INIT_MA_BADPlCP 10	/* Initial moving average for badplcp cnt */
#define PHY_NOISE_STATE_MON		0x1
#define PHY_NOISE_STATE_EXTERNAL	0x2
#define PHY_NOISE_SAMPLE_LOG_NUM_NPHY	10
#define PHY_NOISE_SAMPLE_LOG_NUM_UCODE	9	/* ucode uses smaller value to speed up process */
/* G-band: 30 (dbm) - 10*log10(50*(2^9/0.4)^2/16) + 2 (front-end-loss) - 68 (init_gain)
 * A-band: 30 (dbm) - 10*log10(50*(2^9/0.4)^2/16) + 3 (front-end-loss) - 69 (init_gain)
 */
#define PHY_NOISE_OFFSETFACT_4322  (-33)
#define HTPHY_NOISE_FLOOR_20M (-101)
#define HTPHY_NOISE_FLOOR_40M (-98)
#define HTPHY_NOISE_INITGAIN  (62)
#define HTPHY_NOISE_INITGAIN_X12_2G  (66)
#define HTPHY_NOISE_INITGAIN_X12_5G  (65)
#define HTPHY_NOISE_INITGAIN_X29_2G   (68)
#define HTPHY_NOISE_INITGAIN_X29_5G   (65)
#define HTPHY_NOISE_SAMPLEPWR_TO_DBM  (-37)
#define NPHY_NOISE_INITGAIN (67)
#define NPHY_NOISE_SAMPLEPWR_TO_DBM  (-37)
/* 1) Max rx_iq_est power per sample (digital) = 2*128^2 (I+Q) = 45 dB
 * 2) Max analog input power to ADCs (0.8V p-p) = 10*log10(0.^4*2/50) + 3(I+Q) + 30(dbm) = 8 dBm
 * 3) rx_iq_est power to dBm conversion = 8 - 45 = -37
 */

#define LCNPHY_NOISE_OFFSETFACT_4330  (-31)
#define LCNPHY_NOISE_FLOOR_20M (-101)
#define LCNPHY_NOISE_FLOOR_40M (-98)

#define PHY_NOISE_MA_WINDOW_SZ	2	/* moving average window size */

#define	PHY_RSSI_TABLE_SIZE	64	/* Table size for PHY RSSI Table */
#define RSSI_ANT_MERGE_MAX	0	/* pick max rssi of all antennas */
#define RSSI_ANT_MERGE_MIN	1	/* pick min rssi of all antennas */
#define RSSI_ANT_MERGE_AVG	2	/* pick average rssi of all antennas */

/* TSSI/txpower */
#define	PHY_TSSI_TABLE_SIZE	64	/* Table size for PHY TSSI */
#define	APHY_TSSI_TABLE_SIZE	256	/* Table size for A-PHY TSSI */
#define	TX_GAIN_TABLE_LENGTH	64	/* Table size for gain_table */
#define	DEFAULT_11A_TXP_IDX	24	/* Default index for 11a Phy tx power */
#define NUM_TSSI_FRAMES        4	/* Num ucode frames for TSSI estimation */
#define	NULL_TSSI		0x7f	/* Default value for TSSI - byte */
#define	NULL_TSSI_W		0x7f7f	/* Default value for word TSSI */

#define PHY_PAPD_EPS_TBL_SIZE_LPPHY 64
#define PHY_PAPD_EPS_TBL_SIZE_LCNPHY 64
#define PHY_PAPD_EPS_TBL_SIZE_LCN40PHY 256
/* the generic PHY_PERICAL defines are in wlc_phy_hal.h */
#define LCNPHY_PERICAL_TEMPBASED_TXPWRCTRL 9

#define PHY_TXPWR_MIN		10	/* default min tx power */
#define PHY_TXPWR_MIN_NPHY	8	/* for nphy devices */
#define RADIOPWR_OVERRIDE_DEF	(-1)

#define PWRTBL_NUM_COEFF	3	/* b0, b1, a1 */

/* calibraiton */
#define SPURAVOID_AUTO		-1	/* enable on certain channels, disable elsewhere */
#define SPURAVOID_DISABLE	0	/* disabled */
#define SPURAVOID_FORCEON	1	/* on mode1 */
#define SPURAVOID_FORCEON2	2	/* on mode2 (different freq) */

#define PHY_SW_TIMER_FAST		15	/* 15 second timeout */
#define PHY_SW_TIMER_SLOW		60	/* 60 second timeout */
#define PHY_SW_TIMER_GLACIAL	120	/* 120 second timeout */

#define PHY_PERICAL_AUTO	0	/* cal type: let PHY decide */
#define PHY_PERICAL_FULL	1	/* cal type: full */
#define PHY_PERICAL_PARTIAL	2	/* cal type: partial (save time) */

#define PHY_PERICAL_NODELAY	0	/* multiphase cal gap, in unit of ms */
#define PHY_PERICAL_INIT_DELAY	5
#define PHY_PERICAL_ASSOC_DELAY	5
#define PHY_PERICAL_WDOG_DELAY	5

#define PHY_PERICAL_MPHASE_PENDING(pi) \
	(pi->cal_info->cal_phase_id > MPHASE_CAL_STATE_IDLE)

#define PHY_CAL_SEARCHMODE_RESTART   0  /* cal search mode (former FULL) */
#define PHY_CAL_SEARCHMODE_REFINE    1 /* cal search mode (former PARTIAL) */


/* PHY SPINWAIT in unit of us */
#define NPHY_SPINWAIT_RXCORE_SETSTATE_RFSEQ_STATUS 1000
#define NPHY_SPINWAIT_RADIO_2055_RCAL		2000
#define NPHY_SPINWAIT_RFCTRLINTC_REV3_OVERRIDE	10000
#define NPHY_SPINWAIT_RUNSAMPLES_RFSEQ_STATUS	1000
#define NPHY_SPINWAIT_CAL_TXIQLO		20000
#define NPHY_SPINWAIT_RX_IQ_EST			10000
#define NPHY_SPINWAIT_PAPDCAL			200000
#define NPHY_SPINWAIT_FORCE_RFSEQ_STATUS	200000

#define HTPHY_SPINWAIT_RFSEQ_STOP		1000
#define HTPHY_SPINWAIT_RFSEQ_FORCE		200000
#define HTPHY_SPINWAIT_RUNSAMPLE		1000
#define HTPHY_SPINWAIT_TXIQLO			20000
#define HTPHY_SPINWAIT_IQEST			10000

enum {
	PHY_ACI_PWR_NOTPRESENT,   /* ACI is not present */
	PHY_ACI_PWR_LOW,
	PHY_ACI_PWR_MED,
	PHY_ACI_PWR_HIGH
};

/* Multiphase calibration states and cmds per Tx Phase (for NPHY) */
#define MPHASE_TXCAL_NUMCMDS	2  /* Number of Tx cal cmds per phase */
enum {
	MPHASE_CAL_STATE_IDLE = 0,
	MPHASE_CAL_STATE_INIT = 1,
	MPHASE_CAL_STATE_TXPHASE0,
	MPHASE_CAL_STATE_TXPHASE1,
	MPHASE_CAL_STATE_TXPHASE2,
	MPHASE_CAL_STATE_TXPHASE3,
	MPHASE_CAL_STATE_TXPHASE4,
	MPHASE_CAL_STATE_TXPHASE5,
	MPHASE_CAL_STATE_PAPDCAL,	/* IPA */
	MPHASE_CAL_STATE_PAPDCAL1,	/* IPA */
	MPHASE_CAL_STATE_RXCAL,
	MPHASE_CAL_STATE_RXCAL1,
	MPHASE_CAL_STATE_RSSICAL,
	MPHASE_CAL_STATE_IDLETSSI
};

/* mphase phases for HTPHY */
enum {
	CAL_PHASE_IDLE = 0,
	CAL_PHASE_INIT = 1,
	CAL_PHASE_TX0,
	CAL_PHASE_TX1,
	CAL_PHASE_TX2,
	CAL_PHASE_TX3,
	CAL_PHASE_TX4,
	CAL_PHASE_TX5,
	CAL_PHASE_TX6,
	CAL_PHASE_TX7,
	CAL_PHASE_TX_LAST,
	CAL_PHASE_PAPDCAL,	/* IPA */
	CAL_PHASE_RXCAL,
	CAL_PHASE_RSSICAL,
	CAL_PHASE_IDLETSSI
};

enum {
	PHY_TSSI_SET_MAX_LIMIT = 1,
	PHY_TSSI_SET_MIN_LIMIT = 2,
	PHY_TSSI_SET_MIN_MAX_LIMIT = 3
};

typedef enum {
	CAL_FULL,
	CAL_RECAL,
	CAL_CURRECAL,
	CAL_DIGCAL,
	CAL_GCTRL,
	CAL_SOFT,
	CAL_DIGLO,
	CAL_IQ_RECAL,
	CAL_TXPWRCTRL
} phy_cal_mode_t;

typedef enum {
	ADC_20M,
	ADC_40M,
	ADC_20M_LP,
	ADC_40M_LP,
	ADC_FLASHONLY
} phy_adc_mode_t;

typedef enum {
	TX_IIR_FILTER_CCK,
	TX_IIR_FILTER_OFDM,
	TX_IIR_FILTER_OFDM40
} phy_tx_iir_filter_mode_t;

typedef struct {
	uint16 gm_gain;
	uint16 pga_gain;
	uint16 pad_gain;
	uint16 dac_gain;
} phy_txgains_t;

typedef struct {
	phy_txgains_t gains;
	bool useindex;
	uint8 index;
} phy_txcalgains_t;

typedef struct {
	uint8 chan;
	int16 a;
	int16 b;
} phy_rx_iqcomp_t;

typedef struct {
	int16 re;
	int16 im;
} phy_spb_tone_t;

typedef struct {
	uint16 re;
	uint16 im;
} phy_unsign16_struct;

typedef struct {
	uint32 iq_prod;
	uint32 i_pwr;
	uint32 q_pwr;
} phy_iq_est_t;

typedef struct {
	uint16 ptcentreTs20;
	uint16 ptcentreFactor;
} phy_sfo_cfg_t;

typedef enum {
	PHY_PAPD_CAL_CW,
	PHY_PAPD_CAL_OFDM
} phy_papd_cal_type_t;

typedef struct {
	uchar gm;
	uchar pga;
	uchar pad;
	uchar dac;
	uchar bb_mult;
} phy_tx_gain_tbl_entry;


#define MAX_NUM_ANCHORS 4

typedef struct ratmodel_paparams {
	int64 p[128], n[128];
	int64 rho[128][3];
	int64 rho_t[3][128];
	int64 c1[3][3];
	int64 c2_calc[3][3];
	int64 c3[3][128];
	int64 c4[3][1];
	int64 det_c1;
} ratmodel_paparams_t;

typedef struct tssi_cal_info {
	int target_pwr_qdBm[MAX_NUM_ANCHORS];
	int measured_pwr_qdBm[MAX_NUM_ANCHORS];
	uint8 anchor_bbmult[MAX_NUM_ANCHORS];
	uint16 anchor_txidx[MAX_NUM_ANCHORS];
	uint16 anchor_tssi[MAX_NUM_ANCHORS];
	uint16 curr_anchor;
	uint8 paparams_calc_in_progress;
	uint8 paparams_calc_done;
	ratmodel_paparams_t rsd;
	int64 paparams_new[4];
} tssi_cal_info_t;


#define PHY_NOISE_DBG_UCODE_NUM_SMPLS (0)
#define PHY_NOISE_DBG_DATA_LEN (38 + PHY_NOISE_DBG_UCODE_NUM_SMPLS)
#define PHY_NOISE_DBG_HISTORY 0

#define k_noise_cal_ucode_data_size (8)

#define k_noise_cal_update_steps 2

typedef struct {
	/* state info */
	bool nvram_enable_2g;
	bool nvram_enable_5g;
	bool enable;
	bool global_adj_en;
	bool adj_en;
	bool tainted;
	uint8 state;
	bool noise_cb;
	int8 ref;
	int8 nvram_ref_2g;
	int8 nvram_ref_5g;
	int8 nvram_po_bias_2g;
	int8 nvram_po_bias_5g;
	bool nvram_dbg_noise;
	int nvram_high_gain;
	int16 nvram_input_pwr_offset_2g;
	int16 nvram_input_pwr_offset_5g[3];
	int8 nvram_gain_tbl_adj_2g;
	int8 nvram_gain_tbl_adj_5g;
	int nvram_nf_substract_val;
	/* phy regs saved */
	int16 high_gain;
	int16 input_pwr_offset;
	uint16 nf_substract_val;
	uint32 power;
	uint32 ucode_data[k_noise_cal_ucode_data_size];
	int8   ucode_data_len;
	int8   ucode_data_idx;
	uint8  ucode_data_ok_cnt;
	uint8  update_cnt;
	uint8  update_step;
	uint8  update_ucode_interval[k_noise_cal_update_steps];
	uint8  update_data_interval[k_noise_cal_update_steps];
	uint8  update_step_interval[k_noise_cal_update_steps];
#if PHY_NOISE_DBG_HISTORY > 0
	/* dbg */
	int16  dbg_adj_min;
	int16  dbg_adj_max;
	uint16 start_time;
	uint16 per_start_time;
	int8 dbg_dump_idx;
	int8 dbg_dump_sub_idx;
	uint32 dbg_dump_cmd;
	int8 dbg_idx;
#if PHY_NOISE_DBG_UCODE_NUM_SMPLS > 0
	int16 dbg_samples[PHY_NOISE_DBG_UCODE_NUM_SMPLS*2];
#endif
	uint16 dbg_info[PHY_NOISE_DBG_HISTORY][PHY_NOISE_DBG_DATA_LEN];
#endif /* #if PHY_NOISE_DBG_HISTORY > 0 */
} noise_t;

/* Radar detect scratchpad area, RDR_NTIER_SIZE must be bigger than RDR_TIER_SIZE */
#define RDR_NTIERS  1	   /* Number of tiers */
#define RDR_TIER_SIZE 64   /* Size per tier, aphy  */
#define RDR_LIST_SIZE 512/3 + 2  /* Size of the list (rev 3 fifo size = 512) */
#define RDR_EPOCH_SIZE 40
#define RDR_NANTENNAS 2
#define RDR_NTIER_SIZE  RDR_LIST_SIZE  /* Size per tier, nphy */
#define RDR_LP_BUFFER_SIZE 64
#define LP_LEN_HIS_SIZE 10
#define LP_BUFFER_SIZE 64
#define MAX_LP_BUFFER_SPAN_20MHZ 240000000
#define MAX_LP_BUFFER_SPAN_40MHZ 480000000
#define RDR_SDEPTH_EXTRA_PULSES 1

/* For bounding the size of the baseband lo comp results array */
#define STATIC_NUM_RF 32	/* Largest number of RF indexes */
#define STATIC_NUM_BB 9		/* Largest number of BB indexes */

#define BB_MULT_MASK		0x0000ffff
#define BB_MULT_VALID_MASK	0x80000000

#define CORDIC_AG	39797
#define	CORDIC_NI	18
#define	FIXED(X)	((int32)((X) << 16))
#define	FLOAT(X)	(((X) >= 0) ? ((((X) >> 15) + 1) >> 1) : -((((-(X)) >> 15) + 1) >> 1))

#define HTPHY_CHAIN_TX_DISABLE_TEMP	150
#define PHY_CHAIN_TX_DISABLE_TEMP	115
#define PHY_HYSTERESIS_DELTATEMP	5

#define PHY_BITSCNT(x)		bcm_bitcount((uint8 *)&(x), sizeof(uint8))

/* validation macros */
#define	VALID_PHYTYPE(phytype)	(((uint)phytype == PHY_TYPE_A) || \
				 ((uint)phytype == PHY_TYPE_B) || \
				 ((uint)phytype == PHY_TYPE_G) || \
				 ((uint)phytype == PHY_TYPE_N) || \
				 ((uint)phytype == PHY_TYPE_LP)|| \
				 ((uint)phytype == PHY_TYPE_SSN) || \
				 ((uint)phytype == PHY_TYPE_LCN) || \
				 ((uint)phytype == PHY_TYPE_LCN40) || \
				 ((uint)phytype == PHY_TYPE_HT))

#define VALID_G_RADIO(radioid)	(radioid == BCM2050_ID)
#define VALID_A_RADIO(radioid)	(radioid == BCM2060_ID)
#define VALID_N_RADIO(radioid)  ((radioid == BCM2055_ID) || (radioid == BCM2056_ID) || \
				(radioid == BCM2057_ID) || (radioid == BCM20671_ID))
#define VALID_HT_RADIO(radioid)  (radioid == BCM2059_ID)

#define LPPHY_RADIO_ID(pi)	(LPREV_GE((pi)->pubpi.phy_rev, 2) ? BCM2063_ID : BCM2062_ID)
#define VALID_LP_RADIO(pi, radioid) (LPPHY_RADIO_ID(pi) == (radioid))
#define VALID_SSLPN_RADIO(radioid)  (radioid == BCM2063_ID)
#define VALID_LCN_RADIO(radioid)  ((radioid == BCM2064_ID) || (radioid == BCM2066_ID))
#define VALID_LCN40_RADIO(radioid)  ((radioid == BCM2067_ID) || (radioid == BCM2065_ID))

#define	VALID_RADIO(pi, radioid) \
	(ISGPHY(pi) ? VALID_G_RADIO(radioid) : \
	 (ISAPHY(pi) ? VALID_A_RADIO(radioid) : \
	  (ISNPHY(pi) ? VALID_N_RADIO(radioid) : \
	   (ISSSLPNPHY(pi) ? VALID_SSLPN_RADIO(radioid) : \
	   (ISLCNPHY(pi) ? VALID_LCN_RADIO(radioid) : \
	   (ISLCN40PHY(pi) ? VALID_LCN40_RADIO(radioid) : \
	    (ISHTPHY(pi) ? VALID_HT_RADIO(radioid) : \
	    VALID_LP_RADIO(pi, radioid))))))))

#define SCAN_INPROG_PHY(pi)	(mboolisset(pi->measure_hold, PHY_HOLD_FOR_SCAN))
#define RM_INPROG_PHY(pi)	(mboolisset(pi->measure_hold, PHY_HOLD_FOR_RM))
#define PLT_INPROG_PHY(pi)	(mboolisset(pi->measure_hold, PHY_HOLD_FOR_PLT))
#define ASSOC_INPROG_PHY(pi)	(mboolisset(pi->measure_hold, PHY_HOLD_FOR_ASSOC))
#define SCAN_RM_IN_PROGRESS(pi) (mboolisset(pi->measure_hold, PHY_HOLD_FOR_SCAN | PHY_HOLD_FOR_RM))
#define PHY_MUTED(pi)		(mboolisset(pi->measure_hold, PHY_HOLD_FOR_MUTE))
#define PUB_NOT_ASSOC(pi)	(mboolisset(pi->measure_hold, PHY_HOLD_FOR_NOT_ASSOC))
#define ACI_SCAN_INPROG_PHY(pi)	(mboolisset(pi->measure_hold, PHY_HOLD_FOR_ACI_SCAN))
#define DCS_INPROG_PHY(pi)	(mboolisset(pi->measure_hold, PHY_HOLD_FOR_DCS))

#if defined(EXT_CBALL) || defined(BCMQT)
#define NORADIO_ENAB(pub) ((pub).radioid == NORADIO_ID)
#else
#define NORADIO_ENAB(pub) 0
#endif

#ifdef BCMECICOEX
/* SECI combo board (SECI enabled) */
#define BCMSECICOEX_ENAB_PHY(pi) \
	(((pi)->sh->boardflags & BFL_BTCOEX) && \
	!((pi)->sh->boardflags2 & BFL2_BTC3WIRE) && \
	(si_seci((pi)->sh->sih)) && \
	((pi)->sh->machwcap & MCAP_BTCX))
/* ECI chip */
#define BCMECICOEX_ENAB_PHY(pi) \
	(((pi)->sh->boardflags & BFL_BTCOEX) && \
	D11REV_GE((pi)->sh->corerev, 15) && \
	(si_eci((pi)->sh->sih)) && \
	((pi)->sh->machwcap & MCAP_BTCX))

#define BCMECISECICOEX_ENAB_PHY(pi)  (BCMECICOEX_ENAB_PHY(pi) || BCMSECICOEX_ENAB_PHY(pi))
#else /* BCMECICOEX */
#define BCMECICOEX_ENAB_PHY(pi)         0
#define BCMECISECICOEX_ENAB_PHY(pi)	0
#endif /* BCMECICOEX */

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  table-driven register operations                            */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

#define PHY_REG_MOD(pi, phy_type, reg_name, field, value) \
	phy_reg_mod(pi, phy_type##_##reg_name, \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT)

#define PHY_REG_MOD2(pi, phy_type, reg_name, field, field2, value, value2) \
	phy_reg_mod(pi, phy_type##_##reg_name, \
	phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK, \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT))

#define PHY_REG_MOD3(pi, phy_type, reg_name, field, field2, field3, value, value2, value3) \
	phy_reg_mod(pi, phy_type##_##reg_name, \
	phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK | \
	phy_type##_##reg_name##_##field3##_MASK, \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT) | \
	((value3) << phy_type##_##reg_name##_##field3##_SHIFT))

#define PHY_REG_MOD_RAW(pi, addr, mask, value) \
	phy_reg_mod(pi, addr, mask, value)

#define PHY_REG_READ(pi, phy_type, reg_name, field) \
	((phy_reg_read(pi, phy_type##_##reg_name) & \
	phy_type##_##reg_name##_##field##_##MASK) >> phy_type##_##reg_name##_##field##_##SHIFT)

#define PHY_REG_READ_CORE(pi, phy_type, core, reg_name) \
	phy_reg_read(pi, \
	core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1)

#define PHY_REG_MOD_CORE(pi, phy_type, core, reg_name, field, value) \
	phy_reg_mod(pi, \
	core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1, \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT)

#define PHY_REG_WRITE(pi, phy_type, reg_name, value) \
	phy_reg_write(pi, phy_type##_##reg_name, value)

#define PHY_REG_WRITE_CORE(pi, phy_type, core, reg_name, value) \
	phy_reg_write(pi, \
	core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1, \
	value)

#define PHY_REG_AND(pi, phy_type, reg_name, value) \
	phy_reg_and(pi, phy_type##_##reg_name, value);

#define PHY_REG_OR(pi, phy_type, reg_name, value) \
	phy_reg_or(pi, phy_type##_##reg_name, value);


#define PHY_REG_MOD_TYPE	0x0000
#define PHY_REG_WRITE_TYPE	0x4000
#define PHY_REG_AND_TYPE	0x8000
#define PHY_REG_OR_TYPE		0xc000
#define PHY_REG_MASK_TYPE	0xc000

extern void phy_reg_write_array(void* pi, const uint16* regp, int length);

#define PHY_REG_LIST_START \
	{ static const uint16 write_phy_reg_table[] = {

#define PHY_REG_LIST_START_WLBANDINITDATA \
	{ static const uint16 WLBANDINITDATA(write_phy_reg_table)[] = {

#define PHY_REG_LIST_EXECUTE(pi) \
	}; \
	phy_reg_write_array(pi, write_phy_reg_table, \
	sizeof(write_phy_reg_table)/sizeof(write_phy_reg_table[0])); }

#define PHY_REG_MOD_ENTRY(phy_type, reg_name, field, value) \
	PHY_REG_MOD_TYPE | (phy_type##_##reg_name), \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT, \

#define PHY_REG_MOD2_ENTRY(phy_type, reg_name, field, field2, value, value2) \
	PHY_REG_MOD_TYPE | (phy_type##_##reg_name), \
	(phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK), \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT), \

#define PHY_REG_MOD3_ENTRY(phy_type, reg_name, field, field2, field3, value, value2, value3) \
	PHY_REG_MOD_TYPE | (phy_type##_##reg_name), \
	(phy_type##_##reg_name##_##field##_MASK | phy_type##_##reg_name##_##field2##_MASK | \
	phy_type##_##reg_name##_##field3##_MASK), \
	((value) << phy_type##_##reg_name##_##field##_SHIFT) | \
	((value2) << phy_type##_##reg_name##_##field2##_SHIFT) | \
	((value3) << phy_type##_##reg_name##_##field3##_SHIFT), \

#define PHY_REG_MOD_CORE_ENTRY(phy_type, core, reg_name, field, value) \
	PHY_REG_MOD_TYPE | (core == 0 ? phy_type##_##reg_name##0 : phy_type##_##reg_name##1), \
	phy_type##_##reg_name##_##field##_MASK, \
	(value) << phy_type##_##reg_name##_##field##_SHIFT, \

#define PHY_REG_MOD_RAW_ENTRY(addr, mask, value) \
	PHY_REG_MOD_TYPE | (addr), mask, value, \

#define PHY_REG_WRITE_ENTRY(phy_type, reg_name, value) \
	PHY_REG_WRITE_TYPE | (phy_type##_##reg_name), value, \

#define PHY_REG_WRITE_RAW_ENTRY(addr, value) \
	PHY_REG_WRITE_TYPE | (addr), value, \

#define PHY_REG_AND_ENTRY(phy_type, reg_name, value) \
	PHY_REG_AND_TYPE | (phy_type##_##reg_name), value, \

#define PHY_REG_AND_RAW_ENTRY(addr, value) \
	PHY_REG_AND_TYPE | (addr), value, \

#define PHY_REG_OR_ENTRY(phy_type, reg_name, value) \
	PHY_REG_OR_TYPE | (phy_type##_##reg_name), value, \

#define PHY_REG_OR_RAW_ENTRY(addr, value) \
	PHY_REG_OR_TYPE | (addr), value, \

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  phy_info_t and its prerequisite                             */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */


#define PHY_LTRN_LIST_LEN	64
extern uint16 ltrn_list[PHY_LTRN_LIST_LEN];

typedef struct _phy_table_info {
	uint	table;
	int	q;
	uint	max;
} phy_table_info_t;

/* phy table structure used for MIMOPHY */
struct phytbl_info {
	const void   *tbl_ptr;
	uint32  tbl_len;
	uint32  tbl_id;
	uint32  tbl_offset;
	uint32  tbl_width;
};

/* phy txcal coeffs structure used for HTPHY */
typedef struct txcal_coeffs {
	uint16 txa;
	uint16 txb;
	uint16 txd;	/* contain di & dq */
	uint8 txei;
	uint8 txeq;
	uint8 txfi;
	uint8 txfq;
	uint16 rxa;
	uint16 rxb;
} txcal_coeffs_t;

/* phy table structure used for LPPHY */
typedef struct lpphytbl_info {
	const void   *tbl_ptr;
	uint32  tbl_len;
	uint32  tbl_id;
	uint32  tbl_offset;
	uint32  tbl_width;
	uint8	tbl_phywidth;
} lpphytbl_info_t;

typedef struct {
	uint32 iq_prod;
	uint32 i_pwr;
	uint32 q_pwr;
} sslpnphy_iq_est_t;

typedef struct  {
	uint8  curr_home_channel;
	uint16 crsminpwrthld_40_stored;
	uint16 crsminpwrthld_20L_stored;
	uint16 crsminpwrthld_20U_stored;
	uint16 crsminpwrthld_20L_base;
	uint16 crsminpwrthld_20U_base;
	uint16 crsminpwr1thld_40_stored;
	uint16 init_gain_code_core0_stored;
	uint16 init_gain_code_core1_stored;
	uint16 init_gain_code_core2_stored;
	uint16 init_gain_codeb_core0_stored;
	uint16 init_gain_codeb_core1_stored;
	uint16 init_gain_codeb_core2_stored;
	uint16 init_gain_table_stored[4];

	uint16 clip1_hi_gain_code_core0_stored;
	uint16 clip1_hi_gain_code_core1_stored;
	uint16 clip1_hi_gain_code_core2_stored;
	uint16 clip1_hi_gain_codeb_core0_stored;
	uint16 clip1_hi_gain_codeb_core1_stored;
	uint16 clip1_hi_gain_codeb_core2_stored;
	uint16 nb_clip_thresh_core0_stored;
	uint16 nb_clip_thresh_core1_stored;
	uint16 nb_clip_thresh_core2_stored;
	uint16 init_ofdmlna2gainchange_stored[4];
	uint16 init_ccklna2gainchange_stored[4];
	uint16 clip1_lo_gain_code_core0_stored;
	uint16 clip1_lo_gain_code_core1_stored;
	uint16 clip1_lo_gain_code_core2_stored;
	uint16 clip1_lo_gain_codeb_core0_stored;
	uint16 clip1_lo_gain_codeb_core1_stored;
	uint16 clip1_lo_gain_codeb_core2_stored;
	uint16 w1_clip_thresh_core0_stored;
	uint16 w1_clip_thresh_core1_stored;
	uint16 w1_clip_thresh_core2_stored;
	uint16 radio_2056_core1_rssi_gain_stored;
	uint16 radio_2056_core2_rssi_gain_stored;
	uint16 energy_drop_timeout_len_stored;

	uint16 ed_crs40_assertthld0_stored;
	uint16 ed_crs40_assertthld1_stored;
	uint16 ed_crs40_deassertthld0_stored;
	uint16 ed_crs40_deassertthld1_stored;
	uint16 ed_crs20L_assertthld0_stored;
	uint16 ed_crs20L_assertthld1_stored;
	uint16 ed_crs20L_deassertthld0_stored;
	uint16 ed_crs20L_deassertthld1_stored;
	uint16 ed_crs20U_assertthld0_stored;
	uint16 ed_crs20U_assertthld1_stored;
	uint16 ed_crs20U_deassertthld0_stored;
	uint16 ed_crs20U_deassertthld1_stored;

	uint	scanroamtimer;

	int8	rssi;
	int8 	rssi_index;
	int8 	rssi_buffer[10];
	int16	max_hpvga_acioff_2G;
	int16	max_hpvga_acion_2G;
	int16	max_hpvga_acioff_5G;
	int16	max_hpvga_acion_5G;

	uint16  badplcp_ma;
	uint16  badplcp_ma_previous;
	uint16  badplcp_ma_total;
	uint16  badplcp_ma_list[MA_WINDOW_SZ];
	int  badplcp_ma_index;
	int16 pre_badplcp_cnt;
	int16  bphy_pre_badplcp_cnt;

	uint16 init_gain_core0;
	uint16 init_gain_core1;
	uint16 init_gain_core2;
	uint16 init_gainb_core0;
	uint16 init_gainb_core1;
	uint16 init_gainb_core2;
	uint16 init_gain_rfseq[4];

	uint16 init_gain_core0_aci_on;
	uint16 init_gain_core1_aci_on;
	uint16 init_gain_core2_aci_on;
	uint16 init_gainb_core0_aci_on;
	uint16 init_gainb_core1_aci_on;
	uint16 init_gainb_core2_aci_on;
	uint16 init_gain_rfseq_aci_on[4];
	uint16 init_gain_core0_aci_off;
	uint16 init_gain_core1_aci_off;
	uint16 init_gain_core2_aci_off;
	uint16 init_gainb_core0_aci_off;
	uint16 init_gainb_core1_aci_off;
	uint16 init_gainb_core2_aci_off;
	uint16 init_gain_rfseq_aci_off[4];

	uint16 crsminpwr0;
	uint16 crsminpwrl0;
	uint16 crsminpwru0;

	uint16 crsminpwr0_aci_on;
	uint16 crsminpwrl0_aci_on;
	uint16 crsminpwru0_aci_on;

	uint16 crsminpwr0_aci_off;
	uint16 crsminpwrl0_aci_off;
	uint16 crsminpwru0_aci_off;

	int16 crsminpwr_index;
	int16 crsminpwr_index_aci_on;
	int16 crsminpwr_index_aci_off;

	uint16 radio_2057_core0_rssi_wb1a_gc_stored;
	uint16 radio_2057_core1_rssi_wb1a_gc_stored;
	uint16 radio_2057_core2_rssi_wb1a_gc_stored;
	uint16 radio_2057_core0_rssi_wb1g_gc_stored;
	uint16 radio_2057_core1_rssi_wb1g_gc_stored;
	uint16 radio_2057_core2_rssi_wb1g_gc_stored;
	uint16 radio_2057_core0_rssi_wb2_gc_stored;
	uint16 radio_2057_core1_rssi_wb2_gc_stored;
	uint16 radio_2057_core2_rssi_wb2_gc_stored;
	uint16 radio_2057_core0_rssi_nb_gc_stored;
	uint16 radio_2057_core1_rssi_nb_gc_stored;
	uint16 radio_2057_core2_rssi_nb_gc_stored;

	bool  aci_on_firsttime;
	bool  noise_sw_set;
	bool  store_values;

	struct {
		uint16  ofdm_glitch_ma;
		uint16  ofdm_glitch_ma_previous;
		int16  bphy_pre_glitch_cnt;
		uint16  ofdm_ma_total;
		uint16  ofdm_glitch_ma_list[PHY_NOISE_MA_WINDOW_SZ];
		int  ofdm_ma_index;

		uint16  ofdm_badplcp_ma;
		uint16  ofdm_badplcp_ma_previous;
		uint16  ofdm_badplcp_ma_total;
		uint16  ofdm_badplcp_ma_list[PHY_NOISE_MA_WINDOW_SZ];
		int  ofdm_badplcp_ma_index;


		uint16 noise_glitch_low_detect_total;
		uint16 noise_glitch_high_detect_total;

		uint16 newgain_initgain;
		uint16 newgainb_initgain;
		uint16 newgain_rfseq;
		bool changeinitgain;
		uint16 newcrsminpwr_20U;
		uint16 newcrsminpwr_20L;
		uint16 newcrsminpwr_40;

		uint16 nphy_noise_noassoc_glitch_th_up; /* wl interference 4 */
		uint16 nphy_noise_noassoc_glitch_th_dn;
		uint16 nphy_noise_assoc_glitch_th_up;
		uint16 nphy_noise_assoc_glitch_th_dn;
		uint16 nphy_noise_assoc_aci_glitch_th_up;
		uint16 nphy_noise_assoc_aci_glitch_th_dn;
		uint16 nphy_noise_assoc_enter_th;
		uint16 nphy_noise_noassoc_enter_th;
		uint16 nphy_noise_assoc_rx_glitch_badplcp_enter_th;
		uint16 nphy_noise_noassoc_crsidx_incr;
		uint16 nphy_noise_assoc_crsidx_incr;
		uint16 nphy_noise_crsidx_decr;

	} noise;

	struct {
		uint16  glitch_ma;
		uint16  glitch_ma_previous;
		int  pre_glitch_cnt;
		uint16  ma_total;
		uint16  ma_list[MA_WINDOW_SZ];
		int  ma_index;
		int  exit_thresh;		/* Lowater to exit aci */
		int  enter_thresh;		/* hiwater to enter aci mode */
		int  usec_spintime;		/* Spintime between samples */
		int  glitch_delay;		/* delay between ACI scans when glitch count is
								 * continously high
								 */
		int  countdown;
		char rssi_buf[CH_MAX_2G_CHANNEL][ACI_SAMPLES + 1];
			/* Save rssi vals for debugging */

		struct {
			uint16 radio_2055_core1_rxrf_spc1;
			uint16 radio_2055_core2_rxrf_spc1;

			uint16 radio_2055_core1_rxbb_rxcal_ctrl;
			uint16 radio_2055_core2_rxbb_rxcal_ctrl;

			uint16 overrideDigiGain1;
			uint16 bphy_peak_energy_lo;

			uint16 init_gain_code_core0;
			uint16 init_gain_code_core1;
			uint16 init_gain_code_core2;
			uint16 init_gain_table[4];

			uint16 clip1_hi_gain_code_core0;
			uint16 clip1_hi_gain_code_core1;
			uint16 clip1_hi_gain_code_core2;
			uint16 clip1_md_gain_code_core0;
			uint16 clip1_md_gain_code_core1;
			uint16 clip1_md_gain_code_core2;
			uint16 clip1_lo_gain_code_core0;
			uint16 clip1_lo_gain_code_core1;
			uint16 clip1_lo_gain_code_core2;

			uint16 nb_clip_thresh_core0;
			uint16 nb_clip_thresh_core1;
			uint16 nb_clip_thresh_core2;

			uint16 w1_clip_thresh_core0;
			uint16 w1_clip_thresh_core1;
			uint16 w1_clip_thresh_core2;

			uint16 cck_compute_gain_info_core0;
			uint16 cck_compute_gain_info_core1;
			uint16 cck_compute_gain_info_core2;

			uint16 energy_drop_timeout_len;
			uint16 crs_threshold2u;

			bool gain_boost;

			/* Phyreg 0xc33 (bphy energy thresh values for ACI pwr levels */
			/*  (lo, md, hi) */
			uint16 b_energy_lo_aci;
			uint16 b_energy_md_aci;
			uint16 b_energy_hi_aci;

			bool detection_in_progress;
			/* can be modified using ioctl/iovar */
			uint16 adcpwr_enter_thresh;
			uint16 adcpwr_exit_thresh;
			uint16 detect_repeat_ctr;
			uint16 detect_num_samples;
			uint16 undetect_window_sz;

		} nphy;

		/* history of ACI detects */
		int  detect_index;
		int  detect_total;
		int  detect_list[ACI_MAX_UNDETECT_WINDOW_SZ];
		int  detect_acipwr_lt_list[ACI_MAX_UNDETECT_WINDOW_SZ];
		int  detect_acipwr_max;

	} aci;


} interference_info_t;


#define LPPHY_ACI_MAX_REGS 16
typedef struct {
	int on_thresh; /* number of glitches */
	int on_timeout; /* in seconds */
	int off_thresh; /* number of glitches */
	int off_timeout; /* in seconds */
	int glitch_timeout; /* in microseconds */
	int glitch_cnt;
	int chan_scan_cnt; /* number of measurements to take on a given channel */
	int chan_scan_pwr_thresh; /* pwr metric threshold */
	int chan_scan_cnt_thresh; /* count threshold */
	int chan_scan_timeout; /* minimum time between channel scans */
	int state;
	int t;
	int t_scan;
	const uint32* aci_reg;
	uint8 dflt_reg[LPPHY_ACI_MAX_REGS];
	int last_chan;
} lpphy_aci_t;

typedef struct {
	wl_radar_args_t radar_args;	/* radar detection parametners */
	wl_radar_thr_t radar_thrs;	/* radar thresholds */
	int min_tint;			/* minimum t_int (1/prf) (20 MHz clocks) */
	int max_tint;			/* maximum t_int (20 MHz clocks) */
	int min_blen;			/* minimum burst length (20 MHz clocks) */
	int max_blen;			/* maximum burst length (20 MHz clocks) */
	int sdepth_extra_pulses;
	int min_deltat_lp;
	int max_deltat_lp;
	int max_type1_pw;		/* max fcc type 1 radar pulse width */
	int jp2_1_intv;		/* min fcc type 1 radar pulse repetition interval */
	int type1_intv;
	int max_type2_pw;
	int max_type2_intv;
	int min_type2_intv;
	int max_type4_pw;
	int min_type3_4_intv;
	int max_type3_4_intv;
	int max_jp1_2_pw;
	int jp1_2_intv;
	int jp2_3_intv;
} radar_params_t;

typedef struct {
	int length;
	uint32 tstart_list[2*RDR_LIST_SIZE];
	int width_list[2*RDR_LIST_SIZE];
	int fm_list[2*RDR_LIST_SIZE];
	int epoch_start[RDR_EPOCH_SIZE];
	int epoch_finish[RDR_EPOCH_SIZE];
	int tiern_list[RDR_NTIERS][RDR_NTIER_SIZE]; /* increased size of tiern list */
	int tiern_pw[RDR_NTIERS][RDR_NTIER_SIZE];
	int nepochs;
	int nphy_length[RDR_NANTENNAS];
	uint32 tstart_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	int width_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	int fm_list_n[RDR_NANTENNAS][RDR_LIST_SIZE];
	int nphy_length_bin5[RDR_NANTENNAS];
	uint32 tstart_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	int width_list_bin5[RDR_NANTENNAS][RDR_LIST_SIZE];
	int lp_length;
	uint32 lp_buffer[RDR_LP_BUFFER_SIZE];
	uint32 last_tstart;
	int lp_cnt;
	int lp_skip_cnt;
	int lp_pw_fm_matched;
	int lp_pw[3];
	int lp_fm[3];
	int lp_n_non_single_pulses;
	int lp_just_skipped;
	int lp_skipped_pw;
	int lp_skipped_fm;
	int lp_skip_tot;
	int lp_csect_single;
	uint32 lp_timer;
	uint32 dfs_timer;
	uint32 last_detection_time;
	uint32 last_detection_time_lp;
	uint32 last_skipped_time;
	int lp_len_his[LP_LEN_HIS_SIZE];
	int lp_len_his_idx;
	uint32 tstart_list_tail[2];
	int width_list_tail[2];
	int fm_list_tail[2];
} radar_work_t;

typedef struct _nphy_iq_comp {
	int16 a0;
	int16 b0;
	int16 a1;
	int16 b1;
} nphy_iq_comp_t;

typedef struct {
	uint16		txcal_interm_coeffs[11]; /* best coefficients */
	chanspec_t	txiqlocal_chanspec;	/* Last Tx IQ/LO cal chanspec */
	uint16 		txiqlocal_coeffs[11]; /* best coefficients */
	bool		txiqlocal_coeffsvalid;   /* bit flag */
} nphy_cal_result_t;

typedef struct {
	/* TX IQ LO cal results */
	uint16 txiqlocal_bestcoeffs[11];
	uint16 txiqlocal_bestcoeffs_valid;
	uint16 didq_cck;

	/* PAPD results */
	uint32 papd_eps_tbl[PHY_PAPD_EPS_TBL_SIZE_LPPHY];
	uint16 papd_indexOffset;
	uint16 papd_startstopindex;

	/* RX IQ cal results */
	uint16 rxiqcal_coeffs;
} lpphy_cal_results_t;

typedef struct {
	/* TX IQ LO cal results */
	uint16 txiqlocal_a[3];
	uint16 txiqlocal_b[3];
	uint16 txiqlocal_didq[3];
	uint8 txiqlocal_ei0;
	uint8 txiqlocal_eq0;
	uint8 txiqlocal_fi0;
	uint8 txiqlocal_fq0;
	/* TX IQ LO cal results */
	uint16 txiqlocal_bestcoeffs[11];
	uint16 txiqlocal_bestcoeffs_valid;

	/* PAPD results */
	uint32 papd_eps_tbl[PHY_PAPD_EPS_TBL_SIZE_LCNPHY];
	uint16 analog_gain_ref;
	uint16 lut_begin;
	uint16 lut_end;
	uint16 lut_step;
	uint16 rxcompdbm;
	uint16 papdctrl;
	uint16 sslpnCalibClkEnCtrl;

	/* RX IQ cal results */
	uint16 rxiqcal_coeff_a0;
	uint16 rxiqcal_coeff_b0;
} lcnphy_cal_results_t;

typedef struct {
	uint8	disable_temp; /* temp at which to drop to 1-Tx chain */
	uint8	disable_temp_max_cap;
	uint8	hysteresis;   /* temp hysteresis to enable multi-Tx chains */
	uint8	enable_temp;  /* temp at which to enable multi-Tx chains */
	bool	heatedup;     /* indicates if chip crossed tempthresh */
	uint8	bitmap;       /* upper/lower nibble is for rxchain/txchain */
	bool    degrade1RXen; /* 1-RX chain is enabled */
} phy_txcore_temp_t;

/* htphy: tx gain settings */
typedef struct {
	uint16 rad_gain; /* Radio gains */
	uint16 dac_gain; /* DAC attenuation */
	uint16 bbmult;   /* BBmult */
} txgain_setting_t;

/* Define a dummy htphy_cal_result structure to reduce memory footprint
 * since this struct is union'd with nphy
 */
#ifdef PHYCAL_CACHE_SMALL
typedef struct {
	uint16  txiqlocal_coeffs[1]; /* dummy structure */
	uint16  txiqlocal_interm_coeffs[1];
	bool    txiqlocal_coeffsvalid;
	uint8   txiqlocal_ladder_updated[1];
	txgain_setting_t cal_orig_txgain[1];
	txgain_setting_t txcal_txgain[1];
	chanspec_t chanspec;
} htphy_cal_result_t;
#else
typedef struct {
	uint16  txiqlocal_coeffs[20]; /* best ("final") comp coeffs (up to 4 cores) */
	uint16  txiqlocal_interm_coeffs[20]; /* intermediate comp coeffs */
	bool    txiqlocal_coeffsvalid;   /* bit flag */
	uint8   txiqlocal_ladder_updated[PHY_CORE_MAX]; /* up to 4 cores */
	txgain_setting_t cal_orig_txgain[PHY_CORE_MAX];
	txgain_setting_t txcal_txgain[PHY_CORE_MAX];
	chanspec_t chanspec;
} htphy_cal_result_t;
#endif /* PHYCAL_CACHE_SMALL */

typedef struct {
	uint8	cal_searchmode;
	uint8	cal_phase_id;	/* mphase cal state */
	uint8	txcal_cmdidx;
	uint8	txcal_numcmds;

	union {
		nphy_cal_result_t ncal;
		htphy_cal_result_t htcal;
	} u;

	uint       last_papd_cal_time; /* Used for LP Phy only till now */
	uint       last_cal_time; /* in [sec], covers 136 years if 32 bit */
	int16      last_cal_temp;
} phy_cal_info_t;

/* hold cached values for all channels, useful for debug */
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)

typedef struct {
	uint16 ofdm_txa[PHY_CORE_MAX];
	uint16 ofdm_txb[PHY_CORE_MAX];
	uint16 ofdm_txd[PHY_CORE_MAX]; /* contain di & dq */
	uint16 bphy_txa[PHY_CORE_MAX];
	uint16 bphy_txb[PHY_CORE_MAX];
	uint16 bphy_txd[PHY_CORE_MAX]; /* contain di & dq */
	uint8  txei[PHY_CORE_MAX];
	uint8  txeq[PHY_CORE_MAX];
	uint8  txfi[PHY_CORE_MAX];
	uint8  txfq[PHY_CORE_MAX];
	uint16 rxa[PHY_CORE_MAX];
	uint16 rxb[PHY_CORE_MAX];
} htphy_calcache_t;

typedef struct {
	uint16 txcal_coeffs[8];
	uint16 txcal_radio_regs[8];
	nphy_iq_comp_t rxcal_coeffs;
	uint16 rssical_radio_regs[2];
	uint16 rssical_phyregs[12];
} nphy_calcache_t;

typedef struct {
	uint16 lpphy_papd_tx_gain_at_last_cal; /* Tx gain index at time of last papd cal */
	uint16 lpphy_papd_cal_gain_index;	/* Tx gain index used during last papd cal */

	/* TX IQ LO cal results */
	uint16 txiqlocal_bestcoeffs[11];
	uint16 txiqlocal_bestcoeffs_valid;
	uint16 didq_cck;

	/* PAPD results */
	uint32 papd_eps_tbl[PHY_PAPD_EPS_TBL_SIZE_LPPHY];
	uint16 papd_indexOffset;
	uint16 papd_startstopindex;

	/* RX IQ cal results */
	uint16 rxiqcal_coeffs;
} lpphy_calcache_t;

typedef struct {
	uint16 lcnphy_gain_index_at_last_cal;

	/* TX IQ LO cal results */
	uint16 txiqlocal_a[3];
	uint16 txiqlocal_b[3];
	uint16 txiqlocal_didq[3];
	uint8 txiqlocal_ei0;
	uint8 txiqlocal_eq0;
	uint8 txiqlocal_fi0;
	uint8 txiqlocal_fq0;
	/* TX IQ LO cal results */
	uint16 txiqlocal_bestcoeffs[11];
	uint16 txiqlocal_bestcoeffs_valid;

	/* PAPD results */
	uint32 papd_eps_tbl[PHY_PAPD_EPS_TBL_SIZE_LCNPHY];
	uint16 analog_gain_ref;
	uint16 lut_begin;
	uint16 lut_end;
	uint16 lut_step;
	uint16 rxcompdbm;
	uint16 papdctrl;
	uint16 sslpnCalibClkEnCtrl;

	/* RX IQ cal results */
	uint16 rxiqcal_coeff_a0;
	uint16 rxiqcal_coeff_b0;
} lcnphy_calcache_t;

typedef struct ch_calcache {
	struct ch_calcache *next;
	bool valid;
	chanspec_t chanspec; /* which chanspec are these values for? */
	uint creation_time;
	union {
		htphy_calcache_t htphy_cache;
		nphy_calcache_t nphy_cache;
#ifndef PHYCAL_CACHE_SMALL
		lpphy_calcache_t lpphy_cache;
		lcnphy_calcache_t lcnphy_cache;
#endif
	} u;
	phy_cal_info_t cal_info;
} ch_calcache_t;
#endif /* PHYCAL_CACHING */

typedef struct {
	uint8 lna1;
	uint8 lna2;
	uint8 mix;
	uint8 lpf0;
	uint8 lpf1;
	uint8 dvga;
} rxgain_t;

typedef struct {
	uint16 rfctrlovrd;
	uint16 rxgain;
	uint16 lpfgain;
} rxgain_ovrd_t;

#define PHY_NOISEVAR_BUFSIZE 10	/* Increase this if we need to save more noise vars */

/* phy state that is per device instance */
struct shared_phy {
	struct phy_info	*phy_head;      /* head of phy list */
	uint	unit;			/* device instance number */
	osl_t	*osh;			/* pointer to os handle */
	si_t	*sih;			/* si handle (cookie for siutils calls) */
	void    *physhim;		/* phy <-> wl shim layer for wlapi */
	uint	corerev;		/* d11corerev, shadow of wlc_hw->corerev */
	uint32	machwcap;		/* mac hw capability */
	bool	up;			/* main driver is up and running */
	bool	clk;			/* main driver make the clk available */
	uint	now;			/* # elapsed seconds */
	uint16	vid;			/* vendorid */
	uint16  did;			/* deviceid */
	uint	chip;			/* chip number */
	uint	chiprev;		/* chip revision */
	uint	chippkg;		/* chip package option */
	uint	sromrev;		/* srom revision */
	uint	subband5Gver;		/* 5G subband partition, 1: legacy, 2: new */
	int8	noisecaloffset;		/* Noise cal offset */
	int8	noisevaroffset;		/* Noise var offset */
	uint	boardtype;		/* board type */
	uint	boardrev;		/* board revision */
	uint	boardvendor;		/* board vendor */
	uint32	boardflags;		/* board specific flags from srom */
	uint32	boardflags2;		/* more board flags if sromrev >= 4 */
	uint	bustype;		/* SI_BUS, PCI_BUS  */
	uint	buscorerev; 		/* buscore rev */
	uint	fast_timer;		/* Periodic timeout for 'fast' timer */
	uint	slow_timer;		/* Periodic timeout for 'slow' timer */
	uint	glacial_timer;		/* Periodic timeout for 'glacial' timer */
	int	interference_mode;	/* interference mitigation mode */
	int	interference_mode_2G;	/* 2G interference mitigation mode */
	int	interference_mode_5G;	/* 5G interference mitigation mode */
	int	interference_mode_2G_override;	/* 2G interference mitigation mode */
	int	interference_mode_5G_override;	/* 5G interference mitigation mode */
	bool	interference_mode_override;	/* override */
	uint8	rx_antdiv;		/* .11b Ant. diversity (rx) selection override */
	int8	phy_noise_window[MA_WINDOW_SZ];	/* noise moving average window */
	uint	phy_noise_index;	/* noise moving average window index */
	uint8	hw_phytxchain;	/* HW tx chain cfg */
	uint8	hw_phyrxchain;	/* HW rx chain cfg */
	uint8	phytxchain;	/* tx chain being used */
	uint8	phyrxchain;	/* rx chain being used */
	uint8	rssi_mode;
	bool	radar;		/* radar detection: on or off */
	bool	_rifs_phy;	/* per-pkt rifs flag passed down from wlc_info */
#ifdef WLMEDIA_TXFILTER_OVERRIDE
	int txfilter_sm_override; /* TX filter spectral mask override */
#endif
};

/* %%%%%% phy_info_t */
struct phy_pub {
	uint		phy_type;		/* PHY_TYPE_XX */
	uint		phy_rev;		/* phy revision */
	uint8		phy_corenum;		/* number of cores */
	uint16		radioid;		/* radio id */
	uint8		radiorev;		/* radio revision */
	uint8		radiover;		/* radio version */

	uint		coreflags;		/* sbtml core/phy specific flags */
	uint		ana_rev;		/* analog core revision */
	bool		abgphy_encore;			/* true if chipset is encore enabled */
};

struct phy_info_abgphy;
typedef struct phy_info_abgphy phy_info_abgphy_t;

struct phy_info_htphy;
typedef struct phy_info_htphy phy_info_htphy_t;

struct phy_info_nphy;
typedef struct phy_info_nphy phy_info_nphy_t;

struct phy_info_lcn40phy;
typedef struct phy_info_lcn40phy phy_info_lcn40phy_t;

struct phy_info_lcnphy;
typedef struct phy_info_lcnphy phy_info_lcnphy_t;

struct phy_info_lpphy;
typedef struct phy_info_lpphy phy_info_lpphy_t;

struct phy_info_sslpnphy;
typedef struct phy_info_sslpnphy phy_info_sslpnphy_t;

struct phy_func_ptr {
	initfn_t init;
	initfn_t calinit;
	chansetfn_t chanset;
	initfn_t txpwrrecalc;
	longtrnfn_t longtrn;
	txiqccgetfn_t txiqccget;
	txiqccsetfn_t txiqccset;
	txloccgetfn_t txloccget;
	txloccsetfn_t txloccset;
	radioloftgetfn_t radioloftget;
	radioloftsetfn_t radioloftset;
	initfn_t carrsuppr;
	rxsigpwrfn_t rxsigpwr;
	detachfn_t detach;
	txcorepwroffsetfn_t txcorepwroffsetget;
	txcorepwroffsetfn_t txcorepwroffsetset;
	settxpwrctrlfn_t    settxpwrctrl;
	gettxpwrctrlfn_t    gettxpwrctrl;
	ishwtxpwrctrlfn_t   ishwtxpwrctrl;
	settxpwrbyindexfn_t settxpwrbyindex;
	btcadjustfn_t       phybtcadjust;
	phywatchdogfn_t    phywatchdog;
	tssicalsweepfn_t   tssicalsweep;
	switchradiofn_t    switchradio;
	anacorefn_t        anacore;
	phywritetablefn_t  phywritetable;
	phyreadtablefn_t   phyreadtable;
	calibmodesfn_t     calibmodes;
#ifdef ENABLE_FCBS
	fcbsinitfn_t fcbsinit;
	fcbsprefn_t prefcbs;
	fcbspostfn_t postfcbs;
#endif /* ENABLE_FCBS */
};
typedef struct phy_func_ptr phy_func_ptr_t;

struct srom8_ppr {
	uint16	cck2gpo;	/* 2G CCK Power offset */
	uint32	ofdm2gpo;	/* 2G OFDM Power offset */
	uint32	ofdm5gpo;	/* 5G OFDM Power offset */
	uint32	ofdm5glpo;	/* 5GL OFDM Power offset */
	uint32	ofdm5ghpo;	/* 5GH OFDM Power offset */
#ifdef	WL_PPR_SUBBAND
	uint32	ofdm5gllpo;	/* 5GH OFDM Power offset */
	uint32	ofdm5glhpo;	/* 5GH OFDM Power offset */
	uint32	ofdm5gmlpo;	/* 5GH OFDM Power offset */
	uint32	ofdm5gmhpo;	/* 5GH OFDM Power offset */
#endif	/* WL_PPR_SUBBAND */
	uint8   bw402gpo; 	/* bw40 2g power offset */
	uint8   bw405gpo; 	/* bw40 5g mid power offset */
	uint8   bw405glpo; 	/* bw40 5g low power offset */
	uint8   bw405ghpo; 	/* bw40 5g high power offset */
#ifdef	WL_PPR_SUBBAND
	uint8   bw405gllpo; 	/* bw40 5g high power offset */
	uint8   bw405glhpo; 	/* bw40 5g high power offset */
	uint8   bw405gmlpo; 	/* bw40 5g high power offset */
	uint8   bw405gmhpo; 	/* bw40 5g high power offset */
#endif	/* WL_PPR_SUBBAND */
	uint8   cdd2gpo; 	/* cdd 2g power offset */
	uint8   cdd5gpo; 	/* cdd 5g mid power offset */
	uint8   cdd5glpo; 	/* cdd 5g low power offset */
	uint8   cdd5ghpo; 	/* cdd 5g high power offset */
#ifdef	WL_PPR_SUBBAND
	uint8   cdd5gllpo; 	/* cdd 5g high power offset */
	uint8   cdd5glhpo; 	/* cdd 5g high power offset */
	uint8   cdd5gmlpo; 	/* cdd 5g high power offset */
	uint8   cdd5gmhpo; 	/* cdd 5g high power offset */
#endif	/* WL_PPR_SUBBAND */
	uint8   stbc2gpo; 	/* stbc 2g power offset */
	uint8   stbc5gpo; 	/* stbc 5g mid power offset */
	uint8   stbc5glpo; 	/* stbc 5g low power offst */
	uint8   stbc5ghpo; 	/* stbc 5g high power offset */
#ifdef	WL_PPR_SUBBAND
	uint8   stbc5gllpo; 	/* stbc 5g high power offset */
	uint8   stbc5glhpo; 	/* stbc 5g high power offset */
	uint8   stbc5gmlpo; 	/* stbc 5g high power offset */
	uint8   stbc5gmhpo; 	/* stbc 5g high power offset */
#endif	/* WL_PPR_SUBBAND */
	uint8   bwdup2gpo; 	/* bwdup 2g power offset */
	uint8   bwdup5gpo; 	/* bwdup 5g mid power offset */
	uint8   bwdup5glpo; 	/* bwdup 5g low power offset */
	uint8   bwdup5ghpo; 	/* bwdup 5g high power offset */
#ifdef	WL_PPR_SUBBAND
	uint8   bwdup5gllpo; 	/* bwdup 5g high power offset */
	uint8   bwdup5glhpo; 	/* bwdup 5g high power offset */
	uint8   bwdup5gmlpo; 	/* bwdup 5g high power offset */
	uint8   bwdup5gmhpo; 	/* bwdup 5g high power offset */
#endif	/* WL_PPR_SUBBAND */
	uint16	mcs2gpo[8];	/* mcs 2g power offset */
	uint16	mcs5gpo[8];	/* mcs 5g mid power offset */
	uint16	mcs5glpo[8];	/* mcs 5g low power offset */
	uint16	mcs5ghpo[8];	/* mcs 5g high power offset */
#ifdef	WL_PPR_SUBBAND
	uint16	mcs5gllpo[8];	/* mcs 5g high power offset */
	uint16	mcs5glhpo[8];	/* mcs 5g high power offset */
	uint16	mcs5gmlpo[8];	/* mcs 5g high power offset */
	uint16	mcs5gmhpo[8];	/* mcs 5g high power offset */
#endif	/* WL_PPR_SUBBAND */
};

struct ppbw {
	uint32	bw20;
	uint32	bw20ul;
	uint32	bw40;
};

struct srom9_ppr {
	uint16	cckbw202gpo;	/* 2G CCK Power offset */
	uint16	cckbw20ul2gpo;	/* 2G CCK 20UL Power offset */
	struct ppbw ofdm2g;
	struct ppbw ofdm5gl;
	struct ppbw ofdm5gm;
	struct ppbw ofdm5gh;
	struct ppbw mcs2g;
	struct ppbw mcs5gl;
	struct ppbw mcs5gm;
	struct ppbw mcs5gh;
	uint16	mcs32po;	/* MCS32 power offset */
	uint16	ofdm40duppo;	/* OFMD DUP power offset */

	struct ppbw ofdm5gll;	/* new 5G subband division */
	struct ppbw ofdm5glh;
	struct ppbw ofdm5gml;
	struct ppbw ofdm5gmh;
	struct ppbw mcs5gll;
	struct ppbw mcs5glh;
	struct ppbw mcs5gml;
	struct ppbw mcs5gmh;
};

typedef struct _nphy_txgains {
	uint16 txlpf[2];
	uint16 txgm[2];
	uint16 pga[2];
	uint16 pad[2];
	uint16 ipa[2];
} nphy_txgains_t;

#ifdef	WL_PPR_SUBBAND
typedef	struct chan_poparam {
	int16	channel;
	int16	a1;
	int16	b0;
	int16	b1;
} chan_poparam_t;
#endif	/* WL_PPR_SUBBAND */

typedef struct srom9_pwrdet {
	int16  pwrdet_a1[PHY_CORE_MAX][4 + PHY_SUBBAND_NUM]; /* 2G, 5G_low, 5G_mid, 5G_high */
	int16  pwrdet_b0[PHY_CORE_MAX][4 + PHY_SUBBAND_NUM]; /* 2G, 5G_low, 5G_mid, 5G_high */
	int16  pwrdet_b1[PHY_CORE_MAX][4 + PHY_SUBBAND_NUM]; /* 2G, 5G_low, 5G_mid, 5G_high */
	int8   max_pwr[PHY_CORE_MAX][4 + PHY_SUBBAND_NUM]; /* 2G, 5G_low, 5G_mid, 5G_high */
#ifdef	WL_PPR_SUBBAND
	chan_poparam_t	*poparam[PHY_CORE_MAX];
#endif	/* WL_PPR_SUBBAND */
} srom9_pwrdet_t;

/* This structure is used to save/modify/restore the noise vars for specific tones */
typedef struct _nphy_noisevar_buf {
	int bufcount;   /* number of valid entries in the buffer */
	int tone_id[PHY_NOISEVAR_BUFSIZE];
	uint32 noise_vars[PHY_NOISEVAR_BUFSIZE];
	uint32 min_noise_vars[PHY_NOISEVAR_BUFSIZE];
} phy_noisevar_buf_t;

typedef struct {
	uint16 rssical_radio_regs_2G[2];
	uint16 rssical_phyregs_2G[12];

	uint16 rssical_radio_regs_5G[2];
	uint16 rssical_phyregs_5G[12];
} rssical_cache_t;

typedef struct {
	/* calibration coefficients for the last 2 GHz channel that was calibrated */
	uint16 txcal_coeffs_2G[8];
	uint16 txcal_radio_regs_2G[8];
	nphy_iq_comp_t rxcal_coeffs_2G;

	/* calibration coefficients for the last 5 GHz channel that was calibrated */
	uint16 txcal_coeffs_5G[8];
	uint16 txcal_radio_regs_5G[8];
	nphy_iq_comp_t rxcal_coeffs_5G;
} txiqcal_cache_t;

/* Fast channel band switch (FCBS) structures and definitions */
#ifdef ENABLE_FCBS

/* Current FCBS Version */
#define FCBS_VER	1

/* FCBS buffer types */
#define FCBS_BUFTYPE_RADIOREG	1
#define FCBS_BUFTYPE_PHYREG		2
#define FCBS_BUFTYPE_PHYTBL16	3
#define FCBS_BUFTYPE_PHYTBL32	4

/* Number of pseudo-simultaneous channels that we support */
#define MAX_FCBS_CHANS	2

/* Channel index of pseudo-simultaneous dual channels */
#define FCBS_CHAN_A	0
#define FCBS_CHAN_B 1

#define FCBS_TMPLRAM_STARTADDR	0x6b0
#define M_FCBS_RADIOREG			0x7a0
#define M_FCBS_PHYTBL16			0x7a2
#define M_FCBS_PHYTBL32			0x7a4
#define M_FCBS_PHYREG			0x7a6
#define M_FCBS_BPHYCTRL			0x7a8
#define M_FCBS_TEMPLATE_PTR		0x7aa

#define FCBS_BPHY_ON			0x3
#define FCBS_BPHY_OFF			0x1

typedef struct _fcbs_radioreg_buf_entry {
	uint16 addr;
	uint16 val;
} fcbs_radioreg_buf_entry;

typedef struct _fcbs_phyreg_buf_entry {
	uint16 addr;
	uint16 val;
} fcbs_phyreg_buf_entry;

typedef struct _fcbs_buf_header {
	uint16 version_buftype;
	uint16 payload_len;
} fcbs_buf_header;

typedef struct _fcbs_phytbl_info_tuple {
	uint16 tbl_addr;
	uint16 num_entries;
} fcbs_phytbl_info_tuple;

typedef struct _fcbs_phyreg_list_entry {
	uint16 regaddr;
} fcbs_phyreg_list_entry;

typedef struct _fcbs_radioreg_list_entry {
	uint16 regaddr;
	uint16 regval;
} fcbs_radioreg_list_entry;

typedef struct _fcbs_phytbl_list_entry {
	uint16 tbl_id;
	uint16 tbl_offset;
	uint16 num_entries;
} fcbs_phytbl_list_entry;

typedef struct _fcbs_info {
	chanspec_t					chanspec[MAX_FCBS_CHANS];
	bool						initialized[MAX_FCBS_CHANS];
	bool						use_rftuning_mem;
	int							curr_fcbs_chan;

	int							num_radio_regs;
	int							phytbl16_buflen;
	int							phytbl32_buflen;
	int							num_phy_regs;
	int							num_bphy_regs[MAX_FCBS_CHANS];
	int							phyreg_buflen[MAX_FCBS_CHANS];

	fcbs_radioreg_buf_entry		*radioreg_buf[MAX_FCBS_CHANS];
	uint16 						*phytbl16_buf[MAX_FCBS_CHANS];
	uint16 						*phytbl32_buf[MAX_FCBS_CHANS];
	fcbs_phyreg_buf_entry		*phyreg_buf[MAX_FCBS_CHANS];
	fcbs_phyreg_buf_entry		*bphyreg_buf[MAX_FCBS_CHANS];

	int				start_template_offset[MAX_FCBS_CHANS];
	int				radioreg_template_offset[MAX_FCBS_CHANS];
	int				phytbl16_template_offset[MAX_FCBS_CHANS];
	int				phytbl32_template_offset[MAX_FCBS_CHANS];
	int				phyreg_template_offset[MAX_FCBS_CHANS];
	int				bphyreg_template_offset[MAX_FCBS_CHANS];
} fcbs_info;

#endif /* ENABLE_FCBS */

struct phy_info {
	wlc_phy_t	pubpi_ro;	/* public attach time constant phy state */
	shared_phy_t	*sh;		/* shared phy state pointer */
	phy_func_ptr_t	pi_fptr;
	union {
		phy_info_lcn40phy_t *pi_lcn40phy;
		phy_info_lcnphy_t *pi_lcnphy;
		phy_info_lpphy_t *pi_lpphy;
		phy_info_sslpnphy_t *pi_sslpnphy;
		phy_info_abgphy_t *pi_abgphy;
		phy_info_nphy_t *pi_nphy;
		phy_info_htphy_t *pi_htphy;
	} u;
	bool	user_txpwr_at_rfport;

#ifdef WLNOKIA_NVMEM
	void	*noknvmem;
#endif /* WLNOKIA_NVMEM */
	d11regs_t	*regs;
	struct phy_info	*next;
	char		*vars;			/* phy attach time only copy of vars */
	wlc_phy_t	pubpi;			/* private attach time constant phy state */

	bool		phytest_on;		/* whether a PHY test is running */
	bool		ofdm_rateset_war;	/* ofdm rateset war */

	chanspec_t	radio_chanspec;		/* current radio chanspec */

	uint8		antsel_type;		/* Type of boardlevel mimo antenna switch-logic
						 * 0 = N/A, 1 = 2x4 board, 2 = 2x3 CB2 board
						 */
	uint16		bw;			/* b/w (10, 20 or 40) [only 20MHZ on non NPHY] */
	uint8		txpwr_percent;		/* power output percentage */
	bool		phy_init_por;		/* power on reset prior to phy init call */

	bool		init_in_progress;	/* init in progress */
	bool		initialized;		/* Have we been initialized ? */
	uint		refcnt;
	bool		phywatchdog_override;	/* to disable/enable phy watchdog */
	uint8		phynoise_state;		/* phy noise sample state */
	uint		phynoise_now;		/* timestamp to run sampling */
	int		phynoise_chan_watchdog;	/* sampling target channel for watchdog */
	bool		phynoise_polling;	/* polling or interrupt for sampling */
	bool		disable_percal;		/* phy agnostic iovar to disable watchdog cals */
	mbool		measure_hold;		/* should we hold off measurements/calibrations */

/* NVRAM values */
	/* PA tssi coefficiencies */
	int16		txpa_2g[PWRTBL_NUM_COEFF];		/* 2G: pa0b%d */
	int16		txpa_2g_lo[PWRTBL_NUM_COEFF];		/* For 2nd LUT */
	int16		txpa_5g_low[PWRTBL_NUM_COEFF];		/* 5G low: pa1lob%d */
	int16		txpa_5g_mid[PWRTBL_NUM_COEFF];		/* 5G mid: pa1b%d   */
	int16		txpa_5g_hi[PWRTBL_NUM_COEFF];		/* 5G hi:  pa1hib%d */

	/* srom max board value (.25 dBm) */
	uint8		tx_srom_max_2g;				/* 2G:     pa0maxpwr   */
	uint8		tx_srom_max_5g_low;			/* 5G low: pa1lomaxpwr */
	uint8		tx_srom_max_5g_mid;			/* 5G mid: pa1maxpwr   */
	uint8		tx_srom_max_5g_hi;			/* 5G hi:  pa1himaxpwr */
#ifdef	WL_PPR_SUBBAND
	uint8		tx_srom_max_5g_ll;			/* 5G low-lo:  pa1llmaxpwr */
	uint8		tx_srom_max_5g_lh;			/* 5G low-hi:  pa1lhmaxpwr */
	uint8		tx_srom_max_5g_ml;			/* 5G mid-lo:  pa1mlmaxpwr */
	uint8		tx_srom_max_5g_mh;			/* 5G mid-hi:  pa1mhmaxpwr */
#endif	/* WL_PPR_SUBBAND */
	uint8		tx_srom_max_rate_2g[TXP_NUM_RATES];	/* 2.4G channels */
	uint8		tx_srom_max_rate_5g_low[TXP_NUM_RATES];	/* 5G mid band channels */
	uint8		tx_srom_max_rate_5g_mid[TXP_NUM_RATES];	/* 5G low band channels */
	uint8		tx_srom_max_rate_5g_hi[TXP_NUM_RATES];	/* 5G hi band channels */
#ifdef	WL_PPR_SUBBAND
	uint8		tx_srom_max_rate_5g_ll[TXP_NUM_RATES];	/* 5G low-lo band channels */
	uint8		tx_srom_max_rate_5g_lh[TXP_NUM_RATES];	/* 5G low-hi band channels */
	uint8		tx_srom_max_rate_5g_ml[TXP_NUM_RATES];	/* 5G mid-lo band channels */
	uint8		tx_srom_max_rate_5g_mh[TXP_NUM_RATES];	/* 5G mid-hi band channels */
#endif	/* WL_PPR_SUBBAND */
	uint8		tx_user_target[TXP_NUM_RATES];		/* Current user override target */
	int8		tx_power_offset[TXP_NUM_RATES];		/* Offset from base power */
	uint8		tx_power_target[TXP_NUM_RATES];		/* Current target power */

	srom_fem_t	srom_fem2g;		/* 2G band FEM attributes */
	srom_fem_t	srom_fem5g;		/* 5G band FEM attributes */

	/* Gain errors measured from phy_rxiqest and stored in srom: */
	int8 srom_rxgainerr_2g[PHY_CORE_MAX];          /* 2G channels */
	bool srom_rxgainerr2g_isempty;
	int8 srom_rxgainerr_5gl[PHY_CORE_MAX];         /* 5G-low channels */
	bool srom_rxgainerr5gl_isempty;
	int8 srom_rxgainerr_5gm[PHY_CORE_MAX];         /* 5G-mid channels */
	bool srom_rxgainerr5gm_isempty;
	int8 srom_rxgainerr_5gh[PHY_CORE_MAX];         /* 5G-high channels */
	bool srom_rxgainerr5gh_isempty;
	int8 srom_rxgainerr_5gu[PHY_CORE_MAX];         /* 5G-upper channels */
	bool srom_rxgainerr5gu_isempty;

	/* Gain-corrected noise-levels (dBm) from SROM (measured using phy_rxiqest): */
	int8 srom_noiselvl_2g[PHY_CORE_MAX];          /* 2G channels */
	int8 srom_noiselvl_5gl[PHY_CORE_MAX];         /* 5G-low channels */
	int8 srom_noiselvl_5gm[PHY_CORE_MAX];         /* 5G-mid channels */
	int8 srom_noiselvl_5gh[PHY_CORE_MAX];         /* 5G-high channels */
	int8 srom_noiselvl_5gu[PHY_CORE_MAX];         /* 5G-upper channels */

	/* Rxgainerror and noise level temperature at calibration */
	int16 srom_rawtempsense;

	int8 phy_rssi_gain_error[PHY_CORE_MAX];		/* per-core gain-error for rssi
							 * measured on current channel
							 */

/* PHY states */
	uint8		tx_power_max;		/* max target power among all rates on this channel.
						 * Power offsets use this.
						 */
	uint8		tx_power_max_rate_ind;  /* Index of the rate amongst the TXP_NUM_RATES rates
						 * with the max target power on this channel. If
						 * multiple rates have the max target power, the
						 * lowest rate index among them is reported.
						 */
	bool		hwpwrctrl;		/* ucode controls txpower instead of driver */
	uint8		nphy_txpwrctrl;		/* tx power control setting */
	uint8		nphy_oclscd;		/* ocl scd settting */
	int8		nphy_txrx_chain;	/* chain override for both TX & RX */
	bool		phy_5g_pwrgain;	/* flag to indicate 5G Power Gain is enabled */

	uint16		phy_wreg;
	uint16		phy_wreg_limit;

	int8		n_preamble_override;	/* preamble override for both TX & RX, both band */
	uint8		antswitch;              /* Antswitch field from SROM */
	uint8		aa2g, aa5g;             /* antennas available for 2G, 5G */

	int8		txpwr_est_Pout;			/* Best guess at current txpower */
	uint8		tx_power_min;			/* min power for all rates on this chan */
	int8		openlp_tx_power_min;
	bool		openlp_tx_power_on;
	uint8		txpwr_limit[TXP_NUM_RATES];	/* regulatory power limit */
	uint8		txpwr_env_limit[TXP_NUM_RATES];	/* env based txpower per rate limit */
	uint8		ldo_voltage;
	bool		txpwroverride;			/* override */
	bool		txpwrnegative;
	int16		radiopwr_override;		/* phy PWR_CTL override, -1=default */
	uint16		hwpwr_txcur;			/* hwpwrctl: current tx power index */
	uint8		saved_txpwr_idx;		/* saved current hwpwrctl txpwr index */
	interference_info_t interf;
	int16 saved_tempsense;
	bool saved_tempsense_valid;

	bool	edcrs_threshold_lock;	/* lock the edcrs detection threshold */

#if defined(AP) && defined(RADAR)
	radar_work_t	radar_work;		/* radar work area */
	phy_radar_detect_mode_t rdm;            /* current radar detect mode FCC/EU */
#endif
	radar_params_t	*rparams;
	radar_params_t	rargs[2];	/* different parameters for 20Mhz, 40Mhz channel */

	int16	ofdm_analog_filt_bw_override;
	int16	ofdm_analog_filt_bw_override_2g;
	int16	ofdm_analog_filt_bw_override_5g;
	int16	cck_analog_filt_bw_override;
	int16	ofdm_rccal_override;
	int16	cck_rccal_override;

	uint	interference_mode_crs_time; /* time at which crs was turned off */
	uint16	crsglitch_prev;		/* crsglitch count at last watchdog */
	bool	interference_mode_crs;	/* aphy crs state for interference mitigation mode */
	uint	aci_start_time;		/* adjacent channel interference start time */
	int	aci_exit_check_period;
	uint	aci_state;

	int32  phy_tx_tone_freq;
	uint	phy_lastcal;   /* last time PHY periodic calibration ran */
	bool 	phy_forcecal; /* run calibration at the earliest opportunity */
	bool    phy_fixed_noise; /* flag to report PHY_NOISE_FIXED_VAL noise */

	int16  noise_level_dBm;
	uint32	xtalfreq; /* Xtal frequency */
	int8	carrier_suppr_disable;	/* disable carrier suppression */

	bool	phy_bphy_evm;     /* flag to report if continuous CCK transmission is ON/OFF */
	bool	phy_bphy_rfcs;  /* flag to report if nphy BPHY RFCS testpattern is ON/OFF */
	int8	phy_scraminit;
	uint16	phy_gpiosel;

	/* tempsense */
	phy_txcore_temp_t txcore_temp;
	int8	phy_tempsense_offset;

	/* 11a Power control */
	int8		txpwridx;
	uint8		min_txpower;	/* minimum allowed tx power */

	/* Original values of the PHY regs, that we modified, to increase the freq tracking b/w */
	uint16		freqtrack_saved_regs [2];
	int		cur_interference_mode;	/* Track interference mode of phy */
	bool 		hwpwrctrl_capable;

	uint	phycal_txpower;		/* last time txpower calibration was done */

	union {
		struct srom8_ppr sr8;
		struct srom9_ppr sr9;
	} ppr;

	srom9_pwrdet_t	pwrdet;

	int8	phy_spuravoid;      /* spur avoidance, 0: disable, 1: auto, 2: on, 3: on2 */
	int16	phy_noise_win[PHY_CORE_MAX][PHY_NOISE_WINDOW_SZ]; /* noise per antenna */
	uint8	phy_noise_index;	/* noise moving average window index */

	phy_cal_info_t *cal_info;	/* Multiple instances of Multi-phase support */

#if defined(WLMCHAN) || defined(PHYCAL_CACHING)
	ch_calcache_t *phy_calcache;	/* Head of the list */
	uint8 phy_calcache_num;		/* Indicates the num of active contexts */
	bool phy_calcache_on;
#endif

	bool	ipa2g_on;   /* using 2G internal PA */
	bool	ipa5g_on;   /* using internal PA */

	/* variables for saving classifier and clip detect registers */
	uint16	phy_classifier_state;
	uint16	phy_clip_state[2];
	uint8	phy_rxiq_samps;
	uint8	phy_rxiq_antsel;
	uint8   phy_rxiq_resln;
	uint8   phy_rxiq_lpfhpc;        /* lpf_hpc override select for rxiqest */
	uint8   phy_rxiq_diglpf;        /* rx dig_lpf override select for rxiqest */
	uint8   phy_rxiq_gain_correct;  /* enable/disable (1/0)  gain-correction
					 * when reporting powers in rxiqest
					 */
	uint16  phy_rx_diglpf_default_coeffs[10];
	bool    phy_rx_diglpf_default_coeffs_valid; /* it should be initialized to FALSE */
	/* Need higher gain for proper noise measurement */
	uint8   phy_rxiq_extra_gain_3dB;    /* INITgain += (extra_gain_3dB * 3) */

	bool	first_cal_after_assoc;

	/* new flag to signal periodic_cal is running to blank radar */
	uint16 radar_percal_mask;

	bool	radio_is_on;
	uint8	phy_cal_mode;		/* periodic calibration mode: disable, single, mphase */
	/* Phy table addr/data register offsets */
	uint16 tbl_data_hi;
	uint16 tbl_data_lo;
	uint16 tbl_addr;
	/* Phy table addr/data split access state */
	uint tbl_save_id;
	uint tbl_save_offset;
	uint8 phycal_tempdelta; /* temperature delta below which phy calibration will not run */
	uint8 papdcal_indexdelta; /* txindex delta below which papd calibration will not run */

	uint8	txpwrctrl; /* tx power control setting */
	bool    btclock_tune;			/* WAR to stabilize btclock  */
	bool    bt_active;
	uint16  bt_period;
	uint16  bt_shm_addr;

	uint16	old_bphy_test;
	uint16	old_bphy_testcontrol;

	/* high channels in a band to be disabled for srom ver 1 */
	uint8		a_band_high_disable;

	/* tssi to dbm translation table */
	uint8		*hwtxpwr;

#ifdef PR43338WAR
	bool	war_b_ap;
	uint16	war_b_ap_cthr_save;	/* b only AP WAR to cache cthr bit 14 */
#endif
	phy_cal_info_t def_cal_info;	/* Default cal info (not allocated) */
	struct wlapi_timer *phycal_timer; /* timer for multiphase cal, can be generalized later */
	int8	nphy_rssisel;

	bool	nphy_gain_boost; /* flag to reduce 2055 NF via higher LNA gain */
	bool	nphy_elna_gain_config; /* flag to reduce Rx gains for external LNA */

	/* nphy calibration */
	bool	nphy_rssical;		/* enable/disable nphy rssical(for rev3 only for now) */

	bool dfs_lp_buffer_nphy;		/* enable/disable clearing DFS LP buffer */

/* debug */
#if defined(WLTEST)
	int8	nphy_tbldump_minidx;
	int8	nphy_tbldump_maxidx;
	uint	nphy_phyreg_skipaddr[128];
	int8	nphy_phyreg_skipcnt;
#endif

#if defined(BCMDBG) || defined(WLTEST)
	uint16	car_sup_phytest;	/* Save phytest */

	/* Used in wlc_evm() */
	uint16	evm_phytest;		/* Save phytest */
	uint32	evm_o;			/* GPIO output */
	uint32	evm_oe;			/* GPIO Output Enables */

	/* Used in wlc_init_test() */
	uint16	tr_loss_ctl;
	uint16	rf_override;

#endif 

	int8 tx_pwr_backoff;  /* qdBm */

	uint16 tx_alpf_bypass;	/* nvram var, bypass tx analog lpf */
	uint16 tx_alpf_bypass_2g;	/* nvram var, bypass tx analog lpf */
	uint16 tx_alpf_bypass_5g;	/* nvram var, bypass tx analog lpf */
	uint16 bphy_scale; /* nvram var, force bphy_scale register */
	tssi_cal_info_t *ptssi_cal;
	uint8 ucode_tssi_limit_en;
	int8 	rssi0_buffer[16];
	int8 	rssi1_buffer[16];
	int8  rssi0_avg;
	int8  rssi1_avg;
	int8  rssi0_index;
	int8  rssi1_index;
	int16	txpa_2g_low_temp[PWRTBL_NUM_COEFF];	/* low temperature  */
	int16	txpa_2g_high_temp[PWRTBL_NUM_COEFF];	/* high temperature */
	uint32 sslpnphy_mcs40_po;
	uint32 sslpnphy_mcs20_po;
	/* for delta thresholds iovar */
	uint8 txidx_delta_threshold;
	uint8 temp_delta_threshold;
	uint8 papd_txidx_delta_threshold;
	uint8 papd_temp_delta_threshold;
	int16 tx_alpf_bypass_cck_2g;
	bool nphy_enable_hw_antsel;

#ifdef ENABLE_FCBS
	/* Fast channel/band switch (FCBS) data */
	fcbs_info phy_fcbs;
#endif /* ENABLE_FCBS */
#ifdef WLMEDIA_APCS
	bool dcs_skip_papd_recal;
	int dcs_papd_delay;
#endif /* WLMEDIA_APCS */
	uint16 fabid;
	int8 rssi_corr_normal;
	int8 rssi_corr_boardatten;

};


/* %%%%%% shared functions */

typedef int32    fixed;	/* s15.16 fixed-point */

typedef struct _cint32 {
	fixed	q;
	fixed	i;
} cint32;

typedef struct radio_regs {
    uint16 address;
    uint32 init_a;
    uint32 init_g;
    uint8  do_init_a;
    uint8  do_init_g;
} radio_regs_t;

/* radio regs that do not have band-specific values */
typedef struct radio_20xx_regs {
	uint16 address;
	uint8  init;
	uint8  do_init;
} radio_20xx_regs_t;

typedef struct radio_20671_regs {
	uint16 address;
	uint16 init;
	uint8  do_init;
} radio_20671_regs_t;

typedef struct sslpnphy_radio_regs {
    uint16 address;
    uint8 init_a;
    uint8 init_g;
} sslpnphy_radio_regs_t;
typedef struct lcnphy_radio_regs {
    uint16 address;
    uint8 init_a;
	uint8 init_g;
	uint8 do_init_a;
	uint8 do_init_g;
} lcnphy_radio_regs_t;

typedef struct lcn40phy_radio_regs {
    uint16 address;
    uint16 init_a;
	uint16 init_g;
	uint8 do_init_a;
	uint8 do_init_g;
} lcn40phy_radio_regs_t;

extern radio_regs_t regs_2063_rev0[];
extern radio_regs_t regs_2062[];
extern radio_regs_t regs_2063_rev0[];
extern radio_regs_t regs_2063_rev1[];
extern sslpnphy_radio_regs_t sslpnphy_radio_regs_2063[];
extern lcnphy_radio_regs_t lcnphy_radio_regs_2064[];
extern lcnphy_radio_regs_t lcnphy_radio_regs_2066[];
extern radio_regs_t regs_2055[], regs_SYN_2056[], regs_TX_2056[], regs_RX_2056[];
extern radio_regs_t regs_SYN_2056_A1[], regs_TX_2056_A1[], regs_RX_2056_A1[];
extern radio_regs_t regs_SYN_2056_rev5[], regs_TX_2056_rev5[], regs_RX_2056_rev5[];
extern radio_regs_t regs_SYN_2056_rev6[], regs_TX_2056_rev6[], regs_RX_2056_rev6[];
extern radio_regs_t regs_SYN_2056_rev7[], regs_TX_2056_rev7[], regs_RX_2056_rev7[];
extern radio_regs_t regs_SYN_2056_rev8[], regs_TX_2056_rev8[], regs_RX_2056_rev8[];
extern radio_20xx_regs_t regs_2057_rev4[], regs_2057_rev5[], regs_2057_rev5v1[];
extern radio_20xx_regs_t regs_2057_rev7[], regs_2057_rev8[], regs_2057_rev9[], regs_2057_rev10[];
extern radio_20xx_regs_t regs_2057_rev12[];
extern radio_20xx_regs_t regs_2059_rev0[];
extern radio_20671_regs_t regs_20671_rev0[];


/* %%%%%% utilities */
#ifdef BCMDBG
extern char *phy_getvar(phy_info_t *pi, const char *name, const char *function);
extern char *phy_getvar_fabid(phy_info_t *pi, const char *name, const char *function);
extern int phy_getintvar(phy_info_t *pi, const char *name, const char *function);
extern int phy_getintvar_default(phy_info_t *pi, const char *name, int default_value);
extern int phy_getintvararray(phy_info_t *pi, const char *name, int idx, const char *function);
extern int phy_getintvararray_default(phy_info_t *pi, const char *name, int idx,
	int default_value, const char *function);
#define PHY_GETVAR(pi, name) phy_getvar_fabid(pi, name, __FUNCTION__)
/* Search the vars for a specific one and return its value as an integer. Returns 0 if not found */
#define PHY_GETINTVAR(pi, name) phy_getintvar(pi, name, __FUNCTION__)
#define PHY_GETINTVAR_DEFAULT(pi, name, default_value) \
	phy_getintvar_default(pi, name, default_value)
#define PHY_GETINTVAR_ARRAY(pi, name, idx) \
	phy_getintvararray(pi, name, idx, __FUNCTION__)
#define PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, default_value) \
	phy_getintvararray_default(pi, name, idx, default_value, __FUNCTION__)
#else
extern char *phy_getvar(phy_info_t *pi, const char *name);
extern char *phy_getvar_fabid(phy_info_t *pi, const char *name);
extern int phy_getintvar(phy_info_t *pi, const char *name);
extern int phy_getintvar_default(phy_info_t *pi, const char *name, int default_value);
extern int phy_getintvararray(phy_info_t *pi, const char *name, int idx);
extern int phy_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value);
#define PHY_GETVAR(pi, name)	phy_getvar_fabid(pi, name)
#define PHY_GETINTVAR(pi, name)	phy_getintvar(pi, name)
#define PHY_GETINTVAR_DEFAULT(pi, name, default_value) \
	phy_getintvar_default(pi, name, default_value)
#define PHY_GETINTVAR_ARRAY(pi, name, idx) \
	phy_getintvararray(pi, name, idx)
#define PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, default_value) \
	phy_getintvararray_default(pi, name, idx, default_value)
#endif /* BCMDBG */

extern uint16 phy_reg_read(phy_info_t *pi, uint16 addr);
extern void phy_reg_write(phy_info_t *pi, uint16 addr, uint16 val);
extern void phy_reg_and(phy_info_t *pi, uint16 addr, uint16 val);
extern void phy_reg_or(phy_info_t *pi, uint16 addr, uint16 val);
extern void phy_reg_mod(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val);
extern void phy_reg_gen(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data);

extern uint16 read_radio_reg(phy_info_t *pi, uint16 addr);
extern void or_radio_reg(phy_info_t *pi, uint16 addr, uint16 val);
extern void and_radio_reg(phy_info_t *pi, uint16 addr, uint16 val);
extern void mod_radio_reg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val);
extern void xor_radio_reg(phy_info_t *pi, uint16 addr, uint16 mask);
extern void gen_radio_reg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data);

extern void write_radio_reg(phy_info_t *pi, uint16 addr, uint16 val);

extern void wlc_phyreg_enter(wlc_phy_t *pih);
extern void wlc_phyreg_exit(wlc_phy_t *pih);
extern void wlc_radioreg_enter(wlc_phy_t *pih);
extern void wlc_radioreg_exit(wlc_phy_t *pih);

extern void wlc_phy_read_table(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDatalo);
extern void wlc_phy_write_table(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDatalo);
extern void wlc_phy_table_addr(phy_info_t *pi, uint tbl_id, uint tbl_offset,
	uint16 tblAddr, uint16 tblDataHi, uint16 tblDataLo);
extern void wlc_phy_table_data_write(phy_info_t *pi, uint width, uint32 val);

extern void wlc_phy_common_read_table(phy_info_t *pi, uint32 tbl_id,
	const void *tbl_ptr, uint32 tbl_len, uint32 tbl_width, uint32 tbl_offset,
	void (*tbl_rfunc)(phy_info_t *, phytbl_info_t *));

extern void wlc_phy_common_write_table(phy_info_t *pi, uint32 tbl_id,
	const void *tbl_ptr, uint32 tbl_len, uint32 tbl_width, uint32 tbl_offset,
	void (*tbl_wfunc)(phy_info_t *, const phytbl_info_t *));

extern void write_phy_channel_reg(phy_info_t *pi, uint val);
extern void wlc_phy_txpower_update_shm(phy_info_t *pi);

extern void wlc_phy_cordic(fixed theta, cint32 *val);
extern uint8 wlc_phy_nbits(int32 value);
extern uint32 wlc_phy_sqrt_int(uint32 value);
extern void wlc_phy_compute_dB(uint32 *cmplx_pwr, int8 *p_dB, uint8 core);
extern uint32 wlc_phy_gcd(uint32 bigger, uint32 smaller);

extern uint wlc_phy_init_radio_regs_allbands(phy_info_t *pi, radio_20xx_regs_t *radioregs);
extern uint wlc_phy_init_radio_regs(phy_info_t *pi, radio_regs_t *radioregs,
	uint16 core_offset);

extern void wlc_phy_txpower_ipa_upd(phy_info_t *pi);

extern void wlc_phy_do_dummy_tx(phy_info_t *pi, bool ofdm, bool pa_on);
extern void wlc_phy_papd_decode_epsilon(uint32 epsilon, int32 *eps_real, int32 *eps_imag);

extern void wlc_phy_cal_perical_mphase_reset(phy_info_t *pi);
extern void wlc_phy_cal_perical_mphase_restart(phy_info_t *pi);
extern void wlc_btcx_override_enable(phy_info_t *pi);
extern bool wlc_phy_no_cal_possible(phy_info_t *pi);
extern void wlc_phy_get_paparams_for_band(phy_info_t *pi, int32 *a1, int32 *b0, int32 *b1);
extern  phy_info_lcnphy_t *wlc_phy_getlcnphy_common(phy_info_t *pi);
extern uint16 wlc_txpwrctrl_lcncommon(phy_info_t *pi);
extern int wlc_iovar_txpwrindex_set_lcncommon(phy_info_t *pi, int8 siso_int_val);

/* %%%%%% common flow function */
extern void wlc_set_uninitted_nphy(phy_info_t *pi);
extern bool wlc_phy_attach_nphy(phy_info_t *pi);
extern bool wlc_phy_attach_htphy(phy_info_t *pi);
extern bool wlc_phy_attach_lpphy(phy_info_t *pi);
extern bool wlc_phy_attach_sslpnphy(phy_info_t *pi);
extern bool wlc_phy_attach_lcnphy(phy_info_t *pi, int bandtype);
extern bool wlc_phy_attach_abgphy(phy_info_t *pi, int bandtype);
extern bool wlc_phy_attach_lcn40phy(phy_info_t *pi);
extern void wlc_set_uninitted_abgphy(phy_info_t *pi);

extern void wlc_phy_detach_lcnphy(phy_info_t *pi);
extern void wlc_phy_detach_lpphy(phy_info_t *pi);

extern void wlc_phy_init_nphy(phy_info_t *pi);
extern void wlc_phy_init_htphy(phy_info_t *pi);
extern void wlc_phy_init_lpphy(phy_info_t *pi);
extern void wlc_phy_init_sslpnphy(phy_info_t *pi);
extern void wlc_phy_init_lcnphy(phy_info_t *pi);
extern void WLBANDINITFN(wlc_phy_init_aphy)(phy_info_t *pi);
extern void WLBANDINITFN(wlc_phy_init_gphy)(phy_info_t *pi);

extern void wlc_phy_cal_init_nphy(phy_info_t *pi);
extern void wlc_phy_cal_init_htphy(phy_info_t *pi);
extern void wlc_phy_cal_init_lpphy(phy_info_t *pi);
extern void wlc_phy_cal_init_sslpnphy(phy_info_t *pi);
extern void wlc_phy_cal_init_lcnphy(phy_info_t *pi);
extern void wlc_phy_cal_init_gphy(phy_info_t *pi);

extern void wlc_phy_chanspec_set_nphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_chanspec_set_htphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_chanspec_set_abgphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_chanspec_set_lpphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_sslpnphy_percal_flags_off(phy_info_t *pi);
extern void wlc_phy_chanspec_set_sslpnphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_chanspec_set_fixup_sslpnphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_chanspec_set_lcnphy(phy_info_t *pi, chanspec_t chanspec);
extern void wlc_phy_chanspec_set_fixup_lcnphy(phy_info_t *pi, chanspec_t chanspec);
extern int  wlc_phy_channel2freq(uint channel);
extern uint wlc_phy_channel2idx(uint channel);
extern int  wlc_phy_chanspec_freq2bandrange_lpssn(uint);
extern int  wlc_phy_chanspec_bandrange_get(phy_info_t*, chanspec_t);

extern void wlc_phy_set_tx_pwr_ctrl_lpphy(phy_info_t *pi, uint16 mode);
extern void wlc_sslpnphy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode);
extern void wlc_lcnphy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode);
extern int8 wlc_lcnphy_get_current_tx_pwr_idx(phy_info_t *pi);
extern int8 wlc_lcn40phy_get_current_tx_pwr_idx(phy_info_t *pi);

extern void wlc_phy_txpower_recalc_target_nphy(phy_info_t *pi);
extern void wlc_phy_txpower_recalc_target_htphy(phy_info_t *pi);
extern void wlc_phy_txpower_recalc_target_lpphy(phy_info_t *pi);
extern void wlc_sslpnphy_txpower_recalc_target(phy_info_t *pi);
extern void wlc_lcnphy_txpower_recalc_target(phy_info_t *pi);
extern int wlc_lcnphy_idle_tssi_est_iovar(phy_info_t *pi, bool type);
extern void wlc_phy_txpower_recalc_target_sslpnphy(phy_info_t *pi);
extern void wlc_phy_txpower_recalc_target_lcnphy(phy_info_t *pi);

extern bool wlc_phy_cal_txpower_recalc_sw_abgphy(phy_info_t *pi);

extern bool wlc_phy_aci_scan_gphy(phy_info_t *pi);
extern void wlc_phy_aci_interf_nwlan_set_gphy(phy_info_t *pi, bool on);
extern void wlc_phy_aci_ctl_gphy(phy_info_t *pi, bool on);
extern void wlc_phy_aci_upd_nphy(phy_info_t *pi);
extern void wlc_phy_aci_ctl_nphy(phy_info_t *pi, bool enable, int aci_pwr);
extern void wlc_phy_aci_inband_noise_reduction_nphy(phy_info_t *pi, bool on, bool raise);

extern void wlc_phy_aci_sw_reset_nphy(phy_info_t *pi);
extern void wlc_phy_noisemode_reset_nphy(phy_info_t *pi);
extern void wlc_phy_acimode_reset_nphy(phy_info_t *pi); /* reset ACI mode */
extern void wlc_phy_aci_noise_upd_nphy(phy_info_t *pi);
extern void wlc_phy_acimode_upd_nphy(phy_info_t *pi);
extern void wlc_phy_noisemode_upd_nphy(phy_info_t *pi);
extern void wlc_phy_acimode_set_nphy(phy_info_t *pi, bool aci_miti_enable, int aci_pwr);
extern void wlc_phy_aci_init_nphy(phy_info_t *pi);
extern void wlc_phy_aci_enable_lpphy(phy_info_t *pi, bool on);
extern void wlc_phy_aci_upd_lpphy(phy_info_t *pi);
extern void wlc_phy_aci_sw_reset_htphy(phy_info_t *pi);
extern void wlc_phy_noisemode_reset_htphy(phy_info_t *pi);
extern void wlc_phy_acimode_reset_htphy(phy_info_t *pi); /* reset ACI mode */
extern void wlc_phy_aci_noise_upd_htphy(phy_info_t *pi);
extern void wlc_phy_acimode_upd_htphy(phy_info_t *pi);
extern void wlc_phy_noisemode_upd_htphy(phy_info_t *pi);
extern void wlc_phy_acimode_set_htphy(phy_info_t *pi, bool aci_miti_enable, int aci_pwr);
extern void wlc_phy_aci_init_htphy(phy_info_t *pi);
extern void wlc_phy_aci_noise_reset_nphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc);
extern void wlc_phy_aci_noise_reset_htphy(phy_info_t *pi, uint channel, bool clear_aci_state,
	bool clear_noise_state, bool disassoc);
extern void wlc_phy_clip_det_nphy(phy_info_t *pi, uint8 write, uint16 *vals);

extern void wlc_phy_periodic_cal_lpphy(phy_info_t *pi);
extern void wlc_phy_papd_cal_txpwr_lpphy(phy_info_t *pi, bool full_cal);
extern void wlc_phy_set_deaf_lpphy(phy_info_t *pi, bool user_flag);
extern void wlc_phy_clear_deaf_lpphy(phy_info_t *pi, bool user_flag);
extern void wlc_phy_stop_tx_tone_lpphy(phy_info_t *pi);
extern void wlc_phy_start_tx_tone_lpphy(phy_info_t *pi, int32 f_kHz, uint16 max_val);
extern void wlc_phy_tx_pu_lpphy(phy_info_t *pi, bool bEnable);
extern void BCMROMFN(wlc_sslpnphy_set_tx_pwr_by_index)(phy_info_t *pi, int indx);
extern void BCMROMFN(wlc_sslpnphy_tx_pu)(phy_info_t *pi, bool bEnable);
extern void BCMROMFN(wlc_sslpnphy_stop_tx_tone)(phy_info_t *pi);
extern void wlc_sslpnphy_start_tx_tone(phy_info_t *pi, int32 f_kHz, uint16 max_val, bool iqcalmode);
extern void wlc_lcnphy_set_tx_pwr_by_index(phy_info_t *pi, int indx);
extern void wlc_lcnphy_tx_pu(phy_info_t *pi, bool bEnable);
extern void wlc_lcnphy_stop_tx_tone(phy_info_t *pi);
extern void wlc_lcnphy_start_tx_tone(phy_info_t *pi, int32 f_kHz, uint16 max_val, bool iqcalmode);
extern void wlc_lcnphy_set_tx_tone_and_gain_idx(phy_info_t *pi);
extern void wlc_lcnphy_set_radio_loft(phy_info_t *pi, uint8 ei0, uint8 eq0, uint8 fi0, uint8 fq0);
extern void wlc_sslpnphy_force_adj_gain(phy_info_t *pi, bool on, int mode);
extern void wlc_sslpnphy_aci(phy_info_t *pi, bool on);
extern void wlc_phy_txpower_sromlimit_get_nphy(phy_info_t *pi, uint chan, uint8 *max_pwr,
	uint8 rate_id);
extern void wlc_phy_txpower_sromlimit_get_htphy(phy_info_t *pi, uint chan, uint8 *max_pwr,
	uint8 rate_id);
extern void wlc_phy_ofdm_to_mcs_powers_nphy(uint8 *power, uint8 rate_mcs_start, uint8 rate_mcs_end,
	uint8 rate_ofdm_start);
extern void wlc_phy_mcs_to_ofdm_powers_nphy(uint8 *power, uint8 rate_ofdm_start,
	uint8 rate_ofdm_end,  uint8 rate_mcs_start);
extern bool wlc_phy_txpwr_srom_read_gphy(phy_info_t *pi);
extern bool wlc_phy_txpwr_srom_read_aphy(phy_info_t *pi);

extern uint16 wlc_lcnphy_tempsense(phy_info_t *pi, bool mode);
extern int16 wlc_lcnphy_tempsense_new(phy_info_t *pi, bool mode);
extern int8 wlc_lcnphy_tempsense_degree(phy_info_t *pi, bool mode);
extern int8 wlc_lcnphy_vbatsense(phy_info_t *pi, bool mode);
extern void wlc_phy_carrier_suppress_lcnphy(phy_info_t *pi);
extern void wlc_lcnphy_crsuprs(phy_info_t *pi, int channel);
extern void wlc_lcnphy_epa_switch(phy_info_t *pi, bool mode);
extern void wlc_2064_vco_cal(phy_info_t *pi);
extern void wlc_phy_noise_cb(phy_info_t *pi, uint8 channel, int8 noise_dbm);
extern void wlc_phy_tempsense_based_minpwr_change(phy_info_t *pi, bool meas_temp);

/* Misc SSLPNPHY funcs */
extern int8 wlc_sslpnphy_noise_avg(phy_info_t *pi);
extern void BCMROMFN(wlc_sslpnphy_rx_gain_override_enable)(phy_info_t *pi, bool enable);
extern void wlc_sslpnphy_detection_disable(phy_info_t *pi, bool mode);
extern bool BCMROMFN(wlc_sslpnphy_rx_iq_est)(phy_info_t *pi, uint16 num_samps,
	uint8 wait_time, sslpnphy_iq_est_t *iq_est);

/* %%%%%% common testing */
#if defined(BCMDBG) || defined(WLTEST)
extern int wlc_phy_test_init(phy_info_t *pi, int channel, bool txpkt);
extern int wlc_phy_test_stop(phy_info_t *pi);
extern void wlc_phy_init_test_lpphy(phy_info_t *pi);
extern void wlc_phy_init_test_sslpnphy(phy_info_t *pi);
extern void wlc_phy_init_test_lcnphy(phy_info_t *pi);

extern void wlc_phy_test_freq_accuracy_prep_abgphy(phy_info_t *pi);
extern void wlc_phy_test_freq_accuracy_run_abgphy(phy_info_t *pi);
extern void wlc_get_11b_txpower(phy_info_t *pi, atten_t *atten);
extern void wlc_phy_set_11b_txpower(phy_info_t *pi, atten_t *atten);

extern int wlc_phy_aphy_long_train(phy_info_t *pi, int channel);
extern int wlc_phy_lpphy_long_train(phy_info_t *pi, int channel);
extern int wlc_phy_sslpnphy_long_train(phy_info_t *pi, int channel);
extern int wlc_phy_lcnphy_long_train(phy_info_t *pi, int channel);
#endif	

#if defined(WLTEST)
extern void wlc_phy_carrier_suppress_lpssnphy(phy_info_t *pi);
extern void wlc_phy_carrier_suppress_sslpnphy(phy_info_t *pi);
#endif

/* %%%%%% ABGPHY functions */
extern void wlc_phy_switch_radio_abgphy(phy_info_t *pi, bool on);
extern void wlc_phy_txpower_get_instant_abgphy(phy_info_t *pi, void *pwr);
extern void wlc_phy_txpower_hw_ctrl_set_abgphy(phy_info_t *pi);
extern void wlc_synth_pu_war(phy_info_t *pi, uint channel);
extern void wlc_phy_ant_rxdiv_set_abgphy(phy_info_t *pi, uint8 val);
extern void BCMATTACHFN(wlc_phy_detach_abgphy)(phy_info_t *pi);
extern uint16 wlc_default_radiopwr_gphy(phy_info_t *pi);


extern void wlc_lcnphy_4313war(phy_info_t *pi);
extern void wlc_set_11a_txpower(phy_info_t *pi, int8 tpi, bool override);
extern void wlc_phy_cal_measurelo_gphy(phy_info_t *pi);
extern void wlc_phy_cal_txpower_stats_clr_gphy(phy_info_t *pi);
extern void wlc_phy_cal_radio2050_nrssioffset_gmode1(phy_info_t *pi);
extern void wlc_phy_cal_radio2050_nrssislope(phy_info_t *pi);
extern int8 wlc_phy_noise_sample_aphy_meas(phy_info_t *pi);
extern int8 wlc_phy_noise_sample_gphy(phy_info_t *pi);
extern int wlc_get_a_band_range(phy_info_t*);
extern int8 wlc_jssi_to_rssi_dbm_abgphy(phy_info_t *pi, int crs_state, int *jssi, int jssi_count);

/* %%%%%% LPCONF function */
#define LPPHY_TBL_ID_PAPD_EPS	0x0
#define LCNPHY_TBL_ID_PAPDCOMPDELTATBL	0x18

#define LPPHY_TX_POWER_TABLE_SIZE	128
#define LPPHY_MAX_TX_POWER_INDEX	(LPPHY_TX_POWER_TABLE_SIZE - 1)
#define LPPHY_TX_PWR_CTRL_OFF	0
#define LPPHY_TX_PWR_CTRL_SW	LPPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK
#define LPPHY_TX_PWR_CTRL_HW \
				(LPPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
				LPPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK)

extern void wlc_phy_table_write_lpphy(phy_info_t *pi, const lpphytbl_info_t *ptbl_info);
extern void wlc_phy_table_read_lpphy(phy_info_t *pi, const lpphytbl_info_t *ptbl_info);
extern bool wlc_phy_tpc_isenabled_lpphy(phy_info_t *pi);
extern uint16 wlc_phy_get_current_tx_pwr_idx_lpphy(phy_info_t *pi);
extern void wlc_phy_tx_pwr_update_npt_lpphy(phy_info_t *pi);
extern int32 wlc_phy_rx_signal_power_lpphy(phy_info_t *pi, int32 gain_index);
extern void wlc_phy_tx_dig_filt_ofdm_setup_lpphy(phy_info_t *pi, bool set_now);
extern void wlc_phy_set_tx_pwr_by_index_lpphy(phy_info_t *pi, int indx);
extern void wlc_phy_aci_init_lpphy(phy_info_t *pi, bool sys);
extern void wlc_phy_get_tx_iqcc_lpphy(phy_info_t *pi, uint16 *a, uint16 *b);

extern void wlc_phy_set_tx_iqcc_lpphy(phy_info_t *pi, uint16 a, uint16 b);
extern void wlc_phy_set_tx_locc_lpphy(phy_info_t *pi, uint16 didq);
extern uint16 wlc_phy_get_tx_locc_lpphy(phy_info_t *pi);
extern void wlc_phy_get_radio_loft_lpphy(phy_info_t *pi, uint8 *ei0, uint8 *eq0,
	uint8 *fi0, uint8 *fq0);
extern void wlc_phy_set_radio_loft_lpphy(phy_info_t *pi, uint8 ei0, uint8 eq0, uint8, uint8);
extern int wlc_phy_tempsense_lpphy(phy_info_t *pi);
extern int wlc_phy_vbatsense_lpphy(phy_info_t *pi);
extern void wlc_phy_rx_gain_temp_adj_lpphy(phy_info_t *pi);
extern void wlc_phy_tx_dig_filt_cck_setup_lpphy(phy_info_t *pi, bool set_now);

extern void wlc_phy_set_tx_locc_ucode_lpphy(phy_info_t *pi, bool iscck, uint16 didq);
extern void wlc_phy_table_lock_lpphy(phy_info_t *pi);
extern void wlc_phy_table_unlock_lpphy(phy_info_t *pi);

extern void wlc_phy_get_tssi_lpphy(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr);

extern void wlc_phy_radio_2062_check_vco_cal(phy_info_t *pi);
extern void wlc_phy_txpower_recalc_target(phy_info_t *pi);
extern uint32 wlc_phy_qdiv_roundup(uint32 dividend, uint32 divisor, uint8 precision);

/* %%%%%% SSLPNCONF function */

/* sslpnphy filter control table for 40MHz */
extern CONST uint32 (fltr_ctrl_tbl_40Mhz)[];

#define SSLPNPHY_TX_POWER_TABLE_SIZE	128
#define SSLPNPHY_MAX_TX_POWER_INDEX	(SSLPNPHY_TX_POWER_TABLE_SIZE - 1)
#define SSLPNPHY_TBL_ID_TXPWRCTL 	0x07
#define SSLPNPHY_TX_PWR_CTRL_OFF	0
#define SSLPNPHY_TX_PWR_CTRL_SW		SSLPNPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK
#define SSLPNPHY_TX_PWR_CTRL_HW         (SSLPNPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
					SSLPNPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK | \
					SSLPNPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)

extern void wlc_sslpnphy_write_table(phy_info_t *pi, const phytbl_info_t *pti);
extern void wlc_sslpnphy_read_table(phy_info_t *pi, phytbl_info_t *pti);
extern void BCMROMFN(wlc_sslpnphy_deaf_mode)(phy_info_t *pi, bool mode);
extern void wlc_sslpnphy_periodic_cal_top(phy_info_t *pi);
extern void wlc_sslpnphy_periodic_cal(phy_info_t *pi);
extern bool wlc_phy_tpc_isenabled_sslpnphy(phy_info_t *pi);
extern void wlc_sslpnphy_tx_pwr_update_npt(phy_info_t *pi);
extern int32 wlc_sslpnphy_tssi2dbm(int32 tssi, int32 a1, int32 b0, int32 b1);
extern void wlc_sslpnphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b);
extern void wlc_sslpnphy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr);
#if defined(WLTEST)
extern int32 wlc_sslpnphy_rx_signal_power(phy_info_t *pi, int32 gain_index);
extern void wlc_sslpnphy_pkteng_stats_get(phy_info_t *pi, wl_pkteng_stats_t *stats);
#endif
extern int wlc_sslpnphy_rssi_compute(phy_info_t *pi, int rssi, d11rxhdr_t *rxh);
extern void wlc_sslpnphy_txpwr_target_adj(phy_info_t *pi, uint8 *tx_pwr_target, uint8 rate);
extern int wlc_sslpnphy_txpwr_idx_get(phy_info_t *pi);
extern void wlc_sslpnphy_iovar_papd_debug(phy_info_t *pi, void *a);
extern void wlc_sslpnphy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val);
extern void wlc_phy_detach_sslpnphy(phy_info_t *pi);
extern void wlc_load_bt_fem_combiner_sslpnphy(phy_info_t *pi, bool force_update);
extern void wlc_phy_watchdog_sslpnphy(phy_info_t *pi);

/* %%%%%% LCNCONF function */
#define LCNPHY_TX_POWER_TABLE_SIZE	128
#define LCNPHY_MAX_TX_POWER_INDEX	(LCNPHY_TX_POWER_TABLE_SIZE - 1)
#define LCNPHY_TBL_ID_TXPWRCTL 	0x07
#define LCNPHY_TX_PWR_CTRL_OFF	0
#define LCNPHY_TX_PWR_CTRL_SW		LCNPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK
#define LCNPHY_TX_PWR_CTRL_HW         (LCNPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
					LCNPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK)

#define LCNPHY_TX_PWR_CTRL_TEMPBASED	0xE001

extern void wlc_lcnphy_write_table(phy_info_t *pi, const phytbl_info_t *pti);
extern void wlc_lcnphy_read_table(phy_info_t *pi, phytbl_info_t *pti);
extern void wlc_lcnphy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b);
extern void wlc_lcnphy_set_tx_locc(phy_info_t *pi, uint16 didq);
extern void wlc_lcnphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b);
extern uint16 wlc_lcnphy_get_tx_locc(phy_info_t *pi);
extern void wlc_lcnphy_get_radio_loft(phy_info_t *pi, uint8 *ei0,
	uint8 *eq0, uint8 *fi0, uint8 *fq0);
extern void wlc_lcnphy_calib_modes(phy_info_t *pi, uint mode);
extern void wlc_lcnphy_deaf_mode(phy_info_t *pi, bool mode);
extern bool wlc_phy_tpc_isenabled_lcnphy(phy_info_t *pi);
extern bool wlc_phy_tpc_iovar_isenabled_lcnphy(phy_info_t *pi);
extern void wlc_lcnphy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr);
extern void wlc_lcnphy_tx_pwr_update_npt(phy_info_t *pi);
extern int32 wlc_lcnphy_tssi2dbm(int32 tssi, int32 a1, int32 b0, int32 b1);
extern void wlc_lcnphy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr);
extern void wlc_sslpnphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b);
extern uint16 wlc_sslpnphy_get_tx_locc(phy_info_t *pi);
extern void BCMROMFN(wlc_sslpnphy_get_radio_loft)(phy_info_t *pi, uint8 *ei0,
	uint8 *eq0, uint8 *fi0, uint8 *fq0);
extern void wlc_sslpnphy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b);
extern void wlc_lcnphy_tx_power_adjustment(wlc_phy_t *ppi);

extern int32 wlc_lcnphy_rx_signal_power(phy_info_t *pi, int32 gain_index);

extern void wlc_lcnphy_noise_measure_stop(phy_info_t *pi);
extern void wlc_lcnphy_noise_measure_start(phy_info_t *pi, bool adj_en);
extern void wlc_lcnphy_noise_measure_resume(phy_info_t *pi);
extern void wlc_lcnphy_noise_measure(phy_info_t *pi);
extern void wlc_lcnphy_noise_measure_disable(phy_info_t *pi, uint32 flag, uint32* p_flag);

extern uint8 wlc_lcnphy_modify_max_txpower(phy_info_t *pi, uint8 maxtxpwr);
extern void wlc_lcnphy_modify_rate_power_offsets(phy_info_t *pi);
extern void wlc_lcnphy_papd_recal(phy_info_t *pi);

/* %%%%%% LCN40CONF function */
#define LCN40PHY_TX_POWER_TABLE_SIZE	128
#define LCN40PHY_MAX_TX_POWER_INDEX	(LCN40PHY_TX_POWER_TABLE_SIZE - 1)
#define LCN40PHY_TBL_ID_TXPWRCTL 	0x07
#define LCN40PHY_TX_PWR_CTRL_OFF	0
#define LCN40PHY_TX_PWR_CTRL_SW		LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK
#define LCN40PHY_TX_PWR_CTRL_HW         (LCN40PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK | \
					LCN40PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK | \
					LCN40PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK)

extern void wlc_lcn40phy_clear_tx_power_offsets(phy_info_t *pi);
extern void wlc_lcn40phy_write_table(phy_info_t *pi, const phytbl_info_t *pti);
extern void wlc_lcn40phy_read_table(phy_info_t *pi, phytbl_info_t *pti);

extern void wlc_lcn40phy_tx_pwr_update_npt(phy_info_t *pi);
extern void wlc_lcn40phy_tssi_ucode_setup(phy_info_t *pi);
extern int wlc_lcn40phy_tssi_cal(phy_info_t *pi);
extern int16 wlc_lcn40phy_tempsense(phy_info_t *pi, bool mode);
extern int8 wlc_lcn40phy_vbatsense(phy_info_t *pi, bool mode);
extern void wlc_lcn40phy_calib_modes(phy_info_t *pi, uint mode);

extern void wlc_lcn40phy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr);
extern void wlc_lcn40phy_tx_pu(phy_info_t *pi, bool bEnable);
extern bool wlc_phy_tpc_iovar_isenabled_lcn40phy(phy_info_t *pi);
extern void wlc_lcn40phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr);
extern int wlc_lcn40phy_idle_tssi_est_iovar(phy_info_t *pi, bool type);
extern uint8 wlc_lcn40phy_get_bbmult_from_index(phy_info_t *pi, int indx);
extern void wlc_phy_init_test_lcn40phy(phy_info_t *pi);
extern void wlc_lcn40phy_deaf_mode(phy_info_t *pi, bool mode);
extern void wlc_lcn40phy_start_tx_tone(phy_info_t *pi, int32 f_kHz, uint16 max_val, bool iqcalmode);
extern void wlc_lcn40phy_stop_tx_tone(phy_info_t *pi);
extern void wlc_lcn40phy_set_tx_tone_and_gain_idx(phy_info_t *pi);
extern void wlc_lcn40phy_crsuprs(phy_info_t *pi, int channel);
extern void wlc_phy_noise_measure_lcn40phy(phy_info_t *pi);
extern void wlc_lcn40phy_noise_measure_start(phy_info_t *pi, bool adj_en);
extern void wlc_lcn40phy_noise_measure_stop(phy_info_t *pi);
extern void wlc_lcn40phy_noise_measure_disable(phy_info_t *pi, uint32 flag, uint32* p_flag);
extern void wlc_lcn40phy_dummytx(wlc_phy_t *ppi, uint16 nframes, uint16 wait_delay);
extern void wlc_lcn40phy_papd_recal(phy_info_t *pi);
extern void wlc_lcn40phy_read_papdepstbl(phy_info_t *pi, struct bcmstrbuf *b);
extern int8 wlc_phy_noise_read_shmem(phy_info_t *pi);
extern void wlc_phy_noise_cb(phy_info_t *pi, uint8 channel, int8 noise_dbm);

/* %%%%%% NCONF function */
#define NPHY_MAX_HPVGA1_INDEX		10
#define NPHY_DEF_HPVGA1_INDEXLIMIT	7

typedef struct _phy_iq_comp {
	int16  a;
	int16  b;
} phy_iq_comp_t;

#define CHANNEL_ISRADAR(channel)  ((((channel) >= 52) && ((channel) <= 64)) || \
				   (((channel) >= 100) && ((channel) <= 140)))

extern void wlc_phy_stay_in_carriersearch_nphy(phy_info_t *pi, bool enable);
extern void wlc_nphy_deaf_mode(phy_info_t *pi, bool mode);

#define wlc_phy_write_table_nphy(pi, pti)	wlc_phy_write_table(pi, pti, NPHY_TableAddress, \
	NPHY_TableDataHi, NPHY_TableDataLo)
#define wlc_phy_read_table_nphy(pi, pti)	wlc_phy_read_table(pi, pti, NPHY_TableAddress, \
	NPHY_TableDataHi, NPHY_TableDataLo)
#define wlc_nphy_table_addr(pi, id, off)	wlc_phy_table_addr((pi), (id), (off), \
	NPHY_TableAddress, NPHY_TableDataHi, NPHY_TableDataLo)
#define wlc_nphy_table_data_write(pi, w, v)	wlc_phy_table_data_write((pi), (w), (v))

extern void wlc_phy_table_read_nphy(phy_info_t *pi, uint32, uint32 l, uint32 o, uint32 w, void *d);
extern void wlc_phy_table_write_nphy(phy_info_t *pi, uint32, uint32, uint32, uint32, const void *);

/* please use this macro extensively for IPA feature to enable compile to optimize codesize */
#if defined(WLPHY_IPA_ONLY) && defined(WLPHY_EPA_ONLY)
#error "invalid configration to enable, IPA only and epa only on the same build"
#endif /* defined(WLPHY_IPA_ONLY) && defined(WLPHY_EPA_ONLY) */

#ifdef WLPHY_IPA_ONLY
#define PHY_IPA(pi)	1
#elif defined(WLPHY_EPA_ONLY)
#define PHY_IPA(pi)	0
#else
#define	PHY_IPA(pi) \
	((pi->ipa2g_on && CHSPEC_IS2G(pi->radio_chanspec)) || \
	 (pi->ipa5g_on && CHSPEC_IS5G(pi->radio_chanspec)))
#endif /* WLPHY_IPA_ONLY */


#define WLC_PHY_WAR_PR51571(pi) \
	if ((BUSTYPE((pi)->sh->bustype) == PCI_BUS) && NREV_LT((pi)->pubpi.phy_rev, 3)) \
		(void)R_REG((pi)->sh->osh, &(pi)->regs->maccontrol)

extern void wlc_phy_cal_perical_nphy_run(phy_info_t *pi, uint8 caltype);
extern void wlc_phy_aci_reset_nphy(phy_info_t *pi);
extern void wlc_phy_pa_override_nphy(phy_info_t *pi, bool en);

extern uint8 wlc_phy_get_chan_freq_range_nphy(phy_info_t *pi, uint chan);
extern void wlc_phy_switch_radio_nphy(phy_info_t *pi, bool on);

extern void wlc_phy_stf_chain_upd_nphy(phy_info_t *pi);

extern void wlc_phy_force_rfseq_nphy(phy_info_t *pi, uint8 cmd);
extern int16 wlc_phy_tempsense_nphy(phy_info_t *pi);

extern uint16 wlc_phy_classifier_nphy(phy_info_t *pi, uint16 mask, uint16 val);

extern void wlc_phy_rx_iq_est_nphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs);

extern void wlc_phy_rx_iq_coeffs_nphy(phy_info_t *pi, uint8 write, nphy_iq_comp_t *comp);
extern void wlc_phy_aci_and_noise_reduction_nphy(phy_info_t *pi);

extern void wlc_phy_rxcore_setstate_nphy(wlc_phy_t *pih, uint8 rxcore_bitmask,
bool enable_phyhangwar);
extern uint8 wlc_phy_rxcore_getstate_nphy(wlc_phy_t *pih);

extern void wlc_phy_txpwrctrl_enable_nphy(phy_info_t *pi, uint8 ctrl_type);
extern void wlc_phy_txpwr_fixpower_nphy(phy_info_t *pi);
extern void wlc_phy_txpwr_apply_nphy(phy_info_t *pi);
extern void wlc_phy_txpwr_papd_cal_nphy(phy_info_t *pi);
extern uint16 wlc_phy_txpwr_idx_get_nphy(phy_info_t *pi);
extern void wlc_phy_store_txindex_nphy(phy_info_t *pi);

extern nphy_txgains_t wlc_phy_get_tx_gain_nphy(phy_info_t *pi);
extern int  wlc_phy_cal_txiqlo_nphy(phy_info_t *pi, nphy_txgains_t target_gain, bool full, bool m);
extern int  wlc_phy_cal_rxiq_nphy(phy_info_t *pi, nphy_txgains_t target_gain, uint8 type, bool d,
                                  uint8 core_mask);
extern void wlc_phy_txpwr_index_nphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex, bool res);
extern void wlc_phy_rssisel_nphy(phy_info_t *pi, uint8 core, uint8 rssi_type);
extern int  wlc_phy_poll_rssi_nphy(phy_info_t *pi, uint8 rssi_type, int32 *rssi_buf, uint8 nsamps);
extern void wlc_phy_rssi_cal_nphy(phy_info_t *pi);
extern int  wlc_phy_aci_scan_nphy(phy_info_t *pi);
extern nphy_txgains_t wlc_phy_cal_txgainctrl_inttssi_nphy(phy_info_t *pi, int8 target_tssi,
                                                          int8 init_gc_idx);
extern void wlc_phy_cal_txgainctrl_nphy(phy_info_t *pi, int32 dBm_targetpower, bool debug);
extern int
wlc_phy_tx_tone_nphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 mode, uint8, bool);
extern void wlc_phy_papd_enable_nphy(phy_info_t *pi, bool papd_state);
extern void wlc_phy_stopplayback_nphy(phy_info_t *pi);
extern void wlc_phy_est_tonepwr_nphy(phy_info_t *pi, int32 *qdBm_pwrbuf, uint8 num_samps);
extern void wlc_phy_radio205x_vcocal_nphy(phy_info_t *pi);
extern void wlc_phy_radio205x_check_vco_cal_nphy(phy_info_t *pi);

extern int wlc_phy_rssi_compute_nphy(phy_info_t *pi, wlc_d11rxhdr_t *wlc_rxh);
extern void wlc_phy_init_hw_antsel(phy_info_t *pi);

#ifdef SAMPLE_COLLECT
extern int wlc_phy_sample_collect_nphy(phy_info_t *pi, wl_samplecollect_args_t *p, uint32 *b);
extern int wlc_phy_sample_data_nphy(phy_info_t *pi, wl_sampledata_t *p, void *b);

extern void wlc_phy_sample_collect_start_nphy(phy_info_t *pi, uint8 coll_us,
	uint16 *crsctl, uint16 *crsctlu, uint16 *crsctll);

extern void wlc_phy_sample_collect_end_nphy(phy_info_t *pi,
	uint16 crsctl, uint16 crsctlu, uint16 crsctll);
#endif

#if defined(BCMDBG) || defined(WLTEST)
extern int wlc_phy_freq_accuracy_nphy(phy_info_t *pi, int channel);
#endif

#define NPHY_TESTPATTERN_BPHY_EVM   0
#define NPHY_TESTPATTERN_BPHY_RFCS  1

#define HTPHY_TESTPATTERN_BPHY_EVM   0
#define HTPHY_TESTPATTERN_BPHY_RFCS  1


#ifdef BCMDBG
extern void wlc_phy_setinitgain_nphy(phy_info_t *pi, uint16 init_gain);
extern void wlc_phy_sethpf1gaintbl_nphy(phy_info_t *pi, int8 maxindex);
extern void wlc_phy_cal_reset_nphy(phy_info_t *pi, uint32 reset_type);
#endif

#if defined(WLTEST)
extern void wlc_phy_bphy_testpattern_nphy(phy_info_t *pi, uint8 testpattern, bool enable, bool);
extern uint32 wlc_phy_cal_sanity_nphy(phy_info_t *pi);
extern void wlc_phy_test_scraminit_nphy(phy_info_t *pi, int8 init);
extern void wlc_phy_gpiosel_nphy(phy_info_t *pi, uint16 sel);
extern int8 wlc_phy_test_tssi_nphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs);
#endif 

#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
extern int wlc_phy_cal_cache_restore(phy_info_t *pi);
extern int wlc_phy_cal_cache_restore_nphy(phy_info_t *pih);
extern int wlc_phy_cal_cache_restore_htphy(phy_info_t *pih);
#endif

#if defined(AP) && defined(RADAR)
extern void wlc_phy_radar_detect_init(phy_info_t *pi, bool on);
extern void wlc_phy_update_radar_detect_param_nphy(phy_info_t *pi);
extern void wlc_phy_update_radar_detect_param_htphy(phy_info_t *pi);
#endif /* defined(AP) && defined(RADAR) */

/* %%%%%% HTCONF function */
extern void wlc_phy_cals_htphy(phy_info_t *pi, uint8 caltype);
extern void wlc_phy_scanroam_cache_cal_htphy(phy_info_t *pi, bool set);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern void wlc_phy_cal_dump_htphy(phy_info_t *pi, struct bcmstrbuf *b);
#endif

extern void wlc_phy_stay_in_carriersearch_htphy(phy_info_t *pi, bool enable);
extern void wlc_phy_deaf_htphy(phy_info_t *pi, bool mode);

#define wlc_phy_write_table_htphy(pi, pti)	wlc_phy_write_table(pi, pti, HTPHY_TableAddress, \
	HTPHY_TableDataHi, HTPHY_TableDataLo)
#define wlc_phy_read_table_htphy(pi, pti)	wlc_phy_read_table(pi, pti, HTPHY_TableAddress, \
	HTPHY_TableDataHi, HTPHY_TableDataLo)
extern void wlc_phy_table_read_htphy(phy_info_t *pi, uint32, uint32 l, uint32 o, uint32 w, void *d);
extern void wlc_phy_table_write_htphy(phy_info_t *pi, uint32, uint32, uint32, uint32, const void *);
extern bool wlc_phy_rfseqtbl_valid_addr_htphy(phy_info_t *pi, uint16 addr);

extern void wlc_phy_pa_override_htphy(phy_info_t *pi, bool en);
extern void wlc_phy_anacore_htphy(phy_info_t *pi,  bool on);
extern void wlc_phy_rxcore_setstate_htphy(wlc_phy_t *pih, uint8 rxcore_bitmask);
extern uint8 wlc_phy_rxcore_getstate_htphy(wlc_phy_t *pih);

extern uint8 wlc_phy_get_chan_freq_range_htphy(phy_info_t *pi, uint chan);
extern void wlc_phy_switch_radio_htphy(phy_info_t *pi, bool on);

extern void wlc_phy_force_rfseq_htphy(phy_info_t *pi, uint8 cmd);
extern int16 wlc_phy_tempsense_htphy(phy_info_t *pi);
extern void wlc_phy_stf_chain_temp_throttle_htphy(phy_info_t *pi);

extern uint16 wlc_phy_classifier_htphy(phy_info_t *pi, uint16 mask, uint16 val);
extern bool wlc_phy_get_rxgainerr_phy(phy_info_t *pi, int16 *gainerr);
extern void wlc_phy_get_SROMnoiselvl_phy(phy_info_t *pi, int8 *noiselvl);
extern void wlc_phy_upd_gain_wrt_temp_phy(phy_info_t *pi, int16 *gain_err_temp_adj);
extern void wlc_phy_rx_iq_est_htphy(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
                                    uint8 wait_time, uint8 wait_for_crs);

extern void wlc_phy_txpower_reg_limit_calc_htphy(phy_info_t *pi, txppr_t *txpwr);
extern void wlc_phy_txpwr_apply_htphy(phy_info_t *pi);
void wlc_phy_txpwr_est_pwr_htphy(phy_info_t *pi, uint8 *Pout, uint8 *Pout_act);
extern uint32 wlc_phy_txpower_est_power_nphy(phy_info_t *pi);

extern uint32 wlc_phy_txpwr_idx_get_htphy(phy_info_t *pi);
extern void wlc_phy_txpwrctrl_enable_htphy(phy_info_t *pi, uint8 ctrl_type);
extern void wlc_phy_txpwr_by_index_htphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex);

extern int
wlc_phy_tx_tone_htphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 mode, uint8, bool);
extern void wlc_phy_stopplayback_htphy(phy_info_t *pi);

extern int
wlc_phy_tx_tone_htphy(phy_info_t *pi, uint32 f_kHz, uint16 max_val, uint8 mode, uint8, bool);
extern void wlc_phy_stopplayback_htphy(phy_info_t *pi);

extern int wlc_phy_rssi_compute_htphy(phy_info_t *pi, wlc_d11rxhdr_t *wlc_rxh);
extern void wlc_phy_lpf_hpc_override_htphy(phy_info_t *pi, bool setup_not_cleanup);
extern void wlc_phy_dig_lpf_override_htphy(phy_info_t *pi, uint8 dig_lpf_ht);
extern void wlc_phy_lpf_hpc_override_nphy(phy_info_t *pi, bool setup_not_cleanup);
extern void wlc_phy_rfctrl_override_rxgain_htphy(phy_info_t *pi, uint8 restore,
                                                 rxgain_t rxgain[], rxgain_ovrd_t rxgain_ovrd[]);
#ifdef SAMPLE_COLLECT
extern int wlc_phy_sample_collect_htphy(phy_info_t *pi, wl_samplecollect_args_t *p, uint32 *b);
extern int wlc_phy_sample_data_htphy(phy_info_t *pi, wl_sampledata_t *p, void *b);

#endif /* SAMPLE_COLLECT */

#if defined(BCMDBG) || defined(WLTEST)
extern int wlc_phy_freq_accuracy_htphy(phy_info_t *pi, int channel);
#endif

#if defined(WLTEST)
extern void wlc_phy_bphy_testpattern_htphy(phy_info_t *pi, uint8 testpattern,
            uint16 rate_reg, bool enable);
#else
#define wlc_phy_bphy_testpattern_htphy(a, b, c, d) do {} while (0)
#endif


#if defined(WLTEST)
extern void wlc_phy_test_scraminit_htphy(phy_info_t *pi, int8 init);
extern int8 wlc_phy_test_tssi_htphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs);
extern void wlc_phy_gpiosel_htphy(phy_info_t *pi, uint16 sel);
extern void wlc_phy_pavars_get_htphy(phy_info_t *pi, uint16 *buf, uint16 band, uint16 core);
extern void wlc_phy_pavars_set_htphy(phy_info_t *pi, uint16 *buf, uint16 band, uint16 core);
extern int wlc_phy_set_po_htphy(phy_info_t *pi, wl_po_t *inpo);
extern int wlc_phy_get_po_htphy(phy_info_t *pi, wl_po_t *outpo);
#endif 

#if defined(AP) && defined(RADAR)
extern void wlc_phy_update_radar_detect_param_htphy(phy_info_t *pi);
#endif /* defined(AP) && defined(RADAR) */

extern void wlc_phy_update_rxldpc_htphy(phy_info_t *pi, bool ldpc);
extern void wlc_phy_nphy_tkip_rifs_war(phy_info_t *pi, uint8 rifs);


extern void wlc_phy_set_filt_war_htphy(phy_info_t *pi, bool war);
extern bool wlc_phy_get_filt_war_htphy(phy_info_t *pi);

/* LPCONF || SSLPNCONF */
extern void wlc_phy_radio_2063_vco_cal(phy_info_t *pi);

/* SSLPNCONF */
extern void wlc_sslpnphy_papd_recal(phy_info_t *pi);
extern void wlc_sslpnphy_tx_pwr_ctrl_init(phy_info_t *pi);

#ifdef PHYMON
extern int wlc_phycal_state_nphy(phy_info_t *pi, void* buff, int len);
#endif /* PHYMON */

void wlc_phy_get_pwrdet_offsets(phy_info_t *pi, int8 *cckoffset, int8 *ofdmoffset);
extern int8 wlc_phy_upd_rssi_offset(phy_info_t *pi, int8 rssi, chanspec_t chanspec);

extern bool wlc_phy_n_txpower_ipa_ison(phy_info_t *pih);
extern bool wlc_phy_txpwr_srom9_read(phy_info_t *pi);
extern void wlc_phy_txpwr_apply(phy_info_t *pi);

extern void wlc_phy_antsel_init_nphy(wlc_phy_t *ppi, bool lut_init);
#if defined(WLMCHAN) || defined(PHYCAL_CACHING)
/* Get the calcache entry given the chanspec */
extern ch_calcache_t *wlc_phy_get_chanctx(phy_info_t *phi, chanspec_t chanspec);
extern void wlc_phydump_cal_cache_nphy(phy_info_t *pih, ch_calcache_t *ctx, struct bcmstrbuf *b);
extern void wlc_phydump_cal_cache_htphy(phy_info_t *pih, ch_calcache_t *ctx, struct bcmstrbuf *b);
extern ch_calcache_t *wlc_phy_get_chanctx_oldest(phy_info_t *phi);
extern int wlc_phy_create_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern int wlc_phy_reinit_chanctx(phy_info_t *pi, ch_calcache_t *ctx, chanspec_t chanspec);
#endif


/*
 * These are utility routines for reading or writing a sequence of registers
 * in a space-efficient manner, driven by a table of addresses.
 * The 'addrvals' parameter is table of uint16s: addr1, val1, addr2, val2, ..., addrN, valN.
 * 'nregs' is the number of pairs, which is twice the number of array uint16 elements.
 *
 * In 'write' operations, the source data is in the array and the dest is in hardware.
 * In read operations, the source is in hardware and the destination is in the array.
 *
 */
extern void wlc_phyregs_bulkread(phy_info_t *pi, uint16 *addrvals, uint32 nregs);
extern void wlc_phyregs_bulkwrite(phy_info_t *pi, const uint16 *addrvals, uint32 nregs);
extern void wlc_radioregs_bulkread(phy_info_t *pi, uint16 *addrvals, uint32 nregs);
extern void wlc_radioregs_bulkwrite(phy_info_t *pi, const uint16 *addrvals, uint32 nregs);
extern void wlc_mod_phyreg_bulk(phy_info_t *pi, uint16 *regs, uint16 *mask, uint16 *val,
	uint32 nregs);

/*
 * This helpful macro calculate the number of registers to access for a particular
 * array of addresses and values. There are twice as many array elements as registers,
 * since each register requires both an address and a value.
 */
#define WLC_BULK_SZ(addrvals) (sizeof(addrvals) / (2 * sizeof(addrvals[0])))

extern void wlc_phy_tx_pwr_limit_check(wlc_phy_t *pih);
#if defined(BCMDBG)
extern void wlc_lcnphy_iovar_cw_tx_pwr_ctrl(phy_info_t *pi, int32 targetpwr, int32 *ret, bool set);
#endif 

#ifdef WLMEDIA_APCS
extern void wlc_sslpnphy_set_skip_papd_recal_flag(phy_info_t *pi);
extern void wlc_sslpnphy_reset_skip_papd_recal_flag(phy_info_t *pi);
extern int wlc_sslpnphy_get_dcs_papd_delay(phy_info_t *pi);
#endif /* WLMEDIA_APCS */

#ifdef ENABLE_FCBS
/* Fast channel/band switch  (FCBS) function prototypes */
extern bool wlc_phy_fcbs_init(phy_info_t *pi, int chanidx);
extern int wlc_phy_fcbs(phy_info_t *pi, int chanidx, bool set);
extern void wlc_phy_fcbs_exit(phy_info_t *pi);
#endif /* ENABLE_FCBS */
#ifdef NOISE_CAL_LCNXNPHY
extern void wlc_phy_noise_trigger_ucode(phy_info_t *pi);
extern void wlc_phy_noise_cal_ocl_enable_disable_nphy(phy_info_t *pi, bool enable);
extern void wlc_phy_noise_cal_measure_nphy(phy_info_t *pi);
extern void wlc_phy_noisepwr_nphy(phy_info_t *pi, uint32 *cmplx_pwr);
extern void wlc_phy_aci_noise_measure_nphy(phy_info_t *pi, bool aciupd);
extern void wlc_phy_noise_cal_init_nphy(phy_info_t *pi);
#endif

extern void wlc_phy_txpwr_papd_cal_nphy_dcs(phy_info_t *pi);
extern bool wlc_phy_txpwr_srom_read_lcnphy(phy_info_t *pi, int bandtype);

#endif	/* _wlc_phy_int_h_ */
