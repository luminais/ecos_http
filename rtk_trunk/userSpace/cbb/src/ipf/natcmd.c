#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <iflib.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <net/if_var.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "netinet/ip_compat.h"
#include "netinet/ip_fil.h"
#include "netinet/ip_nat.h"
#include "ipf.h"
#include "netinet/ip_state.h"
#include "netinet/ip_proxy.h"
#include "ip_filfast.h"
#include <netconf.h>
#include <nvparse.h>
#include "natcmd.h"
#include <sys/syslog.h>
extern int rtl_setAliasAddrByInfName(char *name);
extern int wan_primary_ifunit(void);
extern int rtl_flushPortFwdEntry(void);
extern int rtl_addRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short PrivatefromPort, unsigned short PrivatetoPort, unsigned short protocol);
extern int rtl_delRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short protocol);
char index_hash[1000];
static int nat_flag_set(char *ifname, int flag, int on)
{
	struct ifreq ifr;
	int rc = 0;
	int s;
	s = socket(AF_INET, SOCK_RAW, 0);
	if (s < 0)
		return -1;
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(s, SIOCGIFFLTFLAGS, &ifr) != 0) {
		rc = -1;
		goto out;
	}
	ifr.ifr_flags &= ~flag;
	if (on)
		ifr.ifr_flags |= flag;
	if (ioctl(s, SIOCSIFFLTFLAGS, &ifr)) {
		rc = -1;
		goto out;
	}
out:
	close(s);
	return rc;
}
static char *ipnat_wanifname(char *wan_ifname)
{
	int unit = wan_primary_ifunit();
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
	    nvram_match(strcat_r(prefix, "proto", tmp), "static") ||
	    nvram_match(strcat_r(prefix, "proto", tmp), "8021x")) {
		strcpy(wan_ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	}
	else {
		sprintf(wan_ifname, "ppp%d", unit);
	}
	return wan_ifname;
}

