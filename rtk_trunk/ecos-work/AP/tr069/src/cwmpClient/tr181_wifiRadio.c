#include "tr181_wifiRadio.h"
#include <math.h>


/*******************************************************************************
Device.WiFi.AccessPoint.{i} Entity
*******************************************************************************/
struct CWMP_OP tWifiAPEntityLeafOP = { getWifiAPEntity, setWifiAPEntity };
struct CWMP_PRMT tWifiAPEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                          eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWifiAPEntityLeafOP}, // default false
	{"Status",                          eCWMP_tSTRING,	CWMP_READ,	&tWifiAPEntityLeafOP},			   // default Disabled
//	{"Alias",                           eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPEntityLeafOP},
	{"SSIDReference",                   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPEntityLeafOP},
	{"SSIDAdvertisementEnabled",        eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWifiAPEntityLeafOP},
	{"RetryLimit",                      eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tWifiAPEntityLeafOP},
	{"WMMCapability",                   eCWMP_tBOOLEAN,	CWMP_READ,	&tWifiAPEntityLeafOP},
	{"UAPSDCapability",                 eCWMP_tBOOLEAN,	CWMP_READ,	&tWifiAPEntityLeafOP},
	{"WMMEnable",                       eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWifiAPEntityLeafOP},
	{"UAPSDEnable",                     eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWifiAPEntityLeafOP},
	{"AssociatedDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tWifiAPEntityLeafOP},
};
enum eWifiAPEntityLeaf
{
	eWifiAPEnable,
	eWifiAPStatus,
//	eWifiAPAlias,
	eWifiAPSSIDReference,
	eWifiAPSSIDAdvertisementEnabled,
	eWifiAPRetryLimit,
	eWifiAPWMMCapability,
	eWifiAPUAPSDCapability,
	eWifiAPWMMEnable,
	eWifiAPUAPSDEnable,
	eWifiAPAssociatedDeviceNumberOfEntries
};
struct CWMP_LEAF tWifiAPEntityLeaf[] =
{
	{ &tWifiAPEntityLeafInfo[eWifiAPEnable] },
	{ &tWifiAPEntityLeafInfo[eWifiAPStatus] },
//	{ &tWifiAPEntityLeafInfo[eWifiAPAlias] },
	{ &tWifiAPEntityLeafInfo[eWifiAPSSIDReference] },
	{ &tWifiAPEntityLeafInfo[eWifiAPSSIDAdvertisementEnabled] },
	{ &tWifiAPEntityLeafInfo[eWifiAPRetryLimit] },
	{ &tWifiAPEntityLeafInfo[eWifiAPWMMCapability] },
	{ &tWifiAPEntityLeafInfo[eWifiAPUAPSDCapability] },
	{ &tWifiAPEntityLeafInfo[eWifiAPWMMEnable] },
	{ &tWifiAPEntityLeafInfo[eWifiAPUAPSDEnable] },
	{ &tWifiAPEntityLeafInfo[eWifiAPAssociatedDeviceNumberOfEntries] },
	{ NULL }
};

/*******************************************************************************
Device.WiFi.SSID.{i} Entity
*******************************************************************************/
struct CWMP_OP tWifiSsidEntityLeafOP = { getWifiSsidEntity, setWifiSsidEntity };
struct CWMP_PRMT tWifiSsidEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWifiSsidEntityLeafOP}, // default false
//	{"Status",      eCWMP_tSTRING,	CWMP_READ,	&tWifiSsidEntityLeafOP},			 // default Down
//	{"Alias",       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiSsidEntityLeafOP},
//	{"Name",        eCWMP_tSTRING,	CWMP_READ,	&tWifiSsidEntityLeafOP},
//	{"LastChange",  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidEntityLeafOP},
//	{"LowerLayers", eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiSsidEntityLeafOP}, // list of string
	{"BSSID",       eCWMP_tSTRING,	CWMP_READ,	&tWifiSsidEntityLeafOP},			 // type: MACAddress
	{"MACAddress",  eCWMP_tSTRING,	CWMP_READ,	&tWifiSsidEntityLeafOP},			 // type: MACAddress
	{"SSID",        eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiSsidEntityLeafOP},
};
enum eWifiSsidEntityLeaf
{
	eWifiSsidEnable,
//	eWifiSsidStatus,
//	eWifiSsidAlias,
//	eWifiSsidName,
//	eWifiSsidLastChange,
//	eWifiSsidLowerLayers,
	eWifiSsidBSSID,
	eWifiSsidMACAddress,
	eWifiSsidSSID
};
struct CWMP_LEAF tWifiSsidEntityLeaf[] =
{
	{ &tWifiSsidEntityLeafInfo[eWifiSsidEnable] },
//	{ &tWifiSsidEntityLeafInfo[eWifiSsidStatus] },
//	{ &tWifiSsidEntityLeafInfo[eWifiSsidAlias] },
//	{ &tWifiSsidEntityLeafInfo[eWifiSsidName] },
//	{ &tWifiSsidEntityLeafInfo[eWifiSsidLastChange] },
//	{ &tWifiSsidEntityLeafInfo[eWifiSsidLowerLayers] },
	{ &tWifiSsidEntityLeafInfo[eWifiSsidBSSID] },
	{ &tWifiSsidEntityLeafInfo[eWifiSsidMACAddress] },
	{ &tWifiSsidEntityLeafInfo[eWifiSsidSSID] },
	{ NULL }
};

/*******************************************************************************
Device.WiFi.Radio.{i} Entity
*******************************************************************************/
struct CWMP_OP tWifiRadioEntityLeafOP = { getWifiRadioEntity, setWifiRadioEntity };
struct CWMP_PRMT tWifiRadioEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",						eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"Status",                      eCWMP_tSTRING,	CWMP_READ, &tWifiRadioEntityLeafOP},
//	{"Alias",                       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"Name",                        eCWMP_tSTRING,	CWMP_READ, &tWifiRadioEntityLeafOP},
	{"LastChange",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT, &tWifiRadioEntityLeafOP},
	{"LowerLayers",                 eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP}, // list of string
	{"Upstream",                    eCWMP_tBOOLEAN,	CWMP_READ, &tWifiRadioEntityLeafOP},
	{"MaxBitRate",                  eCWMP_tUINT,	CWMP_READ, &tWifiRadioEntityLeafOP},
	{"SupportedFrequencyBands",     eCWMP_tSTRING,	CWMP_READ, &tWifiRadioEntityLeafOP}, // list of string
	{"OperatingFrequencyBand",      eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"SupportedStandards",          eCWMP_tSTRING,	CWMP_READ, &tWifiRadioEntityLeafOP}, // list of string
	{"OperatingStandards",          eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP}, // list of string
	{"PossibleChannels",            eCWMP_tSTRING,	CWMP_READ, &tWifiRadioEntityLeafOP}, // list of string
	{"ChannelsInUse",               eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT, &tWifiRadioEntityLeafOP}, // list of string
	{"Channel",                     eCWMP_tUINT,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"AutoChannelSupported",        eCWMP_tBOOLEAN,	CWMP_READ, &tWifiRadioEntityLeafOP}, // default false
	{"AutoChannelEnable",           eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"AutoChannelRefreshPeriod",    eCWMP_tUINT,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"OperatingChannelBandwidth",   eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"ExtensionChannel",            eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"GuardInterval",               eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"MCS",                         eCWMP_tINT,		CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"TransmitPowerSupported",      eCWMP_tSTRING,		CWMP_READ, &tWifiRadioEntityLeafOP},// list of int
	{"TransmitPower",               eCWMP_tINT,		CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
//	{"IEEE80211hSupported",         eCWMP_tBOOLEAN,	CWMP_READ, &tWifiRadioEntityLeafOP},
//	{"IEEE80211hEnabled",           eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
	{"RegulatoryDomain",            eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE, &tWifiRadioEntityLeafOP},
};
enum eWifiRadioEntityLeaf
{
	eWifiRaEnable,
	eWifiRaStatus,
//	eWifiRaAlias,
	eWifiRaName,
	eWifiRaLastChange,
	eWifiRaLowerLayers,
	eWifiRaUpstream,
	eWifiRaMaxBitRate,
	eWifiRaSupportedFrequencyBands,
	eWifiRaOperatingFrequencyBand,
	eWifiRaSupportedStandards,
	eWifiRaOperatingStandards,
	eWifiRaPossibleChannels,
	eWifiRaChannelsInUse,
	eWifiRaChannel,
	eWifiRaAutoChannelSupported,
	eWifiRaAutoChannelEnable,
	eWifiRaAutoChannelRefreshPeriod,
	eWifiRaOperatingChannelBandwidth,
	eWifiRaExtensionChannel,
	eWifiRaGuardInterval,
	eWifiRaMCS,
	eWifiRaTransmitPowerSupported,
	eWifiRaTransmitPower,
//	eWifiRaIEEE80211hSupported,
//	eWifiRaIEEE80211hEnabled,
	eWifiRaRegulatoryDomain
};
struct CWMP_LEAF tWifiRadioEntityLeaf[] =
{
	{ &tWifiRadioEntityLeafInfo[eWifiRaEnable] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaStatus] },
