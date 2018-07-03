
#include <string.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <cli.h>

#include "sys_ecos.h"
#include "sys_module.h"
#include "sys_option.h"

/*msg send 0 1 op=1,index=1*/
RET_INFO msg_test(int argc, char* argv[])
{
	RET_INFO ret =  RET_SUC;

	if(4 == argc)
	{	
		PI_PRINTF(MAIN,"argv[2]=%s,argv[3]=%s[%d]\n",argv[2],argv[3],atoi(argv[3]));
		if(RET_ERR == msg_send(atoi(argv[2]),atoi(argv[3]),NULL))
		{
			printf("send msg error!\n");
		}
	}
	else if(5 == argc)
	{
		PI_PRINTF(MAIN,"argv[2]=%s,argv[3]=%s[%d],msg=%s\n",argv[2],argv[3],atoi(argv[3]),argv[4]);
		if(RET_ERR == msg_send(atoi(argv[2]),atoi(argv[3]),argv[4]))
		{
			printf("send msg error!\n");
		}		
	}
	else
	{
		printf("\nage:\t\tmsg send xx[center id] xx[module id] xxx[msg]\n");
	}
	
	return ret;
}

CLI_NODE("msg",	msg_test,		"msg");