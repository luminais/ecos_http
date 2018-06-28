/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : Software Vlan process
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/

#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_fw.h>
#include <netinet/udp.h>
#ifdef DUMMYNET
#include <netinet/ip_dummynet.h>
#endif
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>
//#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netdb.h>

#include <netinet/if_ether.h> /* XXX ethertype_ip */

#include "./libalias/alias.h"

#include <switch/rtl865x_ip_api.h>

#define RTL_DIVERT_PORT 	8668
#define DEFAULT_NATD_INF	"eth1"

#define GETLOPORT(x)     ((x) >> 0x10)
#define GETNUMPORTS(x)   ((x) & 0x0000ffff)
#define GETHIPORT(x)     (GETLOPORT((x)) + GETNUMPORTS((x)))

/* Set y to be the low-port value in port_range variable x. */
#define SETLOPORT(x,y)   ((x) = ((x) & 0x0000ffff) | ((y) << 0x10))

/* Set y to be the number of ports in port_range variable x. */
#define SETNUMPORTS(x,y) ((x) = ((x) & 0xffff0000) | (y))

typedef u_long port_range;

/*
 * Globals.
 */
static	struct in_addr		aliasAddr;
static	char			ifName[16]={0};
/*
 * Function prototypes.
 */
 extern unsigned long rtl_getWanIfpIp(char *name);
void 		rtl_parseNaptOption (const char* option, const char* parms);
static void	SetupPortRedirect (const char* parms);
static void	SetupProtoRedirect(const char* parms);
static void	SetupAddressRedirect (const char* parms);
static void	StrToAddr (const char* str, struct in_addr* addr);
static u_short  StrToPort (const char* str, const char* proto);
static int      	StrToPortRange (const char* str, const char* proto, port_range *portRange);
static int 		StrToProto (const char* str);
static int      	StrToAddrAndPortRange (const char* str, struct in_addr* addr, char* proto, port_range *portRange);
static void	SetupPunchFW(const char *strValue);

/* 
 * Different options recognized by this program.
 */

enum Option {
	PacketAliasOption,
	AliasAddress,
	TargetAddress,
	InterfaceName,
	RedirectPort,
	RedirectProto,
	RedirectAddress,
	ProxyRule,
	PunchFW
};

enum Param {
	YesNo,
	Numeric,
	String,
	None,
	Address,
	Service
};

/*
 * Option information structure (used by ParseOption).
 */

struct OptionInfo {
	
	enum Option		type;
	int			packetAliasOpt;
	enum Param		parm;
	const char*		parmDescription;
	const char*		description;
	const char*		name; 
	const char*		shortName;
};

/*
 * Table of known options.
 */

static struct OptionInfo optionTable[] = {

	{ PacketAliasOption,
		PKT_ALIAS_UNREGISTERED_ONLY,
		YesNo,
		"[yes|no]",
		"alias only unregistered addresses",
		"unregistered_only",
		"u" },

	{ PacketAliasOption,
		PKT_ALIAS_LOG,
		YesNo,
		"[yes|no]",
		"enable logging",
		"log",
		"l" },

	{ PacketAliasOption,
		PKT_ALIAS_PROXY_ONLY,
		YesNo,
		"[yes|no]",
		"proxy only",
		"proxy_only",
		NULL },

	{ PacketAliasOption,
		PKT_ALIAS_REVERSE,
		YesNo,
		"[yes|no]",
		"operate in reverse mode",
		"reverse",
		NULL },

	{ PacketAliasOption,
		PKT_ALIAS_DENY_INCOMING,
		YesNo,
		"[yes|no]",
		"allow incoming connections",
		"deny_incoming",
		"d" },

	{ PacketAliasOption,
		PKT_ALIAS_USE_SOCKETS,
		YesNo,
		"[yes|no]",
		"use sockets to inhibit port conflict",
		"use_sockets",
		"s" },

	{ PacketAliasOption,
		PKT_ALIAS_SAME_PORTS,
		YesNo,
		"[yes|no]",
		"try to keep original port numbers for connections",
		"same_ports",
		"m" },

	{ AliasAddress,
		0,
		Address,
		"x.x.x.x",
		"address to use for aliasing",
		"alias_address",
		"a" },
	
	{ TargetAddress,
		0,
		Address,
		"x.x.x.x",
		"address to use for incoming sessions",
		"target_address",
		"t" },
	
	{ InterfaceName,
		0,
		String,
	        "network_if_name",
		"take aliasing address from interface",
		"interface",
		"n" },

