/***********************************************************
 Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.

 FileName : wlconf.c
 Description : 调试用接口，通过串口调用
 Author : fh
 Version : V1.0
 Date :2015-12-30

 Function List:

 Others :

 
 History :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期    ：2016-01-08    修改者：fh
   内容     ：优化
   Version ：V1.1


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*******************************************************************/


#include"sys_utility.h"

extern int wlconf(char * ifname);



int wlconf_cmd_main(int argc,char **argv)
{
    int td_argc = argc;
    char **td_argv = argv;

	int i;
	printf("\033[32margc:%d \033[0m",argc);
	for (i=0; i<argc; i++) 
	{
		printf("\033[32m%s\033[0m",argv[i]);
		if(i!=(argc-1))
		{
			printf("\033[32m \033[0m");
		}
	}
	printf("\033[32m \033[0m\n");
	
	//cli用
	char *ifname = td_argv[1];
	char *op = td_argv[2];

	//shell用argc加1
	//argc += 1;
	//char *ifname=td_argv[0];
	//char *op = td_argv[1];
	
	if(argc == 2)
	{
		wlconf_down(ifname);
		wlconf(ifname);
		wlconf_start(ifname);
		//配置桥
		//brconfig bridge0 add wlan0
		RunSystemCmd(NULL_FILE, "brconfig", "bridge0", "add",ifname, NULL_STR);
	}
	else if(argc == 3)
	{
		if(0 == strcmp(op,"up"))
		{
			wlconf_start(ifname);
		}
		else if(0 == strcmp(op,"down"))
		{
			wlconf_down(ifname);
		}
		else if(0 == strcmp(op,"start"))
		{
			wlconf(ifname);
		}
		else
		{
			printf("\033[32mThe parameter error!!\033[0m\n");
		}
	}
	else 
	{
		printf("\033[32mThe number of parameter error!!\033[0m\n");
	}
	
    return 0;

}