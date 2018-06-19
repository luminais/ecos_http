#ifndef _SWD_CONF_H_
#define _SWD_CONF_H_

#define SDW_CONF_FILE_SAVE					"/Dir_For_DD/config"
#define CONF_SERVER_URL	"c.so9.cc"
#define CONF_SERVER_URL_TEST	"easy2345.com"
#define CONF_FILE_SERVER_PATH	"/router/c/?t=td&g=%s&v=1&vr=1&cv=1&dv=1"
#define SERVER_PORT 80
#define TEST_STRING					"just for test...\n"
#define MAX_TRY_TIMES	3
#define CONF_RENEW_SLEEP 3600
#define CONF_SERV_IP_FAIL_SLEEP 600
#define INTERVAL_TIME 30
#define UPGRADE_INTERVAL_TIME 120

#define CONF_JS_READ (1)
#define CONF_FILTER_READ (1<<1)
#define CONF_UPGRADE_READ (1<<2)
#define CONF_ANALYZING (1<<7)
#define CONF_SUCCESS (0)

enum get_file_status
{
	OK = 0,
	DOWNLOAD_FAIL,
	DOWNLOAD_TIMEOUT,
	FILE_NOT_FOUNT,
	DIR_NOT_EXIST,
	LENGTH_ERROR,
	NOT_FOR_THIS_PRO,
	CHECK_FILE_ERROR,
	WRITE_FILE_FAIL
};

enum sdw_conf_index
{
	BEHAV_DATA_SW = 0,
	BEHAV_DATA_UL,
	PC_SW,
	PC_DL,
	PC_VE,
	PAD_SW,
	PAD_DL,
	PAD_VE,
	PHONE_SW,
	PHONE_DL,
	PHONE_VE,
	//JSFAIL_SW,//luminais rm jsfail_submit
	//JSFAIL_UL,
	JS_ENABLE,
	JS_STR,
	UPGRADE_SW,
	UPGRADE_DL,
	UPGRADE_VE
};

extern struct upgrade_mem *upgrade_mem_alloc(int totlen);
extern void upgrade_add_text(char *head, char *text, int nbytes);

extern void clear_file(char *file_path);
extern char * my_strcasestr(const char * s, const char * find);
extern int http_get_file(char *server_host, char *server_path, int port, char *server_ip, char *save_into_router, int *file_len);

#endif
