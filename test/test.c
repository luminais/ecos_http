#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jhash.h"
typedef unsigned char	uint8;

#define DIFF_RULE_DELIM_STR ";"
#define SAME_RULE_DELIM_STR "|"
#define DIFF_RULE_DELIM_CHR ';'
#define SAME_RULE_DELIM_CHR '|'

#define MAX_TYPE_NUM (5)

#define WHITE_HOST_HASH_LEN (30)
#define URL_HOST_HASH_LEN (10)

#define CHECK_NULL_RETURN(p, ret) \
	do \
	{ \
		if(!p) \
		{ \
			printf("[%s][%d]\n", __FUNCTION__, __LINE__); \
			return (ret); \
		} \
	}while(0)

typedef enum white_rule_type
{
	WHITE_RULE_TYPE_HOST,
	WHITE_RULE_TYPE_URI,
	WHITE_RULE_TYPE_MAX
}white_rule_type_e;

typedef struct len_string
{
	unsigned int len;
	char *str;
}len_string_t;

typedef struct http_hdr_params
{
	len_string_t host;
	len_string_t uri;
	len_string_t suffix;
	len_string_t referer;
}http_hdr_params_t;

typedef struct url_rule
{
	uint8 type;
	char *host;
	char *suffix;
	char *uri;
	char *redirect;
}url_rule_t;

typedef struct white_rule
{
	uint8 type;
	white_rule_type_e white_type;
	char *value;
}white_rule_t;

typedef struct len_string_list
{
	len_string_t list;
	struct len_string_list *next;
}len_string_list_t;

typedef struct url_match_rule
{
	len_string_list_t *host[URL_HOST_HASH_LEN];
	len_string_t suffix;
	len_string_t uri;
	len_string_t redirect;
	struct url_match_rule *next;
}url_match_rule_t;

typedef struct url_type_match_rule
{
	uint8 type;
	url_match_rule_t *rule_list;
	len_string_list_t *white_host[WHITE_HOST_HASH_LEN];
	len_string_list_t *white_uri;
}url_match_rules_t;

url_match_rules_t g_url_rules[MAX_TYPE_NUM] = {0};

#if 1
void print_url_rule_t(url_rule_t *url_rule_p)
{
	printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	printf("[%s][%d]\t type : %u\n", __FUNCTION__, __LINE__, url_rule_p->type);
	if(NULL == url_rule_p->host)
		printf("[%s][%d]\t host : ALL\n", __FUNCTION__, __LINE__);
	else
		printf("[%s][%d]\t host : %s\n", __FUNCTION__, __LINE__, url_rule_p->host);
	if(NULL == url_rule_p->suffix)
		printf("[%s][%d]\t suffix : ALL\n", __FUNCTION__, __LINE__);
	else
		printf("[%s][%d]\t suffix : %s\n", __FUNCTION__, __LINE__, url_rule_p->suffix);
	if(NULL == url_rule_p->uri)
		printf("[%s][%d]\t uri : ALL\n", __FUNCTION__, __LINE__);
	else
		printf("[%s][%d]\t uri : %s\n", __FUNCTION__, __LINE__, url_rule_p->uri);
	printf("[%s][%d]\t redirect : %s\n", __FUNCTION__, __LINE__, url_rule_p->redirect);
	printf("[%s][%d]\n", __FUNCTION__, __LINE__);
}

#define WHITE_TYPE_STR(type) \
	((type==WHITE_RULE_TYPE_HOST)?"host": \
	((type==WHITE_RULE_TYPE_URI)?"uri":"unknow"))

void print_white_rule_t(white_rule_t *white_rule_p)
{
	printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	printf("[%s][%d]\t type : %u\n", __FUNCTION__, __LINE__, white_rule_p->type);
	printf("[%s][%d]\t white_type : %s\n", __FUNCTION__, __LINE__, WHITE_TYPE_STR(white_rule_p->white_type));
	printf("[%s][%d]\t value : %s\n", __FUNCTION__, __LINE__, white_rule_p->value);
	printf("[%s][%d]\n", __FUNCTION__, __LINE__);
}

