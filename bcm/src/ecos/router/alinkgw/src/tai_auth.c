/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_auth.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.30
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.30   1.0          learn from linux
************************************************************/
#include <string.h>
#include <stdio.h> 
#include <stdlib.h>
#include <bcmnvram.h>
#include "flash_cgi.h"
#include "route_cfg.h"
#include "alinkgw_api.h"
#include "tai.h"
#include "cJSON.h"
#include  "../../rc/rc.h"

extern login_ip_time loginUserInfo[MAX_USER_NUM];
extern char  g_Pass[64];

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="; 
char *nkgw_base64_encode(const char* data, int data_len); 
char *nkgw_base64_decode(const char* data, int data_len); 
static char find_pos(char ch);

static char find_pos(char ch)   
{ 
    char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[] 
    return (ptr - base); 
} 


char *nkgw_base64_encode(const char* data, int data_len) 
{ 
    //int data_len = strlen(data); 
    int prepare = 0; 
    int ret_len; 
    int temp = 0; 
    char *ret = NULL; 
    char *f = NULL; 
    int tmp = 0; 
    unsigned char changed[4]; 
    int i = 0; 
    ret_len = data_len / 3; 
    temp = data_len % 3; 
    if (temp > 0) 
    { 
        ret_len += 1; 
    } 
    ret_len = ret_len*4 + 1; 
    ret = (char *)malloc(ret_len); 
      
    if ( ret == NULL) 
    { 
        printf("No enough memory.\n"); 
        exit(0); 
    } 
    memset(ret, 0, ret_len); 
    f = ret; 
    while (tmp < data_len) 
    { 
        temp = 0; 
        prepare = 0; 
        memset(changed, '\0', 4); 
        while (temp < 3) 
        { 
            //printf("tmp = %d\n", tmp); 
            if (tmp >= data_len) 
            { 
                break; 
            } 
            prepare = ((prepare << 8) | (data[tmp] & 0xFF)); 
            tmp++; 
            temp++; 
        } 
        prepare = (prepare<<((3-temp)*8)); 
        //printf("before for : temp = %d, prepare = %d\n", temp, prepare); 
        for (i = 0; i < 4 ;i++ ) 
        { 
            if (temp < i) 
            { 
                changed[i] = 0x40; 
            } 
            else 
            { 
                changed[i] = (prepare>>((3-i)*6)) & 0x3F; 
            } 
            *f = base[changed[i]]; 
            //printf("%.2X", changed[i]); 
            f++; 
        } 
    } 
    *f = '\0'; 
      
    return ret; 
      
} 

char *nkgw_base64_decode(const char *data, int data_len) 
{ 
    int ret_len = (data_len / 4) * 3; 
    int equal_count = 0; 
    char *ret = NULL; 
    char *f = NULL; 
    int tmp = 0; 
    int temp = 0; 
    char need[3]; 
    int prepare = 0; 
    int i = 0; 
	
    if (*(data + data_len - 1) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 2) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 3) == '=') 
    {//seems impossible 
        equal_count += 1; 
    } 
    switch (equal_count) 
    { 
    case 0: 
        ret_len += 4;//3 + 1 [1 for NULL] 
        break; 
    case 1: 
        ret_len += 4;//Ceil((6*3)/8)+1 
        break; 
    case 2: 
        ret_len += 3;//Ceil((6*2)/8)+1 
        break; 
    case 3: 
        ret_len += 2;//Ceil((6*1)/8)+1 
        break; 
    } 
    ret = (char *)malloc(ret_len); 
    if (ret == NULL) 
    { 
        printf("No enough memory.\n"); 
        exit(0); 
    } 
    memset(ret, 0, ret_len); 
    f = ret; 	
    while (tmp < (data_len - equal_count)) 
    { 
        temp = 0; 
        prepare = 0; 
        memset(need, 0, 3); 
        while (temp < 4) 
        { 
            if (tmp >= (data_len - equal_count)) 
            { 
                break; 
            } 
            prepare = (prepare << 6) | (find_pos(data[tmp])); 
            temp++; 
            tmp++; 
        } 
        prepare = prepare << ((4-temp) * 6); 
        for (i=0; i<3 ;i++ ) 
        { 
            if (i == temp) 
            { 
                break; 
            } 
            *f = (char)((prepare>>((2-i)*8)) & 0xFF);			
            f++; 
        } 
    } 
    *f = '\0'; 	
    return ret; 
}


