#include "tr181_dhcpv4Client.h"
#include "prmt_utility.h"


/*******************************************************************************
Device.DHCPv4.Client.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDhcpv4ClientEntityLeafOP = { getDhcpv4ClientEntity, setDhcpv4ClientEntity };
struct CWMP_PRMT tDhcpv4ClientEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                      eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientEntityLeafOP},
//	{"Alias",                       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientEntityLeafOP},
	{"Interface",                   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientEntityLeafOP},
	{"Status",                      eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
//	{"DHCPStatus",                  eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
	{"Renew",                       eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientEntityLeafOP},
	{"IPAddress",                   eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
	{"SubnetMask",                  eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
	{"IPRouters",                   eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
	{"DNSServers",                  eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
//	{"LeaseTimeRemaining",          eCWMP_tINT,		CWMP_READ|CWMP_DENY_ACT,	&tDhcpv4ClientEntityLeafOP},
//	{"DHCPServer",                  eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
//	{"PassthroughEnable",           eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientEntityLeafOP},
//	{"PassthroughDHCPPool",         eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientEntityLeafOP},
//	{"SentOptionNumberOfEntries",   eCWMP_tUINT,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
//	{"ReqOptionNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tDhcpv4ClientEntityLeafOP},
};
enum eDhcpv4ClientEntityLeaf
{
	eDhcpv4ClientEnable,
//	eDhcpv4ClientAlias,
	eDhcpv4ClientInterface,
	eDhcpv4ClientStatus,
//	eDhcpv4ClientDHCPStatus,
	eDhcpv4ClientRenew,
	eDhcpv4ClientIPAddress,
	eDhcpv4ClientSubnetMask,
	eDhcpv4ClientIPRouters,
	eDhcpv4ClientDNSServers,
//	eDhcpv4ClientLeaseTimeRemaining,
//	eDhcpv4ClientDHCPServer,
//	eDhcpv4ClientPassthroughEnable,
//	eDhcpv4ClientPassthroughDHCPPool,
//	eDhcpv4ClientSentOptionNumberOfEntries,
//	eDhcpv4ClientReqOptionNumberOfEntries
};
struct CWMP_LEAF tDhcpv4ClientEntityLeaf[] =
{
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientEnable] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientAlias] },
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientInterface] },
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientStatus] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientDHCPStatus] },
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientRenew] },
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientIPAddress] },
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientSubnetMask] },
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientIPRouters] },
	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientDNSServers] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientLeaseTimeRemaining] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientDHCPServer] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientPassthroughEnable] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientPassthroughDHCPPool] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientSentOptionNumberOfEntries] },
//	{ &tDhcpv4ClientEntityLeafInfo[eDhcpv4ClientReqOptionNumberOfEntries] },
	{ NULL }
};

/*******************************************************************************
Device.DHCPv4.Client.{i}.SentOption.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDhcpv4ClientSentEntityLeafOP = { getDhcpv4ClientSentEntity, setDhcpv4ClientSentEntity };
struct CWMP_PRMT tDhcpv4ClientSentEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientSentEntityLeafOP},
	{"Alias",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientSentEntityLeafOP},
	{"Tag",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientSentEntityLeafOP},
	{"Value",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientSentEntityLeafOP},
};
enum eDhcpv4ClientSentEntityLeaf
{
	eDhcpv4ClientSentEnable,
	eDhcpv4ClientSentAlias,
	eDhcpv4ClientSentTag,
	eDhcpv4ClientSentValue
};
struct CWMP_LEAF tDhcpv4ClientSentEntityLeaf[] =
{
	{ &tDhcpv4ClientSentEntityLeafInfo[eDhcpv4ClientSentEnable] },
	{ &tDhcpv4ClientSentEntityLeafInfo[eDhcpv4ClientSentAlias] },
	{ &tDhcpv4ClientSentEntityLeafInfo[eDhcpv4ClientSentTag] },
	{ &tDhcpv4ClientSentEntityLeafInfo[eDhcpv4ClientSentValue] },
	{ NULL }
};

/*******************************************************************************
Device.DHCPv4.Client.{i}.SentOption.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDhcpv4ClientSentOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eLinkDhcpv4ClientSentOjbect
{
	eLinkDhcpv4ClientSent0
};
struct CWMP_LINKNODE tLinkDhcpv4ClientSentObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkDhcpv4ClientSentOjbectInfo[eLinkDhcpv4ClientSent0],	tDhcpv4ClientSentEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.DHCPv4.Client.{i}.ReqOption.{i}. Entity
*******************************************************************************/
struct CWMP_OP tDhcpv4ClientReqEntityLeafOP = { getDhcpv4ClientReqEntity, setDhcpv4ClientReqEntity };
struct CWMP_PRMT tDhcpv4ClientReqEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientReqEntityLeafOP},
	{"Order",   eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientReqEntityLeafOP},
	{"Alias",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientReqEntityLeafOP},
	{"Tag",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientReqEntityLeafOP},
	{"Value",   eCWMP_tSTRING,	CWMP_READ,	&tDhcpv4ClientReqEntityLeafOP},
};
enum eDhcpv4ClientReqEntityLeaf
{
	eDhcpv4ClientReqEnable,
	eDhcpv4ClientReqOrder,
	eDhcpv4ClientReqAlias,
	eDhcpv4ClientReqTag,
	eDhcpv4ClientReqValue
};
struct CWMP_LEAF tDhcpv4ClientReqEntityLeaf[] =
{
	{ &tDhcpv4ClientReqEntityLeafInfo[eDhcpv4ClientReqEnable] },
	{ &tDhcpv4ClientReqEntityLeafInfo[eDhcpv4ClientReqOrder] },
	{ &tDhcpv4ClientReqEntityLeafInfo[eDhcpv4ClientReqAlias] },
	{ &tDhcpv4ClientReqEntityLeafInfo[eDhcpv4ClientReqTag] },
	{ &tDhcpv4ClientReqEntityLeafInfo[eDhcpv4ClientReqValue] },
	{ NULL }
};

