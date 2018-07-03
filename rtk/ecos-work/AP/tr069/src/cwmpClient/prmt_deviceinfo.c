#include "prmt_deviceinfo.h"
//#include <sys/sysinfo.h>
#include <time.h>
#if defined(HAVE_SNTP)
extern time_t cwmp_timep;
#endif
//#include "headers.h"

#include "prmt_utility.h" //keith add.
#define DEF_MANUFACTURER_STR		"REALTEK SEMICONDUCTOR CORP."
#define DEF_MANUFACTUREROUI_STR		"00E04C" 
#define DEF_PRODUCTCLASS_STR		"IGD"
//#define MIB_CWMP_PROVISIONINGCODE       711	//069


//#include "../../boa/src/LINUX/options.h"
//#include <rtk/options.h> keith remove
//#include <config/autoconf.h> keith remove

#ifdef CONFIG_MIDDLEWARE
#define MANUFACTURER_STR	"REALTEK"
#else 
#if defined(ZTE_531b_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define MANUFACTURER_STR	"ZTE."
#else
#define MANUFACTURER_STR	DEF_MANUFACTURER_STR //"REALTEK SEMICONDUCTOR CORP."
#endif
#endif
#define MANUFACTUREROUI_STR	DEF_MANUFACTUREROUI_STR //"00E04C"
#define SPECVERSION_STR		"1.0"
#define HWVERSION_STR		"81xx"

extern char *fwVersion;	// defined in version.c
int gCTStartPing=0;

/*extern int icmp_test(char *intf, char *host, unsigned int count, unsigned int timeout, unsigned int datasize, unsigned char tos,
	unsigned int *cntOK, unsigned int *cntFail, unsigned int *timeAvg, unsigned int *timeMin, unsigned int *timeMax, unsigned int needWaitRsp);*/

#ifdef _PRMT_X_CT_COM_ALARM_
extern ALARM_TIMER *alarm_timer;
#endif
#ifdef _PRMT_X_CT_COM_MONITOR_
extern ALARM_TIMER *monitor_timer;
#endif



/*ping_zhang:20100525 START:CT 1PW Extension*/
#ifdef _PRMT_X_CT_COM_SRVMNG_
struct CWMP_OP tCTServiceLeafOP = { getCTService, setCTService };
struct CWMP_PRMT tCTServiceLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"FtpEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpUserName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_PASSWORD,	&tCTServiceLeafOP},
{"FtpPort",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetUserName",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetPassword",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_PASSWORD,	&tCTServiceLeafOP},
{"TelnetPort",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP}
};
enum eCTServiceLeaf
{
	eCT_FtpEnable,
	eCT_FtpUserName,
	eCT_FtpPassword,
	eCT_FtpPort,
	eCT_TelnetEnable,
	eCT_TelnetUserName,
	eCT_TelnetPassword,
	eCT_TelnetPort
};
struct CWMP_LEAF tCTServiceLeaf[] =
{
{ &tCTServiceLeafInfo[eCT_FtpEnable] },
{ &tCTServiceLeafInfo[eCT_FtpUserName] },
{ &tCTServiceLeafInfo[eCT_FtpPassword] },
{ &tCTServiceLeafInfo[eCT_FtpPort] },
{ &tCTServiceLeafInfo[eCT_TelnetEnable] },
{ &tCTServiceLeafInfo[eCT_TelnetUserName] },
{ &tCTServiceLeafInfo[eCT_TelnetPassword] },
{ &tCTServiceLeafInfo[eCT_TelnetPort] },
{ NULL }
};
#endif //_PRMT_X_CT_COM_SRVMNG_

#ifdef _PRMT_X_CT_COM_ACCOUNT_
struct CWMP_OP tCTAccountLeafOP = { getCTAccount, setCTAccount };
struct CWMP_PRMT tCTAccountLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTAccountLeafOP},
{"Password",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_PASSWORD,	&tCTAccountLeafOP}
};
enum eCTAccountLeaf
{
	eCTTA_Enable,
	eCTTA_Password
};
struct CWMP_LEAF tCTAccountLeaf[] =
{
{ &tCTAccountLeafInfo[eCTTA_Enable] },
{ &tCTAccountLeafInfo[eCTTA_Password] },
{ NULL }
};
#endif /*_PRMT_X_CT_COM_ACCOUNT_*/

#if defined(_PRMT_X_CT_COM_ALG_) || defined(_PRMT_C_CU_ALG_)
struct CWMP_OP tXCTCOMALGLeafOP = { getXCTCOMALG, setXCTCOMALG };
struct CWMP_PRMT tXCTCOMALGLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"H323Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"SIPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"RTSPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"L2TPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"IPSECEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"FTPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
};
enum eXCTCOMALGLeaf
{
	eCT_H323Enable,
	eCT_SIPEnable,
	eCT_RTSPEnable,
	eCT_L2TPEnable,
	eCT_IPSECEnable,
	eCT_FTPEnable
};
struct CWMP_LEAF tXCTCOMALGLeaf[] =
{
{ &tXCTCOMALGLeafInfo[eCT_H323Enable] },
{ &tXCTCOMALGLeafInfo[eCT_SIPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_RTSPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_L2TPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_IPSECEnable] },
{ &tXCTCOMALGLeafInfo[eCT_FTPEnable] },
{ NULL }
};
#endif


#ifdef _PRMT_X_CT_COM_RECON_
struct CWMP_OP tCT_ReConLeafOP = { getCT_ReCon,	setCT_ReCon };
struct CWMP_PRMT tCT_ReConLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_ReConLeafOP},
};
enum eCT_ReConLeafInfo
{
	eCT_RCEnable
};
struct CWMP_LEAF tCT_ReConLeaf[] =
{
{ &tCT_ReConLeafInfo[eCT_RCEnable] },
{ NULL }
};
#endif //_PRMT_X_CT_COM_RECON_



#ifdef _PRMT_X_CT_COM_PORTALMNT_
struct CWMP_OP tCT_PortalMNTLeafOP = { getCT_PortalMNT, setCT_PortalMNT };
struct CWMP_PRMT tCT_PortalMNTLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-Computer",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-STB",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-Phone",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
};
enum eCT_PortalMNTLeaf
{
	eCT_PMEnable,
	eCT_PMComputer,
	eCT_PMSTB,
	eCT_PMPhone
};
struct CWMP_LEAF tCT_PortalMNTLeaf[] =
{
{ &tCT_PortalMNTLeafInfo[eCT_PMEnable] },
{ &tCT_PortalMNTLeafInfo[eCT_PMComputer] },
{ &tCT_PortalMNTLeafInfo[eCT_PMSTB] },
{ &tCT_PortalMNTLeafInfo[eCT_PMPhone] },
{ NULL } 
};
#endif //_PRMT_X_CT_COM_PORTALMNT_

#ifdef _PRMT_X_CT_COM_SYSLOG_
struct CWMP_OP tCT_SyslogLeafOP = { getCT_Syslog, setCT_Syslog };
struct CWMP_PRMT tCT_SyslogLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_SyslogLeafOP},
{"Level",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_SyslogLeafOP},
};
enum eCT_SyslogLeaf
{
	eCT_SyslogEnable,
	eCT_SyslogLevel,
};
struct CWMP_LEAF tCT_SyslogLeaf[] =
{
{ &tCT_SyslogLeafInfo[eCT_SyslogEnable] },
{ &tCT_SyslogLeafInfo[eCT_SyslogLevel] },
{ NULL } 
};
#endif //_PRMT_X_CT_COM_SYSLOG_

#ifdef _PRMT_X_CT_COM_UPNP_
struct CWMP_OP tCT_UpnpLeafOP = { getCT_Upnp, setCT_Upnp };
struct CWMP_PRMT tCT_UpnpLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_UpnpLeafOP},
};
enum eCT_UpnpLeaf
{
	eCT_UpnpEnable,
};
struct CWMP_LEAF tCT_UpnpLeaf[] =
{
{ &tCT_UpnpLeafInfo[eCT_UpnpEnable] },	
{ NULL }
};
#endif

#ifdef _PRMT_X_CT_COM_USB_RESTORE_
struct CWMP_OP tCT_USBRestoreLeafOP = { getCT_USB_Restore, setCT_USB_Restore };
struct CWMP_PRMT tCT_USBRestoreLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_USBRestoreLeafOP},
};
enum eCT_USBRestoreLeaf
{
	eCT_USBRestoreEnable,
};
struct CWMP_LEAF tCT_USBRestoreLeaf[] =
{
{ &tCT_USBRestoreLeafInfo[eCT_USBRestoreEnable] },	
{ NULL }
};
#endif

/*ping_zhang:20100525 END*/

/*ping_zhang:20100702 START:to support CT new standard*/
#ifdef _PRMT_X_CT_COM_IPVERSION_
struct CWMP_OP tCT_IPProtocolVersionLeafOP = { getCT_IPProtocolVersion, setCT_IPProtocolVersion };
struct CWMP_PRMT tCT_IPProtocolVersionLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Mode",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,		&tCT_IPProtocolVersionLeafOP},
};
enum eCT_IPProtocolVersionLeaf
{
	eCT_IPProtocolVersionMode,
};
struct CWMP_LEAF tCT_IPProtocolVersionLeaf[] =
{
{ &tCT_IPProtocolVersionLeafInfo[eCT_IPProtocolVersionMode] },	
{ NULL }
};
#endif

#ifdef _PRMT_X_CT_COM_PING_
struct CWMP_OP tCT_PingLeafOP = { getCT_Ping, setCT_Ping };
struct CWMP_PRMT tCT_PingLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_PingLeafOP},
{"PingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tCT_PingLeafOP},
};
enum eCT_PingLeaf
{
	eCT_PingEnable,
	eCT_PingPingNumberOfEntries
};
struct CWMP_LEAF tCT_PingLeaf[] =
{
{ &tCT_PingLeafInfo[eCT_PingEnable] },	
{ &tCT_PingLeafInfo[eCT_PingPingNumberOfEntries] },	
{ NULL }
};

struct CWMP_OP tCT_PingEntityLeafOP = { getCT_PingEntity, setCT_PingEntity };
struct CWMP_PRMT tCT_PingEntityLeafInfo[] =
{
/*(name,	type,		flag,		op)*/
{"DiagnosticsState",eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"Interface",		eCWMP_tSTRING,		CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"Host",			eCWMP_tSTRING,		CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"NumberOfRepetitions",	eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"DataBlockSize",	eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"DSCP",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"Interval",		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"Stop",			eCWMP_tBOOLEAN,		CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
};
enum eCT_PingEntityLeaf
{
	eCT_PingDiagnosticsState,
	eCT_PingInterface,
	eCT_PingHost,
	eCT_PingNumberOfRepetitions,
	eCT_PingDataBlockSize,
	eCT_PingDSCP,
	eCT_PingInterval,
	eCT_PingStop
};
struct CWMP_LEAF tCT_PingEntityLeaf[] =
{
{ &tCT_PingEntityLeafInfo[eCT_PingDiagnosticsState] },
{ &tCT_PingEntityLeafInfo[eCT_PingInterface] },
{ &tCT_PingEntityLeafInfo[eCT_PingHost] },
{ &tCT_PingEntityLeafInfo[eCT_PingNumberOfRepetitions] },
{ &tCT_PingEntityLeafInfo[eCT_PingDataBlockSize] },
{ &tCT_PingEntityLeafInfo[eCT_PingDSCP] },
{ &tCT_PingEntityLeafInfo[eCT_PingInterval] },
{ &tCT_PingEntityLeafInfo[eCT_PingStop] },
{ NULL }
};

struct CWMP_PRMT tCT_PingEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,		NULL}
};
enum eCT_PingEntityObject
{
	ePingEntity0
};
struct CWMP_LINKNODE tCT_PingEntityObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tCT_PingEntityObjectInfo[ePingEntity0], 	tCT_PingEntityLeaf,	NULL,		NULL,			0}
};

struct CWMP_OP tCT_Ping_OP = { NULL, objCT_PingEntity};
struct CWMP_PRMT tCT_PingObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PingConfig",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Ping_OP}
};
enum eCT_PingObject
{
	ePing
};
struct CWMP_NODE tCT_PingObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_PingObjectInfo[ePing], 		NULL,		NULL},
{NULL,					NULL,			NULL}
};
#endif



#ifdef _PRMT_X_CT_COM_ALARM_

