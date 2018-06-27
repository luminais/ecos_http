// remove by cairui
//#include <stdint.h>
//#include <linux/atm.h>
//#include <linux/atmdev.h>
//#include <sys/sysinfo.h>
extern int getWanSpeed(char *interface);
extern int setWanSpeed(char *interface, int speed);

#include "prmt_wandevice.h" //includes apmib.h
#include "prmt_wandsldiagnostics.h"
#include "prmt_wancondevice.h"
#include "prmt_utility.h"
//remove by cairui
//#include <linux/wireless.h>
//#include <linux/netdevice.h>


#ifdef CONFIG_DEV_xDSL
int get_TotalWANStat( unsigned int *bs, unsigned int *br, unsigned int *ps, unsigned int *pr );


struct CWMP_OP tStatsTotalLeafOP = { getStatsTotal, NULL };
struct CWMP_PRMT tStatsTotalLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ReceiveBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"TransmitBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"CellDelin",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"LinkRetrain",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"InitErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"InitTimeouts",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"LossOfFraming",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ErroredSecs",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"SeverelyErroredSecs",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"FECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ATUCFECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"HECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ATUCHECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"CRCErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP},
{"ATUCCRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsTotalLeafOP}
};
enum eStatsTotalLeaf
{
	eTT_ReceiveBlocks,	
	eTT_TransmitBlocks,
	eTT_CellDelin,	
	eTT_LinkRetrain,	
	eTT_InitErrors,	
	eTT_InitTimeouts,	
	eTT_LossOfFraming,	
	eTT_ErroredSecs,	
	eTT_SeverelyErroredSecs,
	eTT_FECErrors,	
	eTT_ATUCFECErrors,	
	eTT_HECErrors,	
	eTT_ATUCHECErrors,
	eTT_CRCErrors,
	eTT_ATUCCRCErrors
};
struct CWMP_LEAF tStatsTotalLeaf[] =
{
{ &tStatsTotalLeafInfo[eTT_ReceiveBlocks] },	
{ &tStatsTotalLeafInfo[eTT_TransmitBlocks] },
{ &tStatsTotalLeafInfo[eTT_CellDelin] },	
{ &tStatsTotalLeafInfo[eTT_LinkRetrain] },	
{ &tStatsTotalLeafInfo[eTT_InitErrors] },	
{ &tStatsTotalLeafInfo[eTT_InitTimeouts] },	
{ &tStatsTotalLeafInfo[eTT_LossOfFraming] },	
{ &tStatsTotalLeafInfo[eTT_ErroredSecs] },	
{ &tStatsTotalLeafInfo[eTT_SeverelyErroredSecs] },
{ &tStatsTotalLeafInfo[eTT_FECErrors] },	
{ &tStatsTotalLeafInfo[eTT_ATUCFECErrors] },	
{ &tStatsTotalLeafInfo[eTT_HECErrors] },	
{ &tStatsTotalLeafInfo[eTT_ATUCHECErrors] },
{ &tStatsTotalLeafInfo[eTT_CRCErrors] },
{ &tStatsTotalLeafInfo[eTT_ATUCCRCErrors] },
{ NULL }
};



struct CWMP_OP tStatsShowtimeLeafOP = { getStatsShowtime,NULL };
struct CWMP_PRMT tStatsShowtimeLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ReceiveBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"TransmitBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"CellDelin",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"LinkRetrain",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"InitErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"InitTimeouts",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"LossOfFraming",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ErroredSecs",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"SeverelyErroredSecs",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"FECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ATUCFECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"HECErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ATUCHECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"CRCErrors",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP},
{"ATUCCRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tStatsShowtimeLeafOP}
};
enum eStatsShowtimeLeaf
{
	eST_ReceiveBlocks,	
	eST_TransmitBlocks,
	eST_CellDelin,	
	eST_LinkRetrain,	
	eST_InitErrors,	
	eST_InitTimeouts,	
	eST_LossOfFraming,	
	eST_ErroredSecs,	
	eST_SeverelyErroredSecs,
	eST_FECErrors,	
	eST_ATUCFECErrors,	
	eST_HECErrors,	
	eST_ATUCHECErrors,
	eST_CRCErrors,
	eST_ATUCCRCErrors
};
struct CWMP_LEAF tStatsShowtimeLeaf[] =
{
{ &tStatsShowtimeLeafInfo[eST_ReceiveBlocks] },	
{ &tStatsShowtimeLeafInfo[eST_TransmitBlocks] },
{ &tStatsShowtimeLeafInfo[eST_CellDelin] },	
{ &tStatsShowtimeLeafInfo[eST_LinkRetrain] },	
{ &tStatsShowtimeLeafInfo[eST_InitErrors] },	
{ &tStatsShowtimeLeafInfo[eST_InitTimeouts] },	
{ &tStatsShowtimeLeafInfo[eST_LossOfFraming] },	
{ &tStatsShowtimeLeafInfo[eST_ErroredSecs] },	
{ &tStatsShowtimeLeafInfo[eST_SeverelyErroredSecs] },
{ &tStatsShowtimeLeafInfo[eST_FECErrors] },	
{ &tStatsShowtimeLeafInfo[eST_ATUCFECErrors] },	
{ &tStatsShowtimeLeafInfo[eST_HECErrors] },	
{ &tStatsShowtimeLeafInfo[eST_ATUCHECErrors] },
{ &tStatsShowtimeLeafInfo[eST_CRCErrors] },
{ &tStatsShowtimeLeafInfo[eST_ATUCCRCErrors] },
{ NULL }
};


struct CWMP_PRMT tWANDSLIFSTATSObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Total",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"Showtime",			eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eWANDSLIFSTATSObject
{
	eDSLIFTotal,
	eDSLIFShowtime
};
struct CWMP_NODE tWANDSLIFSTATSObject[] =
{
/*info,  					leaf,			node)*/
{&tWANDSLIFSTATSObjectInfo[eDSLIFTotal],	tStatsTotalLeaf,	NULL},
{&tWANDSLIFSTATSObjectInfo[eDSLIFShowtime],	tStatsShowtimeLeaf,	NULL},
{NULL,						NULL,			NULL}
};


