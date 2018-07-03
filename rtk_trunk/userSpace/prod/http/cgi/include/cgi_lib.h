#ifndef __CGI_LIB_H__
#define __CGI_LIB_H__
#include "flash_cgi.h"
#include "cgi_lib_config.h"


/*******************************interface func************************************/
RET_INFO cgi_lib_get(CGI_LIB_INFO get_info, void *info);
RET_INFO cgi_lib_set(CGI_LIB_INFO set_info,CGI_MSG_MODULE *msg, char *err_code, void *info);
char_t *	 cgi_lib_get_var(webs_t wp,cJSON*root, char_t *var, char_t *defaultGetValue);
/******************************************************************************/



#define FREE_P(p) if(*p) \
    { \
        free(*p); \
        *p=NULL; \
    }
#endif __CGI_LIB_H__*/
