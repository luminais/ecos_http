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

#if (RTL8822B_SUPPORT == 1)


/*---------------------------Define Local Constant---------------------------*/


/*---------------------------Define Local Constant---------------------------*/


#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
void DoIQK_8822B(
	PVOID		pDM_VOID,
	u1Byte 		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte 		Threshold
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

	PADAPTER 		Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	ODM_ResetIQKResult(pDM_Odm);		

	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK= ThermalValue;
    
	PHY_IQCalibrate_8822B(pDM_Odm, FALSE);
	
}
#else
/*Originally pConfig->DoIQK is hooked PHY_IQCalibrate_8822B, but DoIQK_8822B and PHY_IQCalibrate_8822B have different arguments*/
void DoIQK_8822B(
	PVOID		pDM_VOID,
	u1Byte	DeltaThermalIndex,
	u1Byte	ThermalValue,	
	u1Byte	Threshold
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	BOOLEAN		bReCovery = (BOOLEAN) DeltaThermalIndex;

	PHY_IQCalibrate_8822B(pDM_Odm, bReCovery);
}
#endif
//1 7.	IQK

VOID 
_IQK_BackupMacBB_8822B(
	IN PDM_ODM_T	pDM_Odm,
	IN pu4Byte		MAC_backup,
	IN pu4Byte		BB_backup,
	IN pu4Byte		Backup_MAC_REG,
	IN pu4Byte		Backup_BB_REG
	)
{
	u4Byte i;
	 //save MACBB default value
	for (i = 0; i < MAC_REG_NUM_8822B; i++)
		MAC_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_MAC_REG[i]);

	for (i = 0; i < BB_REG_NUM_8822B; i++)
		BB_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_BB_REG[i]);
	
//	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]BackupMacBB Success!!!!\n"));
}


VOID
_IQK_BackupRF_8822B(
	IN PDM_ODM_T	pDM_Odm,
	IN u4Byte		RF_backup[][2],
	IN pu4Byte		Backup_RF_REG
	)	
{
	u4Byte i;
	//Save RF Parameters
    for (i = 0; i < RF_REG_NUM_8822B; i++)
	{
        	RF_backup[i][ODM_RF_PATH_A] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG[i], bRFRegOffsetMask);
		RF_backup[i][ODM_RF_PATH_B] = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, Backup_RF_REG[i], bRFRegOffsetMask);
    	}
//	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]BackupRF Success!!!!\n"));
}


VOID
_IQK_AFESetting_8822B(
	IN PDM_ODM_T	pDM_Odm,
	IN BOOLEAN		Do_IQK
	)
{
	if(Do_IQK)
	{
		// IQK AFE Setting RX_WAIT_CCA mode 
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x50000000); 
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x70070040);
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x70170040); //for HW trigger issue
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x50000000); 
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x70070040);
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x70170040); //for HW trigger issue
		 
		//AFE setting
		ODM_Write4Byte(pDM_Odm, 0xc58, 0xd8000402);
		ODM_Write4Byte(pDM_Odm, 0xc5c, 0xd1000120);
		ODM_Write4Byte(pDM_Odm, 0xc6c, 0x00000a15);
		ODM_Write4Byte(pDM_Odm, 0xe58, 0xd8000402);
		ODM_Write4Byte(pDM_Odm, 0xe5c, 0xd1000120);
		ODM_Write4Byte(pDM_Odm, 0xe6c, 0x00000a15);

//		ODM_SetBBReg(pDM_Odm, 0x90c, BIT(13), 0x1);
		 
//		ODM_SetBBReg(pDM_Odm, 0x764, BIT(10)|BIT(9), 0x3);
//		ODM_SetBBReg(pDM_Odm, 0x764, BIT(10)|BIT(9), 0x0);

//		ODM_SetBBReg(pDM_Odm, 0x804, BIT(2), 0x1);
//		ODM_SetBBReg(pDM_Odm, 0x804, BIT(2), 0x0);
		
