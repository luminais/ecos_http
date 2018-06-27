
#include "unistd.h"
#include <netinet/in.h>
#include <net/if.h>
#include "extra.h"
#include "list.h"
#include "cJSON.h"
#include "elink_common.h"
#include "proto.h"

struct soaming_sta
{
	struct list_head list;
	char mac[18];
	unsigned long long expire_time;//老化时间
	unsigned long long time; //在线时长
	int    rssi;
	int    enable;                  //在列表中但是不上报，30秒内重新链接上
};

struct roaming_ctx roaming_config;
static struct list_head roaming_sta_list = LIST_HEAD_INIT(roaming_sta_list);	
static struct list_head roaming_del_sta_list = LIST_HEAD_INIT(roaming_del_sta_list);	

void _remote_roaming_cb(struct roaming_timeout *timeout);


struct roaming_timeout remote_roaming_timer = { .cb = _remote_roaming_cb };

int update_roaming_sta_list(struct list_head *head)
{
	int find_sta = 0;
	unsigned long long  current_time = 0;
	struct soaming_sta *tmp = NULL;
	struct soaming_sta *node = NULL;
	struct soaming_sta *next_node = NULL;

	current_time = cyg_current_time();
	
	list_for_each_entry_safe(node, next_node,head, list)
	{
		if(node->expire_time < current_time){
			list_del(&node->list);
			free(node);
		}
	}
}

int clear_roaming_sta_list(struct list_head *head)
{
	int find_sta = 0;
	unsigned long long  current_time = 0;
	struct soaming_sta *tmp = NULL;
	struct soaming_sta *node = NULL;
	struct soaming_sta *next_node = NULL;

	current_time = cyg_current_time();
	
	list_for_each_entry_safe(node, next_node,head, list)
	{
		if(node){
			list_del(&node->list);
			free(node);
		}
	}
}

int find_mac_in_roaming_del_list(char *mac)
{
	int find_sta = 0;
	struct soaming_sta *tmp = NULL;
	list_for_each_entry(tmp, &roaming_del_sta_list, list) 
	{
		if(0 == strcmp(tmp->mac,mac))
		{
			find_sta = 1;
			break;
		}
	}
	return find_sta;
}

int add_roaming_sta_to_list(char *mac,int onlinetime,int rssi)
{
	int find_sta = 0;
	int flag_insert = 0;
	unsigned long long  current_time = 0;
	struct soaming_sta *new_node = NULL;
	struct soaming_sta *tmp = NULL;
	struct list_head *h = &roaming_sta_list;
	current_time = cyg_current_time();
	//printf("=======mac:%s   %d   %d========%s [%d]\n",mac,onlinetime,rssi,__FUNCTION__, __LINE__);
	//是否在上报列表中，在则更新超时时间和信息
	list_for_each_entry(tmp, &roaming_sta_list, list) 
	{
		if(0 == strcmp(tmp->mac,mac))
		{
			tmp->expire_time = current_time + (roaming_config.report_interval * 2);
			tmp->rssi = rssi;
			tmp->time = onlinetime;
			return 0;
		}
	}
	
	//是否在del列表中
	list_for_each_entry(tmp, &roaming_del_sta_list, list) 
	{
		if(0 == strcmp(tmp->mac,mac))
		{
			find_sta = 1;
			break;
		}
	}
	//如果存在，则判断是否是在30秒内重新链接上路由器的
	if(find_sta == 1)
	{
		if(((current_time - onlinetime) - (tmp->expire_time - (roaming_config.report_interval * 2))) < 30)	
		{
			flag_insert  = 1;
		}
		//如果查找到，则同时将该节点从dellist中移除
		list_del(&tmp->list);
		free(tmp);
	}
	
	if(onlinetime > roaming_config.start_time || rssi > roaming_config.start_rssi || flag_insert == 1)
	{
		new_node = (struct soaming_sta*)malloc(sizeof(struct soaming_sta));
		new_node->enable = flag_insert ? 0 : 1;
		new_node->expire_time = current_time + (roaming_config.report_interval * 2);
		new_node->rssi = rssi;
		new_node->time = onlinetime;
		strcpy(new_node->mac,mac);
		list_add_tail(&new_node->list, h);
	}
}

