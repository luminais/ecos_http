/******************************************************************************
          版权所有 (C), 2015-2018, 深圳市吉祥腾达科技有限公司
 ******************************************************************************
  文 件 名   : cjson_nickname.c
  版 本 号   : 初稿
  作    者   : liusongming
  生成日期   : 2016年12月16日
  最近修改   :
  功能描述   :

  功能描述   : app获取，设置设备别名

  修改历史   :
  1.日    期   : 2016年12月16日
    作    者   : liusongming
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cjson_handle.h"
#include "cgi_lib_config.h"

/****************************************************************************
函数名  :app_get_dev_remark
描述    :用于app获取设备别名
参数    :
    recv_root:传入参数，包括模块名module，数据:extern_data
        格式:{"module":["APP_GET_DEV_REMARK"],"extern_data":{"mac_list":["44:37:E6:9E:7E:A2", 
            "90:67:1c:f8:d8:4a", "C8:3A:35:48:B2:02"],"cnt":3}}
        
    send_root:用于函数回传数据，
        格式:{"name_pair_s":[{"id":"44:37:E6:9E:7E:A2","nickname":"aaffhdwferhwer"}, 
             {"id":"90:67:1c:f8:d8:4a","nickname":"55681223"}]}
        
    info:无
日    期   : 2016年12月23日
作    者   : liusongming
修改内容   : 新建函数
****************************************************************************/

void app_get_dev_remark(cJSON *recv_root,cJSON *send_root, void *info)
{
  int count = 0,size = 0,i = 0;
  int number = 0;
  cJSON *extern_data = NULL;
  cJSON *mac_list = NULL;
  cJSON *mac_item = NULL;
  cJSON *name_pair_s = NULL;
  cJSON *obj = NULL;
  char *m_remark = NULL;

  if(NULL == recv_root)
  {
      printf("func:%s line:%d recv_root is null\n",__func__,__LINE__);
      return;
  }

  extern_data = cJSON_GetObjectItem(recv_root,"extern_data");
  if(NULL == extern_data)
  {
     printf("func:%s line:%d get extern_data fail\n",__func__,__LINE__);
     return ;
  }

  count = cjson_get_number(extern_data,"cnt",0);        //取出app传来的mac地址个数
  
  mac_list = cJSON_GetObjectItem(extern_data,"mac_list");//获取app传过来的mac地址列表
  if(NULL == mac_list)
  {
    printf("func:%s line:%d get mac_list fail\n",__func__,__LINE__);
    return;
  }

 

  cJSON_AddItemToObject(send_root,"name_pair_s",name_pair_s = cJSON_CreateArray()); //name_pair_s用于别名对
  for(i =0; i < count; i++)
  {
    mac_item = cJSON_GetArrayItem(mac_list,i);
    if(mac_item->valuestring != NULL)
    {
        m_remark = get_remark(mac_item->valuestring);
        if(NULL == m_remark)
        {
            break;
        }
        cJSON_AddItemToArray(name_pair_s,obj = cJSON_CreateObject());
        cJSON_AddStringToObject(obj,"id",mac_item->valuestring); //添加mac地址
        cJSON_AddStringToObject(obj,"nickname",m_remark);	//添加别名
        number++;
    }
  }

  cJSON_AddNumberToObject(send_root,"cnt",number);  //添加设备的数量

    
  return ;
}

/****************************************************************************
函数名  :app_set_dev_remark
描述    :用于app设置设备的别名
参数    :
    send_root:
        格式:{"name_pairs":[{"id":"44:37:e6:9e:7e:a2","nickname":"1111111111"}, 
             {"id":"90:67:1c:f8:d8:4a","nickname":"2222222222222222"}, {"id":"C8:3A:35:48:B2:02","nickname":"3333333333333333333"}],"cnt":3}
        
          msg:存放模块消息
     err_code:存放错误代码
        
         info:无
日    期   : 2016年12月24日
作    者   : liusongming
修改内容   : 新建函数
****************************************************************************/

void app_set_dev_remark(cJSON *send_root,CGI_MSG_MODULE *msg,int *result_code,void *info)
{
    cJSON *name_pairs = NULL,*obj = NULL;
    char *mac_string = NULL,*dev_remark = NULL;
    int count = 0,size = 0,i = 0;

    if(NULL == send_root)
    {
        printf("func:%s line:%d send_root is null\n",__func__,__LINE__);
        return;
    }

    name_pairs = cJSON_GetObjectItem(send_root,"name_pairs");  //获取app发过来的别名对
    if(NULL == name_pairs)
    {
        printf("func:%s line:%d get name_pairs fail\n",__func__,__LINE__);
        return ;
    }
    size = cJSON_GetArraySize(name_pairs);   
    count = cjson_get_number(send_root,"cnt",0);    //获取app发来的别名对数
   
    for(i = 0; i < size; i++)
    {
        obj = cJSON_GetArrayItem(name_pairs,i);
        if(NULL == obj)
        {
            printf("func:%s line:%d get Arrayitem fail\n",__func__,__LINE__);
            return ;
        }
        mac_string = cjson_get_value(obj,"id","");	//获取mac地址
        dev_remark = cjson_get_value(obj,"nickname",""); //获取别名
        if((NULL == mac_string) || (NULL == dev_remark))
        {
            printf("func:%s line:%d\n",__func__,__LINE__);
            return;
        }
        
        add_remark(mac_string, dev_remark);     //设置别名
      
    }
   
    return ;
}






