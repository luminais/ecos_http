#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include <net/if.h>
#include "wlioctl.h"
#include <wlutils.h>
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
#include "sys_timer.h"
#endif
#include "sys_module.h"
#include "http.h"
#include "apclient_dhcpc.h"
#include "debug.h"

#define APCLIENT_DHCPC_SIZE		8*1024
static cyg_handle_t apclient_dhcpc_daemon_handle;
static PI8 apclient_dhcpc_daemon_stack[APCLIENT_DHCPC_SIZE];
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
struct apclient_client_info* client_info_list;
static void tpi_apclient_dhcpc_timer_init();
#endif
static cyg_thread apclient_dhcpc_daemon_thread;

#ifdef __CONFIG_A9__
static APCLIENT_DHCPC_WEB_WAIT_STRUCT apclient_dhcpc_web_wait;
#endif

static APCLIENT_DHCPC_INFO_STRUCT apclient_dhcpc_info;
static RET_INFO tpi_apclient_dhcpc_handle(MODULE_COMMON_OP_TYPE action);
static RET_INFO tpi_apclient_dhcpc_main();
static RET_INFO tpi_apclient_dhcpc_start();
static RET_INFO tpi_apclient_dhcpc_stop();
static RET_INFO tpi_apclient_dhcpc_restart();

/*以下函数用于api调用*/
RET_INFO tpi_apclient_dhcpc_update_info()
{
	/* 桥模式下的AP模式和ap+client模式router_disable为1，都需要开启apclient_dhcpc */
	if(nvram_match(SYSCONFIG_WORKMODE, "client+ap") 
		|| nvram_match(SYSCONFIG_WORKMODE, "bridge"))
	{
		apclient_dhcpc_info.enable = 1;
	}
	else
	{
		apclient_dhcpc_info.enable = 0;
	}
#ifdef __CONFIG_BRIDGE_AP__
	/* 通过桥的方式实现桥AP模式设置dhcpc接口为wan口
	但是从上级获取到ip后设置到lan口 */
	if(nvram_match(SYSCONFIG_WORKMODE,"bridge"))
	{
		strncpy(apclient_dhcpc_info.lan_ifnames,nvram_safe_get("wan0_ifname"),PI_IP_STRING_LEN-1);
	}
	else
#endif//__CONFIG_BRIDGE_AP__
	{
		strncpy(apclient_dhcpc_info.lan_ifnames,nvram_safe_get("lan_ifname"),PI_IP_STRING_LEN-1);
	}
	strcpy__(apclient_dhcpc_info.ipaddr,"");
	strcpy__(apclient_dhcpc_info.mask,"");
	strcpy__(apclient_dhcpc_info.gateway,"");
#ifdef __CONFIG_A9__
	apclient_dhcpc_info.ping_enable = 0;
#endif

	apclient_dhcpc_info.now_status = WAN_DISCONNECTED;
	apclient_dhcpc_info.before_status = WAN_DISCONNECTED;

	apclient_dhcpc_info.connected_count = 0;
	apclient_dhcpc_info.disconnected_count = 0;

	return RET_SUC;
}

RET_INFO tpi_apclient_dhcpc_struct_init()
{
	memset(&apclient_dhcpc_info,0x0,sizeof(apclient_dhcpc_info));
	return tpi_apclient_dhcpc_update_info();
}

RET_INFO tpi_apclient_dhcpc_first_init()
{
	if(apclient_dhcpc_info.enable)
		tpi_apclient_dhcpc_start();
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
	tpi_apclient_dhcpc_timer_init();
#endif
	return RET_SUC;
}
/*添加线程退出接口函数lq*/
static int check_apclient_dhcpc_exit_flag = 0;
void set_apclient_dhcpc_exit_flag(int flag)
{
	check_apclient_dhcpc_exit_flag = flag;
}

int get_apclient_dhcpc_exit_flag()
{
	return set_apclient_dhcpc_exit_flag;
}

