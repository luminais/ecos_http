/* project: miniupnp
 * webpage: http://miniupnp.free.fr/
 * (c) 2006 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file provided with this distribution */
/* $Id: upnpdescgen.c,v 1.2 2007-12-03 02:26:23 bradhuang Exp $ */
#ifdef __ECOS
#include <network.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "upnpdescgen.h"
#include "miniupnpdpath.h"
#include "upnpglobalvars.h"
#include <sys/stat.h>
#ifndef __ECOS
#include <syslog.h>
#endif
#ifdef 	ECOS_DBG_STAT
#include "../system/sys_utility.h"
#endif

#ifdef	ECOS_DBG_STAT
extern int	dbg_igd_index;
#endif

#ifdef ENABLE_EVENTS
#include "getifaddr.h"
#include "upnpredirect.h"
#include <netinet/in.h>

#define PossibleConnectionTypes_EVENTS 255
#define ConnectionStatus_EVENTS 254
#define ExternalIPAddress_EVENTS 253
#define PhysicalLinkStatus_EVENTS 252
#endif
extern char picsdesc_xml[];
static const char * const upnptypes[] =
{
	"string",
	"boolean",
	"ui2",
	"ui4"
};

static const char * const upnpdefaultvalues[] =
{
	0,
	"Unconfigured"
};

static const char * const upnpallowedvalues[] =
{
	0,		/* 0 */
	"DSL",	/* 1 */
	"POTS",
	"Cable",
	"Ethernet",
	0,
	"Up",	/* 6 */
	"Down",
	"Initializing",
	"Unavailable",
	0,
	"TCP",	/* 11 */
	"UDP",
	0,
	"Unconfigured",	/* 14 */
	"IP_Routed",
	"IP_Bridged",
	0,
	"Unconfigured",	/* 18 */
	"Connecting",
	"Connected",
	"PendingDisconnect",
	"Disconnecting",
	"Disconnected",
	0,
	"ERROR_NONE",	/* 25 */
	0,
	"",		/* 27 */
	0
};

static const char xmlver[] = "<?xml version=\"1.0\"?>\n";
/* static const char ns_service[] = "urn:schemas-upnp-org:service-1-0"; */
static const char root_service[] =
	"scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"";
/* static const char ns_device[] = "urn:schemas-upnp-org:device-1-0"; */
static const char root_device[] = 
	"root xmlns=\"urn:schemas-upnp-org:device-1-0\"";

/* this string is not constant ! */
/* static char uuid[] = "uuid:00000000-0000-0000-0000-000000000000";
*/

/* root Description of the UPnP Device 
 * fixed to match UPnP_IGD_InternetGatewayDevice 1.0.pdf */
static const struct XMLElt rootDesc[] =
{
/* 0 */
	{root_device, INITHELPER(1,2)},
	{"specVersion", INITHELPER(3,2)},
	{"device", INITHELPER(5,9)},
	{"/major", "1"},
	{"/minor", "0"},
	{"/deviceType", "urn:schemas-upnp-org:device:InternetGatewayDevice:1"},
	{"/friendlyName", "Internet Gateway Device"},	/* required */
	{"/manufacturer", "Realtek Semiconductor"},		/* required */
/* 8 */
	{"/manufacturerURL", "http://www.realtek.com.tw/"},	/* optional */
	/* presentationURL */
	/* modelDescription = recommended */
	{"/modelName", "RTL8186"},	/* required */
	{"/UDN", uuidvalue},	/* required */
	{"serviceList", INITHELPER(53,1)},
	{"deviceList", INITHELPER(14,1)},
	{"/presentationURL", presentationurl},	/* Recommended */
	{"device", INITHELPER(15,13)},
	{"/deviceType", "urn:schemas-upnp-org:device:WANDevice:1"}, /* Required */
	{"/friendlyName", "WANDevice"},
	{"/manufacturer", "miniupnp"},
	{"/manufacturerURL", "http://www.realtek.com.tw/"},
	{"/modelDescription" , "WAN Device"},
	{"/modelName", "WAN Device"},
	{"/modelNumber", "1"},
	{"/modelURL", "http://www.realtek.com.tw/"},
	{"/serialNumber", "00000000"},
	{"/UDN", uuidvalue},
	{"/UPC", "MINIUPNPD"},
	{"serviceList", INITHELPER(28,1)},
	{"deviceList", INITHELPER(34,1)},
	{"service", INITHELPER(29,5)},
	{"/serviceType",
			"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1"},
	/*{"/serviceId", "urn:upnp-org:serviceId:WANCommonInterfaceConfig"}, */
	{"/serviceId", "urn:upnp-org:serviceId:WANCommonIFC1"}, /* required */
	{"/controlURL", WANCFG_CONTROLURL},
	{"/eventSubURL", WANCFG_EVENTURL},
	{"/SCPDURL", WANCFG_PATH},
	{"device", INITHELPER(35,12)},
	{"/deviceType", "urn:schemas-upnp-org:device:WANConnectionDevice:1"},
	{"/friendlyName", "WANConnectionDevice"},
	{"/manufacturer", "Realtek"},
	{"/manufacturerURL", "http://www.realtek.com.tw/"},
	{"/modelDescription", "miniupnp daemon"},
	{"/modelName", "miniupnpd"},
	{"/modelNumber", "1"},
	{"/modelURL", "http://www.realtek.com.tw/"},
	{"/serialNumber", "00000000"},
	{"/UDN", uuidvalue},
	{"/UPC", "MINIUPNPD"},
	{"serviceList", INITHELPER(47,1)},
	{"service", INITHELPER(48,5)},
	{"/serviceType", "urn:schemas-upnp-org:service:WANIPConnection:1"},
/*	{"/serviceId", "urn:upnp-org:serviceId:WANIPConnection"}, */
	{"/serviceId", "urn:upnp-org:serviceId:WANIPConn1"},
	{"/controlURL", WANIPC_CONTROLURL},
	{"/eventSubURL", WANIPC_EVENTURL},
	{"/SCPDURL", WANIPC_PATH},
	{"service", INITHELPER(54,5)},
	{"/serviceType", "urn:schemas-dummy-com:service:Dummy:1"},
	{"/serviceId", "urn:dummy-com:serviceId:dummy1"},
	{"/controlURL", "/dummy"},
	{"/eventSubURL", "/dummy"},
	{"/SCPDURL", DUMMY_PATH},
	{0, 0}
};