//	{ &tWifiRadioEntityLeafInfo[eWifiRaAlias] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaName] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaLastChange] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaLowerLayers] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaUpstream] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaMaxBitRate] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaSupportedFrequencyBands] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaOperatingFrequencyBand] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaSupportedStandards] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaOperatingStandards] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaPossibleChannels] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaChannelsInUse] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaChannel] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaAutoChannelSupported] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaAutoChannelEnable] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaAutoChannelRefreshPeriod] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaOperatingChannelBandwidth] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaExtensionChannel] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaGuardInterval] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaMCS] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaTransmitPowerSupported] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaTransmitPower] },
//	{ &tWifiRadioEntityLeafInfo[eWifiRaIEEE80211hSupported] },
//	{ &tWifiRadioEntityLeafInfo[eWifiRaIEEE80211hEnabled] },
	{ &tWifiRadioEntityLeafInfo[eWifiRaRegulatoryDomain] },
	{ NULL }
};

/*******************************************************************************
Device.WiFi.Radio.{i}.Stats
*******************************************************************************/
struct CWMP_OP tWifiRaStEntityLeafOP = { getWifiRaStats, NULL };
struct CWMP_PRMT tWifiRaStEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"BytesSent",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP}, // unsigned long?
	{"BytesReceived",               eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP},
	{"PacketsSent",                 eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP},
	{"PacketsReceived",             eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP},
//	{"ErrorsSent",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP}, // unsigned int
//	{"ErrorsReceived",              eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP}, // unsigned int
	{"DiscardPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP}, // unsigned int
	{"DiscardPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiRaStEntityLeafOP}, // unsigned int
};
enum eWifiRaStEntityLeaf
{
	eWifiRaStBytesSent,
	eWifiRaStBytesReceived,
	eWifiRaStPacketsSent,
	eWifiRaStPacketsReceived,
//	eWifiRaStErrorsSent,
//	eWifiRaStErrorsReceived,
	eWifiRaStDiscardPacketsSent,
	eWifiRaStDiscardPacketsReceived
};
struct CWMP_LEAF tWifiRaStEntityLeaf[] =
{
	{ &tWifiRaStEntityLeafInfo[eWifiRaStBytesSent] },
	{ &tWifiRaStEntityLeafInfo[eWifiRaStBytesReceived] },
	{ &tWifiRaStEntityLeafInfo[eWifiRaStPacketsSent] },
	{ &tWifiRaStEntityLeafInfo[eWifiRaStPacketsReceived] },
//	{ &tWifiRaStEntityLeafInfo[eWifiRaStErrorsSent] },
//	{ &tWifiRaStEntityLeafInfo[eWifiRaStErrorsReceived] },
	{ &tWifiRaStEntityLeafInfo[eWifiRaStDiscardPacketsSent] },
	{ &tWifiRaStEntityLeafInfo[eWifiRaStDiscardPacketsReceived] },
	{ NULL }
};

/*******************************************************************************
Device.WiFi.Radio.{i}.Stats
*******************************************************************************/
struct CWMP_PRMT tWifiRaStatsObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Stats",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eWifiRaStatsObject
{
	eWifiRaStats
};

struct CWMP_NODE tWifiRaStatsObject[] =
{
	/*info,  					leaf,			node)*/
	{&tWifiRaStatsObjectInfo[eWifiRaStats],	&tWifiRaStEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.WiFi.Radio.{i}
Device.WiFi.Radio.0
Device.WiFi.Radio.1
*******************************************************************************/
struct CWMP_PRMT tLinkWifiRadioOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eLinkWifiRadioOjbect
{
	eLinkWifiRadio0,
	eLinkWifiRadio1
};
struct CWMP_NODE tLinkWifiRadioObject[] =
{
	/*info,  				leaf,			next*/
	{&tLinkWifiRadioOjbectInfo[eLinkWifiRadio0],	tWifiRadioEntityLeaf,	tWifiRaStatsObject},
	{&tLinkWifiRadioOjbectInfo[eLinkWifiRadio1],	tWifiRadioEntityLeaf,	tWifiRaStatsObject},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.WiFi.SSID.{i}.Stats.
*******************************************************************************/
struct CWMP_OP tWifiSsidStEntityLeafOP = { getWifiSsidStats, NULL };
struct CWMP_PRMT tWifiSsidStEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"BytesSent",                   eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP}, // unsigned long?
	{"BytesReceived",               eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
	{"PacketsSent",                 eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
	{"PacketsReceived",             eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
//	{"ErrorsSent",                  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP}, // unsigned int
//	{"ErrorsReceived",              eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP}, // unsigned int
//	{"UnicastPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
//	{"UnicastPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
	{"DiscardPacketsSent",          eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP}, // unsigned int
	{"DiscardPacketsReceived",      eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP}, // unsigned int
//	{"MulticastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
//	{"MulticastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
//	{"BroadcastPacketsSent",        eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
//	{"BroadcastPacketsReceived",    eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
//	{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiSsidStEntityLeafOP},
};
enum eWifiSsidStEntityLeaf
{
	eWifiSsidStBytesSent,
	eWifiSsidStBytesReceived,
	eWifiSsidStPacketsSent,
	eWifiSsidStPacketsReceived,
//	eWifiSsidStErrorsSent,
//	eWifiSsidStErrorsReceived,
//	eWifiSsidStUnicastPacketsSent,
//	eWifiSsidStUnicastPacketsReceived,
	eWifiSsidStDiscardPacketsSent,
	eWifiSsidStDiscardPacketsReceived
//	eWifiSsidStMulticastPacketsSent,
//	eWifiSsidStMulticastPacketsReceived,
//	eWifiSsidStBroadcastPacketsSent,
//	eWifiSsidStBroadcastPacketsReceived,
//	eWifiSsidStUnknownProtoPacketsReceived
};
struct CWMP_LEAF tWifiSsidStEntityLeaf[] =
{
	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStBytesSent] },
	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStBytesReceived] },
	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStPacketsSent] },
	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStPacketsReceived] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStErrorsSent] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStErrorsReceived] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStUnicastPacketsSent] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStUnicastPacketsReceived] },
	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStDiscardPacketsSent] },
	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStDiscardPacketsReceived] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStMulticastPacketsSent] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStMulticastPacketsReceived] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStBroadcastPacketsSent] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStBroadcastPacketsReceived] },
//	{ &tWifiSsidStEntityLeafInfo[eWifiSsidStUnknownProtoPacketsReceived] },
	{ NULL }
};

/*******************************************************************************
Device.WiFi.SSID.{i}.Stats.
*******************************************************************************/
struct CWMP_PRMT tWifiSsidStatsObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Stats",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eWifiSsidStatsObject
{
	eWifiSsidStats
};

struct CWMP_NODE tWifiSsidStatsObject[] =
{
	/*info,  					leaf,			node)*/
	{&tWifiSsidStatsObjectInfo[eWifiSsidStats],	&tWifiSsidStEntityLeaf,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.WiFi.SSID.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkWifiSsidOjbectInfo[] =
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
};
enum eLinkWifiSsidOjbect
{
	eLinkWifiSsid1,
	eLinkWifiSsid2,
	eLinkWifiSsid3,
	eLinkWifiSsid4,
	eLinkWifiSsid5,
	eLinkWifiSsid6,
	eLinkWifiSsid7,
	eLinkWifiSsid8,
	eLinkWifiSsid9,
	eLinkWifiSsid10
};
struct CWMP_NODE tLinkWifiSsidObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid1],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid2],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid3],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid4],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid5],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid6],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid7],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid8],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid9],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{&tLinkWifiSsidOjbectInfo[eLinkWifiSsid10],	tWifiSsidEntityLeaf,	tWifiSsidStatsObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Device.WiFi.AccessPoint.{i}.Security.
*******************************************************************************/
struct CWMP_OP tWifiAPSecurityLeafOP = { getWifiAPSecurity, setWifiAPSecurity };
struct CWMP_PRMT tWifiAPSecurityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"ModesSupported",		eCWMP_tSTRING,	CWMP_READ,	&tWifiAPSecurityLeafOP},
	{"ModeEnabled",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
	{"WEPKey",              eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
	{"PreSharedKey",        eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
	{"KeyPassphrase",       eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
	{"RekeyingInterval",    eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
	{"RadiusServerIPAddr",  eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
	{"RadiusServerPort",    eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
	{"RadiusSecret",        eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPSecurityLeafOP},
};
enum eWifiAPSecurityLeaf
{
	eWifiAPSecurityModesSupported,
	eWifiAPSecurityModeEnabled,
	eWifiAPSecurityWEPKey,
	eWifiAPSecurityPreSharedKey,
	eWifiAPSecurityKeyPassphrase,
	eWifiAPSecurityRekeyingInterval,
	eWifiAPSecurityRadiusServerIPAddr,
	eWifiAPSecurityRadiusServerPort,
	eWifiAPSecurityRadiusSecret
};
struct CWMP_LEAF tWifiAPSecurityLeaf[] =
{
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityModesSupported] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityModeEnabled] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityWEPKey] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityPreSharedKey] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityKeyPassphrase] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityRekeyingInterval] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityRadiusServerIPAddr] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityRadiusServerPort] },
	{ &tWifiAPSecurityLeafInfo[eWifiAPSecurityRadiusSecret] },
	{ NULL }
};


