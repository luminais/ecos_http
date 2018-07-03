#ifndef _SWD_FILTER_H_
#define _SWD_FILTER_H_

#include <stdio.h>
#include <arpa/nameser.h>

#define POST_MAC_LEN 					32 //转换后的mac地址长度

#define IP_LEN_16						16

#define MAC_LEN_20						20

#define ARRAY_LEN_128					128

#define ARRAY_LEN_256					256

#define ARRAY_LEN_1024					1024

#define ARRAY_LEN_2048 2048

#define MAX_ORIGIN_URL 					500 //长传url最大长度

#define MAX_HOST_URL					100

#define URL_RECORD_MAX_NUM			50

#define MAX_FULL_URL_LEN				1500


#define SERVER_DOWNLOAD_IP 			"111.67.200.87"

#define SERVER_DOWNLOAD_CLI_IP		"111.67.200.90"

#define SERVER_URL_POST_IP 				"111.67.200.90" //上传url的服务器的ip，使用域名

#define SERVER_HOST_NAME_QUERY 		"www.shooooooooowbox.net" //下载指定页面的域名，not use

#define SERVER_URL_DOWNLOAD_CLIENT	"server.sho9wbox.com" //下载客户端的域名，not use

#define SERVER_URL_SUBMIT				"manager.sho9wbox.com"	//接收url的服务器的域名

#define SDW_CLIENT_REQUERY_DNS 		"sho9wbox.net"		//发送指定域名

#define SDW_REDIRECT_HOST				"www.sho9wbox.com"

#define HTTP_OK							"200 OK"

#define SUCC_INFO						"succ"

#define POST_LOG_INTERFACE				"/manage/rebehavior"	/*客户提供上传日志接口*/

//#define URL_RECORD_FILE					"/Dir_For_DD/url_log.txt"
#define URL_RECORD_FILE					"/Dir_For_DD/url_log.gz"

#define SEND_URL_END					"%2C"			

#define SEND_JSON_END 					"%5D"
	
#define SEND_JSON_START				"data=%5B"

#define DATA_RULE_HASH_LEN (30)
#define DATA_FILE_HASH_LEN (10)
#define POST_URL_LOG_LEN (1000)
#define AES_BUF_LEN (1025)

struct file_content
{
	char content_name[16];
	char content[ARRAY_LEN_128];
};

enum{
	PARSE_ERROR = 0,
	PARSE_OK,
};

enum
{
	 IS_NOT_NEED_PACKET = 0 ,
	 IS_PHONE_PAD_CLI , 
	 IS_WIN_PC_CLI 
};

typedef HEADER SDWDNS ;


struct redirect_info
{
	unsigned char status;
	unsigned int time;
	unsigned int mark ;
};

extern  int have_success_download ;
extern int close( int fd );
extern in_addr_t inet_addr(const char *cp);
extern int inet_aton(register const char *cp, struct in_addr *addr);
extern int	connect __P((int, const struct sockaddr *, socklen_t));
extern int	getsockname __P((int, struct sockaddr *, socklen_t *));
extern ssize_t	recv __P((int, void *, size_t, int));
extern ssize_t	send __P((int, const void *, size_t, int));
extern int	setsockopt __P((int, int, int, const void *, socklen_t));
extern int	socket __P((int, int, int));

int nis_init_wanip(void);

int get_full_url(void);

extern int hash_match_url_handle( char * http_full_url ,int len , char* matched_redirect_url , int matched_url_len);

extern int dnsmasq_parse_request(SDWDNS *dnsheader, unsigned int qlen, char * name ) ;

extern int dnsmasq_parse_replay(SDWDNS *dnsheader, unsigned int qlen, char *name) ;
extern void SetTimeout(int sock);
extern char *get_post_head(char *url,char *host,int post_data_len , char* post_head);
extern int get_ip(char *host , char *ip) ;
extern int create_tcp_socket(void);
extern int oslib_getpidbyname(char *tname) ;

extern int get_url_path(char *full_path, char *url, char *path, int *port);
extern int sdw_conf_done;
extern struct file_content sdw_conf_content[];

#endif 
