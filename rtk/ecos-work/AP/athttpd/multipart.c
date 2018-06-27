/*mutipart handle functions*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#if 0
#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>
#include <cyg/athttpd/handler.h>
#include <cyg/athttpd/forms.h>
#endif

#include "athttpd.h"
#include "asp.h"
#include "fmget.h"
#include "apmib.h"
#include "common.h"
#include "utility.h"
#include "asp_form.h"
#include "multipart.h"

#if defined(ECOS_MEM_CHAIN_UPGRADE)
extern int formupload_upgrade_flag;
#endif


char *memstr(char *membuf, char *param, int memsize)
{
	char charfind;
	char charmem;
	char charfindfisrt;
	char *findpos;
	char *mempos;
	int  mmsz;

	if ((charfindfisrt = *param++) == 0) {
        return membuf;
    }

    while (memsize-- > 0) {
        charmem = *membuf++;
        if (charmem == charfindfisrt) {
            findpos = param;
            mempos = membuf;
            mmsz = memsize;

            while ((charfind = *findpos++) != 0) {
                if (mmsz-- <= 0) {
                    return NULL;
                }
                charmem = *mempos++;

                if (charmem != charfind) {
                    break;
                }
            }

            if (charfind == 0) {
                return (membuf - 1);
            }
        }
    }
    return NULL;
}

/*the return value will skip the founded boundary*/
char *rtl_mime_find_boundary(char *data, int data_len, char *boundary)
{
	char *substr_start;
	int len;

#if defined(ECOS_MEM_CHAIN_UPGRADE)
	if(formupload_upgrade_flag == 1)
		substr_start = mem_chain_upgrade_memstr(data,boundary,data_len);
	else
		substr_start=memstr(data,boundary,data_len);
#else
	substr_start=memstr(data,boundary,data_len);
#endif
	
	if(substr_start==NULL)
		return NULL;
	len=strlen(boundary);
	substr_start+=len;	 
	return (substr_start+2);
}


#if defined(ECOS_MEM_CHAIN_UPGRADE)
int mem_chain_upgrade_rtl_mime_get_boundary(char *data, int len, char *boundary, int boundary_len)
{
	char *substr_start,*ptr;
	char* ptr1,*ptr2;
	char *substr="----------------------------";
	int i=0;
	substr_start=mem_chain_upgrade_memstr(data,substr,len);
	if(substr_start==NULL)
	{
		printf("can't get boundry string\n");
		return -1;
	}
	
	ptr=substr_start;

	ptr1 = mem_chain_upgrade_mem_convert(ptr+i);
	ptr2 = mem_chain_upgrade_mem_convert(ptr+i+1);
	if(ptr1 == NULL || ptr2 == NULL){
		printf("cannot convert address\n");
		return -1;
	}
	
	while( (*ptr1 !=0x0d || *ptr2 !=0x0a) && i<MAX_BOUNDRY_LEN)
	{	
		boundary[i]=*ptr1;
		i++;
		ptr1 = mem_chain_upgrade_mem_convert(ptr+i);
		ptr2 = mem_chain_upgrade_mem_convert(ptr+i+1);
		if(ptr1 == NULL || ptr2 == NULL){
			printf("cannot convert address\n");
			return -1;
		}
		
	}
	if(i>=boundary_len)
	{
		printf("the string of boundry is too long\n");
		return -1;
	}
	
	boundary[i]=0;
	return 0;
}

