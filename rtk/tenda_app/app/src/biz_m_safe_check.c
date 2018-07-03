#include "cgi_module_interface.h"

#include "biz_m_safe_check.h"

/*************************************************************************
  功能:  wifi密码强度检测
  参数: 需要填入string
  返回值: 0-没有密码，1-不安全，2-安全
  是否需要用户实现: 否
************************************************************************/
static int biz_check_wifi_password_strength(char *string)
{
	
	int i;
	char *simple_wifi_passwd[16] = {
		"123456", "12345678", "abc123", "password", "123456789", 
		"12345",  "88888888", "654321", "87654321", "666666",
		"666666","admin","root","987654321","11223344","0123456789"};
	
	if (string == NULL || strlen(string) == 0) {
		return 0;
	}
	
	
	for (i = 0; i < 16; i ++) {
		if (strcmp(simple_wifi_passwd[i], string) == 0) {
			return 1;
		}
	}
	
	return 2;
	
}

/*************************************************************************
  功能:  web登录密码安全检测 
  参数: 需要填入string
  返回值: 0-没有密码，1-不安全，2-安全
  是否需要用户实现: 否
************************************************************************/
static char wifi_password[65]={0};
static int biz_check_web_login_password_strength(char *string)
{
	int i;
	char *simple_login_passwd[16] = {
		"123456", "12345678", "abc123", "password", "123456789", 
		"12345",  "88888888", "654321", "87654321", "666666",
		"666666","admin","root","987654321","11223344","0123456789"};

		
	if (string == NULL || strlen(string) == 0) {
		return 0;
	}

	if(strcmp(string,wifi_password) == 0){
		return 0x110;
	}
	
	for (i = 0; i < 16; i ++) {
		if (strcmp(simple_login_passwd[i], string) == 0) {
			return 1;
		}
	}
	
	return 2;
}

/*************************************************************************
  功能: 实现安全检测的回调函数
  参数: 需要填入safe_check_info_t
  返回值: 成功返回0，失败返回1
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_safe_check_get_cb(const api_cmd_t *cmd, 
					safe_check_info_t *info, void *privdata)
{

	char login_depw[64] = {0},wl_password[65] = {0};
	int ret = 0;
	cJSON *cj_get	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());	
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_SAFE_GRADE));

	ret = app_get_module_param(cj_query,cj_get);	
	strcpy(wl_password,cjson_get_value(cj_get, "wifi_password", ""));
	strcpy(login_depw,cjson_get_value(cj_get, "decode_login_password", ""));
	strcpy(wifi_password,wl_password);


	/* 必填， web登录密码检测结果 0-没有密码，1-不安全，2-安全 0x110 -与wifi密码一致*/
	if(biz_check_web_login_password_strength(login_depw) == 0)
	{
		info->auth_pwd_sta = 0; 
	}
	else if(biz_check_web_login_password_strength(login_depw) == 1)
	{
		info->auth_pwd_sta = 1;
	}		
	else if(biz_check_web_login_password_strength(login_depw) == 0x110)
	{
		info->auth_pwd_sta = 0x110;
	}
	else
	{
		info->auth_pwd_sta = 2;
	}

	
	/* 必填， wifi 2.4g密码的检测结果  0-没有密码，1-不安全，2-安全  */	
	if(biz_check_wifi_password_strength(wl_password) == 0)
	{
		info->wifi_24_pwd_sta = 0;
	}
	else if(biz_check_wifi_password_strength(wl_password) == 1)
	{
		info->wifi_24_pwd_sta = 1;
	}		
	else
	{
		info->wifi_24_pwd_sta = 2;
	}		

	/* 必填， 是否有dns劫持 0-安全, 1不安全  */ 
	info->dns_hijack_sta = 0;   

/*下面是保留接口*/
#if 0	
	info->wifi_50_pwd_sta = 1;  /* wifi 5g密码的检测结果 0-没有密码；1-弱；2-中；3-强；4-高 */
	SET_SAFE_CHECK_INFO_WIFI_50_PWD(info); /* 设置了5g, 必须设置该标志位, 因为wifi_50_pwd_sta字段是可选的 */
	
	info->ddos_attack_sta = 1;  /* 是否有ddos攻击 0-安全 默认填0 */
	SET_SAFE_CHECK_INFO_DDOS_ATTACK(info); /* 设置了ddos攻击, 必须设置该标志位，因为ddos_attack_sta字段是可选的 */
#endif

    cJSON_Delete(cj_query);
	cJSON_Delete(cj_get);

	return ret;
}

/*************************************************************************
  功能:  dns优化
  参数:  不需要
  返回值: 0-成功 1-失败
  是否需要用户实现: 否
************************************************************************/
int biz_m_dns_optimize(const api_cmd_t *cmd, 
							   void *privdata)
{
 	printf("\n-----biz_m_dns_optimize-----\n");
	printf("\n");
	return 0;
}


