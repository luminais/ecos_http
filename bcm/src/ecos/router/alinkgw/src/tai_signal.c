/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_signal.c
	Description:Wifi api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.6.29
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang      2015.6.29   	1.0          new
************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include "route_cfg.h"
#include "alinkgw_api.h"
#include "tai.h"
#include "cJSON.h"
#include "flash_cgi.h"


#define MIN_PAMODE_LEN		4



int get_wlanpamode(char *buff, unsigned int buff_sz)
{
	char *value = NULL ;
	int wl_mode = -1 ;
	if(!buff)
	{
		return ALINKGW_ERR;	
	}

	if(buff_sz <= MIN_PAMODE_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}
	
	_GET_VALUE("wl_ctv_pwr_percent", value);

	if(!strcmp("100" , value))
	{
		snprintf(buff , buff_sz , "%s" , "2");
	}
	else if(!strcmp("92" , value))
	{
		snprintf(buff , buff_sz , "%s" , "1");
	}
	else if(!strcmp("80" , value))
	{
		snprintf(buff , buff_sz , "%s" , "0");
	}
	else
	{
		return ALINKGW_ERR ;
	}

	AL_2_DEBUG("[buff:%s]\n" , buff);

	return ALINKGW_OK ;
	 
}


int set_wlanpamode(const char *json_in )
{

	char temp_str[ARRAY_LEN_4] = {0} ;
	
	AL_2_DEBUG("json_in:%s\n" , json_in);

	if(!json_in)
	{
		return ALINKGW_ERR ;
	}

	if(strcmp("0" , json_in) && strcmp("1" , json_in) && strcmp("2" , json_in))
	{
		return ALINKGW_ERR ;
	}

	strncpy(temp_str , json_in , ARRAY_LEN_4) ;

	if(!strcmp("0" , temp_str))
	{
		_SET_VALUE("wl_ctv_pwr_percent", "80");
		_SET_VALUE("wl_ctv_power", "middle");
	}
	else if(!strcmp("1" , temp_str))
	{
		_SET_VALUE("wl_ctv_pwr_percent", "92");
		_SET_VALUE("wl_ctv_power", "middle");
	}		
	else if(!strcmp("2" , temp_str))
	{
		_SET_VALUE("wl_ctv_pwr_percent", "100");
		_SET_VALUE("wl_ctv_power", "high");
	}

	_COMMIT();
		
	_RESTART();

	return ALINKGW_OK ;
}