RET_INFO tpi_apclient_dhcpc_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_apclient_dhcpc_start();
			break;
		case OP_STOP:
			tpi_apclient_dhcpc_stop();
			break;
		case OP_RESTART:
			tpi_apclient_dhcpc_handle(OP_STOP);
			tpi_apclient_dhcpc_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}

	if(0 == strcmp(var->string_info,"restart_dhcpc_only"))
	{
		if((apclient_dhcpc_info.before_status == WAN_CONNECTED && apclient_dhcpc_info.connected_count >= CONNECTED_COUNT_MAX)
		   || (apclient_dhcpc_info.before_status == WAN_DISCONNECTED && apclient_dhcpc_info.disconnected_count < DISCONNECTED_COUNT_MAX))
		{
			/*需要开启DHCP服务器，关闭dhcpc_eth0*/
			dhcpc_stop(apclient_dhcpc_info.lan_ifnames);
			PI_PRINTF(TPI,"restart_dhcpc_only dhcpc stoped.\n");
#if 0
			tpi_apclient_dhcpc_lan_dhcp_action_handle(OP_START);
#else
			nvram_set("lan_proto", "dhcp");
			msg_send(MODULE_RC,RC_DHCP_SERVER_MODULE,"op=3");
#endif
		}
#ifdef __CONFIG_AUTO_CONN_CLIENT__
		if(extend_is_done_status())
			extend_set_undo_status();
#endif
		apclient_dhcpc_info.before_status = WAN_DISCONNECTED;
		apclient_dhcpc_info.connected_count = 0;
		apclient_dhcpc_info.disconnected_count = 0;
		strcpy__(apclient_dhcpc_info.ipaddr,"");
		strcpy__(apclient_dhcpc_info.mask,"");
		strcpy__(apclient_dhcpc_info.gateway,"");
#ifdef __CONFIG_A9__
		apclient_dhcpc_info.ping_enable = 0;
#endif
	}

	return RET_SUC;
}

/*以下用于gpi获取信息函数*/
PI8 tpi_apclient_dhcpc_get_enable()
{
	return apclient_dhcpc_info.enable;
}

RET_INFO tpi_apclient_dhcpc_get_ip(PI8 *ip,PI8 *mask)
{
	RET_INFO ret = RET_SUC;

	if(NULL == ip || NULL == mask)
		return RET_ERR;

	strcpy__(ip,apclient_dhcpc_info.ipaddr);
	strcpy__(mask,apclient_dhcpc_info.mask);

	return ret;
}

/*****************************************************************************
 函 数 名  : tpi_apclient_dhcpc_ping_gateway
 功能描述  : ping网关操作
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 能ping通RET_SUC，否则RET_ERR
 
 修改历史      :
  1.日    期   : 2016年12月1日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO tpi_apclient_dhcpc_ping_gateway()
{
	RET_INFO ret = RET_ERR;
	PIU32 dst = 0;
	if(0 != strcmp(apclient_dhcpc_info.gateway,"") && 0 != strcmp(apclient_dhcpc_info.gateway,"0.0.0.0") && strlen(apclient_dhcpc_info.gateway) >= 7)
	{
		dst = inet_addr(apclient_dhcpc_info.gateway);
		dst = ntohl(dst);
	
		/*进行PING网关操作，最多等待3秒钟*/
		if(0 == icmp_ping(dst, 0, 3000))
		{
			ret = RET_SUC;
		}
	}
	RC_MODULE_DEBUG(RC_APCLIENT_DHCPC_MODULE,TPI,"ping gateway %s %s\n",apclient_dhcpc_info.gateway, ret?"ok":"timeout");
	return ret;
}


/*以下为该模块具体执行实现函数*/

/*在apclient_dhcpc等其他线程可能会用到*/
#ifdef __CONFIG_A9__
void tpi_apclient_dhcpc_set_web_wait(PIU8 set_wait,PIU8 get_return)
{
	if(0 == set_wait || 1 == set_wait)
	{
		apclient_dhcpc_web_wait.apclient_dhcpc_web_set_wait = set_wait;
	}
	if(0 == get_return || 1 == get_return)
	{
		apclient_dhcpc_web_wait.apclient_dhcpc_web_get_return = get_return;
	}
}

PIU8 tpi_apclient_dhcpc_get_web_wait_tag()
{
	return apclient_dhcpc_web_wait.apclient_dhcpc_web_set_wait;
}

PIU8 tpi_apclient_dhcpc_get_web_get_return_tag()
{
	return apclient_dhcpc_web_wait.apclient_dhcpc_web_get_return;
}
#endif

