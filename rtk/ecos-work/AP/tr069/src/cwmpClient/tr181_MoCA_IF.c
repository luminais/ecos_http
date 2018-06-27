#include "tr181_MoCA_IF.h"


/*******************************************************************************
Device.MoCA.Interface.{i}.AssociatedDevice.{i} Entity
*******************************************************************************/
struct CWMP_OP tMoCAIFAssocDevEntityLeafOP = { getMoCAIFAssocDevEntity, NULL };
struct CWMP_PRMT tMoCAIFAssocDevEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"MACAddress",					eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP}, // type: MACAddress
	{"NodeID",                      eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"PreferredNC",                 eCWMP_tBOOLEAN,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"HighestVersion",              eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"PHYTxRate",                   eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"PHYRxRate",                   eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"TxPowerControlReduction",     eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"RxPowerLevel",                eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFAssocDevEntityLeafOP},
	{"TxBcastRate",                 eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"RxBcastPowerLevel",           eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFAssocDevEntityLeafOP},
	{"TxPackets",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFAssocDevEntityLeafOP},
	{"RxPackets",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFAssocDevEntityLeafOP},
	{"RxErroredAndMissedPackets",   eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"QAM256Capable",               eCWMP_tBOOLEAN,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"PacketAggregationCapability", eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"RxSNR",						eCWMP_tUINT,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
	{"Active",						eCWMP_tBOOLEAN,	CWMP_READ,	&tMoCAIFAssocDevEntityLeafOP},
};
enum eMoCAIFAssocDevEntityLeaf
{
	eMoCAIFAssocDevMACAddress,
	eMoCAIFAssocDevNodeID,
	eMoCAIFAssocDevPreferredNC,
	eMoCAIFAssocDevHighestVersion,
	eMoCAIFAssocDevPHYTxRate,
	eMoCAIFAssocDevPHYRxRate,
	eMoCAIFAssocDevTxPowerControlReduction,
	eMoCAIFAssocDevRxPowerLevel,
	eMoCAIFAssocDevTxBcastRate,
	eMoCAIFAssocDevRxBcastPowerLevel,
	eMoCAIFAssocDevTxPackets,
	eMoCAIFAssocDevRxPackets,
	eMoCAIFAssocDevRxErroredAndMissedPackets,
	eMoCAIFAssocDevQAM256Capable,
	eMoCAIFAssocDevPacketAggregationCapability,
	eMoCAIFAssocDevRxSNR,
	eMoCAIFAssocDevActive
};
struct CWMP_LEAF tMoCAIFAssocDevEntityLeaf[] =
{
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevMACAddress] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevNodeID] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevPreferredNC] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevHighestVersion] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevPHYTxRate] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevPHYRxRate] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevTxPowerControlReduction] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevRxPowerLevel] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevTxBcastRate] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevRxBcastPowerLevel] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevTxPackets] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevRxPackets] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevRxErroredAndMissedPackets] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevQAM256Capable] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevPacketAggregationCapability] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevRxSNR] },
	{ &tMoCAIFAssocDevEntityLeafInfo[eMoCAIFAssocDevActive] },
	{ NULL }
};

