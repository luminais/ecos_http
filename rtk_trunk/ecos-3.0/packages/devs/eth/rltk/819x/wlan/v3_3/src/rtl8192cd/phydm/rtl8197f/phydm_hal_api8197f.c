/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8197F_SUPPORT == 1)  

/* ======================================================================== */
/* These following functions can be used for PHY DM only*/

VOID
phydm_CcaParByBw_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	u4Byte		reg82c;
	
	return;	/*will set this later by YuChen 20151023*/
	reg82c = ODM_GetBBReg(pDM_Odm, 0x82c, bMaskDWord);
	
	if (bandwidth == ODM_BW20M) {
		/* 82c[15:12] = 4 */
		/* 82c[27:24] = 6 */
		
		reg82c &= (~(0x0f00f000));
		reg82c |= ((0x4) << 12);
		reg82c |= ((0x6) << 24);
	} else if (bandwidth == ODM_BW40M) {
		/* 82c[19:16] = 9 */
		/* 82c[27:24] = 6 */
	
		reg82c &= (~(0x0f0f0000));
		reg82c |= ((0x9) << 16);
		reg82c |= ((0x6) << 24);
	}

	ODM_SetBBReg(pDM_Odm, 0x82c, bMaskDWord, reg82c);
}

VOID
phydm_CcaParByRxPath_8197f(
	IN	PDM_ODM_T				pDM_Odm
	)
{
	if ((pDM_Odm->RXAntStatus == ODM_RF_A) || (pDM_Odm->RXAntStatus == ODM_RF_B)) {
		/* Preamble parameters for ch-D */
		/*ODM_SetBBReg(pDM_Odm, 0x834, 0x70000, 0x4);*/

		/* 838[7:4] = 8 */
		/* 838[11:8] = 7 */
		/* 838[15:12] = 6 */
		/* 838[19:16] = 7 */
		/* 838[23:20] = 7 */
		/* 838[27:24] = 7 */
		ODM_SetBBReg(pDM_Odm, 0x838, 0x0ffffff0, 0x777678);
	} else {
		/* Preamble parameters for ch-D */
		/*ODM_SetBBReg(pDM_Odm, 0x834, 0x70000, 0x6);*/

		/* 838[7:4] = 3 */
		/* 838[11:8] = 3 */
		/* 838[15:12] = 6 */
		/* 838[19:16] = 6 */
		/* 838[23:20] = 7 */
		/* 838[27:24] = 7 */
		ODM_SetBBReg(pDM_Odm, 0x838, 0x0ffffff0, 0x776633);
	}

}

VOID
phydm_RxDfirParByBw_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	ODM_SetBBReg(pDM_Odm, ODM_REG_TAP_UPD_97F, (BIT21|BIT20), 0x2);
	ODM_SetBBReg(pDM_Odm, ODM_REG_DOWNSAM_FACTOR_11N, (BIT29|BIT28), 0x2);

	if (bandwidth == ODM_BW40M) {
		/* RX DFIR for BW40 */
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, BIT8, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, bMaskByte0, 0x3);
	} else {
		/* RX DFIR for BW20, BW10 and BW5*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, BIT8, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, bMaskByte0, 0xa3);
}
}
/* ======================================================================== */

/* ======================================================================== */
/* These following functions can be used by driver*/

u4Byte
config_phydm_read_rf_reg_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask
	)
{
	u4Byte	Readback_Value, Direct_Addr;
	u4Byte	offset_readRF[2] = {0x2800, 0x2c00};
	u4Byte	power_RF[2] = {0x1c, 0x78};

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8197f(): unsupported path (%d)\n", RFPath));
		return INVALID_RF_DATA;
	}

	/*  Error handling. Check if RF power is enable or not */
	/*  0xffffffff means RF power is disable */
	if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8197f(): Read fail, RF is disabled\n"));
		return INVALID_RF_DATA;
	}

	/* Calculate offset */
	RegAddr &= 0xff;
	Direct_Addr = offset_readRF[RFPath] + (RegAddr << 2);

	/* RF register only has 20bits */
	BitMask &= bRFRegOffsetMask;

	/* Read RF register directly */
	Readback_Value = ODM_GetBBReg(pDM_Odm, Direct_Addr, BitMask);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8197f(): RF-%d 0x%x = 0x%x, bit mask = 0x%x\n", 
		RFPath, RegAddr, Readback_Value, BitMask));
	return Readback_Value;
}

