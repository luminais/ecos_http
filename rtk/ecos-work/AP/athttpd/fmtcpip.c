#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include "http.h"
#include "apmib.h"
#include "sys_init.h"
#include "sys_utility.h"
#include "asp.h"
#include "common.h"

#ifndef ASP_LIST_BUFFER_SIZE
#define ASP_LIST_BUFFER_SIZE 512
#endif
#ifndef DHCPDB_LINESIZE_MAX
#define DHCPDB_LINESIZE_MAX 128
#endif
#define _DHCPD_PROG_NAME	"udhcpd"
#define _DHCPD_PID_PATH		"/var/run"
#define _DHCPC_PROG_NAME	"udhcpc"
#define _DHCPC_PID_PATH		"/etc/udhcpc"
#define _PATH_DHCPS_LEASES	"/etc/dhcpdb.bind"


int static handle_fmGetValAndSetDHCP_T(char * name, int mibid, DHCP_T* pDhcp, char * postData, int len, char * errMsgBuf)
{
	char *strVal= NULL; 
	DHCP_T wan_mode;
	if(!errMsgBuf)
	{
	   diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
	   return (-1);
	}

	if(!name||!postData)
	{
		strcpy(errMsgBuf, ("Invalid input value!"));
		return -1;
	}
	strVal = get_cstream_var(postData,len,name,"");
	if(strVal[0])
	{
		if(strcmp(strVal,"fixedIp")==0) 	wan_mode=STATIC_IP;
		else if(strcmp(strVal,"autoIp")==0)	wan_mode=DHCP_CLIENT;
		else if(strcmp(strVal,"ppp")==0)	wan_mode=PPPOE;
		else if(strcmp(strVal,"pptp")==0)	wan_mode=PPTP;
		else if(strcmp(strVal,"l2tp")==0)	wan_mode=L2TP;
		else if(strcmp(strVal,"disable")==0)  wan_mode=STATIC_IP;
		else if(strcmp(strVal,"client")==0)	wan_mode=DHCP_CLIENT;
		else if(strcmp(strVal,"server")==0)	wan_mode=DHCP_SERVER;
		else if(strcmp(strVal,"disable")==0)   wan_mode=DHCP_SERVER;
		else if(strcmp(strVal,"auto")==0)      wan_mode=DHCP_AUTO;
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        else if(strcmp(strVal,"pppoehenan")==0)	wan_mode=PPPOE_HENAN;
		else if(strcmp(strVal,"pppoenanchang")==0)wan_mode=PPPOE_NANCHANG;
		else if(strcmp(strVal,"pppoeother1")==0)	wan_mode=PPPOE_OTHER1;
		else if(strcmp(strVal,"pppoeother2")==0)	wan_mode=PPPOE_OTHER2;
		else if(strcmp(strVal,"dhcpplus")==0)wan_mode=DHCP_PLUS;
        #endif
		else
		{
			sprintf(errMsgBuf,"%s","Wrong WAN Access Type!!");	
			return -1;
		}	
		if ( !apmib_set(mibid, (void *)&wan_mode)) 
		{
			sprintf(errMsgBuf, "Set %s MIB error!",name);
			return -1;
		}
		if(pDhcp)	*pDhcp=wan_mode;
	}
	return 0;
}
#ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
int handle_fmGetValAndSetPPPCONN_T(char * name, int mibid, PPP_CONNECT_TYPE_T *pPppConnType, char * postData, int len, char * errMsgBuf)
#else
int static handle_fmGetValAndSetPPPCONN_T(char * name, int mibid, PPP_CONNECT_TYPE_T *pPppConnType, char * postData, int len, char * errMsgBuf)
#endif
{
	char * strVal =NULL; 
	PPP_CONNECT_TYPE_T pppType;
	if(!errMsgBuf)
	{
	   diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
	   return (-1);
	}

	if(!name||!postData)
	{
		strcpy(errMsgBuf, ("Invalid input value!"));
		return -1;
	}
	strVal =get_cstream_var(postData,len,name,"");
	if(strVal[0])
	{
		if(strcmp(strVal,"continuous")==0) 		pppType=CONTINUOUS;
		else if(strcmp(strVal,"on_demand")==0)	pppType=CONNECT_ON_DEMAND;
		else if(strcmp(strVal,"manual")==0)		pppType=MANUAL;
		else
		{
			sprintf(errMsgBuf,"%s","Wrong ppp connect Type!!");	
			return -1;
		}

		if(!apmib_set(mibid,(void*)&pppType))
		{
			sprintf(errMsgBuf, "Set %s MIB error!",name);
			return -1;
		}
		if(pPppConnType) *pPppConnType=pppType;
	}
	return 0;
}
int static handle_fmGetValAndSetSelEnable(char*name,int mibid,int *pEnable,char * postData, int len, char * errMsgBuf)
{
	char *strVal =NULL;
	int enable;
	if(!errMsgBuf)
	{
	   diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
	   return (-1);
	}

	if(!name||!postData)
	{
		strcpy(errMsgBuf, ("Invalid input value!"));
		return -1;
	}
	strVal = get_cstream_var(postData,len,name,"");
	if(strVal[0])
	{
		if(strcmp(strVal,"disabled")==0)	enable=0;
		else if(strcmp(strVal,"enabled")==0)enable=1;
		else
		{
			sprintf(errMsgBuf,"%s","Wrong ppp connect Type!!");	
			return -1;
		}
		if(!apmib_set(mibid,(void*)&enable))
		{
			sprintf(errMsgBuf, "Set %s MIB error!",name);
			return -1;
		}
		if(pEnable) *pEnable=enable;
	}
	return 0;
}
#ifdef HOME_GATEWAY
int static handle_wantcpip_common(char *postData, int len,char* errMsgBuf,int *pDnsChange)
{
	char *strDns=NULL,*strVal=NULL;
	DNS_TYPE_T dns_mode = DNS_AUTO,old_dns_mode;
	int intVal=0;
	char tmpbuf[24]={0};
	#if defined(CONFIG_RTL_WEB_WAN_ACCESS_PORT)
	int web_access_en = 0;
	int web_access_port = 0;
	#endif
	
	if(!errMsgBuf)
	{
	diag_printf("%s,%d, buffer null, error\n",__FUNCTION__,__LINE__);
	return (-1);
	}	
	if(!postData || !pDnsChange)
	{
		strcpy(errMsgBuf,"Invalid input!!!");
		return -1;
	}	
	
	//printf("%s:%d\n",__FILE__,__LINE__);
//set dns
	strDns=get_cstream_var(postData,len,"dnsMode","");
	if(strDns[0])
	{
		if(strcmp(strDns,"dnsAuto")==0)
			dns_mode=DNS_AUTO;
		else if(strcmp(strDns,"dnsManual")==0)
			dns_mode=DNS_MANUAL;
		else
		{
			strcpy(errMsgBuf,"Invalid DNS mode value!");
			return -1;
		}
	}
	if(!apmib_get(MIB_DNS_MODE,(void*)&old_dns_mode))
	{
		strcpy(errMsgBuf,"Get DNS MIB error!");
		return -1;
	}
	if(old_dns_mode!=dns_mode)
	{
		*pDnsChange=1;
		if(!apmib_set(MIB_DNS_MODE,(void*)&dns_mode))
		{
			strcpy(errMsgBuf,"Set DNS MIB error!");
			return -1;
		}
	}
	if(dns_mode==DNS_MANUAL)
	{		
		struct in_addr dns1_old,dns1, dns2_old,dns2, dns3_old,dns3;
		if ( !apmib_get(MIB_DNS1, (void *)&dns1_old)) {
  			strcpy(errMsgBuf, "Get DNS1 MIB error!");
			return -1;
		}
		if ( !apmib_get(MIB_DNS2, (void *)&dns2_old)) {
  			strcpy(errMsgBuf, "Get DNS2 MIB error!");
			return -1;
		}
		if ( !apmib_get(MIB_DNS3, (void *)&dns3_old)) {
  			strcpy(errMsgBuf, "Get DNS3 MIB error!");
			return -1;
		}
		if(handle_fmGetValAndSetIp("dns1",MIB_DNS1,(void*)&dns1,postData,len,errMsgBuf)<0) return -1;
		if(handle_fmGetValAndSetIp("dns2",MIB_DNS2,(void*)&dns2,postData,len,errMsgBuf)<0) return -1;
		if(handle_fmGetValAndSetIp("dns3",MIB_DNS3,(void*)&dns3,postData,len,errMsgBuf)<0) return -1;

		if ( *((long *)&dns1) != *((long *)&dns1_old) ||
			 *((long *)&dns2) != *((long *)&dns2_old) ||
			 *((long *)&dns3) != *((long *)&dns3_old) )
				*pDnsChange = 1;
	}	
	
//set mac clone
	if(handle_fmGetValAndSetMac("wan_macAddr",MIB_WAN_MAC_ADDR,NULL,postData,len,errMsgBuf)<0) return -1;
//printf("%s:%d\n",__FILE__,__LINE__);
//set others
	if(handle_fmGetValAndSetChkbox("upnpEnabled",MIB_UPNP_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
//for igmpproxyEnabled
	strVal = get_cstream_var(postData,len, "igmpproxyEnabled", "");
	if(!strcmp(strVal,"ON"))
		intVal=0;
	else intVal=1;
	if ( !apmib_set(MIB_IGMP_PROXY_DISABLED, (void *)&intVal)) {
		 sprintf(errMsgBuf, "Set igmpproxyEnabled mib error!");
		 return -1;
	 }
	//if(handle_fmGetValAndSetChkbox("igmpproxyEnabled",MIB_IGMP_PROXY_DISABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetChkbox("webWanAccess",MIB_WEB_WAN_ACCESS_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	#if defined(CONFIG_RTL_WEB_WAN_ACCESS_PORT)
	if(!apmib_get(MIB_WEB_WAN_ACCESS_ENABLED,(void*)&web_access_en))
	{
		strcpy(errMsgBuf,"Get MIB_WEB_WAN_ACCESS_ENABLED error!");
		return -1;
	}
	if (web_access_en){
		strVal = get_cstream_var(postData,len, "webAccessPort", "");
		if (!string_to_dec(strVal, &web_access_port) || web_access_port<1 || web_access_port>65535) {
			strcpy(errMsgBuf, ("Error! Invalid value of port."));
			return -1;
		}
		if ( !apmib_set(MIB_WEB_WAN_ACCESS_PORT, (void *)&web_access_port)) {
			strcpy(errMsgBuf, ("Set WEB_WAN_ACCESS_ENABLED error!"));
			return -1;
		}
	}
	#endif
	
	if(handle_fmGetValAndSetChkbox("pingWanAccess",MIB_PING_WAN_ACCESS_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetChkbox("WANPassThru1",MIB_VPN_PASSTHRU_IPSEC_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetChkbox("WANPassThru2",MIB_VPN_PASSTHRU_PPTP_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetChkbox("WANPassThru3",MIB_VPN_PASSTHRU_L2TP_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetChkbox("ipv6_passthru_enabled",MIB_CUSTOM_PASSTHRU_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
    #ifdef CONFIG_RTL_NETSNIPER_SUPPORT
	if(handle_fmGetValAndSetChkbox("netsniper",MIB_NET_SNIPER_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
    #endif
	
    return 1;
}
int static handle_wantcpip_static(char *postData, int len,char* errMsgBuf)
{
	if(handle_fmGetValAndSetIp("wan_ip",MIB_WAN_IP_ADDR,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetIp("wan_mask",MIB_WAN_SUBNET_MASK,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetIp("wan_gateway",MIB_WAN_DEFAULT_GATEWAY,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetIntVal("fixedIpMtuSize",MIB_FIXED_IP_MTU_SIZE,NULL,postData,len,errMsgBuf)<0) return -1;
	return 0;
}
int static handle_wantcpip_dhcpc(char *postData, int len,char* errMsgBuf)
{
	if(handle_fmGetValAndSetIntVal("dhcpMtuSize",MIB_DHCP_MTU_SIZE,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetStrVal("hostName",MIB_HOST_NAME,NULL,postData,len,errMsgBuf)<0) return -1;
	return 0;
}
int static handle_wantcpip_ppp(char *postData, int len,char* errMsgBuf)
{
	char *strConnect;
	PPP_CONNECT_TYPE_T pppType;
	
	if(handle_fmGetValAndSetChkbox("tr069DhcpEnabled",MIB_PPPOE_DHCP_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetStrVal("pppUserName",MIB_PPP_USER_NAME,NULL,postData,len,errMsgBuf)<0) return -1;
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    set_encrypt_pppoe_name(postData, len,errMsgBuf);
    #endif
	if(handle_fmGetValAndSetStrVal("pppPassword",MIB_PPP_PASSWORD,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetStrVal("pppServiceName",MIB_PPP_SERVICE_NAME,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetPPPCONN_T("pppConnectType",MIB_PPP_CONNECT_TYPE,&pppType,postData,len,errMsgBuf)<0) return -1;
	if(pppType==CONNECT_ON_DEMAND)
	{		
		if(handle_fmGetValAndSetIntVal("pppIdleTime",MIB_PPP_IDLE_TIME,NULL,postData,len,errMsgBuf)<0) return -1;
	}
	if(handle_fmGetValAndSetIntVal("pppMtuSize",MIB_PPP_MTU_SIZE,NULL,postData,len,errMsgBuf)<0) return -1;
#ifdef HAVE_PPPOE
	strConnect=get_cstream_var(postData,len,"pppConnect","");
	if (strConnect && strConnect[0]) {
		diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		pppoe_reconnect();
		return 2;
	}

	strConnect =get_cstream_var(postData,len,"pppDisconnect","");
	
	if (strConnect && strConnect[0]) {		
		diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		pppoe_disconnect();
		return 1;
	}
#endif
	return 0;
}
int static handle_wantcpip_pptp(char *postData, int len,char* errMsgBuf)
{
	char *strConnect;
	PPP_CONNECT_TYPE_T pptpType;
	if(handle_fmGetValAndSetStrVal("pptpUserName",MIB_PPTP_USER_NAME,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetStrVal("pptpPassword",MIB_PPTP_PASSWORD,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetIp("pptpIpAddr",MIB_PPTP_IP_ADDR,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetIp("pptpSubnetMask",MIB_PPTP_SUBNET_MASK,NULL,postData,len,errMsgBuf)<0) return -1;
#if defined(CONFIG_DYNAMIC_WAN_IP)
	if(handle_fmGetValAndSetIp("pptpDefGw",MIB_PPTP_DEFAULT_GW,NULL,postData,len,errMsgBuf)<0) return -1;
#endif
	if(handle_fmGetValAndSetIp("pptpServerIpAddr",MIB_PPTP_SERVER_IP_ADDR,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetPPPCONN_T("pptpConnectType",MIB_PPTP_CONNECTION_TYPE,&pptpType,postData,len,errMsgBuf)<0) return -1;
	if(pptpType!=CONTINUOUS)
	{		
		if(handle_fmGetValAndSetIntVal("pptpIdleTime",MIB_PPTP_IDLE_TIME,NULL,postData,len,errMsgBuf)<0) return -1;
	}
	if(handle_fmGetValAndSetIntVal("pptpMtuSize",MIB_PPTP_MTU_SIZE,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetChkbox("pptpSecurity",MIB_PPTP_SECURITY_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetChkbox("pptpCompress",MIB_PPTP_MPPC_ENABLED,NULL,postData,len,errMsgBuf)<0) return -1;
#ifdef CONFIG_DYNAMIC_WAN_IP
	{
		int pptpDyn=0;
		char *strPptpDyn=get_cstream_var(postData,len,"wan_pptp_use_dynamic_carrier_radio","");
		if(strPptpDyn[0])
		{
			if(strcmp(strPptpDyn,"dynamicIP")==0)
				pptpDyn=0;
			else if(strcmp(strPptpDyn,"staticIP")==0)
				pptpDyn=1;
			else
			{
				strcpy(errMsgBuf,"Invalid pptp_use_dynamic_carrier_radio mode value!");
				return -1;
			}
			if(!apmib_set(MIB_PPTP_WAN_IP_DYNAMIC,(void*)&pptpDyn))
			{
				strcpy(errMsgBuf,"Set MIB_PPTP_WAN_IP_DYNAMIC MIB error!");
				return -1;
			}
		}		
	}
#endif
#ifdef CONFIG_GET_SERVER_IP_BY_DOMAIN
	{
		int pptpGetServMode=0;
		char *strPptpGetServMode = get_cstream_var(postData,len,"pptpGetServMode","");
		if(strPptpGetServMode[0])
		{
			if(strcmp(strPptpGetServMode,"pptpGetServByDomainName")==0)
				pptpGetServMode=1;
			else if(strcmp(strPptpGetServMode,"pptpGetServByIp")==0)
				pptpGetServMode=0;
			else
			{
				strcpy(errMsgBuf,"Invalid pptpGetServMode value!");
				return -1;
			}
			if(!apmib_set(MIB_PPTP_GET_SERV_BY_DOMAIN,(void*)&pptpGetServMode))
			{
				strcpy(errMsgBuf,"Set MIB_PPTP_GET_SERV_BY_DOMAIN MIB error!");
				return -1;
			}
		}		
		if(handle_fmGetValAndSetStrVal("pptpServerDomainName",MIB_PPTP_SERVER_DOMAIN,NULL,postData,len,errMsgBuf)<0) 
				return -1;
	}
#endif
#ifdef HAVE_PPTP
		diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		strConnect=get_cstream_var(postData,len,"pptpConnect","");
		if (strConnect && strConnect[0]) {
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			pptp_reconnect();
			return 2;
		}
	
		strConnect =get_cstream_var(postData,len,"pptpDisconnect","");
		
		if (strConnect && strConnect[0]) {		
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			pptp_disconnect();
			return 1;
		}
#endif	
	return 0;
}
int static handle_wantcpip_l2tp(char *postData, int len,char* errMsgBuf)
{	
	char *strConnect;
	PPP_CONNECT_TYPE_T l2tpType;
	
	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	if(handle_fmGetValAndSetStrVal("l2tpUserName",MIB_L2TP_USER_NAME,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetStrVal("l2tpPassword",MIB_L2TP_PASSWORD,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetIp("l2tpIpAddr",MIB_L2TP_IP_ADDR,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetIp("l2tpSubnetMask",MIB_L2TP_SUBNET_MASK,NULL,postData,len,errMsgBuf)<0) return -1;
#if defined(CONFIG_DYNAMIC_WAN_IP)
	if(handle_fmGetValAndSetIp("l2tpDefGw",MIB_L2TP_DEFAULT_GW,NULL,postData,len,errMsgBuf)<0) return -1;
#endif
	if(handle_fmGetValAndSetIp("l2tpServerIpAddr",MIB_L2TP_SERVER_IP_ADDR,NULL,postData,len,errMsgBuf)<0) return -1;
	if(handle_fmGetValAndSetPPPCONN_T("l2tpConnectType",MIB_L2TP_CONNECTION_TYPE,&l2tpType,postData,len,errMsgBuf)<0) return -1;
	if(l2tpType!=CONTINUOUS)
	{
		if(handle_fmGetValAndSetIntVal("l2tpIdleTime",MIB_L2TP_IDLE_TIME,NULL,postData,len,errMsgBuf)<0) return -1;		
	}
	if(handle_fmGetValAndSetIntVal("l2tpMtuSize",MIB_L2TP_MTU_SIZE,NULL,postData,len,errMsgBuf)<0) return -1;

#ifdef CONFIG_DYNAMIC_WAN_IP
	{
		int l2tpDyn=0;
		char *strL2tpDyn=get_cstream_var(postData,len,"wan_l2tp_use_dynamic_carrier_radio","");
		if(strL2tpDyn[0])
		{
			if(strcmp(strL2tpDyn,"dynamicIP")==0)
				l2tpDyn=0;
			else if(strcmp(strL2tpDyn,"staticIP")==0)
				l2tpDyn=1;
			else
			{
				strcpy(errMsgBuf,"Invalid l2tp_use_dynamic_carrier_radio mode value!");
				return -1;
			}
			if(!apmib_set(MIB_L2TP_WAN_IP_DYNAMIC,(void*)&l2tpDyn))
			{
				strcpy(errMsgBuf,"Set MIB_L2TP_WAN_IP_DYNAMIC MIB error!");
				return -1;
			}
		}		
	}
#endif
#ifdef CONFIG_GET_SERVER_IP_BY_DOMAIN
	{
		int l2tpGetServMode=0;
		char *strL2tpGetServMode = get_cstream_var(postData,len,"l2tpGetServMode","");
		if(strL2tpGetServMode[0])
		{
			if(strcmp(strL2tpGetServMode,"l2tpGetServByDomainName")==0)
				l2tpGetServMode=1;
			else if(strcmp(strL2tpGetServMode,"l2tpGetServByIp")==0)
				l2tpGetServMode=0;
			else
			{
				strcpy(errMsgBuf,"Invalid l2tpGetServMode value!");
				return -1;
			}
			if(!apmib_set(MIB_L2TP_GET_SERV_BY_DOMAIN,(void*)&l2tpGetServMode))
			{
				strcpy(errMsgBuf,"Set MIB_L2TP_GET_SERV_BY_DOMAIN MIB error!");
				return -1;
			}
		}		
		if(handle_fmGetValAndSetStrVal("l2tpServerDomainName",MIB_L2TP_SERVER_DOMAIN,NULL,postData,len,errMsgBuf)<0) 
				return -1;
	}
#endif

#ifdef HAVE_L2TP
		//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		strConnect=get_cstream_var(postData,len,"l2tpConnect","");
		if (strConnect && strConnect[0]) {
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			l2tp_reconnect();
			return 2;
		}
	
		strConnect =get_cstream_var(postData,len,"l2tpDisconnect","");
		
		if (strConnect && strConnect[0]) {		
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			l2tp_disconnect();
			return 1;
		}
#endif

	return 0;
}

void formWanTcpipSetup(char *postData, int len)
{
	char	*strMode=NULL,  *submitUrl=NULL;
	char  *strVal, *strType;
	int retVal=0,dnsChange=0, old_passthru=0, new_passthru=0, opmode=0;
	struct in_addr inIp, inMask,dns1, dns2, dns3, inGateway;
	DHCP_T wan_mode;	
	char *errMsgBuf=NULL;
	errMsgBuf=(char*)malloc(MSG_BUFFER_SIZE);	
	if(!errMsgBuf)
	{
		printf("malloc fail!!\n");
		goto setErr_tcpip;
	}
	bzero(errMsgBuf,MSG_BUFFER_SIZE);

	/*save old wan dhcp for reinit*/
	apmib_get(MIB_WAN_DHCP,&wan_mode);
	apmib_set(MIB_WAN_OLD_DHCP, (void *)&wan_mode);
	
	if(handle_fmGetValAndSetDHCP_T("wanType",MIB_WAN_DHCP,&wan_mode,postData,len,errMsgBuf)<0) 
		goto setErr_tcpip;

	
	//diag_printf("%s,%d, wan_mode (%d)\n\n",__FUNCTION__,__LINE__,wan_mode);
	switch(wan_mode)
	{
		case STATIC_IP:
			retVal=handle_wantcpip_static(postData,len,errMsgBuf);
			if(retVal<0)
				goto setErr_tcpip;
			break;
		case DHCP_CLIENT:
			retVal=handle_wantcpip_dhcpc(postData,len,errMsgBuf);
			if(retVal<0)
				goto setErr_tcpip;
			break;
		case PPPOE:
			retVal=handle_wantcpip_ppp(postData,len,errMsgBuf);
			if(retVal<0)
				goto setErr_tcpip;
			break;
		case PPTP:
			retVal=handle_wantcpip_pptp(postData,len,errMsgBuf);
			if(retVal<0)
				goto setErr_tcpip;
			break;
		case L2TP:
			retVal = handle_wantcpip_l2tp(postData,len,errMsgBuf);
			if(retVal < 0)
				goto setErr_tcpip;
			break;
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        case PPPOE_HENAN:
        case PPPOE_NANCHANG:
        case PPPOE_OTHER1:
        case PPPOE_OTHER2:
            //diag_printf("%s %d wan_mode = %d \n", __FUNCTION__, __LINE__, wan_mode);
			retVal = handle_wantcpip_ppp_netsniper(postData,len,errMsgBuf, wan_mode);
			if(retVal < 0)
				goto setErr_tcpip;
			break;
        case DHCP_PLUS:
            retVal = handle_wantcpip_dhcphenan_netsniper(postData,len,errMsgBuf, wan_mode);
			if(retVal < 0)
				goto setErr_tcpip;
			break;
        #endif
		default:
			break;
	}	

	apmib_get(MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&old_passthru);
	apmib_get(MIB_OP_MODE, (void *)&opmode);
	if(handle_wantcpip_common(postData,len,errMsgBuf,&dnsChange)<0)
		goto setErr_tcpip;

	if(old_passthru == 1 && opmode == WISP_MODE)
	{
		apmib_get(MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&new_passthru);
	}
	
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	apmib_update_web(CURRENT_SETTING);
	submitUrl=get_cstream_var(postData,len,"submit-url","");

	//diag_printf("%s,%d, retVal (%d)\n\n",__FUNCTION__,__LINE__,retVal);
#ifdef HAVE_SYSTEM_REINIT	
	if(!retVal)		
	{
		if(old_passthru == 1 && new_passthru == 0 && opmode == WISP_MODE)
			wait_redirect("Apply Changes",25,submitUrl);
		else
			wait_redirect("Apply Changes",20,submitUrl);
	}
#else
	if(!retVal)
		OK_MSG(submitUrl);
#endif
	else if(retVal==1)
		send_redirect_perm(submitUrl);
	else if(retVal==2)
	{	
		if(PPPOE == wan_mode)
			wait_redirect("pppoe is dialing...",10,submitUrl);
		else if(L2TP == wan_mode)
			wait_redirect("l2tp is dialing...",10,submitUrl);	
		else if(PPTP == wan_mode)		
			wait_redirect("PPTP is dialing...",10,submitUrl);	
	}
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
#ifdef HAVE_SYSTEM_REINIT
	if(!retVal)	/*if ppp need reinit */
	{
		if(old_passthru == 1 && new_passthru == 0 && opmode == WISP_MODE)
		{
			//it should reinit all for it will down pwlan0 then cause wisp wan can't work well.
			//it can down/up interface in start_custom_passthru_configure, but it will reinit wlan twice when call SYS_REINIT_ALL from other GUI.
			kick_reinit_m(SYS_REINIT_ALL);
		}
		else
			kick_reinit_m(SYS_WAN_M);
	}
#endif

	save_cs_to_file();

	return 0;	
setErr_tcpip:
	ERR_MSG(errMsgBuf);
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	return 0;
}


int clear_firewall_info(struct in_addr inLanaddr_orig, struct in_addr inLanmask_orig,
			struct in_addr inLanaddr_new,struct in_addr inLanmask_new, char* errMsgBuf)
{
	char buffer[200] = {0};
	//char *errMsgBuf=NULL;
	//errMsgBuf=(char*)malloc(MSG_BUFFER_SIZE);	
	if(!errMsgBuf)
	{
		diag_printf("errMsgBuf is NULL!!\n");
		return -1;
	}
	//bzero(errMsgBuf,MSG_BUFFER_SIZE);	
#if 1
	if((inLanaddr_orig.s_addr & inLanmask_orig.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr))
	{		 
		struct in_addr	ipAddr = {0}, newipAddr = {0};
		int entryNum = 0, i = 0;
		PORTFW_T portfw_delentry = {0};
		IPFILTER_T filter_delentry = {0};
#ifdef QOS_BY_BANDWIDTH	
		IPQOS_T qos_delentry = {0};
#endif
		DHCPRSVDIP_T dhcprsv_entry;
		
		/* clear dmz address */
		apmib_get( MIB_DMZ_HOST,  (void *)buffer);
		memcpy((void *)&ipAddr, buffer, 4);
		if(ipAddr.s_addr != 0 && ((ipAddr.s_addr & inLanmask_new.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)))
		{
			memset(&ipAddr, 0x00, sizeof(ipAddr));
			if ( apmib_set(MIB_DMZ_HOST, (void *)&ipAddr) == 0) {
				strcpy(errMsgBuf, "Delete DMZ host error!");
                return -1;
			}
		}
		/* clear port forwarding table */
		if (!apmib_set(MIB_PORTFW_DELALL,(void*)&portfw_delentry))
		{
			strcpy(errMsgBuf, ("\"Delete all portfw table error!\""));
            return -1;
		}
		/* clear ipfilter table */
		if ( !apmib_set(MIB_IPFILTER_DELALL, (void *)&filter_delentry)) {
			strcpy(errMsgBuf, ("Delete all firewall table error!"));
            return -1;
		}
#ifdef QOS_BY_BANDWIDTH
		/* clear qos table	*/
		apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum);
		for(i=1;i<=entryNum;i++)
		{
			memset((void*)&qos_delentry, 0x00, sizeof(qos_delentry));
			*((char *)&qos_delentry) = (char)i;
			if ( apmib_get(MIB_QOS_RULE_TBL, (void *)&qos_delentry))
			{
				if(qos_delentry.mode & QOS_RESTRICT_IP)
				{
					ipAddr.s_addr = (*((struct in_addr *)qos_delentry.local_ip_start)).s_addr;
					//diag_printf("ipAddr:%x [%s]:[%d].\n",ipAddr.s_addr, __FUNCTION__,__LINE__);				
					if(ipAddr.s_addr != 0 && ((ipAddr.s_addr & inLanmask_orig.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)))
					{
						apmib_set(MIB_QOS_DEL, (void *)&qos_delentry);
					}
					
				}
			}
		}
#endif
		/* change static dhcp to the new subnet */
		if(!apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum))
		{
			strcpy(errMsgBuf, "get mib MIB_DHCPRSVDIP_TBL_NUM fail!");
            return -1;
		}
		
		for (i=1; i<=entryNum; i++) {
			memset((void*)&dhcprsv_entry, 0x00, sizeof(dhcprsv_entry));
			*((char *)&dhcprsv_entry) = (char)i;
			if(!apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&dhcprsv_entry))
			{
				strcpy(errMsgBuf, "get mib MIB_DHCPRSVDIP_TBL fail!");
                		return -1;
			}

			memcpy(&ipAddr.s_addr, dhcprsv_entry.ipAddr, 4);			
			
			if((ipAddr.s_addr & inLanmask_new.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr))
			{
				newipAddr.s_addr = (inLanaddr_new.s_addr & inLanmask_new.s_addr) | (ipAddr.s_addr & (~inLanmask_new.s_addr));

				apmib_set(MIB_DHCPRSVDIP_DEL, (void *)&dhcprsv_entry);

				memcpy(dhcprsv_entry.ipAddr, &newipAddr.s_addr, 4);				
							 
				if ( !apmib_set(MIB_DHCPRSVDIP_ADD, (void *)&dhcprsv_entry))
				{
					strcpy(errMsgBuf, "set MIB_DHCPRSVDIP_MOD fail!");
                    			return -1;
				}
			}

		}
	}
#endif
    return 0;	
}
#endif
unsigned int convertFourCharToOneInt(unsigned char char_IP_addr[])
{
	unsigned int int_IP_addr=0;
	unsigned char char0=char_IP_addr[0];
	unsigned char char1=char_IP_addr[1];
	unsigned char char2=char_IP_addr[2];
	unsigned char char3=char_IP_addr[3];
	int_IP_addr=char3+(char2<<8)+(char1<<16)+(char0<<24);
	return int_IP_addr;
}
int checkStaticIpIsValid(char *tmpBuf)
{
	int i, entryNum=0, enabled=0;
	DHCPRSVDIP_T entry;
	struct in_addr start_ip, end_ip, router_ip;
	
	apmib_get(MIB_DHCPRSVDIP_ENABLED, (void *)&enabled);
	if(enabled==0)
		return 0;
	
	apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum);
	apmib_get(MIB_DHCP_CLIENT_START,  (void *)&start_ip);
	apmib_get(MIB_DHCP_CLIENT_END,  (void *)&end_ip);
	apmib_get(MIB_IP_ADDR,  (void *)&router_ip);
	
	for (i=1; i<=entryNum; i++) 
	{
		*((char *)&entry) = (char)i;
		if(!apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&entry))
		{
			printf("get mib MIB_DHCPRSVDIP_TBL fail!\n");
			return 0;
		}		
		unsigned int int_IP_addr=convertFourCharToOneInt(entry.ipAddr);
		if(int_IP_addr<start_ip.s_addr || int_IP_addr>end_ip.s_addr || int_IP_addr==router_ip.s_addr)
		{
			strcpy(tmpBuf, ("Please check your \"Static DHCP\" setting. The static IP address must be in the range of dhcpd ip pool, and is not same with router's ip!"));
			return 1;	
		}
	}	
	return 0;
}

void formTcpipSetup(char *postData, int len)
{
//	char *buf;	
	char *submitUrl=NULL, *strDhcp, *strIp, *strMask, *strStartIp, *strEndIp;
	struct in_addr inIp,inMask;
	char *errMsgBuf=NULL;
	DHCP_T lan_dhcp,lan_dhcp_orig, dhcp;
	char buffer[200] = {0};
	struct in_addr inLanaddr_orig, inLanaddr_new;
	struct in_addr inLanmask_orig, inLanmask_new;
	errMsgBuf=(char*)malloc(MSG_BUFFER_SIZE);	
	if(!errMsgBuf)
	{
		printf("malloc fail!!\n");
		goto setErr_tcpipLan;
	}
	bzero(errMsgBuf,MSG_BUFFER_SIZE);

    /* save old ip address and mask */
    apmib_get( MIB_IP_ADDR,  (void *)buffer); //save the orig lan subnet
	memcpy((void *)&inLanaddr_orig, buffer, 4);
	
	apmib_get( MIB_SUBNET_MASK,  (void *)buffer); //save the orig lan mask
	memcpy((void *)&inLanmask_orig, buffer, 4);
	apmib_get(MIB_DHCP,&lan_dhcp_orig);




	////////////////////////////////////////
	int enable;
	apmib_get(MIB_DHCPRSVDIP_ENABLED, (void *)&enable);

	strDhcp = get_cstream_var(postData,len,"dhcp","");
	if ( enable > 0 && strDhcp[0] ) 
	{
		if(strcmp(strDhcp,"server")==0)	
		{
			int entryNum, i;
			DHCPRSVDIP_T entry;
			struct in_addr start_ip, end_ip, router_ip, netmask;
			unsigned int staticIp;
			apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum);
			
			strIp = get_cstream_var(postData,len,"lan_ip","");
			
			if ( strIp[0] ) 
				inet_aton(strIp, &router_ip);

			strMask = get_cstream_var(postData,len,"lan_mask","");
			
			if ( strMask[0] ) 
				inet_aton(strMask, &netmask);

			strStartIp = get_cstream_var(postData,len,"dhcpRangeStart","");
			
			if ( strStartIp[0] ) 
				inet_aton(strStartIp, &start_ip);

			strEndIp = get_cstream_var(postData,len,"dhcpRangeEnd","");
			
			if ( strEndIp[0] ) 
				inet_aton(strEndIp, &end_ip);		
		
			for (i=1; i<=entryNum; i++) 
			{
				*((char *)&entry) = (char)i;			
				apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&entry);
				memcpy(&staticIp, entry.ipAddr, 4);

				if((staticIp & (~netmask.s_addr)) < ((start_ip.s_addr) & (~netmask.s_addr)) 
					|| (staticIp & (~netmask.s_addr)) > ((end_ip.s_addr) & (~netmask.s_addr)) 
					//|| (staticIp & (~netmask.s_addr))==((router_ip.s_addr) & (~netmask.s_addr))
				  )
				{
					strcpy(errMsgBuf, "DHCP Client Range can't cover Static DHCP List!!!");
					goto setErr_tcpipLan ; 
				}
			}	
		}
	}
///////////////////////////////////////
    
	if(handle_fmGetValAndSetDHCP_T("dhcp",MIB_DHCP,&lan_dhcp,postData,len,errMsgBuf)<0)						goto setErr_tcpipLan;
	if(handle_fmGetValAndSetIp("lan_ip",MIB_IP_ADDR,NULL,postData,len,errMsgBuf)<0)							goto setErr_tcpipLan;
	if(handle_fmGetValAndSetIp("lan_mask",MIB_SUBNET_MASK,NULL,postData,len,errMsgBuf)<0)					goto setErr_tcpipLan;
	if(handle_fmGetValAndSetIp("lan_gateway",MIB_DEFAULT_GATEWAY,NULL,postData,len,errMsgBuf)<0)			goto setErr_tcpipLan;
	if(handle_fmGetValAndSetIp("dhcpRangeStart",MIB_DHCP_CLIENT_START,NULL,postData,len,errMsgBuf)<0)	goto setErr_tcpipLan;
	if(handle_fmGetValAndSetIp("dhcpRangeEnd",MIB_DHCP_CLIENT_END,NULL,postData,len,errMsgBuf)<0)		goto setErr_tcpipLan;
	if(handle_fmGetValAndSetIntVal("dhcpLeaseTime",MIB_DHCP_LEASE_TIME,NULL,postData,len,errMsgBuf)<0)		goto setErr_tcpipLan;
	if(handle_fmGetValAndSetStrVal("domainName",MIB_DOMAIN_NAME,NULL,postData,len,errMsgBuf)<0)				goto setErr_tcpipLan;
#ifdef HAVE_NBSERVER
	if(handle_fmGetValAndSetStrVal("netbiosName",MIB_NETBIOS_NAME,NULL,postData,len,errMsgBuf)<0)				goto setErr_tcpipLan;
#endif
	if(handle_fmGetValAndSetSelEnable("stp",MIB_STP_ENABLED,NULL,postData,len,errMsgBuf)<0)					goto setErr_tcpipLan;
	if(handle_fmGetValAndSetMac("lan_macAddr",MIB_ELAN_MAC_ADDR,NULL,postData,len,errMsgBuf)<0) 			goto setErr_tcpipLan;

	/* check the subnet changed or not */
	apmib_get( MIB_IP_ADDR,  (void *)buffer); //check the new lan subnet
	memcpy((void *)&inLanaddr_new, buffer, 4);
	
	apmib_get( MIB_SUBNET_MASK,  (void *)buffer); //check the new lan mask
	memcpy((void *)&inLanmask_new, buffer, 4);

	//if(checkStaticIpIsValid(errMsgBuf)>0)
	//	goto setErr_tcpipLan ;
	
#ifdef HOME_GATEWAY
    /* clear dmz /port forwarding/firewall rules /qos rules when lan subnet changed 
           and change static dhcp to the new subnet */
    if (clear_firewall_info(inLanaddr_orig,inLanmask_orig,inLanaddr_new,inLanmask_new, errMsgBuf)<0)   goto setErr_tcpipLan;
#if 0
    if((inLanaddr_orig.s_addr & inLanmask_orig.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr))
    {        
        struct in_addr  ipAddr = {0}, newipAddr = {0};
        int entryNum = 0, i = 0;
        PORTFW_T portfw_delentry = {0};
        IPFILTER_T filter_delentry = {0};
	#ifdef QOS_BY_BANDWIDTH	
        IPQOS_T qos_delentry = {0};
	#endif
        DHCPRSVDIP_T dhcprsv_entry[2] = {0};
        
        /* clear dmz address */
        apmib_get( MIB_DMZ_HOST,  (void *)buffer);
        memcpy((void *)&ipAddr, buffer, 4);
        if(ipAddr.s_addr != 0 && ((ipAddr.s_addr & inLanmask_new.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)))
        {
            memset(&ipAddr, 0x00, sizeof(ipAddr));
            if ( apmib_set(MIB_DMZ_HOST, (void *)&ipAddr) == 0) {
                strcpy(errMsgBuf, "Delete DMZ host error!");
                goto setErr_tcpipLan;
            }
        }
        /* clear port forwarding table */
        if (!apmib_set(MIB_PORTFW_DELALL,(void*)&portfw_delentry))
        {
            strcpy(errMsgBuf, ("\"Delete all portfw table error!\""));
            goto setErr_tcpipLan;
        }
        /* clear ipfilter table */
        if ( !apmib_set(MIB_IPFILTER_DELALL, (void *)&filter_delentry)) {
            strcpy(errMsgBuf, ("Delete all firewall table error!"));
            goto setErr_tcpipLan;
        }
    #ifdef QOS_BY_BANDWIDTH
        /* clear qos table  */
		apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum);
		for(i=1;i<=entryNum;i++)
		{
		    memset((void*)&qos_delentry, 0x00, sizeof(qos_delentry));
			*((char *)&qos_delentry) = (char)i;
			if ( apmib_get(MIB_QOS_RULE_TBL, (void *)&qos_delentry))
			{
				if(qos_delentry.mode & QOS_RESTRICT_IP)
				{
					ipAddr.s_addr = (*((struct in_addr *)qos_delentry.local_ip_start)).s_addr;
					//diag_printf("ipAddr:%x [%s]:[%d].\n",ipAddr.s_addr, __FUNCTION__,__LINE__);				
					if(ipAddr.s_addr != 0 && ((ipAddr.s_addr & inLanmask_orig.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)))
					{
                		apmib_set(MIB_QOS_DEL, (void *)&qos_delentry);
					}
					
				}
			}
		}
    #endif
        /* change static dhcp to the new subnet */
    	if(!apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum))
    	{
            strcpy(errMsgBuf, "get mib MIB_DHCPRSVDIP_TBL_NUM fail!");
            goto setErr_tcpipLan;
    	}
        
        for (i=1; i<=entryNum; i++) {
		    memset((void*)&dhcprsv_entry, 0x00, sizeof(dhcprsv_entry));
            *((char *)&dhcprsv_entry) = (char)i;
            if(!apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&dhcprsv_entry))
            {
                strcpy(errMsgBuf, "get mib MIB_DHCPRSVDIP_TBL fail!");
                goto setErr_tcpipLan;
            }
            ipAddr.s_addr = 
            (dhcprsv_entry[0].ipAddr[0]<<24)|(dhcprsv_entry[0].ipAddr[1]<<16)|(dhcprsv_entry[0].ipAddr[2]<<8)|dhcprsv_entry[0].ipAddr[3];
            if((ipAddr.s_addr & inLanmask_orig.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr))
            {
                newipAddr.s_addr = (inLanaddr_new.s_addr & inLanmask_new.s_addr) | (ipAddr.s_addr & (~inLanmask_orig.s_addr));
                memcpy(&(dhcprsv_entry[1]),&dhcprsv_entry[0],sizeof(DHCPRSVDIP_T));                
                dhcprsv_entry[1].ipAddr[0] = (newipAddr.s_addr & 0xff000000)>>24;
                dhcprsv_entry[1].ipAddr[1] = (newipAddr.s_addr & 0x00ff0000)>>16;
                dhcprsv_entry[1].ipAddr[2] = (newipAddr.s_addr & 0x0000ff00)>>8;
                dhcprsv_entry[1].ipAddr[3] = (newipAddr.s_addr & 0x000000ff);                
                //diag_printf("ipAddr:%x newipAddr:%x [%s]:[%d].\n",ipAddr.s_addr,newipAddr.s_addr, __FUNCTION__,__LINE__);                
    			if ( !apmib_set(MIB_DHCPRSVDIP_MOD, (void *)dhcprsv_entry))
    			{
                    strcpy(errMsgBuf, "set MIB_DHCPRSVDIP_MOD fail!");
                    goto setErr_tcpipLan;
    			}
            }

        }
    }
#endif
#endif
	apmib_update_web(CURRENT_SETTING);

	submitUrl=get_cstream_var(postData,len,"submit-url","");

	if((inLanaddr_orig.s_addr & inLanmask_orig.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)||lan_dhcp_orig!=lan_dhcp)
	{
#ifdef HAVE_SYSTEM_REINIT	
		wait_redirect_home("Apply Changes",15);
		sleep(1);
		kick_reinit_m(SYS_LAN_M);
#else
		sprintf(errMsgBuf, "%s","<h4>Change setting successfully!<BR><BR>Do not turn off or reboot the Device during this time.</h4>");
		reboot_wait_redirect(errMsgBuf);
		if(errMsgBuf)
		{
			free(errMsgBuf);
			errMsgBuf=NULL;
		}
#endif		
		return 0;
	}
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	
#ifdef HAVE_SYSTEM_REINIT	
	wait_redirect_home("Apply Changes",15);
	//wait_redirect("Apply Changes",15,submitUrl);
	sleep(1);
	kick_reinit_m(SYS_LAN_M);
#else
	OK_MSG(submitUrl);
#endif

	return 0;
setErr_tcpipLan:
	ERR_MSG(errMsgBuf);
	if(errMsgBuf)
	{
		free(errMsgBuf);
		errMsgBuf=NULL;
	}
	return 0;

}
//////////////////////////////////////////////////////////////////////////////
static int getOneDhcpClient(char *buffer, char *ip, char *mac, char *liveTime)
{
	char *p=NULL;
	if(!buffer||!ip||!mac||!liveTime)
		return -1;
	//printf("%s\n",buffer);
	//1:0xf04da27de6e3:192.168.1.0:100:232
	strtok(buffer,":");
	p=strtok(NULL,":");
	strcpy(mac,p);
	p=strtok(NULL,":");
	strcpy(ip,p);
	p=strtok(NULL,":");	
	strcpy(liveTime,p);
	if(!strcmp(liveTime,"\"expired\""))
		return -1;
//	printf("ip=%s\nmac=%s\nliveTime=%s\n",ip,mac,liveTime);
	return 1;
}

int dhcpClientList(int argc, char **argv)
{
	FILE *fp;
	int nBytesSent=0;
	int element=0, ret;
	char ipAddr[40], macAddr[40], liveTime[80], *buf=NULL, *ptr, tmpBuf[100];
	
	char *tmpbuf = malloc(ASP_LIST_BUFFER_SIZE);
	if(!tmpbuf)
	{
		printf("malloc fail!\n");
		return -1;
	}
	bzero(tmpbuf,ASP_LIST_BUFFER_SIZE);
	
	buf = (char*)malloc(DHCPDB_LINESIZE_MAX);
	if( buf == NULL )
	{
		free(tmpbuf);
		printf("malloc fail!\n");
		return -1;	
	}
	bzero(buf,DHCPDB_LINESIZE_MAX);
	// siganl DHCP server to update lease file,
#ifdef HAVE_DHCPD
	extern void dump_bind_db();
	dump_bind_db();
#endif
	fp = fopen(_PATH_DHCPS_LEASES, "r");
	//printf("%s:%d\n",__FILE__,__LINE__);
	if ( fp == NULL )
		goto err;
	while(fgets(buf,DHCPDB_LINESIZE_MAX,fp))
	{
		if(getOneDhcpClient(buf,ipAddr,macAddr,liveTime)<0)
			continue;
		nBytesSent=snprintf(tmpbuf, ASP_LIST_BUFFER_SIZE,
			("<tr bgcolor=#b7b7b7><td><font size=2>%s</td><td><font size=2>%s</td><td><font size=2>%s</td></tr>"),
			ipAddr, macAddr, liveTime);
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		
		element++;
		bzero(buf,DHCPDB_LINESIZE_MAX);
	}
	//fread(buf, 1, fileSize, fp);
	fclose(fp);	
err:
	if (element == 0) {
		nBytesSent=snprintf(tmpbuf, ASP_LIST_BUFFER_SIZE,
			("<tr bgcolor=#b7b7b7><td><font size=2>None</td><td><font size=2>----</td><td><font size=2>----</td></tr>"));
		cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	}
	if (buf)
	{
		free(buf);
		buf=NULL;
	}
	if(tmpbuf) 
	{
		free(tmpbuf);
		tmpbuf=NULL;
	}

	return 0;
}


void formReflashClientTbl(char *postData, int len)
{
	char  *submitUrl=NULL;
	submitUrl=get_cstream_var(postData,len,"submit-url","");
	if (submitUrl[0])
		send_redirect_perm(submitUrl);
}

int dhcpRsvdIp_List(int argc, char **argv)
{
	int	entryNum, i;
	int nBytesSent=0;
	DHCPRSVDIP_T entry;
	char macaddr[30];
	char *tmpbuf = malloc(ASP_LIST_BUFFER_SIZE);
	if(!tmpbuf)
	{
		printf("malloc fail!\n");
		return -1;
	}
	bzero(tmpbuf,ASP_LIST_BUFFER_SIZE);
	if(!apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum))
	{
		printf("get mib MIB_DHCPRSVDIP_TBL_NUM fail!\n");
		free(tmpbuf);
		return -1;
	}
	nBytesSent=snprintf(tmpbuf, ASP_LIST_BUFFER_SIZE,
			("<tr>"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(status_ip)</script></b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(status_mac)</script></b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(tcpip_dhcp_comment)</script></b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(tcpip_dhcp_select)</script></b></font></td></tr>\n"));
	cyg_httpd_write_chunked(tmpbuf,nBytesSent);
	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if(!apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&entry))
		{
			printf("get mib MIB_DHCPRSVDIP_TBL fail!\n");
			return -1;
		}
        #ifdef IPCONFLICT_UPDATE_FIREWALL
		//diag_printf("%s:%d ipAddr=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.ipAddr)));
		*((unsigned int*)(entry.ipAddr))=get_conflict_ip(*((unsigned int*)(entry.ipAddr)));
		//diag_printf("%s:%d ipAddr=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.ipAddr)));
        #endif
		if (!memcmp(entry.macAddr, "\x0\x0\x0\x0\x0\x0", 6))
			macaddr[0]='\0';
		else
		{
			sprintf(macaddr," %02x-%02x-%02x-%02x-%02x-%02x", entry.macAddr[0], entry.macAddr[1], entry.macAddr[2], entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);
		nBytesSent=snprintf(tmpbuf, ASP_LIST_BUFFER_SIZE, ("<tr>"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
			inet_ntoa(*((struct in_addr*)entry.ipAddr)), macaddr,entry.hostName, i);
			cyg_httpd_write_chunked(tmpbuf,nBytesSent);
		}
	}
	if(tmpbuf)
	{
		free(tmpbuf);
		tmpbuf=NULL;
	}
	return 0;
}

int checkStaticIp(struct in_addr *staticIpAddr)
{
	struct in_addr lanIpAddr, netMaskAddr, startIpAddr, endIpAddr;
	if (!(getInAddr("eth0", 0, (void *)&lanIpAddr)>0 && lanIpAddr.s_addr>0))
		apmib_get( MIB_IP_ADDR,  (void *)&lanIpAddr); //save the orig lan subnet
	
	apmib_get( MIB_SUBNET_MASK,  (void *)&netMaskAddr); //save the orig lan mask

	//printf("\n%s:%d lanIpAddr.s_addr=%08x  netMaskAddr.s_addr=%08x staticIpAddr->s_addr=%08x\n",__FUNCTION__,__LINE__,lanIpAddr.s_addr,netMaskAddr.s_addr,staticIpAddr->s_addr);

	if((staticIpAddr->s_addr & netMaskAddr.s_addr) != (lanIpAddr.s_addr & netMaskAddr.s_addr))
		return 1;
			
	apmib_get( MIB_DHCP_CLIENT_START, (void *)&startIpAddr);	
	if((startIpAddr.s_addr & netMaskAddr.s_addr) != (lanIpAddr.s_addr & netMaskAddr.s_addr))
		startIpAddr.s_addr=(lanIpAddr.s_addr & netMaskAddr.s_addr) | (startIpAddr.s_addr & (~(netMaskAddr.s_addr)));

	
	//printf("\n%s:%d startIpAddr.s_addr=%08x\n",__FUNCTION__,__LINE__,startIpAddr.s_addr);
	
	//printf("\n%s:%d startIpAddr.s_addr=%08x\n",__FUNCTION__,__LINE__,startIpAddr.s_addr);
	apmib_get( MIB_DHCP_CLIENT_END, (void *)&endIpAddr);	
	if((endIpAddr.s_addr & netMaskAddr.s_addr) != (lanIpAddr.s_addr & netMaskAddr.s_addr))
		endIpAddr.s_addr=(lanIpAddr.s_addr & netMaskAddr.s_addr) | (endIpAddr.s_addr & (~(netMaskAddr.s_addr)));

	//printf("\n%s:%d endIpAddr.s_addr=%08x\n",__FUNCTION__,__LINE__,endIpAddr.s_addr);

	if((htonl(staticIpAddr->s_addr) > htonl(endIpAddr.s_addr)) || (htonl(staticIpAddr->s_addr) < htonl(startIpAddr.s_addr)) || (staticIpAddr->s_addr==lanIpAddr.s_addr))
		return 2;

	return 0;
}

int checkSameIpOrMac(struct in_addr *IpAddr, char *macAddr, int entryNum)
{
	if(IpAddr==NULL || macAddr==NULL || entryNum<1)
		return 0;
	int i;
	DHCPRSVDIP_T entry;
	struct in_addr start_ip, end_ip, router_ip, netmask;
	
	for (i=1; i<=entryNum; i++) 
	{
		*((char *)&entry) = (char)i;
		if(!apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&entry))
		{
			printf("get mib MIB_DHCPRSVDIP_TBL fail!\n");
			return -1;
		}
		if(memcmp(IpAddr, entry.ipAddr, 4)==0)
			return 1;
		if(memcmp(macAddr, entry.macAddr, 6)==0)
			return 2;		
	}	
	return 0;
}

void formStaticDHCP(char *postData, int len)
{
	char *strStp, *strIp, *strHostName, *strAddRsvIP, *strDelRsvIP, *strDelAllRsvIP, *strVal, *submitUrl;
	char tmpBuf[100];
	char buffer[100];
	int entryNum, i, stp;
	DHCPRSVDIP_T staticIPEntry, delEntry;
	struct in_addr inIp={0};
	strAddRsvIP = get_cstream_var(postData,len, ("addRsvIP"), "");
	strDelRsvIP = get_cstream_var(postData,len,("deleteSelRsvIP"), "");
	strDelAllRsvIP = get_cstream_var(postData,len, ("deleteAllRsvIP"), "");
	int retval;
//displayPostDate(wp->post_data);
	// Set static DHCP
	strStp = get_cstream_var(postData,len, ("static_dhcp"), "");
	if (strStp[0]) {
		if (strStp[0] == '0')
			stp = 0;
		else
			stp = 1;
		if ( !apmib_set(MIB_DHCPRSVDIP_ENABLED, (void *)&stp)) {
			strcpy(tmpBuf, ("Set static DHCP mib error!"));
			goto setErr_rsv;
		}
	}
	
	if (strAddRsvIP[0]) {
		memset(&staticIPEntry, '\0', sizeof(staticIPEntry));	
		strHostName = (char *)get_cstream_var(postData,len, ("hostname"), "");	
		if (strHostName[0])
			strcpy((char *)staticIPEntry.hostName, strHostName);				
		strIp = get_cstream_var(postData,len,( "ip_addr"), "");
		if (strIp[0]) {
			inet_aton(strIp, &inIp);
			memcpy(staticIPEntry.ipAddr, &inIp, 4);
		}
		strVal = get_cstream_var(postData,len, ("mac_addr"), "");
		if ( !strVal[0] ) {
	//		strcpy(tmpBuf, ("Error! No mac address to set."));
			goto setac_ret;
		}
		if (strlen(strVal)!=12 || !string_to_hex(strVal, staticIPEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_rsv;
		}
		if ( !apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_rsv;
		}
		if ( (entryNum + 1) > MAX_DHCP_RSVD_IP_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_rsv;
		}
		
		retval=checkStaticIp(&inIp);
		if(retval>0)
		{
			if(retval==1)
				strcpy(tmpBuf, ("Cannot add new entry because the ip is not the same subnet as LAN network!"));
			if(retval==2)				
				strcpy(tmpBuf, ("Invalid ip address! It must be in the lan dhcp server ip range, and is not same with router's ip!"));

			goto setErr_rsv;
		}
		retval=checkSameIpOrMac(&inIp, staticIPEntry.macAddr, entryNum);
		if(retval>0)
		{
			if(retval==1)
				strcpy(tmpBuf, ("This IP address has been setted!"));
			if(retval==2)				
				strcpy(tmpBuf, ("This MAC address has been setted!"));		

			goto setErr_rsv;
		}
		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_DHCPRSVDIP_DEL, (void *)&staticIPEntry);
		if ( apmib_set(MIB_DHCPRSVDIP_ADD, (void *)&staticIPEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_rsv;
		}
	}

	/* Delete entry */
	if (strDelRsvIP[0]) {
		if ( !apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_rsv;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);
			memset(&delEntry, '\0', sizeof(delEntry));	
			strVal = get_cstream_var(postData,len, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&delEntry) = (char)i;
				if ( !apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&delEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_rsv;
				}
				if ( !apmib_set(MIB_DHCPRSVDIP_DEL, (void *)&delEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_rsv;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllRsvIP[0]) {
		if ( !apmib_set(MIB_DHCPRSVDIP_DELALL, (void *)&staticIPEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_rsv;
		}
	}

setac_ret:
	apmib_update_web(CURRENT_SETTING);


	submitUrl = get_cstream_var(postData,len, "submit-url", "");	 // hidden page


#ifdef HAVE_SYSTEM_REINIT	
	wait_redirect("Apply Changes", 10, submitUrl);
	sleep(1);
	kick_reinit_m(SYS_LAN_M);
#else
	OK_MSG( submitUrl );
#endif
	
	return;

setErr_rsv:
	ERR_MSG(tmpBuf);
}

