/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cjson_wan.c
  版 本 号   : 初稿
  作    者   : liusongming
  生成日期   : 2016年12月19日
  最近修改   :
  功能描述   :

  功能描述   : app获取，设置设备别名

  修改历史   :
  1.日    期   : 2016年12月19日
    作    者   : liusongming
    修改内容   : 创建文件

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "cjson_handle.h"
#include "cgi_lib_config.h"
#include "wan.h"


#define WAN_NUM 1   //WAN口个数
/****************************************************************************
函数名  :app_get_wan_basic
描述    :用于app获取WAN口基本信息
参数    :
    recv_root:
         格式:{"module":["APP_GET_WAN_BASIC"]}
    send_root:用于函数回传数据，
        格式:{"n_wan":1,"wan":[{"interfacenum":1,"type":1,"adsl_name":"ruanjian06",
        "adsl_pwd":"ruanjian06","ip_addr":0,"netmask":2148813625,"gateway":2160369456,
        "primary_dns":4,"backup_dns":0,"conn_time":0,"sta":0}]}

    info:无
日    期   : 2016年12月23日
作    者   : liusongming
修改内容   : 新建函数
****************************************************************************/

void app_get_wan_basic(cJSON *recv_root,cJSON *send_root, void *info)
{
    cJSON *obj;
    cJSON *wan = NULL,*wanitem = NULL;
    int n_wan = 0;
    int i = 0;
    char *tmp_var = NULL;
    char *adsl_name = NULL,*adsl_pwd = NULL;
    char *wanIp = NULL,*wanMask = NULL,*wanGateway = NULL,*wanDns1 = NULL,*wanDns2 = NULL;
    char *wanConnectTime = NULL;
    unsigned int ip_addr = 0,netmask = 0,gateway = 0,primary_dns = 0,backup_dns = 0;
    int conn_time = 0;
    int wanConnectStatus = 0;
    CGI_LIB_INFO get_info;

    if(NULL == recv_root)
    {
        printf("func:%s line:%d recv_root is NULL\n",__func__,__LINE__);
        return;
    }


    //调用cgi_lib库获取app需要的信息
    PIU8 modules[] =
    {
        MODULE_GET_WAN_DETECTION,
        MODULE_GET_WAN_TYPE,
        MODULE_GET_ADSL_INFO,
        MODULE_GET_NET_INFO,
        MODULE_GET_WAN_SPEED,
        MODULE_GET_WAN_MAC,
        MODULE_GET_WAN_CONNECT_TIME,
        MODULE_GET_INTERNET_STATUS,
    };

    obj = cJSON_CreateObject();
    if(NULL == obj)
    {
        printf("func:%s line:%d create obj fail\n",__func__,__LINE__);
        return;
    }

    get_info.wp = NULL;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);

    cgi_lib_get(get_info, NULL);        //调用lib库公共函数

    cJSON_AddNumberToObject(send_root,"n_wan",WAN_NUM);  //添加wan口个个数，F9只有一个WAN口
    cJSON_AddItemToObject(send_root,"wan",wan = cJSON_CreateArray()); //添加数组对象，用于存放wan口的基本信息

    //进行数据解析以及组装
    for(i = 0; i < WAN_NUM; i++)
    {
        cJSON_AddItemToArray(wan,wanitem = cJSON_CreateObject());
        if(NULL == wanitem)
        {
            printf("func:%s line:%d Create wanitem fail\n",__func__,__LINE__);
            cJSON_Delete(obj);
            return ;
        }
        cJSON_AddNumberToObject(wanitem,"interfacenum",i + 1);      //表示第几个wan口

        tmp_var = cjson_get_value(obj,"wanType","");    //获取WAN口的接入类型
        if(NULL == tmp_var)
        {
            printf("func:%s line:%d get wanType fail\n",__func__,__LINE__);
            cJSON_Delete(obj);
            return;
        }
        if(!strcmp("static",tmp_var))       //静态接入方式,app的表示方式为:pppoe:1; dhcp:2;static:3
        {
            cJSON_AddNumberToObject(wanitem,"type",STATIC);
        }

        else if(!strcmp("dhcp",tmp_var))
        {
            cJSON_AddNumberToObject(wanitem,"type",DYNAMIC);
        }
        else if(!strcmp("pppoe",tmp_var))
        {
            cJSON_AddNumberToObject(wanitem,"type",ADSL);

        }
        else
        {
            cJSON_AddNumberToObject(wanitem,"type",0);  //0代表未知类型
        }
        //获取pppoe的账户名和密码
        adsl_name = cjson_get_value(obj,LIB_PPPOE_USER,"");
        adsl_pwd = cjson_get_value(obj,LIB_PPPOE_PWD,"");
        if((adsl_name == NULL) || (adsl_pwd == NULL))
        {
            printf("get adsl name or adsl password fail\n");
            cJSON_Delete(obj);
            return;
        }
        //将账户名，密码以Cjson的形式添加
        cJSON_AddStringToObject(wanitem,"adsl_name",adsl_name);
        cJSON_AddStringToObject(wanitem,"adsl_pwd",adsl_pwd);

        //获取WAN口的ip,mask,gateway,dns
        wanIp = cjson_get_value(obj,LIB_WAN_IP,"");
        wanMask = cjson_get_value(obj,LIB_WAN_MASK,"");
        wanGateway = cjson_get_value(obj,LIB_WAN_GATEWAY,"");
        wanDns1 = cjson_get_value(obj,LIB_WAN_DNS1,"");
        wanDns2 = cjson_get_value(obj,LIB_WAN_DNS2,"");
        wanConnectTime = cjson_get_value(obj,LIB_WAN_CONNECT_TIME,"0");
        //将wan口的ip,mask,gateway,dns以CJSON的形式添加
        cJSON_AddStringToObject(wanitem,"ip_addr",wanIp);
        cJSON_AddStringToObject(wanitem,"netmask",wanMask);
        cJSON_AddStringToObject(wanitem,"gateway",wanGateway);
        cJSON_AddStringToObject(wanitem,"primary_dns",wanDns1);
        cJSON_AddStringToObject(wanitem,"backup_dns",wanDns2);
        cJSON_AddStringToObject(wanitem,"conn_time",wanConnectTime);

        //转换网络监测码,只有在未联网的情况下才会给app返回err字段，所以在app上取err字段时，必须要先判断是否是未联网
        tmp_var = cjson_get_value(obj,"wanConnectStatus","0");
        wanConnectStatus = atoi(tmp_var);
        wanConnectStatus = (wanConnectStatus % 1000)/10;
        switch(wanConnectStatus)
        {
            case COMMON_NO_WIRE:
                cJSON_AddNumberToObject(wanitem,"sta",0);  //未插网线
                break;
            case COMMON_NOT_CONNECT:
                cJSON_AddNumberToObject(wanitem,"sta",1);  //未联网
                break;
            case COMMON_CONNECTING:
                cJSON_AddNumberToObject(wanitem,"sta",2);  //联网中
                break;
            case COMMON_CONNECTED_ONLINEING:
                cJSON_AddNumberToObject(wanitem,"sta",3);  //已联网
                break;
            case COMMON_ONLINEED:
                cJSON_AddNumberToObject(wanitem,"sta",3);  //已联网
                cJSON_AddNumberToObject(wanitem,"err",0);  //app规定:0-正常
                break;
            case COMMON_NOT_ONLINE:
                cJSON_AddNumberToObject(wanitem,"sta",1);      //未联网
                cJSON_AddNumberToObject(wanitem,"err",566);    //566-无法连接到互联网
                break;
            case (COMMON_ONLINEED + 1):
                cJSON_AddNumberToObject(wanitem,"sta",1);     //未联网
                cJSON_AddNumberToObject(wanitem,"err",549);   // 549-用户名或密码错误
                break;
            case (COMMON_ONLINEED + 2):
                cJSON_AddNumberToObject(wanitem,"sta",1);             //未联网
                cJSON_AddNumberToObject(wanitem,"err",548); //0-正常， 566-无法连接到互联网，549-用户名或密码错误，548-远端服务器无响应 */
                break;
            default:
                break;
        }


    }

    cJSON_Delete(obj);

    return ;
}


