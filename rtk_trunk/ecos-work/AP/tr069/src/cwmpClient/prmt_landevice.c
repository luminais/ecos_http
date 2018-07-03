#include <sys/stat.h>
#include <signal.h>
#include "prmt_landevice.h"
#include "prmt_landevice_eth.h"
//#include <config/autoconf.h> keith remove
#ifdef WLAN_SUPPORT
#include "prmt_landevice_wlan.h"
#else
#define WLAN_IF_NUM	0
#endif
#if defined(_PRMT_USB_ETH_) || defined(_PRMT_USB_)
#include "prmt_landevice_usb.h"
#else
#define CWMP_LANUSBIFNO		0
#define CWMP_ID						700 
#define MIB_CWMP_LAN_IPIFENABLE					CWMP_ID + 14
#endif

#if 0
//extern int getMIBtoStr(unsigned int id, char *strbuf);
#define CWMP_ID                                         700
#define MIB_CWMP_SERIALNUMBER                                   CWMP_ID + 12
#define MIB_CWMP_ACS_PARAMETERKEY                               CWMP_ID + 24
int getMIBtoStr(unsigned int id, char *strbuf)
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
#endif


//#define _CHECK_DHCP_PREFIX_
#ifdef _CHECK_DHCP_PREFIX_
#define CheckDhcpPrefix(ip1,ip2,ip3,ip4) \
		do{\
			char lan_ip[32];\
			int  lanip1,lanip2, lanip3;\
			/*getMIB2Str(MIB_ADSL_LAN_DHCP_GATEWAY, lan_ip);*/\
			/*sscanf( lan_ip, "%d.%d.%d.%*d", &lanip1, &lanip2, &lanip3 );*/\
			getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, lan_ip);\
			sscanf(lan_ip, "%d.%d.%d.", &lanip1, &lanip2, &lanip3 );\
			if( (lanip1!=ip1) || (lanip2!=ip2) || (lanip3!=ip3)  )\
				return ERR_9007;\
		}while(0)

#else
#define CheckDhcpPrefix(ip1,ip2,ip3,ip4) do{}while(0)
#endif //_CHECK_DHCP_PREFIX_

#define GetDhcpPrefix(ipadd1,ipadd2,ipadd3,ipadd4) \
		do{\
			/*getMIB2Str(MIB_ADSL_LAN_DHCP_GATEWAY, buf);*/\
			/*sscanf( buf,"%d.%d.%d.%d", &ipadd1, &ipadd2, &ipadd3, &ipadd4 );*/\
			getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, buf);\
			sscanf( buf,"%d.%d.%d.", &ipadd1, &ipadd2, &ipadd3 );\
		}while(0);



#define	DHCPUPDATETIME		90
char		*gDHCPHosts=NULL;
time_t		gDHCPUpdateTime=0;
unsigned int	gDHCPTotalHosts=0;
struct CWMP_LINKNODE **gDHCPTable=NULL;

int updateDHCP(void);
int getDHCPClient( int id,  char *ip, char *mac, int *liveTime );

#ifdef _CWMP_MAC_FILTER_
int getMACAddressList( char *buf, int len );
int setMACAddressList( char *buf );
#endif /*_CWMP_MAC_FILTER_*/

#ifdef SECONDARY_IP
unsigned int getIPItfInstNum( char *name );
#endif //SECONDARY_IP

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
struct CWMP_OP tLANInterfacesLeafOP = { getLANInterfaces, NULL };
struct CWMP_PRMT tLANInterfacesLeafInfo[] = 
{
/*(name,				type,		flag,			op)*/
{"LANEthernetInterfaceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANInterfacesLeafOP},
{"LANUSBInterfaceNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tLANInterfacesLeafOP},
{"LANWLANConfigurationNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANInterfacesLeafOP}
};
enum eLANInterfacesLeaf
{
	eLANItfEthernetInterfaceNumberOfEntries,
	eLANItfUSBInterfaceNumberOfEntries,
	eLANItfWLANConfigurationNumberOfEntries
};
struct CWMP_LEAF tLANInterfacesLeaf[] =
{
{ &tLANInterfacesLeafInfo[eLANItfEthernetInterfaceNumberOfEntries] },
{ &tLANInterfacesLeafInfo[eLANItfUSBInterfaceNumberOfEntries] },
{ &tLANInterfacesLeafInfo[eLANItfWLANConfigurationNumberOfEntries] },
{ NULL }
};
#endif
/*ping_zhang:20081217 END*/

struct CWMP_OP tHostEntityLeafOP = { getHostEntity, NULL };
struct CWMP_PRMT tHostEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"IPAddress",			eCWMP_tSTRING,	CWMP_READ,		&tHostEntityLeafOP},
{"AddressSource",		eCWMP_tSTRING,	CWMP_READ,		&tHostEntityLeafOP},
{"LeaseTimeRemaining",		eCWMP_tINT,	CWMP_READ|CWMP_DENY_ACT,&tHostEntityLeafOP},
{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tHostEntityLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Layer2Interface",			eCWMP_tSTRING,	CWMP_READ,		&tHostEntityLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"HostName",			eCWMP_tSTRING,	CWMP_READ,		&tHostEntityLeafOP},
{"InterfaceType",		eCWMP_tSTRING,	CWMP_READ,		&tHostEntityLeafOP},
{"Active",			eCWMP_tBOOLEAN,	CWMP_READ,		&tHostEntityLeafOP}
};
enum eHostEntityLeaf
{
	eHost_IPAddress,
	eHost_AddressSource,	
	eHost_LeaseTimeRemaining,
	eHost_MACAddress,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eHost_Layer2Interface,
#endif
/*ping_zhang:20081217 END*/	
	eHost_HostName,
	eHost_InterfaceType,
	eHost_Active
};
struct CWMP_LEAF tHostEntityLeaf[] =
{
{ &tHostEntityLeafInfo[eHost_IPAddress] },
{ &tHostEntityLeafInfo[eHost_AddressSource] },
{ &tHostEntityLeafInfo[eHost_LeaseTimeRemaining] },
{ &tHostEntityLeafInfo[eHost_MACAddress] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tHostEntityLeafInfo[eHost_Layer2Interface] },
#endif
/*ping_zhang:20081217 END*/
{ &tHostEntityLeafInfo[eHost_HostName] },
{ &tHostEntityLeafInfo[eHost_InterfaceType] },
{ &tHostEntityLeafInfo[eHost_Active] },
{ NULL }
};




struct CWMP_PRMT tHostObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eHostObject
{
	eHost0
};
struct CWMP_LINKNODE tHostObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tHostObjectInfo[eHost0],	tHostEntityLeaf,	NULL,		NULL,			0},
};


struct CWMP_OP tHostsLeafOP =  { getHosts, NULL};
struct CWMP_PRMT tHostsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"HostNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tHostsLeafOP}
};
enum eHostsLeaf
{
	eHostsNumberOfEntries
};
struct CWMP_LEAF tHostsLeaf[] =
{
{ &tHostsLeafInfo[eHostsNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tHosts_Host_OP = { NULL, objHosts };
struct CWMP_PRMT tHostsObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Host",			eCWMP_tOBJECT,	CWMP_READ,		&tHosts_Host_OP}
};
enum eHostsObject
{
	eHostsHost
};
struct CWMP_NODE tHostsObject[] =
{
/*info,  			leaf,			node)*/
{&tHostsObjectInfo[eHostsHost],	NULL,			NULL},
{NULL,				NULL,			NULL}
};

struct CWMP_OP tIPInterfaceEntityLeafOP = { getIPItfEntity, setIPItfEntity };
struct CWMP_PRMT tIPInterfaceEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tIPInterfaceEntityLeafOP},
{"IPInterfaceIPAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tIPInterfaceEntityLeafOP},
{"IPInterfaceSubnetMask",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tIPInterfaceEntityLeafOP},
{"IPInterfaceAddressingType",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tIPInterfaceEntityLeafOP}
};
enum eIPInterfaceEntityLeaf
{
	eIPIF_Enable,
	eIPIF_IPInterfaceIPAddress,
	eIPIF_IPInterfaceSubnetMask,
	eIPIF_IPInterfaceAddressingType
};
struct CWMP_LEAF tIPInterfaceEntityLeaf[] =
{
{ &tIPInterfaceEntityLeafInfo[eIPIF_Enable] },
{ &tIPInterfaceEntityLeafInfo[eIPIF_IPInterfaceIPAddress] },
{ &tIPInterfaceEntityLeafInfo[eIPIF_IPInterfaceSubnetMask] },
{ &tIPInterfaceEntityLeafInfo[eIPIF_IPInterfaceAddressingType] },
{ NULL }
};
struct CWMP_PRMT tIPInterfaceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef SECONDARY_IP
{"2",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif //SECONDARY_IP

};
enum eIPInterfaceObject
{
	eIPInterface1,
#ifdef SECONDARY_IP
	eIPInterface2,
#endif //SECONDARY_IP
};


struct CWMP_NODE tIPInterfaceObject[] =
{
/*info,  					leaf,				node)*/
{ &tIPInterfaceObjectInfo[eIPInterface1],	tIPInterfaceEntityLeaf,		NULL},
#ifdef SECONDARY_IP
{ &tIPInterfaceObjectInfo[eIPInterface2],	tIPInterfaceEntityLeaf,		NULL},
#endif //SECONDARY_IP
{ NULL,						NULL,				NULL}
};

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.DHCPOption.{i}.*/
/*InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.DHCPOption.{i}.*/
struct CWMP_OP tDHCPOptionEntityLeafOP = { getDHCPOptionEntity, setDHCPOptionEntity };
struct CWMP_PRMT tDHCPOptionLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDHCPOptionEntityLeafOP},
{"Tag",			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tDHCPOptionEntityLeafOP},
{"Value",			eCWMP_tBASE64,	CWMP_WRITE|CWMP_READ,	&tDHCPOptionEntityLeafOP}
};
enum eDHCPOptionEntityLeaf
{
	eDHCPOptionEnable,
	eDHCPOptionTag,
	eDHCPOptionValue
};
struct CWMP_LEAF tDHCPOptionENTITYLeaf[] =
{
{ &tDHCPOptionLeafInfo[eDHCPOptionEnable] },
{ &tDHCPOptionLeafInfo[eDHCPOptionTag] },
{ &tDHCPOptionLeafInfo[eDHCPOptionValue] },
{ NULL }
};

struct CWMP_PRMT tDHCPOptionObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,		NULL}
};
enum eDHCPOptionObject
{
	eDHCPOption0
};
struct CWMP_LINKNODE tDHCPOptionObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tDHCPOptionObjectInfo[eDHCPOption0],	tDHCPOptionENTITYLeaf,	NULL,		NULL,			0}
};

