#include "tr181_eth.h"
#include "tr181_ethIF.h"



/*******************************************************************************
Device.Ethernet.Link.{i}.Stats
*******************************************************************************/
struct CWMP_OP tEthLinkStEntityLeafOP = { getEthLinkStats, NULL };
struct CWMP_PRMT tEthLinkStEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"BytesSent",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP}, // unsigned long?
	{"BytesReceived",               eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"PacketsSent",                 eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"PacketsReceived",             eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"ErrorsSent",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP}, // unsigned int
	{"ErrorsReceived",              eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP}, // unsigned int
	{"UnicastPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"UnicastPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"DiscardPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP}, // unsigned int
	{"DiscardPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP}, // unsigned int
	{"MulticastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"MulticastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"BroadcastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
	{"BroadcastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
//	{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthLinkStEntityLeafOP},
};
enum eEthLinkStEntityLeaf
{
	eEthLinkStBytesSent,
	eEthLinkStBytesReceived,
	eEthLinkStPacketsSent,
	eEthLinkStPacketsReceived,
	eEthLinkStErrorsSent,
	eEthLinkStErrorsReceived,
	eEthLinkStUnicastPacketsSent,
	eEthLinkStUnicastPacketsReceived,
	eEthLinkStDiscardPacketsSent,
	eEthLinkStDiscardPacketsReceived,
	eEthLinkStMulticastPacketsSent,
	eEthLinkStMulticastPacketsReceived,
	eEthLinkStBroadcastPacketsSent,
	eEthLinkStBroadcastPacketsReceived
//	eEthLinkStUnknownProtoPacketsReceived
};
struct CWMP_LEAF tEthLinkStEntityLeaf[] =
{
	{ &tEthLinkStEntityLeafInfo[eEthLinkStBytesSent] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStBytesReceived] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStPacketsSent] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStPacketsReceived] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStErrorsSent] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStErrorsReceived] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStUnicastPacketsSent] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStUnicastPacketsReceived] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStDiscardPacketsSent] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStDiscardPacketsReceived] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStMulticastPacketsSent] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStMulticastPacketsReceived] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStBroadcastPacketsSent] },
	{ &tEthLinkStEntityLeafInfo[eEthLinkStBroadcastPacketsReceived] },
//	{ &tEthLinkStEntityLeafInfo[eEthLinkStUnknownProtoPacketsReceived] },
	{ NULL }
};

/*******************************************************************************
Device.Ethernet.Link.{i}.Stats
*******************************************************************************/
struct CWMP_PRMT tEthLinkStatsObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Stats",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eEthLinkStatsObject
{
	eEthLinkStats
};

struct CWMP_NODE tEthLinkStatsObject[] =
{
	/*info,  					leaf,			node)*/
	{&tEthLinkStatsObjectInfo[eEthLinkStats],	&tEthLinkStEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.Ethernet.Link.{i} Entity
*******************************************************************************/
struct CWMP_OP tEthLinkEntityLeafOP = { getEthLinkEntity, setEthLinkEntity };
struct CWMP_PRMT tEthLinkEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE, &tEthLinkEntityLeafOP},
	{"Status",      eCWMP_tSTRING,	CWMP_READ, &tEthLinkEntityLeafOP},
//	{"Alias",       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tEthLinkEntityLeafOP},
	{"Name",        eCWMP_tSTRING,	CWMP_READ, &tEthLinkEntityLeafOP},
	{"LastChange",  eCWMP_tUINT,	CWMP_READ, &tEthLinkEntityLeafOP},
//	{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tEthLinkEntityLeafOP}, // list of string
	{"MACAddress",  eCWMP_tSTRING,	CWMP_READ, &tEthLinkEntityLeafOP}, // type: MACAddress
//	{"PriorityTagging",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE, &tEthLinkEntityLeafOP}, // default false
};
enum eEthLinkEntityLeaf
{
	eEthLinkEnable,
	eEthLinkStatus,
//	eEthLinkAlias,
	eEthLinkName,
	eEthLinkLastChange,
//	eEthLinkLowerLayers,
	eEthLinkMACAddress
//	eEthLinkPriorityTagging
};
struct CWMP_LEAF tEthLinkEntityLeaf[] =
{
	{ &tEthLinkEntityLeafInfo[eEthLinkEnable] },
	{ &tEthLinkEntityLeafInfo[eEthLinkStatus] },
//	{ &tEthLinkEntityLeafInfo[eEthLinkAlias] },
	{ &tEthLinkEntityLeafInfo[eEthLinkName] },
	{ &tEthLinkEntityLeafInfo[eEthLinkLastChange] },
//	{ &tEthLinkEntityLeafInfo[eEthLinkLowerLayers] },
	{ &tEthLinkEntityLeafInfo[eEthLinkMACAddress] },
//	{ &tEthLinkEntityLeafInfo[eEthLinkPriorityTagging] },
	{ NULL }
};