/* WANIPCn.xml */
/* see UPnP_IGD_WANIPConnection 1.0.pdf
static struct XMLElt scpdWANIPCn[] =
{
	{root_service, {INITHELPER(1,2)}},
	{0, {0}}
};
*/
static const struct argument AddPortMappingArgs[] =
{
	{"NewRemoteHost", 1, 11},
	{"NewExternalPort", 1, 12},
	{"NewProtocol", 1, 14},
	{"NewInternalPort", 1, 13},
	{"NewInternalClient", 1, 15},
	{"NewEnabled", 1, 9},
	{"NewPortMappingDescription", 1, 16},
	{"NewLeaseDuration", 1, 10},
	{0, 0, 0}
};

static const struct argument GetExternalIPAddressArgs[] =
{
	{"NewExternalIPAddress", 2, 7},
	{0, 0, 0}
};

static const struct argument DeletePortMappingArgs[] = 
{
	{"NewRemoteHost", 1, 11},
	{"NewExternalPort", 1, 13},
	{"NewProtocol", 1, 14},
	{0, 0, 0}
};

static const struct argument SetConnectionTypeArgs[] =
{
	{"NewConnectionType", 1, 0},
	{0, 0, 0}
};

static const struct argument GetConnectionTypeInfoArgs[] =
{
	{"NewConnectionType", 2, 0},
	{"NewPossibleConnectionTypes", 2, 1},
	{0, 0, 0}
};

static const struct argument GetStatusInfoArgs[] =
{
	{"NewConnectionStatus", 2, 2},
	{"NewLastConnectionError", 2, 4},
	{"NewUptime", 2, 3},
	{0, 0, 0}
};

static const struct argument GetNATRSIPStatusArgs[] =
{
	{"NewRSIPAvailable", 2, 5},
	{"NewNATEnabled", 2, 6},
	{0, 0, 0}
};

static const struct argument GetGenericPortMappingEntryArgs[] =
{
	{"NewPortMappingIndex", 1, 8},
	{"NewRemoteHost", 2, 11},
	{"NewExternalPort", 2, 12},
	{"NewProtocol", 2, 14},
	{"NewInternalPort", 2, 13},
	{"NewInternalClient", 2, 15},
	{"NewEnabled", 2, 9},
	{"NewPortMappingDescription", 2, 16},
	{"NewLeaseDuration", 2, 10},
	{0, 0, 0}
};

static const struct argument GetSpecificPortMappingEntryArgs[] =
{
	{"NewRemoteHost", 1, 11},
	{"NewExternalPort", 1, 12},
	{"NewProtocol", 1, 14},
	{"NewInternalPort", 2, 13},
	{"NewInternalClient", 2, 15},
	{"NewEnabled", 2, 9},
	{"NewPortMappingDescription", 2, 16},
	{"NewLeaseDuration", 2, 10},
	{0, 0, 0}
};

#ifdef CONFIG_RTL8186_GR
static const struct argument GetWarnDisconnectDelayArgs[] =
{
	{"NewWarnDisconnectDelay", 2, 17},
	{0, 0, 0}
};
#endif

static const struct action WANIPCnActions[] =
{
	{"AddPortMapping", AddPortMappingArgs}, /* R */
	{"GetExternalIPAddress", GetExternalIPAddressArgs}, /* R */
	{"DeletePortMapping", DeletePortMappingArgs}, /* R */
	{"SetConnectionType", SetConnectionTypeArgs}, /* R */
	{"GetConnectionTypeInfo", GetConnectionTypeInfoArgs}, /* R */
	{"RequestConnection", 0}, /* R */
	{"ForceTermination", 0}, /* R */
	{"GetStatusInfo", GetStatusInfoArgs}, /* R */
	{"GetNATRSIPStatus", GetNATRSIPStatusArgs}, /* R */
	{"GetGenericPortMappingEntry", GetGenericPortMappingEntryArgs}, /* R */
	{"GetSpecificPortMappingEntry", GetSpecificPortMappingEntryArgs}, /* R */
#ifdef CONFIG_RTL8186_GR
	{"GetWarnDisconnectDelay", GetWarnDisconnectDelayArgs}, /* O */
	{"RequestTermination", 0}, /* O */
#endif
	{0, 0}
};
/* R=Required, O=Optional */

