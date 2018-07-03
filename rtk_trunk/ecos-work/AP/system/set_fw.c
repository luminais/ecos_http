/*
 *      Utiltiy function for setting firewall 
 *
 */

#include <sys/param.h>
//#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/ip.h>


#include "apmib.h"
#include "common.h"

#ifndef NULL_FILE
#define NULL_FILE 0
#endif
#ifndef NULL_STR
#define NULL_STR ""
#endif
#define REARRANGE_IPFW 1

#if defined (DUMMYNET)
#define CONFIG_RTL_QOS 1
#endif

#if defined (DUMMYNET) && defined(CONFIG_RTL_FREEBSD_FAST_PATH)

extern int set_QosEnabled( int enabled);
#endif

void set_ipfw_rules(char *wan_intf, char *lan_intf);
int setIpFilter(char *wan_intf, char *lan_intf);
int setPortFilter(char *wan_intf, char *lan_intf);
int setMACFilter(char *wan_intf, char *lan_intf);
int set_qos(char *wan_intf, char *lan_intf);
int setUrlFilter(char *wan_intf, char *lan_intf);


static int ipfw_no =100;
char IPFW[]="ipfw";
char ADD[]="add";
char DEL[]="delete";
char ALLOW[]= "allow";
char DENY[]= "deny";
char DIVERT[]="divert";
char FROM[]="from";
char TO[]="to";
char ANY[]="any";
char ME[]="me";
char TCP[]="tcp";
char TCPOPTIONS[]="tcpoptions";
char MSS[]="mss";
char UDP[]="udp";
char IP[]="ip";
char SMAC[]="smac";
char VIA[]="via";
char PIPE[]="pipe";
char QUEUE[]="queue";
char CONFIG[]="config";
char BANDWIDTH[]="bw";
char FSBANDWIDTH[]="qbw";
char TX[]="out";
char RX[]="in";
#define udp	1
#define tcp 2
#define all 3

#ifdef HAVE_IPV6
//#define IP6FW_START_INDEX 50
char IP6FW[]="ip6fw";
char ALL[]="all";
char RECV[]="recv";
#endif

#if defined(HAVE_NAPT)
#if defined(HAVE_NATD)
extern void ParseOption (const char* option, const char* parms);
#else
extern void rtl_parseNaptOption (const char* option, const char* parms);
#endif
#endif

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
extern int rtl_port_forwarding_enabled;
#ifdef KLD_ENABLED
int rtl_addRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short PrivatefromPort, unsigned short PrivatetoPort, unsigned short protocol);
#else
extern int rtl_addRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short protocol);
#endif
extern int rtl_flushPortFwdEntry(void);
#endif


#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
#define IPFW_TRIGGER_PORT_INDEX_MAX	(IPFW_INDEX_TRIGGER_PORT+800)
#ifndef RTL_TRIGGER_PORT_RULE_MAX
#define RTL_TRIGGER_PORT_RULE_MAX 128
#endif
struct trigger_info {
	unsigned int		ipfw_num;
	unsigned short	rproto;
	unsigned short	rportMin;
	unsigned short	rportMax;
	unsigned short	valid;
};

//#define IPFW_TRIGGER_PORT_ENTRY_MAX	20	/*!!!keep the same with RTL_TRIGGER_PORT_RULE_MAX*/
static struct trigger_info trigger_ipfw_info[RTL_TRIGGER_PORT_RULE_MAX];
extern int rtl_trigger_port_enabled;
extern void rtl_flushTriggerPortRule(void);
extern int rtl_addTriggerPortRule(unsigned short rule_no, unsigned short mproto, unsigned short rproto,
									     unsigned short mport[2], unsigned short rport[2]);
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT)
extern void rtl_hwNatOnOff(int value);

void rtl_hwNatOnOffByApp(void)
{

#if 0
#define HW_NAT_LIMIT_NETMASK 0xFFFFFF00 //for arp table 512 limitation,
//net mask at lease 255.255.255.0,or disable hw_nat
	int vlan_intVal = 0;
	int url_intVal_num=0;
	int url_intVal=0;
	int qos_intVal_num=0;
	int qos_intVal=0;
	int ivalue=0;
	int wan_type = 0;

#ifdef CONFIG_RTL_VLAN_SUPPORT
	apmib_get(MIB_VLAN_ENABLED, (void *)&vlan_intVal);
#endif
	apmib_get(MIB_URLFILTER_ENABLED,  (void *)&url_intVal);
	apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&url_intVal_num);
	apmib_get(MIB_QOS_ENABLED,  (void *)&qos_intVal);
	apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&qos_intVal_num);
	apmib_get(MIB_SUBNET_MASK,(void*)&ivalue);
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);

	if((vlan_intVal==1) ||(url_intVal==1&&url_intVal_num>0) ||(qos_intVal==1&&qos_intVal_num>0)
		||((htonl(ivalue)&HW_NAT_LIMIT_NETMASK)!=HW_NAT_LIMIT_NETMASK) || (wan_type == L2TP)||(wan_type == PPTP))
		rtl_hwNatOnOff(0);
	else
		rtl_hwNatOnOff(1);

#endif

//net mask at lease 255.255.255.0,or disable hw_nat
#define nvram_safe_get(name) (nvram_get(name) ? : "")
if (!strcmp(nvram_safe_get("lan_netmask"), "255.255.255.0"))
{
	rtl_hwNatOnOff(1);
}
else
{
	rtl_hwNatOnOff(0);
}


}

#endif

void set_dmz(char *wan_intf, char *lan_intf)
{
	int intVal = 0;
	char dmz_ipbuff[30] = {0}, *dmz_ipstr = NULL;
	char ipfw_index[10];
#if defined (REARRANGE_IPFW)		
	char toipfw_index[10];
	int qosenabled=0;
#if defined(QOS_BY_BANDWIDTH)
	apmib_get(MIB_QOS_ENABLED,  (void *)&qosenabled);
#endif
#endif
    apmib_get( MIB_DMZ_ENABLED, (void *)&intVal);
    if(intVal == 1)
    {
        apmib_get( MIB_DMZ_HOST,  (void *)dmz_ipbuff);
#ifdef IPCONFLICT_UPDATE_FIREWALL
		*((unsigned int*)(dmz_ipbuff))=get_conflict_ip(*((unsigned int*)(dmz_ipbuff))); 
#endif	
        dmz_ipstr = inet_ntoa(*((struct in_addr *)dmz_ipbuff));
        if(strcmp(dmz_ipstr, "0.0.0.0"))
        {
        	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			snprintf(dmz_ipbuff,sizeof(dmz_ipbuff),"%s/32",dmz_ipstr);
		#if defined (REARRANGE_IPFW)
		if(qosenabled)
		{
			sprintf(toipfw_index,"%d",IPFW_INDEX_QOS_NEW);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",toipfw_index, 
				"ip", FROM, ANY, TO, dmz_ipbuff, "in","via",wan_intf,NULL_STR);
		}else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "ip", 
				FROM, ANY, TO, dmz_ipbuff, "in","via",wan_intf,NULL_STR);
		#else	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "ip", 
				FROM, ANY, TO, dmz_ipbuff, "in","via",wan_intf,NULL_STR);
		#endif	
            #if defined(HAVE_NAPT)
	     #if defined(HAVE_NATD)
	     ParseOption("target_address", dmz_ipstr);
	     #else
            rtl_parseNaptOption("target_address", dmz_ipstr);
	     #endif
            #endif
        }
    }
    else
    {
	     #if defined(HAVE_NAPT)
	     #if defined(HAVE_NATD)
	     ParseOption("target_address", "0.0.0.0");
	     #else
            rtl_parseNaptOption("target_address", "0.0.0.0");
	     #endif
            #endif
    }

	
	return;
}

#ifdef KLD_ENABLED
#if HAVE_NAT_ALG
void set_alg(void)
{
	int intVal = 0;
    
	apmib_get(MIB_FTP_ALG_ENABLED, (void *)&intVal);
    if (intVal == 0)
		rtl_algOnOff("ftp", 0);
    else
		rtl_algOnOff("ftp", 1);
        
    apmib_get(MIB_TFTP_ALG_ENABLED, (void *)&intVal);
    if (intVal == 0)
		rtl_algOnOff("tftp", 0);
    else
		rtl_algOnOff("tftp", 1);

    apmib_get(MIB_PPTP_ALG_ENABLED, (void *)&intVal);
    if (intVal == 0)
		rtl_algOnOff("pptp", 0);
    else
		rtl_algOnOff("pptp", 1);

    apmib_get(MIB_L2TP_ALG_ENABLED, (void *)&intVal);
    if (intVal == 0)
		rtl_algOnOff("l2tp", 0);
    else
		rtl_algOnOff("l2tp", 1);

    apmib_get(MIB_RTSP_ALG_ENABLED, (void *)&intVal);
    if (intVal == 0)
		rtl_algOnOff("rtsp", 0);
    else
		rtl_algOnOff("rtsp", 1);

    apmib_get(MIB_SIP_ALG_ENABLED, (void *)&intVal);
    if (intVal == 0)
		rtl_algOnOff("sip", 0);
    else
		rtl_algOnOff("sip", 1);

    apmib_get(MIB_H323_ALG_ENABLED, (void *)&intVal);
    if (intVal == 0)
		rtl_algOnOff("h323", 0);
    else
		rtl_algOnOff("h323", 1);

    return;
    
}
#endif
#ifdef CONFIG_RTL_SPI_FIREWALL_SUPPORT
void set_spi_firewall(void)
{
	int intVal = 0;
    
	apmib_get(MIB_SPIFIREWALL_ENABLED, (void *)&intVal);
    if (intVal == 0)
        rtl_spifirewall_onoff(0);
    else
        rtl_spifirewall_onoff(1);

    return;
    
}
#endif
#endif

