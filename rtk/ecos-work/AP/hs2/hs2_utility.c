#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "hs2.h"

unsigned short convert_atob_sh(char *data, int base)
{
    char tmpbuf[10];
    unsigned short rbin;
    int bin;

    memcpy(tmpbuf, data, 4);
    tmpbuf[4]='\0';
    if (base == 16)
        sscanf(tmpbuf, "%04x", &bin);
    else
        sscanf(tmpbuf, "%04d", &bin);
    
    rbin = (((bin & 0xff) << 8) | ((bin & 0xff00) >>8));
    return rbin;
}

unsigned char convert_atob(char *data, int base)
{
    char tmpbuf[10];
    int bin;

    memcpy(tmpbuf, data, 2);
    tmpbuf[2]='\0';
    if (base == 16)
        sscanf(tmpbuf, "%02x", &bin);
    else
        sscanf(tmpbuf, "%02d", &bin);
    return((unsigned char)bin);
}

int isFileExist(char *file_name)
{
    struct stat status;

    if ( stat(file_name, &status) < 0)
        return 0;

    return 1;
}

char *get_token(char *data, char *token)
{
    char *ptr=data, *ptrbak=data;
    int len=0, idx=0;
	
	while ((*ptr == ' ')||(*ptr == '	'))
	{
		ptr++;
	}
	
	data = ptr;
    
	while (*ptr && *ptr != '\n' ) {
        if (*ptr == '=') {
            if (len <= 1)
			{
				data = ptrbak;
                return NULL;
			}
            memcpy(token, data, len);

            /* delete ending space */
            for (idx=len-1; idx>=0; idx--) {
                if (token[idx] !=  ' ')
                    break;
            }
            token[idx+1] = '\0';

			data = ptrbak;
            return ptr+1;
        }

		len++;
		ptr++;
    }

	data = ptrbak;
    return NULL;
}

int get_value(char *data, char *value)
{
    char *ptr=data;
    int len=0, idx, i;

    while (*ptr && *ptr != '\n' && *ptr != '\r') {
        len++;
        ptr++;
    }

    /* delete leading space */
    idx = 0;
    while (len-idx > 0) {
        if (data[idx] != ' ')
            break;
        idx++;
    }
    len -= idx;

    /* delete bracing '"' */
    if (data[idx] == '"') {
        for (i=idx+len-1; i>idx; i--) {
            if (data[i] == '"') {
                idx++;
                len = i - idx;
            }
            break;
        }
    }

    if (len > 0) {
        memcpy(value, &data[idx], len);
        value[len] = '\0';
    }
    return len;
}