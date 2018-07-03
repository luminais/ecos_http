/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cgi_lib_system_status.c
  版 本 号   : 初稿
  作    者   : liquan
  生成日期   : 2016年12月12日
  最近修改   :
  功能描述   :

  功能描述   : system_info的最小功能单元的get和set库

  修改历史   :
  1.日    期   : 2016年12月12日
    作    者   : luorilin
    修改内容   : 创建文件

******************************************************************************/
#include "cgi_lib.h"
#include "wan.h"
#include "wifi.h"
#include <router_net.h>
#include <netinet/if_ether.h>
#include "dhcp_server.h"
#include "systools.h"



#ifdef __CONFIG_INDICATE_LED__
typedef enum
{
    INDICATE_LED_OFF = 0,
    INDICATE_LED_EXIT
} LED_OFF_TYPE;
#endif
extern int dns_redirect_disable;//定于与 ./userSpace/cbb/src/dnsmasq/src/dnsmasq_proto.c


/*****************************************************************************
 函 数 名  : cgi_lib_get_system_status_info
 功能描述  : 获取system_status的wan信息
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
RET_INFO cgi_lib_get_system_status_info(webs_t wp, cJSON *root, void *info)
{
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
    cJSON_AddStringToObject(root,LIB_STATUS_WAN_IP,value);

    //wanMask
    memset(value, 0, sizeof(value));
    snprintf(value, sizeof(value), "%s", wan_common_info->wan_cur_mask);
    if (strncmp(value, "0.0.0.0", strlen("0.0.0.0")) == 0)
    {
        strcpy(value,"");
    }
    cJSON_AddStringToObject(root,LIB_STATUS_WAN_MASK,value);

    //wan gateway
    memset(value, 0, sizeof(value));
    snprintf(value, sizeof(value), "%s", wan_common_info->wan_cur_gw);
    if (strncmp(value, "0.0.0.0", strlen("0.0.0.0")) == 0)
    {
        strcpy(value,"");
    }
    cJSON_AddStringToObject(root,LIB_STATUS_WAN_GATWAY,value);

    //wanDns1
    cJSON_AddStringToObject(root,LIB_STATUS_DNS1,wan_common_info->wan_cur_dns1);

    //wanDns2
    cJSON_AddStringToObject(root,LIB_STATUS_DNS2,wan_common_info->wan_cur_dns2);

    return RET_SUC;
}
/*****************************************************************************
 函 蔵 名  : cgi_lib_get_system_status_wanMac
 功能描述  : 获取system_status的wanMac
 输入参蔵  : webs_t wp
             cJSON *root
             void *info
 输出参蔵  : 蜸
 返 回 值  :

 GS改历史      :
  1.日    期   : 2016年12月13日
    作    者   : luorilin
    GS改内菼   : G律珊i

*****************************************************************************/
RET_INFO cgi_lib_get_system_status_wanMac(webs_t wp, cJSON *root, void *info)
{
    P_WAN_HWADDR_INFO_STRUCT wan_hwaddr_info = gpi_wan_get_hwaddr_info();

    /*wan mac*/
    cJSON_AddStringToObject(root, LIB_STATUS_WAN_MAC, wan_hwaddr_info->wan_hwaddr);
    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_software_version
 功能描述  : 获取software_version
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
RET_INFO cgi_lib_get_software_version(webs_t wp, cJSON *root, void *info)
{
    char iterm_value[PI_BUFLEN_64];

    /*software version*/
    memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
    snprintf(iterm_value, PI_BUFLEN_64, T("%s_%s"), W311R_ECOS_SV, NORMAL_WEB_VERSION);
    cJSON_AddStringToObject(root, LIB_SOFTWARE_VERSION, iterm_value);
    return RET_SUC;
}
/*****************************************************************************
 函 数 名  : cgi_lib_get_wan_connectTime
 功能描述  : 获取wan_connectTime
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
RET_INFO cgi_lib_get_wan_connectTime(webs_t wp, cJSON *root, void *info)
{
    char iterm_value[PI_BUFLEN_64];

    P_WAN_INFO_STRUCT wan_common_info = NULL;
    wan_common_info = gpi_wan_get_info();

    /*run time*/
    memset(iterm_value, '\0', PI_BUFLEN_64 * sizeof(char));
    snprintf(iterm_value, PI_BUFLEN_64, "%llu", wan_common_info->connect_time);
    cJSON_AddStringToObject(root, LIB_WAN_CONNECT_TIME, iterm_value);

    return RET_SUC;
}