struct CWMP_OP tCT_AlarmLeafOP = { getCT_Alarm, setCT_Alarm };
struct CWMP_PRMT tCT_AlarmLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmLeafOP},
{"AlarmNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tCT_AlarmLeafOP}
};
enum eCT_AlarmLeaf
{
	eCT_AlarmEnable,
	eCT_AlarmNumberOfEntries
};
struct CWMP_LEAF tCT_AlarmLeaf[] =
{
{ &tCT_AlarmLeafInfo[eCT_AlarmEnable] },
{ &tCT_AlarmLeafInfo[eCT_AlarmNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tCT_AlarmEntityLeafOP = { getCT_AlarmEntity, setCT_AlarmEntity };
struct CWMP_PRMT tCT_AlarmEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"ParaList",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"Limit-Max",		eCWMP_tSTRING,		CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"Limit-Min",		eCWMP_tSTRING,		CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"TimeList",		eCWMP_tSTRING,		CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"Mode",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,				&tCT_AlarmEntityLeafOP},
};
enum eCT_AlarmEntityLeaf
{
	eCT_AlarmParaList,
	eCT_AlarmLimit_Max,
	eCT_AlarmLimit_Min,
	eCT_AlarmTimeList,
	eCT_AlarmMode,
};
struct CWMP_LEAF tCT_AlarmEntityLeaf[] =
{
	{ &tCT_AlarmEntityLeafInfo[eCT_AlarmParaList] },
	{ &tCT_AlarmEntityLeafInfo[eCT_AlarmLimit_Max] },
	{ &tCT_AlarmEntityLeafInfo[eCT_AlarmLimit_Min] },
	{ &tCT_AlarmEntityLeafInfo[eCT_AlarmTimeList] },
	{ &tCT_AlarmEntityLeafInfo[eCT_AlarmMode] },
{ NULL }
};

struct CWMP_PRMT tCT_AlarmEntityObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_AlarmEntityObject
{
	eAlarmEntity0
};
struct CWMP_LINKNODE tCT_AlarmEntityObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_AlarmEntityObjectInfo[eAlarmEntity0], 	tCT_AlarmEntityLeaf,	NULL,		NULL,			0}
};

struct CWMP_OP tCT_Alarm_OP = { NULL, objCT_AlarmEntity};
struct CWMP_PRMT tCT_AlarmObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"AlarmConfig",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Alarm_OP}
};
enum eCT_AlarmObject
{
	eAlarm
};
struct CWMP_NODE tCT_AlarmObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_AlarmObjectInfo[eAlarm], 		NULL,			NULL},
{NULL,					NULL,			NULL}
};
#endif //#ifdef _PRMT_X_CT_COM_ALARM_


#ifdef _PRMT_X_CT_COM_MONITOR_

struct CWMP_OP tCT_MonitorLeafOP = { getCT_Monitor, setCT_Monitor };
struct CWMP_PRMT tCT_MonitorLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_MonitorLeafOP},
{"MonitorNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tCT_MonitorLeafOP}
};
enum eCT_MonitorLeaf
{
	eCT_MonitorEnable,
	eCT_MonitorNumberOfEntries	
};
struct CWMP_LEAF tCT_MonitorLeaf[] =
{
{ &tCT_MonitorLeafInfo[eCT_MonitorEnable] },	
{ &tCT_MonitorLeafInfo[eCT_MonitorNumberOfEntries] },	
{ NULL }
};

struct CWMP_OP tCT_MonitorEntityLeafOP = { getCT_MonitorEntity, setCT_MonitorEntity };
struct CWMP_PRMT tCT_MonitorEntityLeafInfo[] =
{
/*(name,	type,		flag,		op)*/
{"ParaList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_MonitorEntityLeafOP},
{"TimeList",	eCWMP_tSTRING,		CWMP_WRITE|CWMP_READ,	&tCT_MonitorEntityLeafOP},
};
enum eCT_MonitorEntityLeaf
{
	eCT_MonitorParaList,
	eCT_MonitorTimeList
};
struct CWMP_LEAF tCT_MonitorEntityLeaf[] =
{
{ &tCT_MonitorEntityLeafInfo[eCT_MonitorParaList] },
{ &tCT_MonitorEntityLeafInfo[eCT_MonitorTimeList] },
{ NULL }
};

struct CWMP_PRMT tCT_MonitorEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,		NULL}
};
enum eCT_MonitorEntityObject
{
	eMonitorEntity0
};
struct CWMP_LINKNODE tCT_MonitorEntityObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tCT_MonitorEntityObjectInfo[eMonitorEntity0], 	tCT_MonitorEntityLeaf,	NULL,		NULL,			0}
};

struct CWMP_OP tCT_Monitor_OP = { NULL, objCT_MonitorEntity};
struct CWMP_PRMT tCT_MonitorObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"MonitorConfig",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Monitor_OP}
};
enum eCT_MonitorObject
{
	eMonitor
};
struct CWMP_NODE tCT_MonitorObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_MonitorObjectInfo[eMonitor], 		NULL,		NULL},
{NULL,					NULL,			NULL}
};

#endif//#ifdef _PRMT_X_CT_COM_MONITOR_

