/*
 * wl_ecos.h exported functions and definitions
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wl_ecos.h,v 1.2.14.1 2010-12-21 02:32:15 Exp $
 */

#ifndef _wl_ecos_h_
#define _wl_ecos_h_

#ifdef	BCMDBG
#ifdef WL_ERROR
#undef WL_ERROR
#define	WL_ERROR(args)		do {if (wl_msg_level & WL_ERROR_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_TRACE
#undef WL_TRACE
#define	WL_TRACE(args)		do {if (wl_msg_level & WL_TRACE_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_INFORM
#undef WL_INFORM
#define	WL_INFORM(args)		do {if (wl_msg_level & WL_INFORM_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_TMP
#undef WL_TMP
#define	WL_TMP(args)		do {if (wl_msg_level & WL_TMP_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_OID
#undef WL_OID
#define	WL_OID(args)		do {if (wl_msg_level & WL_OID_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_RATE
#undef WL_RATE
#define	WL_RATE(args)		do {if (wl_msg_level & WL_RATE_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_ASSOC
#undef WL_ASSOC
#define WL_ASSOC(args)		do {if (wl_msg_level & WL_ASSOC_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_PS
#undef WL_PS
#define WL_PS(args)		do {if (wl_msg_level & WL_PS_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_TXPWR
#undef WL_TXPWR
#define WL_TXPWR(args)		do {if (wl_msg_level & WL_TXPWR_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_GMODE
#undef WL_GMODE
#define WL_GMODE(args)		do {if (wl_msg_level & WL_GMODE_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_DUAL
#undef WL_DUAL
#define WL_DUAL(args)		do {if (wl_msg_level & WL_DUAL_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_WSEC
#undef WL_WSEC
#define WL_WSEC(args)		do {if (wl_msg_level & WL_WSEC_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_WSEC_DUMP
#undef WL_WSEC_DUMP
#define WL_WSEC_DUMP(args)	\
	do {if (wl_msg_level & WL_WSEC_DUMP_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_NRSSI
#undef WL_NRSSI
#define WL_NRSSI(args)		do {if (wl_msg_level & WL_NRSSI_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_LOFT
#undef WL_LOFT
#define WL_LOFT(args)		do {if (wl_msg_level & WL_LOFT_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_REGULATORY
#undef WL_REGULATORY
#define WL_REGULATORY(args)	\
	do {if (wl_msg_level & WL_REGULATORY_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_ACI
#undef WL_ACI
#define WL_ACI(args)		do {if (wl_msg_level & WL_ACI_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_RADAR
#undef WL_RADAR
#define WL_RADAR(args)		do {if (wl_msg_level & WL_RADAR_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_MPC
#undef WL_MPC
#define WL_MPC(args)		do {if (wl_msg_level & WL_MPC_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_APSTA
#undef WL_APSTA
#define WL_APSTA(args)		do {if (wl_msg_level & WL_APSTA_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_DFS
#undef WL_DFS
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_BA
#undef WL_BA
#define WL_BA(args)		do {if (wl_msg_level & WL_BA_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_NITRO
#undef WL_NITRO
#define WL_NITRO(args)		do {if (wl_msg_level & WL_NITRO_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_PHYDBG
#undef WL_PHYDBG
#define	WL_PHYDBG(args)		do {if (wl_msg_level & 0) diag_printf args;} while (0)
#endif

#ifdef WL_PHYCAL
#undef WL_PHYCAL
#define	WL_PHYCAL(args)		do {if (wl_msg_level & 0) diag_printf args;} while (0)
#endif

#ifdef WL_CAC
#undef WL_CAC
#define WL_CAC(args)		do {if (wl_msg_level & WL_CAC_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_AMSDU
#undef WL_AMSDU
#define WL_AMSDU(args)		do {if (wl_msg_level & WL_AMSDU_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_AMPDU
#undef WL_AMPDU
#define WL_AMPDU(args)		do {if (wl_msg_level & WL_AMPDU_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_FFPLD
#undef WL_FFPLD
#define WL_FFPLD(args)		do {if (wl_msg_level & WL_FFPLD_VAL) diag_printf args;} while (0)
#endif

#ifdef WL_NONE
#undef WL_NONE
#define	WL_NONE(args)		do {if (wl_msg_level & 0) diag_printf args;} while (0)
#endif

#else	/* BCMDBG */

#ifdef BCMDBG_ERR

#ifdef WL_ERROR
#undef WL_ERROR
#define	WL_ERROR(args)		diag_printf args
#endif /* WL_ERROR */

#endif /* BCMDBG_ERR */

#ifdef WLTEST

#ifdef WL_DFS
#undef WL_DFS
#define WL_DFS(args)		do {if (wl_msg_level & WL_DFS_VAL) diag_printf args;} while (0)
#endif /* WL_DFS */

#endif /* WLTEST */

#endif /* BCMDBG */

#endif	/* _wl_ecos_h_ */
