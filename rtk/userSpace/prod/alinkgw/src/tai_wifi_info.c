/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_wifi_info.c
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
#include "alinkgw_api.h"
#include "tai.h"
#include "cJSON.h"
#include "flash_cgi.h"



#define WIFI_INFO_STR "{\"enable\":\"%s\" , \"mode\":\"%s\" , \"bssid\":\"%s\" , \"ssid\" :\"%s\" , \"ssidBroadcast\":\"%s\"}"

#define MIN_WIFI_INFO_LEN	256

extern int g_cur_wl_radio_status ;

struct wifi_status
{
	char enable[ARRAY_LEN_4] ;
	char mode[ARRAY_LEN_4] ;
	char bssid[ARRAY_LEN_20] ;
	char ssid[ARRAY_LEN_32] ;
	char ssidBroadcast[ARRAY_LEN_4] ;
} ;


static int wlansetting_get_enable(struct wifi_status * p )
{

	snprintf(p->enable , ARRAY_LEN_4 , "%d" , g_cur_wl_radio_status);//?????????ò?????????????
	
	return ALINKGW_OK ;
}


static int wlansetting_get_mode(struct wifi_status * p)
{
	if(!strcmp("-1" , nvram_safe_get("wl0_nmode")) && !strcmp("1" , nvram_safe_get("wl0_gmode")) )//11bgn
	{
		strcpy(p->mode , "bgn") ;
	}
	else if(!strcmp("0" , nvram_safe_get("wl0_nmode"))&& !strcmp("0" , nvram_safe_get("wl0_gmode")))//11b
	{
	
		strcpy(p->mode , "b") ;
	}
	else if(!strcmp("0" , nvram_safe_get("wl0_nmode")) && !strcmp("1" , nvram_safe_get("wl0_gmode")))//11bg
	{
	
		strcpy(p->mode , "bg") ;
	}
	else if(!strcmp("0" , nvram_safe_get("wl0_nmode"))&& !strcmp("2" , nvram_safe_get("wl0_gmode")))//11g
	{
	
		strcpy(p->mode , "g") ;
	}
	else
	{
		return ALINKGW_ERR ;
	}

	
	return ALINKGW_OK ;
}


static int wlansetting_get_bssid(struct wifi_status * p)
{

	strncpy(p->bssid ,nvram_safe_get("wl0_hwaddr") , ARRAY_LEN_20 ) ;
	
	return ALINKGW_OK ;
}

static int wlansetting_get_ssid(struct wifi_status * p )
{
	int wl_mode = - 1;

	wl_mode = get_wl_mode();

	if(wl_mode != 0 && wl_mode != 1)
	{
		return ALINKGW_ERR ;
	}

	if(!wl_mode)//ap
	{
		strncpy(p->ssid ,nvram_safe_get("wl0_ssid") , ARRAY_LEN_32) ;
	}
	else
	{
		strncpy(p->ssid ,nvram_safe_get("wl0.1_ssid") , ARRAY_LEN_32) ;
	}
	
	return ALINKGW_OK ;
}

static int wlansetting_get_ssidBroadcast(struct wifi_status * p )
{
	if(!strcmp(nvram_safe_get("wl0_closed") , "0"))
	{
		strncpy(p->ssidBroadcast ,"1" , ARRAY_LEN_4);
	}
	else
	{
		strncpy(p->ssidBroadcast ,"0" , ARRAY_LEN_4);
	}

	return ALINKGW_OK ;
}

static int (*wifi_setting_get[])(struct wifi_status * p ) =
{
	wlansetting_get_enable,
	wlansetting_get_mode,
	wlansetting_get_bssid,
	wlansetting_get_ssid,
	wlansetting_get_ssidBroadcast
} ;


int get_wlansetting24g(char *buff, unsigned int buff_sz)
{
	int i = 0 ;
	int ret = 0 ;
	
	struct wifi_status wifi_info24 ;
	
	if(!buff)
	{
		return ALINKGW_ERR;	
	}

	if(buff_sz <= MIN_WIFI_INFO_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}
	
	memset(&wifi_info24 , 0x0 , sizeof(wifi_info24));

	for(i = 0 ; i < sizeof(wifi_setting_get)/sizeof(wifi_setting_get[0]) ; i++)
	{
		ret = wifi_setting_get[i](&wifi_info24) ;
		
		if(ret == ALINKGW_ERR)
		{
			return ALINKGW_ERR ;
		}
	}

	snprintf(buff , buff_sz , WIFI_INFO_STR , wifi_info24.enable , wifi_info24.mode , 
		wifi_info24.bssid , wifi_info24.ssid , wifi_info24.ssidBroadcast);

	AL_2_DEBUG("[wifi_setting info:%s]\n" , buff) ;
	
	return ALINKGW_OK ;
}