void print_len_string_list(len_string_list_t *len_string_list_p)
{
	int first = 1;
	while(len_string_list_p)
	{
		if(first)
		{
			printf("%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
			first = 0;
		}
		else
			printf("|%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
		len_string_list_p = len_string_list_p->next;
	}
}

void print_url_rules(void)
{
	int i, j;
	url_match_rules_t *rules_p;
	url_match_rule_t *rule_p;
	len_string_list_t *len_string_list_p;
	int first;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		printf("[%s][%d] type %u : \n", __FUNCTION__, __LINE__, i+1);
		rules_p = &(g_url_rules[i]);
		printf("\t<url rule> : \n");
		rule_p = rules_p->rule_list;
		while(rule_p)
		{
			first = 1;
			printf("\t");
            for(j=0; j<URL_HOST_HASH_LEN; j++)
    		{
    			len_string_list_p = rule_p->host[j];
				while(len_string_list_p)
				{
					if(first)
					{
						printf("%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
						first = 0;
					}
					else
						printf("|%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
					len_string_list_p = len_string_list_p->next;
				}
    		}
			printf(";");
			if(rule_p->suffix.len > 0)
				printf("%.*s", rule_p->suffix.len, rule_p->suffix.str);
			printf(";");
			if(rule_p->uri.len > 0)
				printf("%.*s", rule_p->uri.len, rule_p->uri.str);
			printf(";");
			if(rule_p->redirect.len > 0)
				printf("%.*s", rule_p->redirect.len, rule_p->redirect.str);
			printf("\n");
			rule_p = rule_p->next;
		}
		printf("\t<white host> : \n");
		for(j=0; j<WHITE_HOST_HASH_LEN; j++)
		{
			if(rules_p->white_host[j])
			{
				printf("\t");
				print_len_string_list(rules_p->white_host[j]);
				printf("\n");
			}
		}
		printf("\t<white uri> : \n");
		printf("\t");
		print_len_string_list(rules_p->white_uri);
		printf("\n");
	}
}
#endif

void init_url_rules(void)
{
	int i;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		memset(&(g_url_rules[i]), 0x0, sizeof(url_match_rules_t));
		g_url_rules[i].type = i+1;
	}
}

void *lm_malloc(size_t size)
{
	void *ptr = malloc(size);

	if(!ptr)
		memset(ptr, 0x0, size);

	return ptr;
}

len_string_t *len_string_new(unsigned int len, char *str)
{
	len_string_t *len_string_p;

	if(len == 0 || !str)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	len_string_p = (len_string_t *)lm_malloc(sizeof(len_string_t));
	if(!len_string_p)
	{
		printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	len_string_p->str = (char *)malloc(len);
	if(!len_string_p->str)
	{
		printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		free(len_string_p);
		return NULL;
	}
	memcpy(len_string_p->str, str, len);
	len_string_p->len = len;

	return len_string_p;
}

int len_string_set(len_string_t *len_string_p, unsigned int len, char *str)
{
	if(len == 0 || !str)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	len_string_p->str = (char *)malloc(len);
	if(!len_string_p->str)
	{
		printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	memcpy(len_string_p->str, str, len);
	len_string_p->len = len;

	return 0;
}

int add_len_string_list(len_string_list_t **list, char *value)
{
	len_string_list_t *len_string_list_p, *list_tmp;
	int str_len;
	if(!list || !value)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	len_string_list_p = (len_string_list_t *)lm_malloc(sizeof(len_string_list_t));
	CHECK_NULL_RETURN(len_string_list_p, -1);

	str_len = strlen(value);
	if(0 != len_string_set(&(len_string_list_p->list), str_len, value))
	{
		printf("[%s][%d] len_string_set failed\n", __FUNCTION__, __LINE__);
		free(len_string_list_p);
		return -1;
	}

	if(*list == NULL)
		*list = len_string_list_p;
	else
	{
		list_tmp = *list;
		while(list_tmp->next)
			list_tmp = list_tmp->next;
		list_tmp->next = len_string_list_p;
	}

	return 0;
}

void len_string_list_free(len_string_list_t *len_string_list_p)
{
	len_string_list_t *p, *q;

	if(!len_string_list_p)
	{
		return;
	}

	p = len_string_list_p;
	while(p)
	{
		q = p->next;
		if(p->list.str)
			free(p->list.str);
		free(p);
		p = q;
	}

	return;
}

void url_match_rule_free(url_match_rule_t *url_match_rule_p)
{
    int i;

	if(!url_match_rule_p)
	{
		return;
	}

    for(i=0; i<URL_HOST_HASH_LEN; i++)
	{
		len_string_list_free(url_match_rule_p->host[i]);
	}

	if(url_match_rule_p->suffix.str)
		free(url_match_rule_p->suffix.str);
	if(url_match_rule_p->uri.str)
		free(url_match_rule_p->uri.str);
	if(url_match_rule_p->redirect.str)
		free(url_match_rule_p->redirect.str);
	free(url_match_rule_p);

	return;
}

void url_rules_free(void)
{
	int i, j;
	url_match_rules_t *rules_p;
	url_match_rule_t *rule_p;
	len_string_list_t *len_string_list_p;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		rules_p = &(g_url_rules[i]);
		rule_p = rules_p->rule_list;
		url_match_rule_free(rule_p);
		for(j=0; j<WHITE_HOST_HASH_LEN; j++)
		{
			len_string_list_free(rules_p->white_host[j]);
		}
		len_string_list_free(rules_p->white_uri);
	}
}

int add_len_string_hash(len_string_list_t *list_hash[], int hash_len, char *value)
{
	char *char_p = NULL, *char_save = NULL;
	u32 hash_index;
	if(!list_hash || !value)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	char_p = strtok_r(value, SAME_RULE_DELIM_STR, &char_save);
	while(char_p)
	{
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
		hash_index = jhash(char_p, strlen(char_p), 0)%hash_len;
		if(0 != add_len_string_list(&(list_hash[hash_index]), char_p))
			goto add_white_host_failed;
		char_p = strtok_r(NULL, SAME_RULE_DELIM_STR, &char_save);
	}
	return 0;
add_white_host_failed:
	// free
	return -1;
}

url_match_rule_t *url_match_rule_new(url_rule_t *url_rule_p)
{
	url_match_rule_t *url_match_rule_p;
	if(!url_rule_p)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	url_match_rule_p = (url_match_rule_t *)lm_malloc(sizeof(url_match_rule_t));
	if(NULL == url_match_rule_p)
	{
		printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	if(NULL != url_rule_p->host)
	{
        if(0 != add_len_string_hash(url_match_rule_p->host, URL_HOST_HASH_LEN, url_rule_p->host))
            goto url_match_rule_new_failed;
	}

	if(NULL != url_rule_p->suffix)
	{
		if(0 != len_string_set(&(url_match_rule_p->suffix), strlen(url_rule_p->suffix), url_rule_p->suffix))
			goto url_match_rule_new_failed;
	}

	if(NULL != url_rule_p->uri)
	{
		if(0 != len_string_set(&(url_match_rule_p->uri), strlen(url_rule_p->uri), url_rule_p->uri))
			goto url_match_rule_new_failed;
	}

	if(NULL != url_rule_p->redirect)
	{
		if(0 != len_string_set(&(url_match_rule_p->redirect), strlen(url_rule_p->redirect), url_rule_p->redirect))
			goto url_match_rule_new_failed;
	}

	return url_match_rule_p;
url_match_rule_new_failed:
	printf("[%s][%d] url_match_rule_new failed\n", __FUNCTION__, __LINE__);
	url_match_rule_free(url_match_rule_p);
	return NULL;
}

int add_url_rule(url_rule_t *url_rule_p)
{
	url_match_rules_t *url_match_rules_p;
	url_match_rule_t *url_match_rule_p;
	url_match_rule_t *p;
	if(!url_rule_p)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	url_match_rules_p = &(g_url_rules[url_rule_p->type-1]);

	url_match_rule_p = url_match_rule_new(url_rule_p);

	CHECK_NULL_RETURN(url_match_rule_p, -1);

	if(NULL == url_match_rules_p->rule_list)
		url_match_rules_p->rule_list = url_match_rule_p;
	else
	{
		p = url_match_rules_p->rule_list;
		while(p->next)
			p = p->next;
		p->next = url_match_rule_p;
	}

	return 0;
}

int parse_url_rule(char *url_rule)
{
	url_rule_t url_rule_s;
	char *char_p = url_rule, *char_q;

	if(!url_rule)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	memset(&url_rule_s, 0x0, sizeof(url_rule_s));

	//type
	char_p = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_p, -1);
	char_p++;
	url_rule_s.type = (uint8)atoi(char_p);
	if(!(url_rule_s.type>=1 && url_rule_s.type <= MAX_TYPE_NUM))
	{
		printf("[%s][%d] invalid type : %d->%u\n", __FUNCTION__, __LINE__, atoi(char_p), url_rule_s.type);
		return -1;
	}

	// host
	char_p = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_p, -1);
	char_p++;
	char_q = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_q, -1);
	*char_q = '\0';
	if(char_p != char_q)
		url_rule_s.host = char_p;
	else
		url_rule_s.host = NULL;

	// suffix
	char_p = char_q + 1;
	char_q = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_q, -1);
	*char_q = '\0';
	if(char_p != char_q)
		url_rule_s.suffix = char_p;
	else
		url_rule_s.suffix = NULL;

	// uri
	char_p = char_q + 1;
	char_q = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_q, -1);
	*char_q = '\0';
	if(char_p != char_q)
		url_rule_s.uri = char_p;
	else
		url_rule_s.uri = NULL;

	// redirect
	char_p = char_q + 1;
	if(*char_p == '\0')
		return -1;
	url_rule_s.redirect = char_p;

	print_url_rule_t(&url_rule_s);

	return add_url_rule(&url_rule_s);
}