//		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]AFE IQK mode Success!!!!\n"));
	}
	else
	{
		// IQK AFE Setting RX_WAIT_CCA mode 
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x50000000); 
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x7003c040);
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x70144040); //for HW trigger issue
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x50000000); 
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x7003c040);
		ODM_Write4Byte(pDM_Odm, 0xe60, 0x70144040); //for HW trigger issue

		//AFE setting
		ODM_Write4Byte(pDM_Odm, 0xc58, 0xd8020402);
		ODM_Write4Byte(pDM_Odm, 0xc5c, 0xde000120);
		ODM_Write4Byte(pDM_Odm, 0xc6c, 0x0000112a);
		ODM_Write4Byte(pDM_Odm, 0xe58, 0xd8020402);
		ODM_Write4Byte(pDM_Odm, 0xe5c, 0xde000120);
		ODM_Write4Byte(pDM_Odm, 0xe6c, 0x0000112a);
	


//		ODM_SetBBReg(pDM_Odm, 0x90c, BIT(13), 0x1);
	
//		ODM_SetBBReg(pDM_Odm, 0x764, BIT(10)|BIT(9), 0x3);
//		ODM_SetBBReg(pDM_Odm, 0x764, BIT(10)|BIT(9), 0x0);

//		ODM_SetBBReg(pDM_Odm, 0x804, BIT(2), 0x1);
//		ODM_SetBBReg(pDM_Odm, 0x804, BIT(2), 0x0);
		
//		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]AFE Normal mode Success!!!!\n"));
	}
}


VOID
_IQK_RestoreMacBB_8822B(
	IN PDM_ODM_T		pDM_Odm,
	IN pu4Byte		MAC_backup,
	IN pu4Byte		BB_backup,
	IN pu4Byte		Backup_MAC_REG, 
	IN pu4Byte		Backup_BB_REG
	)	
{
	u4Byte i;
	//Reload MacBB Parameters 
   	for (i = 0; i < MAC_REG_NUM_8822B; i++)
        	ODM_Write4Byte(pDM_Odm, Backup_MAC_REG[i], MAC_backup[i]);

	for (i = 0; i < BB_REG_NUM_8822B; i++)
        	ODM_Write4Byte(pDM_Odm, Backup_BB_REG[i], BB_backup[i]);

//   	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]RestoreMacBB Success!!!!\n"));
}

VOID
_IQK_RestoreRF_8822B(
	IN PDM_ODM_T			pDM_Odm,
	IN pu4Byte			Backup_RF_REG,
	IN u4Byte 			RF_backup[][2]
	)
{	
	u4Byte i;

	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x0);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, bRFRegOffsetMask, 0x0);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, 0x9);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, bRFRegOffsetMask, 0x9);

    for (i = 0; i < RF_REG_NUM_8822B; i++)
	{
        	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i][ODM_RF_PATH_A]);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i][ODM_RF_PATH_B]);
    	}
//	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]RestoreRF Success!!!!\n"));
	
}

VOID 
PHY_ResetIQKResult_8822B(
	IN	PDM_ODM_T	pDM_Odm
)
{
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000000);
	ODM_Write4Byte(pDM_Odm, 0x1b38, 0x20000000);
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000002);
	ODM_Write4Byte(pDM_Odm, 0x1b38, 0x20000000);
	ODM_Write4Byte(pDM_Odm, 0xc10, 0x100);
	ODM_Write4Byte(pDM_Odm, 0xe10, 0x100);
}

VOID 
_IQK_ResetNCTL_8822B(
	IN PDM_ODM_T	pDM_Odm
)
{ 
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000000);
	ODM_Write4Byte(pDM_Odm, 0x1b80, 0x00000006);
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000000);
	ODM_Write4Byte(pDM_Odm, 0x1b80, 0x00000002);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]ResetNCTL Success!!!!\n"));
}

