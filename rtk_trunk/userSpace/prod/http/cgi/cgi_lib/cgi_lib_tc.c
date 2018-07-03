#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include "webs.h"
#include "uemf.h"
#include "cgi_common.h"
#include "../tc/tc.h"
#include <dhcp_server.h>
#include "cgi_tc.h"
#include "tenda_arp.h"
#include "wlioctl.h"
#include "wifi.h"
#include "apclient_dhcpc.h"
#include "shutils.h"
#include "cgi_lib.h"


#ifdef __CONFIG_STREAM_STATISTIC__
extern statistic_ip_index_t stream_ip[STREAM_CLIENT_NUMBER];
extern unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];
#ifdef __CONFIG_GUEST__
extern statistic_ip_index_t guest_stream_ip[STREAM_CLIENT_NUMBER];
extern unsigned int guest_stream_ip_per[STREAM_CLIENT_NUMBER][2];
#endif
#define KILO    1024
#endif
extern int get_menufacture_name(char* dev_mac,char* facturer_name);
extern int qosMacToLower(char *mac);
extern int is_gb2312_code(char * str);
extern void macSetMibEmpty();
extern int sscanfArglistConfig(const char *value, const char key , char **argc, int count);
extern void freeArglistConfig(char **argc, int count);
extern int get_macfilter_mode();
extern int wl_add_acladdr(char *ifname,char *mac);
extern void add_macfilter_rule(unsigned char *mac) ;
extern void remove_macfilter_rule(unsigned char *mac);//移除

enum {
	MODE_DISABLED = 0,	//禁用
	MODE_DENY,			//仅禁止
	MODE_PASS			//仅允许
};