struct CWMP_OP tVendorCfgEntityLeafOP = { getVendorCfgEntity, NULL };
struct CWMP_PRMT tVendorCfgEntityLeafInfo[] =
{
/*(name,	type,		flag,		op)*/
{"Name",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Version",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Date",	eCWMP_tDATETIME,CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Description",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
};
enum eVendorCfgEntityLeaf
{
	eVCName,
	eVCVersion,
	eVCDate,
	eDescription
};
struct CWMP_LEAF tVendorCfgEntityLeaf[] =
{
{ &tVendorCfgEntityLeafInfo[eVCName] },
{ &tVendorCfgEntityLeafInfo[eVCVersion] },
{ &tVendorCfgEntityLeafInfo[eVCDate] },
{ &tVendorCfgEntityLeafInfo[eDescription] },
{ NULL }
};


struct CWMP_PRMT tVendorConfigObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eVendorConfigObject
{
	eVC1
};
struct CWMP_NODE tVendorConfigObject[] =
{
/*info,  				leaf,			next)*/
{&tVendorConfigObjectInfo[eVC1],	tVendorCfgEntityLeaf, 	NULL},
{NULL, 					NULL, 			NULL}
};


struct CWMP_OP tDeviceInfoLeafOP = { getDeviceInfo, setDeviceInfo };
struct CWMP_PRMT tDeviceInfoLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Manufacturer",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ManufacturerOUI",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ModelName",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"Description",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ProductClass",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"SerialNumber",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"HardwareVersion",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"SoftwareVersion",		eCWMP_tSTRING,	CWMP_READ|CWMP_FORCE_ACT,		&tDeviceInfoLeafOP},
#ifdef CONFIG_DEV_xDSL
{"ModemFirmwareVersion",	eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
#endif
{"EnabledOptions",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
/*AdditionalHardwareVersion*/
/*AdditionalSoftwareVersion*/
{"SpecVersion",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ProvisioningCode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_FORCE_ACT,	&tDeviceInfoLeafOP},
{"UpTime",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDeviceInfoLeafOP},
{"FirstUseDate",		eCWMP_tDATETIME,CWMP_READ,		&tDeviceInfoLeafOP},
{"DeviceLog",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tDeviceInfoLeafOP},
{"VendorConfigFileNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tDeviceInfoLeafOP}
};
enum eDeviceInfoLeaf
{
	eDIManufacturer,
	eDIManufacturerOUI,
	eDIModelName,
	eDIDescription,
	eDIProductClass,
	eDISerialNumber,
	eDIHardwareVersion,
	eDISoftwareVersion,
#ifdef CONFIG_DEV_xDSL
	eDIModemFirmwareVersion,
#endif
	eDIEnabledOptions,
	eDISpecVersion,
	eDIProvisioningCode,
	eDIUpTime,
	eDIFirstUseDate,
	eDIDeviceLog,
	eDIVendorConfigFileNumberOfEntries
};
struct CWMP_LEAF tDeviceInfoLeaf[] =
{
{ &tDeviceInfoLeafInfo[eDIManufacturer] },
{ &tDeviceInfoLeafInfo[eDIManufacturerOUI] },
{ &tDeviceInfoLeafInfo[eDIModelName] },
{ &tDeviceInfoLeafInfo[eDIDescription] },
{ &tDeviceInfoLeafInfo[eDIProductClass] },
{ &tDeviceInfoLeafInfo[eDISerialNumber] },
{ &tDeviceInfoLeafInfo[eDIHardwareVersion] },
{ &tDeviceInfoLeafInfo[eDISoftwareVersion] },
#ifdef CONFIG_DEV_xDSL
{ &tDeviceInfoLeafInfo[eDIModemFirmwareVersion] },
#endif
{ &tDeviceInfoLeafInfo[eDIEnabledOptions] },
{ &tDeviceInfoLeafInfo[eDISpecVersion] },
{ &tDeviceInfoLeafInfo[eDIProvisioningCode] },
{ &tDeviceInfoLeafInfo[eDIUpTime] },
{ &tDeviceInfoLeafInfo[eDIFirstUseDate] },
{ &tDeviceInfoLeafInfo[eDIDeviceLog] },
{ &tDeviceInfoLeafInfo[eDIVendorConfigFileNumberOfEntries] },
{ NULL	}
};
struct CWMP_PRMT tDeviceInfoObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"VendorConfigFile",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef _PRMT_X_CT_COM_ACCOUNT_
	{CREATE_NAME(TeleComAccount),	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_ALG_
	{CREATE_NAME(ALGAbility),		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_RECON_
	{CREATE_NAME(ReConnect),		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
	{CREATE_NAME(PortalManagement), eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
	{CREATE_NAME(ServiceManage),	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
	{CREATE_NAME(Syslog),	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_ALARM_
	{CREATE_NAME(Alarm),  eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_MONITOR_
	{CREATE_NAME(Monitor),  eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
/*ping_zhang:20100128 END*/
#ifdef _PRMT_X_CT_COM_UPNP_
	{CREATE_NAME(UPNP),  eCWMP_tOBJECT,		CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_USB_RESTORE_
	{CREATE_NAME(Restore),  eCWMP_tOBJECT,		CWMP_READ,		NULL},
#endif
/*ping_zhang:20100525 END*/
/*ping_zhang:20100702 START:to support CT new standard*/
#ifdef _PRMT_X_CT_COM_IPVERSION_
	{CREATE_NAME(IPProtocolVersion),  eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_PING_
	{CREATE_NAME(Ping),  eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
/*ping_zhang:20100702 END*/
};
enum eDeviceInfoObject
{
	eDIVendorConfigFile,
		/*ping_zhang:20100525 START:CT 1PW Extension*/
#ifdef _PRMT_X_CT_COM_ACCOUNT_
	eDIX_CTCOM_TeleComAccount,
#endif
#ifdef _PRMT_X_CT_COM_ALG_
	eDIX_CTCOM_ALGAbility,
#endif
#ifdef _PRMT_X_CT_COM_RECON_
	eDIX_CTCOM_ReConnect,
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
	eDIX_CTCOM_PortalManagement,
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
	eDIX_CTCOM_ServiceManage,
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
	eDIX_CTCOM_Syslog,
#endif
#ifdef _PRMT_X_CT_COM_ALARM_
	eDIX_CTCOM_Alarm,
#endif
#ifdef _PRMT_X_CT_COM_MONITOR_
	eDIX_CTCOM_Monitor,
#endif
#ifdef _PRMT_X_CT_COM_UPNP_
			eDIX_CTCOM_Upnp,
#endif
#ifdef _PRMT_X_CT_COM_UPNP_
			eDIX_CTCOM_USBRestore,
#endif
		/*ping_zhang:20100525 END*/
		/*ping_zhang:20100702 START:to support CT new standard*/
#ifdef _PRMT_X_CT_COM_IPVERSION_
	eDIX_CTCOM_IPProtocolVersion,
#endif
#ifdef _PRMT_X_CT_COM_PING_
			eDIX_CTCOM_Ping,
#endif
		/*ping_zhang:20100702 END*/
};
struct CWMP_NODE tDeviceInfoObject[] =
{
/*info,  						leaf,		next)*/
{ &tDeviceInfoObjectInfo[eDIVendorConfigFile],		NULL,		tVendorConfigObject },
	/*ping_zhang:20100525 START:CT 1PW Extension*/
#ifdef _PRMT_X_CT_COM_ACCOUNT_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_TeleComAccount],	tCTAccountLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_ALG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ALGAbility],	tXCTCOMALGLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_RECON_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ReConnect],		tCT_ReConLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_PortalManagement],	tCT_PortalMNTLeaf,NULL },
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ServiceManage],	tCTServiceLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Syslog],		tCT_SyslogLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_ALARM_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Alarm],		tCT_AlarmLeaf,	tCT_AlarmObject },
#endif
#ifdef _PRMT_X_CT_COM_MONITOR_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Monitor],	tCT_MonitorLeaf,	tCT_MonitorObject },
#endif
#ifdef _PRMT_X_CT_COM_UPNP_
	{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Upnp],	tCT_UpnpLeaf,	NULL },
#endif
	/*ping_zhang:20100525 END*/
	/*ping_zhang:20100702 START:to support CT new standard*/
#ifdef _PRMT_X_CT_COM_IPVERSION_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_IPProtocolVersion],		tCT_IPProtocolVersionLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_PING_
	{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Ping],	tCT_PingLeaf,	tCT_PingObject },
#endif
#ifdef _PRMT_X_CT_COM_USB_RESTORE_
		{ &tDeviceInfoObjectInfo[eDIX_CTCOM_USBRestore],	tCT_USBRestoreLeaf,	NULL },
#endif

	/*ping_zhang:20100702 END*/
{ NULL,							NULL,		NULL }
};


#ifdef _PRMT_DEVICECONFIG_
struct CWMP_OP tDeviceConfigLeafOP = { getDeviceConfig, setDeviceConfig };
struct CWMP_PRMT tDeviceConfigLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PersistentData",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDeviceConfigLeafOP},
{"ConfigFile",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDeviceConfigLeafOP}
};
enum eDeviceConfigLeaf
{
	eDCPersistentData,
	eDCConfigFile
};
struct CWMP_LEAF tDeviceConfigLeaf[] =
{
{ &tDeviceConfigLeafInfo[eDCPersistentData] },
{ &tDeviceConfigLeafInfo[eDCConfigFile] },
{ NULL	}
};
#endif //_PRMT_DEVICECONFIG_

int getVendorCfgEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup( "config.dat" );
	}else if( strcmp( lastname, "Version" )==0 )
	{
		*data = strdup( fwVersion );
	}else if( strcmp( lastname, "Date" )==0 )
	{
		*data = timedup( 0 );//unknown time
	}else if( strcmp( lastname, "Description" )==0 )
	{
		*data = strdup( "" ); 
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int getDeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Manufacturer" )==0 )
	{
		*data = strdup( MANUFACTURER_STR );
	}else if( strcmp( lastname, "ManufacturerOUI" )==0 )
	{
		*data = strdup( MANUFACTUREROUI_STR );
	}else if( strcmp( lastname, "ModelName" )==0 )
	{
		//mib_get( MIB_SNMP_SYS_NAME, (void *)buf);
		*data = strdup( "ModelName" );
	}else if( strcmp( lastname, "Description" )==0 )
	{
		//mib_get( MIB_SNMP_SYS_DESCR, (void *)buf);
		*data = strdup( "Description" );
	}else if( strcmp( lastname, "ProductClass" )==0 )
	{
		*data = strdup( DEF_PRODUCTCLASS_STR ); //"IGD"
	}else if( strcmp( lastname, "SerialNumber" )==0 )
	{
		char tmpBuff[512];
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get( MIB_CWMP_SERIALNUMBER, (void *)tmpBuff);
#else
		mib_get(MIB_HW_NIC1_ADDR,  (void *)tmpBuff);

		sprintf(buf, "%02x%02x%02x%02x%02x%02x", (unsigned char)tmpBuff[0], (unsigned char)tmpBuff[1], 
			(unsigned char)tmpBuff[2], (unsigned char)tmpBuff[3], (unsigned char)tmpBuff[4], (unsigned char)tmpBuff[5]);
			
		
#endif
		*data = strdup( buf );
	}else if( strcmp( lastname, "HardwareVersion" )==0 )
	{
		*data = strdup( HWVERSION_STR );
	}else if( strcmp( lastname, "SoftwareVersion" )==0 )
	{
		*data = strdup( fwVersion );
	}
#ifdef CONFIG_DEV_xDSL
	else if( strcmp( lastname, "ModemFirmwareVersion" )==0 )
	{
		getAdslInfo( ADSL_GET_VERSION, buf, 256 );
		*data = strdup( buf );
	}
#endif
	else if( strcmp( lastname, "EnabledOptions" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "SpecVersion" )==0 )
	{
		*data = strdup( SPECVERSION_STR );
	}else if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		mib_get( MIB_CWMP_PROVISIONINGCODE, (void *)buf);
		*data = strdup( buf );	
	}else if( strcmp( lastname, "UpTime" )==0 )
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
	}else if( strcmp( lastname, "FirstUseDate" )==0 )
	{
/*
 * Date and time in UTC that the CPE first both successfully established an IP-layer network connection and acquired an absolute time reference using NTP or equivalent over that network connection. The CPE MAY reset this date after a factory reset.
 * If NTP or equivalent is not available, this parameter, if present, SHOULD be set to the Unknown Time value.
 */
#if defined(HAVE_SNTP)
		unsigned int vInt=0;
		mib_get( MIB_NTP_ENABLED, (void *)&vInt);
		//printf("<%s:%d>vInt=%d\n", __FUNCTION__, __LINE__, vInt);
		if(vInt==0) //NTP not enabled
			*data = timedup( 0 );
		else{
			//printf("<%s:%d>%d => %s\n", __FUNCTION__, __LINE__, cwmp_timep, ctime(&cwmp_timep));
			*data = timedup(cwmp_timep);
		}
#else
		*data =timedup(0);
#endif
	}else if( strcmp( lastname, "DeviceLog" )==0 )
	{
#if defined(CONFIG_USER_BUSYBOX_SYSLOGD) || defined(SYSLOG_SUPPORT)
		*type = eCWMP_tFILE; /*special case*/
#ifdef CONFIG_BOA_WEB_E8B_CH //NOT Support Yet 20120104
		*data = strdup( "/var/config/syslogd.txt" );
#else
		*data = strdup( "/var/log/messages" );
#endif
#else
		*data = strdup( "" );
#endif //#ifdef CONFIG_USER_BUSYBOX_SYSLOGD
	}else if( strcmp( lastname, "VendorConfigFileNumberOfEntries" )==0 )
	{
		*data = uintdup( 1 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setDeviceInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;	
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	
	if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		if( buf ) len = strlen( buf );
		if( len ==0 )
			mib_set( MIB_CWMP_PROVISIONINGCODE, (void *)"");
		else if( len < 64 )
			mib_set( MIB_CWMP_PROVISIONINGCODE, (void *)buf);
		else
			return ERR_9007;
			
		return 0;
	}else
		return ERR_9005; 
	return 0;
}

#ifdef _PRMT_DEVICECONFIG_
int getDeviceConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char persis_data[256];
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PersistentData" )==0 )
	{
		if(!mib_get(MIB_CWMP_PERSISTENT_DATA,  (void *)persis_data))
			return ERR_9005;

		*data = strdup(persis_data);
	}else if( strcmp( lastname, "ConfigFile" )==0 )
	{
#ifdef CONFIG_USE_XML
		if( va_cmd("/bin/saveconfig",0,1) )
		{
			fprintf( stderr, "<%s:%d>exec /bin/saveconfig error!\n", __FUNCTION__, __LINE__  );
			return ERR_9002;
		}
		// rename
		rename("/tmp/config.xml", CONFIG_FILE_NAME);
		*type = eCWMP_tFILE; /*special case*/
		*data = strdup(CONFIG_FILE_NAME);
#else
		*data = strdup("*** RAW Data ***");
#endif	
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setDeviceConfig(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PersistentData" )==0 )
	{
		if(strlen(data) >= 256)
			return ERR_9007;

		if(!mib_set(MIB_CWMP_PERSISTENT_DATA,  (void *)data))
		return ERR_9001;
	}else if( strcmp( lastname, "ConfigFile" )==0 )
	{
		char *buf=data;
		int  buflen=0;
		FILE *fp=NULL;
		
		if( buf==NULL ) return ERR_9007;
		buflen = strlen(buf);
		if( buflen==0 ) return ERR_9007;
		
#if !defined(CONFIG_BOA_WEB_E8B_CH	)	
/*star:20100312 START add to check the value*/
		if(strncmp(buf, "<Config_Information_File", 24))
		{
			return ERR_9007;
		}
/*star:20100312 END*/

		fp=fopen( "/tmp/config.xml", "w" );
		if(fp)
		{
			int retlen=0, id;

			fprintf( stderr, "New config length:%d\n", buflen );
#if 0
			retlen = fwrite( buf, 1, buflen, fp );
#else
			/*somehow, the '\n'is gone between lines, 
			  but loadconfig needs it to parse config.
			  the better way is to rewirte the parsing code of loadconfig*/
			for( id=0;id<buflen;id++ )
			{
				if( (id>0) && (buf[id-1]=='>') && (buf[id]=='<') )
					if( fputc( '\n',fp )==EOF )
						break;
				
				if(fputc( buf[id],fp )==EOF)
					break;
					
				retlen=id+1;
			}
			fputc( '\n',fp );
#endif
			fclose(fp);
			if( retlen==buflen )
			{
				if( va_cmd("/bin/loadconfig",0,1) )
				{
					fprintf( stderr, "<%s:%d>exec /bin/loadconfig error!\n", __FUNCTION__, __LINE__ );
					return ERR_9002; 
				}
			}else
				return ERR_9002;
		}else
			return ERR_9002;
#endif		
		return 1;
	}else{
		return ERR_9005;
	}

	return 0;

}
#endif //_PRMT_DEVICECONFIG_
/*ping_zhang:20100525 START:CT 1PW Extension*/
#ifdef _PRMT_X_CT_COM_ACCOUNT_
int getCTAccount(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char buff[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		int enabled;
		mib_get(MIB_CWMP_CT_ACCOUNT_ENABLED, (void*)&enabled);
		*data = booldup( enabled!=0 );
	}else if( strcmp( lastname, "Password" )==0 )
	{
		mib_get(MIB_SUPER_PASSWORD, (void*)buff);
		if(buff[0])
			*data=strdup(buff);
		else
			*data=strdup("");
		
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCTAccount(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	char pwdbuff[256]={0};
	int enabled;
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		enabled = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_ACCOUNT_ENABLED, (void*)&enabled);
	}else if( strcmp( lastname, "Password" )==0 )
	{
		
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=STR_LEN_64) return ERR_9001;
		
		strncpy(pwdbuff, buf, STR_LEN_64-1);
		pwdbuff[STR_LEN_64-1]=0;
		mib_set(MIB_SUPER_PASSWORD, (void *)pwdbuff);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif /*_PRMT_X_CT_COM_ACCOUNT_*/

#if defined(_PRMT_X_CT_COM_ALG_) || defined(_PRMT_C_CU_ALG_)
int getXCTCOMALG(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int state=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "H323Enable" )==0 )
	{
		mib_get(MIB_ALG_H323_ENABLED, &state);
		if(state==0)
			*data = booldup(0);
		else
			*data = booldup(1);
	}else if( strcmp( lastname, "SIPEnable" )==0 )
	{
		mib_get(MIB_ALG_SIP_ENABLED, &state);
		if(state==0)
			*data = booldup(0);
		else
			*data = booldup(1);
	}else if( strcmp( lastname, "RTSPEnable" )==0 )
	{
		mib_get(MIB_ALG_RTSP_ENABLED, &state);
		if(state==0)
			*data = booldup(0);
		else
			*data = booldup(1);
	}else if( strcmp( lastname, "L2TPEnable" )==0 )
	{
		mib_get(MIB_ALG_L2TP_ENABLED, &state);
		if(state==0)
			*data = booldup(0);
		else
			*data = booldup(1);
	}else if( strcmp( lastname, "IPSECEnable" )==0 )
	{
		mib_get(MIB_ALG_IPSEC_ENABLED, &state);
		if(state==0)
			*data = booldup(0);
		else
			*data = booldup(1);
	}else if( strcmp( lastname, "FTPEnable" )==0 )
	{
		mib_get(MIB_ALG_FTP_ENABLED, &state);
		if(state==0)
			*data = booldup(0);
		else
			*data = booldup(1);	
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setXCTCOMALG(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int enabled;
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "H323Enable" )==0 )
	{
		char algH323EnableStr[]=" %s ip nat alg h323";
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		enabled = (*i==0)?0:1;
		mib_set(MIB_ALG_H323_ENABLED, &enabled);
	}
	else if( strcmp( lastname, "SIPEnable" )==0 )
	{
		char algSipEnableStr[]=" %s ip nat alg sip";
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		enabled = (*i==0)?0:1;	
		mib_set(MIB_ALG_SIP_ENABLED, &enabled);
	}
	else if( strcmp( lastname, "RTSPEnable" )==0 )
	{
		char algRtspEnableStr[]=" %s ip nat alg rtsp";
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		enabled = (*i==0)?0:1;
		mib_set(MIB_ALG_RTSP_ENABLED, &enabled);
	}
	else	if( strcmp( lastname, "L2TPEnable" )==0 )
	{
		char algL2tpEnableStr[]=" %s ip nat pass-thru l2tp";
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		enabled = (*i==0)?0:1;
		mib_set(MIB_ALG_L2TP_ENABLED, &enabled);
	}
	else if( strcmp( lastname, "IPSECEnable" )==0 )
	{
		char algIpsecEnableStr[]=" %s ip nat pass-thru ipsec";
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		enabled = (*i==0)?0:1;
		mib_set(MIB_ALG_IPSEC_ENABLED, &enabled);
	}else if( strcmp( lastname, "FTPEnable" )==0 )
	{
		char algFtpEnableStr[]=" %s ip nat alg ftp";
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		enabled = (*i==0)?0:1;
		mib_set(MIB_ALG_FTP_ENABLED, &enabled);
	}
	else 
		return ERR_9005;
	
	return 0;
}
#endif //#ifdef _PRMT_X_CT_COM_ALG_

#ifdef _PRMT_X_CT_COM_RECON_
int getCT_ReCon(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int vInt=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_WAN_RE_CONNECT,(void *)&vInt);
		*data = booldup( vInt!=0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_ReCon(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int vInt=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vInt = (*i==0)?0:1;
		mib_set(MIB_WAN_RE_CONNECT,&vInt);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_RECON_

#ifdef _PRMT_X_CT_COM_PORTALMNT_
int getCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int vInt=0;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_PORTAL_MNT_ENABLED,(void *)&vInt);
		if(vInt)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}else if( strcmp( lastname, "PortalUrl-Computer" )==0 )
	{
		mib_get(MIB_PORTAL_MNT_URL_PC,(void *)&buf);
		if(buf[0])
			*data = strdup( buf);	
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "PortalUrl-STB" )==0 )
	{
		mib_get(MIB_PORTAL_MNT_URL_STB,(void *)&buf);
		if(buf[0])
			*data = strdup( buf);	
		else
			*data = strdup( "" );

	}else if( strcmp( lastname, "PortalUrl-Phone" )==0 )
	{
		mib_get(MIB_PORTAL_MNT_URL_MOBILE_PHONE,(void *)&buf);
		if(buf[0])
			*data = strdup( buf);	
		else
			*data = strdup( "" );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

//static char strset_ctportal_enableflag[]="urlredirect ctportal enableflag %d";;
//static char strset_ctportal_url_pc[]="urlredirect ctportal pcredirecturl %s";
//static char strset_ctportal_url_stb[]="urlredirect ctportal stbredirecturl %s";
//static char strset_ctportal_url_phone[]="urlredirect ctportal phoneredirecturl %s";
int setCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int vInt=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vInt = (*i==0)?0:1;
		mib_set(MIB_PORTAL_MNT_ENABLED,(void *)&vInt);
		
	}else if( strcmp( lastname, "PortalUrl-Computer" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		mib_set(MIB_PORTAL_MNT_URL_PC, (void *)buf);
	}else if( strcmp( lastname, "PortalUrl-STB" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		mib_set(MIB_PORTAL_MNT_URL_STB, (void *)buf);
	}else if( strcmp( lastname, "PortalUrl-Phone" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		mib_set(MIB_PORTAL_MNT_URL_MOBILE_PHONE, (void *)buf);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_PORTALMNT_


#ifdef _PRMT_X_CT_COM_SRVMNG_
int getCTService(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int vInt;
	char buff[256]={0};
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "FtpEnable" )==0 )
	{
		mib_get(MIB_FTP_ENABLED, (void *)&vInt);
		if(vInt)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}else if( strcmp( lastname, "FtpUserName" )==0 )
	{
		mib_get(MIB_FTP_USERNAME, (void*)buff);
		if(buff[0])
			*data=strdup(buff);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "FtpPassword" )==0 )
	{
		mib_get(MIB_FTP_PASSWD, (void*)buff);
		if(buff[0])
			*data=strdup(buff);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "FtpPort" )==0 )
	{
		mib_get(MIB_FTP_PORT, (void *)&vInt);
		*data=intdup(vInt);
	}else if( strcmp( lastname, "TelnetEnable" )==0 )
	{
		mib_get(MIB_TELNET_ENABLED, (void *)&vInt);
		if(vInt)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}else if( strcmp( lastname, "TelnetUserName" )==0 )
	{
		mib_get(MIB_TELNET_USERNAME, (void*)buff);
		if(buff[0])
			*data=strdup(buff);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "TelnetPassword" )==0 )
	{
		mib_get(MIB_TELNET_PASSWD, (void*)buff);
		if(buff[0])
			*data=strdup(buff);
		else
			*data=strdup("");
	}else if( strcmp( lastname, "TelnetPort" )==0 )
	{
		mib_get(MIB_TELNET_PORT, (void *)&vInt);
		*data=intdup(vInt);	
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCTService(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	char buff[256]={0};

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "FtpEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		int state;
		if( i==NULL ) return ERR_9007;
		state = (*i==0) ? 0:1;
		mib_set(MIB_FTP_ENABLED, (void *)&state);
	}else if( strcmp( lastname, "FtpUserName" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=STR_LEN_64) return ERR_9001;
		
		strncpy(buff, buf, STR_LEN_64-1);
		buff[STR_LEN_64-1]=0;
		mib_set(MIB_FTP_USERNAME, (void *)buff);
		
	}else if( strcmp( lastname, "FtpPassword" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=STR_LEN_64) return ERR_9001;
		
		strncpy(buff, buf, STR_LEN_64-1);
		buff[STR_LEN_64-1]=0;
		mib_set(MIB_FTP_PASSWD, (void *)buff);
		
	}else if( strcmp( lastname, "FtpPort" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif

		if( i==NULL ) return ERR_9007;
		if(*i<0 || *i>65535 ) return ERR_9007;
		
		mib_set(MIB_FTP_PORT, (void *)i);
		
	}else if( strcmp( lastname, "TelnetEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		int state;
		if( i==NULL ) return ERR_9007;
		state = (*i==0) ? 0:1;
		mib_set(MIB_TELNET_ENABLED, (void *)&state);
	}else if( strcmp( lastname, "TelnetUserName" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=STR_LEN_64) return ERR_9001;
		
		strncpy(buff, buf, STR_LEN_64-1);
		buff[STR_LEN_64-1]=0;
		mib_set(MIB_TELNET_USERNAME, (void *)buff);
	}else if( strcmp( lastname, "TelnetPassword" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=STR_LEN_64) return ERR_9001;
		
		strncpy(buff, buf, STR_LEN_64-1);
		buff[STR_LEN_64-1]=0;
		mib_set(MIB_TELNET_PASSWD, (void *)buff);
		
	}else if( strcmp( lastname, "TelnetPort" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if(*i<0 || *i>65535 ) return ERR_9007;
		
		mib_set(MIB_FTP_PORT, (void *)i);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif /*_PRMT_X_CT_COM_SRVMNG_*/

#ifdef _PRMT_X_CT_COM_SYSLOG_
typedef enum LOG_LEVEL{
	LogLevel_Emergency,
	LogLevel_Alert,
	LogLevel_Critical,
	LogLevel_Error,
	LogLevel_Warning,
	LogLevel_Notice,
	LogLevel_Informational,
	LogLevel_Debug,
};

int getCT_Syslog(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int vInt=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_SYSLOG_ENABLED, (void *)&vInt);
		*data = booldup( vInt!=0 );
	}else if( strcmp( lastname, "Level" )==0 )
	{
		mib_get(MIB_SYSLOG_LEVEL, (void *)&vInt);
		*data=intdup(vInt);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_Syslog(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int vInt=0;
	char cmdbuf[128];
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vInt = (*i==0)?0:1;
		mib_set(MIB_SYSLOG_ENABLED, (void *)&vInt);
		
	}else if( strcmp( lastname, "Level" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if( *i>7) return ERR_9007;

		mib_set(MIB_SYSLOG_LEVEL, (void *)i);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_SYSLOG_




/*ping_zhang:20100128 START:add for e8b tr069 Alarm*/
#if defined(_PRMT_X_CT_COM_ALARM_) || defined(_PRMT_X_CT_COM_MONITOR_)

void clearAlarmMonitorTimer(unsigned int flag)
{
	ALARM_TIMER *timer, *timerNext;
	if(flag==0)
		timer=alarm_timer;
	else
		timer=monitor_timer;

	while(timer)
{
		timerNext = timer->next;
		free(timer);
		timer = timerNext;
	}

	if(flag==0)
	alarm_timer = NULL;
	else
		monitor_timer=NULL;	
}

void insertAlarmMonitorTimer(unsigned int instnum,unsigned int period, unsigned int flag)
{
	ALARM_TIMER *timer;
	if(flag==0) {
		timer = alarm_timer;
	}else {
		timer = monitor_timer;
	}
	while(timer)
	{
		if(timer->instnum==instnum)
		{
			return;
		}
		timer=timer->next;
	}

	timer = (ALARM_TIMER *)malloc(sizeof(ALARM_TIMER));
	if(!timer)
		return;
	memset(timer,0,sizeof(ALARM_TIMER));
	timer->period = period;
	timer->state = STATE_OFF;
	timer->instnum = instnum;
	if(flag==0) {
	timer->next = alarm_timer;
	alarm_timer = timer;
	}else {
		timer->next = monitor_timer;
		monitor_timer = timer;
	}
}
void deleteAlarmMonitorTimerByInstnum(unsigned int instnum, unsigned int flag)
{
	ALARM_TIMER *timer,*timerPrev,*timerList;
	if(flag==0)
		timerList =alarm_timer;
	else
		timerList = monitor_timer;


	for(timer=timerPrev=timerList; timer; timer=timer->next)
	{
		if(timer->instnum== instnum)
		{	
			if(timer==timerPrev) //the first node
				timerList = timer->next;
			else
			        timerPrev->next = timer->next;
			free(timer);				
			timer=NULL;
			return;
		}
		timerPrev = timer;
		}
	}

#endif
#ifdef _PRMT_X_CT_COM_ALARM_

int getAlarmInstNum(char *name, unsigned int *pInstNum)
{
	if ((name == NULL) || (pInstNum == NULL))
		return -1;

	*pInstNum = getInstNum(name, "AlarmConfig");

	return 0;
}

int getAlarmByInstnum(unsigned int instnum, CWMP_CT_ALARM_T *p, unsigned int *id)
{
	int ret = -1, i, num;

	if ((instnum == 0) || (p == NULL) || (id == NULL))
		return ret;

	mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&num);
	for (i = 1; i <= num; i++) {

		*((char *)p) = (char)i;
		if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)p))
			continue;
		if (p->InstanceNum == instnum) {
			*id = i;
			ret = 0;
			break;
	}
	}

	return ret;
}

int getAlarmPeriodByInstnum(unsigned int instnum, unsigned int *pPeriod)
{
	int ret = -1, i, num, val;
	unsigned int vChar = 0;

	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;
	if ((instnum == 0) || (pPeriod == NULL))
		return ret;

	mib_get(MIB_CWMP_CT_ALARM_ENABLED, (void *)&vChar);
	if (vChar) {
		mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&num);
		for (i = 1; i <= num; i++) {
			*((char *)pAlarm) = (char)i;
			if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)pAlarm))
				continue;

			if (pAlarm->InstanceNum == instnum) {
				*pPeriod = pAlarm->alarmPeriod * 60;
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int findMaxAlarmInstNum(unsigned int *pInstNum)
{
	int ret = -1, i, num;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	if (pInstNum == NULL)
		return ret;

	*pInstNum = 0;

	mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&num);
	for (i = 1; i <= num; i++) {
		*((char *)pAlarm) = (char)i;
		if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)pAlarm))
			continue;

		if (pAlarm->InstanceNum > *pInstNum) {
			*pInstNum = pAlarm->InstanceNum;
			ret = 0;
		}
	}

	return ret;
}

static const char *alarmNumPrefix[] = {
	"100", "101", "102", "103", "104",	 	//device alarm
	"201", "202",					//service quality alarm
	"301", "302", "303",				//process error alarm
	"401",						//comnunication alarm
	"500" };					//environment alarm

int isValidAlarmNumber(unsigned int alarmNumber)
{
	int ret = 0;
	unsigned int i;
	char alarmNumStr[8];

	if (alarmNumber > 999999) {
		return ret;
	}

	sprintf(alarmNumStr, "%u", alarmNumber);
	for (i = 0; i < ARRAY_SIZE(alarmNumPrefix); i++) {
		if (!strcmp(alarmNumPrefix[i], alarmNumStr)) {
			ret = 1;
			break;
		}
	}

	return ret;
}

int isOverThresholdAlarm(unsigned int alarmNumber)
{
	char alarmNumStr[8];

	sprintf(alarmNumStr, "%u", alarmNumber);
	return (!strncmp(alarmNumPrefix[5], alarmNumStr, 3));
}

int alarm_check_threshold(unsigned int instnum)
{
	int type, ret = 0;
	unsigned int i, num;
	void *value;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&num);
	for (i = 1; i <= num; i++) {

		*((char *)pAlarm) = (char)i;
		if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)pAlarm))
			continue;
		

		if (pAlarm->alarmMode == ALARM_MODE_NONE) {
			continue;
		}

		if (!isOverThresholdAlarm(pAlarm->alarmNumber)) {
			continue;
		}

		if (pAlarm->InstanceNum == instnum) {
			if (get_ParameterValue(pAlarm->alarmName, &type, &value) == 0) {
				int intValue = *(int *)value;
				
				
				if ((intValue < pAlarm->thresholdLower)|| (intValue > pAlarm->thresholdUpper)) {
					ret = 1;
				}
			}

			break;
		}
	}
	return ret;
}