/*以下是在线升级模块，需要调用cgi_api.h接口，之后编译app后才能使用相应的接口，所以这里加宏控制*/
#ifdef __CONFIG_TENDA_APP__

/*在线升级中，服务器返回的错误码*/
typedef enum
{
    UP_TYPE_MIN = 0,
    UP_TYPE_UPGRADE_Q = UP_TYPE_MIN,    //正在询问升级服务器
    UP_TYPE_MEM_NOT_ENOUGH,             //内存不足,无法升级
    UP_TYPE_WAIT_SVR,                   //路由器在排队升级
    UP_TYPE_FW_DOWNING,                 //路由器正在下载固件
    UP_TYPE_WRITING_TO_FLASH,           //路由器正在烧写固件
    UP_TYPE_MAX
} ENUM_UP_TYPE;

#define ERR_UCLOUD_BUSY 19  /*服务器正繁忙*/

static unsigned int fw_dowing_flage = 0; /*是否进入下载状态的标志*/

/*****************************************************************************
 函 数 名  : strCut_from_mid
 功能描述  : 从字符串的中间截取n个字符
 输入参数  : char *dst,char *src, int n,int m
             n:要截取n个字符
             m:从第m个字符开始截取
 输出参数  : 无
 返 回 值  : char *dst

 修改历史      :
  1.日    期   : 2017年2月10日
    作    者   : luorilin
    修改内容   : 新生成函数

*****************************************************************************/
static char *strCut_from_mid(char *dst,const char *src, int n,int m) /*n为长度，m为位置*/
{
    char *p = src;
    char *q = dst;
    int len = 0;
    if(NULL == src)
    {
        printf("[%s][%d] str is null!!\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    len = strlen(src);

    if(n > len || n < 0 || m > len || m < 0)
    {
        printf("[%s][%d] input n/m error!!\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    p += m;
    while(n--)
        *(q++) = *(p++);
    *(q++)='\0';

    return dst;
}
/*****************************************************************************
 函 数 名  : is_wan_status_connect_ok
 功能描述  : 判断是否已联网
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 已联网:0  未联网:1

 修改历史      :
  1.日    期   : 2017年2月10日
    作    者   : luorilin
    修改内容   : 新生成函数

*****************************************************************************/
int is_wan_status_connect_ok()
{
    char wan_connstatus[9] = {0};
    char temp[2] = {0};
    int wlmode = 0;   //wan口接入模式
    int ret = -1;
    /*获取联网状态码*/
    sprintf(wan_connstatus,"%d",tpi_wan_get_err_result_info());

    /*；
    联网状态码:例:13102060 (第七位用来判断是否已联网;6:已联网; 4:wan口已获取到ip)
    */
    strCut_from_mid(temp,wan_connstatus,1,6);  /*从状态码中截取出第七位*/

    wlmode = gpi_wifi_get_mode();   /*获取WAN口接入模式*/

    /*WL_APCLIENT_MODE 和 WL_BRIDGEAP_MODE 模式下,现有的规格规定:只要DUT wan口获取到ip,就认为已联网*/
    /*如果后期规格修改，可以定位到tpi_wan_get_connected_err_info查看,这里再做修改*/
    if(WL_APCLIENT_MODE == wlmode || WL_BRIDGEAP_MODE == wlmode)
    {
        ret = strncmp(temp,"4",1);
    }
    else
    {
        ret = strncmp(temp,"6",1);
    }

    return ret;
}

/*这个静态全局变量 用于忽略新版本时，对比用*/
static char s_cur_version[PI_BUFLEN_64] = {0};

/*成研提供的接口，通过这个公共接口，获取新软件版本以及在线升级和升级中的信息*/
extern int uc_cgi_get_module_param(cJSON *cj_query, cJSON *cj_get);


/*****************************************************************************
 函 数 名  : cgi_lib_get_ucloud_version
 功能描述  : cgi获取新版本
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
RET_INFO cgi_lib_get_ucloud_version(webs_t wp, cJSON *root, void *info)
{
    char new_version[PI_BUFLEN_64] = {0};
    char cur_version[PI_BUFLEN_64] = {0};
    char *temp = NULL;
    char *description_save = NULL;
	int len = 0;

    cJSON *cj_get   = NULL;
    cJSON *module   = NULL;
    cJSON *cj_query = NULL;

	cj_query = cJSON_CreateObject();
    cj_get = cJSON_CreateObject();

	if(NULL == cj_query)
	{
		printf("[%s][%d] create cj_query err!\n",__func__,__LINE__);
		return RET_ERR; 
	}
	if(NULL == cj_get)
	{
		cJSON_Delete(cj_query);
		printf("[%s][%d] create cj_get err!\n",__func__,__LINE__);
		return RET_ERR;
	}
    //判断联网状态
    if ( 0 != is_wan_status_connect_ok())
    {
        /*未联网情况下*/
        cJSON_AddStringToObject(root, LIB_HANSNEWSOFTVERSION,"false");
    }
    else/*联网正常情况下开始查询版本号信息*/
    {
        cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
        cJSON_AddItemToArray(module,cJSON_CreateString("uc_get_version"));
        uc_cgi_get_module_param(cj_query,cj_get);
        /*新软件版本号*/
        strcpy(new_version,cgi_lib_get_var(NULL,cj_get,"newest_ver",""));
    }
    //无新版本
    if(0 == strcmp(new_version,""))
    {
        cJSON_AddStringToObject(root, LIB_HANSNEWSOFTVERSION,"false");
    }
    else
    {
        memset(cur_version,0x0,sizeof(cur_version));
        strcpy(cur_version,nvram_safe_get("noprompt_version"));
        /*flash没有保存保本号前*/
        if(0 == strcmp(cur_version,""))
        {
            sprintf(cur_version,"%s_%s", W311R_ECOS_SV, NORMAL_WEB_VERSION);
        }
        //与保存在nvram中的参考版本对比
        if(0 != strcmp(cur_version,"") && 0 != strcmp(new_version,cur_version))
        {
            strcpy(s_cur_version,new_version); /*保存当前最新版本号/用于忽略版本时，对比用*/

            cJSON_AddStringToObject(root, LIB_HANSNEWSOFTVERSION,"true");

            cJSON_AddStringToObject(root, LIB_UPDATEVERSION,new_version);  /*软件版本号*/

            /*软件新版本信息:新增及修复的功能列表*/
            cJSON *description = NULL;

            description = cJSON_GetObjectItem(cj_get,"description"); /*中文简体描述*/

            temp = cJSON_Print(description);
			len = strlen(temp)*sizeof(char) + 1;
            description_save = malloc(len);
            memset(description_save,0x0,len);
            strcpy(description_save,temp);

            cJSON_AddItemToObject(root,LIB_NEWVERISONOPTIMIZE,cJSON_Parse(description_save));

            /*软件更新时间*/
            cJSON_AddStringToObject(root,LIB_UPDATEDATE,cgi_lib_get_var(NULL,cj_get,"update_date",""));

			free(temp);
			free(description_save);
        }
        /*点击了暂不升级后,会把当前最新版本保存在flash中*/
        else if(0 != strcmp(cur_version,"") && 0 == strcmp(new_version,cur_version))
        {
            cJSON_AddStringToObject(root, LIB_HANSNEWSOFTVERSION,"false");
        }
    }
    /*释放内存*/
    cJSON_Delete(cj_query);
    cJSON_Delete(cj_get);

    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_lib_get_ucloud_upgrade
 功能描述  : 立即升级
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
RET_INFO cgi_lib_get_ucloud_upgrade(webs_t wp, cJSON *root, void *info)
{
    cJSON *cj_get   = NULL;
    cJSON *module   = NULL;
    cJSON *cj_query = NULL;

    int fw_size = 0;    //升级文件总大小
    int fw_recved = 0;     //当前下载量
    int fw_down_sec_left = 0;  //下载剩余时间

    int error_code = -1;
    char *upgrade_btn = NULL;   //立即升级按钮;upgrade=iimmediately

    upgrade_btn =  cgi_lib_get_var(wp,root, T("upgrade"), T(""));  //点击立即升级按钮

    /*是否点击立即升级按钮*/
    if(NULL != strstr(upgrade_btn, "immediately"))
    {
        cj_query = cJSON_CreateObject();
        cj_get = cJSON_CreateObject();

		if(NULL == cj_query)
		{
			printf("[%s][%d] create cj_query err!\n",__func__,__LINE__);
			return RET_ERR; 
		}
		if(NULL == cj_get)
		{
			cJSON_Delete(cj_query);
			printf("[%s][%d] create cj_get err!\n",__func__,__LINE__);
			return RET_ERR;
		}
		
        cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
        cJSON_AddItemToArray(module,cJSON_CreateString("uc_get_download_progress"));

        uc_cgi_get_module_param(cj_query,cj_get);  /*升级，并获取升级中的信息*/

        /*调用成研提供的接口，实现在线升级并获取升级中的信息*/
        error_code = cjson_get_number(cj_get,"error_code",0); /*获取升级中的各种状态*/

        /*进入到了下载镜像状态,如果收到 UP_TYPE_UPGRADE_Q 或 ERR_UCLOUD_BUSY,不做处理,
         还停留在下载页面, 一直到下载失败或下载成功才推出下载状态页面*/
        if(((UP_TYPE_UPGRADE_Q == error_code) || (ERR_UCLOUD_BUSY == error_code)) && (1 == fw_dowing_flage))
        {
            error_code = UP_TYPE_FW_DOWNING;
        }

        switch(error_code)
        {
            case UP_TYPE_UPGRADE_Q:/*正在询问升级服务器*/
            case UP_TYPE_WAIT_SVR:/*正在排队中*/
            case ERR_UCLOUD_BUSY: /*服务器正忙*/
                //fw_dowing_flage = 0;
                cJSON_AddStringToObject(root,LIB_UPGRADEREADY,"wait");
                cJSON_AddStringToObject(root,LIB_WRITING_TO_FLASH,"false");
                /*错误码*/
                cJSON_AddNumberToObject(root, LIB_UPGRADEERORCODE,0);
                /*总下载量*/
                cJSON_AddNumberToObject(root, LIB_TOTALSIZE,0);
                /*已经下载量*/
                cJSON_AddNumberToObject(root, LIB_DOWNLOADSIZE,0);
                /*剩余下载时间*/
                cJSON_AddNumberToObject(root, LIB_RESTTIME,0);
                break;
            case UP_TYPE_FW_DOWNING:/*正在下载固件*/
                /*获取升级镜像文件总大小，当前下载量，以及下载剩余时间*/
                fw_size = cjson_get_number(cj_get,"fw_size",0);
                fw_recved = cjson_get_number(cj_get,"recved",0);
                fw_down_sec_left = cjson_get_number(cj_get,"sec_left",0);

                if(0 == fw_size || 0 == fw_recved)
                {
                    fw_dowing_flage = 0;
                    cJSON_AddStringToObject(root, LIB_UPGRADEREADY,"wait"); /*获取到升级文件大小为0时，让页面继续处于等待下载状态*/
                }
                else
                {
                    /*如果还没下载完,下载剩余时间就为0时,设置剩余时间为1s*/
                    if((fw_size > fw_recved) && (0 == fw_down_sec_left))
                        fw_down_sec_left = 1;

                    fw_dowing_flage = 1;
                    cJSON_AddStringToObject(root, LIB_UPGRADEREADY,"true");
                }
                cJSON_AddStringToObject(root,LIB_WRITING_TO_FLASH,"false");
                /*错误码*/
                cJSON_AddNumberToObject(root, LIB_UPGRADEERORCODE,0);
                /*总下载量*/
                cJSON_AddNumberToObject(root, LIB_TOTALSIZE,fw_size);
                /*已经下载量*/
                cJSON_AddNumberToObject(root, LIB_DOWNLOADSIZE,fw_recved);
                /*剩余下载时间*/
                cJSON_AddNumberToObject(root, LIB_RESTTIME,fw_down_sec_left);
                break;
            case UP_TYPE_WRITING_TO_FLASH: /*正在烧录固件*/
                fw_dowing_flage = 0;
                cJSON_AddStringToObject(root, LIB_UPGRADEREADY,"true");
                cJSON_AddStringToObject(root,LIB_WRITING_TO_FLASH,"true");/*正在烧录*/
                /*错误码*/
                cJSON_AddNumberToObject(root, LIB_UPGRADEERORCODE,0);
                /*总下载量*/
                cJSON_AddNumberToObject(root, LIB_TOTALSIZE,1040384);
                /*已经下载量*/
                cJSON_AddNumberToObject(root, LIB_DOWNLOADSIZE,1040384);
                /*剩余下载时间*/
                cJSON_AddNumberToObject(root, LIB_RESTTIME,0);
                break;
            default:
                fw_dowing_flage = 0;
                cJSON_AddStringToObject(root,LIB_UPGRADEREADY,"fail");
                cJSON_AddStringToObject(root,LIB_WRITING_TO_FLASH,"false");
                /*错误码*/
                cJSON_AddNumberToObject(root, LIB_UPGRADEERORCODE,error_code);  /*其它错误码返回*/
                /*总下载量*/
                cJSON_AddNumberToObject(root, LIB_TOTALSIZE,0);
                /*已经下载量*/
                cJSON_AddNumberToObject(root, LIB_DOWNLOADSIZE,0);
                /*剩余下载时间*/
                cJSON_AddNumberToObject(root, LIB_RESTTIME,0);
                break;
        }

        /*释放内存*/
        cJSON_Delete(cj_query);
        cJSON_Delete(cj_get);

    }

    return RET_SUC;
}

/*****************************************************************************
 函 数 名  : cgi_notNow_upgrade_set
 功能描述  : 暂不升级
 输入参数  : struct upgrade_message *mess
             char *data_save
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年12月13日
    作    者   : luorilin
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_notNow_upgrade_set(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg, char *err_code,void *info)
{
    char    *no_prompt = NULL;  //参考版本;

    no_prompt =  cgi_lib_get_var(wp,root, T(LIB_NOPROMPT), T("off"));//暂不升级按钮

    if(0 == strcmp(no_prompt, "on"))
    {
        nvram_set("noprompt_version", s_cur_version);   //把最新版本保存到nvram中 下次做参考
    }

    sprintf(err_code, "%s", "0");

    return RET_SUC;
}
#endif //__CONFIG_TENDA_APP__


/*****************************************************************************
 函 数 名  : cgi_lib_set_wizard_succeed
 功能描述  : app完成快速设置
 输入参数  :
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2017年1月5日
    作    者   : liusongming
    修改内容   : 新生成函数

*****************************************************************************/
RET_INFO cgi_lib_set_wizard_succeed(webs_t wp,cJSON *root,CGI_MSG_MODULE *msg,char *err_code,void *info)
{
    //add by z10312 dns重定向只在快速设置前生效
    //dns_redirect_disable = 1;
    _SET_VALUE("mode_need_switch", "no");//标志着联网类型检测结束或提前结束
    _SET_VALUE(SYSCONFIG_QUICK_SET_ENABLE, "0"); //标志着快速设置结束
#ifdef __CONFIG_PPPOE_SERVER__
    set_synchro_type(MANUAL_SYNCHRO);
#endif
#ifdef __CONFIG_INDICATE_LED__
    set_indicate_led_off(INDICATE_LED_EXIT);
#endif

    return RET_SUC;
}


