#include "tr181_ipIF.h"
#include "prmt_ippingdiag.h"
#include "prmt_utility.h"


/*******************************************************************************
Device.IP.Diagnostics.IPPing.
*******************************************************************************/
struct CWMP_OP tIpDiagIpPingLeafOP = { getIPPingDiag, setIPPingDiag };

struct CWMP_PRMT tIpDiagIpPingLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"DiagnosticsState",    eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE|CWMP_DENY_ACT,	&tIpDiagIpPingLeafOP},
	{"Interface",           eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpDiagIpPingLeafOP},
	{"Host",                eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpDiagIpPingLeafOP},
	{"NumberOfRepetitions", eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tIpDiagIpPingLeafOP},
	{"Timeout",             eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tIpDiagIpPingLeafOP},
	{"DataBlockSize",       eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tIpDiagIpPingLeafOP},
	{"DSCP",                eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tIpDiagIpPingLeafOP},
	{"SuccessCount",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpDiagIpPingLeafOP},
	{"FailureCount",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpDiagIpPingLeafOP},
	{"AverageResponseTime", eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpDiagIpPingLeafOP},
	{"MinimumResponseTime", eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpDiagIpPingLeafOP},
	{"MaximumResponseTime",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpDiagIpPingLeafOP},
};

enum eIpDiagIpPingLeaf
{
	eIpPingDiagnosticsState,
	eIpPingInterface,
	eIpPingHost,
	eIpPingNumberOfRepetitions,
	eIpPingTimeout,
	eIpPingDataBlockSize,
	eIpPingDSCP,
	eIpPingSuccessCount,
	eIpPingFailureCount,
	eIpPingAverageResponseTime,
	eIpPingMinimumResponseTime,
	eIpPingMaximumResponseTime

};

struct CWMP_LEAF tIpDiagIpPingLeaf[] =
{
	{ &tIpDiagIpPingLeafInfo[eIpPingDiagnosticsState] },
	{ &tIpDiagIpPingLeafInfo[eIpPingInterface] },
	{ &tIpDiagIpPingLeafInfo[eIpPingHost] },
	{ &tIpDiagIpPingLeafInfo[eIpPingNumberOfRepetitions] },
	{ &tIpDiagIpPingLeafInfo[eIpPingTimeout] },
	{ &tIpDiagIpPingLeafInfo[eIpPingDataBlockSize] },
	{ &tIpDiagIpPingLeafInfo[eIpPingDSCP] },
	{ &tIpDiagIpPingLeafInfo[eIpPingSuccessCount] },
	{ &tIpDiagIpPingLeafInfo[eIpPingFailureCount] },
	{ &tIpDiagIpPingLeafInfo[eIpPingAverageResponseTime] },
	{ &tIpDiagIpPingLeafInfo[eIpPingMinimumResponseTime] },
	{ &tIpDiagIpPingLeafInfo[eIpPingMaximumResponseTime] },
	{ NULL	}
};

/*******************************************************************************
Device.IP.Diagnostics.IPPing.
*******************************************************************************/
struct CWMP_PRMT tIpDiagIpPingObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"IPPing",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eIpDiagIpPingObject
{
	eIpDiagIPPing
};