#ifdef KLD_ENABLED
int setFirewallRules(char *wan_intf,char *lan_intf, unsigned char type, int *ipfwnum)
{
	int entryNum=0, i, allow_flg = 0, ipfw_number = *ipfwnum; //, check_num=0, pingWan=0;
	FIREWALLRULE_T entry;
	char *tmpStr;
	char src_Iface[10], dst_Iface[10], portRange[50],portRange1[50], portRange2[50], srcStart[16], srcEnd[16], srcip_range[60]; 
	char dstStart[16], dstEnd[16], dstip_range[60], act[10];
	int setEnabled;
	//char *timeMatch;    
	char ipfw_index[10], ipfw_index1[10], ipfw_index2[10];
	
	apmib_get(MIB_FIREWALLRULE_NUM, (void *)&entryNum);
	apmib_get(MIB_FIREWALLRULE_ENABLED, (void *)&setEnabled);
		
	if (setEnabled == 0)
		return 0;	
		
	for (i=1; i<=entryNum; i++) 
	{
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)i;
		apmib_get(MIB_FIREWALLRULE, (void *)&entry);
		
		if(entry.enabled == 0)
			continue;
		
		if(entry.src_interface==1)
			sprintf(src_Iface, "%s", lan_intf);
		else
			sprintf(src_Iface, "%s", wan_intf);
				
			if(entry.dst_interface==1)
			sprintf(dst_Iface, "%s", lan_intf);
		else
			sprintf(dst_Iface, "%s", wan_intf);	
			
	    /* check downlink or uplink rules ? type =1 just handle uplink rules 
	        * type = 2 just handle downlink rules
             */
        if (type == 1 && !(entry.src_interface ==1 && entry.dst_interface==2))
            continue;
        if (type == 2 && !(entry.src_interface ==2 && entry.dst_interface==1))
            continue;

#ifdef IPCONFLICT_UPDATE_FIREWALL
		*((unsigned int*)(entry.srcStartAddr))=get_conflict_ip(*((unsigned int*)(entry.srcStartAddr))); 
		*((unsigned int*)(entry.srcEndAddr))=get_conflict_ip(*((unsigned int*)(entry.srcEndAddr))); 
		*((unsigned int*)(entry.dstStartAddr))=get_conflict_ip(*((unsigned int*)(entry.dstStartAddr))); 
		*((unsigned int*)(entry.dstEndAddr))=get_conflict_ip(*((unsigned int*)(entry.dstEndAddr))); 
#endif							
        
		tmpStr = inet_ntoa(*((struct in_addr *)entry.srcStartAddr));	
		sprintf(srcStart, "%s", tmpStr);

		tmpStr = inet_ntoa(*((struct in_addr *)entry.srcEndAddr));	
		sprintf(srcEnd, "%s", tmpStr);
        
		if (!strcmp(srcStart, srcEnd))//start address == end address
            sprintf(srcip_range, "%s", srcStart);
        else if (strcmp(srcStart, "0.0.0.0") && !strcmp(srcEnd, "0.0.0.0"))
            sprintf(srcip_range, "%s", srcStart);
        else if (!strcmp(srcStart, "0.0.0.0") && strcmp(srcEnd, "0.0.0.0"))
            sprintf(srcip_range, "%s", srcEnd);
        else    
            sprintf(srcip_range, "%s-%s", srcStart, srcEnd);
            
		tmpStr = inet_ntoa(*((struct in_addr *)entry.dstStartAddr));	
		sprintf(dstStart, "%s", tmpStr);
	
		tmpStr = inet_ntoa(*((struct in_addr *)entry.dstEndAddr));			
		sprintf(dstEnd, "%s", tmpStr);
        
		if (!strcmp(dstStart, dstEnd))//start address == end address
            sprintf(dstip_range, "%s", dstStart);
        else if (strcmp(dstStart, "0.0.0.0") && !strcmp(dstEnd, "0.0.0.0"))
            sprintf(dstip_range, "%s", dstStart);
        else if (!strcmp(dstStart, "0.0.0.0") && strcmp(dstEnd, "0.0.0.0"))
            sprintf(dstip_range, "%s", dstEnd);
        else    
            sprintf(dstip_range, "%s-%s", dstStart, dstEnd);

		#if 0	
		if ((timeMatch = getTimeMatchStrByScheduleName(entry.scheRule)) == NULL)
		{
			printf("\r\n Error!! parse timeMatch fail.");
			continue;
		}
        #endif
    	
    	
    	if(entry.action ==1)
        {   
			sprintf(act, "%s", "allow");//mixed blacklist and whitelist
			allow_flg = 1;
			//continue;
        }
    	else
			sprintf(act, "%s", "deny");
    		
    	if(entry.fromPort != 0 && entry.toPort != 0 && entry.toPort != entry.fromPort)
			sprintf(portRange, "%d-%d", entry.fromPort, entry.toPort); 
   	    else if ((entry.fromPort != 0 && entry.toPort == entry.fromPort) || (entry.fromPort != 0 && entry.toPort == 0))            
            sprintf(portRange, "%d", entry.fromPort);
        else
			portRange[0] = '\0';
        
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		sprintf(ipfw_index2,"%d",NEXT_IPFW_INDEX(ipfw_number));
        if (portRange[0])
        {
            if (entry.action == 1 && type == 1)//lan->wan
            {
                sprintf(portRange1, "%s", portRange);
                sprintf(portRange2, "%s", portRange);
                sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
        		if(entry.protoType == FW_PROTO_TCP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", ipfw_index1, "tcp", 
        				FROM, srcip_range, TO, dstip_range, portRange, "keep-state", NULL_STR);	
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "tcp", 
        				FROM, "any", TO, dstip_range, portRange1, NULL_STR);                   
                    sprintf(ipfw_index2,"%d",NEXT_IPFW_INDEX(ipfw_number));
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "tcp", 
        				FROM, dstip_range, portRange2, TO, "any", NULL_STR); 
        		}
                if(entry.protoType == FW_PROTO_UDP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ipfw_index, "skipto",ipfw_index1, "udp", 
        				FROM, srcip_range, TO, dstip_range, portRange, "keep-state",NULL_STR);
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "udp", 
        				FROM, "any", TO, dstip_range, portRange1, NULL_STR);
                    sprintf(ipfw_index2,"%d",NEXT_IPFW_INDEX(ipfw_number));
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "udp", 
        				FROM, dstip_range, portRange2, TO, "any",NULL_STR);
                }   
        		if(entry.protoType == FW_PROTO_ICMP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ipfw_index, "skipto", ipfw_index1,"icmp", 
        				FROM, srcip_range, TO, dstip_range, portRange, "keep-state",NULL_STR);	
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "icmp", 
        				FROM, "any", TO, dstip_range, portRange1,NULL_STR);
                    sprintf(ipfw_index2,"%d",NEXT_IPFW_INDEX(ipfw_number));
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "icmp", 
        				FROM, dstip_range, portRange2, TO, "any", NULL_STR);
        		}
            }
            else
            {
                /*  ipfw add 100 deny ip from srcip-srcip2 to  dstip-dstip2 port1-port2 out recv eth0 xmit eth1 */
        		if(entry.protoType == FW_PROTO_TCP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, act, "tcp", 
        				FROM, srcip_range, TO, dstip_range, portRange,NULL_STR);				
        		}
                if(entry.protoType == FW_PROTO_UDP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, act, "udp", 
        				FROM, srcip_range, TO, dstip_range, portRange,NULL_STR);	
                }   
        		if(entry.protoType == FW_PROTO_ICMP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, act, "icmp", 
        				FROM, srcip_range, TO, dstip_range, portRange, NULL_STR);					
        		}
            }
        }
        else
        {
            
            if (entry.action == 1 && type == 1)//lan->wan
            {
                sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
                if(entry.protoType == FW_PROTO_TCP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", ipfw_index1, "tcp", 
        				FROM, srcip_range, TO, dstip_range, "keep-state",NULL_STR);
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "tcp", 
        				FROM, "any", TO, dstip_range,NULL_STR);
                    sprintf(ipfw_index2,"%d",NEXT_IPFW_INDEX(ipfw_number));
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "tcp", 
        				FROM, dstip_range, TO, "any",NULL_STR);
        		}
                if(entry.protoType == FW_PROTO_UDP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", ipfw_index1, "udp", 
        				FROM, srcip_range, TO, dstip_range, "keep-state",NULL_STR);
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "udp", 
        				FROM, "any", TO, dstip_range,NULL_STR);
                    sprintf(ipfw_index2,"%d",NEXT_IPFW_INDEX(ipfw_number));
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "udp", 
        				FROM, dstip_range, TO, "any",NULL_STR);
                }   
        		if(entry.protoType == FW_PROTO_ICMP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", ipfw_index1, "icmp", 
        				FROM, srcip_range, TO, dstip_range, "keep-state",NULL_STR);
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "icmp", 
        				FROM, "any", TO, dstip_range, NULL_STR);
                    sprintf(ipfw_index2,"%d",NEXT_IPFW_INDEX(ipfw_number));
                    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index2, "allow", "icmp", 
        				FROM, dstip_range, TO, "any", NULL_STR);
        		}
                
            }
            else
            {
        		if(entry.protoType == FW_PROTO_TCP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, act, "tcp", 
        				FROM, srcip_range, TO, dstip_range,NULL_STR);				
        		}
                if(entry.protoType == FW_PROTO_UDP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, act, "udp", 
        				FROM, srcip_range, TO, dstip_range, NULL_STR);	
                }   
        		if(entry.protoType == FW_PROTO_ICMP || entry.protoType == FW_PROTO_ALL){
        			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, act, "icmp", 
        				FROM, srcip_range, TO, dstip_range, NULL_STR);					
        		}
            }
        }
		
	}

    if (type == 1 && allow_flg == 1)//lan->wan find whitelist deny others
    {
        sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, IP , FROM, ANY, TO, ANY, "out", "recv", lan_intf, "xmit", wan_intf, NULL_STR);
        
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_number));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, IP , FROM, ANY, TO, ANY, NULL_STR);
    }
	*ipfwnum = ipfw_number;
	return 0;	
}
#endif
#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
void set_portFwd(char *wan_intf,char *lan_intf)
{
	int i;
	unsigned long ipAddr;
	int intVal = 0;
	int tblNum = 0;
	PORTFW_T entry;
	char ipfw_index[10];
	char port[20], port1[20];
	char ip[20];
#if defined (REARRANGE_IPFW)		
	char toipfw_index[10];
	int qosenabled;
#if defined(QOS_BY_BANDWIDTH)
	apmib_get(MIB_QOS_ENABLED,	(void *)&qosenabled);
#endif
#endif

	apmib_get(MIB_PORTFW_ENABLED, (void *)&intVal);
	rtl_port_forwarding_enabled = intVal;
	if(intVal == 0)
		return;
	
	memset(&entry, '\0', sizeof(entry));
	apmib_get(MIB_PORTFW_TBL_NUM,  (void *)&tblNum);
#if defined (REARRANGE_IPFW)		
	sprintf(toipfw_index,"%d",IPFW_INDEX_QOS_NEW);
#endif
	/*Flush port forwarding table beforce add entry*/
	rtl_flushPortFwdEntry();
	for(i=1; i<=tblNum; i++)
	{
		*((char *)&entry) = (char)i;
		apmib_get(MIB_PORTFW_TBL, (void *)&entry);
        #if defined(KLD_ENABLED)
        if (entry.Enabled == 0)
            continue;
        #endif
		ipAddr = *((unsigned long*)(entry.ipAddr));	
#ifdef IPCONFLICT_UPDATE_FIREWALL
		ipAddr=get_conflict_ip(ipAddr);	
#endif
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		sprintf(ip,"%s",inet_ntoa(*(struct in_addr *)(&ipAddr)));
		/* For dlink, dport match is range, so use PrivatefromPort and  PrivatetoPort  to do DNAT
		    And port allow ipfw rule is after divert, so should use PrivatefromPort and  PrivatetoPort */
		#if defined(KLD_ENABLED)	
		sprintf(port,"%d-%d",entry.PrivatefromPort, entry.PrivatetoPort);
        sprintf(port1, "%s", port);
		#else
		sprintf(port,"%d-%d",entry.fromPort,entry.toPort);
        sprintf(port1, "%s", port);
		#endif
		
        #ifdef KLD_ENABLED
        /* PROTO_BOTH = 255 */
		if(255 == entry.protoType ||
			IPPROTO_TCP == entry.protoType){
			#if defined (REARRANGE_IPFW)
			if(qosenabled)
			{
				
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",toipfw_index, 
					 "tcp", FROM, ANY, TO, ip, port,"in","via",wan_intf,NULL_STR);
			}
			else
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
					FROM, ANY, TO, ip, port,"in","via",wan_intf,NULL_STR);
			#else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
				FROM, ANY, TO, ip, port,"in","via",wan_intf,NULL_STR);
			#endif
		}
		
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		if(255 == entry.protoType ||
			IPPROTO_UDP == entry.protoType)
			#if defined (REARRANGE_IPFW)
			if(qosenabled){
				
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",toipfw_index, 
					 "udp", FROM, ANY, TO, ip, port1,"in","via",wan_intf,NULL_STR);
			}
			else
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "udp", 
					FROM, ANY, TO, ip, port1,"in","via",wan_intf,NULL_STR);
			
			#else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "udp", 
				FROM, ANY, TO, ip, port1,"in","via",wan_intf,NULL_STR);
			#endif
//		diag_printf("----%s[%d], %d, ipAddr is 0x%x, fromPort is %d, toPort is %d, protoType is %d----\n",
//			__FUNCTION__, __LINE__, i, ipAddr, entry.fromPort, entry.toPort, entry.protoType);
		rtl_addRtlPortFwdEntry(ipAddr, entry.fromPort, entry.toPort, entry.PrivatefromPort, entry.PrivatetoPort, entry.protoType);
        #else
        if(PROTO_BOTH == entry.protoType ||
			PROTO_TCP == entry.protoType){
			#if defined (REARRANGE_IPFW)
			if(qosenabled){
				
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",toipfw_index, 
					 "tcp", FROM, ANY, TO, ip, port,"in","via",wan_intf,NULL_STR);
			}
			else
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
					FROM, ANY, TO, ip, port,"in","via",wan_intf,NULL_STR);
			#else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
				FROM, ANY, TO, ip, port,"in","via",wan_intf,NULL_STR);
			#endif
		}
		
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		if(PROTO_BOTH == entry.protoType ||
			PROTO_UDP == entry.protoType)
		{
			#if defined (REARRANGE_IPFW)
			if(qosenabled)
			{
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",toipfw_index, 
					 "udp", FROM, ANY, TO, ip, port1,"in","via",wan_intf,NULL_STR);
			}
			else
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "udp", 
					FROM, ANY, TO, ip, port1,"in","via",wan_intf,NULL_STR);
			#else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "udp", 
				FROM, ANY, TO, ip, port1,"in","via",wan_intf,NULL_STR);
			#endif
		}
		//diag_printf("----%s[%d], %d, ipAddr is 0x%x, fromPort is %d, toPort is %d, protoType is %d----\n",
			//__FUNCTION__, __LINE__, i, ipAddr, entry.fromPort, entry.toPort, entry.protoType);
		rtl_addRtlPortFwdEntry(ipAddr, entry.fromPort, entry.toPort, entry.protoType);
        #endif
	}
}
#endif


#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
int rtl_setTriggerPortIpfw(unsigned short rproto, unsigned short rportMin, unsigned short rportMax)
{
	int i;
	int index = -1;
	char port[20];
	char ipfw_index[10];	
	char wan_intf[MAX_NAME_LEN];
	char lan_intf[MAX_NAME_LEN];

	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++)
	{
		if (trigger_ipfw_info[i].valid==1) {
			if ((trigger_ipfw_info[i].rproto==rproto)&&(trigger_ipfw_info[i].rportMin==rportMin)&&
			    (trigger_ipfw_info[i].rportMax==rportMax))
			    	return 0;	/*entry already exist*/
		} else {
			if (index<0)
				index = i;
		}
	}

	if (index<0)
		return -1;	/*no empty entry*/

	if ((IPFW_INDEX_TRIGGER_PORT+2*index)>IPFW_TRIGGER_PORT_INDEX_MAX)
		return -1;	/*trigger port ipfw no out of range*/

	trigger_ipfw_info[index].valid			= 1;
	trigger_ipfw_info[index].ipfw_num		= IPFW_INDEX_TRIGGER_PORT+2*index;
	trigger_ipfw_info[index].rproto			= rproto;
	trigger_ipfw_info[index].rportMin		= rportMin;
	trigger_ipfw_info[index].rportMax		= rportMax;
	
	getInterfaces(lan_intf,wan_intf);	
	sprintf(ipfw_index, "%d", trigger_ipfw_info[index].ipfw_num);
    if (trigger_ipfw_info[index].rportMin == trigger_ipfw_info[index].rportMax)        
        sprintf(port, "%d", trigger_ipfw_info[index].rportMin);
    else
	    sprintf(port, "%d-%d", trigger_ipfw_info[index].rportMin, trigger_ipfw_info[index].rportMax);
	if (trigger_ipfw_info[index].rproto==IPPROTO_TCP) {
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
				FROM, ANY, TO, ANY, port,"in","via",wan_intf,NULL_STR);
	} else if (trigger_ipfw_info[index].rproto==IPPROTO_UDP) {
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "udp", 
				FROM, ANY, TO, ANY, port,"in","via",wan_intf,NULL_STR);
	}

	return 0;
}

