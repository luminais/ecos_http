/***********************************************************
Copyright (C), 2013, Tenda Tech. Co., Ltd.
FileName: cgi_web.c
Description: 开机检测网络环境线程 
Author: hqw;
Version : 1.0
Date: 2013-11-05
Function List:   
 

History:
<author>   <time>   <version >   <desc>
hqw        2013-11-05   1.0       后台数据按照前台要求格式进行转换
************************************************************/
#include <stdio.h>
#include <string.h>
extern void string_cat(char *str , char *str1 , char *str2);
extern void json_cat(char *str, char*json);

/************************************************************
Function:	 string_cat               
Description: 	 将str2接到str1后面并且按照web要求的格式加入头部和尾部

Input:          字符串指针str1，字符串指针str2    ,type标志是否要加','                           

Output: 	      格式为str+"str1":"str2"

Return:      

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

void string_cat(char *str , char *str1 , char *str2)
{ 
	//"str1"
	strncat(str,"\"",strlen("\""));
	strncat(str,str1,strlen(str1));
	strncat(str,"\"",strlen("\""));
	
	strncat(str,":",strlen(":"));
	//"str2"
	strncat(str,"\"",strlen("\""));
	strncat(str,str2,strlen(str2));
	strncat(str,"\"",strlen("\""));
	return ;
}

void json_cat(char *str, char*json)
{
	char *char_p = NULL, *char_q = NULL;

	if(NULL==str || NULL==json)
	{
		diag_printf("[%s][%d] invalid para!!!\n", __FUNCTION__, __LINE__);
		return;
	}

	char_p = strchr(json, '{');

	if(NULL == char_p)
	{
		diag_printf("[%s][%d] invalid json!!!\n", __FUNCTION__, __LINE__);
		return;
	}
	
	char_p++;
	
	char_q = strrchr(char_p, '}');
	*char_q = '\0';
	
	strcat(str, char_p);

	return;
}