/*******************************************************************************
Device.WiFi.AccessPoint.{i}.WPS.
*******************************************************************************/
struct CWMP_OP tWifiAPWpsLeafOP = { getWifiAPWps, setWifiAPWps };
struct CWMP_PRMT tWifiAPWpsLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Enable",                  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWifiAPWpsLeafOP},
	{"ConfigMethodsSupported",  eCWMP_tSTRING,	CWMP_READ,	&tWifiAPWpsLeafOP},
	{"ConfigMethodsEnabled",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWifiAPWpsLeafOP},
};
enum eWifiAPWpsLeaf
{
	eWifiAPWpsEnable,
	eWifiAPWpsConfigMethodsSupported,
	eWifiAPWpsConfigMethodsEnabled
};
struct CWMP_LEAF tWifiAPWpsLeaf[] =
{
	{ &tWifiAPWpsLeafInfo[eWifiAPWpsEnable] },
	{ &tWifiAPWpsLeafInfo[eWifiAPWpsConfigMethodsSupported] },
	{ &tWifiAPWpsLeafInfo[eWifiAPWpsConfigMethodsEnabled] },
	{ NULL }
};

/*******************************************************************************
Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i} Entity
*******************************************************************************/
struct CWMP_OP tWifiAPAssocEntityLeafOP = { getWifiAPAssocats, NULL };
struct CWMP_PRMT tWifiAPAssocEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"MACAddress",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWifiAPAssocEntityLeafOP},
	{"AuthenticationState", eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tWifiAPAssocEntityLeafOP},
	{"LastDataDownlinkRate",eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiAPAssocEntityLeafOP},
	{"LastDataUplinkRate",  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiAPAssocEntityLeafOP},
	{"SignalStrength",      eCWMP_tINT,		CWMP_READ|CWMP_DENY_ACT,	&tWifiAPAssocEntityLeafOP},
	{"Retransmissions",     eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWifiAPAssocEntityLeafOP},
	{"Active",              eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tWifiAPAssocEntityLeafOP},
};
enum eWifiAPAssocEntityLeaf
{
	eWifiAPAssocMACAddress,
	eWifiAPAssocAuthenticationState,
	eWifiAPAssocLastDataDownlinkRate,
	eWifiAPAssocLastDataUplinkRate,
	eWifiAPAssocSignalStrength,
	eWifiAPAssocRetransmissions,
	eWifiAPAssocActive
};
struct CWMP_LEAF tWifiAPAssocEntityLeaf[] =
{
	{ &tWifiAPAssocEntityLeafInfo[eWifiAPAssocMACAddress] },
	{ &tWifiAPAssocEntityLeafInfo[eWifiAPAssocAuthenticationState] },
	{ &tWifiAPAssocEntityLeafInfo[eWifiAPAssocLastDataDownlinkRate] },
	{ &tWifiAPAssocEntityLeafInfo[eWifiAPAssocLastDataUplinkRate] },
	{ &tWifiAPAssocEntityLeafInfo[eWifiAPAssocSignalStrength] },
	{ &tWifiAPAssocEntityLeafInfo[eWifiAPAssocRetransmissions] },
	{ &tWifiAPAssocEntityLeafInfo[eWifiAPAssocActive] },
	{ NULL }
};

/*******************************************************************************
Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkWifiAPAssocOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkWifiAPAssocOjbect
{
	eLinkWifiAPAssoc0
};
struct CWMP_LINKNODE tLinkWifiAPAssocObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkWifiAPAssocOjbectInfo[eLinkWifiAPAssoc0],	tWifiAPAssocEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Device.WiFi.AccessPoint.{i}.Security.
Device.WiFi.AccessPoint.{i}.WPS.
Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.
*******************************************************************************/
struct CWMP_OP tWifiAPAssocDev_OP = { NULL, objWifiAPAssocDev };
struct CWMP_PRMT tWifiAPSecurityObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Security",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"WPS",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"AssociatedDevice",eCWMP_tOBJECT,	CWMP_READ,	&tWifiAPAssocDev_OP},
};

enum eWifiAPObject
{
	eWifiAPSecurity,
	eWifiAPWPS,
	eWifiAPAssociatedDevice
};

struct CWMP_NODE tWifiAPObject[] =
{
	/*info,  					leaf,			node)*/
	{&tWifiAPSecurityObjectInfo[eWifiAPSecurity],	&tWifiAPSecurityLeaf,	NULL},
	{&tWifiAPSecurityObjectInfo[eWifiAPWPS],	&tWifiAPWpsLeaf,	NULL},
	{&tWifiAPSecurityObjectInfo[eWifiAPAssociatedDevice],	NULL,	NULL},
	{NULL,						NULL,			NULL}
};

/*******************************************************************************
Device.WiFi.AccessPoint.{i} LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkWifiAPOjbectInfo[] =
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
};
enum eLinkWifiAPOjbect
{
	eLinkWifiAP1,
	eLinkWifiAP2,
	eLinkWifiAP3,
	eLinkWifiAP4,
	eLinkWifiAP5,
	eLinkWifiAP6,
	eLinkWifiAP7,
	eLinkWifiAP8,
	eLinkWifiAP9,
	eLinkWifiAP10
};
struct CWMP_NODE tLinkWifiAPObject[] =
{
	/*info,  				leaf,			next)*/
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP1],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP2],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP3],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP4],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP5],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP6],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP7],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP8],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP9],	tWifiAPEntityLeaf,	tWifiAPObject},
	{&tLinkWifiAPOjbectInfo[eLinkWifiAP10],	tWifiAPEntityLeaf,	tWifiAPObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Device.WiFi.Radio.{i}
Device.WiFi.SSID.{i}
Device.WiFi.AccessPoint.{i}
*******************************************************************************/
struct CWMP_PRMT tWifiObjectInfo[] =
{
	/*(name,			type,		flag,			)*/
	{"Radio",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"SSID",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"AccessPoint",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eWifiObject
{
	eWifiRadio,
	eWifiSSID,
	eWifiAccessPoint
};

struct CWMP_NODE tWifiObject[] =
{
	/*info,  					leaf,			node)*/
	{&tWifiObjectInfo[eWifiRadio],	NULL,	&tLinkWifiRadioObject},
	{&tWifiObjectInfo[eWifiSSID],	NULL,	&tLinkWifiSsidObject},
	{&tWifiObjectInfo[eWifiAccessPoint],	NULL,	&tLinkWifiAPObject},
	{NULL,						NULL,			NULL}
};


/*******************************************************************************
Function
*******************************************************************************/
int _getWLANIdxFromInstNum(int instnum, int *rootIdx, int *vwlan_idx)
{
	int maxWLANIface=NUM_WLAN_INTERFACE;
	int maxVWLANIface =NUM_VWLAN_INTERFACE;
	
	if(instnum ==1){
		*rootIdx=0;
		*vwlan_idx=0;
	}else{
		if(maxWLANIface > 1 ){ //DualBand
			if(maxVWLANIface > 4 ){ //MBSSID included && UNIVERSAL_REPEATER included
				if(instnum < (maxWLANIface+maxVWLANIface+1)){
					*rootIdx=0;
					*vwlan_idx=instnum-1;
				}else if(instnum >= (maxWLANIface+maxVWLANIface+1)){
					*rootIdx=1;
					*vwlan_idx=(instnum-(maxWLANIface+maxVWLANIface))-1;
				}
					
			}else if (maxVWLANIface == 4) {//MBSSID included && NOT UNIVERSAL_REPEATER included
				
				if(instnum < (maxWLANIface+maxVWLANIface)){
					*rootIdx=0;
					*vwlan_idx=instnum-1;
				}else if(instnum >= (maxWLANIface+maxVWLANIface)){
					*rootIdx=1;
					*vwlan_idx=(instnum-(maxWLANIface+maxVWLANIface))-1;
				}
				
			
			}else {
					
				//MBSSID NOT included
				if(maxVWLANIface > 0){ //UNIVERSAL_REPEATER included ONLY
					if(instnum==2){
						*rootIdx=0;
						*vwlan_idx=1;
					}else if (instnum==3){
						*rootIdx=1;
						*vwlan_idx=0;
					}else if (instnum==4){
						*rootIdx=1;
						*vwlan_idx=1;
					}
				}else{
					if(instnum==2){
						*rootIdx=1;
						*vwlan_idx=0;
					}else{
						*rootIdx=0;
						*vwlan_idx=0;
					}
				}
			}
		
		}else {//singleBand

			*rootIdx=0;
			*vwlan_idx=instnum-1;

		}
	}
		return 0;
}

extern int loadWLANAssInfoByInstNum( unsigned int instnum );
extern unsigned int gWLANTotalClients;
int objWifiAPAssocDev(char *name, struct CWMP_LEAF *e, int type, void *data)
{
#if 0 // rewrite
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	unsigned int wlaninst=0;
		
	wlaninst = getInstanceNum(name, "AccessPoint");

	//tr181_printf("name %s wlaninst %d", name, wlaninst);
	
	if( wlaninst<1 || wlaninst>10 ) return ERR_9007;

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			*c=NULL;

			loadWLANAssInfoByInstNum(wlaninst);
			//tr181_printf("gWLANTotalClients %d", gWLANTotalClients);
			if(gWLANTotalClients>0)
				return create_Object( c, tLinkWifiAPAssocObject, sizeof(tLinkWifiAPAssocObject), gWLANTotalClients, 1 );

			return 0;
			break;
		}
		case eCWMP_tUPDATEOBJ:
		{
			unsigned int num,i;
			struct CWMP_LINKNODE *old_table;

			loadWLANAssInfoByInstNum(wlaninst);
			//tr181_printf("gWLANTotalClients %d", gWLANTotalClients);
			num = gWLANTotalClients;
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
					add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tLinkWifiAPAssocObject, sizeof(tLinkWifiAPAssocObject), &InstNum );
				}
			}
			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE*)old_table );
			return 0;
			break;
		}
	}
