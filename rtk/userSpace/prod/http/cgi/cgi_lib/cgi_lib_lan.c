#include "cgi_lib.h"
#include <router_net.h>
#include "dhcp_server.h"

void get_lan_dns(char *dns1_old, char * dns2_old)
{
	char str_temp[128] = {0};
	P_DHCP_SERVER_INFO_STRUCT dhcp_lan_info = NULL; 
	dhcp_lan_info = gpi_dhcp_server_info();
	strncpy(str_temp, dhcp_lan_info->dns, sizeof(str_temp));
	if( strstr(str_temp, " "))
	{
		sscanf(str_temp, "%s %s", dns1_old, dns2_old);
	}
	else 
	{
		sscanf(str_temp, "%s", dns1_old);
		dns2_old[0] = 0;
	}
}

RET_INFO cgi_lib_get_lan_info(webs_t wp, cJSON *root, void *info)
{
	P_DHCP_SERVER_INFO_STRUCT dhcp_lan_info = NULL;
	dhcp_lan_info = gpi_dhcp_server_info();
	/*lq ap 和 ap+client模式下获取lan口ip*/
	if(nvram_match(SYSCONFIG_WORKMODE,"bridge") 
		||nvram_match(SYSCONFIG_WORKMODE,"client+ap"))
	{
		char apclient_dhcpc_ip[17] = {0},apclient_dhcpc_mask[17] = {0};
		gpi_apclient_dhcpc_addr(apclient_dhcpc_ip,apclient_dhcpc_mask);
		if(0 == strcmp(apclient_dhcpc_ip,"") || 0 == strcmp(apclient_dhcpc_mask,""))
		{
			strcpy(apclient_dhcpc_ip,dhcp_lan_info->lan_ip);
			strcpy(apclient_dhcpc_mask,dhcp_lan_info->lan_mask);
		}
		cJSON_AddStringToObject(root, LIB_LAN_IP, apclient_dhcpc_ip);
		cJSON_AddStringToObject(root, LIB_LAN_MASK,apclient_dhcpc_mask);
	}
	else
	{
		cJSON_AddStringToObject(root, LIB_LAN_IP, dhcp_lan_info->lan_ip);
		cJSON_AddStringToObject(root, LIB_LAN_MASK, dhcp_lan_info->lan_mask);
	}
	return RET_SUC;
}

RET_INFO cgi_lib_get_dhcp_server_info(webs_t wp, cJSON *root, void *info)
{
	char dns1[32], dns2[32];
	P_DHCP_SERVER_INFO_STRUCT dhcp_lan_info = NULL;
#if defined (BCM535X)
	char str_temp[128] = {0};
#endif
	dhcp_lan_info = gpi_dhcp_server_info();

	if (1 == dhcp_lan_info->enable)
		cJSON_AddStringToObject(root, LIB_LAN_DHCP_ENABLE, "true");
	else
		cJSON_AddStringToObject(root, LIB_LAN_DHCP_ENABLE, "false");
	cJSON_AddStringToObject(root, LIB_LAN_DHCP_START_IP, dhcp_lan_info->start_ip);
	cJSON_AddStringToObject(root, LIB_LAN_DHCP_END_IP, dhcp_lan_info->end_ip);
#if defined (BCM535X)
	memset(str_temp, 0x0, sizeof(str_temp));
	sprintf(str_temp , "%d" , dhcp_lan_info->lease_time/3600);
	cJSON_AddStringToObject(root, LIB_LAN_DHCP_LEASE, str_temp);
#endif
		get_lan_dns(dns1, dns2);
	cJSON_AddStringToObject(root, LIB_LAN_DNS1, dns1);
	cJSON_AddStringToObject(root, LIB_LAN_DNS2, dns2);

	return RET_SUC;
}

