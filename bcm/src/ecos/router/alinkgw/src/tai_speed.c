#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <bcmnvram.h>
#include "flash_cgi.h"
#include "route_cfg.h"
#include "alinkgw_api.h"
#include "tai.h"
#include "cJSON.h"
#include  "tc.h"

#define ALILINK_SPEEDTEST_LIST(i)			racat("alilink_speedtest_list", i)
#define MILLION	(1000*1000)

 struct wan_speed_detect{
	 unsigned int enable;
	 unsigned int duration;
	 unsigned long uspeed;
	 unsigned long dspeed;
 };
struct wan_speed_detect gWanSpeedDetect;

typedef enum detect_state{
	DETECT_STOP,
	DETECT_START,
	DETECTING,
	DETECTEND
}DETECT_STATE;

DETECT_STATE g_detect_state = DETECT_STOP;

#define MAX_DETECT_NUM	10

struct detect_ctx {
	int sockfd;
	struct sockaddr_in dest_addr;
};
static struct detect_ctx detect_ctx[MAX_DETECT_NUM];

struct ip_list {
	struct	in_addr sin_addr;
};
static struct ip_list g_ip_list[MAX_DETECT_NUM];

extern char speed_detect_ifname[16] ;
extern unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];
extern unsigned char *lookmac(in_addr_t ip);
extern in_addr_t inet_addr(const char *cp);
extern int ismulticast(unsigned char *mac);
extern int ali_get_connection_status();
extern char *inet_ntoa_tenda(in_addr_t ina);
extern int if_TX_RX_status(char *ifname,u_long * tx,u_long * rx);

int report_thread_running = 0;
float g_wan_rate_float  = 0 ;
unsigned long g_wan_rate = 0 ;
int detect_thread_running = 0;
unsigned int g_ip_count = 0;

//speed report
static cyg_handle_t wan_speed_report_handle;
static char wan_speed_report_stack[1024*4];
static cyg_thread wan_speed_report_thread;
//speed detect
static cyg_handle_t wan_speed_detect_handle;
static char wan_speed_detect_stack[1024*16];
static cyg_thread wan_speed_detect_thread;

//获取实时速率单位字节每秒
void get_stream_statistic_speed(u_long * up_speed,u_long * down_speed)
{
	char pre_ip[32]={'\0'}, pr_ip[32]={'\0'};
	char *lanip, *p;
	unsigned int index;
	unsigned char *mac_look_list;
	
	_GET_VALUE(_LAN0_IP, lanip);
	strcpy(pr_ip, lanip);
	p=rindex(pr_ip, '.');
	if(p) *p='\0';

	u_long   up_byte_pers = 0;
	u_long   down_byte_pers = 0;

	for(index=0; index< STREAM_CLIENT_NUMBER; ++index)
	{		
		sprintf(pre_ip, "%s.%d", pr_ip, index+1);
		mac_look_list = lookmac(inet_addr(pre_ip));
		if( NULL == mac_look_list)
			continue;

		if(ismulticast(mac_look_list))
			continue;

		up_byte_pers += ip_ubs(index);
		down_byte_pers += ip_dbs(index);
	}
	(*up_speed) = up_byte_pers;
	(*down_speed) = down_byte_pers;
	return;
}

int get_wandl_speed(char *buf, unsigned int buff_len)
{
	int len = 0;
	char str[32] = {0};
	u_long up_speed = 0;
	u_long down_speed =0;
	
	if(NULL == buf)
		return ALINKGW_ERR;

	get_stream_statistic_speed(&up_speed,&down_speed);
	len = sprintf(str,"%ld",down_speed);

	if(len >= buff_len)
		return ALINKGW_BUFFER_INSUFFICENT;
	memcpy(buf, str, len+1);
	printf("%s buf:%s\n",__func__,buf);

	return ALINKGW_OK;
}

int get_wanul_speed(char *buf, unsigned int buff_len)
{
	int len = 0;
	char str[32] = {0};
	u_long up_speed = 0;
	u_long down_speed =0;
	
	if(NULL == buf)
		return ALINKGW_ERR;
	get_stream_statistic_speed(&up_speed,&down_speed);
	len = sprintf(str,"%ld",  up_speed);

	if(len >= buff_len)
		return ALINKGW_BUFFER_INSUFFICENT;
	memcpy(buf, str, len+1);
	printf("%s buf:%s\n",__func__,buf);
	return ALINKGW_OK;
}

