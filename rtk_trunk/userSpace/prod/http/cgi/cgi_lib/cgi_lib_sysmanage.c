/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_lib_sysmanage.c
  版 本 号   : 初稿
  作    者   : 段靖铖
  生成日期   : 2016年12月13日
  最近修改   :
  功能描述   :

  功能描述   : sysmanage的最小功能单元的get和set库

  修改历史   :
  1.日    期   : 2016年12月13日
    作    者   : 段靖铖
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <router_net.h>
#include "flash_cgi.h"
#include "webs.h"
#include "cJSON.h"
#include "cgi_common.h"
#include "sys_module.h"
#include "http.h"
#include <autoconf.h>
#include "systools.h"
#include "bcmsntp.h"
#include <tz_info.h>
#include "cgi_lib.h"

extern char g_Pass[64];
extern char change_ip[];

#define TIME_ZONES_NUMBER 75
/*****************************************************************************
 函 数 名  : cgi_lib_get_isWifiClient
 功能描述  : 获取是否是无线客户端
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : luorilin
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wziard_systime(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    int   i = 0 ;
    int i_timezone;
    float ftime_zone;
    char   value_timezone[15] = {0};
    char*  time_zone = cgi_lib_get_var(wp,root, T(LIB_SYSTIMEZONE), T(""));

    if(time_zone[0] != 0)
    {
        ftime_zone = atof(time_zone);
        i_timezone = ftime_zone * 3600;

        for(i = 0 ; i < TIME_ZONES_NUMBER ; i++ )
        {


            if(time_zones[i].tz_offset == i_timezone )
            {
                sprintf(value_timezone , "%d" , time_zones[i].index );

                //diag_printf("Fast set : timeZone = %s, index=%s\n", time_zone, value_timezone);
                _SET_VALUE(FUNCTION_TIME_ZONE, value_timezone);
                sntp_renew_time_from_server();
                break;
            }
        }
    }

    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }

    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_remote_info
 功能描述  : 获取远程管理基本信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_remote_info(webs_t wp, cJSON *root, void *info)
{

    char *value = NULL;

    _GET_VALUE(ADVANCE_RM_WEB_EN, value);
    if (0 == strcmp(value, "0"))
        cJSON_AddStringToObject(root,LIB_REMOTE_ENABLE, "false");
    else
        cJSON_AddStringToObject(root,LIB_REMOTE_ENABLE, "true");
    _GET_VALUE(ADVANCE_RM_WEB_IP, value);

    if (0 == strcmp(value, "0.0.0.0") || strlen(value) < 7)
    {
        cJSON_AddStringToObject(root,LIB_REMOTE_TYPE, "any");
        cJSON_AddStringToObject(root,LIB_REMOTE_IP, "");
    }
    else
    {
        cJSON_AddStringToObject(root,LIB_REMOTE_TYPE, "specified");
        cJSON_AddStringToObject(root,LIB_REMOTE_IP, value);
    }
    _GET_VALUE(ADVANCE_RM_WEB_PORT, value);
    cJSON_AddStringToObject(root,LIB_REMOTE_PORT, value);

    return RET_SUC;

}


/*****************************************************************************
 函 数 名  : cgi_lib_set_remote_info
 功能描述  : 设置远程管理功能
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_remote_info(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{

    char_t *remoteWebEn, *remoteWebType, *remoteWebIP, *remoteWebPort;
    int remoteWeb_restart = 0;

    remoteWebEn = cgi_lib_get_var(wp,root, T(LIB_REMOTE_ENABLE), "");
    remoteWebType = cgi_lib_get_var(wp,root, T(LIB_REMOTE_TYPE), "");
    remoteWebIP = cgi_lib_get_var(wp,root, T(LIB_REMOTE_IP), "");
    remoteWebPort = cgi_lib_get_var(wp, root,T(LIB_REMOTE_PORT), "");
    printf("fun=%s   line=%d     %s\n",__FUNCTION__,__LINE__,remoteWebEn);
    if ( !strcmp(remoteWebEn, "false"))
    {
        remoteWebEn = "0";
    }
    else
    {
        remoteWebEn = "1";
        if ( !nvram_match(ADVANCE_RM_WEB_PORT, remoteWebPort))
        {
            remoteWeb_restart = 1;
            _SET_VALUE(ADVANCE_RM_WEB_PORT, remoteWebPort);
        }
        if (!strcmp(remoteWebType, "any")) //页面没有对ip 做模式下方校验
        {
            remoteWebIP = "0.0.0.0";
        }

        if ( !nvram_match(ADVANCE_RM_WEB_IP, remoteWebIP))
        {
            remoteWeb_restart = 1;
            _SET_VALUE(ADVANCE_RM_WEB_IP, remoteWebIP);
        }
    }
    if ( !nvram_match(ADVANCE_RM_WEB_EN, remoteWebEn))
    {
        remoteWeb_restart = 1;
        _SET_VALUE(ADVANCE_RM_WEB_EN, remoteWebEn);
        printf("fun=%s   line=%d\n",__FUNCTION__,__LINE__);
        if (!strcmp(remoteWebEn, "0")) //disable后, port初始化为8080
        {
            _SET_VALUE(ADVANCE_RM_WEB_PORT, "8080");
        }
    }

    if( remoteWeb_restart )
    {
        if (strcmp(remoteWebEn, "1") == 0)
        {
            printf("remote web (re)start.\n");
            LoadWanListen(1);
        }
        else
        {
            printf("remote web disabled.\n");
            LoadWanListen(0);
        }


        CGI_MSG_MODULE msg_tmp;
        msg_tmp.id = RC_FIREWALL_MODULE;
        sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
        add_msg_to_list(msg,&msg_tmp);
    }
    //设置成功后，向页面返回0
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
    return RET_SUC;

}


/*****************************************************************************
 函 数 名  : cgi_sysmanage_fireware_get
 功能描述  : 获取当前版本信息
 输入参数  : webs_t wp
             cJSON *root
             void * info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/

RET_INFO cgi_lib_get_fireware(webs_t wp, cJSON *root, void * info)
{
    char  str_temp[64] = {0};


#if defined(__CONFIG_WEB_VERSION__)
    sprintf(str_temp, "%s_%s", W311R_ECOS_SV,NORMAL_WEB_VERSION);
#else
    sprintf(str_temp, "%s", W311R_ECOS_SV);
#endif
    cJSON_AddStringToObject(root,LIB_SOFT_VER, str_temp);
#ifdef __CONFIG_RESTART_CHECK__
    if (1 == gpi_reboot_check_enable())
        cJSON_AddStringToObject(root,LIB_AUTO_MAINT_ENABLE, "true");
    else
        cJSON_AddStringToObject(root,LIB_AUTO_MAINT_ENABLE, "false");
#endif

    return RET_SUC;

}

/*****************************************************************************
 函 数 名  : cgi_lib_set_fireware
 功能描述  : 设置固件版本信息
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月14日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_fireware(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t *auto_reboot_en;
    int restart_check_temp = 0;
    auto_reboot_en = cgi_lib_get_var(wp,root,T(LIB_AUTO_MAINT_ENABLE), "false");
    if (0 == strcmp(auto_reboot_en, "true"))
    {
        if(1 != gpi_reboot_check_enable())
        {
            restart_check_temp = 1;
            _SET_VALUE("reboot_enable", "enable");
        }
    }
    else
    {
        if(1 == gpi_reboot_check_enable())
        {
            restart_check_temp = 1;
            _SET_VALUE("reboot_enable", "disable");
        }
    }

    if(restart_check_temp)
    {
        CGI_MSG_MODULE msg_tmp;
        msg_tmp.id = RC_REBOOT_CHECK_MODULE;
        sprintf(msg_tmp.msg, "op=%d", OP_RESTART);
        add_msg_to_list(msg, &msg_tmp);
    }

    if(!err_code[0])
    {
        strcpy(err_code, "100");
    }
    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_operate
 功能描述  : 重启，恢复出厂设置功能
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月14日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_operate(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char ret_buf[32] = {0};
    char *v = NULL;
    char * action = NULL;

    action = cgi_lib_get_var(wp,root, T("action"), "");

    if(0 == strcmp(action,"reboot"))
    {
        sprintf(msg->msg,"string_info=%s","reboot");
    }
    else if(0 == strcmp(action,"restore"))
    {
        _SET_VALUE(SYSCONFIG_RESTORE,"1");
        _SET_VALUE(LAN_IPADDR,_GET_DEFAULT(LAN_IPADDR,v));

#ifdef A5
        _SET_VALUE("vlan1ports", "1 2 3 4 5*");
        _SET_VALUE("vlan2ports", "0 5");
#endif

        _COMMIT();
        sprintf(msg->msg,"string_info=%s","restore");
    }
    msg->id = RC_SYSTOOLS_MODULE;
    if(!err_code[0])
    {
        strcpy(err_code, "100");
    }

    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_systime
 功能描述  : 获取系统时间
 输入参数  : webs_t wp
             cJSON *root
             void * info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月14日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_systime(webs_t wp, cJSON *root, void * info)
{
    char str_temp[128] = {0};
    P_SNTP_INFO_STRUCT sntp_info = NULL;

    sntp_info = gpi_sntp_info();

    cJSON_AddStringToObject(root, LIB_SYS_TIME_ZONE, sntp_info->time_zone);
    struct tm *cur_time_tm;
    time_t  cur_time_t = time(0);
    cur_time_tm = localtime(&cur_time_t);
    memset(str_temp, 0x0, sizeof(str_temp));
    snprintf(str_temp,sizeof(str_temp),"%d-%02d-%02d %02d:%02d:%02d" , 1900+cur_time_tm->tm_year ,cur_time_tm->tm_mon+1 ,
             cur_time_tm->tm_mday, cur_time_tm->tm_hour , cur_time_tm->tm_min , cur_time_tm->tm_sec);
    cJSON_AddStringToObject(root, LIB_SYS_CURRENT_TIME, str_temp);
    memset(str_temp, 0x0, sizeof(str_temp));
    switch(sntp_info->enable)
    {
        case 0:
            strcpy(str_temp, "auto");
            break;
        case 1:
            strcpy(str_temp, "manual");
            break;
        default:
            break;
    }
    cJSON_AddStringToObject(root, LIB_SYS_SNTP_TYPE, str_temp);
    if(gpi_common_time_update_result()!= 1)
    {
        cJSON_AddStringToObject(root,LIB_SYS_INTER_STATE,"false");
    }
    else
    {
        cJSON_AddStringToObject(root,LIB_SYS_INTER_STATE,"true");
    }
    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_set_systime
 功能描述  : 设置系统时间
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月14日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_systime(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t *timeZone;
    int restart_wan = 0;
    char msg_param[PI_BUFLEN_256] = {0};


    P_SNTP_INFO_STRUCT sntp_info = NULL;
    sntp_info = gpi_sntp_info();

    timeZone = cgi_lib_get_var(wp,root,T(LIB_SYS_TIME_ZONE), "");

    if(strcmp(sntp_info->time_zone, timeZone))
    {
        _SET_VALUE("old_time_zone", sntp_info->time_zone);//备份旧的时区，本地时间修改成功后清空
        _SET_VALUE(FUNCTION_TIME_ZONE, timeZone);
        restart_wan = 1;
    }

    if(restart_wan)
    {
        printf("timeZone change .\n");
        int i;
        for(i = 0; i < MAX_MSG_NUM; i++)
        {
            if(msg[i].id == 0)
            {
                msg[i].id = RC_SNTP_MODULE;
                sprintf(msg[i].msg, "op=%d", OP_RESTART);
                break;
            }
            else if(msg[i].id == RC_SNTP_MODULE)
            {
                break;
            }
        }
    }
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }

    return RET_SUC;

}


/*****************************************************************************
 函 数 名  : cgi_lib_get_login_pwd
 功能描述  : 获取登录密码状态
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月14日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_login_pwd(webs_t wp, cJSON *root, void *info)
{

    char  haspwd[16] = {0};



    if (strcmp(nvram_safe_get("http_defaultpwd1"), "0"))
    {
        sprintf(haspwd, "true");
    }
    else
    {
        sprintf(haspwd, "false");
    }
    cJSON_AddStringToObject(root, LIB_HAS_HTTP_PWD, haspwd);

    cJSON_AddStringToObject(root, LIB_HTTP_USERNAME, nvram_safe_get(LAN_HTTP_LOGIN_USERNAME));

    return RET_SUC;

}


/*****************************************************************************
 函 数 名  : cgi_lib_set_login_pwd
 功能描述  : 设置web登录密码
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月14日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_login_pwd(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t *new_password,*old_password;
    int pwd_temp = -1;
    int errCode = 0;

    new_password = cgi_lib_get_var(wp,root, T("newPwd"), "");
    old_password = cgi_lib_get_var(wp,root, T("oldPwd"), "");

    /*comtrast old_password and new_password */
    if(strcmp(old_password,"") == 0 && strcmp(new_password,"") == 0)
    {

    }
    else if(strcmp(old_password,"") != 0 && strcmp(new_password,"") == 0)
    {

        if(strcmp(nvram_safe_get(LAN_HTTP_LOGIN_PASSWD), old_password) == 0)
        {
            _SET_VALUE(LAN_HTTP_LOGIN_PASSWD, new_password);
            _SET_VALUE("http_defaultpwd1", "0");
            strncpy(g_Pass,new_password,sizeof(g_Pass));
            pwd_temp = 1;  //这种情况属于删除密码, 页面 errCode 返回 0
        }
        else
        {
            pwd_temp = 0;
        }
    }
    else if(strcmp(old_password,"") == 0 && strcmp(new_password,"") != 0)
    {
        if(strcmp(nvram_safe_get(LAN_HTTP_LOGIN_PASSWD),old_password) == 0)
        {
            _SET_VALUE(LAN_HTTP_LOGIN_PASSWD, new_password);
            pwd_temp = 1;
            _SET_VALUE("http_defaultpwd1", "1");
            strncpy(g_Pass,new_password,sizeof(g_Pass));
            errCode = 101; //修改密码成功
        }
        else
        {
            pwd_temp = 0;
        }
    }
    else
    {
        if(strcmp(nvram_safe_get(LAN_HTTP_LOGIN_PASSWD),old_password) == 0)
        {
            _SET_VALUE(LAN_HTTP_LOGIN_PASSWD, new_password);
            pwd_temp = 1;
            errCode = 101; //修改密码成功
            _SET_VALUE("http_defaultpwd1", "1");
            strncpy(g_Pass,new_password,sizeof(g_Pass));
        }
        else
        {
            pwd_temp = 0;
        }
    }/*end contrast*/

    if(pwd_temp == 0)   //修改失败
    {
        errCode = 2;
    }
    else if(pwd_temp == 1)  //修改密码成功
    {
        tpi_http_clear_login_time();
    }

 	sprintf(err_code,"%d",errCode);
    
    return RET_SUC;

}

/*****************************************************************************
 函 数 名  : cgi_lib_get_internet_status
 功能描述  : 获取系统联网状态
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月14日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_internet_status(webs_t wp, cJSON *root, void *info)
{
    char iterm_value[PI_BUFLEN_64];

    if(NULL == root)
    {
        printf("*** fun=%s; line=%d; no buffers! ***\n", __func__, __LINE__);
        return RET_ERR;
    }

    /* get status code of Internet */
    memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
    snprintf(iterm_value, PI_BUFLEN_64, "%d", gpi_wan_get_err_result_info());
    cJSON_AddStringToObject(root,LIB_WAN_CONNECT_STATUS, iterm_value);

    //newIP
    cJSON_AddStringToObject(root,LIB_NEW_LAN_IP,change_ip);

    // lanWanIPConflict表示冲突标志：为"true"时表示与上级IP冲突
    memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
    if(0 == strcmp(nvram_safe_get("err_check"), "11"))
    {
        snprintf(iterm_value, PI_BUFLEN_64, "true");
    }
    else
    {
        snprintf(iterm_value, PI_BUFLEN_64, "false");
    }
    cJSON_AddStringToObject(root,LIB_LAN_WAN_IP_CONFILICT, iterm_value);

    return RET_SUC;

}




