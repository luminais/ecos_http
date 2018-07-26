#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jhash.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define URL_REDIRECT_MATCH_DEBUG 1
typedef unsigned int	uint32;
typedef unsigned char	uint8;

#define R_FILE "rule"
#define W_FILE "white"

#define diag_printf printf

#define HOST_STR "Host: "
#define HOST_LEN 6
#define REFERER_STR "Referer: "
#define REFERER_LEN 9
#define UA_STR "User-Agent: "
#define UA_LEN 12
#define COOKIE_STR "Cookie: "
#define COOKIE_LEN 8
#define CONTENT_T_STR "Content-Type: "
#define CONTENT_T_LEN 14
#define CONTENT_L_STR "Content-Length: "
#define CONTENT_L_LEN 16

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

typedef enum url_redirect_match_rst
{
	URL_REDIRECT_MATCH_WHITE,
	URL_REDIRECT_MATCH_REDIRECT,
	URL_REDIRECT_MATCH_NULL,
	URL_REDIRECT_MATCH_MAX
}url_redirect_match_rst_e;

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
	len_string_t query;
	len_string_t referer;
}http_hdr_params_t;

typedef struct url_rule
{
	uint8 type;
	uint32 time;
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
	uint32 time;
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
	printf("[%s][%d]\t time : %u\n", __FUNCTION__, __LINE__, url_rule_p->time);
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

void print_url_rule(url_match_rule_t *rule_p)
{
	len_string_list_t *len_string_list_p;
	int j, first = 1;
	printf("\t");
	printf("%u;", rule_p->time);
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
}

void print_url_rules(void)
{
	int i, j;
	url_match_rules_t *rules_p;
	url_match_rule_t *rule_p;
	len_string_list_t *len_string_list_p;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		printf("[%s][%d] type %u : \n", __FUNCTION__, __LINE__, i+1);
		rules_p = &(g_url_rules[i]);
		printf("\t<url rule> : \n");
		rule_p = rules_p->rule_list;
		while(rule_p)
		{
			print_url_rule(rule_p);
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

void print_http_hdr_params(http_hdr_params_t *http_hdr_params_p)
{
	if(!http_hdr_params_p)
		return;
	printf("[%s][%d] http_hdr_params : \n", __FUNCTION__, __LINE__);
	printf("\thost : %.*s\n", http_hdr_params_p->host.len, http_hdr_params_p->host.str);
	printf("\turi : %.*s\n", http_hdr_params_p->uri.len, http_hdr_params_p->uri.str);
	if(http_hdr_params_p->suffix.len == 0)
		printf("\tsuffix : is NULL\n");
	else
		printf("\tsuffix : %.*s\n", http_hdr_params_p->suffix.len, http_hdr_params_p->suffix.str);
	if(http_hdr_params_p->query.len == 0)
		printf("\tquery : is NULL\n");
	else
		printf("\tquery : %.*s\n", http_hdr_params_p->query.len, http_hdr_params_p->query.str);
	if(http_hdr_params_p->referer.len == 0)
		printf("\treferer : is NULL\n");
	else
		printf("\treferer : %.*s\n", http_hdr_params_p->referer.len, http_hdr_params_p->referer.str);
}
#endif

int file_to_buf(unsigned char **file_buf_save, char *file_path, int file_len)
{
	unsigned char *file_buf = NULL;
	int file_fd = -1, read_count = 0;

	file_buf = (unsigned char *)malloc(file_len);
	if(NULL == file_buf )
	{
		perror("file_to_buf : No memory.\n");
		goto fail_exit;
	}
	
	memset(file_buf, 0x0, file_len);

	file_fd = open(file_path, O_RDONLY);
	if(file_fd < 0)
	{
		diag_printf("[%s][%d] open file_path failed\n", __FUNCTION__, __LINE__);
		goto fail_exit;
	}
	
	read_count = read(file_fd, file_buf, file_len);
	if(read_count != file_len)
	{
		diag_printf("[%s][%d] read_count = %d, file_len = %d\n", __FUNCTION__, __LINE__, read_count, file_len);
		goto fail_exit;
	}

	close(file_fd);
	//unlink(file_path);
	//diag_printf("[%s][%d] clear_file \n", __FUNCTION__, __LINE__);
	//clear_file(file_path);
	*file_buf_save = file_buf;
	return 0;

fail_exit:
	if(NULL != file_buf)
		free(file_buf);
	if(file_fd != -1)
		close(file_fd);
	//unlink(file_path);
	//diag_printf("[%s][%d] clear_file \n", __FUNCTION__, __LINE__);
	//clear_file(file_path);
	return -1;
}

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
#ifdef URL_REDIRECT_MATCH_DEBUG
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
#endif
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

	url_match_rule_p->time = url_rule_p->time;

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

	// time
	char_p = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_p, -1);
	char_p++;
	url_rule_s.time = (uint)atoi(char_p);

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
	if(NULL == strstr(char_p, "?s="))
	{
		printf("[%s][%d] no ?s=\n", __FUNCTION__, __LINE__);
		return -1;
	}
	url_rule_s.redirect = char_p;
#ifdef URL_REDIRECT_MATCH_DEBUG
	print_url_rule_t(&url_rule_s);
#endif
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
#ifdef URL_REDIRECT_MATCH_DEBUG
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
#endif
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
#ifdef URL_REDIRECT_MATCH_DEBUG
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
#endif
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

	printf("[%s][%d] white_rule_s.type : %u\n", __FUNCTION__, __LINE__, white_rule_s.type);

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

	printf("[%s][%d] white_rule_s.white_type : %s\n", __FUNCTION__, __LINE__, WHITE_TYPE_STR(white_rule_s.white_type));

	// value
	char_p = char_q + 1;
	if(*char_p == '\0')
	{
		return -1;
	}
	white_rule_s.value = char_p;
#if 1//def URL_REDIRECT_MATCH_DEBUG
	print_white_rule_t(&white_rule_s);
#endif
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
#if 1//def URL_REDIRECT_MATCH_DEBUG
		printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
#endif
		parse_white_rule(char_p);
		char_p = strtok_r(NULL, delim, &char_save);
	}

	return 0;
}