/*InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.*/
struct CWMP_OP tDHCPConSPENTITYLeafOP = { getDHCPConSPEntity, setDHCPConSPEntity };
struct CWMP_PRMT tDHCPConSPENTITYLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"PoolOrder",		eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"SourceInterface",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"VendorClassID",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"VendorClassIDExclude",		eCWMP_tBOOLEAN,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"VendorClassIDMode",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"ClientID",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"ClientIDExclude",		eCWMP_tBOOLEAN,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"UserClassID",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"UserClassIDExclude",		eCWMP_tBOOLEAN,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"Chaddr",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"ChaddrMask",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"ChaddrExclude",		eCWMP_tBOOLEAN,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"LocallyServed",		eCWMP_tBOOLEAN,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"MinAddress",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"MaxAddress",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"ReservedAddresses",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"SubnetMask",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"DNSServers",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"DomainName",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"IPRouters",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"DHCPLeaseTime",		eCWMP_tINT,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"UseAllocatedWAN",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"AssociatedConnection",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
{"DHCPServerIPAddress",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE,	&tDHCPConSPENTITYLeafOP},
//{"DHCPStaticAddressNumberOfEntries",		eCWMP_tUINT,		CWMP_READ,	&tDHCPConSPENTITYLeafOP},
{"DHCPOptionNumberOfEntries",		eCWMP_tUINT,		CWMP_READ,	&tDHCPConSPENTITYLeafOP},
};
enum eDHCPConSPDHCPOptionEntityLeaf
{
	eDHCPConSPEnable,
	eDHCPConSPPoolOrder,
	eDHCPConSPSourceInterface,
	eDHCPConSPVendorClassID,
	eDHCPConSPVendorClassIDExclude,
	eDHCPConSPVendorClassIDMode,
	eDHCPConSPClientID,
	eDHCPConSPClientIDExclude,
	eDHCPConSPUserClassID,
	eDHCPConSPUserClassIDExclude,
	eDHCPConSPChaddr,
	eDHCPConSPChaddrMask,
	eDHCPConSPChaddrExclude,
	eDHCPConSPLocallyServed,
	eDHCPConSPMinAddress,
	eDHCPConSPMaxAddress,
	eDHCPConSPReservedAddresses,
	eDHCPConSPSubnetMask,
	eDHCPConSPDNSServers,
	eDHCPConSPDomainName,
	eDHCPConSPIPRouters,
	eDHCPConSPDHCPLeaseTime,
	eDHCPConSPUseAllocatedWAN,
	eDHCPConSPAssociatedConnection,
	eDHCPConSPDHCPServerIPAddress,
//	eDHCPConSPDHCPStaticAddressNumberOfEntries,
	eDHCPConSPDHCPOptionNumberOfEntries,
};
struct CWMP_LEAF tDHCPConSPENTITYLeaf[] =
{
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPEnable] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPPoolOrder] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPSourceInterface] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPVendorClassID] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPVendorClassIDExclude] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPVendorClassIDMode] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPClientID] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPClientIDExclude] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPUserClassID] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPUserClassIDExclude] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPChaddr] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPChaddrMask] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPChaddrExclude] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPLocallyServed] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPMinAddress] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPMaxAddress] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPReservedAddresses] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPSubnetMask] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPDNSServers] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPDomainName] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPIPRouters] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPDHCPLeaseTime] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPUseAllocatedWAN] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPAssociatedConnection] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPDHCPServerIPAddress] },
//{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPDHCPStaticAddressNumberOfEntries] },
{ &tDHCPConSPENTITYLeafInfo[eDHCPConSPDHCPOptionNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tDHCPConSPDHCPOption_OP = { NULL, objDHCPOption};
struct CWMP_PRMT tDHCPConSPENTITYObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"DHCPOption",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,		&tDHCPConSPDHCPOption_OP},
};
enum eDHCPConSPENTITYObject
{
	etDHCPConSPE_DHCPOption
};
struct CWMP_NODE tDHCPConSPENTITYObject[] =
{
/*info,  							leaf,			node)*/
{&tDHCPConSPENTITYObjectInfo[etDHCPConSPE_DHCPOption],	NULL,	NULL},
{NULL,								NULL,			NULL}
};

struct CWMP_PRMT tDHCPConSPObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,		NULL}
};
enum eDHCPConSPObject
{
	eDHCPConSP0
};
struct CWMP_LINKNODE tDHCPConSPObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tDHCPConSPObjectInfo[eDHCPConSP0],	tDHCPConSPENTITYLeaf,	tDHCPConSPENTITYObject,		NULL,			0}
};
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/

struct CWMP_OP tLANHostConfLeafOP = { getLANHostConf, setLANHostConf };
struct CWMP_PRMT tLANHostConfLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tLANHostConfLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"DHCPServerConfigurable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"DHCPServerEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"DHCPRelay",			eCWMP_tBOOLEAN,	CWMP_READ,		&tLANHostConfLeafOP},
{"MinAddress",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"MaxAddress",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
#ifdef _PRMT_X_CT_COM_DHCP_
{"X_CT-COM_STB-MinAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_STB-MaxAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_Phone-MinAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_Phone-MaxAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_Camera-MinAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_Camera-MaxAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_Computer-MinAddress",eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_Computer-MaxAddress",eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
#ifndef CONFIG_BOA_WEB_E8B_CH
{"X_CT-COM_HGW-MinAddress",eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"X_CT-COM_HGW-MaxAddress",eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
#endif
#endif
{"ReservedAddresses",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"SubnetMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"DNSServers",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"DomainName",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"IPRouters",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"DHCPLeaseTime",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"UseAllocatedWAN",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"AssociatedConnection",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"PassthroughLease",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
{"PassthroughMACAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
#ifdef _CWMP_MAC_FILTER_
{"AllowedMACAddresses",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANHostConfLeafOP},
#endif /*_CWMP_MAC_FILTER_*/
{"IPInterfaceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANHostConfLeafOP},
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{"DHCPOptionNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANHostConfLeafOP},
{"DHCPConditionalPoolNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANHostConfLeafOP},
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
};
enum eLANHostConfLeaf
{
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eMACAddress,
#endif
/*ping_zhang:20081217 END*/
	eDHCPServerConfigurable,
	eDHCPServerEnable,
	eDHCPRelay,
	eMinAddress,
	eMaxAddress,
#ifdef _PRMT_X_CT_COM_DHCP_
	eSTB_MinAddress,
	eSTB_MaxAddress,
	ePhone_MinAddress,
	ePhone_MaxAddress,
	eCamera_MinAddress,
	eCamera_MaxAddress,
	eComputer_MinAddress,
	eComputer_MaxAddress,
#ifndef CONFIG_BOA_WEB_E8B_CH
	eHGW_MinAddress,
	eHGW_MaxAddress,
#endif
#endif
	eReservedAddresses,
	eSubnetMask,
	eDNSServers,
	eDomainName,
	eIPRouters,
	eDHCPLeaseTime,
	eUseAllocatedWAN,
	eAssociatedConnection,
	ePassthroughLease,
	ePassthroughMACAddress,
#ifdef _CWMP_MAC_FILTER_
	eAllowedMACAddresses,
#endif /*_CWMP_MAC_FILTER_*/
	eIPInterfaceNumberOfEntries,
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	eDHCPOptionNumberOfEntries,
	eDHCPConditionalPoolNumberOfEntries
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
};
struct CWMP_LEAF tLANHostConfLeaf[] =
{
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tLANHostConfLeafInfo[eMACAddress] },
#endif
/*ping_zhang:20081217 END*/
{ &tLANHostConfLeafInfo[eDHCPServerConfigurable] },
{ &tLANHostConfLeafInfo[eDHCPServerEnable] },
{ &tLANHostConfLeafInfo[eDHCPRelay] },
{ &tLANHostConfLeafInfo[eMinAddress] },
{ &tLANHostConfLeafInfo[eMaxAddress] },
#ifdef _PRMT_X_CT_COM_DHCP_
{ &tLANHostConfLeafInfo[eSTB_MinAddress] },
{ &tLANHostConfLeafInfo[eSTB_MaxAddress] },
{ &tLANHostConfLeafInfo[ePhone_MinAddress] },
{ &tLANHostConfLeafInfo[ePhone_MaxAddress] },
{ &tLANHostConfLeafInfo[eCamera_MinAddress] },
{ &tLANHostConfLeafInfo[eCamera_MaxAddress] },
{ &tLANHostConfLeafInfo[eComputer_MinAddress] },
{ &tLANHostConfLeafInfo[eComputer_MaxAddress] },
#ifndef CONFIG_BOA_WEB_E8B_CH
{ &tLANHostConfLeafInfo[eHGW_MinAddress] },
{ &tLANHostConfLeafInfo[eHGW_MaxAddress] },
#endif
#endif
{ &tLANHostConfLeafInfo[eReservedAddresses] },
{ &tLANHostConfLeafInfo[eSubnetMask] },
{ &tLANHostConfLeafInfo[eDNSServers] },
{ &tLANHostConfLeafInfo[eDomainName] },
{ &tLANHostConfLeafInfo[eIPRouters] },
{ &tLANHostConfLeafInfo[eDHCPLeaseTime] },
{ &tLANHostConfLeafInfo[eUseAllocatedWAN] },
{ &tLANHostConfLeafInfo[eAssociatedConnection] },
{ &tLANHostConfLeafInfo[ePassthroughLease] },
{ &tLANHostConfLeafInfo[ePassthroughMACAddress] },
#ifdef _CWMP_MAC_FILTER_
{ &tLANHostConfLeafInfo[eAllowedMACAddresses] },
#endif /*_CWMP_MAC_FILTER_*/
{ &tLANHostConfLeafInfo[eIPInterfaceNumberOfEntries] },
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{ &tLANHostConfLeafInfo[eDHCPOptionNumberOfEntries] },
{ &tLANHostConfLeafInfo[eDHCPConditionalPoolNumberOfEntries] },
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
{ NULL }
};

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
struct CWMP_OP tDHCPOption_OP = { NULL, objDHCPOption};
struct CWMP_OP tDHCPConSP_OP = { NULL, objDHCPConSP};
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
struct CWMP_PRMT tLANHostConfObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"IPInterface",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{"DHCPOption",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,		&tDHCPOption_OP},
{"DHCPConditionalServingPool",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,		&tDHCPConSP_OP},
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
};
enum eLANHostConfObject
{
	eIPInterface,
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	eDHCPOption,
	eDHCPConditionServingPool
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
};
struct CWMP_NODE tLANHostConfObject[] =
{
/*info,  					leaf,		node)*/
{ &tLANHostConfObjectInfo[eIPInterface],	NULL,		tIPInterfaceObject},
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{ &tLANHostConfObjectInfo[eDHCPOption],	NULL,		NULL},
{ &tLANHostConfObjectInfo[eDHCPConditionServingPool],	NULL,		NULL},
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
{ NULL,						NULL,		NULL}
};

#if 1//def CTCOM_WLAN_REQ
struct CWMP_OP tWLANConfiguration_OP = { NULL, objWLANConfiguration };
#endif

struct CWMP_OP tLANDeviceEntityLeafOP = { getLDEntity,	NULL };
struct CWMP_PRMT tLANDeviceEntityLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"LANEthernetInterfaceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANDeviceEntityLeafOP},
{"LANUSBInterfaceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANDeviceEntityLeafOP},
{"LANWLANConfigurationNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLANDeviceEntityLeafOP}
};
enum eLANDeviceEntityLeaf
{
	eLANEthernetInterfaceNumberOfEntries,
	eLANUSBInterfaceNumberOfEntries,
	eLANWLANConfigurationNumberOfEntries
};
struct CWMP_LEAF tLANDeviceEntityLeaf[] =
{
{ &tLANDeviceEntityLeafInfo[eLANEthernetInterfaceNumberOfEntries] },
{ &tLANDeviceEntityLeafInfo[eLANUSBInterfaceNumberOfEntries] },
{ &tLANDeviceEntityLeafInfo[eLANWLANConfigurationNumberOfEntries] },
{ NULL }
};

struct CWMP_PRMT tLANDeviceEntityObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"LANHostConfigManagement",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"LANEthernetInterfaceConfig",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#if defined(_PRMT_USB_ETH_) || defined(_PRMT_USB_)
{"LANUSBInterfaceConfig",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef WLAN_SUPPORT
#if 1//def CTCOM_WLAN_REQ
{"WLANConfiguration",			eCWMP_tOBJECT,		CWMP_WRITE|CWMP_READ,	&tWLANConfiguration_OP},
#else
{"WLANConfiguration",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#endif
{"Hosts",				eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eLANDeviceEntityObject
{
	eLANHostConfigManagement,
	eLANEthernetInterfaceConfig,
#if defined(_PRMT_USB_ETH_) || defined(_PRMT_USB_)
	eLANUSBInterfaceConfig,
#endif
#ifdef WLAN_SUPPORT
	eWLANConfiguration,
#endif
	eLANHosts
};
struct CWMP_NODE tLANDeviceEntityObject[] =
{
/*info,  							leaf,			node)*/
{ &tLANDeviceEntityObjectInfo[eLANHostConfigManagement],	tLANHostConfLeaf,	tLANHostConfObject},
{ &tLANDeviceEntityObjectInfo[eLANEthernetInterfaceConfig],	NULL,			tLANEthConfObject},
#if defined(_PRMT_USB_ETH_) || defined(_PRMT_USB_)
{ &tLANDeviceEntityObjectInfo[eLANUSBInterfaceConfig],	NULL,			tLANUSBConfObject},
#endif
#ifdef WLAN_SUPPORT
#if 1//def CTCOM_WLAN_REQ
{ &tLANDeviceEntityObjectInfo[eWLANConfiguration], 		NULL,			NULL},
#else
{ &tLANDeviceEntityObjectInfo[eWLANConfiguration], 		NULL,			tWLANConfigObject},
#endif
#endif
{ &tLANDeviceEntityObjectInfo[eLANHosts],			tHostsLeaf,		tHostsObject},
{ NULL,								NULL,			NULL}
};


struct CWMP_PRMT tLANDeviceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
};
enum eLANDeviceObject
{
	eLANDev1
};
struct CWMP_NODE tLANDeviceObject[] =
{
/*info,  				leaf,			node)*/
{&tLANDeviceObjectInfo[eLANDev1],	tLANDeviceEntityLeaf,	tLANDeviceEntityObject},
{NULL,					NULL,			NULL}
};

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
int a =ddvv;
int getLANInterfaces(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LANEthernetInterfaceNumberOfEntries" )==0 )
	{
	#if 1
		*data = uintdup( 0 );
	#else
		int flags=0;
		if( getInFlags("eth0", &flags)==1 )
		{
			if (flags & IFF_UP)
				*data = uintdup( 0 );
			else
				*data = uintdup(CWMP_LANETHIFNO);
		}else
			return ERR_9002;
	#endif
	}else if( strcmp( lastname, "LANUSBInterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup(0);
	}else if( strcmp( lastname, "LANWLANConfigurationNumberOfEntries" )==0 )
	{
	#if 1
		*data = uintdup( 0 );
	#else
	   #ifdef WLAN_SUPPORT
		int flags=0,i;
		unsigned int count = WLAN_IF_NUM;
		for(i=0; i<WLAN_IF_NUM; i++)
		{
			if( getInFlags(wlan_name[i], &flags)==1 )
			{
				if (flags & IFF_UP)
					count --;
			}else
                                continue;
				//return ERR_9002;
		}
		*data = uintdup(count);
	   #else
		*data = uintdup(WLAN_IF_NUM);
	   #endif
	#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif
/*ping_zhang:20081217 END*/

int getHostEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	*tok=NULL;
	char	ipAddr[40], macAddr[40];
	int	livetime=-1;
	int	id;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	updateDHCP();
	//the name should be like "xxxxx.Hosts.Host.{i}.XXXXX"
	tok = strstr( name, "Host." );
	if( tok == NULL ) return ERR_9005;
	sscanf( tok, "Host.%d.%*s", &id );
	
	CWMPDBG( 2, ( stderr, "<%s:%d>id=%d\n", __FUNCTION__, __LINE__,id ) );
	if( (id<1) || (id>gDHCPTotalHosts) ) return ERR_9005;
	getDHCPClient( id-1, ipAddr, macAddr, &livetime );
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "IPAddress" )==0 )
	{
		*data = strdup( ipAddr );
	}else if( strcmp( lastname, "AddressSource" )==0 )
	{
		if( findDHCPStaticAssign( macAddr, ipAddr )>0 )
			*data = strdup( "Static" );
		else
			*data = strdup( "DHCP" );
	}else if( strcmp( lastname, "LeaseTimeRemaining" )==0 )
	{
		//the value must be 0 if the AddressSource is not DHCP
		if( findDHCPStaticAssign( macAddr, ipAddr )>0 )
			*data = intdup( 0 );
		else
			*data = intdup( livetime );
	}else if( strcmp( lastname, "MACAddress" )==0 )
	{
		*data = strdup( macAddr );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Layer2Interface" )==0 )
	{
		/*currently we don't known wich interface the host connect with, so just return eth0_sw0.*/
		*data = strdup( "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1." );
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "HostName" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "InterfaceType" )==0 )
	{
		*data = strdup( "Ethernet" ); //or 802.11
	}else if( strcmp( lastname, "Active" )==0 )
	{
		*data = booldup( 1 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int objHosts(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	CWMPDBG( 2, ( stderr, "<%s:%d>type=%d\n", __FUNCTION__, __LINE__,type ) );
	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int update=0;
		struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

		if(data==NULL) return -1;

		gDHCPTable = ptable;
		*gDHCPTable = NULL;
		gDHCPTotalHosts=0;

		updateDHCPList(&update);
		if(gDHCPTotalHosts>0)
			return create_Object( gDHCPTable, tHostObject, sizeof(tHostObject), gDHCPTotalHosts, 1 );
		else
			return 0;
	     	break;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
		updateDHCP();
		return 0;
	     	break;
	     }
	}

	return -1;
}

int getHosts(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "HostNumberOfEntries" )==0 )
	{
		updateDHCP();
		*data = uintdup( gDHCPTotalHosts );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getIPItfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256];
	unsigned int vInt=0;
#ifdef SECONDARY_IP
	unsigned char instnum=0;
#endif //SECONDARY_IP	

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
#ifdef SECONDARY_IP
	instnum = getIPItfInstNum(name);
	if(instnum<=0) return ERR_9005;
#endif //SECONDARY_IP

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef SECONDARY_IP
	    if(instnum==2)
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vInt);
	    else
#endif //SECONDARY_IP
		mib_get(MIB_CWMP_LAN_IPIFENABLE, (void *)&vInt);
		//printf("<%s:%d>vInt=%d\n", __FUNCTION__,__LINE__,vInt);
		if(vInt==0)
			*data = uintdup(0);
		else
			*data = uintdup(1);
	}
	else if( strcmp( lastname, "IPInterfaceIPAddress" )==0 )
	{
#ifdef SECONDARY_IP
	    if(instnum==2)
		getMIBtoStr(MIB_ADSL_LAN_IP2, buf);
	    else
#endif //SECONDARY_IP
		getMIBtoStr(MIB_IP_ADDR, buf);
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "IPInterfaceSubnetMask" )==0 )
	{
#ifdef SECONDARY_IP
	    if(instnum==2)
		getMIBtoStr(MIB_ADSL_LAN_SUBNET2, buf);
	    else
#endif //SECONDARY_IP
		getMIBtoStr(MIB_SUBNET_MASK, buf);
		*data=strdup( buf );
	}
	else if( strcmp( lastname, "IPInterfaceAddressingType" )==0 )
		{
				*data = strdup( "Static" );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setIPItfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int vInt=0;
	struct in_addr in;
#ifdef SECONDARY_IP
	unsigned char instnum=0;
#endif //SECONDARY_IP

	
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
	if( data==NULL ) return ERR_9007;
#ifdef SECONDARY_IP
	instnum = getIPItfInstNum(name);
	if(instnum<=0) return ERR_9005;
#endif //SECONDARY_IP

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif
		
		if( i==NULL ) return ERR_9007;
		vInt = (*i==0)?0:1;
#ifdef SECONDARY_IP
	    if(instnum==2)
		mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
	    else
#endif //SECONDARY_IP
		mib_set(MIB_CWMP_LAN_IPIFENABLE, (void *)&vInt);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_LANIP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "IPInterfaceIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if(  inet_aton( buf, &in )==0  ) return ERR_9007;

/*for telefonica, when modifying this parameter, do not modify the dhcp gateway at the same time*/
#ifndef TELEFONICA_DEFAULT_CFG
		/*web behavior => after setting lan ip, change the dhcp gateway with the same ip*/
#ifdef SECONDARY_IP
		if(instnum==1)
#endif //SECONDARY_IP
		{
		    /*lan ip & dhcp gateway setting should be set independently*/
		    #if 1
			struct in_addr dhcp_gw, origIp;
			mib_get(MIB_IP_ADDR, (void *)&origIp);
			mib_get(MIB_DEFAULT_GATEWAY, (void *)&dhcp_gw);
			if(dhcp_gw.s_addr==origIp.s_addr)
				mib_set(MIB_DEFAULT_GATEWAY, (void *)&in);
		    #else
			mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&in);
		    #endif
		}
#endif //TELEFONICA_DEFAULT_CFG


#ifdef SECONDARY_IP
	    if(instnum==2)
		mib_set(MIB_ADSL_LAN_IP2, (void *)&in);	
	    else
#endif //SECONDARY_IP
		mib_set(MIB_IP_ADDR, (void *)&in);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_LANIP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif	
		
	}
	else if( strcmp( lastname, "IPInterfaceSubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
#ifdef SECONDARY_IP
	    if(instnum==2)
		mib_set(MIB_ADSL_LAN_SUBNET2, (void *)&in);
	    else
#endif //SECONDARY_IP
		mib_set(MIB_SUBNET_MASK, (void *)&in);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_LANIP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "IPInterfaceAddressingType" )==0 )
	{
		if( (buf!=NULL) && (strlen(buf)==0) ) return ERR_9007;
		if( strcmp(buf, "DHCP")==0 ) return ERR_9001;
		if( strcmp(buf, "AutoIP")==0 ) return ERR_9001;
		if( strcmp(buf, "Static")!=0 ) return ERR_9007;
	}
	else{
		return ERR_9005;
	}

	return 0;
}

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
unsigned int getDHCPConSPInstNum( char *name )
{
	return getInstNum( name, "DHCPConditionalServingPool" );
}

unsigned int getDHCPOptInstNum( char *name )
{
	return getInstNum( name, "DHCPOption" );
}

unsigned int getDHCPOptEntryNum(unsigned int usedFor)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOPT_entity;

	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOPT_entity;
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor==usedFor)
			ret++;
	}