int parse_url_rules(char *url_rules, const char *delim)
{
	char *char_p = NULL, *char_save = NULL;
	if(!url_rules || !delim)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	char_p = strtok_r(url_rules, delim, &char_save);
	while(char_p)
	{
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
		if(*char_p == '1')
			parse_url_rule(char_p);
		char_p = strtok_r(NULL, delim, &char_save);
	}

	return 0;
}

int add_white_uri(len_string_list_t **list, char *value)
{
	char *char_p = NULL, *char_save = NULL;
	if(!list || !value)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	char_p = strtok_r(value, SAME_RULE_DELIM_STR, &char_save);
	while(char_p)
	{
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
		if(0 != add_len_string_list(list, char_p))
			goto add_white_uri_failed;
		char_p = strtok_r(NULL, SAME_RULE_DELIM_STR, &char_save);
	}
	return 0;
add_white_uri_failed:
	// free
	return -1;
}

int add_white_rule(white_rule_t *white_rule_p)
{
	url_match_rules_t *url_match_rules_p;
	int ret = -1;
	if(!white_rule_p)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	url_match_rules_p = &(g_url_rules[white_rule_p->type-1]);

	switch(white_rule_p->white_type)
	{
		case WHITE_RULE_TYPE_HOST:
			ret = add_len_string_hash(url_match_rules_p->white_host, WHITE_HOST_HASH_LEN, white_rule_p->value);
			break;
		case WHITE_RULE_TYPE_URI:
			ret = add_white_uri(&(url_match_rules_p->white_uri), white_rule_p->value);
			break;
		default:
			break;
	}

	return ret;
}

