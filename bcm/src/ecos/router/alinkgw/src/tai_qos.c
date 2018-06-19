/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_qos.c
	Description:Wifi api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.7.9
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang      2015.7.9   	1.0          new
************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include "route_cfg.h"
#include "alinkgw_api.h"
#include "tai.h"
#include "cJSON.h"
#include "flash_cgi.h"
#include "tc.h"


#define MIN_QOS_LEN		32

#define QOS_STR			"{\"enabled\":\"%d\" , \"mac\":\"%s\"}"

#define		THRESHOLD_FACTOR  	7/10

#define		PART_FACTOR			6/10

int al_qos_priority = 0 ;

static unsigned long max_bandwidth = 0 ;

static unsigned long max_signmac_bandwidth = 0 ;

static int previous_client_num = 0 ;

static int currect_client_num = 0 ;

static int globle_ipindex = 0 ;

static int first_spit_flag = 0 ;


char qos_sign_mac[ARRAY_LEN_20] = "00:00:00:00:00:00" ;

extern 	in_addr_t lookip(int *mac) ;

extern 	unsigned long get_total_down_bandwidth();

extern 	statistic_list_ip_index_t stream__ip_iist[STREAM_CLIENT_NUMBER];

extern 	statistic_ip_index_t stream_ip[STREAM_CLIENT_NUMBER];

extern 	tc_ip_index_t ip_index[STREAM_CLIENT_NUMBER] ;

extern 	unsigned long g_wan_rate ;

extern  unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];  //0:up, 1: down;

struct al_qos
{
	unsigned int expect_up ;
	unsigned int expect_down ;
	unsigned int real_down ;
	unsigned int real_up ;
};

struct al_qos al_qos_stream[TC_CLIENT_NUMBER] ;

int start_al_qos_flag = 0 ;


int get_al_qos_flag()
{
	return al_qos_priority ;
}

int set_al_qos_enable()
{

	memset(&al_qos_stream , 0 , sizeof(al_qos_stream));

	al_qos_priority = 1 ;

	return ALINKGW_OK ;
}

int set_al_qos_disable()
{
	al_qos_priority = 0 ;

	start_al_qos_flag = 0 ;

	first_spit_flag = 0 ;

	strncpy(qos_sign_mac , "00:00:00:00:00:00" , ARRAY_LEN_20 );

	return ALINKGW_OK ;
}


struct qosset_status
{
	char enabled[ARRAY_LEN_4] ;
	char mac[ARRAY_LEN_20] ;
} ;

struct prase_qos 
{
	char name[ARRAY_LEN_8] ;
	int (*f)(char* var , struct qosset_status *p) ;
} ;


int get_qos_speedup(char *buff, unsigned int buff_sz)
{

	if(!buff)
	{
		return ALINKGW_ERR;	
	}

	if(buff_sz <= MIN_QOS_LEN)
	{
		return ALINKGW_BUFFER_INSUFFICENT ;
	}

	snprintf(buff , buff_sz , QOS_STR , al_qos_priority , qos_sign_mac );

	AL_2_DEBUG("[buff:%s]\n" , buff) ;

	return ALINKGW_OK ;
	
}


int varify_qos_enable(char * var , struct qosset_status *p)
{

	if(strcmp("0" , var) && strcmp("1" , var))
	{
		return ALINKGW_ERR ;
	}

	strncpy(p->enabled , var , ARRAY_LEN_4) ;

	return ALINKGW_OK ;
}

int varify_qos_mac(char * var , struct qosset_status *p)
{
	AL_2_DEBUG("[var:%s]\n" , var);
	
	int i ;
	if(strlen(var) != 17)
	{
		AL_SHOW("strlen(var) != 17\n");
		
		return ALINKGW_ERR ;
	}

	//¹ã²¥ºÍ×é²¥
	//if((*(var + 1))& 1 ==  1)
	if( strtol((var + 1) , NULL , 16)& 1 ==  1)
	{
		AL_SHOW("the mac is broadcast\n");
	
		return ALINKGW_ERR ;
	}
	
	for(i = 0 ; i < strlen(var) ; i++)
	{
		if((i+1)%3 == 0)
		{
			if(*(var + i) != ':')
			{
				AL_2_DEBUG("[i:%d]\n" , i);
				
				return ALINKGW_ERR ;
			}
		}
		else
		{
			if(!isxdigit(*(var + i)))
			{
				AL_2_DEBUG("[i:%d]is not xdigit \n" , i);
			
				return ALINKGW_ERR ;
			}
		}
	}

	strncpy(p->mac , var , ARRAY_LEN_20);
	
	strncpy(qos_sign_mac , var , ARRAY_LEN_20);

	return ALINKGW_OK ;
}

struct prase_qos qos_prase_array[]  =
{
	{"enabled" 		, 	varify_qos_enable 	} ,
	{"mac"			,	varify_qos_mac 		} ,
};

