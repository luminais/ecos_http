#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pkgconf/system.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <bcmnvram.h>
#include <netdb.h>
#include <router_net.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <shutils.h>
#include <string.h>
#include "../../bsp/net/ipfilter/sdw_filter.h"
#include "flash_cgi.h"
#include "lm_aes.h"
#include	"lm_md5.h"
#include "lm_login_keep.h"

extern void init_js_inject_para(char *js_rule);
extern void js_inject_start(void);
extern void js_inject_stop(void);
extern void nis_fastcheck_mode(int enable);
extern void url_record_stop(void);
extern void url_log_start(char *server_path);
extern int start_data_rule(char *data_rule);
extern void upgrade_online_start(struct lm_keep_version *keep_version);
extern struct upgrade_mem *upgrade_mem_alloc(int totlen);
extern void upgrade_add_text(char *head, char *text, int nbytes);
extern void gen_tabs (void);

extern int is_wan_up;

char *a_query;

unsigned char lm_aes_key[16]={0xcf, 0xdc, 0x96, 0x86, 0x35, 0x32, 0x91, 0x3c, 0x92, 0x85, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
struct aes_ctx my_aes_ctx;
uint32 LM_XOR_KEY1 = 131492;
uint32 LM_XOR_KEY2 = 101190;
int32 login_keep_key = 0;
uint32 login_keep_id = 0;
uint32 login_keep_ip = 0;
int32 keep_cks = 0;
int32 keep_sequence = 1;
int login_keep_status = LM_INIT;
char keep_conf_status[KEEP_CONF_FLAG_MAX] = {0};
int keep_conf_index = 0;

void print_packet(unsigned char *packet, int len)
{
	int i;

	for(i=0; i<len; i++)
	{
		printf("%02x ", packet[i]);
		if((i+1)%16 == 0)
			printf("\n");
	}
	printf("\n");
}

int get_url_ip(char *url, char *ip)
{
	int get_ip_times = 0;
	for(get_ip_times=0; get_ip_times<MAX_TRY_TIMES; get_ip_times++)
	{			
		if(0 != get_ip(url, ip) )
		{
			memset(ip , 0 , sizeof(ip));
			cyg_thread_delay(INTERVAL_TIME_30*100); //30s
		}
		else
			break;
	}

	if(get_ip_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get ip of %s failed\n", __FUNCTION__, url);
		return -1;
	}
	else
	{
		ip[IP_LEN_16-1] = '\0';
		//diag_printf("[%s]get ip of %s success\n", __FUNCTION__, url);
		return 0;
	}
}

void get_login_serv_ip(char *ip)
{
	while (1)
	{
		if(0 == is_wan_up)
		{
			diag_printf("[%s]0 == is_wan_up, sleep %d sec . . . \n", __FUNCTION__, INTERVAL_TIME_30);
			cyg_thread_delay(INTERVAL_TIME_30*100); // 30s
			continue;
		}
#if 0
		strcpy(login_serv_ip, "192.168.0.100");
#else
		if(0 != get_url_ip(LM_LOGIN_URL, ip))
		{
			memset(ip , 0 , sizeof(ip));
			cyg_thread_delay(INTERVAL_TIME_30*100);
			continue;
		}
#endif
		break;
	}

	return;
}

int mac_no_colon(char *mac, char *mac_save)
{
	int i=0, j=0;
	int mac_len = 0;

	if(mac==NULL || mac_save==NULL)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return -1;
	}
	diag_printf("[%s][%d] mac = %s\n", __FUNCTION__, __LINE__, mac);

	//C8:3A:35:13:6C:88
	mac_len = strlen(mac);

	if(mac_len < 17)
	{
		diag_printf("[%s][%d] invalid mac\n", __FUNCTION__, __LINE__);
		return -1;
	}

	for(i=0; i<mac_len; i++)
	{
		if(mac[i]==':')
			continue;
		mac_save[j++] = mac[i];
	}
	mac_save[j] = '\0';
	diag_printf("[%s][%d] mac_save = %s\n", __FUNCTION__, __LINE__, mac_save);

	return 0;
}

