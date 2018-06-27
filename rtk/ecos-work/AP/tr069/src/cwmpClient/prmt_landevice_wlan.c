#include "prmt_landevice_wlan.h"
#include "prmt_landevice.h"
// by cairui
#define	IFF_UP		0x1		/* interface is up from ./96c-92c-gw_install/include/net/if.h:139*/
//#include <linux/wireless.h>
//#include "../../../auth/src/dlisten/wireless.h"
//#include "mibtbl.h"

//#include <rtk/utility.h>
#ifdef WLAN_SUPPORT

#ifdef TELEFONICA_DEFAULT_CFG
#define _SHOW_WLAN_KEY_WHEN_GET_
#endif //TELEFONICA_DEFAULT_CFG

#define MAX_STA_NUM 31

#define GETPSKINDEX(a,b) ( (unsigned char)(((a&0xf)<<4)|(b&0xf)) )
#define	WLANUPDATETIME	90

//CONFIG_WLAN_SETTING_T cwmpWlan, *pcwmpWlan;

char		gLocationDescription[4096]={0};
char		gWLANAssociations[ sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1) ];
time_t		gWLANAssUpdateTime=0;
unsigned int	gWLANTotalClients=0;
int		gWLANIDForAssInfo=-1; /*-1:no info, 0:wlan0, 1:wlan0-vap0, 2:wlan-vap1.....*/
int MaxWLANIface=(NUM_WLAN_INTERFACE*NUM_VWLAN_INTERFACE)+NUM_WLAN_INTERFACE;


extern int clone_wlaninfo_set(CONFIG_WLAN_SETTING_T *wlanptr, int rootwlan_idx,int vwlan_idx,int Newrootwlan_idx,int Newvwlan_idx, int ChangeRFBand );
extern int _is_hex(char c);
extern int string_to_hex(char *string, unsigned char *key, int len);
int getWLANIdxFromInstNum(int instnum, CWMP_WLANCONF_T *pwlanConf, int *rootIdx, int *vwlan_idx);
int getNewWLANIdxFromReq(int *rootIdx, int *vwlan_idx, int ReqBand);
int updateWLANAssociations(void);
int loadWLANAssInfoByInstNum( unsigned int instnum );
int getWLANSTAINFO(int id, WLAN_STA_INFO_T *info);
int getRateStr( unsigned short rate, char *buf );
int setRateStr( char *buf, unsigned short *rate );
int getIPbyMAC( char *mac, char *ip );

unsigned int getWLANConfInstNum( char *name );
unsigned int getWEPInstNum( char *name );
unsigned int getAssDevInstNum( char *name );
unsigned int getPreSharedKeyInstNum( char *name );

#ifdef E8B_NEW_DIAGNOSE
void writeSSIDFile(char *msg, int index)
{
	FILE *fp;
	char buf[32];

	fp = fopen(NEW_SETTING, "r+");
	if (fp) {
		fgets(buf, sizeof(buf), fp);
		if (strcmp(buf, SSID_FILE)) {
			clearssidfile();
		} else {
			rewind(fp);
			ftruncate(fileno(fp), 0);
			fprintf(fp, "%s", SSID_FILE);
		}
		fclose(fp);
	}

	sprintf(buf, "%s%d", SSID_FILE, index);
	fp = fopen(buf, "w");
	if (fp) {
		fprintf(fp, msg);
		fclose(fp);

		fp = fopen(REMOTE_SETSAVE_FILE, "r");
		if (fp) {
			fclose(fp);
			unlink(REMOTE_SETSAVE_FILE);
			va_cmd("/bin/flatfsd", 1, 1, "-s");
		}
	}
}
#endif

struct CWMP_OP tPreSharedKeyEntityLeafOP = { getPreSharedKeyEntity, setPreSharedKeyEntity };
struct CWMP_PRMT tPreSharedKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PreSharedKey",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP},
{"KeyPassphrase",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP},
{"AssociatedDeviceMACAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP}
};
enum ePreSharedKeyEntityLeaf
{
	ePreSharedKey,
	eKeyPassphrase,
	ePreAssociatedDeviceMACAddress
};
struct CWMP_LEAF tPreSharedKeyEntityLeaf[] =
{
{ &tPreSharedKeyEntityLeafInfo[ePreSharedKey] },
{ &tPreSharedKeyEntityLeafInfo[eKeyPassphrase] },
{ &tPreSharedKeyEntityLeafInfo[ePreAssociatedDeviceMACAddress] },
{ NULL }
};


struct CWMP_PRMT tPreSharedKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"5",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"6",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"7",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"8",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"9",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"10",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum ePreSharedKeyObject
{
	ePreSharedKey1,
	ePreSharedKey2,
	ePreSharedKey3,
	ePreSharedKey4,
	ePreSharedKey5,
	ePreSharedKey6,
	ePreSharedKey7,
	ePreSharedKey8,
	ePreSharedKey9,
	ePreSharedKey10
};
struct CWMP_NODE tPreSharedKeyObject[] =
{
/*info,  					leaf,				next)*/
{&tPreSharedKeyObjectInfo[ePreSharedKey1],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey2],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey3],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey4],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey5],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey6],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey7],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey8],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey9],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey10],	tPreSharedKeyEntityLeaf,	NULL},
{NULL,						NULL,				NULL}
};


struct CWMP_OP tWEPKeyEntityLeafOP = { getWEPKeyEntity, setWEPKeyEntity };
struct CWMP_PRMT tWEPKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WEPKey",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWEPKeyEntityLeafOP}
};
enum eWEPKeyEntityLeaf
{
	eWEPKey
};
struct CWMP_LEAF tWEPKeyEntityLeaf[] =
{
{ &tWEPKeyEntityLeafInfo[eWEPKey] },
{ NULL }
};


struct CWMP_PRMT tWEPKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eWEPKeyObject
{
	eWEP1,
	eWEP2,
	eWEP3,
	eWEP4
};
struct CWMP_NODE tWEPKeyObject[] =
{
/*info,  			leaf,			next)*/
{&tWEPKeyObjectInfo[eWEP1],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP2],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP3],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP4],	tWEPKeyEntityLeaf,	NULL},
{NULL,				NULL,			NULL}
};


struct CWMP_OP tAscDeviceEntityLeafOP = { getAscDeviceEntity,NULL };
struct CWMP_PRMT tAscDeviceEntityLeafInfo[] =
{
/*(name,				type,		flag,				op)*/
{"AssociatedDeviceMACAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"AssociatedDeviceIPAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"AssociatedDeviceAuthenticationState",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastRequestedUnicastCipher",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastRequestedMulticastCipher",	eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastPMKId",				eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP}
};
enum eAscDeviceEntityLeaf
{
	eAssociatedDeviceMACAddress,
	eAssociatedDeviceIPAddress,
	eAssociatedDeviceAuthenticationState,
	eLastRequestedUnicastCipher,
	eLastRequestedMulticastCipher,
	eLastPMKId
};
struct CWMP_LEAF tAscDeviceEntityLeaf[] =
{
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceMACAddress] },
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceIPAddress] },
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceAuthenticationState] },
{ &tAscDeviceEntityLeafInfo[eLastRequestedUnicastCipher] },
{ &tAscDeviceEntityLeafInfo[eLastRequestedMulticastCipher] },
{ &tAscDeviceEntityLeafInfo[eLastPMKId] },
{ NULL }
};


struct CWMP_PRMT tAscDeviceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eAscDeviceObject
{
	eAscDevice0
};
struct CWMP_LINKNODE tAscDeviceObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tAscDeviceObjectInfo[eAscDevice0],	tAscDeviceEntityLeaf,	NULL,		NULL,			0}
};