	return ret;
}

/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
//move to utility.c
#if 0
unsigned int getSPDHCPOptEntryNum(unsigned int usedFor, unsigned int instnum)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOPT_entity;

	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOPT_entity;
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor==usedFor && p->dhcpConSPInstNum==instnum)
			ret++;
	}

	return ret;
}
#endif
/*ping_zhang:20090319 END*/

unsigned int DHCPServerReservedOption[]={
	53,  //DHCP Message type
	54,  //Server id
	51,  //Leasetime
	1,   //subnet mask
	3,   //router
	6,   //DNS Servers
	15,  //Domain name
	0
};//star: I think some important options can't be changed by user, because they are necessary or are managed by other parameters.
int checkDHCPOptionTag(unsigned int tagvalue)
{
	int i;
	unsigned int *tmp=DHCPServerReservedOption;

	while(*tmp!=0){
		if(tagvalue == *tmp)
			return -1;
		tmp++;
	}

	return 0;
}
/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
//move to utility.c
#if 0
void clearOptTbl(unsigned int instnum)
{
	unsigned int  i,num,found;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;

delOpt:
	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=eUsedFor_DHCPServer_ServingPool)
			continue;
		if(p->dhcpConSPInstNum==instnum){
			mib_chain_delete(MIB_DHCP_SERVER_OPTION_TBL,i);
			break;
		}
	}
	printf("\nnum=%d,i=%d\n",num,i);
	if(i<num)
		goto delOpt;
	return;

}
#endif
/*ping_zhang:20090319 END*/

int getDHCPOptionEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int dhcpOptNum,dhcpConSPOptNum;
	unsigned char usedFor;
	unsigned int chainid;

	dhcpOptNum = getDHCPOptInstNum( name );
	dhcpConSPOptNum = getDHCPConSPInstNum( name );

	pDHCPOptEntry = &DhcpOptEntry;
	if(dhcpConSPOptNum != 0)  						 //for IGD.LANDevice.{i}.LANHostConfigManagement.DHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPServer_ServingPool;
		if( getDHCPOptionByOptInstNum(dhcpOptNum,dhcpConSPOptNum, usedFor, pDHCPOptEntry, &chainid) < 0 )
		return ERR_9002;
	}
	else if(dhcpConSPOptNum==0 && dhcpOptNum!=0)	//for IGD.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.DHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPServer;
		if( getDHCPOptionByOptInstNum(dhcpOptNum,dhcpConSPOptNum, usedFor, pDHCPOptEntry, &chainid) < 0)
		return ERR_9002;
	}
	else
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pDHCPOptEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
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

int setDHCPOptionEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int dhcpOptNum,dhcpConSPOptNum;
	unsigned char usedFor;
	unsigned int chainid;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	dhcpOptNum = getDHCPOptInstNum( name );
	dhcpConSPOptNum = getDHCPConSPInstNum( name );

	pDHCPOptEntry = &DhcpOptEntry;
	if(dhcpConSPOptNum != 0)  						 //for IGD.LANDevice.{i}.LANHostConfigManagement.DHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPServer_ServingPool;
		if( getDHCPOptionByOptInstNum(dhcpOptNum, dhcpConSPOptNum, usedFor, pDHCPOptEntry, &chainid) < 0 )
		return ERR_9002;
	}
	else if(dhcpConSPOptNum==0 && dhcpOptNum!=0)	//for IGD.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.DHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPServer;
		if( getDHCPOptionByOptInstNum(dhcpOptNum, dhcpConSPOptNum, usedFor, pDHCPOptEntry, &chainid) < 0)
		return ERR_9002;
	}
	else
		return ERR_9005;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPOptEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 || *i>254) return ERR_9007;
		if(checkDHCPOptionTag(*i)<0)  return ERR_9001;
		pDHCPOptEntry->tag = *i;
		mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(data)
		{
			   int i;
			   struct xsd__base64 *b=data;
			   fprintf( stderr, "<xsd__base64:size %d>", b->__size );
			   for( i=0; i<b->__size; i++ )
			   {
			    fprintf( stderr, "%u(%c) ", b->__ptr[i], b->__ptr[i]  );
			   }
			   fprintf( stderr, "\n" );
			   if(b->__size>DHCP_OPT_VAL_LEN) return ERR_9001;
			   pDHCPOptEntry->len=b->__size;
			   memcpy(pDHCPOptEntry->value,b->__ptr,b->__size);
		}
		mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int objDHCPOption(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int dhcpConSPInstNum, usedFor,num,i;
	unsigned int chainid;

	dhcpConSPInstNum = getDHCPConSPInstNum( name );
	if(dhcpConSPInstNum != 0)   //for IGD.LANDevice.{i}.LANHostConfigManagement.DHCPConditionalServingPool.{i}.DHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPServer_ServingPool;
		switch( type )
		{
		case eCWMP_tINITOBJ:
			{
				int MaxInstNum;
				struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				MaxInstNum = findMaxDHCPOptionInstNum(usedFor,dhcpConSPInstNum);
				num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
				for( i=0; i<num;i++ )
				{
					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ))
						continue;

					if(pDHCPOptEntry->usedFor != usedFor)
						continue;

					if( pDHCPOptEntry->dhcpConSPInstNum == dhcpConSPInstNum)
					{
						if( pDHCPOptEntry->dhcpOptInstNum==0 ) //maybe createn by web or cli
						{
							MaxInstNum++;
							pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
							mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
						}
						if( create_Object( c, tDHCPOptionObject, sizeof(tDHCPOptionObject), 1, pDHCPOptEntry->dhcpOptInstNum ) < 0 )
							return -1;
					}
				}
				add_objectNum( name, MaxInstNum );
				return 0;
			}
		case eCWMP_tADDOBJ:
			{
				int ret, found=0;
				DHCPS_SERVING_POOL_T *pDHCPSPOptEntry, DhcpSPOptEntry;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
				for(i=0; i<num; i++)
				{
					pDHCPSPOptEntry = &DhcpSPOptEntry;
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPSPOptEntry ) )
						continue;
					if(pDHCPSPOptEntry->InstanceNum == dhcpConSPInstNum )
					{
						found = 1;
						break;
					}
				}
				if(found ==0) return ERR_9005;
				ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPOptionObject, sizeof(tDHCPOptionObject), data );
				if( ret >= 0 )
				{
					MIB_CE_DHCP_OPTION_T entry;
					memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
					{ //default values for this new entry
						entry.enable = 0;
						entry.dhcpConSPInstNum = pDHCPSPOptEntry->InstanceNum;
						entry.usedFor = usedFor;
						entry.dhcpOptInstNum =*(int*)data;
					}
					mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)&entry);
				}
				return ret;
			}
		case eCWMP_tDELOBJ:
			{
				int ret, num, i;
				int found = 0;
				unsigned int *pUint=data;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
				for( i=0;i<num;i++ )
				{
					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
						continue;
					if(pDHCPOptEntry->usedFor == usedFor
						&& pDHCPOptEntry->dhcpConSPInstNum==dhcpConSPInstNum
						&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
					{
						found =1;
						mib_chain_delete( MIB_DHCP_SERVER_OPTION_TBL, i );
						break;
					}
				}

				if(found==0) return ERR_9005;
				ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
				apply_DHCP(CWMP_RESTART,0,NULL);
				//if( ret==0 )	ret=1;
				return ret;
			}
		case eCWMP_tUPDATEOBJ:
			{
				int num,i;
				struct CWMP_LINKNODE *old_table;

				num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
				old_table = (struct CWMP_LINKNODE *)entity->next;
				entity->next = NULL;
				for( i=0; i<num;i++ )
				{
					struct CWMP_LINKNODE *remove_entity=NULL;

					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ))
						continue;

					if( (pDHCPOptEntry->usedFor == usedFor) &&
						(pDHCPOptEntry->dhcpConSPInstNum == dhcpConSPInstNum) ) // &&
						//(pDHCPOptEntry->dhcpOptInstNum!=0))
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
								add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPOptionObject, sizeof(tDHCPOptionObject), &MaxInstNum );
								if(MaxInstNum!=pDHCPOptEntry->dhcpOptInstNum)
								{
									pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
									mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
								}
							}//else already in next_table
						}
					}
				}

				if( old_table )
					destroy_ParameterTable( (struct CWMP_NODE *)old_table );

				return 0;
			}
		}
	}
	else					//for IGD.LANDevice.{i}.LANHostConfigManagement.DHCPOption.{i}.
	{
		usedFor = eUsedFor_DHCPServer;
		switch( type )
		{
		case eCWMP_tINITOBJ:
			{
				int MaxInstNum;
				struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				MaxInstNum = findMaxDHCPOptionInstNum(usedFor,0);
				num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
				for( i=0; i<num;i++ )
				{
					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ))
						continue;
					if(pDHCPOptEntry->usedFor != usedFor )
						continue;
					if( pDHCPOptEntry->dhcpOptInstNum==0 ) //maybe createn by web or cli
					{
						MaxInstNum++;
						pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
						mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
					}
					if( create_Object( c, tDHCPOptionObject, sizeof(tDHCPOptionObject), 1, pDHCPOptEntry->dhcpOptInstNum ) < 0 )
						return -1;

				}
				add_objectNum( name, MaxInstNum );
				return 0;
			}
		case eCWMP_tADDOBJ:
			{
				int ret;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		
				ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPOptionObject, sizeof(tDHCPOptionObject), data );
				if( ret >= 0 )
				{
					MIB_CE_DHCP_OPTION_T entry;
					memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
					{ //default values for this new entry
						entry.enable = 0;
						entry.usedFor = usedFor;
						entry.dhcpOptInstNum = *(int*)data;
						entry.dhcpConSPInstNum = 0;
					}
					mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)&entry);
				}
				return ret;
			}
		case eCWMP_tDELOBJ:
			{
				int ret, num, i;
				int found = 0;
				unsigned int *pUint=data;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
				for( i=0;i<num;i++ )
				{
					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
						continue;
					if(pDHCPOptEntry->usedFor == eUsedFor_DHCPServer && pDHCPOptEntry->dhcpOptInstNum==*pUint)
					{
						found =1;
						mib_chain_delete( MIB_DHCP_SERVER_OPTION_TBL, i );
						break;
					}
				}

				if(found==0) return ERR_9005;
				ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
				//if( ret==0 )	ret=1;
				return ret;
			}
		case eCWMP_tUPDATEOBJ:
			{
				int num,i;
				struct CWMP_LINKNODE *old_table;

				num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
				old_table = (struct CWMP_LINKNODE *)entity->next;
				entity->next = NULL;
				for( i=0; i<num;i++ )
				{
					struct CWMP_LINKNODE *remove_entity=NULL;

					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ))
						continue;

					if( (pDHCPOptEntry->usedFor == usedFor) ) //&& (pDHCPOptEntry->dhcpOptInstNum!=0))
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
								add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPOptionObject, sizeof(tDHCPOptionObject), &MaxInstNum );
								if(MaxInstNum!=pDHCPOptEntry->dhcpOptInstNum)
								{
									pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
									mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
								}
							}//else already in next_table
						}
					}
				}

				if( old_table )
					destroy_ParameterTable( (struct CWMP_NODE *)old_table );

				return 0;
			}
		}
	}



	return -1;
}

int getSPDNSList( DHCPS_SERVING_POOL_T *p, char *buf )
{
	unsigned char tmp[64];
	char *zeroip="0.0.0.0";

	if( buf==NULL ) return -1;

	buf[0]=0;
	tmp[0]=0;
	strcpy(tmp,inet_ntoa(*((struct in_addr *)p->dnsserver1)));
	if( (strlen(tmp)>0) && (strcmp(tmp, zeroip)!=0) )
		strcat( buf, tmp );

	tmp[0]=0;
	strcpy(tmp,inet_ntoa(*((struct in_addr *)p->dnsserver2)));
	if( (strlen(tmp)>0) && (strcmp(tmp, zeroip)!=0) )
	{
		if( strlen(buf) > 0 )
			strcat( buf, ",");
		strcat( buf, tmp );
	}

	tmp[0]=0;
	strcpy(tmp,inet_ntoa(*((struct in_addr *)p->dnsserver3)));
	if( (strlen(tmp)>0) && (strcmp(tmp, zeroip)!=0) )
	{
		if( strlen(buf) > 0 )
			strcat( buf, ",");
		strcat( buf, tmp );
	}

	return 0;
}
int setSPDNSList( DHCPS_SERVING_POOL_T *p, char *buf )
{
	char *tok1, *tok2, *tok3;
	int  ret=0;
	struct in_addr in1,in2,in3,emp_in;

	if( buf==NULL ) return -1;
	tok1 = NULL;
	tok2 = NULL;
	tok3 = NULL;

	tok1=strtok( buf, "," );
	tok2=strtok( NULL, "," );
	tok3=strtok( NULL, "," );
	if( (tok1==NULL)&&(tok2==NULL)&&(tok3==NULL) )
		return -1;

	if(tok1)
		if(  inet_aton( tok1, &in1 )==0  ) ret=-1;
	if(tok2)
		if(  inet_aton( tok2, &in2 )==0  ) ret=-1;
	if(tok3)
		if(  inet_aton( tok3, &in3 )==0  ) ret=-1;

	memset( &emp_in, 0, sizeof(struct in_addr) );
	if(ret==0)
	{
		if( tok1!=NULL )
			memcpy(p->dnsserver1, &in1, 4);
		else
			memcpy(p->dnsserver1, &emp_in, 4);

/*ping_zhang:20081104 START:telefonica tr069 new request verify*/
#if 0
		if( tok2!=NULL )
			memcpy(p->dnsserver1, &in2, 4);
		else
			memcpy(p->dnsserver1, &emp_in, 4);

		if( tok3!=NULL )
			memcpy(p->dnsserver1, &in3, 4);
		else
			memcpy(p->dnsserver1, &emp_in, 4);
#else
		if( tok2!=NULL )
			memcpy(p->dnsserver2, &in2, 4);
		else
			memcpy(p->dnsserver2, &emp_in, 4);

		if( tok3!=NULL )
			memcpy(p->dnsserver3, &in3, 4);
		else
			memcpy(p->dnsserver3, &emp_in, 4);
#endif
/*ping_zhang:20081104 END*/
	}
	return ret;
}


int checkandmodify_poolorder(unsigned int order, int chainid)
{
	int ret=-1;
	int num,i;
	int maxorder;
	DHCPS_SERVING_POOL_T *p,pentry;

	p=&pentry;
	maxorder=findMaxDHCPConSPOrder();
	if(order>maxorder+1)
		goto checkresult;
	else{
		num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
		for( i=0; i<num;i++ )
		{
			if(i==chainid)
				continue;
			if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ))
				continue;
			if(p->poolorder>=order){
				(p->poolorder)++;
				mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL,(void*)p,i);
			}
		}
		ret=0;
	}

checkresult:
	return ret;
}

