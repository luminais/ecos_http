#include <string.h>
#include "prmt_igd.h"
#include "prmt_deviceinfo.h"
#include "prmt_mngmtserver.h"
#include "prmt_layer3fw.h"
#include "prmt_landevice.h"
#include "prmt_ippingdiag.h"
#include "prmt_wandevice.h"
//#define MIB_CWMP_LAN_CONFIGPASSWD 711


#ifdef TIME_ZONE
#include "prmt_time.h"
#endif
#if defined( _PRMT_SERVICES_)
#include "prmt_services.h"
#endif
#ifdef _PRMT_USERINTERFACE_
#include "prmt_userif.h"
#endif 
#ifdef _PRMT_X_CT_COM_QOS_
#include "prmt_ctcom_queuemnt.h"
#endif
#ifdef _PRMT_X_STD_QOS_
#include "prmt_ctcom_queuemnt.h"
#elif IP_QOS
#include "prmt_queuemnt.h"
#endif
#ifdef _PRMT_TR143_
#include "prmt_tr143.h"
#endif //_PRMT_TR143_
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#include "prmt_captiveportal.h"
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
#include "prmt_traceroute.h"
#endif //_SUPPORT_TRACEROUTE_PROFILE_

#ifdef _PRMT_TR143_
#define _PRMT_CAPABILITIES_
#endif

#define LANDEVICE_NUM	1	/* one instance of LANDevice*/
#define WANDEVICE_NUM	1	/* one instance of WANDevice*/

/******DEVICESUMMARY*****************************************************************************/
//baseline profile
#ifdef _PRMT_WT107_
#define DS_PREFIX	"InternetGatewayDevice:1.4[](Baseline:2"
#else
#define DS_PREFIX	"InternetGatewayDevice:1.1[](Baseline:1"
#endif //_PRMT_WT107_

#define DS_ETH_WAN		",EthernetWAN:1"

//ethernetlan profile
#ifdef _PRMT_WT107_
#define DS_ETH		",EthernetLAN:2"
#else
#define DS_ETH		",EthernetLAN:1"
#endif //_PRMT_WT107_

//usblan profile
#ifdef _PRMT_USB_ETH_
#ifdef _PRMT_WT107_
#define DS_USB		",USBLAN:2"
#else
#define DS_USB		",USBLAN:1"
#endif //_PRMT_WT107_
#else
#define DS_USB		""
#endif //_PRMT_USB_ETH_

//wifilan profile
#ifdef WLAN_SUPPORT
#ifdef _PRMT_WT107_
#define DS_WIFI		",WiFiLAN:2"
#else
#define DS_WIFI		",WiFiLAN:1"
#endif //_PRMT_WT107_
#else
#define DS_WIFI		""
#endif //WLAN_SUPPORT

//wifiwmm profile
#ifdef _SUPPORT_WIFIWMM_PROFILE_
#define DS_WIFIWMM		",WiFiWMM:1"
#else
#define DS_WIFIWMM		""
#endif //_SUPPORT_WIFIWMM_PROFILE_

//wifiwps profile
#ifdef _SUPPORT_WIFIWPS_PROFILE_
#define DS_WIFIWPS		",WiFiWPS:1"
#else
#define DS_WIFIWPS		""
#endif //_SUPPORT_WIFIWPS_PROFILE_

#ifdef CONFIG_DEV_xDSL
//adslwan profile
#define DS_ADSLWAN		",ADSLWAN:1"
#else
#define DS_ADSLWAN		""
#endif

//adsl2wan profile
#ifdef _SUPPORT_ADSL2WAN_PROFILE_
#define DS_ADSL2WAN		",ADSL2WAN:1"
#else
#define DS_ADSL2WAN		""
#endif //_PRMT_WT107_

//qos profile
#ifdef IP_QOS
#ifdef _PRMT_WT107_
#define DS_QOS		",QoS:2"
#else
#define DS_QOS		",QoS:1"
#endif //_PRMT_WT107_
#else
#define DS_QOS		""
#endif //IP_QOS