BOOLEAN
config_phydm_write_rf_reg_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask,
	IN	u4Byte					Data
	)
{
	u4Byte	DataAndAddr = 0, Data_original = 0;
	u4Byte	offset_writeRF[2] = {0x840, 0x844};
	u4Byte	power_RF[2] = {0x1c, 0x78};
	u1Byte	BitShift;

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): unsupported path (%d)\n", RFPath));
		return FALSE;
	}

	/* Read RF register content first */
	RegAddr &= 0xff;
	BitMask = BitMask & bRFRegOffsetMask;

	if (BitMask != bRFRegOffsetMask) {
		Data_original = config_phydm_read_rf_reg_8197f(pDM_Odm, RFPath, RegAddr, bRFRegOffsetMask);

		/* Error handling. RF is disabled */
		if (config_phydm_read_rf_check_8197f(Data_original) == FALSE) {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): Write fail, RF is disable\n"));
			return FALSE;
		}

		/* check bit mask */
		if (BitMask != 0xfffff) {
			for (BitShift = 0; BitShift <= 19; BitShift++) {
				if (((BitMask >> BitShift) & 0x1) == 1)
					break;
			}
			Data = ((Data_original) & (~BitMask)) | (Data << BitShift);
		}
	} 
	else if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): Write fail, RF is disabled\n"));
		return FALSE;
	}

	/* Put write addr in [27:20]  and write data in [19:00] */
	DataAndAddr = ((RegAddr<<20) | (Data&0x000fffff)) & 0x0fffffff;	

	/* Write Operation */
	ODM_SetBBReg(pDM_Odm, offset_writeRF[RFPath], bMaskDWord, DataAndAddr);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): RF-%d 0x%x = 0x%x (original: 0x%x), bit mask = 0x%x\n", 
		RFPath, RegAddr, Data, Data_original, BitMask));
	return TRUE;
}

