#include "cgi_module_interface.h"
#include "biz_m_ucloud_info.h"

/*************************************************************************
  功能: 设置云管理sn序列号
  参数: data  sn序列号指针
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_ucloud_info_sn_set_cb(api_cmd_t *cmd, 
						   					unsigned int data_len,
						   					void *data,
						   					void *privdata)
{
	int ret = 0;
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	if (!data) 
	{
		printf("[%s][%d]data is null!\n",__func__,__LINE__);
		return 1;
	}
    char *sn = (char *)data;
	
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_UCLOUD_INFO_SN));

	cJSON_AddStringToObject(cj_set, "uc_serialnum",sn);
	
	
	ret = app_set_module_param(cj_query,cj_set);
	
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
	
	return ret;
}
/*************************************************************************
  功能: 获取云管理基本信息
  参数: data  填入云管理基本信息
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_ucloud_info_basic_get(cloud_basic_info_t *data)
{
	int ret = 0;
	cJSON *cj_get	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	
    if (!data)
	{
		return 1;
	}
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_UCLOUD_BASIC_INFO));
	
	ret = app_get_module_param(cj_query,cj_get);
    /* 云管理是否开启 1-开，0-关 */
	data->enable = cjson_get_number(cj_get,"uc_manage_en",1);
	/* 云管理sn序列号 */
	strncpy(&data->sn[0], cjson_get_value(cj_get,"uc_serialnum",""), sizeof(data->sn));
	/*mac 去除冒号后的mac*/
	strncpy(&data->mac[0], cjson_get_value(cj_get,"host_mac",""), sizeof(data->mac));
	/*软件版本号*/
	strncpy(&data->ver[0],cjson_get_value(cj_get,"version",""), sizeof(data->ver));
	/*公司*/
	strncpy(&data->company[0], cjson_get_value(cj_get,"company","Tenda"), sizeof(data->company));
	/*/型号*/
	strncpy(&data->product[0], 	cjson_get_value(cj_get,"product","F9"), sizeof(data->product));
	//镜像密码 编译时 通过脚本生成随机密码
	strncpy(&data->ver_passwd[0], cjson_get_value(cj_get,"update_random",""), sizeof(data->ver_passwd)); 
	//strcpy(&data->ver_passwd[0], "cpoej5");
	
	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	
	return ret;  
}

/*************************************************************************
  功能: 获取云管理开关
  参数: manage_en 填入云管理开关状态
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_ucloud_info_manage_en_get_cb(
						   const api_cmd_t *cmd,
						   m_cloud_info_manage_en_t *manage_en,
						   void *privdata)
{

	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	int uc_enable = 1;//云管理默认开启
	
	if (!manage_en) 
	{
		printf("[%s][%d]manage_en is null!\n",__func__,__LINE__);
		return 1;	
	} 

	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_UCLOUD_INFO_MESSAGE_EN));
	
	ret = app_get_module_param(cj_query,cj_get);

	uc_enable = cjson_get_number(cj_get,"uc_manage_en",1);
	manage_en->en = uc_enable;
	
	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	return ret;
}

/*************************************************************************
  功能: 设置云管理开关
  参数: manage_en 用于设置云管理开关状态
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_ucloud_info_manage_en_set_cb(
							   const api_cmd_t *cmd,
							   m_cloud_info_manage_en_t *manage_en,
							   void *privdata)
{
	int ret = 0;
	cJSON *cj_set	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	
	if (!manage_en) 
	{
		return 1;	
	} 

	int en = !! manage_en->en;
	char buf[8] = {0};
	snprintf(buf, 8, "%d", en);
	
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_UCLOUD_INFO_MESSAGE_EN));

	cJSON_AddStringToObject(cj_set, "uc_manage_en",buf);
	
	
	ret = app_set_module_param(cj_query,cj_set);
	
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
	return ret;
}

/*************************************************************************
  功能: 设置判断是否需要清空云账号
  参数: 忽略
  返回值: 0-成功，-1-失败
  是否需要用户实现: 否
 ************************************************************************/
static int biz_ucloud_info_send_clear_acc(int already_exec)
{
	if (!already_exec)
	{
		if (-1 == uc_api_m_cloud_info_send_clear_account()) 
		{
			printf("[%s][%d]send clear account failure!\n",__func__,__LINE__);
			return -1;
		}
	}
	return 0;
}

/*************************************************************************
  功能: 设置判断是否需要清空云账号
  参数: 忽略
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
#ifdef CONFIG_APP_COSTDOWN
int biz_ucloud_info_try_clear_acc(int *need_clear)
#else
int biz_ucloud_info_try_clear_acc()
#endif
{
	int ret = 0;
	char value[32] = {0};
	int clear;
	
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_UCLOUD_INFO_TRY_CLEAR));
	
	ret = app_get_module_param(cj_query,cj_get);

	strncpy(value, cjson_get_value(cj_get,"ucloud_need_clear_acc",""), sizeof(value));
	clear = !!atoi(value);
	
	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
#ifdef CONFIG_APP_COSTDOWN
	if (need_clear)
		*need_clear = !clear;
	return ret;
#else
	return biz_ucloud_info_send_clear_acc(clear);
#endif
}

/*************************************************************************
  功能: 清空云账号后，设置云账号已清空标志
  参数: 忽略
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_cloud_info_clear_account_ack_cb (
										const api_cmd_t *cmd, 
										void *privdata)
{
	int ret = 0;
	cJSON *cj_set	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_UCLOUD_INFO_CLEAR_ACCOUNT));

	cJSON_AddStringToObject(cj_set, "ucloud_need_clear_acc","1");
	
	
	ret = app_set_module_param(cj_query,cj_set);
	
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);		
	return ret;
}