//time profile
#ifdef TIME_ZONE
#ifdef _PRMT_WT107_
#define DS_TIME		",Time:2"
#else
#define DS_TIME		",Time:1"
#endif //_PRMT_WT107_
#else
#define DS_TIME		""
#endif

//captiveportal profile
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#define DS_CAPTIVEPORTAL	",CaptivePortal:1"
#else
#define DS_CAPTIVEPORTAL	""
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_

//ipping profile
#define DS_PING		",IPPing:1"

//traceroute profile
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
#define DS_TRACEROUTE	",TraceRoute:1"
#else
#define DS_TRACEROUTE	""
#endif //_SUPPORT_TRACEROUTE_PROFILE_

//download,downloadtcp,upload,uploadtcp,udpecho,udpechoplus profiles
#ifdef _PRMT_TR143_
#define DS_TR143	",Download:1,DownloadTCP:1,Upload:1,UploadTCP:1,UDPEcho:1,UDPEchoPlus:1"
#else
#define DS_TR143	""
#endif //_PRMT_TR143_

#ifdef CONFIG_DEV_xDSL
//atmloopback profile
#define DS_ATMLB	",ATMLoopback:1"
#else
#define DS_ATMLB	""

#endif
#ifdef CONFIG_DEV_xDSL
//dsldiagnostics profile
#define DS_DSLDIAG	",DSLDiagnostics:1"
#else
#define DS_DSLDIAG	""
#endif
//adsl2dsldiagnostics profile
#ifdef _SUPPORT_ADSL2DSLDIAG_PROFILE_
#define DS_DSL2DIAG	",ADSL2DSLDiagnostics:1"
#else
#define DS_DSL2DIAG	""
#endif //_SUPPORT_ADSL2DSLDIAG_PROFILE_

//deviceassociation profile
#ifdef _TR_111_PRMT_
#ifdef _PRMT_WT107_
#define DS_TR111	",DeviceAssociation:2"
#else
#define DS_TR111	",DeviceAssociation:1"
#endif //_PRMT_WT107_
#else
#define DS_TR111	""
#endif //_TR_111_PRMT_

//dhcpcondserving profile
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define DS_DHCPCONDSERV	",DHCPCondServing:1"
#else
#define DS_DHCPCONDSERV	""
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_

//dhcpoption profile
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define DS_DHCPOPTION	",DHCPOption:1"
#else
#define DS_DHCPOPTION	""
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_

#define DS_END		")"
#define DEVICESUMMARY	DS_PREFIX DS_ETH_WAN DS_ETH DS_USB DS_WIFI DS_WIFIWMM DS_WIFIWPS \
			DS_ADSLWAN DS_ADSL2WAN DS_QOS DS_TIME DS_CAPTIVEPORTAL \
			DS_PING DS_TRACEROUTE DS_TR143 DS_ATMLB DS_DSLDIAG \
			DS_DSL2DIAG DS_TR111 DS_DHCPCONDSERV DS_DHCPOPTION DS_END
/*******end DEVICESUMMARY****************************************************************************/


/******LANConfigSecurity***************************************************************************/
struct CWMP_OP tLANConfigSecurityLeafOP = { getLANConfSec,	setLANConfSec };
struct CWMP_PRMT tLANConfigSecurityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ConfigPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANConfigSecurityLeafOP },
};
enum eLANConfigSecurityLeaf
{
	eConfigPassword,
};
struct CWMP_LEAF tLANConfigSecurityLeaf[] =
{
{ &tLANConfigSecurityLeafInfo[eConfigPassword] },
{ NULL	}
};

