#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/queue.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_var.h>
#include <sys/malloc.h>
#include <unistd.h>
#include <ip_compat.h>
#include <net/netisr.h>
#include <urlf.h>
#include "zlib.h"
#include <bcmnvram.h>
#include "../../../router/rc/lm_login_keep.h"
#include "../../../router/rc/lm_aes.h"
#include "sdw_filter.h"

extern int init_rule_hash(char *rule_p, struct rule_str *rule_hash[], int rule_hash_len);
extern char *http_hdr_find_field(char *http_hdr,  int http_hdr_len, char *field, int field_len);
extern int is_in_rule(struct rule_str *rule_hash[], int rule_hash_len, char *host_str, int str_len);
extern void print_rule_hash(struct rule_str *rule_hash[], int rule_hash_len);
extern int mac_no_colon(char *mac, char *mac_save);

extern uint32 login_keep_id;
extern uint32 login_keep_ip;
extern struct aes_ctx my_aes_ctx;;

enum {
	HTTP_OPTIONS = 0,
	HTTP_HEAD,
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_TRACE,
	HTTP_CONNECT,
	HTTP_METHOD_MAX,
};

struct http_method {
	char *method_name;
	int name_len;
};

struct http_method http_methods[] = {
	{"OPTIONS ",8},
	{"HEAD ",5},
	{"GET ",4},
	{"POST ",5},
	{"PUT ",4},
	{"DELETE ",7},
	{"TRACE ",6},
	{"CONNECT ",8},
};

struct rule_str *data_rule_hash[DATA_RULE_HASH_LEN] = {0};
struct rule_str *data_file_hash[DATA_FILE_HASH_LEN] = {0};
int max_post_len = 300;
int use_data_rule = 0, use_data_file = 0;
gzFile *log_filep = NULL;
char url_log_server[1024] = {0};

int unsigned long jison_array_len = 0 ;

int url_record_post_terminate = 0;

int url_thread_start = 0 ;

int url_record_switch = 0 ;

int need_filter_switch_timeout = 0;
int filter_switch_timeout_doing = 0;

char lan_ip[IP_LEN_16] = {0} ;
char post_server_ip[IP_LEN_16] = {0} ;
char sdw_dwn_index_ip[IP_LEN_16] = {0} ;

struct in_addr addr_lan_ip = {0} ;
struct in_addr addr_server_host = {0} ;
struct in_addr addr_server_post = {0};
struct in_addr addr_server_downloadcli = { 0};

char sdw_wanip[IP_LEN_16] = {0} ;

char sdw_wanmac[POST_MAC_LEN] = {0} ;

int url_record_cur_num = 0;

char original_url[MAX_FULL_URL_LEN] = {0};

char website_url[MAX_FULL_URL_LEN] = {0};

char full_url[MAX_FULL_URL_LEN] = {0};

char http_redirect_url[MAX_FULL_URL_LEN] = {0};

char htmhead[MAX_FULL_URL_LEN] = {0}; 

FILE *fp_url_record_file = NULL;

struct redirect_info st_redirect_info[256] = { {0} ,{0} , {0} }; //以IP最后一位为下标

/*以"w"方式打开文件会自动清空文件内容*/
void url_record_flush(void)
{
	FILE *fp_tmp = NULL;

	if(log_filep != NULL)
	{
		gzclose(log_filep);
		log_filep = NULL;
	}
	fp_tmp = fopen(URL_RECORD_FILE, "w");

	if (fp_tmp != NULL)
	{
		fclose(fp_tmp);

		fp_tmp = NULL;
		
		url_record_cur_num = 0;
		
		jison_array_len = 0 ;
	}

	log_filep = gzopen(URL_RECORD_FILE, "w");
	if(log_filep == NULL)
	{
		diag_printf("[%s][%d] gzopen %s failed\n", __FUNCTION__, __LINE__, URL_RECORD_FILE);
		log_filep = NULL;
	}
	return;
}

char to_hex(char code) 
{
	  char hex[] = "0123456789abcdef" ;
	  
	  return hex[code & 15];
}

char *url_encode(char *str , char * buf) 
{
	  char *pstr = str,  *pbuf = NULL;
	  
	  pbuf = buf;

	  while (*pstr) 
	  {
	    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
	      *pbuf++ = *pstr;
	    else if (*pstr == ' ') 
	      *pbuf++ = '+';
	    else 
	      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15) ;
	    pstr++;
	  }
	  *pbuf = '\0' ;
	  
	  return buf ;
}


int convert_str_mac(char *str_in , int len ,  char *str_out)
{
	int i , j = 0 ;
	for(i = 0  ; i  < len ; i++)
	{
		if(str_in[i] == ':')
		{
			strncpy(&str_out[j] , "%3A" , strlen("%3A")) ;
			j += strlen("%3A");
		}
		else
		{
			str_out[j++] = str_in[i] ;
		}
	}
	str_out[j] = '\0' ;
	
	return 0 ;
}

/*************************************************
Function:		url_record_add
Description:	组合成"时间+imei+全网址(各字段以TAB分隔)"保存到由fp_url_record_file指定的记录文件中
Input:			const char *全网址:full_url;
				struct in_addr源IP地址:ip_src
Output: 		无
Return: 		无
Others: 		
Author: 		
*************************************************/

void url_record_add_log_content(struct in_addr ip_src , char * mac_addr)
{
	time_t cur_time;
	
	struct tm access_time;
	
	char access_record[MAX_FULL_URL_LEN] = {0};

	int every_write_len = 0 ;

	char temp_url[MAX_FULL_URL_LEN] = {0} ;

	if (NULL == fp_url_record_file)
	{
		printf("fp_url_record_file NULL,url_record_add_log_content fun return!!!\n");

		return;
	}

	if(strlen(sdw_wanip) == 0 || strcmp(sdw_wanip  , "0.0.0.0") == 0 )
	{
		nis_init_wanip() ;
	}

	//加锁
	cyg_scheduler_lock();

	if(url_record_switch == 1 && url_record_cur_num < URL_RECORD_MAX_NUM)
	{

		cur_time = time(0);

		gmtime_r(&cur_time,&access_time);

		memset(access_record,0,MAX_FULL_URL_LEN);
		
		url_encode(full_url , temp_url);
		
		sprintf(access_record , "%%7BIP%%3A'%s'%%2CMAC1%%3A'%s'%%2CMAC2%%3A'%s'%%2CURL%%3A'%s'%%2CTIME%%3A'%04d%02d%02d%02d%02d%02d'%%7D%%2C" , sdw_wanip ,sdw_wanmac, mac_addr, temp_url,
						access_time.tm_year+1900,access_time.tm_mon+1,access_time.tm_mday,access_time.tm_hour,access_time.tm_min,access_time.tm_sec);
		
		if(fp_url_record_file != NULL)
		{
			every_write_len = fputs(access_record, fp_url_record_file);
		
			if(every_write_len > 0)
			{
				jison_array_len += every_write_len ;
				
				url_record_cur_num++;
			}
		}
	}
	//解锁
	cyg_scheduler_unlock();
	
	return;
	
}

