
//roy +++ 2010/09/08 common function
#include <stdio.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <ecos_oslib.h>
#include <net/if_var.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/param.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/sockio.h>

#include <router_net.h>
#ifdef __CONFIG_TENDA_HTTPD_V3__
#include <shutils.h>
#endif
#include  "route_cfg.h"
#include  "flash_cgi.h"


extern int wan_link_status(void);
extern int str2arglist(char *buf, int *list, char c, int max);
extern char *strsep(char **stringp, const char *delim);
extern int    arpioctl(int req, caddr_t data, struct proc *p);

extern int dhcpc_release1(void);
extern int dhcpc_renew1(void);
extern void sys_pppoe_disconn(void);
extern void sys_pppoe_conn(void);


char *racat(char *s, int i)
{
	static char str[128];
	snprintf(str, 128, "%s%d", s, i);
	return str;
}

static char *wan_mode_str[]={
	"disabled",
	"static",
	"dhcp",
	"pppoe",
	"pptp",
	"l2tp",
	"8021x",
	"",
	"",
	"pppoe2",
	"pptp2",
	NULL
};

int get_wan0_dns(int index, char *dnsBuf)
{
	char dns[40],dns1[2][20],*pdns;
	int nu;
	
	if(index !=1 && index != 2)
		return 0;
	memset(dns1,0,sizeof(dns1));

	_SAFE_GET_VALUE(_WAN0_DNS,pdns);
	
	strcpy(dns,pdns);
	nu = sscanf(dns,"%s %s",dns1[0],dns1[1]);
	if(nu>0){
		strcpy(dnsBuf,dns1[index-1]);
	}else{
			return 0;
	}

	return 1;
}

int get_pptp_dns(int index, char *dnsBuf)
{
	char dns[40],dns1[2][20],*pdns;
	int nu;
	
	if(index !=1 && index != 2)
		return 0;
	memset(dns1,0,sizeof(dns1));

	_SAFE_GET_VALUE(_WAN0_PPTP_DNS,pdns);
	
	strcpy(dns,pdns);
	nu = sscanf(dns,"%s %s",dns1[0],dns1[1]);
	if(nu>0){
		strcpy(dnsBuf,dns1[index-1]);
	}else{
			return 0;
	}

	return 1;
}

int get_l2tp_dns(int index, char *dnsBuf)
{
	char dns[40],dns1[2][20],*pdns;
	int nu;
	
	if(index !=1 && index != 2)
		return 0;
	memset(dns1,0,sizeof(dns1));

	_SAFE_GET_VALUE(_WAN0_l2TP_DNS,pdns);
	
	strcpy(dns,pdns);
	nu = sscanf(dns,"%s %s",dns1[0],dns1[1]);
	if(nu>0){
		strcpy(dnsBuf,dns1[index-1]);
	}else{
			return 0;
	}

	return 1;
}

int get_wan_type(void)
{
	const char *wan_str;
	char *p;
	int i;
	
	_SAFE_GET_VALUE(_WAN0_PROTO,wan_str);
	for(i = 0;(p = wan_mode_str[i]) != NULL;i++)
	{
		if(strcmp(p,wan_str) == 0){
			break;
		}	
	}
	return (p?i:0);
}

//------------weige add for index wan type no change------------------
int get_wan_type_index(void)
{
	const char *wan_str;
	char *p;
	int i;
	
	_SAFE_GET_VALUE(_WAN0_PROTO_INDEX,wan_str);
	for(i = 0;(p = wan_mode_str[i]) != NULL;i++)
	{
		if(strcmp(p,wan_str) == 0){
			break;
		}	
	}
	return (p?i:0);
}

int set_wan_str_index(int type)
{
	int i;
	char *p;
	
	for(i = 0;(p = wan_mode_str[i]) != NULL;i++){
		if(type == i){
			 _SET_VALUE(_WAN0_PROTO_INDEX, p);
			 break;
		}
	}

	return (p?1:0);
}

//-----------------end-------------------
int set_wan_str(int type)
{
	int i;
	char *p;
	
	for(i = 0;(p = wan_mode_str[i]) != NULL;i++){
		if(type == i){
			 _SET_VALUE(_WAN0_PROTO, p);
			 break;
		}
	}

	return (p?1:0);
}

void set_dhcpd_en(int enable)
{
	if(!enable){
		_SET_VALUE(_LAN0_DHCPD_EN, "static");
	}else{
		_SET_VALUE(_LAN0_DHCPD_EN, "dhcp");
	}

}

int get_dhcpd_en(){
	const char *proto;

	_SAFE_GET_VALUE(_LAN0_DHCPD_EN,proto);

	if(strcmp(proto,"dhcp") == 0){
		return 1;
	}else{
		return 0;
	}
}

