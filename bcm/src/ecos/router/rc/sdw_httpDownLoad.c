#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <bcmnvram.h>
#include <arpa/inet.h>
#include <shutils.h>
#include <rc.h>
#include <ecos_oslib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//#include <netdb.h>

#include "sdw_httpDownLoad.h"

extern int strpos(const char *src, char c, int flag);

/*是邦信重定向页面元素的返回1，其他返回0*/
int check_bos_index_element(char *url)
{
	if (NULL == url)
		return 0;
	
	int i;
	for (i=0; i<cur_bos_index_element_num; i++)
	{
		if (strlen(bos_index_element[i]) > 0 && 
			strstr(url,bos_index_element[i]) != NULL)
			return 1;
	}

	return 0;
}

/*是邦信重定向页面元素的返回1，其他返回0*/
int check_bos_index_path(char *url)
{
	if (NULL == url)
		return 0;
	
	int i;
	for (i=0; i<cur_bos_index_path_num; i++)
	{
		if (strlen(bos_index_path[i]) > 0 && 
			strstr(url,bos_index_path[i]) != NULL)
			return 1;
	}

	return 0;
}

int analysis_dir_path(char *full_pathname)
{
	int pos,len;	
	
	char dir_path[ARRAY_LEN_256] = {0};
	char *p_path = NULL;
 	
	if (!full_pathname)  
	  	return -1;  
	
	if (strlen(full_pathname) > ARRAY_LEN_256)
	{
		printf("full_pathname len > 255,return!\n");
		return -1;
	}

	p_path = full_pathname;

	if(p_path == NULL)
	{
		return -1 ;
	}
	
	len = pos = 0;  
	
	while ((pos = strpos(p_path, '/', 0)) != -1)
	{
		len += pos;
		
		strncpy(dir_path,full_pathname,len);
		
		dir_path[len] = '\0';
	
		if (access(dir_path,0) != 0)	//目录不存在，需创建
		{
			//printf("mkdir %s!\n",dir_path);
			if (mkdir(dir_path, 0777) != 0)//创建失败
			{
				printf("mkdir error,return!\n");
				return -1;
			}

			if (cur_bos_index_path_num < BOS_INDEX_ELEMENT_NUM)
		   	{
		   		strncpy(bos_index_path[cur_bos_index_path_num],dir_path,BOS_INDEX_ELEMENT_LEN);
				cur_bos_index_path_num++;
		   	}
		}

		p_path = full_pathname+len;

	}
	
	return 0;
}


int download_redirect_url(void)
{
	int load_url_sucess = 0;
	
	char *tmp_value = NULL;
	
	char lan_ip[IP_LEN_16] = {0};

	char new_redirect_url[MAX_FULL_URL_LEN] = {0};

	int i = 0 ;

	/*bos_index_path[0]固定存放bonson专有的文件夹，由DIR_FOR_DD指定*/
	strncpy(bos_index_path[0],DIR_FOR_DD,BOS_INDEX_ELEMENT_LEN);

	tmp_value = nvram_safe_get("lan_ipaddr");
	if (NULL == tmp_value)
	{
		printf("get lan_ipaddr fail,thread exit!!!\n");

		return 0;
	}
	strcpy(lan_ip, tmp_value);

	printf("begin download_redirect_url thread...\n");

	cyg_thread_delay(100);

	int  pid = oslib_getpidbyname("DNS daemon");

	if(pid == 0)
	{
		cyg_thread_delay(300);
		pid = oslib_getpidbyname("DNS daemon");
		if(pid == 0)
		{
			return 1 ;
		}
	}
	if( nis_init_server_ip() != 0)
	{
		return 1 ;
	}
	
	/*for循环下载页面，重复LOAD_URL_RETRY_TIME次*/
	for (i=0; i<LOAD_URL_RETRY_TIME; i++)
	{
		if (Bos_index_Sev() > 0)
		{
			load_url_sucess = 1;
			
			break;
		}
		
		cyg_thread_delay(30*100);
	}

	/*页面下载成功，往url_recored模块设置http_redirect_url为本地链接*/
	if (1 == load_url_sucess)
	{
		memset(new_redirect_url , 0 , sizeof(new_redirect_url));
		sprintf(new_redirect_url,"http://%s%s%s",lan_ip,DIR_FOR_DD,BOS_INDEX);
		set_new_http_redirect_url(new_redirect_url);
		read_url_from_file() ;	
		have_success_download = 1 ;
		printf("have_success_download[%d]\n" , have_success_download);
	}
	
	return 0;
}

