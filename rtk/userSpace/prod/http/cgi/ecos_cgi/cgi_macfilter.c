#include "cgi_lib.h"
#if 0
PIU8 set_modules_macFilter[] =
{
    MODULE_SET_MACFILTER,
    MODULE_SET_END,
};

PIU8 get_modules_macFilter[] =
{
    MODULE_GET_MACFILTER_MODE,
    MODULE_GET_MACFILTER_LIST,
    MODULE_GET_END,
};
#endif
/*****************************************************************************
 函 数 名  : cgi_macfilter_set
 功能描述  : 设置mac过滤的ui接口函数
 输入参数  : webs_t wp
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO  cgi_macfilter_set(webs_t wp, CGI_MSG_MODULE *msg, char *err_code, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO set_info;
    PIU8 modules[] =
    {
        MODULE_SET_MACFILTER,
    };

    set_info.wp = wp;
    set_info.root = NULL;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info,msg,err_code,NULL);
    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_get_macfilter_list
 功能描述  : 页面获取macfilter列表的函数接口，里面区分AP和route模式
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年11月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_get_macfilter_list(webs_t wp, cJSON *root, void *info)
{
    cJSON *obj = NULL;
    CGI_LIB_INFO get_info;
    PIU8 modules[] =
    {
        MODULE_GET_MACFILTER_MODE,
        MODULE_GET_MACFILTER_LIST,
    };

    cJSON_AddItemToObject(root, T("macFilter"), obj = cJSON_CreateObject());

    get_info.wp = wp;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info,NULL);
    return RET_SUC;
}