static char *ipnat_lanifname(int lanid, char *lan_ifname)
{
	char tmp[100];
	char *value;

	if (lan_ifname == 0)
		return 0;
	if (lanid == 0) {
		value = nvram_get("lan_ifname");
	} else {
		snprintf(tmp, sizeof(tmp), "lan%x_ifname", lanid);
		value = nvram_get(tmp);
	}
	if (value == 0)
		return 0;
	strncpy(lan_ifname, value, IFNAMSIZ);
	lan_ifname[IFNAMSIZ] = 0;
	return lan_ifname;
}
/*add by lq about nat loopback */
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
	char skipto_snat_index[10];
	char port[24], port1[24];
	char *ip;	
	char dmz_ipbuff[30] = {0}, *dmz_ipstr = NULL;
	struct in_addr addr;
	char* lan_addr;
	char addrstr[24], lan_addrstr[24];
	char* value;
	int port_fwd_en=0;
	int tcp_set_rule = 0;
	int udp_set_rule = 0;
	netconf_nat_t nat;		
	int protocol; 
	getInAddr(wan_intf, IP_ADDR, (void *)&addr);
	strcpy(addrstr, inet_ntoa(addr));	
	sprintf(skipto_index,"%d",IPFW_INDEX_NATLOOPBACK_DNAT); 
	sprintf(skipto_snat_index,"%d",IPFW_INDEX_NATLOOPBACK_SNAT); 
	if(ipnat_lanifname(0, lan_intf) == 0)
		return ;
	if (ipnat_wanifname(wan_intf) == 0)
		return;
	lan_addr = nvram_safe_get("lan_ipaddr");
	strcpy(lan_addrstr, lan_addr);
	value = nvram_get(ADVANCE_DMZ_IPADDR_EN);
	if(value && atoi(value))
	{
		dmz_ipstr = nvram_get(ADVANCE_DMZ_IPADDR);
		if(strcmp(dmz_ipstr, "0.0.0.0"))
		{	
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(skip_rule_no)); 
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "ip", 
					FROM, lan_addrstr, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(skip_rule_no));	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "ip", 
					FROM, lan_subnet, TO, addrstr, "in","via",lan_intf,NULL_STR);

			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(skip_rule_no));	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_snat_index, "ip", 
					FROM, lan_subnet, TO, dmz_ipstr, "out","xmit",lan_intf,NULL_STR);

			sprintf(ipfw_index,"%d", dnat_rule_no);	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "ip",
					FROM, lan_subnet, TO, addrstr, "in","via",lan_intf,NULL_STR);
			sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "ip",
					FROM, lan_subnet, TO, dmz_ipstr, "out","xmit",lan_intf,NULL_STR);
			sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(snat_rule_no));	
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "ip",
					FROM, dmz_ipstr, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
		}
	}
	for(i=0; i<=VTS_MAX_NUM_1; i++)
	{	
		memset(&nat, 0, sizeof(nat));
		if (get_forward_port(i, &nat) && !(nat.match.flags & NETCONF_DISABLED)) 
		{
			if(port_fwd_en == 0)
			{
					sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(skip_rule_no)); 
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "ip", 
					FROM, lan_addrstr, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
					port_fwd_en = 1;
			}
			memset(port, '\0', sizeof(port));
			memset(port1, '\0', sizeof(port1));
			sprintf(port, "%u-%u",nat.match.dst.ports[0],nat.match.dst.ports[0]);
			sprintf(port1,"%s",port);
			ip = inet_ntoa(nat.ipaddr);
			if(!strncmp(nat.match.ipproto_str,"tcp/udp",strlen("tcp/udp")))
				protocol = PROTO_BOTH;
			else if(!strncmp(nat.match.ipproto_str,"tcp",strlen("tcp")))
				protocol = PROTO_TCP;
			else
				protocol = PROTO_UDP;
	        	if(PROTO_TCP == protocol || PROTO_BOTH == protocol)
			{
				if (tcp_set_rule == 0){
					//skip rules
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "tcp", 
						FROM, lan_subnet, TO, addrstr,"in","via",lan_intf,NULL_STR);
					
					//skip rules
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_snat_index, "tcp", 
						FROM, lan_subnet, TO, lan_subnet,"out","xmit",lan_intf,NULL_STR);
					
					//dnat rules
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
							FROM, lan_subnet, TO, addrstr, "in","via",lan_intf,NULL_STR);
					
					//snat rules
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
							FROM, lan_subnet, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
					tcp_set_rule = 1;
				}
				#if 0
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "tcp", 
					FROM, lan_subnet, TO, addrstr, port,"in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "tcp", 
					FROM, lan_subnet, port, TO, addrstr, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
					FROM, lan_subnet, TO, addrstr, port, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
					FROM, lan_subnet, port, TO, addrstr, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
					FROM, lan_subnet, TO, ip, port, "out","xmit",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "tcp",
					FROM, ip, port, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
				#endif
			}
			if(PROTO_UDP == protocol || PROTO_BOTH == protocol)
			{
				if (udp_set_rule == 0){
					//skip rules
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "udp", 
						FROM, lan_subnet, TO, addrstr, "in","via",lan_intf,NULL_STR);
					
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_snat_index, "udp", 
						FROM, lan_subnet, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
					
					//dnat rules
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
							FROM, lan_subnet, TO, addrstr, "in","via",lan_intf,NULL_STR);
					
					//snat rules
					sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
					RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
							FROM, lan_subnet, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
					udp_set_rule = 1;
				}
				#if 0
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "udp", 
					FROM, lan_subnet, TO, addrstr, port1,"in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(skip_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto", skipto_index, "udp", 
					FROM, lan_subnet, port1, TO, addrstr, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
					FROM, lan_subnet, TO, addrstr, port1, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(dnat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
					FROM, lan_subnet, port1, TO, addrstr, "in","via",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
					FROM, lan_subnet, TO, ip, port1, "out","xmit",lan_intf,NULL_STR);
				sprintf(ipfw_index,"%d", NEXT_IPFW_INDEX(snat_rule_no));	
				RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DIVERT, "natd", "udp",
					FROM, ip, port1, TO, lan_subnet, "out","xmit",lan_intf,NULL_STR);
				#endif
			}
		}
	}
	*ipfw_no = skip_rule_no;
	return;
}
int get_hash_index()
{
	int temp_index;
	int i = 2;
	while(i < 1000)		//跑coverity会报数组越界的问题，根据分析i不会跑到1000 所以无需修改
	{
		if(0 == index_hash[i])
			break;
		i++;
	}
	index_hash[i] = 1;
	temp_index = IPFW_INDEX_NAT_DOWNLINK_BASE + (i + 1) * 2;
	return temp_index;	
}

