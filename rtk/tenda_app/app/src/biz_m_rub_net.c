#include "cgi_module_interface.h"
#include "biz_m_rub_net.h"

#define RUB_NET_BLACKLIST_MAX 10 /* 定义一个黑名单的最大值，比如10条 */

/*************************************************************************
  功能: 实现添加黑名单
  参数: mac用于将设备添加到黑名单
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
static int biz_m_rub_net_add_blacklist(const char *mac)
{	
	int ret = 0;
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	if(NULL == mac)
	{
		printf("[%s][%d]mac is null!\n",__func__,__LINE__);
		return 1;
	}
	
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_RUB_NET_ADD_BLACKLIST));
	cJSON_AddStringToObject(cj_set,"mac",mac);
	
	ret = app_set_module_param(cj_query,cj_set);

	
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
	
	return ret;
}

/*************************************************************************
  功能: 实现删除黑名单
  参数: mac用于将设备从黑名单中删除
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
static int biz_m_rub_net_delete_blacklist(const char *mac)
{	
	int ret = 0;
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	if(NULL == mac)
	{
		printf("[%s][%d]mac is null!\n",__func__,__LINE__);
		return 1;
	}
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_RUB_NET_DEL_BLACKLIST));

	cJSON_AddStringToObject(cj_set,"mac",mac);
	
	ret = app_set_module_param(cj_query,cj_set);
	
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
	
	return ret;
}
/*************************************************************************
  功能: 实现关闭上线提醒
  参数: mac用于给设备添加信任，信任的设备上线就不提醒
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
static int biz_m_rub_net_add_to_trustlist(const char *mac)
{		
	int ret = 0;
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	if(NULL == mac)
	{
		printf("[%s][%d]mac is null!\n",__func__,__LINE__);
		return 1;
	}
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_RUB_NET_ADD_TO_TRUSTLIST));

	cJSON_AddStringToObject(cj_set,"mac",mac);

	ret = app_set_module_param(cj_query,cj_set);

	
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
	
	return ret;	
}


/*************************************************************************
  功能: 实现开启上线提醒
  参数: mac用于将设备从信任中删除，不信任的设备上线后就会提醒
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
static int biz_m_rub_net_delete_from_trustlist(const char *mac)
{
	int ret = 0;
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	if(NULL == mac)
	{
		printf("[%s][%d]mac is null!\n",__func__,__LINE__);
		return 1;
	}
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_RUB_NET_DEL_TO_TRUSTLIST));

	cJSON_AddStringToObject(cj_set,"mac",mac);
	
	ret = app_set_module_param(cj_query,cj_set);

	
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);

	return ret;
}

/*************************************************************************
  功能: 实现添加黑名单、删除黑名单、添加到信任列表、删除信任列表功能
  参数: 需要设置access_user_t中的数据到路由器
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_access_user_set_cb( const api_cmd_t *cmd,
									     access_user_t *user_info,
										 void *privdata)
{
	int ret = 0;
	//根据coverity分析结果修改，原来为无效的判断:if (user_info && user_info->mac)  2017/1/11 F9项目修改
	if (user_info != NULL) {
		switch (user_info->op) {
			case RUB_NET_OPT_ADD_TO_BLACKLIST:	/*加入黑名单*/
				ret = biz_m_rub_net_add_blacklist(user_info->mac);
			    break;
			case RUB_NET_OPT_RM_FROM_BLACKLIST: /*从黑名单中删除*/
				ret = biz_m_rub_net_delete_blacklist(user_info->mac);
				break;
				
			case RUB_NET_OPT_ADD_TO_TRUSTLIST: /* 关闭上线提醒 */
				ret = biz_m_rub_net_add_to_trustlist(user_info->mac);
				break;
				
			case RUB_NET_OPT_RM_FROM_TRUSTLIST: /* 开启上线提醒 */
				ret = biz_m_rub_net_delete_from_trustlist(user_info->mac);
				break;
				
			default:
				ret = -1;
				break;
		}
	}

	return ret;
}
/*************************************************************************
  功能: 实现获取黑名单功能
  参数: 需要将数据填入black_list结构，这里涉及到动态内存申请
  返回值: 0-成功，1-失败，，80-超过长度限制
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_rub_net_black_list_get_cb(
									const api_cmd_t *cmd,
									black_list_t **black_list, 
#ifdef CONFIG_APP_COSTDOWN
									int head_keep_size,
#endif
									void *privdata)
{
	int ret = 0;
    int i = 0;
    int count = 0;
	char *p_a_mem = NULL;
    cJSON *black_listInfo = NULL;
    cJSON *child_obj = NULL;
	
	cJSON *cj_get	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	cJSON *extern_data = NULL;
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_RUB_NET_BLACK_LIST));

    ret = app_get_module_param(cj_query,cj_get);

	if (black_list)
	{
		/*获取规则*/	
		black_listInfo = cJSON_GetObjectItem(cj_get,"black_list_info");
		count = cJSON_GetArraySize(black_listInfo);
	
		/* 分配最大的黑名单数量的空间 */