struct CWMP_OP tWLANConfEntityLeafOP = { getWLANConf, setWLANConf };
struct CWMP_PRMT tWLANConfEntityLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"Enable",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"Status",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"BSSID",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"MaxBitRate",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"Channel",				eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"SSID",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"BeaconType",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#ifdef MAC_FILTER
{"MACAddressControlEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#endif /*MAC_FILTER*/
#ifdef _PRMT_X_CT_COM_WLAN_
{"Standard",					eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tWLANConfEntityLeafOP},
#else
{"Standard",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
#endif
{"WEPKeyIndex",				eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"KeyPassphrase",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#ifdef _PRMT_X_CT_COM_WLAN_	//cathy
{"WEPEncryptionLevel",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tWLANConfEntityLeafOP},
#else
{"WEPEncryptionLevel",			eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
#endif
{"BasicEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"BasicAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"WPAEncryptionModes",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"WPAAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"IEEE11iEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"IEEE11iAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"PossibleChannels",			eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"BasicDataTransmitRates",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"OperationalDataTransmitRates",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"PossibleDataTransmitRates",		eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
/*InsecureOOBAccessEnabled*/
{"BeaconAdvertisementEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"SSIDAdvertisementEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP}, /*version 1.4*/
{"RadioEnabled",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"AutoRateFallBackEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"LocationDescription",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
/*RequlatoryDomain*/
/*TotalPSKFailures*/
/*TotalIntegrityFailures*/
{"ChannelsInUse",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"DeviceOperationMode",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
/*DistanceFromRoot*/
/*PeerBSSID*/
{"AuthenticationServiceMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"TotalBytesSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalBytesReceived",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
#ifdef _PRMT_X_CT_COM_WLAN_
{"X_CT-COM_SSIDHide",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_RFBand", 			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_ChannelWidth",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_GuardInterval",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_RetryTimeout",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_Powerlevel",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_PowerValue",			eCWMP_tUINT,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"X_CT-COM_APModuleEnable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tWLANConfEntityLeafOP},
{"X_CT-COM_WPSKeyWord", 		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#endif //_PRMT_X_CT_COM_WLAN_
#ifdef CUSTOMIZE_MIDDLE_EAST
{"X_DLINK_COM_Guest1MaxClients",eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,&tWLANConfEntityLeafOP},
{"X_DLINK_COM_HT",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#endif //CUSTOMIZE_MIDDLE_EAST
{"TotalAssociations",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP}
};
enum eWLANConfEntityLeafInfo
{
	eWL_Enable,
	eWL_Status,
	eWL_BSSID,
	eWL_MaxBitRate,
	eWL_Channel,
	eWL_SSID,
	eWL_BeaconType,
#ifdef MAC_FILTER
	eWL_MACAddressControlEnabled,
#endif
	eWL_Standard,
	eWL_WEPKeyIndex,
	eWL_KeyPassphrase,
	eWL_WEPEncryptionLevel,
	eWL_BasicEncryptionModes,
	eWL_BasicAuthenticationMode,
	eWL_WPAEncryptionModes,
	eWL_WPAAuthenticationMode,
	eWL_IEEE11iEncryptionModes,
	eWL_IEEE11iAuthenticationMode,
	eWL_PossibleChannels,
	eWL_BasicDataTransmitRates,
	eWL_OperationalDataTransmitRates,
	eWL_PossibleDataTransmitRates,
	/*InsecureOOBAccessEnabled*/
	eWL_BeaconAdvertisementEnabled,
	eWL_SSIDAdvertisementEnabled,
	eWL_RadioEnabled,
	eWL_AutoRateFallBackEnabled,
	eWL_LocationDescription,
	/*RequlatoryDomain*/
	/*TotalPSKFailures*/
	/*TotalIntegrityFailures*/
	eWL_ChannelsInUse,
	eWL_DeviceOperationMode,
	/*DistanceFromRoot*/
	/*PeerBSSID*/
	eWL_AuthenticationServiceMode,
	eWL_TotalBytesSent,
	eWL_TotalBytesReceived,
	eWL_TotalPacketsSent,
	eWL_TotalPacketsReceived,
#ifdef _PRMT_X_CT_COM_WLAN_
	eWL_X_CTCOM_SSIDHide,
	eWL_X_CTCOM_RFBand,
	eWL_X_CTCOM_ChannelWidth,
	eWL_X_CTCOM_GuardInterval,
	eWL_X_CTCOM_RetryTimeout,
	eWL_X_CTCOM_Powerlevel,
	eWL_X_CTCOM_PowerValue,
	eWL_X_CTCOM_APModuleEnable,
	eWL_X_CTCOM_WPSKeyWord,
#endif //_PRMT_X_CT_COM_WLAN_
#ifdef CUSTOMIZE_MIDDLE_EAST	
	eWL_X_DLINK_COM_Guest1MaxClients,
	eWL_X_DLINK_COM_HT,
#endif //CUSTOMIZE_MIDDLE_EAST
	eWL_TotalAssociations
};
struct CWMP_LEAF tWLANConfEntityLeaf[] =
{
{ &tWLANConfEntityLeafInfo[eWL_Enable] },
{ &tWLANConfEntityLeafInfo[eWL_Status] },
{ &tWLANConfEntityLeafInfo[eWL_BSSID] },
{ &tWLANConfEntityLeafInfo[eWL_MaxBitRate] },
{ &tWLANConfEntityLeafInfo[eWL_Channel] },
{ &tWLANConfEntityLeafInfo[eWL_SSID] },
{ &tWLANConfEntityLeafInfo[eWL_BeaconType] },
#ifdef MAC_FILTER
{ &tWLANConfEntityLeafInfo[eWL_MACAddressControlEnabled] },
#endif
{ &tWLANConfEntityLeafInfo[eWL_Standard] },
{ &tWLANConfEntityLeafInfo[eWL_WEPKeyIndex] },
{ &tWLANConfEntityLeafInfo[eWL_KeyPassphrase] },
{ &tWLANConfEntityLeafInfo[eWL_WEPEncryptionLevel] },
{ &tWLANConfEntityLeafInfo[eWL_BasicEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_BasicAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_WPAEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_WPAAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_IEEE11iEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_IEEE11iAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_PossibleChannels] },
{ &tWLANConfEntityLeafInfo[eWL_BasicDataTransmitRates] },
{ &tWLANConfEntityLeafInfo[eWL_OperationalDataTransmitRates] },
{ &tWLANConfEntityLeafInfo[eWL_PossibleDataTransmitRates] },
/*InsecureOOBAccessEnabled*/
{ &tWLANConfEntityLeafInfo[eWL_BeaconAdvertisementEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_SSIDAdvertisementEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_RadioEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_AutoRateFallBackEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_LocationDescription] },
/*RequlatoryDomain*/
/*TotalPSKFailures*/
/*TotalIntegrityFailures*/
{ &tWLANConfEntityLeafInfo[eWL_ChannelsInUse] },
{ &tWLANConfEntityLeafInfo[eWL_DeviceOperationMode] },
/*DistanceFromRoot*/
/*PeerBSSID*/
{ &tWLANConfEntityLeafInfo[eWL_AuthenticationServiceMode] },
{ &tWLANConfEntityLeafInfo[eWL_TotalBytesSent] },
{ &tWLANConfEntityLeafInfo[eWL_TotalBytesReceived] },
{ &tWLANConfEntityLeafInfo[eWL_TotalPacketsSent] },
{ &tWLANConfEntityLeafInfo[eWL_TotalPacketsReceived] },
#ifdef _PRMT_X_CT_COM_WLAN_

{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_SSIDHide] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_RFBand] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_ChannelWidth] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_GuardInterval] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_RetryTimeout] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_Powerlevel] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_PowerValue] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_APModuleEnable] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_WPSKeyWord] },

#endif //_PRMT_X_CT_COM_WLAN_
#ifdef CUSTOMIZE_MIDDLE_EAST
{ &tWLANConfEntityLeafInfo[eWL_X_DLINK_COM_Guest1MaxClients] },	
{ &tWLANConfEntityLeafInfo[eWL_X_DLINK_COM_HT] },	
#endif //CUSTOMIZE_MIDDLE_EAST
{ &tWLANConfEntityLeafInfo[eWL_TotalAssociations] },
{ NULL }
};

struct CWMP_OP tWLAN_AssociatedDevice_OP = { NULL, objAscDevice };
struct CWMP_PRMT tWLANConfEntityObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"AssociatedDevice",			eCWMP_tOBJECT,	CWMP_READ,		&tWLAN_AssociatedDevice_OP},
{"WEPKey",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"PreSharedKey",			eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eWLANConfEntityObject
{
	eWLAN_AssociatedDevice,
	eWLAN_WEPKey,
	eWLAN_PreSharedKey
};
struct CWMP_NODE tWLANConfEntityObject[] =
{
/*info,  						leaf,	next)*/
{&tWLANConfEntityObjectInfo[eWLAN_AssociatedDevice],	NULL,	NULL},
{&tWLANConfEntityObjectInfo[eWLAN_WEPKey],		NULL,	tWEPKeyObject},
{&tWLANConfEntityObjectInfo[eWLAN_PreSharedKey],	NULL,	tPreSharedKeyObject},
{NULL,							NULL,	NULL}
};



struct CWMP_PRMT tWLANConfigObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef WLAN_MBSSID
{"2",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"3",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"4",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"5",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
};
enum eWLANConfigObject
{
	eWLAN1,
#ifdef WLAN_MBSSID
	eWLAN2,
	eWLAN3,
	eWLAN4,
	eWLAN5
#endif
};
struct CWMP_NODE tWLANConfigObject[] =
{
/*info,  			leaf,			node)*/
{ &tWLANConfigObjectInfo[eWLAN1],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
#ifdef WLAN_MBSSID
{ &tWLANConfigObjectInfo[eWLAN2],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
{ &tWLANConfigObjectInfo[eWLAN3],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
{ &tWLANConfigObjectInfo[eWLAN4],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
{ &tWLANConfigObjectInfo[eWLAN5],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
#endif
{ NULL,				NULL,			NULL}
};

#if 1//def CTCOM_WLAN_REQ
struct CWMP_PRMT tWLANObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWLANObject
{
	eWLAN0
};
struct CWMP_LINKNODE tWLANObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tWLANObjectInfo[eWLAN0],	tWLANConfEntityLeaf,	tWLANConfEntityObject,		NULL,			0}

};
#endif

int getPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	int		id=0,chain_id=0, wlaninst=0;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	unsigned char	buf[128]="";
	int rootIdx=0,vwlanIdx=0;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>MaxWLANIface )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif

	id = getPreSharedKeyInstNum( name );
	if( (id<1) || (id>10) ) return ERR_9007;


	*type = entity->info->type;
	*data = NULL;
	

	getWLANIdxFromInstNum(wlaninst, &wlanConf, &rootIdx, &vwlanIdx);
	
	if( strcmp( lastname, "PreSharedKey" )==0 )
	{
		if( id==1 )
		{
			unsigned int pskfmt;//0:Passphrase,   1:hex
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
			if(pskfmt==1)
			{
				getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
				*data = strdup( buf );
			}else
				*data = strdup( "" );
		}
		return 0;
	}
	else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		if( id==1 )
		{
			unsigned int pskfmt;//0:Passphrase,   1:hex
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
			if(pskfmt==0)
	{
				getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
				*data = strdup( buf );
			}else
				*data = strdup( "" );
		}
		return 0;
	}
	else if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		*data = strdup( "" );
	}else{
		return ERR_9005;
	}
	return 0;
}

int setPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char		*lastname = entity->info->name;
	int	id=0, chain_id=0, wlaninst=0;
	char		*tok;
	char		*buf=data;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	int rootIdx=0,vwlanIdx=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>MaxWLANIface )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif

	id = getPreSharedKeyInstNum( name );
	if( (id<1) || (id>10) ) return ERR_9007;

	
	getWLANIdxFromInstNum(wlaninst, &wlanConf, &rootIdx, &vwlanIdx);
	
	if( strcmp( lastname, "PreSharedKey" )==0 )
	{
		int i,len;
		if( buf==NULL ) return ERR_9007;
		
		len = strlen(buf);
		if( len==0 || len>64 ) return ERR_9007;
	
		for( i=0; i<len; i++ )
			if( _is_hex(buf[i])==0 ) return ERR_9007;
		
		if( id==1 ) //also update MIB_WLAN_WPA_PSK
		{
			unsigned char pskfmt;
			
			pskfmt = 1;//0:Passphrase,   1:hex
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
			
#ifdef _CWMP_APPLY_
			apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
			return 0;
		#else
			return 1;
		#endif
		}

		return 0;
	}
	else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)<8) || (strlen(buf)>63) ) return ERR_9007;

		
		if( id==1 ) //also update MIB_WLAN_WPA_PSK
		{
			unsigned char pskfmt;
			pskfmt = 0; //0:Passphrase,   1:hex
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
#ifdef _CWMP_APPLY_
			apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
			return 1;
#endif
		}
		
		return 0;
	}
	else if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int	keyid=0;
	char	*tok;
	unsigned int wlaninst=0,wepinst=0;;
	unsigned char	hex_key[32], ascii_key[32];
	unsigned int keyfmt = WEP64; 	//0:disable, 1:64, 2:128
	int rootIdx=0,vwlanIdx=0;
	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>MaxWLANIface )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif
	wepinst = getWEPInstNum( name );
	if( wepinst<1 || wepinst>4 )	return ERR_9007;


	getWLANIdxFromInstNum(wlaninst, &wlanConf, &rootIdx, &vwlanIdx);
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WEPKey" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&keyfmt);
		keyid = wepinst;
		switch( keyid )
		{
			case 1:
				if(keyfmt == WEP64)
				{
					getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY1, (void *)ascii_key);
				}
				else
				{
					getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY1, (void *)ascii_key);
				}
				break;
			case 2:
				if(keyfmt == WEP64)
				{
#ifdef WLAN_MBSSID
					if( wlaninst!=1 )
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						//memcpy(ascii_key, EntryMbssid.wep64Key2, sizeof(EntryMbssid.wep64Key2));
						bzero(ascii_key, sizeof(ascii_key));
					else
#endif //WLAN_MBSSID
						getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY2, (void *)ascii_key);
				}
				else
				{
#ifdef WLAN_MBSSID
					if( wlaninst!=1 )
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						//memcpy(ascii_key, EntryMbssid.wep128Key2, sizeof(EntryMbssid.wep128Key2));
						bzero(ascii_key, sizeof(ascii_key));
					else
#endif //WLAN_MBSSID

						getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY2, (void *)ascii_key);
				}
				break;
		case 3:
				if(keyfmt == WEP64)
				{
#ifdef WLAN_MBSSID
					if( wlaninst!=1 )
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						//memcpy(ascii_key, EntryMbssid.wep64Key3, sizeof(EntryMbssid.wep64Key3));
						bzero(ascii_key, sizeof(ascii_key));
					else
#endif //WLAN_MBSSID
						getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY3, (void *)ascii_key);
				}
				else
				{
#ifdef WLAN_MBSSID
					if( wlaninst!=1 )
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						//memcpy(ascii_key, EntryMbssid.wep128Key3, sizeof(EntryMbssid.wep128Key3));
						bzero(ascii_key, sizeof(ascii_key));
					else
#endif //WLAN_MBSSID
						getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY3, (void *)ascii_key);
				}
				break;
			case 4:
				if(keyfmt == WEP64)
				{
#ifdef WLAN_MBSSID
					if( wlaninst!=1 )
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						//memcpy(ascii_key, EntryMbssid.wep64Key4, sizeof(EntryMbssid.wep64Key4));
						bzero(ascii_key, sizeof(ascii_key));
					else
#endif //WLAN_MBSSID
						getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY4, (void *)ascii_key);
				}
				else
				{
#ifdef WLAN_MBSSID
					if( wlaninst!=1 )
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						//memcpy(ascii_key, EntryMbssid.wep128Key4, sizeof(EntryMbssid.wep128Key4));
						bzero(ascii_key, sizeof(ascii_key));
					else
#endif //WLAN_MBSSID
						getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY4, (void *)ascii_key);
				}
				break;
			default:
				return ERR_9005;
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
	}else{
		return ERR_9005;
	}
	return 0;
}

int setWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int wlaninst=0,wepinst=0;
	int rootIdx=0,vwlanIdx=0;
	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>MaxWLANIface )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif
	wepinst = getWEPInstNum( name );
	if( wepinst<1 || wepinst>4 )	return ERR_9007;

	
	getWLANIdxFromInstNum(wlaninst, &wlanConf, &rootIdx, &vwlanIdx);

	if( strcmp( lastname, "WEPKey" )==0 )
	{
		unsigned char ascii_key[32];
		int len=0, keyid;
		unsigned int keyfmt,key_type;

		if( buf==NULL ) return ERR_9007;
		len = strlen(buf);
		if( (len!=10) && (len!=26) ) return ERR_9007;
		memset( ascii_key, 0, sizeof(ascii_key) );
		if(!string_to_hex(buf, ascii_key, len)) return ERR_9007;

		keyfmt = (len==10)?WEP64:WEP128; //key format==>0:disable, 1:64, 2:128
		key_type = KEY_HEX; //key type==>KEY_ASCII:ascii, KEY_HEX:hex, tr-069 always uses the hex format.
		
		
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&keyfmt);
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP_KEY_TYPE, (void *)&key_type);
		keyid = wepinst;
		//printf("\n~~ascii_key=%s,keyfmt=%d,keyid=%d\n\n", ascii_key, keyfmt, keyid);
				switch( keyid )
		{			
					case 1:
					if(keyfmt==WEP64)
					{
						setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY1, (void *)ascii_key);
		}
		else
		{
						setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY1, (void *)ascii_key);
		}
					break;
					case 2:
					if(keyfmt==WEP64)
					{
				#ifdef WLAN_MBSSID
						if(wlaninst != 1)
							// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
							//memcpy( EntryMbssid.wep64Key2, ascii_key, sizeof(EntryMbssid.wep64Key2) );
							return ERR_9000;
						else
		#endif
							setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY2, (void *)ascii_key);
					}
		else
		{
				#ifdef WLAN_MBSSID
						if(wlaninst != 1)
							// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
							//memcpy( EntryMbssid.wep128Key2, ascii_key, sizeof(EntryMbssid.wep128Key2) );
							return ERR_9000;
						else
		#endif
							setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY2, (void *)ascii_key);
		}
					break;
					case 3:
					if(keyfmt==WEP64)
		{
				#ifdef WLAN_MBSSID
						if(wlaninst != 1)
							// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
							//memcpy( EntryMbssid.wep64Key3, ascii_key, sizeof(EntryMbssid.wep64Key3) );
							return ERR_9000;
						else
				#endif
						setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY3, (void *)ascii_key);
			
					}
					else
			{
				#ifdef WLAN_MBSSID
						if(wlaninst != 1)
							// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
							//memcpy( EntryMbssid.wep128Key3, ascii_key, sizeof(EntryMbssid.wep128Key3) );
							return ERR_9000;
						else
						#endif
							setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY3, (void *)ascii_key);
					}
							break;
					case 4:
					if(keyfmt==WEP64)
					{
				#ifdef WLAN_MBSSID
						if(wlaninst != 1)
							// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
							//memcpy( EntryMbssid.wep64Key4, ascii_key, sizeof(EntryMbssid.wep64Key4) );
							return ERR_9000;
						else
						#endif
							setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP64_KEY4, (void *)ascii_key);
			}
			else
			{
				#ifdef WLAN_MBSSID
						if(wlaninst != 1)
							// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
							//memcpy( EntryMbssid.wep128Key4, ascii_key, sizeof(EntryMbssid.wep128Key4) );
							return ERR_9000;
						else
						#endif
							setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP128_KEY4, (void *)ascii_key);
					}
							break;			
					default:
							return ERR_9005;		
				}
				
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int getAscDeviceEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	device_id=0;
	WLAN_STA_INFO_T info;
	char		*tok=NULL;
	unsigned int wlaninst=0;

	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>MaxWLANIface )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif

	if( loadWLANAssInfoByInstNum(wlaninst)<0 ) return ERR_9002;
	
	device_id = getAssDevInstNum(name);
	
	if( device_id<1 || device_id>gWLANTotalClients ) return ERR_9005;
	
	if( getWLANSTAINFO( device_id-1, &info )<0 ) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		char buf[32];
		sprintf( buf, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "AssociatedDeviceIPAddress" )==0 )
	{
		char aip[32]="",amac[32]="";
		sprintf( amac, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		if( getIPbyMAC( amac, aip ) < 0 )
			*data = strdup( "" );
		else
			*data = strdup( aip );	
	}
	else if( strcmp( lastname, "AssociatedDeviceAuthenticationState" )==0 )
	{
		int i = ((info.flag & STA_INFO_FLAG_ASOC)==STA_INFO_FLAG_ASOC);
		*data = intdup( i );
	}
	else if( strcmp( lastname, "LastRequestedUnicastCipher" )==0 )
	{
		*data = strdup( "" );
	}
	else if( strcmp( lastname, "LastRequestedMulticastCipher" )==0 )
	{
		*data = strdup( "" );
	}
	else if( strcmp( lastname, "LastPMKId" )==0 )
	{
		*data = strdup( "" );
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int objAscDevice(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	unsigned int wlaninst=0;
		
	wlaninst = getWLANConfInstNum( name );
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM ) return ERR_9007;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		
		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		
		*c=NULL;

		loadWLANAssInfoByInstNum(wlaninst);
		if(gWLANTotalClients>0)
			return create_Object( c, tAscDeviceObject, sizeof(tAscDeviceObject), gWLANTotalClients, 1 );
		
		return 0;
		break;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
	     	unsigned int num,i;
	     	struct CWMP_LINKNODE *old_table;

		loadWLANAssInfoByInstNum(wlaninst);
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
			}else{ 
				unsigned int InstNum=i+1;
				add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tAscDeviceObject, sizeof(tAscDeviceObject), &InstNum );
			}
	     	}
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE*)old_table );
		return 0;
	     }
	}

	return -1;
}
#if 1//def CTCOM_WLAN_REQ

int getCwmpWlanConById(int w_idx, int vw_idx, CWMP_WLANCONF_T *pwlanConf)
{
	unsigned int ret = 0;	
	CWMP_WLANCONF_T wlanCon_entity;
	unsigned int total,i;
	
	mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&total);
	for( i=1;i<=total;i++ )
	{

		*((char *)&wlanCon_entity) = (char)i;
		if( mib_get( MIB_CWMP_WLANCONF_TBL, (void*)&wlanCon_entity ) )
		{
			if(wlanCon_entity.RootIdx == w_idx &&  wlanCon_entity.VWlanIdx == vw_idx)
			{
				memcpy(pwlanConf, &wlanCon_entity, sizeof(CWMP_WLANCONF_T));
				ret = 1;
				break;
			}
		}
	}

	return ret;
}

int getCwmpWlanConMaxInstNum()
{
	unsigned int ret = 0;	
	CWMP_WLANCONF_T wlanCon_entity;
	unsigned int total,i;
	

	mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&total);
	for( i=1;i<=total;i++ )
	{

		*((char *)&wlanCon_entity) = (char)i;
		if( mib_get( MIB_CWMP_WLANCONF_TBL, (void*)&wlanCon_entity ) )
		{
			if(wlanCon_entity.InstanceNum > ret) 
				ret = wlanCon_entity.InstanceNum;
		}
	}
	return ret;
}

int objWLANConfiguration(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	int maxWLAN=NUM_WLAN_INTERFACE;
	int maxVWLAN=(NUM_VWLAN_INTERFACE*NUM_WLAN_INTERFACE);
	int i, j, instnum=0;
	int rootwlanIdx=0, vwlanIdx=0;
	int vInt=0;
	int num;
	int vWlanCount=0;
	CWMP_WLANCONF_T *pwlanConf, wlanconf_entity;
	CWMP_WLANCONF_T target[2];
	CWMPDBG( 1, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int MaxInstNum=0;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		
		CWMPDBG( 1, ( stderr, "<%s:%d>eCWMP_tINITOBJ name:%s(MaxInstNum:%d)\n", __FUNCTION__, __LINE__, name,MaxInstNum ) );
		//mbssid:

//#if defined(MBSSID)
		
		mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);
		pwlanConf = &wlanconf_entity;
		vWlanCount=0;
		for( i=1; i<=num;i++ )
		{
			*((char *)pwlanConf) = (char)i;
			if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
				continue;


			if(pwlanConf->IsConfigured && pwlanConf->InstanceNum)
			{
				vInt = pwlanConf->InstanceNum;
				MaxInstNum ++;
				if( create_Object( c, tWLANObject, sizeof(tWLANObject), 1, vInt ) < 0 )
					return -1;
			}
		}	


//#endif

		add_objectNum( name, MaxInstNum );
	return 0;
		}
	case eCWMP_tADDOBJ:
		{
	     	int ret;
		
		int *datap = (int *)data;

	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

#if defined(MBSSID)

		mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);
		pwlanConf = &wlanconf_entity;
		vWlanCount=0;
		for( i=1; i<=num;i++ )
		{
			*((char *)pwlanConf) = (char)i;
			if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
				continue;

			if(i < (maxWLAN+1) ) //skip root interface
				continue;
			
			if(vWlanCount > maxVWLAN)
				break;
			
			if( pwlanConf->IsConfigured==0 )
			{
				memcpy(&target[0], pwlanConf, sizeof(CWMP_WLANCONF_T));
				break;
			}
			vWlanCount++;
		}
		if(vWlanCount==maxVWLAN)
			return ERR_9004;
		
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWLANObject, sizeof(tWLANObject), data );
		if( ret >= 0 )
		{
			pwlanConf->InstanceNum = *(int*)data;
			pwlanConf->IsConfigured = 1;
			//getWlanMib(pwlanConf->RootIdx, pwlanConf->VWlanIdx, MIB_WLAN_PHY_BAND_SELECT, (void *)&vInt);
			//pwlanConf->RfBand =vInt;
			memcpy(&target[1], pwlanConf, sizeof(CWMP_WLANCONF_T));
			mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);
			ret=1;
		}
		return ret;
#else
		return ERR_9004;		
		
#endif
		}
	case eCWMP_tDELOBJ:
	     {
	     	int ret;	     	
		unsigned int *pUint=data;
		
		if(*pUint==1)
			return ERR_9001;
		
#if defined(MBSSID)
		
		mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);
		pwlanConf = &wlanconf_entity;
		vWlanCount=0;
		for( i=1; i<=num;i++ )
		{
			*((char *)pwlanConf) = (char)i;
			if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
				continue;

			if(i < (maxWLAN+1) ) //skip root interface
				continue;
			
			if(vWlanCount > maxVWLAN)
				break;
			
			if( pwlanConf->InstanceNum==*pUint )
			{
				int w_idx, vw_idx;
				CWMP_WLANCONF_T wlanConf;
					
				memcpy(&target[0], pwlanConf, sizeof(CWMP_WLANCONF_T));
				//pwlanConf->InstanceNum = 0;
				pwlanConf->IsConfigured = 0;
				memcpy(&target[1], pwlanConf, sizeof(CWMP_WLANCONF_T));
				mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);

				/* disable wlan interface */
				if(getWLANIdxFromInstNum(pwlanConf->InstanceNum, &wlanConf, &w_idx, &vw_idx) == 0)
				{
					char wlan_ifname[10] = {0};
					if(vw_idx == 0)
						sprintf(wlan_ifname, "wlan%d",w_idx);
					else
						sprintf(wlan_ifname, "wlan%d-va%d",w_idx,vw_idx-1);

					apmib_save_wlanIdx();
					if(SetWlan_idx(wlan_ifname))
					{
						int wlan_disabled = 1;
						mib_set( MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled);
					}
					apmib_recov_wlanIdx();

				}
				break;
			}
			vWlanCount++;
		}
		if(vWlanCount==maxVWLAN)
			return ERR_9004;
		
		
		//ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		ret = 0;
		if(ret==0)
		{
			ret=1;
		}
		return ret;
