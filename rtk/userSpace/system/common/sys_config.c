
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <router_net.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#if __CONFIG_WAN_MODE_CHECK__
#include "wan_surf_check.h"
#endif
#include "sys_config.h"
 char *  racat(char *s, int i)
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
int get_wan_type(void)
{
	const char *wan_str;
	char *p;
	int i;
	
	wan_str = nvram_safe_get(WAN0_PROTO);
	for(i = 0;(p = wan_mode_str[i]) != NULL;i++)
	{
		if(strcmp(p,wan_str) == 0){
			break;
		}	
	}
	return (p?i:0);
}

void set_dhcpd_en(int enable)
{
	if(!enable){
		nvram_set(LAN_PROTO, "static");
		syslog(LOG_INFO,"dhcp server down!");
	}else{
		nvram_set(LAN_PROTO, "dhcp");
		syslog(LOG_INFO,"dhcp server up!");
	}

}

int get_wan_onln_connstatus(void)//gong 20130619 标记能否上网
{
#if __CONFIG_WAN_MODE_CHECK__
	return gpi_wan_surf_check_result();
#else
	return 0;
#endif
}

char change_ip[] = "255.255.255.255";
/*
	return 0:no conflict
	return 1:conflict
*/
unsigned int conflict_ip_modify(unsigned int my_ip,unsigned int my_mask,unsigned int other_ip,unsigned int other_mask)
{
	unsigned int tMask;
	if(
		(other_ip & other_mask) == (my_ip & my_mask) ||
		(other_ip & my_mask) == (my_ip & my_mask) ||
		(other_ip & other_mask) == (my_ip & other_mask)
		)
	{
		tMask = (ntohl(my_mask) >= ntohl(other_mask))?other_mask:my_mask;
		my_ip = ntohl(other_ip & tMask) + (~(ntohl(tMask)) + 1)/*one segment*/ + 1;

		if( ((my_ip&0XFF000000) == (127<<24)) ||
		((my_ip&0XFF000000) >= (224<<24)) )
		{
			my_ip = ntohl(other_ip & tMask) - (~(ntohl(tMask)) + 1)/*one segment*/ + 1;
		}

		return my_ip;
	}
	return 0;
}

int CGI_same_net_with_lan(unsigned int wan_ip,unsigned int wan_mask)
{
	char * ipa, *mask;
	unsigned int lan_ip,lan_mask;
	unsigned int tMask;
	struct in_addr  modify_lan_ip;
	
	ipa = nvram_safe_get(LAN_IPADDR);
	mask = nvram_safe_get(LAN_NETMASK);

	if(strcmp(ipa, "") == 0 || strcmp(mask, "") == 0)
		return 0;
	
	lan_ip = inet_addr(ipa);
	lan_mask = inet_addr(mask);
	lan_ip = conflict_ip_modify(lan_ip,lan_mask,wan_ip,wan_mask);
	if(lan_ip)
	{
		modify_lan_ip.s_addr= htonl(lan_ip);
		sprintf(change_ip, "%s", inet_ntoa(modify_lan_ip));

		return 1;
	}
	else
	{
		return 0;
	}
}

int same_net(unsigned int lan_ip,unsigned int lan_mask,
			 unsigned int lan_ip_start,unsigned int lan_ip_end,
			 char *ip_start,char *ip_end)
{
	unsigned int start_ip,end_ip,lan;
	struct in_addr  lan_start_ip,lan_end_ip;

	if(NULL == ip_start || NULL == ip_end)
		return 0;

	lan =  ntohl(lan_ip & lan_mask);
	
	start_ip =  ntohl(lan_ip_start - (lan_ip_start & lan_mask));
	end_ip =  ntohl(lan_ip_end - (lan_ip_end & lan_mask));
	
	lan_start_ip.s_addr= htonl(lan + start_ip);
	sprintf(ip_start, "%s", inet_ntoa(lan_start_ip));

	lan_end_ip.s_addr= htonl(lan + end_ip);
	sprintf(ip_end, "%s", inet_ntoa(lan_end_ip));
	
	return 1;
}

