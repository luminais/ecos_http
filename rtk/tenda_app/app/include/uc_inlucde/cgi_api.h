#ifndef	__CGI_API_H__
#define __CGI_Api_H__

typedef enum {
	VER_TYPE_MIN = 0,
	VER_TYPE_SUCCESS = VER_TYPE_MIN,	//获取新版本信息成功
	VER_TYPE_NO_VER,					//服务器没有新版本
	VER_TYPE_NO_CUR_VER,                         //策略升级时，无法获取当前版本信息
	VER_TYPE_MAX
}ENUM_VER_TYPE;

typedef enum {
	UP_TYPE_MIN = 0,
	UP_TYPE_UPGRADE_Q = UP_TYPE_MIN,	//正在询问升级服务器
	UP_TYPE_MEM_NOT_ENOUGH,				//内存不足,无法升级
	UP_TYPE_WAIT_SVR,					//路由器在排队升级
	UP_TYPE_FW_DOWNING,					//路由器正在下载固件
	UP_TYPE_WRITING_TO_FLASH,			//路由器正在烧写固件
	UP_TYPE_MAX
}ENUM_UP_TYPE;

typedef enum {
	OPT_TYPE_MIN = 0,
	OPT_TYPE_OL_UPGRADE = 1,
	OPT_TYPE_OL_POLICY_UPGRADE = 4,
	OPT_TYPE_MAX
}OPT_TYPE;

//询问新版本时收到的回复数据
typedef struct query_version_s{
	ENUM_VER_TYPE	 type;
	char data[0];
}query_version_t;

//设置升级时收到的回复数据
typedef struct upgrade_s{
	ENUM_UP_TYPE  type;
	char data[0];
}upgrade_t;



enum {
	CL_UNKNOW = 0,									/*未知*/
	CL_MONITOR_SRV = 1, 							/*监控中心*/
	CL_DISP_SRV = 2,								/*分配服务器*/
	CL_DEV_SRV =3 ,									/*设备服务器*/
	CL_WEB_SRV = 4, 								/*Web服务器*/
	CL_APP_SRV = 5,									/*应用服务器*/
	CL_ROUTE = 6,									/*route 设备*/
	CL_APP = 7,										/*app*/
	CL_UPGRADE_SRV = 8,								/* Upgrade server */
	CL_SPEEDTEST_SRV = 9,							/* Speed test server */
	CL_APNS_SRV = 10,								/*app push notification server*/
	CL_CGI = 11,										/*cgi连接*/
	CL_INNER_PROCESS = 12,							/*内部实现进程*/
	CL_MAX,
};


typedef enum {
	ERR_MIN			= 0,
	ERR_NO_ERR		= ERR_MIN,
	ERR_UNKNOWN_ERR,				//未知错误
	ERR_JSON_TOO_LONG,				//JSON太长
	ERR_OUT_MEMORY,					//malloc失败,没内存
	ERR_CON_FAIL,					//连接ucloud失败
	ERR_SOCKET_ERR,					//socket错误
	ERR_SOCKET_TIMEOUT,				//socket select超时
	ERR_COMMON_ERR,					//统一 错误,一般都是命令在内部进程执行失败时回复
	ERR_INVALID_MODULE_CMD,			//收到不正确的模块或命令
	ERR_CMD_UNREGISTER,				//命令在ucloud中未被注册
	ERR_PARSE_FAILED,				//数据解析失败
	ERR_PACK_FAILED,				//数据打包失败
	ERR_CHECK_FAILED,				//数据检测失败
	ERR_LINK_FAILED, 				//ucloud连接内部实现进程失败
	ERR_SVR_CON_FAIL,				//连接服务器失败
	ERR_SVR_AUTH_FAIL,				//和服务器认证时失败
	ERR_MANAGE_DISABLE,				//未开启云管理
	ERR_CMD_FAIL,					//和云服务器连接时命令错误
	ERR_UCLOUD_BUSY,				//ucloud繁忙,正在升级或测速
	ERR_CONNECTING_SVR,				//正在连接服务器
	ERR_ACCOUNT_BIND_TOO_MANY_ROUTE //该帐号绑定了过多的路由器
}errno_t;