struct CWMP_OP tWANDSLIFCFGLeafOP = { getWANDSLIfCfg,	setWANDSLIfCfg };
struct CWMP_PRMT tWANDSLIFCFGLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANDSLIFCFGLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{"LinkEncapsulationSupported",	eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"LinkEncapsulationRequested",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANDSLIFCFGLeafOP},
{"LinkEncapsulationUsed",	eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{"ModulationType",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{"StandardsSupported",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"StandardUsed",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{"LineEncoding",		eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"DataPath",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"InterleaveDepth",		eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
/*LineNumber*/
{"UpstreamCurrRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamCurrRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamMaxRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamMaxRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamNoiseMargin",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamNoiseMargin",	eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamAttenuation",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamAttenuation",	eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"UpstreamPower",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"DownstreamPower",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"ATURVendor",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"ATURCountry",			eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
/*ATURANSIStd*/
/*ATURANSIRev*/
{"ATUCVendor",			eCWMP_tSTRING,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
{"ATUCCountry",			eCWMP_tUINT,	CWMP_READ,		&tWANDSLIFCFGLeafOP},
/*ATUCANSIStd*/
/*ATUCANSIRev*/
{"TotalStart",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP},
{"ShowtimeStart",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANDSLIFCFGLeafOP}
/*LastShowtimeStart*/
/*CurrentDayStart*/
/*QuarterHourStart*/
};
enum eWANDSLIFCFGLeaf
{
	eDSL_Enable,			
	eDSL_Status,			
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	eDSL_LinkEncapsulationSupported,
	eDSL_LinkEncapsulationRequested,
	eDSL_LinkEncapsulationUsed,
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	eDSL_ModulationType,		
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	eDSL_StandardsSupported,
	eDSL_StandardUsed,
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	eDSL_LineEncoding,		
	eDSL_DataPath,		
	eDSL_InterleaveDepth,	
	/*LineNumber*/
	eDSL_UpstreamCurrRate,	
	eDSL_DownstreamCurrRate,	
	eDSL_UpstreamMaxRate,	
	eDSL_DownstreamMaxRate,	
	eDSL_UpstreamNoiseMargin,	
	eDSL_DownstreamNoiseMargin,	
	eDSL_UpstreamAttenuation,	
	eDSL_DownstreamAttenuation,	
	eDSL_UpstreamPower,		
	eDSL_DownstreamPower,	
	eDSL_ATURVendor,		
	eDSL_ATURCountry,		
	/*ATURANSIStd*/
	/*ATURANSIRev*/
	eDSL_ATUCVendor,		
	eDSL_ATUCCountry,		
	/*ATUCANSIStd*/
	/*ATUCANSIRev*/
	eDSL_TotalStart,		
	eDSL_ShowtimeStart
	/*LastShowtimeStart*/
	/*CurrentDayStart*/
	/*QuarterHourStart*/
};
struct  CWMP_LEAF tWANDSLIFCFGLeaf[] =
{
{ &tWANDSLIFCFGLeafInfo[eDSL_Enable] },			
{ &tWANDSLIFCFGLeafInfo[eDSL_Status] },			
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_LinkEncapsulationSupported] },			
{ &tWANDSLIFCFGLeafInfo[eDSL_LinkEncapsulationRequested] },			
{ &tWANDSLIFCFGLeafInfo[eDSL_LinkEncapsulationUsed] },			
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_ModulationType] },		
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_StandardsSupported] },			
{ &tWANDSLIFCFGLeafInfo[eDSL_StandardUsed] },			
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{ &tWANDSLIFCFGLeafInfo[eDSL_LineEncoding] },		
{ &tWANDSLIFCFGLeafInfo[eDSL_DataPath] },		
{ &tWANDSLIFCFGLeafInfo[eDSL_InterleaveDepth] },	
/*LineNumber*/
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamCurrRate] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamCurrRate] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamMaxRate] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamMaxRate] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamNoiseMargin] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamNoiseMargin] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamAttenuation] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamAttenuation] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_UpstreamPower] },		
{ &tWANDSLIFCFGLeafInfo[eDSL_DownstreamPower] },	
{ &tWANDSLIFCFGLeafInfo[eDSL_ATURVendor] },		
{ &tWANDSLIFCFGLeafInfo[eDSL_ATURCountry] },		
/*ATURANSIStd*/
/*ATURANSIRev*/
{ &tWANDSLIFCFGLeafInfo[eDSL_ATUCVendor] },		
{ &tWANDSLIFCFGLeafInfo[eDSL_ATUCCountry] },		
/*ATUCANSIStd*/
/*ATUCANSIRev*/
{ &tWANDSLIFCFGLeafInfo[eDSL_TotalStart] },		
{ &tWANDSLIFCFGLeafInfo[eDSL_ShowtimeStart] },
/*LastShowtimeStart*/
/*CurrentDayStart*/
/*QuarterHourStart*/
{ NULL }
};