#endif
	return -1;
}

int getWifiAPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	extern int MaxWLANIface;
	char	*lastname = entity->info->name;
	unsigned long bs=0,br=0,ps=0,pr=0;
	unsigned char buf[256]="";
	unsigned int vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	bss_info bss;
	unsigned int GetValue=0;
	unsigned short uShort;
	unsigned int wpa_cipher=0, wpa2_cipher=0;
	char wlan_ifname[10]={0};
	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getInstanceNum(name, "AccessPoint");

	//tr181_printf("wlaninst %d", wlaninst);

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;

	if(vwlanIdx == 0)
		sprintf(wlan_ifname,"wlan%d",rootIdx);
	else
		sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
	
	//CWMPDBG( 1, ( stderr, "<%s:%d>target:<%d,%d>\n", __FUNCTION__, __LINE__, rootIdx,vwlanIdx ) );
		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		int func_off;
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&func_off);

		if(vChar==1 || func_off == 1)
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int func_off;
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&func_off);

		if(vChar==1 || func_off == 1)
			*data = strdup("Disabled");
		else
			*data = strdup("Enabled");
	}
	else if( strcmp( lastname, "SSIDReference" )==0 )
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "Device.WiFi.SSID.%d.", wlaninst);
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
		getWlanMib(rootIdx, 0,MIB_WLAN_HIDDEN_SSID,(void *)&GetValue);
		*data = booldup( (GetValue==0) );
	}
	else if( strcmp( lastname, "RetryLimit" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RETRY_LIMIT, (void *)&vChar);
		*data = uintdup(vChar);
	}
	else if( strcmp( lastname, "WMMCapability" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "UAPSDCapability" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "WMMEnable" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WMM_ENABLED,(void *)&GetValue);
		*data = booldup( (GetValue==1) );
	}

	else if( strcmp( lastname, "UAPSDEnable" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_UAPSD_ENABLED, (void *)&vChar);
		*data = uintdup(vChar);
	}

	else if( strcmp( lastname, "AssociatedDeviceNumberOfEntries" )==0 )
	{
		loadWLANAssInfoByInstNum(wlaninst);
		*data = uintdup(gWLANTotalClients);
	}
	else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int setWifiAPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int	vChar=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	int NewRootIdx=0, NewvwlanIdx=0;
	int wpa_cipher=0, wpa2_cipher=0;
	unsigned int SetValue=0;
	unsigned int GetValue=0;
	unsigned short uShort;

	
	int isWLANMIBUpdated=1;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	int wlanBand2G5GSelect=0;
	int MaxWlanIface=NUM_WLAN_INTERFACE;

	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wlaninst = getInstanceNum(name, "AccessPoint");

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;


	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		//pcwmpWlan->func_off=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
		int *i = data;
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		//pcwmpWlan->hiddenSSID=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_HIDDEN_SSID, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "RetryLimit" )==0 )
	{
		unsigned int *i = data;
		
		if( i==NULL ) return ERR_9007;
		
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RETRY_LIMIT, (void *)i);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "WMMEnable" )==0 )
	{
		int *i = data;
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		//pcwmpWlan->hiddenSSID=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WMM_ENABLED, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "UAPSDEnable" )==0 )
	{
		int *i = data;
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		//pcwmpWlan->hiddenSSID=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_UAPSD_ENABLED, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else{
		return ERR_9005;
	}

	if(isWLANMIBUpdated)
		return 1;
	else
#endif
		return 0;
}

int getWifiSsidEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	extern int MaxWLANIface;
	char	*lastname = entity->info->name;
	unsigned long bs=0,br=0,ps=0,pr=0;
	unsigned char buf[256]="";
	unsigned int vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	bss_info bss;
	unsigned int GetValue=0;
	unsigned short uShort;
	unsigned int wpa_cipher=0, wpa2_cipher=0;
	char wlan_ifname[10]={0};
	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getInstanceNum(name, "SSID");

	//tr181_printf("wlaninst %d", wlaninst);

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;

	if(vwlanIdx == 0)
		sprintf(wlan_ifname,"wlan%d",rootIdx);
	else
		sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
	
	//CWMPDBG( 1, ( stderr, "<%s:%d>target:<%d,%d>\n", __FUNCTION__, __LINE__, rootIdx,vwlanIdx ) );
		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		int func_off;
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&func_off);

		if(vChar==1 || func_off == 1)
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
	}
	else if( strcmp( lastname, "BSSID" )==0 )
	{
		getWlanMib(rootIdx, 0,MIB_WLAN_WLAN_DISABLED,(void *)&vUint);
		if(vUint == 0){
			getWlanBssInfo(rootIdx, vwlanIdx, (void*)&bss);
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x", bss.bssid[0], bss.bssid[1], bss.bssid[2], bss.bssid[3], bss.bssid[4], bss.bssid[5]);	
		}
		else
		{
			strcpy(buf,"00:00:00:00:00:00");
		}		

		*data=strdup(buf);
	}
	else if( strcmp( lastname, "MACAddress" )==0 )
	{
		getWlanMib(rootIdx, 0,MIB_WLAN_WLAN_DISABLED,(void *)&vUint);
		if(vUint == 0){
			getWlanBssInfo(rootIdx, vwlanIdx, (void*)&bss);
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x", bss.bssid[0], bss.bssid[1], bss.bssid[2], bss.bssid[3], bss.bssid[4], bss.bssid[5]);	
		}
		else
		{
			strcpy(buf,"00:00:00:00:00:00");
		}		

		*data=strdup(buf);
	}
	else if( strcmp( lastname, "SSID" )==0 )
	{
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_SSID, (void *)buf);
		*data = strdup( buf );
	}
	else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int setWifiSsidEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int	vChar=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	int NewRootIdx=0, NewvwlanIdx=0;
	int wpa_cipher=0, wpa2_cipher=0;
	unsigned int SetValue=0;
	unsigned int GetValue=0;
	unsigned short uShort;

	
	int isWLANMIBUpdated=1;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	int wlanBand2G5GSelect=0;
	int MaxWlanIface=NUM_WLAN_INTERFACE;

	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wlaninst = getInstanceNum(name, "SSID");

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;


	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		//pcwmpWlan->func_off=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "SSID" )==0 )
	{						
		//MIB_WLAN_SSID
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)==0) || (strlen(buf)>=MAX_SSID_LEN) ) return ERR_9007;

		//sprintf(pcwmpWlan->ssid, "%s", buf);
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_SSID, (void *)buf);
		isWLANMIBUpdated=1;
	}
	else{
		return ERR_9005;
	}

	if(isWLANMIBUpdated)
		return 1;
	else
#endif
		return 0;
}

int getWifiRadioEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	unsigned int raInst=0;
	unsigned int vChar=0;
	int rootIdx=0,vwlanIdx=0;
	char wlan_ifname[10]={0};
	unsigned char buf[256]="";
	bss_info bss;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	raInst = getInstanceNum(name, "Radio");
	if (raInst > 1)
		return ERR_9007;

	if (raInst == 0) { // wlan0
		rootIdx = 0;
		vwlanIdx = 0;
	}
	else { // wlan1
		rootIdx = 1;
		vwlanIdx = 0;
	}
	
	sprintf(wlan_ifname,"wlan%d",raInst);

	//tr181_printf("wlan_ifname %s", wlan_ifname);

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int func_off;
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&func_off);

		if(vChar==1 || func_off == 1)
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int flags=0;
		if( getInFlags(wlan_ifname, &flags)==1 )
		{
			if (flags & IFF_UP)
				*data = strdup( "Up" );
		else
				*data = strdup( "Disabled" );
		}else
			*data = strdup( "Error" );
	}
	else if( strcmp( lastname, "Name" )==0 )
	{
		if (raInst == 0) // wlan0
			*data = strdup("wlan0");
		else // wlan1
			*data = strdup("wlan1");
	}
	else if( strcmp( lastname, "LastChange" )==0 )
	{
		*data = uintdup( 0 );
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "Upstream" )==0 )
	{
		*data = booldup( 0 );
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_BAND, (void *)&vChar);
		if( vChar==BAND_11B ) //2.4 GHz (B)
			*data = uintdup( 11 ); 
		else if( vChar==BAND_11G )//2.4 GHz (G)
			*data = uintdup( 54 );
		else if( vChar==BAND_11BG )//2.4 GHz (B+G)
			*data = uintdup( 54 );
		else if( vChar==BAND_11A )
			*data = uintdup( 54 );
		else if( vChar==BAND_11N )
			*data = uintdup( 144 );
		else if( vChar==BAND_5G_11AN )
			*data = uintdup( 144 );
		else if( vChar==(BAND_11B|BAND_11G|BAND_11N) )
			*data = uintdup( 144 );
		else /* 0 */
			*data = uintdup( 0 );//return ERR_9002;
	}
	else if( strcmp( lastname, "SupportedFrequencyBands" )==0 )
	{
		if (raInst == 0) // wlan0
			*data = strdup("5GHz");
		else // wlan1
			*data = strdup("2.4GHz");
	}
	else if( strcmp( lastname, "OperatingFrequencyBand" )==0 )
	{
		if (raInst == 0) // wlan0
			*data = strdup("5GHz");
		else // wlan1
			*data = strdup("2.4GHz");
	}
	else if( strcmp( lastname, "SupportedStandards" )==0 )
	{
		if (raInst == 0) // wlan0
			*data = strdup("a,n");
		else // wlan1
			*data = strdup("b,g,n");
	}
	else if( strcmp( lastname, "OperatingStandards" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_BAND, (void *)&vChar);
		
		//CWMPDBG( 1, ( stderr, "<%s:%d>target:<%d,%d>, WiFi Standard=%d\n", __FUNCTION__, __LINE__, rootIdx, vwlanIdx, vChar ) );
		if( vChar==BAND_11B )
			*data = strdup( "b" );
		else if( vChar==BAND_11G )
			*data = strdup( "g" );
		else if( vChar==BAND_11BG )
			*data = strdup( "b,g" );
		else if( vChar==BAND_11A)
			*data = strdup( "a" );
		else if( vChar==BAND_11N )
			*data = strdup( "n" );
		else if( vChar==(BAND_11G|BAND_11N))
			*data = strdup( "g,n" );
		else if( vChar==(BAND_11B|BAND_11G|BAND_11N))
			*data = strdup( "b,g,n" );
		else if( vChar==BAND_5G_11AN)
			*data = strdup( "a,n" );
		else if( vChar==BAND_5G_11AC)
			*data = strdup( "ac" );
		else if( vChar==BAND_5G_11AAC)
			*data = strdup( "a,ac" );
		else if( vChar==BAND_5G_11NAC)
			*data = strdup( "n,ac" );
		else if( vChar==BAND_5G_11ANAC)
			*data = strdup( "a,n,ac" );
		else 
			*data = strdup( "" ); 
	}
	else if( strcmp( lastname, "PossibleChannels" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_BAND, (void *)&vChar);

		if( vChar == BAND_5G_11AN)
			*data = strdup("34-165");
		else if( vChar & (BAND_11B|BAND_11G|BAND_11N))
			*data = strdup("1-14");
		else
			*data = strdup( "" ); 
	}
	else if( strcmp( lastname, "ChannelsInUse" )==0 )
	{
		char WlanIf[32];

		sprintf(WlanIf, "wlan%d", rootIdx);
		if ( getWlBssInfo(WlanIf, &bss) < 0)
			return -1;

		if (bss.channel)
			sprintf( buf, "%u", bss.channel );
		else
			sprintf( buf, "%s", "Auto");
				
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "Channel" )==0 )
	{	
		getWlanMib(rootIdx, 0, MIB_WLAN_CHANNEL, (void *)&vChar);

		if (vChar == 0) // AutoChannelEnable == 1
		{
			char WlanIf[32];

			sprintf(WlanIf, "wlan%d", rootIdx);
			if ( getWlBssInfo(WlanIf, &bss) < 0)
				return -1;

			vChar = bss.channel;
		}
		*data = uintdup( (unsigned int)vChar );
	}
	else if( strcmp( lastname, "AutoChannelSupported" )==0 )
	{
		*data = booldup( 1 );
	}
	else if( strcmp( lastname, "AutoChannelEnable" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_CHANNEL, (void *)&vChar);
		if ((unsigned int)vChar == 0) {
			*data = booldup( 1 );
		}
		else {
			*data = booldup( 0 );
		}
	}
	else if( strcmp( lastname, "AutoChannelRefreshPeriod" )==0 )
	{
		*data = uintdup( 0 );
	}
	else if( strcmp( lastname, "OperatingChannelBandwidth" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_CHANNEL_BONDING, (void *)&vChar);
		if ((unsigned int)vChar == 0)
			*data = strdup( "20MHz" );
		else
			*data = strdup( "40MHz" );
	}
	else if( strcmp( lastname, "ExtensionChannel" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_CHANNEL_BONDING, (void *)&vChar);

		if ((unsigned int)vChar == 0) // 20MHz
			*data = strdup( "" );
		else // 40MHz
		{
			getWlanMib(rootIdx, 0, MIB_WLAN_CONTROL_SIDEBAND, (void *)&vChar);

			if ((unsigned int)vChar == 0)
				*data = strdup( "BelowControlChannel" );
			else
				*data = strdup( "AboveControlChannel" );
		}
	}
	else if( strcmp( lastname, "GuardInterval" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_SHORT_GI, (void *)&vChar);

		if (vChar == 1)
			*data = strdup( "400nsec" );
		else
			*data = strdup( "800nsec" );
	}
	else if( strcmp( lastname, "MCS" )==0 )
	{
		int mcs;
		getWlanMib(rootIdx, 0, MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);

		if (vChar == 1)
			*data = intdup( -1 );
		else
		{
			getWlanMib(rootIdx, 0, MIB_WLAN_FIX_RATE, (void *)&vChar);
			mcs = log2(vChar)-12;
			if (mcs >=0 && mcs <= 15)
				*data = intdup( mcs );
			else
				*data = intdup( -1 );
		}
	}
	else if( strcmp( lastname, "TransmitPowerSupported" )==0 )
	{
		*data = strdup("15,35,50,70,100");
	}
	else if( strcmp( lastname, "TransmitPower" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_RFPOWER_SCALE, (void *)&vChar);

		if ((int)vChar == 0)
			*data = intdup( 100 );
		else if ((int)vChar == 1)
			*data = intdup( 70 );
		else if ((int)vChar == 2)
			*data = intdup( 50 );
		else if ((int)vChar == 3)
			*data = intdup( 35 );
		else if ((int)vChar == 4)
			*data = intdup( 15 );
	}
	else if( strcmp( lastname, "IEEE80211hSupported" )==0 )
	{
		if (raInst == 0) // wlan0
			*data = booldup( 1 );
		else // wlan1
			*data = booldup( 0 );
	}
	else if( strcmp( lastname, "IEEE80211hEnabled" )==0 )
	{
		if (raInst == 0) // wlan0
			*data = booldup( 1 );
		else // wlan1
			*data = booldup( 0 );
	}
	else if( strcmp( lastname, "RegulatoryDomain" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_REGULATORY_DOMAIN, (void *)buf);
		*data = strdup(buf);
	}
	else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int setWifiRadioEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	unsigned int raInst=0;
	int rootIdx=0,vwlanIdx=0;
	char wlan_ifname[10]={0};

	int isWLANMIBUpdated=1;
	unsigned int	vChar=0;
	char *buf = data;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	raInst = getInstanceNum(name, "Radio");
	if (raInst > 1)
		return ERR_9007;

	if (raInst == 0) { // wlan0
		rootIdx = 0;
		vwlanIdx = 0;
	}
	else { // wlan1
		rootIdx = 1;
		vwlanIdx = 0;
	}
	
	sprintf(wlan_ifname,"wlan%d",raInst);

	//tr181_printf("wlan_ifname %s", wlan_ifname);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		
		vChar = (*i==0)?1:0;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		return ERR_9007; // layer 1 IF, not used.
	}
	else if( strcmp( lastname, "OperatingFrequencyBand" )==0 )
	{
		return ERR_9007;
	}
	else if( strcmp( lastname, "OperatingStandards" )==0 )
	{
		if (buf == NULL) return ERR_9007;
		if (strlen(buf) == 0) return ERR_9007;
		
		if (raInst == 0) { // wlan0
			if (strcmp(buf, "a") == 0)
				vChar = BAND_11A;
			else if (strcmp(buf, "n") == 0)
				vChar = BAND_11N;
			else if ((strcmp(buf, "a,n") == 0) || (strcmp(buf, "n,a") == 0))
				vChar = BAND_5G_11AN;
		}
		else { // wlan1
			if (strcmp(buf, "b") == 0)
				vChar = BAND_11B;
			else if (strcmp(buf, "g") == 0)
				vChar = BAND_11G;
			else if (strcmp(buf, "n") == 0)
				vChar = BAND_11N;
			else if ((strcmp(buf, "b,g") == 0) || (strcmp(buf, "g,b") == 0))
				vChar = BAND_11BG;
			else if ((strcmp(buf, "g,n") == 0) || (strcmp(buf, "n,g") == 0))
				vChar = BAND_11G | BAND_11N;
			else if ((strcmp(buf, "b,g,n") == 0) || (strcmp(buf, "b,n,g") == 0) ||
				     (strcmp(buf, "g,n,b") == 0) || (strcmp(buf, "g,b,n") == 0) ||
				     (strcmp(buf, "n,b,g") == 0) || (strcmp(buf, "n,g,b") == 0))
				vChar = BAND_11B | BAND_11G | BAND_11N;
		}

		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_BAND, (void *)&vChar);;
	}
	else if( strcmp( lastname, "Channel" )==0 )
	{
		unsigned int *i = data;

		if (i == NULL) return ERR_9007;

		if (raInst == 0) { // wlan0
			if(*i != 0) { // 0:auto
				int valid = 0;

				if( (*i>=34) && (*i<=165) ) 
					valid = 1;
				else
					valid =0;

				if( valid==0 ) return ERR_9007;		
			}
		}
		else { // wlan1
			if(*i != 0) { // 0:auto
				int valid = 0;

				if( (*i>=1) && (*i<=14) ) 
					valid = 1;
				else
					valid =0;

				if( valid==0 ) return ERR_9007;		
			}
		}

		vChar = *i;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_CHANNEL, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "AutoChannelEnable" )==0 )
	{
		int *i = data;

		if (i == NULL) return ERR_9007;

		if (*i == 1) {
			vChar = 0;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_CHANNEL, (void *)&vChar);
			isWLANMIBUpdated = 1;
		}
		else {
			bss_info bss;
			char WlanIf[32];

			sprintf(WlanIf, "wlan%d", rootIdx);
			if ( getWlBssInfo(WlanIf, &bss) < 0)
				return -1;

			vChar = bss.channel;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_CHANNEL, (void *)&vChar);
			isWLANMIBUpdated = 1;

		}
	}
	else if( strcmp( lastname, "AutoChannelRefreshPeriod" )==0 )
	{
		unsigned int *i = data;

		if (i == NULL) return ERR_9007;
		if (*i != 0) return ERR_9007;

		isWLANMIBUpdated = 0;
	}
	else if( strcmp( lastname, "OperatingChannelBandwidth" )==0 )
	{
		if (buf == NULL) return ERR_9007;
		if (strlen(buf) == 0) return ERR_9007;

		if (strcmp(buf, "20MHz") == 0)
			vChar = 0;
		else if (strcmp(buf, "40MHz") == 0)
			vChar = 1;
		else
			vChar = 0;

		setWlanMib(rootIdx, 0, MIB_WLAN_CHANNEL_BONDING, (void *)&vChar);
		isWLANMIBUpdated = 1;
	}
	else if( strcmp( lastname, "ExtensionChannel" )==0 )
	{
		if (buf == NULL) return ERR_9007;
		if (strlen(buf) == 0) return ERR_9007;

		getWlanMib(rootIdx, 0, MIB_WLAN_CHANNEL_BONDING, (void *)&vChar);

		if ((unsigned int)vChar == 1) // 40MHz
		{
			if (strcmp(buf, "AboveControlChannel") == 0)
				vChar = 1;
			else if (strcmp(buf, "BelowControlChannel") == 0)
				vChar = 0;

			setWlanMib(rootIdx, 0, MIB_WLAN_CONTROL_SIDEBAND, (void *)&vChar);	
		}
		else // 20MHz
			return ERR_9001;
	}
	else if( strcmp( lastname, "GuardInterval" )==0 )
	{
		unsigned char vChar;
		
		if (buf == NULL) return ERR_9007;
		if (strlen(buf) == 0) return ERR_9007;

		if (strcmp(buf, "400nsec") == 0)
			vChar = 1;
		else if (strcmp(buf, "800nsec") == 0)
			vChar = 0;
		else if (strcmp(buf, "Auto") == 0)
			return ERR_9001;
		else
			return ERR_9007;

		setWlanMib(rootIdx, 0, MIB_WLAN_SHORT_GI, (void *)&vChar);
	}
	else if( strcmp( lastname, "MCS" )==0 )
	{
		int mcs;
		int *i = data;

		if (i == NULL) return ERR_9007;

		if (*i == -1) // Auto 
		{
			vChar = 1;
			setWlanMib(rootIdx, 0, MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		}
		else if (*i >= 0 && *i <= 15)
		{
			vChar = 0;
			setWlanMib(rootIdx, 0, MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);

			mcs = 1 << (*i+12);
			setWlanMib(rootIdx, 0, MIB_WLAN_FIX_RATE, (void *)&mcs);
		}
		else
			return ERR_9007;
	}
	else if( strcmp( lastname, "TransmitPower" )==0 )
	{
		int *i = data;

		if (i == NULL) return ERR_9007;

		if (*i == 100)
			vChar = 0;
		else if (*i == 70)
			vChar = 1;
		else if (*i == 50)
			vChar = 2;
		else if (*i == 35)
			vChar = 3;
		else if (*i == 15)
			vChar = 4;
		else if (*i == -1)
			vChar = 0;

		setWlanMib(rootIdx, 0, MIB_WLAN_RFPOWER_SCALE, (void *)&vChar);
			
	}
#if 0
	else if( strcmp( lastname, "IEEE80211hEnabled" )==0 )
	{
	}
#endif
	else if( strcmp( lastname, "RegulatoryDomain" )==0 )
	{
		int regDomain;
	
		if (buf == NULL) return ERR_9007;
		if (strlen(buf) != 3) return ERR_9007;

		//tr181_printf("RegulatoryDomain=%s", buf);
		
		if (buf[0] < 'A' || buf[0] > 'Z') { /*tr181_trace();*/ return ERR_9007; }
		if (buf[1] < 'A' || buf[1] > 'Z') { /*tr181_trace();*/ return ERR_9007; }
		if (buf[2] != ' ' && buf[2] != 'I' && buf[2] != 'O') { /*tr181_trace();*/ return ERR_9007; }

		setWlanMib(rootIdx, 0, MIB_WLAN_REGULATORY_DOMAIN, (void *)buf);

		if (strncmp(buf, "US", 2) == 0 ||
			strncmp(buf, "CA", 2) == 0)
		{
			//tr181_trace();
			regDomain = FCC;
		}
		else if (strncmp(buf, "JP", 2) == 0)
		{
			//tr181_trace();
			regDomain = MKK;
		}
		else
		{
			//tr181_trace();
			regDomain = ETSI;
		}

		mib_set( MIB_HW_REG_DOMAIN, (void *)&regDomain);
		isWLANMIBUpdated = 1;
	}
	else
	{
		return ERR_9005;
	}
	
	if(isWLANMIBUpdated)
		return 1;
	else
#endif
		return 0;
}

int getWifiRaStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	unsigned int raInst=0;
	unsigned int vChar=0;
	int rootIdx=0,vwlanIdx=0;
	struct user_net_device_stats nds;

	raInst = getInstanceNum(name, "Radio");

	if (raInst > 1)
		return ERR_9007;

	if (raInst == 0) { // wlan0
		rootIdx = 0;
		vwlanIdx = 0;
	}
	else { // wlan1
		rootIdx = 1;
		vwlanIdx = 0;
	}

	if (raInst == 0)
	{
		sprintf(ifname, "%s","wlan0");
	}
	else if(raInst == 1)
	{
		sprintf(ifname, "%s","wlan1");
	}
	else
	{
		sprintf(ifname, "%s","wlan0");
	}
	
	if(getStats(ifname, &nds) < 0)
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
	else if (strcmp(lastname, "DiscardPacketsSent") == 0) 
	{
		*data = uintdup(nds.tx_dropped);
	} 
	else if (strcmp(lastname, "DiscardPacketsReceived") == 0) 
	{
		*data = uintdup(nds.rx_dropped);
	} 
	else
		return ERR_9005;
#endif
	return 0;
}

int getWifiSsidStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char ifname[IF_NAME_SIZE];
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	unsigned int vChar=0;
	int rootIdx=0,vwlanIdx=0;
	struct user_net_device_stats nds;

	wlaninst = getInstanceNum(name, "SSID");

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;

	if(vwlanIdx == 0)
		sprintf(ifname,"wlan%d",rootIdx);
	else if (vwlanIdx == 5)
		sprintf(ifname,"wlan%d-vxd",rootIdx);
	else
		sprintf(ifname,"wlan%d-va%d",rootIdx,vwlanIdx-1);
	
	if(getStats(ifname, &nds) < 0)
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
	else if (strcmp(lastname, "DiscardPacketsSent") == 0) 
	{
		*data = uintdup(nds.tx_dropped);
	} 
	else if (strcmp(lastname, "DiscardPacketsReceived") == 0) 
	{
		*data = uintdup(nds.rx_dropped);
	} 
	else
		return ERR_9005;
#endif
	return 0;
}

