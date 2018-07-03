/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_channel.c
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
#include <wlioctl.h>
#include "alinkgw_api.h"
#include "tai.h"
#include "cJSON.h"
#include "flash_cgi.h"

extern 	int get_wl_mode() ;

int start_ali_channel_service = 0 ;

int old_channel_value = -1 ;

int get_ali_channel_service_enable()
{
	return start_ali_channel_service ;
}

int set_ali_channel_service_enable()
{

	printf("###############come to set_ali_channel_service_enable\n");

	if(start_ali_channel_service)
	{
		start_ali_channel_service = 0 ;
	}
	return ALINKGW_OK ;
}


int rt_srv_refineWlanChannel(const char *json_in, char *buff, unsigned int buff_len)
{
	if(!start_ali_channel_service)
	{
		start_ali_channel_service =  1;

		if(strcmp("0" , nvram_safe_get("wl0_channel")))
		{
			nvram_set("wl0_channel" , "0" ) ;

			nvram_commit() ;
		}

		cyg_thread_delay(10);

		_RESTART();
		
	}
	return ALINKGW_OK ;
}

//–≈µ¿ Ù–‘
#define CHANNEL_INFO_STR "{\"channel\":\"%s\" ,\"bindwidth\":\"%s\" ,\"quality\":\"%s\"}"

#define MIN_WIFI_CHANNEL_LEN 128

struct channel_status
{
	char channel[ARRAY_LEN_4] ;
	char bandwidth[ARRAY_LEN_8] ;
	char quality[ARRAY_LEN_4] ;
};

struct channel_status wifi_channel ;

static int get_al_wl_prefix(char * ifname , int len , int mode)
{

	int unit=-1,subunit=-1;
	char *wl_unit=NULL;

	char tmp[ARRAY_LEN_16], prefix[] = "wlXXXXXXXXXX_";

	wl_unit = nvram_safe_get("wl_unit") ;

	if(!strcmp("" , wl_unit))
	{
		AL_SHOW("wl_unit =\"\"");
		return ALINKGW_ERR ;
	}

	if (get_ifname_unit(wl_unit,&unit,&subunit) < 0  || unit < 0)
	{
		AL_2_DEBUG("get_ifname_unit err , unit=%d\n" , unit);
		return ALINKGW_ERR;
	}

	if( !mode && subunit > 0)
	{
		snprintf(tmp , ARRAY_LEN_16 , "wl%d.%d_" , unit , subunit);
	}
	else
	{
		snprintf(tmp , ARRAY_LEN_16 , "wl%d_" , unit);
	}

	strncat(tmp , "ifname" , strlen("ifname")) ;

	strncpy(ifname , nvram_safe_get(tmp) , len );
	
	if(!strcmp("" , ifname))
	{
		return ALINKGW_ERR ;
	}

	return ALINKGW_OK ;
}

#if 0
static int get_wifi_channel(struct channel_status *p , char * ifname)
{

	int ret ;

	channel_info_t var ;

	ret = wl_ioctl(ifname, WLC_GET_CHANNEL, var, sizeof(var));

	printf("##var channel[%d] [%d] [%d]\n" , var.scan_channel , var.hw_channel , var.target_channel);

	if(ret < 0)
	{
		return ALINKGW_ERR ;
	}

	if(var.target_channel <= 0 && var.target_channel >= 14)
	{
		return ALINKGW_ERR ;
	}
	
	sprintf(p->channel , "%d" , var.target_channel);
	
	return ALINKGW_OK ;
	
}
#endif

static int get_wifi_channel(struct channel_status *p )
{

	sprintf(p->channel , "%s" , nvram_safe_get("wl0_channel"));


	return ALINKGW_OK ;
}



static int get_wifi_bandwidth(struct channel_status *p)
{
	if(!strcmp("1" , nvram_safe_get("wl0_nbw_cap")))
	{
		if(!strcmp("1" , nvram_safe_get("ht_bw_fake_auto")))
		{
			sprintf(p->bandwidth, "%s" , "20/40");
		}
		else
		{
			sprintf(p->bandwidth, "%s" , "40");
		}
	}
	else
	{
		sprintf(p->bandwidth, "%s" , "20");
	}

	return ALINKGW_OK ;

}