RET_INFO cgi_lib_get_online_num(webs_t wp, cJSON *root, void *info)
{
	char iterm_value[PI_BUFLEN_64];
	int wl2_num = 0;
	int wl5_num = 0;
	memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
	if (nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
	{
			
		getWlStaNum( TENDA_WLAN24_AP_IFNAME, &wl2_num );
		getWlStaNum( TENDA_WLAN5_AP_IFNAME, &wl5_num );
	//桥模式下只需要统计无线客户端在线数量，且不管是否已经获取到ip，与用户管理页面一致
        	snprintf(iterm_value, PI_BUFLEN_64, "%d",wl2_num + wl5_num);
	}
	else
	{
	       snprintf(iterm_value, PI_BUFLEN_64, "%d", tenda_arp_get_online_client_num());
  	}
	cJSON_AddStringToObject(root, LIB_STATUS_ONLINE_NUM, iterm_value);
    	return RET_SUC;
}


RET_INFO cgi_lib_get_black_num(webs_t wp, cJSON *root, void *info)
{
	int wl_mode = 0, wl_mac_count = 0;
	char prefix[PI_BUFLEN_8] = {0}, tmp[PI_BUFLEN_64] = {0};
	char wl_mac_num[PI_BUFLEN_8] = {0};

	strcpy(wl_mac_num, nvram_safe_get("wl_macnum"));

	cJSON_AddStringToObject(root, LIB_STATUS_BLACK_NUM, wl_mac_num);
    
   	 return RET_SUC;
}

u_long get_stream_statistic(int dir)
{	
	char iterm_value[PI_BUFLEN_64];
	int	i = 0;
	u_long up_speed = 0;
	u_long down_speed = 0;
	char_t pre_ip[32]={'\0'}, pr_ip[32]={'\0'};
	char_t *lanip, *p;
	unsigned int index;
	unsigned int *mac_look_list;
	mac_look_list = NULL;
	

	for(i = 0;i<MAX_BRIDGE_NUM;++i)
	{
		if(i == 0)
		{
			_GET_VALUE(LAN_IPADDR, lanip);
			strcpy(pr_ip, lanip);
			p=rindex(pr_ip, '.');
			if(p) *p='\0';
		}
#ifdef __CONFIG_GUEST__
		else
		{
			_GET_VALUE(LAN1_IPADDR, lanip);
			strcpy(pr_ip, lanip);
			p=rindex(pr_ip, '.');
			if(p) *p='\0';
		}
#endif
		for(index = 1; index < STREAM_CLIENT_NUMBER; ++index)
		{
			sprintf(pre_ip, "%s.%u", pr_ip, index+1);
			if(!strcmp(pre_ip , lanip))
			{
				continue ;
			}
			
			mac_look_list = tenda_arp_ip_to_mac(inet_addr(pre_ip));
			if( NULL == mac_look_list)
			{
				continue ;
			}
		
			if(tenda_arp_is_multicast(mac_look_list))
				continue ;
			if(i == 0)
			{
				(up_speed) += (float)stream_ip_per[index][0] / (TIMES * KILO);
				(down_speed) += (float)stream_ip_per[index][1] / (TIMES * KILO);
			}
#ifdef __CONFIG_GUEST__
			else
			{
				(up_speed) += (float)guest_stream_ip_per[index][0] / (TIMES * KILO);
				(down_speed) += (float)guest_stream_ip_per[index][1] / (TIMES * KILO);
			}
#endif
		}

	}

	return dir ? up_speed:down_speed;
	
}

RET_INFO cgi_lib_get_stream_statistic(webs_t wp, cJSON *root, void *info)
{
	char iterm_value[PI_BUFLEN_64];
	int	i = 0;
	u_long up_speed = 0;
	u_long down_speed = 0;
	char_t pre_ip[32]={'\0'}, pr_ip[32]={'\0'};
	char_t *lanip, *p;
	unsigned int index;
	unsigned int *mac_look_list;
	mac_look_list = NULL;
	

	for(i = 0;i<MAX_BRIDGE_NUM;++i)
	{
		if(i == 0)
		{
			_GET_VALUE(LAN_IPADDR, lanip);
			strcpy(pr_ip, lanip);
			p=rindex(pr_ip, '.');
			if(p) *p='\0';
		}
#ifdef __CONFIG_GUEST__
		else
		{
			_GET_VALUE(LAN1_IPADDR, lanip);
			strcpy(pr_ip, lanip);
			p=rindex(pr_ip, '.');
			if(p) *p='\0';
		}
#endif
		for(index = 1; index < STREAM_CLIENT_NUMBER; ++index)
		{
			sprintf(pre_ip, "%s.%u", pr_ip, index+1);
			if(!strcmp(pre_ip , lanip))
			{
				continue ;
			}
			
			mac_look_list = tenda_arp_ip_to_mac(inet_addr(pre_ip));
			if( NULL == mac_look_list)
			{
				continue ;
			}
		
			if(tenda_arp_is_multicast(mac_look_list))
				continue ;
			if(i == 0)
			{
				(up_speed) += (float)stream_ip_per[index][0] / (TIMES * KILO);
				(down_speed) += (float)stream_ip_per[index][1] / (TIMES * KILO);
			}
#ifdef __CONFIG_GUEST__
			else
			{
				(up_speed) += (float)guest_stream_ip_per[index][0] / (TIMES * KILO);
				(down_speed) += (float)guest_stream_ip_per[index][1] / (TIMES * KILO);
			}
#endif
		}

	}
	
	memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
	snprintf(iterm_value, PI_BUFLEN_64, "%lu", up_speed);
	cJSON_AddStringToObject(root, LIB_STATUS_UP_SPEED, iterm_value);
	memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
	snprintf(iterm_value, PI_BUFLEN_64, "%lu", down_speed);
	cJSON_AddStringToObject(root, LIB_STATUS_DOWN_SPEED, iterm_value);
    
    return RET_SUC;
}


RET_INFO cgi_lib_get_wifi_rate(webs_t wp, cJSON *root, void *info)
{
	char iterm_value[PI_BUFLEN_64];

	WIFI_BRIDGE_INFO_STRUCT bridge_info;
	memset((char *)&bridge_info, 0x0, 1 * sizeof(WIFI_BRIDGE_INFO_STRUCT));
	gpi_wifi_get_bridge_rssi(WLAN_RATE_24G,&bridge_info);
	memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
	snprintf(iterm_value, PI_BUFLEN_64, "%d", (int)(bridge_info.rssi) - 100);
	cJSON_AddStringToObject(root, LIB_WIFI_RATE, iterm_value);
    return RET_SUC;
}


RET_INFO cgi_lib_get_wifi_name(webs_t wp, cJSON *root, void *info)
{
	char iterm_value[PI_BUFLEN_64];

	memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
	snprintf(iterm_value, PI_BUFLEN_64, "%s", nvram_safe_get("wl0_ssid"));
#ifdef __CONFIG_AUTO_CONN_CLIENT__
	if(0 == strcmp(iterm_value, "Tenda_Extender_1"))
	{
		memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
		snprintf(iterm_value, PI_BUFLEN_64, "%s", nvram_safe_get("wl0.1_ssid"));
	}
#endif
	cJSON_AddStringToObject(root, LIB_ROUTER_NAME, iterm_value);

	memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
	snprintf(iterm_value, PI_BUFLEN_64, "%s", nvram_safe_get("wl0.1_ssid"));
	cJSON_AddStringToObject(root, LIB_EXTEND_NAME, iterm_value);
	
    
    return RET_SUC;
}

RET_INFO cgi_lib_get_localhost(webs_t wp, cJSON *root, void *info)
{
	P_DHCP_SERVER_INFO_STRUCT dhcp_lan_info   = NULL;
	char* p_host_mac = NULL;
	in_addr_t local_ip;
	dhcp_lan_info   = gpi_dhcp_server_info();
	if(wp  != NULL)
	{
		inet_aton(wp->ipaddr, &local_ip);
		p_host_mac = tenda_arp_ip_to_mac(local_ip);
		if(p_host_mac)
			cJSON_AddStringToObject(root, LIB_LOCAL_HOST_MAC, inet_mactoa(p_host_mac));
		else
			cJSON_AddStringToObject(root, LIB_LOCAL_HOST_MAC,	"00:00:00:00:00:00");
		/* get ip address of loacal host */
		cJSON_AddStringToObject(root, LIB_LOCAL_HOST, wp->ipaddr);

		cJSON_AddStringToObject(root, LIB_LAN_MASK, dhcp_lan_info->lan_mask);
	}
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : get_cur_rate
 功能描述  : 获取该设备当前的上传下载速率
 输入参数  : 无net_type   1 主网络   0 访客网络
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年10月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO get_cur_rate(int net_type,in_addr_t  ip, unsigned int*  up_byte_pers, unsigned int*  down_byte_pers)
{
	struct in_addr cur_ip;
	int ip_part[4][4] = {0};
	int  u_kbs = 0, d_kbs = 0, u_kbs0 = 0, d_kbs0 = 0, u_kbs1 = 0, d_kbs1 = 0, u_kbs2 = 0, d_kbs2 = 0;
	unsigned int ip_number = 0;
	int up = 0, down = 0;
	int up1 = 0, down1 = 0;
	int up2 = 0, down2 = 0;
	char  vlaue[32] = {0};
	cur_ip.s_addr = ip;

	bzero(ip_part, sizeof(ip_part));
	sscanf(inet_ntoa(cur_ip), "%[^.].%[^.].%[^.].%u", ip_part[0], ip_part[1], ip_part[2], &ip_number);

	(ip_ubs(net_type,ip_number - 1) != 0) ? (u_kbs0 = ip_ubs(net_type,ip_number - 1) / KILO) : (u_kbs0 = 0);
	(ip_dbs(net_type,ip_number - 1) != 0) ? (d_kbs0 = ip_dbs(net_type,ip_number - 1) / KILO) : (d_kbs0 = 0);

	(ip_ubs(net_type,ip_number - 1) != 0) ? (u_kbs = ip_ubs(net_type,ip_number - 1) % KILO) : (u_kbs = 0);
	(ip_dbs(net_type,ip_number - 1) != 0) ? (d_kbs = ip_dbs(net_type,ip_number - 1) % KILO) : (d_kbs = 0);

	(u_kbs != 0) ? (u_kbs1 = u_kbs / 100) : (u_kbs1 = 0);
	(d_kbs != 0) ? (d_kbs1 = d_kbs / 100) : (d_kbs1 = 0);
	(u_kbs1 != 0) ? (u_kbs2 = u_kbs1 / 10) : (u_kbs2 = 0);
	(d_kbs1 != 0) ? (d_kbs2 = d_kbs1 / 10) : (d_kbs2 = 0);

	(ip_uby(net_type,ip_number - 1) != 0) ? (up = ip_uby(net_type,ip_number - 1) / KILO) : (up = 0);
	(ip_dby(net_type,ip_number - 1) != 0) ? (down = ip_dby(net_type,ip_number - 1) / KILO) : (down = 0);

	(up != 0) ? (up1 = up / 100) : (up1 = 0);
	(down != 0) ? (down1 = down / 100) : (down1 = 0);
	(up1 != 0) ? (up2 = up1 / 10) : (up2 = 0);
	(down1 != 0) ? (down2 = down1 / 10) : (down2 = 0);
	sprintf(vlaue, "%d.%d%d", (d_kbs0), (d_kbs1), (d_kbs2));
	*down_byte_pers = (int)(atof(vlaue));
	memset(vlaue, 0x0, sizeof(vlaue));
	sprintf(vlaue, "%d.%d%d", (u_kbs0), (u_kbs1), (u_kbs2));
	*up_byte_pers = (int)(atof(vlaue));

	return RET_SUC;		
}

/*****************************************************************************
 函 数 名  : cgi_route_get_online_list
 功能描述  : 获取在线用户列表，在路由模式下
 输入参数  : 无
 输出参数  : 无
 返 回 值  : RET_INFO

 修改历史      :
  1.日    期   : 2016年10月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_router_mode_get_online_list(webs_t wp, cJSON *root, void *info)
{
	int is_in_tc = 0;
	int client_num = 0;
	int print_black = 0;
	char name[6] = {0};
	cJSON *obj = NULL;
	struct in_addr cur_ip;
	int is_in_qos_list = 0;
	char  vlaue[32] = {0};
	char  facturer_name[16] = {0};
	cJSON *array = NULL;
	unsigned int index = 0;
	int is_in_mac_filter = 0;
	float cur_up, cur_down;
	char *qosListRemark = NULL;
	unsigned int  up_byte_pers = 0;
	unsigned int  down_byte_pers = 0;
	struct ether_addr *hw_addr = NULL;
	struct client_info  *clients_list = NULL;
	time_t now_time = time(0);
    
	stream_list_t  *qosInfo = (stream_list_t *)malloc(sizeof(stream_list_t));

	clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list != NULL )
	{
		memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
		client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);
	}
	else
	{
		perror("memery exhausted!");
		client_num = 0;
	}

	for(index = 0; index < client_num; ++index)
	{
		hw_addr = ether_aton(clients_list[index].mac);
		if(hw_addr == NULL)
			continue;

		qosMacToLower(clients_list[index].mac);
		memset(qosInfo, 0x0, sizeof(stream_list_t));
		if(get_macfilter_mode() == MODE_PASS)
		{
			is_in_mac_filter = -1;
		}
		else
		{
		is_in_mac_filter = is_mac_filter(clients_list[index].mac,"deny");
		}
		if(is_in_mac_filter == -1)
		{

			is_in_tc = is_tc(clients_list[index].mac);
			if(is_in_tc != -1)
			{
				get_limit_rate(&cur_up, &cur_down, is_in_tc);
				qosInfo->up_limit = cur_up;
				qosInfo->down_limit = cur_down;
			}
			else
			{
				qosInfo->up_limit = UP_RATE_LIMIT;
				qosInfo->down_limit = DOWN_RATE_LIMIT;
			}
			qosInfo->access = 1;
			qosInfo->ip = inet_addr(clients_list[index].ip);
			cur_ip.s_addr = qosInfo->ip;
			qosInfo->type = tenda_arp_mac_to_flag(clients_list[index].mac);
#ifdef __CONFIG_GUEST__
			if(qosInfo->type == 3 || qosInfo->type == 4)
				continue;
#endif
			if(qosInfo->type == 1 || qosInfo->type == 2)
			{
				if(0 == tenda_arp_is_wireless_client(hw_addr))
				{
					continue;
				}
			}
			
			strcpy(qosInfo->mac,  clients_list[index].mac);
			if (clients_list[index].hostname[0] == '\0')
			{
				strcpy(clients_list[index].hostname, "Unknown");
			}
			snprintf(qosInfo->hostname, TPI_BUFLEN_64, "%s", clients_list[index].hostname);
			strcpy(qosInfo->remark, clients_list[index].mark);

			get_cur_rate(1,qosInfo->ip, &up_byte_pers, &down_byte_pers);

			qosInfo->down_byte_pers = down_byte_pers;

			qosInfo->up_byte_pers = up_byte_pers;
			if(qosInfo->access == 1 && WL_APCLIENT_MODE != gpi_wifi_get_mode())
			{
				cJSON_AddItemToArray(root, obj = cJSON_CreateObject());

				qosListRemark = get_remark(qosInfo->mac);

#ifdef __CONFIG_SUPPORT_GB2312__
				if (1 == is_cn_encode(qosInfo->hostname))
				{
					char host_name_utf_8[64] = {0};
					set_cn_ssid_encode("utf-8", qosInfo->hostname, host_name_utf_8);
					strcpy(qosInfo->hostname, host_name_utf_8);
				}
#else
				if(1 == is_gb2312_code(qosInfo->hostname))
				{
					strcpy(qosInfo->hostname, "Unknown") ;
				}
#endif
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_HOSTNAME), qosInfo->hostname);
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_REMARK), (qosListRemark == NULL) ? "" : qosListRemark);
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_IP), inet_ntoa(cur_ip));
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_CONN_TYPE), (clients_list[index].l2type ) ? "wifi" : "wires");
				if(qosInfo->type == 1)
				{
					cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_ACCESS_TYPE),"wifi_2G");
				}
				else if(qosInfo->type == 2)
				{
					cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_ACCESS_TYPE),"wifi_5G");
				}
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_MAC), qosInfo->mac);
#ifdef __CONFIG_GET_MANUFACTURER__
				get_menufacture_name(qosInfo->mac,facturer_name);
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_MANUFACTURER),facturer_name);
#endif
				sprintf(vlaue, "%.02f", ((float)qosInfo->down_byte_pers));
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_SPEED), vlaue);
				sprintf(vlaue, "%.02f", ((float)qosInfo->up_byte_pers));
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_SPEED), vlaue);
				sprintf(vlaue, "%.02f", qosInfo->down_limit * 128);
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_LIMIT), vlaue);
				sprintf(vlaue, "%.02f", qosInfo->up_limit * 128);
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_LIMIT), vlaue);
				cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_ACCESS), "true");
				cJSON_AddNumberToObject(obj, T(LIB_QOS_LIST_CONNECT_TIME), now_time -clients_list[index].time);
			}
		}
	}
	FREE_P(&clients_list);
	FREE_P(&qosInfo);

	return RET_SUC;		//根据coverity分析结果修改 原来存在问题:无返回值  2017/2/5 F9项目修改
}

/*****************************************************************************
 函 数 名  : cgi_ap_mode_online_list
 功能描述  : 获取AP模式下在线用户列表
 输入参数  : 无
 输出参数  : 无
 返 回 值  : RET_INFO

 修改历史      :
  1.日    期   : 2016年10月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
#ifndef MAX_STA_NUM
#define MAX_STA_NUM  64
#endif
RET_INFO cgi_ap_mode_online_list(webs_t wp, cJSON *root, void *info)
{
	unsigned int i;
	char dev_mac[PI_MAC_STRING_LEN] = {0};
	char dev_ip[PI_IP_STRING_LEN] = {0};
	char dev_name[PI_DEVNAME_STRING_LEN] = {0};
	char *qosListRemark = NULL;
	struct maclist *mac_list = NULL;
	int mac_list_size;
	cJSON *obj = NULL;
	in_addr_t local_ip;
	char local_mac[20] = {0};
	char* p_host_mac = NULL;
	char cur_IP[20] = {0};
	in_addr_t  cur_addr;
	char facturer_name[16] = {0};
	int is_in_tc = 0;
	float cur_up, cur_down;
	char  vlaue[32] = {0};
	char *wlan_ifname = TENDA_WLAN24_AP_IFNAME;
	int wlan_index = 0;
	struct in_addr a;
	if(wp != NULL)
	{
		inet_aton(wp->ipaddr, &local_ip);
		p_host_mac = tenda_arp_ip_to_mac(local_ip);
		if(p_host_mac)
			snprintf(local_mac, 18, "%s", inet_mactoa(p_host_mac));
		else
			snprintf(local_mac, 18, "%s", "00:00:00:00:00:00");
	}
	else
	{
		snprintf(local_mac, 18, "%s", "00:00:00:00:00:00");
	}
	qosMacToLower(local_mac);
	mac_list_size = sizeof(uint) + MAX_STA_NUM * sizeof(struct ether_addr);
	mac_list = (struct maclist *)malloc(mac_list_size);
	if(NULL == mac_list)
	{
		printf("*** fun=%s; line=%d; no buffers! ***\n", __func__, __LINE__);

		return -1;
	}
	for(wlan_index = 0 ; wlan_index < TENDA_WLAN_SUPPORT_HZ;wlan_index++)
	{
		memset(mac_list,0x0,mac_list_size);
		if(wlan_index)
			wlan_ifname = TENDA_WLAN5_AP_IFNAME;
		
		if (getwlmaclist(wlan_ifname, mac_list))
		{
			free(mac_list);

			return 0;
		}
		struct apclient_client_info* temp = NULL;

		for (i = 0; i < mac_list->count; ++i)
		{
			memset(dev_mac, '\0', PI_MAC_STRING_LEN * sizeof(char));
			snprintf(dev_mac, PI_MAC_STRING_LEN, "%s", inet_mactoa(mac_list->ea[i].octet));
			qosMacToLower(dev_mac);
			temp = gpi_apclient_dhcpc_get_client_info(mac_list->ea[i].octet);
			if(temp == NULL)
			{
				strcpy(dev_name, "UnKnown");
			}
			else
			{
				memset(dev_name, '\0', PI_DEVNAME_STRING_LEN * sizeof(char));
				snprintf(dev_name, PI_DEVNAME_STRING_LEN, temp->dev_name);
				if(!strcmp(dev_name, ""))
				{
					strcpy(dev_name, "UnKnown");
				}

				if(1 == is_gb2312_code(dev_name))
				{
					strcpy(dev_name, "Unknown") ;

				}
			}
			if(strcmp(dev_mac, local_mac) == 0)
			{
				if(wp != NULL)
				{
					strcpy(cur_IP, wp->ipaddr);
				}
				else
				{
					strcpy(cur_IP, "0.0.0.1");
				}
			}
			else
			{
				strcpy(cur_IP, "0.0.0.1");
			}
			
			if(( is_mac_filter(dev_mac,"deny") != -1) && (get_macfilter_mode() == MODE_DENY))
				continue;

			
			is_in_tc = is_tc(dev_mac);
			if(is_in_tc != -1)
			{
				get_limit_rate(&cur_up, &cur_down, is_in_tc);
			}
			else
			{
				cur_up = UP_RATE_LIMIT;
				cur_down = DOWN_RATE_LIMIT;
			}
			cJSON_AddItemToArray(root, obj = cJSON_CreateObject());

			qosListRemark = get_remark(dev_mac);

			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_HOSTNAME), dev_name);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_REMARK), (qosListRemark == NULL) ? "" : qosListRemark);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_IP), cur_IP);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_CONN_TYPE), "wifi");
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_MAC), dev_mac);
#ifdef __CONFIG_GET_MANUFACTURER__
			get_menufacture_name(dev_mac,facturer_name);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_MANUFACTURER), facturer_name);
#endif
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_SPEED), "0");
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_SPEED), "0");
			sprintf(vlaue, "%.02f", cur_down * 128);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_LIMIT), vlaue);
			sprintf(vlaue, "%.02f", cur_up * 128);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_LIMIT), vlaue);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_ACCESS), "true");
			cJSON_AddNumberToObject(obj, T(LIB_QOS_LIST_CONNECT_TIME), 0);
		}
	}
	FREE_P(&mac_list);
	return RET_SUC;		//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
}


RET_INFO cgi_lib_get_tc_online_list(webs_t wp, cJSON *root, void *info)
{
	if(nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
	{
		#ifdef __CONFIG_BRIDGE_AP__
		cgi_ap_mode_online_list(wp, root, info);
		#endif
	}
	else
	{
		cgi_router_mode_get_online_list(wp, root, info);
	}
	return RET_SUC;
}
#ifdef __CONFIG_GUEST__
RET_INFO cgi_lib_get_guest_online_list(webs_t wp, cJSON *root, void *info)
{
	int client_num = 0;
	cJSON *obj = NULL;
	struct in_addr cur_ip;
	char  vlaue[32] = {0};
	char  facturer_name[16] = {0};
	unsigned int index = 0;
	unsigned int  up_byte_pers = 0;
	unsigned int  down_byte_pers = 0;
	struct ether_addr *hw_addr = NULL;
	struct client_info  *clients_list = NULL;
	time_t now_time = time(0);
    
	stream_list_t  *qosInfo = (stream_list_t *)malloc(sizeof(stream_list_t));

	clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list != NULL )
	{
		memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
		client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);
	}
	else
	{
		perror("memery exhausted!");
		client_num = 0;
	}

	for(index = 0; index < client_num; ++index)
	{
		hw_addr = ether_aton(clients_list[index].mac);
		if(hw_addr == NULL)
			continue;

		memset(qosInfo, 0x0, sizeof(stream_list_t));

		qosInfo->ip = inet_addr(clients_list[index].ip);
		cur_ip.s_addr = qosInfo->ip;
		qosInfo->type = tenda_arp_mac_to_flag(clients_list[index].mac);
		if(qosInfo->type == 0 
			|| qosInfo->type == 1 
			|| qosInfo->type == 2)
			continue;
		
		if(qosInfo->type == 3 || qosInfo->type == 4)
		{
			if(0 == tenda_arp_is_wireless_client(hw_addr))
			{
				continue;
			}
		}
			
		snprintf(qosInfo->hostname, TPI_BUFLEN_64, "%s", clients_list[index].hostname);
		strcpy(qosInfo->mac,  clients_list[index].mac);

		get_cur_rate(0,qosInfo->ip, &up_byte_pers, &down_byte_pers);

		qosInfo->down_byte_pers = down_byte_pers;

		qosInfo->up_byte_pers = up_byte_pers;
		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_HOSTNAME), qosInfo->hostname);
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_IP), inet_ntoa(cur_ip));
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_MAC), qosInfo->mac);
#ifdef __CONFIG_GET_MANUFACTURER__
		get_menufacture_name(clients_list[index].mac,facturer_name);
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_MANUFACTURER),facturer_name);
#endif
		sprintf(vlaue, "%.02f", ((float)qosInfo->down_byte_pers));
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_SPEED), vlaue);
		sprintf(vlaue, "%.02f", ((float)qosInfo->up_byte_pers));
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_SPEED), vlaue);
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_CONN_TYPE), "wifi");
		//接入类型,APP区分是2.4G访客还是5G访客
		if(qosInfo->type == 3)	//2.4G访客
		{
			cJSON_AddStringToObject(obj,LIB_QOS_LIST_ACCESS_TYPE,"wifi_guest_2G");
		}
		else if(qosInfo->type == 4)
		{
			cJSON_AddStringToObject(obj,LIB_QOS_LIST_ACCESS_TYPE,"wifi_guest_5G");
		}
		cJSON_AddNumberToObject(obj, T(LIB_QOS_LIST_CONNECT_TIME), now_time -clients_list[index].time);
	}
	FREE_P(&clients_list);
	FREE_P(&qosInfo);

	return RET_SUC;
}
#endif
/*****************************************************************************
 函 数 名  : get_limit_rate
 功能描述  : 获取限速的大小
 输入参数  : int* up
             int* down
             int index
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年10月9日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void get_limit_rate( float* up, float* down, int index )
{
	int i = 0;
	char *value = NULL;
	char mac[32], proto[32], start_ip[32], end_ip[32], up_limit[32], down_limit[32];
	char up_or_down[2];
	int m_access;
	char end[256] = {0};

	_GET_VALUE(ADVANCE_TC_RULE(index), value);
	sscanf(value, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", mac, proto, start_ip, end_ip, up_or_down, up_limit, end);
	*up = atof(up_limit) / 128;

	index++;
	_GET_VALUE(ADVANCE_TC_RULE(index), value);
	sscanf(value, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", mac, proto, start_ip, end_ip, up_or_down, down_limit, end);
	*down = atof(down_limit) / 128;
}


/*****************************************************************************
 函 数 名  : is_tc
 功能描述  : 判断是否在TC中，如果是，则返回序号
 输入参数  : char* mac
 输出参数  : 无
 返 回 值  : int

 修改历史      :
  1.日    期   : 2016年10月9日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int is_tc(char* mac)
{
	int i = 0;
	char *value = NULL;
	char name[16] = {0};

	for (i = 0; i < TC_RULE_NUMBER_MAX * 2; ++i)
	{
		sprintf(name, "tc_%d", i);
		_GET_VALUE(name, value);
		if (!strcmp(value, "") || strlen(value) < 8  || 0 != strncmp(mac, value, strlen(mac)))
			continue;
		return i;
	}
	return -1;
}



/*****************************************************************************
 函 数 名  : set_tc_nvram
 功能描述  : 配置TC的nvram参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年10月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/

struct stream_list_enable* add_node_head = NULL;
void set_tc_param(stream_list_t *qosInfo, char *remark, char *m_hostname)
{
	int i = 0;
	char *value = NULL;
	char name[16] = {0};
	int   mac_filter_adrr = -1;
	int   is_old_param = 0;

	if (NULL == qosInfo || NULL == remark || NULL == m_hostname)
		return;
	/*将当前nvram中在线的配置清空*/
	for (i = 0; i < 2 * TC_RULE_NUMBER_MAX; ++i)
	{
		_GET_VALUE(ADVANCE_TC_RULE(i), value);

		if (!strcmp(value, "") ||  strlen(value) <= 8 || 0 != strncmp(value, qosInfo->mac, strlen(qosInfo->mac)))
			continue;

		_SET_VALUE(ADVANCE_TC_RULE(i), "");
		i++;
		_SET_VALUE(ADVANCE_TC_RULE(i), "");
		break;
	}
	/******************************************/
	if( qosInfo->up_limit == 301 && qosInfo->down_limit == 301 )
	{
		return;
	}

	/**********将一条配置添加到临时的链表中**********/
	struct stream_list_enable* add_node =  (struct stream_list_enable*)malloc(sizeof(struct stream_list_enable));

	if(add_node == NULL)
	{
		printf("%s  %d malloc error\n", __FUNCTION__, __LINE__);
		return;
	}
	memset(add_node, 0x0, sizeof(struct stream_list_enable));

	stream_list_t *qosInfo_temp = (stream_list_t*)malloc(sizeof(stream_list_t));
	if(qosInfo_temp == NULL)
	{
		printf("%s  %d malloc error\n", __FUNCTION__, __LINE__);
		free(add_node);		//根据converity分析结果修改   2017/1/10 F9项目
		return;
	}
	memset(qosInfo_temp, 0x0, sizeof(stream_list_t));

	qosInfo_temp->type = qosInfo->type;
	qosInfo_temp->limit = qosInfo->limit;
	qosInfo_temp->access = qosInfo->access;
	qosInfo_temp->down_limit = qosInfo->down_limit;
	qosInfo_temp->up_limit = qosInfo->up_limit;
	qosInfo_temp->set_pers = qosInfo->set_pers;
	qosInfo_temp->down_byte_pers = qosInfo->down_byte_pers;
	qosInfo_temp->up_byte_pers = qosInfo->up_byte_pers;
	qosInfo_temp->ip = qosInfo->ip;
	if(memcpy(qosInfo_temp->hostname, m_hostname, 64) == NULL)
	{
		goto error;
	}

	if(memcpy(qosInfo_temp->remark, remark, 128) == NULL)
	{
		goto error;
	}

	if(memcpy(qosInfo_temp->mac, qosInfo->mac, 20) == NULL)
	{
		goto error;
	}


	add_node->node = qosInfo_temp;
	add_node->next = add_node_head;
	add_node_head = add_node;
	/**************************************************************/
	return ;

error:
	free(qosInfo_temp);
	free(add_node);
	qosInfo_temp = NULL;
}


/*****************************************************************************
 函 数 名  : reversed_tc_node
 功能描述  : 执行逆序操纵
 输入参数  : 无
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
inline int  reversed_tc_node(struct stream_list_enable** head)
{
	struct stream_list_enable* cur_node = NULL;
	struct stream_list_enable* temp_node = NULL;
	struct stream_list_enable* new_head = NULL;
	cur_node = *head;
	while(cur_node != NULL)
	{
		temp_node = cur_node;
		cur_node = cur_node->next;
		temp_node->next = new_head;
		new_head = temp_node;
	}

	*head = new_head;
	cur_node = *head;

	return 0;	//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
}

/*****************************************************************************
 函 数 名  : set_item_tc
 功能描述  : 设置一条tc的nvram参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
inline void set_item_tc(struct stream_list_enable* cur_node, int i)
{
	struct in_addr cur_addr;
	int ip_part[4][4] = {0};
	char tc_list[128] = {0};
	cur_addr.s_addr = cur_node->node->ip;
	bzero(ip_part, sizeof(ip_part));
	sscanf(inet_ntoa(cur_addr), "%[^.].%[^.].%[^.].%s", ip_part[0], ip_part[1], ip_part[2], ip_part[3]);
	memset(tc_list, 0x0, sizeof(tc_list));

	sprintf(tc_list, "%s,80,%s,%s,0,%d,%d,1,0,%s", cur_node->node->mac, ip_part[3], ip_part[3],
	        (int)((cur_node->node->up_limit) * 128), (int)((cur_node->node->up_limit) * 128), cur_node->node->remark[0] == '\0' ? cur_node->node->hostname : cur_node->node->remark);
	_SET_VALUE(ADVANCE_TC_RULE(i), tc_list);

	memset(tc_list, 0x0, sizeof(tc_list));

	sprintf(tc_list, "%s,80,%s,%s,1,%d,%d,1,0,%s", cur_node->node->mac, ip_part[3], ip_part[3],
	        (int)((cur_node->node->down_limit) * 128), (int)((cur_node->node->down_limit) * 128),  cur_node->node->remark[0] == '\0' ? cur_node->node->hostname : cur_node->node->remark);

	_SET_VALUE(ADVANCE_TC_RULE(i + 1), tc_list);
}

/*****************************************************************************
 函 数 名  : free_tc_node
 功能描述  : 清空申请的内存
 输入参数  : 无
 输出参数  : 无
 返 回 值  : inline void

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
inline void free_tc_node(struct stream_list_enable** head)
{
	struct stream_list_enable* temp = NULL;
	struct stream_list_enable* cur_node = NULL;
	cur_node = *head;
	while(cur_node)
	{
		temp = cur_node;
		cur_node = temp->next;
		free(temp->node);
		free(temp);
		temp = NULL;
	}
	*head = NULL;
}

/*****************************************************************************
 函 数 名  : set_tc_nvram
 功能描述  : 将TC的所有参数设置到nvram里面
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年10月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void set_tc_nvram()
{
	int i = 0;
	int j = 0;
	char *value = NULL;
	char name[16] = {0};
	char cur_tc_value[256] = {0};

	/*将不再线的设备配置向前移动*/
	for (i = 0; i < 2 * TC_RULE_NUMBER_MAX; ++i)
	{

		_GET_VALUE(ADVANCE_TC_RULE(i), value);
		memset(cur_tc_value, 0x0, 256);
		strncpy(cur_tc_value, value, strlen(value));

		if (!strcmp(value, "") ||  strlen(value) <= 8)
			continue;

		_SET_VALUE(ADVANCE_TC_RULE(i), "");
		_SET_VALUE(ADVANCE_TC_RULE(j), cur_tc_value);
		j++;
	}
	/******************************************/

	/*执行一次逆序操作，因为之前的插入是头插*/
	reversed_tc_node(&add_node_head);

	/*************************************************************/

	/*将所有在线并且设置了流控的配置进行写入*/
	struct stream_list_enable* cur_node = NULL;
	i = j;

	cur_node = add_node_head;
	if(cur_node != NULL)
	{
		_SET_VALUE(ADVANCE_TC_ENABLE, "1");
	}
	
	while(cur_node != NULL)
	{
		set_item_tc(cur_node, i);
		i += 2;
		cur_node = cur_node->next;
	}
	/**************************************************************/

	/*将所有的配置向前移动，也就是将前面不再线的删除掉*/
	if(i  >  2 * TC_RULE_NUMBER_MAX)
	{
		int offset = i - 2 * TC_RULE_NUMBER_MAX;

		for (i = 0; i < 2 * TC_RULE_NUMBER_MAX; ++i)
		{
			_GET_VALUE(ADVANCE_TC_RULE(i + offset), value);
			memset(cur_tc_value, 0x0, 256);
			strncpy(cur_tc_value, value, strlen(value));
			_SET_VALUE(ADVANCE_TC_RULE(i), cur_tc_value);
		}
		for(i = 0; i < offset; i++)
		{
			nvram_unset(ADVANCE_TC_RULE(i + 2 * TC_RULE_NUMBER_MAX));
		}

	}
	/*********************************************************************************/

	/********free 掉申请的内存*******/
	free_tc_node(&add_node_head);

	/**************************************/
}


