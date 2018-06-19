/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_wifi_security.c
	Description:Wifi api
	Author: Lvliang;
	Version : 2.0
	Date: 2015.6.29
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang      2015.7.2   	1.0          new
************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include "route_cfg.h"
#include "alinkgw_api.h"
#include "tai.h"
#include "cJSON.h"
#include "flash_cgi.h"



#define MIN_WIFI_SEC_LEN 256

#define WIFI_SEC_STR "{\"enabled\":\"%s\" , \"type\":\"%s\" , \"encryption\":\"%s\" , \"passphrase\":\"%s\"}"

struct wifi_security 
{
	char enable[ARRAY_LEN_4] ;
	char type[ARRAY_LEN_8] ;
	char encryption[ARRAY_LEN_12] ;
	char passhrase[ARRAY_LEN_64] ;
};


int get_wl_mode()
{
	char temp_mode[ARRAY_LEN_8] = { 0 } ;
	strncpy(temp_mode, nvram_safe_get("wl0_mode") , ARRAY_LEN_8);
	
	if(strcmp("ap" , temp_mode) == 0)//ap
	{
		return 0 ;
	}
	else if(strcmp("sta" , temp_mode) || strcmp("wet" , temp_mode))
	{
		return 1 ;
	}

	return -1 ;
	
}



static int wlansecurity_get_enable(struct wifi_security * p , int mode)
{
	char *value = NULL ;

	if(!mode)//ap
	{
		_GET_VALUE("wl0_akm", value);
	}
	else
	{
		_GET_VALUE("wl0.1_akm", value);
	}
	
	
	if(!strcmp("" , value))
	{
		strncpy(p->enable , "0" , ARRAY_LEN_4);
	}
	else if(!strcmp("psk"  , value )|| !strcmp("psk2"  , value )|| !strcmp("psk psk2"  , value))
	{
		strncpy(p->enable , "1" , ARRAY_LEN_4);
	}
	else
	{
		return ALINKGW_ERR ;
	}
	return ALINKGW_OK ;
}
	

static int wlansecurity_get_type(struct wifi_security * p , int mode)
{
	char *value = NULL ;

	if(!mode)//ap
	{
		_GET_VALUE("wl0_akm", value);
	}
	else
	{
		_GET_VALUE("wl0.1_akm", value);
	}

	if(!strcmp("" , value))
	{
		strncpy(p->type, "" , ARRAY_LEN_8);
	}
	else if(!strcmp("psk"  , value))
	{
		strncpy(p->type, "WPA" , ARRAY_LEN_8);
	}
	else if(!strcmp("psk2"  , value))
	{
		strncpy(p->type , "WPA2" , ARRAY_LEN_8);
	}
	else if(!strcmp("psk psk2" , value))
	{
		strncpy(p->type , "WPA+2" , ARRAY_LEN_8);
	}
	else
	{
		return ALINKGW_ERR ;
	}

	return ALINKGW_OK ;
	
}


static int wlansecurity_get_encryption(struct wifi_security * p , int mode)
{
	char *value = NULL ;

	if(!mode)//ap
	{
		_GET_VALUE("wl0_crypto", value);
	}
	else
	{
		_GET_VALUE("wl0.1_crypto", value);
	}

	if(!strcmp("" , value))
	{
		strncpy(p->encryption , "" , ARRAY_LEN_12);
	}
	else if(!strcmp("aes" , value))
	{
		strncpy(p->encryption , "AES" , ARRAY_LEN_12);
	}
	else if(!strcmp("tkip" , value))
	{
		strncpy(p->encryption , "TKIP" , ARRAY_LEN_12);
	}
	else if(!strcmp("tkip+aes" , value))
	{
		strncpy(p->encryption , "AES+TKIP" , ARRAY_LEN_12);
	}
	else
	{
		return ALINKGW_ERR ;
	}
	return ALINKGW_OK ;
	
}

