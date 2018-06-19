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
#include <string.h>
#include "../../bsp/net/ipfilter/sdw_filter.h"
#include "flash_cgi.h"
#include "sdw_conf.h"

char *a_query;

extern int is_wan_up;
extern void init_router_mac(char *router_mac_char);

struct file_content sdw_conf_content[] = 
{
	{"bd-sw", ""},
	{"bd-ul", ""},
	{"pc-sw", ""},
	{"pc-dl", ""},
	{"pc-ve", ""},
	{"pad-sw", ""},
	{"pad-dl", ""},
	{"pad-ve", ""},
	{"phone-sw", ""},
	{"phone-dl", ""},
	{"phone-ve", ""},
	/*{"jsfail-sw", ""},//luminais rm jsfail_submit
	{"jsfail-ul", ""},*/
	{"js-enable",""},
	{"js-str",""},
	{"upgrade-sw", ""},
	{"upgrade-dl", ""},
	{"upgrade-ve", ""},
};

int sdw_conf_done = CONF_ANALYZING;
char router_mac_conf[MAC_LEN_20] = {0};

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
		diag_printf("[%s][%d]save_path = %s\n", __FUNCTION__, __LINE__, save_path);
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
		unlink(save_into_router);
	}

	if(sockfd  != -1)
	{
		close(sockfd) ;
	}
	return ret;
}

int content_name_value(char *content, char *content_name, char *content_value)
{
	char *p = NULL, *q = NULL;
	if(NULL==content || NULL==content_name || NULL==content_value)
	{
		diag_printf("[%s]invalid parameter\n", __FUNCTION__);
		return -1;
	}
	p = strchr(content, '=');
	if(NULL == p)
	{
		diag_printf("[%s]illegal content\n", __FUNCTION__);
		return -1;
	}
	*p = '\0';
	p++;
	strncpy(content_name, content, strlen(content));
	q = strchr(p, '\n');
	if(NULL != q)
		*q = '\0';
	q = NULL;
	q = strchr(p, '\r');
	if(NULL != q)
		*q = '\0';
	strncpy(content_value, p, strlen(p));
	//diag_printf("[%s]content_name = %s\n", __FUNCTION__, content_name);
	//diag_printf("[%s]content_value = %s\n", __FUNCTION__, content_value);
	return 0;
}

int save_conf_from_file()
{
	char content[256] = {0};
	char content_name[16] = {0};
	char content_value[128] = {0};
	FILE *fp = NULL;
	int content_index = BEHAV_DATA_SW;
	
	fp = fopen(SDW_CONF_FILE_SAVE, "r");
	if(NULL == fp)
	{
		diag_printf("[%s]no such file : %s\n", __FUNCTION__, SDW_CONF_FILE_SAVE);
		return -1;
	}

	while(fgets(content, 256, fp))
	{
		//diag_printf("%s\n", content);
		if(0 != content_name_value(content, content_name, content_value))
			goto read_again;
		//diag_printf("content_name = %s, content_value = %s\n", content_name, content_value);
		for(content_index=BEHAV_DATA_SW; content_index<=UPGRADE_VE; content_index++)
		{
			if(0 == strcmp(sdw_conf_content[content_index].content_name, content_name))
				strncpy(sdw_conf_content[content_index].content, content_value, strlen(content_value));
		}
read_again:
		memset(content, 0, sizeof(content));
		memset(content_name, 0, sizeof(content_name));
		memset(content_value, 0, sizeof(content_value));
	}
	fclose(fp);
		
	return 0;
}

