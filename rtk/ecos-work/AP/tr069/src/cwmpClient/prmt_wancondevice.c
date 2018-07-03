// remove by cairui
//#include <stdint.h>
#include <signal.h>
//#include <linux/atm.h>
//#include <linux/atmdev.h>
//add by cairui
#define IFNAMSIZ 16

//by cairui temp
struct CWMP_LEAF OldPort_Entity;
struct CWMP_LEAF vtlsvr_entryx;

#include <sys/time.h>
//#include <sys/sysinfo.h>
#include "prmt_wancondevice.h"
#include "prmt_wanatmf5loopback.h"

//#include "mibtbl.h" //keith add.
#include "prmt_utility.h" //keith add.
#include <sys/stat.h> //keith add.
//#include <linux/wireless.h>
		extern char WLAN_IF[];
		extern char WAN_IF[];
#if defined(_PRMT_X_CT_COM_DDNS_)
#include "prmt_ddns.h"
#endif
#ifdef WLAN_SUPPORT
#include "prmt_landevice_wlan.h"
#endif

/*old: all the same rip version, new: different rip version for different WAN*/
#define _USE_NEW_RIP_

extern int getInterfaceStat(
	char *ifname,
	unsigned long *bs,
	unsigned long *br,
	unsigned long *ps,
	unsigned long *pr );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
extern int getInterfaceStat1(
	char *ifname,
	unsigned long *es, unsigned long *er,
	unsigned long *ups, unsigned long *upr, 
	unsigned long *dps, unsigned long *dpr, 
	unsigned long *mps, unsigned long *mpr, 
	unsigned long *bps, unsigned long *bpr, 
	unsigned long *uppr);
#endif
/*ping_zhang:20081217 END*/
extern int getChainID( struct CWMP_LINKNODE *ctable, int num );

//int getObjectID( struct sCWMP_ENTITY *ctable, int chainid );
unsigned char getNewIfIndex(int cmode);
unsigned int getWANConDevInstNum( char *name );
unsigned int getWANPPPConInstNum( char *name );
unsigned int getWANIPConInstNum( char *name );
unsigned int getWANPortMapInstNum( char *name );
unsigned int getInstNum( char *name, char *objname );

/*****port mapping api****/
unsigned int getPortMappingMaxInstNum(void);
int getPortMappingCount(int *portEntityCount);
#ifdef VIRTUAL_SERVER_SUPPORT
int getPortMappingByID( unsigned char ifindex, int id, vtlsvr_entryx *c, unsigned int *chainID );
int getPortMappingByInstNum( unsigned char ifindex, unsigned int instnum, vtlsvr_entryx *c, unsigned int *chainID );
#else
int getPortMappingByID( unsigned char ifindex, int id, PORTFW_T *c, unsigned int *chainID );
int getPortMappingByInstNum( unsigned char ifindex, unsigned int instnum, PORTFW_T *c, unsigned int *chainID );
#endif
/*****endi port mapping api****/

//int getAAL5CRCErrors( unsigned char vpi, unsigned short vci, unsigned int *count );
//int getATMCellCnt( unsigned char vpi, unsigned short vci, unsigned int *txcnt, unsigned int *rxcnt );
#ifdef _USE_NEW_RIP_
//int getRIPInfo( unsigned char ifIndex, unsigned char *ripmode );
//int updateRIPInfo( unsigned char ifIndex, unsigned char newripmode );
#else
//int updateRIPInfo( int chainid, unsigned char newripmode );
#endif

/*ppp utilities*/
int getPPPConStatus( char *pppname, char *status );
//int getPPPUptime( char *pppname, int ppptype, unsigned int *uptime );
//int getPPPCurrentMRU( char *pppname, int ppptype, unsigned int *cmru );
//int getPPPLCPEcho( char *pppname, int ppptype, unsigned int *echo );
int getPPPEchoRetry( char *pppname, int ppptype, unsigned int *retry );
//const char *getLastConnectionError(unsigned char ifindex); keith remove	

#if 0 //Keith remove. no support DSL
int getATMVCEntryByIPInstNum( unsigned int devnum, unsigned int ipnum, MIB_CE_ATM_VC_T *p, unsigned int *id );
int getATMVCEntryByPPPInstNum( unsigned int devnum, unsigned int pppnum, MIB_CE_ATM_VC_T *p, unsigned int *id );
int resetATMVCConnection( MIB_CE_ATM_VC_T *p );
#endif //#if 0 //Keith remove. no support DSL

#ifdef _PRMT_X_CT_COM_WANEXT_
int convertFlag2ServiceList( unsigned short flag, char *buf );
int convertServiceList2Flag( char *buf, unsigned short *flag );
#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
int getLanInterface( unsigned char itfGroup, char *buf );
int setLanInterface( char *buf, MIB_CE_ATM_VC_T *pEntry );
#endif //defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
#endif //_PRMT_X_CT_COM_WANEXT_

#if 1//defined(CONFIG_ETHWAN)
int getWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif
//char gSharedWanName[256+1]="";


struct CWMP_OP tWANCONSTATSLeafOP = { getWANCONSTATS, NULL };
struct CWMP_PRMT tWANCONSTATSLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"EthernetBytesSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetBytesReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"EthernetErrorsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetErrorsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetUnicastPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetUnicastPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetDiscardPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetDiscardPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetMulticastPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetMulticastPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetBroadcastPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetBroadcastPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP},
{"EthernetUnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANCONSTATSLeafOP}
#endif
/*ping_zhang:20081217 END*/
};
enum eWANCONSTATSLeaf
{
	eEthernetBytesSent,
	eEthernetBytesReceived,
	eEthernetPacketsSent,
	eEthernetPacketsReceived,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eEthernetErrorsSent,
	eEthernetErrorsReceived,
	eEthernetUnicastPacketsSent,
	eEthernetUnicastPacketsReceived,
	eEthernetDiscardPacketsSent,
	eEthernetDiscardPacketsReceived,
	eEthernetMulticastPacketsSent,
	eEthernetMulticastPacketsReceived,
	eEthernetBroadcastPacketsSent,
	eEthernetBroadcastPacketsReceived,
	eEthernetUnknownProtoPacketsReceived
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tWANCONSTATSLeaf[] =
{
{ &tWANCONSTATSLeafInfo[eEthernetBytesSent] },
{ &tWANCONSTATSLeafInfo[eEthernetBytesReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetPacketsReceived] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANCONSTATSLeafInfo[eEthernetErrorsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetErrorsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetUnicastPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetUnicastPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetDiscardPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetDiscardPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetMulticastPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetMulticastPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetBroadcastPacketsSent] },
{ &tWANCONSTATSLeafInfo[eEthernetBroadcastPacketsReceived] },
{ &tWANCONSTATSLeafInfo[eEthernetUnknownProtoPacketsReceived] },
#endif
/*ping_zhang:20081217 END*/
{ NULL }
};

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.DHCPClient.SentDHCPOption.{i}.*/
struct CWMP_OP tDHCPClientSentENTITYLeafOP = { getDHCPClientOptENTITY, setDHCPClientOptENTITY };
struct CWMP_PRMT tDHCPCLientSentENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDHCPClientSentENTITYLeafOP},
{"Tag",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tDHCPClientSentENTITYLeafOP},
{"Value",			eCWMP_tBASE64,	CWMP_WRITE|CWMP_READ,	&tDHCPClientSentENTITYLeafOP}
};
enum eDHCPCLientSentENTITYLeaf
{
	eSentEnable,
	eSentTag,
	eSentValue
};
struct CWMP_LEAF tDHCPCLientSentENTITYLeaf[] =
{
{ &tDHCPCLientSentENTITYLeafInfo[eSentEnable] },
{ &tDHCPCLientSentENTITYLeafInfo[eSentTag] },
{ &tDHCPCLientSentENTITYLeafInfo[eSentValue] },
{ NULL }
};

struct CWMP_PRMT tDHCPClientSentObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eDHCPClientSentObjectInfo
{
	eDHCPCLientSent0
};
struct CWMP_LINKNODE tDHCPClientSentObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPClientSentObjectInfo[eDHCPCLientSent0],	tDHCPCLientSentENTITYLeaf,	NULL,		NULL,			0},
};

/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.{i}.DHCPClient.ReqDHCPOption.{i}.*/
struct CWMP_OP tDHCPClientReqENTITYLeafOP = { getDHCPClientOptENTITY, setDHCPClientOptENTITY };
struct CWMP_PRMT tDHCPCLientReqENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDHCPClientReqENTITYLeafOP},
{"Order",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tDHCPClientReqENTITYLeafOP},
{"Tag",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tDHCPClientReqENTITYLeafOP},
{"Value",			eCWMP_tBASE64,	CWMP_READ,				&tDHCPClientReqENTITYLeafOP}
};
enum eDHCPCLientReqENTITYLeaf
{
	eReqEnable,
	eReqOrder,
	eReqTag,
	eReqValue
};
struct CWMP_LEAF tDHCPCLientReqENTITYLeaf[] =
{
{ &tDHCPCLientReqENTITYLeafInfo[eReqEnable] },
{ &tDHCPCLientReqENTITYLeafInfo[eReqOrder] },
{ &tDHCPCLientReqENTITYLeafInfo[eReqTag] },
{ &tDHCPCLientReqENTITYLeafInfo[eReqValue] },
{ NULL }
};

struct CWMP_PRMT tDHCPClientReqObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eDHCPClientReqObjectInfo
{
	eDHCPCLientReq0
};
struct CWMP_LINKNODE tDHCPClientReqObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPClientReqObjectInfo[eDHCPCLientReq0],	tDHCPCLientReqENTITYLeaf,	NULL,		NULL,			0},
};
#endif
/*ping_zhang:20080919 END*/

struct CWMP_OP tPORTMAPENTITYLeafOP = { getPORMAPTENTITY, setPORMAPTENTITY };
struct CWMP_PRMT tPORTMAPENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PortMappingEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"PortMappingLeaseDuration",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ|CWMP_DENY_ACT,&tPORTMAPENTITYLeafOP}, 
{"RemoteHost",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP}, 
{"ExternalPort",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"InternalPort",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"PortMappingProtocol",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"InternalClient",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP},
{"PortMappingDescription",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPORTMAPENTITYLeafOP}
};
enum ePORTMAPENTITYLeaf
{
	ePortMappingEnabled,
	ePortMappingLeaseDuration,
	eRemoteHost,
	eExternalPort,
	eInternalPort,
	ePortMappingProtocol,
	eInternalClient,
	ePortMappingDescription
};
struct CWMP_LEAF tPORTMAPENTITYLeaf[] =
{
{ &tPORTMAPENTITYLeafInfo[ePortMappingEnabled] },
{ &tPORTMAPENTITYLeafInfo[ePortMappingLeaseDuration] },
{ &tPORTMAPENTITYLeafInfo[eRemoteHost] },
{ &tPORTMAPENTITYLeafInfo[eExternalPort] },
{ &tPORTMAPENTITYLeafInfo[eInternalPort] },
{ &tPORTMAPENTITYLeafInfo[ePortMappingProtocol] },
{ &tPORTMAPENTITYLeafInfo[eInternalClient] },
{ &tPORTMAPENTITYLeafInfo[ePortMappingDescription] },
{ NULL }
};


struct CWMP_PRMT tWANPORTMAPObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANPORTMAPObject
{
	eWANPORTMAP0
};
struct CWMP_LINKNODE tWANPORTMAPObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tWANPORTMAPObjectInfo[eWANPORTMAP0],	tPORTMAPENTITYLeaf,	NULL,		NULL,			0},
};


struct CWMP_OP tWANPPPCONENTITYLeafOP = { getWANPPPCONENTITY, setWANPPPCONENTITY };
struct CWMP_PRMT tWANPPPCONENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Reset",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"ConnectionStatus",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PossibleConnectionTypes",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"ConnectionType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"PPPoESessionID",		eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"DefaultGateway",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"Name",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"Uptime",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANPPPCONENTITYLeafOP},
{"LastConnectionError",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP}, 
{"AutoDisconnectTime",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP}, 
{"IdleDisconnectTime",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"WarnDisconnectDelay",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP}, 
{"RSIPAvailable",		eCWMP_tBOOLEAN,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"NATEnabled",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"Username",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"Password",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"PPPEncryptionProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PPPCompressionProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PPPAuthenticationProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP}, 
{"ExternalIPAddress",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"RemoteIPAddress",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP}, 	
{"MaxMRUSize",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP}, 	
{"CurrentMRUSize",		eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP}, 

{"DNSEnabled",			eCWMP_tBOOLEAN,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"DNSOverrideAllowed",		eCWMP_tBOOLEAN,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"DNSServers",			eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},

{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
/*MACAddressOverride*/
{"TransportType",		eCWMP_tSTRING,	CWMP_READ,		&tWANPPPCONENTITYLeafOP},
{"PPPoEACName",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP}, 
{"PPPoEServiceName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"ConnectionTrigger",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"RouteProtocolRx",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"PPPLCPEcho",			eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP}, 
{"PPPLCPEchoRetry",		eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP}, 
#ifdef _PRMT_X_CT_COM_PPPOEv2_
{"X_CT-COM_ProxyEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"X_CT-COM_MAXUser",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
{"X_CT-COM_LanInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"X_CT-COM_ServiceList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPMode", eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"X_CT-COM_IPv6ConnectionStatus",	eCWMP_tSTRING,	CWMP_READ,	&tWANPPPCONENTITYLeafOP},
{"X_CT-COM_LanInterface-DHCPEnable",	eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
#endif //_PRMT_X_CT_COM_WANEXT_
#if defined(CONFIG_ETHWAN)
{"X_CT-COM_MulticastVlan", eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tWANPPPCONENTITYLeafOP},
#endif
{"PortMappingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tWANPPPCONENTITYLeafOP}
};
enum eWANPPPCONENTITYLeaf
{
	ePPP_Enable,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	ePPP_Reset,
#endif
/*ping_zhang:20081217 END*/
	ePPP_ConnectionStatus,
	ePPP_PossibleConnectionTypes,
	ePPP_ConnectionType,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	ePPP_PPPoESessionID,
	ePPP_DefaultGateway,
#endif
/*ping_zhang:20081217 END*/
	ePPP_Name,
	ePPP_Uptime,
	ePPP_LastConnectionError,
	ePPP_AutoDisconnectTime,
	ePPP_IdleDisconnectTime,
	ePPP_WarnDisconnectDelay, 
	ePPP_RSIPAvailable,
	ePPP_NATEnabled,
	ePPP_Username,
	ePPP_Password,
	ePPP_PPPEncryptionProtocol,
	ePPP_PPPCompressionProtocol,
	ePPP_PPPAuthenticationProtocol, 
	ePPP_ExternalIPAddress,
	ePPP_RemoteIPAddress, 
	ePPP_MaxMRUSize, 	
	ePPP_CurrentMRUSize, 
	ePPP_DNSEnabled,
	ePPP_DNSOverrideAllowed,
	ePPP_DNSServers,
	ePPP_MACAddress,
	/*MACAddressOverride*/
	ePPP_TransportType,
	ePPP_PPPoEACName, 
	ePPP_PPPoEServiceName,
	ePPP_ConnectionTrigger,
	ePPP_RouteProtocolRx,
	ePPP_PPPLCPEcho, 
	ePPP_PPPLCPEchoRetry, 
#ifdef _PRMT_X_CT_COM_PPPOEv2_
	ePPP_X_CTCOM_ProxyEnable,
	ePPP_X_CTCOM_MAXUser,
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
	ePPP_X_CTCOM_LanInterface,
	ePPP_X_CTCOM_ServiceList,
	ePPP_X_CTCOM_IPMode,
	ePPP_X_CTCOM_IPv6ConnectionStatus,
	ePPP_X_CTCOM_LanInterfaceDHCPEnable,
#endif //_PRMT_X_CT_COM_WANEXT_
#if defined(CONFIG_ETHWAN)
	ePPP_X_CTCOM_MulticastVlan,
#endif

	ePPP_PortMappingNumberOfEntries
};
struct CWMP_LEAF tWANPPPCONENTITYLeaf[] =
{
{ &tWANPPPCONENTITYLeafInfo[ePPP_Enable] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANPPPCONENTITYLeafInfo[ePPP_Reset] },
#endif
/*ping_zhang:20081217 END*/
{ &tWANPPPCONENTITYLeafInfo[ePPP_ConnectionStatus] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PossibleConnectionTypes] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_ConnectionType] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPoESessionID] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_DefaultGateway] },
#endif
/*ping_zhang:20081217 END*/
{ &tWANPPPCONENTITYLeafInfo[ePPP_Name] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_Uptime] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_LastConnectionError] }, 
{ &tWANPPPCONENTITYLeafInfo[ePPP_AutoDisconnectTime] }, 
{ &tWANPPPCONENTITYLeafInfo[ePPP_IdleDisconnectTime] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_WarnDisconnectDelay] }, 
{ &tWANPPPCONENTITYLeafInfo[ePPP_RSIPAvailable] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_NATEnabled] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_Username] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_Password] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPEncryptionProtocol] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPCompressionProtocol] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPAuthenticationProtocol] }, 
{ &tWANPPPCONENTITYLeafInfo[ePPP_ExternalIPAddress] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_RemoteIPAddress] }, 	
{ &tWANPPPCONENTITYLeafInfo[ePPP_MaxMRUSize] }, 	
{ &tWANPPPCONENTITYLeafInfo[ePPP_CurrentMRUSize] }, 
{ &tWANPPPCONENTITYLeafInfo[ePPP_DNSEnabled] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_DNSOverrideAllowed] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_DNSServers] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_MACAddress] },
/*MACAddressOverride*/
{ &tWANPPPCONENTITYLeafInfo[ePPP_TransportType] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPoEACName] }, 
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPoEServiceName] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_ConnectionTrigger] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_RouteProtocolRx] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPLCPEcho] }, 
{ &tWANPPPCONENTITYLeafInfo[ePPP_PPPLCPEchoRetry] }, 
#ifdef _PRMT_X_CT_COM_PPPOEv2_
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_ProxyEnable] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_MAXUser] },
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_LanInterface] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_ServiceList] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPMode] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_IPv6ConnectionStatus] },
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_LanInterfaceDHCPEnable] },
#endif //_PRMT_X_CT_COM_WANEXT_
#if defined(CONFIG_ETHWAN)
{ &tWANPPPCONENTITYLeafInfo[ePPP_X_CTCOM_MulticastVlan] },
#endif
{ &tWANPPPCONENTITYLeafInfo[ePPP_PortMappingNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tWAN_PortMapping_OP = { NULL, objWANPORTMAPPING };
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
struct CWMP_OP tWAN_DDNSConf_OP = { NULL, objDDNS };
#endif

struct CWMP_PRMT tWANPPPCONENTITYObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PortMapping",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_PortMapping_OP},
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{"X_CT-COM_DDNSConfiguration",	eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_DDNSConf_OP},
#endif
};
enum eWANPPPCONENTITYObject
{
	ePPP_PortMapping,
	ePPP_Stats,
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
	ePPP_X_CTCOM_DDNSConfiguration
#endif
};
struct CWMP_NODE tWANPPPCONENTITYObject[] =
{
/*info,  							leaf,			node)*/
{&tWANPPPCONENTITYObjectInfo[ePPP_PortMapping],			NULL,			NULL},
{&tWANPPPCONENTITYObjectInfo[ePPP_Stats],			tWANCONSTATSLeaf,	NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{&tWANPPPCONENTITYObjectInfo[ePPP_X_CTCOM_DDNSConfiguration],	NULL,			NULL},
#endif
{NULL,								NULL,			NULL}
};


struct CWMP_PRMT tWANPPPCONObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANPPPCONObject
{
	WANPPPCON0
};
struct CWMP_LINKNODE tWANPPPCONObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tWANPPPCONObjectInfo[WANPPPCON0],	tWANPPPCONENTITYLeaf,	tWANPPPCONENTITYObject,		NULL,			0}
};


/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
struct CWMP_OP tWANIPConDHCPClientLeafOP = { getWANIPConDHCPClientENTITY, NULL };
struct CWMP_PRMT tWANIPConDHCPClientLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"SentDHCPOptionNumberOfEntries",		eCWMP_tUINT,		CWMP_READ,	&tWANIPConDHCPClientLeafOP},
{"ReqDHCPOptionNumberOfEntries",		eCWMP_tUINT,		CWMP_READ,	&tWANIPConDHCPClientLeafOP}
};
enum eWANIPConDHCPClientLeaf
{
	eSentDHCPOptionNumberOfEntries,
	eReqDHCPOptionNumberOfEntries
};
struct CWMP_LEAF tWANIPConDHCPClientLeaf[] =
{
{ &tWANIPConDHCPClientLeafInfo[eSentDHCPOptionNumberOfEntries] },
{ &tWANIPConDHCPClientLeafInfo[eReqDHCPOptionNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tDHCPClientSentOpt_OP = { NULL, objDHCPClientSentOpt};
struct CWMP_OP tDHCPClientReqOpt_OP = { NULL, objDHCPClientReqOpt};
struct CWMP_PRMT tWANIPConDHCPClientObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"SentDHCPOption",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,		&tDHCPClientSentOpt_OP},
{"ReqDHCPOption",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,		&tDHCPClientReqOpt_OP},
};
enum eWANIPConDHCPClientObjectInfo
{
	eSentDHCPOption,
	eReqDHCPOption
};
struct CWMP_NODE tWANIPConDHCPClientObject[] =
{
/*info,  							leaf,			node)*/
{&tWANIPConDHCPClientObjectInfo[eSentDHCPOption],	NULL,	NULL},
{&tWANIPConDHCPClientObjectInfo[eReqDHCPOption],	NULL,	NULL},
{NULL,								NULL,			NULL}
};
#endif
/*ping_zhang:20080919 END*/

struct CWMP_OP tWANIPCONENTITYLeafOP = { getWANIPCONENTITY, setWANIPCONENTITY };
struct CWMP_PRMT tWANIPCONENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Reset",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"ConnectionStatus",		eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"PossibleConnectionTypes",	eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"ConnectionType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"Name",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP}, 
{"Uptime",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWANIPCONENTITYLeafOP},
{"LastConnectionError",		eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP}, 
/*AutoDisconnectTime*/
/*IdleDisconnectTime*/
/*WarnDisconnectDelay*/
{"RSIPAvailable",		eCWMP_tBOOLEAN,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"NATEnabled",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"AddressingType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"ExternalIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"SubnetMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"DefaultGateway",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},

{"DNSEnabled",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"DNSOverrideAllowed",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tWANIPCONENTITYLeafOP},
{"DNSServers",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tWANIPCONENTITYLeafOP},

{"MaxMTUSize",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tWANIPCONENTITYLeafOP},
/*MACAddressOverride*/
{"ConnectionTrigger",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"RouteProtocolRx",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
#ifdef _PRMT_X_CT_COM_WANEXT_
{"X_CT-COM_LanInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"X_CT-COM_ServiceList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"X_CT-COM_IPMode",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"X_CT-COM_IPv6ConnectionStatus",	eCWMP_tSTRING,	CWMP_READ,	&tWANIPCONENTITYLeafOP},
{"X_CT-COM_LanInterface-DHCPEnable",	eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
#endif //_PRMT_X_CT_COM_WANEXT_
#if defined(CONFIG_ETHWAN)
{"X_CT-COM_MulticastVlan",	eCWMP_tINT, CWMP_WRITE|CWMP_READ,	&tWANIPCONENTITYLeafOP},
#endif

{"PortMappingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tWANIPCONENTITYLeafOP}
};
enum eWANIPCONENTITYLeaf
{
	eIP_Enable,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eIP_Reset,
#endif
/*ping_zhang:20081217 END*/
	eIP_ConnectionStatus,
	eIP_PossibleConnectionTypes,
	eIP_ConnectionType,
	eIP_Name, 
	eIP_Uptime,
	eIP_LastConnectionError,
	/*AutoDisconnectTime*/
	/*IdleDisconnectTime*/
	/*WarnDisconnectDelay*/
	eIP_RSIPAvailable,
	eIP_NATEnabled,
	eIP_AddressingType,
	eIP_ExternalIPAddress,
	eIP_SubnetMask,
	eIP_DefaultGateway,
	eIP_DNSEnabled,
	eIP_DNSOverrideAllowed,
	eIP_DNSServers,
	eIP_MaxMTUSize,
	eIP_MACAddress,
	/*MACAddressOverride*/
	eIP_ConnectionTrigger,
	eIP_RouteProtocolRx,
#ifdef _PRMT_X_CT_COM_WANEXT_
	eIP_X_CTCOM_LanInterface,
	eIP_X_CTCOM_ServiceList,
	eIP_X_CTCOM_IP_Mode,
	eIP_X_CTCOM_IP_IPv6ConnectionStatus,
	eIP_X_CTCOM_LanInterfaceDHCPEnable,
#endif //_PRMT_X_CT_COM_WANEXT_
#if defined(CONFIG_ETHWAN)
	eIP_X_CTCOM_MulticastVlan,
#endif

	eIP_PortMappingNumberOfEntries
};
struct CWMP_LEAF tWANIPCONENTITYLeaf[] =
{
{ &tWANIPCONENTITYLeafInfo[eIP_Enable] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tWANIPCONENTITYLeafInfo[eIP_Reset] },
#endif
/*ping_zhang:20081217 END*/
{ &tWANIPCONENTITYLeafInfo[eIP_ConnectionStatus] },
{ &tWANIPCONENTITYLeafInfo[eIP_PossibleConnectionTypes] },
{ &tWANIPCONENTITYLeafInfo[eIP_ConnectionType] },
{ &tWANIPCONENTITYLeafInfo[eIP_Name] }, 
{ &tWANIPCONENTITYLeafInfo[eIP_Uptime] },
{ &tWANIPCONENTITYLeafInfo[eIP_LastConnectionError] }, 
/*AutoDisconnectTime*/
/*IdleDisconnectTime*/
/*WarnDisconnectDelay*/
{ &tWANIPCONENTITYLeafInfo[eIP_RSIPAvailable] },
{ &tWANIPCONENTITYLeafInfo[eIP_NATEnabled] },
{ &tWANIPCONENTITYLeafInfo[eIP_AddressingType] },
{ &tWANIPCONENTITYLeafInfo[eIP_ExternalIPAddress] },
{ &tWANIPCONENTITYLeafInfo[eIP_SubnetMask] },
{ &tWANIPCONENTITYLeafInfo[eIP_DefaultGateway] },
{ &tWANIPCONENTITYLeafInfo[eIP_DNSEnabled] },
{ &tWANIPCONENTITYLeafInfo[eIP_DNSOverrideAllowed] },
{ &tWANIPCONENTITYLeafInfo[eIP_DNSServers] },
{ &tWANIPCONENTITYLeafInfo[eIP_MaxMTUSize] },
{ &tWANIPCONENTITYLeafInfo[eIP_MACAddress] },
/*MACAddressOverride*/
{ &tWANIPCONENTITYLeafInfo[eIP_ConnectionTrigger] },
{ &tWANIPCONENTITYLeafInfo[eIP_RouteProtocolRx] },
#ifdef _PRMT_X_CT_COM_WANEXT_
{ &tWANIPCONENTITYLeafInfo[eIP_X_CTCOM_LanInterface] },
{ &tWANIPCONENTITYLeafInfo[eIP_X_CTCOM_ServiceList] },
{ &tWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IP_Mode] },
{ &tWANIPCONENTITYLeafInfo[eIP_X_CTCOM_IP_IPv6ConnectionStatus] },
{ &tWANIPCONENTITYLeafInfo[eIP_X_CTCOM_LanInterfaceDHCPEnable] },
#endif //_PRMT_X_CT_COM_WANEXT_
#if defined(CONFIG_ETHWAN)
{ &tWANIPCONENTITYLeafInfo[eIP_X_CTCOM_MulticastVlan] },
#endif
{ &tWANIPCONENTITYLeafInfo[eIP_PortMappingNumberOfEntries] },
{ NULL }
};

struct CWMP_PRMT tWANIPCONENTITYObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{"DHCPClient",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
/*ping_zhang:20080919 END*/
{"PortMapping",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_PortMapping_OP},
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{"X_CT-COM_DDNSConfiguration",	eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_DDNSConf_OP}
#endif
};
enum eWANIPCONENTITYObject
{
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	eIP_DHCPClient,
#endif
/*ping_zhang:20080919 END*/
	eIP_PortMapping,
	eIP_Stats,
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
	eIP_X_CTCOM_DDNSConfiguration
#endif
};
struct CWMP_NODE tWANIPCONENTITYObject[] =
{
/*info,  							leaf,			node)*/
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{&tWANIPCONENTITYObjectInfo[eIP_DHCPClient],			tWANIPConDHCPClientLeaf,			tWANIPConDHCPClientObject},
#endif
/*ping_zhang:20080919 END*/
{&tWANIPCONENTITYObjectInfo[eIP_PortMapping],			NULL,			NULL},
{&tWANIPCONENTITYObjectInfo[eIP_Stats],				tWANCONSTATSLeaf,	NULL},
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)
{&tWANIPCONENTITYObjectInfo[eIP_X_CTCOM_DDNSConfiguration],	NULL,			NULL},
#endif
{NULL,								NULL,			NULL}
};


struct CWMP_PRMT tWANIPCONObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANIPCONObject
{
	eWANIPCON0
};
struct CWMP_LINKNODE tWANIPCONObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tWANIPCONObjectInfo[eWANIPCON0],	tWANIPCONENTITYLeaf,	tWANIPCONENTITYObject,		NULL,			0}
};