static int wlansecurity_get_passhrase(struct wifi_security * p , int mode)
{
	char *value = NULL ;
	if(!mode)//ap
	{
		_GET_VALUE("wl0_wpa_psk", value);
	}
	else
	{
		_GET_VALUE("wl0.1_wpa_psk", value);
	}
	
	strncpy(p->passhrase, value , ARRAY_LEN_64);
	
	return ALINKGW_OK ;
}


static int (*wifi_sec_get[])(struct wifi_security * p , int mode) =
{
	wlansecurity_get_enable,
	wlansecurity_get_type,
	wlansecurity_get_encryption,
	wlansecurity_get_passhrase
} ;

int get_wlansecurity24g(char *buff, unsigned int buff_sz)
{
	int i ;
	int ret ;
	int wl_mode = -1 ;
	struct wifi_security wifi_sec_info ;
	if(!buff)
	{
		return ALINKGW_ERR;	
	}

	if(buff_sz < MIN_WIFI_SEC_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}

	memset(&wifi_sec_info , 0x0 , sizeof(wifi_sec_info)) ;

	wl_mode = get_wl_mode();

	if(wl_mode != 0 && wl_mode != 1)
	{
		return ALINKGW_ERR ;
	}


	for(i = 0 ; i < sizeof(wifi_sec_get)/ sizeof(wifi_sec_get[0]) ; i++)
	{
		ret = wifi_sec_get[i](&wifi_sec_info , wl_mode) ;
		
		if(ret == ALINKGW_ERR)
		{
			AL_2_DEBUG("[wifi_security info:%d]\n" , i) ;
			
			return ALINKGW_ERR ;
		}
	}

	snprintf(buff , buff_sz , WIFI_SEC_STR , wifi_sec_info.enable , wifi_sec_info.type , wifi_sec_info.encryption , wifi_sec_info.passhrase);

	AL_2_DEBUG("[wifi_security info:%s]\n" , buff) ;
	
	return ALINKGW_OK ;
	
}



static int verify_wifi_sec_enable(char * var , struct wifi_security * p)
{

	if(strcmp("1" , var) && strcmp("0" , var) )
	{
		AL_SHOW("verify_wifi_sec_enable\n");
		
		return ALINKGW_ERR;	
	}
	strncpy(p->enable , var , ARRAY_LEN_4);
	
	return ALINKGW_OK ;
}

static int verify_wifi_sec_type(char * var , struct wifi_security * p)
{
	if(strcmp("0" , p->enable))
	{
		if(strcmp("WPA" , var) && strcmp("WPA2" , var) && strcmp("WPA+2" , var))
		{
			AL_SHOW("verify_wifi_sec_type\n");
			
			return ALINKGW_ERR; 
		}
		
		strncpy(p->type , var , ARRAY_LEN_8);
	}

	
	return ALINKGW_OK ;
}

static int verify_wifi_sec_encryption(char * var , struct wifi_security * p)
{

	if(strcmp("0" , p->enable))
	{
		if(strcmp("AES" , var) && strcmp("TKIP" , var) && strcmp("AES+TKIP" , var))
		{
			AL_SHOW("verify_wifi_sec_encryption\n");
			
			return ALINKGW_ERR; 
		}

		if(!strcmp("WPA" , p->type))
		{
			if(!strcmp("AES+TKIP" , var))
			{
				AL_SHOW("verify_wifi_sec_encryption\n");
				
				return ALINKGW_ERR ;
			}
		}

		strncpy(p->encryption , var , ARRAY_LEN_12);
		
	}


	return ALINKGW_OK ;
}
static int verify_wifi_sec_passphrase(char * var , struct wifi_security * p)
{

	if(strcmp("0" , p->enable))
	{
		if(strlen(var) < ARRAY_LEN_8 || strlen(var) > ARRAY_LEN_64)
		{
			AL_SHOW("verify_wifi_sec_passphrase\n");
			
			return ALINKGW_ERR ;
		}
		
		strncpy(p->passhrase , var , ARRAY_LEN_64);
	}

	return ALINKGW_OK ;
}