int prase_al_qosseting(const char *json_in , struct qosset_status * p )
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
			printf("cJSON_Parse error.\n");
			return ALINKGW_ERR;
		}
	
		for(i = 0 ; i < sizeof(qos_prase_array)/sizeof(qos_prase_array[0]) ; i++)
		{
			pSub = cJSON_GetObjectItem(pJson, qos_prase_array[i].name);
			if(!pSub || pSub->valuestring == NULL)
			{
				printf("%s NULL \n" , qos_prase_array[i].name);
				goto ERR;
			}
			
			if(qos_prase_array[i].f(pSub->valuestring , p))
			{
				printf("%s errer \n" , qos_prase_array[i].name);
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


int varify_al_qospriority(struct qosset_status *p)
{

	if(atoi(p->enabled) && strcmp("00:00:00:00:00:00" , p->mac))
	{
		if(!get_al_qos_flag())
		{
			set_al_qos_enable();

			return ALINKGW_OK ;
		}
	}
	else
	{
		set_al_qos_disable();
		
		return ALINKGW_OK ;

	}
		
	return ALINKGW_ERR ;
}

int remove_mac_forbid(char * mac_str)
{
	int i = 0 ;

	for(i = 0 ; i < STREAM_CLIENT_NUMBER; i++ )
	{
		if(stream__ip_iist[i].index == NULL)
		{
			continue;
		}

		if(stream__ip_iist[i].index->set_pers == 0)
		{
			if(!strcmp(mac_str , stream__ip_iist[i].index->mac))
			{
				del("filter_mac" , mac_str);
				
				return ALINKGW_OK ;
			}
		}
	}
	return ALINKGW_OK ;
}


unsigned long get_max_bandwidth()
{
	return g_wan_rate/8 ;
}


int get_stream_statistic_speed_qos(int ipindex , u_long * signup_speed , u_long * signdown_speed , u_long * up_speed , u_long * down_speed)
{
	char pre_ip[32]={'\0'}, pr_ip[32]={'\0'};
	char *lanip, *p;
	unsigned int index;
	unsigned char *mac_look_list;
	
	_GET_VALUE(_LAN0_IP, lanip);
	strcpy(pr_ip, lanip);
	p=rindex(pr_ip, '.');
	if(p) *p='\0';

	u_long   up_byte_pers = 0;
	u_long   down_byte_pers = 0;

	AL_2_DEBUG("[ipindex:%d]\n" , ipindex);

	for(index=0; index< STREAM_CLIENT_NUMBER; ++index)
	{		
		sprintf(pre_ip, "%s.%d", pr_ip, index+1);
		mac_look_list = lookmac(inet_addr(pre_ip));
		if( NULL == mac_look_list)
			continue;

		if(ismulticast(mac_look_list))
			continue;

		if( index == ipindex && ipindex != -1)
		{
			*signup_speed   = ip_dbs(index);
			*signdown_speed = ip_dbs(index);						
		}
		
		up_byte_pers += ip_ubs(index);
		down_byte_pers += ip_dbs(index);

	}
	(*up_speed) = up_byte_pers;
	(*down_speed) = down_byte_pers;
AL_2_DEBUG("[down_speed:%ld]\n" , *down_speed);
	return ALINKGW_OK ;
}


int get_current_clients_num(int num)
{
	currect_client_num = num ;

	return ALINKGW_OK ;
}


int split_other_bandwidth(unsigned long currect_bandwidth , int num , int ipindex)
{
	int i ;
	AL_2_DEBUG("[ipindex:%d] , currect_bandwidth/num:%ld\n" , ipindex , currect_bandwidth/num );
	for(i = 0 ; i < STREAM_CLIENT_NUMBER ; i++)
	{
	
		if(i !=  ipindex)
		{
			al_qos_stream[i].expect_down= currect_bandwidth/num/5;
		}
		else
		{
			al_qos_stream[i].expect_down = 300 *1000*1000;
		}
	}
	
	start_al_qos_flag = 1 ;
	
	return ALINKGW_OK ;

}


int set_increase_signmac_bandwidth()
{

	AL_SHOW("set_increase_signmac_bandwidth");

	if(max_bandwidth != max_signmac_bandwidth)
	{
		max_signmac_bandwidth =  max_signmac_bandwidth + max_signmac_bandwidth * 2 / 10 ;
	}
	else
	{
		max_signmac_bandwidth = max_bandwidth * 6 / 10 ;
	}

	if( max_signmac_bandwidth <= max_bandwidth * 9 /10)
	{
		split_other_bandwidth(max_signmac_bandwidth , currect_client_num , globle_ipindex);
	}
	
	return ALINKGW_OK ;
}

int set_reduce_signmac_bandwidth()
{
	AL_SHOW("set_reduce_signmac_bandwidth");

	max_signmac_bandwidth = max_signmac_bandwidth - max_signmac_bandwidth * 2 / 10 ;

	if( max_signmac_bandwidth >= max_bandwidth * 2 /10)
	{
		split_other_bandwidth(max_signmac_bandwidth , currect_client_num , globle_ipindex);
	}
	return ALINKGW_OK ;
}


int get_mac_relevent_ipindex(char * mac_str)
{

	int index = 0 ;

	int mactoip[6]={0};

	sscanf(mac_str , "%02x:%02x:%02x:%02x:%02x:%02x" , mactoip,mactoip+1,mactoip+2,mactoip+3,mactoip+4,mactoip+5);

	index = (ntohl(lookip(mactoip))&0xff) -1 ;

	if(index < 0 || index >= TC_CLIENT_NUMBER )
	{
		return ALINKGW_ERR ;
	}

	return index ;
}

int check_ipmac_and_clientnum(char* mac_str)
{

	int local_ip_index = 0 ;

	local_ip_index =  get_mac_relevent_ipindex(mac_str) ;

	if(local_ip_index < 0)
	{
		return ALINKGW_ERR ;
	}

	if(local_ip_index != globle_ipindex || previous_client_num != currect_client_num)
	{

		globle_ipindex = local_ip_index ;
		
		previous_client_num = currect_client_num ;

		if(globle_ipindex >  0 && globle_ipindex < 255 && currect_client_num > 1)
		{
			split_other_bandwidth(max_signmac_bandwidth , currect_client_num , globle_ipindex);
		}
	}
		
	
	return ALINKGW_OK ;
	
}

int handle_al_qos_priority(char * mac_str)
{
		
	unsigned long current_down_speed = 0 ;

	unsigned long current_up_speed = 0 ;

	unsigned long sign_down_speed = 0 ;

	unsigned long sign_up_speed = 0 ;

	int temp_count = 0 ;

	previous_client_num = currect_client_num ;

	if(currect_client_num == 1)
	{
		return ALINKGW_OK ;
	}
	
	check_ipmac_and_clientnum(mac_str) ;

	get_stream_statistic_speed_qos( globle_ipindex , &sign_up_speed , &sign_down_speed , &current_up_speed , &current_down_speed) ;

	AL_2_DEBUG("[sign_down_speed:%ld][max_signmac_bandwidth * THRESHOLD_FACTOR:%ld][current_down_speed:%ld][max_signmac_bandwidth * THRESHOLD_FACTOR:%ld]\n" , sign_down_speed ,max_signmac_bandwidth * THRESHOLD_FACTOR, current_down_speed ,  max_signmac_bandwidth * 5/10);

	if(sign_down_speed <= 0 )
	{
		return ALINKGW_OK ;
	}

	if(!first_spit_flag)
	{
		if( current_down_speed >= max_bandwidth * 5/10)
		{
			set_increase_signmac_bandwidth();
			
			first_spit_flag = 1;

		}
	}
	
	if(sign_down_speed >= max_signmac_bandwidth * THRESHOLD_FACTOR)
	{
		set_increase_signmac_bandwidth() ;
		
		temp_count = 0 ;
	}
	else if(current_down_speed - sign_down_speed >=  ( max_bandwidth - max_signmac_bandwidth ) * THRESHOLD_FACTOR)
	{
		temp_count ++ ;
	}

	if(temp_count >= 5)
	{
		set_reduce_signmac_bandwidth() ;

		temp_count = 0 ;
	}
	
	return ALINKGW_OK ;
}


int start_qos_priority = 0 ;

int set_qos_speedup(const char *json_in)
{

	struct qosset_status qos_set_info;
	
	AL_2_DEBUG("[json_in:%s]\n" , json_in);

	if(!json_in)
	{
		return ALINKGW_ERR ;
	}

	memset(&qos_set_info , 0x0 , sizeof(qos_set_info));

	if(prase_al_qosseting(json_in , &qos_set_info))
	{
		return ALINKGW_ERR ;
	}
	
	if(varify_al_qospriority(&qos_set_info))
	{
		return ALINKGW_ERR ;
	}
	
	AL_2_DEBUG("al_qos_priority :%d \n" , al_qos_priority);
	
	if(get_al_qos_flag())
	{
		printf("set_al_qos_enable\n");

		start_al_qos_flag = 0  ;
		
		max_bandwidth = get_max_bandwidth();

		AL_2_DEBUG("[max_bandwidth:%ld][g_wan_rate:%ld] \n" , max_bandwidth , g_wan_rate);
		
		if(max_bandwidth <= 0)
		{
			return ALINKGW_ERR ;
		}
		
		max_signmac_bandwidth = max_bandwidth ;
		
		set_al_qos_enable() ;
	}

	return ALINKGW_OK ;
	
}