#ifdef CONFIG_APP_COSTDOWN
		p_a_mem = (char *)malloc(head_keep_size + sizeof(black_list_t)+count*sizeof(black_user_t)); 
		if (!p_a_mem) 
#else
		*black_list = (black_list_t *)malloc(sizeof(black_list_t)+count*sizeof(black_user_t)); 
		if (!(*black_list)) 
#endif
		{
		    printf("black_list alloc mem is fail\n");
			*black_list= NULL;
			ret = 1; /* 内部实现错误 */
			goto out;
		}
#ifdef CONFIG_APP_COSTDOWN
		*black_list = p_a_mem + head_keep_size;
#endif
		memset(*black_list, 0, sizeof(black_list_t)+count*sizeof(black_user_t));
		
        black_user_t *black_user_list = (*black_list)->buser;

		(*black_list)->n_mac = count; /*黑名总数*/
		
		for(i = 0; i < count; i++)
		{		
		    child_obj = cJSON_GetArrayItem(black_listInfo,i);

			strcpy(black_user_list[i].mac,cjson_get_value(child_obj, "mac",""));  /*mac*/		
			strcpy(black_user_list[i].dev_name,cjson_get_value(child_obj, "dev_name",""));  /*dev_name*/

			//printf("====>>>>black_list_mac:%s,black_list_dev_name:%s\n",black_user_list[i].mac,black_user_list[i].dev_name);
		}	
	}

out:
	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);

	return ret;
}

/*************************************************************************
  功能: 获取刚上线的陌生主机信息
  参数: info 将陌生主机信息填入到该结构体中
  返回值: 0-成功，1失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_strange_host_info_get(rub_strange_host_info_t **rub_info,int *n_host)
{
	int ret = 0;
	cJSON *cj_get	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	cJSON *extern_data = NULL;
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_STRANGE_INFO));

    ret = app_get_module_param(cj_query,cj_get);

	int i = 0;
	int count = 0;
    int len = 0;

	cJSON *strange_host_info = NULL;
	cJSON *child_obj = NULL;	

	strange_host_info = cJSON_GetObjectItem(cj_get,"strang_host_info");
	if(NULL == strange_host_info)
	{
		cJSON_Delete(cj_get);
		cJSON_Delete(cj_query);
		return 1;
	}
	count = cJSON_GetArraySize(strange_host_info); /*陌生主机总数*/
	
	if(count > 0)
	{
		len = count * sizeof(rub_strange_host_info_t);
		*rub_info= (rub_strange_host_info_t *)malloc(len);	
		memset(*rub_info, 0, len);

		*n_host = count;
		for(i = 0; i < count; i++)
	    {	
	    	child_obj = cJSON_GetArrayItem(strange_host_info,i);

			strcpy((*rub_info)[i].dev_name,cjson_get_value(child_obj, "strange_name",""));  /*mac*/	
			strcpy((*rub_info)[i].mac,cjson_get_value(child_obj, "strange_mac",""));  /*mac*/		
			strcpy((*rub_info)[i].date,cjson_get_value(child_obj, "strange_date",""));  /*date*/
			strcpy((*rub_info)[i].time,cjson_get_value(child_obj, "strange_time",""));  /*time*/
			strcpy((*rub_info)[i].ssid,cjson_get_value(child_obj, "strange_ssid","")); /*ssid*/
			
			(*rub_info)[i].type = cjson_get_number(child_obj, "strange_type",0);
			
			strcpy((*rub_info)[i].serial_num,cjson_get_value(child_obj, "uc_serialnum",""));
			
			(*rub_info)[i].dtype = cjson_get_number(child_obj, "strange_dtype",0);
		}	
	}

	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	
	return ret;
}