void set_hash_index(int index)
{
	index_hash[(index % IPFW_INDEX_NAT_DOWNLINK_BASE) / 2 - 1] = 0; 
}
extern int rtl_port_forwarding_enabled;
int add_port_forward(char* protocols,unsigned int ipAddr,unsigned int Private_Port,unsigned int Public_Port,char*wan_ifname,char* tcp_index,char* udp_index)
{
	char port[20];
	char port1[20];
	unsigned short protocol;
	char ipfw_index[10];
	struct in_addr ipaddr;
	int result;
	sprintf(port, "%u-%u",Public_Port,Public_Port);
	sprintf(port1,"%s",port);
	if(!strncmp(protocols,"tcp/udp",strlen("tcp/udp")))
		protocol = PROTO_BOTH;
	else if(!strncmp(protocols,"tcp",strlen("tcp")))
		protocol = PROTO_TCP;
	else
		protocol = PROTO_UDP;
	ipaddr.s_addr = ipAddr;
	result = rtl_addRtlPortFwdEntry(ipAddr,Public_Port,Public_Port,Private_Port,Private_Port,protocol);
	if(SUCCESS != result) /*lq 修改，添加重复规则判断*/
	{
		return FAILED;
	}
	
	if(PROTO_BOTH == protocol || PROTO_TCP == protocol)
	{
		sprintf(ipfw_index,"%d",get_hash_index());
		strcpy(tcp_index,ipfw_index);
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "tcp", FROM, ANY, TO, inet_ntoa(ipaddr), port,"in","via",wan_ifname,NULL_STR);
	}
	
	if(PROTO_BOTH == protocol || PROTO_UDP == protocol)
	{
		sprintf(ipfw_index,"%d",get_hash_index());
		strcpy(udp_index,ipfw_index);
    	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "udp", FROM, ANY, TO, inet_ntoa(ipaddr), port1,"in","via",wan_ifname,NULL_STR);
	}
	return SUCCESS;
}


int delete_port_forward(char* protocols,unsigned int ipAddr,unsigned int Private_Port,unsigned int Public_Port,char*wan_ifname,char* ipfw_index)
{
	char port[20];
	char port1[20];
	unsigned short protocol;
	struct in_addr ipaddr;
	int result;
	sprintf(port, "%u-%u",Public_Port,Public_Port);
	sprintf(port1,"%s",port);
	if(!strncmp(protocols,"tcp/udp",strlen("tcp/udp")))
		protocol = PROTO_BOTH;
	else if(!strncmp(protocols,"tcp",strlen("tcp")))
		protocol = PROTO_TCP;
	else
		protocol = PROTO_UDP;
	ipaddr.s_addr = ipAddr;
	result = rtl_delRtlPortFwdEntry(ipAddr, Public_Port, Public_Port,protocol);
	if(SUCCESS != result) /*lq 修改，添加删除是否成功判断*/
	{
		return FAILED;
	}
	RunSystemCmd(NULL_FILE, IPFW, DEL, ipfw_index, NULL_STR);
	set_hash_index(atoi(ipfw_index));
	return SUCCESS;
}
static void ipnat_loadrule(void)
{
	int i;
	char *value;
	char lan_ifname[32];
	char wan_ifname[32];
	char ipfw_index[10];
	char tcp_index[10];
	char udp_index[10];
	
	netconf_nat_t nat;
	if(ipnat_lanifname(0, lan_ifname) == 0)
		return ;
	if (ipnat_wanifname(wan_ifname) == 0)
		return;
/**************************DMZ BEGIN***************************/
	value = nvram_get(ADVANCE_DMZ_IPADDR_EN);
	if(value && atoi(value))
	{
		value = nvram_get(ADVANCE_DMZ_IPADDR);
		if (value)
		{
        	sprintf(ipfw_index,"%d",get_hash_index());
			RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "ip", FROM, ANY, TO, value, "in","via",wan_ifname,NULL_STR);
            rtl_parseNaptOption("target_address", value);
		}
	}
	else
		   	rtl_parseNaptOption("target_address", "0.0.0.0");

/**************************DMZ END******************************/