	{ ProxyRule,
		0,
		String,
	        "[type encode_ip_hdr|encode_tcp_stream] port xxxx server "
		"a.b.c.d:yyyy",
		"add transparent proxying / destination NAT",
		"proxy_rule",
		NULL },

	{ RedirectPort,
		0,
		String,
	        "tcp|udp local_addr:local_port_range[,...] [public_addr:]public_port_range"
	 	" [remote_addr[:remote_port_range]]",
		"redirect a port (or ports) for incoming traffic",
		"redirect_port",
		NULL },

	{ RedirectProto,
		0,
		String,
	        "proto local_addr [public_addr] [remote_addr]",
		"redirect packets of a given proto",
		"redirect_proto",
		NULL },

	{ RedirectAddress,
		0,
		String,
	        "local_addr[,...] public_addr",
		"define mapping between local and public addresses",
		"redirect_address",
		NULL },

	{ PunchFW,
		0,
		String,
	        "basenumber:count",
		"punch holes in the firewall for incoming FTP/IRC DCC connections",
		"punch_fw",
		NULL }
};

void rtl_parseNaptOption(const char* option, const char* parms)
{
	int			i;
	struct OptionInfo*	info;
	int			yesNoValue;
	int			aliasValue;
	int			numValue;
	u_short			uNumValue;
	const char*		strValue;
	struct in_addr		addrValue;
	int			max;
	char*			end;
/*
 * Find option from table.
 */
	max = sizeof (optionTable) / sizeof (struct OptionInfo);
	for (i = 0, info = optionTable; i < max; i++, info++) {

		if (!strcmp (info->name, option))
			break;

		if (info->shortName)
			if (!strcmp (info->shortName, option))
				break;
	}

	if (i >= max) {
		diag_printf("unknown option %s", option);
		return;
	}

	uNumValue	= 0;
	yesNoValue	= 0;
	numValue	= 0;
	strValue	= NULL;
/*
 * Check parameters.
 */
	switch (info->parm) {
	case YesNo:
		if (!parms)
			parms = "yes";

		if (!strcmp (parms, "yes"))
			yesNoValue = 1;
		else
			if (!strcmp (parms, "no"))
				yesNoValue = 0;
			else{
				diag_printf("%s needs yes/no parameter", option);
				return;
			}
		break;

	case Service:
		if (!parms){
			diag_printf("%s needs service name or " "port number parameter", option);
			return;
		}

		uNumValue = StrToPort (parms, "divert");
		break;

	case Numeric:
		if (parms)
			numValue = strtol (parms, &end, 10);
		else
			end = NULL;

		if (end == parms){
			diag_printf("%s needs numeric parameter", option);
			return;
		}
		break;

	case String:
		strValue = parms;
		if (!strValue){
			diag_printf("%s needs parameter", option);
			return;
		}
		break;

	case None:
		if (parms){
			diag_printf("%s does not take parameters", option);
			return;
		}
		break;

	case Address:
		if (!parms){
			diag_printf("%s needs address/host parameter", option);
			return;
		}

		StrToAddr (parms, &addrValue);
		break;

	default:
		break;
	}

	switch (info->type) {
	case PacketAliasOption:
		aliasValue = yesNoValue ? info->packetAliasOpt : 0;
		PacketAliasSetMode (aliasValue, info->packetAliasOpt);
		break;

	case AliasAddress:
		memcpy (&aliasAddr, &addrValue, sizeof (struct in_addr));
		break;

	case TargetAddress:
		PacketAliasSetTarget(addrValue);
		break;

	case RedirectPort:
		if(strValue)
			SetupPortRedirect (strValue);
		break;

	case RedirectProto:
		if(strValue)
			SetupProtoRedirect(strValue);
		break;

	case RedirectAddress:
		if(strValue)
			SetupAddressRedirect (strValue);
		break;

	case ProxyRule:
		if(strValue)		
			PacketAliasProxyRule (strValue);
		break;

	case InterfaceName:
		memset(ifName, 0, 16);
		if(strValue)		
			strncpy(ifName, strValue, 16);
		break;

	case PunchFW:
		if(strValue)
			SetupPunchFW(strValue);
		break;

	default:
		break;
	}
}

