/* =================================================================
 *
 *      asp.c
 *
 *      Handles the CGI requests via asp
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####                                     
 * -------------------------------------------                       
 * This file is part of eCos, the Embedded Configurable Operating System.
 * Copyright (C) 2005 Free Software Foundation, Inc.                 
 *
 * eCos is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 or (at your option) any later
 * version.                                                          
 *
 * eCos is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.                                                 
 *
 * You should have received a copy of the GNU General Public License 
 * along with eCos; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.     
 *
 * As a special exception, if other files instantiate templates or use
 * macros or inline functions from this file, or you compile this file
 * and link it with other works to produce a work based on this file,
 * this file does not by itself cause the resulting work to be covered by
 * the GNU General Public License. However the source code for this file
 * must still be made available in accordance with section (3) of the GNU
 * General Public License v2.                                        
 *
 * This exception does not invalidate any other reasons why a work based
 * on this file might be covered by the GNU General Public License.  
 * -------------------------------------------                       
 * ####ECOSGPLCOPYRIGHTEND####                                       
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    hf_shi
 *  Contributors:
 *  Date:         2012-05-30
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "athttpd.h"
#include "asp.h"
#include "asp_form.h"
#include "apmib.h"

static char g_tmp_val[MAX_QUERY_TEMP_VAL_SIZE];

/*root_temp as head*/
temp_mem_t root_temp;
unsigned char last_host[4];

/*
  * addTempStr 
  * tempStr used in get_cstream_var
  */

int addTempStr(char *str)
{
	temp_mem_t *temp,*newtemp;
	temp = root_temp.next;
	newtemp = (temp_mem_t *)malloc(sizeof(temp_mem_t));
	if (newtemp==NULL)
		return FAILED;
	newtemp->str = str;
	newtemp->next = temp;
	root_temp.next = newtemp;	
	return SUCCESS;
}

/*
  * freeAllTempStr
  */
void freeAllTempStr(void)
{
	temp_mem_t *temp,*ntemp;
	
	temp = root_temp.next;
	root_temp.next = NULL;
	while (temp) {
		ntemp = temp->next;
		free(temp->str);
		free(temp);
		temp = ntemp;
	}
}


/*
  * get parameter value
  */
int getcgiparam(char *dst,char *query_string,char *param,int maxlen)
{
	int len,plen;
	int y;
	char *end_str;
	
	end_str = query_string + strlen(query_string);
	plen = strlen(param);
	while (*query_string && query_string <= end_str) {
		len = strlen(query_string);
		if ((len=strlen(query_string)) > plen) {
			if (!strncmp(query_string,param,plen)) {
				if (query_string[plen] == '=') { //copy parameter
					query_string += plen+1;
					y = 0;
					while ((*query_string)&&(*query_string!='&')) {
						if ((*query_string=='%') && (strlen(query_string)>2)) {
							if ((isxdigit(query_string[1])) && (isxdigit(query_string[2]))) {
								if (y<maxlen) {
									dst[y++] = ((toupper(query_string[1])>='A'?toupper(query_string[1])-'A'+0xa:toupper(query_string[1])-'0') << 4)
									+ (toupper(query_string[2])>='A'?toupper(query_string[2])-'A'+0xa:toupper(query_string[2])-'0');
								}
								query_string += 3;
								continue;
							}
						}
						if (*query_string=='+') {
							if (y < maxlen)
								dst[y++] = ' ';
							query_string++;
							continue;
						}
						if (y<maxlen)
							dst[y++] = *query_string;
						query_string++;
					}
					if (y<maxlen)
						dst[y] = 0;
					return y;
				}
			}
		}
		while ((*query_string)&&(*query_string!='&'))
			query_string++;
		if(!(*query_string))
			break;
		query_string++;
	}
	if (maxlen)
		dst[0] = 0;
	return -1;
}

/*
  * get_cstream_var 
  * return as string
  * assume only used in POST , is this right ?
  */
char *get_cstream_var(char *postData, int len, char *var, char *defaultGetValue)
{	
	int ret;
	char *buf;
	ret=getcgiparam(g_tmp_val,postData,var,MAX_QUERY_TEMP_VAL_SIZE);
	if(ret < 0)
		return (char *) defaultGetValue;
	buf = (char *)malloc(ret+1);
	memcpy(buf, g_tmp_val, ret);
	buf[ret] = 0;
	addTempStr(buf);	
	return (char *)buf; //this buffer will be free by freeAllTempStr().
}

/*
  * get_mime_var
  * return as string
  * assume only used in POST, is this right ?
  */
char *get_mime_var(char *buf, int len, char *var, char *defaultGetValue)
{
	return NULL;
}

/*
  * Form Handle
  */
void handleForm(char *formname,char *post_data, int len)
{
	int i;
	if (formname==NULL  || post_data == NULL) {
		/*Sent Not Found ??*/
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
		diag_printf("%s %d formname(%s)\n",__FUNCTION__,__LINE__,formname);
#endif	
		return;
	}
	else {
		form_name_t *now_form;

		for (i=0; root_form[i].name!=NULL; i++) {
			now_form = &root_form[i];
			if ((strlen(formname) == strlen(now_form->name)) &&
				(memcmp(formname,now_form->name,strlen(now_form->name))==0)) {
#ifdef CSRF_SECURITY_PATCH
#if defined(HTTP_FILE_SERVER_SUPPORTED)
				if(strcmp(now_form->name,"formusbdisk_uploadfile")==0)
				{
					log_boaform(now_form->name);	
				}
#endif
				if ( !is_any_log() || !is_valid_boaform(now_form->name)) {
					cyg_httpd_send_error(CYG_HTTPD_STATUS_FORBIDDEN);
					return;					
				}
				delete_boaform(now_form->name);				
#endif
				now_form->function(post_data,len);
				freeAllTempStr();
				return;
			}
		}
		diag_printf("WARNING: %s not registered\n",formname);
	}
	/*Sent Not Found ??*/
}

