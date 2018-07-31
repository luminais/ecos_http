#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/mbuf.h>
#include <sys/stat.h>
// #include <net/if.h>
// #include <net/if_var.h>
// #include <net/route.h>
// #include <netinet/in.h>
// #include <netinet/in_var.h>
// #include <netinet/ip.h>
// #include <netinet/tcp.h>
// #include <netinet/tcp_fsm.h>
// #include <netinet/udp.h>
// #include <netinet/ip_var.h>
// #include <arpa/inet.h>
// #include <ip_compat.h>
// #include <ip_fil.h>
// #include <ip_nat.h>
// #include <ip_frag.h>
// #include <netdb.h>
// #include <router_net.h>
// #include <bcmnvram.h>
// #include <ctype.h>
// #include <time.h>
#include "jhash.h"
// #include "sdw_filter.h"
#include "url_rule_match.h"

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define NIPQUAD_FMT "%u.%u.%u.%u"

#define URLF_PASS	0
#define URLF_BLOCK	1

// unsigned char *tenda_arp_ip_to_mac(in_addr_t ip);

// extern char router_mac_save[32];

// cyg_mutex_t keep_conf_mutex;
url_match_record_t *g_match_record[MATCH_REDIRECT_ARRAY_SIZE] = {0};
referer_match_record_t *g_referfer_record[MATCH_REFERER_ARRAY_SIZE] = {0};

// static char htmhead[1024*2] = {0};
// static char g_url[MAX_FULL_URL_LEN] = {0};

enum {
	HTTP_OPTIONS = 0,
	HTTP_HEAD,
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_TRACE,
	HTTP_CONNECT,
	HTTP_METHOD_MAX,
};

struct http_method {
	char *method_name;
	int name_len;
};

url_match_rules_t g_url_rules[MAX_TYPE_NUM] = {0};
// struct in_addr addr_lan_ip = {0};
// char lan_ip[IP_LEN_16] = {0} ;

struct http_method http_methods[] = {
	{"OPTIONS ",8},
	{"HEAD ",5},
	{"GET ",4},
	{"POST ",5},
	{"PUT ",4},
	{"DELETE ",7},
	{"TRACE ",6},
	{"CONNECT ",8},
};

#if 1
void print_url_rule_t(url_rule_t *url_rule_p)
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	diag_printf("[%s][%d]\t type : %u\n", __FUNCTION__, __LINE__, url_rule_p->type);
	diag_printf("[%s][%d]\t time : %u\n", __FUNCTION__, __LINE__, url_rule_p->time);
	diag_printf("[%s][%d]\t max_times : %u\n", __FUNCTION__, __LINE__, url_rule_p->max_times);
	if(NULL == url_rule_p->host)
		diag_printf("[%s][%d]\t host : ALL\n", __FUNCTION__, __LINE__);
	else
		diag_printf("[%s][%d]\t host : %s\n", __FUNCTION__, __LINE__, url_rule_p->host);
	if(NULL == url_rule_p->suffix)
		diag_printf("[%s][%d]\t suffix : ALL\n", __FUNCTION__, __LINE__);
	else
		diag_printf("[%s][%d]\t suffix : %s\n", __FUNCTION__, __LINE__, url_rule_p->suffix);
	if(NULL == url_rule_p->uri)
		diag_printf("[%s][%d]\t uri : ALL\n", __FUNCTION__, __LINE__);
	else
		diag_printf("[%s][%d]\t uri : %s\n", __FUNCTION__, __LINE__, url_rule_p->uri);
	diag_printf("[%s][%d]\t redirect : %s\n", __FUNCTION__, __LINE__, url_rule_p->redirect);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
}

#define WHITE_TYPE_STR(type) \
	((type==WHITE_RULE_TYPE_HOST)?"host": \
	((type==WHITE_RULE_TYPE_URI)?"uri":"unknow"))

void print_white_rule_t(white_rule_t *white_rule_p)
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	diag_printf("[%s][%d]\t type : %u\n", __FUNCTION__, __LINE__, white_rule_p->type);
	diag_printf("[%s][%d]\t white_type : %s\n", __FUNCTION__, __LINE__, WHITE_TYPE_STR(white_rule_p->white_type));
	diag_printf("[%s][%d]\t value : %s\n", __FUNCTION__, __LINE__, white_rule_p->value);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
}

void print_len_string_list(len_string_list_t *len_string_list_p)
{
	int first = 1;
	while(len_string_list_p)
	{
		if(first)
		{
			//printf("%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
			diag_printf("%s", len_string_list_p->list.str);
			first = 0;
		}
		else
		{
			//printf("|%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
			diag_printf("|%s", len_string_list_p->list.str);
		}
		len_string_list_p = len_string_list_p->next;
	}
}

