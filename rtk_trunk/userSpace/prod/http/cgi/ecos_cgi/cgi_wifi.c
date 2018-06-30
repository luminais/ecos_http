/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_wifi.c
  版 本 号   : 初稿
  作    者   : fh
  生成日期   : 2016年12月13日
  最近修改   :
  功能描述   :

  功能描述   : 无线参数的获取与设置

  修改历史   :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#include "cgi_lib.h"

#if 0
PIU8 get_modules_wifiEn[] =
{
	MODULE_GET_WIFI_EN,
	MODULE_GET_END,
};

PIU8 get_modules_isWifiClients[] = 
{		
	MODULE_GET_ISWIFICLIENT,
	MODULE_GET_END,
};	

PIU8 get_modules_wifiBasicCfg[] =
{
	MODULE_GET_WIFI_BASIC,
	MODULE_GET_END,
};
	
PIU8 get_modules_wifiPower[] =
{
	MODULE_GET_WIFI_POWER,
	MODULE_GET_END,
};

PIU8 get_modules_wifiTime[] =
{
	MODULE_GET_WIFI_SCHED,
	MODULE_GET_WIFI_RELAY_TYPE,
	MODULE_GET_END,
};

PIU8 get_modules_wifiAdvCfg[] =
{
	MODULE_GET_WIFI_ADV_CFG,
	MODULE_GET_WIFI_RELAY_TYPE,
	MODULE_GET_END,
};

PIU8 get_modules_wifiWPS[] =
{
	MODULE_GET_WIFI_WPS,
	MODULE_GET_END,
};


PIU8 set_modules_wifiEn[] =
{
	MODULE_SET_WIFI_EN,
	MODULE_SET_END,
};

PIU8 set_modules_wifiBasicCfg[] =
{
	MODULE_SET_WIFI_BASIC,
	MODULE_SET_END,
};

PIU8 set_modules_wifiPower[] =
{
	MODULE_SET_WIFI_POWER,
	MODULE_SET_END,
};

PIU8 set_modules_wifiTime[] =
{
	MODULE_SET_WIFI_SCHED,
	MODULE_SET_END,
};

PIU8 set_modules_wifiAdvCfg[] =
{
	MODULE_SET_WIFI_ADV_CFG,
	MODULE_SET_END,
};

PIU8 set_modules_wifiWPS[] =
{
	MODULE_SET_WIFI_WPS,
	MODULE_SET_END,
};
#endif
/*****************************************************************************
 函 数 名  : cgi_wifiEn_get
 功能描述  : 无线开关状态的获取
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  : RET_INFO

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiEn_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_EN,
	};

	cJSON_AddItemToObject(root, "wifiEn", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}
/*是否是无线客户端*/
RET_INFO cgi_isWifiClient_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;	
	PIU8 modules[] = 
	{		
		MODULE_GET_ISWIFICLIENT,
	};	
	cJSON_AddItemToObject(root, "isWifiClients", obj = cJSON_CreateObject());
	get_info.wp = wp;	
	get_info.root = obj;	
	get_info.modules = modules;	
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_wifiBasic_get
 功能描述  : 无线基本参数获取
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiBasic_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_BASIC,
	};

	cJSON_AddItemToObject(root, "wifiBasicCfg", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_wifiPower_get
 功能描述  : 无线功率等级获取
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiPower_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_POWER,
	};

	cJSON_AddItemToObject(root, "wifiPower", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_wifiSched_get
 功能描述  : 无线定时开关信息获取
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiSched_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_SCHED,
		MODULE_GET_WIFI_RELAY_TYPE,
	};

	cJSON_AddItemToObject(root, "wifiTime", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}