int authDevice(const char *json_in, char *buff, unsigned int buf_sz)
{
	char buf[64];
	char req[64];
	char *p = NULL;
	char *decode;	


	if(buff == NULL || json_in == NULL)
	{
		return ALINKGW_ERR ;
	}

	LLDEBUG("[json_in:%s]\n" , json_in);
	p = strchr(json_in,':');
	if(p == NULL)
	{
		return ALINKGW_ERR ;
	}
	while(*p != '\0' && *p++!='\"');
	strncpy(req,p,64);
	p = req;
	while(p-req < 64 && *p++ != '\"');
	*--p = '\0';

	strcpy(buf , nvram_safe_get("http_passwd") ) ;
	
	if( strlen(buf) == 0)
	{
		if(strcmp("" , req )== 0 )
		{
			return ALINKGW_OK;
		}
		else
		{
			return ALINKGW_ERR;
		}
	}
	decode = nkgw_base64_decode(buf,strlen(buf));
	LLDEBUG("%s - %s\n",req,decode);
	if(decode == NULL)
	{
		return ALINKGW_ERR;
	}
	if(strcmp(req,decode) == 0)
	{
		free(decode);		
		return ALINKGW_OK;
	}	
	free(decode);
	return ALINKGW_ERR;
}
/*json_in:{ "current": "admin", "new": "12345" }*/
int ChangePassword(const char *json_in, char *buff, unsigned int buf_sz)
{
	printf("%s %d json_in:%s buff:%s buf_sz:%u\n",__func__,__LINE__,json_in ,buff ,buf_sz);
	char current_pwd[64] = {0}, new_pwd[64] = {0};
	char *encode = NULL;
	if( json_in == NULL)
	{
		return ALINKGW_ERR ;
	}
	cJSON * pJson = cJSON_Parse(json_in);
	if(NULL == pJson)
	{
		printf("cJSON_Parse error.\n");
		return ALINKGW_ERR;
	}
	cJSON * pSub = cJSON_GetObjectItem(pJson, "current");
	if(NULL == pSub)
	{
		printf("Without current_pwd \n");
		goto ERR;
	}
	if(pSub->valuestring ==NULL)
	{
		printf("current_pwd err.\n");
		goto ERR;
	}
	if(strlen(pSub->valuestring) >= sizeof(current_pwd))
	{
		printf("current_pwd len err.\n");
		goto ERR;
	}
	strncpy(current_pwd, pSub->valuestring, strlen(pSub->valuestring));

	pSub = cJSON_GetObjectItem(pJson, "new");
	if(NULL == pSub)
	{
		printf("Without new_pwd \n");
		goto ERR;
	}
	if(pSub->valuestring ==NULL)
	{
		printf("new_pwd err.\n");
		goto ERR;
	}
	if(strlen(pSub->valuestring) >= sizeof(new_pwd))
	{
		printf("new_pwd len err.\n");
		goto ERR;
	}
	strncpy(new_pwd, pSub->valuestring, strlen(pSub->valuestring));
	
	
	printf("get current_pwd:%s new_pwd:%s ,old g_Pass:%s \n",current_pwd, new_pwd, g_Pass);

	//先检验current 和nvram配置
	if(strcmp(current_pwd,"") == 0 )
	{
		if(strcmp(nvram_safe_get(HTTP_PASSWD),"") != 0)
		{
			printf("current match err.\n");
			goto ERR;
		}
	}
	else
	{
		encode = nkgw_base64_encode(current_pwd, strlen(current_pwd));
		if(encode == NULL)
			goto ERR;
		printf("current_pwd encode:%s \n",encode);
		if(strcmp(nvram_safe_get(HTTP_PASSWD),encode) != 0)
		{
			printf("current match err.\n");
			free(encode);
			encode = NULL;
			goto ERR;
		}
		free(encode);
		encode = NULL;
	}

	//设置新密码
	if(strcmp(new_pwd,"") == 0 )
	{
		_SET_VALUE(HTTP_PASSWD, "");
		_SET_VALUE(HTTP_DEFAULTPWD1, "0");
		strncpy(g_Pass,"",sizeof(g_Pass));
	}
	else
	{
		encode = nkgw_base64_encode(new_pwd, strlen(new_pwd));
		if(encode == NULL)
		{
			goto ERR;
		}
		printf("new_pwd encode:%s \n",encode);
		_SET_VALUE(HTTP_PASSWD, encode);
		_SET_VALUE(HTTP_DEFAULTPWD1, "1");
		strncpy(g_Pass, encode, sizeof(g_Pass));
		free(encode);
		encode = NULL;
	}
		
	_COMMIT();
	//清空所有web登陆者信息
	memset(loginUserInfo, 0x0 , sizeof(login_ip_time) * MAX_USER_NUM);
	if(pJson)
		cJSON_Delete(pJson);
	return ALINKGW_OK;

ERR:
	printf("ChangePassword err, return.\n");
	if(pJson)
		cJSON_Delete(pJson);
	return ALINKGW_ERR;
}