VOID 
_IQK_RFESetting_8822B(
	IN PDM_ODM_T	pDM_Odm,
	IN BOOLEAN		extPAon
	)
{
	if(extPAon)
	{
		//RFE setting
		ODM_Write4Byte(pDM_Odm, 0xcb0, 0x77777777);
		ODM_Write4Byte(pDM_Odm, 0xcb4, 0x00007777);
		ODM_Write4Byte(pDM_Odm, 0xcbc, 0x0000083B);
		ODM_Write4Byte(pDM_Odm, 0xeb0, 0x77777777);
		ODM_Write4Byte(pDM_Odm, 0xeb4, 0x00007777);
		ODM_Write4Byte(pDM_Odm, 0xebc, 0x0000083B);
		//	ODM_Write4Byte(pDM_Odm, 0x1990, 0x00000c30);	
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]external PA on!!!!\n"));
	}
	else
	{
		//RFE setting
		ODM_Write4Byte(pDM_Odm, 0xcb0, 0x77171117);
		ODM_Write4Byte(pDM_Odm, 0xcb4, 0x00001177);
		ODM_Write4Byte(pDM_Odm, 0xcbc, 0x00000404);
		ODM_Write4Byte(pDM_Odm, 0xeb0, 0x77171117);
		ODM_Write4Byte(pDM_Odm, 0xeb4, 0x00001177);
		ODM_Write4Byte(pDM_Odm, 0xebc, 0x00000404);
		//	ODM_Write4Byte(pDM_Odm, 0x1990, 0x00000c30);		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]external PA off!!!!\n"));
	}
	
}



VOID 
_IQK_ConfigureMACBB_8822B(
	IN PDM_ODM_T		pDM_Odm
	)
{
	// ========MACBB register setting========
	ODM_Write1Byte(pDM_Odm, 0x522, 0x7f);
	ODM_SetBBReg(pDM_Odm, 0x550, BIT(11)|BIT(3), 0x0);
	ODM_Write1Byte(pDM_Odm, 0x808, 0x00);				//		RX ante off
//	ODM_SetBBReg(pDM_Odm, 0x838, 0xf, 0xe);				//		CCA off
//	ODM_SetBBReg(pDM_Odm, 0xa14, BIT(9)|BIT(8), 0x3);	//  		CCK RX Path off

	ODM_SetBBReg(pDM_Odm, 0x90c, BIT(15), 0x1);			//0x90c[15]=1: dac_buf reset selection
	ODM_SetBBReg(pDM_Odm, 0x9a4, BIT(31), 0x0);         //0x9a4[31]=0: Select da clock
	//0xc94[0]=1, 0xe94[0]=1: 讓tx從iqk打出來
	ODM_SetBBReg(pDM_Odm, 0xc94, BIT(0), 0x1);
	ODM_SetBBReg(pDM_Odm, 0xe94, BIT(0), 0x1); 

	// 3-wire off
	ODM_Write4Byte(pDM_Odm, 0xc00, 0x00000004);
	ODM_Write4Byte(pDM_Odm, 0xe00, 0x00000004);

	//by YN
//	ODM_SetBBReg(pDM_Odm, 0xcbc, 0xf, 0x0);
}




VOID
_LOK_One_Shot_8822B(
	IN	PVOID		pDM_VOID,
	u1Byte			Path
)
{	
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte		delay_count = 0, ii;
	BOOLEAN		LOK_notready = FALSE;
	u4Byte		LOK_temp1 = 0, LOK_temp2 = 0, LOK_temp3 = 0;
	u4Byte		IQK_CMD;

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
			("[IQK]==========S%d LOK ==========\n", Path));
		
