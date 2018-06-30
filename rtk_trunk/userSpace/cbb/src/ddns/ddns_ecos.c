/*
 * $ddns_main.c$
 */
#include <ddns.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <ecos_oslib.h>


#define DDNS_PRIORITY  15
#define DDNS_STACKSIZE 8192 //CYGNUM_HAL_STACK_SIZE_TYPICAL


//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================

//==============================================================================
//                               LOCAL VARIABLES
//==============================================================================
cyg_handle_t ddns_handle ;
cyg_thread ddns_thread ;
unsigned char ddns_stack[DDNS_STACKSIZE];

int DDNS_running = 0;

extern int str2arglist(char *, char **, char, int);
extern struct service_t *find_service(char *);


static void
ddns_main(void)
{
	DDNS_running = 1;
	/* Enter os independent main loop */
	DDNS_mainloop();

	DDNS_running = 0;
	return;
}

/*
 * Functions to raise the DNSMASQ daemon,
 * called by application main entry and
 * the mointor task.
 */
void
ddns_start(void)
{	
	/*
	 * Don't enable DDNS masquerate when
	 * router mode is not enabled.
	 */
	if (nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		||nvram_match(SYSCONFIG_WORKMODE, "client+ap")) {
		/* Terminate it anyway */
		if(DDNS_running == 1)
			DDNS_down();
		return;
	}
	if (nvram_match("ddns_enable", "0")) {
		/* Terminate it anyway */
		if(DDNS_running == 1)
			DDNS_down();
		return;
	}
	
	if (DDNS_running == 0) {
		cyg_thread_create(
			9,
			(cyg_thread_entry_t *)ddns_main,
			0,
			"DDNS",
			ddns_stack,
			sizeof(ddns_stack),
			&ddns_handle,
			&ddns_thread);
		cyg_thread_resume(ddns_handle);\

		/* Wait until thread scheduled */
		while (!DDNS_running)
			cyg_thread_delay(1);
	}
}

/*
 * Functions to shut down the DNSMASQ daemon,
 * called by the mointor task.
 */
void
ddns_stop(void)
{
#if 1//don't stop anyway,roy modify,2010/11/24,ddns bug fixed???
	int pid;
	if(DDNS_running == 1)
		DDNS_down();
	else
		return;
	/* Wait until thread exit */
	pid = oslib_getpidbyname("DDNS");
	if (pid) {
		while (oslib_waitpid(pid, NULL) != 0)
			cyg_thread_delay(1);
	}
#endif
	return;
}

void
ddns_restart(void)
{
	ddns_stop();
	ddns_start();
}

//------------------------------------------------------------------------------
// FUNCTION
//		load_config
//
// DESCRIPTION
//	format: [enable];[type];[host_name];[username];[password];[mail_exchanger];[updated_ip];[updated_time];[retry_time]
//  
// PARAMETERS
//
//  
// RETURN
//		0	: success
//		-1:	: fail
//
//------------------------------------------------------------------------------
int load_config(void)
{
	struct user_info *account;
	char line[255];
	char *arglists[9];
//	struct user_info *info;
	struct service_t *service;
	strcpy(line,nvram_safe_get("ddns_set1"));
	if(strlen(line)>5)//simple check
	{
		if ((str2arglist(line, arglists, ';', 9) == 9) && (atoi(arglists[0]) == 1))
		{
			service = find_service(arglists[1]); //this value may return NULL
			if (service == NULL){
				if(DDNS_running == 1)
					DDNS_down();
				return -1;
			}
			account = (struct user_info * )malloc(sizeof(struct user_info));
			if (account == NULL)	
			{
				if(DDNS_running == 1)
					DDNS_down();
				return -1;	
			}
			bzero(account, sizeof(struct user_info));
			
			account->service = service;
			strcpy(account->host, arglists[2]);
			strcpy(account->usrname, arglists[3]);
			strcpy(account->usrpwd, arglists[4]);
			strcpy(account->mx, arglists[5]);
			account->updated_time = atoi(arglists[7]);
			account->trytime = atoi(arglists[8]);
			account->ticks = account->trytime * 100;
			account->ip = atoi(arglists[6]);
			switch (account->ip)
			{
			case 0:
				account->status = UPDATERES_ERROR; //need to do update
				break;
				
			//case -1:
			//	account->status = UPDATERES_SHUTDOWN; //should not do update
			//	break;
			
			default:
				account->status = UPDATERES_OK;
				break;
			}
			DDNS_add_account(account);
		}
	}
	
	return 0;
}



//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
void update_ddns_config(struct user_info *info)
{
	char *arglists[9];
	char line[255];
	char newline[255];
	char buf[255];
	
	strcpy(line,nvram_safe_get("ddns_set1"));
	if(strlen(line)>5)//simple check
	{
		strcpy(buf,line);
		if ((str2arglist(line, arglists, ';', 9) == 9) &&
			info->service == find_service(arglists[1]))
		{
			//Write update information to cfg
			sprintf(newline, "%s;%s;%s;%s;%s;%s;%d;%d;%d",
					arglists[0], arglists[1], arglists[2], arglists[3], arglists[4], arglists[5],
					info->ip, info->updated_time, info->trytime);
			DDNS_DBG("%s:%s\n%s\n",__FUNCTION__,buf,newline);		
			if (strcmp(buf,newline))		
			{
				nvram_set("ddns_set1",newline);
				//nvram_commit();
			}	
		}
	}

}


