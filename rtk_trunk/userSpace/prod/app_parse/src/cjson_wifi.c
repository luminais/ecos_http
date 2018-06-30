#include <stdio.h>
#include <stdlib.h>
#include "flash_cgi.h"
#include "cgi_handle_module.h"
#include "cgi_lib_config.h"
/*************************************************************************
  将int型的4个字节分成两个字节一组，高两个字节共有16位（0-15位），每一位表示支持的一个档位。
  比如高两个字节为0x0007，则表示支持高中低3档；若高两个字节为0x0005，则表示支持高和低两档；
  若高两个字节为0x0003，则表示支持中和低两档。低两个字节的每一位则表示当前的wifi强度，
  比如低两个字节为0x0004,0x0002,0x0001,则分别表示当前wifi强度为高中低
  
  0x0007 0001，表示支持高中低3个档位，当前wifi强度为低；
  0x0007 0002，表示支持高中低3个档位，当前wifi强度为中；
  0x0007 0004，表示支持高中低3个档位，当前wifi强度为高；
  0x0005 0001，表示支持高和低2个档位，当前wifi强度为低；
  0x0005 0004，表示支持高和低2个档位，当前wifi强度为高;
 ************************************************************************/
#define THIRD_GEAR      458752  /*0x0007 0000 支持高、中、低三个档位*/
#define SECOND_GEAR     393216  /*0x0006 0000 支持增强、普通档两个位*/ 
#define HIGH_POWER		4 
#define NOMAL_POWER		2
#define LOW_POWER		1	

//支持的功率等级,用于页面显示功率等级
#define SURPOT_HIGH_POWER    "no_hide"
#define SURPOT_NORMAL_POWER  "hide_normal"
#define SURPOT_LOW_POWER     "hide_power"

extern int app_msg_op_code_2g;
extern int app_msg_op_code_5g;
/****************************************************************************
函数名  :security_mode_format
描述    :用于app获取/设置 无线加密方式 格式与后台格式的转换
参数    :
    	char *sec_mode
        char *sec_mode_format
日    期   : 2017年11月15日
作    者   : luorilin
修改内容   : 新建函数
****************************************************************************/ 
static void security_mode_format(char *sec_mode,char *sec_mode_format) 
{ 
	if(NULL == sec_mode || NULL == sec_mode_format)
	{
		return;
	}
	
	if(0 == strcmp(sec_mode,"wpa-psk"))
	{
		sprintf(sec_mode_format,"%s","WPA/AES");
	}
	else if(0 == strcmp(sec_mode,"wpa2-psk"))
	{
		sprintf(sec_mode_format,"%s","WPA2/AES");
	}
	else if(0 == strcmp(sec_mode,"wpa&wpa2") 
		|| (0 == strcmp(sec_mode,"WPA/WPA2-PSK")))
	{
		sprintf(sec_mode_format,"%s","WPAWPA2/AES");
	}
    else if(0 == strcmp(sec_mode,"WPA/AES"))
	{
		sprintf(sec_mode_format,"%s","wpa-psk");
	}
	else if(0 == strcmp(sec_mode,"WPA2/AES"))
	{
		sprintf(sec_mode_format,"%s","wpa2-psk");
	}
	else if(0 == strcmp(sec_mode,"WPAWPA2/AES"))
	{
		sprintf(sec_mode_format,"%s","wpa&wpa2");
	}
	else
	{
		sprintf(sec_mode_format,"%s","NONE");
	}
	
	return;
}