int rtl_delTriggerPortIpfw(unsigned short rproto, unsigned short rportMin, unsigned short rportMax)
{
	int i;
	char ipfw_index[10];	

	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++)
	{
		if (trigger_ipfw_info[i].valid==1) {
			if ((trigger_ipfw_info[i].rproto==rproto)&&(trigger_ipfw_info[i].rportMin==rportMin)&&
			    (trigger_ipfw_info[i].rportMax==rportMax)) {
			    	sprintf(ipfw_index, "%d", trigger_ipfw_info[i].ipfw_num);
			    	trigger_ipfw_info[i].valid = 0;
				RunSystemCmd(NULL_FILE, IPFW, DEL, ipfw_index, NULL_STR);
			    	return 0;
			}
		}
	}

	return -1;
}

void rtl_flushTriggerPortIpfw(void)
{
	int i;
	char ipfw_index[10];	

	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++)
	{
		if (trigger_ipfw_info[i].valid==1) {
			trigger_ipfw_info[i].valid = 0;
			sprintf(ipfw_index, "%d", trigger_ipfw_info[i].ipfw_num);
			RunSystemCmd(NULL_FILE, IPFW, DEL, ipfw_index, NULL_STR);
		}
	}
}

#if defined(KLD_ENABLED)
void rtl_addTriggerPort(unsigned short *rule_no, unsigned short tri_protoType, unsigned short inc_protoType,unsigned short mport[2], unsigned short rport[2])
{
    unsigned short ruleno = *rule_no;
    /* protocol both = 255  */
    if (255 == tri_protoType ||IPPROTO_TCP == tri_protoType) {
		if (255 == inc_protoType || IPPROTO_TCP == inc_protoType) {
			rtl_addTriggerPortRule(ruleno, IPPROTO_TCP, IPPROTO_TCP, mport, rport);
			ruleno++;
		}

		if (255 == inc_protoType || IPPROTO_UDP == inc_protoType) {
			rtl_addTriggerPortRule(ruleno, IPPROTO_TCP, IPPROTO_UDP, mport, rport);
			ruleno++;
		}
	}

	if (255 == tri_protoType ||IPPROTO_UDP == tri_protoType) {
		if (255 == inc_protoType || IPPROTO_TCP == inc_protoType) {
			rtl_addTriggerPortRule(ruleno, IPPROTO_UDP, IPPROTO_TCP, mport, rport);
			ruleno++;
		}

		if (255 == inc_protoType || IPPROTO_UDP == inc_protoType) {
			rtl_addTriggerPortRule(ruleno, IPPROTO_UDP, IPPROTO_UDP, mport, rport);
			ruleno++;
		}
	}

    *rule_no = ruleno;

    return ;
}

void set_triggerPort(void)
{
	int i,j;
	unsigned short mport[2], rport[2];
	unsigned short rule_no = 0;
	int intVal = 0;
	int tblNum = 0;
	TRIGGERPORT_T entry;    
    unsigned char tempbuf[12] = {0};
    unsigned char *str = NULL, *token = NULL, *pcomma = NULL;

	apmib_get(MIB_TRIGGERPORT_ENABLED, (void *)&intVal);
	rtl_trigger_port_enabled = intVal;
	if(intVal == 0)
		return;
	
	memset(&entry, '\0', sizeof(entry));
	apmib_get(MIB_TRIGGERPORT_TBL_NUM,  (void *)&tblNum);

	/*Flush trigger port table beforce add entry*/
	rtl_flushTriggerPortRule();
	
	for(i=1; i<=tblNum; i++)
	{
		*((char *)&entry) = (char)i;
		apmib_get(MIB_TRIGGERPORT_TBL, (void *)&entry);
        
        if ((entry.Enabled ==0))
            continue;
        
        /* triPortRng port only support single port or port range */
        str = entry.triPortRng;
        token = strchr(str, '-');
        if (token != NULL)//port range
        {
            //sscanf(entry.triPortRng, "%d-%d", &entry.tri_fromPort, &entry.tri_toPort);
            *token = '\0';
            entry.tri_fromPort = (unsigned short)atoi(str);
            *token = '-';
            token++;
            if (token != NULL)
                entry.tri_toPort = (unsigned short)atoi(token);
            else
                entry.tri_toPort = entry.tri_fromPort;
        }
        else
        {
            entry.tri_fromPort = entry.tri_toPort = (unsigned short)atoi(entry.triPortRng);
        }
        
        /* incPortRng port only support single port or port range or port1,port2 */
        str = entry.incPortRng;
        mport[0] = entry.tri_fromPort;
		mport[1] = entry.tri_toPort;
        j = 0;
        while (*str != '\0')
        {
            if (*str == '-')
            {
                rport[0] = (unsigned short)atoi(tempbuf);
                memset(tempbuf, 0x00, sizeof(tempbuf));
                j = 0;
                str++;
                while (*str != '\0' && *str != ',')
                {
                    tempbuf[j] = *str;
                    j++;
                    str++;
                }
                rport[1] = (unsigned short)atoi(tempbuf);                
                rtl_addTriggerPort(&rule_no, entry.tri_protoType, entry.inc_protoType,mport, rport);               
                memset(tempbuf, 0x00, sizeof(tempbuf));
                j = 0;
                str++;
            }
            else if (*str == ',')
            {
                rport[0] = rport[1] = (unsigned short)atoi(tempbuf);
                memset(tempbuf, 0x00, sizeof(tempbuf));
                j = 0;
                str++;                
                rtl_addTriggerPort(&rule_no, entry.tri_protoType, entry.inc_protoType,mport, rport);
            }
            else
            {
                tempbuf[j] = *str;
                j++;
                str++;
            }
    
        }
        if (tempbuf[0])
        {
            rport[0] = rport[1] = (unsigned short)atoi(tempbuf);            
            rtl_addTriggerPort(&rule_no, entry.tri_protoType, entry.inc_protoType,mport, rport);
            memset(tempbuf, 0x00, sizeof(tempbuf));
        }
        #if 0
        mport[0] = entry.tri_fromPort;
		mport[1] = entry.tri_toPort;
		rport[0] 	= entry.inc_fromPort;
		rport[1] 	= entry.inc_toPort;
        
        rtl_addTriggerPort(&rule_no, entry.tri_protoType, entry.inc_protoType,mport, rport);
		if (PROTO_BOTH == entry.tri_protoType ||PROTO_TCP == entry.tri_protoType) {
			if (PROTO_BOTH == entry.inc_protoType || PROTO_TCP == entry.inc_protoType) {
				rtl_addTriggerPortRule(rule_no, IPPROTO_TCP, IPPROTO_TCP, mport, rport);
				rule_no++;
			}

			if (PROTO_BOTH == entry.inc_protoType || PROTO_UDP == entry.inc_protoType) {
				rtl_addTriggerPortRule(rule_no, IPPROTO_TCP, IPPROTO_UDP, mport, rport);
				rule_no++;
			}
		}

		if (PROTO_BOTH == entry.tri_protoType ||PROTO_UDP == entry.tri_protoType) {
			if (PROTO_BOTH == entry.inc_protoType || PROTO_TCP == entry.inc_protoType) {
				rtl_addTriggerPortRule(rule_no, IPPROTO_UDP, IPPROTO_TCP, mport, rport);
				rule_no++;
			}

			if (PROTO_BOTH == entry.inc_protoType || PROTO_UDP == entry.inc_protoType) {
				rtl_addTriggerPortRule(rule_no, IPPROTO_UDP, IPPROTO_UDP, mport, rport);
				rule_no++;
			}
		}
        #endif
	}
}
#endif
#endif

#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
#if defined(DOS_SUPPORT)
struct dos_param
{
	unsigned long enabled;
	unsigned long lanIp;
	unsigned long lanMask;
	unsigned int opmode;
	unsigned int syssynflood;
	unsigned int sysfinflood;
	unsigned int sysudpflood;
	unsigned int sysicmpflood;
	unsigned int pipsynflood;
	unsigned int pipfinflood;
	unsigned int pipudpflood;
	unsigned int pipicmpflood;
	unsigned int blockTime;
};

extern void rtl_setDosParam(struct dos_param *dosParam);
	
void set_dos(void)
{
	unsigned char ipBuf[10];
	unsigned char maskBuf[10];
	unsigned long dos_enabled;
	struct dos_param	dosParam;
	memset(&dosParam, 0, sizeof(struct dos_param));
	
	apmib_get(MIB_DOS_ENABLED, (void *)&(dosParam.enabled));

	if(dosParam.enabled > 0){
		apmib_get(MIB_OP_MODE, (void *)&(dosParam.opmode));
		apmib_get(MIB_DOS_SYSSYN_FLOOD, (void *)&(dosParam.syssynflood));
		apmib_get(MIB_DOS_SYSFIN_FLOOD, (void *)&(dosParam.sysfinflood));
		apmib_get(MIB_DOS_SYSUDP_FLOOD, (void *)&(dosParam.sysudpflood));
		apmib_get(MIB_DOS_SYSICMP_FLOOD, (void *)&(dosParam.sysicmpflood));
		apmib_get(MIB_DOS_PIPSYN_FLOOD, (void *)&(dosParam.pipsynflood));
		apmib_get(MIB_DOS_PIPFIN_FLOOD, (void *)&(dosParam.pipfinflood));
		apmib_get(MIB_DOS_PIPUDP_FLOOD, (void *)&(dosParam.pipudpflood));
		apmib_get(MIB_DOS_PIPICMP_FLOOD, (void *)&(dosParam.pipicmpflood));
		apmib_get(MIB_DOS_BLOCK_TIME, (void *)&(dosParam.blockTime));
		apmib_get(MIB_IP_ADDR,  (void *)ipBuf);
		dosParam.lanIp = *((unsigned long*)(ipBuf));
	  	apmib_get( MIB_SUBNET_MASK,  (void *)maskBuf);
		dosParam.lanMask = *((unsigned long*)(maskBuf));
	  	rtl_setDosParam(&dosParam);
	}else{
		rtl_setDosParam(&dosParam);
	}
}
#endif
#endif

#ifdef HAVE_IPV6
//add rule to avoid DOS attack
#ifdef SUPPORT_DEFEAT_IP_SPOOL_DOS
void setRulesToDefeatIpSpoolDos()
{
	//char ip6fw_index[10];
	
	RunSystemCmd(NULL_FILE, IP6FW, ADD, "52", ALLOW, ALL, FROM, "fe80::/64", TO, ANY, RX, RECV, "eth0", NULL_STR);

	char prefix_buf[256];
	
#ifdef HAVE_RADVD
	if(isFileExist("/etc/radvd.conf"))
	{
		FILE *fp=NULL;
		char line_buf[128];
		char *pline=NULL;
		
		if((fp=fopen("/etc/radvd.conf", "r"))!=NULL)
		{
			while(fgets(line_buf, sizeof(line_buf), fp))
			{
				line_buf[strlen(line_buf)-1]=0;
				if((pline=strstr(line_buf, "prefix"))!=NULL)
				{
					strcpy(prefix_buf, line_buf+7);					
					//RunSystemCmd(NULL_FILE, Ip6tables, ADD, INPUT, in, bInterface, _src, prefix_buf, jump, ACCEPT, NULL_STR);
					
					RunSystemCmd(NULL_FILE, IP6FW, ADD, "54", ALLOW, ALL, FROM, prefix_buf, TO, ANY, RX, RECV, "eth0", NULL_STR);
				}
			}
			fclose(fp);
		}		
	}
#endif

#ifdef DHAVE_DHCP6S
	dhcp6sCfgParam_t dhcp6sCfgParam;
	memset(&dhcp6sCfgParam, 0, sizeof(dhcp6sCfgParam));
	apmib_get(MIB_IPV6_DHCPV6S_PARAM,(void *)&dhcp6sCfgParam);
	
	if(dhcp6sCfgParam.enabled)
	{
		if(dhcp6sCfgParam.addr6PoolS && dhcp6sCfgParam.addr6PoolE)
		{
			sprintf(prefix_buf, "from %s to %s", dhcp6sCfgParam.addr6PoolS, dhcp6sCfgParam.addr6PoolE);			
			//RunSystemCmd(NULL_FILE, Ip6tables, ADD, INPUT, in, bInterface, match, ip_range, src_rnage, prefix_buf, jump, ACCEPT, NULL_STR);
			
			RunSystemCmd(NULL_FILE, IP6FW, ADD, "56", ALLOW, ALL, prefix_buf, RX, RECV, "eth0", NULL_STR);
		}
	}	
	//RunSystemCmd(NULL_FILE, Ip6tables, ADD, INPUT, in, bInterface, jump, DROP, NULL_STR);	
	
#endif	
}
#endif
#endif