RET_INFO  cgi_lib_set_tc_qoslist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	char *list;
	char m_hostname[128] = {0};
	char m_remark[128] = {0};
	char m_mac[32] = {0};
	char m_access[32] = {0};
	float  m_up_limit, m_down_limit;
	char up_limit_temp[32];
	char down_limit_temp[32];
	char *arglist[MAX_CLIENT_NUMBER] = {""};
	char *p = NULL;
	int i = 0;
	stream_list_t qosInfo;
	char *argc[6] = {0};
	int count = 0;
	char msg_param[PI_BUFLEN_256] = {0};


	list = cgi_lib_get_var(wp,root, T(LIB_QOS_ONLINE_LIST), T(""));

	p = list;

	int tc_num = 0;
	tc_num = str2arglist(p, arglist, '\n', MAX_CLIENT_NUMBER);
	for (i = 0; i < tc_num; ++i)
	{

		if (arglist[i] == NULL || strlen(arglist[i]) < 10)
			continue;

		count = sscanfArglistConfig(arglist[i], '\t' ,  argc, 6);

		if(count != 6)
		{
			freeArglistConfig(argc, count);
			sprintf(err_code,"%s","1");
			return RET_ERR;
		}
		sprintf(m_hostname , "%s" , argc[0]);
		sprintf(m_remark 	, "%s" , argc[1]);
		sprintf(m_mac , "%s" , argc[2]);
		sprintf(m_access , "%s" , argc[5]);
		m_up_limit = atof(argc[3]) / 128;
		m_down_limit = atof(argc[4]) / 128;
		snprintf(up_limit_temp, sizeof(up_limit_temp), "%f", m_up_limit);
		snprintf(down_limit_temp, sizeof(down_limit_temp), "%f", m_down_limit);
		freeArglistConfig(argc, 6);
		memset(&qosInfo, 0x0, sizeof(qosInfo));
		qosMacToLower(m_mac);
		strcpy(qosInfo.mac, m_mac);

		qosInfo.ip = tenda_arp_mac_to_ip(m_mac);
		qosInfo.up_limit = (m_up_limit > 300.00) ? 301 : m_up_limit;
		qosInfo.down_limit = ( m_down_limit > 300.00) ? 301 : m_down_limit;

		if (0 == strcmp(m_access, "true") && 0 != qosInfo.up_limit && 0 != qosInfo.down_limit )
			qosInfo.access = 1;
		else
			qosInfo.access = 0;

		//保存备注名

		if(m_remark[0] != '\0' )
		{
			add_remark(m_mac, m_remark);
		}
		else if(get_remark(m_mac) != NULL)
		{
			add_remark(m_mac, m_hostname);
		}

		set_tc_param(&qosInfo, m_remark, m_hostname);

	}
	set_tc_nvram();	
	sprintf(err_code,"%s","0");
	CGI_MSG_MODULE msg_tmp;
	msg_tmp.id = RC_TC_MODULE;
	sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
	add_msg_to_list(msg,&msg_tmp);
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : get_all_expired_info
 功能描述  : 获取所有离线设备信息
 输入参数  : webs_t wp,
 			 cJSON *root
 			 void *info
 输出参数  : 无
 返 回 值  : 

 修改历史      :
  1.日    期   : 2016年12月20日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
