#include "tr181_bridge.h"


#define		NUM_BRIDGE_PORT		5

/*******************************************************************************
Device.Bridging.Bridge.{i}.VLANPort.{i}. Entity
*******************************************************************************/
struct CWMP_OP tBrVlanPortEntityLeafOP = { getBrVlanPortEntity, setBrVlanPortEntity };
struct CWMP_PRMT tBrVlanPortEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrVlanPortEntityLeafOP},
	{"Alias",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrVlanPortEntityLeafOP},
	{"VLAN",    eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrVlanPortEntityLeafOP},
	{"Port",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrVlanPortEntityLeafOP},
	{"Untagged",eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrVlanPortEntityLeafOP},
};
enum eBrVlanPortEntityLeaf
{
	eBrVlanPortEnable,
	eBrVlanPortAlias,
	eBrVlanPortVLAN,
	eBrVlanPortPort,
	eBrVlanPortUntagged
};
struct CWMP_LEAF tBrVlanPortEntityLeaf[] =
{
	{ &tBrVlanPortEntityLeafInfo[eBrVlanPortEnable] },
	{ &tBrVlanPortEntityLeafInfo[eBrVlanPortAlias] },
	{ &tBrVlanPortEntityLeafInfo[eBrVlanPortVLAN] },
	{ &tBrVlanPortEntityLeafInfo[eBrVlanPortPort] },
	{ &tBrVlanPortEntityLeafInfo[eBrVlanPortUntagged] },
	{ NULL }
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.VLAN.{i}. Entity
*******************************************************************************/
struct CWMP_OP tBrVlanEntityLeafOP = { getBrVlanEntity, setBrVlanEntity };
struct CWMP_PRMT tBrVlanEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrVlanEntityLeafOP},
	{"Alias",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrVlanEntityLeafOP},
	{"Name",    eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrVlanEntityLeafOP},
	{"VLANID",	eCWMP_tINT,		CWMP_READ|CWMP_WRITE,	&tBrVlanEntityLeafOP},
};
enum eBrVlanEntityLeaf
{
	eBrVlanEnable,
	eBrVlanAlias,
	eBrVlanName,
	eBrVlanVLANID
};
struct CWMP_LEAF tBrVlanEntityLeaf[] =
{
	{ &tBrVlanEntityLeafInfo[eBrVlanEnable] },
	{ &tBrVlanEntityLeafInfo[eBrVlanAlias] },
	{ &tBrVlanEntityLeafInfo[eBrVlanName] },
	{ &tBrVlanEntityLeafInfo[eBrVlanVLANID] },
	{ NULL }
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.Port.{i}. Entity
*******************************************************************************/
struct CWMP_OP tBrPortEntityLeafOP = { getBrPortEntity, setBrPortEntity };
struct CWMP_PRMT tBrPortEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",              eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
	{"Status",              eCWMP_tSTRING,	CWMP_READ,	&tBrPortEntityLeafOP},
//	{"Alias",               eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
	{"Name",                eCWMP_tSTRING,	CWMP_READ,	&tBrPortEntityLeafOP},
	{"LastChange",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortEntityLeafOP},
//	{"LowerLayers",         eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
//	{"ManagementPort",      eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
//	{"DefaultUserPriority", eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
//	{"PriorityRegeneration",eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
//	{"PortState",           eCWMP_tSTRING,	CWMP_READ,	&tBrPortEntityLeafOP},
//	{"PVID",                eCWMP_tINT,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
//	{"AcceptableFrameTypes",eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
//	{"IngressFiltering",    eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
//	{"PriorityTagging",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrPortEntityLeafOP},
};
enum eBrPortEntityLeaf
{
	eBrPortEnable,
	eBrPortStatus,
//	eBrPortAlias,
	eBrPortName,
	eBrPortLastChange,
//	eBrPortLowerLayers,
//	eBrPortManagementPort,
//	eBrPortDefaultUserPriority,
//	eBrPortPriorityRegeneration,
//	eBrPortPortState,
//	eBrPortPVID,
//	eBrPortAcceptableFrameTypes,
//	eBrPortIngressFiltering,
//	eBrPortPriorityTagging
};
struct CWMP_LEAF tBrPortEntityLeaf[] =
{
	{ &tBrPortEntityLeafInfo[eBrPortEnable] },
	{ &tBrPortEntityLeafInfo[eBrPortStatus] },
//	{ &tBrPortEntityLeafInfo[eBrPortAlias] },
	{ &tBrPortEntityLeafInfo[eBrPortName] },
	{ &tBrPortEntityLeafInfo[eBrPortLastChange] },
//	{ &tBrPortEntityLeafInfo[eBrPortLowerLayers] },
//	{ &tBrPortEntityLeafInfo[eBrPortManagementPort] },
//	{ &tBrPortEntityLeafInfo[eBrPortDefaultUserPriority] },
//	{ &tBrPortEntityLeafInfo[eBrPortPriorityRegeneration] },
//	{ &tBrPortEntityLeafInfo[eBrPortPortState] },
//	{ &tBrPortEntityLeafInfo[eBrPortPVID] },
//	{ &tBrPortEntityLeafInfo[eBrPortAcceptableFrameTypes] },
//	{ &tBrPortEntityLeafInfo[eBrPortIngressFiltering] },
//	{ &tBrPortEntityLeafInfo[eBrPortPriorityTagging] },
	{ NULL }
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.Port.{i}.Stats.
*******************************************************************************/
struct CWMP_OP tBrPortStEntityLeafOP = { getBrPortStats, NULL };
struct CWMP_PRMT tBrPortStEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"BytesSent",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP}, // unsigned long?
	{"BytesReceived",               eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"PacketsSent",                 eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"PacketsReceived",             eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"ErrorsSent",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP}, // unsigned int
	{"ErrorsReceived",              eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP}, // unsigned int
	{"UnicastPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"UnicastPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"DiscardPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP}, // unsigned int
	{"DiscardPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP}, // unsigned int
	{"MulticastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"MulticastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"BroadcastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
	{"BroadcastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
//	{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBrPortStEntityLeafOP},
};
enum eBrPortStEntityLeaf
{
	eBrPortStBytesSent,
	eBrPortStBytesReceived,
	eBrPortStPacketsSent,
	eBrPortStPacketsReceived,
	eBrPortStErrorsSent,
	eBrPortStErrorsReceived,
	eBrPortStUnicastPacketsSent,
	eBrPortStUnicastPacketsReceived,
	eBrPortStDiscardPacketsSent,
	eBrPortStDiscardPacketsReceived,
	eBrPortStMulticastPacketsSent,
	eBrPortStMulticastPacketsReceived,
	eBrPortStBroadcastPacketsSent,
	eBrPortStBroadcastPacketsReceived,
//	eBrPortStUnknownProtoPacketsReceived
};
struct CWMP_LEAF tBrPortStEntityLeaf[] =
{
	{ &tBrPortStEntityLeafInfo[eBrPortStBytesSent] },
	{ &tBrPortStEntityLeafInfo[eBrPortStBytesReceived] },
	{ &tBrPortStEntityLeafInfo[eBrPortStPacketsSent] },
	{ &tBrPortStEntityLeafInfo[eBrPortStPacketsReceived] },
	{ &tBrPortStEntityLeafInfo[eBrPortStErrorsSent] },
	{ &tBrPortStEntityLeafInfo[eBrPortStErrorsReceived] },
	{ &tBrPortStEntityLeafInfo[eBrPortStUnicastPacketsSent] },
	{ &tBrPortStEntityLeafInfo[eBrPortStUnicastPacketsReceived] },
	{ &tBrPortStEntityLeafInfo[eBrPortStDiscardPacketsSent] },
	{ &tBrPortStEntityLeafInfo[eBrPortStDiscardPacketsReceived] },
	{ &tBrPortStEntityLeafInfo[eBrPortStMulticastPacketsSent] },
	{ &tBrPortStEntityLeafInfo[eBrPortStMulticastPacketsReceived] },
	{ &tBrPortStEntityLeafInfo[eBrPortStBroadcastPacketsSent] },
	{ &tBrPortStEntityLeafInfo[eBrPortStBroadcastPacketsReceived] },
//	{ &tBrPortStEntityLeafInfo[eBrPortStUnknownProtoPacketsReceived] },
	{ NULL }
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.Port.{i}.Stats.
*******************************************************************************/
struct CWMP_PRMT tBrPortStatsObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Stats",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eBrPortStatsObject
{
	eBrPortStats
};

struct CWMP_NODE tBrPortStatsObject[] =
{
	/*info,  					leaf,			node)*/
	{&tBrPortStatsObjectInfo[eBrPortStats],	&tBrPortStEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.Port.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkBrPortOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"5",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"6",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"7",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"8",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"9",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"10",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"11",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"12",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"13",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eLinkBrPortOjbect
{
	eLinkBrPort1,
	eLinkBrPort2,
	eLinkBrPort3,
	eLinkBrPort4,
	eLinkBrPort5,
	eLinkBrPort6,
	eLinkBrPort7,
	eLinkBrPort8,
	eLinkBrPort9,
	eLinkBrPort10,
	eLinkBrPort11,
	eLinkBrPort12,
	eLinkBrPort13
};
struct CWMP_NODE tLinkBrPortObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkBrPortOjbectInfo[eLinkBrPort1],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort2],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort3],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort4],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort5],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort6],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort7],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort8],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort9],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort10],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort11],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort12],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{&tLinkBrPortOjbectInfo[eLinkBrPort13],	tBrPortEntityLeaf,	tBrPortStatsObject},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.VLAN.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkBrVlanOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eLinkBrVlanOjbect
{
	eLinkBrVlan0
};
struct CWMP_LINKNODE tLinkBrVlanObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkBrVlanOjbectInfo[eLinkBrVlan0],	tBrVlanEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.VLANPort.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkBrVlanPortOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eLinkBrVlanPortOjbect
{
	eLinkBrVlanPort0
};
struct CWMP_LINKNODE tLinkBrVlanPortObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkBrVlanPortOjbectInfo[eLinkBrVlanPort0],	tBrVlanPortEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.Bridging.Bridge.{i}.Port.{i}.
Device.Bridging.Bridge.{i}.VLAN.{i}.
Device.Bridging.Bridge.{i}.VLANPort.{i}.
*******************************************************************************/
struct CWMP_PRMT tBrPortObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Port",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
//	{"VLAN",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
//	{"VLANPort",eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

