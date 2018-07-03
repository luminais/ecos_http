#include "prmt_wandsldiagnostics.h"
//#include <config/autoconf.h> keith remove

#define MAX_DSL_TONE		512

enum { 
	DSLDIAG_NONE=0,
	DSLDIAG_REQUESTED,
	DSLDIAG_COMPLETE,
	DSLDIAG_ERROR_INTERNAL,
	DSLDIAG_ERROR_OTHER
};

static char *strDSLDiagState[]=
{
	"None",
	"Requested",
	"Complete",
	"Error_Internal",
	"Error_Other"
};

struct WANDSLDiag
{
	int DiagState;
};
struct WANDSLDiag gWANDSLDiag=
{
	DSLDIAG_NONE,	/*DiagState*/
};
int gStartDSLDiag=0;
void reset_WANDSLDiag( struct WANDSLDiag *p );
extern void cwmpDiagnosticDone();
extern void cwmpSetCpeHold(int holdit);



struct CWMP_OP tWANDSLDIAGLeafOP = { getWANDSLDIAG, setWANDSLDIAG };
struct CWMP_PRMT tWANDSLDIAGLeafInfo[] =
{
	/*(name,			type,		flag,				op)*/
	{"LoopDiagnosticsState",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_DENY_ACT,&tWANDSLDIAGLeafOP},
	{"ACTPSDds",			eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"ACTPSDus",			eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"ACTATPds",			eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"ACTATPus",			eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"HLINSCds",			eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"HLINSCus",			eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	/*HLINGds*/
	/*HLINGus*/
	/*HLOGGds*/
	/*HLOGGus*/
	{"HLOGpsds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"HLOGpsus",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"HLOGMTds",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"HLOGMTus",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"LATNpbds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"LATNpbus",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"SATNds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"SATNus",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"HLINpsds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"HLINpsus",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	/*QLNGds*/
	/*QLNGus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"QLNpsds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"QLNpsus",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"QLNMTds",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"QLNMTus",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	/*SNRGds*/
	/*SNRGus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"SNRpsds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"SNRpsus",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"SNRMTds",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
	{"SNRMTus",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"BITSpsds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP},
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	/*BITSpsus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{"GAINSpsds",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWANDSLDIAGLeafOP}
};

enum eWANDSLDIAGLeaf
{
	eDI_LoopDiagnosticsState,
	eDI_ACTPSDds,
	eDI_ACTPSDus,
	eDI_ACTATPds,
	eDI_ACTATPus,
	eDI_HLINSCds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_HLINSCus,
	/*HLINGds*/
	/*HLINGus*/
	/*HLOGGds*/
	/*HLOGGus*/
	eDI_HLOGpsds,
	eDI_HLOGpsus,
	eDI_HLOGMTds,
	eDI_HLOGMTus,
	eDI_LATNpbds,
	eDI_LATNpbus,
	eDI_SATNds,
	eDI_SATNus,
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_HLINpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_HLINpsus,
	/*QLNGds*/
	/*QLNGus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_QLNpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_QLNpsus,
	eDI_QLNMTds,
	eDI_QLNMTus,
	/*SNRGds*/
	/*SNRGus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_SNRpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_SNRpsus,
	eDI_SNRMTds,
	eDI_SNRMTus,
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_BITSpsds,
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	/*BITSpsus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	eDI_GAINSpsds
};

struct CWMP_LEAF tWANDSLDIAGLeaf[] =
{
	{ &tWANDSLDIAGLeafInfo[eDI_LoopDiagnosticsState] },
	{ &tWANDSLDIAGLeafInfo[eDI_ACTPSDds] },
	{ &tWANDSLDIAGLeafInfo[eDI_ACTPSDus] },
	{ &tWANDSLDIAGLeafInfo[eDI_ACTATPds] },
	{ &tWANDSLDIAGLeafInfo[eDI_ACTATPus] },
	{ &tWANDSLDIAGLeafInfo[eDI_HLINSCds] },
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_HLINSCus] },
	/*HLINGds*/
	/*HLINGus*/
	/*HLOGGds*/
	/*HLOGGus*/
	{ &tWANDSLDIAGLeafInfo[eDI_HLOGpsds] },
	{ &tWANDSLDIAGLeafInfo[eDI_HLOGpsus] },
	{ &tWANDSLDIAGLeafInfo[eDI_HLOGMTds] },
	{ &tWANDSLDIAGLeafInfo[eDI_HLOGMTus] },
	{ &tWANDSLDIAGLeafInfo[eDI_LATNpbds] },
	{ &tWANDSLDIAGLeafInfo[eDI_LATNpbus] },
	{ &tWANDSLDIAGLeafInfo[eDI_SATNds] },
	{ &tWANDSLDIAGLeafInfo[eDI_SATNus] },
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_HLINpsds] },
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_HLINpsus] },
	/*QLNGds*/
	/*QLNGus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_QLNpsds] },
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_QLNpsus] },
	{ &tWANDSLDIAGLeafInfo[eDI_QLNMTds] },
	{ &tWANDSLDIAGLeafInfo[eDI_QLNMTus] },
	/*SNRGds*/
	/*SNRGus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_SNRpsds] },
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_SNRpsus] },
	{ &tWANDSLDIAGLeafInfo[eDI_SNRMTds] },
	{ &tWANDSLDIAGLeafInfo[eDI_SNRMTus] },
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_BITSpsds] },
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	/*BITSpsus*/
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	{ &tWANDSLDIAGLeafInfo[eDI_GAINSpsds] },
	{ NULL }
};

