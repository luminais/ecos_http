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
#include "uemf.h"
#include "router_net.h"
#ifdef __CONFIG_STREAM_STATISTIC__
#include "tc.h"
#endif
#include "arp_clients.h"
#include "tenda_arp.h"
#include "urlf.h"
#include "cgi_lib.h"


#define	STATIC_LIST_LOCK()		cyg_scheduler_lock()
#define	STATIC_LIST_UNLOCK()	cyg_scheduler_unlock()


extern int tenda_arp_is_wireless_client(unsigned char *mac);

/****************************************************************/

extern struct detec *det[TENDA_ARP_MAX_NUM];

enum {
	MODE_DISABLED = 0,//禁用
	MODE_DENY,//仅禁止
	MODE_PASS//仅允许
};


int parsePerParentControlClientConfig(char *value, char *hostname, char *mark,char *mac, char *ip, char *enable)
{
	char *sp,*ep;
	
	if( !value || !hostname ||  !mark || !mac || !ip ||!enable)
		return -1;

	if(strlen(value) < 28) //最小长度31
	{
		return -1;
	}

	char *list = (char *)malloc(strlen(value)+1);
	if(list == NULL){
		printf("parsePerParentControlClientConfig: malloc failed\n");
		return -1;
	}
	
	memset(list,0,strlen(value)+1);

	strcpy(list, value);
	
	sp = list;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	if(strlen(sp) > TPI_BUFLEN_64-1)
		strncpy(hostname, sp, TPI_BUFLEN_64-1);
	else
		strncpy(hostname, sp, strlen(sp));

	sp = ep;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	if(strlen(sp) > TPI_BUFLEN_64-1)
		strncpy(mark, sp, TPI_BUFLEN_64-1);
	else
		strncpy(mark, sp, strlen(sp));

	
	sp = ep;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	strcpy(mac, sp);
	
	sp = ep;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	strcpy(ip, sp);
	
	sp = ep;
	strcpy(enable, sp);

	free(list);
	list = NULL;
	return 0;
}


enum {
	REMARK_NOT_NEED_SAVE = 0,
	REMARK_NEED_SAVE,
};

static int getParentControlUrlFilterList2web(cJSON *array)
{
	char *value;
	if( array ==NULL)
		return 0;
	_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_URLFILTER_LIST, value);

	if(strcmp(value, "") == 0)
		return 0;

	char *list = (char *)malloc(strlen(value)+1);
	if(list == NULL){
		printf("getParentControlUrlFilterList2web: malloc failed\n");
		return -1;
	}
	
	memset(list,0,strlen(value)+1);

	strcpy(list, value);
	
	int num = 0;
	char *sp,*ep;

	num++;
	sp = list;
	while((ep = strchr(sp, '\n'))) {
		*ep++ = '\0';
		cJSON_AddStringToObject(array, "Fuck", sp);
		sp = ep;
		num++;	
	}
	
	/* 只有一条或最后一条*/
	cJSON_AddStringToObject(array, "Fuck", sp);

	free(list);
	list = NULL;
	return num;
}

static int updateParentControlClientInfo(struct client_info *clients_list, int client_num)
{
	int i = 0, j = 0;
	char *remark;
	struct in_addr ina;
	struct detec *tmp_node = NULL;

	for(i = 0 , j = 0 ; i < TENDA_ARP_SIZE(det); i++)
	{
		tmp_node = det[i];
		while(tmp_node)
		{
			clients_list[j].in_use = 1;
			ina.s_addr = tmp_node->ip;

			snprintf(clients_list[j].mac , TPI_MAC_STRING_LEN,  "%02x:%02x:%02x:%02x:%02x:%02x" , tmp_node->mac[0], tmp_node->mac[1], tmp_node->mac[2], tmp_node->mac[3], tmp_node->mac[4], tmp_node->mac[5]);
			snprintf( clients_list[j].ip , TPI_IP_STRING_LEN, "%s" , inet_ntoa(ina));

			remark = get_remark(clients_list[j].mac);
			if(remark)
			{
				snprintf(clients_list[j].mark, TPI_BUFLEN_64  , "%s", remark);
			}

			if(strlen(tmp_node->hostname) > 0)
			{
				memset(clients_list[j].hostname, 0, TPI_BUFLEN_64);
				snprintf(clients_list[j].hostname, TPI_BUFLEN_64, "%s", tmp_node->hostname);
			}
			clients_list[j].l2type =  tmp_node->flag;
			clients_list[j].time =  tmp_node->time;
			clients_list[j].interval_time =  tmp_node->interval_time;
			j++;
			tmp_node = tmp_node->next;
		}
	}

	return j;
}