int get_wan_connstatus(void)
{
	char *val;
	if(wan_link_status()){
		_SAFE_GET_VALUE(_WAN0_CONNECT,val);
		
		if (strcmp(val, "Connected") == 0)
			return 2;
		else if (strcmp(val, "Connecting") == 0)
			return 1;
		else
			return 0;
	}else{
		return 0;
	}
}

int get_wan_linkstatus(void)
{
	char *val;
	if(wan_link_status()){
		_SAFE_GET_VALUE(_WAN0_CONNECT,val);
		
		if (strcmp(val, "Connected") == 0)
			return 1;
		else if (strcmp(val, "Connecting") == 0)
			return 2;
		else
			return 3;
	}else{
		return 0;
	}
}

#if 0
char change_ip[] = "255.255.255.255";
int CGI_same_net_with_lan(unsigned int wan_ip,unsigned int wan_mask)
{
	unsigned long int lan_ip,mask,ip_tag,ip_class = 0,lan_mask = 0,ip_change_num = 0;
	char lan_each_mask[4][4] = {0},wan_each_mask[4][4] = {0},lan_each_ip[4][4] = {0},wan_each_ip[4][4] = {0};
	struct in_addr addr_mask = {0};

	char *ipAddr,*subnetMask;

	_GET_VALUE( _LAN0_IP, ipAddr);
	_GET_VALUE( _LAN0_NETMASK, subnetMask);

	if(NULL == ipAddr || NULL == subnetMask)
	{
		return 0;
	}
	
	addr_mask.s_addr = wan_mask;
	lan_mask = inet_addr(subnetMask);
	sscanf(inet_ntoa(addr_mask),"%[^.].%[^.].%[^.].%s",wan_each_mask[0],wan_each_mask[1],wan_each_mask[2],wan_each_mask[3]);
	sscanf(subnetMask,"%[^.].%[^.].%[^.].%s",lan_each_mask[0],lan_each_mask[1],lan_each_mask[2],lan_each_mask[3]);
	sscanf(ipAddr,"%[^.].%[^.].%[^.].%s",lan_each_ip[0],lan_each_ip[1],lan_each_ip[2],lan_each_ip[3]);
	sscanf(inet_ntoa(wan_mask),"%[^.].%[^.].%[^.].%s",wan_each_ip[0],wan_each_ip[1],wan_each_ip[2],wan_each_ip[3]);

	if(atoi(wan_each_mask[1]) == 0)
		ip_class = 1;
	else if(atoi(wan_each_mask[2]) == 0)
		ip_class = 2;
	else if(atoi(wan_each_mask[3]) == 0)
		ip_class = 3;
	else
		return 0;

	lan_ip = inet_addr(ipAddr);

	if(lan_mask >= wan_mask)
		mask = wan_mask;
	else
		mask = lan_mask;
	
	switch(ip_class)
	{
		case 1://A类地址
			//如果lan口不是A类则不冲突
			if(atoi(lan_each_mask[1]) != 0)
				return 0;
			
			ip_tag = atoi(wan_each_ip[0]);
			
			if(atoi(wan_each_mask[0]) < atoi(lan_each_mask[0]))
			{	
				ip_change_num = atoi(lan_each_mask[0]) - atoi(wan_each_mask[0]) + 1;
				if((ip_tag & ip_change_num) != 0)
				{
					ip_change_num = ip_change_num * 2;
				}
			}
			else if(atoi(wan_each_mask[0]) == atoi(lan_each_mask[0]))
			{	
				ip_change_num = atoi(lan_each_ip[0]) + 1;
			}
			else
			{	
				ip_change_num = atoi(wan_each_mask[0]) - atoi(lan_each_mask[0]) + 1;
				if((ip_tag & ip_change_num) != 0)
				{
					ip_change_num = ip_change_num * 2;
				}
			}

			if(0 != ip_change_num)
				sprintf(change_ip,"%d.%s.%s.%s",ip_change_num % 256,lan_each_ip[1],lan_each_ip[2],lan_each_ip[3]);
			
			break;
		case 2://B类地址
			//如果lan口不是B类则不冲突
			if(atoi(lan_each_mask[2]) != 0)
				return 0;
			
			ip_tag = atoi(wan_each_ip[1]);
			
			if(atoi(wan_each_mask[1]) < atoi(lan_each_mask[1]))
			{	
				ip_change_num = atoi(lan_each_mask[1]) - atoi(wan_each_mask[1]) + 1;
				if((ip_tag & ip_change_num) != 0)
				{
					ip_change_num = ip_change_num * 2;
				}
			}
			else if(atoi(wan_each_mask[1]) == atoi(lan_each_mask[1]))
			{	
				ip_change_num = atoi(lan_each_ip[1]) + 1;
			}
			else
			{	
				ip_change_num = atoi(wan_each_mask[1]) - atoi(lan_each_mask[1]) + 1;
				if((ip_tag & ip_change_num) != 0)
				{
					ip_change_num = ip_change_num * 2;
				}
			}
	
			if(0 != ip_change_num)
				sprintf(change_ip,"%s.%d.%s.%s",lan_each_ip[0],ip_change_num % 256,lan_each_ip[2],lan_each_ip[3]);
			
			break;
		case 3://C类地址
			//如果lan口不是C类则不冲突
			if(atoi(lan_each_mask[3]) != 0)
				return 0;
			
			ip_tag = atoi(wan_each_ip[2]);
			
			if(atoi(wan_each_mask[2]) < atoi(lan_each_mask[2]))
			{	
				ip_change_num = atoi(lan_each_mask[2]) - atoi(wan_each_mask[2]) + 1;
				if((ip_tag & ip_change_num) != 0)
				{
					ip_change_num = ip_change_num * 2;
				}
			}
			else if(atoi(wan_each_mask[2]) == atoi(lan_each_mask[2]))
			{	
				ip_change_num = atoi(lan_each_ip[2]) + 1;
			}
			else
			{	
				ip_change_num = atoi(wan_each_mask[2]) - atoi(lan_each_mask[2]) + 1;
				if((ip_tag & ip_change_num) != 0)
				{
					ip_change_num = ip_change_num * 2;
				}
			}
			
			if(0 != ip_change_num)
				sprintf(change_ip,"%s.%s.%d.%s",lan_each_ip[0],lan_each_ip[1],ip_change_num % 256,lan_each_ip[3]);
			
			break;
		default:
			return 0;
			break;
	}

	if((wan_ip&mask) == (lan_ip&mask))
		return 1;
	else
		return 0;
}
#else
char change_ip[] = "255.255.255.255";
int CGI_same_net_with_lan(unsigned int wan_ip,unsigned int wan_mask)
{
	char * ipa, *mask;
	unsigned int lan_ip,lan_mask;
	unsigned int tMask;
	struct in_addr  modify_lan_ip;
	
	_GET_VALUE( _LAN0_IP, ipa);
	_GET_VALUE( _LAN0_NETMASK, mask);

	lan_ip = inet_addr(ipa);
	lan_mask = inet_addr(mask);
	
/*	printf("---->func=%s,wan_ip=%s,", __FUNCTION__, inet_ntoa(wan_ip));
	printf("wan_mask=%s,", inet_ntoa(wan_mask));
	printf("lan_ip=%s,", inet_ntoa(lan_ip));
	printf("lan_mask=%s\n", inet_ntoa(lan_mask));
	printf("wan_mask=%X,lan_mask=%X\n", wan_mask, lan_mask);
	printf("ntohl(wan_mask)=%X,ntohl(lan_mask)=%X\n", ntohl(wan_mask), ntohl(lan_mask));
*/
	if(
		(wan_ip & wan_mask) == (lan_ip & lan_mask) ||
		(wan_ip & lan_mask) == (lan_ip & lan_mask) ||
		(wan_ip & wan_mask) == (lan_ip & wan_mask)
		)
	{
		tMask = (ntohl(lan_mask) >= ntohl(wan_mask))?wan_mask:lan_mask;
		lan_ip = ntohl(wan_ip & tMask) + (~(ntohl(tMask)) + 1)/*one segment*/ + 1;

		if( ((lan_ip&0XFF000000) == (127<<24)) ||
		((lan_ip&0XFF000000) >= (224<<24)) )
		{
			lan_ip = ntohl(wan_ip & tMask) - (~(ntohl(tMask)) + 1)/*one segment*/ + 1;
		}

		modify_lan_ip.s_addr= htonl(lan_ip);
		printf("----->modify_lan_ip=%s\n", inet_ntoa(modify_lan_ip));	
		sprintf(change_ip, "%s", inet_ntoa(modify_lan_ip));

		return 1;
	}
	else
	{
		return 0;
	}
}
#endif

