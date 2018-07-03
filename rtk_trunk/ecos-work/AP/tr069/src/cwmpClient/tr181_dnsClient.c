#include "tr181_dnsClient.h"

#ifdef CONFIG_IPV6
/*******************************************************************************
Device.DNS.Client.Server.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDNSClientServerEntityLeafOP = { getDNSClientServerEntity, setDNSClientServerEntity };
struct CWMP_PRMT tDNSClientServerEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",  	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDNSClientServerEntityLeafOP},
	{"Status",   	eCWMP_tSTRING,	CWMP_READ,	&tDNSClientServerEntityLeafOP},
	{"DNSServer",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDNSClientServerEntityLeafOP},
//	{"Interface",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDNSClientServerEntityLeafOP},
	{"Type",		eCWMP_tSTRING,	CWMP_READ,	&tDNSClientServerEntityLeafOP},
};
enum eDNSClientServerEntityLeaf
{
	eDNSClientServerEnable,
	eDNSClientServerStatus,
	eDNSClientServerDNSServer,
//	eDNSClientServerInterface,
	eDNSClientServerType
};
struct CWMP_LEAF tDNSClientServerEntityLeaf[] =
{
	{ &tDNSClientServerEntityLeafInfo[eDNSClientServerEnable] },
	{ &tDNSClientServerEntityLeafInfo[eDNSClientServerStatus] },
	{ &tDNSClientServerEntityLeafInfo[eDNSClientServerDNSServer] },
//	{ &tDNSClientServerEntityLeafInfo[eDNSClientServerInterface] },
	{ &tDNSClientServerEntityLeafInfo[eDNSClientServerType] },
	{ NULL }
};

/*******************************************************************************
Device.DNS.Client.Server.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDNSClientServerOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eLinkDNSClientServerOjbect
{
	eLinkDNSClientServer0
};
struct CWMP_LINKNODE tLinkDNSClientSentObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkDNSClientServerOjbectInfo[eLinkDNSClientServer0],	tDNSClientServerEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.DNS.Client.Server.
*******************************************************************************/
struct CWMP_OP tDNSClientServer_OP = { NULL, objDNSClientServer };

struct CWMP_PRMT tDNSClientServerObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Server",	eCWMP_tOBJECT,	CWMP_READ,	&tDNSClientServer_OP},
};

enum eDNSClientServerObject
{
	eDNSClientServer
};

struct CWMP_NODE tDNSClientServerObject[] =
{
	/*info,  					leaf,			node)*/
	{&tDNSClientServerObjectInfo[eDNSClientServer],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.DNS.Client. Entity
*******************************************************************************/
struct CWMP_OP tDNSClientEntityLeafOP = { getDNSClientEntity, setDNSClientEntity };
struct CWMP_PRMT tDNSClientEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                       eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDNSClientEntityLeafOP},
	{"Status",                       eCWMP_tSTRING,	CWMP_READ,	&tDNSClientEntityLeafOP},
	{"ServerNumberOfEntries",        eCWMP_tUINT,	CWMP_READ,	&tDNSClientEntityLeafOP},
};
enum eDNSClientEntityLeaf
{
	eDNSClientEnable,
	eDNSClientStatus,
	eDNSClientServerNumberOfEntries
};
struct CWMP_LEAF tDNSClientEntityLeaf[] =
{
	{ &tDNSClientEntityLeafInfo[eDNSClientEnable] },
	{ &tDNSClientEntityLeafInfo[eDNSClientStatus] },
	{ &tDNSClientEntityLeafInfo[eDNSClientServerNumberOfEntries] },
	{ NULL }
};


/*******************************************************************************
Device.DNS.Client.
*******************************************************************************/
struct CWMP_PRMT tDNSClientObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Client",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDNSClientObject
{
	eDNSClient,
};

struct CWMP_NODE tDNSClientObject[] =
{
	/*info,  					leaf,			node)*/
	{&tDNSClientObjectInfo[eDNSClient],	tDNSClientEntityLeaf,	tDNSClientServerObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int objDNSClientServer(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			*c=NULL;

			if(DNS_CLIENT_SERVER_NUM>0)
				return create_Object( c, tLinkDNSClientSentObject, sizeof(tLinkDNSClientSentObject), DNS_CLIENT_SERVER_NUM, 1 );

			return 0;
			break;
		}
		case eCWMP_tUPDATEOBJ:
		{
			unsigned int num,i;
			struct CWMP_LINKNODE *old_table;

			num = DNS_CLIENT_SERVER_NUM;
			old_table = (struct CWMP_LINKNODE*)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				remove_entity = remove_SiblingEntity( &old_table, i+1 );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
				}
				else
				{ 
					unsigned int InstNum=i+1;
					add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tLinkDNSClientSentObject, sizeof(tLinkDNSClientSentObject), &InstNum );
				}
			}
			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE*)old_table );
			return 0;
			break;
		}
	}

	return -1;
}

int getDNSClientEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int uInt;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_DNS_CLIENT_ENABLE,(void*)&uInt);
		
		*data = booldup(uInt);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		mib_get(MIB_DNS_CLIENT_ENABLE,(void*)&uInt);

		if (uInt == 1)
			*data = strdup("Enabled");
		else
			*data = strdup("Disabled");
	}
	else if( strcmp( lastname, "ServerNumberOfEntries" )==0 )
	{
		*data = uintdup(DNS_CLIENT_SERVER_NUM);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDNSClientEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf = data;
	int		*pIntValue;
	int		intValue;
	unsigned int *pUintValue;
	unsigned int uintValue;
	int isMIBUpdated = 1;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	if( strcmp( lastname, "Enable" )==0 )
	{
		pUintValue = (unsigned int *)data;
		CHECK_PARAM_NUM(*pUintValue, 0, 1);
		uintValue = (*pUintValue == 0) ? 0 : 1;
		CONFIG_SET(MIB_DNS_CLIENT_ENABLE, &uintValue);	
	}
	else
	{
		return ERR_9005;
	}

	if(isMIBUpdated)
		return 1;
	else
		return 0;
}

int getDNSClientServerEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	unsigned int sndInst;
	DNS_CLIENT_SERVER_T entry={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	sndInst = getInstanceNum(name, "Server");

	//tr181_printf("sndInst %d", sndInst);

	if( sndInst<1 || sndInst>DNS_CLIENT_SERVER_NUM ) return ERR_9007;

	*((char*)&entry) = (char)sndInst;

	if(mib_get(MIB_DNS_CLIENT_SERVER_TBL,(void*)&entry)==0)
	{
		printf("get MIB_DNS_CLIENT_SERVER_TBL fail!\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(entry.enable);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		if(entry.status == 1)
			*data = strdup("Enable");
		else
			*data = strdup("Disable");
	}
	else if( strcmp( lastname, "DNSServer" )==0 )
	{
		*data = strdup(entry.ipAddr);
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		if(entry.type == 1)
			*data = strdup("DHCPv4");
		else if(entry.type == 2)
			*data = strdup("DHCPv6");
		else if(entry.type == 3)
			*data = strdup("RouterAdvertisement");
		else if(entry.type == 4)
			*data = strdup("IPCP");
		else if(entry.type == 5)
			*data = strdup("Static");
		else
			*data = strdup("");
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDNSClientServerEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf = data;
	int isMIBUpdated = 1;
	int maxNum=DNS_CLIENT_SERVER_NUM;
	unsigned int sndInst;
	DNS_CLIENT_SERVER_T entry[2]={0};

	sndInst = getInstanceNum(name, "Server");

	//tr181_printf("sndInst %d", sndInst);

	if( sndInst<1 || sndInst>DNS_CLIENT_SERVER_NUM ) return ERR_9007;

	mib_set(MIB_DNS_CLIENT_SERVER_TBL_NUM,(void*)&maxNum);

	*((char*)&entry) = (char)sndInst;

	if(mib_get(MIB_DNS_CLIENT_SERVER_TBL,(void*)&entry)==0)
	{
		tr181_printf("get MIB_DNS_CLIENT_SERVER_TBL fail!\n");
		return ERR_9002;
	}

	entry[1]=entry[0];

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned int uintValue;

		uintValue = *((unsigned int *)data);
		entry[1].enable=uintValue;	
	}
	else if( strcmp( lastname, "DNSServer" )==0 )
	{
		if (entry[1].type == 5) // Static
		{
			struct in_addr ia_val;
#ifdef CONFIG_IPV6
			addr6CfgParam_t addr6_dns;
			addr6_dns.prefix_len = 64;
#endif

			if ( inet_aton(buf, &ia_val) == 0 
#ifdef CONFIG_IPV6
				&& inet_pton6(buf, addr6_dns.addrIPv6) == 0
#endif
				) 
			{
				tr181_printf("invalid internet address!\n");
				return ERR_9007;
			}

			strcpy(entry[1].ipAddr, buf);

			if(sndInst == 4)
			{
				mib_set(MIB_DNS1, (void *)&ia_val);
			}
			else if(sndInst == 5)
			{
				mib_set(MIB_DNS2, (void *)&ia_val);
			}
			else if(sndInst == 6)
			{
				mib_set(MIB_DNS3, (void *)&ia_val);
			}
#ifdef CONFIG_IPV6
			else if(sndInst == 9)
			{
				if(!mib_set(MIB_IPV6_ADDR_DNS_PARAM,  (void *)&addr6_dns))
				{
					tr181_printf("set MIB_IPV6_ADDR_DNS_PARAM failed\n");
					return ERR_9002;
				}
			}
#endif
		}
		else
			return ERR_9005;
	}
	else
	{
		return ERR_9005;
	}

	if(mib_set(MIB_DNS_CLIENT_SERVER_MOD,(void*)entry)==0)
	{
		tr181_printf("set %s fail!\n",name);
		return ERR_9002;
	}

	if(isMIBUpdated)
		return 1;
	else
		return 0;
}
#endif // CONFIG_IPV6