static const struct stateVar WANIPCnVars[] =
{
	{"ConnectionType", 0, 0/*1*/}, /* required */
		
#ifdef ENABLE_EVENTS		
	{"PossibleConnectionTypes", 0|0x80, 0, 14, PossibleConnectionTypes_EVENTS},
#else
	{"PossibleConnectionTypes", 0|0x80, 0, 14},
#endif

	 /* Required
	  * Allowed values : Unconfigured / IP_Routed / IP_Bridged */
#ifdef ENABLE_EVENTS			  
	{"ConnectionStatus", 0|0x80, 0/*1*/, 18, ConnectionStatus_EVENTS}, /* required */
#else
	{"ConnectionStatus", 0|0x80, 0/*1*/, 18}, /* required */
#endif
	 /* Allowed Values : Unconfigured / Connecting(opt) / Connected
	  *                  PendingDisconnect(opt) / Disconnecting (opt)
	  *                  Disconnected */
	{"Uptime", 3, 0},	/* Required */
	{"LastConnectionError", 0, 0, 25},	/* required : */
	 /* Allowed Values : ERROR_NONE(req) / ERROR_COMMAND_ABORTED(opt)
	  *                  ERROR_NOT_ENABLED_FOR_INTERNET(opt)
	  *                  ERROR_USER_DISCONNECT(opt)
	  *                  ERROR_ISP_DISCONNECT(opt)
	  *                  ERROR_IDLE_DISCONNECT(opt)
	  *                  ERROR_FORCED_DISCONNECT(opt)
	  *                  ERROR_NO_CARRIER(opt)
	  *                  ERROR_IP_CONFIGURATION(opt)
	  *                  ERROR_UNKNOWN(opt) */
	{"RSIPAvailable", 1, 0}, /* required */
	{"NATEnabled", 1, 0},    /* required */
	
#ifdef ENABLE_EVENTS		
	{"ExternalIPAddress", 0|0x80, 0,0,ExternalIPAddress_EVENTS}, /* required. Default : empty string */
#else
	{"ExternalIPAddress", 0|0x80, 0,0}, /* required. Default : empty string */
#endif

	{"PortMappingNumberOfEntries", 2, 0,0}, /* required >= 0 */
	{"PortMappingEnabled", 1, 0}, /* Required */
	{"PortMappingLeaseDuration", 3, 0}, /* required */
	{"RemoteHost", 0, 0},   /* required. Default : empty string */
	{"ExternalPort", 2, 0}, /* required */
	{"InternalPort", 2, 0}, /* required */
	{"PortMappingProtocol", 0, 0, 11}, /* required allowedValues: TCP/UDP */
	{"InternalClient", 0, 0}, /* required */
	{"PortMappingDescription", 0, 0}, /* required default: empty string */
#ifdef CONFIG_RTL8186_GR
	{"WarnDisconnectDelay", 3, 0}, /* Optional */
#endif
	{0, 0, 0}
};

static const struct serviceDesc scpdWANIPCn =
{ WANIPCnActions, WANIPCnVars };

/* WANCfg.xml */
/* See UPnP_IGD_WANCommonInterfaceConfig 1.0.pdf */

static const struct argument GetCommonLinkPropertiesArgs[] =
{
	{"NewWANAccessType", 2, 0},
	{"NewLayer1UpstreamMaxBitRate", 2, 1},
	{"NewLayer1DownstreamMaxBitRate", 2, 2},
	{"NewPhysicalLinkStatus", 2, 3},
	{0, 0, 0}
};

static const struct argument GetTotalBytesSentArgs[] =
{
	{"NewTotalBytesSent", 2, 4},
	{0, 0, 0}
};

static const struct argument GetTotalBytesReceivedArgs[] =
{
	{"NewTotalBytesReceived", 2, 5},
	{0, 0, 0}
};

static const struct argument GetTotalPacketsSentArgs[] =
{
	{"NewTotalPacketsSent", 2, 6},
	{0, 0, 0}
};

static const struct argument GetTotalPacketsReceivedArgs[] =
{
	{"NewTotalPacketsReceived", 2, 7},
	{0, 0, 0}
};

static const struct action WANCfgActions[] =
{
	{"GetCommonLinkProperties", GetCommonLinkPropertiesArgs}, /* Required */
	{"GetTotalBytesSent", GetTotalBytesSentArgs},             /* optional */
	{"GetTotalBytesReceived", GetTotalBytesReceivedArgs},     /* optional */
	{"GetTotalPacketsSent", GetTotalPacketsSentArgs},         /* optional */
	{"GetTotalPacketsReceived", GetTotalPacketsReceivedArgs}, /* optional */
	{0, 0}
};

