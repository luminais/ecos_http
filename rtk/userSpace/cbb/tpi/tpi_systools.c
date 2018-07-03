#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <netdb.h>
#include <sys/time.h>
#include "debug.h"
#include "systools.h"
#include "trxhdr.h"

#define DEBUG_INFO(fmt, ...)	do{printf(fmt,##__VA_ARGS__);/*cyg_thread_delay(100);*/}while(0)	
extern void do_reset(int );
extern void nvram_default(void);

/*本文件里面使用*/
static RET_INFO tpi_reboot_action()
{
	cyg_thread_delay(2*RC_MODULE_1S_TIME);
	do_reset(1);
	return RET_SUC;
}

static RET_INFO tpi_restore_action()
{
	cyg_thread_delay(2*RC_MODULE_1S_TIME);
	nvram_default();
	do_reset(1);
	return RET_SUC;
}

UPGRADE_SERVER_INFO dx_upgrade_info;
int save_upgrade_url(char *url,char *dir,int port)
{
	memset(dx_upgrade_info.uri,0x0,256);
	memset(dx_upgrade_info.dir,0x0,256);
	dx_upgrade_info.port= 80;
	if(url == NULL || dir == NULL )
		return 0;

	if(strlen(url) > 255 || strlen(dir) > 255)
		return 0;

	strcpy(dx_upgrade_info.uri,url);
	strcpy(dx_upgrade_info.dir,dir);
	if(port != 0)
		dx_upgrade_info.port = port;
	return 1;
}

char* get_upgrade_url()
{
	if(0 == strcmp(dx_upgrade_info.uri,""))
		return NULL;

	return dx_upgrade_info.uri;
}

char* get_upgrade_dir()
{
	if(0 == strcmp(dx_upgrade_info.dir,""))
		return NULL;

	return dx_upgrade_info.dir;
}

int get_upgrade_port()
{
	return dx_upgrade_info.port;
}

static unsigned long dns(char* host_name)
{
	struct hostent* host = NULL;
	struct in_addr addr;
	char ** pp;

	host = gethostbyname(host_name);
	if (host == NULL)
	{
		printf("gethostbyname %s failed\n", host_name);
		return 0;
	}

	pp = host->h_addr_list;
	
	if (*pp!=NULL)
	{
		addr.s_addr = *((unsigned int *)*pp);
		printf("address is %s\n",inet_ntoa(addr));
		pp++;
		return addr.s_addr;
	}

	return 1;
}

struct upgrade_mem *imageheaderPtr = NULL;		
struct upgrade_mem *curmemPtr = NULL;
int imagememLen = 0;	
int upgrade_get_mem_free(PMEM_FREE pmfree)
{
	struct mallinfo mem_info;
	cyg_mempool_info info;

	if(!pmfree){
		return -1;
	}
		
	mem_info = mallinfo();
	pmfree->heap_free = mem_info.fordblks;

	cyg_net_get_mem_stats(NET_BUF_CLUST, &info);
	pmfree->clust_free = info.freemem;

	cyg_net_get_mem_stats(NET_BUF_MEM, &info);
	pmfree->misc_free = info.freemem;

	cyg_net_get_mem_stats(NET_BUF_MBUF, &info);
	pmfree->mbuf_free = info.freemem;

	return 0;	
}
int upgrade_init()
{
	MEM_FREE mfree;
	int heap_free, clust_free, flash_free, true_size;

	upgrade_get_mem_free(&mfree);
	heap_free = mfree.heap_free - HEAP_RESOLVE_SIZE;
	heap_free = (heap_free / MCLBYTES) * MCLBYTES;	
	clust_free = mfree.clust_free - CLUST_RESOLVE_SIZE;
	
	true_size = flash_free <= heap_free? flash_free: heap_free;

	/* Initilize the image buf, filling it with heap memory first*/
	imageheaderPtr = (struct upgrade_mem *)malloc(sizeof(struct upgrade_mem));
	if(imageheaderPtr == NULL)
	{
		return -2;
	}
	imageheaderPtr->totlen = heap_free;
	imageheaderPtr->inuse = 0;
	imageheaderPtr->next = NULL;
	imageheaderPtr->data = (char *)malloc(heap_free);
	if(imageheaderPtr->data == NULL){
		return -2;
	}
	memset(imageheaderPtr->data, 0x0, heap_free);
	curmemPtr = imageheaderPtr;

	return 1;
}

