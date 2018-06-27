#include "tr181_device.h"
#include "tr181_deviceInfo.h"
#include "tr181_deviceInfoVendorLogFile.h"
#include "tr181_gatewayInfo.h"
#include "tr181_userInterface.h"
#include "tr181_mgmtServer.h"
#include "tr181_mgableDev.h"
#include "tr181_eth.h"
#include "tr181_ethIF.h"
#include "tr181_MoCA.h"
#include "tr181_MoCA_IF.h"
#include "tr181_wifi.h"
#include "tr181_wifiRadio.h"
#include "tr181_bridging.h"
#include "tr181_bridge.h"
#include "tr181_ip.h"
#include "tr181_ipIF.h"
#include "tr181_dhcpv4.h"
#include "tr181_dhcpv4Client.h"
#include "tr181_dhcpv6.h"
#include "tr181_dhcpv6Client.h"
#include "tr181_dns.h"
#include "tr181_dnsClient.h"
#include "tr181_lanConfigSecurity.h"



#define INTERFACESTACK_NUM	1	/* one instance of INTERFACESTACK */


/*******************************************************************************
DEVICE.ManagementServer.ManageableDevice Entity
*******************************************************************************/
struct CWMP_OP tIFStackEntityLeafOP = { getIFStackEntity, NULL }; // op not implement yet
struct CWMP_PRMT tIFStackEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"HigherLayer",	eCWMP_tSTRING,	CWMP_READ,	&tIFStackEntityLeafOP},
	{"LowerLayer",	eCWMP_tSTRING,	CWMP_READ,	&tIFStackEntityLeafOP},
	{"HigherAlias",	eCWMP_tSTRING,	CWMP_READ,	&tIFStackEntityLeafOP},
	{"LowerAlias",	eCWMP_tSTRING,	CWMP_READ,	&tIFStackEntityLeafOP},
};
enum eIFStackEntityLeaf
{
	eIFStackHigherLayer,
	eIFStackLowerLayer,
	eMIFStackHigherAlias,
	eIFStackLowerAlias
};
struct CWMP_LEAF tIFStackEntityLeaf[] =
{
	{ &tIFStackEntityLeafInfo[eIFStackHigherLayer] },
	{ &tIFStackEntityLeafInfo[eIFStackLowerLayer] },
	{ &tIFStackEntityLeafInfo[eMIFStackHigherAlias] },
	{ &tIFStackEntityLeafInfo[eIFStackLowerAlias] },
	{ NULL }
};

/*******************************************************************************
DEVICE.InterfaceStack LINKNODE
*******************************************************************************/
struct CWMP_PRMT tLinkIFStackOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eLinkIFStackvOjbect
{
	eLinkIFStack0
};
struct CWMP_LINKNODE tLinkIFStackObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tLinkIFStackOjbectInfo[eLinkIFStack0],	tIFStackEntityLeaf,	NULL,	NULL,	0},
};

/*******************************************************************************
Second Level
*******************************************************************************/
struct CWMP_OP tDev_OP = { NULL, objIFStack };
struct CWMP_PRMT tDevObjectInfo[] =
{
	/*(name,			type,		flag,		op)*/
	{"Services",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"DeviceInfo",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"ManagementServer",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"GatewayInfo",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"UserInterface",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
//	{"InterfaceStack",		eCWMP_tOBJECT,	CWMP_READ,	&tDev_OP}, // op not implement yet
	{"Ethernet",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
//	{"MoCA",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"WiFi",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"Bridging",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"IP",					eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"DHCPv4",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef CONFIG_IPV6
	{"DHCPv6",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
	{"DNS",					eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
	{"LANConfigSecurity",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDevObject
{
	eServices,
	eDeviceInfo,
	eManagementServer,
	eGatewayInfo,
	eUserInterface,
//	eInterfaceStack,
	eEthernet,
//	eMoCA,
	eWiFi,
	eBridging,
	eIP,
	eDHCPv4,
#ifdef CONFIG_IPV6
	eDHCPv6,
	eDNS,
#endif
	eLANCfgSecurity
};

struct CWMP_NODE tDevObject[] =
{
	/*info,  				leaf,			next)*/
	{&tDevObjectInfo[eServices],		NULL,	NULL},
	{&tDevObjectInfo[eDeviceInfo],		&tDevInfoLeaf,	&tVendorLogFileObject},
	{&tDevObjectInfo[eManagementServer],&tMgmtServerLeaf,	&tMgableDevObject},
	{&tDevObjectInfo[eGatewayInfo],		&tGatewayInfoLeaf,	NULL},
	{&tDevObjectInfo[eUserInterface],	&tUserInterfaceLeaf,	NULL},
//	{&tDevObjectInfo[eInterfaceStack],	NULL,	NULL},
	{&tDevObjectInfo[eEthernet],		&tEthLeaf,	&tEthIFObject},
//	{&tDevObjectInfo[eMoCA],			&tMoCALeaf,	&tMoCAIFObject},
	{&tDevObjectInfo[eWiFi],			&tWifiLeaf,	&tWifiObject},
	{&tDevObjectInfo[eBridging],		&tBridgingLeaf,	&tBrObject},
	{&tDevObjectInfo[eIP],				&tIpLeaf,	&tIpIFObject},
	{&tDevObjectInfo[eDHCPv4],			&tDhcpv4Leaf,	&tDhcpv4ClientObject},
#ifdef CONFIG_IPV6
	{&tDevObjectInfo[eDHCPv6],			&tDhcpv6Leaf,	&tDhcpv6ClientObject},
	{&tDevObjectInfo[eDNS],				&tDNSLeaf,	&tDNSClientObject},
#endif
	{&tDevObjectInfo[eLANCfgSecurity],	&tLANCfgSecurityLeaf,	NULL},
	{NULL,					NULL,			NULL}
};

/*******************************************************************************
DEVICE.InterfaceStackNumberOfEntries
*******************************************************************************/
struct CWMP_OP tDevLeafOP = { getDev, NULL };
struct CWMP_PRMT tDevLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"InterfaceStackNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tDevLeafOP},
};

enum eDevLeaf
{
	eInterfaceStackNumberOfEntries
};

struct CWMP_LEAF tDevLeaf[] =
{
	{ &tDevLeafInfo[eInterfaceStackNumberOfEntries]  },
	{ NULL	}
};

/*******************************************************************************
TR181 ROOT DEVICE.
*******************************************************************************/
struct CWMP_PRMT tDevROOTObjectInfo[] =
{
	/*(name,			type,		flag,		op)*/
	{"Device",	eCWMP_tOBJECT,	CWMP_READ,	NULL}
};

enum eDevROOTObject
{
	eDevice
};

struct CWMP_NODE tDevROOT[] =
{
	/*info, 	 				leaf,			next*/
	{&tDevROOTObjectInfo[eDevice],	tDevLeaf,		tDevObject	},
	{NULL,						NULL,			NULL		}
};


/*******************************************************************************
Function
*******************************************************************************/
int getDev(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "InterfaceStackNumberOfEntries" )==0 )
	{
		*data = uintdup( INTERFACESTACK_NUM ); 
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int objIFStack(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	return 0;
}

int getIFStackEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	return 0;
}

unsigned int getInstanceNum(char *name, char *objname)
{
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

char *get_name(char *name, char *p)
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


