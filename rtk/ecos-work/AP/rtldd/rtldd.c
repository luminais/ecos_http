/*rtldd is a wrapper for dyndns oraydns tzo*/
#include <cyg/kernel/kapi.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include "rtldebug.h"
#include "rtldd.h"
#include "../apmib/apmib.h"

extern int	getopt(int argc, char* const * argv, const char* opts);

#ifdef __ECOS
#ifdef printf
#undef printf
#endif
#define printf diag_printf
#endif
/*debug flag*/
unsigned long debug_flag=0;

#ifdef HAVE_SYSTEM_REINIT
int rtldd_quitting = 0;
static int rtldd_type = -1;
static int rtldd_is_running = 0;
static cyg_sem_t rtldd_sem_load;
cyg_sem_t oray_sem_load;
#endif

int help(char *name)
{
	printf("help:\n");
	usage(name);
	return 1;
}
int version_dump()
{
	printf("ddns %s\n",VERSION);
	return 1;
}
int wrong_usage()
{
	printf("wrong command line\n");
	return 1;
}
int usage(char *name)
{
	printf("%s usage:\n",name);
	printf("--help or -h\n");	
	printf("--version or -v\n");	
	printf("serviecename  .... please refer to different service format\n");	
	return 1;
}

int register_server(char *servicename,int argc,char *argv[]) {
#ifdef SUPPORT_ORAY
	if(!strncmp(servicename,ORAY_DDNS,sizeof(ORAY_DDNS)))
	{
#ifdef HAVE_SYSTEM_REINIT
		rtldd_type = 1;
#endif
		return register_oray(argc,argv);
	}
#endif
#ifdef SUPPORT_DYN
	if(!strncmp(servicename,DYNDNS_DDNS,sizeof(DYNDNS_DDNS)))
	{
#ifdef HAVE_SYSTEM_REINIT
		rtldd_type = 2;
#endif
		return register_dyndns(argc,argv);
	}
#endif
#ifdef SUPPORT_TZO
	if(!strncmp(servicename,TZO_DDNS,sizeof(TZO_DDNS)))
	{
#ifdef HAVE_SYSTEM_REINIT
		rtldd_type = 3;
#endif
		return register_tzo(argc,argv);
	}
#endif
	return FAIL;
}
#if 1
int rtldd_main(int argc,char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int c;
	char *servername;
	int option_index;
	if(argc < 1)
	{
		usage(argv[0]);
		exit(FAIL);
	}
        /*if not help or version dump. get server wanted*/
        servername=argv[1];
        DEBUG_PRINT("servername (%s), optind (%d)\n",servername,1);
	if(OK == register_server(servername,argc,argv))
		return OK;
#if 0
	while(1) {
#if 0
		static struct option long_options[] = {
			{ "help",       0, 0, 'h' },
			{ "version",    0, 0, 'v' },
			{0, 0, 0, 0}
		};
		c = getopt_long (argc, argv, "hv",
                        long_options, &option_index);
#else
		c = getopt(argc, argv, "hv");
#endif
               	if (c == -1)
			break;

		switch (c) {
			case 'h':
				help(argv[0]);
				exit(OK);
			case 'v':
				version_dump();
				exit(OK);
			default:
				wrong_usage(argv[0]);
				exit(FAIL);
				
				
		}
	}
#endif
	return OK;
}

#define RTLDD_PRIORITY   28
#define RTLDD_STACKSIZE 0x00003000 
//#define RTLDD_STACKSIZE  CYGNUM_HAL_STACK_SIZE_TYPICAL

static unsigned char rtldd_stack[RTLDD_STACKSIZE];
static cyg_handle_t rtldd_handle;
static cyg_thread   rtldd_thread;

/*usage 
 * rtldd dyndns username:password hostname
 * rtldd oray -u username -p password -H hostname
 */