/*****************************************************************************
 函 数 名  : app_get_wl_power
 功能描述  : app获取DUT无线功率等级
 输入参数  : cJSON *recv_root  
             cJSON *send_root  
             void *info        
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月31日
    作    者   : 段靖铖
    修改内容   : 新生成函数
  2.日	  期   : 2017年11月17日
    作	  者   : luorilin
    修改内容   : 添加获取5G功率

*****************************************************************************/
void app_get_wl_power(cJSON *recv_root,cJSON *send_root, void *info)
{
	CGI_LIB_INFO get_info;
	int app_wifi_power = 0;
	int app_wifi_power_5g = 0;
    PI8 err_code[PI_BUFLEN_32] = {0};
	PIU8 modules[] = 
	{
		MODULE_GET_WIFI_POWER,
	};

	get_info.wp = NULL;
	get_info.root = recv_root;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);

	//2.4G支持的功率等级
	cJSON *obj = cJSON_GetObjectItem(get_info.root, LIB_WIFI_POWER_GEAR);
	if(strcmp(obj->valuestring,SURPOT_HIGH_POWER) == 0)
		app_wifi_power += THIRD_GEAR;
	else if(strcmp(obj->valuestring,SURPOT_NORMAL_POWER) == 0)//表示支持两档调节
		app_wifi_power += SECOND_GEAR;
	else if(strcmp(obj->valuestring,SURPOT_LOW_POWER) == 0)//表示不支持功率调节
		app_wifi_power = 0;

	//2.4G获取当前功率
	obj = cJSON_GetObjectItem(get_info.root, LIB_WIFI_POWER);
	if(strcmp(obj->valuestring,"high") == 0)
		app_wifi_power += HIGH_POWER;
	else if(strcmp("normal",obj->valuestring) == 0 || app_wifi_power == SECOND_GEAR)
		app_wifi_power += NOMAL_POWER;
	else if(strcmp(obj->valuestring,"low") == 0)
		app_wifi_power += LOW_POWER;

	cJSON_AddNumberToObject(send_root, "wifi_2g_power",app_wifi_power);

	//5G支持的功率等级
	cJSON *obj_5g = cJSON_GetObjectItem(get_info.root, LIB_WIFI_POWER_GEAR);
	if(strcmp(obj_5g->valuestring,SURPOT_HIGH_POWER) == 0)
		app_wifi_power += THIRD_GEAR;
	else if(strcmp(obj_5g->valuestring,SURPOT_NORMAL_POWER) == 0)//表示支持两档调节
		app_wifi_power += SECOND_GEAR;
	else if(strcmp(obj_5g->valuestring,SURPOT_LOW_POWER) == 0)//表示不支持功率调节
		app_wifi_power = 0;

	//5G获取当前功率
	obj_5g = cJSON_GetObjectItem(get_info.root, LIB_WIFI_POWER);
	if(strcmp(obj_5g->valuestring,"high") == 0)
		app_wifi_power += HIGH_POWER;
	else if(strcmp(obj_5g->valuestring,"low") == 0)
		app_wifi_power += LOW_POWER;

	cJSON_AddNumberToObject(send_root, "wifi_5g_power",app_wifi_power_5g);
	
}


/*****************************************************************************
 函 数 名  : app_set_wl_power_process
 功能描述  : 使set 功率等级生效的函数，其实就是发送消息给DUT重启模块
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月31日
    作    者   : 段靖铖
    修改内容   : 新生成函数
  1.日    期   : 2017年11月17日
    作    者   : luorilin
    修改内容   : 调用wifi公用发消息函数发消息

*****************************************************************************/
void app_set_wl_power_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,app_msg_op_code_2g,msg);
	app_msg_op_code_2g = COMMON_MSG_MAX;
	send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,app_msg_op_code_5g,msg);
	app_msg_op_code_5g = COMMON_MSG_MAX;
}