char *http_hdr_find_field(char *http_hdr,  int http_hdr_len, char *field, int field_len)
{
	char *cp = http_hdr;
	char *s1, *s2;
	int len, line_len;
	
	if(NULL==http_hdr || NULL==field || 0==http_hdr_len || 0==field_len)
		return NULL;

	if(http_hdr_len >= field_len)
	{
		len = http_hdr_len - field_len;
		
		while (*cp)
		{
			s1 = cp;
			s2 = field;

			while ( *s1 && *s2 && !(*s1-*s2) )
				s1++, s2++;

			if (!*s2)
				return(cp);
			
			s2 = strchr(cp, '\r');
			if(NULL==s2)
				break;

			s2 += 2;
			line_len = (int)(s2-cp);
			
			len -= line_len;
			if(len < 0)
				break;

			cp = s2;
		}
	}

	return NULL;
}

char *strstr_len(char *s, int s_len, char *d, int d_len)
{
	int i, j, k;
	if(!s || *s == '\0' || !d || *d == '\0')
		return NULL;
	if(s_len < d_len)
		return NULL;
	for(i=0; i<s_len&&(s_len-i>=d_len); i++)
	{
		if(s[i] == d[0])
		{
			for(k=i+1, j=1; j<d_len; k++, j++)
				if(s[k] != d[j])
					break;
			if(j == d_len)
				return &(s[i]);
		}
	}

	return NULL;
}

