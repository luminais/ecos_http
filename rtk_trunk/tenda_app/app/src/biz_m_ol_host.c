#include "cgi_module_interface.h"

#include "biz_m_ol_host.h"


/*************************************************************************
  功能: 获取在线设备信息
  参数: ol_host_common_ack_t  填入设备信息的结构
  返回值: 0-成功，1-失败，80-超过长度限制
  是否需要用户实现: 是
  注意: 该函数模板中给出的参数全部都要使用
 ************************************************************************/
int biz_m_ol_host_info_get_cb(const api_cmd_t *cmd, 
								  ol_host_common_ack_t ** hosts_info,
								  void *privdata)
{
	int ret = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_OL_HOST_INFO));

	ret = app_get_module_param(cj_query,cj_get);

    cJSON *hostInfo_obj = NULL;
	cJSON *child_obj = NULL;
	int count_num = 0;
	int i = 0;

	hostInfo_obj = cJSON_GetObjectItem(cj_get,"hostInfo");

	if(hostInfo_obj)
	{
		count_num = cJSON_GetArraySize(hostInfo_obj);  /*离线设备和在线设备的总数*/
	}
	else
	{
		count_num = 0;
	}
	/*申请内存*/
	int len = sizeof(ol_host_common_ack_t) +(count_num)* sizeof(ol_hosts_dev_t);
	*hosts_info =(ol_host_common_ack_t *) malloc(len);
	if(NULL == (*hosts_info))
	{
		cJSON_Delete(cj_get);
		cJSON_Delete(cj_query);
		return 1; 
	}
	memset((char *)(*hosts_info), 0, len);
	ol_hosts_dev_t *hosts = (*hosts_info)->info.hosts;

	/*根据在线或离线列表填充数据*/
	for(i = 0; i < count_num; i++)
    {
        child_obj = cJSON_GetArrayItem(hostInfo_obj,i);
	
		strcpy(hosts[i].host_name, cjson_get_value(child_obj,"hostname","")); 
	    strcpy(hosts[i].host_alias, cjson_get_value(child_obj,"remark",""));
	    SET_OL_HOSTS_DEV_ALIAS(&hosts[i]); 	                 //设置host_alias标志位
	    strcpy(hosts[i].ip,cjson_get_value(child_obj,"iP",""));
	    strcpy(hosts[i].mac, cjson_get_value(child_obj,"mac",""));

		hosts[i].access_type = cjson_get_number(child_obj,"ConnectType",1);  /*/连接类型 1:无线*/
		SET_OL_HOSTS_DEV_TYPE(&hosts[i]);
		
        hosts[i].trust = cjson_get_number(child_obj,"is_mac_trust",0); /*1表示在信任列表 0不在信任列表*/
        SET_OL_HOSTS_DEV_TRUST(&hosts[i]); 	    //设置trust标志位          
        
		hosts[i].online_time = cjson_get_number(child_obj,"ConnetTime",0);
		SET_OL_HOSTS_ONLINE_TIME(&hosts[i]);        /*设置online_time标志位*/

		/*上传下载限制速率*/		
		hosts[i].up_limit = cjson_get_number(child_obj,"up_limit",-1);
		hosts[i].down_limit = cjson_get_number(child_obj,"down_limit",-1);
		hosts[i].bw_limited = cjson_get_number(child_obj,"bw_limited",0);
		SET_OL_HOSTS_UP_LIMIT(&hosts[i]);	        /* 设置up_limit标志位 */
	    SET_OL_HOSTS_DOWN_LIMIT(&hosts[i]);	        /* 设置down_limit标志位 */
        SET_OL_HOSTS_BW_LIMIT(&hosts[i]);	        /* 设置bw_limited标志位 */

		/*当前速率*/		
		hosts[i].curr_up_rate =  cjson_get_number(child_obj,"curr_up_rate",0.0);
		hosts[i].curr_down_rate =  cjson_get_number(child_obj,"curr_down_rate",0.0);

		/*是否在黑名单中*/
	    hosts[i].mac_blocked =  cjson_get_number(child_obj,"mac_blocked",0); 
	    SET_OL_HOSTS_MAC_BLOCK(&hosts[i]); 	    // 设置mac_blocked标志位 

	    hosts[i].host_type = cjson_get_number(child_obj,"hostTpye",0);  //设备类型
        SET_OL_HOSTS_DEV_TYPE(&hosts[i]);	        //设置host_type 标志位 

        //获取设备的厂商
        strcpy(hosts[i].manufactory_desc,cjson_get_value(child_obj,"manufactory",""));
        SET_OL_HOSTS_DEV_MANUFACTORY_DESC(&hosts[i]);

        hosts[i].online = cjson_get_number(child_obj,"online",0); /*/设备在线 默认值为0*/
        SET_OL_HOSTS_DEV_OL(&hosts[i]);		   //设置online标志位 

	}	
	
	(*hosts_info)->info.hosts_count = count_num;
	(*hosts_info)->info.mem_len = ((*hosts_info)->info.hosts_count) * sizeof(ol_hosts_dev_t);	

	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	
	return ret;
}

