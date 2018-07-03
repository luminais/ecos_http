/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_subdev.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.27
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.27   1.0          new
************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "alinkgw_api.h"
#include "tai_hash.h"
#include "tai.h"
#include "tenda_client.h"
#include "tc.h"

#define SUB_MIN_LEN	 8

extern unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];  //0:up, 1: down;

extern 	int get_mac_relevent_ipindex(char * mac_str) ;

int varify_mac_str(char * var)
{
	int i ;
	if(strlen(var) != 17)
	{
		return ALINKGW_ERR ;
	}
	
	//广播和组播
	if( strtol((var + 1) , NULL , 16)& 1 ==  1)
	{
		return ALINKGW_ERR ;
	}
	
	for(i = 0 ; i < strlen(var) ; i++)
	{
		if((i+1)%3 == 0)
		{
			if(*(var + i) != ':')
			{
				return ALINKGW_ERR ;
			}
		}
		else
		{
			if(!isxdigit(*(var + i)))
			{
				return ALINKGW_ERR ;
			}
		}
	}
	return ALINKGW_OK ;
}

//获取实时速率单位字节每秒
int get_stream_statistic_speed_sub(char * subdev_mac , int flag ,unsigned long * currect_speed)
{
	int index = 0 ;
	index = get_mac_relevent_ipindex(subdev_mac) ;

	if(index < 0 || index >= TC_CLIENT_NUMBER)
	{
		return ALINKGW_ERR ;
	}

	if(flag)//down
	{
		*currect_speed = ip_dbs(index);
	}
	else//up
	{
		*currect_speed = ip_ubs(index);
	}

	return ALINKGW_OK ;
}


int subdevice_get_downspeed_value(const char *subdev_mac , char *buf,  unsigned int buf_sz)
{
	unsigned long down_speed = 0  ;
	
	if(!subdev_mac || !buf )
	{
		return ALINKGW_ERR ;
	}

	if(buf_sz < SUB_MIN_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}
	
	if(varify_mac_str(subdev_mac))
	{
		return ALINKGW_ERR ;
	}

	if(get_stream_statistic_speed_sub(subdev_mac , 1 , &down_speed))
	{
		return ALINKGW_ERR ;
	}

	snprintf(buf , SUB_MIN_LEN , "%ld" , down_speed);

	AL_2_DEBUG("[buf:%s]\n" , buf);
	return ALINKGW_OK ;
}


int subdevice_get_upspeed_value(const char *subdev_mac , char *buf,  unsigned int buf_sz)
{
	unsigned long up_speed = 0 ;

	if(!subdev_mac || !buf )
	{
		return ALINKGW_ERR ;
	}

	if(buf_sz < SUB_MIN_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}
	
	if(varify_mac_str(subdev_mac))
	{
		return ALINKGW_ERR ;
	}


	if(get_stream_statistic_speed_sub(subdev_mac , 0 , &up_speed))
	{
		return ALINKGW_ERR ;
	}

	snprintf(buf , SUB_MIN_LEN , "%ld" , up_speed);
	
	AL_2_DEBUG("[buf:%s]\n" , buf);
	
	return ALINKGW_OK ;
}


