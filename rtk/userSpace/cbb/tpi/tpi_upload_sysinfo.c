#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <network.h>
#include <string.h>
#include <netdb.h>
#include <bcmnvram.h>
#include "cJSON.h"
#include "debug.h"
#include "wan.h"
#include "version.h"
#include "common.h"
#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

#define DX_CLOUD_SERVER_NAME       "pdm.tydevice.com"
#define DX_CLOUD_SERVER_PORT       80
#define DELAY_2S                                   200

int upload_sysinfo_thread_exit = 0;
static RET_INFO tpi_upload_sysinfo_start();
static RET_INFO tpi_upload_sysinfo_restart();
static RET_INFO tpi_upload_sysinfo_stop();
/*
struct upload_sys_info
{
	char ver[3];    //"01"
	char ctei[16];  //"CT 123456  1234567    前两位固定中间六位电信提供，后七位厂商自行决定 "
	char mac[18]; //需要有:
	char ip[16];  //need .
	char uplinkmac[18];  //:
	char link;   //  1 elink  2,非elink
	char fwver[30];//固件版本号
	char date[20];//YYYY-MM-DD  HH:MM:SS
};
*/
extern int extra_get_gateway_mac(char *mac);
RET_INFO tpi_get_upload_sysinfo(cJSON*root)
{
	char tmp[32] = {0};
	char *value = NULL;
	char str_temp[128] = {0};
	P_WAN_HWADDR_INFO_STRUCT wan_hwaddr_info = NULL;
	P_WAN_INFO_STRUCT wan_common_info = NULL;
	struct tm *cur_time_tm;
	time_t  cur_time_t = time(0);


	cJSON_AddStringToObject(root, "VER", "01");

	value = nvram_get("ctei_sn");
	
	if(value == NULL)
		cJSON_AddStringToObject(root, "CTEI", "CT0000689999901");
	else
		cJSON_AddStringToObject(root, "CTEI", value);

	wan_hwaddr_info = gpi_wan_get_hwaddr_info();
	
	if(strlen(wan_hwaddr_info->wan_hwaddr) < 17)
	{
		printf("get wan mac error\n");
		return 0;
	}

	cJSON_AddStringToObject(root, "MAC", wan_hwaddr_info->wan_hwaddr);

	if(nvram_match(SYSCONFIG_WORKMODE,"bridge") 
		||nvram_match(SYSCONFIG_WORKMODE,"client+ap"))
	{
		char apclient_dhcpc_ip[17] = {0},apclient_dhcpc_mask[17] = {0};
		gpi_apclient_dhcpc_addr(apclient_dhcpc_ip,apclient_dhcpc_mask);
		cJSON_AddStringToObject(root, "IP", apclient_dhcpc_ip);
	}else
	{
		wan_common_info = gpi_wan_get_info();
		cJSON_AddStringToObject(root, "IP", wan_common_info->wan_cur_ipaddr);
	}
		
	memset(tmp,0x0,sizeof(tmp));
	extra_get_gateway_mac(tmp);
	cJSON_AddStringToObject(root, "UPLINKMAC", tmp);

	cJSON_AddStringToObject(root, "LINK", "01");
	 sprintf(tmp, "%s_%s", W311R_ECOS_SV,NORMAL_WEB_VERSION);
	cJSON_AddStringToObject(root, "FWVER", tmp);
	cur_time_tm = localtime(&cur_time_t);
	memset(str_temp, 0x0, sizeof(str_temp));
	snprintf(str_temp,sizeof(str_temp),"%d-%02d-%02d %02d:%02d:%02d" , 1900+cur_time_tm->tm_year ,cur_time_tm->tm_mon+1 ,
	cur_time_tm->tm_mday, cur_time_tm->tm_hour , cur_time_tm->tm_min , cur_time_tm->tm_sec);
	cJSON_AddStringToObject(root, "DATE", str_temp);	
}

RET_INFO tpi_upload_sysinfo_init()
{
	tpi_upload_sysinfo_start();
}
RET_INFO tpi_upload_sysinfo_action(RC_MODULES_COMMON_STRUCT *var)
{
    PI_PRINTF(TPI,"op=%d\n",var->op);
    switch(var->op)
    {
        case OP_START:
            tpi_upload_sysinfo_start();
            break;
		case OP_STOP:
            tpi_upload_sysinfo_stop();
            break;
		case OP_RESTART:
            tpi_upload_sysinfo_restart();
            break;
        default:
            PI_ERROR(TPI,"op[%d] donnot have handle!\n",var->op);
            break;
    }

    return RET_SUC;
}