char *mem_chain_upgrade_rtl_mime_get_name_value(char *data,int data_len,struct mutlipart_entry  *pentry, char* boundary)
{
	char *ptr=NULL;
	char* ptr1,*ptr2,*ptr3,*ptr4;
	int len=0;
	int left=data_len;
	ptr = mem_chain_upgrade_memstr(data,NAME_FORMAT,data_len);

	if(ptr==NULL)
		return NULL;
	
	ptr+=strlen(NAME_FORMAT)+1;
	//pentry->name=ptr;

	left =data_len-((unsigned long)ptr-(unsigned long)data);

	ptr1 = mem_chain_upgrade_mem_convert((char*)ptr+len);
	ptr2 = mem_chain_upgrade_mem_convert((char*)ptr+len+1);
	ptr3 = mem_chain_upgrade_mem_convert((char*)ptr+len+2);
	ptr4 = mem_chain_upgrade_mem_convert((char*)ptr+len+3);
	if(ptr1 == NULL || ptr2 ==  NULL || ptr3 == NULL || ptr4 == NULL){
		printf("cannot convert address\n");
		return NULL;
	}


	while((*ptr1 != 0x0d || *ptr2 != 0x0a || *ptr3 != 0x0d || *ptr4 != 0x0a)
		&& (*ptr1 != ';') && (len < left)){
		len++;
		ptr1 = mem_chain_upgrade_mem_convert((char*)ptr+len);
		ptr2 = mem_chain_upgrade_mem_convert((char*)ptr+len+1);
		ptr3 = mem_chain_upgrade_mem_convert((char*)ptr+len+2);
		ptr4 = mem_chain_upgrade_mem_convert((char*)ptr+len+3);

		if(ptr1 == NULL || ptr2 ==  NULL || ptr3 == NULL || ptr4 == NULL){
			printf("cannot convert address\n");
			return NULL;
		}
	}

	pentry->name_len=(len-1);
	pentry->name = (char*)mem_chain_upgrade_mem_str_convert(ptr,pentry->name_len);

	/*toward value*/
	if(*ptr1 == ';')
	{
		while(*ptr1 != 0x0d || (*ptr2!=0x0a) || (*ptr3!= 0x0d) || (*ptr4!= 0x0a)
				&& (len <left))
		{
			len++;
			ptr1 = mem_chain_upgrade_mem_convert((char*)ptr+len);
			ptr2 = mem_chain_upgrade_mem_convert((char*)ptr+len+1);
			ptr3 = mem_chain_upgrade_mem_convert((char*)ptr+len+2);
			ptr4 = mem_chain_upgrade_mem_convert((char*)ptr+len+3);

			if(ptr1 == NULL || ptr2 ==  NULL || ptr3 == NULL || ptr4 == NULL){
				printf("cannot convert address\n");
				return NULL;
			}
		}	
	}

	/*value start*/
	ptr+=(len+4);
	pentry->value=ptr;

	/*value end*/	
	left =data_len-((unsigned long)ptr-(unsigned long)data);
	ptr=rtl_mime_find_boundary(ptr,left,boundary);
	
	if(NULL == ptr)
		return NULL;
	pentry->value_len=((unsigned long)ptr-(unsigned long)pentry->value)-strlen(boundary)-4;
	return ptr;
}

#endif

/*Get boundary*/
int rtl_mime_get_boundary(char *data, int len, char *boundary, int boundary_len)
{
	char *substr_start,*ptr;
	char *substr="----------------------------";
	int i=0;
	substr_start=memstr(data,substr,len);
	if(substr_start==NULL)
	{
		printf("can't get boundry string\n");
		return -1;
	}
	
	ptr=substr_start;
	while( (ptr[i] !=0x0d || ptr[i+1] !=0x0a) && i<MAX_BOUNDRY_LEN)
	{	
		boundary[i]=ptr[i];
		i++;
	}
	if(i>=boundary_len)
	{
		printf("the string of boundry is too long\n");
		return -1;
	}
	boundary[i]=0;
	return 0;
}
	