int parse_white_rule(char *white_rule)
{
	white_rule_t white_rule_s;
	char *char_p = white_rule, *char_q;

	if(!white_rule)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	memset(&white_rule_s, 0x0, sizeof(white_rule_s));

	//type
	white_rule_s.type = (uint8)atoi(char_p);
	if(!(white_rule_s.type>=1 && white_rule_s.type <= MAX_TYPE_NUM))
	{
		printf("[%s][%d] invalid type : %d->%u\n", __FUNCTION__, __LINE__, atoi(char_p), white_rule_s.type);
		return -1;
	}

	// white_type
	char_p = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_p, -1);
	char_p++;
	char_q = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_q, -1);
	*char_q = '\0';
	if(char_p == char_q)
	{
		return -1;
	}
	if(0 == strcasecmp(char_p, "host"))
		white_rule_s.white_type = WHITE_RULE_TYPE_HOST;
	else if(0 == strcasecmp(char_p, "uri"))
		white_rule_s.white_type = WHITE_RULE_TYPE_URI;
	else
		return -1;

	// value
	char_p = char_q + 1;
	if(*char_p == '\0')
	{
		return -1;
	}
	white_rule_s.value = char_p;

	print_white_rule_t(&white_rule_s);

	return add_white_rule(&white_rule_s);
}

