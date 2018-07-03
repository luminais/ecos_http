/*************************************************************************
	> Copyright (C) 2015, Tenda Tech. Co., All Rights Reserved.
	> File Name: cli_basic.c
	> Description: 
	> Author: zhuhuan
	> Mail: zhuhuan_IT@outlook.com
	> Version: 1.0
	> Created Time: Wed 23 Dec 2015 05:14:22 PM CST
	> Function List: 

	> History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Functions declaration */
extern double get_uptime(void);
extern int syslog_open();
extern int syslog_close();
extern char *syslog_get();
extern void syslog(int level, const char *fmtStr, ...);

int time_cmd(int argc, char *argv[])
 {
	 unsigned long sec, mn, hr, day;
	 sec = (unsigned long)get_uptime();
	 day = sec / 86400;
	 sec %= 86400;
	 hr = sec / 3600;
	 sec %= 3600;
	 mn = sec / 60;
	 sec %= 60;  
	 printf("Up time: %luday %luh %lum %lus\n", day, hr, mn, sec);
 
	 time_t timer;
	 struct tm *tblock;
	 timer = time(NULL);
	 tblock = localtime(&timer);
	 printf("Local time: %s\n",asctime(tblock));
	 
	 return 0;
 }
 
int syslog_cmd(int argc, char *argv[])
 {
	 int len = 0;
	 int a = 1;
	 char buf[1024] = {0};
	 char temp[1024] ={0};
	 char *msg;
	 int max, i = 0;
 
	 if (argc >= 3 && !strcmp(argv[1], "set")) {
		 max = atoi(argv[2]);
		 printf("Set %d entries to syslog\n", max);
		 while (i++ <= max) {
			 snprintf(temp, sizeof(temp), "%s%d", buf, a);
			 strcpy(buf,temp);
			 len += strlen(buf);
			 syslog(0, buf);
			 if (a ==9)
				 a = 0;
			 else
				 a++;
		 }
	 }
	 else if (argc < 2 || !strcmp(argv[1], "show")) {
		 syslog_open();
		 while((msg = syslog_get())) {
			 printf("%s\n", msg);
		 }
		 syslog_close();
	 }
	 else {
		 printf("syslog [set num] [show]\n");
		 return -1;
	 }
		 
	 return 0;
 }


#ifdef __CONFIG_MUTEX_DEBUG__
typedef  struct mutex_locked
{

	char * mutex_locked_ptr;
	int mutex_try;
	int mutex_couter;
}mutex_locked_t;


typedef struct mutex_print
{
	
	char thread_name_buf[128];
	mutex_locked_t mutex_locked_arry[128];
	char mutex_locked_arry_over;  //ÊÇ·ñÒç³ö
	
}mutex_print_t;




mutex_print_t g_mutex_print[32] = {0};
int g_mutex_print_over= 0;

void g_mutex_print_fun( void)
{
	
	
	int i, j;
	for (i = 0; i < 32; i++)
	{
			
		if (g_mutex_print[i].thread_name_buf[0])
		{
			
			printf ("thread=%s  mutex_locked_arry_over=%d=\n", g_mutex_print[i].thread_name_buf,g_mutex_print[i].mutex_locked_arry_over);
			for (j = 0; j < 128; j++)
			{
							
				if (g_mutex_print[i].mutex_locked_arry[j].mutex_locked_ptr) 
				{
					printf ("%p: try:%d: counter:%d\n", g_mutex_print[i].mutex_locked_arry[j].mutex_locked_ptr,
					g_mutex_print[i].mutex_locked_arry[j].mutex_try, g_mutex_print[i].mutex_locked_arry[j].mutex_couter);		
				}
			}			
		}
	}

	printf ("g_mutex_print_over==%d=\n", g_mutex_print_over);
}

#endif
int debug_cmd(int argc, char *argv[])
{
	int debugValue;

	
#ifdef __CONFIG_MUTEX_DEBUG__
	g_mutex_print_fun();
#endif
		
	if(argc == 3)
	{
		debugValue = atoi(argv[2]);
	#ifdef __CONFIG_AUTO_CONN_CLIENT__
		if(0 == strcmp(argv[1],"auto_conn_extend"))
		{
			auto_conn_debug_level(debugValue);
		}
	#endif
		//for httpd debug,yp
		if(0 == strcmp(argv[1],"httpd"))
		{
			HTTP_debug_level(debugValue);
		}
        // dhcp server debug, added by zhuhuan on 2016.05.15
        #ifdef __CONFIG_A9__
        extern void dhcpd_debug_level(int level);
		if(0 == strcmp(argv[1],"dhcpd"))
		{
			dhcpd_debug_level(debugValue);
		}
        #endif
	}
#if 0

		 else if(strcmp(argv[1],"wl_restart") == 0){
			 wl_restart_check_debug_level(debugValue);
		 }
	#ifdef __CONFIG_ALINKGW__
		 else if(strcmp(argv[1],"probe") == 0){
			 probe_info_debug_level(debugValue);
		 }
		 else if(strcmp(argv[1],"tpsk") == 0){
			 tpsk_debug_level(debugValue);
		 }
	#endif
	#ifdef __CONFIG_AUTO_CONN_SERVER__
		 else if(strcmp(argv[1],"auto_conn_route") == 0)
		 {
			 auto_conn_debug_level(debugValue);
		 }
	#endif
		 else if(strcmp(argv[1],"dhcpd") == 0)
		 {
			 dhcpd_debug_level(debugValue);
		 }
		 //to be added
	 }else{
	#ifdef __CONFIG_ALINKGW__
		 printf("debug <httpd | wl_restart | dhpcd | probe | tpsk > <level>\n");
	#else
		 printf("debug <httpd | wl_restart | dhpcd> <level>\n");
	#endif
		 return -1;
	 }
 #endif
 	else
	{
		printf("debug <module name> <level>\n");
		#ifdef __CONFIG_AUTO_CONN_CLIENT__
		printf("       auto_conn_extend\n");
		#endif
		printf("       httpd\n");
		#ifdef __CONFIG_A9__
		printf("       dhcpd\n");
		#endif
		return -1;
	}
	return 0;
}