BOOLEAN
config_phydm_write_txagc_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	)
{
	/* Input need to be HW rate index, not driver rate index!!!! */

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > ODM_RATEMCS15)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8197f(): unsupported path (%d)\n", Path));
		return FALSE;
	}

	if (Path == ODM_RF_PATH_A) {
		switch (HwRate) {
		case ODM_RATE1M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_CCK1_Mcs32, bMaskByte1, PowerIndex); break;
		case ODM_RATE2M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte1, PowerIndex); break;
		case ODM_RATE5_5M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte2, PowerIndex); break;
		case ODM_RATE11M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte3, PowerIndex); break;

		case ODM_RATE6M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte0, PowerIndex); break;
		case ODM_RATE9M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte1, PowerIndex); break;
		case ODM_RATE12M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte2, PowerIndex); break;
		case ODM_RATE18M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte3, PowerIndex); break;
		case ODM_RATE24M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte0, PowerIndex); break;
		case ODM_RATE36M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte1, PowerIndex); break;
		case ODM_RATE48M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte2, PowerIndex); break;
		case ODM_RATE54M:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte3, PowerIndex); break;

		case ODM_RATEMCS0:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS1:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS2:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS3:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte3, PowerIndex); break;
		case ODM_RATEMCS4:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS5:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS6:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS7:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte3, PowerIndex); break;

		case ODM_RATEMCS8:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS9:		ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS10:	ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS11:	ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte3, PowerIndex); break;
		case ODM_RATEMCS12:	ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS13:	ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS14:	ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS15:	ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte3, PowerIndex); break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
		}
	} else if (Path == ODM_RF_PATH_B) {
    		switch (HwRate) {
		case ODM_RATE1M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, bMaskByte1, PowerIndex); break;
		case ODM_RATE2M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, bMaskByte2, PowerIndex); break;
		case ODM_RATE5_5M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, bMaskByte3, PowerIndex); break;
		case ODM_RATE11M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte0, PowerIndex); break;

		case ODM_RATE6M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte0, PowerIndex); break;
		case ODM_RATE9M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte1, PowerIndex); break;
		case ODM_RATE12M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte2, PowerIndex); break;
		case ODM_RATE18M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte3, PowerIndex); break;
		case ODM_RATE24M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte0, PowerIndex); break;
		case ODM_RATE36M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte1, PowerIndex); break;
		case ODM_RATE48M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte2, PowerIndex); break;
		case ODM_RATE54M:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte3, PowerIndex); break;

		case ODM_RATEMCS0:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS1:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS2:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS3:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte3, PowerIndex); break;
		case ODM_RATEMCS4:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS5:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS6:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS7:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte3, PowerIndex); break;

		case ODM_RATEMCS8:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS9:		ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS10:	ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS11:	ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte3, PowerIndex); break;
		case ODM_RATEMCS12:	ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte0, PowerIndex); break;
		case ODM_RATEMCS13:	ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte1, PowerIndex); break;
		case ODM_RATEMCS14:	ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte2, PowerIndex); break;
		case ODM_RATEMCS15:	ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte3, PowerIndex); break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
		}
	} else
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid RF path!!\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8197f(): Path-%d Rate index 0x%x = 0x%x\n", 
		Path, HwRate, PowerIndex));
	return TRUE;
}

u1Byte
config_phydm_read_txagc_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		Path,
	IN	u1Byte					HwRate
	)
{
	u1Byte	readBack_data;

	/* Input need to be HW rate index, not driver rate index!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > ODM_RATEMCS15)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8197f(): unsupported path (%d)\n", Path));
		return INVALID_TXAGC_DATA;
	}

	if (Path == ODM_RF_PATH_A) {
		switch (HwRate) {
		case ODM_RATE1M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_CCK1_Mcs32, bMaskByte1); break;
		case ODM_RATE2M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte1); break;
		case ODM_RATE5_5M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte2); break;
		case ODM_RATE11M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte3); break;

		case ODM_RATE6M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte0); break;
		case ODM_RATE9M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte1); break;
		case ODM_RATE12M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte2); break;
		case ODM_RATE18M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, bMaskByte3); break;
		case ODM_RATE24M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte0); break;
		case ODM_RATE36M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte1); break;
		case ODM_RATE48M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte2); break;
		case ODM_RATE54M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, bMaskByte3); break;

		case ODM_RATEMCS0:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte0); break;
		case ODM_RATEMCS1:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte1); break;
		case ODM_RATEMCS2:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte2); break;
		case ODM_RATEMCS3:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, bMaskByte3); break;
		case ODM_RATEMCS4:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte0); break;
		case ODM_RATEMCS5:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte1); break;
		case ODM_RATEMCS6:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte2); break;
		case ODM_RATEMCS7:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, bMaskByte3); break;

		case ODM_RATEMCS8:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte0); break;
		case ODM_RATEMCS9:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte1); break;
		case ODM_RATEMCS10:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte2); break;
		case ODM_RATEMCS11:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, bMaskByte3); break;
		case ODM_RATEMCS12:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte0); break;
		case ODM_RATEMCS13:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte1); break;
		case ODM_RATEMCS14:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte2); break;
		case ODM_RATEMCS15:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, bMaskByte3); break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
	}
	} else if (Path == ODM_RF_PATH_B) {
    		switch (HwRate) {
		case ODM_RATE1M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, bMaskByte1); break;
		case ODM_RATE2M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, bMaskByte2); break;
		case ODM_RATE5_5M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, bMaskByte3); break;
		case ODM_RATE11M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte0); break;

		case ODM_RATE6M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte0); break;
		case ODM_RATE9M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte1); break;
		case ODM_RATE12M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte2); break;
		case ODM_RATE18M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, bMaskByte3); break;
		case ODM_RATE24M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte0); break;
		case ODM_RATE36M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte1); break;
		case ODM_RATE48M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte2); break;
		case ODM_RATE54M:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, bMaskByte3); break;

		case ODM_RATEMCS0:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte0); break;
		case ODM_RATEMCS1:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte1); break;
		case ODM_RATEMCS2:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte2); break;
		case ODM_RATEMCS3:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, bMaskByte3); break;
		case ODM_RATEMCS4:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte0); break;
		case ODM_RATEMCS5:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte1); break;
		case ODM_RATEMCS6:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte2); break;
		case ODM_RATEMCS7:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, bMaskByte3); break;

		case ODM_RATEMCS8:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte0); break;
		case ODM_RATEMCS9:		readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte1); break;
		case ODM_RATEMCS10:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte2); break;
		case ODM_RATEMCS11:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, bMaskByte3); break;
		case ODM_RATEMCS12:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte0); break;
		case ODM_RATEMCS13:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte1); break;
		case ODM_RATEMCS14:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte2); break;
		case ODM_RATEMCS15:	readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, bMaskByte3); break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
	}
	} else
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid RF path!!\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8197f(): Path-%d rate index 0x%x = 0x%x\n", Path, HwRate, readBack_data));
	return readBack_data;
}

BOOLEAN
config_phydm_switch_channel_8197f(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f()====================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8197f(rf_reg18);

	/* Switch band and channel */
	if (central_ch <= 14) {
		/* 2.4G */

		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18 & (~(BIT18|BIT17|bMaskByte0)));
		rf_reg18 = (rf_reg18|central_ch);

	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): Fail to switch band (ch: %d)\n", central_ch));
		return FALSE;
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);
	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_B, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): Fail to switch channel (ch: %d), because writing RF register is fail\n", central_ch));
		return FALSE;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): Success to switch channel (ch: %d)\n", central_ch));
	return TRUE;
}