void SetupPortRedirect (const char* parms)
{
	char		buf[128];
	char*		ptr;
	char*		serverPool;
	struct in_addr	localAddr;
	struct in_addr	publicAddr;
	struct in_addr	remoteAddr;
	port_range      portRange;
	u_short         localPort      = 0;
	u_short         publicPort     = 0;
	u_short         remotePort     = 0;
	u_short         numLocalPorts  = 0;
	u_short         numPublicPorts = 0;
	u_short         numRemotePorts = 0;
	int		proto;
	char*		protoName;
	char*		separator;
	int             i;
	struct alias_link *link = NULL;
	if(parms)
		strcpy (buf, parms);
/*
 * Extract protocol.
 */
	protoName = strtok (buf, " \t");
	if (!protoName){
		diag_printf("redirect_port: missing protocol");
		return;
	}

	proto = StrToProto (protoName);
/*
 * Extract local address.
 */
	ptr = strtok (NULL, " \t");
	if (!ptr){
		diag_printf("redirect_port: missing local address");
		return;
	}

	separator = strchr(ptr, ',');
	if (separator) {		/* LSNAT redirection syntax. */
		localAddr.s_addr = INADDR_NONE;
		localPort = ~0;
		numLocalPorts = 1;
		serverPool = ptr;
	} else {
		if ( StrToAddrAndPortRange (ptr, &localAddr, protoName, &portRange) != 0 ){
			diag_printf("redirect_port: invalid local port range");
			return;
		}

		localPort     = GETLOPORT(portRange);
		numLocalPorts = GETNUMPORTS(portRange);
		serverPool = NULL;
	}

/*
 * Extract public port and optionally address.
 */
	ptr = strtok (NULL, " \t");
	if (!ptr){
		diag_printf("redirect_port: missing public port");
		return;
	}

	separator = strchr (ptr, ':');
	if (separator) {
	        if (StrToAddrAndPortRange (ptr, &publicAddr, protoName, &portRange) != 0 ){
		       diag_printf("redirect_port: invalid public port range");
			return;
	        }
	}
	else {
		publicAddr.s_addr = INADDR_ANY;
		if (StrToPortRange (ptr, protoName, &portRange) != 0){
		        diag_printf("redirect_port: invalid public port range");
			 return;
		}
	}

	publicPort     = GETLOPORT(portRange);
	numPublicPorts = GETNUMPORTS(portRange);

/*
 * Extract remote address and optionally port.
 */
	ptr = strtok (NULL, " \t");
	if (ptr) {
		separator = strchr (ptr, ':');
		if (separator) {
		        if (StrToAddrAndPortRange (ptr, &remoteAddr, protoName, &portRange) != 0){
			        diag_printf("redirect_port: invalid remote port range");
				 return;
		        }
		} else {
		        SETLOPORT(portRange, 0);
			SETNUMPORTS(portRange, 1);
			StrToAddr (ptr, &remoteAddr);
		}
	}
	else {
	        SETLOPORT(portRange, 0);
		SETNUMPORTS(portRange, 1);
		remoteAddr.s_addr = INADDR_ANY;
	}

	remotePort     = GETLOPORT(portRange);
	numRemotePorts = GETNUMPORTS(portRange);

/*
 * Make sure port ranges match up, then add the redirect ports.
 */
	if (numLocalPorts != numPublicPorts){
	        diag_printf("redirect_port: port ranges must be equal in size");
		 return;
	}

	/* Remote port range is allowed to be '0' which means all ports. */
	if (numRemotePorts != numLocalPorts && (numRemotePorts != 1 || remotePort != 0)){
	        diag_printf("redirect_port: remote port must be 0 or equal to local port range in size");
		 return;
	}

	for (i = 0 ; i < numPublicPorts ; ++i) {
	        /* If remotePort is all ports, set it to 0. */
	        u_short remotePortCopy = remotePort + i;
	        if (numRemotePorts == 1 && remotePort == 0)
		        remotePortCopy = 0;

		link = PacketAliasRedirectPort (localAddr,
						htons(localPort + i),
						remoteAddr,
						htons(remotePortCopy),
						publicAddr,
						htons(publicPort + i),
						proto);
	}

/*
 * Setup LSNAT server pool.
 */
	if (serverPool != NULL && link != NULL) {
		ptr = strtok(serverPool, ",");
		while (ptr != NULL) {
			if (StrToAddrAndPortRange(ptr, &localAddr, protoName, &portRange) != 0){
				diag_printf("redirect_port: invalid local port range");
				return;
			}

			localPort = GETLOPORT(portRange);
			if (GETNUMPORTS(portRange) != 1){
				diag_printf("redirect_port: local port must be single in this context");
				return;
			}
			PacketAliasAddServer(link, localAddr, htons(localPort));
			ptr = strtok(NULL, ",");
		}
	}
}

