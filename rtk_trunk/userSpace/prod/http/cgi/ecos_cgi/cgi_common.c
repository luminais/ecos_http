/*************************************************************************
  > Copyright (C) 1998-2016, Tenda Tech. Co., All Rights Reserved.
  > File Name: userSpace/prod/http/cgi/cgi_moddle_layer/cgi_common.c
  > Description: 
  > Author: ZhuHuan
  > Mail: zhuhuan_IT@outlook.com
  > Version: 1.0
  > Created Time: Thursday 2016-07-21 14:57 CST
  > Function List: 

  > History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#include "webs.h"
#include "cgi_common.h"

/* 
 * Function: add_msg_to_list
 * Description: add message to the message list, and restruct those messages in the list
 * Paramater: 
  	msg_list: list that message will be added to
  	msg: message which needs adding to the list
 * Return value: NULL
 * Author: zhuhuan
 * create time: 2016.03.15
 * Version: v1.0
 */
void add_msg_to_list(CGI_MSG_MODULE *msg_list, CGI_MSG_MODULE *msg)
{
	int i;
	char *p_list_position, *p_msg_position;

	p_list_position = p_msg_position = NULL;

	if(NULL == msg_list || NULL == msg)
	{
		printf("fun=%s; line=%d; msg: paramater error!\n", __func__, __LINE__);

		return ;
	}

	for(i = 0; i < MAX_MSG_NUM; ++i)
	{
		/* restruct if this message is existed */
		if(0 != msg_list[i].id)
		{
			/* check the two messages are belong to the same module, continue if not */
			if(msg_list[i].id != msg->id)
			{
				continue ;
			}

			/* check if the two messages are the same */
			if(0 == strcmp(msg_list[i].msg, msg->msg))
			{
				return ;
			}
			continue ;
		}

		/* add it to the list if no more messages in the blacklist */
		msg_list[i].id = msg->id;
		printf("msg->id[%d]   msg->msg[%s]\n",msg->id,msg->msg);
		strncpy(msg_list[i].msg, msg->msg, strlen(msg->msg));
		
		return ;
	}

	return ;
}

void remove_msg_to_list(CGI_MSG_MODULE *msg_list, CGI_MSG_MODULE msg)
{
	int i;
	
	if(NULL == msg_list)
	{
		printf("fun=%s; line=%d; msg: paramater error!\n", __func__, __LINE__);

		return ;
	}

	for(i = 0; i < MAX_MSG_NUM; ++i)
	{
		/* restruct if this message is existed */
		if(msg.id == msg_list[i].id)
		{
			msg_list[i].id = 0;
			memset(msg_list[i].msg,0x0,MAX_MODULE_MSG_MAX_LEN);
		}
	}

	return ;
}

int sscanfArglistConfig(const char *value, const char key , char **argc, int count)
{
	char *arglist_tmp = NULL, *arglist_tmp_head  = NULL, *argc_tmp = NULL;
	char **argc_p ;
	int  number = count;
	int  curr_count = 0;
	int  n = 0 , curr_n = 0, max_len = 0;
	
	if (value == NULL  || argc == NULL ||  count == 0){
		return -1;
	}
	argc_p = argc;

	max_len = strlen(value);
	arglist_tmp =	malloc(max_len+1);
	if(NULL == arglist_tmp)
	{
		return -1;
	}
	memset(arglist_tmp, 0x0 , max_len+1 );
	arglist_tmp_head = arglist_tmp;
	sprintf(arglist_tmp , "%s" , value);
	
	argc_tmp = strchr(arglist_tmp, key );
	while( number > 0){
		if(argc_tmp == NULL){
			*argc_p =	malloc(strlen(arglist_tmp)+1);
			if(*argc_p == NULL ){
				break;
			}
			memset(*argc_p, 0x0 , strlen(arglist_tmp)+1);
			
			strcpy(*argc_p , arglist_tmp );

			curr_count++;
			number--;
			argc_p++;
			break;
		}
		
		*argc_tmp = '\0';
		*argc_p =	malloc(strlen(arglist_tmp)+1);
		if(*argc_p == NULL ){
			break;
		}
		memset(*argc_p, 0x0 , strlen(arglist_tmp)+1 );
		strcpy(*argc_p , arglist_tmp );	

		curr_count++;
		number--;
		argc_p++;
		argc_tmp++;

		arglist_tmp = argc_tmp;

		argc_tmp = strchr(arglist_tmp, key );
	}
	
	free(arglist_tmp_head);
	return curr_count;
	
}

void freeArglistConfig(char **argc, int count)
{
	int current = count ; 
	char **argc_p ;
	
	
	if ( argc == NULL ||  count == 0){
		return ;
	}
	argc_p = argc;
	while( current > 0){
		if(*argc_p != NULL){
			free(*argc_p);
		}
		argc_p++;
		current--;
	}

}
int qosMacToLower(char *mac)
{
	int i = 0;
	int len = 0;
	
	if(NULL == mac)
		return -1;
		
	len = strlen(mac);
	if (len <= 0)
		return -1;
	
	len = (len > 17) ? 17 : len;

	for(i = 0; i < len; ++i)
		mac[i] = isupper(mac[i]) ? (mac[i] - 'A' + 'a') : (mac[i]);
	
	return 0;
}
/*****************************************************************************
 函 数 名  : biz_parse_fmt_mac_to_fmt1_mac
 功能描述  : 去掉mac中的冒号
 修改历史      :
  1.日    期   : 2016年12月20日
    作    者   : lrl
    修改内容   : 新生成函数

*****************************************************************************/

void biz_parse_fmt_mac_to_fmt1_mac(const char *in_mac, char *out_mac, int size)
{
	const char *p = in_mac;
	char *q = out_mac; 
	while ('\0' != *p && q != out_mac+size-1) 
	{       
		if (':' == *p) 
		{            
			p++;            
			continue;
		}        
		*q=*p;        
		p++,q++;
	}    
	*q = '\0';
}

int is_gb2312_code(char * str)
{
	unsigned one_byte = 0X00; //binary 00000000
	int i = 0;
    unsigned char k = 0;
    unsigned char c = 0;
	if(str == NULL)
	{
		return -1;
	}
    for (i=0; (unsigned char)str[i] != '\0' ;)
    {
        c = (unsigned char)str[i];
        if (c>>7 == one_byte) 
		{
            i++;
            continue;
        } 
		else if(c >= 0XA1 && c <= 0XF7)
		{
            k = (unsigned char)str[i+1];
            if (k >= 0XA1 && k <= 0XFE)
			{				
				return 1;
            }       
        }
        i += 2; 
    }
    return 0; 
}

int web_check_addr (char * lan_ip)
{

	struct in_addr check_sa;
	if (lan_ip && inet_aton(lan_ip, &check_sa) && ( strlen(lan_ip) >= 7) )
	{
		return 0;
	}
	else 
	{
		return -1;
	}
}

int web_check_mtu(char *str,int wan_type)
{
	int i = 0;
	int value = 0;
	if(NULL == str)
		return -1;
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] < '0' || str[i] > '9')
		{
			return -1;
		}
	}
	value = atoi(str);

	/*MTU 设置的有效范围 pppoe576~1492*/
	if(WAN_PPPOE_MODE == wan_type)
	{
		if(value < 576 || value > 1492)
			return -1;
	}
	else
	{
		if(value < 576 || value > 1500)
			return -1;
	}
	
	return 0;
}