/*****************************************************************************
 函 数 名  : app_set_wl_power
 功能描述  : app配置路由器功率等级，只是保存配置，并不生效，最后移除消息
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月31日
    作    者   : 段靖铖
    修改内容   : 新生成函数
  2.日	  期   : 2017年11月17日
  	作	  者   : luorilin
    修改内容   : 添加设置5G功率

*****************************************************************************/
void app_set_wl_power(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	CGI_LIB_INFO set_info;
	int wifi_2g_power = 0;
	int wifi_5g_power = 0;
	int current_power_2g = 0;
	int current_power_5g = 0;
    PI8 err_code[PI_BUFLEN_32] = {0};
	PI8 wl_power_2g[PI_BUFLEN_8] = {0};
	PI8 wl_power_5g[PI_BUFLEN_8] = {0};
	CGI_MSG_MODULE remov_msg;
	PIU8 modules[] = 
	{
		MODULE_SET_WIFI_POWER,	
	};

	//2.4G功率设置
	cJSON *obj = cJSON_GetObjectItem(send_root, "wifi_2g_power");
	if(NULL != obj)
	{
		/* 0x0007 0001，表示支持高中低3个档位，当前wifi强度为低*/
		wifi_2g_power = obj->valueint;
		
		if(THIRD_GEAR == (wifi_2g_power & THIRD_GEAR))/*支持高中低功率*/
		{		 
			current_power_2g = wifi_2g_power - THIRD_GEAR; /*当前功率*/
			
			if(HIGH_POWER == current_power_2g)
				strcpy(wl_power_2g,"high");
			else if(NOMAL_POWER == current_power_2g)
				strcpy(wl_power_2g,"normal");
			else if(LOW_POWER == current_power_2g)
				strcpy(wl_power_2g,"low");
		}
		else if(SECOND_GEAR == (wifi_2g_power &SECOND_GEAR))/*支持高低功率*/
		{
			current_power_2g = wifi_2g_power - SECOND_GEAR;
			
			if(HIGH_POWER == current_power_2g)
				strcpy(wl_power_2g,"high");
			else if(NOMAL_POWER == current_power_2g)
				strcpy(wl_power_2g,"low");
		}
		cJSON_AddStringToObject(send_root, LIB_WIFI_POWER,wl_power_2g);
	}
	/*5G功率设置 目前app不支持5G功率设置*/
	obj = cJSON_GetObjectItem(send_root, "wifi_5g_power");
	if(NULL != obj)
	{
		/* 0x0007 0001，表示支持高中低3个档位，当前wifi强度为低*/
		wifi_5g_power = obj->valueint;

		if(THIRD_GEAR == (wifi_5g_power &THIRD_GEAR))/*支持高中低功率*/
		{		 
			current_power_5g = wifi_5g_power - THIRD_GEAR; /*当前功率*/
			
			if(HIGH_POWER == current_power_5g)
				strcpy(wl_power_5g,"high");
			else if(NOMAL_POWER == current_power_5g)
				strcpy(wl_power_5g,"normal");
			else if(LOW_POWER == current_power_5g)
				strcpy(wl_power_5g,"low");
		}
		else if(SECOND_GEAR == (wifi_5g_power &SECOND_GEAR))/*支持高低功率*/
		{
			current_power_5g = wifi_5g_power - SECOND_GEAR;
			
			if(HIGH_POWER == current_power_5g)
				strcpy(wl_power_5g,"high");
			else if(NOMAL_POWER == current_power_5g)
				strcpy(wl_power_5g,"low");
		}
		cJSON_AddStringToObject(send_root, LIB_WIFI_POWER_5G,wl_power_5g);
	}
    
	set_info.wp = NULL;
	set_info.root = send_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);

	//因为保存配置需要与生效分离，故删除消息列表中的消息，在app调用set_process后重新加入消息列表
	memset(remov_msg.msg,0x0,MAX_MODULE_MSG_MAX_LEN);
	remov_msg.id = RC_WIFI_MODULE;
	remove_msg_to_list(msg,remov_msg);

	//返回错误码
	if(strcmp(err_code,"0"))
		*result_code = 1;

}


/*****************************************************************************
 函 数 名  : app_get_wl_channel_grade
 功能描述  : app获取wifi的信号优劣等级，主要根据信道获取。评分:20最高，10最低
 输入参数  : cJSON *recv_root  
             cJSON *send_root  
             void *info        
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月31日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
static unsigned int app_current_time = 0;
void app_get_wl_channel_grade(cJSON *recv_root,cJSON *send_root, void *info)
{
	unsigned int current_time = cyg_current_time()/100;

	if(NULL == send_root)
	{
		printf("func:%s line:%d send_root is NULL\n",__func__,__LINE__);
        return;
	}
	/*需求:如果一键优化了以后，五分钟以内再进入监测界面，都是100分*/
	if((current_time - app_current_time) < 300 && app_current_time!=0)
	{
		cJSON_AddNumberToObject(send_root, "channel_2g_sta",20);
		return;
	}
	
	CGI_LIB_INFO get_info;
	cJSON *obj = NULL;
	PIU8 modules[] = 
	{
		MODULE_GET_WLFI_CHANNEL_GRADE,

	};

	obj = cJSON_CreateObject();
    if(NULL == obj)
    {
        printf("[%s][%d]create obj fail\n",__func__,__LINE__);
        return ;
    }
	get_info.wp = NULL;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);

	cJSON_AddNumberToObject(send_root, "channel_2g_sta",cjson_get_number(obj,LIB_CHANNEL_2G_GRADE,15));
	cJSON_Delete(obj);
	return;
}


