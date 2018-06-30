#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#include "wlschedule.h"
#define MAX_WLAN_NUM 2
#define WLAN_NAME_MAX 8
#define NAME_MAX 32
#define SCHSTR_MAX 64
#define BUFFER_SIZE 128
#ifndef ONE_DAY_SECONDS
#define ONE_DAY_SECONDS 24*60*60
#endif
static char wlanSchedule_started=0;
static char wlanSchedule_running=0;
#ifdef HAVE_SYSTEM_REINIT
static char wlanSchedule_cleaning=0;
#endif
static cyg_sem_t sem_load;
cyg_uint8  wlanSchedule_stack[WLAN_SCHEDULE_THREAD_STACK_SIZE];
cyg_handle_t  wlanSchedule_thread;
cyg_thread  wlanSchedule_thread_object;
static char schStrBuff[SCHSTR_MAX];

typedef struct _scheduleItem
{
	char enable;
	char interfaceName[WLAN_NAME_MAX];
	int day;
	int fTime;
	int tTime;
} scheduleItem;
int getSchedule(scheduleItem *pSchedule,char* input)
{
	char *inputBuf=(char*)malloc(SCHSTR_MAX);
	char* token=NULL;
	char wlan_num=0;
	char wlanName[WLAN_NAME_MAX]={0};
	int i=0,index=0;

    if(!inputBuf)
    {
        diag_printf("malloc fail!\n");
        return -1;
    }	
	if(!input||!pSchedule||!strstr(input,"wlan0")&&!strstr(input,"wlan1"))
	{
		diag_printf("input error!\n");
		free(inputBuf);
		return -1;
	}
	bzero(inputBuf,SCHSTR_MAX);
	strcpy(inputBuf,input);
	for(i=0;i<MAX_WLAN_NUM;i++)
	{
		sprintf(wlanName,"wlan%d",i);
		if(strstr(inputBuf,wlanName))
			wlan_num=i+1;
	}
	
	
	token = strtok(inputBuf,",");
	while(token!=NULL)
	{
		if(strncmp(token,"wlan",4)==0)
		{
			index=atoi(token+4);
			strcpy(pSchedule[index].interfaceName,token);
			token = strtok(NULL,",");			
			if(!token) return -1;
			pSchedule[index].day=atoi(token);
			token = strtok(NULL,",");
			if(!token) return -1;
			pSchedule[index].fTime=atoi(token);
			token = strtok(NULL,",");
			if(!token) return -1;
			pSchedule[index].tTime=atoi(token);
			pSchedule[index].enable=1;
		}
		token = strtok(NULL,",");
	}
	if(inputBuf)
		free(inputBuf);
//	diag_printf("%s:%d day=%d ftime=%d ttime=%d\n",__FILE__,__LINE__,(pSchedule[0].day),(pSchedule[0].fTime),(pSchedule[0].tTime));
	return 0;
}
int check_wlanOnOff(char wlanIndex)
{
	int sock=0,flags=0;
	struct	ifreq	ifr={0};
	snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),"wlan%d",wlanIndex);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		diag_printf("socket error!\n");
		return -1;
	}
	if (ioctl(sock, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) 
	{		
		diag_printf("SIOCGIFFLAGS error!\n");
		close(sock);
		return (-1);
	}
	close(sock);
	if((ifr.ifr_flags & IFF_UP) != 0)
	{
		return 1;
	}	
	return 0;
}
extern int run_clicmd(char *command_buf);
int open_wlan(char wlanIndex)
{
	char buffer[BUFFER_SIZE]={0};
	sprintf(buffer,"ifconfig wlan%d up",wlanIndex);
	run_clicmd(buffer);
	return 0;
}
int close_wlan(char wlanIndex)
{
	char buffer[BUFFER_SIZE]={0};
	sprintf(buffer,"ifconfig wlan%d down",wlanIndex);
	run_clicmd(buffer);
	return 0;
}
int check_hit(scheduleItem * pSchedule,char wlanIndex,int* leftTime)
{
	time_t calTime={0};
	struct tm currentTime={0};
	int currentMin=0;
	int leftSleepTime=0;
	
	time(&calTime);
	
	memcpy(&currentTime,localtime(&calTime),sizeof(currentTime));
	currentMin=currentTime.tm_hour*60+currentTime.tm_min;
	//diag_printf("currentTime.tm_wday=%d pSchedule[wlanIndex].day=%d\n",(1<<currentTime.tm_wday) , pSchedule[wlanIndex].day);
	if(((1<<currentTime.tm_wday) & pSchedule[wlanIndex].day)
	 	&& currentMin>=pSchedule[wlanIndex].fTime
	 	&& currentMin<pSchedule[wlanIndex].tTime)
	{
		leftSleepTime=(pSchedule[wlanIndex].tTime-currentMin)*60;
		if(leftSleepTime<(*leftTime))
			(*leftTime)=leftSleepTime;
		return 1;
	}
	else
	{
		if(((1<<currentTime.tm_wday) & pSchedule[wlanIndex].day)
		&&currentMin<pSchedule[wlanIndex].fTime)		
			leftSleepTime=(pSchedule[wlanIndex].fTime-currentMin)*60;
		else
			leftSleepTime=ONE_DAY_SECONDS-(currentMin*60);	
		if(leftSleepTime<(*leftTime))
			(*leftTime)=leftSleepTime;
		return 0;
	}		
}