#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)	
extern int rtl_delQosForNat(void);
#endif

#ifdef HAVE_IPV6FIREWALL
void set_ipv6_firewall()
{
	// when add ipv6 firewall web UI, please remove it
	RunSystemCmd(NULL_FILE, "ip6fw", "flush", NULL_STR);
	RunSystemCmd(NULL_FILE, "ip6fw", "add", "1000", "allow", "ipv6-icmp", "from", "any", "to", "any", NULL_STR);
	RunSystemCmd(NULL_FILE, "ip6fw", "add", "1001", "allow", "tcp", "from", "any", "to", "any", NULL_STR);
	RunSystemCmd(NULL_FILE, "ip6fw", "add", "1002", "allow", "udp", "from", "any", "to", "any", NULL_STR);
}
void set_ip6fw_rules(char *wan_intf, char *lan_intf)
{		
	set_ipv6_firewall();
}
#endif

#if 0
void set_nat_loopback_ipfw_rules(char *lan_subnet, char *wan_intf,char *lan_intf, int *ipfw_no)
{
	int i;
	unsigned long ipAddr;
	int intVal = 0, portfw_en = 0;
	int tblNum = 0;
	int skip_rule_no =  *ipfw_no;
	int snat_rule_no = IPFW_INDEX_NATLOOPBACK_SNAT;
	int dnat_rule_no = IPFW_INDEX_NATLOOPBACK_DNAT;
	char ipfw_index[10];
	char skipto_index[10];
	char port[24], port1[24];
	char ip[20];	
	char dmz_ipbuff[30] = {0}, *dmz_ipstr = NULL;
	struct in_addr addr, lan_addr;
	char addrstr[24], lan_addrstr[24];
	#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
	PORTFW_T entry;
	#endif			

	getInAddr(wan_intf, IP_ADDR, (void *)&addr);
	strcpy(addrstr, inet_ntoa(addr));	
	sprintf(skipto_index,"%d",IPFW_INDEX_NATLOOPBACK_DNAT); 

	#ifdef HAVE_APMIB
	if ((getInAddr("eth0", IP_ADDR, (void *)&lan_addr)==0) || (lan_addr.s_addr == 0))
		apmib_get(MIB_IP_ADDR,  (void *)&lan_addr);
	strcpy(lan_addrstr, inet_ntoa(lan_addr));
	#else
	sprintf(lan_addrstr, "192.168.1.0/24");
	#endif
	

	//dmz 	
	apmib_get( MIB_DMZ_ENABLED, (void *)&intVal);
	if(intVal == 1)
	{
		apmib_get( MIB_DMZ_HOST,  (void *)dmz_ipbuff);
		#ifdef IPCONFLICT_UPDATE_FIREWALL
		*((unsigned int*)(dmz_ipbuff))=get_conflict_ip(*((unsigned int*)(dmz_ipbuff))); 
		#endif	
		dmz_ipstr = inet_ntoa(*((struct in_addr *)dmz_ipbuff));
		if(strcmp(dmz_ipstr, "0.0.0.0"))
		{	
			
			//allow dut to lan
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(skip_rule_no)); 
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "ip", 
					FROM, lan_addrstr, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
			
			//skip rules
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(skip_rule_no));	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "ip", 
					FROM, lan_subnet, TO, addrstr, "in","via",lan_intf,NULL_STR);
			
			//dnat rules
			sprintf(ipfw_index,"%d", dnat_rule_no);	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "ip",
					FROM, lan_subnet, TO, addrstr, "in","via",lan_intf,NULL_STR);
			//snat rules
			sprintf(ipfw_index,"%d", snat_rule_no);	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "ip",
					FROM, lan_subnet, TO, dmz_ipstr, "out","xmit",lan_intf,NULL_STR);
			
			sprintf(ipfw_index,"%d", snat_rule_no);	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "ip",
					FROM, dmz_ipstr, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
		}
	}
	
	#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
	apmib_get(MIB_PORTFW_ENABLED, (void *)&portfw_en);
	if(portfw_en){
		apmib_get(MIB_PORTFW_TBL_NUM,  (void *)&tblNum);
		for(i=1; i<=tblNum; i++)
		{	
			memset(&entry, '\0', sizeof(entry));
			*((char *)&entry) = (char)i;
			apmib_get(MIB_PORTFW_TBL, (void *)&entry);
	        #if defined(KLD_ENABLED)
	        if (entry.Enabled == 0)
	            continue;
	        #endif
			ipAddr = *((unsigned long*)(entry.ipAddr));	
			#ifdef IPCONFLICT_UPDATE_FIREWALL
			ipAddr=get_conflict_ip(ipAddr);	
			#endif
			sprintf(ip,"%s",inet_ntoa(*(struct in_addr *)(&ipAddr)));
			/* For dlink, dport match is range, so use PrivatefromPort and  PrivatetoPort  to do DNAT
			    And port allow ipfw rule is after divert, so should use PrivatefromPort and  PrivatetoPort */
			memset(port, '\0', sizeof(port));
			memset(port1, '\0', sizeof(port1));
			#if defined(KLD_ENABLED)	
			sprintf(port,"%d-%d",entry.PrivatefromPort, entry.PrivatetoPort);
	        sprintf(port1, "%s", port);
			#else
			sprintf(port,"%d-%d",entry.fromPort,entry.toPort);
	        sprintf(port1, "%s", port);
			#endif

	        if(PROTO_TCP == entry.protoType
				#if defined(KLD_ENABLED)
				|| 255 == entry.protoType
				#else
				|| PROTO_BOTH == entry.protoType
				#endif
				)
			{
				//skip rules
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "tcp", 
					FROM, lan_subnet, TO, addrstr, port,"in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "tcp", 
					FROM, lan_subnet, port, TO, addrstr, "in","via",lan_intf,NULL_STR);
				//dnat rules
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
						FROM, lan_subnet, TO, addrstr, port, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
						FROM, lan_subnet, port, TO, addrstr, "in","via",lan_intf,NULL_STR);
				//snat rules
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
						FROM, lan_subnet, TO, ip, port, "out","xmit",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
						FROM, ip, port, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
				
			}
			
			if(PROTO_UDP == entry.protoType
				#if defined(KLD_ENABLED)
				|| 255 == entry.protoType
				#else
				|| PROTO_BOTH == entry.protoType
				#endif
				)
			{
				//skip rules
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "udp", 
					FROM, lan_subnet, TO, addrstr, port1,"in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "udp", 
					FROM, lan_subnet, port1, TO, addrstr, "in","via",lan_intf,NULL_STR);
				//dnat rules
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
						FROM, lan_subnet, TO, addrstr, port1, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
						FROM, lan_subnet, port1, TO, addrstr, "in","via",lan_intf,NULL_STR);
				//snat rules
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
						FROM, lan_subnet, TO, ip, port1, "out","xmit",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
						FROM, ip, port1, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
			}
		}
	}
	#endif

	*ipfw_no = skip_rule_no;

	return;
	
}
#endif

void set_ipfw_rules(char *wan_intf, char *lan_intf)
{
	int intVal_num=0;
	int intVal=0;
	char ipfw_index[10];	
	char ipfw_index1[10];
	struct in_addr addr, mask;
	char addrstr[16], maskstr[16];
	int i;
	int mask_count = 0;
	char buf[32];
/*Support wan pc ip range ping dut wan ok, need to refine after dlink web finished----jwj*/	
#ifdef KLD_ENABLED
	unsigned long ip_start;
	char *tmpStr;
	char startipAddr[16] = {0};
	char ip_info[60] = {0};
	int webAccessPort=0;
	char webAccessPortStr[16]={0};
#if defined(RTL_IPFILTER_SUPPORT_IP_RANGE)
	int wan_ip_range = 0;	
	unsigned long ip_end;	
	char endIpAddr[16] = {0};	
	char ip_info2[60] = {0};
#endif
#endif
#if defined (REARRANGE_IPFW)
	char toipfw_index[10]= {0};
	int qosenabled=0;
#endif
	#if defined(CONFIG_RTL_WEB_WAN_ACCESS_PORT)
	unsigned char tmp_buff[128] = {0};
	int web_access_port = 0;
	unsigned char port_buff[16] = {'\0'};
	#endif

	//ipfw -q -f flush
 
#if defined (CONFIG_RTL_QOS)
	rtl_delQosInfo();
#endif

	RunSystemCmd(NULL_FILE, IPFW, "-q","-f","flush", NULL_STR);

#if defined (REARRANGE_IPFW)
#if defined(QOS_BY_BANDWIDTH)
	apmib_get(MIB_QOS_ENABLED,  (void *)&qosenabled);
#endif
#endif

#ifdef HAVE_APMIB
	if ((getInAddr("eth0", IP_ADDR, (void *)&addr)==0) || (addr.s_addr == 0))
		apmib_get(MIB_IP_ADDR,  (void *)&addr);
	
	if ((getInAddr("eth0", SUBNET_MASK, (void *)&mask)==0) || (mask.s_addr == 0))
		apmib_get(MIB_SUBNET_MASK,  (void *)&mask);
	
	addr.s_addr = addr.s_addr&mask.s_addr;
	strcpy(addrstr, inet_ntoa(addr));
	for(i=0; i<32; i++)
	{
		if(((1<<i)&mask.s_addr)==0){
			mask_count++;
		}
	}
	mask_count = 32-mask_count;
	sprintf(buf, "%s/%d", addrstr, mask_count);
#else
	sprintf(buf, "192.168.1.0/24");
#endif
	
	sprintf(ipfw_index,"%d",IPFW_INDEX_ALLOW_ALL);
	//RunSystemCmd(NULL_FILE, IPFW, "add", "10", "allow", "ip", "from", "any", "to", "any", "via", lan_intf, NULL_STR);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", IP, FROM, ANY, TO, ANY, NULL_STR);

	sprintf(ipfw_index,"%d",IPFW_INDEX_NAT_UPLINK);
    /* if buf's value = 0.0.0.0/0 soure nat rule  will be wrong and may cause wan to dut packets(tcp and udp) enter fastpath */
	#ifndef CONFIG_RTL_ROUTE_MODE
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, buf, TO, ANY, TX,VIA,wan_intf,NULL_STR);
	#endif	

#ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS
    extern unsigned char alias2_wan_interface[16];
#endif

#ifdef HAVE_NAPT_NO_DYN
 	sprintf(ipfw_index,"%d",IPFW_INDEX_NAT_DOWNLINK_CHECK);
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1, IP, FROM, ANY, TO, buf, RX,VIA,wan_intf,NULL_STR);
#ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS   
    if (rtl_getDoubleAlias() != 0)
    {
	sprintf(ipfw_index,"%d",(IPFW_INDEX_NAT_DOWNLINK_CHECK+2)); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1, IP, FROM, ANY, TO, buf, RX,VIA,alias2_wan_interface,NULL_STR);
    }
#endif	
#endif

    #ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS
    if (rtl_getDoubleAlias() != 0)
    {
    	sprintf(ipfw_index,"%d",IPFW_INDEX_NAT_UPLINK2);
    	#ifndef CONFIG_RTL_ROUTE_MODE
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, buf, TO, ANY, TX,VIA,alias2_wan_interface,NULL_STR);
		#endif
    	//RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, ANY, TO, ANY, TX,VIA,alias2_wan_interface,NULL_STR);
    }
    #endif
    
	sprintf(ipfw_index,"%d",IPFW_INDEX_DENY_DOWNLINK_ALL);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY,IP,FROM,ANY,TO,ANY,RX,VIA,wan_intf,NULL_STR);
	
	ipfw_no=IPFW_INDEX_START;     
    #ifdef KLD_ENABLED    
    int ipfwnum = IPFW_INDEX_FIREWALL_RULES;
	/* add fifty uplink firewall rules */
	apmib_get(MIB_FIREWALLRULE_ENABLED,  (void *)&intVal);
	if(intVal != 0){		
		//set firewall rules
		setFirewallRules(wan_intf, lan_intf, 1, &ipfwnum);
	}
    #endif
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW,UDP,FROM,ANY,"67",TO,ANY,"68",NULL_STR);	
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW,UDP,FROM,ANY,"68",TO,ANY,"67",NULL_STR);

    #ifdef KLD_ENABLED    
    intVal = 0;
	apmib_get(MIB_MACFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&intVal_num);
	if(intVal != 0 && intVal_num>0){
		//set mac filter		
		setMACFilter(wan_intf,lan_intf);
	}
    #endif
    
    #ifndef KLD_ENABLED
	//set url filter
	intVal = 0;
	apmib_get(MIB_URLFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&intVal_num);
	
	if((intVal!=0) && intVal_num>0)	
	{
		
		setUrlFilter(wan_intf,lan_intf);
	}
    #endif
    
	//nat loopback  rules...
	#if defined(CONFIG_RTL_NAT_LOOPBACK)
	set_nat_loopback_ipfw_rules(buf, wan_intf, lan_intf, &ipfw_no);
	#endif	
	
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW,IP,FROM,ANY,TO,"me",RX,VIA,lan_intf,NULL_STR);
    #ifdef KLD_ENABLED
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW,IP,FROM,"me",TO,ANY,"out","xmit",lan_intf,NULL_STR);    
    #endif
    /* deny udp dst:239.255.255.250 dst port:1900 packets in receive from wan for ssdp */
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	if (strncmp(wan_intf, "eth1", strlen("eth1")))
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY,UDP,FROM,"any",TO,"239.255.255.250/32", "1900","in","recv","eth1",NULL_STR);
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY,UDP,FROM,"any",TO,"239.255.255.250/32", "1900","in","recv",wan_intf,NULL_STR);
    
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
#ifdef KLD_ENABLED   	
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	/*allow multicast packet*/
	#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
		RunSystemCmd(NULL_FILE, "ipfw", "add", ipfw_index, "allow", "ip", "from", "any", "to", "224.0.0.0/4",NULL_STR);
	#else
		RunSystemCmd(NULL_FILE, "ipfw", "add", ipfw_index, "allow", "ip", "from", "any", "to", "224.0.0.0/4",NULL_STR);
	#endif
