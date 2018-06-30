#include "tr181_dhcpv6.h"

#ifdef CONFIG_IPV6
#define DHCPV6_CLIENT_NUM			1	/* one instance of INTERFACE */


/*******************************************************************************
Device.DHCPv6.
*******************************************************************************/
struct CWMP_OP tDhcpv6LeafOP = { getDhcpv6Info, NULL };
struct CWMP_PRMT tDhcpv6LeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"ClientNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tDhcpv6LeafOP},
};

enum eDhcpv6Leaf
{
	eDhcpv6ClientNumberOfEntries
};

struct CWMP_LEAF tDhcpv6Leaf[] =
{
	{ &tDhcpv6LeafInfo[eDhcpv6ClientNumberOfEntries]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getDhcpv6Info(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ClientNumberOfEntries" )==0 )
	{
		*data = uintdup( DHCPV6_CLIENT_NUM ); 
	}else
	{
		return ERR_9005;
	}
	
	return 0;
}

#endif // CONFIG_IPV6
