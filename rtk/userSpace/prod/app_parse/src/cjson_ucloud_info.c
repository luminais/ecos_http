#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi_lib.h"
#include "systools.h"

/*svn版本号头文件由脚本生成，放在主目录下，Makefile已包含 by lxp 2017.11.28*/
#include "svn_version.h"

#define BIZ_COMPANY_NAME "Tenda"

extern char *cjson_get_value(cJSON *root, char *name, char *defaultGetValue);
extern int cjson_get_number(cJSON *root, char *name, int defaultGetValue);

extern int __envram_set(const char *name, const char *value);
extern void biz_parse_fmt_mac_to_fmt1_mac(const char *in_mac, char *out_mac, int size);


/*************************************************************************
  功能: 设置云管理sn序列号
  参数: data  sn序列号指针
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
void app_set_ucloud_info_sn(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	char sn[PI_BUFLEN_32] = {0} ;
    if(NULL == recv_root)
    {
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
    }
	strcpy(sn,cjson_get_value(recv_root,"uc_serialnum",""));

	__envram_set("uc_serialnum", sn); /* sn是云服务器给的, 需要保存到falsh中 */
	envram_commit(0,NULL);
	nvram_set("uc_serialnum", sn);
	//nvram_commit();    /*nvram_commit统一在公共函数中commit*/
	
	return;
}
/*************************************************************************
  功能: 获取云管理基本信息
  参数: data  填入云管理基本信息
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
void app_get_ucloud_basic_info(cJSON *recv_root,cJSON *send_root,void *info)
{
	char mac_temp[PI_MAC_STRING_LEN] = {0};
	char version[PI_BUFLEN_32] = {0};
	char product_ver[PI_BUFLEN_32] = {0};
	char *mac = nvram_safe_get("et0macaddr");
	int uc_enable = 1;

	if(nvram_match("uc_manage_en","0"))
	{
		uc_enable = 0;  
	}
	if(NULL != mac)
	{
		memset(mac_temp,0x0,sizeof(mac_temp));
		biz_parse_fmt_mac_to_fmt1_mac((const char *)mac, mac_temp, sizeof(mac_temp));
	}
	else
	{
		printf("[%s][%d]Read %s failed!\n",__func__,__LINE__,"et0macaddr");
		return 1;
	}

	/* 云管理是否开启 1-开，0-关 */ 
	cJSON_AddNumberToObject(send_root,"uc_manage_en",uc_enable);
	/* 云管理sn序列号 */
	cJSON_AddStringToObject(send_root,"uc_serialnum",nvram_safe_get("uc_serialnum"));
	/*mac*/
    cJSON_AddStringToObject(send_root,"host_mac",mac_temp);

	snprintf(version, sizeof(version), "%s_%s(%s)", W311R_ECOS_SV, NORMAL_WEB_VERSION,SVN_VERSION);  
	cJSON_AddStringToObject(send_root,"version",version);
	//printf("*****************SVN_VERSION = %s\n",SVN_VERSION);
    /*公司名称*/
	cJSON_AddStringToObject(send_root,"company",COMPANY_NAME);
	
	//获取硬件版本号
	//sscanf(nvram_safe_get("BOARD_NAME"),"%[^_]",hard_var);
	strcpy(product_ver,nvram_safe_get("BOARD_NAME"));
	cJSON_AddStringToObject(send_root,"product" ,product_ver);

	cJSON_AddStringToObject(send_root,"update_random",UPDATE_RANDOM);//镜像密码 编译时 通过脚本生成随机密码
	//printf("*****************UPDATE_RANDOM = %s\n",UPDATE_RANDOM);
	
	//cJSON_AddStringToObject(send_root,"update_random",UPDATE_RANDOM222);//镜像密码 编译时 通过脚本生成随机密码

	//cJSON_AddStringToObject(send_root,"update_random","cpoej5");/*/镜像密码 编译时 通过脚本生成随机密码*/

	return;
}

/*************************************************************************
  功能: 获取云管理开关
  参数: manage_en 填入云管理开关状态
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
void app_get_ucloud_info_manage_en(cJSON *recv_root,cJSON *send_root,void *info)
{
	char *uc_enable = NULL;//云管理默认开启

    uc_enable =  nvram_safe_get("uc_manage_en");

    cJSON_AddNumberToObject(send_root, "uc_manage_en", atoi(uc_enable));
	
	return;
}

/*************************************************************************
  功能: 设置云管理开关
  参数: manage_en 用于设置云管理开关状态
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
void app_set_ucloud_info_manage_en(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	char manage_en[PI_BUFLEN_8]  = {0};
	if(NULL == recv_root)
    {
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
    }
	strcpy(manage_en,cjson_get_value(recv_root,"uc_manage_en",""));

	nvram_set("uc_manage_en", manage_en); /* 设置云管理开关, 需要保存到falsh中 */
	//nvram_commit(); /*nvram_commit统一在公共函数中commit*/
	return;
}
/*************************************************************************
  功能: 设置判断是否需要清空云账号
  参数: 忽略
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
void app_get_ucloud_info_try_clear_acc(cJSON *recv_root,cJSON *send_root,void *info)
{
	char value[PI_BUFLEN_32] = {0};
	
	strncpy(value, nvram_safe_get("ucloud_need_clear_acc"), sizeof(value));

	cJSON_AddStringToObject(send_root, "ucloud_need_clear_acc", value);

	return;
}

/*************************************************************************
  功能: 清空云账号后，设置云账号已清空标志
  参数: 忽略
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
void app_set_ucloud_info_clear_account(cJSON *recv_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	char account_en[PI_BUFLEN_8] = {0};
	if(NULL == recv_root)
    {
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
    }
	strcpy(account_en,cjson_get_value(recv_root, "ucloud_need_clear_acc",""));
	nvram_set( "ucloud_need_clear_acc", account_en);
	//nvram_commit(); /*nvram_commit统一在公共函数中commit*/
	
	return;
}