extern struct detec *expired_list;
extern cyg_mutex_t expired_list_lock;

RET_INFO cgi_lib_get_all_expired_info (webs_t wp, cJSON *root, void *info)
{
	int i = 0, j = 0;
	int is_in_tc = 0;
	char value[32] = {0};
	char facturer_name[16] = {0};
	int client_num = 0;
	float cur_up, cur_down;
	struct detec *tmp_node = NULL;
    char expired_mac[TPI_MAC_STRING_LEN] = {0};  //mac address
    char expired_ip[TPI_IP_STRING_LEN] = {0};    //ip address
	char hostname[TPI_BUFLEN_64] = {0};          //hostname
	char mark[TPI_BUFLEN_64] = {0};              //mark
    char *remark = NULL;
	time_t now_time = time(0);
	
    cJSON *obj = NULL;
    
    cyg_mutex_lock(&expired_list_lock);
	tmp_node = expired_list;
	while(tmp_node)
	{		
		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());//
		
        snprintf(expired_mac ,TPI_MAC_STRING_LEN, "%02x:%02x:%02x:%02x:%02x:%02x" , 
			    tmp_node->mac[0], tmp_node->mac[1], tmp_node->mac[2], tmp_node->mac[3], tmp_node->mac[4], tmp_node->mac[5]);
		
		cJSON_AddStringToObject(obj,LIB_QOS_LIST_MAC,expired_mac);
		/*获取离线设备的是上传下载的限速*/
		is_in_tc = is_tc(expired_mac);
		if(-1 != is_in_tc)
		{
			get_limit_rate(&cur_up, &cur_down, is_in_tc);

			sprintf(value, "%.02f", cur_down * 128);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_LIMIT), value);
			sprintf(value, "%.02f", cur_up *128);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_LIMIT), value);
		}
		else
		{
			sprintf(value, "%.02f", DOWN_RATE_LIMIT * 128);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_LIMIT), value);
			sprintf(value, "%.02f", UP_RATE_LIMIT *128);
			cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_LIMIT), value);
		}
        cJSON_AddStringToObject(obj,LIB_QOS_LIST_IP,"0.0.0.0");	//根据app规格要求离线设备的IP统一为0 2017/1/9 修改
        remark = get_remark(expired_mac);
		//2017/2/5修改 解决离线设备主机名出现乱码问题