void print_url_rule(url_match_rule_t *rule_p)
{
	len_string_list_t *len_string_list_p;
	int j, first = 1;
	diag_printf("\t");
	diag_printf("%u;", rule_p->time);
	diag_printf("%u;", rule_p->max_times);
    for(j=0; j<URL_HOST_HASH_LEN; j++)
	{
		len_string_list_p = rule_p->host[j];
		while(len_string_list_p)
		{
			if(first)
			{
				//printf("%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
				diag_printf("%s", len_string_list_p->list.str);
				first = 0;
			}
			else
			{
				//printf("|%.*s", len_string_list_p->list.len, len_string_list_p->list.str);
				diag_printf("|%s", len_string_list_p->list.str);
			}
			len_string_list_p = len_string_list_p->next;
		}
	}
	diag_printf(";");
	if(rule_p->suffix.len > 0)
	{
		//printf("%.*s", rule_p->suffix.len, rule_p->suffix.str);
		diag_printf("%s", rule_p->suffix.str);
	}
	diag_printf(";");
	if(rule_p->uri.len > 0)
	{
		//printf("%.*s", rule_p->uri.len, rule_p->uri.str);
		diag_printf("%s", rule_p->uri.str);
	}
	diag_printf(";");
	if(rule_p->redirect.len > 0)
	{
		//printf("%.*s", rule_p->redirect.len, rule_p->redirect.str);
		diag_printf("%s", rule_p->redirect.str);
	}
	diag_printf("\n");
}

