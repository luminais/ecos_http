#include "process_api.h"
#include "cgi_api.h"
#include <ecos_oslib.h>
#include "cJSON.h"

/*
 *  在线升级特殊错误码
 */
typedef enum {
	ERR_UC_UCLOUD_BUSY = 19, //ucloud繁忙，需要等待
	ERR_UC_UCLOUD_DOWNLOAD_FAIL = 14 //下载失败，连接已断开
} err_uc_ucloud_t;

#define UC_GET_VERSION 				"uc_get_version"
#define UC_GET_DOWNLOAD_PROGRESS    "uc_get_download_progress"
#define UC_SET_LOGIN_STA_UPDATE     "uc_set_login_sta_update"
#define UC_SET_RESTART_CONNECT_SVR "restart_connect_svr"
#define BIZ_LIGHT_BLUE  "\033[1;34m"
#define BIZ_FMT_NONE    "\033[m"

#if 0
#define BIZ_DEBUG(fmt, arg...) \
		fprintf(stdout, "[ %s ] [ %d ] [ %s ]:" \
		BIZ_LIGHT_BLUE fmt BIZ_FMT_NONE, basename(__FILE__), __LINE__, __FUNCTION__, ##arg);
#else

#define BIZ_DEBUG(fmt, arg...)

#endif

extern errno_t get_ucloud_errno(void); /* 获取信息失败时，调用该函数获取错误码 */

/*************************************************************************
  功能: 从服务器获取新版本
  参数: 无
  返回值: NULL未获取到数据，非空获取到数据
  注意: 调用该函数成功后请free掉version
 ************************************************************************/
static query_version_t * ucloud_query_version(void)
{
	query_version_t *version = NULL;

	/* 初始化cgi同ucloud的连接 */
	if (-1 != cgi_ucloud_init()) {
		if (-1 == cgi_ucloud_query_version(&version)) {
			if (version) {
				free(version);
				version = NULL;
			}
		}
	}
	
	/*断开同ucloud连接,清空数据*/
	cgi_ucloud_destory();
	return version;
}

/*************************************************************************
  功能: 开始升级，并获取升级服务器状态
  参数: 无
  返回值: NULL未获取到数据，非空获取到数据
  注意: 调用该函数成功后请free掉upgrade
 ************************************************************************/
static upgrade_t * ucloud_query_upgrade(void)
{
	upgrade_t *upgrade = NULL;
	
	/* 初始化cgi同ucloud的连接 */
	if (-1 != cgi_ucloud_init()) {
		if (-1 == cgi_ucloud_upgrade_enable(&upgrade)) {
			if (upgrade) { /* 调用失败之后释放掉 */
				free(upgrade);
				upgrade = NULL;
			}
		}
	}
	
	/*断开同ucloud连接,清空数据*/
	cgi_ucloud_destory();
	return upgrade;
}

/*************************************************************************
  功能: 更新登录密码状态
  参数: 无
  返回值: 0成功，1失败
  注意: 调用cgi函数时，必须先cgi_ucloud_init再cgi_ucloud_destory
 ************************************************************************/
static int ucloud_login_sta_update(void)
{
	int ret;
	
	/* 初始化cgi同ucloud的连接 */
	if (-1 != cgi_ucloud_init()) {
		ret = cgi_ucloud_login_sta_update();
	} else {
		ret = 1;
	}
	
	/*断开同ucloud连接,清空数据*/
	cgi_ucloud_destory();
	return ret;
}
#ifdef CONFIG_APP_COSTDOWN
/*************************************************************************
  功能: 重启云服务
  参数: 无
  返回值: 0成功，1失败
  注意: 调用cgi函数时，必须先cgi_ucloud_init再cgi_ucloud_destory
 ************************************************************************/
static int ucloud_restart_connect_svr(void)
{
	int ret;
	
	/* 初始化cgi同ucloud的连接 */
	if (-1 != cgi_ucloud_init()) {
		ret = cgi_restart_connect_svr();
	} else {
		ret = 1;
	}
	
	/*断开同ucloud连接,清空数据*/
	cgi_ucloud_destory();
	return ret;
}
#endif

static void copy_object(cJSON *src, cJSON *dst, const char *name)
{
	cJSON *new_obj = cJSON_Duplicate(cJSON_GetObjectItem(src, name), 1);
	cJSON_AddItemToObject(dst, name, new_obj);
}

static void copy_version_data(cJSON *src, cJSON *dst)
{
	copy_object(src, dst, "newest_ver");
	copy_object(src, dst, "update_date");
	copy_object(src, dst, "fw_size");
	copy_object(src, dst, "description");
	copy_object(src, dst, "description_en");
	copy_object(src, dst, "description_zh_tw");
}

static void copy_download_proccess_data(cJSON *src, cJSON *dst)
{
	copy_object(src, dst, "fw_size");
	copy_object(src, dst, "recved");
	copy_object(src, dst, "sec_left");
}

static err_uc_ucloud_t download_proccess_get_errno(void)
{
	errno_t ucloud_err = get_ucloud_errno();
	BIZ_DEBUG("==>ucloud_err [%d]\n", ucloud_err)
	if (ucloud_err == 19) {
		BIZ_DEBUG("==> ERR_UC_UCLOUD_BUSY [%d]\n", ERR_UC_UCLOUD_BUSY);
		return ERR_UC_UCLOUD_BUSY;
	} else {
		BIZ_DEBUG("==> ERR_UC_UCLOUD_DOWNLOAD_FAIL [%d]\n", ERR_UC_UCLOUD_DOWNLOAD_FAIL);
		return ERR_UC_UCLOUD_DOWNLOAD_FAIL;
	}
}

static int module_cmd_get_version(cJSON *cj_get)
{
	query_version_t *version = NULL;
	
	if ((version = ucloud_query_version())) {
		switch (version->type) {
			case VER_TYPE_SUCCESS: 	/* 有新版本 */
				BIZ_DEBUG("new version %s\n", version->data);
				cJSON * data = cJSON_Parse(version->data);
				copy_version_data(data, cj_get);
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(VER_TYPE_SUCCESS));
				cJSON_Delete(data);
				//
				char * p = cJSON_Print(cj_get);
				BIZ_DEBUG("cj_get: %s\n", p);
				free(p);

				
				/*{
				        "newest_ver":   "V3.0.0.0(89)_cn-circle",
				        "update_date":  "2015-12-26",
				        "fw_size":      1425408,
				        "description":  ["V3.0.0.0(89)_cn-circle\n", "V3.0.0.0(89)_cn\r\n", "ECOS测试版本3\r\n"],
				        "description_en":       ["V3.0.0.0(89)_cn-circle\n", "V3.0.0.0(89)_cn\r\n", "test\r\n"],
				        "description_zh_tw":    ["V3.0.0.0(89)_cn-circle\n", "V3.0.0.0(89)_cn\r\n", "測試\r\n"]
				}*/
				// 以上是version->data获取的数据，是json对象的字符串。
				// newest_ver 表示新版本
				// update_date 新镜像发布的时间
				// fw_size 表示镜像的大小
				// description 中文简体描述
				// description_en 英文描述
				// description_zh_tw 中文简体描述
				break;
			case VER_TYPE_NO_VER:				
				BIZ_DEBUG("no new version\n"); 	/* 没有新版本 */
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(VER_TYPE_NO_VER));
				break;

			case VER_TYPE_NO_CUR_VER: /* 这里不会用到，只关心VER_TYPE_SUCCESS和VER_TYPE_NO_VER */
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(VER_TYPE_NO_CUR_VER));

				break;
				
			default:
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(ERR_UC_UCLOUD_BUSY));
				break;
		}

		free(version);
	} else {
		cJSON_AddItemToObject(cj_get, "error_code", 
							  cJSON_CreateNumber(ERR_UC_UCLOUD_BUSY));
		BIZ_DEBUG("get upgrade fail, code [%d]\n",
				   cJSON_GetObjectItem(cj_get, "error_code")->valueint); /* 获取版本信息失败 */

	}
	return 0;
}

