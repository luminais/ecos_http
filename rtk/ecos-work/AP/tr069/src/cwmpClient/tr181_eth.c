#include "tr181_eth.h"


/*******************************************************************************
DEVICE.Ethernet
*******************************************************************************/
struct CWMP_OP tEthLeafOP = { getEthInfo, NULL };
struct CWMP_PRMT tEthLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"InterfaceNumberOfEntries",				eCWMP_tUINT,	CWMP_READ,		&tEthLeafOP},
	{"LinkNumberOfEntries",				eCWMP_tUINT,	CWMP_READ,		&tEthLeafOP},
};

enum eEthLeaf
{
	eEthInterfaceNumberOfEntries,
	eEthLinkNumberOfEntries
};

struct CWMP_LEAF tEthLeaf[] =
{
	{ &tEthLeafInfo[eEthInterfaceNumberOfEntries]  },
	{ &tEthLeafInfo[eEthLinkNumberOfEntries]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getEthInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "InterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup( ETHERNET_NUM ); 
	}
	else if( strcmp( lastname, "LinkNumberOfEntries" )==0 )
	{
		*data = uintdup( LINK_NUM ); 
	}else
	{
		return ERR_9005;
	}
	
	return 0;
}