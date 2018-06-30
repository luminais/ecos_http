/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_wifiswitch.c
	Description:Wifi api
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
#include <bcmnvram.h>
#include "alinkgw_api.h"
#include "tai.h"

#define WLAN_SWITCH_STATE_LEN				8
extern int gWifiStatusConfig;
extern int g_cur_wl_radio_status ;

extern void sys_restart(void);

int get_wlan_switch_state(char *buff, unsigned int buff_sz)
{
	
	if(buff == NULL)
	{
		return ALINKGW_ERR ;
	}

	
	LLDEBUG("[buff_sz:%d]\n" , buff_sz) ;

	if(buff_sz <= WLAN_SWITCH_STATE_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}

	snprintf(buff , buff_sz , "%d" , g_cur_wl_radio_status);
	
	LLDEBUG("[buff:%s]\n" , buff ) ;

	return ALINKGW_OK ;
	
}

int set_wlan_switch_state(const char *json_in)
{

	int flag = 0 ;

	static char temp_wlanstate[WLAN_SWITCH_STATE_LEN] =   {"1"};
	
	if(json_in == NULL)
	{
		return ALINKGW_ERR ;
	}
	
	if(strlen(json_in) >= WLAN_SWITCH_STATE_LEN || strlen(json_in) <= 0)
	{
		return ALINKGW_ERR;
	}
	
	LLDEBUG("[json_in:%s]\n" , json_in ) ;

	strncpy(temp_wlanstate, json_in, strlen(json_in));

	if(strncmp(temp_wlanstate , "1" , strlen(json_in)) == 0 )
	{
		if(g_cur_wl_radio_status == 0)
		{
			SHOWDEBUG("start wifi");
			g_cur_wl_radio_status = 1 ;
			flag = 1 ;
		}
	}
	else if(strncmp(temp_wlanstate , "0" , strlen(json_in)) == 0 )
	{
		if(g_cur_wl_radio_status == 1)
		{
			SHOWDEBUG("stop wifi");
			g_cur_wl_radio_status = 0 ;
			flag = 1 ;
		}
	}
	else
	{
		SHOWDEBUG("[json_in error]\n" ) ;
		return ALINKGW_ERR;
	}

	if(1 == flag)
	{
		sys_restart();
		gWifiStatusConfig = 1;
	}
	return ALINKGW_OK;
	
}


int report_wlanswitch_state(void)
{
	if( ALINKGW_OK != ALINKGW_report_attr(ALINKGW_ATTR_WLAN_SWITCH_STATE))
	{
		printf("[debug] [%s], report attibute(%s) failed\n", __func__, ALINKGW_ATTR_WLAN_SWITCH_STATE);
		
		return ALINKGW_ERR;
	}
	return ALINKGW_OK ;
}

int check_wlanswitch_state(void)
{
	static int  last_wifi_status = -1 ;

	if(last_wifi_status != -1 )
	{
		if(last_wifi_status != g_cur_wl_radio_status )
		{
			if(ALINKGW_OK != ALINKGW_report_attr(ALINKGW_ATTR_WLAN_SWITCH_STATE))
			{
				printf("[debug] [%s], report attibute(%s) failed\n", __func__, ALINKGW_ATTR_WLAN_SWITCH_STATE);
			}
		}
	}

	last_wifi_status = g_cur_wl_radio_status ;

	return ALINKGW_OK ;
	
}