/*wan 口速率每 5 秒计算一次，每逢整点及半点主动上报一次*/
void wan_speed_report()
{
	char buf[64] = {0};
	static int need_report = 0;
	time_t now;
	struct tm tm;
	u_long up_speed = 0;
	u_long down_speed =0;
	int ret = ALINKGW_OK;

	if(0 == ali_get_connection_status())
		return ;
	
	now = time(0);
	gmtime_r(&now,&tm);

	if(tm.tm_year == 70)
	{
		return;
	}

	if(tm.tm_min%30 == 0)
	{	
		if(need_report == 0)
		{
			need_report = 1;
			get_stream_statistic_speed(&up_speed,&down_speed);
			printf("Report wan speed per 30min,now %d:%d ,get up_speed:%ld down_speed:%ld\n", tm.tm_hour, tm.tm_min, up_speed , down_speed);
			memset(buf , 0, sizeof(buf));
			sprintf(buf, "%ld",  down_speed);
			ret = ALINKGW_report_attr_direct(ALINKGW_SUBDEV_ATTR_WANDLSPEED, ALINKGW_ATTRIBUTE_simple, buf);
			if(ret != ALINKGW_OK)
			{
				printf("%s, report attibute(%s) failed\n", __func__, ALINKGW_SUBDEV_ATTR_WANDLSPEED);		
			}
			memset(buf , 0, sizeof(buf));
			sprintf(buf, "%ld",  up_speed);
			ret = ALINKGW_report_attr_direct(ALINKGW_SUBDEV_ATTR_WANULSPEED, ALINKGW_ATTRIBUTE_simple, buf);
			if(ret != ALINKGW_OK)
			{
				printf("%s, report attibute(%s) failed\n", __func__, ALINKGW_SUBDEV_ATTR_WANULSPEED);		
			}
		}
	}
	else
	{
		need_report = 0;
	}
}

int wan_speed_detecting_report()
{
	char buf[64] = {0};
	unsigned long last_up_bytes;
	unsigned long last_down_bytes;
	unsigned long up_bytes;
	unsigned long down_bytes;
	int ret ;
	report_thread_running = 1;
	/*修改为0.5秒上报一次，增加app端计算平均值的准确性*/
	int duration = gWanSpeedDetect.duration*2;

	if(0 == ali_get_connection_status())
		return 0;
	
	while(gWanSpeedDetect.enable == 1 && gWanSpeedDetect.duration > 0 )
	{
		if_TX_RX_status(speed_detect_ifname, &last_up_bytes, &last_down_bytes);
		cyg_thread_delay(50);
		if(g_detect_state != DETECTING)
			continue;
		duration--;
		if_TX_RX_status(speed_detect_ifname, &up_bytes, &down_bytes);
		
		printf("Report wan speed per 0.5s . get down_speed:%ld bytes ; up_speed:%ld bytes.\n", (down_bytes - last_down_bytes)*2 , (up_bytes - last_up_bytes)*2);
		memset(buf , 0, sizeof(buf));
		sprintf(buf,"%lu",  (down_bytes - last_down_bytes)*16);
		ret = ALINKGW_report_attr_direct(ALINKGW_SUBDEV_ATTR_DLBWINFO, ALINKGW_ATTRIBUTE_simple, buf);

		memset(buf , 0, sizeof(buf));
		sprintf(buf,"%lu",  (up_bytes - last_up_bytes)*16);
		ret = ALINKGW_report_attr_direct(ALINKGW_SUBDEV_ATTR_ULBWINFO, ALINKGW_ATTRIBUTE_simple, buf);
	}
	report_thread_running = 0;
	return 0;
 }

void init_detect_ctx_fd()
{
	int i;
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		detect_ctx[i].sockfd = -1;
	}
}
void close_detect_ctx_fd()
{
	int i;
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		if (detect_ctx[i].sockfd >= 0) 
			close(detect_ctx[i].sockfd);	
	}
}

void init_ip_list()
{	
	int i;
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		g_ip_list[i].sin_addr.s_addr = 0;
	}
}

