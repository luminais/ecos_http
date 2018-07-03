/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_security.c
	Description:al api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.27
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.2.28   1.0          new
************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alinkgw_api.h"
#include "cJSON.h"
#include "flash_cgi.h"
#include "tai.h"


#define SECURITY_BUFF_LEN		64

#define SECURITY_BUFF 			"{\"enabled\":\"%d\" , \"urlProtectNum\":\"%d\" , \"dnsProtectNum\":\"%d\"}"

#define ALINKGW_ATTR_URL_PROTECT      "urlProtectInfo"

int dns_fish_times = 0 ;
int dns_hijack_times = 0 ;
int al_security_flag = 1 ;

extern char globle_alsec_info[300] ;

extern void al_security_start(int flag);

extern int ali_get_connection_status() ;


int get_alsecurity_flag()
{
	return al_security_flag ;
}

int get_dns_finish_times()
{
	return dns_fish_times ;
}

int get_dns_hijack_times()
{
	return dns_hijack_times ;
}

int get_ali_sec_info(char *buff, unsigned int buff_sz)
{
	
	if(buff == NULL || buff_sz <= 0)
	{
		return -1 ;
	}

	if(buff_sz < SECURITY_BUFF_LEN)
	{
		return -2 ;
	}
	
	snprintf(buff , buff_sz , SECURITY_BUFF ,get_alsecurity_flag() , get_dns_finish_times() , get_dns_hijack_times());
	
	return 0 ;
}


int set_ali_sec_info(const char *json_in)
{
	cJSON * pJson = NULL ;
	cJSON * pSub = NULL ;

	if(!json_in)
	{
		
		AL_SHOW("BBBBBBBBB\n");
		
		return ALINKGW_ERR;	
	}
	
	AL_2_DEBUG("[json_in:%s]\n" , json_in);
	
	pJson = cJSON_Parse(json_in);
	if(!pJson)
	{
		AL_SHOW("cJSON_Parse error.\n");
		return ALINKGW_ERR;
	}	

	pSub = cJSON_GetObjectItem(pJson, "enabled");
	if(!pSub || pSub->valuestring ==NULL)
	{
		AL_SHOW("elabled NULL \n");
		goto ERR;
	}
	
	if(strcmp("0" , pSub->valuestring ) && strcmp("1" , pSub->valuestring ))
	{
		goto ERR;
	}

	al_security_flag = atoi(pSub->valuestring) ;

	al_security_start(al_security_flag) ;

	return ALINKGW_OK ;

ERR :

	if(pJson)
	{
		cJSON_Delete(pJson);
	}

	return ALINKGW_ERR; 
}

extern int ali_get_connection_status();

int report_ali_sec_info(void)
{

	if(ali_get_connection_status())
	{
		if( ALINKGW_OK != ALINKGW_report_attr(ALINKGW_ATTR_ALI_SECURITY))
		{
			printf("[debug] [%s], report attibute(%s) failed\n", __func__, ALINKGW_ATTR_ALI_SECURITY);
			
			return ALINKGW_ERR;
		}
	}
	return ALINKGW_OK ;
}

int report_ali_sec_info_direct(void)
{
	if( ali_get_connection_status() )
	{
		if(0 != ALINKGW_report_attr_direct(ALINKGW_ATTR_URL_PROTECT, ALINKGW_ATTRIBUTE_complex, globle_alsec_info))
		{
			printf("repoot alinkgw url protect failure!!!\n");
		}
	}

	return ALINKGW_OK ;
}


