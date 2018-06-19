/*
 * LCNPHY module header file
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlc_phy_lcn40.h 277184 2011-08-12 22:08:44Z yesun $*
 */

#ifndef _wlc_phy_lcn40_h_
#define _wlc_phy_lcn40_h_

#include <typedefs.h>
#include <wlc_phy_int.h>
#include <wlc_phy_lcn.h>
#define LCN40PHY_SWCTRL_NVRAM_PARAMS 5
#define LCN40PHY_RXIQCOMP_PARAMS 2

struct phy_info_lcn40phy {
	phy_info_lcnphy_t lcnphycommon;
	uint16 rx_iq_comp_5g[LCN40PHY_RXIQCOMP_PARAMS];
	uint8 trGain;
	int16 tx_iir_filter_type_cck;
	int16 tx_iir_filter_type_ofdm;
	int16 tx_iir_filter_type_ofdm40;
	bool phycrs_war_en;
	uint16 saved_user_qdbm;
};
extern void wlc_lcn40phy_set_bbmult(phy_info_t *pi, uint8 m0);
#endif /* _wlc_phy_lcn40_h_ */