#ifdef __CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__
#define DHCPD_IP_POOL_MAX_NUM 50
static RET_INFO tpi_apclient_dhcpc_set_dhcpd_pool(PIU32 wan_ip,PIU32 wan_mask)
{
	PI8 lan_mask_ip[PI_IP_STRING_LEN] = {0};
	PIU32 lan_mask = 0,max_mask = 0;
	PIU32 dhcp_start_change = 0,dhcp_end_change = 0;
	struct in_addr  modify_dhcp_start_ip,modify_dhcp_end_ip;

	strcpy(lan_mask_ip, nvram_safe_get("lan_netmask"));

	lan_mask = inet_addr(lan_mask_ip);

	max_mask = (lan_mask > wan_mask)?lan_mask:wan_mask;

	dhcp_start_change = (wan_ip&max_mask) + 1;

	if((~(max_mask)) >= DHCPD_IP_POOL_MAX_NUM)
	{
		dhcp_end_change = (wan_ip&max_mask) + DHCPD_IP_POOL_MAX_NUM;
	}
	else
	{
		dhcp_end_change = (wan_ip&max_mask) + (~(max_mask)) - 1;
	}

	modify_dhcp_start_ip.s_addr= dhcp_start_change;
	modify_dhcp_end_ip.s_addr= dhcp_end_change;

	nvram_set("dhcp_start",inet_ntoa(modify_dhcp_start_ip));
	nvram_set("dhcp_end",inet_ntoa(modify_dhcp_end_ip));

	return RET_SUC;
}

RET_INFO tpi_apclient_dhcpc_set_dhcpd(PI8 *ip,PI8 *mask)
{
	PIU8 commit_tag = 0;

	if(NULL == ip || NULL == mask)
		return RET_ERR;

	if(0 == strcmp(ip,"") || 0 == strcmp(mask,""))
		return RET_ERR;

	PIU32 wan_ip = 0,wan_mask = 0;
	PIU32 lan_ip = 0,lan_mask = 0;
	wan_ip = inet_addr(ip);
	wan_mask = inet_addr(mask);
	lan_ip = inet_addr(nvram_safe_get("lan_ipaddr"));
	lan_mask = inet_addr(nvram_safe_get("lan_netmask"));

	//跟原来 ip&mac 不是在同个 网段,需修改dhcp配置
	if(wan_ip != lan_ip || wan_mask != lan_mask)
	{
		/*修改DHCP地址池配置*/
		tpi_apclient_dhcpc_set_dhcpd_pool(wan_ip,wan_mask);

		/*修改TC/家长控制/DMZ/等配置*/
		/*这里不太完整，结果可能不能达到预期的效果，N318里面需要进行详细测试*/
		modify_filter_virtual_server(ip);

		/*这里修改LAN配置*/
		nvram_set("lan_ipaddr", ip);
		nvram_set("lan_netmask", mask);
		nvram_set("lan_dns", ip);
		nvram_set("lan_gateway", ip);

		nvram_commit();
	}

	return RET_SUC;
}
#endif

RET_INFO tpi_apclient_dhcpc_set_ip(PI8 *ip,PI8 *mask,PI8 *gateway)
{
	RET_INFO ret = RET_SUC;

	if(NULL == ip || NULL == mask)
		return RET_ERR;

	PI_PRINTF(TPI,"get ip %s/%s\n",ip,mask);

	strcpy__(apclient_dhcpc_info.ipaddr,ip);
	strcpy__(apclient_dhcpc_info.mask,mask);

	if(NULL != gateway)
	{
		strcpy__(apclient_dhcpc_info.gateway, gateway);
	}

	return ret;
}
unsigned long long  uplink_time = 0;
RET_INFO tpi_apclient_dhcpc_lan_dhcp_action_handle(MODULE_COMMON_OP_TYPE action)
{
	PI8 par_str[PI_BUFLEN_32] = {0};
	switch(action)
	{
		case OP_STOP:
			/*断开无线客户端，让无线客户端重连，因为考虑到是为了DHCP服务器服务的，所以移动到DHCP的动作这里*/
			
			nvram_set("lan_proto", "static");
			sprintf(par_str,"op=%d,string_info=lan_link",OP_STOP);
			msg_send(MODULE_RC,RC_DHCP_SERVER_MODULE,par_str);
			tpi_http_clear_login_time();
			uplink_time = cyg_current_time();
#ifdef __CONFIG_CE_POWER__
			nvram_set("wan0_connect","Connected");
#endif
			break;
		case OP_START:
			nvram_set("lan_proto", "dhcp");

			sprintf(par_str,"op=%d,string_info=lan_link",OP_START);
			msg_send(MODULE_RC,RC_DHCP_SERVER_MODULE,par_str);
			tpi_http_clear_login_time();
			break;
		case OP_RESTART:
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",action);
			break;
	}
	return RET_SUC;
}
unsigned int get_olnline_time()
{
	unsigned int online_time = 0;
	online_time = (int)(cyg_current_time() - uplink_time) / 100;
	return online_time;
}
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
/*lq 根据mac获取无线客户端的信息*/
struct apclient_client_info* tpi_apclient_dhcpc_get_client_info(unsigned char* mac)
{
	struct apclient_client_info *client_temp = client_info_list;