/* See UPnP_IGD_WANCommonInterfaceConfig 1.0.pdf */
static const struct stateVar WANCfgVars[] =
{
	{"WANAccessType", 0, 0, 1},
	/* Allowed Values : DSL / POTS / Cable / Ethernet 
	 * Default value : empty string */
	{"Layer1UpstreamMaxBitRate", 3, 0},
	{"Layer1DownstreamMaxBitRate", 3, 0},
	
#ifdef ENABLE_EVENTS			
	{"PhysicalLinkStatus", 0|0x80, 0, 6, PhysicalLinkStatus_EVENTS},
#else
	{"PhysicalLinkStatus", 0|0x80, 0, 6},
#endif

	/*  allowed values : 
	 *      Up / Down / Initializing (optional) / Unavailable (optionnal)
	 *  no Default value 
	 *  Evented */
	{"TotalBytesSent", 3, 0},	   /* Optional */
	{"TotalBytesReceived", 3, 0},  /* Optional */
	{"TotalPacketsSent", 3, 0},    /* Optional */
	{"TotalPacketsReceived", 3, 0},/* Optional */
	/*{"MaximumActiveConnections", 2, 0},	// allowed Range value // OPTIONAL */
	{0, 0, 0}
};

static const struct serviceDesc scpdWANCfg =
{ WANCfgActions, WANCfgVars };

/* recursive sub routine */
void DisplayXML(const struct XMLElt * p, int i)
{
	int j, k;
	const char * eltname;

	eltname = p[i].eltname;
	if(eltname[0] == '/')
	{
		printf("<%s>%s<%s>\n", eltname+1,
				p[i].data, eltname);
	}
	else
	{
		/*j = p[i].index; */
		/*k = j + p[i].nchild; */
		j = (unsigned)p[i].data & 0xffff;
		k = j + ((unsigned)p[i].data >> 16);
		if(j == k)
		{
			printf("<%s/>\n", eltname);
		}
		else
		{
			printf("<%s>\n", eltname);
			while(j < k)
				DisplayXML(p, j++);
			printf("</%s>\n", eltname);
		}
	}
}

void DisplayRootDesc(void)
{
	printf("%s", xmlver);
	DisplayXML(rootDesc, 0);
}

static char * strcat_str(char * str, int * len, int * tmplen, const char * s2)
{
	int s2len;
	s2len = (int)strlen(s2);
	if(*tmplen <= (*len + s2len))
	{
		if(s2len < 256)
			*tmplen += 256;
		else
			*tmplen += s2len;
		str = (char *)realloc(str, *tmplen);
		if(str == NULL)
			return NULL;
	}
	/*strcpy(str + *len, s2); */
	memcpy(str + *len, s2, s2len + 1);
	*len += s2len;
	return str;
}

static char * strcat_char(char * str, int * len, int * tmplen, char c)
{
	if(*tmplen <= (*len + 1))
	{
		*tmplen += 256;
		str = (char *)realloc(str, *tmplen);
		if(str == NULL)
			return NULL;
	}
	str[*len] = c;
	(*len)++;
	return str;
}

#if 0
/* iterative subroutine using a stack*/
static char * genXML(char * str, int * len, int * tmplen,
                   const struct XMLElt * p)
{
	unsigned short i, j, k;
	int top;
	const char * eltname, *s;
	char c;
	struct {
		unsigned short i;
		unsigned short j;
		const char * eltname;
	} pile[16]; /* stack */
	top = -1;
	i = 0;	/* current node */
	j = 1;	/* i + number of nodes*/
	for(;;)
	{
		eltname = p[i].eltname;
		if(!eltname)
			return str;
		if(eltname[0] == '/')
		{
			/*printf("<%s>%s<%s>\n", eltname+1, p[i].data, eltname); */
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname+1);
			str = strcat_char(str, len, tmplen, '>');
			str = strcat_str(str, len, tmplen, p[i].data);
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname);
			str = strcat_char(str, len, tmplen, '>');
			for(;;)
			{
				if(top < 0)
					return str;
				i = ++(pile[top].i);
				j = pile[top].j;
				/*printf("  pile[%d]\t%d %d\n", top, i, j); */
				if(i==j)
				{
					/*printf("</%s>\n", pile[top].eltname); */
					str = strcat_char(str, len, tmplen, '<');
					str = strcat_char(str, len, tmplen, '/');
					s = pile[top].eltname;
					for(c = *s; c > ' '; c = *(++s))
						str = strcat_char(str, len, tmplen, c);
					str = strcat_char(str, len, tmplen, '>');
					top--;
				}
				else
					break;
			}
		}
		else
		{
			/*printf("<%s>\n", eltname); */
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname);
			str = strcat_char(str, len, tmplen, '>');
			k = i;
			/*i = p[k].index; */
			/*j = i + p[k].nchild; */
			i = (unsigned)p[k].data & 0xffff;
			j = i + ((unsigned)p[k].data >> 16);
			top++;
			/*printf(" +pile[%d]\t%d %d\n", top, i, j); */
			pile[top].i = i;
			pile[top].j = j;
			pile[top].eltname = eltname;
		}
	}
}
#endif