/*****************************************************************************
 函 数 名  : app_set_wl_channel_grade
 功能描述  : app将wifi信号调到最优，其实就是讲信道重新自动选择一次，选择最佳
             信道
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月31日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
void app_set_wl_channel_grade(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	/*开始计时*/
	app_current_time =  cyg_current_time()/100;
	CGI_LIB_INFO set_info;
	
    PI8 err_code[PI_BUFLEN_32] = {0};
	PIU8 modules[] = 
	{
		MODULE_SET_WLFI_CHANNEL_GRADE,	
	};
	
	set_info.wp = NULL;
	set_info.root = send_root;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
}


/****************************************************************************
函数名  :app_get_wifi_basic_info
描述    :用于app获取wifi基本信息
参数    :
    send_root:用于函数回传数据，
        
    info:无
1.日    期   : 2016年12月23日
  作    者   : liusongming
  修改内容   : 新建函数
2.日	期	 : 2017年11月17日
  作	者	 : luorilin
  修改内容	 : 添加获取5G基本信息

****************************************************************************/
void app_get_wifi_basic_info(cJSON *recv_root,cJSON *send_root, void *info)
{
	int ssid_hide_en = 0;
	int wifi_enable = 1;
    int i = 0;
	char sec_mode[PI_BUFLEN_16] = {0};
    int n_sec_option = 0;
    cJSON *sec_option,* item;
	char *sec[4] = {"NONE","wpa-psk","wpa2-psk","wpa&wpa2"};    //路由器可支持的加密方式，目前路由器只支持4种
    cJSON *obj;
	CGI_LIB_INFO get_info;
    
    if(NULL == send_root)
    {
        printf("[%s][%d] send_root is NULL\n",__func__,__LINE__);
        return;
    }

    //调用lib库获取相应的数据
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_BASIC,
	};

    obj = cJSON_CreateObject();
    if(NULL == obj)
    {
        printf("[%s][%d]create obj fail\n",__func__,__LINE__);
        return ;
    }
	get_info.wp = NULL;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);   //调用公共get函数进行获取信息

	/********************2.4G 无线基本配置*************************/
    //ssid
    cJSON_AddStringToObject(send_root,"ssid",cjson_get_value(obj,LIB_WIFI_AP_SSID,""));
    //wifi security
    security_mode_format(cjson_get_value(obj,LIB_WIFI_AP_SEC_MODE,"NONE"),sec_mode);
    cJSON_AddStringToObject(send_root,"sec",sec_mode);
    //wifi password
    cJSON_AddStringToObject(send_root,"passwd",cjson_get_value(obj,LIB_WIFI_AP_PWD,""));
    //WiFi hide
    ssid_hide_en = strncmp(cjson_get_value(obj,LIB_WIFI_HIDE_SSID,"false"),"true",strlen(true)) ? 0 : 1;
    cJSON_AddNumberToObject(send_root,"ssid_hide",ssid_hide_en);
	//wifi enable
    wifi_enable = strncmp(cjson_get_value(obj,LIB_WIFI_EN,"true"),"true",strlen("true")) ? 0 : 1;  
    cJSON_AddNumberToObject(send_root,"enable",wifi_enable);

	//surppot security mode
    cJSON_AddItemToObject(send_root,"sec_option",sec_option = cJSON_CreateArray());
    for(i = 0; i < 4; i++)
    {
        cJSON_AddItemToArray(sec_option,item = cJSON_CreateObject());
        cJSON_AddStringToObject(item,"sec",sec[i]);
    }
    //获取路由器可加密方式的数量
    n_sec_option = cJSON_GetArraySize(sec_option);
    cJSON_AddNumberToObject(send_root,"n_sec_option",n_sec_option);
    
	/********************5G 无线基本配置*************************/
	//ssid
    cJSON_AddStringToObject(send_root,"ssid_5g",cjson_get_value(obj,LIB_WIFI_AP_SSID_5G,""));
    //wifi security
    memset(sec_mode,0x0,sizeof(sec_mode));
    security_mode_format(cjson_get_value(obj,LIB_WIFI_AP_SEC_MODE_5G,"NONE"),sec_mode);
    cJSON_AddStringToObject(send_root,"sec_5g",sec_mode);
    //wifi password
    cJSON_AddStringToObject(send_root,"passwd_5g",cjson_get_value(obj,LIB_WIFI_AP_PWD_5G,""));
    //WiFi hide
    ssid_hide_en = strncmp(cjson_get_value(obj,LIB_WIFI_HIDE_SSID_5G,"false"),"true",strlen(true)) ? 0 : 1;
    cJSON_AddNumberToObject(send_root,"ssid_hide_5g",ssid_hide_en);
	//wifi enable
    wifi_enable = strncmp(cjson_get_value(obj,LIB_WIFI_EN_5G,"true"),"true",strlen("true")) ? 0 : 1;  
    cJSON_AddNumberToObject(send_root,"enable_5g",wifi_enable);

	//surppot security mode
    cJSON_AddItemToObject(send_root,"sec_option_5g",sec_option = cJSON_CreateArray());
    for(i = 0; i < 4; i++)
    {
        cJSON_AddItemToArray(sec_option,item = cJSON_CreateObject());
        cJSON_AddStringToObject(item,"sec_5g",sec[i]);
    }
    //获取路由器可加密方式的数量
    n_sec_option = cJSON_GetArraySize(sec_option);
    cJSON_AddNumberToObject(send_root,"n_sec_option_5g",n_sec_option);
	
    cJSON_Delete(obj);

    return ;
}