#ifdef CONFIG_DEV_xDSL
struct CWMP_OP tDSLLNKCONFLeafOP = { getDSLLNKCONF, setDSLLNKCONF };
struct CWMP_PRMT tDSLLNKCONFLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"LinkStatus",			eCWMP_tSTRING,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"LinkType",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"AutoConfig",			eCWMP_tBOOLEAN,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"ModulationType",		eCWMP_tSTRING,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"DestinationAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMEncapsulation",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
/*FCSPreserved*/
/*VCSearchList*/
{"ATMAAL",			eCWMP_tSTRING,	CWMP_READ,		&tDSLLNKCONFLeafOP},
{"ATMTransmittedBlocks",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP},
{"ATMReceivedBlocks",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP},
{"ATMQoS",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMPeakCellRate",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMMaximumBurstSize",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"ATMSustainableCellRate",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP},
{"AAL5CRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP},
{"ATMCRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLLNKCONFLeafOP}
/*ATMHECErrors*/
#ifdef _PRMT_X_TELEFONICA_ES_IGMPCONFIG_
{"X_TELEFONICA-ES_IGMPEnabled",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDSLLNKCONFLeafOP}, //Spec v10, correction of the parameter's name
#endif //_PRMT_X_TELEFONICA_ES_IGMPCONFIG_
};
#endif //#if 0 //keith remove. no support DSL

#if 0 //keith remove. no support DSL
enum eDSLLNKCONFLeaf
{
	eDL_Enable,
	eDL_LinkStatus,
	eDL_LinkType,
	eDL_AutoConfig,
	eDL_ModulationType,
	eDL_DestinationAddress,
	eDL_ATMEncapsulation,
	eDL_ATMAAL,
	eDL_ATMTransmittedBlocks,
	eDL_ATMReceivedBlocks,
	eDL_ATMQoS,
	eDL_ATMPeakCellRate,
	eDL_ATMMaximumBurstSize,
	eDL_ATMSustainableCellRate,
	eDL_AAL5CRCErrors,
	eDL_ATMCRCErrors,
#ifdef _PRMT_X_TELEFONICA_ES_IGMPCONFIG_
	eX_TELEFONICA_IGMPEnabled,
#endif //_PRMT_X_TELEFONICA_ES_IGMPCONFIG_
};
struct CWMP_LEAF tDSLLNKCONFLeaf[] =
{
{ &tDSLLNKCONFLeafInfo[eDL_Enable] },
{ &tDSLLNKCONFLeafInfo[eDL_LinkStatus] },
{ &tDSLLNKCONFLeafInfo[eDL_LinkType] },
{ &tDSLLNKCONFLeafInfo[eDL_AutoConfig] },
{ &tDSLLNKCONFLeafInfo[eDL_ModulationType] },
{ &tDSLLNKCONFLeafInfo[eDL_DestinationAddress] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMEncapsulation] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMAAL] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMTransmittedBlocks] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMReceivedBlocks] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMQoS] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMPeakCellRate] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMMaximumBurstSize] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMSustainableCellRate] },
{ &tDSLLNKCONFLeafInfo[eDL_AAL5CRCErrors] },
{ &tDSLLNKCONFLeafInfo[eDL_ATMCRCErrors] },
#ifdef _PRMT_X_TELEFONICA_ES_IGMPCONFIG_
{ &tDSLLNKCONFLeafInfo[eX_TELEFONICA_IGMPEnabled] },
#endif //_PRMT_X_TELEFONICA_ES_IGMPCONFIG_
{ NULL }
};
#endif //#if 0 //keith remove. no support DSL
#if 1//defined CONFIG_ETHWAN // #ifdef CONFIG_DEV_xDSL

struct CWMP_OP tWANETHLINKFLeafOP = { getWANETHLINKCONF, setWANETHLINKCONF };
struct CWMP_PRMT tWANETHLINKLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
#ifdef _PRMT_X_CT_COM_ETHLINK_
	{"X_CT-COM_Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tWANETHLINKFLeafOP},
	{"X_CT-COM_Mode",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tWANETHLINKFLeafOP},
{"X_CT-COM_VLANIDMark",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWANETHLINKFLeafOP},
{"X_CT-COM_802-1pMark",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tWANETHLINKFLeafOP},
#endif
	{"EthernetLinkStatus",			eCWMP_tSTRING,	CWMP_READ,	&tWANETHLINKFLeafOP}
};
enum eWANETHLINKLeaf
{
#ifdef _PRMT_X_CT_COM_ETHLINK_
	eX_CTCOM_Enable,
	eX_CTCOM_Mode,
	eX_CTCOM_VLANIDMark,
	eX_CTCOM_8021pMark,
#endif
	eEthernetLinkStatus
};
struct CWMP_LEAF tWANETHLINKLeaf[] =
{
#ifdef _PRMT_X_CT_COM_ETHLINK_
	{ &tWANETHLINKLeafInfo[eX_CTCOM_Enable] },
	{ &tWANETHLINKLeafInfo[eX_CTCOM_Mode] },
{ &tWANETHLINKLeafInfo[eX_CTCOM_VLANIDMark] },
{ &tWANETHLINKLeafInfo[eX_CTCOM_8021pMark] },
#endif
{ &tWANETHLINKLeafInfo[eEthernetLinkStatus] },
{ NULL }
};
#endif

struct CWMP_OP tWANCONDEVENTITYLeafOP = { getWANCONDEVENTITY, NULL };
struct CWMP_PRMT tWANCONDEVENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WANIPConnectionNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tWANCONDEVENTITYLeafOP},
{"WANPPPConnectionNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tWANCONDEVENTITYLeafOP}
};
enum eWANCONDEVENTITYLeaf
{
	eWANIPConnectionNumberOfEntries,
	eWANPPPConnectionNumberOfEntries
};
struct CWMP_LEAF tWANCONDEVENTITYLeaf[] =
{
{ &tWANCONDEVENTITYLeafInfo[eWANIPConnectionNumberOfEntries] },
{ &tWANCONDEVENTITYLeafInfo[eWANPPPConnectionNumberOfEntries] },
{ NULL }
};



struct CWMP_OP tWAN_WANIPConnection_OP = { NULL, objWANIPConn };
struct CWMP_OP tWAN_WANPPPConnection_OP = { NULL, objWANPPPConn };
struct CWMP_PRMT tWANCONDEVENTITYObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
#ifdef CONFIG_DEV_xDSL
	{"WANDSLLinkConfig",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#if 1//defined CONFIG_ETHWAN
{"WANEthernetLinkConfig",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef CONFIG_DEV_xDSL
	{"WANATMF5LoopbackDiagnostics",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
{"WANIPConnection",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_WANIPConnection_OP},
{"WANPPPConnection",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tWAN_WANPPPConnection_OP}
};
enum eWANCONDEVENTITYObject
{
#ifdef CONFIG_DEV_xDSL
	eWANDSLLinkConfig,
#endif
#if 1//defined CONFIG_ETHWAN
	eWANEthernetLinkConfig,
#endif
#ifdef CONFIG_DEV_xDSL
	eWANATMF5LoopbackDiagnostics,
#endif
	eWANIPConnection,
	eWANPPPConnection
};
struct CWMP_NODE tWANCONDEVENTITYObject[] =
{
	/*info,  							leaf,			node)*/
#ifdef CONFIG_DEV_xDSL
	{&tWANCONDEVENTITYObjectInfo[eWANDSLLinkConfig], tDSLLNKCONFLeaf, NULL},
#endif
#if 1//defined CONFIG_ETHWAN
{&tWANCONDEVENTITYObjectInfo[eWANEthernetLinkConfig],	tWANETHLINKLeaf,	NULL},
#endif
#ifdef CONFIG_DEV_xDSL
	{&tWANCONDEVENTITYObjectInfo[eWANATMF5LoopbackDiagnostics],	 tWANATMF5LBLeaf, NULL},
#endif
{&tWANCONDEVENTITYObjectInfo[eWANIPConnection],			NULL,			NULL},
{&tWANCONDEVENTITYObjectInfo[eWANPPPConnection],		NULL,			NULL},
{NULL,								NULL,			NULL}
};


struct CWMP_PRMT tWANCONDEVObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWANCONDEVObject
{
	eWANCONDEV0
};
struct CWMP_LINKNODE tWANCONDEVObject[] =
{
/*info,  				leaf,			next,			sibling,		instnum)*/
{&tWANCONDEVObjectInfo[eWANCONDEV0],	tWANCONDEVENTITYLeaf,	tWANCONDEVENTITYObject,	NULL,			0},
};

#if 0 //keith remove. no support DSL
struct CWMP_OP tCONSERENTITYLeafOP = { getCONSERENTITY, NULL };
struct CWMP_PRMT tCONSERENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WANConnectionDevice",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"WANConnectionService",	eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"DestinationAddress",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"LinkType",			eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
{"ConnectionType",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Name",		eCWMP_tSTRING,	CWMP_READ,		&tCONSERENTITYLeafOP},
#endif
/*ping_zhang:20081217 END*/
};
enum eCONSERENTITYLeaf
{
	eCS_WANConnectionDevice,
	eCS_WANConnectionService,
	eCS_DestinationAddress,
	eCS_LinkType,
	eCS_ConnectionType,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eCS_Name,
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tCONSERENTITYLeaf[] =
{
{ &tCONSERENTITYLeafInfo[eCS_WANConnectionDevice] },
{ &tCONSERENTITYLeafInfo[eCS_WANConnectionService] },
{ &tCONSERENTITYLeafInfo[eCS_DestinationAddress] },
{ &tCONSERENTITYLeafInfo[eCS_LinkType] },
{ &tCONSERENTITYLeafInfo[eCS_ConnectionType] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tCONSERENTITYLeafInfo[eCS_Name] },
#endif
/*ping_zhang:20081217 END*/
{ NULL }
};
#endif //#if 0 //keith remove. no support DSL


#if 0 //keith remove. no support DSL
struct CWMP_PRMT tCONSERVICEObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eCONSERVICEObject
{
	eCONSERVICE0
};
struct CWMP_LINKNODE tCONSERVICEObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCONSERVICEObjectInfo[eCONSERVICE0],	tCONSERENTITYLeaf,	NULL,		NULL,			0},
};
#endif //#if 0 //keith remove. no support DSL

#if 0 //keith remove. no support DSL
struct CWMP_OP tWANDSLCNTMNGLeafOP = { getWANDSLCNTMNG,NULL };
struct CWMP_PRMT tWANDSLCNTMNGLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ConnectionServiceNumberOfEntries",eCWMP_tUINT,CWMP_READ,		&tWANDSLCNTMNGLeafOP}
};
#endif //#if 0 //keith remove. no support DSL

#if 0 //keith remove. no support DSL
enum eWANDSLCNTMNGLeaf
{
	eConnectionServiceNumberOfEntries
};
struct CWMP_LEAF tWANDSLCNTMNGLeaf[]=
{
{ &tWANDSLCNTMNGLeafInfo[eConnectionServiceNumberOfEntries]},
{ NULL }
};
#endif //#if 0 //keith remove. no support DSL

#if 0 //keith remove. no support DSL
struct CWMP_OP tWAN_ConnectionService_OP = { NULL, objConService };
struct CWMP_PRMT tWANDSLCNTMNGObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ConnectionService",		eCWMP_tOBJECT,	CWMP_READ,		&tWAN_ConnectionService_OP}
};
#endif //#if 0 //keith remove. no support DSL

#if 0 //keith remove. no support DSL
enum eWANDSLCNTMNGObject
{
	eConnectionService
};
struct CWMP_NODE tWANDSLCNTMNGObject[] =
{
/*info,  					leaf,			node)*/
{&tWANDSLCNTMNGObjectInfo[eConnectionService],	NULL,			NULL},
{NULL,						NULL,			NULL}
};
#endif //#if 0 //keith remove. no support DSL


/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_

int checkandmodify_reqoption_order(unsigned int order, int chainid)
{
	int ret=-1;
	int num,i;
	int maxorder;
	MIB_CE_DHCP_OPTION_T *p,pentry;

	p=&pentry;
	maxorder=findMaxDHCPReqOptionOrder();
	if(order>maxorder+1)
		goto checkresult;
	else{
		num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
		for( i=0; i<num;i++ )
		{
			if(i==chainid)
				continue;
			if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
				continue;
			if(p->usedFor!=eUsedFor_DHCPClient_Req)
				continue;
			if(p->order>=order){
				(p->order)++;
				mib_chain_update(MIB_DHCP_CLIENT_OPTION_TBL,(void*)p,i);
			}
		}
		ret=0;
	}

checkresult:
	return ret;
}

void compact_reqoption_order( )
{
	int ret=-1;
	int num,i,j;
	int maxorder;
	MIB_CE_DHCP_OPTION_T *p,pentry;
	char *orderflag;

	while(1){
		p=&pentry;
		maxorder=findMaxDHCPReqOptionOrder();
		orderflag=(char*)malloc(maxorder+1);
		if(orderflag==NULL) return;
		memset(orderflag,0,maxorder+1);

		num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
		for( i=0; i<num;i++ )
		{
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
					continue;
				if(p->usedFor!=eUsedFor_DHCPClient_Req)
					continue;
				orderflag[p->order]=1;
		}
		for(j=1;j<=maxorder;j++){
			if(orderflag[j]==0)
				break;
		} //star: there only one 0 in orderflag array
		if(j==(maxorder+1))
			break;
		for( i=0; i<num;i++ )
		{

				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
					continue;
				if(p->usedFor!=eUsedFor_DHCPClient_Req)
					continue;
				if(p->order>j){
					(p->order)--;
					mib_chain_update(MIB_DHCP_CLIENT_OPTION_TBL,(void*)p,i);
				}
		}

		if(orderflag)
		{
			free(orderflag);
			orderflag=NULL;
		}
	}

}

unsigned int DHCPClientReservedOption[]={
	53,  //DHCP Message type
	54,  //Server id
	50,  //Requested IP Addr
	0
};//star: I think some important options can't be changed by user, because they are necessary or are managed by other parameters.
int checkDHCPClientOptionTag(unsigned int tagvalue)
{
	int i;
	unsigned int *tmp=DHCPClientReservedOption;

	while(*tmp!=0){
		if(tagvalue == *tmp)
			return -1;
		tmp++;
	}

	return 0;
}

int getWANIPConDHCPClientENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char usedFor;
	unsigned int devnum,ipnum;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned int pvcchainid;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	ipnum = getWANIPConInstNum( name );
	devnum = getWANConDevInstNum(name);
	if(ipnum==0 || devnum==0)   return ERR_9005;
	getATMVCEntryByIPInstNum(devnum,ipnum,&pvcentry,&pvcchainid);

	if( strcmp( lastname, "SentDHCPOptionNumberOfEntries" )==0 )
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		*data = uintdup(findDHCPOptionNum(usedFor, pvcentry.ifIndex));
	}
	else if( strcmp( lastname, "ReqDHCPOptionNumberOfEntries" )==0 )
	{
		usedFor = eUsedFor_DHCPClient_Req;
		*data = uintdup(findDHCPOptionNum(usedFor,pvcentry.ifIndex));
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int getDHCPClientOptENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int sentDhcpOptNum,reqDhcpOptNum,ipnum,devnum;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned char usedFor;
	unsigned int chainid,pvcchainid;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	sentDhcpOptNum = getSentDHCPOptInstNum( name );
	reqDhcpOptNum = getReqDHCPOptInstNum( name );
	ipnum = getWANIPConInstNum( name );
	devnum = getWANConDevInstNum(name);

	if(ipnum==0 || devnum==0)   return ERR_9005;
	if( sentDhcpOptNum==0 && reqDhcpOptNum ==0 ) return ERR_9005;

	pDHCPOptEntry = &DhcpOptEntry;
	getATMVCEntryByIPInstNum(devnum,ipnum,&pvcentry,&pvcchainid);
	if(sentDhcpOptNum != 0)   	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.SentDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		if( getDHCPClientOptionByOptInstNum(sentDhcpOptNum, pvcentry.ifIndex,  usedFor, pDHCPOptEntry, &chainid) < 0)
			return ERR_9002;
	}
	else if(reqDhcpOptNum !=0)	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.ReqDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Req;
		if( getDHCPClientOptionByOptInstNum(reqDhcpOptNum, pvcentry.ifIndex, usedFor,pDHCPOptEntry, &chainid) < 0 )
			return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pDHCPOptEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Order")==0)
	{
		if(usedFor != eUsedFor_DHCPClient_Req)
			return ERR_9005;
		*data = uintdup(pDHCPOptEntry->order);
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		*data = uintdup(pDHCPOptEntry->tag);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		struct xsd__base64 tmp;
		tmp.__ptr=pDHCPOptEntry->value;
		tmp.__size=pDHCPOptEntry->len;
		*data=base64dup( tmp );
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int setDHCPClientOptENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int sentDhcpOptNum,reqDhcpOptNum,ipnum,devnum;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned char usedFor;
	unsigned int chainid,pvcchainid;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	sentDhcpOptNum = getSentDHCPOptInstNum( name );
	reqDhcpOptNum = getReqDHCPOptInstNum( name );
	ipnum = getWANIPConInstNum( name );
	devnum = getWANConDevInstNum(name);

	if(ipnum==0 || devnum==0)   return ERR_9005;
	if (sentDhcpOptNum==0 && reqDhcpOptNum ==0 ) return ERR_9005;

	pDHCPOptEntry = &DhcpOptEntry;
	getATMVCEntryByIPInstNum(devnum,ipnum,&pvcentry,&pvcchainid);
	if(sentDhcpOptNum != 0)   	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.SentDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		if( getDHCPClientOptionByOptInstNum(sentDhcpOptNum, pvcentry.ifIndex, usedFor, pDHCPOptEntry, &chainid) < 0)
		return ERR_9002;
	}
	else if(reqDhcpOptNum !=0)	//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.ReqDHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPClient_Req;
		if( getDHCPClientOptionByOptInstNum(reqDhcpOptNum, pvcentry.ifIndex, usedFor,pDHCPOptEntry, &chainid) < 0 )
		return ERR_9002;
	}

	if(usedFor==eUsedFor_DHCPClient_Sent){
		if(checkDHCPClientOptionTag(pDHCPOptEntry->tag)<0)
			return ERR_9001;
	}

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPOptEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return 1;
	}
	else if( strcmp( lastname, "Order" )==0 )
	{
		unsigned int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 ) return ERR_9007;
		if(pDHCPOptEntry->usedFor != eUsedFor_DHCPClient_Req) return ERR_9005;
		if(checkandmodify_reqoption_order(*i,chainid)<0) return ERR_9007;
		pDHCPOptEntry->order = *i;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		compact_reqoption_order();
		return 1;
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 || *i>254) return ERR_9007;
		if(usedFor==eUsedFor_DHCPClient_Sent){
			if(checkDHCPClientOptionTag(*i)<0)
				return ERR_9001;
		}
		pDHCPOptEntry->tag = *i;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return 1;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(pDHCPOptEntry->usedFor == eUsedFor_DHCPClient_Req)
			return ERR_9001;

		if(data)
		{
			   int i;
			   struct xsd__base64 *b=data;
			   //fprintf( stderr, "<xsd__base64:size %d>", b->__size );
			   for( i=0; i<b->__size; i++ )
			   {
			    //fprintf( stderr, "%u(%c) ", b->__ptr[i], b->__ptr[i]  );
			   }
			   //fprintf( stderr, "\n" ); // by cairui
			    if(b->__size>DHCP_OPT_VAL_LEN) return ERR_9001;
			   pDHCPOptEntry->len=b->__size;
			   memcpy(pDHCPOptEntry->value,b->__ptr,b->__size);
		}
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return 1;
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.SentDHCPOption.{i}.
int objDHCPClientSentOpt(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int num,i;
	unsigned char usedFor;
	unsigned int chainid;
	unsigned int ipnum,devnum;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned int pvcchainid;

	usedFor = eUsedFor_DHCPClient_Sent;

	ipnum = getWANIPConInstNum( name );
	devnum = getWANConDevInstNum(name);
	getATMVCEntryByIPInstNum(devnum,ipnum,&pvcentry,&pvcchainid);
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			int MaxInstNum;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = findMaxDHCPClientOptionInstNum(usedFor, pvcentry.ifIndex);
			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=0; i<num;i++ )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if(pDHCPOptEntry->usedFor != usedFor ||  pDHCPOptEntry->ifIndex != pvcentry.ifIndex)
					continue;
				if( pDHCPOptEntry->dhcpOptInstNum==0 ) //maybe createn by web or cli
				{
					MaxInstNum++;
					pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
					mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
				}
				{
					if( create_Object( c, tDHCPClientSentObject, sizeof(tDHCPClientSentObject), 1, pDHCPOptEntry->dhcpOptInstNum ) < 0 )
						return -1;
				}
			}
			add_objectNum( name, MaxInstNum );
			return 0;
		}
	case eCWMP_tADDOBJ:
		{
			int ret, found=0;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientSentObject, sizeof(tDHCPClientSentObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DHCP_OPTION_T entry;
				memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.usedFor = usedFor;
					entry.dhcpOptInstNum = *(int *)data;
					entry.dhcpConSPInstNum = 0;
					entry.ifIndex = pvcentry.ifIndex;
				}

				mib_chain_add( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)&entry);
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=num-1; i>=0;i-- )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
					continue;
				if(pDHCPOptEntry->usedFor == usedFor && pDHCPOptEntry->ifIndex == pvcentry.ifIndex
					&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
				{
					found =1;
					mib_chain_delete( MIB_DHCP_CLIENT_OPTION_TBL, i );
					break;
				}
			}

			if(found==0) return ERR_9005;
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;
			return ret;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if( (pDHCPOptEntry->usedFor == usedFor) && ( pDHCPOptEntry->ifIndex == pvcentry.ifIndex) && (pDHCPOptEntry->dhcpOptInstNum!=0))
				{
					remove_entity = remove_SiblingEntity( &old_table, pDHCPOptEntry->dhcpOptInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
						if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pDHCPOptEntry->dhcpOptInstNum )==NULL )
						{
							unsigned int MaxInstNum = pDHCPOptEntry->dhcpOptInstNum;
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientSentObject, sizeof(tDHCPClientSentObject), &MaxInstNum );
						}//else already in next_table
					}
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			return 0;
		}
	}
	return -1;
}

//for IGD.LANDevice.{i}.WANDevice.{i}.WANIPConnectionDevice.{i}.DHCPClient.ReqDHCPOption.{i}.
int objDHCPClientReqOpt(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int num,i;
	unsigned char usedFor;
	unsigned int chainid;
	unsigned int ipnum,devnum;
	MIB_CE_ATM_VC_T pvcentry;
	unsigned int pvcchainid;

	usedFor = eUsedFor_DHCPClient_Req;

	ipnum = getWANIPConInstNum( name );
	devnum = getWANConDevInstNum(name);
	getATMVCEntryByIPInstNum(devnum,ipnum,&pvcentry,&pvcchainid);

	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			int MaxInstNum;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = findMaxDHCPClientOptionInstNum(usedFor,pvcentry.ifIndex);
			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=0; i<num;i++ )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;
				if(pDHCPOptEntry->usedFor != usedFor || pDHCPOptEntry->ifIndex!=pvcentry.ifIndex)
					continue;
				if( pDHCPOptEntry->dhcpOptInstNum==0 ) //maybe createn by web or cli
				{
					MaxInstNum++;
					pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
					mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
				}
				{
					if( create_Object( c, tDHCPClientReqObject, sizeof(tDHCPClientReqObject), 1, pDHCPOptEntry->dhcpOptInstNum ) < 0 )
						return -1;
				}
			}
			add_objectNum( name, MaxInstNum );
			return 0;
		}
	case eCWMP_tADDOBJ:
		{
			int ret, found=0;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientReqObject, sizeof(tDHCPClientReqObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DHCP_OPTION_T entry;
				memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.usedFor = usedFor;
					entry.dhcpOptInstNum = *(int *)data;
					entry.dhcpConSPInstNum = 0;
					entry.ifIndex = pvcentry.ifIndex;
					entry.order = findMaxDHCPReqOptionOrder()+1;
				}
				mib_chain_add( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)&entry);
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=num-1; i>=0;i-- )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
					continue;
				if(pDHCPOptEntry->usedFor == usedFor && pDHCPOptEntry->ifIndex == pvcentry.ifIndex
					&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
				{
					found =1;
					mib_chain_delete( MIB_DHCP_CLIENT_OPTION_TBL, i );
					compact_reqoption_order();
					break;
				}
			}

			if(found==0) return ERR_9005;
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;
			return ret;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if( (pDHCPOptEntry->usedFor == usedFor) && (pDHCPOptEntry->ifIndex == pvcentry.ifIndex) && (pDHCPOptEntry->dhcpOptInstNum!=0))
				{
					remove_entity = remove_SiblingEntity( &old_table, pDHCPOptEntry->dhcpOptInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
						if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pDHCPOptEntry->dhcpOptInstNum )==NULL )
						{
							unsigned int MaxInstNum = pDHCPOptEntry->dhcpOptInstNum;
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPClientReqObject, sizeof(tDHCPClientReqObject), &MaxInstNum );
						}//else already in next_table
					}
				}
			}

			if( old_table )
			destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			return 0;
		}
	}
	return -1;
}
#endif
/*ping_zhang:20080919 END*/

int getPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int 	devnum, mapnum, pppnum, ipnum;
	PORTFW_T port_entity;
	int num=0;
	int		port_chainid=0;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	devnum  = getWANConDevInstNum( name );
	ipnum   = getWANIPConInstNum( name );
	pppnum  = getWANPPPConInstNum( name );
	mapnum  = getWANPortMapInstNum( name );
	if( (mapnum==0) || (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return ERR_9005;

	getPortMappingCount( &num );
	memset( &port_entity, 0, sizeof( PORTFW_T ) );


	if( getPortMappingByInstNum( 0, mapnum, &port_entity, &port_chainid ) )
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PortMappingEnabled" )==0 )
	{
		int vUint;
		mib_get( MIB_PORTFW_ENABLED, (void *)&vUint);
		*data = booldup(vUint);
	}

	else if( strcmp( lastname, "PortMappingLeaseDuration" )==0 )
	{	
		*data = booldup(0);
	}else if( strcmp( lastname, "RemoteHost" )==0 )
	{	
		*data = strdup( "" );
	}

	else if( strcmp( lastname, "ExternalPort" )==0 )
	{	
		*data=uintdup( port_entity.toPort );
	}	
	else if( strcmp( lastname, "InternalPort" )==0 )
	{	
		*data=uintdup( port_entity.svrport );
	}
	else if( strcmp( lastname, "PortMappingProtocol" )==0 )
	{	
		if( port_entity.protoType==PROTO_TCP )
			*data = strdup( "TCP" );
		else if( port_entity.protoType==PROTO_UDP )
			*data = strdup( "UDP" );
		else if (port_entity.protoType==PROTO_BOTH )
			*data = strdup("TCPandUDP");
		else /*PROTO_NONE or PROTO_ICMP*/
			*data = strdup( "" );
	}else if( strcmp( lastname, "InternalClient" )==0 )
	{	
		char *tmp;		
		tmp = inet_ntoa(*((struct in_addr *)&(port_entity.ipAddr)));
		if(tmp)
			*data = strdup( tmp ); 
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "PortMappingDescription" )==0 )
	{	
		*data = strdup( port_entity.comment );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int 	devnum, mapnum, pppnum, ipnum;
	char		*buf=data;
	int		port_chainid=0;
	PORTFW_T target[2], *port_ent=NULL;
	PORTFW_T port_entity;

	int num=0;
	int ret = 0;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;

	if( entity->info->type!=type ) return ERR_9006;


	devnum  = getWANConDevInstNum( name );
	ipnum   = getWANIPConInstNum( name );
	pppnum  = getWANPPPConInstNum( name );
	mapnum  = getWANPortMapInstNum( name );
	if( (mapnum==0) || (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return ERR_9005;

	getPortMappingCount( &num );
	memset( &target[0], 0, sizeof( PORTFW_T ) );
	memset( &target[1], 0, sizeof( PORTFW_T ) );


	if( getPortMappingByInstNum( 0, mapnum, &port_entity, &port_chainid ) )
		return ERR_9002;

	*((char *)&target[0]) = (char)port_chainid;
	if(!(mib_get(MIB_PORTFW_TBL, (void *)&target[0])))
		return ERR_9002;

	memcpy(&target[1], &target[0], sizeof(PORTFW_T));
	port_ent = &target[1];


	if( strcmp( lastname, "PortMappingEnabled" )==0 )
	{
		int vUint = data;
		mib_set( MIB_PORTFW_ENABLED, (void *)&vUint);
		ret = 1;
	}

	else if( strcmp( lastname, "PortMappingLeaseDuration" )==0 )
	{	
		unsigned int *i = data;
		//only support value 0
		if( *i!=0 ) return ERR_9001;
		return 1;
	}
	else if( strcmp( lastname, "RemoteHost" )==0 )
	{	
		return 1;
	}
	else if( strcmp( lastname, "ExternalPort" )==0 )
	{	

		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;

#if defined(MOD_FOR_TR098_PORTMAP)
		port_ent->fromPort= *i;
		port_ent->toPort = *i;
#else
		port_entity.fromPort= *i;
		port_entity.toPort = *i;
#endif

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(vtlsvr_entryx) );
		ret = 0;
#else

		ret = 1;
#endif
	}	
	else if( strcmp( lastname, "InternalPort" )==0 )
	{	

		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;

#if defined(MOD_FOR_TR098_PORTMAP)
		port_ent->fromPort= *i;
		port_ent->toPort = *i;
#else
		port_entity.fromPort= *i;
		port_entity.toPort = *i;
#endif

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(vtlsvr_entryx) );
		ret = 0;
#else
		ret = 1;
#endif
	}
	else if( strcmp( lastname, "PortMappingProtocol" )==0 )
	{	
		if( strlen(buf)==0 ) return ERR_9007;

#if defined(MOD_FOR_TR098_PORTMAP)
		if( strcmp( buf, "TCP" )==0 )
			port_ent->protoType = PROTO_TCP;
		else if( strcmp( buf, "UDP" )==0 )
			port_ent->protoType = PROTO_UDP;
		else if ( strcmp( buf, "TCPandUDP" )==0 )
			port_ent->protoType = PROTO_BOTH;
		else
			return ERR_9007;
#else
		if( strcmp( buf, "TCP" )==0 )
			port_entity.protoType = PROTO_TCP;
		else if( strcmp( buf, "UDP" )==0 )
			port_entity.protoType = PROTO_UDP;
		else if ( strcmp( buf, "TCPandUDP" )==0 )
			port_entity.protoType = PROTO_BOTH;
		else
			return ERR_9007;
#endif

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(vtlsvr_entryx) );
		ret = 0;
#else
		ret = 1;
#endif
	}else if( strcmp( lastname, "InternalClient" )==0 )
	{	
		if( strlen(buf)==0 ) return ERR_9007; //can't be empty
			
#if defined(MOD_FOR_TR098_PORTMAP)
		if ( !inet_aton(buf, (struct in_addr *)&(port_ent->ipAddr)) )
			return ERR_9007;
#else
		if ( !inet_aton(buf, (struct in_addr *)&(port_entity.ipAddr)) )
			return ERR_9007;
#endif

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, port_chainid, &OldPort_Entity, sizeof(vtlsvr_entryx) );
		ret = 0;
#else
		ret = 1;
#endif
	}else if( strcmp( lastname, "PortMappingDescription" )==0 )
	{
#if defined(MOD_FOR_TR098_PORTMAP)
		strncpy( port_ent->comment, buf, COMMENT_LEN-1 );
		port_ent->comment[COMMENT_LEN-1]=0;
#else
		strncpy( port_entity.comment, buf, COMMENT_LEN-1 );
		port_entity.comment[COMMENT_LEN-1]=0;
#endif
		ret = 0;
	}
	else
	{
		return ERR_9005;
	}