/*************************************************************************
  功能: 获取黑白名单模式
  参数: mode->enable (mac 过滤模式是否打开，1-开启，0-未开启)
  	mode->mac_mode	(当前生效的黑白名单模式，1-黑名单，2-白名单)
  	mode->supt_mac_mode (路由器支持的模式，1-仅支持黑名单，2-仅支持白名单，3-黑白名单都支持)
  返回值: 0-成功，1失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_rub_net_mf_mode_get_cb(mac_filter_mode_t * mode, 
											  void * privdata)
{
	cJSON *cj_query = NULL;
	cJSON *cj_get = NULL;
	cJSON *module = NULL;
	int ret = 0;

	if (mode == NULL) {
		return -1;
	}

	cj_query = cJSON_CreateObject();
	if(NULL == cj_query)
	{
		printf("line:%d create fail\n",__LINE__);
		return 1;
	}
	
	cj_get = cJSON_CreateObject();
	if(NULL == cj_get)
	{
		cJSON_Delete(cj_query);
		printf("line:%d create fail\n",__LINE__);
		return 1;
	}
	
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_MAC_FILTER_MODE));

	ret = app_get_module_param(cj_query,cj_get);	//调用获取的公共函数

	mode->enable = cjson_get_number(cj_get,"enable",1);
	SET_MACFILTER_ENABLE(mode);

	mode->mac_mode = cjson_get_number(cj_get,"mac_mode",1); /* 当前生效的黑白名单模式，1-黑名单，2-白名单 */
	SET_MACFILTER_MODE(mode);

	
	mode->supt_mac_mode = cjson_get_number(cj_get,"supt_mac_mode",1); /* 路由器支持的模式，1-仅支持黑名单，2-仅支持白名单，3-黑白名单都支持 */
	SET_MACFILTER_SUPT_MODE(mode);

	cJSON_Delete(cj_query);
	cJSON_Delete(cj_get);
	return 0;
}

/*************************************************************************
  功能: 设置黑白名单模式
  参数: mode->mac_mode	(当前生效的黑白名单模式，1-黑名单，2-白名单)
  返回值: 0-成功，1失败
  是否需要用户实现: 是
 ************************************************************************/

int biz_m_rub_net_mf_mode_set_cb(mac_filter_mode_t * mode,
											  void * privdata)
{
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	int ret = 0;

	if (mode == NULL) {
		return -1;
	}

	cj_query = cJSON_CreateObject();
	if(NULL == cj_query)
	{
		printf("create cj_query fail\n");
		return 1;
	}
	cj_set = cJSON_CreateObject();
	if(NULL == cj_set)
	{
		printf("create cj_set fail\n");
		cJSON_Delete(cj_query);
		return 1;
	}
	
	if (HAS_MACFILTER_ENABLE(mode))  //后台暂时不支持这个字段，什么也不传
	{
		
	}
	if (HAS_MACFILTER_MODE(mode)) 
	{
		if((2 == mode->mac_mode) || (1 == mode->mac_mode))
		{
			cJSON_AddNumberToObject(cj_set,"mac_mode",mode->mac_mode);
		}
		else
		{
			cJSON_Delete(cj_query);
			cJSON_Delete(cj_set);
			return 1;
		}
	}

	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_MAC_FILTER_MODE));
	

    ret = app_set_module_param(cj_query,cj_set);

	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);
	
	return 0;
}

/*************************************************************************
  功能: 清空黑名单
  参数: 无
  返回值: 0-成功，1失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_rub_net_black_list_flush_cb(void * privdata)
{
	int ret = 0;
	cJSON *module = NULL;
	cJSON *cj_query = NULL;
	cJSON *cj_set = NULL;

	cj_query = cJSON_CreateObject();
	if(NULL == cj_query)
	{
		printf("line[%d] create cj_query fail\n",__LINE__);
		return 1;
	}
	cj_set = cJSON_CreateObject();
	if(NULL == cj_set)
	{
		cJSON_Delete(cj_query);
		printf("line[%d] create cj_set fail\n",__LINE__);
		return 1;
	}

	cJSON_AddItemToObject(cj_query,"module",module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_RUB_NET_FLUSH_BLACKLIST));

	ret = app_set_module_param(cj_query,cj_set);
	
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);
	/* 清空所有黑名单中的设备 */
	return 0;
}