/****************************************************************************
函数名  :app_set_wifi_basic_info
描述    :用于app设置wifi基本信息
参数    :
    send_root:用于函数回传数据，
        
    info:无
1.日    期   : 2016年12月23日
  作    者   : liusongming
  修改内容   : 新建函数
2.日	期	 : 2017年11月17日
  作	者	 : luorilin
  修改内容	 : 添加设置5G基本信息

****************************************************************************/  
void app_set_wifi_basic_info(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
    cJSON *obj = NULL;
    char *ssid_hide = NULL;
	char *wifi_enable = NULL;
	char sec_mode[PI_BUFLEN_32] = {0};
    PI8 err_code[PI_BUFLEN_32] = {0};
    CGI_MSG_MODULE remov_msg;
	
    if(NULL == send_root)
    {
        printf("func:%s line:%d send_root is NULL\n",__func__,__LINE__);
        return;
    }
    obj = cJSON_CreateObject();
    if(NULL == obj)
    {
    	printf("func:%s line:%d create obj fail\n",__func__,__LINE__);
        return;
    }

	/*组装数据,格式如页面下发的数据格式*/ 
	/*************************2.4G基本配置**************************/
	//wifi hide app:1/0 web:true/false
    ssid_hide = cjson_get_number(send_root,"ssid_hide",0) ? "true" : "false"; 
    cJSON_AddStringToObject(obj,LIB_WIFI_HIDE_SSID,ssid_hide);	
    //wifi enable  app:1/0 web:true/false
    wifi_enable = cjson_get_number(send_root,"enable",1) ? "true" : "false";
    cJSON_AddStringToObject(obj,LIB_WIFI_EN,wifi_enable);
    cJSON_AddStringToObject(obj,LIB_WIFI_AP_SSID,cjson_get_value(send_root,"ssid",""));    
    cJSON_AddStringToObject(obj,LIB_WIFI_AP_PWD,cjson_get_value(send_root,"passwd",""));    

	/*app端设置 加密方式下发的格式：wpa-psk、wpa2-psk、wpa&wpa2、NONE*/
	/*注意，设置是需要与web页面下发的格式保持一致：WPA/AES、WPA2/AES、WPAWPA2/AES、NONE*/
	security_mode_format(cjson_get_value(send_root,"sec","NONE"),sec_mode);
    cJSON_AddStringToObject(obj,LIB_WIFI_AP_SEC_MODE,sec_mode);  
    /*************************5G基本配置**************************/
	//wifi hide app:1/0 web:true/false
    ssid_hide = cjson_get_number(send_root,"ssid_hide_5g",0) ? "true" : "false"; 
    cJSON_AddStringToObject(obj,LIB_WIFI_HIDE_SSID_5G,ssid_hide);	
    //wifi enable  app:1/0 web:true/false
    wifi_enable = cjson_get_number(send_root,"enable_5g",1) ? "true" : "false";
    cJSON_AddStringToObject(obj,LIB_WIFI_EN_5G,wifi_enable);
    cJSON_AddStringToObject(obj,LIB_WIFI_AP_SSID_5G,cjson_get_value(send_root,"ssid_5g",""));    
    cJSON_AddStringToObject(obj,LIB_WIFI_AP_PWD_5G,cjson_get_value(send_root,"passwd_5g",""));    

	/*app端设置 加密方式下发的格式：WPA-PSK、WPA2-PSK、WPA/WPA2-PSK、NONE*/
	/*注意，设置是需要与web页面下发的格式保持一致：WPA/AES、WPA2/AES、WPAWPA2/AES、NONE*/
	memset(sec_mode,0x0,sizeof(sec_mode));
	security_mode_format(cjson_get_value(send_root,"sec_5g","NONE"),sec_mode);
    cJSON_AddStringToObject(obj,LIB_WIFI_AP_SEC_MODE_5G,sec_mode); 

	//调用lib库函数进行无线基本配置生效
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
	{
        MODULE_SET_WIFI_BASIC,
	};
    set_info.wp = NULL;
	set_info.root = obj;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, info);

    //判断调用lib库函数是否有出错
    if(strcmp(err_code,"0"))
    {
        *result_code = 1;
    }
    
    memset(remov_msg.msg,0x0,MAX_MODULE_MSG_MAX_LEN);
	remov_msg.id = RC_WIFI_MODULE;
	remove_msg_to_list(msg,remov_msg);

    cJSON_Delete(obj);

    return ;
}