//		ODM_SetBBReg(pDM_Odm, 0x9a4, BIT(21)|BIT(20), Path);	 // ADC Clock source

		//trigger LOK
	IQK_CMD = 0xf8000008|(1<<(4+Path))|(Path<<1);
			
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,("[IQK]LOK_Trigger = 0x%x\n", IQK_CMD));

	ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD);		//LOK: CMD ID = 0	{0xf8000018, 0xf8000028}
	ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD+1);		// LOK: CMD ID = 0  {0xf8000019, 0xf8000029}

		ODM_delay_ms(LOK_delay_8822B);

		delay_count = 0;
		LOK_notready = TRUE;
		
	while(LOK_notready)
	{
			LOK_notready = (BOOLEAN) ODM_GetBBReg(pDM_Odm, 0x1b00, BIT(0));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]S%d ==> 0x1b00 = 0x%x\n", Path, ODM_Read4Byte(pDM_Odm, 0x1b00)));
			ODM_delay_ms(1);
			delay_count++;

		if(delay_count >= 300)
		{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
					("[IQK]S%d LOK timeout!!!\n", Path));
//				_IQK_ResetNCTL_8822B(pDM_Odm);
				break;
			}
		}

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
			("[IQK]S%d ==> delay_count = 0x%d\n", Path, delay_count));
		
	if(!LOK_notready)
	{
//			ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000000|(Path<<1));
//			ODM_Write4Byte(pDM_Odm, 0x1bd4, 0x001f0001);
//			LOK_temp1 = ODM_GetBBReg(pDM_Odm, 0x1bfc, bMaskDWord);
//			LOK_temp1 = ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)Path, 0xdf, bRFRegOffsetMask);			
			LOK_temp2 = ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)Path, 0x8, bRFRegOffsetMask);
			LOK_temp3 = ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)Path, 0x58, bRFRegOffsetMask);

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]0x8 = 0x%x, 0x58 = 0x%x\n", LOK_temp2, LOK_temp3));
		}
	else
	{
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]==>S%d LOK Fail!!!\n", Path));
//			ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)Path, 0x8, bRFRegOffsetMask, 0x08400);
		}
		pIQK_info->LOK_fail[Path] = LOK_notready;
}