enum eBrPortObject
{
	eBrPort,
//	eBrVLAN,
//	eBrVLANPort
};

struct CWMP_NODE tBrPortObject[] =
{
	/*info,  					leaf,			node)*/
	{&tBrPortObjectInfo[eBrPort],	NULL,	tLinkBrPortObject},
//	{&tBrPortObjectInfo[eBrVLAN],	NULL,	NULL},
//	{&tBrPortObjectInfo[eBrVLANPort],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Device.Bridging.Bridge.{i} Entity
*******************************************************************************/
struct CWMP_OP tBrEntityLeafOP = { getBrEntity, setBrEntity };
struct CWMP_PRMT tBrEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                 eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBrEntityLeafOP},
	{"Status",                 eCWMP_tSTRING,	CWMP_READ,	&tBrEntityLeafOP},
//	{"Alias",                  eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrEntityLeafOP},
	{"Standard",               eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBrEntityLeafOP},
	{"PortNumberOfEntries",    eCWMP_tUINT,	CWMP_READ,	&tBrEntityLeafOP},
//	{"VLANNumberOfEntries",    eCWMP_tUINT,	CWMP_READ,	&tBrEntityLeafOP},
//	{"VLANPortNumberOfEntries",eCWMP_tUINT,	CWMP_READ,	&tBrEntityLeafOP},
};
enum eBrEntityLeaf
{
	eBrEnable,
	eBrStatus,
//	eBrAlias,
	eBrStandard,
	eBrPortNumberOfEntries,
//	eBrVLANNumberOfEntries,
//	eBrVLANPortNumberOfEntries,
};
struct CWMP_LEAF tBrEntityLeaf[] =
{
	{ &tBrEntityLeafInfo[eBrEnable] },
	{ &tBrEntityLeafInfo[eBrStatus] },
//	{ &tBrEntityLeafInfo[eBrAlias] },
	{ &tBrEntityLeafInfo[eBrStandard] },
	{ &tBrEntityLeafInfo[eBrPortNumberOfEntries] },
//	{ &tBrEntityLeafInfo[eBrVLANNumberOfEntries] },
//	{ &tBrEntityLeafInfo[eBrVLANPortNumberOfEntries] },
	{ NULL }
};