#endif
#endif
	apmib_get( MIB_VPN_PASSTHRU_L2TP_ENABLED, (void *)&intVal);
	if(intVal==0){
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, UDP, FROM, ANY, "to", ANY, "1701", "in", "recv", lan_intf, NULL_STR);
	}else{
		#if 0
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, UDP, FROM, ANY, "to", ANY, "1701", "in", "recv", lan_intf, NULL_STR);
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, UDP, FROM, ANY, "1701", "to", ANY, "in", "recv", lan_intf, NULL_STR);
		#endif
	}

	apmib_get( MIB_VPN_PASSTHRU_PPTP_ENABLED, (void *)&intVal);
	if(intVal==0){
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, TCP, FROM, ANY, "to", ANY, "1723", "in", "recv", lan_intf, NULL_STR);
	}else{
		#if 0
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, TCP, FROM, ANY, "to", ANY, "1723", "in", "recv", lan_intf, NULL_STR);
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, TCP, FROM, ANY, "1723", "to", ANY, "in", "recv", lan_intf, NULL_STR);
        #endif        
		sprintf(ipfw_index,"%d",IPFW_INDEX_PPTP_ALLOW_RECV_FROM_WAN);
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, "47", FROM, ANY, "to", ANY, "in", "recv", wan_intf, NULL_STR);
	}

	apmib_get(MIB_VPN_PASSTHRU_IPSEC_ENABLED, (void *)&intVal);
	if(intVal ==0){
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, UDP, FROM, ANY, "to", ANY, "500", "in", "recv", lan_intf, NULL_STR);
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, UDP, FROM, ANY, "500", "to", ANY, "in", "recv", lan_intf, NULL_STR);
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "50", FROM, ANY, "to", ANY, NULL_STR);
	}else{
		sprintf(ipfw_index,"%d", IPFW_INDEX_IPSEC_ALLOW_RECV_FROM_WAN);
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, UDP, FROM, ANY, "to", ANY, "500", "in", "recv", wan_intf, NULL_STR);
		sprintf(ipfw_index,"%d", IPFW_INDEX_IPSEC_ALLOW_RECV_FROM_WAN+2);
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, "50", FROM, ANY, "to", ANY, "in", "recv", wan_intf, NULL_STR);
	}
	
#if 0//def CONFIG_RTL_SUPPORT_DOUBLE_ALIAS
		if (rtl_getDoubleAlias() != 0)
		{
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, ANY, TO, ANY, RX,VIA,alias2_wan_interface,NULL_STR);
		}
#endif
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "check-state", NULL_STR);
	
#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
	ipfw_no=IPFW_INDEX_STATE_NOT_MATCH;
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "icmp", 
			FROM, ANY, TO, "me", "in","via",wan_intf,"icmptypes","8",NULL_STR);
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "icmp", 
			FROM, ANY, TO, "me", "in","via",wan_intf,NULL_STR);	
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	#if defined (REARRANGE_IPFW)
	
	if (qosenabled){
		sprintf(toipfw_index,"%d",IPFW_INDEX_QOS_NEW);
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",toipfw_index, 
			 IP,FROM,ANY,TO,ANY,RX,"recv",wan_intf,"fragment",NULL_STR);
	}else
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow",IP,FROM,ANY,TO,ANY,RX,"recv",wan_intf, "fragment", NULL_STR);
	#else
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow",IP,FROM,ANY,TO,ANY,RX,"recv",wan_intf, "fragment", NULL_STR);
	#endif	
#endif
	ipfw_no=IPFW_INDEX_NAT_DOWNLINK_BASE;
	//ipfw add 30 divert natd ip from any to any in via eth1
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	sprintf(ipfw_index1,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,TCP,FROM,ANY,TO,ANY,RX,VIA,wan_intf,TCPOPTIONS, MSS,NULL_STR);	

	sprintf(ipfw_index,"%s",ipfw_index1); 
	#ifndef CONFIG_RTL_ROUTE_MODE
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, ANY, TO, ANY, RX,VIA,wan_intf,NULL_STR);
    #endif

	/* To support TR069, by cairui*/
#if defined(HAVE_TR069)
	unsigned int cwmp_enabled=0;
	unsigned int tr069_port_no=0;
	unsigned int temp=ipfw_no;
	if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_enabled) && apmib_get(MIB_CWMP_CONREQ_PORT, (void*)&tr069_port_no) )
	{
		if(cwmp_enabled&CWMP_FLAG_AUTORUN) // CWMP_FLAG_AUTORUN=0x20, means tr069 enabled
		{
			char port_index[10];
			// add cmd: ipfw add 10003 allow tcp from any to me 7547 in recv eth1
			ipfw_no+=1;
			sprintf(ipfw_index,"%d", ipfw_no);
			sprintf(port_index,"%d", tr069_port_no);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", FROM, ANY, TO, "me", port_index, RX, "recv", wan_intf, NULL_STR);
				
			ipfw_no=temp;
		}
	}
#endif

#ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS
    if (rtl_getDoubleAlias() != 0)
    {
        sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));  
		#ifndef CONFIG_RTL_ROUTE_MODE
        RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, ANY, TO, ANY, RX,VIA,alias2_wan_interface,NULL_STR);
		#endif
    }
#endif

	/*Support wan pc ip range ping dut wan ok, need to refine after dlink web finished----jwj*/
#ifdef KLD_ENABLED
    /* anti spoofing */
	apmib_get( MIB_ANTI_SPOOFING, (void *)&intVal);
	if(intVal != 0){
        char ip_buff[32] = {0};
    #ifdef HAVE_APMIB
        sprintf(ip_buff, "%s/%d", addrstr, mask_count);
    #else       
        sprintf(ip_buff, "192.168.1.0/24");
    #endif
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, IP, FROM, ip_buff, "to", ANY, "in", "recv", wan_intf, NULL_STR);
	}else{
	}
    /* add fifty downlink firewall rules */
	apmib_get(MIB_FIREWALLRULE_ENABLED,  (void *)&intVal);
	if(intVal != 0){		
		//set firewall rules
		setFirewallRules(wan_intf, lan_intf, 2, &ipfwnum);
	}
#if 0
#if defined(RTL_IPFILTER_SUPPORT_IP_RANGE)
	if (wan_ip_range) {
		ip_start = 0xc0a80264;
		ip_end  = 0xc0a802c8;
		tmpStr = inet_ntoa(ip_start);
		sprintf(startipAddr, "%s", tmpStr);
        	tmpStr = inet_ntoa(ip_end);
		sprintf(endIpAddr, "%s", tmpStr);

	       	 if (!strcmp(startipAddr, endIpAddr))//start address == end address
	            sprintf(ip_info, "%s", startipAddr);
	        else if (strcmp(startipAddr, "0.0.0.0") && !strcmp(endIpAddr, "0.0.0.0"))
	            sprintf(ip_info, "%s", startipAddr);
	        else if (!strcmp(startipAddr, "0.0.0.0") && strcmp(endIpAddr, "0.0.0.0"))
	            sprintf(ip_info, "%s", endIpAddr);
	        else    
	            sprintf(ip_info, "%s-%s", startipAddr, endIpAddr);
	}
#else
	apmib_get(MIB_REMOTE_ACCESS_IP_START,(void*)&ip_start);
	tmpStr = inet_ntoa(ip_start);
	sprintf(ip_info, "%s", tmpStr);
	apmib_get(MIB_REMOTE_ACCESS_PORT,(void*)&webAccessPort);
	sprintf(webAccessPortStr,"%d",webAccessPort);
#endif
#endif
#endif
	apmib_get( MIB_PING_WAN_ACCESS_ENABLED, (void *)&intVal);
	if (intVal){
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		#if defined(RTL_IPFILTER_SUPPORT_IP_RANGE)&&defined(KLD_ENABLED)
		if (wan_ip_range) {
			strcpy(ip_info2, ip_info);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "icmp", 
				FROM, ip_info2, TO, "me", "in","via",wan_intf,"icmptypes","8",NULL_STR);
		} else 
		#endif
		{
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "icmp", 
				FROM, ANY, TO, "me", "in","via",wan_intf,"icmptypes","8",NULL_STR);
		}
	}
	
	apmib_get( MIB_WEB_WAN_ACCESS_ENABLED, (void *)&intVal);
    #ifdef KLD_ENABLED
    apmib_get(MIB_REMOTE_ACCESS_IP_START,(void*)&ip_start);
	tmpStr = inet_ntoa(ip_start);
	sprintf(ip_info, "%s", tmpStr);
	apmib_get(MIB_REMOTE_ACCESS_PORT,(void*)&webAccessPort);
	sprintf(webAccessPortStr,"%d",webAccessPort);
    #endif
	if (intVal){
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
#ifdef KLD_ENABLED

    {
        if (strcmp(ip_info, "0.0.0.0") == 0)
        {
            RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
            FROM, "any", TO, "me","80", "in","via",wan_intf,NULL_STR);
        }
        else
        {
            RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
            FROM, ip_info, TO, "me","80", "in","via",wan_intf,NULL_STR);
        }
        //diag_printf("%s:%d %s %s %s allow tcp %s %s %s me %s in via %s\n",__FUNCTION__,__LINE__,IPFW,ADD,ipfw_index,FROM,
        //  ip_info,TO,webAccessPortStr,wan_intf);
    }

#if 0
#if defined(RTL_IPFILTER_SUPPORT_IP_RANGE)
		if (wan_ip_range) {
			strcpy(ip_info2, ip_info);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
				FROM, ip_info2, TO, "me","80", "in","via",wan_intf,NULL_STR);
		}
#else
		{
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
				FROM, ip_info, TO, "me","80", "in","via",wan_intf,NULL_STR);
			//diag_printf("%s:%d %s %s %s allow tcp %s %s %s me %s in via %s\n",__FUNCTION__,__LINE__,IPFW,ADD,ipfw_index,FROM,
			//	ip_info,TO,webAccessPortStr,wan_intf);
		}
#endif
#endif
#else
		{
			#if defined(CONFIG_RTL_WEB_WAN_ACCESS_PORT)
			apmib_get( MIB_WEB_WAN_ACCESS_PORT, (void *)&web_access_port);
			sprintf(port_buff, "%d", web_access_port);

			if (((web_access_port>=1) && (web_access_port<=65535)) && (web_access_port != 80)){
				//deny the default port 80 when web access port set to another port
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "deny", "tcp", 
					FROM, ANY, TO, "me","80", "in","via",wan_intf,NULL_STR);
			}
			
			sprintf(tmp_buff, "%s", "127.0.0.1,80");
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "fwd",tmp_buff, "tcp", FROM, ANY, TO, "me", port_buff, "in","via",wan_intf,NULL_STR);

			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			#endif
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
				FROM, ANY, TO, "me","80", "in","via",wan_intf,NULL_STR);

		}	
#endif
	}

    #ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    apmib_get( MIB_NET_SNIPER_ENABLED, (void *)&intVal);
	if (intVal){
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, UDP, 
			FROM, ANY, TO, ANY,"161", "in","via",wan_intf,NULL_STR);
        sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, UDP, 
			FROM, ANY, TO, ANY,"162", "in","via",wan_intf,NULL_STR);
        rtl_netsniperOnOff(0xffffffff);
	}
    else
    {
        rtl_netsniperOnOff(0);
    }
    #endif
#ifdef HAVE_TELNETD
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	#if defined(RTL_IPFILTER_SUPPORT_IP_RANGE)&&defined(KLD_ENABLED)
	if (wan_ip_range) {
		strcpy(ip_info2, ip_info);
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
			FROM, ip_info2, TO, "me","23", "in","via",wan_intf,NULL_STR);
	} else 
	#endif
	{
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", 
			FROM, ANY, TO, "me","23", "in","via",wan_intf,NULL_STR);
	}
#endif

#if HAVE_NAPT
	set_dmz(wan_intf,lan_intf);
#endif

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
	set_portFwd(wan_intf,lan_intf);
#endif

#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)&&defined(KLD_ENABLED)
	set_triggerPort();
#endif

#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
#if defined(DOS_SUPPORT)
	set_dos();
#endif
#endif

#ifdef KLD_ENABLED
#if HAVE_NAT_ALG
    set_alg();
#endif
#ifdef CONFIG_RTL_SPI_FIREWALL_SUPPORT
    set_spi_firewall();
