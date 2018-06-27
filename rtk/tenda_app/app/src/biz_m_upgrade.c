#include "biz_m_upgrade.h"
#include "cgi_module_interface.h"

#include <trxhdr.h>


/*************************************************************************
  功能: 获取在线升级升级镜像的地址
  参数: download_path_t  填入信息的结构
  返回值: 0-成功，-1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_ucloud_info_download_path_get (
								  struct download_path_t * dpath)
{
	int ret = 0;
	int len = 0;
	char *path = NULL;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_UPGRADE_IMAGE_PATH));
	
	ret = app_get_module_param(cj_query,cj_get);

	len = cjson_get_number(cj_get,"uprade_image_path_len",0);  /*镜像地址长度*/
	path = cjson_get_value(cj_get,"uprade_image_path",""); /*镜像地址*/
    
	dpath->len = len;      /* 地址长度 */
	dpath->path = (char *)malloc((len+1) * sizeof(char));
	strncpy(dpath->path, path, len); 
	dpath->path[len] = '\0';

	/*释放内存*/
	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	
	return ret;
}

/*************************************************************************
  功能: 在线升级前检查内存是否充足
  参数: memory_state  填入信息的结构
  	img_size  镜像的大小
  返回值: 0-成功，-1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_ucloud_info_memory_check_cb (
						struct mem_state_t *memory_state,
						int img_size)
{
	int ret = 0;
	int enough_memory = 0;
	
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query 	= NULL;
	cJSON *extern_data = NULL;
	
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_UPGRADE_MEMORY_STATE));
    cJSON_AddItemToObject(cj_query, "extern_data", extern_data = cJSON_CreateObject());
	cJSON_AddNumberToObject(extern_data,"img_size",img_size);
	
	ret = app_get_module_param(cj_query,cj_get);

	enough_memory = cjson_get_number(cj_get,"memory_state",0);

	memory_state->enough_memory = enough_memory;

	cJSON_Delete(cj_get);
	cJSON_Delete(cj_query);
	
	return ret;
}

extern int do_upgrade_check(char *stream, int offset_in, int *flash_offset);
extern void uc_ol_upgrade_mem_free(char *head);

/*************************************************************************
  功能: 实现在线升级的回调函数
  参数: data_len  未使用 
  	 data      镜像数据结构地址
  返回值: 0-成功，-1-失败
  是否需要用户实现: 是
 ************************************************************************/
int biz_m_ucloud_begin_upgrade_cb (api_cmd_t *cmd, 
										unsigned int data_len, 
										void *data,
										void *privdata)
{	
	int offset = 0;   //头部偏移量 传过来的data已经去除头部了的
	int flash_offset = 0;

	//在线升级时，为了显示进度条 推迟升级锁调度器的时间
	cyg_thread_delay(100);
		
	tapf_watchdog_disable(); //关闭开门狗
	
	char *query = *(size_t *)data;
   
	if(do_upgrade_check(query,offset,&flash_offset)<0)
	{
		printf("[%s][%d]check error!\n",__func__,__LINE__);
		uc_ol_upgrade_mem_free(query);
		sys_reboot();
		return -1;
	}
	
#ifdef REALTEK
	/*realtek 升级跳过头,add*/
	offset += sizeof(struct trx_header);
#endif

	if(tenda_upload_fw(query,offset,flash_offset) < 0)
	{
		printf("[%s][%d]upload error!\n",__func__,__LINE__);
		sys_reboot();
		uc_ol_upgrade_mem_free(query);
		return -1;
	}
	
	uc_ol_upgrade_mem_free(query);   
	sys_reboot();

	return 0;
}

