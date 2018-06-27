#include "tr181_dhcpv4.h"


#define DHCPV4_CLIENT_NUM			1	/* one instance of INTERFACE */


/*******************************************************************************
Device.DHCPv4.
*******************************************************************************/
struct CWMP_OP tDhcpv4LeafOP = { getDhcpv4Info, NULL };
struct CWMP_PRMT tDhcpv4LeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"ClientNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tDhcpv4LeafOP},
};

enum eDhcpv4Leaf
{
	eDhcpv4ClientNumberOfEntries
};

struct CWMP_LEAF tDhcpv4Leaf[] =
{
	{ &tDhcpv4LeafInfo[eDhcpv4ClientNumberOfEntries]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getDhcpv4Info(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ClientNumberOfEntries" )==0 )
	{
		*data = uintdup( DHCPV4_CLIENT_NUM ); 
	}else
	{
		return ERR_9005;
	}
	
	return 0;
}