int init_alarm_timer()
{
	unsigned int vChar = 0;
	int period, i, num;
	ALARM_TIMER *timer;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	mib_get(MIB_CWMP_CT_ALARM_ENABLED, &vChar);
	if (vChar == 0) {
		clearAlarmMonitorTimer(0);
		return -1;
	}

	mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&num);
	for (i = 1; i <= num; i++) {
		*((char *)pAlarm) = (char)i;
		if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)pAlarm)){
			continue;
		}

		/* Tsai: why is these checkings necessary */
		if (pAlarm->alarmName[0]
				&& pAlarm->alarmMode
				&& pAlarm->thresholdUpper
				&& pAlarm->thresholdLower
				&& pAlarm->alarmPeriod) {
			period = pAlarm->alarmPeriod * 60;
		} else {
			period = 0;
		}

		insertAlarmMonitorTimer(pAlarm->InstanceNum, period,0);
	}

	return 0;
}

int updateAlarmTimer(CWMP_CT_ALARM_T *pAlarm)
{
	int ret = 0;
	ALARM_TIMER *timer;

	if (!pAlarm || pAlarm->alarmName[0] == 0
			|| pAlarm->alarmMode == ALARM_MODE_NONE
			|| pAlarm->thresholdUpper == 0
			|| pAlarm->thresholdLower == 0
			|| pAlarm->alarmPeriod == 0) {
		return ret;
	}

	for (timer = alarm_timer; timer; timer = timer->next) {
		if (timer->instnum == pAlarm->InstanceNum) {
			timer->period = pAlarm->alarmPeriod * 60;
			ret = 1;
			break;
		}
	}

	return ret;
}