int getWifiAPSecurity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	extern int MaxWLANIface;
	char	*lastname = entity->info->name;
	unsigned long bs=0,br=0,ps=0,pr=0;
	unsigned char buf[256]="";
	unsigned int vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	bss_info bss;
	unsigned int GetValue=0;
	unsigned short uShort;
	unsigned int wpa_cipher=0, wpa2_cipher=0;
	char wlan_ifname[10]={0};
	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getInstanceNum(name, "AccessPoint");

	//tr181_printf("wlaninst %d", wlaninst);

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;

	if(vwlanIdx == 0)
		sprintf(wlan_ifname,"wlan%d",rootIdx);
	else
		sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
	
	//CWMPDBG( 1, ( stderr, "<%s:%d>target:<%d,%d>\n", __FUNCTION__, __LINE__, rootIdx,vwlanIdx ) );
		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ModesSupported" )==0 )
	{
		*data = strdup("None,WEP-64,WEP-128,WPA-Personal,WPA-Enterprise,"
                       "WPA2-Personal,WPA2-Enterprise,"
                       "WPA-WPA2-Personal,WPA-WPA2-Enterprise");
	}
	else if ( strcmp( lastname, "ModeEnabled" )==0 )
	{
		unsigned int encrypt = 0;
		unsigned int wep = 0;
		unsigned int auth = 0;
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&wep);
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&auth);

		if (encrypt == ENCRYPT_DISABLED) {
			*data = strdup("None");
		}
		else if (encrypt == ENCRYPT_WEP) {
			if (wep == WEP64) {
				*data = strdup("WEP-64");
			}
			else if (wep == WEP128) {
				*data = strdup("WEP-128");
			}
			else {
				*data = strdup("None");
			}
		}
		else if (encrypt == ENCRYPT_WPA) {
			if (auth == WPA_AUTH_AUTO) {
				*data = strdup("WPA-Enterprise");
			}
			else if (auth == WPA_AUTH_PSK) {
				*data = strdup("WPA-Personal");
			}
		}
		else if (encrypt == ENCRYPT_WPA2) {
			if (auth == WPA_AUTH_AUTO) {
				*data = strdup("WPA2-Enterprise");
			}
			else if (auth == WPA_AUTH_PSK) {
				*data = strdup("WPA2-Personal");
			}
		}
		else if (encrypt == ENCRYPT_WPA2_MIXED) {
			if (auth == WPA_AUTH_AUTO) {
				*data = strdup("WPA-WPA2-Enterprise");
			}
			else if (auth == WPA_AUTH_PSK) {
				*data = strdup("WPA-WPA2-Personal");
			}
		}
		else {
			*data = strdup("None");
		}
	}
	else if ( strcmp( lastname, "WEPKey" )==0 )
	{
#if 0
		unsigned int keyfmt = WEP64; 	//0:disable, 1:64, 2:128
		unsigned char	hex_key[32], ascii_key[32];

		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&keyfmt);
		if(keyfmt == WEP64) {
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY1, (void *)ascii_key);
		}
		else {
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY1, (void *)ascii_key);
		}

		if( keyfmt!=WEP128 )
			sprintf( hex_key, "%02x%02x%02x%02x%02x",
				ascii_key[0], ascii_key[1], ascii_key[2], ascii_key[3], ascii_key[4] );
		else
			sprintf( hex_key, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				ascii_key[0], ascii_key[1], ascii_key[2], ascii_key[3], ascii_key[4],
				ascii_key[5], ascii_key[6], ascii_key[7], ascii_key[8], ascii_key[9],
				ascii_key[10], ascii_key[11], ascii_key[12] );

		*data = strdup( hex_key );
#else
		*data = strdup("");
#endif
	}
	else if ( strcmp( lastname, "PreSharedKey" )==0 )
	{
#if 0
		unsigned int pskfmt;//0:Passphrase,   1:hex
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
		if(pskfmt==1)
		{
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
			*data = strdup( buf );
		}else
			*data = strdup( "" );
#else
		*data = strdup("");
#endif
	}
	else if ( strcmp( lastname, "KeyPassphrase" )==0 )
	{
#if 0
		unsigned int pskfmt;//0:Passphrase,   1:hex
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
		if(pskfmt==0)
		{
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
			*data = strdup( buf );
		}else
			*data = strdup( "" );
#else
		*data = strdup("");
#endif
	}
	else if ( strcmp( lastname, "RekeyingInterval" )==0 )
	{
		unsigned int rekeying_intv = 0;

		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&rekeying_intv);

		*data = uintdup(rekeying_intv);
	}
	else if ( strcmp( lastname, "RadiusServerIPAddr" )==0 )
	{
		char line[256];
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RS_IP, (void *)buf);

		sprintf(line,"%u.%u.%u.%u\n",*((char *)&buf)&0xff,*(((char *)&buf)+1)&0xff,*(((char *)&buf)+2)&0xff,*(((char *)&buf)+3)&0xff);
		*data = strdup(line);
	}
	else if ( strcmp( lastname, "RadiusServerPort" )==0 )
	{
		unsigned int radiusPort = 0;

		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RS_PORT, (void *)&radiusPort);
		*data = uintdup(radiusPort);
	}
	else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

/*copy from mib.c, because it defines with "static" */
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

int setWifiAPSecurity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int	vChar=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	int NewRootIdx=0, NewvwlanIdx=0;
	int wpa_cipher=0, wpa2_cipher=0;
	unsigned int SetValue=0;
	unsigned int GetValue=0;
	unsigned short uShort;
	int len = 0;

	
	int isWLANMIBUpdated=1;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	int wlanBand2G5GSelect=0;
	int MaxWlanIface=NUM_WLAN_INTERFACE;

	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wlaninst = getInstanceNum(name, "AccessPoint");

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;


	if ( strcmp( lastname, "ModeEnabled" )==0 )
	{
		unsigned int encrypt = 0;
		unsigned int wep = 0;
		unsigned int auth = 0;
		
		if (buf==NULL) return ERR_9007;

		//tr181_printf("ModeEnabled [%s]", buf);

		if (strcmp(buf, "None") == 0) {
			encrypt = ENCRYPT_DISABLED;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WEP-64") == 0) {
			encrypt = ENCRYPT_WEP;
			wep = WEP64;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&wep);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WEP-128") == 0) {
			encrypt = ENCRYPT_WEP;
			wep = WEP128;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&wep);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WPA-Personal") == 0) {
			encrypt = ENCRYPT_WPA;
			auth = WPA_AUTH_PSK;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&auth);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WPA-Enterprise") == 0) {
			encrypt = ENCRYPT_WPA;
			auth = WPA_AUTH_AUTO;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&auth);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WPA2-Personal") == 0) {
			encrypt = ENCRYPT_WPA2;
			auth = WPA_AUTH_PSK;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&auth);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WPA2-Enterprise") == 0) {
			encrypt = ENCRYPT_WPA2;
			auth = WPA_AUTH_AUTO;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&auth);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WPA-WPA2-Personal") == 0) {
			encrypt = ENCRYPT_WPA2_MIXED;
			auth = WPA_AUTH_PSK;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&auth);
			isWLANMIBUpdated = 1;
		}
		else if (strcmp(buf, "WPA-WPA2-Enterprise") == 0) {
			encrypt = ENCRYPT_WPA2_MIXED;
			auth = WPA_AUTH_AUTO;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&encrypt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&auth);
			isWLANMIBUpdated = 1;
		}
		else {
			return ERR_9007;
		}
	}
	else if ( strcmp( lastname, "WEPKey" )==0 )
	{
		unsigned char ascii_key[32];
		int len=0, keyid;
		unsigned int keyfmt,key_type;

		if ( buf==NULL ) return ERR_9007;
		len = strlen(buf);
		if ( (len!=10) && (len!=26) ) return ERR_9007;
		memset( ascii_key, 0, sizeof(ascii_key) );
		if (!string_to_hex(buf, ascii_key, len)) return ERR_9007;

		keyfmt = (len==10)?WEP64:WEP128; //key format==>0:disable, 1:64, 2:128
		key_type = KEY_HEX; //key type==>KEY_ASCII:ascii, KEY_HEX:hex, tr-069 always uses the hex format.
		
		
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&keyfmt);
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP_KEY_TYPE, (void *)&key_type);

		if (keyfmt == WEP64) {
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY1, (void *)ascii_key);
		}
		else {
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY1, (void *)ascii_key);
		}

		isWLANMIBUpdated = 1;
	}
	else if ( strcmp( lastname, "PreSharedKey" )==0 )
	{
		int i,len;
		unsigned char pskfmt;
		
		if( buf==NULL ) return ERR_9007;
		
		len = strlen(buf);
		if( len==0 || len>64 ) return ERR_9007;
	
		for( i=0; i<len; i++ )
			if( _is_hex(buf[i])==0 ) return ERR_9007;
			
		pskfmt = 1;//0:Passphrase,   1:hex
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);

		isWLANMIBUpdated = 1;
	}
	else if ( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		unsigned char pskfmt;
		
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)<8) || (strlen(buf)>63) ) return ERR_9007;

		pskfmt = 0; //0:Passphrase,   1:hex
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);

		isWLANMIBUpdated = 1;
	}
	else if ( strcmp( lastname, "RekeyingInterval" )==0 )
	{
		unsigned int *rekeying_intv = data;

		if( rekeying_intv == NULL ) return ERR_9007;

		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)rekeying_intv);		
		isWLANMIBUpdated = 1;
	}
	else if ( strcmp( lastname, "RadiusServerIPAddr" )==0 )
	{
		struct in_addr inIp;
		
		if (buf == NULL) return ERR_9007;

		//tr181_printf("RadiusServerIPAddr %s(%d)", buf, strlen(buf));
		inet_aton(buf, &inIp);

		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RS_IP, (void *)&inIp);
		
		isWLANMIBUpdated = 1;
	}
	else if ( strcmp( lastname, "RadiusServerPort" )==0 )
	{
		unsigned int *radiusPort = data;

		if( radiusPort == NULL ) return ERR_9007;

		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RS_PORT, (void *)radiusPort);
		isWLANMIBUpdated = 1;
	}
	else
	{
		return ERR_9005;
	}

	if(isWLANMIBUpdated)
		return 1;
	else
