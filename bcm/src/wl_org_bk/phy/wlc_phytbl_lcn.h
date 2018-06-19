/*
 * Declarations for Broadcom PHY core tables,
 * Networking Adapter Device Driver.
 *
 * THIS IS A GENERATED FILE - DO NOT EDIT
 * Generated on Sun Aug 28 21:01:36 PDT 2011
 *
 * Copyright(c) 2007 Broadcom Corp.
 * All Rights Reserved.
 *
 * $Id: wlc_phytbl_lcn.h 280316 2011-08-29 05:57:33Z luisgm $
 */
/* FILE-CSTYLED */

typedef phytbl_info_t dot11lcnphytbl_info_t;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_rx_gain_info_rev0[];
extern CONST uint32 dot11lcnphytbl_rx_gain_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_2G_rx_gain_info_rev2[];
extern CONST uint32 dot11lcnphytbl_2G_rx_gain_info_sz_rev2;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_2G_aci_rx_gain_info_rev2[];
extern CONST uint32 dot11lcnphytbl_2G_aci_rx_gain_info_sz_rev2;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_2G_ext_lna_rx_gain_info_rev2[];
extern CONST uint32 dot11lcnphytbl_2G_ext_lna_rx_gain_info_sz_rev2;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_2G_ext_lna_aci_rx_gain_info_rev2[];
extern CONST uint32 dot11lcnphytbl_2G_ext_lna_aci_rx_gain_info_sz_rev2;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_5G_rx_gain_info_rev2[];
extern CONST uint32 dot11lcnphytbl_5G_rx_gain_info_sz_rev2;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_5G_ext_lna_rx_gain_info_rev2[];
extern CONST uint32 dot11lcnphytbl_5G_ext_lna_rx_gain_info_sz_rev2;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_4336wlbga_rx_gain_info_rev0[];
extern CONST uint32 dot11lcnphytbl_4336wlbga_rx_gain_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_info_rev0[];
extern CONST uint32 dot11lcnphytbl_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_gain_tbl_info_rev0[];
extern CONST uint32 dot11lcnphytbl_gain_tbl_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_gain_tbl_info_rev1[];
extern CONST uint32 dot11lcnphytbl_gain_tbl_info_sz_rev1;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_gain_idx_tbl_info_rev0[];
extern CONST uint32 dot11lcnphytbl_gain_idx_tbl_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_gain_idx_bt_tbl_info_rev0[];
extern CONST uint32 dot11lcnphytbl_gain_idx_bt_tbl_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_gain_bt_tbl_info_rev0[];
extern CONST uint32 dot11lcnphytbl_gain_bt_tbl_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_sw_ctrl_tbl_info_rev0[];
extern CONST uint32 dot11lcnphytbl_sw_ctrl_tbl_info_sz_rev0;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_nf_table_info_rev2[];
extern CONST uint32 dot11lcnphytbl_nf_table_info_sz_rev2;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_radio2064_rx_gain_info_rev3[];
extern CONST uint32 dot11lcnphytbl_radio2064_rx_gain_info_sz_rev3;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_radio2064_ext_lna_rx_gain_info_rev3[];
extern CONST uint32 dot11lcnphytbl_radio2064_ext_lna_rx_gain_info_sz_rev3;


extern CONST dot11lcnphytbl_info_t dot11lcnphytbl_2G_ext_lna_rx_gain_info_rev1[];
extern CONST uint32 dot11lcnphytbl_2G_ext_lna_rx_gain_info_sz_rev1;


typedef struct {
	uchar gm;
	uchar pga;
	uchar pad;
	uchar dac;
	uchar bb_mult;
} lcnphy_tx_gain_tbl_entry;

extern CONST lcnphy_tx_gain_tbl_entry dot11lcnphy_2GHz_gaintable_rev0[];

extern CONST lcnphy_tx_gain_tbl_entry dot11lcnphy_5GHz_gaintable_rev0[];


extern CONST lcnphy_tx_gain_tbl_entry dot11lcnphy_2GHz_extPA_gaintable_rev0[];

extern CONST lcnphy_tx_gain_tbl_entry dot11lcnphy_2GHz_gaintable_1_rev0[];

extern CONST lcnphy_tx_gain_tbl_entry dot11lcnphy_5GHz_extPA_gaintable_rev0[];

extern CONST lcnphy_tx_gain_tbl_entry dot11lcnphy_5GHz_extPA_gaintable_1_rev0[];