char *rtl_mime_get_name_value(char *data,int data_len,struct mutlipart_entry  *pentry, char* boundary)
{
	char *ptr=NULL;
	int len=0;
	int left=data_len;
	ptr=memstr(data,NAME_FORMAT,data_len);
	if(ptr==NULL)
		return NULL;
	
	ptr+=strlen(NAME_FORMAT)+1;
	pentry->name=ptr;	
	left =data_len-((unsigned long)ptr-(unsigned long)data);
	while((ptr[len] != 0x0d || (ptr[len+1]!=0x0a) || (ptr[len+2]!= 0x0d) || (ptr[len+3]!= 0x0a)) 
		&& (ptr[len]!=';')  && (len<left)) {
		len++;
	}
	pentry->name_len=(len-1);

	/*toward value*/
	if(ptr[len] == ';')
	{
		while(ptr[len] != 0x0d || (ptr[len+1]!=0x0a) || (ptr[len+2]!= 0x0d) || (ptr[len+3]!= 0x0a)
				&& (len <left))
		{
			len++;
		}	
	}

	/*value start*/
	ptr+=(len+4);
	pentry->value=ptr;

	/*value end*/	
	left =data_len-((unsigned long)ptr-(unsigned long)data);
	ptr=rtl_mime_find_boundary(ptr,left,boundary);
	if(NULL == ptr)
		return NULL;
	pentry->value_len=((unsigned long)ptr-(unsigned long)pentry->value)-strlen(boundary)-4;
	return ptr;
}

void parse_MIME(MULTIPART_Tp pMIME, char *data, int len)
{
	int count=0;
	int left;
	char *ptr,*ptr2;
	int i = 0;
	char boundary[MAX_BOUNDRY_LEN];
	
	ptr=data;
	left=len;
	pMIME->entry_count=0;
	memset(boundary,0,MAX_BOUNDRY_LEN);

	extern char	contentType[CYG_HTTPD_MAXCONTENTTYPE];
	ptr2 = memstr(contentType,"boundary=",CYG_HTTPD_MAXCONTENTTYPE);
	if(ptr2 ==  NULL)
		printf("no boundry found, Parse Error\n");
	else
		ptr2 += 9;
	
	while(*ptr2 != ';' &&
		(*ptr2 !=0x0d || *(ptr2+1) !=0x0a)){
		if(i < MAX_BOUNDRY_LEN)
			boundary[i++] = *ptr2++;
		else
			printf("the string of boundry is too long\n");		
	}
	boundary[i] = 0;	
	ptr=rtl_mime_find_boundary(ptr,left,boundary);	
	left =len-((unsigned long) ptr-(unsigned long)data);
	
	while(left >0)
	{
#if defined(ECOS_MEM_CHAIN_UPGRADE)
		if(formupload_upgrade_flag == 1){
			ptr=mem_chain_upgrade_rtl_mime_get_name_value(ptr,left,&pMIME->entry[pMIME->entry_count],boundary);
		}
		else
			ptr=rtl_mime_get_name_value(ptr,left,&pMIME->entry[pMIME->entry_count],boundary);
#else
			ptr=rtl_mime_get_name_value(ptr,left,&pMIME->entry[pMIME->entry_count],boundary);
#endif
		
		if(NULL == ptr)
		{
			break;
		}	
		left =len-((unsigned long) ptr-(unsigned long)data);
		pMIME->entry_count++;		
	}
}

void dump_MIME(MULTIPART_Tp pMIME)
{
	int i,j;
	for(i=0;i<pMIME->entry_count;i++)
	{
		printf("name:");
		for(j=0;j<(pMIME->entry[i].name_len > 32 ? 32: pMIME->entry[i].name_len);j++)
			printf("%c",pMIME->entry[i].name[j]);
		printf("\nlen:%d\n",pMIME->entry[i].name_len);

		printf("value:");
		for(j=0;j<(pMIME->entry[i].value_len > 32 ? 32:pMIME->entry[i].value_len);j++)
			printf("%c",pMIME->entry[i].value[j]);
		printf(" \nlen:%d\n",pMIME->entry[i].value_len);
	}
}
/*
  * Note: the value returned is NOT a string.
  */
char *get_MIME_var(MULTIPART_Tp pMIME, char *var, int *value_len,char *default_value)
{
	int i;
	if(0 ==pMIME->entry_count)
		printf("no entry exist\n");
	
	for(i=0;i<pMIME->entry_count;i++)
	{
		if(!strncmp(var,pMIME->entry[i].name,pMIME->entry[i].name_len))
		{
			*value_len=pMIME->entry[i].value_len;
			return pMIME->entry[i].value;
		}
	}
	/*Not found*/
	*value_len=strlen(default_value);
	return default_value;
}