#endif
#endif

	ipfw_no=IPFW_INDEX_FILTER;		
	intVal = 0;
	apmib_get(MIB_IPFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_IPFILTER_TBL_NUM,  (void *)&intVal_num);
	if(intVal ==1 && intVal_num>0){
		//set ip filter
		setIpFilter(wan_intf,lan_intf);
	}

	intVal=0;
	apmib_get(MIB_PORTFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_PORTFILTER_TBL_NUM, (void *)&intVal_num);
	if(intVal==1 && intVal_num>0){
		setPortFilter(wan_intf,lan_intf);
	}	

    #ifndef KLD_ENABLED
	intVal = 0;
	apmib_get(MIB_MACFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&intVal_num);
	if(intVal != 0 && intVal_num>0){
		//set mac filter
		setMACFilter(wan_intf,lan_intf);
	}
	   	
	#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
	/*allow multicast packet after fw filter*/
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
			RunSystemCmd(NULL_FILE, "ipfw", "add", ipfw_index, "allow", "ip", "from", "any", "to", "224.0.0.0/4",NULL_STR);
	#else
			RunSystemCmd(NULL_FILE, "ipfw", "add", ipfw_index, "allow", "ip", "from", "any", "to", "224.0.0.0/4",NULL_STR);
	#endif

	#endif
	
    #endif
	#if 0
	intVal = 0;
	apmib_get(MIB_URLFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&intVal_num);
	if(intVal==1 && intVal_num>0){
		//set mac filter
		setUrlFilter(wan_intf,lan_intf);
	}
	#endif
#if defined (REARRANGE_IPFW)
	if (qosenabled)
		ipfw_no=IPFW_INDEX_QOS_NEW;
	
#else
	ipfw_no=IPFW_INDEX_QOS;
#endif
#if defined(QOS_BY_BANDWIDTH)

	intVal = 0;
	apmib_get(MIB_QOS_ENABLED,  (void *)&intVal);
	apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&intVal_num);
	
#if defined (DUMMYNET) && defined(CONFIG_RTL_FREEBSD_FAST_PATH)

	set_QosEnabled(intVal);
	set_Qosintf(wan_intf,lan_intf);
#endif
	//diag_printf("intVal:%d,intVal_num:%d,[%s]:[%d].\n",intVal,intVal_num,__FUNCTION__,__LINE__);
	
	if(intVal==1 )

	
	{
		//set mac filter
		set_qos(wan_intf,lan_intf);
		
	}
#endif

#if 0
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)


	/*allow multicast packet
	ipfw add 2500 allow ip from  any to 224.0.0.0/4
	*/
	#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
	RunSystemCmd(NULL_FILE, "ipfw", "add", "60", "allow", "ip", "from", "any", "to", "224.0.0.0/4",NULL_STR);
	#else
	RunSystemCmd(NULL_FILE, "ipfw", "add", "2500", "allow", "ip", "from", "any", "to", "224.0.0.0/4",NULL_STR);
	#endif
#endif
#endif
	/*
	ipfw add 200 divert natd ip from 192.168.1.0/24 to any out via eth1
	ipfw add 01100 check-state
	ipfw add 01101 allow all from  any to any in via eth0 keep-state
	ipfw add 01102 allow all from  any to any out via eth1 keep-state
	ipfw add 01103 deny tcp from any to any established in via eth1
	*/

    	ipfw_no=IPFW_INDEX_ALG;
    /*
         ipfw rule numbers 3000~3999 reserved for alg.
        */
	ipfw_no=IPFW_INDEX_SKIP_TO_NAT_UPLINK_BASE;
 
#ifdef HAVE_NAPT_NO_DYN
#else
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "check-state", NULL_STR);
#endif
	
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	sprintf(ipfw_index1,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,TCP,FROM,ANY,TO,ANY,TX,VIA,wan_intf,TCPOPTIONS, MSS,NULL_STR);	
	sprintf(ipfw_index,"%s",ipfw_index1); 
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
#ifdef 	HAVE_NAPT_NO_DYN
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,IP,FROM,ME,TO,ANY,TX,VIA,wan_intf,"keep-state",NULL_STR);	

	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no)); 	
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,IP,FROM,ANY,TO,ANY,TX,VIA,wan_intf, NULL_STR);	
	
#else
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,IP,FROM,ANY,TO,ANY,TX,VIA,wan_intf,"keep-state", NULL_STR);	
#endif
    #ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS    
    if (rtl_getDoubleAlias() != 0)
    {
    	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
    	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK2); 
    	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,IP,FROM,ANY,TO,ANY,TX,VIA,alias2_wan_interface, NULL_STR);	
    }
    #endif
    
    #ifdef KLD_ENABLED
	//set url filter
	intVal = 0;
	apmib_get(MIB_URLFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&intVal_num);	
	if((intVal!=0) && intVal_num>0)	
	{
	    ipfw_no = IPFW_INDEX_URL_FILTER;		
		setUrlFilter(wan_intf,lan_intf);
	}
    #endif
    #if 0//def KLD_ENABLED
	intVal = 0;
	apmib_get(MIB_MACFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&intVal_num);
	if(intVal != 0 && intVal_num>0){
		//set mac filter		
        ipfw_no = IPFW_INDEX_MAC_FILTER;
		setMACFilter(wan_intf,lan_intf);
	}
    #endif

	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_hwNatOnOffByApp();
	#endif

#ifdef HAVE_IPV6
//add rule to avoid DOS attack
#ifdef SUPPORT_DEFEAT_IP_SPOOL_DOS
	setRulesToDefeatIpSpoolDos();
#endif
#endif
}

#if defined(QOS_BY_BANDWIDTH)
#ifdef KLD_ENABLED
int set_qos(char *wan_intf, char *lan_intf)
{
		char  pipe_index[32]={0}, band_info[32]={0}, ip_info[64]={0},port_info[32]={0};

		char  ip_from[32]={0};
#ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
		char  ip_to[32]={0};
#endif
		char tmp_args3[64]={0};
		char *tmpStr=NULL;
		int i=1;
		int QoS_Enabled=0,qos_mode=0;
		int QoS_Auto_Uplink=0, QoS_Manual_Uplink=0;
		int QoS_Auto_Downlink=0, QoS_Manual_Downlink=0;
		int QoS_Rule_EntryNum=0;
		int uplink_speed=102400, downlink_speed=102400;
		IPQOS_T entry;
		int get_wanip=0;
		char ipfw_index[10];
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_8367R)
		uplink_speed=1024000;
		downlink_speed=1024000;
#endif

		apmib_get( MIB_QOS_ENABLED, (void *)&QoS_Enabled);
		apmib_get( MIB_QOS_MODE, (void *)&qos_mode);
		apmib_get( MIB_QOS_AUTO_UPLINK_SPEED, (void *)&QoS_Auto_Uplink);
		apmib_get( MIB_QOS_MANUAL_UPLINK_SPEED, (void *)&QoS_Manual_Uplink);
		apmib_get( MIB_QOS_MANUAL_DOWNLINK_SPEED, (void *)&QoS_Manual_Downlink);
		apmib_get( MIB_QOS_AUTO_DOWNLINK_SPEED, (void *)&QoS_Auto_Downlink);
		apmib_get( MIB_QOS_RULE_TBL_NUM, (void *)&QoS_Rule_EntryNum);
		//diag_printf("QoS_Enabled:%d,QoS_Rule_EntryNum:%d.[%s]:[%d].\n",QoS_Enabled,QoS_Rule_EntryNum,__FUNCTION__,__LINE__);
		//downlink
		if(QoS_Enabled==1){
			
			if(QoS_Auto_Downlink!=0){

				//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
				downlink_speed=QoS_Manual_Downlink;
				if(downlink_speed < 100)
					downlink_speed=100;

				/*DOWNlink*/
				sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
				sprintf(band_info,"%dkbits/s",QoS_Manual_Downlink);
				
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "10", IP, FROM , ANY, TO, ANY, RX, VIA, wan_intf, NULL_STR);								
				RunSystemCmd(NULL_FILE, IPFW, "pipe", "10", "config", BANDWIDTH,band_info, NULL_STR);
			}
			else/*QoS_Auto_Uplink ==0*/
			{
				//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);
				downlink_speed=QoS_Manual_Downlink;
				if(downlink_speed < 100)
					downlink_speed=100;
				
				for (i=1; i<=QoS_Rule_EntryNum; i++) {
					*((char *)&entry) = (char)i;
					apmib_get(MIB_QOS_RULE_TBL, (void *)&entry);
					if(entry.enabled)
					{
						sprintf(pipe_index,"%d",4*i);	/*pipe index*/
						if(entry.mode & QOS_RESTRICT_IP)
						{
#ifdef IPCONFLICT_UPDATE_FIREWALL
							//diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
							*((unsigned int*)(entry.local_ip_start))=get_conflict_ip(*((unsigned int*)(entry.local_ip_start))); 
							*((unsigned int*)(entry.local_ip_end))=get_conflict_ip(*((unsigned int*)(entry.local_ip_end))); 
							//diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
#endif

							if(((struct in_addr *)entry.local_ip_start)->s_addr)
								tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));/*source ip*/
							else
								tmpStr = ANY;
							sprintf(ip_from,"%s",tmpStr);
							
							#ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
							if(((struct in_addr *)entry.local_ip_end)->s_addr)
								tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_end));/*source ip*/
							else
								tmpStr = ANY;
							sprintf(ip_to,"%s",tmpStr);
							#endif
							
							//set ip
                            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
                            if((strcmp(ip_from,ANY)==0)||(strcmp(ip_from,ip_to)==0))
                                sprintf(ip_info,"%s",ip_from);
                            else
                                sprintf(ip_info,"%s-%s",ip_from,ip_to);
                            #else
                                sprintf(ip_info,"%s",ip_from);
                            #endif
							
							if(entry.bandwidth)
							{
								sprintf(band_info,"%dkbits/s",entry.bandwidth);
				
							}
							
							if (entry.mode & QOS_RESTRICT_MAX)
							{

								RunSystemCmd(NULL_FILE, IPFW, PIPE, pipe_index, CONFIG, BANDWIDTH, band_info, NULL_STR);
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
								RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, pipe_index, IP, FROM ,ANY , TO, ip_info, RX,VIA, wan_intf, NULL_STR);
								
							}
							
							else if	(entry.mode & QOS_RESTRICT_MIN)
							{
							
							}
						
						}
						
					}
						
						
				}	
				#if 1
				sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
				sprintf(band_info,"%dkbits/s",QoS_Manual_Downlink);
				sprintf(ip_info,"%s","192.168.1.255:0.0.0.255");
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "10", IP, FROM , ANY, TO,"not",ip_info , RX, VIA, wan_intf, NULL_STR);								
				RunSystemCmd(NULL_FILE, IPFW, "pipe", "10", "config", BANDWIDTH,band_info, NULL_STR);
				#endif
			}	
			
			
		}
	
		return 0;
}

#else
int set_qos(char *wan_intf, char *lan_intf)
{
		char tmp_args[32]={0}, tmp_args1[32]={0}, tmp_args2[32]={0}, tmp_args4[32]={0};
		char tmp_args3[64]={0};
		char *tmpStr=NULL;
		int i=1;
		int QoS_Enabled=0,qos_mode=0;
		int QoS_Auto_Uplink=0, QoS_Manual_Uplink=0;
		int QoS_Auto_Downlink=0, QoS_Manual_Downlink=0;
		int QoS_Rule_EntryNum=0;
		int uplink_speed=102400, downlink_speed=102400;
		IPQOS_T entry;
		int get_wanip=0;
		char ipfw_index[10];
		char queuesize[32]={0};
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_8367R)
		uplink_speed=1024000;
		downlink_speed=1024000;