void lan_change_config_reboot(unsigned int wan_ip,unsigned int wan_mask)
{
	char ip[64] = {0},ip_start[64] = {0},ip_end[64] = {0},ip_mask[64] = {0},ip_dns[128] = {0},ip_each_dns[3][64] = {0};
	char lan_ip[64] = {0},lan_start_ip[64],lan_end_ip[64] = {0},lan_dns[128] = {0};
	int i = 0,dns_num = 0;
	
	sprintf(ip,"%s",nvram_safe_get(LAN_IPADDR));
	sprintf(ip_mask,"%s",nvram_safe_get(LAN_NETMASK));
	sprintf(ip_start,"%s",nvram_safe_get(LAN_DHCP_POOL_START));
	sprintf(ip_end,"%s",nvram_safe_get(LAN_DHCP_POOL_END));
	sprintf(ip_dns,"%s",nvram_safe_get(LAN_DNS));
	dns_num = sscanf(ip_dns,"%[^ ] %[^ ] %s",ip_each_dns[0],ip_each_dns[1],ip_each_dns[2]);

	strcpy(lan_ip,change_ip);
	
	for(i = 0;i < dns_num;i++)
	{
		if(0 != strcmp(ip_each_dns[i],""))
		{
			if((0 != i))
				strncat(lan_dns," ",strlen(" "));
			
			if(0 == strcmp(ip_each_dns[i],ip))
				strncat(lan_dns,lan_ip,strlen(lan_ip));
			else
				strncat(lan_dns,ip_each_dns[i],strlen(ip_each_dns[i]));
		}
	}
	
	same_net(inet_addr(lan_ip),inet_addr(ip_mask),inet_addr(ip_start),inet_addr(ip_end),lan_start_ip,lan_end_ip);
	
	nvram_set(LAN_IPADDR,lan_ip);
	nvram_set(LAN_DHCP_POOL_START,lan_start_ip);
	nvram_set(LAN_DHCP_POOL_END,lan_end_ip);
	nvram_set(LAN_DNS,lan_dns);
#ifdef __CONFIG_GUEST__	
	check_guest_net_with_other(wan_ip,wan_mask);
#endif
	modify_filter_virtual_server(lan_ip);
	
	nvram_commit();

	cyg_thread_delay(200);
	
	tapf_board_reboot();
}

/*bei++*/
int CGI_same_net(unsigned int one_ip,unsigned int other_ip,unsigned int net_mask)
{
	if((one_ip&net_mask) == (other_ip&net_mask))
		return 1;
	else
		return 0;
}

extern unsigned int   guest_ip_addr;
extern unsigned int   guest_ip_mask;