/**************************PORT FOWARD BEGIN********************/
	rtl_flushPortFwdEntry();
	rtl_port_forwarding_enabled = 1;
	for (i = 0; i < VTS_MAX_NUM_1; i++) 
	{
		memset(&nat, 0, sizeof(nat));
		if (get_forward_port(i, &nat) && !(nat.match.flags & NETCONF_DISABLED)) 
		{
			add_port_forward(nat.match.ipproto_str,nat.ipaddr.s_addr,nat.ports[0],nat.match.dst.ports[0],wan_ifname,tcp_index,udp_index);
		}
	}
/**************************PORT FOWARD END**********************/
	return;
}
#ifdef __CONFIG_TCP_AUTO_MTU__
extern int ifconfig_mtu(char *ifname,int mtu);
extern void ip_input_set_mss(unsigned int mss);
extern unsigned int ip_input_get_mss(); 
extern void ip_input_set_mss_enable(unsigned int enable);
extern int ip_input_change_mtu(int unit, int mtu);

#define PPPOE_DEFAULT_MTU 	(1480)
#define DHCP_DEFAULT_MTU	(1500)
#define STATIC_DEFAULT_MTU 	(1500)

#define TCP_AUTO_ON	 		1
#define TCP_AUTO_OFF 		0

static unsigned int tcp_auto_mtu_enable = TCP_AUTO_OFF;