#ifdef _PRMT_CAPABILITIES_
/******Capabilities**********************************************************************************/
struct CWMP_PRMT tCapabilitiesObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
#ifdef _PRMT_TR143_
{"PerformanceDiagnostic",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif //_PRMT_TR143_
};
enum eCapabilitiesObject
{
#ifdef _PRMT_TR143_
	ePerformanceDiagnostic,
#endif //_PRMT_TR143_
	eCapabilities_END /*the last one*/
};
struct CWMP_NODE tCapabilitiesObject[] =
{
/*info,  						leaf,				node)*/
#ifdef _PRMT_TR143_
{&tCapabilitiesObjectInfo[ePerformanceDiagnostic],	tPerformanceDiagnosticLeaf,	NULL},
#endif //_PRMT_TR143_
{NULL,							NULL,				NULL}
};
#endif //_PRMT_CAPABILITIES_

/******IGD*****************************************************************************************/
struct CWMP_OP tIGDLeafOP = { getIGD, NULL };
struct CWMP_PRMT tIGDLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"LANDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tIGDLeafOP},
{"WANDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tIGDLeafOP},
{"DeviceSummary",		eCWMP_tSTRING,	CWMP_READ,		&tIGDLeafOP},
};
enum eIGDLeaf
{
	eLANDeviceNumberOfEntries,
	eWANDeviceNumberOfEntries,
	eDeviceSummary
};
struct CWMP_LEAF tIGDLeaf[] =
{
{ &tIGDLeafInfo[eLANDeviceNumberOfEntries]  },
{ &tIGDLeafInfo[eWANDeviceNumberOfEntries]  },
{ &tIGDLeafInfo[eDeviceSummary]  },
{ NULL	}
};
struct CWMP_PRMT tIGDObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"DeviceInfo",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _PRMT_DEVICECONFIG_
{"DeviceConfig",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_DEVICECONFIG_
{"ManagementServer",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef TIME_ZONE
{"Time",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif /*TIME_ZONE*/
#ifdef _PRMT_USERINTERFACE_
{"UserInterface",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif /*_PRMT_USERINTERFACE_*/
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
{"CaptivePortal",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
{"Layer3Forwarding",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _PRMT_X_STD_QOS_
{"QueueManagement",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#elif IP_QOS
{"QueueManagement",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
{"LANConfigSecurity",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"IPPingDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
{"TraceRouteDiagnostics",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_SUPPORT_TRACEROUTE_PROFILE_
{"LANDevice",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"LANInterfaces",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
/*ping_zhang:20081217 END*/
{"WANDevice",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _PRMT_SERVICES_
{"Services",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_SERVICES_
#ifdef _PRMT_CAPABILITIES_
{"Capabilities",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_CAPABILITIES_
#ifdef _PRMT_TR143_
{"DownloadDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"UploadDiagnostics",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"UDPEchoConfig",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_QOS_
{"X_CT-COM_UplinkQoS",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
{"X_CT-COM_UserInfo",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
};
enum eIGDObject
{
	eDeviceInfo,
#ifdef _PRMT_DEVICECONFIG_
	eDeviceConfig,
#endif //_PRMT_DEVICECONFIG_
	eManagementServer,
#ifdef TIME_ZONE
	eTime,
#endif //TIME_ZONE
#ifdef _PRMT_USERINTERFACE_
	eUserInterface,
#endif /*_PRMT_USERINTERFACE_*/
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	eCaptivePortal,
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
	eLayer3Forwarding,
#ifdef _PRMT_X_STD_QOS_
	eQueueManagement,
#elif IP_QOS
	eQueueManagement,
#endif //IP_QOS
	eLANConfigSecurity,
	eIPPingDiagnostics,
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
	eTraceRouteDiagnostics,
#endif //_SUPPORT_TRACEROUTE_PROFILE_
	eLANDevice,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eLANInterfaces,
#endif
/*ping_zhang:20081217 END*/
	eWANDevice,
#ifdef _PRMT_SERVICES_
	eServices,
#endif //_PRMT_SERVICES_
#ifdef _PRMT_CAPABILITIES_
	eCapabilities,
#endif //_PRMT_CAPABILITIES_
#ifdef _PRMT_TR143_
	eDownloadDiagnostics,
	eUploadDiagnostics,
	eUDPEchoConfig,
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_QOS_
	eX_CT_COM_QueueManagement,
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
	eX_CT_COM_UserInfo,
#endif
};
struct CWMP_NODE tIGDObject[] =
{
/*info,  				leaf,			next)*/
{&tIGDObjectInfo[eDeviceInfo],		tDeviceInfoLeaf,	tDeviceInfoObject},
#ifdef _PRMT_DEVICECONFIG_
{&tIGDObjectInfo[eDeviceConfig],	tDeviceConfigLeaf,	NULL},
#endif //_PRMT_DEVICECONFIG_
{&tIGDObjectInfo[eManagementServer],	tManagementServerLeaf,	tManagementServerObject},
#ifdef TIME_ZONE
{&tIGDObjectInfo[eTime],		tTimeLeaf,		NULL},
#endif //TIME_ZONE
#ifdef _PRMT_USERINTERFACE_
{&tIGDObjectInfo[eUserInterface],	tUserIFLeaf,		NULL},
#endif /*_PRMT_USERINTERFACE_*/
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
{&tIGDObjectInfo[eCaptivePortal],	tCaptivePortalLeaf,	NULL},
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
{&tIGDObjectInfo[eLayer3Forwarding],	tLayer3ForwardingLeaf,	tLayer3ForwardingObject},
#ifdef _PRMT_X_STD_QOS_
{&tIGDObjectInfo[eQueueManagement],	tQueueMntLeaf,		tQueueMntObject},
#elif IP_QOS
{&tIGDObjectInfo[eQueueManagement],	tQueueMntLeaf,		tQueueMntObject},
#endif //IP_QOS
{&tIGDObjectInfo[eLANConfigSecurity],	tLANConfigSecurityLeaf,	NULL},
{&tIGDObjectInfo[eIPPingDiagnostics],	tIPPingDiagnosticsLeaf,	NULL},
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
{&tIGDObjectInfo[eTraceRouteDiagnostics],tTraceRouteDiagLeaf,	tTraceRouteDiagObject},
#endif //_SUPPORT_TRACEROUTE_PROFILE_
{&tIGDObjectInfo[eLANDevice],		NULL,			tLANDeviceObject},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{&tIGDObjectInfo[eLANInterfaces],	tLANInterfacesLeaf,			NULL},
#endif
/*ping_zhang:20081217 END*/
{&tIGDObjectInfo[eWANDevice],		NULL,			tWANDeviceObject},
#ifdef _PRMT_SERVICES_
{&tIGDObjectInfo[eServices],		NULL,			tServicesObject},
#endif //_PRMT_SERVICES_
#ifdef _PRMT_CAPABILITIES_
{&tIGDObjectInfo[eCapabilities],	NULL,			tCapabilitiesObject},
#endif //_PRMT_CAPABILITIES_
#ifdef _PRMT_TR143_
{&tIGDObjectInfo[eDownloadDiagnostics],	tDownloadDiagnosticsLeaf,NULL},
{&tIGDObjectInfo[eUploadDiagnostics],	tUploadDiagnosticsLeaf,	NULL},
{&tIGDObjectInfo[eUDPEchoConfig],	tUDPEchoConfigLeaf,	NULL},
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_QOS_
{&tIGDObjectInfo[eX_CT_COM_QueueManagement],	tCT_QueueMntLeaf,		tCT_QueueMntObject},
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
{&tIGDObjectInfo[eX_CT_COM_UserInfo],	tCT_UserInfoLeaf,	NULL},
#endif
{NULL,					NULL,			NULL}
};

/******Root*****************************************************************************************/
struct CWMP_PRMT tROOTObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"InternetGatewayDevice",	eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eROOTObject
{
	eInternetGatewayDevice
};
struct CWMP_NODE tROOT[] =
{
/*info, 	 				leaf,			next*/
{&tROOTObjectInfo[eInternetGatewayDevice],	tIGDLeaf,		tIGDObject	},
{NULL,						NULL,			NULL		}
};

#ifdef CONFIG_MIDDLEWARE
int getInternetPvc(void ** data)
{
	unsigned int devnum=0;
	unsigned int pppnum=0;
	MIB_CE_ATM_VC_T Entry;
	int ret=-1;
	unsigned int i,num;
	char buf[200];
	char tmp[16];

	memset(buf,0,200);
	memset(tmp,0,16);
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)&Entry ) )
			continue;

		if(Entry.cmode != ADSL_PPPoE)
			continue;
		
		if(Entry.ServiceList & X_CT_SRV_INTERNET)
		{
			devnum = Entry.ConDevInstNum;
			pppnum = Entry.ConPPPInstNum;
			sprintf(tmp,"WAN%d.PPP%d",devnum,pppnum);
			if(ret == 0){//has found a internet pppoe pvc again
				strcat(buf,"$");
			}
			strcat(buf,tmp);
			ret = 0;
		}
	}	
	
	if(ret == 0){
		*data = strdup(buf);
	}else{
		*data = strdup("NULL");
	}

	return 0;	
}

int getMgtDNS(void ** data)
{
	char buf[100];
	FILE * fp;
#ifdef DNS_BIND_PVC_SUPPORT
	unsigned char DnsBindPvcEnable;
	unsigned int DnsBindPvc;
	struct in_addr dnsIP;
	int num,i=0;
	MIB_CE_ATM_VC_T Entry;
	unsigned int mgtIfindex=0;
#endif
	memset(buf,0,sizeof(buf));

	/*search mgtdns is file*/
	fp = fopen(MGT_DNS_FILE,"r");
	if(fp){
		while(fgets(buf,sizeof(buf),fp)){
			*data = strdup(buf);
			return 0;
		}
	}

	/*search DNS Banding*/
#ifdef DNS_BIND_PVC_SUPPORT
	mib_get(MIB_DNS_BIND_PVC_ENABLE,(void *)&DnsBindPvcEnable);
	if(DnsBindPvcEnable)
	{
		num = mib_chain_total( MIB_ATM_VC_TBL );
		for( i=0; i<num;i++ )
		{
			if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)&Entry ) )
				continue;

			if(Entry.ServiceList & X_CT_SRV_TR069)
			{
				mgtIfindex = Entry.ifIndex;
				break;
			}
		}	
		if(mgtIfindex == 0){
			goto notFind;
		}

		for(i=0;i<3;i++)
		{
			mib_get(MIB_DNS_BIND_PVC1+i,&DnsBindPvc);
			if(DnsBindPvc != mgtIfindex)
				continue;
		
			mib_get(MIB_ADSL_WAN_DNS1+i, (void *)&dnsIP);
			if(dnsIP.s_addr){
				strcpy(buf,inet_ntoa(dnsIP));
				*data = strdup(buf);
				return 0;
			}
		}
	}
#endif

notFind:
	*data = strdup("0.0.0.0");
	return 0;	
}

int getCTMgtIPAddress(void ** data)
{
	unsigned int devnum=0;
	unsigned int pppnum=0;
	MIB_CE_ATM_VC_T Entry,*pEntry;
	int ret=-1;
	unsigned int i,num;

	pEntry = &Entry;
	
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode == ADSL_BR1483){
			continue;
		}

		if(pEntry->ServiceList & X_CT_SRV_TR069)
		{
			ret = 0;
			break;
		}
	}	
	if(ret == 0){
		char ifname[16];
		char *temp=NULL;	
		struct in_addr inAddr;
		
		if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) 
			return ERR_9002;
		if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			temp = inet_ntoa(inAddr);
		if(temp)
			*data=strdup(temp);
		else
			*data=strdup("0.0.0.0");
	}else{
		*data = strdup("0.0.0.0");
	}

	return 0;	
}

int getCTUserIPAddress(void ** data,int ipIndex)
{
	unsigned int devnum=0;
	unsigned int pppnum=0;
	MIB_CE_ATM_VC_T Entry,*pEntry;
	int ret=-1;
	unsigned int i,num,InvalidNum=0;

	pEntry = &Entry;
	
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode == ADSL_BR1483){
			continue;
		}
		
		if(pEntry->ServiceList & X_CT_SRV_INTERNET)
		{
			InvalidNum++;	//find the InvalidNumth Internet wandevice
			if(ipIndex == InvalidNum){
				char ifname[16];
				char *temp=NULL;	
				struct in_addr inAddr;
		
				if(ifGetName( pEntry->ifIndex, ifname, 16 )==0) 
					return ERR_9002;
				if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
					temp = inet_ntoa(inAddr);
				if(temp)
					*data=strdup(temp);
				else
					*data=strdup("0.0.0.0");

				return 0;
			}
		}
	}

	*data = strdup("0.0.0.0");
	return 0;	
}

int getAppendInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "InternetPvc" )==0 )
	{
		return(getInternetPvc(data));
	}else if( strcmp( lastname, "MgtDNS" )==0 )
	{
		return(getMgtDNS(data));
	}else if( strcmp( lastname, "CTMgtIPAddress" )==0 )
	{
		return(getCTMgtIPAddress(data));
	}else if( strcmp( lastname, "CTUserIPAddress1" )==0 )
	{
		return(getCTUserIPAddress(data,1));
	}else if( strcmp( lastname, "CTUserIPAddress2" )==0 )
	{
		return(getCTUserIPAddress(data,2));
	}else if( strcmp( lastname, "CTUserIPAddress3" )==0 )
	{
		return(getCTUserIPAddress(data,3));
	}else if( strcmp( lastname, "CTUserIPAddress4" )==0 )
	{
		return(getCTUserIPAddress(data,4));
	}else if( strcmp( lastname, "CTUserIPAddress5" )==0 )
	{
		return(getCTUserIPAddress(data,5));
	}else if( strcmp( lastname, "CTUserIPAddress6" )==0 )
	{
		return(getCTUserIPAddress(data,6));
	}else if( strcmp( lastname, "CTUserIPAddress7" )==0 )
	{
		return(getCTUserIPAddress(data,7));
	}else if( strcmp( lastname, "CTUserIPAddress8" )==0 )
	{
		return(getCTUserIPAddress(data,8));
	}else{
		return ERR_9005;
	}
	
	return 0;
}

struct CWMP_OP tAppendInfoOP = { getAppendInfo, NULL };

struct CWMP_PRMT tAppendLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InternetPvc",	eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"MgtDNS",	eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTMgtIPAddress",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress1",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress2",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress3",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress4",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress5",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress6",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress7",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
{"CTUserIPAddress8",		eCWMP_tSTRING,	CWMP_READ,		&tAppendInfoOP},
};

