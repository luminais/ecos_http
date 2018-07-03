#include "cgi_module_interface.h"
#include "biz_m_dev_nickname.h"

/*************************************************************************
  功能: 获取设备别名(获取备注)
  参数: info 返回数据的结构
  	param 指定要获取那些设备的别名
  返回值: 0成功，1失败
  是否需要用户实现: 是
  
  cj_query的形式为:{"module":["APP_GET_DEV_REMARK"],"extern_data":{"mac_list":
                   ["44:37:E6:9E:7E:A2", "90:67:1c:f8:d8:4a", "C8:3A:35:48:B2:02"],"cnt":3}}
                
  cj_get的形式为:{"name_pair_s":[{"id":"44:37:E6:9E:7E:A2","nickname":"aaffhdwferhwer"}, 
                    {"id":"90:67:1c:f8:d8:4a","nickname":"55681223"}],"cnt":2}
 ************************************************************************/
int biz_m_dev_nickname_info_get(const api_cmd_t *cmd,
 										 dev_name_t *param,
										 nickname_info_t *info,
										 void *privdata)
{
 	int ret = 0,i = 0,cnt = 0;
    char *id = NULL,*nickname = NULL;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query = NULL;
    cJSON *extern_data = NULL;
    cJSON *name_pair_s = NULL;
    cJSON *mac_list = NULL;
    cJSON *pairitem = NULL;

    if ((NULL == param) || (NULL == info)) 
	{
        printf("param NULL or privdata NULL\n");
 		return 1;
	}
    
	cj_query = cJSON_CreateObject();
	cj_get = cJSON_CreateObject();
    
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(GET_DEV_REMARK)); //获取GET_DEV_REMARK模块的数据
    //需要添加获取另外其它模块的数据如下添加
    //cJSON_AddItemToArray(module,cJSON_CreateString(MODULE_NAME)); 

    cJSON_AddItemToObject(cj_query, "extern_data", extern_data = cJSON_CreateObject());//get时带参数extern_data,添加形式为:关键字:值
    cJSON_AddItemToObject(extern_data,"mac_list",mac_list = cJSON_CreateArray());    //添加mac_list对象，存放的是mac地址
    for(i = 0; i < param->cnt; i++)
    {
        cJSON_AddItemToArray(mac_list,cJSON_CreateString(param->ids[i].val));
    }
    cJSON_AddNumberToObject(extern_data,"cnt",param->cnt); //添加mac地址个数

   
	ret = app_get_module_param(cj_query,cj_get);	//调用公共get函数获取数据
   
    
    cnt = cjson_get_number(cj_get,"cnt",0);		//获取设置数量
    info->cnt = cnt;

    name_pair_s = cJSON_GetObjectItem(cj_get,"name_pair_s");  //name_pair_s取出的为数组，里边存放的是别名对
    if(NULL == name_pair_s)
    {
        cJSON_Delete(cj_query);
        cJSON_Delete(cj_get);
        return 1;
    }
    for(i = 0; i < cnt; i++)
    {
        pairitem = cJSON_GetArrayItem(name_pair_s,i);
        if(NULL == pairitem)
        {
            cJSON_Delete(cj_query);
            cJSON_Delete(cj_get);
            return 1;
        }
        id = cjson_get_value(pairitem,"id","");		//获取mac地址
        nickname = cjson_get_value(pairitem,"nickname","");	//获取别名
        if((NULL != id) && (NULL != nickname))
        {
            strcpy(info->name_pairs[i].id.val,id);      //填充mac地址
            strcpy(info->name_pairs[i].nickname,nickname);  //填充别名
        }
    }
    
    cJSON_Delete(cj_query);
    cJSON_Delete(cj_get);

	return 0;
}

/*************************************************************************
  功能: 设置设备别名(修改备注)
  参数: set_info 指定要设置哪些设备的别名
  返回值: 0成功，1失败
  是否需要用户实现: 是
  cj_query格式:{"module":["APP_SET_DEV_REMARK"]}
  cj_get格式:{"name_pairs":[{"id":"44:37:e6:9e:7e:a2","nickname":"1111111111"}, 
             {"id":"90:67:1c:f8:d8:4a","nickname":"2222222222222222"}, 
             {"id":"C8:3A:35:48:B2:02","nickname":"3333333333333333333"}],"cnt":3}
 ************************************************************************/
int biz_m_dev_nickname_info_set(const api_cmd_t *cmd, 
									     nickname_info_t *set_info,
									     void *privdata)
{      
    int ret = 0,i = 0;
	cJSON *cj_get 	= NULL;
	cJSON *module 	= NULL;
	cJSON *cj_query = NULL;
    cJSON *cj_set = NULL;
    cJSON *name_pairs = NULL;
    cJSON *obj = NULL;

    if(NULL == set_info) 
    {
        printf("nickname set info is error\n");
		return 1;
	}
    
    cj_query = cJSON_CreateObject();
    cj_set = cJSON_CreateObject();
    cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
    cJSON_AddItemToArray(module,cJSON_CreateString(SET_DEV_REMARK)); //如后续需要添加其他模块，照此方式往module中添加

    //cj_set用于存放设置的数据
    cJSON_AddItemToObject(cj_set,"name_pairs",name_pairs = cJSON_CreateArray()); //添加name_pairs对象，存放别名对
    for(i = 0; i < set_info->cnt; i++)
    {
        cJSON_AddItemToArray(name_pairs,obj = cJSON_CreateObject());
        if(NULL == obj)
        {
            printf("func:%s line:%d is error\n",__func__,__LINE__);
			cJSON_Delete(cj_query);
			cJSON_Delete(cj_set);
            return 1;
        }
        cJSON_AddStringToObject(obj,"id",set_info->name_pairs[i].id.val);   //往对象中添加id
        cJSON_AddStringToObject(obj,"nickname",set_info->name_pairs[i].nickname);  //往对象中添加别名
    }
    cJSON_AddNumberToObject(cj_set,"cnt",set_info->cnt); //往对象中添加别名数量

    app_set_module_param(cj_query,cj_set);	//调用公共set函数进行设置
   
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);   
	return 0;
}