void ip_input_update_mss(unsigned int mss)
{
	int i;
	char wan_type[32];
	char wan_ifname[32];
	char lan_ifname[32];
	int mtu;
	struct in_addr lanip;
	struct in_addr lanmask;
	char ifname_mtu[64] = {0};
	char wifi_mode[32] = {0};

	if(TCP_AUTO_OFF == tcp_auto_mtu_enable)
	{
		return;
	}
	
	get_wl0_mode(wifi_mode);
	
	if(0 != strcmp(wifi_mode,"ap"))
		return;

	strcpy(wan_type,nvram_safe_get("wan0_proto"));

	if(0 == strcmp(wan_type,"pppoe"))
	{
		strcpy(wan_ifname,"ppp0");
	}
	else
	{
		return;
	}

	for (i = 0; i < 1; i++) {
		/*
		 * Make sure this lan
		 * interface is on.
		 */
		if (ipnat_lanifname(i, lan_ifname) == 0)
			continue;
		if (iflib_getifaddr(lan_ifname, &lanip, &lanmask) != 0)
			continue;
		/* Set default TCP/UDP rules */
		if (iflib_ioctl(wan_ifname, SIOCGIFMTU, (void *)&mtu) != 0)
			mtu = 1500;
		
		if((mtu - 40) <= mss)
		{
			ip_input_set_mss(mtu - 40);
			break;
		}
		else
		{		
			mtu = mss + 40;
			//hqw add for mtu log
			sprintf(ifname_mtu,"%s mtu %d",wan_ifname,mtu);
			syslog(LOG_INFO,ifname_mtu);
			//end	
			
			if(0 == strncmp(wan_ifname,"ppp",strlen("ppp")))
			{		
				ip_input_change_mtu(0, mtu);
			}
			else
			{
				ifconfig_mtu(wan_ifname,mtu);
				#ifdef CONFIG_RTL_HARDWARE_NAT
				#ifndef HAVE_NOETH
				extern int rtl_setWanNetifMtu(int mtu);
				rtl_setWanNetifMtu(mtu);
				#endif
				#endif

			}
			
		}
	}
}
#endif
/*****************************************************************************
 函 数 名  : get_lan_addrstr
 功能描述  : 获取LAN口的IP地址
 输入参数  : int unit       
             char* addrstr  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月9日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void get_lan_addrstr(int unit,char* addrstr)
{
	int i = 0;
	int mask_count = 0;
	char tmp[64] = {0};
	char value[64] = {0};
	struct in_addr lan_addr, lan_mask;
	if(!unit)
	{
		inet_aton(nvram_get("lan_ipaddr"),&lan_addr);
		inet_aton(nvram_get("lan_netmask"),&lan_mask);
	}else
	{
		snprintf(tmp, sizeof(tmp), "lan%d_ipaddr", unit);
		inet_aton(nvram_get(tmp),&lan_addr);
		snprintf(tmp, sizeof(tmp), "lan%d_netmask", unit);
		inet_aton(nvram_get(tmp),&lan_mask);
	}

	lan_addr.s_addr = lan_addr.s_addr&lan_mask.s_addr;
	strcpy(addrstr, inet_ntoa(lan_addr));
	for(i=0; i<32; i++)
	{
		if(((1<<i)&lan_mask.s_addr)==0){
			mask_count++;
		}
	}
	mask_count = 32-mask_count;
	sprintf(value, "%s/%d", addrstr, mask_count);
	strcpy(addrstr,value);
}
void
ipnat_napt_init(char *wan_ifname)
{
	char ipfw_index[10];	
	char ipfw_index1[10];
	char buf[32];
	char guest_buf[32];
	char *guest_lan_ipaddr = NULL;
	char lan_ifname[16] = "eth0";	//根据coverity结果进行了修改 原来为:char lan_ifname[12] = "eth0"  2017/1/10 F9项目修改
	int mtu;
	if (iflib_ioctl(wan_ifname, SIOCGIFMTU, (void *)&mtu) != 0)
			mtu = 1500;
#ifdef __CONFIG_TCP_AUTO_MTU__	
		char wifi_mode[32] = {0};
		char wan_type[32];
		int ip_input_mss = 0;
		
		strcpy(wan_type,nvram_safe_get("wan0_proto"));

		ip_input_set_mss(1460);
		
		if(0 == strcmp(wan_type,"pppoe"))
		{
			tcp_auto_mtu_enable = (PPPOE_DEFAULT_MTU == atoi(nvram_safe_get("wan0_pppoe_mtu")))? TCP_AUTO_ON: TCP_AUTO_OFF;			
			ip_input_set_mss_enable(tcp_auto_mtu_enable);
		}
		else
		{
			tcp_auto_mtu_enable = TCP_AUTO_OFF;
			ip_input_set_mss_enable(TCP_AUTO_OFF);
		}

		if(tcp_auto_mtu_enable)
		{
			get_wl0_mode(wifi_mode);
			
			if(0 == strcmp(wifi_mode,"ap"))
			{
				ip_input_mss = ip_input_get_mss();

				if(ip_input_mss < (mtu - 40))
				{
					mtu = ip_input_mss + 40;			
					if(0 == strncmp(wan_ifname,"ppp",strlen("ppp")))
					{		
						ip_input_change_mtu(0, mtu);
					}
					else
					{
						ifconfig_mtu(wan_ifname,mtu);
						#ifdef CONFIG_RTL_HARDWARE_NAT
						#ifndef HAVE_NOETH
						extern int rtl_setWanNetifMtu(int mtu);
						rtl_setWanNetifMtu(mtu);
						#endif
						#endif
					}
				}
			}
		}
#endif

	get_lan_addrstr(0,buf);
	get_lan_addrstr(1,guest_buf);
#ifdef __CONFIG_GUEST__
	guest_lan_ipaddr = nvram_get("lan1_ipaddr");
#endif
	rtl_setAliasAddrByInfName(wan_ifname);
	RunSystemCmd(0, "ipfw", "-q","-f","flush", "");
	
	sprintf(ipfw_index,"%d",IPFW_INDEX_ALLOW_ALL);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", IP, FROM, ANY, TO, ANY, NULL_STR);

	sprintf(ipfw_index,"%d",IPFW_INDEX_NAT_UPLINK);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, buf, TO, ANY, TX,VIA,wan_ifname,NULL_STR);
	//guest network ip set	
#ifdef __CONFIG_GUEST__	
	sprintf(ipfw_index,"%d",IPFW_INDEX_NAT_UPLINK+IPFW_INDEX_STEP);	
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, guest_buf, TO, ANY, TX,VIA,wan_ifname,NULL_STR);
#endif
	sprintf(ipfw_index,"%d",IPFW_INDEX_NAT_DOWNLINK_CHECK);
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1, IP, FROM, ANY, TO, buf, RX,VIA,wan_ifname,NULL_STR);
	//add by lq guest network ip set
#ifdef __CONFIG_GUEST__	
	sprintf(ipfw_index,"%d",IPFW_INDEX_NAT_DOWNLINK_CHECK + IPFW_INDEX_STEP);
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK + IPFW_INDEX_STEP); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1, IP, FROM, ANY, TO, guest_buf, RX,VIA,wan_ifname,NULL_STR);
	//end
#endif
	sprintf(ipfw_index,"%d",IPFW_INDEX_DENY_DOWNLINK_ALL);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY,IP,FROM,ANY,TO,ANY,RX,VIA,wan_ifname,NULL_STR);

	ipfw_no=IPFW_INDEX_START;     
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW,UDP,FROM,ANY,"67",TO,ANY,"68",NULL_STR);	

	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW,UDP,FROM,ANY,"68",TO,ANY,"67",NULL_STR);
/*add by lq about nat loopback */
	set_nat_loopback_ipfw_rules(buf, wan_ifname, lan_ifname, &ipfw_no);
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW,IP,FROM,ANY,TO,"me",RX,VIA,lan_ifname,NULL_STR);
//end
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	if (strncmp(wan_ifname, "eth1", strlen("eth1")))
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY,UDP,FROM,"any",TO,"239.255.255.250", "1900","in","recv","eth1",NULL_STR);

	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY,UDP,FROM,"any",TO,"239.255.255.250", "1900","in","recv",wan_ifname,NULL_STR);