char *strrstr_len(char *s, int s_len, char *d, int d_len)
{
	int i, j, k;
	if(!s || *s=='\0' || !d || *d=='\0')
		return NULL;
	if(s_len < d_len)
		return NULL;
	for(i=s_len-d_len; i>=0; i--)
	{
		if(s[i] == d[0])
		{
			for(k=i+1, j=1; j<d_len; k++, j++)
				if(s[k] != d[j])
					break;
			if(j == d_len)
				return &(s[i]);
		}
	}

	return NULL;
}

char *strchr_len(char *s, int s_len, char c)
{
	int i;
	if(!s || *s == '\0')
		return NULL;
	for(i=0; i<s_len; i++)
		if(c == s[i])
			break;
	if(i<s_len)
		return &(s[i]);
	else
		return NULL;
}

char *strrchr_len(char *s, int s_len, char c)
{
	int i;
	if(!s)
		return NULL;
	for(i=s_len-1; i>=0; i--)
		if(c == s[i])
			break;
	if(i>=0)
		return &(s[i]);
	else
		return NULL;
}

char *strrchr_len_step(char *s, int s_len, char c, char *pre)
{
	int len;
	if(!s)
		return NULL;
	if(!pre)
		return strrchr_len(s, s_len, c);
	else
	{
		len = (int)(pre - s);
		return strrchr_len(s, len, c);
	}
}

int parse_http_hdr_params(char *http_hdr,  int http_hdr_len, http_hdr_params_t *http_hdr_params_p)
{
	len_string_t *len_string_p;
	char *char_p, *char_q;
	int query_off = 0, suffix_len = 0;

	if(NULL==http_hdr || NULL==http_hdr_params_p)
		return -1;

	// host
	len_string_p = &(http_hdr_params_p->host);
	len_string_p->str = http_hdr_find_field(http_hdr, http_hdr_len, HOST_STR, HOST_LEN);
	CHECK_NULL_RETURN(len_string_p->str, -1);
	len_string_p->str += HOST_LEN;
	char_p = strchr(len_string_p->str, '\r');
	CHECK_NULL_RETURN(char_p, -1);
	len_string_p->len = (int)(char_p - len_string_p->str);

	// uri
	len_string_p = &(http_hdr_params_p->uri);
	len_string_p->str = http_hdr + 4; // "GET "
	char_p = strchr(len_string_p->str, '\r');
	CHECK_NULL_RETURN(char_p, -1);
	char_p -= 9;
	if(0 != memcmp(char_p, " HTTP/1.", 8))
		return -1;
	len_string_p->len = (int)(char_p - len_string_p->str);

	// query
	char_q = strchr_len(len_string_p->str, len_string_p->len, '?');
	if(char_q)
	{
		query_off = (int)(char_p - char_q);
		len_string_p = &(http_hdr_params_p->query);
		len_string_p->str = char_q + 1;
		len_string_p->len = query_off - 1;
	}

	// suffix
	len_string_p = &(http_hdr_params_p->uri);
	char_q = strchr_len(len_string_p->str, len_string_p->len - query_off, '.');
	if(char_q)
	{
		suffix_len = len_string_p->len - (int)(char_q - len_string_p->str +1) - query_off;
		len_string_p = &(http_hdr_params_p->suffix);
		len_string_p->str = char_q + 1;
		len_string_p->len = suffix_len;
	}

	// referer
	len_string_p = &(http_hdr_params_p->referer);
	len_string_p->str = http_hdr_find_field(http_hdr, http_hdr_len, REFERER_STR, REFERER_LEN);
	if(len_string_p->str)
	{
		len_string_p->str += REFERER_LEN;
		char_p = strchr(len_string_p->str, '\r');
		if(char_p)
			len_string_p->len = (int)(char_p - len_string_p->str);
	}

	return 0;
}

int len_string_cmp(len_string_t *s, len_string_t *d)
{
	if(!s || !d)
		return -1;
	if(s->len == d->len)
		return memcmp(s->str, d->str, s->len);
	else
		return ((s->len > d->len)?1:-1);
}

