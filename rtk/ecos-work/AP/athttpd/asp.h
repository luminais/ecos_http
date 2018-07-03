#ifndef _CYG_ATHHTTPD_ASP_H_
#define _CYG_ATHHTTPD_ASP_H_

#define FAILED (-1)
#define SUCCESS (0)

#define MAX_PATH_LEN	128
#define MAX_QUERY_TEMP_VAL_SIZE 1024
#define MAX_BOUNDRY_LEN 64
#define CYG_HTTPD_MAXCONTENTTYPE	256

typedef int (*asp_funcptr)(int argc, char **argv);
typedef void (*form_funcptr)(char* query, int len);

typedef struct asp_name_s
{
	char *name;
	asp_funcptr function;
} asp_name_t;

typedef struct form_name_s
{
	char *name;
	form_funcptr function;
} form_name_t;

typedef struct temp_mem_s
{
	char *str;
	struct temp_mem_s *next;
} temp_mem_t;
#if 0
static bool _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}
#endif

// Validate digit
static bool _isdigit(char c)
{
    return ((c >= '0') && (c <= '9'));
}
#if 0
static int __inline__ string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}
#endif
static int __inline__ string_to_dec(char *string, int *val)
{
	int idx;
	int len = strlen(string);

	for (idx=0; idx<len; idx++) {
		if ( !_isdigit(string[idx]))
			return 0;
	}

	*val = strtol(string, (char**)NULL, 10);
	return 1;
}

char *get_cstream_var(char *buf, int len, char *var, char *defaultGetValue);
char *get_mime_var(char *buf, int len, char *var, char *defaultGetValue);
/*may need hash table to accerate the asp execution---TODO*/


#endif    /*_CYG_ATHHTTPD_ASP_H_*/