#ifdef _SUPPORT_ADSL2WAN_PROFILE_
struct CWMP_OP tWANDSLIFTestParamsLeafOP = { getWANDSLIFTestParams,	NULL };
struct CWMP_PRMT tWANDSLIFTestParamsLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
/*HLOGGds*/
/*HLOGGus*/
{"HLOGpsds",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"HLOGpsus",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"HLOGMTds",		eCWMP_tUINT,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"HLOGMTus",		eCWMP_tUINT,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
/*QLNGds*/
/*QLNGus*/
{"QLNpsds",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"QLNpsus",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"QLNMTds",		eCWMP_tUINT,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"QLNMTus",		eCWMP_tUINT,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
/*SNRGds*/
/*SNRGus*/
{"SNRpsds",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"SNRpsus",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"SNRMTds",		eCWMP_tUINT,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"SNRMTus",		eCWMP_tUINT,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"LATNds",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"LATNus",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"SATNds",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
{"SATNus",		eCWMP_tSTRING,	CWMP_READ,	&tWANDSLIFTestParamsLeafOP},
};
enum eWANDSLIFTestParamsLeaf
{
	/*eTP_HLOGGds*/
	/*eTP_HLOGGus*/
	eTP_HLOGpsds,
	eTP_HLOGpsus,
	eTP_HLOGMTds,
	eTP_HLOGMTus,
	/*eTP_QLNGds*/
	/*eTP_QLNGus*/
	eTP_QLNpsds,
	eTP_QLNpsus,
	eTP_QLNMTds,
	eTP_QLNMTus,
	/*eTP_SNRGds*/
	/*eTP_SNRGus*/
	eTP_SNRpsds,
	eTP_SNRpsus,
	eTP_SNRMTds,
	eTP_SNRMTus,
	eTP_LATNds,
	eTP_LATNus,
	eTP_SATNds,
	eTP_SATNus,
};
struct  CWMP_LEAF tWANDSLIFTestParamsLeaf[] =
{
/*eTP_HLOGGds*/
/*eTP_HLOGGus*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGMTus] },
/*eTP_QLNGds*/
/*eTP_QLNGus*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNMTus] },
/*eTP_SNRGds*/
/*eTP_SNRGus*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRMTus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_LATNds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_LATNus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SATNds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SATNus] },
{ NULL }
};

int getWANDSLIFTestParams(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	char		*pChar=NULL;
	int		vINT=0;
	unsigned int	vUINT=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "HLOGpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLOGpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "HLOGpsus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_HLOGpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "HLOGMTds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_HLOGMTds, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "HLOGMTus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_HLOGMTus, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "QLNpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_QLNpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "QLNpsus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_QLNpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "QLNMTds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_QLNMTds, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "QLNMTus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_QLNMTus, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "SNRpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SNRpsus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_SNRpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SNRMTds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_SNRMTds, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "SNRMTus" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SNRMTus, &vUINT )<0 )
			*data = intdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "LATNds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_LATNds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "LATNus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_LATNus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SATNds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_SATNds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SATNus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_SATNus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_SUPPORT_ADSL2WAN_PROFILE_

struct CWMP_PRMT tWANDSLIFCFGObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{"TestParams",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif //_SUPPORT_ADSL2WAN_PROFILE_
};
enum eWANDSLIFCFGObject
{
	eDSLIFCFG_Stats,
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	eDSLIFCFG_TestParams,
#endif //_SUPPORT_ADSL2WAN_PROFILE_
};
struct CWMP_NODE tWANDSLIFCFGObject[] =
{
/*info,  					leaf,			node)*/
{&tWANDSLIFCFGObjectInfo[eDSLIFCFG_Stats],	NULL,			tWANDSLIFSTATSObject},
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
{&tWANDSLIFCFGObjectInfo[eDSLIFCFG_TestParams],	tWANDSLIFTestParamsLeaf,NULL},
#endif //_SUPPORT_ADSL2WAN_PROFILE_
{NULL,						NULL,			NULL}
};
#endif //#ifdef CONFIG_DEV_xDSL

#if 1//defined CONFIG_ETHWAN
struct CWMP_OP tWANEthInfCfgLeafOP = {getWANEthInfCfg, setWANEthInfCfg};
struct CWMP_PRMT tWANEthInfCfgLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"Enable", eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANEthInfCfgLeafOP},
	{"Status", eCWMP_tSTRING,	CWMP_READ,		&tWANEthInfCfgLeafOP},
	{"MACAddress", eCWMP_tSTRING,	CWMP_READ,		&tWANEthInfCfgLeafOP},
	{"MaxBitRate", eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANEthInfCfgLeafOP},
	{"DuplexMode", eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANEthInfCfgLeafOP}
};

enum eWANEthInfCfgLeaf
{
	eETHWAN_Enable,
	eETHWAN_Status,
	eETHWAN_MACAddress,
	eETHWAN_MaxBitRate,
	eETHWAN_DuplexMode
};

struct  CWMP_LEAF tWANEthInfCfgLeaf[] =
{
	{ &tWANEthInfCfgLeafInfo[eETHWAN_Enable] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_Status] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_MACAddress] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_MaxBitRate] },
	{ &tWANEthInfCfgLeafInfo[eETHWAN_DuplexMode] },
	{ NULL }
};

struct CWMP_PRMT tWANEthInfCfgObjectInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"Stats", eCWMP_tOBJECT, CWMP_READ, NULL},
};

enum eWANEthInfCfgObject
{
	eWANEthInfCfg_Stats,
};

struct CWMP_OP tWANEthInfCfgStatsLeafOP = {getWANEthStatsLeaf, NULL};
struct CWMP_PRMT tWANEthInfCfgStatsLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"BytesSent", eCWMP_tUINT, CWMP_READ, &tWANEthInfCfgStatsLeafOP},
	{"BytesReceived", eCWMP_tUINT, CWMP_READ, &tWANEthInfCfgStatsLeafOP},
	{"PacketsSent", eCWMP_tUINT,	CWMP_READ, &tWANEthInfCfgStatsLeafOP},
	{"PacketsReceived", eCWMP_tUINT, CWMP_READ, &tWANEthInfCfgStatsLeafOP}
};

enum eWANEthInfCfgStatsLeaf
{
	eETHWANStats_BytesSent,
	eETHWANStats_BytesReceived,
	eETHWANStats_PacketsSent,
	eETHWANStats_PacketsReceived
};

struct  CWMP_LEAF tWANEthInfCfgStatsLeaf[] =
{
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_BytesSent] },
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_BytesReceived] },
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_PacketsSent] },
	{ &tWANEthInfCfgStatsLeafInfo[eETHWANStats_PacketsReceived] },
	{ NULL }
};

struct CWMP_NODE tWANEthInfCfgObject[] =
{
 /*info,  					leaf,			node)*/
 {&tWANEthInfCfgObjectInfo[eWANEthInfCfg_Stats], tWANEthInfCfgStatsLeaf, NULL},
 {NULL, NULL, NULL}
};
#endif // #if defined CONFIG_ETHWAN

