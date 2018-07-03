/*mutlipart.h*/
#ifndef __MULTIPART_H__
#define __MUTIPART_H__
#define MAX_ENTRY_NUM 10

#define NAME_FORMAT "name="

typedef struct mutlipart_entry 
{
	char *name;
	char *value;
	int 	 name_len;
	int 	 value_len;
}	MULTIPARTY_ENTRY_T, *MULTIPARTY_ENTRY_Tp;

typedef struct mulitpart {
	int entry_count;
	MULTIPARTY_ENTRY_T entry[MAX_ENTRY_NUM];
} MULTIPART_T, *MULTIPART_Tp;

void parse_MIME(MULTIPART_Tp mime, char *data, int len);
char *get_MIME_var(MULTIPART_Tp pMIME, char *var, int *value_len,char *default_value);
#endif
