/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_lib_wan.c
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2016年12月12日
  最近修改   :
  功能描述   :

  功能描述   : wan口的最小功能单元的get和set库

  修改历史   :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 创建文件

******************************************************************************/
#include "cgi_lib.h"
#include <wan.h>
#include <router_net.h>
#include <netinet/if_ether.h>
/*****************************************************************************
 函 数 名  : cgi_lib_get_wan_type
 功能描述  : 获取wan口接入类型
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
extern int APP_DEBUG;
RET_INFO cgi_lib_get_wan_type(webs_t wp, cJSON *root, void *info)
{

    P_WAN_INFO_STRUCT wan_common_info = NULL;
    wan_common_info = gpi_wan_get_info();
    cJSON_AddStringToObject(root,LIB_WAN_TYPE,wan_common_info->wan_proto);

    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_adsl_info
 功能描述  : 获取pppoe账号和密码
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_adsl_info(webs_t wp, cJSON *root, void *info)
{
    char result[2048]    = { 0 };
    char value[PI_BUFLEN_256]      = { 0 };
    P_WAN_INFO_STRUCT wan_common_info = NULL;

    wan_common_info = gpi_wan_get_info();

    memset(value, 0, sizeof(value));
    cJSON_AddStringToObject(root,LIB_PPPOE_USER,wan_common_info->wan_pppoe_info.wan_pppoe_username);

    memset(value, 0, sizeof(value));
    cJSON_AddStringToObject(root,LIB_PPPOE_PWD,wan_common_info->wan_pppoe_info.wan_pppoe_password);

    return RET_SUC;

}
/*****************************************************************************
 函 数 名  : cgi_lib_get_net_info
 功能描述  : 获取wan口基本信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_net_info(webs_t wp, cJSON *root, void *info)
{
    char result[2048]    = { 0 };
    char value[PI_BUFLEN_256]      = { 0 };
    P_WAN_INFO_STRUCT wan_common_info = NULL;

    wan_common_info = gpi_wan_get_info();
    //wanIP
    memset(value, 0, sizeof(value));
    snprintf(value, sizeof(value), "%s", wan_common_info->wan_cur_ipaddr);
    if (strncmp(value, "0.0.0.0", strlen("0.0.0.0")) == 0)
    {
        strcpy(value,"");
    }
    cJSON_AddStringToObject(root,LIB_WAN_IP,value);



    //wanMask
    memset(value, 0, sizeof(value));
    snprintf(value, sizeof(value), "%s", wan_common_info->wan_cur_mask);
    if (strncmp(value, "0.0.0.0", strlen("0.0.0.0")) == 0)
    {
        strcpy(value,"");
    }
    cJSON_AddStringToObject(root,LIB_WAN_MASK,value);

    //wan gateway
    memset(value, 0, sizeof(value));
    snprintf(value, sizeof(value), "%s", wan_common_info->wan_cur_gw);
    if (strncmp(value, "0.0.0.0", strlen("0.0.0.0")) == 0)
    {
        strcpy(value,"");
    }
    cJSON_AddStringToObject(root,LIB_WAN_GATEWAY,value);

    //wanDns1
    cJSON_AddStringToObject(root,LIB_WAN_DNS1,wan_common_info->wan_cur_dns1);

    //wanDns2
    cJSON_AddStringToObject(root,LIB_WAN_DNS2,wan_common_info->wan_cur_dns2);

    return RET_SUC;

}

/*****************************************************************************
 函 数 名  : cgi_lib_get_wan_detection
 功能描述  : 获取wan口的接入状态
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wan_detection(webs_t wp, cJSON *root, void *info)
{
    P_WAN_ERR_INFO_STRUCT wan_err_info = NULL;
    char wanDetection[32] = {0};

    wan_err_info = gpi_wan_get_err_info_other();

    if(COMMON_NO_WIRE == wan_err_info->code)
    {
        sprintf(wanDetection , "disabled" );
    }
    else
    {
        if(WAN_STATIC_MODE == wan_err_info->network_check)
            sprintf(wanDetection , "static");
        else if(WAN_DHCP_MODE == wan_err_info->network_check)
            sprintf(wanDetection , "dhcp");
        else if(WAN_PPPOE_MODE == wan_err_info->network_check)
            sprintf(wanDetection , "pppoe");
        else if(WAN_MAX_MODE == wan_err_info->network_check)
            sprintf(wanDetection , "detecting");
    }


    cJSON_AddStringToObject(root,LIB_WAN_DETECTION,wanDetection);

    return RET_SUC;

}
/*****************************************************************************
 函 数 名  : cgi_lib_get_wan_server
 功能描述  : 获取pppoe server name和service name
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wan_server(webs_t wp, cJSON *root, void *info)
{
    char *value = NULL;

    _GET_VALUE("wan0_pppoe_ac", value);
    cJSON_AddStringToObject(root,LIB_WAN_SERVERNAME, value);
    _GET_VALUE("wan0_pppoe_service", value);
    cJSON_AddStringToObject(root, LIB_WAN_SERVICENAME, value);

    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_wan_mtu
 功能描述  : 获取wan mtu
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wan_mtu(webs_t wp, cJSON *root, void *info)
{
    char str_temp[128] = {0};
    WAN_CURRCET_INFO_STRUCT wan_curr_info;

    memset(&wan_curr_info, 0x0, sizeof(WAN_CURRCET_INFO_STRUCT));
    gpi_wan_get_currcet_info(&wan_curr_info);

    memset(str_temp, 0x0, sizeof(str_temp));
    sprintf(str_temp, "%d", wan_curr_info.mtu);

    cJSON_AddStringToObject(root, LIB_WAN_MTU, str_temp);
    cJSON_AddStringToObject(root, LIB_WAN_CURR_MTU, str_temp);

    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_wan_speed
 功能描述  : 获取wan口速率
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wan_speed(webs_t wp, cJSON *root, void *info)
{
    char str_temp[128] = {0};

    P_WAN_INFO_STRUCT wan_common_info = NULL;
    WAN_CURRCET_INFO_STRUCT wan_curr_info;

    memset(&wan_curr_info, 0x0, sizeof(WAN_CURRCET_INFO_STRUCT));
    gpi_wan_get_currcet_info(&wan_curr_info);
    wan_common_info = gpi_wan_get_info();

    memset(str_temp, 0x0, sizeof(str_temp));
    if(wan_common_info->wan_speed == 0)
        strcpy(str_temp,"Auto");
    else if(wan_common_info->wan_speed == 1)
        strcpy(str_temp,"-10");
    else if(wan_common_info->wan_speed == 2)
        strcpy(str_temp,"10");
    else if(wan_common_info->wan_speed == 3)
        strcpy(str_temp,"-100");
    else if(wan_common_info->wan_speed == 4)
        strcpy(str_temp,"100");
    cJSON_AddStringToObject(root, LIB_WAN_SPEED, str_temp);
    if (!strcmp(str_temp, "Auto"))
    {
        if(wan_curr_info.wan_speed  == WAN_SPEED_10_HALF)
            strcpy(str_temp,"-10");
        else if(wan_curr_info.wan_speed  == WAN_SPEED_10_FULL)
            strcpy(str_temp,"10");
        else if(wan_curr_info.wan_speed  == WAN_SPEED_100_HALF)
            strcpy(str_temp,"-100");
        else
            strcpy(str_temp,"100");
        cJSON_AddStringToObject(root, LIB_WAN_CURR_SPEED, str_temp);
    }
    else
    {
        cJSON_AddStringToObject(root, LIB_WAN_CURR_SPEED, str_temp);
    }

    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_mac_clone
 功能描述  : 获取mac克隆信息
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_mac_clone(webs_t wp, cJSON *root, void *info)
{
    char str_temp[128] = {0};

    P_WAN_HWADDR_INFO_STRUCT wan_hwaddr_info = NULL;

    wan_hwaddr_info = gpi_wan_get_hwaddr_info();

    memset(str_temp, 0x0, sizeof(str_temp));
    switch(wan_hwaddr_info->wan_hwaddr_type)
    {
        case WAN_MAC_DEFAULT:
            sprintf(str_temp, "default");
            break;
        case WAN_MAC_PC:
            sprintf(str_temp, "clone");
            break;
        case WAN_MAC_HAND:
            sprintf(str_temp, "manual");
            break;
        default:
            break;
    }
    cJSON_AddStringToObject(root, LIB_WAN_MAC_CLONE, str_temp);
    cJSON_AddStringToObject(root, LIB_WAN_MAC_ROUTER, wan_hwaddr_info->wan_default_hwaddr);
    if(wp != NULL)
    {
        if (tenda_arp_ip_to_flag(inet_addr(wp->ipaddr)) )  //wifi
        {
            cJSON_AddStringToObject(root, LIB_WAN_MAC_EIFI_DEV, "true");
        }
        else
        {
            cJSON_AddStringToObject(root, LIB_WAN_MAC_EIFI_DEV, "false");
        }
    }
    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_wan_mac
 功能描述  : 获取wan口的mac地址
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_wan_mac(webs_t wp, cJSON *root, void *info)
{
    P_WAN_HWADDR_INFO_STRUCT wan_hwaddr_info = NULL;

    wan_hwaddr_info = gpi_wan_get_hwaddr_info();
    cJSON_AddStringToObject(root, LIB_WAN_MAC_CUR_WAN, wan_hwaddr_info->wan_hwaddr);

    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_native_host_mac
 功能描述  : 获取访问路由器本机的mac
 输入参数  : webs_t wp
             cJSON *root
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_get_native_host_mac(webs_t wp, cJSON *root, void *info)
{
    char pcMac[20];
    char mm[20]= {0};
    if(wp != NULL)
    {
        ((struct in_addr*) pcMac)->s_addr=inet_addr(wp->ipaddr);
        if(arpioctl(SIOCGARPRT, pcMac, NULL) == 0)
        {
            sprintf(mm,"%02X:%02X:%02X:%02X:%02X:%02X",
                    pcMac[4]&0XFF,
                    pcMac[5]&0XFF,
                    pcMac[6]&0XFF,
                    pcMac[7]&0XFF,
                    pcMac[8]&0XFF,
                    pcMac[9]&0XFF);
        }
        else
        {
            sprintf(mm,"%02X:%02X:%02X:%02X:%02X:%02X",
                    0,
                    0,
                    0,
                    0,
                    0,
                    0);
        }
        cJSON_AddStringToObject(root, LIB_WAN_MAC_HOST, mm);
    }
    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_set_wan_access
 功能描述  : 设置wan口的接入方式
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wan_access(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t  *con_type;

    con_type = cgi_lib_get_var(wp,root, T(LIB_WAN_TYPE), T(""));
    int sys_reboot = 0;
    int tm = 0;
	int wan_mod = 0;
	int old_wan_mode = 0;
    char val[128];
    char dns3[128];
    P_WAN_INFO_STRUCT wan_common_info = NULL;
    old_wan_mode    = get_wan_type();
    wan_common_info = gpi_wan_get_info();
    old_wan_mode    = wan_common_info->wan_type;
    _SET_VALUE(WAN0_PROTO, con_type);
    _SET_VALUE(WAN0_INDEX, con_type);
    memset(val, 0x0, sizeof(val));
    if (strcmp(con_type, "") != 0)
    {
        if (strcmp(con_type, "static") == 0)
        {
            char_t *wan_ip, *wan_msk, *gw, *dns1, *dns2, *static_mtu;
            wan_ip  = cgi_lib_get_var(wp,root, T(LIB_WAN_IP), T("0.0.0.0"));
            wan_msk = cgi_lib_get_var(wp,root, T(LIB_WAN_MASK), T("0.0.0.0"));
            gw      = cgi_lib_get_var(wp,root, T(LIB_WAN_GATEWAY), T("0.0.0.0"));
            dns1    = cgi_lib_get_var(wp,root, T(LIB_WAN_DNS1), T(""));
            dns2    = cgi_lib_get_var(wp,root, T(LIB_WAN_DNS2), T(""));

            wan_mod = 1;
            _GET_VALUE(WAN_STATIC_MTU, static_mtu);
            sprintf(dns3, "%s %s", dns1, dns2);
            sprintf(val, "%s %s", wan_common_info->wan_static_info.wan_static_dns1, wan_common_info->wan_static_info.wan_static_dns2);
            if ((strcmp(wan_common_info->wan_static_info.wan_static_ip, wan_ip) != 0) ||
                (strcmp(wan_common_info->wan_static_info.wan_static_mask, wan_msk) != 0) ||
                (strcmp(wan_common_info->wan_static_info.wan_static_gw, gw) != 0) ||
                (strcmp(val, dns3) != 0))
            {
                sys_reboot = 1;
                _SET_VALUE(WAN0_IPADDR, wan_ip);
                _SET_VALUE(WAN0_NETMASK, wan_msk);
                _SET_VALUE(WAN0_GATEWAY, gw);
                _SET_VALUE("wan0_static_dns", dns3);
            }
        }
        else if (strcmp(con_type, "dhcp") == 0)     /* DHCP~{D#J=~} */
        {
            char_t *DynStatic_mtu, *dhcp_mtu;
            wan_mod = 2;
            if (old_wan_mode != wan_mod)
            {
                sys_reboot = 1;
            }
        }
