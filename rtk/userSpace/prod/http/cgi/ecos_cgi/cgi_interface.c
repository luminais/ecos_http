/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_config.c
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2016年12月8日
  最近修改   :
  功能描述   :

  功能描述   : 用于页面set和get的接口函数

  修改历史   :
  1.日    期   : 2016年12月8日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/
#include "webs.h"
#include "cgi_common.h"
#include "flash_cgi.h"
extern CGI_MODULE_INFO cgi_set_public_modules[];
extern CGI_MODULE_INFO cgi_get_public_modules[];
extern CGI_MODULE_INFO cgi_set_private_modules[];

/*****************************************************************************
 函 数 名  : formGet
 功能描述  : 处理页面的GET请求
 输入参数  : webs_t wp
             char_t *path
             char_t *query
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void formGet(webs_t wp, char_t *path, char_t *query)
{
    PIU32 index;
    PI8 searched = 0;
    cJSON *send = NULL;
    PI8_P modules = NULL;
    PI8_P send_buf = NULL;
    PI8_P cur_module = NULL;

    send = cJSON_CreateObject();
    if(NULL == send)
    {
        return ;
    }

    modules = websGetVar(wp, "modules", T(""));

    while(modules != NULL)
    {
        /* 获取单个模块，页面请求格式为modules=XXX,YYY */
        cur_module = strsep(&modules,",");
        if(cur_module)
        {
            for(index = 0; cgi_get_public_modules[index].type != CGI_NONE; ++index)
            {
                /* 查找对应的请求模块 */
                if(0 == strcmp(cur_module, cgi_get_public_modules[index].name))
                {
                    searched = 1;
                    CGI_GET_FUN(&cgi_get_public_modules[index])(wp, send, NULL);
                    break;
                }
            }
        }
    }

    websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
    if(searched == 1)
    {
        send_buf = cJSON_Print(send);
        if(NULL != send_buf)
        {
            websWriteLongString(wp, send_buf);
        }

    }

    websDone(wp, 200);

    if(NULL != send)
    {
        cJSON_Delete(send);
        send = NULL;
    }
    if(searched == 1)
        FREE_P(&send_buf);

    return ;
}

/*****************************************************************************
 函 数 名  : formSet
 功能描述  : 处理页面的set请求
 输入参数  : webs_t wp
             char_t *path
             char_t *query
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月8日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void formSet(webs_t wp, char_t *path, char_t *query)
{

    PIU32 index = 0;
    PIU32 module_index = 0;
    PI8_P cur_module = NULL;
    RET_INFO retval = RET_SUC;
    PI8 ret_buf[PI_BUFLEN_32] = {0};
    PI8 err_code[PI_BUFLEN_32] = {0};
    PI8 module_name[PI_BUFLEN_16] = {0};
    PI8 private_event[PI_BUFLEN_32]  = {0};
    CGI_MSG_MODULE msg_list[MAX_MSG_NUM];

    memset((char *)&msg_list, 0x0, MAX_MSG_NUM * sizeof(CGI_MSG_MODULE));
    for(index = 1; index <= MAX_MODULE_NUM; ++index)
    {
        memset(module_name, '\0', PI_BUFLEN_16 * sizeof(char));
        snprintf(module_name, PI_BUFLEN_16, "module%d", index);
        /* 获取当前子模块的模块名 */
        cur_module = websGetVar(wp, module_name, T(""));

        if(0 == strcmp(cur_module, ""))
        {
            continue;
        }

        for(module_index = 0; cgi_set_public_modules[module_index].type != CGI_NONE; ++module_index)
        {
            /* 查找对应的请求模块 */
            if(CGI_SET == cgi_set_public_modules[module_index].type && 0 == strcmp(cur_module, cgi_set_public_modules[module_index].name))
            {
                retval &= CGI_SET_FUN(&cgi_set_public_modules[module_index])(wp, msg_list, err_code,NULL);
                break;
            }
        }
    }

    for(module_index = 0; cgi_set_private_modules[module_index].type != CGI_NONE; ++module_index)
    {
        if(CGI_SET == cgi_set_private_modules[module_index].type && strstr(wp->path, cgi_set_private_modules[module_index].name))
        {
            /*处理当前的私有模块*/
            memset(private_event,0x0,PI_BUFLEN_32);
            CGI_SET_FUN(&cgi_set_private_modules[module_index])(wp, msg_list, err_code,private_event);
            break;
        }
    }

    _COMMIT();
    /* 判断私有模块的时间，当前使用了skip_send，标识在私有模块中已经执行了功能
    模块的消息发送，此处不需要进行消息发送操作 */

    sprintf(ret_buf, "{\"errCode\":\"%s\"}", err_code);
    websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
    websWrite(wp,"%s", ret_buf);
    websDone(wp, 200);
	
	for(index = 0; index < MAX_MSG_NUM; ++index)
    {
    	if(msg_list[index].id != 0)
        {
        	printf("=====msg_list[index].id:%d, msg_list[index].msg[%s]==========%s [%d]\n",msg_list[index].id, msg_list[index].msg, __FUNCTION__, __LINE__);
        	msg_send(MODULE_RC, msg_list[index].id, msg_list[index].msg);
        }
    }
	
    return ;
}