void SetupProtoRedirect(const char* parms)
{
	char		buf[128];
	char*		ptr;
	struct in_addr	localAddr;
	struct in_addr	publicAddr;
	struct in_addr	remoteAddr;
	int		proto;
	char*		protoName;
	struct protoent *protoent;

	strcpy (buf, parms);
/*
 * Extract protocol.
 */
	protoName = strtok(buf, " \t");
	if (!protoName){
		diag_printf("redirect_proto: missing protocol");
		return;
	}

	protoent = getprotobyname(protoName);
	if (protoent == NULL){
		diag_printf("redirect_proto: unknown protocol %s", protoName);
		return;
	}
	else
		proto = protoent->p_proto;
/*
 * Extract local address.
 */
	ptr = strtok(NULL, " \t");
	if (!ptr){
		diag_printf("redirect_proto: missing local address");
		return;
	}
	else
		StrToAddr(ptr, &localAddr);
/*
 * Extract optional public address.
 */
	ptr = strtok(NULL, " \t");
	if (ptr)
		StrToAddr(ptr, &publicAddr);
	else
		publicAddr.s_addr = INADDR_ANY;
/*
 * Extract optional remote address.
 */
	ptr = strtok(NULL, " \t");
	if (ptr)
		StrToAddr(ptr, &remoteAddr);
	else
		remoteAddr.s_addr = INADDR_ANY;
/*
 * Create aliasing link.
 */
	(void)PacketAliasRedirectProto(localAddr, remoteAddr, publicAddr,
				       proto);
}

void SetupAddressRedirect (const char* parms)
{
	char		buf[128];
	char*		ptr;
	char*		separator;
	struct in_addr	localAddr;
	struct in_addr	publicAddr;
	char*		serverPool;
	struct alias_link *link;

	strcpy (buf, parms);
/*
 * Extract local address.
 */
	ptr = strtok (buf, " \t");
	if (!ptr){
		diag_printf("redirect_address: missing local address");
		return;
	}

	separator = strchr(ptr, ',');
	if (separator) {		/* LSNAT redirection syntax. */
		localAddr.s_addr = INADDR_NONE;
		serverPool = ptr;
	} else {
		StrToAddr (ptr, &localAddr);
		serverPool = NULL;
	}
/*
 * Extract public address.
 */
	ptr = strtok (NULL, " \t");
	if (!ptr){
		diag_printf("redirect_address: missing public address");
		return;
	}

	StrToAddr (ptr, &publicAddr);
	link = PacketAliasRedirectAddr(localAddr, publicAddr);

/*
 * Setup LSNAT server pool.
 */
	if (serverPool != NULL && link != NULL) {
		ptr = strtok(serverPool, ",");
		while (ptr != NULL) {
			StrToAddr(ptr, &localAddr);
			PacketAliasAddServer(link, localAddr, htons(~0));
			ptr = strtok(NULL, ",");
		}
	}
}

void StrToAddr (const char* str, struct in_addr* addr)
{
	struct hostent* hp;

	if (inet_aton (str, addr))
		return;

	hp = gethostbyname (str);
	if (!hp){
		diag_printf("unknown host %s", str);
		return;
	}

	memcpy (addr, hp->h_addr, sizeof (struct in_addr));
}

u_short StrToPort (const char* str, const char* proto)
{
	u_short		port;
	struct servent*	sp;
	char*		end;

	port = strtol (str, &end, 10);
	if (end != str)
		return htons (port);

	sp = getservbyname (str, proto);
	if (!sp){
		diag_printf("unknown service %s/%s", str, proto);
		return -1;
	}

	return sp->s_port;
}

int StrToPortRange (const char* str, const char* proto, port_range *portRange)
{
	char*           sep;
	struct servent*	sp;
	char*		end;
	u_short         loPort;
	u_short         hiPort;
	
	/* First see if this is a service, return corresponding port if so. */
	sp = getservbyname (str,proto);
	if (sp) {
	        SETLOPORT(*portRange, ntohs(sp->s_port));
		 SETNUMPORTS(*portRange, 1);
		 return 0;
	}
	        
	/* Not a service, see if it's a single port or port range. */
	sep = strchr (str, '-');
	if (sep == NULL) {
	        SETLOPORT(*portRange, strtol(str, &end, 10));
		if (end != str) {
		        /* Single port. */
		        SETNUMPORTS(*portRange, 1);
			return 0;
		}

		/* Error in port range field. */
		diag_printf ("unknown service %s/%s", str, proto);
		return -1;
	}

	/* Port range, get the values and sanity check. */
	sscanf (str, "%hu-%hu", &loPort, &hiPort);
	SETLOPORT(*portRange, loPort);
	SETNUMPORTS(*portRange, 0);	/* Error by default */
	if (loPort <= hiPort)
	        SETNUMPORTS(*portRange, hiPort - loPort + 1);

	if (GETNUMPORTS(*portRange) == 0){
	        diag_printf("invalid port range %s", str);
		 return -1;
	}

	return 0;
}