BOOLEAN
config_phydm_switch_bandwidth_8197f(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f()===================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((bandwidth >= ODM_BW_MAX) || ((bandwidth == ODM_BW40M) && (primary_ch_idx > 2)) || ((bandwidth == ODM_BW80M) && (primary_ch_idx > 4))) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8197f(rf_reg18);

	/* Switch bandwidth */
	switch (bandwidth) {
	case ODM_BW20M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 0 for 20M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x0);

		/* ADC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, ODM_REG_RXCK_RFMOD, (BIT18|BIT17|BIT16), 0x4);

		/* DAC clock = 160M clock for BW20 = 3'b101*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x5);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 | BIT11 | BIT10);
		break;
	}
	case ODM_BW40M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 1 for 40M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x1);

		/* ADC clock = 160M clock for BW40 no need to setting, it will be setting in PHY_REG */

		/* DAC clock = 160M clock for BW20 = 3'b101*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x5);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* CCK primary channel */
		if (primary_ch_idx == 1)
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT4, primary_ch_idx);
		else
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT4, 0);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 & (~(BIT11|BIT10)));
		rf_reg18 = (rf_reg18|BIT11);		
		break;
	}
	case ODM_BW5M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 0 for 5M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x0);

		/* ADC clock = 40M clock for BW5 = 3'b010*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_RXCK_RFMOD, (BIT18|BIT17|BIT16), 0x2);

		/* DAC clock = 20M clock for BW20 = 3'b110*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x6);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	case ODM_BW10M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 0 for 10M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x2);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x0);

		/* ADC clock = 80M clock for BW5 = 3'b011*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_RXCK_RFMOD, (BIT18|BIT17|BIT16), 0x3);

		/* DAC clock = 160M clock for BW20 = 3'b110*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x4);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	}

	/* Write RF register */
	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_B, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Fail to switch bandwidth (bw: %d, primary ch: %d), because writing RF register is fail\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	/* Modify RX DFIR parameters */
	phydm_RxDfirParByBw_8197f(pDM_Odm, bandwidth);

	/* Modify CCA parameters */
	phydm_CcaParByBw_8197f(pDM_Odm, bandwidth);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Success to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_bw_8197f(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u1Byte			eRFPath = 0;
	u4Byte			RFValToWR , RFTmpVal, BitShift, BitMask;

	/* Switch band */
	/*97F no need*/

	/* Switch channel */
	if (config_phydm_switch_channel_8197f(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch bandwidth */
	if (config_phydm_switch_bandwidth_8197f(pDM_Odm, primary_ch_idx, bandwidth) == FALSE)
		return FALSE;

	return TRUE;
}

BOOLEAN
config_phydm_trx_mode_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_PATH_E			TxPath,
	IN	ODM_RF_PATH_E			RxPath,
	IN	BOOLEAN					bTx2Path
	)
{
	BOOLEAN		rf_reg_status = TRUE;
	u1Byte		IGI;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f()=====================>\n"));	

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	if ((TxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): Wrong TX setting (TX: 0x%x)\n", TxPath));
		return FALSE;
	}

	if ((RxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): Wrong RX setting (RX: 0x%x)\n", RxPath));
		return FALSE;
	}

	/* RF mode of path-A and path-B */
	/* OFDM Tx and Rx path setting */
	if (TxPath == (ODM_RF_A|ODM_RF_B))
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x81121333);
	else if (TxPath & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x81121311);
	else
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x81121322);

	if (RxPath == (ODM_RF_A|ODM_RF_B)) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11N, bMaskByte0, 0x33);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_ANT_11N, BIT3|BIT2|BIT1|BIT0, 0x3);
	} else if (RxPath & ODM_RF_A) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11N, bMaskByte0, 0x11);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_ANT_11N, BIT3|BIT2|BIT1|BIT0, 0x1);
	} else {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11N, bMaskByte0, 0x22);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_ANT_11N, BIT3|BIT2|BIT1|BIT0, 0x2);
	}
		
    /* CCK Tx and Rx path setting*/
	if (TxPath & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, bMaskH4Bits, 0x8);
	else
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, bMaskH4Bits, 0x4);

	if (RxPath == (ODM_RF_A|ODM_RF_B)) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, BIT27|BIT26|BIT25|BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT18, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT22, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT15, 0x0);
	} else if (RxPath & ODM_RF_A) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, BIT27|BIT26|BIT25|BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT18, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT22, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT15, 0x0);
	} else {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, BIT27|BIT26|BIT25|BIT24, 0x4);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT18, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT22, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT15, 0x0);
	}

	if(bTx2Path) {
		/*OFDM tx setting*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x81333333);
		/*CCK tx setting*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, BIT27|BIT26|BIT25|BIT24, 0xc);
	}

	/* Update TXRX antenna status for PHYDM */
	pDM_Odm->TXAntStatus =  (TxPath & 0x3);
	pDM_Odm->RXAntStatus =  (RxPath & 0x3);

	/* Modify CCA parameters */
	/*phydm_CcaParByRxPath_8197f(pDM_Odm);*/

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): Success to set TRx mode setting (TX: 0x%x, RX: 0x%x)\n", TxPath, RxPath));
	return TRUE;
}

BOOLEAN
config_phydm_parameter_8197f_init(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_PARAMETER_INIT_E	type
	)
{
	if (type == ODM_PRE_SETTING) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT25|BIT24), 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_8197f_init(): Pre setting: disable OFDM and CCK block\n"));
	} else if (type == ODM_POST_SETTING) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT25|BIT24), 0x3);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_8197f_init(): Post setting: enable OFDM and CCK block\n"));
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_8197f_init(): Wrong type!!\n"));
		return FALSE;
	}

	return TRUE;
}

/* ======================================================================== */
#endif	/* RTL8197F_SUPPORT == 1 */