#ifndef HAVE_SYSTEM_REINIT
static int rtldd_reinit;
#endif
static void
rtldd(cyg_addrword_t data)
{

	int ret;
	int argc=0;
	int ddnstype;
	char *argv[10];
	char service[32]={0};
	char user[32]={0};
	char password[32]={0};
	char hostname[128]={0};
	char tmpbuf[64]={0};
	/*prepare parameter*/
#ifndef HAVE_SYSTEM_REINIT
_reinit:	
	rtldd_reinit=0;
#endif
	apmib_get(MIB_DDNS_TYPE,&ddnstype);
	apmib_get(MIB_DDNS_USER,user);
	apmib_get(MIB_DDNS_PASSWORD,password);
	apmib_get(MIB_DDNS_DOMAIN_NAME,hostname);
	argv[argc]="rtldd";
	/*0 means dyndns, 1 means orayddns*/
	if((0 == ddnstype)  || (2 == ddnstype))
	{
		argc++;
		if(0 == ddnstype)
			strcpy(service,DYNDNS_DDNS);
		else
			strcpy(service,TZO_DDNS);
		argv[argc]=service;
		argc++;
		memset(tmpbuf,0,sizeof(tmpbuf));
		sprintf(tmpbuf,"%s:%s",user,password);
		argv[argc]=tmpbuf;
		argc++;
		argv[argc]=hostname;
		argc++;
	}
	else if(1 == ddnstype)
	{
		argc++;
		strcpy(service,ORAY_DDNS);
                argv[argc]=service;
                argc++;
                argv[argc]="-u";
                argc++;
                argv[argc]=user;
                argc++;
                argv[argc]="-p";
                argc++;
                argv[argc]=password;
                argc++;
                argv[argc]="-H";
                argc++;
                argv[argc]=hostname;
		   argc++;
	}
	/*try register in loop*/
#ifdef HAVE_SYSTEM_REINIT
		rtldd_is_running = 1;
#endif
	while (1
#ifdef HAVE_SYSTEM_REINIT
		&& (rtldd_quitting == 0)
#endif
			) 
	{
#ifndef HAVE_SYSTEM_REINIT
		if(rtldd_reinit)
			goto _reinit;
#endif
		ret=rtldd_main(argc,argv);
		if(FAIL==ret)
		{
#ifdef HAVE_SYSTEM_REINIT
			cyg_semaphore_timed_wait(&rtldd_sem_load,cyg_current_time()+100*100);
#else
			sleep(100);
#endif
		}
		else
		{
#ifdef HAVE_SYSTEM_REINIT
			cyg_semaphore_timed_wait(&rtldd_sem_load,cyg_current_time()+100*100*100);
#else
			sleep(100*100);
#endif
		}
	}

#ifdef HAVE_SYSTEM_REINIT
	rtldd_quitting = 0;
	rtldd_is_running = 0;
#endif

}

int init_rtldd(void)
{
	/*start a thread*/
    int enabled;
    apmib_get(MIB_DDNS_ENABLED,&enabled);
    if(!enabled)
		return (-1);
#ifdef HAVE_SYSTEM_REINIT
	if(rtldd_is_running == 1)
		return;
	cyg_semaphore_init(&rtldd_sem_load,0);
	cyg_semaphore_init(&oray_sem_load,0);
#else
    if(rtldd_handle)
    {
 		rtldd_reinit=1;
		/*should wakeup thread*/
		return 0;
    }
#endif
    cyg_thread_create(RTLDD_PRIORITY, &rtldd, 0, "rtlddd",
                      rtldd_stack, RTLDD_STACKSIZE,
                      &rtldd_handle, &rtldd_thread);
    cyg_thread_resume(rtldd_handle);
	return 1;
}

#ifdef HAVE_SYSTEM_REINIT
void kill_rtldd()
{
	if(rtldd_is_running == 0)
		return;
	diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	rtldd_quitting = 1;
	cyg_semaphore_post(&rtldd_sem_load);
	cyg_thread_release(rtldd_handle);
	if(rtldd_type == 1)
	{
		cyg_semaphore_post(&oray_sem_load);
		rtldd_type = -1;
	}
	while(rtldd_quitting){
		cyg_thread_delay(200);
	}
	rtldd_is_running = 0;
	cyg_semaphore_destroy(&rtldd_sem_load);
	cyg_semaphore_destroy(&oray_sem_load);
	cyg_thread_kill(rtldd_handle);
	cyg_thread_delete(rtldd_handle);
}
#endif