char *miniigd_UPnP_UploadXML(char *file_path)
{
        FILE *fp = NULL;
        int retVal = 0;
        struct stat file_info;
        char *buf=NULL;
        unsigned int fileLen=0;
        unsigned int num_read=0;
        char *membuf=NULL;

        buf = file_path;
        retVal = stat(buf, &file_info );
        if ( retVal == -1 ) {
                return NULL;
	        }

        fileLen = file_info.st_size;
        if ( ( fp = fopen( buf, "rb" ) ) == NULL ) {
	        return NULL;
	        }
	

        if ( ( membuf = ( char * )malloc( fileLen + 1 ) ) == NULL ) {
	        fclose( fp );
	        return NULL;
        	}

        num_read = fread( membuf, 1, fileLen, fp );
        if ( num_read != fileLen ) {
                fclose( fp );
                free( membuf );
                return NULL;
        }

        membuf[fileLen] = '\0';

        fclose( fp );

        return membuf;
}
/* genRootDesc() :
 * - Generate the root description of the UPnP device.
 * - the len argument is used to return the length of
 *   the returned string.
 * - tmp_uuid argument is used to build the uuid string */

char * genRootDesc(int * len)
{
	char * str;
	char * buf;
#ifndef __ECOS
	char file_path[40];
	strcpy(file_path,"/etc/linuxigd/picsdesc.xml");

	buf = miniigd_UPnP_UploadXML(file_path);
#endif
	buf = picsdesc_xml;
	str = (char *)malloc(strlen(buf)+1);
	if(str==NULL)
		return NULL;
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 2048);
#endif
	*len=strlen(buf);
	memcpy(str,buf,*len);
#ifndef __ECOS
    free(buf);
#endif
	return str;
}



/* genRootDesc() :
 * - Generate the root description of the UPnP device.
 * - the len argument is used to return the length of
 *   the returned string. 
 * - tmp_uuid argument is used to build the uuid string */
/*
char * genRootDesc(int * len)
{
	char * str;
	int tmplen;
	str = (char *)malloc(2048);
	tmplen = 2048;
	* len = strlen(xmlver);
	//strcpy(str, xmlver); 
	memcpy(str, xmlver, *len + 1);
	str = genXML(str, len, &tmplen, rootDesc);
	str[*len] = '\0';
	return str;
}
*/

static char * genServiceDesc(int * len, const struct serviceDesc * s)
{
	int i, j;
	const struct action * acts;
	const struct stateVar * vars;
	const struct argument * args;
	char * str;
	int tmplen;
	tmplen = 2048;
	str = (char *)malloc(tmplen);
	if(str ==NULL)
		return NULL;
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 2048);
#endif
	/*strcpy(str, xmlver); */
	*len = strlen(xmlver);
	memcpy(str, xmlver, *len + 1);
	
	acts = s->actionList;
	vars = s->serviceStateTable;

	str = strcat_char(str, len, &tmplen, '<');
	str = strcat_str(str, len, &tmplen, root_service);
	str = strcat_char(str, len, &tmplen, '>');

	str = strcat_str(str, len, &tmplen,
		"<specVersion><major>1</major><minor>0</minor></specVersion>");

	i = 0;
	str = strcat_str(str, len, &tmplen, "<actionList>");
	while(acts[i].name)
	{
		str = strcat_str(str, len, &tmplen, "<action><name>");
		str = strcat_str(str, len, &tmplen, acts[i].name);
		str = strcat_str(str, len, &tmplen, "</name>");
		/* argument List */
		args = acts[i].args;
		if(args)
		{
			str = strcat_str(str, len, &tmplen, "<argumentList>");
			j = 0;
			while(args[j].name)
			{
				str = strcat_str(str, len, &tmplen, "<argument><name>");
				str = strcat_str(str, len, &tmplen, args[j].name);
				str = strcat_str(str, len, &tmplen, "</name><direction>");
				str = strcat_str(str, len, &tmplen, args[j].dir==1?"in":"out");
				str = strcat_str(str, len, &tmplen,
						"</direction><relatedStateVariable>");
				str = strcat_str(str, len, &tmplen,
						vars[args[j].relatedVar].name);
				str = strcat_str(str, len, &tmplen,
						"</relatedStateVariable></argument>");
				j++;
			}
			str = strcat_str(str, len, &tmplen,"</argumentList>");
		}
		str = strcat_str(str, len, &tmplen, "</action>");
		/*str = strcat_char(str, len, &tmplen, '\n'); // TEMP ! */
		i++;
	}
	str = strcat_str(str, len, &tmplen, "</actionList><serviceStateTable>");
	i = 0;
	while(vars[i].name)
	{
		str = strcat_str(str, len, &tmplen,
				"<stateVariable sendEvents=\"");
		/* for the moment allways send no. Wait for SUBSCRIBE implementation
		 * before setting it to yes */
#ifdef ENABLE_EVENTS
		str = strcat_str(str, len, &tmplen, (vars[i].itype & 0x80)?"yes":"no");
#else
					/* for the moment allways send no. Wait for SUBSCRIBE implementation
					 * before setting it to yes */
		str = strcat_str(str, len, &tmplen, "no");
#endif

		str = strcat_str(str, len, &tmplen, "\"><name>");
		str = strcat_str(str, len, &tmplen, vars[i].name);
		str = strcat_str(str, len, &tmplen, "</name><dataType>");
		/*str = strcat_str(str, len, &tmplen, vars[i].dataType); */
		str = strcat_str(str, len, &tmplen, upnptypes[vars[i].itype & 0x0f]);
		str = strcat_str(str, len, &tmplen, "</dataType>");
		if(vars[i].iallowedlist)
		{
		  str = strcat_str(str, len, &tmplen, "<allowedValueList>");
		  for(j=vars[i].iallowedlist; upnpallowedvalues[j]; j++)
		  {
		    str = strcat_str(str, len, &tmplen, "<allowedValue>");
		    str = strcat_str(str, len, &tmplen, upnpallowedvalues[j]);
		    str = strcat_str(str, len, &tmplen, "</allowedValue>");
		  }
		  str = strcat_str(str, len, &tmplen, "</allowedValueList>");
		}
		/*if(vars[i].defaultValue) */
		if(vars[i].idefault)
		{
		  str = strcat_str(str, len, &tmplen, "<defaultValue>");
		  /*str = strcat_str(str, len, &tmplen, vars[i].defaultValue); */
		  str = strcat_str(str, len, &tmplen, upnpdefaultvalues[vars[i].idefault]);
		  str = strcat_str(str, len, &tmplen, "</defaultValue>");
		}
		str = strcat_str(str, len, &tmplen, "</stateVariable>");
		/*str = strcat_char(str, len, &tmplen, '\n'); // TEMP ! */
		i++;
	}
	str = strcat_str(str, len, &tmplen, "</serviceStateTable></scpd>");
	str[*len] = '\0';
	return str;
}