#else
		return ERR_9005;
#endif
		
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			int w_idx, vw_idx;

			old_table = (struct CWMP_LINKNODE *)entity->next;
		     	entity->next = NULL;

				
			for(w_idx=0;w_idx<NUM_WLAN_INTERFACE;w_idx++)
			{
				for(vw_idx=0;vw_idx<NUM_VWLAN_INTERFACE;vw_idx++)
				{
					char wlan_ifname[10] = {0};
					if(vw_idx == 0)
						sprintf(wlan_ifname, "wlan%d",w_idx);
					else
						sprintf(wlan_ifname, "wlan%d-va%d",w_idx,vw_idx-1);

					apmib_save_wlanIdx();
					if(SetWlan_idx(wlan_ifname))
					{
						int wlan_disabled = 1;
						
						mib_get( MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled);

 						memset( &wlanconf_entity, 0, sizeof( CWMP_WLANCONF_T ) );

						if( getCwmpWlanConById( w_idx, vw_idx, &wlanconf_entity ) )
						{
							
							struct CWMP_LINKNODE *remove_entity=NULL;
							memset( &target[0], 0, sizeof( CWMP_WLANCONF_T ) );
							memset( &target[1], 0, sizeof( CWMP_WLANCONF_T ) );

							remove_entity = remove_SiblingEntity( &old_table, wlanconf_entity.InstanceNum );
							memcpy(&target[0], &wlanconf_entity, sizeof(CWMP_WLANCONF_T));
						
							if(wlan_disabled== 0 || wlanconf_entity.IsConfigured == 1)
							{
								if( remove_entity!=NULL )
								{
									add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
								}
								else
								{							
									unsigned int MaxInstNum=getCwmpWlanConMaxInstNum();

									if(wlanconf_entity.InstanceNum == 0)
										MaxInstNum++;
									else
										MaxInstNum = wlanconf_entity.InstanceNum;

									
									add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWLANObject, sizeof(tWLANObject), &MaxInstNum );

									wlanconf_entity.IsConfigured = 1;
									wlanconf_entity.InstanceNum = MaxInstNum;
									memcpy(&target[1], &wlanconf_entity, sizeof(CWMP_WLANCONF_T));
									mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);

								}
							}							
						}
					}

					apmib_recov_wlanIdx();
				}
			}

			if( old_table )
	     		{
	     		
						destroy_ParameterTable( (struct CWMP_NODE *)old_table );
					}


			
			return 0;
		}
	}
		
	return -1;
}
#endif
		
int getWLANConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
		{
	char	*lastname = entity->info->name;
	unsigned long bs=0,br=0,ps=0,pr=0;
	unsigned char buf[256]="";
	unsigned int vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
	int rootIdx=0,vwlanIdx=0;
	bss_info bss;
	unsigned int GetValue=0;
	unsigned short uShort;
	unsigned int wpa_cipher=0, wpa2_cipher=0;
	char wlan_ifname[10]={0};
	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>MaxWLANIface )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif
			
				
	getWLANIdxFromInstNum(wlaninst, &wlanConf, &rootIdx, &vwlanIdx);

	if(vwlanIdx == 0)
		sprintf(wlan_ifname,"wlan%d",rootIdx);
	else
		sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
	
	//CWMPDBG( 1, ( stderr, "<%s:%d>target:<%d,%d>\n", __FUNCTION__, __LINE__, rootIdx,vwlanIdx ) );
		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		int func_off=0;
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
		//getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&func_off);

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
	else if( strcmp( lastname, "BSSID" )==0 )
	{
		getWlanMib(rootIdx, 0,MIB_WLAN_WLAN_DISABLED,(void *)&vUint);
		if(vUint == 0){
			getWlanBssInfo(rootIdx, 0, (void*)&bss);
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x", bss.bssid[0], bss.bssid[1], bss.bssid[2], bss.bssid[3], bss.bssid[4], bss.bssid[5]);	
		}
		else
		{
			strcpy(buf,"00:00:00:00:00:00");
		}		

		*data=strdup(buf);
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		if( vChar==1 )
			*data=strdup( "Auto" );
			else
			{
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_BAND, (void *)&vChar);
			if( vChar==1 ) //2.4 GHz (B)
				*data = strdup( "11" );
			else if( vChar==2 )//2.4 GHz (G)
				*data = strdup( "54" );
			else if( vChar==3 )//2.4 GHz (B+G)
				*data = strdup( "54" );
			else /*0, wifi_g==4, or wifi_bg==5?????*/
				*data = strdup( "" );//return ERR_9002;
			}
		}
	else if( strcmp( lastname, "Channel" )==0 )
		{

		
		getWlanMib(rootIdx, 0, MIB_WLAN_CHANNEL, (void *)&vChar);
		*data = uintdup( (unsigned int)vChar );
		}
	else if( strcmp( lastname, "SSID" )==0 )
	{
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_SSID, (void *)buf);
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "BeaconType" )==0 )
	{
#if 1//def CTCOM_WLAN_REQ	//cathy
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
		if(vChar==ENCRYPT_DISABLED)
				*data = strdup( "None" );
		else if(vChar==ENCRYPT_WEP)
			*data = strdup( "Basic" );
#else
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
		if(vChar==1) //disabled, so no beacon type
			{
				*data = strdup( "None" );
				return 0;
			}

		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
		getWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA_CIPHER_SUITE,(void *)&wpa_cipher);
		getWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA2_CIPHER_SUITE,(void *)&wpa2_cipher);
		
		if( vChar >= ENCRYPT_WPA && vChar < ENCRYPT_WAPI ){
			if(vChar == ENCRYPT_WPA && wpa_cipher ==WPA_CIPHER_TKIP)
				vChar = ENCRYPT_WPA_TKIP;
			else if (vChar == ENCRYPT_WPA && wpa_cipher ==WPA_CIPHER_AES)
				vChar = ENCRYPT_WPA_AES;
			else if (vChar == ENCRYPT_WPA2 && wpa2_cipher ==WPA_CIPHER_TKIP)
				vChar = ENCRYPT_WPA2_TKIP;
			else if (vChar == ENCRYPT_WPA2 && wpa2_cipher ==WPA_CIPHER_AES)
				vChar = ENCRYPT_WPA2_AES;
		}
		if( (vChar==ENCRYPT_WEP) || (vChar==ENCRYPT_DISABLED) )
			*data = strdup( "Basic" );
#endif
		else if( vChar==ENCRYPT_WPA_TKIP|| vChar==ENCRYPT_WPA_AES)
			*data = strdup( "WPA" );
		else if( vChar==ENCRYPT_WPA2_AES /*IEEE 802.11i*/
			|| vChar==ENCRYPT_WPA2_TKIP
				)
			*data = strdup( "11i" );
		else if( vChar==ENCRYPT_WPA2_MIXED ) /*WPA & WPA2*/
			*data = strdup( "WPAand11i" );
		else
			return ERR_9002;
	}
	
#ifdef _CWMP_MAC_FILTER_	
	else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		mib_get(MIB_CWMP_MACFILTER_WLAN_MAC_CTRL, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}
#endif /*_CWMP_MAC_FILTER_*/	
	else if( strcmp( lastname, "Standard" )==0 )
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
		else if( vChar==(BAND_11B|BAND_11G|BAND_11N))
			*data = strdup( "b,g,n" );
		else if( vChar==BAND_5G_11AN)
			*data = strdup( "a,n" );
		else if( vChar==(BAND_11G|BAND_11N))
			*data = strdup( "g,n" );
		else if( vChar==(BAND_11A | BAND_11N))
			*data = strdup( "a,n" );
		else if( vChar==(BAND_11A | BAND_11N | BAND_11AC))
			*data = strdup( "a,n,ac" );
		else if( vChar==(BAND_11N | BAND_11AC))
			*data = strdup( "n,ac" );
		else if( vChar==(BAND_11A | BAND_11AC))
			*data = strdup( "a,ac" );
		else 
			*data = strdup( "" ); 		
	}
	else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar);
		vChar = vChar + 1;//mib's wepid is from 0 to 3
		*data = uintdup( (unsigned int)vChar );
	}
	else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
#if 1//def CTCOM_WLAN_REQ	//cathy
			//0:disable, 1:64, 2:128
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&vChar);

		if(vChar == 0)
				*data = strdup("Disabled");
		else if (vChar == 1)
				*data = strdup("40-bit");
		else
				*data = strdup("104-bit");
#else
		*data = strdup( "Disabled,40-bit,104-bit" );
#endif
	}
	else if( strcmp( lastname, "BasicEncryptionModes" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
		if( vChar==ENCRYPT_WEP )
			*data = strdup( "WEPEncryption" );
		else if( vChar==ENCRYPT_DISABLED )
			*data = strdup( "None" );
		else
		{			
			*data = strdup( "None" );
		}
		}
	else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
		{			
#if 1//def CTCOM_WLAN_REQ	//cathy
	getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_AUTH_TYPE, (void *)&vChar);
		if(vChar==0)
			*data = strdup( "OpenSystem" );
		else if(vChar==1)
			*data = strdup( "SharedKey" );
		else
			*data = strdup( "Both" );
#else
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENABLE_1X, (void *)&vChar);
		if(vChar)
			*data = strdup( "EAPAuthentication" );
		else
			*data = strdup( "None" );
