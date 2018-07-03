#include "cgi_module_interface.h"

#include "biz_m_login.h"


/*************************************************************************
  功能: 判断登录用户名和密码是否正确
  参数: login_info  用于判断用户名和密码的参数
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_login_login_cb(const api_cmd_t *cmd, 
 						   		  m_login_t *login_info,
 						   		  void *privdata) 
{
	char login_depw[64] = {0};
	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;

	if (!login_info) {
		printf("func param is null\n");
		return 1;
	}
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_DECODE_LOGIN_PWD));

	ret = app_get_module_param(cj_query,cj_get);
	strcpy(login_depw, cjson_get_value(cj_get, "decode_login_password", ""));
	
	if (HAS_LOGIN_USER_NAME(login_info)&& 
		HAS_LOGIN_PASSWORD(login_info)) 
	{ /* 下发了用户名和密码 */

		if(strncmp(login_info->password, login_depw, PWD_LEN) == 0 ) /* 判断输入的密码是否正确 */
		{
			cJSON_Delete(cj_query);
			cJSON_Delete(cj_get);
			return 0;
		}
	}

	cJSON_Delete(cj_query);
	cJSON_Delete(cj_get);
	return 1;
}

/*************************************************************************
  功能: 获取登录密码状态
  参数: ack_info  填入是否有登录密码
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
#ifdef CONFIG_APP_COSTDOWN
 int biz_m_login_pwd_sta_get_cb(const api_cmd_t *cmd,
 									  pwd_sta_t *sta,
 									  void *privdata)
#else
int biz_m_login_pwd_sta_get_cb(const api_cmd_t *cmd,
 									  login_common_ack_t **ack_info,
 									  void *privdata)
#endif
{
	char login_depw[64] = {0};
	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
 #ifdef CONFIG_APP_COSTDOWN
 	if (!sta) {
 #else
	if (!ack_info || !(*ack_info)) {
 #endif
		printf("func param is null\n");
		return 1;
	}
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_DECODE_LOGIN_PWD));

	ret = app_get_module_param(cj_query,cj_get);
	strcpy(login_depw, cjson_get_value(cj_get, "decode_login_password", ""));

	/* 判断登录密码是否为空 */
	if(strcmp("",login_depw) == 0)
	{
	#ifdef CONFIG_APP_COSTDOWN
		sta->is_none = 1;
	#else
		(*ack_info)->sta.is_none = 1;
	#endif
	}
	else
	{
	#ifdef CONFIG_APP_COSTDOWN
		sta->is_none = 0;
	#else
		(*ack_info)->sta.is_none = 0;
	#endif
	}

	cJSON_Delete(cj_query);
	cJSON_Delete(cj_get);
	return ret;
}

/*************************************************************************
  功能: 设置登录密码状态
  参数: update_info  用于设置登录密码
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_login_update_pwd_cb(const api_cmd_t *cmd, 
									m_login_update_pwd_t *update_info,
									void *privdata) 
{
	char old_encode[64] = {0};
	char new_encode[64] = {0};
	int ret = 1;
	/*APP传入的值是否设置了用户名，老密码和新密码*/
	if (HAS_UPDATE_USER_NAME(update_info) &&
		HAS_UPDATE_OLD_PASSWORD(update_info) &&
		HAS_UPDATE_NEW_PASSWORD(update_info)) 
	{
		base64_encode(update_info->new_password,strlen(update_info->new_password),new_encode,64);
		base64_encode(update_info->old_password,strlen(update_info->old_password),old_encode,64);

		cJSON *cj_set 	= NULL;
		cJSON *module 	= NULL;
		cJSON *cj_query 	= NULL;
		cj_query = cJSON_CreateObject();
		cj_set = cJSON_CreateObject();
		cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
		cJSON_AddItemToArray(module,cJSON_CreateString(SET_LOGIN_PWD));
		cJSON_AddStringToObject(cj_set, "newPwd",new_encode);
		cJSON_AddStringToObject(cj_set, "oldPwd",old_encode);

		ret = app_set_module_param(cj_query,cj_set);

		cJSON_Delete(cj_query);
		cJSON_Delete(cj_set);
		
	}
	return ret;
}


