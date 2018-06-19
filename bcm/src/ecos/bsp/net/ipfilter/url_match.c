

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>

#include <urlf.h>

#define ARRAY_LEN_1500					1500

#define HASH_TABLE_SIZE 				1024

#define REDIRECT_URL_LEN				 64

#define ARRAY_LEN_512					512

#define SDW_URL_SAVE_PATH 				"/Dir_For_DD/url.xml"

#define OLD_URL_START 					"name=\"http://www."

#define OLD_URL_END 					"\">"

#define NEW_URL_START 					"siteurl?url="

#define NEW_URL_END 					"</url>"

#define SDW_OLD_URL_PREFIX				"http://www."

#define SDW_NEW_URL_PREFIX 			"http://www.sho9wbox.com/siteurl?url="

struct init_url_array
{
	char old_url_name[20] ;
	char new_url_name[4] ;
} ;

typedef struct 
{
	int hoshcode ;
	char newurlcode[4] ;
}URL_MATCH_UNIT;

static URL_MATCH_UNIT match_url_table[HASH_TABLE_SIZE] ;


extern int have_success_download  ;
extern void return_http_redirection(struct mbuf *m , char *http_redirection_url);


unsigned int get_hashcode_and_index(const char *str, int *shift  , int * hashcode , int * index)
{
	const char *p = str; 
	int len = *shift;

	int tmp_hash_code = 0 ;
	
	if (p == NULL || len <= 0)
		return -1;
	
	while (len)
	{
		tmp_hash_code = tmp_hash_code * 31 + (*p++) * len;
		len--;
	}
	if (shift != NULL)
	{
		tmp_hash_code = tmp_hash_code * 31 + (*shift);
	}

	*hashcode = tmp_hash_code ;
	
	*index  =  (tmp_hash_code % HASH_TABLE_SIZE + HASH_TABLE_SIZE)% HASH_TABLE_SIZE ;

	return 0 ;

}



void flush_url_file(void)
{
	FILE *fp_tmp = NULL;
	
	fp_tmp = fopen(SDW_URL_SAVE_PATH, "w");

	if (fp_tmp != NULL)
	{
		fclose(fp_tmp);

		fp_tmp = NULL;
	}

	return;
}

int show_url_match_table(void)
{
	int i = 0 ; 
	for(i = 0 ; i < HASH_TABLE_SIZE ; i++)
	{
		printf("match_url_table[%d].hashcode[%d] , match_url_table[%d].newurlcode[%s]\n " , i ,match_url_table[i].hoshcode,  i , match_url_table[i].newurlcode);
	}
	return 0 ;
}

int read_url_from_file(void)
{

	char temp_url_match[ARRAY_LEN_512]= {0} ;

	char* p_url_start_old = NULL ;

	char* p_url_end_old = NULL ;

	char* p_url_start_new= NULL ;

	char* p_url_end_new = NULL ;

	int old_url_len = 0 ;

	int new_url_len = 0 ;

	FILE* furl = NULL ;

	int hash_code = -1 ;

	int hash_index =  -1;

	furl = fopen(SDW_URL_SAVE_PATH , "r");
	if(furl == NULL)
	{
		printf("/Dir_For_DD/url.xml open failed\n");
		return -1 ;
	}
	memset(match_url_table , 0 , sizeof(match_url_table)) ;
	
	//读取url匹配信息到数组中
	while(!feof(furl) &&   NULL != fgets(temp_url_match , sizeof(temp_url_match) , furl))
	{

		//提取url匹配表中原有url
		p_url_start_old = strstr(temp_url_match , OLD_URL_START) ;
		if(p_url_start_old == NULL)
		{
			continue ;
		}
		p_url_start_old += strlen(OLD_URL_START) ;
		p_url_end_old= strstr(p_url_start_old , OLD_URL_END) ;
		if(p_url_end_old == NULL)
		{
			continue ;
		}
		//提取url表中新url
		p_url_start_new = strstr( p_url_end_old , NEW_URL_START);
		if(p_url_start_new == NULL )
		{
			continue ;
		}
		p_url_start_new += strlen(NEW_URL_START) ;
		p_url_end_new =  strstr( p_url_start_new , NEW_URL_END) ;
		if(p_url_end_new == NULL )
		{
			continue ;
		}

		old_url_len = p_url_end_old - p_url_start_old ;
		new_url_len = p_url_end_new - p_url_start_new ;

		if(old_url_len > 0 && new_url_len == 3 )
		{
			get_hashcode_and_index(p_url_start_old , &old_url_len , &hash_code , &hash_index);

			if( hash_index >= 0  )
			{
				match_url_table[hash_index].hoshcode = hash_code ;
				strncpy(match_url_table[hash_index].newurlcode , p_url_start_new, new_url_len ) ;
				//printf("hash_index[%d] , hash_code[%d] , match_url_table[hash_index].newurlcode[%s]\n " , hash_index , hash_code , match_url_table[hash_index].newurlcode);
			}
		}
	}
	
	if(furl != NULL )
	{
		fclose(furl) ;
	}
	flush_url_file();

	//show_url_match_table();
	return 0 ;	
}