static int verify_wifi_set_enable(char * var , struct wifi_status *p)
{

	if(strcmp("1" , var)!= 0 && strcmp("0" , var)!= 0 )
	{
		return ALINKGW_ERR;	
	}
	
	strncpy(p->enable, var , ARRAY_LEN_4);
	
	return ALINKGW_OK ;
}


static int verify_wifi_set_mode(char * var , struct wifi_status *p)
{

	if(strcmp("0" , p->enable))
	{
		if(strcmp("b" , var) && strcmp("bg" , var) && 
		   strcmp("g" , var) && strcmp("bgn" , var))
		{
			return ALINKGW_ERR ;
		}
		
		strncpy(p->mode , var , ARRAY_LEN_4);
	}

	return ALINKGW_OK ;
}



static int verify_wifi_set_bssid(char *var , struct wifi_status *p)
{
	int i ;
	
	if(strcmp("0" , p->enable))
	{
		if(strlen(var) != 17)
		{
			return ALINKGW_ERR ;
		}

		if((*(var + 1))& 1 ==  1)//Multicast
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

		strncpy(p->bssid , var , sizeof(p->bssid));
	}
	return ALINKGW_OK ;
}


static int verify_wifi_set_ssid(char *var , struct wifi_status *p)
{

	if(strcmp("0" , p->enable))
	{
		if(strlen(var) == 0  || strlen(var) > 32)
		{
			return ALINKGW_ERR; 
		}

		strncpy(p->ssid , var , ARRAY_LEN_32);
	}
	return ALINKGW_OK ;
}


static int verify_wifi_set_ssidBroadcast(char *var , struct wifi_status *p)
{
	if(strcmp("0" , p->enable))
	{
		if(strcmp("1" , var) && strcmp("0" , var))
		{
			return ALINKGW_ERR;	
		}

		strncpy(p->ssidBroadcast , var , ARRAY_LEN_4);
	}
	return ALINKGW_OK ;
}


struct prase_struct
{
	char name[ARRAY_LEN_16] ;
	int (*f)(char* var , struct wifi_status *p) ;
};

static struct prase_struct wifi_prase_array[] =
{
	{"enable" 			, 		verify_wifi_set_enable			} , 
	{"mode" 			, 		verify_wifi_set_mode			} ,
	{"bssid" 			, 		verify_wifi_set_bssid			} ,
	{"ssid" 			, 		verify_wifi_set_ssid			} ,
	{"ssidBroadcast"	, 		verify_wifi_set_ssidBroadcast	}
};



static int prase_wlansetting24g_json(const char *json_in , struct wifi_status *p)
{

	int i  = 0 ;
	cJSON * pJson = NULL ;
	cJSON * pSub = NULL ;

	if(!json_in)
	{
		return ALINKGW_ERR;	
	}

	pJson = cJSON_Parse(json_in);
	if(!pJson)
	{
		AL_SHOW("cJSON_Parse error.\n");
		return ALINKGW_ERR;
	}

	for(i = 0 ; i < sizeof(wifi_prase_array)/sizeof(wifi_prase_array[0]) ; i++)
	{
		pSub = cJSON_GetObjectItem(pJson, wifi_prase_array[i].name);
		if(!pSub || pSub->valuestring ==NULL)
		{
			printf("%s NULL \n" , wifi_prase_array[i].name);
			goto ERR;
		}
		
		if(wifi_prase_array[i].f(pSub->valuestring , p))
		{
			printf("%s errer \n" , wifi_prase_array[i].name);
			goto ERR;
		}
	}

	if(pJson)
	{
		cJSON_Delete(pJson);
	}

	return ALINKGW_OK ;
	
ERR :

	if(pJson)
	{
		cJSON_Delete(pJson);
	}
	
	return ALINKGW_ERR ;
	
	
}


static int set_wifi_enable(struct wifi_status * p)
{

	if(g_cur_wl_radio_status != atoi(p->enable))
	{
		g_cur_wl_radio_status = atoi(p->enable) ;

		if(!atoi(p->enable))
		{
			return ALINKGW_DISABLE ;
		}
		
		return ALINKGW_UPDATE ;
	}
	else
	{
		if(!atoi(p->enable))
		{
			return ALINKGW_DISABLE ;
		}
	}
	
	return ALINKGW_OK ;
}

