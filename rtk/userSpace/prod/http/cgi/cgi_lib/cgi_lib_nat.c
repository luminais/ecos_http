/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_lib_nat.c
  版 本 号   : 初稿
  作    者   : liusongming
  生成日期   : 2016年12月13日
  最近修改   :
  功能描述   : 

  功能描述   : nat的最小功能单元的get和set库

  修改历史   :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 创建文件

******************************************************************************/
#include "webs.h"
#include "cJSON.h"
#include "cgi_common.h"
#include "firewall.h"
#include "dhcp_server.h"
#include "bcmddns.h"
#include "ddns.h"
#include "bcmupnp.h"
#include "natcmd.h"
#include "iptv.h"
#include "cgi_lib.h"
#include "elink.h"



static void clear_all_virtual_server()
{
	int i = 0;
	
	for (i = 0; i < VTS_MAX_NUM_1; ++i)
	{
		_SET_VALUE(ADVANCE_PORT_FORWART_RULE(i), "");
	}
}



static int prase_webstr2virtualServer(char *old_value, char *new_value)
{
	/*   
	*web string:   192.168.0.100;53;7008
	*                   192.168.0.21;21;22;Both
	*nvram value: 7008-7008>192.168.0.100:53-53,tcp,on
	        		       56-56>192.168.0.215:56-56,tcp/udp,on
	*/
	char *arglists[4] = {NULL};
	if (! (str2arglist(old_value, arglists, ';', 4) == 4)) {
		return 0;
	}
	
	if (!strcmp(arglists[3], "tcp"))
	{
		strcpy(arglists[3], "tcp");	
	}
	else if (!strcmp(arglists[3], "udp"))
	{
		strcpy(arglists[3], "udp");
	}
	else
	{
		strcpy(arglists[3], "tcp/udp");
	}
	
	
sprintf(new_value, "%s-%s>%s:%s-%s,%s,%s",
			"All", "All", arglists[0], arglists[1], arglists[2], arglists[3], "on");

	printf("===%s===new_value:%s\n",__FUNCTION__, new_value);
		
				
	return 1;		
}