struct prase_sec_struct
{
	char name[ARRAY_LEN_16] ;
	int (*f)(char* var , struct wifi_security * p) ;
};


static struct prase_sec_struct wifi_sec_prase_array[] =
{
	{"enabled" 			, 		verify_wifi_sec_enable		}, 
	{"type" 			, 		verify_wifi_sec_type		},
	{"encryption" 		, 		verify_wifi_sec_encryption	},
	{"passphrase" 		, 		verify_wifi_sec_passphrase	}
};


static int prase_wlansec24g_json(const char *json_in , struct wifi_security * p)
{
	AL_2_DEBUG("[json_in:%s]\n" , json_in);
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

	for(i = 0 ; i < sizeof(wifi_sec_prase_array)/sizeof(wifi_sec_prase_array[0]) ; i++)
	{
		pSub = cJSON_GetObjectItem(pJson, wifi_sec_prase_array[i].name);
		if(!pSub || pSub->valuestring ==NULL)
		{
			AL_2_DEBUG("%s NULL \n" , wifi_sec_prase_array[i].name);
			goto ERR;
		}
		
		if(wifi_sec_prase_array[i].f(pSub->valuestring , p))
		{
			AL_2_DEBUG("%s errer \n" , wifi_sec_prase_array[i].name);
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




static int wlansecurity_set_enable(struct wifi_security * p , int mode)
{

	char *value = NULL ;

	if(!mode)//ap
	{
		_GET_VALUE("wl0_akm", value);

		if(strcmp("" , value ) && !strcmp("0" , p->enable))
		{
			_SET_VALUE("wl0_akm" , "");
			
		}
		else
		{
			return ALINKGW_OK ;
		}
		
	}
	else
	{
		_GET_VALUE("wl0.1_akm", value);
		
		if(strcmp("" , value ) && !strcmp("0" , p->enable))
		{
			_SET_VALUE("wl0.1_akm" , "");
			
		}
		else
		{
			return ALINKGW_OK ;
		}
		
	}
		
	return ALINKGW_UPDATE;
}

static int wlansecurity_set_type(struct wifi_security * p , int mode )
{
	char *value = NULL ;

	if(!mode)//ap
	{
		_GET_VALUE("wl0_akm", value);
	}
	else
	{
		_GET_VALUE("wl0.1_akm", value);
	}

	if(strcmp("0" , p->enable))
	{

		if(!strcmp("WPA" , p->type) && strcmp("psk" , value) )
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_akm" , "psk");
			}
			else
			{
				_SET_VALUE("wl0.1_akm" , "psk");
			}
		}
		else if(!strcmp("WPA2" , p->type) && strcmp("psk2" , value) )
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_akm" , "psk2");
			}
			else
			{
				_SET_VALUE("wl0.1_akm" , "psk2");
			}	
		}
		else if(!strcmp("WPA+2" , p->type) && strcmp("psk psk2" , value))
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_akm" , "psk psk2");
			}
			else
			{
				_SET_VALUE("wl0.1_akm" , "psk psk2");
			}	
		}
		else
		{
			return ALINKGW_OK ;
		}
		
	}
	else
	{

		if(strcmp("" , value))
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_akm" , "");
			}
			else
			{
				_SET_VALUE("wl0.1_akm" , "");
			}
		}
		else
		{
			return ALINKGW_OK ;
		}
	}
	
	return ALINKGW_UPDATE;
	
}