void compact_poolorder( )
{
	int ret=-1;
	int num,i,j;
	int maxorder;
	DHCPS_SERVING_POOL_T *p,pentry;
	char *orderflag;

	while(1){
		p=&pentry;
		maxorder=findMaxDHCPConSPOrder();
		orderflag=(char*)malloc(maxorder+1);
		if(orderflag==NULL) return;
		memset(orderflag,0,maxorder+1);

		num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
		for( i=0; i<num;i++ )
		{
				if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ))
					continue;
				orderflag[p->poolorder]=1;
		}
		for(j=1;j<=maxorder;j++){
			if(orderflag[j]==0)
				break;
		} //star: there only one 0 in orderflag array
		if(j==(maxorder+1))
			break;
		for( i=0; i<num;i++ )
		{

				if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ))
					continue;
				if(p->poolorder>j){
					(p->poolorder)--;
					mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL,(void*)p,i);
				}
		}

		if(orderflag)
		{
			free(orderflag);
			orderflag=NULL;
		}
	}

}
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
int compactmacaddr(char *newmac, char* oldmac){
	int i,maclen;
	char *p;
	char tmp[10];
	unsigned char vChar;

	maclen=strlen(oldmac);

	p=strtok(oldmac,":");
	while(p){
		strcat(newmac,p);
		p=strtok(NULL,":");
	}

	//printf("newmac=%s,oldmac=%s",newmac,oldmac);
	return 1;
}
int getDHCPConSPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;;
	unsigned int chainid;
	unsigned int dhcpConSPInstNum;
	int num,i;
	int ret=0;
	char buf[256];
	DHCPS_SERVING_POOL_T *pDHCPSPEntry, DhcpSPEntry;

	*type = entity->info->type;
	*data = NULL;
		
	pDHCPSPEntry = &DhcpSPEntry;
	dhcpConSPInstNum = getDHCPConSPInstNum( name );
	ret=getDHCPConSPByInstNum(dhcpConSPInstNum,pDHCPSPEntry,&chainid);
	if(ret<0) return ERR_9002;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(pDHCPSPEntry->enable);
	}else if( strcmp( lastname, "PoolOrder" )==0 )
	{
		*data = uintdup(pDHCPSPEntry->poolorder);
	}else if( strcmp( lastname, "SourceInterface" )==0 )
	{
		char qstr[302];
		char focr = 0;

		qstr[0] = 0;
			
		if(!pDHCPSPEntry->sourceinterface)
			*data = strdup("");
		else
		{
			strcpy(buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.");

			for( i = 0; i < 4; i++ )
			{
				if((pDHCPSPEntry->sourceinterface >> i) & 1)
				{
					sprintf(qstr, "%s%s%s%d", qstr, focr? ",": "", buf, i+1);
					focr = 1;
				}
			}

#ifdef WLAN_SUPPORT
			strcpy(buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1");
		
			if((pDHCPSPEntry->sourceinterface >> 4) & 1)
				sprintf(qstr, "%s%s%s", qstr, focr? ",": "", buf);
#endif

			*data = strdup(qstr);
		}
	}else if( strcmp( lastname, "VendorClassID" )==0 )
	{
		*data = strdup(pDHCPSPEntry->vendorclass);
	}else if( strcmp( lastname, "VendorClassIDExclude" )==0 )
	{
		*data = booldup(pDHCPSPEntry->vendorclassflag);
	}else if( strcmp( lastname, "VendorClassIDMode" )==0 )
	{
		*data = strdup(pDHCPSPEntry->vendorclassmode);
	}else if( strcmp( lastname, "ClientID" )==0 )
	{
		*data = strdup(pDHCPSPEntry->clientid);
	}else if( strcmp( lastname, "ClientIDExclude" )==0 ){
		*data = booldup(pDHCPSPEntry->clientidflag);
	}else if( strcmp( lastname, "UserClassID" )==0 )
	{
		*data = strdup(pDHCPSPEntry->userclass);
	}else if( strcmp( lastname, "UserClassIDExclude" )==0 )
	{
		*data = booldup(pDHCPSPEntry->userclassflag);
	}else if( strcmp( lastname, "Chaddr" )==0 )
	{
		//00:00:00:00:00:00 returns an empty string, not used for conditional serving
		if( (pDHCPSPEntry->chaddr[0]==0) &&
			(pDHCPSPEntry->chaddr[1]==0) &&
			(pDHCPSPEntry->chaddr[2]==0) &&
			(pDHCPSPEntry->chaddr[3]==0) &&
			(pDHCPSPEntry->chaddr[4]==0) &&
			(pDHCPSPEntry->chaddr[5]==0) )
		  	buf[0]=0;
		else
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pDHCPSPEntry->chaddr[0],pDHCPSPEntry->chaddr[1],pDHCPSPEntry->chaddr[2],
				pDHCPSPEntry->chaddr[3],pDHCPSPEntry->chaddr[4],pDHCPSPEntry->chaddr[5]);
		*data = strdup(buf);
	}else if( strcmp( lastname, "ChaddrMask" )==0 )
	{
		sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pDHCPSPEntry->chaddrmask[0],pDHCPSPEntry->chaddrmask[1],pDHCPSPEntry->chaddrmask[2],
			pDHCPSPEntry->chaddrmask[3],pDHCPSPEntry->chaddrmask[4],pDHCPSPEntry->chaddrmask[5]);
		*data = strdup(buf);
	}else if( strcmp( lastname, "ChaddrExclude" )==0 )
	{
		*data = booldup(pDHCPSPEntry->chaddrflag);
	}else if( strcmp( lastname, "LocallyServed" )==0 )
	{
		*data = booldup(pDHCPSPEntry->localserved);
	}else if( strcmp( lastname, "MinAddress" )==0 )
	{
		strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->startaddr)));
		*data=strdup( buf );
	}else if( strcmp( lastname, "MaxAddress" )==0 )
	{
		strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->endaddr)));
		*data=strdup( buf );
	}else if( strcmp( lastname, "ReservedAddresses" )==0 )
	{
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
		if( getDHCPReservedIPAddr( pDHCPSPEntry->InstanceNum, FILE4DHCPReservedIPAddr )==0 )
		{
			*type = eCWMP_tFILE; /*special case*/
			*data=strdup( FILE4DHCPReservedIPAddr );
		}else
			*data=strdup( "" );
#else
		*data=strdup( "" );
#endif //SUPPORT_DHCP_RESERVED_IPADDR
	}else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->subnetmask)));
		*data=strdup( buf );
	}else if( strcmp( lastname, "DNSServers" )==0 )
	{
		if( pDHCPSPEntry->dnsservermode==0 ) //automatically attain DNS
			*data=strdup("");
		else
		{
			getSPDNSList(pDHCPSPEntry,buf);
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "DomainName" )==0 )
	{
		*data=strdup( pDHCPSPEntry->domainname );
	}else if( strcmp( lastname, "IPRouters" )==0 )
	{
		strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->iprouter)));
		*data=strdup( buf );
	}else if( strcmp( lastname, "DHCPLeaseTime" )==0 )
	{
		*data=intdup( pDHCPSPEntry->leasetime );
	}else if( strcmp( lastname, "UseAllocatedWAN" )==0 )
	{
		*data=strdup( "Normal" );
	}else if( strcmp( lastname, "AssociatedConnection" )==0 )
	{
		*data=strdup( "" );
	}else if( strcmp( lastname, "DHCPOptionNumberOfEntries" )==0 )
	{
		unsigned int usedFor=eUsedFor_DHCPServer_ServingPool;
		*data = uintdup(getSPDHCPOptEntryNum(usedFor, dhcpConSPInstNum));
	}else if( strcmp( lastname, "DHCPServerIPAddress" )==0 )
	{
		strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->dhcprelayip)));
		*data=strdup( buf );
	}else
	{
		return ERR_9005;
	}
	return 0;
}

int setDHCPConSPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int chainid;
	unsigned int dhcpConSPInstNum;
	int num,i;
	int ret=0;
	char *buf=data;
	DHCPS_SERVING_POOL_T *pDHCPSPEntry, DhcpSPEntry;
	char tmpbuf[30]={0};
	char *tok, del[] = ", ", *pstr;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	pDHCPSPEntry = &DhcpSPEntry;
	dhcpConSPInstNum = getDHCPConSPInstNum( name );
	ret=getDHCPConSPByInstNum(dhcpConSPInstNum,pDHCPSPEntry,&chainid);
	if(ret<0) return ERR_9002;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPSPEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "PoolOrder" )==0 )
	{
		unsigned int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1) return ERR_9007;
		if(checkandmodify_poolorder(*i,chainid)<0) return ERR_9007;
		pDHCPSPEntry->poolorder = *i;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		compact_poolorder();
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "SourceInterface" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if( strlen(buf)==0 )
		{
			pDHCPSPEntry->sourceinterface = 0;
#ifdef _CWMP_APPLY_
			apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
			return 0;
#else
			return 1;
#endif
		}

		pDHCPSPEntry->sourceinterface = 0;
		
		for( tok = strtok(buf, del); tok; tok = strtok(NULL, del) )
		{
			if(pstr = strstr(tok, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig."))
			{
				pstr += 61;  // Length of "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig."

				if( *pstr < '1' && *pstr > '4' )
					return ERR_9007;
			
				pDHCPSPEntry->sourceinterface |= 1 << (*pstr - '0' - 1);
			}
#ifdef WLAN_SUPPORT
			else if(strstr(tok, "InternetGatewayDevice.LANDevice.1.WLANConfiguration."))
			{
				pDHCPSPEntry->sourceinterface |= 0x10;
			}
#endif
		else
				return ERR_9007;
		}
		mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid);
		
		//printf("source interface=%d%d%d%d%d\n", (pDHCPSPEntry->sourceinterface & 0x10) >> 4, (pDHCPSPEntry->sourceinterface & 0x8) >> 3,
			(pDHCPSPEntry->sourceinterface & 0x4) >> 2, (pDHCPSPEntry->sourceinterface & 0x2) >> 1,	pDHCPSPEntry->sourceinterface & 0x1);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "VendorClassID" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#if 0
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->vendorclass,0,OPTION_LEN);
		else{
			strncpy(pDHCPSPEntry->vendorclass,buf,OPTION_LEN-1);
			pDHCPSPEntry->vendorclass[OPTION_LEN-1]=0;
		}
#else
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->vendorclass,0,OPTION_60_LEN+1);
		else{
			strncpy(pDHCPSPEntry->vendorclass,buf,OPTION_60_LEN);
			pDHCPSPEntry->vendorclass[OPTION_60_LEN]=0;
		}