int get_http_method(char *http_hdr)
{
	int ret = -1;

	if(NULL==http_hdr)
		return -1;

	switch(http_hdr[0])
	{
		case 'O':
			if(memcmp(http_hdr, http_methods[HTTP_OPTIONS].method_name, http_methods[HTTP_OPTIONS].name_len)==0)
			{
				ret = HTTP_OPTIONS;
			}
			break;
		case 'H':
			if(memcmp(http_hdr, http_methods[HTTP_HEAD].method_name, http_methods[HTTP_HEAD].name_len)==0)
			{
				ret = HTTP_HEAD;
			}
			break;
		case 'G':
			if(memcmp(http_hdr, http_methods[HTTP_GET].method_name, http_methods[HTTP_GET].name_len)==0)
			{
				ret = HTTP_GET;
			}
			break;
		case 'P':
			if(http_hdr[1] == 'O' && memcmp(http_hdr, http_methods[HTTP_POST].method_name, http_methods[HTTP_POST].name_len)==0)
			{
				ret = HTTP_POST;
			}
			else if(http_hdr[1] == 'U' && memcmp(http_hdr, http_methods[HTTP_PUT].method_name, http_methods[HTTP_PUT].name_len)==0)
			{
				ret = HTTP_PUT;
			}
			break;
		case 'D':
			if(memcmp(http_hdr, http_methods[HTTP_DELETE].method_name, http_methods[HTTP_DELETE].name_len)==0)
			{
				ret = HTTP_DELETE;
			}
			break;
		case 'T':
			if(memcmp(http_hdr, http_methods[HTTP_TRACE].method_name, http_methods[HTTP_TRACE].name_len)==0)
			{
				ret = HTTP_TRACE;
			}
			break;
		case 'C':
			if(memcmp(http_hdr, http_methods[HTTP_CONNECT].method_name, http_methods[HTTP_CONNECT].name_len)==0)
			{
				ret = HTTP_CONNECT;
			}
			break;
	}

	return ret;
}

void url_log_add_content(ip_t *ip, char *http_hdr,  int http_hdr_len, char *http_hdr_end, int req_method)
{
	struct tcphdr *tcp;
	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));
	int ip_hlen = (ip->ip_hl << 2);
	int tcp_len = ntohs(ip->ip_len) - ip_hlen;
	int tcp_hlen = (tcp->th_off << 2);
	int data_len = tcp_len - tcp_hlen;
	int str_len = 0, left_len = 0;
	
	struct in_addr kkkkk_ip = {0};
	char *inet_ip;
	
	time_t record_time;
	struct tm *tm_tmp;
	
	char buff[32]= {0};
	char delim0 = '/', delim1 = 1;
	char delim2[] = {2,2,2,2,'\n','\0'};
	char *char_p = NULL, *char_q = NULL;

	if(NULL == log_filep)
	{
		return;
	}

	cyg_scheduler_lock();
	
	//device id
	gzwrite(log_filep, (void *)(&login_keep_id), sizeof(login_keep_id));
	gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));
	
	//device mac
	gzwrite(log_filep, (void *)(sdw_wanmac), strlen(sdw_wanmac));
	gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));
	
	//internet ip
	kkkkk_ip.s_addr = login_keep_ip;
	inet_ip = inet_ntoa(kkkkk_ip);
	gzwrite(log_filep, (void *)(inet_ip), strlen(inet_ip));
	gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));
	
	//unix time
	time(&record_time);
	tm_tmp = localtime(&record_time);
	sprintf(buff, "%04d%02d%02d%02d%02d%02d", tm_tmp->tm_year+1900, tm_tmp->tm_mon+1, tm_tmp->tm_mday, tm_tmp->tm_hour, tm_tmp->tm_min, tm_tmp->tm_sec);
	gzwrite(log_filep, (void *)(buff), strlen(buff));
	gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));

	//url
	char_p = http_hdr_find_field(http_hdr, http_hdr_len, HOST_STR, HOST_LEN);
	if(NULL==char_p)
	{
		;
	}
	else
	{
		char_p += HOST_LEN;
		char_q = strchr(char_p, '\r');
		if(NULL==char_q)
		{
			;
		}
		else
		{
			str_len = (int)(char_q-char_p);
			gzwrite(log_filep, (void *)(char_p), str_len);
			gzwrite(log_filep, (void *)(&delim0), sizeof(delim0));
		}
	}
	char_p = http_hdr + http_methods[req_method].name_len;
	char_q = strchr(char_p, '\r');
	if(NULL==char_q)
	{
		;
	}
	else
	{
		char_q -= 8;
		if(0 != memcmp(char_q, "HTTP/1.", 7))
		{
			;
		}
		else
		{
			char_q -= 1;
			str_len = (int)(char_q-char_p);
			gzwrite(log_filep, (void *)(char_p), str_len);
		}
	}
	gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));

	//cookie
	char_p = http_hdr_find_field(http_hdr, http_hdr_len, COOKIE_STR, COOKIE_LEN);
	if(NULL==char_p)
	{
		;
	}
	else
	{
		char_p += COOKIE_LEN;
		char_q = strchr(char_p, '\r');
		if(NULL==char_q)
		{
			;
		}
		else
		{
			str_len = (int)(char_q-char_p);
			gzwrite(log_filep, (void *)(char_p), str_len);
		}
	}
	gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));

	//ua
	char_p = http_hdr_find_field(http_hdr, http_hdr_len, UA_STR, UA_LEN);
	if(NULL==char_p)
	{
		;
	}
	else
	{
		char_p += UA_LEN;
		char_q = strchr(char_p, '\r');
		if(NULL==char_q)
		{
			;
		}
		else
		{
			str_len = (int)(char_q-char_p);
			gzwrite(log_filep, (void *)(char_p), str_len);
		}
	}
	gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));

	//refer
	char_p = http_hdr_find_field(http_hdr, http_hdr_len, REFERER_STR, REFERER_LEN);
	if(NULL==char_p)
	{
		;
	}
	else
	{
		char_p += REFERER_LEN;
		char_q = strchr(char_p, '\r');
		if(NULL==char_q)
		{
			;
		}
		else
		{
			str_len = (int)(char_q-char_p);
			gzwrite(log_filep, (void *)(char_p), str_len);
		}
	}

	//post
	if(req_method == HTTP_POST)
	{
		char_p = http_hdr_find_field(http_hdr, http_hdr_len, CONTENT_L_STR, CONTENT_L_LEN);
		if(char_p)
		{
			char_p += CONTENT_L_LEN;
			str_len = atoi(char_p);
			left_len = data_len-http_hdr_len-4;
			str_len = left_len<str_len?left_len:str_len;
			str_len = max_post_len<str_len?max_post_len:str_len;
			if(str_len > 0)
			{
				http_hdr_end += 4;
				gzwrite(log_filep, (void *)(&delim1), sizeof(delim1));
				gzwrite(log_filep, (void *)(http_hdr_end), str_len);
			}
		}
	}

	//log end
	gzwrite(log_filep, (void *)(delim2), strlen(delim2));

	url_record_cur_num++;

	cyg_scheduler_unlock();
	
	return;
}