int parse_white_rules(char *white_rules, const char *delim)
{
	char *char_p = NULL, *char_save = NULL;
	if(!white_rules || !delim)
	{
		printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	char_p = strtok_r(white_rules, delim, &char_save);
	while(char_p)
	{
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
		parse_white_rule(char_p);
		char_p = strtok_r(NULL, delim, &char_save);
	}

	return 0;
}

int main()
{
#if 1
	//开启关闭;类型;Host;后缀;URI;重定向网址
	char test_rule[1024] = "\r\n\r\n1;1;baidu.com|163.com;js;a=2;http://www.hao123.com/\n\n\n1;1;qq.com;js;;http://www.hao123.com/\r\n0;2;liquan.com;css;x=1;http://www.shubao.com/\n1;2;;html;x=1;http://www.taobao.com/\n1;3;google.com|buglist.com;nosuffix;cc=k;http://www.luminais.com/\n\n\n";
	//char *test_rule = "1;1;baidu.com|163.com;js;a=2;http://www.hao123.com/^^^1;1;qq.com;js;;http://www.hao123.com/@^1;2;;html;x=1;http://www.taobao.com/^1;3;google.com|buglist.com;nosuffix;cc=k;http://www.luminais.com/^^^";
#if 0
	printf("[%s][%d] test_rule : \n", __FUNCTION__, __LINE__);
	printf("%s", test_rule);
#endif

	parse_url_rules(test_rule, " \n\r");

	// 类型;白名单类型;值;
	char test_white[1024] = "1;uri;dn=2\n1;host;diannao.com\r\n2;HoSt;edu.cn|163.com|org.net\n\n3;uri;cc=a&d=1\n4;Uri;tx=2|rx=34|tt=60\n\n5;;dd.cn|aa.net\n4;host;";
#if 0
	printf("[%s][%d] test_white : \n", __FUNCTION__, __LINE__);
	printf("%s", test_white);
#endif
	parse_white_rules(test_white, " \n\r");

#if 1
	printf("[%s][%d] print_url_rules : \n", __FUNCTION__, __LINE__);
	print_url_rules();
#endif

	char http_hdr[1024] = "GET /jzt/tpl/sspPic.html?ad_ids=3194:5&adflag=0&clkmn=&expose= HTTP/1.1\r\nHost: static-alias-1.360buyimg.com\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:28.0) Gecko/20100101 Firefox/28.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\nAccept-Encoding: gzip, deflate\r\nReferer: http://wa.gtimg.com/website/201709/bjjdsj_QNR_20170901175534.html?tclick=http%3A%2F%2Fc.l.qq.com%2Flclick%3Foid%3D4028810%26cid%3D2691790%26loc%3DQQCOM_N_Rectangle3%26soid%3DbhwOtwAAWzzb4Q5t9QjyJplYARJu%26click_data%3DdXNlcl9pbmZvPW9BRGptejA2Rmg0PSZheHBoZWFkZXI9MSZwYWdlX3R5cGU9MSZzc3A9MSZ1cF92ZXJzaW9uPVM5MnxMNTcxJnNpPTE4MzUyMjQ2MQ%3D%3D%26index%3D1%26chl%3D478\r\nConnection: keep-alive\r\n\r\n"

	printf("[%s][%d] url_rules_free\n", __FUNCTION__, __LINE__);
	url_rules_free();
#endif
#if 0
#if 0
	char test_str[256] = "1;2;;html;x=1;http://www.taobao.com/";
	printf("[%s][%d] test_str : %s\n", __FUNCTION__, __LINE__, test_str);
	parse_url_rule(test_str);
	strcpy(test_str, "1;1;baidu.com|163.com;js;a=2;http://www.hao123.com/");
	printf("[%s][%d] test_str : %s\n", __FUNCTION__, __LINE__, test_str);
	parse_url_rule(test_str);
	strcpy(test_str, "1;1;;;;http://www.hao123.com/");
	printf("[%s][%d] test_str : %s\n", __FUNCTION__, __LINE__, test_str);
	parse_url_rule(test_str);
	strcpy(test_str, "1;2;;html;x=1;");
	printf("[%s][%d] test_str : %s\n", __FUNCTION__, __LINE__, test_str);
	parse_url_rule(test_str);
#endif

	int ret;
	ret = strcasecmp("URI", "uri");
	printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
#endif
}