#endif
/*ping_zhang:20090319 END*/
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "VendorClassIDExclude" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPSPEntry->vendorclassflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "VendorClassIDMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if(strcmp(buf,"Exact")&&strcmp(buf,"Prefix")&&strcmp(buf,"Suffix")&&strcmp(buf,"Substring"))
			return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->vendorclassmode,0,MODE_LEN);
		else{
			strncpy(pDHCPSPEntry->vendorclassmode,buf,MODE_LEN-1);
			pDHCPSPEntry->vendorclassmode[MODE_LEN-1]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "ClientID" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->clientid,0,OPTION_LEN);
		else{
			strncpy(pDHCPSPEntry->clientid,buf,OPTION_LEN-1);
			pDHCPSPEntry->clientid[OPTION_LEN-1]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "ClientIDExclude" )==0 ){
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPSPEntry->clientidflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "UserClassID" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->userclass,0,OPTION_LEN);
		else{
			strncpy(pDHCPSPEntry->userclass,buf,OPTION_LEN-1);
			pDHCPSPEntry->userclass[OPTION_LEN-1]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "UserClassIDExclude" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPSPEntry->userclassflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "Chaddr" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		//printf("\nbuf=%s\n",buf);
		//00:00:00:00:00:00 or an empty string means "not used for conditional serving"
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->chaddr,0,MAC_ADDR_LEN);
		else{
			if( strlen(buf)!=17) return ERR_9007;
			if(compactmacaddr(tmpbuf,buf)==0) return ERR_9007;
			if(string_to_hex(tmpbuf,pDHCPSPEntry->chaddr,12)==0) return ERR_9007;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "ChaddrMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		//FF:FF:FF:FF:FF:FF or an empty string indicates all bits of the Chaddr are to be used for conditional serving classification
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->chaddrmask,0xff,MAC_ADDR_LEN);
		else{
			if(strlen(buf)!=17) return ERR_9007;
			if(compactmacaddr(tmpbuf,buf)==0) return ERR_9007;
			if(string_to_hex(tmpbuf,pDHCPSPEntry->chaddrmask,12)==0) return ERR_9007;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "ChaddrExclude" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPSPEntry->chaddrflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "LocallyServed" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPSPEntry->localserved = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "MinAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->startaddr,0,IP_ADDR_LEN);
		else
			if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->startaddr)) return ERR_9007;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "MaxAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->endaddr,0,IP_ADDR_LEN);
		else
			if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->endaddr)) return ERR_9007;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "ReservedAddresses" )==0 )
	{
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
		if( setDHCPReservedIPAddr(pDHCPSPEntry->InstanceNum,buf)<0 ) return ERR_9007;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif //_CWMP_APPLY_
#else
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
		return 0;
#endif //SUPPORT_DHCP_RESERVED_IPADDR
	}else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->subnetmask,0,IP_ADDR_LEN);
		else
			if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->subnetmask)) return ERR_9007;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DNSServers" )==0 )
	{
		char *tok;
		if( (buf==NULL) || (strlen(buf)==0) )
		{ 	//automatically attain DNS
			pDHCPSPEntry->dnsservermode=0;
		}else if( setSPDNSList( pDHCPSPEntry, buf ) == 0 )
		{
			pDHCPSPEntry->dnsservermode=1;
		}else
			 return ERR_9007;
		
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DomainName" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->domainname,0,GENERAL_LEN);
		else{
			strncpy(pDHCPSPEntry->domainname,buf,GENERAL_LEN-1);
			pDHCPSPEntry->domainname[GENERAL_LEN-1]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "IPRouters" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->iprouter,0,IP_ADDR_LEN);
		else
			if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->iprouter)) return ERR_9007;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DHCPLeaseTime" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<-1) return ERR_9007;
		pDHCPSPEntry->leasetime=*i;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "UseAllocatedWAN" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "Normal" )!=0 ) return ERR_9007;
		return 0;
	}else if( strcmp( lastname, "AssociatedConnection" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		//if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "" )!=0 ) return ERR_9001;
		return 0;
	}else if( strcmp(lastname, "DHCPServerIPAddress") == 0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->dhcprelayip,0,IP_ADDR_LEN);
		else
			if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->dhcprelayip)) return ERR_9007;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else
	{
		return ERR_9005;
	}
	return 0;
}

int objDHCPConSP(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	DHCPS_SERVING_POOL_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int chainid;
	unsigned int num,i;

	switch(type)
		{
		case eCWMP_tINITOBJ:
			{
				int MaxInstNum;
				struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				MaxInstNum = findMaxDHCPConSPInsNum();
				num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
				for( i=0; i<num;i++ )
				{
					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPOptEntry ))
						continue;
	
					if( pDHCPOptEntry->InstanceNum==0 )
					{
						MaxInstNum++;
						pDHCPOptEntry->InstanceNum = MaxInstNum;
						mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPOptEntry, i );
					}
					if( create_Object( c, tDHCPConSPObject, sizeof(tDHCPConSPObject), 1, pDHCPOptEntry->InstanceNum ) < 0 )
						return -1;
				}
				add_objectNum( name, MaxInstNum );
	return 0;
}
		case eCWMP_tADDOBJ:
			{
				int ret;
				char tmpbuf[128];

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPConSPObject, sizeof(tDHCPConSPObject), data );
				if( ret >= 0 )
				{
					DHCPS_SERVING_POOL_T entry;
					memset( &entry, 0, sizeof( DHCPS_SERVING_POOL_T ) );
					{ //default values for this new entry
						entry.enable = 0;
						entry.poolorder = findMaxDHCPConSPOrder() + 1;
						sprintf(tmpbuf,"servingpool%d",*(int*)data);
						strcpy(entry.poolname,tmpbuf);
						entry.leasetime=86400;
						entry.InstanceNum = *(int *)data;
						entry.localserved = 1;//default: locallyserved=true;
						memset(entry.chaddrmask,0xff,MAC_ADDR_LEN);//default to all 0xff
						strncpy(entry.vendorclassmode,"Substring",MODE_LEN-1);
						entry.vendorclassmode[MODE_LEN-1]=0;
					}
					mib_chain_add( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)&entry);
				}
				return ret;
			}
		case eCWMP_tDELOBJ:
			{
				int ret, num, i;
				int found = 0;
				unsigned int *pUint=data;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
				for( i=0; i<num;i++ )
				{
					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPOptEntry ) )
						continue;
					if(pDHCPOptEntry->InstanceNum==*pUint)
					{
						found =1;
						clearOptTbl(pDHCPOptEntry->InstanceNum);
					#ifdef SUPPORT_DHCP_RESERVED_IPADDR
						clearDHCPReservedIPAddrByInstNum( pDHCPOptEntry->InstanceNum );
					#endif //SUPPORT_DHCP_RESERVED_IPADDR
						mib_chain_delete( MIB_DHCPS_SERVING_POOL_TBL, i );
						compact_poolorder();
						break;
					}
				}

				if(found==0) return ERR_9005;
				ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
				apply_DHCP(CWMP_RESTART,0,NULL);
				//if( ret==0 )	ret=1;
				return ret;
			}
		case eCWMP_tUPDATEOBJ:
			{
				int num,i;
				struct CWMP_LINKNODE *old_table;

				num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
				old_table = (struct CWMP_LINKNODE *)entity->next;
				entity->next = NULL;
				for( i=0; i<num;i++ )
				{
					struct CWMP_LINKNODE *remove_entity=NULL;

					pDHCPOptEntry = &DhcpOptEntry;
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPOptEntry ))
						continue;

					remove_entity = remove_SiblingEntity( &old_table, pDHCPOptEntry->InstanceNum );
					if( remove_entity!=NULL )
					{
							add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
							if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pDHCPOptEntry->InstanceNum )==NULL )
							{
								unsigned int MaxInstNum = pDHCPOptEntry->InstanceNum;
								add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPConSPObject, sizeof(tDHCPConSPObject), &MaxInstNum );
								if(MaxInstNum!=pDHCPOptEntry->InstanceNum)
								{
									pDHCPOptEntry->InstanceNum = MaxInstNum;
									mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPOptEntry, i );
								}
							}//else already in next_table
					}
				}

				if( old_table )
					destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	
	return 0;
}
		}
}
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/

int getLANHostConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]="";
	//unsigned char vChar=0;
	unsigned int vChar=0;
	int  vInt=0;
	int dhcp_enynum;
	int i, entryNum;

	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	if( strcmp( lastname, "MACAddress" )==0 )
	{
		unsigned char buffer[64];
		unsigned char macadd[MAC_ADDR_LEN];
		mib_get(MIB_ELAN_MAC_ADDR, (void *)macadd);
		sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0], macadd[1],						
			macadd[2], macadd[3], macadd[4], macadd[5]);
		*data=strdup(buffer);
	}else
#endif
/*ping_zhang:20081217 END*/
	if( strcmp( lastname, "DHCPServerConfigurable" )==0 )
	{		
		mib_get( MIB_LAN_DHCP_CONFIGURABLE, (void *)&vInt);
		*data = booldup( (vInt!=0) );
	}
	else if( strcmp( lastname, "DHCPServerEnable" )==0 )
	{
		mib_get( MIB_DHCP, (void *)&vInt);
		*data = booldup( (vInt!=DHCP_DISABLED) );
	}
	else if( strcmp( lastname, "DHCPRelay" )==0 )
	{
		mib_get( MIB_DHCP, (void *)&vInt);
		*data = booldup( (vInt==DHCP_RELAY) ); //default 'server' when disable
	}
	else if( strcmp( lastname, "MinAddress" )==0 )
	{
		getMIBtoStr(MIB_DHCP_CLIENT_START, buf);
		*data=strdup( buf );
	}
	else if( strcmp( lastname, "MaxAddress" )==0 )
	{
		getMIBtoStr(MIB_DHCP_CLIENT_END, buf);
		*data=strdup( buf );
	}
#ifdef _PRMT_X_CT_COM_DHCP_
	else if( strcmp( lastname, "X_CT-COM_STB-MinAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_STB_MINADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_STB-MaxAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_STB_MAXADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MinAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_PHN_MINADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MaxAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_PHN_MAXADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_Camera-MinAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_CMR_MINADDR, buf);
	}else if( strcmp( lastname, "X_CT-COM_Camera-MaxAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_CMR_MAXADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MinAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_PC_MINADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MaxAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_PC_MAXADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_HGW-MaxAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_HGW_MAXADDR, buf);
	}
	else if( strcmp( lastname, "X_CT-COM_HGW-MinAddress" )==0 )
	{
		getMIBtoStr(MIB_CT_HGW_MINADDR, buf);
	}	
#endif //_PRMT_X_CT_COM_DHCP_
	else if( strcmp( lastname, "ReservedAddresses" )==0 )
	{
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
		if( getDHCPReservedIPAddr( 0, FILE4DHCPReservedIPAddr )==0 )
		{
			*type = eCWMP_tFILE; /*special case*/
			*data=strdup( FILE4DHCPReservedIPAddr );
		}else
			*data=strdup( "" );		
#else
		*data=strdup( "" );
#endif //SUPPORT_DHCP_RESERVED_IPADDR
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
#ifdef SECONDARY_IP
		mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar);
#else
		vChar = 0;
#endif //SECONDARY_IP
		getMIBtoStr(MIB_SUBNET_MASK, buf);
		*data=strdup( buf );
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		mib_get( MIB_DNS_MODE, (void *)&vInt); // 0-Auto; 1-Manual
		//printf("<%s: %d>vInt=%d\n", __FUNCTION__, __LINE__, vInt);
		if( vInt==0 ) //automatically attain DNS
			*data=strdup("");
		else
		{
			getDNSList(buf);			
			*data = strdup( buf );
		}
	}
	else if( strcmp( lastname, "DomainName" )==0 )
	{
		getMIBtoStr(MIB_DOMAIN_NAME, buf);
		*data=strdup( buf );
	}
	else if( strcmp( lastname, "IPRouters" )==0 )
	{
		getMIBtoStr(MIB_DEFAULT_GATEWAY, buf);
		*data=strdup( buf );
	}
	else if( strcmp( lastname, "DHCPLeaseTime" )==0 )
	{
		mib_get(MIB_DHCP_LEASE_TIME, (void *)&vInt);
		*data=intdup( vInt );
	}
	else if( strcmp( lastname, "UseAllocatedWAN" )==0 )
	{
		*data=strdup( "Normal" );
	}
	else if( strcmp( lastname, "AssociatedConnection" )==0 )
	{
		*data=strdup( "" );
	}
	else if( strcmp( lastname, "PassthroughLease" )==0 )
	{
		*data=uintdup( 0 );
	}
	else if( strcmp( lastname, "PassthroughMACAddress" )==0 )
	{
		*data=strdup( "" );
	}
#ifdef _CWMP_MAC_FILTER_
	else if( strcmp( lastname, "AllowedMACAddresses" )==0 )
	{
		buf[0]=0;
		getMACAddressList( buf,256 );
		*data=strdup( buf );
	}
#endif /*_CWMP_MAC_FILTER_*/
	
	else if( strcmp( lastname, "IPInterfaceNumberOfEntries" )==0 )
	{
#if defined(CONFIG_X_CWMP_MULTI_LAN)	
		unsigned int IPIFCount=0;
			IPIFCount = getLANConDevIPIFCount(chainid);
			*data = uintdup(IPIFCount);
#else
		*data = uintdup(1);
#endif
	}
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	
	else if( strcmp( lastname, "DHCPOptionNumberOfEntries" )==0 )
	{
		unsigned int usedFor=eUsedFor_DHCPServer;
		*data = uintdup(getDHCPOptEntryNum(usedFor));
	}
	else if( strcmp( lastname, "DHCPConditionalPoolNumberOfEntries" )==0 )
	{
		int var=mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
		*data = uintdup(var);
	}	
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_
/*ping_zhang:20080919 END*/
	else{
		return ERR_9005;
	}
	
	return 0;
}

int setLANHostConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int	vInt=0;
	unsigned int	ServerEn=0;
	struct in_addr in;
	
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
	//if( data==NULL ) return ERR_9007;

	mib_get( MIB_DHCP, (void *)&ServerEn);
	if( strcmp( lastname, "DHCPServerConfigurable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif

		if( i==NULL ) return ERR_9007;
		vInt = (*i==0)?0:1;
		mib_set(MIB_LAN_DHCP_CONFIGURABLE, (void *)&vInt);
		return 0;
	}
	else if( strcmp( lastname, "DHCPServerEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;
#else
		int *i = data;
#endif
		
		if( i==NULL ) return ERR_9007;
		//if( ServerEn==0 ) return ERR_9001;
		vInt = (*i==0)?0:DHCP_SERVER; /*default:DHCP Server, not relay*/
		mib_set(MIB_DHCP, (void *)&vInt);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "MinAddress" )==0 )
	{		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set( MIB_DHCP_CLIENT_START, (void *)&in);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
}
	else if( strcmp( lastname, "MaxAddress" )==0 )
{		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set( MIB_DHCP_CLIENT_END, (void *)&in);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#ifdef _PRMT_X_CT_COM_DHCP_
	}
	else if( strcmp( lastname, "X_CT-COM_STB-MinAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_STB_MINADDR, (void *)&in);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_STB-MaxAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_STB_MAXADDR, (void *)&in);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MinAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_PHN_MINADDR, (void *)&in);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Phone-MaxAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_PHN_MAXADDR, (void *)&in);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Camera-MinAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_CMR_MINADDR, (void *)&in);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Camera-MaxAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_CMR_MAXADDR, (void *)&in);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MinAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_PC_MINADDR, (void *)&in);
		return 0;
	}
	else if( strcmp( lastname, "X_CT-COM_Computer-MaxAddress" )==0 )
{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_PC_MAXADDR, (void *)&in);
		return 0;
}
	else if( strcmp( lastname, "X_CT-COM_HGW-MaxAddress" )==0 )
{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_HGW_MAXADDR, (void *)&in);

		return 0;
}
	else if( strcmp( lastname, "X_CT-COM_HGW-MinAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		mib_set(MIB_CT_HGW_MINADDR, (void *)&in);
		return 0;
#endif //#ifdef _PRMT_X_CT_COM_DHCP_
	}
	else if( strcmp( lastname, "ReservedAddresses" )==0 )
	{
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
		if( setDHCPReservedIPAddr(0,buf)<0 ) return ERR_9007;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif //_CWMP_APPLY_
#else
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
		return 0;
#endif //SUPPORT_DHCP_RESERVED_IPADDR
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		struct in_addr lan_mask;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;

			mib_set(MIB_SUBNET_MASK, (void *)&in);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		char *tok;
		if( ServerEn==0 ) return ERR_9001;
		if( (buf==NULL) || (strlen(buf)==0) )
		{ 	//automatically attain DNS
			vInt = 0;
			mib_set( MIB_DNS_MODE, (void *)&vInt);
		}
		else if( setDNSList( buf ) == 0 )
		{
			vInt = 1;
			mib_set( MIB_DNS_MODE, (void *)&vInt);
		}
		else
			 return ERR_9007;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_SL, apply_DNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DomainName" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		//if( ServerEn==0 ) return ERR_9001;
		//mib defines the length of domainname=30
		if( strlen(buf)>=30 ) return ERR_9001;
		mib_set(MIB_DOMAIN_NAME, (void *)buf);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "IPRouters" )==0 )
	{
		struct in_addr lan_ip;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( inet_aton( buf, &in )==0 ) return ERR_9007;		
		mib_set( MIB_DEFAULT_GATEWAY, (void *)&in);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "DHCPLeaseTime" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *pInt = &tmpint;
#else
		int *pInt = data;
#endif
		if(pInt==NULL) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if(*pInt<-1) return ERR_9007;
		mib_set(MIB_DHCP_LEASE_TIME, (void *)pInt);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "UseAllocatedWAN" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "Normal" )!=0 ) return ERR_9007;
		return 0;
	}
	else if( strcmp( lastname, "AssociatedConnection" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "" )!=0 ) return ERR_9001;
		return 0;
	}
	else if( strcmp( lastname, "PassthroughLease" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *pUint = &tmpuint;
#else
		unsigned int *pUint = data;
#endif
		if( pUint==NULL ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( *pUint!=0 ) return ERR_9001;
		return 0;
	}
	else if( strcmp( lastname, "PassthroughMACAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( ServerEn==0 ) return ERR_9001;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "" )!=0 ) return ERR_9001;
		return 0;
	}
#ifdef _CWMP_MAC_FILTER_
	else if( strcmp( lastname, "AllowedMACAddresses" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( setMACAddressList(buf)< 0) return ERR_9007;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_MACFILTER, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
#endif /*_CWMP_MAC_FILTER_*/
	else{
		return ERR_9005;
	}
	
	return 0;
}

int getLDEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LANEthernetInterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup(CWMP_LANETHIFNO);
	}else if( strcmp( lastname, "LANUSBInterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup(CWMP_LANUSBIFNO);
	}else if( strcmp( lastname, "LANWLANConfigurationNumberOfEntries" )==0 )
	{
		int num, i;
		CWMP_WLANCONF_T *pwlanConf, wlanconf_entity;
		int wlanCount = 0;
		
		mib_get(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);
		pwlanConf = &wlanconf_entity;
		for( i=1; i<=num;i++ )
		{
			*((char *)pwlanConf) = (char)i;
			if(!mib_get(MIB_CWMP_WLANCONF_TBL, (void *)pwlanConf))
				continue;

			if(pwlanConf->IsConfigured == 1 ) //skip root interface
				wlanCount++;
		}
	
		*data = uintdup( wlanCount );
		
		
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
int getPid(char *filename)
{
	struct stat status;
	char buff[100];
	FILE *fp;

	if ( stat(filename, &status) < 0)
		return -1;
	fp = fopen(filename, "r");
	if (!fp) {
        	//error(__FILE__, __LINE__, 0x2, "Read pid file error!\n");
		return -1;
   	}
	fgets(buff, 100, fp);
	fclose(fp);

	return (atoi(buff));
}
/*copy from fmmgmt.c in boa dir*/
static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

int getInterfaceStat(
	char *ifname,
	unsigned long *bs,
	unsigned long *br,
	unsigned long *ps,
	unsigned long *pr )
{
#if !defined(ECOS)
	int	ret=-1;
	FILE 	*fh;
	char 	buf[512];
	unsigned long rx_pkt, rx_err, rx_drop, tx_pkt, tx_err, tx_drop;

	if( (bs==NULL) || (br==NULL) || (ps==NULL) || (pr==NULL) )	return ret;
	*bs=0; *br=0; *ps=0; *pr=0;
	
	fh = fopen("/proc/net/dev", "r");
	if (!fh) return ret;
	
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);
	while (fgets(buf, sizeof buf, fh))
	{
		char *s, name[128];
		s = get_name(name, buf);    
		if (!strcmp(ifname, name)) {
			sscanf(s,
			"%lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu",
			br, pr, bs, ps);
			ret=0;
			break;
		}
	}
	fclose(fh);
	return ret;
#else
	struct user_net_device_stats stats;
	
	if ( getStats(ifname, &stats) != -1) {
		*bs = stats.tx_bytes;
		*br = stats.rx_bytes;
		*ps = stats.tx_packets;
		*pr = stats.rx_packets;
	}
#endif
}

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
int getInterfaceStat1(
	char *ifname,
	unsigned long *es, unsigned long *er,
	unsigned long *ups, unsigned long *upr, 
	unsigned long *dps, unsigned long *dpr, 
	unsigned long *mps, unsigned long *mpr, 
	unsigned long *bps, unsigned long *bpr, 
	unsigned long *uppr)
{
	int	ret=-1;
	FILE 	*fh;
	char 	buf[512];
	unsigned long rx_pkt, rx_err, rx_drop, tx_pkt, tx_err, tx_drop;

	if( (es==NULL) || (er==NULL) || (ups==NULL) || (upr==NULL) 
		|| (dps==NULL) || (dpr==NULL) || (mps==NULL) || (mpr==NULL)
		|| (bps==NULL) || (bpr==NULL) || (uppr==NULL))	
		return ret;
	*es=0; *er=0; *ups=0; *upr=0;
	*dps=0; *dpr=0; *mps=0; *mpr=0;
	*bps=0; *bpr=0; *uppr=0;
	
	fh = fopen("/proc/net/dev", "r");
	if (!fh) return ret;
	
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);
	while (fgets(buf, sizeof buf, fh))
	{
		char *s, name[128];
		s = get_name(name, buf);    
		if (!strcmp(ifname, name)) {
			sscanf(s,
			"%*lu %*lu %lu %lu %*lu %*lu %*lu %lu %*lu %*lu %lu %lu %*lu %*lu %*lu %*lu",
			er, dpr, mpr,es, dps);
			ret=0;
			break;
		}
	}
	fclose(fh);
	return ret;
}

#endif
/*ping_zhang:20081217 END*/
/*
int getMIBDefaultValue( int id, void *data )
{
	int i;
	
	if( data==NULL ) return -1;
	
	for (i=0; mib_table[i].id; i++)
		if ( mib_table[i].id == id ) break;

	return 0;
}
*/

int getDNSList( char *buf )
{
	//printf("<%s:%d>", __FUNCTION__, __LINE__);
	unsigned char tmp[64];
	char *zeroip="0.0.0.0";

	if( buf==NULL ) return -1;

	buf[0]=0;
	tmp[0]=0;
	getMIBtoStr(MIB_DNS1, tmp);
	if( (strlen(tmp)>0) && (strcmp(tmp, zeroip)!=0) )
		strcat( buf, tmp );
	
	tmp[0]=0;
	getMIBtoStr(MIB_DNS2, tmp);
	if( (strlen(tmp)>0) && (strcmp(tmp, zeroip)!=0) )
	{
		if( strlen(buf) > 0 )
			strcat( buf, ",");
		strcat( buf, tmp );
	}
	
	tmp[0]=0;
	getMIBtoStr(MIB_DNS3, tmp);
	if( (strlen(tmp)>0) && (strcmp(tmp, zeroip)!=0) )
	{
		if( strlen(buf) > 0 )
			strcat( buf, ",");
		strcat( buf, tmp );
	}
	
	return 0;
}

int setDNSList( char *buf)
{
	char *tok1, *tok2, *tok3;
	int  ret=0;
	struct in_addr in1,in2,in3,emp_in;

	if( buf==NULL ) return -1;
	tok1 = NULL;
	tok2 = NULL;
	tok3 = NULL;

	tok1=strtok( buf, "," );
	tok2=strtok( NULL, "," );
	tok3=strtok( NULL, "," );
	if( (tok1==NULL)&&(tok2==NULL)&&(tok3==NULL) )
		return -1;
	
	if(tok1) 
		if(  inet_aton( tok1, &in1 )==0  ) ret=-1;
	if(tok2) 
		if(  inet_aton( tok2, &in2 )==0  ) ret=-1;
	if(tok3) 
		if(  inet_aton( tok3, &in3 )==0  ) ret=-1;

	memset( &emp_in, 0, sizeof(struct in_addr) );
	if(ret==0)
	{
		if( tok1!=NULL )
			mib_set(MIB_DNS1, (void *)&in1);
		else
			mib_set(MIB_DNS1, (void *)&emp_in);
	
		if(tok2!=NULL)
			mib_set(MIB_DNS2, (void *)&in2);
		else
			mib_set(MIB_DNS2, (void *)&emp_in);
	
		if(tok3!=NULL)
			mib_set(MIB_DNS3, (void *)&in3);
		else
			mib_set(MIB_DNS3, (void *)&emp_in);
	}
	return ret;
}

#ifdef _CWMP_MAC_FILTER_
int getMACAddressList( char *buf, int len )
{
	int total,i;
	CWMP_MAC_FILTER_T MacEntry;
	
	if(buf==NULL || len<=0) return -1;
	buf[0]='\0';

	mib_get(MIB_CWMP_MACFILTER_TBL_NUM, (void *)&total);

	for (i = 1; i <= total; i++)
	{
		*((char *)&MacEntry) = (char)i;
		if ( !mib_get(MIB_CWMP_MACFILTER_TBL, (void *)&MacEntry))
			continue;
		//action==allow, dstMac==0, dir==outgoing
		if( (MacEntry.action==1)    && (MacEntry.dir==DIR_OUT) &&
		    (MacEntry.dstMac[0]==0) && (MacEntry.dstMac[1]==0) &&
		    (MacEntry.dstMac[2]==0) && (MacEntry.dstMac[3]==0) &&
		    (MacEntry.dstMac[4]==0) && (MacEntry.dstMac[5]==0)
		  )
		{
			char tmp[19];
			if(strlen(buf)+19>len) break;
			
			if(buf[0]) strcat(buf,",");
			snprintf( tmp, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				MacEntry.srcMac[0], MacEntry.srcMac[1],
				MacEntry.srcMac[2], MacEntry.srcMac[3],
				MacEntry.srcMac[4], MacEntry.srcMac[5]);
			strcat(buf,tmp);
	}
	}
	return 0;
}

int setMACAddressList( char *buf )
{
	int total,i;
	CWMP_MAC_FILTER_T MacEntry;
	
	if(buf==NULL) return -1;

	//clear all allow list entries
	mib_get(MIB_CWMP_MACFILTER_TBL_NUM, (void *)&total);
	for (i = 1; i <= total; i++)
	{
		*((char *)&MacEntry) = (char)i;
		if ( !mib_get(MIB_CWMP_MACFILTER_TBL, (void *)&MacEntry))
			continue;

		//action==allow, dstMac==0, dir==outgoing
		if( (MacEntry.action==1)    && (MacEntry.dir==DIR_OUT) &&
		    (MacEntry.dstMac[0]==0) && (MacEntry.dstMac[1]==0) &&
		    (MacEntry.dstMac[2]==0) && (MacEntry.dstMac[3]==0) &&
		    (MacEntry.dstMac[4]==0) && (MacEntry.dstMac[5]==0)
		  )
		{
			mib_set(MIB_CWMP_MACFILTER_DEL, (void *)&MacEntry);
		}
	}
	
	//set a new allow list
	{
		char *tok;
		
		tok = strtok( buf, ", \n\r" );
		while(tok!=NULL)
		{
			unsigned int m0,m1,m2,m3,m4,m5;
			if( sscanf( tok, "%x:%x:%x:%x:%x:%x", &m0, &m1, &m2, &m3, &m4, &m5 )==6 )
			{
				if( m0<256 && m1<256 && m2<256 && m3<256 && m4<256 && m5<256)
				{
					memset( &MacEntry, 0, sizeof( CWMP_MAC_FILTER_T ) );
					MacEntry.action=1;
					MacEntry.dir=DIR_OUT;
					MacEntry.srcMac[0]=(unsigned char)m0;
					MacEntry.srcMac[1]=(unsigned char)m1;
					MacEntry.srcMac[2]=(unsigned char)m2;
					MacEntry.srcMac[3]=(unsigned char)m3;
					MacEntry.srcMac[4]=(unsigned char)m4;
					MacEntry.srcMac[5]=(unsigned char)m5;
					mib_set(MIB_CWMP_MACFILTER_ADD, (void *)&MacEntry);
				}else
					return -1;
			}else
				return -1;
			
			
			tok=strtok( NULL, ", \n\r" );
		}
	}

	return 0;
}
#endif /*_CWMP_MAC_FILTER_*/




/*copy from fmdhcpd.c*/
#define _DHCPD_PID_FILE			"/var/run/udhcpd.pid"
	#define _DHCPD_LEASES_FILE		"/var/lib/misc/udhcpd.leases"

struct dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;       /* network order */
	u_int32_t expires;      /* host order */
};
int updateDHCPList( int *update )
{
	time_t	c_time=0;
	int 	pid;
	FILE 	*fp;
	struct stat status;

	if(update==NULL) return -1;

	*update = 0;
	c_time = time(NULL);
	if( c_time >= gDHCPUpdateTime+DHCPUPDATETIME )
	{
		*update = 1;
#if 1
		// siganl DHCP server to update lease file
		pid = getPid(_DHCPD_PID_FILE);
		if ( pid > 0)
			kill(pid, SIGUSR1);
#if 0
		usleep(1000);
#else
		int counter=0;
		for(counter=0;counter<=10000;counter++){}
#endif // temporary ! by cairui
		if ( stat(_DHCPD_LEASES_FILE, &status) < 0 )
				goto err;
		
		if(gDHCPHosts) free(gDHCPHosts);
		gDHCPHosts = malloc(status.st_size);
		if(gDHCPHosts==NULL) goto err;
		
		fp = fopen(_DHCPD_LEASES_FILE, "r");
		if ( fp == NULL ) goto err;
		fread(gDHCPHosts, 1, status.st_size, fp);
		fclose(fp);
		
		gDHCPTotalHosts = status.st_size / sizeof( struct dhcpOfferedAddr );
#else
{
		int num=0, i;
		
		num = time(NULL) %  10;
		gDHCPHosts = malloc( num*sizeof(struct dhcpOfferedAddr) );
		gDHCPTotalHosts = num;
		fprintf( stderr, "dhcp count=%d\n", num );
		for( i=0; i<num;i++ )
		{
			struct dhcpOfferedAddr *p;
			p = gDHCPHosts + i*sizeof(struct dhcpOfferedAddr);
			p->chaddr[0] = i;
			p->chaddr[1] = i;
			p->chaddr[2] = i;
			p->chaddr[3] = i;
			p->chaddr[4] = i;
			p->chaddr[5] = i;
			p->yiaddr = i;
			p->expires=5 +i;
		}
		
}
#endif
		gDHCPUpdateTime = c_time;
	}
	return 0;

err:
	if(gDHCPHosts)
	{
		free(gDHCPHosts);
		gDHCPHosts=NULL;
	}
	gDHCPTotalHosts=0;
	return -1;
}

int getDHCPClient( int id,  char *ip, char *mac, int *liveTime )
{
	struct dhcpOfferedAddr *p = NULL;
	//id starts from 0
	if( (id<0) || (id>=gDHCPTotalHosts) ) return -1;
	if( (ip==NULL) || (mac==NULL) || (liveTime)==0 ) return -1;
	if( (gDHCPHosts==NULL) || (gDHCPTotalHosts==0) ) return -1;
	
	p = (struct dhcpOfferedAddr *) (gDHCPHosts + id * sizeof( struct dhcpOfferedAddr ));
	strcpy(ip, inet_ntoa(*((struct in_addr *)&p->yiaddr)) );
	sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
			p->chaddr[0],p->chaddr[1],p->chaddr[2],
			p->chaddr[3],p->chaddr[4],p->chaddr[5]);

	*liveTime = (int)p->expires;
	return 0;
}

int updateDHCPTables(void)
{
	int ret=0;

	if( *gDHCPTable )
	{
		destroy_ParameterTable( (struct CWMP_NODE*)*gDHCPTable );
		*gDHCPTable = NULL;
	}
	
	if( gDHCPTotalHosts > 0 )
	{
		struct CWMP_LINKNODE **last_sibling;
		ret = create_Object( gDHCPTable, tHostObject, sizeof(tHostObject), gDHCPTotalHosts, 1 );
		
		last_sibling = gDHCPTable;
		while( *last_sibling!=NULL )
		{
			struct CWMP_NODE  *ntable;
			ntable = (*last_sibling)->next;
			init_ParameterTable( &(*last_sibling)->next, ntable, NULL);
			last_sibling = &(*last_sibling)->sibling;
		}
	}
	return 0;
}

int updateDHCP(void)
{
	int update;
	updateDHCPList(&update);
	if(update)
		updateDHCPTables();
	return 0;
}

//return -1:error, 0:not found, 1:found
int findDHCPStaticAssign( char *mac, char *ip )
{
	int i, total=0;
	int ret=0;
	DHCPRSVDIP_T Entry;
	char m[6];
	
	if( mac==NULL || ip==NULL ) return ret;
	if( sscanf( mac, "%x:%x:%x:%x:%x:%x", 
		&m[0],&m[1],&m[2],&m[3],&m[4],&m[5] )!=6 )
		return ret;
	
	mib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&total);
	for( i=1;i<=total;i++ )
	{


		*((char *)&Entry) = (char)i;
		if(!mib_get(MIB_DHCPRSVDIP_TBL, (void *)&Entry))
			continue;
		
		if( memcmp(Entry.macAddr, m, 6 )==0  && strcmp( inet_ntoa(*((struct in_addr *)Entry.ipAddr)), ip)==0 )
		{
			ret=1;
			break;
		}
	}
	
	return ret;	
}