int add_ip_list(in_addr_t s_addr)
{
	int i;
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		if(s_addr ==g_ip_list[i].sin_addr.s_addr )
			return 0;
	}
	
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		if(0 ==g_ip_list[i].sin_addr.s_addr )
		{
			g_ip_list[i].sin_addr.s_addr = s_addr;
			g_ip_count++;
			return 0;
		}		
	}
	return 0;
}

void del_ip_list(in_addr_t s_addr)
{	
	int i;
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		if(s_addr ==g_ip_list[i].sin_addr.s_addr )
		{
			g_ip_list[i].sin_addr.s_addr = 0;
			g_ip_count--;
			return ;
		}	
	}
}

void show_ip_list()
{
	int i;
	if( g_ip_list[0].sin_addr.s_addr == 0)
	{
		printf("ip list is null.\n");
		return;
	}
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		if( g_ip_list[i].sin_addr.s_addr != 0){
			printf("%d s_addr:%u %s\n",i+1, g_ip_list[i].sin_addr.s_addr ,inet_ntoa_tenda(g_ip_list[i].sin_addr.s_addr));
		}
	}
}

int wan_speed_detecting()
{
	int  selectfd = 0 ,sockfd_max = 0;
	fd_set readfds;
	struct timeval tv;
	int i = 0,ret = 0, count = 0, len = 0, port = 80;
	char strRequest[512] = {0}, strResponse[2048*2] = {0};
	char req_host[64] = {0}, host[64] = {0}, req_get[64] = {0},dst[16] = {0};
	struct hostent *ht = NULL;
	const char *url = "image.tv.yunos.com/product/500M.dat";
	char *mib_val;
	struct timeval s_time, e_time;
	unsigned long last_up_bytes, last_down_bytes, up_bytes, down_bytes;
	detect_thread_running = 1;
	g_detect_state = DETECT_START;
	_GET_VALUE(ALILINK_SPEEDTEST_LIST(1), mib_val);
	printf("get %s , value=%s\n", ALILINK_SPEEDTEST_LIST(1),mib_val);
	if(strcmp(mib_val, "") != 0)
	{
		if(2 != sscanf(mib_val,"%[^/]/%s",req_host,req_get)) {
			if(2 != sscanf(url,"%[^/]/%s",req_host,req_get)) {
				ret = -1;
				goto exit;
			}
		}
	}
	else
	{	
		if(2 != sscanf(url,"%[^/]/%s",req_host,req_get)) {
			ret = -1;
			goto exit;
		}
	}
	sscanf(req_host,"%[^:]:%d",host,&port);

   	printf("host:%s port:%d\n",host, port);
	sprintf(strRequest, "GET /%s HTTP/1.1\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/5.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
		"Host: %s\r\n"
		"Connection: keep-alive\r\n\r\n", req_get, host);
	
	if(g_ip_count < MAX_DETECT_NUM)
	{
	   	if((ht = gethostbyname(host)) == NULL)
	   	{
			ret = -1;
			goto exit;
	   	}
	}
	else
		printf("ip count is full [%u], use g_ip_count , not use gethostbyname.\n", g_ip_count);
//	int nRecvBuf=64*1024;
//	memset(detect_ctx, 0x0 , sizeof(struct detect_ctx) * MAX_DETECT_NUM);
	
	init_detect_ctx_fd();
	struct in_addr sin_addr;
	printf("###before gethostbyname:\n");
	show_ip_list();
	printf("###gethostbyname:%s get ip list:\n",host);
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		if (ht && ht->h_addr_list[i] != NULL)
		{ 
			printf("IP:%s\n", inet_ntop(ht->h_addrtype,ht->h_addr_list[i],dst,sizeof(dst))); 
			sin_addr = *((struct in_addr *)ht->h_addr_list[i]);
			add_ip_list(sin_addr.s_addr);
		}
		else
		{
			break;          /*退出for循环 */
		}
	}
	printf("###after gethostbyname:\n");
	show_ip_list();
	for (i = 0; i<MAX_DETECT_NUM; i++)
	{
		if( g_ip_list[i].sin_addr.s_addr != 0)
		{ 
	
			detect_ctx[i].dest_addr.sin_family = AF_INET;
    			detect_ctx[i].dest_addr.sin_port = htons(port);
   			detect_ctx[i].dest_addr.sin_addr.s_addr = g_ip_list[i].sin_addr.s_addr ;
			if((detect_ctx[i].sockfd =  socket(AF_INET, SOCK_STREAM, 0)) <0)
			{
				printf("socket create failed\n");
				detect_ctx[i].sockfd = -1;
				continue;
			}
		//	struct timeval timeout = {1,0}; 
			 //设置发送超时
		//	setsockopt(detect_ctx[i].sockfd, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout,sizeof(struct timeval));
			//设置接收超时
		//	setsockopt(detect_ctx[i].sockfd, SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
		//	if(setsockopt(detect_ctx[i].sockfd, SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int)) < 0)
		//		printf("_____setsockopt nRecvBuf error._____\n");
			if (-1 == connect(detect_ctx[i].sockfd, (struct sockaddr *)&detect_ctx[i].dest_addr, sizeof(struct sockaddr)))
			{
				printf("connect failed\n");
				close(detect_ctx[i].sockfd);
				detect_ctx[i].sockfd = -1;
				del_ip_list(g_ip_list[i].sin_addr.s_addr);
				continue;
			}
			if (-1 == send(detect_ctx[i].sockfd, strRequest, strlen(strRequest), 0))
			{
				printf("send failed\n");
				close(detect_ctx[i].sockfd);
				detect_ctx[i].sockfd = -1;
				del_ip_list(g_ip_list[i].sin_addr.s_addr);
				continue;
			} 
			count++;
		}
	}
	
	
	gettimeofday(&s_time, NULL);