static int module_cmd_get_download_progress(cJSON *cj_get)
{
	upgrade_t *upgrade = NULL;

	if ((upgrade = ucloud_query_upgrade())) {
		switch (upgrade->type) {
			case UP_TYPE_UPGRADE_Q: /* 正在连接升级服务器  */
				BIZ_DEBUG("router is contacting with upgrade server.\n");
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(UP_TYPE_UPGRADE_Q));

				break;
				
			case UP_TYPE_MEM_NOT_ENOUGH:  /* 内存不足无法升级  */
				BIZ_DEBUG("router doesn't have enough memory to download image.\n");
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(UP_TYPE_MEM_NOT_ENOUGH));
				break;
				
			case UP_TYPE_WAIT_SVR: /* 路由器在排队等待升级 */
				BIZ_DEBUG("router is queuing up for upgrade. : %s\n", upgrade->data);
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(UP_TYPE_WAIT_SVR));
				/*
				{
				        "pos":      2,
				        "time":     2,
				}
				*/
				//以上是upgrade->data获取数据，是json对象的字符串。其中有两个字段，pos代表在等待升级排队的位置，time代表等待了多久
				break;
				
			case UP_TYPE_FW_DOWNING: /* 路由器正在下载镜像 */
				BIZ_DEBUG("router is downloading the image file. : %s\n", upgrade->data);
				cJSON * data = cJSON_Parse(upgrade->data);
				copy_download_proccess_data(data, cj_get);
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(UP_TYPE_FW_DOWNING));
				cJSON_Delete(data);
				//
				char * p = cJSON_Print(cj_get);
				BIZ_DEBUG("cj_get: %s\n", p);
				free(p);

				/*
				{
				        "fw_size":      1425408,
				        "recved":       190689,
				        "sec_left":     6
				}
				*/
				//以上是upgrade->data获取数据，是json对象的字符串。其中有三个字段，fw_size代表镜像的总大小，recved代表已经下载了多上，sec_left表示离下载完成的剩余时间
				break;
				
			case UP_TYPE_WRITING_TO_FLASH: /* 路由器正在烧写固件 */
				BIZ_DEBUG("router is writing upgrade file into flash.\n");
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(UP_TYPE_WRITING_TO_FLASH));
				break;
				
			default:
				cJSON_AddItemToObject(cj_get, "error_code", 
									  cJSON_CreateNumber(download_proccess_get_errno()));
				BIZ_DEBUG("unkonw upgrade type.\n");
		}

		free(upgrade);
	} else {
		cJSON_AddItemToObject(cj_get, "error_code", 
							  cJSON_CreateNumber(download_proccess_get_errno()));
		BIZ_DEBUG("get upgrade fail, code [%d]\n",
				   cJSON_GetObjectItem(cj_get, "error_code")->valueint); /* 获取版本信息失败 */
	}
	return 0;
}