struct CWMP_NODE tIpDiagIpPingObject[] =
{
	/*info,  					leaf,			node)*/
	{&tIpDiagIpPingObjectInfo[eIpDiagIPPing],	&tIpDiagIpPingLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.IP.Interface.{i}.  Entity
*******************************************************************************/
struct CWMP_OP tIpIFEntityLeafOP = { getIpIFEntity, setIpIFEntity };
struct CWMP_PRMT tIpIFEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                      eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
//	{"IPv4Enable",                  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
#ifdef CONFIG_IPV6
	{"IPv6Enable",                  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
	{"ULAEnable",                   eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
#endif
	{"Status",                      eCWMP_tSTRING,	CWMP_READ,	&tIpIFEntityLeafOP},
//	{"Alias",                       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
	{"Name",                        eCWMP_tSTRING,	CWMP_READ,	&tIpIFEntityLeafOP},
//	{"LastChange",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFEntityLeafOP},
//	{"LowerLayers",                 eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
//	{"Router",                      eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
	{"Reset",                       eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
	{"MaxMTUSize",                  eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
	{"Type",                        eCWMP_tSTRING,	CWMP_READ,	&tIpIFEntityLeafOP},
	{"Loopback",                    eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
	{"IPv4AddressNumberOfEntries",  eCWMP_tUINT,	CWMP_READ,	&tIpIFEntityLeafOP},
#ifdef CONFIG_IPV6
	{"IPv6AddressNumberOfEntries",  eCWMP_tUINT,	CWMP_READ,	&tIpIFEntityLeafOP},
	{"IPv6PrefixNumberOfEntries",   eCWMP_tUINT,	CWMP_READ,	&tIpIFEntityLeafOP},
#endif
//	{"AutoIPEnable",				eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFEntityLeafOP},
};
enum eIpIFEntityLeaf
{
	eIpIFEnable,
//	eIpIFIPv4Enable,
#ifdef CONFIG_IPV6
	eIpIFIPv6Enable,
	eIpIFULAEnable,
#endif
	eIpIFStatus,
//	eIpIFAlias,
	eIpIFName,
//	eIpIFLastChange,
//	eIpIFLowerLayers,
//	eIpIFRouter,
	eIpIFReset,
	eIpIFMaxMTUSize,
	eIpIFType,
	eIpIFLoopback,
	eIpIFIPv4AddressNumberOfEntries,
#ifdef CONFIG_IPV6
	eIpIFIPv6AddressNumberOfEntries,
	eIpIFIPv6PrefixNumberOfEntries,
#endif
//	eIpIFAutoIPEnable
};
struct CWMP_LEAF tIpIFEntityLeaf[] =
{
	{ &tIpIFEntityLeafInfo[eIpIFEnable] },
//	{ &tIpIFEntityLeafInfo[eIpIFIPv4Enable] },
#ifdef CONFIG_IPV6
	{ &tIpIFEntityLeafInfo[eIpIFIPv6Enable] },
	{ &tIpIFEntityLeafInfo[eIpIFULAEnable] },
#endif
	{ &tIpIFEntityLeafInfo[eIpIFStatus] },
//	{ &tIpIFEntityLeafInfo[eIpIFAlias] },
	{ &tIpIFEntityLeafInfo[eIpIFName] },
//	{ &tIpIFEntityLeafInfo[eIpIFLastChange] },
//	{ &tIpIFEntityLeafInfo[eIpIFLowerLayers] },
//	{ &tIpIFEntityLeafInfo[eIpIFRouter] },
	{ &tIpIFEntityLeafInfo[eIpIFReset] },
	{ &tIpIFEntityLeafInfo[eIpIFMaxMTUSize] },
	{ &tIpIFEntityLeafInfo[eIpIFType] },
	{ &tIpIFEntityLeafInfo[eIpIFLoopback] },
	{ &tIpIFEntityLeafInfo[eIpIFIPv4AddressNumberOfEntries] },
#ifdef CONFIG_IPV6
	{ &tIpIFEntityLeafInfo[eIpIFIPv6AddressNumberOfEntries] },
	{ &tIpIFEntityLeafInfo[eIpIFIPv6PrefixNumberOfEntries] },
#endif
//	{ &tIpIFEntityLeafInfo[eIpIFAutoIPEnable] },
	{ NULL }
};

/*******************************************************************************
Device.IP.Interface.{i}.Stats.
*******************************************************************************/
struct CWMP_OP tIpIFStatsLeafOP = { getIpIFStats, NULL };
struct CWMP_PRMT tIpIFStatsLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"BytesSent",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP}, // unsigned long?
	{"BytesReceived",               eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"PacketsSent",                 eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"PacketsReceived",             eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"ErrorsSent",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP}, // unsigned int
	{"ErrorsReceived",              eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP}, // unsigned int
	{"UnicastPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"UnicastPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"DiscardPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP}, // unsigned int
	{"DiscardPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP}, // unsigned int
	{"MulticastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"MulticastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"BroadcastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
	{"BroadcastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
//	{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIpIFStatsLeafOP},
};
enum eIpIFStatsLeaf
{
	eIpIFStatsBytesSent,
	eIpIFStatsBytesReceived,
	eIpIFStatsPacketsSent,
	eIpIFStatsPacketsReceived,
	eIpIFStatsErrorsSent,
	eIpIFStatsErrorsReceived,
	eIpIFStatsUnicastPacketsSent,
	eIpIFStatsUnicastPacketsReceived,
	eIpIFStatsDiscardPacketsSent,
	eIpIFStatsDiscardPacketsReceived,
	eIpIFStatsMulticastPacketsSent,
	eIpIFStatsMulticastPacketsReceived,
	eIpIFStatsBroadcastPacketsSent,
	eIpIFStatsBroadcastPacketsReceived,
//	eIpIFStatsUnknownProtoPacketsReceived
};
struct CWMP_LEAF tIpIFStatsLeaf[] =
{
	{ &tIpIFStatsLeafInfo[eIpIFStatsBytesSent] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsBytesReceived] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsPacketsSent] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsPacketsReceived] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsErrorsSent] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsErrorsReceived] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsUnicastPacketsSent] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsUnicastPacketsReceived] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsDiscardPacketsSent] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsDiscardPacketsReceived] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsMulticastPacketsSent] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsMulticastPacketsReceived] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsBroadcastPacketsSent] },
	{ &tIpIFStatsLeafInfo[eIpIFStatsBroadcastPacketsReceived] },