//	printf("tv_sec:%d tv_usec:%d \n",s_time.tv_sec, s_time.tv_usec);
	if_TX_RX_status(speed_detect_ifname, &last_up_bytes, &last_down_bytes);
	g_detect_state = DETECTING;
	while (1)
	{
		if(gWanSpeedDetect.enable == 0 ||  gWanSpeedDetect.duration == 0 )
			break;
		
		FD_ZERO(&readfds);
		sockfd_max = 0;
		for (i = 0; i < count; i++) 
		{
			if (detect_ctx[i].sockfd != -1) 
			{
				FD_SET(detect_ctx[i].sockfd, &readfds);
				if(detect_ctx[i].sockfd > sockfd_max)
					sockfd_max = detect_ctx[i].sockfd;
			}
		}
		
		tv.tv_sec = 1;		
		tv.tv_usec = 0;
		
		selectfd = select(sockfd_max + 1, &readfds, NULL, NULL, &tv); 

		if(selectfd < 0)
		{
			break;
		}
		else if(selectfd == 0)
		{
			continue;
		}
		else
		{
			for (i = 0; i < count; i++) 
			{
				if (FD_ISSET(detect_ctx[i].sockfd, &readfds))
				{
					if ((len = recv(detect_ctx[i].sockfd, strResponse, sizeof(strResponse), 0)) <= 0)
					{
						printf("recv failed. len=%d\n",len);
						close(detect_ctx[i].sockfd);
						FD_CLR(detect_ctx[i].sockfd, &readfds);
						detect_ctx[i].sockfd = -1;
					}
					
				}
			}
		}
	}
	close_detect_ctx_fd();
	ret = 0;
	gettimeofday(&e_time, NULL);
	if_TX_RX_status(speed_detect_ifname, &up_bytes, &down_bytes);
//	printf("tv_sec:%d tv_usec:%d \n",e_time.tv_sec, e_time.tv_usec);
//	printf("OK. receive bytes:%lu \n", down_bytes - last_down_bytes);
	float time = (float) (e_time.tv_sec*MILLION + e_time.tv_usec - s_time.tv_sec*MILLION - s_time.tv_usec)/MILLION;
//	printf("time  %lf sec.\n",time);
	unsigned long bites = (( down_bytes - last_down_bytes)*8);

 	g_wan_rate = bites/time;
	g_wan_rate_float = (float) g_wan_rate/MILLION;
	
	printf("g_wan_rate = %lu bps  ,%lf Mb/s\n",g_wan_rate,g_wan_rate_float);
exit:
	g_detect_state = DETECTEND;
	detect_thread_running = 0;
	return ret;

}