static void getParentControlClientConfig(struct client_info *clients_list, int client_num)
{
	char *mib_val;
	int i = 0, j = 0, client_mib_count = 0;
	char hostname[TPI_BUFLEN_64] = {0};
	char mark[TPI_BUFLEN_64] = {0};
	char ip[TPI_IP_STRING_LEN] = {0};
	char mac[TPI_MAC_STRING_LEN] = {0};
	char limitenable[8] = {0};

	_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_CLIENTNUM, mib_val);
	client_mib_count = atoi(mib_val);

	for(i = 0; i < client_mib_count; i++)
	{
		_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i + 1), mib_val); //hostname \t ip \t limitEn
		if(strcmp(mib_val, "") == 0)
			continue;
		
		memset(hostname, 0x0, sizeof(hostname));
		memset(mark, 0x0, sizeof(mark));
		memset(ip, 0x0, sizeof(ip));
		memset(mac, 0x0, sizeof(mac));
		memset(limitenable, 0x0, sizeof(limitenable));
		if(parsePerParentControlClientConfig(mib_val, hostname, mark , mac, ip, limitenable) == -1)
			continue;

		for (j = 0; j < client_num; j++)
		{
			if(1 == clients_list[j].in_use  &&  !strncasecmp(mac, clients_list[j].mac, strlen(mac)))
			{
				if(strcmp(limitenable, "true") == 0)
					clients_list[j].limitenable =  1;
				break;
			}
		}
	}
	return ;
}

int get_all_client_info( struct client_info * clients_list,  int max_client_num)
{
	int i;
	int client_num = 0;
	struct client_info * tmp;

	//init
	tmp = clients_list;
	for(i = 0; i < max_client_num; i++)
	{
		tmp->in_use = 0;
		tmp++;
	}

	client_num = updateParentControlClientInfo(clients_list, max_client_num);

	getParentControlClientConfig(clients_list, client_num);

	return client_num;

}

RET_INFO cgi_lib_get_parent_online_list(webs_t wp, cJSON *root, void *info)
{
	int i = 0, client_num = 0;
	unsigned int type = 0;
	time_t now_time = time(0);
	struct client_info * clients_list ;
	cJSON *obj = NULL;
	unsigned char cur_hostname[TPI_BUFLEN_64] = {0};     //hostname

	clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list == NULL )
	{
		return RET_ERR;		//根据coverity分析结果修改 原来存在问题:无返回值  2017/1/11 F9项目修改
	}
	memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);

	client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);



	for(i = 0; i < client_num; i++)
	{

		if( get_macfilter_mode() != MODE_PASS && is_mac_filter(clients_list[i].mac,"deny") != -1 )
			continue;
		if(clients_list[i].time == 0)	
			continue;

		type = tenda_arp_ip_to_flag(inet_addr(clients_list[i].ip));
#ifdef __CONFIG_GUEST__
		if(type == 3 || type == 4)
			continue;
#endif
		if(type == 1 || type == 2)
		{
			if(0 == tenda_arp_is_wireless_client(ether_aton(clients_list[i].mac)))
				continue;
		}
		cJSON_AddItemToArray(root, obj = cJSON_CreateObject());
		if(strlen(clients_list[i].hostname) > 0)
		{
			/*没有进行编码格式的判断，导致设备名称出现乱码*/
				memset(cur_hostname,0x0,TPI_BUFLEN_64);
				
			#ifdef __CONFIG_SUPPORT_GB2312__
				if (1 == is_cn_encode(clients_list[i].hostname))
				{
					set_cn_ssid_encode("utf-8", clients_list[i].hostname, cur_hostname);
					strcpy(clients_list[i].hostname, cur_hostname);
				}
			#else
				if(1 == is_gb2312_code(clients_list[i].hostname))
				{
					strcpy(cur_hostname, "Unknown") ;
				}
			#endif
				else
				{
					strcpy(cur_hostname,clients_list[i].hostname);
				}
					
			cJSON_AddStringToObject(obj, "parentCtrlHostname", cur_hostname);
		}
		else
		cJSON_AddStringToObject(obj, "parentCtrlHostname", "Unknown");
		cJSON_AddStringToObject(obj, "parentCtrlRemark", clients_list[i].mark);
		cJSON_AddStringToObject(obj, "parentCtrlMAC", clients_list[i].mac);
		cJSON_AddStringToObject(obj, "parentCtrlIP", clients_list[i].ip);
		cJSON_AddNumberToObject(obj, "parentCtrlConnectTime", now_time - clients_list[i].time);
		if(clients_list[i].limitenable == 1)
			cJSON_AddStringToObject(obj, "parentCtrlEn", "true");
		else
			cJSON_AddStringToObject(obj, "parentCtrlEn", "false");
		
	}

	free(clients_list);
	clients_list = NULL;
	return RET_SUC;

}