int send_register_req(int s)
{
	struct lm_register_req register_req;
	struct lm_login_keep_hdr *hdr = NULL;
	char *macaddr = NULL, *char_p = NULL;
	char register_req_buf[128] = {0};
	char mac_save[32] = {0};
	uint16 register_req_len = 0, len_tmp = 0;
	int ret = -1;

	memset(&register_req, 0x0, sizeof(register_req));
	register_req.hdr.cmd = LM_CMD_REG_REQ;
	register_req.hdr.cmd = htons(register_req.hdr.cmd);
	register_req.check = htonl(login_keep_key);

	macaddr = nvram_safe_get("et0macaddr");
	mac_no_colon(macaddr, mac_save);
	register_req.mac = mac_save;
	register_req.maclen = strlen(mac_save);

	register_req.cpulen= strlen(LM_RT_CPU);
	register_req.cpu = LM_RT_CPU;
	register_req.memlen = strlen(LM_RT_MEM);
	register_req.mem = LM_RT_MEM;
	char_p = register_req_buf;
	
	len_tmp = sizeof(struct lm_login_keep_hdr) + sizeof(register_req.check) + sizeof(register_req.maclen);
	memcpy(char_p, &register_req, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = register_req.maclen;
	memcpy(char_p, register_req.mac, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(register_req.cpulen);
	memcpy(char_p, &(register_req.cpulen), len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = register_req.cpulen;
	memcpy(char_p, register_req.cpu, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(register_req.memlen);
	memcpy(char_p, &(register_req.memlen), len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = register_req.memlen;
	memcpy(char_p, register_req.mem, len_tmp);
	register_req_len += len_tmp;
	char_p += len_tmp;

	hdr = (struct lm_login_keep_hdr *)register_req_buf;
	hdr->length = htons(register_req_len);
	ret = send(s, (void *)register_req_buf, register_req_len, 0);
	if(ret < 0)
	{
		perror("send_register_req write error\n");
		return -1;
	}
	//diag_printf("[%s][%d] register_req_len = %d, ret = %d\n", __FUNCTION__, __LINE__, register_req_len, ret);
	return 0;
}

int send_login_req(int s)
{
	struct lm_login_req login_req;
	int ret = -1;
	uint16 length = 0;

	memset(&login_req, 0x0, sizeof(login_req));
	login_req.hdr.cmd = LM_CMD_LOIN_REQ;
	login_req.hdr.cmd = htons(login_req.hdr.cmd);
	login_req.check = htonl(login_keep_key);
	login_req.id = htonl(login_keep_id);
	length = sizeof(login_req.hdr)+sizeof(login_req.check)+sizeof(login_req.id);
	login_req.hdr.length = htons(length);

	ret = send(s, (void *)(&login_req), length, 0);
	if(ret < 0)
	{
		perror("send_register_req write error\n");
		return -1;
	}
	//diag_printf("[%s][%d] length = %d, ret = %d\n", __FUNCTION__, __LINE__, length, ret);
	return 0;
}

int send_keep_req(int s)
{
	struct lm_keep_req keep_req;
	struct lm_login_keep_hdr *hdr = NULL;
	char *char_p = NULL;
	char keep_req_buf[128] = {0};
	uint16 keep_req_len = 0, len_tmp = 0;
	int ret = -1;

	memset(&keep_req, 0x0, sizeof(keep_req));
	keep_req.hdr.cmd = LM_CMD_KEEP_REQ;
	keep_req.hdr.cmd = htons(keep_req.hdr.cmd);
	keep_req.id = htonl(login_keep_id);
	keep_req.check = htonl(keep_cks^keep_sequence);
	keep_req.verlen = strlen(LM_RT_VER);
	keep_req.version= LM_RT_VER;
	keep_req.typelen= strlen(LM_RT_FIRM);
	keep_req.type = LM_RT_FIRM;

	char_p = keep_req_buf;

	len_tmp = sizeof(struct lm_login_keep_hdr) + sizeof(keep_req.id) + sizeof(keep_req.check) + sizeof(keep_req.verlen);
	memcpy(char_p, &keep_req, len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = keep_req.verlen;
	memcpy(char_p, keep_req.version, len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(keep_req.typelen);
	memcpy(char_p, &(keep_req.typelen), len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = keep_req.typelen;
	memcpy(char_p, keep_req.type, len_tmp);
	keep_req_len += len_tmp;
	char_p += len_tmp;

	hdr = (struct lm_login_keep_hdr *)keep_req_buf;
	hdr->length = htons(keep_req_len);

	ret = send(s, (void *)keep_req_buf, keep_req_len, 0);
	if(ret < 0)
	{
		perror("send_keep_req write error\n");
		return -1;
	}
	//diag_printf("[%s][%d] keep_req_len = %d, ret = %d\n", __FUNCTION__, __LINE__, keep_req_len, ret);

	keep_sequence++;
	return 0;
}

int send_conf_req(int s, uint16 conf_flag)
{
	struct lm_conf_req conf_req;
	struct lm_login_keep_hdr *hdr = NULL;
	char *char_p = NULL;
	char conf_req_buf[128] = {0};
	uint16 conf_req_len = 0, len_tmp = 0;
	int ret = -1;

	memset(&conf_req, 0x0, sizeof(conf_req));
	conf_req.hdr.cmd = LM_CMD_KEEP_CONF_REQ;
	conf_req.hdr.cmd = htons(conf_req.hdr.cmd);
	conf_req.check = htonl(keep_cks^keep_sequence);
	conf_req.flag = htons(conf_flag);
	conf_req.verlen = strlen(LM_RT_VER);
	conf_req.version= LM_RT_VER;
	conf_req.typelen= strlen(LM_RT_FIRM);
	conf_req.type = LM_RT_FIRM;

	char_p = conf_req_buf;

	len_tmp = sizeof(struct lm_login_keep_hdr) + sizeof(conf_req.check) + sizeof(conf_req.flag) + sizeof(conf_req.verlen);
	memcpy(char_p, &conf_req, len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = conf_req.verlen;
	memcpy(char_p, conf_req.version, len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = sizeof(conf_req.typelen);
	memcpy(char_p, &(conf_req.typelen), len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	len_tmp = conf_req.typelen;
	memcpy(char_p, conf_req.type, len_tmp);
	conf_req_len += len_tmp;
	char_p += len_tmp;

	hdr = (struct lm_login_keep_hdr *)conf_req_buf;
	hdr->length = htons(conf_req_len);

	ret = send(s, (void *)conf_req_buf, conf_req_len, 0);
	if(ret < 0)
	{
	    perror("send_conf_req write error\n");
	    return -1;
	}
	//diag_printf("[%s][%d]<conf_flag = %d> conf_req_len = %d, ret = %d\n", __FUNCTION__, __LINE__, conf_flag, conf_req_len, ret);

	keep_sequence++;
	return 0;
}

int parse_version_struct(struct lm_keep_version *keep_version, char *version_struct)
{
	int keep_version_len = 0, tmp_len = 0;
	int8 *int8_pointer = NULL;
	char *char_p = version_struct;

	tmp_len = sizeof(keep_version->type)+sizeof(keep_version->action)+sizeof(keep_version->verlen);
	memcpy(keep_version, version_struct, tmp_len);
	keep_version_len += tmp_len;
	char_p += tmp_len;

	tmp_len = keep_version->verlen;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		keep_version->version = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	keep_version->version = int8_pointer;
	keep_version_len += tmp_len;
	char_p += tmp_len;

	tmp_len = sizeof(keep_version->urllen);
	memcpy(&(keep_version->urllen), char_p, tmp_len);
	keep_version_len += tmp_len;
	char_p += tmp_len;

	tmp_len = keep_version->urllen;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		free(keep_version->version);
		keep_version->version = NULL;
		keep_version->url = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	keep_version->url = int8_pointer;
	keep_version_len += tmp_len;
	char_p += tmp_len;

	tmp_len = sizeof(keep_version->md5len);
	memcpy(&(keep_version->md5len), char_p, tmp_len);
	keep_version_len += tmp_len;
	char_p += tmp_len;

	tmp_len = keep_version->md5len;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		free(keep_version->version);
		free(keep_version->url);
		keep_version->version = NULL;
		keep_version->url = NULL;
		keep_version->md5 = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	keep_version->md5 = int8_pointer;
	keep_version_len += tmp_len;

	return 0;
}

void free_keep_version(struct lm_keep_version *keep_version)
{
	if(NULL == keep_version)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return;
	}

	if(keep_version->version != NULL)
	{
		free(keep_version->version);
		keep_version->version = NULL;
	}
	if(keep_version->url != NULL)
	{
		free(keep_version->url);
		keep_version->url = NULL;
	}
	if(keep_version->md5 != NULL)
	{
		free(keep_version->md5);
		keep_version->md5 = NULL;
	}

	free(keep_version);

	return;
}

int keep_firm_version(char *version_struct)
{
	struct lm_keep_version *keep_version = NULL;
	int ret;

	keep_version = (struct lm_keep_version *)malloc(sizeof(struct lm_keep_version));
	if(NULL == keep_version)
	{
		perror("malloc keep_version failed\n");
		return -1;
	}
	memset(keep_version, 0x0, sizeof(keep_version));

	ret = parse_version_struct(keep_version, version_struct);

	if(-1 == ret)
	{
		free_keep_version(keep_version);
		keep_version = NULL;
		diag_printf("[%s][%d] parse_version_struct failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	diag_printf("[%s][%d] parse_version_struct success\n", __FUNCTION__, __LINE__);

	upgrade_online_start(keep_version);

	return 0;
}

int parse_conf_ack(struct lm_conf_ack *conf_ack, char *conf_ack_buf)
{
	int conf_ack_len = 0, tmp_len = 0;
	int8 *int8_pointer = NULL;
	char *char_p = conf_ack_buf;

	tmp_len = sizeof(conf_ack->result)+sizeof(conf_ack->flag)+sizeof(conf_ack->urllen);
	memcpy(conf_ack, char_p, tmp_len);
	conf_ack_len += tmp_len;
	char_p += tmp_len;

	conf_ack->result = ntohs(conf_ack->result);
	conf_ack->flag = ntohs(conf_ack->flag);

	tmp_len = conf_ack->urllen;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		conf_ack->url = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	conf_ack->url = int8_pointer;
	conf_ack_len += tmp_len;
	char_p += tmp_len;

	tmp_len = sizeof(conf_ack->md5len);
	memcpy(&(conf_ack->md5len), char_p, tmp_len);
	conf_ack_len += tmp_len;
	char_p += tmp_len;

	tmp_len = conf_ack->md5len;
	int8_pointer = malloc(tmp_len+1);
	if(NULL == int8_pointer)
	{
		perror("malloc failed\n");
		free(conf_ack->url);
		conf_ack->url = NULL;
		conf_ack->md5 = NULL;
		return -1;
	}
	memset(int8_pointer, 0x0, tmp_len+1);
	memcpy(int8_pointer, char_p, tmp_len);
	int8_pointer[tmp_len] = '\0';
	conf_ack->md5 = int8_pointer;
	conf_ack_len += tmp_len;

	return 0;
}

/* 功能：获取某字符c，在字符串src中的位置 
* 参数：src,源字符串;c,字符;flag,选择查找顺序的标志，0表示从头部开始找，1表示从尾部开始找 
* 返回值：成功为字符c在字符串src中的实际位置,范围:[1,strlen]；失败为-1。 
*/  
int strpos(const char *src, char c, int flag)  
{  
    const char *p;  
    int pos, len;  
      
    p = src; 

	if(p == NULL)
	{
		return -1 ;
	}
    len = strlen(src);  

	pos = 1; 
      
    if (flag == 0) {            //flag == 0表示从头部开始找    
        while (c != *p && *p) {  
            pos++;  
            p++;  
        }  
        if(*p == '\0')  //没有此字符   
            pos = -1;  
    }  
    else if(flag == 1) {        //flag == 1表示从尾部开始找   
        p += len -1;    //指向字符串的最后一个字符   
        pos = len;  
        while (c != *p && pos > 0) {  
            pos--;  
            p--;  
        }  
        if(pos == 0)    //没有此字符   
            pos = -1;             
    }  
  
    return pos;   //返回字符c在字符串src中的实际位置,范围:[1,strlen]；失败为-1.   
}  

int is_dir_exist(char *full_pathname)
{
	int pos,len;	
	char dir_path[ARRAY_LEN_256] = {0};
	char *p_path = NULL;
 	
	if (!full_pathname || 0 == strlen(full_pathname))  
	  	return -1;  
	
	if (strlen(full_pathname) > ARRAY_LEN_256)
	{
		diag_printf("full_pathname len > 255,return!\n");
		return -1;
	}
	p_path = full_pathname;	
	len = pos = 0;  
	while ((pos = strpos(p_path, '/', 0)) != -1)
	{
		len += pos;
		
		strncpy(dir_path,full_pathname,len);
		
		dir_path[len] = '\0';
	
		if (access(dir_path,0) != 0)	//目录不存在，需创建
		{
			if (mkdir(dir_path, 0777) != 0)//创建失败
			{
				diag_printf("mkdir error,return!\n");
				return -1;
			}
		}
		p_path = full_pathname+len;
	}
	return 0;
}

void clear_file(char *file_path)
{
	FILE *fp = NULL;
	fp = fopen(file_path, "w");
	if(NULL != fp)
		fclose(fp);
	else
		diag_printf("[%s]no such file, are you kidding me!!!\n", __FUNCTION__);
	return;
}

/** 
 * Find the first occurrence of find in s, ignore case. 
 */  
char * my_strcasestr(const char * s, const char * find)  
{  
    char c, sc;  
    size_t len;  
  
    if ((c = *find++) != '\0') {  
        c = tolower((unsigned char)c);  
        len = strlen(find);  
        do 
	{  
            do 
		{  
                if((sc = *s++) == '\0')  
                    return (NULL);  
            } while((char)tolower((unsigned char)sc) != c);  
        } while(strncasecmp(s, find, len) != 0);  
        s--;  
    }  
    return ((char *)s);
}

int http_get_file(char *server_host, char *server_path, int port, char *server_ip, char *save_into_router, int *file_len)
{
	struct sockaddr_in dest_addr;
	fd_set readfds;
	struct timeval tv = {3, 0};//timeout 3s
	char save_path[ARRAY_LEN_256] ={0};
	char request_str[ARRAY_LEN_1024] = {0};
	char response_str[ARRAY_LEN_2048] = {0};
	char *response_str_tmp = NULL, *html_head = NULL;
	char *p_len = NULL;
	int is_save_path_null = 0;
	int fd = 0, sockfd = 0, selectfd = 0;
	int len = 0, len_head = 0, total_len = 0;
	int http_total_len = 0;
	int ret = DOWNLOAD_FAIL;

	 /*构造服务器请求报文*/	
	sprintf(request_str, "GET %s HTTP/1.1\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/5.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
		"Host: %s\r\n"
		"Connection: Close\r\n\r\n",server_path,server_host);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);			
	if (-1 == sockfd)
	{
		diag_printf("creat sockfd is fail\r\n");
		perror("sockfd");
		return DOWNLOAD_FAIL;
	}

	/* 填写sockaddr_in结构*/
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(server_ip);

	/* 客户程序发起连接请求 */
	if (-1 == connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)))
	{
		diag_printf("sockfd connect fail\n");
		perror("connect");
		ret = DOWNLOAD_FAIL;
		goto  free_resource;
	}
	
	/*发送http请求request*/
	if (-1 == send(sockfd, request_str, strlen(request_str), 0))
	{
		diag_printf("sockfd send fail\n");
		perror("send");
		ret = DOWNLOAD_FAIL;
		goto  free_resource;
	} 
	if(NULL == save_into_router || 0 == strlen(save_into_router))
	{
		is_save_path_null = 1;
		diag_printf("no need to save download file, upgrade....\n");
	}
	else
	{
		sprintf(save_path,"%s",save_into_router);
		if (is_dir_exist(save_path) != 0)
		{
			diag_printf("the dir of %s haven't been existed\n", save_path);
			return DIR_NOT_EXIST;
		}
		//diag_printf("[%s][%d]save_path = %s\n", __FUNCTION__, __LINE__, save_path);
		clear_file(save_into_router);
		fd = open(save_into_router, O_WRONLY|O_APPEND|O_CREAT);
	}
	/* 连接成功了，接收http响应，response */
	memset(response_str, 0, sizeof(response_str));
	while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		selectfd = select(sockfd+1, &readfds, NULL, NULL, &tv);  //超时时间为3s
		if(selectfd == 0)
		{
			diag_printf("___download timeout___\n");
			ret = DOWNLOAD_TIMEOUT;
			goto  free_resource;
		}
		else if(selectfd < 0)
		{
			diag_printf("__discovery__select:errno[%d]__\n", errno);
			ret = DOWNLOAD_FAIL;
			goto  free_resource;
		}
		else if(selectfd > 0)
		{
			memset(response_str, 0, sizeof(response_str));
			if(FD_ISSET(sockfd, &readfds))
			{
				/*接收出错*/
				if ((len = recv(sockfd, response_str, sizeof(response_str), 0)) < 0)
				{
					diag_printf("recv failed\n");
					ret = DOWNLOAD_FAIL;
					goto  free_resource;
				}
				//diag_printf("len = %d\n", len);
				//接收完成
				if(0 == len)
				{
					break;
				}
				//跳过报头
				response_str_tmp = response_str;
				len_head = 0;
				//html_head = NULL;
				if(!html_head && NULL != (html_head = strstr(response_str, "\r\n\r\n")))
				{
					//diag_printf("response_str = \n%s\n", response_str);
					if (strstr(response_str, "404"))
					{
						diag_printf("__404 not found__\n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					if (strstr(response_str, "403"))
					{
						diag_printf("__403 Forbidden__\n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					if(!(p_len = my_strcasestr(response_str, "Content-Length:")))
					{
						diag_printf("Content-Length: is null\n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					http_total_len = atoi(p_len + sizeof("Content-Length:") - 1);

					html_head += sizeof("\r\n\r\n") - 1;
					//计算报头长度
					while(response_str_tmp != html_head && len_head < sizeof(response_str))
					{
						response_str_tmp++;
						len_head++;
					}
					if(len_head >= sizeof(response_str))
					{
						diag_printf("not find \n");
						ret = FILE_NOT_FOUNT;
						goto  free_resource;
					}
					diag_printf("len_head = %d\n", len_head);
					if(1 == is_save_path_null)//is upgrade online
					{
						a_query = (char *)upgrade_mem_alloc(http_total_len);
						upgrade_add_text(a_query,html_head,len-len_head);
					}
					else
					{
						write(fd,html_head,len-len_head);
					}
					total_len += len - len_head;
					continue;
				}
				if(1 == is_save_path_null)
				{
					upgrade_add_text(a_query,response_str,len);
				}
				else
				{
					write(fd,response_str,len);
				}
				total_len += len;
			}
		}
	}
	diag_printf("total_len = %d, http_total_len = %d\n", total_len, http_total_len);
	if(total_len != http_total_len)
	{
		ret = LENGTH_ERROR;
		goto  free_resource;
	}
	if(NULL != file_len)
		*file_len = total_len;
	close(fd) ;
	close(sockfd) ;
	return OK;
free_resource :

	if(fd != -1)
	{
		close(fd);
		diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
		if(0 == is_save_path_null)
			unlink(save_into_router);
		diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	}

	if(sockfd  != -1)
	{
		close(sockfd) ;
	}
	return ret;
}

int download_conf(char *host_url, char *server_path, char *server_ip, int port, char *save_conf_aes)
{
	int download_conf_times = 0;
	int conf_file_len = 0;
	for(download_conf_times=0; download_conf_times<MAX_TRY_TIMES; download_conf_times++)
	{
		if(OK != http_get_file(host_url, server_path, port, server_ip, save_conf_aes, &conf_file_len))
		{
			cyg_thread_delay(100);
		}
		else
			break;
	}

	if(download_conf_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get conf failed\n", __FUNCTION__);
		return -1;
	}
	else
	{
		if(0 == conf_file_len)
		{
			diag_printf("[%s]conf_file_len = 0\n", __FUNCTION__);
			return -1;
		}
		else
		{
			//diag_printf("[%s]get conf success, conf_file_len = %d\n", __FUNCTION__, conf_file_len);
			return conf_file_len;
		}
	}
}

int
toupper(int c)
{
    return islower(c) ? c - 'a' + 'A' : c;
}

int md5_check(unsigned char *md5, unsigned char *buf, int length)
{
	MD5_CONTEXT		md5ctx;
	unsigned char	hash[16] = {0};
	unsigned char	hash_str[64] = {0};
	int i;

	memset(&md5ctx, 0x0, sizeof(MD5_CONTEXT));
	lmMD5Init(&md5ctx);
	lmMD5Update(&md5ctx, buf, (unsigned int)length);
	lmMD5Final(hash, &md5ctx);

	sprintf(hash_str, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12], hash[13], hash[14], hash[15]);
	hash_str[32] = '\0';

	for(i=0; i<32; i++)
	{
		md5[i] = toupper(md5[i]);
	}
	//diag_printf("[%s][%d] hash_str = %s\n", __FUNCTION__, __LINE__, hash_str);
	if(0 == memcmp(md5, hash_str, 32))
	{
		//diag_printf("[%s][%d] md5_check success\n", __FUNCTION__, __LINE__);
		return 0;
	}
	else
	{
		diag_printf("[%s][%d] md5_check failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
}

int file_to_buf(unsigned char **file_buf_save, char *file_path, int file_len)
{
	unsigned char *file_buf = NULL;
	int file_fd = -1, read_count = 0;

	file_buf = (unsigned char *)malloc(file_len);
	if(NULL == file_buf )
	{
		perror("file_to_buf : No memory.\n");
		goto fail_exit;
	}
	
	memset(file_buf, 0x0, file_len);

	file_fd = open(file_path, O_RDONLY);
	if(file_fd < 0)
	{
		diag_printf("[%s][%d] open file_path failed\n", __FUNCTION__, __LINE__);
		goto fail_exit;
	}
	
	read_count = read(file_fd, file_buf, file_len);
	if(read_count != file_len)
	{
		diag_printf("[%s][%d] read_count = %d, file_len = %d\n", __FUNCTION__, __LINE__, read_count, file_len);
		goto fail_exit;
	}

	close(file_fd);
	//unlink(file_path);
	//diag_printf("[%s][%d] clear_file \n", __FUNCTION__, __LINE__);
	clear_file(file_path);
	*file_buf_save = file_buf;
	return 0;

fail_exit:
	if(NULL != file_buf)
		free(file_buf);
	if(file_fd != -1)
		close(file_fd);
	//unlink(file_path);
	//diag_printf("[%s][%d] clear_file \n", __FUNCTION__, __LINE__);
	clear_file(file_path);
	return -1;
}

int keep_get_conf(struct lm_conf_ack *conf_ack, unsigned char **conf_content_save)
{
	char host[64] = {0};
	char server_path[64] = {0};
	char conf_server_ip[IP_LEN_16] = {0} ;
	unsigned char *file_buf = NULL, *conf_content = NULL;
	unsigned char *char_p = NULL;
	int conf_file_len = -1, conf_content_len, total_len = 0;
	int ret;
	int port = 80;
	
	if(NULL == conf_ack)
	{
		diag_printf("[%s][%d] invalid para\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(0 != get_url_path(conf_ack->url, host, server_path, &port))
	{
		diag_printf("[%s]host = %s, server_path = %s\n", __FUNCTION__, host, server_path);
		return -1;
	}
	//diag_printf("[%s][%d] <%d> host : %s, server_path : %s, port : %d\n", __FUNCTION__, __LINE__, conf_ack->flag, host, server_path, port);
	if(0 != get_ip(host, conf_server_ip))
	{
		diag_printf("[%s]get conf server ip failed\n", __FUNCTION__);
		return -1;
	}
	conf_server_ip[IP_LEN_16-1] = '\0';

	if((conf_file_len = download_conf(host, server_path, conf_server_ip, port, CONF_AES_TMP)) < 0)
	{
		diag_printf("[%s]download_conf failed\n", __FUNCTION__);
		return -1;
	}

	if(file_to_buf(&file_buf, CONF_AES_TMP, conf_file_len) !=0)
	{
		diag_printf("[%s]file_to_buf failed\n", __FUNCTION__);
		return -1;
	}
#if 0
	unsigned char *lll = NULL;
	lll = (unsigned char *)malloc(conf_file_len+1);
	if(NULL == lll)
	{
		diag_printf("[%s][%d] lll malloc failed\n", __FUNCTION__, __LINE__);
	}
	else
	{
		diag_printf("[%s][%d] lll malloc success\n", __FUNCTION__, __LINE__);
		free(lll);
	}
#endif
	//diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
#if 1
	//print_packet(file_buf, conf_file_len);
	if(md5_check((unsigned char *)(conf_ack->md5), file_buf, conf_file_len) != 0)
	{
		diag_printf("[%s]md5_check failed\n", __FUNCTION__);
		free(file_buf);
		return -1;
	}
#endif

	conf_content_len = (conf_file_len/16+1)*16;
	conf_content = (unsigned char *)malloc(conf_content_len);
	if(NULL == conf_content )
	{
		diag_printf("[%s][%d] errno = %d\n", __FUNCTION__, __LINE__, errno);
		perror("[keep_get_conf] malloc failed.");
		free(file_buf);
		return -1;
	}
#if 0
	else
	{
		diag_printf("[%s][%d] conf_content malloc success\n", __FUNCTION__, __LINE__);
	}
#endif
	memset(conf_content, 0x0, conf_content_len);
	ret = lm_aes_decrypt(&my_aes_ctx, conf_content, conf_content_len, file_buf, conf_file_len, &total_len);
	if(ret == -1)
	{
		printf("[%s][%d]lm_aes_decrypt failed\n", __FUNCTION__, __LINE__);
		free(file_buf);
		free(conf_content);
		return -1;
	}
	printf("[%s][%d]total_len = %d\n", __FUNCTION__, __LINE__, total_len);
	conf_content[total_len] = '\0';
	char_p = strchr(conf_content, '\r');
	if(char_p)
		*char_p = '\0';
	char_p = strchr(conf_content, '\n');
	if(char_p)
		*char_p = '\0';
	diag_printf("[%s][%d] conf_content = %s\n", __FUNCTION__, __LINE__, conf_content);
	*conf_content_save = conf_content;
	free(file_buf);

	return 0;
}

int print_conf_ack(struct lm_conf_ack *conf_ack)
{
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	printf("result : %d\n", conf_ack->result);
	printf("flag : %d\n", conf_ack->flag);
	printf("urllen : %d\n", conf_ack->urllen);
	printf("url : %s\n", conf_ack->url);
	printf("md5len : %d\n", conf_ack->md5len);
#if 0
	int i;

	for(i=0; i<conf_ack->md5len; i++)
	{
		printf("%02x ", conf_ack->md5[i]);
		if((i+1)%16 == 0)
			printf("\n");
	}
	printf("\n");
#else
	printf("md5 : %s\n", conf_ack->md5);
#endif
	diag_printf("[%s][%d]\n", __FUNCTION__, __LINE__);
	return 0;
}

int keep_conf_ack(char *conf_ack_buf)
{
	struct lm_conf_ack conf_ack;
	unsigned char *conf_content = NULL;
	int ret;

	memset(&conf_ack, 0x0, sizeof(conf_ack));

	ret = parse_conf_ack(&conf_ack, conf_ack_buf);

	if(-1 == ret)
	{
		diag_printf("[%s][%d] parse_conf_ack failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(conf_ack.result != 0)
	{
		diag_printf("[%s][%d] conf_ack result not ok\n", __FUNCTION__, __LINE__);
		free(conf_ack.url);
		free(conf_ack.md5);
		return -1;
	}

	print_conf_ack(&conf_ack);

	ret = keep_get_conf(&conf_ack, &conf_content);
	if(ret != 0)
	{
		diag_printf("[%s][%d] keep_get_conf failed\n", __FUNCTION__, __LINE__);
		free(conf_ack.url);
		free(conf_ack.md5);
		return -1;
	}

	free(conf_ack.url);
	free(conf_ack.md5);
	diag_printf("[%s][%d] <%d> conf_content = %s\n", __FUNCTION__, __LINE__, conf_ack.flag, conf_content);

	switch(conf_ack.flag)
	{
		case KEEP_CONF_DATA_RULE:
			nis_fastcheck_mode(0);
			start_data_rule(conf_content);
			keep_conf_status[KEEP_CONF_DATA_RULE-1] = 1;
			break;
		case KEEP_CONF_JS_RULE:
			js_inject_stop();
			init_js_inject_para(conf_content);
			js_inject_start();
			keep_conf_status[KEEP_CONF_JS_RULE-1] = 1;
			break;
		case KEEP_CONF_LOG_URL:
			url_record_stop();
			url_log_start(conf_content);
			keep_conf_status[KEEP_CONF_LOG_URL-1] = 1;
			break;
		default:
			break;
	}

	free(conf_content);
	return 0;
}

#if 0
void set_tcp_keepalive(int s)
{
	int keepAlive = 1; // 开启keepalive属性
	int keepIdle = 60; // 如该连接在60秒内没有任何数据往来,则进行探测 
	int keepInterval = 5; // 探测时发包的时间间隔为5 秒
	int keepCount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	setsockopt(s, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	setsockopt(s, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(s, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	return;
}
#endif

int conn_login(char *ip, int port)
{
	int s = -1;
	int ret;
	struct sockaddr_in server_addr;

	s = create_tcp_socket();
	if(-1 == s)
	{
		perror("socket error\n");
		return -1;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, (void *)(&(server_addr.sin_addr.s_addr)));
	ret = connect(s, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	if(ret < 0)
	{
		perror("Could not connect.\n");
		close(s);
		return -1;
	}

	return s;
}

int lm_read_hdr(int s, struct lm_login_keep_hdr *hdr_save)
{
	fd_set rfds;
	struct timeval tv;
	struct lm_login_keep_hdr *hdr = NULL;
	int retval, hdr_len = sizeof(struct lm_login_keep_hdr), recv_len = 0;
	char recvbuf[8] = {0};
	char *char_p = recvbuf;

	FD_ZERO(&rfds);
	FD_SET(s, &rfds);
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	retval = select(s + 1, &rfds, NULL, NULL, &tv);

	if (retval < 0)
	{
		diag_printf("[%s][%d] select failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if(retval==0 || !FD_ISSET(s, &rfds))
	{
		//diag_printf("[%s][%d] retval==0 || !FD_ISSET\n", __FUNCTION__, __LINE__);
		return -2;
	}

	memset(recvbuf,0,sizeof(recvbuf));
	retval = 0;
	while(recv_len<hdr_len)
	{
		retval = recv(s, char_p, hdr_len-recv_len, 0);
		if(retval <= 0)
		{
			diag_printf("[%s][%d] recv : retval = %d\n", __FUNCTION__, __LINE__, retval);
			return -1;
		}
		recv_len += retval;
		char_p += retval;
	}

	hdr = (struct lm_login_keep_hdr *)recvbuf;
	hdr_save->length = ntohs(hdr->length);
	hdr_save->cmd = ntohs(hdr->cmd);

	return 0;
}

int lm_read_serv(int s, char *buff, int data_len)
{
	fd_set rfds;
	struct timeval tv;
	int retval, recv_len = 0;
	int try_times = 0;
	char *char_p = buff;

	FD_ZERO(&rfds);
	FD_SET(s, &rfds);
	tv.tv_sec = 2;
	tv.tv_usec = 0;

re_select:
	retval = select(s + 1, &rfds, NULL, NULL, &tv);

	if (retval < 0)
	{
		diag_printf("[%s][%d] select failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if(retval==0 || !FD_ISSET(s, &rfds))
	{
		try_times++;
		if(try_times < 5)
			goto re_select;
		else
			return -1;
	}

	retval = 0;
	while(recv_len<data_len)
	{
		retval = recv(s, char_p, data_len-recv_len, 0);
		if(retval <= 0)
		{
			perror("recv : \n");
			return -1;
		}
		recv_len += retval;
		char_p += retval;
	}

	return 0;
}

int login_key_do(int s)
{
	char *device_id = NULL;
	int ret;

	device_id = nvram_safe_get("lm_device_id");
	if(0 == strlen(device_id) || 0 == atoi(device_id))
	{
		diag_printf("[%s][%d] need do register\n", __FUNCTION__, __LINE__);
		ret = send_register_req(s);
		if(-1 == ret)
		{
			diag_printf("[%s][%d] send_register_req failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		else
		{
			//diag_printf("[%s][%d] send_register_req success\n", __FUNCTION__, __LINE__);
			return 0;
		}
	}
	else
	{
		login_keep_id = atoi(device_id);
		login_keep_status = LM_REG_ACK;
		diag_printf("[%s][%d] do login\n", __FUNCTION__, __LINE__);
		ret = send_login_req(s);
		if(-1 == ret)
		{
			diag_printf("[%s][%d] send_login_req failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		else
		{
			//diag_printf("[%s][%d] send_login_req success\n", __FUNCTION__, __LINE__);
			return 0;
		}
	}

	return 0;
}

int login_reg_ack_do(int s, struct lm_register_ack *register_ack)
{
	int ret;
	char device_id[64] = {0};
	
	register_ack->result = ntohs(register_ack->result);
	if(register_ack->result != 0)
	{
		printf("[%s][%d] register failed\n", __FUNCTION__, __LINE__);
		diag_printf("[%s][%d] need do register\n", __FUNCTION__, __LINE__);
		ret = send_register_req(s);
		if(-1 == ret)
		{
			diag_printf("[%s][%d] send_register_req failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		else
		{
			//diag_printf("[%s][%d] send_register_req success\n", __FUNCTION__, __LINE__);
			return 0;
		}
	}

	diag_printf("[%s][%d] id = %u\n", __FUNCTION__, __LINE__, ntohl(register_ack->id));
	login_keep_id = ntohl(register_ack->id);
	sprintf(device_id, "%u", login_keep_id);
	nvram_set("lm_device_id", device_id);
	nvram_commit();

	login_keep_ip = register_ack->ip;
	login_keep_status = LM_LOGIN_ACK;
	
#if 1
	struct in_addr kkkkk_ip = {0};
	kkkkk_ip.s_addr = login_keep_ip;
	diag_printf("[%s][%d] login_keep_ip = %s\n", __FUNCTION__, __LINE__, inet_ntoa(kkkkk_ip));
#endif

#if 0
	diag_printf("[%s][%d] do login\n", __FUNCTION__, __LINE__);
	ret = send_login_req(s);
	if(-1 == ret)
	{
		diag_printf("[%s][%d] send_login_req failed\n", __FUNCTION__, __LINE__);
		return -1;
	}
	else
	{
		diag_printf("[%s][%d] send_login_req success\n", __FUNCTION__, __LINE__);
		return 0;
	}
#endif

	return 0;
}

int login_ack_do(int s, struct lm_login_ack *login_ack)
{
	int ret;
	
	login_ack->result = ntohs(login_ack->result);

	if(login_ack->result != 0)
	{
		diag_printf("[%s][%d] do login\n", __FUNCTION__, __LINE__);
		ret = send_login_req(s);
		if(-1 == ret)
		{
			diag_printf("[%s][%d] send_login_req failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		else
		{
			//diag_printf("[%s][%d] send_login_req success\n", __FUNCTION__, __LINE__);
			return 0;
		}
	}

	login_keep_ip = login_ack->ip;
	login_keep_status = LM_LOGIN_ACK;
#if 1
	struct in_addr kkkkk_ip = {0};
	kkkkk_ip.s_addr = login_keep_ip;
	diag_printf("[%s][%d] login_keep_ip = %s\n", __FUNCTION__, __LINE__, inet_ntoa(kkkkk_ip));
#endif

	return 0;
}

int conn_keep(uint32 ip, uint16 port)
{
	int s = -1;
	int ret;
	struct sockaddr_in server_addr;

	keep_sequence = 1;

	s = create_tcp_socket();
	if(-1 == s)
	{
		perror("socket error\n");
		return -1;
	}

	//set_tcp_keepalive(s);//luminais

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = ip;
	ret = connect(s, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	if(ret < 0)
	{
		perror("Could not connect.\n");
		close(s);
		return -1;
	}

	return s;
}

int keep_ack_do(int s, struct lm_keep_ack *keep_ack)
{
	int ret = 0;
	
	keep_ack->result = ntohs(keep_ack->result);
	
	if(keep_ack->result != 0)
	{
		diag_printf("[%s][%d] do send_keep_req\n", __FUNCTION__, __LINE__);
		ret = send_keep_req(s);
		if(-1 == ret)
		{
			diag_printf("[%s][%d] send_keep_req failed\n", __FUNCTION__, __LINE__);
			return -1;
		}
		else
		{
			//diag_printf("[%s][%d] send_keep_req success\n", __FUNCTION__, __LINE__);
			return 0;
		}
	}
	keep_cks = ntohl(keep_ack->check);
	keep_cks ^= LM_XOR_KEY1;

	login_keep_status = LM_KEEP_ACK;
	return 0;
}

int send_conf_reqs(int s)
{
	uint16 conf_flag = keep_conf_index+1;
	int ret = 0;
	
	if(keep_conf_status[keep_conf_index] == 0)
	{
		ret = send_conf_req(s, conf_flag);
		if(-1 == ret)
		{
			diag_printf("[%s][%d] <%d> send_conf_req failed\n", __FUNCTION__, __LINE__, keep_conf_index+1);
		}
#if 0
		else
		{
			diag_printf("[%s][%d] <%d> send_conf_req success\n", __FUNCTION__, __LINE__, keep_conf_index+1);
		}
#endif
	}

	keep_conf_index = (keep_conf_index+1)%(KEEP_CONF_FLAG_MAX-1);

	return ret;
}

void init_aes_para(void)
{
	gen_tabs();
	memset(&my_aes_ctx, 0x0, sizeof(my_aes_ctx));
	aes_set_key(&my_aes_ctx, lm_aes_key, 16, 0);

	return;
}

void lm_login_keep_main()
{
	char login_serv_ip[IP_LEN_16] = {0};
	char buff[ARRAY_LEN_256] = {0};
	int sock_fd = -1, ret;
	struct lm_login_keep_hdr hdr;
	struct lm_login_key *login_key = NULL;
	struct lm_register_ack *register_ack = NULL;
	struct lm_login_ack *login_ack = NULL;
	struct lm_login_redirect *login_redirect = NULL;
	struct lm_keep_ack *keep_ack = NULL;
	uint32 red_ip = 0;
	uint16 red_port = 0;

	init_aes_para();

	login_keep_status = LM_INIT;

	while(1)
	{
		switch(login_keep_status)
		{
			case LM_INIT:
				memset(login_serv_ip, 0x0, sizeof(login_serv_ip));
				get_login_serv_ip(login_serv_ip);
				sock_fd = conn_login(login_serv_ip, LM_LOGIN_PORT);
				if(sock_fd == -1)
				{
					diag_printf("[%s][%d] conn_login failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				else
				{
					//diag_printf("[%s][%d] conn_login success\n", __FUNCTION__, __LINE__);
					login_keep_status = LM_CONN_LOGIN;
				}
				break;
			case LM_CONN_LOGIN:
				memset(&hdr, 0x0, sizeof(hdr));
				ret = lm_read_hdr(sock_fd, &hdr);
				if(ret == 0)
				{
					memset(buff, 0x0, sizeof(buff));
					ret = lm_read_serv(sock_fd, buff, hdr.length-sizeof(struct lm_login_keep_hdr));
					if(ret == 0)
					{
						switch(hdr.cmd)
						{
							case LM_CMD_LOIN_KEY:
								login_key = (struct lm_login_key *)buff;
								login_keep_key = ntohl(login_key->key);
								login_keep_key ^= LM_XOR_KEY1;
								login_keep_key ^= LM_XOR_KEY2;
								login_keep_status = LM_KEY_OK;
								ret = login_key_do(sock_fd);
								if(ret == 0)
								{
									;
								}
								else
								{
									diag_printf("[%s][%d] login_key_do failed\n", __FUNCTION__, __LINE__);
									goto re_connect_login;
								}
								break;
							default:
								diag_printf("[%s][%d] <%d>\n", __FUNCTION__, __LINE__, hdr.cmd);
								break;
						}
					}
					else
					{
						diag_printf("[%s][%d] lm_read_serv failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				else if(ret == -1)
				{
					diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				break;
			case LM_KEY_OK:
				memset(&hdr, 0x0, sizeof(hdr));
				ret = lm_read_hdr(sock_fd, &hdr);
				if(ret == 0)
				{
					memset(buff, 0x0, sizeof(buff));
					ret = lm_read_serv(sock_fd, buff, hdr.length-sizeof(struct lm_login_keep_hdr));
					if(ret == 0)
					{
						switch(hdr.cmd)
						{
							case LM_CMD_REG_ACK:
								//diag_printf("[%s][%d] register_ack hdr.length = %d\n", __FUNCTION__, __LINE__, hdr.length);
								register_ack = (struct lm_register_ack *)buff;
								ret = login_reg_ack_do(sock_fd, register_ack);
								if(ret == 0)
								{
									;
								}
								else
								{
									diag_printf("[%s][%d] login_reg_ack_do failed\n", __FUNCTION__, __LINE__);
									goto re_connect_login;
								}
								break;
							default:
								diag_printf("[%s][%d] <%d> register_req\n", __FUNCTION__, __LINE__, hdr.cmd);
								ret = send_register_req(sock_fd);
								if(-1 == ret)
								{
									diag_printf("[%s][%d] send_register_req failed\n", __FUNCTION__, __LINE__);
									goto re_connect_login;
								}
								break;
						}
					}
					else
					{
						diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				else if(ret == -1)
				{
					diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				else
				{
					ret = send_register_req(sock_fd);
					if(-1 == ret)
					{
						diag_printf("[%s][%d] send_register_req failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				break;
			case LM_REG_ACK:
				memset(&hdr, 0x0, sizeof(hdr));
				ret = lm_read_hdr(sock_fd, &hdr);
				if(ret == 0)
				{
					memset(buff, 0x0, sizeof(buff));
					ret = lm_read_serv(sock_fd, buff, hdr.length-sizeof(struct lm_login_keep_hdr));
					if(ret == 0)
					{
						switch(hdr.cmd)
						{
							case LM_CMD_LOIN_ACK:
								//diag_printf("[%s][%d] login_ack hdr.length = %d\n", __FUNCTION__, __LINE__, hdr.length);
								login_ack = (struct lm_login_ack *)buff;
								ret = login_ack_do(sock_fd, login_ack);
								if(ret == 0)
								{
									;
								}
								else
								{
									diag_printf("[%s][%d] login_ack_do failed\n", __FUNCTION__, __LINE__);
									goto re_connect_login;
								}
								break;
							default:
								diag_printf("[%s][%d] <%d:%d> login_req\n", __FUNCTION__, __LINE__, hdr.cmd, hdr.length);
								ret = send_login_req(sock_fd);
								if(-1 == ret)
								{
									diag_printf("[%s][%d] send_register_req failed\n", __FUNCTION__, __LINE__);
									goto re_connect_login;
								}
								break;
						}
					}
					else
					{
						diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				else if(ret == -1)
				{
					diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				else
				{
					diag_printf("[%s][%d] do login\n", __FUNCTION__, __LINE__);
					ret = send_login_req(sock_fd);
					if(-1 == ret)
					{
						diag_printf("[%s][%d] send_register_req failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				break;
			case LM_LOGIN_ACK:
				memset(&hdr, 0x0, sizeof(hdr));
				ret = lm_read_hdr(sock_fd, &hdr);
				if(ret == 0)
				{
					memset(buff, 0x0, sizeof(buff));
					ret = lm_read_serv(sock_fd, buff, hdr.length-sizeof(struct lm_login_keep_hdr));
					if(ret == 0)
					{
						switch(hdr.cmd)
						{
							case LM_CMD_LOIN_RED:
								//diag_printf("[%s][%d] login_redirect hdr.length = %d\n", __FUNCTION__, __LINE__, hdr.length);
								close(sock_fd);
								sock_fd = -1;
								login_redirect = (struct lm_login_redirect *)buff;
								keep_cks = ntohl(login_redirect->checksum);
								keep_cks ^= LM_XOR_KEY1;
								red_ip = login_redirect->ip;
								red_port = ntohs(login_redirect->port);
#if 1
								struct in_addr kkkkk_ip = {0};
								kkkkk_ip.s_addr = red_ip;
								diag_printf("[%s][%d] redirect %s:%d\n", __FUNCTION__, __LINE__, inet_ntoa(kkkkk_ip), red_port);
#endif
								login_keep_status = LM_LOGIN_RED;
								break;
							default:
								diag_printf("[%s][%d] <%d>\n", __FUNCTION__, __LINE__, hdr.cmd);
								break;
						}
					}
					else
					{
						diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				else if(ret == -1)
				{
					diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				break;
			case LM_LOGIN_RED:
				sock_fd = conn_keep(red_ip, red_port);
				if(sock_fd == -1)
				{
					diag_printf("[%s][%d] conn_keep failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				else
				{
					//diag_printf("[%s][%d] conn_keep success\n", __FUNCTION__, __LINE__);
					login_keep_status = LM_CONN_KEEP;
					ret = send_keep_req(sock_fd);
					if(ret == 0)
					{
						;
					}
					else
					{
						diag_printf("[%s][%d] send_keep_req failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				break;
			case LM_CONN_KEEP:
				memset(&hdr, 0x0, sizeof(hdr));
				ret = lm_read_hdr(sock_fd, &hdr);
				if(ret == 0)
				{
					memset(buff, 0x0, sizeof(buff));
					ret = lm_read_serv(sock_fd, buff, hdr.length-sizeof(struct lm_login_keep_hdr));
					if(ret == 0)
					{
						switch(hdr.cmd)
						{
							case LM_CMD_KEEP_ACK:
								//diag_printf("[%s][%d] keep_ack hdr.length = %d\n", __FUNCTION__, __LINE__, hdr.length);
								keep_ack = (struct lm_keep_ack *)buff;
								ret = keep_ack_do(sock_fd, keep_ack);
								if(0 == ret)
								{
									;
								}
								else
								{
									diag_printf("[%s][%d] keep_ack_do failed\n", __FUNCTION__, __LINE__);
									goto re_connect_login;
								}
								break;
							default:
								diag_printf("[%s][%d] <%d> keep_req\n", __FUNCTION__, __LINE__, hdr.cmd);
								ret = send_keep_req(sock_fd);
								if(ret == -1)
								{
									diag_printf("[%s][%d] send_keep_req failed\n", __FUNCTION__, __LINE__);
									goto re_connect_login;
								}
								break;
						}
					}
					else
					{
						goto re_connect_login;
					}
				}
				else if(ret == -1)
				{
					diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				else
				{
					ret = send_keep_req(sock_fd);
					if(ret == -1)
					{
						diag_printf("[%s][%d] send_keep_req failed\n", __FUNCTION__, __LINE__);
						goto re_connect_login;
					}
				}
				break;
			case LM_KEEP_ACK:
				memset(&hdr, 0x0, sizeof(hdr));
				ret = lm_read_hdr(sock_fd, &hdr);
				if(ret == 0)
				{
					memset(buff, 0x0, sizeof(buff));
					ret = lm_read_serv(sock_fd, buff, hdr.length-sizeof(struct lm_login_keep_hdr));
					if(ret == 0)
					{
						switch(hdr.cmd)
						{
							case LM_CMD_KEEP_FIRM_VER:
								diag_printf("[%s][%d] keep_version hdr.length = %d\n", __FUNCTION__, __LINE__, hdr.length);
								ret = keep_firm_version(buff);
								if(0 == ret)
								{
									;//luminais mark
								}
								break;
							case LM_CMD_KEEP_CONF_ACK:
								//diag_printf("[%s][%d] keep_conf hdr.length = %d\n", __FUNCTION__, __LINE__, hdr.length);
								//print_packet(buff, hdr.length-sizeof(struct lm_login_keep_hdr));
								ret = keep_conf_ack(buff);
								ret = send_conf_reqs(sock_fd);
								if(ret == -1)
									goto re_connect_login;
								break;
							default:
								diag_printf("[%s][%d] <%d> conf_reqs\n", __FUNCTION__, __LINE__, hdr.cmd);
								ret = send_conf_reqs(sock_fd);
								if(ret == -1)
									goto re_connect_login;
								break;
						}
					}
					else
					{
						goto re_connect_login;
					}
				}
				else if(ret == -1)
				{
					diag_printf("[%s][%d] lm_read_hdr failed\n", __FUNCTION__, __LINE__);
					goto re_connect_login;
				}
				else
				{
					ret = send_conf_reqs(sock_fd);
					if(ret == -1)
						goto re_connect_login;
				}
				break;
			default:
				break;
		}

		continue;
re_connect_login:
		if(sock_fd != -1)
			close(sock_fd);
		login_keep_status = LM_INIT;
		cyg_thread_delay(10*100);
		continue;
	}

	diag_printf("[%s][%d] exit!!!\n", __FUNCTION__, __LINE__);
	return;
}

static cyg_handle_t lm_login_keep_handle;
static cyg_thread lm_login_keep_thread;
static char lm_login_keep_stack[1024*32];

void login_keep_start()
{
    int pid;
    
    pid = oslib_getpidbyname("lm_login_keep");
    if (pid != 0)//线程已存在，直接返回
        return;
    cyg_thread_create(
        5,
        (cyg_thread_entry_t *)lm_login_keep_main,
        0,
        "lm_login_keep",
        (void *)&lm_login_keep_stack[0],
        sizeof(lm_login_keep_stack),
        &lm_login_keep_handle,
        &lm_login_keep_thread);
    
    cyg_thread_resume(lm_login_keep_handle);
}

void login_keep_stop()
{
    int pid;
    
    pid = oslib_getpidbyname("lm_login_keep");
    
    if (pid != 0)
    {
        cyg_thread_kill(lm_login_keep_handle);
        cyg_thread_delete(lm_login_keep_handle);
    }

    return;
}