int len_string_list_find(len_string_list_t *list, len_string_t *s)
{
	len_string_list_t *len_string_p = list;
	if(!list || !s)
		return 0;

	while(len_string_p)
	{
		if(0 == len_string_cmp(&(len_string_p->list), s))
			return 1;
		len_string_p = len_string_p->next;
	}

	return 0;
}

int len_string_list_str(len_string_list_t *list, len_string_t *s)
{
	len_string_list_t *len_string_p = list;
	if(!list || !s)
		return 0;

	while(len_string_p)
	{
		if(NULL != strstr_len(s->str, s->len, len_string_p->list.str, len_string_p->list.len))
			return 1;
		len_string_p = len_string_p->next;
	}

	return 0;
}

int url_rule_host_match(len_string_list_t *list_hash[], int hash_len, char *host, int host_len)
{
	u32 hash_index;
	char *char_p, *char_q;
	int str_len;
	len_string_t len_string_tmp;
	if(!list_hash || !host)
		return 0;

	hash_index = jhash(host, host_len, 0)%hash_len;
	len_string_tmp.len = host_len;
	len_string_tmp.str = host;
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] host : %.*s\n", __FUNCTION__, __LINE__, len_string_tmp.len, len_string_tmp.str);
#endif
	if(1 == len_string_list_find(list_hash[hash_index], &len_string_tmp))
		return 1;

	char_p = strrchr_len_step(host, host_len, '.', NULL);
	char_p = strrchr_len_step(host, host_len, '.', char_p);
	while(char_p)
	{
		char_q = char_p + 1;
		str_len = host_len - (int)(char_q - host);
		hash_index = jhash(char_q, str_len, 0)%hash_len;
		len_string_tmp.len = str_len;
		len_string_tmp.str = char_q;
#ifdef URL_REDIRECT_MATCH_DEBUG
		printf("[%s][%d] host : %.*s\n", __FUNCTION__, __LINE__, len_string_tmp.len, len_string_tmp.str);
#endif
		if(1 == len_string_list_find(list_hash[hash_index], &len_string_tmp))
			return 1;
		char_p = strrchr_len_step(host, host_len, '.', char_p);
	}

	return 0;
}

int url_match_redirect_rule(url_match_rule_t *rule, http_hdr_params_t *http_hdr_params_p)
{
	int i;
	char strtmp[16] = {0};
	if(!rule || !http_hdr_params_p)
		return 0;
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	print_url_rule(rule);
#endif
	// suffix
	if(0 != rule->suffix.len)
	{
		strcpy(strtmp, "nosuffix");
		if(rule->suffix.len == strlen(strtmp) && 0 == memcmp(strtmp, rule->suffix.str, rule->suffix.len))
		{
			if(0 != http_hdr_params_p->suffix.len)
				return 0;
		}
		else
		{
			if(0 != len_string_cmp(&(rule->suffix), &(http_hdr_params_p->suffix)))
				return 0;
		}
	}
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] suffix match\n", __FUNCTION__, __LINE__);
#endif
	// uri
	if(0 != rule->uri.len)
	{
		if(NULL == strstr_len(http_hdr_params_p->uri.str, http_hdr_params_p->uri.len, rule->uri.str, rule->uri.len))
			return 0;
	}
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] uri match\n", __FUNCTION__, __LINE__);
#endif
	// host
	if(1 != url_rule_host_match(rule->host, URL_HOST_HASH_LEN, http_hdr_params_p->host.str, http_hdr_params_p->host.len))
	{
		for(i=0; i<URL_HOST_HASH_LEN && NULL == (rule->host)[i]; i++);
		if(URL_HOST_HASH_LEN != i)
			return 0;
	}
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] host match\n", __FUNCTION__, __LINE__);
#endif
	return 1;
}