/****************************************************************************
函数名  :app_set_wan_basic
描述    :用于app设置WAN口基本信息
参数    :
    send_root:用于函数回传数据，
        格式:{"n_wan":1,"wan":[{"adsl_name":"ruanjian06","adsl_pwd":"ruanjian06"}]}

        msg:
   err_code:用于存放错误码
    info:无
日    期   : 2016年12月23日
作    者   : liusongming
修改内容   : 新建函数
****************************************************************************/
void app_set_wan_basic(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
    CGI_LIB_INFO set_info;
    cJSON *obj = NULL;
    PI8 err_code[PI_BUFLEN_32] = {0};
    cJSON *wan = NULL,*wanitem = NULL;
    int type = 0;
    CGI_MSG_MODULE remov_msg;
    char *wan_ip = NULL, *gw = NULL,*wan_msk = NULL,*dns1 = NULL,*dns2 = NULL;
    char *wanPPPoEUser = NULL, *wanPPPoEPwd = NULL;

    if((NULL == send_root) || (NULL == result_code))
    {
        printf("recv_root is null\n");
        return ;
    }

    wan = cJSON_GetObjectItem(send_root,"wan");
    if(NULL == wan)
    {
        printf("func:%s line:%d get wan obj fail\n",__func__,__LINE__);
        return ;
    }
    //将app传入的数据组装成CGI要求的数据格式
    wanitem = cJSON_GetArrayItem(wan,0);  //默认取wan数组中的第一个wan口信息，双wan的情况要循环取
    obj = cJSON_CreateObject();
    if(NULL == obj)
    {
        printf("create obj fail\n");
        return;
    }
    //获取联网类型
    type = cjson_get_number(wanitem,"type",0);
    if(type == STATIC)      //静态接入
    {
        cJSON_AddStringToObject(obj, LIB_WAN_TYPE, "static");
        //获取IP
        wan_ip = cjson_get_value(wanitem,"ip_addr","0.0.0.0");
        cJSON_AddStringToObject(obj, LIB_WAN_IP, wan_ip);
        //获取子网掩码
        wan_msk = cjson_get_value(wanitem,"netmask","0.0.0.0");
        cJSON_AddStringToObject(obj, LIB_WAN_MASK, wan_msk);
        //获取网关
        gw = cjson_get_value(wanitem,"gateway","0.0.0.0");
        cJSON_AddStringToObject(obj, LIB_WAN_GATEWAY, gw);
        //获取DNS
        dns1 = cjson_get_value(wanitem,"primary_dns","");
        cJSON_AddStringToObject(obj, LIB_WAN_DNS1, dns1);

        dns2 = cjson_get_value(wanitem,"backup_dns","");
		dns2 = strncmp(dns2,"0.0.0.0",strlen("0.0.0.0")) ? dns2 : "";
        cJSON_AddStringToObject(obj, LIB_WAN_DNS2, dns2);

    }
    else if(DYNAMIC == type)    //动态接入
    {
        cJSON_AddStringToObject(obj, LIB_WAN_TYPE, "dhcp");
    }
    else if(ADSL == type)       //pppoe接入
    {
        //获取账户名，密码
        wanPPPoEUser = cjson_get_value(wanitem,"adsl_name","");
        wanPPPoEPwd = cjson_get_value(wanitem,"adsl_pwd","");

        cJSON_AddStringToObject(obj, LIB_WAN_TYPE, "pppoe");
        cJSON_AddStringToObject(obj, LIB_PPPOE_USER, wanPPPoEUser);
        cJSON_AddStringToObject(obj, LIB_PPPOE_PWD, wanPPPoEPwd);
    }
    else        //未知类型
    {
        printf("unknow type\n");
        cJSON_Delete(obj);
        return ;
    }

    //调用lib库获取基本信息
    PIU8 modules[] =
    {
        MODULE_SET_WAN_ACCESS,
    };
    set_info.wp = NULL;
    set_info.root = obj;
    set_info.modules = modules;
    set_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_set(set_info, msg, err_code, NULL);  //调用公共set函数设置

    memset(remov_msg.msg,0x0,MAX_MODULE_MSG_MAX_LEN);
    remov_msg.id = RC_WAN_MODULE;
    remove_msg_to_list(msg,remov_msg);  //移除WAN模块相关的信息

    if(strcmp(err_code,"0"))
        *result_code = 1;

    cJSON_Delete(obj);
    return ;
}
/****************************************************************************
函数名  :app_set_wan_basic_process
描述    :用于app设置wan口之后使设置生效
参数    :
    send_root:

    info:无
日    期   : 2016年12月23日
作    者   : liusongming
修改内容   : 新建函数
****************************************************************************/
void app_set_wan_basic_process(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
    CGI_MSG_MODULE msg_tmp;
    msg_tmp.id = RC_WAN_MODULE;
    sprintf(msg_tmp.msg, "op=%d", OP_RESTART);
    add_msg_to_list(msg, &msg_tmp);
}



