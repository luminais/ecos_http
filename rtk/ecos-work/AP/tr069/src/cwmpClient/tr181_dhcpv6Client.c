#include <sys/ioctl.h>
#ifdef CONFIG_IPV6
#include <linux/if_addr.h>
#endif
#include "tr181_dhcpv6Client.h"

#ifdef CONFIG_IPV6
#define CONFIG_SET(key, val) if ( mib_set(key, val)==0)  return ERR_9002
#define CONFIG_GET(key, ret) if ( mib_get(key, ret)==0)  return ERR_9002

#define CHECK_PARAM_NUM(input, min, max) if ( (input < min) || (input > max) ) return ERR_9007;
#define CHECK_PARAM_STR(str, min, max)  do { \
	int tmp; \
	if (!str) return ERR_9007; \
	tmp=strlen(str); \
	if ((tmp < min) || (tmp > max)) return ERR_9007; \
}	while (0)


/*******************************************************************************
Device.DHCPv6.Client.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDhcpv6ClientEntityLeafOP = { getDhcpv6ClientEntity, setDhcpv6ClientEntity };
struct CWMP_PRMT tDhcpv6ClientEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                       eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
//	{"Alias",                        eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"Interface",                    eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"Status",                       eCWMP_tSTRING,	CWMP_READ,	&tDhcpv6ClientEntityLeafOP},
	{"DUID",                         eCWMP_tSTRING,	CWMP_READ,	&tDhcpv6ClientEntityLeafOP},
	{"RequestAddresses",             eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"RequestPrefixes",              eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"RapidCommit",                  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"Renew",                        eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"SuggestedT1",                  eCWMP_tINT,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"SuggestedT2",                  eCWMP_tINT,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
	{"SupportedOptions",             eCWMP_tSTRING,	CWMP_READ,	&tDhcpv6ClientEntityLeafOP},
//	{"RequestedOptions",             eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientEntityLeafOP},
//	{"ServerNumberOfEntries",        eCWMP_tUINT,	CWMP_READ,	&tDhcpv6ClientEntityLeafOP},
	{"SentOptionNumberOfEntries",    eCWMP_tUINT,	CWMP_READ,	&tDhcpv6ClientEntityLeafOP},
	{"ReceivedOptionNumberOfEntries",eCWMP_tUINT,	CWMP_READ,	&tDhcpv6ClientEntityLeafOP},
};
enum eDhcpv6ClientEntityLeaf
{
	eDhcpv6ClientEnable,
//	eDhcpv6ClientAlias,
	eDhcpv6ClientInterface,
	eDhcpv6ClientStatus,
	eDhcpv6ClientDUID,
	eDhcpv6ClientRequestAddresses,
	eDhcpv6ClientRequestPrefixes,
	eDhcpv6ClientRapidCommit,
	eDhcpv6ClientRenew,
	eDhcpv6ClientSuggestedT1,
	eDhcpv6ClientSuggestedT2,
	eDhcpv6ClientSupportedOptions,
//	eDhcpv6ClientRequestedOptions,
//	eDhcpv6ClientServerNumberOfEntries,
	eDhcpv6ClientSentOptionNumberOfEntries,
	eDhcpv6ClientReceivedOptionNumberOfEntries
};
struct CWMP_LEAF tDhcpv6ClientEntityLeaf[] =
{
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientEnable] },
//	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientAlias] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientInterface] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientStatus] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientDUID] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientRequestAddresses] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientRequestPrefixes] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientRapidCommit] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientRenew] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientSuggestedT1] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientSuggestedT2] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientSupportedOptions] },
//	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientRequestedOptions] },
//	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientServerNumberOfEntries] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientSentOptionNumberOfEntries] },
	{ &tDhcpv6ClientEntityLeafInfo[eDhcpv6ClientReceivedOptionNumberOfEntries] },
	{ NULL }
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.SentOption.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDhcpv6ClientSentEntityLeafOP = { getDhcpv6ClientSentEntity, setDhcpv6ClientSentEntity };
struct CWMP_PRMT tDhcpv6ClientSentEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientSentEntityLeafOP},
//	{"Alias",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientSentEntityLeafOP},
	{"Tag",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientSentEntityLeafOP},
	{"Value",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientSentEntityLeafOP},
};
enum eDhcpv6ClientSentEntityLeaf
{
	eDhcpv6ClientSentEnable,
//	eDhcpv6ClientSentAlias,
	eDhcpv6ClientSentTag,
	eDhcpv6ClientSentValue
};
struct CWMP_LEAF tDhcpv6ClientSentEntityLeaf[] =
{
	{ &tDhcpv6ClientSentEntityLeafInfo[eDhcpv6ClientSentEnable] },
//	{ &tDhcpv6ClientSentEntityLeafInfo[eDhcpv6ClientSentAlias] },
	{ &tDhcpv6ClientSentEntityLeafInfo[eDhcpv6ClientSentTag] },
	{ &tDhcpv6ClientSentEntityLeafInfo[eDhcpv6ClientSentValue] },
	{ NULL }
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.SentOption.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDhcpv6ClientSentOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eLinkDhcpv6ClientSentOjbect
{
	eLinkDhcpv6ClientSent0
};
struct CWMP_LINKNODE tLinkDhcpv6ClientSentObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkDhcpv6ClientSentOjbectInfo[eLinkDhcpv6ClientSent0],	tDhcpv6ClientSentEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.ReceivedOption.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDhcpv6ClientRecvEntityLeafOP = { getDhcpv6ClientRecvEntity, setDhcpv6ClientRecvEntity };
struct CWMP_PRMT tDhcpv6ClientRecvEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Tag",		eCWMP_tUINT,	CWMP_READ,	&tDhcpv6ClientRecvEntityLeafOP},
	{"Value",   eCWMP_tSTRING,	CWMP_READ,	&tDhcpv6ClientRecvEntityLeafOP},
//	{"Server",  eCWMP_tSTRING,	CWMP_READ,	&tDhcpv6ClientRecvEntityLeafOP},
};
enum eDhcpv6ClientRecvEntityLeaf
{
	eDhcpv6ClientRecvTag,
	eDhcpv6ClientRecvValue,
//	eDhcpv6ClientRecvServer
};
struct CWMP_LEAF tDhcpv6ClientRecvEntityLeaf[] =
{
	{ &tDhcpv6ClientRecvEntityLeafInfo[eDhcpv6ClientRecvTag] },
	{ &tDhcpv6ClientRecvEntityLeafInfo[eDhcpv6ClientRecvValue] },
//	{ &tDhcpv6ClientRecvEntityLeafInfo[eDhcpv6ClientRecvServer] },
	{ NULL }
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.ReceivedOption.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDhcpv6ClientRecvOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkDhcpv6ClientRecvOjbect
{
	eLinkDhcpv6ClientRecv0
};
struct CWMP_LINKNODE tLinkDhcpv6ClientRecvObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkDhcpv6ClientRecvOjbectInfo[eLinkDhcpv6ClientRecv0],	tDhcpv6ClientRecvEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.Server.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDhcpv6ClientServerEntityLeafOP = { getDhcpv6ClientServerEntity, setDhcpv6ClientServerEntity };
struct CWMP_PRMT tDhcpv6ClientServerEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"SourceAddress",  			eCWMP_tSTRING,	CWMP_READ,	&tDhcpv6ClientServerEntityLeafOP},
	{"DUID",   					eCWMP_tSTRING,	CWMP_READ,	&tDhcpv6ClientServerEntityLeafOP},
	{"InformationRefreshTime",	eCWMP_tDATETIME,CWMP_READ,	&tDhcpv6ClientServerEntityLeafOP},
};
enum eDhcpv6ClientServerEntityLeaf
{
	eDhcpv6ClientServerSrcAddr,
	eDhcpv6ClientServerDUID,
	eDhcpv6ClientServerInfoRef
};
struct CWMP_LEAF tDhcpv6ClientServerEntityLeaf[] =
{
	{ &tDhcpv6ClientServerEntityLeafInfo[eDhcpv6ClientServerSrcAddr] },
	{ &tDhcpv6ClientServerEntityLeafInfo[eDhcpv6ClientServerDUID] },
	{ &tDhcpv6ClientServerEntityLeafInfo[eDhcpv6ClientServerInfoRef] },
	{ NULL }
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.Server.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDhcpv6ClientServerOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkDhcpv6ClientServerOjbect
{
	eLinkDhcpv6ClientServer0
};
struct CWMP_LINKNODE tLinkDhcpv6ClientServerObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkDhcpv6ClientServerOjbectInfo[eLinkDhcpv6ClientServer0],	tDhcpv6ClientServerEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.Server.{i}.
Device.DHCPv6.Client.{i}.SentOption.{i}.
Device.DHCPv6.Client.{i}.ReceivedOption.{i}.
*******************************************************************************/
struct CWMP_OP tDhcpv6ClientSent_OP = { NULL, objDhcpv6ClientSent };
struct CWMP_OP tDhcpv6ClientReq_OP = { NULL, objDhcpv6ClientReq };
struct CWMP_PRMT tDhcpv6ClientSentObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
//	{"Server",	eCWMP_tOBJECT,	CWMP_READ,	&tDhcpv6ClientSent_OP},
	{"SentOption",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDhcpv6ClientSent_OP},
	{"ReceivedOption",	eCWMP_tOBJECT,	CWMP_READ,	&tDhcpv6ClientReq_OP},
};