int download_redirect_url_thread(void)
{	
	int pid;
	
	pid = oslib_getpidbyname("download_redirect_url");
	if (pid != 0)//download_redirect_url线程已存在，直接返回
		return 0;
	cyg_thread_create(
		10, 
		(cyg_thread_entry_t *)download_redirect_url,
		0, 
		"download_redirect_url",
		load_url_thread_stack, 
		sizeof(load_url_thread_stack), 
		&load_url_thread_handle, 
		&load_url_thread);
		
	cyg_thread_resume(load_url_thread_handle);
	return 0;
}


/*返回下载的元素个数。根据下载成功元素个数判断是否成功，<=0表示失败*/
int Bos_index_Sev(void)
{

	int i;

	int error_flag = -1 ;
	
	char pathResources[ARRAY_LEN_128] = {0};

	memset(bos_index_element,0,BOS_INDEX_ELEMENT_NUM*BOS_INDEX_ELEMENT_LEN*sizeof(char));

	cur_bos_index_element_num = 0;


	//下载url表
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,SDW_URL);
	error_flag = http_download(DOWNLOAD_URL_PATH , sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad bos_url.xml fail\n");
		return -1 ;
	}	
	
	/*下载html*/
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,BOS_INDEX);
	error_flag = http_download(DOWNLOAD_INDEX_PATH , sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad bos_index_html fail\n");
		return -1 ;
	}

	/*下载agree_tit.png*/
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,SDW_AGREE_ELE);
	error_flag = http_download(DOWNLOAD_AGREE_ELE  ,sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad agree_tit.png fail\n");
		return -1 ;
	}
	
	/*下载agree_true.png*/
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,SDW_TRUE_ELE);
	error_flag = http_download(DOWNLOAD_TRUE_ELE , sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad agree_true.png fail\n");
		return -1 ;
	}

	/*下载header.png*/
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,SDW_HEAD_PATH);
	error_flag = http_download(DOWNLOAD_HEAD_PNG , sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad agree_true.png fail\n");
		return -1 ;
	}

	/*下载img_item01.png*/
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,SDW_ITEM01_PATH);
	error_flag = http_download(DOWNLOAD_ITEM01_PNG , sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad agree_true.png fail\n");
		return -1 ;
	}

	/*下载img_item02.png*/
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,SDW_ITEM02_PATH);
	error_flag = http_download(DOWNLOAD_ITEM02_PNG , sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad agree_true.png fail\n");
		return -1 ;
	}

	/*下载img_item03.png*/
	memset(pathResources, 0, sizeof(pathResources));
	sprintf(pathResources,"%s%s",DIR_FOR_DD,SDW_ITEM03_PATH);
	error_flag = http_download(DOWNLOAD_ITEM03_PNG , sdw_dwn_index_ip,pathResources);
	if(-1 == error_flag)
	{
		perror("DownLoad agree_true.png fail\n");
		return -1 ;
	}

	
	printf("Bos_index_Sev sucess,cur_bos_index_element_num is[%d] cur_bos_index_path_num[%d]!!!\n",cur_bos_index_element_num,cur_bos_index_path_num);
	
	for (i=0; i<cur_bos_index_element_num; i++)
	{
		printf("bos_index_element[%d]=[%s]\n",i,bos_index_element[i]);
	}

	for (i=0; i<cur_bos_index_path_num; i++)
   	{
		printf("bos_index_path[%d]=[%s]\n",i,bos_index_path[i]);
   	}

	return cur_bos_index_element_num;
	
}