int CGI_same_net(unsigned int one_ip,unsigned int other_ip,unsigned int net_mask)
{
	if((one_ip&net_mask) == (other_ip&net_mask))
		return 1;
	else
		return 0;
}

int parse_webstr2portforward(char *oldstr, char *newstr)
{
	//0;ee;1-2;tcp;10.10.10.5;3;0;;
	//-->1-2>10.10.10.5:1-2,tcp,off
	
	char *arglists[9];
	char *enable;
	if (! (str2arglist(oldstr, arglists, ';', 9) == 9))
		return 0;

	if(atoi(arglists[0])==0){
		enable = "off";
	}else{
		enable = "on";
	}

	sprintf(newstr,"%s>%s:%s,%s,%s",
			arglists[2],arglists[4],arglists[2],arglists[3],enable);

	//diag_printf("===%s===%s\n",__FUNCTION__,newstr);

	return 1;
	
}

int parse_portforward2webstr(char *oldstr,char *newstr)
{
	//forward_port0=50-60>192.168.1.23:50-60,tcp,on
	//-->0;portforward;50-60;tcp;192.168.1.23
	
	char value[128];
	char *wan_port0, *lan_ipaddr, *lan_port0, *proto;
	char *enable, *desc;
	int ienable;

	/* Parse wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1[:,]proto[:,]enable[:,]desc */
	
	strcpy(value, oldstr);

	/* Check for LAN IP address specification */
	lan_ipaddr = value;
	wan_port0 = strsep(&lan_ipaddr, ">");
	if (!lan_ipaddr)
		return 0;
	//wan_port0="50-60"

	/* Check for LAN destination port specification */
	lan_port0 = lan_ipaddr;
	lan_ipaddr = strsep(&lan_port0, ":");
	if (!lan_port0)
		return 0;

	//lan_ipaddr="192.168.1.23"

	/* Check for protocol specification */
	proto = lan_port0;
	lan_port0 = strsep(&proto, ":,");
	if (!proto)
		return 0;

	/* Check for enable specification */
	enable = proto;
	proto = strsep(&enable, ":,");
	if (!enable)
		return 0;
	//proto="tcp"

	/* Check for description specification (optional) */
	desc = enable;
	enable = strsep(&desc, ":,");

	//enable="on"
	ienable = (strcmp(enable,"on") == 0);
	//0;portforward;50-60;tcp;192.168.1.23
	sprintf(newstr,"%d;%s;%s;%s;%s",
			ienable,"portforward",wan_port0,proto,lan_ipaddr);

	//diag_printf("===%s===%s\n",__FUNCTION__,newstr);

	return 1;
	
}