#if defined(MOD_FOR_TR098_PORTMAP)
	//if(memcmp(&port_entity,&remove_port_entity,sizeof(PORTFW_T))!=0)
	{
		if ( !mib_set(MIB_PORTFW_MOD, (void *)target)) 
		{
			return ERR_9005;
		}
	}
#else
	if(memcmp(&port_entity,&remove_port_entity,sizeof(PORTFW_T))!=0)
		{
		if ( !mib_set(MIB_PORTFW_DEL, (void *)&remove_port_entity)) 
		{
			return ERR_9005;
		}
		else
		{
			if ( !mib_set(MIB_PORTFW_ADD, (void *)&port_entity)) 
			{
				return ERR_9005;
			}
		}
	}
#endif
//mib_update(CURRENT_SETTING);	//brucehou, tmp
	
	return ret;
}

int objWANPORTMAPPING(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int devnum,ipnum,pppnum;
	unsigned int chainid;
	PORTFW_T entry;
	
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);		
	if( (name==NULL) || (entity==NULL) ) return -1;

	devnum  = getWANConDevInstNum( name );
	ipnum   = getWANIPConInstNum( name );
	pppnum  = getWANPPPConInstNum( name );
	if( (devnum==0) || ( (ipnum==0) && (pppnum==0) ) ) return ERR_9005;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int num=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		PORTFW_T *p, port_entity;
		unsigned int port_chainID=0, MaxInstNum=0;
		PORTFW_T target[2];
		memset( &target[0], 0, sizeof( PORTFW_T ) );
		memset( &target[1], 0, sizeof( PORTFW_T ) );
		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		
		MaxInstNum = getPortMappingMaxInstNum();
		getPortMappingCount( &num );
		
		for( i=1; i<=num;i++ )
		{
			*((char *)&port_entity) = (char)i;
			mib_get(MIB_PORTFW_TBL, (void *)&port_entity);
			memcpy(&target[0], &port_entity, sizeof(PORTFW_T));
					
			if( port_entity.InstanceNum== 0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				port_entity.InstanceNum = MaxInstNum;
				memcpy(&target[1], &port_entity, sizeof(PORTFW_T));
				mib_set(MIB_PORTFW_MOD, (void *)&target);
			}

			if( create_Object( c, tWANPORTMAPObject, sizeof(tWANPORTMAPObject), 1, port_entity.InstanceNum ) < 0 )				
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
		int num;



		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = get_ParameterNameCount(name,1);

		if(num >= MAX_FILTER_NUM)
	  		return ERR_9004;
	  		
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPORTMAPObject, sizeof(tWANPORTMAPObject), data );


		if( ret >= 0 )
		{
			

			PORTFW_T port_entity;
			char lan_ip[32];
			
			memset(lan_ip,0x00,sizeof(lan_ip));
			memset( &port_entity, 0, sizeof( PORTFW_T ) );


			sprintf(lan_ip,"%d.%d.%d.%d",0, 0, 0, 0 ); //110: temp ip.
			inet_aton(lan_ip, (struct in_addr *)&port_entity.ipAddr);
			port_entity.fromPort = 0;
			port_entity.toPort = 0;
		
			port_entity.InstanceNum= *(int*)data;

			port_entity.protoType = PROTO_BOTH;
			sprintf(port_entity.comment,"%s[%u]", "Created by TR069",port_entity.InstanceNum);

			if ( mib_set(MIB_PORTFW_ADD, (void *)&port_entity) == 0) 
			{
				//fprintf(stderr,"\r\n Add PW table entry error!");
				return -1;
			}
		}


#ifndef _CWMP_APPLY_
		if(ret==0) ret=1;
#endif
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret;
		PORTFW_T port_entity;
		unsigned int port_chainID=0;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		if( getPortMappingByInstNum( 0, *(unsigned int*)data, &port_entity, &port_chainID ) )
			return -1;

		if ( !mib_set(MIB_PORTFW_DEL, (void *)&port_entity)) 
		{
			//fprintf(stderr,"\r\n Del PW table entry error!");
			return -1;
		}

		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
#ifndef _CWMP_APPLY_
		if(ret==0) ret=1;
#endif
		return ret;
	     	break;
	     }
	case eCWMP_tUPDATEOBJ:		
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	unsigned int port_chainID=0;
	     	PORTFW_T target[2];
	     	//fprintf(stderr, "<%s:%d>action=eCWMP_tUPDATEOBJ(name=%s)\n", __FUNCTION__, __LINE__, name );
	     	
		getPortMappingCount( &num );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
		memset( &target[0], 0, sizeof( PORTFW_T ) );
		memset( &target[1], 0, sizeof( PORTFW_T ) );
	     	for( i=1; i<=num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			PORTFW_T *p, port_entity;

			p = &port_entity;

			*((char *)&port_entity) = (char)i;
			mib_get(MIB_PORTFW_TBL, (void *)&port_entity);
			memcpy(&target[0], &port_entity, sizeof(PORTFW_T));
			//printf("%s:%d:port_entity.InstanceNum=%d\n", __FUNCTION__, __LINE__, port_entity.InstanceNum);
			remove_entity = remove_SiblingEntity( &old_table, port_entity.InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=port_entity.InstanceNum;

				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPORTMAPObject, sizeof(tWANPORTMAPObject), &MaxInstNum );

				if(MaxInstNum!=port_entity.InstanceNum)
				{				
					port_entity.InstanceNum = MaxInstNum;
					//printf("%s:%d:port_entity.InstanceNum=%d\n", __FUNCTION__, __LINE__, port_entity.InstanceNum);
					memcpy(&target[1], &port_entity, sizeof(PORTFW_T));
					if ( !mib_set(MIB_PORTFW_MOD, (void *)&target)) 
	     				{
							return -1;
					}
				}
			}	
	     	}


		//add_objectNum(name, num); //update ParameterTable max num
	     	
	     	if( old_table )
	     		{
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	     		}


	     	return 0;
	     }
	
	}

	return -1;
}
#ifdef VIRTUAL_SERVER_SUPPORT

#else

#endif