/*******************************************************************************
Device.Bridging.Bridge.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkBrOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkBrOjbect
{
	eLinkBr1
};
struct CWMP_NODE tLinkBrObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkBrOjbectInfo[eLinkBr1],	tBrEntityLeaf,	tBrPortObject},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.Bridging.Bridge.{i}
*******************************************************************************/
struct CWMP_PRMT tBrObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Bridge",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eBrObject
{
	eBr
};

struct CWMP_NODE tBrObject[] =
{
	/*info,  					leaf,			node)*/
	{&tBrObjectInfo[eBr],	NULL,	tLinkBrObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int getBrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup( 1 ); 
	}else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup("Enabled"); 
	}else if( strcmp( lastname, "Standard" )==0 )
	{
		*data = strdup("802.1D-2004"); 
	}else if( strcmp( lastname, "PortNumberOfEntries" )==0 )
	{
		*data = uintdup( NUM_BRIDGE_PORT ); 
	}else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int setBrEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return ERR_9008;
}

int getBrPortEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	unsigned int instnum;
	struct eth_port_if epi;
	int rootIdx=0,vwlanIdx=0;
	unsigned int vChar=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instnum = getInstanceNum(name, "Port");

	//tr181_printf("Bridge.1.Port. -> %d", instnum);

	if (instnum == 1)
		sprintf(ifname, "%s", "port0");
	else if (instnum == 2)
		sprintf(ifname, "%s", "port2");
	else if (instnum == 3)
		sprintf(ifname, "%s", "port7");
	else if (instnum == 4)
	{
		sprintf(ifname, "%s", "wlan0");
		rootIdx = 0;
		vwlanIdx = 0;
	}
	else if (instnum == 9)
	{
		sprintf(ifname, "%s", "wlan1");
		rootIdx = 1;
		vwlanIdx = 0;
	}
	else if (instnum >=5 && instnum <= 8)
	{
		rootIdx = 0;
		vwlanIdx = instnum - 5;
		sprintf(ifname, "wlan%d-va%d", rootIdx, vwlanIdx);
	}
	else if (instnum >=10 && instnum <= 13)
	{
		rootIdx = 1;
		vwlanIdx = instnum - 10;
		sprintf(ifname, "wlan%d-va%d", rootIdx, vwlanIdx);
	}
	else
		return ERR_9005;

	//tr181_printf("IF: %s", ifname);

	*type = entity->info->type;
	*data = NULL;

	if (instnum >= 1 && instnum <= 3)
	{
		if(getEthPortIf(ifname, &epi) < 0)
			return ERR_9002;
		
		if( strcmp( lastname, "Enable" )==0 )
		{
			if (strcmp(epi.enable, "true") == 0)
				*data = booldup(1);
			else
				*data = booldup(0);
		}else if( strcmp( lastname, "Status" )==0 )
		{
			*data = strdup(epi.status);
		}else if( strcmp( lastname, "Name" )==0 )
		{
			*data = strdup(ifname);
		}else if( strcmp( lastname, "LastChange" )==0 )
		{
			*data = uintdup(epi.lastChange);
		}else
		{
			return ERR_9005;
		}
	}
	else if (instnum >= 4 && instnum <= 13)
	{
		if( strcmp( lastname, "Enable" )==0 )
		{
			int func_off;
		
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&func_off);

			if(vChar==1 || func_off == 1)
				*data = booldup( 0 );
			else
				*data = booldup( 1 );
		}else if( strcmp( lastname, "Status" )==0 )
		{
			int flags=0;
			if( getInFlags(ifname, &flags)==1 )
			{
				if (flags & IFF_UP)
					*data = strdup( "Up" );
				else
					*data = strdup( "Disabled" );
			}else
				*data = strdup( "Error" );
		}else if( strcmp( lastname, "Name" )==0 )
		{
			*data = strdup(ifname);
		}else if( strcmp( lastname, "LastChange" )==0 )
		{
			*data = uintdup( 0 );
		}else
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

int setBrPortEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	char cmd[512];
	char *buf = data;
	unsigned int instnum;
	int isWLANMIBUpdated=0;
	unsigned int	vChar=0;
	int rootIdx=0,vwlanIdx=0;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	instnum = getInstanceNum(name, "Port");
	
	if (instnum == 1)
		sprintf(ifname, "%s", "if0");
	else if (instnum == 2)
		sprintf(ifname, "%s", "if2");
	else if (instnum == 3)
		sprintf(ifname, "%s", "if7");
	else if (instnum == 4)
	{
		sprintf(ifname, "%s", "wlan0");
		rootIdx = 0;
		vwlanIdx = 0;
	}
	else if (instnum == 9)
	{
		sprintf(ifname, "%s", "wlan1");
		rootIdx = 1;
		vwlanIdx = 0;
	}
	else if (instnum >=5 && instnum <= 8)
	{
		rootIdx = 0;
		vwlanIdx = instnum - 5;
		sprintf(ifname, "wlan%d-va%d", rootIdx, vwlanIdx);
	}
	else if (instnum >=10 && instnum <= 13)
	{
		rootIdx = 1;
		vwlanIdx = instnum - 10;
		sprintf(ifname, "wlan%d-va%d", rootIdx, vwlanIdx);
	}
	else
		return ERR_9005;

	if (instnum >= 1 && instnum <= 3)
	{
		if (strcmp(lastname, "Enable") == 0) 
		{
			int *i = data;

			if (i == NULL) return ERR_9007;

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "echo set eth %s Enable %s > /proc/rtl865x/tr181_eth_set",
				ifname, (*i == 1 ? "true" : "false"));
			//tr181_printf("%s", cmd);

			system(cmd);		
		}
		else
			return ERR_9005;
	}
	else if (instnum >= 4 && instnum <= 13)
	{
		if (strcmp(lastname, "Enable") == 0) 
		{
			int *i = data;

			if( i==NULL ) return ERR_9007;
			
			vChar = (*i==0)?1:0;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&vChar);
			isWLANMIBUpdated=1;
		}
		else
			return ERR_9005;
	}
	else
	{
		return ERR_9005;
	}
	
	if(isWLANMIBUpdated)
		return 1;
	else
		return 0;
}

int getBrVlanEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int setBrVlanEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

int getBrVlanPortEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int setBrVlanPortEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

int getBrPortStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	unsigned int instnum;
	int rootIdx=0,vwlanIdx=0;
	struct user_net_device_stats nds;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	instnum = getInstanceNum(name, "Port");

	//tr181_printf("Bridge.1.Port. -> %d", instnum);

	if (instnum == 1)
		sprintf(ifname, "%s", "port0");
	else if (instnum == 2)
		sprintf(ifname, "%s", "port2");
	else if (instnum == 3)
		sprintf(ifname, "%s", "port7");
	else if (instnum == 4)
	{
		sprintf(ifname, "%s", "wlan0");
		rootIdx = 0;
		vwlanIdx = 0;
	}
	else if (instnum == 9)
	{
		sprintf(ifname, "%s", "wlan1");
		rootIdx = 1;
		vwlanIdx = 0;
	}
	else if (instnum >=5 && instnum <= 8)
	{
		rootIdx = 0;
		vwlanIdx = instnum - 5;
		sprintf(ifname, "wlan%d-va%d", rootIdx, vwlanIdx);
	}
	else if (instnum >=10 && instnum <= 13)
	{
		rootIdx = 1;
		vwlanIdx = instnum - 10;
		sprintf(ifname, "wlan%d-va%d", rootIdx, vwlanIdx);
	}
	else
		return ERR_9005;

	//tr181_printf("IF: %s", ifname);

	*type = entity->info->type;
	*data = NULL;

	if (instnum >= 1 && instnum <= 3)
	{
		if(getEthStats(ifname, &nds) < 0)
			return ERR_9002;
	}
	else if (instnum >= 4 && instnum <= 13)
	{
		if(getStats(ifname, &nds) < 0)
			return ERR_9002;
	}
	else
		return ERR_9005;

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