int http_download(char *pURL,char *paddr,char *pSave_fileName)
{
	struct sockaddr_in dest_addr;
	char save_path[ARRAY_LEN_128] ={0};
	char path[ARRAY_LEN_1024] = {0};
	char strRequest[ARRAY_LEN_1024] = {0};
	char strResponse[ARRAY_LEN_1024] = {0};
	int i = 0;
	int  j = 0;
	int nRequestLen = 0;
	int sockfd = 0;
	char head_buf[ARRAY_LEN_1024] = {0};
	//char *head_index = NULL;	
	FILE* fp = NULL ;
	 /*构造服务器请求报文*/	
	sprintf(path, "GET %s HTTP/1.1\r\n"
		"Accept: text/html, application/xhtml+xml, */*\r\n"
		"Accept-Language: zh-CN\r\n"
		"User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Host: %s\r\n"
		"Connection: Keep-Alive\r\n\r\n",pURL,paddr);

	sprintf(save_path,"%s",pSave_fileName);

	if (analysis_dir_path(save_path) != 0)
	{
		printf("TestBos:analysis_dir_path fail\r\n");

		return -1;
	}

	//printf("TestBos:save_path =%s \r\n",save_path);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);			
	if (-1 == sockfd)
	{
		printf("TestBos:creat sockfd is fail\r\n");
		perror("sockfd");
		return -1;
	}

	/* 填写sockaddr_in结构*/
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(80);
	dest_addr.sin_addr.s_addr = inet_addr(paddr);

	/* 客户程序发起连接请求 */
	if (-1 == connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)))
	{
		printf("TestBos:sockfd connect is fail\r\n");
		perror("connect");
		goto  free_resource;
	}
	
	/*发送http请求request*/
	nRequestLen = strlen(path);
	memset(strRequest, 0, sizeof(strRequest));
	strncpy(strRequest, path, nRequestLen);
	if (-1 == write(sockfd, strRequest, nRequestLen))
	{
		printf("TestBos:sockfd write is fail\r\n");
		perror("write");
		goto  free_resource;
	} 

	
	/* 连接成功了，接收http响应，response */
	memset(strResponse, 0, sizeof(strResponse));
	//printf("TestBos:strResponse:\r\n");

	while (1 == (nRequestLen = read(sockfd, strResponse, 1)))
	{
		//printf("%s",strResponse);
		/*连续找到4个连续的\r或者\n，表示头部结束*/
		if (i < 4)
		{
			head_buf[j++] = strResponse[0];
			if (strstr(head_buf, "404 "))
			{
				   printf("TestBos:serv send is 404 Not Found\r\n");
				   goto  free_resource;
			}
			
			if (strResponse[0] == '\r' || strResponse[0] == '\n')
			{
				i++;

				if (i >= 4)
					break;
			}
			else
			{
				i = 0;
			}
		}
		
	}

	memset(strResponse, 0, sizeof(strResponse));

	printf("TestBos:download file save_path = %s\r\n",save_path);
	
	fp = fopen(save_path, "wb+");
	if(fp== NULL)
	{
		printf("TestBos:2.fopen fs is fail\r\n");
		goto  free_resource;
	}
	while (1)
	{
		memset(strResponse, 0, sizeof(strResponse));
		i = read(sockfd, strResponse, 1024-1);
		if (i <= 0)
		{	
		    break;
		}

		/*将http回应报文写入文件*/		
		fwrite(strResponse,i,1,fp);
	}

	fclose(fp) ;
	close(sockfd) ;
   	if (cur_bos_index_element_num < BOS_INDEX_ELEMENT_NUM)
   	{
   		strncpy(bos_index_element[cur_bos_index_element_num],pSave_fileName,BOS_INDEX_ELEMENT_LEN);
		cur_bos_index_element_num++;
   	}
	return 0 ;
free_resource :

	if(fp != NULL)
	{
		fclose(fp);
	}

	if(sockfd != -1)
	{
		close(sockfd) ;
	}
	return -1;
}