struct CWMP_OP tWANCmnIfCfgLeafOP = { getWANCmnIfCfg, setWANCmnIfCfg };
struct CWMP_PRMT tWANCmnIfCfgLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"EnabledForInternet",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANCmnIfCfgLeafOP},
{"WANAccessType",		eCWMP_tSTRING,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"Layer1UpstreamMaxBitRate",	eCWMP_tUINT,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"Layer1DownstreamMaxBitRate",	eCWMP_tUINT,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
{"PhysicalLinkStatus",		eCWMP_tSTRING,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
#if 0 //keith	remove
{"WANAccessProvider",		eCWMP_tSTRING,	CWMP_READ,		&tWANCmnIfCfgLeafOP},
#endif 
{"TotalBytesSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP},
{"TotalBytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP},
{"TotalPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP},
{"TotalPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCmnIfCfgLeafOP}
/*MaximumActiveConnections*/
/*NumberOfActiveConnections*/
};
enum eWANCmnIfCfgLeaf
{
	eEnabledForInternet,
	eWANAccessType,
	eLayer1UpstreamMaxBitRate,
	eLayer1DownstreamMaxBitRate,
	ePhysicalLinkStatus,
#if 0 //keith	remove	
	eWANAccessProvider,
#endif	
	eTotalBytesSent,
	eTotalBytesReceived,
	eTotalPacketsSent,
	eTotalPacketsReceived
};
struct CWMP_LEAF tWANCmnIfCfgLeaf[] =
{
{ &tWANCmnIfCfgLeafInfo[eEnabledForInternet] },
{ &tWANCmnIfCfgLeafInfo[eWANAccessType] },
{ &tWANCmnIfCfgLeafInfo[eLayer1UpstreamMaxBitRate] },
{ &tWANCmnIfCfgLeafInfo[eLayer1DownstreamMaxBitRate] },
{ &tWANCmnIfCfgLeafInfo[ePhysicalLinkStatus] },
#if 0 //keith	remove
{ &tWANCmnIfCfgLeafInfo[eWANAccessProvider] },
#endif
{ &tWANCmnIfCfgLeafInfo[eTotalBytesSent] },
{ &tWANCmnIfCfgLeafInfo[eTotalBytesReceived] },
{ &tWANCmnIfCfgLeafInfo[eTotalPacketsSent] },
{ &tWANCmnIfCfgLeafInfo[eTotalPacketsReceived] },
{ NULL }
};

struct CWMP_OP tWANDevEntityLeafOP = { getWANDevEntity, NULL };
struct CWMP_PRMT tWANDevEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WANConnectionNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tWANDevEntityLeafOP}
};
enum eWANDevEntityLeaf
{
	eWANConnectionNumberOfEntries
};
struct CWMP_LEAF tWANDevEntityLeaf[] =
{
{ &tWANDevEntityLeafInfo[eWANConnectionNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tWAN_WANConnectionDevice_OP = { NULL, objConDev };
struct CWMP_PRMT tWANDevEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WANCommonInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef CONFIG_DEV_xDSL
	{"WANDSLInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#if 1//defined CONFIG_ETHWAN
	{"WANEthernetInterfaceConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef CONFIG_DEV_xDSL
	{"WANDSLConnectionManagement",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
	{"WANDSLDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
{"WANConnectionDevice",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_WANConnectionDevice_OP}
};

enum eWANDevEntityObject
{
	eWANCommonInterfaceConfig,
#ifdef CONFIG_DEV_xDSL
	eWANDSLInterfaceConfig,
#endif
#if 1//defined CONFIG_ETHWAN
	eWANEthernetInterfaceConfig,
#endif
#ifdef CONFIG_DEV_xDSL
	eWANDSLConnectionManagement,
	eWANDSLDiagnostics,
#endif
	eWANConnectionDevice
};

struct CWMP_NODE tWANDevEntityObject[] =
{
/*info,  						leaf,			node)*/
{ &tWANDevEntityObjectInfo[eWANCommonInterfaceConfig],	tWANCmnIfCfgLeaf,	NULL},
#ifdef CONFIG_DEV_xDSL
	{ &tWANDevEntityObjectInfo[eWANDSLInterfaceConfig],	tWANDSLIFCFGLeaf,	tWANDSLIFCFGObject},
#endif
#if 1//defined CONFIG_ETHWAN
	{ &tWANDevEntityObjectInfo[eWANEthernetInterfaceConfig],	tWANEthInfCfgLeaf,	tWANEthInfCfgObject},
#endif
#ifdef CONFIG_DEV_xDSL
	{ &tWANDevEntityObjectInfo[eWANDSLConnectionManagement],tWANDSLCNTMNGLeaf,	tWANDSLCNTMNGObject},
	{ &tWANDevEntityObjectInfo[eWANDSLDiagnostics],		tWANDSLDIAGLeaf, NULL},
#endif
{ &tWANDevEntityObjectInfo[eWANConnectionDevice],	NULL,			NULL},
{ NULL,							NULL,			NULL }
};


struct CWMP_PRMT tWANDeviceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
};
enum eWANDeviceObject
{
	eWANDevice1
};
struct CWMP_NODE tWANDeviceObject[] =
{
/*info,  				leaf,			node)*/
{&tWANDeviceObjectInfo[eWANDevice1],	tWANDevEntityLeaf,	tWANDevEntityObject},
{NULL,					NULL,			NULL}
};

#ifdef CONFIG_DEV_xDSL
int getStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int vUInt=0;
	Modem_DSLConfigStatus MDS;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if(adsl_drv_get(RLCM_GET_DSL_STAT_TOTAL, (void *)&MDS, TR069_STAT_SIZE)==0)
	{
#if 0
		return ERR_9002;
#else
		memset( &MDS, 0, sizeof(MDS) );
#endif
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ReceiveBlocks" )==0 )
	{
		*data = uintdup( MDS.ReceiveBlocks );
	}else if( strcmp( lastname, "TransmitBlocks" )==0 )
	{	
		*data = uintdup( MDS.TransmitBlocks );
	}else if( strcmp( lastname, "CellDelin" )==0 )
	{	
		*data = uintdup( MDS.CellDelin );
	}else if( strcmp( lastname, "LinkRetrain" )==0 )
	{	
		*data = uintdup( MDS.LinkRetain );
	}else if( strcmp( lastname, "InitErrors" )==0 )
	{	
		*data = uintdup( MDS.InitErrors );
	}else if( strcmp( lastname, "InitTimeouts" )==0 )
	{	
		*data = uintdup( MDS.InitTimeouts );
	}else if( strcmp( lastname, "LossOfFraming" )==0 )
	{	
		*data = uintdup( MDS.LOF );
	}else if( strcmp( lastname, "ErroredSecs" )==0 )
	{	
		*data = uintdup( MDS.ES );
	}else if( strcmp( lastname, "SeverelyErroredSecs" )==0 )
	{	
		*data = uintdup( MDS.SES );
	}else if( strcmp( lastname, "FECErrors" )==0 )
	{	
		*data = uintdup( MDS.FEC );
	}else if( strcmp( lastname, "ATUCFECErrors" )==0 )
	{	
		*data = uintdup( MDS.AtucFEC );
	}else if( strcmp( lastname, "HECErrors" )==0 )
	{	
		*data = uintdup( MDS.HEC );
	}else if( strcmp( lastname, "ATUCHECErrors" )==0 )
	{	
		*data = uintdup( MDS.AtucHEC );
	}else if( strcmp( lastname, "CRCErrors" )==0 )
	{	
		*data = uintdup( MDS.CRC );
	}else if( strcmp( lastname, "ATUCCRCErrors" )==0 )
	{	
		*data = uintdup( MDS.AtucCRC );
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int getStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256]={0};
	Modem_def_counter_set vCs;
	unsigned int vUInt=0;
	Modem_DSLConfigStatus MDS;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if(adsl_drv_get(RLCM_GET_DSL_STAT_SHOWTIME, (void *)&MDS, TR069_STAT_SIZE)==0)
	{
#if 0
		return ERR_9002;
#else
		memset( &MDS, 0, sizeof(MDS) );
#endif
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ReceiveBlocks" )==0 )
	{
		*data = uintdup( MDS.ReceiveBlocks );
	}else if( strcmp( lastname, "TransmitBlocks" )==0 )
	{	
		*data = uintdup( MDS.TransmitBlocks );
	}else if( strcmp( lastname, "CellDelin" )==0 )
	{	
		*data = uintdup( MDS.CellDelin );
	}else if( strcmp( lastname, "LinkRetrain" )==0 )
	{	
		*data = uintdup( MDS.LinkRetain );
	}else if( strcmp( lastname, "InitErrors" )==0 )
	{	
		*data = uintdup( MDS.InitErrors );
	}else if( strcmp( lastname, "InitTimeouts" )==0 )
	{	
		*data = uintdup( MDS.InitTimeouts );
	}else if( strcmp( lastname, "LossOfFraming" )==0 )
	{	
		*data = uintdup( MDS.LOF );
	}else if( strcmp( lastname, "ErroredSecs" )==0 )
	{	
		*data = uintdup( MDS.ES );
	}else if( strcmp( lastname, "SeverelyErroredSecs" )==0 )
	{	
		*data = uintdup( MDS.SES );
	}else if( strcmp( lastname, "FECErrors" )==0 )
	{	
		*data = uintdup( MDS.FEC );
	}else if( strcmp( lastname, "ATUCFECErrors" )==0 )
	{	
		*data = uintdup( MDS.AtucFEC );
	}else if( strcmp( lastname, "HECErrors" )==0 )
	{	
		*data = uintdup( MDS.HEC );
	}else if( strcmp( lastname, "ATUCHECErrors" )==0 )
	{	
		*data = uintdup( MDS.AtucHEC );
	}else if( strcmp( lastname, "CRCErrors" )==0 )
	{	
		*data = uintdup( MDS.CRC );
	}else if( strcmp( lastname, "ATUCCRCErrors" )==0 )
	{	
		*data = uintdup( MDS.AtucCRC );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256]="";
	unsigned int vUInt=0;
	double vd=0;
	Modem_Identification vMId;
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	Modem_ADSL2WANConfig vMA2WC;
	char	*strAdsl2WanStd[]=
	{
		"G.992.1_Annex_A",
		"G.992.1_Annex_B",
		"G.992.1_Annex_C",
		"T1.413",
		"T1.413i2",
		"ETSI_101_388",
		"G.992.2",
		"G.992.3_Annex_A",
		"G.992.3_Annex_B",
		"G.992.3_Annex_C",

		"G.992.3_Annex_I",
		"G.992.3_Annex_J",
		"G.992.3_Annex_L",
		"G.992.3_Annex_M",
		"G.992.4",
		"G.992.5_Annex_A",
		"G.992.5_Annex_B",
		"G.992.5_Annex_C",
		"G.992.5_Annex_I",
		"G.992.5_Annex_J",

		"G.992.5_Annex_M",
		"G.993.1",
		"G.993.1_Annex_A",
		"G.993.2_Annex_A",                    
		"G.993.2_Annex_B",
		"G.993.1_Annex_C"
	};
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = intdup( 1 );
	}else if( strcmp( lastname, "Status" )==0 )
	{	
		getAdslInfo(ADSL_GET_STATE, buf, 256);
		if( strncmp( buf, "HANDSHAKING", 11 )==0 )
			*data = strdup( "Initializing" );
		else if( strncmp( buf, "SHOWTIME", 8 )==0 )
			*data = strdup( "Up" );
		else if( (strncmp( buf, "ACTIVATING", 10 )==0) ||
		         (strncmp( buf, "IDLE", 4 )==0) )
			*data = strdup( "EstablishingLink" );
		else
			*data = strdup( "NoSignal" );//or Error, Disabled
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "LinkEncapsulationSupported" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationSupported=0x%x\n", vMA2WC.LinkEncapSupported );
			buf[0]=0;;
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcat( buf, "G.992.3_Annex_K_ATM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcat( buf, "G.992.3_Annex_K_PTM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcat( buf, "G.993.2_Annex_K_ATM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcat( buf, "G.993.2_Annex_K_PTM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_994_1))
				strcat( buf, "G.994.1," );
			if(buf[0]) buf[ strlen(buf)-1 ]=0;
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "LinkEncapsulationRequested" )==0 )
	{	
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationRequested=0x%x\n", vMA2WC.LinkEncapRequested );
			if(vMA2WC.LinkEncapRequested & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcpy( buf, "G.992.3_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcpy( buf, "G.992.3_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcpy( buf, "G.993.2_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcpy( buf, "G.993.2_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapRequested & (1<<LE_G_994_1))
				strcpy( buf, "G.994.1" );
			else
				strcpy( buf, "" );
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "LinkEncapsulationUsed" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationUsed=0x%x\n", vMA2WC.LinkEncapUsed );
			if(vMA2WC.LinkEncapUsed & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcpy( buf, "G.992.3_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcpy( buf, "G.992.3_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcpy( buf, "G.993.2_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcpy( buf, "G.993.2_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_994_1))
				strcpy( buf, "G.994.1" );
			else
				strcpy( buf, "" );
			*data = strdup( buf );
		}
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "ModulationType" )==0 )
	{	
		getAdslInfo(ADSL_GET_MODE, buf, 256);
		if( strncmp( buf, "T1.413", 6 )==0 )
			*data = strdup( "ADSL_ANSI_T1.413" );
		else if( strncmp( buf, "G.dmt", 5 )==0 )
			*data = strdup( "ADSL_G.dmt" );
		else if( strncmp( buf, "G.Lite", 6 )==0 )
			*data = strdup( "ADSL_G.lite" );
		else if( strncmp( buf, "ADSL2", 5 )==0 )
			*data = strdup( "ADSL_G.dmt.bis" );
		else if( strncmp( buf, "ADSL2+", 6 )==0 )
			*data = strdup( "ADSL_2plus" );
		else
			*data = strdup( "" );
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "StandardsSupported" )==0 )
	{	
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			int std_idx;
			//fprintf( stderr, "StandardsSupported=0x%x\n", vMA2WC.StandardsSuported );
			strcpy( buf, "" );
			std_idx=0;
			while( std_idx<(sizeof(strAdsl2WanStd)/sizeof(char *)) )
			{
				if( vMA2WC.StandardsSuported & (1<<std_idx) )
				{
					if( (strlen(buf)+strlen(strAdsl2WanStd[std_idx])+1)>=sizeof(buf) )
					{
						fprintf( stderr, "(%s:%d)buf is too small!!\n", __FUNCTION__, __LINE__ );
						break;
					}
					if(buf[0]) strcat( buf, "," );
					strcat( buf, strAdsl2WanStd[std_idx] );
				}
				std_idx++;
			}
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "StandardUsed" )==0 )
	{
		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			int std_idx;
			//fprintf( stderr, "StandardUsed=0x%x\n", vMA2WC.StandardUsed );
			strcpy( buf, "" );
			std_idx=0;
			while( std_idx<(sizeof(strAdsl2WanStd)/sizeof(char *)) )
			{
				if( vMA2WC.StandardUsed & (1<<std_idx) )
				{
					strcpy( buf, strAdsl2WanStd[std_idx] );
					break;
				}
				std_idx++;
			}
			*data = strdup( buf );
		}
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "LineEncoding" )==0 )
	{	
		*data = strdup( "DMT" );
	}else if( strcmp( lastname, "DataPath" )==0 )
	{	
		getAdslInfo(ADSL_GET_LATENCY, buf, 256);
		if( strncmp( buf, "Fast", 4 )==0 )
			*data = strdup( "Fast" );
		else if( strncmp( buf, "Interleave", 10 )==0 )
			*data = strdup( "Interleaved" );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "InterleaveDepth" )==0 )
	{	
		getAdslInfo(ADSL_GET_D_DS, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "UpstreamCurrRate" )==0 )
	{	
		getAdslInfo(ADSL_GET_RATE_US, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "DownstreamCurrRate" )==0 )
	{	
		getAdslInfo(ADSL_GET_RATE_DS, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "UpstreamMaxRate" )==0 )
	{	
		getAdslInfo(ADSL_GET_ATTRATE_US, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "DownstreamMaxRate" )==0 )
	{	
		getAdslInfo(ADSL_GET_ATTRATE_DS, buf, 256);
		*data = uintdup( atoi(buf) );
	}else if( strcmp( lastname, "UpstreamNoiseMargin" )==0 )
	{	
		getAdslInfo(ADSL_GET_SNR_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "DownstreamNoiseMargin" )==0 )
	{	
		getAdslInfo(ADSL_GET_SNR_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "UpstreamAttenuation" )==0 )
	{	
		getAdslInfo(ADSL_GET_LPATT_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "DownstreamAttenuation" )==0 )
	{	
		getAdslInfo(ADSL_GET_LPATT_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "UpstreamPower" )==0 )
	{	
		getAdslInfo(ADSL_GET_POWER_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "DownstreamPower" )==0 )
	{	
		getAdslInfo(ADSL_GET_POWER_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}else if( strcmp( lastname, "ATURVendor" )==0 )
	{
		if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = strdup("");
		else
		{
			snprintf( buf, 256, "%c%c%c%c",
					(vMId.ITU_VendorId.vendorCode>>24) &0xff,
					(vMId.ITU_VendorId.vendorCode>>16) &0xff,
					(vMId.ITU_VendorId.vendorCode>>8) &0xff,
					(vMId.ITU_VendorId.vendorCode) &0xff
					 );
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "ATURCountry" )==0 )
	{	
		if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vMId.ITU_VendorId.countryCode );
	}else if( strcmp( lastname, "ATUCVendor" )==0 )
	{	
		if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = strdup("");
		else
		{
			snprintf( buf, 256, "%c%c%c%c",
					(vMId.ITU_VendorId.vendorCode>>24) &0xff,
					(vMId.ITU_VendorId.vendorCode>>16) &0xff,
					(vMId.ITU_VendorId.vendorCode>>8) &0xff,
					(vMId.ITU_VendorId.vendorCode) &0xff
					 );
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "ATUCCountry" )==0 )
	{	
		if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vMId.ITU_VendorId.countryCode );
	}else if( strcmp( lastname, "TotalStart" )==0 )
	{	
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
	}else if( strcmp( lastname, "ShowtimeStart" )==0 )
	{	
		unsigned int vUint[3];
		if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vUint[0] );
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int setWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if( *i!= 1 ) return ERR_9001;
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
	}else if( strcmp( lastname, "LinkEncapsulationRequested" )==0 )
	{	
		unsigned char *buf=data;
		if( buf==NULL || strlen(buf)==0 ) return ERR_9007;
		//fprintf( stderr, "set %s=%s\n", name, buf );
		if( strcmp( buf, "G.992.3_Annex_K_ATM" )!=0 ) return ERR_9001;
		return 0;
#endif //_SUPPORT_ADSL2WAN_PROFILE_
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //#ifdef CONFIG_DEV_xDSL

#if 1//defined CONFIG_ETHWAN //#ifdef CONFIG_DEV_xDSL
int getWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int bs=0,br=0,ps=0,pr=0;
	char	buf[256]="";
	//struct net_link_info netlink_info;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp(lastname, "Enable")==0 )
	{
	#if 0
		int flags;
		getInFlags("eth1", &flags);
		*data = booldup(flags & IFF_UP);
	#endif	
		*data = booldup(1);
	}
	else if( strcmp(lastname, "Status")==0 )
	{
	#if 0
		int link_status = get_net_link_status("nas0");

		switch(link_status)
		{
			case -1:
				strcpy(buf, "Error");
				break;
			case 0:
				strcpy(buf, "NoLink");
				break;
			case 1:
				strcpy(buf, "Up");
				break;
			default:
				return ERR_9002;
		}
		*data = strdup(buf);
#endif
		*data = strdup("Up");
		
	}
	else if( strcmp(lastname, "MACAddress")==0 )
	{
		//struct in_addr inAddr;
		unsigned char *pMacAddr;
		struct sockaddr hwaddr;
		if(!getInAddr("eth1", HW_ADDR, (void *)&hwaddr))
			return ERR_9002;
		
		pMacAddr = (unsigned char *)hwaddr.sa_data;
		sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
	

		*data = strdup(buf);
#if 0
		unsigned char macadd[MAC_ADDR_LEN];
		MIB_CE_ATM_VC_T *pEntry, tmpentry;

		pEntry = &tmpentry;
		if(!mib_chain_get( MIB_ATM_VC_TBL, 0, (void*)pEntry ))
			return ERR_9002;

#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
		char wanif[IFNAMSIZ];
		int num;
		MEDIA_TYPE_T mType = MEDIA_INDEX(pEntry->ifIndex);

		ifGetName(PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));
	#ifdef CONFIG_RTL_ALIASNAME
		TOKEN_NUM(wanif,&num);
	#else
		if (mType == MEDIA_ETH) //assured that the Wan device is nas0 not vc0
			sscanf(wanif, "nas%u", &num);
		else 	if (mType == MEDIA_ATM) //assured that the Wan device is nas0 not vc0
			sscanf(wanif, "vc%u", &num);
	#endif
#endif

		mib_get(MIB_ELAN_MAC_ADDR, (void *)macadd);
		sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0], macadd[1], macadd[2],
#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
			macadd[3], macadd[4], macadd[5]+(num+1));
#else
			macadd[3], macadd[4], macadd[5]);