#endif
	}
	else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
		{
		getWlanMib(rootIdx, vwlanIdx,MIB_WLAN_ENCRYPT,(void *)&vUint);
			if( vUint >= ENCRYPT_WPA && vUint < ENCRYPT_WAPI )
			{
				unsigned int cipher=0,cipher2=0;
				getWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA_CIPHER_SUITE,(void *)&cipher);
				getWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA2_CIPHER_SUITE,(void *)&cipher2);
				if(vUint == ENCRYPT_WPA)
					vUint = cipher;
				else if(vUint == ENCRYPT_WPA2)
					vUint = cipher2;
				else if(vUint == ENCRYPT_WPA2_MIXED)
					vUint = (cipher | cipher2);
				else
 					vUint = 0;

				if( vUint==WPA_CIPHER_TKIP )
					*data = strdup( "TKIPEncryption" );
				else if( vUint==WPA_CIPHER_AES )
					*data = strdup( "AESEncryption" );
				else if( vUint==WPA_CIPHER_MIXED )
					*data = strdup( "TKIPandAESEncryptions" );
				else
					return ERR_9002;
			}
			else if (vUint == ENCRYPT_WEP)
			{
				*data = strdup( "WEPEncryption" );
			}
			else
			{
				*data = strdup( "None" );
			}
			
		}
	else if( strcmp( lastname, "WPAAuthenticationMode" )==0 )
		{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&vChar);
		if(vChar==WPA_AUTH_PSK)
				*data = strdup( "PSKAuthentication" );
			else
				*data = strdup( "EAPAuthentication" );
		}
	else if( strcmp( lastname, "IEEE11iEncryptionModes" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar);
		if( vChar==WPA_CIPHER_AES )
			*data = strdup( "AESEncryption" );
		else if( vChar==WPA_CIPHER_TKIP )
				*data = strdup( "TKIPEncryption" );
		else if( vChar==WPA_CIPHER_MIXED )
			*data = strdup( "TKIPandAESEncryption" );
		else if( vChar==0 ) //disabled in current state
			*data = strdup( "" );
			else
				return ERR_9002;
		}
	else if( strcmp( lastname, "IEEE11iAuthenticationMode" )==0 )
		{

		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&vChar);
		if(vChar==WPA_AUTH_PSK)
				*data = strdup( "PSKAuthentication" );
			else
				*data = strdup( "EAPAuthentication" );
		}
	else if( strcmp( lastname, "PossibleChannels" )==0 )
	{
		getWlanMib(rootIdx, 0, MIB_WLAN_BAND, (void *)&vChar);
		
		if( vChar & (BAND_11B|BAND_11G|BAND_11N))
			*data = strdup("1-14");
		else if( vChar & (BAND_5G_11AN))
			*data = strdup("34 -165");
		else
			*data = strdup( "" ); 
		}
	else if( strcmp( lastname, "BasicDataTransmitRates" )==0 )
		{
		getWlanMib(rootIdx, 0,MIB_WLAN_BASIC_RATES,(void *)&GetValue);
		uShort = (unsigned short)GetValue;
		getRateStr( uShort, buf );
		*data = strdup( buf );		
	}
	else if( strcmp( lastname, "OperationalDataTransmitRates" )==0 )
	{
		getWlanMib(rootIdx, 0,MIB_WLAN_SUPPORTED_RATES,(void *)&GetValue);
		uShort = (unsigned short)GetValue;
		getRateStr( uShort, buf );
		*data = strdup( buf );		
	}
	else if( strcmp( lastname, "PossibleDataTransmitRates" )==0 )
	{
		getWlanMib(rootIdx, 0,MIB_WLAN_SUPPORTED_RATES,(void *)&GetValue);
		uShort = (unsigned short)GetValue;
		getRateStr( uShort, buf );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "BeaconAdvertisementEnabled" )==0 )
	{		
		// always beacons.
		*data = booldup( 1 );

	}
	else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
		getWlanMib(rootIdx, 0,MIB_WLAN_HIDDEN_SSID,(void *)&GetValue);
		*data = booldup( (GetValue==0) );
	}
	else if( strcmp( lastname, "RadioEnabled" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);
		*data = booldup( (vChar==0) );
		}
	else if( strcmp( lastname, "AutoRateFallBackEnabled" )==0 )
		{
		getWlanMib(rootIdx, 0, MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		*data = booldup( (vChar==1) );
	}
	else if( strcmp( lastname, "LocationDescription" )==0 )
	{
		*data = strdup( gLocationDescription );
	}
	else if( strcmp( lastname, "ChannelsInUse" )==0 )
	{
		char WlanIf[32];

		sprintf(WlanIf, "wlan%d", rootIdx);
		if ( cwmp_getWlBssInfo(WlanIf, &bss) < 0)
			return -1;

		if (bss.channel)
			sprintf( buf, "%u", bss.channel );
		else
			sprintf( buf, "%s", "Auto");
				
		*data = strdup( buf );
		}
	else if( strcmp( lastname, "DeviceOperationMode" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx,MIB_WLAN_MODE,(void *)&vChar);
		if(vChar==AP_MODE)
			*data = strdup( "InfrastructureAccessPoint" );
		else if(vChar==CLIENT_MODE)
			*data = strdup( "WirelessStation" );
		else if(vChar==WDS_MODE)
			*data = strdup( "WirelessBridge" );
		else /*WirelessRepeater or others*/
			return ERR_9002;
	}
	else if( strcmp( lastname, "AuthenticationServiceMode" )==0 )
	{
		*data = strdup( "None" );
	}
	else if( strcmp( lastname, "TotalBytesSent" )==0 )
	{
		if( getInterfaceStat( wlan_ifname, &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( bs );
	}
	else if( strcmp( lastname, "TotalBytesReceived" )==0 )
	{
		if( getInterfaceStat( wlan_ifname, &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( br );
	}
	else if( strcmp( lastname, "TotalPacketsSent" )==0 )
	{
		if( getInterfaceStat( wlan_ifname, &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( ps );
	}
	else if( strcmp( lastname, "TotalPacketsReceived" )==0 )
	{
		if( getInterfaceStat( wlan_ifname, &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( pr );
	}		
	else if( strcmp( lastname, "TotalAssociations" )==0 )
	{
		if( loadWLANAssInfoByInstNum(wlaninst)< 0 )
			*data = uintdup( 0 );
		else
			*data = uintdup( gWLANTotalClients );
	}
#ifdef CUSTOMIZE_MIDDLE_EAST
	else if( strcmp( lastname, "X_DLINK_COM_Guest1MaxClients" )==0 )
	{
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_STA_NUM, (void *)&vChar);	
		if(vChar == 0)
			vChar = MAX_STA_NUM;//0 ~ max station num
		*data = uintdup(vChar);
	}
	else if( strcmp( lastname, "X_DLINK_COM_HT" )==0 )
	{	
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_CHANNEL_BONDING, (void *)&vChar);
		if( vChar==0 )
			*data = strdup( "20M" );
		else if( vChar==1 )
			*data = strdup( "40M" );
		else if( vChar==2 )
			*data = strdup( "80M" );
		else 
			*data = strdup( "" );
	}
#endif //CUSTOMIZE_MIDDLE_EAST
	else{
		return ERR_9005;
	}
	
	return 0;
}

int setWLANConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int	vChar=0;
	unsigned int wlaninst=0;
	int rootIdx=0,vwlanIdx=0;
	int NewRootIdx=0, NewvwlanIdx=0;
	int wpa_cipher=0, wpa2_cipher=0;
	unsigned int SetValue=0;
	unsigned int GetValue=0;
	unsigned short uShort;

	
	int isWLANMIBUpdated=1;
#if 0	
	unsigned char origRFBand;
	unsigned char NewRFBand;
	unsigned char origWlanDisabled;
	unsigned char NewWlanDisabled;
	unsigned char origChannelWidth;
	unsigned char NewChannelWidth;
	int isRFBandChanged=0;
	CWMP_WLANCONF_T target[2];
#endif	
	CWMP_WLANCONF_T *pwlanConf, wlanConf;
	int wlanBand2G5GSelect=0;
	int MaxWlanIface=NUM_WLAN_INTERFACE;

	
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


	wlaninst = getWLANConfInstNum( name );

#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>MaxWLANIface )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif


	pwlanConf = &wlanConf;
	getWLANIdxFromInstNum(wlaninst, &wlanConf, &rootIdx, &vwlanIdx);

#if 0
	memcpy(&target[0], &wlanConf, sizeof(CWMP_WLANCONF_T));

	pcwmpWlan = &cwmpWlan;
	memset(pcwmpWlan, 0x00, sizeof(CONFIG_WLAN_SETTING_T));

	clone_wlaninfo_get(pcwmpWlan, rootIdx,vwlanIdx);

	origRFBand=pcwmpWlan->phyBandSelect;
	origWlanDisabled = pcwmpWlan->wlanDisabled;
	origChannelWidth = pcwmpWlan->channelbonding;

	//memcpy(pcwmpWlan, &pMib->wlan[rootIdx][vwlanIdx], sizeof(CONFIG_WLAN_SETTING_T));
	//CWMPDBG( 1, ( stderr, "<%s:%d>pMib:%p\n", __FUNCTION__, __LINE__, pMib) );
	//CWMPDBG( 1, ( stderr, "<%s:%d>orig ssid:%s\n", __FUNCTION__, __LINE__, pcwmpWlan->ssid ) );
	//CWMPDBG( 1, ( stderr, "<%s:%d>orig channel:%d\n", __FUNCTION__, __LINE__, pcwmpWlan->channel ) );
#endif //#if 0


	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		//pcwmpWlan->func_off=vChar;
		//setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp(buf,"Auto")!=0 ) return ERR_9001;
		vChar = 1;
		//pcwmpWlan->rateAdaptiveEnabled = vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "Channel" )==0 )
	{
		unsigned int *i = data;

		if( i==NULL ) return ERR_9007;
		
		if(*i!=0) //0:auto
		{
			
			int valid = 0;

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			int wlanChannel = *i;

			if(vwlanIdx == 0) // root wlan can not modify RFband
			{
				if( (wlanChannel>=1 && wlanChannel<=14 && pwlanConf->RfBandAvailable == PHYBAND_5G)					
				||(wlanChannel>=34 && wlanChannel<=165 && pwlanConf->RfBandAvailable == PHYBAND_2G)
				)
				{
					return ERR_9007;

				}

			}

			
			if( (*i>=1) && (*i<=14))
			{
				if(pwlanConf->RfBandAvailable == PHYBAND_5G)
				{
					// find free wlan
					if(getNewWLANIdxFromReq(&NewRootIdx, &NewvwlanIdx,PHYBAND_2G)<0)
					{
						return ERR_9007; //no free wlan.
					}
					else
					{
						CWMP_WLANCONF_T target[2], newTarget[2];
						CONFIG_WLAN_SETTING_T origWlanCfg, newWlanCfg;

						clone_wlaninfo_get(&origWlanCfg, rootIdx, vwlanIdx);
						clone_wlaninfo_get(&newWlanCfg, NewRootIdx, NewvwlanIdx);
						
						memset( &target[0], 0, sizeof( CWMP_WLANCONF_T ) );
						memset( &target[1], 0, sizeof( CWMP_WLANCONF_T ) );

						memset( &newTarget[0], 0, sizeof( CWMP_WLANCONF_T ) );
						memset( &newTarget[1], 0, sizeof( CWMP_WLANCONF_T ) );

						// copy 5g data to 2g
						memcpy(&newWlanCfg, &origWlanCfg, sizeof(CONFIG_WLAN_SETTING_T));

						newWlanCfg.phyBandSelect = PHYBAND_2G; // restore original phyBandSelect

						/* reinit original 5g wlan setting */
						origWlanCfg.wlanDisabled = 1;
						//origWlanCfg.func_off = 0;

						/* update cwmp_wlan date */
						if(getCwmpWlanConById(rootIdx,vwlanIdx, &target[0])
							&& getCwmpWlanConById(NewRootIdx,NewvwlanIdx, &newTarget[0])
						)
						{
							int tmpInstanceNum;
							
							memcpy(&target[1], &target[0], sizeof( CWMP_WLANCONF_T ));
							memcpy(&newTarget[1], &newTarget[0], sizeof( CWMP_WLANCONF_T ));

							/* swap instance number */
							tmpInstanceNum = target[1].InstanceNum;
							target[1].InstanceNum = newTarget[1].InstanceNum;
							newTarget[1].InstanceNum = tmpInstanceNum;

							/* set  IsConfigured value*/
							target[1].IsConfigured = 0;
							newTarget[1].IsConfigured = 1;

							mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);
							mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&newTarget);

#if 0 // The channel of vrtual wlan is follow root wlan, here just to change rfband
							if(newWlanCfg.channelbonding == 1) // 40 MHz
							{
								
								if(wlanChannel <6){
									newWlanCfg.controlsideband=1; //lower
								}else{
									newWlanCfg.controlsideband=0; //upper
								}

							}
#endif
							clone_wlaninfo_set(&origWlanCfg, 0, 0, rootIdx, vwlanIdx, 0);
							clone_wlaninfo_set(&newWlanCfg, 0, 0, NewRootIdx, NewvwlanIdx, 0);


							rootIdx = NewRootIdx;
							vwlanIdx = NewvwlanIdx;
						}
						else
						{
							return ERR_9007; //no free wlan.
						}

					}
				}
				else
				{

				}
				valid = 1;
			}
			else if( (*i>=34) && (*i<=165) ) 
			{
				if(pwlanConf->RfBandAvailable == PHYBAND_2G)
				{
					// find free wlan
					if(getNewWLANIdxFromReq(&NewRootIdx, &NewvwlanIdx,PHYBAND_5G)<0)
					{
						return ERR_9007; //no free wlan.
					}
					else
					{
						CWMP_WLANCONF_T target[2], newTarget[2];
						CONFIG_WLAN_SETTING_T origWlanCfg, newWlanCfg;

						clone_wlaninfo_get(&origWlanCfg, rootIdx, vwlanIdx);
						clone_wlaninfo_get(&newWlanCfg, NewRootIdx, NewvwlanIdx);
						
						memset( &target[0], 0, sizeof( CWMP_WLANCONF_T ) );
						memset( &target[1], 0, sizeof( CWMP_WLANCONF_T ) );

						memset( &newTarget[0], 0, sizeof( CWMP_WLANCONF_T ) );
						memset( &newTarget[1], 0, sizeof( CWMP_WLANCONF_T ) );

						// copy 2g data to 5g
						memcpy(&newWlanCfg, &origWlanCfg, sizeof(CONFIG_WLAN_SETTING_T));

						newWlanCfg.phyBandSelect = PHYBAND_5G; // restore original phyBandSelect

						/* reinit original 2g wlan setting */
						origWlanCfg.wlanDisabled = 1;
						//origWlanCfg.func_off = 0;

						/* update cwmp_wlan date */
						if(getCwmpWlanConById(rootIdx,vwlanIdx, &target[0])
							&& getCwmpWlanConById(NewRootIdx,NewvwlanIdx, &newTarget[0])
						)
						{
							int tmpInstanceNum;
							
							memcpy(&target[1], &target[0], sizeof( CWMP_WLANCONF_T ));
							memcpy(&newTarget[1], &newTarget[0], sizeof( CWMP_WLANCONF_T ));

							/* swap instance number */
							tmpInstanceNum = target[1].InstanceNum;
							target[1].InstanceNum = newTarget[1].InstanceNum;
							newTarget[1].InstanceNum = tmpInstanceNum;

							/* set  IsConfigured value*/
							target[1].IsConfigured = 0;
							newTarget[1].IsConfigured = 1;

							mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);
							mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&newTarget);

#if 0 // vrtual wlan's channel is follow root wlan, here just to change rfband
							if(newWlanCfg.channelbonding == 1) // 40 MHz
							{
								if(wlanChannel > 0 && (wlanChannel == 36 
									|| wlanChannel==44 || wlanChannel==149
									|| wlanChannel == 157 ||wlanChannel==165)){
									newWlanCfg.controlsideband=1; //lower
								}else if(wlanChannel > 0 && (wlanChannel == 40 
									|| wlanChannel==48 || wlanChannel==153
									|| wlanChannel == 161)){
									newWlanCfg.controlsideband=0; //upper
								}

							}
#endif

							clone_wlaninfo_set(&origWlanCfg, 0, 0, rootIdx, vwlanIdx, 0);
							clone_wlaninfo_set(&newWlanCfg, 0, 0, NewRootIdx, NewvwlanIdx, 0);


							rootIdx = NewRootIdx;
							vwlanIdx = NewvwlanIdx;
						}
						else
						{
							return ERR_9007; //no free wlan.
						}

					}
				}
				else
				{

				}
				valid = 1;
			}
			else
				valid =0;
#else

			if( (*i>=1) && (*i<=14) ) 
				valid = 1;
			else
				valid =0;
#endif
			
			
			if( valid==0 ) return ERR_9007;			
		}
		vChar = *i;

#if 0
		memset(pcwmpWlan, 0x00, sizeof(CONFIG_WLAN_SETTING_T));

		clone_wlaninfo_get(pcwmpWlan, rootIdx,vwlanIdx);
	
		pcwmpWlan->channel = vChar;
#endif

		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_CHANNEL, (void *)&vChar);
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
	else if( strcmp( lastname, "BeaconType" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "None" )==0 )
		{
#if 1//def CTCOM_WLAN_REQ	//cathy
			vChar = ENCRYPT_DISABLED;
			//pcwmpWlan->encrypt=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
			//pcwmpWlan->authType=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_AUTH_TYPE, (void *)&vChar);

#else
			vChar = 1;
			pcwmpWlan->wlanDisabled=vChar;
			//setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WLAN_DISABLED, (void *)&vChar);

#endif
			isWLANMIBUpdated=1;
		}
		else if( strcmp( buf, "Basic" )==0 )
		{
#if 1//def CTCOM_WLAN_REQ	//cathy
			vChar = ENCRYPT_WEP;
			//pcwmpWlan->encrypt=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
			
			//0:disable, 1:64, 2:128
			//vChar=pcwmpWlan->wep;
			getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&vChar);
			if(vChar==0)
			{
				vChar=WEP64;
				//pcwmpWlan->wep=vChar;
				setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&vChar);
			}
			vChar = 2;	//both open and shared mode
			//pcwmpWlan->authType=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_AUTH_TYPE, (void *)&vChar);
#else

			vChar = ENCRYPT_WEP;
			pcwmpWlan->encrypt=vChar;
			//setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);


#endif
			isWLANMIBUpdated=1;
			}
		else if( strcmp( buf, "WPA" )==0 )
		{
			vChar = ENCRYPT_WPA;
			wpa_cipher = WPA_CIPHER_TKIP;
			//pcwmpWlan->encrypt=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
			//pcwmpWlan->wpaCipher=wpa_cipher;
			setWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA_CIPHER_SUITE,(void *)&wpa_cipher);
			vChar = 0;
			//pcwmpWlan->authType=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_AUTH_TYPE, (void *)&vChar);
			isWLANMIBUpdated=1;
		}
		else if( strcmp( buf, "11i" )==0 )
		{
								
			vChar = ENCRYPT_WPA2;
			wpa2_cipher = WPA_CIPHER_AES;
			//pcwmpWlan->encrypt=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
			//pcwmpWlan->wpa2Cipher = wpa2_cipher;
			setWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA2_CIPHER_SUITE,(void *)&wpa2_cipher);
			vChar = 0;
			//pcwmpWlan->authType=vChar;
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_AUTH_TYPE, (void *)&vChar);
			isWLANMIBUpdated=1;
		}
		else if( strcmp( buf, "WPAand11i" )==0 )
		{
				vChar = ENCRYPT_WPA2_MIXED;
				wpa_cipher = WPA_CIPHER_MIXED;
				wpa2_cipher = WPA_CIPHER_MIXED;
				//pcwmpWlan->encrypt=vChar;
				//pcwmpWlan->wpaCipher=wpa_cipher;
				//pcwmpWlan->wpa2Cipher = wpa2_cipher;
				setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
				setWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA_CIPHER_SUITE,(void *)&wpa_cipher);
				setWlanMib(rootIdx, vwlanIdx,MIB_WLAN_WPA2_CIPHER_SUITE,(void *)&wpa2_cipher);
			
				vChar = 0;
				//pcwmpWlan->authType=vChar;
				setWlanMib(rootIdx, vwlanIdx,MIB_WLAN_AUTH_TYPE, (void *)&vChar);
				isWLANMIBUpdated=1;
		}else
			return ERR_9007;
	}