#ifdef __CONFIG_SUPPORT_GB2312__
		if (1 == is_cn_encode(tmp_node->hostname))
		{
			char host_name_utf_8[64] = {0};
			set_cn_ssid_encode("utf-8", tmp_node->hostname, host_name_utf_8);
			strcpy(tmp_node->hostname, host_name_utf_8);
		}
#else
		if(1 == is_gb2312_code(tmp_node->hostname))
		{
			strcpy(tmp_node->hostname, "Unknown");
		}
#endif
		//2017/1/12修改  解决离线设备备注名导致所有的离线设备备注名都改变的问题
		cJSON_AddStringToObject(obj,LIB_QOS_LIST_REMARK,(remark == NULL) ? "" : remark); 
		cJSON_AddStringToObject(obj,LIB_QOS_LIST_HOSTNAME,tmp_node->hostname);
		
#ifdef __CONFIG_GET_MANUFACTURER__
		get_menufacture_name(expired_mac,facturer_name);
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_MANUFACTURER),facturer_name);
#endif
		cJSON_AddStringToObject(obj,LIB_QOS_LIST_CONN_TYPE,(tmp_node->flag) ? "wifi" : "wires");
        
	    /*离线设备，这些字段都是假数据*/	
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_DOWN_SPEED), "0");
		cJSON_AddStringToObject(obj, T(LIB_QOS_LIST_UP_SPEED), "0");
		
		cJSON_AddNumberToObject(obj,LIB_QOS_LIST_CONNECT_TIME,now_time - tmp_node->update_time);
	    cJSON_AddNumberToObject(obj,LIB_EXPIRED_IS,0);

        tmp_node = tmp_node->next;
	}
    cyg_mutex_unlock(&expired_list_lock);
   
	return RET_SUC;
}

