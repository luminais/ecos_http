#ifndef __HTTP_H__
#define __HTTP_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#define MAX_USER_NUM  4		//最大登录页面用户数
#define MAX_LOGIN_OUT_TIME (5*60*RC_MODULE_1S_TIME)

typedef struct{
	PI8 ip[PI_IP_STRING_LEN];
	PIU32 time;
}login_ip_time; 

extern login_ip_time loginUserInfo[MAX_USER_NUM];

/*API*/

/*GPI*/

/*TPI*/
extern void tpi_http_clear_login_time();

extern RET_INFO tpi_http_action(RC_MODULES_COMMON_STRUCT *var);
extern RET_INFO tpi_http_struct_init();
extern RET_INFO tpi_http_first_init();
#endif/*__HTTP_H__*/