#ifdef _CWMP_MAC_FILTER_
	else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?1:0;
		mib_set( MIB_CWMP_MACFILTER_WLAN_MAC_CTRL, (void *)&vChar);
		{
			unsigned char eth_mac_ctrl=0,mac_out_dft=1;
			mib_get(MIB_CWMP_MACFILTER_ETH_MAC_CTRL, (void *)&eth_mac_ctrl);
			if( vChar==1 || eth_mac_ctrl==1 )
				mac_out_dft=0;//0:deny, 1:allow
			mib_set(MIB_CWMP_MACFILTER_OUT_ACTION, (void *)&mac_out_dft);
		}
		isWLANMIBUpdated=1;
	}
#endif /*_CWMP_MAC_FILTER_*/
	else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
		unsigned int *i = data;

		if( i==NULL ) return ERR_9007;
		if( (*i<1) || (*i>4) ) return ERR_9007;
		vChar = (unsigned char)*i;
		vChar = vChar - 1; //mib's wepid is from 0 to 3
		//pcwmpWlan->wepDefaultKey=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		unsigned char pskfmt;
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)<8) || (strlen(buf)>63) ) return ERR_9007;
		pskfmt = 0; //0:Passphrase,   1:hex

		{
			//pcwmpWlan->wpaPSKFormat=pskfmt;
			//sprintf(pcwmpWlan->wpaPSK,"%s",buf);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_PSK_FORMAT, (void *)&pskfmt);
			setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_PSK, (void *)buf);
		}
		isWLANMIBUpdated=1;

	}
#if 1//def CTCOM_WLAN_REQ	//cathy
	else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "Disabled" )==0 )
			vChar = WEP_DISABLED;
		else if( strcmp( buf, "40-bit" )==0 )
			vChar = WEP64;
		else if( strcmp( buf, "104-bit" )==0 )
			vChar = WEP128;
		else return ERR_9007;
		//pcwmpWlan->wep=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
#endif
	else if( strcmp( lastname, "BasicEncryptionModes" )==0 )
	{
		unsigned char c_mode;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
	
		
		//c_mode = pcwmpWlan->encrypt;
		getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&c_mode);
		if( strcmp( buf, "WEPEncryption" )==0 )
		{
			if( c_mode==ENCRYPT_DISABLED )
			{
				vChar = ENCRYPT_WEP;
				{
					//pcwmpWlan->encrypt = vChar;
					setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
					{
						unsigned char c_key=0;
						//c_key = pcwmpWlan->wep;
						getWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&c_key);
						if( c_key==WEP_DISABLED )
						{
							c_key=WEP64;
							//pcwmpWlan->wep = c_key;
							setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WEP, (void *)&c_key);
						}
					}
				}

			}
		}
		else if( strcmp( buf, "None" )==0 )
		{
			if( c_mode==ENCRYPT_WEP )
			{
				vChar = ENCRYPT_DISABLED;
				//pcwmpWlan->encrypt = vChar;
				setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENCRYPT, (void *)&vChar);
			}
			
		}else
			return ERR_9007;

		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
#if 1//def CTCOM_WLAN_REQ	//cathy
		if( strcmp( buf, "OpenSystem")==0 )
			vChar=0;
		else if( strcmp( buf, "SharedKey")==0 )
			vChar=1;
		else if( strcmp( buf, "Both")==0 )
			vChar=2;
		else
#else
		if( strcmp( buf, "None")==0 )
			vChar=0;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=1;
		else		
#endif
			return ERR_9007;
	
#if 0//ndef CTCOM_WLAN_REQ	//cathy
		pcwmpWlan->enable1X = vChar;
		//setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_ENABLE_1X, (void *)&vChar);
		vChar=AUTH_BOTH;