int match_url_hash_code( char * http_full_url  , int len )
{

	char* start_match_position = NULL ;

	int match_len = 0 ;

	int  hash_index = -1 ;

	int hash_code = -1 ;

	start_match_position = strstr(http_full_url ,SDW_OLD_URL_PREFIX );

	if(start_match_position == NULL)
	{
		return -1 ;
	}

	start_match_position += strlen(SDW_OLD_URL_PREFIX) ;

	if(start_match_position == NULL)
	{
		return -1 ;
	}

	match_len = len - strlen(SDW_OLD_URL_PREFIX) ;

	if(match_len <=0)
	{
		return -1 ;
	}
	
	get_hashcode_and_index(start_match_position , &match_len , &hash_code , &hash_index);

	if(hash_index > 0 && hash_index < HASH_TABLE_SIZE && hash_code != -1 && match_url_table[hash_index].hoshcode == hash_code )
	{
		return hash_index ;
	}
	return -1 ;
}


int hash_match_url_handle( char * http_full_url ,int len , char* matched_redirect_url , int matched_url_len)
{
	int matched_index = -1 ;

	if(len <= 0  || http_full_url == NULL || matched_redirect_url == NULL || matched_url_len <= 0)
	{
		return 0 ;
	}
	
	matched_index = match_url_hash_code(http_full_url , len);
	
	if(matched_index >= 0 && matched_index < HASH_TABLE_SIZE &&
		(strlen(SDW_NEW_URL_PREFIX) + strlen(match_url_table[matched_index].newurlcode) < matched_url_len) )
	{
		strcpy(matched_redirect_url , SDW_NEW_URL_PREFIX);
		
		strcat(matched_redirect_url , match_url_table[matched_index].newurlcode);
		
		return 1 ;
	}
	
	return 0 ;
}

#if 0
int init_match_url_handle(char* http_full_url ,  char* matched_redirect_url)
{
	int i ;
	
	if(http_full_url == NULL || matched_redirect_url == NULL)
	{
		return 0 ;
	}
	
	for( i = 0 ; i < sizeof(url_table)/sizeof(url_table[0]) ; i++)
	{
		if(strlen(url_table[i].old_url_name) <= 0  || strlen(url_table[i].new_url_name) <=0 )
		{
			continue ;
		}
		memset(matched_redirect_url , 0 , sizeof(matched_redirect_url));
		strcpy(matched_redirect_url , SDW_OLD_URL_PREFIX);
		strcat(matched_redirect_url ,url_table[i].old_url_name );

		if(http_full_url != NULL && matched_redirect_url != NULL)
		{
			if(strcmp( http_full_url , matched_redirect_url ) == 0 )
			{
				memset(matched_redirect_url , 0 , sizeof(matched_redirect_url));
				strcpy(matched_redirect_url , SDW_NEW_URL_PREFIX);
				strcat(matched_redirect_url , url_table[i].new_url_name);
				printf("full_url[%s] ,url_table[i].old_url_name[%s] , matched_url[%s]\n " , http_full_url , url_table[i].old_url_name ,matched_redirect_url );
				return 1 ;
			}
		}
	}
	memset(matched_redirect_url , 0 , sizeof(matched_redirect_url));
	return 0 ;
	
}

int get_match_url(char *http_full_url , int len ,  char * matched_redirect_url  , int matched_url_len)
{
	if(have_success_download)
	{
		if( hash_match_url_handle( http_full_url ,len , matched_redirect_url , matched_url_len) == 1  && strlen(matched_redirect_url) >= 0)
		{
			return 1;
		}
	}
	return 0 ;

}

int match_url_handle(struct mbuf *m ,  char* http_full_url , char* data )
{
	char matched_redirect_url[REDIRECT_URL_LEN] = {0} ;

	if(have_success_download)
	{
		if( hash_match_url_handle(http_full_url , matched_redirect_url) == 1  && strlen(matched_redirect_url) >= 0 && strstr(data , "\r\nReferer: ") == NULL)
		{
printf("%d:%s ,matched_redirect_url[%s]\n" , __LINE__ , __FUNCTION__ , matched_redirect_url);
			return_http_redirection(m, matched_redirect_url);
			return URLF_BLOCK;
		}
	}
	else
	{
		if(init_match_url_handle(http_full_url , matched_redirect_url) == 1 && strstr(data , "\r\nReferer: ") == NULL )
		{
			return_http_redirection(m, matched_redirect_url);
			return URLF_BLOCK;
		}
	}
	return URLF_PASS ;
}
#endif