RET_INFO cgi_lib_get_parentAccessCtrl(webs_t wp, cJSON *root, void *info)
{
	cJSON *array = NULL;
	cJSON_AddStringToObject(root, "parentCtrlOnlineTime", nvram_safe_get(ADVANCE_PARENT_CONTROL_TIME));
	cJSON_AddStringToObject(root, "parentCtrlOnlineDate", nvram_safe_get(ADVANCE_PARENT_CONTROL_DATE));
	cJSON_AddStringToObject(root, "parentCtrlURLFilterMode", nvram_safe_get(ADVANCE_PARENT_CONTROL_URLFILTER_MODE));
	cJSON_AddItemToObject(root, "parentCtrlURL", array = cJSON_CreateArray());

	getParentControlUrlFilterList2web(array);

	return RET_SUC;
}

struct Parent_ctl_param
{
	char param[256];
	struct Parent_ctl_param* next;
};

/*****************************************************************************
 函 数 名  : is_parent_ctl
 功能描述  : 判断是否在家长控制的配置参数中
 输入参数  : 无
 输出参数  : 无
 返 回 值  : inline int

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int is_parent_ctl(char* mac)
{
	char old_hostname[TPI_BUFLEN_64] = {0};
	char old_mark[TPI_BUFLEN_64] = {0};
	char old_ip[TPI_IP_STRING_LEN] = {0};
	char old_mac[TPI_MAC_STRING_LEN] = {0};
	char old_limitenable[8] = {0};
	char *value = NULL;
	int i =1;
	for(i = 1; i <= MAX_PARENTCTL_NUM; i++)
	{
		_GET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i), value);
		if (!strcmp(value, "") || strlen(value) < 8)
			continue;
		if(parsePerParentControlClientConfig(value, old_hostname, old_mark, old_mac, old_ip, old_limitenable) != 0)
			return -1;
		if(strcmp(old_mac, mac) == 0)
		{
			_SET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i), "");
			return i;
		}
	}
	return -1;
}
/*****************************************************************************
 函 数 名  : set_parent_ctl_param
 功能描述  : 设置家长控制的参数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
struct Parent_ctl_param* parent_ctl_head = NULL;
void set_parent_ctl_param(char* parent_ctl_config)
{
	char hostname[TPI_BUFLEN_64] = {0};
	char mark[TPI_BUFLEN_64] = {0};
	char ip[TPI_IP_STRING_LEN] = {0};
	char mac[TPI_MAC_STRING_LEN] = {0};
	char limitenable[8] = {0};
	int index = 0;

	char* value = NULL;
	if(parsePerParentControlClientConfig(parent_ctl_config, hostname, mark, mac, ip, limitenable) != 0)
		return -1;

	qosMacToLower(mac);
	if(strlen(mark) > 0)
	{
		if( add_remark(mac, mark) != 0)
			return -1;
	}
	else if(get_remark(mac) != NULL)
	{
		add_remark(mac, hostname);
	}

	index = is_parent_ctl(mac);
	if(index != -1)
	{
		_SET_VALUE(ADVANCE_PARENT_CONTROL_RULE(index), "");
	}

	if(strcmp("false" , limitenable ) != 0)
	{
		struct Parent_ctl_param* temp = (struct Parent_ctl_param*)malloc(sizeof(struct Parent_ctl_param));
		memset(temp,0x0,sizeof(struct Parent_ctl_param));
		strncpy(temp->param, parent_ctl_config,strlen(parent_ctl_config));
		printf("%s\n",temp->param);
		printf("%s\n",parent_ctl_config);
		temp->next = parent_ctl_head;
		parent_ctl_head = temp;
	}
}

/*****************************************************************************
 函 数 名  : reversed_parent_ctl_node
 功能描述  : 执行逆序操纵
 输入参数  : 无
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
inline int  reversed_parent_ctl_node(struct Parent_ctl_param** head)
{
	struct Parent_ctl_param* cur_node = NULL;
	struct Parent_ctl_param* temp_node = NULL;
	struct Parent_ctl_param* new_head = NULL;
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
 函 数 名  : free_parent_ctl_node
 功能描述  : 清空申请的内存
 输入参数  : 无
 输出参数  : 无
 返 回 值  : inline void

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
inline void free_parent_ctl_node(struct Parent_ctl_param** head)
{
	struct Parent_ctl_param* temp = NULL;
	struct Parent_ctl_param* cur_node = NULL;
	cur_node = *head;
	while(cur_node)
	{
		temp = cur_node;
		cur_node = temp->next;
		free(temp);
		temp = NULL;
	}
	*head = NULL;
}
/*****************************************************************************
 函 数 名  : set_parent_ctl_nvram
 功能描述  : 将家长控制的规则设置到nvram中
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年10月10日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int  set_parent_ctl_nvram(   )
{
	int i = 1;
	int j = 1;
	char *value = NULL;
	char cur_rule_value[256] = {0};
	char nums[6] ={0};

	/*将原有的所有配置都向最前移动*/
	for (i = 1; i <= MAX_PARENTCTL_NUM; ++i)
	{

		_GET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i), value);
		memset(cur_rule_value, 0x0, 256);
		strncpy(cur_rule_value, value, strlen(value));

		if (!strcmp(value, "") ||  strlen(value) <= 8)
			continue;

		_SET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i), "");
		_SET_VALUE(ADVANCE_PARENT_CONTROL_RULE(j), cur_rule_value);
		j++;
	}
	/**********************************************/

	/******************将链表逆序************/
	reversed_parent_ctl_node(&parent_ctl_head);
	/*********************************************/

	struct Parent_ctl_param* cur_node = NULL;
	i = j;

	cur_node = parent_ctl_head;
	while(cur_node != NULL)
	{
		_SET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i), cur_node->param);
		i ++;
		cur_node = cur_node->next;
	}
	/*------------------------------------------------------------------------*/
	j = i;
	if(i  >  (MAX_PARENTCTL_NUM + 1))
	{
		sprintf(nums,"%d",MAX_PARENTCTL_NUM);
		_SET_VALUE(ADVANCE_PARENT_CONTROL_CLIENTNUM,nums);
		int offset = i - MAX_PARENTCTL_NUM -1;

		for (i = 1; i <= MAX_PARENTCTL_NUM; ++i)
		{
			_GET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i + offset), value);
			memset(cur_rule_value, 0x0, 256);
			strncpy(cur_rule_value, value, strlen(value));
			_SET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i), cur_rule_value);
		}
		for(i = 0; i < offset; i++)
		{
			nvram_unset(ADVANCE_PARENT_CONTROL_RULE(i + MAX_PARENTCTL_NUM + 1));
		}

	}
	else
	{
		sprintf(nums,"%d",j -1);
		_SET_VALUE(ADVANCE_PARENT_CONTROL_CLIENTNUM, nums);
	}

	/********free 掉申请的内存*******/
	free_parent_ctl_node(&parent_ctl_head);
	return j;


}