int getCT_Alarm(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int vChar = 0;
	unsigned int num=0;
	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0) {
		
		mib_get(MIB_CWMP_CT_ALARM_ENABLED, &vChar);
		*data = booldup(vChar != 0);
	} else if (strcmp(lastname, "AlarmNumberOfEntries") == 0) {
		mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&num);
		*data = uintdup(num);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setCT_Alarm(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint = 0;
	unsigned int tmpuint = 0;
	int tmpbool = 0;
	if (changestring2int(data, entity->info->type, type, &tmpint, &tmpuint, &tmpbool) < 0)
		return ERR_9006;
#else
	if (entity->info->type != type)
		return ERR_9006;
#endif

	if (strcmp(lastname, "Enable") == 0) {
		unsigned int vChar=0;

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif	
		if (i == NULL) {
			return ERR_9007;
		}
		vChar = (*i != 0);
		mib_set(MIB_CWMP_CT_ALARM_ENABLED, &vChar);
		init_alarm_timer();
	} else {
		return ERR_9005;
	}
	
	return 0;
}

int getCT_AlarmEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int instNum, chainid;
	char buf[8];
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	getAlarmInstNum(name, &instNum);
	if (instNum == 0)
		return ERR_9005;

	if (getAlarmByInstnum(instNum, pAlarm, &chainid) <0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ParaList" )==0 )
	{
		*data = strdup(pAlarm->alarmName);
	}else if( strcmp( lastname, "Limit-Max" )==0 )
	{
		char max[8];
		sprintf(max,"%d", pAlarm->thresholdUpper);
		*data =strdup(max);
	}else if( strcmp( lastname, "Limit-Min" )==0 )
	{
		char min[8];
		sprintf(min,"%d", pAlarm->thresholdLower);
		*data =strdup(min);
	}else if( strcmp( lastname, "TimeList" )==0 )
	{
		char time[8];
		sprintf(time,"%d", pAlarm->alarmPeriod);
		*data =strdup(time);
	}else if( strcmp( lastname, "Mode" )==0 )
	{
		*data = uintdup(pAlarm->alarmMode);
	} else {
		return ERR_9005;
}

	return 0;
}

int setCT_AlarmEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int instNum, chainid;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;
	CWMP_CT_ALARM_T target[2];
	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	getAlarmInstNum(name, &instNum);
	if (instNum == 0)
		return ERR_9005;

	if (getAlarmByInstnum(instNum, pAlarm, &chainid) < 0)
		return ERR_9005;

	memset( &target[0], 0, sizeof( CWMP_CT_ALARM_T ) );
	memset( &target[1], 0, sizeof( CWMP_CT_ALARM_T ) );
	memcpy(&target[0], pAlarm, sizeof(CWMP_CT_ALARM_T));
	
	if (strcmp(lastname, "ParaList") == 0) {
		struct CWMP_LEAF *e;
		char *buf = data;

		if (buf == NULL || strlen(buf) == 0 || get_ParameterEntity(buf, &e) < 0)
			return ERR_9007;

		strncpy(pAlarm->alarmName, buf, PARA_NAME_LEN - 1);
		pAlarm->alarmName[PARA_NAME_LEN] = 0;
		updateAlarmTimer(pAlarm);
		
		memcpy(&target[1], pAlarm, sizeof(CWMP_CT_ALARM_T));
		if ( !mib_set(MIB_CWMP_CT_ALARM_MOD, (void *)&target)) 
	     	{
			return ERR_9007;
		}
#ifdef _CWMP_APPLY_
		apply_add(CWMP_PRI_L, apply_Alarm, CWMP_RESTART, 0, NULL, 0);
		return 0;
#else
		return 1;
#endif
	} else if (strcmp(lastname, "Limit-Max") == 0) {
		unsigned int *upper = data;
		int max;

		if( upper==NULL ) return ERR_9007;
		if( strlen(upper)==0 ) return ERR_9007;
		if((max=atoi(upper))<0) return ERR_9007;
		if(pAlarm->thresholdLower && max<pAlarm->thresholdLower) return ERR_9007;
		
		pAlarm->thresholdUpper= max;

		updateAlarmTimer(pAlarm);
		memcpy(&target[1], pAlarm, sizeof(CWMP_CT_ALARM_T));
		if ( !mib_set(MIB_CWMP_CT_ALARM_MOD, (void *)&target)) 
	     	{
			return ERR_9007;
		}
#ifdef _CWMP_APPLY_
		apply_add(CWMP_PRI_L, apply_Alarm, CWMP_RESTART, 0, NULL, 0);
		return 0;
#else
		return 1;
#endif
	} else if (strcmp(lastname, "Limit-Min") == 0) {
		unsigned int *lower = data;
		int min;

		if( lower==NULL ) return ERR_9007;
		if( strlen(lower)==0 ) return ERR_9007;
		if((min=atoi(lower))<0) return ERR_9007;
		if(pAlarm->thresholdUpper && min>pAlarm->thresholdUpper) return ERR_9007;
		
		pAlarm->thresholdLower= min;

		updateAlarmTimer(pAlarm);
		memcpy(&target[1], pAlarm, sizeof(CWMP_CT_ALARM_T));
		if ( !mib_set(MIB_CWMP_CT_ALARM_MOD, (void *)&target)) 
	     	{
			return ERR_9007;
		}
#ifdef _CWMP_APPLY_
		apply_add(CWMP_PRI_L, apply_Alarm, CWMP_RESTART, 0, NULL, 0);
		return 0;
#else
		return 1;
#endif
	} else if (strcmp(lastname, "TimeList") == 0) {
		unsigned int *period = data;
		int time;

		if( period==NULL ) return ERR_9007;
		if( strlen(period)==0 ) return ERR_9007;
		if((time=atoi(period))<0) return ERR_9007;
		pAlarm->alarmPeriod= time;
		
		updateAlarmTimer(pAlarm);
		memcpy(&target[1], pAlarm, sizeof(CWMP_CT_ALARM_T));
		if ( !mib_set(MIB_CWMP_CT_ALARM_MOD, (void *)&target)) 
	     	{
			return ERR_9007;
		}

#ifdef _CWMP_APPLY_
		apply_add(CWMP_PRI_L, apply_Alarm, CWMP_RESTART, 0, NULL, 0);
		return 0;
#else
		return 1;
#endif
	} else if (strcmp(lastname, "Mode") == 0) {
		unsigned int *mode = data;

		if (mode == NULL || *mode < ALARM_MODE_ADD || *mode > ALARM_MODE_INST)
			return ERR_9007;
		pAlarm->alarmMode = *mode;
		updateAlarmTimer(pAlarm);
		memcpy(&target[1], pAlarm, sizeof(CWMP_CT_ALARM_T));
		if ( !mib_set(MIB_CWMP_CT_ALARM_MOD, (void *)&target)) 
	     	{
			return ERR_9007;
		}
#ifdef _CWMP_APPLY_
		apply_add(CWMP_PRI_L, apply_Alarm, CWMP_RESTART, 0, NULL, 0);
		return 0;
#else
		return 1;
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int objCT_AlarmEntity(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;
	CWMP_CT_ALARM_T target[2];
	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&num);

	switch (type) {
	case eCWMP_tINITOBJ: 
	{
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		findMaxAlarmInstNum(&maxInstNum);

		for (i = 1; i <= num; i++) {
			*((char *)&alarm) = (char)i;
			memset( &target[0], 0, sizeof( CWMP_CT_ALARM_T ) );
			memset( &target[1], 0, sizeof( CWMP_CT_ALARM_T ) );
			if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)&alarm))
				continue;
			memcpy(&target[0], &alarm, sizeof(CWMP_CT_ALARM_T));

			if (pAlarm->InstanceNum == 0) {
				maxInstNum++;
				pAlarm->InstanceNum = maxInstNum;
				memcpy(&target[1], &alarm, sizeof(CWMP_CT_ALARM_T));
				if ( !mib_set(MIB_CWMP_CT_ALARM_MOD, (void *)&target)) 
	     			{
					return -1;
				}
			}

			if (create_Object(c, tCT_AlarmEntityObject,sizeof(tCT_AlarmEntityObject), 1, pAlarm->InstanceNum) < 0){
				return -1;
		}
		}

		add_objectNum(name, maxInstNum);

		break;
	}
	case eCWMP_tADDOBJ: {
		ret = add_Object(name, (struct CWMP_LINKNODE **)&entity->next,
				tCT_AlarmEntityObject, sizeof(tCT_AlarmEntityObject), data);
		if (ret >= 0) {
			CWMP_CT_ALARM_T entry;
			int totalNum=0;
			mib_get(MIB_CWMP_CT_ALARM_TBL_NUM, (void *)&totalNum);
			if((totalNum+1) > MAX_CWMP_CT_ALARM_NUM)
				ret=-1;
			else
			{
			memset(&entry, 0, sizeof(entry));
			entry.alarmNumber = 201000 + *(int *)data;
			entry.InstanceNum = *(int *)data;
				insertAlarmMonitorTimer(entry.InstanceNum, entry.alarmName[0]?entry.alarmPeriod*60:0, 0);
				mib_set(MIB_CWMP_CT_ALARM_ADD, (void *)&entry);
		}
		}
		if( ret >= 0 )
			ret=1;
		break;
	}
	case eCWMP_tDELOBJ: {
		int found = 0;
		unsigned int *pUint = data;

		for (i = 1; i <= num; i++) {
			*((char *)&alarm) = (char)i;
			if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)&alarm))
				continue;

			if (pAlarm->InstanceNum == *pUint) {
				found = 1;
				deleteAlarmMonitorTimerByInstnum(pAlarm->InstanceNum,0);
				mib_set(MIB_CWMP_CT_ALARM_DEL, (void *)&alarm);
				break;
			}
		}

		if (found) {
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next,*(int*)data );
		} else {
			ret = ERR_9005;
		}

		break;
	}
	case eCWMP_tUPDATEOBJ: 
	{
		struct CWMP_LINKNODE *old_table;

		old_table = (struct CWMP_LINKNODE *)entity->next;
		entity->next = NULL;

		for (i = 1; i <= num; i++) {
			struct CWMP_LINKNODE *remove_entity = NULL;

			*((char *)&alarm) = (char)i;
			if(!mib_get(MIB_CWMP_CT_ALARM_TBL, (void *)&alarm))
				continue;

			
			remove_entity = remove_SiblingEntity(&old_table, pAlarm->InstanceNum);

			if (remove_entity != NULL) {
				/* Tsai: it exists both in the MIB and the parameter tree */
				add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
			} else {
				/* Tsai: it exists only in the MIB,
				 * so we add it into the parameter tree */
				if (find_SiblingEntity((struct CWMP_LINKNODE **)&entity->next,
							pAlarm->InstanceNum ) == NULL) {
					add_Object(name, (struct CWMP_LINKNODE **)&entity->next,
						tCT_AlarmEntityObject, sizeof(tCT_AlarmEntityObject),
						&pAlarm->InstanceNum);
				}
				/* Tsai: if find_SiblingEntity() return non-NULL,
				 * then there are duplicate entries with the same InstanceNum
				 * in the MIB, so we only add the first entry into the
				 * parameter tree
				 */
			}
		}

		if (old_table) {
			/* Tsai: it exists only in the parameter tree,
			 * so we remove it from the parameter tree */
			destroy_ParameterTable((struct CWMP_NODE *)old_table);
		}

		break;
	}
	}

	return ret;
	}