/****************************************************************************
函数名  :app_get_wan_rate
描述    :用于app获取WAN口速率
参数    :
    send_root:用于函数回传数据，

    info:无
日    期   : 2016年12月23日
作    者   : liusongming
修改内容   : 新建函数
****************************************************************************/

void app_get_wan_rate(cJSON *recv_root,cJSON *send_root, void *info)
{
    cJSON *obj;
    CGI_LIB_INFO get_info;
    int interfacenum = 0,cur_uplink = 0,cur_downlink = 0,max_uplink = 0,max_downlink =0;
    char *statusUpSpeed = NULL,*statusDownSpeed = NULL;

    if(NULL == recv_root)
    {
        printf("file:%s func:%s line:%d\n",__FILE__,__func__,__LINE__);
        return ;
    }

    //调用库函数获取基本信息
    PIU8 modules[] =
    {
        MODULE_GET_STREAM_STATISTIC,
    };

    obj = cJSON_CreateObject();

    get_info.wp = NULL;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info, NULL);    //调用WAN口公共get函数获取信息

    //进行数据转换
    statusUpSpeed = cjson_get_value(obj,"statusUpSpeed","0");
    cur_uplink = atoi(statusUpSpeed) * 1000;
    statusDownSpeed = cjson_get_value(obj,"statusDownSpeed","0");
    cur_downlink = atoi(statusDownSpeed) * 1000;

    cJSON_AddNumberToObject(send_root,"interfacenum",1);
    cJSON_AddNumberToObject(send_root,"cur_uplink",cur_uplink);
    cJSON_AddNumberToObject(send_root,"cur_downlink",cur_downlink);
    cJSON_AddNumberToObject(send_root,"max_uplink",0);  //默认为0即可，app用不到这两个数据
    cJSON_AddNumberToObject(send_root,"max_downlink",0);

    cJSON_Delete(obj);

    return ;
}