enum eDhcpv6ClientSentObject
{
//	eDhcpv6ClientServer,
	eDhcpv6ClientSent,
	eDhcpv6ClientRecv
};

struct CWMP_NODE tDhcpv6ClientSentObject[] =
{
	/*info,  					leaf,			node)*/
//	{&tDhcpv6ClientSentObjectInfo[eDhcpv6ClientServer],	NULL,	NULL},
	{&tDhcpv6ClientSentObjectInfo[eDhcpv6ClientSent],	NULL,	NULL},
	{&tDhcpv6ClientSentObjectInfo[eDhcpv6ClientRecv],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.DHCPv6.Client.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDhcpv6ClientOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkDhcpv6ClientOjbect
{
	eLinkDhcpv6Client0
};
struct CWMP_NODE tLinkDhcpv6ClientObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkDhcpv6ClientOjbectInfo[eLinkDhcpv6Client0],	tDhcpv6ClientEntityLeaf,	tDhcpv6ClientSentObject},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.DHCPv6.Client.{i}.
*******************************************************************************/
struct CWMP_PRMT tDhcpv6ClientObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Client",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDhcpv6ClientObject
{
	eDhcpv6Client,
};

struct CWMP_NODE tDhcpv6ClientObject[] =
{
	/*info,  					leaf,			node)*/
	{&tDhcpv6ClientObjectInfo[eDhcpv6Client],	NULL,	tLinkDhcpv6ClientObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int objDhcpv6Client(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int objDhcpv6ClientSent(char *name, struct CWMP_LEAF *e, int type, void *data)
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

			if(IPV6_DHCPC_SENDOPT_NUM>0)
				return create_Object( c, tLinkDhcpv6ClientSentObject, sizeof(tLinkDhcpv6ClientSentObject), IPV6_DHCPC_SENDOPT_NUM, 1 );

			return 0;
			break;
		}
		case eCWMP_tUPDATEOBJ:
		{
			unsigned int num,i;
			struct CWMP_LINKNODE *old_table;

			num = IPV6_DHCPC_SENDOPT_NUM;
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
					add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tLinkDhcpv6ClientSentObject, sizeof(tLinkDhcpv6ClientSentObject), &InstNum );
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

int objDhcpv6ClientReq(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	FILE*fp=fopen("/var/dhcp6c_rcvOpt","r");
	int count=0;
	char valueBuf[128]={0};
		
	if(fp==NULL)
	{
		return ERR_9002;
	}
	
	for(count=0;count<IPV6_DHCPC_RCVOPT_NUM+1;count++)
	{
		if(fgets(valueBuf,sizeof(valueBuf),fp)==NULL)
		{
			break;
		}
	}
	fclose(fp);
	
	if(count==IPV6_DHCPC_RCVOPT_NUM+1)
	{
		printf("over max!check /var/dhcp6c_rcvOpt\n");
		return ERR_9002;
	}

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			*c=NULL;

			tr181_printf("count %d", count);
			if(count>0)
				return create_Object( c, tLinkDhcpv6ClientRecvObject, sizeof(tLinkDhcpv6ClientRecvObject), count, 1 );

			return 0;
			break;
		}
		case eCWMP_tUPDATEOBJ:
		{
			unsigned int num,i;
			struct CWMP_LINKNODE *old_table;

			tr181_printf("count %d", count);
			num = count;
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
					add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tLinkDhcpv6ClientRecvObject, sizeof(tLinkDhcpv6ClientRecvObject), &InstNum );
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

int getDhcpv6ClientEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	unsigned int vInt=0;
	int value=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_IPV6_WAN_ENABLE, (void*)&vInt);
		