/************************************************
 * Function: 		cgi_ucloud_init
 * Description: 	初始化cgi同ucloud的连接
 * Input: 		
 * Output:		
 * Return: 		0:初始化成功
 * 				-1:初始化失败
************************************************/
int cgi_ucloud_init(void);
/************************************************
 * Function: 		cgi_ucloud_destory
 * Description: 	断开同ucloud连接,清空数据
 * Input: 			
 * Output:		
 * Return: 		
************************************************/
void cgi_ucloud_destory(void);
/************************************************
 * Function: 	c	gi_ucloud_query_version
 * Description: 	cgi通过ucloud询问新版本
 * Input: 		放置返回结果的地址
 * Output:		返回结果的类型及数据
 * Return: 		0:询问版本命令执行成功,可以
 * 				从结构体中拿到返回数据
 * 				-1:询问版本命令执行失败
************************************************/
int cgi_ucloud_query_version(query_version_t **version);
/************************************************
 * Function: 	cgi_ucloud_policy_query_version
 * Description: 	cgi通过ucloud询问特征库的新版本
 * Input: 		放置返回结果的地址
 * Output:		返回结果的类型及数据
 * Return: 		0:询问版本命令执行成功,可以
 * 				从结构体中拿到返回数据
 * 				-1:询问版本命令执行失败
************************************************/
int cgi_ucloud_policy_query_version(query_version_t **version);
/************************************************
 * Function: 		cgi_ucloud_upgrade_enable
 * Description: 	cgi通过ucloud执行在线升级
 * Input: 		放置返回结果的地址
 * Output:		返回结果的类型及数据
 * Return: 		0:在线升级命令执行成功,可以
 * 				从结构体中拿到返回数据
 * 				-1:在线升级命令执行失败
************************************************/
int cgi_ucloud_upgrade_enable(upgrade_t **upgrade);
/************************************************
 * Function: 		cgi_ucloud_upgrade_enable
 * Description: 	cgi通过ucloud执行在线策略升级
 * Input: 		放置返回结果的地址
 * Output:		返回结果的类型及数据
 * Return: 		0:在线升级命令执行成功,可以
 * 				从结构体中拿到返回数据
 * 				-1:在线升级命令执行失败
************************************************/
int cgi_ucloud_policy_upgrade_enable(upgrade_t **upgrade);
/************************************************
 * Function: 		cgi_ucloud_speedtest_enable
 * Description: 	cgi通过ucloud执行在线测速
 * Input: 		
 * Output:		
 * Return: 		0:在线测速命令执行成功
 * 				-1:在线测速命令执行失败
************************************************/
int cgi_ucloud_speedtest_enable(void);
/************************************************
 * Function: 		get_ucloud_errno
 * Description: 	命令执行失败时执行该函数
 * 				获取错误码
 * Input: 		
 * Output:		
 * Return: 		errno_t类型的错误码
************************************************/
errno_t get_ucloud_errno(void);

#if 1
int cgi_ucloud_login(char *in_json);
int cgi_ucloud_update_pwd(char *in_json);
int cgi_ucloud_login_sta_update();
#endif

/************************************************
 * Function: 		cgi_ucloud_sys_basic_info_get
 * Description: 	获取系统信息
 * Input: 		以下为系统模块命令的cgi调用函数,in
 * Output:		*out_json 指向数据缓冲区
 * Return: 		
 * 				
************************************************/
int cgi_ucloud_sys_basic_info_get(char **out_json, int *out_len);
int cgi_ucloud_sys_advance_info_get(char **out_json, int *out_len);
int cgi_ucloud_sys_reboot(char **out_json, int *out_len);
int cgi_ucloud_sys_reset(char **out_json, int *out_len);
int cgi_ucloud_sys_set_time_zone(char *in, char **out_json, int *out_len);
int cgi_ucloud_query_uc_account(char **out_json, int *out_len);
int cgi_ucloud_update_uc_account(char *in);
int cgi_ucloud_set_manage_en(char *in, char **out_json, int *out_len);
int cgi_ucloud_get_manage_en(char **out_json, int *out_len);
int cgi_ucloud_common_test();
#ifdef ECOS
int cgi_restart_connect_svr();
#endif

#endif