#ifdef SECONDARY_IP
unsigned int getIPItfInstNum( char *name )
{
	return getInstNum( name, "IPInterface" );
}
#endif //SECONDARY_IP



#ifdef SUPPORT_DHCP_RESERVED_IPADDR
int getDHCPReservedIPAddr( unsigned int inst_num, char *pfilename )
{
	FILE *fp;
	int num,i,count=0;

	if(pfilename==NULL) return -1;
	fp=fopen( pfilename, "w" );
	if(!fp) return -1;

	mib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&num);
	for( i=1;i<=num;i++ )
	{
		DHCPRSVDIP_T Entry;
		
		*((char *)&Entry) = (char)i;
		mib_get(MIB_DHCPRSVDIP_TBL, (void *)&Entry);
		
		if( Entry.InstanceNum==inst_num )
		{
			//fprintf( stderr, "(get: inst_num=%u, ip=%d:%d:%d:%d)\n", Entry.InstanceNum, Entry.IPAddr[0], Entry.IPAddr[1], Entry.IPAddr[2], Entry.IPAddr[3] );
			count++;
			if(count>1) fprintf( fp, "," );
			fprintf( fp, "%s",  inet_ntoa(*((struct in_addr *)&(Entry.ipAddr))) );			
		}
	}
	fclose(fp);
	return 0;
}
int setDHCPReservedIPAddr( unsigned int inst_num, char *iplist )
{
	FILE *fp;
	int num,i, count=0;
	char buf[32];
	struct in_addr	inaddr;
	int intValue=0;
	fp=fopen( FILE4DHCPReservedIPAddr, "w" );
	if(!fp) return -1;
	if( iplist && strlen(iplist) )
	{
		char *tok;
		tok=strtok( iplist, ", " );
		while(tok)
		{
			int paser_error=0;

			if(paser_error==0 && (inet_aton(tok, &inaddr)==0) )
				paser_error=1;

			if(paser_error)
			{
				fclose(fp);
				unlink(FILE4DHCPReservedIPAddr);
				return -1;
			}

			fprintf( fp, "%s\n", tok );

			//next
			count++;
			tok=strtok( NULL, ", " );
		}
	}
	fclose(fp);

	//destroy allowlist
	mib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&num);
	if( num>0 )
	{

		for (i=1; i<=num; i++) {
			DHCPRSVDIP_T Entry;
			*((char *)&Entry) = (char)i;
			mib_get(MIB_DHCPRSVDIP_TBL, (void *)&Entry);
			if ( Entry.InstanceNum==inst_num )
				mib_set(MIB_DHCPRSVDIP_DEL,(void *)&Entry);
		}
	}

	//save the new allowlist
	fp=fopen( FILE4DHCPReservedIPAddr, "r" );
	if(!fp) return -1;
	while(fgets(buf, 32, fp))
	{
		char *p;
		DHCPRSVDIP_T Entry;
		memset(&Entry, 0x00, sizeof(DHCPRSVDIP_T));
		//fprintf( stderr, "set:buf=%s\n", buf );
		p=strchr( buf, '\n' );
		if(p) *p=0;
		inet_aton(buf, (struct in_addr *)Entry.ipAddr);
		Entry.InstanceNum=inst_num;
		//fprintf( stderr, "set:ipbuf=%s(%d,%d,%d,%d, inst_num=%u)\n", buf, Entry.IPAddr[0], Entry.IPAddr[1], Entry.IPAddr[2], Entry.IPAddr[3],Entry.InstanceNum );
		mib_set( MIB_DHCPRSVDIP_ADD, (unsigned char*)&Entry);
	}
	fclose(fp);

	unlink(FILE4DHCPReservedIPAddr);

	//default set ReservedIPAddr is Disabled from TR069, since the mac address is empty 
	//even IP Address is set
	mib_set(MIB_DHCPRSVDIP_ENABLED, (void *)&intValue);

	
	return 0;
}
#endif //SUPPORT_DHCP_RESERVED_IPADDR