char * genWANIPCn(int * len)
{
	return genServiceDesc(len, &scpdWANIPCn);
}



char * genWANCfg(int * len)
{
	return genServiceDesc(len, &scpdWANCfg);
}



void DisplayService(const struct serviceDesc * s)
{
	int i, j;
	const struct action * acts;
	const struct stateVar * vars;
	const struct argument * args;
	printf("%s", xmlver);
	printf("<%s>\n<specVersion><major>1</major>"
	       "<minor>0</minor></specVersion>\n", root_service);
	acts = s->actionList;
	vars = s->serviceStateTable;
	i = 0;
	printf("<actionList>\n");
	while(acts[i].name)
	{
		printf("  <action>\n");
		printf("    <name>%s</name>\n", acts[i].name);
		/* argument List */
		args = acts[i].args;
		if(args)
		{
			printf("    <argumentList>\n");
			j = 0;
			while(args[j].name)
			{
				printf("      <argument>\n");
				printf("        <name>%s</name>\n", args[j].name);
				printf("        <direction>%s</direction>\n", 
				       args[j].dir==1?"in":"out");
				printf("        <relatedStateVariable>%s</relatedStateVariable>\n",
				       vars[args[j].relatedVar].name);
				printf("      </argument>\n");
				j++;
			}
			printf("    </argumentList>\n");
		}
		printf("  </action>\n");
		i++;
	}
	printf("</actionList>\n");
	printf("<serviceStateTable>\n");
	i = 0;
	while(vars[i].name)
	{
		printf("  <stateVariable sendEvents=\"%s\">\n", "no");
		printf("     <name>%s</name>\n", vars[i].name);
		/*printf("     <dataType>%s</dataType>\n", vars[i].dataType); */
		printf("     <dataType>%s</dataType>\n", upnptypes[vars[i].itype]);
		/*
		if(vars[i].defaultValue)
		  printf("     <defaultValue>%s</defaultValue>\n", vars[i].defaultValue);
		*/
		if(vars[i].idefault)
		  printf("     <defaultValue>%s</defaultValue>\n", upnpdefaultvalues[vars[i].idefault]);
		/* TODO : allowedValueList */
		printf("  </stateVariable>\n");
		i++;
	}
	printf("</serviceStateTable>\n");
	printf("</scpd>\n");
}

void DisplayWANIPCn(void)
{
	DisplayService(&scpdWANIPCn);
}

