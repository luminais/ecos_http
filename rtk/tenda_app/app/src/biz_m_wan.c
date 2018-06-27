#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/endian.h>

#include "cgi_module_interface.h"
#include "biz_m_wan.h"
/*************************************************************************
  功能: 实现获取上网设置的函数
  参数: 需要填入数据到wan_basic_info_t结构
  返回值: 2-连接类型未知，0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_wan_basic_get_cb(const api_cmd_t *cmd, 
								 wan_basic_info_t *basic_info,
								 void *privdata)
{
    cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query = NULL;
    cJSON *wan = NULL;
    cJSON *wanitem = NULL;
    char *adsl_name = NULL,*adsl_pwd = NULL;
    int ret = 0,i = 0;
    int n_wan = 0,wan_count = 0;
    unsigned int ip_addr = 0,netmask = 0,gateway = 0,primary_dns = 0,backup_dns = 0,interfacenum = 0;
    char *wanIp = NULL,*wanMask = NULL,*wanGateway = NULL,*wanDns1 = NULL,*wanDns2 = NULL;
    char *wanConnectTime = NULL;
    int type = 0;
    int sta = 0,err = 0;
    int conn_time = 0;
    int wanConnectStatus = 0;
    
    cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
  
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_WAN_BASIC)); //获取GET_DEV_REMARK模块的数据
  	
	ret = app_get_module_param(cj_query,cj_get);

    //获取wan口个数
    n_wan = cjson_get_number(cj_get,"n_wan",0);
    basic_info->n_wan = n_wan;

    //获取用于存放各个wan信息的数组
    wan = cJSON_GetObjectItem(cj_get,"wan");
    if(NULL == wan)
    {
        printf("get wan error\n");
        cJSON_Delete(cj_query);
        cJSON_Delete(cj_get);
        return 1;
    }

    wan_count = cJSON_GetArraySize(wan);
    if(n_wan != wan_count)      //wan口个数不匹配
    {
        printf("wan number not match\n");
        cJSON_Delete(cj_query);
        cJSON_Delete(cj_get);
        return 1;
    }
    basic_info->n_wan = wan_count;      //获取wan口的总个数
    //获取各个wan口的详细信息

    for(i = 0; i < wan_count; i++)
    {
        wanitem = cJSON_GetArrayItem(wan,i);
        if(NULL == wanitem)
        {
            printf("get wan detail error\n");
            cJSON_Delete(cj_query);
            cJSON_Delete(cj_get);
            return 1;
        }
        //interfacenum表示第几个wan口
        interfacenum = cjson_get_number(wanitem,"interfacenum",0);    
        basic_info->wan[i].interfacenum= interfacenum;
        
        //获取联网类型
        type = cjson_get_number(wanitem,"type",0);      //默认返回未知类型，后台已经用0代表未知类型，app维护人员需注意
        if(0 == type)
        {
            printf("wan_type:unknow\n");
            cJSON_Delete(cj_query);
            cJSON_Delete(cj_get);
		    return 2;   //连接类型未知 
        }
        basic_info->wan[i].type = type;

        //获取账号密码
        adsl_name = cjson_get_value(wanitem,"adsl_name","");
        adsl_pwd = cjson_get_value(wanitem,"adsl_pwd","");

        if((NULL != adsl_name) || (NULL != adsl_pwd))
        {
            strcpy(basic_info->wan[i].adsl_info.adsl_name,adsl_name); // 必填，账号 
	        strcpy(basic_info->wan[i].adsl_info.adsl_pwd,adsl_pwd);    //必填，密码
	        SET_WAN_BASIC_WAN_ADSL_INFO(&basic_info->wan[i]);    //设置标志位
        }
        
        //获取wan口基本信息
        wanIp = cjson_get_value(wanitem,"ip_addr","0.0.0.0");
        wanMask = cjson_get_value(wanitem,"netmask","0.0.0.0");
        wanGateway = cjson_get_value(wanitem,"gateway","0.0.0.0");
        wanDns1 = cjson_get_value(wanitem,"primary_dns","");
        wanDns2 = cjson_get_value(wanitem,"backup_dns","");
        wanConnectTime = cjson_get_value(wanitem,"conn_time","0");

        inet_aton(wanIp,&ip_addr);
        inet_aton(wanMask,&netmask);
        inet_aton(wanGateway,&gateway);
        inet_aton(wanDns1,&primary_dns);
        inet_aton(wanDns2,&backup_dns);
        inet_aton(wanConnectTime,&conn_time);
        conn_time = atoi(wanConnectTime);
        
        basic_info->wan[i].netaddr_info.ip_addr  = ntohl(ip_addr); /*必填， 32位无符整形ip地址 "114.114.114.114" */
  		basic_info->wan[i].netaddr_info.netmask = ntohl(netmask); /*必填， 32位无符整形子网掩码 "255.255.255.0" */
  		basic_info->wan[i].netaddr_info.gateway = ntohl(gateway); /* 必填，32位无符整形网关 "192.168.10.1" */
  		basic_info->wan[i].netaddr_info.primary_dns = ntohl(primary_dns); /*必填， 首选dns1 */
  		basic_info->wan[i].netaddr_info.backup_dns= ntohl(backup_dns); /* 必填，备份dns1 */
  		basic_info->wan[i].netaddr_info.conn_time = conn_time; /* 设置联网时间标志位*/
        SET_WAN_CONN_TIME(&basic_info->wan[i].netaddr_info); /* 设置标志位 */
		SET_WAN_BASIC_WAN_NET_ADDR_INFO(&basic_info->wan[i]); /* 设置标志位 */


        //获取网络监测状态
        sta = cjson_get_number(wanitem,"sta",0);    //0-未插网线，1-未联网，2-联网中，3-已联网 
        basic_info->wan[i].wan_status.sta = sta;
        SET_WAN_BASIC_WAN_STA(&basic_info->wan[i]); /* 设置标志位 */
        
        //只有未联网的情况下才能够去读取错误码，否则会出现异常,app维护人员需注意
        if(1 == sta)
        {
            err = cjson_get_number(wanitem,"err",0);    //0-正常， 566-无法连接到互联  网，549-用户名或密码错误，548-远端服务器无响应
            basic_info->wan[i].wan_status.err = err;
        }
        
    }

    cJSON_Delete(cj_query);
    cJSON_Delete(cj_get);

	return 0;
}