static int module_cmd_login_sta_update(const cJSON *cj_set)
{
	const cJSON * new_passwd_obj =
		cJSON_GetObjectItem(cj_set, "new_passwd");
	const cJSON * old_passwd_obj =
		cJSON_GetObjectItem(cj_set, "old_passwd");
	
	if (!new_passwd_obj || !old_passwd_obj) {
        BIZ_DEBUG("no new_passwd or old_passwd\n");
		return 1;
	}

	const char * new_pwd = new_passwd_obj->valuestring;
	const char * old_pwd = old_passwd_obj->valuestring;

	if (0 != strlen(new_pwd) &&
	    0 != strcmp(new_pwd,old_pwd)) {
		int ret = ucloud_login_sta_update();
		if (0 == ret) {
			BIZ_DEBUG("update passwd status success!\n");
			return 0;
		} else {
			BIZ_DEBUG("update passwd status failure!\n");
			return 1;
		}
	} else {
		BIZ_DEBUG("don't need to update the password status!\n");
		return 0;
	}
}

int uc_cgi_get_module_param(cJSON *cj_query, cJSON *cj_get)
{
	cJSON * cmd = NULL;	
	if (cj_query) {
		cmd = cJSON_GetObjectItem(cj_query, "module");
	} else {
		return 1;
	}

	int size = 0;
	if (cmd && (size = cJSON_GetArraySize(cmd))) {
		if (!strcmp(UC_GET_VERSION,
			        cJSON_GetArrayItem(cmd,0)->valuestring)) {
			return module_cmd_get_version(cj_get);
		} else if (!strcmp(UC_GET_DOWNLOAD_PROGRESS,
					       cJSON_GetArrayItem(cmd,0)->valuestring)) {
			return module_cmd_get_download_progress(cj_get);
		} else {
			return 1;
		}
	} else {
		return 1;
	}

	return 0;
}

