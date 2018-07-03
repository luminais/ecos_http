#include "tr181_ip.h"


#define IP_INTERFACE_NUM	1	/* one instance of Ip Interface */

/*******************************************************************************
Device.IP.
*******************************************************************************/
struct CWMP_OP tIpLeafOP = { getIpInfo, NULL };
struct CWMP_PRMT tIpLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"IPv4Capable",              eCWMP_tBOOLEAN,	CWMP_READ,	&tIpLeafOP},
//	{"IPv4Enable",               eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpLeafOP},    // from 2.2
	{"IPv4Status",               eCWMP_tSTRING,		CWMP_READ,	&tIpLeafOP},    // from 2.2
	{"IPv6Capable",              eCWMP_tBOOLEAN,	CWMP_READ,	&tIpLeafOP},    // from 2.2
//	{"IPv6Enable",               eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpLeafOP},    // from 2.2
	{"IPv6Status",               eCWMP_tSTRING,		CWMP_READ,	&tIpLeafOP},    // from 2.2
//	{"ULAPrefix",                eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tIpLeafOP},	// from 2.2
	{"InterfaceNumberOfEntries", eCWMP_tUINT,		CWMP_READ,	&tIpLeafOP},
//	{"ActivePortNumberOfEntries",eCWMP_tUINT,		CWMP_READ,	&tIpLeafOP},
};

enum eIpLeaf
{
	eIpIPv4Capable,
//	eIpIPv4Enable,
	eIpIPv4Status,
	eIpIPv6Capable,
//	eIpIPv6Enable,
	eIpIPv6Status,
//	eIpULAPrefix,
	eIpInterfaceNumberOfEntries,
//	eIpActivePortNumberOfEntries
};

struct CWMP_LEAF tIpLeaf[] =
{
	{ &tIpLeafInfo[eIpIPv4Capable] },
//	{ &tIpLeafInfo[eIpIPv4Enable] },
	{ &tIpLeafInfo[eIpIPv4Status] },
	{ &tIpLeafInfo[eIpIPv6Capable] },
//	{ &tIpLeafInfo[eIpIPv6Enable] },
	{ &tIpLeafInfo[eIpIPv6Status] },
//	{ &tIpLeafInfo[eIpULAPrefix] },
	{ &tIpLeafInfo[eIpInterfaceNumberOfEntries] },
//	{ &tIpLeafInfo[eIpActivePortNumberOfEntries] },
	{ NULL	}
};


/*******************************************************************************
Function
*******************************************************************************/
int getIpInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "IPv4Capable" )==0 )
	{
		*data = booldup( 1 ); 
	}else if( strcmp( lastname, "IPv4Status" )==0 )
	{
		*data = strdup("Enabled");
	}else if( strcmp( lastname, "IPv6Capable" )==0 )
	{
#ifdef CONFIG_IPV6
		*data = booldup( 1 ); 
#else
		*data = booldup( 0 ); 
#endif
	}else if( strcmp( lastname, "IPv6Status" )==0 )
	{
#ifdef CONFIG_IPV6
		*data = strdup("Enabled");
#else
		*data = strdup("Disabled");
#endif
	}else if( strcmp( lastname, "InterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup( IP_INTERFACE_NUM ); 
	}else{
		return ERR_9005;
	}
	
	return 0;
}