//	{ &tIpIFStatsLeafInfo[eIpIFStatsUnknownProtoPacketsReceived] },
	{ NULL }
};

#ifdef CONFIG_IPV6
/*******************************************************************************
Device.IP.Interface.{i}.IPv6Prefix.{i}.   Entity
*******************************************************************************/
struct CWMP_OP tIpIFIpv6PreEntityLeafOP = { getIpIFIpv6PreEntity, setIpIFIpv6PreEntity };
struct CWMP_PRMT tIpIFIpv6PreEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
//	{"Enable",              eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"Status",              eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv6PreEntityLeafOP},
//	{"PrefixStatus",        eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv6PreEntityLeafOP},
//	{"Alias",               eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
	{"Prefix",              eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"Origin",              eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv6PreEntityLeafOP},
//	{"StaticType",          eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"ParentPrefix",        eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"ChildPrefixBits",     eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"OnLink",              eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"Autonomous",          eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"PreferredLifetime",   eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
//	{"ValidLifetime",		eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6PreEntityLeafOP},
};
enum eIpIFIpv6PreEntityLeaf
{
//	eIpIFIpv6PreEnable,
//	eIpIFIpv6PreStatus,
//	eIpIFIpv6PrePrefixStatus,
//	eIpIFIpv6PreAlias,
	eIpIFIpv6PrePrefix,
//	eIpIFIpv6PreOrigin,
//	eIpIFIpv6PreStaticType,
//	eIpIFIpv6PreParentPrefix,
//	eIpIFIpv6PreChildPrefixBits,
//	eIpIFIpv6PreOnLink,
//	eIpIFIpv6PreAutonomous,
//	eIpIFIpv6PrePreferredLifetime,
//	eIpIFIpv6PreValidLifetime
};
struct CWMP_LEAF tIpIFIpv6PreEntityLeaf[] =
{
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreEnable] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreStatus] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PrePrefixStatus] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreAlias] },
	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PrePrefix] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreOrigin] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreStaticType] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreParentPrefix] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreChildPrefixBits] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreOnLink] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreAutonomous] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PrePreferredLifetime] },
//	{ &tIpIFIpv6PreEntityLeafInfo[eIpIFIpv6PreValidLifetime] },
	{ NULL }
};

/*******************************************************************************
Device.IP.Interface.{i}.IPv6Prefix.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkIpIFIpv6PreOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkIpIFIpv6PreOjbect
{
	eLinkIpIFIpv6Pre0
};
struct CWMP_NODE tLinkIpIFIpv6PreObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkIpIFIpv6PreOjbectInfo[eLinkIpIFIpv6Pre0],	tIpIFIpv6PreEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.IP.Interface.{i}.IPv6Address.{i}.   Entity
*******************************************************************************/
struct CWMP_OP tIpIFIpv6AddrEntityLeafOP = { getIpIFIpv6AddrEntity, setIpIFIpv6AddrEntity };
struct CWMP_PRMT tIpIFIpv6AddrEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
//	{"Enable",              eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6AddrEntityLeafOP},
//	{"Status",              eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv6AddrEntityLeafOP},
//	{"IPAddressStatus",     eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv6AddrEntityLeafOP},
//	{"Alias",               eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6AddrEntityLeafOP},
	{"IPAddress",           eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6AddrEntityLeafOP},
