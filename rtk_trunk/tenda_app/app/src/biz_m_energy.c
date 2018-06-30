#include "cgi_module_interface.h"

#include "biz_m_energy.h"

/*************************************************************************
   功能: 实现获取wifi定时开关的回调函数
   参数: 需要填写m_energy_wireless_timer_t
   返回值: 0成功，1失败
   是否需要用户实现: 是
*************************************************************************/
int biz_wl_timer_get_cb(const api_cmd_t *cmd, 
								  m_energy_wireless_timer_t *timer_info,
								  void *privdata)
 {
 	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	if (!timer_info) 
	{
		printf("func param is null\n");
 		return 1;
	}

	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_WIFI_TIMER));

	ret = app_get_module_param(cj_query,cj_get);
	
	timer_info->timer_status = cjson_get_number(cj_get,"timer_status",0);
	timer_info->day 		= cjson_get_number(cj_get,"timer_date",0);
	timer_info->start_time 	= cjson_get_number(cj_get,"start_time",0);
	timer_info->end_time 	= cjson_get_number(cj_get,"end_time",0);
	
	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);

	return ret;

 }


 /*************************************************************************
   功能: 实现设置wifi定时开关的回调函数
   参数: 需要将m_energy_wireless_timer_t设置到路由器
   返回值: 0成功，1失败
   是否需要用户实现: 是
  ************************************************************************/
int biz_wl_timer_set_cb(const api_cmd_t *cmd, 
								  m_energy_wireless_timer_t *timer_info,
								  void *privdata)
{
	int ret = 0;
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;

	if (!timer_info) 
	{
		printf("func param is null\n");
 		return 1;
	}
	
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_WIFI_TIMER));

	cJSON_AddNumberToObject(cj_set,"timer_status", timer_info->timer_status);
	cJSON_AddNumberToObject(cj_set,"timer_date", timer_info->day);
	cJSON_AddNumberToObject(cj_set,"start_time",timer_info->start_time);
	cJSON_AddNumberToObject(cj_set,"end_time", timer_info->end_time);

	ret = app_set_module_param(cj_query,cj_set);
	
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);
	return ret;
 }

/*************************************************************************
  功能: 实现获取led状态的回调函数
  参数: 需要填入开关状态到led_info
  返回值: 0成功，1失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_led_get_cb ( const api_cmd_t *cmd,
						  	   m_energy_led_t *led_info,
							   void *privdata)
{
	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;

	if (!led_info) {
		return 1;
	}

	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_LED));
	ret = app_get_module_param(cj_query,cj_get);
	led_info->led_status = cjson_get_number(cj_get,"led_status",1);
	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	return ret;
}

/*************************************************************************
  功能: 实现获取led状态的回调函数
  参数: 需要将led_info设置到路由器
  返回值: 0成功，1失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_led_set_cb (const api_cmd_t *cmd,
							  m_energy_led_t *led_info,
							  void *privdata)
{
	int ret = 0;
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	if (!led_info) {
		return 1;
	}
	
	cj_query = cJSON_CreateObject();
	if(NULL == cj_query)
	{
		printf("create cj_query error\n");
		return 1;
	}
	cj_set = cJSON_CreateObject();
	if(NULL == cj_set)
	{
		printf("create cj_set error\n");
		cJSON_Delete(cj_query);
		return 1;
	}
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_LED));
	cJSON_AddNumberToObject(cj_set,"led_status",led_info->led_status);

	ret = app_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);
	return ret;

}