int StrToProto (const char* str)
{
	if (!strcmp (str, "tcp"))
		return IPPROTO_TCP;

	if (!strcmp (str, "udp"))
		return IPPROTO_UDP;

	diag_printf("unknown protocol %s. Expected tcp or udp", str);
	return -1;
}

int StrToAddrAndPortRange (const char* str, struct in_addr* addr, char* proto, port_range *portRange)
{
	char*	ptr;

	ptr = strchr (str, ':');
	if (!ptr){
		diag_printf("%s is missing port number", str);
		return -1;
	}

	*ptr = '\0';
	++ptr;

	StrToAddr (str, addr);
	return StrToPortRange (ptr, proto, portRange);
}

static void SetupPunchFW(const char *strValue)
{
	unsigned int base, num;

	if (sscanf(strValue, "%u:%u", &base, &num) != 2){
		diag_printf("punch_fw: basenumber:count parameter required");
		return;
	}

	PacketAliasSetFWBase(base, num);
	(void)PacketAliasSetMode(PKT_ALIAS_PUNCH_FW, PKT_ALIAS_PUNCH_FW);
}

int rtl_setAliasAddrByInfName(char *name)
{
	struct in_addr wanIp;
	int spl;
	wanIp.s_addr = rtl_getWanIfpIp(name);
	printf("Alias ip is 0x%x, Wan dev is %s\n", wanIp.s_addr, name);
#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)&&defined(CONFIG_RTL_HARDWARE_NAT)
	rtl865x_reinitIpTable();
	rtl865x_addIp(0, htonl(wanIp.s_addr), IP_TYPE_NAPT);
#endif

	/*do not call this rtl_setAliasAddrByInfName in netint context .!!!!!*/
	spl = splsoftnet(); // Prevent any overlapping "stack" processing
	PacketAliasSetAddress(wanIp);
	splx(spl);
	
	return 0;
}

int rtl_enterNaptProcess(struct mbuf *m, int incoming, int port)
{
	int ret = -1;
	struct ip* pip;
	
	if(port != RTL_DIVERT_PORT)
		return ret;	//This packet not need to do napt.

	pip = (struct ip*)m->m_data;
	/*Do not create alias entry for broadcast, multicast packet, #define PKT_ALIAS_OK 1*/
	if((incoming&&m->m_pkthdr.rcvif&&in_broadcast(pip->ip_dst, m->m_pkthdr.rcvif))
		||IN_MULTICAST(ntohl(pip->ip_dst.s_addr)))
		return 1;	

	//printf("----rtl_enterNaptProcess, incoming is %d, port is %d----\n", incoming, port);
	if(incoming){
		ret = PacketAliasIn(m->m_data, IP_MAXPACKET);
	}else{
		ret = PacketAliasOut(m->m_data, IP_MAXPACKET);
	}
    #ifdef HAVE_NAT_ALG
    /* alg maybe modify data part which leads to mbuf total data length not equal to ip total length */
    if((m->m_flags & M_PKTHDR) == M_PKTHDR)
    {
        /* in our ethernet driver donot use mbuf chain to store data(use single mubf or cluster) 
                 otherwise maybe not work well!!!! */
        if (m->m_pkthdr.len != (pip->ip_len))
        {
            m->m_pkthdr.len = (pip->ip_len);
            m->m_len= m->m_pkthdr.len;
        }
    }
    #endif

	return ret;
}

int rtl_initNapt(void)
{
	aliasAddr.s_addr	= INADDR_NONE;
/* 
 * Initialize packet aliasing software.
 * Done already here to be able to alter option bits
 * during command line and configuration file processing.
 */
    #ifdef	ECOS_DBG_STAT
    extern int dbg_napt_index;
	dbg_napt_index=dbg_stat_register("napt");
    #endif

	PacketAliasInit();
#if 0
#if HAVE_NAT_ALG   //lynn enable Punch hole in firewall
#ifndef NO_FW_PUNCH
	rtl_parseNaptOption("punch_fw", "20000:1999");
#endif
#endif
#endif
	return 0;
}


int rtl_UninitNapt(void)
{
	aliasAddr.s_addr	= INADDR_NONE;
	PacketAliasUninit();
	rtl_UninitTftpAlg();
}