#endif
		int weightsum=0;
		int quenum=0;
		int entryQosMode=0;
        int queue_size=0;
		
		apmib_get( MIB_QOS_ENABLED, (void *)&QoS_Enabled);
		apmib_get( MIB_QOS_MODE, (void *)&qos_mode);
		apmib_get( MIB_QOS_AUTO_UPLINK_SPEED, (void *)&QoS_Auto_Uplink);
		apmib_get( MIB_QOS_MANUAL_UPLINK_SPEED, (void *)&QoS_Manual_Uplink);
		apmib_get( MIB_QOS_MANUAL_DOWNLINK_SPEED, (void *)&QoS_Manual_Downlink);
		apmib_get( MIB_QOS_AUTO_DOWNLINK_SPEED, (void *)&QoS_Auto_Downlink);
		apmib_get( MIB_QOS_RULE_TBL_NUM, (void *)&QoS_Rule_EntryNum);
		//diag_printf("QoS_Enabled:%d,QoS_Rule_EntryNum:%d,qos_mode:%d.[%s]:[%d].\n",QoS_Enabled,QoS_Rule_EntryNum,qos_mode,__FUNCTION__,__LINE__);
		
		if(QoS_Enabled==1){
			
			if(QoS_Auto_Uplink==0){
				uplink_speed=QoS_Manual_Uplink;
				if(uplink_speed < 100)
					uplink_speed=100;
			}
	
			if(QoS_Auto_Downlink==0){
				downlink_speed=QoS_Manual_Downlink;
				if(downlink_speed < 100)
					downlink_speed=100;
			}
			/*wfq or bandwidth shaping?*/	
			if(qos_mode){			
				/*wfq
				ipfw add 2001 queue 1 tcp from 192.168.1.122 5566 to any in via eth0 
				ipfw add 2002 queue 2 tcp from 192.168.1.122 5588 to any in via eth0
				ipfw pipe 10 config bw 30Mbits/s 
				ipfw queue 1 config pipe 10 weight 8
				ipfw queue 2 config pipe 10 weight 2
				*/
				if(QoS_Rule_EntryNum > 0){
					/*in wfq we use fix pipe number 10 for uplink and pipe 20 for downlink*/
					#if 1//uplink
					
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Uplink);
					//sprintf(queuesize,"%dk",(QoS_Manual_Uplink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Uplink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);			
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "10", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);
					#endif
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Downlink);
					
					//sprintf(queuesize,"%dk",(QoS_Manual_Downlink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Downlink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "20", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);
					
					for (i=1; i<=QoS_Rule_EntryNum; i++) {
						*((char *)&entry) = (char)i;
						apmib_get(MIB_QOS_RULE_TBL, (void *)&entry);
						entryQosMode=0;
						if(entry.mode & QOS_WFQ_SCH)
							entryQosMode=1;
						if(entry.enabled > 0 && entry.weight> 0){
								/*UPlink*/
								#if 1
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
								sprintf(tmp_args2,"%d",4*i);	/*queue index*/
								if(entry.mode & QOS_RESTRICT_IP)
								{
									/*this qos rule is set by IP address*/
#ifdef IPCONFLICT_UPDATE_FIREWALL
									//diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
									*((unsigned int*)(entry.local_ip_start))=get_conflict_ip(*((unsigned int*)(entry.local_ip_start))); 
									*((unsigned int*)(entry.local_ip_end))=get_conflict_ip(*((unsigned int*)(entry.local_ip_end))); 
									//diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
#endif									

									if(((struct in_addr *)entry.local_ip_start)->s_addr)
										tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));/*source ip*/
									else
										tmpStr = ANY;
									
									if((entry.local_port_start)&&(entry.local_port_end))
									{
										sprintf(tmp_args1, "%d",entry.local_port_start);	/*source port (start)*/
										if(entry.local_port_start==entry.local_port_end)
										{
											
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, "both", FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+2);	/*queue index*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#endif
											}
										}
										else
										{
											sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);
											/*source port (end)*/
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM , tmpStr, tmp_args4,  TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM , tmpStr, tmp_args4,	TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,"both", FROM , tmpStr, tmp_args4, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM , tmpStr, tmp_args4, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+2);	/*queue index*/
												sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);	/*source port (end)*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM , tmpStr, tmp_args4, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#endif		
											}
										}
									}
									else{
										/*ipfw add 100 pipe 1 ip from any to any out via eth1*/	
										if(entry.protocol == udp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , tmpStr, TO, ANY, RX,VIA, lan_intf, NULL_STR);
										else if(entry.protocol == tcp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , tmpStr, TO, ANY, RX,VIA, lan_intf, NULL_STR);
										else{
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , tmpStr, TO, ANY, RX,VIA,lan_intf, NULL_STR);
										}
									}
								}
								else if(entry.mode & QOS_RESTRICT_MAC){
									sprintf(tmp_args3, "%02x%02x%02x%02x%02x%02x",entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
									RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, SMAC, tmp_args3, RX, VIA, lan_intf, NULL_STR);
								}
								else //any
								{								
									RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);								
								}
								/*ipfw queue 2 config pipe 10 weight 2 */
								sprintf(tmp_args,"%d",100*entry.weight);	/*weight magnify for default queue*/
								RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "10", "weight",tmp_args,NULL_STR);
								if(entry.protocol == all) {
									sprintf(tmp_args2,"%d",4*i);	/*queue index*/
									RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "10", "weight",tmp_args,NULL_STR);
								}								
								#endif
								/*DOWNlink*/
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
								sprintf(tmp_args2,"%d",4*i+1);		/*queue index*/
								if(entry.mode & QOS_RESTRICT_IP)
								{
									/*this qos rule is set by IP address*/
									if(((struct in_addr *)entry.local_ip_start)->s_addr)
										tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));
									else
										tmpStr = ANY;
				
									if((entry.local_port_start)&&(entry.local_port_end))
									{
										sprintf(tmp_args1, "%d", entry.local_port_start);	/*dest ip and port*/
										if(entry.local_port_start==entry.local_port_end)
										{
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, "both", FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+3);		/*queue index*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
												#endif
											}
										}
										else
										{
											sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);	/*source port (end)*/
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,"both", FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+3);	/*queue index*/
												sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);	/*source port (end)*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
												#endif
											}
										}
									}
									else{									
										/*ipfw add 100 pipe 1 ip from any to any out via eth1*/		
										if(entry.protocol == udp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , ANY, TO, tmpStr, RX, VIA, wan_intf, NULL_STR);
										else if(entry.protocol == tcp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , ANY, TO, tmpStr, RX, VIA, wan_intf, NULL_STR);
										else{
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, tmpStr, RX, VIA, wan_intf, NULL_STR);
										}	
									}
									
								}	
								else if(entry.mode & QOS_RESTRICT_MAC){
									sprintf(tmp_args3, "%02x%02x%02x%02x%02x%02x",entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
									RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, "dmac", tmp_args3, RX, VIA, wan_intf, NULL_STR);
								}

								/*ipfw queue 2 config pipe 20 weight 2 */	
								sprintf(tmp_args,"%d",100*entry.weight);		/*weight magnify for default queue*/
								RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "20", "weight",tmp_args,NULL_STR);
								if(entry.protocol == all){
									sprintf(tmp_args2,"%d",4*i+1);		/*queue index*/
									RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "20", "weight",tmp_args,NULL_STR);
								}
						}
					
					}

					/*default queue...*/
					
					/*UPlink*/				
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					sprintf(tmp_args2,"%d",4*i);	/*queue index*/
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);

					RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "10", "weight","1",NULL_STR);

					/*Downlink*/				
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					sprintf(tmp_args2,"%d",4*i+1);	/*queue index*/
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, ANY, RX, VIA, wan_intf, NULL_STR);

					RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "20", "weight","1",NULL_STR);
				}				
				else 
				{	/*only do bandwidth shaping*/
					/*default queue...*/
					
					/*UPlink*/				
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Uplink);
					//sprintf(queuesize,"%dk",(QoS_Manual_Uplink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Uplink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "10", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);
					
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, "1", IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);
	
					RunSystemCmd(NULL_FILE, IPFW, QUEUE, "1", CONFIG, PIPE, "10", "weight","1",NULL_STR);
	
					/*Downlink*/				
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Downlink);
					//sprintf(queuesize,"%dk",(QoS_Manual_Downlink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Downlink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "20", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);

					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, "2", IP, FROM , ANY, TO, ANY, RX, VIA, wan_intf, NULL_STR);

					RunSystemCmd(NULL_FILE, IPFW, QUEUE, "2", CONFIG, PIPE, "20", "weight", "1", NULL_STR);
				}
			}
			

			else{	/*strict priority...*/
					if(QoS_Rule_EntryNum > 0){
					/*in sp we use fix pipe number 30 for uplink and pipe 40 for downlink*/
					#if 1//uplink

					/*by chloe...*/
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Uplink);
					//sprintf(queuesize,"%dk",(QoS_Manual_Uplink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Uplink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "30", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);
					#endif
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Downlink);
					//sprintf(queuesize,"%dk",(QoS_Manual_Downlink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Downlink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "40", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);
					
					for (i=1; i<=QoS_Rule_EntryNum; i++) {
						*((char *)&entry) = (char)i;
						apmib_get(MIB_QOS_RULE_TBL, (void *)&entry);
						entryQosMode=0;
						if(entry.mode & QOS_WFQ_SCH)
							entryQosMode=1;
						if(entry.enabled > 0 && entry.prior > 0)
						{
			/*UPlink*/
							if(entry.bandwidth > 0)
							{
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
								sprintf(tmp_args2,"%d",4*i);	/*queue index*/
								if(entry.mode & QOS_RESTRICT_IP)
								{
									/*this qos rule is set by IP address*/
#ifdef IPCONFLICT_UPDATE_FIREWALL
									//diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
									*((unsigned int*)(entry.local_ip_start))=get_conflict_ip(*((unsigned int*)(entry.local_ip_start))); 
									*((unsigned int*)(entry.local_ip_end))=get_conflict_ip(*((unsigned int*)(entry.local_ip_end))); 
									//diag_printf("%s:%d local_ip_start=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
#endif									

									if(((struct in_addr *)entry.local_ip_start)->s_addr)
										tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));/*source ip*/
									else
										tmpStr = ANY;
									
									if((entry.local_port_start)&&(entry.local_port_end))
									{
										sprintf(tmp_args1, "%d",entry.local_port_start);	/*source port (start)*/
										if(entry.local_port_start==entry.local_port_end)
										{
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, "both", FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+2);	/*queue index*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , tmpStr,tmp_args1, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#endif
											}
										}
										else
										{
											sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);
											/*source port (end)*/
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM , tmpStr, tmp_args4,  TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM , tmpStr, tmp_args4,	TO, ANY, RX,VIA, lan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,"both", FROM , tmpStr, tmp_args4, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM , tmpStr, tmp_args4, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+2);	/*queue index*/
												sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);	/*source port (end)*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM , tmpStr, tmp_args4, TO, ANY, RX,VIA, lan_intf, NULL_STR);
												#endif		
											}
										}
									}
									else{
										/*ipfw add 100 pipe 1 ip from any to any out via eth1*/	
										if(entry.protocol == udp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , tmpStr, TO, ANY, RX,VIA, lan_intf, NULL_STR);
										else if(entry.protocol == tcp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , tmpStr, TO, ANY, RX,VIA, lan_intf, NULL_STR);
										else{
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , tmpStr, TO, ANY, RX,VIA,lan_intf, NULL_STR);
										}
									}
								}
								else if(entry.mode & QOS_RESTRICT_MAC){
									sprintf(tmp_args3, "%02x%02x%02x%02x%02x%02x",entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
									RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, SMAC, tmp_args3, RX, VIA, lan_intf, NULL_STR);
								}
								else //any
								{								
									RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);								
								}
								/*ipfw queue 2 config pipe 30 qbw 30M*/
								sprintf(tmp_args,"%dkbits",entry.bandwidth);
								sprintf(tmp_args1,"%d",entry.prior );
								RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "30", FSBANDWIDTH,tmp_args, "priority", tmp_args1, NULL_STR);

								#if 0
								if(entry.protocol == all) {
									sprintf(tmp_args2,"%d",4*i);	/*queue index*/
									RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "30", "qbw",tmp_args, NULL_STR);
								}	
								#endif
							}

							if(entry.bandwidth_downlink > 0)
							{			
								/*DOWNlink*/
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
								sprintf(tmp_args2,"%d",4*i+1);		/*queue index*/
								if(entry.mode & QOS_RESTRICT_IP)
								{
									/*this qos rule is set by IP address*/
									if(((struct in_addr *)entry.local_ip_start)->s_addr)
										tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));
									else
										tmpStr = ANY;
				
									if((entry.local_port_start)&&(entry.local_port_end))
									{
										sprintf(tmp_args1, "%d", entry.local_port_start);	/*dest ip and port*/
										if(entry.local_port_start==entry.local_port_end)
										{
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, "both", FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+3);		/*queue index*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , ANY, TO, tmpStr, tmp_args1, RX, VIA, wan_intf, NULL_STR);
												#endif
											}
										}
										else
										{
											sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);	/*source port (end)*/
											if(entry.protocol == udp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
											else if(entry.protocol == tcp)
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
											else{
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,"both", FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
												#if 0
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,TCP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
												sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
												sprintf(tmp_args2,"%d",4*i+3);	/*queue index*/
												sprintf(tmp_args4,"%d-%d",entry.local_port_start,entry.local_port_end);	/*source port (end)*/
												RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2,UDP, FROM ,ANY, TO, tmpStr,  tmp_args4, RX,VIA, wan_intf, NULL_STR);
												#endif
											}
										}
									}
									else{									
										/*ipfw add 100 pipe 1 ip from any to any out via eth1*/		
										if(entry.protocol == udp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, UDP, FROM , ANY, TO, tmpStr, RX, VIA, wan_intf, NULL_STR);
										else if(entry.protocol == tcp)
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, TCP, FROM , ANY, TO, tmpStr, RX, VIA, wan_intf, NULL_STR);
										else{
											RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, tmpStr, RX, VIA, wan_intf, NULL_STR);
										}	
									}
									
								}	
								else if(entry.mode & QOS_RESTRICT_MAC){
									sprintf(tmp_args3, "%02x%02x%02x%02x%02x%02x",entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
									RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, "dmac", tmp_args3, RX, VIA, wan_intf, NULL_STR);
								}

								/*ipfw queue 2 qbw 30M config pipe 40 */	
								sprintf(tmp_args,"%dkbits",entry.bandwidth_downlink);
								sprintf(tmp_args1,"%d",entry.prior );
									
								RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "40", FSBANDWIDTH,tmp_args, "priority", tmp_args1, NULL_STR);

								
								#if 0
								if(entry.protocol == all){
									sprintf(tmp_args2,"%d",4*i+1);		/*queue index*/
									RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "40", "qbw",tmp_args1, "priority", tmp_args, NULL_STR);
								}	
								#endif 
							}
						}
						#if 0
						else if(entryQosMode == 0)
						{
							//diag_printf("sp prior = 0\n");
							#if 1
							if(entry.protocol == udp)
							{
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
								RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "30", UDP, FROM ,ANY, TO,  ANY, RX, VIA, lan_intf, NULL_STR);

								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));								
								RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "40", UDP, FROM ,ANY, TO,	ANY, RX, VIA, lan_intf, NULL_STR);		
							}
							else if(entry.protocol == tcp)
							{
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
								RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "30", TCP, FROM ,ANY, TO,  ANY, RX, VIA, lan_intf, NULL_STR);

								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));								
								RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "40", TCP, FROM ,ANY, TO,	ANY, RX, VIA, lan_intf, NULL_STR);
							
							}
							else
							{
								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));								
								RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "30", IP, FROM ,ANY, TO,	ANY, RX, VIA, lan_intf, NULL_STR);

								sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));								
								RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "40", IP, FROM ,ANY, TO,	ANY, RX, VIA, lan_intf, NULL_STR);								
							}
							#endif
						
							#if 0
							sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
							RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "30", IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);								

							sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
							RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "40", IP, FROM , ANY, TO, ANY, RX, VIA, wan_intf, NULL_STR);								
							#endif
							
						}
						#endif
					}

					/*default queue...*/
					
					/*UPlink*/				
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					sprintf(tmp_args2,"%d",4*i+2);	/*queue index*/
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);

					sprintf(tmp_args,"%dkbits",QoS_Manual_Uplink);				
					RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "30", FSBANDWIDTH,tmp_args, "priority", "0", NULL_STR);

					/*Downlink*/				
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					sprintf(tmp_args2,"%d",4*i+3);	/*queue index*/
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, tmp_args2, IP, FROM , ANY, TO, ANY, RX, VIA, wan_intf, NULL_STR);
				
					sprintf(tmp_args,"%dkbits",QoS_Manual_Downlink);														
					RunSystemCmd(NULL_FILE, IPFW, QUEUE, tmp_args2, CONFIG, PIPE, "40", FSBANDWIDTH,tmp_args, "priority", "0", NULL_STR);

				}

				else 
				{	/*only do bandwidth shaping*/
					#if 0
					/*UPlink*/
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Uplink);
					
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "30", IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);								
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "30", "config", BANDWIDTH,tmp_args2, NULL_STR);
					/*DOWNlink*/
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Downlink);
					
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, "40", IP, FROM , ANY, TO, ANY, RX, VIA, wan_intf, NULL_STR);								
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "40", "config", BANDWIDTH,tmp_args2, NULL_STR);
					#endif

					/*UPlink*/				
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Uplink);
					//sprintf(queuesize,"%dk",(QoS_Manual_Uplink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Uplink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "30", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);

					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, "1", IP, FROM , ANY, TO, ANY, RX, VIA, lan_intf, NULL_STR);
			
					RunSystemCmd(NULL_FILE, IPFW, QUEUE, "1", CONFIG, PIPE, "30", FSBANDWIDTH,tmp_args2, "priority", "0", NULL_STR);
					
					/*Downlink*/				
					sprintf(tmp_args2,"%dkbits/s",QoS_Manual_Downlink);
					//sprintf(queuesize,"%dk",(QoS_Manual_Downlink/(20*8)));
					// queue size must be < 1MB: ecos-work/AP/ipfw/ipfw.c
					queue_size = QoS_Manual_Downlink/(20*8);
					if(queue_size > 1024)
                        queue_size = 1024;
					sprintf(queuesize,"%dk",queue_size);
					
					RunSystemCmd(NULL_FILE, IPFW, "pipe", "40", "config", BANDWIDTH,tmp_args2,QUEUE,queuesize, NULL_STR);

					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, QUEUE, "2", IP, FROM , ANY, TO, ANY, RX, VIA, wan_intf, NULL_STR);

					RunSystemCmd(NULL_FILE, IPFW, QUEUE, "2", CONFIG, PIPE, "40", FSBANDWIDTH,tmp_args2, "priority", "0", NULL_STR);
					
				}
			}
			
			#ifndef CONFIG_RTL_FREEBSD_FAST_PATH
			/* total bandwidth section--uplink*/
			/*ipfw add 100 pipe 1 ip from any to any out via eth1*/
			/*ipfw pipe 1 config bw 20Mbit/s */
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			sprintf(tmp_args2,"%d",4*i);	/*pipe index*/
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, tmp_args2, IP, FROM , ANY, TO, ANY, TX,VIA, wan_intf, NULL_STR);
			sprintf(tmp_args,"%dKbit/s",uplink_speed);
			RunSystemCmd(NULL_FILE, IPFW, PIPE, tmp_args2, CONFIG, BANDWIDTH, tmp_args, NULL_STR);
			
			/* total bandwidth section--downlink*/
			/*ipfw add 200 pipe 2 ip from any to any out via eth0*/
			/*ipfw pipe 2 config bw 10Mbit/s */
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			sprintf(tmp_args2,"%d",4*i+1);	/*pipe index*/
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, PIPE, tmp_args2, IP, FROM , ANY, TO, ANY, TX,VIA, lan_intf, NULL_STR);
			sprintf(tmp_args,"%dKbit/s",downlink_speed);
			RunSystemCmd(NULL_FILE, IPFW, PIPE, tmp_args2, CONFIG, BANDWIDTH, tmp_args, NULL_STR);
			#endif
		}
	
		return 0;
}
#endif
#endif