/****************************************************************************
函数名  :app_set_wifi_process
描述    :用于app设置wifi基本信息后使设置生效的函数
参数    :
    send_root:
        
    info:无
1.日    期   : 2016年12月23日
  作    者   : liusongming
  修改内容   : 新建函数
2.日	期	 : 2017年11月17日
  作	者	 : luorilin
  修改内容	 : 通过wifi公用发消息函数发消息

****************************************************************************/  
void app_set_wifi_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	send_wifi_msg_handle(TENDA_WLAN24_AP_IFNAME,app_msg_op_code_2g,msg);
	app_msg_op_code_2g = COMMON_MSG_MAX;
	send_wifi_msg_handle(TENDA_WLAN5_AP_IFNAME,app_msg_op_code_5g,msg);
	app_msg_op_code_5g = COMMON_MSG_MAX;
}

void app_get_wifi_push_info(cJSON *recv_root,cJSON *send_root, void *info)
{
	CGI_LIB_INFO get_info;
	cJSON *wifi_info = NULL;
	wifi_info =cJSON_CreateObject();

	if(NULL == wifi_info)
	{
		printf("func:%s line:%d wifi_info created error!\n",__func__,__LINE__);
        return;
	}
	
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_BASIC,
		MODULE_GET_WIFI_EN,	
	};
	
	get_info.wp = NULL;
	get_info.root = wifi_info;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);

    int wifi_enable = 1;
	
	cJSON_AddNumberToObject(send_root,"wifi_type",0);    
	cJSON_AddStringToObject(send_root,"ssid",cjson_get_value(wifi_info,LIB_WIFI_AP_SSID,""));
    wifi_enable = strncmp(cjson_get_value(wifi_info,LIB_WIFI_EN,""),"true",strlen("true")) ? 0 : 1;
	cJSON_AddNumberToObject(send_root,"wifi_enable",wifi_enable);

	cJSON_AddNumberToObject(send_root,"wifi_type_5g",1);  
	cJSON_AddStringToObject(send_root,"ssid_5g",cjson_get_value(wifi_info,LIB_WIFI_AP_SSID_5G,""));
    wifi_enable = strncmp(cjson_get_value(wifi_info,LIB_WIFI_EN_5G,""),"true",strlen("true")) ? 0 : 1;
	cJSON_AddNumberToObject(send_root,"wifi_enable_5g",wifi_enable);

	cJSON_Delete(wifi_info);
	return;
}