#ifdef __CONFIG_GUEST__	
	if(guest_lan_ipaddr)
	{
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "deny", TCP, FROM, guest_buf, TO, guest_lan_ipaddr,"9000", NULL_STR);
	}
#endif
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "check-state", NULL_STR);

	ipfw_no=IPFW_INDEX_STATE_NOT_MATCH;
	if (0 == strcmp(nvram_safe_get("pingwan_enable"), "0"))
	{
		printf("ganda_debug--[%s]%d\n",__FUNCTION__,__LINE__);
		sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, DENY, "icmp", FROM, ANY, TO, "me", "in","via",wan_ifname,"icmptypes","8",NULL_STR);
	}

	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow", "icmp", FROM, ANY, TO, "me", "in","via",wan_ifname,NULL_STR);	
	

	if(nvram_match(ADVANCE_RM_WEB_EN, "1"))
      {
    
	    	char port[8];
	    	char web_ip[32];
			struct in_addr wan_ipaddr;
			char wan_ip_string[32];
	    	strcpy(port, nvram_safe_get(ADVANCE_RM_WEB_PORT));
	    	strcpy(web_ip, nvram_safe_get(ADVANCE_RM_WEB_IP));
	    	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
		
		
		if (iflib_getifaddr(wan_ifname, &wan_ipaddr, 0)  == 0)
		{
			
			snprintf (wan_ip_string, sizeof (wan_ip_string), inet_ntoa(wan_ipaddr));
			if (!strcmp (web_ip, "0.0.0.0"))
		    	{
		    		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, TCP, FROM,"any",TO,wan_ip_string, port,"in","recv",wan_ifname,NULL_STR);	
		    	}
		    	else 
		    	{
		    		RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, TCP, FROM, web_ip,TO,wan_ip_string, port,"in","recv",wan_ifname,NULL_STR);	
		    	}
				
		}
		

	
	    	
      }
		  
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "allow",IP,FROM,ANY,TO,ANY,RX,"recv",wan_ifname, "fragment", NULL_STR);

	ipfw_no=IPFW_INDEX_FILTER;	
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	RunSystemCmd(NULL_FILE, "ipfw", "add", ipfw_index, "allow", "ip", "from", "any", "to", "224.0.0.0/4",NULL_STR);

	ipfw_no=IPFW_INDEX_SKIP_TO_NAT_UPLINK_BASE;
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	sprintf(ipfw_index1,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,TCP,FROM,ANY,TO,ANY,TX,VIA,wan_ifname,TCPOPTIONS, MSS,NULL_STR);	

	sprintf(ipfw_index,"%d",IPFW_INDEX_PPTP_ALLOW_RECV_FROM_WAN);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, "47", FROM, ANY, "to", ANY, "in", "recv", wan_ifname, NULL_STR);

	sprintf(ipfw_index,"%s",ipfw_index1); 
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,IP,FROM,ME,TO,ANY,TX,VIA,wan_ifname,"keep-state",NULL_STR);	

	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no)); 	
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,IP,FROM,ANY,TO,ANY,TX,VIA,wan_ifname, NULL_STR);	

	ipfw_no=IPFW_INDEX_NAT_DOWNLINK_BASE;
	sprintf(ipfw_index,"%d",NEXT_IPFW_INDEX(ipfw_no));
	sprintf(ipfw_index1,"%d",IPFW_INDEX_NAT_UPLINK); 
	sprintf(ipfw_index1,"%d",NEXT_IPFW_INDEX(ipfw_no)); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "skipto",ipfw_index1,TCP,FROM,ANY,TO,ANY,RX,VIA,wan_ifname,TCPOPTIONS, MSS,NULL_STR);	

	sprintf(ipfw_index,"%s",ipfw_index1); 
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, "divert", "natd",IP, FROM, ANY, TO, ANY, RX,VIA,wan_ifname,NULL_STR);
	sprintf(ipfw_index,"%d", IPFW_INDEX_IPSEC_ALLOW_RECV_FROM_WAN);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, UDP, FROM, ANY, "to", ANY, "500", "in", "recv", wan_ifname, NULL_STR);

	sprintf(ipfw_index,"%d", IPFW_INDEX_IPSEC_ALLOW_RECV_FROM_WAN+2);
	RunSystemCmd(NULL_FILE, IPFW, ADD, ipfw_index, ALLOW, "50", FROM, ANY, "to", ANY, "in", "recv", wan_ifname, NULL_STR);

}