int uc_cgi_set_module_param(cJSON *cj_query, cJSON *cj_set)
{
	cJSON * cmd = NULL;	
	if (cj_query) {
		cmd = cJSON_GetObjectItem(cj_query, "module");
	} else {
		return 1;
	}

	int size = 0;
	if (cmd && (size = cJSON_GetArraySize(cmd))) {
		if (!strcmp(UC_SET_LOGIN_STA_UPDATE,
			        cJSON_GetArrayItem(cmd,0)->valuestring)) {
			return module_cmd_login_sta_update(cj_set);
		}else if(!strcmp(UC_SET_RESTART_CONNECT_SVR,
			        cJSON_GetArrayItem(cmd,0)->valuestring)){
#ifdef CONFIG_APP_COSTDOWN
					ucloud_restart_connect_svr();
#else
			        uc_api_m_cloud_info_restart_connect_svr();
				 	uc_api_lib_commit(biz_m_api_lib_thread_fd_get());
#endif
			        return 1;
			}
		else {
			return 1;
		}
	} else {
		return 1;
	}

	return 0;
}


//web升级测试函数
void cgi_cjson_upgrade_test()
{
	BIZ_DEBUG("start time [%d]\n", time(0));
	{
		cJSON *cj_get 	= NULL;
		cJSON *module 	= NULL;
		cJSON *cj_query = NULL;
		char * p;

		cj_query = cJSON_CreateObject();
		cj_get = cJSON_CreateObject();
		cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
		cJSON_AddItemToArray(module,cJSON_CreateString(UC_GET_VERSION));
		uc_cgi_get_module_param(cj_query,cj_get);
		
		p = cJSON_Print(cj_get);
		BIZ_DEBUG("version: %s\n", p);
		free(p);

		cJSON_Delete(cj_query);
		cJSON_Delete(cj_get);
	}
	BIZ_DEBUG("end time [%d]\n", time(0));

	while (0) {
		cyg_thread_delay(1*80);
		{
			cJSON *cj_get 	= NULL;
			cJSON *module 	= NULL;
			cJSON *cj_query = NULL;
			char * p;

			cj_query = cJSON_CreateObject();
			cj_get = cJSON_CreateObject();
			cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
			cJSON_AddItemToArray(module,cJSON_CreateString(UC_GET_DOWNLOAD_PROGRESS));
			uc_cgi_get_module_param(cj_query,cj_get);
			
			p = cJSON_Print(cj_get);
			BIZ_DEBUG("download_progress: %s\n", p);
			free(p);

			cJSON_Delete(cj_query);
			cJSON_Delete(cj_get);
		}
	}
}

void cgi_cjson_login_sta_update_test()
{
	cJSON *cj_set	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query = NULL;
	char * p;

	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString(UC_SET_LOGIN_STA_UPDATE));
	cJSON_AddStringToObject(cj_set,"new_passwd", "");
	cJSON_AddStringToObject(cj_set,"old_passwd", "123");
	uc_cgi_set_module_param(cj_query,cj_set);
	
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);
}