/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIP/PPPConnection.Stats*/
int getWANCONSTATS(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	*tok;
	unsigned int	chainid;
#if defined(MULTI_WAN_SUPPORT)
	WANIFACE_T *pEntry,wan_entity;
#endif 	
	char	ifname[16];
	char	buf[256];
#ifdef CONFIG_DEV_xDSL
	unsigned long 	bs=0,br=0,ps=0,pr=0;
#endif
	struct user_net_device_stats stats;

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	unsigned long es=0,er=0, ups=0,upr=0, dps=0,dpr=0, mps=0,mpr=0, bps=0, bpr=0, uppr=0;
#endif
/*ping_zhang:20081217 END*/
	unsigned int devnum,ipnum,pppnum;
	OPMODE_T opmode=-1;
	char *iface=NULL;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	devnum  = getWANConDevInstNum( name );
	ipnum   = getWANIPConInstNum( name );
	pppnum  = getWANPPPConInstNum( name );
	if( (devnum==0) || ( (ipnum==0) && (pppnum==0) ) ) return ERR_9005;

#if defined(MULTI_WAN_SUPPORT)
	aaaa;
	pEntry = &wan_entity;	
	if( ipnum ) //ipconnection
	{
		if( getATMVCEntryByIPInstNum( devnum, ipnum, pEntry, &chainid )<0 ) 
			return ERR_9002;
	}else{ //pppconnection
		if( getATMVCEntryByPPPInstNum( devnum, pppnum, pEntry, &chainid )<0 ) 
			return ERR_9002;
	}	
	
	ifGetName(pEntry->ifIndex, ifname, sizeof(ifname));
	
#ifdef CONFIG_DEV_xDSL
	getInterfaceStat( ifname, &bs, &br, &ps, &pr );
#endif
	if(getStats(ifname, &stats)< 0)
		return ERR_9002;
#endif //#if defined(MULTI_WAN_SUPPORT)

	mib_get( MIB_OP_MODE, (void *)&opmode);
	if(opmode == WISP_MODE)
		iface = "wlan0";
	else
		iface = "eth1";

	if (getStats(iface, &stats) < 0)
		stats.tx_packets = 0;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "EthernetBytesSent" )==0 )
	{
#ifdef CONFIG_DEV_xDSL
		*data = uintdup( bs );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup( stats.tx_bytes );
#else
		*data = uintdup(1);
#endif
	}
	else if( strcmp( lastname, "EthernetBytesReceived" )==0 )
	{	
			
#ifdef CONFIG_DEV_xDSL
		*data = uintdup( br );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup( stats.rx_bytes );
#else
		*data = uintdup(1);	
#endif
			
	}
	else if( strcmp( lastname, "EthernetPacketsSent" )==0 )
	{	
			
#ifdef CONFIG_DEV_xDSL
		*data = uintdup( ps );
#endif
#if 1//defined CONFIG_ETHWAN
			*data = uintdup( stats.tx_packets );
#else
			*data = uintdup(1);
#endif
			
	}
	else if( strcmp( lastname, "EthernetPacketsReceived" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		*data = uintdup( pr );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup( stats.rx_packets );
#else
		*data = uintdup(1);
#endif
	}
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	else if( strcmp( lastname, "EthernetErrorsSent" )==0 )
	{	
		
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( es );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(nds.tx_errors);
#endif
		
	}else if( strcmp( lastname, "EthernetErrorsReceived" )==0 )
	{	
		
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( er );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(nds.rx_errors);
#endif
		
	}
	else if( strcmp( lastname, "EthernetUnicastPacketsSent" )==0 )
	{
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( ups );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(0);
#endif
	}
	else if( strcmp( lastname, "EthernetUnicastPacketsReceived" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( upr );
#endif
#if 1//defined CONFIG_ETHWAN
			*data = uintdup(0);
#endif
	}

	else if( strcmp( lastname, "EthernetDiscardPacketsSent" )==0 )
	{
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( dps );
#endif
#if 0//defined CONFIG_ETHWAN
		*data = uintdup( stats.tx_dropped );
#else
		*data = uintdup(1);
#endif
	}else if( strcmp( lastname, "EthernetDiscardPacketsReceived" )==0 )
	{

#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( dpr );
#endif
#if 0//defined CONFIG_ETHWAN
		*data = uintdup( stats.rx_dropped );
#else
		*data = uintdup(1);
#endif
	}
	else if( strcmp( lastname, "EthernetMulticastPacketsSent" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( mps );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(0);
#endif
	}
	else if( strcmp( lastname, "EthernetMulticastPacketsReceived" )==0 )
	{
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( mpr );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(0);
#endif
	}
	else if( strcmp( lastname, "EthernetBroadcastPacketsSent" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( mpr );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(0);
#endif
	}
	else if( strcmp( lastname, "EthernetBroadcastPacketsReceived" )==0 )
	{	
#ifdef CONFIG_DEV_xDSL	
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( bpr );	
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(0);
#endif

	}
	else if( strcmp( lastname, "EthernetUnknownProtoPacketsReceived" )==0 )
	{
#ifdef CONFIG_DEV_xDSL		
		if( getInterfaceStat1( ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( uppr );
#endif
#if 1//defined CONFIG_ETHWAN
		*data = uintdup(0);
#endif		
	}
#endif //#ifdef _PRMT_WT107_
/*ping_zhang:20081217 END*/
	else{
		return ERR_9005;
	}
	
	return 0;
}


/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANPPPConnection.*/
int getWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	//printf("enter getWANPPPCONENTITY\n");
	char	*lastname = entity->info->name;
	char	*tok;
	unsigned int	chainid;
#if defined(MULTI_WAN_SUPPORT)
	WANIFACE_T *pEntry,wan_entity;
#endif 	
	char	ifname[16];
	char	buf[512]="";
	unsigned int devnum,pppnum;
	unsigned int vChar=0;
	int wan_dhcp;
	mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
	//printf("wan_dhcp=%d\n", wan_dhcp);
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;


	devnum = getWANConDevInstNum( name );
	pppnum  = getWANPPPConInstNum( name );
	if( (devnum==0) || (pppnum==0) ) return ERR_9005;

#if defined(MULTI_WAN_SUPPORT)
	pEntry = &wan_entity;
	if( getATMVCEntryByPPPInstNum( devnum, pppnum, pEntry, &chainid )<0 ) 
		return ERR_9002;
	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;
	//printf("%s:ifname=%s\n", __FUNCTION__,ifname);
#else
	strcpy(ifname,"ppp0");
#endif

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
#else
		if(wan_dhcp != PPPOE)
			*data = booldup(0);
		else
			*data = booldup(1);
		
		
#endif
	}
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	else if( strcmp( lastname, "Reset" )==0 )
	{
		/*while read, this parameter always returns False.*/
		*data = booldup(0);
	}
#endif
/*ping_zhang:20081217 END*/
	else if( strcmp( lastname, "ConnectionStatus" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		char pppstatus[32]={0};
#ifdef PPPOE_PASSTHROUGH
		int flags;
		
		if(pEntry->cmode==IP_BRIDGE)
		{	
			if(pEntry->enable==0)
				*data = strdup( "Disconnected" );
			else if(getInFlags( ifname, &flags) == 1)
		{
				if (flags & IFF_UP)
					*data = strdup( "Connected" );
				else
					*data = strdup( "Disconnected" );
			}else
				*data = strdup( "Disconnected" );
		}else
#endif
#else //#if defined(MULTI_WAN_SUPPORT)
		int isWanPhy_Link=0;
		OPMODE_T opmode=-1;
		DHCP_T dhcp;
		mib_get(MIB_WAN_DHCP, (void *)&dhcp);
		
		mib_get( MIB_OP_MODE, (void *)&opmode);
		
		if(opmode != WISP_MODE)
		{	
 			isWanPhy_Link=getWanLink("eth1"); 
 		}
		
		if ( dhcp ==  PPPOE )
		{
			if ( isConnectPPP())
			{
				if(isWanPhy_Link < 0)
						*data = strdup( "Disconnected" );
					else
						*data = strdup("Connected");				
			}
			else			
				*data = strdup( "Disconnected" );
		}
		else
		{
			*data = strdup( "Disconnected" );
		}
			
#endif //#if defined(MULTI_WAN_SUPPORT)
#if 0  //NOT Support YET
		if( getPPPConStatus( ifname, pppstatus )==0 )
			{
				/*pppstatus is defined in if_spppsubr.c*/
			if( strcmp( pppstatus, "Dead" )==0 )
						*data = strdup( "Disconnected" );
			else if( strcmp( pppstatus, "Establish" )==0 )
				*data = strdup( "Connecting" );
			else if( strcmp( pppstatus, "Terminate" )==0 )
				*data = strdup( "Disconnecting" );
			else if( strcmp( pppstatus, "Authenticate" )==0 )
				*data = strdup( "Authenticating" );
			else if( strcmp( pppstatus, "Network" )==0 )
						*data = strdup("Connected");
			else			
				*data = strdup( "Disconnected" );
		}else
#endif		
	}	
	else if( strcmp( lastname, "PossibleConnectionTypes" )==0 )
	{	
#ifdef PPPOE_PASSTHROUGH
		*data = strdup( "IP_Routed,PPPoE_Bridged" );
#else
		*data = strdup( "IP_Routed" );
#endif
	}
	else if( strcmp( lastname, "ConnectionType" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)	
#ifdef PPPOE_PASSTHROUGH
		if(pEntry->cmode==IP_BRIDGE)
			*data = strdup( "PPPoE_Bridged" );
		else
#endif
#endif //#if defined(MULTI_WAN_SUPPORT)
			*data = strdup( "IP_Routed" );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
	}
#ifdef _PRMT_WT107_
	else if( strcmp( lastname, "PPPoESessionID" )==0 )
	{
		unsigned char totalEntry;
		MIB_CE_PPPOE_SESSION_T Entry;
		unsigned int i,sessionID=0,found=0;

		totalEntry = mib_chain_total(MIB_PPPOE_SESSION_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_PPPOE_SESSION_TBL, i, (void *)&Entry)) {
				return ERR_9002;
			}
			if (Entry.vpi == pEntry->vpi && Entry.vci == pEntry->vci
				&& Entry.ifNo == PPP_INDEX(pEntry->ifIndex)  ) 
			{
				sessionID = Entry.sessionId;
				break;
			}
		}
		if(sessionID < 1) 
			*data = intdup(0);
		else
			*data = intdup(sessionID);
	}else if( strcmp( lastname, "DefaultGateway" )==0 )
	{
		aaaaa;
		char strWanIP[16];
		char strWanMask[16];
		char strWanDefIP[16];
		char strWanHWAddr[18];
		cwmp_getWanInfo(strWanIP,strWanMask,strWanDefIP,strWanHWAddr);
			
		*data=strdup(strWanDefIP);
	}
#endif // end ifdef _PRMT_WT107_
/*ping_zhang:20081217 END*/	
	else if( strcmp( lastname, "Name" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		if(*(pEntry->WanName))
			*data = strdup( pEntry->WanName );
#ifdef CTC_WAN_NAME
		else
		{//if not set by ACS. then generate automaticly.
			char wanname[40];
			memset(wanname, 0, sizeof(wanname));
			generateWanName(pEntry, wanname);
			*data = strdup( wanname );
		}
#endif
#else //#if defined(MULTI_WAN_SUPPORT)
                *data = strdup( "" );
#endif //#if defined(MULTI_WAN_SUPPORT)
	}
	else if( strcmp( lastname, "Uptime" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		unsigned int uptime;
#ifdef PPPOE_PASSTHROUGH
		if(pEntry->cmode==IP_BRIDGE)
		{
#if 0
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
#else
		//*data = time(NULL);
		unsigned long sec;
		sec = (unsigned long)get_uptime();
		*data = uintdup( sec );
#endif
			return 0;
		}
#endif
		//NOT Support YET
//		if( getPPPUptime( ifname, pEntry->cmode, &uptime )==0 )
//			*data = uintdup( uptime );
//		else
			*data = uintdup( 0 );
#else //
		//struct sysinfo info;
		DHCP_T dhcp;
		mib_get(MIB_WAN_DHCP, (void *)&dhcp);
		int isWanPhy_Link;
		
		isWanPhy_Link=getWanLink("eth1"); 
		
		if ( dhcp ==  PPPOE )
		{
			if ( isConnectPPP())
			{
				if(isWanPhy_Link < 0)
					*data = uintdup( 0 );
				else
				{
#if 0
					sysinfo(&info);
					*data = uintdup( info.uptime );
#else
					*data = uintdup( 0 );
#endif
				}
			}
			else			
				*data = uintdup( 0 );
		}
		else
			*data = uintdup( 0 );
#endif //#if defined(MULTI_WAN_SUPPORT)
	}
	else if( strcmp( lastname, "LastConnectionError" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)	
#ifdef PPPOE_PASSTHROUGH
		if(pEntry->cmode==IP_BRIDGE)
			*data = strdup( "ERROR_NONE" );
		else
#endif
		//NOT Support YET
//		*data = strdup(getLastConnectionError(pEntry->ifIndex));	// Jenny
#endif //#if defined(MULTI_WAN_SUPPORT)
		*data = strdup( "ERROR_NONE" );
	}
	else if( strcmp( lastname, "AutoDisconnectTime" )==0 )
	{	
		*data = uintdup( 0 ); //only 0:the connection is not to be shut down automatically
//		*data = uintdup( pEntry->autoDisTime );		// Jenny
	}
	else if( strcmp( lastname, "IdleDisconnectTime" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)		
		*data = uintdup( pEntry->pppIdleTime*60 );
#else
		int idleTime=0;

		mib_get(MIB_PPP_IDLE_TIME, (void *)&idleTime);
		*data = uintdup(idleTime);
#endif
	}

	else if( strcmp( lastname, "WarnDisconnectDelay" )==0 )	// Jenny
	{	
#if defined(MULTI_WAN_SUPPORT)
		*data = uintdup( pEntry->warnDisDelay );
#else
		*data = uintdup( 0 );
#endif
	}

	else if( strcmp( lastname, "RSIPAvailable" )==0 )
	{	
		*data = uintdup( 0 );
	}
	else if( strcmp( lastname, "NATEnabled" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->napt)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
#else
		*data = booldup( 1 );
#endif//#if defined(MULTI_WAN_SUPPORT)
	}
	else if( strcmp( lastname, "Username" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->pppUsername[0])
			*data = strdup( pEntry->pppUsername );
		else
			*data = strdup( "" );
#else
		mib_get(MIB_PPP_USER_NAME, (void *)buf);
		*data = strdup(buf);
#endif
	}else if( strcmp( lastname, "Password" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->pppPassword[0])
			*data = strdup( pEntry->pppPassword );
		else
		*data = strdup( "" ); /*return an empty string*/
#else
		mib_get(MIB_PPP_PASSWORD, (void*)buf);
		*data = strdup(buf);
#endif //#if defined(MULTI_WAN_SUPPORT)
	}
	else if( strcmp( lastname, "PPPEncryptionProtocol" )==0 )	// Jenny
	{
		*data = strdup( "None" );
	}
	else if( strcmp( lastname, "PPPCompressionProtocol" )==0 )	// Jenny
	{
		*data = strdup( "None" );
	}

	else if( strcmp( lastname, "PPPAuthenticationProtocol" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->pppAuth==PPP_AUTH_PAP)
			*data = strdup( "PAP" );
		else if(pEntry->pppAuth==PPP_AUTH_CHAP)
			*data = strdup( "CHAP" );
		else if(pEntry->pppAuth==PPP_AUTH_AUTO)
			*data = strdup( "PAPandCHAP" );
		else
			return ERR_9002;
#else
                *data = strdup( "PAP, CHAP, PAPandCHAP" );
#endif
	}

	else if( strcmp( lastname, "ExternalIPAddress" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		char *temp=NULL;
		struct in_addr inAddr;
			
		if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("");
#else
		struct in_addr inAddr;
		char *temp=NULL;

		
		if (getInAddr( "ppp0", IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("0.0.0.0");
#endif
	}

	else if( strcmp( lastname, "RemoteIPAddress" )==0 )
	{	
		char *temp=NULL;	
		struct in_addr inAddr;
		
		if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("0.0.0.0");
	}
	else if( strcmp( lastname, "MaxMRUSize" )==0 )	// Jenny
	{	
#if defined(MULTI_WAN_SUPPORT)	
		*data = uintdup( pEntry->pppoeMtu );
#else
int mtu_size;
mib_get( MIB_PPTP_MTU_SIZE, (void *)&mtu_size);
*data = uintdup( mtu_size );
#endif
	}
	else if( strcmp( lastname, "CurrentMRUSize" )==0 )	// Jenny
	{	
#if defined(MULTI_WAN_SUPPORT)
	//NOT Support YET
	//	unsigned int cmru;
	//	if( getPPPCurrentMRU( ifname, pEntry->cmode, &cmru )==0 )
	//		*data = uintdup( cmru );
	//	else
			*data = uintdup( pEntry->pppoeMtu );
#else
int mtu_size;
mib_get( MIB_PPTP_MTU_SIZE, (void *)&mtu_size);
*data = uintdup( mtu_size );
#endif //#if defined(MULTI_WAN_SUPPORT)
	}

	else if( strcmp( lastname, "DNSEnabled" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)	
		*data = booldup( 1 );
#else
		int dns;
		mib_get( MIB_DNS_MODE, (void *)&dns);
		if(dns == 1) //Manual set DNS
			*data = booldup( 0 );
		else
		*data = booldup( 1 );
		
#endif
	}
	else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		vChar=pEntry->dnsAuto;
		if( vChar==1 ) //automatically attain DNS
			*data = booldup( 1 );
		else
		*data = booldup( 0 );
#else
		int dns;
		mib_get( MIB_DNS_MODE, (void *)&dns);
		if(dns == 1) //Manual set DNS
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
		
#endif	
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		vChar=pEntry->dnsAuto;
		if( vChar==1 ) //automatically attain DNS
			*data=strdup("");
		else
	{
			getWANDNSList(buf, pEntry); 		
			*data = strdup( buf );
		}
#else
		int vInt;
		mib_get( MIB_DNS_MODE, (void *)&vInt);
		if( vInt==0 ) //automatically attain DNS
			*data=strdup("");
		else
		{
			getDNSList(buf);			
			*data = strdup( buf );
		}
#endif
	}
	else if( strcmp( lastname, "MACAddress" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		char wanif[IFNAMSIZ];
		unsigned char *pMacAddr;
		struct sockaddr hwaddr;
		ifGetName(PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));
		//printf("%s:%d:wanif=%s\n",__FUNCTION__, __LINE__, wanif);
		if(!getInAddr(wanif, HW_ADDR, (void *)&hwaddr))
			return ERR_9002;
		pMacAddr = (unsigned char *)hwaddr.sa_data;
		sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);

		*data = strdup(buf);
#else
		struct sockaddr hwaddr;
		unsigned char *pMacAddr;

		if(!getInAddr("ppp0", HW_ADDR, (void *)&hwaddr))
			*data = strdup("00:00:00:00:00:00");
		else
		{
			pMacAddr = (unsigned char *)hwaddr.sa_data;
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
	
			*data = strdup(buf);
		}
#endif //#if defined(MULTI_WAN_SUPPORT)
	}
	else if( strcmp( lastname, "TransportType" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		if( pEntry->cmode == IP_PPP )
			*data = strdup("PPPoE");
		else
			*data = strdup("");
#else
		int vInt;
		
		mib_get(MIB_WAN_DHCP, (void *)&vInt);
		if(vInt == PPPOE)
			*data = strdup("PPPoE");
		else
			*data = strdup("");
#endif
	}

	else if( strcmp( lastname, "PPPoEACName" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		if( pEntry->cmode==IP_PPP && pEntry->pppACName[0])
			*data = strdup( pEntry->pppACName );
		else
			*data = strdup( "" );
#else
*data = strdup( "notsupport" );
#endif
	}

	else if( strcmp( lastname, "PPPoEServiceName" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)	
		if( pEntry->cmode==IP_PPP && pEntry->pppServiceName[0] )	// Jenny
			*data = strdup( pEntry->pppServiceName );
		else
			*data = strdup( "" );
#else
char ser_name[200]={0};
mib_get( MIB_PPP_SERVICE_NAME,  (void *)ser_name);
*data = strdup( ser_name );
#endif
	}
	else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)	
#ifdef PPPOE_PASSTHROUGH
		if( pEntry->cmode==IP_BRIDGE )
			*data = strdup( "AlwaysOn" );
		else
#endif
		if( pEntry->pppCtype==CONTINUOUS )
			*data = strdup( "AlwaysOn" );
		else if( pEntry->pppCtype==CONNECT_ON_DEMAND )
			*data = strdup( "OnDemand" );
		else
			*data = strdup( "Manual" );
#else
		PPP_CONNECT_TYPE_T type;
		mib_get(MIB_PPP_CONNECT_TYPE, (void *)&type);
		*data=strdup(buf);
		
		if( type==CONTINUOUS )
			*data = strdup( "AlwaysOn" );
		else if( type==CONNECT_ON_DEMAND )
			*data = strdup( "OnDemand" );
		else
			*data = strdup( "Manual" );
#endif
	}	
	else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{	
		return 0;
	}	
	else if( strcmp( lastname, "PPPLCPEcho" )==0 )	// Jenny
	{	
#if defined(MULTI_WAN_SUPPORT)
		//NOT Support YET
		//unsigned int echo;
		
		//if( getPPPLCPEcho( ifname, pEntry->cmode, &echo )==0 )
		//	*data = uintdup( echo );
		//else
#endif
			*data = uintdup( 0 );

	}


	else if( strcmp( lastname, "PPPLCPEchoRetry" )==0 )	// Jenny
	{	
#if defined(MULTI_WAN_SUPPORT)	
		//NOT Support YET
		//unsigned int retry;
		//if( getPPPEchoRetry( ifname, pEntry->cmode, &retry )==0 )
		//	*data = uintdup( retry );
		//else
#endif
			*data = uintdup( 0 );

	}

#ifdef _PRMT_X_CT_COM_PPPOEv2_
	else if( strcmp( lastname, "X_CT-COM_ProxyEnable" )==0 )
	{
		if(pEntry->PPPoEProxyEnable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "X_CT-COM_MAXUser" )==0 )
	{
		*data = uintdup( pEntry->PPPoEProxyMaxUser );
	}
#endif //_PRMT_X_CT_COM_PPPOEv2_

#ifdef _PRMT_X_CT_COM_WANEXT_
	else if( strcmp( lastname, "X_CT-COM_LanInterface" )==0 )
	{
		char tmp[1024];
		getLanInterface(  chainid, tmp);
		*data = strdup( tmp );

	}
	
	else if( strcmp( lastname, "X_CT-COM_ServiceList" )==0 )
	{
		convertFlag2ServiceList( pEntry->ServiceList, buf );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "X_CT-COM_IPMode" ) == 0 )
	{
#ifdef CONFIG_IPV6
		switch(pEntry->IpProtocol)
		{
			case 0:
				*data = uintdup(2);
				break;
			case 1:
				*data = uintdup(0);
				break;
			case 2:
				*data = uintdup(1);
				break;
			default:
			return ERR_9003;
			break;
		}
#else
		*data = uintdup(0);
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6ConnectionStatus" ) == 0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface-DHCPEnable" )==0 )
	{
			if(!pEntry->enableLanDhcp)
				*data=booldup(0);
			else
				*data = booldup(1);
	}

#endif //_PRMT_X_CT_COM_WANEXT_
#if defined (CONFIG_ETHWAN)
	else if( strcmp( lastname, "X_CT-COM_MulticastVlan" )==0 )
	{
			*data = intdup(pEntry->multicastVlan);
	}
#endif
	else if( strcmp( lastname, "PortMappingNumberOfEntries" )==0 )
	{	
		int portEntityCount;
		if(getPortMappingCount(&portEntityCount) == 0) //o:OK
			*data = uintdup( portEntityCount );
		else
			*data = uintdup( 0 );
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
int gStartReset=0;
static int resetThread=0;
static int resetChainID=-1;
const char DHCPC_SCRIPT[] = "/etc/scripts/udhcpc.sh";
const char DHCPC_SCRIPT_NAME[] = "/var/udhcpc/udhcpc";
const char DHCPC_PID[] = "/var/run/udhcpc.pid";

// calculate the 15-0(bits) Cell Rate register value (PCR or SCR)
// return its corresponding register value
static int cr2reg(int pcr)
{
	int k, e, m, pow2, reg;
	
	k = pcr;
	e=0;
	
	while (k>1) {
		k = k/2;
		e++;
	}
	
	//printf("pcr=%d, e=%d\n", pcr,e);
	pow2 = 1;
	for (k = 1; k <= e; k++)
		pow2*=2;
	
	//printf("pow2=%d\n", pow2);
	//m = ((pcr/pow2)-1)*512;
	k = 0;
	while (pcr >= pow2) {
		pcr -= pow2;
		k++;
	}
	m = (k-1)*512 + pcr*512/pow2;
	//printf("m=%d\n", m);
	reg = (e<<9 | m );
	//printf("reg=%d\n", reg);
	return reg;
}

static void *reset_thread(void *arg) {
	MIB_CE_ATM_VC_T *pEntry,Entry;
	char wanif[5], ifIdx[3], pppif[6],vpivci[6],qosParms[64],cmdbuf[256] ;
	int pcreg,screg;

	pEntry = &Entry;
	//printf("%s:%d resetChainID=%d\n",__FUNCTION__,__LINE__,resetChainID);
	if( !mib_chain_get( MIB_ATM_VC_TBL, resetChainID, (void*)pEntry ) )
		return;
	snprintf(wanif, 5, "vc%d", VC_INDEX(pEntry->ifIndex));
	snprintf(ifIdx, 3, "%u", PPP_INDEX(pEntry->ifIndex));
	snprintf(pppif, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	if ((pEntry->cmode == ADSL_MER1483) && (pEntry->ipDhcp == DHCP_CLIENT))
		delDhcpcOption(pEntry->ifIndex);
#endif

	if (pEntry->cmode == ACC_BRIDGED)
	{
		aaaaaaaaaaaaaa;
		va_cmd(IFCONFIG,2,1,wanif,"down");
		va_cmd(IFCONFIG,2,1,wanif,"up");
	}
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_MER
		||(REMOTE_ACCESS_T)pEntry->cmode == ACC_ROUTED
		#ifdef CONFIG_ATM_CLIP
		||(REMOTE_ACCESS_T)pEntry->cmode == ACC_IPOA
		#endif
		)
	{
		if(pEntry->ipDhcp != (char)DHCP_DISABLED)
		{
			int dhcpcpid;
			unsigned char infname[6],pidfile[32];
			
			snprintf(pidfile, 32, "/var/run/udhcpc.pid", wanif);
			dhcpcpid = getPid(pidfile);
			if(dhcpcpid > 0)
				kill(dhcpcpid, 15);
			
			bbbbbbbbbbbbbbbb;	
			va_cmd("/bin/udhcpc",6,0,
				"-i",wanif,
				"-p",DHCPC_PID,
				"-s",DHCPC_SCRIPT
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
				,"-w",pEntry->ifIndex
#endif
				);
		}
		else
		{
			cccccccccccccccccc;
			va_cmd(IFCONFIG,2,1,wanif,"down");
			va_cmd(IFCONFIG,2,1,wanif,"up");
		}
	}
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOE)
	{
		if (pEntry->pppCtype != MANUAL)
		{
			dddddddddddddddddddddddd;
			// spppctl add 0 pppoe vc0 username USER password PASSWORD gw 1 mru xxxx acname xxx
			va_cmd("/bin/spppctl",14,0,
				"add",ifIdx,
				"pppoe",wanif,
				"username",pEntry->pppUsername,
				"password",pEntry->pppPassword,
				"gw",pEntry->dgw,
				"mru",pEntry->mtu,
				"acname",pEntry->pppACName);
		}	
		else
		{
			va_cmd(IFCONFIG,2,1,pppif,"down");
			va_cmd(IFCONFIG,2,1,pppif,"up");
		}
	}
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOA)
	{
		if (pEntry->pppCtype != MANUAL)
		{
			pcreg = cr2reg(pEntry->pcr);
			sprintf(vpivci,"%u.%u",pEntry->vpi, pEntry->vci);
			if ((ATM_QOS_T)pEntry->qos == ATMQOS_CBR)
			{
				snprintf(qosParms, 64, "cbr:pcr=%u", pcreg);
			}
			else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_NRT)
			{
				screg = cr2reg(pEntry->scr);
				snprintf(qosParms, 64, "nrt-vbr:pcr=%u,scr=%u,mbs=%u",
					pcreg, screg, pEntry->mbs);
			}
			else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_RT)
			{
				screg = cr2reg(pEntry->scr);
				snprintf(qosParms, 64, "rt-vbr:pcr=%u,scr=%u,mbs=%u",
					pcreg, screg, pEntry->mbs);
			}
			else //if ((ATM_QOS_T)pEntry->qos == ATMQOS_UBR)
			{
				snprintf(qosParms, 64, "ubr:pcr=%u", pcreg);
			}

			//spppctl add 0 pppoa vpi.vci encaps ENCAP qos xxx  username USER password PASSWORD gw 1 mru xxxx
			va_cmd("/bin/spppctl",16,0,
				"add",ifIdx,
				"pppoa",vpivci,
				"encaps",pEntry->encap,
				"qos",qosParms,
				"username",pEntry->pppUsername,
				"password",pEntry->pppPassword,
				"gw",pEntry->dgw,
				"mru",pEntry->mtu);
		}	
		else
		{
			va_cmd(IFCONFIG,2,1,pppif,"down");
			va_cmd(IFCONFIG,2,1,pppif,"up");
		}
	}

END:
	resetThread=0;
}

void cwmpStartReset() {
	pthread_t reset_pid;

	//printf("%s:%d resetThread=%d,resetChainID=%d\n",__FUNCTION__,__LINE__,resetThread,resetChainID);
	if (resetThread) {
		//printf("reset in progress, try again later=\n");
		return;
	}

	resetThread = 1;
	if( pthread_create( &reset_pid, NULL, reset_thread, 0 ) != 0 )
	{
		resetThread = 0;
		return;
	}
	pthread_detach(reset_pid);
	
}
#endif// end of _PRMT_WT107_
/*ping_zhang:20081217 END*/
int setWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	char	*tok;
	unsigned int	chainid;
#if defined(MULTI_WAN_SUPPORT)	
	WANIFACE_T *pEntry, wan_entity;
	WANIFACE_T target[2];
#endif
	unsigned int devnum,pppnum;
	
	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	devnum = getWANConDevInstNum( name );
	pppnum  = getWANPPPConInstNum( name );
	if( (devnum==0) || (pppnum==0) ) return ERR_9005;

#if defined(MULTI_WAN_SUPPORT)	
	pEntry = &wan_entity;
	memset( &target[0], 0, sizeof( WANIFACE_T ) );
	memset( &target[1], 0, sizeof( WANIFACE_T ) );
	if( getATMVCEntryByPPPInstNum( devnum, pppnum, pEntry, &chainid )<0 ) 
		return ERR_9002;

	memcpy(&target[0], pEntry, sizeof(WANIFACE_T));
#endif
	if( strcmp( lastname, "Enable" )==0 )
	{

		int *i=data;
		unsigned int vInt=0;
		unsigned int vChar=0;
		
		if(i==NULL) return ERR_9007;
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)		
		if(*i == 1){
			vChar = 0;
			mib_set(MIB_CWMP_PPPOE_DISABLED, (void *)&vChar);	
		}
		else if (*i == 0){
			vChar = 1;
			mib_set(MIB_CWMP_PPPOE_DISABLED, (void *)&vChar);	
		}
#else
		vInt = PPPOE;			
		mib_set(MIB_WAN_DHCP, (void *)&vInt);
#endif		
		return 1;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Reset" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i==1) 
		{
			resetChainID = chainid;
			gStartReset = 1;
		}
		return 0;
#endif
/*ping_zhang:20081217 END*/
	}
	else if( strcmp( lastname, "ConnectionType" )==0 )
	{
		unsigned int vInt=0;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9001;		
			
		if(strstr(buf,"PPPoE") != NULL || strstr(buf,"IP_Routed") != NULL) //IP_Routed == PPPoE ??????
			vInt = PPPOE;
		else
			return ERR_9005;
			
		mib_set(MIB_WAN_DHCP, (void *)&vInt);
		
		return 1;
	}
	else if( strcmp( lastname, "Name" )==0 )
	{

		return 0;
	}	

	else if( strcmp( lastname, "AutoDisconnectTime" )==0 )
	{
		unsigned int *idletime=data;

		if(idletime==NULL) return ERR_9007;
		if(*idletime > 0 && *idletime < 1001)
			mib_set(MIB_PPP_IDLE_TIME, (void *)idletime);
		return 1;
	}

	else if( strcmp( lastname, "IdleDisconnectTime" )==0 )
	{
		unsigned int *idletime=data;

		if(idletime==NULL) return ERR_9007;
		if(*idletime > 0 && *idletime < 1001)
			mib_set(MIB_PPP_IDLE_TIME, (void *)idletime);
		return 1;
	}

	else if( strcmp( lastname, "WarnDisconnectDelay" )==0 )	// Jenny
	{
#if 0 //not support
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *disconndelay = &tmpuint;
#else
		unsigned int *disconndelay = data;
#endif
		if(disconndelay==NULL) return ERR_9007;
		pEntry->warnDisDelay = (unsigned short) (*disconndelay);
		memcpy(&target[1], pEntry, sizeof(WANIFACE_T));
		mib_set(MIB_WANIFACE_MOD, (void *)&target);
		return 1;
#endif
return 0;
	}
	else if( strcmp( lastname, "NATEnabled" )==0 )
	{
		return 0;
	}
	else if( strcmp( lastname, "Username" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		mib_set(MIB_PPP_USER_NAME, (void*)buf);
		return 1;
	}else if( strcmp( lastname, "Password" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		mib_set(MIB_PPP_PASSWORD, (void*)buf);
		return 1;
	}
	else if( strcmp( lastname, "MaxMRUSize" )==0 )	// Jenny
	{
		unsigned int *mru = data;
		unsigned mtu_size = 0;
		
		if (mru==NULL) return ERR_9007;
		if (*mru<1)	*mru = 1;
		else if (*mru>1540)	*mru = 1540;

		mtu_size = *mru;
		mib_set( MIB_PPTP_MTU_SIZE, (void *)&mtu_size);
		return 1;
	}
	else if( strcmp( lastname, "DNSEnabled" )==0 )
	{
		int *i=data;
		int dns;
		
		if(i==NULL) return ERR_9007;
		
		if(*i ==0)
			dns=1; //manual mode
		else if(*i ==1)
			dns=0; //auto mode

		mib_set( MIB_DNS_MODE, (void *)&dns);
		return 1;	
			
	}
	else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{	
		int *i=data;
		int dns;
			
		if(i==NULL) return ERR_9007;
		
		if(*i ==0)
			dns=1; //manual mode
		else if(*i ==1)
			dns=0; //auto mode

		mib_set( MIB_DNS_MODE, (void *)&dns);
		return 1;	
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{	
		int vInt = 0;
		if( (buf==NULL) || (strlen(buf)==0) )
		{	//automatically attain DNS
			vInt = 0; //automatically
		}
		else
		{
			if( setDNSList( buf ) == 0 )
			{
				vInt = 1;

			}

		}
		mib_set( MIB_DNS_MODE, (void *)&vInt);


			
		return 1;			
	}
	else if( strcmp( lastname, "PPPoEACName" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		mib_set( MIB_PPP_SERVICE_NAME,  (void *)buf);
		return 1;

	}
	else if( strcmp( lastname, "PPPoEServiceName" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		mib_set( MIB_PPP_SERVICE_NAME,  (void *)buf);
		return 1;
	}else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
		PPP_CONNECT_TYPE_T type;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
			
		if( strcmp( buf, "AlwaysOn" )==0 )
			type = CONTINUOUS;
		else if( strcmp( buf, "OnDemand" )==0 )
			type = CONNECT_ON_DEMAND;
		else if( strcmp( buf, "Manual" )==0 )
			type = MANUAL;
		else
			return ERR_9007;
			
		mib_set(MIB_PPP_CONNECT_TYPE, (void *)&type);
		return 1;
	}
	else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{
		return 0;
	
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int objWANPPPConn(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	char	*tok;	
#if defined(MULTI_WAN_SUPPORT)
	WANIFACE_T *pEntry, wan_entity;
	WANIFACE_T target[2];
#endif
	unsigned int devnum;
	unsigned int num=0,i,maxnum=0,chainid=0;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	if( (name==NULL) || (entity==NULL) ) return -1;

	devnum = getWANConDevInstNum( name );
	if(devnum==0) {
		//CWMPDBG( 2, ( stderr, "<%s:%d>devnum:%d\n", __FUNCTION__, __LINE__, devnum) );
		return ERR_9005;
		}

	switch( type )
	{		
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;
			
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

#if defined(MULTI_WAN_SUPPORT)
			maxnum = findMaxPPPConInstNum( devnum );
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			//CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tINITOBJ:maxnum:%d,num=%d\n", __FUNCTION__, __LINE__, maxnum,num) );
			
			for( i=1; i<=num;i++ )
			{
				pEntry = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				if( (pEntry->ConDevInstNum==devnum) &&
					(pEntry->connDisable==0) )
				{
					if( (pEntry->cmode == IP_BRIDGE) ||(pEntry->cmode == IP_PPP)
	#ifdef PPPOE_PASSTHROUGH
					|| ( (pEntry->cmode==IP_BRIDGE)&&(pEntry->brmode==BRIDGE_PPPOE) ) 
	#endif
					)
					{
				if( create_Object( ptable, tWANPPPCONObject, sizeof(tWANPPPCONObject), 1, pEntry->ConPPPInstNum ) < 0 )
				return -1;
						//CWMPDBG( 1, ( stderr, "<%s:%d>add conppp:%d\n", __FUNCTION__, __LINE__, pEntry->ConPPPInstNum ) );
					}//if
				}//if
			}//for

			add_objectNum( name, maxnum );
#else //#if defined(MULTI_WAN_SUPPORT)
			int wan_dhcp;
			int cwmp_pppconn_instnum;
			int cwmp_pppconn_created;
			
			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
			mib_get( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);
			mib_get( MIB_CWMP_PPPCON_INSTNUM, (void *)&cwmp_pppconn_instnum);

			if(wan_dhcp == PPPOE)			
			{
				if(cwmp_pppconn_instnum == 0)
					cwmp_pppconn_instnum = 1;

				if( create_Object( ptable, tWANPPPCONObject, sizeof(tWANPPPCONObject), 1, cwmp_pppconn_instnum ) < 0 )
					return -1;
#ifdef CUSTOMIZE_MIDDLE_EAST
				maxnum = 2; //customize 1 empty ,2 pppoe value 
#else
				maxnum = 1; //only support 1 ppp connection
#endif				
			}
			add_objectNum( name, maxnum );
			mib_set( MIB_CWMP_PPPCON_INSTNUM, (void *)&maxnum);

#endif //#if defined(MULTI_WAN_SUPPORT)			
			
			return 0;
		}
		case eCWMP_tADDOBJ:	     
		{
			int ret;
#if defined(MULTI_WAN_SUPPORT)			
			int cnt=0,found_first=-1, found_nocon=-1;
			WANIFACE_T firstentity, noconentity;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			memset( &target[0], 0, sizeof( WANIFACE_T ) );
			memset( &target[1], 0, sizeof( WANIFACE_T ) );
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			//CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tADDOBJ:num=%d\n", __FUNCTION__, __LINE__, num) );
			for( i=1; i<=num;i++ )
			{
				pEntry = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				
				if(pEntry->ConDevInstNum==devnum)
				{
					cnt++;
					if(cnt==1)
					{
						found_first=i;
						memcpy( &firstentity, pEntry, sizeof(WANIFACE_T) );
						memcpy(&target[0], &wan_entity, sizeof(WANIFACE_T));
					}

					if(pEntry->connDisable==1)
					{
						found_nocon = i;
						memcpy( &noconentity, pEntry, sizeof(WANIFACE_T) );
						memcpy(&target[0], &wan_entity, sizeof(WANIFACE_T));
					}
				}
			}//for
			if(cnt==0) return ERR_9005;
			//connection > MAX_POE_PER_VC or ==MAX_POE_PER_VC with ip/ppp connection
			if( (cnt>MAX_POE_PER_VC) || ((cnt==MAX_POE_PER_VC) && (found_nocon==-1)) )
				return ERR_9004;

			//printf("\nfount=%d %d\n",found_nocon,found_first);
			if( found_nocon!=-1 ) //has one entry without pppconnection
			{
				i = found_nocon;
				pEntry = &noconentity;
			}
			else if( found_first!=-1 )  //create new one
			{
				unsigned int new_ifindex;
				new_ifindex = getNewIfIndex(IP_PPP, firstentity.ConDevInstNum);
				if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;

				i = found_first;
				pEntry = &firstentity;
				//set value to default;
				//pEntry->connDisable=1;
				//pEntry->ConPPPInstNum=0;
				resetATMVCConnection( pEntry );
#ifdef CONFIG_DEV_xDSL
				pEntry->ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(new_ifindex), VC_INDEX(pEntry->ifIndex));
#endif
#if 1//defined CONFIG_ETHWAN
				pEntry->ifIndex = new_ifindex;
#endif
			}else
				return ERR_9002;

			//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( pEntry->connDisable==0 ) /*already has one connection*/
				return ERR_9004;

#if 0 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
			if( (pEntry->cmode != ADSL_PPPoE) && (pEntry->cmode != ADSL_PPPoA) )
				return ERR_9001;
#endif
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPPPCONObject, sizeof(tWANPPPCONObject), data );
			if( ret >= 0 )
			{
				pEntry->connDisable=0;
				pEntry->ConPPPInstNum=*(int*)data;
				//CWMPDBG( 2, ( stderr, "<%s:%d>pEntry->ConPPPInstNum:%d\n", __FUNCTION__, __LINE__, pEntry->ConPPPInstNum) );
				pEntry->ConIPInstNum=0;
				pEntry->enable=0;

#if 1 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
				if( (pEntry->cmode != IP_PPP) )
				{
					unsigned int new_ifindex;
					new_ifindex = getNewIfIndex(IP_PPP, pEntry->ConDevInstNum);
					//CWMPDBG( 2, ( stderr, "<%s:%d>new_ifindex:0x%08X, pEntry->ConDevInstNum=%d\n", __FUNCTION__, __LINE__, new_ifindex,pEntry->ConDevInstNum ) );
					if( (new_ifindex==NA_VC)||(new_ifindex==NA_VC) ) return ERR_9001;
					pEntry->cmode = IP_PPP;
					pEntry->AddressType = PPPOE;
#ifdef CONFIG_DEV_xDSL
					pEntry->ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(new_ifindex), VC_INDEX(pEntry->ifIndex));
#endif
#if 1//defined CONFIG_ETHWAN
					pEntry->ifIndex = new_ifindex;
#endif
					pEntry->mtu = 1492;
					//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
				}
#endif
				if(found_nocon!=-1 ){
					memcpy(&target[1], pEntry, sizeof(WANIFACE_T));
					//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) WANIFACE_MOD\n", __FUNCTION__, __LINE__, name,type ) );
					if ( !mib_set(MIB_WANIFACE_MOD, (void *)&target)) 
					{
							ret=-1;
							//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
					}
				}else{
				//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) WANIFACE_ADD\n", __FUNCTION__, __LINE__, name,type ) );
					if ( !mib_set(MIB_WANIFACE_ADD, (void *)pEntry)) 
	     			{
							ret=-1;
					}
				}	
//#ifdef E8B_NEW_DIAGNOSE
//				writePVCFile(pEntry->vpi, pEntry->vci, "add", "", "");
//#endif
			}
			notify_set_wan_changed();

			if(ret==0) ret=1;

			return ret;
#else
			int wan_dhcp;
			int cwmp_pppconn_instnum;
			int num=0;
			int cwmp_pppconn_created;
			
			mib_get( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);
			mib_get( MIB_CWMP_PPPCON_INSTNUM, (void *)&cwmp_pppconn_instnum);

			num = get_ParameterNameCount(name,1);
			
			if(cwmp_pppconn_created == 1) // ppp connection max is 1
	  			return ERR_9004;

			cwmp_pppconn_created = 1;
			
			//CWMPDBG( 2, ( stderr, "<%s:%d>addobj,\n", __FUNCTION__, __LINE__) );

			if(cwmp_pppconn_instnum == 0)
				cwmp_pppconn_instnum = 1;

			*(unsigned int*)data = cwmp_pppconn_instnum;
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPPPCONObject, sizeof(tWANPPPCONObject), data );
		
			if(ret == 0)
			{
				wan_dhcp = PPPOE;
				mib_set( MIB_WAN_DHCP, (void *)&wan_dhcp);

			mib_set( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);
				cwmp_pppconn_instnum = *(unsigned int*)data;
			mib_set( MIB_CWMP_PPPCON_INSTNUM, (void *)&cwmp_pppconn_instnum);

				notify_set_wan_changed();
			}
			return 1;
#endif //#if defined(MULTI_WAN_SUPPORT)
		}	     
		case eCWMP_tDELOBJ:	     		
		{
			int ret;
#if defined(MULTI_WAN_SUPPORT)
			int ret,cnt=0;
			char s_appname[32];
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			memset( &target[0], 0, sizeof( WANIFACE_T ) );
			memset( &target[1], 0, sizeof( WANIFACE_T ) );
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			for( i=1; i<=num;i++ )
			{
				
				pEntry = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				if(pEntry->ConDevInstNum==devnum)
				{
					cnt++;
					if(pEntry->ConPPPInstNum==*(unsigned int*)data){
						memcpy(&target[0], &wan_entity, sizeof(WANIFACE_T));
						chainid=i;
					}
				}
			}//for
			if(chainid==0) return ERR_9005;

			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )
			{
				//delete port-mappings of this connection.
//				delPortForwarding( pEntry->ifIndex );
//				delRoutingTable( pEntry->ifIndex );
//#ifdef E8B_NEW_DIAGNOSE
//				setWanName(s_appname, pEntry->applicationtype);
//				writePVCFile(pEntry->vpi, pEntry->vci, "del", s_appname,
//						(pEntry->cmode == ADSL_BR1483) ? "桥接" : "路由");
//#endif
				//if the pppconnection is more than 1, delete the chain , not update it.
				if( cnt==1 )
				{
					//pEntry->connDisable=1;
					//reset ppp-related var.
					resetATMVCConnection( pEntry );
					memcpy(&target[1], pEntry, sizeof(WANIFACE_T));
					if ( !mib_set(MIB_WANIFACE_MOD, (void *)&target)) 
	     			{
						ret=-1;
					}
				}else{
					if ( !mib_set(MIB_WANIFACE_DEL, (void *)pEntry)) 
	     			{
							ret=-1;
					}
				}
				ret=1;
			}
			return ret;
			break;
#else //#if defined(MULTI_WAN_SUPPORT)
			int whichOne=*(unsigned int*)data;
			int wan_dhcp;
			int cwmp_pppconn_instnum;
			int cwmp_pppconn_created;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
			
			mib_get( MIB_CWMP_PPPCON_INSTNUM, (void *)&cwmp_pppconn_instnum);

			if(whichOne != cwmp_pppconn_instnum) return ERR_9007; 

			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if(ret == 0)
			{
			wan_dhcp = DHCP_CLIENT;
			mib_set( MIB_WAN_DHCP, (void *)&wan_dhcp);

			
			/* change cwmp_pppconn_created to 0, after reboot the ppp connection will be not created*/
			cwmp_pppconn_created = 0;
			mib_set( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);

			}

			//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tDELOBJ\n", __FUNCTION__, __LINE__, name,type ) );
			return ret;
#endif //#if defined(MULTI_WAN_SUPPORT)
		}
		case eCWMP_tUPDATEOBJ:
		{
#if defined(MULTI_WAN_SUPPORT)
			int has_new=0;
			struct CWMP_LINKNODE *old_table;

			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			//CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tUPDATEOBJ:num=%d\n", __FUNCTION__, __LINE__, num) );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=1; i<=num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				WANIFACE_T *p,wan_entity;

				p = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				if(p->connDisable==1){
					//CWMPDBG( 2, ( stderr, "<%s:%d>devnum:%d, connDisableD\n", __FUNCTION__, __LINE__, devnum) );
					continue;
					}

				if( (p->ConDevInstNum==devnum) && (p->ConPPPInstNum!=0)  )
				{
					remove_entity = remove_SiblingEntity( &old_table, p->ConPPPInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}else{
						unsigned int MaxInstNum=p->ConPPPInstNum;
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPPPCONObject, sizeof(tWANPPPCONObject), &MaxInstNum );
						has_new=1;
					}
				}else{
					//CWMPDBG( 2, ( stderr, "<%s:%d>devnum:%d, ConPPPInstNum=0\n", __FUNCTION__, __LINE__, devnum) );
					continue;
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			if(has_new) notify_set_wan_changed();
			return 0;
#else

			int has_new=0;
			struct CWMP_LINKNODE *old_table;
			int wan_dhcp;
			int num=0;
			int cwmp_pppconn_created, cwmp_pppconn_instnum;
			int ret = 0;


			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
			mib_get( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);
			mib_get( MIB_CWMP_PPPCON_INSTNUM, (void *)&cwmp_pppconn_instnum);

			if( wan_dhcp == PPPOE) //only support 1 ppp connection		
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				old_table = (struct CWMP_LINKNODE *)entity->next;	     	
				entity->next = NULL;
				
				remove_entity = remove_SiblingEntity( &old_table, cwmp_pppconn_instnum );
				if( remove_entity!=NULL )
				{					
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}else{
					unsigned int MaxInstNum=cwmp_pppconn_instnum;
#ifdef CUSTOMIZE_MIDDLE_EAST
					MaxInstNum = 2; //customize 1 empty ,2 pppoe value 
#endif
					ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANPPPCONObject, sizeof(tWANPPPCONObject), &MaxInstNum );
					cwmp_pppconn_instnum = MaxInstNum;				
				}

				if(ret == 0)
				{
					has_new=1;
					mib_set( MIB_CWMP_PPPCON_INSTNUM, (void *)&cwmp_pppconn_instnum);

				cwmp_pppconn_created = 1;
				mib_set( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);
			}
				if( old_table )
					destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			}
			else
			{
				cwmp_pppconn_created = 0;				
				mib_set( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);
				//CWMPDBG( 2, ( stderr, "<%s:%d>devnum:%d, ConPPPInstNum=0\n", __FUNCTION__, __LINE__, devnum) );
			}				

			
			if(has_new) notify_set_wan_changed();
			return 0;
				
#endif //#if defined(MULTI_WAN_SUPPORT)
		}
	}

	return -1;
}


/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.*/
int getWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	//printf("enter getWANIPCONENTITY\n");
	char	*lastname = entity->info->name;
	int	chainid;
#if defined(MULTI_WAN_SUPPORT)
	WANIFACE_T *pEntry,wan_entity;
#endif
	char	ifname[16];
	char	buf[512]="";
	unsigned int devnum,ipnum;
	int vChar=0;
	int wan_dhcp;
	mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);


	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	if( (devnum==0) || (ipnum==0) ) return ERR_9005;

#if defined(MULTI_WAN_SUPPORT)
	pEntry = &wan_entity;
	if( getATMVCEntryByIPInstNum( devnum, ipnum, pEntry, &chainid )<0 ) 
		return ERR_9002;
	if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;
#endif

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)	
		if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
#else
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
		if( (wan_dhcp == DHCP_CLIENT) || (wan_dhcp == DHCP_DISABLED) || (wan_dhcp == PPPOE))
#else
		if(wan_dhcp == DHCP_CLIENT || wan_dhcp == DHCP_DISABLED)
#endif			
			*data = booldup(1);
		else
			*data = booldup(0);
#endif //#if defined(MULTI_WAN_SUPPORT)
	}	
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	else if( strcmp( lastname, "Reset" )==0 )
	{
		/*while read, this parameter always returns False.*/
		*data = booldup(0);
	}	
#endif
/*ping_zhang:20081217 END*/
	else if( strcmp( lastname, "ConnectionStatus" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)	
		int flags;
		
		if(pEntry->enable==0)
			*data = strdup( "Disconnected" );
		else if(getInFlags( ifname, &flags) == 1)
		{	
			if (flags & IFF_UP)
				*data = strdup( "Connected" );
			else
				*data = strdup( "Disconnected" );
		}else
					*data = strdup( "Disconnected" );
#else
		char *temp=NULL;		

#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
		if( (wan_dhcp == DHCP_CLIENT) || (wan_dhcp == DHCP_DISABLED) || (wan_dhcp == PPPOE))
#else
		if(wan_dhcp == DHCP_CLIENT || wan_dhcp == DHCP_DISABLED)
#endif			
		{
			struct in_addr inAddr;
			if (getInAddr( "eth1", IP_ADDR, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);

			if(temp && strcmp(temp,"0.0.0.0")!=0)
				*data = strdup( "Connected" );
			else
				*data = strdup( "Disconnected" );

		}
		else
			*data = strdup( "Disconnected" );			
		
#endif //#if defined(MULTI_WAN_SUPPORT)
		
	}
	else if( strcmp( lastname, "PossibleConnectionTypes" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)	
		*data = strdup( "IP_Routed,IP_Bridged" );
#else
		*data = strdup( "IP_Routed" );
#endif
	}
	else if( strcmp( lastname, "ConnectionType" )==0 )
	{	

#if defined(MULTI_WAN_SUPPORT)
		if (pEntry->cmode == IP_BRIDGE)
			*data = strdup("IP_Bridged");
		else if (pEntry->cmode == IP_ROUTE)
		*data = strdup("IP_Routed");		
		else
			return ERR_9002;
#else
		*data = strdup("IP_Routed");	
#endif
	}

	else if( strcmp( lastname, "Name" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)	
		if(*(pEntry->WanName))
			*data = strdup( pEntry->WanName );
#ifdef CTC_WAN_NAME // Magician: obsoleted due to Name is the same in the field DestinationAddress.
		else		
		{//if not set by ACS. then generate automaticly.  
			char wanname[40];
			memset(wanname, 0, sizeof(wanname));
			generateWanName(pEntry, wanname);
			*data = strdup( wanname );
		}
#endif
#else
		*data = strdup( "wanname" );
#endif
	}
	
	else if( strcmp( lastname, "Uptime" )==0 )
	{	
#if 0
		struct sysinfo info;

		sysinfo(&info);
		*data = uintdup( info.uptime );
#else
		//*data = time(NULL) ;
		unsigned long sec;
		sec = (unsigned long)get_uptime();
		*data = uintdup( sec );
#endif
	}
	
	else if( strcmp( lastname, "LastConnectionError" )==0 )
	{	
		*data = strdup( "ERROR_NONE" );
	}

	else if( strcmp( lastname, "RSIPAvailable" )==0 )
	{	
		*data = booldup( 0 );
	}
	else if( strcmp( lastname, "NATEnabled" )==0 )
	{	
		*data = booldup( 1 );		
	}
	else if( strcmp( lastname, "AddressingType" )==0 )
	{
		
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->AddressType == (char)DHCP_DISABLED)
			*data = strdup( "Static" );
		else
			*data = strdup( "DHCP" );
#else
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
		if(wan_dhcp == DHCP_CLIENT || wan_dhcp == PPPOE)
#else
		if(wan_dhcp == DHCP_CLIENT)
#endif			
			*data = strdup( "DHCP" );
		else if(wan_dhcp == DHCP_DISABLED)
			*data = strdup( "Static" );
		else
			*data = strdup( "" );
#endif
	}
	else if( strcmp( lastname, "ExternalIPAddress" )==0 )
	{
		char *temp=NULL;		
		struct in_addr inAddr;
#if defined(MULTI_WAN_SUPPORT)		
		if(pEntry->AddressType == (char)DHCP_DISABLED)
			temp = inet_ntoa(*((struct in_addr *)pEntry->ipAddr));
		else			
			if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);