//	{"Origin",              eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv6AddrEntityLeafOP},
	{"Prefix",              eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6AddrEntityLeafOP},
//	{"PreferredLifetime",   eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6AddrEntityLeafOP},
//	{"ValidLifetime",       eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6AddrEntityLeafOP},
//	{"Anycast",				eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv6AddrEntityLeafOP},
};
enum eIpIFIpv6AddrEntityLeaf
{
//	eIpIFIpv6AddrEnable,
//	eIpIFIpv6AddrStatus,
//	eIpIFIpv6AddrIPAddressStatus,
//	eIpIFIpv6AddrAlias,
	eIpIFIpv6AddrIPAddress,
//	eIpIFIpv6AddrOrigin,
	eIpIFIpv6AddrPrefix,
//	eIpIFIpv6AddrPreferredLifetime,
//	eIpIFIpv6AddrValidLifetime,
//	eIpIFIpv6AddrAnycast
};
struct CWMP_LEAF tIpIFIpv6AddrEntityLeaf[] =
{
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrEnable] },
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrStatus] },
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrIPAddressStatus] },
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrAlias] },
	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrIPAddress] },
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrOrigin] },
	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrPrefix] },
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrPreferredLifetime] },
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrValidLifetime] },
//	{ &tIpIFIpv6AddrEntityLeafInfo[eIpIFIpv6AddrAnycast] },
	{ NULL }
};

/*******************************************************************************
Device.IP.Interface.{i}.IPv6Address.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkIpIFIpv6AddrOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkIpIFIpv6AddrOjbect
{
	eLinkIpIFIpv6Addr0
};
struct CWMP_NODE tLinkIpIFIpv6AddrObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkIpIFIpv6AddrOjbectInfo[eLinkIpIFIpv6Addr0],	tIpIFIpv6AddrEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};
#endif // CONFIG_IPV6

/*******************************************************************************
Device.IP.Interface.{i}.IPv4Address.{i}.  Entity
*******************************************************************************/
struct CWMP_OP tIpIFIpv4AddrEntityLeafOP = { getIpIFIpv4AddrEntity, setIpIFIpv4AddrEntity };
struct CWMP_PRMT tIpIFIpv4AddrEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",          eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv4AddrEntityLeafOP},
	{"Status",          eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv4AddrEntityLeafOP},
//	{"Alias",           eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv4AddrEntityLeafOP},
	{"IPAddress",       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv4AddrEntityLeafOP},
	{"SubnetMask",      eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIpIFIpv4AddrEntityLeafOP},
	{"AddressingType",	eCWMP_tSTRING,	CWMP_READ,	&tIpIFIpv4AddrEntityLeafOP},
};
enum eIpIFIpv4AddrEntityLeaf
{
	eIpIFIpv4AddrEnable,
	eIpIFIpv4AddrStatus,
//	eIpIFIpv4AddrAlias,
	eIpIFIpv4AddrIPAddress,
	eIpIFIpv4AddrSubnetMask,
	eIpIFIpv4AddrAddressingType
};
struct CWMP_LEAF tIpIFIpv4AddrEntityLeaf[] =
{
	{ &tIpIFIpv4AddrEntityLeafInfo[eIpIFIpv4AddrEnable] },
	{ &tIpIFIpv4AddrEntityLeafInfo[eIpIFIpv4AddrStatus] },
//	{ &tIpIFIpv4AddrEntityLeafInfo[eIpIFIpv4AddrAlias] },
	{ &tIpIFIpv4AddrEntityLeafInfo[eIpIFIpv4AddrIPAddress] },
	{ &tIpIFIpv4AddrEntityLeafInfo[eIpIFIpv4AddrSubnetMask] },
	{ &tIpIFIpv4AddrEntityLeafInfo[eIpIFIpv4AddrAddressingType] },
	{ NULL }
};