/*****************************************************************************

/*****************************************************************************
 函 数 名  : cgi_lib_nat_staticip_get
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/

RET_INFO cgi_lib_get_nat_staticip(webs_t wp, cJSON *root, void *info)
{
    char *value = NULL;
	cJSON *obj = NULL;
	cJSON *array = NULL;
	P_STATIC_IP_LIST static_ip_list = NULL;
	
	static_ip_list = gpi_dhcp_server_static_ip_get_info();
	
	while(static_ip_list)
	{
		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		cJSON_AddStringToObject(obj, LIB_STATIC_IPLIST_IP, (static_ip_list->ip_addr));
		cJSON_AddStringToObject(obj, LIB_STATIC_IPLIST_MAC, (static_ip_list->mac_addr));
		cJSON_AddStringToObject(obj, LIB_STATIC_IPLIST_MARK, (static_ip_list->remark));
		
		static_ip_list = static_ip_list->next;
	}


	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_nat_staticip
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_nat_staticip(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char *staticList = NULL;
	char *arglist[20] = {NULL};
	char *argc[6] = {0}; 
	int  count = 0;
	char *p = NULL;
	int i =0;
	int dhcpd_restart = 0;
	char listBuf[128] = {0};

    staticList = cgi_lib_get_var(wp,root, T("staticList"), "");
	if(strcmp(staticList, nvram_safe_get(ADVANCE_STATIC_IP_MAPPING_LIST))  != 0)
	{
		dhcpd_restart = 1;
		_SET_VALUE(ADVANCE_STATIC_IP_MAPPING_LIST, staticList);
		
		p = staticList;
		
		str2arglist(p, arglist, '\n', 20);
		__static_dhcp_config_delete_all ();	
		
		for( i = 0 ; i <= DHCPD_STATIC_LEASE_NU ; i++){
			if (arglist[i] == NULL || strlen(arglist[i]) < 10)
				continue;
			
			count = sscanfArglistConfig(arglist[i],'\t' ,  argc, 3);
			if(count == 3){
				qosMacToLower(argc[1]);
				sprintf(listBuf,"%s;%s;%s;1;60",argc[2],argc[0],argc[1]);
				_SET_VALUE(ADVANCE_STATIC_IP_MAPPING_RULE(i), listBuf);					
				add_remark(argc[1],argc[2]);
			}
			freeArglistConfig(argc,count);
		}
	}
	
	_COMMIT();
	if(dhcpd_restart)
	{	
		printf("dhcp server restart.\n");
		
			
		CGI_MSG_MODULE msg_tmp;
		msg_tmp.id = RC_DHCP_SERVER_MODULE;
		sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
		add_msg_to_list(msg,&msg_tmp);
	}
	
	if(!err_code[0])
	{
		strcpy(err_code, "0");
	}
		
	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_nat_portForward_get
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/

RET_INFO cgi_lib_get_nat_portForward(webs_t wp, cJSON *root, void *info)
{
	int i = 0;
	cJSON *obj = NULL;
	int in_port = 0;
	int ex_port = 0; 
	char *value = NULL;
	char ip_ex[32] = {0},ip[32] = {0};
	char porto[32] = {0};
	P_FIREWALL_INFO_STRUCT firewall_nat = NULL;
	

	for ( i = 0; i < VTS_MAX_NUM_1; ++i)
	{
		/*56-56>192.168.0.215:56-56,tcp/udp,on
		 *"53-80>192.168.0.100:53-80,tcp,on"
		 *Parse wan_port0-wan_port1>lan_ipaddr:lan_port0-lan_port1[:,]proto[:,]enable[:,]desc 
		 */

		firewall_nat = gpi_firewall_info();
		value =  firewall_nat->portfd_info.forward_port[i];
		if (value && strlen(value) < 12)
		{
			continue;
		}

		if (5 != sscanf(value,	"%[^-]-%*[^>]>%[^:]:%d-%d,%[^,],%*s", ip_ex , ip , &in_port , &ex_port, porto)) {
			continue;
		}

		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		cJSON_AddStringToObject(obj, "portListIntranetIP", ip);
		cJSON_AddStringToObject(obj, "portListExtranetIP", ip_ex);
		cJSON_AddNumberToObject(obj, "portListIntranetPort", in_port);
		cJSON_AddNumberToObject(obj, "portListExtranetPort", ex_port);
		
		if (!strcmp(porto, "tcp/udp"))
		{
			strcpy(porto, "both");
		}
		cJSON_AddStringToObject(obj, "portListProtocol", porto);
	}

	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_get_nat_upnp
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_nat_portForward(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    int i = 0;
	P_FIREWALL_INFO_STRUCT firewall_nat = NULL;
	int firewall_restart = 0;
	char_t *portList = NULL;
	char listBuf[512] = {0};
	char old_value[64] = {0}, new_value[64] = {0};
	char *p = NULL;

	portList = cgi_lib_get_var(wp,root,T("portList"), "");
	/*firewall and nat rule infot*/
	firewall_nat = gpi_firewall_info();
	//Port Forwarding
	/*
	 *   web  string:  "192.168.0.100;53;7008~192.168.0.10.;21;65535~..."
	 *   nvram value: "7008-7008>192.168.0.100:53-53,tcp/udp,on","65535-65535>192.168.0.100:21-21,tcp/udp,on"......
	 */

	if(strcmp(portList, firewall_nat->portfd_info.forward_port_list) != 0)
	{
		firewall_restart = 1;
		_SET_VALUE(ADVANCE_PORT_FORWART_LIST, portList);
		
		clear_all_virtual_server();
		
		i = 0;
		strcpy(listBuf, portList);
		
		for (p = strtok(listBuf, "~"); p; p = strtok(NULL, "~"))
		{
			if (i >= VTS_MAX_NUM_1)
				break;
			
			if (strlen(p) > 10)
			{
				printf("---->>>%s\n", p);
				strcpy(old_value, p);
				if (!prase_webstr2virtualServer(old_value, new_value))
					continue;

				_SET_VALUE(ADVANCE_PORT_FORWART_RULE(i),new_value);

				++i;
			}
		}
	}

	if(firewall_restart)
	{	
		printf("firewall restart.\n");
		int i;
		for(i = 0; i < MAX_MSG_NUM; i++)
		{
			if(msg[i].id == 0)
			{
				msg[i].id = RC_FIREWALL_MODULE;
				sprintf(msg[i].msg, "op=%d", OP_RESTART);
				break;
			}
			else if(msg[i].id == RC_FIREWALL_MODULE)
			{
				break;
			}
		}
	}
	
	
	if(!err_code[0])
	{
		strcpy(err_code, "0");
	}
	
	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_nat_ddns_get
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_nat_ddns(webs_t wp, cJSON *root, void *info)
{
    P_DDNS_INFO_STRUCT ddns_info = NULL;
    char *arglists[9]= {NULL};
    char ddns_value[256] = {0};
#ifdef __CONFIG_DDNS__
    ddns_info = gpi_ddns_info();

   
    if (ddns_info->enable == 1)
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_EN, "true");
    else
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_EN, "false");

    switch(ddns_info->isp)
    {
        case 1:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "oray.com");
            break;
        case 2:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "dyn.com");
            break;
        case 3:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "88ip.cn");
            break;
        case 4:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "freedns.afraid.org");
            break;
        case 5:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "zoneedit.com");
            break;
        case 6:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "no-ip.com");
            break;
        case 7:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "3322.org");
            break;
        default:
            cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVICE_NAME, "oray.com");
            break;
    }
    
    strncpy(ddns_value,ddns_info->ddns_set,sizeof(ddns_value));
    if(str2arglist(ddns_value, arglists, ';', 9) == 9)
    {
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_USER, arglists[3]);
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_PWD, arglists[4]);
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVER, arglists[2]);
    }
    else
    {
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_USER, "");
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_PWD, "");
        cJSON_AddStringToObject(root, LIB_NAT_DDNS_SERVER, "");
    }
    
    cJSON_AddStringToObject(root, LIB_NAT_DDNS_STATUS, ddns_info->status);