int url_match_redirect_rules(url_match_rule_t *rule_list, http_hdr_params_t *http_hdr_params_p, len_string_t **redirect)
{
	url_match_rule_t *url_match_rule_p = rule_list;
	if(!rule_list || !http_hdr_params_p || !redirect)
		return 0;

	while(url_match_rule_p)
	{
		if(1 == url_match_redirect_rule(url_match_rule_p, http_hdr_params_p))
		{
			*redirect = &(url_match_rule_p->redirect);
			return 1;
		}
		url_match_rule_p = url_match_rule_p->next;
	}

	return 0;
}

url_redirect_match_rst_e url_match_rules_rst(url_match_rules_t *url_match_rules_p, http_hdr_params_t *http_hdr_params_p, len_string_t **redirect)
{
	if(!url_match_rules_p || !http_hdr_params_p || !redirect)
		return URL_REDIRECT_MATCH_NULL;

	// white host
	if(1 == url_rule_host_match(url_match_rules_p->white_host, WHITE_HOST_HASH_LEN, http_hdr_params_p->host.str, http_hdr_params_p->host.len))
		return URL_REDIRECT_MATCH_WHITE;
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] white host not match\n", __FUNCTION__, __LINE__);
#endif

	// white uri
	if(1 == len_string_list_str(url_match_rules_p->white_uri, &(http_hdr_params_p->uri)))
		return URL_REDIRECT_MATCH_WHITE;
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] white uri not match\n", __FUNCTION__, __LINE__);
#endif
	// rule_list
	if(1 == url_match_redirect_rules(url_match_rules_p->rule_list, http_hdr_params_p, redirect))
		return URL_REDIRECT_MATCH_REDIRECT;
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] rule_list not match\n", __FUNCTION__, __LINE__);
#endif
	return URL_REDIRECT_MATCH_NULL;
}

url_redirect_match_rst_e url_redirect_match(http_hdr_params_t *http_hdr_params_p, len_string_t **redirect)
{
	int i;
	url_redirect_match_rst_e match_rst = URL_REDIRECT_MATCH_NULL;
	url_match_rules_t *url_match_rules_p;

	if(!http_hdr_params_p || !redirect)
		return URL_REDIRECT_MATCH_NULL;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		url_match_rules_p = &(g_url_rules[i]);
		match_rst = url_match_rules_rst(url_match_rules_p, http_hdr_params_p, redirect);
		if(URL_REDIRECT_MATCH_NULL != match_rst)
		{
			return match_rst;
		}
	}

	return match_rst;
}

void mac_to_id(unsigned char *mac, unsigned char *dst)
{
	dst[0] = mac[0] << 6 | mac[1] >> 2;
	dst[1] = mac[1] << 6 | mac[2] >> 2;
	dst[2] = mac[2] << 6 | mac[3] >> 2;
	dst[3] = mac[3] << 6 | mac[4] >> 2;
	dst[4] = mac[4] << 6 | mac[5] >> 2;
	dst[5] = mac[5] << 6 | mac[0] >> 2;
}
#define ETHER_ADDR_LEN (6)
int ether_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, ETHER_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}

void print_packet(unsigned char *packet, int len)
{
	int i;

	for(i=0; i<len; i++)
	{
		diag_printf("%02x ", packet[i]);
		if((i+1)%16 == 0)
			diag_printf("\n");
	}
	diag_printf("\n");
}

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define NIPQUAD_FMT "%u.%u.%u.%u"