/*******************************************************************************
Device.IP.Interface.{i}.IPv4Address.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkIpIFIpv4AddrOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkIpIFIpv4AddrOjbect
{
	eLinkIpIFIpv4Addr1
};
struct CWMP_LINKNODE tLinkIpIFIpv4AddrObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkIpIFIpv4AddrOjbectInfo[eLinkIpIFIpv4Addr1],	tIpIFIpv4AddrEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.IP.Interface.{i}.IPv4Address.{i}.
Device.IP.Interface.{i}.IPv6Address.{i}.
Device.IP.Interface.{i}.IPv6Prefix.{i}.
Device.IP.Interface.{i}.Stats.
*******************************************************************************/
//struct CWMP_OP tIpIFIpv6Addr_OP = { NULL, objIpIFIpv6Addr };
//struct CWMP_OP tIpIFIpv6Prefix_OP = { NULL, objIpIFIpv6Prefix };
struct CWMP_PRMT tIpIFIpv4AddrObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"IPv4Address",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef CONFIG_IPV6
	{"IPv6Address",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"IPv6Prefix",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
	{"Stats",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eIpIFIpv4AddrObject
{
	eIpIFIpv4Addr,
#ifdef CONFIG_IPV6
	eIpIFIpv6Addr,
	eIpIFIpv6Prefix,
#endif
	eIpIFStats
};

struct CWMP_NODE tIpIFIpv4AddrObject[] =
{
	/*info,  					leaf,			node)*/
	{&tIpIFIpv4AddrObjectInfo[eIpIFIpv4Addr],	NULL,	tLinkIpIFIpv4AddrObject},
#ifdef CONFIG_IPV6
	{&tIpIFIpv4AddrObjectInfo[eIpIFIpv6Addr],	NULL,	tLinkIpIFIpv6AddrObject},
	{&tIpIFIpv4AddrObjectInfo[eIpIFIpv6Prefix],	NULL,	tLinkIpIFIpv6PreObject},
#endif
	{&tIpIFIpv4AddrObjectInfo[eIpIFStats],		&tIpIFStatsLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.IP.Interface.{i}. LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkIpIFOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eLinkIpIFOjbect
{
	eLinkIpIF1
};
struct CWMP_NODE tLinkIpIFObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkIpIFOjbectInfo[eLinkIpIF1],	tIpIFEntityLeaf,	tIpIFIpv4AddrObject},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.IP.Interface.{i}.
Device.IP.Diagnostics.
*******************************************************************************/
struct CWMP_PRMT tIpIFObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Interface",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"Diagnostics",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eIpIFObject
{
	eIpIF,
	eIpDiag
};

struct CWMP_NODE tIpIFObject[] =
{
	/*info,  					leaf,			node)*/
	{&tIpIFObjectInfo[eIpIF],	NULL,	&tLinkIpIFObject},
	{&tIpIFObjectInfo[eIpDiag],	NULL,	&tIpDiagIpPingObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
static int getMIB2Str(unsigned int id, char *strbuf)
{
	unsigned char buffer[64];

	if (!strbuf)
		return -1;

	switch (id) {
		// INET address
		case MIB_IP_ADDR:
		case MIB_SUBNET_MASK:
		case MIB_DMZ_HOST:
		case MIB_DHCP_CLIENT_START:
		case MIB_DHCP_CLIENT_END:
		case MIB_DNS1:
		case MIB_DNS2:
		case MIB_DNS3:
		case MIB_DEFAULT_GATEWAY:
#if defined(_PRMT_X_CT_COM_DHCP_)
		case MIB_CT_STB_MINADDR:
		case MIB_CT_STB_MAXADDR:
		case MIB_CT_PHN_MINADDR:
		case MIB_CT_PHN_MAXADDR:
		case MIB_CT_CMR_MINADDR:
		case MIB_CT_CMR_MAXADDR:
		case MIB_CT_PC_MINADDR:
		case MIB_CT_PC_MAXADDR:
		case MIB_CT_HGW_MINADDR:
		case MIB_CT_HGW_MAXADDR:
#endif //_PRMT_X_CT_COM_DHCP_

#ifdef _PRMT_TR143_
		case MIB_TR143_UDPECHO_SRCIP:
#endif //_PRMT_TR143_

			if(!mib_get( id, (void *)buffer))
				return -1;
			// Mason Yu
			if ( ((struct in_addr *)buffer)->s_addr == INADDR_NONE ) {
				sprintf(strbuf, "%s", "");
			} else {
				sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
			}
			break;
		// Ethernet address
		case MIB_ELAN_MAC_ADDR:
		case MIB_WLAN_WLAN_MAC_ADDR:
			if(!mib_get( id,  (void *)buffer))
				return -1;
			
			sprintf(strbuf, "%02x%02x%02x%02x%02x%02x", buffer[0], buffer[1],
				buffer[2], buffer[3], buffer[4], buffer[5]);
			
			break;
		case MIB_WLAN_CHANNEL:
		case MIB_WLAN_WLAN_DISABLED:
		case MIB_WLAN_ENABLE_1X:
		case MIB_WLAN_ENCRYPT:
		case MIB_WLAN_WPA_AUTH:
		case MIB_WLAN_NETWORK_TYPE:
			if(!mib_get( id,  (void *)buffer))
				return -1;
	   		sprintf(strbuf, "%u", *(unsigned char *)buffer);
	   		break;
#ifdef WLAN_SUPPORT
	   	case MIB_WLAN_FRAG_THRESHOLD:
	   	case MIB_WLAN_RTS_THRESHOLD:
	   	case MIB_WLAN_BEACON_INTERVAL:
#endif
			if(!mib_get( id,  (void *)buffer))
				return -1;
			sprintf(strbuf, "%u", *(unsigned short *)buffer);
			break;

		case MIB_DHCP_LEASE_TIME:
			if(!mib_get( id,  (void *)buffer))
				return -1;
			// if MIB_ADSL_LAN_DHCP_LEASE=0xffffffff, it indicate an infinate lease
			if ( *(unsigned long *)buffer == 0xffffffff )
				sprintf(strbuf, "-1");
			else
				sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;

		case MIB_CWMP_CONREQ_PORT:
		case MIB_CWMP_INFORM_INTERVAL:

			if(!mib_get( id,  (void *)buffer))
				return -1;
			sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;
#ifdef WLAN_SUPPORT
		case MIB_WLAN_SSID:
#endif
		case MIB_CWMP_PROVISIONINGCODE:
		case MIB_CWMP_ACS_URL:
		case MIB_CWMP_ACS_USERNAME:
		case MIB_CWMP_ACS_PASSWORD:
		case MIB_CWMP_CONREQ_USERNAME:
		case MIB_CWMP_CONREQ_PASSWORD:
		case MIB_CWMP_CONREQ_PATH:
		case MIB_CWMP_LAN_CONFIGPASSWD:
		case MIB_CWMP_SERIALNUMBER:
		case MIB_CWMP_DL_COMMANDKEY:
		case MIB_CWMP_RB_COMMANDKEY:
		case MIB_CWMP_ACS_PARAMETERKEY:
		case MIB_CWMP_CERT_PASSWORD:
#ifdef _PRMT_USERINTERFACE_
		case MIB_UIF_AUTOUPDATESERVER:
		case MIB_UIF_USERUPDATESERVER:
#endif
		case MIB_CWMP_SI_COMMANDKEY:
		case MIB_CWMP_ACS_KICKURL:
		case MIB_CWMP_ACS_DOWNLOADURL:
		case MIB_SUPER_NAME:
		case MIB_USER_NAME:
#ifdef _PRMT_X_CT_COM_USERINFO_
		case MIB_CWMP_USERINFO_USERID:
		case MIB_CWMP_USERINFO_USERNAME:
#endif
			if(!mib_get( id,  (void *)strbuf)){
				return -1;
			}
			break;


		default:
			return -1;
	}

	return 0;
}

int getIpIFEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	
	if ((name == NULL) || (type == NULL) || ( data == NULL) || (entity == NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	
	if (strcmp(lastname, "Enable" ) == 0)
	{
		*data = booldup(1); 
	}
#ifdef CONFIG_IPV6
	else if (strcmp(lastname, "IPv6Enable" ) == 0)
	{
		*data = booldup(1); 
	}
	else if (strcmp(lastname, "ULAEnable" ) == 0)
	{
		*data = booldup(1); 
	}
#endif
	else if (strcmp(lastname, "Status" ) == 0)
	{
		*data = strdup("Up"); 
	}
	else if (strcmp(lastname, "Name" ) == 0)
	{
		*data = strdup("br0"); 
	}
	else if (strcmp(lastname, "Reset" ) == 0)
	{
		*data = booldup(0); 
	}
	else if (strcmp(lastname, "MaxMTUSize" ) == 0)
	{
		unsigned int mtuSize = 0;
		
		mib_get(MIB_DHCP_MTU_SIZE, (void *)&mtuSize);
		*data = uintdup(mtuSize); 
	}
	else if (strcmp(lastname, "Type" ) == 0)
	{
		*data = strdup("Normal"); 
	}
	else if (strcmp(lastname, "Loopback" ) == 0)
	{
		*data = booldup(0); 
	}
	else if (strcmp(lastname, "IPv4AddressNumberOfEntries" ) == 0)
	{
		*data = uintdup(1); 
	}
#ifdef CONFIG_IPV6
	else if (strcmp(lastname, "IPv6AddressNumberOfEntries" ) == 0)
	{
		*data = uintdup(1); 
	}
	else if (strcmp(lastname, "IPv6PrefixNumberOfEntries" ) == 0)
	{
		*data = uintdup(1); 
	}
#endif
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setIpIFEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;	
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	
	if( strcmp( lastname, "Reset" )==0 )
	{
		unsigned int *i = data;

		if( i==NULL ) return ERR_9007;

		if (*i == 1)
		{
			system("ifconfig br0 down");
			system("ifconfig br0 up");
		}
	}
	else if( strcmp( lastname, "MaxMTUSize" )==0 )
	{
		unsigned int *i = data;
		unsigned int uInt;

		if( i==NULL ) return ERR_9007;
		
		uInt = *i;
		mib_set(MIB_DHCP_MTU_SIZE, &uInt);

		return 1;
	}
	else
		return ERR_9005; 
	
	return 0;
}

#ifdef CONFIG_IPV6
int objIpIFIpv6Addr(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int objIpIFIpv6Prefix(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}
#endif // CONFIG_IPV6

int getIpIFStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	struct user_net_device_stats nds_eth0, nds_eth1, nds_wlan0, nds_wlan1;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if(getEthStats("eth0", &nds_eth0) < 0)
			return ERR_9002;
	if(getEthStats("eth1", &nds_eth1) < 0)
			return ERR_9002;

	if(getStats("wlan0", &nds_wlan0) < 0)
			return ERR_9002;
	if(getStats("wlan1", &nds_wlan1) < 0)
			return ERR_9002;

	if( strcmp(lastname, "BytesSent")==0 )
	{
		*data = uintdup(nds_eth0.tx_bytes+nds_eth1.tx_bytes+nds_wlan0.tx_bytes+nds_wlan1.tx_bytes);
	}
	else if( strcmp(lastname, "BytesReceived")==0 )
	{
		*data = uintdup(nds_eth0.rx_bytes+nds_eth1.rx_bytes+nds_wlan0.rx_bytes+nds_wlan1.rx_bytes);
	}
	else if( strcmp(lastname, "PacketsSent")==0 )
	{
		*data = uintdup(nds_eth0.tx_packets+nds_eth1.tx_packets+nds_wlan0.tx_packets+nds_wlan1.tx_packets);
	}
	else if( strcmp(lastname, "PacketsReceived")==0 )
	{
		*data = uintdup(nds_eth0.rx_packets+nds_eth1.rx_packets+nds_wlan0.rx_packets+nds_wlan1.rx_packets);
	}
	else if (strcmp(lastname, "ErrorsSent") == 0) 
	{	
		*data = uintdup(nds_eth0.tx_errors+nds_eth1.tx_errors+nds_wlan0.tx_errors+nds_wlan1.tx_errors);
	} 
	else if (strcmp(lastname, "ErrorsReceived") == 0) 
	{	
		*data = uintdup(nds_eth0.rx_errors+nds_eth1.rx_errors+nds_wlan0.rx_errors+nds_wlan1.rx_errors);
	} 
	else if (strcmp(lastname, "UnicastPacketsSent") == 0) 
	{
		*data = uintdup(nds_eth0.tx_unicast+nds_eth1.tx_unicast+nds_wlan0.tx_unicast+nds_wlan1.tx_unicast);
	} 
	else if (strcmp(lastname, "UnicastPacketsReceived") == 0) 
	{	
		*data = uintdup(nds_eth0.rx_unicast+nds_eth1.rx_unicast+nds_wlan0.rx_unicast+nds_wlan1.rx_unicast);
	} 
	else if (strcmp(lastname, "DiscardPacketsSent") == 0) 
	{
		*data = uintdup(nds_eth0.tx_dropped+nds_eth1.tx_dropped+nds_wlan0.tx_dropped+nds_wlan1.tx_dropped);
	} 
	else if (strcmp(lastname, "DiscardPacketsReceived") == 0) 
	{
		*data = uintdup(nds_eth0.rx_dropped+nds_eth1.rx_dropped+nds_wlan0.rx_dropped+nds_wlan1.rx_dropped);
	} 
	else if (strcmp(lastname, "MulticastPacketsSent") == 0) 
	{	
		*data = uintdup(nds_eth0.tx_multicast+nds_eth1.tx_multicast+nds_wlan0.tx_multicast+nds_wlan1.tx_multicast);
	} 
	else if (strcmp(lastname, "MulticastPacketsReceived") == 0) 
	{
		*data = uintdup(nds_eth0.rx_multicast+nds_eth1.rx_multicast+nds_wlan0.rx_multicast+nds_wlan1.rx_multicast);
	} 
	else if (strcmp(lastname, "BroadcastPacketsSent") == 0) 
	{	
		*data = uintdup(nds_eth0.tx_broadcast+nds_eth1.tx_broadcast+nds_wlan0.tx_broadcast+nds_wlan1.tx_broadcast);
	} 
	else if (strcmp(lastname, "BroadcastPacketsReceived") == 0) 
	{	
		*data = uintdup(nds_eth0.rx_broadcast+nds_eth1.rx_broadcast+nds_wlan0.rx_broadcast+nds_wlan1.rx_broadcast);
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

int getIpIFIpv4AddrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	unsigned int vInt=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup("Enabled");
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
	else if( strcmp( lastname, "AddressingType" )==0 )
		{
				*data = strdup( "DHCP" );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setIpIFIpv4AddrEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return ERR_9008;
}

#ifdef CONFIG_IPV6
int getIpIFIpv6AddrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	unsigned int vInt=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "IPAddress" )==0 )
	{
		char addrBuf[64]={0};
		int retVal=0;
		
		retVal=get_inet6_stats("br0",0,addrBuf,NULL,NULL,NULL,NULL);
		
		if(retVal<0) return ERR_9002;
		
		if(retVal==0)
		{
			printf("not find %s\n",name);
			return ERR_9005;
		}
		
		*data = strdup(addrBuf);
	}
	else if( strcmp( lastname, "Prefix" )==0 )
	{
		char addrBuf[64]={0};
		int retVal=0;
		int prefixLen=0;
		
		retVal=get_inet6_stats("br0",0,addrBuf,NULL,&prefixLen,NULL,NULL);
		
		if(retVal<0) return ERR_9002;
		
		if(retVal==0)
		{
			printf("not find %s\n",name);
			return ERR_9005;
		}
		if(prefixLen>128||prefixLen==0)
		{
			printf("prefixLen invalid %s\n",name);
			return ERR_9007;
		}
		sprintf(buf,"%s/%d",addrBuf,prefixLen);
		
		*data = strdup(buf);
	}
	
	return 0;
}

int setIpIFIpv6AddrEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}

int getIpIFIpv6PreEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	unsigned int vInt=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Prefix" )==0 )
	{
		char addrBuf[64]={0};
		int retVal=0;
		int prefixLen=0;
		
		retVal=get_inet6_stats("br0",0,addrBuf,NULL,&prefixLen,NULL,NULL);
		
		if(retVal<0) return ERR_9002;
		
		if(retVal==0)
		{
			printf("not find %s\n",name);
			return ERR_9005;
		}
		if(prefixLen>128||prefixLen==0)
		{
			printf("prefixLen invalid %s\n",name);
			return ERR_9007;
		}
		sprintf(buf,"%s/%d",addrBuf,prefixLen);
		
		*data = strdup(buf);
	}
	
	return 0;
}

int setIpIFIpv6PreEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return 0;
}
#endif // CONFIG_IPV6