int upgrade_cache(char *data, unsigned int len)
{
	struct upgrade_mem *pnode;
	static int iiii = 0;

	if(len > curmemPtr->totlen - curmemPtr->inuse)
	{
		pnode = (struct upgrade_mem *)malloc(sizeof(struct upgrade_mem));
		if(pnode == NULL)
		{
			printf("%s: malloc failed!\n", __func__);
			return -1;
		}

		pnode->totlen = MCLBYTES;
		pnode->inuse = 0;
		pnode->next = NULL;
		pnode->data = (char *)cluster_malloc();
		if(pnode->data == NULL)
		{
			printf("%s: cluster malloc failed!\n", __func__);
			free(pnode);
			return -1;
		}

		curmemPtr->next = pnode;
		curmemPtr = pnode;
	}
if(iiii == 0){
	diag_printf("curmemPtr->inuse:%d\r\n",curmemPtr->inuse);
}
	memcpy(&curmemPtr->data[curmemPtr->inuse], data, len);

	if(iiii == 0){
	diag_printf("[%c][%c][%c][%c][%c]         curmemPtr->inuse:%d\r\n",
	curmemPtr->data[0],curmemPtr->data[1],curmemPtr->data[2],curmemPtr->data[3],curmemPtr->data[4],curmemPtr->inuse);
	diag_printf("[%c][%c][%c][%c][%c]\r\n",
	imageheaderPtr->data[0],imageheaderPtr->data[1],imageheaderPtr->data[2],imageheaderPtr->data[3],imageheaderPtr->data[4]);
		iiii = 1;
	}
	curmemPtr->inuse += len;
	
	imagememLen += len;
	
	printf(".");
	return len;
}
#define BOUNDRY_SIZE_BUF   64
#define DASH                '-'
#define LINE_END0	0x0d
#define LINE_END1	0x0a

void elink_upgrade()
{
	int offset,flash_offset;
	char ret_buf[32] = {0};
	int ret = UPGRADE_SUCCESSFUL; 

	diag_printf("Start elink_upgrade...\r\n");
	tapf_watchdog_disable();		
	if(do_upgrade_check(imageheaderPtr,0,&flash_offset)<0){
		
		ret = UPGRADE_FILE_FAIL;
		goto upgrade_reboot;
	}
upgrade_reboot:

#ifdef REALTEK
	/*realtek 升级跳过头, llm add*/
	offset += sizeof(struct trx_header);
#endif

	if(ret==UPGRADE_SUCCESSFUL)
	{
		if(tenda_upload_fw(imageheaderPtr,offset,flash_offset) < 0)
		{	
			ret = UPGRADE_FALI;
		}
		else 
		{	
			diag_printf("Upgrade success...");
		}
	}

	sys_reboot();
	return;
}

