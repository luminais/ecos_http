#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include <net/if.h>
#include "wlioctl.h"
#include <wlutils.h>

#include "rc.h"
#include "debug.h"
#include "dhcp_server.h"
#include <bcmtc.h>

static DHCP_SERVER_INFO_STRUCT dhcp_server_info;
extern int delete_all_online_client(char* ifname);
static RET_INFO tpi_dhcp_server_restart_phy();
static RET_INFO tpi_dhcp_server_start();
static RET_INFO tpi_dhcp_server_stop();
static RET_INFO tpi_dhcp_server_restart();
P_STATIC_IP_LIST tpi_dhcp_server_static_ip_get_info();
RET_INFO tpi_dhcp_server_static_ip_update_info();

extern int et_restart_lan_port(int lan_port);

/*以下函数用于api调用*/
RET_INFO tpi_dhcp_server_update_info()
{
	if(0 == strcmp("dhcp",nvram_safe_get("lan_proto")))
		dhcp_server_info.enable = 1;
	else
		dhcp_server_info.enable = 0;		

	dhcp_server_info.lease_time = atoi(nvram_safe_get("lan_lease"));

	dhcp_server_info.wl_mode = gpi_wifi_get_mode();
		
	strcpy__(dhcp_server_info.ifname,nvram_safe_get("lan_ifname"));
	strcpy__(dhcp_server_info.lan_ip,nvram_safe_get("lan_ipaddr"));
	strcpy__(dhcp_server_info.lan_mask,nvram_safe_get("lan_netmask"));
	strcpy__(dhcp_server_info.start_ip,nvram_safe_get("dhcp_start"));
	strcpy__(dhcp_server_info.end_ip,nvram_safe_get("dhcp_end"));
	strcpy__(dhcp_server_info.gateway,nvram_safe_get("lan_gateway"));
	strcpy__(dhcp_server_info.dns,nvram_safe_get("lan_dns"));
	
	return RET_SUC;
}

RET_INFO string_reserve(char* reserver)
{
	int start_local = 0;
	int end_local = 0;
	char temp = 0;
	if(reserver == NULL)
		return RET_ERR;
	end_local = strlen(reserver) - 1;
	while(start_local < end_local)
	{
		temp = reserver[start_local];
		reserver[start_local] = reserver[end_local];
		reserver[end_local] = temp;
		start_local++;
		end_local--;
	}
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : tpi_dhcp_server_static_ip_get_item_info
 功能描述  : 解析dhcp_static
 输入参数  : char* static_ip_value  
             char* remark           
             char* ip_addr          
             char* mac_addr         
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年10月31日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO tpi_dhcp_server_static_ip_get_item_info(char* static_ip_value,char* remark,char* ip_addr,char* mac_addr)
{
	int i = 0;
	char* temp = NULL;
	char cur_time[12] = {0};
	char cur_enable[2] = {0};
	char cur_remark[PI_REMARK_STRING_LEN] = {0};
	char cur_ip_addr[PI_IP_STRING_LEN] = {0};
	char cur_mac_addr[PI_MAC_STRING_LEN] = {0};
	int str_len = 0;

	if(static_ip_value == NULL || remark == NULL || ip_addr == NULL ||mac_addr == NULL)
		return RET_ERR; 
	
	str_len = strlen(static_ip_value);
	temp = (char*)malloc(str_len+1);

	if(temp == NULL)
		return RET_ERR;
	
	while(i < str_len)
	{
		temp[str_len - i - 1] = static_ip_value[i];
		i++;
	}

	sscanf(temp,"%[^;];%[^;];%[^;];%[^;];%s",cur_time,cur_enable,cur_mac_addr,cur_ip_addr,cur_remark);
	//根据converity分析结果修改   2017/1/10 F9项目
	if(string_reserve(cur_mac_addr) == RET_ERR)
	{
		free(temp);
		return RET_ERR;
	}
	if(strcpy(mac_addr,cur_mac_addr) == NULL)
	{
		free(temp);
		return RET_ERR;
	}
	
	if(string_reserve(cur_ip_addr) == RET_ERR)
	{
		free(temp);
		return RET_ERR;
	}
	if(strcpy(ip_addr,cur_ip_addr) == NULL)
	{
		free(temp);
		return RET_ERR;
	}
	
	if(string_reserve(cur_remark) == RET_ERR)
	{
		free(temp);
		return RET_ERR;
	}
	if(strcpy(remark,cur_remark) == NULL)
	{
		free(temp);
		return RET_ERR;
	}

	free(temp);
	return RET_SUC;
	
}

RET_INFO tpi_dhcp_server_static_ip_update_info()
{
	PIU32 i;
	PI8 *static_ip_value;
	P_STATIC_IP_LIST static_ip_bind_node = NULL, static_ip_bind_node_tmp = NULL;

	/* free the list if it aready exists */
	while(static_ip_bind_node_tmp = dhcp_server_info.static_ip_bind_info)
	{
		dhcp_server_info.static_ip_bind_info = static_ip_bind_node_tmp->next;
		free(static_ip_bind_node_tmp);
	}
	
	for(i = 0; i <= DHCPD_STATIC_LEASE_NU; ++i)
	{
		static_ip_value = nvram_safe_get(ADVANCE_STATIC_IP_MAPPING_RULE(i));
		if(strcmp(static_ip_value, ""))
		{
			static_ip_bind_node = (P_STATIC_IP_LIST)malloc(sizeof(STATIC_IP_LIST));
			if(!static_ip_bind_node)
			{
				printf("func=%s; line=%d; msg: No buf!\n", __func__, __LINE__);
                /* free the list, created before this */
				goto error_out;
			}
			
			/* get value */
			memset(static_ip_bind_node, 0x0, sizeof(STATIC_IP_LIST));
			/*逆序解析，原有的解析方法处理不了;;;;;;;192.168.0.101;00:90:4c:88:88:23;1;60*/
			PI8 *remark = NULL;
			PI8 *hostname = NULL;
			tpi_dhcp_server_static_ip_get_item_info(static_ip_value,static_ip_bind_node->remark,static_ip_bind_node->ip_addr,static_ip_bind_node->mac_addr);
			//备注名优先
			remark = get_remark(static_ip_bind_node->mac_addr);
			if (remark)
			{
				strcpy__(static_ip_bind_node->remark, remark);
			}
			 //备注名为空时,在线设备名优先 
			else if ( (static_ip_bind_node->remark[0] == 0) &&  (hostname = tenda_arp_mac_to_hostname(static_ip_bind_node->mac_addr) )) 
			{
				if (strcmp (hostname, "") != 0)
				{
					strcpy__(static_ip_bind_node->remark, hostname);	
				}
			}
			else //备注名为空时且设备不在线 或 设备在线设备名为空时
			{
				strcpy__(static_ip_bind_node->remark, "Unknown");
			}
		
			
			/* chain the list */
			static_ip_bind_node->next = dhcp_server_info.static_ip_bind_info;
			dhcp_server_info.static_ip_bind_info = static_ip_bind_node;
		}
		
	}
	
	return RET_SUC;

error_out:
	/* free the list if any node had get a buffer */
	while(static_ip_bind_node_tmp = dhcp_server_info.static_ip_bind_info)
	{
		dhcp_server_info.static_ip_bind_info = static_ip_bind_node_tmp->next;
		free(static_ip_bind_node_tmp);
	}
	
	return RET_ERR;
}

RET_INFO tpi_dhcp_server_struct_init()
{
	memset(&dhcp_server_info,0x0,sizeof(dhcp_server_info));
	return RET_SUC;
}

RET_INFO tpi_dhcp_server_first_init()
{
	return tpi_dhcp_server_start();
}

RET_INFO tpi_dhcp_server_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_dhcp_server_start();
			break;
		case OP_STOP:
			tpi_dhcp_server_stop();
			break;
		case OP_RESTART:
			tpi_dhcp_server_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}

	if(var->string_info && 0 == strcmp(var->string_info,"lan_link"))
	{
		tpi_dhcp_server_restart_phy();
	}
	
	return RET_SUC;
}