int connect_to_dx_cloud_server(int *sock, char *host, unsigned short port)
{
    struct sockaddr_in addr;
    int len;
    int result;
    struct hostent *hostinfo;

    if((*sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return(-1);
    }

    hostinfo = gethostbyname(host);

    if(!hostinfo)
    {
        RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"gethostbyname error\n");
        close(*sock);
        return(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
    addr.sin_port = htons(port);
    memset(addr.sin_zero,0,sizeof(addr.sin_zero));

    len = sizeof(addr);
    if((result=connect(*sock, (struct sockaddr *)&addr, len)) == -1)
    {
        perror("connect");
        close(*sock);
        return(-1);
    }

    return 0;
}

static void output_message_to_cloud(int fd, void *buf)
{
    fd_set writefds;
    int max_fd;
    struct timeval tv;
    int ret;

    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);
    max_fd = fd;

    tv.tv_sec = 0;
    tv.tv_usec = 20000;

    ret = select(max_fd + 1, NULL, &writefds, NULL, &tv);

    if(ret == -1)
    {
        RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"select error ret:%d\n",ret);

    }
    else if(ret == 0)
    {
        RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"select timeout ret:%d\n",ret);
    }
    else
    {
        if(FD_ISSET(fd, &writefds))
        {
            if(send(fd, buf, strlen(buf), 0) == -1)
                RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"error send()ing request\n");
        }
        else
            RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"socket was not exist!!\n");
    }
}

int read_message_by_cloud(int fd, void *buf, int len)
{
    fd_set readfds;
    int max_fd;
    struct timeval tv;
    int ret;
    int bread = -1;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    max_fd = fd;

    tv.tv_sec = 9;
    tv.tv_usec = 0;
    ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);

    if(ret == -1)
    {
        RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"[cloud]: select error\n");
    }
    else if(ret == 0)
    {
        RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"select timeout\n");
    }
    else
    {
        if(FD_ISSET(fd, &readfds))
        {
            if((bread=recv(fd, buf, len, 0)) == -1)
            {
                RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"error recv()ing reply\n");
            }
        }
        else
            RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"socket was not exist!!\n");
    }

    return(bread);
}

static int parse_response_string_by_cloud(char* response)
{
	char *pos = NULL;
	cJSON *ret = NULL;
	cJSON *pItem = NULL;
	if(NULL == response)
	{
	    return 0;
	}
	pos = strstr(response, "\r\n\r\n");

	if(NULL == pos)
	{
	    return 0;
	}

	ret = cJSON_Parse(pos);

	if(ret == NULL)
		return 0;
	pItem = cJSON_GetObjectItem(ret, "Status Code");
	if(pItem == NULL)
	{
		cJSON_Delete(ret);
		return 0;
	}

	if(pItem->valueint == 200)
	{
		cJSON_Delete(ret);
		return 1;
	}

	cJSON_Delete(ret);
	return 0;
	
}

int URLEncode(const char* str, const int strSize, char* result, const int resultSize)  
{  
    int i;  
    int j = 0;//for result index  
    char ch;  
  
    if ((str==NULL) || (result==NULL) || (strSize<=0) || (resultSize<=0)) {  
        return 0;  
    }  
  
    for ( i=0; (i<strSize)&&(j<resultSize); ++i) {  
        ch = str[i];  
        if (((ch>='A') && (ch<'Z')) ||  
            ((ch>='a') && (ch<'z')) ||  
            ((ch>='0') && (ch<'9'))) {  
            result[j++] = ch;  
        } else if (ch == ' ') {  
            result[j++] = '+';  
        } else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {  
            result[j++] = ch;  
        } else {  
            if (j+3 < resultSize) {  
                sprintf(result+j, "%%%02X", (unsigned char)ch);  
                j += 3;  
            } else {  
                return 0;  
            }  
        }  
    }  
  
    result[j] = '\0';  
    return j;  
}  

