#include "cgi_module_interface.h"

#include "biz_m_sys.h"
#include "biz_m_ucloud_info.h"


/*************************************************************************
  功能: 获取系统基本信息
  参数: basic 填入系统基本信息数据
  返回值: 0成功，1失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_sys_basic_info_get_cb(const api_cmd_t *cmd, 
				   						    sys_basic_info_t *basic,
				   							void *privdata)
{

	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;

	if (!basic) {
		printf("func param is null\n");
		return 1;
	}
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_SYS_BASIC_INFO));

	app_get_module_param(cj_query,cj_get);
	strcpy(basic->sn, cjson_get_value(cj_get, "sn_number", ""));  /* 获取云管理sn序列号 */
	strcpy(basic->mac, cjson_get_value(cj_get,"mac", "")); /* mac地址 */
	basic->wl_mode = 	cjson_get_number(cj_get,"wl_mode",0); /* 无线中继模式 */
	strcpy(basic->product.hard_ver, cjson_get_value(cj_get,"hard_ver", ""));     /* 硬件版本号，没有填空字符串 */
	basic->product.release_date = cjson_get_number(cj_get,"release_date",0);  /* 镜像编译时生成的时间，时间戳 */
	strcpy(basic->product.firm, cjson_get_value(cj_get,"product_firm", "")); /* 公司名称 */
	strcpy(basic->product.model, cjson_get_value(cj_get, "product_model", "")); /* 产品名F9 */
	snprintf((basic->product).soft_ver, sizeof((basic->product).soft_ver), ("%s"), cjson_get_value(cj_get,"soft_ver", ""));
	basic->init.guide_done = cjson_get_number(cj_get,"guid_done", 1);	/* 是否进行过设置向导 1-进行过 0-未进行过 */
	strcpy(basic->supt_wan_type, cjson_get_value(cj_get,"supt_wan_type", "")); /* wan口支持的接入类型 */
	basic->dev_mode = cjson_get_number(cj_get,"dev_mode", DEV_MODE_ROUTER); /* DUT当前是否为AP模式 */
	SET_SUPT_WAN_TYPE(basic);

	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	return 0;

}

/*************************************************************************
  功能: 获取系统高级信息
  参数: advance 填入系统高级信息数据
  返回值: 0成功，1失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_sys_advance_info_get_cb(const api_cmd_t *cmd, 
                   						 	sys_advance_info_t *advance,
                   						    void *privdata)
{
   	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;

	if (!advance) {
		printf("func param is null\n");
		return 1;
	}
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_SYS_ADVANCE_INFO));

	app_get_module_param(cj_query,cj_get);
	memset(advance, 0, sizeof(sys_advance_info_t)); 	
	advance->systime = cjson_get_number(cj_get,"systime",0); /* unix时间戳 */
	advance->uptime = cjson_get_number(cj_get,"uptime",0);  /* 系统开机启动时到现在的时间 s*/
	advance->cpu_info.max_freq = cjson_get_number(cj_get,"cpu_max_freq",0);  /* cpu频率 MHz */ 
	advance->cpu_info.curr_idle = cjson_get_number(cj_get,"cpu_curr_idle",0); /* cpu空闲率，60% */
	advance->mem_info.total = cjson_get_number(cj_get,"mem_total",0); /* 总内存大小 KB*/
	advance->mem_info.used =cjson_get_number(cj_get,"mem_used",0);   /* 使用了多少内存 KB*/

	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	return ret;
}
#ifdef CONFIG_APP_COSTDOWN
void biz_m_sys_reset(void)
{
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_RESET));
	cJSON_AddStringToObject(cj_set, "action","restore");
	
	app_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
}

void biz_m_sys_reboot(void)
{
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_REBOOT));
	cJSON_AddStringToObject(cj_set, "action","reboot");
	
	app_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
}

#else
/*************************************************************************
  功能:实现恢复出厂的函数
  参数: 不需要使用
  返回值: 无
  是否需要用户实现: 是
 ************************************************************************/
static void biz_m_sys_reset(
	const api_cmd_t * cmd, 
	int data_len, 
	int ret)
{
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_RESET));
	cJSON_AddStringToObject(cj_set, "action","restore");
	
	app_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
}

/*************************************************************************
  功能: 实现恢复出厂的回调函数
  参数: 不需要使用
  返回值: 默认返回 10
  是否需要用户实现: 否
 ************************************************************************/
int biz_m_sys_reset_cb(const api_cmd_t *cmd, void *privdata)
{
	sys_common_ack_t sys_ack;
	memset(&sys_ack, 0, sizeof(sys_ack));
	sys_ack.err_code = 0;
	uc_api_lib_cmd_resp_notify(cmd, 0, sizeof(sys_ack), &sys_ack, biz_m_sys_reset); /* 在恢复出厂之前通知手机app恢复出厂下发成功 */
	return COMPLETE_RET; /* 10 */
}

/*************************************************************************
  功能:实现路由器重启的功能的实现函数
  参数: 不需要使用
  返回值: 无
  是否需要用户实现: 是
 ************************************************************************/
static void biz_m_sys_reboot(
	const api_cmd_t * cmd, 
	int data_len, 
	int ret)
{
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_REBOOT));
	cJSON_AddStringToObject(cj_set, "action","reboot");
	
	app_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
}

/*************************************************************************
  功能: 实现重启的回调函数
  参数: 不需要使用
  返回值: 默认返回 10
  是否需要用户实现: 否
 ************************************************************************/
int biz_m_sys_reboot_cb(const api_cmd_t *cmd, void *privdata)
{
	sys_common_ack_t sys_ack;
	memset(&sys_ack, 0, sizeof(sys_ack));
	sys_ack.err_code = 0;
	uc_api_lib_cmd_resp_notify(cmd, 0, sizeof(sys_ack), &sys_ack, biz_m_sys_reboot); /* 在恢复出厂之前通知手机app恢复出厂下发成功 */
	return COMPLETE_RET; /* 10 */
}
#endif