/*****************************************************************************
 函 数 名  : cgi_wifiParam_get
 功能描述  : 无线高级参数获取
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiParam_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_ADV_CFG,
		MODULE_GET_WIFI_RELAY_TYPE,
	};

	cJSON_AddItemToObject(root, "wifiAdvCfg", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_wps_hasmodule
 功能描述  : 获取是否支持wps模块
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年03月21日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wps_hasmodule(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	cJSON_AddItemToObject(root, "wpsModule", obj = cJSON_CreateObject());
#ifdef __CONFIG_WPS_RTK__
	//支持wps模块
	cJSON_AddStringToObject(obj,LIB_WPS_HAS_MODE,"true");
#else
	//支持wps模块
	cJSON_AddStringToObject(obj,LIB_WPS_HAS_MODE,"false");
#endif
	return RET_SUC;
}

#ifdef __CONFIG_WPS_RTK__
/*****************************************************************************
 函 数 名  : cgi_wifiWps_get
 功能描述  : 获取WPS信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiWps_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_WPS,
	};

	cJSON_AddItemToObject(root, "wifiWPS", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_wifiWps_set
 功能描述  : 设置WPS参数
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiWps_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] =
	{
		MODULE_SET_WIFI_WPS,
	};

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, NULL);
	return (RET_SUC);
}
#endif

/*****************************************************************************
 函 数 名  : cgi_wifiBasic_set
 功能描述  : 设置无线基本参数
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiBasic_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] =
	{
		MODULE_SET_WIFI_BASIC,
	};

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, NULL);
	return (RET_SUC);
}

/*****************************************************************************
 函 数 名  : cgi_wifiPower_set
 功能描述  : 设置无线功率
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiPower_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] =
	{
		MODULE_SET_WIFI_POWER,
	};

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, NULL);
	return (RET_SUC);
}

/*****************************************************************************
 函 数 名  : cgi_wifiSched_set
 功能描述  : 设置无线定时开关
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiSched_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] =
	{
		MODULE_SET_WIFI_SCHED,
	};

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, NULL);
	return (RET_SUC);
}

/*****************************************************************************
 函 数 名  : cgi_wifiParam_set
 功能描述  : 设置无线高级参数
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifiParam_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] =
	{
		MODULE_SET_WIFI_ADV_CFG,
	};

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, NULL);
	return (RET_SUC);
}

#ifdef __CONFIG_AUTO_CONN_CLIENT__
/*****************************************************************************
 函 数 名  : cgi_auto_sync_info_get
 功能描述  : 自动配置同步信息获取模块
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年3月25日
    作    者   : fh
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_auto_sync_info_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_AUTO_SYNC_INFO,
	};

	cJSON_AddItemToObject(root, "synchroStatus", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}
#endif

#ifdef __CONFIG_GUEST__
/*****************************************************************************
 函 数 名  : cgi_wifi_guest_info_get
 功能描述  : 获取访客网络基本信息
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifi_guest_info_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_WIFI_GUEST_INFO,
	};

	cJSON_AddItemToObject(root, "wifiGuest", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_wifi_guest_info_set
 功能描述  : set访客网络基本信息
 输入参数  : webs_t wp    
             CGI_MSG_MODULE *msgt  
             char *err_code
             void *info    
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_wifi_guest_info_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] =
	{
		MODULE_SET_WIFI_GUEST_INFO,
	};

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, NULL);

	return RET_SUC;
}
#endif

#ifdef __CONFIG_WL_BEAMFORMING_EN__
/*****************************************************************************
 函 数 名  : cgi_beaforming_enable_get
 功能描述  : 获取beaforming开关
 输入参数  : webs_t wp    
             cJSON *root  
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_beaforming_enable_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] =
	{
		MODULE_GET_BEAFORMING_ENABLE,
	};

	cJSON_AddItemToObject(root, "wifiBeamforming", obj = cJSON_CreateObject());

	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info, NULL);
	return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_beaforming_enable_set
 功能描述  : set beaforming
 输入参数  : webs_t wp    
             CGI_MSG_MODULE *msgt  
             char *err_code
             void *info   
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_beaforming_enable_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
	CGI_LIB_INFO set_info;
	PIU8 modules[] =
	{
		MODULE_SET_BEAFORMING_ENABLE,
	};

	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info, msg, err_code, NULL);

	return RET_SUC;
}
#endif