#endif

    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_nat_ddns_get
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/

RET_INFO cgi_lib_set_nat_ddns(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t *ddnsEn = NULL, *ddnsServiceName = NULL, *ddnsUser = NULL, *ddnsPwd = NULL, *ddnsHostName = NULL;
    char_t *serName = NULL;
    char value[256] = {0};
    int ddns_restart = 0;
    char null_str[2] = {0};

    ddnsEn = cgi_lib_get_var(wp,root, LIB_NAT_DDNS_EN, "");
    ddnsServiceName = cgi_lib_get_var(wp,root, LIB_NAT_DDNS_SERVICE_NAME, "");
    ddnsUser = cgi_lib_get_var(wp,root, LIB_NAT_DDNS_USER, "");
    ddnsPwd = cgi_lib_get_var(wp,root, LIB_NAT_DDNS_PWD, "");
    ddnsHostName = cgi_lib_get_var(wp,root, LIB_NAT_DDNS_SERVER, "");
                    
                
    if (0 == strcmp(ddnsEn, "true")) 
    {
        if (0 == strcmp(ddnsServiceName, "dyn.com"))
        {
            if((strcmp("2", nvram_safe_get(ADVANCE_DDNS_ISP))  != 0)
                ||(strcmp("2", nvram_safe_get(ADVANCE_DDNS_ENABLE))  != 0) )
            {
                ddns_restart = 1;
                _SET_VALUE(ADVANCE_DDNS_ISP,"2");
                _SET_VALUE(ADVANCE_DDNS_ENABLE,"2");
            }
            serName= DYNDNS_NAME;
        }
        else if (0 == strcmp(ddnsServiceName, "no-ip.com"))
        {
            if((strcmp("6", nvram_safe_get(ADVANCE_DDNS_ISP))  != 0)
                ||(strcmp("6", nvram_safe_get(ADVANCE_DDNS_ENABLE))  != 0) )
            {
                ddns_restart = 1;
                _SET_VALUE(ADVANCE_DDNS_ISP,"6");
                _SET_VALUE(ADVANCE_DDNS_ENABLE,"6");
            }
            serName = NOIP_NAME;
        }
        else if (0 == strcmp(ddnsServiceName, "oray.com"))
        {
            if((strcmp("1", nvram_safe_get(ADVANCE_DDNS_ISP))  != 0)
                ||(strcmp("1", nvram_safe_get(ADVANCE_DDNS_ENABLE))  != 0) )
            {
                ddns_restart = 1;
                _SET_VALUE(ADVANCE_DDNS_ISP,"1");
                _SET_VALUE(ADVANCE_DDNS_ENABLE,"1");
            }
            ddnsHostName = null_str;
            serName = ORAY_NAME;
        }
        else if (0 == strcmp(ddnsServiceName, "88ip.cn"))
        {
            if((strcmp("3", nvram_safe_get(ADVANCE_DDNS_ISP))  != 0)
                ||(strcmp("3", nvram_safe_get(ADVANCE_DDNS_ENABLE))  != 0) )
            {
                ddns_restart = 1;
                _SET_VALUE(ADVANCE_DDNS_ISP,"3");
                _SET_VALUE(ADVANCE_DDNS_ENABLE,"3");
            }
            ddnsHostName = null_str;
            serName = M88IP_NAME;
        }
        else if (0 == strcmp(ddnsServiceName, "3322.org"))
        {
            if((strcmp("7", nvram_safe_get(ADVANCE_DDNS_ISP))  != 0)
                ||(strcmp("7", nvram_safe_get(ADVANCE_DDNS_ENABLE))  != 0) )
            {
                ddns_restart = 1;
                _SET_VALUE(ADVANCE_DDNS_ISP,"7");
                _SET_VALUE(ADVANCE_DDNS_ENABLE,"7");
            }
            serName = QDNS_NAME;
        }
        else
        {
            ;
        }

        /*1;dyndns;dyndnsyue.com;admin;admin;;;;60*/
        memset(value,0,sizeof(value));
        snprintf(value,sizeof(value),"%s;%s;%s;%s;%s;%s;%s;%s;%s",
            "1", serName, ddnsHostName, ddnsUser, ddnsPwd, "", "", "", "60");

        if((strcmp(value, nvram_safe_get(ADVANCE_DDNS_SET1))  != 0)
            ||(strcmp(ddnsHostName, nvram_safe_get(ADVANCE_DDNS_HOSTNAME))  != 0) )
        {
            ddns_restart = 1;
            _SET_VALUE(ADVANCE_DDNS_SET1,value);
            _SET_VALUE(ADVANCE_DDNS_HOSTNAME,ddnsHostName);
        }
        _SET_VALUE("wan0_host",ddnsHostName);
    } 
    else 
    {
        if(strcmp("0", nvram_safe_get(ADVANCE_DDNS_ENABLE))  != 0)
        {
            ddns_restart = 1;
            _SET_VALUE(ADVANCE_DDNS_ENABLE,"0");
        }
        _SET_VALUE("wan0_host","");
    }
    
    if(ddns_restart)
    {   
        printf("firewall restart.\n");
        int i;
        for(i = 0; i < MAX_MSG_NUM; i++)
        {
            if(msg[i].id == 0)
            {
                msg[i].id = RC_DDNS_MODULE;
                sprintf(msg[i].msg, "op=%d", OP_RESTART);
                break;
            }
            else if(msg[i].id == RC_DDNS_MODULE)
            {
                break;
            }
        }
    }
    
    
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
    
    
    return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_get_nat_dmz
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_nat_dmz(webs_t wp, cJSON *root, void *info)
{
    char *value = NULL;
    P_FIREWALL_INFO_STRUCT firewall_nat = NULL;
    firewall_nat = gpi_firewall_info();
    
    
    value = firewall_nat->dmz_info.dmz_ipaddr_en;
    if (0 == strcmp(value, "1"))
        cJSON_AddStringToObject(root, LIB_NAT_DMZ_EN, "true");
    else
        cJSON_AddStringToObject(root, LIB_NAT_DMZ_EN, "false");
    value = firewall_nat->dmz_info.dmz_ipaddr;
    cJSON_AddStringToObject(root, LIB_NAT_DMZ_HOST_IP, value);

    return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_set_nat_dmz
 功能描述  : 配置dmz主机
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_nat_dmz(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t *dmzEn, *dmzHostIP;
	int firewall_restart = 0;
	P_FIREWALL_INFO_STRUCT firewall_nat = NULL;
	dmzEn = cgi_lib_get_var(wp,root, LIB_NAT_DMZ_EN, "");
	dmzHostIP = cgi_lib_get_var(wp,root, LIB_NAT_DMZ_HOST_IP, "");

	firewall_nat = gpi_firewall_info();
	
	if (0 == strcmp(dmzEn, "true"))
	{
		if((strcmp("1", firewall_nat->dmz_info.dmz_ipaddr_en)  != 0)
			||(strcmp(dmzHostIP, firewall_nat->dmz_info.dmz_ipaddr)  != 0) )
		{
			firewall_restart = 1;
			_SET_VALUE(ADVANCE_DMZ_IPADDR_EN, "1");
			_SET_VALUE(ADVANCE_DMZ_IPADDR, dmzHostIP);
		}
	}	
	else
	{
		if(strcmp("0", firewall_nat->dmz_info.dmz_ipaddr_en)  != 0)
		{
			firewall_restart = 1;
			_SET_VALUE(ADVANCE_DMZ_IPADDR_EN, "0");
		}
	}

	if(firewall_restart)
	{	
		printf("firewall restart.\n");
		int i;
		for(i = 0; i < MAX_MSG_NUM; i++)
		{
			if(msg[i].id == 0)
			{
				msg[i].id = RC_FIREWALL_MODULE;
				sprintf(msg[i].msg, "op=%d", OP_RESTART);
				break;
			}
			else if(msg[i].id == RC_FIREWALL_MODULE)
			{
				break;
			}
		}
	}
	

	if(!err_code[0])
	{
		strcpy(err_code, "0");
	}

	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_lib_get_nat_upnp
 功能描述  : 获取静态ip列表
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_nat_upnp(webs_t wp, cJSON *root, void *info)
{
    P_UPNP_INFO_STRUCT upnp_info = NULL;

    upnp_info = gpi_upnp_get_info();
    if (upnp_info->enable)
        cJSON_AddStringToObject(root, LIB_NAT_UPNP_EN, "true");
    else
        cJSON_AddStringToObject(root, LIB_NAT_UPNP_EN, "false");

    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_nat_upnp
 功能描述  : 配置dmz主机
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/

RET_INFO cgi_lib_set_nat_upnp(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    int upnp_restart = 0;
    char_t *upnpEn;
    P_UPNP_INFO_STRUCT upnp_info = NULL;
    upnp_info = gpi_upnp_get_info();
    
    upnpEn = cgi_lib_get_var(wp,root, LIB_NAT_UPNP_EN, "");
    if (0 == strcmp(upnpEn, "true"))
    {
        if(!upnp_info->enable)
        {
            _SET_VALUE(ADVANCE_UPNP_ENABLE, "1");
            upnp_restart = 1;
        }
    }
    else 
    {
        if(upnp_info->enable)
        {
            _SET_VALUE(ADVANCE_UPNP_ENABLE, "0");
            upnp_restart = 1;
        }
    }   

    if(upnp_restart)
    {   
        printf("upnp restart.\n");
        int i;
        for(i = 0; i < MAX_MSG_NUM; i++)
        {
            if(msg[i].id == 0)
            {
                msg[i].id = RC_UPNP_MODULE;
                sprintf(msg[i].msg, "op=%d", OP_RESTART);
                break;
            }
            else if(msg[i].id == RC_UPNP_MODULE)
            {
                break;
            }
        }
    }
    
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }

    return RET_SUC;
}


#ifdef __CONFIG_TENDA_APP__
/*发送重新建立链接到ucloud*/
int restart_ucloud_connect_svr()
{
	cJSON *cj_set	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString("restart_connect_svr"));

	uc_cgi_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);
}
#endif


#ifdef __CONFIG_IPTV__
RET_INFO cgi_lib_get_nat_iptv(webs_t wp, cJSON *root, void *info)
{
	char iptv_enable[8] = {0};
	char vlan_area[8] = {0};
	char vlan_id[128] = {0};
	char vlan_sel[8] = {0};
	cJSON *arry = NULL;
	cJSON *obj = NULL;
	int num = 0;

	char *p = NULL;
	char *q = NULL;

    strcpy(iptv_enable,nvram_safe_get(ADVANCE_IPTV_ENABLE));
	if(0 == strcmp("1",iptv_enable))
	{
		cJSON_AddStringToObject(root,LIB_NAT_IPTV_EN,"true");
	}
	else
	{
		cJSON_AddStringToObject(root,LIB_NAT_IPTV_EN,"false");
	}

	cJSON_AddItemToObject(root,LIB_NAT_IPTV_VLANID,arry = cJSON_CreateArray());
	strcpy(vlan_area,nvram_safe_get(ADVANCE_IPTV_VLAN_ZONE));
	if(0 == strcmp(VLAN_AREA_DEFAULT,vlan_area))	//地区选择表示默认
	{
		cJSON_AddStringToObject(root,LIB_NAT_IPTV_ZONE,VLAN_AREA_DEFAULT);
		cJSON_AddStringToObject(root,LIB_NAT_IPTV_SELECT,"");
	}
	else if(0 == strcmp(VLAN_AREA_SHANGHAI,vlan_area))	//1代表上海地区
	{
		strcpy(vlan_id,nvram_safe_get(ADVANCE_IPTV_VLAN_ID));

		cJSON_AddStringToObject(root,LIB_NAT_IPTV_ZONE,VLAN_AREA_SHANGHAI);
		sscanf(vlan_id,"%d",&num);
		cJSON_AddItemToArray(arry,cJSON_CreateNumber(num));
		cJSON_AddStringToObject(root,LIB_NAT_IPTV_SELECT,"");
	}
	else if(0 == strcmp(VLAN_AREA_CUSTOM,vlan_area))	//2表示自定义
	{
		strcpy(vlan_id,nvram_safe_get(ADVANCE_IPTV_VLAN_ID));
		strcpy(vlan_sel,nvram_safe_get(ADVANCE_IPTV_VLAN_SEL));

		cJSON_AddStringToObject(root,LIB_NAT_IPTV_ZONE,VLAN_AREA_CUSTOM);
		p = vlan_id;
		for(p; *p != '\0'; )
		{
			q = strchr(p,',');
			if(NULL != q)
			{
				*q = '\0';
				sscanf(p,"%d",&num);
				cJSON_AddItemToArray(arry,cJSON_CreateNumber(num));
				p = q + 1;
				continue;
			}
			if(strcmp(p,""))
			{
				sscanf(p,"%d",&num);
				cJSON_AddItemToArray(arry,cJSON_CreateNumber(num));
			}
			break;
		}
		cJSON_AddStringToObject(root,LIB_NAT_IPTV_SELECT,vlan_sel);
	}
	return RET_SUC;
}
RET_INFO cgi_lib_set_nat_iptv(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	int iptv_restart = 0;
	char_t *IPTVEn = NULL;
	char_t *VLANArea = NULL;
	char_t * VLANID = NULL;
	char_t *VLANSelect = NULL;
	char IPTV_enable[32] = {0};
	char vlan_zone[32] = {0};
	char vlan_id[8] = {0};
	char vlan_sel[8] = {0};
	int vlan_id_cur = 0;
	char value[128] = {0};
	int i = 0,index = 0;
	char *p = NULL;
	char *q = NULL;
	IPTVEn = cgi_lib_get_var(wp,root,LIB_NAT_IPTV_EN,"");
	VLANArea = cgi_lib_get_var(wp,root,LIB_NAT_IPTV_ZONE,"");
	VLANID = cgi_lib_get_var(wp,root,LIB_NAT_IPTV_VLANID,"");
	VLANSelect = cgi_lib_get_var(wp,root,LIB_NAT_IPTV_SELECT,"");
	if(0 == strcmp("true",IPTVEn))
	{
		strcpy(IPTV_enable,"1");
	}
	else 
	{
		strcpy(IPTV_enable,"0");
	}
	if(0 == strcmp(VLAN_AREA_DEFAULT,VLANArea))	//地区选择表示默认
	{
		strcpy(vlan_id,"");
		strcpy(vlan_sel,"");
	}
	else if(0 == strcmp(VLAN_AREA_SHANGHAI,VLANArea))	//1代表上海地区
	{
		strcpy(vlan_id,VLANID);
		strcpy(vlan_sel,"");
	}
	else if(0 == strcmp(VLAN_AREA_CUSTOM,VLANArea))	//2表示自定义
	{
		index = atoi(VLANSelect) + 1;
		memset(value,0x0,sizeof(value));
		strcpy(value,VLANID);
		p = value;
		for(p; *p != '\0';)
		{
			q = strchr(p,',');
			if(NULL != q)
			{
				i++;
				*q = '\0';
				if(i == index)
				{
					strcpy(vlan_id,p);
					break;
				}
				p = q + 1;
				continue;
			}
			if((i + 1) == index)
			{
				strcpy(vlan_id,p);
				break;
			}
		}
	}
	if((strcmp(IPTV_enable,nvram_safe_get(ADVANCE_IPTV_ENABLE))) ||
	   (strcmp(VLANArea,nvram_safe_get(ADVANCE_IPTV_VLAN_ZONE))) ||
	   (strcmp(VLANID,nvram_safe_get(ADVANCE_IPTV_VLAN_ID)))	||
	   (strcmp(VLANSelect,nvram_safe_get(ADVANCE_IPTV_VLAN_SEL))) ||
	   (strcmp(vlan_id,nvram_safe_get(ADVANCE_IPTV_VLAN_ID_CUR)))
	  )
	{
		nvram_set(ADVANCE_IPTV_ENABLE,IPTV_enable);
		nvram_set(ADVANCE_IPTV_VLAN_ZONE,VLANArea);
		nvram_set(ADVANCE_IPTV_VLAN_ID,VLANID);
		nvram_set(ADVANCE_IPTV_VLAN_SEL,VLANSelect);
		nvram_set(ADVANCE_IPTV_VLAN_ID_CUR,vlan_id);
		iptv_restart = 1;
	}
	else
	{
		if(!err_code[0])
	    {
	        strcpy(err_code, "0");
	    }
		return RET_SUC;
	}
	if(iptv_restart)
    {   
		/* 将消息加入消息列表 */
		CGI_MSG_MODULE msg_tmp;
		msg_tmp.id = RC_SYSTOOLS_MODULE;
		sprintf(msg_tmp.msg, "%s", "string_info=reboot");
		add_msg_to_list(msg, &msg_tmp);
    }
	if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
	return RET_SUC;
}
#endif
RET_INFO cgi_lib_get_nat_elink(webs_t wp, cJSON *root, void *info)
{
    P_ELINK_INFO_STRUCT pelink_info = NULL;
    char st_buf[32] = {0};

    pelink_info = gpi_elink_get_info();
    
    if (pelink_info->enable)
        cJSON_AddStringToObject(root, LIB_NAT_ELINK_EN, "true");
    else
        cJSON_AddStringToObject(root, LIB_NAT_ELINK_EN, "false");

    sprintf(st_buf, "%d", pelink_info->status);
    cJSON_AddStringToObject(root, LIB_NAT_ELINK_ST, st_buf);
    
    return RET_SUC;
}




RET_INFO cgi_lib_set_nat_elink(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    int elink_restart = 0;
    char_t *elinkEn;
    P_ELINK_INFO_STRUCT elink_info = NULL;
    elink_info = gpi_elink_get_info();

    
    elinkEn = cgi_lib_get_var(wp,root, LIB_NAT_ELINK_EN, "");

    printf("%s [%d]WEB elinkEn=%s, elink_info->enable=%d\n", __FUNCTION__, __LINE__, elinkEn, elink_info->enable);

    
    if (0 == strcmp(elinkEn, "true"))
    {
        if(!elink_info->enable)
        {
            _SET_VALUE(ADVANCE_ELINK_ENABLE, "1");
            elink_restart = 1;
        }
    }
    else 
    {
        if(elink_info->enable)
        {
            _SET_VALUE(ADVANCE_ELINK_ENABLE, "0");
            elink_restart = 1;
        }
    }   

    if(elink_restart)
    {   
        printf("elink restart.\n");
        int i;
        for(i = 0; i < MAX_MSG_NUM; i++)
        {
            if(msg[i].id == 0)
            {
                msg[i].id = RC_ELINK_MODULE;
                sprintf(msg[i].msg, "op=%d", OP_RESTART);
                break;
            }
            else if(msg[i].id == RC_ELINK_MODULE)
            {
                break;
            }
        }
    }
    
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }

    return RET_SUC;
}