VOID
_IQK_One_Shot_8822B(
	IN	PVOID		pDM_VOID,
	u1Byte		Path,
	u1Byte 		idx
)
{	
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte		delay_count = 0, cal_retry = 0;
	BOOLEAN		notready = TRUE, fail = TRUE;
	u4Byte		IQK_CMD;
	u2Byte		IQK_Apply[2]	= {0xc94, 0xe94};
	
	cal_retry = 0;
	fail = TRUE;

	if(*pDM_Odm->pBandWidth == 2)
	{ //80M	
		if(idx == TX_IQK)
		{
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]============ S%d WBTXIQK ============\n", Path));
			}
		else
		{
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]============ S%d RXIQK ============\n", Path));
		    }	
		}
	else
	{
		if(idx == TX_IQK)
		{
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]============ S%d TXIQK ============\n", Path));
			}
		else
		{
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]============ S%d RXIQK ============\n", Path));
		    }
		}
		

	while(fail)
	{

		delay_count = 0;
		notready = TRUE;
//				ODM_SetBBReg(pDM_Odm, 0x9a4, BIT(21)|BIT(20), Path);	
		if(idx == TX_IQK)
		{
					
					if(*pDM_Odm->pBandWidth == 2) //80M
						IQK_CMD =  0xf8000508;	//WBIQK
					else
						IQK_CMD = 0xf8000108;	//normal IQK

			IQK_CMD = IQK_CMD|(1<<(Path+4))|(Path<<1);
					
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
						("[IQK]TXK_Trigger = 0x%x\n", IQK_CMD));
						/*  
						{0xf8000118, 0xf8000128} ==> 20 TXK (CMD = 3)
						{0xf8000118, 0xf8000128} ==> 40 TXK (CMD = 4)
						{0xf8000518, 0xf8000528} ==> 80 WBTXK (CMD = 5)
						*/

			ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD);
			ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD+0x1);
				}
		else if(idx == RX_IQK)
		{
//					IQK_CMD = (0xf8000001|(9-*pDM_Odm->pBandWidth)<<8|(1<<(4+Path)));
			IQK_CMD = 0xf8000208 |(1<<(Path+4))|(Path<<1);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
						("[IQK]TXK_Trigger = 0x%x\n", IQK_CMD));
						/*
						{0xf8000911, 0xf8000921, 0xf8000941, 0xf8000981} ==> 20 WBRXK (CMD = 9)
						{0xf8000811, 0xf8000821, 0xf8000841, 0xf8000881} ==> 40 WBRXK (CMD = 8)
						{0xf8000711, 0xf8000721, 0xf8000741, 0xf8000781} ==> 80 WBRXK (CMD = 7)
						*/
			ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD);
			ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD+0x1);
				}

		if((*pDM_Odm->pBandWidth == 2)&&(idx == TX_IQK)) //80M				
					ODM_delay_ms(WBIQK_delay_8822B);
				else
					ODM_delay_ms(IQK_delay_8822B);
				
		while(notready)
		{
					notready = (BOOLEAN) ODM_GetBBReg(pDM_Odm, 0x1b00, BIT(0));
			if(!notready)
			{
							fail = (BOOLEAN) ODM_GetBBReg(pDM_Odm, 0x1b08, BIT(26));
							break;
					}
					ODM_delay_ms(1);
					delay_count++;
			if(delay_count >= 300)
			{
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
							("[IQK]S%d IQK timeout!!!\n", Path));
//						_IQK_ResetNCTL_8822B(pDM_Odm);
						break;
					}
				}

		//switch subpage
		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]S%d ==> 0x1b20 = 0x%x\n", Path, ODM_Read4Byte(pDM_Odm, 0x1b20)));
		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]S%d ==> 0x1b00 = 0x%x, 0x1b08 = 0x%x\n", Path, ODM_Read4Byte(pDM_Odm, 0x1b00), ODM_Read4Byte(pDM_Odm, 0x1b08)));

				if(fail)
					cal_retry++;
		if(cal_retry >2)
					break;
	}
				
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]S%d ==> delay_count = 0x%d, cal_retry = %x\n", Path, delay_count, cal_retry));
			
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008|(Path<<1));
	if(!fail)
	{
		if(idx == TX_IQK)
					pIQK_info->IQC_Matrix[idx][Path] = ODM_Read4Byte(pDM_Odm, 0x1b38);	
		else if(idx == RX_IQK)
					pIQK_info->IQC_Matrix[idx][Path] = ODM_Read4Byte(pDM_Odm, 0x1b3c);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
					("[IQK]S%d_IQC = 0x%x\n", Path, pIQK_info->IQC_Matrix[idx][Path]));
			}

	if(idx == RX_IQK)
	{
				if(pIQK_info->IQK_fail[TX_IQK][Path] == FALSE)			// TXIQK success in RXIQK
					ODM_Write4Byte( pDM_Odm, 0x1b38, pIQK_info->IQC_Matrix[TX_IQK][Path]);
				else
			ODM_Write4Byte( pDM_Odm, 0x1b38, 0x20000000	);

		if(!fail)												// RXIQK success
			ODM_SetBBReg(pDM_Odm, IQK_Apply[Path], (BIT11|BIT10), 0x1);
			}		
			pIQK_info->IQK_fail[idx][Path] = fail;
}




VOID
_IQK_IQKbyPath_8822B(
	IN	PVOID		pDM_VOID
)
{	
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte		Path = 0, delay_count = 0, cal_retry = 0, idx;
	BOOLEAN		notready = TRUE, fail = TRUE;
	u4Byte		IQK_CMD;
	u2Byte		IQK_Apply[2]	= {0xc94, 0xe94};

#if 1
	//S0&S1 LOK
	for(Path =0; Path <=1; Path++)
	{
		cal_retry = 0;
		fail = TRUE;
		
		if(Path == 0)
		{
			ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf800000a);
			ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x3f);
		}
		else
		{
			ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008);
			ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x3f);
		}
		
		//switch subpage
		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);

		//I&Q MUX setting, It will be placed in NCTL 
		ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x9);

		//tone index