#endif
		return 0;
}

int getWifiAPWps(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	extern int MaxWLANIface;
	char	*lastname = entity->info->name;
	unsigned long bs=0,br=0,ps=0,pr=0;
	unsigned char buf[256]="";
	unsigned int vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	bss_info bss;
	unsigned int GetValue=0;
	unsigned short uShort;
	unsigned int wpa_cipher=0, wpa2_cipher=0;
	char wlan_ifname[10]={0};
	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getInstanceNum(name, "AccessPoint");

	//tr181_printf("wlaninst %d", wlaninst);

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;

	if(vwlanIdx == 0)
		sprintf(wlan_ifname,"wlan%d",rootIdx);
	else
		sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
	
	//CWMPDBG( 1, ( stderr, "<%s:%d>target:<%d,%d>\n", __FUNCTION__, __LINE__, rootIdx,vwlanIdx ) );
		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WSC_DISABLE, (void *)&vChar);

		if(vChar==1)
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
	}
	else if ( strcmp( lastname, "ConfigMethodsSupported" )==0 )
	{
		*data = strdup("PushButton,PIN");
	}
	else if ( strcmp( lastname, "ConfigMethodsEnabled" )==0 )
	{
		*data = strdup("PushButton,PIN");
	}
	else
	{
		return ERR_9005;
	}
#endif
	return 0;
}

int setWifiAPWps(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#if 0 // rewrite
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int	vChar=0;
	unsigned int wlaninst=0;
	unsigned int wlaninst1=0;
	int rootIdx=0,vwlanIdx=0;
	int NewRootIdx=0, NewvwlanIdx=0;
	int wpa_cipher=0, wpa2_cipher=0;
	unsigned int SetValue=0;
	unsigned int GetValue=0;
	unsigned short uShort;
	int len = 0;

	
	int isWLANMIBUpdated=1;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	int wlanBand2G5GSelect=0;
	int MaxWlanIface=NUM_WLAN_INTERFACE;

	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wlaninst = getInstanceNum(name, "AccessPoint");

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if (wlaninst > 5) {
		wlaninst1 = wlaninst - 5;
		_getWLANIdxFromInstNum(wlaninst1, &rootIdx, &vwlanIdx);
	}
	else {
		_getWLANIdxFromInstNum(wlaninst, &rootIdx, &vwlanIdx);
	}

	if (wlaninst > 5)	rootIdx = 1;


	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		//pcwmpWlan->func_off=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WSC_DISABLE, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
#if 0
	else if( strcmp( lastname, "ConfigMethodsEnabled" )==0 )
	{
		;
	}
#endif
	else{
		return ERR_9005;
	}

	if(isWLANMIBUpdated)
		return 1;
	else
#endif
		return 0;
}

#if 0 // rewrite
//changes in following table should be synced to VHT_MCS_DATA_RATE[] in 8812_vht_gen.c
const unsigned short VHT_MCS_DATA_RATE[3][2][20] = 
	{	{	{13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
			 26, 52, 78, 104, 156, 208, 234, 260, 312, 312},			// Long GI, 20MHz
			{14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
			29, 58, 87, 116, 173, 231, 260, 289, 347, 347}	},		// Short GI, 20MHz
		{	{27, 54, 81, 108, 162, 216, 243, 270, 324, 360, 
			54, 108, 162, 216, 324, 432, 486, 540, 648, 720}, 		// Long GI, 40MHz
			{30, 60, 90, 120, 180, 240, 270, 300,360, 400, 
			60, 120, 180, 240, 360, 480, 540, 600, 720, 800}},		// Short GI, 40MHz
		{	{59, 117,  176, 234, 351, 468, 527, 585, 702, 780,
			117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560}, 	// Long GI, 80MHz
			{65, 130, 195, 260, 390, 520, 585, 650, 780, 867, 
			130, 260, 390, 520, 780, 1040, 1170, 1300, 1560,1733}	}	// Short GI, 80MHz
	};

//changes in following table should be synced to MCS_DATA_RATEStr[] in 8190n_proc.c
WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.4"},
	{MCS9, 	"28.9"},
	{MCS10, 	"43.3"},
	{MCS11, 	"57.8"},
	{MCS12, 	"86.7"},
	{MCS13, 	"115.6"},
	{MCS14, 	"130"},
	{MCS15, 	"144.5"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};

void set_11ac_txrate(WLAN_STA_INFO_Tp pInfo,char* txrate)
{
	char channelWidth=0;//20M 0,40M 1,80M 2
	char shortGi=0;
	char rate_idx=pInfo->txOperaRates-0x90;
	if(!txrate)return;
/*
	TX_USE_40M_MODE		= BIT(0),
	TX_USE_SHORT_GI		= BIT(1),
	TX_USE_80M_MODE		= BIT(2)
*/
	if(pInfo->ht_info & 0x4)
		channelWidth=2;
	else if(pInfo->ht_info & 0x1)
		channelWidth=1;
	else
		channelWidth=0;
	if(pInfo->ht_info & 0x2)
		shortGi=1;

	sprintf(txrate, "%d", VHT_MCS_DATA_RATE[channelWidth][shortGi][rate_idx]>>1);
}

int tranRate(WLAN_STA_INFO_T *pInfo, unsigned char rate)
{
	char txrate[20];
	int rateid=0;
	
	if(rate >= 0x90) {
		//sprintf(txrate, "%d", pInfo->acTxOperaRate); 
		set_11ac_txrate(pInfo, txrate);
	} else if((rate & 0x80) != 0x80){	
		if(rate%2){
			sprintf(txrate, "%d%s",rate/2, ".5"); 
		}else{
			sprintf(txrate, "%d",rate/2); 
		}
	}else{
		if((pInfo->ht_info & 0x1)==0){ //20M
			if((pInfo->ht_info & 0x2)==0){//long
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_20M_LONG[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
						break;
					}
				}
			}else if((pInfo->ht_info & 0x2)==0x2){//short
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_20M_SHORT[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
						break;
					}
				}
			}
		}else if((pInfo->ht_info & 0x1)==0x1){//40M
			if((pInfo->ht_info & 0x2)==0){//long
				
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_40M_LONG[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
						break;
					}
				}
			}else if((pInfo->ht_info & 0x2)==0x2){//short
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_40M_SHORT[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
						break;
					}
				}
			}
		}
		
	}

	//tr181_printf("txrate %s", txrate);
	return atoi(txrate);
}
#endif

extern int getWLANSTAINFO(int id, WLAN_STA_INFO_T *info);
int getWifiAPAssocats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
#if 0 // rewrite
	char		*lastname = entity->info->name;
	unsigned int	device_id=0;
	WLAN_STA_INFO_T info;
	char		*tok=NULL;
	unsigned int wlaninst=0;

	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getInstanceNum(name, "AccessPoint");

	//tr181_printf("wlaninst %d", wlaninst);

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if( loadWLANAssInfoByInstNum(wlaninst)<0 ) return ERR_9002;
	
	device_id = getInstanceNum( name, "AssociatedDevice" );

	//tr181_printf("device_id %d gWLANTotalClients %d", device_id, gWLANTotalClients);
	
	if( device_id<1 || device_id>gWLANTotalClients ) return ERR_9005;
	
	if( getWLANSTAINFO( device_id-1, &info )<0 ) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "MACAddress" )==0 )
	{
		char buf[32];
		sprintf( buf, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "AuthenticationState" )==0 )
	{
		int i = ((info.flag & STA_INFO_FLAG_ASOC)==STA_INFO_FLAG_ASOC);
		*data = intdup( i );
	}
	else if( strcmp( lastname, "LastDataDownlinkRate" )==0 )
	{
		*data = uintdup(tranRate(&info, info.txOperaRates));
	}
	else if( strcmp( lastname, "LastDataUplinkRate" )==0 )
	{
		*data = uintdup(tranRate(&info, info.RxOperaRate));
	}
	else if( strcmp( lastname, "SignalStrength" )==0 )
	{
		//tr181_printf("info.rssi %d", info.rssi);
		*data = intdup(info.rssi - 100);
	}
	else{
		return ERR_9005;
	}
#endif
	return 0;
}