#endif
		*data = strdup(buf);
#endif
	}
	else if( strcmp(lastname, "MaxBitRate")==0 ) //get
	{
	#if 0
		if(get_net_link_info("nas0", &netlink_info))
			return ERR_9002;

		sprintf(buf, "%d", netlink_info.speed);
		*data = strdup(buf);
	#endif
		int wanSpeed = 0;
		wanSpeed = getWanSpeed("eth1");	
		//printf("<%s:%d>wanSpeed=%d\n", __FUNCTION__, __LINE__,wanSpeed);
		if(wanSpeed==0)
			*data = strdup("10");
		else if(wanSpeed==1)
			*data = strdup("100");
		else if(wanSpeed==2)
			*data = strdup("1000");
		else
			*data = strdup("Auto");
	}
	else if( strcmp(lastname, "DuplexMode")==0 ) //get
	{
	#if 0
		if(get_net_link_info("nas0", &netlink_info))
			return ERR_9002;

		if(netlink_info.duplex == 0)
			*data = strdup("Half");
		else if(netlink_info.duplex == 1)
			*data = strdup("Full");
		else
			return ERR_9002;
	#else	
		//*data = strdup("Full");
		int mode = getWanDuplex();
		//printf("<%s:%d>mode=%d\n", __FUNCTION__,__LINE__,mode);
#if defined(CONFIG_RTL_8367R)
		if(mode==0)
			*data=strdup("Half");
		else if(mode==1)
			*data=strdup("Full");
#else
		if(mode==0)
			*data=strdup("Full");
		else if(mode==1)
			*data=strdup("Half");
#endif
	#endif
	}
	else
		return ERR_9005;

	return 0;
}

int setWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type, type, &tmpint, &tmpuint, &tmpbool) < 0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
		int entrynum;
//		MIB_CE_ATM_VC_T *pEntry, tmpentry;

//		pEntry = &tmpentry;

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif
		if( i == NULL ) return ERR_9007;
		//printf("<%s:%d> *i=%d\n", __FUNCTION__, __LINE__, *i);

#define NULL_FILE 0
#define NULL_STR ""
		if( *i == 1 )
			//system("ifconfig eth1 up");
			RunSystemCmd(NULL_FILE, "ifconfig", "eth1", "up", NULL_STR);
		else if( *i == 0 )
			//system("ifconfig eth1 down");
			RunSystemCmd(NULL_FILE, "ifconfig", "eth1", "down", NULL_STR);
		else
			return ERR_9003;
	}
	else if( strcmp(lastname, "MaxBitRate")==0 ) //set
	{
#if 0
		return ERR_9001;
#else
		int speedToSet = atoi(data);	
		//printf("<%s:%d>toSet=%d\n", __FUNCTION__, __LINE__, speedToSet);
		setWanSpeed("eth1", speedToSet);
#endif
	}
	else if( strcmp(lastname, "DuplexMode")==0 ) //set
	{
#if 0
		return ERR_9001;
#else
		if(strcmp(data, "Half") == 0)
			setWanDuplex(0);
		else if(strcmp(data, "Full") == 0)	
			setWanDuplex(1);
		else 
			return ERR_9001;
#endif
	}
	else
	{
		return ERR_9005;
	}

	//return ERR_9001;

	return 0;
}

