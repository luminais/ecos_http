/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_clientlist.c
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

#define STACK_SIZE 							65535
#define MAX_CLIENT_NUMBER 					255

#define AL_ATTRIBUTE_LEN_8					8
#define AL_ATTRIBUTE_LEN_20					20
#define AL_ATTRIBUTE_LEN_64					64

#define DEV_KEY_MAC_LEN_8					8
#define DEV_KEY_MAC_LEN_12				12

#define CLI_ATTRIBUTE_UNKONW				"unknown"
#define CLI_TYPE_IPHONE						"iPhone"
#define CLI_TYPE_IPAD						"iPad"
#define CLI_TYPE_ANDROID					"Android"
#define CLI_TYPE_PC							"PC"

#define  UPPER_IPHONE						"IPHONE"
#define  UPPER_IPAD							"IPAD"
#define  UPPER_ANDROID						"ANDROID"
#define  UPPER_PC							"PC"					

struct al_client
{
	int client_num ;
	PTPI_ALL_CLIENTS pclient ;
};

struct al_attribute
{
	char name[AL_ATTRIBUTE_LEN_64] ;
	char type[AL_ATTRIBUTE_LEN_8] ;
	char category[AL_ATTRIBUTE_LEN_8];
	char manufacturer[AL_ATTRIBUTE_LEN_8];
	char mac[AL_ATTRIBUTE_LEN_20];
};

static struct al_client clients_list1  = {0 , {NULL}};
static struct al_client clients_list2  = {0 , {NULL}};


extern int tpi_get_all_clients(OUT PTPI_ALL_CLIENTS clients_list, IN int max_client_num);

extern unsigned int bkdrhash(const char *str) ;

extern int get_current_clients_num(int num) ;

char tai_toupper(char c)
{
    if ((c >= 'a') && (c <= 'z')) c -= 32;
    return c;
}

static int find_client_name(const PTPI_ALL_CLIENTS client_node ,  struct al_attribute * client_attribute )
{
	if(0 != strcmp("" , client_node->hostname ))
	{
		strncpy(client_attribute->name , client_node->hostname , sizeof(client_attribute->name)) ;
	}
	else
	{
		strncpy(client_attribute->name , CLI_ATTRIBUTE_UNKONW , sizeof(client_attribute->name) ) ;
	}
	
	return 0 ;
}

static int find_client_type(const PTPI_ALL_CLIENTS client_node ,  struct al_attribute * client_attribute )
{
	int i = 0 ;
	char hostname_upstr[TPI_BUFLEN_64] = { 0 } ;
	for(i = 0 ;  i < strlen(client_node->hostname) ; i++)
	{
		hostname_upstr[i] = tai_toupper(client_node->hostname[i]) ;
	}
	hostname_upstr[i] = '\0' ;
	
	if(0 != strcmp("" , client_node->hostname ))
	{
		if(strstr(hostname_upstr , UPPER_ANDROID) != NULL)
		{
			strncpy(client_attribute->type , CLI_TYPE_ANDROID , sizeof(client_attribute->type)) ;

			return 0 ;
		}
		else if(strstr(hostname_upstr , UPPER_IPHONE) != NULL)
		{
			strncpy(client_attribute->type , CLI_TYPE_IPHONE , sizeof(client_attribute->type)) ;
			
			return 0 ;
		}
		else if(strstr(hostname_upstr , UPPER_IPAD) != NULL)
		{
			strncpy(client_attribute->type , CLI_TYPE_IPAD , sizeof(client_attribute->type)) ;

			return 0 ;
		}
		else if(strstr(hostname_upstr , UPPER_PC) != NULL)
		{
			strncpy(client_attribute->type , CLI_TYPE_PC , sizeof(client_attribute->type)) ;

			return 0 ;
		}
	}

	if(strcmp(client_attribute->type , CLI_TYPE_IPHONE))
	{
		strncpy(client_attribute->type , CLI_ATTRIBUTE_UNKONW , sizeof(client_attribute->type)) ;
	}

	return 0 ;
}

static int find_client_manufacturer(const PTPI_ALL_CLIENTS client_node ,  struct al_attribute * client_attribute )
{
	int i = 0 ;
	int key_index = 0 ;
	
	struct man_hash *manu = NULL ;

	unsigned char key_mac[DEV_KEY_MAC_LEN_12] = {0} ;

	//if(client_attribute != NULL && client_attribute->mac != NULL)
	{
		//LLDEBUG("client_attribute->mac[%s]\n" , client_attribute->mac ) ;
		
		for(i = 0 ; i <	DEV_KEY_MAC_LEN_8 ;  i++ )
		{
			key_mac[i] = tai_toupper(client_attribute->mac[i]) ;
		}
		key_mac[i] = '\0' ;

		//LLDEBUG("key_mac[%s]\n" , key_mac ) ;

		//hash 查找
		key_index = bkdrhash(key_mac) ;

		//LLDEBUG("key_index[%d]\n" , key_index ) ;
		
		manu = (struct man_hash *)manutbl[key_index].first ;

		while(manu != NULL)
		{		
			//LLDEBUG("manu->info->oui[%s]\n" , manu->info->oui ) ;
			if(memcmp(manu->info->oui, key_mac, 8) == 0)
			{			
				strncpy(client_attribute->manufacturer , manu->info->manufacture , sizeof(client_attribute->manufacturer));

				if(!manu->info->type)
				{
					strncpy(client_attribute->type , CLI_TYPE_IPHONE , sizeof(client_attribute->type)) ;
				}
				else
				{
					strncpy(client_attribute->type , CLI_ATTRIBUTE_UNKONW , sizeof(client_attribute->type)) ;
				}

				return  0 ;
			}
			
			manu = (struct man_hash *)manu->node.next;		
		}
		
	}

	strncpy(client_attribute->manufacturer , CLI_ATTRIBUTE_UNKONW , sizeof(client_attribute->manufacturer));
	
	return -1 ;
	
}