//			ODM_Write1Byte(pDM_Odm, 0x1b23, 0x01); //tone index
		// diff_ob
		if(*pDM_Odm->pBandType == ODM_BAND_5G)
			ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x80);
		else
			ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
	
		// RXPI data, TXPI data are filled by NCTL
		if(*pDM_Odm->pBandType == ODM_BAND_5G)
		{
			ODM_Write4Byte(pDM_Odm, 0x1b20, 0x01060028);
			ODM_Write4Byte(pDM_Odm, 0x1b24, 0x01060028);
		}
		else
		{
			ODM_Write4Byte(pDM_Odm, 0x1b20, 0x01060028);
			ODM_Write4Byte(pDM_Odm, 0x1b24, 0x01060048);
		}

		if(*pDM_Odm->pBandType == ODM_BAND_5G)
		{
			//TxAGC
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x514eb);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x514eb);
			// Rx PGA2
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xadc00);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xadc00);
		}
		else
		{
			//TxAGC
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x5102f);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x5102f);
			// Rx PGA2
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xadc00);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xadc00);
		}
		_IQK_RFESetting_8822B(pDM_Odm,FALSE);

					
		_LOK_One_Shot_8822B(pDM_Odm, Path);
	}


	//S0, S1 TX/RX IQK
	for(Path =0; Path <=1; Path++)
	{
		cal_retry = 0;
		fail = TRUE;

		
		if(Path == 0)
		{
			ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf800000a);
			ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x3f);
		}
		else
		{
			ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008);
			ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x3f);
		}

		//switch subpage
		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);

		//I&Q MUX setting, It will be placed in NCTL 
		ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x9);

		//tone index
//			ODM_Write1Byte(pDM_Odm, 0x1b23, 0x01); //tone index
		// diff_ob
		if(*pDM_Odm->pBandType == ODM_BAND_5G)
			ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x80);
		else
			ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
	
		for(idx = 0; idx <= 1; idx++)
		{						// ii = 0:TXK , 1: RXK


			// RXPI data, TXPI data are filled by NCTL
			if(*pDM_Odm->pBandType == ODM_BAND_5G)
			{
				ODM_Write4Byte(pDM_Odm, 0x1b20, 0x01060028);
				ODM_Write4Byte(pDM_Odm, 0x1b24, 0x01060028);
			}
			else
			{
				ODM_Write4Byte(pDM_Odm, 0x1b20, 0x01060028);
				ODM_Write4Byte(pDM_Odm, 0x1b24, 0x01060048);
			}

			if(idx == TX_IQK)
			{
				if(*pDM_Odm->pBandType == ODM_BAND_5G)
				{
					//TxAGC
			//		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x5146d);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x514eb);
			//		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x5146d);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x514eb);
//			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x514e9);
//					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x515f2);				
					// Rx PGA2
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xadc00);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xadc00);
				}
				else
				{
					//TxAGC
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x5102f);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x5102f);
							
					// Rx PGA2
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xadc00);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xadc00);
				}
				_IQK_RFESetting_8822B(pDM_Odm,FALSE);
			}
			else
			{
				if(*pDM_Odm->pBandType == ODM_BAND_5G)
				{
					//TxAGC
			//		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x5146d);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x50c00);
			//		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x5146d);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x50c00);
			
					// Rx PGA2
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xa9c00);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xa9c00);
				}
				else
				{
						//TxAGC
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask, 0x504e0);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask, 0x504e0);
						
						// Rx PGA2
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xadc00);
					ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xadc00);
				}
				_IQK_RFESetting_8822B(pDM_Odm,TRUE);
			}
		
//			if(idx == TX_IQK)
//				_LOK_One_Shot_8822B(pDM_Odm, Path);
			_IQK_One_Shot_8822B(pDM_Odm, Path,idx);

		}
	}

	/*reload IQK default setting, It will be placed in NCTL*/
	for (Path = 0; Path <= 1; Path++) {
		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);		
		ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x0);
		if(*pDM_Odm->pBandWidth == 2)
			ODM_Write4Byte(pDM_Odm, 0x1b2c, 0x7);	
		else
			ODM_Write4Byte(pDM_Odm, 0x1b2c, 0x3);	
		ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT(13) | BIT(12), 0x3); 

	}