/*
  * hadle ASP script
  */
void handleScript(char *left1,char *right1)
{
	char *left=left1,*right=right1;
	asp_name_t *now_asp;
	unsigned int funcNameLength;
	int i, j;
	
	left += 2;
	right -= 1;
	while (1) {
		while (*left==' ') {if(left>=right) break;++left;}
		while (*left==';') {if(left>=right) break;++left;}
		while (*left=='(') {if(left>=right) break;++left;}
		while (*left==')') {if(left>=right) break;++left;}
		while (*left==',') {if(left>=right) break;++left;}
		if (left >= right)
			break;

		/* count the function name length */
		{
			char *ptr = left;

			funcNameLength = 0;
			while (*ptr!='(' && *ptr!=' ') {
				ptr++;
				funcNameLength++;
				if ((unsigned int )ptr >= (unsigned int)right) {
					break;
				}
			}
		}

		for (j=0; root_asp[j].name!=NULL; j++) {
			now_asp = &root_asp[j];
			if ((strlen(now_asp->name) == funcNameLength) &&
				(memcmp(left,now_asp->name,strlen(now_asp->name))==0)) {
				char *leftc,*rightc;
				int argc=0;
				char *argv[8]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
				
				left += strlen(now_asp->name);
				while (1) {
					int size,exit=0;
					while (1) {
						if (*left==')') {
							exit=1;
							break;
						}
						if (*left=='\"')
							break;
						if ((unsigned int)left > (unsigned int)right) {
							exit=1;
							break;
						}
						left++; 
					}
					
					if (exit==1)
						break;					
					leftc = left;
					leftc++;
					rightc = strstr(leftc,"\"");
					if (rightc==NULL)
						break;
					size = (unsigned int)rightc-(unsigned int)leftc+1;
					argv[argc] = (char *)malloc(size);
					if (argv[argc]==NULL)
						break;
					memcpy(argv[argc],leftc,size-1);
					argv[argc][size-1] = '\0';
					argc++;
					left = rightc + 1;
				}
				//fprintf(stderr, "###%s:%d asp=%s name=%s ###\n", __FILE__, __LINE__, now_asp->name,argv[0]);
				//sleep(1);
				now_asp->function(argc,argv);
				for (i=0; i<argc; i++)
					free(argv[i]);
				break;
			}
		}
		++left;
	}
}

void cyg_httpd_handle_redirect_asp(char* filename,char* inbuffer)
{
	char * ptr=strstr(inbuffer,"redirect-url=");
	char * redirecturl= ptr + strlen("redirect-url=");

	char * strIdx=NULL;
	int i=0,j=0;
	int index=0;
	//printf("%s:%d filename=%s \n",__FILE__,__LINE__,filename);

	j=strstr(filename,REDIRECT_FORM_NAME)-filename;
	while(redirecturl[i] && redirecturl[i]!='&' && redirecturl[i]!=' ')
	{
		filename[j++]=redirecturl[i++];		
	}
	filename[j]='\0';
	//printf("%s:%d filename=%s redirecturl=%s\n",__FILE__,__LINE__,filename,redirecturl);
	if(ptr= strstr(inbuffer,"wlan_id="))
	{
		strIdx= ptr+strlen("wlan_id=");
		index=strIdx[0]-'0';
		apmib_set_wlanidx(index);
		/*handle WLAN_IF here*/
		snprintf(WLAN_IF,sizeof(WLAN_IF),"%s%d",DEF_WLAN_PREFIX,index);
	}else if(ptr= strstr(inbuffer,"wan_id="))
	{
		strIdx= ptr+strlen("wan_id=");
		index=strIdx[0]-'0';
		apmib_set_wanidx(index);
	}
	
}

/*
  * asp_handle_GET
  */
void asp_handle_GET(char *filename,char* inbuffer)
{
	if(strstr(filename,REDIRECT_FORM_NAME))
	{//redirect file
		cyg_httpd_handle_redirect_asp(filename,inbuffer);
	}
	
	//printf("%s:%d filename=%s wanidx=%d wlanidx=%d\n",__FUNCTION__,__LINE__,
	//filename,apmib_get_wanidx(),apmib_get_wlanidx());
	{//get htm file
		cyg_httpd_send_file_asp(filename);
	}
	return;
}

/*
  * asp_handle_POST
  */
void asp_handle_POST(char *filename,char *post_data, int len)
{
	char formname[MAX_PATH_LEN];
	char *extension,*name;
	memset(formname,0,MAX_PATH_LEN);
	extension=rindex(filename,'.');
	name=rindex(filename,'/');
	strncpy(formname,name+1,((unsigned int)extension-(unsigned int)name-1));
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
	diag_printf("%s %d formname(%s)\n",__FUNCTION__,__LINE__,formname);
#endif
	handleForm(formname,post_data,len);
	return;
}
/*
*  save last ip for redirect
*/
void asp_save_last_ip( cyg_int32 *host)
{
	last_host[0]=host[0];
	last_host[1]=host[1];
	last_host[2]=host[2];
	last_host[3]=host[3];
	return;
}

/*
  * asp_handle
  */
void asp_handle(char* filename)
{
	return;
}

/*
  * Init Function
  */
void asp_init(int argc,char **argv)
{
	memset((void *)&root_temp,0,sizeof(root_temp));
}