#ifdef ENABLE_EVENTS
static char *
genEventVars(int * len, const struct serviceDesc * s, const char * servns)
{
	char tmp[16];
	char ext_ip_addr[INET_ADDRSTRLEN];
	const struct stateVar * v;
	char * str;
	int tmplen;
	int number=0;
	tmplen = 512;
	str = (char *)malloc(tmplen);
	if(str == NULL)
		return NULL;
#ifdef	ECOS_DBG_STAT
	dbg_stat_add(dbg_igd_index, DBG_TYPE_MALLOC,DBG_ACTION_ADD, 512);
#endif
	*len = 0;
	v = s->serviceStateTable;
	str = strcat_str(str, len, &tmplen, "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\" xmlns:s=\"");
	str = strcat_str(str, len, &tmplen, servns);
	str = strcat_str(str, len, &tmplen, "\">");
	while(v->name) {
		if(v->itype & 0x80) {
			str = strcat_str(str, len, &tmplen, "<e:property><s:");
			str = strcat_str(str, len, &tmplen, v->name);
			str = strcat_str(str, len, &tmplen, ">");
			//printf("<e:property><s:%s>", v->name);
			switch(v->ieventvalue) {
			case PossibleConnectionTypes_EVENTS:
				str = strcat_str(str, len, &tmplen, "IP_Routed");
				break;
			case ExternalIPAddress_EVENTS:
				if(wan_uptime==0)
					str = strcat_str(str, len, &tmplen, "0.0.0.0");
				else{
					if(getifaddr(ext_if_name, ext_ip_addr, INET_ADDRSTRLEN) < 0) {
						str = strcat_str(str, len, &tmplen, "0.0.0.0");
					} else {
						str = strcat_str(str, len, &tmplen, ext_ip_addr);
					}
				}
				break;
			case PhysicalLinkStatus_EVENTS:	
				if(wan_uptime==0)
					str = strcat_str(str, len, &tmplen, "Down");
				else
					str = strcat_str(str, len, &tmplen, "Up");
				break;
			case ConnectionStatus_EVENTS:	
				if(wan_uptime==0)
					str = strcat_str(str, len, &tmplen, "Disconnected");
				else
					str = strcat_str(str, len, &tmplen, "Connected");
				break;
				
			default:
				str = strcat_str(str, len, &tmplen, upnpallowedvalues[v->ieventvalue]);
				
			}
			str = strcat_str(str, len, &tmplen, "</s:");
			str = strcat_str(str, len, &tmplen, v->name);
			str = strcat_str(str, len, &tmplen, "></e:property>");
			
		}
		v++;
	}
	str = strcat_str(str, len, &tmplen, "</e:propertyset>");
	str[*len] = '\0';
	return str;
}

char *getVarsWANIPCn(int * l)
{
	return genEventVars(l,
                        &scpdWANIPCn,
	                    "urn:schemas-upnp-org:service:WANIPConnection:1");
}

char *getVarsWANCfg(int * l)
{
	return genEventVars(l,
	                    &scpdWANCfg,
	                    "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1");
}
#endif
#ifdef XMLDESC_INTERFACE
/* To be improved */
#if 0
int
xml_pretty_print(const char * s, int len, FILE * f)
{
	int n = 0, i;
	int elt_close = 0;
	int c, indent = 0;

	if(!s)
		return n;
	while(len > 0)
	{
		c = *(s++);	len--;
		switch(c)
		{
		case '<':
			if(len>0 && *s == '/')
				elt_close++;
			else if(len>0 && *s == '?')
				elt_close = 1;
			else
				elt_close = 0;
			if(elt_close!=1)
			{
				if(elt_close > 1)
					indent--;
				fputc('\n', f); n++;
				for(i=indent; i>0; i--)
					fputc(' ', f);
				n += indent;
			}
			fputc(c, f); n++;
			break;
		case '>':
			fputc(c, f); n++;
			if(elt_close==1)
			{
				/*fputc('\n', f); n++; */
				/* elt_close = 0; */
				if(indent > 0)
					indent--;
			}
			else if(elt_close == 0)
				indent++;
			break;
		case '\n':
			/* remove existing LF */
			break;
		default:
			fputc(c, f); n++;
		}
	}
	return n;
}
void free_rootdesc_data()
{
	int i = 0;
	for(;;){
		if(!rootdesc_xml[i].name)
			break;
		if(rootdesc_xml[i].data)
			free(rootdesc_xml[i].data);
		i++;
	}
}
int get_desc_idx(char* name,int *idx)
{
	int i = 0;
	for(;;){
		if(!rootdesc_xml[i].name){
			return -1;
		}
		if(strcmp(rootdesc_xml[i].name,name) == 0){
			*idx = i;
			break;
		}
		i++;
	}
	return 0;
}
/* iterative subroutine using a small stack
 * This way, the progam stack usage is kept low */