	if(NULL == mac)
		return NULL;
	while(client_temp)
	{
		if(!memcmp(client_temp->mac,mac,ETHER_ADDR_LEN))
			return client_temp;
		client_temp = client_temp->next;
	}
	return NULL;
}
/*lq 清空所有无线客户端的信息*/
RET_INFO tpi_apclient_dhcpc_flush_client_info()
{
	struct apclient_client_info *client_temp = client_info_list;
	struct apclient_client_info *temp 		 = client_temp;
	while(client_temp)
	{
		temp = client_temp;
		client_temp = temp->next;
		free(temp);
		temp = NULL;
	}
	client_info_list = NULL;
	return RET_SUC;
}

/*lq 更新所有无线客户端的信息*/
RET_INFO tpi_apclient_dhcpc_update_client_info()
{
	struct maclist *mac_list = NULL;
	struct apclient_client_info* client_temp = NULL;
	struct apclient_client_info* client_head = NULL;
	struct apclient_client_info* temp 		 = NULL;
	char dev_mac[13] = {0};
	int mac_list_size = 0;
	int i =0,j=0;
	char *ifname[] ={TENDA_WLAN24_AP_IFNAME,
					TENDA_WLAN5_AP_IFNAME,
					TENDA_WLAN24_GUEST_IFNAME,
					TENDA_WLAN5_GUEST_IFNAME,
					""};

	mac_list_size = sizeof(uint) + MAX_STA_NUM * sizeof(struct ether_addr);
	mac_list = (struct maclist *)malloc(mac_list_size);

	for(j = 0;strcmp(ifname[j],"");j++)
	{
		memset(mac_list,0x0,mac_list_size);
		if (getwlmaclist(ifname[j], mac_list))
		{
			free(mac_list);
			return RET_ERR;
		}

		for (i = 0; i < mac_list->count; ++i)
		{

			if(temp = tpi_apclient_dhcpc_get_client_info(mac_list->ea[i].octet))
			{
				client_temp = (struct apclient_client_info*)malloc(sizeof(struct apclient_client_info));
				if(NULL == client_temp)
					continue;
				memset(client_temp,0x0,sizeof(struct apclient_client_info));
				strncpy(client_temp->dev_name,temp->dev_name,strlen(temp->dev_name));
				memcpy(client_temp->mac,temp->mac,ETHER_ADDR_LEN);

				(client_temp->client_ip).s_addr = (temp->client_ip).s_addr;
				client_temp->next = client_head;
				client_head = client_temp;
			}

		}
	}
	tpi_apclient_dhcpc_flush_client_info();
	client_info_list = client_head;

	free(mac_list);
	return RET_SUC;
}
/*lq 添加一个无线客户端的信息*/
RET_INFO tpi_apclient_dhcpc_add_client(struct apclient_client_info client)
{
	struct apclient_client_info* client_temp = NULL;

	if(client_temp =tpi_apclient_dhcpc_get_client_info(client.mac))
	{
		memset(client_temp->dev_name,0x0,255);
		strncpy(client_temp->dev_name,client.dev_name,strlen(client.dev_name));
		(client_temp->client_ip).s_addr = client.client_ip.s_addr;
		return RET_SUC;
	}

	client_temp = (struct apclient_client_info*)malloc(sizeof(struct apclient_client_info));
	if(NULL == client_temp)
		return RET_ERR;
	memset(client_temp,0x0,sizeof(struct apclient_client_info));
	strncpy(client_temp->dev_name,client.dev_name,strlen(client.dev_name));
	memcpy(client_temp->mac,client.mac,ETHER_ADDR_LEN);

	(client_temp->client_ip).s_addr = client.client_ip.s_addr;
	client_temp->next = client_info_list;
	client_info_list = client_temp;
	return RET_SUC;
}