#endif	
/*ping_zhang:20100128 END*/
#ifdef _PRMT_X_CT_COM_MONITOR_

int getMonitorByInstnum( unsigned int instnum, CWMP_CT_MONITOR_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) )
		return ret;

	mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		*((char *)p) = (char)i;
		if(!mib_get(MIB_CWMP_CT_MONITOR_TBL, (void *)p))
					continue;
		if( p->InstanceNum==instnum )
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}

unsigned int getMonitorPeriodByInstnum(unsigned int instnum)
{
	int ret=0;
	unsigned int i,num,val;
	CWMP_CT_MONITOR_T monitor, *pMonitor;
	unsigned int vChar=0;
	
	mib_get(MIB_CWMP_CT_MONITOR_ENABLED, (void *)&vChar);
	if(vChar!=0 ) {	
		mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);	
		for( i=1; i<=num;i++ )
		{
			pMonitor = &monitor;
			*((char *)&monitor) = (char)i;
			if(!mib_get(MIB_CWMP_CT_MONITOR_TBL, (void *)&monitor))
					continue;

			if(pMonitor->InstanceNum == instnum) {
				ret = pMonitor->monitorPeriod*60;
				break;
			}
		}	
	}
	return ret;	
}

unsigned int findMaxMonitorInstNum(void)
{
	int ret=0;
	unsigned int i,num;
	CWMP_CT_MONITOR_T monitor, *pMonitor;
		
	mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		pMonitor = &monitor;
		*((char *)&monitor) = (char)i;
		if(!mib_get(MIB_CWMP_CT_MONITOR_TBL, (void *)&monitor))
			continue;

		if( pMonitor->InstanceNum > ret )
			ret = pMonitor->InstanceNum;
	}	
	return ret;	
}

void init_monitor_timer(void)
{
	int i,num;
	CWMP_CT_MONITOR_T monitor, *pMonitor;
	unsigned int vChar=0;
	
	mib_get(MIB_CWMP_CT_MONITOR_ENABLED, (void *)&vChar);
	if( vChar==0 ) {
		clearAlarmMonitorTimer(1);
		return;
	}
	
	mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		pMonitor = &monitor;
		*((char *)&monitor) = (char)i;
		if(!mib_get(MIB_CWMP_CT_MONITOR_TBL, (void *)&monitor))
			continue;
		
		insertAlarmMonitorTimer(pMonitor->InstanceNum, pMonitor->monitorName[0]?pMonitor->monitorPeriod*60:0, 1);
	}	

	return;
}

int updateMonitorTimer(CWMP_CT_MONITOR_T *pMonitor)
{
	int ret=0;
	ALARM_TIMER *timer=monitor_timer;

	if(!pMonitor
		||pMonitor->monitorName[0] ==0
		||pMonitor->monitorPeriod==0)
		return ret;
	
	while(timer)
	{
		if(timer->instnum == pMonitor->InstanceNum)
		{
			ret = 1;
			timer->period = pMonitor->monitorPeriod*60;
			break;
		}
		timer = timer->next;
	}
	
	return ret;
}

unsigned int getMonitorInstNum( char *name )
{
	return getInstNum( name, "MonitorConfig" );
}

int getCT_Monitor(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int vChar=0;
	unsigned int num=0;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		
		mib_get(MIB_CWMP_CT_MONITOR_ENABLED, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "MonitorNumberOfEntries" )==0 )
	{
		mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);
		*data =uintdup(num);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_Monitor(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned int vChar=0;

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CWMP_CT_MONITOR_ENABLED, (void *)&vChar);
		init_monitor_timer();
	}else{
		return ERR_9005;
	}

	return 0;
}

int getCT_MonitorEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int instNum;
	unsigned int	chainid;
	CWMP_CT_MONITOR_T monitor, *pMonitor;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instNum = getMonitorInstNum( name );
	if(instNum==0) return ERR_9005;

	pMonitor = &monitor;
	if(getMonitorByInstnum(instNum, pMonitor, &chainid) <0)
		return ERR_9005;

	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ParaList" )==0 )
	{
		*data = strdup(pMonitor->monitorName);
	}else if( strcmp( lastname, "TimeList" )==0 )
	{	
		char timeList[8]={0};
		sprintf(timeList,"%d",pMonitor->monitorPeriod);
		*data = strdup(timeList);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_MonitorEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int instNum,chainid=-1;
	CWMP_CT_MONITOR_T monitor, *pMonitor;
	CWMP_CT_MONITOR_T target[2];
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	instNum = getMonitorInstNum( name );
	if(instNum==0) return ERR_9005;
	
	pMonitor = &monitor;
	if(getMonitorByInstnum(instNum, pMonitor, &chainid) <0)
		return ERR_9005;

	memset( &target[0], 0, sizeof( CWMP_CT_MONITOR_T ) );
	memset( &target[1], 0, sizeof( CWMP_CT_MONITOR_T ) );
	memcpy(&target[0], pMonitor, sizeof(CWMP_CT_MONITOR_T));
	if( strcmp( lastname, "ParaList" )==0 )
	{
		struct CWMP_LEAF  *e=NULL;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if(get_ParameterEntity(buf, &e)<0) return ERR_9007;
		
		strncpy(pMonitor->monitorName,buf,PARA_NAME_LEN-1);
		pMonitor->monitorName[PARA_NAME_LEN]=0;
		updateMonitorTimer(pMonitor);
	}
	else if( strcmp( lastname, "TimeList" )==0 )
	{
		int time;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		time = atoi(buf);
		if( time<0 ) return ERR_9007;
		pMonitor->monitorPeriod = time;
		updateMonitorTimer(pMonitor);
	}else{
		return ERR_9005;
	}

	memcpy(&target[1], pMonitor, sizeof(CWMP_CT_MONITOR_T));
	if ( !mib_set(MIB_CWMP_CT_MONITOR_MOD, (void *)&target)) 
	{
		return ERR_9007;
	}
	
	return 0;
}

int objCT_MonitorEntity(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	int i=0, ret=0;
	CWMP_CT_MONITOR_T *pMonitor, monitor;
	CWMP_CT_MONITOR_T target[2];
	 //CWMPDBG( 1, ( stderr, "<%s:%d>name=%s\n", __FUNCTION__, __LINE__,name ) );
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			int MaxInstNum=0,num=0;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
			MaxInstNum = findMaxMonitorInstNum();
			mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);

			for(i=1; i<=num;i++)
			{
				pMonitor = &monitor;

				*((char *)&monitor) = (char)i;
				memset( &target[0], 0, sizeof( CWMP_CT_MONITOR_T ) );
				memset( &target[1], 0, sizeof( CWMP_CT_MONITOR_T ) );
				if(!mib_get(MIB_CWMP_CT_MONITOR_TBL, (void *)&monitor))
						continue;
				memcpy(&target[0], &monitor, sizeof(CWMP_CT_MONITOR_T));
				if(pMonitor->InstanceNum==0)
				{
					MaxInstNum++;
					pMonitor->InstanceNum = MaxInstNum;
					memcpy(&target[1], &monitor, sizeof(CWMP_CT_MONITOR_T));
					if ( !mib_set(MIB_CWMP_CT_MONITOR_MOD, (void *)&target)) 
		     			{
						return -1;
					}
				}

				if( create_Object( c, tCT_MonitorEntityObject, sizeof(tCT_MonitorEntityObject), 1, pMonitor->InstanceNum ) < 0 )
					return -1;
			}

			add_objectNum( name, MaxInstNum);
			break;
		}
	case eCWMP_tADDOBJ:
	     	{
		     	
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
			
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tCT_MonitorEntityObject, sizeof(tCT_MonitorEntityObject), data );
			if( ret >= 0 )
			{
				CWMP_CT_MONITOR_T entry;
				memset( &entry, 0, sizeof( CWMP_CT_MONITOR_T ) );
				{
					entry.InstanceNum =*(int*)data;
				}
				insertAlarmMonitorTimer(entry.InstanceNum, 0, 1);
				mib_set(MIB_CWMP_CT_MONITOR_ADD, (void *)&entry);
			}
			
			if( ret >= 0 )
				ret=1;
			break;
	     	}
	case eCWMP_tDELOBJ:
	     	{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);
			for( i=1;i<=num;i++ )
			{
				pMonitor = &monitor;
				*((char *)&monitor) = (char)i;
				if(!mib_get(MIB_CWMP_CT_MONITOR_TBL, (void *)&monitor))
						continue;
				if(pMonitor->InstanceNum==*pUint)
				{
					found = 1;
					deleteAlarmMonitorTimerByInstnum(pMonitor->InstanceNum, 1);
					mib_set(MIB_CWMP_CT_ALARM_DEL, (void *)&monitor);
					break;
				}
			}

			if(found)
				ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			else
				ret= ERR_9005;
			break;
		}
	case eCWMP_tUPDATEOBJ:	
	        {
			int num,i;
			struct CWMP_LINKNODE *old_table;

			mib_get(MIB_CWMP_CT_MONITOR_TBL_NUM, (void *)&num);
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=1; i<=num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pMonitor = &monitor;
				*((char *)&monitor) = (char)i;
				if(!mib_get(MIB_CWMP_CT_MONITOR_TBL, (void *)&monitor))
						continue;

				remove_entity = remove_SiblingEntity( &old_table, pMonitor->InstanceNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pMonitor->InstanceNum )==NULL ) 
					{
						unsigned int MaxInstNum = pMonitor->InstanceNum;					
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tCT_MonitorEntityObject, sizeof(tCT_MonitorEntityObject), &MaxInstNum );
					}//else already in next_table
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	
			break;
		}
	}

	return ret;
}
#endif

#ifdef _PRMT_X_CT_COM_UPNP_

int getCT_Upnp(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int GetValue=0;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_UPNP_ENABLED, (void *)&GetValue);
		if(GetValue)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_Upnp(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int SetValue=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		SetValue = (*i==0)?0:1;

		mib_set(MIB_UPNP_ENABLED, (void *)&SetValue);
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif
/*ping_zhang:20100525 END*/

#ifdef _PRMT_X_CT_COM_USB_RESTORE_

int getCT_USB_Restore(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int GetValue=0;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_USB_RESTORE_ENABLED, (void *)&GetValue);
		if(GetValue)
			*data = booldup( 1 );
		else
			*data = booldup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_USB_Restore(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int SetValue=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		SetValue = (*i==0)?0:1;

		mib_set(MIB_USB_RESTORE_ENABLED, (void *)&SetValue);
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif
/*ping_zhang:20100525 END*/


#ifdef _PRMT_X_CT_COM_IPVERSION_

int getCT_IPProtocolVersion(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int mode;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp( lastname, "Mode" )==0) {
		if (!mib_get(MIB_IP_PROTOCOL_VERSION, &mode))
			return ERR_9001;
		*data = uintdup(mode);
	} else {
		return ERR_9005;
	}
	
	return 0;
}

int setCT_IPProtocolVersion(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int vInt;
	unsigned int mode;
	FILE *proc;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp( lastname, "Mode" )==0) {
		mode = *(unsigned int *)data;

		if (mode < 1 || mode > 3)
			return ERR_9007;

		/* Tsai: IPv4 should be always enabled,
		 * so only testing whether IPv6 should be disabled */
		mode |= 1;
		vInt = (mode & 2) ? '0' : '1';

		mib_set(MIB_IP_PROTOCOL_VERSION, &mode);
		
#if 0		//NOT SUPPORT YET! 2012-01-10
		proc = fopen("/proc/sys/net/ipv6/conf/br0/disable_ipv6", "w");
		if (proc) {
			fputc(vInt, proc);
			fclose(proc);
}
#endif			
	} else {
		return ERR_9005;
	}

	return 0;
}
#endif



#ifdef _PRMT_X_CT_COM_PING_