int setUrlFilter(char *wan_intf, char *lan_intf)
{
	char urlEntry[32];
	int entryNum=0, index;
	URLFILTER_T entry;
	char ipfw_index[10];
#ifdef KLD_ENABLED
	int fltMode = 0;
    //char ipfw_index2[10];
    //sprintf(ipfw_index2, "%d", IPFW_INDEX_NAT_UPLINK);
#endif
	apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&entryNum);
#ifdef KLD_ENABLED
	//FltMode: 0 disable, 1 deny, 2 allow
	apmib_get(MIB_URLFILTER_ENABLED,  (void *)&fltMode);
	//diag_printf("fltMode:%d,[%s]:[%d].\n",fltMode,__FUNCTION__,__LINE__);
	if (fltMode == 2)
	{
		for (index=1; index<=entryNum; index++) {
			memset(&entry, '\0', sizeof(entry));
			*((char *)&entry) = (char)index;
			apmib_get(MIB_URLFILTER_TBL, (void *)&entry);
            if (!entry.enabled)
                continue;
			strcpy(urlEntry,entry.urlAddr);
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			// ipfw add 100 deny url www.baidu.com
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, "url" , urlEntry, NULL_STR);			
			//RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", ipfw_index2, "url" , urlEntry, "keep-state",NULL_STR);
		}	
		
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "tcp",FROM, ANY, TO, ANY,"53", NULL_STR);
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "udp",FROM, ANY, TO, ANY,"53", NULL_STR);
	}
	else if (fltMode == 1)
#endif	
	{
		
		for (index=1; index<=entryNum; index++) {
			memset(&entry, '\0', sizeof(entry));
			*((char *)&entry) = (char)index;
			apmib_get(MIB_URLFILTER_TBL, (void *)&entry);
            #ifdef KLD_ENABLED
            if (!entry.enabled)
                continue;
            #endif
			strcpy(urlEntry,entry.urlAddr);
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			// ipfw add 100 deny url www.baidu.com
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "url" , urlEntry, NULL_STR);
		}
	}
	return 0;

}

int setMACFilter(char *wan_intf, char *lan_intf)
{
	char macEntry[30];
	int entryNum=0, index;
	MACFILTER_T entry;
	char ipfw_index[10];
    #ifdef KLD_ENABLED
    int enable = 0;
    unsigned char action[6] = {0};
    char ipfw_index2[10];
    apmib_get(MIB_MACFILTER_ENABLED, (void *)&enable);
    sprintf(ipfw_index2, "%d", IPFW_INDEX_NAT_UPLINK);
    #if 0
    if (enable == 1)//allow
        snprintf(action, sizeof(action), "%s", ALLOW);
    else
        snprintf(action, sizeof(action), "%s", DENY);
    #endif
    #endif
	
	apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&entryNum);

	for (index=1; index<=entryNum; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_MACFILTER_TBL, (void *)&entry);
        #ifdef KLD_ENABLED
        if (!entry.Enabled)
            continue;
        #endif
		sprintf(macEntry,"%02X%02X%02X%02X%02X%02X", 
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2], entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		// ipfw add 100 deny smac 0023cd9ecc2e
		#ifdef KLD_ENABLED
        if (enable == 1)
		    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", ipfw_index2, "smac" , macEntry,"keep-state", NULL_STR);
        else
		    RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "smac" , macEntry, NULL_STR);
        #else
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "smac" , macEntry, NULL_STR);
        #endif
	}
    #ifdef KLD_ENABLED
    if (enable == 1)//allow
    {
        #if 0
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, IP , FROM, ANY, TO, ANY, "in", "recv", wan_intf, NULL_STR);
        sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, IP , FROM, ANY, TO, ANY, "out", "xmit", lan_intf, NULL_STR);        
        #endif
        sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, IP , FROM, ANY, TO, ANY,"in", "recv", lan_intf, NULL_STR);
    }
    #endif
    
	return 0;

}


int setIpFilter(char *wan_intf, char *lan_intf)
{
	int entryNum=0, index;
	IPFILTER_T entry;
	char ipAddr[30];
    #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
	char endIpAddr[16] = {0};
    char ipInfo[64] = {0};
    #endif
	char *tmpStr;
	char ipfw_index[10];
	apmib_get(MIB_IPFILTER_TBL_NUM, (void *)&entryNum);

	for(index=1; index <= entryNum ; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_IPFILTER_TBL, (void *)&entry);

#ifdef IPCONFLICT_UPDATE_FIREWALL               
		*((unsigned int*)(entry.ipAddr))=get_conflict_ip(*((unsigned int*)(entry.ipAddr)));
#endif
		tmpStr = inet_ntoa(*((struct in_addr *)entry.ipAddr));
		sprintf(ipAddr, "%s", tmpStr);
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
#ifdef IPCONFLICT_UPDATE_FIREWALL               
		*((unsigned int*)(entry.ipAddrEnd))=get_conflict_ip(*((unsigned int*)(entry.ipAddrEnd)));
#endif

        tmpStr = inet_ntoa(*((struct in_addr *)entry.ipAddrEnd));
		sprintf(endIpAddr, "%s", tmpStr);
        if (!strcmp(ipAddr, endIpAddr))//start address == end address
            sprintf(ipInfo, "%s", ipAddr);
        else if (strcmp(ipAddr, "0.0.0.0") && !strcmp(endIpAddr, "0.0.0.0"))
            sprintf(ipInfo, "%s", ipAddr);
        else if (!strcmp(ipAddr, "0.0.0.0") && strcmp(endIpAddr, "0.0.0.0"))
            sprintf(ipInfo, "%s", endIpAddr);
        else    
            sprintf(ipInfo, "%s-%s", ipAddr, endIpAddr);
        #endif

		// ipfw add 100 deny ip from any to any out recv ed0 xmit ed1
		if(entry.protoType==PROTO_TCP){
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));            
            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index,DENY, "tcp", FROM, ipInfo, TO, ANY, NULL_STR);
            #else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index,DENY, "tcp", FROM, ipAddr, TO, ANY, NULL_STR);
            #endif
		}
		if(entry.protoType==PROTO_UDP){
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index,DENY, "udp", FROM, ipInfo, TO, ANY, NULL_STR);
            #else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index,DENY, "udp", FROM, ipAddr, TO, ANY, NULL_STR);
            #endif
		}
		if(entry.protoType==PROTO_BOTH)	{
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "ip", FROM, ipInfo, TO, ANY, NULL_STR);
            #else
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "ip", FROM, ipAddr, TO, ANY, NULL_STR);
            #endif
		}
	}
	return 0;
}

int setPortFilter(char *wan_intf, char *lan_intf)
{
	char PortRange[30];
	char port[30];
	//int DNS_Filter=0;
	int entryNum=0,index;
	PORTFILTER_T entry;
	char ipfw_index[10];
	apmib_get(MIB_PORTFILTER_TBL_NUM, (void *)&entryNum);
	for (index=1; index<=entryNum; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_PORTFILTER_TBL, (void *)&entry);
		if(entry.fromPort<entry.toPort)
			sprintf(PortRange, "%d-%d", entry.fromPort, entry.toPort);
		else
			sprintf(PortRange, "%d", entry.fromPort);
		// ipfw add 100 deny ip from any to any out recv ed0 xmit ed1

		if(entry.protoType==PROTO_TCP){
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "tcp",FROM, ANY, TO, ANY,PortRange, NULL_STR);
		}
				
		if(entry.protoType==PROTO_UDP){
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			strcpy(port,PortRange);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "udp",FROM,ANY, TO, ANY,port, NULL_STR);			
			/*udp need to deny srcport*/
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			strcpy(port,PortRange);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "udp",FROM,ANY, port,TO, ANY, NULL_STR);			
		
		}
		
		if(entry.protoType==PROTO_BOTH)	{			
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			/*!!!!warn!!!! RunSystemCmd can change the parameter's value*/
			strcpy(port,PortRange);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "udp",FROM, ANY, TO, ANY,port, NULL_STR);	

			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
			strcpy(port,PortRange);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "udp",FROM, ANY,port, TO, ANY, NULL_STR);	
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));	
			strcpy(port,PortRange);
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "tcp",FROM, ANY, TO, ANY,port, NULL_STR);			
		}	
	}
	return 0;
}