#ifdef __CONFIG_GUEST__
/****************************************************************************
函数名  :app_get_wifi_guest_info
描述    :用于app获取guest wifi基本信息
参数    :
    send_root:
        
    info:无
日    期   : 2017年11月15日
作    者   : luorilin
修改内容   : 新建函数
****************************************************************************/ 
void app_get_wifi_guest_info(cJSON *recv_root,cJSON *send_root, void *info)
{
	char *wifi_guest_enable = NULL;
	char security_mode[PI_BUFLEN_32] = {0};
	CGI_LIB_INFO get_info;
	cJSON *wifi_guest_info = NULL;
	wifi_guest_info = cJSON_CreateObject();

	if(NULL == wifi_guest_info)
	{
		printf("[%s][%d] wifi_guest_info created error!\n",__func__,__LINE__);
		return;
	}
	
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_GUEST_INFO,
	};
	
	get_info.wp = NULL;
	get_info.root = wifi_guest_info;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);

	//2.4G wifi guest
	wifi_guest_enable = strcmp(cjson_get_value(wifi_guest_info, LIB_WIFI_GUEST_EN,"false"),"true") ? "0" : "1";
	cJSON_AddNumberToObject(send_root,"wifi_guest_en",atoi(wifi_guest_enable));
	cJSON_AddStringToObject(send_root,"wifi_guest_ssid",cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_SSID,"Tenda_VIP"));
    cJSON_AddStringToObject(send_root,"wifi_guest_passwd",cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_PWD,""));	
	/*security mode*/
	security_mode_format(cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_SEC_MODE,"NONE"),security_mode);
	cJSON_AddStringToObject(send_root, "wifi_guest_sec", security_mode);

	//5G wifi guest
	wifi_guest_enable = strcmp(cjson_get_value(wifi_guest_info, LIB_WIFI_GUEST_EN_5G,"false"),"true") ? "0" : "1";
	cJSON_AddNumberToObject(send_root,"wifi_guest_en_5g",atoi(wifi_guest_enable));
	cJSON_AddStringToObject(send_root,"wifi_guest_ssid_5g",cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_SSID_5G,"Tenda_VIP_5G"));
    cJSON_AddStringToObject(send_root,"wifi_guest_passwd_5g",cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_PWD_5G,""));	
	/*security mode*/
	memset(security_mode,0x0,sizeof(security_mode));
	security_mode_format(cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_SEC_MODE_5G,"NONE"),security_mode);
	cJSON_AddStringToObject(send_root, "wifi_guest_sec_5g", security_mode);
	
	/*wifi guest effect_time app端 effect_time 分钟表示*/
	cJSON_AddNumberToObject(send_root,"wifi_guest_timeout",atoi(cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_EFFECT_TIME,"8"))*60);
	/*wifi guest down speed limit /Mbps*/
	cJSON_AddNumberToObject(send_root,"wifi_guest_rate",atoi(cjson_get_value(wifi_guest_info,LIB_WIFI_GUEST_SHARE_SPEED,"0")));

	cJSON_Delete(wifi_guest_info);
	
	return;
}