#else
		//if(wan_dhcp == DHCP_CLIENT || wan_dhcp == DHCP_DISABLED)
		{
			if (getInAddr( "eth1", IP_ADDR, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);
		}
#endif
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("0.0.0.0");
			
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{	
		char *temp=NULL;
		struct in_addr inAddr;

#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->AddressType == (char)DHCP_DISABLED)
			temp = inet_ntoa(*((struct in_addr *)pEntry->netMask));
		else
			if (getInAddr( ifname, SUBNET_MASK, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);
#else
		if(wan_dhcp == DHCP_CLIENT || wan_dhcp == DHCP_DISABLED)
		{
			if (getInAddr( "eth1", SUBNET_MASK, (void *)&inAddr) == 1)
				temp = inet_ntoa(inAddr);
		}
#endif
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("0.0.0.0");
	}
	else if( strcmp( lastname, "DefaultGateway" )==0 )
	{	
		char *temp=NULL;
		struct in_addr inAddr;
		char route[32];
		//printf("wan_dhcp=%d\n", wan_dhcp);
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->AddressType == (char)DHCP_DISABLED)
			temp = inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr));
		else
			if(cwmp_getDefaultRoute(ifname, &inAddr) )
				temp = inet_ntoa(inAddr);
#else
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
		if((wan_dhcp == DHCP_CLIENT) || (wan_dhcp == DHCP_DISABLED) || (wan_dhcp == PPPOE))
#else
		if(wan_dhcp == DHCP_CLIENT || wan_dhcp == DHCP_DISABLED)
#endif
		{
			if(getDefaultRoute("eth1", route, wan_dhcp) )
				temp = route;
		}
#endif
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("0.0.0.0");
	}
	else if( strcmp( lastname, "DNSEnabled" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		vChar=pEntry->dnsAuto;
		if( vChar==0 ) //Manual set DNS
			*data = booldup( 0 );
		else
		*data = booldup(1);
#else
		int dns;
		mib_get( MIB_DNS_MODE, (void *)&dns);
		if(dns == 1) //Manual set DNS
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
		
#endif
	}
	else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{	
		/* TR098A2:Whether or not a manually set, non-empty DNS address can be overridden by a DNS entry received from the WAN. */
#if defined(MULTI_WAN_SUPPORT)
		vChar=pEntry->dnsAuto;
		if( vChar==0 ) //Manual set DNS
		*data = booldup( 0 );
		else
			*data = booldup( 1 );
#else
		int dns;
		mib_get( MIB_DNS_MODE, (void *)&dns);
		if(dns == 1) //Manual set DNS
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
		
#endif		
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{	
#if defined(MULTI_WAN_SUPPORT)
		vChar=pEntry->dnsAuto;
		if( vChar==1 ) //automatically attain DNS
			*data=strdup("");
		else
		{
			getWANDNSList(buf, pEntry);			
			*data = strdup( buf );
		}
#else
		int dns;
		mib_get( MIB_DNS_MODE, (void *)&dns);
		if(dns == 0) //Auto get DNS
			*data=strdup("");
		else
		{
			getDNSList(buf);			
			*data = strdup( buf );
		}
#endif
	}
	else if( strcmp( lastname, "MaxMTUSize" )==0 )
	{
#if defined(MULTI_WAN_SUPPORT)
		if(pEntry->AddressType ==DHCP_CLIENT)
			*data = uintdup(pEntry->dhcpMtu);
		if(pEntry->AddressType ==DHCP_DISABLED)
			*data = uintdup(pEntry->staticIpMtu);
#else
		int mtu_size;
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
		if( (wan_dhcp == DHCP_CLIENT) || (wan_dhcp == PPPOE))
#else		
		if(wan_dhcp == DHCP_CLIENT)
#endif			
			mib_get( MIB_DHCP_MTU_SIZE, (void *)&mtu_size);
		else if(wan_dhcp == DHCP_DISABLED)
			mib_get( MIB_FIXED_IP_MTU_SIZE, (void *)&mtu_size);
		else
			mtu_size = 0;
		*data = uintdup(mtu_size);
#endif		
	}
	else if( strcmp( lastname, "MACAddress" )==0 )
	{	
		
		char wanif[IFNAMSIZ];
		unsigned char *pMacAddr;
		struct sockaddr hwaddr;

#if defined(MULTI_WAN_SUPPORT)	
		ifGetName(PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));
#else
		sprintf(wanif,"%s","eth1");
#endif
		if(!getInAddr(wanif, HW_ADDR, (void *)&hwaddr))
			*data = strdup("00:00:00:00:00:00");
		else
		{
			pMacAddr = (unsigned char *)hwaddr.sa_data;
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
				
			*data = strdup(buf);
		}
		
	}
	else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{	
		/*For IP Connection, the connection type is always-on, which is unsetable*/
		*data = strdup( "AlwaysOn" );
	}
	else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{
//		*data=strdup("");
#if 0 //NOT Support Yet for waniface entry	--20120102
#ifdef _USE_NEW_RIP_
		unsigned char ripVer=RIP_NONE;
		getRIPInfo( pEntry->ifIndex, &ripVer );
		switch( ripVer )
		{
		case RIP_NONE:	
			*data=strdup( "Off" );
			break;
		case RIP_V1:
			*data=strdup( "RIPv1" );
			break;
		case RIP_V2:
			*data=strdup( "RIPv2" );
			break;
		case RIP_V1_V2:
			*data=strdup( "RIPv1andRIPv2" );
			break;
		default:
			return ERR_9002;
		}		
#else
		unsigned char ripOn=0, ripVer=1;
		mib_get(MIB_RIP_ENABLE, (void *)&ripOn);
		if( (ripOn==0) || (pEntry->rip==0) )
			*data = strdup( "Off" );
		else{
			mib_get(MIB_RIP_VERSION, (void *)&ripVer);
			//0:RIPv1, 1:RIPv2
			if(ripVer==0)
				*data = strdup( "RIPv1" );
			else if(ripVer==1)
				*data = strdup( "RIPv2" );
			else
				return ERR_9002;
		}
#endif

#else
		int ripVersion=0;
		mib_get(MIB_RIP_ENABLED, (void *)&ripVersion);
		if(ripVersion==0)
			*data = strdup("Off");
		else if(ripVersion==1)
			*data = strdup("RIPv1");
		else if(ripVersion==2)
			*data = strdup("RIPv2");
#endif 	//end if 0	
	}
#ifdef _PRMT_X_CT_COM_WANEXT_
	else if( strcmp( lastname, "X_CT-COM_LanInterface" )==0 )
	{
		char tmp[1024];
		getLanInterface(chainid, tmp);
		*data = strdup( tmp );

	}
	else if( strcmp( lastname, "X_CT-COM_ServiceList" )==0 )
	{
		convertFlag2ServiceList( pEntry->ServiceList, buf );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "X_CT-COM_IPMode" ) == 0 )
	{
#ifdef CONFIG_IPV6
			switch(pEntry->IpProtocol)
			{
				case 0:
					*data = uintdup(2);
					break;
				case 1:
					*data = uintdup(0);
					break;
				case 2:
					*data = uintdup(1);
					break;
				default:
					return ERR_9003;
					break;
			}
#else
			*data = uintdup(0);
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_IPv6ConnectionStatus" ) == 0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "X_CT-COM_LanInterface-DHCPEnable" )==0 )
	{
			if(!pEntry->enableLanDhcp)
				*data=booldup(0);
			else
				*data=booldup(1);
	}
#endif //_PRMT_X_CT_COM_WANEXT_
#if defined (CONFIG_ETHWAN)
	else if( strcmp( lastname, "X_CT-COM_MulticastVlan" )==0 )
	{
			*data = intdup(pEntry->multicastVlan);
	}
#endif
	else if( strcmp( lastname, "PortMappingNumberOfEntries" )==0 )
	{
		int portEntityCount;
		if(getPortMappingCount(&portEntityCount) == 0){ //o:OK
			*data = uintdup( portEntityCount );
		}else
			*data = uintdup( 0 );
	}
	else{
		return ERR_9005;
	}
	return 0;
}

int setWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int	chainid;
#if defined(MULTI_WAN_SUPPORT)	
	WANIFACE_T *pEntry, wan_entity;
	WANIFACE_T target[2];
#endif
	unsigned int devnum,ipnum;
	int wah_dhcp;

	
	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif


	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	if( (devnum==0) || (ipnum==0) ) return ERR_9005;
		
#if defined(MULTI_WAN_SUPPORT)	
	pEntry = &wan_entity;
	memset( &target[0], 0, sizeof( WANIFACE_T ) );
	memset( &target[1], 0, sizeof( WANIFACE_T ) );
	if( getATMVCEntryByIPInstNum( devnum, ipnum, pEntry, &chainid )<0 ) 
		return ERR_9002;

	memcpy(&target[0], pEntry, sizeof(WANIFACE_T));
#endif

#ifdef CUSTOMIZE_MIDDLE_EAST 
	extern unsigned int set_param_wan_dhcp_flag ;//parameter wan dhcp changed
	set_param_wan_dhcp_flag = 1;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned int vChar=0;
		
		if(i==NULL) 
			return ERR_9007;
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)		
		if(*i == 1){
			vChar = 0;
			mib_set(MIB_CWMP_DHCP_DISABLED, (void *)&vChar);	
		}
		else if	(*i == 0){
			vChar = 1;
			mib_set(MIB_CWMP_DHCP_DISABLED, (void *)&vChar);	
		}
#else
		vChar = DHCP_CLIENT;
		mib_set(MIB_WAN_DHCP, (void *)&vChar);
#endif			
		return 1;
	}
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_	
	else if( strcmp( lastname, "Reset" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i==1) 
		{
			resetChainID = chainid;
			gStartReset = 1;
		}
		return 0;
	}
#endif
/*ping_zhang:20081217 END*/	
	else if( strcmp( lastname, "ConnectionType" )==0 )
	{		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		
		if( strcmp( buf, "IP_Routed" )==0 )
		{

			return 1;
                }
                else
		         return ERR_9007;
	}
	else if( strcmp( lastname, "Name" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		return 0;
	}
	else if( strcmp( lastname, "NATEnabled" )==0 )
	{
		return 0;
	}
	else if( strcmp( lastname, "AddressingType" )==0 )
	{
		unsigned int vChar=0;
		
		if( buf==NULL ) 
			return ERR_9007;
		if( strlen(buf)==0 ) 
			return ERR_9007;
		if( strcmp( buf, "DHCP" )==0 )
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
			vChar = PPPOE;
#else
			vChar = DHCP_CLIENT;
#endif
		else if( strcmp( buf, "Static" )==0 )
			vChar =  DHCP_DISABLED;
		else
			return ERR_9007;
		mib_set(MIB_WAN_DHCP, (void*)&vChar);
		return 1;		
	}
	else if( strcmp( lastname, "ExternalIPAddress" )==0 )
	{
		unsigned int vChar=0;
		struct in_addr in;
		
		if( buf==NULL ) 
			return ERR_9007;
		if( strlen(buf)==0 ) 
			return ERR_9007;

		if(  inet_aton( buf, &in )==0  ) 
			return ERR_9007;
		mib_set(MIB_WAN_IP_ADDR, (void *)&in);	
		return 1;		
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		unsigned int vChar=0;
		struct in_addr in;
		
		if( buf==NULL ) 
			return ERR_9007;
		if( strlen(buf)==0 ) 
			return ERR_9007;
		if(  inet_aton( buf, &in )==0  ) 
			return ERR_9007;
		mib_set(MIB_WAN_SUBNET_MASK, (void *)&in);	

		return 1;	
	}
	else if( strcmp( lastname, "DefaultGateway" )==0 )
	{
		unsigned int vChar=0;
		struct in_addr in;
		
		if( buf==NULL ) 
			return ERR_9007;
		if( strlen(buf)==0 ) 
			return ERR_9007;
		if(  inet_aton( buf, &in )==0  ) 
			return ERR_9007;
		mib_set(MIB_WAN_DEFAULT_GATEWAY, (void *)&in);	

		return 1;
	}
	else if( strcmp( lastname, "MaxMTUSize" )==0 )
	{
		unsigned int vChar=0;
		unsigned int *newmtu=(unsigned int *)data;
		mib_get(MIB_WAN_DHCP, (void*)&vChar);

		
		if(*newmtu > 1399 && *newmtu < 1501)
		{
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
		if( (vChar == DHCP_CLIENT) || (vChar == PPPOE))
#else		
		if(vChar == DHCP_CLIENT)
#endif			
			mib_set(MIB_DHCP_MTU_SIZE, (void *)newmtu);
		else if(vChar == DHCP_DISABLED)
			mib_set( MIB_FIXED_IP_MTU_SIZE, (void *)newmtu);			
		}
		else
			return ERR_9007;
		return 1;
	}
	else if( strcmp( lastname, "DNSEnabled" )==0 )
	{
		int *i=data;
		int dns;
		
		if(i==NULL) return ERR_9007;

		if(*i ==0)
			dns=1; //manual mode
		else if(*i ==1)
			dns=0; //auto mode

		mib_set( MIB_DNS_MODE, (void *)&dns);
		return 1;

	}
	else if( strcmp( lastname, "DNSOverrideAllowed" )==0 )
	{	
		int *i=data;
		int dns;
		
		if(i==NULL) return ERR_9007;
		
		if(*i ==0)
			dns=1; //manual mode
		else if(*i ==1)
			dns=0; //auto mode

		mib_set( MIB_DNS_MODE, (void *)&dns);
		return 1;
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		int vInt = 0;
		if( (buf==NULL) || (strlen(buf)==0) )
		{	//automatically attain DNS
			vInt = 0; //automatically
			}
		else
		{
			if( setDNSList( buf ) == 0 )
			{
				vInt = 1;

		}

		}
		mib_set( MIB_DNS_MODE, (void *)&vInt);


		
		return 1;	
		}
	else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "AlwaysOn" )!=0 ) return ERR_9007;
		}
	else if( strcmp( lastname, "RouteProtocolRx" )==0 )
	{
		return 0;
}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int objWANIPConn(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
#if defined(MULTI_WAN_SUPPORT)
	WANIFACE_T *pEntry, wan_entity;
	WANIFACE_T target[2];
#endif
	unsigned int devnum;
	unsigned int num=0,i,maxnum=0;
	int Found=0;
	//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	devnum = getWANConDevInstNum( name );
	if(devnum==0) return ERR_9005;

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;
			
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

#if defined(MULTI_WAN_SUPPORT)
			maxnum = findMaxIPConInstNum( devnum );
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			for( i=1; i<=num;i++ )
			{
				pEntry = &wan_entity;

				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				
				if( (pEntry->ConDevInstNum==devnum) && (pEntry->connDisable==0) )
				{
						if( (pEntry->cmode == IP_ROUTE) ||(pEntry->cmode == IP_BRIDGE) 
#ifdef PPPOE_PASSTHROUGH
							    || ( (pEntry->cmode==IP_BRIDGE)&&(pEntry->brmode!=BRIDGE_PPPOE) )  
#endif
						    )
						{
								if( create_Object( ptable, tWANIPCONObject, sizeof(tWANIPCONObject), 1, pEntry->ConIPInstNum ) < 0 )
										return -1;
								//fprintf( stderr, "<%s:%d>add condev:%u ipcon:%u\n", __FUNCTION__, __LINE__, pEntry->ConDevInstNum, pEntry->ConIPInstNum);
						}//if
				}//if
			}//for
			add_objectNum( name, maxnum );
#else //#if defined(MULTI_WAN_SUPPORT)
			int wan_dhcp;
			int cwmp_ipconn_instnum;
			int cwmp_ipconn_created;
			
			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
			mib_get( MIB_CWMP_IPCON_CREATED, (void *)&cwmp_ipconn_created);
			mib_get( MIB_CWMP_IPCON_INSTNUM, (void *)&cwmp_ipconn_instnum);
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
			if( (wan_dhcp == DHCP_CLIENT) || (wan_dhcp == DHCP_DISABLED) || (wan_dhcp == PPPOE))
#else
			if( (wan_dhcp == DHCP_CLIENT) || (wan_dhcp == DHCP_DISABLED))
#endif
			{
				if(cwmp_ipconn_instnum == 0)
					cwmp_ipconn_instnum = 1;

				if( create_Object( ptable, tWANIPCONObject, sizeof(tWANIPCONObject), 1, cwmp_ipconn_instnum ) < 0 )
					return -1;

				maxnum = 1; //only support 1 ip connection
			}
			add_objectNum( name, maxnum );
			mib_set( MIB_CWMP_IPCON_INSTNUM, (void *)&maxnum);
#endif //#if defined(MULTI_WAN_SUPPORT)
			
			return 0;
		}
		case eCWMP_tADDOBJ:	     
		{
			int ret;
#if defined(MULTI_WAN_SUPPORT)			
			
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			Found=0;
			memset( &target[0], 0, sizeof( WANIFACE_T ) );
			memset( &target[1], 0, sizeof( WANIFACE_T ) );
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			for( i=1; i<=num;i++ )
			{
				pEntry = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				memcpy(&target[0], &wan_entity, sizeof(WANIFACE_T));
				
				if(pEntry->ConDevInstNum==devnum){
					Found=1;
					break;
				}
			}

			if(Found==0) return ERR_9005;///do not find entry for update

     	//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( pEntry->connDisable==0 ) /*already has one connection*/
				return ERR_9004;

#if 0 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
			if( (pEntry->cmode != ADSL_BR1483) &&
			    (pEntry->cmode != ADSL_MER1483) &&
			    (pEntry->cmode != ADSL_RT1483) )
				return ERR_9001;
#endif

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANIPCONObject, sizeof(tWANIPCONObject), data );
			if( ret >= 0 )
			{
				pEntry->connDisable=0;
				pEntry->ConIPInstNum=*(int*)data;
				pEntry->ConPPPInstNum=0;
				pEntry->enable=0;
#if 1 /*ct-com doesn't specify the mode first before creating the wanip/pppconnection */
				if( ((pEntry->cmode != IP_ROUTE) && (pEntry->cmode != IP_BRIDGE) )
#ifdef PPPOE_PASSTHROUGH
				    ||((pEntry->cmode==IP_BRIDGE) && (pEntry->brmode==BRIDGE_PPPOE))
#endif
				)
					{
						unsigned int new_ifindex;
						new_ifindex = getNewIfIndex(IP_BRIDGE, pEntry->ConDevInstNum);
						if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
						pEntry->cmode = IP_BRIDGE;
						pEntry->ifIndex = new_ifindex;
						pEntry->mtu = 1500;
					}
#endif
				memcpy(&target[1], &wan_entity, sizeof(WANIFACE_T)); ///wan_entry has been updated
				if ( !mib_set(MIB_WANIFACE_MOD, (void *)&target)) 
				{
					ret=-1;
				}
//#ifdef E8B_NEW_DIAGNOSE
//				writePVCFile(pEntry->vpi, pEntry->vci, "add", "", "");
//#endif
			}
			notify_set_wan_changed();
			if( ret >= 0 )
				ret=1;
			return ret;
#else
			int wan_dhcp;
			int cwmp_ipconn_instnum;
			int num=0;
			int cwmp_ipconn_created;
			
			mib_get( MIB_CWMP_IPCON_CREATED, (void *)&cwmp_ipconn_created);
			mib_get( MIB_CWMP_IPCON_INSTNUM, (void *)&cwmp_ipconn_instnum);

			num = get_ParameterNameCount(name,1);
			
			if(cwmp_ipconn_created == 1) // ip connection max is 1
	  			return ERR_9004;

			cwmp_ipconn_created = 1;
			
			//CWMPDBG( 2, ( stderr, "<%s:%d>addobj,\n", __FUNCTION__, __LINE__) );

			if(cwmp_ipconn_instnum == 0)
				cwmp_ipconn_instnum = 1;

			*(unsigned int*)data = cwmp_ipconn_instnum;
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANIPCONObject, sizeof(tWANIPCONObject), data );		
			if(ret == 0)
			{
				wan_dhcp = DHCP_CLIENT;
				mib_set( MIB_WAN_DHCP, (void *)&wan_dhcp);				
			mib_set( MIB_CWMP_IPCON_CREATED, (void *)&cwmp_ipconn_created);
				cwmp_ipconn_instnum = *(unsigned int*)data;
			mib_set( MIB_CWMP_IPCON_INSTNUM, (void *)&cwmp_ipconn_instnum);

				notify_set_wan_changed();
			}
			return 1;

#endif //#if defined(MULTI_WAN_SUPPORT)
		}	     
		case eCWMP_tDELOBJ:	     		
		{
			int ret;
#if defined(MULTI_WAN_SUPPORT)
			
			char s_appname[32];
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
			
			Found=0;
			memset( &target[0], 0, sizeof( WANIFACE_T ) );
			memset( &target[1], 0, sizeof( WANIFACE_T ) );
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			for( i=1; i<=num;i++ )
			{
				pEntry = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				memcpy(&target[0], &wan_entity, sizeof(WANIFACE_T));
				
				if( (pEntry->ConDevInstNum==devnum) &&
				    (pEntry->ConIPInstNum==*(unsigned int*)data) ){
				    	Found=1;
						break;
					}
			}//for
			if(Found==0) return ERR_9005;


			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )
			{
			
				//delete port-mappings of this connection.
				//delPortForwarding( pEntry->ifIndex );
				//delRoutingTable( pEntry->ifIndex );
//#ifdef E8B_NEW_DIAGNOSE
//				setWanName(s_appname, pEntry->applicationtype);
//				writePVCFile(pEntry->vpi, pEntry->vci, "del", s_appname, (pEntry->cmode == ADSL_BR1483) ? "桥接" : "路由");
//#endif

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
				delDhcpcOption( pEntry->ifIndex );
#endif
				//if the ipconnection is more than 1, delete the chain , not update it.
				//pEntry->connDisable=1;
				
				resetATMVCConnection( pEntry );
				memcpy(&target[1], &wan_entity, sizeof(WANIFACE_T)); ///wan_entry has been updated

				if ( !mib_set(MIB_WANIFACE_MOD, (void *)&target)) 
				{
					ret=-1;
				}
				ret=1;
			}
			return ret;
#else //#if defined(MULTI_WAN_SUPPORT)
			int whichOne=*(unsigned int*)data;
			int wan_dhcp;
			int cwmp_ipconn_instnum;
			int cwmp_ipconn_created;
			
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
			
			mib_get( MIB_CWMP_IPCON_INSTNUM, (void *)&cwmp_ipconn_instnum);
			
			if(whichOne != cwmp_ipconn_instnum) return ERR_9007; 

			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if(ret == 0)
			{
			wan_dhcp = PPPOE;
			mib_set( MIB_WAN_DHCP, (void *)&wan_dhcp);

			
				/* change cwmp_pppconn_created to 0, after reboot the ppp connection will be not created*/
			cwmp_ipconn_created = 0;

				mib_set( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_ipconn_created);

			}

			//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tDELOBJ\n", __FUNCTION__, __LINE__, name,type ) );
			return ret;
#endif //#if defined(MULTI_WAN_SUPPORT)
		}
		case eCWMP_tUPDATEOBJ:
		{
#if defined(MULTI_WAN_SUPPORT)
			struct CWMP_LINKNODE *old_table;
			int has_new=0;

			//CWMPDBG( 1, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=1; i<=num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				WANIFACE_T *p,wan_entity;

				p = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;

				if(p->connDisable==1) continue;

				if( (p->ConDevInstNum==devnum) && (p->ConIPInstNum!=0) )
				{
					remove_entity = remove_SiblingEntity( &old_table, p->ConIPInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
						unsigned int MaxInstNum=p->ConIPInstNum;
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANIPCONObject, sizeof(tWANIPCONObject), &MaxInstNum );
						has_new=1;
					}
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			if(has_new) notify_set_wan_changed();
			return 0;
#else


			int has_new=0;
			struct CWMP_LINKNODE *old_table;
			int wan_dhcp;
			int num=0;
			int cwmp_ipconn_created, cwmp_ipconn_instnum;




			num = get_ParameterNameCount(name,1);
			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
			mib_get( MIB_CWMP_IPCON_CREATED, (void *)&cwmp_ipconn_created);
#if defined (CUSTOMIZE_MIDDLE_EAST) && defined(CONFIG_RTL_DHCP_PPPOE)
			if( (wan_dhcp == DHCP_CLIENT) || (wan_dhcp == DHCP_DISABLED) || (wan_dhcp == PPPOE))
#else
			if( wan_dhcp == DHCP_CLIENT || wan_dhcp == DHCP_DISABLED)
#endif
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				int ret = 0;

				old_table = (struct CWMP_LINKNODE *)entity->next;	     	
				entity->next = NULL;
			
				mib_get( MIB_CWMP_IPCON_INSTNUM, (void *)&cwmp_ipconn_instnum);

				remove_entity = remove_SiblingEntity( &old_table, cwmp_ipconn_instnum );
				if( remove_entity!=NULL )
			{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}else{
					unsigned int MaxInstNum=cwmp_ipconn_instnum;
					ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANIPCONObject, sizeof(tWANIPCONObject), &MaxInstNum );

					cwmp_ipconn_instnum = MaxInstNum;									
				}
				
				if(ret == 0)
				{
					has_new=1;
					mib_set( MIB_CWMP_IPCON_INSTNUM, (void *)&cwmp_ipconn_instnum);

				cwmp_ipconn_created = 1;
					mib_set( MIB_CWMP_IPCON_CREATED, (void *)&cwmp_ipconn_created);
				}
				
				if( old_table )
					destroy_ParameterTable( (struct CWMP_NODE *)old_table );
				
			}
			else
			{
				cwmp_ipconn_created = 0;
				mib_set( MIB_CWMP_IPCON_CREATED, (void *)&cwmp_ipconn_created);
				//CWMPDBG( 2, ( stderr, "<%s:%d>devnum:%d, ConPPPInstNum=0\n", __FUNCTION__, __LINE__, devnum) );
			}
				
			
			
			if(has_new) notify_set_wan_changed();

			return 0;
				
#endif //#if defined(MULTI_WAN_SUPPORT)
	}
	}
	return -1;
}