#ifdef __CONFIG_PPPOE__                                         /* added by yp 2016-3-17 */
        else if (strcmp(con_type, "pppoe") == 0)        /* PPPOE ~{D#J=~} */
        {
            char_t *user_id, *pwd, *old_username, *old_passwd, *mtu, *ac, *sev, *conmode;
            char_t *idle_time, *hour_s, *min_s, *hour_e, *min_e;
            char_t *v12_time;
            char tmp_xkjs_user_id[64];
            char tmp_xkjs_pwd[64];

            wan_mod = 3;
            user_id   = cgi_lib_get_var(wp,root, T(LIB_PPPOE_USER), T(""));
            pwd       = cgi_lib_get_var(wp,root, T(LIB_PPPOE_PWD), T(""));
            ac        = cgi_lib_get_var(wp,root, T("AC"), T(""));
            sev       = cgi_lib_get_var(wp,root, T("SVC"), T(""));
            conmode   = cgi_lib_get_var(wp,root, T("PCM"), T("0"));
            idle_time = cgi_lib_get_var(wp,root, T("PIDL"), T("60"));

            hour_s   = cgi_lib_get_var(wp,root, T("hour1"), T("0"));
            min_s    = cgi_lib_get_var(wp,root, T("minute1"), T("0"));
            hour_e   = cgi_lib_get_var(wp,root, T("hour2"), T("0"));
            min_e    = cgi_lib_get_var(wp,root, T("minute2"), T("0"));
            v12_time = cgi_lib_get_var(wp,root, T("v12_time"), T("0"));

            /*获取前一次拨号的账户名和密码*/
            _GET_VALUE(WAN0_PPPOE_USERNAME, old_username);
            _GET_VALUE(WAN0_PPPOE_PASSWD, old_passwd);

            if ((strcmp(wan_common_info->wan_pppoe_info.wan_pppoe_username, user_id) != 0) ||
                (strcmp(wan_common_info->wan_pppoe_info.wan_pppoe_password, pwd) != 0))
            {
                sys_reboot = 1;
                slprintf(tmp_xkjs_user_id, sizeof(tmp_xkjs_user_id), "%.*v", PI_BUFLEN_64, wan_common_info->wan_pppoe_info.wan_pppoe_username);
                slprintf(tmp_xkjs_pwd, sizeof(tmp_xkjs_pwd), "%.*v", PI_BUFLEN_64, wan_common_info->wan_pppoe_info.wan_pppoe_password);
                if (strcmp(user_id, tmp_xkjs_user_id) == 0)
                {
                }
                else
                {
                    _SET_VALUE(WAN0_PPPOE_USERNAME, user_id);
                }
                if (strcmp(pwd, tmp_xkjs_pwd) == 0)
                {
                }
                else
                {
                    _SET_VALUE(WAN0_PPPOE_PASSWD, pwd);
                }
                _GET_VALUE(WAN_PPPOE_MTU, mtu);
                _SET_VALUE(WAN0_PPPOE_MTU, mtu);
                _SET_VALUE(WAN0_PPPOE_MRU, mtu);
                _SET_VALUE(WAN0_PPPOE_AC, ac);
                _SET_VALUE(WAN0_PPPOE_SERVICE, sev);
                _SET_VALUE(WAN0_PPPOE_DEMAND, conmode);
                if (atoi(conmode) == PPPOE_TRAFFIC)
                {
                    _SET_VALUE(WAN0_PPPOE_IDLETIME, idle_time);
                }
                if (atoi(conmode) == PPPOE_BY_TIME)
                {
                    tm = atoi(hour_s) * 3600 + atoi(min_s) * 60;
                    sprintf(val, "%d", tm);
                    _SET_VALUE(WAN0_PPPOE_ST, val);
                    tm = atoi(hour_e) * 3600 + atoi(min_e) * 60;
                    sprintf(val, "%d", tm);
                    _SET_VALUE(WAN0_PPPOE_ET, val);
                }
#if defined(__CONFIG_CHINA_NET_CLIENT__)
                _SET_VALUE(WAN0_PPPOE_XKJX_TIME, v12_time);

                /*add by lrl 用户没有修改mac情况下，用户修改了宽带账号密码登录则触发一次自动mac克隆*/
                if(0 == strcmp(nvram_safe_get("et0macaddr"),nvram_safe_get("wan0_hwaddr")))
                {
                    if((0 != strcmp(old_username,user_id)) || (0 != strcmp(old_passwd,pwd)))
                    {
                        _SET_VALUE("restore_pppoe_first", "1");
                    }
                }
#endif
#ifdef __CONFIG_IPV6__
                /*ipv4和ipv6 pppoe用户名密码保持一致*/
                _SET_VALUE("wan6_pppoe_username", user_id);
                _SET_VALUE("wan6_pppoe_passwd", pwd);
#endif
            }
        }