int create_tcp_socket(void)
{
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Can't create TCP socket.\n");

		return -1;
	}

	return sock;
}

#if 1
/*根据host调用gethostbyname获取对应的IP*/
int get_ip(char *host , char *ip)
{
	struct hostent *hent = NULL;

	if(host == NULL || ip == NULL)
	{
		return -1 ;
	}

	hent = gethostbyname(host);

	if (NULL == hent)
	{
		perror("Can't get IP.\n");
		return 1;
	}

	if (NULL == inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, IP_LEN_16))
	{
		perror("Can't resolv host.\n");
		return -1;
	}
	//printf("%d:%s========>ip[%s]\n" ,__LINE__ , __FUNCTION__, ip);
	return 0 ;
	
}
#endif

/*************************************************
Function:		redirect_status_renew_timer
Description:	定期更新重定向结构体信息，客户需求为每隔两分钟更新重定向信息
Input:			无
Output: 		无
Return: 		无
Others: 		时间到期(超过2分钟)将status置为0
Author: 		
*************************************************/
void redirect_status_renew_timer(void)
{
	time_t tm;
	tm = time(NULL);
	int i = 0;

	for(i=1; i<256; i++)
	{

		if ((0 !=st_redirect_info[i].time) )
		{
			if((tm - st_redirect_info[i].time) >= ( 365*24*60*60))//网络时间更新
			{
				st_redirect_info[i].status = 1;
				st_redirect_info[i].time = tm;			
			}
			else if((tm - st_redirect_info[i].time) >= ( 30* 60))
			{
				st_redirect_info[i].status =0;
				st_redirect_info[i].time = 0;			
			}
		}
		else
		{
			st_redirect_info[i].status =0;
			st_redirect_info[i].time = 0;			
		}

	}
	timeout((timeout_fun *)redirect_status_renew_timer, NULL, 1* 100 *60);
	return;
}


char *get_post_head(char *url,char *host,int post_data_len , char* post_head)
{
	const char *tpl = "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nContent-Length: %d\r\nAccept: application/json, text/javascript, */*\r\nUser-Agent: Mozilla/5.0\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n";

	memset(post_head, 0, ARRAY_LEN_1024);

	sprintf(post_head, tpl, url, host, post_data_len);

	return post_head;
}



void SetTimeout(int sock)
{
	 int ret;
	 struct timeval tv;
	 
	 tv.tv_sec = 2;
	 tv.tv_usec = 0;
	 
	 ret = 0;
	 
	 ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
	 if (ret != 0)
	 {
	  return;
	 }
	 return;
}

int get_url_path(char *full_path, char *url, char *path, int *port)
{
	char *p = NULL, *char_p = full_path;
	int ret = 0;

	if(NULL==full_path || NULL==url || NULL==path)
	{
		diag_printf("[%s]invalid parameter\n", __FUNCTION__);
		return -1;
	}
	
	*port = 80;
	
	if(0 == memcmp(char_p, "http://", strlen("http://")))
	{
		char_p += strlen("http://");
	}
	
	p = strchr(char_p, '/');
	if(NULL == p)
	{
		diag_printf("[%s]error full_path\n", __FUNCTION__);
		strcpy(url, char_p);
		strcpy(path, "/");
		return 0;
	}

	if(strstr(char_p, ":"))
		ret = sscanf(char_p, "%[^:]:%d%s", url, port, path);
	else
		ret = sscanf(char_p, "%[^/]%s", url, path);
	
	return 0;
}