/*******************************************************************************
Device.Ethernet.Link.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkEthIFOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eLinkEthIFOjbect
{
	eLinkEthIF1,
	eLinkEthIF2
};
struct CWMP_NODE tLinkEthIFObject[] =
{
	/*info,  					leaf,			node)*/
	{&tLinkEthIFOjbectInfo[eLinkEthIF1],	&tEthLinkEntityLeaf,	&tEthLinkStatsObject},
	{&tLinkEthIFOjbectInfo[eLinkEthIF2],	&tEthLinkEntityLeaf,	&tEthLinkStatsObject},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.Ethernet.Interface.{i}.Stats
*******************************************************************************/
struct CWMP_OP tEthIFStEntityLeafOP = { getEthIFStats, NULL };
struct CWMP_PRMT tEthIFStEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"BytesSent",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP}, // unsigned long?
	{"BytesReceived",               eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"PacketsSent",                 eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"PacketsReceived",             eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"ErrorsSent",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP}, // unsigned int
	{"ErrorsReceived",              eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP}, // unsigned int
	{"UnicastPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"UnicastPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"DiscardPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP}, // unsigned int
	{"DiscardPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP}, // unsigned int
	{"MulticastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"MulticastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"BroadcastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
	{"BroadcastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
//	{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEthIFStEntityLeafOP},
};
enum eEthIFStEntityLeaf
{
	eEthIFStBytesSent,
	eEthIFStBytesReceived,
	eEthIFStPacketsSent,
	eEthIFStPacketsReceived,
	eEthIFStErrorsSent,
	eEthIFStErrorsReceived,
	eEthIFStUnicastPacketsSent,
	eEthIFStUnicastPacketsReceived,
	eEthIFStDiscardPacketsSent,
	eEthIFStDiscardPacketsReceived,
	eEthIFStMulticastPacketsSent,
	eEthIFStMulticastPacketsReceived,
	eEthIFStBroadcastPacketsSent,
	eEthIFStBroadcastPacketsReceived
//	eEthIFStUnknownProtoPacketsReceived
};
struct CWMP_LEAF tEthIFStEntityLeaf[] =
{
	{ &tEthIFStEntityLeafInfo[eEthIFStBytesSent] },
	{ &tEthIFStEntityLeafInfo[eEthIFStBytesReceived] },
	{ &tEthIFStEntityLeafInfo[eEthIFStPacketsSent] },
	{ &tEthIFStEntityLeafInfo[eEthIFStPacketsReceived] },
	{ &tEthIFStEntityLeafInfo[eEthIFStErrorsSent] },
	{ &tEthIFStEntityLeafInfo[eEthIFStErrorsReceived] },
	{ &tEthIFStEntityLeafInfo[eEthIFStUnicastPacketsSent] },
	{ &tEthIFStEntityLeafInfo[eEthIFStUnicastPacketsReceived] },
	{ &tEthIFStEntityLeafInfo[eEthIFStDiscardPacketsSent] },
	{ &tEthIFStEntityLeafInfo[eEthIFStDiscardPacketsReceived] },
	{ &tEthIFStEntityLeafInfo[eEthIFStMulticastPacketsSent] },
	{ &tEthIFStEntityLeafInfo[eEthIFStMulticastPacketsReceived] },
	{ &tEthIFStEntityLeafInfo[eEthIFStBroadcastPacketsSent] },
	{ &tEthIFStEntityLeafInfo[eEthIFStBroadcastPacketsReceived] },
//	{ &tEthIFStEntityLeafInfo[eEthIFStUnknownProtoPacketsReceived] },
	{ NULL }
};

/*******************************************************************************
Device.Ethernet.Interface.{i}.Stats
*******************************************************************************/
struct CWMP_PRMT tEthIFStatsObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Stats",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eEthIFStatsObject
{
	eEthIFStats
};

struct CWMP_NODE tEthIFStatsObject[] =
{
	/*info,  					leaf,			node)*/
	{&tEthIFStatsObjectInfo[eEthIFStats],	&tEthIFStEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.Ethernet.Interface.{i} Entity
*******************************************************************************/
struct CWMP_OP tEthIFEntityLeafOP = { getEthIFEntity, setEthIFEntity };
struct CWMP_PRMT tEthIFEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE, &tEthIFEntityLeafOP},
	{"Status",      eCWMP_tSTRING,	CWMP_READ, &tEthIFEntityLeafOP},
//	{"Alias",       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tEthIFEntityLeafOP},
//	{"Name",        eCWMP_tSTRING,	CWMP_READ, &tEthIFEntityLeafOP},
	{"LastChange",  eCWMP_tUINT,	CWMP_READ, &tEthIFEntityLeafOP},
//	{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tEthIFEntityLeafOP}, // list of string
	{"Upstream",    eCWMP_tBOOLEAN,	CWMP_READ, &tEthIFEntityLeafOP},
//	{"MACAddress",  eCWMP_tSTRING,	CWMP_READ, &tEthIFEntityLeafOP}, // type: MACAddress
	{"MaxBitRate",  eCWMP_tINT,		CWMP_READ|CWMP_WRITE, &tEthIFEntityLeafOP},
	{"DuplexMode",  eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tEthIFEntityLeafOP}, // Half, Full, Auto
};
enum eEthIFEntityLeaf
{
	eEthIFEnable,
	eEthIFStatus,
//	eEthIFAlias,
//	eEthIFName,
	eEthIFLastChange,
//	eEthIFLowerLayers,
	eEthIFUpstream,
//	eEthIFMACAddress,
	eEthIFMaxBitRate,
	eEthIFDuplexMode
};
struct CWMP_LEAF tEthIFEntityLeaf[] =
{
	{ &tEthIFEntityLeafInfo[eEthIFEnable] },
	{ &tEthIFEntityLeafInfo[eEthIFStatus] },
//	{ &tEthIFEntityLeafInfo[eEthIFAlias] },
//	{ &tEthIFEntityLeafInfo[eEthIFName] },
	{ &tEthIFEntityLeafInfo[eEthIFLastChange] },
//	{ &tEthIFEntityLeafInfo[eEthIFLowerLayers] },
	{ &tEthIFEntityLeafInfo[eEthIFUpstream] },
//	{ &tEthIFEntityLeafInfo[eEthIFMACAddress] },
	{ &tEthIFEntityLeafInfo[eEthIFMaxBitRate] },
	{ &tEthIFEntityLeafInfo[eEthIFDuplexMode] },
	{ NULL }
};

/*******************************************************************************
Device.Ethernet.Interface.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tInterfaceEthIFOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"5",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eInterfaceEthIFOjbect
{
	eInterfaceEthIF1,
	eInterfaceEthIF2,
	eInterfaceEthIF3,
	eInterfaceEthIF4,
	eInterfaceEthIF5
};
struct CWMP_NODE tInterfaceEthIFObject[] =
{
	/*info,  					leaf,			node)*/
	{&tInterfaceEthIFOjbectInfo[eInterfaceEthIF1],	&tEthIFEntityLeaf,	&tEthIFStatsObject},
	{&tInterfaceEthIFOjbectInfo[eInterfaceEthIF2],	&tEthIFEntityLeaf,	&tEthIFStatsObject},
	{&tInterfaceEthIFOjbectInfo[eInterfaceEthIF3],	&tEthIFEntityLeaf,	&tEthIFStatsObject},
	{&tInterfaceEthIFOjbectInfo[eInterfaceEthIF4],	&tEthIFEntityLeaf,	&tEthIFStatsObject},
	{&tInterfaceEthIFOjbectInfo[eInterfaceEthIF5],	&tEthIFEntityLeaf,	&tEthIFStatsObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Device.Ethernet.Interface.{i}
Device.Ethernet.Link.{i}
*******************************************************************************/
struct CWMP_PRMT tEthIFObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Interface",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"Link",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eEthIFObject
{
	eEthInterface,
	eEthLink
};

struct CWMP_NODE tEthIFObject[] =
{
	/*info,  					leaf,			node)*/
	{&tEthIFObjectInfo[eEthInterface],	NULL,	&tInterfaceEthIFObject},
	{&tEthIFObjectInfo[eEthLink],	NULL,	&tLinkEthIFObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int getEthIFEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char *lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	unsigned int instnum;
	struct eth_port_if epi;

	instnum = getInstanceNum(name, "Interface");
	
	if (instnum < 1 || instnum > ETHERNET_NUM)
		return ERR_9007;

	sprintf(ifname, "%s%d", "port", instnum-1);

	//tr181_printf("Interface.%d -> %s", instnum, ifname);

	if(getEthPortIf(ifname, &epi) < 0)
		return ERR_9002;


	if ((name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp(lastname, "Enable")==0 )
	{
		int disable = 0;

		if (instnum == 1)
			mib_get( MIB_CWMP_SW_PORT1_DISABLE, (void *)&disable);
		else if (instnum == 2)
			mib_get( MIB_CWMP_SW_PORT2_DISABLE, (void *)&disable);
		else if (instnum == 3)
			mib_get( MIB_CWMP_SW_PORT3_DISABLE, (void *)&disable);
		else if (instnum == 4)
			mib_get( MIB_CWMP_SW_PORT4_DISABLE, (void *)&disable);
		else
			mib_get( MIB_CWMP_SW_PORT5_DISABLE, (void *)&disable);

		*data = booldup(disable?0:1);
	}
	else if( strcmp(lastname, "Status")==0 )
	{
		*data = strdup(epi.status);
	}
	else if( strcmp(lastname, "LastChange")==0 )
	{
		*data = uintdup(epi.lastChange);
	}
	else if( strcmp(lastname, "Upstream")==0 )
	{
		if (strcmp(epi.upStream, "true") == 0)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp(lastname, "MaxBitRate")==0 )
	{
		int maxBitRate = 0;

		if (instnum == 1)
			mib_get( MIB_CWMP_SW_PORT1_MAXBITRATE, (void *)&maxBitRate);
		else if (instnum == 2)
			mib_get( MIB_CWMP_SW_PORT2_MAXBITRATE, (void *)&maxBitRate);
		else if (instnum == 3)
			mib_get( MIB_CWMP_SW_PORT3_MAXBITRATE, (void *)&maxBitRate);
		else if (instnum == 4)
			mib_get( MIB_CWMP_SW_PORT4_MAXBITRATE, (void *)&maxBitRate);
		else
			mib_get( MIB_CWMP_SW_PORT5_MAXBITRATE, (void *)&maxBitRate);

		*data = intdup(maxBitRate);
	}
	else if( strcmp(lastname, "DuplexMode")==0 )
	{
		char duplexMode[5];

		if (instnum == 1)
			mib_get( MIB_CWMP_SW_PORT1_DUPLEXMODE, (void *)duplexMode);
		else if (instnum == 2)
			mib_get( MIB_CWMP_SW_PORT2_DUPLEXMODE, (void *)duplexMode);
		else if (instnum == 3)
			mib_get( MIB_CWMP_SW_PORT3_DUPLEXMODE, (void *)duplexMode);
		else if (instnum == 4)
			mib_get( MIB_CWMP_SW_PORT4_DUPLEXMODE, (void *)duplexMode);
		else
			mib_get( MIB_CWMP_SW_PORT5_DUPLEXMODE, (void *)duplexMode);
		
		*data = strdup(duplexMode);
	}
	else
		return ERR_9005;

	return 0;
#endif
	
	return 0;
}

int setEthIFEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 0 // rewrite
	char *lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	char cmd[512];
	char *buf = data;
	unsigned int instnum;
	unsigned int vInt;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	instnum = getInstanceNum(name, "Interface");
	
	if (instnum < 1 || instnum > ETHERNET_NUM)
		return ERR_9007;

	sprintf(ifname, "%s%d", "if", instnum-1);

	//tr181_printf("Interface.%d -> %s", instnum, ifname);

	if (strcmp(lastname, "Enable") == 0) 
	{
		int *i = data;

		if (i == NULL) return ERR_9007;
		vInt = (*i==0)?1:0;

		if (instnum == 1)
			mib_set(MIB_CWMP_SW_PORT1_DISABLE, &vInt);
		else if (instnum == 2)
			mib_set(MIB_CWMP_SW_PORT2_DISABLE, &vInt);
		else if (instnum == 3)
			mib_set(MIB_CWMP_SW_PORT3_DISABLE, &vInt);
		else if (instnum == 4)
			mib_set(MIB_CWMP_SW_PORT4_DISABLE, &vInt);
		else
			mib_set(MIB_CWMP_SW_PORT5_DISABLE, &vInt);

	}
	else if (strcmp(lastname, "MaxBitRate") == 0) 
	{
		int *i = data;

		if (i == NULL) return ERR_9007;
		if ((*i == -1) || (*i == 10) || (*i == 100) || (*i == 1000)) {
			if (instnum == 1)
				mib_set(MIB_CWMP_SW_PORT1_MAXBITRATE, i);
			else if (instnum == 2)
				mib_set(MIB_CWMP_SW_PORT2_MAXBITRATE, i);
			else if (instnum == 3)
				mib_set(MIB_CWMP_SW_PORT3_MAXBITRATE, i);
			else if (instnum == 4)
				mib_set(MIB_CWMP_SW_PORT4_MAXBITRATE, i);
			else
				mib_set(MIB_CWMP_SW_PORT5_MAXBITRATE, i);
		}
		else
			return ERR_9007;
	}
	else if (strcmp(lastname, "DuplexMode") == 0) 
	{
		if (buf == NULL) return ERR_9007;
		if ((strcmp(buf, "Half") == 0) ||
			(strcmp(buf, "Full") == 0) ||
			(strcmp(buf, "Auto") == 0)) {
			
			if (instnum == 1)
				mib_set(MIB_CWMP_SW_PORT1_DUPLEXMODE, buf);
			else if (instnum == 2)
				mib_set(MIB_CWMP_SW_PORT2_DUPLEXMODE, buf);
			else if (instnum == 3)
				mib_set(MIB_CWMP_SW_PORT3_DUPLEXMODE, buf);
			else if (instnum == 4)
				mib_set(MIB_CWMP_SW_PORT4_DUPLEXMODE, buf);
			else
				mib_set(MIB_CWMP_SW_PORT5_DUPLEXMODE, buf);
		}
		else
			return ERR_9007;
	}
	else
		return ERR_9005;
#endif
	return 1;
}

int getEthIFStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	unsigned int instnum;
	struct user_net_device_stats nds;

	instnum = getInstanceNum(name, "Interface");
	if (instnum < 1 || instnum > ETHERNET_NUM)
		return ERR_9007;

	sprintf(ifname, "%s%d", "port", instnum-1);

	//tr181_printf("Interface.%d -> %s", instnum, ifname);
	
	if(getEthStats(ifname, &nds) < 0)
		return ERR_9002;


	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp(lastname, "BytesSent")==0 )
	{
		*data = uintdup(nds.tx_bytes);
	}
	else if( strcmp(lastname, "BytesReceived")==0 )
	{
		*data = uintdup(nds.rx_bytes);
	}
	else if( strcmp(lastname, "PacketsSent")==0 )
	{
		*data = uintdup(nds.tx_packets);
	}
	else if( strcmp(lastname, "PacketsReceived")==0 )
	{
		*data = uintdup(nds.rx_packets);
	}
	else if (strcmp(lastname, "ErrorsSent") == 0) 
	{	
		*data = uintdup(nds.tx_errors);
	} 
	else if (strcmp(lastname, "ErrorsReceived") == 0) 
	{	
		*data = uintdup(nds.rx_errors);
	} 
	else if (strcmp(lastname, "UnicastPacketsSent") == 0) 
	{
		*data = uintdup(nds.tx_unicast);
	} 
	else if (strcmp(lastname, "UnicastPacketsReceived") == 0) 
	{	
		*data = uintdup(nds.rx_unicast);
	} 
	else if (strcmp(lastname, "DiscardPacketsSent") == 0) 
	{
		*data = uintdup(nds.tx_dropped);
	} 
	else if (strcmp(lastname, "DiscardPacketsReceived") == 0) 
	{
		*data = uintdup(nds.rx_dropped);
	} 
	else if (strcmp(lastname, "MulticastPacketsSent") == 0) 
	{	
		*data = uintdup(nds.tx_multicast);
	} 
	else if (strcmp(lastname, "MulticastPacketsReceived") == 0) 
	{
		*data = uintdup(nds.rx_multicast);
	} 
	else if (strcmp(lastname, "BroadcastPacketsSent") == 0) 
	{	
		*data = uintdup(nds.tx_broadcast);
	} 
	else if (strcmp(lastname, "BroadcastPacketsReceived") == 0) 
	{	
		*data = uintdup(nds.rx_broadcast);	
	} 
	else if (strcmp(lastname, "UnknownProtoPacketsReceived") == 0) 
	{
		*data = uintdup( 0 );
	}
	else
		return ERR_9005;
#endif
	return 0;
}

int getEthLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	char	buf[256]="";
	unsigned int instnum;
	unsigned int vInt;
	struct eth_link_if eli;
	
	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	instnum = getInstanceNum(name, "Link");
	if (instnum == 0)
		return ERR_9007;

	snprintf(ifname, sizeof(ifname), "eth%u", instnum - 1);

	if(getEthLink(ifname, &eli) < 0)
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "Enable") == 0)
	{
		int disable = 0;

		if (instnum == 1)
			mib_get( MIB_CWMP_LAN_ETHIFDISABLE, (void *)&disable);
		else
			mib_get( MIB_CWMP_WAN_ETHIFDISABLE, (void *)&disable);

		*data = booldup(disable?0:1);
	}
	else if (strcmp(lastname, "Status") == 0)
	{
		*data = strdup(eli.status);
	}
	else if (strcmp(lastname, "Name") == 0)
	{
		*data = strdup(eli.name);	
	}
	else if (strcmp(lastname, "LastChange") == 0)
	{
		*data = uintdup(eli.LastChange);
	}
	else if (strcmp(lastname, "MACAddress") == 0)
	{
		*data=strdup(eli.MACAddress);
	}
	else if( strcmp( lastname, "PriorityTagging" )==0 )
	{
		*data = booldup( false );
	}
	else 
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int setEthLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	unsigned int vInt;
	unsigned int instnum;
	char ifname[IF_NAME_SIZE];
	char	*buf=data;
	
	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	instnum = getInstanceNum(name, "Link");
	if (instnum == 0)
		return ERR_9007;

	snprintf(ifname, sizeof(ifname), "eth%u", instnum - 1);

	if (instnum == 1) // lan eth0
	{
		if (strcmp(lastname, "Enable") == 0) 
		{
			int *i = data;

			if( i==NULL ) return ERR_9007;
			vInt = (*i==0)?1:0;
			mib_set(MIB_CWMP_LAN_ETHIFDISABLE, &vInt);
			
			return 1;
		}
		else 
		{
			return ERR_9005;
		}
	}
	else if (instnum == 2) // wan eth1
	{
		if( strcmp( lastname, "Enable" )==0 )
		{
			int entrynum;
			int *i = data;

			if( i == NULL ) return ERR_9007;
			vInt = (*i==0)?1:0;
			mib_set(MIB_CWMP_WAN_ETHIFDISABLE, &vInt);

			return 1;
		}
		else
		{
			return ERR_9005;
		}
	}
	else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int getEthLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	unsigned int instnum;
	struct user_net_device_stats nds;

	instnum = getInstanceNum(name, "Link");
	if (instnum == 0)
		return ERR_9007;

	if (instnum == 1)
	{
		sprintf(ifname, "%s","eth0");
	}
	else if(instnum == 2)
	{
		sprintf(ifname, "%s","eth1");
	}
	else
	{
		sprintf(ifname, "%s","eth0");
	}
	
	if(getEthStats(ifname, &nds) < 0)
		return ERR_9002;


	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp(lastname, "BytesSent")==0 )
	{
		*data = uintdup(nds.tx_bytes);
	}
	else if( strcmp(lastname, "BytesReceived")==0 )
	{
		*data = uintdup(nds.rx_bytes);
	}
	else if( strcmp(lastname, "PacketsSent")==0 )
	{
		*data = uintdup(nds.tx_packets);
	}
	else if( strcmp(lastname, "PacketsReceived")==0 )
	{
		*data = uintdup(nds.rx_packets);
	}
	else if (strcmp(lastname, "ErrorsSent") == 0) 
	{	
		*data = uintdup(nds.tx_errors);
	} 
	else if (strcmp(lastname, "ErrorsReceived") == 0) 
	{	
		*data = uintdup(nds.rx_errors);
	} 
	else if (strcmp(lastname, "UnicastPacketsSent") == 0) 
	{
		*data = uintdup(nds.tx_unicast);
	} 
	else if (strcmp(lastname, "UnicastPacketsReceived") == 0) 
	{	
		*data = uintdup(nds.rx_unicast);
	} 
	else if (strcmp(lastname, "DiscardPacketsSent") == 0) 
	{
		*data = uintdup(nds.tx_dropped);
	} 
	else if (strcmp(lastname, "DiscardPacketsReceived") == 0) 
	{
		*data = uintdup(nds.rx_dropped);
	} 
	else if (strcmp(lastname, "MulticastPacketsSent") == 0) 
	{	
		*data = uintdup(nds.tx_multicast);
	} 
	else if (strcmp(lastname, "MulticastPacketsReceived") == 0) 
	{
		*data = uintdup(nds.rx_multicast);
	} 
	else if (strcmp(lastname, "BroadcastPacketsSent") == 0) 
	{	
		*data = uintdup(nds.tx_broadcast);
	} 
	else if (strcmp(lastname, "BroadcastPacketsReceived") == 0) 
	{	
		*data = uintdup(nds.rx_broadcast);	
	} 
	else if (strcmp(lastname, "UnknownProtoPacketsReceived") == 0) 
	{
		*data = uintdup( 0 );
	}
	else
		return ERR_9005;
#endif
	return 0;
}