/*************************************************************************
  功能: 获取离线设备的数量
  参数: 无
  返回值: 离线设备的个数
 ************************************************************************/
RET_INFO cgi_lib_get_expired_client_num(webs_t wp, cJSON *root, void *info)
{
	int i, client_num = 0,j = 0;
	
	struct detec *tmp_node = NULL;

    cyg_mutex_lock(&expired_list_lock);
	tmp_node = expired_list;
	while(tmp_node)
	{
		tmp_node = tmp_node->next;
        client_num++;
	}
     
    cyg_mutex_unlock(&expired_list_lock);

	cJSON_AddNumberToObject(root,LIB_EXPRIED_NUM,client_num);
	
	return RET_SUC;
}
/*************************************************************************
  功能: 实现添加黑名单(单条生效)
  参数: mac用于将设备添加到黑名单
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
  修改历史      :
  1.日    期   : 2016年12月1日
    作    者   : luorilin
    修改内容   : 
 ************************************************************************/
RET_INFO cgi_lib_set_rub_net_add_blacklist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info)
{
	char *mac = NULL;
	int index = 0;
	char dev_name[PI_DEVNAME_STRING_LEN] = {0};  
	char *value = NULL; 
	char black_value[128] = {0};
	char *Remark = NULL;
	char *mac_hostname = NULL;
	char *maclist = NULL;
	char maclist_wl0[WL0_MACLIST_MAX] = {0};
	struct apclient_client_info* temp = NULL;
	struct ether_addr *hw_addr;

	mac = cgi_lib_get_var(wp,root, T(LIB_FILTER_MAC), T(""));

	if(0 == strcmp(mac,""))
	{
		printf("[%s][%d]input mac is NULL!!!\n", __FUNCTION__, __LINE__);
		sprintf(err_code, "%s", "1");
		return RET_ERR;
	}

	qosMacToLower(mac);
	
	for (index = 0; index < RUB_NET_BLACKLIST_MAX; ++index)
	{
		_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(index), value);
		
		if(strcmp(value, ""))
		{
			if(0 == strncmp(value,mac,17))
			{
				printf("[%s][%d]this rule already exist!!\n", __FUNCTION__, __LINE__);
				break;
			}
			continue;
		}
		memset(dev_name, '\0', PI_DEVNAME_STRING_LEN * sizeof(char));
	    //app 不需要考虑ap模式
		mac_hostname = tenda_arp_mac_to_hostname(mac); //主机名
		if(mac_hostname != NULL)
		{
#ifdef __CONFIG_SUPPORT_GB2312__
			if (1 == is_cn_encode(mac_hostname))
			{
				char host_name_utf_8[64] = {0};
				set_cn_ssid_encode("utf-8", mac_hostname, host_name_utf_8);
				strcpy(dev_name, host_name_utf_8);
			}
#else
			if(1 == is_gb2312_code(mac_hostname))
			{
				strcpy(dev_name, "Unknown");
			}
#endif
			else
		    {
		 		strcpy(dev_name, mac_hostname);
		    }
		}
		else
		{
			strcpy(dev_name, "Unknown");
		}
		Remark = get_remark(mac); //备注名
		if(NULL != Remark)
		{
			memset(dev_name, '\0', PI_DEVNAME_STRING_LEN * sizeof(char));
			strcpy(dev_name,Remark);
		}	
		hw_addr = ether_aton(mac);
		wl_add_acladdr(TENDA_WLAN5_AP_IFNAME,mac);  //5G无线黑名单添加生效
		wl_add_acladdr(TENDA_WLAN24_AP_IFNAME,mac);  //2.4G无线黑名单添加生效
		add_macfilter_rule(hw_addr->octet);  //有线添加生效
		
		//把黑名单写入到flash
		sprintf(black_value,"%s,0-6,0-0,on,%s",mac,dev_name);
	    _SET_VALUE(ADVANCE_MACFILTER_DENY_RULE(index),black_value);

		//更新wl_maclist表
		_GET_VALUE(ADVICE_MAC_LIST,maclist);

		if(0 == strcmp(maclist,""))
		{
			_SET_VALUE(ADVICE_MAC_LIST,mac);
		}
		else
		{
			memset(maclist_wl0,0x0,WL0_MACLIST_MAX);	
			strcpy(maclist_wl0,maclist);
			add_to_list(mac,maclist_wl0,WL0_MACLIST_MAX);
			_SET_VALUE(ADVICE_MAC_LIST,maclist_wl0);
		}	
		sprintf(err_code, "%s", "0");
		return RET_SUC;
	}

	if(RUB_NET_BLACKLIST_MAX == index)		//表示已经添加满
	{
		sprintf(err_code, "%s", "80");
		return RET_ERR;
	}
		
	
	sprintf(err_code, "%s", "0");
	return RET_ERR;
}
/*************************************************************************
  功能: 实现删除黑名单
  参数: mac用于将设备从黑名单中删除
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
RET_INFO cgi_lib_set_rub_net_delete_blacklist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info)
{	
	char *mac = NULL;
	int index = 0;
	char *value = NULL;
	int j = 0;
    char mac_temp[PI_MAC_STRING_LEN] = {0};
	int maclist_len = 0;
	char *maclist = NULL;
	char *maclist_wl0 = NULL;
	struct ether_addr *hw_addr;
	
    mac = cgi_lib_get_var(wp,root, T(LIB_FILTER_MAC), T(""));
	if(0 == strcmp(mac,""))
	{
		sprintf(err_code, "%s", "1");
		return RET_ERR;
	}
	
    qosMacToLower(mac);

	hw_addr = ether_aton(mac);

	/*传入6位格式的mac*/
	remove_macfilter_rule(hw_addr->octet);//移除

	/*传入的mac格式是没有:的*/
	biz_parse_fmt_mac_to_fmt1_mac(mac,mac_temp,PI_MAC_STRING_LEN); 
    RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "rm_acl_table", mac_temp, "");
	RunSystemCmd(0, "iwpriv", TENDA_WLAN5_AP_IFNAME, "rm_acl_table", mac_temp, "");

    //从flash中移除 并重新排序
    for (index = 0; index < RUB_NET_BLACKLIST_MAX; ++index)
	{
		_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(index), value);
	
		if (!strcmp(value, "") || strlen(value) < 8 )
			continue;

		if(0 == strncmp(value,mac,17))
		{
			for(j = index; j < RUB_NET_BLACKLIST_MAX; ++j)
			{
				_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(j + 1), value);
			    _SET_VALUE(ADVANCE_MACFILTER_DENY_RULE(j),value);
			}
			_SET_VALUE(ADVANCE_MACFILTER_DENY_RULE(j),"");

			/*更新wl_maclist表*/
			_GET_VALUE(ADVICE_MAC_LIST,maclist);
			/*申请内存保存访问控制的列表*/
			maclist_len = strlen(maclist);
			maclist_wl0 = (char*)malloc(maclist_len+1);
			if(!maclist_wl0)
			{
				printf("%s   malloc fail\n",__FUNCTION__);
				return RET_ERR;
			}
			memset(maclist_wl0,0x0,maclist_len+1);
			strncpy(maclist_wl0,maclist,maclist_len);
			/*移除该客户端*/
			remove_from_list(mac,maclist_wl0,WL0_MACLIST_MAX);
			
			_SET_VALUE(ADVICE_MAC_LIST,maclist_wl0);

			/*释放内存*/
			free(maclist_wl0);
			maclist_wl0 = NULL;
			maclist_len = 0;
			sprintf(err_code, "%s", "0");
			return RET_SUC;
		}
    }
	
	sprintf(err_code, "%s", "1");
	return RET_ERR;
}
/*************************************************************************
  功能: 实现关闭上线提醒
  参数: mac用于给设备添加信任，信任的设备上线就不提醒
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
RET_INFO cgi_lib_set_rub_net_add_to_trustlist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info)
{	
    int i = 0;
	char *value = NULL;
	char tmp_buf[20] = {0};
	char *mac = NULL;

	mac = cgi_lib_get_var(wp,root, T(LIB_FILTER_MAC), T(""));
    if(0 == strcmp(mac,""))
    {
		printf("[%s][%d]mac is error from APP!!!\n", __FUNCTION__, __LINE__);
		sprintf(err_code, "%s", "1");
        return RET_ERR;
    }

	for (i = 0; i < TR_RULE_NUMBER_MAX; i++)
	{   
		_GET_VALUE(_FW_TRUST_MAC(i), value);
        if(strncasecmp(value, mac, 17) == 0)
        {
            return RET_ERR;
        }
		if (strlen(value) == 17)
			continue;

		sprintf(tmp_buf, "%s", mac);
		_SET_VALUE(_FW_TRUST_MAC(i), tmp_buf);
      
        sprintf(err_code, "%s", "0");
		return RET_SUC;
	}
	
	sprintf(err_code, "%s", "1");
	return RET_ERR;
}


/*************************************************************************
  功能: 实现开启上线提醒
  参数: mac用于将设备从信任中删除，不信任的设备上线后就会提醒
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
RET_INFO cgi_lib_set_rub_net_delete_from_trustlist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info)
{
    int i = 0,j = 0;
	char *value = NULL;
	char *tmp = NULL;
	char *mac = NULL;
	
	mac = cgi_lib_get_var(wp,root, T(LIB_FILTER_MAC), T(""));
    if(0 == strcmp(mac,""))
    {
		printf("[%s][%d]mac is error from APP!!!\n", __FUNCTION__, __LINE__);
		sprintf(err_code, "%s", "1");
        return RET_ERR;
    }

	for (i = 0; i < TR_RULE_NUMBER_MAX; ++i)
	{
		_GET_VALUE(_FW_TRUST_MAC(i), value);
        if(0 == strcmp(value,""))
        {
            break;
        }
		if((NULL != value) && (strncasecmp(value,mac, 17) == 0))
		{
            for(j = i; j < TR_RULE_NUMBER_MAX - 1; j++)
            {
                _GET_VALUE(_FW_TRUST_MAC(j + 1), tmp);
                if(0 == strcmp(tmp,""))
                {
                    break;
                }
                 _SET_VALUE(_FW_TRUST_MAC(j), tmp);
            }
            _SET_VALUE(_FW_TRUST_MAC(j), "");
			sprintf(err_code, "%s", "0");
            return RET_SUC;        
		}
      }
    sprintf(err_code, "%s", "1");   
	return RET_ERR;
}


/*************************************************************************
  功能: 实现清空所有的黑名单
  参数: 无
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
RET_INFO cgi_lib_set_flush_blacklist(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info)
{
	int index = 0;
	char mac[PI_MAC_STRING_LEN] = {0};
	char mac_temp[PI_MAC_STRING_LEN] = {0};
	char *value = NULL;
	struct ether_addr *hw_addr;

	for(index = 0; index < RUB_NET_BLACKLIST_MAX;index++)
	{
		_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(index), value); 
		if (!strcmp(value, "") || strlen(value) < 17 )
			continue;
			
		_SET_VALUE(ADVANCE_MACFILTER_DENY_RULE(index),"");

		sscanf(value,"%17s",mac);	
		hw_addr = ether_aton(mac);
		
		remove_macfilter_rule(hw_addr->octet);//移除

		biz_parse_fmt_mac_to_fmt1_mac(mac,mac_temp,PI_MAC_STRING_LEN); 
    	RunSystemCmd(0, "iwpriv", TENDA_WLAN24_AP_IFNAME, "rm_acl_table", mac_temp, "");
		RunSystemCmd(0, "iwpriv", TENDA_WLAN5_AP_IFNAME, "rm_acl_table", mac_temp, "");
	}
	_SET_VALUE(ADVICE_MAC_LIST,"");	//更新wl0_maclist表

	
	sprintf(err_code, "%s", "0");
	return RET_SUC;
}