int getWANEthStatsLeaf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int bs=0,br=0,ps=0,pr=0;
	struct user_net_device_stats nds;

#if 1
	//if(getStats((char (* const)[IFNAMSIZ])"eth1", &nds) < 0)
	//if(get_net_device_stats((char (* const)[IFNAMSIZ])"nas0", 1, &nds) < 0)
	if(getStats("eth1", &nds) < 0)
		return ERR_9002;
#else
	diag_printf("WARNING!!!Ignore stats\n");
#endif

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp(lastname, "BytesSent")==0 )
	{
#if 1
		*data = uintdup(nds.tx_bytes);
#else
		*data = uintdup(1);
#endif
	}
	else if( strcmp(lastname, "BytesReceived")==0 )
	{
#if 1
		*data = uintdup(nds.rx_bytes);
#else
		*data = uintdup(1);
#endif
	}
	else if( strcmp(lastname, "PacketsSent")==0 )
	{
#if 1
		*data = uintdup(nds.tx_packets);
#else
		*data = uintdup(1);
#endif
	}
	else if( strcmp(lastname, "PacketsReceived")==0 )
	{
#if 1
		*data = uintdup(nds.rx_packets);
#else
		*data = uintdup(1);
#endif
	}
	else
		return ERR_9005;

	return 0;
}
#endif // #elif defined CONFIG_ETHWAN

int getWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	//struct user_net_device_stats stats;	
	char	*lastname = entity->info->name;
	char	buf[256]="";
#ifdef CONFIG_DEV_xDSL
	unsigned int bs=0,br=0,ps=0,pr=0;
#endif
#if 1//defined CONFIG_ETHWAN
	struct user_net_device_stats nds;
	//struct net_link_info netlink_info;

	//if(getStats((char (* const)[IFNAMSIZ])"eth1", &nds) < 0)
	//if(get_net_device_stats((char (* const)[IFNAMSIZ])"nas0", 1, &nds) < 0)
	if(getStats("eth1", &nds) < 0)
		return ERR_9002;
#else // by cairui
	diag_printf("WARNING!!!Ignore stats\n");
#endif
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "EnabledForInternet" )==0 )
	{
		*data = booldup( 1 );
	}else if( strcmp( lastname, "WANAccessType" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		*data = strdup( "DSL" );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = strdup( "Ethernet" );
#endif
	}else if( strcmp( lastname, "Layer1UpstreamMaxBitRate" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		unsigned int vUint[3];
		if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vUint[2] );
#endif
#if 1//defined CONFIG_ETHWAN
//BRAD DEBUG
		//if(get_net_link_info("eth1", &netlink_info))
		//	return ERR_9002;
		
		//*data = uintdup(netlink_info.speed*1000000);
		//For 100Mbps ethernet card
		*data = uintdup(100000000);
#endif
	}else if( strcmp( lastname, "Layer1DownstreamMaxBitRate" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		unsigned int vUint[3];
		if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vUint[1] );
#endif
#if 1//defined CONFIG_ETHWAN
//BRAD DEBUG
		//if(get_net_link_info("nas0", &netlink_info))
		//	return ERR_9002;
		
		//*data = uintdup(netlink_info.speed*1000000);
		//For 100Mbps ethernet card
		*data = uintdup(100000000);
#endif
	}else if( strcmp( lastname, "PhysicalLinkStatus" )==0 )
	{
#ifdef CONFIG_DEV_xDSL
		getAdslInfo(ADSL_GET_STATE, buf, 256);
		if( strncmp( buf, "HANDSHAKING", 11 )==0 )
			*data = strdup( "Initializing" );
		else if( strncmp( buf, "SHOWTIME", 8 )==0 )
			*data = strdup( "Up" );
		else if( (strncmp( buf, "ACTIVATING", 10 )==0) ||
		         (strncmp( buf, "IDLE", 4 )==0) )
			*data = strdup( "Down" );
		else
			*data = strdup( "Unavailable" );
#endif
#if 1//defined CONFIG_ETHWAN
			*data = strdup( "Up" );
#endif
	}else if( strcmp( lastname, "WANAccessProvider" )==0 )
	{
			*data = strdup( "" );
	}else if( strcmp( lastname, "TotalBytesSent" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		if( get_TotalWANStat( &bs, &br, &ps, &pr )<0 )
			return ERR_9002;
		*data = uintdup( bs );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(nds.tx_bytes);
#else
		*data = uintdup(1);
#endif
	}else if( strcmp( lastname, "TotalBytesReceived" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		if( get_TotalWANStat( &bs, &br, &ps, &pr )<0 )
			return ERR_9002;
		*data = uintdup( br );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(nds.rx_bytes);
#else
		*data = uintdup(1);
#endif
	}else if( strcmp( lastname, "TotalPacketsSent" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		if( get_TotalWANStat( &bs, &br, &ps, &pr )<0 )
			return ERR_9002;
		*data = uintdup( ps );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(nds.tx_packets);
#else
		*data = uintdup(1);
#endif
	}else if( strcmp( lastname, "TotalPacketsReceived" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		if( get_TotalWANStat( &bs, &br, &ps, &pr )<0 )
			return ERR_9002;
		*data = uintdup( pr );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(nds.rx_packets);
#else
		*data = uintdup(1);
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int setWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "EnabledForInternet" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if( *i!= 1 ) return ERR_9001;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getWANDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WANConnectionNumberOfEntries" )==0 )
	{
		int instnum=0;
#if defined(MULTI_WAN_SUPPORT)
		int a = dsafaasd;
		int num,i;
		char inst[20];
		WANIFACE_T *p,wan_entity;

		memset(inst,0,20);
		mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
		for( i=1; i<=num;i++ )
		{
			int j=0;
			p = &wan_entity;
			*((char *)&wan_entity) = (char)i;
			
			if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
			
			while(inst[j]!=0){
				if(p->ConDevInstNum==inst[j])
					break;
				j++;
			}
			if(inst[j]==0){
				instnum++;
				inst[j]=p->ConDevInstNum;
			}
		}
#else
		instnum = 1;
#endif //#if defined(MULTI_WAN_SUPPORT)

		*data = uintdup( instnum ); 
	}else{
		return ERR_9005;
	}
	
	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
/* copy from boa/src/LINUX/fmmgmt.c */
int get_TotalWANStat( unsigned int *bs, unsigned int *br, unsigned int *ps, unsigned int *pr )
{
#ifdef CONFIG_DEV_xDSL
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;

	
	if( bs==NULL || br==NULL || ps==NULL || pr==NULL ) return -1;
	*bs=0; *br=0; *ps=0; *pr=0;
	
	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}
	
	mysio.number = 0;
	
	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return -1;
		}
		
		if (cfg.created == 0)
			continue;

		*bs += cfg.stat.tx_byte_cnt;
		*br += cfg.stat.rx_byte_cnt;
		*ps += cfg.stat.tx_pkt_ok_cnt;
		*pr += cfg.stat.rx_pkt_cnt;
	}
	(void)close(skfd);
	#endif
	return 0;
}


