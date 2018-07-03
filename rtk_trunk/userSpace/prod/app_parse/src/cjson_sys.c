#include <stdio.h>
#include <stdlib.h>
#include <bcmnvram.h>

#include "cgi_handle_module.h"
#include "cgi_lib_config.h"
#include "autoconf.h"
#include "time.h"
#include "systools.h"

/*****************************************************************************
 函 数 名  : app_get_sys_advance_info
 功能描述  : app获取系统高级信息
 			 注:其他cpu频率相关未实现
 输入参数  : cJSON *recv_root  
             cJSON *send_root  
             void *info        
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月28日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
void app_get_sys_advance_info(cJSON *recv_root,cJSON *send_root, void *info)
{
	CGI_LIB_INFO get_info;
	struct mallinfo mem_info;
	mem_info = mallinfo();

	PIU8 modules[] = 
	{
		
	};

	get_info.wp = NULL;
	get_info.root = recv_root;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);

	cJSON_AddNumberToObject(send_root, "systime",(uint32_t)time(NULL));//unix时间戳
	cJSON_AddNumberToObject(send_root, "uptime",cyg_current_time()/100);//开机时间
	cJSON_AddNumberToObject(send_root, "cpu_max_freq",300);//暂时未实现
	cJSON_AddNumberToObject(send_root, "cpu_curr_idle",30);//暂时未实现
	cJSON_AddNumberToObject(send_root, "mem_total",mem_info.arena);//总内存
	cJSON_AddNumberToObject(send_root, "mem_used",mem_info.arena - mem_info.fordblks);//空闲内存
}


/*****************************************************************************
 函 数 名  : app_get_sys_basic_info
 功能描述  : app获取系统基本信息
 			 注:其中编译时间的未实现，没有用到
 输入参数  : cJSON *recv_root  
             cJSON *send_root  
             void *info        
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月28日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
#define 	DUT_MODE_AP   			   	1
#define 	DUT_MODE_ROUTER			2
void app_get_sys_basic_info(cJSON *recv_root,cJSON *send_root, void *info)
{
	CGI_LIB_INFO get_info;
	int wl_mode = 0;
	int dev_mode = DUT_MODE_ROUTER;
	char hard_var[8] = {0};
	char product_name[8] = {0};
	PIU8 modules[] = 
	{
		MODULE_GET_WIFI_RELAY_TYPE,
		MODULE_GET_WAN_MAC,
		MODULE_GET_FIREWARE,
	};

	get_info.wp = NULL;
	get_info.root = recv_root;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);

	//获取系统MAC地址
	cJSON *obj = cJSON_GetObjectItem(get_info.root, LIB_WAN_MAC_CUR_WAN);
	cJSON_AddStringToObject(send_root, "mac",obj->valuestring);

	//获取系统工作模式
	obj = cJSON_GetObjectItem(get_info.root, LIB_WIFI_RELAY_TYPE);
	if(strcmp(obj->valuestring,"wisp") == 0)
		wl_mode = 3;
	else if(strcmp(obj->valuestring,"client+ap") == 0)
		wl_mode = 2;
	else if(strcmp(obj->valuestring,"ap") == 0){
		/*根据成研提供的信息，如果为AP 模式，则dev_mode = 1,否则为2*/
		dev_mode = DUT_MODE_AP;
		wl_mode = 0;
	}
	else{
		wl_mode = 0;
	}
		
	cJSON_AddNumberToObject(send_root, "wl_mode",wl_mode);
	cJSON_AddNumberToObject(send_root, "dev_mode",dev_mode);
	//获取uc_sn号
	cJSON_AddStringToObject(send_root, "sn_number", nvram_safe_get("uc_serialnum"));
	
	//获取硬件版本号
	sscanf(nvram_safe_get("BOARD_NAME"),"%*[^_]_%s",hard_var);
	cJSON_AddStringToObject(send_root,"hard_ver" ,hard_var);

	//获取软件版本号
	obj = cJSON_GetObjectItem(get_info.root, LIB_SOFT_VER);
	cJSON_AddStringToObject(send_root, "soft_ver",obj->valuestring);

	//获取镜像编译时间(未实现，没有用处)
	cJSON_AddNumberToObject(send_root, "release_date",1478742773);

	//获取公司名称
	cJSON_AddStringToObject(send_root, "product_firm",COMPANY_NAME);
	
	//获取产品型号
	//sscanf(nvram_safe_get("BOARD_NAME"),"%*[^_]_%s",product_name);
	sscanf(nvram_safe_get("BOARD_NAME"),"%[^_]",product_name);
	cJSON_AddStringToObject(send_root, "product_model",product_name);

	//获取是否进行过快速设置页面状态
	if(strcmp(nvram_safe_get("restore_quick_set"),"1") == 0)
	{
		cJSON_AddNumberToObject(send_root,"guid_done",0);
	}
		
	else
	{
		cJSON_AddNumberToObject(send_root,"guid_done",1);
	}
	//WAN口支持的上网方式
	cJSON_AddStringToObject(send_root,"supt_wan_type","static,dhcp,adsl");

}


/*****************************************************************************
 函 数 名  : app_set_reboot
 功能描述  : app设置重启
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月28日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
void app_set_reboot(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	CGI_LIB_INFO set_info;
    PI8 err_code[PI_BUFLEN_32] = {0};

	PIU8 modules[] = 
	{
		MODULE_SET_OPERATE,	
	};
	
	set_info.wp = NULL;
	set_info.root = send_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);

}


/*****************************************************************************
 函 数 名  : app_set_reset
 功能描述  : app设置恢复出厂设置
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月28日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
void app_set_reset(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	CGI_LIB_INFO set_info;
    PI8 err_code[PI_BUFLEN_32] = {0};
	
	PIU8 modules[] = 
	{
		MODULE_SET_OPERATE,	
	};
	
	set_info.wp = NULL;
	set_info.root = send_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);

}

/*****************************************************************************
 函 数 名  : app_set_wizard_succeed
 功能描述  : app快速设置完成函数
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年1月6日
    作    者   : 刘松明
    修改内容   : 新生成函数

*****************************************************************************/
void app_set_wizard_succeed(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	CGI_LIB_INFO set_info;
    PI8 err_code[PI_BUFLEN_32] = {0};
	
	PIU8 modules[] = 
	{
		MODULE_SET_WIZARD_SUCCEED,	
		
	};
	
	set_info.wp = NULL;
	set_info.root = send_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);	//调用lib库函数进行设置
	
	return ;
}
	