/*************************************************************************
  功能: 判断并完成设置向导
  参数:   无
  返回值: 无
  是否需要用户实现: 否
 ************************************************************************/
static void end_the_wizard(void)
{
	printf("end the wizard successfully !\n");
	
	cJSON *cj_set 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query = NULL;

	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_WIZARD_SUCCEED));
	app_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_set);
	cJSON_Delete(cj_query);
}

/*************************************************************************
  功能: 设置上网类型后使配置生效的函数
  参数: 不使用
  返回值: 无
  是否需要用户实现: 是
 ************************************************************************/
#ifdef CONFIG_APP_COSTDOWN
void biz_m_wan_basic_set_process(void)
#else
static void biz_m_wan_basic_set_process(
											const api_cmd_t *cmd,
											int data_len,
											int ret)
#endif
{
	int biz_ret = 100;
	cJSON *cj_set	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_WAN_BASIC_PROCESS));
	//调用公共set函数
	app_set_module_param(cj_query,cj_set);
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);

    //完成设置向导
    end_the_wizard();
}


/*************************************************************************
  功能: 实现设置上网类型的函数
  参数: 将wan_basic_info_t中数据保存到路由器
  返回值: 2-连接类型未知，10-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_wan_basic_set_cb(const api_cmd_t *cmd, 
								 	 wan_basic_info_t *basic_info,
								 	 void *privdata)
{
    int ret = 0,i = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query = NULL;
    cJSON *cj_set = NULL;
    cJSON *wan = NULL;
    cJSON *wanitem = NULL;
    char wan_ip[16] = {0}, wan_gateway[16] = {0};
    char wan_msk[16] = {0},dns1[16] = {0}, dns2[16] = {0};
    struct in_addr ina;

    if(NULL == basic_info)
    {
        printf("func:%s line:%d basic_info is NULL\n",__func__,__LINE__);
        return 1;
    }

    cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(SET_WAN_BASIC));
    
    cJSON_AddNumberToObject(cj_set,"n_wan",basic_info->n_wan);  //添加wan口的个数

    cJSON_AddItemToObject(cj_set,"wan",wan = cJSON_CreateArray()); //添加用于存放wan口详细数据的数组

    //进行数据的转换以及组装，IP地址如果是以整形传送到cjson层解析会导致截断，所以对于IP地址必须在
    //biz层进行转换成字符串的形式
     if(basic_info->wan[0].type == STATIC) 
     {
        if (HAS_WAN_BASIC_WAN_NET_ADDR_INFO(&basic_info->wan[0]))
        {
            cJSON_AddItemToArray(wan,wanitem = cJSON_CreateObject());
            if(NULL == wanitem)
            {
                cJSON_Delete(cj_query);
                cJSON_Delete(cj_get);
                return 1;
            }
           	//将IP转换成字符串的形式
            ina.s_addr = ntohl(basic_info->wan[0].netaddr_info.ip_addr);
            sprintf(wan_ip, "%s", inet_ntoa(ina));

			//转换子网掩码
            ina.s_addr = ntohl(basic_info->wan[0].netaddr_info.netmask);
            sprintf(wan_msk, "%s", inet_ntoa(ina));

			//转换网关
            ina.s_addr = ntohl(basic_info->wan[0].netaddr_info.gateway);
            sprintf(wan_gateway, "%s", inet_ntoa(ina));

			//转换DNS
            ina.s_addr = ntohl(basic_info->wan[0].netaddr_info.primary_dns);
            sprintf(dns1, "%s", inet_ntoa(ina));

            ina.s_addr = ntohl(basic_info->wan[0].netaddr_info.backup_dns);
            sprintf(dns2, "%s", inet_ntoa(ina));

            //将app的数据添加到cjson对象
            cJSON_AddNumberToObject(wanitem, "type", basic_info->wan[0].type);
            cJSON_AddStringToObject(wanitem,"ip_addr",wan_ip);
            cJSON_AddStringToObject(wanitem,"netmask",wan_msk);
            cJSON_AddStringToObject(wanitem,"gateway",wan_gateway);
            cJSON_AddStringToObject(wanitem,"primary_dns",dns1);
            cJSON_AddStringToObject(wanitem,"backup_dns",dns2);
        }
     }
     else if(basic_info->wan[0].type == DYNAMIC)	//动态接入
     {
        cJSON_AddItemToArray(wan,wanitem = cJSON_CreateObject());
        if(NULL == wanitem)
        {
            cJSON_Delete(cj_query);
            cJSON_Delete(cj_get);
            return 1;
        }
        cJSON_AddNumberToObject(wanitem, "type", basic_info->wan[0].type);
     }
     else if(basic_info->wan[0].type == ADSL)	//pppoe接入
     {
        if (HAS_WAN_BASIC_WAN_ADSL_INFO(&basic_info->wan[0]))
        {
            cJSON_AddItemToArray(wan,wanitem = cJSON_CreateObject());
            if(NULL == wanitem)
            {
                cJSON_Delete(cj_query);
                cJSON_Delete(cj_get);
                return 1;
            }
			//将账户名和密码添加到对象中
            cJSON_AddNumberToObject(wanitem, "type", basic_info->wan[0].type);
            cJSON_AddStringToObject(wanitem, "adsl_name", basic_info->wan[0].adsl_info.adsl_name);
            cJSON_AddStringToObject(wanitem, "adsl_pwd", basic_info->wan[0].adsl_info.adsl_pwd);
        }
     }
     else	//未知类型
     {
        cJSON_Delete(cj_query);
        cJSON_Delete(cj_get);
        return 2;
     }
    
   ret = app_set_module_param(cj_query,cj_set);

   //底层执行出错
   if(ret != 0)
   {
       cJSON_Delete(cj_query);
       cJSON_Delete(cj_get);
       return 1; 
   }
   #ifndef CONFIG_APP_COSTDOWN
	//2.使配置生效，该代码保留，无需修改
	wan_common_ack_t wan_ack_t = {0};
	wan_ack_t.err_code = 0;
	uc_api_lib_cmd_resp_notify(cmd,0, sizeof(wan_common_ack_t), &wan_ack_t, biz_m_wan_basic_set_process);
	#endif

    cJSON_Delete(cj_query);
    cJSON_Delete(cj_get);
  #ifdef CONFIG_APP_COSTDOWN
  	return 0;
  #else  
	return COMPLETE_RET; /* 10 */
  #endif
}