/*******************************************************************************
Device.MoCA.Interface.{i}.AssociatedDevice.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkMoCAIFAssocDevOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkMoCAIFAssocDevOjbect
{
	eLinkMoCAIFAssocDev0
};
struct CWMP_LINKNODE tLinkMoCAIFAssocDevObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkMoCAIFAssocDevOjbectInfo[eLinkMoCAIFAssocDev0],	tMoCAIFAssocDevEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.MoCA.Interface.{i}.QoS.FlowStats.{i} Entity
*******************************************************************************/
struct CWMP_OP tMoCAIFQoSFlowStEntityLeafOP = { getMoCAIFQoSFlowStEntity, NULL };
struct CWMP_PRMT tMoCAIFQoSFlowStEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"FlowID",			eCWMP_tUINT,	CWMP_READ,	&tMoCAIFQoSFlowStEntityLeafOP},
	{"PacketDA",        eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFQoSFlowStEntityLeafOP}, // type: MACAddress
	{"MaxRate",         eCWMP_tUINT,	CWMP_READ,	&tMoCAIFQoSFlowStEntityLeafOP},
	{"MaxBurstSize",    eCWMP_tUINT,	CWMP_READ,	&tMoCAIFQoSFlowStEntityLeafOP},
	{"LeaseTime",       eCWMP_tUINT,	CWMP_READ,	&tMoCAIFQoSFlowStEntityLeafOP},
	{"LeaseTimeLeft",   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFQoSFlowStEntityLeafOP},
	{"FlowPackets",     eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFQoSFlowStEntityLeafOP},
};
enum eMoCAIFQoSFlowStEntityLeaf
{
	eMoCAIFQoSFlowStFlowID,
	eMoCAIFQoSFlowStPacketDA,
	eMoCAIFQoSFlowStMaxRate,
	eMoCAIFQoSFlowStMaxBurstSize,
	eMoCAIFQoSFlowStLeaseTime,
	eMoCAIFQoSFlowStLeaseTimeLeft,
	eMoCAIFQoSFlowStFlowPackets
};
struct CWMP_LEAF tMoCAIFQoSFlowStEntityLeaf[] =
{
	{ &tMoCAIFQoSFlowStEntityLeafInfo[eMoCAIFQoSFlowStFlowID] },
	{ &tMoCAIFQoSFlowStEntityLeafInfo[eMoCAIFQoSFlowStPacketDA] },
	{ &tMoCAIFQoSFlowStEntityLeafInfo[eMoCAIFQoSFlowStMaxRate] },
	{ &tMoCAIFQoSFlowStEntityLeafInfo[eMoCAIFQoSFlowStMaxBurstSize] },
	{ &tMoCAIFQoSFlowStEntityLeafInfo[eMoCAIFQoSFlowStLeaseTime] },
	{ &tMoCAIFQoSFlowStEntityLeafInfo[eMoCAIFQoSFlowStLeaseTimeLeft] },
	{ &tMoCAIFQoSFlowStEntityLeafInfo[eMoCAIFQoSFlowStFlowPackets] },
	{ NULL }
};

/*******************************************************************************
Device.MoCA.Interface.{i}.QoS.FlowStats.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkMoCAIFQoSFlowStatsOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkMoCAIFQoSFlowStatsOjbect
{
	eLinkMoCAIFQoSFlowStats0
};
struct CWMP_LINKNODE tLinkMoCAIFQoSFlowStatsObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkMoCAIFQoSFlowStatsOjbectInfo[eLinkMoCAIFQoSFlowStats0],	tMoCAIFQoSFlowStEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.MoCA.Interface.{i}.QoS.FlowStats.{i}
*******************************************************************************/
struct CWMP_OP tMoCAIFQoSFlowStats_OP = { NULL, objMoCAIFQoSFlowStats };
struct CWMP_PRMT tMoCAIFQoSFlowStatsObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"FlowStats",	eCWMP_tOBJECT,	CWMP_READ,	&tMoCAIFQoSFlowStats_OP},
};

enum eMoCAIFQoSFlowStatsObject
{
	eMoCAIFQoSFlowStats
};

struct CWMP_NODE tMoCAIFQoSFlowStatsObject[] =
{
	/*info,  					leaf,			node)*/
	{&tMoCAIFQoSFlowStatsObjectInfo[eMoCAIFQoSFlowStats],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.MoCA.Interface.{i}.QoS
*******************************************************************************/
struct CWMP_OP tMoCAIFQoSEntityLeafOP = { getMoCAIFQoS, NULL };
struct CWMP_PRMT tMoCAIFQoSEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"EgressNumFlows",			eCWMP_tUINT,	CWMP_READ,	&tMoCAIFQoSEntityLeafOP},
	{"IngressNumFlows",			eCWMP_tUINT,	CWMP_READ,	&tMoCAIFQoSEntityLeafOP},
	{"FlowStatsNumberOfEntries",eCWMP_tUINT,	CWMP_READ,	&tMoCAIFQoSEntityLeafOP},
};
enum eMoCAIFQoSEntityLeaf
{
	eMoCAIFQoSEgressNumFlows,
	eMoCAIFQoSIngressNumFlows,
	eMoCAIFQoSFlowStatsNumberOfEntries
};
struct CWMP_LEAF tMoCAIFQoSEntityLeaf[] =
{
	{ &tMoCAIFQoSEntityLeafInfo[eMoCAIFQoSEgressNumFlows] },
	{ &tMoCAIFQoSEntityLeafInfo[eMoCAIFQoSIngressNumFlows] },
	{ &tMoCAIFQoSEntityLeafInfo[eMoCAIFQoSFlowStatsNumberOfEntries] },
	{ NULL }
};