enum eCT_PingDiagState
{
	eCTPing_None,
	eCTPing_Requested,
	eCTPing_Complete,
	eCTPing_Error_CannotResolveHostName,
	eCTPing_Error_Internal,
	eCTPing_Error_Other,
	eCTPing_Error_Stop,
};
char *strCT_PingDiagState[] =
{
	"None",
	"Requested",
	"Complete",
	"Error_CannotResolveHostName",
	"Error_Internal",
	"Error_Other",
	"Error_Stop",
	"Requested"
};
unsigned int getPingInstNum( char *name )
{
	return getInstNum( name, "PingConfig" );
}
#if 0
static int transfer2Ifindex( char *name)
{
	struct CWMP_LEAF *e=NULL;
	char ifname[10];
    
	if( (name==NULL)) return -1;
	if( get_ParameterEntity( name, &e ) < 0 ) return -1;

	//lan interface
	if( strcmp( name, "InternetGatewayDevice.LANDevice.1" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1." )==0)
		return(getLanPort());
#if (defined(CONFIG_EXT_SWITCH)  || defined(IP_QOS_VPORT))
#ifndef CONFIG_CT_TWO_LANPORTS
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" )==0
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4." )==0)
		return(BRG_LAN_SWITCH_PORT4);
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3." )==0)
		return(BRG_LAN_SWITCH_PORT3);
#endif
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2." )==0)
		return(BRG_LAN_SWITCH_PORT2);
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1." )==0)
		return(BRG_LAN_SWITCH_PORT1);
#else
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1." )==0)
		return(BRG_NIC_PORT0);
#endif

	//wlan interface
#ifdef CONFIG_WLAN
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" )==0
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1." )==0 )
		return(BRG_WLAN_PORT0_ROOT); 
#ifdef CONFIG_MBSSID
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2." )==0 )
		return(BRG_WLAN_PORT0_VAP0); 
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3." )==0 )
		return(BRG_WLAN_PORT0_VAP1);
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" )==0
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4." )==0)
		return(BRG_WLAN_PORT0_VAP2);
	else if( strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" )==0 
		||strcmp( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5." )==0)
		return(BRG_WLAN_PORT0_VAP3);
#endif //CONFIG_MBSSID
#endif //CONFIG_WLAN

	//wan interface
	else
	{
		unsigned int devnum,ipnum,pppnum;
		set_adsl_entry *pEntry,vc_entity;
		int total,i;
		
		devnum = getWANConDevInstNum( name );
		ipnum  = getWANIPConInstNum( name );
		pppnum = getWANPPPConInstNum( name );
		if( (devnum==0) || ((ipnum==0)&&(pppnum==0)) ) return -1;

		total = mib_chain_total(CONFIG_ATM_VC_TBL);
		for( i=0; i<total; i++ )
		{
			pEntry = &vc_entity;
			if( 0 == mib_chain_get(CONFIG_ATM_VC_TBL, i, (void*)pEntry ) )
				continue;
			
			if( (pEntry->ConDevInstNum==devnum) &&
			    (pEntry->ConIPInstNum==ipnum) &&
			    (pEntry->ConPPPInstNum==pppnum) ) 
			{
				return(pEntry->ifindex);			
				break;
			}
		}	
	}
	return -1;	
}

int transfer2Path( unsigned int ifindex, char *name )
{
	int total,i;
	set_adsl_entry *pEntry, vc_entity;

	if( ifindex==0xff ) return -1;
	if( name==NULL ) return -1;
	name[0]=0;

#if (defined(CONFIG_EXT_SWITCH)  || defined(IP_QOS_VPORT))
#ifndef CONFIG_CT_TWO_LANPORTS
	if(ifindex==BRG_LAN_SWITCH_PORT4)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" );
	else if(ifindex==BRG_LAN_SWITCH_PORT3)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" );
	else
#endif
	if(ifindex==BRG_LAN_SWITCH_PORT2)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" );
	else  if(ifindex==BRG_LAN_SWITCH_PORT1)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
#else
	if(ifindex==BRG_NIC_PORT0)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" );
#endif

	//wlan interface
#ifdef CONFIG_WLAN
	else if(ifindex==BRG_WLAN_PORT0_ROOT)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" );
#ifdef CONFIG_MBSSID
	else if(ifindex==BRG_WLAN_PORT0_VAP0)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" );
	else if(ifindex==BRG_WLAN_PORT0_VAP1)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" );
	else if(ifindex==BRG_WLAN_PORT0_VAP2)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" );
	else if(ifindex==BRG_WLAN_PORT0_VAP3)
		strcpy( name, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" );
#endif //CONFIG_MBSSID
#endif //CONFIG_WLAN

	//wan interface
	else {
		total = mib_chain_total(CONFIG_ATM_VC_TBL);
		for( i=0; i<total; i++ )
		{
			pEntry = &vc_entity;
			if( 0 == mib_chain_get(CONFIG_ATM_VC_TBL, i, (void*)pEntry ) )
				continue;
			if(pEntry->ifindex==ifindex)
			{
				char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
				char ipstr[]="WANIPConnection";
				char pppstr[]="WANPPPConnection";
				char *pconn=NULL;
				unsigned int instnum=0;

				if( (pEntry->adslconnmode==WEB_PPPOE) || (pEntry->adslconnmode==WEB_PPPOA) )
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
	}
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	return 0;
}

#endif

int getPingByInstnum( unsigned int instnum, CWMP_CT_PING_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	mib_get(MIB_CWMP_CT_PING_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{

		*((char *)p) = (char)i;
		 mib_get( MIB_CWMP_CT_PING_TBL, (void*)p);
		if( p->InstanceNum==instnum )
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}

int getPingByDiagState( unsigned int diagState, CWMP_CT_PING_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (p==NULL) || (id==NULL) )
		return ret;

	mib_get(MIB_CWMP_CT_PING_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		*((char *)p) = (char)i;
		 mib_get( MIB_CWMP_CT_PING_TBL, (void*)p);

		if( p->diagnosticsState==diagState)
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}

unsigned int findMaxPingInstNum()
{
	int ret=0;
	unsigned int i,num;
	CWMP_CT_PING_T ping, *pPing;
		
	mib_get(MIB_CWMP_CT_PING_TBL_NUM, (void *)&num);
	for( i=1; i<=num;i++ )
	{
		pPing = &ping;
		*((char *)pPing) = (char)i;
		if( ! mib_get( MIB_CWMP_CT_PING_TBL, (void*)pPing) )
			continue;

		if( pPing->InstanceNum > ret )
			ret = pPing->InstanceNum;
	}	
	return ret;	
}

#if 0
static void *ctipping_thread(void *arg) {
	CCB *ccbptr;
	int index=0,count,cid,i=0,status;
	uint32 tstart, tend, tdiff;
	ipping_pt pent;
	unsigned int chainid,chainidNew;
	CWMP_CT_PING_T *p,pEntry,pNew;
       CWMP_CT_PING_T *pArg=(CWMP_CT_PING_T *)arg;
       struct in_addr in;
	OSKMAILBOX queue;
	unsigned int uOK = 0, uFail = 0;
	unsigned int tAvg = 0, tMin = 0, tMax = 0;
       unsigned char ctPingEnable=0;
xprintfk("%s:%d pArg=0x%X\n",__FUNCTION__,__LINE__,pArg);
	p = &pEntry;
	if(getPingByInstnum(pArg->InstanceNum, p, &chainid) <0) {
		xprintfk("%s:%d The CT ping entry that InstanceNum=%d is not exist!\n",__FUNCTION__,__LINE__,pArg->InstanceNum);
              free(pArg);
		return;
	}

	//update diagnosticsState
	p->diagnosticsState=eCTPing_Requested;
	mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
	
	if(  inet_aton( pArg->host, &in )==0  ) {
		p->diagnosticsState=eCTPing_Error_Internal;
		mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
              free(pArg);
		return;
	}
    
	count=p->numberOfRepitition;
	if(p->ifindex==0xff)
		cid=0;
	else {
		unsigned int ifindex=p->ifindex;
		icirc_pt c;
		if(ifindex==BRG_LAN_SWITCH_PORT1 || ifindex==BRG_LAN_SWITCH_PORT2
#ifndef CONFIG_CT_TWO_LANPORTS
			||ifindex==BRG_LAN_SWITCH_PORT3 || ifindex==BRG_LAN_SWITCH_PORT4
#endif
#ifdef CONFIG_WLAN
			|| ifindex==BRG_WLAN_PORT0_ROOT
#ifdef CONFIG_MBSSID
			|| ifindex==BRG_WLAN_PORT0_VAP0 || ifindex==BRG_WLAN_PORT0_VAP1
			|| ifindex==BRG_WLAN_PORT0_VAP2 || ifindex==BRG_WLAN_PORT0_VAP3
#endif //CONFIG_MBSSID
#endif //CONFIG_WLAN
			)
			ifindex=BRG_NIC_PORT0;
		if(c=(icirc_pt)(getCircFromPhyport(ifindex)))
			cid=c->rc_id;
		else
			cid=0;
	}
	//xprintfk("%s:%d cid=%d\n",__FUNCTION__,__LINE__,cid);
	do{
		if(getPingByInstnum(p->InstanceNum, &pNew, &chainidNew)<0) {
			p->diagnosticsState=eCTPing_Error_Internal;
			mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
                     free(pArg);
			return;
		}
              cfg_mib_get( CONFIG_CWMP_CT_PING_ENABLE, (void *)&ctPingEnable);
		if(pNew.stop || ctPingEnable==0)
			break;
		tstart = oskTimerGetMilliSecs();
		ccbptr = (CCB *)ctpingEcho(in.s_addr, 0, cid, p->dataBlockSize, p->dscp, p->timeout);
		tend = oskTimerGetMilliSecs();
		tdiff = tend-tstart;

		if (ccbptr && (ccbptr->status == PINGOK))
		{
			/*should get information from ccb*/
			pent = (ipping_pt) ccbptr->datap;
			uOK++;
			// delta = (time2.tv_sec * 1000 + time2.tv_usec / 1000) - (time1.tv_sec * 1000 + time1.tv_usec / 1000);

			if (uOK == 1) {
				tAvg = tMin = tMax = tdiff;
			} else {
				tAvg = (unsigned int)(tAvg*(uOK-1) + tdiff)/uOK;
				if (tMin > tdiff) tMin = tdiff;
				if (tMax < tdiff)tMax = tdiff;
			}
			xprintfk( "%d : %d bytes from %s: icmp_seq=%d ttl=%d time=%d ms\n", 
		            index+1, p->dataBlockSize, getIPString(pent->p_ipa), index+1, pent->p_ttl, tdiff);
		}
		else /*fail*/
		{
			uFail++;
			xprintfk("%d : timeout.\n", index+1);
		}
		
		if(p->numberOfRepitition==0)
			count=2;
		index++;
		xsleep( p->interval*oskTimerGetTicksPerSecond() );
	}while(--count>0);

	/*here uOK, uFail, tAvg, tMin, tMax can be used for statistics*/
	if(count==0)
		p->diagnosticsState=eCTPing_Complete;
	else
		p->diagnosticsState=eCTPing_Error_Stop;
	mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
	cwmpDiagnosticDone();
	//xprintfk("ping test return %d\n", res);
       free(pArg);
	return NULL;
}

void cwmpStartCTPingDiag() {
	unsigned int chainid;
	uint32 ipaddr;
	CWMP_CT_PING_T *p;
	pthread_t ping_pid;
	struct hostent *h;
	//printf("\n\nPing commence!!\n");
	xprintfk("%s:%d\n",__FUNCTION__,__LINE__);
	p = (CWMP_CT_PING_T *)malloc(sizeof(CWMP_CT_PING_T));
	if(getPingByInstnum(gStartCTPing, p, &chainid)<0) {
		p->diagnosticsState=eCTPing_Error_Internal;
		mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
		return;
	}
	
	if (p->host && (h=gethostbyname(p->host))) {
		if(h->h_addrtype!=AF_INET){
			p->diagnosticsState=eCTPing_Error_Internal;
			mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
			return;
		}
		ipaddr=*(ipa*)h->h_addr;
              strcpy(p->host,getIPString(ipaddr));
	}
	else{
		p->diagnosticsState=eCTPing_Error_CannotResolveHostName;
		mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
		return;
	}
#if 0
	if( pthread_create( &ping_pid, NULL, ctipping_thread, (void *)p ) != 0 )
	{
		p->diagnosticsState=eCTPing_Error_Internal;
		mib_chain_update(CONFIG_CWMP_CT_PING_TBL, chainid, (void *) p);
		return;
	}
	pthread_detach(ping_pid);
#endif

}
#endif

int getCT_Ping(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int entryNum=0;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		int vInt=0;
		mib_get( MIB_CWMP_CT_PING_ENABLED, (void *)&vInt);
		*data = booldup( vInt!=0 );
	}else if( strcmp( lastname, "PingNumberOfEntries" )==0 )
	{
		mib_get(MIB_CWMP_CT_PING_TBL_NUM, (void *)&entryNum);
		*data =uintdup(entryNum);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_Ping(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned int vInt=0;

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vInt = (*i==0)?0:1;
		mib_set( MIB_CWMP_CT_PING_ENABLED, (void *)&vInt);
	}else{
		return ERR_9005;
	}

	return 0;
}

int getCT_PingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int instNum;
	unsigned int	chainid;
	CWMP_CT_PING_T ping, *pPing;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instNum = getPingInstNum( name );
	if(instNum==0) return ERR_9005;

	pPing = &ping;
	if(getPingByInstnum(instNum, pPing, &chainid) <0)
		return ERR_9005;

	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		*data = strdup(strCT_PingDiagState[pPing->diagnosticsState]);
	}else if( strcmp( lastname, "Interface" )==0 )
	{	
		if(pPing->IfaceName[0])
			*data = strdup(pPing->IfaceName);
		else
		*data = strdup("");
	}else if( strcmp( lastname, "Host" )==0 )
	{	
		if(pPing->host[0])
		*data = strdup(pPing->host);
		else
			*data = strdup("");
	}else if( strcmp( lastname, "NumberOfRepetitions" )==0 )
	{	
		*data = uintdup(pPing->numberOfRepitition);
	}else if( strcmp( lastname, "TimeOut" )==0 )
	{	
		*data = uintdup(pPing->timeout);
	}else if( strcmp( lastname, "DataBlockSize" )==0 )
	{	
		*data = uintdup(pPing->dataBlockSize);
	}else if( strcmp( lastname, "DSCP" )==0 )
	{
		*data = uintdup(pPing->dscp);
	}else if( strcmp( lastname, "Interval" )==0 )
	{
		*data = uintdup(pPing->interval);
	}else if( strcmp( lastname, "Stop" )==0 )
	{
		*data = booldup(pPing->stop);
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int setCT_PingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int instNum,chainid=-1;
	CWMP_CT_PING_T ping, *pPing;
	CWMP_CT_PING_T target[2];
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	instNum = getPingInstNum( name );
	if(instNum==0) return ERR_9005;
	pPing = &ping;
	if(getPingByInstnum(instNum, pPing, &chainid) <0)
		return ERR_9005;

	memset( &target[0], 0, sizeof( CWMP_CT_PING_T ) );
	memset( &target[1], 0, sizeof( CWMP_CT_PING_T ) );
	
	memcpy(&target[0], &ping, sizeof(CWMP_CT_PING_T));
	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		
		if( buf==NULL ) return ERR_9007;
		if( strcmp(buf,strCT_PingDiagState[eCTPing_Requested])) return ERR_9007;
		
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		pPing->diagnosticsState=eCTPing_Requested;
		pPing->numberOfRemain=pPing->numberOfRepitition;
		pPing->intervalRemain=0;
		pPing->stop=0;
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
		
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		int ifindex;
		if( buf==NULL || strlen(buf)>ITF_HOST_LEN) return ERR_9007;
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		if( strlen(buf)==0 ){
			pPing->IfaceName[0]=0x0;
		}else{
			strcpy(pPing->IfaceName, (char *)buf);
		}
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else if( strcmp( lastname, "Host" )==0 )
	{
		if( buf==NULL || strlen(buf)>ITF_HOST_LEN) return ERR_9007;
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		if( strlen(buf)==0 ){
			pPing->host[0]=0x0;
		}else{
			strcpy(pPing->host, (char *)buf);
/*rong_wang 20110428 START: make the hostIP to be resolved again when host is modified by TR069*/			
		pPing->hostIP[0]=0;
		pPing->hostIP[1]=0;
		pPing->hostIP[2]=0;
		pPing->hostIP[3]=0;
		}
/*rong_wang 20110428 END */
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else if( strcmp( lastname, "NumberOfRepetitions" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpbool;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL) return ERR_9007;
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		pPing->numberOfRepitition=*i;
		pPing->numberOfRemain=*i;
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else if( strcmp( lastname, "Timeout" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpbool;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL || *i<1 ) return ERR_9007;
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		pPing->timeout=*i;
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else if( strcmp( lastname, "DataBlockSize" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpbool;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL || *i<1  || *i>65535) return ERR_9007;
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		pPing->dataBlockSize=*i;
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else if( strcmp( lastname, "DSCP" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpbool;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL  || *i>63) return ERR_9007;
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		pPing->dscp=*i;
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else if( strcmp( lastname, "Interval" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpbool;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL || *i<1 ) return ERR_9007;
		if(pPing->diagnosticsState==eCTPing_Requested) return ERR_9001;
		
		pPing->interval=*i;
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else if( strcmp( lastname, "Stop" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL || *i>1) return ERR_9007;
		
		
		pPing->stop=*i;
		memcpy(&target[1], &ping, sizeof(CWMP_CT_PING_T));
		mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
	}else{
		return ERR_9005;
	}

	return 0;
}

int objCT_PingEntity(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	int i=0;
	CWMP_CT_PING_T *pPing, monitor;
	CWMP_CT_PING_T target[2];
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			int MaxInstNum=0,num=0;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = findMaxPingInstNum();
			mib_get(MIB_CWMP_CT_PING_TBL_NUM, (void *)&num);

			for(i=1; i<=num;i++)
			{
				pPing = &monitor;
				*((char *)pPing) = (char)i;
				memset( &target[0], 0, sizeof( CWMP_CT_PING_T ) );
				memset( &target[1], 0, sizeof( CWMP_CT_PING_T ) );
				if( ! mib_get( MIB_CWMP_CT_PING_TBL, (void*)pPing) )
					continue;
				memcpy(&target[0], &monitor, sizeof(CWMP_CT_PING_T));
				if(pPing->InstanceNum==0)
				{
					MaxInstNum++;
					pPing->InstanceNum = MaxInstNum;
					memcpy(&target[1], &monitor, sizeof(CWMP_CT_PING_T));
					if ( !mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target)) 
	     				{
							return -1;
					}
					
				}
				if( create_Object( c, tCT_PingEntityObject, sizeof(tCT_PingEntityObject), 1, pPing->InstanceNum ) < 0 )
					return -1;
			}

			add_objectNum( name, MaxInstNum);
			return 0;
		}
	case eCWMP_tADDOBJ:
	     	{
		     	int ret;
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
			
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tCT_PingEntityObject, sizeof(tCT_PingEntityObject), data );
			if( ret >= 0 )
			{
				CWMP_CT_PING_T entry;
				memset( &entry, 0, sizeof( CWMP_CT_PING_T ) );
				{
					entry.interval=1;
					entry.InstanceNum =*(int*)data;
				}
				if ( !mib_set(MIB_CWMP_CT_PING_ADD, (void *)&entry)) 
	     			{
							ret= -1;
				}
			}
			if(ret >=0) ret=1;
			return ret;
	     	}
	case eCWMP_tDELOBJ:
	     	{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			mib_get(MIB_CWMP_CT_PING_TBL_NUM, (void *)&num);
			for( i=1;i<=num;i++ )
			{
				pPing = &monitor;
				*((char *)pPing) = (char)i;
				if( ! mib_get( MIB_CWMP_CT_PING_TBL, (void*)pPing) )
					continue;
				if(pPing->InstanceNum==*pUint)
				{
					found = 1;
					mib_set( MIB_CWMP_CT_PING_DEL, (void*)pPing);
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

			mib_get(MIB_CWMP_CT_PING_TBL_NUM, (void *)&num);
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=1; i<=num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pPing = &monitor;
				*((char *)pPing) = (char)i;
				if( ! mib_get( MIB_CWMP_CT_PING_TBL, (void*)pPing) )
					continue;

				remove_entity = remove_SiblingEntity( &old_table, pPing->InstanceNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pPing->InstanceNum )==NULL ) 
					{
						unsigned int MaxInstNum = pPing->InstanceNum;					
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tCT_PingEntityObject, sizeof(tCT_PingEntityObject), &MaxInstNum );
					}//else already in next_table
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	

			return 0;
		}
	}

	return -1;
}




int checkCTPing(void) 
{
	CWMP_CT_PING_T *p,pEntry;
	CWMP_CT_PING_T target[2];
	char pifname[16];
	unsigned int i,num;	
	unsigned int vInt=0;
	struct hostent *h;
	int oldremainnum;
	int res=0;
	int needUpdateConfig=0;
	unsigned int SuccessCount;
	unsigned int FailureCount;
	unsigned int AverageResponseTime;
	unsigned int MinimumResponseTime;
	unsigned int MaxmumResponseTime;



	mib_get( MIB_CWMP_CT_PING_ENABLED, (void *)&vInt);
	if(vInt==0)
		return;

	mib_get( MIB_CWMP_CT_PING_TBL_NUM,(void *)&num);
	for( i=1; i<=num; i++ )
	{ 
		p = &pEntry;
		oldremainnum=0;
		
		*((char *)p) = (char)i;
		 mib_get( MIB_CWMP_CT_PING_TBL, (void*)p);
		memcpy(&target[0], &pEntry, sizeof(CWMP_CT_PING_T));
		
		if( p->diagnosticsState==eCTPing_Requested)
		{      
			oldremainnum=p->numberOfRemain;
			/*check whether ping is stoped*/
			if(p->stop)
			{
				p->diagnosticsState=eCTPing_Error_Stop;
				
				memcpy(&target[1], &pEntry, sizeof(CWMP_CT_PING_T));
        			mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
				needUpdateConfig++;
        			continue;
			}
			
			/*check whether ping is complete*/
			if(p->numberOfRepitition && p->numberOfRemain==0)
			{
				p->diagnosticsState=eCTPing_Complete;
				
        			memcpy(&target[1], &pEntry, sizeof(CWMP_CT_PING_T));
        			mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
				needUpdateConfig++;
        			continue;
			}

			/*check wheter host have been resolved*/
                	if (p->hostIP[0]==0) {
				if( p->host[0] && (h=gethostbyname(p->host))) {
	                		if(h->h_addrtype!=AF_INET){
	                			p->diagnosticsState=eCTPing_Error_Internal;
								
	                			memcpy(&target[1], &pEntry, sizeof(CWMP_CT_PING_T));
        					mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
						needUpdateConfig++;
	                			continue;
	                		}
					memcpy(&p->hostIP[0], h->h_addr, sizeof(unsigned int));	
					memcpy(&target[1], &pEntry, sizeof(CWMP_CT_PING_T));
        				mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
					needUpdateConfig++;
	                	}
	                	else{
	                		p->diagnosticsState=eCTPing_Error_CannotResolveHostName;
							
	                		memcpy(&target[1], &pEntry, sizeof(CWMP_CT_PING_T));
        				mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
					needUpdateConfig++;
	                		continue;
	                	}
                	}

			if(GetNetdevName(p->IfaceName, pifname)==0){
				//printf("convert %s to %s\n", p->IfaceName, pifname);
			}else if (transfer2IfName(p->IfaceName, pifname)) {
					//printf("convert %s Fail!\n", p->IfaceName);
					pifname[0] = 0x0;
			} else {
					LANDEVNAME2BR0(pifname);
			}

			/*check interval*/
			//printf("%s:%d p->intervalRemain=%d\n",__FUNCTION__,__LINE__,p->intervalRemain);
			if(p->interval > 1){
				if(p->intervalRemain)
				{
					p->intervalRemain--;
				}
				else 
				{
					//here just send ping request and don't wait ping response
				/*res = icmp_test(pifname, p->host, 1, p->timeout*1000, p->dataBlockSize, p->dscp, &SuccessCount, 
			&FailureCount, &AverageResponseTime, &MinimumResponseTime, &MaxmumResponseTime, 0);*/
					
					p->intervalRemain=p->interval-1;
					if(p->numberOfRepitition)
						p->numberOfRemain--;
				}
			}
			else if(p->interval == 1) {
				/*res = icmp_test(pifname, p->host, 1, p->timeout*1000, p->dataBlockSize, p->dscp, &SuccessCount, 
			&FailureCount, &AverageResponseTime, &MinimumResponseTime, &MaxmumResponseTime, 0);*/
				p->intervalRemain=p->interval;
					if(p->numberOfRepitition)
						p->numberOfRemain--;
			}

			memcpy(&target[1], &pEntry, sizeof(CWMP_CT_PING_T));
			mib_set(MIB_CWMP_CT_PING_MOD, (void *)&target);
			needUpdateConfig++;
			
			/*save ping entry's numberOfRemain to flash*/	
			if(oldremainnum!=p->numberOfRemain)
				needUpdateConfig = needUpdateConfig+1;
								
		}
        	
       }
	if(needUpdateConfig > 0){
		//CWMPDBG( 1, ( stderr, "<%s:%d>May Update to flash for entry data \n", __FUNCTION__, __LINE__) );
		//apmib_update(CURRENT_SETTING);
	}
	return 0;
}



#endif