extern void set_fast_skb_fw(int value);
int ipnat_fastnat_activate(int activate)
{
	set_fast_skb_fw(activate);
	return activate;
}

void
ipnat_init(void)
{
	int nat_en;
	int i = 0;
	char lan_ifname[32];
	char wan_ifname[32];
	if (nvram_match(SYSCONFIG_WORKMODE, "route") 
		||nvram_match(SYSCONFIG_WORKMODE, "wisp"))
		nat_en = 1;
	else
		nat_en = 0;
	
	memset(index_hash,0,1000);
	printf("func:%s i = %d\n",__func__,i);
	ipnat_lanifname(i, lan_ifname);
	nat_flag_set(lan_ifname, IFFLT_NAT, nat_en);
	if (nat_en == 0)
		return;
	rtl_initNapt();
	ipnat_wanifname(wan_ifname);
	if (iflib_getifaddr(wan_ifname, 0, 0) != 0)
		return;
	
	ipnat_napt_init(wan_ifname);
	ipnat_loadrule();

	if(!strcmp(wan_ifname,"ppp0"))
	{
		set_fast_pppoe_fw(1);
	}

	ipnat_fastnat_activate(1);
	#ifdef __CONFIG_TENDA_APP__
	/*重启nat后通知ucloud重启和server的connect*/
	restart_ucloud_connect_svr();
	#endif
	

}
extern int rtl_initNapt(void);

static void ipnat_flush(void)
{
	char guest_buf[32] = {0};
	char *guest_lan_ipaddr = NULL;
	
	RunSystemCmd(0, "ipfw", "-q","-f","flush", "");
	RunSystemCmd(NULL_FILE, "ipfw", "add", "65534", "allow", "ip", "from", "any", "to", "any", NULL_STR);
	rtl_initNapt();
	#if 1
	char temp[32] = {0};
    sprintf(temp, "%u:%u", IPFW_INDEX_ALG, IPFW_SECTOR_SIZE-1);
  	rtl_parseNaptOption("punch_fw", temp);
	#endif

	RunSystemCmd(NULL_FILE, "ipfw", "add", "10", "allow", "ip", "from", "any", "to", "any", NULL_STR);

		
#ifdef __CONFIG_GUEST__
	if(nvram_match(SYSCONFIG_WORKMODE,"route"))
	{
		get_lan_addrstr(1,guest_buf);
		guest_lan_ipaddr = nvram_get("lan1_ipaddr");
	
		if(guest_lan_ipaddr)
		{
			RunSystemCmd(NULL_FILE, IPFW, ADD, "8", "deny", TCP, FROM, guest_buf, TO, guest_lan_ipaddr,"9000", NULL_STR);
		}
	}
#endif
}
void
ipnat_deinit(void)
{
#if 0
//去掉关闭加速模块的设置，解决wlan lan 到有线lan速率低的问题
	ipnat_fastnat_activate(0);
#endif
	set_fast_pppoe_fw(0);

	ipnat_flush();
}