int remove_roaming_sta_to_list(char *mac)
{
	struct soaming_sta *node = NULL;
	struct soaming_sta *next_node = NULL;
	
	list_for_each_entry_safe(node, next_node,&roaming_sta_list, list)
	{
		if(0 == strcmp(mac,node->mac)){
			list_del(&node->list);
			free(node);
			break;
		}
	}
}


int add_roaming_sta_to_del_list(char *mac)
{
	int find_sta = 0;
	unsigned long long  current_time = 0;
	struct soaming_sta *new_node = NULL;
	struct soaming_sta *tmp = NULL;
	current_time = cyg_current_time();
	//是否在del_list列表中，在则更新超时时间和信息
	list_for_each_entry(tmp, &roaming_del_sta_list, list) 
	{
		if(0 == strcmp(tmp->mac,mac))
		{
			tmp->expire_time = current_time + (roaming_config.report_interval * 2);
			find_sta = 1;
			break;
		}
	}
	
	remove_roaming_sta_to_list(mac);

	if(!find_sta)
	{
		new_node = (struct soaming_sta*)malloc(sizeof(struct soaming_sta));
		new_node->expire_time = current_time + (roaming_config.report_interval * 2);
		memcpy(new_node->mac,mac,6);
		list_add_tail(&new_node->list,&roaming_del_sta_list);
	}
}

int update_sta_info()
{
	int i = 0;
	char mac[32] = {0};
	char value[32] = {0};
	int online_time = 0;
	int rssi = 0;
	int array_num = 0;
	cJSON *tmp = NULL;
	cJSON *tmp_value = NULL;
	cJSON *clinet_list = NULL;
	
	update_roaming_sta_list(&roaming_sta_list);
	update_roaming_sta_list(&roaming_del_sta_list);
	
	clinet_list = cJSON_CreateArray();
	ap_mode_online_list(clinet_list);

	array_num = cJSON_GetArraySize(clinet_list);

	for(i = 0;i <array_num;i++)
	{
		tmp = cJSON_GetArrayItem(clinet_list,i);

		tmp_value = cJSON_GetObjectItem(tmp, "mac");
		strcpy(mac,tmp_value->valuestring);
		tmp_value = cJSON_GetObjectItem(tmp, "onlineTime");
		online_time = atoi(tmp_value->valuestring);
		tmp_value = cJSON_GetObjectItem(tmp, "rssi");
		rssi = atoi(tmp_value->valuestring);
		add_roaming_sta_to_list(mac,online_time,rssi);
	}
	
}

int make_upload_sta_info_package(cJSON*array)
{
	cJSON *item = NULL;
	struct soaming_sta *node = NULL;
	
	list_for_each_entry(node,&roaming_sta_list, list)
	{
		if(node->rssi < roaming_config.threshold_rssi && node->enable)
		{
			cJSON_AddItemToArray(array, item = cJSON_CreateObject());
			cJSON_AddStringToObject(item, "mac",node->mac);
			cJSON_AddNumberToObject(item, "connect_time",node->time);
			cJSON_AddNumberToObject(item, "rssi",node->rssi);
		}
	}
}

void upload_roaming_message_to_remote()
{
	cJSON	*root = NULL;
	cJSON	*array = NULL;
	cJSON *send_root = NULL;
	char *pr = NULL;
	send_root = cJSON_CreateObject();

	cJSON_AddItemToObject(send_root,"roaming_report",array = cJSON_CreateArray());
	
	make_upload_sta_info_package(array);
	pr = cJSON_Print(send_root);
	printf("=======%s========%s [%d]\n", pr,__FUNCTION__, __LINE__);
	if(pr)
		free(pr);
	//目前elinksdk中没有上报接口，所以该功能目前不生效
	send_roaming_report(send_root);
	//return_Parameter_to_remote(send_root,ID);

	if(send_root)
		free(send_root);
}
void _remote_roaming_cb(struct roaming_timeout *timeout)
{
	update_sta_info();
	upload_roaming_message_to_remote();
	register_roaming_process(&remote_roaming_timer, roaming_config.report_interval);
}

void restart_roaming()
{
	roaming_process_canle(&remote_roaming_timer);
	updata_roaming_info(&roaming_config);
	clear_roaming_sta_list(&roaming_sta_list);
	clear_roaming_sta_list(&roaming_del_sta_list);
	if(roaming_config.roaming_enable)
	{
		update_sta_info();
		register_roaming_process(&remote_roaming_timer, roaming_config.report_interval);
	}
}