static RET_INFO tpi_autoupgrade()
{
	int port = 0;
	int ret = 0;
	int len = 0;
	char *url = NULL;
	char *dir = NULL;
	struct in_addr in;
	struct sockaddr_in ser;
	int sock = -1;
	char buf[1024] = {0};
	struct timeval tm;
	fd_set readfds;
	int content_length = 0;
	int total_len = 0;
	int wlen = 0;
	int err = 0;
	int done = 0;
	char *p = NULL;

	url = get_upgrade_url();
	if(url == NULL)
		return RET_ERR;

	dir = get_upgrade_dir();

	if(dir == NULL)
		return RET_ERR;

	port = get_upgrade_port();
	
	in.s_addr = dns(url);
	if(in.s_addr == 0)
	{
		printf("resolve_dns error\n");
		goto fail;
	}

	memset(&ser, 0x0, sizeof(ser));
	ser.sin_addr = in;
	ser.sin_port = htons(port);
	ser.sin_family = AF_INET;


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		printf("socket error\n");
		goto fail;
	}

	ret = connect(sock, (struct sockaddr *)&ser, sizeof(ser));
	if(ret < 0)
	{
		
		printf("connect %s error\n", url);
		goto fail;
	}

	len = snprintf(buf, sizeof(buf), 
		"GET %s "
		"HTTP/1.1\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/5.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
		"Host: %s:%d\r\n"
		"Connection: Close\r\n\r\n", dir, url,port);

	ret = send(sock , buf, len, 0);
	if(ret != len)
	{
		printf("send error: ret=%d, len=%d\n", ret, len);
		close(sock);
		goto fail;
	}

	/* Parse http header first */
	memset(buf, 0x0, sizeof(buf));
	len = recv(sock, buf, sizeof(buf), 0);
	//printf("[%s][%d]buf:%s len:%d\r\n", __FUNCTION__, __LINE__,buf,len);
	if(len <= 0)
	{
		printf("recv error : ret= %d\n", ret);
		goto fail;
	}

	upgrade_init();
	if(p = strstr(buf, "\r\n\r\n"))
	{
		p+= 	strlen("\r\n\r\n");
		wlen = len - (p - buf);
		if(wlen > 0)
		{
			diag_printf("[%c][%c][%c][%c][%c]\r\n",
				p[0],p[1],p[2],p[3],p[4]);
			upgrade_cache(p, wlen);
			total_len += wlen;
		}
		
	}
	tm.tv_sec = 10*60;//10分钟没下载成功直接go to fail
	tm.tv_usec = 0;
	int iiii = 0;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		ret = select(sock + 1,  &readfds, NULL, NULL, &tm);
		switch(ret)
		{
			case -1:
				printf("select error\n");
				if(EINTR == errno)
				{
					continue;
				}
				err=1;
				break;
			case 0:
				printf("select time out.\n");
				err = 1;
				break;
			default:
				if(!FD_ISSET(sock, &readfds))
				{
					printf("?? what happen ?\n");
					continue;
				}
				
				len = recv(sock , buf, sizeof(buf), 0);
				if(len <= 0)
				{
					done = 1;
					break;
				}
				
				ret = upgrade_cache(buf, len);
				total_len += len;				
				break;
		}

		if(err)
		{
			printf("Error happen.\n");
			goto fail;
		}

		if(done)
		{
			//printf("Done-------total_len=%d--.\n", total_len);
			break;
		}
	}
	diag_printf("[%c][%c][%c][%c][%c]\r\n",
				imageheaderPtr->data[0],imageheaderPtr->data[1],imageheaderPtr->data[2],imageheaderPtr->data[3],imageheaderPtr->data[4]);

	diag_printf("total_len=%d\r\n",total_len);
	close(sock);
	elink_upgrade();
	//下载完成
	
	return 0;

fail:
	if(sock > 0)
		close(sock);
	printf("Failed ....\n");
	return -1;
	}

RET_INFO tpi_systools_action(RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;

	//根据coverity分析结果修改，原来为无效的判断:NULL == var->string_info  2017/1/11 F9项目修改
	if(NULL == var || 0 == strcmp("",var->string_info))
	{
		return RET_ERR;
	}
	
	if(0 == memcmp("reboot",var->string_info,strlen("reboot")))
	{
		ret = tpi_reboot_action();
	}
	else if(0 == memcmp("restore",var->string_info,strlen("restore")))
	{
		ret = tpi_restore_action();
	}
	else if(0 == memcmp("commit",var->string_info,strlen("commit")))
	{
	/*添加commit操作，对于一些需要及时返回的操作，可以先发commit消息,
	然后立即返回，前提条件，发送消息的线程优先级需要大于等于main线程的优先级*/
		nvram_commit();
	}
	else if(0 == memcmp("autoupgrade",var->string_info,strlen("autoupgrade")))
	{
	/*添加commit操作，对于一些需要及时返回的操作，可以先发commit消息,
	然后立即返回，前提条件，发送消息的线程优先级需要大于等于main线程的优先级*/
		DEBUG_INFO("===============%s [%d]\n", __FUNCTION__, __LINE__);
		 tpi_autoupgrade();
	}

	return ret;
}