int getWANDSLDIAG(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	char		*pChar=NULL;
	int		vINT=0;
	unsigned int	vUINT=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LoopDiagnosticsState" )==0 )
	{
		*data = strdup( strDSLDiagState[gWANDSLDiag.DiagState] );
	}else if( strcmp( lastname, "ACTPSDds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_ACTPSDds, &vINT )<0 )
			*data = intdup( 0 );
		else
			*data = intdup( vINT );
	}else if( strcmp( lastname, "ACTPSDus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_ACTPSDus, &vINT )<0 )
			*data = intdup( 0 );
		else
			*data = intdup( vINT );
	}else if( strcmp( lastname, "ACTATPds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_ACTATPds, &vINT )<0 )
			*data = intdup( 0 );
		else
			*data = intdup( vINT );
	}else if( strcmp( lastname, "ACTATPus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_ACTATPus, &vINT )<0 )
			*data = intdup( 0 );
		else
			*data = intdup( vINT );
	}else if( strcmp( lastname, "HLINSCds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_HLINSCds, &vINT )<0 )
			*data = intdup( 0 );
		else
			*data = intdup( vINT );
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "HLINSCus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_HLINSCus, &vINT )<0 )
			*data = intdup( 0 );
		else
			*data = intdup( vINT );
	}else if( strcmp( lastname, "HLOGpsds" )==0 )
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
			*data = uintdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "HLOGMTus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_HLOGMTus, &vUINT )<0 )
			*data = uintdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "LATNpbds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_LATNds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "LATNpbus" )==0 )
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
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "HLINpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_HLINpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "HLINpsus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_HLINpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "QLNpsds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_QLNpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "QLNpsus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_QLNpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "QLNMTds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_QLNMTds, &vUINT )<0 )
			*data = uintdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "QLNMTus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_QLNMTus, &vUINT )<0 )
			*data = uintdup( 0 );
		else
			*data = uintdup( vUINT );
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "SNRpsds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_SNRpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "SNRpsus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_SNRpsus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "SNRMTds" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_SNRMTds, &vUINT )<0 )
			*data = uintdup( 0 );
		else
			*data = uintdup( vUINT );
	}else if( strcmp( lastname, "SNRMTus" )==0 )
	{	
		if( getDSLParameterValue( GET_DSL_SNRMTus, &vUINT )<0 )
			*data = uintdup( 0 );
		else
			*data = uintdup( vUINT );
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_
	}else if( strcmp( lastname, "BITSpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_BITSpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else if( strcmp( lastname, "GAINSpsds" )==0 )
	{
		if( getDSLParameterValue( GET_DSL_GAINSpsds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setWANDSLDIAG(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "LoopDiagnosticsState" )==0 )
	{
		char *buf=data;		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, strDSLDiagState[DSLDIAG_REQUESTED] )!=0 ) return ERR_9007;
		if( gWANDSLDiag.DiagState==DSLDIAG_REQUESTED ) return ERR_9001;
		
		reset_WANDSLDiag( &gWANDSLDiag );
		gStartDSLDiag=1;
		gWANDSLDiag.DiagState = DSLDIAG_REQUESTED;
	}else{
		return ERR_9005;
	}
	
	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
/*refer to  boa/src/LINUX/fmmgmt.c*/
void reset_WANDSLDiag( struct WANDSLDiag *p )
{
	p->DiagState = DSLDIAG_NONE;
}

int UShortArray2Str( unsigned short a[], int asize, char *buf, int bsize )
{
	int i;
	char tmp[16];
	
	if( a==NULL || buf==NULL) return -1;
	
	buf[0]=0;
	for( i=0;i<asize;i++ )
	{
		if(i==0)
			sprintf( tmp, "%u", a[i]  );
		else
			sprintf( tmp, ",%u", a[i]  );
			
		if( strlen(buf)+strlen(tmp) >= bsize ) break;		
		strcat( buf, tmp );
	}
	return 0;
}

int CharArray2Str( char *a, int asize, char *buf, int bsize )
{
	int i;
	char tmp[16];
	
	if( a==NULL || buf==NULL) return -1;
	
	buf[0]=0;
	for( i=0;i<asize;i++ )
	{
		if(i==0)
			sprintf( tmp, "%u", a[i]  );
		else
			sprintf( tmp, ",%u", a[i]  );
			
		if( strlen(buf)+strlen(tmp) >= bsize ) break;		
		strcat( buf, tmp );
	}
	return 0;
}

int ShortArray2StrForQLN( short a[], int asize, char *buf, int bsize, int IsUS, int NSCus )
{
	int i;
	char tmp[16];
	
	if( a==NULL || buf==NULL) return -1;
	
	buf[0]=0;
	if(IsUS)
	{
		for( i=0;i<NSCus;i++ )
		{
			if(i==0)
				sprintf( tmp, "%u", ((-230-a[i])/5) & 0xff  );
			else
				sprintf( tmp, ",%u", ((-230-a[i])/5) & 0xff  );
				
			if( strlen(buf)+strlen(tmp) >= bsize ) break;			
			strcat( buf, tmp );
		}
	}
	else
	{
		for( i=NSCus;i<asize;i++ )
		{
			if(i==NSCus)
				sprintf( tmp, "%u", ((-230-a[i])/5) & 0xff  );
			else
				sprintf( tmp, ",%u", ((-230-a[i])/5) & 0xff  );
				
			if( strlen(buf)+strlen(tmp) >= bsize ) break;			
			strcat( buf, tmp );
		}
	}
	return 0;
}

int ShortArray2StrForSNR( short a[], int asize, char *buf, int bsize, int IsUS, int NSCus )
{
	int i;
	char tmp[16];
	
	if( a==NULL || buf==NULL) return -1;
	
	buf[0]=0;
	if(IsUS)
	{
		for( i=0;i<NSCus;i++ )
		{
			if(i==0)
				sprintf( tmp, "%u", ((320-a[i])/5) & 0xff  );
			else
				sprintf( tmp, ",%u", ((320-a[i])/5) & 0xff  );
				
			if( strlen(buf)+strlen(tmp) >= bsize ) break;
			strcat( buf, tmp );
		}
	}
	else
	{
		for( i=NSCus;i<asize;i++ )
		{
			if(i==NSCus)
				sprintf( tmp, "%u", ((320-a[i])/5) & 0xff  );
			else
				sprintf( tmp, ",%u", ((320-a[i])/5) & 0xff  );
				
			if( strlen(buf)+strlen(tmp) >= bsize ) break;			
			strcat( buf, tmp );
		}
	}
	return 0;
}

int ShortArray2StrForHLOG( short a[], int asize,char *buf, int bsize, int IsUS, int NSCus  )
{
	int i;
	char tmp[16];
	int f1=0;
	if( a==NULL || buf==NULL) return -1;

	
	buf[0]=0;
	if(IsUS)
	{
		for( i=0;i<NSCus;i++ )
		{
			if(i==0)
			{
				f1= 60-a[i];
				sprintf( tmp, "%d",(int)f1);
			}else{

				f1= 60-a[i];
				sprintf( tmp, ",%d",(int)f1);
			}	
			if( strlen(buf)+strlen(tmp) >= bsize ) break;			
			strcat( buf, tmp );
		}
	}
	else
	{
		for( i=NSCus;i<asize;i++ )
		{
			if(i==NSCus)
			{
				f1= 60-a[i];
				sprintf( tmp, "%d",(int)f1);
			}else{
				f1= 60-a[i];
				sprintf( tmp, ",%d",(int)f1);
			}	
			if( strlen(buf)+strlen(tmp) >= bsize ) break;			
			strcat( buf, tmp );
		}
	}
	return 0;
}

int ShortArray2StrForHLIN( short a[], int asize, int hlinscds, char *buf, int bsize, int IsUS, int NSCus  )
{
	int i;
	char tmp[32];
	float f1=0,f2=0;
	if( a==NULL || buf==NULL || hlinscds==0 ) return -1;
	int const2 = 0x40000000;// 2^30
	
	buf[0]=0;
	if(IsUS)
	{
		for( i=0;i<NSCus;i++ )
		{
			if(i==0)
			{
				f1= (float)a[i+asize]*const2/1000/hlinscds;
				f2= (float)a[i+asize*2]*const2/1000/hlinscds;
				sprintf( tmp, "%d,%d", (int)f1, (int)f2  );
			}else{
				f1= (float)a[i+asize]*const2/1000/hlinscds;
				f2= (float)a[i+asize*2]*const2/1000/hlinscds;
				sprintf( tmp, ",%d,%d", (int)f1, (int)f2  );
			}	
			if( strlen(buf)+strlen(tmp) >= bsize ) break;			
			strcat( buf, tmp );
		}
	}
	else
	{
		for( i=NSCus;i<asize;i++ )
		{
			if(i==NSCus)
			{
				f1= (float)a[i+asize]*const2/1000/hlinscds;
				f2= (float)a[i+asize*2]*const2/1000/hlinscds;
				sprintf( tmp, "%d,%d", (int)f1, (int)f2  );
			}else{
				f1= (float)a[i+asize]*const2/1000/hlinscds;
				f2= (float)a[i+asize*2]*const2/1000/hlinscds;
				sprintf( tmp, ",%d,%d", (int)f1, (int)f2  );
			}	
			if( strlen(buf)+strlen(tmp) >= bsize ) break;
			strcat( buf, tmp );
		}
	}
	return 0;
}

union _DSLSTRUCT_{
	short	hlog[MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE]; //for hlog = malloc(sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE));
	short	qln[MAX_DSL_TONE]; //for qln = malloc(sizeof(short)*MAX_DSL_TONE);
	short	snr[MAX_DSL_TONE]; //for snr = malloc(sizeof(short)*MAX_DSL_TONE);
	short	bits[MAX_DSL_TONE]; //for bits = malloc(sizeof(short)*MAX_DSL_TONE);
	unsigned short gains[MAX_DSL_TONE+1]; //for gains = malloc(sizeof(unsigned short)*(MAX_DSL_TONE+1) );
};
static union _DSLSTRUCT_ gDSLStruct;
static char gDSLBuf[8192];

int getDSLParameterValue( unsigned int name_idx, void *pvalue )
{
	char	**ppChar=pvalue;
	int	vINT, *pINT=pvalue;
	unsigned int vUINT, *pUINT=pvalue;
	int	vPSD[2];
	int 	chan=256;
	char 	xdsl_mode;
		
	if((pvalue==NULL) || (name_idx>=DSL_END)) return -1;	
	if( adsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&xdsl_mode, 1)==0 )
	{
		return -1;
	}else{
		xdsl_mode&=0x1f;
		if (xdsl_mode < 5) //adsl1/adsl2
			chan = 256;
		else
			chan = 512;
	}

	switch(name_idx)
	{
	case GET_DSL_ACTPSDds://int
		{
			if(adsl_drv_get(RLCM_GET_DSL_PSD, (void *)vPSD, TR069_DSL_PSD_SIZE)==0)
				*pINT=0;
			else
				*pINT=vPSD[0];
		}
		break;
	case GET_DSL_ACTPSDus://int
		{
			if(adsl_drv_get(RLCM_GET_DSL_PSD, (void *)vPSD, TR069_DSL_PSD_SIZE)==0)
				*pINT=0;
			else
				*pINT=vPSD[1];
		}
		break;
	case GET_DSL_ACTATPds://int
		{
			if(adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				*pINT=0;
			else
				*pINT=gDSLStruct.hlog[chan*3+11];
		}
		break;
	case GET_DSL_ACTATPus://int
		{
			if(adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				*pINT=0;
			else
				*pINT=gDSLStruct.hlog[chan*3+10];
		}
		break;
/*HLIN*************************/
	case GET_DSL_HLINpsds://string
		{
			strcpy(gDSLBuf, "");
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				int vHlinscds=(unsigned short)gDSLStruct.hlog[chan*3+1];
				int vNscus=gDSLStruct.hlog[chan*3+12];
				ShortArray2StrForHLIN( gDSLStruct.hlog, chan, vHlinscds, gDSLBuf, sizeof(gDSLBuf), 0, vNscus);
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_HLINpsus://string
		{
			strcpy(gDSLBuf, "");
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				//pass vHlinscds, not vHlinscus, lupin
				//int vHlinscus=(unsigned short)gDSLStruct.hlog[chan*3];
				int vHlinscds=(unsigned short)gDSLStruct.hlog[chan*3+1];
				int vNscus=gDSLStruct.hlog[chan*3+12];
				//ShortArray2StrForHLIN( gDSLStruct.hlog, chan, vHlinsccs, gDSLBuf, sizeof(gDSLBuf), 1, vNscus);
				ShortArray2StrForHLIN( gDSLStruct.hlog, chan, vHlinscds, gDSLBuf, sizeof(gDSLBuf), 1, vNscus);
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_HLINSCds://int
		{
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				*pINT=0;
			else
				*pINT=(unsigned short)gDSLStruct.hlog[chan*3+1];
		}
		break;
	case GET_DSL_HLINSCus://int
		{
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				*pINT=0;
			else
				*pINT=(unsigned short)gDSLStruct.hlog[chan*3];
		}
		break;
/*END HLIN*************************/
/*HLOG*****************************/
	case GET_DSL_HLOGpsds://string
		{
			strcpy(gDSLBuf, "");
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				int vNscus=gDSLStruct.hlog[chan*3+12];
				ShortArray2StrForHLOG( gDSLStruct.hlog, chan, gDSLBuf, sizeof(gDSLBuf), 0, vNscus);
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_HLOGpsus://string
		{
			strcpy(gDSLBuf, "");
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				int vNscus=gDSLStruct.hlog[chan*3+12];
				ShortArray2StrForHLOG( gDSLStruct.hlog, chan, gDSLBuf, sizeof(gDSLBuf), 1, vNscus);
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_HLOGMTds://uint
		{
			*pUINT=256;
		}
		break;
	case GET_DSL_HLOGMTus://uint
		{
			*pUINT=256;
		}
		break;
/*END HLOG*************************/
/*QLN******************************/
	case GET_DSL_QLNpsds://string
		{
			strcpy(gDSLBuf, "");
			if(adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				int vNscus=gDSLStruct.hlog[chan*3+12];
				if(adsl_drv_get(RLCM_GET_DIAG_QLN, (void *)gDSLStruct.qln, sizeof(gDSLStruct.qln)))
					ShortArray2StrForQLN( gDSLStruct.qln, chan, gDSLBuf, sizeof(gDSLBuf), 0, vNscus );				
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_QLNpsus://string
		{
			strcpy(gDSLBuf, "");
			if(adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				int vNscus=gDSLStruct.hlog[chan*3+12];
				if(adsl_drv_get(RLCM_GET_DIAG_QLN, (void *)gDSLStruct.qln, sizeof(gDSLStruct.qln)))
					ShortArray2StrForQLN( gDSLStruct.qln, chan, gDSLBuf, sizeof(gDSLBuf), 1, vNscus );				
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_QLNMTds://uint
		{
			*pUINT=128;
		}
		break;
	case GET_DSL_QLNMTus://uint
		{
			*pUINT=128;
		}
		break;
/*END QLN*************************/
/*SNR*****************************/
	case GET_DSL_SNRpsds://string
		{
			strcpy(gDSLBuf, "");
			if(adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				int vNscus=gDSLStruct.hlog[chan*3+12];
				if(adsl_drv_get(RLCM_GET_DIAG_SNR, (void *)gDSLStruct.snr, sizeof(gDSLStruct.snr)))
					ShortArray2StrForSNR( gDSLStruct.snr, chan, gDSLBuf, sizeof(gDSLBuf), 0, vNscus );
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_SNRpsus://string
		{
			strcpy(gDSLBuf, "");
			if(adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog)))
			{
				int vNscus=gDSLStruct.hlog[chan*3+12];
				if(adsl_drv_get(RLCM_GET_DIAG_SNR, (void *)gDSLStruct.snr, sizeof(gDSLStruct.snr)))
					ShortArray2StrForSNR( gDSLStruct.snr, chan, gDSLBuf, sizeof(gDSLBuf), 1, vNscus );
			}
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_SNRMTds://uint
		{
			*pUINT=512;
		}
		break;
	case GET_DSL_SNRMTus://uint
		{
			*pUINT=512;
		}
		break;
/*END SNR*************************/
	case GET_DSL_LATNds://string
		{
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				vINT=0;
			else
				vINT=(unsigned short)gDSLStruct.hlog[chan*3+3];
			sprintf( gDSLBuf, "%d", vINT );
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_LATNus://string
		{
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				vINT=0;
			else
				vINT=(unsigned short)gDSLStruct.hlog[chan*3+2];
			sprintf( gDSLBuf, "%d", vINT );
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_SATNds://string
		{
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				vINT=0;
			else
				vINT=(unsigned short)gDSLStruct.hlog[chan*3+5];
			sprintf( gDSLBuf, "%d", vINT );
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_SATNus://string
		{
			if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)gDSLStruct.hlog, sizeof(gDSLStruct.hlog))==0 )
				vINT=0;
			else
				vINT=(unsigned short)gDSLStruct.hlog[chan*3+4];
			sprintf( gDSLBuf, "%d", vINT );
			*ppChar=gDSLBuf;
		}
		break;	
	case GET_DSL_BITSpsds://string
		{
			if(adsl_drv_get(RLCM_GET_CHANNEL_BITLOAD, (void *)gDSLStruct.bits, sizeof(gDSLStruct.bits) )==0)
				strcpy(gDSLBuf, "");
			else
				CharArray2Str( gDSLStruct.bits, chan, gDSLBuf, sizeof(gDSLBuf) );
			*ppChar=gDSLBuf;
		}
		break;
	case GET_DSL_GAINSpsds://string
		{
			if(adsl_drv_get(RLCM_GET_DSL_GI, (void *)gDSLStruct.gains, sizeof(gDSLStruct.gains))==0)
				strcpy(gDSLBuf, "");
			else
				UShortArray2Str( &gDSLStruct.gains[1], chan, gDSLBuf, sizeof(gDSLBuf) );
			*ppChar=gDSLBuf;
		}
		break;		
	default:
		return -1;
	}
	return 0;
}

/***************************************************************/
#ifdef CONFIG_MIDDLEWARE
extern int sendOpertionDoneMsg2MidIntf(char opertion);
#endif

void *WANDSLDiag_thread(void *data)
{
	int mode;
	time_t tStart;
	int ldstate=0;
	Modem_LinkSpeed vLs;
	int is_timeout;
	
	CWMPDBG( 0, ( stderr, "<%s:%d>Start WANDSLDiagnostics\n", __FUNCTION__, __LINE__ ) );

	//start diagnostics
#ifdef _USE_NEW_IOCTL_FOR_DSLDIAG_
	fprintf( stderr, "use RLCM_ENABLE_DIAGNOSTIC to start dsldiag\n" );
	mode=0;
	adsl_drv_get(RLCM_ENABLE_DIAGNOSTIC, (void *)&mode, sizeof(int));//Lupin
#else
	mode = 41;
	adsl_drv_get(RLCM_DEBUG_MODE, (void *)&mode, sizeof(int));
#endif
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);

#if 1
	//wait 3 mins???? or get the diagnostics status???
	tStart = time(NULL);
	while( tStart+180 > time(NULL) )   sleep(5);
#else
	//wait 3 mins???? or get the diagnostics status???
	tStart = time(NULL);
	is_timeout=0;
	//copy from climenu.c:startADSLDiagnostic()
	//wait until showtime
	while(1)
	{
		//(!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		if(adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE))
		{
			if(vLs.upstreamRate!=0) break;
		}
		
		usleep(1000000);
		if(tStart+180 > time(NULL))//timeout
		{
			is_timeout=1;
			break;
		}
	}

	//wait the ip connection is ready
	tStart = time(NULL);
	while( tStart+15 > time(NULL) )   sleep(5);
#endif 

	//get result
	adsl_drv_get(RLCM_GET_LD_STATE, (void *)&ldstate, 4);
	if (ldstate != 0) //successful
		gWANDSLDiag.DiagState = DSLDIAG_COMPLETE;
	else
		gWANDSLDiag.DiagState = DSLDIAG_ERROR_INTERNAL;

	CWMPDBG( 0, ( stderr, "<%s:%d>Finished WANDSLDiagnostics ( %s )\n", __FUNCTION__, __LINE__, ldstate==0?"Failure":"Success" ) );

#ifdef CONFIG_MIDDLEWARE
	sendOpertionDoneMsg2MidIntf('3');
#endif
	//send event to the acs
	cwmpDiagnosticDone();
	cwmpSetCpeHold(0);
	return NULL;	
}

void cwmpStartDSLDiag(void)
{
	pthread_t pid;
	
	aaaaaaa;
	if( pthread_create( &pid, NULL, WANDSLDiag_thread ,NULL  ) )
	{
		gWANDSLDiag.DiagState = DSLDIAG_ERROR_INTERNAL;
		cwmpDiagnosticDone();
		cwmpSetCpeHold(0);
		return;
	}
	pthread_detach(pid);
		
	return;
}
/***************************************************************/