void disable_speed_detect()
{
	printf("disable_speed_detect.\n");
	gWanSpeedDetect.enable = 0;
}

void create_wan_speed_detect_thread()
{
	cyg_thread_create(
		10, 
		(cyg_thread_entry_t *)wan_speed_detecting,
		0, 
		"Bandwidth Detection",
		wan_speed_detect_stack, 
		sizeof(wan_speed_detect_stack), 
		&wan_speed_detect_handle, 
		&wan_speed_detect_thread);
	cyg_thread_resume(wan_speed_detect_handle);
}

int wan_speed_test(int duration)
{
	if(detect_thread_running == 1)
	{
		return 0;
	}
	
	if(duration >= 5 && duration <= 30)
		gWanSpeedDetect.duration = duration;
	else
		gWanSpeedDetect.duration = 10;
	
	gWanSpeedDetect.enable = 1;
	printf("duration:%u\n", gWanSpeedDetect.duration);
	untimeout((timeout_fun *)disable_speed_detect,NULL);
	timeout((timeout_fun *)disable_speed_detect, NULL, (gWanSpeedDetect.duration)*100); 
	create_wan_speed_detect_thread();
	return 0;
}
	

int wan_speed_detect_main_loop()
{
	untimeout((timeout_fun *)disable_speed_detect,NULL);
	timeout((timeout_fun *)disable_speed_detect, NULL, (gWanSpeedDetect.duration)*100); 
	
	if(detect_thread_running == 0)
	{
		create_wan_speed_detect_thread();
	}
	
	if(report_thread_running == 0)
	{
		cyg_thread_create(
			2, 
			(cyg_thread_entry_t *)wan_speed_detecting_report,
			0, 
			"Bandwidth Report",
			wan_speed_report_stack, 
			sizeof(wan_speed_report_stack), 
			&wan_speed_report_handle, 
			&wan_speed_report_thread);
		cyg_thread_resume(wan_speed_report_handle);
	}
	return 0;
}

int set_wan_speed_detect(const char *json_in, char *buf, unsigned int sz)
{
	printf("%s %d json_in:%s buff:%s buf_sz:%u\n",__func__,__LINE__,json_in ,buf ,sz);
	cJSON *pJson = NULL;
	cJSON *pSub =NULL;
	int enable, duration;
	if( json_in == NULL)
	{
		return ALINKGW_ERR ;
	}
	pJson = cJSON_Parse(json_in);
	if(NULL == pJson)
	{
		printf("cJSON_Parse error.\n");
		return ALINKGW_ERR;
	}
	//enabled
	pSub = cJSON_GetObjectItem(pJson, "enabled");
	if(NULL == pSub)
	{
		printf("Without enabled \n");
		goto ERR;
	}
	if(pSub->valuestring ==NULL)
	{
		printf("enabled err.\n");
		goto ERR;
	}
	if( strlen(pSub->valuestring) != 1)
	{
		printf("enable len err,len=%d \n",strlen(pSub->valuestring));
		goto ERR;
	}
	if(strcmp(pSub->valuestring, "1") == 0)
		enable = 1;
	else if(strcmp(pSub->valuestring, "0") == 0)
		enable = 0;
	else
		goto ERR;
	//duration
	pSub = cJSON_GetObjectItem(pJson, "duration");
	if(NULL == pSub)
	{
		printf("Without duration \n");
		goto ERR;
	}
	if(pSub->valuestring ==NULL)
	{
		printf("duration err.\n");
		goto ERR;
	}
	duration = atoi(pSub->valuestring);
	printf("get enabled:%d duration:%d\n",enable, duration);
	if(duration >30 || duration <10)
		duration = 20;
	
	if(enable == 0)
	{
		gWanSpeedDetect.enable = 0;
	}
	else{	
		if(gWanSpeedDetect.enable == 0)
		{
			gWanSpeedDetect.enable = 1;
			gWanSpeedDetect.duration = duration;
			wan_speed_detect_main_loop();
		}
	}
	if(pJson)
		cJSON_Delete(pJson);
	return ALINKGW_OK;

ERR:
	if(pJson)
		cJSON_Delete(pJson);
	return ALINKGW_ERR;
}