int wlanSchedule_main(cyg_addrword_t data)
{
	char i=0,hit=0,wlanOnOff=0;
	int leftSleepTime=0;
	scheduleItem schedule[MAX_WLAN_NUM]={0};
	
	while(1)
	{
		bzero(schedule,sizeof(schedule));
		if(strcmp(schStrBuff,"disable")==0)
		{
			//diag_printf("wlanSchedule disable!\n");
			wlanSchedule_running=0;
			return;
		}
		leftSleepTime=ONE_DAY_SECONDS+1;
		if(getSchedule(schedule,schStrBuff)<0)
		{
			diag_printf("invalid input!\n");
			wlanSchedule_running=0;
			return -1;
		}	
		for(i=0;i<MAX_WLAN_NUM;i++)
		{
			if(!schedule[i].enable)
				continue;
			hit=check_hit(schedule,i,&leftSleepTime);
			//diag_printf("%s:%d hit=%d\n",__FILE__,__LINE__,hit);
			wlanOnOff=check_wlanOnOff(i);
			//diag_printf("%s:%d wlanOnOff=%d\n",__FILE__,__LINE__,wlanOnOff);
			if(hit==1 &&wlanOnOff==0)
				open_wlan(i);
			else if(hit==0 &&wlanOnOff==1)
				close_wlan(i);
		}
		if(leftSleepTime==ONE_DAY_SECONDS+1)
		{
			//diag_printf("no wlan scheule enable!\n");
			wlanSchedule_running=0;
			return;
		}
		
		cyg_semaphore_timed_wait(&sem_load,cyg_current_time()+leftSleepTime*100);
#ifdef HAVE_SYSTEM_REINIT
		if(wlanSchedule_cleaning==1)
			break;
#endif
		//sleep(leftSleepTime);
	}
#ifdef HAVE_SYSTEM_REINIT
	wlanSchedule_cleaning=0;
#endif
}
int wlschedule_main(unsigned int argc, unsigned char *argv[])
{
	//diag_printf("enter wlanSchedule_startup\n");
	
	if(argc!=1)
	{
		diag_printf("invalid input!\n should be 'wlschedule wlan0,127,0,1440'/'wlschedule disable' format\n");
		return(-1);
	}
	
	strcpy(schStrBuff,argv[0]);
	if(!strcmp(schStrBuff,"disable")&&!wlanSchedule_running)
	{
		return 0;
	}
	//diag_printf("%s:%d schStrBuff=%s\n",__FILE__,__LINE__,schStrBuff);
	if (wlanSchedule_started==0)
	{		
		cyg_semaphore_init(&sem_load,0);
		cyg_thread_create(WLAN_SCHEDULE_THREAD_PRIORITY,
		wlanSchedule_main,
		0,
		"wlan schedule",
		wlanSchedule_stack,
		sizeof(wlanSchedule_stack),
		&wlanSchedule_thread,
		&wlanSchedule_thread_object);
		
		//diag_printf("Starting WLAN SCHEDULE thread\n");
		cyg_thread_resume(wlanSchedule_thread);
		wlanSchedule_started=1;
		
		wlanSchedule_running=1;
		return(0);
	}
	else
	{
		if(wlanSchedule_running==0)
		{
			cyg_thread_resume(wlanSchedule_thread);
			wlanSchedule_running=1;
		}else
			cyg_semaphore_post(&sem_load);
		//diag_printf("WLAN SCHEDULE is already running\n");
		return(-1);
	}
}
#ifdef HAVE_SYSTEM_REINIT
void clean_wlachedule()
{
	if(wlanSchedule_started)
	{
		wlanSchedule_cleaning=1;
		cyg_semaphore_post(&sem_load);
		while(wlanSchedule_cleaning)
		{
			cyg_thread_delay(20);
		}
		wlanSchedule_running=0;
		wlanSchedule_started=0;
		cyg_thread_kill(wlanSchedule_thread);
		cyg_thread_delete(wlanSchedule_thread);
	}
}
#endif