int main()
{
#if 0
	init_url_rules();
	//开启关闭;类型;Host;后缀;URI;重定向网址
	char test_rule[1024] = "\r\n\r\n1;1;300;baidu.com|163.com;js;a=2;http://www.hao123.com/\n\n\n1;1;3600;qq.com;js;;http://www.hao123.com/\r\n0;2;1800;liquan.com;css;x=1;http://www.shubao.com/\n1;2;11111;;html;x=1;http://www.taobao.com/\n1;3;99999;google.com|buglist.com;nosuffix;cc=k;http://www.luminais.com/\n\n\n";
	//char *test_rule = "1;1;baidu.com|163.com;js;a=2;http://www.hao123.com/^^^1;1;qq.com;js;;http://www.hao123.com/@^1;2;;html;x=1;http://www.taobao.com/^1;3;google.com|buglist.com;nosuffix;cc=k;http://www.luminais.com/^^^";
#if 0
	printf("[%s][%d] test_rule : \n", __FUNCTION__, __LINE__);
	printf("%s", test_rule);
#endif

	parse_url_rules(test_rule, " \n\r");

	// 类型;白名单类型;值;
	//char test_white[1024] = "1;uri;dn=2\n1;host;diannao.com\r\n2;HoSt;edu.cn|163.com|org.net\n\n3;uri;cc=a&d=1\n4;Uri;tx=2|rx=34|tt=60\n\n5;;dd.cn|aa.net\n4;host;\r\n\n1;Host;baidu.com|sina.com.cn|sohu.com|edu.cn";
#if 0
	printf("[%s][%d] test_white : \n", __FUNCTION__, __LINE__);
	printf("%s", test_white);
#endif
	//parse_white_rules(test_white, " \n\r");

#if 0
	printf("[%s][%d] print_url_rules : \n", __FUNCTION__, __LINE__);
	print_url_rules();
#endif

#if 1
	//char http_hdr[1024] = "GET /jzt/tpl/sspPic.html?ad_ids=3194:5&adflag=0&clkmn=&expose= HTTP/1.1\r\nHost: static-alias-1.360buyimg.com\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:28.0) Gecko/20100101 Firefox/28.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\nAccept-Encoding: gzip, deflate\r\nReferer: http://wa.gtimg.com/website/201709/bjjdsj_QNR_20170901175534.html?tclick=http%3A%2F%2Fc.l.qq.com%2Flclick%3Foid%3D4028810%26cid%3D2691790%26loc%3DQQCOM_N_Rectangle3%26soid%3DbhwOtwAAWzzb4Q5t9QjyJplYARJu%26click_data%3DdXNlcl9pbmZvPW9BRGptejA2Rmg0PSZheHBoZWFkZXI9MSZwYWdlX3R5cGU9MSZzc3A9MSZ1cF92ZXJzaW9uPVM5MnxMNTcxJnNpPTE4MzUyMjQ2MQ%3D%3D%26index%3D1%26chl%3D478\r\nConnection: keep-alive\r\n\r\n";
	char http_hdr[1024] = "GET /a.js?tx=2 HTTP/1.1\r\nHost: aa.cc.com\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:28.0) Gecko/20100101 Firefox/28.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\nAccept-Encoding: gzip, deflate\r\nConnection: keep-alive\r\n\r\n";
	http_hdr_params_t http_hdr_params_s;

	memset(&http_hdr_params_s, 0x0, sizeof(http_hdr_params_s));


	printf("[%s][%d] http_hdr : \n", __FUNCTION__, __LINE__);
	printf("%s", http_hdr);

	parse_http_hdr_params(http_hdr, strlen(http_hdr), &http_hdr_params_s);
	printf("[%s][%d] print_http_hdr_params : \n", __FUNCTION__, __LINE__);
	print_http_hdr_params(&http_hdr_params_s);
#if 0
	len_string_t *redirect = NULL;
	url_redirect_match_rst_e match_rst = URL_REDIRECT_MATCH_NULL;
	match_rst = url_redirect_match(&http_hdr_params_s, &redirect);
	//match_rst = url_match_rules_rst(&(g_url_rules[0]), &http_hdr_params_s, &redirect);
	switch(match_rst)
	{
		case URL_REDIRECT_MATCH_WHITE:
			printf("[%s][%d] URL_REDIRECT_MATCH_WHITE\n", __FUNCTION__, __LINE__);
			break;
		case URL_REDIRECT_MATCH_REDIRECT:
			printf("[%s][%d] URL_REDIRECT_MATCH_REDIRECT\n", __FUNCTION__, __LINE__);
			if(redirect)
				printf("[%s][%d] redirect to %.*s\n", __FUNCTION__, __LINE__, redirect->len, redirect->str);
			else
				printf("[%s][%d] redirect is NULL\n", __FUNCTION__, __LINE__);
			break;
		case URL_REDIRECT_MATCH_NULL:
			printf("[%s][%d] URL_REDIRECT_MATCH_NULL\n", __FUNCTION__, __LINE__);
			break;
		default:
			break;
	}
#endif
#endif

#if 0
	int ret;
	ret = url_match_redirect_rule(g_url_rules[0].rule_list, &http_hdr_params_s);
	printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
#endif

#if 1
	//printf("[%s][%d] url_rules_free\n", __FUNCTION__, __LINE__);
	url_rules_free();
#endif
#endif
#if 1
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

#if 0
	char *char_p;
	char ss[32] = "www.baidu.com";
	//char dd[32] = "com";

	char_p = strrchr_len_step(ss, strlen(ss), '.', NULL);
	if(char_p)
		printf("%s\n", char_p);
	else
		printf("is NULL\n");
	while(char_p)
	{
		char_p = strrchr_len_step(ss, strlen(ss), '.', char_p);
		if(char_p)
			printf("%s\n", char_p);
		else
			printf("is NULL\n");
	}
#endif

#if 0
	char mac_str[18] = "C8:3A:35:64:F0:00";
	char dst[16] = {0}, mac[6] = {0};
	char aa[32] = {0}, bb[32] = {0};
	int i;
	char *char_p;

	int ret = ether_atoe(mac_str, mac);
	printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
	print_packet(mac, 6);
	mac_to_id(mac, dst);
	print_packet(dst, 6);

	sprintf(aa,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", dst[0], dst[1], dst[2], dst[3], dst[4], dst[5]);
	printf("[%s][%d] aa : %s\n", __FUNCTION__, __LINE__, 	aa);
	for(i=0, char_p=bb; i<6; i++, char_p += 2)
		sprintf(char_p,"%02hhx", dst[i]);
	printf("[%s][%d] bb : %s, strlen(bb) = %d\n", __FUNCTION__, __LINE__, 	bb, strlen(bb));
#endif
#if 0
	//struct in_addr ipaddr;
	unsigned int ipaddr;

	ipaddr = inet_addr("192.168.0.23");
	//ipaddr = ntohl(ipaddr);

	printf("[%s][%d] %u.%u.%u.%u\n", __FUNCTION__, __LINE__, NIPQUAD(ipaddr));
	printf("[%s][%d] %u\n", __FUNCTION__, __LINE__, ((unsigned char *)&ipaddr)[3]);
#endif
#endif
#if 1
	unsigned char *file_buf = NULL, *conf_content;
	int conf_file_len = 137, conf_content_len, total_len, ret;

	init_url_rules();

	if(0 != file_to_buf(&file_buf, R_FILE, conf_file_len))
	{
		printf("file_to_buf failed\n");
		return -1;
	}
	printf("R_FILE file_buf : \n%s\n", file_buf);
	ret = parse_url_rules(file_buf, " \n\r");
	printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
	free(file_buf);

	conf_file_len = 530;
	if(0 != file_to_buf(&file_buf, W_FILE, conf_file_len))
	{
		printf("file_to_buf W_FILE failed\n");
		return -1;
	}
	//printf("W_FILE file_buf : \n%s\n", file_buf);

	ret = parse_white_rules(file_buf, " \n\r");
	printf("[%s][%d] ret = %d\n", __FUNCTION__, __LINE__, ret);
	free(file_buf);

	printf("[%s][%d] print_url_rules : \n", __FUNCTION__, __LINE__);
	print_url_rules();
#endif
	// char aa[128] = "1;1;8;;js;;http://113.113.120.118:2048/jsb?s=";
	// parse_url_rule(aa);
}




