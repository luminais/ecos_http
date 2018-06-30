/*
 *      Header file of AP mib
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: apmib.h,v 1.55 2009/10/06 05:49:10 bradhuang Exp $
 *
 */
#ifdef MIB_HW_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	boardVer,	,	BOARD_VER,	BYTE_T, HW_SETTING_T, 0, 0)
MIBDEF(unsigned char,	nic0Addr,	[6],	NIC0_ADDR,	BYTE6_T, HW_SETTING_T, 0, 0)
MIBDEF(unsigned char,	nic1Addr,	[6],	NIC1_ADDR,	BYTE6_T, HW_SETTING_T, 0, 0)
 #ifdef CONFIG_CUTE_MAHJONG_SELECTABLE
MIBDEF(HW_WLAN_SETTING_T,	 wlan, [1], WLAN_ROOT,  TABLE_LIST_T, HW_SETTING_T, 0, hwmib_wlan_table)
 #else
MIBDEF(HW_WLAN_SETTING_T,	wlan, [NUM_WLAN_INTERFACE],	WLAN_ROOT,	TABLE_LIST_T, HW_SETTING_T, 0, hwmib_wlan_table)
 #endif
#endif // #ifdef MIB_HW_IMPORT

#ifdef MIB_HW_WLAN_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char, macAddr, [6],	WLAN_ADDR,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr1, [6],	WLAN_ADDR1,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr2, [6],	WLAN_ADDR2,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr3, [6],	WLAN_ADDR3,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr4, [6],	WLAN_ADDR4,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr5, [6],	WLAN_ADDR5,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr6,[6],	    WLAN_ADDR6,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, macAddr7, [6],	WLAN_ADDR7,	BYTE6_T, HW_WLAN_SETTING_T, 0, 0)
//#if defined(CONFIG_RTL_8196C)
MIBDEF(unsigned char, pwrlevelCCK_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_CCK_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelCCK_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_CCK_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelHT40_1S_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_HT40_1S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelHT40_1S_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_HT40_1S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiffHT40_2S, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_HT40_2S,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiffHT20, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_HT20,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiffOFDM, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, regDomain, ,	REG_DOMAIN,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, rfType, ,	RF_TYPE,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, ledType, ,	LED_TYPE,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, xCap, ,	11N_XCAP,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, TSSI1, ,	11N_TSSI1,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, TSSI2, ,	11N_TSSI2,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Ther, ,	11N_THER,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, trswitch, ,      11N_TRSWITCH,   BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, trswpape_C9, ,	11N_TRSWPAPE_C9, BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, trswpape_CC, ,	11N_TRSWPAPE_CC, BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, target_pwr, ,	11N_TARGET_PWR,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pa_type, ,	11N_PA_TYPE,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
#ifdef CONFIG_RTL_8881A_SELECTIVE
MIBDEF(unsigned char, Ther2, ,	11N_THER_2,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, xCap2, ,	11N_XCAP_2,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
#else
MIBDEF(unsigned char, Ther2, ,	11N_THER2,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved7, ,	11N_RESERVED7,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
#endif
MIBDEF(unsigned char, Reserved8, ,	11N_RESERVED8,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved9, ,	11N_RESERVED9,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, Reserved10, ,	11N_RESERVED10,	BYTE_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevel5GHT40_1S_A, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_5G_HT40_1S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevel5GHT40_1S_B, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_5G_HT40_1S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff5GHT40_2S, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_5G_HT40_2S,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff5GHT20, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_5G_HT20,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff5GOFDM, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_5G_OFDM,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
//#endif
//#ifdef WIFI_SIMPLE_CONFIG
MIBDEF(unsigned char, wscPin, [PIN_LEN+1],	WSC_PIN,	STRING_T, HW_WLAN_SETTING_T, 0, 0)
//#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE)

MIBDEF(unsigned char, pwrdiff_20BW1S_OFDM1T_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_20BW1S_OFDM1T_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW2S_20BW2S_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW2S_20BW2S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM2T_CCK2T_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM2T_CCK2T_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW3S_20BW3S_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW3S_20BW3S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_4OFDM3T_CCK3T_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM3T_CCK3T_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW4S_20BW4S_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW4S_20BW4S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM4T_CCK4T_A, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM4T_CCK4T_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char, pwrdiff_5G_20BW1S_OFDM1T_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_20BW1S_OFDM1T_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW2S_20BW2S_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW2S_20BW2S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW3S_20BW3S_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW3S_20BW3S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW4S_20BW4S_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW4S_20BW4S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_RSVD_OFDM4T_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_RSVD_OFDM4T_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW1S_160BW1S_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW1S_160BW1S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW2S_160BW2S_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW2S_160BW2S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW3S_160BW3S_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW3S_160BW3S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW4S_160BW4S_A, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW4S_160BW4S_A,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)


MIBDEF(unsigned char, pwrdiff_20BW1S_OFDM1T_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_20BW1S_OFDM1T_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW2S_20BW2S_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW2S_20BW2S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM2T_CCK2T_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM2T_CCK2T_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW3S_20BW3S_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW3S_20BW3S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM3T_CCK3T_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM3T_CCK3T_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW4S_20BW4S_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW4S_20BW4S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM4T_CCK4T_B, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM4T_CCK4T_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char, pwrdiff_5G_20BW1S_OFDM1T_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_20BW1S_OFDM1T_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW2S_20BW2S_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW2S_20BW2S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW3S_20BW3S_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW3S_20BW3S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW4S_20BW4S_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW4S_20BW4S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_RSVD_OFDM4T_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_RSVD_OFDM4T_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW1S_160BW1S_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW1S_160BW1S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW2S_160BW2S_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW2S_160BW2S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW3S_160BW3S_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW3S_160BW3S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW4S_160BW4S_B, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW4S_160BW4S_B,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)


#endif

#if defined(CONFIG_WLAN_HAL_8814AE)
MIBDEF(unsigned char, pwrdiff_20BW1S_OFDM1T_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_20BW1S_OFDM1T_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW2S_20BW2S_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW2S_20BW2S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM2T_CCK2T_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM2T_CCK2T_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW3S_20BW3S_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW3S_20BW3S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_4OFDM3T_CCK3T_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM3T_CCK3T_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW4S_20BW4S_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW4S_20BW4S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM4T_CCK4T_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM4T_CCK4T_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char, pwrdiff_5G_20BW1S_OFDM1T_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_20BW1S_OFDM1T_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW2S_20BW2S_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW2S_20BW2S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW3S_20BW3S_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW3S_20BW3S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW4S_20BW4S_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW4S_20BW4S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_RSVD_OFDM4T_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_RSVD_OFDM4T_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW1S_160BW1S_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW1S_160BW1S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW2S_160BW2S_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW2S_160BW2S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW3S_160BW3S_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW3S_160BW3S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW4S_160BW4S_C, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW4S_160BW4S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)


MIBDEF(unsigned char, pwrdiff_20BW1S_OFDM1T_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_20BW1S_OFDM1T_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW2S_20BW2S_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW2S_20BW2S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM2T_CCK2T_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM2T_CCK2T_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW3S_20BW3S_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW3S_20BW3S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM3T_CCK3T_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM3T_CCK3T_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_40BW4S_20BW4S_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_40BW4S_20BW4S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_OFDM4T_CCK4T_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_DIFF_OFDM4T_CCK4T_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char, pwrdiff_5G_20BW1S_OFDM1T_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_20BW1S_OFDM1T_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW2S_20BW2S_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW2S_20BW2S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW3S_20BW3S_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW3S_20BW3S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_40BW4S_20BW4S_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_40BW4S_20BW4S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_RSVD_OFDM4T_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_RSVD_OFDM4T_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW1S_160BW1S_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW1S_160BW1S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW2S_160BW2S_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW2S_160BW2S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW3S_160BW3S_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW3S_160BW3S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrdiff_5G_80BW4S_160BW4S_D, [MAX_5G_DIFF_NUM],	TX_POWER_DIFF_5G_80BW4S_160BW4S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)


MIBDEF(unsigned char, pwrlevelCCK_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_CCK_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelCCK_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_CCK_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelHT40_1S_C, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_HT40_1S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevelHT40_1S_D, [MAX_2G_CHANNEL_NUM_MIB],	TX_POWER_HT40_1S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevel5GHT40_1S_C, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_5G_HT40_1S_C,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char, pwrlevel5GHT40_1S_D, [MAX_5G_CHANNEL_NUM_MIB],	TX_POWER_5G_HT40_1S_D,	BYTE_ARRAY_T, HW_WLAN_SETTING_T, 0, 0)

#endif
#endif // #ifdef MIB_HW_WLAN_IMPORT

#ifdef MIB_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
// TCP/IP stuffs
MIBDEF(unsigned char,	ipAddr, [4],	IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	subnetMask, [4],	SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	defaultGateway, [4],	DEFAULT_GATEWAY,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcp, ,	DHCP,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcpClientStart, [4],	DHCP_CLIENT_START,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcpClientEnd, [4],	DHCP_CLIENT_END,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,   dhcpLeaseTime, ,    DHCP_LEASE_TIME, DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	elanMacAddr, [6],	ELAN_MAC_ADDR,	BYTE6_T, APMIB_T, 0, 0)
#if defined(HAVE_TR069)
//by cairui
MIBDEF(unsigned char,   LanDhcpConfigurable,    ,       LAN_DHCP_CONFIGURABLE,  BYTE_T, APMIB_T, 0, 0)
#endif


//Brad add for static dhcp
MIBDEF(unsigned char,	dns1,	[4],	DNS1,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dns2,	[4],	DNS2,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dns3,	[4],	DNS3,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	stpEnabled,	,	STP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	deviceName,	[MAX_NAME_LEN],	DEVICE_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	scrlogEnabled,	,	SCRLOG_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	autoDiscoveryEnabled,	,	AUTO_DISCOVERY_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	domainName,	[MAX_NAME_LEN],	DOMAIN_NAME,	STRING_T, APMIB_T, 0, 0)


// Supervisor of web server account
MIBDEF(unsigned char,	superName,	[MAX_NAME_LEN],	SUPER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	superPassword, [MAX_NAME_LEN],	SUPER_PASSWORD,	STRING_T, APMIB_T, 0, 0)

// web server account
MIBDEF(unsigned char,	userName, [MAX_NAME_LEN],	USER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	userPassword, [MAX_NAME_LEN],	USER_PASSWORD,	STRING_T, APMIB_T, 0, 0)

#ifdef HAVE_NBSERVER
MIBDEF(unsigned char,	netbiosName, [MAX_NAME_LEN], NETBIOS_NAME,	STRING_T, APMIB_T, 0, 0)	
#endif

#ifdef KLD_ENABLED
MIBDEF(unsigned char,   webLoginTimeout, ,    WEB_LOGIN_TIMEOUT, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,	remoteAccessPort, ,  REMOTE_ACCESS_PORT, DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanIPMode, ,	  WAN_IP_DYNAMIC, BYTE_T, APMIB_T, 0, 0)
//MIBDEF(unsigned char,	RemoteAccessEnabled, ,	REMOTE_ACCESS_ENABLED, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	RemoteAccessStartIPaddr, [4], REMOTE_ACCESS_IP_START,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	userIpaddr, [4],	USER_LAST_IPADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,	userLastTime, , USER_LAST_TIME, DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	userAuthFlag, ,	USER_AUTH_FLAG,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	PppoewanIPMode, ,	  PPPOE_WAN_IP_DYNAMIC, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	PptpwanIPMode, ,	  PPTP_WAN_IP_DYNAMIC, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppoeipAddr, ,	  PPPOE_IP, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpGateway, ,	  PPTP_GATEWAY, IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	dlSaveOffset, ,	DL_SAVE_OFFSET, WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlStartHour, ,	DL_START_HOUR, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlEndHour, ,	DL_END_HOUR, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlSaving, ,	DAYLIGHT_SAVING, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlStartMonth, , DL_START_MONTH, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlStartDay, ,	DL_START_DAY, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlEndMonth, ,	DL_END_MONTH, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlEndDay, ,	DL_END_DAY, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlStartWeek, ,	DL_START_WEEK, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dlEndWeek, ,	DL_END_WEEK, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanTrafficShaping, ,WAN_TRAFFIC_SHAPING, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	igmpProxyDisabled, ,IGMPPROXY_DISABLED, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppoeDnsMode, ,	PPPOE_DNS_MODE, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanDnsMode, , WAN_DNS_MODE, BYTE_T, APMIB_T, 0, 0)

//Time.asp
MIBDEF(unsigned short,	timeYear, ,TIME_YEAR,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	timeMonth, ,TIME_MONTH,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	timeDay, ,	TIME_DAY,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	timeHour, ,TIME_HOUR,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	timeMin, ,	TIME_MIN,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	timeSec, ,	TIME_SEC,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpTimezoneIdx, ,	NTP_TIMEZONE_IDX,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	dlStartDate, ,DL_START_DATE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	dlEndDate, ,DL_END_DATE,	WORD_T, APMIB_T, 0, 0)

//DDNS
MIBDEF(unsigned short,	ddnsTimeout, ,DDNS_TIMEOUT,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	modelNum,	[MAX_MODEL_NAME_LEN], MODEL_NUM, STRING_T, APMIB_T, 0, 0)

//Email.asp
MIBDEF(unsigned char,	smtpUserName,			[MAX_SMTP_NAME_LEN],		SMTP_USERNAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	smtpPassword,			[MAX_SMTP_PASSWORD_LEN],	SMTP_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	smtpServer,				[MAX_SMTP_SERVER_LEN],		SMTP_SERVER	,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	logSendFrom, 			[MAX_LOG_SEND_FROM_LEN],	LOG_SEND_FROM ,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	logSendTo, 				[MAX_LOG_SEND_TO_LEN],		LOG_SEND_TO ,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	emailSubject, 			[MAX_EMAIL_SUBJECT_LEN],	EMAIL_SUBJECT ,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	emailLogScheduleName,	[MAX_EMAIL_LOG_SCHEDULE_NAME_LEN],	EMAIL_LOG_SCHEDULE_NAME , STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	logEnabled, 	,	LOG_ENABLED	, 		BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	smtpAuthEnable, ,	SMTP_AUTH_ENABLED, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	emailNotification, ,EMAIL_NOTIFICATION, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	emailLogSchedule, ,EMAIL_LOG_SCHEDULE, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	emailLogFull, ,EMAIL_LOG_FULL,	BYTE_T, APMIB_T, 0, 0)
//Schedule.asp
MIBDEF(unsigned char,	wlanScheSelectInx, ,WLAN_SCHEDULE_SELECT_IDX, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestScheId, ,	WLAN_GUEST_SCHEDULE_ID,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestScheId2, ,	WLAN_GUEST_SCHEDULE_ID2,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppoeDialSche,[MAX_PPPOEDIAL_SCHE_LEN] ,PPPOE_DIAL_SCUEDULE	, 	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpDialSche,[MAX_PPTPDIAL_SCHE_LEN] ,	PPTP_DIAL_SCUEDULE ,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpDialSche,[MAX_L2TPDIAL_SCHE_LEN] ,	L2TP_DIAL_SCUEDULE , 	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	domainFilterNum, ,	DOMAINFILTER_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	scheduleNum, ,		SCHEDULE_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	firewallRuleNum, ,	FIREWALLRULE_NUM, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   firewallRuleEnable, ,  FIREWALLRULE_ENABLED, BYTE_T, APMIB_T, 0, 0)
MIBDEF(DOMAINFILTER_T,	domainFilterArray, 	[MAX_DOMAINFILTER_NUM], 	DOMAINFILTER,	DOMAINFILTER_ARRAY_T, 	APMIB_T, 0, mib_domainFilter_tbl)
MIBDEF(SCHEDULE_T,		scheduleArray, 		[MAX_SCHEDULE_NUM],		SCHEDULE,		SCHEDULE_ARRAY_T, 		APMIB_T, 0, mib_schedule_tbl)
MIBDEF(FIREWALLRULE_T,	firewallRuleArray, 	[MAX_FIREWALL_RULE_NUM],	FIREWALLRULE,	VIRTUALSERV_ARRAY_T, 	APMIB_T, 0, mib_firewallRule_tbl)
//GUEST_ZONE.asp
MIBDEF(unsigned char,	lockClientNum, ,	LOCK_CLIENT_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(DOMAINFILTER_T,	lockClientArray,	[MAX_LOCKCLIENT_NUM], 	LOCK_CLIENT	,	LOCKCLIENT_ARRAY_T,	APMIB_T, 0, mib_lockClient_tbl)

MIBDEF(unsigned char,	wlanGuestEncrypt, ,		WLAN_GUEST_ENCRYPT,			BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestEncrypt2, ,		WLAN_GUEST_ENCRYPT2,			BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWepKeyType, ,	WLAN_GUEST_WEP_KEY_TYPE, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWepKeyType2, ,	WLAN_GUEST_WEP_KEY_TYPE2, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestDefaultKey, ,	WLAN_GUEST_DEFAULT_KEY,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestDefaultKey2, ,	WLAN_GUEST_DEFAULT_KEY2,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep, ,			WLAN_GUEST_WEP,				BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep2, ,			WLAN_GUEST_WEP2,				BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZoneEnabled, , 	WLAN_GUEST_ZONE_ENABLED, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZoneEnabled2, , 	WLAN_GUEST_ZONE_ENABLED2, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZonePortlist, , 	WLAN_GUEST_ZONE_PORTLIST, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZonePortlist2, , 	WLAN_GUEST_ZONE_PORTLIST2, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZoneWlanEnable, , 	WLAN_GUEST_ZONE_WLANENABLE, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZoneWlanEnable2, , WLAN_GUEST_ZONE_WLANENABLE2, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpaAuth, ,		WLAN_GUEST_WPA_AUTH, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpaAuth2, ,		WLAN_GUEST_WPA_AUTH2, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestAuthType, ,	WLAN_GUEST_AUTH_TYPE, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestAuthType2, ,	WLAN_GUEST_AUTH_TYPE2, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpaCipher, ,	WLAN_GUEST_WPA_CIPHER, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpaCipher2, ,	WLAN_GUEST_WPA_CIPHER2, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpa2Cipher, ,	WLAN_GUEST_WPA2_CIPHER, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpa2Cipher2, ,	WLAN_GUEST_WPA2_CIPHER2, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZoneLockClientlist, ,	WLAN_GUEST_ZONE_LOCK_CLIENTLIST, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestRouteZone, ,			WLAN_GUEST_ROUTE_ZONE, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestZoneIsolation, ,			WLAN_GUEST_ZONE_ISOLATION, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestEnable1x, ,			WLAN_GUEST_ENABLE_1X, BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	wlanGuestWep64Key1, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY1,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep64Key2, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY2,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep64Key3, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY3,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep64Key4, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY4,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key1, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY1,	BYTE13_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key2, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY2,	BYTE13_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key3, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY3,	BYTE13_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key4, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY4,	BYTE13_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	wlanGuestWep64Key12, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY12,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep64Key22, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY22,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep64Key32, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY32,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep64Key42, [WEP64_KEY_LEN],	WLAN_GUEST_WEP64_KEY42,	BYTE5_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key12, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY12,	BYTE13_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key22, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY22,	BYTE13_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key32, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY32,	BYTE13_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWep128Key42, [WEP128_KEY_LEN],	WLAN_GUEST_WEP128_KEY42,	BYTE13_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	wlanGuestSsid, 	[MAX_SSID_LEN],	WLAN_GUEST_SSID,	STRING_T,	APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestSsid2, 	[MAX_SSID_LEN],	WLAN_GUEST_SSID2,	STRING_T,	APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpaPsk, [MAX_PSK_LEN+1],	WLAN_GUEST_WPA_PSK,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestWpaPsk2, [MAX_PSK_LEN+1],	WLAN_GUEST_WPA_PSK2,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestRsIpAddr1, [4], WLAN_GUEST_RS_IP_ADDR1,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestRsIpAddr12, [4], WLAN_GUEST_RS_IP_ADDR12,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	wlanGuestRsPort1, ,WLAN_GUEST_RS_PORT1,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	wlanGuestRsPort12, ,WLAN_GUEST_RS_PORT12,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestRsPwd1, [MAX_WLAN_GUEST_RS_PWD1_LEN],	WLAN_GUEST_RS_PWD1, STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanGuestRsPwd12, [MAX_WLAN_GUEST_RS_PWD1_LEN],	WLAN_GUEST_RS_PWD12, STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	lanNetBiosName, [MAX_NAME_LEN],	LAN_NETBIOS_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcpdBroadcast, , DHCPD_BROADCAST,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dnsRelayEnabled, ,	DNSRELAY_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	netBiosAnnounce, ,	NETBIOS_ANNOUNCE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	netBiosSource, ,	NETBIOS_SOURCE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	netBiosScope, ,	NETBOIS_SCOPE, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	netBiosNodeType, , NETBIOS_NODE_TYPE, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	priWinsIp, [4],	PRI_WINS_IP,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ipMacBinding, , IP_MAC_BINDING,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	secWinsIp, [4], SEC_WINS_IP,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanForceSpeed, , WAN_FORCE_SPEED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pingWanInBound, [MAX_NAME_LEN], PING_WAN_INBOUND, STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlanEnhanceMode, , WLAN_ENHANCE_MODE,	BYTE_T, APMIB_T, 0, 0)

//firewall.asp
MIBDEF(unsigned char,	antiSpoofing,	,	ANTI_SPOOFING,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   spiFirewall,   ,   SPIFIREWALL_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   rtspAlgEnable,   ,   RTSP_ALG_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   sipAlgEnable,   ,   SIP_ALG_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   ftpAlgEnable,   ,   FTP_ALG_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   tftpAlgEnable,   ,   TFTP_ALG_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   pptpAlgEnable,   ,   PPTP_ALG_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   l2tpAlgEnable,   ,   L2TP_ALG_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   h323AlgEnable,   ,   H323_ALG_ENABLED,  BYTE_T, APMIB_T, 0, 0)
#endif
#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	wanMacAddr, [6],	WAN_MAC_ADDR,	BYTE6_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanDhcp,	,	WAN_DHCP,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanOldDhcp,	,	WAN_OLD_DHCP,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanIpAddr, [4],	WAN_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanSubnetMask, [4],	WAN_SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wanDefaultGateway, [4],	WAN_DEFAULT_GATEWAY,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppUserName, [MAX_NAME_LEN_LONG],	PPP_USER_NAME,	STRING_T, APMIB_T, 0, 0)
#ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
MIBDEF(unsigned char,	pppEncryptUserName, [MAX_NAME_LEN_LONG],	PPP_ENCRYPT_USER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   dhcpPlusUserName, [32 + 1],    DHCP_PLUS_USER_NAME,  STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   dhcpPlusPassword, [32 + 1],   DHCP_PLUS_PASS_WORD,   STRING_T, APMIB_T, 0, 0)
#endif
MIBDEF(unsigned char,	pppPassword, [MAX_NAME_LEN_LONG],	PPP_PASSWORD,	STRING_T, APMIB_T, 0, 0)

MIBDEF(DNS_TYPE_T,	dnsMode,	,	DNS_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pppIdleTime,	,	PPP_IDLE_TIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppConnectType,	,	PPP_CONNECT_TYPE,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	dmzEnabled,	,	DMZ_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dmzHost, [4],	DMZ_HOST,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	upnpEnabled, ,	UPNP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pppMtuSize, ,	PPP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpIpAddr, [4],	PPTP_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpSubnetMask, [4],	PPTP_SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpServerIpAddr, [4],	PPTP_SERVER_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpUserName, [MAX_NAME_LEN_LONG],	PPTP_USER_NAME,	STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pptpPassword, [MAX_NAME_LEN_LONG],	PPTP_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pptpMtuSize, ,	PPTP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	pppoeWithDhcpEnabled, ,	PPPOE_DHCP_ENABLED,	BYTE_T, APMIB_T, 0, 0)

/* # keith: add l2tp support. 20080515 */
MIBDEF(unsigned char,	l2tpIpAddr, [4],	L2TP_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpSubnetMask, [4],	L2TP_SUBNET_MASK,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpServerIpAddr, [MAX_PPTP_HOST_NAME_LEN],	L2TP_SERVER_IP_ADDR,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpGateway, [4],	L2TP_GATEWAY,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpUserName, [MAX_NAME_LEN_LONG],	L2TP_USER_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpPassword, [MAX_NAME_LEN_LONG],	L2TP_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	l2tpMtuSize, ,	L2TP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	l2tpIdleTime, ,	L2TP_IDLE_TIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpConnectType, ,	L2TP_CONNECTION_TYPE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	L2tpwanIPMode, ,	L2TP_WAN_IP_DYNAMIC,	BYTE_T, APMIB_T, 0, 0)

#if defined(CONFIG_DYNAMIC_WAN_IP)
MIBDEF(unsigned char,	l2tpDefGw, [4], L2TP_DEFAULT_GW,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpDefGw, [4], PPTP_DEFAULT_GW,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpWanIPMode, ,	PPTP_WAN_IP_DYNAMIC,	BYTE_T, APMIB_T, 0, 0)
#ifdef CONFIG_GET_SERVER_IP_BY_DOMAIN
MIBDEF(unsigned char,	pptpGetServByDomain, ,	PPTP_GET_SERV_BY_DOMAIN,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpServerDomain, [MAX_SERVER_DOMAIN_LEN],	PPTP_SERVER_DOMAIN, STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpGetServByDomain, ,	L2TP_GET_SERV_BY_DOMAIN,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	l2tpServerDomain, [MAX_SERVER_DOMAIN_LEN],	L2TP_SERVER_DOMAIN, STRING_T, APMIB_T, 0, 0)
#endif
#endif

/* USB3G */
MIBDEF(unsigned char,   usb3g_user,     [32],    USB3G_USER,        STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_pass,     [32],    USB3G_PASS,        STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_pin,      [5],     USB3G_PIN,         STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_apn,      [20],    USB3G_APN,         STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_dialnum,  [12],    USB3G_DIALNUM,     STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_connType, [5],     USB3G_CONN_TYPE,   STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_idleTime, [5] ,    USB3G_IDLE_TIME,   STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   usb3g_mtuSize,  [5],     USB3G_MTU_SIZE,    STRING_T, APMIB_T, 0, 0)

/* CONFIG_CUTE_MAHJONG */
MIBDEF(unsigned char,	wan_detect, ,	WAN_DETECT,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	upgrade_kit, ,	UPGRADE_KIT,	BYTE_T, APMIB_T, 0, 0)


MIBDEF(unsigned char,	ddnsEnabled, ,	DDNS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsType, ,	DDNS_TYPE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsDomainName, [MAX_DOMAIN_LEN],	DDNS_DOMAIN_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsUser, [MAX_DOMAIN_LEN],	DDNS_USER,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ddnsPassword, [MAX_NAME_LEN],	DDNS_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	fixedIpMtuSize, ,	FIXED_IP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	dhcpMtuSize, ,	DHCP_MTU_SIZE,	WORD_T, APMIB_T, 0, 0)
#endif // HOME_GATEWAY

MIBDEF(unsigned char,	ntpEnabled, ,	NTP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	daylightsaveEnabled, ,	DAYLIGHT_SAVE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpServerId, ,	NTP_SERVER_ID,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpTimeZone, [8],	NTP_TIMEZONE,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpServerIp1, [4],	NTP_SERVER_IP1,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ntpServerIp2, [4],	NTP_SERVER_IP2,	IA_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	opMode, ,	OP_MODE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wispWanId, ,	WISP_WAN_ID,	BYTE_T, APMIB_T, 0, 0)

#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	wanAccessEnabled, ,	WEB_WAN_ACCESS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
#if defined(CONFIG_RTL_WEB_WAN_ACCESS_PORT)
MIBDEF(unsigned short,	wanAccessPort, , WEB_WAN_ACCESS_PORT, WORD_T, APMIB_T, 0, 0)
#endif
MIBDEF(unsigned char,	pingAccessEnabled, ,	PING_WAN_ACCESS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	hostName, [MAX_NAME_LEN],	HOST_NAME,	STRING_T, APMIB_T, 0, 0)
#endif // #ifdef HOME_GATEWAY


MIBDEF(unsigned char,	rtLogEnabled, ,	REMOTELOG_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	rtLogServer, [4],	REMOTELOG_SERVER,	IA_T, APMIB_T, 0, 0)

#ifdef UNIVERSAL_REPEATER
// for wlan0 interface
MIBDEF(unsigned char,	repeaterEnabled1, ,	REPEATER_ENABLED1,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	repeaterSSID1, [MAX_SSID_LEN],	REPEATER_SSID1,	STRING_T, APMIB_T, 0, 0)

// for wlan1 interface
MIBDEF(unsigned char,	repeaterEnabled2, ,	REPEATER_ENABLED2,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	repeaterSSID2, [MAX_SSID_LEN],	REPEATER_SSID2,	STRING_T, APMIB_T, 0, 0)
#endif // #ifdef UNIVERSAL_REPEATER

MIBDEF(unsigned char,	wifiSpecific, ,	WIFI_SPECIFIC,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wscIntf, , WSC_INTF_INDEX,	BYTE_T, APMIB_T, 0, 0)


#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	pppServiceName, [41],	PPP_SERVICE_NAME,	STRING_T, APMIB_T, 0, 0)

#ifdef DOS_SUPPORT
MIBDEF(unsigned long,	dosEnabled, ,	DOS_ENABLED,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	syssynFlood, ,	DOS_SYSSYN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	sysfinFlood, ,	DOS_SYSFIN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	sysudpFlood, ,	DOS_SYSUDP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	sysicmpFlood, ,	DOS_SYSICMP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipsynFlood, ,	DOS_PIPSYN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipfinFlood, ,	DOS_PIPFIN_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipudpFlood, ,	DOS_PIPUDP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pipicmpFlood, ,	DOS_PIPICMP_FLOOD,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	blockTime, ,	DOS_BLOCK_TIME,	WORD_T, APMIB_T, 0, 0)
#endif // #ifdef DOS_SUPPORT

MIBDEF(unsigned char,	vpnPassthruIPsecEnabled, ,	VPN_PASSTHRU_IPSEC_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vpnPassthruPPTPEnabled, ,	VPN_PASSTHRU_PPTP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vpnPassthruL2TPEnabled, ,	VPN_PASSTHRU_L2TP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cusPassThru, ,	CUSTOM_PASSTHRU_ENABLED,	BYTE_T, APMIB_T, 0, 0)
#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
MIBDEF(unsigned char,   netSniper, ,  NET_SNIPER_ENABLED,    BYTE_T, APMIB_T, 0, 0)
#endif
MIBDEF(unsigned char,	pptpSecurityEnabled, ,	PPTP_SECURITY_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	igmpproxyDisabled, ,	IGMP_PROXY_DISABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpMppcEnabled, ,	PPTP_MPPC_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	pptpIdleTime, ,	PPTP_IDLE_TIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pptpConnectType, ,	PPTP_CONNECTION_TYPE,	BYTE_T, APMIB_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(unsigned char,	pptpSecurityLength, ,	PPTP_SECURITY_LENGTH,	BYTE_T, APMIB_T, 0, 0)
#endif
#endif // #ifdef HOME_GATEWAY

MIBDEF(unsigned char,   mibVer, , MIB_VER,    BYTE_T, APMIB_T, 0, 0)

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT 
MIBDEF(voipCfgParam_t,	voipCfgParam, ,	VOIP_CFG,	TABLE_LIST_T, APMIB_T, 0, 0) 
#endif

MIBDEF(unsigned char,	startMp, ,	START_MP,	BYTE_T, APMIB_T, 0, 0)

#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
MIBDEF(radvdCfgParam_t,			radvdCfgParam, ,	IPV6_RADVD_PARAM,	RADVDPREFIX_T, APMIB_T, 0, 0)
MIBDEF(dnsv6CfgParam_t,	        dnsCfgParam, ,		IPV6_DNSV6_PARAM,	DNSV6_T, APMIB_T, 0, 0)
MIBDEF(dhcp6sCfgParam_t,	    dhcp6sCfgParam, ,	IPV6_DHCPV6S_PARAM,	DHCPV6S_T, APMIB_T, 0, 0)
MIBDEF(dhcp6cCfgParam_t,		dhcp6cCfgParam, ,	IPV6_DHCPV6C_PARAM, DHCPV6C_T, APMIB_T, 0, 0)
MIBDEF(addrIPv6CfgParam_t,	    addrIPv6CfgParam, ,	IPV6_ADDR_PARAM,	ADDR6_T, APMIB_T, 0, 0)
MIBDEF(addr6CfgParam_t,			addr6CfgParam, , 	IPV6_ADDR6_PARAM,	ADDRV6_T, APMIB_T, 0, 0)
MIBDEF(addr6CfgParam_t, 		addr6LanCfgParam, ,	IPV6_ADDR_LAN_PARAM,ADDRV6_T, APMIB_T, 0, 0)
MIBDEF(addr6CfgParam_t, 		addr6WanCfgParam, , IPV6_ADDR_WAN_PARAM,ADDRV6_T, APMIB_T, 0, 0)
MIBDEF(addr6CfgParam_t, 		addr6GwCfgParam, , 	IPV6_ADDR_GW_PARAM,ADDRV6_T, APMIB_T, 0, 0)
MIBDEF(addr6CfgParam_t, 		addr6PrefixCfgParam, , IPV6_ADDR_PFEFIX_PARAM,ADDRV6_T, APMIB_T, 0, 0)
MIBDEF(addr6CfgParam_t, 		addr6DnsCfgParam, , IPV6_ADDR_DNS_PARAM,ADDRV6_T, APMIB_T, 0, 0)
MIBDEF(tunnelCfgParam_t,	    tunnelCfgParam, ,	IPV6_TUNNEL_PARAM,	TUNNEL6_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,			linkType, ,		IPV6_LINK_TYPE,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,			orignType, , 		IPV6_ORIGIN_TYPE, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,			wanEnable, ,		IPV6_WAN_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,			ipv6DnsAuto, ,		IPV6_DNS_AUTO,		BYTE_T, APMIB_T, 0, 0)
//MIBDEF(unsigned char,			mldproxyEnabled, ,	IPV6_MLD_PROXY_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,			mldproxyDisabled, ,	MLD_PROXY_DISABLED,	BYTE_T, APMIB_T, 0, 0)

#ifdef CONFIG_IPV6_WAN_RA
MIBDEF(radvdCfgParam_t,			radvdCfgParam_wan, ,	IPV6_RADVD_PARAM_WAN,	RADVDPREFIX_T, APMIB_T, 0, 0)
#endif
#endif /* #ifdef CONFIG_IPV6*/
#endif

#ifdef CONFIG_RTL_BT_CLIENT
MIBDEF(unsigned char,	uploadDir, [64] , BT_UPLOAD_DIR,  STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	downloadDir, [64] , BT_DOWNLOAD_DIR,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	uLimit, ,	BT_TOTAL_ULIMIT,  DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	dLimit, ,	BT_TOTAL_DLIMIT,	 DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	refreshTime, ,	BT_REFRESH_TIME, BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	bt_enabled, ,	BT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
#endif
#ifdef PPP_POWEROFF_DISCONNECT
MIBDEF(unsigned short,	pppSessionNum, ,	PPP_SESSION_NUM,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	pppServerMac, [6],	PPP_SERVER_MAC,	BYTE6_T, APMIB_T, 0, 0)
#endif
/*+++++added by Jack for Tr-069 configuration+++++*/
#if defined(HAVE_TR069)
//MIBDEF(unsigned char,	cwmp_onoff, ,	CWMP_ID,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_enabled, , CWMP_ENABLED,   BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ProvisioningCode, [CWMP_PROVISION_CODE_LEN],	CWMP_PROVISIONINGCODE,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACSURL, [CWMP_ACS_URL_LEN],	CWMP_ACS_URL,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACSUserName, [CWMP_ACS_USERNAME_LEN],	CWMP_ACS_USERNAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACSPassword, [CWMP_ACS_PASSWD_LEN],	CWMP_ACS_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_InformEnable, ,	CWMP_INFORM_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_InformInterval, ,	CWMP_INFORM_INTERVAL,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_InformTime, ,	CWMP_INFORM_TIME,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ConnReqUserName, [CWMP_CONREQ_USERNAME_LEN],	CWMP_CONREQ_USERNAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ConnReqPassword, [CWMP_CONREQ_PASSWD_LEN],	CWMP_CONREQ_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_UpgradesManaged, ,	CWMP_ACS_UPGRADESMANAGED,	BYTE_T, APMIB_T, 0, 0)
#if 1
MIBDEF(unsigned char,	cwmp_LANConfPassword, [CWMP_LANCONF_PASSWD_LEN],	CWMP_LAN_CONFIGPASSWD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_SerialNumber, [CWMP_SERIALNUMBER_LEN],	CWMP_SERIALNUMBER,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_DHCP_ServerConf, ,	CWMP_DHCP_SERVERCONF,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_LAN_IPIFEnable, ,	CWMP_LAN_IPIFENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_LAN_EthIFEnable, ,	CWMP_LAN_ETHIFENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_WLAN_BasicEncry, ,	CWMP_WLAN_BASICENCRY,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_WLAN_WPAEncry, ,	CWMP_WLAN_WPAENCRY,	BYTE_T, APMIB_T, 0, 0)
#endif
MIBDEF(unsigned char,	cwmp_DL_CommandKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_DL_COMMANDKEY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_DL_StartTime, ,	CWMP_DL_STARTTIME,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_DL_CompleteTime, ,	CWMP_DL_COMPLETETIME,	WORD_T, APMIB_T, 0, 0)

MIBDEF(unsigned int,	cwmp_DL_FaultCode, ,	CWMP_DL_FAULTCODE,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_Inform_EventCode, ,	CWMP_INFORM_EVENTCODE,	WORD_T, APMIB_T, 0, 0)





MIBDEF(unsigned char,	cwmp_RB_CommandKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_RB_COMMANDKEY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACS_ParameterKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_ACS_PARAMETERKEY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_CERT_Password, [CWMP_CERT_PASSWD_LEN+1],	CWMP_CERT_PASSWORD,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_Flag, ,	CWMP_FLAG,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_SI_CommandKey, [CWMP_COMMAND_KEY_LEN+1],	CWMP_SI_COMMANDKEY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_ParameterKey, [CWMP_COMMAND_KEY_LEN+1],    CWMP_PARAMETERKEY,      STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,    cwmp_pppconn_instnum, , CWMP_PPPCON_INSTNUM,    DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,    cwmp_ipconn_instnum, ,  CWMP_IPCON_INSTNUM,     DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_pppconn_created, , CWMP_PPPCON_CREATED,    BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_ipconn_created, ,  CWMP_IPCON_CREATED,     BYTE_T, APMIB_T, 0, 0) 

#ifdef _PRMT_USERINTERFACE_
MIBDEF(unsigned char,	UIF_PW_Required, ,	UIF_PW_REQUIRED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_PW_User_Sel, ,	UIF_PW_USER_SEL,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_Upgrade, ,	UIF_UPGRADE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	UIF_WarrantyDate, ,	UIF_WARRANTYDATE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_AutoUpdateServer, [256],	UIF_AUTOUPDATESERVER,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	UIF_UserUpdateServer, [256],	UIF_USERUPDATESERVER,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef _PRMT_USERINTERFACE_

MIBDEF(unsigned char,	cwmp_ACS_KickURL, [CWMP_KICK_URL],	CWMP_ACS_KICKURL,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ACS_DownloadURL, [CWMP_DOWNLOAD_URL],	CWMP_ACS_DOWNLOADURL,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned int,	cwmp_ConnReqPort, ,	CWMP_CONREQ_PORT,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_ConnReqPath, [CONN_REQ_PATH_LEN],	CWMP_CONREQ_PATH,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmp_Flag2, ,	CWMP_FLAG2,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_NotifyList, [CWMP_NOTIFY_LIST_LEN],        CWMP_NOTIFY_LIST,       STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_ACSURL_old, [CWMP_ACS_URL_LEN],    CWMP_ACS_URL_OLD,       STRING_T, APMIB_T, 0, 0)

#ifdef _PRMT_TR143_
MIBDEF(unsigned char,	tr143_udpecho_enable, ,	TR143_UDPECHO_ENABLE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	tr143_udpecho_itftype, ,	TR143_UDPECHO_ITFTYPE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	tr143_udpecho_srcip, [4],	TR143_UDPECHO_SRCIP,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	tr143_udpecho_port, ,	TR143_UDPECHO_PORT,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	tr143_udpecho_plus, ,	TR143_UDPECHO_PLUS,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef _PRMT_TR143_
MIBDEF(unsigned int,    cwmp_UserInfo_Result, , CWMP_USERINFO_RESULT,   DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_Needreboot, ,      CWMP_NEED_REBOOT ,      BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_Persistent_Data,[256] ,    CWMP_PERSISTENT_DATA,   STRING_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	cwmpPppoeDisabled, ,	   CWMP_PPPOE_DISABLED, 	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	cwmpDhcpDisabled, ,	   CWMP_DHCP_DISABLED, 	BYTE_T, APMIB_T, 0, 0)

#endif // #ifdef CONFIG_CWMP_TR069

MIBDEF(LANGUAGE_TYPE_T,webLang, ,WEB_LANGUAGE,DWORD_T,APMIB_T,0,0)

// SNMP, Forrest added, 2007.10.25.
#ifdef CONFIG_SNMP
MIBDEF(unsigned char,	snmpEnabled, ,	SNMP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpName, [MAX_SNMP_NAME_LEN],	SNMP_NAME,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpLocation, [MAX_SNMP_LOCATION_LEN],	SNMP_LOCATION,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpContact, [MAX_SNMP_CONTACT_LEN],	SNMP_CONTACT,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpRWCommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_RWCOMMUNITY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpROCommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_ROCOMMUNITY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpTrapReceiver1, [4],	SNMP_TRAP_RECEIVER1,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpTrapReceiver2, [4],	SNMP_TRAP_RECEIVER2,	IA_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpTrapReceiver3, [4],	SNMP_TRAP_RECEIVER3,	IA_T, APMIB_T, 0, 0)
#endif // #ifdef CONFIG_SNMP

MIBDEF(unsigned short,	system_time_year, ,	SYSTIME_YEAR,	WORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_month, ,	SYSTIME_MON,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_day, ,	SYSTIME_DAY,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_hour, ,	SYSTIME_HOUR,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_min, ,	SYSTIME_MIN,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	system_time_sec, ,	SYSTIME_SEC,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlan11nOnOffTKIP, ,	WLAN_11N_ONOFF_TKIP,	BYTE_T, APMIB_T, 0, 0)

MIBDEF(unsigned char,	dhcpRsvdIpEnabled, ,	DHCPRSVDIP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	dhcpRsvdIpNum, ,	DHCPRSVDIP_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(DHCPRSVDIP_T,	dhcpRsvdIpArray, [MAX_DHCP_RSVD_IP_NUM],	DHCPRSVDIP_TBL,	DHCPRSVDIP_ARRY_T, APMIB_T, 0, mib_dhcpRsvdIp_tbl)
#ifdef WLAN_PROFILE
MIBDEF(unsigned char,	wlan_profile_enable1, , PROFILE_ENABLED1,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlan_profile_num1, ,	PROFILE_NUM1,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(WLAN_PROFILE_T,	wlan_profile_arrary1, [MAX_WLAN_PROFILE_NUM],PROFILE_TBL1,PROFILE_ARRAY_T, APMIB_T, 0, mib_wlan_profile_tbl1)

MIBDEF(unsigned char,	wlan_profile_enable2, , PROFILE_ENABLED2,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	wlan_profile_num2, ,	PROFILE_NUM2,		BYTE_T, APMIB_T, 0, 0)
MIBDEF(WLAN_PROFILE_T,	wlan_profile_arrary2, [MAX_WLAN_PROFILE_NUM],PROFILE_TBL2,PROFILE_ARRAY_T, APMIB_T, 0, mib_wlan_profile_tbl2)
#endif

#ifdef HOME_GATEWAY
MIBDEF(unsigned char,	portFwEnabled, ,	PORTFW_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	portFwNum, ,	PORTFW_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(PORTFW_T,	portFwArray, [MAX_VIRTUAL_SERVER_NUM],	PORTFW_TBL,	PORTFW_ARRAY_T, APMIB_T, 0, mib_portfw_tbl)
#else
MIBDEF(PORTFW_T,	portFwArray, [MAX_FILTER_NUM],	PORTFW_TBL,	PORTFW_ARRAY_T, APMIB_T, 0, mib_portfw_tbl)
#endif
MIBDEF(unsigned char,	ipFilterEnabled, ,	IPFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ipFilterNum, ,	IPFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(IPFILTER_T,	ipFilterArray, [MAX_FILTER_NUM],	IPFILTER_TBL,	IPFILTER_ARRAY_T, APMIB_T, 0, mib_ipfilter_tbl)

MIBDEF(unsigned char,	portFilterEnabled, ,	PORTFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	portFilterNum, ,	PORTFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(PORTFILTER_T,	portFilterArray, [MAX_FILTER_NUM],	PORTFILTER_TBL,	PORTFILTER_ARRAY_T, APMIB_T, 0, mib_portfilter_tbl)

MIBDEF(unsigned char,	macFilterEnabled, ,	MACFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	macFilterNum, ,	MACFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(MACFILTER_T,	macFilterArray, [MAX_MAC_FILTER_NUM],	MACFILTER_TBL,	MACFILTER_ARRAY_T, APMIB_T, 0, mib_macfilter_tbl)
#else
MIBDEF(MACFILTER_T,	macFilterArray, [MAX_FILTER_NUM],	MACFILTER_TBL,	MACFILTER_ARRAY_T, APMIB_T, 0, mib_macfilter_tbl)
#endif

MIBDEF(unsigned char,	triggerPortEnabled, ,	TRIGGERPORT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	triggerPortNum, ,	TRIGGERPORT_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(TRIGGERPORT_T,	triggerPortArray, [MAX_PORT_TRIGGER_NUM],	TRIGGERPORT_TBL,	TRIGGERPORT_ARRAY_T, APMIB_T, 0, mib_triggerport_tbl)
#else
MIBDEF(TRIGGERPORT_T,	triggerPortArray, [MAX_FILTER_NUM],	TRIGGERPORT_TBL,	TRIGGERPORT_ARRAY_T, APMIB_T, 0, mib_triggerport_tbl)
#endif
MIBDEF(unsigned char,	urlFilterEnabled, ,	URLFILTER_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	urlFilterNum, ,	URLFILTER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(URLFILTER_T,	urlFilterArray, [MAX_URL_FILTER_NUM],	URLFILTER_TBL,	URLFILTER_ARRAY_T, APMIB_T, 0, mib_urlfilter_tbl)
#else
MIBDEF(URLFILTER_T,	urlFilterArray, [MAX_URLFILTER_NUM],	URLFILTER_TBL,	URLFILTER_ARRAY_T, APMIB_T, 0, mib_urlfilter_tbl)
#endif
MIBDEF(unsigned char,	VlanConfigEnabled, ,	VLANCONFIG_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	VlanConfigNum, ,	VLANCONFIG_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
#if defined(VLAN_CONFIG_SUPPORTED)
MIBDEF(VLAN_CONFIG_T,	VlanConfigArray, [MAX_IFACE_VLAN_CONFIG],	VLANCONFIG_TBL,	VLANCONFIG_ARRAY_T, APMIB_T, 0, mib_vlanconfig_tbl)
#endif

#ifdef ROUTE_SUPPORT
MIBDEF(unsigned char,	staticRouteEnabled, ,	STATICROUTE_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	staticRouteNum, ,	STATICROUTE_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(STATICROUTE_T,	staticRouteArray, [MAX_ROUTE_NUM],	STATICROUTE_TBL,	STATICROUTE_ARRAY_T, APMIB_T, 0, mib_staticroute_tbl)
MIBDEF(unsigned char,	dynamicRouteEnabled, ,	DYNAMICROUTE_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	natEnabled, ,	NAT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ripEnabled, ,	RIP_ENABLED,	BYTE_T, APMIB_T, 0, 0)
//#ifdef ROUTE6D_SUPPORT
MIBDEF(unsigned char,	rip6Enabled, ,	RIP6_ENABLED,	BYTE_T, APMIB_T, 0, 0)
//#endif
#endif // #ifdef ROUTE_SUPPORT

MIBDEF(unsigned char,	sambaEnabled, ,	SAMBA_ENABLED,	BYTE_T, APMIB_T, 0, 0)
#ifdef VPN_SUPPORT
MIBDEF(unsigned char,	ipsecTunnelEnabled, ,	IPSECTUNNEL_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ipsecTunnelNum, ,	IPSECTUNNEL_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(IPSECTUNNEL_T,	ipsecTunnelArray, [MAX_TUNNEL_NUM],	IPSECTUNNEL_TBL,	IPSECTUNNEL_ARRAY_T, APMIB_T, 0, mib_ipsectunnel_tbl)
MIBDEF(unsigned char,	ipsecNattEnabled, ,	IPSEC_NATT_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	ipsecRsaKeyFile, [MAX_RSA_FILE_LEN],	IPSEC_RSA_FILE,	BYTE_ARRAY_T, APMIB_T, 0, 0)
#endif // #ifdef VPN_SUPPORT

#endif // #ifdef HOME_GATEWAY

#ifdef CONFIG_RTL_SIMPLE_CONFIG
MIBDEF(unsigned char,	scDeviceType, ,	SC_DEVICE_TYPE,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	scDeviceName, [MAX_SC_DEVICE_NAME],	SC_DEVICE_NAME,	STRING_T, APMIB_T, 0, 0)
#endif

#ifdef TLS_CLIENT
MIBDEF(unsigned char,	certRootNum, ,	CERTROOT_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(CERTROOT_T,	certRootArray, [MAX_CERTROOT_NUM],	CERTROOT_TBL,	CERTROOT_ARRAY_T, APMIB_T, 0, mib_certroot_tbl)
MIBDEF(unsigned char,	certUserNum, ,	CERTUSER_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(CERTUSER_T,	certUserArray, [MAX_CERTUSER_NUM],	CERTUSER_TBL,	CERTUSER_ARRAY_T, APMIB_T, 0, mib_certuser_tbl)
MIBDEF(unsigned char,	rootIdx, ,	ROOT_IDX,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	userIdx, ,	USER_IDX,	BYTE_T, APMIB_T, 0, 0)
#endif // #ifdef TLS_CLIENT

#ifdef HOME_GATEWAY

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */

MIBDEF(unsigned char,	qosEnabled, ,	QOS_ENABLED,	BYTE_T, APMIB_T, 0, 0)
//MIBDEF(unsigned char,	qosAutoRateEnabled, ,	QOS_AUTO_RATE_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	qosAutoUplinkSpeed, ,	QOS_AUTO_UPLINK_SPEED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,	qosManualUplinkSpeed, ,	QOS_MANUAL_UPLINK_SPEED,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	qosAutoDownLinkSpeed, ,	QOS_AUTO_DOWNLINK_SPEED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned long,	qosManualDownLinkSpeed, ,	QOS_MANUAL_DOWNLINK_SPEED,	DWORD_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	qosRuleNum, ,	QOS_RULE_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	qosMode, ,	QOS_MODE,	BYTE_T, APMIB_T, 0, 0)
//MIBDEF(unsigned char,	qosScheRule,[SCHEDULE_NAME_LEN] ,	QOS_SCHERULE,	BYTE_T, APMIB_T, 0, 0)
#endif // #if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)

#if defined(GW_QOS_ENGINE)
MIBDEF(QOS_T,	qosRuleArray, [MAX_QOS_RULE_NUM],	QOS_RULE_TBL,	QOS_ARRAY_T, APMIB_T, 0, mib_qos_tbl)
#endif // #if defined(GW_QOS_ENGINE)

#if defined(QOS_BY_BANDWIDTH)
MIBDEF(IPQOS_T,	qosRuleArray, [MAX_QOS_RULE_NUM],	QOS_RULE_TBL,	QOS_ARRAY_T, APMIB_T, 0, mib_qos_tbl)
#endif // #if defined(GW_QOS_ENGINE)

#endif//HOME_GATEWAY

MIBDEF(unsigned char,	snmpROcommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_RO_COMMUNITY,	STRING_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	snmpRWcommunity, [MAX_SNMP_COMMUNITY_LEN],	SNMP_RW_COMMUNITY,	STRING_T, APMIB_T, 0, 0)

MIBDEF(CONFIG_WLAN_SETTING_T,	wlan, [NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1],	WLAN_ROOT,	TABLE_LIST_T, APMIB_T, 0, mib_wlan_table)

//#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
MIBDEF(unsigned char,	dualBankEnabled,	, DUALBANK_ENABLED,	BYTE_T, APMIB_T, 0, 0) //default test
MIBDEF(unsigned char,	wlanBand2G5GSelect,	, WLAN_BAND2G5G_SELECT,	BYTE_T, APMIB_T, 0, 0)
#ifdef CONFIG_RTL_VLAN_SUPPORT
MIBDEF(unsigned char,	vlanEnable, ,	VLAN_ENABLED,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,	vlanNum, ,	VLAN_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned short,	currentVlanId, ,	CURRENT_VLAN_ID,	WORD_T, APMIB_T, 0, 0)
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
MIBDEF(unsigned char,	currentVlanFwdRule, ,	CURRENT_VLAN_FORWARD_RULE,	BYTE_T, APMIB_T, 0, 0)
#endif
MIBDEF(VLAN_T ,vlanArray, [MAX_VLAN_NUM],	VLAN_TBL,	VLAN_ARRAY_T, APMIB_T, 0, mib_vlan_tbl)
MIBDEF(unsigned char,	netIfaceNum, ,	NETIFACE_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(NETIFACE_T ,netIfaceArray, [MAX_NETIFACE_NUM],	NETIFACE_TBL,	NETIFACE_ARRAY_T, APMIB_T, 0, mib_netiface_tbl)
MIBDEF(unsigned char,	vlanNetifBindNum, ,	VLAN_NETIF_BIND_TBL_NUM,	BYTE_T, APMIB_T, 0, 0)
MIBDEF(VLAN_NETIF_BIND_T ,vlanNetifBindArray, [MAX_VLAN_NETIF_BIND_NUM],	VLAN_NETIF_BIND_TBL,	VLAN_NETIF_BIND_ARRAY_T, APMIB_T, 0, mib_vlan_netif_bind_tbl)
#endif

#ifdef HOME_GATEWAY

#endif // HOME_GATEWAY
#if defined(HAVE_TR069)
#if defined(WLAN_SUPPORT)
MIBDEF(unsigned char,   cwmp_WlanConf_Enabled, ,    CWMP_WLANCONF_ENABLED,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(unsigned char,   cwmp_WlanConf_EntryNum, ,   CWMP_WLANCONF_TBL_NUM,  BYTE_T, APMIB_T, 0, 0)
MIBDEF(CWMP_WLANCONF_T, cwmp_WlanConfArray, [MAX_CWMP_WLANCONF_NUM],    CWMP_WLANCONF_TBL,  CWMP_WLANCONF_ARRAY_T, APMIB_T, 0, mib_cwmp_wlanconf_tbl)
#endif
#endif
#endif // #ifdef MIB_IMPORT

#ifdef MIB_DHCPRSVDIP_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ipAddr, [4],	DHCPRSVDIP_IPADDR,	IA_T, DHCPRSVDIP_T, 0, 0)
MIBDEF(unsigned char,	macAddr,	[6],	DHCPRSVDIP_MACADDR,	BYTE6_T, DHCPRSVDIP_T, 0, 0)
MIBDEF(unsigned char,	hostName, [32],	DHCPRSVDIP_HOSTNAME,	STRING_T, DHCPRSVDIP_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(unsigned char,	enabled, ,	DHCPRSVDIP_ENABLED,	BYTE_T, DHCPRSVDIP_T, 0, 0)
MIBDEF(unsigned char,	used, , DHCPRSVDIP_USED, BYTE_T, DHCPRSVDIP_T, 0, 0)
MIBDEF(unsigned char,	index, ,	DHCPRSVDIP_INDEX, BYTE_T, DHCPRSVDIP_T, 0, 0)
#endif
#endif // #ifdef MIB_DHCPRSVDIP_IMPORT

#ifdef WLAN_PROFILE
#ifdef MIB_WLAN_PROFILE_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ssid, [MAX_SSID_LEN],	PROFILE_SSID,	STRING_T,	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	encryption, ,			PROFILE_ENC,	BYTE_T, 	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	auth, ,					PROFILE_AUTH,	BYTE_T, 	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wpa_cipher, , 			PROFILE_WPA_CIPHER,	BYTE_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wpaPSK, [MAX_PSK_LEN+1],PROFILE_WPA_PSK,STRING_T, 	WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wep_default_key, ,		PROFILE_WEP_DEFAULT_KEY,BYTE_T,WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey1, [WEP128_KEY_LEN],PROFILE_WEP_KEY1,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey2, [WEP128_KEY_LEN],PROFILE_WEP_KEY2,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey3, [WEP128_KEY_LEN],PROFILE_WEP_KEY3,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKey4, [WEP128_KEY_LEN],PROFILE_WEP_KEY4,BYTE_ARRAY_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wepKeyType, ,	PROFILE_WEP_KEY_TYPE,	BYTE_T, WLAN_PROFILE_T, 0, 0)
MIBDEF(unsigned char,	wpaPSKFormat, ,	PROFILE_PSK_FORMAT,	BYTE_T, WLAN_PROFILE_T, 0, 0)
#endif // #ifdef MIB_WLAN_PROFILE_IMPORT
#endif

#ifdef MIB_SCHEDULE_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	text, [SCHEDULE_NAME_LEN],	SCHEDULE_TEXT,	STRING_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	eco,	,	SCHEDULE_ECO,	WORD_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	fTime, ,	SCHEDULE_FTIME,	WORD_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	tTime,	,	SCHEDULE_TTIME,	WORD_T, SCHEDULE_T, 0, 0)
MIBDEF(unsigned short,	day,	,	SCHEDULE_DAY,	WORD_T, SCHEDULE_T, 0, 0)
#endif // #ifdef MIB_SCHEDULE_IMPORT

#ifdef MIB_MACFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
#ifdef KLD_ENABLED
MIBDEF(unsigned char,	scheRule, [SCHEDULE_NAME_LEN],	MACFILTER_SCHEDULE_RULE,	BYTE_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	index, ,	MACFILTER_ITEM_INDEX, BYTE_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	Enabled, ,	MACFILTER_ITEM_ENABLED, BYTE_T, MACFILTER_T, 0, 0)
#endif
MIBDEF(unsigned char,	macAddr, [6],	MACFILTER_MACADDR,	BYTE6_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	MACFILTER_COMMENT,	STRING_T, MACFILTER_T, 0, 0)
#endif // #ifdef MIB_MACFILTER_IMPORT

#ifdef KLD_ENABLED
#ifdef MIB_DOMAINFILTER_IMPORT
MIBDEF(unsigned char,	index,	,						DOMAINFILTER_INDEX,	BYTE_T, 	DOMAINFILTER_T, 0, 0)
MIBDEF(unsigned char,	enabled,,						DOMAINFILTER_ENABLED,	BYTE_T, 	DOMAINFILTER_T, 0, 0)
MIBDEF(unsigned char,	domainAddr,[60],				DOMAINFILTER_ADDR,	STRING_T, 	DOMAINFILTER_T, 0, 0)
MIBDEF(unsigned char,	scheRule,[SCHEDULE_NAME_LEN],	DOMAINFILTER_SCHERULE,	STRING_T, 	DOMAINFILTER_T, 0, 0)
#endif

#ifdef MIB_FIREWALLRULE_IMPORT
MIBDEF(unsigned char,	srcStartAddr,[4],	FIREWALLRULE_SRC_STARTADDR,	STRING_T, 	FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	srcEndAddr,[4],		FIREWALLRULE_SRC_ENDADDR,	STRING_T,	FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	dstStartAddr,[4],	FIREWALLRULE_DST_STARTADDR,	STRING_T, 	FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	dstEndAddr,[4],		FIREWALLRULE_DST_ENDADDR,	STRING_T, 	FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	scheRule,[SCHEDULE_NAME_LEN], 	FIREWALLRULE_SCHERULE,	STRING_T,	FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	comment,[32], 	FIREWALLRULE_COMMENT,	STRING_T,	FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned short,	fromPort,	,	FIREWALLRULE_FROMPORT,	WORD_T, FIREWALLRULE_T, 0, 0)	
MIBDEF(unsigned short,	toPort,	,	FIREWALLRULE_TOPORT,	WORD_T, FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	src_interface,	,	FIREWALLRULE_SRC_INTERFACE,	BYTE_T, FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	dst_interface,	,	FIREWALLRULE_DST_INTERFACE,	BYTE_T, FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	protoType,	,	FIREWALLRULE_PROTOTYPE,	BYTE_T, FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	action,	,		FIREWALLRULE_ACTION,	BYTE_T, FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	index_id,	,	FIREWALLRULE_INDEX_ID,	BYTE_T, FIREWALLRULE_T, 0, 0)
MIBDEF(unsigned char,	enabled,	,	FIREWALLRULE_ENTRY_ENABLED,	BYTE_T, FIREWALLRULE_T, 0, 0)
#endif

#ifdef MIB_LOCK_CLIENT_IMPORT
MIBDEF(unsigned char,	macAddr,[6], 	LOCK_CLIENT_MACADDR,	STRING_T,	LOCKCLIENT_T, 0, 0)
#endif
#endif

//#if	1//def HOME_GATEWAY
#ifdef 	HOME_GATEWAY
#ifdef MIB_PORTFW_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
#ifdef KLD_ENABLED
MIBDEF(unsigned char,	index,	,	PORTFW_INDEX,	BYTE_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,   Enabled,    ,   PORTFW_ENTRY_ENABLED,   BYTE_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,   scheduleRule,[20],  PORTFW_SCHEDULE_RULE,  STRING_T, PORTFW_T, 0, 0)
MIBDEF(unsigned short,  PrivatefromPort,    ,   PORTFW_PRIVATE_FROM_PORT,  WORD_T, PORTFW_T, 0, 0)
MIBDEF(unsigned short,  PrivatetoPort,  ,   PORTFW_PRIVATE_TO_PORT,    WORD_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,   inBoundFilterRule,[20], PORTFW_INBOUND_FILTER_RULE,    STRING_T, PORTFW_T, 0, 0)
#endif
MIBDEF(unsigned char,	ipAddr, [4],	PORTFW_IPADDR,	IA_T, PORTFW_T, 0, 0)
MIBDEF(unsigned short,	fromPort,	,	PORTFW_FROMPORT,	WORD_T, PORTFW_T, 0, 0)
MIBDEF(unsigned short,	toPort, ,	PORTFW_TOPORT,	WORD_T, PORTFW_T, 0, 0)
MIBDEF(unsigned short,  svrport, ,      PORTFW_SVRPORT, WORD_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,   svrName,         [COMMENT_LEN], PORTFW_SVRNAME, STRING_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,	protoType, ,	PORTFW_PROTOTYPE,	BYTE_T, PORTFW_T, 0, 0)
MIBDEF(unsigned char,	comment,	 [COMMENT_LEN],	PORTFW_COMMENT,	STRING_T, PORTFW_T, 0, 0)
#endif // #ifdef MIB_PORTFW_IMPORT

#ifdef MIB_IPFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ipAddr, [4],	IPFILTER_IPADDR,	IA_T, IPFILTER_T, 0, 0)
MIBDEF(unsigned char,   ipAddrEnd, [4],  IPFILTER_IPADDR_END,    IA_T, IPFILTER_T, 0, 0)
MIBDEF(unsigned char,	protoType,	,	IPFILTER_PROTOTYPE,	BYTE_T, IPFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	IPFILTER_COMMENT,	STRING_T, IPFILTER_T, 0, 0)
#ifdef CONFIG_IPV6
MIBDEF(unsigned char, 	ip6Addr, [48], 	IPFILTER_IP6ADDR,	STRING_T, IPFILTER_T, 0, 0)
MIBDEF(unsigned char,	ipVer, 	,	IPFILTER_IP_VERSION,	BYTE_T, IPFILTER_T, 0, 0)
#endif
#endif // #ifdef MIB_IPFILTER_IMPORT

#ifdef MIB_PORTFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned short,	fromPort, ,	PORTFILTER_FROMPORT,	WORD_T, PORTFILTER_T, 0, 0)
MIBDEF(unsigned short,	toPort, ,	PORTFILTER_TOPORT,	WORD_T, PORTFILTER_T, 0, 0)
MIBDEF(unsigned char,	protoType, ,	PORTFILTER_PROTOTYPE,	BYTE_T, PORTFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	PORTFILTER_COMMENT,	STRING_T, PORTFILTER_T, 0, 0)
MIBDEF(unsigned char,	ipVer, , 	PORTFILTER_IPVERSION, 	BYTE_T, PORTFILTER_T, 0, 0)
#endif // #ifdef MIB_PORTFILTER_IMPORT

#ifdef MIB_TRIGGERPORT_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
#ifdef KLD_ENABLED
MIBDEF(unsigned char,	index,	,	TRIGGERPORT_INDEX,	BYTE_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,   Enabled,    ,   TRIGGERPORT_ENTRY_ENABLED,   BYTE_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,   scheduleRule,[8],  TRIGGERPORT_SCHEDULE_RULE,  STRING_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,   triPortRng,[65],  TRIGGERPORT_TRI_PORTRNG,  STRING_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,   incPortRng,[65],  TRIGGERPORT_INC_PORTRNG,  STRING_T, TRIGGERPORT_T, 0, 0)
#endif
MIBDEF(unsigned short,	tri_fromPort, ,	TRIGGERPORT_TRI_FROMPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned short,	tri_toPort, ,	TRIGGERPORT_TRI_TOPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,	tri_protoType, ,	TRIGGERPORT_TRI_PROTOTYPE,	BYTE_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned short,	inc_fromPort, ,	TRIGGERPORT_INC_FROMPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned short,	inc_toPort, ,	TRIGGERPORT_INC_TOPORT,	WORD_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,	inc_protoType, ,	TRIGGERPORT_INC_PROTOTYPE,	BYTE_T, TRIGGERPORT_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	TRIGGERPORT_COMMENT,	STRING_T, TRIGGERPORT_T, 0, 0)
#endif // #ifdef MIB_TRIGGERPORT_IMPORT

#ifdef MIB_URLFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */

MIBDEF(unsigned char,	urlAddr, [31],	URLFILTER_URLADDR,	STRING_T, URLFILTER_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(unsigned char,	index, ,	URLFILTER_INDEX,	BYTE_T, URLFILTER_T, 0, 0)
MIBDEF(unsigned char,	enabled, ,	URLFILTER_ENABLED_SUB,	BYTE_T, URLFILTER_T, 0, 0)
MIBDEF(unsigned char,	scheRule, [SCHEDULE_NAME_LEN+1],	URLFILTER_SCHERULE,	STRING_T, URLFILTER_T, 0, 0)
#endif

#endif // #ifdef MIB_URLFILTER_IMPORT
#ifdef MIB_VLAN_CONFIG_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	enabled, ,	VLANCONFIG_ENTRY_ENABLED,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	netIface, [IFNAMSIZE],	VLANCONFIG_NETIFACE,	STRING_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	tagged, ,	VLANCONFIG_TAGGED,	BYTE_T, VLAN_CONFIG_T, 0, 0)
//MIBDEF(unsigned char,	untagged, ,	VLANCONFIG_UNTAGGED,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	priority, ,	VLANCONFIG_PRIORITY,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned char,	cfi, ,	VLANCONFIG_CFI,	BYTE_T, VLAN_CONFIG_T, 0, 0)
//MIBDEF(unsigned char,	groupId, ,	VLANCONFIG_GROUPID,	BYTE_T, VLAN_CONFIG_T, 0, 0)
MIBDEF(unsigned short,	vlanId, ,	VLANCONFIG_VLANID,	WORD_T, VLAN_CONFIG_T, 0, 0)
#endif // #ifdef MIB_VLAN_CONFIG_IMPORT

#ifdef ROUTE_SUPPORT
#ifdef MIB_STATICROUTE_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */

MIBDEF(unsigned char,	dstAddr, [4],	STATICROUTE_DSTADDR,	IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	netmask, [4],	STATICROUTE_NETMASK,	IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	gateway, [4],	STATICROUTE_GATEWAY,	IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	interface, ,	STATICROUTE_INTERFACE,	BYTE_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	metric, ,	STATICROUTE_METRIC,	BYTE_T, STATICROUTE_T, 0, 0)
#ifdef KLD_ENABLED
MIBDEF(unsigned char,	enabled, ,	STATICROUTE_ENABLE_SUB, BYTE_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,	name,[16] , STATICROUTE_NAME, STRING_T, STATICROUTE_T, 0, 0)
#endif

#endif // #ifdef MIB_STATICROUTE_IMPORT
#endif // #ifdef ROUTE_SUPPORT

#ifdef VPN_SUPPORT
#ifdef MIB_IPSECTUNNEL_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	tunnelId, ,	IPSECTUNNEL_TUNNELID,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	authType, ,	IPSECTUNNEL_AUTHTYPE,	IA_T, IPSECTUNNEL_T, 0, 0)
//local info
MIBDEF(unsigned char,	lcType, ,	IPSECTUNNEL_LCTYPE,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lc_ipAddr, [4],	IPSECTUNNEL_LC_IPADDR,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lc_maskLen, ,	IPSECTUNNEL_LC_MASKLEN,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//remote Info
MIBDEF(unsigned char,	rtType, ,	IPSECTUNNEL_RTTYPE,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rt_ipAddr, [4],	IPSECTUNNEL_RT_IPADDR,	IA_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rt_maskLen, ,	IPSECTUNNEL_RT_MASKLEN,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rt_gwAddr, [4],	IPSECTUNNEL_RT_GWADDR,	IA_T, IPSECTUNNEL_T, 0, 0)
// Key mode common
MIBDEF(unsigned char,	keyMode, ,	IPSECTUNNEL_KEYMODE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//MIBDEF(unsigned char,	espAh, ,	IPSECTUNNEL_ESPAH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	espEncr, ,	IPSECTUNNEL_ESPENCR,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	espAuth, ,	IPSECTUNNEL_ESPAUTH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//MIBDEF(unsigned char,	ahAuth, ,	IPSECTUNNEL_AHAUTH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
//IKE mode
MIBDEF(unsigned char,	conType, ,	IPSECTUNNEL_CONTYPE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	psKey, [MAX_NAME_LEN],	IPSECTUNNEL_PSKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rsaKey, [MAX_RSA_KEY_LEN],	IPSECTUNNEL_RSAKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
//Manual Mode
MIBDEF(unsigned char,	spi, [MAX_SPI_LEN],	IPSECTUNNEL_SPI,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	encrKey, [MAX_ENCRKEY_LEN],	IPSECTUNNEL_ENCRKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	authKey, [MAX_AUTHKEY_LEN],	IPSECTUNNEL_AUTHKEY,	STRING_T, IPSECTUNNEL_T, 0, 0)
// tunnel info
MIBDEF(unsigned char,	enable, ,	IPSECTUNNEL_ENABLE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	connName, [MAX_NAME_LEN],	IPSECTUNNEL_CONNNAME,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lcIdType, ,	IPSECTUNNEL_LCIDTYPE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rtIdType, ,	IPSECTUNNEL_LCIDTYPE,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	lcId, [MAX_NAME_LEN],	IPSECTUNNEL_LCID,	STRING_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	rtId, [MAX_NAME_LEN],	IPSECTUNNEL_RTID,	STRING_T, IPSECTUNNEL_T, 0, 0)
// ike Advanced setup
MIBDEF(unsigned long,	ikeLifeTime, ,	IPSECTUNNEL_IKELIFETIME,	DWORD_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ikeEncr, ,	IPSECTUNNEL_IKEENCR,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ikeAuth, ,	IPSECTUNNEL_IKEAUTH,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ikeKeyGroup, ,	IPSECTUNNEL_IKEKEYGROUP,	BYTE_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned long,	ipsecLifeTime, ,	IPSECTUNNEL_IPSECLIFETIME,	DWORD_T, IPSECTUNNEL_T, 0, 0)
MIBDEF(unsigned char,	ipsecPfs, ,	IPSECTUNNEL_IPSECPFS,	BYTE_T, IPSECTUNNEL_T, 0, 0)
#endif // #ifdef MIB_IPSECTUNNEL_IMPORT
#endif //#ifdef VPN_SUPPORT

#if defined(GW_QOS_ENGINE)
#ifdef MIB_QOS_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	entry_name, [MAX_QOS_NAME_LEN+1],	QOS_ENTRY_NAME,	STRING_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	enabled, ,	QOS_ENTRY_ENABLED,	STRING_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	priority, ,	QOS_PRIORITY,	STRING_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	protocol, ,	QOS_PROTOCOL,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_start, [4],	QOS_LOCAL_IP_START,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_end, [4],	QOS_LOCAL_IP_END,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	local_port_start, ,	QOS_LOCAL_PORT_START,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	local_port_end, ,	QOS_LOCAL_PORT_END,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	remote_ip_start, [4],	QOS_REMOTE_IP_START,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned char,	remote_ip_end, [4],	QOS_REMOTE_IP_END,	IA_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	remote_port_start, ,	QOS_REMOTE_PORT_START,	WORD_T, QOS_T, 0, 0)
MIBDEF(unsigned short,	remote_port_send, ,	QOS_REMOTE_PORT_END,	WORD_T, QOS_T, 0, 0)
#endif // #ifdef MIB_QOS_IMPORT
#endif // #if defined(GW_QOS_ENGINE)

#if defined(QOS_BY_BANDWIDTH)
#ifdef MIB_IPQOS_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	entry_name, [MAX_QOS_NAME_LEN+1],	IPQOS_ENTRY_NAME,	STRING_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	enabled, ,	IPQOS_ENABLED,	BYTE_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	mac, [MAC_ADDR_LEN],	IPQOS_MAC,	BYTE6_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	mode, ,	IPQOS_MODE,	BYTE_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_start, [4],	IPQOS_LOCAL_IP_START,	IA_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	local_ip_end, [4],	IPQOS_LOCAL_IP_END,	IA_T, IPQOS_T, 0, 0)
MIBDEF(unsigned long,	bandwidth, ,	IPQOS_BANDWIDTH,	DWORD_T, IPQOS_T, 0, 0)
MIBDEF(unsigned long,	bandwidth_downlink, ,	IPQOS_BANDWIDTH_DOWNLINK,	DWORD_T, IPQOS_T, 0, 0)
MIBDEF(unsigned long,	local_port_start, ,	IPQOS_PORT_START,	DWORD_T, IPQOS_T, 0, 0)
MIBDEF(unsigned long,	local_port_end, ,	IPQOS_PORT_END,		DWORD_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	protocol, ,		IPQOS_PROTOCOL, 	BYTE_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	weight, , 	IPQOS_WEIGHT, 	BYTE_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,   prior, ,   IPQOS_PRIORITY,   BYTE_T, IPQOS_T, 0, 0)
#ifdef KLD_ENABLED
/*shecdule*/
MIBDEF(unsigned char,	scheRule, [SCHEDULE_NAME_LEN+1],	IPQOS_SCHERULE,	STRING_T, IPQOS_T, 0, 0)
MIBDEF(unsigned char,	index, ,	IPQOS_INDEX,	BYTE_T, IPQOS_T, 0, 0)
#endif
#endif // #ifdef MIB_IPQOS_IMPORT
#endif // #if defined(QOS_BY_BANDWIDTH)
#endif // #ifdef HOME_GATEWAY

#ifdef TLS_CLIENT
#ifdef MIB_CERTROOT_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	CERTROOT_COMMENT,	STRING_T, CERTROOT_T, 0, 0)
#endif // #ifdef MIB_CERTROOT_IMPORT

#ifdef MIB_CERTUSER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	CERTUSER_COMMENT,	STRING_T, CERTUSER_T, 0, 0)
MIBDEF(unsigned char,	pass, [MAX_RS_PASS_LEN],	CERTROOT_PASS,	STRING_T, CERTUSER_T, 0, 0)
#endif // #ifdef MIB_CERTUSER_IMPORT
#endif //#ifdef TLS_CLIENT

#ifdef MIB_MESH_MACFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	macAddr, [6],	MECH_ACL_MACADDR,	BYTE6_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	MECH_ACL_COMMENT,	STRING_T, MACFILTER_T, 0, 0)
#endif // #ifdef MIB_MESH_MACFILTER_IMPORT

#ifdef MIB_WLAN_MACFILTER_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	macAddr, [6],	WLAN_ACL_ADDR_MACADDR,	BYTE6_T, MACFILTER_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	WLAN_ACL_ADDR_COMMENT,	STRING_T, MACFILTER_T, 0, 0)
#endif // #ifdef MIB_WLAN_MACFILTER_IMPORT

#ifdef MIB_WDS_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	macAddr, [6],	WLAN_WDS_MACADDR,	BYTE6_T, WDS_T, 0, 0)
MIBDEF(unsigned int,	fixedTxRate, ,	WLAN_WDS_FIXEDTXRATE,	DWORD_T, WDS_T, 0, 0)
MIBDEF(unsigned char,	comment, [COMMENT_LEN],	WLAN_WDS_COMMENT,	STRING_T, WDS_T, 0, 0)
#endif // #ifdef MIB_WDS_IMPORT
#ifdef CONFIG_RTL_VLAN_SUPPORT
#ifdef MIB_VLAN_IMPORT
MIBDEF(unsigned short,	vlanId, ,	VLAN_ID,	WORD_T, VLAN_T, 0, 0)
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
MIBDEF(unsigned char,  ForwardRule, ,   VLAN_FORWARD_RULE,    BYTE_T, VLAN_T, 0, 0)
#endif
#endif
#ifdef MIB_NETIFACE_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	netifEnable, ,	NETIF_ENABLE,	BYTE_T, NETIFACE_T, 0, 0)
MIBDEF(unsigned char,	netifId, ,	NETIF_ID,	BYTE_T, NETIFACE_T, 0, 0)
MIBDEF(unsigned char,	netifName,[COMMENT_LEN] ,	NETIF_NAME,	STRING_T, NETIFACE_T, 0, 0)
MIBDEF(unsigned short,	netifPvid, ,	NETIF_PREVID, WORD_T, NETIFACE_T, 0, 0)
MIBDEF(unsigned char,	netifDefPriority, , NETIF_PRIORITY,	BYTE_T, NETIFACE_T, 0, 0)	
MIBDEF(unsigned char,	netifDefCfi, , NETIF_CFI,	BYTE_T, NETIFACE_T, 0, 0)
#endif
#ifdef MIB_VLAN_NETIF_BIND_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
//MIBDEF(unsigned short,	bindId, ,	BIND_ID,	WORD_T, VLAN_NETIF_BIND_T, 0, 0)
MIBDEF(unsigned short,	vlanId, ,	BIND_VLAN_ID,	WORD_T, VLAN_NETIF_BIND_T, 0, 0)
MIBDEF(unsigned char,	netifId, ,	BIND_NETIF_ID,	BYTE_T, VLAN_NETIF_BIND_T, 0, 0)
MIBDEF(unsigned char,	tagged, ,	BIND_TAGGED,	BYTE_T, VLAN_NETIF_BIND_T, 0, 0)
#endif
#endif
#ifdef MIB_CONFIG_WLAN_SETTING_IMPORT
/* _ctype,	_cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl */
MIBDEF(unsigned char,	ssid, [MAX_SSID_LEN],	SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	channel, ,	CHANNEL,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanMacAddr, [6],	WLAN_MAC_ADDR,	BYTE6_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep, ,	WEP,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
//MIBDEF(unsigned char,	wep64Key, [WEP64_KEY_LEN],	WEP64_KEY,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key1, [WEP64_KEY_LEN],	WEP64_KEY1,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key2, [WEP64_KEY_LEN],	WEP64_KEY2,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key3, [WEP64_KEY_LEN],	WEP64_KEY3,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep64Key4, [WEP64_KEY_LEN],	WEP64_KEY4,	BYTE5_T, CONFIG_WLAN_SETTING_T, 0, 0)
//MIBDEF(unsigned char,	wep128Key, [WEP128_KEY_LEN],	WEP128_KEY,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key1, [WEP128_KEY_LEN],	WEP128_KEY1,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key2, [WEP128_KEY_LEN],	WEP128_KEY2,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key3, [WEP128_KEY_LEN],	WEP128_KEY3,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wep128Key4, [WEP128_KEY_LEN],	WEP128_KEY4,	BYTE13_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char,	wepDefaultKey, ,	WEP_DEFAULT_KEY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wepKeyType, ,	WEP_KEY_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned short,	fragThreshold, ,	FRAG_THRESHOLD,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rtsThreshold, ,	RTS_THRESHOLD,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	supportedRates, ,	SUPPORTED_RATES,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	basicRates, ,	BASIC_RATES,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	beaconInterval, ,	BEACON_INTERVAL,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	preambleType, ,	PREAMBLE_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	authType, ,	AUTH_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	ackTimeout, , ACK_TIMEOUT, BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char,	acEnabled, ,	MACAC_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acNum, ,	MACAC_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(MACFILTER_T,	acAddrArray, [MAX_WLAN_AC_NUM],	MACAC_ADDR,	WLAC_ARRAY_T, CONFIG_WLAN_SETTING_T, 0, wlan_acl_addr_tbl)

MIBDEF(unsigned char,	scheduleRuleEnabled, ,	SCHEDULE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	scheduleRuleNum, ,	SCHEDULE_TBL_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(SCHEDULE_T,	scheduleRuleArray, [MAX_SCHEDULE_NUM],	SCHEDULE_TBL,	SCHEDULE_ARRAY_T, CONFIG_WLAN_SETTING_T, 0, mib_schedule_tbl)

MIBDEF(unsigned char,	hiddenSSID, ,	HIDDEN_SSID,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanDisabled, ,	WLAN_DISABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	inactivityTime, ,	INACTIVITY_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rateAdaptiveEnabled, ,	RATE_ADAPTIVE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	dtimPeriod, ,	DTIM_PERIOD,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanMode, ,	MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	networkType, ,	NETWORK_TYPE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	iappDisabled, ,	IAPP_DISABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	protectionDisabled, ,	PROTECTION_DISABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	defaultSsid, [MAX_SSID_LEN],	DEFAULT_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	blockRelay, ,	BLOCK_RELAY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	maccloneEnabled, ,	MACCLONE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wlanBand, ,	BAND,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned int,	fixedTxRate, ,	FIX_RATE,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	turboMode, ,	TURBO_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	RFPowerScale, ,	RFPOWER_SCALE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

// WPA stuffs
MIBDEF(unsigned char,	encrypt, ,	ENCRYPT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	enableSuppNonWpa, ,	ENABLE_SUPP_NONWPA,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	suppNonWpa, ,	SUPP_NONWPA,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaAuth, ,	WPA_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaCipher, ,	WPA_CIPHER_SUITE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaPSK, [MAX_PSK_LEN+1],	WPA_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wpaGroupRekeyTime, ,	WPA_GROUP_REKEY_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsIpAddr, [4],	RS_IP,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rsPort, ,	RS_PORT,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsPassword, [MAX_RS_PASS_LEN],	RS_PASSWORD,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	enable1X, ,	ENABLE_1X,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpaPSKFormat, ,	PSK_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsEnabled, ,	ACCOUNT_RS_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsIpAddr, [4],	ACCOUNT_RS_IP,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	accountRsPort, ,	ACCOUNT_RS_PORT,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsPassword, [MAX_RS_PASS_LEN],	ACCOUNT_RS_PASSWORD,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsUpdateEnabled, ,	ACCOUNT_RS_UPDATE_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	accountRsUpdateDelay, ,	ACCOUNT_RS_UPDATE_DELAY,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	macAuthEnabled, ,	MAC_AUTH_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsMaxRetry, ,	RS_MAXRETRY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rsIntervalTime, ,	RS_INTERVAL_TIME,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	accountRsMaxRetry, ,	ACCOUNT_RS_MAXRETRY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	accountRsIntervalTime, ,	ACCOUNT_RS_INTERVAL_TIME,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpa2PreAuth, ,	WPA2_PRE_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wpa2Cipher, ,	WPA2_CIPHER_SUITE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

// WDS stuffs
MIBDEF(unsigned char,	wdsEnabled, ,	WDS_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsNum, ,	WDS_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(WDS_T,	wdsArray, [MAX_WDS_NUM],	WDS,	WDS_ARRAY_T, CONFIG_WLAN_SETTING_T, 0, wlan_wds_tbl)
MIBDEF(unsigned char,	wdsEncrypt, ,	WDS_ENCRYPT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsWepKeyFormat, ,	WDS_WEP_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsWepKey, [WEP128_KEY_LEN*2+1],	WDS_WEP_KEY,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsPskFormat, ,	WDS_PSK_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wdsPsk, [MAX_PSK_LEN+1],	WDS_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)

//=========add for MESH=========
MIBDEF(unsigned char,	meshEnabled, ,	MESH_ENABLE,	BYTE_T,	CONFIG_WLAN_SETTING_T,	0,	0)
MIBDEF(unsigned char,	meshRootEnabled, ,	MESH_ROOT_ENABLE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	meshID, [33],	MESH_ID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshMaxNumOfNeighbors, ,	MESH_MAX_NEIGHTBOR,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)

// for backbone security
MIBDEF(unsigned char,	meshEncrypt, ,	MESH_ENCRYPT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	meshWpaPSKFormat, ,	MESH_PSK_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	meshWpaPSK, [MAX_PSK_LEN+1],	MESH_WPA_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	meshWpaAuth, ,	MESH_WPA_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	meshWpa2Cipher, ,	MESH_WPA2_CIPHER_SUITE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

MIBDEF(unsigned char,	meshAclEnabled, ,	MESH_ACL_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	meshAclNum, ,	MESH_ACL_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
MIBDEF(MACFILTER_T,	meshAclAddrArray, [MAX_MESH_ACL_NUM],	MESH_ACL_ADDR,	MESH_ACL_ARRAY_T, CONFIG_WLAN_SETTING_T, 0, mib_mech_acl_tbl)
#endif
#ifdef 	_11s_TEST_MODE_	
MIBDEF(unsigned short,	meshTestParam1, ,	MESH_TEST_PARAM1,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam2, ,	MESH_TEST_PARAM2,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam3, ,	MESH_TEST_PARAM3,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam4, ,	MESH_TEST_PARAM4,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam5, ,	MESH_TEST_PARAM5,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam6, ,	MESH_TEST_PARAM6,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam7, ,	MESH_TEST_PARAM7,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam8, ,	MESH_TEST_PARAM8,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParam9, ,	MESH_TEST_PARAM9,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParama, ,	MESH_TEST_PARAMA,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamb, ,	MESH_TEST_PARAMB,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamc, ,	MESH_TEST_PARAMC	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamd, ,	MESH_TEST_PARAMD,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParame, ,	MESH_TEST_PARAME	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	meshTestParamf, ,	MESH_TEST_PARAMF,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	meshTestParamStr1, [16],	MESH_TEST_PARAMSTR1,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif // #ifdef 	_11s_TEST_MODE_	

// for WMM
MIBDEF(unsigned char,	wmmEnabled, ,	WMM_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

#ifdef WLAN_EASY_CONFIG
MIBDEF(unsigned char,	acfEnabled, ,	EASYCFG_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfMode, ,	EASYCFG_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfSSID, [MAX_SSID_LEN],	EASYCFG_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfKey, [MAX_ACF_KEY_LEN+1],	EASYCFG_KEY,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfDigest, [MAX_ACF_DIGEST_LEN+1],	EASYCFG_DIGEST,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfAlgReq, ,	EASYCFG_ALG_REQ,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfAlgSupp, ,	EASYCFG_ALG_SUPP,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfRole, ,	EASYCFG_ROLE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfScanSSID, [MAX_SSID_LEN],	EASYCFG_SCAN_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	acfWlanMode, ,	EASYCFG_WLAN_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif // #ifdef WLAN_EASY_CONFIG

//#ifdef WIFI_SIMPLE_CONFIG
MIBDEF(unsigned char,	wscDisable, ,	WSC_DISABLE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscMethod, ,	WSC_METHOD,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscConfigured, ,	WSC_CONFIGURED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscAuth, ,	WSC_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscEnc, ,	WSC_ENC,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscManualEnabled, ,	WSC_MANUAL_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscUpnpEnabled, ,	WSC_UPNP_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscRegistrarEnabled, ,	WSC_REGISTRAR_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscSsid, [MAX_SSID_LEN],	WSC_SSID,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscPsk, [MAX_PSK_LEN+1],	WSC_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscConfigByExtReg, ,	WSC_CONFIGBYEXTREG,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#ifdef HAVE_HS2_SUPPORT
MIBDEF(unsigned char,  hs2Enabled, ,   HS2_ENABLE, BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif
#ifdef KLD_ENABLED
MIBDEF(unsigned char,  wscPin2, [PIN_LEN+1],	WSC_PIN,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wscLocked, ,	WSC_LOCKED ,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	reAuthTime, ,	REAUTH_TIME ,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsIP1, [4],	RS_IP1 ,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rsPort1, ,	RS_PORT1 ,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsPassword1, [MAX_PSK_LEN],	RS_PASSWORD1,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	enableMacAuth1, ,	ENABLE_MAC_AUTH1 ,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsIP2, [4], RS_IP2 ,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rsPort2, ,	RS_PORT2 ,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rsPassword2, [MAX_PSK_LEN], RS_PASSWORD2,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	enableMacAuth2, ,	ENABLE_MAC_AUTH2 ,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wepKeyCheck, ,	WEPKEY_CHECK,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	rfPower, ,	RF_POWER,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	chanNum, ,	CHAN_NUM,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	schedule_select_idx, ,	SCHEDULE_SELECT_IDX,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	auto_scan, ,	AUTO_SCAN,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short,	rsReAuthTime, ,	REAUTH_TIME,	WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif
//#endif // #ifdef WIFI_SIMPLE_CONFIG

#ifdef HAVE_TWINKLE_RSSI
MIBDEF(unsigned int, ScGoodRssiThreshold, ,	SC_GOOD_RSSI_THRESHOLD, DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned int, ScNormalRssiThreshold, , SC_NORMAL_RSSI_THRESHOLD, DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned int, ScPoorRssiThreshold, , SC_POOR_RSSI_THRESHOLD, DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif

//for 11N
MIBDEF(unsigned char,	channelbonding, ,	CHANNEL_BONDING,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	controlsideband, ,	CONTROL_SIDEBAND,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	aggregation, ,	AGGREGATION,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	shortgiEnabled, ,	SHORT_GI,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	access, ,	ACCESS,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	priority, ,	PRIORITY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)


#ifdef CONFIG_RTL_8881A_SELECTIVE
MIBDEF(unsigned char,  channel_2,         ,	CHANNEL_2,          BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,  channelbonding_2,  ,	CHANNEL_BONDING_2,  BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,  controlsideband_2, ,	CONTROL_SIDEBAND_2, BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short, supportedRates_2,  ,	SUPPORTED_RATES_2,  WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned short, basicRates_2,      ,	BASIC_RATES_2,      WORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,  CoexistEnabled_2,  ,	COEXIST_ENABLED_2,  BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif

// for WAPI
#ifdef CONFIG_RTL_WAPI_SUPPORT
MIBDEF(unsigned char,	wapiPsk, [MAX_PSK_LEN+1],	WAPI_PSK,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiPskLen, ,	WAPI_PSKLEN,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiAuth, ,	WAPI_AUTH,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiPskFormat, ,	WAPI_PSK_FORMAT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiAsIpAddr, [4],	WAPI_ASIPADDR,	IA_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiMcastkey, ,	WAPI_MCASTREKEY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiMcastRekeyTime, ,	WAPI_MCAST_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiMcastRekeyPackets, ,	WAPI_MCAST_PACKETS,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiUcastkey, ,	WAPI_UCASTREKEY,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiUcastRekeyTime, ,	WAPI_UCAST_TIME,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned long,	wapiUcastRekeyPackets, ,	WAPI_UCAST_PACKETS,	DWORD_T, CONFIG_WLAN_SETTING_T, 0, 0)
//internal use
MIBDEF(unsigned char,	wapiSearchCertInfo, [32],	WAPI_SEARCHINFO,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiSearchIndex, ,	WAPI_SEARCHINDEX,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	wapiCAInit, ,	WAPI_CA_INIT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif // #if CONFIG_RTL_WAPI_SUPPORT

MIBDEF(unsigned char,	STBCEnabled, ,	STBC_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	LDPCEnabled, ,	LDPC_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	CoexistEnabled, ,	COEXIST_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	phyBandSelect,	, PHY_BAND_SELECT,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0) //bit1:2G bit2:5G
MIBDEF(unsigned char,	macPhyMode,	, MAC_PHY_MODE,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0) //bit0:SmSphy. bit1:DmSphy. bit2:DmDphy.
MIBDEF(unsigned char,	TxBeamforming, ,	TX_BEAMFORMING,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	CountryStr, [4],	COUNTRY_STRING,	STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	staNum, ,	STA_NUM, BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)

#ifdef	CONFIG_RTL_SIMPLE_CONFIG
MIBDEF(unsigned char,	ScEnabled, ,	SC_ENABLED,	BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	ScPinEnabled, , SC_PIN_ENABLED, BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	ScSaveProfile, ,	SC_SAVE_PROFILE,	 BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	ScSyncProfile, ,	SC_SYNC_PROFILE,	 BYTE_T, CONFIG_WLAN_SETTING_T, 0, 0)
MIBDEF(unsigned char,	ScPasswd, [MAX_PSK_LEN+1], SC_PASSWD, STRING_T, CONFIG_WLAN_SETTING_T, 0, 0)
#endif

#endif // #ifdef MIB_CONFIG_WLAN_SETTING_IMPORT

#if defined(HAVE_TR069)
#if defined(MIB_CWMP_WLANCONF_IMPORT)
MIBDEF(unsigned char,   InstanceNum, ,  CWMP_WLANCONF_INSTANCENUM,      BYTE_T, CWMP_WLANCONF_T, 0, 0)
MIBDEF(unsigned char,   IsConfigured, ,CWMP_WLANCONF_ISCONFIGURED,      BYTE_T, CWMP_WLANCONF_T, 0, 0)
MIBDEF(unsigned char,   RootIdx, ,      CWMP_WLANCONF_ROOT_IDX, BYTE_T, CWMP_WLANCONF_T, 0, 0)
MIBDEF(unsigned char,   VWlanIdx, ,     CWMP_WLANCONF_VWLAN_IDX,        BYTE_T, CWMP_WLANCONF_T, 0, 0)
MIBDEF(unsigned char,   RfBandAvailable, ,CWMP_WLANCONF_RFBAND, BYTE_T, CWMP_WLANCONF_T, 0, 0)
#endif
#ifdef ROUTE_SUPPORT
#if defined(MIB_STATICROUTE_IMPORT)
MIBDEF(unsigned char,   Enable, ,       STATICROUTE_ENABLE,     BYTE_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,   Type, , STATICROUTE_TYPE,       BYTE_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,   SourceIP, [4],  STATICROUTE_SRCADDR,    IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned char,   SourceMask, [4],        STATICROUTE_SRCNETMASK, IA_T, STATICROUTE_T, 0, 0)
MIBDEF(unsigned int,    ifIndex, ,      STATICROUTE_IFACEINDEX, DWORD_T, STATICROUTE_T, 0, 0)
//MIBDEF(unsigned int,    InstanceNum, ,  STATICROUTE_INSTANCENUM,        DWORD_T, STATICROUTE_T, 0, 0)
#endif 
#endif
#endif