void save_conf_by_defaults()
{
	int content_index = BEHAV_DATA_SW;

	for(content_index=BEHAV_DATA_SW; content_index<=UPGRADE_VE; content_index++)
	{
		switch(content_index)
		{
			case BEHAV_DATA_SW:
				strcpy(sdw_conf_content[BEHAV_DATA_SW].content, "0");
				break;
			case BEHAV_DATA_UL:
				strcpy(sdw_conf_content[BEHAV_DATA_UL].content, "");
				break;
			case PC_SW:
				strcpy(sdw_conf_content[PC_SW].content, "1");
				break;
			case PC_DL:
				strcpy(sdw_conf_content[PC_DL].content, "");
				break;
			case PC_VE:
				strcpy(sdw_conf_content[PC_VE].content, "0");
				break;
			case PAD_SW:
				strcpy(sdw_conf_content[PAD_SW].content, "0");
				break;
			case PAD_DL:
				strcpy(sdw_conf_content[PAD_DL].content, "");
				break;
			case PAD_VE:
				strcpy(sdw_conf_content[PAD_VE].content, "0");
				break;
			case PHONE_SW:
				strcpy(sdw_conf_content[PHONE_SW].content, "0");
				break;
			case PHONE_DL:
				strcpy(sdw_conf_content[PHONE_DL].content, "");
				break;
			case PHONE_VE:
				strcpy(sdw_conf_content[PHONE_VE].content, "0");
				break;
			/*case JSFAIL_SW://luminais rm jsfail_submit
				strcpy(sdw_conf_content[JSFAIL_SW].content, "0");
				break;
			case JSFAIL_UL:
				strcpy(sdw_conf_content[JSFAIL_UL].content, "");
				break;*/
			case JS_ENABLE:
				strcpy(sdw_conf_content[JS_ENABLE].content, "1");
				break;
			case JS_STR:
				strcpy(sdw_conf_content[JS_STR].content, "");
				break;
			case UPGRADE_SW:
				strcpy(sdw_conf_content[UPGRADE_SW].content, "0");
				break;
			case UPGRADE_DL:
				strcpy(sdw_conf_content[UPGRADE_DL].content, "");
				break;
			case UPGRADE_VE:
				strcpy(sdw_conf_content[UPGRADE_VE].content, "0");
				break;
			default:break;
		}
	}
	return;
}

int get_conf_serv_ip(char *ip)
{
	int get_ip_times = 0;
	for(get_ip_times=0; get_ip_times<MAX_TRY_TIMES; get_ip_times++)
	{			
		if(0 != get_ip(CONF_SERVER_URL, ip) )
		{
			memset(ip , 0 , sizeof(ip));
			if(0 == get_ip_times)
			{
				save_conf_by_defaults();
				sdw_conf_done = CONF_SUCCESS;
			}
			cyg_thread_delay(INTERVAL_TIME*100); //30s
		}
		else
			break;
	}

	if(get_ip_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get conf serv ip failed, sleep 1 hour\n", __FUNCTION__);
		return -1;
	}
	else
	{
		ip[IP_LEN_16-1] = '\0';
		diag_printf("[%s]get conf serv ip success\n", __FUNCTION__);
		return 0;
	}
}

int get_conf_from_serv(char *host_url, char *server_path, int port, char *server_ip, char *save_into_router)
{
	int get_conf_times = 0;
	int conf_file_len = 0;
	for(get_conf_times=0; get_conf_times<MAX_TRY_TIMES; get_conf_times++)
	{
		if(OK != http_get_file(host_url, server_path, port, server_ip, save_into_router, &conf_file_len))
		{
			if(0 == get_conf_times)
			{
				save_conf_by_defaults();
				sdw_conf_done = CONF_SUCCESS;
			}
			cyg_thread_delay(INTERVAL_TIME*100); //30s
		}
		else
			break;
	}

	if(get_conf_times == MAX_TRY_TIMES)
	{
		diag_printf("[%s]get conf file failed, sleep 10 mins\n", __FUNCTION__);
		return -1;
	}
	else
	{
		if(0 == conf_file_len)
		{
			save_conf_by_defaults();
			sdw_conf_done = CONF_SUCCESS;
			diag_printf("[%s]conf_file_len = 0, sleep 1 hour\n", __FUNCTION__);
			return -1;
		}
		else
		{
			diag_printf("[%s]get conf file success, get_conf_times = %d\n", __FUNCTION__, get_conf_times);
			return 0;
		}
	}
}