/*************************************************************************
  功能: 获取WAN口总流量
  参数: wan_rate_info_t 需要填入数据的结构
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_wan_rate_get_cb(const api_cmd_t *cmd,
 									wan_rate_info_t *rate_info,
 									void *privdata)
{
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query = NULL;
    int interfacenum = 0,cur_uplink = 0,cur_downlink = 0,max_uplink = 0,max_downlink =0;


    cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
    
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_WAN_RATE)); //获取GET_DEV_REMARK模块的数据

    //调用公共get函数
	app_get_module_param(cj_query,cj_get);
 	//取出数据
    interfacenum = cjson_get_number(cj_get,"interfacenum",0);
    cur_uplink = cjson_get_number(cj_get,"cur_uplink",0);
    cur_downlink = cjson_get_number(cj_get,"cur_downlink",0);
    max_uplink = cjson_get_number(cj_get,"max_uplink",0);
    max_downlink = cjson_get_number(cj_get,"max_downlink",0);

    rate_info->n_wan = 1;                   //wan口的数量
    rate_info->wan[0].cur_uplink = cur_uplink;   /* WAN口当前上行速率 byte/s */
	rate_info->wan[0].cur_downlink = cur_downlink; /* WAN口当前下行速率 byte/s */
	rate_info->wan[0].interfacenum = interfacenum;  /* 第一个WAN口 */
	rate_info->wan[0].max_uplink = max_uplink;    /* 设置0即可 */
	rate_info->wan[0].max_downlink = max_downlink;  /* 设置0即可 */

    cJSON_Delete(cj_query);
    cJSON_Delete(cj_get);

	return 0;
}

/*************************************************************************
  功能: 进入设置向导的时候，获取wan口连接状态
  参数: type_info 需要填入数据的结构
  返回值: 0-成功，1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_wan_detect_type_cb (
						const api_cmd_t *cmd, 
						wan_detecttype_info_t *type_info,
						void *privdata)
{
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query = NULL;
    int detect_type = 0;

    cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
    
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_WAN_DETECT)); 

    //调用公共函数获取数据
	app_get_module_param(cj_query,cj_get);
    
 	detect_type = cjson_get_number(cj_get,"detect_type",0);
    type_info->wan[0].detect_type = detect_type;
    type_info->n_wan = 1;	//WAN口个数
    
    cJSON_Delete(cj_query);
    cJSON_Delete(cj_get);
 	return 0;
}


