#include "tr181_dns.h"

#ifdef CONFIG_IPV6
/*******************************************************************************
Device.DNS.
*******************************************************************************/
struct CWMP_OP tDNSLeafOP = { getDNSInfo, NULL };
struct CWMP_PRMT tDNSLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"SupportedRecordTypes",		eCWMP_tSTRING,	CWMP_READ,		&tDNSLeafOP},
};

enum eDNSLeaf
{
	eDNSSupportedRecordTypes
};

struct CWMP_LEAF tDNSLeaf[] =
{
	{ &tDNSLeafInfo[eDNSSupportedRecordTypes]  },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getDNSInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "SupportedRecordTypes" )==0 )
	{
		*data = strdup( "A,AAAA" ); 
	}else
	{
		return ERR_9005;
	}
	
	return 0;
}
#endif // CONFIG_IPV6