static int find_client_attribute(const PTPI_ALL_CLIENTS client_node ,  struct al_attribute * client_attribute )
{

	SHOWDEBUG("come in find_client_attribute");

	strncpy(client_attribute->category, CLI_ATTRIBUTE_UNKONW , strlen(CLI_ATTRIBUTE_UNKONW) ) ;
	strncpy(client_attribute->mac , client_node->mac , sizeof(client_attribute->mac));

	find_client_name(client_node , client_attribute) ;
	find_client_manufacturer(client_node , client_attribute) ;
	find_client_type(client_node , client_attribute) ;

	LLDEBUG("[name:%s][type:%s][category:%s][manufacturer:%s][mac:%s]\n" , client_attribute->name , 
		client_attribute->type , client_attribute->category, client_attribute->manufacturer, client_attribute->mac);
	return 0 ;
}

static int report_join_clients(const PTPI_ALL_CLIENTS client_node)
{
	int ret = 0 ;

	struct al_attribute client_attribute ;

	
	//处理设备名、类型、分类、制造厂商和mac信息
	memset(&client_attribute , 0 , sizeof(client_attribute));
	
	find_client_attribute(client_node , &client_attribute );	
	
	ret = ALINKGW_attach_sub_device(client_attribute.name, client_attribute.type, client_attribute.category, client_attribute.manufacturer, client_attribute.mac) ;
	if(ret != ALINKGW_OK)
	{
		printf("[debug] [%s], attach sub device(%s) failed\n", __func__, client_attribute.mac);
	}
	
	return 0 ;
}

int renew_maintain_list(void)
{
	if(clients_list2.pclient)
	{
		free(clients_list2.pclient) ;
		clients_list2.pclient = NULL ;
		clients_list2.client_num = 0 ;

	}

	clients_list2.pclient = clients_list1.pclient ;
	clients_list1.pclient = NULL ;

	clients_list2.client_num= clients_list1.client_num;
	clients_list1.client_num = 0 ;

	SHOWDEBUG("renew_maintain_list success\n");
	return 0 ;
}

int check_and_report_clientlist_state(void)
{
	int i = 0 ;
	int j = 0 ;
	unsigned char exist_flag[MAX_CLIENT_NUMBER] ;
	memset(&clients_list1 , 0 , sizeof(clients_list1));
	clients_list1.pclient = (PTPI_ALL_CLIENTS)malloc(MAX_CLIENT_NUMBER * sizeof(TPI_ALL_CLIENTS));
	if(clients_list1.pclient == NULL )
	{
		return ALINKGW_ERR;
	}
	memset(clients_list1.pclient, 0x0 , sizeof(TPI_ALL_CLIENTS) * MAX_CLIENT_NUMBER);
	if ((clients_list1.client_num = tpi_get_all_clients(clients_list1.pclient , MAX_CLIENT_NUMBER)) <= 0)
	{
		free(clients_list1.pclient) ;
		return ALINKGW_ERR;
	}

	get_current_clients_num(clients_list1.client_num);//use for qos priority

	//比较两个表
	if(clients_list2.pclient)
	{
		memset(exist_flag , 0 , MAX_CLIENT_NUMBER) ;
		for(i = 0 ; i < clients_list1.client_num ; i++)
		{
			for(j = 0 ; j < clients_list2.client_num ; j++)
			{
				if(strncmp((&clients_list1.pclient[i])->mac , (&clients_list2.pclient[j])->mac , TPI_MAC_STRING_LEN) == 0)
				{
					exist_flag[j] = 1 ;
					break ;
				}
			}

			if(j == clients_list2.client_num )
			{
				LLDEBUG("[i:%d][(&clients_list1.pclient[i])->mac:%s]\n" , i , (&clients_list1.pclient[i])->mac);
				report_join_clients(&clients_list1.pclient[i]) ;
			}

		}

		for(j = 0 ; j < clients_list2.client_num ; j++ )
		{
			if(exist_flag[j] == 0)
			{
				LLDEBUG("[i:%d][(&clients_list2.pclient[i])->mac:%s]\n" , i , (&clients_list2.pclient[j])->mac);
				//标志位为0，上报离线客户端信息
				ALINKGW_detach_sub_device((&clients_list2.pclient[j])->mac);
			}
		}
			
	}
	else
	{
		for(i = 0 ; i < clients_list1.client_num; i++)
		{
			LLDEBUG("[i:%d][(&clients_list1.pclient[i])->mac:%s]\n" , i , (&clients_list1.pclient[i])->mac);
			report_join_clients(&clients_list1.pclient[i]) ;
		}
	}	
	
	renew_maintain_list();

	return ALINKGW_OK ;
}