#endif
		//pcwmpWlan->authType=vChar;	
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_AUTH_TYPE, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
	{
		unsigned char c_mode;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		if( strcmp( buf, "TKIPEncryption" )==0 )
			vChar=WPA_CIPHER_TKIP;
		else if( strcmp( buf, "AESEncryption" )==0 )
			vChar=WPA_CIPHER_AES;
		else if( strcmp( buf, "TKIPandAESEncryption" )==0 )
			vChar=WPA_CIPHER_MIXED;
		else
			return ERR_9001;
		//pcwmpWlan->wpaCipher=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar);
		isWLANMIBUpdated=1;
						}
	else if( strcmp( lastname, "WPAAuthenticationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "PSKAuthentication")==0 )
			vChar=WPA_AUTH_PSK;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=WPA_AUTH_AUTO;
		else
			return ERR_9001;
		//pcwmpWlan->wpaAuth=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "IEEE11iEncryptionModes" )==0 )
	{
		unsigned char c_mode;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		if( strcmp( buf, "AESEncryption" )==0 )
			vChar=WPA_CIPHER_AES;
		else if( strcmp( buf, "TKIPEncryption" )==0 )
			vChar=WPA_CIPHER_TKIP;
		else if( strcmp( buf, "TKIPandAESEncryption" )==0 )
			vChar=WPA_CIPHER_MIXED;
		else
			return ERR_9001;

		//pcwmpWlan->wpa2Cipher=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "IEEE11iAuthenticationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "PSKAuthentication")==0 )
			vChar=WPA_AUTH_PSK;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=WPA_AUTH_AUTO;
		else
			return ERR_9001;
		//pcwmpWlan->wpaAuth=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_WPA_AUTH, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "BasicDataTransmitRates" )==0 )
	{
		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( setRateStr( buf, &uShort )<0 ) return ERR_9007;
			
		//pcwmpWlan->basicRates=uShort;
		SetValue=uShort;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_BASIC_RATES, (void *)&SetValue);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "OperationalDataTransmitRates" )==0 )
	{

		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( setRateStr( buf, &uShort )<0 ) return ERR_9007;
			
		//pcwmpWlan->supportedRates=uShort;
		SetValue=uShort;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_SUPPORTED_RATES, (void *)&SetValue);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "BeaconAdvertisementEnabled" )==0 )
	{
		int *i = data;		
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;

		isWLANMIBUpdated=0;
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
	else if( strcmp( lastname, "RadioEnabled" )==0 )
	{
		int *i = data;
	
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		//pcwmpWlan->func_off=vChar;
		//setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_FUNC_OFF, (void *)&vChar);
		isWLANMIBUpdated=1;
			
	}
	else if( strcmp( lastname, "AutoRateFallBackEnabled" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?1:0;
		//pcwmpWlan->rateAdaptiveEnabled=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "LocationDescription" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 )
			strcpy( gLocationDescription, "" );
		else if( strlen(buf)<4096 )
			strcpy( gLocationDescription, buf );
		else 
			return ERR_9007;
		
		isWLANMIBUpdated=0;
	}
	else if( strcmp( lastname, "DeviceOperationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strcmp(buf, "InfrastructureAccessPoint" )==0 )
			vChar=AP_MODE;
		else if( strcmp(buf, "WirelessStation" )==0 )
#ifdef WLAN_CLIENT
			vChar=CLIENT_MODE;
#else
			return ERR_9001;
#endif
		else if( strcmp(buf, "WirelessBridge" )==0 )
			vChar=WDS_MODE;
		else
			return ERR_9007;
		//pcwmpWlan->wlanMode=vChar;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_MODE, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
		
	else if( strcmp( lastname, "AuthenticationServiceMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strcmp(buf, "None" )!=0 ) return ERR_9001;
		isWLANMIBUpdated=0;
	}
#ifdef CUSTOMIZE_MIDDLE_EAST
	else if( strcmp( lastname, "X_DLINK_COM_Guest1MaxClients" )==0 )
	{
		int *i = data;
		if( i==NULL ) 
			return ERR_9007;
		vChar = (unsigned char)*i;
		if(vChar >= MAX_STA_NUM)
			vChar = 0; //0 ~ max station num
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_STA_NUM, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
	else if( strcmp( lastname, "X_DLINK_COM_HT" )==0 )
	{	
		if( buf==NULL ) 
			return ERR_9007;
		if( strlen(buf)==0 ) 
			return ERR_9007;
		if( strcmp( buf, "20M" )==0 )
			vChar = 0;
		else if( strcmp( buf, "40M" )==0 )
			vChar = 1;
		else if( strcmp( buf, "80M" )==0 )
			vChar = 2;
		else 
			return ERR_9007;
		setWlanMib(rootIdx, vwlanIdx, MIB_WLAN_CHANNEL_BONDING, (void *)&vChar);
		isWLANMIBUpdated=1;
	}
#endif// CUSTOMIZE_MIDDLE_EAST
	else{
		return ERR_9005;
	}
	

#if 0
	NewChannelWidth = pcwmpWlan->channelbonding;
#if defined(CONFIG_RTL_92D_SUPPORT)
	if(origChannelWidth != NewChannelWidth){
		if(origChannelWidth ==0 && NewChannelWidth==1){ //Old is 20MHz --> New is 40Mhz
			if(pcwmpWlan->wlanBand & (BAND_5G_11AN)){ //5G
				if((pcwmpWlan->channel) > 0 && (pcwmpWlan->channel == 36 
					|| pcwmpWlan->channel==44 || pcwmpWlan->channel==149
					|| pcwmpWlan->channel == 157 ||pcwmpWlan->channel==165)){
					pcwmpWlan->controlsideband=1;
				}else if((pcwmpWlan->channel) > 0 && (pcwmpWlan->channel == 40 
					|| pcwmpWlan->channel==48 || pcwmpWlan->channel==153
					|| pcwmpWlan->channel == 161)){
					pcwmpWlan->controlsideband=0;
				}
			
			}else if(pcwmpWlan->wlanBand & (BAND_11BG|BAND_11N)){ //2.4G
				if(pcwmpWlan->channel<6){
					pcwmpWlan->controlsideband=1;
				}else{
					pcwmpWlan->controlsideband=0;
				}
			}
		}
	}
		
#endif


	
	NewRFBand=pcwmpWlan->phyBandSelect;
	NewWlanDisabled = pcwmpWlan->wlanDisabled;
	CWMPDBG( 2, ( stderr, "<%s:%d>origWlanDisabled:%d, NewWlanDisabled:%d, \n", __FUNCTION__, __LINE__, origWlanDisabled,NewWlanDisabled ) );
#endif


#if 0	
	if(origRFBand != NewRFBand)
	{ //RFBand Changed

		if(wlaninst > MaxWlanIface){ //Virtual wlan
			//check available mbssid from another root
			if(getNewWLANIdxFromReq(&NewRootIdx, &NewvwlanIdx,NewRFBand)<0){
				///do not have available mdssid for this instance, if rfband is changed
				return ERR_9007;
			}else{
				pwlanConf->RootIdx = NewRootIdx;
				pwlanConf->VWlanIdx = NewvwlanIdx;
				memcpy(&target[1], &wlanConf, sizeof(CWMP_WLANCONF_T));
				mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);
				isRFBandChanged=1;
				//CWMPDBG( 1, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
				clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, NewRootIdx,NewvwlanIdx,isRFBandChanged);
			}
		}else{ //Root wlan do not allow change RFBand
			return ERR_9007;
		}
	}
	else if(origWlanDisabled != NewWlanDisabled){
		if(wlaninst > MaxWlanIface){ //Virtual wlan
			//don't care enable/disable about 1x1 or 2x2 issue
			
			if(isWLANMIBUpdated){
				isRFBandChanged=0;
				//CWMPDBG( 1, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
				clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, NewRootIdx,NewvwlanIdx, isRFBandChanged);
			}
		}else{
			//Root wlan
			if(MaxWlanIface > 1){ //Dual band
				if(origWlanDisabled ==0 && NewWlanDisabled==1){ //WLAN Enabled ===>Disabled  
					if(rootIdx==1 && vwlanIdx==0){ //wlan1 enabled ==>disabled
					
						//CWMPDBG( 1, ( stderr, "<%s:%d>target:%s(action:%s)\n", __FUNCTION__, __LINE__, "<1,0>","wlan1 2.4G enabled ==>disabled" ) );
						SetValue = SMACSPHY;
						setWlanMib(0, 0, MIB_WLAN_MAC_PHY_MODE, (void *)&SetValue); //2.4G/5G Selective Mode
						SetValue = BANDMODESINGLE;
						mib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&SetValue);
						if(isWLANMIBUpdated){
							isRFBandChanged=0;
							//CWMPDBG( 1, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
							clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, rootIdx,vwlanIdx, isRFBandChanged);
						}
					
					}else if(rootIdx==0 && vwlanIdx==0){//wlan0 5G enabled ==>disabled
						
						
						CWMPDBG( 2, ( stderr, "<%s:%d>target:%s(action:%s)\n", __FUNCTION__, __LINE__, "<0,0>","wlan0 5G disabled ===> enabled" ) );
						SetValue = SMACSPHY;
						setWlanMib(1, 0, MIB_WLAN_MAC_PHY_MODE, (void *)&SetValue); //2.4G/5G Selective Mode
						SetValue = BANDMODESINGLE;
						mib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&SetValue);
						if(isWLANMIBUpdated){
							isRFBandChanged=0;
							//CWMPDBG( 1, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
							clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, rootIdx,vwlanIdx, isRFBandChanged);
						}
						swapWlanMibSetting(0,1);
						
						
						
						
					}
				}else if(origWlanDisabled ==1 && NewWlanDisabled==0){ //WLAN Disabled ===> Enabled
					if(rootIdx==1 && vwlanIdx==0){ //wlan1 2.4G disabled ===> enabled
						
						
						CWMPDBG( 2, ( stderr, "<%s:%d>target:%s(action:%s)\n", __FUNCTION__, __LINE__, "<1,0>","wlan1 disabled ===> enabled" ) );
						getWlanMib(0, 0, MIB_WLAN_WLAN_DISABLED, (void *)&GetValue);
						if(GetValue == 0){
							SetValue = DMACDPHY;
							setWlanMib(0, 0, MIB_WLAN_MAC_PHY_MODE, (void *)&SetValue);
							
							pcwmpWlan->macPhyMode = DMACDPHY; //2.4G/5G Concurrent Mode
							
							SetValue = BANDMODEBOTH;
							mib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&SetValue);
						}else if(GetValue == 1){
							pcwmpWlan->macPhyMode = SMACSPHY; //2.4G/5G Selective Mode
							
							SetValue = BANDMODESINGLE;
							mib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&SetValue);
						}
						if(isWLANMIBUpdated){
							isRFBandChanged=0;
							//CWMPDBG( 2, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
							clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, rootIdx,vwlanIdx, isRFBandChanged);
						}
						if(GetValue == 1){ //5G is disabled, swap 2.4G setting to 0,0 as 2x2 single
							swapWlanMibSetting(0,1);
						}
					
					}else if(rootIdx==0 && vwlanIdx==0){//wlan0 5G disabled ===> enabled
						
						CWMPDBG( 2, ( stderr, "<%s:%d>target:%s(action:%s)\n", __FUNCTION__, __LINE__, "<0,0>","wlan0 disabled ===> enabled" ) );
						getWlanMib(1, 0, MIB_WLAN_WLAN_DISABLED, (void *)&GetValue);
						if(GetValue == 0){ ///Originally WLAN 2.4G is Enabled
							SetValue = DMACDPHY;
							setWlanMib(1, 0, MIB_WLAN_MAC_PHY_MODE, (void *)&SetValue);
							pcwmpWlan->macPhyMode = DMACDPHY; ////2.4G/5G Concurrent Mode
							
							SetValue = BANDMODEBOTH;
							mib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&SetValue);
						}else if(GetValue==1){
							pcwmpWlan->macPhyMode = SMACSPHY; //2.4G/5G Selective Mode
							
							SetValue = BANDMODESINGLE;
							mib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&SetValue);
						}
						if(isWLANMIBUpdated){
							isRFBandChanged=0;
							//CWMPDBG( 1, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
							clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, rootIdx,vwlanIdx, isRFBandChanged);
						}
					
					}
				}
				
			}else{

				if(isWLANMIBUpdated){
					//copy the settng directly if single band
					//it do not care mib index in single band 
					isRFBandChanged=0;
					//CWMPDBG( 1, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
					clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, rootIdx,vwlanIdx, isRFBandChanged);
				}
			}
		}
	}
	else 		
	{
		
		if(isWLANMIBUpdated){
			//copy the settng directly if single band
			//it do not care mib index in single band 
			isRFBandChanged=0;
			//CWMPDBG( 1, ( stderr, "<%s:%d>call clone_wlaninfo_set \n", __FUNCTION__, __LINE__) );
			clone_wlaninfo_set(pcwmpWlan, rootIdx,vwlanIdx, rootIdx,vwlanIdx, isRFBandChanged);
		}
	}

	if(pwlanConf->IsConfigured==0){
		pwlanConf->IsConfigured=1;
		memcpy(&target[1], &wlanConf, sizeof(CWMP_WLANCONF_T));
		mib_set(MIB_CWMP_WLANCONF_MOD, (void *)&target);
	}