#ifdef __CONFIG_TC__
RET_INFO tpi_tc_dhcp_server_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
		case OP_STOP:
		case OP_RESTART:
			tpi_dhcp_server_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}
#endif

/*以下用于gpi获取信息函数*/
P_DHCP_SERVER_INFO_STRUCT tpi_dhcp_server_get_info()
{
	tpi_dhcp_server_update_info();
	return &dhcp_server_info;
}

/*以下用于gpi获取静态绑定IP信息函数*/
P_STATIC_IP_LIST tpi_dhcp_server_static_ip_get_info()
{
	tpi_dhcp_server_static_ip_update_info();

	return dhcp_server_info.static_ip_bind_info;
}

/*以下为该模块具体执行实现函数*/

/*在dhcp_server等其他线程可能会用到*/

/*只有本文件里面用*/
static RET_INFO tpi_dhcp_server_restart_phy()
{
	RET_INFO ret = RET_SUC;
	int i = 0;

	et_restart_lan_port(0);
	delete_all_online_client(TENDA_WLAN24_AP_IFNAME);
	delete_all_online_client(TENDA_WLAN5_AP_IFNAME);
	return ret;
}

static RET_INFO tpi_dhcp_server_start()
{
	tpi_dhcp_server_update_info();
	if(1 == dhcp_server_info.enable)
	{
		if(0 != dhcpd_start())
		{
			PI_ERROR(TPI,"start error!\n");
			return RET_ERR;
		}
		else
		{
			PI_PRINTF(TPI,"start success!\n");
		}
	}
	else
	{
		PI_ERROR(TPI,"the mib is off, connot start!\n");
	}
	
	return RET_SUC;
}

static RET_INFO tpi_dhcp_server_stop()
{
	tpi_dhcp_server_update_info();
	dhcpd_stop();

	PI_PRINTF(TPI,"stop success!\n");	
	return RET_SUC;
}

static RET_INFO tpi_dhcp_server_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_dhcp_server_stop() || RET_ERR == tpi_dhcp_server_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}