/*******************************************************************************
Device.DHCPv4.Client.{i}.ReqOption.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDhcpv4ClientReqOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eLinkDhcpv4ClientReqOjbect
{
	eLinkDhcpv4ClientReq0
};
struct CWMP_LINKNODE tLinkDhcpv4ClientReqObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkDhcpv4ClientReqOjbectInfo[eLinkDhcpv4ClientReq0],	tDhcpv4ClientReqEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.DHCPv4.Client.{i}.SentOption.{i}.
Device.DHCPv4.Client.{i}.ReqOption.{i}.
*******************************************************************************/
struct CWMP_OP tDhcpv4ClientSent_OP = { NULL, objDhcpv4ClientSent };
struct CWMP_OP tDhcpv4ClientReq_OP = { NULL, objDhcpv4ClientReq };
struct CWMP_PRMT tDhcpv4ClientSentObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"SentOption",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientSent_OP},
	{"ReqOption",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDhcpv4ClientReq_OP},
};

enum eDhcpv4ClientSentObject
{
	eDhcpv4ClientSent,
	eDhcpv4ClientReq
};

struct CWMP_NODE tDhcpv4ClientSentObject[] =
{
	/*info,  					leaf,			node)*/
	{&tDhcpv4ClientSentObjectInfo[eDhcpv4ClientSent],	NULL,	NULL},
	{&tDhcpv4ClientSentObjectInfo[eDhcpv4ClientReq],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.DHCPv4.Client.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkDhcpv4ClientOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkDhcpv4ClientOjbect
{
	eLinkDhcpv4Client1
};
struct CWMP_LINKNODE tLinkDhcpv4ClientObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkDhcpv4ClientOjbectInfo[eLinkDhcpv4Client1],	tDhcpv4ClientEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.DHCPv4.Client.{i}.
*******************************************************************************/
struct CWMP_PRMT tDhcpv4ClientObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Client",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

enum eDhcpv4ClientObject
{
	eDhcpv4Client,
};

struct CWMP_NODE tDhcpv4ClientObject[] =
{
	/*info,  					leaf,			node)*/
	{&tDhcpv4ClientObjectInfo[eDhcpv4Client],	NULL,	tLinkDhcpv4ClientObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
/***************************************************
/		get data from file, file must be
/   dataName data
/   	mode,
/   when dataName appear more than once,use nubmber 
/   count and get the data,begin from 1 
***************************************************/
static int getPid_fromFile(char *file_name)
{
	FILE *fp;
	char *pidfile = file_name;
	int result = -1;
	
	fp= fopen(pidfile, "r");
	if (!fp) {
        	printf("can not open:%s\n", file_name);
		return -1;
   	}
	fscanf(fp,"%d",&result);
	fclose(fp);
	
	return result;
}

static int getDataFormFile(char* fileName,char* dataName,char* data,char number)
{
	char buff[128]={0};
	char strName[64]={0},strData[64]={0};
	char count=0;
	FILE *fp=fopen(fileName,"r");
	if (!fp) 
	{
   		printf("Open %s file error.\n", fileName);
		return 0;
	}

	while(fgets(buff,127,fp))
	{
		sscanf(buff,"%s %s",strName,strData);
		if(!strcmp(dataName,strName))
		{
			count++;
			if(number==0||count==number)
				goto FIND;			
		}		
	}
	fclose(fp);
	return 0;	
FIND:
	strcpy(data,strData);
	fclose(fp);
	return 1;
}

int objDhcpv4ClientSent(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int objDhcpv4ClientReq(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int getDhcpv4ClientEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		int lan_dhcp_mode;
		
		mib_get(MIB_DHCP, (void *)&lan_dhcp_mode);
		if (lan_dhcp_mode == DHCP_CLIENT)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		*data = strdup("Device.IP.Interface.1.");
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int lan_dhcp_mode;
		
		mib_get(MIB_DHCP, (void *)&lan_dhcp_mode);
		if (lan_dhcp_mode == DHCP_CLIENT)
			*data = strdup("Enabled");
		else
			*data = strdup("Disabled");
	}
	else if( strcmp( lastname, "Renew" )==0 )
	{
		*data = booldup(0); 
	}
	else if( strcmp( lastname, "IPAddress" )==0 )
	{
		struct in_addr ipAddr={0};
		
		if(!getInAddr("br0", IP_ADDR, (void *)&ipAddr))
			*data = strdup("");
		else
			*data = strdup(inet_ntoa(ipAddr));
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		struct in_addr netmask={0};
		
		if(!getInAddr("br0", SUBNET_MASK, (void *)&netmask))
			*data = strdup("");
		else
			*data = strdup(inet_ntoa(netmask));
	}
	else if( strcmp( lastname, "IPRouters" )==0 )
	{
		struct in_addr router={0};
		
		if(!getDefaultRoute("br0",&router))
			*data = strdup("");
		else
			*data = strdup(inet_ntoa(router));
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		char buf[256]={0};
		char strDns1[64]={0};
		char strDns2[64]={0};
		char strDns3[64]={0};

		getDataFormFile("/var/resolv.conf","nameserver",&strDns1,1);
		getDataFormFile("/var/resolv.conf","nameserver",&strDns2,2);
		getDataFormFile("/var/resolv.conf","nameserver",&strDns3,3);

		if (strlen(strDns1) > 0)
			sprintf(buf, "%s", strDns1);

		if (strlen(strDns2) > 0) {
			strcat(buf, ",");
			strcat(buf, strDns2);
		}

		if (strlen(strDns3) > 0) {
			strcat(buf, ",");
			strcat(buf, strDns3);
		}

		*data = strdup(buf);
	}
	else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int setDhcpv4ClientEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;	
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	
	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned int vInt;
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vInt = (*i==0)?DHCP_DISABLED:DHCP_CLIENT;
		mib_set(MIB_DHCP, &vInt);

		return 1;
	}
	else if( strcmp( lastname, "Renew" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;

		if( *i==1 )
		{
			char renewCmd[30];
			int pid=-1;
			
			if(isFileExist("/etc/udhcpc/udhcpc-br0.pid"))
			{
				pid=getPid_fromFile("/etc/udhcpc/udhcpc-br0.pid");
				if(pid != 0)
				{
					sprintf(renewCmd, "kill -SIGUSR1 %d", pid);

					system(renewCmd);
				}

			}
		}
	}
	else
		return ERR_9005; 
	
	return 0;
}



int getDhcpv4ClientSentEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int setDhcpv4ClientSentEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

int getDhcpv4ClientReqEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int setDhcpv4ClientReqEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}