int make_package(cJSON *root,char *message)
{	
	char *upload_info = NULL;
	char tmp[1024] = {0};
	int len = 0;
	char *bp = NULL;
	if(root == NULL || message == NULL)
	{
		 RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"root == NULL ||message == NULL\n");
		return -1;
	}

	bp = message;
	upload_info = cJSON_Print(root);
	URLEncode(upload_info,strlen(upload_info),tmp,1024);
	len = sprintf(message, "GET /?jsonstr=%s HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Connection: keep-alive\r\n"
					"Cache-Control: no-cache\r\n"
					"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/66.0.3359.181 Safari/537.36\r\n"
					"Accept: */*\r\n"
					"Accept-Encoding: gzip, deflate\r\n"
					"Accept-Language: zh-CN,zh;q=0.8\r\n"
					"Cookie: JSESSIONID=7AB8F9A617415AFD280E6A96380D35A9\r\n\r\n", tmp,DX_CLOUD_SERVER_NAME);
	bp += len;
	
	*bp = '\0';

	if(upload_info)
		free(upload_info);
	return 0;
}
int send_message_to_cloud(int fd)
{
	int ret = 0;
	int bytes = 0;
	cJSON *root = NULL;
	char buf[2048] = {0};
	char response[1024] = {0};

	root = cJSON_CreateObject();

	tpi_get_upload_sysinfo(root);

	if(-1 ==  make_package(root,buf))
		goto bad_message;
	/* 发送请求报文 */
	printf("===============%s [%d]\n", buf, __LINE__);
	output_message_to_cloud(fd, buf);
	/* 接收报文 */
	if((bytes=read_message_by_cloud(fd, response, 2048)) > 0)
	{
	    /* 数据解析 */
	    RC_MODULE_DEBUG(RC_UPLOAD_SYSINFO_MODULE,TPI,"%s\n",response);
	     printf("response:%s\n",response, __FUNCTION__, __LINE__);

	    ret = parse_response_string_by_cloud(response);
	}

	bad_message:
	if(root)
	  	cJSON_Delete(root);
	return ret;
}

static void upload_sysinfo_main_loop()
{
	int fd = 0;
	int try_times = 0;
	static TIME_UPDATE_RESULT time_update_tag = TIME_UPDATE_FAIL;

	while(1)
	{	
		fd = 0;
		if(upload_sysinfo_thread_exit)
			break;
		//检测系统时间是否更新
		if (TIME_UPDATE_FAIL == time_update_tag)
		{
			time_update_tag = gpi_common_time_update_result();

			if (TIME_UPDATE_FAIL == time_update_tag)
			{
				cyg_thread_delay(DELAY_2S);
	        		continue;
			}
		}
		
	        if(connect_to_dx_cloud_server(&fd,DX_CLOUD_SERVER_NAME, DX_CLOUD_SERVER_PORT) != 0)
	        {
	        	cyg_thread_delay(DELAY_2S);
	        	continue;
	        }

		if(send_message_to_cloud(fd))
		{
			close(fd);
			break;
		}
		cyg_thread_delay(DELAY_2S );
		
		if(try_times++ >  5)
		{
			close(fd);
			break;
		}
		close(fd);
	}
       		
	return 0;
}


static int upload_sysinfo_thread_running = 0;
static char upload_sysinfo_daemon_stack[1024*10];
static cyg_handle_t upload_sysinfo_daemon_handle;
static cyg_thread upload_sysinfo_daemon_thread;
static RET_INFO tpi_upload_sysinfo_start()
{
    upload_sysinfo_thread_exit = 0;
    if(upload_sysinfo_thread_running == 0)
    {
        cyg_thread_create( 8,
                           (cyg_thread_entry_t *)upload_sysinfo_main_loop,
                           0,
                           "upload_sysinfo",
                           &upload_sysinfo_daemon_stack,
                           sizeof(upload_sysinfo_daemon_stack),
                           &upload_sysinfo_daemon_handle,
                           &upload_sysinfo_daemon_thread);
        cyg_thread_resume(upload_sysinfo_daemon_handle);
        upload_sysinfo_thread_running = 1;
    }
    return RET_SUC;
}




static RET_INFO tpi_upload_sysinfo_stop()
{
	RET_INFO ret = RET_SUC;
	PI32 pid = 0;
	upload_sysinfo_thread_exit = 1;
	if(upload_sysinfo_thread_running != 0)
	{
		/* Wait until thread exit */
		pid = oslib_getpidbyname("upload_sysinfo");
		if (pid)
		{
			while(oslib_waitpid(pid, NULL) != 0)
			{
				cyg_thread_delay(10);
			}
		}
       		 upload_sysinfo_thread_running = 0;
		PI_PRINTF(TPI,"stop success!\n");		
	}
	else
	{
		PI_PRINTF(TPI,"is already stop!\n");
	}

	return ret;
}

static RET_INFO tpi_upload_sysinfo_restart()
{
	RET_INFO ret = RET_SUC;
	
	printf("upload_sysinfo restart ...\n");
	if(RET_ERR == tpi_upload_sysinfo_stop() || RET_ERR == tpi_upload_sysinfo_start())
	{
		PI_ERROR(TPI,"restart error!\n");
	}

	return ret;
}