static char *
genXML(char * str, int * len, int * tmplen,
                    struct XMLDESC * p)
{
	unsigned short i, j;
	unsigned long k;
	int top;
	const char * eltname, *s;
	char c;
	unsigned char line = 1;
	struct {
		unsigned short i;
		unsigned short j;
		const char * eltname;
	} pile[16]; /* stack */
	top = -1;
	i = 0;	/* current node */
	j = 1;	/* i + number of nodes*/
	for(;;){
		eltname = p[i].eltname;
		if(!eltname)
			return str;
		if(eltname[0] == '/'){
			if(p[i].data && p[i].data[0]){
				/*printf("<%s>%s<%s>\n", eltname+1, p[i].data, eltname); */
				str = strcat_char(str, len, tmplen, '<');
				str = strcat_str(str, len, tmplen, eltname+1);
				str = strcat_char(str, len, tmplen, '>');
				str = strcat_str(str, len, tmplen, p[i].data);
				str = strcat_char(str, len, tmplen, '<');
				str = strcat_str(str, len, tmplen, eltname);
				str = strcat_char(str, len, tmplen, '>');
				p[i].line = line++;
			}
			else{
				if(p[i].def && p[i].def[0]){
				/*printf("<%s>%s<%s>\n", eltname+1, p[i].data, eltname); */
				str = strcat_char(str, len, tmplen, '<');
				str = strcat_str(str, len, tmplen, eltname+1);
				str = strcat_char(str, len, tmplen, '>');
				str = strcat_str(str, len, tmplen, p[i].def);
				str = strcat_char(str, len, tmplen, '<');
				str = strcat_str(str, len, tmplen, eltname);
				str = strcat_char(str, len, tmplen, '>');
				p[i].line = line++;
				}
			}
			for(;;){
				if(top < 0)
					return str;
				i = ++(pile[top].i);
				j = pile[top].j;
				/*printf("  pile[%d]\t%d %d\n", top, i, j); */
				if(i==j){
					/*printf("</%s>\n", pile[top].eltname); */
					str = strcat_char(str, len, tmplen, '<');
					str = strcat_char(str, len, tmplen, '/');
					s = pile[top].eltname;
					for(c = *s; c > ' '; c = *(++s))
						str = strcat_char(str, len, tmplen, c);
					str = strcat_char(str, len, tmplen, '>');
					line++;
					top--;
				}
				else
					break;
			}
		}
		else{
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname);
			str = strcat_char(str, len, tmplen, '>');
			p[i].line = line++;
			k = (unsigned long)p[i].def;
			i = k & 0xffff;
			j = i + (k >> 16);
			top++;
			/*printf(" +pile[%d]\t%d %d\n", top, i, j); */
			pile[top].i = i;
			pile[top].j = j;
			pile[top].eltname = eltname;
		}
	}
}
int gen_root_desc()
{
	char * str;
	int tmplen,len;
	FILE* fpw;
	tmplen = 2048;
	str = (char *)malloc(tmplen);
	if(str == NULL)
		return -1;
	len = strlen(xmlver);
	/*strcpy(str, xmlver); */
	memcpy(str, xmlver, len + 1);
	str = genXML(str, &len, &tmplen, rootdesc_xml);
	str[len] = '\0';
	fpw = fopen("./rootdesc.xml","w");
	if(fpw == NULL){
		free(str);
		return -1;
	}
	if(fpw) {
		xml_pretty_print(str, len, fpw);
		fclose(fpw);
	}
	free(str);
	return 0;
	
}
/*
int mod_root_desc()
{
	char * str;
	int tmplen,idx;
	unsigned char line,mod,i;
	FILE* fpwr;
	char buf[2048];
	tmplen = 2048;
	mod = 1;
	i = 0;
	str = (char *)malloc(tmplen);
	if(str == NULL)
		return -1;
	fpwr = fopen("./rootdesc.xml","r");
	if(fpwr == NULL){
		free(str);
		return -1;
	}
	while (fgets(buf, 2048, fpwr) != NULL){
		if(mod){
			if(get_min_line(&line,&idx) < 0)
				mod = 0;
		}
		if(mod){
			for(;;){
				if(i == line){
					
					break;
				}
				//format into str
				strcat(str,buf);
				fgets();
				i++;
			}
		}
		if(mod) continue;
		//format into str
		strcat(str,buf);
		i++;
	}
	
}
*/
/*
int format_desc(char *option,...)
{
	va_list argp;
	char* para_name;
	char* para_value;
	char* str;
	int status = 0;
	int idx = 0;
	if((strcmp(option,"gen") == 0)||(strcmp(option,"modify") == 0)){
		va_start(argp,option);
		while(1){
			para_name = va_arg(argp,char *);
			if(strcmp(para_name,"") == 0) 
				break;
			para_value = va_arg(argp,char *);
			if(strcmp(para_value,"") == 0){
				printf("device description miss value\n");
				status = -1;
			}
			if(get_desc_idx(para_name,&idx) < 0){
				printf("device description name %s invalid\n",para_name);
				status = -1;
			}
			str = (char *)malloc(strlen(para_value));
			if(!str){
				printf("memory allocate failed!\n");
				status = -1;
			}
			strcpy(str,para_value);
			rootdesc_xml[idx].data = str;	
		}
		if(status == 0){
			if(strcmp(option,"gen") == 0){		
				if(gen_root_desc() < 0){
						printf("generate descption failed \n");
						status = -1;
					}
				}
			if(strcmp(option,"modify") == 0){ 	
				if(mod_root_desc() < 0){
					printf("modify device description name %s failed\n");
					status = -1;
				}
			}
		}
		va_end(argp);
		free_rootdesc_data();
	}
	else{
		printf("unknown option %s,please select option:gen or modify\n");
		status = -1;
	}	
	return status;
}*/
#endif

#endif