void print_url_rules(void)
{
	int i, j;
	url_match_rules_t *rules_p;
	url_match_rule_t *rule_p;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		diag_printf("[%s][%d] type %u : \n", __FUNCTION__, __LINE__, i+1);
		rules_p = &(g_url_rules[i]);
		diag_printf("\t<url rule> : \n");
		rule_p = rules_p->rule_list;
		while(rule_p)
		{
			print_url_rule(rule_p);
			rule_p = rule_p->next;
		}
		diag_printf("\t<white host> : \n");
		for(j=0; j<WHITE_HOST_HASH_LEN; j++)
		{
			if(rules_p->white_host[j])
			{
				diag_printf("\t");
				print_len_string_list(rules_p->white_host[j]);
				diag_printf("\n");
			}
		}
		diag_printf("\t<white uri> : \n");
		diag_printf("\t");
		print_len_string_list(rules_p->white_uri);
		diag_printf("\n");
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

int get_http_method(char *http_hdr)
{
	int ret = -1;

	if(NULL==http_hdr)
		return -1;

	switch(http_hdr[0])
	{
		case 'O':
			if(memcmp(http_hdr, http_methods[HTTP_OPTIONS].method_name, http_methods[HTTP_OPTIONS].name_len)==0)
			{
				ret = HTTP_OPTIONS;
			}
			break;
		case 'H':
			if(memcmp(http_hdr, http_methods[HTTP_HEAD].method_name, http_methods[HTTP_HEAD].name_len)==0)
			{
				ret = HTTP_HEAD;
			}
			break;
		case 'G':
			if(memcmp(http_hdr, http_methods[HTTP_GET].method_name, http_methods[HTTP_GET].name_len)==0)
			{
				ret = HTTP_GET;
			}
			break;
		case 'P':
			if(http_hdr[1] == 'O' && memcmp(http_hdr, http_methods[HTTP_POST].method_name, http_methods[HTTP_POST].name_len)==0)
			{
				ret = HTTP_POST;
			}
			else if(http_hdr[1] == 'U' && memcmp(http_hdr, http_methods[HTTP_PUT].method_name, http_methods[HTTP_PUT].name_len)==0)
			{
				ret = HTTP_PUT;
			}
			break;
		case 'D':
			if(memcmp(http_hdr, http_methods[HTTP_DELETE].method_name, http_methods[HTTP_DELETE].name_len)==0)
			{
				ret = HTTP_DELETE;
			}
			break;
		case 'T':
			if(memcmp(http_hdr, http_methods[HTTP_TRACE].method_name, http_methods[HTTP_TRACE].name_len)==0)
			{
				ret = HTTP_TRACE;
			}
			break;
		case 'C':
			if(memcmp(http_hdr, http_methods[HTTP_CONNECT].method_name, http_methods[HTTP_CONNECT].name_len)==0)
			{
				ret = HTTP_CONNECT;
			}
			break;
	}

	return ret;
}

void init_url_rules(void)
{
	int i, j;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		//memset(&(g_url_rules[i]), 0x0, sizeof(url_match_rules_t));
		g_url_rules[i].type = i+1;
		g_url_rules[i].rule_list = NULL;
		g_url_rules[i].white_uri = NULL;
		for(j=0; j<WHITE_HOST_HASH_LEN; j++)
			g_url_rules[i].white_host[j] = NULL;
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

	len_string_p->str = (char *)malloc(len+1);
	if(!len_string_p->str)
	{
		printf("[%s][%d] malloc failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	memcpy(len_string_p->str, str, len);
	len_string_p->str[len] = '\0';
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
	len_string_list_p->next = NULL;
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

void url_white_rules_free(void)
{
	int i, j;
	url_match_rules_t *rules_p;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		rules_p = &(g_url_rules[i]);
		for(j=0; j<WHITE_HOST_HASH_LEN; j++)
		{
			len_string_list_free(rules_p->white_host[j]);
			rules_p->white_host[j] = NULL;
		}
		len_string_list_free(rules_p->white_uri);
		rules_p->white_uri = NULL;
	}
}

void url_match_rules_free(void)
{
	int i;
	url_match_rules_t *rules_p;
	url_match_rule_t *rule_p;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		rules_p = &(g_url_rules[i]);
		rule_p = rules_p->rule_list;
		url_match_rule_free(rule_p);
		g_url_rules[i].rule_list = NULL;
	}
}

void url_all_rules_free(void)
{
	int i, j;
	url_match_rules_t *rules_p;
	url_match_rule_t *rule_p;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		rules_p = &(g_url_rules[i]);
		rule_p = rules_p->rule_list;
		url_match_rule_free(rule_p);
		g_url_rules[i].rule_list = NULL;
		for(j=0; j<WHITE_HOST_HASH_LEN; j++)
		{
			len_string_list_free(rules_p->white_host[j]);
			rules_p->white_host[j] = NULL;
		}
		len_string_list_free(rules_p->white_uri);
		rules_p->white_uri = NULL;
	}
}

int add_len_string_hash(len_string_list_t *list_hash[], u32 hash_len, char *value)
{
	char *char_p = NULL, *char_save = NULL;
	u32 hash_index;
	if(!list_hash || !value)
	{
		diag_printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	char_p = strtok_r(value, SAME_RULE_DELIM_STR, &char_save);
	while(char_p)
	{
#ifdef URL_REDIRECT_MATCH_DEBUG
		diag_printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
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

void url_match_rule_init(url_match_rule_t *url_match_rule_p)
{
	int i;
	if(!url_match_rule_p)
		return;
	for(i=0; i<URL_HOST_HASH_LEN; i++)
		url_match_rule_p->host[i] = NULL;
	url_match_rule_p->suffix.str = NULL;
	url_match_rule_p->suffix.len = 0;
	url_match_rule_p->uri.str = NULL;
	url_match_rule_p->uri.len = 0;
	url_match_rule_p->redirect.str = NULL;
	url_match_rule_p->redirect.len = 0;
	url_match_rule_p->next = NULL;

	return;
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
	url_match_rule_init(url_match_rule_p);

	url_match_rule_p->time = url_rule_p->time;
	url_match_rule_p->max_times = url_rule_p->max_times;

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
	diag_printf("[%s][%d] url_match_rule_new failed\n", __FUNCTION__, __LINE__);
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
		diag_printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
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
		diag_printf("[%s][%d] invalid param\n", __FUNCTION__, __LINE__);
		return -1;
	}

	memset(&url_rule_s, 0x0, sizeof(url_rule_s));
	// 1;1;8;6;;js;;http://113.113.120.118:2048/jsb?s=
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

	// max_times
	char_p = strchr(char_p, DIFF_RULE_DELIM_CHR);
	CHECK_NULL_RETURN(char_p, -1);
	char_p++;
	url_rule_s.max_times = (uint)atoi(char_p);

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
		return -1;
	url_rule_s.redirect = char_p;
#if 1 //def URL_REDIRECT_MATCH_DEBUG
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
		diag_printf("[%s][%d] [%s]\n", __FUNCTION__, __LINE__, char_p);
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
#ifdef URL_REDIRECT_MATCH_DEBUG
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
#ifdef URL_REDIRECT_MATCH_DEBUG
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
	LEN_STRING_INIT(len_string_p);
	len_string_p->str = http_hdr_find_field(http_hdr, http_hdr_len, HOST_STR, HOST_LEN);
	CHECK_NULL_RETURN(len_string_p->str, -1);
	len_string_p->str += HOST_LEN;
	char_p = strchr(len_string_p->str, '\r');
	CHECK_NULL_RETURN(char_p, -1);
	len_string_p->len = (int)(char_p - len_string_p->str);

	// uri
	len_string_p = &(http_hdr_params_p->uri);
	LEN_STRING_INIT(len_string_p);
	len_string_p->str = http_hdr + 4; // "GET "
	char_p = strchr(len_string_p->str, '\r');
	CHECK_NULL_RETURN(char_p, -1);
	char_p -= 9;
	if(0 != memcmp(char_p, " HTTP/1.", 8))
		return -1;
	len_string_p->len = (int)(char_p - len_string_p->str);

	// query
	char_q = strchr_len(len_string_p->str, len_string_p->len, '?');
	LEN_STRING_INIT(&(http_hdr_params_p->query));
	if(char_q)
	{
		query_off = (int)(char_p - char_q);
		len_string_p = &(http_hdr_params_p->query);
		len_string_p->str = char_q + 1;
		len_string_p->len = query_off - 1;
	}

	// suffix
	LEN_STRING_INIT(&(http_hdr_params_p->suffix));
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
	LEN_STRING_INIT(len_string_p);
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

int url_match_redirect_rules(url_match_rule_t *rule_list, http_hdr_params_t *http_hdr_params_p, url_match_rule_t **url_match_rule_pp)
{
	url_match_rule_t *url_match_rule_p = rule_list;
	if(!rule_list || !http_hdr_params_p || !url_match_rule_pp)
		return 0;

	while(url_match_rule_p)
	{
		if(1 == url_match_redirect_rule(url_match_rule_p, http_hdr_params_p))
		{
			*url_match_rule_pp = url_match_rule_p;
			return 1;
		}
		url_match_rule_p = url_match_rule_p->next;
	}

	return 0;
}

url_redirect_match_rst_e url_match_rules_rst(url_match_rules_t *url_match_rules_p, http_hdr_params_t *http_hdr_params_p, url_match_rule_t **url_match_rule_p)
{
	if(!url_match_rules_p || !http_hdr_params_p || !url_match_rule_p)
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
	if(1 == url_match_redirect_rules(url_match_rules_p->rule_list, http_hdr_params_p, url_match_rule_p))
		return URL_REDIRECT_MATCH_REDIRECT;
#ifdef URL_REDIRECT_MATCH_DEBUG
	printf("[%s][%d] rule_list not match\n", __FUNCTION__, __LINE__);
#endif
	return URL_REDIRECT_MATCH_NULL;
}

url_redirect_match_rst_e url_redirect_match(http_hdr_params_t *http_hdr_params_p, url_match_rule_t **url_match_rule_p)
{
	int i;
	url_redirect_match_rst_e match_rst = URL_REDIRECT_MATCH_NULL;
	url_match_rules_t *url_match_rules_p;

	if(!http_hdr_params_p || !url_match_rule_p)
		return URL_REDIRECT_MATCH_NULL;

	for(i=0; i<MAX_TYPE_NUM; i++)
	{
		url_match_rules_p = &(g_url_rules[i]);
		match_rst = url_match_rules_rst(url_match_rules_p, http_hdr_params_p, url_match_rule_p);
		if(URL_REDIRECT_MATCH_NULL != match_rst)
		{
			return match_rst;
		}
	}

	return match_rst;
}

// int nis_init_lanip(void)
// {
// 	char *tmp_value = NULL;
// 	//char tmp_mac_value[MAC_LEN_20] = {0} ;
// 	memset(&addr_lan_ip , 0, sizeof(addr_lan_ip));
// 	memset(lan_ip , 0, sizeof(lan_ip));
	
// 	tmp_value = nvram_safe_get("lan_ipaddr");
	
// 	strcpy(lan_ip, tmp_value);
// 	if(strlen(lan_ip) > 0)
// 	{
// 		if (0 == inet_aton(lan_ip, &addr_lan_ip)) 
// 		{
// 			printf("lan_ip inet_aton error,thread exit!!!\n");
// 			return 0;
// 		}
// 	}
	
// 	return 1 ;
// }

// int check_lan_ip(struct in_addr ip_src,struct in_addr ip_dst)
// {

// 	if (ip_src.s_addr==addr_lan_ip.s_addr || ip_dst.s_addr==addr_lan_ip.s_addr)
// 	{	
// 		return 1;
// 	}

// 	return 0;
// }

// static void http_init_302_pkt(char *url)
// {
// 	char htmbody[ARRAY_LEN_1024] = {0};

// 	int n = 0,len=0;
// 	char *p = htmbody;

// 	//printf("http_redirection_url [%s]\n",url);
	
// 	//build html body
// 	n = sprintf(p, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
// 	p = p + n;
// 	n = sprintf(p, "<html><head>\n");
// 	p = p + n;
// 	n = sprintf(p, "<title>302 Moved Temporarily</title>\n");
// 	p = p + n;
// 	n = sprintf(p, "</head><body>\n");
// 	p = p + n;
// 	n = sprintf(p, "<h1>Moved Temporarily</h1>\n");
// 	p = p + n;
// 	n = sprintf(p,"<p>The document has moved <a href=\"%s\">here</a>.</p>\n", url);
// 	p = p + n;
// 	n = sprintf(p, "<h1></body></html></h1>\n");
// 	p = p + n;
// 	n = sprintf(p, "\n");
// 	p = p + n;

// 	len = p - htmbody;

// 	p = htmhead;

// 	n = sprintf(p, "HTTP/1.1 302 Moved Temporarily\r\n");
// 	p = p+n;
// 	n = sprintf(p,"Location: %s\r\n", url);
// 	p = p+n;

// 	n= sprintf(p, "Content-Type: text/html; charset=iso-8859-1\r\n");
// 	p = p+n;
// 	n= sprintf(p,"Content-Length: %d\r\n", len);
// 	p = p+n;
// 	n= sprintf(p, "\r\n");
// 	p = p+n;
	
// 	n= sprintf(p, "%s", htmbody);

// }

// int return_http_redirection(struct mbuf *m , char *http_redirection_url)
// {
// 	struct tcphdr *tcph = NULL;
//     struct ip *ip = NULL;
// 	struct route ro;   

// 	int iphlen = 0;
// 	int off = 0,olen=0,nlen = 0;
// 	int inc = 0;
// 	unsigned long src_addr = 0 ,dest_addr = 0 ;
// 	unsigned short dest_port = 0 ;

// 	ip = mtod(m, struct ip *);
// 	if(ip == NULL)
// 	{
// 		return -1;
// 	}

// 	iphlen = ip->ip_hl << 2;
	
// 	http_init_302_pkt(http_redirection_url);
	
// 	tcph = (struct tcphdr *)((unsigned char*)ip + iphlen);
// 	if(tcph == NULL)
// 	{
// 		return -1;
// 	}

// 	off = iphlen + (tcph->th_off << 2);	
// 	olen = ntohs(ip->ip_len) -off;
// 	//printf("===========> \n\n %s \n\n" , htmhead);
// 	nlen = strlen(htmhead);
// 	inc = nlen - olen;
	
// //must do this
// 	if (inc < 0)
// 	{
// 		m_adj(m, inc);
// 	}
	
// //learn form ip_ftp_pxy.c->ippr_ftp_port()
// 	m_copyback(m, off, nlen, htmhead);

// 	src_addr = ip->ip_dst.s_addr;
// 	dest_addr = ip->ip_src.s_addr;

// 	bzero(ip,iphlen);

// 	//make presudo uip header,learn form l2tp_usrreq.c->ifl2tp_output()
// 	ip->ip_p = IPPROTO_TCP;
// 	//ip_len = tcp head len+ data_len
// 	ip->ip_len = htons((tcph->th_off << 2) + nlen);
// 	ip->ip_src.s_addr = src_addr ;
// 	ip->ip_dst.s_addr = dest_addr;


// 	dest_port = tcph->th_sport;
// 	tcph->th_sport = tcph->th_dport;
// 	tcph->th_dport = dest_port;

// 	src_addr = tcph->th_seq;
// 	dest_addr = tcph->th_ack;

// 	tcph->th_seq = dest_addr;
// 	tcph->th_ack = htonl(ntohl(src_addr)+olen);
// 	tcph->th_win = htons(ntohs(tcph->th_win) - nlen);

// 	//cksum = tcp_sum((unsigned short *)tcph, ip->ip_len, tcp_pseudo_sum(ip));

// 	tcph->th_sum = 0;//very important
//     	tcph->th_sum =in_cksum(m, iphlen+ntohs(ip->ip_len));
	

// 	ip->ip_len = iphlen + ntohs(ip->ip_len);
// 	ip->ip_ttl= 128;
// 	ip->ip_off|=IP_DF;

// 	bzero(&ro, sizeof ro);
	
// 	ip_output(m, 0, &ro, 0, 0);
	
// 	return 0;
// }

// char *make_redirect_url(unsigned int ipaddr, http_hdr_params_t *http_hdr_params_p, url_match_rule_t *url_match_rule_p, int suffix_js, int *is_malloc)
// {
// 	char *char_p, *char_q, *cli_mac;
	
// 	int tt_len;

// 	if(!http_hdr_params_p || !url_match_rule_p)
// 		return NULL;

// 	tt_len = http_hdr_params_p->host.len + http_hdr_params_p->uri.len + url_match_rule_p->redirect.len;
// 	if(suffix_js)
// 		tt_len += 34;

// 	if(tt_len>MAX_FULL_URL_LEN)
// 	{
// 		char_p = (char *)malloc(tt_len+1);
// 		if(!char_p)
// 			return NULL;
// 		*is_malloc = 1;
// 	}
// 	else
// 	{
// 		char_p = g_url;
// 		*is_malloc = 0;
// 	}
// 	char_p[tt_len] = '\0';
// 	char_q = char_p;

// 	memcpy(char_p, url_match_rule_p->redirect.str, url_match_rule_p->redirect.len);
// 	char_p += url_match_rule_p->redirect.len;
// 	memcpy(char_p, http_hdr_params_p->host.str, http_hdr_params_p->host.len);
// 	char_p += http_hdr_params_p->host.len;
// 	memcpy(char_p, http_hdr_params_p->uri.str, http_hdr_params_p->uri.len);
// 	char_p += http_hdr_params_p->uri.len;
// 	if(suffix_js)
// 	{
// 		// &tid=0e8d593c0032&rid=0e8d593c0032
// 		cli_mac = tenda_arp_ip_to_mac(ipaddr);
// 		if(NULL == cli_mac)
// 		{
// 			tt_len -= 34;
// 			char_q[tt_len] = '\0';
// 		}
// 		else
// 		{
// 			memcpy(char_p, "&tid=", 5);
// 			char_p += 5;
// 			sprintf(char_p,"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", cli_mac[0], cli_mac[1], cli_mac[2], cli_mac[3], cli_mac[4], cli_mac[5]);
// 			char_p += 12;
// 			memcpy(char_p, "&rid=", 5);
// 			char_p += 5;
// 			memcpy(char_p, router_mac_save, 12);
// 			char_p += 12;
// 			*char_p = '\0';
// 		}
// 	}

// 	printf("[%s][%d] redirect_url = %.*s\n", __FUNCTION__, __LINE__, tt_len, char_q);
// 	return char_p;
// }

// int url_match_do_redirect(struct mbuf *m, unsigned int ipaddr, http_hdr_params_t *http_hdr_params_p, url_match_rule_t *url_match_rule_p, int suffix_js)
// {
// 	int is_malloc = 0, ret;
// 	char *char_p;

// 	if(!http_hdr_params_p || !url_match_rule_p)
// 		return -1;

// 	char_p = make_redirect_url(ipaddr, http_hdr_params_p, url_match_rule_p, suffix_js, &is_malloc);
// 	if(NULL == char_p)
// 		return -1;

// 	ret = return_http_redirection(m, char_p);

// 	if(is_malloc)
// 		free(char_p);

// 	return ret;
// }

// void match_redirect_record_update(void)
// {
// 	int i;
// 	url_match_record_t *url_match_record_p;
// 	uint32 tm;
// 	tm = (uint32)cyg_current_time();

// 	for(i=0; i<MATCH_REDIRECT_ARRAY_SIZE; i++)
// 	{
// 		url_match_record_p = g_match_record[i];
// 		while(url_match_record_p != NULL)
// 		{
// 			if(tm >= url_match_record_p->time)
// 			{
// 				g_match_record[i] = url_match_record_p->next;
// 				free(url_match_record_p);
// 				url_match_record_p = g_match_record[i];
// 			}
// 			else
// 			{
// 				break;
// 			}
// 		}
// 	}
// }

// void free_referer_match_record(referer_match_record_t *referer_match_record_p)
// {
// 	int i;
// 	if(!referer_match_record_p)
// 		return;

// 	if(referer_match_record_p->is_all != 1)
// 	{
// 		for(i=0; i<3; i++)
// 		{
// 			if(referer_match_record_p->arr[i])
// 				free(referer_match_record_p->arr[i]);
// 		}
// 	}
// 	free(referer_match_record_p);

// 	return;
// }

// void referer_match_record_update(void)
// {
// 	int i;
// 	referer_match_record_t *referer_match_record_p;
// 	uint32 tm;
// 	tm = (uint32)cyg_current_time();

// 	for(i=0; i<MATCH_REFERER_ARRAY_SIZE; i++)
// 	{
// 		referer_match_record_p = g_referfer_record[i];
// 		while(referer_match_record_p != NULL)
// 		{
// 			if(tm >= referer_match_record_p->time)
// 			{
// 				g_referfer_record[i] = referer_match_record_p->next;
// 				free_referer_match_record(referer_match_record_p);
// 				referer_match_record_p = g_referfer_record[i];
// 			}
// 			else
// 			{
// 				break;
// 			}
// 		}
// 	}
// }

// referer_match_record_t *referer_match_record_new(unsigned int ipaddr, len_string_t *referer_p)
// {
// 	referer_match_record_t *referer_match_record_p = NULL;
// 	char *char_p;
// 	uint32 tm;

// 	if(!referer_p)
// 		return NULL;

// 	referer_match_record_p = (referer_match_record_t *)lm_malloc(sizeof(referer_match_record_t));
// 	if(referer_match_record_p)
// 	{
// 		if(referer_p->len <= 3*MATCH_REFERER_BASE_LEN)
// 		{
// 			char_p = (char *)malloc(referer_p->len);
// 			if(NULL == char_p)
// 				goto referer_match_record_new_failed;
// 			memcpy(char_p, referer_p->str, referer_p->len);
// 			(referer_match_record_p->arr)[0] = char_p;
// 			referer_match_record_p->is_all = 1;
// 		}
// 		else
// 		{
// 			(referer_match_record_p->idx)[0] = referer_p->len/2 - MATCH_REFERER_BASE_LEN/2;
// 			(referer_match_record_p->idx)[1] = referer_p->len - MATCH_REFERER_BASE_LEN;
// 			char_p = (char *)malloc(MATCH_REFERER_BASE_LEN);
// 			if(NULL == char_p)
// 				goto referer_match_record_new_failed;
// 			memcpy(char_p, referer_p->str, MATCH_REFERER_BASE_LEN);
// 			(referer_match_record_p->arr)[0] = char_p;

// 			char_p = (char *)malloc(MATCH_REFERER_BASE_LEN);
// 			if(NULL == char_p)
// 				goto referer_match_record_new_failed;
// 			memcpy(char_p, &(referer_p->str[(referer_match_record_p->idx)[0]]), MATCH_REFERER_BASE_LEN);
// 			(referer_match_record_p->arr)[1] = char_p;

// 			char_p = (char *)malloc(MATCH_REFERER_BASE_LEN);
// 			if(NULL == char_p)
// 				goto referer_match_record_new_failed;
// 			memcpy(char_p, &(referer_p->str[(referer_match_record_p->idx)[1]]), MATCH_REFERER_BASE_LEN);
// 			(referer_match_record_p->arr)[2] = char_p;

// 			referer_match_record_p->is_all = 0;
// 		}
// 		referer_match_record_p->ipaddr = ipaddr;
// 		referer_match_record_p->len = referer_p->len;
// 		tm = (uint32)cyg_current_time();
// 		referer_match_record_p->time = tm + REFERER_RECORD_TIMEOUT;
// 	}

// 	return referer_match_record_p;
// referer_match_record_new_failed:
// 	free_referer_match_record(referer_match_record_p);
// 	return NULL;
// }

// url_match_record_t *url_match_record_new(unsigned int ipaddr, url_match_rule_t *url_match_rule_p)
// {
// 	url_match_record_t *url_match_record_p;
// 	uint32 tm;

// 	if(!url_match_rule_p)
// 		return NULL;

// 	url_match_record_p = (url_match_record_t *)lm_malloc(sizeof(url_match_record_t));
// 	if(url_match_record_p)
// 	{
// 		url_match_record_p->ipaddr = ipaddr;
// 		url_match_record_p->matched = url_match_rule_p;
// 		tm = (uint32)cyg_current_time();
// 		url_match_record_p->time = tm + 100 * url_match_rule_p->time;
// 	}

// 	return url_match_record_p;
// }

// void url_match_record_insert(url_match_record_t **head, url_match_record_t *new_p)
// {
// 	url_match_record_t *p, *q;
// 	if(*head == NULL)
// 	{
// 		*head = new_p;
// 		return;
// 	}

// 	p = *head;
// 	while(new_p->time >= p->time && p->next != NULL)
// 	{
// 		q = p;
// 		p = p->next;
// 	}

// 	if(new_p->time < p->time)
// 	{
// 		if(p == *head)
// 		{
// 			*head = new_p;
// 			new_p->next = p;
// 		}
// 		else
// 		{
// 			q->next = new_p;
// 			new_p->next = p;
// 		}
// 	}
// 	else
// 	{
// 		p->next = new_p;
// 	}
// }

// int url_match_record_check(unsigned int ipaddr, len_string_t *referer, url_match_rule_t *url_match_rule_p, int suffix_js)
// {
// 	int idx;
// 	referer_match_record_t *referer_match_record_p, *referer_match_record_q = NULL;
// 	url_match_record_t *url_match_record_p, *url_match_record_q;

// 	if(!referer || !url_match_rule_p)
// 		return -1;

// 	printf("[%s][%d] %u.%u.%u.%u\n", __FUNCTION__, __LINE__, NIPQUAD(ipaddr));
// 	printf("[%s][%d] %u\n", __FUNCTION__, __LINE__, ((unsigned char *)&ipaddr)[3]);
// 	idx = ((unsigned char *)&ipaddr)[3];
// 	printf("[%s][%d] idx = %d\n", __FUNCTION__, __LINE__, idx);

// 	// referfer check
// 	if(suffix_js && referer->len != 0)
// 	{
// 		referer_match_record_p = g_referfer_record[idx];
// 		while(referer_match_record_p != NULL)
// 		{
// 			referer_match_record_q = referer_match_record_p;
// 			if(ipaddr == referer_match_record_p->ipaddr && referer->len == referer_match_record_p->len)
// 			{
// 				if(referer_match_record_p->is_all)
// 				{
// 					if(0 == memcmp(referer->str, (referer_match_record_p->arr)[0], referer->len))
// 						return 1;
// 				}
// 				else
// 				{
// 					if(0 == memcmp(referer->str, (referer_match_record_p->arr)[0], MATCH_REFERER_BASE_LEN)
// 						&& 0 == memcmp(&((referer->str)[(referer_match_record_p->idx)[0]]), (referer_match_record_p->arr)[1], MATCH_REFERER_BASE_LEN)
// 						&& 0 == memcmp(&((referer->str)[(referer_match_record_p->idx)[1]]), (referer_match_record_p->arr)[2], MATCH_REFERER_BASE_LEN))
// 						return 1;
// 				}
// 			}
// 			referer_match_record_p = referer_match_record_p->next;
// 		}
// 		referer_match_record_p = referer_match_record_new(ipaddr, referer);
// 		if(referer_match_record_p)
// 		{
// 			if(referer_match_record_q)
// 				referer_match_record_q->next = referer_match_record_p;
// 			else
// 				g_referfer_record[idx] = referer_match_record_p;
// 		}
// 	}

// 	// redirect record check

// 	url_match_record_p = g_match_record[idx];
// 	while(url_match_record_p != NULL)
// 	{
// 		if(url_match_record_p->ipaddr == ipaddr && url_match_record_p->matched == url_match_rule_p)
// 			return 1;
// 		url_match_record_p = url_match_record_p->next;
// 	}

// 	url_match_record_q = url_match_record_new(ipaddr, url_match_rule_p);
// 	if(url_match_record_q)
// 	{
// 		url_match_record_insert(&(g_match_record[idx]), url_match_record_q);
// 	}
// 	return 0;
// }

// void url_match_record_free(void)
// {
// 	int i;
// 	referer_match_record_t *p, *q;
// 	url_match_record_t *url_match_record_p, *url_match_record_q;

// 	for(i=0; i<MATCH_REFERER_ARRAY_SIZE; i++)
// 	{
// 		p = g_referfer_record[i];
// 		while(p != NULL)
// 		{
// 			q = p->next;
// 			free_referer_match_record(p);
// 			p = q;
// 		}
// 	}

// 	for(i=0; i<MATCH_REDIRECT_ARRAY_SIZE; i++)
// 	{
// 		url_match_record_p = g_match_record[i];
// 		while(url_match_record_p != NULL)
// 		{
// 			url_match_record_q = url_match_record_p->next;
// 			free(url_match_record_p);
// 			url_match_record_p = url_match_record_q;
// 		}
// 	}
// }

// void url_match_record_update(void)
// {
// 	memset(g_match_record, 0x0, sizeof(g_match_record));
// 	diag_printf("[%s][%d] url_match_record_update start\n", __FUNCTION__, __LINE__);

// 	for (;;) 
// 	{
// 		if(cyg_mutex_lock(&keep_conf_mutex))
// 		{
// 			match_redirect_record_update();
// 			referer_match_record_update();
// 			cyg_mutex_unlock(&keep_conf_mutex);
// 		}
// 		cyg_thread_delay(100);
// 	}
// 	return;
// }

// static cyg_handle_t url_match_record_thread_h;
// static cyg_uint8 url_match_record_stack[65536];
// static cyg_thread url_match_record_thread;

// void url_record_post_init(void)
// {
// 	if (0 == url_match_record_thread_h)
// 	{
// 		cyg_thread_create(
// 			10,
// 			(cyg_thread_entry_t *)url_match_record_update,
// 			0,
// 			"url_match_record",
// 			url_match_record_stack,
// 			sizeof(url_match_record_stack),
// 			&url_match_record_thread_h,
// 			&url_match_record_thread);
// 		cyg_thread_resume(url_match_record_thread_h);
// 	}
	
// 	return;
// }

// int url_match_rule_handle(struct ifnet *ifp, char *head, struct mbuf *m)
// {
// 	struct ip *ip = NULL;
// 	struct tcphdr *tcp = NULL ;
// 	char *data = NULL;
// 	char *char_p = NULL, *char_q = NULL, *http_hdr_end = NULL;
// 	int hdr_left_len = 0, ret = 0, suffix_js = 0;
// 	int pk_len = 0 ;
// 	int d_method = -1;
// 	unsigned int ipaddr;
// 	http_hdr_params_t http_hdr_params_s;
// 	url_match_rule_t *url_match_rule_p;
// 	len_string_t *redirect = NULL;
// 	url_redirect_match_rst_e match_rst = URL_REDIRECT_MATCH_NULL;

// 	if(head == NULL || ifp == NULL || m == NULL)
// 	{
// 		return URLF_PASS;
// 	}

// 	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) == 0)
// 	{
// 		return URLF_PASS;
// 	}
	
// 	if (m->m_pkthdr.len < 40)
// 	{
// 		return URLF_PASS;
// 	}
// 	ip = mtod(m, struct ip *);
// 	if(ip == NULL)
// 	{
// 		return URLF_PASS ;
// 	}

// 	if (ip->ip_p != IPPROTO_TCP)
// 	{
// 		return URLF_PASS ;
// 	}

// 	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));
// 	if(tcp == NULL)
// 	{
// 		return URLF_PASS ;
// 	}

// 	if (tcp->th_flags & (TH_RST | TH_FIN | TH_SYN))
// 		return URLF_PASS;
	
// 	if((htons(80) == tcp->th_dport)/*|| (htons(80) == tcp->th_sport)*/)
// 	{
// 		;
// 	}
// 	else
// 		return URLF_PASS;
	
// 	data = (char *)tcp + (tcp->th_off << 2);
	
// 	if(data == NULL)
// 	{
// 		return URLF_PASS ;
// 	}
// 	pk_len = m->m_pkthdr.len - (tcp->th_off << 2) - (ip->ip_hl << 2);

// 	if(pk_len <= 5 )
// 	{
// 		return URLF_PASS ;
// 	}
	
// 	if (check_lan_ip(ip->ip_src,ip->ip_dst))
// 	{
// 		return URLF_PASS;
// 	}

// 	d_method = get_http_method(data);
// 	if(-1 == d_method)
// 	{
// 		return URLF_PASS;
// 	}

// 	char_q = strchr(data, '\r');
// 	if(NULL==char_q)
// 	{
// 		return URLF_PASS;
// 	}
// 	char_q -= 8;
// 	if(0 != memcmp(char_q, "HTTP/1.", 7))
// 	{
// 		return URLF_PASS;
// 		return -1;
// 	}

// 	char_p = strstr(data, "\r\n\r\n");
// 	if(NULL==char_p)
// 	{
// 		return URLF_PASS;
// 	}
// 	http_hdr_end = char_p;
// 	hdr_left_len = (int)(char_p - data);
// #if 1
// 	printf("[%s][%d] http header :\n", __FUNCTION__, __LINE__);
// 	printf("%.*s\n", hdr_left_len, data);
// #endif

// 	memset(&http_hdr_params_s, 0x0, sizeof(http_hdr_params_s));
// 	parse_http_hdr_params(data, hdr_left_len, &http_hdr_params_s);
// #if 1
// 	printf("[%s][%d] ip_src %u.%u.%u.%u\n", __FUNCTION__, __LINE__, NIPQUAD(ip->ip_src.s_addr));
// 	printf("[%s][%d] ip_dst %u.%u.%u.%u\n", __FUNCTION__, __LINE__, NIPQUAD(ip->ip_dst.s_addr));
// 	printf("[%s][%d] print_http_hdr_params : \n", __FUNCTION__, __LINE__);
// 	print_http_hdr_params(&http_hdr_params_s);
// #endif

// #if 0
// 	return_http_redirection(m, "http://192.168.0.1/index.html");
// 	return URLF_BLOCK;
// #else
// 	match_rst = url_redirect_match(&http_hdr_params_s, &url_match_rule_p);
// 	if(URL_REDIRECT_MATCH_REDIRECT == match_rst && url_match_rule_p)
// 	{
// 		printf("[%s][%d] redirect to %.*s\n", __FUNCTION__, __LINE__, url_match_rule_p->redirect.len, url_match_rule_p->redirect.str);
// 		if(http_hdr_params_s.suffix.len==2 && 0 == memcmp(http_hdr_params_s.suffix.str, "js", 2))
// 			suffix_js = 1;
// 		ipaddr = (unsigned int)(ip->ip_src.s_addr);
// 		if(1 == url_match_record_check(ipaddr, &(http_hdr_params_s.referer), url_match_rule_p, suffix_js))
// 		{
// 			return URLF_PASS;
// 		}
// 		if(0 == url_match_do_redirect(m, ipaddr, &http_hdr_params_s, url_match_rule_p, suffix_js))
// 			return URLF_BLOCK;
// 	}
// 	return URLF_PASS;
// #endif
// }

