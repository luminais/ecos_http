#ifndef _SDW_HTTP_DOWNLOAD_H_
#define _SDW_HTTP_DOWNLOAD_H_


#define IP_LEN_16						16

#define ARRAY_LEN_128				128
#define ARRAY_LEN_256				256
#define ARRAY_LEN_1024				1024

#define DIR_FOR_DD					"/Dir_For_DD"	/*Bosson专有目录*/

#define BOS_INDEX					"/index.html"	/*邦信首页名称*/	

#define SDW_URL					 	"/url.xml"

#define SDW_AGREE_ELE				"/agree_tit.png"

#define SDW_TRUE_ELE				"/agree_true.png"

#define SDW_HEAD_PATH				"/header.png"

#define SDW_ITEM01_PATH			"/img_item01.png"

#define SDW_ITEM02_PATH			"/img_item02.png"

#define SDW_ITEM03_PATH			"/img_item03.png"

#define DOWNLOAD_URL_PATH 		"/showbox/url.xml"

#define DOWNLOAD_INDEX_PATH		"/showbox/index.html"

#define DOWNLOAD_AGREE_ELE		"/showbox/agree_tit.png"

#define DOWNLOAD_TRUE_ELE			"/showbox/agree_true.png"

#define DOWNLOAD_HEAD_PNG		"/showbox/header.png"

#define DOWNLOAD_ITEM01_PNG		"/showbox/img_item01.png"

#define DOWNLOAD_ITEM02_PNG		"/showbox/img_item02.png"

#define DOWNLOAD_ITEM03_PNG		"/showbox/img_item03.png"

#define SERVER_DOWNLOAD_IP 			"111.67.200.87"

#define SERVER_PORT 					8097	/*客户服务器端口号*/

#define LOAD_URL_RETRY_TIME			10

#define MAX_FULL_URL_LEN				1500

#define BOS_INDEX_ELEMENT_NUM	 	20

#define BOS_INDEX_ELEMENT_LEN		256

 int have_success_download = 0 ;

static int cur_bos_index_element_num=0;

static int cur_bos_index_path_num=1;	/*从1开始，bos_index_path[0]固定存放bonson专有的文件夹，由DIR_FOR_DD指定*/

static char bos_index_element[BOS_INDEX_ELEMENT_NUM][BOS_INDEX_ELEMENT_LEN];

static char bos_index_path[BOS_INDEX_ELEMENT_NUM][BOS_INDEX_ELEMENT_LEN];

static cyg_handle_t load_url_thread_handle;

static char load_url_thread_stack[4*12*1024];

static cyg_thread load_url_thread ;

#if 0
static cyg_handle_t load_sdwdns_thread_handle;

static char load_sdwdns_thread_stack[12*1024];

static cyg_thread load_sdwdns_thread ;
#endif

///extern struct in_addr addr_server_host ;

extern char sdw_dwn_index_ip[IP_LEN_16]  ;


 
int Bos_index_Sev();

int http_download(char *pURL,char *paddr,char *pSave_fileName);

extern void set_new_http_redirect_url(const char *new_redirect_url);

extern int read_url_from_file(void) ;

extern int nis_init_server_ip(void);


#endif