void sdw_print_conf_content()
{
	int content_index = BEHAV_DATA_SW;
	diag_printf("------------print start-------------\n");
	for(content_index=BEHAV_DATA_SW; content_index<=UPGRADE_VE; content_index++)
		diag_printf("%s = %s\n", sdw_conf_content[content_index].content_name, sdw_conf_content[content_index].content);
	diag_printf("------------print done-------------\n");
	return;
}

void sdw_conf_main()
{
	char sdw_conf_serv_ip[IP_LEN_16] = {0};
	char conf_serv_url[ARRAY_LEN_256] = {0};
	int is_wan_up_old = -1;
	init_router_mac(router_mac_conf);
	while (1){
#if 1
		if(0 == is_wan_up)
		{
			if(-1 == is_wan_up_old)
			{
				is_wan_up_old = 0;
				save_conf_by_defaults();
				sdw_conf_done = CONF_SUCCESS;
			}
			//diag_printf("[%s]0 == is_wan_up, sleep %d sec . . . \n", __FUNCTION__, INTERVAL_TIME);
			cyg_thread_delay(INTERVAL_TIME*100); // 30s
			continue;
		}
		else
		{
			is_wan_up_old = -1;
		}
#endif
#if 0
		strcpy(sdw_conf_serv_ip, "192.168.0.100");
#else
		if(0 != get_conf_serv_ip(sdw_conf_serv_ip))
		{
			memset(sdw_conf_serv_ip , 0 , sizeof(sdw_conf_serv_ip));
			cyg_thread_delay(CONF_SERV_IP_FAIL_SLEEP*100); //1 hour -> 10 mins
			continue;
		}
#endif
		sprintf(conf_serv_url,CONF_FILE_SERVER_PATH,router_mac_conf);
		if(0 != get_conf_from_serv(CONF_SERVER_URL, conf_serv_url, SERVER_PORT, sdw_conf_serv_ip, SDW_CONF_FILE_SAVE))
		{
			cyg_thread_delay(CONF_SERV_IP_FAIL_SLEEP*100); //1 hour -> 10 mins
			continue;
		}
		if(0 != save_conf_from_file())
		{
			unlink(SDW_CONF_FILE_SAVE);
			continue;
		}
		unlink(SDW_CONF_FILE_SAVE);
		sdw_print_conf_content();
		diag_printf("[%s]config file process done\n", __FUNCTION__);
		sdw_conf_done = CONF_SUCCESS;
		cyg_thread_delay(CONF_RENEW_SLEEP*100);//24 hours -> 1 hour
	}
	diag_printf("%s exit!!!\n", __FUNCTION__);
	return;
}

static cyg_handle_t sdw_conf_handle;
static cyg_thread sdw_conf_thread;
static char sdw_conf_stack[1024*16];

void sdw_conf_start()
{
	int pid;
	
	pid = oslib_getpidbyname("sdw_conf");
	if (pid != 0)//线程已存在，直接返回
		return;
	cyg_thread_create(
		5,
		(cyg_thread_entry_t *)sdw_conf_main,
		0,
		"sdw_conf",
		(void *)&sdw_conf_stack[0],
		sizeof(sdw_conf_stack),
		&sdw_conf_handle,
		&sdw_conf_thread);
	
	cyg_thread_resume(sdw_conf_handle);
}

void sdw_conf_stop()
{
	int pid;
	
	pid = oslib_getpidbyname("sdw_conf");
	
	if (pid != 0)
	{
		cyg_thread_kill(sdw_conf_handle);
		sdw_conf_done = CONF_ANALYZING;
		cyg_thread_delete(sdw_conf_handle);
	}

	return;
}