/*******************************************************************************
Device.MoCA.Interface.{i}.Stats
*******************************************************************************/
struct CWMP_OP tMoCAIFStEntityLeafOP = { getMoCAIFStats, NULL };
struct CWMP_PRMT tMoCAIFStEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"BytesSent",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP}, // unsigned long?
	{"BytesReceived",               eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"PacketsSent",                 eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"PacketsReceived",             eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"ErrorsSent",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP}, // unsigned int
	{"ErrorsReceived",              eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP}, // unsigned int
	{"UnicastPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"UnicastPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"DiscardPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP}, // unsigned int
	{"DiscardPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP}, // unsigned int
	{"MulticastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"MulticastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"BroadcastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"BroadcastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
	{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFStEntityLeafOP},
};
enum eMoCAIFStEntityLeaf
{
	eMoCAIFStBytesSent,
	eMoCAIFStBytesReceived,
	eMoCAIFStPacketsSent,
	eMoCAIFStPacketsReceived,
	eMoCAIFStErrorsSent,
	eMoCAIFStErrorsReceived,
	eMoCAIFStUnicastPacketsSent,
	eMoCAIFStUnicastPacketsReceived,
	eMoCAIFStDiscardPacketsSent,
	eMoCAIFStDiscardPacketsReceived,
	eMoCAIFStMulticastPacketsSent,
	eMoCAIFStMulticastPacketsReceived,
	eMoCAIFStBroadcastPacketsSent,
	eMoCAIFStBroadcastPacketsReceived,
	eMoCAIFStUnknownProtoPacketsReceived
};
struct CWMP_LEAF tMoCAIFStEntityLeaf[] =
{
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStBytesSent] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStBytesReceived] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStPacketsSent] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStPacketsReceived] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStErrorsSent] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStErrorsReceived] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStUnicastPacketsSent] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStUnicastPacketsReceived] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStDiscardPacketsSent] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStDiscardPacketsReceived] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStMulticastPacketsSent] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStMulticastPacketsReceived] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStBroadcastPacketsSent] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStBroadcastPacketsReceived] },
	{ &tMoCAIFStEntityLeafInfo[eMoCAIFStUnknownProtoPacketsReceived] },
	{ NULL }
};

/*******************************************************************************
Device.MoCA.Interface.{i}.Stats
Device.MoCA.Interface.{i}.QoS
Device.MoCA.Interface.{i}.AssociatedDevice
*******************************************************************************/
struct CWMP_OP tMoCAIFAssocDev_OP = { NULL, objMoCAIFAssocDev };
struct CWMP_PRMT tMoCAIFStatsObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"QoS",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"AssociatedDevice",eCWMP_tOBJECT,	CWMP_READ,	&tMoCAIFAssocDev_OP},
};

enum eMoCAIFStatsObject
{
	eMoCAIFStats,
	eMoCAIFQoS,
	eMoCAIFAssociatedDevice
};

