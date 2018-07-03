#include "flash_cgi.h"
#include "cJSON.h"


#define NEED_MALLOC_MEM  136 *1024 //需要大致400KB内存用于升级

extern int net_mbuf_free_mem(void);
extern int get_kern_free_mem(void); /*获取内核剩余内存*/
extern int get_kern_mbuf_free_mem();
extern int get_kern_misc_free_mem();

extern char *cjson_get_value(cJSON *root, char *name, char *defaultGetValue);
extern int cjson_get_number(cJSON *root, char *name, int defaultGetValue);


/*****************************************************************************
 函 数 名  : app_get_upgrade_path
 功能描述  : 获取在线升级升级镜像的地址
 输入参数  : cJSON *recv_root	
			 cJSON *send_root 
			 void *info

 返 回 值  : 
 
 修改历史	   :
  1.日	  期   : 2016年12月13日
	作	  者   : luorilin
	修改内容   : 新生成函数
*****************************************************************************/
void  app_get_upgrade_path(cJSON *recv_root,cJSON *send_root,void *info)
{
	char path[] = "/tmp/fv9.bin"; /* 升级镜像的地址  */	
	int len = strlen(path);

	cJSON_AddNumberToObject(send_root,"uprade_image_path_len",len);
	cJSON_AddStringToObject(send_root,"uprade_image_path",path);
	
	return;
}

/*****************************************************************************
 函 数 名  : app_get_upgrade_memory_state
 功能描述  : 在线升级前检查内存是否充足
 输入参数  : cJSON *recv_root	
			 cJSON *send_root 
			 void *info
 输出参数  : 无
 返 回 值  : 
 修改历史	   :
  1.日	  期   : 2016年12月13日
	作	  者   : luorilin
	修改内容   : 新生成函数
*****************************************************************************/
void  app_get_upgrade_memory_state (cJSON *recv_root,cJSON *send_root,void *info)
{
	int free_malloc, free_clone_mem, mbuf_mem, misc_mem;
    	int enough_memory = 0;
	int image_size = 0;
	cJSON *extern_data = NULL;
	
	if(NULL == recv_root)
	{
		printf("[%s][%d]recv_root is null!\n",__func__,__LINE__);
		return;
	}
	extern_data = cJSON_GetObjectItem(recv_root,"extern_data");
	if(NULL == extern_data)
	{
		printf("[%s][%d]extern_data is null!\n",__func__,__LINE__);
		return;
	}
	image_size = cjson_get_number(extern_data,"img_size",0);
	
	free_malloc = net_mbuf_free_mem();  //镜像升级需要的额外的内存
	free_clone_mem = get_kern_free_mem(); //内核 内存剩余大小，用于存放镜像文件
	mbuf_mem = get_kern_mbuf_free_mem();
	misc_mem = get_kern_misc_free_mem(); //获取misc内存
		
       if((free_malloc >=NEED_MALLOC_MEM) && 
		(  ((free_malloc - NEED_MALLOC_MEM) + (free_clone_mem - 5*1024) + (mbuf_mem - 5*1024) + (misc_mem - 50*1024) )  >= image_size))
	{
		enough_memory = 1;
	}
	else 
	{
		enough_memory = 0;		
	}
	
	cJSON_AddNumberToObject(send_root,"memory_state",enough_memory);		
	return ;
}