int get_filter_mode(char *filter)
{
	const char *mode;

	_SAFE_GET_VALUE(filter,mode);
	if(strcmp(mode,"deny") == 0){
		return 1;
	}
	else if(strcmp(mode,"pass") == 0){
		return 2;
	}
	else{
		//disable
		return 0;
	}	
}

void set_wan_mac(const  char *ipaddr)
{
	char pcMac[20];
	char macmac[20];
	((struct in_addr*) pcMac)->s_addr=inet_addr(ipaddr);
		
	if(arpioctl(SIOCGARPRT, pcMac, NULL) == 0)
	{
		snprintf(macmac,sizeof(macmac),
				"%02X:%02X:%02X:%02X:%02X:%02X",
				pcMac[4]&0XFF,
				pcMac[5]&0XFF,
				pcMac[6]&0XFF,
				pcMac[7]&0XFF,
				pcMac[8]&0XFF,
				pcMac[9]&0XFF);
		_SET_VALUE(_WAN0_HWADDR,macmac);
	}
}

int CGI_do_wan_connect_tenda(int action) 
{ 

#ifdef __CONFIG_TENDA_HTTPD_V3__
	int demand = 0 , unit = 0,manage_flag = 0;
	char tmp[64], prefix[] = "wanXXXXXXXXXX_";
#endif
	int wan_status = get_wan_connstatus();
	int wan_type = get_wan_type();

	_SET_VALUE("err_check","0");/*add by ldm*/

	if(wan_type == 2)
	{
		if(action == 1){
			//dhcp release
			if(wan_status == 2){
				dhcpc_release1();
			}
		}else if(action == 2){
				dhcpc_renew1();
		}
	}else if(wan_type == 3){
		if(action == 3){
#ifdef __CONFIG_TENDA_HTTPD_V3__		
			if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
				unit = 0;
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			demand = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_demand", tmp)));
			manage_flag = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_manage_flag", tmp)));
			
			if(demand == 2 && manage_flag <= 0){
				nvram_set(strcat_r(prefix, "pppoe_manage_flag", tmp) , "1");
			}
#endif			
			//pppoe connect
			if(wan_status == 0){
				sys_pppoe_conn();
			}
		}else if(action == 4){
			//pppoe disc
			if(wan_status != 0){
				sys_pppoe_disconn();
			}
		}
	}

	return 0;
} 

void strtoupper(char *buf)
{
	int i,len;
	char bufTmp[128];
	char chValue,chTmp[4];
	memset(bufTmp,0,sizeof(bufTmp));

	len = strlen(buf);
	for (i = 0; i < len; i ++) {
		if (*(buf + i) != ' ') {
			chValue = toupper(*(buf + i));
			memset(chTmp,0,sizeof(chTmp));
			sprintf(chTmp,"%c",chValue);
			strcat(bufTmp,chTmp);
		}else{
			//keep space
			chValue = *(buf + i);
			memset(chTmp,0,sizeof(chTmp));
			sprintf(chTmp,"%c",chValue);
			strcat(bufTmp,chTmp);
		}
	}
	strcpy(buf,bufTmp);
}

//