#endif
        if (wan_common_info->wan_type != wan_mod)
        {
            sys_reboot = 1;
        }
        if (sys_reboot == 1)
        {
            _SET_VALUE("err_check", "0");
            _SET_VALUE("config_index", "1");
            _SET_VALUE("wan0_check", "0");
            _SET_VALUE("mode_need_switch", "no");
        }
    }

    if (sys_reboot == 1)
    {
        if (tenda_wan_link_status())
        {
            CGI_MSG_MODULE msg_tmp;
            msg_tmp.id = RC_WAN_MODULE;
            sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
            add_msg_to_list(msg,&msg_tmp);
        }
    }
    sprintf(err_code,"%s","0");
    return(RET_SUC);
}
/*****************************************************************************
 函 数 名  : cgi_lib_set_wan_mtu
 功能描述  : 设置wan口的mtu
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wan_mtu(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t  *wan_mtu;
    int wan_restart = 0;
    P_WAN_INFO_STRUCT wan_common_info = NULL;
    wan_mtu = cgi_lib_get_var(wp,root, T(LIB_WAN_MTU), "");
    wan_common_info = gpi_wan_get_info();
    /*ad by lrl 添加对设置的MTU做合法性判断*/
    if(strcmp(wan_mtu,"") && (0 == web_check_mtu(wan_mtu,wan_common_info->wan_type)))
    {
        switch(wan_common_info->wan_type)
        {
            case WAN_PPPOE_MODE:
                if(!nvram_match(WAN0_PPPOE_MTU, wan_mtu))
                {
                    _SET_VALUE(WAN_PPPOE_MTU, wan_mtu);
                    _SET_VALUE(WAN0_PPPOE_MTU, wan_mtu);
                    _SET_VALUE(WAN0_PPPOE_MRU, wan_mtu);
                    wan_restart = 1;
                }
                break;
            case PPPOEMODE2:
                if(!nvram_match(WAN0_PPPOE_MTU, wan_mtu))
                {
                    _SET_VALUE(WAN0_PPPOE_MTU, wan_mtu);
                    _SET_VALUE(WAN0_PPPOE_MRU, wan_mtu);
                    _SET_VALUE(WAN0_MTU, wan_mtu);
                    wan_restart = 1;
                }
                break;
            case L2TPMODE:
                if(!nvram_match(WAN0_L2TP_MTU, wan_mtu))
                {
                    _SET_VALUE(WAN0_L2TP_MTU, wan_mtu);
                    _SET_VALUE(WAN0_L2TP_MRU, wan_mtu);
                    wan_restart = 1;
                }
                break;
            case PPTPMODE:
            case PPTPMODE2:
                if(!nvram_match(WAN0_PPTP_MTU, wan_mtu))
                {
                    _SET_VALUE(WAN0_PPTP_MTU, wan_mtu);
                    _SET_VALUE(WAN0_PPTP_MRU, wan_mtu);
                    wan_restart = 1;
                }
                break;
            case WAN_STATIC_MODE:
                if(!nvram_match(WAN_STATIC_MTU, wan_mtu))
                {
                    _SET_VALUE(WAN_STATIC_MTU, wan_mtu);
                    _SET_VALUE(WAN0_MTU, wan_mtu);
                    wan_restart = 1;
                }
                break;
            case WAN_DHCP_MODE:
                if(!nvram_match(WAN_DHCP_MTU, wan_mtu))
                {
                    _SET_VALUE(WAN_DHCP_MTU, wan_mtu);
                    _SET_VALUE(WAN0_MTU, wan_mtu);
                    wan_restart = 1;
                }
                break;
            case C8021XMODE:
            default:
                break;
        }
    }

    if(wan_restart)
    {
        CGI_MSG_MODULE msg_tmp;
        msg_tmp.id = RC_WAN_MODULE;
        sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
        add_msg_to_list(msg,&msg_tmp);
    }

    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_set_wan_speed
 功能描述  : 设置wan口的速率
 输入参数  : webs_t wp
             cJSON *root
             CGI_MSG_MODULE *msg
             char *err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wan_speed(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t  *wan_speed;
    int modify_wan_speed_2_auto = 0;
    int wan_speed_change = 0;
    char str_temp[128] = {0};
    char wan_speed_temp[PI_BUFLEN_4] = {0};
    char msg_param[PI_BUFLEN_256] = {0};
    P_WAN_HWADDR_INFO_STRUCT wan_hwaddr_info = NULL;
    P_WAN_INFO_STRUCT wan_common_info = NULL;

    wan_speed = cgi_lib_get_var(wp,root, T(LIB_WAN_SPEED), "");
    wan_hwaddr_info = gpi_wan_get_hwaddr_info();
    wan_common_info = gpi_wan_get_info();


    if(strcmp(wan_speed,""))
    {
        if(!strcmp(wan_speed,"Auto"))
            strcpy(str_temp,"0");
        else if(!strcmp(wan_speed,"-10"))
            strcpy(str_temp,"1");
        else if(!strcmp(wan_speed,"10"))
            strcpy(str_temp,"2");
        else if(!strcmp(wan_speed,"-100"))
            strcpy(str_temp,"3");
        else if(!strcmp(wan_speed,"100"))
            strcpy(str_temp,"4");
        else
            strcpy(str_temp,"0");

        sprintf(wan_speed_temp, "%d", wan_common_info->wan_speed);
        if(strcmp(str_temp, wan_speed_temp) != 0)
        {
            if(!strcmp(wan_speed,"Auto"))
            {
                modify_wan_speed_2_auto = 1;
            }
            _SET_VALUE("wan_speed", str_temp);
            wan_speed_change = 1;
        }
    }
    _COMMIT();
    if(wan_speed_change)
    {
        printf("if wan speed change.\n");
        sprintf(msg_param, "string_info=%s", "WanSpeedChange");
        msg_send(MODULE_RC, RC_WAN_MODULE, msg_param);
        if(1 == modify_wan_speed_2_auto)
        {
            msg_waitback(MODULE_RC,RC_WAN_MODULE,100,30);
        }
    }

    //设置成功直接向页面返回0
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_set_wan_macClone
 功能描述  : 设置mac克隆
 输入参数  : webs_t wp
             CGI_MSG_MODULE * msg
             char * err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wan_macClone(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t  *wan_mac, *macClone;
    char str_temp[128] = {0};
    int wan_restart = 0;
    P_WAN_HWADDR_INFO_STRUCT wan_hwaddr_info = NULL;

    wan_mac = cgi_lib_get_var(wp,root, T(LIB_WAN_MAC), T(""));
    macClone = cgi_lib_get_var(wp,root, T(LIB_WAN_MAC_CLONE), T(""));
    wan_hwaddr_info = gpi_wan_get_hwaddr_info();
    if(strcmp(wan_mac,"") != 0 && strcmp(wan_mac, wan_hwaddr_info->wan_hwaddr) != 0)
    {
        _SET_VALUE(WAN0_MAC_ADDR,wan_mac);
        wan_restart = 1;
    }
    memset(str_temp, 0x0, sizeof(str_temp));
    switch(wan_hwaddr_info->wan_hwaddr_type)
    {
        case WAN_MAC_DEFAULT:
            sprintf(str_temp, "default");
            break;
        case WAN_MAC_PC:
            sprintf(str_temp, "clone");
            break;
        case WAN_MAC_HAND:
            sprintf(str_temp, "manual");
            break;
        default:
            break;
    }
    if(strcmp(macClone,"") != 0 && strcmp(macClone, str_temp) != 0)
    {
        _SET_VALUE(WAN0_MACCLONE_MODE,macClone);
        /*lq 如果用户在第一次进行PPPOE拨号前手动执行了MAC克隆，则取消自动克隆MAC*/
#ifdef __CONFIG_CHINA_NET_CLIENT__
        _SET_VALUE("restore_pppoe_first","0");
#endif
    }

    if(wan_restart)
    {
        CGI_MSG_MODULE msg_tmp;
        msg_tmp.id = RC_WAN_MODULE;
        sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
        add_msg_to_list(msg,&msg_tmp);
    }
    //设置成功直接向页面返回0
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_set_wan_server
 功能描述  : 设置pppoe service name和server name
 输入参数  : webs_t wp
             CGI_MSG_MODULE * msg
             char * err_code
             void *info
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月12日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wan_server(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char_t *wan_server_name = NULL;
    char_t*wan_service_name = NULL;
    int wan_restart = 0;
    wan_server_name = cgi_lib_get_var(wp,root, T(LIB_WAN_SERVERNAME), "no_wanServerName");   //这里添加默认值是为了区分动态IP联网时克隆MAC地址不会对ServerNameerviceName字段造成影响
    wan_service_name = cgi_lib_get_var(wp,root, T(LIB_WAN_SERVICENAME), "no_wanServiceName");


    if(strcmp("no_wanServerName", wan_server_name))
    {
        if(!nvram_match(WAN0_PPPOE_AC, wan_server_name))
        {
            _SET_VALUE(WAN0_PPPOE_AC, wan_server_name);
            wan_restart = 1;
        }
    }
    if(strcmp("no_wanServiceName", wan_service_name))
    {
        if(!nvram_match(WAN0_PPPOE_SERVICE, wan_service_name))
        {
            _SET_VALUE(WAN0_PPPOE_SERVICE, wan_service_name);
            wan_restart = 1;
        }
    }

    if(wan_restart)
    {
        CGI_MSG_MODULE msg_tmp;
        msg_tmp.id = RC_WAN_MODULE;
        sprintf(msg_tmp.msg, "op=%d",OP_RESTART);
        add_msg_to_list(msg,&msg_tmp);
    }
    //设置成功直接向页面返回0
    if(!err_code[0])
    {
        strcpy(err_code, "0");
    }
    return RET_SUC;
}