static int wlansecurity_set_encryption(struct wifi_security * p , int mode )
{
	char *value = NULL ;

	if(!mode)//ap
	{
		_GET_VALUE("wl0_crypto", value);
	}
	else
	{
		_GET_VALUE("wl0.1_crypto", value);
	}


	if(!strcmp("0" , p->enable))
	{

		if(strcmp("" , value))
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_crypto" , "");
			}
			else
			{
				_SET_VALUE("wl0.1_crypto" , "");
			}	
		}
		else
		{
			return ALINKGW_OK ;
		}
		
	}
	else
	{
		if(!strcmp("AES" , p->encryption) && strcmp("aes" , value))
		{
		
			if(!mode)//ap
			{
				_SET_VALUE("wl0_crypto" , "aes");
			}
			else
			{
				_SET_VALUE("wl0.1_crypto" , "aes");
			}	
		}
		else if(!strcmp("TKIP" , p->encryption) && strcmp("tkip" , value))
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_crypto" , "tkip");
			}
			else
			{
				_SET_VALUE("wl0.1_crypto" , "tkip");
			}	
		
		}
		else if(!strcmp("AES+TKIP" , p->encryption) && strcmp("tkip+aes" , value))
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_crypto" , "tkip+aes");
			}
			else
			{
				_SET_VALUE("wl0.1_crypto" , "tkip+aes");
			}	
		}
		else
		{
			return ALINKGW_OK ;
		}

	}

	return ALINKGW_UPDATE;

}

static int wlansecurity_set_passhrase(struct wifi_security * p , int mode )
{
	char *value = NULL ;

	if(!mode)//ap
	{
		_GET_VALUE("wl0_wpa_psk" , value);
	}
	else
	{
		_GET_VALUE("wl0.1_wpa_psk" , value);
	}	

	if(!strcmp("0" , p->enable))
	{

		if(strcmp("" , value))
		{
			if(!mode)//ap
			{
				_SET_VALUE("wl0_wpa_psk" , "");
			}
			else
			{
				_SET_VALUE("wl0.1_wpa_psk" , "");
			}	
		}
		else
		{
			return ALINKGW_OK ;
		}
	}
	else
	{
		if(strcmp(value , p->passhrase))
		{
		
			if(!mode)//ap
			{
				_SET_VALUE("wl0_wpa_psk" , p->passhrase);
			}
			else
			{
				_SET_VALUE("wl0.1_wpa_psk" , p->passhrase);
			}	
		
		}
		else
		{
			return ALINKGW_OK ;
		}
	}

	return ALINKGW_UPDATE;
}


static int (*wifi_sec_set[])(struct wifi_security * p , int mode)=
{
	wlansecurity_set_enable,
	wlansecurity_set_type,
	wlansecurity_set_encryption,
	wlansecurity_set_passhrase
} ;


int set_wlansecurity24g(const char *json_in )
{
	int i ;
	int wl_mode = -1 ;
	int result = 0 ;
	int change_nvram_flag = 0 ;
	struct wifi_security set_sec_info24 ;

	memset(&set_sec_info24 , 0x0 , sizeof(set_sec_info24));

	wl_mode = get_wl_mode();

	if(wl_mode != 0 && wl_mode != 1)
	{
		AL_SHOW("#########get_wl_mode err\n");
		return ALINKGW_ERR ;
	}
	
	if(prase_wlansec24g_json(json_in , &set_sec_info24))
	{
		AL_SHOW("#########prase_wlansec24g_json err\n");
		
		return ALINKGW_ERR ;
	}

	for(i = 0 ; i < sizeof(wifi_sec_set)/sizeof(wifi_sec_set[0]) ; i++)
	{
		result = wifi_sec_set[i](&set_sec_info24 , wl_mode) ;
		
		if(result == ALINKGW_UPDATE)
		{
			if(change_nvram_flag == 0)
			{
				change_nvram_flag = 1 ;
			}
		}
		else if(result == ALINKGW_OK)
		{
			printf("tai_wifi_security.c %d is not set\n" , i);
		}
		else
		{
			return ALINKGW_ERR ;
		}
		
	}

	if(change_nvram_flag)
	{
		change_nvram_flag = 0 ;
		
		_COMMIT();

		cyg_thread_delay(100);
		
		_RESTART();
		
	}
	
	return ALINKGW_OK ;
}




