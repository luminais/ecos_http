#include "tr181_bridging.h"


#define	MAXBRIDGE		1
#define	MAXDBRIDGE      1
#define	MAXQBRIDGE      1
#define	MAXVLAN         1
#define	MAXFILTER       1
#define	BRIDGENUM       1
#define	FILTERNUM       1

/*******************************************************************************
Device.Bridging
*******************************************************************************/
struct CWMP_OP tBridgingLeafOP = { getBridgingInfo, NULL };
struct CWMP_PRMT tBridgingLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"MaxBridgeEntries",     eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
	{"MaxDBridgeEntries",    eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
//	{"MaxQBridgeEntries",    eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
//	{"MaxVLANEntries",       eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
//	{"MaxFilterEntries",     eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBridgingLeafOP},
	{"BridgeNumberOfEntries",eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
//	{"FilterNumberOfEntries",eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
};

enum eBridgingLeaf
{
	eBrMaxBridgeEntries,
	eBrMaxDBridgeEntries,
//	eBrMaxQBridgeEntries,
//	eBrMaxVLANEntries,
//	eBrMaxFilterEntries,
	eBrBridgeNumberOfEntries
//	eBrFilterNumberOfEntries
};

struct CWMP_LEAF tBridgingLeaf[] =
{
	{ &tBridgingLeafInfo[eBrMaxBridgeEntries] },
	{ &tBridgingLeafInfo[eBrMaxDBridgeEntries] },
//	{ &tBridgingLeafInfo[eBrMaxQBridgeEntries] },
//	{ &tBridgingLeafInfo[eBrMaxVLANEntries] },
//	{ &tBridgingLeafInfo[eBrMaxFilterEntries] },
	{ &tBridgingLeafInfo[eBrBridgeNumberOfEntries] },
//	{ &tBridgingLeafInfo[eBrFilterNumberOfEntries] },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getBridgingInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "MaxBridgeEntries" )==0 )
	{
		*data = uintdup( MAXBRIDGE ); 
	}else if( strcmp( lastname, "MaxDBridgeEntries" )==0 )
	{
		*data = uintdup( MAXDBRIDGE ); 
	}else if( strcmp( lastname, "MaxQBridgeEntries" )==0 )
	{
		*data = uintdup( MAXQBRIDGE ); 
	}else if( strcmp( lastname, "MaxVLANEntries" )==0 )
	{
		*data = uintdup( MAXVLAN ); 
	}else if( strcmp( lastname, "MaxFilterEntries" )==0 )
	{
		*data = uintdup( MAXFILTER ); 
	}else if( strcmp( lastname, "BridgeNumberOfEntries" )==0 )
	{
		*data = uintdup( BRIDGENUM ); 
	}else if( strcmp( lastname, "FilterNumberOfEntries" )==0 )
	{
		*data = uintdup( FILTERNUM ); 
	}else
	{
		return ERR_9005;
	}

	return 0;
}