		*data = booldup(vInt);
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		mib_get(MIB_IPV6_DHCPC_IFACE, (void*)buf);
			
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		mib_get(MIB_IPV6_DHCP_MODE, (void*)&vInt);

		if (vInt == IPV6_DHCP_STATELESS)
		{
			*data = strdup("Disabled");
		}
		else if (vInt == IPV6_DHCP_STATEFUL)
		{
			if (access("/var/dhcp6c_parse_fail",0) < 0)
				*data = strdup("Enabled");
			else
				*data = strdup("Error_Misconfigured");
		}
	}
	else if( strcmp( lastname, "DUID" )==0 )
	{
		char wanIf[16]={0};
		struct sockaddr hwaddr={0};
		struct duid_t dhcp6c_duid={0};
		struct ifreq ifr={0};
    	int skfd=0;
		char DUID_buff[130]={0};
		
		if(get_wanif(wanIf)<0)
			return ERR_9002;
		
    	skfd = socket(AF_INET, SOCK_DGRAM, 0);
   		strcpy(ifr.ifr_name, wanIf);
		
		if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) 
		{
			memcpy(&hwaddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
		}
		else		
		{
			fprintf(stderr, "Read hwaddr Error\n");
			return ERR_9002;
		}
		
		dhcp6c_duid.duid_type=3;
		dhcp6c_duid.hw_type=1;
		memcpy(dhcp6c_duid.mac,hwaddr.sa_data,6);
		
		sprintf(DUID_buff,"%04x%04x%02x%02x%02x%02x%02x%02x",
			3,1,hwaddr.sa_data[0],hwaddr.sa_data[1],
			hwaddr.sa_data[2],hwaddr.sa_data[3],
			hwaddr.sa_data[4],hwaddr.sa_data[5]);
		
		*data = strdup(DUID_buff);
	}
	else if( strcmp( lastname, "RequestAddresses" )==0 )
	{
		mib_get(MIB_IPV6_DHCPC_REQUEST_ADDR, (void*)&vInt);
		
		*data = booldup(vInt);
	}
	else if( strcmp( lastname, "RequestPrefixes" )==0 )
	{
		mib_get(MIB_IPV6_DHCP_PD_ENABLE, (void*)&vInt);
		
		*data = booldup(vInt);
	}
	else if( strcmp( lastname, "RapidCommit" )==0 )
	{
		mib_get(MIB_IPV6_DHCP_RAPID_COMMIT_ENABLE, (void*)&vInt);
		
		*data = booldup(vInt);
	}
	else if( strcmp( lastname, "Renew" )==0 )
	{
		*data = booldup(0);
	}
	else if( strcmp( lastname, "SuggestedT1" )==0 )
	{
		mib_get(MIB_IPV6_DHCPC_SUGGESTEDT1, (void*)&value);
		
		*data = intdup(value);
	}
	else if( strcmp( lastname, "SuggestedT2" )==0 )
	{
		mib_get(MIB_IPV6_DHCPC_SUGGESTEDT2, (void*)&value);
		
		*data = intdup(value);
	}
	else if( strcmp( lastname, "SupportedOptions" )==0 )
	{
		*data = strdup("1,2,3,5,6,8,14,21,22,23,27,28,29,30,31,33,34");
	}
	else if( strcmp( lastname, "SentOptionNumberOfEntries" )==0 )
	{
		*data = uintdup(IPV6_DHCPC_SENDOPT_NUM);
	}
	else if( strcmp( lastname, "ReceivedOptionNumberOfEntries" )==0 )
	{
		FILE*fp=fopen("/var/dhcp6c_rcvOpt","r");
		int count=0;
		char valueBuf[128]={0};
		
		if(fp==NULL)
		{
			*data = uintdup( 0 );
			return 0;
		}
		
		for(count=0;count<IPV6_DHCPC_RCVOPT_NUM+1;count++)
		{
			if(fgets(valueBuf,sizeof(valueBuf),fp)==NULL)
			{
				break;
			}
		}
		fclose(fp);
		
		if(count==IPV6_DHCPC_RCVOPT_NUM+1)
		{
			printf("over max!check /var/dhcp6c_rcvOpt\n");
			return ERR_9002;
		}
		
		*data = uintdup(count);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDhcpv6ClientEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
		CONFIG_SET(MIB_IPV6_WAN_ENABLE, &uintValue);	
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(MIB_IPV6_DHCPC_IFACE, buf);
	}
	else if( strcmp( lastname, "RequestAddresses" )==0 )
	{
		pUintValue = (unsigned int *)data;
		CHECK_PARAM_NUM(*pUintValue, 0, 1);
		uintValue = (*pUintValue == 0) ? 0 : 1;
		CONFIG_SET(MIB_IPV6_DHCPC_REQUEST_ADDR, &uintValue);	
	}
	else if( strcmp( lastname, "RequestPrefixes" )==0 )
	{
		pUintValue = (unsigned int *)data;
		CHECK_PARAM_NUM(*pUintValue, 0, 1);
		uintValue = (*pUintValue == 0) ? 0 : 1;
		CONFIG_SET(MIB_IPV6_DHCP_PD_ENABLE, &uintValue);	
	}
	else if( strcmp( lastname, "RapidCommit" )==0 )
	{
		pUintValue = (unsigned int *)data;
		CHECK_PARAM_NUM(*pUintValue, 0, 1);
		uintValue = (*pUintValue == 0) ? 0 : 1;
		CONFIG_SET(MIB_IPV6_DHCP_RAPID_COMMIT_ENABLE, &uintValue);	
	}
	else if( strcmp( lastname, "Renew" )==0 )
	{
		isMIBUpdated = 0;
		pUintValue = (unsigned int *)data;
		CHECK_PARAM_NUM(*pUintValue, 0, 1);
		uintValue = (*pUintValue == 0) ? 0 : 1;
		
		if(uintValue)
		{
			FILE* fp=fopen("/var/run/dhcp6c.pid","r");
			int pid=0;
			if(fp==NULL)
			{
				printf("can't open pid file /var/run/dhcp6c.pid!\n");
				return -1;
			}
			fscanf(fp,"%d",&pid);
			fclose(fp);
			if(pid<0 || pid>100000)
			{
				printf("invalid pid in file /var/run/dhcp6c.pid!\n");
				return -1;
			}
			kill(pid,SIGHUP);
		}
	}
	else if( strcmp( lastname, "SuggestedT1" )==0 )
	{
		pIntValue = (int *)data;
		CHECK_PARAM_NUM(*pIntValue, -1, 32767);
		intValue = *pIntValue;
		CONFIG_SET(MIB_IPV6_DHCPC_SUGGESTEDT1, &intValue);	
	}
	else if( strcmp( lastname, "SuggestedT2" )==0 )
	{
		pIntValue = (int *)data;
		CHECK_PARAM_NUM(*pIntValue, -1, 32767);
		intValue = *pIntValue;
		CONFIG_SET(MIB_IPV6_DHCPC_SUGGESTEDT2, &intValue);	
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


int getDhcpv6ClientSentEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	unsigned int sndInst;
	DHCPV6C_SENDOPT_T entry={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	sndInst = getInstanceNum(name, "SentOption");

	//tr181_printf("sndInst %d", sndInst);

	if( sndInst<1 || sndInst>IPV6_DHCPC_SENDOPT_NUM ) return ERR_9007;

	*((char*)&entry) = (char)sndInst;

	if(mib_get(MIB_IPV6_DHCPC_SENDOPT_TBL,(void*)&entry)==0)
	{
		printf("get MIB_IPV6_DHCPC_SENDOPT_TBL fail!\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(entry.enable);
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		*data = uintdup(entry.tag);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		*data = strdup(entry.value);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDhcpv6ClientSentEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf = data;
	int isMIBUpdated = 1;
	int maxNum=IPV6_DHCPC_SENDOPT_NUM;
	unsigned int sndInst;
	DHCPV6C_SENDOPT_T entry[2]={0};

	sndInst = getInstanceNum(name, "SentOption");

	//tr181_printf("sndInst %d", sndInst);

	if( sndInst<1 || sndInst>IPV6_DHCPC_SENDOPT_NUM ) return ERR_9007;

	mib_set(MIB_IPV6_DHCPC_SENDOPT_TBL_NUM,(void*)&maxNum);

	*((char*)&entry) = (char)sndInst;

	if(mib_get(MIB_IPV6_DHCPC_SENDOPT_TBL,(void*)&entry)==0)
	{
		tr181_printf("get MIB_IPV6_DHCPC_SENDOPT_TBL fail!\n");
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
	else if( strcmp( lastname, "Tag" )==0 )
	{
		int i=0,tagVal=0;
		DHCPV6C_SENDOPT_T entryTmp={0};

		tagVal=*((int*)data);
		for(i=1;i<=maxNum;i++)
		{
			*((char *)&entryTmp) = (char)i;
			if ( !mib_get(MIB_IPV6_DHCPC_SENDOPT_TBL, (void *)&entryTmp)){
				tr181_printf("set %s fail!\n",name);
				return ERR_9002;
			}
			if(entryTmp.tag==tagVal && tagVal!=0)
			{
				tr181_printf("set %s fail! Tag exist!\n",name);
				return ERR_9002;
			}
		}
		entry[1].tag=tagVal;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		strcpy(entry[1].value,buf);
		//tr181_printf("buf: %s value: %s", buf, entry[1].value);
	}
	else
	{
		return ERR_9005;
	}

	if(mib_set(MIB_IPV6_DHCPC_SENDOPT_MOD,(void*)entry)==0)
	{
		tr181_printf("set %s fail!\n",name);
		return ERR_9002;
	}

	if(isMIBUpdated)
		return 1;
	else
		return 0;
}

int getDhcpv6ClientRecvEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	int count=0;
	int iTag=0;
	char valueBuf[128]={0};
	char valueNameBuf[128]={0};
	char valueValBuf[128]={0};
	unsigned int recvInst=0;
	FILE*fp=fopen("/var/dhcp6c_rcvOpt","r");

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	recvInst = getInstanceNum(name, "ReceivedOption");

	if( recvInst<1 || recvInst>IPV6_DHCPC_RCVOPT_NUM ) return ERR_9007;

	if(fp==NULL)
	{
		return ERR_9002;
	}

	for(count=0;count<recvInst;count++)
	{
		bzero(valueBuf,sizeof(valueBuf));
		if(fgets(valueBuf,sizeof(valueBuf),fp)==NULL)
		{
			tr181_printf("fgets is NULL");
		}
	}
	fclose(fp);
	sscanf(valueBuf,"%s	%d	%s\n",valueNameBuf,&iTag,valueValBuf);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Tag" )==0 )
	{
		*data = uintdup(iTag);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		*data = strdup(valueValBuf);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDhcpv6ClientRecvEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

int getDhcpv6ClientServerEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int setDhcpv6ClientServerEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

#endif // CONFIG_IPV6