/*lq 移除一个无线客户端的信息*/
RET_INFO tpi_apclient_dhcpc_remove_client(unsigned char* mac)
{
	struct apclient_client_info *client_temp = client_info_list;
	struct apclient_client_info *client_pro 	 = client_info_list;

	if(NULL == mac)
		return RET_ERR;

	if(!memcmp(client_temp->mac,mac,ETHER_ADDR_LEN))
	{
		client_info_list = client_temp->next;
		free(client_temp);
		client_temp = NULL;
		return RET_SUC;
	}

	client_temp = client_temp->next;
	while(client_temp)
	{
		if(!memcmp(client_temp->mac,mac,ETHER_ADDR_LEN))
		{
			client_pro->next = client_temp->next;
			free(client_temp);
			client_temp = NULL;
			return RET_SUC;
		}
		client_pro = client_temp;
		client_temp = client_temp->next;
	}
	return 1;
}
/*lq 添加定时器，五分钟更新一会用户链表*/
static void tpi_apclient_dhcpc_timer_init()
{
    DO_TIMER_FUN timer;
    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,APCLIENT_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME * 5;
    timer.fun = tpi_apclient_dhcpc_update_client_info;
    sys_do_timer_add(&timer);

    return;
}
#endif
/*只有本文件里面用*/
static RET_INFO tpi_apclient_dhcpc_handle(MODULE_COMMON_OP_TYPE action)
{
	switch(action)
	{
		case OP_STOP:
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
#ifndef __CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__
			/*lq 桥接上级或者与上级断开，清空无线客户端信息链表*/
			/*如果定义了__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__表示更新DHCP配置，那么这里就不需要清空列表*/
			tpi_apclient_dhcpc_flush_client_info();
#endif
#endif
			dhcpc_stop(apclient_dhcpc_info.lan_ifnames);
#ifndef __CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__
			/*如果定义了__CONFIG_APCLIENT_DHCPC_CHANGE_DHCPD__表示更新DHCP配置，那么这里就不需要将IP变回去*/
			if(nvram_match(SYSCONFIG_WORKMODE,"client+ap"))
			{
				tenda_ifconfig(apclient_dhcpc_info.lan_ifnames, IFUP,nvram_safe_get("lan_ipaddr"),nvram_safe_get("lan_netmask"));
			}
		#ifdef __CONFIG_BRIDGE_AP__
			else if(nvram_match(SYSCONFIG_WORKMODE,"bridge"))
			{
				tenda_ifconfig(nvram_safe_get("lan_ifname"), IFUP,nvram_safe_get("lan_ipaddr"),nvram_safe_get("lan_netmask"));
			}
		#endif
#endif
			tpi_apclient_dhcpc_lan_dhcp_action_handle(OP_START);
#ifdef __CONFIG_A9__
			apclient_dhcpc_info.ping_enable = 0;
#endif
			break;
		case OP_START:
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
			/*lq 桥接上级或者与上级断开，清空无线客户端信息链表*/
			tpi_apclient_dhcpc_flush_client_info();
#endif
			nvram_unset("resolv_conf");
			dhcpc_start(apclient_dhcpc_info.lan_ifnames, "landhcpc", "");
#ifdef __CONFIG_A9__
			/*由于桥接模式下上级修改SSID或者密码并且本级没有无线客户端连接的时候，本级不会检视出的问题，只有存在数据交互的情况下才能检视出来，那么增加PING机制来进行数据交互*/
			apclient_dhcpc_info.ping_enable = 1;
#endif
			break;
		case OP_RESTART:
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",action);
			break;
	}

	return RET_SUC;
}