#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,
		("[IQK]==========LOK summary ==========\n"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]LOK0_notready = %d, LOK1_notready = %d\n", 
		pIQK_info->LOK_fail[0], pIQK_info->LOK_fail[1]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,
		("[IQK]==========IQK summary ==========\n"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]TXIQK0_fail = %d, TXIQK1_fail = %d\n", 
		pIQK_info->IQK_fail[0][0], pIQK_info->IQK_fail[0][1]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]RXIQK0_fail = %d, RXIQK1_fail = %d\n", 
		pIQK_info->IQK_fail[1][0], pIQK_info->IQK_fail[1][1]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,
		("[IQK]================================\n"));
}

VOID
_IQK_StartIQK_8822B(
	IN PDM_ODM_T		pDM_Odm,
	IN u1Byte chnlIdx
	)
{	

	u4Byte tmp;
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
		("[IQK]pBandType = %d, BandWidth = %d, ExtPA2G = %d, ExtPA5G = %d\n", *pDM_Odm->pBandType, *pDM_Odm->pBandWidth, pDM_Odm->ExtPA, pDM_Odm->ExtPA5G));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
		("[IQK]Interface = %d, CutVersion = %x\n", pDM_Odm->SupportInterface, pDM_Odm->CutVersion));

	//0xdf:B11 = 1,B4 = 0, B1 = 1
#if 1	
	tmp = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask);
    tmp = (tmp&(~BIT4))|BIT1|BIT11;
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, tmp);	
	
	tmp = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, bRFRegOffsetMask);
	tmp = (tmp&(~BIT4))|BIT1|BIT11;
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, bRFRegOffsetMask, tmp);
#endif

	//release 0x56 TXBB
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x65, bRFRegOffsetMask, 0x09000);	
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x65, bRFRegOffsetMask, 0x09000);

	// WE_LUT_TX_LOK
	if(*pDM_Odm->pBandType == ODM_BAND_5G)
	{
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, BIT4, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x33, BIT1|BIT0, 0x1);//[1:0]: AMODE=1; GMODE=0
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, BIT4, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x33, BIT1|BIT0, 0x1);//[1:0]: AMODE=1; GMODE=0
	}	
	else
	{
    	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, BIT4, 0x1);
    	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x33, BIT1|BIT0, 0x0);//[1:0]: AMODE=1; GMODE=0
    	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xef, BIT4, 0x1);
    	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x33, BIT1|BIT0, 0x0);//[1:0]: AMODE=1; GMODE=0
	}

	_IQK_IQKbyPath_8822B(pDM_Odm);

	// Ext. PA ON
//	ODM_Write4Byte(pDM_Odm, 0xc00, 0x00000004);
//	ODM_Write4Byte(pDM_Odm, 0xcb0, 0x77777777);
//	ODM_Write4Byte(pDM_Odm, 0xcb4, 0x00007777);
//	ODM_Write4Byte(pDM_Odm, 0xcbc, 0x0000083b);
//	ODM_Write4Byte(pDM_Odm, 0x1990, 0x00000000);

}

VOID
_IQCalibrate_8822B_Init(
	IN	PVOID		pDM_VOID
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte	ii, jj;

	if(pIQK_info->IQKtimes == 0x0)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]=====>PHY_IQCalibrate_8822B_Init\n"));
		for(jj = 0; jj < 2; jj++)
		{
			for(ii = 0; ii < NUM; ii++)
			{
				pIQK_info->LOK_fail[ii] = TRUE;
				pIQK_info->IQK_fail[jj][ii] = TRUE;
				pIQK_info->IQC_Matrix[jj][ii] = 0x20000000;
			}
		}
	}
	pIQK_info->IQKtimes++;
}