/*****************************************************************************
 函 数 名  : app_set_wifi_guest_info_process
 功能描述  : 使设置访客网络生效
 输入参数  : cJSON *send_root     
             CGI_MSG_MODULE *msg  
             int *result_code     
             void *info           
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月15日
    作    者   : luorilin
    修改内容   : 新生成函数

*****************************************************************************/
void app_set_wifi_guest_info_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	send_wifi_msg_handle(TENDA_WLAN24_GUEST_IFNAME,app_msg_op_code_2g,msg);
	app_msg_op_code_2g = COMMON_MSG_MAX;
	send_wifi_msg_handle(TENDA_WLAN5_GUEST_IFNAME,app_msg_op_code_5g,msg);
	app_msg_op_code_5g = COMMON_MSG_MAX;
}

/****************************************************************************
函数名  :app_set_wifi_guest_info
描述    :用于app设置guest wifi基本信息后使设置生效的函数
参数    :
    send_root:
        
    info:无
日    期   : 2017年11月15日
作    者   : luorilin
修改内容   : 新建函数
****************************************************************************/ 
void app_set_wifi_guest_info(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
	char security_mode[PI_BUFLEN_32] = {0}; 
	char effect_time_str[PI_BUFLEN_32] = {0};
	char down_speed_limit_str[PI_BUFLEN_32] = {0};
	PI8 err_code[PI_BUFLEN_16] = {0};
	cJSON *obj = NULL;
	CGI_MSG_MODULE remov_msg;
	
	if(NULL == send_root)
    {
        printf("[%s][%d] send_root is NULL\n",__func__,__LINE__);
        return;
    }
    obj = cJSON_CreateObject();
    if(NULL == obj)
    {
    	printf("[%s][%d] create obj fail\n",__func__,__LINE__);
        return;
    }

	//2.4G wifi guest	
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_EN,cjson_get_number(send_root,"wifi_guest_en",0) ? "true" : "false");
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_SSID,cjson_get_value(send_root,"wifi_guest_ssid","Tenda_VIP"));
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_PWD,cjson_get_value(send_root,"wifi_guest_passwd",""));
	/*security mode*/
	get_wlan_security_mode(cjson_get_value(send_root,"wifi_guest_sec","NONE"),security_mode);
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_SEC_MODE, security_mode);

	//5G wifi guest	
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_EN_5G,cjson_get_number(send_root,"wifi_guest_en_5g",0) ? "true" : "false");
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_SSID_5G,cjson_get_value(send_root,"wifi_guest_ssid_5g","Tenda_VIP_5G"));
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_PWD_5G,cjson_get_value(send_root,"wifi_guest_passwd_5g",""));
	/*security mode*/
	get_wlan_security_mode(cjson_get_value(send_root,"wifi_guest_sec_5g","NONE"),security_mode);
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_SEC_MODE_5G, security_mode);
	/*effect  time app下发的时间分钟 后台统一将小时转化为秒*/
	snprintf(effect_time_str,sizeof(effect_time_str),"%d",cjson_get_number(send_root,"wifi_guest_timeout",8)/60);   
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_EFFECT_TIME, effect_time_str);
	/*down_speed limit 如256KBs 上传不做限制*/
	snprintf(down_speed_limit_str,sizeof(down_speed_limit_str),"%d",cjson_get_number(send_root,"wifi_guest_rate",0));  
	cJSON_AddStringToObject(obj,LIB_WIFI_GUEST_SHARE_SPEED, down_speed_limit_str);
	
	//调用lib库函数进行无线基本配置生效
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
	{
        MODULE_SET_WIFI_GUEST_INFO,
	};
    set_info.wp = NULL;
	set_info.root = obj;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, info);

    //判断调用lib库函数是否有出错
    if(strcmp(err_code,"0"))
    {
        *result_code = 1;
    }
    
    memset(remov_msg.msg,0x0,MAX_MODULE_MSG_MAX_LEN);
	remov_msg.id = RC_WIFI_MODULE;
	remove_msg_to_list(msg,remov_msg);

    cJSON_Delete(obj);

	return;
}
#endif 