#if defined(CONFIG_DEV_xDSL)
/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANDSLLinkConfig.*/
int getDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256];
	unsigned int devnum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	devnum = getWANConDevInstNum( name );
	if(devnum==0) return ERR_9005;	

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;
		if( pEntry->ConDevInstNum==devnum )
			break;
	}
	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}else if( strcmp( lastname, "LinkStatus" )==0 )
	{	
		int 	flags;
		char	ifname[16];
		
		if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) return ERR_9002;
		
		if(pEntry->enable==0)
			*data = strdup( "Down" );
		else if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_UP)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Unavailable" );
	}else if( strcmp( lastname, "LinkType" )==0 )
	{
		if( pEntry->cmode == ADSL_PPPoE )
			*data = strdup("EoA");//PPPoE => EoA
		else if (pEntry->cmode == ADSL_PPPoA)
			*data = strdup("PPPoA");
		else if (pEntry->cmode == ADSL_BR1483)
			*data = strdup("EoA");
		else if (pEntry->cmode == ADSL_MER1483)
			*data = strdup("EoA");
		else if (pEntry->cmode == ADSL_RT1483)
			*data = strdup("IPoA");
		else
			*data = strdup("Unconfigured");
	}else if( strcmp( lastname, "AutoConfig" )==0 )
	{	
		*data = booldup( 0 );
	}else if( strcmp( lastname, "ModulationType" )==0 )
	{	
		getAdslInfo(ADSL_GET_MODE, buf, 256);
		if( strncmp( buf, "T1.413", 6 )==0 )
			*data = strdup( "ADSL_ANSI_T1.413" );
		else if( strncmp( buf, "G.dmt", 5 )==0 )
			*data = strdup( "ADSL_G.dmt" );
		else if( strncmp( buf, "G.Lite", 6 )==0 )
			*data = strdup( "ADSL_G.lite" );
		else if( strncmp( buf, "ADSL2+", 6 )==0 )
			*data = strdup( "ADSL_2plus" );
		else if( strncmp( buf, "ADSL2", 5 )==0 )
			*data = strdup( "ADSL_G.dmt.bis" );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "DestinationAddress" )==0 )
	{	
		sprintf( buf, "PVC:%u/%u", pEntry->vpi, pEntry->vci );
		*data = strdup( buf );
	}else if( strcmp( lastname, "ATMEncapsulation" )==0 )
	{	
		if( pEntry->encap==ENCAP_VCMUX )
			*data = strdup( "VCMUX" );
		else if( pEntry->encap==ENCAP_LLC )
			*data = strdup( "LLC" );
		else
			return ERR_9002;
	}else if( strcmp( lastname, "ATMAAL" )==0 )
	{	
		*data = strdup( "AAL5" );
	}else if( strcmp( lastname, "ATMTransmittedBlocks" )==0 )
	{	
		unsigned int txcnt=0, rxcnt=0;
		getATMCellCnt( pEntry->vpi, pEntry->vci, &txcnt, &rxcnt );
		*data = uintdup( txcnt );
	}else if( strcmp( lastname, "ATMReceivedBlocks" )==0 )
	{	
		unsigned int txcnt=0, rxcnt=0;
		getATMCellCnt( pEntry->vpi, pEntry->vci, &txcnt, &rxcnt );
		*data = uintdup( rxcnt );
	}else if( strcmp( lastname, "ATMQoS" )==0 )
	{	
		if( pEntry->qos==ATMQOS_UBR )
			*data = strdup( "UBR" );
		else if( pEntry->qos==ATMQOS_CBR )
			*data = strdup( "CBR" );
		else if( pEntry->qos==ATMQOS_VBR_NRT )
			*data = strdup( "VBR-nrt" );
		else if( pEntry->qos==ATMQOS_VBR_RT )
			*data = strdup( "VBR-rt" );
		else
			return ERR_9002;
	}else if( strcmp( lastname, "ATMPeakCellRate" )==0 )
	{	
		*data = uintdup( pEntry->pcr );
	}else if( strcmp( lastname, "ATMMaximumBurstSize" )==0 )
	{	
		*data = uintdup( pEntry->mbs );
	}else if( strcmp( lastname, "ATMSustainableCellRate" )==0 )
	{	
		*data = uintdup( pEntry->scr );
	}else if( strcmp( lastname, "AAL5CRCErrors" )==0 ) /*the same value with ATMCRCErrors, wt121*/
	{
		unsigned int count;
		if( getAAL5CRCErrors( pEntry->vpi, pEntry->vci, &count )<0 ) return ERR_9002;
		*data = uintdup( count );
	}else if( strcmp( lastname, "ATMCRCErrors" )==0 ) /*the same value with AAL5CRCErrors, wt121*/
	{	
		unsigned int count;
		if( getAAL5CRCErrors( pEntry->vpi, pEntry->vci, &count )<0 ) return ERR_9002;
		*data = uintdup( count );
#ifdef _PRMT_X_TELEFONICA_ES_IGMPCONFIG_
	}else if( strcmp( lastname, "X_TELEFONICA-ES_IGMPEnabled" )==0 ) //Spec v10, correction of the parameter's name
	{
/*ping_zhang:20081230 START:telefonica tr069 new request IGMP configuration*/
		if(pEntry->enableIGMP)
		*data = booldup(1);
		else
			*data = booldup(0);
/*ping_zhang:20081230 END*/
#endif //_PRMT_X_TELEFONICA_ES_IGMPCONFIG_
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*tok;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	buf[256];
	unsigned int devnum,num,chainid;
	int ret=0;
	unsigned char pppoe_ifindex=0xff;
	unsigned int ipnum, pppnum;

	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif
	if( data==NULL ) return ERR_9007;

	devnum = getWANConDevInstNum( name );

	if(devnum==0) return ERR_9005;	

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( chainid=0; chainid<num;chainid++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, chainid, (void*)pEntry ))
			continue;	

		if( pEntry->ConDevInstNum==devnum )
		{
			if( strcmp( lastname, "Enable" )==0 )
			{
#ifdef _PRMT_X_CT_COM_DATATYPE
				int *i = &tmpbool;
#else
				int *i = data;
#endif
				if( i==NULL ) return ERR_9007;
				pEntry->enable = (*i==0)?0:1;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=1;//return 1;
			}else if( strcmp( lastname, "LinkType" )==0 )
			{
				char *tmp=data;
				unsigned char old_ifindex, new_ifindex;
		
				old_ifindex = pEntry->ifIndex;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;
		
				if( strcmp( "PPPoE", tmp )==0 )
				{
#if 1
					//use EoA to create PPPoE
					return ERR_9001;
#else
					if( pEntry->cmode != ADSL_PPPoE )
					{
						new_ifindex = getNewIfIndex( ADSL_PPPoE );
						if( (new_ifindex==0xff)||(new_ifindex==0xef) ) return ERR_9001;
						if(pppoe_ifindex==0xff) pppoe_ifindex=new_ifindex;
						pEntry->cmode = ADSL_PPPoE;
						//use the same vc, only the pppoe has multi-seesion
						pEntry->ifIndex = (new_ifindex&0xf0)|(pppoe_ifindex&0x0f);
						pEntry->mtu = 1492;
					}
#endif
				}else if( strcmp( "PPPoA", tmp )==0 )
				{
					if(pEntry->cmode != ADSL_PPPoA)
					{
						new_ifindex = getNewIfIndex( ADSL_PPPoA );
						if( (new_ifindex==0xff)||(new_ifindex==0xef) ) return ERR_9001;
						pEntry->cmode = ADSL_PPPoA;
						pEntry->ifIndex = new_ifindex;
						pEntry->mtu = 1500;
					}
				}else if( strcmp( "EoA", tmp )==0 )
				{
					if( (pEntry->cmode!=ADSL_BR1483) &&
					    (pEntry->cmode!=ADSL_MER1483) &&
					    (pEntry->cmode!=ADSL_PPPoE) )
					{
						new_ifindex = getNewIfIndex( ADSL_BR1483 );
						if( (new_ifindex==0xff)||(new_ifindex==0xef) ) return ERR_9001;
						pEntry->cmode = ADSL_BR1483;
						pEntry->ifIndex = new_ifindex;
						pEntry->mtu = 1500;
				#ifdef CONFIG_BOA_WEB_E8B_CH
					#ifdef _PRMT_X_CT_COM_WANEXT_
						pEntry->ServiceList=X_CT_SRV_INTERNET;
					#endif
					#ifdef CTC_WAN_NAME
						pEntry->applicationtype = 1;//INTERNET
					#endif
				#endif
					}
				}else if( strcmp( "IPoA", tmp )==0 )
				{
					if(pEntry->ConPPPInstNum == 0)  // Magician: Prevent WANPPPConnecntion being changed to IPoA.
					{
					if(pEntry->cmode != ADSL_RT1483)
					{
						new_ifindex = getNewIfIndex( ADSL_RT1483 );
						if( (new_ifindex==0xff)||(new_ifindex==0xef) ) return ERR_9001;
						pEntry->cmode = ADSL_RT1483;
						pEntry->ifIndex = new_ifindex;
						pEntry->mtu = 1500;
				}else
					return ERR_9007;	
		
				if(pEntry->ifIndex!=old_ifindex)
				{
					//if ip -> ppp	or ppp->ip, destroy ppp/ip objects and reset values??
					mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
					updatePortForwarding( old_ifindex, pEntry->ifIndex );
					updateRoutingTable( old_ifindex, pEntry->ifIndex );
					ret=1;//return 1;
				}
					}
				}
				else
					return ERR_9007;
			}else if( strcmp( lastname, "DestinationAddress" )==0 )
			{
				char tmp1[64],*tmp=data, *sVPI, *sVCI;
				int vpi,vci;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;
				/*for loop, strtok will change the data */
				strncpy( tmp1,tmp,63 );
				tmp1[63]=0;
				tmp = tmp1;
				if( strncmp( tmp, "PVC", 3 )!=0 ) return ERR_9007;
				tmp = tmp + 3;
				sVPI = strtok( tmp, " :/" );
				sVCI = strtok( NULL, " :/" );
				if( (sVPI==NULL) || (sVCI==NULL) ) return ERR_9007;
				vpi = atoi( sVPI );
				vci = atoi( sVCI );
				if( (vpi<0) || (vci<0) ) return ERR_9007;
				//if (vpi,vci) conflicts with other channels??
				pEntry->vpi = vpi;
				pEntry->vci = vci;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=1;//return 1;
			}else if( strcmp( lastname, "ATMEncapsulation" )==0 )
			{
				char *tmp=data;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;
				if( strcmp(tmp, "VCMUX")==0 )
					pEntry->encap=ENCAP_VCMUX;
				else if( strcmp(tmp, "LLC")==0 )
					pEntry->encap=ENCAP_LLC;
				else
					return ERR_9007;
					
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=1;//return 1;		
			}else if( strcmp( lastname, "ATMQoS" )==0 )
			{	
				char *tmp=data;
				if( tmp==NULL ) return ERR_9007;
				if( strlen(tmp)==0 ) return ERR_9007;
				if( strcmp(tmp, "UBR")==0 )
					pEntry->qos=ATMQOS_UBR;
				else if( strcmp(tmp, "CBR")==0 )
					pEntry->qos=ATMQOS_CBR;
				else if( strcmp(tmp, "VBR-nrt")==0 )
					pEntry->qos=ATMQOS_VBR_NRT;
				else if( strcmp(tmp, "VBR-rt")==0 )
					pEntry->qos=ATMQOS_VBR_RT;
				else
					return ERR_9007;
					
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=1;//return 1;
			}else if( strcmp( lastname, "ATMPeakCellRate" )==0 )
			{
#ifdef _PRMT_X_CT_COM_DATATYPE
				unsigned int *pUInt = &tmpuint;
#else
				unsigned int *pUInt = data;
#endif
				if(pUInt==NULL) return ERR_9007;
				if( *pUInt<0 || *pUInt>ATM_MAX_US_PCR )	return ERR_9001;
				pEntry->pcr = *pUInt;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=1;//return 1;
			}else if( strcmp( lastname, "ATMMaximumBurstSize" )==0 )
			{	
#ifdef _PRMT_X_CT_COM_DATATYPE
				unsigned int *pUInt = &tmpuint;
#else
				unsigned int *pUInt = data;
#endif
				if(pUInt==NULL) return ERR_9007;
				if( *pUInt>65535 ) return ERR_9001;
				pEntry->mbs = *pUInt;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=1;//return 1;
			}else if( strcmp( lastname, "ATMSustainableCellRate" )==0 )
			{	
#ifdef _PRMT_X_CT_COM_DATATYPE
				unsigned int *pUInt = &tmpuint;
#else
				unsigned int *pUInt = data;
#endif
				if(pUInt==NULL) return ERR_9007;
				if( *pUInt<0 || *pUInt>ATM_MAX_US_PCR )	return ERR_9001;
				pEntry->scr = *pUInt;
				mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				ret=1;//return 1;
#ifdef _PRMT_X_TELEFONICA_ES_IGMPCONFIG_
			}else if( strcmp( lastname, "X_TELEFONICA-ES_IGMPEnabled" )==0 ) //Spec v10, correction of the parameter's name
			{
				int *i = data;
				if( i==NULL ) return ERR_9007;
/*ping_zhang:20081230 START:telefonica tr069 new request IGMP configuration*/
				if(pEntry->enableIGMP != *i)
				{
					pEntry->enableIGMP = *i;
					mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, chainid );
				}
#ifdef _CWMP_APPLY_
				apply_add( CWMP_PRI_L, apply_IGMPProxy, CWMP_RESTART, 0, NULL, 0 );
				return 0;
#else
				return 1;
#endif
/*ping_zhang:20081230 END*/
				//fprintf( stderr, "set %s=%d\n", name, (*i==0)?0:1 );
				return 0;
#endif //_PRMT_X_TELEFONICA_ES_IGMPCONFIG_
			}else{
				return ERR_9005;
			}
		}//if
	}//for
	//if(chainid==num) return ERR_9005;

	return ret;
}
#endif //#if 0 //keith remove. no support DSL

/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANEthernetLinkConfig.*/
int getWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	buf[256];
	int isFound=0;
	unsigned int devnum,num,i;
#if defined(MULTI_WAN_SUPPORT)	
	WANIFACE_T *pEntry,wan_entity;
#endif

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	devnum = getWANConDevInstNum( name );
	if(devnum==0) return ERR_9005;

#if defined(MULTI_WAN_SUPPORT)	
	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		pEntry = &wan_entity;
		*((char *)&wan_entity) = (char)i;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
			continue;
		if( pEntry->ConDevInstNum==devnum ){
			isFound=1;
			break;
		}
	}
	if(isFound==0) return ERR_9005;
#endif

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "EthernetLinkStatus" )==0 )
	{
		*data = strdup("Unavailable");
	}
#ifdef _PRMT_X_CT_COM_ETHLINK_
	else if( strcmp( lastname, "X_CT-COM_Enable" )==0 )
	{
		//return ERR_9001;
		*data = booldup(1);
	}
	else if( strcmp( lastname, "X_CT-COM_Mode" )==0 )
	{
		*data = uintdup(pEntry->vlan);
	}
	else if( strcmp( lastname, "X_CT-COM_VLANIDMark" )==0 )
	{
		if(pEntry->vlan)
			*data = uintdup(pEntry->vlanid);
		else
			*data = uintdup(0);
	}else if( strcmp( lastname, "X_CT-COM_802-1pMark" )==0 )
	{
		if(pEntry->vlan)
		*data = uintdup(pEntry->vlanpriority);
		else
			*data = uintdup(0);
	}
#endif
	else
		return ERR_9005;

	return 0;
}

int setWANETHLINKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
#ifdef _PRMT_X_CT_COM_ETHLINK_
	char	*lastname = entity->info->name;
	char	*tok;
	WANIFACE_T *pEntry,wan_entity;
	WANIFACE_T target[2];
	char	buf[256];
	unsigned int devnum,num,chainid;
	int ret=0;
	int isFound=0;
	unsigned int pppoe_ifindex=0xff;

	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif
	if( data==NULL ) return ERR_9007;

	devnum = getWANConDevInstNum( name );
	if(devnum==0) return ERR_9005;

	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
	for( chainid=1; chainid<=num;chainid++ )
	{
		
		
		pEntry = &wan_entity;
		memset( &target[0], 0, sizeof( WANIFACE_T ) );
		memset( &target[1], 0, sizeof( WANIFACE_T ) );
		*((char *)&wan_entity) = (char)chainid;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
			continue;


		memcpy(&target[0], &wan_entity, sizeof(WANIFACE_T));
		
		if( pEntry->ConDevInstNum==devnum )
		{
			isFound++;
			if( strcmp( lastname, "X_CT-COM_VLANIDMark" )==0 )
			{
#ifdef _PRMT_X_CT_COM_DATATYPE
				unsigned int *i = &tmpuint;
#else
				unsigned int *i = data;
#endif
				if( i==NULL ) return ERR_9007;

				pEntry->vlanid=(unsigned short)*i;
				memcpy(&target[1], &wan_entity, sizeof(WANIFACE_T));
				if ( !mib_set(MIB_WANIFACE_MOD, (void *)&target)) 
				{
						ret=-1;
				}else
						ret=1;//return 1;

			}
			else if( strcmp( lastname, "X_CT-COM_Enable" )==0 )
			{
				return ERR_9001;
			}
			else if( strcmp( lastname, "X_CT-COM_Mode" )==0 )
			{
#ifdef _PRMT_X_CT_COM_DATATYPE
				unsigned int *i = &tmpuint;
#else
				unsigned int *i = data;
#endif
				if( i==NULL ) return ERR_9007;

				pEntry->vlan = *i;
				memcpy(&target[1], &wan_entity, sizeof(WANIFACE_T));
				if ( !mib_set(MIB_WANIFACE_MOD, (void *)&target)) 
	     			{
						ret=-1;
				}else
						ret=1;//return 1;
			}
			else if( strcmp( lastname, "X_CT-COM_802-1pMark" )==0 )
			{
#ifdef _PRMT_X_CT_COM_DATATYPE
				unsigned int *i = &tmpuint;
#else
				unsigned int *i = data;
#endif
				if( i==NULL ) return ERR_9007;
				if( *i<0 || *i>7) return ERR_9007;
				//pEntry->pmark=*i;
				pEntry->vlanpriority = *i;
				memcpy(&target[1], &wan_entity, sizeof(WANIFACE_T));
				if ( !mib_set(MIB_WANIFACE_MOD, (void *)&target)) 
	     			{
						ret=-1;
				}else
						ret=1;//return 1;
			}else{
				return ERR_9005;
			}
		}//if
	}//for
	if(isFound==0) return ERR_9005;
	
	return ret;
#else
	return ERR_9005; // No value can be set for WANEthernetLinkConfig of standard TR-069.
#endif
}



/*InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.*/
int getWANCONDEVENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
#if defined(MULTI_WAN_SUPPORT)	
	WANIFACE_T *p,wan_entity;
#endif
	unsigned int instnum, i,num, ipcnt=0,pppcnt=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	instnum = getWANConDevInstNum( name );
	if(instnum==0) return ERR_9005;

#if defined(MULTI_WAN_SUPPORT)
	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		p = &wan_entity;
		
		*((char *)&wan_entity) = (char)i;
		
		if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
			continue;	
			
		if( p->ConDevInstNum==instnum )
		{
			switch( p->cmode )
			{
			case IP_BRIDGE:
#ifdef PPPOE_PASSTHROUGH
				if( p->connDisable==0 )
				{
					if(p->brmode==BRIDGE_PPPOE)
						pppcnt++;
					else
						ipcnt++;
				}
				break;
#endif
			case IP_ROUTE:
				if(p->AddressType == DHCP_CLIENT || p->AddressType ==DHCP_DISABLED){
				if( p->connDisable==0 ) ipcnt++;
				}
				if(p->AddressType == PPPOE){
				if( p->connDisable==0 ) pppcnt++;
				}
				break;
			
			case IP_PPP:
				if(p->AddressType == PPPOE){
					if( p->connDisable==0 ) pppcnt++;
				}
				break;
			}
		}
	}

#else //#if defined(MULTI_WAN_SUPPORT)
		int wan_dhcp;

		int cwmp_ipconn_created;
		int cwmp_pppconn_created;

		mib_get( MIB_CWMP_IPCON_CREATED, (void *)&cwmp_ipconn_created);			
		mib_get( MIB_CWMP_PPPCON_CREATED, (void *)&cwmp_pppconn_created);

		ipcnt = cwmp_ipconn_created;
		pppcnt = cwmp_pppconn_created;
#endif //#if defined(MULTI_WAN_SUPPORT)		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WANIPConnectionNumberOfEntries" )==0 )
	{
			*data = uintdup( ipcnt );
	}
	else if( strcmp( lastname, "WANPPPConnectionNumberOfEntries" )==0 )
	{	
			*data = uintdup( pppcnt );
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int objConDev(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	
	//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			
			int num, MaxInstNum, i;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;		
#if defined(MULTI_WAN_SUPPORT)
			WANIFACE_T *p,wan_entity;
#endif	
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

#if defined(MULTI_WAN_SUPPORT)
			MaxInstNum = findMaxConDevInstNum();
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			//CWMPDBG( 2, ( stderr, "<%s:%d>initobj, MaxInstNum:%d, num:%d\n", __FUNCTION__, __LINE__, MaxInstNum,num ) );
			for( i=1; i<=num;i++ )
			{
				int j,addit=1;

				p = &wan_entity;
				*((char *)&wan_entity) = (char)i;
	
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;

				//check if added??
				for( j=1; j<=i;j++ )
				{
					WANIFACE_T match_entity;
					*((char *)&match_entity) = (char)j;
					if(!mib_get(MIB_WANIFACE_TBL, (void *)&match_entity))
						continue;
					
					if( wan_entity.ConDevInstNum == match_entity.ConDevInstNum )
					{
						//CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tINITOBJ Find the same InstNum\n", __FUNCTION__, __LINE__) );
						addit=0;
						break;
					}
				}

				if(addit)
				{
					if( create_Object( c, tWANCONDEVObject, sizeof(tWANCONDEVObject), 1, wan_entity.ConDevInstNum ) < 0 )
				return -1;
					//CWMPDBG( 2, ( stderr, "<%s:%d>add ConDev:%d\n", __FUNCTION__, __LINE__, p->ConDevInstNum ) );
				}
			}
#else //#if defined(MULTI_WAN_SUPPORT)
			int wan_dhcp;

			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);		

			if( wan_dhcp != DHCP_NONE )
			{	
				MaxInstNum = 1;
				if( create_Object( c, tWANCONDEVObject, sizeof(tWANCONDEVObject), 1, 1 ) < 0 )
					return -1;

				add_objectNum( name, MaxInstNum );
				//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tINITOBJ\n", __FUNCTION__, __LINE__, name,type ) );
				return 0;
			}
			else
			{

			}			
#endif //#if defined(MULTI_WAN_SUPPORT)
			break;
				
		}
		case eCWMP_tADDOBJ:
		{
			int ret;
#if defined(MULTI_WAN_SUPPORT)			
			unsigned int ifindex=0;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ifindex = getNewIfIndex( IP_BRIDGE, 0 );//default: bridge mode
			//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( ifindex==NA_VC ) return ERR_9004; //Maximum number of VC exceeds

			//CWMPDBG( 2, ( stderr, "<%s:%d>addobj, ifindex:0x%08X\n", __FUNCTION__, __LINE__, ifindex ) );
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObject, sizeof(tWANCONDEVObject), data );
			if( ret >= 0 )
			{
				WANIFACE_T entry;
				memset( &entry, 0, sizeof( WANIFACE_T ) );
				{ //default values for this new entry
					entry.mtu=1500;
					entry.ifIndex=ifindex;
					entry.connDisable=1; //0:enable, 1:disable
					entry.ConDevInstNum=*(int*)data;
#ifdef CONFIG_BOA_WEB_E8B_CH
					entry.applicationtype=X_CT_SRV_INTERNET;
#ifdef _PRMT_X_CT_COM_WANEXT_
					entry.ServiceList=X_CT_SRV_INTERNET;
#endif
#endif
#ifdef CONFIG_USER_WT_146
					wt146_set_default_config( &entry );
#endif //CONFIG_USER_WT_146
				}
				if ( mib_set(MIB_WANIFACE_ADD, (void *)&entry) == 0) 
				{
					//fprintf(stderr,"\r\n Add WANIFACE table entry error!");
					ret= -1;
				}
			}
			notify_set_wan_changed();
			//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tADDOBJ\n", __FUNCTION__, __LINE__, name,type ) );
			if(ret >= 0)
				ret=1;
			return ret;
#else
			int wan_dhcp;

			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);		

			if( wan_dhcp != DHCP_NONE ) return ERR_9004; //Already create.

			//CWMPDBG( 2, ( stderr, "<%s:%d>addobj,\n", __FUNCTION__, __LINE__) );
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tWANCONDEVObject, sizeof(tWANCONDEVObject), data );
			if( ret >= 0 )
			{
				wan_dhcp = DHCP_CLIENT;
				mib_set( MIB_WAN_DHCP, (void *)&wan_dhcp);

				notify_set_wan_changed();
				//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tADDOBJ\n", __FUNCTION__, __LINE__, name,type ) );
				if(ret >= 0)
					ret=1;
				return ret;
			}
#endif //#if defined(MULTI_WAN_SUPPORT)
			break;
		}	     
		case eCWMP_tDELOBJ:	     		
		{
			int ret;
#if defined(MULTI_WAN_SUPPORT)
			int num, i;
			unsigned int *pUint=data;
			WANIFACE_T *p, wan_entity;
			char s_appname[32];

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			for( i=num; i>=1;i-- )
			{
				
				p = &wan_entity;

				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;
				
				if(p->ConDevInstNum==*pUint )
				{
					//delete port-mappings of this connectiondevice.
					//delPortForwarding( p->ifIndex );
					//delRoutingTable( p->ifIndex );
					mib_set(MIB_WANIFACE_DEL, (void *)&wan_entity);
#ifdef E8B_NEW_DIAGNOSE
					setWanName(s_appname, p->applicationtype);
					//writePVCFile(p->vpi, p->vci, "del", s_appname, (p->cmode == ADSL_BR1483) ? "桥接" : "路由");
#endif
				}
			}

			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;
			//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tDELOBJ\n", __FUNCTION__, __LINE__, name,type ) );
			return ret;
#else //#if defined(MULTI_WAN_SUPPORT)
			int whichOne=*(unsigned int*)data;
			int wan_dhcp;
			
			if(whichOne > 1) return ERR_9007; 

			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);

			if( wan_dhcp == DHCP_NONE ) return ERR_9007; //object doesn't exist.

			wan_dhcp = DHCP_NONE;
			mib_set( MIB_WAN_DHCP, (void *)&wan_dhcp);

			
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;

			notify_set_wan_changed();

			//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tDELOBJ\n", __FUNCTION__, __LINE__, name,type ) );
			return ret;
#endif //#if defined(MULTI_WAN_SUPPORT)
		}
		case eCWMP_tUPDATEOBJ:
		{
#if defined(MULTI_WAN_SUPPORT)
			int num,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;
    
			
			mib_get(MIB_WANIFACE_TBL_NUM, (void *)&num);
			
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=1; i<=num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				WANIFACE_T *p,wan_entity;

				p = &wan_entity;
				*((char *)&wan_entity) = (char)i;
				if(!mib_get(MIB_WANIFACE_TBL, (void *)&wan_entity))
					continue;

				//if(wan_entity.connDisable)
				//	continue;

				remove_entity = remove_SiblingEntity( &old_table, wan_entity.ConDevInstNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, wan_entity.ConDevInstNum  )==NULL )
					{
						unsigned int MaxInstNum=wan_entity.ConDevInstNum;
						//CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tUPDATEOBJ,1 MaxInstNum=%d\n", __FUNCTION__, __LINE__, MaxInstNum) );
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWANCONDEVObject, sizeof(tWANCONDEVObject), &MaxInstNum );
						//CWMPDBG( 2, ( stderr, "<%s:%d>eCWMP_tUPDATEOBJ,2 MaxInstNum=%d\n", __FUNCTION__, __LINE__, MaxInstNum) );
						has_new=1;
					}//else already in next_table
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			if(has_new) notify_set_wan_changed();
			//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) eCWMP_tUPDATEOBJ\n", __FUNCTION__, __LINE__, name,type ) );
			return 0;
#else
			struct CWMP_LINKNODE *old_table;
			int wan_dhcp;
			mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
                    	if( wan_dhcp == DHCP_NONE ) 
			{
				old_table = (struct CWMP_LINKNODE *)entity->next;
				entity->next = NULL;

				if( old_table )
					destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			}
			
#endif //#if defined(MULTI_WAN_SUPPORT)
	}
	}
	//CWMPDBG( 2, ( stderr, "<%s:%d>name:%s(action:%d) return -1\n", __FUNCTION__, __LINE__, name,type ) );

	return -1;
}

#if 0 //keith remove. no support DSL
/*InternetGatewayDevice.WANDevice.{i}.WANDSLConnectionManagement.*/
int getCONSERENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	*tok=NULL;
	int	chainid;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	buf[256];
	unsigned int conserid;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	conserid = getInstNum( name, "ConnectionService" );
	if(conserid==0) return ERR_9005;
	
	chainid = conserid -1;
	pEntry = &vc_entity;
	if( !mib_chain_get(MIB_ATM_VC_TBL, chainid, (void*)pEntry ) )
		return ERR_9002;
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WANConnectionDevice" )==0 )
	{
		char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%u";//wt-121v8-3.33, no trailing dot
		sprintf( buf, strfmt, pEntry->ConDevInstNum );
		*data = strdup( buf );
	}else if( strcmp( lastname, "WANConnectionService" )==0 )
	{	
		char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%u.%s.%u"; //wt-121v8-3.33, no trailing dot
		char ipstr[]="WANIPConnection";
		char pppstr[]="WANPPPConnection";
		char *pconn=NULL;

		if( pEntry->connDisable==1) //no connection with this connectiondevice
			*data = strdup("");
		else
		{
			if( (pEntry->cmode==ADSL_PPPoE) || (pEntry->cmode==ADSL_PPPoA) )
				sprintf( buf, strfmt, pEntry->ConDevInstNum, pppstr, pEntry->ConPPPInstNum );
			else
				sprintf( buf, strfmt, pEntry->ConDevInstNum, ipstr, pEntry->ConIPInstNum );
		}
		*data = strdup( buf );
	}else if( strcmp( lastname, "DestinationAddress" )==0 )
	{	
		sprintf( buf, "PVC:%u/%u", pEntry->vpi, pEntry->vci );
		*data = strdup( buf );
	}else if( strcmp( lastname, "LinkType" )==0 )
	{	
		if( pEntry->cmode == ADSL_PPPoE )
			*data = strdup("PPPoE");
		else if (pEntry->cmode == ADSL_PPPoA)
			*data = strdup("PPPoA");
		else if (pEntry->cmode == ADSL_BR1483)
			*data = strdup("EoA");
		else if (pEntry->cmode == ADSL_MER1483)
			*data = strdup("EoA");
		else if (pEntry->cmode == ADSL_RT1483)
			*data = strdup("IPoA");
		else
			*data = strdup("Unconfigured");
	}
#if 0 //keith remove	
	else if( strcmp( lastname, "ConnectionType" )==0 )
	{
		if( pEntry->connDisable==1) //don't create a connection with this connectiondevice
			*data = strdup("Unconfigured");
		else if( pEntry->cmode == ADSL_PPPoE )
			*data = strdup("IP_Routed");
		else if (pEntry->cmode == ADSL_PPPoA)
			*data = strdup("IP_Routed");
		else if (pEntry->cmode == ADSL_BR1483)
			*data = strdup("IP_Bridged");
		else if (pEntry->cmode == ADSL_MER1483)
			*data = strdup("IP_Routed");
		else if (pEntry->cmode == ADSL_RT1483)
			*data = strdup("IP_Routed");
		else
			*data = strdup("Unconfigured");
	}
#endif //#if 0 //keith remove
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	else if( strcmp( lastname, "Name" )==0 )
	{	
		if(*(pEntry->WanName))
			*data = strdup( pEntry->WanName );
		else
			*data = strdup("");
	}
#endif
/*ping_zhang:20081217 END*/	
	else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //#if 0 //keith remove. no support DSL

#if 0 //keith remove. no support DSL
int objConService(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int num=0;
		struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;
		if(data==NULL) return -1;
		num = mib_chain_total( MIB_ATM_VC_TBL );
		*ptable = NULL;
		return create_Object( ptable, tCONSERVICEObject, sizeof(tCONSERVICEObject), num, 1 );
	     }
	case eCWMP_tUPDATEOBJ:
	     {
	     	unsigned int num,i;
	     	struct CWMP_LINKNODE *old_table;

		num = mib_chain_total( MIB_ATM_VC_TBL );
	     	old_table = (struct CWMP_LINKNODE*)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;

			remove_entity = remove_SiblingEntity( &old_table, i+1 );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
			}else{ 
				unsigned int InstNum=i+1;
				add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tCONSERVICEObject, sizeof(tCONSERVICEObject), &InstNum );
			}
	     	}
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
		return 0;
	     }
	}
	
	return -1;
}
#endif //#if 0 //keith remove. no support DSL