int check_guest_net_with_other(unsigned int wan_ip,unsigned int wan_mask)
{
	char *tmp = NULL;
	unsigned int modify_ip = 0;
	struct in_addr  modify_lan_ip;
	unsigned int modify_ip_by_wan = 0;
	unsigned int modify_ip_by_lan = 0;
	unsigned int lan_ip = 0,lan_mask = 0,guest_ip = 0,guest_mask = 0;
	char *ip_start = NULL,*ip_end = NULL;
	char lan_start_ip[64],lan_end_ip[64] = {0},lan_modify_ip[64] = {0};
	//lan ip and mask
	tmp = nvram_get(LAN_IPADDR);
	if(tmp == NULL)
		return;
	lan_ip = inet_addr(tmp);
	
	tmp = nvram_get(LAN_NETMASK);
	if(tmp == NULL)
		return;
	lan_mask = inet_addr(tmp);
	//guest ip and mask
	tmp = nvram_get(LAN1_IPADDR);
	if(tmp == NULL)
		return;
	guest_ip = inet_addr(tmp);
	
	tmp = nvram_get(LAN1_NETMASK);
	if(tmp == NULL)
		return;
	guest_mask = inet_addr(tmp);
	if(wan_ip != 0 && wan_mask != 0)
	{
		modify_ip_by_wan = conflict_ip_modify(guest_ip,guest_mask,wan_ip,wan_mask);
		if(modify_ip_by_wan)
		{
			modify_ip_by_lan = conflict_ip_modify(htonl(modify_ip_by_wan),guest_mask,lan_ip,lan_mask);
			if(modify_ip_by_lan)
			{
				modify_ip = modify_ip_by_lan;
			}
			else
				modify_ip = modify_ip_by_wan;
		}
		else
		{
			modify_ip_by_lan = conflict_ip_modify(guest_ip,guest_mask,lan_ip,lan_mask);
			if(modify_ip_by_lan)
			{
				modify_ip_by_wan = conflict_ip_modify(htonl(modify_ip_by_lan),guest_mask,wan_ip,wan_mask);
				if(modify_ip_by_wan)
					modify_ip = modify_ip_by_wan;
				else
					modify_ip = modify_ip_by_lan;
			}
			else
				return 0;
		}
	}
	else
	{
		modify_ip_by_lan = conflict_ip_modify(guest_ip,guest_mask,lan_ip,lan_mask);
		if(modify_ip_by_lan)
		{
			modify_ip = modify_ip_by_lan;
		}
		else
			return 0;
	}
	modify_lan_ip.s_addr= htonl(modify_ip);
	sprintf(lan_modify_ip, "%s", inet_ntoa(modify_lan_ip));
	printf("====lan_modify_ip:%s=====%s [%d]\n",lan_modify_ip, __FUNCTION__, __LINE__);
	ip_start = nvram_safe_get("dhcp1_start");
	ip_end = nvram_safe_get("dhcp1_end");
	same_net(inet_addr(lan_modify_ip),inet_addr(nvram_safe_get(LAN1_NETMASK)),inet_addr(ip_start),inet_addr(ip_end),lan_start_ip,lan_end_ip);
	nvram_set(LAN1_IPADDR,lan_modify_ip);
	nvram_set("dhcp1_start",lan_start_ip);
	nvram_set("dhcp1_end",lan_end_ip);
	nvram_set(LAN1_DNS,lan_modify_ip);
	inet_aton(nvram_safe_get("lan1_ipaddr"), (struct in_addr *)&guest_ip_addr);
	inet_aton(nvram_safe_get("lan1_netmask"), (struct in_addr *)&guest_ip_mask);
	nvram_commit();
	return 1;
	
}
int get_wan_connstatus(void)
{
	char *val;
	if(tenda_wan_link_status()){
		val = nvram_safe_get(WAN0_CONNECT);
		
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

int CGI_do_wan_connect_tenda(int action) 
{ 
	int wan_status = get_wan_connstatus();
	int wan_type = get_wan_type();

	nvram_set("err_check","0");/*add by ldm*/
	if(wan_type == 2)
	{
		if(action == 1){
			//dhcp release
			if(wan_status == 2){
				dhcpc_release1();
				nvram_set(WAN0_CONNECT,"Disconnected");
				nvram_set("wan0_isonln","no");
			}
		}else if(action == 2){
			//dhcp renew
			if(wan_status == 0){
				dhcpc_renew1();
				nvram_set(WAN0_CONNECT,"Connecting");				
			}
		}
	}else if(wan_type == 3){
		if(action == 3){
			//pppoe connect
			if(wan_status == 0){
				sys_pppoe_conn();
				nvram_set(WAN0_CONNECT,"Connecting");
			}
		}else if(action == 4){
			//pppoe disc
			if(wan_status != 0){
				sys_pppoe_disconn();
				nvram_set(WAN0_CONNECT,"Disconnected");
				nvram_set("wan0_isonln","no");
			}
		}
	}

	return 0;
} 
/*****************************************************************************
 函 数 名  : get_interface_state
 功能描述  : 获取当前接口的状态，
 输入参数  : char* net_name  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年10月24日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
NETWORK_INTERFACE_STATE get_interface_state(char* net_name)
{
	int skfd = 0;
	struct ifreq ifr;
	NETWORK_INTERFACE_STATE inter_state = INTERFACE_STAT_UNKNOW;

	memset(&ifr, 0, sizeof(ifr));
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0) 
	{
		printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
		return INTERFACE_STAT_UNKNOW;
	}

	strcpy(ifr.ifr_name, net_name);

	if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 ) 
	{
		printf("Maybe ethernet inferface %s is not valid!", ifr.ifr_name);
		close(skfd);
		return INTERFACE_STAT_UNKNOW;
	}

	if(ifr.ifr_flags & IFF_RUNNING) 
	{
		inter_state =  INTERFACE_UP;
	} 
	else 
	{
		inter_state = INTERFACE_DOWN;
	}
	close(skfd);
	return inter_state;
}
/*截取字符串的前几位，主要是为了防止
多个字节的被截断，导致读取出来的是乱码
Unicode符号范围 | UTF-8编码方式
(十六进制) | （二进制）
--------------------+---------------------------------------------
0000 0000-0000 007F | 0xxxxxxx
0000 0080-0000 07FF | 110xxxxx 10xxxxxx
0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

void get_ssid_prefix(char* origin_ssid,char*ssid_prefix,int cur_length)
{
	int index = 0;
	int nbytes = 0;	
	int offset_nbytes = 0;	
	unsigned char tmp = 0x0;
	unsigned char offset = 0x80;	 
	char ssid_tmp[64] = {0};	
	if(origin_ssid == NULL || ssid_prefix == NULL)
	{
		printf("error origin_ssid == NULL || ssid_prefix == NULL\n");
		return ;
	}
	printf("%s  %d\n",origin_ssid,cur_length);	 
	strncpy(ssid_tmp,origin_ssid,cur_length);	 
	/*先移动到最后一个字节*/	 
	index = strlen(ssid_tmp) - 1;         
	tmp = ssid_tmp[index];          
	while(tmp&0x80)	 
	{	 	
		nbytes++;        
		if(tmp&0x40)	 	
		{	 		
			while(tmp&offset)	 		
			{	 			
				offset_nbytes++;	 			
				offset = offset >> 1;			 		
			}			
			if(offset_nbytes != nbytes)				
				ssid_tmp[index] = '\0';           
			break;	 	
		}		
		index--;		
		if(index < 0)			
			break;		
		tmp = ssid_tmp[index];	 
	}     
	strcpy(ssid_prefix,ssid_tmp);
}