static int set_wifi_mode(struct wifi_status * p )
{
	if(!strcmp("b" , p->mode))
	{
		_SET_VALUE("wl0_gmode", "0");	
		_SET_VALUE("wl0_nmode", "0");	
		_SET_VALUE("wl0_plcphdr", "long"); 
		
		return ALINKGW_UPDATE ;
	}
	else if(!strcmp("bg" , p->mode))
	{
		_SET_VALUE("wl0_gmode", "1");	
		_SET_VALUE("wl0_nmode", "0");	
		_SET_VALUE("wl0_plcphdr", "long"); 
		
		return ALINKGW_UPDATE ;
	}
	else if(!strcmp("g" , p->mode))
	{
		_SET_VALUE("wl0_gmode", "2");
		_SET_VALUE("wl0_nmode", "0");
		_SET_VALUE("wl0_rateset", "default"); 
		
		return ALINKGW_UPDATE ;
	}
	else if(!strcmp("bgn" , p->mode))
	{
		_SET_VALUE("wl0_nmode", "-1");
		_SET_VALUE("wl0_gmode", "1");	
		_SET_VALUE("wl0_plcphdr", "long");
		
		return ALINKGW_UPDATE ;
	}

	return ALINKGW_OK ;
}

static int set_wifi_bssid(struct wifi_status * p )
{
#if 0
	if(strcmp(nvram_safe_get("sb/1/macaddr") , p->bssid))
	{		
		return ALINKGW_ERR ;
	}
#endif
	return ALINKGW_OK ;
}

static int set_wifi_ssid(struct wifi_status * p )
{

	int wl_mode = -1 ;

	wl_mode = get_wl_mode();

	if(wl_mode != 0 && wl_mode != 1)
	{
		return ALINKGW_ERR ;
	}

	if(!wl_mode)
	{
		if(strcmp(nvram_safe_get("wl0_ssid") , p->ssid))
		{
			_SET_VALUE("wl0_ssid" , p->ssid);
			
			return ALINKGW_UPDATE ;
		}
	}
	else
	{
		if(strcmp(nvram_safe_get("wl0.1_ssid") , p->ssid))
		{
			_SET_VALUE("wl0.1_ssid" , p->ssid);
			
			return ALINKGW_UPDATE ;
		}
	}

	return ALINKGW_OK ;
}

static int set_wifi_ssidBroadcast(struct wifi_status * p )
{
	if((!strcmp(nvram_safe_get("wl0_closed") , "0"))&& (!strcmp("0" , p->ssidBroadcast)))
	{
		_SET_VALUE("wl0_closed", "1");
	
		AL_2_DEBUG("[wl0_closed:%s]\n" , nvram_safe_get("wl0_closed"));
	
		return ALINKGW_UPDATE ;
	}
	else if((!strcmp(nvram_safe_get("wl0_closed") , "1")) && (!strcmp("1" , p->ssidBroadcast)))
	{
		_SET_VALUE("wl0_closed", "0");
		
		AL_2_DEBUG("[wl0_closed:%s]\n" , nvram_safe_get("wl0_closed"));
		
		return ALINKGW_UPDATE ;
	}

	
	return ALINKGW_OK ;
}

static int (*wifi_set_function[])(struct wifi_status * p)=
{
	set_wifi_enable,
	set_wifi_mode,
	set_wifi_bssid,
	set_wifi_ssid,
	set_wifi_ssidBroadcast
} ;


int set_wlansetting24g(const char *json_in)
{
	int i ;
	int result = 0 ;
	int change_nvram_flag = 0 ;
	struct wifi_status set_wifi_info24 ;

	memset(&set_wifi_info24 , 0x0 , sizeof(set_wifi_info24));
	
	if(prase_wlansetting24g_json(json_in , &set_wifi_info24))
	{
		return ALINKGW_ERR ;
	}

	for(i = 0 ; i < sizeof(wifi_set_function)/sizeof(wifi_set_function[0]) ; i++)
	{
		result = wifi_set_function[i](&set_wifi_info24) ;

		if(result == ALINKGW_UPDATE || result == ALINKGW_DISABLE)
		{
			if(change_nvram_flag == 0)
			{
				change_nvram_flag = 1 ;
			}

			if(result == ALINKGW_DISABLE)
			{
				break ;
			}
		}
		else if(result == ALINKGW_OK)
		{
			printf("tai_wifi_info.c %d is not set\n" , i);
			
		}
	}

	if(change_nvram_flag)
	{
		change_nvram_flag = 0 ;
		
		_COMMIT();
		
		_RESTART();
		
	}
	
	return ALINKGW_OK ;
}