RET_INFO cgi_lib_set_parent_onlineList(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	int ret = 0;
	int  i = 0;
	char *sp, *ep;
	char *onlineList = NULL;
	onlineList = cgi_lib_get_var(wp,root, T("onlineList"), T(""));

	if(NULL == onlineList)
		return -1;

	if(strlen(onlineList) < 28)
	{
		_SET_VALUE(ADVANCE_PARENT_CONTROL_CLIENTNUM, "0");
		return -1;
	}

	sp = onlineList;
	while((ep = strchr(sp, '\n')))
	{
		*ep++ = '\0';
		set_parent_ctl_param(sp);
		
		sp = ep;
	}
	set_parent_ctl_param(sp);

	ret = set_parent_ctl_nvram();
	if(ret)
		sprintf(err_code,"0");
	else
		sprintf(err_code,"1");
	
	CGI_MSG_MODULE msg_tmp;
	msg_tmp.id = RC_FIREWALL_MODULE;
	sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
	add_msg_to_list(msg,&msg_tmp);
	return RET_SUC;
	
}

RET_INFO cgi_lib_set_parent_access_ctrl(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
	char *onlineList, *onlineTime, *onlineDate, *urlFilterMode, *urlList;
	char msg_param[PI_BUFLEN_256] = {0};
	onlineTime = cgi_lib_get_var(wp,root, T("parentCtrlOnlineTime"), T(""));
	onlineDate = cgi_lib_get_var(wp,root, T("parentCtrlOnlineDate"), T("11111111"));
	urlFilterMode = cgi_lib_get_var(wp,root, T("parentCtrlURLFilterMode"), T("disable"));
	urlList = cgi_lib_get_var(wp,root, T("urlList"), T(""));
	_SET_VALUE(ADVANCE_PARENT_CONTROL_TIME, onlineTime);
	_SET_VALUE(ADVANCE_PARENT_CONTROL_DATE, onlineDate);
	_SET_VALUE(ADVANCE_PARENT_CONTROL_URLFILTER_MODE, urlFilterMode);
	_SET_VALUE(ADVANCE_PARENT_CONTROL_URLFILTER_LIST, urlList);
	CGI_MSG_MODULE msg_tmp;
	msg_tmp.id = RC_FIREWALL_MODULE;
	sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
	add_msg_to_list(msg,&msg_tmp);
	return RET_SUC;
}