RET_INFO cgi_lib_set_lan_info(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	
	int reboot_temp = 0;
	char str_temp[128] = {0};
	char msg_param[PI_BUFLEN_256] = {0};
	char_t *dhcpEn, *dns_temp;
	char_t *lan_ip, *lan_mask;
	char_t dhcp_ip_start[20],dhcp_ip_end[20];
	char_t old_lan_ip[20],old_lanmask[20];
	char_t lanDns1[32], lanDns2[32], dns1_old[32], dns2_old[32];
	unsigned int dhcp_ip[4],lan_ip2[4];
	int dhcpd_restart = 0, lan_restart = 0;
	

	P_DHCP_SERVER_INFO_STRUCT dhcp_lan_info = NULL, dhcp_lan_info_new = NULL;

	dhcp_lan_info = gpi_dhcp_server_info();
	get_lan_dns(dns1_old, dns2_old);
	dhcp_lan_info_new = gpi_dhcp_server_info();
	
	dhcpEn = cgi_lib_get_var(wp,root,T(LIB_LAN_DHCP_ENABLE),T(""));
	lan_ip = cgi_lib_get_var(wp,root, T(LIB_LAN_IP), "");  
	lan_mask = cgi_lib_get_var(wp,root, T(LIB_LAN_MASK), "255.255.255.0");  //mask 为固定的 255.255.255.0
	strncpy(lanDns1, cgi_lib_get_var(wp,root,T(LIB_LAN_DNS1),T("")), sizeof(lanDns1));
	strncpy(lanDns2, cgi_lib_get_var(wp,root,T(LIB_LAN_DNS2),T("")), sizeof(lanDns2));
	//dhcpen
	if (dhcp_lan_info->enable == 0 && strcmp(dhcpEn,"true")==0)
	{
		set_dhcpd_en(1);
		dhcpd_restart = 1;
	}	
	else if (dhcp_lan_info ->enable == 1 && strcmp(dhcpEn,"false")==0)
	{	
		set_dhcpd_en(0);
		dhcpd_restart = 1;
	}	

	//lan_ip,  lan_mask,  dhcp_ip limits
	strcpy(old_lan_ip, dhcp_lan_info->lan_ip);
	strcpy(old_lanmask, dhcp_lan_info->lan_mask);
	if (strcmp(dhcpEn,"true")==0)
	{
		sprintf(dhcp_ip_start,"%s",cgi_lib_get_var(wp,root, T(LIB_LAN_DHCP_START_IP), ""));
		sprintf(dhcp_ip_end,"%s",cgi_lib_get_var(wp,root, T(LIB_LAN_DHCP_END_IP), "")); 
		if(strcmp(dhcp_lan_info->start_ip, dhcp_ip_start)){
			_SET_VALUE(LAN_DHCP_POOL_START,dhcp_ip_start);
			dhcpd_restart = 1;
		}
		if(strcmp(dhcp_lan_info->end_ip, dhcp_ip_end)){
			_SET_VALUE(LAN_DHCP_POOL_END,dhcp_ip_end);
			dhcpd_restart = 1;
		} 
	}
	else
	{
		if (!web_check_addr(lan_ip))
		{
			get_address_pool( lan_ip , lan_mask,dhcp_ip_start,dhcp_ip_end );
			_SET_VALUE(LAN_DHCP_POOL_START,dhcp_ip_start);
			_SET_VALUE(LAN_DHCP_POOL_END,dhcp_ip_end);
		}
	}
	if(!nvram_match(LAN_NETMASK, lan_mask)){
		_SET_VALUE( LAN_NETMASK , lan_mask );
		dhcpd_restart = 1;
		lan_restart = 1;
	}
	if(strcmp(lan_ip, "") && strcmp(old_lan_ip, lan_ip) && (!web_check_addr(lan_ip)))
	{	
		_SET_VALUE(LAN_IPADDR,lan_ip );  	
		//跟原来 ip&mac 不是在同个 网段,需修改dhcp配置
		if(CGI_same_net_with_lan(SYS_lan_ip,SYS_lan_mask) == 0)
		{
			//change dhcp settings
			modify_filter_virtual_server(lan_ip);
		}
		reboot_temp = 1;
		strcpy(err_code, "100");
	}

	//lan dns
	memset(str_temp, 0x0, sizeof(str_temp));	
	// 由于页面没有做校验, 先更新 下发的dns 值。
	if (!strcmp(lanDns1,""))//页面下发为空
	{	
		if(strcmp(dhcp_lan_info_new->lan_ip, old_lan_ip)) //ip更新, 如果之前配置了 route 对应的ip,则dns需更新
		{	
			if (!strcmp(dns1_old, old_lan_ip))
			{
				strncpy(lanDns1, dhcp_lan_info_new->lan_ip, sizeof(lanDns1));		
			}	
			else if (!strcmp(dns2_old, old_lan_ip))
			{
				strncpy(lanDns2, dhcp_lan_info_new->lan_ip, sizeof(lanDns2));		
			}
		}
	}	
	else   //页面下发不为空
	{	
		if(!nvram_match(LAN_IPADDR, old_lan_ip) ) //ip更新, 如果之前配置了 route 对应的ip,则dns需更新
		{
			if (!strcmp(lanDns1, old_lan_ip))
			{
				strncpy(lanDns1, dhcp_lan_info_new->lan_ip, sizeof(lanDns1));		
			}	
			else if (!strcmp(lanDns2, old_lan_ip))  
			{			
				strncpy(lanDns2, dhcp_lan_info_new->lan_ip, sizeof(lanDns2));
			}
		}
	}
	if (strcmp(lanDns1,""))
	{
		if (strcmp(lanDns2,""))
		{
			sprintf(str_temp, "%s %s", lanDns1, lanDns2);
		}
		else
		{
			sprintf(str_temp, "%s", lanDns1);	
		}
		
		if (strcmp(str_temp, dhcp_lan_info->dns))
		{
			_SET_VALUE(LAN_DNS, str_temp);
		}	
	}	

	if(reboot_temp)
	{		
		int i;
		for(i = 0; i < MAX_MSG_NUM; i++)
		{
			if(msg[i].id == 0)
			{
				msg[i].id = RC_SYSTOOLS_MODULE;
				sprintf(msg[i].msg, "string_info=%s", "reboot");
				break;
			}
			else if(msg[i].id == RC_SYSTOOLS_MODULE)
			{
				break;
			}
		}
	}
	else if(dhcpd_restart && lan_restart)
	{
		printf("dhcp server && if lan && wl restart.\n");
		sprintf(msg_param, "op=%d", OP_STOP);
		msg_send(MODULE_RC, RC_DHCP_SERVER_MODULE, msg_param);
		msg_send(MODULE_RC, RC_LAN_MODULE, msg_param);
		sprintf(msg_param, "op=%d", OP_START);
		msg_send(MODULE_RC, RC_LAN_MODULE, msg_param);
		msg_send(MODULE_RC, RC_DHCP_SERVER_MODULE, msg_param);
	}
	else if(dhcpd_restart)
	{
		printf("dhcp server restart.\n");
		int i;
		for(i = 0; i < MAX_MSG_NUM; i++)
		{
			if(msg[i].id == 0)
			{
				msg[i].id = RC_DHCP_SERVER_MODULE;
				sprintf(msg[i].msg, "op=%d", OP_RESTART);
				break;
			}
			else if(msg[i].id == RC_DHCP_SERVER_MODULE)
			{
				break;
			}
		}
	}

	if(!err_code[0])
	{
		strcpy(err_code, "100");
	}
	return RET_SUC;
}