#endif //#if 0

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
	return 0;
#else
		if(isWLANMIBUpdated)
			return 1;
		else
			return 0;
#endif
	
}

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
/*InternetGatewayDevice.LANDevice.{i}.WLANConfiguration.{i}. 
SingleBand Mapping:
InstanceNum:
1,		2,			3,			4,			5,			6,
wlan iface:
wlan0	wlan0-va0	wlan0-va1	wlan0-va2	wlan0-va3	wlan0-vxd

mib index:<rootIdx, vwlan_idx>	
<0,0>	<0,1>		<0,2>		<0,3>		<0,4>		<0,5>

DualBand Mapping:
InstanceNum:
1,		2,			3,			4,			5,			6,			7,			8,			9,			10,			11,			12

wlan iface:
wlan0	wlan0-va0	wlan0-va1	wlan0-va2	wlan0-va3	wlan0-vxd	wlan1		wlan1-va0	wlan1-va1	wlan1-va2	wlan1-va3	wlan1-vxd

mib index:<rootIdx, vwlan_idx>	
<0,0>	<0,1>		<0,2>		<0,3>		<0,4>		<0,5>		<1,0>		<1,1>		<1,2>		<1,3>		<1,4>		<1,5>

*/
int getWLANIdxFromInstNum(int instnum, CWMP_WLANCONF_T *pwlanConf, int *rootIdx, int *vwlan_idx)
{
	
	int ret=-1;
	unsigned int i,num;
		
	mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);

	for( i=1; i<=num;i++ )
	{
		*((char *)pwlanConf) = (char)i;
		if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
			continue;
		if( (pwlanConf->InstanceNum==instnum))
		{
			*rootIdx = pwlanConf->RootIdx;
			*vwlan_idx = pwlanConf->VWlanIdx;
			ret = 0;
			break;
		}
	}	
	return ret; 

}		

#if 0
int getWLANIdxFromInstNum(int instnum, int *rootIdx, int *vwlan_idx)
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
#endif
int getInstNumFromWLANIdx(int rootIdx, int vwlan_idx,CWMP_WLANCONF_T *pwlanConf, int *instnum)
{

	int ret=-1;
	unsigned int i,num;
		
	mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);

	for( i=1; i<=num;i++ )
	{
		*((char *)pwlanConf) = (char)i;
		if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
			continue;
		if( (pwlanConf->RootIdx==rootIdx) && (pwlanConf->VWlanIdx==vwlan_idx))
		{
			*instnum = pwlanConf->InstanceNum;
			ret = 0;
			break;
		}
	}	
	return ret; 

	return 0;
}

int getNewWLANIdxFromReq(int *rootIdx, int *vwlan_idx, int ReqBand)
{
	
	int ret=-1;
	unsigned int i,num;
	int maxWLAN=NUM_WLAN_INTERFACE;
	CWMP_WLANCONF_T *pwlanConf, wlanConf;	

	pwlanConf = &wlanConf;
	mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);

	for( i=1; i<=num;i++ )
	{
		*((char *)pwlanConf) = (char)i;
		if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
			continue;	
		
		if( pwlanConf->IsConfigured==0 && pwlanConf->RfBandAvailable ==ReqBand)
		{
			*rootIdx = pwlanConf->RootIdx;
			*vwlan_idx = pwlanConf->VWlanIdx;
			ret = 0;
			break;
		}
	}	
	return ret; 

}		

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getWLANConfInstNum( char *name )
{
	return getInstNum( name, "WLANConfiguration" );
}

unsigned int getWEPInstNum( char *name )
{
	return getInstNum( name, "WEPKey" );
}

unsigned int getAssDevInstNum( char *name )
{
	return getInstNum( name, "AssociatedDevice" );
}

unsigned int getPreSharedKeyInstNum( char *name )
{
	return getInstNum( name, "PreSharedKey" );
}


char WLANASSFILE[] = "/tmp/stainfo";
int updateWLANAssociations( void )
{
	int i;
	time_t c_time=0;
	int w_idx, vw_idx;
	
	c_time = time(NULL);
	if( c_time >= gWLANAssUpdateTime+WLANUPDATETIME )
	{

		for(w_idx=0;w_idx<NUM_WLAN_INTERFACE;w_idx++)
		{

			for(vw_idx=0;vw_idx<NUM_VWLAN_INTERFACE;vw_idx++)
			{
				char filename[32];
				FILE *fp=NULL;
				int  has_info;
				char wlan_ifname[10] = {0};

				if(vw_idx == 0)
					sprintf(wlan_ifname, "wlan%d",w_idx);
				else
					sprintf(wlan_ifname, "wlan%d-va%d",w_idx,vw_idx-1);					

				has_info=1;
				memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );	
				if ( cwmp_getWlStaInfo( wlan_ifname,  (WLAN_STA_INFO_T *)gWLANAssociations ) < 0 )
				{
					
					CWMPDBG( 1, ( stderr, "<%s:%d>wlanIface=%s getSTAInfo error\n", __FUNCTION__, __LINE__,wlan_ifname ) );
					memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
					has_info=0;
				}

			
				sprintf( filename, "%s.%s", WLANASSFILE, wlan_ifname );
				fp=fopen( filename, "wb" );
				if(fp)
				{
					if(has_info)
					{
						fwrite( gWLANAssociations, 1, sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1), fp );
					}
					fclose(fp);
				}	
			}
		}

		
		gWLANAssUpdateTime = c_time;
	}
	return 0;
}


int loadWLANAssInfoByInstNum( unsigned int instnum )
{
	char filename[32];
	FILE *fp=NULL;
	int  found=0;
	CWMP_WLANCONF_T  wlanConf;
	int rootIdx, vwlanIdx;
	char wlan_ifname[10]={0};
	
	if( instnum==0 || instnum>WLAN_IF_NUM ) return -1;

	if( updateWLANAssociations()< 0 )
	{
		gWLANIDForAssInfo = -1;
		memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
		gWLANTotalClients=0;
		return -1;
	}

	getWLANIdxFromInstNum(instnum, &wlanConf, &rootIdx, &vwlanIdx);

	if(vwlanIdx == 0)
		sprintf(wlan_ifname,"wlan%d",rootIdx);
	else
		sprintf(wlan_ifname,"wlan%d-va%d",rootIdx,vwlanIdx);
	
	gWLANIDForAssInfo = -1;
	memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
	gWLANTotalClients=0;

	
	sprintf( filename, "%s.%s", WLANASSFILE, wlan_ifname );
	fp=fopen( filename, "rb" );
	if(fp)
	{
		int i;
		WLAN_STA_INFO_T *pInfo;

		
		fread( gWLANAssociations,  1, sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1), fp );	
		fclose(fp);
		
		for (i=1; i<=MAX_STA_NUM; i++)
		{
			pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
			if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
				found++;
		}


		
		gWLANTotalClients = found;		
	}
	
	return 0;
}


int getWLANSTAINFO(int id, WLAN_STA_INFO_T *info)
{
	WLAN_STA_INFO_T* pInfo;
	int found=-1, i;
	//id starts from 0,1,2...
	if( (id<0) || (id>=gWLANTotalClients) || (info==NULL) ) return -1;
	
	for (i=1; i<=MAX_STA_NUM; i++)
	{
		pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
		{
			found++;
			if(found==id) break;
		}
	}
	if( i>MAX_STA_NUM ) return -1;
	
	memcpy( info, pInfo, sizeof(WLAN_STA_INFO_T) );
	return 0;
}

#if 0
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
#endif
#if 0
int getPSKChainId( int index )
{
	CWMP_PSK_T	*pEntry=NULL,psk_entity;
	int total=0, i=0;

	total = mib_chain_total( CWMP_PSK_TBL );
	if( total<=0 ) return -1;

	for( i=0; i<total; i++)
	{
		pEntry = &psk_entity;
		if( mib_chain_get(CWMP_PSK_TBL, i, (void*)pEntry ) )
			if( pEntry->index==index ) break;
	}
	if( i==total ) return -1;

	return i;
}
#endif
int getRateStr( unsigned short rate, char *buf )
{
	int len;
	if( buf==NULL ) return -1;

	buf[0]=0;
	if( rate & TX_RATE_1M )
		strcat( buf, "1," );
	if( rate & TX_RATE_2M )
		strcat( buf, "2," );
	if( rate & TX_RATE_5M )
		strcat( buf, "5.5," );
	if( rate & TX_RATE_11M )
		strcat( buf, "11," );
	if( rate & TX_RATE_6M )
		strcat( buf, "6," );
	if( rate & TX_RATE_9M )
		strcat( buf, "9," );
	if( rate & TX_RATE_12M )
		strcat( buf, "12," );
	if( rate & TX_RATE_18M )
		strcat( buf, "18," );
	if( rate & TX_RATE_24M )
		strcat( buf, "24," );
	if( rate & TX_RATE_36M )
		strcat( buf, "36," );
	if( rate & TX_RATE_48M )
		strcat( buf, "48," );
	if( rate & TX_RATE_54M )
		strcat( buf, "54," );
		
	len = strlen(buf);
	if( len>1 )
		buf[len-1]=0;
	return 0;
}

int setRateStr( char *buf, unsigned short *rate )
{
	if( (rate!=NULL) && (buf!=NULL) )
	{
		char *tok;
		
		*rate=0;
		tok = strtok(buf,", \n\r");
		while(tok)
		{
			if( strcmp( tok, "1" )==0 )
				*rate = *rate | TX_RATE_1M;
			else if( strcmp( tok, "2" )==0 )
				*rate = *rate | TX_RATE_2M;
			else if( strcmp( tok, "5.5" )==0 )
				*rate = *rate | TX_RATE_5M;
			else if( strcmp( tok, "11" )==0 )
				*rate = *rate | TX_RATE_11M;
			else if( strcmp( tok, "6" )==0 )
				*rate = *rate | TX_RATE_6M;
			else if( strcmp( tok, "9" )==0 )
				*rate = *rate | TX_RATE_9M;
			else if( strcmp( tok, "12" )==0 )
				*rate = *rate | TX_RATE_12M;
			else if( strcmp( tok, "18" )==0 )
				*rate = *rate | TX_RATE_18M;
			else if( strcmp( tok, "24" )==0 )
				*rate = *rate | TX_RATE_24M;
			else if( strcmp( tok, "36" )==0 )
				*rate = *rate | TX_RATE_36M;
			else if( strcmp( tok, "48" )==0 )
				*rate = *rate | TX_RATE_48M;
			else if( strcmp( tok, "54" )==0 )
				*rate = *rate | TX_RATE_54M;
			else{
				*rate=0;
				return -1;
			}
							
			tok = strtok(NULL,", \n\r");
		}		
		return 0;
	}
	
	return -1;
}


int getIPbyMAC( char *mac, char *ip )
{
	int	ret=-1;
	FILE 	*fh;
	char 	buf[128];

	if( (mac==NULL) || (ip==NULL) )	return ret;
	ip[0]=0;
	
	fh = fopen("/proc/net/arp", "r");
	if (!fh) return ret;

	fgets(buf, sizeof buf, fh);	/* eat line */
	//fprintf( stderr, "%s\n", buf );
	while (fgets(buf, sizeof buf, fh))
	{
		char cip[32],cmac[32];
		
		//fprintf( stderr, "%s\n", buf );
		//format: IP address       HW type     Flags       HW address            Mask     Device
		if( sscanf(buf,"%s %*s %*s %s %*s %*s", cip,cmac)!=2 )
			continue;

		if( strcasecmp( mac, cmac )==0 )
		{
			strcpy( ip, cip );
			ret=0;
			break;
		}
	}
	fclose(fh);
	return ret;
}
#endif /*#ifdef WLAN_SUPPORT*/