static int get_wifi_quality(struct channel_status *p )
{
	int ret ;
	int wl_mode = -1 ;
	int channel_quality = 0;
	char ifname[ARRAY_LEN_8] ;

	wl_mode = get_wl_mode() ;

	AL_2_DEBUG("[mode:%d]\n" , wl_mode) ;

	if(wl_mode != 0 && wl_mode != 1)
	{
		return ALINKGW_ERR ;
	}

	if(get_al_wl_prefix(ifname , sizeof(ifname) , wl_mode))
	{
		return ALINKGW_ERR ;
	}

	wl_ioctl(ifname , WLC_START_CHANNEL_QA , NULL, 0);

	cyg_thread_delay(50);

	wl_ioctl(ifname , WLC_GET_CHANNEL_QA , &channel_quality , sizeof(channel_quality));

	AL_2_DEBUG("[channel_quality:%d]\n" , channel_quality);
	
	switch(channel_quality)
	{
		case 0:
			sprintf(p->quality , "%d" , 1);
			break ;
			
		case 1:
			sprintf(p->quality , "%d" , 2);
			break ;
			
		case 2:			
		case 3:
			sprintf(p->quality , "%d" , 3);
			break ;
		
		default :
			sprintf(p->quality , "%d" , 2);
			break ;
			
	}

	return ALINKGW_OK ;

}


int (*get_channel_array[])(struct channel_status *p ) = 
{
	get_wifi_channel , 
	get_wifi_bandwidth , 
	get_wifi_quality
};


int get_wlan_channel_24g(char *buff, unsigned int buff_sz)
{
	int i = 0 ;
	int ret = 0 ;

	struct channel_status channel_info;
	if(!buff)
	{
		return ALINKGW_ERR;	
	}

	if(buff_sz <= MIN_WIFI_CHANNEL_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}

	memset(&channel_info , 0x0 , sizeof(channel_info));

	for(i = 0 ; i < sizeof(get_channel_array)/sizeof(get_channel_array[0]) ; i++ )
	{
		ret = get_channel_array[i](&channel_info) ;

		if(ret == ALINKGW_ERR)
		{
			AL_2_DEBUG("[i:%d]\n" , i) ;
			
			return ALINKGW_ERR ;
		}
	}
	
	snprintf(buff , buff_sz , CHANNEL_INFO_STR , channel_info.channel , channel_info.bandwidth , channel_info.quality);

	AL_2_DEBUG("[buff:%s]\n" , buff);
	
	return ALINKGW_OK ;

	
}

extern int ali_get_connection_status();

int report_wifichannel_state(void)
{
	if(ali_get_connection_status())
	{
		if( ALINKGW_OK != ALINKGW_report_attr(ALINKGW_ATTR_WLAN_CHANNEL_24))
		{				
			printf("######### report the "ALINKGW_ATTR_WLAN_CHANNEL_24" info err!\n");
			
			return ALINKGW_ERR;
		}
	}
	return ALINKGW_OK ;
}


int report_wifisecurity_state(void)
{
	if(ali_get_connection_status())
	{

		if( ALINKGW_OK != ALINKGW_report_attr(ALINKGW_ATTR_WLAN_SECURITY_24))
		{				
			printf("######### report the "ALINKGW_ATTR_WLAN_SECURITY_24" info err!\n");
			
			return ALINKGW_ERR;
		}
	}
	return ALINKGW_OK ;
}


int report_wifipamode_state(void)
{
	if(ali_get_connection_status())
	{
		if( ALINKGW_OK != ALINKGW_report_attr(ALINKGW_ATTR_WLAN_PAMODE))
		{				
			printf("######### report the "ALINKGW_ATTR_WLAN_PAMODE" info err!\n");
			
			return ALINKGW_ERR;
		}
	}
	return ALINKGW_OK ;
}

int report_wifiset_state(void)
{
	if(ali_get_connection_status())
	{

		if( ALINKGW_OK != ALINKGW_report_attr(ALINKGW_ATTR_WLAN_SETTING_24))
		{				
			printf("######### report the "ALINKGW_ATTR_WLAN_SETTING_24" info err!\n");
			
			return ALINKGW_ERR;
		}
	}
	return ALINKGW_OK ;
}





