#ifndef __CGI_HANDLE_H__
#define __CGI_HANDLE_H__

#include "cgi_handle_module.h"


//此枚举与app中biz层定义的一致
typedef enum NETWORKTYPE {
	ADSL = 1,
	DYNAMIC,
	STATIC,
	L2TP,
	PPTP,
	DOUBLE_ADSL,
}E_NETWORK_T;

//detect type 
typedef enum DETECT_TYPE {
	NO_LINE = -2,
	DETECTING  = -1,
	DET_DHCP = 0,
	DET_STATIC= 1,
	DET_PPPOE = 2	
}DETECT_TYPE_T;


typedef struct cgi_option_info{
	PI8 *name;
	
	union
	{
		RET_INFO (*get_fun)(cJSON *recv_root,cJSON *send_root, void *info);
		RET_INFO (*set_fun)(cJSON *send_root,CGI_MSG_MODULE *msg,int *err_code,void *info);
	} action;

	
}CGI_OPTION_INFO;



#define APP_GET_FUN(node) (((node)->action).get_fun)
#define APP_SET_FUN(node) (((node)->action).set_fun)

#endif __CGI_HANDLE_H__*/