/*只有本文件里面用*/
static RET_INFO tpi_apclient_dhcpc_main()
{
	int first_time = 1,dhcp_start = 1,wifiStat = 0;
#ifdef __CONFIG_A9__
	extern int getWlStaNum( char *interface, int *num );
	PIU32 dst = 0,num = 0,max_num = APCLIENT_DHCPC_PING_HAS_CLINET_NUM,client_num = 0;
#endif

#ifdef __CONFIG_A9__
	if(nvram_match("product_finish", "0"))
	{
		tpi_apclient_dhcpc_lan_dhcp_action_handle(OP_START);
		cyg_thread_delay(APCLINET_DHCPC_SLEEP);
		return RET_SUC;
	}
#endif

	while(apclient_dhcpc_info.enable)
	{
		//判断线程推出标志是否为请求退出lq
		if(1 == get_apclient_dhcpc_exit_flag())
		{
			break;
		}
#ifdef __CONFIG_A9__
		if(apclient_dhcpc_info.ping_enable)
		{
			if(0 != strcmp(apclient_dhcpc_info.gateway,"") && 0 != strcmp(apclient_dhcpc_info.gateway,"0.0.0.0") && strlen(apclient_dhcpc_info.gateway) >= 7 && num == 0)
			{
				dst = inet_addr(apclient_dhcpc_info.gateway);
				dst = ntohl(dst);
			
				/*进行PING网关操作，最多等待APCLIENT_DHCPC_PING_TIME秒钟*/
				icmp_ping(dst, 0, APCLIENT_DHCPC_PING_TIME);
				
				if(0 == getWlStaNum("wlan0",&client_num))
				{
					max_num = APCLIENT_DHCPC_PING_NO_CLINET_NUM;
				}
				else
				{
					max_num = APCLIENT_DHCPC_PING_HAS_CLINET_NUM;
				}
			}

			if((num++) >= max_num)
			{
				num = 0;
			}
		}
#endif
#if defined(__CONFIG_EXTEND_LED__) && (defined(__CONFIG_AUTO_CONN_CLIENT__) || defined(__CONFIG_WPS_RTK__))
		RC_MODULE_DEBUG(RC_APCLIENT_DHCPC_MODULE,TPI,"extend_led_blink:%d,extend_is_doing_status:%d,extend_is_setdef_status:%d\n",extend_led_blink(),extend_is_doing_status(),extend_is_setdef_status());
		if(extend_led_blink() && (extend_is_doing_status() || extend_is_setdef_status()))
		{
			cyg_thread_delay(APCLINET_DHCPC_SLEEP);
			continue;
		}

		extern PIU8 tpi_auto_conn_client_get_restarting_tag();
		/*自动桥接最后一步如果正在重启无线，此时需要等待无线重启完成之后才能从上级获取IP*/
		if(1 == tpi_auto_conn_client_get_restarting_tag() && extend_is_done_status())
		{
			cyg_thread_delay(50);
			continue;
		}
#endif

		gpi_wifi_get_status_info(&(apclient_dhcpc_info.now_status),&wifiStat);

		if(WIFI_OK == wifiStat && WAN_CONNECTING == apclient_dhcpc_info.now_status)
		{
			apclient_dhcpc_info.now_status = WAN_CONNECTED;
		}

		RC_MODULE_DEBUG(RC_APCLIENT_DHCPC_MODULE,TPI,"apclient_dhcpc_info.now_status:%d\n",apclient_dhcpc_info.now_status);
		switch(apclient_dhcpc_info.now_status)
		{
			case WAN_CONNECTED:
				apclient_dhcpc_info.connected_count++;
				apclient_dhcpc_info.disconnected_count = 0;
				RC_MODULE_DEBUG(RC_APCLIENT_DHCPC_MODULE,TPI,"apclient_dhcpc_info.connected_count:%d\n",apclient_dhcpc_info.connected_count);

				if(1 == first_time)
				{
				#ifdef __CONFIG_AUTO_CONN_CLIENT__
					extend_set_done_status();
				#endif

					apclient_dhcpc_info.before_status = WAN_CONNECTED;
					tpi_apclient_dhcpc_handle(OP_START);
					first_time = 0;
				}
				else
				{
				#ifdef __CONFIG_AUTO_CONN_CLIENT__
					if(WAN_DISCONNECTED == (apclient_dhcpc_info.before_status) && (apclient_dhcpc_info.connected_count) >= CONNECTED_COUNT_MIN)
					{
						extend_set_done_status();
					}
				#endif

					if(WAN_DISCONNECTED == (apclient_dhcpc_info.before_status) && (apclient_dhcpc_info.connected_count) >= CONNECTED_COUNT_MAX)
					{
						apclient_dhcpc_info.before_status = WAN_CONNECTED;
						tpi_apclient_dhcpc_handle(OP_START);
					}
				}
				dhcp_start = 0;
				break;
			default:
				apclient_dhcpc_info.connected_count = 0;
				apclient_dhcpc_info.disconnected_count++;
				RC_MODULE_DEBUG(RC_APCLIENT_DHCPC_MODULE,TPI,"apclient_dhcpc_info.disconnected_count:%d\n",apclient_dhcpc_info.disconnected_count);

				#ifdef __CONFIG_AUTO_CONN_CLIENT__
				if(0 != strcmp(nvram_safe_get("manual_set"), "0") && (apclient_dhcpc_info.disconnected_count) >= (DISCONNECTED_COUNT_MAX * 4))
				{
					nvram_set("manual_set", "0");//桥接失败，可以启动自动桥接
				}
				#endif
			#ifdef __CONFIG_A9__
				if(1 == dhcp_start && (apclient_dhcpc_info.disconnected_count) >= DISCONNECTED_COUNT_MAX)
			#else
				if(1 == dhcp_start)
			#endif
				{
				#ifdef __CONFIG_AUTO_CONN_CLIENT__
					if(extend_is_done_status())
						extend_set_undo_status();
				#endif

					tpi_apclient_dhcpc_lan_dhcp_action_handle(OP_START);
					dhcp_start = 0;
				}
				if(WAN_CONNECTED == (apclient_dhcpc_info.before_status) && (apclient_dhcpc_info.disconnected_count) >= DISCONNECTED_COUNT_MAX)
				{
				#ifdef __CONFIG_AUTO_CONN_CLIENT__
					if(extend_is_done_status())
						extend_set_undo_status();
				#endif

					apclient_dhcpc_info.before_status = WAN_DISCONNECTED;
					strcpy__(apclient_dhcpc_info.ipaddr,"");
					strcpy__(apclient_dhcpc_info.mask,"");
					strcpy__(apclient_dhcpc_info.gateway,"");
#ifdef __CONFIG_A9__					
					apclient_dhcpc_info.ping_enable = 0;
#endif
					tpi_apclient_dhcpc_handle(OP_STOP);
				}
				break;
		}
		cyg_thread_delay(APCLINET_DHCPC_SLEEP);
	}

	return RET_SUC;
}