#if 0 //keith remove. no support DSL
int getWANDSLCNTMNG(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ConnectionServiceNumberOfEntries" )==0 )
	{
		int total;
		total = mib_chain_total(MIB_ATM_VC_TBL);
		*data = uintdup( total ); 
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
#ifdef HOME_GATEWAY
#define _DHCPC_PID_PATH		"/etc/udhcpc"
#define _DHCPC_PROG_NAME	"udhcpc"

int cwmp_isDhcpClientExist(char *name)
{
	char tmpBuf[100];
	struct in_addr intaddr;

	if ( getInAddr(name, IP_ADDR, (void *)&intaddr ) ) {
		snprintf(tmpBuf, 100, "%s/%s-%s.pid", _DHCPC_PID_PATH, _DHCPC_PROG_NAME, name);
		if ( getPid(tmpBuf) > 0)
			return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
#if 0 //by cairui user extern in common.c
int isConnectPPP()
{
	struct stat status;

	if ( stat("/etc/ppp/link", &status) < 0)
		return 0;

	return 1;
}
#endif

#endif
#if 0
int getObjectID( struct sCWMP_ENTITY *ctable, int chainid )
{
	int i=-1;
	int objectid=0;

	if( chainid< 0 ) return objectid;
	
	while( ctable )
	{
		i++;
		if( i==chainid ) break;
		ctable = ctable->sibling;
	}

	if(ctable)
		sscanf( ctable->name, "%d", &objectid );
	
	return objectid;
}
#endif //#if 0 //keith remove

#if 0 //keith remove
/*copy from fmwan.c:if_find_index*/
unsigned char getNewIfIndex(int cmode)
{
	int i;
	unsigned char index;

	
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	unsigned int map=0;	// high half for PPP bitmap, low half for vc bitmap
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		pEntry = &vc_entity;
		if( mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) ) /* get the specified chain record */
		{
			map |= 1 << (pEntry->ifIndex & 0x0f);	// vc map
			map |= (1 << 16) << ((pEntry->ifIndex >> 4) & 0x0f);	// PPP map
		}
	}

	// find the first available vc index (mpoa interface)
	i = 0;
	for (i=0; i<MAX_VC_NUM; i++)
	     {
		if (!((map>>i) & 1))
	     	break;
	     }
	
	if (i != MAX_VC_NUM)
		index = i;
	else
		return 0xff;
	
	if (cmode == ADSL_PPPoE || cmode == ADSL_PPPoA)
	     {
		// find an available PPP index
		map >>= 16;
		i = 0;
		while (map & 1)
		{
			map >>= 1;
			i++;
			}
		if (i<=(MAX_PPP_NUM-1))
			index |= i << 4;	// high nibble for PPP index
		else
			return 0xef;
		
		if (cmode == ADSL_PPPoA)
			index |= 0x0f;	// PPPoA doesn't use mpoa interface, set to 0x0f (don't care)
		}
	else
	{
		// don't care the PPP index
		index |= 0xf0;
	}
	return index;
}
#endif //#if 0 //keith remove

/*functions for get an object's number*/
unsigned int getInstNum( char *name, char *objname )
{
	//diag_printf("enter getInstNum, name=%s, objname=%s\n", name, objname);
	unsigned int num=0;
	
	if( (objname!=NULL)  && (name!=NULL) )
	{
		char buf[256],*tok;
		sprintf( buf, ".%s.", objname );
		tok = strstr( name, buf );
		if(tok)
		{
			tok = tok + strlen(buf);
			sscanf( tok, "%u.%*s", &num );
		}
	}
	
	return num;
}

unsigned int getWANConDevInstNum( char *name )
{
	return getInstNum( name, "WANConnectionDevice" );
}

unsigned int getWANPPPConInstNum( char *name )
{
	return getInstNum( name, "WANPPPConnection" );
}

unsigned int getWANIPConInstNum( char *name )
{
	return getInstNum( name, "WANIPConnection" );
}


unsigned int getWANPortMapInstNum( char *name )
{
	return getInstNum( name, "PortMapping" );
}

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
unsigned int getSentDHCPOptInstNum( char *name )
{
	return getInstNum( name, "SentDHCPOption" );
}

unsigned int getReqDHCPOptInstNum( char *name )
{
	return getInstNum( name, "ReqDHCPOption" );
}
#endif
/*ping_zhang:20080919 END*/


int getPortMappingCount(int *portEntityCount)
{
	int count = 0;	

	mib_get(MIB_PORTFW_TBL_NUM, (void *)&count);
	*portEntityCount = count;

	return 0;
}

unsigned int getPortMappingMaxInstNum( void )
{
	unsigned int ret = 0;	
	PORTFW_T port_entity;
	unsigned int total,i;
	

	mib_get(MIB_PORTFW_TBL_NUM, (void *)&total);
	for( i=1;i<=total;i++ )
	{

		*((char *)&port_entity) = (char)i;
		if( mib_get( MIB_PORTFW_TBL, (void*)&port_entity ) )
		{
			if(port_entity.InstanceNum > ret) ret = port_entity.InstanceNum;
		}
	}
	return ret;
}

int getPortMappingByInstNum( unsigned char ifindex, unsigned int instnum, PORTFW_T *c, unsigned int *chainID )
{
	unsigned int total,i;
	
	if( (instnum == 0) || (c==NULL) || (chainID==NULL) ) return -1;


	mib_get(MIB_PORTFW_TBL_NUM, (void *)&total);
	for( i=1;i<=total;i++ )
	{
		*((char *)c) = (char)i;
		 mib_get( MIB_PORTFW_TBL, (void*)c);
		if( (c->InstanceNum==instnum) )
		{
			*chainID = i;
     			return 0;
     		}
	}
	
	return -1;
}

#ifdef VIRTUAL_SERVER_SUPPORT
#if 0 //keith remove
/*portmapping utilities*/
unsigned int getPortMappingMaxInstNum( unsigned char ifindex )
{
	unsigned int ret = 0;	
	vtlsvr_entryx *p, port_entity;
	unsigned int total,i;
	
	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)p ) )
		{
			if(p->InstanceNum>ret) ret = p->InstanceNum;
		}
	}

	return ret;
}

int getPortMappingCount( unsigned char ifindex )
{
	int count = 0;	
	vtlsvr_entryx *p, port_entity;
	unsigned int total,i;
	
	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)p ) )
			count++;
	}

	return count;
}

/*id starts from 0, 1, 2, ...*/
int getPortMappingByID( unsigned char ifindex, int id, vtlsvr_entryx *c, unsigned int *chainID )
{
	unsigned int total,i;
	
	if( (id < 0) || (c==NULL) || (chainID==NULL) ) return -1;
	
	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)c ) )
			continue;

		id--;
		if(id==-1)
		{
			*chainID = i; 
			return 0;
		}
	}
	
	return -1;
}


int getPortMappingByInstNum( unsigned char ifindex, unsigned int instnum, vtlsvr_entryx *c, unsigned int *chainID )
{
	unsigned int total,i;
	
	if( (instnum == 0) || (c==NULL) || (chainID==NULL) ) return -1;
	
	total = mib_chain_total( MIB_VIRTUAL_SVR_TBL );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( MIB_VIRTUAL_SVR_TBL, i, (void*)c ) )
			continue;

		if( (c->InstanceNum==instnum) )
		{
			*chainID = i;
     			return 0;
     		}
	}
	
	return -1;
}
/*end portmapping utilities*/


#else
/*portmapping utilities*/
unsigned int getPortMappingMaxInstNum( unsigned char ifindex )
{
	unsigned int ret = 0;	
	PORTFW_T *p, port_entity;
	unsigned int total,i;
	
	total = mib_chain_total( PORTFW_ARRAY_T );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( PORTFW_ARRAY_T, i, (void*)p ) )
		{
			if(p->ifIndex==ifindex)
			{
				if(p->InstanceNum>ret) ret = p->InstanceNum;
			}
		}
	}

	return ret;
}

int getPortMappingCount( unsigned int ifindex )
{
	int count = 0;
	MIB_CE_PORT_FW_T *p, port_entity;
	unsigned int total,i;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		p = &port_entity;
		if( mib_chain_get( MIB_PORT_FW_TBL, i, (void*)p ) )
			if(p->ifIndex==ifindex) count++;
	}

	return count;
}

/*id starts from 0, 1, 2, ...*/
int getPortMappingByID( unsigned char ifindex, int id, PORTFW_T *c, unsigned int *chainID )
{
	unsigned int total,i;
	
	if( (id < 0) || (c==NULL) || (chainID==NULL) ) return -1;
	
	total = mib_chain_total( PORTFW_ARRAY_T );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( PORTFW_ARRAY_T, i, (void*)c ) )
			continue;

		if(c->ifIndex==ifindex)
		{
			id--;
			if(id==-1)
			{
				*chainID = i; 
				return 0;
			}
		}
	}
	
	return -1;
}

int getPortMappingByInstNum( unsigned char ifindex, unsigned int instnum, PORTFW_T *c, unsigned int *chainID )
{
	unsigned int total,i;
	
	if( (instnum == 0) || (c==NULL) || (chainID==NULL) ) return -1;
	
	total = mib_chain_total( PORTFW_ARRAY_T );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( PORTFW_ARRAY_T, i, (void*)c ) )
			continue;

		//if( (c->ifIndex==ifindex)&&(c->InstanceNum==instnum) )
		if( (c->InstanceNum==instnum) )
		{
			*chainID = i;
     			return 0;
     		}
	}
	
	return -1;
}
/*end portmapping utilities*/
#endif //#if 0 //keith remove
#endif

#if 0
/*for DefaultConnectionService, set/get default route */
int getDefaultRoute( char *name )
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( name==NULL ) return -1;
	name[0]=0;

#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned char dgw;
		if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
		{
			transfer2PathName( dgw, name );
		}
	}
#else	
#ifdef TELEFONICA_DEFAULT_CFG
	int num,k;
	MIB_CE_IP_ROUTE_T *fw=NULL, route_entity;
	unsigned char zeroip[IP_ADDR_LEN];
	char pathname[256];

	memset(zeroip,0,IP_ADDR_LEN);
	fw=&route_entity;
	num=getDynamicForwardingTotalNum();
	for(k=1;k<=num;k++){
		getDynamicForwardingEntryByInstNum(k,fw);
		if(!memcmp(fw->destID,zeroip,IP_ADDR_LEN)){
			if( transfer2PathName( fw->ifIndex, pathname )<0 )
				continue;
			else{
				total = mib_chain_total(MIB_ATM_VC_TBL);
				for(i=0;i<total;i++){
					pEntry = &vc_entity;
					if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
						continue;
					if(pEntry->ifIndex==fw->ifIndex && pEntry->dgw==1){
						strcpy(name,pathname);
						return 0;
					}
				}
			}
		}

	}
	name[0]=0;
#endif

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
		if(pEntry->dgw==1)
		{
			char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
			char ipstr[]="WANIPConnection";
			char pppstr[]="WANPPPConnection";
			char *pconn=NULL;
			unsigned int instnum=0;

			if( (pEntry->cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((pEntry->cmode==ADSL_BR1483)&&(pEntry->cmode==BRIDGE_PPPOE)) ||
#endif
			    (pEntry->cmode==ADSL_PPPoA) )
			{
				pconn = pppstr;
				instnum = pEntry->ConPPPInstNum;
			}else{
				pconn = ipstr;
				instnum = pEntry->ConIPInstNum;
			}

			if( pEntry->connDisable==0 )
			{
				sprintf( name, strfmt, pEntry->ConDevInstNum , pconn, instnum );
				break;
			}else
				return -1;

		}
	}
#endif
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	return 0;
}
#endif //#if 0

#if 0 //keith remove
int setDefaultRoute( char *name )
{
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	struct CWMP_LEAF *e=NULL;
	unsigned int devnum,ipnum, pppnum;
	int total,i;
	
	if(name==NULL) return -1;
	if( get_ParameterEntity( name, &e ) < 0 ) return -1;


#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned char dgw;
		dgw = transfer2IfIndex( name );
		if( dgw==0xff ) return -1;
		mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);
	}
#else
	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	if( (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return -1;

	//delete old/update new default route
	total = mib_chain_total(MIB_ATM_VC_TBL);
#ifdef CONFIG_BOA_WEB_E8B_CH
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if( (pEntry->ConDevInstNum==devnum) &&
		    (pEntry->ConIPInstNum==ipnum) &&
		    (pEntry->ConPPPInstNum==pppnum) ) //new default route
		{
			if(pEntry->applicationtype==2) // TR069 connection can not be default connection
				return -1;
		}
	}
#endif
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
		
		if( (pEntry->ConDevInstNum==devnum) &&
		    (pEntry->ConIPInstNum==ipnum) &&
		    (pEntry->ConPPPInstNum==pppnum) ) //new default route
		{
			pEntry->dgw=1;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		}else if(pEntry->dgw==1) //old default route
		{
			pEntry->dgw=0;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		}
	}
#endif
	return 0;
}
#endif //#if 0 //keith remove

#if defined(CTC_WAN_NAME)&&defined(CONFIG_BOA_WEB_E8B_CH)
int setDefaultRoutefore8b( MIB_CE_ATM_VC_T *oldEntry )
{
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	MIB_CE_ATM_VC_T *pEntry, vc_entity;
	struct CWMP_LEAF *e=NULL;
	unsigned int devnum,ipnum, pppnum;
	int total,i;


	//delete old/update new default route
	total = mib_chain_total(MIB_ATM_VC_TBL);

	oldEntry->dgw=1;

	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->dgw==1) //old default route
		{
			pEntry->dgw=0;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		}
	}

	return 0;
}
#endif

/*for DefaultConnectionService, set/get default route */
int getDefaultRouteIfaceName( char *name )
{
#if defined(MULTI_WAN_SUPPORT)
	aaaaa;
	int total,i;
	WANIFACE_T *pEntry,wan_entity;

	if( name==NULL ) return -1;
	name[0]=0;

#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned int dgw;
		if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
		{
			transfer2PathName( dgw, name );
		}
	}
#else	
	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&total);

	for( i=1; i<=total; i++ )
	{
		pEntry = &wan_entity;
		
		*((char *)pEntry) = (char)i;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)pEntry))
			continue;
		
		if(pEntry->dgw==1)
		{
			char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
			char ipstr[]="WANIPConnection";
			char pppstr[]="WANPPPConnection";
			char *pconn=NULL;
			unsigned int instnum=0;

			if( (pEntry->cmode==IP_PPP) 
#ifdef PPPOE_PASSTHROUGH
			   || ((pEntry->cmode==IP_BRIDGE)&&(pEntry->brmode==BRIDGE_PPPOE))
#endif
			    )
			{
				pconn = pppstr;
				instnum = pEntry->ConPPPInstNum;
			}else{
				pconn = ipstr;
				instnum = pEntry->ConIPInstNum;
			}

			if( pEntry->connDisable==0 )
			{
				sprintf( name, strfmt, pEntry->ConDevInstNum , pconn, instnum );
				break;
			}else
				return -1;

		}
	}
#endif
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.

#else
	//bbbbb;
	int wan_dhcp;
	char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.%s.1"; //wt-121v8-3.33, no trailing dot
	char ipstr[]="WANIPConnection";
	char pppstr[]="WANPPPConnection";

	mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
	if(wan_dhcp== PPPOE)
		sprintf( name, strfmt, pppstr );
	else
		sprintf( name, strfmt, ipstr );
#endif //#if defined(MULTI_WAN_SUPPORT)
	return 0;
} // end getDefaultRouteIfaceName

int setDefaultRouteIfaceName( char *name )
{
#if defined(MULTI_WAN_SUPPORT)
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	WANIFACE_T *pEntry, wan_entity;
	WANIFACE_T target[2];
	struct CWMP_LEAF *e=NULL;
	unsigned int devnum,ipnum, pppnum;
	int total,i;
	
	if(name==NULL) return -1;
	if( get_ParameterEntity( name, &e ) < 0 ) return -1;


#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned int dgw;
		dgw = transfer2IfIndex( name );
		if( dgw==DUMMY_IFINDEX ) return -1;
		mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);
	}
#else
	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	if( (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return -1;

	//delete old/update new default route
	
	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&total);
#ifdef CONFIG_BOA_WEB_E8B_CH
	for( i=1; i<=total; i++ )
	{
		pEntry = &wan_entity;
		*((char *)pEntry) = (char)i;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)pEntry))
			continue;


		if( (pEntry->ConDevInstNum==devnum) &&
		    (pEntry->ConIPInstNum==ipnum) &&
		    (pEntry->ConPPPInstNum==pppnum) ) //new default route
		{
			if(!(pEntry->applicationtype&X_CT_SRV_INTERNET)) // Only INTERNET connection can be default connection
				return -1;
		}
	}
#endif
	for( i=1; i<=total; i++ )
	{
		pEntry = &wan_entity;
		memset( &target[0], 0, sizeof( WANIFACE_T ) );
		memset( &target[1], 0, sizeof( WANIFACE_T ) );
		*((char *)pEntry) = (char)i;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)pEntry))
			continue;
		
		memcpy(&target[0], &wan_entity, sizeof(WANIFACE_T));
		if( (pEntry->ConDevInstNum==devnum) &&
		    (pEntry->ConIPInstNum==ipnum) &&
		    (pEntry->ConPPPInstNum==pppnum) ) //new default route
		{
			pEntry->dgw=1;
			memcpy(&target[1], &wan_entity, sizeof(WANIFACE_T));
			mib_set(MIB_WANIFACE_MOD, (void *)&target);
		}else if(pEntry->dgw==1) //old default route
		{
			pEntry->dgw=0;
			memcpy(&target[1], &wan_entity, sizeof(WANIFACE_T));
			mib_set(MIB_WANIFACE_MOD, (void *)&target);
		}
	}
#endif

#else

#endif //#if defined(MULTI_WAN_SUPPORT)
	return 0;
}

int transfer2IfName( char *name, char *ifname )
{
	struct CWMP_LEAF *e=NULL;
	
	if( (name==NULL) || ( ifname==NULL ) ) return -1;
	if( get_ParameterEntity( name, &e ) < 0 ) return -1;
	ifname[0]=0;
	
	if( strcmp( name, "InternetGatewayDevice.LANDevice.1" )==0 )
		strcpy( ifname, "br0" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1" )==0 )
		strcpy( ifname, "br0" );
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 ) //lan interface
		strcpy( ifname, "eth0" );
//#ifdef WLAN_SUPPORT
#if 0 //by cairui
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" )==0 )
	{
		CWMP_WLANCONF_T wlanConf;
		int rootIdx=0,vwlanIdx=0;
		char wlan_ifname[10]={0};
		
		getWLANIdxFromInstNum(1, &wlanConf, &rootIdx, &vwlanIdx);

		if(vwlanIdx == 0)
			sprintf(wlan_ifname,"wlan%d",rootIdx);
		else
			sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
		
		strcpy( ifname, wlan_ifname );
	}
#ifdef WLAN_MBSSID
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" )==0 )
	{
		CWMP_WLANCONF_T wlanConf;
		int rootIdx=0,vwlanIdx=0;
		char wlan_ifname[10]={0};
		
		getWLANIdxFromInstNum(2, &wlanConf, &rootIdx, &vwlanIdx);

		if(vwlanIdx == 0)
			sprintf(wlan_ifname,"wlan%d",rootIdx);
		else
			sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
		
		strcpy( ifname, wlan_ifname );
	}
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" )==0 )
	{
		CWMP_WLANCONF_T wlanConf;
		int rootIdx=0,vwlanIdx=0;
		char wlan_ifname[10]={0};
		
		getWLANIdxFromInstNum(3, &wlanConf, &rootIdx, &vwlanIdx);

		if(vwlanIdx == 0)
			sprintf(wlan_ifname,"wlan%d",rootIdx);
		else
			sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
		
		strcpy( ifname, wlan_ifname );
	}
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" )==0 )
	{
		CWMP_WLANCONF_T wlanConf;
		int rootIdx=0,vwlanIdx=0;
		char wlan_ifname[10]={0};
		
		getWLANIdxFromInstNum(4, &wlanConf, &rootIdx, &vwlanIdx);

		if(vwlanIdx == 0)
			sprintf(wlan_ifname,"wlan%d",rootIdx);
		else
			sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
		
		strcpy( ifname, wlan_ifname );
	}
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" )==0 )
	{
		CWMP_WLANCONF_T wlanConf;
		int rootIdx=0,vwlanIdx=0;
		char wlan_ifname[10]={0};
		
		getWLANIdxFromInstNum(5, &wlanConf, &rootIdx, &vwlanIdx);

		if(vwlanIdx == 0)
			sprintf(wlan_ifname,"wlan%d",rootIdx);
		else
			sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
		
		strcpy( ifname, wlan_ifname );
	}
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
	else if( strcmp( name, "InternetGatewayDevice.WANDevice.1.WANEthernetInterfaceConfig" )==0 ) //wan interface
		strcpy( ifname, "eth1" );
	//else all interfaces

	return 0;	
}

#if 0 //keith remove
unsigned char transfer2IfIndxfromIfName( char *ifname )
{
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	int total,i;
	unsigned char ifindex=0xff;
	
	if(ifname==NULL) return ifindex;
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		char tmp_ifname[32];
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
	
		if(pEntry->cmode == ADSL_PPPoE || pEntry->cmode == ADSL_PPPoA)
			sprintf( tmp_ifname, "ppp%u", PPP_INDEX(pEntry->ifIndex)  );
		else
			sprintf( tmp_ifname, "vc%u", VC_INDEX(pEntry->ifIndex)  );
		
		if( strcmp(ifname, tmp_ifname)==0 )
		{
			ifindex=pEntry->ifIndex;
			break;
		}
	}
	
	return ifindex;
}
#endif //#if 0 //keith remove

