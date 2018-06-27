#include "tr181_MoCA.h"


#define MOCA_INTERFACE_NUM			1	/* one instance of INTERFACE */


/*******************************************************************************
DEVICE.MoCA
*******************************************************************************/
struct CWMP_OP tMoCALeafOP = { getMoCAInfo, NULL };
struct CWMP_PRMT tMoCALeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"InterfaceNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tMoCALeafOP},
};

enum eMoCALeaf
{
	eMoCAInterfaceNumberOfEntries
};

struct CWMP_LEAF tMoCALeaf[] =
{
	{ &tMoCALeafInfo[eMoCAInterfaceNumberOfEntries]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getMoCAInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "InterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup( MOCA_INTERFACE_NUM ); 
	}else
	{
		return ERR_9005;
	}
	
	return 0;
}