VOID	
phy_IQCalibrate_8822B(
	IN PDM_ODM_T		pDM_Odm,
	IN u1Byte		Channel
	)
{

	u4Byte	MAC_backup[MAC_REG_NUM_8822B], BB_backup[BB_REG_NUM_8822B], RF_backup[RF_REG_NUM_8822B][2];
	u4Byte 	Backup_MAC_REG[MAC_REG_NUM_8822B] = {0x520, 0x550}; 
	u4Byte 	Backup_BB_REG[BB_REG_NUM_8822B] = {0x808, 0x90c, 0xc00, 0xcb0, 0xcb4, 0xcbc, 0xe00, 0xeb0, 0xeb4, 0xebc, 0x1990, 0x9a4}; 
	u4Byte 	Backup_RF_REG[RF_REG_NUM_8822B] = {0x0, 0x8f, 0x65}; 
	u1Byte 	chnlIdx = ODM_GetRightChnlPlaceforIQK(Channel);
	
	_IQK_BackupMacBB_8822B(pDM_Odm, MAC_backup, BB_backup, Backup_MAC_REG, Backup_BB_REG);
	_IQK_AFESetting_8822B(pDM_Odm,TRUE);
	_IQK_BackupRF_8822B(pDM_Odm, RF_backup, Backup_RF_REG);
	_IQK_ConfigureMACBB_8822B(pDM_Odm);
	_IQK_StartIQK_8822B(pDM_Odm, chnlIdx);
//	_IQK_ResetNCTL_8822B(pDM_Odm);  //for 3-wire to  BB use
	_IQK_AFESetting_8822B(pDM_Odm,FALSE);
	_IQK_RestoreMacBB_8822B(pDM_Odm, MAC_backup, BB_backup, Backup_MAC_REG, Backup_BB_REG);
 	_IQK_RestoreRF_8822B(pDM_Odm, Backup_RF_REG, RF_backup);

}

/*
IQK version:v0.6 , NCTL v0.2
1. MAC TX pause when IQK
2. RX don't go through CFIR by 0x1b0c
*/

VOID
PHY_IQCalibrate_8822B(
	IN	PVOID		pDM_VOID,
	IN	BOOLEAN 	bReCovery
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	PADAPTER 		pAdapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);	
	
	#if (MP_DRIVER == 1)
		#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)	
			PMPT_CONTEXT	pMptCtx = &(pAdapter->MptCtx);	
		#else// (DM_ODM_SUPPORT_TYPE == ODM_CE)
			PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);		
		#endif	
	#endif//(MP_DRIVER == 1)
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
		if (ODM_CheckPowerStatus(pAdapter) == FALSE)
			return;
	#endif

	#if MP_DRIVER == 1	
		if( pMptCtx->bSingleTone || pMptCtx->bCarrierSuppression )
			return;
	#endif
	
#endif

	_IQCalibrate_8822B_Init(pDM_VOID);


	if ( ! pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
		
		ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
		pDM_Odm->RFCalibrateInfo.bIQKInProgress = TRUE;
		ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
	
		pDM_Odm->RFCalibrateInfo.IQK_StartTime = ODM_GetCurrentTime(pDM_Odm);
		
	#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
		phy_IQCalibrate_8822B(pDM_Odm, pHalData->CurrentChannel);
		/*DBG_871X("%s,%d, do IQK %u ms\n", __func__, __LINE__, rtw_get_passing_time_ms(time_iqk));*/
	#else
		phy_IQCalibrate_8822B(pDM_Odm, *pDM_Odm->pChannel);
	#endif

		pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime = ODM_GetProgressingTime( pDM_Odm, pDM_Odm->RFCalibrateInfo.IQK_StartTime);
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD,  ("[IQK]IQK ProgressingTime = %lld ms\n", pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime));
		
		ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
		pDM_Odm->RFCalibrateInfo.bIQKInProgress = FALSE;
		ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
	}
	else{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]== Return the IQK CMD, because the IQK in Progress ==\n"));
	}





}

#endif