//#if 0 //keith remove
int transfer2PathNamefromItf( char *ifname, char *pathname )
{
	struct CWMP_LEAF *e=NULL;
	
	if( (ifname==NULL) || ( pathname==NULL ) ) return -1;
	pathname[0]=0;
	
	//if( strcmp( ifname, "br0" )==0 )
	//	strcpy( pathname, "InternetGatewayDevice.LANDevice.1" );
	//else 
	if( strcmp( ifname, "br0" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANHostConfigManagement.IPInterface.1" );
#ifdef CONFIG_EXT_SWITCH
  #ifdef CONFIG_BOA_WEB_E8B_CH
	else if( strcmp( ifname, "eth0_sw3" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" );
	else if( strcmp( ifname, "eth0_sw2" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" );
	else if( strcmp( ifname, "eth0_sw1" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" );
	else if( strcmp( ifname,  "eth0_sw0" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
  #else  //CONFIG_BOA_WEB_E8B_CH
	else if( strcmp( ifname, "eth0_sw0" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" );
	else if( strcmp( ifname, "eth0_sw1" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" );
	else if( strcmp( ifname, "eth0_sw2" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" );
	else if( strcmp( ifname,  "eth0_sw3" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
  #endif //CONFIG_BOA_WEB_E8B_CH
#else
	else if( strcmp( ifname, "eth0" )==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
#endif
	else if( strcmp( ifname, "eth1" )==0 ) // added by lynn_pu, 20150211
		strcpy( pathname, "InternetGatewayDevice.WANDevice.1.WANEthernetInterfaceConfig" );
#if 0
#ifdef WLAN_SUPPORT
	else if( strcmp( ifname, wlan_name[0])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" );
#ifdef WLAN_MBSSID
	else if( strcmp( ifname, wlan_name[1])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" );
	else if( strcmp( ifname, wlan_name[2])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" );
	else if( strcmp( ifname, wlan_name[3])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" );
	else if( strcmp( ifname, wlan_name[4])==0 )
		strcpy( pathname, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" );
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
	else //wan interface
	{
		MIB_CE_ATM_VC_T *pEntry,vc_entity;
		int total,i;
		unsigned char ifindex=0xff;

		total = mib_chain_total(MIB_ATM_VC_TBL);
		for( i=0; i<total; i++ )
		{
			char tmp_ifname[32];
			pEntry = &vc_entity;
			if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
				continue;
		
			if(pEntry->cmode == ADSL_PPPoE || pEntry->cmode == ADSL_PPPoA)
				sprintf( tmp_ifname, "ppp%u", PPP_INDEX(pEntry->ifIndex)  );
			else
				sprintf( tmp_ifname, "vc%u", VC_INDEX(pEntry->ifIndex)  );
			
			if( strcmp(ifname, tmp_ifname)==0 )
			{
				ifindex=pEntry->ifIndex;
				break;
			}
		}
		return transfer2PathName( ifindex, pathname );
	}
#endif
	return 0;	
}
//#endif //#if 0 //keith remove

unsigned int transfer2IfIndex( char *name )
{
#if defined(MULTI_WAN_SUPPORT)
	struct CWMP_LEAF *e=NULL;
	unsigned int ret=DUMMY_IFINDEX;
	
	if( name==NULL ) return ret;
	if( get_ParameterEntity( name, &e ) < 0 ) return ret;
	
	{
		unsigned int devnum,ipnum,pppnum;
		WANIFACE_T *pEntry,wan_entity;
		int total,i;
		
		devnum = getWANConDevInstNum( name );
		ipnum  = getWANIPConInstNum( name );
		pppnum = getWANPPPConInstNum( name );
		if( (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return -1;

		mib_get(MIB_WANIFACE_TBL_NUM, (void *)&total);
		for( i=1; i<=total; i++ )
		{
			pEntry = &wan_entity;

			*((char *)pEntry) = (char)i;
			if(!mib_get(MIB_WANIFACE_TBL, (void *)pEntry))
				continue;
			
			if( (pEntry->ConDevInstNum==devnum) &&
			    (pEntry->ConIPInstNum==ipnum) &&
			    (pEntry->ConPPPInstNum==pppnum) ) 
			{
				ret = pEntry->ifIndex;
				break;
			}
		}		
	}
	return ret;
#else
	return 1;
#endif
}

int transfer2PathName( unsigned int ifindex, char *name )
{
#if defined(MULTI_WAN_SUPPORT)
	int total,i;
	WANIFACE_T *pEntry, wan_entity;

	if( ifindex==DUMMY_IFINDEX ) return -1;
	if( name==NULL ) return -1;
	name[0]=0;
	
	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&total);
	for( i=1; i<=total; i++ )
	{
		pEntry = &wan_entity;
		
		*((char *)pEntry) = (char)i;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)pEntry))
			continue;
		
		if(pEntry->ifIndex==ifindex)
		{
			char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
			char ipstr[]="WANIPConnection";
			char pppstr[]="WANPPPConnection";
			char *pconn=NULL;
			unsigned int instnum=0;

			if( (pEntry->cmode==IP_PPP) 
#ifdef PPPOE_PASSTHROUGH
			    || ((pEntry->cmode==IP_BRIDGE)&&(pEntry->brmode==BRIDGE_PPPOE))
#endif
			    )
			{
				pconn = pppstr;
				instnum = pEntry->ConPPPInstNum;
			}else{
				pconn = ipstr;
				instnum = pEntry->ConIPInstNum;
			}

			if( pEntry->connDisable==0 )
			{
				sprintf( name, strfmt, pEntry->ConDevInstNum , pconn, instnum );
				break;
			}else
				return -1;
		}
	}
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
#else

	int wan_dhcp;
	char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.%s.1"; //wt-121v8-3.33, no trailing dot
	char ipstr[]="WANIPConnection";
	char pppstr[]="WANPPPConnection";

	mib_get( MIB_WAN_DHCP, (void *)&wan_dhcp);
	if(wan_dhcp== PPPOE)
		sprintf( name, strfmt, pppstr );
	else
		sprintf( name, strfmt, ipstr );
#endif
	return 0;
		
}

#if 0 //keith remove
int getAAL5CRCErrors( unsigned char vpi, unsigned short vci, unsigned int *count )
{
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;

	if( count==NULL ) return -1;
	*count = 0;
	
	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}
	
	mysio.number = 0;
	
	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return -1;
		}
		
		if (cfg.created == 0)
			continue;

		if( cfg.vpi==vpi && cfg.vci==vci )
		{
			*count = cfg.stat.rx_crc_error;
			break;
		}
		
	}
	(void)close(skfd);
	
		
	return 0;
}
#endif //#if 0 //keith remove

#if 0 //keith remove
#ifdef _USE_NEW_RIP_
int getRIPInfo( unsigned char ifIndex, unsigned char *ripmode )
{

	unsigned char ripOn=0;

	if( ripmode==NULL ) return -1;

	*ripmode = RIP_NONE;
	mib_get(MIB_RIP_ENABLE, (void *)&ripOn);
	if( ripOn!=0 )
	{
		unsigned int entryNum=0, i=0;
		MIB_CE_RIP_T Entry;
		entryNum = mib_chain_total(MIB_RIP_TBL);
		for( i=0; i<entryNum; i++ )
		{
			if (!mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry))
				continue;
			if(Entry.ifIndex==ifIndex)
			{
#ifdef CONFIG_BOA_WEB_E8B_CH
				*ripmode = Entry.version;
#else
				//read receiveMode.
				*ripmode = Entry.receiveMode;
#endif
				break;
			}
		}
	}

	return 0;

}
#endif

#if 0 //keith remove
int updateRIPInfo( unsigned char ifIndex, unsigned char newripmode )
{

	unsigned int entryNum=0, i=0, found=0;
	unsigned char vChar=0;
	MIB_CE_RIP_T Entry;
	entryNum = mib_chain_total(MIB_RIP_TBL);
	for( i=0; i<entryNum; i++ )
	{
		if (!mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry))
			continue;
		if(Entry.ifIndex==ifIndex)
		{
			found = 1;
			break;
		}
	}
	
	if(found)
	{
		//receiveMode can only accept: none, v1, v2, v1_v2
		switch( newripmode )
		{
		case RIP_NONE:
#ifdef CONFIG_BOA_WEB_E8B_CH
			mib_chain_delete(MIB_RIP_TBL, i);
			entryNum = mib_chain_total(MIB_RIP_TBL);
			if(entryNum==0)
			{
				vChar = 0;
				mib_set(MIB_RIP_ENABLE, (void *)&vChar);
			}
#else
			if(Entry.sendMode==RIP_NONE)
			{
				mib_chain_delete(MIB_RIP_TBL, i);
				entryNum = mib_chain_total(MIB_RIP_TBL);
				if(entryNum==0)
				{
					vChar = 0;
					mib_set(MIB_RIP_ENABLE, (void *)&vChar);
				}
			}else{
				Entry.receiveMode = newripmode;
				mib_chain_update( MIB_RIP_TBL, (unsigned char*)&Entry, i );
			}
#endif
			break;
		case RIP_V1:
		case RIP_V2:
		case RIP_V1_V2:
#ifdef CONFIG_BOA_WEB_E8B_CH
			Entry.version = newripmode;
			mib_chain_update( MIB_RIP_TBL, (unsigned char*)&Entry, i );
			vChar = 1;
			mib_set(MIB_RIP_ENABLE, (void *)&vChar);
#else
			Entry.receiveMode = newripmode;
			mib_chain_update( MIB_RIP_TBL, (unsigned char*)&Entry, i );
			vChar = 1;
			mib_set(MIB_RIP_ENABLE, (void *)&vChar);
#endif
			break;
		default:
			return -1;
		}
	}else{
		//receiveMode can only accept: none, v1, v2, v1_v2
		switch( newripmode )
		{
		case RIP_NONE:
			//nothing to do
			break;
		case RIP_V1:
		case RIP_V2:
		case RIP_V1_V2:
#ifdef CONFIG_BOA_WEB_E8B_CH
			memset( &Entry, 0, sizeof(Entry) );
			Entry.ifIndex = ifIndex;
			Entry.version = newripmode;
			mib_chain_add( MIB_RIP_TBL, (unsigned char*)&Entry );
			vChar = 1;
			mib_set(MIB_RIP_ENABLE, (void *)&vChar);
#else
			memset( &Entry, 0, sizeof(Entry) );
			Entry.ifIndex = ifIndex;
			Entry.sendMode = RIP_NONE;
			Entry.receiveMode = newripmode;
			mib_chain_add( MIB_RIP_TBL, (unsigned char*)&Entry );
			vChar = 1;
			mib_set(MIB_RIP_ENABLE, (void *)&vChar);
#endif
			break;
		default:
			return -1;
		}
	}
	
	return 0;

}
#else /*_USE_NEW_RIP_*/
//newripmode: 0:off, 1:ripv1, 2:ripv2
int updateRIPInfo( int chainid, unsigned char newripmode )
{

	int total,i;
	unsigned char use_rip, ripver,lanrip=0;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	if( newripmode>2 ) return -1;
	total = mib_chain_total(MIB_ATM_VC_TBL);
	if(chainid>=total) return -1;
	
	use_rip=0;
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(i==chainid)
		{
			pEntry->rip=(newripmode==0)?0:1;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		}
		
		if(pEntry->rip) use_rip=1;
	}
	
	mib_get(MIB_ADSL_LAN_RIP, (void *)&lanrip);
	if(lanrip) use_rip=1;
	
	mib_set(MIB_RIP_ENABLE, (void *)&use_rip);
	if(use_rip)
	{
		ripver = newripmode-1;
		mib_set(MIB_RIP_VERSION, (void *)&ripver);
	}
		
	return 0;

}
#endif /*_USE_NEW_RIP_*/
#endif //#if 0 //keith remove

/*ppp utilities*/
/*refer to fmstatus.c & utility.c*/
char *PPP_CONF = "/var/ppp/ppp.conf";
char *PPPOE_CONF = "/var/ppp/pppoe.conf";
char *PPPOA_CONF = "/var/ppp/pppoa.conf";
char *PPP_PID = "/var/run/spppd.pid";
int getPPPConStatus( char *pppname, char *status )
{
	char buff[256];
	FILE *fp;
	char strif[6],tmpst[32];
	int  ret=-1;

	if( (pppname==NULL) || (status==NULL) ) return -1;
	
	status[0]=0;
	if (!(fp=fopen(PPP_CONF, "r")))
	{
		fclose(fp);
		//fprintf( stderr, "%s not exists.\n", PPP_CONF);
		return -1;
	}else{
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			//if   dev   dev_v   gw phase          username                password
			if(sscanf(buff, "%s %*s %*s %*s %s", strif, tmpst)!=2) 
			{
				//fprintf( stderr, "Unsuported ppp configuration format\n");
				break;
			}
			if ( !strcmp(pppname, strif) )
			{
				strcpy( status, tmpst );
				ret=0;
				break;
			}
		}
		fclose(fp);
	}
	return ret;
}

#if 0 //keith remove
int getPPPUptime( char *pppname, int ppptype, unsigned int *uptime )
{
	char buff[256],*fname;
	FILE *fp;
	char strif[6],tmpst[64]={0};
	int  ret=-1,spid=0;

	if( (pppname==NULL) || (uptime==NULL) ||
	    ((ppptype!=ADSL_PPPoE) && (ppptype!=ADSL_PPPoA)) ) 
	      return -1;

	// get spppd pid
	if ((fp = fopen(PPP_PID, "r"))) {
		fscanf(fp, "%d\n", &spid);
		fclose(fp);
	}else{
		fprintf( stderr, "spppd pidfile not exists\n");	
		return -1;
	}
	if(spid) kill(spid, SIGUSR2);
	usleep(500);

	*uptime=0;
	if(ppptype==ADSL_PPPoE)
		fname=PPPOE_CONF;
	else
		fname=PPPOA_CONF;
	if (!(fp=fopen(fname, "r")))
	{
		fclose(fp);
		fprintf( stderr, "%s not exists.\n", fname);
		return -1;
	}else{
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			if(ppptype==ADSL_PPPoE)
			{	//if      dev     phase           MAC          AC_MAC       uptime	totaluptime
				if(sscanf(buff, "%s %*s %*s %*s %*s %s", strif, tmpst)!=2) 
				{
					fprintf( stderr, "Unsuported pppoe configuration format\n");
					break;
				}
			}else{	//if   dev     class  encaps qos  pcr   scr   mbs   uptime           totaluptime
				if(sscanf(buff, "%s %*s %*s %*s %*s %*s %*s %*s %s", strif, tmpst)!=2) 
				{
					fprintf( stderr, "Unsuported pppoa configuration format\n");
					break;
				}
			}

			if ( !strcmp(pppname, strif) )
			{
				break;
			}
		}
		fclose(fp);
	}
	
	{ //uptime format in the conf file    XXday(s),hh:mm:ss or 0sec
		if( strlen(tmpst)==0 ) return -1;
		
		if( strcmp( tmpst, "0sec" ) )
		{
			int day=0, hh=0, mm=0, ss=0;
			char *tok1,*tok2;
			
			tok1 = strtok( tmpst, "days," );
			tok2 = strtok( NULL, "days," );
			if(tok2!=NULL) //has days
				day=atoi( tok1 );
			else
				tok2=tok1;
			
			tok1 = strtok( tok2,":" );
			if(tok1) hh=atoi( tok1 );
			tok1 = strtok( NULL,":" );
			if(tok1) mm=atoi( tok1 );
			tok1 = strtok( NULL,":" );
			if(tok1) ss=atoi( tok1 );

			//convert to seconds
			*uptime =(unsigned int) ((day*24+hh)*60+mm)*60+ss;
		}else
			*uptime=0;
			
	}
	return 0;
}
#endif //#if 0 //keith remove

#if 0 //keith remove
int getPPPCurrentMRU( char *pppname, int ppptype, unsigned int *cmru )
{
	char buff[256];
	FILE *fp;
	char strif[6];
	int  ret=-1;

	if( (pppname==NULL) || (cmru==NULL) ||
	    ((ppptype!=ADSL_PPPoE) && (ppptype!=ADSL_PPPoA)) ) 
	      return -1;

	*cmru=0;
	if (!(fp=fopen(PPP_CONF, "r")))
	{
		fclose(fp);
		fprintf( stderr, "%s not exists.\n", PPP_CONF);
		return -1;
	}else{
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			//if   dev   dev_v   gw phase          username                password	mru
			if(sscanf(buff, "%s %*s %*s %*s %*s %*s %*s %u", strif, cmru)!=2) 
			{
				fprintf( stderr, "Unsuported ppp configuration format\n");
				break;
			}
			if ( !strcmp(pppname, strif) )
			{
				ret=0;
				break;
			}
		}
		fclose(fp);
	}
	return ret;
}
#endif //#if 0 //keith remove

#if 0 //keith remove
int getPPPLCPEcho( char *pppname, int ppptype, unsigned int *echo )
{
	FILE *fp;

	if( (pppname==NULL) || (echo==NULL) ||
	    ((ppptype!=ADSL_PPPoE) && (ppptype!=ADSL_PPPoA)) ) 
	      return -1;

	*echo=0;
	if (fp = fopen("/tmp/ppp_lcp_echo", "r")) {
		fscanf(fp, "%d", echo);
		fclose(fp);
		return 0;
	}
	else
		return -1;
}
#endif //#if 0 //keith remove

#if 0 //keith remove
int getPPPEchoRetry( char *pppname, int ppptype, unsigned int *retry )
{
	FILE *fp;

	if( (pppname==NULL) || (retry==NULL) ||
	    ((ppptype!=ADSL_PPPoE) && (ppptype!=ADSL_PPPoA)) ) 
	      return -1;

	*retry=0;
	if (fp = fopen("/tmp/ppp_echo_retry", "r")) {
		fscanf(fp, "%d", retry);
		fclose(fp);
		return 0;
	}
	else
		return -1;
}
#endif //#if 0 //keith remove
/*end ppp utility*/

#if 0 //keith remove
int getATMCellCnt( unsigned char vpi, unsigned short vci, unsigned int *txcnt, unsigned int *rxcnt )
{
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;

	
	if( txcnt==NULL || rxcnt==NULL ) return -1;
	*txcnt=0; *rxcnt=0;
	
	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}
	
	mysio.number = 0;
	
	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return -1;
		}
		
		if (cfg.created == 0)
			continue;

		
		if( cfg.vpi==vpi && cfg.vci==vci )
		{
			*txcnt = cfg.stat.tx_byte_cnt/48;
			*rxcnt = cfg.stat.rx_byte_cnt/48;
			break;
		}
	}
	(void)close(skfd);
	
	return 0;
}
#endif 

/* Jenny, refer to if_sppp.h */
enum ppp_last_connection_error {
	ERROR_NONE, ERROR_ISP_TIME_OUT, ERROR_COMMAND_ABORTED,
	ERROR_NOT_ENABLED_FOR_INTERNET, ERROR_BAD_PHONE_NUMBER,
	ERROR_USER_DISCONNECT, ERROR_ISP_DISCONNECT, ERROR_IDLE_DISCONNECT,
	ERROR_FORCED_DISCONNECT, ERROR_SERVER_OUT_OF_RESOURCES,
	ERROR_RESTRICTED_LOGON_HOURS, ERROR_ACCOUNT_DISABLED,
	ERROR_ACCOUNT_EXPIRED, ERROR_PASSWORD_EXPIRED,
	ERROR_AUTHENTICATION_FAILURE, ERROR_NO_DIALTONE, ERROR_NO_CARRIER,
	ERROR_NO_ANSWER, ERROR_LINE_BUSY, ERROR_UNSUPPORTED_BITSPERSECOND,
	ERROR_TOO_MANY_LINE_ERRORS, ERROR_IP_CONFIGURATION, ERROR_UNKNOWN
};

#if 0 //keith remove	
const char * getLastConnectionError(unsigned char ifindex)
{
	FILE *fp;
	char buff[10];
	int pppif;
	enum ppp_last_connection_error error;

	if (fp = fopen("/tmp/ppp_error_log", "r")) {
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			sscanf(buff, "%d:%d", &pppif, &error);
			if (pppif == PPP_INDEX(ifindex))
					break;
		}
		fclose(fp);
	}

	switch (error) {
		case ERROR_NONE:						return "ERROR_NONE";
		case ERROR_ISP_TIME_OUT:				return "ERROR_ISP_TIME_OUT";
		case ERROR_COMMAND_ABORTED:			return "ERROR_COMMAND_ABORTED";
		case ERROR_NOT_ENABLED_FOR_INTERNET: 	return "ERROR_NOT_ENABLED_FOR_INTERNET";
		case ERROR_BAD_PHONE_NUMBER:			return "ERROR_BAD_PHONE_NUMBER";
		case ERROR_USER_DISCONNECT:			return "ERROR_USER_DISCONNECT";
		case ERROR_ISP_DISCONNECT:				return "ERROR_ISP_DISCONNECT";
		case ERROR_IDLE_DISCONNECT:			return "ERROR_IDLE_DISCONNECT";
		case ERROR_FORCED_DISCONNECT: 		return "ERROR_FORCED_DISCONNECT";
		case ERROR_SERVER_OUT_OF_RESOURCES:	return "ERROR_SERVER_OUT_OF_RESOURCES";
		case ERROR_RESTRICTED_LOGON_HOURS:	return "ERROR_RESTRICTED_LOGON_HOURS";
		case ERROR_ACCOUNT_DISABLED:			return "ERROR_ACCOUNT_DISABLED";
		case ERROR_ACCOUNT_EXPIRED:			return "ERROR_ACCOUNT_EXPIRED";
		case ERROR_PASSWORD_EXPIRED: 			return "ERROR_PASSWORD_EXPIRED";
		case ERROR_AUTHENTICATION_FAILURE:		return "ERROR_AUTHENTICATION_FAILURE";
		case ERROR_NO_DIALTONE:				return "ERROR_NO_DIALTONE";
		case ERROR_NO_CARRIER:					return "ERROR_NO_CARRIER";
		case ERROR_NO_ANSWER:					return "ERROR_NO_ANSWER";
		case ERROR_LINE_BUSY: 					return "ERROR_LINE_BUSY";
		case ERROR_UNSUPPORTED_BITSPERSECOND:	return "ERROR_UNSUPPORTED_BITSPERSECOND";
		case ERROR_TOO_MANY_LINE_ERRORS:		return "ERROR_TOO_MANY_LINE_ERRORS";
		case ERROR_IP_CONFIGURATION:			return "ERROR_IP_CONFIGURATION";
		case ERROR_UNKNOWN: 					return "ERROR_UNKNOWN";
	}
	return "ERROR_UNKNOWN";
}
#endif //#if 0 //keith remove	

#if 0 //keith remove
int getFirstATMVCEntryByInstNum( unsigned int devnum, MIB_CE_ATM_VC_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (devnum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ) )
			continue;

		if( p->ConDevInstNum==devnum )
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}
#endif //#if 0 //keith remove

#if 0 //Keith remove. no support DSL
int getATMVCEntryByIPInstNum( unsigned int devnum, unsigned int ipnum, MIB_CE_ATM_VC_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (devnum==0) || (ipnum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ) )
			continue;

		if( (p->ConDevInstNum==devnum) && (p->ConIPInstNum==ipnum) )
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}

#if 0 //keith remove. no support DSL	
int getATMVCEntryByPPPInstNum( unsigned int devnum, unsigned int pppnum, MIB_CE_ATM_VC_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (devnum==0) || (pppnum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ) )
			continue;

		if( (p->ConDevInstNum==devnum) && (p->ConPPPInstNum==pppnum) )
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}
#endif //#if 0 //keith remove. no support DSL	

int resetATMVCConnection( MIB_CE_ATM_VC_T *p )
{
	MIB_CE_ATM_VC_T bkentry;
	
	if(p==NULL) return -1;
	
	memset( &bkentry, 0, sizeof(MIB_CE_ATM_VC_T) );
	bkentry.ifIndex = p->ifIndex;
	bkentry.vpi = p->vpi;
	bkentry.qos = p->qos;
	bkentry.vci = p->vci;
	bkentry.pcr = p->pcr;
	bkentry.scr = p->scr;
	bkentry.mbs = p->mbs;
	bkentry.cdvt = p->cdvt;
	bkentry.encap = p->encap;
	bkentry.cmode = p->cmode;
	bkentry.brmode = p->brmode;
	bkentry.mtu = p->mtu;
	bkentry.enable = p->enable;
	bkentry.connDisable = 1;
	bkentry.ConDevInstNum = p->ConDevInstNum;
	
	memcpy( p, &bkentry, sizeof(MIB_CE_ATM_VC_T) );
	
	return 0;
}
#endif //#if 0 //Keith remove. no support DSL

#ifdef _PRMT_X_CT_COM_WANEXT_
int convertFlag2ServiceList( unsigned short flag, char *buf )
{
	int buflen=0;

	if(buf==NULL) return -1;

	if( flag&X_CT_SRV_TR069 )
		strcat( buf, "TR069," );
	if( flag&X_CT_SRV_INTERNET )
		strcat( buf, "INTERNET," );
	if( flag&X_CT_SRV_OTHER )
		strcat( buf, "OTHER," );
	buflen = strlen(buf);
	if(buflen>0) buf[buflen-1]=0;
	return 0;
}
int convertServiceList2Flag( char *buf, unsigned short *flag )
{
	char *tok;
	if( (buf==NULL) || (flag==NULL) ) return -1;

	*flag = 0;
	tok = strtok( buf, ", \t\r\n" );
	while(tok)
	{
		if( strcmp( tok, "TR069" )==0 )
			*flag |= X_CT_SRV_TR069;
		else if( strcmp( tok, "INTERNET" )==0 )
			*flag |= X_CT_SRV_INTERNET;
		else if( strcmp( tok, "OTHER" )==0 )
			*flag |= X_CT_SRV_OTHER;
		else{	//skip unknown toks or return error?
			*flag = 0;
			return -1;
		}
		tok = strtok( NULL, ", \t\r\n" );
	}

	return 0;
}

#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
#ifdef WLAN_SUPPORT
char *sWLANIF[WLAN_IF_NUM]={
			"InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" //wlan0
#ifdef WLAN_MBSSID
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" //wlan0-vap0
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" //wlan0-vap1
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" //wlan0-vap2
			, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" //wlan0-vap3
#endif //WLAN_MBSSID
		};
#endif //WLAN_SUPPORT
#ifdef CONFIG_BOA_WEB_E8B_CH
char *sLANIF[4]={	"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1", //eth0_sw0
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2", //eth0_sw1
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3", //eth0_sw2
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4"  //eth0_sw3
		 };
#else
char *sLANIF[4]={	"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4", //eth0_sw0
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3", //eth0_sw1
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2", //eth0_sw2
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1"  //eth0_sw3
		 };
#endif

int getLanInterface( unsigned char itfGroup, char *buf )
{
	unsigned char mode=0;
	int i;

	if(buf==NULL) return -1;

	buf[0]=0;
	mib_get(MIB_MPMODE, (void *)&mode);
	if( (mode&0x01)==0 ) return 0;

	if(itfGroup!=0)
	{
#ifdef WLAN_SUPPORT
		{
			unsigned char wlan_group=0;
			mib_get(MIB_WLAN_ITF_GROUP, (void *)&wlan_group);
			if(wlan_group==itfGroup)
				strcat( buf, sWLANIF[0] );

#ifdef WLAN_MBSSID
			for(i=1;i<5;i++)
			{
				wlan_group=0;
				mib_get(MIB_WLAN_VAP0_ITF_GROUP+i-1, (void *)&wlan_group);
				if(wlan_group==itfGroup)
				{
					if(buf[0]!=0) strcat( buf, "," );
					strcat( buf, sWLANIF[i] );
				}
			}
#endif //WLAN_MBSSID
		}
#endif //WLAN_SUPPORT

		{
			MIB_CE_SW_PORT_T Port;
			for (i=0; i<SW_PORT_NUM; i++)
			{
				if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Port))
					continue;
				if( Port.itfGroup==itfGroup )
				{
					if(buf[0]!=0) strcat( buf, "," );
					strcat( buf, sLANIF[i] );
				}
			}
		}
	}
	return 0;
}

int setLanInterface( char *buf, MIB_CE_ATM_VC_T *pEntry )
{
	char *tok;
	int fsw0=0, fsw1=0, fsw2=0, fsw3=0, fwan=0;
	unsigned char gsw0=0, gsw1=0, gsw2=0, gsw3=0;
#ifdef WLAN_SUPPORT
	int fwlan0=0, fwlan1=0, fwlan2=0, fwlan3=0, fwlan4=0;
	unsigned char gwlan0=0, gwlan1=0, gwlan2=0, gwlan3=0, gwlan4=0;
#endif //WLAN_SUPPORT
	MIB_CE_SW_PORT_T Port;
	unsigned char group=0,def_group=0;

	if( (buf==NULL) || (pEntry==NULL) ) return ERR_9002;

	if( strlen(buf)==0 )
	{
		MIB_CE_ATM_VC_T pvcEntry;
		unsigned int vcNum,i;
		if( pEntry->itfGroup==def_group ) return 0; //no change
		//check if other wan interfaces are in the same group
		vcNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<vcNum; i++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
				continue;
			if( (pvcEntry.ifIndex!=pEntry->ifIndex) &&
			    (pvcEntry.itfGroup==pEntry->itfGroup) )
			{
				//just remove the pEntry from this group
				pEntry->itfGroup=def_group;
				return 0;
			}
		}
		fwan=0; //delete
	}else
		fwan=1;	//add
#ifdef CONFIG_BOA_WEB_E8B_CH
if(pEntry->cmode==ADSL_PPPoE) {

		MIB_CE_ATM_VC_T pvcEntry;
		unsigned int vcNum;
		int i;
                 vcNum = mib_chain_total(MIB_ATM_VC_TBL);
			for (i=0; i<vcNum; i++)
			{
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
					continue;
				if( (pvcEntry.cmode==ADSL_PPPoE)&&(pvcEntry.ifIndex!=pEntry->ifIndex) &&
				    (pvcEntry.vpi==pEntry->vpi) &&(pvcEntry.vci==pEntry->vci))
				{

					pEntry->itfGroup=pvcEntry.itfGroup;

				}
			}

	   }

#endif


	//(buf's length>0)  or (buf==0 and no other interface in the same group)
	tok = strtok( buf, ", \t\r\n" );
	while( tok )
	{
		int i=0;
#ifdef CONFIG_BOA_WEB_E8B_CH
		if( strstr( tok, sLANIF[0] )!=0 ) fsw0=1;
		else if( strstr( tok, sLANIF[1] )!=0 ) fsw1=1;
		else if( strstr( tok, sLANIF[2] )!=0 ) fsw2=1;
		else if( strstr( tok, sLANIF[3] )!=0 ) fsw3=1;
#ifdef WLAN_SUPPORT
		else if( strstr( tok, sWLANIF[0] )!=0 ) fwlan0=1;
#ifdef WLAN_MBSSID
		else if( strstr( tok, sWLANIF[1] )!=0 ) fwlan1=1;
		else if( strstr( tok, sWLANIF[2] )!=0 ) fwlan2=1;
		else if( strstr( tok, sWLANIF[3] )!=0 ) fwlan3=1;
		else if( strstr( tok, sWLANIF[4] )!=0 ) fwlan4=1;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		else
			return ERR_9007;
#else
		if( strcmp( tok, sLANIF[0] )==0 ) fsw0=1;
		else if( strcmp( tok, sLANIF[1] )==0 ) fsw1=1;
		else if( strcmp( tok, sLANIF[2] )==0 ) fsw2=1;
		else if( strcmp( tok, sLANIF[3] )==0 ) fsw3=1;
#ifdef WLAN_SUPPORT
		else if( strcmp( tok, sWLANIF[0] )==0 ) fwlan0=1;
#ifdef WLAN_MBSSID
		else if( strcmp( tok, sWLANIF[1] )==0 ) fwlan1=1;
		else if( strcmp( tok, sWLANIF[2] )==0 ) fwlan2=1;
		else if( strcmp( tok, sWLANIF[3] )==0 ) fwlan3=1;
		else if( strcmp( tok, sWLANIF[4] )==0 ) fwlan4=1;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		else
			return ERR_9007;
#endif
		tok = strtok( NULL, ", \t\r\n" );
	}


	//get all information about switch's and wlan's group
	{
#ifdef WLAN_SUPPORT
		mib_get(MIB_WLAN_ITF_GROUP, (void *)&gwlan0);
#ifdef WLAN_MBSSID
		mib_get(MIB_WLAN_VAP0_ITF_GROUP, (void *)&gwlan1);
		mib_get(MIB_WLAN_VAP1_ITF_GROUP, (void *)&gwlan2);
		mib_get(MIB_WLAN_VAP2_ITF_GROUP, (void *)&gwlan3);
		mib_get(MIB_WLAN_VAP3_ITF_GROUP, (void *)&gwlan4);
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		if( mib_chain_get(MIB_SW_PORT_TBL, 0, (void *)&Port) )
			gsw0=Port.itfGroup;
		if( mib_chain_get(MIB_SW_PORT_TBL, 1, (void *)&Port) )
			gsw1=Port.itfGroup;
		if( mib_chain_get(MIB_SW_PORT_TBL, 2, (void *)&Port) )
			gsw2=Port.itfGroup;
		if( mib_chain_get(MIB_SW_PORT_TBL, 3, (void *)&Port) )
			gsw3=Port.itfGroup;
	}

	//found if there are mixed groups
	{
		if( (fwan==1)&&(pEntry->itfGroup!=0) )  group=pEntry->itfGroup;
#ifdef WLAN_SUPPORT
		else if( (fwlan0==1)&&(gwlan0!=0) ) group=gwlan0;
#ifdef WLAN_MBSSID
		else if( (fwlan1==1)&&(gwlan1!=0) ) group=gwlan1;
		else if( (fwlan2==1)&&(gwlan2!=0) ) group=gwlan2;
		else if( (fwlan3==1)&&(gwlan3!=0) ) group=gwlan3;
		else if( (fwlan4==1)&&(gwlan4!=0) ) group=gwlan4;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		else if( (fsw0==1)&&(gsw0!=0) ) group=gsw0;
		else if( (fsw1==1)&&(gsw1!=0) ) group=gsw1;
		else if( (fsw2==1)&&(gsw2!=0) ) group=gsw2;
		else if( (fsw3==1)&&(gsw3!=0) ) group=gsw3;
		else group=0;

		if( (group!=0) &&
		      (((fwan==1)&&(pEntry->itfGroup!=0)&&(group!=pEntry->itfGroup)) ||
#ifdef WLAN_SUPPORT
		       ((fwlan0==1)&&(gwlan0!=0)&&(group!=gwlan0)) ||
#ifdef WLAN_MBSSID
		       ((fwlan1==1)&&(gwlan1!=0)&&(group!=gwlan1)) ||
		       ((fwlan2==1)&&(gwlan2!=0)&&(group!=gwlan2)) ||
		       ((fwlan3==1)&&(gwlan3!=0)&&(group!=gwlan3)) ||
		       ((fwlan4==1)&&(gwlan4!=0)&&(group!=gwlan4)) ||
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		       ((fsw0==1)&&(gsw0!=0)&&(group!=gsw0)) ||
		       ((fsw1==1)&&(gsw1!=0)&&(group!=gsw1)) ||
		       ((fsw2==1)&&(gsw2!=0)&&(group!=gsw2)) ||
		       ((fsw3==1)&&(gsw3!=0)&&(group!=gsw3))
		  ))
		{
			return ERR_9001;
		}
	}

	if( (fwan==1)&&(group==0) ) //found a avaliable group num;
	{
		struct itfInfo itfs[16];
		int ifnum,i;
		for( i=1;i<IFGROUP_NUM;i++ ) //from 1 to IFGROUP_NUM-1, 0:default group
		{
			ifnum = get_group_ifinfo(itfs, 16, i);
			if(ifnum==0)
			{
				group = i;
				break;
			}
		}

		if(group==0) return ERR_9004;
	}else if( fwan==0 ) //delete
		group = pEntry->itfGroup;

	//enable the portmapping function
	if(fwan==1) //add
	{
		unsigned char mode=0;
		mib_get(MIB_MPMODE, (void *)&mode);
		mode |= 0x01;
		mib_set(MIB_MPMODE, (void *)&mode);
	}

	//update the wan's group
	if( fwan==1 )
		pEntry->itfGroup = group; //call mib_chain_update() after reutrn
	else
		pEntry->itfGroup = def_group; //call mib_chain_update() after reutrn

#ifdef WLAN_SUPPORT
	{ //update wlan's group
		if( fwlan0==1 ) //add
			mib_set(MIB_WLAN_ITF_GROUP, (void *)&group);
		else if( (fwlan0==0) && ( gwlan0==group ) ) //delete
			mib_set(MIB_WLAN_ITF_GROUP, (void *)&def_group);
#ifdef WLAN_MBSSID
		if( fwlan1==1 ) //add
			mib_set(MIB_WLAN_VAP0_ITF_GROUP, (void *)&group);
		else if( (fwlan1==0) && ( gwlan1==group ) ) //delete
			mib_set(MIB_WLAN_VAP0_ITF_GROUP, (void *)&def_group);
		if( fwlan2==1 ) //add
			mib_set(MIB_WLAN_VAP1_ITF_GROUP, (void *)&group);
		else if( (fwlan2==0) && ( gwlan2==group ) ) //delete
			mib_set(MIB_WLAN_VAP1_ITF_GROUP, (void *)&def_group);
		if( fwlan3==1 ) //add
			mib_set(MIB_WLAN_VAP2_ITF_GROUP, (void *)&group);
		else if( (fwlan3==0) && ( gwlan3==group ) ) //delete
			mib_set(MIB_WLAN_VAP2_ITF_GROUP, (void *)&def_group);
		if( fwlan4==1 ) //add
			mib_set(MIB_WLAN_VAP3_ITF_GROUP, (void *)&group);
		else if( (fwlan4==0) && ( gwlan4==group ) ) //delete
			mib_set(MIB_WLAN_VAP3_ITF_GROUP, (void *)&def_group);
#endif //WLAN_MBSSID
	}
#endif //WLAN_SUPPORT

	{//update lan port's group
		int i;
		for( i=0;i<SW_PORT_NUM;i++ )
		{
			if( !mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Port) )
				continue;
			if( i==0 )
			{
				if(fsw0==1) //add
					Port.itfGroup=group;
				else if( (fsw0==0) && ( gsw0==group ) ) //delete
					Port.itfGroup=def_group;
			}else if( i==1 )
			{
				if(fsw1==1) //add
					Port.itfGroup=group;
				else if( (fsw1==0) && ( gsw1==group ) ) //delete
					Port.itfGroup=def_group;
			}else if( i==2 )
			{
				if(fsw2==1) //add
					Port.itfGroup=group;
				else if( (fsw2==0) && ( gsw2==group ) ) //delete
					Port.itfGroup=def_group;
			}else if( i==3 )
			{
				if(fsw3==1) //add
					Port.itfGroup=group;
				else if( (fsw3==0) && ( gsw3==group ) ) //delete
					Port.itfGroup=def_group;
			}

			mib_chain_update( MIB_SW_PORT_TBL, (unsigned char*)&Port, i );
		}
	}

	return 0;

}
#endif //defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
#endif //_PRMT_X_CT_COM_WANEXT_