enum eAppendLeaf
{
	eInternetPvc,
	eMgtDNS,
	eCTMgtIPAddress,
	eCTUserIPAddress1,
	eCTUserIPAddress2,
	eCTUserIPAddress3,
	eCTUserIPAddress4,
	eCTUserIPAddress5,
	eCTUserIPAddress6,
	eCTUserIPAddress7,
	eCTUserIPAddress8
};

struct CWMP_LEAF tAppendLeaf[] =
{
{ &tAppendLeafInfo[eInternetPvc]  },
{ &tAppendLeafInfo[eMgtDNS]  },
{ &tAppendLeafInfo[eCTMgtIPAddress]  },
{ &tAppendLeafInfo[eCTUserIPAddress1]  },
{ &tAppendLeafInfo[eCTUserIPAddress2]  },
{ &tAppendLeafInfo[eCTUserIPAddress3]  },
{ &tAppendLeafInfo[eCTUserIPAddress4]  },
{ &tAppendLeafInfo[eCTUserIPAddress5]  },
{ &tAppendLeafInfo[eCTUserIPAddress6]  },
{ &tAppendLeafInfo[eCTUserIPAddress7]  },
{ &tAppendLeafInfo[eCTUserIPAddress8]  },
{ NULL	}
};

struct CWMP_PRMT tROOTAppendInfo[] =
{
/*(name,			type,		flag,		op)*/
{"AppendInfo",	eCWMP_tOBJECT,	CWMP_READ,	NULL}
};

