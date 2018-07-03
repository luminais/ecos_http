#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_timer.h"
#include "debug.h"
#include "http.h"

login_ip_time loginUserInfo[MAX_USER_NUM];

static void tpi_http_init_login_timer();
static void tpi_http_update_login_time();
static RET_INFO tpi_http_start();
static RET_INFO tpi_http_stop();
static RET_INFO tpi_http_restart();

#ifdef __CONFIG_HTTPD__
extern void	httpd_start();
extern void httpd_stop();
#endif/* __CONFIG_HTTPD__ */


#ifdef __CONFIG_TENDA_HTTPD__
extern void tenda_httpd_start();
extern void tenda_httpd_stop();
#endif/* __CONFIG_TENDA_HTTPD__ */


/*以下函数用于api调用*/
RET_INFO tpi_http_struct_init()
{
	return RET_SUC;
}

RET_INFO tpi_http_first_init()
{
	tpi_http_init_login_timer();
	
	return tpi_http_start();
}

RET_INFO tpi_http_action(RC_MODULES_COMMON_STRUCT *var)
{
	PI_PRINTF(TPI,"op=%d\n",var->op);
	switch(var->op)
	{
		case OP_START:
			tpi_http_start();
			break;
		case OP_STOP:
			tpi_http_stop();
			break;
		case OP_RESTART:
			tpi_http_restart();
			break;
		default:
			PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
			break;
	}
	
	return RET_SUC;
}

/*以下用于gpi获取信息函数*/

/*以下为该模块具体执行实现函数*/

/*在http等其他线程可能会用到*/
void tpi_http_clear_login_time()
{
	PIU8 i = 0;
	
	for (i=0; i< MAX_USER_NUM; i++)
	{	
		memset(loginUserInfo[i].ip, 0, PI_IP_STRING_LEN);				
		loginUserInfo[i].time = 0;
	}

	return;
}

/*只有本文件里面用*/
static void tpi_http_init_login_timer()
{
    DO_TIMER_FUN timer;

    memset(&timer,0x0,sizeof(DO_TIMER_FUN));

	strcpy(timer.name,HTTP_LOGIN_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = tpi_http_update_login_time;
    sys_do_timer_add(&timer);
	
	return;
}

static void tpi_http_update_login_time()
{
	PIU32 tm;
	tm = (PIU32)cyg_current_time();
	PIU8 i = 0;

	for(i=0; i<MAX_USER_NUM; i++)
	{
		if(0 != loginUserInfo[i].time && (tm - loginUserInfo[i].time) > MAX_LOGIN_OUT_TIME)
		{
			PI_PRINTF(TPI,"http [%s] login time expired.\n", loginUserInfo[i].ip);
			memset(loginUserInfo[i].ip, 0x0, PI_IP_STRING_LEN);
			loginUserInfo[i].time = 0;
		}	
	}
	return;
}

static RET_INFO tpi_http_start()
{
#ifdef __CONFIG_HTTPD__
	httpd_start();
#endif/* __CONFIG_HTTPD__ */


#ifdef __CONFIG_TENDA_HTTPD__
	tenda_httpd_start();
#endif/* __CONFIG_TENDA_HTTPD__ */


	PI_PRINTF(TPI,"http start success!\n");
	return RET_SUC;
}

static RET_INFO tpi_http_stop()
{
#ifdef __CONFIG_HTTPD__
	httpd_stop();
#endif/* __CONFIG_HTTPD__ */

#ifdef __CONFIG_TENDA_HTTPD__
	tenda_httpd_stop();
#endif/* __CONFIG_TENDA_HTTPD__ */

	PI_PRINTF(TPI,"stop success!\n");	
	return RET_SUC;
}

static RET_INFO tpi_http_restart()
{
	RET_INFO ret = RET_SUC;
	
	if(RET_ERR == tpi_http_stop() || RET_ERR == tpi_http_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}
	
	return ret;
}
