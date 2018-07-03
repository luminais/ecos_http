#include "cJSON.h"
/*****************************************************************************
 函 数 名  : cjson_get_number
 功能描述  : 从json数据格式中安全的读取数字，最后一个参数是默认值
 输入参数  : cJSON *root          
             char *name           
             int defaultGetValue  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年12月26日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
int cjson_get_number(cJSON *root, char *name, int defaultGetValue)
{
	if (!name)
	{	
		return -1;
	}

	if(!root)
	{
		printf("root is NULL!!! get name %s\n",name);	
		return defaultGetValue;
	}
	cJSON *obj = cJSON_GetObjectItem(root, name);

	if (!obj)
	{
		return defaultGetValue;
	}
	else
	{
		return obj->valueint;
	}
}

/*****************************************************************************
 函 数 名  : cjson_get_value
 功能描述  : 从json数据格式中安全的读取字符串，最后一个参数是默认值
 输入参数  : cJSON *root            
             char *name             
             char *defaultGetValue  
 输出参数  : 无
 返 回 值  : char
 
 修改历史      :
  1.日    期   : 2016年12月26日
    作    者   : 段靖铖
    修改内容   : 新生成函数

*****************************************************************************/
char *cjson_get_value(cJSON *root, char *name, char *defaultGetValue)
{
	if (!name)
	{
		return NULL;
	}

	if(!root)
	{
		printf("root is NULL!!! get name %s\n",name);	
		return defaultGetValue;
	}
	cJSON *obj = cJSON_GetObjectItem(root, name);

	if (!obj)
	{
		//printf("__%s__obj is not live",name);
		return defaultGetValue;
	}
	else
	{
		if (obj->valuestring)
		{
			//APP_MODULE_DEBUG("___in cjson_get_value  %s = %s\n", name, obj->valuestring);
			return obj->valuestring;
		}
		else
		{
			return "";
		}
	}
}