static RET_INFO tpi_apclient_dhcpc_start()
{
	RET_INFO ret = RET_SUC;

	if(apclient_dhcpc_daemon_handle == 0)
	{
		tpi_apclient_dhcpc_update_info();
		if(apclient_dhcpc_info.enable)
		{
			cyg_thread_create(
				8,
				(cyg_thread_entry_t *)tpi_apclient_dhcpc_main,
				0,
				"apclient_dhcpc",
				apclient_dhcpc_daemon_stack,
				sizeof(apclient_dhcpc_daemon_stack),
				&apclient_dhcpc_daemon_handle,
				&apclient_dhcpc_daemon_thread);

			cyg_thread_resume(apclient_dhcpc_daemon_handle);
			cyg_thread_delay(1);
			PI_PRINTF(TPI,"start success!\n");
		}
		else
		{
			PI_ERROR(TPI,"the mib is off, connot start!\n");
		}
	}
	else
	{
		PI_PRINTF(TPI,"is already start!\n");
	}

	return ret;
}

static RET_INFO tpi_apclient_dhcpc_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;

	if(apclient_dhcpc_daemon_handle != 0)
	{
		//设置退出标志为退出lq
		set_apclient_dhcpc_exit_flag(1);

		/* Wait until thread exit */
		pid = oslib_getpidbyname("apclient_dhcpc");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}

		cyg_thread_delete(apclient_dhcpc_daemon_handle);

		PI_PRINTF(TPI,"stop success!\n");
		//设置退出标志为退出lq
		set_apclient_dhcpc_exit_flag(0);
		apclient_dhcpc_daemon_handle = 0;
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");
	}

	return ret;
}

static RET_INFO tpi_apclient_dhcpc_restart()
{
	RET_INFO ret = RET_SUC;

	if(RET_ERR == tpi_apclient_dhcpc_stop() || RET_ERR == tpi_apclient_dhcpc_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}

	return ret;
}