struct CWMP_NODE tMoCAIFStatsObject[] =
{
	/*info,  					leaf,			node)*/
	{&tMoCAIFStatsObjectInfo[eMoCAIFStats],	&tMoCAIFStEntityLeaf,	NULL},
	{&tMoCAIFStatsObjectInfo[eMoCAIFQoS],	&tMoCAIFQoSEntityLeaf,	&tMoCAIFQoSFlowStatsObject},
	{&tMoCAIFStatsObjectInfo[eMoCAIFAssociatedDevice],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.MoCA.Interface.{i} Entity
*******************************************************************************/
struct CWMP_OP tMoCAIFEntityLeafOP = { getMoCAIFEntity, setMoCAIFEntity };
struct CWMP_PRMT tMoCAIFEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                          eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"Status",                          eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"Alias",                           eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"Name",                            eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"LastChange",	                    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tMoCAIFEntityLeafOP},
	{"LowerLayers",                     eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP}, // list of string
	{"Upstream",                        eCWMP_tBOOLEAN,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"MACAddress",                      eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP}, // type: MACAddress
	{"FirmwareVersion",                 eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"MaxBitRate",                      eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"MaxIngressBW",                    eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"MaxEgressBW",                     eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"HighestVersion",                  eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"CurrentVersion",                  eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"NetworkCoordinator",              eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"NodeID",                          eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"MaxNodes",                        eCWMP_tBOOLEAN,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"PreferredNC",                     eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"BackupNC",                        eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"PrivacyEnabledSetting",           eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"PrivacyEnabled",                  eCWMP_tBOOLEAN,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"FreqCapabilityMask",              eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP}, // teyp: hexBinary
	{"FreqCurrentMaskSetting",          eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP}, // teyp: hexBinary
	{"FreqCurrentMask",                 eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP}, // teyp: hexBinary
	{"CurrentOperFreq",                 eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"LastOperFreq",                    eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"KeyPassphrase",                   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"TxPowerLimit",                    eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"PowerCntlPhyTarget",              eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"BeaconPowerLimit",                eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tMoCAIFEntityLeafOP},
	{"NetworkTabooMask",                eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP}, // teyp: hexBinary
	{"NodeTabooMask",                   eCWMP_tSTRING,	CWMP_READ,	&tMoCAIFEntityLeafOP}, // teyp: hexBinary
	{"TxBcastRate",                     eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"TxBcastPowerReduction",           eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"QAM256Capable",                   eCWMP_tBOOLEAN,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"PacketAggregationCapability",     eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
	{"AssociatedDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tMoCAIFEntityLeafOP},
};
enum eMoCAIFEntityLeaf
{
	eMoCAIFEnable,
	eMoCAIFStatus,
	eMoCAIFAlias,
	eMoCAIFName,
	eMoCAIFLastChange,
	eMoCAIFLowerLayers,
	eMoCAIFUpstream,
	eMoCAIFMACAddress,
	eMoCAIFFirmwareVersion,
	eMoCAIFMaxBitRate,
	eMoCAIFMaxIngressBW,
	eMoCAIFMaxEgressBW,
	eMoCAIFHighestVersion,
	eMoCAIFCurrentVersion,
	eMoCAIFNetworkCoordinator,
	eMoCAIFNodeID,
	eMoCAIFMaxNodes,
	eMoCAIFPreferredNC,
	eMoCAIFBackupNC,
	eMoCAIFPrivacyEnabledSetting,
	eMoCAIFPrivacyEnabled,
	eMoCAIFFreqCapabilityMask,
	eMoCAIFFreqCurrentMaskSetting,
	eMoCAIFFreqCurrentMask,
	eMoCAIFCurrentOperFreq,
	eMoCAIFLastOperFreq,
	eMoCAIFKeyPassphrase,
	eMoCAIFTxPowerLimit,
	eMoCAIFPowerCntlPhyTarget,
	eMoCAIFBeaconPowerLimit,
	eMoCAIFNetworkTabooMask,
	eMoCAIFNodeTabooMask,
	eMoCAIFTxBcastRate,
	eMoCAIFTxBcastPowerReduction,
	eMoCAIFQAM256Capable,
	eMoCAIFPacketAggregationCapability,
	eMoCAIFAssociatedDeviceNumberOfEntries
};
struct CWMP_LEAF tMoCAIFEntityLeaf[] =
{
	{ &tMoCAIFEntityLeafInfo[eMoCAIFEnable] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFStatus] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFAlias] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFName] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFLastChange] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFLowerLayers] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFUpstream] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFMACAddress] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFFirmwareVersion] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFMaxBitRate] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFMaxIngressBW] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFMaxEgressBW] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFHighestVersion] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFCurrentVersion] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFNetworkCoordinator] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFNodeID] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFMaxNodes] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFPreferredNC] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFBackupNC] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFPrivacyEnabledSetting] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFPrivacyEnabled] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFFreqCapabilityMask] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFFreqCurrentMaskSetting] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFFreqCurrentMask] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFCurrentOperFreq] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFLastOperFreq] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFKeyPassphrase] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFTxPowerLimit] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFPowerCntlPhyTarget] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFBeaconPowerLimit] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFNetworkTabooMask] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFNodeTabooMask] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFTxBcastRate] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFTxBcastPowerReduction] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFQAM256Capable] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFPacketAggregationCapability] },
	{ &tMoCAIFEntityLeafInfo[eMoCAIFAssociatedDeviceNumberOfEntries] },
	{ NULL }
};

/*******************************************************************************
Device.MoCA.Interface.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkMoCAIFOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkMoCAIFOjbect
{
	eLinkMoCAIF0
};
struct CWMP_LINKNODE tLinkMoCAIFObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkMoCAIFOjbectInfo[eLinkMoCAIF0],	tMoCAIFEntityLeaf,	tMoCAIFStatsObject,	NULL,	0},
};

/*******************************************************************************
Device.MoCA.Interface.{i}
*******************************************************************************/
struct CWMP_OP tMoCAIF_OP = { NULL, objMoCAIF };
struct CWMP_PRMT tMoCAIFObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Interface",	eCWMP_tOBJECT,	CWMP_READ,	&tMoCAIF_OP},
};

enum eMoCAIFObject
{
	eMoCAIF
};

struct CWMP_NODE tMoCAIFObject[] =
{
	/*info,  					leaf,			node)*/
	{&tMoCAIFObjectInfo[eMoCAIF],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int objMoCAIF(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int getMoCAIFEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int setMoCAIFEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

int getMoCAIFStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int getMoCAIFQoS(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int objMoCAIFQoSFlowStats(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int getMoCAIFQoSFlowStEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

int objMoCAIFAssocDev(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int getMoCAIFAssocDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}




