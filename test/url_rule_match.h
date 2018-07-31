#ifndef __URL_RULE_MATCH_H__
#define __URL_RULE_MATCH_H__

#define diag_printf printf
//#define URL_REDIRECT_MATCH_DEBUG 1
typedef unsigned char	uint8;
typedef unsigned int	uint32;

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
			diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__); \
			return (ret); \
		} \
	}while(0)

#define LEN_STRING_INIT(ls) \
	do \
	{ \
		if(ls) \
		{ \
			(ls)->str = NULL; \
			(ls)->len = 0; \
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
	URL_REDIRECT_MATCH_WHITE = 0,
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
	uint32 max_times;
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
	uint32 max_times;
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

void init_url_rules(void);
void url_white_rules_free(void);
void url_match_rules_free(void);
void url_rules_free(void);
int parse_url_rules(char *url_rules, const char *delim);
int parse_white_rules(char *white_rules, const char *delim);
int parse_http_hdr_params(char *http_hdr,  int http_hdr_len, http_hdr_params_t *http_hdr_params_p);
url_redirect_match_rst_e url_redirect_match(http_hdr_params_t *http_hdr_params_p, url_match_rule_t **url_match_rule_p);
// int url_match_rule_handle(struct ifnet *ifp, char *head, struct mbuf *m);
// int nis_init_lanip(void);

#ifdef URL_REDIRECT_MATCH_DEBUG
void print_url_rules(void);
void print_url_rule_t(url_rule_t *url_rule_p);
void print_white_rule_t(white_rule_t *white_rule_p);
void print_len_string_list(len_string_list_t *len_string_list_p);
void print_url_rule(url_match_rule_t *rule_p);
void print_http_hdr_params(http_hdr_params_t *http_hdr_params_p);
#endif

// url match record update
#define MATCH_REDIRECT_ARRAY_SIZE (256)
#define MATCH_REFERER_ARRAY_SIZE (256)
#define MATCH_REFERER_BASE_LEN (8)
#define REFERER_RECORD_TIMEOUT (60*100)

typedef struct url_match_record
{
	struct url_match_record *next;
	unsigned int ipaddr;
	url_match_rule_t *matched;
	uint32 time;
	uint32 rd_times;
}url_match_record_t;

typedef struct referer_match_record
{
	struct referer_match_record *next;
	unsigned int ipaddr;
	uint32 time;
	int is_all;
	unsigned int len;
	int idx[2];
	char *arr[3];
}referer_match_record_t;

void url_record_post_init(void);
void url_match_record_free(void);

#endif /* __URL_RULE_MATCH_H__ */