enum eROOTAppendInfo
{
	eAppendInfo
};

struct CWMP_NODE mw_tROOT[] =	/*add some parameters in AppendInfo for midware*/
{
/*info, 	 				leaf,			next*/
{&tROOTObjectInfo[eInternetGatewayDevice],	tIGDLeaf,		tIGDObject	},
{&tROOTAppendInfo[eAppendInfo],	tAppendLeaf,		NULL	},
{NULL,						NULL,			NULL		}
};
#endif
/***********************************************************************************************/

int getIGD(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LANDeviceNumberOfEntries" )==0 )
	{
		*data = uintdup( LANDEVICE_NUM ); 
	}else if( strcmp( lastname, "WANDeviceNumberOfEntries" )==0 )
	{
		*data = uintdup( WANDEVICE_NUM ); 
	}else if( strcmp( lastname, "DeviceSummary" )==0 )
	{
		*data = strdup( DEVICESUMMARY );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getLANConfSec(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ConfigPassword" )==0 )
	{
#if 0
#if 1
		*data = strdup( "" ); /*return an empty string*/
#else
		char buf[65];
	#ifdef TELEFONICA_DEFAULT_CFG
		mib_get(MIB_SUSER_PASSWORD, (void *)buf);
	#else
		mib_get(CWMP_LAN_CONFIGPASSWD, (void *)buf);
	#endif //TELEFONICA_DEFAULT_CFG
		*data = strdup( buf );
#endif
#else
		char buf[100];
		mib_get(MIB_USER_PASSWORD, (void *)buf);
		*data = strdup(buf);
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setLANConfSec(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "ConfigPassword" )==0 )
	{
#ifdef TELEFONICA_DEFAULT_CFG
		aaaa;
		char *buf=data;
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);
	#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	#else
		return 1;
	#endif //_CWMP_APPLY_
#else
		//bbbb;
		char *buf=data;
		int  len=0;
		if( buf ) len = strlen( buf );
		if( len == 0 )
			mib_set( MIB_CWMP_LAN_CONFIGPASSWD, (void *)"");
		else if( len < 64 )
			mib_set( MIB_CWMP_LAN_CONFIGPASSWD, (void *)buf);
		else
			return ERR_9007;

		return 0;
#endif //TELEFONICA_DEFAULT_CFG
	}else{
		return ERR_9005;
	}
	
	return 0;

}

#ifdef _PRMT_X_CT_COM_DATATYPE
int changestring2int(void* dataaddr, int righttype, int wrongtype, int* tmpint, unsigned int* tmpuint, int* tmpbool)
{
	char* buf=dataaddr;

	if(righttype==wrongtype){
		switch(righttype){
			case eCWMP_tUINT:
				*tmpuint = *(unsigned int*)dataaddr;
				break;
			case eCWMP_tINT:
				*tmpint = *(int*)dataaddr;
				break;
			case eCWMP_tBOOLEAN:
				*tmpbool = *(int*)dataaddr;
				break;
			default:
				return 1;
		}
		return 1;
	}

	if(wrongtype==eCWMP_tSTRING){
		//printf("\nstring=%s\n",buf);
		switch(righttype){
			case eCWMP_tUINT:
				sscanf(buf,"%u",tmpuint);
				break;
			case eCWMP_tINT:
				sscanf(buf,"%d",tmpint);
				break;
			case eCWMP_tBOOLEAN:
				sscanf(buf,"%d",tmpbool);
				break;
			default:
				return -1;
		}

		return 1;

	}else
		return -1;
}
#endif