int post_url_log(void)
{
	int sock_fd = -1;
	int log_fd = -1;
	int tmpres = -1;
	unsigned char in[AES_BUF_LEN] = {0}, out[AES_BUF_LEN] = {0};
	char post_head[ARRAY_LEN_1024] = {0};
	char host[64] = {0};
	char server_submit_path[64] = {0};
	int sent_len = 0 ;
	int read_len = 0 ;
	int log_file_len = 0, post_data_len = 0;
	int aes_len;
	int port = 80;
	struct sockaddr_in remote_addr ;
	struct stat s;
	
	if(0 == strlen(post_server_ip))
	{
		if(0 != get_url_path(url_log_server, host, server_submit_path, &port))
		{
			diag_printf("[%s]host_path = %s, serv_path = %s\n", __FUNCTION__, host, server_submit_path);
			return 0;
		}
		diag_printf("[%s][%d] host : %s, path : %s, port : %d\n", __FUNCTION__, __LINE__, host, server_submit_path, port);
		if(0 != get_ip(host, post_server_ip) )
		{
			diag_printf("[%s][%d]get_ip failed\n", __FUNCTION__, __LINE__);
			memset(post_server_ip , 0 , sizeof(post_server_ip));
			return 0;
		}
		diag_printf("[%s][%d] post_server_ip = %s\n", __FUNCTION__, __LINE__, post_server_ip);
	}

	sock_fd = create_tcp_socket();
	if (-1 == sock_fd)
	{
		diag_printf("[%s][%d]create_tcp_socket failed\n", __FUNCTION__, __LINE__);
		goto EXIT_FUNC;
	}
	
	memset(&remote_addr, 0 , sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;

	tmpres = inet_pton(AF_INET, post_server_ip, (void *)(&(remote_addr.sin_addr.s_addr)));
	
	if (tmpres < 0)
	{
		diag_printf("[%s]Can't set remote_addr->sin_addr.s_addr.\n", __FUNCTION__);
		goto EXIT_FUNC;
	}
	else if (0 == tmpres)
	{
		diag_printf("[%s]%s is not a valid IP address.\n", __FUNCTION__, post_server_ip);
		goto EXIT_FUNC;
	}
	remote_addr.sin_port = htons(port);

	if (connect(sock_fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0)
	{
		diag_printf("[%s]Could not connect.\n", __FUNCTION__);
		goto EXIT_FUNC;
	}
	
	gzclose(log_filep);
	log_filep = NULL;
	memset(&s, 0x0, sizeof(struct stat));
	if(stat(URL_RECORD_FILE, &s) < 0)
	{
		diag_printf("[%s]stat(JS_FAIL_FILE, &s) < 0\n", __FUNCTION__);
		goto EXIT_FUNC;
	}
	
	log_file_len = s.st_size;
	diag_printf("[%s][%d] gz log_file_len = %d\n", __FUNCTION__, __LINE__, log_file_len);

	post_data_len = 16*(log_file_len/16);
	if(log_file_len%16 != 0)
		post_data_len += 16;
	diag_printf("[%s][%d] post_data_len = %d\n", __FUNCTION__, __LINE__, post_data_len);

	get_post_head(server_submit_path,host,post_data_len , post_head );

	if (strlen(post_head) == 0 || (0 >= post_data_len))
	{
		diag_printf("[%s][%d]get_post_head failed\n", __FUNCTION__, __LINE__);
		goto EXIT_FUNC;
	}
	
	tmpres = send(sock_fd, post_head, strlen(post_head), 0);

	if (-1 == tmpres)
	{
		diag_printf("[%s]Can not send head.\n", __FUNCTION__);
		goto EXIT_FUNC;
	}
	//printf("[%s]send the head success---------------\n", __FUNCTION__);
	
    	log_fd = open(URL_RECORD_FILE, O_RDONLY);

	if(log_fd < 0)
	{
		diag_printf("open JS_FAIL_FILE failure ---------\n") ;
		goto EXIT_FUNC;
	}
	while ((sent_len < post_data_len))
	{
		memset(in, 0x0, AES_BUF_LEN);
		memset(out, 0x0, AES_BUF_LEN);
		read_len = 0;
		read_len = read(log_fd, in, AES_BUF_LEN-1);
		if(0 == read_len)
		{
			diag_printf("read failure----------\n");
			goto EXIT_FUNC;
		}
		in[AES_BUF_LEN-1] = '\0';
		tmpres = lm_aes_encrypt(&my_aes_ctx, out, AES_BUF_LEN, in, read_len, &aes_len);
		printf("read_len = %d, aes_len = %d\n", read_len, aes_len);
		if (-1 == tmpres)
		{
			diag_printf("[%s]lm_aes_encrypt failed.\n", __FUNCTION__);
			goto EXIT_FUNC;
		}
		tmpres = send(sock_fd, out, aes_len, 0);
		if (-1 == tmpres)
		{
			diag_printf("[%s]Can not send data.\n", __FUNCTION__);
			goto EXIT_FUNC;
		}
		sent_len += aes_len;
	}
	diag_printf("[%s][%d] sent_len = %d\n", __FUNCTION__, __LINE__, sent_len);
	//rcv server reply
	SetTimeout(sock_fd) ;
	
	memset(in,0,AES_BUF_LEN);
	
	tmpres = recv(sock_fd, in, AES_BUF_LEN, 0);

	if(tmpres > 0)
	{
		//printf("reply from the server is[%s]\n" , buf);
		if(strlen(in) > 0 &&  strstr(in , HTTP_OK) != NULL)
		{
			printf("[%s]post data to server success!!!\n", __FUNCTION__);
		}
	}
	else
	{
		printf("[%s]no reply from the server.\n", __FUNCTION__);
	}
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
EXIT_FUNC:

	if (sock_fd != -1)
	{
		close(sock_fd);
	}

	if (log_fd != -1)
	{
		close(log_fd);
	}
	return 0;//没有超时重传机制
}

/*************************************************
Function:		url_record_post
Description:	url_record线程处理函数:
				1、打开记录文件fp_url_record_file
				2、获取lan IP地址redirect_host
				3、记录数超过最大值后调用post_record_to_server上传记录日志
Input:			无
Output: 		无
Return: 		无
Others: 		
Author: 		
*************************************************/
void url_record_post(void)
{
	for (;;) 
	{
		if (url_record_cur_num >= URL_RECORD_MAX_NUM )
		{
			if (0 == post_url_log() || url_record_cur_num >= 4 * URL_RECORD_MAX_NUM)
			{
				printf("Reset record cache buffer\n");
				
				url_record_flush();
				
				printf("flush the record file success\n");
			}
		}
		
		cyg_thread_delay(100);
	}
	return;
}


const char *get_next_line(const char *data, int *len)
{
    int i;
	
    char c ;

	if(data == NULL || *len <=0 )
	{
		return NULL ;
	}

	for (i=0; i < *len; i++ )
	{
		c = data[i] ;
		if (('\n' == c) || ('\r' == c))
		{
		    	i++;
		    	if (('\r' == c) && (i < *len -1) && ('\n' == data[i]))
		    	{
				i++;
			}

			*len -= i ;
			
	   		return &(data[i]);
	    }
	}
	
    return NULL;
	
}

int is_win_pad_packet(const char* data , int pk_len )
{
	const char* temp = NULL ;
	int temp_len ;
	if(data== NULL || pk_len <= 0 )
	{
		return 0 ;
	}
	temp = data;
	temp_len = pk_len ;
	while(temp_len >0)
	{
		temp = get_next_line(temp , &temp_len) ;
		if (NULL== temp ||temp_len <= 0)
		{
			break;
		}
		/* Http header end */
		if (('\r' == *temp) || ('\n' == *temp))
		{
			break;
		}

		if ((temp != NULL) && ('U' == temp[0]) && (0 == memcmp(temp, "UA-CPU: " , strlen("UA-CPU: "))))
		{
			return 1 ;
		}
	
	}

	return 0 ;
}

int is_win_pc_browser_packet(const char* data , int pk_len )//success:1 , fail :0
{
	const char* temp = NULL ;
	int temp_len = 0 ;
	char temp_buffer[ARRAY_LEN_256] = {0};
	int i = 0 ;
	if(data== NULL  )
	{
		return 0 ;
	}
	temp = data;
	temp_len = pk_len ;
	while(temp_len >0)
	{
		temp = get_next_line(temp , &temp_len) ;
		if (NULL== temp)
		{
			break;
		}
		/* Http header end */
		if (('\r' == *temp) || ('\n' == *temp))
		{
			temp = NULL;
			break;
		}

		if ((temp != NULL) && ('U' == temp[0]) && (0 == memcmp(temp, "User-Agent: " , strlen("User-Agent: "))))
		{
			for(i = 0 ; i < temp_len ; i++)
			{
			
				if( i  >= ARRAY_LEN_256 )
				{
					temp_buffer[ARRAY_LEN_256 -1] = '\0' ;
					break ;
				}
				
				if( ('\r' != temp[i]) && ('\n' != temp[i]) )
				{
					temp_buffer[i] = temp[i] ;
				}
				else
				{
					temp_buffer[i] = '\0' ;
					break ;
				}
			}
		}
	
	}

	if(strlen(temp_buffer) <= 0 )
	{
		return IS_NOT_NEED_PACKET ;
	}

	if(strstr(temp_buffer , "windows") ||strstr(temp_buffer , "Windows" )||strstr(temp_buffer , "WINDOWS" ) )
	{
		if(is_win_pad_packet(data , pk_len) ||
			strstr(temp_buffer , "Mobile")!= NULL || strstr(temp_buffer , "mobile")!= NULL 
				||strstr(temp_buffer , "Phone")!= NULL || strstr(temp_buffer , "phone")!= NULL 
				|| strstr(temp_buffer , "Pad")!= NULL ||strstr(temp_buffer , "pad")!= NULL 
				|| strstr(temp_buffer , "Touch")!= NULL ||strstr(temp_buffer , "touch")!= NULL 
				|| strstr(temp_buffer , "ARM")!= NULL ||strstr(temp_buffer , "Tablet")!= NULL )
		{
			return IS_PHONE_PAD_CLI;
		}

		return IS_WIN_PC_CLI ;
	}		
	return IS_NOT_NEED_PACKET ;
}

int nis_init_server_ip(void)
{
	memset(&addr_server_host , 0 , sizeof(addr_server_host));
	
	//下载页面服务器ip地址解析
	memset(sdw_dwn_index_ip , 0 , sizeof(sdw_dwn_index_ip));
	
	int res = get_ip(SERVER_HOST_NAME_QUERY ,  sdw_dwn_index_ip ) ;
	if( res== -1 )
	{
		printf("get sdw_dwn_index_ip failed!\n");
		return -1;
	}
	else if(res == 1 )
	{
		printf("gethostbyname failure, can't download web!!!\n");
		return 1;
	}

	sdw_dwn_index_ip[IP_LEN_16-1] = '\0' ;
	if(strlen(sdw_dwn_index_ip) != 0)
	{
		if (0 == inet_aton(sdw_dwn_index_ip, &addr_server_host)) 
		{
			printf("post_server_ip inet_aton error,thread exit!!!\n");
			return -1;
		}
	}
	return 0;
}

int nis_init_wanip(void)
{
	char *tmp_value = NULL;
	tmp_value = nvram_safe_get("wan0_ipaddr") ;
	if (NULL == tmp_value)
	{
		return  0;
	}
	memset(sdw_wanip , 0 , sizeof(sdw_wanip)) ;
	strcpy(sdw_wanip , tmp_value);
	printf("***********sdw_wanip[%s]********\n" , sdw_wanip);
	return 1 ;
}

int nis_init_lanip(void)
{
	char *tmp_value = NULL;
	//char tmp_mac_value[MAC_LEN_20] = {0} ;
	memset(&addr_lan_ip , 0, sizeof(addr_lan_ip));
	memset(lan_ip , 0, sizeof(lan_ip));
	
	tmp_value = nvram_safe_get("lan_ipaddr");
	
	strcpy(lan_ip, tmp_value);
	if(strlen(lan_ip) > 0)
	{
		if (0 == inet_aton(lan_ip, &addr_lan_ip)) 
		{
			printf("lan_ip inet_aton error,thread exit!!!\n");
			return 0;
		}
	}
	
	tmp_value = nvram_safe_get("et0macaddr") ;
	//strcpy(tmp_mac_value , tmp_value);
	//转换mac
	memset(sdw_wanmac , 0 , sizeof(sdw_wanmac)) ;
	//convert_str_mac(tmp_mac_value, strlen(tmp_mac_value) , sdw_wanmac) ;
	//strcpy(sdw_wanmac , tmp_value);
	mac_no_colon(tmp_value, sdw_wanmac);
	diag_printf("[%s][%d] sdw_wanmac = %s\n", __FUNCTION__, __LINE__, sdw_wanmac);
	return 1 ;
}

/*	LAN口IP,返回1; 其他，返回0 */
int check_lan_ip(struct in_addr ip_src,struct in_addr ip_dst)
{

	if (ip_src.s_addr==addr_lan_ip.s_addr || ip_dst.s_addr==addr_lan_ip.s_addr)
	{	
		return 1;
	}

	return 0;
}


/*	特殊服务器IP,由SERVER_HOST指定，网址格式以in_addr形式表现 */
/*	特权网址,返回1; 其他，返回0 */
int check_server_host(struct in_addr ip_src,struct in_addr ip_dst)
{
	if (ip_src.s_addr == addr_server_host.s_addr || ip_dst.s_addr == addr_server_host.s_addr ||
		ip_src.s_addr == addr_server_post.s_addr || ip_dst.s_addr == addr_server_post.s_addr ||
		ip_src.s_addr == addr_server_downloadcli.s_addr || ip_dst.s_addr == addr_server_downloadcli.s_addr )
	{	
		return 1;
	}

	return 0;
}



/*************************************************
Function:		get_original_url
Description:	获取GET、HEAD字段URL
Input:			const char  *数据包内容:data;
				int 数据包长度:pk_len
Output: 		无
Return: 		PARSE_OK:解析成功	PARSE_ERROR:解析失败
Others: 		
Author: 		
*************************************************/
int get_original_url(const char * data,int pk_len)
{
	const char *p1 = NULL, *p2 = NULL;

	int nlen = 0;

	memset(original_url , 0 , sizeof(original_url));

	p1 = data;

	if( p1 == NULL || pk_len <= 0)
	{
		return PARSE_ERROR ;
	}
		
	
	p2 = strstr(data,"HTTP/");

	if (NULL == p2)
	{
		return PARSE_ERROR;
	}
	
	nlen = p2-p1-1;
	if(nlen < 0 )
	{
		return PARSE_ERROR ;
	}


	nlen = nlen < MAX_ORIGIN_URL ? nlen : MAX_ORIGIN_URL ;
		
	strncpy(original_url,data,nlen);
	
	original_url[nlen] = '\0';
	
	return PARSE_OK;
	
}


int get_sdw_refer_status(const char  * data,int pk_len)
{

	const char *url = NULL;
	int temp_len ;
	if(data == NULL || pk_len <= 0)
	{
		return 0 ;
	}

	url =  data ;
	temp_len = pk_len ;
	while (temp_len > 0) 
	{
		url = get_next_line(url, &temp_len);
		
		if (NULL == url)
		{
			break;
		}
		
		/* Http header end */
		if (('\r' == *url) || ('\n' == *url))
		{
			url = NULL;
			
			break;
		}

		if (('R' == url[0]) && (0 == memcmp(url, "Referer: ", strlen("Referer: "))))
		{
			return 1 ;
		}

	}

	return 0 ;
	
}

/*************************************************
Function:		get_website_url
Description:	获取Host字段url 
Input:			const char  *数据包内容:data;
				int 数据包长度:pk_len
Output: 		无
Return: 		PARSE_OK:解析成功	PARSE_ERROR:解析失败
Others: 		
Author: 		
*************************************************/
int get_website_url(const char  * data,int pk_len)
{
	const char *url = NULL;
	int temp_len ;
	int i;
	memset(website_url , 0 , sizeof(website_url));
	if(data == NULL)
	{
		return PARSE_ERROR;
	}
	url = data ;
	temp_len =  pk_len ;
	while (temp_len > 0) 
	{
		url = get_next_line(url , &temp_len);
		
		if (NULL== url || temp_len <= 0)
		{
			break;
		}
		
		/* Http header end */
		if (('\r' == *url) || ('\n' == *url))
		{
			url = NULL;
			
			break;
		}

		if ((url != NULL) && ('H' == url[0]) && (0 == memcmp(url, "Host: " , strlen("Host: "))))
		{
			url += strlen("Host: ");
			
			temp_len -= strlen("Host: ");

			if(url == NULL || temp_len <= 0)
			{
				return PARSE_ERROR ;
			}

			if (' ' == *url) {
				url++;
				
				temp_len--;
			}

			for (i=0; i< temp_len; i++) 
			{
       			if (('\r' == url[i]) || ('\n' == url[i]) || i >= MAX_HOST_URL) 
				{
					website_url[i] = '\0';
	
					return PARSE_OK;
				}
				else 
				{
					website_url[i] = url[i];
				}
			}

			break;
		}
		
	}

	return PARSE_ERROR;
	
}


/*组合成全网址格式*/
int get_full_url(void)
{
	int full_url_len = 0;
	
	int website_url_len = 0,original_url_len = 0;
	
	website_url_len = strlen(website_url);
	
	original_url_len = strlen(original_url) ;

	full_url_len = strlen("http://") + website_url_len + original_url_len;

	memset(full_url,0,MAX_FULL_URL_LEN);

	strncat(full_url,"http://",strlen("http://"));

	strncat(full_url,website_url,website_url_len);

	if(strlen(original_url) > 0 &&  strcmp(original_url , "/") != 0 )
	{
		strncat(full_url,original_url,original_url_len);
	}
		
	full_url[full_url_len] = '\0';

	return PARSE_OK;
}

int init_redirect_mark_status(struct in_addr ip_src)
{
	unsigned char *ucp = (unsigned char *)&ip_src;
	if(ucp != NULL && ucp + 3 != NULL)
	{
		st_redirect_info[ucp[3]].mark = 1 ;
		st_redirect_info[ucp[3]].status = 1;
		st_redirect_info[ucp[3]].time = time(NULL);
		return 0 ;
	}
	return -1 ;
}

int get_redirect_mark_status(struct in_addr ip_src)
{
	unsigned char *ucp = (unsigned char *)&ip_src;
	
	if(ucp != NULL && ucp + 3 != NULL)
	{
		return st_redirect_info[ucp[3]].mark ;
	}
	return -1 ;
}


/*************************************************
Function:		set_redirect_status
Description:	设置重定向信息
Input:			struct in_addr数据包源IP地址:ip_src
Output: 		无
Return: 		0
Others: 			1、更新status为1，表示不需要重定向
				2、更新time为当前时间
Author: 		
*************************************************/
int set_redirect_status(struct in_addr ip_src)
{
	unsigned char *ucp = (unsigned char *)&ip_src;

	if(ucp != NULL && ucp + 3 != NULL)
	{

		st_redirect_info[ucp[3]].status = 1;
	
		st_redirect_info[ucp[3]].time = time(NULL);

		return 0 ;
	}
	//printf("ucp[%s], st_redirect_info[ucp[3]].status[%d] , st_redirect_info[ucp[3]].time[%d]\n" , ucp,st_redirect_info[ucp[3]].status, st_redirect_info[ucp[3]].status );
	printf("convert the  ip pstr to char \n");
	return -1;
}

/*************************************************
Function:		get_redirect_status
Description:	获取是否需要重定向信息
Input:			struct in_addr数据包源IP地址:ip_src
Output: 		无
Return: 		status状态信息，1->不需要重定向；0->需重定向
Others: 		
Author: 		
*************************************************/
int get_redirect_status(struct in_addr ip_src)
{
	unsigned char *ucp = (unsigned char *)&ip_src;

	if(ucp != NULL && ucp + 3 != NULL)
	{
		return st_redirect_info[ucp[3]].status;
	}
	return -1 ;
}

/*设置默认http_redirect_url值，在服务器重定向页面开始下载前、url_record线程初始化时调用该函数*/
void set_default_http_redirect_url(void)
{
	memset(http_redirect_url,0,MAX_FULL_URL_LEN);

	sprintf(http_redirect_url,"http://%s/ddweb/index.html",lan_ip);

	printf("http_redirect_url[%s]\n",http_redirect_url);
	
	return;
}

/*设置新http_redirect_url值，在服务器重定向页面完成下载后调用该函数*/
void set_new_http_redirect_url(const char *new_redirect_url)
{
	if (new_redirect_url != NULL)
	{
		memset(http_redirect_url,0,MAX_FULL_URL_LEN);

		strncpy(http_redirect_url,new_redirect_url,MAX_FULL_URL_LEN);

		printf("http_redirect_url[%s]\n",http_redirect_url);
	}
	
	return;
}


/*************************************************
Function:		http_init_pkt
Description:	构造http数据包，用于重定向
Input:			char *重定向的URL:url
Output: 		无
Return: 		无
Others: 		根据url构造302重定向数据包
Author: 		
*************************************************/
static void http_init_pkt(char *url)
{
	char htmbody[ARRAY_LEN_1024] = {0};

	int n = 0,len=0;
	char *p = htmbody;

	//printf("http_redirection_url [%s]\n",url);
	
	//build html body
	n = sprintf(p, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
	p = p + n;
	n = sprintf(p, "<html><head>\n");
	p = p + n;
	n = sprintf(p, "<title>302 Moved Temporarily</title>\n");
	p = p + n;
	n = sprintf(p, "</head><body>\n");
	p = p + n;
	n = sprintf(p, "<h1>Moved Temporarily</h1>\n");
	p = p + n;
	n = sprintf(p,"<p>The document has moved <a href=\"%s\">here</a>.</p>\n", url);
	p = p + n;
	n = sprintf(p, "<h1></body></html></h1>\n");
	p = p + n;
	n = sprintf(p, "\n");
	p = p + n;

	len = p - htmbody;

	p = htmhead;

	n = sprintf(p, "HTTP/1.1 302 Moved Temporarily\r\n");
	p = p+n;
	n = sprintf(p,"Location: %s\r\n", url);
	p = p+n;

	n= sprintf(p, "Content-Type: text/html; charset=iso-8859-1\r\n");
	p = p+n;
	n= sprintf(p,"Content-Length: %d\r\n", len);
	p = p+n;
	n= sprintf(p, "\r\n");
	p = p+n;
	
	n= sprintf(p, "%s", htmbody);

}



/*************************************************
Function:		return_http_redirection
Description:	重新封装ip数据包，用于重定向
Input:			struct mbuf *数据包:m；
			    char *重定向的URL:http_redirection_url
Output: 		无
Return: 		无
Others: 		直接用ip_output发送新封装好的数据包
Author: 		
*************************************************/
void return_http_redirection(struct mbuf *m , char *http_redirection_url)
{
	struct tcphdr *tcph = NULL;
    	struct ip *ip = NULL;
	struct route ro;   

	int iphlen = 0;
	int off = 0,olen=0,nlen = 0;
	int inc = 0;
	unsigned long src_addr = 0 ,dest_addr = 0 ;
	unsigned short dest_port = 0 ;

	ip = mtod(m, struct ip *);
	if(ip == NULL)
	{
		return ;
	}

	iphlen = ip->ip_hl << 2;
	
	http_init_pkt(http_redirection_url);
	
	tcph = (struct tcphdr *)((unsigned char*)ip + iphlen);
	if(tcph == NULL)
	{
		return ;
	}

	off = iphlen + (tcph->th_off << 2);	
	olen = ntohs(ip->ip_len) -off;
	//printf("===========> \n\n %s \n\n" , htmhead);
	nlen = strlen(htmhead);
	inc = nlen - olen;
	
//must do this
	if (inc < 0)
	{
		m_adj(m, inc);
	}
	
//learn form ip_ftp_pxy.c->ippr_ftp_port()
	m_copyback(m, off, nlen, htmhead);

	src_addr = ip->ip_dst.s_addr;
	dest_addr = ip->ip_src.s_addr;

	bzero(ip,iphlen);

	//make presudo uip header,learn form l2tp_usrreq.c->ifl2tp_output()
	ip->ip_p = IPPROTO_TCP;
	//ip_len = tcp head len+ data_len
	ip->ip_len = htons((tcph->th_off << 2) + nlen);
	ip->ip_src.s_addr = src_addr ;
	ip->ip_dst.s_addr = dest_addr;


	dest_port = tcph->th_sport;
	tcph->th_sport = tcph->th_dport;
	tcph->th_dport = dest_port;

	src_addr = tcph->th_seq;
	dest_addr = tcph->th_ack;

	tcph->th_seq = dest_addr;
	tcph->th_ack = htonl(ntohl(src_addr)+olen);
	tcph->th_win = htons(ntohs(tcph->th_win) - nlen);

	//cksum = tcp_sum((unsigned short *)tcph, ip->ip_len, tcp_pseudo_sum(ip));

	tcph->th_sum = 0;//very important
    	tcph->th_sum =in_cksum(m, iphlen+ntohs(ip->ip_len));
	

	ip->ip_len = iphlen + ntohs(ip->ip_len);
	ip->ip_ttl= 128;
	ip->ip_off|=IP_DF;

	bzero(&ro, sizeof ro);
	
	ip_output(m, 0, &ro, 0, 0);
	
	return;
}


//把mac地址转换成url上传的地址
int convert_post_mac(char *head , char * mac_addr)
{
	if(head == NULL || mac_addr == NULL)
	{
		return 0 ;
	}
	
	if(head != NULL && (head +11) != NULL)
	{
		sprintf(mac_addr , "%02x%%3A%02x%%3A%02x%%3A%02x%%3A%02x%%3A%02x" , head[6] & 0xFF , head[7] &0xFF , head[8] & 0xFF , 
			head[9] & 0xFF , head[10] & 0xFF , head[11] & 0xFF);
	}
	else
	{
		strcpy(mac_addr , "00%%3A00%%3A00%%3A00%%3A00%%3A00");
	}
	return 0 ;
}

int isSpeciDsnReply(struct mbuf *m)
{
	struct ip *ip = NULL;
	struct udphdr *udp = NULL ;
	char *data = NULL ;
	int pk_len = 0 ;
	char dns_query_name[ARRAY_LEN_256] = {0} ;
	
	if ( !m  || m->m_pkthdr.len < 20)
	{
		return URLF_PASS;
	}
	
	ip = mtod(m, struct ip *);
	if(ip == NULL)
	{
		return URLF_PASS ;
	}

	if (ip->ip_p != IPPROTO_UDP)
	{
		return URLF_PASS ;
	}
	
	udp = (struct udphdr *)((char *)ip + (ip->ip_hl << 2));
	if(udp == NULL)
	{
		return URLF_PASS ;
	}
	if(udp->uh_sport == htons(53))
	{	
		data = (char *)udp + sizeof(struct udphdr) ;
		
		SDWDNS* dns = (SDWDNS*)data ;
		
		if(data == NULL || dns == NULL)
		{
			return URLF_PASS;
		}	
		pk_len = m->m_pkthdr.len - sizeof(struct udphdr) - (ip->ip_hl << 2);
		
		memset(dns_query_name , 0, sizeof(dns_query_name));
		
		dnsmasq_parse_replay(dns, pk_len, dns_query_name) ;

		dns_query_name[ARRAY_LEN_256 -1] = '\0' ;
		
		if( 0 == strcmp(SDW_CLIENT_REQUERY_DNS , dns_query_name))
		{
			printf("----------------free the mark reply dns---------------------\n");
			//m_freem(m);//二级路由问题，释放dns包
			return URLF_BLOCK ;
		}
	}
	return URLF_PASS ;
}

int is_in_data_rule(char *http_hdr,  int http_hdr_len)
{
	char host_str[64] = {0};
	char *host_p, *host_q;
	int host_url_len;
	host_p = http_hdr_find_field(http_hdr, http_hdr_len, HOST_STR, HOST_LEN);
	if(NULL == host_p)
	{
		return -1;
	}
	else
	{
		host_p += HOST_LEN;
		host_q = strchr(host_p, '\r');
		if(!host_q)
		{
			diag_printf("[%s][%d] host_q == NULL\n", __FUNCTION__, __LINE__);
		}
		host_url_len = (int)(host_q - host_p);
		memcpy(host_str, host_p, host_url_len);
		host_str[host_url_len] = '\0';
		//diag_printf("[%s][%d] host_str = %s\n", __FUNCTION__, __LINE__, host_str);
		host_p = strrchr(host_str, '.');
		if(NULL != host_p)
		{
			host_q = host_p;
			*host_p = '\0';
			host_p = strrchr(host_str, '.');
			*host_q = '.';
			if(NULL != host_p)
			{
				host_q = host_p+1;
			}
			else
			{
				host_q = host_str;
			}
			//diag_printf("[%s][%d] host url %s\n", __FUNCTION__, __LINE__, host_q);

			if(1 == is_in_rule(data_rule_hash, DATA_RULE_HASH_LEN, host_q, strlen(host_q)))
			{
				//diag_printf("[%s][%d] %s is is_in_rule\n", __FUNCTION__, __LINE__, host_q);
				return 1;
			}
			else
			{
				//diag_printf("[%s][%d] %s is not is_in_rule\n", __FUNCTION__, __LINE__, host_q);
			}
		}
	}

	return 0;
}

int is_in_data_file(char *http_hdr,  int http_hdr_len)
{
	char head_line[1024] = {0};
	char *char_p = NULL;
	int head_line_len = 0;

	char_p = strchr(http_hdr, '\r');
	if(NULL == char_p)
	{
		return -1;
	}

	char_p -= 8;
	if(memcmp(char_p, "HTTP/1.", strlen("HTTP/1.")) != 0)
		return -1;
	
	char_p -= 1;
	head_line_len = (int)(char_p - http_hdr);
	memcpy(head_line, http_hdr, head_line_len);
	head_line[head_line_len] = '\0';

	char_p = strchr(head_line, '?');
	if(char_p)
	{
		*char_p = '\0';
	}

	char_p = strrchr(head_line, '.');
	if(char_p)
	{
		char_p += 1;
		//diag_printf("[%s][%d] file type %s\n", __FUNCTION__, __LINE__, char_p);
		if(1 == is_in_rule(data_file_hash, DATA_FILE_HASH_LEN, char_p, strlen(char_p)))
		{
			//diag_printf("[%s][%d] %s is is_in_rule\n", __FUNCTION__, __LINE__, char_p);
			return 1;
		}
		else
		{
			//diag_printf("[%s][%d] %s is not is_in_rule\n", __FUNCTION__, __LINE__, char_p);
		}
	}

	return 0;
}

int nis_fastcheck_hook(struct ifnet *ifp, char *head, struct mbuf *m)
{
	struct ip *ip = NULL;
	struct tcphdr *tcp = NULL ;
	char *data = NULL;
	char *char_p = NULL, *char_q = NULL, *http_hdr_end = NULL;
	int hdr_left_len = 0, ret = 0;
	int pk_len = 0 ;
	int d_method = -1; 

	if(0 == url_thread_start)
		return URLF_PASS;

	if(head == NULL || ifp == NULL || m == NULL)
	{
		return URLF_PASS;
	}

	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) == 0)
	{
		return URLF_PASS;
	}
	
	if (m->m_pkthdr.len < 40)
	{
		return URLF_PASS;
	}
	ip = mtod(m, struct ip *);
	if(ip == NULL)
	{
		return URLF_PASS ;
	}

	if (ip->ip_p != IPPROTO_TCP)
	{
		return URLF_PASS ;
	}

	tcp = (struct tcphdr *)((char *)ip + (ip->ip_hl << 2));
	if(tcp == NULL)
	{
		return URLF_PASS ;
	}

	if (tcp->th_flags & (TH_RST | TH_FIN | TH_SYN))
		return URLF_PASS;
	
	if((htons(80) == tcp->th_dport)|| (htons(80) == tcp->th_sport))
	{
		;
	}
	else
		return URLF_PASS;
	
	data = (char *)tcp + (tcp->th_off << 2);
	
	if(data == NULL)
	{
		return URLF_PASS ;
	}
	pk_len = m->m_pkthdr.len - (tcp->th_off << 2) - (ip->ip_hl << 2);

	if(pk_len <= 5 )
	{
		return URLF_PASS ;
	}
	
	if (check_lan_ip(ip->ip_src,ip->ip_dst))
	{
		return URLF_PASS;
	}

	d_method = get_http_method(data);
	if(-1 == d_method)
	{
		return URLF_PASS;
	}

	char_q = strchr(data, '\r');
	if(NULL==char_q)
	{
		return URLF_PASS;
	}
	char_q -= 8;
	if(0 != memcmp(char_q, "HTTP/1.", 7))
	{
		return URLF_PASS;
		return -1;
	}

	char_p = strstr(data, "\r\n\r\n");
	if(NULL==char_p)
	{
		return URLF_PASS;
	}
	http_hdr_end = char_p;
	hdr_left_len = (int)(char_p - data);

	if(1 == use_data_rule)
	{
		ret = is_in_data_rule(data, hdr_left_len);
		if(1 != ret)
			return URLF_PASS;
	}

	if(1 == use_data_file)
	{
		ret = is_in_data_file(data, hdr_left_len);
		if(-1 == ret || 1 == ret)
			return URLF_PASS;
	}

	if(url_record_cur_num < URL_RECORD_MAX_NUM)
	{
		url_log_add_content(ip, data, hdr_left_len, http_hdr_end, d_method);
	}
	
	return URLF_PASS ;

}

int init_data_rule(char *data_rule)
{
	char *url_p, *file_p, *post_len;
	int ret;
    
	url_p = strtok(data_rule, DIFF_RULE_DELIM);
	file_p = strtok(NULL, DIFF_RULE_DELIM);
	post_len = strtok(NULL, DIFF_RULE_DELIM);
	if(!post_len)
		post_len = "300";

	if(strcmp(url_p, "all") == 0)
	{
		use_data_rule = 0;
	}
	else
	{
		ret = init_rule_hash(url_p, data_rule_hash, DATA_RULE_HASH_LEN);
		if(0 != ret)
		{
			diag_printf("[%s][%d] init_rule_hash failed\n", __FUNCTION__, __LINE__);
			use_data_rule = 0;
			return -1;
		}
		else
		{
			use_data_rule = 1;
		}
	}


	if(strcmp(file_p, "all") == 0)
	{
		use_data_file = 0;
	}
	else
	{
		ret = init_rule_hash(file_p, data_file_hash, DATA_FILE_HASH_LEN);
		if(0 != ret)
		{
			diag_printf("[%s][%d] init_rule_hash failed\n", __FUNCTION__, __LINE__);
			use_data_file = 0;
			return -1;
		}
		else
		{
			use_data_file = 1;
		}
	}

	max_post_len = atoi(post_len);

	return 0;
}

void print_data_rule(void)
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	printf("data_rule_hash : \n");
	print_rule_hash(data_rule_hash, DATA_RULE_HASH_LEN);
	printf("data_file_hash : \n");
	print_rule_hash(data_file_hash, DATA_FILE_HASH_LEN);
	printf("max_post_len = %d\n", max_post_len);
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	return;
}