/****************************************************************************
函数名  :app_get_wan_rate
描述    :用于WAN口检测，只在快速设置页面调用
参数    :
    send_root:用于函数回传数据，

    info:无
日    期   : 2016年12月23日
作    者   : liusongming
修改内容   : 新建函数
****************************************************************************/
void app_get_wan_detect(cJSON *recv_root,cJSON *send_root, void *info)
{
    cJSON *obj;
    char *wanDetection = NULL;
    CGI_LIB_INFO get_info;

    if(NULL == recv_root)
    {
        printf("file:%s func:%s line:%d\n",__FILE__,__func__,__LINE__);
        return ;
    }

    //调用lib库获取数据
    PIU8 modules[] =
    {
        MODULE_GET_WAN_DETECTION,
    };

    obj = cJSON_CreateObject();

    get_info.wp = NULL;
    get_info.root = obj;
    get_info.modules = modules;
    get_info.module_num = ARRAY_SIZE(modules);
    cgi_lib_get(get_info, NULL);

    //对数据进行解析
    wanDetection = cjson_get_value(obj,"wanDetection","dhcp");
    if(NULL == wanDetection)
    {
        printf("get wanDetection fail\n");
        cJSON_Delete(obj);
        return ;
    }
    if(!strcmp(wanDetection,"disabled"))    //未插网线
    {
        cJSON_AddNumberToObject(send_root,"detect_type",NO_LINE);
    }
    else
    {
        if(!strcmp(wanDetection,"static"))  //静态
        {
            cJSON_AddNumberToObject(send_root,"detect_type",DET_STATIC);

        }
        else if(!strcmp(wanDetection,"dhcp")) //动态
        {
            cJSON_AddNumberToObject(send_root,"detect_type",DET_DHCP);

        }
        else if(!strcmp(wanDetection,"pppoe"))  //pppoe
        {
            cJSON_AddNumberToObject(send_root,"detect_type",DET_PPPOE);

        }
        else if(!strcmp(wanDetection,"detecting"))  //检测中
        {
            cJSON_AddNumberToObject(send_root,"detect_type",DETECTING);

        }
    }


    cJSON_Delete(obj);


    return ;
}



