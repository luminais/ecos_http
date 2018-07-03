/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_macfilter.c
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2016年11月27日
  最近修改   :
  功能描述   : 

  功能描述   : 黑白名单的cgi配置文件

  修改历史   :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedefs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <net/ethernet.h>
#include <net/netdb.h>
#include <arpa/inet.h>
#include "webs.h"
#include "uemf.h"
#include "router_net.h"
#include "cgi_tc.h"
#include "cgi_common.h"
#include "apclient_dhcpc.h"
#include "cgi_lib.h"
#include "wlioctl.h"
/****************************************************************/
#define MACFILTER_RULE_NUMBER_MAX 20

enum {
	MODE_DISABLED = 0,//禁用
	MODE_DENY,//仅禁止
	MODE_PASS//仅允许
};
/*****************************************************************************
 函 数 名  : macfilter_mode
 功能描述  : 更具模式返回对应的类型，方便判断
 输入参数  : char *filter_mode  
 输出参数  : 无
 返 回 值  : static
 
 修改历史      :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
static int macfilter_mode(char *filter_mode)
{
    if(strcmp(filter_mode,"deny") == 0)
    {
        return MODE_DENY;//仅禁止
    }
    else if(strcmp(filter_mode,"pass") == 0)
    {
        return MODE_PASS;//仅允许
    }
    else
    {
        return MODE_DISABLED;//禁用
    }
}

/*****************************************************************************
 函 数 名  : is_mac_filter
 功能描述  : 判断是否在MAC过滤中，如过是，则返回序号
 输入参数  : char* mac
 输出参数  : 无
 返 回 值  : int

 修改历史      :
  1.日    期   : 2016年10月9日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int is_mac_filter( char* mac ,char* filter_mode)
{
	int i = 0;
	char *value = NULL;
	int filtermode = 0;
	if(filter_mode == NULL || mac == NULL )
		return -1;	//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改

	filtermode = macfilter_mode(filter_mode);
	
	for (i = 0; i < MACFILTER_RULE_NUMBER_MAX; ++i)
	{
		if(filtermode == MODE_DENY)
			_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(i), value);
		else
			_GET_VALUE(ADVANCE_MACFILTER_PASS_RULE(i), value);
		
		if (!strcmp(value, "") || strlen(value) < 8  || 0 != strncmp(mac, value, strlen(mac)))
			continue;
		return i;
	}
	return -1;
}
/*****************************************************************************
 函 数 名  : macSetMib
 功能描述  : 根据不同的模式设置相应的参数
 输入参数  : const char *m_mac   
             const char *remark  
             char* filter_mode   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void macSetMib(const char *m_mac, const char *remark,char* filter_mode)
{
	int i = 0;
	char *mac_list = NULL;
	char *value = NULL;
	char tmp_buf[128] = {0};
	int filtermode = 0;
	
	
	if(filter_mode == NULL || m_mac == NULL || remark == NULL)
		return ;

	filtermode = macfilter_mode(filter_mode);
	
	for (i = 0; i < MACFILTER_RULE_NUMBER_MAX; ++i)
	{
		if(filtermode == MODE_DENY)
			_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(i), value);
		else
			_GET_VALUE(ADVANCE_MACFILTER_PASS_RULE(i), value);
		if (NULL != value && strlen(value) > 8  && 0 != strncmp(m_mac, value, strlen(m_mac)))
			continue;

		sprintf(tmp_buf, "%s,0-6,0-0,on,%s", m_mac, remark);
		if(filtermode == MODE_DENY)
			_SET_VALUE(ADVANCE_MACFILTER_DENY_RULE(i), tmp_buf);
		else
			_SET_VALUE(ADVANCE_MACFILTER_PASS_RULE(i), tmp_buf);
		break;
	}

}
/*****************************************************************************
 函 数 名  : macSetMibEmpty
 功能描述  : 将对应模式的maclist清空
 输入参数  : char* filter_mode  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void macSetMibEmpty(char* filter_mode)
{
	int i = 0;
	int filtermode = 0;
	if(filter_mode == NULL)
		return ;

	filtermode = macfilter_mode(filter_mode);
	
	for (i = 0; i < MACFILTER_RULE_NUMBER_MAX; ++i)
	{
		if(filtermode == MODE_DENY)
			_SET_VALUE(ADVANCE_MACFILTER_DENY_RULE(i), "");
		else
			_SET_VALUE(ADVANCE_MACFILTER_PASS_RULE(i), "");
	}
	//clean_filter_mac_list();
}	

/*****************************************************************************
 函 数 名  : cgi_macfilter_set
 功能描述  : 设置mac过滤的ui接口函数
 输入参数  : webs_t wp            
             CGI_MSG_MODULE *msg  
             char *err_code       
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO  cgi_lib_set_disposable_macfilter(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	char *list = NULL;
	char *old_mode = NULL;
	char m_hostname[128] = {0};
	char m_remark[128] = {0};
	char m_mac[32] = {0};
	char *argc[3] = {0};
	char *arglist[MAX_CLIENT_NUMBER] = {""};
	char *p = NULL;
	int i = 0;
	int count = 0;
	int send_msg_to_firewall = 0;
	char msg_param[PI_BUFLEN_256] = {0};
	char tmp[100];
	char *filter_mode = NULL;
	int filter_num = 0;
	char *mac_list = NULL;
       char acl_mac_list[(MACFILTER_ITEM_LEN + 1) * MAX_MACFILTER_LIST_MUM +1] = {0};
	filter_mode =  cgi_lib_get_var(wp,root, T(LIB_MAC_FILTER_MODE), T("deny"));
	list = cgi_lib_get_var(wp,root, T(LIB_MAC_FILTER_LIST), T(""));
	p = list;
	
	_GET_VALUE(ADVANCE_MACFILTER_MODE, old_mode);

	if(strcmp(filter_mode,old_mode))
	{
		send_msg_to_firewall = 1;
	}
	
	_SET_VALUE(ADVANCE_MACFILTER_MODE, filter_mode);
	filter_num = str2arglist(p, arglist, '\n', MAX_CLIENT_NUMBER);
	macSetMibEmpty(filter_mode);
	for (i = 0; i < filter_num; ++i)
	{

		if (arglist[i] == NULL || strlen(arglist[i]) < 10)
			continue;

		count = sscanfArglistConfig(arglist[i], '\t' ,  argc, 3);
		if(count != 3)
		{
			freeArglistConfig(argc, count);
			sprintf(err_code,"%s","1");
			return RET_ERR;
		}
		sprintf(m_hostname , "%s" , argc[0]);
		sprintf(m_remark 	, "%s" , argc[1]);
		sprintf(m_mac , "%s" , argc[2]);
		
		freeArglistConfig(argc, 3);
		qosMacToLower(m_mac);
	
		if(m_remark[0] != '\0' )
		{
			add_remark(m_mac, m_remark);
		}
		else if(get_remark(m_mac) != NULL)
		{
			add_remark(m_mac, m_hostname);
		}
		macSetMib(m_mac, m_remark,filter_mode);
	
		if(strlen(acl_mac_list) < MACFILTER_ITEM_LEN)
		{
			strcpy(acl_mac_list,m_mac);
		}
		else
		{
			sprintf(acl_mac_list,"%s %s",acl_mac_list,m_mac);
		}
	}
	_GET_VALUE(ADVICE_MAC_LIST, mac_list);
	if(strcmp(acl_mac_list,mac_list))
		send_msg_to_firewall = 1;
	
	sprintf(err_code,"%s","0");
	if(send_msg_to_firewall == 1)
	{
		CGI_MSG_MODULE msg_tmp;
		msg_tmp.id = RC_FIREWALL_MODULE;
		sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
		add_msg_to_list(msg,&msg_tmp);
	}
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_router_mode_black_list
 功能描述  : 后去路由模式下的黑名单
 输入参数  : webs_t wp
 输出参数  : 无
 返 回 值  : RET_INFO

 修改历史      :
  1.日    期   : 2016年10月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_router_mode_black_list(webs_t wp, cJSON *root, void *info)
{
	char *value = NULL;
	char cur_black_mac[20] = {0};
	int index = 0;
	unsigned char hostname[TPI_BUFLEN_64] = {0};    
	char *Remark = NULL;
	char *mac_hostname = NULL;
	cJSON *obj = NULL;
	int filtermode = MODE_DENY;

	while(filtermode <= MODE_PASS)
	{
		for (index = 0; index < MACFILTER_RULE_NUMBER_MAX; ++index)
		{
			if(filtermode == MODE_DENY)
				_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(index), value);
			else
				_GET_VALUE(ADVANCE_MACFILTER_PASS_RULE(index), value);
			if (!strcmp(value, "") || strlen(value) < 8 )
				continue;
			
			memset(cur_black_mac, 0x0, 20);
			sscanf(value, "%[^,]", cur_black_mac);

			Remark = get_remark(cur_black_mac);

			cJSON_AddItemToArray(root, obj = cJSON_CreateObject());

			mac_hostname = tenda_arp_mac_to_hostname(cur_black_mac);
			
			if(mac_hostname != NULL)
			{
#ifdef __CONFIG_SUPPORT_GB2312__
				if (1 == is_cn_encode(mac_hostname))
				{
					char host_name_utf_8[64] = {0};
					set_cn_ssid_encode("utf-8", mac_hostname, host_name_utf_8);
					strcpy(hostname, host_name_utf_8);
				}
#else
				if(1 == is_gb2312_code(mac_hostname))
				{
					strcpy(hostname, "Unknown") ;
				}
#endif
				else
				{
					strcpy(hostname, mac_hostname) ;
				}

			}
			else
			{
				strcpy(hostname, "Unknown") ;
			}
			
			cJSON_AddStringToObject(obj, T(LIB_HOST_NAME),hostname);
			cJSON_AddStringToObject(obj, T(LIB_REMARK), (Remark == NULL) ? "" : Remark);
			cJSON_AddStringToObject(obj, T(LIB_FILTER_MAC), cur_black_mac);
			if(filtermode == MODE_DENY)
				cJSON_AddStringToObject(obj, T(LIB_MAC_FILTER_MODE), "deny");
			else
				cJSON_AddStringToObject(obj, T(LIB_MAC_FILTER_MODE), "pass");
		}
		filtermode++;
	}

	return RET_SUC;		//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
}

/*****************************************************************************
 函 数 名  : cgi_ap_mode_black_list
 功能描述  : 后去路由模式下的黑名单
 输入参数  : webs_t wp
 输出参数  : 无
 返 回 值  : RET_INFO

 修改历史      :
  1.日    期   : 2016年10月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
#ifdef __CONFIG_BRIDGE_AP__
RET_INFO cgi_ap_mode_black_list(webs_t wp, cJSON *root, void *info)
{
	char *value = NULL;
	char cur_black_mac[20] = {0};
	int index = 0;
	int i = 0;
	char *Remark = NULL;
	cJSON *obj = NULL;
	struct maclist *mac_list = NULL;
	int mac_list_size;
	char dev_mac[PI_MAC_STRING_LEN] = {0};
	char dev_name[PI_DEVNAME_STRING_LEN] = {0};
	struct apclient_client_info* temp = NULL;
	int filtermode = MODE_DENY;

	mac_list_size = sizeof(uint) + 10 * sizeof(struct ether_addr);
	mac_list = (struct maclist *)malloc(mac_list_size);
	if(NULL == mac_list)
	{
		printf("*** fun=%s; line=%d; no buffers! ***\n", __func__, __LINE__);

		return -1;
	}

	if (getwlmaclist(AP_IFNAME, mac_list))
	{
		free(mac_list);

		return 0;
	}


	while(filtermode <= MODE_PASS)
	{
		for (index = 0; index < MACFILTER_RULE_NUMBER_MAX; ++index)
		{
			if(filtermode == MODE_DENY)
				_GET_VALUE(ADVANCE_MACFILTER_DENY_RULE(index), value);
			else
				_GET_VALUE(ADVANCE_MACFILTER_PASS_RULE(index), value);
			if (!strcmp(value, "") || strlen(value) < 8 )
				continue;
			
			memset(cur_black_mac, 0x0, 20);
			sscanf(value, "%[^,]", cur_black_mac);

			for (i = 0; i < mac_list->count; ++i)
			{
				memset(dev_mac, '\0', PI_MAC_STRING_LEN * sizeof(char));
				snprintf(dev_mac, PI_MAC_STRING_LEN, "%s", inet_mactoa(mac_list->ea[i].octet));
				qosMacToLower(dev_mac);
				if(0 == strncmp(cur_black_mac, dev_mac, strlen(dev_mac)))
				{
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
					break;
				}
			}
			if(i >= mac_list->count)
			{
				strcpy(dev_name, "Unknown") ;
			}
			Remark = get_remark(cur_black_mac);

			cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
			cJSON_AddStringToObject(obj, T(LIB_HOST_NAME),dev_name);
			cJSON_AddStringToObject(obj, T(LIB_REMARK), (Remark == NULL) ? "" : Remark);
			cJSON_AddStringToObject(obj, T(LIB_FILTER_MAC), cur_black_mac);
			if(filtermode == MODE_DENY)
				cJSON_AddStringToObject(obj, T(LIB_MAC_FILTER_MODE), "deny");
			else
				cJSON_AddStringToObject(obj, T(LIB_MAC_FILTER_MODE), "pass");
		}
		filtermode++;
	}
	FREE_P(&mac_list);
	return RET_SUC;		//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
}
#endif
//获取路由器当前的mac过滤方式
RET_INFO cgi_lib_get_macfilter_mode(webs_t wp, cJSON *root, void *info)
{
	char *filter_mode = NULL;
	_GET_VALUE(ADVANCE_MACFILTER_MODE, filter_mode);
	if(strcmp(filter_mode,"pass") == 0)
	{
		cJSON_AddStringToObject(root, T(LIB_MAC_FILTER_CUR_MODE),filter_mode);
	}
	else
	{
		cJSON_AddStringToObject(root, T(LIB_MAC_FILTER_CUR_MODE),"deny");
	}
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_get_macfilter_list
 功能描述  : 页面获取macfilter列表的函数接口，里面区分AP和route模式
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_macfilter_list(webs_t wp, cJSON *root, void *info)
{
	cJSON *item = NULL;
	cJSON_AddItemToObject(root, T(LIB_MAC_FILTER_LIST), item = cJSON_CreateArray());
	if(nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
	{
		#ifdef __CONFIG_BRIDGE_AP__
		cgi_ap_mode_black_list(wp, item, info);
		#endif
	}
	else
	{
		cgi_router_mode_black_list(wp, item, info);
	}
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_macfilter_mode
 功能描述  : app设置黑白名单工作方式
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年04月24日
    作    者   : 刘松明
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_macfilter_mode(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info)
{
	char *filter_mode = NULL;

	
	filter_mode =  cgi_lib_get_var(wp,root, T(LIB_MAC_FILTER_MODE), T("deny"));
	if((0 == strcmp(filter_mode,"deny")) || (0 == strcmp(filter_mode,"pass")))
	{
		_SET_VALUE(ADVANCE_MACFILTER_MODE, filter_mode);
	}
	else
	{
		printf("nothing to do\n");
		return RET_SUC;
	}

	sprintf(err_code,"%s","0");
	CGI_MSG_MODULE msg_tmp;
	msg_tmp.id = RC_FIREWALL_MODULE;
	sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
	add_msg_to_list(msg,&msg_tmp);
	
	return RET_SUC;
}

